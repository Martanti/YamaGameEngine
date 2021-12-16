export module SceneObject;
import <string>;

/**
 * @brief The scene structure.
*/
export struct SceneObject
{
	/**
	 * @brief Path to the JSON file containing the data about the scene.
	*/
	std::string mConfigPath;

	/**
	 * @brief Scene name.
	*/
	std::string mSceneName;

	/**
	 * @brief ID of the scene, which also determines the order in which the scenes are called, if using the "next scene" type scene change.
	*/
	int mId;
};