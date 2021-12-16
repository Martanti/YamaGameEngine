// Irrlicht was chosen based on peer recommendation and looked simpler to use, to it's alternative OGRE, based on the tutorials
module;
export module Graphics;
import Logger;
import "irrlicht.h";
import "driverChoice.h";
import Component.Transform;
import Component.GraphicsComponents;
import <optional>;
import "IrrAssimp/IrrAssimp.h";
import YaMath;
using namespace irr;
using namespace video;
using namespace scene;
using namespace yamath::opr;

/**
 * @brief Irrlicht graphics engine wrapper.
*/
export class GraphicsWrapper
{
private:
	/**
	 * @brief The device, which is the window that is displaying the output.
	 * @note The main thing - responsible for everything.
	*/
	IrrlichtDevice* mDevice;

	/**
	 * @brief Video driver.
	*/
	IVideoDriver* mVideDriver;

	/**
	 * @brief Scene manger.
	*/
	ISceneManager* mSceneManager;

	/**
	 * @brief Loader for non default file formats.
	*/
	std::unique_ptr<IrrAssimp> mAssimpLoader;

	/**
	 * @brief Window title.
	*/
	wchar_t const* windowTitle = L"Yama Game Engine - 0.1v"
#ifdef GAME
		" GAME MODE"
#endif //GAME
#ifdef EDITOR
		" EDITOR MODE"
#endif // EDITOR
#ifdef PROFILING
		" [PROFILING ENABLED]"
#endif // PROFILING
		;
public:

	GraphicsWrapper(irr::u32 const& width, irr::u32 const& height, void* eventReceiverRaw = nullptr ,bool askForDriver = false)
	{
		auto eventReceiver =(irr::IEventReceiver*)eventReceiverRaw;
		Log(Verbosity::LGR_INFO, "Initializing graphics engine...");
		auto driver = askForDriver ? driverChoiceConsole() : video::E_DRIVER_TYPE::EDT_DIRECT3D9;
		AssertNotEqual(driver, EDT_COUNT, "A non-valid driver has been selected");

		irr::SIrrlichtCreationParameters param;
		param.DriverType = driver;
		param.EventReceiver = eventReceiver;
		param.Stencilbuffer = true;
		param.WindowSize = core::dimension2d<u32>(width, height);
		param.Bits = 32;
		param.AntiAlias = 16;
		mDevice = irr::createDeviceEx(param);
		AssertNotNull(mDevice, "Yama engine was unable to create the graphic's mDevice");

		mDevice->setWindowCaption(windowTitle);

		mVideDriver = mDevice->getVideoDriver();
		AssertNotNull(mVideDriver);

		mSceneManager = mDevice->getSceneManager();
		AssertNotNull(mSceneManager);

		mAssimpLoader = std::make_unique<IrrAssimp>(mSceneManager);

		mSceneManager->setShadowColor(video::SColor(130, 0, 0, 0));
		Log(Verbosity::LGR_INFO, "Graphics engine is running");
	}

	~GraphicsWrapper()
	{
		if (mDevice)
		{
			mDevice->closeDevice();
			mDevice->drop();
		}
	}

	/**
	 * @brief Check if the video device is still running and thus the program should continue.
	*/
	inline bool IsDeviceRunning()
	{
		return  mDevice->run();
	}

	/**
	 * @brief Call this before drawing anything else. This will inform the graphics engine to start drawing items.
	*/
	inline void BeginScene()
	{
		mVideDriver->beginScene();
	}

	/**
	 * @brief Call this after all of the drawing is done
	*/
	inline void EndScene()
	{
		mVideDriver->endScene();
	}


	//TODO: Check if stuff outside of the camera is drawn - if yes do individual draws
	inline void DrawScene()
	{
		// Probably only go through the list of components and then draw active objects
		mSceneManager->drawAll();
	}

	/**
	 * @brief Get the FPS counter provided by the Graphics engine internals.
	*/
	inline s32 GetFPS()
	{
		return mVideDriver->getFPS();
	}

	/**
	 * @brief Get object for controlling the states of the mouse cursor.
	 * @return
	*/
	inline void* GetCursorControl()
	{
		return mDevice->getCursorControl();
	}

	/**
	 * @brief Initializes static mesh component in the scene and then returns it.
	 * @param meshPath Path to the mesh (.obj file, or some others).
	 * @return Initialized component.
	*/
	inline std::optional<MeshComp> AddMesh(std::string const& meshPath, std::string const& texturePath, bool const acceptEmpty = false)
	{
		//auto mesh = mSceneManager->getMesh(meshPath.c_str());
		auto mesh = mAssimpLoader->getMesh(meshPath.c_str());

		if (mesh)
		{
			Log(Verbosity::LGR_INFO, std::format("Mesh at {} was loaded successfully!", meshPath));
		}
		else if (!acceptEmpty && meshPath.size() == 0)
		{
			return MeshComp(nullptr, "");
		}
		else
		{
			Log(LGR_ERROR, std::format("Could not load mesh at {}", meshPath));
			return {};
		}

		auto node =  mSceneManager->addAnimatedMeshSceneNode(mesh);
		node->setAnimationSpeed(mesh->getAnimationSpeed()); // Not sure why to keep if it's set in the config file, but keeping it for now as it was part of the demo for the irrassimp

#ifdef _DEBUG
		// Very useful for checking where the mesh could be
		//node->setDebugDataVisible(scene::EDS_SKELETON | scene::EDS_BBOX_ALL);
#endif // DEBUG

#ifdef EDITOR
		node->setMaterialFlag(EMF_LIGHTING, false);
#endif // EDITOR

		// Useful
		//node->setMaterialFlag(EMF_WIREFRAME, true);

		if (!texturePath.empty())
		{
			node->setMaterialTexture(0, mVideDriver->getTexture(texturePath.c_str()));
		}
#ifdef GAME
		node->setMaterialFlag(video::EMF_FOG_ENABLE, true);
		node->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
#endif // GAME

		return MeshComp(node, meshPath, texturePath);
	}

	/**
	 * @brief Get number of primitives that are currently drawn from the graphics engine internals.
	*/
	inline u32 GetDrawnPrimitives()
	{
		return mVideDriver->getPrimitiveCountDrawn();
	}

	/**
	 * @brief Update the mesh node with data coming from game scripts or physics engine.
	*/
#ifdef GAME
	inline void UpdateMeshNode(Transform const& oldState, Transform const& newState, MeshComp& meshComp, float const& alpha)
#else
	inline void UpdateMeshNode(Transform const& currentState, MeshComp& meshComp)
#endif // GAME
	{
		// Updating the mesh
		auto& node = meshComp.mSceneNode; // An alias to a ptr.
		if (meshComp.mMeshDirtyFlag)
		{
			auto const& meshPath = meshComp.mMeshPath;
			auto mesh = mAssimpLoader->getMesh(meshPath.data());

			if (mesh)
			{
				if(node)
				{
					node->setMesh(mesh);
					// Newly added items usually just start animating.
					meshComp.SetCurrentAnimationSpeed(0);
				}
				else
				{
					node = mSceneManager->addAnimatedMeshSceneNode(mesh);
					node->setMaterialFlag(EMF_LIGHTING, false);
				}
				Log(Verbosity::LGR_INFO, std::format("Mesh at {} was loaded successfully!", meshPath));
			}
			else
			{
				Log(LGR_ERROR, std::format("Could not load mesh at {}", meshPath));
			}

			// Handled the change
			meshComp.mMeshDirtyFlag = false;
		}

		if (!meshComp.mSceneNode)
			return;

		if (meshComp.mTexturePathDirtyFlag)
		{
			node->setMaterialTexture(0, mVideDriver->getTexture(meshComp.mTexturePath.data()));
			meshComp.mTexturePathDirtyFlag = false;
		}

		// Updating the transform

#ifdef GAME
		// Maybe make it into a macro since this is used in other components as well
		auto currentPos = Lerp(newState.mPosition, oldState.mPosition, alpha);
		auto currentRot = Lerp(newState.mRotation, oldState.mRotation, alpha);
		auto currentScale = Lerp(newState.mScale, oldState.mScale, alpha);
#else
		auto& currentPos = currentState.mPosition;
		auto& currentRot = currentState.mRotation;
		auto& currentScale = currentState.mScale;

#endif // GAME

		//Log(LGR_INFO, yamath::opr::ToStringASL(currentRot));

		meshComp.mSceneNode->setPosition({currentPos.X, currentPos.Y, currentPos.Z});
		meshComp.mSceneNode->setRotation({ currentRot.X, currentRot.Y, currentRot.Z });
		meshComp.mSceneNode->setScale({ currentScale.X, currentScale.Y, currentScale.Z });

		auto rot =  meshComp.mSceneNode->getRotation();
	}

	/**
	 * @brief Get the Irrlicht device.
	 * @note This function is needed mainly for IrrImGui.
	 * @return
	*/
	inline void* GetDevice()
	{
		return mDevice;
	}

	/**
	 * @brief Add basic FPS camera to the scene.
	 * @return
	*/
	inline CameraComponent AddFPSCamera()
	{
		CameraComponent cam;

		auto *node = mSceneManager->addCameraSceneNodeFPS();
		cam.mCamType = "FPS";
		cam.mSceneNode = node;
		return cam;
	};

	/**
	 * @brief Add classic camera to the scene.
	 * @return
	*/
	inline CameraComponent AddCamera()
	{
		CameraComponent cam;
		auto* node = mSceneManager->addCameraSceneNode();
		cam.mCamType = "Classic";
		cam.mSceneNode = node;
		//https://irrlicht.sourceforge.io/forum/viewtopic.php?t=5265
		auto* child = mSceneManager->addEmptySceneNode(node);
		child->setPosition({ 0,0,1 });
		return cam;
	}

#ifdef GAME
	/**
	 * @brief Update the camera object with the data coming from game scripts or physics engine.
	 * @param oldState
	 * @param newState
	 * @param camera
	 * @param alpha
	*/
	inline void UpdateCameraNode(Transform const& oldState, Transform const& newState, CameraComponent& camera, float const& alpha)
	{
		auto currentPos = Lerp(newState.mPosition, oldState.mPosition, alpha);
		auto currentScale = Lerp(newState.mScale, oldState.mScale, alpha);

		currentPos = Add(currentPos, camera.mOffset);

		camera.mSceneNode->setPosition({ currentPos.X, currentPos.Y, currentPos.Z });

		// When using an Irrlciht FPS camera, the rotation is set internally.
		if(camera.mCamType != "FPS")
		{
			auto currentRot = Lerp(newState.mRotation, oldState.mRotation, alpha);
			camera.mSceneNode->setRotation({ currentRot.X, currentRot.Y, currentRot.Z });
		}

		camera.mSceneNode->setScale({ currentScale.X, currentScale.Y, currentScale.Z });
	}

	/**
	 * @brief Add light node to the scene.
	 * @return
	*/
	inline LightComponent AddLight()
	{
		LightComponent light;
		auto* lightNode = mSceneManager->addLightSceneNode(0, core::vector3df(0, 0, 0), video::SColorf(0.f, 0.f, 0.f, 0.f), 0.0f);
		light.mLightNode = lightNode;
		return light;
	}

	/**
	 * @brief Update the light object with the data coming from either game scripts or physics engine.
	 * @param oldState
	 * @param newState
	 * @param light
	 * @param alpha
	*/
	inline void UpdateLightNode(Transform const& oldState, Transform const& newState, LightComponent& light, float const& alpha)
	{
		auto currentPos = Lerp(newState.mPosition, oldState.mPosition, alpha);
		auto currentRot = Lerp(newState.mRotation, oldState.mRotation, alpha);
		auto currentScale = Lerp(newState.mScale, oldState.mScale, alpha);

		light.mLightNode->setPosition({ currentPos.X, currentPos.Y, currentPos.Z });
		light.mLightNode->setRotation({ currentRot.X, currentRot.Y, currentRot.Z });
		light.mLightNode->setScale({ currentScale.X, currentScale.Y, currentScale.Z });
	}
#endif // GAME

#ifdef EDITOR
	// Easier to separate the entire function rather than butcher it.
	/**
	 * @brief Update the camera node coming from the editor scripts.
	 * @param transform
	 * @param camera
	*/
	inline void UpdateCameraNode(Transform const& transform, CameraComponent& camera)
	{
		camera.mSceneNode->setPosition({ transform.mPosition.X,  transform.mPosition.Y,  transform.mPosition.Z });
		camera.mSceneNode->setRotation({ transform.mRotation.X, transform.mRotation.Y, transform.mRotation.Z });
		camera.mSceneNode->setScale({ transform.mScale.X, transform.mScale.Y, transform.mScale.Z });
		camera.mSceneNode->updateAbsolutePosition();
		auto children = camera.mSceneNode->getChildren();
		if (children.size() > 0)
		{
			auto child = children.getLast();
			(*child)->updateAbsolutePosition();
			auto rel = (*child)->getPosition();
			auto pos = (*child)->getAbsolutePosition();
			camera.mSceneNode->setTarget(pos);
		}
	}
#endif // EDITOR
};