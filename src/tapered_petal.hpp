#pragma once
#include "object3d.hpp"

#include "raylib.h"

class TaperedPetal : public ParameterObject {
public:
    TaperedPetal();
    TaperedPetal(float scale);
    TaperedPetal(Vector3 position, float scale);
    TaperedPetal(std::string data);

    void generate_mesh() override;
    void generate_mesh(uint64_t seed);
    std::string to_string() const override;

    void set_slices(std::pair<int,int> slices);
private:
    float X(float u, float v) const;
    float Y(float u, float v) const;
    float Z(float u, float v) const;
    void initialize_parameters() override;
    std::pair<int,int> slices_;
    std::mt19937_64 rng_;
};