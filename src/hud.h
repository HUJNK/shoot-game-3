#ifndef HUD_H
#define HUD_H

#include <glad/glad.h>
#include <vector>
using namespace std;
#include "shader.h"

class HUD {
private:
    Shader* shader;
    GLuint VAO, VBO_POS, VBO_COLOR;

    // 3x5 dot matrix patterns for digits 0-9
    bool digitPatterns[10][15] = {
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

    void drawDigit(int digit, float startX, float startY, float dotSize, float gap) {
        if (digit < 0 || digit > 9) return;
        vector<float> posData;
        vector<float> colorData;
        for (int row = 0; row < 5; row++) {
            for (int col = 0; col < 3; col++) {
                if (digitPatterns[digit][row * 3 + col]) {
                    float x = startX + col * (dotSize + gap);
                    float y = startY - row * (dotSize + gap);
                    // 2 triangles = 6 vertices per dot
                    float verts[] = {
                        x, y,  x+dotSize, y,  x+dotSize, y-dotSize,
                        x, y,  x+dotSize, y-dotSize,  x, y-dotSize,
                    };
                    for (int v = 0; v < 12; v++) posData.push_back(verts[v]);
                    for (int v = 0; v < 6; v++) {
                        colorData.push_back(1.0f); colorData.push_back(1.0f);
                        colorData.push_back(1.0f); colorData.push_back(1.0f);
                    }
                }
            }
        }
        if (!posData.empty()) {
            drawQuads(posData, colorData);
        }
    }

    void drawQuad(float x, float y, float w, float h, float r, float g, float b, float a) {
        float verts[] = { x,y, x+w,y, x+w,y-h, x,y, x+w,y-h, x,y-h };
        vector<float> posData(verts, verts + 12);
        vector<float> colorData;
        for (int i = 0; i < 6; i++) {
            colorData.push_back(r); colorData.push_back(g);
            colorData.push_back(b); colorData.push_back(a);
        }
        drawQuads(posData, colorData);
    }

    void drawQuads(vector<float>& posData, vector<float>& colorData) {
        glBindBuffer(GL_ARRAY_BUFFER, VBO_POS);
        glBufferData(GL_ARRAY_BUFFER, posData.size() * sizeof(float), &posData[0], GL_STREAM_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glBindBuffer(GL_ARRAY_BUFFER, VBO_COLOR);
        glBufferData(GL_ARRAY_BUFFER, colorData.size() * sizeof(float), &colorData[0], GL_STREAM_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(posData.size() / 2));
    }

public:
    HUD() {
        shader = new Shader("res/shader/hud.vert", "res/shader/hud.frag");
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO_POS);
        glGenBuffers(1, &VBO_COLOR);
    }

    void Render(int lives, int score, int comboMult) {
        shader->Bind();
        glBindVertexArray(VAO);
        glDisable(GL_DEPTH_TEST);

        // === LIVES: 3 heart-like squares top-left ===
        float heartX = -0.95f, heartY = 0.92f, heartW = 0.04f, heartH = 0.06f;
        for (int i = 0; i < 3; i++) {
            if (i < lives)
                drawQuad(heartX + i * 0.055f, heartY, heartW, heartH, 1.0f, 0.15f, 0.15f, 1.0f); // red
            else
                drawQuad(heartX + i * 0.055f, heartY, heartW, heartH, 0.25f, 0.25f, 0.25f, 1.0f); // dark gray
        }

        // === SCORE: dot-matrix digits ===
        float digitStartX = -0.95f, digitStartY = 0.78f;
        float dotSize = 0.015f, gap = 0.004f;

        // Convert score to digits
        int s = score;
        int digits[10];
        int numDigits = 0;
        if (s == 0) {
            digits[0] = 0;
            numDigits = 1;
        } else {
            while (s > 0 && numDigits < 10) {
                digits[numDigits++] = s % 10;
                s /= 10;
            }
        }
        // Draw digits from most significant to least
        for (int i = numDigits - 1; i >= 0; i--) {
            int col = numDigits - 1 - i;
            drawDigit(digits[i], digitStartX + col * (dotSize * 3 + gap * 2 + 0.02f), digitStartY, dotSize, gap);
        }

        // === COMBO: show multiplier if > 1 ===
        if (true) {  // always show combo
            float cx = digitStartX + numDigits * (dotSize * 3 + gap * 2 + 0.02f) + 0.03f;
            // x marker
            drawQuad(cx, digitStartY, dotSize * 2, dotSize * 5, 1.0f, 0.8f, 0.0f, 1.0f);
            // multiplier number
            drawDigit(comboMult, cx + 0.03f, digitStartY, dotSize, gap);
        }

        glEnable(GL_DEPTH_TEST);
        glBindVertexArray(0);
        shader->Unbind();
    }
};

#endif
