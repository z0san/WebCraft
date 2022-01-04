//
// Created by CHOI Jongmin on 20/11/2021.
//

#include "Texture.h"
#include <SDL_image.h>
#include <iostream>


Texture::Texture()
        : m_textureHandle(-1), m_textureImage(nullptr)
{}

Texture::~Texture()
{}

int invert_image(int width, int height, void *image_pixels) {
    auto temp_row = std::unique_ptr<char>(new char[width]);
    if (temp_row.get() == nullptr) {
        SDL_SetError("Not enough memory for image inversion");
        return -1;
    }
    //if height is odd, don't need to swap middle row
    int height_div_2 = height / 2;
    for (int index = 0; index < height_div_2; index++) {
        //uses string.h
        memcpy((Uint8 *)temp_row.get(),
               (Uint8 *)(image_pixels)+
               width * index,
               width);

        memcpy(
                (Uint8 *)(image_pixels)+
                width * index,
                (Uint8 *)(image_pixels)+
                width * (height - index - 1),
                width);
        memcpy(
                (Uint8 *)(image_pixels)+
                width * (height - index - 1),
                temp_row.get(),
                width);
    }
    return 0;
}

void Texture::create(const char *texturePath)
{
//    QImage img(texturePath);
//    img.convertToFormat(QImage::Format_ARGB32);
    SDL_Surface *TextureImage[1];


    if ( ( TextureImage[0] = IMG_Load( texturePath ) ) ) {
        /* Set the status to true */
        int format;
        SDL_Surface *formattedSurf;
        if (TextureImage[0]->format->BytesPerPixel == 3) {

            formattedSurf = SDL_ConvertSurfaceFormat(TextureImage[0],
                                                     SDL_PIXELFORMAT_RGB24,
                                                     0);
            format = GL_RGB;
        } else {
            formattedSurf = SDL_ConvertSurfaceFormat(TextureImage[0],
                                                     SDL_PIXELFORMAT_RGBA32,
                                                     0);
            format = GL_RGBA;
        }
        invert_image(formattedSurf->w * formattedSurf->format->BytesPerPixel, formattedSurf->h,
                     (char *) formattedSurf->pixels);

        m_textureImage = formattedSurf;
    }

    glGenTextures(1, &m_textureHandle);
}

void Texture::load(int texSlot = 0)
{
    glActiveTexture(GL_TEXTURE0 + texSlot);
    glBindTexture(GL_TEXTURE_2D, m_textureHandle);

    // These parameters need to be set for EVERY texture you create
    // They don't always have to be set to the values given here, but they do need
    // to be set
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
//                          256, 256,
//                          0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, formattedSurf->pixels);

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, m_textureImage->w,
                  m_textureImage->h, 0, GL_RGBA,
                  GL_UNSIGNED_BYTE, m_textureImage->pixels );
}


void Texture::bind(int texSlot = 0)
{
    glActiveTexture(GL_TEXTURE0 + texSlot);
    glBindTexture(GL_TEXTURE_2D, m_textureHandle);
}