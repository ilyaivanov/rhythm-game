#pragma once
#include <gl/gl.h>
#include "utils/all.c"
#include "constants.c"
#include "enemies.c"

typedef struct Bullet
{
    // used to denote if memory segment is an actual living bullet
    i32 isAlive;

    BulletType type;

    V2f pos;
    V2f direction;
    f32 speed;
    f32 size;
} Bullet;

Bullet bullets[256];

void Fire(V2f from, V2f to, BulletType type, f32 speed, f32 size)
{
    for (i32 i = 0; i < ArrayLength(bullets); i++)
    {
        if (!bullets[i].isAlive)
        {
            bullets[i].isAlive = 1;

            bullets[i].type = type;
            bullets[i].size = size;

            bullets[i].pos = V2fDiffScalar(from, size / 2);
            bullets[i].direction = V2fNormalize(V2fDiff(V2fDiffScalar(to, size / 2), bullets[i].pos));

            bullets[i].speed = speed;

            break;
        }
    }
}

void UpdateBullets(V2f playerPos, V2i screen)
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
        Bullet *bullet = &bullets[i];
        for (i32 j = 0; j < ArrayLength(enemies); j++)
        {
            Enemy *enemy = &enemies[j];
            if (enemy->isAlive && bullet->isAlive && bullet->type == PlayerBullet && CheckTwoSquareOverlap(bullet->pos, bullet->size, enemy->pos, enemySize))
            {
                enemy->isAlive = 0;
                bullet->isAlive = 0;
            }
        }

        if (bullet->type == EnemyBullet && CheckTwoSquareOverlap(bullet->pos, bullet->size, playerPos, playerSize))
        {
            bullet->isAlive = 0;
        }

        if (!IsPointInsideRect((V2f){0, 0}, (V2f){(f32)screen.x, (f32)screen.y}, bullet->pos))
            bullet->isAlive = 0;
    }
}

void DrawBullets(GLint viewLocation, GLint colorLocation)
{

    for (i32 i = 0; i < ArrayLength(bullets); i++)
    {
        Bullet *bullet = &bullets[i];
        if (bullet->isAlive)
        {
            if (bullet->type == PlayerBullet)
                glUniform4f(colorLocation, 1.0f, 1.0f, 0.5f, 1.0f);
            else
                glUniform4f(colorLocation, 1.0f, 0.3f, 0.3f, 1.0f);

            Mat4 view = Mat4ScaleUniform(Mat4TranslateV2f(Mat4Identity(), bullet->pos), bullet->size);
            glUniformMatrix4fv(viewLocation, 1, GL_TRUE, view.values);

            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
    }
}