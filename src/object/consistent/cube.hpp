#pragma once
#include <string>
#include "raylib.h"

#include "object/object3d.hpp"

class Cube : public Object3d {
public:
    Cube(std::string data);
    Cube(Vector3 position, Vector3 size, float scale, Color color);
    Cube(const Cube&) = delete;
    Cube& operator=(const Cube&) = delete;

    std::string to_string() const override;
private:
    Vector3 size_;
    Color color_;
};