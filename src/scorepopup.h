#ifndef SCOREPOPUP_H
#define SCOREPOPUP_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
#include <vector>
#include <ctime>
#include <cstdlib>
using namespace std;
#include "shader.h"

struct Popup {
    vec3 position;
    int value;         // score gained
    int comboMult;      // combo multiplier
    float life;
    float maxLife;
};

class ScorePopupManager {
private:
    vector<Popup> popups;
    Shader* shader;
    GLuint VAO, VBO_POS, VBO_COLOR, VBO_SIZE;

    // 3x5 dot matrix for digits 0-9
    bool digitPattern[10][15] = {
        {1,1,1, 1,0,1, 1,0,1, 1,0,1, 1,1,1}, // 0
        {0,1,0, 0,1,0, 0,1,0, 0,1,0, 0,1,0}, // 1
        {1,1,1, 0,0,1, 1,1,1, 1,0,0, 1,1,1}, // 2
        {1,1,1, 0,0,1, 1,1,1, 0,0,1, 1,1,1}, // 3
        {1,0,1, 1,0,1, 1,1,1, 0,0,1, 0,0,1}, // 4
        {1,1,1, 1,0,0, 1,1,1, 0,0,1, 1,1,1}, // 5
        {1,1,1, 1,0,0, 1,1,1, 1,0,1, 1,1,1}, // 6
        {1,1,1, 0,0,1, 0,0,1, 0,0,1, 0,0,1}, // 7
        {1,1,1, 1,0,1, 1,1,1, 1,0,1, 1,1,1}, // 8
        {1,1,1, 1,0,1, 1,1,1, 0,0,1, 1,1,1}, // 9
    };

    void addDigit(int digit, vec3 basePos, vec3 right, vec3 up, float dotSize, float gap,
                  float alpha, vector<vec3>& posData, vector<vec4>& colData, vector<float>& sizeData) {
        if (digit < 0 || digit > 9) return;
        for (int row = 0; row < 5; row++) {
            for (int col = 0; col < 3; col++) {
                if (digitPattern[digit][row * 3 + col]) {
                    vec3 dotPos = basePos + right * float(col) * (dotSize + gap) - up * float(row) * (dotSize + gap);
                    posData.push_back(dotPos);
                    colData.push_back(vec4(1.0f, 1.0f, 0.3f, alpha));
                    sizeData.push_back(dotSize * 0.5f);
                }
            }
        }
    }

public:
    ScorePopupManager() {
        shader = new Shader("res/shader/particle.vert", "res/shader/particle.frag");
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO_POS);
        glGenBuffers(1, &VBO_COLOR);
        glGenBuffers(1, &VBO_SIZE);
    }

    void Spawn(vec3 position, int value, int comboMult) {
        Popup p;
        p.position = position;
        p.value = value;
        p.comboMult = comboMult;
        p.maxLife = 1.2f;
        p.life = p.maxLife;
        popups.push_back(p);
    }

    void Update(float deltaTime) {
        for (int i = popups.size() - 1; i >= 0; i--) {
            popups[i].life -= deltaTime;
            if (popups[i].life <= 0.0f) {
                popups.erase(popups.begin() + i);
                continue;
            }
            popups[i].position.y += 2.5f * deltaTime;  // float upward
        }
    }

    void Render(mat4 projection, mat4 view) {
        if (popups.empty()) return;

        // compute camera right/up for billboarding
        mat4 viewInv = inverse(view);
        vec3 camRight = normalize(vec3(viewInv[0]));
        vec3 camUp = normalize(vec3(viewInv[1]));

        vector<vec3> posData;
        vector<vec4> colData;
        vector<float> sizeData;

        for (auto& p : popups) {
            float alpha = p.life / p.maxLife;
            float size = 0.45f;
            float gap = 0.12f;
            float digitW = 3 * (size + gap);

            // Build text: "+" + value + " x" + combo (if > 1)
            int val = p.value;
            int digits[10]; int nd = 0;
            if (val == 0) { digits[0]=0; nd=1; }
            else { while(val>0){digits[nd++]=val%10; val/=10;} }

            // Start position: center the text
            int numChars = nd + 1; // + for "+" sign
            if (p.comboMult > 1) numChars += 2; // " x" + multiplier digit
            float totalW = numChars * digitW;
            vec3 startPos = p.position - camRight * totalW * 0.5f;

            // "+" sign (simple cross: 2 lines of dots)
            vec3 plusPos = startPos;
            // vertical bar of +
            for (int r = 0; r < 5; r++) {
                if (r == 2) { // full row for horizontal
                    for (int c = 0; c < 3; c++) {
                        vec3 dp = plusPos + camRight*float(c)*(size+gap) - camUp*float(r)*(size+gap);
                        posData.push_back(dp);
                        colData.push_back(vec4(1.0f, 1.0f, 1.0f, alpha));
                        sizeData.push_back(size * 0.5f);
                    }
                } else {
                    // just center column
                    vec3 dp = plusPos + camRight*1.0f*(size+gap) - camUp*float(r)*(size+gap);
                    posData.push_back(dp);
                    colData.push_back(vec4(1.0f, 1.0f, 1.0f, alpha));
                    sizeData.push_back(size * 0.5f);
                }
            }

            // digits of value
            float xOff = digitW;
            for (int di = nd-1; di >= 0; di--) {
                addDigit(digits[di], startPos + camRight*xOff, camRight, camUp, size, gap, alpha, posData, colData, sizeData);
                xOff += digitW;
            }

            // "x" multiplier if combo > 1
            if (p.comboMult > 1) {
                // draw "x" mark
                xOff += size; // small gap
                vec3 xPos = startPos + camRight*xOff;
                // diagonal lines of x
                for (int j = 0; j < 5; j++) {
                    vec3 dp1 = xPos + camRight*float(j)*(size+gap)*0.7f - camUp*float(j)*(size+gap)*0.7f;
                    vec3 dp2 = xPos + camRight*float(j)*(size+gap)*0.7f - camUp*float(4-j)*(size+gap)*0.7f;
                    posData.push_back(dp1); colData.push_back(vec4(1.0f,0.7f,0.2f,alpha)); sizeData.push_back(size*0.5f);
                    posData.push_back(dp2); colData.push_back(vec4(1.0f,0.7f,0.2f,alpha)); sizeData.push_back(size*0.5f);
                }
                xOff += digitW * 0.5f;
                addDigit(p.comboMult, startPos + camRight*xOff, camRight, camUp, size, gap, alpha, posData, colData, sizeData);
            }
        }

        if (posData.empty()) return;

        shader->Bind();
        shader->SetMat4("projection", projection);
        shader->SetMat4("view", view);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_POS);
        glBufferData(GL_ARRAY_BUFFER, posData.size()*sizeof(vec3), &posData[0], GL_STREAM_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glBindBuffer(GL_ARRAY_BUFFER, VBO_COLOR);
        glBufferData(GL_ARRAY_BUFFER, colData.size()*sizeof(vec4), &colData[0], GL_STREAM_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glBindBuffer(GL_ARRAY_BUFFER, VBO_SIZE);
        glBufferData(GL_ARRAY_BUFFER, sizeData.size()*sizeof(float), &sizeData[0], GL_STREAM_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glDrawArrays(GL_POINTS, 0, (GLsizei)posData.size());
        glBindVertexArray(0);
        shader->Unbind();
    }
};

#endif
