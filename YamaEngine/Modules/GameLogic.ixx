module;
#include "entity/registry.hpp";
export module Component.GameLogic;
import GameLogic.Base;
#ifdef GAME
import GameLogic.Player;
import GameLogic.GameFlow;
import GameLogic.ScoreTracker;
import GameLogic.MainMenu;
import GameLogic.ScoreSubmission;
import GameLogic.AIWalker;
import GameLogic.ReactiveElement;
#endif // GAME

#ifdef EDITOR
import GameLogic.EditorCameraManager;
import GameLogic.SceneEditor;
#endif // EDITOR

import <memory>;
import <vector>;
import <string>;
import <functional>;
import <set>;
import YamaEvents;

/**
 * @brief Component responsible for managing many game scripts.
*/
export class GameLogic
{
private:
	/**
	 * @brief The collection of scripts.
	*/
	std::vector<std::unique_ptr<GameScriptBase>> mGameLogicScripts;

#ifdef GAME
	/**
	 * @brief A Pair of 2 lambdas for handling a physics event. First one will fetch a set of entity IDs that the collision and the second one will call appropriate function to handle said physics event.
	*/
	typedef std::pair<std::function<std::set<int>(PhysicalBody&)>, std::function<YmEventReturn(GameScriptBase*, float dt, entt::registry&, entt::entity const&, entt::entity const&)>> CollisionCallPair;

	std::vector<CollisionCallPair> collisionCalls = {
		// On Collision Exit
		std::make_pair(
			[](PhysicalBody& physicalBody) ->std::set<int> { return physicalBody.GetItemsLeftCollisionWith(); },
			[](GameScriptBase* script, float dt, entt::registry& registry, entt::entity const& thisEntity, entt::entity const& thatEntity) -> YmEventReturn { return script->OnCollisionExit(dt, registry, thisEntity, thatEntity); }),

		// On Collision Enter
		std::make_pair(
			[](PhysicalBody& physicalBody) ->std::set<int> { return physicalBody.GetItemsEnterdCollisionWith(); },
			[](GameScriptBase* script, float dt, entt::registry& registry, entt::entity const& thisEntity, entt::entity const& thatEntity) -> YmEventReturn { return script->OnCollisionEnter(dt, registry, thisEntity, thatEntity); }),

		//On Collision Stay
		std::make_pair(
			[](PhysicalBody& physicalBody) ->std::set<int> { return physicalBody.GetItemsStayedCollisionWith(); },
			[](GameScriptBase* script, float dt, entt::registry& registry, entt::entity const& thisEntity, entt::entity const& thatEntity) -> YmEventReturn { return script->OnCollisionStay(dt, registry, thisEntity, thatEntity); })
	};
#endif // GAME

public:


	/**
	 * @brief Calls the start section of the game logic scripts.
	*/
	[[nodiscard]] inline YmEventReturn Start(float deltaTime, entt::registry& registry, entt::entity const& thisEntity)
	{
		YmEventList eventList;
		for (auto& script : mGameLogicScripts)
		{
			auto events = script->Start(deltaTime, registry, thisEntity);
			if (events.has_value())
				MergeYmEventLists(eventList, events.value());
		}

		if (eventList.size() > 0)
			return eventList;

		return {};
	}

	/**
	 * @brief Calls the update section of the game logic scripts.
	*/
	[[nodiscard]] inline YmEventReturn Update(float deltaTime, entt::registry& registry, entt::entity const& thisEntity)
	{
		YmEventList eventList;
		for (auto& script : mGameLogicScripts)
		{
			auto events = script->Update(deltaTime, registry, thisEntity);

			if (events.has_value())
				MergeYmEventLists(eventList, events.value());
		}

		if (eventList.size() > 0)
			return eventList;

		return {};
	}

	/**
	 * @brief Calls the draw section within the game logic scripts.
	*/
	[[nodiscard]] inline YmEventReturn DrawGraphicalItems()
	{
		YmEventList eventList;
		for (auto& script : mGameLogicScripts)
		{
			auto events = script->DrawItems();
			if (events.has_value())
				MergeYmEventLists(eventList, events.value());

		}
		if (eventList.size() > 0)
			return eventList;

		return {};
	}
#ifdef GAME

	/**
	* @brief Calls the collision event functions from game logic scripts.
	*/
	[[nodiscard]] inline YmEventReturn CollisionUpdates(PhysicalBody& physicalBody, float const& deltaTime, entt::registry& registry)
	{
		auto thisEntity = (entt::entity)physicalBody.GetEntityId();
		YmEventList eventList;

		// Going through the 3 different collision event functions
		for (auto& [getItems, executeCollisionFunction] : collisionCalls)
		{
			// Going throigh the list of items that collision events have happened with
			for (auto& thatEntityRaw : getItems(physicalBody))
			{
				// Calling an appropriate collision event handling function
				for (auto& script : mGameLogicScripts)
				{
					auto thatEntity = (entt::entity)thatEntityRaw;
					if (registry.valid(thisEntity) && registry.valid(thatEntity))
					{
						auto events = executeCollisionFunction(script.get(), deltaTime, registry, thisEntity, thatEntity);

						if (events.has_value())
							MergeYmEventLists(eventList, events.value());
					}
				}
			}
		}

		if (eventList.size() > 0)
			return eventList;

		return {};
	}
#endif // GAME

	/**
	 * @brief Adds a script to the collection based on the provided type.
	 * @param type
	*/
	void AddScript(std::string const& type)
	{
#ifdef GAME
		if (type == "PlayerMovement")
		{
			mGameLogicScripts.emplace_back(std::make_unique<PlayerMovement>());
		}
		else if (type == "GameFlowManager")
		{
			mGameLogicScripts.emplace_back(std::make_unique<GameFlowManager>());
		}
		else if (type == "ScoreTracker")
		{
			mGameLogicScripts.emplace_back(std::make_unique<PlayerScoreTracker>());
		}
		else if (type == "ScoreSubmission")
		{
			mGameLogicScripts.emplace_back(std::make_unique<ScoreSubmission>());
		}
		else if (type == "MainMenu")
		{
			mGameLogicScripts.emplace_back(std::make_unique<MainMenu>());
		}
		else if (type == "AIWalker")
		{
			mGameLogicScripts.emplace_back(std::make_unique<AIWalker>());
		}
		else if (type == "Reactive")
		{
			mGameLogicScripts.emplace_back(std::make_unique<ReactiveItem>());
		}
#endif // GAME

#ifdef EDITOR
		if (type == "EditorCamera")
		{
			mGameLogicScripts.emplace_back(std::make_unique<EditorCameraManager>());
		}

		else if (type == "SceneEditor")
		{
			mGameLogicScripts.emplace_back(std::make_unique<SceneEditor>());
		}
#endif // EDITOR
		else
		{
			Log(LGR_ERROR, std::format("Could not recognize script type \"{}\"", type));
		}
	}
};
