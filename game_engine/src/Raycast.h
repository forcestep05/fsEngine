#ifndef RAYCAST_H
#define RAYCAST_H

#include <algorithm>
#include <vector>
#include <tuple>

#include "box2d/box2d.h"

using CastInfo = std::tuple<b2Fixture*, b2Vec2, b2Vec2, float>;

class RaycastCallback : public b2RayCastCallback {

public:
	std::vector<CastInfo> found_fixtures;

	float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override {
		if (fixture->GetFilterData().categoryBits == 2) { return -1; }
		CastInfo to_push = std::make_tuple(fixture, point, normal, fraction);
		found_fixtures.push_back(to_push);
		return fraction;
	}

	CastInfo* find_closest() {
		if (found_fixtures.empty()) { return nullptr; }
		std::sort(found_fixtures.begin(), found_fixtures.end(), [](const CastInfo& a, const CastInfo& b) {
			return std::get<3>(a) < std::get<3>(b); });
		return &found_fixtures.front();
	}
	std::vector<CastInfo>* find_all() {
		if (found_fixtures.empty()) { return nullptr; }
		std::sort(found_fixtures.begin(), found_fixtures.end(), [](const CastInfo& a, const CastInfo& b) {
			return std::get<3>(a) < std::get<3>(b); });
		return &found_fixtures;
	}

	void clear() {found_fixtures.clear();}
};

#endif