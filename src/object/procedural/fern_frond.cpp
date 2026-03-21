#include <cassert>
#include <functional>

#include "object/procedural/fern_frond.hpp"

#include "raymath.h"

#include "logging.hpp"

constexpr int DEFAULT_SLICES_X = 40;
constexpr int DEFAULT_SLICES_Y = 20;

FernFrond::FernFrond() : FernFrond(std::random_device{}()) {}
FernFrond::FernFrond(uint32_t seed) : ParameterObject(seed), slices_(DEFAULT_SLICES_X, DEFAULT_SLICES_Y) {
    std::mt19937_64 rng(seed);
    initialize_parameters();
}
FernFrond::FernFrond(Quaternion quaternion, Vector3 position, float scale) : FernFrond(quaternion,position,scale,std::random_device{}()) {}
FernFrond::FernFrond(Quaternion quaternion, Vector3 position, float scale, uint32_t seed) : ParameterObject(quaternion, position, scale,seed), slices_(DEFAULT_SLICES_X, DEFAULT_SLICES_Y) {
    std::mt19937_64 rng(seed);
    initialize_parameters();
}
FernFrond::FernFrond(const json& j) {
    from_json(j);
    initialize_parameters();
}

json FernFrond::to_json() const {
    json j = {
        {"type","LilyFlower"},
        {"object_type",object_type_},
        {"position",{{"x",position_.x},{"y",position_.y},{"z",position_.z}}},
        {"scale",scale_},
        {"quaternion",{{"x",quaternion_.x},{"y",quaternion_.y},{"z",quaternion_.z},{"w",quaternion_.w}}},
        {"seed",seed_},
        {"parameter_map",parameter_map_.to_json()},
        {"slices",{{"first",slices_.first},{"second",slices_.second}}},
        {"leaf",leaf_->to_json()}
    };
    return j;
}

void FernFrond::from_json(const json& j) {
    object_type_ = j.at("object_type");
    position_ = {j.at("position")["x"],j.at("position")["y"],j.at("position")["z"]};
    scale_ = j.at("scale");
    quaternion_ = {j.at("quaternion")["x"],j.at("quaternion")["y"],j.at("quaternion")["z"],j.at("quaternion")["w"]};
    seed_ = j.at("seed");
    parameter_map_ = ParameterMap{j.at("parameter_map")};
    slices_ = {j.at("slices")["first"],j.at("slices")["second"]};
    leaf_ = std::make_unique<TaperedLeaf>(j.at("leaf"));
}

void FernFrond::draw(Game& game, Material material) const {
    for (int i = 0; i < 3; i++) {
        leaf_->draw(game,leaf_transforms_[i],material);
    }
}
void FernFrond::draw(Game& game,Matrix transform, Material material) const {
    std::array<Matrix,3> t = leaf_transforms_;
    for (int i = 0; i < 3; i++) {
        t[i] = MatrixMultiply(t[i],transform);
    }
    for (int i = 0; i < 3; i++) {
        leaf_->draw(game,t[i],material);
    }
    // upper_petal_->draw_instanced(game,material,ut.data(),ut.size());
    // lower_petal_->draw_instanced(game,material,lt.data(),lt.size());
}

void FernFrond::draw_instanced(Game& game, Material material, const Matrix* transforms, int matrix_count) const {
    std::vector<Matrix> t (matrix_count*3);
    for (int i = 0; i < matrix_count*3; i++) {
        t[i] = MatrixMultiply(leaf_transforms_[i%3],transforms[i/3]);
    }
    for (int i = 0; i < t.size(); i++) {
        leaf_->draw(game,t[i],material);
    }
    // upper_petal_->draw_instanced(game,material,ut.data(),ut.size());
    // lower_petal_->draw_instanced(game,material,lt.data(),lt.size());
}

void FernFrond::initialize() {
    if (lsystem_.get_base() == nullptr)
        lsystem_ = LSystem(std::make_shared<LNode>("B",position_,Vector3{1,0,0},std::vector<std::shared_ptr<LNode>>{},nullptr));
        
        // base rule
        std::function<std::vector<std::shared_ptr<LNode>>(std::shared_ptr<LNode> node, std::mt19937_64& rng)> base = [](std::shared_ptr<LNode> node, std::mt19937_64& rng){
            std::shared_ptr<LNode> stipe = std::make_shared<LNode>("S",node->position,node->direction,std::vector<std::shared_ptr<LNode>>{},node);
            std::shared_ptr<LNode> rachi = std::make_shared<LNode>("R",node->position,node->direction,std::vector<std::shared_ptr<LNode>>{},node);
            std::shared_ptr<LNode> head = std::make_shared<LNode>("H",node->position,node->direction,std::vector<std::shared_ptr<LNode>>{},node);
            
            return result;
        };
}

void FernFrond::update_matrix() {
    // TODO: update matrix
}

BoundingBox FernFrond::get_bounding_box() const {
    // TODO: get bounding box
    return BoundingBox{};
}
BoundingBox FernFrond::get_bounding_box(Matrix transform) const {
    Matrix m = MatrixMultiply(transform_, transform);
    // TODO: get bounding box
    return BoundingBox{};
}

void FernFrond::generate_mesh() {
    leaf_->generate_mesh();
    // TODO: generate stem mesh
    update_matrix();
}

void FernFrond::set_slices(std::pair<int,int> slices) {
    leaf_->set_slices(slices);
}
void FernFrond::initialize_parameters() {
    std::mt19937_64 rng(seed_);
    // TODO: initialize parameters
}