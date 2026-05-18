#define GL_SILENCE_DEPRECATION
#define GLUT_SILENCE_DEPRECATION

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#include <iostream>
#include <deque>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <algorithm>

using namespace std;

// ==================================================
// ================== GLOBALS =======================
// ==================================================
enum GameState { MENU, SNAKE, CAR, SPACE };
GameState currentState = MENU;

const int WIDTH  = 800;
const int HEIGHT = 600;
float animTime   = 0.0f;

// ==================================================
// ================== TEXT ==========================
// ==================================================
void drawText(const string& t, int x, int y) {
    glRasterPos2i(x, y);
    for (char c : t) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
}
void drawTextLarge(const string& t, int x, int y) {
    glRasterPos2i(x, y);
    for (char c : t) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
}
void drawTextSmall(const string& t, int x, int y) {
    glRasterPos2i(x, y);
    for (char c : t) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
}

// ==================================================
// ================== SNAKE =========================
// ==================================================
struct GridPt { int x; int y; };

deque<GridPt> snake;
GridPt        food;
int   snakeDir       = 2;
int   snakeScore     = 0;
int   highScore      = 0;
int   snakeTickAccum = 0;
bool  snakeOver      = false;

const int CELL   = 20;
const int GRID_X = 100;
const int GRID_Y = 100;
const int GRID_W = 30;
const int GRID_H = 20;

int snakeInterval() { return max(80, 220 - (snakeScore / 50) * 15); }

GridPt makeGridPt(int x, int y) {
    GridPt p; p.x = x; p.y = y; return p;
}
GridPt randomFood() {
    return makeGridPt((int)(rand() % GRID_W), (int)(rand() % GRID_H));
}

void initSnake() {
    snake.clear();
    snake.push_back(makeGridPt(10, 10));
    snake.push_back(makeGridPt(9,  10));
    snake.push_back(makeGridPt(8,  10));
    snakeDir       = 2;
    snakeScore     = 0;
    snakeOver      = false;
    snakeTickAccum = 0;
    food           = randomFood();
}

void drawCircle(float cx, float cy, float r, int segs = 24) {
    glBegin(GL_POLYGON);
    for (int i = 0; i < segs; i++) {
        float a = (float)(i * M_PI * 2.0 / segs);
        glVertex2f(cx + r * cosf(a), cy + r * sinf(a));
    }
    glEnd();
}

void drawSnakeGame() {
    glClearColor(0.02f, 0.06f, 0.03f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    for (int gx = 0; gx <= GRID_W; gx++)
        for (int gy = 0; gy <= GRID_H; gy++) {
            float p = 0.5f + 0.5f * sinf(animTime*0.4f + gx*0.3f + gy*0.3f);
            glColor4f(0.2f, 0.9f, 0.3f, 0.07f + 0.04f*p);
            glPointSize(2.0f);
            glBegin(GL_POINTS);
            glVertex2f((float)(GRID_X + gx*CELL), (float)(GRID_Y + gy*CELL));
            glEnd();
        }

    for (int l = GRID_Y; l < GRID_Y + GRID_H*CELL; l += 4) {
        glColor4f(0.0f, 0.0f, 0.0f, 0.17f);
        glBegin(GL_QUADS);
        glVertex2f((float)GRID_X,               (float)l);
        glVertex2f((float)(GRID_X+GRID_W*CELL), (float)l);
        glVertex2f((float)(GRID_X+GRID_W*CELL), (float)(l+2));
        glVertex2f((float)GRID_X,               (float)(l+2));
        glEnd();
    }

    float gw = 0.5f + 0.5f * sinf(animTime * 1.5f);
    for (int lay = 8; lay >= 1; lay--) {
        glColor4f(0.1f, 1.0f, 0.3f, (0.04f + 0.03f*gw) * (9-lay) / 8.0f);
        glLineWidth((float)(lay * 2));
        glBegin(GL_LINE_LOOP);
        glVertex2i(GRID_X,             GRID_Y);
        glVertex2i(GRID_X+GRID_W*CELL, GRID_Y);
        glVertex2i(GRID_X+GRID_W*CELL, GRID_Y+GRID_H*CELL);
        glVertex2i(GRID_X,             GRID_Y+GRID_H*CELL);
        glEnd();
    }
    glColor3f(0.15f, 1.0f, 0.35f); glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2i(GRID_X,             GRID_Y);
    glVertex2i(GRID_X+GRID_W*CELL, GRID_Y);
    glVertex2i(GRID_X+GRID_W*CELL, GRID_Y+GRID_H*CELL);
    glVertex2i(GRID_X,             GRID_Y+GRID_H*CELL);
    glEnd();

    const int cs = 8;
    glColor3f(0.1f, 0.95f, 0.3f);
    glRectf((float)(GRID_X-1),              (float)(GRID_Y-1),
            (float)(GRID_X+cs),             (float)(GRID_Y+cs));
    glRectf((float)(GRID_X+GRID_W*CELL-cs), (float)(GRID_Y-1),
            (float)(GRID_X+GRID_W*CELL+1),  (float)(GRID_Y+cs));
    glRectf((float)(GRID_X-1),              (float)(GRID_Y+GRID_H*CELL-cs),
            (float)(GRID_X+cs),             (float)(GRID_Y+GRID_H*CELL+1));
    glRectf((float)(GRID_X+GRID_W*CELL-cs), (float)(GRID_Y+GRID_H*CELL-cs),
            (float)(GRID_X+GRID_W*CELL+1),  (float)(GRID_Y+GRID_H*CELL+1));

    if (snakeOver) {
        glColor4f(0.0f, 0.0f, 0.0f, 0.65f);
        glRectf((float)GRID_X,            (float)GRID_Y,
                (float)(GRID_X+GRID_W*CELL), (float)(GRID_Y+GRID_H*CELL));
        float g = 0.7f + 0.3f * sinf(animTime * 2.0f);
        glColor3f(g, 0.1f, 0.1f);
        drawTextLarge("GAME OVER", 300, 275);
        glColor3f(0.9f, 0.9f, 0.9f);
        drawText("Press SPACE to restart", 283, 310);
        glColor3f(0.8f, 0.8f, 0.1f);
        drawText("High Score: " + to_string(highScore), 340, 345);
        glColor3f(0.7f, 0.9f, 0.7f);
        drawText("Score: " + to_string(snakeScore), 120, 82);
        return;
    }

    float fp = 0.6f + 0.4f * sinf(animTime * 3.0f);
    float fx  = (float)(GRID_X + food.x*CELL + CELL/2);
    float fy  = (float)(GRID_Y + food.y*CELL + CELL/2);
    for (int l = 6; l >= 1; l--) {
        glColor4f(1.0f, 0.1f, 0.1f, 0.08f * fp * (7-l) / 6.0f);
        drawCircle(fx, fy, (float)(CELL/2 + l*3));
    }
    glColor3f(1.0f, 0.15f + 0.1f*fp, 0.05f);
    drawCircle(fx, fy, (float)(CELL/2 - 2));
    glColor4f(1.0f, 0.8f, 0.8f, 0.7f);
    drawCircle(fx - 3.0f, fy - 3.0f, 3.0f, 12);

    int slen = (int)snake.size();
    for (int i = slen-1; i >= 0; i--) {
        float sx = (float)(GRID_X + snake[i].x * CELL);
        float sy = (float)(GRID_Y + snake[i].y * CELL);
        float t  = (float)i / (float)max(slen-1, 1);
        if (i == 0) {
            float hp = 0.8f + 0.2f * sinf(animTime * 2.0f);
            glColor3f(0.95f*hp, 1.0f*hp, 0.0f);
        } else {
            glColor3f(0.0f, 0.7f - 0.4f*t, 0.3f - 0.2f*t);
        }
        glRectf(sx+2.0f, sy+2.0f, sx+(float)CELL-2.0f, sy+(float)CELL-2.0f);
        if (i < slen-2) {
            glColor4f(1.0f, 1.0f, 1.0f, 0.12f);
            glRectf(sx+2.0f, sy+2.0f, sx+(float)(CELL/2), sy+6.0f);
        }
    }

    if (!snake.empty()) {
        float hx = (float)(GRID_X + snake[0].x * CELL);
        float hy = (float)(GRID_Y + snake[0].y * CELL);
        glColor3f(0.0f, 0.0f, 0.0f);
        float ex1x, ex1y, ex2x, ex2y;
        if      (snakeDir==2){ ex1x=hx+14; ex1y=hy+5;  ex2x=hx+14; ex2y=hy+13; }
        else if (snakeDir==4){ ex1x=hx+4;  ex1y=hy+5;  ex2x=hx+4;  ex2y=hy+13; }
        else if (snakeDir==1){ ex1x=hx+5;  ex1y=hy+5;  ex2x=hx+13; ex2y=hy+5;  }
        else                 { ex1x=hx+5;  ex1y=hy+13; ex2x=hx+13; ex2y=hy+13; }
        glRectf(ex1x, ex1y, ex1x+3.0f, ex1y+3.0f);
        glRectf(ex2x, ex2y, ex2x+3.0f, ex2y+3.0f);
    }

    glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
    glRectf(100.0f, 60.0f, 700.0f, 90.0f);
    glColor3f(0.1f, 1.0f, 0.3f); glLineWidth(1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2i(100,60); glVertex2i(700,60);
    glVertex2i(700,90); glVertex2i(100,90);
    glEnd();
    glColor3f(0.8f, 1.0f, 0.8f);
    drawText("SCORE: " + to_string(snakeScore), 120, 82);
    glColor3f(0.8f, 0.7f, 0.0f);
    drawText("BEST: " + to_string(highScore), 540, 82);
    int spd = max(1, (220 - snakeInterval()) / 15 + 1);
    glColor3f(0.4f, 0.8f, 0.4f);
    drawTextSmall("SPEED: " + to_string(spd), 325, 82);
    glColor3f(0.3f, 0.5f, 0.35f);
    drawText("Arrow keys  |  ESC: Menu", 285, 555);
}

// ==================================================
// ================== CAR GAME ======================
// ==================================================
const float ROAD_L   = 175.0f;
const float ROAD_R   = 625.0f;
const float ROAD_MID = (ROAD_L + ROAD_R) / 2.0f;
const float LANE_W   = (ROAD_R - ROAD_L) / 3.0f;

float laneX[3] = {
    ROAD_L + LANE_W * 0.5f,
    ROAD_L + LANE_W * 1.5f,
    ROAD_L + LANE_W * 2.5f
};

int   playerLane  = 1;
bool  carOver     = false;
int   carScore    = 0;
int   lives       = 3;
float carSpeed    = 6.0f;
float roadOffset  = 0.0f;
float objOffset   = 0.0f;
float signalTimer = 0.0f;
int   signalState = 0;

struct EnemyCar { float y; int lane; bool active; float cr, cg, cb; };
EnemyCar enemies[4];

struct RoadObj { float y; int side, type; };
vector<RoadObj> roadObjs;

void initCar() {
    playerLane  = 1;
    carOver     = false;
    carScore    = 0;
    lives       = 3;
    carSpeed    = 6.0f;
    roadOffset  = 0.0f;
    objOffset   = 0.0f;
    signalTimer = 0.0f;
    signalState = 0;

    for (int i = 0; i < 4; i++) {
        enemies[i].y      = -(rand() % 400 + 50);
        enemies[i].lane   = rand() % 3;
        enemies[i].active = (i == 0);
        enemies[i].cr     = (rand()%100)/100.0f;
        enemies[i].cg     = (rand()%100)/100.0f;
        enemies[i].cb     = (rand()%100)/100.0f;
    }
    roadObjs.clear();
    for (int i = 0; i < 14; i++) {
        RoadObj o;
        o.y    = (float)(rand() % 600);
        o.side = rand() % 2;
        o.type = rand() % 3;
        roadObjs.push_back(o);
    }
}

void drawTree(float x, float y) {
    glColor3f(0.42f, 0.25f, 0.07f);
    glRectf(x-5.0f, y-28.0f, x+5.0f, y+12.0f);
    glColor3f(0.07f, 0.45f, 0.10f); drawCircle(x, y-8.0f,  22.0f);
    glColor3f(0.10f, 0.55f, 0.15f); drawCircle(x, y-20.0f, 16.0f);
    glColor3f(0.15f, 0.65f, 0.20f); drawCircle(x, y-31.0f, 10.0f);
    glColor4f(0.0f, 0.0f, 0.0f, 0.13f);
    glBegin(GL_POLYGON);
    for (int i=0;i<16;i++){
        float a=i*(float)M_PI*2.0f/16.0f;
        glVertex2f(x+18.0f*cosf(a), y+10.0f+5.0f*sinf(a));
    }
    glEnd();
}

void drawLamp(float x, float y) {
    glColor3f(0.58f, 0.58f, 0.62f); glLineWidth(3.0f);
    glBegin(GL_LINES); glVertex2f(x, y); glVertex2f(x, y-58.0f); glEnd();
    glBegin(GL_LINES); glVertex2f(x, y-56.0f); glVertex2f(x+14.0f, y-61.0f); glEnd();
    glColor3f(0.48f, 0.48f, 0.52f);
    glRectf(x+9.0f, y-67.0f, x+21.0f, y-61.0f);
    float lp = 0.7f + 0.3f * sinf(animTime * 2.1f + x * 0.07f);
    glColor4f(1.0f, 0.95f, 0.55f, 0.22f*lp);
    drawCircle(x+15.0f, y-64.0f, 15.0f);
    glColor3f(1.0f, 0.95f, 0.50f);
    drawCircle(x+15.0f, y-64.0f,  5.0f);
    glColor4f(1.0f, 0.95f, 0.35f, 0.07f*lp);
    glBegin(GL_POLYGON);
    for (int i=0;i<16;i++){
        float a=i*(float)M_PI*2.0f/16.0f;
        glVertex2f(x+15.0f+22.0f*cosf(a), y+5.0f*sinf(a));
    }
    glEnd();
}

void drawSignal(float x, float y) {
    glColor3f(0.52f, 0.52f, 0.57f); glLineWidth(3.0f);
    glBegin(GL_LINES); glVertex2f(x, y); glVertex2f(x, y-82.0f); glEnd();
    glColor3f(0.13f, 0.13f, 0.13f);
    glRectf(x-11.0f, y-82.0f, x+11.0f, y-30.0f);
    glColor3f(0.40f, 0.40f, 0.40f); glLineWidth(1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x-11.0f, y-82.0f); glVertex2f(x+11.0f, y-82.0f);
    glVertex2f(x+11.0f, y-30.0f); glVertex2f(x-11.0f, y-30.0f);
    glEnd();
    float rp = (signalState==2) ? 1.0f : 0.25f;
    float yp = (signalState==1) ? 1.0f : 0.25f;
    float gp = (signalState==0) ? 1.0f : 0.25f;
    glColor3f(rp,       0.1f,      0.1f); drawCircle(x, y-72.0f, 7.0f);
    glColor3f(yp, yp*0.8f,         0.1f); drawCircle(x, y-56.0f, 7.0f);
    glColor3f(0.1f,       gp,      0.1f); drawCircle(x, y-40.0f, 7.0f);
    if      (signalState==0){ glColor4f(0,1,0,0.3f); drawCircle(x, y-40.0f, 12.0f); }
    else if (signalState==1){ glColor4f(1,1,0,0.3f); drawCircle(x, y-56.0f, 12.0f); }
    else                    { glColor4f(1,0,0,0.3f); drawCircle(x, y-72.0f, 12.0f); }
}

void drawRoadside() {
    for (auto& o : roadObjs) {
        float ry = fmodf(o.y + objOffset, 720.0f) - 60.0f;
        float rx = (o.side == 0) ? ROAD_L - 52.0f : ROAD_R + 52.0f;
        if      (o.type == 0) drawTree(rx, ry);
        else if (o.type == 1) drawLamp(rx + (o.side ? 0.0f : -22.0f), ry + 28.0f);
        else                  drawSignal(rx + (o.side ? -10.0f : 10.0f), ry + 82.0f);
    }
}

void drawBeautifulCar(float x, float y, float cr, float cg, float cb, bool player) {
    glColor4f(0.0f, 0.0f, 0.0f, 0.17f);
    glBegin(GL_POLYGON);
    for (int i=0;i<16;i++){
        float a=i*(float)M_PI*2.0f/16.0f;
        glVertex2f(x+22.0f*cosf(a), y+87.0f+4.0f*sinf(a));
    }
    glEnd();
    glColor3f(cr, cg, cb);
    glRectf(x-22.0f, y+10.0f, x+22.0f, y+75.0f);
    glColor3f(cr*0.72f, cg*0.72f, cb*0.72f);
    glBegin(GL_POLYGON);
    glVertex2f(x-13.0f, y+30.0f); glVertex2f(x+13.0f, y+30.0f);
    glVertex2f(x+10.0f, y+11.0f); glVertex2f(x-10.0f, y+11.0f);
    glEnd();
    glColor4f(0.45f, 0.80f, 1.0f, 0.70f);
    glBegin(GL_POLYGON);
    glVertex2f(x-9.0f, y+29.0f); glVertex2f(x+9.0f, y+29.0f);
    glVertex2f(x+7.0f, y+14.0f); glVertex2f(x-7.0f, y+14.0f);
    glEnd();
    glColor4f(0.45f, 0.80f, 1.0f, 0.45f);
    glBegin(GL_POLYGON);
    glVertex2f(x-9.0f, y+33.0f); glVertex2f(x+9.0f, y+33.0f);
    glVertex2f(x+7.0f, y+47.0f); glVertex2f(x-7.0f, y+47.0f);
    glEnd();
    glColor4f(1.0f, 1.0f, 1.0f, 0.11f);
    glRectf(x-18.0f, y+50.0f, x-6.0f, y+72.0f);
    glColor3f(cr*0.83f, cg*0.83f, cb*0.83f);
    glRectf(x-18.0f, y+60.0f, x+18.0f, y+75.0f);
    glColor3f(0.68f, 0.68f, 0.72f);
    glRectf(x-22.0f, y+73.0f, x+22.0f, y+80.0f);
    glRectf(x-22.0f, y+5.0f,  x+22.0f, y+11.0f);
    if (player) {
        float lp = 0.8f + 0.2f * sinf(animTime * 3.0f);
        glColor3f(1.0f, 1.0f, 0.82f*lp);
        glRectf(x-20.0f, y+73.0f, x-11.0f, y+79.0f);
        glRectf(x+11.0f, y+73.0f, x+20.0f, y+79.0f);
        glColor4f(1.0f, 1.0f, 0.6f, 0.07f*lp);
        glBegin(GL_POLYGON);
        glVertex2f(x-20.0f,y+79.0f); glVertex2f(x-11.0f,y+79.0f);
        glVertex2f(x+4.0f, y+120.0f); glVertex2f(x-35.0f,y+120.0f);
        glEnd();
        glBegin(GL_POLYGON);
        glVertex2f(x+11.0f,y+79.0f); glVertex2f(x+20.0f,y+79.0f);
        glVertex2f(x+35.0f,y+120.0f); glVertex2f(x-4.0f, y+120.0f);
        glEnd();
    } else {
        glColor3f(1.0f, 0.25f, 0.15f);
        glRectf(x-20.0f, y+73.0f, x-11.0f, y+79.0f);
        glRectf(x+11.0f, y+73.0f, x+20.0f, y+79.0f);
    }
    glColor3f(0.88f, 0.1f, 0.1f);
    glRectf(x-20.0f, y+5.0f, x-12.0f, y+10.0f);
    glRectf(x+12.0f, y+5.0f, x+20.0f, y+10.0f);
    glColor3f(0.25f, 0.25f, 0.27f);
    glRectf(x-14.0f, y+75.0f, x+14.0f, y+78.0f);
    glColor3f(0.40f, 0.40f, 0.42f); glLineWidth(1.0f);
    for (int i=-10; i<=10; i+=5) {
        glBegin(GL_LINES);
        glVertex2f(x+(float)i, y+75.0f);
        glVertex2f(x+(float)i, y+78.0f);
        glEnd();
    }
    glColor3f(0.08f, 0.08f, 0.08f);
    glRectf(x-28.0f, y+13.0f, x-22.0f, y+30.0f);
    glRectf(x+22.0f, y+13.0f, x+28.0f, y+30.0f);
    glRectf(x-28.0f, y+55.0f, x-22.0f, y+73.0f);
    glRectf(x+22.0f, y+55.0f, x+28.0f, y+73.0f);
    glColor3f(0.75f, 0.75f, 0.78f);
    drawCircle(x-25.0f, y+21.0f, 4.0f);
    drawCircle(x+25.0f, y+21.0f, 4.0f);
    drawCircle(x-25.0f, y+64.0f, 4.0f);
    drawCircle(x+25.0f, y+64.0f, 4.0f);
    glColor3f(0.60f, 0.60f, 0.65f); glLineWidth(1.0f);
    float hubs[4][2] = {
        {x-25.0f,y+21.0f},{x+25.0f,y+21.0f},
        {x-25.0f,y+64.0f},{x+25.0f,y+64.0f}
    };
    for (int w=0; w<4; w++)
        for (int k=0; k<4; k++) {
            float a = animTime*3.0f + (float)k*(float)M_PI/2.0f;
            glBegin(GL_LINES);
            glVertex2f(hubs[w][0], hubs[w][1]);
            glVertex2f(hubs[w][0]+4.0f*cosf(a), hubs[w][1]+4.0f*sinf(a));
            glEnd();
        }
    glColor3f(cr*0.8f, cg*0.8f, cb*0.8f);
    glRectf(x-26.0f, y+25.0f, x-22.0f, y+30.0f);
    glRectf(x+22.0f, y+25.0f, x+26.0f, y+30.0f);
}

void drawRoad() {
    glColor3f(0.28f, 0.52f, 0.18f);
    glRectf(0.0f, 0.0f, (float)WIDTH, (float)HEIGHT);
    for (int i=0; i<HEIGHT; i+=14) {
        float g = 0.26f + 0.07f * ((i/14) % 2);
        glColor3f(g*0.75f, g, g*0.35f);
        glRectf(0.0f,   (float)i, ROAD_L,      (float)(i+7));
        glRectf(ROAD_R, (float)i, (float)WIDTH, (float)(i+7));
    }
    glColor3f(0.20f, 0.20f, 0.22f);
    glRectf(ROAD_L, 0.0f, ROAD_R, (float)HEIGHT);
    for (int i=0; i<HEIGHT; i+=50) {
        glColor4f(0.0f, 0.0f, 0.0f, 0.05f);
        glRectf(ROAD_L+8.0f, (float)i, ROAD_R-8.0f, (float)(i+25));
    }
    for (int i=0; i<HEIGHT; i+=30) {
        float kc = ((i/30) % 2) ? 0.85f : 0.20f;
        glColor3f(kc, kc, kc);
        glRectf(ROAD_L,      (float)i, ROAD_L+9.0f, (float)(i+15));
        glRectf(ROAD_R-9.0f, (float)i, ROAD_R,      (float)(i+15));
    }
    glColor3f(0.92f, 0.88f, 0.28f); glLineWidth(2.5f);
    float period = 46.0f;
    for (int lane=1; lane<=2; lane++) {
        float lx    = ROAD_L + (float)lane * LANE_W;
        float start = fmodf(roadOffset, period);
        for (float dy = start - period; dy < (float)HEIGHT + period; dy += period) {
            glBegin(GL_LINES);
            glVertex2f(lx, dy);
            glVertex2f(lx, dy + 28.0f);
            glEnd();
        }
    }
    glColor3f(0.98f, 0.88f, 0.08f); glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(ROAD_MID-2.0f, 0.0f); glVertex2f(ROAD_MID-2.0f, (float)HEIGHT);
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(ROAD_MID+2.0f, 0.0f); glVertex2f(ROAD_MID+2.0f, (float)HEIGHT);
    glEnd();
}

void drawCarGame() {
    drawRoad();
    drawRoadside();

    if (carOver) {
        glColor4f(0.0f, 0.0f, 0.0f, 0.62f);
        glRectf(0.0f, 0.0f, (float)WIDTH, (float)HEIGHT);
        float g = 0.7f + 0.3f * sinf(animTime * 2.0f);
        glColor3f(g, 0.15f, 0.15f);
        drawTextLarge("GAME OVER", 290, 268);
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText("Final Score: " + to_string(carScore), 310, 303);
        drawText("Press SPACE to restart", 280, 333);
        return;
    }

    drawBeautifulCar(laneX[playerLane], 468.0f, 0.1f, 0.45f, 0.95f, true);
    for (int i=0; i<4; i++)
        if (enemies[i].active)
            drawBeautifulCar(laneX[enemies[i].lane], enemies[i].y,
                             enemies[i].cr, enemies[i].cg, enemies[i].cb, false);

    glColor4f(0.0f, 0.0f, 0.0f, 0.55f);
    glRectf(0.0f, 0.0f, (float)WIDTH, 44.0f);
    glColor3f(0.2f, 0.6f, 1.0f); glLineWidth(1.0f);
    glBegin(GL_LINES);
    glVertex2f(0.0f, 44.0f); glVertex2f((float)WIDTH, 44.0f);
    glEnd();
    glColor3f(0.9f, 0.9f, 1.0f);
    drawText("SCORE: " + to_string(carScore), 20, 30);
    for (int i=0; i<lives; i++) {
        glColor3f(1.0f, 0.3f, 0.3f);
        drawCircle(655.0f + (float)i * 24.0f, 22.0f, 7.0f);
    }
    glColor3f(0.7f, 0.9f, 0.7f);
    drawTextSmall("LIVES", 593, 28);
    float sp = min(carSpeed / 14.0f, 1.0f);
    glColor3f(0.18f, 0.18f, 0.18f);
    glRectf(295.0f, 10.0f, 500.0f, 26.0f);
    glColor3f(0.2f + sp*0.7f, 0.8f - sp*0.45f, 0.1f);
    glRectf(295.0f, 10.0f, 295.0f + 205.0f*sp, 26.0f);
    glColor3f(0.80f, 0.80f, 0.80f);
    drawTextSmall("SPEED", 243, 24);
    glColor3f(0.45f, 0.65f, 0.45f);
    drawTextSmall("Left/Right arrows  |  ESC: Menu", 490, 578);
}

// ==================================================
// ================== SPACE SHOOTER =================
// ==================================================

struct SpaceBullet { float x, y; bool active; };
struct Alien       { float x, y; bool active; float cr, cg, cb; };
struct Particle    { float x, y, vx, vy, life; float r, g, b; };

float      shipX         = 400.0f;
int        spaceScore    = 0;
bool       spaceOver     = false;
int        spaceLives    = 3;
float      alienSpeed    = 1.2f;
float      alienDir      = 1.0f;   // 1 = right, -1 = left
float      alienDropTimer= 0.0f;
bool       alienDropping = false;
float      alienDropAmt  = 0.0f;
float      shootCooldown = 0.0f;

// Star field
struct Star { float x, y, brightness, speed; };
Star stars[120];

SpaceBullet bullets[8];
Alien       aliens[15];      // 3 rows × 5 cols
vector<Particle> particles;

void initSpace() {
    shipX         = 400.0f;
    spaceScore    = 0;
    spaceOver     = false;
    spaceLives    = 3;
    alienSpeed    = 1.2f;
    alienDir      = 1.0f;
    alienDropTimer= 0.0f;
    alienDropping = false;
    alienDropAmt  = 0.0f;
    shootCooldown = 0.0f;
    particles.clear();

    for (int i = 0; i < 8; i++) bullets[i].active = false;

    // 3 rows × 5 cols of aliens
    for (int row = 0; row < 3; row++)
        for (int col = 0; col < 5; col++) {
            int idx = row * 5 + col;
            aliens[idx].x      = 170.0f + col * 100.0f;
            aliens[idx].y      = 80.0f  + row * 70.0f;
            aliens[idx].active = true;
            // Row color: red / orange / yellow
            if (row == 0)      { aliens[idx].cr=1.0f; aliens[idx].cg=0.25f; aliens[idx].cb=0.25f; }
            else if (row == 1) { aliens[idx].cr=1.0f; aliens[idx].cg=0.60f; aliens[idx].cb=0.10f; }
            else               { aliens[idx].cr=0.95f;aliens[idx].cg=0.95f; aliens[idx].cb=0.15f; }
        }

    // Randomise star field once
    for (int i = 0; i < 120; i++) {
        stars[i].x          = (float)(rand() % WIDTH);
        stars[i].y          = (float)(rand() % HEIGHT);
        stars[i].brightness = 0.3f + (rand()%70)/100.0f;
        stars[i].speed      = 0.2f + (rand()%30)/100.0f;
    }
}

// Spawn explosion particles at (px,py)
void spawnExplosion(float px, float py, float r, float g, float b) {
    for (int i = 0; i < 18; i++) {
        Particle p;
        p.x    = px; p.y = py;
        float a = (float)(rand() % 628) / 100.0f;
        float spd = 1.5f + (rand()%30)/10.0f;
        p.vx   = cosf(a) * spd;
        p.vy   = sinf(a) * spd;
        p.life = 1.0f;
        p.r=r; p.g=g; p.b=b;
        particles.push_back(p);
    }
}

void drawSpaceShip(float x, float y) {
    // Thruster glow
    float tp = 0.5f + 0.5f * sinf(animTime * 8.0f);
    glColor4f(0.3f, 0.6f, 1.0f, 0.25f * tp);
    drawCircle(x, y + 22.0f, 14.0f);
    // Thruster flame
    glColor3f(0.2f, 0.5f + 0.4f*tp, 1.0f);
    glBegin(GL_TRIANGLES);
    glVertex2f(x - 8.0f, y + 18.0f);
    glVertex2f(x + 8.0f, y + 18.0f);
    glVertex2f(x,         y + 30.0f + 8.0f*tp);
    glEnd();
    // Body
    glColor3f(0.55f, 0.75f, 1.0f);
    glBegin(GL_POLYGON);
    glVertex2f(x,         y - 22.0f);  // nose
    glVertex2f(x + 18.0f, y + 18.0f);
    glVertex2f(x,         y + 10.0f);
    glVertex2f(x - 18.0f, y + 18.0f);
    glEnd();
    // Cockpit
    glColor4f(0.4f, 0.85f, 1.0f, 0.85f);
    drawCircle(x, y - 4.0f, 7.0f, 16);
    // Wings
    glColor3f(0.35f, 0.55f, 0.85f);
    glBegin(GL_TRIANGLES);
    glVertex2f(x - 18.0f, y + 18.0f);
    glVertex2f(x - 32.0f, y + 26.0f);
    glVertex2f(x - 10.0f, y +  6.0f);
    glEnd();
    glBegin(GL_TRIANGLES);
    glVertex2f(x + 18.0f, y + 18.0f);
    glVertex2f(x + 32.0f, y + 26.0f);
    glVertex2f(x + 10.0f, y +  6.0f);
    glEnd();
    // Highlight
    glColor4f(1.0f, 1.0f, 1.0f, 0.18f);
    glBegin(GL_TRIANGLES);
    glVertex2f(x,         y - 22.0f);
    glVertex2f(x + 6.0f,  y +  4.0f);
    glVertex2f(x - 6.0f,  y +  4.0f);
    glEnd();
}

void drawAlien(float x, float y, float cr, float cg, float cb) {
    float p = 0.7f + 0.3f * sinf(animTime * 2.5f + x * 0.05f);
    // Body oval
    glColor3f(cr*p, cg*p, cb*p);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 20; i++) {
        float a = i * (float)M_PI * 2.0f / 20.0f;
        glVertex2f(x + 18.0f*cosf(a), y + 11.0f*sinf(a));
    }
    glEnd();
    // Dome
    glColor4f(0.5f, 1.0f, 0.8f, 0.55f);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 14; i++) {
        float a = (float)M_PI + i * (float)M_PI / 13.0f;
        glVertex2f(x + 10.0f*cosf(a), y + 9.0f*sinf(a));
    }
    glEnd();
    // Eyes
    glColor3f(0.05f, 0.05f, 0.05f);
    glRectf(x - 9.0f, y - 3.0f, x - 4.0f, y + 2.0f);
    glRectf(x + 4.0f, y - 3.0f, x + 9.0f, y + 2.0f);
    // Legs (2 on each side)
    glColor3f(cr*0.7f, cg*0.7f, cb*0.7f); glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(x - 12.0f, y + 10.0f); glVertex2f(x - 20.0f, y + 20.0f);
    glVertex2f(x -  6.0f, y + 10.0f); glVertex2f(x - 10.0f, y + 20.0f);
    glVertex2f(x + 12.0f, y + 10.0f); glVertex2f(x + 20.0f, y + 20.0f);
    glVertex2f(x +  6.0f, y + 10.0f); glVertex2f(x + 10.0f, y + 20.0f);
    glEnd();
}

void drawSpaceGame() {
    // Space background
    glClearColor(0.0f, 0.0f, 0.06f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Scrolling stars
    for (int i = 0; i < 120; i++) {
        float b = stars[i].brightness;
        glColor3f(b, b, b);
        glPointSize(b > 0.7f ? 2.5f : 1.5f);
        glBegin(GL_POINTS);
        glVertex2f(stars[i].x, fmodf(stars[i].y + animTime * stars[i].speed * 5.0f, (float)HEIGHT));
        glEnd();
    }

    // Nebula streaks (decorative)
    for (int i = 0; i < 3; i++) {
        float nx = 150.0f + i * 250.0f;
        float ny = 100.0f + i * 150.0f;
        float np = 0.5f + 0.5f * sinf(animTime * 0.3f + i);
        glColor4f(0.2f + i*0.1f, 0.05f, 0.3f + i*0.05f, 0.06f * np);
        drawCircle(nx, ny, 80.0f + 20.0f*sinf(animTime*0.2f + i), 32);
    }

    if (spaceOver) {
        glColor4f(0.0f, 0.0f, 0.0f, 0.70f);
        glRectf(0.0f, 0.0f, (float)WIDTH, (float)HEIGHT);
        float g = 0.7f + 0.3f * sinf(animTime * 2.0f);
        glColor3f(g, 0.15f, 0.15f);
        drawTextLarge("GAME OVER", 290, 270);
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText("Score: " + to_string(spaceScore), 345, 308);
        drawText("Press SPACE to restart", 280, 340);
        return;
    }

    // Check win
    bool anyAlive = false;
    for (int i = 0; i < 15; i++) if (aliens[i].active) { anyAlive = true; break; }
    if (!anyAlive) {
        // Next wave: speed up, reset aliens
        alienSpeed = min(alienSpeed + 0.6f, 5.5f);
        for (int row = 0; row < 3; row++)
            for (int col = 0; col < 5; col++) {
                int idx = row * 5 + col;
                aliens[idx].x      = 170.0f + col * 100.0f;
                aliens[idx].y      = 80.0f  + row * 70.0f;
                aliens[idx].active = true;
            }
        alienDir = 1.0f;
        alienDropping = false;
    }

    // Draw bullets
    for (int i = 0; i < 8; i++) {
        if (!bullets[i].active) continue;
        float bp = 0.7f + 0.3f * sinf(animTime * 10.0f);
        glColor4f(0.4f, 1.0f, 0.4f, 0.35f * bp);
        drawCircle(bullets[i].x, bullets[i].y, 6.0f, 12);
        glColor3f(0.6f, 1.0f, 0.6f);
        glRectf(bullets[i].x - 2.0f, bullets[i].y - 10.0f,
                bullets[i].x + 2.0f, bullets[i].y + 4.0f);
    }

    // Draw aliens
    for (int i = 0; i < 15; i++)
        if (aliens[i].active)
            drawAlien(aliens[i].x, aliens[i].y, aliens[i].cr, aliens[i].cg, aliens[i].cb);

    // Draw particles
    for (auto& p : particles) {
        glColor4f(p.r, p.g, p.b, p.life);
        glPointSize(3.0f * p.life + 1.0f);
        glBegin(GL_POINTS);
        glVertex2f(p.x, p.y);
        glEnd();
    }

    // Draw ship
    drawSpaceShip(shipX, 510.0f);

    // Ground line
    glColor3f(0.15f, 0.15f, 0.35f); glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(0.0f, 545.0f); glVertex2f((float)WIDTH, 545.0f);
    glEnd();

    // HUD
    glColor4f(0.0f, 0.0f, 0.0f, 0.55f);
    glRectf(0.0f, 558.0f, (float)WIDTH, (float)HEIGHT);
    glColor3f(0.2f, 0.5f, 1.0f); glLineWidth(1.0f);
    glBegin(GL_LINES);
    glVertex2f(0.0f, 558.0f); glVertex2f((float)WIDTH, 558.0f);
    glEnd();
    glColor3f(0.85f, 0.95f, 1.0f);
    drawText("SCORE: " + to_string(spaceScore), 20, 582);
    // Ship lives as small ships
    for (int i = 0; i < spaceLives; i++) {
        glColor3f(0.4f, 0.7f, 1.0f);
        float lx = 660.0f + i * 30.0f;
        glBegin(GL_TRIANGLES);
        glVertex2f(lx,        570.0f);
        glVertex2f(lx - 8.0f, 584.0f);
        glVertex2f(lx + 8.0f, 584.0f);
        glEnd();
    }
    glColor3f(0.6f, 0.7f, 0.9f);
    drawTextSmall("LIVES", 622, 582);
    glColor3f(0.3f, 0.4f, 0.6f);
    drawTextSmall("Left/Right: Move  |  SPACE: Shoot  |  ESC: Menu", 190, 595);
}

// ==================================================
// ================== MENU ==========================
// ==================================================

// Simple clickable button helper
struct Button {
    float x, y, w, h;
    string label;
    float r, g, b;
};

Button menuButtons[3] = {
    { 275.0f, 220.0f, 250.0f, 46.0f, "1   Snake Game",  0.20f, 0.75f, 0.30f },
    { 275.0f, 285.0f, 250.0f, 46.0f, "2   Car Game",    0.90f, 0.55f, 0.10f },
    { 275.0f, 350.0f, 250.0f, 46.0f, "3   Space Shooter",0.30f, 0.55f, 1.00f }
};

// Track which button the mouse is hovering over (-1 = none)
int hoveredButton = -1;

void drawMenuButton(const Button& b, bool hovered) {
    float glow = hovered ? (0.6f + 0.4f * sinf(animTime * 4.0f)) : 0.0f;

    // Shadow
    glColor4f(0.0f, 0.0f, 0.0f, 0.40f);
    glRectf(b.x + 4.0f, b.y + 4.0f, b.x + b.w + 4.0f, b.y + b.h + 4.0f);

    // Fill
    float dim = hovered ? 1.0f : 0.55f;
    glColor4f(b.r * dim * 0.25f, b.g * dim * 0.25f, b.b * dim * 0.25f, 0.88f);
    glRectf(b.x, b.y, b.x + b.w, b.y + b.h);

    // Glow border (multi-layer when hovered)
    int layers = hovered ? 5 : 2;
    for (int l = layers; l >= 1; l--) {
        float alpha = hovered
            ? (0.06f + 0.10f * glow) * (float)(layers - l + 1) / (float)layers
            : 0.25f / (float)l;
        glColor4f(b.r, b.g, b.b, alpha);
        glLineWidth(hovered ? (float)(l * 2) : 1.5f);
        float off = (float)(l - 1);
        glBegin(GL_LINE_LOOP);
        glVertex2f(b.x - off,       b.y - off);
        glVertex2f(b.x + b.w + off, b.y - off);
        glVertex2f(b.x + b.w + off, b.y + b.h + off);
        glVertex2f(b.x - off,       b.y + b.h + off);
        glEnd();
    }

    // Label
    float lc = hovered ? 1.0f : 0.75f;
    glColor3f(b.r * lc + (1.0f - b.r*lc) * glow * 0.3f,
              b.g * lc + (1.0f - b.g*lc) * glow * 0.3f,
              b.b * lc + (1.0f - b.b*lc) * glow * 0.3f);
    // Centre the text roughly
    drawText(b.label, (int)(b.x + 28.0f), (int)(b.y + b.h * 0.62f));
}

void drawMenu() {
    glClearColor(0.04f, 0.04f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Animated star field in menu too
    for (int i = 0; i < 120; i++) {
        float b = stars[i].brightness * 0.6f;
        glColor3f(b, b, b * 1.3f);
        glPointSize(b > 0.4f ? 2.0f : 1.0f);
        glBegin(GL_POINTS);
        glVertex2f(stars[i].x,
                   fmodf(stars[i].y + animTime * stars[i].speed * 1.5f, (float)HEIGHT));
        glEnd();
    }

    // Title glow
    float gw = 0.5f + 0.5f * sinf(animTime * 1.2f);
    // Glow layers behind title
    for (int l = 6; l >= 1; l--) {
        glColor4f(0.1f, 0.6f * gw, 0.2f, 0.04f * gw * (7-l) / 6.0f);
        drawTextLarge("Arcade Multiverse", 252 - l, 130 - l);
        drawTextLarge("Arcade Multiverse", 252 + l, 130 + l);
    }
    glColor3f(0.25f*gw, 0.85f + 0.15f*gw, 0.35f*gw);
    drawTextLarge("Arcade Multiverse", 253, 130);

    // Subtitle
    glColor3f(0.35f, 0.45f, 0.70f);
    drawTextSmall("Select a game below or press its number key", 220, 175);

    // Draw the three buttons
    for (int i = 0; i < 3; i++)
        drawMenuButton(menuButtons[i], hoveredButton == i);

    // ESC hint
    glColor3f(0.38f, 0.38f, 0.55f);
    drawText("ESC  —  Exit", 330, 430);

    // Controls hint
    glColor3f(0.22f, 0.22f, 0.38f);
    drawTextSmall("Snake: arrow keys     Car: left/right arrows     Space: left/right + SPACE", 90, 490);
}

// ==================================================
// ================== UPDATE ========================
// ==================================================
void update(int) {
    animTime += 0.05f;

    // ---- Snake ----
    if (currentState == SNAKE && !snakeOver) {
        snakeTickAccum += 120;
        if (snakeTickAccum >= snakeInterval()) {
            snakeTickAccum = 0;
            GridPt head = snake.front();
            if (snakeDir==1) head.y--;
            if (snakeDir==2) head.x++;
            if (snakeDir==3) head.y++;
            if (snakeDir==4) head.x--;

            bool dead = (head.x < 0 || head.x >= GRID_W ||
                         head.y < 0 || head.y >= GRID_H);
            for (int i=1; !dead && i<(int)snake.size(); i++)
                if (snake[i].x==head.x && snake[i].y==head.y) dead=true;

            if (dead) { snakeOver = true; }
            else {
                snake.push_front(head);
                if (head.x==food.x && head.y==food.y) {
                    snakeScore += 10;
                    food = randomFood();
                } else {
                    snake.pop_back();
                }
            }
        }
    }

    // ---- Car ----
    if (currentState == CAR && !carOver) {
        carSpeed = min(6.0f + (float)carScore * 0.035f, 18.0f);
        roadOffset += carSpeed;
        objOffset  += carSpeed * 0.9f;

        signalTimer += 0.05f;
        if (signalTimer > 5.0f) {
            signalTimer = 0.0f;
            signalState = (signalState + 1) % 3;
        }

        if (carScore >= 20)  enemies[1].active = true;
        if (carScore >= 60)  enemies[2].active = true;
        if (carScore >= 100) enemies[3].active = true;

        for (int i = 0; i < 4; i++) {
            if (!enemies[i].active) continue;
            enemies[i].y += carSpeed * 1.3f;

            if (enemies[i].y > HEIGHT + 100) {
                enemies[i].y    = -(60 + rand() % 200);
                enemies[i].lane = rand() % 3;
                carScore += 10;
            }

            if (enemies[i].lane == playerLane &&
                enemies[i].y > 403.0f && enemies[i].y < 533.0f) {
                lives--;
                enemies[i].y    = -(60 + rand() % 200);
                enemies[i].lane = rand() % 3;
                if (lives <= 0) carOver = true;
            }
        }
    }

    // ---- Space Shooter ----
    if (currentState == SPACE && !spaceOver) {

        // Move bullets upward
        for (int i = 0; i < 8; i++) {
            if (!bullets[i].active) continue;
            bullets[i].y -= 9.0f;
            if (bullets[i].y < -10.0f) bullets[i].active = false;
        }

        // Shoot cooldown
        if (shootCooldown > 0.0f) shootCooldown -= 1.0f;

        // Find leftmost and rightmost active alien for wall-bounce logic
        float leftmost  =  9999.0f;
        float rightmost = -9999.0f;
        for (int i = 0; i < 15; i++) {
            if (!aliens[i].active) continue;
            if (aliens[i].x - 20.0f < leftmost)  leftmost  = aliens[i].x - 20.0f;
            if (aliens[i].x + 20.0f > rightmost)  rightmost = aliens[i].x + 20.0f;
        }

        // Drop logic
        if (alienDropping) {
            for (int i = 0; i < 15; i++)
                if (aliens[i].active) aliens[i].y += 3.5f;
            alienDropAmt += 3.5f;
            if (alienDropAmt >= 28.0f) {
                alienDropping = false;
                alienDropAmt  = 0.0f;
                alienDir      = -alienDir;  // reverse after drop
            }
        } else {
            // Move horizontally
            for (int i = 0; i < 15; i++)
                if (aliens[i].active) aliens[i].x += alienSpeed * alienDir;

            // Hit wall?
            if ((alienDir > 0 && rightmost >= (float)WIDTH - 20.0f) ||
                (alienDir < 0 && leftmost  <= 20.0f)) {
                alienDropping = true;
                alienDropAmt  = 0.0f;
            }
        }

        // Aliens reach bottom → game over
        for (int i = 0; i < 15; i++)
            if (aliens[i].active && aliens[i].y > 530.0f) {
                spaceOver = true;
            }

        // Bullet–alien collision
        for (int b = 0; b < 8; b++) {
            if (!bullets[b].active) continue;
            for (int a = 0; a < 15; a++) {
                if (!aliens[a].active) continue;
                float dx = bullets[b].x - aliens[a].x;
                float dy = bullets[b].y - aliens[a].y;
                if (dx > -22.0f && dx < 22.0f && dy > -18.0f && dy < 18.0f) {
                    spawnExplosion(aliens[a].x, aliens[a].y,
                                   aliens[a].cr, aliens[a].cg, aliens[a].cb);
                    aliens[a].active  = false;
                    bullets[b].active = false;
                    spaceScore       += 10;
                    alienSpeed        = min(alienSpeed + 0.08f, 5.5f);
                    break;
                }
            }
        }

        // Update particles
        for (auto& p : particles) {
            p.x    += p.vx;
            p.y    += p.vy;
            p.life -= 0.045f;
        }
        particles.erase(
            remove_if(particles.begin(), particles.end(),
                      [](const Particle& p){ return p.life <= 0.0f; }),
            particles.end());
    }

    glutPostRedisplay();
    glutTimerFunc(60, update, 0);   // ~60 fps
}

// ==================================================
// ================== DISPLAY =======================
// ==================================================
void display() {
    if      (currentState == MENU)  drawMenu();
    else if (currentState == SNAKE) drawSnakeGame();
    else if (currentState == CAR)   drawCarGame();
    else if (currentState == SPACE) drawSpaceGame();
    glutSwapBuffers();
}

// ==================================================
// ================== INPUT =========================
// ==================================================

// Key-down state for smooth ship movement
bool keyLeft  = false;
bool keyRight = false;

void keyboard(unsigned char key, int, int) {
    // Number-key shortcuts from anywhere
    if (key == '1') {
        if (snakeScore > highScore) highScore = snakeScore;
        currentState = SNAKE;
        initSnake();
    }
    if (key == '2') {
        currentState = CAR;
        initCar();
    }
    if (key == '3') {
        currentState = SPACE;
        initSpace();
    }
    if (key == 27) {   // ESC
        if (currentState != MENU) {
            if (currentState == SNAKE && snakeScore > highScore)
                highScore = snakeScore;
            currentState = MENU;
        } else {
            exit(0);
        }
    }
    if (key == ' ') {
        if (snakeOver) {
            if (snakeScore > highScore) highScore = snakeScore;
            initSnake();
        }
        if (carOver)   initCar();
        if (spaceOver) initSpace();
        // Space: shoot
        if (currentState == SPACE && !spaceOver && shootCooldown <= 0.0f) {
            for (int i = 0; i < 8; i++) {
                if (!bullets[i].active) {
                    bullets[i].x      = shipX;
                    bullets[i].y      = 490.0f;
                    bullets[i].active = true;
                    shootCooldown     = 10.0f;
                    break;
                }
            }
        }
    }
}

void keyboardUp(unsigned char key, int, int) {
    (void)key;
}

void special(int key, int, int) {
    if (currentState == SNAKE) {
        if (key==GLUT_KEY_UP    && snakeDir!=3) snakeDir=1;
        if (key==GLUT_KEY_RIGHT && snakeDir!=4) snakeDir=2;
        if (key==GLUT_KEY_DOWN  && snakeDir!=1) snakeDir=3;
        if (key==GLUT_KEY_LEFT  && snakeDir!=2) snakeDir=4;
    }
    if (currentState == CAR) {
        if (key==GLUT_KEY_LEFT  && playerLane>0) playerLane--;
        if (key==GLUT_KEY_RIGHT && playerLane<2) playerLane++;
    }
    if (currentState == SPACE && !spaceOver) {
        if (key==GLUT_KEY_LEFT)  keyLeft  = true;
        if (key==GLUT_KEY_RIGHT) keyRight = true;
    }
}

void specialUp(int key, int, int) {
    if (key==GLUT_KEY_LEFT)  keyLeft  = false;
    if (key==GLUT_KEY_RIGHT) keyRight = false;
}

// Idle: smooth ship movement polled per frame
void idle() {
    if (currentState == SPACE && !spaceOver) {
        if (keyLeft)  { shipX -= 4.5f; if (shipX < 34.0f) shipX = 34.0f; }
        if (keyRight) { shipX += 4.5f; if (shipX > (float)WIDTH - 34.0f) shipX = (float)WIDTH - 34.0f; }
    }
    glutPostRedisplay();
}

// Mouse click on menu buttons
void mouse(int button, int state, int mx, int my) {
    if (button != GLUT_LEFT_BUTTON || state != GLUT_UP) return;
    if (currentState != MENU) return;
    float fx = (float)mx;
    float fy = (float)my;
    for (int i = 0; i < 3; i++) {
        Button& b = menuButtons[i];
        if (fx >= b.x && fx <= b.x + b.w &&
            fy >= b.y && fy <= b.y + b.h) {
            if (i == 0) { if (snakeScore > highScore) highScore = snakeScore; currentState = SNAKE; initSnake(); }
            if (i == 1) { currentState = CAR;   initCar();   }
            if (i == 2) { currentState = SPACE; initSpace(); }
        }
    }
}

// Mouse motion for hover effect on buttons
void mouseMotion(int mx, int my) {
    if (currentState != MENU) { hoveredButton = -1; return; }
    float fx = (float)mx;
    float fy = (float)my;
    hoveredButton = -1;
    for (int i = 0; i < 3; i++) {
        Button& b = menuButtons[i];
        if (fx >= b.x && fx <= b.x + b.w &&
            fy >= b.y && fy <= b.y + b.h) {
            hoveredButton = i;
            break;
        }
    }
}

// ==================================================
int main(int argc, char** argv) {
    srand((unsigned)time(nullptr));

    // Pre-init star field so menu has stars from frame 0
    for (int i = 0; i < 120; i++) {
        stars[i].x          = (float)(rand() % WIDTH);
        stars[i].y          = (float)(rand() % HEIGHT);
        stars[i].brightness = 0.3f + (rand()%70)/100.0f;
        stars[i].speed      = 0.2f + (rand()%30)/100.0f;
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Arcade Multiverse");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIDTH, HEIGHT, 0);

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(special);
    glutSpecialUpFunc(specialUp);
    glutMouseFunc(mouse);
    glutPassiveMotionFunc(mouseMotion);
    glutIdleFunc(idle);
    glutTimerFunc(16, update, 0);

    glutMainLoop();
    return 0;
}
