#ifndef IMAGEDB_H
#define IMAGEDB_H

#include <string>
#include <vector>
#include <unordered_map>

#include "SDL2/SDL.h"
#include "lua.hpp"

class ImageDB
{
public:
	ImageDB() {};

	static SDL_Texture* load_image(std::string str);

	static void QueueDrawUI(std::string image_name, float x, float y);

	static void QueueDrawUIEx(std::string image_name, float x, float y, float r, float g, float b, float a, float sorting_order);

	static void QueueDraw(std::string image_name, float x, float y);

	static void QueueDrawEx(std::string image_name, float x, float y, float rotation_degrees,
							float scale_x, float scale_y, float pivot_x, float pivot_y,
							float r, float g, float b, float a, float sorting_order);

	static void QueueDrawPixel(float x, float y, float r, float g, float b, float a);

	static void QueueDrawParticle(std::string image_name, float x, float y, float rotation_degrees,
								  float scale_x, float scale_y, float pivot_x, float pivot_y,
								  int r, int g, int b, int a, int sorting_order);

	static void set_namespace(lua_State* state);

	static void Draw(float zoom, float x_pos, float y_pos, int x_dim, int y_dim);

	static void DrawPixel();

	static void clear_requests();

	static inline SDL_Renderer* rend = nullptr;

private:
	static inline std::unordered_map<std::string, SDL_Texture*> img_map;

	struct ImageDrawRequest {
		std::string image_name;
		float x;
		float y; 
		int rotation_degrees = 0;
		float scale_x = 1.0f;
		float scale_y = 1.0f;
		float pivot_x = 0.5f;
		float pivot_y = 0.5f;
		int r = 255;
		int g = 255;
		int b = 255;
		int a = 255;
		int sorting_order = 0;
		bool isUI = false;
	};

	static inline std::vector<ImageDrawRequest> draws;
	static inline std::vector<ImageDrawRequest> ui_draws;
	static inline std::vector<ImageDrawRequest> pixel_draws;

};

#endif // IMAGEDB_H

