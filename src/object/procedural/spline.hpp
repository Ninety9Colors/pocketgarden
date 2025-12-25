#pragma once
#include <string>
#include <vector>

#include "json.hpp"
using json = nlohmann::json;

#include "raylib.h"

struct SplineNode {
    Vector3 position;
    Vector3 tangent;
};

class Spline {
public:
    Spline();
    Spline(const json& j);

    int size() const;

    void add(Vector3 position);
    void insert(int index, Vector3 position);
    void update_position(int index, Vector3 new_position);
    Vector3 get(float t) const;
    const SplineNode& get_node(int index) const;

    json to_json() const;
    void from_json(const json& j);
private:
    void update_tangent(int index);

    std::vector<SplineNode> nodes_;
};