#include <assert.h>
#include <string>

#include "cube.hpp"
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
    mesh_ = GenMeshCube(size_.x, size_.y, size_.z);
    material_ = LoadMaterialDefault();
    material_.maps[MATERIAL_MAP_DIFFUSE].color = color_;
    transform_ = MatrixMultiply(MatrixScale(scale_, scale_, scale_),MatrixTranslate(position_.x, position_.y, position_.z));
}
Cube::Cube(Vector3 position, Vector3 size, float scale, Color color) : size_(size), scale_(scale), color_(color) {
    position_ = std::move(position);
    mesh_ = GenMeshCube(size_.x, size_.y, size_.z);
    material_ = LoadMaterialDefault();
    material_.maps[MATERIAL_MAP_DIFFUSE].color = color_;
    transform_ = MatrixMultiply(MatrixScale(scale_, scale_, scale_),MatrixTranslate(position_.x, position_.y, position_.z));
}
Cube::~Cube() {
    UnloadMesh(mesh_);
    UnloadMaterial(material_);
}

void Cube::draw() const {
    DrawMesh(mesh_, material_, transform_);
}

void Cube::draw_offset(float x, float y, float z) const {
    Matrix offset = MatrixAdd(transform_,Matrix{
        0,0,0,0,
        0,0,0,0,
        0,0,0,0,
        x,y,z,0
    });
    std::cout << std::to_string(offset.m0) << " " << std::to_string(offset.m4) << " " << std::to_string(offset.m8) << " " << std::to_string(offset.m12) << "\n";
    std::cout << std::to_string(offset.m1) << " " << std::to_string(offset.m5) << " " << std::to_string(offset.m9) << " " << std::to_string(offset.m13) << "\n";
    std::cout << std::to_string(offset.m2) << " " << std::to_string(offset.m6) << " " << std::to_string(offset.m10) << " " << std::to_string(offset.m14) << "\n";
    std::cout << std::to_string(offset.m3) << " " << std::to_string(offset.m7) << " " << std::to_string(offset.m11) << " " << std::to_string(offset.m15) << "\n";
    DrawMesh(mesh_, material_, offset);
}

void Cube::set_shader(std::shared_ptr<Shader> shader) {
    shader_ = shader;
    material_.shader = *shader;
}

void Cube::set_position(Vector3 position) {
    position_ = position;
    transform_ = MatrixMultiply(MatrixScale(scale_, scale_, scale_),MatrixTranslate(position_.x, position_.y, position_.z));
}

Vector3 Cube::get_position() const {
    return position_;
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
        std::to_string(color_.r) + " " + std::to_string(color_.g) + " " + std::to_string(color_.b) + " " + std::to_string(color_.a) + " " +
        std::to_string(quaternion_.x) + " " + std::to_string(quaternion_.y) + " " + std::to_string(quaternion_.z) + " " + std::to_string(quaternion_.w);
}