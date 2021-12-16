#pragma once
#include "nlohmann/json.hpp"
#include "entity/registry.hpp"

import <fstream>;
import <string>;
import <string_view>;
import Logger;
import <vector>;
import <optional>;
import Component.Transform;
import Graphics;
import SceneObject;
import Component.GameLogic;

#ifdef GAME
import AudioWrapper;
import PhysicsWrapper;
import BeaconNavigation;
#endif // GAME

#ifdef EDITOR
import "EditorComponents.h";
#endif // EDITOR

/**
 * @brief Classed used for object initialization that depend on the external sources (such as configuration or scene files).
*/
class Initializer
{
private:
	using json = nlohmann::json;

	/**
	 * @brief Main configuration document. Which holds necessary data for the engine to function.
	*/
	json mConfigDocument;

#ifdef GAME
	/**
	* @brief Collection of scenes saved within the main configuration file.
	* @note Is loaded once on startup and then not modified.
	*/
	std::vector<SceneObject> mAvailableScene;
#endif // GAME

	/**
	 * @brief Current scene ID.
	*/
	int mCurrentScene;
private:

	/**
	 * @brief Add a transform component to the entity registry and apply it to the provided entity.
	 * @param registry Game scene registry.
	 * @param entity Target entity.
	 * @param transformComponentJson JSON data about the component.
	*/
	void PopulateWithTransform(entt::registry& registry, entt::entity& entity, json const& transformComponentJson);

	/**
	 * @brief Add a mesh component to the entity registry and apply it to the provided entity.
	 * @param registry Game scene registry.
	 * @param entity Target entity.
	 * @param meshComponentJson JSON data about the component.
	*/
	void PopulateWithMesh(entt::registry& registry, entt::entity& entity, json const& meshComponentJson);

#ifdef GAME
	/**
	* @brief Get scene object by the ID.
	* @param id
	* @return Returns empty if no scene was not found by the provided ID. Returns scene file if it was found.
	*/
	std::optional<SceneObject> GetSceneById(unsigned int const id);

	/**
	 * @brief Add a physics component to the entity registry and apply it to the provided entity.
	 * @param registry Game scene registry.
	 * @param entity Target entity.
	 * @param physicsComponentJson JSON data about the component.
	*/
	void PopulateWithPhysics(entt::registry& registry, entt::entity& entity, json const& physicsComponentJson);

	/**
	 * @brief Add a script manager component to the entity registry and apply it to the provided entity.
	 * @param registry Game scene registry.
	 * @param entity Target entity.
	 * @param scriptsComponentJson JSON data about the component.
	*/
	void PopulateWithScripts(entt::registry& registry, entt::entity& entity, json const& scriptsComponentJson);

	/**
	 * @brief Add an audio manager component to the entity registry and apply it to the provided entity.
	 * @param registry Game scene registry.
	 * @param entity Target entity.
	 * @param audioComponentJson JSON data about the component.
	*/
	void PopulateWithAudio(entt::registry& registry, entt::entity& entity, json const& audioComponentJson);

	/**
	 * @brief Add a camera component to the entity registry and apply it to the provided entity.
	 * @param registry Game scene registry.
	 * @param entity Target entity.
	 * @param cameraComponentJson JSON data about the component.
	*/
	void PopulateWithCamera(entt::registry& registry, entt::entity& entity, json const& cameraComponentJson);

	/**
	 * @brief Add a light component to the entity registry and apply it to the provided entity.
	 * @param registry Game scene registry.
	 * @param entity Target entity.
	 * @param lightComponentJson JSON data about the component.
	*/
	void PopulateWithLight(entt::registry& registry, entt::entity& entity, json const& lightComponentJson);

	/**
	 * @brief Add an AI navigation component to the entity registry and apply it to the provided entity.
	 * @param registry Game scene registry.
	 * @param entity Target entity.
	 * @param aiNavComponentJson JSON data about the component.
	*/
	void PopulateWithAINav(entt::registry& registry, entt::entity& entity, json const& aiNavComponentJson);
#endif // GAME

public:

	/**
	 * @brief Graphics wrapper object.
	 * @note Only intended for initializations and should not outlast the engine ownership.
	*/
	GraphicsWrapper* mGraphics;
#ifdef GAME

	/**
	 * @brief Physics wrapper object.
	 * @note Only intended for initializations and should not outlast the engine ownership.
	*/
	PhysicsWrapper* mPhysics;
#endif // DEBUG

public:
	/**
	 * @brief Load the main configuration file.
	 * @param fullPath
	*/
	void LoadConfig(std::string const& fullPath);

	/**
	 * @brief Get resolution dimensions from the main configuration file.
	 * @return [Width, Height]
	*/
	std::pair<unsigned int, unsigned int> GetResolution();

	/**
	 * @brief Load the logging files from the main configuration file.
	*/
	void LoadLoggingFiles();

	/**
	 * @brief Populate the scene registry from based on the scene file.
	 * @param registry Game scene registry.
	 * @param jScene The JSON object of the scene data.
	*/
	void PopulateRegistry(entt::registry& registry, json const& jScene);

#ifdef GAME
	/**
	 * @brief Load the scene file paths into memory so they can be loaded from.
	*/
	void LoadScenesConfigs();

	/**
	 * @brief Load scene by its ID.
	 * @param sceneId
	 * @param registry Scene registry.
	*/
	void LoadSceneById(unsigned int const sceneId, entt::registry& registry);

	/**
	 * @brief Get the ID of current scene.
	 * @return
	*/
	inline int GetCurrentScene()
	{
		return mCurrentScene;
	}

	/**
	 * @brief Get an ID of next scene.
	 * @return If there is a scene whose ID is larger than the current one, the ID will be returned, otherwise - empty object.
	*/
	std::optional<int> GetNextSceneId();
#endif // GAME

#ifdef EDITOR

	/**
	 * @brief Load a scene into the editor.
	 * @param pathToScene
	 * @param registry Edited scene entity registry.
	*/
	void LoadSceneInEditor(std::string const& pathToScene ,entt::registry& registry);

	/**
	 * @brief Load editor data from the main config file.
	 * @param editorRegistry
	*/
	void LoadEditor(entt::registry& editorRegistry);
#endif // EDITOR
};