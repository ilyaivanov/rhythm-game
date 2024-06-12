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

inline V2f BulletCenter(Bullet *bullet)
{
    f32 size = bullet->type == PlayerBullet ? playerBulletSize : enemyBulletSize;
    return V2fAddScalar(bullet->pos, size / 2);
}

inline V2f EnemyCenter(Enemy *enemy)
{
    return V2fAddScalar(enemy->pos, enemySize / 2);
}

void Burst(V2f from, V3f color);

void HandleCollisions(V2f playerPos, V2i screen, i32 *score, i32 *health)
{
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

                *score = *score + 1;

                V3f color;
                if (enemy->type == Walker)
                    color = (V3f){1.0f, 0.3f, 0.3f};
                else if (enemy->type == WallShooter)
                    color = (V3f){0.8f, 0.5f, 0.8f};
                else
                    color = (V3f){0.3f, 0.8f, 0.8f};

                Burst(EnemyCenter(enemy), color);
            }
        }

        if (bullet->type == EnemyBullet && CheckTwoSquareOverlap(bullet->pos, bullet->size, playerPos, playerSize))
        {
            // Bullet hit on Player
            bullet->isAlive = 0;
            *health = *health - 1;
        }

        if (!IsPointInsideRect((V2f){0, 0}, (V2f){(f32)screen.x, (f32)screen.y}, bullet->pos))
            bullet->isAlive = 0;
    }

    for (i32 j = 0; j < ArrayLength(enemies); j++)
    {
        Enemy *enemy = &enemies[j];
        if (enemy->isAlive && CheckTwoSquareOverlap(enemy->pos, enemySize, playerPos, playerSize))
        {
            // Enemy hit on Player
            enemy->isAlive = 0;
            *health = *health - 1;
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