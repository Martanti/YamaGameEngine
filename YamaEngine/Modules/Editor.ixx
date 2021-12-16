#ifdef EDITOR
module;
#include "entity/registry.hpp"
export module Editor;
import "EditorComponents.h";
import Logger;
import GUIWrapper;
import <memory>;
import <vector>;
import <filesystem>;
import Component.Transform;
import Component.GraphicsComponents;
import "SceneFileExporter.h";

using std::vector;
using json = nlohmann::json;
namespace fs = std::filesystem;

/**
 * @brief Editor helper functionality.
*/
export class Editor
{
private:
	/**
	 * @brief Helper for exporting the saved scene files.
	*/
	SceneFileExporter mExporter;
public:

	/**
	 * @brief Populate the transform component from its editor counterpart.
	 * @param transform
	 * @param transformEditorComponent
	*/
	void PopulateTransform(Transform& transform, TransformEditorComponent& transformEditorComponent)
	{
		auto& values = transformEditorComponent;
		transform.mPosition = values.mPos;

		transform.mRotation = values.mRot;

		transform.mScale = values.mScale;
		transform.mName = values.mName;
		transform.mTag = values.mTag;
	}

	/**
	 * @brief Populate mesh component with the data from its editor counterpart.
	 * @param MeshComp
	 * @param graphicsEditorComponent
	*/
	void PopulateMeshComp(MeshComp& MeshComp, GraphicsEditorComponent& graphicsEditorComponent)
	{
		auto& values = graphicsEditorComponent;
		fs::path meshPath = values.mMeshPath;
		fs::path texturePath = values.mTexturePath;

		// Extensions is preferred over filename, as filaneme is okay with '.'
		// ... (and more dots) passes the check :/

		if (meshPath.has_extension() && fs::exists(meshPath.c_str()) && meshPath != MeshComp.GetMeshPath())
			MeshComp.SetMeshPath(meshPath.string());

		if (texturePath.has_extension() && fs::exists(texturePath.c_str()) && texturePath != MeshComp.GetTexturePath())
			MeshComp.SetTexturePath(texturePath.string());
	}

	/**
	 * @brief Save the currently present items within the scene into a file.
	 * @param sceneObject
	 * @param registry
	 * @param pathToConfigFile
	*/
	void SaveToAFile(SceneObject const& sceneObject, entt::registry& registry, std::string const& pathToConfigFile)
	{
		vector<vector<json>> jsonedEntities;

		for (auto&& [entity, editTransform] : registry.view<TransformEditorComponent>().each())
		{
			vector<json> components;
			components.emplace_back(editTransform.GetJsonedObject());

			{
				auto* meshComp = registry.try_get<GraphicsEditorComponent>(entity);
				if (meshComp)
					components.emplace_back(meshComp->GetJsonedObject());
			}

			{
				auto* colliderComp = registry.try_get<PhysicsEditorComponent>(entity);
				if (colliderComp)
					components.emplace_back(colliderComp->GetJsonedObject());
			}

			{
				auto* scriptsComp = registry.try_get<ScriptsEditorComponent>(entity);
				if (scriptsComp)
					components.emplace_back(scriptsComp->GetJsonedObject());
			}

			{
				auto* audioComp = registry.try_get<AudioEditorComponent>(entity);
				if (audioComp)
					components.emplace_back(audioComp->GetJsonedObject());
			}

			{
				auto* cameraComp = registry.try_get<CameraEditorComponent>(entity);
				if (cameraComp)
					components.emplace_back(cameraComp->GetJsonedObject());
			}

			{
				auto* lightComp = registry.try_get<LightEditorComponent>(entity);
				if (lightComp)
					components.emplace_back(lightComp->GetJsonedObject());
			}

			{
				auto* beaconNavComp = registry.try_get<BeaconNavEditorComponent>(entity);
				if (beaconNavComp)
					components.emplace_back(beaconNavComp->GetJsonedObject());
			}

			jsonedEntities.emplace_back(components);
		}

		auto document = mExporter.GetJsonedDocument(jsonedEntities);

		mExporter.ExportScene(document, sceneObject, pathToConfigFile);
	}
};
#endif // EDITOR
