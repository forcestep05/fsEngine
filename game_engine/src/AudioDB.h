#ifndef AUDIODB_H
#define AUDIODB_H

#include <string>
#include <unordered_map>
#include "SDL_mixer/SDL_mixer.h"
#include "lua.hpp"
#include "LuaBridge.h"

class AudioDB
{
public:
	AudioDB() {};

	static void play_track(int channel, std::string file, bool loop);
	static void stop_track(int channel);
	static void set_volume(int channel, int volume);

	static inline void set_namespace(lua_State* state) {
		luabridge::getGlobalNamespace(state)
			.beginNamespace("Audio")
			.addFunction("Play", &AudioDB::play_track)
			.addFunction("Halt", &AudioDB::stop_track)
			.addFunction("SetVolume", &AudioDB::set_volume)
			.endNamespace();
	};

private:
	static inline std::unordered_map<std::string, Mix_Chunk*> db;
};

#endif
