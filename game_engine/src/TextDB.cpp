#include "TextDB.h"

#include <string>
#include <unordered_map>
#include "SDL_ttf/SDL_ttf.h"
#include "SDL2/SDL.h"
#include "lua.hpp"
#include "LuaBridge.h"

#include "Helper.h"

void TextDB::Draw() {
	if (requests_this_frame.empty()) { return; }
	int size = requests_this_frame.size();
	TTF_Init();
	for (int i = 0; i < size; i++) {
		TextDrawRequest req = requests_this_frame.front();
		std::string fontpath = "resources/fonts/" + req.fontname + ".ttf";
		TTF_Font* fontPtr = TTF_OpenFont(fontpath.c_str(), req.fontSize);
		SDL_Color color = { static_cast<Uint8>(req.r), static_cast<Uint8>(req.g), static_cast<Uint8>(req.b), static_cast<Uint8>(req.a) };
		SDL_Surface* textSurface = TTF_RenderText_Solid(fontPtr, req.content.c_str(), color);
		SDL_Texture* textTexture = SDL_CreateTextureFromSurface(rend, textSurface);
		SDL_FRect renderQuad = { static_cast<float>(req.x), static_cast<float>(req.y), static_cast<float>(textSurface->w), static_cast<float>(textSurface->h) };
		Helper::SDL_RenderCopy(rend, textTexture, nullptr, &renderQuad);
		requests_this_frame.pop();
	}
	
}

void TextDB::QueueDraw(std::string content, int x, int y, std::string fontname, int fontSize, int r, int g, int b, int a) {
	TextDrawRequest req;
	req.content = content;
	req.x = x;
	req.y = y;
	req.fontname = fontname;
	req.fontSize = fontSize;
	req.r = r;
	req.g = g;
	req.b = b;
	req.a = a;
	requests_this_frame.push(req);
}

void TextDB::clear_requests() {
	while (!requests_this_frame.empty()) {
		requests_this_frame.pop();
	}
}