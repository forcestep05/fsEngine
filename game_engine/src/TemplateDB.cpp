#include "TemplateDB.h"

#include <string>
#include <unordered_map>
#include <iostream>
#include <filesystem>
#include <cstdlib>

#include "RigidBody.h"
#include "ParticleSystem.h"
#include "EngineUtils.h"

#include "rapidjson/document.h"
#include "lua.hpp"
#include "LuaBridge.h"

TemplateDB::TemplateDB(ComponentDB comps, lua_State* state) {
	EngineUtils e;
	if (std::filesystem::exists("resources/actor_templates")) {
		for (const auto& entry : std::filesystem::directory_iterator("resources/actor_templates")) {
			std::filesystem::path entry_path = entry.path();
			std::string filename = "resources/actor_templates/" + entry_path.filename().string();
			Actor a = set_actor_from_template(filename, e, comps, state);
			size_t cut = filename.find('.');
			template_map[filename.substr(26, cut - 26)] = a;
		}
	}

}

std::shared_ptr<Actor> TemplateDB::check_templates(std::string temp, ComponentDB& comps, lua_State* state) {
	if (template_map.find(temp) != template_map.end()) {
		Actor a = template_map[temp];
		for (Component& c : a.components) {
			if (c.type == "Rigidbody") {
				RigidBody* parent = c.compRef->cast<RigidBody*>();
				RigidBody loc = *parent;
				luabridge::LuaRef ref(state, loc);
				c.compRef = std::make_shared<luabridge::LuaRef>(ref);
			}
			else if (c.type == "ParticleSystem") {
				ParticleSystem* parent = c.compRef->cast<ParticleSystem*>();
				ParticleSystem loc = *parent;
				luabridge::LuaRef ref(state, loc);
				c.compRef = std::make_shared<luabridge::LuaRef>(ref);
			}
			else {
				luabridge::LuaRef new_table = luabridge::newTable(state);
				comps.establish_inheritance(new_table, *c.compRef, state);
				std::shared_ptr<luabridge::LuaRef> ptr = std::make_shared<luabridge::LuaRef>(new_table);
				c.compRef = ptr;
			}
		}
		std::shared_ptr<Actor> ret = std::make_shared<Actor>(a);
		for (Component& c : ret->components) {
			if (c.type == "Rigidbody" || c.type == "ParticleSystem") { continue; }
			ret->inject_reference(c.compRef);
		}
		return ret;
	}
	else {
		std::cout << "error: template " << temp << " is missing";
		exit(0);
	}
}

Actor TemplateDB::set_actor_from_template(std::string filename, EngineUtils e, ComponentDB& comps, lua_State* state) {
	rapidjson::Document doc;
	e.ReadJsonFile(filename, doc);
	Actor a;
	a.comps = &comps;
	if (doc.HasMember("name")) {
		a.name = doc["name"].GetString();
	}
	if (doc.HasMember("components")) {
		const rapidjson::Value& size = doc["components"];
		for (auto it = size.MemberBegin(); it != size.MemberEnd(); ++it) {
			std::string key = it->name.GetString();
			const rapidjson::Value& obj = it->value;
			std::string type = obj["type"].GetString();
			Component comp = comps.find_component(type, state);
			comp.type = type;
			comp.key = key;
			if (type != "Rigidbody" && type != "ParticleSystem") {
				(*comp.compRef)["key"] = comp.key;
			}
			for (auto itr = obj.MemberBegin(); itr != obj.MemberEnd(); ++itr) {
				std::string k = itr->name.GetString();
				if (k == "type") { continue; }
				const rapidjson::Value& v = itr->value;
				if (v.IsString()) { (*comp.compRef)[k] = v.GetString(); }
				else if (v.IsBool()) { (*comp.compRef)[k] = v.GetBool(); }
				else if (v.IsFloat()) { (*comp.compRef)[k] = v.GetFloat(); }
				else if (v.IsInt()) { (*comp.compRef)[k] = v.GetInt(); }
			}
			a.components.push_back(comp);
		}
	}
	return a;
}