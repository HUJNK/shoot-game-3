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
			waveNumber = 0;

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
					ball->Update(camera->GetPosition(), camera->GetFront(), false);
				} else {
					// muzzle flash for mode 2
					if (gameModel == 2) {
						vec3 muzzlePos = camera->GetPosition() + camera->GetFront() * 2.0f;
						particles->Explode(muzzlePos, vec4(1.0f, 0.9f, 0.3f, 1.0f), 30, MUZZLE);
					}
					ball->Update(camera->GetPosition(), camera->GetFront(), true);
				}
			player->Update(deltaTime, true);
		}
		else {
			ball->Update(camera->GetPosition(), camera->GetFront(), false);
			player->Update(deltaTime, false);
		}
		// spawn obstacles for mode 2 when wave cleared
		if (gameModel == 2 && obstacles->IsEmpty()) {
			obstacles->SpawnWave();
		}
		place->Update();

		vector<vec3> hits = ball->GetHitPositions();
		for (vec3 hitPos : hits) {
			particles->Explode(hitPos, vec4(0.5f, 0.7f, 0.95f, 1.0f), 250, WATER);
				// spawn floating score text
				int mult = (gameModel == 2) ? ball->GetComboMult() : 1;
				scorePopups->Spawn(hitPos, mult, (gameModel == 2) ? mult : 1);
		}
		particles->Update(deltaTime);
		scorePopups->Update(deltaTime);
		hud->Update(deltaTime);
		// wave detection for mode 2: ballmanager sets flag when balls replenish
		if (gameModel == 2 && ball->CheckWaveTrigger()) {
			waveNumber++;
			hud->ShowWave(waveNumber);
		}
		// ball trail particles for challenge mode
		if (gameModel == 2) {
			vector<vec3> ballPositions = ball->GetBallPositions();
			for (vec3& bp : ballPositions) {
				particles->EmitTrail(bp);
			}
		}
		char title[64];
		if (gameModel == 2) {
			sprintf_s(title, "Lives: %d | Score: %d | Combo: x%d", ball->GetLives(), ball->GetScore(), ball->GetComboMult());
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
		ball->Render(NULL, depthMap);
		particles->Render(
			perspective(radians(camera->GetZoom()), windowSize.x / windowSize.y, 0.1f, 500.0f),
			camera->GetViewMatrix()
		);
		scorePopups->Render(
			perspective(radians(camera->GetZoom()), windowSize.x / windowSize.y, 0.1f, 500.0f),
			camera->GetViewMatrix()
		);
		// HUD: mode 1 shows casual (hits+score+combo), mode 2 shows challenge (lives+score+combo)
		hud->Render((int)ball->GetLives(), (int)ball->GetScore(), (int)ball->GetComboMult(),
			        (int)ball->GetTotalHits(), gameModel == 1);
	}

	GLuint GetScore() {
		return ball->GetScore();
	}

	GLuint GetLives() {
		return ball->GetLives();
	}

	bool IsOver() {
		return ball->IsOver();
	}

	void SetGameModel(GLuint num) {
		ball->SetGameModel(num);
		gameModel = num;
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
