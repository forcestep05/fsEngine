#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include <string>
#include <vector>
#include <queue>

#include "ImageDB.h"
#include "Helper.h"
#include "glm/glm.hpp"

struct Particle {
	float x = 0.0f;
	float y = 0.0f;
	float v_x = 0.0f;
	float v_y = 0.0f;
	float v_t = 0.0f;
	int r = 255;
	int g = 255;
	int b = 255;
	int a = 255;
	int init_r = 255;
	int init_g = 255;
	int init_b = 255;
	int init_a = 255;
	float scale = 1.0f;
	float init_scale = 1.0f;
	float rotation = 0.0f;
	int age = 0;
	bool enabled = true;

};
struct ParticleSystem {
	inline void OnStart();
	inline void OnUpdate();

	inline void create_particles();
	inline void render_particles();

	inline void Stop();
	inline void Play();
	inline void Burst();

	
	std::string file = "";
	float x = 0.0f;
	float y = 0.0f;
	int frames_between_bursts = 1;
	int burst_quantity = 1;
	int duration_frames = 300;
	int frame_num = 0;
	bool enabled = true;
	bool emission_enabled = true;
	

	float emit_angle_min = 0.0f;
	float emit_angle_max = 360.0f;
	float emit_radius_min = 0.0f;
	float emit_radius_max = 0.5f;

	float start_scale_min = 1.0f;
	float start_scale_max = 1.0f;
	float rotation_min = 0.0f;
	float rotation_max = 0.0f;

	float start_speed_min = 0.0f;
	float start_speed_max = 0.0f;
	float rotation_speed_min = 0.0f;
	float rotation_speed_max = 0.0f;

	float gravity_scale_x = 0.0f;
	float gravity_scale_y = 0.0f;
	float drag_factor = 1.0f;
	float angular_drag_factor = 1.0f;
	float end_scale = -1.0f;
	bool has_end_scale = false;


	int start_color_r = 255;
	int start_color_g = 255;
	int start_color_b = 255;
	int start_color_a = 255;

	int end_color_r = -1;
	int end_color_g = -1;
	int end_color_b = -1;
	int end_color_a = -1;
	bool has_end_color = false;


	int sorting_order = 9999;
	std::queue<int> free_spaces;
	RandomEngine emit_angle;
	RandomEngine emit_radius;
	RandomEngine start_scale;
	RandomEngine rotation;
	RandomEngine start_speed;
	RandomEngine rotation_speed;
	ImageDB* IMG = nullptr;
	std::vector<Particle> particles;

};

void ParticleSystem::OnStart() {
	emit_angle = RandomEngine(emit_angle_min, emit_angle_max, 298);
	emit_radius = RandomEngine(emit_radius_min, emit_radius_max, 404);
	start_scale = RandomEngine(start_scale_min, start_scale_max, 494);
	rotation = RandomEngine(rotation_min, rotation_max, 440);
	start_speed = RandomEngine(start_speed_min, start_speed_max, 498);
	rotation_speed = RandomEngine(rotation_speed_min, rotation_speed_max, 305);

	if (frames_between_bursts < 1) { frames_between_bursts = 1; }
	if (burst_quantity < 1) { burst_quantity = 1; }
	if (duration_frames < 1) { duration_frames = 1; }
	if (end_scale != -1.0f) { has_end_scale = true; }
	if (end_color_r != -1 ||
		end_color_g != -1 ||
		end_color_b != -1 ||
		end_color_a != -1) {
		has_end_color = true;
		if (end_color_a == -1) { end_color_a = start_color_a; }
		if (end_color_r == -1) { end_color_r = start_color_r; }
		if (end_color_g == -1) { end_color_g = start_color_g; }
		if (end_color_b == -1) { end_color_b = start_color_b; }
	}
}

void ParticleSystem::OnUpdate() {
	if (!enabled) { return; }
	// burst code
	if (frame_num % frames_between_bursts == 0 && emission_enabled) {
		create_particles();
	}
	render_particles();
	frame_num++;
}

void ParticleSystem::create_particles() {
	for (int i = 0; i < burst_quantity; i++) {
		Particle a;
		float rads = glm::radians(emit_angle.Sample());
		float radius = emit_radius.Sample();
		float cos = glm::cos(rads);
		float sin = glm::sin(rads);
		float speed = start_speed.Sample();
		a.x = x + cos * radius;
		a.y = y + sin * radius;
		a.v_x = cos * speed;
		a.v_y = sin * speed;
		a.v_t = rotation_speed.Sample();

		a.r = start_color_r;
		a.g = start_color_g;
		a.b = start_color_b;
		a.a = start_color_a;
		a.init_r = a.r;
		a.init_g = a.g;
		a.init_b = a.b;
		a.init_a = a.a;

		a.scale = start_scale.Sample();
		a.init_scale = a.scale;
		a.rotation = rotation.Sample();
		if (!free_spaces.empty()) {
			int i = free_spaces.front();
			particles[i] = a;
			free_spaces.pop();
		}
		else {
			particles.push_back(a);
		}
	}
}
void ParticleSystem::render_particles() {
	int sz = particles.size();
	// activate rendering
	for (int i = 0; i < sz; i++) {
		if (particles[i].age >= duration_frames && particles[i].enabled) {
			particles[i].enabled = false;
			free_spaces.push(i);
		}
		if (!particles[i].enabled) { continue; }

		particles[i].v_x += gravity_scale_x;
		particles[i].v_y += gravity_scale_y;

		particles[i].v_x *= drag_factor;
		particles[i].v_y *= drag_factor;
		particles[i].v_t *= angular_drag_factor;

		particles[i].x += particles[i].v_x;
		particles[i].y += particles[i].v_y;
		particles[i].rotation += particles[i].v_t;
		float age_frac = static_cast<float>(particles[i].age) / duration_frames;
		if (has_end_scale) {
			particles[i].scale = glm::mix(particles[i].init_scale, end_scale, age_frac);
		}
		if (has_end_color) {
			particles[i].r = glm::mix(particles[i].init_r, end_color_r, age_frac);
			particles[i].g = glm::mix(particles[i].init_g, end_color_g, age_frac);
			particles[i].b = glm::mix(particles[i].init_b, end_color_b, age_frac);
			particles[i].a = glm::mix(particles[i].init_a, end_color_a, age_frac);
		}

		IMG->QueueDrawParticle(file, particles[i].x, particles[i].y, particles[i].rotation,
			particles[i].scale, particles[i].scale, 0.5f, 0.5f,
			particles[i].r, particles[i].g, particles[i].b, particles[i].a, sorting_order);

		particles[i].age++;
	}
}

void ParticleSystem::Stop() {
	emission_enabled = false;
}

void ParticleSystem::Play() {
	emission_enabled = true;
}

void ParticleSystem::Burst() {
	create_particles();
	render_particles();

}


#endif