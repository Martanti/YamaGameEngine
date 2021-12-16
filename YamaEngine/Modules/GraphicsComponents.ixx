export module Component.GraphicsComponents;
import "irrlicht.h";
import <map>;
import <string>;
import <string_view>;
import <filesystem>;
import YaMath;
import Logger;

using std::string;
/**
 * @brief The component to hold a mesh, its texture and the animation.
*/
export class MeshComp
{
private:
	/**
	 * @brief A reference to the scene node within the graphics engine.
	 * Should only be used internally.
	*/
	irr::scene::IAnimatedMeshSceneNode* mSceneNode;
	/**
	 * @brief Path to the mesh file.
	*/
	std::string mMeshPath;

	/**
	 * @brief Path to the texture file.
	*/
	std::string mTexturePath;

	/**
	 * @brief To track when mesh should be changed.
	*/
	bool mMeshDirtyFlag = false;

	/**
	 * @brief A dirty flag to track the change in texture path.
	*/
	bool mTexturePathDirtyFlag = false;
private:
	/**
	 * @brief Construct a mesh component.
	 * @note Meant to be called only by the
	*/
	inline MeshComp(irr::scene::IAnimatedMeshSceneNode* sceneNode = nullptr, std::string const& meshPath = "", std::string const& texturePath = "") : mSceneNode(sceneNode), mMeshPath(meshPath),
		mTexturePath (texturePath) {}
public:

	/**
	 * @brief Animation data.
	 * @note [Start Frame, End Frame, Animation Speed]
	*/
	typedef std::tuple<int, int, float> AnimationData; // At the moment tuple feels a bit simpler way to handle this than a constructor + range binding is nice.
	std::map<std::string, AnimationData> Animations;

public:
	~MeshComp()
	{
		if (mSceneNode)
			mSceneNode->remove();
	}


	MeshComp(MeshComp const& copy) = delete;
	MeshComp(MeshComp&& move) noexcept :
		mSceneNode(std::exchange(move.mSceneNode, nullptr )),
		mMeshPath(std::move(move.mMeshPath)),
		mTexturePath(std::move(move.mTexturePath)),
		Animations(std::move(move.Animations))
	{}
	MeshComp& operator= (MeshComp const& copy) = delete;
	MeshComp& operator= (MeshComp&& move) noexcept
	{
		mSceneNode = std::exchange(move.mSceneNode, nullptr);
		mMeshPath = std::move(move.mMeshPath);
		mTexturePath = std::move(move.mTexturePath);
		Animations = std::move(move.Animations);
		return *this;
	}

	/**
	 * @brief Set a new path to the mesh file.
	 * @param meshPath
	*/
	inline void SetMeshPath(std::string const& meshPath)
	{
		if(std::filesystem::exists(meshPath))
		{
			mMeshDirtyFlag = true;
			mMeshPath = meshPath;
		}
	}

	/**
	 * @brief Get the path to the mesh file.
	 * @return
	*/
	inline string GetMeshPath()
	{
		return std::string(mMeshPath);
	}

	/**
	 * @brief Set a new path to the texture file.
	 * @param texturePath
	*/
	inline void SetTexturePath(std::string const& texturePath)
	{
		if (std::filesystem::exists(texturePath))
		{
			mTexturePathDirtyFlag = true;
			mTexturePath = texturePath;
		}
	}

	/**
	 * @brief Get current texture file path.
	 * @return
	*/
	inline string GetTexturePath()
	{
		return std::string(mTexturePath);
	}

	/**
	 * @brief Start animation based on its name.
	 * @param animationName
	 * @note Will also set animation speed to the default speed that's stored within the object.
	*/
	inline void SetAnimation(std::string const& animationName)
	{
		if (!mSceneNode)
		{
			Log(LGR_ERROR, "Scene node is nullptr.");
			return;
		}
		auto& [start, finish, speedBuiltin] = Animations[animationName];

		mSceneNode->setFrameLoop(start, finish);
		mSceneNode->setAnimationSpeed(speedBuiltin);
	}

	/**
	 * @brief Set animation speed.
	 * @param speed
	*/
	inline void SetCurrentAnimationSpeed(float const& speed)
	{
		if (!mSceneNode)
		{
			Log(LGR_ERROR, "Scene node is nullptr.");
			return;
		}
		mSceneNode->setAnimationSpeed(speed);
	}

	friend class GraphicsWrapper;
};

/**
 * @brief Camera component.
*/
export class CameraComponent
{
private:
	/**
	 * @brief A reference to the scene node within the graphics engine.
	 * Should only be used internally.
	*/
	irr::scene::ICameraSceneNode* mSceneNode = nullptr;

private:
	CameraComponent() {};

	/**
	 * @brief Get forward vector using graphics engine units.
	 * @return
	*/
	inline irr::core::vector3df GetCameraForwardInternal()
	{
		if (!mSceneNode)
		{
			Log(LGR_ERROR, "Scene node is nullptr.");
			return {0, 0, 0};
		}
		auto& targetPos = mSceneNode->getTarget();
		auto& cameraPos = mSceneNode->getPosition();
		auto dir = targetPos - cameraPos;
		return  dir.normalize();
	}

	/**
	 * @brief Get right vector using graphics engine units.
	 * @return
	*/
	inline irr::core::vector3df GetCameraRightInternal()
	{
		if (!mSceneNode)
		{
			Log(LGR_ERROR, "Scene node is nullptr.");
			return { 0, 0, 0 };
		}
		auto forward = GetCameraForwardInternal();
		auto& up = mSceneNode->getUpVector();

		auto cross = up.crossProduct(forward);

		return cross.normalize();
	}

public:

	/**
	 * @brief Camera type.
	*/
	std::string mCamType = "";

	/**
	 * @brief Offset from the center point, which is the point set by the transform component.
	*/
	yamath::Vec3D mOffset;

public:
	~CameraComponent()
	{
		if (mSceneNode)
			mSceneNode->remove();
	}

	CameraComponent(CameraComponent&& move) noexcept
	{
		std::swap(mSceneNode, move.mSceneNode);
		std::swap(mCamType, move.mCamType);
		mOffset = std::move(move.mOffset);
	}

	CameraComponent& operator=(CameraComponent&& move) noexcept
	{
		std::swap(mSceneNode, move.mSceneNode);
		std::swap(mCamType, move.mCamType);
		mOffset = std::move(move.mOffset);
		return *this;
	}

	/**
	 * @brief Get the camera forward vector.
	 * @return [X, Y, Z]
	*/
	inline std::tuple<float, float, float> GetCameraForward()
	{
		auto forward = GetCameraForwardInternal();
		return { forward.X, forward.Y, forward.Z }; // Doesn't matter if using a Vec3D or tuple
	}

	/**
	 * @brief Get the camera right vector.
	 * @return [X, Y, Z]
	*/
	inline std::tuple<float, float, float> GetCameraRight()
	{
		auto right = GetCameraRightInternal();
		return { right.X , right.Y, right.Z }; // Doesn't matter if using a Vec3D or tuple
	}

	friend class GraphicsWrapper;
};

/**
 * @brief Light component.
*/
export class LightComponent
{
private:
	/**
	 * @brief Light source type.
	*/
	enum LightTypes;

	/**
	 * @brief A reference to the scene node within the graphics engine.
	 * Should only be used internally.
	*/
	irr::scene::ILightSceneNode* mLightNode = nullptr;

private:
	LightComponent(){}
public:
	/**
	 * @brief Light types.
	*/
	enum LightTypes
	{
		PointLight = irr::video::ELT_POINT,
		SpotLight = irr::video::ELT_SPOT,
		DirectionalLight= irr::video::ELT_DIRECTIONAL
	};

public:
	inline LightComponent(LightComponent&& move)
	{
		std::swap(mLightNode, move.mLightNode);
	}

	inline LightComponent& operator=(LightComponent&& move)
	{
		std::swap(mLightNode, move.mLightNode);
		return *this;
	}

	/**
	 * @brief Set a type for light source.
	 * @param type
	*/
	inline void SetType(LightTypes const& type)
	{
		mLightNode->setLightType((irr::video::E_LIGHT_TYPE) type);
	}

	/**
	 * @brief Set the light source range.
	 * @param range
	*/
	inline void SetRange(float range)
	{
		mLightNode->setRadius(range);
	}

	/**
	 * @brief Set light source diffuse colour values.
	 * @param r
	 * @param g
	 * @param b
	 * @param a
	*/
	inline void SetDiffuse(float r, float g, float b, float a)
	{
		mLightNode->getLightData().DiffuseColor = irr::video::SColorf(r, g, b, a);
	}

	/**
	 * @brief Set light source ambient colour values.
	 * @param r
	 * @param g
	 * @param b
	 * @param a
	*/
	inline void SetAmbient(float r, float g, float b, float a)
	{
		mLightNode->getLightData().AmbientColor = irr::video::SColorf(r, g, b, a);
	}

	/**
	 * @brief Convert string'ed version of the light type into an actual type value.
	 * @param typeName
	 * @return
	*/
	inline static LightTypes TypeNameToType(std::string const& typeName)
	{
		if (typeName == "Point")
		{
			return PointLight;
		}
		else if (typeName == "Directional")
		{
			return SpotLight;
		}
		else if (typeName == "Spot")
		{
			return DirectionalLight;
		}
		else
		{
			Log(LGR_ERROR, std::format("Could not determine the light type from string \"{}\"", typeName));
			Log(LGR_WARNING, "By default a point light type will be returned");
			return PointLight;
		}
	}

	friend class GraphicsWrapper;
};