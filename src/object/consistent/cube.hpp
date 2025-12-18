#pragma once
#include <string>
#include "raylib.h"

#include "object/object3d.hpp"

class Cube : public Object3d {
public:
    Cube();
    Cube(std::string data);
    Cube(Quaternion quaternion, Vector3 position, Vector3 size, float scale, Color color);

    std::string to_string() const override;
    friend void swap(Cube& a, Cube& b) noexcept;
private:
    Vector3 size_;
    Color color_;
};