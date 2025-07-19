#pragma once
#include "raylib.h"

#include "object3d.hpp"

class Cube : public Object3d {
public:
    Cube();
    Cube(Vector3 pos, Vector3 s, Color c);

    void draw() const override;
    void draw_outline() const override;

    void set_x(float new_x) override;
    void set_y(float new_y) override;
    void set_z(float new_z) override;
    float get_x() const override;
    float get_y() const override;
    float get_z() const override;
private:
    Vector3 position_;
    Vector3 size_;
    Color color_;
};