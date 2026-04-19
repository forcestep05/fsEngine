#ifndef ACTOR_H
#define ACTOR_H

#include <string>
#include <vector>
#include <algorithm>
#include <optional>

#include "Component.h"
#include "ComponentDB.h"

#include "glm/glm.hpp"
#include "lua.hpp"
#include "LuaBridge.h"
#include "box2d/box2d.h"

static inline int CREATED_ACTORS = 0;

struct Actor {
	std::string name = "";
	int id = 0;
	bool to_be_removed = false;
	bool to_survive_next = false;
	bool has_rigidbody = false;
	std::vector<Component> components;
	std::vector<Component> to_add;
	std::vector<Component> to_remove;
	lua_State* state;
	ComponentDB* comps;

	inline void OnStart();
	inline void OnUpdate();
	inline void OnLateUpdate();

	inline void OnCollisionEnter(Actor* other, b2Vec2 pt, b2Vec2 rel, b2Vec2 norm);
	inline void OnCollisionExit(Actor* other, b2Vec2 pt, b2Vec2 rel, b2Vec2 norm);

	inline void OnTriggerEnter(Actor* other, b2Vec2 pt, b2Vec2 rel, b2Vec2 norm);
	inline void OnTriggerExit(Actor* other, b2Vec2 pt, b2Vec2 rel, b2Vec2 norm);

	inline void OnDestroy();

	inline void post_update();
	inline void pre_update();

	std::string getName() { return name; }
	int getID() { return id; }

	inline void inject_reference(std::shared_ptr<luabridge::LuaRef> ref) { (*ref)["actor"] = this;}

	inline luabridge::LuaRef getComponentByKey(std::string key);
	inline luabridge::LuaRef getComponent(std::string type);
	inline luabridge::LuaRef getComponents(std::string type);
	inline void RemoveComponent(luabridge::LuaRef ref);
	inline luabridge::LuaRef AddComponent(std::string type);

	inline void print_exception(luabridge::LuaException e);

};

struct Collision {
	Actor* other = nullptr;
	b2Vec2 point = b2Vec2(0.0f, 0.0f);
	b2Vec2 relative_velocity = b2Vec2(0.0f, 0.0f);
	b2Vec2 normal = b2Vec2(0.0f, 0.0f);
};

void Actor::OnStart() { 
	if (to_survive_next) { return; }
	for (Component& c : components) { 
		if (!c.hasStart || !c.isEnabled()) { continue; }
		std::shared_ptr<luabridge::LuaRef> ref = c.compRef;
		try {
			(*ref)["OnStart"](*ref);
		}
		catch (luabridge::LuaException e) {
			print_exception(e);
		}
	} 
}
void Actor::OnUpdate() {
	for (Component c : components) {
		if (!c.hasUpdate || !c.isEnabled()) { continue; }
		std::shared_ptr<luabridge::LuaRef> ref = c.compRef;
		try {
			(*ref)["OnUpdate"](*ref);
		}
		catch (luabridge::LuaException e) {
			print_exception(e);
		}
	}
}
void Actor::OnLateUpdate() {
	for (Component c : components) {
		if (!c.hasLateUpdate || !c.isEnabled()) { continue; }
		std::shared_ptr<luabridge::LuaRef> ref = c.compRef;
		try {
			(*ref)["OnLateUpdate"](*ref);
		}
		catch (luabridge::LuaException e) {
			print_exception(e);
		}
	}
}

void Actor::OnDestroy() {
	for (Component c : components) {
		if (!c.hasDestroy) { continue; }
		std::shared_ptr<luabridge::LuaRef> ref = c.compRef;
		try {
			(*ref)["OnDestroy"](*ref);
		}
		catch (luabridge::LuaException e) {
			print_exception(e);
		}
	}
}
void Actor::OnCollisionEnter(Actor* other, b2Vec2 pt, b2Vec2 rel, b2Vec2 norm) {
	Collision col;
	col.normal = norm;
	col.point = pt;
	col.relative_velocity = rel;
	col.other = other;

	for (Component c : components) {
		if (!c.hasCollisionEnter || !c.isEnabled()) { continue; }
		std::shared_ptr<luabridge::LuaRef> ref = c.compRef;
		try {
			(*ref)["OnCollisionEnter"](*ref, col);
		}
		catch (luabridge::LuaException e) {
			print_exception(e);
		}
	}
}
void Actor::OnCollisionExit(Actor* other, b2Vec2 pt, b2Vec2 rel, b2Vec2 norm) {
	Collision col;
	col.normal = norm;
	col.point = pt;
	col.relative_velocity = rel;
	col.other = other;
	for (Component c : components) {
		if (!c.hasCollisionExit || !c.isEnabled()) { continue; }
		std::shared_ptr<luabridge::LuaRef> ref = c.compRef;
		try {
			(*ref)["OnCollisionExit"](*ref, col);
		}
		catch (luabridge::LuaException e) {
			print_exception(e);
		}
	}
}
void Actor::OnTriggerEnter(Actor* other, b2Vec2 pt, b2Vec2 rel, b2Vec2 norm) {
	Collision col;
	col.normal = norm;
	col.point = pt;
	col.relative_velocity = rel;
	col.other = other;

	for (Component c : components) {
		if (!c.hasTriggerEnter || !c.isEnabled()) { continue; }
		std::shared_ptr<luabridge::LuaRef> ref = c.compRef;
		try {
			(*ref)["OnTriggerEnter"](*ref, col);
		}
		catch (luabridge::LuaException e) {
			print_exception(e);
		}
	}
}
void Actor::OnTriggerExit(Actor* other, b2Vec2 pt, b2Vec2 rel, b2Vec2 norm) {
	Collision col;
	col.normal = norm;
	col.point = pt;
	col.relative_velocity = rel;
	col.other = other;
	for (Component c : components) {
		if (!c.hasTriggerExit || !c.isEnabled()) { continue; }
		std::shared_ptr<luabridge::LuaRef> ref = c.compRef;
		try {
			(*ref)["OnTriggerExit"](*ref, col);
		}
		catch (luabridge::LuaException e) {
			print_exception(e);
		}
	}
}
luabridge::LuaRef Actor::getComponentByKey(std::string key) {
	bool find = false;
	Component a;
	for (Component c : components) {
		if (c.key == key && !c.to_remove) {
			a = c;
			find = true;
			break;
		}
	}
	if (find) {
		return *a.compRef;
	}
	else {
		return luabridge::LuaRef(state);
	}
}

luabridge::LuaRef Actor::getComponent(std::string type) {
	bool find = false;
	Component a;
	for (Component c : components) {
		if (c.type == type && !c.to_remove) {
			a = c;
			find = true;
			break;
		}
	}
	if (find) {
		return *a.compRef;
	}
	else {
		return luabridge::LuaRef(state);
	}
}

luabridge::LuaRef Actor::getComponents(std::string type) {
	bool find = false;
	std::vector<Component> A;
	for (Component c : components) {
		if (c.type == type && !c.to_remove) {
			A.push_back(c);
			find = true;
		}
	}
	luabridge::LuaRef table = luabridge::newTable(state);
	for (int i = 0; i < A.size(); i++) {
		table[i + 1] = *A[i].compRef;
	}
	return table;
}

void Actor::print_exception(luabridge::LuaException e) {
	std::string error_message = e.what();

	std::replace(error_message.begin(), error_message.end(), '\\', '/');

	std::cout << "\033[31m" << name << " : " << error_message << "\033[0m" << std::endl;
}

luabridge::LuaRef Actor::AddComponent(std::string type) {
	Component add = comps->find_component(type, state);
	add.key = "r" + std::to_string(CREATED_ACTORS);
	if(type != "Rigidbody") { (*add.compRef)["key"] = add.key; }
	CREATED_ACTORS++;
	to_add.push_back(add);
	return *add.compRef;
}

void Actor::RemoveComponent(luabridge::LuaRef ref) {
	bool find = false;
	Component a;
	for (Component &c : components) {
		if (*(c.compRef) == ref) {
			a = c; find = true;
			c.to_remove = true;
			if (c.type != "Rigidbody") {
				(*c.compRef)["enabled"] = false;
			}
			break;
		}
	}
	if (find) {
		to_remove.push_back(a);
	}
}

void Actor::post_update() {
	
	if (!to_remove.empty() || !to_add.empty()) {
		if (!to_remove.empty()) {
			for (Component c : to_remove) {
				auto it = std::find(components.begin(), components.end(), c);
				if (it != components.end()) {
					Component &to_destroy = *it;
					if (to_destroy.hasDestroy) {
						(*to_destroy.compRef)["OnDestroy"](*to_destroy.compRef);
					}
					components.erase(it);
				}
			}
		}
		if (!to_add.empty()) {
			for (Component& a : to_add) {
				components.push_back(a);
			}
		}
		std::sort(components.begin(), components.end(), [](const Component a, const Component b) {
			return a.key < b.key; });
	}
	to_remove.clear();
}

void Actor::pre_update() {
	if (to_add.empty()) { return; }
	for (Component c : to_add) {
		if (c.hasStart && c.isEnabled()) {
			(*c.compRef)["OnStart"](*c.compRef);
		}
	}
	to_add.clear();
}

static inline bool operator==(const std::shared_ptr<Actor> a, const std::shared_ptr<Actor> b) { return a->id == b->id; }

#endif
