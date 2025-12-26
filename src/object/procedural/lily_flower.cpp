#include <cassert>

#include "object/procedural/lily_flower.hpp"

#include "raymath.h"

constexpr int DEFAULT_SLICES_X = 40;
constexpr int DEFAULT_SLICES_Y = 20;

LilyFlower::LilyFlower() : LilyFlower(std::random_device{}()) {}
LilyFlower::LilyFlower(uint32_t seed) : ParameterObject(seed), slices_(DEFAULT_SLICES_X, DEFAULT_SLICES_Y) {
    upper_petal_ = std::make_unique<TaperedPetal>(seed);
    lower_petal_ = std::make_unique<TaperedPetal>(seed);
    initialize_parameters();
}
LilyFlower::LilyFlower(Quaternion quaternion, Vector3 position, float scale) : LilyFlower(quaternion,position,scale,std::random_device{}()) {}
LilyFlower::LilyFlower(Quaternion quaternion, Vector3 position, float scale, uint32_t seed) : ParameterObject(quaternion, position, scale,seed), slices_(DEFAULT_SLICES_X, DEFAULT_SLICES_Y) {
    upper_petal_ = std::make_unique<TaperedPetal>(seed);
    lower_petal_ = std::make_unique<TaperedPetal>(seed);
    initialize_parameters();
}
LilyFlower::LilyFlower(const json& j) {
    from_json(j);
    initialize_parameters();
}

json LilyFlower::to_json() const {
    json j = {
        {"type","LilyFlower"},
        {"object_type",object_type_},
        {"position",{{"x",position_.x},{"y",position_.y},{"z",position_.z}}},
        {"scale",scale_},
        {"quaternion",{{"x",quaternion_.x},{"y",quaternion_.y},{"z",quaternion_.z},{"w",quaternion_.w}}},
        {"seed",seed_},
        {"parameter_map",parameter_map_.to_json()},
        {"slices",{{"first",slices_.first},{"second",slices_.second}}},
        {"upper_petal",upper_petal_->to_json()},
        {"lower_petal",lower_petal_->to_json()}
    };
    return j;
}

void LilyFlower::from_json(const json& j) {
    object_type_ = j.at("object_type");
    position_ = {j.at("position")["x"],j.at("position")["y"],j.at("position")["z"]};
    scale_ = j.at("scale");
    quaternion_ = {j.at("quaternion")["x"],j.at("quaternion")["y"],j.at("quaternion")["z"],j.at("quaternion")["w"]};
    seed_ = j.at("seed");
    parameter_map_ = ParameterMap{j.at("parameter_map")};
    slices_ = {j.at("slices")["first"],j.at("slices")["second"]};
    upper_petal_ = std::make_unique<TaperedPetal>(j.at("upper_petal"));
    lower_petal_ = std::make_unique<TaperedPetal>(j.at("lower_petal"));
}

void LilyFlower::draw(Game& game, Material material) const {
    upper_petal_->draw(game,upper_transforms_[0],material);
    upper_petal_->draw(game,upper_transforms_[1],material);
    upper_petal_->draw(game,upper_transforms_[2],material);
    lower_petal_->draw(game,lower_transforms_[0],material);
    lower_petal_->draw(game,lower_transforms_[1],material);
    lower_petal_->draw(game,lower_transforms_[2],material);
}
void LilyFlower::draw(Game& game,Matrix transform, Material material) const {
    upper_petal_->draw(game,MatrixMultiply(upper_transforms_[0], transform),material);
    upper_petal_->draw(game,MatrixMultiply(upper_transforms_[1], transform),material);
    upper_petal_->draw(game,MatrixMultiply(upper_transforms_[2], transform),material);
    lower_petal_->draw(game,MatrixMultiply(lower_transforms_[0], transform),material);
    lower_petal_->draw(game,MatrixMultiply(lower_transforms_[1], transform),material);
    lower_petal_->draw(game,MatrixMultiply(lower_transforms_[2], transform),material);
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

void LilyFlower::set_slices(std::pair<int,int> slices) {
    upper_petal_->set_slices(slices);
    lower_petal_->set_slices(slices);
}
void LilyFlower::initialize_parameters() {
    parameter_map_.set_parameter("PetalPitchUpper", Parameter{-90.0f,35.0f,80.0f});
    parameter_map_.set_parameter("PetalPitchLower", Parameter{-90.0f,35.0f,80.0f});
}