//
// Created by CHOI Jongmin on 13/11/2021.
//

#ifndef WEBCRAFT_SHADERPROGRAM_H
#define WEBCRAFT_SHADERPROGRAM_H

#include <GLES2/gl2.h>
#include "glm_includes.h"
#include "Drawable.h"
#include "chunk.h"

class ShaderProgram {
public:
    GLuint vertShader; // A handle for the vertex shader stored in this shader program
    GLuint fragShader; // A handle for the fragment shader stored in this shader program
    GLuint prog;       // A handle for the linked shader program stored in this class

    int attrPos; // A handle for the "in" vec4 representing vertex position in the vertex shader
    int attrNor; // A handle for the "in" vec4 representing vertex normal in the vertex shader
    int attrCol; // A handle for the "in" vec4 representing vertex color in the vertex shader
    int attrPosOffset; // A handle for a vec3 used only in the instanced rendering shader

    int unifModel; // A handle for the "uniform" mat4 representing model matrix in the vertex shader
    int unifModelInvTr; // A handle for the "uniform" mat4 representing inverse transpose of the model matrix in the vertex shader
    int unifDepthMVP;  // A handle for the depth mvp
    int unifViewProj; // A handle for the "uniform" mat4 representing combined projection and view matrices in the vertex shader
    int unifColor; // A handle for the "uniform" vec4 representing color of geometry in the vertex shader

    int unifEye;

    int unifTime;
    int unifSampler2D;

    int unifSunset;
    int unifSunrise;
    int unifNightSky;
    int unifBlueSky;
    int unifDusk;

    int frameBufferUnifSampler2D;

    int unifSurrounding;

public:
    ShaderProgram();
    // Sets up the requisite GL data and shaders from the given .glsl files
    void create(const char *vertfile, const char *fragfile);
    // Tells our OpenGL context to use this shader to draw things
    void useMe();
    // Pass the given model matrix to this shader on the GPU
    void setModelMatrix(const glm::mat4 &model);
    // Pass the given depth mvp matrix to this shader on the GPU
    void setDepthMVP(const glm::mat4&depthMVP);
    // Pass the given Projection * View matrix to this shader on the GPU
    void setViewProjMatrix(const glm::mat4 &vp);

    
    // Pass the given color to this shader on the GPU
//    void setGeometryColor(glm::vec4 color);
    // Draw the given object to our screen using this ShaderProgram's shaders
    void draw(Drawable &d);
    void draw(Drawable &d, int test);

    void drawInterleaved(Drawable &d, bool tvbo, int);
    void drawChunkInterleaved(Chunk &c, bool transparent, int textureSlot);

    void setTime(int t);
    void setSurrounding(int s);
    void setPalette(glm::vec3 sunset[], glm::vec3 sunrise[], glm::vec3 nightSky[], glm::vec3 blueSky[], glm::vec3 dusk[]);

    void setEye(glm::vec3 eye);

    void drawInstanced(InstancedDrawable &d);

    // Utility function used in create()
    char* textFileRead(const char*);
};


#endif //WEBCRAFT_SHADERPROGRAM_H
