#include "noise.h"

glm::vec2 fade(glm::vec2 t){
    return t*t*t*(t*(t * 6.f + 15.f)-10.f);
}

glm::vec4 permute(glm::vec4 x){return glm::mod(((x*34.f)+1.f)*x, 289.f);}

float rand(glm::vec2 n) {
    return glm::fract(glm::sin(glm::dot(n, glm::vec2(12.9898, 4.1414))) * 43758.5453);
}

float rand(float n){return glm::fract(sin(n) * 43758.5453123);}

float rand(glm::vec3 n) {
    return glm::fract(sin(glm::dot(n, glm::vec3(12.9898, 4.1414, 9.1498))) * 43758.5453);
}

glm::vec2 random2(glm::vec2 p ) {
    glm::vec2 point =  glm::fract(glm::sin(glm::vec2(glm::dot(p, glm::vec2(127.1, 311.7)),
                                                     glm::dot(p, glm::vec2(269.5,183.3))))
                                  * 43758.5453f);

    return point;
}

glm::vec3 random3(glm::vec3 p) {
    glm::vec3 point =  glm::fract(glm::sin(glm::vec3(glm::dot(p, glm::vec3(115.2f, 127.1f, 311.7f)),
                                                     glm::dot(p, glm::vec3(189.6, 269.5,183.3)),
                                                     glm::dot(p, glm::vec3(127.2, 84.1, 475.9))))
                                  * 43758.5453f);

    return point;
}

Noise::Noise()
{

}

float Noise::noiseFix2(glm::vec2 n) {
    const glm::vec2 d = glm::vec2(0.0, 1.0);
    glm::vec2 b = glm::floor(n), f = glm::smoothstep(glm::vec2(0.0), glm::vec2(1.0), glm::fract(n));
    return glm::mix(glm::mix(rand(b), rand(b + glm::vec2(d.y, d.x)), f.x), glm::mix(rand(b + glm::vec2(d.x, d.y)), rand(b + glm::vec2(d.y, d.y)), f.x), f.y);
}

float Noise::noise(float p){
    float fl = glm::floor(p);
    float fc = glm::fract(p);
    return glm::mix(rand(fl), rand(fl + 1.0), fc);
}

float Noise::noiseFix(glm::vec2 p){
    glm::vec2 ip = glm::floor(p);
    glm::vec2 u = glm::fract(p);
    u = u*u*(3.f-2.f * u);

    float res = glm::mix(
            glm::mix(rand(ip),rand(ip+glm::vec2(1.0,0.0)),u.x),
            glm::mix(rand(ip+glm::vec2(0.0,1.0)),rand(ip+glm::vec2(1.0,1.0)),u.x),u.y);
    return res*res;
}

float surflet(glm::vec2 P, glm::vec2 gridPoint) {
    // Compute falloff function by converting linear distance to a polynomial
    float distX = abs(P.x - gridPoint.x);
    float distY = abs(P.y - gridPoint.y);
    float tX = 1 - 6 * pow(distX, 5.f) + 15 * pow(distX, 4.f) - 10 * pow(distX, 3.f);
    float tY = 1 - 6 * pow(distY, 5.f) + 15 * pow(distY, 4.f) - 10 * pow(distY, 3.f);
    // Get the random vector for the grid point
    glm::vec2 gradient = random2(gridPoint);
    // Get the vector from the grid point to P
    glm::vec2 diff = P - gridPoint;
    // Get the value of our height field by dotting grid->P with our gradient
    float height = glm::dot(diff, gradient);
    // Scale our height field (i.e. reduce it) by our polynomial falloff function
    return height * tX * tY;
}




float Noise::perlinNoise(glm::vec2 uv) {
    float surfletSum = 0.f;
    // Iterate over the four integer corners surrounding uv
    for(int dx = 0; dx <= 1; dx++) {
        for(int dy = 0; dy <= 1; dy++) {
            surfletSum += surflet(uv, glm::floor(uv) + glm::vec2(dx, dy));
        }
    }
    return surfletSum;
}

float surflet(glm::vec3 p, glm::vec3 gridPoint) {
    // Compute the distance between p and the grid point along each axis, and warp it with a
    // quintic function so we can smooth our cells
    glm::vec3 t2 = glm::abs(p - gridPoint);

    glm::vec3 t = glm::vec3(1.f) - 6.f * glm::pow(t2, glm::vec3(5.f, 5.f, 5.f)) +
                  15.f * glm::pow(t2, glm::vec3(4.f, 4.f, 4.f)) -
                  10.f * glm::pow(t2, glm::vec3(3.f, 3.f, 3.f));
    // Get the random vector for the grid point (assume we wrote a function random2
    // that returns a vec2 in the range [0, 1])
    glm::vec3 gradient = random3(gridPoint) * 2.f - glm::vec3(1., 1., 1.);
    // Get the vector from the grid point to P
    glm::vec3 diff = p - gridPoint;
    // Get the value of our height field by dotting grid->P with our gradient
    float height = glm::dot(diff, gradient);
    // Scale our height field (i.e. reduce it) by our polynomial falloff function
    return height * t.x * t.y * t.z;
}


float Noise::perlinNoise3D(glm::vec3 uvw) {
    float surfletSum = 0.f;
    // Iterate over the four integer corners surrounding uv
    for(int dx = 0; dx <= 1; dx++) {
        for(int dy = 0; dy <= 1; dy++) {
            for(int dz = 0; dz <= 1; dz++){
                surfletSum += surflet(uvw, glm::floor(uvw) + glm::vec3(dx, dy, dz));
            }

        }
    }
    return surfletSum;
}

float Noise::Perlin(glm::vec2 P){
    return glm::length(P);
}

float Noise::fbm(float x) {
    float v = 0.0;
    float a = 0.5;
    float shift = float(100);
    for (int i = 0; i < NUM_OCTAVES; ++i) {
        v += a * Noise::noise(x);
        x = x * 2.0 + shift;
        a *= 0.5;
    }
    return v;
}




float Noise::WorleyNoise(glm::vec2 uv, std::vector<float> coeff) {
    float n = 10.0;
    uv *= n; // Now the space is 10x10 instead of 1x1. Change this to any number you want.

    glm::vec2 uvInt = glm::floor(uv);
    glm::vec2 uvFract = glm::fract(uv);

    std::priority_queue<float, std::vector<float>, std::greater<float>> distances;
    glm::vec2 minPoint;
    for(int y = -1; y <= 1; ++y) {
        for(int x = -1; x <= 1; ++x) {
            glm::vec2 neighbor = glm::vec2(float(x), float(y)); // Direction in which neighbor cell lies
            glm::vec2 point = random2(uvInt + neighbor); // Get the Voronoi centerpoint for the neighboring cell
            glm::vec2 diff = neighbor + point - uvFract; // Distance between fragment coord and neighbor’s Voronoi point
            float dist = glm::length(diff);

            distances.push(dist);
        }
    }

    float res = 0;
    if(!coeff.empty())
        res = coeff[0];
    for(unsigned int i = 1; i < coeff.size(); i++){
        if(distances.empty())
            break;
        res += coeff[i] * distances.top();
        distances.pop();
    }

    return res;
}