#include <assert.h>
#include <string>

#include "object/consistent/cube.hpp"
#include "raymath.h"
#include "logging.hpp"

#include <iostream>

Cube::Cube() : Object3d(), size_(1.0f,1.0f,1.0f), color_(WHITE) {
    generate_mesh();
    update_matrix();
}
Cube::Cube(const json& j) {
    from_json(j);
    generate_mesh();
    update_matrix();
}
Cube::Cube(Quaternion quaternion, Vector3 position, Vector3 size, float scale, Color color) : Object3d(quaternion, position, scale), size_(size), color_(color) {
    INFO("Initialized cube at " + std::to_string(position.x) + "," + std::to_string(position.y) + "," + std::to_string(position.z));
    generate_mesh();
    update_matrix();
}
Cube::Cube(const Cube& rhs) : Object3d(rhs), size_(rhs.size_), color_(rhs.color_) {
    generate_mesh();
    update_matrix();
}
void Cube::generate_mesh() {
    mesh_ = GenMeshCube(size_.x, size_.y, size_.z);
    material_.maps[MATERIAL_MAP_DIFFUSE].color = color_;
}
json Cube::to_json() const {
    json j = {
        {"type","Cube"},
        {"object_type",object_type_},
        {"position",{{"x",position_.x},{"y",position_.y},{"z",position_.z}}},
        {"size",{{"x",size_.x},{"y",size_.y},{"z",size_.z}}},
        {"scale",scale_},
        {"color",{{"r",color_.r},{"g",color_.g},{"b",color_.b},{"a",color_.a}}},
        {"quaternion",{{"x",quaternion_.x},{"y",quaternion_.y},{"z",quaternion_.z},{"w",quaternion_.w}}}
    };
    return j;
}
void Cube::from_json(const json& j) {
    object_type_ = j.at("object_type");
    position_ = {j.at("position")["x"],j.at("position")["y"],j.at("position")["z"]};
    size_ = {j.at("size")["x"],j.at("size")["y"],j.at("size")["z"]};
    scale_ = j.at("scale");
    color_ = {j.at("color")["r"],j.at("color")["g"],j.at("color")["b"],j.at("color")["a"]};
    quaternion_ = {j.at("quaternion")["x"],j.at("quaternion")["y"],j.at("quaternion")["z"],j.at("quaternion")["w"]};
}

void swap(Cube& a, Cube& b) noexcept {
    using std::swap;
    swap(static_cast<Object3d&>(a),static_cast<Object3d&>(b));
    swap(a.size_,b.size_);
    swap(a.color_,b.color_);
}