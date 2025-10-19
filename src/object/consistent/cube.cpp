#include <assert.h>
#include <string>

#include "object/consistent/cube.hpp"
#include "util.hpp"

#include "raymath.h"

#include <iostream>

Cube::Cube(std::string data) {
    std::vector<std::string> split = split_string(data);
    assert(split[0] == "Cube" && split.size() == 16);
    position_ = Vector3{std::stof(split[1]), std::stof(split[2]), std::stof(split[3])};
    size_ = Vector3{std::stof(split[4]), std::stof(split[5]), std::stof(split[6])};
    scale_ = std::stof(split[7]);
    color_ = Color{(unsigned char)std::stoi(split[8]), (unsigned char)std::stoi(split[9]), (unsigned char)std::stoi(split[10]), (unsigned char)std::stoi(split[11])};
    quaternion_ = Quaternion{std::stof(split[12]),std::stof(split[13]),std::stof(split[14]),std::stof(split[15])};
    UnloadMesh(mesh_);
    mesh_ = GenMeshCube(size_.x, size_.y, size_.z);
    material_.maps[MATERIAL_MAP_DIFFUSE].color = color_;
    update_matrix();
}
Cube::Cube(Vector3 position, Vector3 size, float scale, Color color) : Object3d(position, scale), size_(size), color_(color) {
    UnloadMesh(mesh_);
    mesh_ = GenMeshCube(size_.x, size_.y, size_.z);
    material_.maps[MATERIAL_MAP_DIFFUSE].color = color_;
    update_matrix();
}

std::string Cube::to_string() const {
    return "Cube " + 
        std::to_string(position_.x) + " " + std::to_string(position_.y) + " " + std::to_string(position_.z) + " " +
        std::to_string(size_.x) + " " + std::to_string(size_.y) + " " + std::to_string(size_.z) + " " +
        std::to_string(scale_) + " " +
        std::to_string(color_.r) + " " + std::to_string(color_.g) + " " + std::to_string(color_.b) + " " + std::to_string(color_.a) + " " +
        std::to_string(quaternion_.x) + " " + std::to_string(quaternion_.y) + " " + std::to_string(quaternion_.z) + " " + std::to_string(quaternion_.w);
}