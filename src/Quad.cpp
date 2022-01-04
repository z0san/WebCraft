//
// Created by CHOI Jongmin on 22/11/2021.
//

#include "Quad.h"
#include "GLES2/gl2.h"

Quad::Quad() : Drawable()
{}

void Quad::createVBOdata()
{
    GLuint idx[6]{0, 1, 2, 0, 2, 3};
    glm::vec4 vert_pos[4] {glm::vec4(-1.f, -1.f, 0.99f, 1.f),
                           glm::vec4(1.f, -1.f, 0.99f, 1.f),
                           glm::vec4(1.f, 1.f, 0.99f, 1.f),
                           glm::vec4(-1.f, 1.f, 0.99f, 1.f)};

    glm::vec4 vert_UV[4] {glm::vec4(0.f, 0.f, 0.f, 0.f),
                          glm::vec4(1.f, 0.f, 0.f, 0.f),
                          glm::vec4(1.f, 1.f, 0.f, 0.f),
                          glm::vec4(0.f, 1.f, 0.f, 0.f)};

    m_count = 6;

    // Create a VBO on our GPU and store its handle in bu
    // fIdx
    generateIdx();
    // Tell OpenGL that we want to perform subsequent operations on the VBO referred to by bufIdx
    // and that it will be treated as an element array buffer (since it will contain triangle indices)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    // Pass the data stored in cyl_idx into the bound buffer, reading a number of bytes equal to
    // CYL_IDX_COUNT multiplied by the size of a GLuint. This data is sent to the GPU to be read by shader programs.
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), idx, GL_STATIC_DRAW);

    // The next few sets of function calls are basically the same as above, except bufPos and bufNor are
    // array buffers rather than element array buffers, as they store vertex attributes like position.
    generatePos();
    glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec4), vert_pos, GL_STATIC_DRAW);

    generateCol();
    glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec4), vert_UV, GL_STATIC_DRAW);
}