#ifndef TEXTDB_H
#define TEXTDB_H
#include <string>
#include <queue>
#include <unordered_map>
#include "SDL2/SDL.h"
#include "lua.hpp"
#include "LuaBridge.h"


class TextDB
{
public:
	TextDB() {};

	static void Draw();

	static inline void set_namespace(lua_State* state) {
		luabridge::getGlobalNamespace(state)
			.beginNamespace("Text")
			.addFunction("Draw", &TextDB::QueueDraw)
			.endNamespace();
	};

	static void QueueDraw(std::string content, int x, int y, std::string fontname, int fontSize, int r, int g, int b, int a);

	static void clear_requests();

	static inline SDL_Renderer* rend = nullptr;

private:
	std::unordered_map<std::string, SDL_Texture*> textmap;

	struct TextDrawRequest {
		std::string content;
		std::string fontname;
		int x;
		int y;
		int fontSize;
		int r;
		int g;
		int b;
		int a;

	};

	static inline std::queue<TextDrawRequest> requests_this_frame;

};

#endif // TEXTDB_H

