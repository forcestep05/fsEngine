#ifndef ACTORDB_H
#define ACTORDB_H

#include <string>
#include <vector>
#include <unordered_map>

#include "actor.h"
#include "TemplateDB.h"
#include "RigidBody.h"

#include "lua.hpp"
#include "LuaBridge.h"



struct ActorDB {
	static inline std::vector<std::shared_ptr<Actor>> actors;
	static inline std::vector<std::shared_ptr<Actor>> to_add;
	static inline std::vector<std::shared_ptr<Actor>> to_destroy;
	static inline lua_State* state;
	static inline TemplateDB templates;
	static inline ComponentDB comps;
	static inline int current_id;
	static inline std::unordered_map <b2Fixture*, std::shared_ptr<Actor>> fixtures;

	void add(std::shared_ptr<Actor> actor) { actors.push_back(actor); }
	inline void getNamespaceRunning();
	inline std::vector<std::shared_ptr<Actor>> get_surviving_actors();
	inline void append_new_actors(std::vector<std::shared_ptr<Actor>> v);
	inline void clear();
	inline void post_update();
	inline void pre_update();

	inline void onStart();
	void onUpdate() { for (std::shared_ptr<Actor> a : actors) { a->OnUpdate(); } }
	void onLateUpdate() { for (std::shared_ptr<Actor> a : actors) { a->OnLateUpdate(); } }
	
	static inline Actor* FindActor(std::string name);
	static inline luabridge::LuaRef FindAllActors(std::string name);
	static inline Actor* Instantiate(std::string temp);
	static inline void Destroy(Actor* act);


};

void ActorDB::onStart() { 
	for (std::shared_ptr<Actor> a : actors) { 
		a->OnStart();
		if (a->has_rigidbody) {
			for (Component c : a->components) {
				if (c.type != "Rigidbody") { continue; }
				else {
					RigidBody* rb = c.compRef->cast<RigidBody*>();
					if (rb->has_collider) {
						fixtures[rb->collider] = a;
					}
					if (rb->has_trigger) {
						fixtures[rb->trigger] = a;
					}
				}
			}
		}
	} 
}

Actor* ActorDB::FindActor(const std::string name) {
	std::shared_ptr<Actor> a;
	bool find = false;
	for (std::shared_ptr<Actor> actor : actors) {
		if (actor->name == name && !actor->to_be_removed) {
			a = actor;
			find = true;
			break;
		}
	}
	if (!find) {
		for (std::shared_ptr<Actor> actor : to_add) {
			if (actor->name == name && !actor->to_be_removed) {
				a = actor;
				find = true;
				break;
			}
		}
	}

	if (find) {
		return a.get();
	}
	else {
		return luabridge::LuaRef(state);
	}
}

 luabridge::LuaRef ActorDB::FindAllActors(const std::string name) {
	std::vector<std::shared_ptr<Actor>> f;
	bool find = false;
	for (std::shared_ptr<Actor> actor : actors) {
		if (actor->name == name && !actor->to_be_removed) {
			f.push_back(actor);
		}
	}
	for (std::shared_ptr<Actor> actor : to_add) {
		if (actor->name == name && !actor->to_be_removed) {
			f.push_back(actor);
		}
	}
	luabridge::LuaRef ref = luabridge::newTable(state);
	for (int i = 0; i < f.size(); i++) {
		ref[i + 1] = f[i].get();
	}

	return ref;

}

 Actor* ActorDB::Instantiate(std::string temp) {
	 std::shared_ptr<Actor> a = templates.check_templates(temp, comps, state);
	 a->id = current_id;
	 a->state = state;
	 a->comps = &comps;
	 current_id++;
	 to_add.push_back(a);
	 return a.get();
 }

 void ActorDB::Destroy(Actor* act) {
	 bool find = false;
	 std::shared_ptr<Actor> t;
	 for (std::shared_ptr<Actor> a : actors) {
		 if (act->id == a->id) {
			 find = true;
			 a->to_be_removed = true;
			 t = a;
			 break;
		 }
	 }
	 if (!find) {
		 for (std::shared_ptr<Actor> a : to_add) {
			 if (act->id == a->id) {
				 find = true;
				 a->to_be_removed = true;
				 t = a;
				 break;
			 }
		 }
	 }
	 if (find) {
		 for (Component c : t->components) {
			 (*c.compRef)["enabled"] = false;
		 }
		 to_destroy.push_back(t);
	 }
 }

void ActorDB::getNamespaceRunning() {
	luabridge::getGlobalNamespace(state)
			.beginNamespace("Actor")
			.addFunction("Find", &ActorDB::FindActor)
			.addFunction("FindAll", &ActorDB::FindAllActors)
			.addFunction("Instantiate", &ActorDB::Instantiate)
			.addFunction("Destroy", &ActorDB::Destroy)
			.endNamespace();
}

void ActorDB::post_update() {
	for (std::shared_ptr<Actor> a : actors) {
		a->post_update();
	}
	if (!to_add.empty() || !to_destroy.empty()) {
		for (std::shared_ptr<Actor> b : to_destroy) {
			auto it = std::find(actors.begin(), actors.end(), b);
			if (it != actors.end()) {
				std::shared_ptr<Actor>& to_remove = *it;
				to_remove->OnDestroy();
				actors.erase(it);
			}
		}
		for (std::shared_ptr<Actor> a : to_add) {
			actors.push_back(a);
		}

		to_destroy.clear();
	}
}

void ActorDB::pre_update() {
	for (std::shared_ptr<Actor> a : actors) {
		a->pre_update();
	}
	for (std::shared_ptr<Actor> act : to_add) {
		act->OnStart();
	}
	to_add.clear();
}

std::vector<std::shared_ptr<Actor>> ActorDB::get_surviving_actors() {
	std::vector<std::shared_ptr<Actor>> ret;
	for (std::shared_ptr<Actor> a : actors) {
		if (a->to_survive_next) {
			ret.push_back(a);
		}
	}
	return ret;
}

void ActorDB::append_new_actors(std::vector<std::shared_ptr<Actor>> v) {
	actors.insert(actors.begin(), v.begin(), v.end());

}

void ActorDB::clear() {
	actors.clear();
}

#endif
