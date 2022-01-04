//
// Created by CHOI Jongmin on 23/11/2021.
//

#ifndef WEBCRAFT_RETICLE_H
#define WEBCRAFT_RETICLE_H

#include "Drawable.h"

class Reticle : public Drawable {
public:
    Reticle() : Drawable(){}
    virtual ~Reticle() override;
    void createVBOdata() override;
    GLenum drawMode() override;
};


#endif //WEBCRAFT_RETICLE_H
