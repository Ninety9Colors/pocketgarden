#pragma once
#include <random>
#include "object/object3d.hpp"
#include "object/procedural/parameter.hpp"

#include "raylib.h"

class TaperedPetal : public ParameterObject {
public:
    TaperedPetal();
    TaperedPetal(float scale);
    TaperedPetal(Vector3 position, float scale);
    TaperedPetal(ParameterMap map, uint64_t seed, Quaternion quaternion, Vector3 position, float scale);
    TaperedPetal(std::string data);

    void set_slices(std::pair<int,int> slices);
    void generate_mesh() override;
    void generate_mesh(uint64_t seed);

    Vector3 tip_vector() const;
    float base_width() const;

    std::string to_string() const override;
private:
    float X(float u, float v) const;
    float Y(float u, float v) const;
    float Z(float u, float v) const;
    void initialize_parameters() override;
    std::pair<int,int> slices_;
    uint64_t seed_;
};