#include "SceneFileExporter.h"
#ifdef EDITOR
namespace fs = std::filesystem;
nlohmann::json SceneFileExporter::GetJsonedDocument(std::vector<std::vector<json>> const& entities)
{
	json document;
	json jsoneEdntities;

	for (auto& ent : entities)
	{
		json componentList;

		for (auto& comp : ent)
		{
			componentList.emplace_back(comp);
		}

		json components;
		components["Components"] = componentList;
		jsoneEdntities.emplace_back(components);
	}

	document["Entities"] = jsoneEdntities;

	return document;
}

void SceneFileExporter::ExportScene(json const& sceneDocument, SceneObject const& sceneObject, std::string const& pathConfigFile)
{
	std::ifstream configFile(pathConfigFile.c_str());
	json configDocument;
	if (configFile.fail())
	{
		Log(LGR_ERROR, std::format("Config file from path \"{}\" was not loaded successfully!", pathConfigFile));
		return;
	}

	try
	{
		configDocument = json::parse(configFile);
	}
	catch (std::exception const& e)
	{
		Log(LGR_ERROR, std::format("Exception occurred when parsing the json in config file at \"{}\". Exception: {}", pathConfigFile, e.what()));
	}

	configFile.close();

	auto& scenes = configDocument["Scenes"];
	bool overwritten = false;
	for (auto& item : scenes)
	{
		if (item["Id"].get<unsigned int>() == sceneObject.mId)
		{
			item["Path"] = sceneObject.mConfigPath;
			item["Name"] = sceneObject.mSceneName;
			overwritten = true;
			break;
		}
	}

	if (!overwritten)
	{
		json jsonedScene;

		jsonedScene["Id"] = sceneObject.mId;
		jsonedScene["Path"] = sceneObject.mConfigPath;
		jsonedScene["Name"] = sceneObject.mSceneName;
		scenes.emplace_back(jsonedScene);
	}

	//fs::path configPath = pathConfigFile; // Path should already exist as it was read
	auto stringConfig = configDocument.dump(4);
	std::ofstream outConfig(pathConfigFile.c_str());
	outConfig << stringConfig.c_str();
	outConfig.close();

	fs::path scenePath = sceneObject.mConfigPath;
	if (scenePath.has_filename() && fs::create_directory(scenePath.remove_filename()))
	{
		Log(LGR_INFO, std::format("Created dir for : {}", scenePath.string()));
	}

	auto stringDocument = sceneDocument.dump(4);
	std::ofstream outScene(sceneObject.mConfigPath.c_str());
	outScene << stringDocument.c_str();
	outScene.close();
}
#endif // EDITOR