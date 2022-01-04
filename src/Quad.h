//
// Created by CHOI Jongmin on 22/11/2021.
//

#ifndef WEBCRAFT_QUAD_H
#define WEBCRAFT_QUAD_H

#include "Drawable.h"

class Quad : public Drawable
{
public:
    Quad();
    virtual void createVBOdata();
};

#endif //WEBCRAFT_QUAD_H
