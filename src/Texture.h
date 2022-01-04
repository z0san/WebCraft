//
// Created by CHOI Jongmin on 20/11/2021.
//

#ifndef WEBCRAFT_TEXTURE_H
#define WEBCRAFT_TEXTURE_H

#include <memory>
#include "GLES2/gl2.h"
#include <SDL_image.h>

class Texture
{
public:
    Texture();
    ~Texture();

    void create(const char *texturePath);
    void load(int texSlot);
    void bind(int texSlot);

private:
    GLuint m_textureHandle;
    SDL_Surface* m_textureImage;
};


#endif //WEBCRAFT_TEXTURE_H
