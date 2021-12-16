module;
#include "entity/registry.hpp";
import <deque>;
export module GameLogic.Base;
export import YamaEvents;
export import Logger;
export import Input;
export import Component.Transform;
export import <optional>;
export import GUIWrapper;

#ifdef GAME
export import Component.Physics;
#endif // GAME

// This and children classes should be treated as if they were not compiled together with the engine and instead are separate script files, meaning - try to expose to this as little as possibles. (Like Unity C# MonoBehaviour)

/**
 * @brief A base for Game Logic Scripts to inherit from.
*/
export class GameScriptBase
{
public:
	// Has delta time for the situation if it was needed to do some velocity related things.

	/**
	* @brief Start function. Called once before the main game loop.
	*/
	[[nodiscard]] inline virtual YmEventReturn Start(float deltaTime, entt::registry& registry, entt::entity const& thisEntity) { return {}; }

	// If the amount of passed items gets out of the hand - make it into a struct
	// Const as it (the entity) is not expected to be deleted or moved, but the data is free to  be edited (registry)

	/**
	* @brief Update function called every game step at a locked intervals.
	*/
	[[nodiscard]] inline virtual YmEventReturn Update(float deltaTime, entt::registry& registry, entt::entity const& thisEntity) { return {}; }

#ifdef GAME
	/**
	* @brief Handle event when another physics body collides with the owner of this script.
	*/
	[[nodiscard]] inline virtual YmEventReturn OnCollisionEnter(float deltaTimem, entt::registry& registry, entt::entity const& thisEntity, entt::entity const& thatEntity) { return {}; }

	/**
	* @brief Handle event when another physics body stays in a collision with the owner of this script.
	*/
	[[nodiscard]] inline virtual YmEventReturn OnCollisionExit(float deltaTimem, entt::registry& registry, entt::entity const& thisEntity, entt::entity const& thatEntity) { return {}; }

	/**
	* @brief Handle event when another physics body stops colliding with the owner of this script.
	*/
	[[nodiscard]] inline virtual YmEventReturn OnCollisionStay(float deltaTimem, entt::registry& registry, entt::entity const& thisEntity, entt::entity const& thatEntity) { return {}; }
#endif // GAME


	// There was also an idea to have a separate GUI component,
	//but it would also require to be fetched in order to populate with data, this introduces some issues:
	// 1) creating unique GUI component for each purpose would be hell to handle (at the time of writing only 2 are in the plans, but editor and the calling wise, doesn't follow with the architecture that's in place)
	// 2) #1 could be solved by having a wrapper like scripts do, but then a need to call the GUI sub-component would appear, which could be solved by making a templated call and would be somewhat over-engineered for the current problem
	// For current needs it doesn't make that much sense to do all of this

	/**
	 * @brief The function that will be called outside of the game step section.
	 * @note In order to provide graphical consistency GUI items should be called here.
	*/
	[[nodiscard]] inline virtual YmEventReturn DrawItems() { return {}; };
};