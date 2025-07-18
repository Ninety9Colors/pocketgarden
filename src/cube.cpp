#include "cube.hpp"

Cube::Cube() : position{0.0f,0.0f,0.0f}, size{0.0f,0.0f,0.0f}, color(WHITE){};
Cube::Cube(Vector3 pos, Vector3 s, Color c) : position(pos), size(s), color(c) {};

void Cube::draw() const {
    DrawCubeV(position, size, color);
};