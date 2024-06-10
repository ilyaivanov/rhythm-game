#pragma once
#include <gl/gl.h>
#include "utils/all.c"

typedef struct Particle
{
    i32 isAlive;
    V2f position;
    V2f direction;
    f32 speed;
    f32 secToDie;
    V3f color;
} Particle;

Particle particles[1024];

f32 particleSize = 10.0f;
f32 timeToLife = 2.0f;
RandomSeries particleRandom;

void InitParticles()
{
    particleRandom = CreateSeries();
}

inline f32 RandomShift()
{
    return RandomF32Normal(&particleRandom) * 0.01f - 0.005f;
}

void Burst(V2f from, V3f color)
{
    i32 count = 30;
    for (int i = 0; i < ArrayLength(particles); i++)
    {
        Particle *particle = &particles[i];

        if (!particle->isAlive)
        {
            particle->isAlive = 1;
            particle->color = color;

            particle->direction = RandomUnitVector(&particleRandom);
            particle->position = V2fAdd(from, V2fMult(particle->direction, 5.0f));
            particle->speed = RandomF32(&particleRandom, 0.4f, 0.8f);
            particle->secToDie = timeToLife;

            count--;
            if (count <= 0)
                break;
        }
    }
}

void UpdateParticles()
{
    for (int i = 0; i < ArrayLength(particles); i++)
    {
        Particle *particle = &particles[i];

        if (particle->isAlive)
        {
            particle->position = V2fAdd(particle->position, V2fMult(particle->direction, particle->speed));
            particle->secToDie -= 16.6666f / 1000.0f;

            if (particle->secToDie < 0)
                particle->isAlive = 0;
        }
    }
}

void DrawParticles(GLint viewLocation, GLint colorLocation)
{
    for (int i = 0; i < ArrayLength(particles); i++)
    {
        Particle *particle = &particles[i];

        if (particle->isAlive)
        {
            V3f color = particle->color;
            glUniform4f(colorLocation, color.x, color.y, color.z, particle->secToDie / timeToLife);

            Mat4 view = Mat4ScaleUniform(Mat4TranslateV2f(Mat4Identity(), V2fDiffScalar(particle->position, particleSize / 2)), particleSize);
            glUniformMatrix4fv(viewLocation, 1, GL_TRUE, view.values);

            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
    }
}