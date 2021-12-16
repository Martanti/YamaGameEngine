#ifdef GAME
export module PhysicsWrapper;
import Logger;
import "btBulletDynamicsCommon.h";
import <memory>;
import Component.Transform;
import <vector>;
import YaMath;
export import Component.Physics;
using std::unique_ptr;
using std::make_unique;

bool collisionStartedCallback(btManifoldPoint& collisionPoint, btCollisionObjectWrapper const* obj1, int id1, int index1, btCollisionObjectWrapper const* obj2, int id2, int index2);

/**
 * @brief Wrapper for Bullet physics engine.
*/
export class PhysicsWrapper
{
private:
	/**
	 * @brief The physics world handler.
	*/
	unique_ptr<btDynamicsWorld> mWorld;

	/**
	 * @brief Physics dispatcher.
	*/
	unique_ptr<btDispatcher> mDispatcher;

	/**
	 * @brief Physics broad-phase.
	*/
	unique_ptr<btBroadphaseInterface> mBroadphase;

	/**
	 * @brief Physics constraint solver.
	*/
	unique_ptr<btConstraintSolver> mSolver;

	/**
	 * @brief Physics collision configuration.
	*/
	unique_ptr<btCollisionConfiguration> mCollisionConfig;

	/**
	 * @brief Gravity constant.
	*/
	btVector3 const mGravity = btVector3(0, -10.f, 0);
public:
	PhysicsWrapper()
	{
		mCollisionConfig = make_unique<btDefaultCollisionConfiguration>();
		mDispatcher = make_unique<btCollisionDispatcher>(mCollisionConfig.get());
		mBroadphase = make_unique<btDbvtBroadphase>();
		mSolver = make_unique<btSequentialImpulseConstraintSolver>(); // should be able to be used with openCL to improve the performance

		mWorld = make_unique<btDiscreteDynamicsWorld>(mDispatcher.get(), mBroadphase.get(), mSolver.get(), mCollisionConfig.get());
		mWorld->setGravity(mGravity);

		gContactAddedCallback = collisionStartedCallback;
	}

	~PhysicsWrapper() {}

	/**
	 * @brief Physics step update.
	 * @param timeStep
	*/
	void Update(float const& timeStep)
	{
		mWorld->stepSimulation(timeStep);
	}

	/**
	 * @brief Create a cube shape collider.
	 * @param extents Half dimensions.
	 * @param mass
	 * @param offset
	 * @param entityID
	 * @return
	*/
	PhysicalBody CreatePhysicsCube(yamath::Vec3D const& extents, float const& mass = 0, yamath::Vec3D const& offset = { 0, 0, 0 }, int const& entityID = -1) const
	{
		btVector3 btDimensions(
			extents.X,
			extents.Y,
			extents.Z);

		auto boxShape = new btBoxShape(btDimensions);

		// This can be moved to some private function as all shapes will have the same init after this point
		btVector3 inertia(0, 0, 0);
		if (mass != 0)
			boxShape->calculateLocalInertia(mass, inertia);

		btTransform t;
		t.setIdentity();
		// An obviously unrealistic position. As it will be replaced later either way, this is going to be a good indicator if position is not being set properly
		t.setOrigin(btVector3(777, 777, 777));
		auto motion = new btDefaultMotionState(t);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motion, boxShape);
		PhysicalBody cube(mWorld.get());
		cube.mShape = ColliderShape::Box;
		cube.mBody = make_unique<btRigidBody>(rbInfo);
		// Declare that the object collision will call custom callback
		cube.mBody->setCollisionFlags(cube.mBody->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);

		// Store the component so it can be retrieved from the collision
		cube.mBody->setUserPointer(&cube);
		cube.mOffset = offset;
		cube.entityId = entityID;
		mWorld->addRigidBody(cube.mBody.get());
		return cube;
	}
};

/**
 * @brief Collision handling callback function.
 * @param collisionPoint
 * @param obj1
 * @param id1
 * @param index1
 * @param obj2
 * @param id2
 * @param index2
 * @return
*/
bool collisionStartedCallback(btManifoldPoint& collisionPoint, btCollisionObjectWrapper const* obj1, int id1, int index1, btCollisionObjectWrapper const* obj2, int id2, int index2)
{
	auto component1 = static_cast<PhysicalBody*>(obj1->m_collisionObject->getUserPointer());
	auto component2 = static_cast<PhysicalBody*>(obj2->m_collisionObject->getUserPointer());

	auto comp1EntityId = component1->GetEntityId();
	auto comp2EntityId = component2->GetEntityId();

	if (comp2EntityId > -1)
		component1->AddIDToItemsCollidedWith(comp2EntityId);

	if (comp1EntityId > -1)
		component2->AddIDToItemsCollidedWith(comp1EntityId);
	return false;
}
#endif // GAME
