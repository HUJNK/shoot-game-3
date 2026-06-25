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
    bool letterR[25] = {
        1,1,1,1,1, 1,0,0,0,1, 1,1,1,1,1, 1,0,0,0,1, 1,0,0,0,1
    };
    bool letterP[25] = {
        1,1,1,1,1, 1,0,0,0,1, 1,1,1,1,1, 1,0,0,0,0, 1,0,0,0,0
    };
    bool letterI[25] = {
        1,1,1,1,1, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0, 1,1,1,1,1
    };
    bool letterD[25] = {
        1,1,1,1,0, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 1,1,1,1,0
    };
    bool letterF[25] = {
        1,1,1,1,1, 1,0,0,0,0, 1,1,1,1,0, 1,0,0,0,0, 1,0,0,0,0
    };
    bool letterS[25] = {
        0,1,1,1,1, 1,0,0,0,0, 0,1,1,1,0, 0,0,0,0,1, 1,1,1,1,0
    };
    bool letterL[25] = {
        1,0,0,0,0, 1,0,0,0,0, 1,0,0,0,0, 1,0,0,0,0, 1,1,1,1,1
    };
    bool letterO[25] = {
        0,1,1,1,0, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 0,1,1,1,0
    };
    bool letterM[25] = {
        1,0,0,0,1, 1,1,0,1,1, 1,0,1,0,1, 1,0,0,0,1, 1,0,0,0,1
    };
    bool letterC[25] = {
        0,1,1,1,0, 1,0,0,0,1, 1,0,0,0,0, 1,0,0,0,1, 0,1,1,1,0
    };
    bool letterB[25] = {
        1,1,1,1,0, 1,0,0,0,1, 1,1,1,1,0, 1,0,0,0,1, 1,1,1,1,0
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
        case 'R': pattern = letterR; break;
        case 'P': pattern = letterP; break;
        case 'I': pattern = letterI; break;
        case 'D': pattern = letterD; break;
        case 'F': pattern = letterF; break;
        case 'S': pattern = letterS; break;
        case 'L': pattern = letterL; break;
        case 'O': pattern = letterO; break;
        case 'M': pattern = letterM; break;
        case 'C': pattern = letterC; break;
        case 'B': pattern = letterB; break;
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

    void Render(int lives, int score, int comboMult, int hitsCount, bool isCasualMode, float rapidTimer = 0.0f, float slowTimer = 0.0f) {
        shader->Bind();
        glBindVertexArray(VAO);
        glDisable(GL_DEPTH_TEST);

        if (isCasualMode) {
            // === Casual Mode HUD ===
            float leftX = -0.95f, leftY = 0.92f;
            float dotSize = 0.02f, gap = 0.005f;

            // Hits (top row, yellow)
            int hits = hitsCount;
            int hitDigits[10];
            int numHitDigits = 0;
            if (hits == 0) { hitDigits[0] = 0; numHitDigits = 1; }
            else { while (hits > 0 && numHitDigits < 10) { hitDigits[numHitDigits++] = hits % 10; hits /= 10; } }
            float hitValX = leftX;
            for (int i = numHitDigits - 1; i >= 0; i--) {
                drawDigit(hitDigits[i], hitValX, leftY, dotSize, gap, 1.0f, 1.0f, 0.3f, 1.0f);
                hitValX += dotSize * 3.5f;
            }

            // Score (bottom row, white)
            float scoreY = leftY - 0.22f;
            int s = score;
            int scoreDigits[10];
            int numScoreDigits = 0;
            if (s == 0) { scoreDigits[0] = 0; numScoreDigits = 1; }
            else { while (s > 0 && numScoreDigits < 10) { scoreDigits[numScoreDigits++] = s % 10; s /= 10; } }
            float scoreValX = leftX;
            for (int i = numScoreDigits - 1; i >= 0; i--) {
                drawDigit(scoreDigits[i], scoreValX, scoreY, dotSize, gap, 1.0f, 1.0f, 1.0f, 1.0f);
                scoreValX += dotSize * 3.5f;
            }

            // === Center: Combo (only when active) ===
            if (comboMult > 1) {
                float comboY = 0.8f;
                float comboDotSize = 0.05f;
                float comboGap = 0.006f;

                // Multiplier number (centered)
                int cm = comboMult;
                int cmDigits[10];
                int numCmDigits = 0;
                if (cm == 0) { cmDigits[0] = 0; numCmDigits = 1; }
                else { while (cm > 0 && numCmDigits < 10) { cmDigits[numCmDigits++] = cm % 10; cm /= 10; } }
                float numWidth = numCmDigits * (comboDotSize * 3.0f + comboGap * 2.0f) - comboGap;
                float numStartX = -numWidth / 2.0f;
                float cmX = numStartX;
                for (int i = numCmDigits - 1; i >= 0; i--) {
                    drawDigit(cmDigits[i], cmX, comboY, comboDotSize, comboGap, 1.0f, 0.85f, 0.0f, 1.0f);
                    cmX += comboDotSize * 3.0f + comboGap;
                }

                // "COMBO" label centered above
                float lblSize = 0.018f, lblGap = 0.003f;
                float letterW = 5.0f * (lblSize + lblGap);
                float lblTotalW = letterW * 5.0f + lblGap * 4.0f;
                float lblStartX = -lblTotalW / 2.0f;
                float lblY = comboY + 0.16f;
                float lx = lblStartX;
                float step = letterW + lblGap;
                drawLetter('C', lx, lblY, lblSize, lblGap, 1.0f, 0.7f, 0.15f, 1.0f); lx += step;
                drawLetter('O', lx, lblY, lblSize, lblGap, 1.0f, 0.7f, 0.15f, 1.0f); lx += step;
                drawLetter('M', lx, lblY, lblSize, lblGap, 1.0f, 0.7f, 0.15f, 1.0f); lx += step;
                drawLetter('B', lx, lblY, lblSize, lblGap, 1.0f, 0.7f, 0.15f, 1.0f); lx += step;
                drawLetter('O', lx, lblY, lblSize, lblGap, 1.0f, 0.7f, 0.15f, 1.0f);
            }

        }
        else {
            // === Challenge Mode: Lives ===
            float heartX = -0.95f, heartY = 0.92f, heartW = 0.04f, heartH = 0.06f;
            for (int i = 0; i < 3; i++) {
                if (i < lives)
                    drawQuad(heartX + i * 0.055f, heartY, heartW, heartH, 1.0f, 0.15f, 0.15f, 1.0f);
                else
                    drawQuad(heartX + i * 0.055f, heartY, heartW, heartH, 0.25f, 0.25f, 0.25f, 1.0f);
            }

            // Score
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

        // === EFFECT INDICATOR ===
        if (rapidTimer > 0.0f) {
            float fxDot = 0.012f, fxGap = 0.003f, fxY = 0.68f, fxX = -0.95f, a = 1.0f;
            drawLetter('R', fxX, fxY, fxDot, fxGap, 1.0f, 0.55f, 0.0f, a); fxX += 5*(fxDot+fxGap)+fxGap;
            drawLetter('A', fxX, fxY, fxDot, fxGap, 1.0f, 0.55f, 0.0f, a); fxX += 5*(fxDot+fxGap)+fxGap;
            drawLetter('P', fxX, fxY, fxDot, fxGap, 1.0f, 0.55f, 0.0f, a); fxX += 5*(fxDot+fxGap)+fxGap;
            drawLetter('I', fxX, fxY, fxDot, fxGap, 1.0f, 0.55f, 0.0f, a); fxX += 5*(fxDot+fxGap)+fxGap;
            drawLetter('D', fxX, fxY, fxDot, fxGap, 1.0f, 0.55f, 0.0f, a);
        }
        if (slowTimer > 0.0f) {
            float fxDot = 0.012f, fxGap = 0.003f, fxY = 0.62f, fxX = -0.95f, a = 1.0f;
            drawLetter('S', fxX, fxY, fxDot, fxGap, 0.3f, 0.6f, 1.0f, a); fxX += 5*(fxDot+fxGap)+fxGap;
            drawLetter('L', fxX, fxY, fxDot, fxGap, 0.3f, 0.6f, 1.0f, a); fxX += 5*(fxDot+fxGap)+fxGap;
            drawLetter('O', fxX, fxY, fxDot, fxGap, 0.3f, 0.6f, 1.0f, a); fxX += 5*(fxDot+fxGap)+fxGap;
            drawLetter('W', fxX, fxY, fxDot, fxGap, 0.3f, 0.6f, 1.0f, a);
        }

	        // === WAVE NOTIFICATION: centered fading number ===
	        if (waveTimer > 0.0f) {
	            float alpha = waveTimer / WAVE_DURATION;
	            float fadeAlpha;
	            if (alpha > 0.7f) {
	                fadeAlpha = (1.0f - alpha) / 0.3f;
	            } else {
	                fadeAlpha = alpha / 0.7f;
	            }
	            float r = 1.0f, g = 0.75f, bCol = 0.1f;

	            // Large centered wave number
	            float wDotSize = 0.04f, wGap = 0.008f;
	            int wn = waveNum;
	            int wDigits[10];
	            int wNumDigits = 0;
	            if (wn == 0) { wDigits[0] = 0; wNumDigits = 1; }
	            else { while (wn > 0 && wNumDigits < 10) { wDigits[wNumDigits++] = wn % 10; wn /= 10; } }
	            float wTotalW = wNumDigits * (wDotSize * 3.0f + wGap * 2.0f) - wGap;
	            float wStartX = -wTotalW / 2.0f;
	            float wY = 0.20f;
	            float wx = wStartX;
	            for (int i = wNumDigits - 1; i >= 0; i--) {
	                drawDigit(wDigits[i], wx, wY, wDotSize, wGap, r, g, bCol, fadeAlpha);
	                wx += wDotSize * 3.0f + wGap;
	            }

	            // Decorative lines above and below
	            float lineW = wTotalW + 0.12f;
	            float lineH = 0.006f;
	            float lineX = -lineW / 2.0f;
	            drawQuad(lineX, wY + 0.05f, lineW, lineH, r, g, bCol, fadeAlpha);
	            drawQuad(lineX, wY - (5.0f * wDotSize + 4.0f * wGap) - 0.05f, lineW, lineH, r, g, bCol, fadeAlpha);
	        }

        glEnable(GL_DEPTH_TEST);
        glBindVertexArray(0);
        shader->Unbind();
    }
};

const float HUD::WAVE_DURATION = 0.7f;

#endif
