#include "EventBus.h"

void EventBus::Publish(std::string event_type, LuaRef event_object) {
	std::vector<RefPair>& subbed = events[event_type];
	for (RefPair refs : subbed) {
		refs.second(refs.first, event_object);
	}
}

void EventBus::Subscribe(std::string event, LuaRef component, LuaRef function) {
	SubscriptionRequest sub(true, event);
	requests_this_frame.push(sub);
	luarefs_this_frame.push(RefPair(component, function));
}

void EventBus::Unsubscribe(std::string event, LuaRef component, LuaRef function) {
	SubscriptionRequest sub(false, event);
	requests_this_frame.push(sub);
	luarefs_this_frame.push(RefPair(component, function));
}

void EventBus::activate_requests() {
	if (requests_this_frame.empty()) { return; }
	while (!requests_this_frame.empty()) {
		SubscriptionRequest sub = requests_this_frame.front();
		RefPair refs = luarefs_this_frame.front();
		if (sub.is_sub) {
			events[sub.event].push_back(refs);
		}
		else {
			std::vector<RefPair> &subbed = events[sub.event];
			bool found = false;
			int place = -1;
			for (int i = 0; i < subbed.size(); i++) {
				if (subbed[i].first == refs.first && subbed[i].second == refs.second) {
					place = i;
					found = true;
					break;
				}
			}
			if (found) {
				subbed.erase(subbed.begin() + place);
			}
		}
		requests_this_frame.pop();
		luarefs_this_frame.pop();
	}
}