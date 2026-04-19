#ifndef COMPONENTDB_H
#define COMPONENTDB_H

#include <string>
#include <unordered_map>

#include "Component.h"
#include "RigidBody.h"
#include "ParticleSystem.h"
#include "Renderer.h"

#include "lua.hpp"
#include "LuaBridge.h"
#include "box2d/box2d.h"

class ComponentDB
{
public:
	ComponentDB() {};
	ComponentDB(lua_State* state, b2World* world, Renderer* rend);

	Component find_component(std::string name, lua_State* state);
	void establish_inheritance(luabridge::LuaRef& instance_table, luabridge::LuaRef& parent_table, lua_State* state);
	lua_State* getState() { return state; }

private:
	std::unordered_map<std::string, Component> component_map;
	
	static void CppLog(std::string message);
	static void CppLogError(std::string message);
	void set_rigidbody();
	void set_particlesystem();
	void set_namespace();
	lua_State* state = nullptr;
	b2World* world = nullptr;
	Renderer* rend = nullptr;
};
#endif

