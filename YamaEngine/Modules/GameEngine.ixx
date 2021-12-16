module;
//https://github.com/Kerndog73/EnTT-Pacman geve some ideas, like creating views and not storing them
#include "entity/registry.hpp" // I tried putting this into it's own module and import it that way, but I would get a some CL.exe error with a negative error code :/
#include "entity/observer.hpp"
export module GameEngine;
import <chrono>;
import <unordered_map>;
import "Initializer.h";
import <memory>;
import <deque>;
import Graphics;
import Logger;
import Component.Transform;
import Input;
import GUIWrapper;
import YamaEvents;
import Component.GameLogic;
import DataVault;

#ifdef GAME
import PhysicsWrapper;
import AudioWrapper;
#endif // GAME

#ifdef PROFILING
import YaMaProfiling;
#endif

using std::unique_ptr;
using std::vector;

/**
 * @brief Path to the main configuration file.
*/
constexpr auto PathToConfigFile = "yamaConfig.json";

// Also works in 240, but just in case set it to 120 as it seems to be a healthy amount.

/**
 * @brief A fixed game and physics step.
*/
constexpr float gameTimeStep = 1 / 120.f;

/**
 * @brief A Game engine class containing all of the functionality needed to run Yama engine
*/
export class GameEngine
{
private:
	/**
	 * @brief Graphics engine wrapper.
	*/
	unique_ptr<GraphicsWrapper> mGraphics;

	/**
	 * @brief Input manager.
	*/
	Input::InputWrapper mInput;

#ifdef GAME
	/**
	 * @brief Physics engine wrapper.
	*/
	unique_ptr<PhysicsWrapper> mPhysics;

	/**
	 * @brief Flag to check if a new scene should be loaded.
	*/
	bool mLoadNewScene = false;
#endif // GAME

	/**
	 * @brief Initializer handler.
	*/
	Initializer mInit;

	// Can't use unique_ptr because there is some weird error saying the type cannot be deducted from within the poly file.
	/**
	 * @brief Main scene ECS handler.
	*/
	entt::registry* mRegistry;

	/**
	 * @brief Flag to check if user decided to exit.
	*/
	bool mUserExit = false;

#ifdef EDITOR
	// Meant for entities that are part of the editor and not of the scene that it is editing. Like a script for editor camera.
	/**
	 * @brief Editor part of the engine ECS handler.
	*/
	entt::registry* mEditorRegistry;
#endif // EDITOR

#ifdef PROFILING
	/**
	 * @brief Profiling watch helper.
	*/
	yaprfl::StopWatch mWatch;
#endif // PROFILING
private:

#ifdef EDITOR
	/**
	 * @brief Apply editor component changes onto the component registry.
	 * @param modification
	*/
	void ApplyModifications(Modification& modification)
	{
		auto& [entity, componentType, add] = modification;
		switch (componentType)
		{
			using enum EditorComponentTypes;

			case EditorTransformType:
			{
				if (add)
				{
					mRegistry->emplace<Transform>(entity);
					mRegistry->emplace<TransformEditorComponent>(entity);
				}
				// Shouldn't be able to remove the transform component as then the entity would become orphaned + every entity (almost) needs it.
				break;
			}
			case EditorMeshType:
			{
				if (add)
				{
					auto mesh = mGraphics->AddMesh("", "");

					if (mesh.has_value())
					{
						mRegistry->emplace<MeshComp>(entity, std::move(mesh.value()));
						mRegistry->emplace<GraphicsEditorComponent>(entity);
					}
					else
					{
						Log(LGR_ERROR, std::format("Could not add mesh to entity #{}", (int)entity));
					}
				}
				else
				{
					mRegistry->remove<MeshComp>(entity);
					mRegistry->remove<GraphicsEditorComponent>(entity);
				}
				break;
			}
			// The code bellow could probably be simplified by stuff mentioned in  "Runtime Views" in the documentation main page.
			case EditorColliderType:
			{
				// Could have a graphics component that would show wireframe for collider
				if (add)
					mRegistry->emplace<PhysicsEditorComponent>(entity);
				else
					mRegistry->remove<PhysicsEditorComponent>(entity);

				break;
			}

			case EditorGameScriptType:
			{
				if (add)
					mRegistry->emplace<ScriptsEditorComponent>(entity);
				else
					mRegistry->remove<ScriptsEditorComponent>(entity);
				break;
			}

			case EditorAudioSourcesType:
			{
				if (add)
					mRegistry->emplace<AudioEditorComponent>(entity);
				else
					mRegistry->remove<AudioEditorComponent>(entity);
				break;
			}

			case EditorCameraType:
			{
				if (add)
					mRegistry->emplace<CameraEditorComponent>(entity);
				else
					mRegistry->remove<CameraEditorComponent>(entity);
				break;
			}

			case EditorLightType:
			{
				if (add)
					mRegistry->emplace<LightEditorComponent>(entity);
				else
					mRegistry->remove<LightEditorComponent>(entity);
				break;
			}

			case EditorPathNavType:
			{
				if (add)
					mRegistry->emplace<BeaconNavEditorComponent>(entity);
				else
					mRegistry->remove<BeaconNavEditorComponent>(entity);

				break;
			}

			default:
				Log(LGR_ERROR, std::format("Could not determine the type of modiciation for entity #{}", (int)entity));
				break;
		}
	}

	/**
	 * @brief Set up toggle-able editor camera.
	*/
	inline void CreateEditorCamera()
	{
		mInit.LoadEditor(*mEditorRegistry);

		// There doesn't seem to be an easier way to pass this to the game editor scripts.
		DataVault["GameSceneRegistry"] = mRegistry;
		DataVault["PathToConfig"] = PathToConfigFile;
	}
#endif // EDITOR

	/**
	 * @brief Handles the generated events.
	 * @param events List of events to be handled.
	*/
	inline void HandleEvents(YmEventList& events)
	{
		// after deleting data needs to be set to zero, otherwise the destructor check will log it as data existance
		while (events.size() > 0)
		{
			auto evnt = std::move(events.front());
			events.pop_front();

			switch (evnt.mType)
			{
				case DeleteEntity:
				{
					auto* entity = (int*)evnt.mData;
					mRegistry->destroy((entt::entity)*entity);
					delete entity;
					evnt.mData = nullptr;
					break;
				}

				case ExitApplication:
				{
					// Currently it's more like just a bool flag, so just having this is enough to indicate a change and no actual data is passed;
					// If there are flags that behave like this it can be turned into a separate enum that will be stored in th mData;
					mUserExit = true;
					break;
				}

#ifdef GAME
				case ChangeScene:
				{
					auto* message = (SceneChangeMessage*)evnt.mData;
					int sceneId;

					if(std::holds_alternative<int>(*message))
						sceneId = std::get<int>(*message);

					if (std::holds_alternative<std::pair<int, bool>>(*message))
					{
						auto[backUp, goNext] = std::get<std::pair<int, bool>>(*message);

						sceneId = backUp;
						if (goNext)
						{
							auto nextSceneID = mInit.GetNextSceneId();
							if (nextSceneID.has_value())
								sceneId = nextSceneID.value();
						}
					}
#ifdef PROFILING
					mWatch.Start();
#endif // PROFILING

					mInit.LoadSceneById(sceneId, *mRegistry);

#ifdef PROFILING
					auto time = mWatch.SecondsPassed();
					Log(LGR_INFO, std::format("Time spent on loading {} scene : {}s", sceneId, time));
#endif // PROFILING


					delete message;
					evnt.mData = nullptr;
					mLoadNewScene = true;
					break;
				}
#endif // GAME

				case CursorState:
				{
					auto* message = (int*)evnt.mData;

					auto cursorVisibility = !(*message & CursorStates::CursorInvisible);
					mInput.SetCursorVisible(cursorVisibility);
					GUI::Instance().SetCursorState(cursorVisibility);

					auto cursorLockedState = (*message & CursorStates::CurosrLocked);
					mInput.SetCursorLocked(cursorLockedState);

					delete message;
					evnt.mData = nullptr;
					break;
				}


#ifdef EDITOR
				case EditorChange:
				{
					auto* mod = (Modification*)evnt.mData;
					ApplyModifications(*mod);
					delete mod;
					evnt.mData = nullptr;
					break;
				}

				case SceneFileLoad:
				{
					auto* message = (std::string*)evnt.mData;
					mInit.LoadSceneInEditor(*message, *mRegistry);
					delete message;
					evnt.mData = nullptr;
					break;
				}
#endif // EDITOR

				default:
					Log(LGR_ERROR, std::format("Did not handle an event of type {}", (int)evnt.mType));
					break;
			}
		}
	}

	/**
	 * @brief Handles the updates for graphical nodes
	 * @param alpha The interpolation state between the old transform and the new transform state
	 * @param meshObserver Entities with mesh component whose transform was modified
	 * @param cameraObserver Entities with camera component whose transform was modified
	 * @param lightObserver Entities with light component whose transform was modified
	 * @param oldTransforms Map of cached transforms from previous frame for smoother interpolation between physics updates.
	*/
	inline void RenderableObjectHandling(
#ifdef GAME
		float const& alpha
		, entt::observer& meshObserver
		, entt::observer& cameraObserver
		, entt::observer& lightObserver
		, std::unordered_map<entt::entity, Transform>& oldTransforms
#endif // GAME
	)
	{

#ifdef GAME
		for (auto const& entity : meshObserver)
#else
		// Observers can't be used right now as the editor values are changed in the GUI, meaning that in order to use the observer there would be a need to constantly check the values if they have changed.
		for (auto&& [entity, transform, meshComp] : mRegistry->view<Transform, MeshComp>().each())
#endif // EDITOR
		{
#ifdef GAME
			auto& newState = mRegistry->get<Transform>(entity);
			auto& meshComp = mRegistry->get<MeshComp>(entity);
			auto oldState = oldTransforms[entity];
			// Transfer the Transform data from the main component to the mesh data
			mGraphics->UpdateMeshNode(oldState, newState, meshComp, alpha);
#endif // GAME

#ifdef EDITOR
		// Since there is no lag calculation and the alpha would always return 0, and both, old and new, would be the same it would to just extra useless calculation and checking.
			mGraphics->UpdateMeshNode(transform, meshComp);
#endif // EDITOR
		}

#ifdef EDITOR
		for (auto&& [entity, transform ,camera] : mEditorRegistry->view<Transform, CameraComponent>().each())
		{
			mGraphics->UpdateCameraNode(transform, camera);
		}
#endif // EDITOR

#ifdef GAME
		meshObserver.clear();

		for (auto const& entity : cameraObserver)
		{
			auto& cameraComp = mRegistry->get<CameraComponent>(entity);
			auto& newState = mRegistry->get<Transform>(entity);
			auto oldState = oldTransforms[entity];
			mGraphics->UpdateCameraNode(oldState, newState, cameraComp, alpha);
		}
		cameraObserver.clear();

		for (auto const& entity : lightObserver)
		{
			auto& newState = mRegistry->get<Transform>(entity);
			auto& lightComp = mRegistry->get<LightComponent>(entity);
			auto oldState = oldTransforms[entity];
			mGraphics->UpdateLightNode(oldState, newState, lightComp, alpha);
		}
		lightObserver.clear();
#endif // GAME
	};

#ifdef GAME
	/**
	* @brief Call Update part of the game logic scripts.
	*/
	[[nodiscard]] inline YmEventReturn GameScriptsHandling(std::unordered_map<entt::entity, Transform>& oldTransforms)
	{
		YmEventList gameEvents;
		// Game script - the first modification of the data
		for (auto&& [entity, scripts] : mRegistry->view<GameLogic>().each())
		{
			Transform transformPre(mRegistry->get<Transform>(entity));
			auto events = scripts.Update(gameTimeStep, *mRegistry, entity);
			auto& transformPost = mRegistry->get<Transform>(entity);

			if (events.has_value())
				MergeYmEventLists(gameEvents, events.value());
			// This doesn't update physics scale - there is no need yet, but if there was - would multiply the offsets by appropriate axis scale values.
			if (!yamath::opr::Equal(transformPost.mPosition, transformPre.mPosition) ||
				!yamath::opr::Equal(transformPost.mRotation, transformPre.mRotation))
			{
				// Empty Patch just to rise a signal to update, so it is also picked up by the graphics observers.
				mRegistry->patch<Transform>(entity, [](auto& trfm) {});
				oldTransforms[entity] = transformPost;

				// This was done in observer before, but issues:
				// - Initializing the observer in this function caused unknown issues;
				// - If initialized with other observers it would pick up other patches, which was fixed by clearing before the script loop, but this seems like a more complex and expensive approach (While profiling the difference was minimal);
				auto* physicsComp = mRegistry->try_get<PhysicalBody>(entity);
				if(physicsComp)
					physicsComp->SetPositionRotation(transformPost.mPosition, transformPost.mRotation);
			}
		}

		if (gameEvents.size() > 0)
			return gameEvents;
		return {};
	}

	/**
	* @brief Call physics handling sections of the game scripts.
	*/
	[[nodiscard]] inline YmEventReturn PhysicsResponseHandling(std::unordered_map<entt::entity, Transform>& oldTransforms)
	{
		YmEventList gameEvents;
		for (auto&& [entity, transform, physicsComp] : mRegistry->view<Transform, PhysicalBody>().each())
		{
			physicsComp.CalculateCollisionCollections();

			auto* gameLogic = mRegistry->try_get<GameLogic>(entity);
			if (gameLogic)
			{
				auto events = gameLogic->CollisionUpdates(physicsComp, gameTimeStep, *mRegistry);

				if (events.has_value())
					MergeYmEventLists(gameEvents, events.value());
			}

			auto currentPos = physicsComp.GetPosition();
			auto currentRot = physicsComp.GetRotation();

			// This should also take of things (set old transforms) for things that are only moved by scripts.

			if (!yamath::opr::Equal(transform.mPosition, currentPos) ||
				!yamath::opr::Equal(transform.mRotation, currentRot))
			{
				// use entity id in a an unsorted map and then access for copy of old state
				oldTransforms[entity] = transform;
				mRegistry->patch<Transform>(entity, [currentPos, currentRot](auto& trfm)
											{
												trfm.mPosition = currentPos;
												trfm.mRotation = currentRot;
											});
			}

			physicsComp.FlushCollision();
		}

		if (gameEvents.size() > 0)
			return gameEvents;
		return {};
	}

	/**
	 * @brief A fixed time game step. Handles game logic scripts, physics update and calling appropriate collision events.
	 * @param oldTransforms Map for caching transform for the next frame for smoother graphical transition between physics updates.
	 * @param lag Accumulated lag
	 * @param gameEvents
	*/
	[[nodiscard]] inline YmEventReturn GameStep(std::unordered_map<entt::entity, Transform>& oldTransforms, float& lag
#ifdef PROFILING
						 , float& gameScriptHandleTime
						 , float& physicsUpdateTime
						 , float& physicsResponseTime
#endif // PROFILING
														)
	{
		YmEventList gameEvents;
		while (lag >= gameTimeStep)
		{
#ifdef PROFILING
			mWatch.Start();
#endif // PROFILING
			auto gameScriptEvents = GameScriptsHandling(oldTransforms);
			if (gameScriptEvents.has_value())
				MergeYmEventLists(gameEvents, gameScriptEvents.value());

#ifdef PROFILING
			gameScriptHandleTime = mWatch.SecondsPassed();
			mWatch.Start();
#endif // PROFILING

			mPhysics->Update(gameTimeStep);
#ifdef PROFILING
			physicsUpdateTime = mWatch.SecondsPassed();
			mWatch.Start();
#endif // PROFILING

			auto physicsResponseEvents =  PhysicsResponseHandling(oldTransforms);
			if (physicsResponseEvents.has_value())
				MergeYmEventLists(gameEvents, physicsResponseEvents.value());
#ifdef PROFILING
			physicsResponseTime = mWatch.SecondsPassed();
#endif // PROFILING

			mInput.Update();
			lag -= gameTimeStep;
		}

		if (gameEvents.size() > 0)
			return gameEvents;
		return {};
	}

	/**
	 * @brief Inits that need to be called before the game loop.
	 * @param gameEvents The game event queue
	 * @param oldTransforms Map for caching old transforms, that will be used for smoother interpolation between the physics updates.
	*/
	inline void PreGameLoopInits(YmEventList& gameEvents, std::unordered_map<entt::entity, Transform> oldTransforms)
	{

		// Components that have neither the game logic, nor physical bodies, meaning they will not change in position - thus old state will only be set before the game loop

		for (auto&& [entity, transform, _] : mRegistry->view<Transform, CameraComponent>(entt::exclude<PhysicalBody, GameLogic>).each())
		{
			oldTransforms[entity] = transform;
		}

		for (auto&& [entity, transform, _] : mRegistry->view<Transform, LightComponent>(entt::exclude<PhysicalBody, GameLogic>).each())
		{
			oldTransforms[entity] = transform;
		}

		// Update physics based on the transform as the component has no idea about the position
		for (auto&& [entity, transform, physicsComp] : mRegistry->view<Transform, PhysicalBody>().each())
		{
			physicsComp.SetPositionRotation(transform.mPosition, transform.mRotation);
		}

		for (auto&& [entity, scripts] : mRegistry->view<GameLogic>().each())
		{
			auto events = scripts.Start(gameTimeStep, *mRegistry, entity);
			if (events.has_value())
				MergeYmEventLists(gameEvents, events.value());
		}
	}
#endif // GAME

	/**
	 * @brief Prepares the graphics entities for the game loop.
		Refreshes the transforms so that the graphics related observes can react.
		Caches transforms, so that the graphical objects that do not move can still be drawn.
	*/
	inline void PrepareGraphicEntities(
#ifdef GAME
		std::unordered_map<entt::entity, Transform>& oldTransforms
#endif // GAME

	)
	{
		for (auto&& [entity, transform] : mRegistry->view<Transform>().each())
		{
			if (!mRegistry->any_of<MeshComp
#ifdef GAME
				, CameraComponent
				, LightComponent
#endif // GAME
			>(entity))
				continue;

			// Setting the position should be enough to trigger the observer.
			mRegistry->patch<Transform>(entity, [transform](auto& trfm)
										{
											trfm.mPosition = transform.mPosition;
										});

#ifdef GAME
			oldTransforms[entity] = transform;
#endif // GAME

		}
	}
public:

	/**
	 * @brief Initialize the engine and it's components
	 * @param argc Passed in argument from the main()
	 * @param argv Passed in argument from the main()
	*/
	void InitEngine(int argc, char* argv[])
	{
		InitLogger(argc, argv);
		// Initialize from data here
		mInit.LoadConfig(PathToConfigFile);

		mInit.LoadLoggingFiles();
#ifdef PROFILING
		yaprfl::Init();
		mWatch.Start();
#endif // PROFILING


		auto [width, height] = mInit.GetResolution();
		mInput.Init(GUI::Instance().GetImguiEventReceiver(), height, width);

		mGraphics = std::make_unique<GraphicsWrapper>(width, height, mInput.GetHandler());
		GUI::Instance().InitUI(mGraphics->GetDevice(), height, width);
		mInput.SetCursorControl(mGraphics->GetCursorControl());

#ifdef GAME
		Audio::Init();
		mInit.LoadScenesConfigs();
		mPhysics = std::make_unique<PhysicsWrapper>();
		mInit.mPhysics = mPhysics.get();
#endif // GAME
		mInit.mGraphics = mGraphics.get();


		mRegistry = new entt::registry();

#ifdef EDITOR
		mEditorRegistry = new entt::registry();
#endif // EDITOR


#ifdef PROFILING
		auto initTime = mWatch.SecondsPassed();
		Log(LGR_INFO, std::format("Time spent for engine init: {}s", initTime));
		mWatch.Start();
#endif // PROFILING


#ifdef GAME
		mInit.LoadSceneById(1, *mRegistry);
#endif // GAME

#ifdef EDITOR
		CreateEditorCamera();
#endif // EDITOR

#ifdef PROFILING
		auto sceneLoadPassed = mWatch.SecondsPassed();
		Log(LGR_INFO, std::format("Time spent for first scene load: {}s", sceneLoadPassed));
#endif // PROFILING

	}

	/**
	 * @brief Main loop of the game is performed here
	*/
	void GameLoop()
	{
		do
		{
			YmEventList gameEvents;
#pragma region Pre loop
#ifdef GAME
			mLoadNewScene = false;
			auto meshObserver = entt::observer{ *mRegistry, entt::collector.update<Transform>().where<MeshComp>() };
			auto cameraObserver = entt::observer{ *mRegistry, entt::collector.update<Transform>().where<CameraComponent>() };
			auto lightObserver = entt::observer{ *mRegistry, entt::collector.update<Transform>().where<LightComponent>() };
			std::unordered_map<entt::entity, Transform> oldTransforms;
#endif // GAME

			PrepareGraphicEntities(
#ifdef GAME
				oldTransforms
#endif // GAME
			);

			// The loop is inspired by https://gameprogrammingpatterns.com/game-loop.html
			float lag = 0.0f;
			auto previousTime = std::chrono::system_clock::now();

#ifdef GAME
			PreGameLoopInits(gameEvents, oldTransforms);
#endif // GAME


#ifdef EDITOR
			for (auto&& [entity, scripts] : mEditorRegistry->view<GameLogic>().each())
			{
				auto events = scripts.Start(gameTimeStep, *mRegistry, entity);
				if (events.has_value())
					MergeYmEventLists(gameEvents, events.value());
			}
#endif // EDITOR


#pragma endregion

			while (mGraphics->IsDeviceRunning() && !mUserExit && !GUI::Instance().GetMenuQuit())
			{
#pragma region Time Calculations
				// Calculate how much time was produced.
				auto currentTime = std::chrono::system_clock::now();
				std::chrono::duration<float> elapsedTime = currentTime - previousTime;
				previousTime = currentTime;
#ifdef GAME
				lag += elapsedTime.count();
#endif // GAME
#ifdef EDITOR
				auto realtimeDeltaTime = elapsedTime.count();
#endif // EDITOR


#pragma endregion

				HandleEvents(gameEvents);

#ifdef GAME
				if (mLoadNewScene)
				{
					// Was thinking about using goto here, but it doesn't simplify the code, as it would only replace the do-while - the bool flag and the check are still necessary as the event handling happens inside a different function
					break;
				}

#pragma region Game step

#ifdef PROFILING
				float gameScriptHandleTime;
				float physicsUpdateTime;
				float physicsResponseTime;
#endif // PROFILING

				auto gameStepEvents = GameStep(oldTransforms, lag
#ifdef PROFILING
						 ,gameScriptHandleTime, physicsUpdateTime, physicsResponseTime
#endif // PROFILING
				);

				if (gameStepEvents.has_value())
					MergeYmEventLists(gameEvents, gameStepEvents.value());

				// The input update has to be called withing the game step so that the cleared data is not gone between them, as that's the only time the user can interact and handle said inputs
				// While in the engine there is no physics time limitations

#endif // GAME

#ifdef EDITOR
				for (auto&& [entity, scripts] : mEditorRegistry->view<GameLogic>().each())
				{
					auto events = scripts.Update(realtimeDeltaTime, *mEditorRegistry, entity);
					if (events.has_value())
						MergeYmEventLists(gameEvents, events.value());
				}
				// While it is not really expected to use Input::Get... in editor, as most of it is handled within ImGUI (for now), but it could cause headaches if something was using it
				mInput.Update();
#endif // EDITOR

#pragma endregion

#pragma region Rendering

#ifdef PROFILING
				mWatch.Start();
#endif // PROFILING


				auto alpha = lag / gameTimeStep;
				RenderableObjectHandling(
#ifdef GAME
					alpha, meshObserver, cameraObserver, lightObserver, oldTransforms
#endif // GAME
				);

				mGraphics->BeginScene();
				GUI::Instance().StartDrawing();

				// Call any kind of GUI related items from game scripts
				for (auto && [entity, scripts] :
#ifdef GAME
					 mRegistry
#endif // GAME
#ifdef EDITOR
					 mEditorRegistry
#endif
					 ->view<GameLogic>().each())
				{
					auto events = scripts.DrawGraphicalItems();
					if (events.has_value())
						MergeYmEventLists(gameEvents, events.value());
				}

				// Probably make this the update of the scene, which will take the transform comp and graphics comp and then just give update the graphics comp
				mGraphics->DrawScene();
#ifdef PROFILING
				auto drawingTime = mWatch.SecondsPassed();
				GUI::Instance().DrawStats(mGraphics->GetFPS(), mGraphics->GetDrawnPrimitives(), yaprfl::GetUsedMemory(), yaprfl::GetCPUUsage(), drawingTime
#ifdef GAME
										  , gameScriptHandleTime
										  , physicsUpdateTime
										  , physicsResponseTime
#endif // GAME
											);
#endif // PROFILING
				GUI::Instance().StopDrawing();

				mGraphics->EndScene();
#pragma endregion
			}
		} while (
#ifdef GAME
			mLoadNewScene
#else
			false
#endif // GAME
			);
	}

	/**
	 * @brief Clean up the leftovers
	*/
	void Cleanup()
	{
		if (mRegistry)
			delete mRegistry;

#ifdef EDITOR
		if (mEditorRegistry)
			delete mEditorRegistry;
#endif // EDITOR

		GUI::Instance().Cleanup();

#ifdef GAME
		Audio::Cleanup();
#endif // GAME
	}
};