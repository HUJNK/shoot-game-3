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
	float bossWarningTimer;  // screen flash + warning before boss spawn
	float gameTime;
	vec3 movingLightPos;
	bool tutorialActive;
	bool snowAlive;
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
		snowAlive = true;
		totalShotsFired = 0;
			items = new ItemManager();
			rapidFireTimer = 0.0f;
			rapidFireCooldown = 0.0f;
			slowMotionTimer = 0.0f;
			bossWarningTimer = 0.0f;
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
				// Check snowman hit first
				if (snowAlive && gameModel == 2) {
					vec3 snowPos(80.0f, 5.0f, -20.0f);
					float snowR = 10.0f;
					vec3 oc = camera->GetPosition() - snowPos;
					vec3 dir = camera->GetFront();
					float b = dot(oc, dir);
					float c = dot(oc, oc) - snowR * snowR;
					float disc = b*b - c;
					if (disc > 0) {
						float t = -b - sqrt(disc);
						if (t > 0 && t < 120.0f) {
							// HIT! Collapse snowman
							snowAlive = false;
							vec3 hitPos = camera->GetPosition() + dir * t;
							// Firework burst on top
							particles->AddFireworkBurst(hitPos + vec3(0, 15.0f, 0), 5);
							particles->AddFireworkBurst(hitPos + vec3(3, 12.0f, 0), 3);
						}
					}
				}
				// check obstacles first
				vec3 obsHit = obstacles->CheckHit(camera->GetPosition(), camera->GetFront());
				if (obsHit.x > -900) {
					// hit obstacle: explode, block shot
					particles->AddFireworkBurst(obsHit, 3);
					particles->ExplodeFirework(obsHit, vec4(0.9f, 0.5f, 0.0f, 1.0f), 6);
					ball->Update(camera->GetPosition(), camera->GetFront(), false, deltaTime);
				} else {
					// muzzle flash for mode 2
					if (gameModel == 2) {
						vec3 muzzlePos = camera->GetPosition() + camera->GetFront() * 2.0f;
						particles->Explode(muzzlePos, vec4(1.0f, 0.85f, 0.15f, 1.0f), 120, MUZZLE);
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
				particles->Explode(hitPos, vec4(1.0f, 0.8f, 0.1f, 1.0f), 400, MUZZLE);
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
		// spawn obstacles for mode 2 when wave cleared or game start
		if (gameModel == 2 && obstacles->IsEmpty()) {
			obstacles->SpawnWave(waveNumber > 0 ? waveNumber : 1);
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
					particles->AddFireworkBurst(obsHit, 3);
					particles->ExplodeFirework(obsHit, vec4(0.9f, 0.5f, 0.0f, 1.0f), 6);
					ball->Update(camera->GetPosition(), camera->GetFront(), false, deltaTime);
				} else {
					ball->Update(camera->GetPosition(), camera->GetFront(), true, deltaTime);
				}
				player->Update(deltaTime, true);
			}
		}

		// boss warning countdown → spawn boss when timer expires
		if (bossWarningTimer > 0.0f) {
			bossWarningTimer -= deltaTime;
			if (bossWarningTimer <= 0.0f) {
				bossWarningTimer = 0.0f;
				ball->SpawnPendingBoss();
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
				// Firework burst: store for rendering as beams
			if (gameModel == 2) {
				particles->AddFireworkBurst(hitPos, basePts);
			}
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
		// Dust & Rain
		{ static float dt=0; dt+=deltaTime; if(dt>0.05f){dt=0;
			for(int i=0;i<3;i++){vec3 dp((rand()%200)-100.0f,(rand()%60)-5.0f,(rand()%150)-85.0f);
				particles->EmitDust(dp);}}}
		{ static float rt=0; rt+=deltaTime; if(rt>0.005f){rt=0;
			int d=3+rand()%4; for(int i=0;i<d;i++) particles->EmitRaindrop();}}

		particles->Update(deltaTime);
		scorePopups->Update(deltaTime);
		hud->Update(deltaTime);
		// wave detection for mode 2: ballmanager sets flag when balls replenish
		if (ball->CheckWaveTrigger()) {
			waveNumber++;
			hud->ShowWave(waveNumber);
			// reset obstacles & snowman on each new wave
			if (gameModel == 2) {
				snowAlive = true;
				obstacles->SpawnWave(waveNumber);
				// boss warning: start 2s countdown if boss is pending
				if (ball->IsBossPending()) {
					bossWarningTimer = 2.0f;
				}
			}
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
		// === Solar system (left+right walls, 3D orbits with lighting) ===
		{
			static float orbA = 0;
			orbA += lastDeltaTime;
			Shader* bs = ball->GetBallShader();
			bs->Bind();
			bs->SetMat4("projection", proj);
			bs->SetMat4("view", view);
			bs->SetVec3("viewPos", camera->GetPosition());
			bs->SetVec3("lightPos", vec3(0.0f, 400.0f, 150.0f));
			bs->SetVec3("movingLightPos", movingLightPos);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, depthMap);
			bs->SetMat4("lightSpaceMatrix", lightSpaceMatrix);
			GLuint bv = ball->GetBallVAO();
			auto bi = ball->GetBallIndices();
			float od[3] = {12.0f, 18.0f, 25.0f};
			float os[3] = {1.5f, 1.1f, 0.8f};
			vec3 oc[3] = {{0.2f,0.5f,0.9f},{0.9f,0.3f,0.2f},{0.2f,0.8f,0.3f}};

			auto sys = [&](vec3 sp, float sgn) {
				mat4 sm = translate(mat4(1.0f), sp);
				sm = scale(sm, vec3(7.0f));
				bs->SetMat4("model", sm);
				bs->SetVec3("color", vec3(1.0f, 0.9f, 0.2f));
				glBindVertexArray(bv);
				glDrawElements(GL_TRIANGLES, (GLsizei)bi.size(), GL_UNSIGNED_INT, nullptr);
				for (int i = 0; i < 3; i++) {
					float a = orbA * os[i] * sgn;
					vec3 p = sp + vec3(sin(a)*od[i]*0.3f, cos(a)*od[i], sin(a)*od[i]);
					mat4 m = translate(mat4(1.0f), p);
					m = scale(m, vec3(3.0f));
					bs->SetMat4("model", m);
					bs->SetVec3("color", oc[i]);
					glDrawElements(GL_TRIANGLES, (GLsizei)bi.size(), GL_UNSIGNED_INT, nullptr);
				}
			};
			sys(vec3(-86.0f, 25.0f, -10.0f), 1.0f);
			sys(vec3( 86.0f, 25.0f, -10.0f), -1.0f);
			glBindVertexArray(0);
			bs->Unbind();
		}

		// === Windmill on front wall ===
		{
			static float wa = 0;
			wa += lastDeltaTime * 2.0f;
			Shader wm("res/shader/ball.vert", "res/shader/sun.frag");
			wm.Bind(); wm.SetMat4("projection", proj); wm.SetMat4("view", view);
			GLuint bv = ball->GetBallVAO(); auto bi = ball->GetBallIndices();
			vec3 wc = vec3(0.0f, 30.0f, -70.0f);
			mat4 pole = translate(mat4(1.0f), vec3(wc.x, wc.y-15.0f, wc.z));
			pole = scale(pole, vec3(1.5f, 20.0f, 1.5f));
			wm.SetMat4("model", pole); wm.SetVec3("objColor", vec3(0.55f,0.32f,0.15f));
			glBindVertexArray(bv); glDrawElements(GL_TRIANGLES,(GLsizei)bi.size(),GL_UNSIGNED_INT,nullptr);
			mat4 hub = translate(mat4(1.0f), wc); hub = scale(hub, vec3(4.0f));
			wm.SetMat4("model", hub); wm.SetVec3("objColor", vec3(0.25f,0.25f,0.28f));
			glDrawElements(GL_TRIANGLES,(GLsizei)bi.size(),GL_UNSIGNED_INT,nullptr);
			vec3 bc[3]={{0.95f,0.18f,0.18f},{1.0f,0.82f,0.1f},{0.18f,0.6f,0.95f}};
			for(int i=0;i<3;i++){float a=wa+i*2.094f;mat4 bl=translate(mat4(1.0f),wc);
				bl=rotate(bl,a,vec3(0,0,1));bl=translate(bl,vec3(10.0f,0,0));
				bl=scale(bl,vec3(14.0f,3.5f,0.5f));wm.SetMat4("model",bl);
				wm.SetVec3("objColor",bc[i]);glDrawElements(GL_TRIANGLES,(GLsizei)bi.size(),GL_UNSIGNED_INT,nullptr);}
			glBindVertexArray(0); wm.Unbind();
		}

		// === Wreaths: Olympic rings in circle ===
		{
			static GLuint rVAO=0,rVBO=0; static int rvc=0;
			if(rVAO==0){vector<float> v; float oR=1.0f,iR=0.7f; int seg=120;
				for(int j=0;j<=seg;j++){float a=(float)j/seg*6.283185f;
					v.push_back(oR*cos(a));v.push_back(oR*sin(a));v.push_back(0.0f);
					v.push_back(iR*cos(a));v.push_back(iR*sin(a));v.push_back(0.0f);}
				rvc=(int)v.size()/3;glGenVertexArrays(1,&rVAO);glGenBuffers(1,&rVBO);
				glBindVertexArray(rVAO);glBindBuffer(GL_ARRAY_BUFFER,rVBO);
				glBufferData(GL_ARRAY_BUFFER,v.size()*sizeof(float),v.data(),GL_STATIC_DRAW);
				glEnableVertexAttribArray(0);glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
				glBindVertexArray(0);}
			Shader wr("res/shader/ball.vert","res/shader/sun.frag");
			wr.Bind();wr.SetMat4("projection",proj);wr.SetMat4("view",view);
			vec3 col[5]={{0.0f,0.51f,0.78f},{0.0f,0.0f,0.0f},{0.93f,0.2f,0.31f},{0.99f,0.69f,0.19f},{0.0f,0.65f,0.32f}};
			float wreathR=8.0f,ringR=6.5f; int rc=6; const float PI=3.14159265f;
			auto drawRing=[&](vec3 p,vec3 c){mat4 m=translate(mat4(1.0f),p);
				m=scale(m,vec3(ringR,ringR,0.3f));wr.SetMat4("model",m);wr.SetVec3("objColor",c);
				glDrawArrays(GL_TRIANGLE_STRIP,0,rvc);};
			for(int side=0;side<2;side++){vec3 wc=vec3(side==0?-55.0f:55.0f,28.0f,-72.0f);
				vec3 rp[6];vec3 rc2[6];for(int i=0;i<rc;i++){float a=(float)i/rc*2.0f*PI;
					rp[i]=wc+vec3(cos(a)*wreathR,sin(a)*wreathR,0);rc2[i]=col[i%5];}
				glBindVertexArray(rVAO);for(int i=0;i<rc;i++)drawRing(rp[i],rc2[i]);
				for(int i=0;i<rc;i++){int ni=(i+1)%rc;float ang=atan2(rp[ni].y-rp[i].y,rp[ni].x-rp[i].x);
					wr.SetVec3("objColor",rc2[i]);mat4 m=translate(mat4(1.0f),rp[i]);
					m=scale(m,vec3(ringR,ringR,0.31f));wr.SetMat4("model",m);
					int as=(int)((ang-0.25f)/(2.0f*PI)*120+120)%120;
					int ae=(int)((ang+0.25f)/(2.0f*PI)*120+120)%120;
					if(as<=ae)glDrawArrays(GL_TRIANGLE_STRIP,as*2,(ae-as+1)*2);
					else{glDrawArrays(GL_TRIANGLE_STRIP,as*2,(120-as+1)*2);
						glDrawArrays(GL_TRIANGLE_STRIP,0,(ae+1)*2);}}}
			glBindVertexArray(0);wr.Unbind();
		}

		// === Clouds: textured cloud spheres, left/right sweep ===
		{
			static float cp = 0;
			static bool loaded = false;
			static GLuint cloudTex = 0;
			if (!loaded) {
				int w, h, c;
				unsigned char* data = stbi_load("res/texture/cloud.jpg", &w, &h, &c, 0);
				if (data) {
					glGenTextures(1, &cloudTex);
					glBindTexture(GL_TEXTURE_2D, cloudTex);
					int fmt = (c == 4) ? GL_RGBA : GL_RGB;
					glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					stbi_image_free(data);
				}
				loaded = true;
			}
			cp += lastDeltaTime;
			Shader cs("res/shader/room.vert", "res/shader/cloud.frag");
			cs.Bind(); cs.SetMat4("projection", proj); cs.SetMat4("view", view);
			if (cloudTex) { glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, cloudTex); }
			cs.SetInt("diffuse", 0); cs.SetVec3("objColor", vec3(1.0f));
			GLuint bv = ball->GetBallVAO(); auto bi = ball->GetBallIndices();
			float s = 12.0f, g = 4.0f;
			float shape[5][2] = {{-0.5f,1.0f},{0.5f,1.0f},{-1.5f,-1.0f},{0.0f,-1.0f},{1.5f,-1.0f}};
			auto drawCloud = [&](vec3 c) { for(int i=0;i<5;i++){
				vec3 off(shape[i][0]*g, shape[i][1]*g, 0);
				mat4 m = translate(mat4(1.0f), c+off); m = scale(m, vec3(s));
				cs.SetMat4("model", m); glBindVertexArray(bv);
				glDrawElements(GL_TRIANGLES,(GLsizei)bi.size(),GL_UNSIGNED_INT,nullptr);}};
			drawCloud(vec3(sin(cp*0.5f)*80.0f, 28.0f, -35.0f));
			drawCloud(vec3(sin(cp*0.6f+1.5f)*75.0f, 35.0f, -28.0f));
			glBindVertexArray(0); cs.Unbind();
		}

		place->RoomRender(NULL, depthMap);
		// === Stars: two bright glowing spheres ===
		{
			static float sg=0; sg+=0.05f; float pulse=1.0f+sin(sg*2.5f)*0.4f; float sz=8.0f*pulse;
			vec3 s1(-40.0f,50.0f,-50.0f),s2(40.0f,50.0f,-50.0f);
			Shader st("res/shader/ball.vert","res/shader/sun.frag");
			glDepthMask(GL_FALSE); st.Bind(); st.SetMat4("projection",proj); st.SetMat4("view",view);
			st.SetVec3("objColor",vec3(1.0f,0.85f,0.1f));
			GLuint bv=ball->GetBallVAO(); auto bi=ball->GetBallIndices();
			mat4 ms=translate(mat4(1.0f),s1);ms=scale(ms,vec3(sz));st.SetMat4("model",ms);
			glBindVertexArray(bv);glDrawElements(GL_TRIANGLES,(GLsizei)bi.size(),GL_UNSIGNED_INT,nullptr);
			ms=translate(mat4(1.0f),s2);ms=scale(ms,vec3(sz));st.SetMat4("model",ms);
			glDrawElements(GL_TRIANGLES,(GLsizei)bi.size(),GL_UNSIGNED_INT,nullptr);
			glBindVertexArray(0);st.Unbind();glDepthMask(GL_TRUE);
		}

		// === Snowman on right side (rotating, shootable) ===
		{
			static float snowRot = 0;
			if (snowAlive) {
				snowRot += lastDeltaTime * 2.5f;
				Shader* sm = ball->GetBallShader();
				sm->Bind(); sm->SetMat4("projection",proj); sm->SetMat4("view",view);
				sm->SetVec3("viewPos",camera->GetPosition());
				sm->SetVec3("lightPos",vec3(0,400,150));
				sm->SetVec3("movingLightPos",movingLightPos);
				glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D,depthMap);
				GLuint bv=ball->GetBallVAO(); auto bi=ball->GetBallIndices();
				vec3 sc(80.0f, 5.0f, -20.0f);
				auto rotPos = [&](vec3 local) -> vec3 {
					mat4 r = rotate(mat4(1.0f), snowRot, vec3(0,1,0));
					return sc + vec3(r * vec4(local, 1.0f));
				};
				// Bottom
				mat4 bb=translate(mat4(1.0f),sc); bb=rotate(bb,snowRot,vec3(0,1,0)); bb=scale(bb,vec3(10.0f,8.5f,10.0f));
				sm->SetMat4("model",bb); sm->SetVec3("color",vec3(0.95f,0.95f,0.97f));
				glBindVertexArray(bv); glDrawElements(GL_TRIANGLES,(GLsizei)bi.size(),GL_UNSIGNED_INT,nullptr);
				// Middle
				mat4 mb=translate(mat4(1.0f),rotPos(vec3(0,6.5f,0))); mb=rotate(mb,snowRot,vec3(0,1,0)); mb=scale(mb,vec3(7.5f,6.5f,7.5f));
				sm->SetMat4("model",mb); sm->SetVec3("color",vec3(0.95f,0.95f,0.97f));
				glDrawElements(GL_TRIANGLES,(GLsizei)bi.size(),GL_UNSIGNED_INT,nullptr);
				// Head
				mat4 hd=translate(mat4(1.0f),rotPos(vec3(0,11.5f,0))); hd=rotate(hd,snowRot,vec3(0,1,0)); hd=scale(hd,vec3(5.5f,5.0f,5.5f));
				sm->SetMat4("model",hd); sm->SetVec3("color",vec3(0.95f,0.95f,0.97f));
				glDrawElements(GL_TRIANGLES,(GLsizei)bi.size(),GL_UNSIGNED_INT,nullptr);
				// Eyes
				mat4 le=translate(mat4(1.0f),rotPos(vec3(-1.5f,13.0f,2.5f))); le=rotate(le,snowRot,vec3(0,1,0)); le=scale(le,vec3(0.7f));
				sm->SetMat4("model",le); sm->SetVec3("color",vec3(0.05f,0.05f,0.05f));
				glDrawElements(GL_TRIANGLES,(GLsizei)bi.size(),GL_UNSIGNED_INT,nullptr);
				mat4 re=translate(mat4(1.0f),rotPos(vec3(1.5f,13.0f,2.5f))); re=rotate(re,snowRot,vec3(0,1,0)); re=scale(re,vec3(0.7f));
				sm->SetMat4("model",re); sm->SetVec3("color",vec3(0.05f,0.05f,0.05f));
				glDrawElements(GL_TRIANGLES,(GLsizei)bi.size(),GL_UNSIGNED_INT,nullptr);
				// Nose
				mat4 cn=translate(mat4(1.0f),rotPos(vec3(0,12.0f,3.0f))); cn=rotate(cn,snowRot,vec3(0,1,0));
				cn=rotate(cn,-0.3f,vec3(1,0,0)); cn=scale(cn,vec3(0.5f,0.5f,2.5f));
				sm->SetMat4("model",cn); sm->SetVec3("color",vec3(1.0f,0.5f,0.15f));
				glDrawElements(GL_TRIANGLES,(GLsizei)bi.size(),GL_UNSIGNED_INT,nullptr);
				glBindVertexArray(0); sm->Unbind();
			}
		}

		place->SunRender();
		obstacles->SetViewPos(camera->GetPosition());
		obstacles->SetMovingLightPos(movingLightPos);
		obstacles->Render(
			perspective(radians(camera->GetZoom()), windowSize.x / windowSize.y, 0.1f, 500.0f),
			camera->GetViewMatrix(),
			lightSpaceMatrix,
			depthMap
		);
		items->Render(
			perspective(radians(camera->GetZoom()), windowSize.x / windowSize.y, 0.1f, 500.0f),
			camera->GetViewMatrix(),
			gameTime
		);
		ball->SetMovingLightPos(movingLightPos);
		ball->Render(NULL, depthMap);
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		particles->Render(
			perspective(radians(camera->GetZoom()), windowSize.x / windowSize.y, 0.1f, 500.0f),
			camera->GetViewMatrix()
		);
		particles->RenderFireworks(
			perspective(radians(camera->GetZoom()), windowSize.x / windowSize.y, 0.1f, 500.0f),
			camera->GetViewMatrix()
		);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_TRUE);
		scorePopups->Render(
			perspective(radians(camera->GetZoom()), windowSize.x / windowSize.y, 0.1f, 500.0f),
			camera->GetViewMatrix()
		);
		// HUD: mode 1 shows casual, mode 2 shows challenge + effect timers
		hud->Render((int)ball->GetLives(), (int)ball->GetScore(), (int)ball->GetComboMult(),
			        (int)ball->GetTotalHits(), gameModel == 1, rapidFireTimer, slowMotionTimer,
			        ball->IsBossActive(), ball->GetBossHP(), ball->GetBossMaxHP(),
			        bossWarningTimer);
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
		obstacles->RenderDepth(simpleDepthShader, lightSpaceMatrix);
		items->RenderDepth(simpleDepthShader, lightSpaceMatrix);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glViewport(0, 0, windowSize.x, windowSize.y);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
};

#endif
