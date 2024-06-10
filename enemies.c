#pragma once
#include "utils/all.c"
#include <gl/gl.h>
#include "constants.c"

typedef struct Enemy
{
    i32 isAlive;

    V2f pos;
    V2f direction;
    f32 speed;
} Enemy;

Enemy enemies[256];

f32 timeToSpan = 0;
f32 enemiesPerSecond = 2.0f;

RandomSeries enemySeries;
void InitEnemies()
{
    enemySeries = CreateSeries();
}

void SpawnEnemy(V2i clientAreaSize)
{
    for (int i = 0; i < ArrayLength(enemies); i++)
    {
        if (!enemies[i].isAlive)
        {
            enemies[i].isAlive = 1;

            u32 direction = RandomChoice(&enemySeries, 4);
            u32 randomX = RandomChoice(&enemySeries, clientAreaSize.x);
            u32 randomY = RandomChoice(&enemySeries, clientAreaSize.y);
            switch (direction)
            {

            // up
            case 0:
                enemies[i].pos = (V2f){randomX, clientAreaSize.y};
                enemies[i].direction = (V2f){0.0f, -1.0f};
                break;
            // down
            case 1:
                enemies[i].pos = (V2f){randomX, -enemySize};
                enemies[i].direction = (V2f){0.0f, 1.0f};
                break;
            // left
            case 2:
                enemies[i].pos = (V2f){-enemySize, randomY};
                enemies[i].direction = (V2f){1.0f, 0.0f};
                break;
            // right
            case 3:
                enemies[i].pos = (V2f){clientAreaSize.x, randomY};
                enemies[i].direction = (V2f){-1.0f, 0.0f};
                break;
            }

            break;
        }
    }
}

void UpdateEnemies(V2i clientAreaSize)
{
    timeToSpan -= 16.666f;
    if (timeToSpan < 0)
    {
        timeToSpan = (1000.0f / enemiesPerSecond) + timeToSpan;
        SpawnEnemy(clientAreaSize);
    }

    for (int i = 0; i < ArrayLength(enemies); i++)
    {
        if (enemies[i].isAlive)
        {
            enemies[i].pos = V2fAdd(enemies[i].pos, V2fMult(enemies[i].direction, enemySpeed));
        }
    }
}

void DrawEnemies(GLint viewLocation, GLint colorLocation)
{
    glUniform4f(colorLocation, 1.0f, 0.3f, 0.3f, 1.0f);
    for (int i = 0; i < ArrayLength(enemies); i++)
    {
        if (enemies[i].isAlive)
        {
            V2f pos = enemies[i].pos;

            Mat4 view = Mat4ScaleUniform(Mat4TranslateV2f(Mat4Identity(), pos), enemySize);
            glUniformMatrix4fv(viewLocation, 1, GL_TRUE, view.values);

            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
    }
}
