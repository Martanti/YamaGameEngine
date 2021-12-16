#ifdef GAME
module;
export module BeaconNavigation;
import YaMath;
import <vector>;
import Logger;

/**
 * @brief The most basic path-finding implementation. Goes through a list of points within the scene back and forth.
*/
export class BeaconNavigation
{
private:
	/**
	 * @brief Index of the current node that is being targeted.
	*/
	int mIndex = 0;

	/**
	 * @brief Collection of nodes within the scene that the AI actor will go to.
	*/
	std::vector<yamath::Vec3D> mBeacons;
public:
	/**
	 * @brief Create the component.
	 * @param beacons
	*/
	inline BeaconNavigation(std::vector<yamath::Vec3D> beacons) : mBeacons(beacons)
	{}

	/**
	 * @brief Get the coordinates of the currently targeted beacon.
	 * @return
	*/
	inline yamath::Vec3D GetCurrentPoint() const noexcept
	{
		if (mBeacons.size() > 0)
		{
			return mBeacons[mIndex];
		}
		else
		{
			Log(LGR_WARNING, "The navigation beacon is empty, thus 0 vector will be returned as the current point ");
			return { 0, 0, 0 };
		}
	}

	/**
	 * @brief Iterates the target point index by one.
	 * @return
	 * @note When the last node is reached the collection of nodes will be reversed and the index will be set to 0 - resulting in backtracking the initial path.
	*/
	inline void MoveToNextPoint() noexcept
	{
		if (mIndex + 1 >= mBeacons.size())
		{
			std::reverse(mBeacons.begin(), mBeacons.end());
			mIndex = 0;
		}
		else
		{
			mIndex++;
		}
	}
};
#endif // GAME
