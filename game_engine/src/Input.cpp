#include "Input.h"

#include <vector>

#include "InputMapHelper.h"

#include "SDL2/SDL.h"

void Input::Init() {
	SDL_Init(SDL_INIT_GAMECONTROLLER);
	just_became_down_scancodes.clear();
	just_became_up_scancodes.clear();
	joystick_dirs.push_back(glm::vec2(0.0f, 0.0f));
	joystick_dirs.push_back(glm::vec2(0.0f, 0.0f));
	controller = SDL_GameControllerOpen(0);
}

void Input::ProcessEvent(const SDL_Event& e) {
	if (e.type == SDL_KEYDOWN) {
		keyboard_states[e.key.keysym.scancode] = INPUT_STATE_JUST_BECAME_DOWN;
		just_became_down_scancodes.push_back(e.key.keysym.scancode);
	}
	else if (e.type == SDL_KEYUP) {
		keyboard_states[e.key.keysym.scancode] = INPUT_STATE_JUST_BECAME_UP;
		just_became_up_scancodes.push_back(e.key.keysym.scancode);
	}
	else if (e.type == SDL_MOUSEBUTTONDOWN) {
		mouse_button_states[e.button.button] = INPUT_STATE_JUST_BECAME_DOWN;
		just_became_down_buttons.push_back(e.button.button);
	}
	else if (e.type == SDL_MOUSEBUTTONUP) {
		mouse_button_states[e.button.button] = INPUT_STATE_JUST_BECAME_UP;
		just_became_up_buttons.push_back(e.button.button);
	}
	else if (e.type == SDL_MOUSEMOTION) {
		mouse_position = glm::vec2(e.motion.x, e.motion.y);
	}
	else if (e.type == SDL_MOUSEWHEEL) {
		mouse_scroll_this_frame = e.wheel.preciseY;
	}
	else if (e.type == SDL_CONTROLLERDEVICEADDED) {
		// Connect controller
		if (controller == nullptr) {
			controller = SDL_GameControllerOpen(e.cdevice.which);
		}
	}
	else if (e.type == SDL_CONTROLLERDEVICEREMOVED) {
		// Disconnect controller
		if (controller != nullptr) {
			SDL_GameControllerClose(controller);
			controller = nullptr;
			joystick_dirs[0] = glm::vec2(0.0f, 0.0f);
			joystick_dirs[1] = glm::vec2(0.0f, 0.0f);
		}
	}
	else if (e.type == SDL_CONTROLLERBUTTONUP) {
		// Button Up
		if (controller == nullptr) { return; }
		SDL_GameControllerButton button = static_cast<SDL_GameControllerButton>(e.cbutton.button);
		controller_states[button] = INPUT_STATE_JUST_BECAME_UP;
		just_became_up_controls.push_back(button);
	}
	else if (e.type == SDL_CONTROLLERBUTTONDOWN) {
		// Button Down
		if (controller == nullptr) { return; }
		SDL_GameControllerButton button = static_cast<SDL_GameControllerButton>(e.cbutton.button);
		controller_states[button] = INPUT_STATE_JUST_BECAME_DOWN;
		just_became_down_controls.push_back(button);
	}
	else if (e.type == SDL_CONTROLLERAXISMOTION) {
		// Joystick Motion
		if (controller == nullptr) { return; }
		SDL_GameControllerAxis axis = static_cast<SDL_GameControllerAxis>(e.caxis.axis);
		set_joystick_dir(axis, e);
	}
}
void Input::LateUpdate() {
	for (SDL_Scancode s : just_became_down_scancodes) {
		keyboard_states[s] = INPUT_STATE_DOWN;
	}
	for (SDL_Scancode s : just_became_up_scancodes) {
		keyboard_states[s] = INPUT_STATE_UP;
	}
	for (int i : just_became_down_buttons) {
		mouse_button_states[i] = INPUT_STATE_DOWN;
	}
	for (int i : just_became_up_buttons) {
		mouse_button_states[i] = INPUT_STATE_UP;
	}
	for (SDL_GameControllerButton c : just_became_down_controls) {
		controller_states[c] = INPUT_STATE_DOWN;
	}
	for (SDL_GameControllerButton c : just_became_up_controls) {
		controller_states[c] = INPUT_STATE_UP;
	}
	just_became_down_scancodes.clear();
	just_became_up_scancodes.clear();
	just_became_down_buttons.clear();
	just_became_up_buttons.clear();
	just_became_down_controls.clear();
	just_became_up_controls.clear();
	mouse_scroll_this_frame = 0.0f;
}
bool Input::GetKey(std::string keycode) {
	if (__keycode_to_scancode.find(keycode) == __keycode_to_scancode.end()) {
		return false;
	}
	else {
		SDL_Scancode code = __keycode_to_scancode.at(keycode);
		return (keyboard_states[code] == INPUT_STATE_DOWN || keyboard_states[code] == INPUT_STATE_JUST_BECAME_DOWN);
	}
	
}

bool Input::GetKeyDown(std::string keycode) {
	if (__keycode_to_scancode.find(keycode) == __keycode_to_scancode.end()) {
		return false;
	}
	else {
		SDL_Scancode code = __keycode_to_scancode.at(keycode);
		bool val = (keyboard_states[code] == INPUT_STATE_JUST_BECAME_DOWN);
		return val;
	}
	
}

bool Input::GetKeyUp(std::string keycode) {
	if (__keycode_to_scancode.find(keycode) == __keycode_to_scancode.end()) {
		return false;
	}
	else {
		SDL_Scancode code = __keycode_to_scancode.at(keycode);
		bool val = (keyboard_states[code] == INPUT_STATE_JUST_BECAME_UP);
		return val;
	}
	
}

bool Input::GetMouseButton(int button) {
	if (button < 1 || button > 3) { return false; }
	return (mouse_button_states[button] == INPUT_STATE_DOWN || mouse_button_states[button] == INPUT_STATE_JUST_BECAME_DOWN);
}

bool Input::GetMouseButtonDown(int button) {
	if (button < 1 || button > 3) { return false; }
	bool val = (mouse_button_states[button] == INPUT_STATE_JUST_BECAME_DOWN);
	return val;
}

bool Input::GetMouseButtonUp(int button) {
	if (button < 1 || button > 3) { return false; }
	bool val = (mouse_button_states[button] == INPUT_STATE_JUST_BECAME_UP);
	return val;
}

float Input::GetMouseScrollDelta() {
	return mouse_scroll_this_frame;
}

glm::vec2 Input::GetMousePosition() {
	return mouse_position;
}

void Input::HideCursor() {
	SDL_ShowCursor(SDL_DISABLE);
}

void Input::ShowCursor() {
	SDL_ShowCursor(SDL_ENABLE);
}

bool Input::GetController() {
	return controller != nullptr;
}

bool Input::GetControllerButton(std::string button) {
	if (__keycode_to_button.find(button) == __keycode_to_button.end()) {
		return false;
	}
	else {
		SDL_GameControllerButton code = __keycode_to_button.at(button);
		return (controller_states[code] == INPUT_STATE_DOWN || controller_states[code] == INPUT_STATE_JUST_BECAME_DOWN);
	}
}

bool Input::GetControllerButtonUp(std::string button) {
	if (__keycode_to_button.find(button) == __keycode_to_button.end()) {
		return false;
	}
	else {
		SDL_GameControllerButton code = __keycode_to_button.at(button);
		return (controller_states[code] == INPUT_STATE_JUST_BECAME_UP);
	}
}

bool Input::GetControllerButtonDown(std::string button) {
	if (__keycode_to_button.find(button) == __keycode_to_button.end()) {
		return false;
	}
	else {
		SDL_GameControllerButton code = __keycode_to_button.at(button);
		return (controller_states[code] == INPUT_STATE_JUST_BECAME_DOWN);
	}
}

glm::vec2 Input::GetJoystickDir(int joystick) {
	if (controller != nullptr) { 
		glm::vec2 dir = joystick_dirs[joystick];
		if (dir != glm::vec2(0.0f, 0.0f)) {
			return glm::normalize(dir);
		}
		else {
			return dir;
		}
	}
	else { return glm::vec2(-404.0f, -404.0f); }
}

float Input::GetJoystickMagnitude(int joystick) {
	if (controller != nullptr) { 
		glm::vec2 dir = joystick_dirs[joystick];
		float mag = glm::length(dir);
		if (mag < 0.08) {
			return 0.0f;
		}
		else {
			return mag;
		}

	}
	else { return -1.0f; }
}

void Input::set_namespace(lua_State* state) {
	luabridge::getGlobalNamespace(state)
		.beginNamespace("Input")
		.addFunction("GetKey", &Input::GetKey)
		.addFunction("GetKeyDown", &Input::GetKeyDown)
		.addFunction("GetKeyUp", &Input::GetKeyUp)
		.addFunction("GetMousePosition", &Input::GetMousePosition)
		.addFunction("GetMouseButton", &Input::GetMouseButton)
		.addFunction("GetMouseButtonDown", &Input::GetMouseButtonDown)
		.addFunction("GetMouseButtonUp", &Input::GetMouseButtonUp)
		.addFunction("GetMouseScrollDelta", &Input::GetMouseScrollDelta)
		.addFunction("GetController", &Input::GetController)
		.addFunction("GetControllerButton", &Input::GetControllerButton)
		.addFunction("GetControllerButtonDown", &Input::GetControllerButtonDown)
		.addFunction("GetControllerButtonUp", &Input::GetControllerButtonUp)
		.addFunction("HideCursor", &Input::HideCursor)
		.addFunction("ShowCursor", &Input::ShowCursor)
		// Controller Functions
		.addFunction("GetController", &Input::GetController)
		.addFunction("GetControllerButton", &Input::GetControllerButton)
		.addFunction("GetControllerButtonUp", &Input::GetControllerButtonUp)
		.addFunction("GetControllerButtonDown", &Input::GetControllerButtonDown)
		.addFunction("GetJoystickDir", &Input::GetJoystickDir)
		.addFunction("GetJoystickMagnitude", &Input::GetJoystickMagnitude)
		.endNamespace();
}

void Input::set_joystick_dir(SDL_GameControllerAxis axis, const SDL_Event &e) {
	if (axis == SDL_CONTROLLER_AXIS_LEFTX) {
		joystick_dirs[0].x = e.caxis.value / static_cast<float>(SDL_JOYSTICK_AXIS_MAX);
	}
	else if (axis == SDL_CONTROLLER_AXIS_LEFTY) {
		joystick_dirs[0].y = e.caxis.value / static_cast<float>(SDL_JOYSTICK_AXIS_MAX);
	}
	else if (axis == SDL_CONTROLLER_AXIS_RIGHTX) {
		joystick_dirs[1].x = e.caxis.value / static_cast<float>(SDL_JOYSTICK_AXIS_MAX);
	}
	else if (axis == SDL_CONTROLLER_AXIS_RIGHTY) {
		joystick_dirs[1].y = e.caxis.value / static_cast<float>(SDL_JOYSTICK_AXIS_MAX);
	}
}