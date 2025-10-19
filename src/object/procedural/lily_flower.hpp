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
    LilyFlower(float scale);
    LilyFlower(Vector3 position, float scale);
    LilyFlower(ParameterMap map, uint64_t seed, Quaternion quaternion, Vector3 position, float scale);
    LilyFlower(std::string data);

    void draw() const override;
    void draw(Matrix transform) const override;
    void draw_offset(float x, float y, float z) const override;
    void set_shader(std::shared_ptr<Shader> shader) override;

    void update_matrix() override;

    BoundingBox get_bounding_box() const override;
    BoundingBox get_bounding_box(Matrix transform) const override;

    void generate_mesh() override;
    void generate_mesh(uint64_t seed);
    void set_slices(std::pair<int,int> slices);

    std::string to_string() const override;
private:
    void initialize_parameters() override;

    std::unique_ptr<TaperedPetal> upper_petal_;
    std::unique_ptr<TaperedPetal> lower_petal_;

    std::array<Matrix,3> upper_transforms_;
    std::array<Matrix,3> lower_transforms_;

    std::pair<int,int> slices_;
    uint64_t seed_;
};