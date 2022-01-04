//
// Created by CHOI Jongmin on 23/11/2021.
//


#include <GLES2/gl2.h>
#include "Reticle.h"
#include "glm_includes.h"

Reticle::~Reticle()
{}

void Reticle::createVBOdata()
{
    GLuint idx[4] = {0, 1, 2, 3};

    float height = 0.04;
    glm::vec4 pos[4] = {glm::vec4(-height * 720.f/1280.f,0,0,1), glm::vec4(height * 720.f/1280.f,0,0,1),
                        glm::vec4(0,-height,0,1), glm::vec4(0,height,0,1)};

    glm::vec4 col[4] = {glm::vec4(1), glm::vec4(1),
                        glm::vec4(1), glm::vec4(1)};

    m_count = 4;

    generateIdx();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(GLuint), idx, GL_STATIC_DRAW);

    generatePos();
    glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec4), pos, GL_STATIC_DRAW);

    generateCol();
    glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec4), col, GL_STATIC_DRAW);
}

GLenum Reticle::drawMode()
{
    return GL_LINES;
}
