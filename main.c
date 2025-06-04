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
float bob;
float boba;
float dt;
bool mode;
int vdep;

Texture2D gun_tex;
float shootFrame;
bool shooting;

int mapS = 64;
int mapX = 16, mapY = 16;
int map[] = {
    1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 2,
    1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1};

Color *wall1;
Color *wall2;
Color *wall3;

Texture2D wall1tex;
Texture2D wall2tex;

Color *floor1;
Color *floor2;
Color *floor3;

Color *walltextures[3];
Color *floortextures[3];
Texture2D walltextures2D[3];

float dist(float ax, float ay, float bx, float by, float ang)
{
    return (sqrt((bx - ax) * (bx - ax) + (by - ay) * (by - ay)));
}
void drawMap2D()
{
    int x, y;
    float newmaps = screenWidth / mapX;
    float scalefactor = newmaps / mapS;
    int mapitem;
    Color sqColor;
    Texture2D sqTex;

    for (y = 0; y < mapY; y++)
    {
        for (x = 0; x < mapX; x++)
        {
            mapitem = map[y * mapX + x];
            DrawRectangle(x * newmaps, y * newmaps, newmaps, newmaps, BLACK);
            if (mapitem >= 1)
            {
                sqTex = walltextures2D[mapitem - 1];
                DrawTextureEx(sqTex, (Vector2){x * newmaps, y * newmaps}, 0.0f, 1, WHITE);
            }
            else
            {
                DrawRectangle((x * newmaps) + 1, (y * newmaps) + 1, newmaps - 1, newmaps - 1, LIGHTGRAY);
            }

            DrawCircle(px * scalefactor, py * scalefactor, 5, RED);
            DrawLineEx((Vector2){px * scalefactor, py * scalefactor}, (Vector2){px * scalefactor + (pdx * 10), py * scalefactor + (pdy * 10)}, 3.0f, MAROON);
        }
    }
}

void drawGame()
{
    int r, mx, my, mp, dof;
    float rx, ry, ra, xo, yo;

    int walltex = -1;
    int walltexv = -1;
    int walltexh = -1;

    ra = pa - DEG2RAD * 30;
    if (ra < 0)
    {
        ra += TPI;
    }
    if (ra > TPI)
    {
        ra -= TPI;
    }
    if (!mode)
    {
        drawMap2D();
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
            if (mp > 0 && mp < mapX * mapY && map[mp] >= 1 && map[mp] <= 3)
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

            if (mp > 0 && mp < mapX * mapY && map[mp] >= 1 && map[mp] <= 3)
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
        // if (mode == 0)
        // {

        //     DrawLineEx((Vector2){px, py}, (Vector2){rx, ry}, 1.0f, RED);
        // }
        if (mode && walltex >= 0 && walltex <= 3)
        {
            if (hitdist <= 0)
            {
                hitdist = 1;
            }
            float lineH = (mapS * screenHeight) / hitdist;
            float ty_step = 32 / (float)lineH;
            float ty_off = 0;

            if (lineH > screenHeight)
            {
                ty_off = (lineH - screenHeight) / 2.0;
                lineH = screenHeight;
            }
            float lineO = (screenHeight / 2) - lineH / 2;
            lineO = lineO + boba;

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

                DrawRectangle(r * 9, pxy + lineO, 9, 9, c);
                ty += ty_step;
            }

            // draw floors
            int floorStart = lineO + lineH;
            float camY = (screenHeight / 2.0f) + boba;

            for (int y = floorStart; y < screenHeight; y++)
            {
                float perspective = camY / (y - camY);
                float fx = px + cos(ra) * perspective * 64;
                float fy = py + sin(ra) * perspective * 64;

                int tx = ((int)fx) % 64;
                int ty = ((int)fy) % 64;

                if (tx < 0)
                {
                    tx += 64;
                }

                if (ty < 0)
                {
                    ty += 64;
                }

                Color fc = floortextures[0][(ty / 2) * 32 + (tx / 2)];
                DrawRectangle(r * 9, y, 9, 9, fc);
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
    mode = true;
    px = 200;
    py = 300;
    pa = 0;
    speed = 0;
    vdep = 16;
    bob = 0.00f;
    shootFrame = 0;

    movespeed = 20;   // acceleration
    maxspeed = 145;   // max running speed
    turnspeed = 5.5f; // turning
    friction = 90;    // decel

    printf("loading textures...");

    Image img = LoadImage("floor.png");
    floor1 = LoadImageColors(img);
    UnloadImage(img);

    img = LoadImage("wall.png");
    wall2 = LoadImageColors(img);
    wall2tex = LoadTextureFromImage(img);
    UnloadImage(img);

    img = LoadImage("wall2.png");
    wall1tex = LoadTextureFromImage(img);
    wall1 = LoadImageColors(img);
    UnloadImage(img);

    img = LoadImage("shotgun_sheet.png");
    Color green = (Color){255, 0, 255, 255};
    Color transparent = (Color){0, 0, 0, 0};
    ImageColorReplace(&img, green, transparent);
    gun_tex = LoadTextureFromImage(img);
    UnloadImage(img);

    walltextures2D[0] = wall1tex;
    walltextures2D[1] = wall2tex;
    walltextures[0] = wall1;
    walltextures[1] = wall2;

    floortextures[0] = floor1;

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
            {
                speed = 0;
            }
        }
        else if (speed < 0)
        {
            speed += friction * dt;
            if (speed > 0)
            {
                speed = 0;
            }
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

    if (IsKeyPressed(KEY_SPACE) && !shooting)
    {
        shooting = true;
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
        DrawRectangle(0, 250, 512, 512, GRAY);
        drawGame();

        if (bob < 40)
        {
            bob += speed * dt;
        }
        else
        {

            bob = 0;
        }
        if (speed == 0 && bob > 0)
        {
            bob -= 10 * dt;
        }

        float n = sin(bob / 40.0f * 2 * PI);
        float out = 5.5f + 4.5f * n;
        boba = out;
        // bobbing off for now
        //boba = 0;

        if (shooting)
        {
            shootFrame += 15 * dt;
            if (shootFrame >= 3)
            {
                shootFrame = 0;
                shooting = false;
            }
        }

        if (mode)
        {

            // DrawTextureRec(gun_tex, (Rectangle){0, 0, 64, 64}, (Vector2){drawX, drawY}, WHITE);
            DrawTexturePro(gun_tex, (Rectangle){(int)shootFrame * 64, 0, 64, 64}, (Rectangle){130, 300, 256, 256}, (Vector2){0, 0}, 0, WHITE);
            // DrawTextureEx(gun_tex, (Vector2){drawX, drawY}, 0.0f, scale, WHITE);
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
