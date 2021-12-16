#ifdef EDITOR
#pragma once
#include "nlohmann/json.hpp"
import "EditorComponents.h";
import <vector>;
import <string>;
import <fstream>;
import <filesystem>;
import Logger;
import SceneObject;

// This exists because having the map-like access gives error : ""CL.exe" exited with code -1073741571" when in module.

/**
 * @brief File for exporting scenes.
*/
class SceneFileExporter
{
private:
	using json = nlohmann::json;
public:
	/**
	 * @brief Get the base JSON structure that the initializer needs.
	 * @param entities
	 * @return
	*/
	json GetJsonedDocument(std::vector<std::vector<json>> const& entities);

	// Will save it as a file
	// Will update config file with appropriate scene id and path

	/**
	 * @brief Exports scene as a JSON file to the destination.
	 * @param sceneDocument
	 * @param sceneToSave
	 * @param pathConfigFile
	 * @note Will update the main configuration file to hold a reference to this scene.
	*/
	void ExportScene(json const& sceneDocument, SceneObject const& sceneToSave, std::string const& pathConfigFile);
};
#endif // DEBUG
