//
// Created by CHOI Jongmin on 13/11/2021.
//

#include <fstream>
#include <iostream>
#include <GLES2/gl2.h>
#include "ShaderProgram.h"

ShaderProgram::ShaderProgram()
        : vertShader(), fragShader(), prog(),
          attrPos(-1), attrNor(-1), attrCol(-1),
          unifModel(-1), unifModelInvTr(-1), unifViewProj(-1), unifColor(-1), unifEye(-1),
          unifSunset(-1), unifSunrise(-1), unifNightSky(-1), unifBlueSky(-1), unifDusk(-1),
          unifTime(-1), unifSampler2D(-1), unifSurrounding(-1), frameBufferUnifSampler2D(-1)
{}

void ShaderProgram::create(const char *vertfile, const char *fragfile)
{
    char* vertSource = textFileRead(vertfile);
    char* fragSource = textFileRead(fragfile);

    // Allocate space on our GPU for a vertex shader and a fragment shader and a shader program to manage the two
    vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, (const char**)&vertSource, nullptr);
    glCompileShader(vertShader);

    fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, (const char**)&fragSource, nullptr);
    glCompileShader(fragShader);

    prog = glCreateProgram();
    glAttachShader(prog, vertShader);
    glAttachShader(prog, fragShader);
    glLinkProgram(prog);

    attrPos = glGetAttribLocation(prog, "vs_Pos");
    attrNor = glGetAttribLocation(prog, "vs_Nor");
    attrCol = glGetAttribLocation(prog, "vs_Col");

    if(attrCol == -1) attrCol = glGetAttribLocation(prog, "vs_ColInstanced");
    attrPosOffset = glGetAttribLocation(prog, "vs_OffsetInstanced");

    unifModel      = glGetUniformLocation(prog, "u_Model");
    unifModelInvTr = glGetUniformLocation(prog, "u_ModelInvTr");
    unifDepthMVP   = glGetUniformLocation(prog, "u_DepthMVP");
    unifViewProj   = glGetUniformLocation(prog, "u_ViewProj");
    unifColor      = glGetUniformLocation(prog, "u_Color");
    unifTime       = glGetUniformLocation(prog, "u_Time");
    unifSurrounding = glGetUniformLocation(prog, "u_Surrounding");

    unifEye = glGetUniformLocation(prog, "u_Eye");

    unifSampler2D = glGetUniformLocation(prog, "u_Texture");

    // palette - OpenGL ES 2.0 doesn't enable const arrays in glsl files
    unifSunset = glGetUniformLocation(prog, "sunset");
    unifSunrise = glGetUniformLocation(prog, "sunrise");
    unifNightSky = glGetUniformLocation(prog, "nightSky");
    unifBlueSky = glGetUniformLocation(prog, "blueSky");
    unifDusk = glGetUniformLocation(prog, "dusk");

    frameBufferUnifSampler2D = glGetUniformLocation(prog, "u_RenderedTexture");




    glUseProgram(prog);
}

void ShaderProgram::useMe() {
    glUseProgram(prog);
}

void ShaderProgram::setEye(glm::vec3 p) {
    useMe();

    if(unifEye != -1){
        glUniform3f(unifEye, p.x, p.y, p.z);
    }
}

void ShaderProgram::setTime(int t) {
    useMe();

    if(unifTime != -1)
    {
        glUniform1i(unifTime, t);
    }
}

void ShaderProgram::setPalette(glm::vec3 sunset[], glm::vec3 sunrise[], glm::vec3 nightSky[], glm::vec3 blueSky[], glm::vec3 dusk[]) {
    useMe();

    if(unifSunset != -1 && unifSunrise != -1 && unifNightSky != -1 && unifBlueSky != -1) {
        glUniform3fv(unifSunset, 5, &sunset[0][0]);
        glUniform3fv(unifSunrise, 5, &sunrise[0][0]);
        glUniform3fv(unifNightSky, 5, &nightSky[0][0]);
        glUniform3fv(unifBlueSky, 5, &blueSky[0][0]);
//        glUniform3fv(unifDusk, 5, &dusk[0][0]);
    } else {
        std::cout << unifSunset << std::endl;
        std::cout << unifSunrise << std::endl;
        std::cout << unifNightSky << std::endl;
        std::cout << unifBlueSky << std::endl;
//        std::cout << unifDusk << std::endl;
    }
}

void ShaderProgram::setModelMatrix(const glm::mat4 &model)
{
    useMe();

    if (unifModel != -1) {
        glUniformMatrix4fv(unifModel, 1, GL_FALSE,&model[0][0]);
    }

    if (unifModelInvTr != -1) {
        glm::mat4 modelinvtr = glm::inverse(glm::transpose(model));
        // Pass a 4x4 matrix into a uniform variable in our shader
        // Handle to the matrix variable on the GPU
        glUniformMatrix4fv(unifModelInvTr, 1, GL_FALSE, &modelinvtr[0][0]);
    }
}

void ShaderProgram::setDepthMVP(const glm::mat4&depthMVP)
{
    useMe();
    // Compute the MVP matrix from the light's point of view

    // glm::mat4 depthMVP = glm::mat4(1.0);

    if (unifDepthMVP != -1) {
        glUniformMatrix4fv(unifDepthMVP, 1, GL_FALSE, &depthMVP[0][0]);
    }
}

void ShaderProgram::setViewProjMatrix(const glm::mat4 &vp)
{
    // Tell OpenGL to use this shader program for subsequent function calls
    useMe();

    if(unifViewProj != -1) {
        // Pass a 4x4 matrix into a uniform variable in our shader
        // Handle to the matrix variable on the GPU
        glUniformMatrix4fv(unifViewProj,1, GL_FALSE,&vp[0][0]);
    }

}

void ShaderProgram::draw(Drawable &d)
{
    useMe();

    if(d.elemCount() < 0) {
        throw std::out_of_range("Attempting to draw a drawable with m_count of " + std::to_string(d.elemCount()) + "!");
    }

    // Each of the following blocks checks that:
    //   * This shader has this attribute, and
    //   * This Drawable has a vertex buffer for this attribute.
    // If so, it binds the appropriate buffers to each attribute.

    // Remember, by calling bindPos(), we call
    // glBindBuffer on the Drawable's VBO for vertex position,
    // meaning that glVertexAttribPointer associates vs_Pos
    // (referred to by attrPos) with that VBO
    if (attrPos != -1 && d.bindPos()) {
        glEnableVertexAttribArray(attrPos);
        glVertexAttribPointer(attrPos, 4, GL_FLOAT, false, 0, NULL);
    }

    if (attrNor != -1 && d.bindNor()) {
        glEnableVertexAttribArray(attrNor);
        glVertexAttribPointer(attrNor, 4, GL_FLOAT, false, 0, NULL);
    }

    if (attrCol != -1 && d.bindCol()) {
        glEnableVertexAttribArray(attrCol);
        glVertexAttribPointer(attrCol, 4, GL_FLOAT, false, 0, NULL);
    }

    // Bind the index buffer and then draw shapes from it.
    // This invokes the shader program, which accesses the vertex buffers.
    d.bindIdx();

    glDrawElements(d.drawMode(), d.elemCount(), GL_UNSIGNED_INT, 0);

    if (attrPos != -1) glDisableVertexAttribArray(attrPos);
    if (attrNor != -1) glDisableVertexAttribArray(attrNor);
    if (attrCol != -1) glDisableVertexAttribArray(attrCol);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error " << error << ": ";
        const char *e =
                error == GL_INVALID_OPERATION             ? "GL_INVALID_OPERATION" :
                error == GL_INVALID_ENUM                  ? "GL_INVALID_ENUM" :
                error == GL_INVALID_VALUE                 ? "GL_INVALID_VALUE" :
                error == GL_INVALID_INDEX                 ? "GL_INVALID_INDEX" :
                error == GL_INVALID_OPERATION             ? "GL_INVALID_OPERATION" : "?";
        std::cerr << e << std::endl;
    }
}

void ShaderProgram::draw(Drawable &d, int test)
{
    useMe();

    if(d.elemCount() < 0) {
        throw std::out_of_range("Attempting to draw a drawable with m_count of " + std::to_string(d.elemCount()) + "!");
    }

    // Each of the following blocks checks that:
    //   * This shader has this attribute, and
    //   * This Drawable has a vertex buffer for this attribute.
    // If so, it binds the appropriate buffers to each attribute.

    // Remember, by calling bindPos(), we call
    // glBindBuffer on the Drawable's VBO for vertex position,
    // meaning that glVertexAttribPointer associates vs_Pos
    // (referred to by attrPos) with that VBO
    if (attrPos != -1 && d.bindPos()) {
        glEnableVertexAttribArray(attrPos);
        glVertexAttribPointer(attrPos, 4, GL_FLOAT, false, 0, NULL);
    }

    if (attrNor != -1 && d.bindNor()) {
        glEnableVertexAttribArray(attrNor);
        glVertexAttribPointer(attrNor, 4, GL_FLOAT, false, 0, NULL);
    }

    if (attrCol != -1 && d.bindCol()) {
        glEnableVertexAttribArray(attrCol);
        glVertexAttribPointer(attrCol, 4, GL_FLOAT, false, 0, NULL);
    }

    if(frameBufferUnifSampler2D != -1) {
        glUniform1i(frameBufferUnifSampler2D, test);
    } else {
        throw;
    }

    // Bind the index buffer and then draw shapes from it.
    // This invokes the shader program, which accesses the vertex buffers.
    d.bindIdx();

    glDrawElements(d.drawMode(), d.elemCount(), GL_UNSIGNED_INT, 0);

    if (attrPos != -1) glDisableVertexAttribArray(attrPos);
    if (attrNor != -1) glDisableVertexAttribArray(attrNor);
    if (attrCol != -1) glDisableVertexAttribArray(attrCol);

    GLenum error = glGetError();

    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error " << error << ": ";
        const char *e =
                error == GL_INVALID_OPERATION             ? "GL_INVALID_OPERATION" :
                error == GL_INVALID_ENUM                  ? "GL_INVALID_ENUM" :
                error == GL_INVALID_VALUE                 ? "GL_INVALID_VALUE" :
                error == GL_INVALID_INDEX                 ? "GL_INVALID_INDEX" :
                error == GL_INVALID_OPERATION             ? "GL_INVALID_OPERATION" : "?";
        std::cerr << e << std::endl;
    }
}

void ShaderProgram::drawChunkInterleaved(Chunk &c, bool transparent, int textureSlot) {
    useMe();

    if (c.transparentDataCount() < 0 || c.dataCount() < 0) {
        throw std::out_of_range("Attempting to draw a drawable with uninitialized count!");
    }

    if (unifSampler2D != -1) {
        glUniform1i(unifSampler2D, 0);
    }

    if(frameBufferUnifSampler2D != -1) {
        glUniform1i(frameBufferUnifSampler2D, textureSlot);
    }

    if (!transparent) {
        // printf("we should be here\n");
        if (c.bindData()) {
            // printf("doing pos\n");
            glEnableVertexAttribArray(attrPos);
            glVertexAttribPointer(attrPos, 4, GL_FLOAT, false, 3 * sizeof(glm::vec4), (void *) 0);

            // printf("doing nor\n");
            glEnableVertexAttribArray(attrNor);
            glVertexAttribPointer(attrNor, 4, GL_FLOAT, false, 3 * sizeof(glm::vec4), (void *) sizeof(glm::vec4));

            // printf("doing col\n");
            glEnableVertexAttribArray(attrCol);
            glVertexAttribPointer(attrCol, 4, GL_FLOAT, false, 3 * sizeof(glm::vec4), (void *) (2 * sizeof(glm::vec4)));

            // printf("doing all\n");
        } else {
            std::cout << "ERROR" << std::endl;
        }

        c.bindDataIdx();

        glDrawElements(c.drawMode(), c.dataCount(), GL_UNSIGNED_INT, 0);
    } else {
        //nor and color
        //nor was pseudo data
        //color was idx values

        if (c.bindTransparentData()) {
            glEnableVertexAttribArray(attrPos);
            glVertexAttribPointer(attrPos, 4, GL_FLOAT, false, 3 * sizeof(glm::vec4), (void *) 0);

            glEnableVertexAttribArray(attrNor);
            glVertexAttribPointer(attrNor, 4, GL_FLOAT, false, 3 * sizeof(glm::vec4), (void *) sizeof(glm::vec4));

            glEnableVertexAttribArray(attrCol);
            glVertexAttribPointer(attrCol, 4, GL_FLOAT, false, 3 * sizeof(glm::vec4), (void *) (2 * sizeof(glm::vec4)));
        } else {
            std::cout << "ERROR" << std::endl;
        }

        c.bindTransparentDataIdx();
        glDrawElements(c.drawMode(), c.transparentDataCount(), GL_UNSIGNED_INT, 0);
    }

//    if (attrPos != -1) glDisableVertexAttribArray(attrPos);
//    if (attrPos != -1) glDisableVertexAttribArray(attrPos);
}

void ShaderProgram::drawInterleaved(Drawable &d, bool tvbo, int tvbocount = 0)
{
    useMe();

    if(d.elemCount() < 0) {
        throw std::out_of_range("Attempting to draw a drawable with m_count of " + std::to_string(d.elemCount()) + "!");
    }

    if (unifSampler2D != -1) {
        glUniform1i(unifSampler2D, 0);
    }

    // Each of the following blocks checks that:
    //   * This shader has this attribute, and
    //   * This Drawable has a vertex buffer for this attribute.
    // If so, it binds the appropriate buffers to each attribute.

    // Remember, by calling bindPos(), we call
    // glBindBuffer on the Drawable's VBO for vertex position,
    // meaning that glVertexAttribPointer associates vs_Pos
    // (referred to by attrPos) with that VBO
    if (!tvbo) {
        if (attrPos != -1 && d.bindPos()) {
            glEnableVertexAttribArray(attrPos);
            glVertexAttribPointer(attrPos, 4, GL_FLOAT, false, 3 * sizeof(glm::vec4), (void *) 0);

            glEnableVertexAttribArray(attrNor);
            glVertexAttribPointer(attrNor, 4, GL_FLOAT, false, 3 * sizeof(glm::vec4), (void *) sizeof(glm::vec4));

            glEnableVertexAttribArray(attrCol);
            glVertexAttribPointer(attrCol, 4, GL_FLOAT, false, 3 * sizeof(glm::vec4), (void *) (2 * sizeof(glm::vec4)));
        } else {
            std::cout << "ERROR" << std::endl;
        }

        d.bindIdx();
        glDrawElements(d.drawMode(), d.elemCount(), GL_UNSIGNED_INT, 0);
    } else {
        if (attrNor != -1 && d.bindNor()) {
            glEnableVertexAttribArray(attrPos);
            glVertexAttribPointer(attrPos, 4, GL_FLOAT, false, 3 * sizeof(glm::vec4), (void *) 0);

            glEnableVertexAttribArray(attrNor);
            glVertexAttribPointer(attrNor, 4, GL_FLOAT, false, 3 * sizeof(glm::vec4), (void *) sizeof(glm::vec4));

            glEnableVertexAttribArray(attrCol);
            glVertexAttribPointer(attrCol, 4, GL_FLOAT, false, 3 * sizeof(glm::vec4), (void *) (2 * sizeof(glm::vec4)));
        } else {
            std::cout << "ERROR" << std::endl;
        }

        d.bindCol();
        glDrawElements(d.drawMode(), tvbocount, GL_UNSIGNED_INT, 0);
    }


    // Bind the index buffer and then draw shapes from it.
    // This invokes the shader program, which accesses the vertex buffers.


//    if (attrPos != -1) glDisableVertexAttribArray(attrPos);
}

void ShaderProgram::setSurrounding(int s)
{
    useMe();

    if (unifSurrounding != -1) {
        glUniform1i(unifSurrounding, s);
    }
}

char* ShaderProgram::textFileRead(const char* fileName) {
    char* text;

    if (fileName != NULL) {
        FILE *file = fopen(fileName, "rt");

        if (file != NULL) {
            fseek(file, 0, SEEK_END);
            int count = ftell(file);
            rewind(file);

            if (count > 0) {
                text = (char*)malloc(sizeof(char) * (count + 1));
                count = fread(text, sizeof(char), count, file);
                text[count] = '\0';	//cap off the string with a terminal symbol, fixed by Cory
            }
            fclose(file);
        } else {
            std::cout << "ERROR" << std::endl;
        }
    }

    return text;
}