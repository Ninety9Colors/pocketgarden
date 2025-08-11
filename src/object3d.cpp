#define MAX_MATERIAL_MAPS 12

#include "object3d.hpp"
#include "rlgl.h"
#include "raymath.h"

Object3d::Object3d() : mesh_(GenMeshCube(1.0f, 1.0f, 1.0f)), material_(std::move(LoadMaterialDefault())), position_{0.0f,0.0f,0.0f}, scale_(1.0f), quaternion_{0.0f,0.0f,0.0f,1.0f} {};
Object3d::Object3d(float scale) : mesh_(GenMeshCube(1.0f, 1.0f, 1.0f)), material_(std::move(LoadMaterialDefault())), position_{0.0f,0.0f,0.0f}, scale_(scale), quaternion_{0.0f,0.0f,0.0f,1.0f} {};
Object3d::Object3d(Vector3 position, float scale) : mesh_(GenMeshCube(1.0f, 1.0f, 1.0f)), material_(std::move(LoadMaterialDefault())), position_(position), scale_(scale), quaternion_{0.0f,0.0f,0.0f,1.0f} {};
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
    DrawBoundingBox(get_bounding_box(),WHITE);
}
void Object3d::draw_offset(float x, float y, float z) const {
    Matrix offset = MatrixAdd(transform_,Matrix{
        0,0,0,x,
        0,0,0,y,
        0,0,0,z,
        0,0,0,0
    });
    DrawMesh(mesh_, material_, offset);
    DrawBoundingBox(BoundingBox{Vector3Add(get_bounding_box().min, Vector3{x,y,z}),Vector3Add(get_bounding_box().max, Vector3{x,y,z})},WHITE);
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

void Object3d::rotate_axis(Vector3 axis, float radians) {
    Quaternion rotation = QuaternionFromAxisAngle(axis,radians);
    quaternion_ = QuaternionMultiply(rotation,quaternion_);
    update_matrix();
}

void Object3d::update_matrix() {
    transform_ = MatrixMultiply(MatrixScale(scale_, scale_, scale_),MatrixMultiply(QuaternionToMatrix(quaternion_), MatrixTranslate(position_.x, position_.y, position_.z)));
}

void Object3d::set_position(Vector3 position) {
    position_ = position;
    update_matrix();
}

Vector3 Object3d::get_position() const {
    return position_;
}

BoundingBox Object3d::get_bounding_box() const {
    Vector3 minVertex = { 0 };
    Vector3 maxVertex = { 0 };

    if (mesh_.vertices != NULL)
    {
        minVertex = Vector3Transform(Vector3{ mesh_.vertices[0], mesh_.vertices[1], mesh_.vertices[2]},transform_);
        maxVertex = Vector3Transform(Vector3{ mesh_.vertices[0], mesh_.vertices[1], mesh_.vertices[2]},transform_);

        for (int i = 1; i < mesh_.vertexCount; i++)
        {
            minVertex = Vector3Min(minVertex, Vector3Transform(Vector3{mesh_.vertices[i*3], mesh_.vertices[i*3 + 1], mesh_.vertices[i*3 + 2] },transform_));
            maxVertex = Vector3Max(maxVertex, Vector3Transform(Vector3{mesh_.vertices[i*3], mesh_.vertices[i*3 + 1], mesh_.vertices[i*3 + 2] },transform_));
        }
    }

    // Create the bounding box
    BoundingBox box = { 0 };
    box.min = minVertex;
    box.max = maxVertex;

    return box;
}

Item::Item() : Object3d() {}
Item::Item(float scale) : Object3d(scale) {}
Item::Item(Vector3 position, float scale) : Object3d(position, scale) {}