#ifdef GAME
module;
#include "entity/registry.hpp";
export module GameLogic.ScoreTracker;
import GameLogic.Base;
import DataVault;
import <chrono>;
import AudioWrapper;

/**
 * @brief A script meant to track user score and then pass it to the next scene.
*/
export class PlayerScoreTracker : public GameScriptBase
{
private:
	typedef std::chrono::steady_clock Timer;
	/**
	 * @brief Tracking the main goal to see if the win condition is met.
	*/
	int mCollectedMainScore = 0;

	/**
	 * @brief Bonus score tracker.
	*/
	int mCollectedExtraScore = 0;

	/**
	 * @brief Score from the previous scene if there was some.
	*/
	int mPreviousScore = 0;

	/**
	 * @brief Goal for achieving the winning condition.
	*/
	int mMaxMainScore = 0;

	/**
	 * @brief A timer as delta time passed in the Update is fixed and is not really representative of the passed time.
	*/
	Timer::time_point mApplicationStart;
private:

	/**
	 * @brief Calculate passed time since the start of the start of the script.
	 * @return
	*/
	inline float ElapsedTimeInSeconds()
	{
		auto currentTime = Timer::now();
		auto spentTime = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - mApplicationStart).count();
		return spentTime * (float)1e-9;
	}

	/**
	 * @brief Calculating the final score of the scene.
	 * @return
	*/
	inline int TotalScoreCalculation()
	{
		float lastFrameInSeconds = ElapsedTimeInSeconds();
		int score = 0;
		score += (int)((1.f / lastFrameInSeconds) * 100);
		score += mCollectedExtraScore * 35;
		score += mCollectedMainScore * 19;
		return score + mPreviousScore;
	}
public:

	[[nodiscard]] inline YmEventReturn Start(float deltaTime, entt::registry& registry, entt::entity const& thisEntity) override
	{
		// Set the cap.
		for (auto&& [entity, transform] : registry.view<Transform>().each())
		{
			if (transform.mTag == "Main point")
				mMaxMainScore++;
		}

		if (mMaxMainScore == 0)
		{
			Log(LGR_ERROR, "The game cannot continue as the scene does not contain any items with the \"Main point\" tag, meaning win condition is already met.");
		}

		mApplicationStart = Timer::now();

		auto scoreFromPreviousScene = DataVault["FinalSceore"];

		if (scoreFromPreviousScene.has_value())
		{
			mPreviousScore = std::any_cast<int>(scoreFromPreviousScene);
		}

		YmEventList events;
		events.emplace_back(YmEventTypes::CursorState, new int(CursorStates::CursorInvisible));
		return events;
	}

	[[nodiscard]] inline YmEventReturn Update(float deltaTime, entt::registry& registry, entt::entity const& thisEntity) override
	{
		if (mCollectedMainScore == mMaxMainScore)
		{


			DataVault["FinalSceore"] = TotalScoreCalculation();
			YmEventList sceneChange;
			sceneChange.emplace_back(YmEventTypes::ChangeScene, new SceneChangeMessage{ std::make_pair(-1, true) });

			return sceneChange;
		}
		return {};
	}

	[[nodiscard]] inline YmEventReturn OnCollisionEnter(float deltaTimem, entt::registry& registry, entt::entity const& thisEntity, entt::entity const& thatEntity) override
	{
		auto* otherTransform = registry.try_get<Transform>(thatEntity);

		auto otherTag = otherTransform->mTag;
		YmEventList events;

		if (!otherTransform)
			return {};

		if (otherTag == "Main point")
		{
			mCollectedMainScore++;

			auto* soundComp = registry.try_get<Audio::AudioComponent>(thisEntity);

			if (soundComp)
			{
				auto source = soundComp->audioStorage["Ping"];
				source.PlaySource();
			}

			events.emplace_back(YmEventTypes::DeleteEntity, new int((int)thatEntity));
		}

		else if (otherTag == "Extra point")
		{
			mCollectedExtraScore++;
			events.emplace_back(YmEventTypes::DeleteEntity, new int((int)thatEntity));
		}

		else if (otherTag == "Death")
		{
			DataVault["FinalSceore"] = TotalScoreCalculation();
			events.emplace_back(YmEventTypes::ChangeScene, new SceneChangeMessage{-1});
		}

		if (events.size() > 0)
			return events;

		return {};
	}

	[[nodiscard]] inline virtual YmEventReturn DrawItems() override
	{
		auto timeSpanSeconds = ElapsedTimeInSeconds();

		GUI::Instance().PlayerCurrentScore(timeSpanSeconds, mCollectedMainScore, mCollectedExtraScore, mMaxMainScore);
		return {};
	}
};
#endif // GAME
