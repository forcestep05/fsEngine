#ifndef RENDERER_H
#define RENDERER_H

#include <string>

#include "Helper.h"
#include "TextDB.h"
#include "AudioDB.h"
#include "ImageDB.h"

#include "SDL2/SDL.h"
#include "glm/glm.hpp"
#include "lua.hpp"

class Renderer {
public:
	Renderer();

	void clear_renderer();
	void render();
	static inline void set_namespace() {
		TXT.set_namespace(state);
		SFX.set_namespace(state);
		IMG.set_namespace(state);
		luabridge::getGlobalNamespace(state)
			.beginNamespace("Camera")
			.addFunction("SetPosition", &Renderer::set_position)
			.addFunction("GetPositionX", &Renderer::get_x_pos)
			.addFunction("GetPositionY", &Renderer::get_y_pos)
			.addFunction("GetZoom", &Renderer::get_zoom)
			.addFunction("SetZoom", &Renderer::set_zoom)
			.endNamespace();
	};

	int get_height() { return y_res / zoom; };
	int get_width() { return x_res / zoom; };
	static float get_x_pos() { return x_pos; };
	static float get_y_pos() { return y_pos; };
	static float get_zoom() { return zoom; };
	static void set_zoom(float z) { zoom = z; }
	static void set_position(float x, float y) { x_pos = x; y_pos = y; }

	static void clear_render_requests();

	glm::vec2 mix(glm::vec2 a, glm::vec2 b);

	static inline TextDB TXT;
	static inline AudioDB SFX;
	static inline ImageDB IMG;
	static inline lua_State* state;

private:
	int x_res = 640;
	int y_res = 360;
	static inline float x_pos = 0.0f;
	static inline float y_pos = 0.0f;
	static inline float zoom = 1.0f;
	float cam_ease = 1.0f;
	int colors[3] = { 255, 255, 255 };
	std::string window_title = "";

	SDL_Window* wind = nullptr;
	SDL_Renderer* rend = nullptr;
};

#endif // RENDERER_H
