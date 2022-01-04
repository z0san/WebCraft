#include "player.h"
#include <iostream>
#include <limits>
//
Player::Player(glm::vec3 pos, const Terrain &terrain)
    : Entity(pos), m_velocity(0,0,0), m_acceleration(0,0,0),
      m_camera(pos + glm::vec3(0, 1.5f, 0)), mcr_terrain(terrain),
      mcr_camera(m_camera), isStanding(false), superMode(true),
      rightLocalRotation(0), inWater(false)
{}

Player::~Player()
{}

void Player::tick(float dT, InputBundle &input) {
    processInputs(input);
    computePhysics(dT, mcr_terrain);
    rotateOnUpGlobal(-input.upRotation);
    rotateOnRightLocal(-input.rightRotation);
}

void Player::processInputs(InputBundle &inputs) {
    m_acceleration = glm::vec3(0);

    glm::vec3 norm_forward = glm::normalize(glm::vec3(m_forward.x, 0, m_forward.z));

    m_acceleration += norm_forward * (float) inputs.wPressed;
    m_acceleration += m_right * (float) inputs.dPressed;
    m_acceleration -= norm_forward * (float) inputs.sPressed;
    m_acceleration -= m_right * (float) inputs.aPressed;

    if (superMode) {
        m_acceleration += glm::vec3(0, 0.7, 0) * (float) inputs.ePressed;
        m_acceleration -= glm::vec3(0, 0.7, 0) * (float) inputs.qPressed;
    } else {
        if(!inWater)
            m_acceleration += glm::vec3(0, -1.0, 0);
        else
            m_acceleration += glm::vec3(0, -0.5, 0);

        if (inWater && inputs.spacePressed) m_velocity[1] = 0.6;
        else if (isStanding && inputs.spacePressed) m_velocity[1] = 2.0;
    }
}

bool gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection, const Terrain &terrain, float *out_dist, glm::ivec3 *out_blockHit, float *interfaceAxis) { // , glm::ivec3 *outBlockNeighbor) {
    float maxLen = glm::length(rayDirection); // Farthest we search
    glm::ivec3 currCell = glm::ivec3(glm::floor(rayOrigin));
    rayDirection = glm::normalize(rayDirection); // Now all t values represent world dist.

    float curr_t = 0.f;
    bool firstIteration = true;

    while(curr_t < maxLen) {
        float min_t = glm::sqrt(3.f);
//        float interfaceAxis = -1; // Track axis for which t is smallest
        *interfaceAxis = -1;
        for(int i = 0; i < 3; ++i) { // Iterate over the three axes
            if(rayDirection[i] != 0) { // Is ray parallel to axis i?
                float offset = glm::max(0.f, glm::sign(rayDirection[i])); // See slide 5

                if (firstIteration && currCell[i] == rayOrigin[i] && offset == 1.f) {
                    offset = 0.f;
                    firstIteration = false;
                }

                int nextIntercept = currCell[i] + offset;
                float axis_t = (nextIntercept - rayOrigin[i]) / rayDirection[i];
                axis_t = glm::min(axis_t, maxLen); // Clamp to max len to avoid super out of bounds errors
                if(axis_t < min_t) {
                    min_t = axis_t;
                    *interfaceAxis = i;
                }
            }
        }

        if(*interfaceAxis == -1) {
            throw std::out_of_range("interfaceAxis was -1 after the for loop in gridMarch!");
        }
        curr_t += min_t; // min_t is declared in slide 7 algorithm
        rayOrigin += rayDirection * min_t;
        glm::ivec3 offset = glm::ivec3(0,0,0);
        // Sets it to 0 if sign is +, -1 if sign is -
        offset[*interfaceAxis] = glm::min(0.f, glm::sign(rayDirection[*interfaceAxis]));
        currCell = glm::ivec3(glm::floor(rayOrigin)) + offset;
        // If currCell contains something other than EMPTY, return
        BlockType cellType = terrain.getBlockAt(currCell.x, currCell.y, currCell.z);

        if(cellType != EMPTY && cellType != WATER && cellType != LAVA) {
            *out_blockHit = currCell;
            *out_dist = glm::min(maxLen, curr_t);
            return true;
        }
    }
    *out_dist = glm::min(maxLen, curr_t);
    return false;
}

void Player::deleteBlock(Terrain *terrain) {
    glm::ivec3 block;
    float axis;
    float distance;

    if (gridMarch(m_camera.mcr_position, m_forward * 3.f, *terrain, &distance, &block, &axis)) {
        terrain->setBlockAt(block.x, block.y, block.z, EMPTY);
        Chunk* chunk = terrain->getChunkAt(block.x, block.z).get();
        chunk->needUpdate = true;
        for (int x = -1; x < 2; x++) {
            for (int z = -1; z < 2; z++) {
                if (terrain->getChunkAt(block.x + x, block.z + z).get() != chunk) {
                    terrain->getChunkAt(block.x + x, block.z + z)->needUpdate = true;
                }
            }
        }
    }
}

void Player::addBlock(Terrain *terrain) {
    glm::ivec3 block;
    float axis;
    float distance;

    if (gridMarch(m_camera.mcr_position, m_forward * 3.f, *terrain, &distance, &block, &axis)) {
        glm::ivec3 axisVec = glm::ivec3();
        axisVec[axis] = 1;
        glm::ivec3 newBlock = block + (int) -glm::sign(m_forward[axis]) * axisVec;

        terrain->setBlockAt(newBlock.x, newBlock.y, newBlock.z, STONE);
        terrain->getChunkAt(newBlock.x, newBlock.z)->needUpdate = true;
    }
}

void Player::computePhysics(float dT, const Terrain &terrain) {
    m_velocity *= glm::vec3(0.65, 0.99, 0.65);

    if(terrain.getBlockAt(this->mcr_position) == WATER || terrain.getBlockAt(this->mcr_position) == LAVA){
        m_velocity *= 0.5;
        inWater = true;
    } else {
        inWater = false;
    }

    m_velocity += m_acceleration * dT;

    if (superMode) {
        moveAlongVector(m_velocity * dT);
    } else {
        float delta[4][2] = {{0.45, 0.45}, {-0.45, 0.45}, {-0.45, -0.45}, {0.45, -0.45}};
        glm::vec3 ray = m_velocity * dT;

        for (int axis = 0; axis < 3; axis++) {
            for (int level = 0; level < 2; level++) {
                for (auto d : delta) {
                    glm::vec3 cornerLoc = m_position + d[0] * glm::vec3(1, 0, 0) + d[1] * glm::vec3(0, 0, 1) + (float) level * glm::vec3(0, 1, 0);

                    float distance = 0;
                    glm::ivec3 out_block = glm::ivec3();


                    glm::vec3 segmentedVelocity = glm::vec3();
                    segmentedVelocity[axis] = ray[axis];
                    float axis = -1;

                    if (gridMarch(cornerLoc, segmentedVelocity, terrain, &distance, &out_block, &axis)) {
                        ray[axis] = glm::sign(ray[axis]) * glm::min((std::abs(distance) - 0.01f), std::abs(ray[axis]));

                        if (std::abs(ray[axis]) < 0.01) {
                            m_velocity[axis] = 0;
                            ray[axis] = 0;
                        }


                    }
                }
            }
        }


        isStanding = (m_velocity[1] == 0);

        moveAlongVector(ray);
    }

}

void Player::setCameraWidthHeight(unsigned int w, unsigned int h) {
    m_camera.setWidthHeight(w, h);
}

void Player::moveAlongVector(glm::vec3 dir) {
    Entity::moveAlongVector(dir);
    m_camera.moveAlongVector(dir);
}
void Player::moveForwardLocal(float amount) {
    Entity::moveForwardLocal(amount);
    m_camera.moveForwardLocal(amount);
}
void Player::moveRightLocal(float amount) {
    Entity::moveRightLocal(amount);
    m_camera.moveRightLocal(amount);
}
void Player::moveUpLocal(float amount) {
    Entity::moveUpLocal(amount);
    m_camera.moveUpLocal(amount);
}
void Player::moveForwardGlobal(float amount) {
    Entity::moveForwardGlobal(amount);
    m_camera.moveForwardGlobal(amount);
}
void Player::moveRightGlobal(float amount) {
    Entity::moveRightGlobal(amount);
    m_camera.moveRightGlobal(amount);
}
void Player::moveUpGlobal(float amount) {
    Entity::moveUpGlobal(amount);
    m_camera.moveUpGlobal(amount);
}
void Player::rotateOnForwardLocal(float degrees) {
    Entity::rotateOnForwardLocal(degrees);
    m_camera.rotateOnForwardLocal(degrees);
}
void Player::rotateOnRightLocal(float degrees) {
    if (std::abs(rightLocalRotation + degrees) < 80)  {
        rightLocalRotation += degrees;
        Entity::rotateOnRightLocal(degrees);
        m_camera.rotateOnRightLocal(degrees);
    }
}

void Player::rotateOnUpLocal(float degrees) {
    Entity::rotateOnUpLocal(degrees);
    m_camera.rotateOnUpLocal(degrees);
}
void Player::rotateOnForwardGlobal(float degrees) {
    Entity::rotateOnForwardGlobal(degrees);
    m_camera.rotateOnForwardGlobal(degrees);
}
void Player::rotateOnRightGlobal(float degrees) {
    Entity::rotateOnRightGlobal(degrees);
    m_camera.rotateOnRightGlobal(degrees);
}
void Player::rotateOnUpGlobal(float degrees) {
    Entity::rotateOnUpGlobal(degrees);
    m_camera.rotateOnUpGlobal(degrees);
}

void Player::toggleSuper() {
    superMode = !superMode;
    m_velocity[1] = 0;
}