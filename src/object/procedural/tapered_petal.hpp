#pragma once
#include <random>
#include "object/object3d.hpp"
#include "object/procedural/parameter.hpp"

#include "raylib.h"

class TaperedPetal : public ParameterObject {
public:
    TaperedPetal();
    TaperedPetal(uint32_t seed);
    TaperedPetal(Quaternion quaternion, Vector3 position, float scale);
    TaperedPetal(Quaternion quaternion, Vector3 position, float scale, uint32_t seed);
    TaperedPetal(const json& j);
    virtual ~TaperedPetal() {};

    void set_slices(std::pair<int,int> slices);
    void generate_mesh() override;

    Vector3 tip_vector() const; // root to tip
    float base_width() const;

    json to_json() const override;
    void from_json(const json& j) override;
private:
    float X(float u, float v) const;
    float Y(float u, float v) const;
    float Z(float u, float v) const;

    void initialize_parameters() override;

    std::pair<int,int> slices_;
};