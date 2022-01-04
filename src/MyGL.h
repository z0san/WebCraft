//
// Created by CHOI Jongmin on 11/11/2021.
//

#ifndef WEBCRAFT_MYGL_H
#define WEBCRAFT_MYGL_H

#include <thread>
#include <mutex>
#include "ShaderProgram.h"
#include "WorldAxes.h"
#include "Camera.h"
#include "terrain.h"
#include "player.h"
#include "Texture.h"
#include "Quad.h"
#include "FrameBuffer.h"
#include "Reticle.h"

class MyGL {

public:
    MyGL(SDL_Window *pWindow, std::vector<std::thread> &spawned_threads,
         std::mutex& blockWorkerMutex, std::deque<std::pair<int, int>>& blockWorkerCoordVector,
         std::mutex& vboWorkerMutex, std::deque<Chunk*>& vboChunkVector, std::mutex &drawChunkMutexes,
         std::deque<Chunk*> &drawChunkVector);

    void tick();

private:
    SDL_Window *window;
    WorldAxes m_worldAxes; // A wireframe representation of the world axes. It is hard-coded to sit centered at (32, 128, 32).

    ShaderProgram m_progLambert;// A shader program that uses lambertian reflection
    ShaderProgram m_progFlat;// A shader program that uses "flat" reflection (no shadowing at all)
    ShaderProgram m_progShadow;// A shader program that uses shadows
    ShaderProgram m_progPostprocess;
    ShaderProgram m_progDepth;
    ShaderProgram m_progStaticFlat;
    ShaderProgram m_progSkybox;

    FrameBuffer m_frameBuffer;
    FrameBuffer m_depthFrameBuffer;

    Quad m_geomQuad;
    Reticle m_reticle;

    int width;
    int height;

    const float PI = 3.14159265359;
    const float TWO_PI = 6.28318530718;
    const float dayCycle = 1200.0;

    Terrain m_terrain; // All of the Chunks that currently comprise the world.
    Player m_player; // The entity controlled by the user. Contains a camera to display what it sees as well.
    InputBundle bundle;
    int lastTick;

    std::unique_ptr<Texture> mp_texture;

    int m_time;

    std::vector<std::thread>& spawned_threads;

    std::mutex &blockWorkerMutex;
    std::deque<std::pair<int, int>> &blockWorkerCoordVector;

    std::mutex &vboWorkerMutex;
    std::deque<Chunk*> &vboChunkVector;

    std::mutex& drawChunkMutex;
    std::deque<Chunk*>& drawChunkVector;

    std::unordered_set<int64_t> keyMap;

    void renderTerrain(int time);

    void handleKeyPressDown(SDL_Keycode);
    void handleKeyPressUp(SDL_Keycode);

    glm::mat4 generateDepthMVP(int d, int time);

    glm::vec3 dayTime(float time);
    float modTime(float x);
};


#endif //WEBCRAFT_MYGL_H
