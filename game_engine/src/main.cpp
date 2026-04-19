#include <iostream>
#include <filesystem>
#include <string>
#include <sstream>
#include <cstdlib>

#include "game.h"

#include "glm/glm.hpp"
#include "rapidjson/document.h"
#include "lua.hpp"


using namespace std;


bool check_filesystem() {
	if (!filesystem::exists("resources") || !filesystem::is_directory("resources")) {
		cout << "error: resources/ missing";
		return false;
	}
	if (!filesystem::exists("resources/game.config")) {
		cout << "error: resources/game.config missing";
		return false;
	}
	return true;
}

int main(int argc, char* argv[]) {
	if (!check_filesystem()) { exit(0); }
	Game eng;
	eng.game_loop();
	return 0;
}
