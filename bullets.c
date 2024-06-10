#pragma once
#include <gl/gl.h>
#include "utils/all.c"
#include "constants.c"
#include "enemies.c"

typedef struct Bullet
{
    // used to denote if memory segment is an actual living bullet
    i32 isAlive;

    V2f pos;
    V2f direction;
    f32 speed;
} Bullet;

Bullet bullets[256];

void Fire(V2f from, V2f to)
{
    for (i32 i = 0; i < ArrayLength(bullets); i++)
    {
        if (!bullets[i].isAlive)
        {
            bullets[i].isAlive = 1;

            bullets[i].pos = V2fDiffScalar(from, bulletSize / 2);
            bullets[i].direction = V2fNormalize(V2fDiff(V2fDiffScalar(to, bulletSize / 2), bullets[i].pos));

            bullets[i].speed = bulletSpeed;

            break;
        }
    }
}

void UpdateBullets()
{
    for (i32 i = 0; i < ArrayLength(bullets); i++)
    {
        if (bullets[i].isAlive)
        {
            bullets[i].pos = V2fAdd(bullets[i].pos, V2fMult(bullets[i].direction, bullets[i].speed));
        }
    }

    for (i32 i = 0; i < ArrayLength(bullets); i++)
    {
        for (i32 j = 0; j < ArrayLength(enemies); j++)
        {
            if (enemies[j].isAlive && bullets[i].isAlive && V2fDistance(bullets[i].pos, enemies[j].pos) < enemySize)
            {
                enemies[j].isAlive = 0;
                bullets[i].isAlive = 0;
            }
        }
    }
}

void DrawBullets(GLint viewLocation, GLint colorLocation)
{
    glUniform4f(colorLocation, 1.0f, 1.0f, 0.5f, 1.0f);
    for (i32 i = 0; i < ArrayLength(bullets); i++)
    {
        if (bullets[i].isAlive)
        {
            V2f pos = bullets[i].pos;
            f32 size = bulletSize;

            Mat4 view = Mat4ScaleUniform(Mat4TranslateV2f(Mat4Identity(), pos), size);
            glUniformMatrix4fv(viewLocation, 1, GL_TRUE, view.values);

            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
    }
}