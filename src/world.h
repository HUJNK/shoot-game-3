#ifndef WORLD_H
#define WORLD_H

#include <GLFW/glfw3.h>
#include "place.h"
#include "player.h"
#include "camera.h"
#include "ballmanager.h"
#include "particle.h"
#include "skybox.h"
#include "hud.h"
#include "obstacle.h"
#include "scorepopup.h"
#include "item.h"

class World {
private:
	GLFWwindow* window;
	vec2 windowSize;

	Place* place;
	Player* player;
	Camera* camera;
	BallManager* ball;
	ParticleSystem* particles;
	Skybox* skybox;
	HUD* hud;
	ObstacleManager* obstacles;
	ScorePopupManager* scorePopups;
	GLuint gameModel;
	bool wasLeftPressed;
	float lastDeltaTime;
	int waveNumber;
	ItemManager* items;
	float rapidFireTimer;
	float rapidFireCooldown;
	float slowMotionTimer;
	float gameTime;
	vec3 movingLightPos;
	bool tutorialActive;
	GLuint totalShotsFired;

	GLuint depthMap;
	GLuint depthMapFBO;
	Shader* simpleDepthShader;
	mat4 lightSpaceMatrix;
public:
	World(GLFWwindow* window, vec2 windowSize) {
		this->window = window;
		this->windowSize = windowSize;

		simpleDepthShader = new Shader("res/shader/shadow.vert", "res/shader/shadow.frag");

		vec3 lightPos(0.0, 400.0, 150.0);
		mat4 lightProjection = ortho(-100.0f, 100.0f, -100.0f, 100.0f, 1.0f, 500.0f);
		mat4 lightView = lookAt(lightPos, vec3(0.0f), vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;

		camera = new Camera(window);
		place = new Place(windowSize, camera);
		player = new Player(windowSize, camera);
		ball = new BallManager(windowSize, camera);
		particles = new ParticleSystem();
		skybox = new Skybox();
			hud = new HUD();
			obstacles = new ObstacleManager();
			scorePopups = new ScorePopupManager();
			wasLeftPressed = false;
				gameModel = 0;
				lastDeltaTime = 0.0f;
			waveNumber = 0;
		tutorialActive = false;
		totalShotsFired = 0;
			items = new ItemManager();
			rapidFireTimer = 0.0f;
			rapidFireCooldown = 0.0f;
			slowMotionTimer = 0.0f;
			gameTime = 0.0f;

		glGenFramebuffers(1, &depthMapFBO);
		glGenTextures(1, &depthMap);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	void Update(float deltaTime) {
		if (tutorialActive) {
			gameTime += deltaTime;
			camera->Update(deltaTime);
			if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
				tutorialActive = false;
				wasLeftPressed = true;
			}
			return;
		}
		gameTime += deltaTime;
		// orbiting moving light (warm, circles through shadow areas)
		movingLightPos = vec3(
			25.0f * sin(gameTime * 0.7f),
			8.0f + 6.0f * sin(gameTime * 1.1f),
			10.0f + 25.0f * cos(gameTime * 0.7f)
		);
		camera->Update(deltaTime);
		bool leftPressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
		bool justShot = leftPressed && !wasLeftPressed;
		wasLeftPressed = leftPressed;

		if (justShot) {
				// check obstacles first
				vec3 obsHit = obstacles->CheckHit(camera->GetPosition(), camera->GetFront());
				if (obsHit.x > -900) {
					// hit obstacle: explode, block shot
					particles->Explode(obsHit, vec4(0.8f, 0.3f, 0.1f, 1.0f), 120, DEBRIS);
					ball->Update(camera->GetPosition(), camera->GetFront(), false, deltaTime);
				} else {
					// muzzle flash for mode 2
					if (gameModel == 2) {
						vec3 muzzlePos = camera->GetPosition() + camera->GetFront() * 2.0f;
						particles->Explode(muzzlePos, vec4(1.0f, 0.9f, 0.3f, 1.0f), 30, MUZZLE);
					}
					ball->Update(camera->GetPosition(), camera->GetFront(), true, deltaTime);
				}
			player->Update(deltaTime, true);
		}
		else {
			ball->Update(camera->GetPosition(), camera->GetFront(), false, deltaTime);
			player->Update(deltaTime, false);
		}
		// item hit check for mode 2 (every frame)
		if (gameModel == 2) {
			int itemHit = items->CheckHit(camera->GetPosition(), camera->GetFront());
			if (itemHit >= 0) {
				vec3 hitPos = camera->GetPosition() + camera->GetFront() * 20.0f;
				particles->Explode(hitPos, vec4(1.0f, 0.85f, 0.2f, 1.0f), 80, WATER);
				switch (itemHit) {
				case ITEM_RAPID_FIRE:
					rapidFireTimer = 2.0f;
					rapidFireCooldown = 0.0f;
					break;
				case ITEM_SLOW_MOTION:
					slowMotionTimer = 3.0f;
					ball->SetSlowMotion(true);
					break;
				case ITEM_BONUS_SCORE:
					ball->AddScore(10);
					break;
				case ITEM_EXTRA_LIFE:
					ball->AddLife(1);
					break;
				}
			}
		}
		// spawn obstacles for mode 2 when wave cleared
		if (gameModel == 2 && obstacles->IsEmpty()) {
			obstacles->SpawnWave();
		}
			// casual mode: update moving obstacles
			obstacles->Update(deltaTime, gameModel == 1);
		place->Update();

		// rapid fire: auto-shoot while timer active
		if (gameModel == 2 && rapidFireTimer > 0.0f) {
			rapidFireCooldown -= deltaTime;
			if (rapidFireCooldown <= 0.0f) {
				rapidFireCooldown = 0.15f;
				vec3 obsHit = obstacles->CheckHit(camera->GetPosition(), camera->GetFront());
				if (obsHit.x > -900) {
					particles->Explode(obsHit, vec4(0.8f, 0.3f, 0.1f, 1.0f), 120, DEBRIS);
					ball->Update(camera->GetPosition(), camera->GetFront(), false, deltaTime);
				} else {
					ball->Update(camera->GetPosition(), camera->GetFront(), true, deltaTime);
				}
				player->Update(deltaTime, true);
			}
		}

		// update effect timers
		if (rapidFireTimer > 0.0f) {
			rapidFireTimer -= deltaTime;
			if (rapidFireTimer <= 0.0f) rapidFireTimer = 0.0f;
		}
		if (slowMotionTimer > 0.0f) {
			slowMotionTimer -= deltaTime;
			if (slowMotionTimer <= 0.0f) {
				slowMotionTimer = 0.0f;
				ball->SetSlowMotion(false);
			}
		}

		vector<vec3> hits = ball->GetHitPositions();
		vector<int> hitScores = ball->GetHitScores();
		for (size_t hi = 0; hi < hits.size(); hi++) {
			vec3 hitPos = hits[hi];
			int basePts = (hi < hitScores.size()) ? hitScores[hi] : 1;
			if (gameModel == 2) {
				if (basePts >= 25) {
					// Boss kill - MASSIVE multi-layer explosion
					particles->Explode(hitPos, vec4(1.0f, 0.15f, 0.05f, 1.0f), 500, DEBRIS);
					particles->Explode(hitPos, vec4(1.0f, 0.7f, 0.1f, 1.0f), 350, WATER);
					particles->Explode(hitPos, vec4(1.0f, 1.0f, 1.0f, 1.0f), 200, MUZZLE);
					particles->Explode(hitPos, vec4(0.5f, 0.2f, 1.0f, 1.0f), 150, WATER);
				} else if (basePts >= 5)
					particles->Explode(hitPos, vec4(1.0f, 0.3f, 0.2f, 1.0f), 350, DEBRIS);
				else if (basePts >= 3)
					particles->Explode(hitPos, vec4(1.0f, 0.85f, 0.15f, 1.0f), 280, WATER);
				else
					particles->Explode(hitPos, vec4(0.5f, 0.7f, 0.95f, 1.0f), 250, WATER);
				int mult = ball->GetComboMult();
				scorePopups->Spawn(hitPos, basePts * mult, mult);
			} else {
				vec4 tint = vec4(0.5f, 0.7f, 0.95f, 1.0f);
				if (basePts >= 5)      tint = vec4(1.0f, 0.3f, 0.2f, 1.0f);
				else if (basePts >= 3) tint = vec4(1.0f, 0.85f, 0.15f, 1.0f);
				else if (basePts >= 2) tint = vec4(0.2f, 0.85f, 0.2f, 1.0f);
				particles->Explode(hitPos, tint, 200, WATER);
				scorePopups->Spawn(hitPos, basePts, 1);
			}
		}
		particles->Update(deltaTime);
		scorePopups->Update(deltaTime);
		hud->Update(deltaTime);
		// wave detection for mode 2: ballmanager sets flag when balls replenish
		if (ball->CheckWaveTrigger()) {
			waveNumber++;
			hud->ShowWave(waveNumber);
		}
		// ball trail particles for challenge mode
		if (gameModel == 2) {
			vector<vec3> ballPositions = ball->GetBallPositions();
			for (vec3& bp : ballPositions) {
				particles->EmitTrail(bp);
			}
			items->Update(deltaTime, particles);
		}
		char title[128];
		if (gameModel == 2) {
			char fx[32] = "";
			if (rapidFireTimer > 0.0f) strcat_s(fx, " [RAPID]");
			if (slowMotionTimer > 0.0f) strcat_s(fx, " [SLOW]");
			sprintf_s(title, "Lives: %d | Score: %d | Combo: x%d%s", ball->GetLives(), ball->GetScore(), ball->GetComboMult(), fx);
		} else {
			sprintf_s(title, "Score: %d", ball->GetScore());
		}
		glfwSetWindowTitle(window, title);
		lastDeltaTime = deltaTime;
	}

	void Render() {
		RenderDepth();
		mat4 proj = perspective(radians(camera->GetZoom()), windowSize.x / windowSize.y, 0.1f, 500.0f);
		mat4 view = camera->GetViewMatrix();
		skybox->Render(proj, view, normalize(vec3(0.0f, 400.0f, 150.0f)), lastDeltaTime);
		player->Render();
		place->RoomRender(NULL, depthMap);
		place->SunRender();
		obstacles->Render(
			perspective(radians(camera->GetZoom()), windowSize.x / windowSize.y, 0.1f, 500.0f),
			camera->GetViewMatrix()
		);
		items->Render(
			perspective(radians(camera->GetZoom()), windowSize.x / windowSize.y, 0.1f, 500.0f),
			camera->GetViewMatrix(),
			gameTime
		);
		ball->SetMovingLightPos(movingLightPos);
		ball->Render(NULL, depthMap);
		particles->Render(
			perspective(radians(camera->GetZoom()), windowSize.x / windowSize.y, 0.1f, 500.0f),
			camera->GetViewMatrix()
		);
		scorePopups->Render(
			perspective(radians(camera->GetZoom()), windowSize.x / windowSize.y, 0.1f, 500.0f),
			camera->GetViewMatrix()
		);
		// HUD: mode 1 shows casual, mode 2 shows challenge + effect timers
		hud->Render((int)ball->GetLives(), (int)ball->GetScore(), (int)ball->GetComboMult(),
			        (int)ball->GetTotalHits(), gameModel == 1, rapidFireTimer, slowMotionTimer,
			        ball->IsBossActive(), ball->GetBossHP(), ball->GetBossMaxHP());
		}

	GLuint GetScore() {
		return ball->GetScore();
	}

	GLuint GetLives() {
		return ball->GetLives();
	}
	GLuint GetTotalShotsFired() { return totalShotsFired; }
	GLuint GetTotalHits() { return ball->GetTotalHits(); }
	GLuint GetMaxCombo() { return ball->GetMaxCombo(); }
	int GetWaveNumber() { return waveNumber; }

	bool IsOver() {
		return ball->IsOver();
	}

	void SetGameModel(GLuint num) {
		ball->SetGameModel(num);
		gameModel = num;
		tutorialActive = false;
		if (num == 1) {
			obstacles->InitCasual();
			particles->SetExplosionSizeScale(1.6f);
		}
	}
private:
	void RenderDepth() {
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		simpleDepthShader->Bind();
		simpleDepthShader->SetMat4("lightSpaceMatrix", lightSpaceMatrix);

		glViewport(0, 0, 1024, 1024);
		glClear(GL_DEPTH_BUFFER_BIT);
		place->RoomRender(simpleDepthShader);
		ball->Render(simpleDepthShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glViewport(0, 0, windowSize.x, windowSize.y);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
};

#endif
