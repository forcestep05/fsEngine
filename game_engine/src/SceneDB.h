#ifndef SCENEDB_H
#define SCENEDB_H

#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#include <cstdlib>
#include <memory>

#include "EngineUtils.h"
#include "actor.h"
#include "ActorDB.h"
#include "TemplateDB.h"
#include "ComponentDB.h"
#include "Raycast.h"

#include "rapidjson/document.h"
#include "glm/glm.hpp"
#include "lua.hpp"
#include "LuaBridge.h"
#include "box2d/box2d.h"

struct HitResult {
	Actor* actor;
	b2Vec2 point;
	b2Vec2 normal;
	bool is_trigger;
};

class SceneDB
{
public:
	SceneDB() {
		scene = "";
		state = nullptr;
	};

	SceneDB(std::string scene, TemplateDB templates, ComponentDB& comps, lua_State* state, b2World* world);

	static inline ActorDB actors;

	bool new_scene() { return next_scene.first; }

	static std::string get_current_scene_name() { return scene; };

	static std::string get_next_scene_name() { return next_scene.second; }

	static void load_new_scene(std::string scene_name) { next_scene.first = true; next_scene.second = scene_name; }

	static void do_not_destroy(Actor* actor) {actor->to_survive_next = true;}

	void process_collision_enter(b2Contact* contact);

	void process_collision_exit(b2Contact* contact);

	static luabridge::LuaRef Raycast(b2Vec2 pos, b2Vec2 dir, float dist) {
			if ((dir.x == 0.0f && dir.y == 0.0f) || dist <= 0.0f) { return luabridge::LuaRef(state); }
			b2Vec2 dest;
			dir.Normalize();
			dest = dist * dir;
			dest += pos;

			world->RayCast(&ray, pos, dest);
			CastInfo* ci = ray.find_closest();
			if (ci == nullptr) { return luabridge::LuaRef(state); }
			b2Fixture* fixture = std::get<0>(*ci);
			std::shared_ptr<Actor> actor = actors.fixtures[fixture];
			bool is_trigger = (fixture->GetFilterData().categoryBits == 3);
			HitResult hit;
			hit.actor = actor.get();
			hit.is_trigger = is_trigger;
			hit.point = std::get<1>(*ci);
			hit.normal = std::get<2>(*ci);
			ray.clear();
			return luabridge::LuaRef(state, hit);
		
	};

	static luabridge::LuaRef RaycastAll(b2Vec2 pos, b2Vec2 dir, float dist);

	void clear() {
		next_scene.first = false;
		actors.clear();
	}

private:
	static inline std::string scene;
	static inline std::pair<bool, std::string> next_scene;
	TemplateDB templates;
	static inline lua_State* state;
	static inline b2World* world = nullptr;
	static inline RaycastCallback ray;
};



#endif

