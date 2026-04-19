#ifndef COMPONENT_H
#define COMPONENT_H

#include <memory>
#include <iostream>
#include <string>

#include "lua.hpp"
#include "LuaBridge.h"

class Component
{
public:
	explicit Component() {};
	inline bool isEnabled();

	std::shared_ptr<luabridge::LuaRef> compRef = nullptr;
	
	std::string key = "";
	std::string type = "";

	bool hasStart = false;
	bool hasUpdate = false;
	bool hasLateUpdate = false;
	bool hasDestroy = false;

	bool hasCollisionEnter = false;
	bool hasCollisionExit = false;

	bool hasTriggerEnter = false;
	bool hasTriggerExit = false;

	bool to_remove = false;
};

bool Component::isEnabled() {
	luabridge::LuaRef a = (*compRef)["enabled"];
	if (a.isNil()) {
		a = true;
		return true;
	}
	else {
		bool ret = a.cast<bool>();
		return ret;
	}
}
static inline bool operator==(const Component& a, const Component& b) {
	return a.key == b.key;
}

#endif

