#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include <string>
#include <memory>

#include "box2d/box2d.h"

class RigidBody
{
public:
	RigidBody();

	RigidBody(const RigidBody& copy);

	b2Vec2 GetPosition();
	float GetRotation();
	b2Vec2 GetVelocity();
	float GetAngularVelocity();
	float GetGravityScale();
	b2Vec2 GetUpDirection();
	b2Vec2 GetRightDirection();

	void AddForce(b2Vec2 vec);
	void SetVelocity(b2Vec2 vec);
	void SetPosition(b2Vec2 vec);
	void SetRotation(float ang);
	void SetAngularVelocity(float vel);
	void SetGravityScale(float scale);
	void SetUpDirection(b2Vec2 dir);
	void SetRightDirection(b2Vec2 dir);

	void OnStart();
	void OnDestroy();


	std::string key = "";
	float x = 0.0f;
	float y = 0.0f;
	std::string body_type = "dynamic";
	bool precise = true;
	float gravity_scale = 1.0f;
	float density = 1.0f;
	float angular_friction = 0.3f;
	float rotation = 0.0f;
	bool has_collider = true;
	bool has_trigger = true;
	b2Body* body = nullptr;
	b2Fixture* collider = nullptr;
	b2Fixture* trigger = nullptr;
	b2World* world = nullptr;

	std::string collider_type = "box";
	float height = 1.0f;
	float width = 1.0f;
	float radius = 0.5f;
	float friction = 0.3f;
	float bounciness = 0.3f;

	std::string trigger_type = "box";
	float trigger_width = 1.0f;
	float trigger_height = 1.0f;
	float trigger_radius = 0.5f;
};

#endif

