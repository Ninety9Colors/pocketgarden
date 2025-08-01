#pragma once
#include <string>
#include "raylib.h"

#include "object3d.hpp"

class Cube : public Object3d {
public:
    Cube();
    Cube(std::string data);
    Cube(Vector3 position, Vector3 size, float scale, Color color);

    void draw() const override;
    void draw_offset(float x, float y, float z) const override;
    void set_shader(std::shared_ptr<Shader> shader) override;

    void set_x(float new_x) override;
    void set_y(float new_y) override;
    void set_z(float new_z) override;
    float get_x() const override;
    float get_y() const override;
    float get_z() const override;

    BoundingBox get_bounding_box() const override;

    std::string to_string() const override;
private:
    Model model_;
    Vector3 position_;
    Vector3 size_;
    float scale_;
    Color color_;
};