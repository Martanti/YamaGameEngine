export module Component.Transform;
import <string>;
import YaMath;
using namespace yamath;
/**
 * @brief Basic transform component.
*/
export struct Transform
{
	/**
	 * @brief The name of the entity.
	*/
	std::string mName;

	/**
	 * @brief A tag for the entity.
	 * @note Used to denote a certain type of object with a more general behavior, e.g. Player, Score, Death.
	*/
	std::string mTag;

	/**
	 * @brief Global position.
	*/
	Vec3D mPosition;

	/**
	 * @brief Global rotation.
	*/
	Vec3D mRotation;

	/**
	 * @brief Global scale.
	*/
	Vec3D mScale;

	// Use to have implementation of the rule of five, but saw this.
	//https://stackoverflow.com/questions/4819936/why-no-default-move-assignment-move-constructor
	// Also removed the getters and setter to make this smaller and lighter.
};