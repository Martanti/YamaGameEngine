#ifdef EDITOR
#pragma once
#include "nlohmann/json.hpp"
// Used to have map and variant and store all of the values (variant of float and string) in a map,
// Was replaced with public variables and custom menu call as it made it easy to represent more complex components, and also custom menus!

// Only real reason why this is in a .h & .cpp pair is that JSON lib does not go well with modules.

import <string>;
import Logger;
import <vector>;
import YaMath;


/**
* @brief Represents every editor component.
* Used when in need to differentiate between them or to display the name of the component.
*/
enum EditorComponentTypes
{
	EditorTransformType,
	EditorMeshType,
	EditorColliderType,
	EditorGameScriptType,
	EditorAudioSourcesType,
	EditorCameraType,
	EditorLightType,
	EditorPathNavType
};

/**
 * @brief A base class for a component that will be modified and viewed within the editor mode.
*/
class EditorComponentBase
{
protected:
	using json = nlohmann::json;
public:
	// The name and the path to this enum has to be long so that names do not match with other things.

	// A pair that represents possible value set in the json field. It's set here so it can be used within a dynamic list
	//
	// Used for naming inside UI
	EditorComponentTypes ComponentType;
public:
	/**
	 * @brief Generate a JSON object out of the
	 * @return Object in a JSON form, that can be saved in a file.
	*/
	virtual nlohmann::json GetJsonedObject();

	/**
	 * @brief Populate component values from a JSON object.
	 * @param jObject JSON version of the component.
	*/
	virtual void PopulateFromJson(nlohmann::json const& jObject);

	/**
	 * @brief Get string version of the component type.
	 * @return
	*/
	std::string GetTypeName();

};

/**
 * @brief Represents the transform component which takes care of name, tag, scale, position and the rotation of the entity.
*/
class TransformEditorComponent : public EditorComponentBase
{
public:
	std::string mName = "Object", mTag = "Untagged";

	/**
	 * @brief Object absolute position within the scene.
	*/
	yamath::Vec3D mPos{ 0, 0, 0 };

	/**
	 * @brief Object absolute rotation within the scene.
	*/
	yamath::Vec3D mRot{ 0, 0, 0 };

	/**
	 * @brief Object absolute scale within the scene.
	*/
	yamath::Vec3D mScale{ 1, 1, 1 };
public:
	TransformEditorComponent();
	nlohmann::json GetJsonedObject() override;
	void PopulateFromJson(nlohmann::json const& jObject) override;
};

/**
 * @brief Represents a graphics based component. Is responsible for the mesh, its texture and the animation if one exists.
 * @note By default the last animation is played when in game. So if the object is desired to stay still when created in the scene, the last animation should have starting and ending frames as 0.
*/
class GraphicsEditorComponent : public EditorComponentBase
{
public:
	/**
	 * @brief Structure containing needed information about the animation.
	*/
	struct AnimationData 	// Unlike in normal component data here can be edited, so tuple doesn't make sense
	{
		/**
		 * @brief Desired animation speed that will be set by default.
		*/
		float Speed;

		/**
		 * @brief Starting frame of the animation.
		*/
		int FrameStart;

		/**
		 * @brief The final frame of the animation.
		*/
		int FrameEnd;

		/**
		 * @brief Name of the animation acting as its unique identifier within the scene.
		 * Is used when trying to play the desired animation.
		*/
		std::string Name;
	};

	/**
	 * @brief A collection of animation data.
	*/
	std::vector<AnimationData> mAnimations;

	/**
	 * @brief Path to the mesh file.
	 * @note Currently, only OBJ and FBX files have been tested.
	*/
	std::string mMeshPath = "";

	/**
	 * @brief Path to a texture file.
	 * @note Currently, only BMP files have been tested.
	*/
	std::string mTexturePath = "";
public:
	GraphicsEditorComponent();
	nlohmann::json GetJsonedObject() override;
	void PopulateFromJson(nlohmann::json const& jObject) override;
};

/**
 * @brief Represents physics component, which is used calculating object movement, collisions, etc.
*/
class PhysicsEditorComponent : public EditorComponentBase
{
public:
	/**
	 * @brief Collider type.
	 * @note Currently only "Box" is supported.
	*/
	std::string mColliderType = "Box"; // Might have to replace by an enum, though as this is the only supported type right now it is not worth the hassle to setup a list-box within the menu.

	/**
	 * @brief Mass of the object that will determine it's movements and collisions with other objects.
	 * @note Setting mass to 0 will make object a static one.
	*/
	float mMass = 0.f;

	/**
	 * @brief The cube collider extents.
	*/
	yamath::Vec3D mExtents{ 0, 0, 0 };

	/**
	 * @brief Offset from the center which is the position of the transform component.
	*/
	yamath::Vec3D mOffsets{ 0, 0, 0 };
public:
	PhysicsEditorComponent();
	nlohmann::json GetJsonedObject() override;
	void PopulateFromJson(nlohmann::json const& jObject) override;
};

/**
 * @brief Representing a component holding different many game logic scripts on a single object.
*/
class ScriptsEditorComponent : public EditorComponentBase
{
public:
	// Ideally this would be a list of paths to a LUA (or other language) scripts.

	/**
	 * @brief A list of script names.
	*/
	std::vector<std::string> mScriptContainer;
public:
	ScriptsEditorComponent();
	nlohmann::json GetJsonedObject() override;
	void PopulateFromJson(nlohmann::json const& jObject) override;
};

/**
 * @brief Represents a component which can hold many audio clips.
*/
class AudioEditorComponent : public EditorComponentBase
{
public:

	/**
	 * @brief A list of audio file sources.
	 * @note First string represents a audio source name, which acts like an ID, second string is a path to the mentioned audio file.
	*/
	std::vector<std::pair<std::string, std::string>> mAudioSources;
public:
	AudioEditorComponent();
	nlohmann::json GetJsonedObject() override;
	void PopulateFromJson(nlohmann::json const& jObject) override;
};

/**
 * @brief Represents a camera component.
*/
class CameraEditorComponent : public EditorComponentBase
{
public:

	/**
	 * @brief Camera type.
	*/
	std::string mCamType = "";

	/**
	 * @brief Offset from the main object point, which is represented by a position held within a Transform component.
	*/
	yamath::Vec3D mOffsets{ 0, 0, 0 };
public:
	CameraEditorComponent();
	nlohmann::json GetJsonedObject() override;
	void PopulateFromJson(nlohmann::json const& jObject) override;
};

/**
 * @brief Represents a light component.
*/
class LightEditorComponent : public EditorComponentBase
{
public:
	/**
	 * @brief Red colour component of light diffuse colour.
	*/
	float mDiffuseR = 0;

	/**
	 * @brief Green colour component of light diffuse colour.
	*/
	float mDiffuseG = 0;

	/**
	 * @brief Blue colour component of light diffuse colour.
	*/
	float mDiffuseB = 0;

	/**
	 * @brief Opacity/alpha component of light diffuse colour.
	*/
	float mDiffuseA = 0;

	/**
	 * @brief Red colour component of light ambient colour.
	*/
	float mAmbientR = 0;

	/**
	 * @brief Green colour component of light ambient colour.
	*/
	float mAmbientG = 0;

	/**
	 * @brief Blue colour component of light ambient colour.
	*/
	float mAmbientB = 0;

	/**
	 * @brief Opacity/alpha component of light ambient colour.
	*/
	float mAmbientA = 0;

	/**
	 * @brief Range of the light source.
	*/
	float mRange = 0;

	/**
	 * @brief Light type.
	*/
	std::string mLightType = "Point";
public:
	LightEditorComponent();
	nlohmann::json GetJsonedObject() override;
	void PopulateFromJson(nlohmann::json const& jObject) override;
};

/**
 * @brief Represents a AI Navigation component.
*/
class BeaconNavEditorComponent : public EditorComponentBase
{
public:

	/**
	 * @brief A collection of points within a scene that the AI will go to.
	*/
	std::vector<yamath::Vec3D> mBeacons;
public:
	BeaconNavEditorComponent();
	nlohmann::json GetJsonedObject() override;
	void PopulateFromJson(nlohmann::json const& jObject) override;
};
#endif // EDITOR