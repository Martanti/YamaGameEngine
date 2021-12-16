#ifdef GAME
export module Component.Physics;
import "btBulletDynamicsCommon.h";
import <memory>;
import <deque>;
import <set>;
import <algorithm>;
import Component.Transform;
import Logger;
import YaMath;
using std::unique_ptr;
using std::make_unique;

/**
 * @brief Collider shapes
*/
export enum ColliderShape
{
	Empty,
	Box
};

/**
 * @brief Physics managed component.
*/
export class PhysicalBody
{
private:
	/**
	 * @brief An owned reference to an entity within the physics engine.
	 * @note For internal purposes only.
	*/
	unique_ptr<btRigidBody> mBody;

	/**
	 * @brief A non owning reference to a physics world, that is managed by the wrapper.
	 * @note Reference to the world used only for proper deletion. For internal purposes only.
	*/
	btDynamicsWorld* mWorld;

	// The construction should only be done via the wrapper as it does all the needed assignments.

	PhysicalBody(btDynamicsWorld* const world) : mWorld(world), mBody(nullptr) {};

	// A casted entity id from ECS
	// Used as there was no other way to get from the component to entity
	// For getting info about collided entities
	// Shouldn't have more entities than MAX_INT
	// The libraries themselves are not included to reduce the dependencies and possible file bloat (as it's modules and they are "compiled")
	// The components can't be reassigned for different entities

	/**
	 * @brief An entity ID representing Entity within the ECS.
	*/
	int entityId = -1;

	std::set<int> mLastFrameCollidedObject;
	std::set<int> mItemsCollidedWith;

	std::set<int> mItemsLeftCollsionWith;
	std::set<int> mItemsStayedInCollisionWith;
	std::set<int> mItemsEnteredInCollisionWith;

private:
	/**
	 * @brief compute the entity IDs that this object has left collision with.
	*/
	inline void CalculateCollisionExits()
	{
		// old - new = exit
		std::set_difference(mLastFrameCollidedObject.begin(), mLastFrameCollidedObject.end(), mItemsCollidedWith.begin(), mItemsCollidedWith.end(), std::inserter(mItemsLeftCollsionWith, mItemsLeftCollsionWith.end()));
	}

	/**
	 * @brief compute the entity IDs that this object has stayed in collision with.
	*/
	inline void CalculateCollisionStays()
	{
		// old & new = stay
		std::set_intersection(mLastFrameCollidedObject.begin(), mLastFrameCollidedObject.end(), mItemsCollidedWith.begin(), mItemsCollidedWith.end(), std::inserter(mItemsStayedInCollisionWith, mItemsStayedInCollisionWith.end()));
	}

	/**
	 * @brief compute the entity IDs that this object has entered into collision with.
	*/
	inline void CalculateCollisionEnters()
	{
		// new - old = enter
		std::set_difference(mItemsCollidedWith.begin(), mItemsCollidedWith.end(), mLastFrameCollidedObject.begin(), mLastFrameCollidedObject.end(), std::inserter(mItemsEnteredInCollisionWith, mItemsEnteredInCollisionWith.end()));
	}

public:
	/**
	 * @brief Collider shape.
	*/
	ColliderShape mShape;

	// These could be deque, but the issue is that I am already throwing away the data for collision point, so what's the point
	yamath::Vec3D mOffset;

public:
	~PhysicalBody()
	{
		if (mBody)
		{
			if (mWorld)
				mWorld->removeCollisionObject(mBody.get());

			auto motionState = mBody->getMotionState();
			if (motionState)
				delete motionState;

			auto shape = mBody->getCollisionShape();
			if (shape)
				delete shape;
		}
	}

	/**
	 * @brief Get entities the object has entered into collision with.
	 * @return
	*/
	inline std::set<int> GetItemsEnterdCollisionWith() const noexcept
	{
		return mItemsEnteredInCollisionWith;
	}

	/**
	 * @brief Get entities the object has stayed in collision with.
	 * @return
	*/
	inline std::set<int> GetItemsStayedCollisionWith() const noexcept
	{
		return mItemsStayedInCollisionWith;
	}

	/**
	 * @brief Get entities the object has left the collision with.
	 * @return
	*/
	inline std::set<int> GetItemsLeftCollisionWith() const noexcept
	{
		return  mItemsLeftCollsionWith;
	}

	/**
	 * @brief Add ID to the set of items the object has collided with in this frame.
	 * @param
	 * @return
	*/
	inline void AddIDToItemsCollidedWith(int ID) noexcept
	{
		mItemsCollidedWith.insert(ID);
	}

	// Copy is not intended because the object has to be added via the wrapper in order to register the new entity
	PhysicalBody(PhysicalBody const& copy) = delete;
	PhysicalBody& operator=(PhysicalBody const& copy) = delete;

	PhysicalBody(PhysicalBody&& move) noexcept :
		mBody(std::move(move.mBody)),
		mShape(std::exchange(move.mShape, ColliderShape::Empty)),
		mItemsCollidedWith(std::move(move.mItemsCollidedWith)),
		mLastFrameCollidedObject(std::move(move.mLastFrameCollidedObject)),
		mWorld(std::move(move.mWorld)),
		mOffset(std::move(move.mOffset)),
		entityId(std::exchange(move.entityId, -1)),
		mItemsLeftCollsionWith(std::move(move.mItemsLeftCollsionWith)),
		mItemsStayedInCollisionWith(std::move(move.mItemsStayedInCollisionWith)),
		mItemsEnteredInCollisionWith(std::move(move.mItemsEnteredInCollisionWith))

	{
		// Otherwise it will reference the object moved from and the casting will break;
		if (mBody)
			mBody->setUserPointer(this);
	}
	PhysicalBody& operator= (PhysicalBody&& move) noexcept
	{
		mBody = std::move(move.mBody);


		if (mBody)
			mBody->setUserPointer(this);
		mShape = std::exchange(move.mShape, Empty);
		mItemsCollidedWith = std::move(move.mItemsCollidedWith);
		mLastFrameCollidedObject = std::move(move.mLastFrameCollidedObject);
		mWorld = std::move(move.mWorld);
		mOffset = std::move(move.mOffset);

		// This is working for now, but I am not sure what will happen if the entity ID will be reused by entt, which I'm not sure if it does
		// As a note entt does move and pop when deleting an element, meaning the move operators will be used
		entityId = std::exchange(move.entityId, -1);

		mItemsLeftCollsionWith = std::move(move.mItemsLeftCollsionWith);
		mItemsStayedInCollisionWith = std::move(move.mItemsStayedInCollisionWith);
		mItemsEnteredInCollisionWith = std::move(move.mItemsEnteredInCollisionWith);


		return *this;
	}

	/**
	 * @brief Update the physics component with the position and rotation.
	 * @note Would be decoupled if there were uses cases for that.
	*/
	inline void SetPositionRotation(yamath::Vec3D const& pos, yamath::Vec3D const& rot)
	{
		// A new motion state has to be created as if only an old one is set it will get replaced by what was set in the Create physics item function
		delete mBody->getMotionState();
		btTransform transform;
		transform.setIdentity();
		auto offset = yamath::opr::Add(pos, mOffset);
		transform.setOrigin(btVector3(offset.X, offset.Y, offset.Z));

		btQuaternion qtrn(0, 0, 0);
		qtrn.setEulerZYX(yamath::opr::ToRadians(rot.Z),
						 yamath::opr::ToRadians(rot.Y),
						 yamath::opr::ToRadians(rot.X));
		transform.setRotation(qtrn);


		auto motion = new btDefaultMotionState(transform);
		mBody->setMotionState(motion);
	}

	/**
	 * @brief And impulse force to the object.
	*/
	inline void AddImpulseForce(yamath::Vec3D const& force, yamath::Vec3D const& relativePos = { 0, 9, 0 })
	{
		btVector3 appliedForce(force.X, force.Y, force.Z);
		btVector3 relativePosition(relativePos.X, relativePos.Y, relativePos.Z);
		mBody->applyImpulse(appliedForce, relativePosition);
	}

	/**
	 * @brief Set linear velocity.
	 * @return
	*/
	inline void SetLinearVelocity(float xVel, float yVel, float zVel)
	{
		btVector3 linearVelocity(xVel, yVel, zVel);
		mBody->setLinearVelocity(linearVelocity);
	}

	/**
	 * @brief Get current
	 * @param relativePosition
	 * @return
	*/
	inline yamath::Vec3D GetVelocity(yamath::Vec3D const& relativePosition = { 0, 0, 0 })
	{

		auto btVelocity = mBody->getVelocityInLocalPoint(btVector3(relativePosition.X, relativePosition.Y, relativePosition.Z));

		return { btVelocity.getX(), btVelocity.getY(), btVelocity.getZ() };
	}

	// This approach creates an expectation that collision detection will be called every frame, which it wasn't at high game steps :'(

	// Collisions lists should be treated like this:
	// 0) mLastFrameCollidedObject contains the data from the last frame
	// 1) list for this frame is populated with new data in the current frame
	// 2) a difference is counted, which results in collision left, new and stay
	// 3) current frame data is flushed to the mLastFrameCollidedObject, so it has it in the next frame

	/**
	 * @brief Flush the previous collision data.
	 * To prepare for the next step.
	*/
	inline void FlushCollision()
	{
		mItemsLeftCollsionWith.clear();
		mItemsStayedInCollisionWith.clear();
		mItemsEnteredInCollisionWith.clear();

		mLastFrameCollidedObject = mItemsCollidedWith;
		mItemsCollidedWith.clear();
	}

	/**
	 * @brief Calculate the newest data about physics events.
	 * @return
	*/
	inline void CalculateCollisionCollections()
	{
		CalculateCollisionStays();
		CalculateCollisionExits();
		CalculateCollisionEnters();
	}

	/**
	 * @brief Gets stored entity ID.
	 * @return
	*/
	inline int GetEntityId() const noexcept
	{
		return entityId;
	}

	/**
	 * @brief Disable the sleep state on this object.
	 * @note Sleep will cause still objects to not be updated, and will wake up only on collision. Meaning, applying forces will be impossible without collision.
	*/
	inline void DisableSleep()
	{
		mBody->setActivationState(DISABLE_DEACTIVATION);
	}

	/**
	 * @brief Log the position of the physics object.
	 * @note Purely for debugging purposes.
	*/
	inline void LogPos() const
	{
		btTransform btTrans;
		mBody->getMotionState()->getWorldTransform(btTrans);

		auto btVec3 = btTrans.getOrigin();
		Log(LGR_INFO, std::format("Position at: {} {} {}", btVec3.getX(), btVec3.getY(), btVec3.getZ()));
	}

	/**
	 * @brief Get the position of the physics object.
	 * @return
	*/
	inline yamath::Vec3D GetPosition() const
	{
		btTransform btTrans;
		mBody->getMotionState()->getWorldTransform(btTrans);

		auto btVec3 = btTrans.getOrigin();
		yamath::Vec3D origin{ btVec3.getX() , btVec3.getY(), btVec3.getZ() };

		return yamath::opr::Sub(origin, mOffset);
	}

	/**
	 * @brief Get the rotation of the physics object.
	 * @return
	*/
	inline yamath::Vec3D GetRotation() const
	{
		btTransform btTrans;
		mBody->getMotionState()->getWorldTransform(btTrans);

		yamath::Vec3D rotation;
		float x, y, z;

		auto a = mBody->getOrientation();
		// For this yaw is Z, pitch is y and roll is x,
		a.getEulerZYX(z, y, x);
		return { yamath::opr::ToDegrees(x),
				yamath::opr::ToDegrees(y),
				yamath::opr::ToDegrees(z) };
	}
	friend class PhysicsWrapper;
};
#endif // GAME
