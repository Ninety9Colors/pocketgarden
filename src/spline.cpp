#include <cassert>

#include "spline.hpp"
#include "util.hpp"

#include "raymath.h"

Spline::Spline() : nodes_() {}
Spline::Spline(std::string data) {
    std::vector<std::string> first_split = split_string(data);
    assert(first_split[0] == "Spline");
    std::vector<std::string> node_data = split_string(first_split[1]);
    for (const std::string& s : node_data) {
        std::vector<std::string> pos = split_string(s);
        nodes_.push_back(SplineNode{Vector3{std::stof(pos[0]), std::stof(pos[1]), std::stof(pos[2])},Vector3{0,0,0}});
    }
    for (int i = 0; i < nodes_.size(); i++)
        update_tangent(i);
}

int Spline::size() const {return nodes_.size();}

void Spline::add(Vector3 position) {
    nodes_.push_back(SplineNode{position, Vector3{0,0,0}});
    update_tangent(nodes_.size()-1);
    update_tangent(nodes_.size()-2);
}

void Spline::insert(int index, Vector3 position) {
    assert(index >= 0);
    if (index >= nodes_.size()) {
        add(position);
        return;
    }
    nodes_.insert(nodes_.begin()+index,SplineNode{position,Vector3{0,0,0}});
    update_tangent(index);
    update_tangent(index+1);
    update_tangent(index-1);
}

void Spline::update_position(int index, Vector3 new_position) {
    if (index >= nodes_.size() || index < 0)
        return;
    nodes_[index].position = new_position;
    update_tangent(index);
    update_tangent(index+1);
    update_tangent(index-1);
}

Vector3 Spline::get(float t) const {
    if (nodes_.size() <= 1)
        return {0,0,0};
    int index = (int) t;
    float t_1 = t-(int)t;
    if (t_1 == 0.0f) return nodes_[index].position;
    assert(t_1 < 1 && t_1 > 0);
    assert(index >= 0);
    const SplineNode& node_one = nodes_[index];
    const SplineNode& node_two = nodes_[index+1];
    const Vector3& p0 = node_one.position;
    const Vector3& m0 = node_one.tangent;
    const Vector3& p1 = node_two.position;
    const Vector3& m1 = node_two.tangent;
    Vector3 a = Vector3Add(Vector3Add(Vector3Add(Vector3Scale(p0,2.0f), m0),Vector3Scale(p1,-2.0f)),m1);
    Vector3 b = Vector3Add(Vector3Add(Vector3Add(Vector3Scale(p0,-3.0f), Vector3Scale(p1,3.0f)),Vector3Scale(m0,-2.0f)),Vector3Scale(m1,-1.0f));
    Vector3 c = m0;
    Vector3 d = p0;
    float t_2 = t_1*t_1;
    float t_3 = t_2*t_1;
    return a*t_3 + b*t_2 + c*t_1 + d;
}

const SplineNode& Spline::get_node(int index) const {
    assert(index >= 0 || index < nodes_.size());
    return nodes_[index];
}

std::string Spline::to_string() const {
    std::string result = "Spline (";
    for (const SplineNode& node : nodes_) {
        result += "(" + std::to_string(node.position.x) + " " + std::to_string(node.position.y) + " " + std::to_string(node.position.z) + ")";
    }
    result += ")";
    return result;
}

void Spline::update_tangent(int index) {
    if (index >= nodes_.size() || index < 0)
        return;
    bool before = index > 0;
    bool after = index < nodes_.size()-1;
    Vector3 pos1 = nodes_[index].position;
    Vector3 pos2 = nodes_[index].position;
    if (before > 0)
        pos1 = nodes_[index-1].position;
    if (after)
        pos2 = nodes_[index+1].position;
    nodes_[index].tangent = Vector3Scale(Vector3Subtract(pos2,pos1),1.0f - 0.5f*(before&&after));
}