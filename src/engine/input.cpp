struct ButtonState {
	bool is_down;
	bool was_down;	
};

struct MouseState {
	union {
		ButtonState buttons[3];
		struct {
			ButtonState left;	
			ButtonState middle;	
			ButtonState right;	
		};
	};
	s32 x, y;
};

enum class Button{
	Up,
	Down,
	Left,
	Right,
	
	LineUp,
	LineDown,
	LineLeft,
	LineRight,
	
	ToggleEditor,
	Fullscreen,
	
	Jump,	
	Dive,	
	MAX,
};

class InputManager {
	private:
		int button_mappings[Button::MAX];
		ButtonState buttons[2][Button::MAX];
		ButtonState (*old_buttons)[Button::MAX];
		ButtonState (*new_buttons)[Button::MAX];
		MouseState mouse_states[2];
		MouseState *old_mouse;
		MouseState *new_mouse;
		SDL_Window *window;
		SDL_GameController *controller = 0;
		
		void processKey(ButtonState *old_input, ButtonState *new_input, const u8 *keyboard_state, SDL_Keycode key) {
			u8 k = (u8)SDL_GetScancodeFromKey(key);
			new_input->is_down = keyboard_state[k] != 0;
			new_input->was_down = old_input->is_down;
		}
		
	public:
		
		InputManager(SDL_Window *w) {
			window = w;
			old_mouse = &mouse_states[0];
			new_mouse = &mouse_states[1];
			old_buttons = &buttons[0];
			new_buttons = &buttons[1];
			
			button_mappings[(int)Button::Left] = SDLK_LEFT;
			button_mappings[(int)Button::Right] = SDLK_RIGHT;
			button_mappings[(int)Button::Up] = SDLK_UP;
			button_mappings[(int)Button::Down] = SDLK_DOWN;
			
			button_mappings[(int)Button::LineLeft] = SDLK_a;
			button_mappings[(int)Button::LineRight] = SDLK_d;
			button_mappings[(int)Button::LineUp] = SDLK_w;
			button_mappings[(int)Button::LineDown] = SDLK_s;
			
			button_mappings[(int)Button::Jump] = SDLK_SPACE;
			button_mappings[(int)Button::Dive] = SDLK_LSHIFT;
			
			
			button_mappings[(int)Button::ToggleEditor] = SDLK_F4;
			button_mappings[(int)Button::Fullscreen] = SDLK_F11;
		}
		
		void connectController() {
			for(int i = 0; i < SDL_NumJoysticks(); ++i) {
				if(SDL_IsGameController(i)) {
					controller = SDL_GameControllerOpen(i);
					if(controller) {
						break;
					}
				}
			}
		}
		
		bool isKeyDown(Button key) {
			
			return (*new_buttons)[(int)key].is_down;
		}
		
		bool isKeyPressedOnce(Button key) {
			return ((*new_buttons)[(int)key].is_down && !(*new_buttons)[(int)key].was_down);
		}
		
		// relative to window
		Vec2i getMousePosition() {
			return Vec2i(new_mouse->x, new_mouse->y);
		}
		
		Vec2i getMouseDelta() {
			return getMousePosition() - Vec2i(old_mouse->x, old_mouse->y);
		}
		
		bool isMouseButtonDown(u8 button) {
			return new_mouse->buttons[button].is_down;
		}
		
		bool isMouseButtonDownOnce(u8 button) {
			return new_mouse->buttons[button].is_down && !new_mouse->buttons[button].was_down;
		}
		
		f32 deadzone = 0.3f;
		
		Vec2 getRightAxis() {
			Vec2 result = Vec2();
			s16 v_axis = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY);
			s16 h_axis = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX);
			f32 y = -(f32)v_axis / 32768.0f;
			f32 x = (f32)h_axis / 32768.0f;
			if(y >= deadzone || y <= -deadzone) result.y = y;
			if(x >= deadzone || x <= -deadzone) result.x = x;
			
			return result;
		}
		
		Vec2 getLeftAxis() {
			Vec2 result = Vec2();
			s16 v_axis = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
			s16 h_axis = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
			f32 y = -(f32)v_axis / 32768.0f;
			f32 x = (f32)h_axis / 32768.0f;
			if(y >= deadzone || y <= -deadzone) result.y = y;
			if(x >= deadzone || x <= -deadzone) result.x = x;
			
			Vec2 keyboard_result = Vec2();
			if(isKeyDown(Button::LineRight)) keyboard_result.x += 1.0f;
			if(isKeyDown(Button::LineLeft)) keyboard_result.x -= 1.0f;
			if(isKeyDown(Button::LineUp)) keyboard_result.y += 1.0f;
			if(isKeyDown(Button::LineDown)) keyboard_result.y -= 1.0f;
			if((keyboard_result.x != 0.0f || keyboard_result.y != 0.0f) || (result.x != 0.0f || result.y != 0.0f))
				return Vec2::normalize(keyboard_result + result);
			else return result;
		}
		
		bool getFire() {
			s16 axis = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
			f32 a = axis / 32768.0f;
			return axis > 0.75f || isMouseButtonDown(0);
			// return (bool)SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_X);
		}
		
		bool getJump() {
			return ((bool)SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) || (bool)SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A)) || isKeyDown(Button::Jump);
		}
		
		bool getDive() {
			return ((bool)SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B)) || isKeyDown(Button::Dive);
		}
		
		// relative to window
		void setMousePosition(s32 x, s32 y) {
			SDL_WarpMouseInWindow(window, x, y);
		}
		
		void processKeys() {
			
			const u8 *keyboard_state = SDL_GetKeyboardState(0);
			
			for(int i = 0; i < (int)Button::MAX; i++) {
				processKey((old_buttons[0]+i), (new_buttons[0]+i), keyboard_state, button_mappings[i]);
			}
			if(!ImGui::IsMouseHoveringAnyWindow() && !ImGui::IsAnyItemActive()) {
				for(u32 i = 0; i < 3; i++)
				{
					new_mouse->buttons[i].is_down = SDL_GetMouseState(0, 0) & SDL_BUTTON(i+1);
					new_mouse->buttons[i].was_down = old_mouse->buttons[i].is_down;
				}
			}
			
			SDL_GetMouseState(&new_mouse->x, &new_mouse->y);
		}
		
		void endFrame() {
			Swap(old_buttons, new_buttons);
			Swap(old_mouse, new_mouse);
		}
};