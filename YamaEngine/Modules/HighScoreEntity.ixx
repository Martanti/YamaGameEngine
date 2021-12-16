#ifdef GAME
export module HighScoreEntity;
import <string>;

/**
 * @brief Single entity for storing the data about the top score.
*/
export struct HighScoreEntity
{
	/**
	 * @brief Score.
	*/
	int mScore;

	/**
	 * @brief The name of the user.
	*/
	std::string mName;

	/**
	 * @brief Data when the score was uploaded.
	 * @note Keyword "uploaded", the data is automatically added within the WEB API.
	*/
	std::string mDate;
};
#endif // GAME
