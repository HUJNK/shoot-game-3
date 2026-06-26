#ifndef PARTICLE_H
#define PARTICLE_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
#include <vector>
#include <ctime>
#include <cstdlib>
using namespace std;
#include "shader.h"

enum ParticleType {
	WATER,     // water splash - blue/white, soft gravity, 4-layer burst
	DEBRIS,    // debris fragments - brown/orange, heavy gravity, floor bounce
	MUZZLE,    // muzzle flash - yellow/white, tiny, no gravity, instant burst
	TRAIL      // ball trail - subtle blue, slow drift, tiny and short-lived
};

struct Particle {
	vec3 position;
	vec3 velocity;
	vec4 color;
	float size;
	float life;
	float maxLife;
	ParticleType type;
	bool hasBounced;
};

class ParticleSystem {
private:
	vector<Particle> particles;
	GLuint VAO, VBO_POS, VBO_COLOR, VBO_SIZE;
	Shader* shader;
	vec3 gravity;
	vec3 debrisGravity;
	float explosionSizeScale;

public:
	ParticleSystem() {
		shader = new Shader("res/shader/particle.vert", "res/shader/particle.frag");

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO_POS);
		glGenBuffers(1, &VBO_COLOR);
		glGenBuffers(1, &VBO_SIZE);
		gravity = vec3(0.0f, -7.0f, 0.0f);
		debrisGravity = vec3(0.0f, -18.0f, 0.0f);
		explosionSizeScale = 1.0f;
	}

	void SetExplosionSizeScale(float s) { explosionSizeScale = s; }

	void Explode(vec3 position, vec4 tintColor, int count = 80, ParticleType type = WATER) {
		switch (type) {
		case WATER:  ExplodeWater(position, tintColor, count); break;
		case DEBRIS: ExplodeDebris(position, tintColor, count); break;
		case MUZZLE: ExplodeMuzzle(position, count); break;
		default: break;
		}
	}

	void EmitTrail(vec3 position) {
		int n = 1 + rand() % 2;
		for (int i = 0; i < n; i++) {
			Particle p;
			p.position = position;
			p.velocity = vec3(
				(rand() % 100 - 50) / 200.0f,
				(rand() % 100 - 50) / 200.0f,
				(rand() % 100 - 50) / 200.0f
			);
			p.color = vec4(0.5f, 0.75f, 1.0f, 0.5f);
			p.size = 2.0f + (rand() % 100) / 50.0f;
			p.maxLife = 0.3f + (rand() % 100) / 300.0f;
			p.life = p.maxLife;
			p.type = TRAIL;
			p.hasBounced = false;
			particles.push_back(p);
		}
	}

	void EmitSparkle(vec3 position, vec4 color) {
		int n = 3 + rand() % 3;
		for (int i = 0; i < n; i++) {
			Particle p;
			p.position = position;
			p.velocity = vec3(
				(rand() % 100 - 50) / 40.0f,
				(rand() % 100 - 50) / 40.0f,
				(rand() % 100 - 50) / 40.0f
			);
			p.color = color;
			p.size = 1.5f + (rand() % 100) / 60.0f;
			p.maxLife = 0.35f + (rand() % 100) / 400.0f;
			p.life = p.maxLife;
			p.type = TRAIL;
			p.hasBounced = false;
			particles.push_back(p);
		}
	}

	void Update(float deltaTime) {
		for (int i = (int)particles.size() - 1; i >= 0; i--) {
			particles[i].life -= deltaTime;
			if (particles[i].life <= 0.0f) {
				particles.erase(particles.begin() + i);
				continue;
			}

			vec3 g = gravity;
			if (particles[i].type == DEBRIS) g = debrisGravity;
			if (particles[i].type == TRAIL)  g = vec3(0.0f, -1.5f, 0.0f);
			if (particles[i].type == MUZZLE) g = vec3(0.0f, -0.5f, 0.0f);

			particles[i].velocity += g * deltaTime;
			particles[i].position += particles[i].velocity * deltaTime;

			// debris floor bounce with ground sparks
			if (particles[i].type == DEBRIS && !particles[i].hasBounced && particles[i].position.y < 0.5f) {
				particles[i].position.y = 0.5f;
				particles[i].velocity.y = abs(particles[i].velocity.y) * 0.35f;
				particles[i].velocity.x *= 0.5f;
				particles[i].velocity.z *= 0.5f;
				particles[i].hasBounced = true;
				// occasional ground spark
				if (rand() % 3 == 0) {
					Particle spark;
					spark.position = particles[i].position;
					spark.velocity = vec3(
						(rand() % 100 - 50) / 30.0f,
						2.0f + (rand() % 100) / 50.0f,
						(rand() % 100 - 50) / 30.0f
					);
					spark.color = vec4(1.0f, 0.8f, 0.3f, 1.0f);
					spark.size = 2.0f + (rand() % 100) / 50.0f;
					spark.maxLife = 0.25f + (rand() % 100) / 400.0f;
					spark.life = spark.maxLife;
					spark.type = DEBRIS;
					spark.hasBounced = true;
					particles.push_back(spark);
				}
			}
		}
	}

	void Render(mat4 projection, mat4 view) {
		if (particles.empty()) return;

		vector<vec3> posData;
		vector<vec4> colorData;
		vector<float> sizeData;
		for (auto& p : particles) {
			posData.push_back(p.position);
			float alpha = p.life / p.maxLife > 0.0f ? p.life / p.maxLife : 0.0f;
			colorData.push_back(vec4(p.color.r, p.color.g, p.color.b, p.color.a * alpha));
			sizeData.push_back(p.size * alpha);
		}

		shader->Bind();
		shader->SetMat4("projection", projection);
		shader->SetMat4("view", view);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO_POS);
		glBufferData(GL_ARRAY_BUFFER, posData.size() * sizeof(vec3), &posData[0], GL_STREAM_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, VBO_COLOR);
		glBufferData(GL_ARRAY_BUFFER, colorData.size() * sizeof(vec4), &colorData[0], GL_STREAM_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, VBO_SIZE);
		glBufferData(GL_ARRAY_BUFFER, sizeData.size() * sizeof(float), &sizeData[0], GL_STREAM_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glDrawArrays(GL_POINTS, 0, (GLsizei)posData.size());

		glBindVertexArray(0);
		shader->Unbind();
	}

private:
	// === WATER SPLASH: 4-layer blue/white burst ===
	void ExplodeWater(vec3 position, vec4 tintColor, int count) {
		int plumeCount = std::max<int>(1, count * 40 / 210);
		int splashCount = std::max<int>(1, count * 100 / 210);
		int ringCount = std::max<int>(1, count * 40 / 210);
		int mistCount = std::max<int>(1, count * 30 / 210);

		// CENTER PLUME -- shoots straight up
		for (int i = 0; i < plumeCount; i++) {
			Particle p;
			p.position = position;
			float spreadAngle = (rand() % 360) * 3.14159f / 180.0f;
			float spreadRadius = (rand() % 100) / 300.0f;
			float upSpeed = 6.0f + (rand() % 100) / 8.0f;
			p.velocity.x = cos(spreadAngle) * spreadRadius * upSpeed;
			p.velocity.y = upSpeed;
			p.velocity.z = sin(spreadAngle) * spreadRadius * upSpeed;
			p.color = vec4(0.7f + (rand() % 30) / 100.0f, 0.85f + (rand() % 15) / 100.0f, 1.0f, 1.0f);
			p.size = (6.0f + (rand() % 100) / 10.0f) * explosionSizeScale;
			p.maxLife = 1.0f + (rand() % 100) / 80.0f;
			p.life = p.maxLife;
			p.type = WATER;
			p.hasBounced = false;
			particles.push_back(p);
		}

		// MAIN SPLASH -- arcs outward in all directions
		for (int i = 0; i < splashCount; i++) {
			Particle p;
			p.position = position;
			float angle = (rand() % 360) * 3.14159f / 180.0f;
			float elevation = (rand() % 100 - 10) * 3.14159f / 180.0f;
			float speed = 5.0f + (rand() % 100) / 6.0f;
			p.velocity.x = cos(elevation) * cos(angle) * speed;
			p.velocity.y = sin(elevation) * speed;
			p.velocity.z = cos(elevation) * sin(angle) * speed;
			p.color = vec4(0.6f + (rand() % 40) / 100.0f, 0.8f + (rand() % 20) / 100.0f, 0.95f + (rand() % 5) / 100.0f, 1.0f);
			p.size = (5.0f + (rand() % 100) / 10.0f) * explosionSizeScale;
			p.maxLife = 0.8f + (rand() % 100) / 70.0f;
			p.life = p.maxLife;
			p.type = WATER;
			p.hasBounced = false;
			particles.push_back(p);
		}

		// FOAM RING -- white ring at base
		for (int j = 0; j < ringCount; j++) {
			Particle r;
			r.position = position;
			float ringAngle = (j / (float)ringCount) * 6.28318f;
			float ringSpeed = 7.0f + (rand() % 100) / 10.0f;
			r.velocity.x = cos(ringAngle) * ringSpeed;
			r.velocity.y = 2.0f + (rand() % 100) / 30.0f;
			r.velocity.z = sin(ringAngle) * ringSpeed;
			r.color = vec4(0.9f, 0.95f, 1.0f, 1.0f);
			r.size = (8.0f + (rand() % 100) / 8.0f) * explosionSizeScale;
			r.maxLife = 0.5f + (rand() % 100) / 100.0f;
			r.life = r.maxLife;
			r.type = WATER;
			r.hasBounced = false;
			particles.push_back(r);
		}

		// MIST -- fine white spray
		for (int k = 0; k < mistCount; k++) {
			Particle m;
			m.position = position;
			float ma = (rand() % 360) * 3.14159f / 180.0f;
			float me = (rand() % 120 - 20) * 3.14159f / 180.0f;
			float ms = 3.0f + (rand() % 100) / 30.0f;
			m.velocity.x = cos(me) * cos(ma) * ms;
			m.velocity.y = sin(me) * ms;
			m.velocity.z = cos(me) * sin(ma) * ms;
			m.color = vec4(1.0f, 1.0f, 1.0f, 0.6f);
			m.size = (4.0f + (rand() % 100) / 20.0f) * explosionSizeScale;
			m.maxLife = 0.3f + (rand() % 100) / 200.0f;
			m.life = m.maxLife;
			m.type = WATER;
			m.hasBounced = false;
			particles.push_back(m);
		}
	}

	// === DEBRIS BURST: 3-layer hard fragments + dust + sparks ===
	void ExplodeDebris(vec3 position, vec4 tintColor, int count) {
		int fragCount = std::max<int>(1, count * 60 / 110);
		int dustCount = std::max<int>(1, count * 30 / 110);
		int sparkCount = std::max<int>(1, count * 20 / 110);

		// MAIN FRAGMENTS -- hard, fast, heavy
		for (int i = 0; i < fragCount; i++) {
			Particle p;
			p.position = position;
			float angle = (rand() % 360) * 3.14159f / 180.0f;
			float elevation = (rand() % 160 - 30) * 3.14159f / 180.0f;
			float speed = 8.0f + (rand() % 100) / 4.0f;
			p.velocity.x = cos(elevation) * cos(angle) * speed;
			p.velocity.y = sin(elevation) * speed;
			p.velocity.z = cos(elevation) * sin(angle) * speed;
			p.color = vec4(
				0.7f + (rand() % 30) / 100.0f,
				0.3f + (rand() % 30) / 100.0f,
				0.1f + (rand() % 20) / 100.0f, 1.0f
			);
			p.size = 3.0f + (rand() % 100) / 15.0f;
			p.maxLife = 0.8f + (rand() % 100) / 60.0f;
			p.life = p.maxLife;
			p.type = DEBRIS;
			p.hasBounced = false;
			particles.push_back(p);
		}

		// DUST CLOUD -- slow, billowing
		for (int i = 0; i < dustCount; i++) {
			Particle d;
			d.position = position;
			float da = (rand() % 360) * 3.14159f / 180.0f;
			float de = (rand() % 90) * 3.14159f / 180.0f;
			float ds = 2.0f + (rand() % 100) / 30.0f;
			d.velocity.x = cos(de) * cos(da) * ds;
			d.velocity.y = sin(de) * ds;
			d.velocity.z = cos(de) * sin(da) * ds;
			d.color = vec4(
				0.6f + (rand() % 20) / 100.0f,
				0.5f + (rand() % 20) / 100.0f,
				0.4f + (rand() % 20) / 100.0f, 0.7f
			);
			d.size = 5.0f + (rand() % 100) / 10.0f;
			d.maxLife = 0.6f + (rand() % 100) / 80.0f;
			d.life = d.maxLife;
			d.type = DEBRIS;
			d.hasBounced = false;
			particles.push_back(d);
		}

		// SPARKS -- bright, tiny, very fast
		for (int i = 0; i < sparkCount; i++) {
			Particle s;
			s.position = position;
			float sa = (rand() % 360) * 3.14159f / 180.0f;
			float se = (rand() % 180 - 20) * 3.14159f / 180.0f;
			float ss = 10.0f + (rand() % 100) / 3.0f;
			s.velocity.x = cos(se) * cos(sa) * ss;
			s.velocity.y = sin(se) * ss;
			s.velocity.z = cos(se) * sin(sa) * ss;
			s.color = vec4(1.0f, 0.85f + (rand() % 15) / 100.0f, 0.2f + (rand() % 30) / 100.0f, 1.0f);
			s.size = 1.5f + (rand() % 100) / 30.0f;
			s.maxLife = 0.3f + (rand() % 100) / 200.0f;
			s.life = s.maxLife;
			s.type = DEBRIS;
			s.hasBounced = false;
			particles.push_back(s);
		}
	}

	// === MUZZLE FLASH: single-layer instant bright burst ===
	void ExplodeMuzzle(vec3 position, int count) {
		int flashCount = std::max<int>(1, count);
		for (int i = 0; i < flashCount; i++) {
			Particle p;
			p.position = position;
			float angle = (rand() % 360) * 3.14159f / 180.0f;
			float spread = (rand() % 60) * 3.14159f / 180.0f;
			float speed = 3.0f + (rand() % 100) / 20.0f;
			p.velocity.x = cos(angle) * sin(spread) * speed;
			p.velocity.y = sin(angle) * sin(spread) * speed;
			p.velocity.z = cos(spread) * speed;
			p.color = vec4(1.0f, 0.9f + (rand() % 10) / 100.0f, 0.4f + (rand() % 40) / 100.0f, 1.0f);
			p.size = 2.0f + (rand() % 100) / 25.0f;
			p.maxLife = 0.08f + (rand() % 100) / 1000.0f;
			p.life = p.maxLife;
			p.type = MUZZLE;
			p.hasBounced = false;
			particles.push_back(p);
		}
	}
};

#endif
