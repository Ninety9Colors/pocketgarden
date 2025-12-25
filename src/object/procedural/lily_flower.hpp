#pragma once
#include <random>
#include <array>
#include <memory>
#include "object/object3d.hpp"
#include "object/procedural/tapered_petal.hpp"
#include "object/procedural/parameter.hpp"

#include "raylib.h"

class LilyFlower : public ParameterObject {
public:
    LilyFlower();
    LilyFlower(uint32_t seed);
    LilyFlower(Quaternion quaternion, Vector3 position, float scale);
    LilyFlower(Quaternion quaternion, Vector3 position, float scale, uint32_t seed);
    LilyFlower(const json& j);
    virtual ~LilyFlower() {};

    void draw(Game& game) const override;
    void draw(Game& game,Matrix transform) const override;
    void draw_offset(Game& game,float x, float y, float z) const override;

    void update_matrix() override;

    BoundingBox get_bounding_box() const override;
    BoundingBox get_bounding_box(Matrix transform) const override;

    void generate_mesh() override;
    void set_slices(std::pair<int,int> slices);

    json to_json() const override;
    void from_json(const json& j) override;
private:
    void initialize_parameters() override;

    std::unique_ptr<TaperedPetal> upper_petal_;
    std::unique_ptr<TaperedPetal> lower_petal_;

    std::array<Matrix,3> upper_transforms_;
    std::array<Matrix,3> lower_transforms_;

    std::pair<int,int> slices_;
};