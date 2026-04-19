#include "AudioDB.h"
#include <string>
#include <filesystem>
#include <iostream>
#include "AudioHelper.h"
#include "SDL_mixer/SDL_mixer.h"
#include "lua.hpp"
#include "LuaBridge.h"

void AudioDB::play_track(int channel, std::string file , bool loop) {
	if (file == "") { return; }
	int looping = loop ? -1 : 0;
	if (db[file] != nullptr) {
		AudioHelper::Mix_PlayChannel(channel, db[file], looping);
		return;
	}
	std::string filepath = "resources/audio/" + file;
	AudioHelper::Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
	Mix_Chunk* chunk = nullptr;
	if (std::filesystem::exists(filepath + ".ogg")) {
		std::string path = filepath + ".ogg";
		chunk = AudioHelper::Mix_LoadWAV(path.c_str());
	}
	else if (std::filesystem::exists(filepath + ".wav")) {
		std::string path = filepath + ".wav";
		chunk = AudioHelper::Mix_LoadWAV(path.c_str());
	}
	else {
		std::cout << "error: failed to play audio clip " << file;
		exit(0);
	}
	db[file] = chunk;

	AudioHelper::Mix_PlayChannel(channel, chunk, looping);
}

void AudioDB::stop_track(int channel) {
	AudioHelper::Mix_HaltChannel(channel);
}

void AudioDB::set_volume(int channel, int volume) {
	AudioHelper::Mix_Volume(channel, volume);
}