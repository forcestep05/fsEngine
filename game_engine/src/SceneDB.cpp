#include <string>
#include <vector>

#include "SceneDB.h"
#include "actor.h"
#include "TemplateDB.h"
#include "Helper.h"

#include "glm/glm.hpp"
#include "rapidjson/document.h"
#include "lua.hpp"
#include "LuaBridge.h"
#include "box2d/box2d.h"

SceneDB::SceneDB(std::string scene, TemplateDB templates, ComponentDB& comps, lua_State* state, b2World * world) : templates(templates) {
	EngineUtils e;
	this->scene = scene;
	this->state = state;
	this->world = world;
	this->next_scene.first = false;
	std::string filepath = "resources/scenes/" + scene + ".scene";
	if (!std::filesystem::exists(filepath)) {
		std::cout << "error: scene " << scene << " is missing";
		exit(0);
	}
	rapidjson::Document data;
	e.ReadJsonFile(filepath, data);
	actors.state = state;
	actors.comps = comps;
	actors.templates = templates;
	const rapidjson::Value& act = data["actors"];
	for (rapidjson::SizeType i = 0; i < act.Size(); i++) {
		std::shared_ptr<Actor> new_actor = std::make_shared<Actor>();
		new_actor->state = state;
		new_actor->comps = &comps;
		const rapidjson::Value& a = act[i];
		bool hasTemplate = false;
		if (a.HasMember("template")) {
			new_actor = templates.check_templates(a["template"].GetString(), comps, state);
			hasTemplate = true;
		}
		if (a.HasMember("name")) {
			new_actor->name = a["name"].GetString();
		}
		if (a.HasMember("components")) {
			const rapidjson::Value& size = a["components"];
			for (auto it = size.MemberBegin(); it != size.MemberEnd(); ++it) {
				std::string key = it->name.GetString();
				const rapidjson::Value& obj = it->value;
				
				bool find = false;
				size_t j = 0;
				if (hasTemplate) {
					Component to_edit;
					for (int i = 0; i < new_actor->components.size(); i++) {
						if (new_actor->components[i].key == key) {
							find = true;
							j = i;
							to_edit = new_actor->components[i];
							break;
						}
					}
					if (find) {
						if (obj.HasMember("type")) {
							std::string type = obj["type"].GetString();
							Component new_comp = comps.find_component(type, state);
							to_edit.compRef = new_comp.compRef;
							to_edit.type = type;
						}
						for (auto itr = obj.MemberBegin(); itr != obj.MemberEnd(); ++itr) {
							std::string k = itr->name.GetString();
							if (k == "type") { continue; }
							const rapidjson::Value& v = itr->value;
							if (v.IsString()) { (*to_edit.compRef)[k] = v.GetString(); }
							else if (v.IsBool()) { (*to_edit.compRef)[k] = v.GetBool(); }
							else if (v.IsFloat()) { (*to_edit.compRef)[k] = v.GetFloat(); }
							else if (v.IsInt()) { (*to_edit.compRef)[k] = v.GetInt(); }
						}
						new_actor->components[j] = to_edit;

					}
				}
				if(!hasTemplate || !find) {
					std::string type = obj["type"].GetString();
					Component comp = comps.find_component(type, state);
					comp.key = key;
					if (type != "Rigidbody" && type != "ParticleSystem") {
						(*comp.compRef)["key"] = comp.key;
					}
					else {
						new_actor->has_rigidbody = true;
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
					if (type != "Rigidbody" && type != "ParticleSystem") {
						new_actor->inject_reference(comp.compRef);
					}
					
					new_actor->components.push_back(comp);
				} 
				
				
			}
		}
		new_actor->id = i;
		std::sort(new_actor->components.begin(), new_actor->components.end(), [](const Component& a, const Component& b) {
																					return a.key < b.key;});
		actors.add(new_actor);
	}
	actors.getNamespaceRunning();
	actors.current_id = static_cast<int>(act.Size());
}

void SceneDB::process_collision_enter(b2Contact* contact) {
	b2Fixture* a = contact->GetFixtureA();
	b2Fixture* b = contact->GetFixtureB();
	b2Filter a_filt = a->GetFilterData();
	b2Filter b_filt = b->GetFilterData();
	if (a_filt.categoryBits != b_filt.categoryBits) { return; }
	std::shared_ptr<Actor> actor_a = actors.fixtures[a];
	std::shared_ptr<Actor> actor_b = actors.fixtures[b];

	b2WorldManifold man;
	contact->GetWorldManifold(&man);
	b2Vec2 fill = b2Vec2(-999.0f, -999.0f);
	b2Vec2 point = man.points[0];
	b2Vec2 normal = man.normal;
	b2Vec2 rel = a->GetBody()->GetLinearVelocity() - b->GetBody()->GetLinearVelocity();

	if (actor_a != nullptr && actor_b != nullptr) {
		if (a_filt.categoryBits == 1) {
			actor_a->OnCollisionEnter(actor_b.get(), point, rel, normal);
			actor_b->OnCollisionEnter(actor_a.get(), point, rel, normal);
		}
		else if (a_filt.categoryBits == 3) {
			actor_a->OnTriggerEnter(actor_b.get(), fill, rel, fill);
			actor_b->OnTriggerEnter(actor_a.get(), fill, rel, fill);
		}

	}

}

void SceneDB::process_collision_exit(b2Contact* contact) {
	b2Fixture* a = contact->GetFixtureA();
	b2Fixture* b = contact->GetFixtureB();
	b2Filter a_filt = a->GetFilterData();
	b2Filter b_filt = b->GetFilterData();
	if (a_filt.categoryBits != b_filt.categoryBits) { return; }
	b2Vec2 fill = b2Vec2(-999.0f, -999.0f);
	b2Vec2 rel = a->GetBody()->GetLinearVelocity() - b->GetBody()->GetLinearVelocity();
	std::shared_ptr<Actor> actor_a = actors.fixtures[a];
	std::shared_ptr<Actor> actor_b = actors.fixtures[b];
	if (actor_a != nullptr && actor_b != nullptr) {
		if (a_filt.categoryBits == 1) {
			actor_a->OnCollisionExit(actor_b.get(), fill, rel, fill);
			actor_b->OnCollisionExit(actor_a.get(), fill, rel, fill);
		}
		else if (a_filt.categoryBits == 3) {
			actor_a->OnTriggerExit(actor_b.get(), fill, rel, fill);
			actor_b->OnTriggerExit(actor_a.get(), fill, rel, fill);
		}

	}

}

luabridge::LuaRef SceneDB::RaycastAll(b2Vec2 pos, b2Vec2 dir, float dist) {

	if ((dir.x == 0.0f && dir.y == 0.0f) || dist <= 0.0f) { return luabridge::LuaRef(state); }
	b2Vec2 dest;
	dir.Normalize();
	dest = dist * dir;
	dest += pos;

	world->RayCast(&ray, pos, dest);
	std::vector<CastInfo>* ci_v = ray.find_all();
	luabridge::LuaRef table = luabridge::newTable(state);
	if (ci_v == nullptr) { return table; }
	for (int i = 0; i < ci_v->size(); i++) {
		CastInfo ci = ci_v->at(i);
		b2Fixture* fixture = std::get<0>(ci);
		std::shared_ptr<Actor> actor = actors.fixtures[fixture];
		bool is_trigger = (fixture->GetFilterData().categoryBits == 3);
		HitResult hit;
		hit.actor = actor.get();
		hit.is_trigger = is_trigger;
		hit.point = std::get<1>(ci);
		hit.normal = std::get<2>(ci);
		table[i + 1] = luabridge::LuaRef(state, hit);
	}
	ray.clear();
	return table;
}