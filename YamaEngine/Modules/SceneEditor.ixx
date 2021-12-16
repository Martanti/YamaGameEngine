#ifdef EDITOR
module;
#include "entity/registry.hpp";
export module GameLogic.SceneEditor;
import GameLogic.Base;
import Editor;
import DataVault;

/**
 * @brief A script that handles editor related functionality.
*/
export class SceneEditor : public GameScriptBase
{
private:
	/**
	 * @brief Game scene registry.
	 * @note The one that is passed with the standard functions is the Editor Registry, which contains this camera, but not the loaded scene items.
	*/
	entt::registry* mRegistry;

	/**
	 * @brief Additional editor functionality.
	*/
	Editor mGameEditor;

	/**
	 * @brief A path to the main configuration file.
	*/
	char const* pathToConfig;
public:

	[[nodiscard]] inline virtual YmEventReturn Start(float deltaTime, entt::registry& registry, entt::entity const& thisEntity)
	{
		auto registyRaw = DataVault["GameSceneRegistry"];
		auto configPathRaw = DataVault["PathToConfig"];

		// Editor has no point if it doesn't have registry pointer in it.
		Assert(registyRaw.has_value(), "Data vault did not contain edited game scene registry!");
		Assert(configPathRaw.has_value(), "Data vault did not contain path to configuration file!");


		mRegistry = std::any_cast<entt::registry*>(registyRaw);
		pathToConfig = std::any_cast<char const*>(configPathRaw);
		return {};
	}

	[[nodiscard]] inline YmEventReturn DrawItems() override
	{
		// The idea to have an editor as a part of game scripts came very late (day before submission) so there wasn't any big plan how to make it fit. That's why the received registry argument is re-purposed to have the scene item registry.
		YmEventList gameEvents;

		GUI::Instance().DrawMenuBar();
		{
			auto events = GUI::Instance().DrawEntityManager(*mRegistry);
			if (events.has_value())
				MergeYmEventLists(gameEvents, events.value());
		}

		// Update the transform and the mesh component (if there is one).
		for (auto&& [entity, editTransform, transform] : mRegistry->view<TransformEditorComponent, Transform>().each())
		{
			auto* editMeshComp = mRegistry->try_get<GraphicsEditorComponent>(entity);
			mGameEditor.PopulateTransform(transform, editTransform);

			if (editMeshComp)
			{
				auto& meshComp = mRegistry->get<MeshComp>(entity);
				mGameEditor.PopulateMeshComp(meshComp, *editMeshComp);
			}
		}

		// Checks if save scene dialog is open and if user pressed the button to open it.
		if (GUI::Instance().GetSaveScene())
		{
			auto scenePath = GUI::Instance().GetSceneFileSave();

			if (scenePath.has_value())
			{
				mGameEditor.SaveToAFile(scenePath.value(), *mRegistry, pathToConfig);
			}
		}

		// Checks if open scene dialog is open and if the user pressed the request button.
		if (GUI::Instance().GetOpenScene())
		{
			auto scenePath = GUI::Instance().GetScenePathOpen();

			if (scenePath.has_value())
			{
				auto path = scenePath.value();
				gameEvents.emplace_back(YmEventTypes::SceneFileLoad, new std::string(path));
			}
		}

		// Checks if player has chosen to delete the scene. The deletion has to be in the menu wrapper as it becomes complicated to track the state of it and reset menu related data.
		GUI::Instance().CheckAndCleaneScene(*mRegistry);

		if (gameEvents.size() > 0)
			return gameEvents;
		return {};
	};
};
#endif // DEBUG
