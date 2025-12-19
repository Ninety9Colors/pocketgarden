#define MAX_MATERIAL_MAPS 12

#include "object/object3d.hpp"
#include "rlgl.h"
#include "raymath.h"

Object3d::Object3d() : object_type_(static_cast<uint8_t>(ObjectType::DEFAULT)), mesh_(GenMeshCube(1.0f, 1.0f, 1.0f)), material_(LoadMaterialDefault()), position_{0.0f,0.0f,0.0f}, scale_(1.0f), quaternion_{0.0f,0.0f,0.0f,1.0f} {
    update_matrix();
}
Object3d::Object3d(Quaternion quaternion, Vector3 position, float scale) : object_type_(static_cast<uint8_t>(ObjectType::DEFAULT)), mesh_(GenMeshCube(1.0f, 1.0f, 1.0f)), material_(LoadMaterialDefault()), position_(position), scale_(scale), quaternion_{quaternion} {
    update_matrix();
}
Object3d::Object3d(const Object3d& rhs) noexcept : object_type_(static_cast<uint8_t>(ObjectType::DEFAULT)), quaternion_(rhs.quaternion_),position_(rhs.position_),mesh_(GenMeshCube(1.0f,1.0f,1.0f)),material_(LoadMaterialDefault()),scale_(rhs.scale_),transform_(rhs.transform_) {
    update_matrix();
}
Object3d::~Object3d() {
    UnloadMesh(mesh_);
    UnloadMaterial(material_);
}
void Object3d::generate_mesh() {
    UnloadMesh(mesh_);
    mesh_ = GenMeshCube(1.0f,1.0f,1.0f);
}
void Object3d::draw() const {
    DrawMesh(mesh_, material_, transform_);
}
void Object3d::draw(Matrix transform) const {
    DrawMesh(mesh_, material_, transform);
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

void Object3d::set_quaternion(Quaternion quaternion) {
    quaternion_ = quaternion;
    update_matrix();
}

Quaternion Object3d::get_quaternion() const {
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

const Matrix& Object3d::get_matrix() const {
    return transform_;
}

void Object3d::set_position(Vector3 position) {
    position_ = position;
    update_matrix();
}

Vector3 Object3d::get_position() const {
    return position_;
}

void Object3d::set_scale(float scale) {
    scale_ = scale;
    update_matrix();
}

float Object3d::get_scale() const {
    return scale_;
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

BoundingBox Object3d::get_bounding_box(Matrix transform) const {
    Vector3 minVertex = { 0 };
    Vector3 maxVertex = { 0 };

    if (mesh_.vertices != NULL)
    {
        minVertex = Vector3Transform(Vector3{ mesh_.vertices[0], mesh_.vertices[1], mesh_.vertices[2]},transform);
        maxVertex = Vector3Transform(Vector3{ mesh_.vertices[0], mesh_.vertices[1], mesh_.vertices[2]},transform);

        for (int i = 1; i < mesh_.vertexCount; i++)
        {
            minVertex = Vector3Min(minVertex, Vector3Transform(Vector3{mesh_.vertices[i*3], mesh_.vertices[i*3 + 1], mesh_.vertices[i*3 + 2] },transform));
            maxVertex = Vector3Max(maxVertex, Vector3Transform(Vector3{mesh_.vertices[i*3], mesh_.vertices[i*3 + 1], mesh_.vertices[i*3 + 2] },transform));
        }
    }

    // Create the bounding box
    BoundingBox box = { 0 };
    box.min = minVertex;
    box.max = maxVertex;

    return box;
}

uint8_t Object3d::get_type() const {
    return object_type_;
}

void swap(Object3d& a, Object3d& b) noexcept {
    using std::swap;
    swap(a.quaternion_,b.quaternion_);
    swap(a.position_,b.position_);
    swap(a.mesh_,b.mesh_);
    swap(a.material_,b.material_);
    swap(a.scale_,b.scale_);
    swap(a.transform_,b.transform_);
    swap(a.id_,b.id_);
}

Item::Item() : Object3d() {
    object_type_ |= static_cast<uint8_t>(ObjectType::ITEM);
}
Item::Item(Quaternion quaternion, Vector3 position, float scale) : Object3d(quaternion, position, scale) {
    object_type_ |= static_cast<uint8_t>(ObjectType::ITEM);
}

ParameterObject::ParameterObject() : Object3d() {};
ParameterObject::ParameterObject(Quaternion quaternion, Vector3 position, float scale) : Object3d(quaternion, position, scale) {}

void ParameterObject::set_parameters(ParameterMap map) {
    parameter_map_ = map;
}
void ParameterObject::set_parameter(std::string name, float value) {
    parameter_map_.set_parameter(name,value);
}
const Parameter ParameterObject::get_parameter(std::string name) const {
    return parameter_map_.get_parameter(name);
}