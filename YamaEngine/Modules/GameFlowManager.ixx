#ifdef GAME
module;
#include "entity/registry.hpp";
export module GameLogic.GameFlow;
import GameLogic.Base;

/**
 * @brief Used for changing scenes or exiting the game.
*/
export class  GameFlowManager : public GameScriptBase
{
private:
	/**
	 * @brief Cool-down limit.
	 * @note Without it the button would be registered as pressed multiple times, which would cause the scene to be loaded more than once.
	*/
	float const timeBeforeNextSceneUpdate = 0.8f;

	/**
	 * @brief Current time for the cool-down.
	*/
	float currentTime = 0;
public:
	[[nodiscard]] inline YmEventReturn Update(float deltaTime, entt::registry& registry, entt::entity const& thisEntity) override
	{
		YmEventList inputEvents;
		if (Input::GetKeyPressed(Input::KC_ESC))
		{
			inputEvents.emplace_back(YmEventTypes::ExitApplication, nullptr);
		}

		else
		{
			if (currentTime > timeBeforeNextSceneUpdate)
			{
				if (Input::GetKeyPressed(Input::KC_F1))
				{
					inputEvents.emplace_back(YmEventTypes::ChangeScene, new SceneChangeMessage{ 1 });
					currentTime = 0.f;
				}

				else if (Input::GetKeyPressed(Input::KC_F2))
				{
					inputEvents.emplace_back(YmEventTypes::ChangeScene, new SceneChangeMessage{ 2 });
					currentTime = 0.f;
				}
			}
		}

		currentTime += deltaTime;
		if (inputEvents.size() > 0)
			return inputEvents;
		return {};
	}
};
#endif // GAME
