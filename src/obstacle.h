#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
#include <vector>
#include <ctime>
#include <cstdlib>
using namespace std;
#include "shader.h"

struct Obstacle {
    vec3 position;
    float radius;      // collision sphere radius
    int health;        // hits to destroy
    int maxHealth;
    vec3 color;
};

class ObstacleManager {
private:
    vector<Obstacle> obstacles;
    Shader* shader;
    GLuint cubeVAO, cubeVBO;
    int maxObstacles;
    vec3 spawnMin, spawnMax;  // spawn area

    // unit cube vertices (position + normal)
    float cubeVerts[288] = {
        // front face
        -0.5f,-0.5f, 0.5f,  0,0,1,   0.5f,-0.5f, 0.5f,  0,0,1,   0.5f, 0.5f, 0.5f,  0,0,1,
        -0.5f,-0.5f, 0.5f,  0,0,1,   0.5f, 0.5f, 0.5f,  0,0,1,  -0.5f, 0.5f, 0.5f,  0,0,1,
        // back
         0.5f,-0.5f,-0.5f,  0,0,-1,  -0.5f,-0.5f,-0.5f,  0,0,-1,  -0.5f, 0.5f,-0.5f,  0,0,-1,
         0.5f,-0.5f,-0.5f,  0,0,-1,  -0.5f, 0.5f,-0.5f,  0,0,-1,   0.5f, 0.5f,-0.5f,  0,0,-1,
        // top
        -0.5f, 0.5f,-0.5f,  0,1,0,  -0.5f, 0.5f, 0.5f,  0,1,0,   0.5f, 0.5f, 0.5f,  0,1,0,
        -0.5f, 0.5f,-0.5f,  0,1,0,   0.5f, 0.5f, 0.5f,  0,1,0,   0.5f, 0.5f,-0.5f,  0,1,0,
        // bottom
        -0.5f,-0.5f,-0.5f,  0,-1,0,   0.5f,-0.5f, 0.5f,  0,-1,0,  -0.5f,-0.5f, 0.5f,  0,-1,0,
        -0.5f,-0.5f,-0.5f,  0,-1,0,   0.5f,-0.5f,-0.5f,  0,-1,0,   0.5f,-0.5f, 0.5f,  0,-1,0,
        // right
         0.5f,-0.5f, 0.5f,  1,0,0,   0.5f,-0.5f,-0.5f,  1,0,0,   0.5f, 0.5f,-0.5f,  1,0,0,
         0.5f,-0.5f, 0.5f,  1,0,0,   0.5f, 0.5f,-0.5f,  1,0,0,   0.5f, 0.5f, 0.5f,  1,0,0,
        // left
        -0.5f,-0.5f,-0.5f, -1,0,0,  -0.5f,-0.5f, 0.5f, -1,0,0,  -0.5f, 0.5f, 0.5f, -1,0,0,
        -0.5f,-0.5f,-0.5f, -1,0,0,  -0.5f, 0.5f, 0.5f, -1,0,0,  -0.5f, 0.5f,-0.5f, -1,0,0,
    };

public:
    ObstacleManager() {
        shader = new Shader("res/shader/obstacle.vert", "res/shader/obstacle.frag");
        maxObstacles = 5;
        spawnMin = vec3(-40, 2, -20);
        spawnMax = vec3(40, 25, 40);

        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        glBindVertexArray(cubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
        glBindVertexArray(0);
    }

    void SpawnWave() {
        obstacles.clear();
        int count = 2 + rand() % (maxObstacles - 1);
        for (int i = 0; i < count; i++) {
            Obstacle o;
            o.position.x = spawnMin.x + (rand() % 100) / 100.0f * (spawnMax.x - spawnMin.x);
            o.position.y = spawnMin.y + (rand() % 100) / 100.0f * (spawnMax.y - spawnMin.y);
            o.position.z = spawnMin.z + (rand() % 100) / 100.0f * (spawnMax.z - spawnMin.z);
            o.radius = 3.0f + (rand() % 100) / 20.0f;
            o.maxHealth = 2 + rand() % 4;
            o.health = o.maxHealth;
            o.color = vec3(0.6f + (rand()%30)/100.0f, 0.3f + (rand()%20)/100.0f, 0.2f + (rand()%20)/100.0f);
            obstacles.push_back(o);
        }
    }

    // check if a ray hits any obstacle, return hit position or (-999,...) if miss
    vec3 CheckHit(vec3 rayOrigin, vec3 rayDir) {
        for (int i = 0; i < (int)obstacles.size(); i++) {
            Obstacle& o = obstacles[i];
            vec3 oc = rayOrigin - o.position;
            float b = dot(oc, rayDir);
            float c = dot(oc, oc) - o.radius * o.radius;
            float disc = b*b - c;
            if (disc > 0) {
                float t = -b - sqrt(disc);
                if (t > 0) {
                    vec3 hitPos = rayOrigin + rayDir * t;
                    o.health--;
                    if (o.health <= 0) {
                        obstacles.erase(obstacles.begin() + i);
                    }
                    return hitPos;
                }
            }
        }
        return vec3(-999, -999, -999);
    }

    bool IsEmpty() { return obstacles.empty(); }

    void Render(mat4 projection, mat4 view) {
        for (auto& o : obstacles) {
            mat4 model = mat4(1.0);
            model[3] = vec4(o.position, 1.0);
            model = scale(model, vec3(o.radius));
            // color gets redder as health decreases
            float ratio = (float)o.health / o.maxHealth;
            vec3 col = mix(vec3(0.8, 0.2, 0.1), o.color, ratio);

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
};

#endif
