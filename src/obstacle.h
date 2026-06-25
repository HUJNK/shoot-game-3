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

// casual mode: movement state per obstacle
struct CasualMoveState {
    vec3 velocity;
    float boundMin, boundMax;  // bounds for current axis
    int axis;                  // 0=x, 1=y
    float switchTimer;         // time until axis/direction switch
    float switchCooldown;      // cooldown between switches
};

class ObstacleManager {
private:
    vector<Obstacle> obstacles;
    vector<CasualMoveState> casualStates;
    bool casualInitialized;
    vector<vec3> heartVoxels;    // decorative heart cubes
    float heartCubeSize;
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
        casualInitialized = false;

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

    // === Casual Mode: stationary obstacles in ball area ===
    void InitCasual() {
        obstacles.clear();
        casualStates.clear();

        // Scatter 8 fixed obstacles across the ball spawn area (x:-28~28, y:6~42)
        float positions[8][2] = {
            {-20.0f, 10.0f}, { 18.0f, 14.0f}, { -8.0f, 22.0f},
            { 22.0f, 28.0f}, {-22.0f, 32.0f}, {  5.0f, 36.0f},
            {-14.0f, 40.0f}, { 15.0f,  8.0f}
        };
        vec3 colors[8] = {
            vec3(0.9f, 0.35f, 0.25f), vec3(0.25f, 0.7f, 0.35f),
            vec3(0.3f, 0.35f, 0.85f), vec3(0.85f, 0.75f, 0.2f),
            vec3(0.7f, 0.25f, 0.7f),  vec3(0.2f, 0.8f, 0.7f),
            vec3(0.9f, 0.55f, 0.1f),  vec3(0.5f, 0.5f, 0.5f)
        };

        for (int i = 0; i < 8; i++) {
            Obstacle o;
            o.position = vec3(positions[i][0], positions[i][1], -30.0f);
            o.radius = 1.5f;
            o.health = 9999;
            o.maxHealth = 9999;
            o.color = colors[i];
            obstacles.push_back(o);
            CasualMoveState s;
            s.velocity = vec3(0.0f, 0.0f, 0.0f);
            s.boundMin = 0.0f;
            s.boundMax = 0.0f;
            s.axis = 0;
            s.switchTimer = 0.0f;
            s.switchCooldown = 0.0f;
            casualStates.push_back(s);
        }

        // === Generate decorative heart above ball area ===
        heartVoxels.clear();
        heartCubeSize = 0.9f;
        float heartScale = 12.0f;       // total heart size
        float heartZ = -30.0f;          // same z-plane as balls
        float heartY = 35.0f;           // mid-height in ball area
        float heartX = -42.0f;          // left-front of player
        float thickness = 2.0f;         // z-depth of the heart
        float step = 1.0f;              // voxel resolution (bigger = fewer cubes)

        for (float y = -1.5f; y <= 1.5f; y += step / heartScale) {
            for (float x = -1.5f; x <= 1.5f; x += step / heartScale) {
                // 2D heart equation: (x² + y² - 1)³ - x²·y³ ≤ 0
                float x2 = x * x, y2 = y * y, y3 = y2 * y;
                float val = (x2 + y2 - 1.0f);
                float heart = val * val * val - x2 * y3;
                if (heart <= 0.0f) {
                    for (float z = -thickness / 2.0f; z <= thickness / 2.0f; z += heartCubeSize) {
                        heartVoxels.push_back(vec3(
                            heartX + x * heartScale,
                            heartY + y * heartScale,
                            heartZ + z
                        ));
                    }
                }
            }
        }

        casualInitialized = true;
    }

    void Update(float deltaTime, bool isCasual) {
        // casual obstacles are stationary, no update needed
        (void)deltaTime; (void)isCasual;
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

        // === Render decorative heart (red cubes) ===
        if (!heartVoxels.empty()) {
            shader->Bind();
            shader->SetMat4("projection", projection);
            shader->SetMat4("view", view);
            glBindVertexArray(cubeVAO);
            for (size_t i = 0; i < heartVoxels.size(); i++) {
                mat4 model = mat4(1.0);
                model[3] = vec4(heartVoxels[i], 1.0);
                model = scale(model, vec3(heartCubeSize));
                shader->SetMat4("model", model);
                shader->SetVec3("objColor", vec3(1.0f, 0.1f, 0.15f));
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
            glBindVertexArray(0);
            shader->Unbind();
        }
    }
};

#endif
