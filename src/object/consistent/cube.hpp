#pragma once
#include <string>
#include "raylib.h"

#include "object/object3d.hpp"

class Cube : public Object3d {
public:
    Cube();
    Cube(const json& j);
    Cube(Quaternion quaternion, Vector3 position, Vector3 size, float scale, Color color);
    Cube(const Cube& rhs);
    Cube& operator=(Cube rhs) {swap(*this,rhs);return *this;}

    void generate_mesh() override;

    json to_json() const override;
    void from_json(const json& j) override;

    friend void swap(Cube& a, Cube& b) noexcept;
private:
    Vector3 size_;
    Color color_;
};