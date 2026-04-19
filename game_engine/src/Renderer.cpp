#include "Renderer.h"

#include <filesystem>
#include <string>

#include "EngineUtils.h"
#include "Helper.h"

#include "rapidjson/document.h"
#include "SDL2/SDL.h"
#include "lua.hpp"

Renderer::Renderer() {
	rapidjson::Document game_config;
	rapidjson::Document render_config;
	EngineUtils e;

	e.ReadJsonFile("resources/game.config", game_config);

	// Read into game settings (must exist)
	if (game_config.HasMember("game_title")) {
		window_title = game_config["game_title"].GetString();
	}

	// Read into rendering settings (if file exists)
	if (std::filesystem::exists("resources/rendering.config")) {
		e.ReadJsonFile("resources/rendering.config", render_config);
		if (render_config.HasMember("x_resolution")) {
			x_res = render_config["x_resolution"].GetInt();
		}
		if (render_config.HasMember("y_resolution")) {
			y_res = render_config["y_resolution"].GetInt();
		}
		if (render_config.HasMember("clear_color_r")) {
			colors[0] = render_config["clear_color_r"].GetInt();
		}
		if (render_config.HasMember("clear_color_g")) {
			colors[1] = render_config["clear_color_g"].GetInt();
		}
		if (render_config.HasMember("clear_color_b")) {
			colors[2] = render_config["clear_color_b"].GetInt();
		}
		if (render_config.HasMember("zoom_factor")) {
			zoom = render_config["zoom_factor"].GetFloat();
		}
		if (render_config.HasMember("cam_ease_factor")) {
			cam_ease = render_config["cam_ease_factor"].GetFloat();
		}
	}
	wind = Helper::SDL_CreateWindow(window_title.c_str(), 10, 10, x_res, y_res, 0);
	rend = Helper::SDL_CreateRenderer(wind, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
	SDL_RenderSetScale(rend, zoom, zoom);
	TXT.rend = rend;
	IMG.rend = rend;
}

void Renderer::clear_renderer() {
	SDL_SetRenderDrawColor(rend, colors[0], colors[1], colors[2], SDL_ALPHA_OPAQUE);
	SDL_RenderClear(rend);
}

glm::vec2 Renderer::mix(glm::vec2 a, glm::vec2 b) {
	glm::vec2 i = glm::mix(a, b, cam_ease);
	return i;
}

void Renderer::render() {
	IMG.Draw(zoom, x_pos, y_pos, x_res, y_res);
	TXT.Draw();
	IMG.DrawPixel();
	Helper::SDL_RenderPresent(rend);
}

void Renderer::clear_render_requests() {
	TXT.clear_requests();
	IMG.clear_requests();
}