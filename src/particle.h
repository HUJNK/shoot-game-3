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
		explosionSizeScale = 1.5f;
	}

	void SetExplosionSizeScale(float s) { explosionSizeScale = s; }

	// Balloon pop firework: rays + stars + dots + fountain
	// Store firework burst data for beam rendering
	struct FireworkBurst {
		vec3 position;
		float time;
		float maxTime;
		int beamCount;
		int bottomTrails;
		vector<vec3> beamDirs;
		vector<float> beamLens;
		vec4 color;
	};
	vector<FireworkBurst> fireworkBursts;

	void AddFireworkBurst(vec3 pos, int score) {
		FireworkBurst fb;
		fb.position = pos;
		fb.time = 0;
		fb.maxTime = 1.5f;
		fb.beamCount = 6 + rand() % 7;  // 6-12 beams
		// Pure vibrant colors per score
		fb.color = score >= 25 ? vec4(0.1f, 0.85f, 0.15f, 1.0f) :   // bright green
		           score >= 5  ? vec4(1.0f, 0.1f, 0.05f, 1.0f) :    // pure red
		           score >= 3  ? vec4(0.1f, 0.5f, 1.0f, 1.0f) :     // bright blue
		                         vec4(1.0f, 0.85f, 0.05f, 1.0f);    // pure gold
		// Full 360° radiating beams, random lengths 40-80 units
		for (int i = 0; i < fb.beamCount; i++) {
			float theta = ((float)i/fb.beamCount + (rand()%15-7)/100.0f) * 6.28318f;
			float phi = (rand()%80 + 50) * 3.14159f/180.0f;
			if (rand()%100 < 15) phi = (rand()%40 + 140) * 3.14159f/180.0f;
			vec3 dir(sin(phi)*cos(theta), cos(phi), sin(phi)*sin(theta));
			fb.beamDirs.push_back(normalize(dir));
			fb.beamLens.push_back(40.0f + (rand()%100)*0.4f);
		}
		// 2-3 bottom crossover trails
		fb.bottomTrails = 2 + rand() % 2;
		for (int i = 0; i < fb.bottomTrails; i++) {
			float theta = (rand()%360)*3.14159f/180.0f;
			float phi = (rand()%30+100)*3.14159f/180.0f;
			vec3 d(sin(phi)*cos(theta), cos(phi), sin(phi)*sin(theta));
			fb.beamDirs.push_back(normalize(d));
			fb.beamLens.push_back(25.0f + (rand()%100)*0.3f);
		}
		fireworkBursts.push_back(fb);
	}

	void ExplodeBalloon(vec3 position) {
		// Phase 1: Center white flash (large bright particles, short life)
		for (int i = 0; i < 30; i++) {
			Particle f;
			f.position = position + vec3((rand()%20-10)/10.0f, (rand()%20-10)/10.0f, (rand()%20-10)/10.0f);
			f.velocity = vec3(0);
			f.color = vec4(1.0f, 1.0f, 0.95f, 1.0f);
			f.size = 8.0f + (rand()%100)/10.0f;
			f.maxLife = 0.15f + (rand()%100)/1000.0f;
			f.life = f.maxLife; f.type = MUZZLE; f.hasBounced = false;
			particles.push_back(f);
		}
		// Phase 2: Radiating ray petals (elongated droplets, red/green/yellow)
		vec4 petalColors[] = {
			vec4(0.95f,0.15f,0.1f,1.0f), vec4(0.15f,0.8f,0.2f,1.0f),
			vec4(1.0f,0.9f,0.15f,1.0f), vec4(0.05f,0.55f,0.15f,1.0f)
		};
		for (int p = 0; p < 60; p++) {
			float theta = (rand()%360) * 3.14159f/180.0f;
			float phi = acos(1.0f - 2.0f*(rand()%10000)/10000.0f);
			vec3 dir(sin(phi)*cos(theta), cos(phi), sin(phi)*sin(theta));
			// Bias upward for fountain feel
			if (dir.y < -0.2f) dir.y = -0.2f + (rand()%20)/100.0f;
			dir = normalize(dir);
			float len = 12.0f + (rand()%100)/3.0f; // 12~45
			int segs = 15 + rand()%20;
			vec3 perp = normalize(cross(dir, vec3(0,1,0)));
			if (length(perp)<0.1f) perp=vec3(1,0,0);
			vec3 perp2 = normalize(cross(dir, perp));
			vec4 col = petalColors[rand()%4];
			float speed = 18.0f + (rand()%100)/3.0f;

			for (int s = 0; s < segs; s++) {
				float t = (float)s/segs;
				float curve = sin(t*3.14159f)*4.0f;
				vec3 off = (perp*cos(t*5)+perp2*sin(t*4))*curve*t;
				Particle pt;
				pt.position = position + (dir*t + off*0.25f)*len;
				pt.velocity = dir*speed*(1.0f-t*0.6f) + off*0.3f;
				float bright = 1.0f - t;
				pt.color = vec4(mix(1.0f,col.r,bright>0.5f?1.0f:bright*2), mix(1.0f,col.g,bright>0.5f?1.0f:bright*2), mix(0.8f,col.b,bright>0.5f?1.0f:bright*2), bright*0.9f);
				pt.size = t<0.1f ? 1.0f+t*20.0f : 4.0f-t*3.5f;
				if (t>0.8f) pt.size = 2.0f-(t-0.8f)*8.0f;
				pt.maxLife = 0.8f + t*1.2f + (rand()%100)/100.0f;
				pt.life = pt.maxLife; pt.type = WATER; pt.hasBounced = false;
				particles.push_back(pt);
			}
		}
		// Phase 3: Scattered stars (bright small dots)
		for (int i = 0; i < 80; i++) {
			float theta = (rand()%360)*3.14159f/180.0f;
			float phi = acos(1.0f-2.0f*(rand()%10000)/10000.0f);
			vec3 dir(sin(phi)*cos(theta), cos(phi), sin(phi)*sin(theta));
			float dist = 8.0f + (rand()%100)/2.0f;
			Particle s;
			s.position = position + dir*dist;
			s.velocity = dir*(10.0f+(rand()%100)/5.0f);
			float r = rand()%2 ? 0.95f : 0.05f;
			float g = rand()%2 ? 0.8f : 0.55f;
			s.color = vec4(r, g, 0.1f, 1.0f);
			s.size = 2.0f + (rand()%100)/25.0f;
			s.maxLife = 0.5f + (rand()%100)/80.0f;
			s.life = s.maxLife; s.type = MUZZLE; s.hasBounced = false;
			particles.push_back(s);
		}
		// Phase 4: Circular dots (yellow/green)
		for (int i = 0; i < 100; i++) {
			float theta = (rand()%360)*3.14159f/180.0f;
			float phi = acos(1.0f-2.0f*(rand()%10000)/10000.0f);
			vec3 dir(sin(phi)*cos(theta), cos(phi), sin(phi)*sin(theta));
			float dist = 5.0f + (rand()%100)/3.0f;
			Particle d;
			d.position = position + dir*dist;
			d.velocity = dir*(8.0f+(rand()%100)/6.0f);
			int colType = rand()%2;
			d.color = colType ? vec4(1.0f,0.9f,0.1f,1.0f) : vec4(0.05f,0.55f,0.15f,1.0f);
			d.size = 1.5f + (rand()%100)/20.0f;
			d.maxLife = 0.6f + (rand()%100)/70.0f;
			d.life = d.maxLife; d.type = TRAIL; d.hasBounced = false;
			particles.push_back(d);
		}
		// Phase 5: Bottom fountain trails (curved upward streaks)
		for (int i = 0; i < 40; i++) {
			float theta = (rand()%360)*3.14159f/180.0f;
			float phi = (rand()%60+60)*3.14159f/180.0f; // mostly upward
			vec3 dir(sin(phi)*cos(theta), cos(phi), sin(phi)*sin(theta));
			float len = 8.0f + (rand()%100)/5.0f;
			int segs = 8 + rand()%12;
			vec4 fc = (rand()%2) ? vec4(0.95f,0.15f,0.1f,1.0f) : vec4(0.15f,0.8f,0.2f,1.0f);
			for (int s=0; s<segs; s++) {
				float t = (float)s/segs;
				Particle ft;
				ft.position = position + dir*t*len;
				ft.velocity = dir*(15.0f-(s>segs/2?10.0f:0));
				ft.velocity.y += 3.0f;
				ft.color = vec4(fc.r, fc.g, fc.b, (1.0f-t)*0.9f);
				ft.size = 2.0f + t*3.0f;
				if (t>0.7f) ft.size = 5.0f-(t-0.7f)*15.0f;
				ft.maxLife = 0.4f + t*0.8f + (rand()%100)/200.0f;
				ft.life = ft.maxLife; ft.type = WATER; ft.hasBounced = false;
				particles.push_back(ft);
			}
		}
	}

	// Chrysanthemum firework: curved petal arcs in a hemisphere
	void ExplodeFirework(vec3 position, vec4 tintColor, int petalCount = 50) {
		for (int p = 0; p < petalCount; p++) {
			// Hemisphere: focus upward (y>0), some downward
			float theta = (rand() % 360) * 3.14159f / 180.0f;
			float phi = acos(1.0f - 2.0f * (rand() % 10000) / 10000.0f);
			// Bias toward upper hemisphere
			if (rand() % 100 < 25) phi = 3.14159f - phi; // flip to lower
			float yDir = cos(phi);
			if (yDir < -0.3f) phi = acos(-0.3f); // limit downward spread

			vec3 dir(sin(phi)*cos(theta), cos(phi), sin(phi)*sin(theta));
			float petalLen = 15.0f + (rand() % 100) / 5.0f; // 15~35 units
			float arcStrength = 3.0f + (rand() % 100) / 20.0f; // curve amount
			int segments = 20 + rand() % 15; // dots per petal

			vec3 perpX = normalize(cross(dir, vec3(0,1,0)));
			if (length(perpX) < 0.1f) perpX = vec3(1,0,0);
			vec3 perpY = normalize(cross(dir, perpX));

			// Petal: sequence of particles along a curved arc
			float baseSpeed = 20.0f + (rand() % 100) / 3.0f;
			for (int s = 0; s < segments; s++) {
				float t = (float)s / segments; // 0 (center) → 1 (tip)
				float curve = sin(t * 3.14159f) * arcStrength;
				vec3 curveOffset = (perpX * cos(t*5.0f) + perpY * sin(t*4.0f)) * curve * t;
				vec3 ptPos = position + (dir * t + curveOffset * 0.3f) * petalLen;

				Particle pt;
				pt.position = ptPos;
				pt.velocity = (dir + curveOffset * 0.1f) * baseSpeed * (1.0f - t * 0.7f);
				// Gradient: white base → saturated mid → fading tip
				float brightness = 1.0f - t;
				float sat = t < 0.5f ? t * 2.0f : 2.0f - t * 2.0f;
				pt.color = vec4(
					mix(1.0f, tintColor.r, sat) * brightness,
					mix(1.0f, tintColor.g, sat) * brightness,
					mix(0.8f, tintColor.b, sat) * brightness,
					brightness * 0.9f
				);
				pt.size = (t < 0.1f ? 0.5f + t*10.0f : 3.5f - t*3.0f) + (rand()%100)/50.0f;
				pt.maxLife = 1.5f + t * 2.0f + (rand()%100)/80.0f;
				pt.life = pt.maxLife;
				pt.type = (t < 0.3f) ? MUZZLE : WATER;
				pt.hasBounced = false;
				particles.push_back(pt);
			}
		}
	}

	void Explode(vec3 position, vec4 tintColor, int count = 80, ParticleType type = WATER) {
		switch (type) {
		case WATER:  ExplodeWater(position, tintColor, count); break;
		case DEBRIS: ExplodeDebris(position, tintColor, count); break;
		case MUZZLE: ExplodeMuzzle(position, tintColor, count); break;
		default: break;
		}
	}

	// Rain droplet: fast-falling streak
	void EmitRaindrop() {
		Particle p;
		p.position = vec3(
			(rand() % 200) - 100.0f,
			55.0f + (rand() % 15),
			(rand() % 140) - 85.0f
		);
		p.velocity = vec3(
			(rand() % 20 - 10) / 8.0f,
			-30.0f - (rand() % 100) / 3.0f,   // -30 ~ -63
			(rand() % 20 - 10) / 8.0f
		);
		p.color = vec4(0.6f, 0.75f, 0.95f, 0.6f);
		p.size = 3.0f + (rand() % 100) / 20.0f;  // 3 ~ 8, bigger
		p.maxLife = 1.8f + (rand() % 100) / 60.0f;
		p.life = p.maxLife;
		p.type = TRAIL;  // use TRAIL type for less gravity interference
		p.hasBounced = false;
		particles.push_back(p);
	}

	// Ambient dust mote: tiny, slow, semi-transparent
	void EmitDust(vec3 position) {
		Particle p;
		p.position = position;
		p.velocity = vec3(
			(rand() % 100 - 50) / 30.0f,
			(rand() % 100 - 50) / 40.0f,
			(rand() % 100 - 50) / 30.0f
		);
		float b = 0.4f + (rand() % 40) / 100.0f;
		p.color = vec4(b, b, b, 0.35f);
		p.size = 1.5f + (rand() % 100) / 30.0f;
		p.maxLife = 3.0f + (rand() % 100) / 20.0f;
		p.life = p.maxLife;
		p.type = TRAIL;
		p.hasBounced = false;
		particles.push_back(p);
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
		// Update firework bursts
		for (int i = (int)fireworkBursts.size()-1; i >= 0; i--) {
			fireworkBursts[i].time += deltaTime;
			if (fireworkBursts[i].time > fireworkBursts[i].maxTime)
				fireworkBursts.erase(fireworkBursts.begin()+i);
		}
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

			// Firework physics: air friction + gravity drift
			if (particles[i].type == WATER || particles[i].type == MUZZLE) {
				float spd = length(particles[i].velocity);
				if (spd > 2.0f) {
					// Strong air resistance during burst phase
					vec3 drag = normalize(particles[i].velocity) * spd * 3.0f;
					particles[i].velocity -= drag * deltaTime;
				}
				particles[i].velocity += g * deltaTime;
				// Zigzag wobble for some sparks as they slow down
				if (spd < 10.0f && (i % 5 == 0)) {
					float wobble = sin(particles[i].life * 15.0f + i) * 3.0f;
					particles[i].velocity.x += wobble * deltaTime;
					particles[i].velocity.z += cos(particles[i].life * 12.0f + i) * 2.0f * deltaTime;
				}
			} else {
				particles[i].velocity += g * deltaTime;
			}
			particles[i].position += particles[i].velocity * deltaTime;

			// Willow trails: spawn trail particle every frame behind moving firework
			if ((particles[i].type == WATER || particles[i].type == MUZZLE) &&
				particles[i].life > 0.1f) {
				Particle trail;
				trail.position = particles[i].position;
				trail.velocity = vec3(0);
				float fade = particles[i].life / particles[i].maxLife;
				trail.color = particles[i].color;
				trail.color.r = mix(0.6f, trail.color.r, fade);
				trail.color.a *= 0.6f;
				trail.size = particles[i].size * 0.5f;
				trail.maxLife = 0.5f + (rand() % 100) / 200.0f;
				trail.life = trail.maxLife;
				trail.type = TRAIL;
				trail.hasBounced = false;
				particles.push_back(trail);
			}

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

	void RenderFireworks(mat4 projection, mat4 view) {
		if (fireworkBursts.empty()) return;
		vector<float> verts;
		vector<float> colors;
		for (auto& fb : fireworkBursts) {
			float t = fb.time / fb.maxTime;
			// Beam expand phase: 0→0.3s linear stretch, then hold, then fade
			float expand = t < 0.3f ? t / 0.3f : 1.0f;
			float fade = t > 0.7f ? (1.0f - (t-0.7f)/0.8f) : 1.0f;
			if (fade < 0) fade = 0;
			float alpha = fade;

			// 1. Central flash disc (0-0.1s expanding, then gone)
			if (t < 0.12f) {
				float ft = t < 0.04f ? t/0.04f : 1.0f - (t-0.04f)/0.08f;
				if (ft > 0) {
					vec3 c = fb.position;
					float r = 5.0f * (0.3f + ft*0.7f);
					for (int a = 0; a < 16; a++) {
						float a1=(float)a/16*6.28318f, a2=(float)(a+1)/16*6.28318f;
						verts.insert(verts.end(),{c.x,c.y,c.z, c.x+cos(a1)*r,c.y+sin(a1)*r,c.z, c.x+cos(a2)*r,c.y+sin(a2)*r,c.z});
						colors.insert(colors.end(),{1,1,1,ft, 1,1,0.9f,ft*0.3f, 1,1,0.9f,ft*0.3f});
					}
				}
			}

			// 2. Vector beams: thick-to-thin lines, 0→max length in 0.3s
			int totalBeams = fb.beamCount + fb.bottomTrails;
			for (int b = 0; b < totalBeams; b++) {
				bool isBottom = (b >= fb.beamCount);
				vec3 d = fb.beamDirs[b];
				float maxL = fb.beamLens[b];
				float L = maxL * expand;
				vec3 tip = fb.position + d * L;
				vec3 perp = normalize(cross(d, vec3(0,1,0)));
				if (length(perp) < 0.1f) perp = vec3(1,0,0);
				vec3 perp2 = normalize(cross(d, perp));

				// Bottom trails curve downward
				if (isBottom) {
					float bt = expand;
					vec3 mid = fb.position + d*L*0.5f - vec3(0, L*0.3f*bt, 0);
					tip = fb.position + d*L*0.8f - vec3(0, L*0.5f*bt, 0);
				}

				float baseW = (isBottom ? 0.4f : 0.6f) * alpha;
				float midW = baseW * 0.4f;
				float tipW = baseW * 0.05f;
				vec3 baseL = fb.position + perp*baseW;
				vec3 baseR = fb.position - perp*baseW;
				vec3 midPt = fb.position + d*L*0.5f;
				vec3 midL = midPt + perp*midW;
				vec3 midR = midPt - perp*midW;
				vec3 tipL = tip + perp*tipW;
				vec3 tipR = tip - perp*tipW;

				vec4 col = fb.color;
				float bright = alpha * (isBottom ? 0.6f : 1.0f);

				// Two tri-strips: base→mid and mid→tip for tapered look
				verts.insert(verts.end(),{
					baseL.x,baseL.y,baseL.z, baseR.x,baseR.y,baseR.z, midL.x,midL.y,midL.z,
					baseR.x,baseR.y,baseR.z, midR.x,midR.y,midR.z, midL.x,midL.y,midL.z,
					midL.x,midL.y,midL.z, midR.x,midR.y,midR.z, tipL.x,tipL.y,tipL.z,
					midR.x,midR.y,midR.z, tipR.x,tipR.y,tipR.z, tipL.x,tipL.y,tipL.z
				});
				// Colors: center saturated → tip desaturated+fading
				float cb[48] = {
					col.r,col.g,col.b,bright, col.r,col.g,col.b,bright, col.r*0.7f,col.g*0.7f,col.b*0.6f,bright*0.6f,
					col.r,col.g,col.b,bright, col.r*0.7f,col.g*0.7f,col.b*0.6f,bright*0.6f, col.r*0.7f,col.g*0.7f,col.b*0.6f,bright*0.6f,
					col.r*0.7f,col.g*0.7f,col.b*0.6f,bright*0.6f, col.r*0.7f,col.g*0.7f,col.b*0.6f,bright*0.6f, col.r*0.3f,col.g*0.4f,col.b*0.3f,bright*0.15f,
					col.r*0.7f,col.g*0.7f,col.b*0.6f,bright*0.6f, col.r*0.3f,col.g*0.4f,col.b*0.3f,bright*0.15f, col.r*0.3f,col.g*0.4f,col.b*0.3f,bright*0.15f
				};
				colors.insert(colors.end(), cb, cb+48);

				// 3. Tip sparkles (8-12 small dots at beam ends, only when fully expanded)
				if (expand > 0.9f && !isBottom) {
					int sparkCount = 8 + rand()%5;
					for (int s = 0; s < sparkCount; s++) {
						float sx = (rand()%20-10)/15.0f, sy = (rand()%20-10)/15.0f, sz = (rand()%20-10)/15.0f;
						vec3 sp = tip + vec3(sx,sy,sz)*(1.5f+rand()%100/30.0f);
						verts.insert(verts.end(),{sp.x-0.15f,sp.y-0.15f,sp.z, sp.x+0.15f,sp.y-0.15f,sp.z, sp.x,sp.y+0.15f,sp.z});
						float sparkBright = 0.6f + (rand()%40)/100.0f;
						colors.insert(colors.end(),{1,1,1,sparkBright*alpha, 1,1,1,sparkBright*alpha, 1,1,1,sparkBright*alpha});
					}
				}
			}
		}
		if (verts.empty()) return;
		GLuint fwVAO,fwVBO,fwCBO;
		glGenVertexArrays(1,&fwVAO); glGenBuffers(1,&fwVBO); glGenBuffers(1,&fwCBO);
		glBindVertexArray(fwVAO);
		glBindBuffer(GL_ARRAY_BUFFER,fwVBO);
		glBufferData(GL_ARRAY_BUFFER,verts.size()*sizeof(float),verts.data(),GL_STREAM_DRAW);
		glEnableVertexAttribArray(0); glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0);
		glBindBuffer(GL_ARRAY_BUFFER,fwCBO);
		glBufferData(GL_ARRAY_BUFFER,colors.size()*sizeof(float),colors.data(),GL_STREAM_DRAW);
		glEnableVertexAttribArray(1); glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,0,(void*)0);
		Shader* fwShader = new Shader("res/shader/particle.vert","res/shader/particle.frag");
		fwShader->Bind();
		fwShader->SetMat4("projection",projection); fwShader->SetMat4("view",view);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE); glDepthMask(GL_FALSE);
		glDrawArrays(GL_TRIANGLES,0,(GLsizei)verts.size()/3);
		glDepthMask(GL_TRUE); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		fwShader->Unbind(); glBindVertexArray(0);
		glDeleteVertexArrays(1,&fwVAO); glDeleteBuffers(1,&fwVBO); glDeleteBuffers(1,&fwCBO);
		delete fwShader;
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
			float theta = (rand() % 360) * 3.14159f / 180.0f;
			float phi = acos(2.0f * (rand() % 10000) / 10000.0f - 1.0f);
			float spd = 30.0f + (rand() % 100) / 4.0f;
			p.velocity.x = sin(phi) * cos(theta) * spd;
			p.velocity.y = sin(phi) * sin(theta) * spd;
			p.velocity.z = cos(phi) * spd;
			p.color = vec4(tintColor.r * (0.9f + (rand()%10)/100.0f), tintColor.g * (0.7f + (rand()%30)/100.0f), tintColor.b * (0.5f + (rand()%50)/100.0f), 1.0f);
			p.size = (60.0f + (rand() % 100) / 2.0f) * explosionSizeScale;
			p.maxLife = 2.5f + (rand() % 100) / 40.0f;
			p.life = p.maxLife;
			p.type = WATER;
			p.hasBounced = false;
			particles.push_back(p);
		}

		// MAIN SPLASH -- arcs outward in all directions
		for (int i = 0; i < splashCount; i++) {
			Particle p;
			p.position = position;
			float theta = (rand() % 360) * 3.14159f / 180.0f;
			float phi = acos(2.0f * (rand() % 10000) / 10000.0f - 1.0f);
			float speed = 25.0f + (rand() % 100) / 3.0f;
			p.velocity.x = sin(phi) * cos(theta) * speed;
			p.velocity.y = sin(phi) * sin(theta) * speed;
			p.velocity.z = cos(phi) * speed;
			p.color = vec4(tintColor.r * (0.9f + (rand()%10)/100.0f), tintColor.g * (0.6f + (rand()%40)/100.0f), tintColor.b * (0.4f + (rand()%60)/100.0f), 1.0f);
			p.size = (5.0f + (rand() % 100) / 10.0f) * explosionSizeScale;
			p.maxLife = 2.0f + (rand() % 100) / 50.0f;
			p.life = p.maxLife;
			p.type = WATER;
			p.hasBounced = false;
			particles.push_back(p);
		}

		// FOAM RING -- white ring at base
		for (int j = 0; j < ringCount; j++) {
			Particle r;
			r.position = position;
			float theta = (rand() % 360) * 3.14159f / 180.0f;
			float phi = acos(2.0f * (rand() % 10000) / 10000.0f - 1.0f);
			float ringSpeed = 35.0f + (rand() % 100) / 5.0f;
			r.velocity.x = sin(phi) * cos(theta) * ringSpeed;
			r.velocity.y = sin(phi) * sin(theta) * ringSpeed;
			r.velocity.z = cos(phi) * ringSpeed;
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
			float theta = (rand() % 360) * 3.14159f / 180.0f;
			float phi = acos(2.0f * (rand() % 10000) / 10000.0f - 1.0f);
			float speed = 40.0f + (rand() % 100) / 2.0f;
			p.velocity.x = sin(phi) * cos(theta) * speed;
			p.velocity.y = sin(phi) * sin(theta) * speed;
			p.velocity.z = cos(phi) * speed;
			p.color = vec4(tintColor.r * (0.95f + (rand()%5)/100.0f), tintColor.g * (0.5f + (rand()%50)/100.0f), tintColor.b * (0.3f + (rand()%70)/100.0f), 1.0f);
			p.size = 3.0f + (rand() % 100) / 15.0f;
			p.maxLife = 2.2f + (rand() % 100) / 40.0f;
			p.life = p.maxLife;
			p.type = DEBRIS;
			p.hasBounced = false;
			particles.push_back(p);
		}

		// DUST CLOUD -- slow, billowing
		for (int i = 0; i < dustCount; i++) {
			Particle d;
			d.position = position;
			float theta = (rand() % 360) * 3.14159f / 180.0f;
			float phi = acos(2.0f * (rand() % 10000) / 10000.0f - 1.0f);
			float ds = 2.0f + (rand() % 100) / 30.0f;
			d.velocity.x = sin(phi) * cos(theta) * ds;
			d.velocity.y = sin(phi) * sin(theta) * ds;
			d.velocity.z = cos(phi) * ds;
			d.color = vec4(tintColor.r * (0.8f + (rand()%20)/100.0f), tintColor.g * (0.5f + (rand()%50)/100.0f), tintColor.b * (0.3f + (rand()%70)/100.0f), 0.7f);
			d.size = 5.0f + (rand() % 100) / 10.0f;
			d.maxLife = 1.8f + (rand() % 100) / 50.0f;
			d.life = d.maxLife;
			d.type = DEBRIS;
			d.hasBounced = false;
			particles.push_back(d);
		}

		// SPARKS -- bright, tiny, very fast
		for (int i = 0; i < sparkCount; i++) {
			Particle s;
			s.position = position;
			float theta = (rand() % 360) * 3.14159f / 180.0f;
			float phi = acos(2.0f * (rand() % 10000) / 10000.0f - 1.0f);
			float ss = 10.0f + (rand() % 100) / 3.0f;
			s.velocity.x = sin(phi) * cos(theta) * ss;
			s.velocity.y = sin(phi) * sin(theta) * ss;
			s.velocity.z = cos(phi) * ss;
			s.color = vec4(tintColor.r * (1.0f + (rand()%5)/100.0f), tintColor.g * (0.8f + (rand()%20)/100.0f), tintColor.b * (0.5f + (rand()%50)/100.0f), 1.0f);
			s.size = 1.5f + (rand() % 100) / 30.0f;
			s.maxLife = 1.5f + (rand() % 100) / 80.0f;
			s.life = s.maxLife;
			s.type = DEBRIS;
			s.hasBounced = false;
			particles.push_back(s);
		}
	}

	// === MUZZLE FLASH: single-layer instant bright burst ===
	void ExplodeMuzzle(vec3 position, vec4 tintColor, int count) {
		int flashCount = std::max<int>(1, count);
		for (int i = 0; i < flashCount; i++) {
			Particle p;
			p.position = position;
			float angle = (rand() % 360) * 3.14159f / 180.0f;
			float elevation = (rand() % 180 - 90) * 3.14159f / 180.0f;
			float speed = 15.0f + (rand() % 100) / 4.0f;
			p.velocity.x = cos(elevation) * cos(angle) * speed;
			p.velocity.y = sin(elevation) * speed;
			p.velocity.z = cos(elevation) * sin(angle) * speed;
			p.color = vec4(tintColor.r * (1.0f + (rand()%5)/100.0f), tintColor.g * (0.8f + (rand()%20)/100.0f), tintColor.b * (0.3f + (rand()%70)/100.0f), 1.0f);
			p.size = 2.0f + (rand() % 100) / 25.0f;
			p.maxLife = 0.6f + (rand() % 100) / 200.0f;
			p.life = p.maxLife;
			p.type = MUZZLE;
			p.hasBounced = false;
			particles.push_back(p);
		}
	}
};

#endif
