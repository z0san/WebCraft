//
// Created by CHOI Jongmin on 13/11/2021.
//

#ifndef WEBCRAFT_ENTITY_H
#define WEBCRAFT_ENTITY_H

#include "glm_includes.h"

struct InputBundle {
    bool wPressed, aPressed, sPressed, dPressed, qPressed, ePressed;
    bool spacePressed;
    float rightRotation, upRotation;

    InputBundle()
            : wPressed(false), aPressed(false), sPressed(false),
              dPressed(false), qPressed(false), ePressed(false),
              spacePressed(false), rightRotation(0.f), upRotation(0.f)
    {}
};

class Entity {
public:
    // Vectors that define the axes of our local coordinate system
    glm::vec3 m_forward, m_right, m_up;
    // The origin of our local coordinate system
    glm::vec3 m_position;

    // A readonly reference to position for external use
    const glm::vec3& mcr_position;

    // Various constructors
    Entity();
    Entity(glm::vec3 pos);
    Entity(const Entity &e);
    virtual ~Entity();

    // To be called by MyGL::tick()
    virtual void tick(float dT, InputBundle &input) = 0;

    // Translate along the given vector
    virtual void moveAlongVector(glm::vec3 dir);

    // Translate along one of our local axes
    virtual void moveForwardLocal(float amount);
    virtual void moveRightLocal(float amount);
    virtual void moveUpLocal(float amount);

    // Translate along one of the world axes
    virtual void moveForwardGlobal(float amount);
    virtual void moveRightGlobal(float amount);
    virtual void moveUpGlobal(float amount);

    // Rotate about one of our local axes
    virtual void rotateOnForwardLocal(float degrees);
    virtual void rotateOnRightLocal(float degrees);
    virtual void rotateOnUpLocal(float degrees);

    // Rotate about one of the world axes
    virtual void rotateOnForwardGlobal(float degrees);
    virtual void rotateOnRightGlobal(float degrees);
    virtual void rotateOnUpGlobal(float degrees);
};
#endif //WEBCRAFT_ENTITY_H
