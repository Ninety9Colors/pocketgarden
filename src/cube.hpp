#pragma once
#include <string>
#include "raylib.h"

#include "object3d.hpp"

class Cube : public Object3d {
public:
    Cube(std::string data);
    Cube(Vector3 position, Vector3 size, float scale, Color color);
    Cube(const Cube&) = delete;
    Cube& operator=(const Cube&) = delete;
    ~Cube();

    void draw() const override;
    void draw_offset(float x, float y, float z) const override;
    void set_shader(std::shared_ptr<Shader> shader) override;

    void set_position(Vector3 position) override;
    Vector3 get_position() const override;

    BoundingBox get_bounding_box() const override;

    std::string to_string() const override;
private:
    Mesh mesh_;
    Material material_;
    Vector3 size_;
    float scale_;
    Color color_;
    Matrix transform_;
};