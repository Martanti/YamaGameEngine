#ifdef EDITOR
module;
#include "entity/registry.hpp"
export module GameLogic.EditorCameraManager;
import Component.GraphicsComponents;
import GameLogic.Base;

/**
 * @brief A camera controller the camera inside the editor.
*/
export class EditorCameraManager : public GameScriptBase
{
private:
	/**
	 * @brief State flag between camera being controlled or not.
	*/
	bool mControlState = false;

	/**
	 * @brief A time until another control state switch might happen.
	 * @note Since the updates in the editor are called at uncapped intervals, pressing a button once might make a lot of calls and that is why there needs to be cool-down of some kind.
	*/
	const float mTimeBeforeNextSwitch = 0.3f;

	/**
	 * @brief Current time for the control state switch cool-down.
	*/
	float mCurrentTime = 0;
public:
	[[nodiscard]] inline YmEventReturn Update(float deltaTime, entt::registry& registry, entt::entity const& thisEntity) override
	{
		YmEventList eventList;

		bool firstUpdateAfterLock = false; // To prevent that awful jump when switching the states.

		if (mCurrentTime >= mTimeBeforeNextSwitch)
		{
			if (Input::GetKeyPressed(Input::KC_F4))
			{
				mCurrentTime = 0;
				mControlState = !mControlState;
				// Show/Hide cursor.
				if (mControlState)
				{
					firstUpdateAfterLock = true;
					eventList.emplace_back(YmEventTypes::CursorState, new int(CursorStates::CursorInvisible | CursorStates::CurosrLocked));
				}
				else
				{
					eventList.emplace_back(YmEventTypes::CursorState, new int(0));
				}
			}
		}
		else
		{
			mCurrentTime += deltaTime;
		}

		if (!firstUpdateAfterLock && mControlState)
		{
			auto* camera = registry.try_get<CameraComponent>(thisEntity);
			if (camera)
			{
				auto& transform = registry.get<Transform>(thisEntity);
				auto camForward = yamath::opr::FromTupleXYZ(camera->GetCameraForward());
				auto camRight = yamath::opr::FromTupleXYZ(camera->GetCameraRight());

				auto camForwardNorm = yamath::opr::Normalize(camForward);
				auto camRightNorm = yamath::opr::Normalize(camRight);

				if (Input::GetKeyPressed(Input::KC_W))
				{
					transform.mPosition = yamath::opr::Add(transform.mPosition, yamath::opr::Mult(camForwardNorm, deltaTime * 100));
				}

				else if (Input::GetKeyPressed(Input::KC_S))
				{
					transform.mPosition = yamath::opr::Add(transform.mPosition, yamath::opr::Mult(camForwardNorm, deltaTime * -100));
				}

				else if (Input::GetKeyPressed(Input::KC_A))
				{
					transform.mPosition = yamath::opr::Add(transform.mPosition, yamath::opr::Mult(camRightNorm, deltaTime * -100));
				}

				else if (Input::GetKeyPressed(Input::KC_D))
				{
					transform.mPosition = yamath::opr::Add(transform.mPosition, yamath::opr::Mult(camRightNorm, deltaTime * 100));
				}

				else if (Input::GetKeyPressed(Input::KC_E))
				{
					transform.mPosition.Y += deltaTime * 100;
				}

				else if (Input::GetKeyPressed(Input::KC_Q))
				{
					transform.mPosition.Y -= deltaTime * 100;
				}

				auto [mouseXRel, mouseYRel] = Input::GetMousePositionRel();
				if (mouseXRel != 0 || mouseYRel != 0)
				{
					transform.mRotation.Y += mouseXRel * deltaTime * 10000;
					transform.mRotation.X += mouseYRel * deltaTime * 10000;

					if (transform.mRotation.Y >= 360 || transform.mRotation.Y <= -360)
					{
						transform.mRotation.Y = 0;
					}

					if (transform.mRotation.X >= 80)
					{
						transform.mRotation.X = 80;
					}
					else if (transform.mRotation.X <= -80)
					{
						transform.mRotation.X = -80;
					}
				}
			}
		}
		if (eventList.size() > 0)
			return eventList;
		return {};
	}

	[[nodiscard]] inline YmEventReturn DrawItems() override
	{
		GUI::Instance().CameraControlState(mControlState);
		return {};
	};
};
#endif // EDITOR
