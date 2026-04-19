#ifndef GAME_H
#define GAME_H

#include <string>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <map>
#include <vector>
#include <utility>

#include "Renderer.h"
#include "SceneDB.h"
#include "TemplateDB.h"
#include "Input.h"
#include "ComponentDB.h"
#include "EventBus.h"
#include "ContactListener.h"
#include "EngineUtils.h"

#include "rapidjson/document.h"
#include "lua.hpp"
#include "box2d/box2d.h"

class Game {

public:

	Game() {
		e.ReadJsonFile("resources/game.config", game_config);
		if (!game_config.HasMember("initial_scene")) {
			std::cout << "error: initial_scene unspecified";
			exit(0);
		}
		state = luaL_newstate();
		luaL_openlibs(state);
		comps = ComponentDB(state, &world, &r);
		templates = TemplateDB(comps, state);
		bus.state = state;
		current_scene = SceneDB(game_config["initial_scene"].GetString(), templates, comps, state, &world);
		setup_loop();

	}

	void game_loop();

private:
	Renderer r;
	lua_State* state = nullptr;
	ComponentDB comps;
	TemplateDB templates;
	static inline b2World world = b2World(b2Vec2(0.0f, 9.8f));
	ContactListener con;
	EventBus bus;
	
	bool over = false;
	bool change_scene = false;

	rapidjson::Document game_config;
	Input inp;
	EngineUtils e;
	SceneDB current_scene;

	void process_input();
	void update_actors();

	void setup_loop();

	void prepare_new_scene();
	void set_namespace();


};

#endif
