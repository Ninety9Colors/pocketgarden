#pragma once
#include <string>
#include <vector>

#include "raylib.h"

struct SplineNode {
    Vector3 position;
    Vector3 tangent;
};

class Spline {
public:
    Spline();
    Spline(std::string data);

    int size() const;

    void add(Vector3 position);
    void insert(int index, Vector3 position);
    void update_position(int index, Vector3 new_position);
    Vector3 get(float t) const;
    const SplineNode& get_node(int index) const;

    std::string to_string() const;
private:
    void update_tangent(int index);

    std::vector<SplineNode> nodes_;
};