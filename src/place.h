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
	// 房间模型 + 两套墙面
	Model* room = nullptr;
	Texture* texWallChallenge = nullptr; // 挑战 wall_1.jpg
	Texture* texWallRelax = nullptr;     // 休闲 wall_2.jpg
	Shader* roomShader = nullptr;

	// 太阳光源
	Model* sun = nullptr;
	glm::vec3 lightPos;
	glm::mat4 lightSpaceMatrix;
	Shader* sunShader = nullptr;

	Camera* camera = nullptr;
	glm::mat4 model;
	glm::mat4 projection;
	glm::mat4 view;

	int gameModel; // 1休闲  2挑战

public:
	Place(glm::vec2 windowSize, Camera* camera) {
		this->windowSize = windowSize;
		this->camera = camera;
		gameModel = 2; // 默认挑战模式
		this->lightPos = glm::vec3(0.0f, 400.0f, 150.0f);

		glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, 1.0f, 500.0f);
		glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		this->lightSpaceMatrix = lightProjection * lightView;

		LoadModel();
		LoadTexture();
		LoadShader();
	}

	// 析构释放所有堆资源，防止内存泄漏
	~Place()
	{
		delete room;
		delete sun;
		delete texWallChallenge;
		delete texWallRelax;
		delete roomShader;
		delete sunShader;
	}

	// World调用，传入游戏模式
	void SetGameModel(int mode)
	{
		gameModel = mode;
	}

	// 更新变换矩阵
	void Update() {
		this->model = glm::mat4(1.0f);
		this->view = camera->GetViewMatrix();
		float aspect = windowSize.x / windowSize.y;
		this->projection = glm::perspective(glm::radians(camera->GetZoom()), aspect, 0.1f, 500.0f);
	}

	// 渲染房间：自动切换两张墙面
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

		// 纹理单元0：根据模式切换墙面
		glActiveTexture(GL_TEXTURE0);
		if (gameModel == 1)
		{
			// 休闲模式 wall_2.jpg
			glBindTexture(GL_TEXTURE_2D, texWallRelax->GetId());
		}
		else
		{
			// 挑战模式 wall_1.jpg
			glBindTexture(GL_TEXTURE_2D, texWallChallenge->GetId());
		}

		// 纹理单元1：阴影贴图
		if (depthMap != -1) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, depthMap);
		}

		glBindVertexArray(room->GetVAO());
		glDrawElements(GL_TRIANGLES, static_cast<GLuint>(room->GetIndices().size()), GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
		useShader->Unbind();
	}

	// 渲染太阳（逻辑不变）
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
	// 加载房间、太阳模型
	void LoadModel() {
		room = new Model("res/model/room.obj");
		sun = new Model("res/model/sun.obj");
	}

	// 加载两张墙面贴图（匹配你的wall_1、wall_2）
	void LoadTexture() {
		texWallChallenge = new Texture("res/texture/wall_1.jpg");
		texWallRelax = new Texture("res/texture/wall_2.jpg");
	}

	// 加载着色器
	void LoadShader() {
		roomShader = new Shader("res/shader/room.vert", "res/shader/room.frag");
		roomShader->Bind();
		roomShader->SetInt("diffuse", 0);
		roomShader->SetInt("shadowMap", 1);
		roomShader->SetVec3("lightPos", lightPos);
		roomShader->SetVec3("viewPos", camera->GetPosition());
		roomShader->SetMat4("lightSpaceMatrix", lightSpaceMatrix);
		roomShader->Unbind();

		sunShader = new Shader("res/shader/sun.vert", "res/shader/sun.frag");
	}
};

#endif // !PLACE_H