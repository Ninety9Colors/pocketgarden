#include "cube.hpp"

Cube::Cube() : position_{0.0f,0.0f,0.0f}, size_{0.0f,0.0f,0.0f}, color_(WHITE){};
Cube::Cube(Vector3 position, Vector3 size, Color color) : position_(position), size_(size), color_(color) {};

void Cube::draw() const {
    DrawCubeV(position_, size_, color_);
    draw_outline();
};

void Cube::draw_outline() const {
    DrawCubeWiresV(position_, size_, WHITE);
};

void Cube::set_x(float new_x) {
    position_.x = new_x;
}

void Cube::set_y(float new_y) {
    position_.y = new_y;
}

void Cube::set_z(float new_z) {
    position_.z = new_z;
}

float Cube::get_x() const {
    return position_.x;
}

float Cube::get_y() const {
    return position_.y;
}

float Cube::get_z() const {
    return position_.z;
}