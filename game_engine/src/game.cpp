#include "game.h"

#include <string>
#include <vector>
#include <tuple>
#include <iostream>
#include <map>
#include <unordered_map>
#include <sstream>
#include <utility>
#include <algorithm>

#include "SceneDB.h"
#include "Renderer.h"
#include "Application.h"

#include "Helper.h"
#include "AudioHelper.h"

#include "glm/glm.hpp"
#include "rapidjson/document.h"
#include "SDL2/SDL.h"
#include "SDL_image/SDL_image.h"
#include "box2d/box2d.h"
#include "lua.hpp"
#include "LuaBridge.h"

using namespace std;
using Map = unordered_map<int, bool>;
using BlockingMap = unordered_map<uint64_t, int>;
using ActorMap = unordered_map<uint64_t, vector<Actor>>;
using ActorVector = std::vector<std::shared_ptr<Actor>>;


SDL_RendererFlip getRendererFlip(bool hflip, bool vflip) {
	int i = 0;
	if (hflip) { i |= SDL_FLIP_HORIZONTAL; }
	if (vflip) { i |= SDL_FLIP_VERTICAL; }
	return static_cast<SDL_RendererFlip>(i);
}

void Game::setup_loop() {
	SDL_Init(SDL_INIT_EVERYTHING);
	AudioHelper::Mix_AllocateChannels(50);
	set_namespace();
	world.SetContactListener(&con);
	con.current_scene = &current_scene;
	inp.set_namespace(state);
	r.state = state;
	r.set_namespace();
	current_scene.actors.onStart();

}

void Game::game_loop() {
	IMG_Init(IMG_INIT_PNG);
	inp.Init();
	while (!over) {
		// Read input
		current_scene.actors.pre_update();
		if (change_scene) { current_scene.actors.onStart(); change_scene = false; }
		process_input();
		r.clear_renderer();

		update_actors();
		inp.LateUpdate();
		current_scene.actors.post_update();
		bus.activate_requests();
		
		if (current_scene.new_scene()) { prepare_new_scene(); }

		r.render();

	}
}

void Game::process_input() {
	SDL_Event next_event;
	while (Helper::SDL_PollEvent(&next_event)) {
		if (next_event.type == SDL_QUIT) {
			over = true;
		}
		else {
			inp.ProcessEvent(next_event);
		}
	}
}

void Game::update_actors() {
	current_scene.actors.onUpdate();
	current_scene.actors.onLateUpdate();
	world.Step(1.0f / 60.0f, 8, 3);
}


void Game::prepare_new_scene() {
	std::vector<std::shared_ptr<Actor>> carry = current_scene.actors.get_surviving_actors();
	current_scene.clear();
	SceneDB new_scene(current_scene.get_next_scene_name(), templates, comps, state, &world);
	current_scene = new_scene;
	current_scene.actors.append_new_actors(carry);
	change_scene = true;
}

void Game::set_namespace() {
	luabridge::getGlobalNamespace(state)
		.beginNamespace("Application")
		.addFunction("Quit", &Application::quit)
		.addFunction("GetFrame", &Application::get_frame)
		.addFunction("Sleep", &Application::sleep)
		.addFunction("OpenURL", &Application::open_url)
		.endNamespace();
	luabridge::getGlobalNamespace(state)
		.beginClass<glm::vec2>("vec2")
		.addProperty("x", &glm::vec2::x)
		.addProperty("y", &glm::vec2::y)
		.endClass();
	luabridge::getGlobalNamespace(state)
		.beginNamespace("Scene")
		.addFunction("GetCurrent", &SceneDB::get_current_scene_name)
		.addFunction("Load", &SceneDB::load_new_scene)
		.addFunction("DontDestroy", &SceneDB::do_not_destroy)
		.endNamespace();
	luabridge::getGlobalNamespace(state)
		.beginClass<b2Vec2>("Vector2")
		.addConstructor<void(*) (float, float)>()
		.addProperty("x", &b2Vec2::x)
		.addProperty("y", &b2Vec2::y)
		.addFunction("Normalize", &b2Vec2::Normalize)
		.addFunction("Length", &b2Vec2::Length)
		.addFunction("__add", &b2Vec2::operator_add)
		.addFunction("__sub", &b2Vec2::operator_sub)
		.addFunction("__mul", &b2Vec2::operator_mult)
		.addStaticFunction("Distance", &b2Distance)
		.addStaticFunction("Dot", static_cast<float (*) (const b2Vec2&, const b2Vec2&)>(&b2Dot))
		.endClass();
	luabridge::getGlobalNamespace(state)
		.beginClass<HitResult>("HitResult")
		.addProperty("actor", &HitResult::actor)
		.addProperty("point", &HitResult::point)
		.addProperty("normal", &HitResult::normal)
		.addProperty("is_trigger", &HitResult::is_trigger)
		.endClass();
	luabridge::getGlobalNamespace(state)
		.beginNamespace("Physics")
		.addFunction("Raycast", &SceneDB::Raycast)
		.addFunction("RaycastAll", &SceneDB::RaycastAll)
		.endNamespace();
	luabridge::getGlobalNamespace(state)
		.beginNamespace("Event")
		.addFunction("Publish", &EventBus::Publish)
		.addFunction("Subscribe", &EventBus::Subscribe)
		.addFunction("Unsubscribe", &EventBus::Unsubscribe)
		.endNamespace();
}
