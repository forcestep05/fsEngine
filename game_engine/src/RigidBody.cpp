#include "RigidBody.h"

#include "glm/glm.hpp"

RigidBody::RigidBody() {
	return;
}

RigidBody::RigidBody(const RigidBody& copy) {
	x = copy.x;
	y = copy.y;
	body_type = copy.body_type;
	precise = copy.precise;
	gravity_scale = copy.gravity_scale;
	density = copy.density;
	angular_friction = copy.angular_friction;
	rotation = copy.rotation;
	has_collider = copy.has_collider;
	has_trigger = copy.has_trigger;

	world = copy.world;
	body = nullptr;
	collider = nullptr;
	trigger = nullptr;


	height = copy.height;
	width = copy.width;
	radius = copy.radius;
	friction = copy.friction;
	bounciness = copy.bounciness;
	collider_type = copy.collider_type;

}


b2Vec2 RigidBody::GetPosition() { 
	if (body == nullptr) { return b2Vec2(x, y); }
	return body->GetPosition(); 
}

float RigidBody::GetRotation() {
	if (body == nullptr) {
		return rotation;
	}
	float rads = body->GetAngle();
	float deg = rads * (180.0f / b2_pi);
	return deg;
}

b2Vec2 RigidBody::GetVelocity() { return body->GetLinearVelocity(); }

float RigidBody::GetAngularVelocity() { 
	float rads = body->GetAngularVelocity(); 
	float degs = rads * (180.0f / b2_pi);
	return degs;
}

float RigidBody::GetGravityScale() { return body->GetGravityScale(); }

b2Vec2 RigidBody::GetUpDirection() {
	float angle = body->GetAngle() - (b2_pi / 2.0f);
	b2Vec2 result = b2Vec2(glm::cos(angle), glm::sin(angle));
	result.Normalize();
	return result;
}

b2Vec2 RigidBody::GetRightDirection() {
	float angle = body->GetAngle();
	b2Vec2 result = b2Vec2(glm::cos(angle), glm::sin(angle));
	result.Normalize();
	return result;
}

void RigidBody::AddForce(b2Vec2 vec) { body->ApplyForceToCenter(vec, true); }

void RigidBody::SetVelocity(b2Vec2 vec) { body->SetLinearVelocity(vec); }

void RigidBody::SetPosition(b2Vec2 vec) { 
	if (body == nullptr) { x = vec.x; y = vec.y; return; }
	body->SetTransform(vec, rotation);
}

void RigidBody::SetGravityScale(float scale) { body->SetGravityScale(scale); }

void RigidBody::SetAngularVelocity(float vel) { 
	float rads = vel * (b2_pi / 180.0f);
	body->SetAngularVelocity(rads); 
}

void RigidBody::SetRotation(float ang) {
	if (body == nullptr) { rotation = ang; return; }
	float rads = ang * (b2_pi / 180.0f);
	body->SetTransform(body->GetPosition(), rads);
}

void RigidBody::SetUpDirection(b2Vec2 dir) {
	dir.Normalize();
	float new_angle = glm::atan(dir.x, -dir.y);
	body->SetTransform(body->GetPosition(), new_angle);
}

void RigidBody::SetRightDirection(b2Vec2 dir) {
	dir.Normalize();
	float new_angle = glm::atan(dir.x, -dir.y) - (b2_pi / 2.0f);
	body->SetTransform(body->GetPosition(), new_angle);
}

void RigidBody::OnStart() {
	b2BodyDef def;
	// Set body type
	if (body_type == "dynamic") {
		def.type = b2_dynamicBody;
	}
	else if (body_type == "static") {
		def.type = b2_staticBody;
	}
	else if (body_type == "kinematic") {
		def.type = b2_kinematicBody;
	}
	def.bullet = precise;
	def.gravityScale = gravity_scale;
	def.angularDamping = angular_friction;
	body = world->CreateBody(&def);
	if (!has_collider && !has_trigger) {
		b2PolygonShape phantom;
		phantom.SetAsBox(width * 0.5f, height * 0.5f);
		b2FixtureDef def;
		def.filter.categoryBits = 2;
		def.shape = &phantom;
		def.density = density;
		def.isSensor = true;
		body->CreateFixture(&def);
	}
	else {
		if (has_collider) {
			b2PolygonShape box;
			box.SetAsBox(width * 0.5f, height * 0.5f);

			b2CircleShape circle;
			circle.m_radius = radius;

			b2FixtureDef fixture;
			fixture.filter.categoryBits = 1;
			if (collider_type == "box") { fixture.shape = &box; }
			else if (collider_type == "circle") { fixture.shape = &circle; }
			fixture.density = density;
			fixture.isSensor = false;
			fixture.friction = friction;
			fixture.restitution = bounciness;
			collider = body->CreateFixture(&fixture);
		}

		if (has_trigger) {
			b2PolygonShape trigger_box;
			trigger_box.SetAsBox(trigger_width * 0.5f, trigger_height * 0.5f);

			b2CircleShape trigger_circle;
			trigger_circle.m_radius = trigger_radius;

			b2FixtureDef trigger_fixture;
			trigger_fixture.filter.categoryBits = 3;
			trigger_fixture.isSensor = true;
			if (trigger_type == "box") { trigger_fixture.shape = &trigger_box; }
			else if (trigger_type == "circle") { trigger_fixture.shape = &trigger_circle; }
			trigger_fixture.density = density;
			trigger_fixture.friction = friction;
			trigger_fixture.restitution = bounciness;
			trigger = body->CreateFixture(&trigger_fixture);
		}
	}

	float rads = rotation * (b2_pi / 180.0f);
	body->SetTransform(b2Vec2(x, y), rads);

}

void RigidBody::OnDestroy() {
	world->DestroyBody(body);
}