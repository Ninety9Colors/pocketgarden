#pragma once
#include "raylib.h"

#include "object3d.hpp"

class Cube : public Object3d {
public:
    Cube();
    Cube(Vector3 pos, Vector3 s, Color c);

    void draw() const override;

    Vector3 position;
    Vector3 size;
    Color color;
};