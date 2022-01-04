//
// Created by CHOI Jongmin on 11/11/2021.
//

#include <SDL_timer.h>
#include <SDL_events.h>
#include "MyGL.h"
#include "glm_includes.h"
#include "ShaderProgram.h"
#include "Camera.h"
#include <iostream>
#include <chrono>
#include "Texture.h"
#include "chunk.h"

int dx = 0;
int dy = 0;

#ifdef __EMSCRIPTEN__

#include <emscripten.h>
extern "C" void EMSCRIPTEN_KEEPALIVE handle_mouse_move(int x, int y) {
    dx += x;
    dy += y;
}
#endif

void blockWorkerHandler(std::deque<std::pair<int, int>> &blockWorkerCoordVector,
                        std::mutex &blockWorkermutex,
                        Terrain &m_terrain,
                        std::mutex &vboWorkerMutex,
                        std::deque<Chunk *> &vboChunkVector) {
    while (true) {
        blockWorkermutex.lock();
        if (!blockWorkerCoordVector.empty()) {
            auto &s = blockWorkerCoordVector.at(0);
            blockWorkerCoordVector.pop_front();
            Chunk *c = m_terrain.instantiateChunkAt(s.first, s.second);
            blockWorkermutex.unlock();

            vboWorkerMutex.lock();
            vboChunkVector.push_back(c);
            vboWorkerMutex.unlock();
        } else {
            blockWorkermutex.unlock();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

void vboWorkerHandler(std::deque<Chunk *> &vboChunkVector,
                      std::mutex &vboWorkerMutex,
                      std::mutex &drawChunkMutex,
                      std::deque<Chunk *> &drawChunkVector) {
    while (true) {
        vboWorkerMutex.lock();

        if (!vboChunkVector.empty()) {
            auto &c = vboChunkVector.at(0);
            vboChunkVector.pop_front();

            c->createVBOdata();
            vboWorkerMutex.unlock();
        } else {
            vboWorkerMutex.unlock();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

//
MyGL::MyGL(SDL_Window *pWindow, std::vector<std::thread> &spawned_threads,
           std::mutex &blockWorkerMutex,
           std::deque<std::pair<int, int>> &blockWorkerCoordVector,
           std::mutex &vboWorkerMutex, std::deque<Chunk *> &vboChunkVector,
           std::mutex &drawChunkMutex,
           std::deque<Chunk *> &drawChunkVector) :
        m_worldAxes(), m_progFlat(), m_terrain(), m_progDepth(),
        m_progLambert(), m_progPostprocess(), m_progStaticFlat(),
        m_player(glm::vec3(993.f, 152.f, 949.f), m_terrain),
        bundle(InputBundle()),
        lastTick(SDL_GetTicks()),
        spawned_threads(spawned_threads), blockWorkerMutex(blockWorkerMutex),
        blockWorkerCoordVector(blockWorkerCoordVector), vboWorkerMutex(vboWorkerMutex),
        vboChunkVector(vboChunkVector), drawChunkMutex(drawChunkMutex), drawChunkVector(drawChunkVector),
        m_time(0), m_geomQuad(), m_frameBuffer(), m_depthFrameBuffer(), m_reticle(), m_progSkybox() {

    int k = 0;
    this->window = pWindow;

    SDL_GetWindowSize(window, &(this->width), &(this->height));
    m_frameBuffer.setDimensions(width, height);
    m_frameBuffer.create();

    m_depthFrameBuffer.setDimensions(width, height);
    m_depthFrameBuffer.create();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    // glEnable(GL_CULL_FACE);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.37f, 0.74f, 1.0f, 1);

    m_progFlat.create("../shader/flat.vert", "../shader/flat.frag");
    m_progShadow.create("../shader/shadow.vert.glsl", "../shader/shadow.frag.glsl");
    // m_progShadow.create("../shader/depth.vert.glsl", "../shader/depth.frag.glsl");
    m_progPostprocess.create("../shader/passthrough.vert.glsl", "../shader/posteffect.frag.glsl");
    m_progDepth.create("../shader/depth.vert.glsl", "../shader/depth.frag.glsl");
    m_progStaticFlat.create("../shader/staticFlat.vert.glsl", "../shader/staticFlat.frag.glsl");
    m_progSkybox.create("../shader/passthrough.vert.glsl", "../shader/skybox.frag.glsl");

    glm::vec3 sunset[] = {glm::vec3(255, 229, 119) / 255.f,
                          glm::vec3(254, 192, 81) / 255.f,
                          glm::vec3(255, 137, 103) / 255.f,
                          glm::vec3(253, 96, 81) / 255.f,
                          glm::vec3(57, 32, 51) / 255.f};

    glm::vec3 sunrise[] = {glm::vec3(255, 224, 132) / 255.f,
                           glm::vec3(255, 192, 87) / 255.f,
                           glm::vec3(249, 139, 96) / 255.f,
                           glm::vec3(234, 89, 89) / 255.f,
                           glm::vec3(224, 36, 1) / 255.f};

    glm::vec3 nightSky[] = {glm::vec3(201, 150, 204) / 255.f,
                            glm::vec3(145, 107, 191) / 255.f,
                            glm::vec3(61, 44, 141) / 255.f,
                            glm::vec3(28, 12, 91) / 255.f,
                            glm::vec3(20, 12, 75) / 320.f};

    glm::vec3 blueSky[] = {glm::vec3(148, 210, 255) / 255.f,
                           glm::vec3(162, 210, 255) / 255.f,
                           glm::vec3(150, 186, 255) / 255.f,
                           glm::vec3(150, 186, 255) / 255.f,
                           glm::vec3(21, 151, 229) / 255.f};

    glm::vec3 dusk[] = {glm::vec3(144, 96, 144) / 255.f,
                        glm::vec3(96, 72, 120) / 255.f,
                        glm::vec3(72, 48, 120) / 255.f,
                        glm::vec3(48, 24, 96) / 255.f,
                        glm::vec3(0, 24, 72) / 255.f};

    m_progSkybox.setPalette(&sunset[0], &sunrise[0], &nightSky[0], &blueSky[0], &dusk[0]);

    m_worldAxes.createVBOdata();
    m_geomQuad.createVBOdata();
    m_reticle.createVBOdata();

    mp_texture = std::unique_ptr<Texture>(new Texture());
    mp_texture->create("../shader/minecraft_textures_all.png");
    mp_texture->load(0);

    for (int i = 0; i < 8; i++) {
        if (i < 6) {
            spawned_threads.push_back(std::thread(std::ref(blockWorkerHandler),
                                                  std::ref(blockWorkerCoordVector),
                                                  std::ref(blockWorkerMutex),
                                                  std::ref(m_terrain),
                                                  std::ref(vboWorkerMutex),
                                                  std::ref(vboChunkVector)));
        } else {
            spawned_threads.push_back(std::thread(std::ref(vboWorkerHandler),
                                                  std::ref(vboChunkVector),
                                                  std::ref(vboWorkerMutex),
                                                  std::ref(drawChunkMutex),
                                                  std::ref(drawChunkVector)));
        }
    }
}

void MyGL::renderTerrain(int time) {
    glm::vec3 pos = m_player.mcr_camera.mcr_position;
    int xCurr = 16 * static_cast<int>(glm::floor(pos[0] / 16.f));
    int zCurr = 16 * static_cast<int>(glm::floor(pos[2] / 16.f));

    int d = 7;

    blockWorkerMutex.lock();
    vboWorkerMutex.lock();
    blockWorkerCoordVector.clear();
    vboChunkVector.clear();
    glCullFace(GL_FRONT);
    glm::mat4 depthMVP = generateDepthMVP(d, time);

    m_depthFrameBuffer.bindFrameBuffer();
    m_progDepth.useMe();
    for (int d1 = -d; d1 <= d; d1++) {
        for (int d2 = -d; d2 <= d; d2++) {
            int x = xCurr + 16 * d1;
            int z = zCurr + 16 * d2;

            if (!m_terrain.hasChunkAt(x, z)) {
                blockWorkerCoordVector.push_back(std::pair<int, int>(x, z));
            } else if (!m_terrain.getChunkAt(x, z)->vboSet && std::abs(d1) <= d - 1 && std::abs(d2) <= d - 1) {
                vboChunkVector.push_back(m_terrain.getChunkAt(x, z).get());
            } else if ((m_terrain.getChunkAt(x, z)->needUpdate && std::abs(d1) <= d - 1 && std::abs(d2) <= d - 1) ||
                        (std::abs(d1) <= d - 3 && std::abs(d2) <= d - 3)) {
                m_progDepth.setModelMatrix(glm::translate(glm::mat4(),
                                                            glm::vec3(m_terrain.getChunkAt(x, z)->chunkPos.x, 0,
                                                                      m_terrain.getChunkAt(x, z)->chunkPos.z)));

                m_progDepth.setDepthMVP(depthMVP);
                m_terrain.getChunkAt(x, z)->setVBOdata();
                m_progDepth.drawChunkInterleaved(*m_terrain.getChunkAt(x, z), false, -1);
            }
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_depthFrameBuffer.bindToTextureSlot(3);
    m_frameBuffer.bindFrameBuffer();
    m_progShadow.useMe();
    glCullFace(GL_BACK);

    for (int d1 = -d; d1 <= d; d1++) {
        for (int d2 = -d; d2 <= d; d2++) {
            int x = xCurr + 16 * d1;
            int z = zCurr + 16 * d2;

            if (!m_terrain.hasChunkAt(x, z)) {
                blockWorkerCoordVector.push_back(std::pair<int, int>(x, z));
            } else if (!m_terrain.getChunkAt(x, z)->vboSet && std::abs(d1) <= d - 1 && std::abs(d2) <= d - 1) {
                vboChunkVector.push_back(m_terrain.getChunkAt(x, z).get());
            } else if ((m_terrain.getChunkAt(x, z)->needUpdate && std::abs(d1) <= d - 1 && std::abs(d2) <= d - 1) ||
                        (std::abs(d1) <= d - 3 && std::abs(d2) <= d - 3)) {
                if (m_terrain.getChunkAt(x, z)->needUpdate && std::abs(d1) <= d - 1 && std::abs(d2) <= d - 1) {
                    vboChunkVector.push_back(m_terrain.getChunkAt(x, z).get());
                    drawChunkVector.push_back(m_terrain.getChunkAt(x, z).get());
                }
                m_progShadow.setModelMatrix(glm::translate(glm::mat4(),
                                                            glm::vec3(m_terrain.getChunkAt(x, z)->chunkPos.x, 0,
                                                                        m_terrain.getChunkAt(x, z)->chunkPos.z)));

                m_progShadow.setDepthMVP(depthMVP);
                m_terrain.getChunkAt(x, z)->setVBOdata();
                m_progShadow.drawChunkInterleaved(*m_terrain.getChunkAt(x, z), false, 3);
                drawChunkVector.push_back(m_terrain.getChunkAt(x, z).get());
            }
        }
    }

    for (auto c : drawChunkVector) {
        m_progShadow.setModelMatrix(glm::translate(glm::mat4(),
                                                    glm::vec3(c->chunkPos.x, 0,
                                                              c->chunkPos.z)));
        m_progShadow.setDepthMVP(depthMVP);
        c->setTVBOdata();
        m_progShadow.drawChunkInterleaved(*c, true, 3);
    }

    drawChunkVector.clear();

    vboWorkerMutex.unlock();
    blockWorkerMutex.unlock();
}

glm::mat4 MyGL::generateDepthMVP(int d, int time) {
    // because of the sides fo the square
    // d *= 2.8285;

    glm::vec3 pos = m_player.mcr_camera.mcr_position;
    pos.y = 128;
    // glm::vec3 lightInvDir = glm::vec3(0.5f, 2, 2);
    glm::vec3 lightInvDir = dayTime(time);
    glm::mat4 depthProjectionMatrix = glm::ortho<float>(-d * 16, d * 16, -d * 16, d * 16, -d * 16, d * 16);

    glm::mat4 depthViewMatrix = glm::lookAt(pos + lightInvDir, pos, glm::vec3(0,1,0));
    glm::mat4 depthMVP = depthProjectionMatrix * depthViewMatrix;

    return depthMVP;
}

void MyGL::tick() {
    float rotationAmount = 60;
    m_player.rotateOnRightLocal(-(float) dy / height * rotationAmount);
    m_player.rotateOnUpGlobal(-(float) dx / width * rotationAmount);

    dy = 0;
    dx = 0;

    m_frameBuffer.bindFrameBuffer();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    SDL_Event e;

    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_MOUSEBUTTONDOWN:
                switch(e.button.button) {
                    case SDL_BUTTON_LEFT:
                        m_player.deleteBlock(&m_terrain);
                        break;
                    case SDL_BUTTON_RIGHT:
                        m_player.addBlock(&m_terrain);
                        break;
                    default:
                        break;
                }
                break;
            case SDL_KEYDOWN:
                handleKeyPressDown(e.key.keysym.sym);
                break;
            case SDL_KEYUP:
                handleKeyPressUp(e.key.keysym.sym);
                break;
            case SDL_QUIT:
                break;
            default:
                break;
        }
    }

    int curr = SDL_GetTicks();
    m_player.tick((curr - lastTick) / 120.0, bundle);
    lastTick = curr;

    mp_texture->bind(0);

    int temp = m_time++;
    m_progShadow.setTime(temp);

    glClearColor(0.37f, 0.74f, 1.0f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_progSkybox.setViewProjMatrix(glm::inverse(m_player.mcr_camera.getViewProj()));
    m_progSkybox.setEye(m_player.mcr_camera.mcr_position);

    glDisable(GL_DEPTH_TEST);
    m_progSkybox.setTime(temp);
    m_progSkybox.draw(m_geomQuad);
    glEnable(GL_DEPTH_TEST);

    m_progFlat.setViewProjMatrix(m_player.mcr_camera.getViewProj());

    m_progShadow.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progDepth.setViewProjMatrix(m_player.mcr_camera.getViewProj());

    m_progPostprocess.setViewProjMatrix(m_player.mcr_camera.getViewProj());

    m_depthFrameBuffer.bindFrameBuffer();
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderTerrain(temp);

    glDisable(GL_DEPTH_TEST);
    m_progFlat.setModelMatrix(glm::mat4());
    m_progFlat.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progFlat.draw(m_worldAxes);
    m_progStaticFlat.draw(m_reticle);
    glEnable(GL_DEPTH_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_frameBuffer.bindToTextureSlot(2);

    glm::vec3 pos = m_player.mcr_camera.mcr_position;

    if(m_terrain.getBlockAt(pos) == WATER)
        m_progPostprocess.setSurrounding(1);
    else if(m_terrain.getBlockAt(pos) == LAVA)
        m_progPostprocess.setSurrounding(2);
    else
        m_progPostprocess.setSurrounding(0);

    m_progPostprocess.draw(m_geomQuad, 2);
}

void MyGL::handleKeyPressUp(SDL_Keycode keyCode) {
    switch (keyCode) {
        case SDLK_w:
            bundle.wPressed = false;
            break;
        case SDLK_a:
            bundle.aPressed = false;
            break;
        case SDLK_s:
            bundle.sPressed = false;
            break;
        case SDLK_d:
            bundle.dPressed = false;
            break;
        case SDLK_SPACE:
            bundle.spacePressed = false;
            break;
        case SDLK_e:
            bundle.ePressed = false;
            break;
        case SDLK_q:
            bundle.qPressed = false;
            break;
    }
}

void MyGL::handleKeyPressDown(SDL_Keycode keyCode) {
    float amount = 2.0f;

    switch (keyCode) {
        case SDLK_w:
            bundle.wPressed = true;
            break;
        case SDLK_a:
            bundle.aPressed = true;
            break;
        case SDLK_s:
            bundle.sPressed = true;
            break;
        case SDLK_d:
            bundle.dPressed = true;
            break;
        case SDLK_SPACE:
            bundle.spacePressed = true;
            break;
        case SDLK_e:
            bundle.ePressed = true;
            break;
        case SDLK_q:
            bundle.qPressed = true;
            break;
        case SDLK_f:
            m_player.toggleSuper();
            break;
        case SDLK_LEFT:
            m_player.rotateOnUpGlobal(amount);
            break;
        case SDLK_RIGHT:
            m_player.rotateOnUpGlobal(-amount);
            break;
        case SDLK_UP:
            m_player.rotateOnRightLocal(amount);
            break;
        case SDLK_DOWN:
            m_player.rotateOnRightLocal(-amount);
            break;
        default:
            break;
    }
}

float MyGL::modTime(float x){return x - floor(x * (1.0 / dayCycle)) * dayCycle;}

glm::vec3 MyGL::dayTime(float time) {
    time = modTime(time);
    glm::vec4 phi = glm::vec4(PI/2.0, PI, 3.0*PI/2.0, TWO_PI);
    glm::vec4 theta = glm::vec4(PI/2.0, PI/12.0, PI/2.0, PI - PI/12.0);
    float t = 0.0;
    float p = 0.0;
    float interval = dayCycle/4.0;

    if(time < interval){
        t = glm::mix(theta[0], theta[1], (time) / interval);
        p = glm::mix(phi[0], phi[1], (time) / interval);
    }
    else if(time < 2.0*interval){
        t = glm::mix(theta[1], theta[2], (time - interval) / interval);
        p = glm::mix(phi[1], phi[2], (time - interval) / interval);
    }
    else if(time < 3.0*interval){
        t = glm::mix(theta[2], theta[3], (time - 2.0*interval) / interval);
        p = glm::mix(phi[2], phi[3], (time - 2.0*interval) / interval);
    }
    else{
        t = glm::mix(theta[3], theta[0], (time - 3.0*interval) / interval);
        p = glm::mix(phi[3] - TWO_PI, phi[0], (time - 3.0*interval) / interval);
    }

    return glm::vec3(sin(t)*cos(p), cos(t), sin(t)*sin(p));
}
