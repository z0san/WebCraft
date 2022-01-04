#include <functional>
#include <glm/vec3.hpp>
#include <iostream>

#ifdef __EMSCRIPTEN__

#include <emscripten.h>
#include <SDL2/SDL.h>
#include <GLES2/gl2.h>
#include "MyGL.h"
#include <thread>
#include <mutex>
#include <vector>
#include <array>

#else

#include <SDL.h>
#include <SDL_opengles2.h>
#include "MyGL.h"

#endif

std::function<void()> loop;
void main_loop() { loop(); }

bool quit = false;

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_Window *window;

    window = SDL_CreateWindow(
            "An SDL2 window",                  // window title
            SDL_WINDOWPOS_CENTERED,           // initial x position
            SDL_WINDOWPOS_CENTERED,           // initial y position
            1280,                               // width, in pixels
            720,                               // height, in pixels
            SDL_WINDOW_OPENGL // flags - see below
    );

    SDL_GLContext glContext = SDL_GL_CreateContext(window);


    std::vector<std::thread> spawned_threads;

    std::mutex blockWorkerMutex;
    std::deque<std::pair<int, int>> blockWorkerCoordVectorArray;

    std::mutex vboWorkerMutex;
    std::deque<Chunk*> vboChunkVectorArray;

    std::mutex drawChunkMutex;
    std::deque<Chunk*> drawChunkVector;

    MyGL myGL = MyGL(window, spawned_threads, blockWorkerMutex, blockWorkerCoordVectorArray, vboWorkerMutex, vboChunkVectorArray,
            drawChunkMutex, drawChunkVector);

    loop = [&] {
        myGL.tick();
    };

#if __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, true);
#else
    while (!quit) {
        main_loop();
    }
#endif

    return EXIT_SUCCESS;
}