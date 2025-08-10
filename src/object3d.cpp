#define MAX_MATERIAL_MAPS 12

#include "object3d.hpp"
#include "rlgl.h"
#include "raymath.h"

Object3d::Object3d() : mesh_(GenMeshCube(1.0f, 1.0f, 1.0f)), material_(std::move(LoadMaterialDefault())), position_{0.0f,0.0f,0.0f}, scale_(1.0f), quaternion_{0.0f,0.0f,0.0f} {};
Object3d::Object3d(float scale) : mesh_(GenMeshCube(1.0f, 1.0f, 1.0f)), material_(std::move(LoadMaterialDefault())), position_{0.0f,0.0f,0.0f}, scale_(scale), quaternion_{0.0f,0.0f,0.0f} {};
Object3d::Object3d(Vector3 position, float scale) : mesh_(GenMeshCube(1.0f, 1.0f, 1.0f)), material_(std::move(LoadMaterialDefault())), position_(position), scale_(scale), quaternion_{0.0f,0.0f,0.0f} {};
Object3d::~Object3d() {
    UnloadMesh(mesh_);
    if (material_.maps != NULL){
        for (int i = 0; i < MAX_MATERIAL_MAPS; i++)
            if (material_.maps[i].texture.id != rlGetTextureIdDefault()) rlUnloadTexture(material_.maps[i].texture.id);
    }
    RL_FREE(material_.maps);
}

void Object3d::draw() const {
    DrawMesh(mesh_, material_, transform_);
}
void Object3d::draw_offset(float x, float y, float z) const {
    Matrix offset = MatrixAdd(transform_,Matrix{
        0,0,0,x,
        0,0,0,y,
        0,0,0,z,
        0,0,0,0
    });
    DrawMesh(mesh_, material_, offset);
}

std::shared_ptr<Shader> Object3d::get_shader() {
    return shader_;
}

void Object3d::set_shader(std::shared_ptr<Shader> shader) {
    shader_ = shader;
    material_.shader = *shader;
}

void Object3d::set_quaternion(Quaternion quaternion) {
    quaternion_ = quaternion;
}

Quaternion Object3d::get_quaternion() {
    return quaternion_;
}

void Object3d::update_matrix() {
    transform_ = MatrixMultiply(MatrixScale(scale_, scale_, scale_),MatrixTranslate(position_.x, position_.y, position_.z));
}

void Object3d::set_position(Vector3 position) {
    position_ = position;
    update_matrix();
}

Vector3 Object3d::get_position() const {
    return position_;
}

BoundingBox Object3d::get_bounding_box() const {
    BoundingBox box = GetMeshBoundingBox(mesh_);
    box.max = Vector3{box.max.x*scale_ + position_.x, box.max.y*scale_ + position_.y, box.max.z*scale_ + position_.z};
    box.min = Vector3{box.min.x*scale_ + position_.x, box.min.y*scale_ + position_.y, box.min.z*scale_ + position_.z};
    return box;
}

Item::Item() : Object3d() {}
Item::Item(float scale) : Object3d(scale) {}
Item::Item(Vector3 position, float scale) : Object3d(position, scale) {}