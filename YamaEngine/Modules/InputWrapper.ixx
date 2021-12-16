module;
export module Input;
import Logger;
import "irrlicht.h";
import <unordered_set>;
import <tuple>;

// Already used:
// Gainput - mouse support was great, couldn't get consistant data for keyboard inputs;
// OIS - initally - keyboard worked fine, but then mouse cursor was gone;
// Was finally scrapped as it was registering key release function
// Finally decided to base my own or Irrlicht as it was tested before,
// But it will still stay in the same container for modularity purposes
// And it needs to combine my own and IrrImGui event receiver as ImGui needs to find it's own events as well


/**
 * @brief Namespace containing all of the Input related functions.
*/
namespace Input
{
	// This was necessary, as using .Char on the KeyInput wouldn't give the correct letter and would return just 0,
	// So for simplicity the Irrlicht keycodes will be translated
	// Commented out keys that are not expected to have use right now for less work.

	/**
	 * @brief Key codes for the keyboard and mouse inputs.
	*/
	enum Keys
	{
		KC_Mouse_Left =  irr::KEY_LBUTTON,  // Left mouse button
		KC_Mouse_Right =  irr::KEY_RBUTTON,  // Right mouse button
		//KC_ =  irr::KEY_CANCEL = 0x03,  // Control-break processing
		KC_Mouse_Middle =  irr::KEY_MBUTTON,  // Middle mouse button (three-button mouse)
		//KC_ =  irr::KEY_XBUTTON1 = 0x05,  // Windows 2000/XP: X1 mouse button
		//KC_ =  irr::KEY_XBUTTON2 = 0x06,  // Windows 2000/XP: X2 mouse button
		//KC_ =  irr::KEY_BACK = 0x08,  // BACKSPACE key
		//KC_ =  irr::KEY_TAB = 0x09,  // TAB key
		//KC_ =  irr::KEY_CLEAR = 0x0C,  // CLEAR key
		//KC_ =  irr::KEY_RETURN = 0x0D,  // ENTER key
		KC_Shift =  irr::KEY_SHIFT,  // SHIFT key
		KC_CTRL =  irr::KEY_CONTROL,  // CTRL key
		KC_ALT =  irr::KEY_MENU,  // ALT key
		//KC_ =  irr::KEY_PAUSE = 0x13,  // PAUSE key
		//KC_ =  irr::KEY_CAPITAL = 0x14,  // CAPS LOCK key
		//KC_ =  irr::KEY_KANA = 0x15,  // IME Kana mode
		//KC_ =  irr::KEY_HANGUEL = 0x15,  // IME Hanguel mode (maintained for compatibility use KEY_HANGUL)
		//KC_ =  irr::KEY_HANGUL = 0x15,  // IME Hangul mode
		//KC_ =  irr::KEY_JUNJA = 0x17,  // IME Junja mode
		//KC_ =  irr::KEY_FINAL = 0x18,  // IME final mode
		//KC_ =  irr::KEY_HANJA = 0x19,  // IME Hanja mode
		//KC_ =  irr::KEY_KANJI = 0x19,  // IME Kanji mode
		KC_ESC =  irr::KEY_ESCAPE,  // ESC key
		//KC_ =  irr::KEY_CONVERT = 0x1C,  // IME convert
		//KC_ =  irr::KEY_NONCONVERT = 0x1D,  // IME nonconvert
		//KC_ =  irr::KEY_ACCEPT = 0x1E,  // IME accept
		//KC_ =  irr::KEY_MODECHANGE = 0x1F,  // IME mode change request
		KC_Space =  irr::KEY_SPACE,  // SPACEBAR
		//KC_ =  irr::KEY_PRIOR = 0x21,  // PAGE UP key
		//KC_ =  irr::KEY_NEXT = 0x22,  // PAGE DOWN key
		//KC_ =  irr::KEY_END = 0x23,  // END key
		//KC_ =  irr::KEY_HOME = 0x24,  // HOME key
		KC_Arrow_Left =  irr::KEY_LEFT,  // LEFT ARROW key
		KC_Arrow_Up =  irr::KEY_UP,  // UP ARROW key
		KC_Arrow_Right =  irr::KEY_RIGHT,  // RIGHT ARROW key
		KC_Arrow_Down =  irr::KEY_DOWN,  // DOWN ARROW key
		//KC_ =  irr::KEY_SELECT = 0x29,  // SELECT key
		//KC_ =  irr::KEY_PRINT = 0x2A,  // PRINT key
		//KC_ =  irr::KEY_EXECUT = 0x2B,  // EXECUTE key
		//KC_ =  irr::KEY_SNAPSHOT = 0x2C,  // PRINT SCREEN key
		//KC_ =  irr::KEY_INSERT = 0x2D,  // INS key
		//KC_ =  irr::KEY_DELETE = 0x2E,  // DEL key
		//KC_ =  irr::KEY_HELP = 0x2F,  // HELP key
		KC_0 =  irr::KEY_KEY_0,  // 0 key
		KC_1 =  irr::KEY_KEY_1,  // 1 key
		KC_2 =  irr::KEY_KEY_2,  // 2 key
		KC_3 =  irr::KEY_KEY_3,  // 3 key
		KC_4 =  irr::KEY_KEY_4,  // 4 key
		KC_5 =  irr::KEY_KEY_5,  // 5 key
		KC_6 =  irr::KEY_KEY_6,  // 6 key
		KC_7 =  irr::KEY_KEY_7,  // 7 key
		KC_8 =  irr::KEY_KEY_8,  // 8 key
		KC_9 =  irr::KEY_KEY_9,  // 9 key
		KC_A =  irr::KEY_KEY_A,  // A key
		KC_B =  irr::KEY_KEY_B,  // B key
		KC_C =  irr::KEY_KEY_C,  // C key
		KC_D =  irr::KEY_KEY_D,  // D key
		KC_E =  irr::KEY_KEY_E,  // E key
		KC_F =  irr::KEY_KEY_F,  // F key
		KC_G =  irr::KEY_KEY_G,  // G key
		KC_H =  irr::KEY_KEY_H,  // H key
		KC_I =  irr::KEY_KEY_I,  // I key
		KC_J =  irr::KEY_KEY_J,  // J key
		KC_K =  irr::KEY_KEY_K,  // K key
		KC_L =  irr::KEY_KEY_L,  // L key
		KC_M =  irr::KEY_KEY_M,  // M key
		KC_N =  irr::KEY_KEY_N,  // N key
		KC_O =  irr::KEY_KEY_O,  // O key
		KC_P =  irr::KEY_KEY_P,  // P key
		KC_Q =  irr::KEY_KEY_Q,  // Q key
		KC_R =  irr::KEY_KEY_R,  // R key
		KC_S =  irr::KEY_KEY_S,  // S key
		KC_T =  irr::KEY_KEY_T,  // T key
		KC_U =  irr::KEY_KEY_U,  // U key
		KC_V =  irr::KEY_KEY_V,  // V key
		KC_W =  irr::KEY_KEY_W,  // W key
		KC_X =  irr::KEY_KEY_X,  // X key
		KC_Y =  irr::KEY_KEY_Y,  // Y key
		KC_Z =  irr::KEY_KEY_Z,  // Z key
		//KC_ =  irr::KEY_LWIN = 0x5B,  // Left Windows key (Microsoft� Natural� keyboard)
		//KC_ =  irr::KEY_RWIN = 0x5C,  // Right Windows key (Natural keyboard)
		//KC_ =  irr::KEY_APPS = 0x5D,  // Applications key (Natural keyboard)
		//KC_ =  irr::KEY_SLEEP = 0x5F,  // Computer Sleep key
		//KC_ =  irr::KEY_NUMPAD0 = 0x60,  // Numeric keypad 0 key
		//KC_ =  irr::KEY_NUMPAD1 = 0x61,  // Numeric keypad 1 key
		//KC_ =  irr::KEY_NUMPAD2 = 0x62,  // Numeric keypad 2 key
		//KC_ =  irr::KEY_NUMPAD3 = 0x63,  // Numeric keypad 3 key
		//KC_ =  irr::KEY_NUMPAD4 = 0x64,  // Numeric keypad 4 key
		//KC_ =  irr::KEY_NUMPAD5 = 0x65,  // Numeric keypad 5 key
		//KC_ =  irr::KEY_NUMPAD6 = 0x66,  // Numeric keypad 6 key
		//KC_ =  irr::KEY_NUMPAD7 = 0x67,  // Numeric keypad 7 key
		//KC_ =  irr::KEY_NUMPAD8 = 0x68,  // Numeric keypad 8 key
		//KC_ =  irr::KEY_NUMPAD9 = 0x69,  // Numeric keypad 9 key
		//KC_ =  irr::KEY_MULTIPLY = 0x6A,  // Multiply key
		//KC_ =  irr::KEY_ADD = 0x6B,  // Add key
		//KC_ =  irr::KEY_SEPARATOR = 0x6C,  // Separator key
		//KC_ =  irr::KEY_SUBTRACT = 0x6D,  // Subtract key
		//KC_ =  irr::KEY_DECIMAL = 0x6E,  // Decimal key
		//KC_ =  irr::KEY_DIVIDE = 0x6F,  // Divide key
		KC_F1 =  irr::KEY_F1,  // F1 key
		KC_F2 =  irr::KEY_F2,  // F2 key
		KC_F3 =  irr::KEY_F3,  // F3 key
		KC_F4 =  irr::KEY_F4,  // F4 key
		KC_F5 =  irr::KEY_F5,  // F5 key
		KC_F6 =  irr::KEY_F6,  // F6 key
		KC_F7 =  irr::KEY_F7,  // F7 key
		KC_F8 =  irr::KEY_F8,  // F8 key
		KC_F9 =  irr::KEY_F9,  // F9 key
		KC_F10 =  irr::KEY_F10,  // F10 key
		KC_F11 =  irr::KEY_F11,  // F11 key
		KC_F12 =  irr::KEY_F12,  // F12 key
		//KC_ =  irr::KEY_F13 = 0x7C,  // F13 key
		//KC_ =  irr::KEY_F14 = 0x7D,  // F14 key
		//KC_ =  irr::KEY_F15 = 0x7E,  // F15 key
		//KC_ =  irr::KEY_F16 = 0x7F,  // F16 key
		//KC_ =  irr::KEY_F17 = 0x80,  // F17 key
		//KC_ =  irr::KEY_F18 = 0x81,  // F18 key
		//KC_ =  irr::KEY_F19 = 0x82,  // F19 key
		//KC_ =  irr::KEY_F20 = 0x83,  // F20 key
		//KC_ =  irr::KEY_F21 = 0x84,  // F21 key
		//KC_ =  irr::KEY_F22 = 0x85,  // F22 key
		//KC_ =  irr::KEY_F23 = 0x86,  // F23 key
		//KC_ =  irr::KEY_F24 = 0x87,  // F24 key
		//KC_ =  irr::KEY_NUMLOCK = 0x90,  // NUM LOCK key
		//KC_ =  irr::KEY_SCROLL = 0x91,  // SCROLL LOCK key
		//KC_ =  irr::KEY_LSHIFT = 0xA0,  // Left SHIFT key
		//KC_ =  irr::KEY_RSHIFT = 0xA1,  // Right SHIFT key
		//KC_ =  irr::KEY_LCONTROL = 0xA2,  // Left CONTROL key
		//KC_ =  irr::KEY_RCONTROL = 0xA3,  // Right CONTROL key
		//KC_ =  irr::KEY_LMENU = 0xA4,  // Left MENU key
		//KC_ =  irr::KEY_RMENU = 0xA5,  // Right MENU key
		//KC_ =  irr::KEY_OEM_1 = 0xBA,  // for US    ";:"
		//KC_ =  irr::KEY_PLUS = 0xBB,  // Plus Key   "+"
		//KC_ =  irr::KEY_COMMA = 0xBC,  // Comma Key  ","
		//KC_ =  irr::KEY_MINUS = 0xBD,  // Minus Key  "-"
		//KC_ =  irr::KEY_PERIOD = 0xBE,  // Period Key "."
		//KC_ =  irr::KEY_OEM_2 = 0xBF,  // for US    "/?"
		//KC_ =  irr::KEY_OEM_3 = 0xC0,  // for US    "`~"
		//KC_ =  irr::KEY_OEM_4 = 0xDB,  // for US    "[{"
		//KC_ =  irr::KEY_OEM_5 = 0xDC,  // for US    "\|"
		//KC_ =  irr::KEY_OEM_6 = 0xDD,  // for US    "]}"
		//KC_ =  irr::KEY_OEM_7 = 0xDE,  // for US    "'""
		//KC_ =  irr::KEY_OEM_8 = 0xDF,  // None
		//KC_ =  irr::KEY_OEM_AX = 0xE1,  // for Japan "AX"
		//KC_ =  irr::KEY_OEM_102 = 0xE2,  // "<>" or "\|"
		//KC_ =  irr::KEY_ATTN = 0xF6,  // Attn key
		//KC_ =  irr::KEY_CRSEL = 0xF7,  // CrSel key
		//KC_ =  irr::KEY_EXSEL = 0xF8,  // ExSel key
		//KC_ =  irr::KEY_EREOF = 0xF9,  // Erase EOF key
		//KC_ =  irr::KEY_PLAY = 0xFA,  // Play key
		//KC_ =  irr::KEY_ZOOM = 0xFB,  // Zoom key
		//KC_ =  irr::KEY_PA1 = 0xFD,  // PA1 key
		//KC_ =  irr::KEY_OEM_CLEAR = 0xFE,   // Clear key
		KC_Code_Count =  irr::KEY_KEY_CODES_COUNT // this is not a key, but the amount of keycodes there are.
	};

	/**
	 * @brief Currently pressed keys.
	*/
	std::unordered_set <Keys> pressedKeys;

	/**
	 * @brief Flag for clicked right mouse button.
	*/
	bool rmbClicked = false;

	/**
	 * @brief Flag for clicked light mouse button.
	*/
	bool lmbClicked = false;

	/**
	 * @brief Flag for clicked middle mouse button.
	*/
	bool mmbClicked = false;

	/**
	 * @brief Relative Mouse position on X axis.
	*/
	float mouseXRel = 0;

	/**
	 * @brief Relative Mouse position on Y axis.
	*/
	float mouseYRel = 0;

	/**
	 * @brief Mouse Scroll wheel moved amount.
	*/
	float mmbScroll = 0;

	/**
	 * @brief Custom input event handler for Irrlicht.
	*/
	class InputEventHandler : public irr::IEventReceiver
	{
	private:
		/**
		 * @brief Used for IrrImGui Input handling.
		*/
		irr::IEventReceiver*  mAdditionalHandler;

		/**
		 * @brief Cursor state control.
		*/
		irr::gui::ICursorControl* mCursorControl;

		/**
		 * @brief Screen Width.
		*/
		unsigned int mScreenWidth;

		/**
		 * @brief Screen Height.
		*/
		unsigned int mScreenHeight;

		/**
		 * @brief State flag for mouse cursor being locked.
		*/
		bool mLockedCursor = false;

		/**
		 * @brief State flag for the first event handling after locking the cursor.
		*/
		bool mFirstAfterLock = false;
	public:
		InputEventHandler() : mAdditionalHandler(nullptr) {};
		InputEventHandler(irr::IEventReceiver* additionalEventHandler, unsigned int screenHieght, unsigned int screenWidth) : mAdditionalHandler(additionalEventHandler), mScreenWidth(screenWidth), mScreenHeight(screenHieght) {};

		/**
		 * @brief Add cursor control.
		 * @param cursorControl
		*/
		inline void AddCursorControl(irr::gui::ICursorControl* cursorControl)
		{
			mCursorControl = (cursorControl);
		}

		/**
		 * @brief Change cursor lock state.
		 * @param lock
		*/
		inline void LockCursor(bool lock)
		{
			mFirstAfterLock = !mLockedCursor && lock;

			// Even though there is the set position in the input event manager, the relative position data is still the same and therefore will allow jump to happen.
			if (mFirstAfterLock)
			{
				mouseXRel = 0;
				mouseYRel = 0;
			}
			mLockedCursor = lock;
		}

		/**
		 * @brief Event handling.
		 * @param event
		 * @return
		*/
		virtual bool OnEvent(irr::SEvent const& event) override
		{
			if(mAdditionalHandler)
				mAdditionalHandler->OnEvent(event);

			switch (event.EventType)
			{
				using enum irr::EEVENT_TYPE;

				case EET_KEY_INPUT_EVENT:
				{
					auto pressed = (Keys)(event.KeyInput.Key);
					if (event.KeyInput.PressedDown)
					{
						Input::pressedKeys.insert(pressed);
					}
					else
					{
						Input::pressedKeys.erase(pressed);
					}
				}

				case EET_MOUSE_INPUT_EVENT:
				{
					switch (event.MouseInput.Event)
					{
						using enum irr::EMOUSE_INPUT_EVENT;
						// Left mouse button
						case EMIE_LMOUSE_PRESSED_DOWN:
						{
							lmbClicked = true;
							break;
						}

						case EMIE_LMOUSE_LEFT_UP:
						{
							lmbClicked = false;
							break;
						}

						// Right mouse button

						case EMIE_RMOUSE_PRESSED_DOWN:
						{
							rmbClicked = true;
							break;
						}

						case EMIE_RMOUSE_LEFT_UP:
						{
							rmbClicked = false;
							break;
						}

						// Middle mouse button

						case EMIE_MMOUSE_PRESSED_DOWN:
						{
							mmbClicked = true;
							break;
						}

						case EMIE_MMOUSE_LEFT_UP:
						{
							mmbClicked = false;
							break;
						}

						case EMIE_MOUSE_WHEEL:
						{
							// Every frames needs resetting so that it doesn't get stuck on one of these
							mmbScroll = event.MouseInput.Wheel;
							break;
						}

						// Mouse general
						case EMIE_MOUSE_MOVED:
						{
							//This is heavily based on CSceneNodeAnimatorCameraFPS definition in CSceneNodeAnimatorCameraFPS.cpp
							if (mCursorControl)
							{
								if (mFirstAfterLock)
								{
									mCursorControl->setPosition(0.5f, 0.5f);
									mFirstAfterLock = false;
								}
								auto relative = mCursorControl->getRelativePosition();
								mouseXRel = (relative.X - 0.5f) * 2;
								mouseYRel = (relative.Y - 0.5f) * 2;

								if (mLockedCursor)
								{
									if (relative.X != 0.5f || relative.Y != 0.5f)
									{
										mCursorControl->setPosition(0.5f, 0.5f);
									}

									irr::core::rect<float> screenRect(0, 0, mScreenWidth, mScreenHeight);

									// Only if we are moving outside quickly.
									bool reset = !screenRect.isPointInside(relative);

									if (reset)
									{
										// Force a reset.
										mCursorControl->setPosition(0.5f, 0.5f);
									}
								}
							}
							break;
						}
						default:
							break;
					}
				}
				default:
					break;
			}

			return false;
		}

	};

	/**
	 * @brief The handler object.
	*/
	InputEventHandler handler;

	/**
	 * @brief Input manager.
	 * @note Mainly for engine use.
	*/
	export class InputWrapper
	{
	private:
		/**
		 * @brief Cursor state control.
		*/
		irr::gui::ICursorControl* mCursorControl;

	public:
		/**
		 * @brief Initialize the input wrapper.
		 * @param additionalInputEventRaw
		 * @param screenHeight
		 * @param screenWidth
		*/
		inline void Init(void* additionalInputEventRaw, unsigned int screenHeight, unsigned int screenWidth)
		{
			auto additionalInputEvent =(irr::IEventReceiver*)additionalInputEventRaw;
			handler = InputEventHandler(additionalInputEvent, screenHeight, screenWidth);
		}

		/**
		 * @brief Reset data that is relative to the previous data or does not update when the state is off.
		*/
		inline void Update()
		{
			mmbScroll = 0;
		}

		/**
		 * @brief Set cursor control.
		 * @param cursorControlRaw
		*/
		inline void SetCursorControl(void* cursorControlRaw)
		{
			mCursorControl = (irr::gui::ICursorControl*)cursorControlRaw;
			handler.AddCursorControl(mCursorControl);
		}

		/**
		 * @brief Get input event handler.
		 * @return
		*/
		inline void* GetHandler()
		{
			return &handler;
		}

		/**
		 * @brief Change cursor lock state.
		 * @param locked
		*/
		inline void SetCursorLocked(bool locked)
		{
			handler.LockCursor(locked);
		}

		/**
		 * @brief Change cursor visibility state.
		 * @param visible
		*/
		inline void SetCursorVisible(bool visible)
		{
			if(mCursorControl)
				mCursorControl->setVisible(visible);
		}
	};

	/**
	 * @brief Check if right mouse button is currently clicked.
	*/
	export inline bool const RMBClicked() noexcept
	{
		return rmbClicked;
	}

	/**
	 * @brief Check if left mouse button is currently clicked.
	*/
	export inline bool const LMBClicked() noexcept
	{
		return lmbClicked;
	}

	/**
	 * @brief Check if middle mouse button is currently checked.
	*/
	export inline bool const MMBClicked() noexcept
	{
		return mmbClicked;
	}

	/**
	 * @brief Get the middle mouse button scroll direction.
	*/
	export inline float const MMBGetScroll() noexcept
	{
		return mmbScroll;
	}

	/**
	 * @brief Get the relative mouse cursor position in the window. Center of the window is 0,0.
	*/
	export inline std::tuple<float, float> const GetMousePositionRel() noexcept
	{
		return { mouseXRel, mouseYRel };
	}

	// This could have an overloaded method that accepts strings, which is then mapped to some of the key codes - this would enable key mapping

	/**
	 * @brief Check if key is currently pressed.
	*/
	export inline bool const GetKeyPressed(Keys const keyCode) noexcept
	{
		return pressedKeys.empty() ? false : pressedKeys.find(keyCode) != pressedKeys.end();
	}
};
