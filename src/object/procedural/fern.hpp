#pragma once

#include "object/procedural/fern_frond.hpp"
#include "object/object3d.hpp"

class Fern : public Object3d {
public:
    Fern();
    Fern(uint32_t seed);
    Fern(Quaternion quaternion, Vector3 position, float scale);
    Fern(Quaternion quaternion, Vector3 position, float scale, uint32_t seed);
    Fern(const json& j);
    virtual ~Fern() {};

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

    std::vector<std::unique_ptr<FernFrond>> ferns_;
    std::vector<Matrix> fern_transforms_;
    std::vector<std::array<Matrix,3>> fern_transforms_base_;
    
    int stage_;
    uint32_t seed_;
    std::mt19937_64 rng_;
};