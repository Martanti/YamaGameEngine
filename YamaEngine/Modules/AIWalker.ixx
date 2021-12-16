#ifdef GAME
module;
#include "entity/registry.hpp";
export module GameLogic.AIWalker;
import GameLogic.Base;
import YaMath;
import BeaconNavigation;

/**
 * @brief Script for handling AI walking from one point to another.
*/
export class AIWalker : public GameScriptBase
{
private:
	/**
	 * @brief Current target point.
	*/
	yamath::Vec3D mCurrentTarget;
private:
public:
public:
	[[nodiscard]] inline YmEventReturn Start(float deltaTime, entt::registry& registry, entt::entity const& thisEntity) override
	{
		auto* navigation = registry.try_get<BeaconNavigation>(thisEntity);
		if (navigation)
		{
			mCurrentTarget = navigation->GetCurrentPoint();
		}
		else
		{
			Log(LGR_WARNING, "The beacon navigation component was not found on an entity that is trying to use AI Walker script.");
		}
		return {};
	}


	[[nodiscard]] inline YmEventReturn Update(float deltaTime, entt::registry& registry, entt::entity const& thisEntity) override
	{
		auto& transform = registry.get<Transform>(thisEntity);

		auto distance = yamath::opr::Sub(mCurrentTarget, transform.mPosition);

		if (yamath::opr::Length(distance) < 1.f)
		{
			auto* navigation = registry.try_get<BeaconNavigation>(thisEntity);
			if (navigation)
			{
				navigation->MoveToNextPoint();
				mCurrentTarget = navigation->GetCurrentPoint();
			}
			else
			{
				Log(LGR_WARNING, "The beacon navigation component was not found on an entity that is trying to use AI Walker script.");
			}
		}
		else
		{
			auto dir = yamath::opr::Normalize(distance);
			transform.mPosition = yamath::opr::Add(transform.mPosition, yamath::opr::Mult(dir, 3 * deltaTime));
		}
		return {};
	}
};
#endif // GAME