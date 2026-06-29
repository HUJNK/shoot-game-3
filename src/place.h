#ifndef PLACE_H
#define PLACE_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "model.h"
#include "texture.h"
#include "shader.h"
#include "camera.h"

class Place {
private:
	glm::vec2 windowSize;
	// ����ģ�� + ����ǽ��
	Model* room = nullptr;
	Texture* texWallChallenge = nullptr; // ��ս wall_1.jpg
	Texture* texWallRelax = nullptr;     // ���� wall_2.jpg
	Texture* texSchool = nullptr;        // school.png for front wall
	Texture* texFloor = nullptr;         // floor.jpg for floor
	Texture* texSideWall = nullptr;      // sidewall.jpg for left/right walls
	Texture* texCeiling = nullptr;       // ceiling.jpg for ceiling
	Texture* texSignature = nullptr;     // signature (student.bmp) on ceiling
	Shader* roomShader = nullptr;

	// ̫����Դ
	Model* sun = nullptr;
	vec3 star1Pos, star2Pos;
	float starGlow;
	glm::vec3 lightPos;
	glm::mat4 lightSpaceMatrix;
	Shader* sunShader = nullptr;

	Camera* camera = nullptr;
	glm::mat4 model;
	glm::mat4 projection;
	glm::mat4 view;

	int gameModel; // 1����  2��ս

public:
	Place(glm::vec2 windowSize, Camera* camera) {
		this->windowSize = windowSize;
		this->camera = camera;
		gameModel = 2; // Ĭ����սģʽ
		star1Pos = vec3(-40.0f, 50.0f, -50.0f);
		star2Pos = vec3( 40.0f, 50.0f, -50.0f);
		starGlow = 0.0f;
		this->lightPos = glm::vec3(0.0f, 400.0f, 150.0f);

		glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, 1.0f, 500.0f);
		glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		this->lightSpaceMatrix = lightProjection * lightView;

		LoadModel();
		LoadTexture();
		LoadShader();
	}

	// �����ͷ����ж���Դ����ֹ�ڴ�й©
	~Place()
	{
		delete room;
		delete sun;
		delete texWallChallenge;
		delete texWallRelax;
			delete texSchool;
			delete texFloor;
			delete texSideWall;
			delete texCeiling;
			delete texSignature;
		delete roomShader;
		delete sunShader;
	}

	// World���ã�������Ϸģʽ
	void SetGameModel(int mode)
	{
		gameModel = mode;
	}

	// ���±任����
	void Update() {
		this->model = glm::mat4(1.0f);
		this->view = camera->GetViewMatrix();
		float aspect = windowSize.x / windowSize.y;
		this->projection = glm::perspective(glm::radians(camera->GetZoom()), aspect, 0.1f, 500.0f);
	}

	// ��Ⱦ���䣺�Զ��л�����ǽ��
	void RoomRender(Shader* shader = nullptr, int depthMap = -1) {
		Shader* useShader = shader;
		if (useShader == nullptr) {
			useShader = roomShader;
			useShader->Bind();
			useShader->SetMat4("view", view);
			useShader->SetMat4("projection", projection);
		}
		else {
			useShader->Bind();
		}
		useShader->SetMat4("model", model);

		// ������Ԫ0������ģʽ�л�ǽ��
		glActiveTexture(GL_TEXTURE0);
		if (gameModel == 1)
		{
			// ����ģʽ wall_2.jpg
			glBindTexture(GL_TEXTURE_2D, texWallRelax->GetId());
		}
		else
		{
			// ��սģʽ wall_1.jpg
			glBindTexture(GL_TEXTURE_2D, texWallChallenge->GetId());
		}

		// ������Ԫ1����Ӱ��ͼ
		if (depthMap != -1) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, depthMap);
		}

		// school.png on unit 2 for front wall
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, texSchool->GetId());

		// floor.jpg on unit 3
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, texFloor->GetId());

		// sidewall.jpg on unit 4
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, texSideWall->GetId());

		// ceiling.jpg on unit 5
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, texCeiling->GetId());

		// signature on unit 6
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, texSignature->GetId());

		glBindVertexArray(room->GetVAO());
		glDrawElements(GL_TRIANGLES, static_cast<GLuint>(room->GetIndices().size()), GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
		useShader->Unbind();
	}

	// ��Ⱦ̫�����߼����䣩
	void StarRender(Model* ballModel, Shader* ballShader) {
		starGlow += 0.05f;
		float pulse = 1.0f + sin(starGlow * 2.5f) * 0.4f;
		float sz = 8.0f * pulse;
		glDepthMask(GL_FALSE);
		ballShader->Bind();
		ballShader->SetMat4("projection", projection);
		ballShader->SetMat4("view", view);
		ballShader->SetVec3("color", vec3(1.0f, 0.85f, 0.15f));
		mat4 m1 = translate(mat4(1.0f), star1Pos);
		m1 = scale(m1, vec3(sz));
		ballShader->SetMat4("model", m1);
		glBindVertexArray(ballModel->GetVAO());
		glDrawElements(GL_TRIANGLES, static_cast<GLuint>(ballModel->GetIndices().size()), GL_UNSIGNED_INT, nullptr);
		mat4 m2 = translate(mat4(1.0f), star2Pos);
		m2 = scale(m2, vec3(sz));
		ballShader->SetMat4("model", m2);
		glDrawElements(GL_TRIANGLES, static_cast<GLuint>(ballModel->GetIndices().size()), GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
		ballShader->Unbind();
		glDepthMask(GL_TRUE);
	}

	void SunRender() {
		Shader* shader = sunShader;
		shader->Bind();
		shader->SetMat4("projection", projection);
		shader->SetMat4("model", model);
		shader->SetMat4("view", view);
		glBindVertexArray(sun->GetVAO());
		glDrawElements(GL_TRIANGLES, static_cast<GLuint>(sun->GetIndices().size()), GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
		shader->Unbind();
	}
private:
	// ���ط��䡢̫��ģ��
	void LoadModel() {
		room = new Model("res/model/room.obj");
		sun = new Model("res/model/sun.obj");
	}

	// ��������ǽ����ͼ��ƥ�����wall_1��wall_2��
	void LoadTexture() {
		texWallChallenge = new Texture("res/texture/wall_new.jpg");
		texWallRelax = new Texture("res/texture/wall_2.jpg");
			texSchool = new Texture("res/texture/school.png");
			texFloor = new Texture("res/texture/floor.jpg");
			texSideWall = new Texture("res/texture/sidewall.jpg");
			texCeiling = new Texture("res/texture/cloud.jpg");
			texSignature = new Texture("res/texture/signature.bmp");
	}

	// ������ɫ��
	void LoadShader() {
		roomShader = new Shader("res/shader/room.vert", "res/shader/room.frag");
		roomShader->Bind();
		roomShader->SetInt("diffuse", 0);
		roomShader->SetInt("shadowMap", 1);
			roomShader->SetInt("schoolTex", 2);
			roomShader->SetInt("floorTex", 3);
			roomShader->SetInt("sideWallTex", 4);
			roomShader->SetInt("ceilingTex", 5);
			roomShader->SetInt("signatureTex", 6);
		roomShader->SetVec3("lightPos", lightPos);
		roomShader->SetVec3("viewPos", camera->GetPosition());
		roomShader->SetMat4("lightSpaceMatrix", lightSpaceMatrix);
		roomShader->Unbind();

		sunShader = new Shader("res/shader/sun.vert", "res/shader/sun.frag");
	}
};

#endif // !PLACE_H