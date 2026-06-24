#ifndef PARTICLE_H
#define PARTICLE_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
#include <vector>
#include <ctime>
#include <cstdlib>
using namespace std;
#include "shader.h"

struct Particle {
    vec3 position;
    vec3 velocity;
    vec4 color;
    float size;
    float life;
    float maxLife;
};

class ParticleSystem {
private:
    vector<Particle> particles;
    GLuint VAO, VBO_POS, VBO_COLOR, VBO_SIZE;
    Shader* shader;
    vec3 gravity;

public:
    ParticleSystem() {
        shader = new Shader("res/shader/particle.vert", "res/shader/particle.frag");

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO_POS);
        glGenBuffers(1, &VBO_COLOR);
        glGenBuffers(1, &VBO_SIZE);
        gravity = vec3(0.0f, -7.0f, 0.0f);
    }

    void Explode(vec3 position, vec4 color, int count = 80) {
        // === CENTER PLUME: shoots straight up like a water column ===
        for (int i = 0; i < 40; i++) {
            Particle p;
            p.position = position;
            float spreadAngle = (rand() % 360) * 3.14159f / 180.0f;
            float spreadRadius = (rand() % 100) / 300.0f;
            float upSpeed = 6.0f + (rand() % 100) / 8.0f;
            p.velocity.x = cos(spreadAngle) * spreadRadius * upSpeed;
            p.velocity.y = upSpeed;
            p.velocity.z = sin(spreadAngle) * spreadRadius * upSpeed;
            p.color = vec4(0.7f + (rand()%30)/100.0f, 0.85f + (rand()%15)/100.0f, 1.0f, 1.0f);
            p.size = 6.0f + (rand() % 100) / 10.0f;
            p.maxLife = 1.0f + (rand() % 100) / 80.0f;
            p.life = p.maxLife;
            particles.push_back(p);
        }
        // === MAIN SPLASH: arcs outward in all directions ===
        for (int i = 0; i < 100; i++) {
            Particle p;
            p.position = position;
            float angle = (rand() % 360) * 3.14159f / 180.0f;
            float elevation = (rand() % 100 - 10) * 3.14159f / 180.0f;
            float speed = 5.0f + (rand() % 100) / 6.0f;
            p.velocity.x = cos(elevation) * cos(angle) * speed;
            p.velocity.y = sin(elevation) * speed;
            p.velocity.z = cos(elevation) * sin(angle) * speed;
            p.color = vec4(0.6f + (rand()%40)/100.0f, 0.8f + (rand()%20)/100.0f, 0.95f + (rand()%5)/100.0f, 1.0f);
            p.size = 5.0f + (rand() % 100) / 10.0f;
            p.maxLife = 0.8f + (rand() % 100) / 70.0f;
            p.life = p.maxLife;
            particles.push_back(p);
        }
        // === FOAM RING: white splash ring at base ===
        for (int j = 0; j < 40; j++) {
            Particle r;
            r.position = position;
            float ringAngle = (j / 40.0f) * 6.28318f;
            float ringSpeed = 7.0f + (rand() % 100) / 10.0f;
            r.velocity.x = cos(ringAngle) * ringSpeed;
            r.velocity.y = 2.0f + (rand() % 100) / 30.0f;
            r.velocity.z = sin(ringAngle) * ringSpeed;
            r.color = vec4(0.9f, 0.95f, 1.0f, 1.0f);
            r.size = 8.0f + (rand() % 100) / 8.0f;
            r.maxLife = 0.5f + (rand() % 100) / 100.0f;
            r.life = r.maxLife;
            particles.push_back(r);
        }
        // === MIST: fine white spray ===
        for (int k = 0; k < 30; k++) {
            Particle m;
            m.position = position;
            float ma = (rand() % 360) * 3.14159f / 180.0f;
            float me = (rand() % 120 - 20) * 3.14159f / 180.0f;
            float ms = 3.0f + (rand() % 100) / 30.0f;
            m.velocity.x = cos(me) * cos(ma) * ms;
            m.velocity.y = sin(me) * ms;
            m.velocity.z = cos(me) * sin(ma) * ms;
            m.color = vec4(1.0f, 1.0f, 1.0f, 0.6f);
            m.size = 4.0f + (rand() % 100) / 20.0f;
            m.maxLife = 0.3f + (rand() % 100) / 200.0f;
            m.life = m.maxLife;
            particles.push_back(m);
        }
    }

    void Update(float deltaTime) {
        for (int i = particles.size() - 1; i >= 0; i--) {
            particles[i].life -= deltaTime;
            if (particles[i].life <= 0.0f) {
                particles.erase(particles.begin() + i);
                continue;
            }
            particles[i].velocity += gravity * deltaTime;
            particles[i].position += particles[i].velocity * deltaTime;
        }
    }

    void Render(mat4 projection, mat4 view) {
        if (particles.empty()) return;

        vector<vec3> posData;
        vector<vec4> colorData;
        vector<float> sizeData;
        for (auto& p : particles) {
            posData.push_back(p.position);
            float alpha = p.life / p.maxLife > 0.0f ? p.life / p.maxLife : 0.0f;
            colorData.push_back(vec4(p.color.r, p.color.g, p.color.b, p.color.a * alpha));
            sizeData.push_back(p.size * alpha);
        }

        shader->Bind();
        shader->SetMat4("projection", projection);
        shader->SetMat4("view", view);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO_POS);
        glBufferData(GL_ARRAY_BUFFER, posData.size() * sizeof(vec3), &posData[0], GL_STREAM_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glBindBuffer(GL_ARRAY_BUFFER, VBO_COLOR);
        glBufferData(GL_ARRAY_BUFFER, colorData.size() * sizeof(vec4), &colorData[0], GL_STREAM_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glBindBuffer(GL_ARRAY_BUFFER, VBO_SIZE);
        glBufferData(GL_ARRAY_BUFFER, sizeData.size() * sizeof(float), &sizeData[0], GL_STREAM_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glDrawArrays(GL_POINTS, 0, (GLsizei)posData.size());

        glBindVertexArray(0);
        shader->Unbind();
    }};

#endif
