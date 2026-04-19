#ifndef CONTACTLISTENER_H
#define CONTACTLISTENER_H

#include <unordered_map>
#include <iostream>
#include <memory>

#include "SceneDB.h"

#include "box2d/box2d.h"


class ContactListener : public b2ContactListener
{
public:
	void BeginContact(b2Contact* contact) override {
		current_scene->process_collision_enter(contact);
	}

	void EndContact(b2Contact* contact) override {
		current_scene->process_collision_exit(contact);
	}

	SceneDB* current_scene = nullptr;
};

#endif

