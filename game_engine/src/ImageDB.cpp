#include "ImageDB.h"

// Ensure the full definition of ImageDB is available before using its members.
// The error E0070 occurs if the compiler only sees a forward declaration.

#include <string>
#include <iostream>
#include <filesystem>

#include "Helper.h"

#include "SDL2/SDL.h"
#include "glm/glm.hpp"
#include "SDL_image/SDL_image.h"
#include "lua.hpp"
#include "LuaBridge.h"

SDL_Texture* ImageDB::load_image(std::string str) {
	if (img_map[str] != nullptr) { return img_map[str]; }
	if (str == "") {
		if (img_map["__particle"] != nullptr) { return img_map["__particle"]; }
		SDL_Surface* s = SDL_CreateRGBSurfaceWithFormat(0, 8, 8, 32, SDL_PIXELFORMAT_RGBA8888);
		Uint32 white = SDL_MapRGBA(s->format, 255, 255, 255, 255);
		SDL_FillRect(s, NULL, white);
		SDL_Texture* text = SDL_CreateTextureFromSurface(rend, s);
		SDL_FreeSurface(s);
		img_map["__particle"] = text;
		return text;
	}
	std::filesystem::path p = std::filesystem::path("resources") / "images" / (str + ".png");
	SDL_Texture * surface = IMG_LoadTexture(rend, p.string().c_str());
	if (!std::filesystem::exists(p)) {
		std::cout << "error: missing image " << str;
		exit(0);
	}
	img_map[str] = surface;
	return surface;
}

void ImageDB::QueueDrawUI(std::string image_name, float x, float y) {
	ImageDrawRequest req;
	int x_downcast = static_cast<int>(x);
	int y_downcast = static_cast<int>(y);
	req.image_name = image_name;
	req.x = static_cast<float>(x_downcast);
	req.y = static_cast<float>(y_downcast);
	req.isUI = true;
	ui_draws.push_back(req);
}

void ImageDB::QueueDraw(std::string image_name, float x, float y) {
	ImageDrawRequest req;
	req.image_name = image_name;
	req.x = x;
	req.y = y;
	draws.push_back(req);
}

void ImageDB::QueueDrawUIEx(std::string image_name, float x, float y, float r, float g, float b, float a, float sorting_order) {
	ImageDrawRequest req;
	int x_downcast = static_cast<int>(x);
	int y_downcast = static_cast<int>(y);
	req.image_name = image_name;
	req.x = static_cast<float>(x_downcast);
	req.y = static_cast<float>(y_downcast);
	req.r = static_cast<int>(r);
	req.g = static_cast<int>(g);
	req.b = static_cast<int>(b);
	req.a = static_cast<int>(a);
	req.sorting_order = static_cast<int>(sorting_order);
	req.isUI = true;
	ui_draws.push_back(req);
}

void ImageDB::QueueDrawEx(std::string image_name, float x, float y, float rotation_degrees,
	float scale_x, float scale_y, float pivot_x, float pivot_y,
	float r, float g, float b, float a, float sorting_order) {
	
	ImageDrawRequest req;
	req.image_name = image_name;
	req.x = x;
	req.y = y;
	req.rotation_degrees = static_cast<int>(rotation_degrees);
	req.scale_x = scale_x;
	req.scale_y = scale_y;
	req.pivot_x = pivot_x;
	req.pivot_y = pivot_y;
	req.r = static_cast<int>(r);
	req.g = static_cast<int>(g);
	req.b = static_cast<int>(b);
	req.a = static_cast<int>(a);
	req.sorting_order = static_cast<int>(sorting_order);
	draws.push_back(req);
}

void ImageDB::QueueDrawPixel(float x, float y, float r, float g, float b, float a) {
	ImageDrawRequest req;
	int x_downcast = static_cast<int>(x);
	int y_downcast = static_cast<int>(y);
	req.x = static_cast<float>(x_downcast);
	req.y = static_cast<float>(y_downcast);
	req.r = static_cast<int>(r);
	req.g = static_cast<int>(g);
	req.b = static_cast<int>(b);
	req.a = static_cast<int>(a);
	pixel_draws.push_back(req);
}

void ImageDB::QueueDrawParticle(std::string image_name, float x, float y, float rotation_degrees,
	float scale_x, float scale_y, float pivot_x, float pivot_y,
	int r, int g, int b, int a, int sorting_order) {

	ImageDrawRequest req;
	req.image_name = image_name;
	req.x = x;
	req.y = y;
	req.rotation_degrees = static_cast<int>(rotation_degrees);
	req.scale_x = scale_x;
	req.scale_y = scale_y;
	req.pivot_x = pivot_x;
	req.pivot_y = pivot_y;
	req.r = r;
	req.g = g;
	req.b = b;
	req.a = a;
	req.sorting_order = sorting_order;
	draws.push_back(req);
}
void ImageDB::set_namespace(lua_State* state) {
	luabridge::getGlobalNamespace(state)
		.beginNamespace("Image")
		.addFunction("DrawUI", &ImageDB::QueueDrawUI)
		.addFunction("DrawUIEx", &ImageDB::QueueDrawUIEx)
		.addFunction("Draw", &ImageDB::QueueDraw)
		.addFunction("DrawEx", &ImageDB::QueueDrawEx)
		.addFunction("DrawPixel", &ImageDB::QueueDrawPixel)
		.endNamespace();
}

void ImageDB::Draw(float zoom, float x_pos, float y_pos, int x_dim, int y_dim) {
	if (draws.empty() && ui_draws.empty()) { return; }
	std::stable_sort(draws.begin(), draws.end(), [](const ImageDrawRequest a, const ImageDrawRequest b)
		{return a.sorting_order < b.sorting_order; });
	std::stable_sort(ui_draws.begin(), ui_draws.end(), [](const ImageDrawRequest a, const ImageDrawRequest b)
		{return a.sorting_order < b.sorting_order; });

	draws.insert(draws.end(), ui_draws.begin(), ui_draws.end());
	
	SDL_RenderSetScale(rend, zoom, zoom);

	for (ImageDrawRequest req : draws) {
		const int ppm = 100;
		glm::vec2 pos = glm::vec2(req.x, req.y) - glm::vec2(x_pos, y_pos);

		SDL_Texture* text = load_image(req.image_name);
		float w, h;
		SDL_FRect text_rect;
		Helper::SDL_QueryTexture(text, &w, &h);

		int flip = SDL_FLIP_NONE;
		if (req.scale_x < 0) {
			flip |= SDL_FLIP_HORIZONTAL;
		}
		if (req.scale_y < 0) {
			flip |= SDL_FLIP_VERTICAL;
		}
		SDL_RendererFlip true_flip = SDL_RendererFlip(flip);

		float x_scale = glm::abs(req.scale_x);
		float y_scale = glm::abs(req.scale_y);
		text_rect.w = w * x_scale;
		text_rect.h = h * y_scale;

		SDL_FPoint pivot = {(req.pivot_x * text_rect.w),
							(req.pivot_y * text_rect.h)};
		glm::ivec2 cam_dims = glm::ivec2(x_dim, y_dim);
		if (req.isUI) {
			SDL_RenderSetScale(rend, 1, 1);
			text_rect.x = (pos.x);
			text_rect.y = (pos.y);
		}
		else {
			text_rect.x = (pos.x * ppm + cam_dims.x * 0.5f * (1.0f / zoom) - pivot.x);
			text_rect.y = (pos.y * ppm + cam_dims.y * 0.5f * (1.0f / zoom) - pivot.y);
		}


		SDL_SetTextureColorMod(text, req.r, req.g, req.b);
		SDL_SetTextureAlphaMod(text, req.a);

		Helper::SDL_RenderCopyEx(1, "NULL", rend, text, NULL, &text_rect, req.rotation_degrees,
			&pivot, true_flip);
		SDL_RenderSetScale(rend, zoom, zoom);

		SDL_SetTextureColorMod(text, 255, 255, 255);
		SDL_SetTextureAlphaMod(text, 255);

	}
	SDL_RenderSetScale(rend, 1, 1);
	draws.clear();
	ui_draws.clear();
}

void ImageDB::DrawPixel() {
	if (pixel_draws.empty()) { return; }
	SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);
	for (ImageDrawRequest req : pixel_draws) {
		SDL_SetRenderDrawColor(rend, req.r, req.g, req.b, req.a);
		SDL_RenderDrawPoint(rend, req.x, req.y);
	}
	pixel_draws.clear();
	SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_NONE);
}

void ImageDB::clear_requests() {
	draws.clear();
	ui_draws.clear();
	pixel_draws.clear();
}