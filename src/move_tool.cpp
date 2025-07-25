#include <assert.h>
#include <iostream>
#include <limits>
#include <cmath>
#include "move_tool.hpp"

#include "raylib.h"

#include "cube.hpp"
#include "maincamera.hpp"
#include "util.hpp"

MoveTool::MoveTool() : position_{0.0f, 0.0f, 0.0f}, scale_(1.0f), holding_distance_(2.0f) {
    model_.push_back(std::make_unique<Cube>(Vector3{0.0f,0.0f,0.0f}, Vector3{1.0f,1.0f,2.0f}, 0.2f, YELLOW));
    model_.push_back(std::make_unique<Cube>(Vector3{0.0f,0.0f,1.5f*0.2f}, Vector3{1.0f,1.0f,1.0f}, 0.2f, LIGHTGRAY));
    held_item = nullptr;
}

MoveTool::MoveTool(std::string data) {
    std::vector<std::string> split = split_string(data);
    assert(split[0] == "MoveTool" && split.size() == 7);
    position_ = Vector3{std::stof(split[1]), std::stof(split[2]), std::stof(split[3])};
    holding_distance_ = std::stof(split[4]);
    scale_ = std::stof(split[5]);
    std::vector<std::string> model_objects = split_string(split[6]);
    for (std::string object_data : model_objects) {
        if (get_first_word(object_data) == "Cube") {
            model_.push_back(std::make_unique<Cube>(object_data));
        }
    }
    held_item = nullptr;
}

MoveTool::MoveTool(Vector3 position, float scale) : position_(position), scale_(scale_), holding_distance_(2.0f) {
    model_.push_back(std::make_unique<Cube>(Vector3{0.0f,0.0f,0.0f}, Vector3{1.0f,1.0f,2.0f}, 0.2f, YELLOW));
    model_.push_back(std::make_unique<Cube>(Vector3{0.0f,0.0f,1.5f*0.2f}, Vector3{1.0f,1.0f,1.0f}, 0.2f, LIGHTGRAY));
    held_item = nullptr;
}

void MoveTool::use(const MainCamera& camera, std::shared_ptr<Player> user, std::shared_ptr<World> world, const std::vector<bool>& keybinds) {
    if (in_use()) {
        assert(held_item != nullptr);
        if (keybinds[7])
            holding_distance_ += 0.1f;
        else if (keybinds[8])
            holding_distance_ -= 0.1f;
        held_item->set_x(camera.get_position().x + camera.get_direction().x * holding_distance_);
        held_item->set_y(camera.get_position().y + camera.get_direction().y * holding_distance_);
        held_item->set_z(camera.get_position().z + camera.get_direction().z * holding_distance_);
    }
    if (!keybinds[6])
        return;
    if (in_use()) {
        held_item = nullptr;
    } else {
        Ray ray = Ray{camera.get_position(), camera.get_direction()};
        std::shared_ptr<Object3d> nearest = nullptr;
        float min_distance = std::numeric_limits<float>::infinity();
        for (auto object : world->get_objects()) {
            if (object.get() == this) {
                continue;
            }
            RayCollision c = GetRayCollisionBox(ray, object->get_bounding_box());
            if (c.hit) {
                float d = c.distance;
                if (d < min_distance) {
                    nearest = object;
                    min_distance = d;
                }
            }
        }
        if (nearest != nullptr) {
            held_item = nearest;
            held_item->set_x(camera.get_position().x + camera.get_direction().x * holding_distance_);
            held_item->set_y(camera.get_position().y + camera.get_direction().y * holding_distance_);
            held_item->set_z(camera.get_position().z + camera.get_direction().z * holding_distance_);
        }
    }
}

void MoveTool::draw() const {
    for (const auto& object : model_)
        object->draw_offset(position_.x, position_.y, position_.z);
}
void MoveTool::draw_outline() const {};
void MoveTool::draw_offset(float x, float y, float z) const {
    for (const auto& object : model_)
        object->draw_offset(position_.x+x, position_.y+y, position_.z+z);
}
void MoveTool::draw_outline_offset(float x, float y, float z) const {};

void MoveTool::set_x(float new_x) {
    position_.x = new_x;
}
void MoveTool::set_y(float new_y) {
    position_.y = new_y;
}
void MoveTool::set_z(float new_z) {
    position_.z = new_z;
}
float MoveTool::get_x() const {
    return position_.x;
}
float MoveTool::get_y() const {
    return position_.y;
}
float MoveTool::get_z() const {
    return position_.z;
}

BoundingBox MoveTool::get_bounding_box() const {
    //TODO IMPLEMENT
    return BoundingBox();
}

bool MoveTool::in_use() const {
    return held_item != nullptr;
}

std::string MoveTool::to_string() const {
    std::string result = "MoveTool " + 
        std::to_string(position_.x) + " " + std::to_string(position_.y) + " " + std::to_string(position_.z) + " " +
        std::to_string(holding_distance_) + " " +
        std::to_string(scale_) + " (";
    for (const auto& object : model_) {
        result += "(" + object->to_string() + ")";
    }
    result += ")";
    return result;
}