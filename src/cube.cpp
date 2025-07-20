#include "cube.hpp"

Cube::Cube() : position_{0.0f,0.0f,0.0f}, size_{0.0f,0.0f,0.0f}, color_(WHITE){};
Cube::Cube(float x, float y, float z, float width, float height, float length, int r, int g, int b, int a) :
    position_{x,y,z},
    size_{width,height,length},
    color_{r,g,b,a}
{};
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

std::string Cube::get_packet_string() const {
    return "Cube " + 
        std::to_string(position_.x) + " " + std::to_string(position_.y) + " " + std::to_string(position_.z) + " " +
        std::to_string(size_.x) + " " + std::to_string(size_.x) + " " + std::to_string(size_.x) + " " +
        std::to_string(color_.r) + " " + std::to_string(color_.g) + " " + std::to_string(color_.b) + " " + std::to_string(color_.a);
}