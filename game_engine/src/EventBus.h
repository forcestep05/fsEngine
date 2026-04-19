#ifndef EVENTBUS_H
#define EVENTBUS_H


#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <queue>

#include "lua.hpp"
#include "LuaBridge.h"


using LuaRef = luabridge::LuaRef;
// first is the component, second is the function //
using EventMap = std::unordered_map<std::string, std::vector<std::pair<LuaRef, LuaRef>>>;
using RefPair = std::pair<LuaRef, LuaRef>;

struct SubscriptionRequest {
	SubscriptionRequest() {};
	SubscriptionRequest(bool sub, std::string evnt) {
		is_sub = sub;
		event = evnt;
	};
	bool is_sub = false;
	std::string event = "";
};

class EventBus {
public:
	EventBus() {};

	static void Publish(std::string event_type, LuaRef event_object);
	static void Subscribe(std::string event, LuaRef component, LuaRef function);
	static void Unsubscribe(std::string event, LuaRef component, LuaRef function);

	void activate_requests();

	static inline  std::queue<SubscriptionRequest> requests_this_frame;
	static inline std::queue<RefPair> luarefs_this_frame;
	static inline EventMap events;
	static inline lua_State* state;

};

#endif
