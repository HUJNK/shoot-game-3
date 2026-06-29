#ifndef ITEM_H
#define ITEM_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
#include <vector>
#include <ctime>
#include <cstdlib>
using namespace std;
#include "shader.h"
#include "particle.h"

enum ItemType {
	ITEM_RAPID_FIRE,   // orange cube - 2s auto-fire
	ITEM_SLOW_MOTION,  // blue cube - 3s slow balls
	ITEM_BONUS_SCORE,  // gold cube - +10 score instant
	ITEM_EXTRA_LIFE    // red cube - +1 life (max 3)
};

struct Item {
	vec3 position;
	ItemType type;
	float life;
	float maxLife;
	float bobPhase;
};

class ItemManager {
private:
	vector<Item> items;
	Shader* shader;
	GLuint cubeVAO, cubeVBO;
	float spawnTimer;
	float spawnInterval;
	int maxItems;
	float sparkTimer;  // for sparkle emission

	// unit cube (same geometry as obstacle)
	float cubeVerts[288] = {
		-0.5f,-0.5f, 0.5f,  0,0,1,   0.5f,-0.5f, 0.5f,  0,0,1,   0.5f, 0.5f, 0.5f,  0,0,1,
		-0.5f,-0.5f, 0.5f,  0,0,1,   0.5f, 0.5f, 0.5f,  0,0,1,  -0.5f, 0.5f, 0.5f,  0,0,1,
		 0.5f,-0.5f,-0.5f,  0,0,-1,  -0.5f,-0.5f,-0.5f,  0,0,-1,  -0.5f, 0.5f,-0.5f,  0,0,-1,
		 0.5f,-0.5f,-0.5f,  0,0,-1,  -0.5f, 0.5f,-0.5f,  0,0,-1,   0.5f, 0.5f,-0.5f,  0,0,-1,
		-0.5f, 0.5f,-0.5f,  0,1,0,  -0.5f, 0.5f, 0.5f,  0,1,0,   0.5f, 0.5f, 0.5f,  0,1,0,
		-0.5f, 0.5f,-0.5f,  0,1,0,   0.5f, 0.5f, 0.5f,  0,1,0,   0.5f, 0.5f,-0.5f,  0,1,0,
		-0.5f,-0.5f,-0.5f,  0,-1,0,   0.5f,-0.5f, 0.5f,  0,-1,0,  -0.5f,-0.5f, 0.5f,  0,-1,0,
		-0.5f,-0.5f,-0.5f,  0,-1,0,   0.5f,-0.5f,-0.5f,  0,-1,0,   0.5f,-0.5f, 0.5f,  0,-1,0,
		 0.5f,-0.5f, 0.5f,  1,0,0,   0.5f,-0.5f,-0.5f,  1,0,0,   0.5f, 0.5f,-0.5f,  1,0,0,
		 0.5f,-0.5f, 0.5f,  1,0,0,   0.5f, 0.5f,-0.5f,  1,0,0,   0.5f, 0.5f, 0.5f,  1,0,0,
		-0.5f,-0.5f,-0.5f, -1,0,0,  -0.5f,-0.5f, 0.5f, -1,0,0,  -0.5f, 0.5f, 0.5f, -1,0,0,
		-0.5f,-0.5f,-0.5f, -1,0,0,  -0.5f, 0.5f, 0.5f, -1,0,0,  -0.5f, 0.5f,-0.5f, -1,0,0,
	};

public:
	ItemManager() {
		shader = new Shader("res/shader/obstacle.vert", "res/shader/obstacle.frag");

		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		glBindVertexArray(cubeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glBindVertexArray(0);

		spawnTimer = 15.0f + (rand() % 100) / 10.0f;
		spawnInterval = 20.0f;
		maxItems = 1;
		sparkTimer = 0.0f;
	}

	void Update(float deltaTime, ParticleSystem* particles) {
		spawnTimer -= deltaTime;
		if (spawnTimer <= 0.0f && (int)items.size() < maxItems) {
			SpawnRandom();
			spawnTimer = spawnInterval + (rand() % 100) / 10.0f;
		}

		sparkTimer += deltaTime;

		for (int i = (int)items.size() - 1; i >= 0; i--) {
			items[i].life -= deltaTime;
			items[i].bobPhase += deltaTime * 3.0f;
			if (items[i].life <= 0.0f) {
				items.erase(items.begin() + i);
				continue;
			}
			// float forward
			items[i].position.z += 0.06f;
			// bob up and down
			items[i].position.y += sin(items[i].bobPhase) * 0.025f;

			// sparkle around item every ~0.2s
			if (particles && sparkTimer > 0.15f) {
				vec3 sparkPos = items[i].position + vec3(
					(rand() % 100 - 50) / 25.0f,
					(rand() % 100 - 50) / 25.0f,
					(rand() % 100 - 50) / 25.0f
				);
				vec3 itemColor = GetItemColor(items[i].type);
				particles->EmitSparkle(sparkPos, vec4(itemColor, 1.0f));
			}

			// despawn if past player
			if (items[i].position.z > 65.0f) {
				items.erase(items.begin() + i);
			}
		}
		if (sparkTimer > 0.15f) sparkTimer = 0.0f;
	}

	void SpawnRandom() {
		Item item;
		item.position = vec3(
			(rand() % 50 - 25),              // x: -25 to 25
			6.0f + (rand() % 120) / 10.0f,   // y: 6 to 18
			-25.0f                            // z: ahead
		);
		int r = rand() % 100;
		if (r < 30)       item.type = ITEM_RAPID_FIRE;
		else if (r < 55)  item.type = ITEM_SLOW_MOTION;
		else if (r < 80)  item.type = ITEM_BONUS_SCORE;
		else              item.type = ITEM_EXTRA_LIFE;

		item.maxLife = 12.0f;
		item.life = item.maxLife;
		item.bobPhase = 0.0f;
		items.push_back(item);
	}

	// returns ItemType or -1 if miss
	int CheckHit(vec3 rayOrigin, vec3 rayDir) {
		for (int i = 0; i < (int)items.size(); i++) {
			vec3 oc = rayOrigin - items[i].position;
			float radius = 2.5f;
			float b = dot(oc, rayDir);
			float c = dot(oc, oc) - radius * radius;
			float disc = b * b - c;
			if (disc > 0.0f) {
				float t = -b - sqrt(disc);
				if (t > 0.0f) {
					int type = items[i].type;
					items.erase(items.begin() + i);
					return type;
				}
			}
		}
		return -1;
	}

	bool IsEmpty() { return items.empty(); }

	// Depth-only render for shadow map (items cast shadows)
	void RenderDepth(Shader* depthShader, mat4 lightSpaceMatrix) {
		if (items.empty()) return;
		depthShader->Bind();
		depthShader->SetMat4("lightSpaceMatrix", lightSpaceMatrix);
		glBindVertexArray(cubeVAO);
		for (auto& item : items) {
			mat4 model = mat4(1.0);
			model[3] = vec4(item.position, 1.0);
			float s = 2.0f + sin(item.bobPhase) * 0.3f;
			model = scale(model, vec3(s));
			depthShader->SetMat4("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		glBindVertexArray(0);
		depthShader->Unbind();
	}

	void Render(mat4 projection, mat4 view, float time) {
		for (auto& item : items) {
			mat4 model = mat4(1.0);
			model[3] = vec4(item.position, 1.0);
			model = rotate(model, time * 2.0f, vec3(0.0f, 1.0f, 0.0f));
			model = rotate(model, time * 1.5f, vec3(1.0f, 0.0f, 0.0f));
			float s = 2.0f + sin(item.bobPhase) * 0.3f;
			model = scale(model, vec3(s));

			float pulse = 0.7f + sin(time * 4.0f) * 0.3f;
			vec3 col = GetItemColor(item.type) * pulse;

			shader->Bind();
			shader->SetMat4("projection", projection);
			shader->SetMat4("view", view);
			shader->SetMat4("model", model);
			shader->SetVec3("objColor", col);

			glBindVertexArray(cubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
			shader->Unbind();
		}
	}

private:
	vec3 GetItemColor(ItemType type) {
		switch (type) {
		case ITEM_RAPID_FIRE:  return vec3(1.0f, 0.45f, 0.05f);  // orange
		case ITEM_SLOW_MOTION: return vec3(0.25f, 0.65f, 1.0f);  // blue
		case ITEM_BONUS_SCORE: return vec3(1.0f, 0.85f, 0.1f);   // gold
		case ITEM_EXTRA_LIFE:  return vec3(1.0f, 0.15f, 0.15f);  // red
		default:               return vec3(1.0f, 0.85f, 0.1f);
		}
	}
};

#endif
