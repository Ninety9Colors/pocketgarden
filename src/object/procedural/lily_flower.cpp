#include <cassert>

#include "object/procedural/lily_flower.hpp"
#include "util.hpp"

#include "raymath.h"

LilyFlower::LilyFlower() : ParameterObject(), slices_(40,20) {
    initialize_parameters();
    std::random_device rd;
    seed_ = rd();
    upper_petal_ = std::make_unique<TaperedPetal>();
    lower_petal_ = std::make_unique<TaperedPetal>();
}
LilyFlower::LilyFlower(float scale) : ParameterObject(scale), slices_(40,20) {
    initialize_parameters();
    std::random_device rd;
    seed_ = rd();
    upper_petal_ = std::make_unique<TaperedPetal>();
    lower_petal_ = std::make_unique<TaperedPetal>();
}
LilyFlower::LilyFlower(Vector3 position, float scale) : ParameterObject(position, scale), slices_(40,20) {
    initialize_parameters();
    std::random_device rd;
    seed_ = rd();
    upper_petal_ = std::make_unique<TaperedPetal>();
    lower_petal_ = std::make_unique<TaperedPetal>();
}
LilyFlower::LilyFlower(ParameterMap map, uint64_t seed, Quaternion quaternion, Vector3 position, float scale) : ParameterObject(quaternion,position,scale), slices_(40,20) {
    parameter_map_ = map;
    seed_ = seed;
    upper_petal_ = std::make_unique<TaperedPetal>();
    lower_petal_ = std::make_unique<TaperedPetal>();
}
LilyFlower::LilyFlower(std::string data) : ParameterObject() {
    std::vector<std::string> split = split_string(data);
    assert(split[0] == "LilyFlower");
    position_ = Vector3{std::stof(split[1]), std::stof(split[2]), std::stof(split[3])};
    scale_ = std::stof(split[4]);
    quaternion_ = Quaternion{std::stof(split[5]),std::stof(split[6]),std::stof(split[7]),std::stof(split[8])};
    seed_ = std::stoull(split[9]);
    parameter_map_ = ParameterMap(split[10]);
    upper_petal_ = std::make_unique<TaperedPetal>(split[11]);
    lower_petal_ = std::make_unique<TaperedPetal>(split[12]);
}

void LilyFlower::draw() const {
    upper_petal_->draw(upper_transforms_[0]);
    upper_petal_->draw(upper_transforms_[1]);
    upper_petal_->draw(upper_transforms_[2]);
    lower_petal_->draw(lower_transforms_[0]);
    lower_petal_->draw(lower_transforms_[1]);
    lower_petal_->draw(lower_transforms_[2]);
}
void LilyFlower::draw(Matrix transform) const {
    upper_petal_->draw(MatrixMultiply(upper_transforms_[0], transform));
    upper_petal_->draw(MatrixMultiply(upper_transforms_[1], transform));
    upper_petal_->draw(MatrixMultiply(upper_transforms_[2], transform));
    lower_petal_->draw(MatrixMultiply(lower_transforms_[0], transform));
    lower_petal_->draw(MatrixMultiply(lower_transforms_[1], transform));
    lower_petal_->draw(MatrixMultiply(lower_transforms_[2], transform));
}
void LilyFlower::draw_offset(float x, float y, float z) const {
    Matrix offset = MatrixTranslate(x,y,z);
    draw(offset);
}
void LilyFlower::set_shader(std::shared_ptr<Shader> shader) {
    upper_petal_->set_shader(shader);
    lower_petal_->set_shader(shader);
}

void LilyFlower::update_matrix() {
    const float SQRT_3 = std::sqrtf(3.0f);

    float pitch_upper = parameter_map_.get_parameter("PetalPitchUpper").value*DEG2RAD;
    float pitch_lower = parameter_map_.get_parameter("PetalPitchLower").value*DEG2RAD;
    
    float default_angle_upper = std::acos(Vector3DotProduct(Vector3Normalize(upper_petal_->tip_vector()), Vector3{1,0,0}));
    float default_angle_lower = std::acos(Vector3DotProduct(Vector3Normalize(lower_petal_->tip_vector()), Vector3{1,0,0}));

    float base_width_upper = upper_petal_->base_width();
    float base_width_lower = lower_petal_->base_width();

    pitch_upper -= default_angle_upper;
    pitch_lower -= default_angle_lower;

    Quaternion pitch_quaternion_upper = QuaternionFromAxisAngle(Vector3{0,0,1},pitch_upper);
    Quaternion pitch_quaternion_lower = QuaternionFromAxisAngle(Vector3{0,0,1},pitch_lower);

    float offset_upper = base_width_upper*SQRT_3/6.0f;
    float offset_lower = base_width_lower*SQRT_3/6.0f;

    Quaternion splay_one = QuaternionFromAxisAngle(Vector3{0,1,0},2*PI/3);
    Quaternion splay_two = QuaternionFromAxisAngle(Vector3{0,1,0},4*PI/3);

    Quaternion splay_three = QuaternionFromAxisAngle(Vector3{0,1,0},PI/3);
    Quaternion splay_four = QuaternionFromAxisAngle(Vector3{0,1,0},PI);
    Quaternion splay_five = QuaternionFromAxisAngle(Vector3{0,1,0},5*PI/3);

    Matrix inner_transform_upper = MatrixMultiply(QuaternionToMatrix(pitch_quaternion_upper),MatrixTranslate(offset_upper,0,0));
    Matrix inner_transform_upper_one = MatrixMultiply(QuaternionToMatrix(pitch_quaternion_upper),MatrixMultiply(MatrixTranslate(offset_upper,0,0),QuaternionToMatrix(splay_one)));
    Matrix inner_transform_upper_two = MatrixMultiply(QuaternionToMatrix(pitch_quaternion_upper),MatrixMultiply(MatrixTranslate(offset_upper,0,0),QuaternionToMatrix(splay_two)));
    Matrix inner_transform_lower = MatrixMultiply(QuaternionToMatrix(pitch_quaternion_lower),MatrixMultiply(MatrixTranslate(offset_lower,0,0),QuaternionToMatrix(splay_three)));
    Matrix inner_transform_lower_one = MatrixMultiply(QuaternionToMatrix(pitch_quaternion_lower),MatrixMultiply(MatrixTranslate(offset_lower,0,0),QuaternionToMatrix(splay_four)));
    Matrix inner_transform_lower_two = MatrixMultiply(QuaternionToMatrix(pitch_quaternion_lower),MatrixMultiply(MatrixTranslate(offset_lower,0,0),QuaternionToMatrix(splay_five)));

    transform_ = MatrixMultiply(MatrixScale(scale_, scale_, scale_),MatrixMultiply(QuaternionToMatrix(quaternion_), MatrixTranslate(position_.x, position_.y, position_.z)));
    
    upper_transforms_[0] = MatrixMultiply(inner_transform_upper, transform_);
    upper_transforms_[1] = MatrixMultiply(inner_transform_upper_one, transform_);
    upper_transforms_[2] = MatrixMultiply(inner_transform_upper_two, transform_);

    lower_transforms_[0] = MatrixMultiply(inner_transform_lower, transform_);
    lower_transforms_[1] = MatrixMultiply(inner_transform_lower_one, transform_);
    lower_transforms_[2] = MatrixMultiply(inner_transform_lower_two, transform_);
}

BoundingBox LilyFlower::get_bounding_box() const {
    BoundingBox upper = upper_petal_->get_bounding_box(transform_);
    BoundingBox lower = lower_petal_->get_bounding_box(transform_);
    return BoundingBox{Vector3{std::max(upper.max.x,lower.max.x),std::max(upper.max.y,lower.max.y),std::max(upper.max.z,lower.max.z)},
                       Vector3{std::min(upper.min.x,lower.min.x),std::min(upper.min.y,lower.min.y),std::min(upper.min.z,lower.min.z)}};
}
BoundingBox LilyFlower::get_bounding_box(Matrix transform) const {
    Matrix m = MatrixMultiply(transform_, transform);
    BoundingBox upper = upper_petal_->get_bounding_box(m);
    BoundingBox lower = lower_petal_->get_bounding_box(m);
    return BoundingBox{Vector3{std::max(upper.max.x,lower.max.x),std::max(upper.max.y,lower.max.y),std::max(upper.max.z,lower.max.z)},
                       Vector3{std::min(upper.min.x,lower.min.x),std::min(upper.min.y,lower.min.y),std::min(upper.min.z,lower.min.z)}};
}

void LilyFlower::generate_mesh() {
    upper_petal_->generate_mesh();
    lower_petal_->generate_mesh();
    update_matrix();
}
void LilyFlower::generate_mesh(uint64_t seed) {
    upper_petal_->generate_mesh(seed);
    lower_petal_->generate_mesh(seed);
    update_matrix();
}
void LilyFlower::set_slices(std::pair<int,int> slices) {
    upper_petal_->set_slices(slices);
    lower_petal_->set_slices(slices);
}
std::string LilyFlower::to_string() const {
    return "LilyFlower " +
        std::to_string(position_.x) + " " + std::to_string(position_.y) + " " + std::to_string(position_.z) + " " +
        std::to_string(scale_) + " " +
        std::to_string(quaternion_.x) + " " + std::to_string(quaternion_.y) + " " + std::to_string(quaternion_.z) + " " + std::to_string(quaternion_.w) + " " +
        std::to_string(seed_) + " (" + 
        parameter_map_.to_string() + ")(" +
        upper_petal_->to_string() + ")(" +
        lower_petal_->to_string() + ")";
}
void LilyFlower::initialize_parameters() {
    parameter_map_.set_parameter("PetalPitchUpper", Parameter{-90.0f,35.0f,80.0f});
    parameter_map_.set_parameter("PetalPitchLower", Parameter{-90.0f,35.0f,80.0f});
}