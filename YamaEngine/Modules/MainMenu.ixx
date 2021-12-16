#ifdef GAME
module;
#include "entity/registry.hpp";
export module GameLogic.MainMenu;
import GameLogic.Base;
import HighScoreEntity;
import "HighscoreWrapper.h";
import <future>;
import <thread>;

export class MainMenu : public GameScriptBase
{
private:
	/**
	 * @brief A list of current high-scores.
	*/
	std::vector<HighScoreEntity> mHighscore;

	/**
	 * @brief A flag to see if the WEB API call has finished.
	*/
	bool mHasFinishedConnecting = false;

	//std::thread scoreThread;
	//std::future<void> handle;
public:
	void SetTopScore()
	{
		// There were several cases where the GetTopScore would trigger an illegal memory access violation "_Pnext was 0xFFFF..", no idea why, so to catch them this was followed:
		// https://stackoverflow.com/questions/457577/catching-access-violation-exceptions
		try
		{
			mHighscore = HighscoreWrapper::GetTopScore();
			mHasFinishedConnecting = true;
		}
		catch (std::exception const& e)
		{
			Log(LGR_ERROR, std::format("Exception occurred within the get top score call: {}", e.what()));
		}
	}

	[[nodiscard]] inline YmEventReturn Start(float deltaTime, entt::registry& registry, entt::entity const& thisEntity) override
	{
		// Disabled async call as for some reason it would get the issue mentioned in the SetTopScore() even with the error catching. Occurs in _CONSTEXPR20_CONTAINER void _Container_base12::_Orphan_all_unlocked_v3() noexcept
		//handle = std::async(&MainMenu::SetTopScore, this);

		// Thread also threw an error at _CONSTEXPR20_CONTAINER void _Container_base12::_Orphan_all_unlocked_v3()
		//scoreThread = std::thread(&MainMenu::SetTopScore, this);

		// There was something about using /MD for linker flags, but this project can't use it as it linked against mixed - static and dynamic libraries.

		SetTopScore();
		YmEventList events;
		events.emplace_back(YmEventTypes::CursorState, new int (0));

		return events;
	}

	[[nodiscard]] inline YmEventReturn DrawItems() override
	{
		GUI::Instance().DisplayHighScores(mHighscore, mHasFinishedConnecting);
		return GUI::Instance().TitleScreenText();
	}

	~MainMenu()
	{
		//scoreThread.join();
	}
};
#endif // GAME