#include "ComponentDB.h"

#include <string>
#include <iostream>
#include <filesystem>

#include "Component.h"
#include "actor.h"
#include "RigidBody.h"

#include "lua.hpp"
#include "LuaBridge.h"


ComponentDB::ComponentDB(lua_State* state, b2World* world, Renderer* rend) : world(world), rend(rend) {
	this->state = state;
	
	set_namespace();
	set_rigidbody();
	set_particlesystem();

	if (!std::filesystem::exists("resources/component_types")) { return; }
	for (const auto& entry : std::filesystem::directory_iterator("resources/component_types")) {
		std::string filename = entry.path().filename().string();
		std::string filepath = entry.path().string();
		size_t cut = filename.find(".");
		std::string f = filename.substr(0, cut);
		int err = luaL_dofile(state, filepath.c_str());
		if (err) {
			std::cout << "problem with lua file " << f;
			exit(0);
		}

		luabridge::LuaRef original_table = luabridge::getGlobal(state, f.c_str());
		luabridge::LuaRef new_table = luabridge::newTable(state);
		establish_inheritance(new_table, original_table, state);
		Component c;
		c.type = f;
		if (!original_table["OnStart"].isNil()) {
			c.hasStart = true;
		}
		if (!original_table["OnUpdate"].isNil()) {
			c.hasUpdate = true;
		}
		if (!original_table["OnLateUpdate"].isNil()) {
			c.hasLateUpdate = true;
		}
		if (!original_table["OnCollisionEnter"].isNil()) {
			c.hasCollisionEnter = true;
		}
		if (!original_table["OnCollisionExit"].isNil()) {
			c.hasCollisionExit = true;
		}
		if (!original_table["OnTriggerEnter"].isNil()) {
			c.hasTriggerEnter = true;
		}
		if (!original_table["OnTriggerExit"].isNil()) {
			c.hasTriggerExit = true;
		}
		if (!original_table["OnDestroy"].isNil()) {
			c.hasDestroy = true;
		}
		c.compRef = std::make_shared<luabridge::LuaRef>(new_table);
		if (original_table["enabled"].isNil()) {
			(*c.compRef)["enabled"] = true;
		}
		component_map[f] = c;
	}
}

void ComponentDB::establish_inheritance(luabridge::LuaRef& instance_table, luabridge::LuaRef& parent_table, lua_State* state) {
	luabridge::LuaRef new_metatable = luabridge::newTable(state);
	new_metatable["__index"] = parent_table;

	instance_table.push(state);
	new_metatable.push(state);
	lua_setmetatable(state, -2);
	lua_pop(state, 1);
}

Component ComponentDB::find_component(std::string name, lua_State* state) {
	if (component_map.find(name) == component_map.end()) {
		std::cout << "error: failed to locate component " << name;
		exit(0);
	}
	Component a = component_map[name];
	Component b = a;
	if (name == "Rigidbody") {
		RigidBody* parent = a.compRef->cast<RigidBody*>();
		RigidBody loc = *parent;
		luabridge::LuaRef ref(state, loc);
		b.compRef = std::make_shared<luabridge::LuaRef>(ref);
		return b;
	}
	else if (name == "ParticleSystem") {
		ParticleSystem* parent = a.compRef->cast<ParticleSystem*>();
		ParticleSystem loc = *parent;
		luabridge::LuaRef ref(state, loc);
		b.compRef = std::make_shared<luabridge::LuaRef>(ref);
		return b;
	}
	luabridge::LuaRef new_table = luabridge::newTable(state);
	establish_inheritance(new_table, *a.compRef, state);
	b.compRef = std::make_shared<luabridge::LuaRef>(new_table);
	return b;
}

void ComponentDB::CppLog(std::string message) {
	std::cout << message << "\n";
}
void ComponentDB::CppLogError(std::string message) {
	std::cout << message << "\n";
}

void ComponentDB::set_rigidbody() {
	Component rigidbody_comp;
	rigidbody_comp.hasStart = true;
	rigidbody_comp.hasDestroy = true;
	rigidbody_comp.type = "Rigidbody";
	RigidBody loc;
	loc.world = world;
	luabridge::LuaRef ref(state, loc);
	rigidbody_comp.compRef = std::make_shared<luabridge::LuaRef>(ref);
	component_map["Rigidbody"] = rigidbody_comp;

}

void ComponentDB::set_particlesystem() {
	Component ps;
	ps.hasUpdate = true;
	ps.hasStart = true;
	ps.type = "ParticleSystem";
	ParticleSystem p;
	p.IMG = &rend->IMG;
	luabridge::LuaRef ref(state, p);
	ps.compRef = std::make_shared<luabridge::LuaRef>(ref);
	component_map["ParticleSystem"] = ps;

}

void ComponentDB::set_namespace() {
	luabridge::getGlobalNamespace(state)
		.beginNamespace("Debug")
		.addFunction("Log", &ComponentDB::CppLog)
		.addFunction("LogError", &ComponentDB::CppLogError)
		.endNamespace();
	luabridge::getGlobalNamespace(state)
		.beginClass<Actor>("Actor")
		.addFunction("GetName", &Actor::getName)
		.addFunction("GetID", &Actor::getID)
		.addFunction("GetComponentByKey", &Actor::getComponentByKey)
		.addFunction("GetComponent", &Actor::getComponent)
		.addFunction("GetComponents", &Actor::getComponents)
		.addFunction("AddComponent", &Actor::AddComponent)
		.addFunction("RemoveComponent", &Actor::RemoveComponent)
		.endClass();
	luabridge::getGlobalNamespace(state)
		.beginClass<Collision>("collision")
		.addProperty("other", &Collision::other)
		.addProperty("point", &Collision::point)
		.addProperty("relative_velocity", &Collision::relative_velocity)
		.addProperty("normal", &Collision::normal)
		.endClass();
	luabridge::getGlobalNamespace(state)
		.beginClass<RigidBody>("Rigidbody")
		.addConstructor<void (*) (void)>()
		// general properties
		.addProperty("x", &RigidBody::x)
		.addProperty("y", &RigidBody::y)
		.addProperty("precise", &RigidBody::precise)
		.addProperty("angular_friction", &RigidBody::angular_friction)
		.addProperty("body_type", &RigidBody::body_type)
		.addProperty("gravity_scale", &RigidBody::gravity_scale)
		.addProperty("density", &RigidBody::density)
		.addProperty("rotation", &RigidBody::rotation)
		.addProperty("has_collider", &RigidBody::has_collider)
		.addProperty("has_trigger", &RigidBody::has_trigger)
		// collider properties
		.addProperty("height", &RigidBody::height)
		.addProperty("width", &RigidBody::width)
		.addProperty("radius", &RigidBody::radius)
		.addProperty("friction", &RigidBody::friction)
		.addProperty("bounciness", &RigidBody::bounciness)
		.addProperty("collider_type", &RigidBody::collider_type)
		// trigger properties
		.addProperty("trigger_type", &RigidBody::trigger_type)
		.addProperty("trigger_width", &RigidBody::trigger_width)
		.addProperty("trigger_height", &RigidBody::trigger_height)
		.addProperty("trigger_radius", &RigidBody::trigger_radius)
		// member functions
		.addFunction("GetPosition", &RigidBody::GetPosition)
		.addFunction("GetRotation", &RigidBody::GetRotation)
		.addFunction("GetVelocity", &RigidBody::GetVelocity)
		.addFunction("GetAngularVelocity", &RigidBody::GetAngularVelocity)
		.addFunction("GetGravityScale", &RigidBody::GetGravityScale)
		.addFunction("GetUpDirection", &RigidBody::GetUpDirection)
		.addFunction("GetRightDirection", &RigidBody::GetRightDirection)
		.addFunction("AddForce", &RigidBody::AddForce)
		.addFunction("SetPosition", &RigidBody::SetPosition)
		.addFunction("SetRotation", &RigidBody::SetRotation)
		.addFunction("SetVelocity", &RigidBody::SetVelocity)
		.addFunction("SetAngularVelocity", &RigidBody::SetAngularVelocity)
		.addFunction("SetGravityScale", &RigidBody::SetGravityScale)
		.addFunction("SetUpDirection", &RigidBody::SetUpDirection)
		.addFunction("SetRightDirection", &RigidBody::SetRightDirection)
		.addFunction("OnStart", &RigidBody::OnStart)
		.addFunction("OnDestroy", &RigidBody::OnDestroy)
		.endClass();
	luabridge::getGlobalNamespace(state)
		.beginClass<Particle>("Particle")
		.endClass();
	luabridge::getGlobalNamespace(state)
		.beginClass<ParticleSystem>("ParticleSystem")
		.addFunction("OnStart", &ParticleSystem::OnStart)
		.addFunction("OnUpdate", &ParticleSystem::OnUpdate)
		// Test suite 1 //
		.addProperty("x", &ParticleSystem::x)
		.addProperty("y", &ParticleSystem::y)
		.addProperty("frames_between_bursts", &ParticleSystem::frames_between_bursts)
		.addProperty("burst_quantity", &ParticleSystem::burst_quantity)
		.addProperty("start_scale_min", &ParticleSystem::start_scale_min)
		.addProperty("start_scale_max", &ParticleSystem::start_scale_max)
		.addProperty("rotation_min", &ParticleSystem::rotation_min)
		.addProperty("rotation_max", &ParticleSystem::rotation_max)
		.addProperty("start_color_r", &ParticleSystem::start_color_r)
		.addProperty("start_color_g", &ParticleSystem::start_color_g)
		.addProperty("start_color_b", &ParticleSystem::start_color_b)
		.addProperty("start_color_a", &ParticleSystem::start_color_a)
		.addProperty("emit_radius_min", &ParticleSystem::emit_radius_min)
		.addProperty("emit_radius_max", &ParticleSystem::emit_radius_max)
		.addProperty("emit_angle_min", &ParticleSystem::emit_angle_min)
		.addProperty("emit_angle_max", &ParticleSystem::emit_angle_max)
		.addProperty("image", &ParticleSystem::file)
		.addProperty("sorting_order", &ParticleSystem::sorting_order)
		// Test Suite 2 //
		.addProperty("duration_frames", &ParticleSystem::duration_frames)
		.addProperty("start_speed_min", &ParticleSystem::start_speed_min)
		.addProperty("start_speed_max", &ParticleSystem::start_speed_max)
		.addProperty("rotation_speed_min", &ParticleSystem::rotation_speed_min)
		.addProperty("rotation_speed_max", &ParticleSystem::rotation_speed_max)
		.addProperty("gravity_scale_x", &ParticleSystem::gravity_scale_x)
		.addProperty("gravity_scale_y", &ParticleSystem::gravity_scale_y)
		.addProperty("drag_factor", &ParticleSystem::drag_factor)
		.addProperty("angular_drag_factor", &ParticleSystem::angular_drag_factor)
		.addProperty("end_scale", &ParticleSystem::end_scale)
		.addProperty("end_color_r", &ParticleSystem::end_color_r)
		.addProperty("end_color_g", &ParticleSystem::end_color_g)
		.addProperty("end_color_b", &ParticleSystem::end_color_b)
		.addProperty("end_color_a", &ParticleSystem::end_color_a)
		// Test Suite 3 //
		.addProperty("enabled", &ParticleSystem::enabled)
		.addFunction("Stop", &ParticleSystem::Stop)
		.addFunction("Play", &ParticleSystem::Play)
		.addFunction("Burst", &ParticleSystem::Burst)
		.endClass();
}