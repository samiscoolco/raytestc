#include "raylib.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

const float TPI = 2 * PI;
const float P2 = PI / 2;
const float P3 = 3 * PI / 2;

const int screenWidth = 512;
const int screenHeight = 512;

float px, py, pa, pdx, pdy;
float speed;
float movespeed;
float maxspeed;
float turnspeed;
float friction;
float dt;
int mode;
int vdep;

Texture2D gun_tex;

int mapX = 8, mapY = 8, mapS = 64;
int map[] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 2, 0, 1, 1, 1,
    2, 0, 0, 0, 0, 0, 0, 2,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 2, 1, 1, 1};

Color *wall1;
Color *wall2;
Color *wall3;

Color *walltextures[3];

float dist(float ax, float ay, float bx, float by, float ang)
{
    return (sqrt((bx - ax) * (bx - ax) + (by - ay) * (by - ay)));
}
void drawMap2D()
{
    int x, y, xo, yo;
    Color sqColor;
    for (y = 0; y < mapY; y++)
    {
        for (x = 0; x < mapX; x++)
        {
            if (map[y * mapX + x] == 1)
            {
                sqColor = GRAY;
            }
            else
            {
                sqColor = LIGHTGRAY;
            }
            xo = x * mapS;
            yo = y * mapS;
            DrawRectangle(x * mapS, y * mapS, mapS, mapS, BLACK);
            DrawRectangle((x * mapS) + 1, (y * mapS) + 1, mapS - 1, mapS - 1, sqColor);
        }
    }
}

void drawPlayer()
{
    DrawCircle(px, py, 5, RED);
    DrawLineEx((Vector2){px, py}, (Vector2){px + (pdx * 5), py + (pdy * 5)}, 3.0f, MAROON);
}

void drawGame()
{
    int r, mx, my, mp, dof, walltex, walltexv, walltexh;
    float rx, ry, ra, xo, yo;

    ra = pa - DEG2RAD * 30;
    if (ra < 0)
    {
        ra += TPI;
    }
    if (ra > TPI)
    {
        ra -= TPI;
    }
    if (mode == 0)
    {
        drawMap2D();

        drawPlayer();
    }
    for (r = 0; r < 60; r++)
    {

        // check hor lines
        float disH = 1000000, hx = px, hy = py;
        // depth of field
        dof = 0;
        float aTan = -1 / tan(ra);
        if (ra > PI)
        {
            // looking up
            ry = (((int)py >> 6) << 6) - 0.0001;
            rx = (py - ry) * aTan + px;
            yo = -64;
            xo = -yo * aTan;
        }
        if (ra < PI)
        {
            // looking down
            ry = (((int)py >> 6) << 6) + 64;
            rx = (py - ry) * aTan + px;
            yo = 64;
            xo = -yo * aTan;
        }
        if (ra == 0 || ra == PI)
        {
            // directly left or right
            rx = px;
            ry = py;
            dof = vdep;
        }
        while (dof < vdep)
        {
            mx = (int)(rx) >> 6;
            my = (int)(ry) >> 6;
            mp = my * mapX + mx;
            if (mp > 0 && mp < mapX * mapY && map[mp] >= 1)
            {
                // hit wall
                hx = rx;
                hy = ry;
                disH = dist(px, py, hx, hy, ra);
                dof = vdep;
                walltexh = map[mp];
            }
            else
            {
                rx += xo;
                ry += yo;
                dof += 1;
            }
        }

        // check vert lines
        float disV = 1000000, vx = px, vy = py;
        dof = 0; // depth of field
        float nTan = -tan(ra);
        if (ra > P2 && ra < P3)
        { // looking left
            rx = (((int)px >> 6) << 6) - 0.0001;
            ry = (px - rx) * nTan + py;
            xo = -64;
            yo = -xo * nTan;
        }
        if (ra < P2 || ra > P3)
        { // looking right
            rx = (((int)px >> 6) << 6) + 64;
            ry = (px - rx) * nTan + py;
            xo = 64;
            yo = -xo * nTan;
        }
        if (ra == 0 || ra == PI)
        { // directly straight up or down
            rx = px;
            ry = py;
            dof = vdep;
        }
        while (dof < vdep)
        {
            mx = (int)(rx) >> 6;
            my = (int)(ry) >> 6;
            mp = my * mapX + mx;

            if (mp > 0 && mp < mapX * mapY && map[mp] >= 1)
            {
                vx = rx;
                vy = ry;
                disV = dist(px, py, vx, vy, ra);
                dof = vdep; // hit wall
                walltexv = map[mp];
            }
            else
            {
                rx += xo;
                ry += yo;
                dof += 1;
            }
        }

        int hitdist;
        int wallSide;
        if (disV < disH)
        {
            rx = vx;
            ry = vy;
            hitdist = disV;
            wallSide = 0;
            walltex = walltexv;
        }
        if (disH < disV)
        {
            rx = hx;
            ry = hy;
            hitdist = disH;
            wallSide = 1;
            walltex = walltexh;
        }
        if (mode == 0)
        {

            DrawLineEx((Vector2){px, py}, (Vector2){rx, ry}, 1.0f, RED);
        }
        if (mode == 1)
        {
            if (hitdist <= 0)
            {
                hitdist = 1;
            }
            float lineH = (mapS * 320) / hitdist;
            float ty_step = 32 / (float)lineH;
            float ty_off = 0;

            if (lineH > 320)
            {
                ty_off = (lineH - 320) / 2.0;
                lineH = 320;
            }
            float lineO = 230 - lineH / 2;

            int pxy;
            float ty = ty_off * ty_step;
            float tx;
            if (wallSide == 1)
            {
                tx = (int)(rx / 2.0) % 32;
            }
            else
            {
                tx = (int)(ry / 2.0) % 32;
            }

            for (pxy = 0; pxy < lineH; pxy++)
            {
                // printf("walltex: %d   ",walltex);
                Color c = walltextures[walltex - 1][((int)(ty) * 32) + (int)tx];
                // shading
                if (wallSide == 1)
                {
                    c.r = c.r * 0.5;
                    c.g = c.g * 0.5;
                    c.b = c.b * 0.5;
                }

                DrawRectangle(r * 8, pxy + lineO, 8, 8, c);
                ty += ty_step;
            }

            // DrawLineEx((Vector2){(r * 8.6), lineO}, (Vector2){(r * 8.6), lineH + lineO}, 8.6f, wallCol);
        }

        ra += DEG2RAD;
        if (ra < 0)
        {
            ra += TPI;
        }
        if (ra > TPI)
        {
            ra -= TPI;
        }
    }
}

void init()
{
    printf("initializing...");
    mode = 1;
    px = 200;
    py = 300;
    pa = 0;
    speed = 0;
    vdep = 8;

    movespeed = 20;   // acceleration
    maxspeed = 120;   // max running speed
    turnspeed = 7.5f; // turning
    friction = 90;    // decel

    printf("loading textures...");

    Image img = LoadImage("wall.png");
    wall2 = LoadImageColors(img);
    UnloadImage(img);

    img = LoadImage("wall2.png");
    wall1 = LoadImageColors(img);
    UnloadImage(img);

    img = LoadImage("gun.png");

    Color green = (Color){0, 255, 0, 255};
    Color transparent = (Color){0, 0, 0, 0};
    ImageColorReplace(&img, green, transparent);
    gun_tex = LoadTextureFromImage(img);
    UnloadImage(img);

    walltextures[0] = wall1;
    walltextures[1] = wall2;

    printf("starting game...");
}

void buttons()
{
    if (IsKeyDown(KEY_A))
    {
        pa -= turnspeed * dt;
        if (pa < 0)
            pa += TPI;
    }
    if (IsKeyDown(KEY_D))
    {
        pa += turnspeed * dt;
        if (pa > TPI)
            pa -= TPI;
    }

    pdx = cos(pa);
    pdy = sin(pa);

    bool moving = false;
    if (IsKeyDown(KEY_W))
    {
        speed += movespeed * dt * 40; // accelerate forward
        moving = true;
    }
    if (IsKeyDown(KEY_S))
    {
        speed -= movespeed * dt * 40; // accelerate backward
        moving = true;
    }

    if (!moving)
    {
        if (speed > 0)
        {
            speed -= friction * dt;
            if (speed < 0)
                speed = 0;
        }
        else if (speed < 0)
        {
            speed += friction * dt;
            if (speed > 0)
                speed = 0;
        }
    }

    if (speed > maxspeed)
        speed = maxspeed;
    if (speed < -maxspeed)
        speed = -maxspeed;

    if (IsKeyPressed(KEY_TAB))
    {
        mode = !mode;
    }
}

void playerMovement()
{

    float opx = px;
    float opy = py;

    px += pdx * speed * dt;
    py += pdy * speed * dt;

    int mx = (int)(px) >> 6;
    int my = (int)(py) >> 6;
    int mp = my * mapX + mx;
    if (mp > 0 && mp < mapX * mapY && map[mp] >= 1)
    {
        // collision
        px = opx;
        py = opy;

        // bump
        px -= pdx * speed * dt;
        py -= pdy * speed * dt;

        // stop
        speed = 0;
    }
}

int main(void)
{

    // SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Raytest");
    init();

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        dt = GetFrameTime();

        buttons();
        playerMovement();

        BeginDrawing();
        ClearBackground(DARKGRAY);
        DrawRectangle(0, 200, 520, 500, GRAY);
        drawGame();

        if (mode == 1)
        {
            float scale = 2.5f; // scale factor

            int texW = gun_tex.width * scale;
            int texH = gun_tex.height * scale;

            // Position: center horizontally, align to bottom
            int drawX = (screenWidth / 2) - (texW / 2);
            int drawY = screenHeight - texH;
            DrawTextureEx(gun_tex, (Vector2){drawX, drawY}, 0.0f, scale, WHITE);
        }

        char myString[50];
        char floatString[20];
        snprintf(myString, 50, "Raytest SPD: %f   ANG: %f", speed, pa);
        DrawText(myString, 10, 10, 20, BLACK);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
