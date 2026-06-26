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
    bool letterG[25] = {
        0,1,1,1,1, 1,0,0,0,0, 1,0,1,1,1, 1,0,0,0,1, 0,1,1,1,0
    };
    bool letterY[25] = {
        1,0,0,0,1, 0,1,0,1,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0
    };
    bool letterT[25] = {
        1,1,1,1,1, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0
    };
    bool letterU[25] = {
        1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 0,1,1,1,0
    };
    bool letterK[25] = {
        1,0,0,0,1, 1,0,0,1,0, 1,1,1,0,0, 1,0,0,1,0, 1,0,0,0,1
    };
    bool letterX[25] = {
        1,0,0,0,1, 0,1,0,1,0, 0,0,1,0,0, 0,1,0,1,0, 1,0,0,0,1
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
        case 'G': pattern = letterG; break;
        case 'Y': pattern = letterY; break;
        case 'T': pattern = letterT; break;
        case 'U': pattern = letterU; break;
        case 'K': pattern = letterK; break;
        case 'X': pattern = letterX; break;
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

    void RenderTutorial() {
        shader->Bind(); glBindVertexArray(VAO); glDisable(GL_DEPTH_TEST);
        drawQuad(-1.0f, 1.0f, 2.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.75f);
        float bs = 0.12f, gap = 0.08f, a = 1.0f, totalW = 4*bs + 3*gap;
        float sx = -totalW/2.0f, sy = 0.25f, nDot = 0.04f, nGap = 0.01f;
        drawQuad(sx, sy, bs, bs, 0.2f, 0.75f, 0.2f, a);
        drawDigit(1, sx+bs/2-0.03f, sy-0.18f, nDot, nGap, 0.3f,0.9f,0.3f,a);
        drawLetter('P', sx+bs/2+0.02f, sy-0.18f, nDot*0.7f, nGap*0.7f, 0.3f,0.9f,0.3f,a);
        sx += bs + gap;
        drawQuad(sx, sy, bs, bs, 0.85f, 0.85f, 0.1f, a);
        drawDigit(2, sx+bs/2-0.03f, sy-0.18f, nDot, nGap, 1,1,0.2f,a);
        drawLetter('P', sx+bs/2+0.02f, sy-0.18f, nDot*0.7f, nGap*0.7f, 1,1,0.2f,a);
        sx += bs + gap;
        drawQuad(sx, sy, bs, bs, 0.9f, 0.2f, 0.1f, a);
        drawDigit(3, sx+bs/2-0.03f, sy-0.18f, nDot, nGap, 1,0.3f,0.2f,a);
        drawLetter('P', sx+bs/2+0.02f, sy-0.18f, nDot*0.7f, nGap*0.7f, 1,0.3f,0.2f,a);
        sx += bs + gap;
        drawQuad(sx, sy, bs, bs, 0.7f, 0.2f, 0.95f, a);
        drawDigit(5, sx+bs/2-0.03f, sy-0.18f, nDot, nGap, 0.8f,0.3f,1,a);
        drawLetter('P', sx+bs/2+0.02f, sy-0.18f, nDot*0.7f, nGap*0.7f, 0.8f,0.3f,1,a);
        float sDot = 0.02f, sGap = 0.005f, ly = -0.15f;
        sx = -totalW/2.0f;
        drawLetter('S', sx+bs*0.1f, ly, sDot, sGap, 0.5f,0.5f,0.5f,a);
        drawLetter('L', sx+bs*0.1f+5*(sDot+sGap), ly, sDot, sGap, 0.5f,0.5f,0.5f,a);
        drawLetter('O', sx+bs*0.1f+10*(sDot+sGap), ly, sDot, sGap, 0.5f,0.5f,0.5f,a);
        drawLetter('W', sx+bs*0.1f+15*(sDot+sGap), ly, sDot, sGap, 0.5f,0.5f,0.5f,a);
        sx += bs + gap; sx += bs + gap;
        drawLetter('F', sx+bs*0.15f, ly, sDot, sGap, 0.5f,0.5f,0.5f,a);
        drawLetter('A', sx+bs*0.15f+5*(sDot+sGap), ly, sDot, sGap, 0.5f,0.5f,0.5f,a);
        drawLetter('S', sx+bs*0.15f+10*(sDot+sGap), ly, sDot, sGap, 0.5f,0.5f,0.5f,a);
        drawLetter('T', sx+bs*0.15f+15*(sDot+sGap), ly, sDot, sGap, 0.5f,0.5f,0.5f,a);
        sx += bs + gap;
        drawLetter('F', sx+bs*0.1f, ly, sDot, sGap, 0.5f,0.5f,0.5f,a);
        drawLetter('A', sx+bs*0.1f+5*(sDot+sGap), ly, sDot, sGap, 0.5f,0.5f,0.5f,a);
        drawLetter('S', sx+bs*0.1f+10*(sDot+sGap), ly, sDot, sGap, 0.5f,0.5f,0.5f,a);
        drawLetter('T', sx+bs*0.1f+15*(sDot+sGap), ly, sDot, sGap, 0.5f,0.5f,0.5f,a);
        ly = -0.35f; sx = -0.15f;
        drawLetter('C', sx, ly, sDot, sGap, 1,0.8f,0,a); sx+=5*(sDot+sGap);
        drawLetter('O', sx, ly, sDot, sGap, 1,0.8f,0,a); sx+=5*(sDot+sGap);
        drawLetter('M', sx, ly, sDot, sGap, 1,0.8f,0,a); sx+=5*(sDot+sGap);
        drawLetter('B', sx, ly, sDot, sGap, 1,0.8f,0,a); sx+=5*(sDot+sGap);
        drawLetter('O', sx, ly, sDot, sGap, 1,0.8f,0,a); sx+=5*(sDot+sGap)+sGap*3;
        drawLetter('X', sx, ly, sDot, sGap, 1,0.8f,0,a); sx+=5*(sDot+sGap)+sGap*2;
        drawDigit(1, sx, ly, sDot*1.5f, sGap, 1,0.8f,0,a); sx+=3*(sDot*1.5f+sGap)+sGap;
        sx+=2*(sDot+sGap)+sGap;
        drawLetter('X', sx, ly, sDot, sGap, 1,0.8f,0,a); sx+=5*(sDot+sGap)+sGap*2;
        drawDigit(5, sx, ly, sDot*1.5f, sGap, 1,0.8f,0,a);
        float bDot = 0.025f, bGap = 0.006f;
        ly = -0.6f; sx = -0.28f;
        drawLetter('C', sx, ly, bDot, bGap, 1,1,1,a); sx+=5*(bDot+bGap);
        drawLetter('L', sx, ly, bDot, bGap, 1,1,1,a); sx+=5*(bDot+bGap);
        drawLetter('I', sx, ly, bDot, bGap, 1,1,1,a); sx+=5*(bDot+bGap);
        drawLetter('C', sx, ly, bDot, bGap, 1,1,1,a); sx+=5*(bDot+bGap);
        drawLetter('K', sx, ly, bDot, bGap, 1,1,1,a); sx+=5*(bDot+bGap)+bGap*2;
        drawLetter('T', sx, ly, bDot, bGap, 1,1,1,a); sx+=5*(bDot+bGap);
        drawLetter('O', sx, ly, bDot, bGap, 1,1,1,a); sx+=5*(bDot+bGap)+bGap*2;
        drawLetter('S', sx, ly, bDot, bGap, 1,1,1,a); sx+=5*(bDot+bGap);
        drawLetter('T', sx, ly, bDot, bGap, 1,1,1,a); sx+=5*(bDot+bGap);
        drawLetter('A', sx, ly, bDot, bGap, 1,1,1,a); sx+=5*(bDot+bGap);
        drawLetter('R', sx, ly, bDot, bGap, 1,1,1,a); sx+=5*(bDot+bGap);
        drawLetter('T', sx, ly, bDot, bGap, 1,1,1,a);
        glEnable(GL_DEPTH_TEST); glBindVertexArray(0); shader->Unbind();
    }

    void Render(int lives, int score, int comboMult, int hitsCount, bool isCasualMode, float rapidTimer = 0.0f, float slowTimer = 0.0f, bool bossActive = false, int bossHP = 0, int bossMaxHP = 0) {
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
            float digitSpacing = dotSize * 3.0f + gap * 2.0f + dotSize * 0.8f;  // digit width + extra gap
            for (int i = numHitDigits - 1; i >= 0; i--) {
                drawDigit(hitDigits[i], hitValX, leftY, dotSize, gap, 1.0f, 1.0f, 0.3f, 1.0f);
                hitValX += digitSpacing;
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
                scoreValX += digitSpacing;
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
                float cx = digitStartX + numDigits * (dotSize * 3 + gap * 2 + 0.02f) + 0.07f;
                drawQuad(cx, digitStartY, dotSize * 1.0f, dotSize * 5, 1.0f, 0.8f, 0.0f, 1.0f);
                drawDigit(comboMult, cx + 0.025f, digitStartY, dotSize, gap, 1.0f, 0.8f, 0.0f, 1.0f);
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

		// === BOSS HEALTH BAR (top center, only when boss active) ===
		if (!isCasualMode && bossActive && bossMaxHP > 0) {
			float barWidth = 0.45f, barHeight = 0.03f;
			float barX = -barWidth / 2.0f, barY = 0.88f;
			// background
			drawQuad(barX, barY, barWidth, barHeight, 0.15f, 0.15f, 0.15f, 0.85f);
			// health fill
			float hpRatio = (float)bossHP / (float)bossMaxHP;
			if (hpRatio < 0.0f) hpRatio = 0.0f;
			if (hpRatio > 1.0f) hpRatio = 1.0f;
			float r = hpRatio > 0.3f ? 1.0f : 1.0f;
			float g = hpRatio > 0.3f ? 0.15f + (1.0f - hpRatio) * 0.3f : 0.55f;
			float b = 0.05f;
			drawQuad(barX, barY, barWidth * hpRatio, barHeight, r, g, b, 1.0f);
			// BOSS label above bar
			float lSize = 0.014f, lGap = 0.002f;
			float lw5 = 5.0f * (lSize + lGap);
			float labelW = lw5 * 4.0f;
			float lx = -labelW / 2.0f, ly = barY + 0.04f;
			drawLetter('B', lx, ly, lSize, lGap, 1.0f, 0.2f, 0.05f, 1.0f); lx += lw5;
			drawLetter('O', lx, ly, lSize, lGap, 1.0f, 0.2f, 0.05f, 1.0f); lx += lw5;
			drawLetter('S', lx, ly, lSize, lGap, 1.0f, 0.2f, 0.05f, 1.0f); lx += lw5;
			drawLetter('S', lx, ly, lSize, lGap, 1.0f, 0.2f, 0.05f, 1.0f);
			// HP text next to bar
			float dSize = 0.013f, dGap = 0.002f;
			int hp = bossHP, hpDigits[10], hpNum = 0;
			if (hp == 0) { hpDigits[0] = 0; hpNum = 1; }
			else { while (hp > 0 && hpNum < 10) { hpDigits[hpNum++] = hp % 10; hp /= 10; } }
			float dpx = barX + barWidth + 0.04f;
			for (int i = hpNum - 1; i >= 0; i--) {
				drawDigit(hpDigits[i], dpx, barY, dSize, dGap, 1.0f, 0.15f, 0.05f, 1.0f);
				dpx += dSize * 3.2f;
			}
		}

	        // === WAVE NOTIFICATION: centered fading digit ===
		        if (waveTimer > 0.0f) {
		            float alpha = waveTimer / WAVE_DURATION;
		            float fadeAlpha;
		            if (alpha > 0.7f) fadeAlpha = (1.0f - alpha) / 0.3f;
		            else fadeAlpha = alpha / 0.7f;
		            float r = 1.0f, g = 0.75f, bCol = 0.1f;
		            float wDotSize = 0.05f, wGap = 0.008f;
		            float wSpacing = wDotSize * 3.5f + wGap;  // wider gap for 2+ digit waves
		            int wn = waveNum, wDigits[10], wNumDigits = 0;
		            if (wn == 0) { wDigits[0] = 0; wNumDigits = 1; }
		            else { while (wn > 0 && wNumDigits < 10) { wDigits[wNumDigits++] = wn % 10; wn /= 10; } }
		            float numWidth = wNumDigits * wSpacing;
		            float totalW = numWidth;
		            float startX = -totalW / 2.0f, wY = 0.22f, wx = startX;
		            for (int i = wNumDigits - 1; i >= 0; i--) {
		                drawDigit(wDigits[i], wx, wY, wDotSize, wGap, r, g, bCol, fadeAlpha);
		                wx += wSpacing;
		            }
		            float lineW = totalW + 0.12f, lineH = 0.006f, lineX = -lineW / 2.0f;
		            drawQuad(lineX, wY + 0.06f, lineW, lineH, r, g, bCol, fadeAlpha);
		            drawQuad(lineX, wY - 5.0f * wDotSize - 0.06f, lineW, lineH, r, g, bCol, fadeAlpha);
		        }
glEnable(GL_DEPTH_TEST);
        glBindVertexArray(0);
        shader->Unbind();
    }
};

const float HUD::WAVE_DURATION = 0.7f;

#endif
