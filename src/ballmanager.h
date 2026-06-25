#ifndef BALLMANAGER_H
#define BALLMANAGER_H

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
using namespace glm;
#include <cstdlib>
#include <ctime>
#include <vector>
using namespace std;
#include "model.h"
#include "shader.h"
#include "camera.h"

class BallManager {
private:
	vec2 windowSize;

	Model* ball;
	Shader* ballShader;
	GLuint number;
	GLuint maxNumber;
	vec3 basicPos;
	vector<vec3> position;
	vector<vec3> ballColors;
	vector<int> ballScores;
	vector<int> hitScores;
	float moveSpeed;
	GLuint score;
	GLuint lives;
	GLuint combo;
	GLuint maxCombo;
	bool waveTriggered;
	bool slowMotion;
	GLuint gameModel;
	// Boss ball
	vec3 bossPosition;
	bool bossActive;
	int bossHP, bossMaxHP;
	float bossPhase;
	int bossWaveCount;
	float bossOriginalX, bossOriginalY;
	GLuint totalHits;
	vec3 lightPos;
	mat4 lightSpaceMatrix;

	Camera* camera;
	vector<vec3> hitPositions;
	mat4 model;
	mat4 projection;
	mat4 view;

	GLuint spikeVAO, spikeVBO, spikeEBO;
	GLuint spikeIndexCount;
	vector<vec3> fixedSpikePositions;

	float WALL_TOP_Y;
	float SPIKE_TRIGGER_MIN_Y;
	float SPIKE_TRIGGER_MAX_Y;
	float BALL_GEN_HALF_WIDTH;
	int SPIKE_ROWS;
	int SPIKE_COLS;

public:
	BallManager(vec2 windowSize, Camera* camera) {
		this->windowSize = windowSize;
		this->camera = camera;
		basicPos = vec3(0.0, 5.0, -30.0);
		number = 0;
		maxNumber = 3;
		moveSpeed = 0.1f;
		score = 0;
		lives = 3;
		combo = 0;
		maxCombo = 0;
		totalHits = 0;
		waveTriggered = false;
		slowMotion = false;
		bossActive = false;
		bossHP = 0;
		bossMaxHP = 0;
		bossPhase = 0.0f;
		bossWaveCount = 0;
		this->lightPos = vec3(0.0, 400.0, 150.0);
		mat4 lightProjection = ortho(-100.0f, 100.0f, -100.0f, 100.0f, 1.0f, 500.0f);
		mat4 lightView = lookAt(lightPos, vec3(0.0f), vec3(0.0, 1.0, 0.0));
		this->lightSpaceMatrix = lightProjection * lightView;

		WALL_TOP_Y = 55.0f;
		SPIKE_TRIGGER_MIN_Y = 45.0f;
		SPIKE_TRIGGER_MAX_Y = 55.0f;
		BALL_GEN_HALF_WIDTH = 30.0f;
		SPIKE_ROWS = 2;
		SPIKE_COLS = 12;

		InitFixedSpikes();
		AddBall();
		LoadModel();
		InitSpikeGeometry();
	}

	void SetGameModel(GLuint num) {
		gameModel = num;
	}

	void Update(vec3 pos, vec3 dir, bool isShoot, float deltaTime) {
		this->view = camera->GetViewMatrix();
		hitPositions.clear();
		hitScores.clear();
		bool hitThisFrame = false;
		this->projection = perspective(radians(camera->GetZoom()), windowSize.x / windowSize.y, 0.1f, 500.0f);

		// === Boss movement (challenge mode) ===
		if (bossActive && gameModel == 2) {
			bossPhase += deltaTime;
			bossPosition.x = bossOriginalX + sin(bossPhase * 2.0f) * 15.0f;
			bossPosition.y = bossOriginalY + cos(bossPhase * 1.5f) * 4.0f;
			float effBossSpeed = slowMotion ? moveSpeed * 0.4f * 0.6f : moveSpeed * 0.6f;
			bossPosition.z += effBossSpeed;
			if (bossPosition.z >= 70) {
				bossActive = false;
				lives--;
				combo = 0;
				cout << "Boss escaped! Life lost." << endl;
			}
		}

		if (isShoot) {
			// === Boss hit check (challenge mode, before regular balls) ===
			bool bossWasHit = false;
			if (bossActive && gameModel == 2) {
				vec3 oc = pos - bossPosition;
				float bVal = dot(oc, dir);
				float cVal = dot(oc, oc) - 25.0f;
				float disc = bVal * bVal - cVal;
				if (disc > 0.0f) {
					float t = -bVal - sqrt(disc);
					if (t > 0.0f && t < 120.0f) {
						bossWasHit = true;
						bossHP--;
						hitPositions.push_back(bossPosition);
						combo++;
						if (combo > 5) combo = 5;
						if (combo > maxCombo) maxCombo = combo;
						int mult = combo;
						if (combo >= 5) mult = 5;
						else if (combo >= 4) mult = 4;
						else if (combo >= 3) mult = 3;
						else if (combo >= 2) mult = 2;
						if (bossHP <= 0) {
							bossActive = false;
							score += 25 * mult;
							hitScores.push_back(25);
							totalHits++;
							cout << "BOSS DEFEATED! +25 x" << mult << endl;
						} else {
							score += 5 * mult;
							hitScores.push_back(5);
							totalHits++;
							cout << "Boss hit! HP: " << bossHP << "/" << bossMaxHP << endl;
						}
						hitThisFrame = true;
					}
				}
			}
			// === Regular ball hit check ===
			if (!bossWasHit) {
			vector<vec3> temp;
			for (GLuint i = 0; i < position.size(); i++) {
				vec3 des = (pos.z - position[i].z) / (-dir.z) * dir + pos;
				if (pow(position[i].x - des.x, 2) + pow(position[i].y - des.y, 2) > 5) {
					temp.push_back(position[i]);
				}
				else {
					number--;
					hitPositions.push_back(position[i]);
					hitThisFrame = true;
					if (gameModel == 2) {
						int oldMult = 1;
						if (combo >= 5) oldMult = 5;
						else if (combo >= 4) oldMult = 4;
						else if (combo >= 3) oldMult = 3;
						else if (combo >= 2) oldMult = 2;
						combo++;
						if (combo > 5) combo = 5;
						if (combo > maxCombo) maxCombo = combo;
						int mult = combo;
						if (combo >= 5) mult = 5;
						else if (combo >= 4) mult = 4;
						else if (combo >= 3) mult = 3;
						else if (combo >= 2) mult = 2;
						int basePt = ballScores[i];
						hitScores.push_back(basePt);
						score += basePt * mult;
						totalHits++;
						if (mult > oldMult)
							cout << "COMBO x" << mult << "!" << endl;
					} else {
						int basePt = ballScores[i];
						hitScores.push_back(basePt);
						score += basePt;
						totalHits++;
						combo++;
						if (combo > 5) combo = 5;
					}
				}
			}
			position = temp;
			} // end if (!bossWasHit)
			if (!hitThisFrame) combo = 0;
		}

		if (gameModel == 1)
		{
			for (GLuint i = 0; i < position.size(); )
			{
				position[i].y += moveSpeed;

				if (position[i].y >= SPIKE_TRIGGER_MIN_Y && position[i].y <= SPIKE_TRIGGER_MAX_Y) {
					hitPositions.push_back(position[i]);
					hitScores.push_back(1);
					score++;
					totalHits++;
					hitThisFrame = true;
					number--;
					ballColors.erase(ballColors.begin() + i);
					ballScores.erase(ballScores.begin() + i);
					position.erase(position.begin() + i);
					continue;
				}

				if (position[i].y > WALL_TOP_Y + 5.0f) {
					position[i].y = -5.0f;
				}
				i++;
			}
		}
		else
		{
			float effSpeed = slowMotion ? moveSpeed * 0.4f : moveSpeed;
		for (int i = (int)position.size() - 1; i >= 0; i--) {
				position[i].z += effSpeed;
				if (position[i].z >= 70) {
					ballColors.erase(ballColors.begin() + i);
					ballScores.erase(ballScores.begin() + i);
					position.erase(position.begin() + i);
					number--;
					lives--;
					combo = 0;
					waveTriggered = false;
					cout << "Ouch! Combo reset." << endl;
				}
			}
		}
		if (number == 0) {
			maxNumber++;
			if (maxNumber == 10) {
				moveSpeed += 0.1f;
				maxNumber = 3;
			}
			if (gameModel == 2) {
				bossWaveCount++;
			}
			AddBall();
			waveTriggered = true;
			// spawn boss every 5 waves (challenge mode only)
			if (gameModel == 2 && bossWaveCount % 5 == 0) {
				SpawnBoss();
			}
		}
	}

	bool IsOver() {
		if (gameModel == 1) {
			if (position.size() > 0)
				if (position[0].z >= 70)
					return true;
			return false;
		}
		else {
			return lives <= 0;
		}
	}

	void AddScore(int n) { score += n; }
	void AddLife(int n) { if (lives < 3) lives += n; }
	void SetSlowMotion(bool on) { slowMotion = on; }
	GLuint GetScore() {
		return score;
	}
	vector<vec3> GetHitPositions() {
		return hitPositions;
	}
	vector<int> GetHitScores() {
		return hitScores;
	}
	void SetMovingLightPos(vec3 pos) {
		ballShader->Bind();
		ballShader->SetVec3("movingLightPos", pos);
		ballShader->Unbind();
	}
	GLuint GetLives() {
		return lives;
	}
	GLuint GetCombo() {
		return combo;
	}
	GLuint GetComboMult() {
		return combo > 0 ? combo : 1;
	}
	GLuint GetTotalHits() {
		return totalHits;
	}
	GLuint GetMaxCombo() {
		return maxCombo;
	}
	vector<vec3> GetBallPositions() {
		return position;
	}
	bool CheckWaveTrigger() {
		if (waveTriggered) {
			waveTriggered = false;
			return true;
		}
		return false;
	}
	bool IsBossActive() { return bossActive; }
	int GetBossHP() { return bossHP; }
	int GetBossMaxHP() { return bossMaxHP; }

	void Render(Shader* shader, GLuint depthMap = -1) {
		for (GLuint i = 0; i < position.size(); i++) {
			model = mat4(1.0);
			model[3] = vec4(position[i], 1.0);
			model = scale(model, vec3(5));
			if (shader == NULL) {
				shader = ballShader;
				shader->Bind();
				shader->SetMat4("projection", projection);
				shader->SetMat4("view", view);
				shader->SetVec3("color", ballColors[i]);
			}
			else {
				shader->Bind();
			}
			shader->SetMat4("model", model);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, depthMap);
			glBindVertexArray(ball->GetVAO());
			glDrawElements(GL_TRIANGLES, static_cast<GLuint>(ball->GetIndices().size()), GL_UNSIGNED_INT, 0);
			shader->Unbind();
			glBindVertexArray(0);
			model = mat4(1.0);
		}

		// === Boss rendering (challenge mode) ===
		if (bossActive && gameModel == 2) {
			model = mat4(1.0);
			model[3] = vec4(bossPosition, 1.0);
			float pulseScale = 8.0f + sin(bossPhase * 3.0f) * 0.8f;
			model = scale(model, vec3(pulseScale));
			ballShader->Bind();
			ballShader->SetMat4("projection", projection);
			ballShader->SetMat4("view", view);
			ballShader->SetVec3("color", vec3(1.0f, 0.12f, 0.05f));
			ballShader->SetMat4("model", model);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, depthMap);
			glBindVertexArray(ball->GetVAO());
			glDrawElements(GL_TRIANGLES, static_cast<GLuint>(ball->GetIndices().size()), GL_UNSIGNED_INT, 0);
			ballShader->Unbind();
			glBindVertexArray(0);
			model = mat4(1.0);
		}

		if (gameModel == 1 && spikeVAO != 0 && ballShader) {
			ballShader->Bind();
			ballShader->SetMat4("projection", projection);
			ballShader->SetMat4("view", view);
			ballShader->SetVec3("color", vec3(0.8, 0.1, 0.1));
			glBindVertexArray(spikeVAO);

			for (const auto& spikePos : fixedSpikePositions) {
				mat4 spikeModelMat = mat4(1.0);
				spikeModelMat = translate(spikeModelMat, spikePos);
				spikeModelMat = rotate(spikeModelMat, radians(180.0f), vec3(1, 0, 0));
				ballShader->SetMat4("model", spikeModelMat);
				glDrawElements(GL_TRIANGLES, spikeIndexCount, GL_UNSIGNED_INT, 0);
			}

			ballShader->Unbind();
			glBindVertexArray(0);
		}
	}
private:
	void LoadModel() {
		ball = new Model("res/model/dot.obj");
		ballShader = new Shader("res/shader/ball.vert", "res/shader/ball.frag");
		ballShader->Bind();
		ballShader->SetVec3("color", vec3(0.2, 0.5, 0.5f));
		ballShader->SetInt("shadowMap", 0);
		ballShader->SetVec3("lightPos", lightPos);
		ballShader->SetVec3("viewPos", camera->GetPosition());
		ballShader->SetMat4("lightSpaceMatrix", lightSpaceMatrix);
		ballShader->Unbind();
	}

	void SpawnBoss() {
		bossActive = true;
		bossMaxHP = 3 + bossWaveCount / 10;
		bossHP = bossMaxHP;
		bossOriginalX = (rand() % 2) ? (float)(rand() % 20) : -(float)(rand() % 20);
		bossOriginalY = 8.0f + (float)(rand() % 15);
		bossPosition = vec3(bossOriginalX, bossOriginalY, basicPos.z - 15.0f);
		bossPhase = 0.0f;
		cout << "=== BOSS WAVE " << bossWaveCount << "! HP: " << bossMaxHP << " ===" << endl;
	}

	void AddBall() {
		for (GLuint i = number; i < maxNumber; i++) {
			float judgeX = rand() % 2;
			float x = (judgeX >= 0.5) ? rand() % 30 : -(rand() % 30);
			float y = rand() % 30;
			vec3 pos = vec3(basicPos.x + x, basicPos.y + y, basicPos.z);
			if (CheckPosition(pos)) {
				position.push_back(pos);
				int r = rand() % 100;
				if (r < 5)       { ballColors.push_back(vec3(1.0f, 0.2f, 0.2f)); ballScores.push_back(5); }
				else if (r < 20) { ballColors.push_back(vec3(1.0f, 0.75f, 0.1f)); ballScores.push_back(3); }
				else if (r < 45) { ballColors.push_back(vec3(0.2f, 0.85f, 0.2f)); ballScores.push_back(2); }
				else            { ballColors.push_back(vec3(0.2f, 0.6f, 0.95f)); ballScores.push_back(1); }
				number++;
			}
			else
				i--;
		}
	}

	bool CheckPosition(vec3 pos) {
		for (GLuint i = 0; i < position.size(); i++) {
			float away = pow(position[i].x - pos.x, 2) + pow(position[i].y - pos.y, 2);
			if (away < 100)
				return false;
		}
		return true;
	}

	void InitFixedSpikes() {
		fixedSpikePositions.clear();
		float startX = -BALL_GEN_HALF_WIDTH + (BALL_GEN_HALF_WIDTH * 2) / (SPIKE_COLS + 1);
		float startZ = basicPos.z - 5.0f;
		float zOffset = 10.0f;

		for (int row = 0; row < SPIKE_ROWS; row++) {
			float currentZ = (row == 0) ? startZ : startZ + zOffset;
			for (int col = 0; col < SPIKE_COLS; col++) {
				float offsetX = (col + 1) * ((BALL_GEN_HALF_WIDTH * 2) / (SPIKE_COLS + 1));
				float x = -BALL_GEN_HALF_WIDTH + offsetX;
				fixedSpikePositions.push_back(vec3(x, WALL_TOP_Y, currentZ));
			}
		}
	}

	void InitSpikeGeometry() {
		const int segments = 8;
		const float radius = 2.5f;
		const float height = 6.0f;

		struct Vertex {
			vec3 position;
			vec3 normal;
			vec2 texCoords;
		};

		vector<Vertex> vertices;
		vector<unsigned int> indices;

		vertices.push_back({ vec3(0.0f, height, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec2(0.5f, 0.0f) });

		for (int i = 0; i <= segments; i++) {
			float angle = 2.0f * 3.1415926f * i / segments;
			float x = radius * cos(angle);
			float z = radius * sin(angle);
			vertices.push_back({ vec3(x, 0.0f, z), vec3(0.0f, 1.0f, 0.0f), vec2((cos(angle) + 1) / 2, (sin(angle) + 1) / 2) });
		}

		for (int i = 0; i < segments; i++) {
			indices.push_back(0);
			indices.push_back(i + 2);
			indices.push_back(i + 1 + 2);
		}

		spikeIndexCount = static_cast<unsigned int>(indices.size());

		glGenVertexArrays(1, &spikeVAO);
		glGenBuffers(1, &spikeVBO);
		glGenBuffers(1, &spikeEBO);

		glBindVertexArray(spikeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, spikeVBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, spikeEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(vec3)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2 * sizeof(vec3)));

		glBindVertexArray(0);
	}
};

#endif // !BALLMANAGER_H
