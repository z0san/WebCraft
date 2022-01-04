////
//// Created by CHOI Jongmin on 22/11/2021.
////
//
#include "FrameBuffer.h"
#include <iostream>
#include <GLES2/gl2.h>

void checkGlError(std::string description){
    GLenum error = glGetError();
    while(error != GL_NO_ERROR)
    {
        switch(error){
            case(GL_NO_ERROR):

                break;
            case(GL_INVALID_ENUM):
                std::cout << description << ": GL_INVALID_ENUM" << std::endl;
                break;
            case(GL_INVALID_VALUE):
                std::cout << description << ": GL_INVALID_VALUE" << std::endl;
                break;
            case(GL_INVALID_OPERATION):
                std::cout << description << ": GL_INVALID_OPERATION" << std::endl;
                break;
            case(GL_INVALID_FRAMEBUFFER_OPERATION):
                std::cout << description << ": GL_INVALID_FRAMEBUFFER_OPERATION" << std::endl;
                break;
            case(GL_OUT_OF_MEMORY):
                std::cout << description << ": GL_OUT_OF_MEMORY" << std::endl;
                break;
            default:
                std::cout << description << ": Unknown error!" << std::endl;

        }
        error = glGetError();

    }
}

FrameBuffer::FrameBuffer()
        : m_frameBuffer(-1),
          m_outputTexture(-1), m_depthRenderBuffer(-1)
{}

void FrameBuffer::setDimensions(unsigned int width, unsigned int height) {
    m_width = width;
    m_height = height;
}

void FrameBuffer::create() {
    // Initialize the frame buffers and render textures
    glGenFramebuffers(1, &m_frameBuffer);
    glGenTextures(1, &m_outputTexture);
    glGenRenderbuffers(1, &m_depthRenderBuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    // Bind our texture so that all functions that deal with textures will interact with this one
    glBindTexture(GL_TEXTURE_2D, m_outputTexture);
    // Give an empty image to OpenGL ( the last "0" )
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_FLOAT, (void*)0);

    // Set the render settings for the texture we've just created.
    // Essentially zero filtering on the "texture" so it appears exactly as rendered
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // Clamp the colors at the edge of our texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Initialize our depth buffer
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_width, m_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderBuffer);

    // Set m_renderedTexture as the color output of our frame buffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_outputTexture, 0);
    checkGlError("glFramebufferTexture2D");

    // Sets the color output of the fragment shader to be stored in GL_COLOR_ATTACHMENT0,
    // which we previously set to m_renderedTexture
    // if (!drawBuff) {
    //     GLenum drawBuffers[1] = {GL_NONE};
    //     glDrawBuffers(1, drawBuffers); // "1" is the size of drawBuffers
    // }
//
    m_created = true;
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        m_created = false;
        std::cout << "Frame buffer did not initialize correctly..." << std::endl;
        throw std::out_of_range("invalid frame buffer");
    }
}

void FrameBuffer::destroy() {
    if(m_created) {
        m_created = false;
        glDeleteFramebuffers(1, &m_frameBuffer);
        glDeleteTextures(1, &m_outputTexture);
        glDeleteRenderbuffers(1, &m_depthRenderBuffer);
    }
}

void FrameBuffer::bindFrameBuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
}

void FrameBuffer::bindToTextureSlot(unsigned int slot) {
    m_textureSlot = slot;
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_outputTexture);
}

unsigned int FrameBuffer::getTextureSlot() const {
    return m_textureSlot;
}