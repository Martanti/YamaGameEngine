#include "Initializer.h"

// Using Try-Catch as the json library heavily relies on that.

void Initializer::LoadConfig(std::string const& fullPath)
{
	std::ifstream configFile(fullPath.data());
	Assert(!configFile.fail(), std::format("Config file from path \"{}\" was not loaded successfully!", fullPath));
	try
	{
		mConfigDocument = json::parse(configFile);
	}
	catch (std::exception const& e)
	{
		Log(LGR_ERROR, std::format("Exception occurred when parsing the json in config file at \"{}\". Exception: {}", fullPath, e.what()));
	}

	configFile.close();
}

std::pair<unsigned int, unsigned int> Initializer::GetResolution()
{
	unsigned int width = 640u, height = 480u;
	try
	{
		auto settings = mConfigDocument["Settings"];
		width = settings["ScreenWidth"].get<unsigned int>();
		height = settings["ScreenHeight"].get<unsigned int>();
	}
	catch (std::exception const& e)
	{
		Log(LGR_ERROR, std::format("Exception occurred while parsing json for Resolution: {}", e.what()));
		Log(LGR_WARNING, std::format("Due to error, resolution will be returned as {}x{}", height, width));
	}
	return { width  , height };
}

void Initializer::LoadLoggingFiles()
{
	auto fileLogs = mConfigDocument["LoggingFiles"];

	for (auto logFile : fileLogs)
	{
		std::string path, severityText;
		FileWriteMode overwriteMode;

		try
		{
			// If path doesn't exist the logger creates it
			path = logFile["Path"].get<std::string>();
			overwriteMode = logFile["Overwrite"].get<bool>() ? FileWriteMode::Overwrite : FileWriteMode::Append;
			severityText = logFile["Severity"].get<std::string>();
		}
		catch (std::exception const& e)
		{
			Log(LGR_ERROR, std::format("Exception occurred while parsing json for log files: {}", e.what()));
			continue;
		}

		Verbosity severity;
		if (severityText == "INFO")
			severity = LGR_INFO;

		else if (severityText == "ERROR")
			severity = LGR_ERROR;

		else if (severityText == "WARNING")
			severity = LGR_WARNING;

		else
		{
			Log(LGR_ERROR, "The severity could not have been determined! Skipping this file.");
			continue;
		}

		try
		{
			AddLogFile(path, overwriteMode, severity);
		}
		catch (std::exception const& e)
		{
			Log(LGR_ERROR, std::format("Exception occurred while creating files for log files: {}", e.what()));
		}
	}
}

#ifdef GAME
void Initializer::LoadScenesConfigs()
{
	auto scenes = mConfigDocument["Scenes"];

	for (auto& item : scenes)
	{
		try
		{
			SceneObject newScene(item["Path"].get<std::string>(), item["Name"].get<std::string>(), item["Id"].get<unsigned int>());
			mAvailableScene.emplace_back(newScene);
		}
		catch (std::exception const& e)
		{
			Log(LGR_ERROR, std::format("Exception occurred while getting the scene data: {}", e.what()));
		}
	}
}

std::optional<SceneObject> Initializer::GetSceneById(unsigned int const id)
{
	std::optional<SceneObject> selectedScene = {};

	for (auto item : mAvailableScene)
		if (item.mId == id)
			selectedScene = item;

	return selectedScene;
}

void Initializer::LoadSceneById(unsigned int const sceneId, entt::registry& registry)
{
	// Find the Scene by ID
	Log(LGR_INFO, std::format("Starting to load scene with an ID {}", sceneId));
	auto selectedSceneResult = GetSceneById(sceneId);

	if (!selectedSceneResult.has_value())
	{
		Log(LGR_ERROR, std::format("Could not load scene with id {}!", sceneId));
		return;
	}

	auto scene = selectedSceneResult.value();

	// Read the file into json

	std::ifstream sceneFile(scene.mConfigPath);
	if (sceneFile.fail())
	{
		Log(LGR_ERROR, std::format("Could not load the scene file at \"{}\"", scene.mConfigPath));
		return;
	}

	json sceneJson;
	try
	{
		sceneJson = json::parse(sceneFile);
	}
	catch (std::exception const& e)
	{
		Log(LGR_ERROR, std::format("Exception occurred when parsing the json in scene file at \"{}\". Exception: {}", scene.mConfigPath, e.what()));
	}

	PopulateRegistry(registry, sceneJson);
	mCurrentScene = sceneId;
	Log(LGR_INFO, std::format("Loaded successfully scene with an ID {}", sceneId));
}
#endif // GAME

void Initializer::PopulateRegistry(entt::registry& registry, json const& jScene)
{

	// Prepare the registry.
	registry.clear();

	// Ideally this would be directly parsed into an object :/
	for (auto entityJson : jScene["Entities"])
	{
		auto entity = registry.create();

		for (auto componentJson : entityJson["Components"])
		{
			auto type = componentJson["Type"].get<std::string>();

			// TODO:? make this into a list that iterates through a pair of string and corresponding function that would handle init

			// A hashed string approach for a switch might be worth it
			if (type == "Transform")
			{
				PopulateWithTransform(registry, entity, componentJson);
#ifdef EDITOR
				auto& editorTransform = registry.emplace<TransformEditorComponent>(entity);
				editorTransform.PopulateFromJson(componentJson);
#endif // EDITOR
			}

			else if (type == "Mesh")
			{
				PopulateWithMesh(registry, entity, componentJson);
#ifdef EDITOR
				auto& editorMesh = registry.emplace<GraphicsEditorComponent>(entity);
				editorMesh.PopulateFromJson(componentJson);
#endif // EDITOR
			}

			else if (type == "Physics")
			{
#ifdef GAME
				PopulateWithPhysics(registry, entity, componentJson);
#endif // GAME

#ifdef EDITOR
				auto& editorPhysics = registry.emplace<PhysicsEditorComponent>(entity);
				editorPhysics.PopulateFromJson(componentJson);
#endif // EDITOR
			}

			else if (type == "Script")
			{
#ifdef GAME
				PopulateWithScripts(registry, entity, componentJson);
#endif

#ifdef EDITOR
				auto& editorScripts = registry.emplace<ScriptsEditorComponent>(entity);
				editorScripts.PopulateFromJson(componentJson);
#endif // EDITOR
			}

			else if (type == "Audio")
			{
#ifdef GAME
				PopulateWithAudio(registry, entity, componentJson);
#endif // GAME

#ifdef EDITOR
				auto& editorAudio = registry.emplace<AudioEditorComponent>(entity);
				editorAudio.PopulateFromJson(componentJson);
#endif // EDITOR
			}

			else if (type == "Camera")
			{
#ifdef GAME
				PopulateWithCamera(registry, entity, componentJson);
#endif // GAME

#ifdef EDITOR
				auto& editorCamera = registry.emplace<CameraEditorComponent>(entity);
				editorCamera.PopulateFromJson(componentJson);
#endif // EDITOR
			}

			else if (type == "Light")
			{
#ifdef GAME
				PopulateWithLight(registry, entity, componentJson);
#endif // GAME

#ifdef EDITOR
				auto& editorLight = registry.emplace<LightEditorComponent>(entity);
				editorLight.PopulateFromJson(componentJson);
#endif // EDITOR
			}

			else if (type == "AINavigation")
			{
#ifdef GAME
				PopulateWithAINav(registry, entity, componentJson);
#endif // GAME

#ifdef EDITOR
				auto& editorNav = registry.emplace<BeaconNavEditorComponent>(entity);
				editorNav.PopulateFromJson(componentJson);
#endif // EDITOR
			}

			else
			{
				Log(LGR_ERROR, std::format("Could not assign appropriate Component interpretor for type \"{}\"", type));
			}
		}
	}
}

void Initializer::PopulateWithTransform(entt::registry& registry, entt::entity& entity, json const& transformComponentJson)
{
	Transform newTransform;

#pragma region Position
	{
		auto positionJson = transformComponentJson["Position"];
		float x = 0, y = 0, z = 0;

		if (positionJson.is_array())
		{
			try
			{
				x = positionJson.at(0).get<float>();
				y = positionJson.at(1).get<float>();
				z = positionJson.at(2).get<float>();
			}
			catch (std::exception const& e)
			{
				Log(LGR_ERROR, std::format("Exception occurred while getting the transform position: {}", e.what()));
			}
		}
		else
		{
			Log(LGR_ERROR, "The position variable was not an array therefore it was skipped and the default was used");
		}
		auto& pos = newTransform.mPosition;
		pos.X = x;
		pos.Y = y;
		pos.Z = z;
	}
#pragma endregion

#pragma region Rotation
	{
		auto rotationJson = transformComponentJson["Rotation"];
		float x = 0, y = 0, z = 0;
		if (rotationJson.is_array())
		{
			try
			{
				x= rotationJson.at(0).get<float>();
				y= rotationJson.at(1).get<float>();
				z= rotationJson.at(2).get<float>();
			}
			catch (std::exception const& e)
			{
				Log(LGR_ERROR, std::format("Exception occurred while getting the transform rotation: {}", e.what()));
			}
		}
		else
		{
			Log(LGR_ERROR, "The rotation variable was not an array therefore it was skipped and the default was used");
		}
		auto& rot = newTransform.mRotation;
		rot.X = x;
		rot.Y = y;
		rot.Z = z;
	}
#pragma endregion

#pragma region Scale
	{
		auto scaleJson = transformComponentJson["Scale"];
		float x = 0, y = 0, z = 0;
		if (scaleJson.is_array())
		{
			try
			{
				x= scaleJson.at(0).get<float>();
				y= scaleJson.at(1).get<float>();
				z= scaleJson.at(2).get<float>();
			}
			catch (std::exception const& e)
			{
				Log(LGR_ERROR, std::format("Exception occurred while getting the transform rotation: {}", e.what()));
			}
		}
		else
		{
			Log(LGR_ERROR, "The rotation variable was not an array therefore it was skipped and the default was used");
		}

		auto& scale = newTransform.mScale;
		scale.X = x;
		scale.Y = y;
		scale.Z = z;
	}
#pragma endregion

	try
	{
		newTransform.mName = transformComponentJson["Name"].get<std::string>();
		newTransform.mTag = transformComponentJson["Tag"].get<std::string>();;
	}
	catch (std::exception const& e)
	{
		Log(LGR_ERROR, std::format("Issues while trying to retrieve a name and or the tag from scene file {}", e.what()));
		newTransform.mName = "Game object";
		newTransform.mTag = "";
	}

	// Either this, or create it at the beginning, take returned reference and populate it that way
	registry.emplace<Transform>(entity, std::move(newTransform));
}

void Initializer::PopulateWithMesh(entt::registry& registry, entt::entity& entity, json const& meshComponentJson)
{
	std::string pathToMesh;
	try
	{
		pathToMesh = meshComponentJson["Path"].get<std::string>();
	}
	catch (std::exception const& e)
	{
		Log(LGR_ERROR, std::format("Exception occurred while getting the mesh path from q scene file: {}", e.what()));
		Log(LGR_WARNING, "Mesh component initialization will be skipped due to failure");
		return;
	}

	// Setting this as separate as severity is difference
	std::string texturePath = "";
	try
	{
		texturePath = meshComponentJson["TexturePath"].get<std::string>();
	}
	catch (std::exception const& e)
	{
		Log(LGR_ERROR, std::format("Exception occurred while getting the texture data from a scene file: {}", e.what()));
	}

	auto mesh = mGraphics->AddMesh(pathToMesh, texturePath);

	if (mesh.has_value())
	{

#ifdef GAME
		try
		{
			auto animationData = meshComponentJson["Animations"];

			if(!animationData.is_null())
				for (auto& item : animationData)
				{
					auto name = item["Name"].get<std::string>();
					auto speed = item["Speed"].get<float>();
					auto frameStart = item["StartFrame"].get<int>();
					auto frameEnd = item["EndFrame"].get<int>();

					mesh->Animations[name] = { frameStart,frameEnd, speed };
					mesh->SetAnimation(name);
					mesh->SetCurrentAnimationSpeed(speed);
				}
		}
		catch (std::exception const& e)
		{
			Log(LGR_ERROR, std::format("Exception occurred while getting the animation data from a scene file: {}", e.what()));
		}
#endif // GAME

#ifdef EDITOR
		mesh->SetCurrentAnimationSpeed(0);
#endif // EDITOR


		registry.emplace<MeshComp>(entity, std::move(mesh.value()));
	}
}

#ifdef GAME
void Initializer::PopulateWithPhysics(entt::registry& registry, entt::entity& entity, json const& physicsComponentJson)
{
	std::string colliderType;
	float mass, offsetX, offsetY, offsetZ;

	try
	{
		colliderType = physicsComponentJson["ColliderType"].get<std::string>();
		mass = physicsComponentJson["Mass"].get<float>();
		auto& offsets = physicsComponentJson["Offset"];
		offsetX = offsets.at(0).get<float>();
		offsetY = offsets.at(1).get<float>();
		offsetZ = offsets.at(2).get<float>();

	}
	catch (std::exception const& e)
	{
		Log(LGR_ERROR, std::format("Exception occurred while getting the Collider data from scene file: {}", e.what()));
		return;
	}

	if (colliderType == "Box")
	{
		float extentX, extentY, extentZ;

		try
		{
			auto& extents = physicsComponentJson["Extents"];
			extentX = extents.at(0).get<float>();
			extentY = extents.at(1).get<float>();
			extentZ = extents.at(2).get<float>();
		}
		catch (std::exception const& e)
		{
			Log(LGR_ERROR, std::format("Exception occurred while getting the Collider extents from scene file: {}", e.what()));
		}

		auto physicsComp = mPhysics->CreatePhysicsCube({ extentX, extentY, extentZ }, mass, { offsetX, offsetY, offsetZ }, (int)entity);
		registry.emplace<PhysicalBody>(entity, std::move(physicsComp));
	}

	else
	{
		Log(LGR_ERROR, std::format("Could not determine the collider type: {}", colliderType));
	}
}

void Initializer::PopulateWithScripts(entt::registry& registry, entt::entity& entity, json const& scriptComponentJson)
{
	auto& scripts = scriptComponentJson["ScriptContainer"];
	auto& component = registry.emplace<GameLogic>(entity);
	for (auto& item : scripts)
	{
		try
		{
			auto type = item.get<std::string>();
			component.AddScript(type);
		}
		catch (std::exception const& e)
		{
			Log(LGR_ERROR, std::format("Could not add script from the scene file: {}", e.what()));
		}
	}
}

void Initializer::PopulateWithAudio(entt::registry& registry, entt::entity& entity, json const& scriptComponentJson)
{
	auto sources = scriptComponentJson["Sources"];
	auto& component = registry.emplace<Audio::AudioComponent>(entity);
	for (auto& item : sources)
	{
		try
		{
			auto name = item["Name"].get<std::string>();
			auto path = item["Path"].get<std::string>();
			Audio::AudioSource src;
			src.SetSoundPath(path);
			component.audioStorage[name] = src;
		}

		catch (std::exception const& e)
		{
			Log(LGR_ERROR, std::format("Could not add Audio from the scene file: {}", e.what()));
		}
	}
}

void Initializer::PopulateWithCamera(entt::registry& registry, entt::entity& entity, json const& cameraComponentJson)
{
	try
	{
		auto type = cameraComponentJson["CamType"].get<std::string>();
		float xOffset = 0, yOffset = 0, zOffset = 0;
		auto& offset = cameraComponentJson["Offset"];

		xOffset = offset.at(0).get<float>();
		yOffset = offset.at(1).get<float>();
		zOffset = offset.at(2).get<float>();

		// As it's only one camera now - this shoul do

		if (type == "FPS")
		{
			auto cam = mGraphics->AddFPSCamera();
			cam.mCamType = type;
			cam.mOffset = { xOffset , yOffset , zOffset };
			registry.emplace<CameraComponent>(entity, std::move(cam));
		}

		else
		{
			Log(LGR_ERROR, std::format("Could not recognize the camera type: \"{}\"", type));
		}

	}
	catch (std::exception const& e)
	{
		Log(LGR_ERROR, std::format("Could not add camera from the scene file: {}", e.what()));
	}
}

void Initializer::PopulateWithLight(entt::registry& registry, entt::entity& entity, json const& lightComponentJson)
{
	try
	{
		auto lights = mGraphics->AddLight();
		auto type = lightComponentJson["LightType"].get<std::string>();
		lights.SetType(LightComponent::TypeNameToType(type));

		auto range = lightComponentJson["Range"].get<float>();
		lights.SetRange(range);
		{
			auto diffuse = lightComponentJson["DiffuseColour"];
			auto diffuseR = diffuse.at(0).get<float>();
			auto diffuseG = diffuse.at(1).get<float>();
			auto diffuseB = diffuse.at(2).get<float>();
			auto diffuseA = diffuse.at(3).get<float>();
			lights.SetDiffuse(diffuseR, diffuseG, diffuseB, diffuseA);
		}

		{
			auto ambient = lightComponentJson["AmbientColour"];
			auto ambientR = ambient.at(0).get<float>();
			auto ambientG = ambient.at(1).get<float>();
			auto ambientB = ambient.at(2).get<float>();
			auto ambientA = ambient.at(3).get<float>();
			lights.SetAmbient(ambientR, ambientG, ambientB, ambientA);
		}

		registry.emplace<LightComponent>(entity, std::move(lights));
	}
	catch (std::exception const& e)
	{
		Log(LGR_ERROR, std::format("Could not add light from the scene file: {}", e.what()));
	}
}

void Initializer::PopulateWithAINav(entt::registry& registry, entt::entity& entity, json const& aiNavComponentJson)
{
	try
	{
		auto& listOfPoints = aiNavComponentJson["Points"];

		std::vector<yamath::Vec3D> beacons;
		for (auto& item : listOfPoints)
		{
			auto x = item.at(0).get<float>();
			auto y = item.at(1).get<float>();
			auto z = item.at(2).get<float>();
			beacons.emplace_back(x, y, z);
		}

		registry.emplace<BeaconNavigation>(entity, beacons);
	}
	catch (std::exception const& e)
	{
		Log(LGR_ERROR, std::format("Could not add AI beacon navigation fully from the scene file: {}", e.what()));
	}
}


std::optional<int> Initializer::GetNextSceneId()
{
	auto const current = GetCurrentScene();
	auto nextScene = INT_MAX;

	for (auto& item : mAvailableScene)
	{
		auto& currID = item.mId;
		if (currID > current && currID < nextScene)
		{
			nextScene = currID;
		}
	}

	if (nextScene == INT_MAX)
		return {};

	return nextScene;
}
#endif // GAME

#ifdef EDITOR

void Initializer::LoadSceneInEditor(std::string const& pathToScene, entt::registry& registry)
{
	std::ifstream sceneFile(pathToScene.data());

	if (sceneFile.fail())
	{
		Log(LGR_ERROR, std::format("Failed to load scene file at \"{}\"", pathToScene));
	}
	else
	{
		try
		{
			json jScene = json::parse(sceneFile);
			PopulateRegistry(registry, jScene);
		}
		catch (std::exception const& e)
		{
			Log(LGR_ERROR, std::format("Exception occurred while loading the scene: {}", e.what()));
		}
	}

	sceneFile.close();
}

void Initializer::LoadEditor(entt::registry& editorRegistry)
{
	auto editorData = mConfigDocument["Editor"];

	try
	{
		auto entity = editorRegistry.create();
		auto& transform = editorRegistry.emplace<Transform>(entity);
		transform.mScale = yamath::Vec3D{ .X = 1, .Y = 1, .Z = 1 };

		auto cameraPositionArr = editorData["CameraPosition"];
		transform.mPosition.X = cameraPositionArr.at(0).get<float>();
		transform.mPosition.Y = cameraPositionArr.at(1).get<float>();
		transform.mPosition.Z = cameraPositionArr.at(2).get<float>();

		auto camera = mGraphics->AddCamera();
		camera.mOffset = { 0, 0, 0 }; // No need to store in config file as the offset is useless in this case.

		editorRegistry.emplace<CameraComponent>(entity, std::move(camera));
		auto& scriptManager = editorRegistry.emplace<GameLogic>(entity);
		for (auto& scriptNameRaw : editorData["EditorScripts"])
		{
			auto scriptName = scriptNameRaw.get<std::string>();
			scriptManager.AddScript(scriptName);
		}
	}
	catch (std::exception const& e)
	{
		Log(LGR_ERROR, std::format("Errors while reading editor data from the config file: {}", e.what()));
		Log(LGR_WARNING, "Some default data might be present.");
	}
}

#endif // EDITOR