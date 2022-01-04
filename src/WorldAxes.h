//
// Created by CHOI Jongmin on 13/11/2021.
//

#ifndef WEBCRAFT_WORLDAXES_H
#define WEBCRAFT_WORLDAXES_H

#include "Drawable.h"

class WorldAxes : public Drawable {
public:
    WorldAxes() : Drawable(){}
    virtual ~WorldAxes() override;
    void createVBOdata() override;
    GLenum drawMode() override;
};


#endif //WEBCRAFT_WORLDAXES_H
