#include "fern.hpp"

#include "raymath.h"
#include "rlgl.h"

#include <random>
#include <memory>

#include "logging.hpp"

Fern::Fern() : Fern(std::random_device{}()) {}
Fern::Fern(uint32_t seed) : Object3d(), seed_(seed), stage_(0), rng_(seed) {}
Fern::Fern(Quaternion quaternion, Vector3 position, float scale) : Fern(quaternion,position,scale,std::random_device{}()) {}
Fern::Fern(Quaternion quaternion, Vector3 position, float scale, uint32_t seed) : Object3d(quaternion,position,scale), seed_(seed), stage_(0), rng_(seed) {}
Fern::Fern(const json& j) {
    from_json(j);
    rng_ = std::mt19937_64(seed_);
}

// Should regenerate mesh if needed
void Fern::advance_stage() {
    INFO("Growing fern from stage " + std::to_string(stage_));
    // add a new fern leaf
    ferns_.push_back(std::make_unique<FernFrond>(rng_()));
    ferns_.back()->initialize();
    fern_transforms_.push_back(MatrixIdentity());

    std::uniform_real_distribution<> gen(0.0,360.0);
    std::uniform_real_distribution<> gen2(-0.25,0.25);
    float dx = gen2(rng_);
    float dz = gen2(rng_);
    Matrix translate = MatrixTranslate(dx,0.0f,dz);
    Matrix rot = MatrixRotate({0,1,0},gen(rng_)*DEG2RAD);
    Matrix scale = MatrixScale(1.0f,1.0f,1.0f);
    fern_transforms_base_.push_back({rot,translate,scale});
    INFO("Added a leaf");

    for (int i = 0; i < ferns_.size(); i++)
        ferns_[i]->grow();
    INFO("Grew leaves");
    stage_++;
}

// Should be called explicitly after the object is created
void Fern::initialize() {
    int original_stage = stage_;
    stage_ = 0;
    while(stage_ < original_stage)
        advance_stage();
    INFO("Initialized Fern with stage: " + std::to_string(stage_));
}

void Fern::draw(Game& game, Material material) const {
    for (int i = 0; i < ferns_.size(); i++)
        ferns_[i]->draw(game,fern_transforms_[i],material);
}

json Fern::to_json() const {
    json j = {
        {"type","Fern"},
        {"object_type",object_type_},
        {"position",{{"x",position_.x},{"y",position_.y},{"z",position_.z}}},
        {"scale",scale_},
        {"quaternion",{{"x",quaternion_.x},{"y",quaternion_.y},{"z",quaternion_.z},{"w",quaternion_.w}}},
        {"seed",seed_},
        {"stage",stage_}
    };
    return j;
}

void Fern::from_json(const json& j) {
    object_type_ = j.at("object_type");
    position_ = {j.at("position")["x"],j.at("position")["y"],j.at("position")["z"]};
    scale_ = j.at("scale");
    quaternion_ = {j.at("quaternion")["x"],j.at("quaternion")["y"],j.at("quaternion")["z"],j.at("quaternion")["w"]};
    seed_ = j.at("seed");
    stage_ = j.at("stage");
}

void Fern::generate_mesh() {
    for (int i = 0; i < ferns_.size(); i++)
        ferns_[i]->generate_mesh();
    update_matrix();
    for (int i = 0; i < ferns_.size(); i++)
        ferns_[i]->update_matrix();
    INFO("Generated Lily mesh");
}

void Fern::update_matrix() {
    transform_ = MatrixMultiply(MatrixScale(scale_, scale_, scale_),MatrixMultiply(QuaternionToMatrix(quaternion_), MatrixTranslate(position_.x, position_.y, position_.z)));
    for (int i = 0; i < fern_transforms_.size(); i++) {
        Matrix base_rotate = fern_transforms_base_[i][0];
        Matrix base_offset = fern_transforms_base_[i][1];
        Matrix base_scale = fern_transforms_base_[i][2]; // scale of the fern, not whole object
        Vector3 fern_offset = Vector3Scale(Vector3{base_offset.m12,base_offset.m13,base_offset.m14},scale_);
        fern_offset = Vector3RotateByQuaternion(fern_offset,quaternion_);

        float scale = 1.0f;
        Matrix transform = MatrixMultiply(MatrixMultiply(MatrixMultiply(MatrixScale(scale_,scale_,scale_),base_scale),MatrixScale(scale, scale, scale)),MatrixMultiply(MatrixMultiply(base_rotate,QuaternionToMatrix(quaternion_)), MatrixTranslate(position_.x+fern_offset.x, position_.y+fern_offset.y, position_.z+fern_offset.z)));
        fern_transforms_[i] = transform;
    }
}