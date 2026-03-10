#define MAX_MATERIAL_MAPS 12

#include "event/event.hpp"
#include "object/object3d.hpp"
#include "game.hpp"
#include "application.hpp"
#include "rlgl.h"
#include "raymath.h"

#include "util/draw.hpp"

#include "logging.hpp"

Object3d::Object3d() : object_type_(static_cast<uint8_t>(ObjectType::DEFAULT)), mesh_{0}, material_{0}, position_{0.0f,0.0f,0.0f}, scale_(1.0f), quaternion_{0.0f,0.0f,0.0f,1.0f} {
    material_ = LoadMaterialDefault();
    material_.shader = Application::get_shader_default();
}
// Object3d::Object3d(const json& j) {
//     from_json(j);
//     generate_mesh();
//     update_matrix();
// }
Object3d::Object3d(Quaternion quaternion, Vector3 position, float scale) : object_type_(static_cast<uint8_t>(ObjectType::DEFAULT)), mesh_{0}, material_{0}, position_(position), scale_(scale), quaternion_{quaternion} {
    INFO("Constructing object at " + std::to_string(position.x) + "," + std::to_string(position.y) + "," + std::to_string(position.z));
    material_ = LoadMaterialDefault();
    material_.shader = Application::get_shader_default();
    INFO(" - constructed material");
}
Object3d::Object3d(const Object3d& rhs) : object_type_(rhs.object_type_), quaternion_(rhs.quaternion_),position_(rhs.position_),mesh_{0},material_{0},scale_(rhs.scale_),transform_(rhs.transform_) {
    material_ = LoadMaterialDefault();
    material_.shader = Application::get_shader_default();
}
Object3d::~Object3d() {
    if (mesh_.indices != nullptr)
        UnloadMesh(mesh_);
}
void Object3d::generate_mesh() {
    mesh_ = GenMeshCube(1.0f,1.0f,1.0f);
}
void Object3d::draw(Game& game) const {
    draw(game,material_);
}
void Object3d::draw(Game& game, Material material) const {
    DrawMesh(mesh_,material,transform_);
    // draw_mesh_skeleton(mesh_,transform_);
}
void Object3d::draw(Game& game, Matrix transform) const {
    draw(game,transform,material_);
}
void Object3d::draw(Game& game, Matrix transform, Material material) const {
    DrawMesh(mesh_, material, transform);
    // draw_mesh_skeleton(mesh_,transform);
}
void Object3d::draw_offset(Game& game, float x, float y, float z) const {
    Matrix offset = MatrixAdd(transform_,Matrix{
        0,0,0,x,
        0,0,0,y,
        0,0,0,z,
        0,0,0,0
    });
    draw(game,offset);
}

void Object3d::draw_instanced(Game& game, const Matrix* transforms, int matrix_count) const {
    draw_instanced(game,material_,transforms,matrix_count);
}

void Object3d::draw_instanced(Game& game, Material material, const Matrix* transforms, int matrix_count) const {
    for (int i = 0; i < matrix_count; i++) {
        DrawMesh(mesh_,material,transforms[i]);
        // draw_mesh_skeleton(mesh_,transforms[i]);
    }
    // TODO: Fix instanced draw
    // DrawMeshInstanced(mesh_,material,transforms,matrix_count);
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

void Object3d::set_material(Material material) {
    material_ = material;
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

void Object3d::use(Game&, const std::string username, const std::vector<bool>& keybinds, float dt) {}
void Object3d::on_drop(Game& game, std::string username, const std::vector<bool>& keybinds, float dt) {}

Item::Item() : Object3d() {
    object_type_ |= static_cast<uint8_t>(ObjectType::ITEM);
}
Item::Item(Quaternion quaternion, Vector3 position, float scale) : Object3d(quaternion, position, scale) {
    object_type_ |= static_cast<uint8_t>(ObjectType::ITEM);
}
ParameterObject::ParameterObject() : ParameterObject(std::random_device{}()) {}
ParameterObject::ParameterObject(uint32_t seed) : Object3d(), seed_(seed) {}
ParameterObject::ParameterObject(Quaternion quaternion, Vector3 position, float scale) : ParameterObject(quaternion,position,scale,std::random_device{}()) {}
ParameterObject::ParameterObject(Quaternion quaternion, Vector3 position, float scale, uint32_t seed) : Object3d(quaternion,position,scale), seed_(seed) {}

void ParameterObject::set_parameter_map(ParameterMap map) {
    assert(map.contains_parameter("seed"));
    parameter_map_ = map;
}
void ParameterObject::set_parameter(std::string name, float value) {
    parameter_map_.set_parameter(name,value);
}
void ParameterObject::set_parameter(std::string name, Parameter parameter) {
    parameter_map_.set_parameter(name,parameter);
}
const Parameter ParameterObject::get_parameter(std::string name) const {
    return parameter_map_.get_parameter(name);
}
LSystemObject::LSystemObject() : LSystemObject(std::random_device{}()) {}
LSystemObject::LSystemObject(uint32_t seed) : Object3d(), seed_(seed), stage_(0) {}
LSystemObject::LSystemObject(Quaternion quaternion, Vector3 position, float scale) : LSystemObject(quaternion,position,scale,std::random_device{}()) {}
LSystemObject::LSystemObject(Quaternion quaternion, Vector3 position, float scale, uint32_t seed) : Object3d(quaternion,position,scale), seed_(seed), stage_(0) {}