#pragma once

#include "object/procedural/lily_flower.hpp"
#include "object/object3d.hpp"

class Lily : public Object3d {
public:
    Lily();
    Lily(uint32_t seed);
    Lily(Quaternion quaternion, Vector3 position, float scale);
    Lily(Quaternion quaternion, Vector3 position, float scale, uint32_t seed);
    Lily(const json& j);
    virtual ~Lily() {};

    void draw(Game& game, Material material) const override;

    // Should regenerate mesh if needed
    void advance_stage();

    // Should be called explicitly after the object is created
    void initialize();
    virtual void generate_mesh() override;

    json to_json() const override;
    void from_json(const json& j) override;
private:
    virtual void update_matrix() override;

    std::unique_ptr<LilyFlower> flower_;
    std::vector<Matrix> flower_transforms_;
    std::vector<std::pair<Matrix,Matrix>> flower_transforms_base_;

    std::unique_ptr<TaperedPetal> leaf_;
    std::vector<Matrix> leaf_transforms_;
    std::vector<std::pair<Matrix,Matrix>> leaf_transforms_base_;
    
    LSystem lsystem_;
    std::map<std::pair<int,int>, RuleSet> stage_transitions_;
    int stage_;
    uint32_t seed_;
};