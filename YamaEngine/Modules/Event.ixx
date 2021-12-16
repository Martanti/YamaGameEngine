export module YamaEvents;
export import <optional>;
export import <list>;
export import <variant>;
import Logger;

// Could be replaced by eventpp if something more complex is needed

// The entire event queue could be global object as it would save time returning and merging the items, however due to the notes about how to treat game scripts it should probably be better to return them and then merge them, as otherwise scripts could modify the event list in an unexpected way.
// The event modification could be prevented by making this a friend class of the engine, but unlike graphics and physics there are no real reasons to avoid it right now.

/**
 * @brief Engine event types.
*/
export enum YmEventTypes
{
	Unassigned,
	DeleteEntity,
	ExitApplication,
	CursorState,
#ifdef GAME
	ChangeScene,
#endif // GAME

#ifdef EDITOR
	EditorChange,
	SceneFileLoad,
#endif // EDITOR
	EventTypeCount
};

#ifdef GAME

// Typedef'ed because it might be susceptible to change and be replaced by a struct, class or different variant.
// If only ID/int is passed go to that scene,
// If both are passed, true means go the next scene, if there are no more scenes left go to the ID/int.
// This is done because: a game script cannot import the initializer as it would create a circular dependency. Which could be solved by having some smaller class that is written to by the initializer, but that would require a friend class to preserve exclusive writing rights to initializer.
// This solution is not perfect but for the current scope should work well enough.

/**
 * @brief Scene change message.
 * @note If only ID/int is passed go to that scene,
 * If both are passed, true means go the next scene, if there are no more scenes left go to the ID/int.
*/
export typedef std::variant<int, std::pair<int, bool>> SceneChangeMessage;
#endif // GAME

/**
 * @brief Structure for an event.
 * @note Use to make system wide changes from anywhere in the code.
*/
export struct YmEvent
{
	/**
	 * @brief Stored event type.
	 * @note Used to predict what kind of data is stored within in.
	*/
	YmEventTypes mType;

	// Wanted to use std::any, but it appears that the performance is affected and the types are usually known. In addition shared_ptr adds too much overhead :'/
	//https://devblogs.microsoft.com/cppblog/stdany-how-when-and-why/

	/**
	 * @brief The event data/message.
	*/
	void* mData;

	YmEvent() = default;
	YmEvent(YmEventTypes const& type, void* data) : mType(type), mData(data) {}
	~YmEvent()
	{
		if (mData)
			Log(LGR_WARNING, std::format("The data withing event of type {}, has not been deleted before the destruction of the event this can potentially cause a memory leak.", (int)mType));
	};

	// Copy operators are kept in as the destructor check should point out any issues.
	// Need to redeclare the move functionality due to the declaration of the destructor.

	// Move is highly preferred over copy as mainly due to the speed and the destructor check.

	YmEvent(YmEvent&& move) noexcept
	{
		mType = std::move(move.mType);
		move.mType = Unassigned;

		mData = std::move(move.mData);
		move.mData = nullptr;
	}
	YmEvent& operator=(YmEvent&& move) noexcept
	{
		std::swap(mType, move.mType);
		std::swap(mData, move.mData);
		return *this;
	}
};

// Chose List because insert and remove head/tail are o(1) and should be cheap to merge

/**
 * @brief List of events.
 * @note Use a storage in a function and then return it.
*/
export typedef std::list<YmEvent> YmEventList;

/**
 * @brief Return type. Useful for when to events happened.
*/
export typedef std::optional<YmEventList> YmEventReturn;

/**
 * @brief Merges contents of list b into the list a without sorting.
 * Doesn't copy, just moves the items.
 * Should be used over the std::list::merge() because of the lack of the comparisons.
 * @param a
 * @param b
 * @return
*/
export inline void MergeYmEventLists(YmEventList& a, YmEventList & b)
{
	while (b.size() > 0)
	{
		a.emplace_back(std::move(b.front()));
		b.pop_front();
	}
}

/**
 * @brief Flags for different cursor states.
*/
export enum CursorStates
{
	CursorInvisible = 1 << 0,
	CurosrLocked = 1 << 1
};