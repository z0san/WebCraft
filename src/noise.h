#ifndef NOISE_H
#define NOISE_H

#define NUM_OCTAVES 5
#include <iostream>
#include <queue>
#include <vector>

#include "glm_includes.h"

class Noise
{
public:
    Noise();

    static float Perlin(glm::vec2 p);
    static float noiseFix(glm::vec2 p);
    static float noise(float p);
    static float noiseFix2(glm::vec2 p);
    static float fbm(float x);
    static float WorleyNoise(glm::vec2 uv, std::vector<float> coeff={1, -1});
    static float perlinNoise(glm::vec2 uv);
    static float perlinNoise3D(glm::vec3 uvw);
};

#endif // NOISE_H
