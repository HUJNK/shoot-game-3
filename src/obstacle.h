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

// casual mode: floating decorative cube that disappears and respawns
struct FloatingCube {
    vec3 position;
    vec3 velocity;
    float lifetime;       // remaining life in seconds
    float maxLifetime;    // total life for fade-out calc
    float bobPhase;
    vec3 color;
    float size;
};

class ObstacleManager {
private:
    vector<Obstacle> obstacles;
    vector<CasualMoveState> casualStates;
    bool casualInitialized;
    vector<FloatingCube> floatingCubes;  // dynamic floating cubes (casual mode)
    float floatingCubeBaseSize;
    int maxFloatingCubes;
    float floatingSpawnTimer;
    vector<vec3> heartVoxels;    // decorative heart cubes (canonical, centered at origin)
    float heartCubeSize;
    vector<vec3> smallHeartPositions;  // centers of small hearts on left side
    float heartZAngle;                // Z-axis rotation for hearts
    // arrows
    GLuint arrowVAO, arrowVBO;
    vector<vec3> arrowPositions;      // glowing upward arrows below spikes
    float arrowGlowPhase;
    float arrowSpinAngle;
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
        maxFloatingCubes = 60;
        floatingCubeBaseSize = 0.7f;
        floatingSpawnTimer = 0.0f;
        heartZAngle = 0.0f;
        arrowGlowPhase = 0.0f;
        arrowSpinAngle = 0.0f;

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

        // Arrow geometry: upward-pointing arrow (flat, facing +Z)
        float arrowVerts[54] = {
            // arrow head triangle (top)
            0.0f, 0.5f, 0.0f,  0,0,1,
            -0.35f, -0.1f, 0.0f,  0,0,1,
             0.35f, -0.1f, 0.0f,  0,0,1,
            // arrow body (rectangle below)
            -0.1f, -0.1f, 0.0f,  0,0,1,
             0.1f, -0.1f, 0.0f,  0,0,1,
             0.1f, -0.5f, 0.0f,  0,0,1,
            -0.1f, -0.1f, 0.0f,  0,0,1,
             0.1f, -0.5f, 0.0f,  0,0,1,
            -0.1f, -0.5f, 0.0f,  0,0,1,
        };
        glGenVertexArrays(1, &arrowVAO);
        glGenBuffers(1, &arrowVBO);
        glBindVertexArray(arrowVAO);
        glBindBuffer(GL_ARRAY_BUFFER, arrowVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(arrowVerts), arrowVerts, GL_STATIC_DRAW);
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

        // === Generate floating decorative cubes in balloon area ===
        floatingCubes.clear();
        // Spawn initial batch of floating cubes scattered across the balloon area
        for (int i = 0; i < maxFloatingCubes; i++) {
            SpawnFloatingCube();
        }
        floatingSpawnTimer = 0.0f;

        // === Generate canonical heart voxels (centered at origin, scale=1.0) ===
        heartVoxels.clear();
        heartCubeSize = 1.0f;
        float heartScale = 1.0f;
        float thickness = 2.0f;
        float step = 0.08f;  // high resolution for clear heart shape

        for (float y = -1.5f; y <= 1.5f; y += step / heartScale) {
            for (float x = -1.5f; x <= 1.5f; x += step / heartScale) {
                float x2 = x * x, y2 = y * y, y3 = y2 * y;
                float val = (x2 + y2 - 1.0f);
                float heart = val * val * val - x2 * y3;
                if (heart <= 0.0f) {
                    for (float z = -thickness / 2.0f; z <= thickness / 2.0f; z += heartCubeSize) {
                        heartVoxels.push_back(vec3(x * heartScale, y * heartScale, z));
                    }
                }
            }
        }

        // === Single small heart on left side ===
        float hz = -30.0f;
        smallHeartPositions.clear();
        smallHeartPositions.push_back(vec3(-38.0f, 24.0f, hz));
        heartZAngle = 0.0f;

        // === Arrow positions in empty space below spike row ===
        arrowPositions.clear();
        float arrowZ = -30.0f;
        for (int ax = -24; ax <= 24; ax += 12) {
            arrowPositions.push_back(vec3((float)ax, 3.0f, arrowZ));
            arrowPositions.push_back(vec3((float)ax + 4.0f, 8.0f, arrowZ));
        }
        for (int ax = -20; ax <= 20; ax += 16) {
            arrowPositions.push_back(vec3((float)ax, 5.0f, arrowZ + 2.0f));
        }
        arrowGlowPhase = 0.0f;
        arrowSpinAngle = 0.0f;

        casualInitialized = true;
    }

    void Update(float deltaTime, bool isCasual) {
        // === Floating cubes: disappear over time + respawn ===
        if (isCasual && casualInitialized) {
            for (int i = (int)floatingCubes.size() - 1; i >= 0; i--) {
                FloatingCube& fc = floatingCubes[i];
                fc.lifetime -= deltaTime;
                fc.bobPhase += deltaTime * (1.5f + fc.size * 2.0f);

                // gentle floating motion
                fc.position += fc.velocity * deltaTime;
                fc.position.y += sin(fc.bobPhase) * 0.3f * deltaTime;

                // wrap around if drifts too far in x
                if (fc.position.x > 35.0f)  fc.position.x = -35.0f;
                if (fc.position.x < -35.0f) fc.position.x = 35.0f;
                // wrap y
                if (fc.position.y > 50.0f)  fc.position.y = 3.0f;
                if (fc.position.y < 3.0f)   fc.position.y = 50.0f;

                // remove if lifetime expired
                if (fc.lifetime <= 0.0f) {
                    floatingCubes.erase(floatingCubes.begin() + i);
                }
            }

            // respawn to maintain count
            floatingSpawnTimer -= deltaTime;
            if (floatingSpawnTimer <= 0.0f && (int)floatingCubes.size() < maxFloatingCubes) {
                SpawnFloatingCube();
                floatingSpawnTimer = 0.3f + (rand() % 100) / 200.0f;  // spawn every 0.3-0.8s
            }

            // heart Z-axis rotation
            heartZAngle += deltaTime * 0.6f;  // gentle spin
            // arrow glow pulse + Z-axis spin
            arrowGlowPhase += deltaTime * 3.0f;
            arrowSpinAngle += deltaTime * 2.5f;  // fast Z spin
        }

        // challenge mode obstacles are stationary, no update needed
        (void)isCasual;
    }

    void SpawnWave(int waveNumber = 1) {
        obstacles.clear();
        // === Progressive difficulty scaling ===
        // Count: starts at 3, +1 every 2 waves, cap at 8
        int count = 3 + waveNumber / 2;
        if (count > 8) count = 8;
        // Health: base 2~3, +1 every 2 waves
        int baseHealth = 2 + waveNumber / 2;
        // Radius: base 3~5, slowly grows
        float baseRadius = 3.0f + waveNumber * 0.4f;
        // Tighter spawn around ball path (z: -35~35 instead of -20~40)
        float tightZ = 20.0f + waveNumber * 2.0f;
        if (tightZ > 50.0f) tightZ = 50.0f;

        for (int i = 0; i < count; i++) {
            Obstacle o;
            o.position.x = spawnMin.x + (rand() % 100) / 100.0f * (spawnMax.x - spawnMin.x);
            o.position.y = spawnMin.y + (rand() % 100) / 100.0f * (spawnMax.y - spawnMin.y);
            o.position.z = spawnMin.z + (rand() % 100) / 100.0f * (tightZ - spawnMin.z);
            o.radius = baseRadius + (rand() % 100) / 20.0f;
            o.maxHealth = baseHealth + rand() % 4;
            o.health = o.maxHealth;
            o.color = vec3(
                0.5f + (rand() % 40) / 100.0f,
                0.2f + (rand() % 30) / 100.0f,
                0.2f + (rand() % 30) / 100.0f
            );
            obstacles.push_back(o);
        }

        cout << "[WAVE " << waveNumber << "] Obstacles: " << count
             << " | HP: " << baseHealth << "~" << (baseHealth + 3)
             << " | Size: " << baseRadius << "~" << (baseRadius + 5.0f) << endl;
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

        // === Render floating decorative cubes (casual mode) ===
        if (!floatingCubes.empty()) {
            shader->Bind();
            shader->SetMat4("projection", projection);
            shader->SetMat4("view", view);
            glBindVertexArray(cubeVAO);
            for (size_t i = 0; i < floatingCubes.size(); i++) {
                const FloatingCube& fc = floatingCubes[i];
                // fade out as lifetime decreases
                float alpha = 1.0f;
                if (fc.lifetime < 1.5f) {
                    alpha = fc.lifetime / 1.5f;
                }

                mat4 model = mat4(1.0);
                model[3] = vec4(fc.position, 1.0);
                model = rotate(model, fc.bobPhase * 0.7f, vec3(0.0f, 1.0f, 0.0f));
                model = rotate(model, fc.bobPhase * 0.5f, vec3(0.0f, 0.0f, 1.0f));
                model = scale(model, vec3(fc.size));
                shader->SetMat4("model", model);
                shader->SetVec3("objColor", fc.color);
                shader->SetFloat("objAlpha", alpha);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
            glBindVertexArray(0);
            shader->Unbind();
        }

        // === Render single rotating heart (red cubes, left side) ===
        if (!heartVoxels.empty() && !smallHeartPositions.empty()) {
            shader->Bind();
            shader->SetMat4("projection", projection);
            shader->SetMat4("view", view);
            shader->SetFloat("objAlpha", 1.0f);
            glBindVertexArray(cubeVAO);
            float hScale = 5.5f;
            vec3 hPos = smallHeartPositions[0];
            for (size_t i = 0; i < heartVoxels.size(); i++) {
                mat4 model = mat4(1.0);
                model = translate(model, hPos);
                model = rotate(model, heartZAngle, vec3(0.0f, 1.0f, 0.0f));  // Y-axis spin
                model = translate(model, heartVoxels[i] * hScale);
                model = scale(model, vec3(heartCubeSize));
                shader->SetMat4("model", model);
                shader->SetVec3("objColor", vec3(1.0f, 0.1f, 0.15f));
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
            glBindVertexArray(0);
            shader->Unbind();
        }

        // === Render glowing upward arrows below spike area ===
        if (!arrowPositions.empty()) {
            // yellow glow pulses
            float glow = 0.5f + sin(arrowGlowPhase) * 0.5f;  // 0.0 to 1.0 pulsing
            vec3 arrowColor = vec3(1.0f, 0.85f + glow * 0.15f, glow * 0.3f);

            shader->Bind();
            shader->SetMat4("projection", projection);
            shader->SetMat4("view", view);
            shader->SetVec3("objColor", arrowColor);
            shader->SetFloat("objAlpha", 1.0f);
            glBindVertexArray(arrowVAO);
            for (size_t i = 0; i < arrowPositions.size(); i++) {
                mat4 model = mat4(1.0);
                model = translate(model, arrowPositions[i]);
                // each arrow spins around Y at its own phase offset
                float spin = arrowSpinAngle + i * 0.7f;
                model = rotate(model, spin, vec3(0.0f, 1.0f, 0.0f));
                model = scale(model, vec3(3.5f));  // arrow size
                shader->SetMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 9);
            }
            glBindVertexArray(0);
            shader->Unbind();
        }
    }

private:
    // Spawn a single floating decorative cube in the balloon area
    void SpawnFloatingCube() {
        FloatingCube fc;
        // balloon area: x ~ -28 to 28, y ~ 3 to 50, z around -30
        fc.position = vec3(
            (rand() % 5600) / 100.0f - 28.0f,   // -28 to 28
            3.0f + (rand() % 4700) / 100.0f,      // 3 to 50
            -30.0f + (rand() % 1000) / 100.0f - 5.0f  // -35 to -25
        );
        // gentle random drift velocity
        fc.velocity = vec3(
            (rand() % 200 - 100) / 100.0f,   // -1.0 to 1.0
            (rand() % 160 - 40) / 100.0f,    // -0.4 to 1.2 (slight upward bias)
            (rand() % 60 - 30) / 100.0f       // -0.3 to 0.3
        );
        fc.maxLifetime = 4.0f + (rand() % 800) / 100.0f;  // 4 to 12 seconds
        fc.lifetime = fc.maxLifetime;
        fc.bobPhase = (rand() % 628) / 100.0f;  // random phase 0 to 2π
        fc.size = floatingCubeBaseSize + (rand() % 60) / 100.0f;  // 0.7 to 1.3

        // random warm colors (pink, red, orange, gold, peach)
        int colorPick = rand() % 6;
        switch (colorPick) {
        case 0: fc.color = vec3(1.0f, 0.1f, 0.2f);  break;  // bright red
        case 1: fc.color = vec3(1.0f, 0.4f, 0.5f);  break;  // pink
        case 2: fc.color = vec3(1.0f, 0.6f, 0.15f); break;  // orange
        case 3: fc.color = vec3(1.0f, 0.85f, 0.25f); break; // gold
        case 4: fc.color = vec3(1.0f, 0.55f, 0.65f); break; // light pink
        case 5: fc.color = vec3(0.95f, 0.2f, 0.35f); break; // crimson
        }

        floatingCubes.push_back(fc);
    }
};

#endif
