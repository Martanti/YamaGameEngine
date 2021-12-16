#ifdef GAME
#pragma once

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include "nlohmann/json.hpp"
#include <sstream>;
#include <iostream>;
#include <vector>;
#include <string>;
import HighScoreEntity;
import Logger;


// Tried using cpr, but requests from azure were causing assert failures when destroying string for URL inside some thread locked code. Does not triggered if stepping through it.

// This also requires including over importing

// Using debug library for curl will result in an breakpoint issue. Unsure why, but again on the destruction of stuff. (Could it be related to the issue with CPR?)

/**
 * @brief A wrapper for the class that connects to the WEB API to get or post score.
*/
class HighscoreWrapper
{
private:
	using json = nlohmann::json;

	/**
	 * @brief A URL to the WEB API that can return or accept the High-score data.
	 * @note Not using the online Azure web app, in order to prevent possible spams or anything else that would be nasty.
	*/
	static inline const char mWebApiUrl[] = "https://localhost:44335/api/highscoresapi";
public:
	/**
	 * @brief Get top 5 score.
	 * @return Top 5 Highscore entity items ordered in descending order.
	*/
	static std::vector<HighScoreEntity> GetTopScore();

	/**
	 * @brief Upload the score to the database.
	 * @param score
	*/
	static void UploadHighscore(HighScoreEntity const& score);
};
#endif // GAME