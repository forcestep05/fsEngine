#ifndef TEMPLATEDB_H
#define TEMPLATEDB_H

#include <string>
#include <unordered_map>

#include "actor.h"
#include "ComponentDB.h"
#include "EngineUtils.h"

#include "lua.hpp"
class TemplateDB
{
public:

	TemplateDB() {};

	TemplateDB(ComponentDB comps, lua_State* state);

	std::shared_ptr<Actor> check_templates(std::string temp, ComponentDB& comps, lua_State* state);

private:
	std::unordered_map<std::string, Actor> template_map;
	Actor set_actor_from_template(std::string filename, EngineUtils e, ComponentDB& comps, lua_State* state);

};

#endif
