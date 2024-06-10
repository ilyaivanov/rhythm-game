#pragma once
#include "utils/all.c"
#include <gl/gl.h>
#include "constants.c"

typedef enum EnemyType
{
    Walker,
    Chaser,
    WallShooter
} EnemyType;

typedef struct Enemy
{
    i32 isAlive;

    EnemyType type;

    f32 speed;

    // Walker and Chaser
    V2f pos;

    // Walker
    V2f direction;

    // WallShooter
    f32 screenWallPosition;
    i32 isClockWise;

} Enemy;

Enemy enemies[256];

f32 timeToSpan = 0;
f32 enemiesPerSecond = 2.0f;

RandomSeries enemySeries;

inline u32 RandomEnemyChoice(u32 optionsCount)
{
    return RandomChoice(&enemySeries, optionsCount);
}
inline u32 CoinFlip()
{
    return RandomChoice(&enemySeries, 2);
}

void InitEnemies()
{
    enemySeries = CreateSeries();
}

V2f BoundaryPositionToPoint(float position, float rectWidth, float rectHeight)
{
    float perimeter = 2 * (rectWidth + rectHeight);

    V2f point;
    if (position <= rectWidth)
    {
        // Top edge (left to right)
        point.x = position;
        point.y = 0;
    }
    else if (position <= rectWidth + rectHeight)
    {
        // Right edge (top to bottom)
        point.x = rectWidth;
        point.y = position - rectWidth;
    }
    else if (position <= 2 * rectWidth + rectHeight)
    {
        // Bottom edge (right to left)
        point.x = rectWidth - (position - rectWidth - rectHeight);
        point.y = rectHeight;
    }
    else
    {
        // Left edge (bottom to top)
        point.x = 0;
        point.y = rectHeight - (position - 2 * rectWidth - rectHeight);
    }

    return point;
}

void SpawnEnemy(V2i clientAreaSize)
{
    for (int i = 0; i < ArrayLength(enemies); i++)
    {
        Enemy *enemy = &enemies[i];
        if (!enemy->isAlive)
        {
            enemy->isAlive = 1;

            i32 typeChoise = RandomEnemyChoice(10);
            if (typeChoise < -1)
                enemy->type = Chaser;
            else if (typeChoise < 30)
                enemy->type = WallShooter;
            else
                enemy->type = Walker;

            if (enemy->type == WallShooter)
            {
                // enemy->screenWallPosition = RandomEnemyChoice(clientAreaSize.x * 2 + clientAreaSize.y * 2);
                enemy->screenWallPosition = RandomEnemyChoice(1500);
                enemy->isClockWise = CoinFlip();
            }
            else
            {
                u32 direction = RandomEnemyChoice(4);
                u32 randomX = RandomEnemyChoice(clientAreaSize.x - enemySize);
                u32 randomY = RandomEnemyChoice(clientAreaSize.y - enemySize);
                switch (direction)
                {
                // up
                case 0:
                    enemy->pos = (V2f){randomX, clientAreaSize.y};
                    enemy->direction = (V2f){0.0f, -1.0f};
                    break;
                // down
                case 1:
                    enemy->pos = (V2f){randomX, -enemySize};
                    enemy->direction = (V2f){0.0f, 1.0f};
                    break;
                // left
                case 2:
                    enemy->pos = (V2f){-enemySize, randomY};
                    enemy->direction = (V2f){1.0f, 0.0f};
                    break;
                // right
                case 3:
                    enemy->pos = (V2f){clientAreaSize.x, randomY};
                    enemy->direction = (V2f){-1.0f, 0.0f};
                    break;
                }
            }

            break;
        }
    }
}

void UpdateEnemies(V2i clientAreaSize, V2f playerPos)
{
    timeToSpan -= 16.666f;
    if (timeToSpan < 0)
    {
        timeToSpan = (1000.0f / enemiesPerSecond) + timeToSpan;
        SpawnEnemy(clientAreaSize);
    }

    for (int i = 0; i < ArrayLength(enemies); i++)
    {
        Enemy *enemy = &enemies[i];
        if (enemy->isAlive)
        {
            if (enemy->type == Walker)
            {
                enemy->pos = V2fAdd(enemy->pos, V2fMult(enemy->direction, enemySpeed));
                if (!IsPointInsideRect((V2f){0, 0}, (V2f){(f32)clientAreaSize.x, (f32)clientAreaSize.y}, enemy->pos))
                    enemy->isAlive = 0;
            }
            else if (enemy->type == Chaser)
            {
                V2f direction = V2fNormalize(V2fDiff(playerPos, enemy->pos));
                enemy->pos = V2fAdd(enemy->pos, V2fMult(direction, enemySpeed));
            }
            else if (enemy->type == WallShooter)
            {
                enemy->screenWallPosition += enemySpeed * 5;

                f32 maxPos = clientAreaSize.y * 2 + clientAreaSize.x * 2;
                if (enemy->screenWallPosition > maxPos)
                    enemy->screenWallPosition = enemy->screenWallPosition - maxPos;
            }
        }
    }
}

void DrawEnemies(GLint viewLocation, GLint colorLocation, V2f screen)
{
    for (int i = 0; i < ArrayLength(enemies); i++)
    {
        Enemy *enemy = &enemies[i];
        if (enemy->isAlive)
        {

            V2f pos;
            if (enemy->type == WallShooter)
                pos = V2fDiffScalar(BoundaryPositionToPoint(enemy->screenWallPosition, screen.x, screen.y), enemySize / 2);
            else
                pos = enemy->pos;

            if (enemy->type == Walker)
                glUniform4f(colorLocation, 1.0f, 0.3f, 0.3f, 1.0f);
            else if (enemy->type == WallShooter)
                glUniform4f(colorLocation, 0.8f, 0.5f, 0.8f, 1.0f);
            else
                glUniform4f(colorLocation, 0.3f, 0.8f, 0.8f, 1.0f);

            Mat4 view = Mat4ScaleUniform(Mat4TranslateV2f(Mat4Identity(), pos), enemySize);
            glUniformMatrix4fv(viewLocation, 1, GL_TRUE, view.values);

            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
    }
}
