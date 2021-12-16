module;
#ifdef EDITOR
#include "entity/registry.hpp"
// Win API Open File dialog - https://www.daniweb.com/programming/software-development/code/217307/a-simple-getopenfilename-example
// Win API open file dialog related includes, read more about it in the function that calls them.
//#include <Windows.h>
//#include <Commdlg.h>
#endif // EDITOR

export module GUIWrapper;
import "irrlicht.h";
import <IrrIMGUI/IncludeIMGUI.h>;
import <IrrIMGUI/IrrIMGUI.h>;
import <IrrIMGUI/IrrIMGUIDebug.h>;
import <string_view>;
import Logger;
import YamaEvents;

#ifdef GAME
import HighScoreEntity;
import <vector>;
import <algorithm>;
#endif // GAME

#ifdef EDITOR
import "EditorComponents.h";
import <filesystem>;

import SceneObject;
// The bool 0 - delete, 1 - add
// As long as it doesn't get any more complex it should be an okay to use a tuple instead of a struct
export typedef std::tuple<entt::entity, EditorComponentTypes, bool> Modification;

namespace fs = std::filesystem;
#endif // EDITOR

using namespace IrrIMGUI;

// Main reason why this is not a namespace with global functions like with Input wrapper is the amount of private variable, just doesn't feel right to have them in the global scope, while there shouldn't be any real problems with that.

/**
 * @brief Class for GUI.
*/
export class GUI
{
private:
	/**
	 * @brief The IrrImGui event receiver.
	*/
	CIMGUIEventReceiver mEventReceiver;

	/**
	 * @brief Handler for the GUI.
	 * @note mainly to manage the GUI. For engine use.
	*/
	IIMGUIHandle* mGUI;

	/**
	 * @brief Flag for checking if user decided to exit via GUI.
	*/
	bool mMenuQuit = false;

#ifdef EDITOR
	/**
	 * @brief Flag to check if user has selected top menu button for saving the scene.
	*/
	bool mSaveScene = false;

	/**
	 * @brief Temporary holder for the scene name.
	*/
	std::string mSceneName = "";

	/**
	 * @brief Temporary holder for the scene paths.
	*/
	std::string mScenePath = "";

	/**
	 * @brief Temporary holder for the scene ID.
	*/
	int sceneId = 0;

	/**
	 * @brief Flag to check if user has pressed the button in the top menu to open a scene.
	*/
	bool mOpenScene = false;

	/**
	 * @brief Temporary path holder for a scene file path.
	*/
	std::string mSceneOpenPath = "";

	/**
	 * @brief Flag to check if user requested registry clean from the top menu.
	*/
	bool mClean = false;

	// Win API open file dialog related variables, read more about it in the function that calls them.
	/*OPENFILENAME ofn;
	char szFile[100];*/

#endif // EDITOR

	/**
	 * @brief A collection of ImGUI flags to make the window transparent and without the chance to move it or interavt with it.
	*/
	ImGuiWindowFlags mJustTextWindowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBringToFrontOnFocus;

	/**
	 * @brief Window height.
	*/
	unsigned int mHeight;

	/**
	 * @brief Window width.
	*/
	unsigned int mWidth;

private:
	GUI() {};

	/**
	 * @brief A small helper for displaying a string in ImGui.
	 * @param str
	 * @param title
	*/
	void InputString(std::string& str, std::string const& title)
	{
		char val[129];
		strcpy_s(val, (str.c_str()));
		ImGui::InputText(title.c_str(), val, 128);
		str = std::string(val);
	}

#ifdef EDITOR
	/**
	 * @brief Display the buttons for various editor components.
	 * @param entity
	 * @param addedItems
	 * @param meshComp
	 * @param colliderComp
	 * @param scriptsComp
	 * @param audioComp
	 * @param cameraComp
	 * @param lightComp
	 * @param beaconNavComp
	*/
	void inline PopulateComponentButtons(entt::entity const& entity, YmEventList& addedItems, GraphicsEditorComponent* meshComp, PhysicsEditorComponent* colliderComp, ScriptsEditorComponent* scriptsComp, AudioEditorComponent* audioComp, CameraEditorComponent* cameraComp, LightEditorComponent* lightComp, BeaconNavEditorComponent* beaconNavComp)
	{
		// Ideally this would be populated by some list that is filled based on reflection (get all the children of editor component class)
		if (!meshComp && ImGui::Button("Add mesh"))
		{
			addedItems.emplace_back(
				EditorChange,
				new Modification(entity, EditorComponentTypes::EditorMeshType, true));
		}

		if (!colliderComp && ImGui::Button("Add cube collider"))
		{
			addedItems.emplace_back(
				EditorChange,
				new Modification(entity, EditorComponentTypes::EditorColliderType, true));
		}

		if (!scriptsComp && ImGui::Button("Add Scripts"))
		{
			addedItems.emplace_back(
				EditorChange,
				new Modification(entity, EditorComponentTypes::EditorGameScriptType, true));
		}

		if (!audioComp && ImGui::Button("Add Audio"))
		{
			addedItems.emplace_back(
				EditorChange,
				new Modification(entity, EditorComponentTypes::EditorAudioSourcesType, true));
		}

		if (!cameraComp && ImGui::Button("Add FPS Camera"))
		{
			addedItems.emplace_back(
				EditorChange,
				new Modification(entity, EditorComponentTypes::EditorCameraType, true));
		}

		if (!lightComp && ImGui::Button("Add Light"))
		{
			addedItems.emplace_back(
				EditorChange,
				new Modification(entity, EditorComponentTypes::EditorLightType, true));
		}

		if (!beaconNavComp && ImGui::Button("Add beacon navigation"))
		{
			addedItems.emplace_back(
				EditorChange,
				new Modification(entity, EditorComponentTypes::EditorPathNavType, true));
		}
	}

	// These DisplayMenu functions were previously inside the EditorComponent class, but in order to reduce coupling they were brought here.

	/**
	 * @brief Display editor for the transform component.
	 * @param comp
	*/
	inline void DisplayMenuTrasform(TransformEditorComponent* comp)
	{
		InputString(comp->mName, "Name");
		InputString(comp->mTag, "Tag");

		ImGui::Text("Position");
		ImGui::Indent();
		ImGui::InputFloat("X##p", &comp->mPos.X);
		ImGui::InputFloat("Y##p", &comp->mPos.Y);
		ImGui::InputFloat("Z##p", &comp->mPos.Z);
		ImGui::Unindent();

		ImGui::Text("Rotation");
		ImGui::Indent();
		ImGui::InputFloat("X##r", &comp->mRot.X);
		ImGui::InputFloat("Y##r", &comp->mRot.Y);
		ImGui::InputFloat("Z##r", &comp->mRot.Z);
		ImGui::Unindent();

		ImGui::Text("Scale");
		ImGui::Indent();
		ImGui::InputFloat("X##s", &comp->mScale.X);
		ImGui::InputFloat("Y##s", &comp->mScale.Y);
		ImGui::InputFloat("Z##s", &comp->mScale.Z);
		ImGui::Unindent();
	}

	/**
	 * @brief Display editor for the graphics component.
	 * @param comp
	*/
	inline void DisplayMenuGraphics(GraphicsEditorComponent* comp)
	{
		InputString(comp->mMeshPath, "Mesh path");
		InputString(comp->mTexturePath, "Texture path");

		if (ImGui::Button("Add animation"))
		{
			comp->mAnimations.emplace_back(GraphicsEditorComponent::AnimationData{.Speed = 0, .FrameStart = 0, .FrameEnd = 0, .Name = "Still" });
		}

		auto iterator = comp->mAnimations.begin();
		while (iterator != comp->mAnimations.end())
		{
			auto id = iterator - comp->mAnimations.begin();

			if (ImGui::Button(std::format("Remove animation##anim{}", id).c_str()))
			{
				iterator = comp->mAnimations.erase(iterator);
				ImGui::Separator();
				continue;
			}
			auto& [Speed, FrameStart, FrameEnd, Name] = *iterator;

			InputString(Name, std::format("Name##animName{}", id));
			ImGui::InputInt(std::format("Starting frame##animStart{}", id).c_str(), &FrameStart);
			ImGui::InputInt(std::format("End frame##animEnd{}", id).c_str(), &FrameEnd);
			ImGui::InputFloat(std::format("Speed/FPS##animSpeed{}", id).c_str(), &Speed);

			ImGui::Separator();
			iterator++;
		}
	}

	/**
	 * @brief Display editor for the physics component.
	 * @param comp
	*/
	inline void DisplayMenuCollider(PhysicsEditorComponent* comp)
	{
		//InputString(mColliderType, "Collider type");
		ImGui::InputFloat("Mass", &comp->mMass);

		ImGui::Text("Offset");
		ImGui::Indent();
		ImGui::InputFloat("X##o", &comp->mOffsets.X);
		ImGui::InputFloat("Y##o", &comp->mOffsets.Y);
		ImGui::InputFloat("Z##o", &comp->mOffsets.Z);
		ImGui::Unindent();

		ImGui::Text("Extents");
		ImGui::Indent();
		ImGui::InputFloat("X##e", &comp->mExtents.X);
		ImGui::InputFloat("Y##e", &comp->mExtents.Y);
		ImGui::InputFloat("Z##e", &comp->mExtents.Z);
		ImGui::Unindent();
	}

	/**
	 * @brief Display editor for the scripts manager component.
	 * @param comp
	*/
	inline void DisplayMenuScripts(ScriptsEditorComponent* comp)
	{
		ImGui::Text("To add a script press a button to add a new entry and the write its name");

		if (ImGui::Button("Add a script"))
		{
			comp->mScriptContainer.emplace_back("Script name here");
		}
		auto iterator = comp->mScriptContainer.begin();
		ImGui::Indent();
		ImGui::Separator();
		while (iterator != comp->mScriptContainer.end())
		{
			auto id = iterator - comp->mScriptContainer.begin();
			InputString(*iterator, std::format("Script##{}", id));

			if (ImGui::Button(std::format("Remove script##{}", id).c_str()))
			{
				iterator = comp->mScriptContainer.erase(iterator);
				ImGui::Separator();
				continue;
			}
			ImGui::Separator();
			iterator++;
		}
		ImGui::Unindent();
	}

	/**
	 * @brief Display editor for the audio manager component.
	 * @param comp
	*/
	inline void DisplayMenuAudio(AudioEditorComponent* comp)
	{

		if (ImGui::Button("Add an audio source"))
		{
			comp->mAudioSources.emplace_back(std::make_pair("Name", "Path"));
		}

		auto iterator = comp->mAudioSources.begin();

		while (iterator != comp->mAudioSources.end())
		{
			auto id = iterator - comp->mAudioSources.begin();

			if (ImGui::Button(std::format("Remove source##{}", id).c_str()))
			{
				iterator = comp->mAudioSources.erase(iterator);
				ImGui::Separator();
				continue;
			}

			auto& [name, path] = *iterator;

			InputString(name, std::format("Name##audio{}", id));
			InputString(path, std::format("Path##audio{}", id));

			iterator++;
			ImGui::Separator();
		}
	}

	/**
	 * @brief Display editor for the camera component.
	 * @param comp
	*/
	inline void DisplayMenuCamera(CameraEditorComponent* comp)
	{
		InputString(comp->mCamType, "Camera type##CamT");
		ImGui::Text("Offsets:");
		ImGui::Indent();
		ImGui::InputFloat("X##CamOff", &comp->mOffsets.X);
		ImGui::InputFloat("Y##CamOff", &comp->mOffsets.Y);
		ImGui::InputFloat("Z##CamOff", &comp->mOffsets.Z);
		ImGui::Unindent();
	}

	/**
	 * @brief Display the editor for the light component.
	 * @param comp
	*/
	inline void DisplayMenuLight(LightEditorComponent* comp)
	{
		InputString(comp->mLightType, "Light Type##lghtT");
		ImGui::InputFloat("Range##lghtR", &comp->mRange);

		ImGui::Text("Diffuse colours");
		ImGui::Indent();
		ImGui::InputFloat("R##lghtDR", &comp->mDiffuseR);
		ImGui::InputFloat("G##lghtDG", &comp->mDiffuseG);
		ImGui::InputFloat("B##lghtDB", &comp->mDiffuseB);
		ImGui::InputFloat("A##lghtDA", &comp->mDiffuseA);
		ImGui::Unindent();

		ImGui::Text("Ambient colours");
		ImGui::Indent();
		ImGui::InputFloat("R##lghtAR", &comp->mAmbientR);
		ImGui::InputFloat("G##lghtAG", &comp->mAmbientG);
		ImGui::InputFloat("B##lghtAB", &comp->mAmbientB);
		ImGui::InputFloat("A##lghtAA", &comp->mAmbientA);
		ImGui::Unindent();
	}

	/**
	 * @brief Display editor for he AI navigation component.
	 * @param comp
	*/
	inline void DisplayAiNav(BeaconNavEditorComponent* comp)
	{
		if (ImGui::Button("Add Point"))
		{
			comp->mBeacons.emplace_back(0, 0, 0);
		}

		auto iterator = comp->mBeacons.begin();
		while (iterator != comp->mBeacons.end())
		{
			ImGui::Separator();
			auto id = iterator - comp->mBeacons.begin();

			if (ImGui::Button(std::format("Remove Point##{}", id).c_str()))
			{
				iterator = comp->mBeacons.erase(iterator);
				ImGui::Separator();
				continue;
			}

			ImGui::Indent();

			// If this is used a lot and needs some kind of simplification tree node structure could be used, so that the items can be collapsed.

			ImGui::InputFloat(std::format("X##AINavX{}", id).c_str(), &iterator->X);
			ImGui::InputFloat(std::format("Y##AINavY{}", id).c_str(), &iterator->Y);
			ImGui::InputFloat(std::format("Z##AINavZ{}", id).c_str(), &iterator->Z);

			ImGui::Unindent();
			iterator++;
		}
	}

	/**
	 * @brief Display the component menu.
	 * @param comp
	 * @param entity
	 * @param addedItems
	*/
	inline void DisplayComponentMenu(EditorComponentBase* comp, entt::entity const& entity, YmEventList& addedItems)
	{
		auto  componentType = comp->GetTypeName();
		if (ImGui::TreeNode(std::format("{}", componentType).c_str()))
		{
			// It will short circuit and hide it in transform section
			if (componentType != "Transform" && ImGui::Button("Remove component"))
			{
				addedItems.emplace_back(
					EditorChange,
					new Modification(entity, comp->ComponentType, false));
				ImGui::TreePop();
				return;
			}

			switch (comp->ComponentType)
			{
				using enum EditorComponentTypes;
				case EditorTransformType:
				{
					auto* compCasted = (TransformEditorComponent*)comp;
					DisplayMenuTrasform(compCasted);
					break;
				}

				case EditorMeshType:
				{
					auto* compCasted = (GraphicsEditorComponent*)comp;
					DisplayMenuGraphics(compCasted);
					break;
				}

				case EditorColliderType:
				{
					auto* compCasted = (PhysicsEditorComponent*)comp;
					DisplayMenuCollider(compCasted);
					break;
				}

				case EditorGameScriptType:
				{
					auto* compCasted = (ScriptsEditorComponent*)comp;
					DisplayMenuScripts(compCasted);
					break;
				}

				case EditorAudioSourcesType:
				{
					auto* compCasted = (AudioEditorComponent*)comp;
					DisplayMenuAudio(compCasted);
					break;
				}

				case EditorCameraType:
				{
					auto* compCasted = (CameraEditorComponent*)comp;
					DisplayMenuCamera(compCasted);
					break;
				}

				case EditorLightType:
				{
					auto* compCasted = (LightEditorComponent*)comp;
					DisplayMenuLight(compCasted);
					break;
				}

				case EditorPathNavType:
				{
					auto* compCasted = (BeaconNavEditorComponent*)comp;
					DisplayAiNav(compCasted);
					break;
				}

				default:
					Log(LGR_WARNING, std::format("The component type {} of editor component is not handled", (int)comp->ComponentType));
					break;
			}
			ImGui::TreePop();
		}
	}

#endif
public:
	GUI(GUI const&) = delete;
	void operator=(GUI const&) = delete;

	// Based on this
	// https://stackoverflow.com/questions/1008019/c-singleton-design-pattern

	/**
	 * @brief Access GUI singleton instance.
	 * @return
	*/
	inline static GUI& Instance()
	{
		static GUI instance;
		return instance;
	}

	/**
	 * @brief Set cursor visibility state.
	 * @param drawCursor
	*/
	inline void SetCursorState(bool const drawCursor)
	{
		auto& io = ImGui::GetIO();
		io.MouseDrawCursor = drawCursor;
	}

	// Will not make this into a constructor as Graphics device requires the event receiver pointer
	// Was thinking about making this private and the make the engine a friend class, issue is that this over-complicates things as variables that are meant to be truly private are no longer really private

	/**
	 * @brief Initialize the GUI.
	 * @param irrDeviceRaw
	 * @param screeHeight
	 * @param screeWidth
	*/
	inline void InitUI(void* irrDeviceRaw, unsigned int& screeHeight, unsigned int& screeWidth)
	{
		mHeight = screeHeight;
		mWidth = screeWidth;
		auto* irrDevice = static_cast<irr::IrrlichtDevice*>(irrDeviceRaw);
		SIMGUISettings Settings;
		Settings.mIsIMGUIMemoryAllocationTrackingEnabled = false;
		mGUI = createIMGUI(irrDevice, &mEventReceiver, &Settings);
		AssertNotNull(mGUI);
	}

	/**
	 * @brief Declare start of the menu drawing section.
	*/
	inline void StartDrawing()
	{
		mGUI->startGUI();
	}

	/**
	 * @brief Declare end to the menu drawing section.
	*/
	inline void StopDrawing()
	{
		mGUI->drawAll();
	}

	/**
	 * @brief Clean up the GUI.
	*/
	inline void Cleanup()
	{
		mGUI->drop();
	}

	/**
	 * @brief Get the IrrImGUI input event handler.
	 * @return
	*/
	inline void* GetImguiEventReceiver()
	{
		return &mEventReceiver;
	}

	/**
	 * @brief Check if user has decided to exit via GUI.
	 * @return
	*/
	inline bool const GetMenuQuit() noexcept
	{
		return mMenuQuit;
	}

#ifdef EDITOR

	/**
	 * @brief Check if user decided to save a scene file.
	 * @return
	*/
	inline bool const GetSaveScene() noexcept
	{
		return mSaveScene;
	}

	/**
	 * @brief Check if user decided to open a scene file.
	 * @return
	*/
	inline bool const GetOpenScene() noexcept
	{
		return mOpenScene;
	}

	/**
	 * @brief Handle if user decided to clear the scene.
	 * @param registry
	 * @return
	*/
	inline void const CheckAndCleaneScene(entt::registry& registry)
	{
		if (mClean)
		{
			registry.clear();
			mClean = false;
		}
	}

	/**
	* @brief Draw menu bar that is at the top of the window.
	*/
	inline void DrawMenuBar()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Start fresh! (Deletes current data)"))
				{
					mOpenScene = false;
					mSaveScene = false;
					mClean = true;
				}
				if (ImGui::MenuItem("Open scene file"))
				{
					mOpenScene = true;
					mSceneOpenPath = "";
				}

				if (ImGui::MenuItem("Save scene file"))
				{
					mSaveScene = true;
					mSceneName = "";
					mScenePath = "";
					sceneId = 0;
				}


				if (ImGui::MenuItem("Quit", "Alt+F4", &mMenuQuit)) {}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}

	// This could be used instead of the string input
	//https://github.com/aiekick/ImGuiFileDialog

	// Will be used as a part of the editor
	// Can create and destroy entities, but doesn't add/remove components

	/**
	* @brief Display the main editor menu.
	*/
	[[nodiscard]] inline YmEventReturn DrawEntityManager(entt::registry& registry)
	{
		ImGui::SetNextWindowPos(ImVec2(50.0f, 50.0f), ImGuiSetCond_FirstUseEver | ImGuiSetCond_Once);
		ImGui::SetNextWindowSize(ImVec2(350.0f, 600.0f), ImGuiSetCond_FirstUseEver | ImGuiSetCond_Once);

		YmEventList addedItems;

		if (ImGui::Begin("Editor", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (ImGui::Button("Add new entity"))
			{
				auto entity = registry.create();

				addedItems.emplace_back(
					EditorChange,
					new Modification(entity, EditorComponentTypes::EditorTransformType, true));
				Log(LGR_INFO, "New entity added");
			}

			for (auto&& [entity, transform] : registry.view<TransformEditorComponent>().each())
			{
				// Tried having trnasform.mName as part of the node name - editing it immediately closes the tree node, so it needs some extra investigation done.
				if (ImGui::TreeNode(std::format("Entity #{}", (int)entity).c_str()))
				{
					auto* meshComp = registry.try_get<GraphicsEditorComponent>(entity);
					auto* colliderComp = registry.try_get<PhysicsEditorComponent>(entity);
					auto* scriptsComp = registry.try_get<ScriptsEditorComponent>(entity);
					auto* audioComp = registry.try_get<AudioEditorComponent>(entity);
					auto* cameraComp = registry.try_get<CameraEditorComponent>(entity);
					auto* lightComp = registry.try_get<LightEditorComponent>(entity);
					auto* beaconNavComp = registry.try_get<BeaconNavEditorComponent>(entity);

					// Do the component addition here
					std::vector<EditorComponentBase*> activeComponents;
					activeComponents.reserve(3);
					activeComponents.emplace_back(&transform);

					if (meshComp)
						activeComponents.emplace_back(meshComp);

					if (colliderComp)
						activeComponents.emplace_back(colliderComp);

					if (scriptsComp)
						activeComponents.emplace_back(scriptsComp);

					if (audioComp)
						activeComponents.emplace_back(audioComp);

					if (cameraComp)
						activeComponents.emplace_back(cameraComp);

					if (lightComp)
						activeComponents.emplace_back(lightComp);

					if(beaconNavComp)
						activeComponents.emplace_back(beaconNavComp);

					if (ImGui::TreeNode("Entity Modification"))
					{
						if (ImGui::Button("Remove entity"))
						{
							registry.destroy(entity);
							ImGui::TreePop();
							ImGui::TreePop();
							continue;
						}

						PopulateComponentButtons(entity, addedItems, meshComp, colliderComp, scriptsComp, audioComp, cameraComp, lightComp, beaconNavComp);

						ImGui::TreePop();
					}

					if (ImGui::TreeNode("Entity Components"))
					{
						for (auto& comp : activeComponents)
						{
							DisplayComponentMenu(comp, entity, addedItems);
						}
						ImGui::TreePop();
					}

					ImGui::TreePop();
					ImGui::Separator();
				}
			}
		}
		ImGui::End();


		if (addedItems.size() > 0)
			return addedItems;

		return {};
	}

	// Scenes will need a file path and a scene id from 1 to 12 (based on f keys)
	// Check if the path exists and if the scene id is used, say that this will overwrite stuff if they are

	/**
	 * @brief Get a path to save the scene file in.
	 * @return If user has submitted a path it will be returned, otherwise - empty object.
	*/
	inline std::optional<SceneObject> GetSceneFileSave()
	{
		// Disable every other overlay, by not rendering when this is out, unless cancel pressed
		// Return empty optional if no "selection has been made"
		// If presses cancel - class member var will be changed to allow further UI to be created

		if (mSaveScene)
		{
			if (ImGui::Begin("Save Scene file", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("Duplicate ID and/or path might overwrite already existing scene files");

				char path[129];
				strcpy_s(path, mScenePath.c_str());
				ImGui::InputText("Path to scene", path, 128);
				mScenePath = path;


				char name[129];
				strcpy_s(name, mSceneName.c_str());
				ImGui::InputText("Scene name", name, 128);
				mSceneName = name;

				ImGui::InputInt("Scene Id", &sceneId);

				bool correct = true;

				auto a = mScenePath.size();
				auto b = mSceneName.size();
				if (a == 0 || b == 0)
				{
					ImGui::Text("Both text fields need to be filled");
					correct = false;
				}

				if (ImGui::Button("Cancel"))
				{
					mSaveScene = false;
				}

				if (correct && ImGui::Button("Save scene"))
				{
					mSaveScene = false;
					ImGui::End();
					return SceneObject(mScenePath, mSceneName, sceneId);
				}
			}
			ImGui::End();
		}
		return {};
	}

	/**
	 * @brief Get a user provided path that user decided to open scene from.
	 * @return Returns a string path if user submitted it, otherwise - empty object.
	*/
	inline std::optional<std::string> GetScenePathOpen()
	{
		if (mOpenScene)
		{
			// Enable this, includes and the related variables to enable windows API open file dialog.
			// This was commented out because (probably due to old ImGui version) the cursor is invisible in the dialog, thus making it hard to use.

			// If newer ImGUI was used this https://github.com/AirGuanZ/imgui-filebrowser
			// Could be used instead

			/*ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = NULL;
			ofn.lpstrFile = szFile;
			ofn.lpstrFile[0] = '\0';
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = ("JSON\0*.JSON\0");
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = NULL;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

			GetOpenFileName(&ofn);
			mOpenScene = false;*/
			if (ImGui::Begin("Open Scene file", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				char path[129];
				strcpy_s(path, mScenePath.c_str());
				ImGui::InputText("Path to the scene file##open", path, 128);
				mScenePath = path;

				if (ImGui::Button("Cancel"))
				{
					mOpenScene = false;
					ImGui::End();
					return {};
				}

				if (fs::exists(mScenePath) && fs::is_regular_file(mScenePath))
				{
					if (ImGui::Button("Open"))
					{
						mOpenScene = false;
						ImGui::End();

						return mScenePath;
					}
				}
				else
				{
					ImGui::Text("The provided path is not valid");
				}
			}
			ImGui::End();
		}
		return {};
	}

	/**
	 * @brief Display editor camera control state.
	 * @param cameraEnabled
	*/
	inline void CameraControlState(bool cameraEnabled)
	{
		ImGui::SetNextWindowPos(ImVec2(mWidth - 275.0f, 25.0f), ImGuiSetCond_FirstUseEver | ImGuiSetCond_Once);
		ImGui::SetNextWindowSize(ImVec2(250.0f, 40.0f), ImGuiSetCond_FirstUseEver | ImGuiSetCond_Once);

		ImVec4 textColor = cameraEnabled ? ImVec4(0, 0.5, 0, 1) : ImVec4(0.5, 0, 0, 1);

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0.3));
		ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_Text, textColor);

		if (ImGui::Begin("CameraControlState", NULL, mJustTextWindowFlags))
		{
			ImGui::Text(std::format("Free flight camera(F4): {}", cameraEnabled ? "enabled" : "disabled").c_str());
			ImGui::End();
		}
		ImGui::PopStyleColor(3);
	}
#endif // EDITOR

#ifdef GAME
	/**
	 * @brief Display current player score.
	 * @param spentSeconds
	 * @param gatheredMainScore
	 * @param gatheredExtraScore
	 * @param maxMainScore
	*/
	inline void PlayerCurrentScore(float spentSeconds, float  gatheredMainScore, float gatheredExtraScore, float maxMainScore)
	{
		ImGui::SetNextWindowPos(ImVec2(50.0f, 50.0f), ImGuiSetCond_FirstUseEver | ImGuiSetCond_Once);
		ImGui::SetNextWindowSize(ImVec2(250.0f, 100.0f), ImGuiSetCond_FirstUseEver | ImGuiSetCond_Once);

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 1, 1));

		if (ImGui::Begin("Current Score", NULL, mJustTextWindowFlags))
		{
			ImGui::Text(std::format("Main score {}/{}", gatheredMainScore, maxMainScore).c_str());
			ImGui::Text(std::format("Extra score {}", gatheredExtraScore).c_str());

			auto miliseconds = spentSeconds - floor(spentSeconds);
			miliseconds = (int)(miliseconds * 100 + .5f);
			int seconds = (int)spentSeconds % 60;
			int minutes = spentSeconds / 60;

			ImGui::Text(std::format("Time spent: {}:{}.{}", minutes, seconds, miliseconds*10).c_str());
			ImGui::End();
		}
		ImGui::PopStyleColor(3);
	}

	/**
	 * @brief Display the top 5 scores in a table.
	 * @param highscores
	 * @param hasFinishedConnecting
	*/
	inline void DisplayHighScores(std::vector<HighScoreEntity> highscores, bool hasFinishedConnecting)
	{
		ImGui::SetNextWindowPos(ImVec2(mWidth - 350, 50), ImGuiSetCond_FirstUseEver | ImGuiSetCond_Once);
		ImGui::SetNextWindowSize(ImVec2(300.0f, 250.0f), ImGuiSetCond_FirstUseEver | ImGuiSetCond_Once);

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0.84, 0, 1));

		if (ImGui::Begin("HighScores", NULL, mJustTextWindowFlags))
		{
			auto size = highscores.size();

			if (!hasFinishedConnecting)
			{
				ImGui::Text("Connecting to WEB API...");
			}

			else {
				if (size == 0)
				{
					ImGui::Text("ERROR: Connection issues!");
				}

				else
				{
					ImGui::Text("RANK - NAME - SCORE - DATE");
					for (size_t i = 0; i < size; i++)
					{
						auto& entry = highscores[i];
						ImGui::Text(std::format("#{} - {} - {} - {}", i + 1, entry.mName, entry.mScore, entry.mDate ).c_str());
					}
				}
			}
			ImGui::End();
		}
		ImGui::PopStyleColor(3);
	}

	/**
	* @brief Show title screen.
	*/
	[[nodiscard]] inline YmEventReturn TitleScreenText()
	{
		YmEventList events;
		ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiSetCond_FirstUseEver | ImGuiSetCond_Once);
		ImGui::SetNextWindowSize(ImVec2(500.0f, 500.0f), ImGuiSetCond_FirstUseEver | ImGuiSetCond_Once);

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));

		auto flags = mJustTextWindowFlags;

		flags &= ~ImGuiWindowFlags_NoInputs;

		if (ImGui::Begin("Title scene", NULL, flags))
		{
			ImGui::Text("Jumpy game - YAMA Engine demo");
			ImGui::Text("By Martynas Antipenkovas");
			ImGui::Text("2021. 301CR Coursework");
			ImGui::Text("");
			ImGui::Text("WASD  - move.");
			ImGui::Text("Mouse - look around.");
			ImGui::Text("Space - jump.");
			ImGui::Text("");
			ImGui::Text("Collect yellow points as fast as you can!");
			ImGui::Text("Collecting purple points will give extra score");
			ImGui::Text("");
			ImGui::Text("Scenes:");

			if (ImGui::Button("Title screen (F1)"))
			{
				events.emplace_back(YmEventTypes::ChangeScene, new SceneChangeMessage{ 1 });
			}

			if (ImGui::Button("The game (F2)"))
			{
				events.emplace_back(YmEventTypes::ChangeScene, new SceneChangeMessage{ 2 });
			}

			if (ImGui::Button("Exit (ESC)"))
			{
				events.emplace_back(YmEventTypes::ExitApplication, nullptr);
			}

			ImGui::End();
		}
		ImGui::PopStyleColor(3);

		if (events.size() > 0)
			return events;

		return {};
	}

	/**
	* @brief Display the window for score submission.
	*/
	[[nodiscard]] inline YmEventReturn ScoreSubmission(float const& score, std::string& name, bool& upload)
	{ // The instead of the referenced arguments they could have been put into a return std::variant or tuple, but it would also probably need to be in a std::optional, so decided to check if something happened by using the std::optional within the YmEventReturn and only then check the bool from arguments.

		ImGui::SetNextWindowPos(ImVec2(150, 150), ImGuiSetCond_FirstUseEver | ImGuiSetCond_Once);
		ImGui::SetNextWindowSize(ImVec2(mWidth * 0.5f, mHeight * 0.5f), ImGuiSetCond_FirstUseEver | ImGuiSetCond_Once);

		YmEventList sceneChange;
		if (ImGui::Begin("Submit your score!", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text(std::format("Your score is {}", score).c_str());
			char nameArr[4];
			transform(name.begin(), name.end(), name.begin(), ::toupper);
			strcpy_s(nameArr, (name.c_str()));
			ImGui::InputText("Name", nameArr, 4);
			name = std::string(nameArr);

			// There could be a functionality to limit the input to allow only letters and numbers, but currently it is skipped due to lack of time.

			bool changeScene = false;
			if (ImGui::Button("Submit"))
			{
				upload = true;
				changeScene = true;
			}

			if (ImGui::Button("Cancel"))
			{
				upload = false;
				changeScene = true;
			}

			ImGui::End();
			if (changeScene)
			{
				sceneChange.emplace_back(YmEventTypes::ChangeScene, new SceneChangeMessage{ 1 });
				return sceneChange;
			}
		}

		return {};
	}
#endif // GAME

#ifdef PROFILING
	/**
	* @brief Draws profiling statistics window
	* @param fps
	* @param drawnPrimitives
	*/
	inline void DrawStats(signed int fps, unsigned int drawnPrimitives, size_t usedMemory, double usedCPU, float drawingTime
#ifdef GAME
						  ,float  gameScriptHandleTime
						  , float physicsUpdateTime
						  , float physicsResponseTime
#endif // GAME
						  )
	{
		ImGui::SetNextWindowPos(ImVec2(mWidth - 400, mHeight - 230), ImGuiSetCond_FirstUseEver | ImGuiSetCond_Once);
		ImGui::SetNextWindowSize(ImVec2(350.0f, 180.0f), ImGuiSetCond_FirstUseEver | ImGuiSetCond_Once);
		if (ImGui::Begin("Profiling", NULL))
		{
			constexpr double MBCoeficient = 1.f / (1024.f * 1024.f);
			float usedMemoryMod = ((int) (usedMemory * MBCoeficient) * 1000) / 1000.f;
			float usedCPUMod = ((int) usedCPU * 100) / 100.f;
			ImGui::Text(std::format("Average FPS (By IRR) (every 1.5s): {}", fps).c_str());
			ImGui::Text(std::format("Drawn primitives (By IRR) : {}", drawnPrimitives).c_str());
			ImGui::Text(std::format("Memory usage(MB): ~{}", usedMemoryMod).c_str());
			ImGui::Text(std::format("CPU usage(%%): ~{}", usedCPUMod).c_str());
			ImGui::Text(std::format("Time Rendering(s): ~{}", drawingTime).c_str());
#ifdef GAME
			ImGui::Text(std::format("Time GameScripts(s): ~{}", gameScriptHandleTime).c_str());
			ImGui::Text(std::format("Time Physics(s): ~{}", physicsUpdateTime).c_str());
			ImGui::Text(std::format("Time Physics responses(s): ~{}", physicsResponseTime).c_str());
#endif // GAME

		}
		ImGui::End();
	}
#endif // PROFILING
};