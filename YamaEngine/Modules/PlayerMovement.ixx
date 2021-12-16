#ifdef GAME
module;
#include "entity/registry.hpp";
export module GameLogic.Player;
import GameLogic.Base;
import Component.GraphicsComponents;

/**
 * @brief A script meant to allow player to move around the scene.
*/
export class PlayerMovement : public GameScriptBase
{
private:
	/**
	 * @brief A flag to stop player from flying into the sky.
	*/
	bool mOnGround = false;
public:
	[[nodiscard]] inline YmEventReturn Start(float deltaTime, entt::registry& registry, entt::entity const& thisEntity) override
	{
		auto* physicalBody = registry.try_get<PhysicalBody>(thisEntity);

		if (!physicalBody)
			return {};

		physicalBody->DisableSleep();

		return {};
	}

	[[nodiscard]] inline YmEventReturn Update(float deltaTime, entt::registry& registry, entt::entity const& thisEntity) override
	{
		auto* camera = registry.try_get<CameraComponent>(thisEntity);

		if (camera)
		{
			// Ignoring the unused
			auto [xForward, _, zForward] = camera->GetCameraForward();
			auto [xRight, __, zRight] = camera->GetCameraRight();

			auto* physicalBody = registry.try_get<PhysicalBody>(thisEntity);
			if (!physicalBody)
				return {};
			auto speed = deltaTime * 30;

			auto [X, Y, Z] = physicalBody->GetVelocity();

			if (X > -10 && X < 10)
			{
				if (Input::GetKeyPressed(Input::KC_D))
				{
					physicalBody->AddImpulseForce({ xRight * speed, 0, zRight * speed });
				}

				else if (Input::GetKeyPressed(Input::KC_A))
				{
					physicalBody->AddImpulseForce({ -xRight * speed, 0, -zRight * speed });
				}
			}

			if (Z > -10 && Z < 10)
			{
				if (Input::GetKeyPressed(Input::KC_W))
				{
					physicalBody->AddImpulseForce({ xForward * speed, 0, zForward * speed });
				}

				else if (Input::GetKeyPressed(Input::KC_S))
				{
					physicalBody->AddImpulseForce({ -xForward * speed, 0, -zForward * speed });
				}
			}

			if (mOnGround && Input::GetKeyPressed(Input::KC_Space))
			{
				physicalBody->AddImpulseForce({ 0, 1 * deltaTime * 600, 0 });
			}
		}

		return {};
	}

	[[nodiscard]] inline YmEventReturn OnCollisionEnter(float deltaTimem, entt::registry& registry, entt::entity const& thisEntity, entt::entity const& thatEntity) override
	{
		auto* otherTransform = registry.try_get<Transform>(thatEntity);

		if (otherTransform && otherTransform->mTag == "Floor")
			mOnGround = true;

		return {};

	}

	[[nodiscard]] inline YmEventReturn OnCollisionExit(float deltaTimem, entt::registry& registry, entt::entity const& thisEntity, entt::entity const& thatEntity) override
	{
		auto* otherTransform = registry.try_get<Transform>(thatEntity);

		if (otherTransform && otherTransform->mTag == "Floor")
			mOnGround = false;
		return {};
	}
};
#endif // GAME