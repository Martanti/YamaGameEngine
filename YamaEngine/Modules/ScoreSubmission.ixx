#ifdef GAME
module;
#include "entity/registry.hpp";
export module GameLogic.ScoreSubmission;
import GameLogic.Base;
import HighScoreEntity;
import "HighscoreWrapper.h";
import DataVault;

/**
 * @brief Score submission script, which handles getting the user name and then submitting the score the top score baord.
*/
export class ScoreSubmission : public GameScriptBase
{
private:
	/**
	 * @brief Score received from the last game scene.
	 * @note WEB API will not accept entries that are <=0.
	*/
	int mScore = -1;

	/**
	 * @brief User picked name.
	*/
	std::string name;
public:

	[[nodiscard]] inline YmEventReturn Start(float deltaTime, entt::registry& registry, entt::entity const& thisEntity) override
	{
		YmEventList events;
		auto value = DataVault.find("FinalSceore");

		if (value != DataVault.end())
		{
			auto[_, value] = *value;

			mScore = std::any_cast<int>(value);
		}
		else
		{
			Log(LGR_INFO, "Score data was not stored prior to the submission scene call");
			events.emplace_back(YmEventTypes::ChangeScene, new SceneChangeMessage{ 1 });
		}

		events.emplace_back(YmEventTypes::CursorState, new int(0));
		return events;
	}

	[[nodiscard]] inline YmEventReturn DrawItems() override
	{

		bool upload = false;
		auto events =  GUI::Instance().ScoreSubmission(mScore, name, upload);
		if (events.has_value())
		{
			if (upload)
			{
				HighScoreEntity score{ .mScore = mScore, .mName = name };
				HighscoreWrapper::UploadHighscore(score);
			}

			return events;
		}

		return {};
	}
};
#endif