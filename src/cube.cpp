#include <assert.h>
#include <string>

#include "cube.hpp"
#include "util.hpp"

Cube::Cube() : position_{0.0f,0.0f,0.0f}, size_{0.0f,0.0f,0.0f}, color_(WHITE){};
Cube::Cube(std::string data) {
    std::vector<std::string> split = split_string(data);
    assert(split[0] == "Cube" && split.size() == 12);
    position_ = Vector3{std::stof(split[1]), std::stof(split[2]), std::stof(split[3])};
    size_ = Vector3{std::stof(split[4]), std::stof(split[5]), std::stof(split[6])};
    scale_ = std::stof(split[7]);
    color_ = Color{(unsigned char)std::stoi(split[8]), (unsigned char)std::stoi(split[9]), (unsigned char)std::stoi(split[10]), (unsigned char)std::stoi(split[11])};
    model_ = LoadModelFromMesh(GenMeshCube(size_.x, size_.y, size_.z));
}
Cube::Cube(Vector3 position, Vector3 size, float scale, Color color) : position_(position), size_(size), scale_(scale), color_(color) {
    model_ = LoadModelFromMesh(GenMeshCube(size_.x, size_.y, size_.z));
};

void Cube::draw() const {
    DrawModel(model_, position_, scale_, color_);
    draw_outline();
};

void Cube::draw_outline() const {
    //TODO
};

void Cube::draw_offset(float x, float y, float z) const {
    DrawModel(model_, Vector3{position_.x+x, position_.y+y, position_.z+z}, scale_, color_);
    draw_outline_offset(x,y,z);
};
void Cube::draw_outline_offset(float x, float y, float z) const {
    //TODO
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

BoundingBox Cube::get_bounding_box() const {
    return BoundingBox{Vector3{position_.x - size_.x/2*scale_, position_.y - size_.y/2*scale_, position_.z - size_.z/2*scale_},
                        Vector3{position_.x + size_.x/2*scale_, position_.y + size_.y/2*scale_, position_.z + size_.z/2*scale_}};
}

std::string Cube::to_string() const {
    return "Cube " + 
        std::to_string(position_.x) + " " + std::to_string(position_.y) + " " + std::to_string(position_.z) + " " +
        std::to_string(size_.x) + " " + std::to_string(size_.y) + " " + std::to_string(size_.z) + " " +
        std::to_string(scale_) + " " +
        std::to_string(color_.r) + " " + std::to_string(color_.g) + " " + std::to_string(color_.b) + " " + std::to_string(color_.a);
}