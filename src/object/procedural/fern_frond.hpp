#pragma once
#include <random>
#include <array>
#include <memory>
#include "object/object3d.hpp"
#include "object/procedural/parameter.hpp"
#include "object/procedural/tapered_petal.hpp"

#include "raylib.h"

class FernFrond : public ParameterObject {
public:
    FernFrond();
    FernFrond(uint32_t seed);
    FernFrond(Quaternion quaternion, Vector3 position, float scale);
    FernFrond(Quaternion quaternion, Vector3 position, float scale, uint32_t seed);
    FernFrond(const json& j);
    virtual ~FernFrond() {};

    void draw(Game& game, Material material) const override;
    void draw(Game& game,Matrix transform, Material material) const override;
    void draw_instanced(Game& game, const Matrix* transforms, int matrix_count) const override {
        draw_instanced(game,material_,transforms,matrix_count);
    }
    void draw_instanced(Game& game, Material material, const Matrix* transforms, int matrix_count) const override;

    void grow();
    // should be called explicitly after object creation
    void initialize();

    void update_matrix() override;

    BoundingBox get_bounding_box() const override;
    BoundingBox get_bounding_box(Matrix transform) const override;

    void generate_mesh() override;
    void set_slices(std::pair<int,int> slices);

    json to_json() const override;
    void from_json(const json& j) override;
private:
    void initialize_parameters() override;
    std::unique_ptr<TaperedLeaf> leaf_;
    std::vector<Matrix> leaf_transforms_;
    std::vector<std::pair<Matrix,Matrix>> leaf_transforms_base_;

    LSystem lsystem_;
    RuleSet productions_;
    std::mt19937_64 rng_;

    std::pair<int,int> slices_;
};