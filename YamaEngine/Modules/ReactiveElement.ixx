#ifdef GAME
module;
#include "entity/registry.hpp";
export module GameLogic.ReactiveElement;
import GameLogic.Base;
import Component.GraphicsComponents;

// Kinda useless in terms of the game and engine, it's just there to show that animations can be played as a reaction to the player interacting with it - colliding (should fall under "user action").

/**
 * @brief A simple script that displays how a game object reacts to user action and plays an animation based on that.
*/
export class ReactiveItem : public GameScriptBase
{
private:
	/**
	 * @brief Simple state flag.
	*/
	bool mIsAnimating = false;

	/**
	 * @brief Used to prevent state changes too quickly.
	*/
	float const mTimeToChange = 1.f;

	/**
	 * @brief Current state for the state change cool-down.
	*/
	float mCurrentTime = 0.f;
public:
	[[nodiscard]] inline YmEventReturn Update(float deltaTime, entt::registry& registry, entt::entity const& thisEntity) override
	{
		mCurrentTime += deltaTime;
		return {};
	}

	[[nodiscard]] inline YmEventReturn OnCollisionEnter(float deltaTime, entt::registry& registry, entt::entity const& thisEntity, entt::entity const& thatEntity) override
	{
		auto* otherTransform = registry.try_get<Transform>(thatEntity);

		if (otherTransform && otherTransform->mTag == "Player")
		{
			if (mCurrentTime >= mTimeToChange)
			{
				mCurrentTime = 0;
				auto* meshComp = registry.try_get<MeshComp>(thisEntity);

				if (meshComp)
				{
					if (mIsAnimating)
					{
						meshComp->SetAnimation("Still");
						mIsAnimating = false;
					}

					else
					{
						mIsAnimating = true;
						meshComp->SetAnimation("Boing");
					}

				}
				else
				{
					Log(LGR_WARNING, "The reactive element couldn't access it's mesh");
				}
			}
		}

		return {};
	}
};
#endif // GAME