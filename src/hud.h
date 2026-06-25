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

    // wave notification state
    int waveNum;
    float waveTimer;
    static const float WAVE_DURATION;

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

    // 5x5 dot matrix patterns for letters (W, A, V, E)
    bool letterW[25] = {
        1,0,0,0,1,
        1,0,0,0,1,
        1,0,1,0,1,
        1,1,0,1,1,
        1,0,0,0,1
    };
    bool letterA[25] = {
        0,1,1,1,0,
        1,0,0,0,1,
        1,1,1,1,1,
        1,0,0,0,1,
        1,0,0,0,1
    };
    bool letterV[25] = {
        1,0,0,0,1,
        1,0,0,0,1,
        1,0,0,0,1,
        0,1,0,1,0,
        0,0,1,0,0
    };
    bool letterE[25] = {
        1,1,1,1,1,
        1,0,0,0,0,
        1,1,1,1,1,
        1,0,0,0,0,
        1,1,1,1,1
    };

    void drawDigit(int digit, float startX, float startY, float dotSize, float gap, float r, float g, float b, float a) {
        if (digit < 0 || digit > 9) return;
        vector<float> posData;
        vector<float> colorData;
        for (int row = 0; row < 5; row++) {
            for (int col = 0; col < 3; col++) {
                if (digitPatterns[digit][row * 3 + col]) {
                    float x = startX + col * (dotSize + gap);
                    float y = startY - row * (dotSize + gap);
                    float verts[] = {
                        x, y,  x+dotSize, y,  x+dotSize, y-dotSize,
                        x, y,  x+dotSize, y-dotSize,  x, y-dotSize,
                    };
                    for (int v = 0; v < 12; v++) posData.push_back(verts[v]);
                    for (int v = 0; v < 6; v++) {
                        colorData.push_back(r); colorData.push_back(g);
                        colorData.push_back(b); colorData.push_back(a);
                    }
                }
            }
        }
        if (!posData.empty()) {
            drawQuads(posData, colorData);
        }
    }

    void drawLetter(char c, float startX, float startY, float dotSize, float gap,
                    float r, float g, float b, float a) {
        bool* pattern = nullptr;
        switch (c) {
        case 'W': pattern = letterW; break;
        case 'A': pattern = letterA; break;
        case 'V': pattern = letterV; break;
        case 'E': pattern = letterE; break;
        default: return;
        }
        vector<float> posData;
        vector<float> colorData;
        for (int row = 0; row < 5; row++) {
            for (int col = 0; col < 5; col++) {
                if (pattern[row * 5 + col]) {
                    float x = startX + col * (dotSize + gap);
                    float y = startY - row * (dotSize + gap);
                    float verts[] = {
                        x, y,  x+dotSize, y,  x+dotSize, y-dotSize,
                        x, y,  x+dotSize, y-dotSize,  x, y-dotSize,
                    };
                    for (int v = 0; v < 12; v++) posData.push_back(verts[v]);
                    for (int v = 0; v < 6; v++) {
                        colorData.push_back(r); colorData.push_back(g);
                        colorData.push_back(b); colorData.push_back(a);
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
        waveNum = 0;
        waveTimer = 0.0f;
    }

    void ShowWave(int n) {
        waveNum = n;
        waveTimer = WAVE_DURATION;
    }

    void Update(float deltaTime) {
        if (waveTimer > 0.0f) {
            waveTimer -= deltaTime;
            if (waveTimer < 0.0f) waveTimer = 0.0f;
        }
    }

    void Render(int lives, int score, int comboMult, int hitsCount, bool isCasualMode) {
        shader->Bind();
        glBindVertexArray(VAO);
        glDisable(GL_DEPTH_TEST);

        if (isCasualMode) {
            // === 休闲模式显示 ===

            // === 左上角：击中数和分数 ===
            float leftX = -0.95f, leftY = 0.92f;
            float dotSize = 0.012f, gap = 0.003f;

            // "击中" 文字（用点阵模拟简化的字符）
            // 击
            float hitX = leftX, hitY = leftY;
            drawDigit(0, hitX, hitY, dotSize, gap, 0.3f, 0.7f, 0.9f, 1.0f); // 击字左侧
            drawDigit(7, hitX + 0.08f, hitY, dotSize, gap, 0.3f, 0.7f, 0.9f, 1.0f); // 击字右侧

            // 击中数量（大一点）
            int hits = hitsCount;
            int hitDigits[10];
            int numHitDigits = 0;
            if (hits == 0) { hitDigits[0] = 0; numHitDigits = 1; }
            else { while (hits > 0 && numHitDigits < 10) { hitDigits[numHitDigits++] = hits % 10; hits /= 10; } }
            float hitValX = leftX + 0.18f;
            for (int i = numHitDigits - 1; i >= 0; i--) {
                drawDigit(hitDigits[i], hitValX, hitY, dotSize * 1.5f, gap * 1.5f, 1.0f, 1.0f, 0.3f, 1.0f);
                hitValX += 0.06f;
            }

            // 分数
            float scoreY = leftY - 0.12f;
            drawDigit(0, leftX, scoreY, dotSize, gap, 0.3f, 0.7f, 0.9f, 1.0f); // 分字左侧
            drawDigit(7, leftX + 0.08f, scoreY, dotSize, gap, 0.3f, 0.7f, 0.9f, 1.0f); // 分字右侧

            int s = score;
            int scoreDigits[10];
            int numScoreDigits = 0;
            if (s == 0) { scoreDigits[0] = 0; numScoreDigits = 1; }
            else { while (s > 0 && numScoreDigits < 10) { scoreDigits[numScoreDigits++] = s % 10; s /= 10; } }
            float scoreValX = leftX + 0.18f;
            for (int i = numScoreDigits - 1; i >= 0; i--) {
                drawDigit(scoreDigits[i], scoreValX, scoreY, dotSize * 1.5f, gap * 1.5f, 1.0f, 1.0f, 0.3f, 1.0f);
                scoreValX += 0.06f;
            }

            // === 中间靠上：醒目的 Combo 计分 ===
            float comboY = 0.65f;
            float comboDotSize = 0.03f;
            float comboGap = 0.008f;

            // "COMBO" 标题（用数字模拟）
            drawDigit(7, -0.15f, comboY + 0.12f, comboDotSize * 0.5f, comboGap * 0.5f, 1.0f, 0.6f, 0.1f, 1.0f);
            drawDigit(7, -0.08f, comboY + 0.12f, comboDotSize * 0.5f, comboGap * 0.5f, 1.0f, 0.6f, 0.1f, 1.0f);
            drawDigit(7, -0.01f, comboY + 0.12f, comboDotSize * 0.5f, comboGap * 0.5f, 1.0f, 0.6f, 0.1f, 1.0f);
            drawDigit(7, 0.06f, comboY + 0.12f, comboDotSize * 0.5f, comboGap * 0.5f, 1.0f, 0.6f, 0.1f, 1.0f);

            // Combo 数字（金色，大，发光效果）
            drawDigit(comboMult, -0.1f, comboY, comboDotSize, comboGap, 1.0f, 0.85f, 0.0f, 1.0f);

            // "x" 符号
            drawQuad(0.15f, comboY + 0.02f, 0.01f, 0.04f, 1.0f, 0.85f, 0.0f, 1.0f);

        }
        else {
            // === 战斗模式显示：生命值 ===
            float heartX = -0.95f, heartY = 0.92f, heartW = 0.04f, heartH = 0.06f;
            for (int i = 0; i < 3; i++) {
                if (i < lives)
                    drawQuad(heartX + i * 0.055f, heartY, heartW, heartH, 1.0f, 0.15f, 0.15f, 1.0f);
                else
                    drawQuad(heartX + i * 0.055f, heartY, heartW, heartH, 0.25f, 0.25f, 0.25f, 1.0f);
            }

            // 分数
            float digitStartX = -0.95f, digitStartY = 0.78f;
            float dotSize = 0.015f, gap = 0.004f;

            int s = score;
            int digits[10];
            int numDigits = 0;
            if (s == 0) { digits[0] = 0; numDigits = 1; }
            else { while (s > 0 && numDigits < 10) { digits[numDigits++] = s % 10; s /= 10; } }

            for (int i = numDigits - 1; i >= 0; i--) {
                int col = numDigits - 1 - i;
                drawDigit(digits[i], digitStartX + col * (dotSize * 3 + gap * 2 + 0.02f), digitStartY, dotSize, gap, 1.0f, 1.0f, 1.0f, 1.0f);
            }

            if (comboMult > 1) {
                float cx = digitStartX + numDigits * (dotSize * 3 + gap * 2 + 0.02f) + 0.03f;
                drawQuad(cx, digitStartY, dotSize * 2, dotSize * 5, 1.0f, 0.8f, 0.0f, 1.0f);
                drawDigit(comboMult, cx + 0.03f, digitStartY, dotSize, gap, 1.0f, 0.8f, 0.0f, 1.0f);
            }
        }

        // === WAVE NOTIFICATION: centered, fading text ===
        if (waveTimer > 0.0f) {
            float alpha = waveTimer / WAVE_DURATION;
            // fade in fast, fade out slow
            float fadeAlpha;
            if (alpha > 0.7f) {
                fadeAlpha = (1.0f - alpha) / 0.3f;  // fade in: 0→1
            } else {
                fadeAlpha = alpha / 0.7f;  // fade out: 1→0
            }
            // gold color for impact
            float r = 1.0f, g = 0.75f, bCol = 0.1f;

            float wDotSize = 0.032f, wGap = 0.009f;
            // "WAVE" is 4 letters at 5x5 + 1 digit at 3x5 + 3 gaps + 1 space
            float letterW5 = 5 * (wDotSize + wGap);    // width of a 5x5 letter
            float letterW3 = 3 * (wDotSize + wGap);     // width of a 3x5 digit
            float totalW = letterW5 * 4 + letterW3 + wGap * 4;
            float startX = -totalW / 2.0f;
            float startY = 0.25f;  // slightly above center

            float xOff = 0.0f;
            drawLetter('W', startX + xOff, startY, wDotSize, wGap, r, g, bCol, fadeAlpha);
            xOff += letterW5 + wGap;
            drawLetter('A', startX + xOff, startY, wDotSize, wGap, r, g, bCol, fadeAlpha);
            xOff += letterW5 + wGap;
            drawLetter('V', startX + xOff, startY, wDotSize, wGap, r, g, bCol, fadeAlpha);
            xOff += letterW5 + wGap;
            drawLetter('E', startX + xOff, startY, wDotSize, wGap, r, g, bCol, fadeAlpha);
            xOff += letterW5 + wGap;
            // draw the wave number digit
            drawDigit(waveNum, startX + xOff, startY, wDotSize * 1.5f, wGap, r, g, bCol, fadeAlpha);
        }

        glEnable(GL_DEPTH_TEST);
        glBindVertexArray(0);
        shader->Unbind();
    }
};

const float HUD::WAVE_DURATION = 1.5f;

#endif
