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
    held_id_ = 0;
    speed_ = 2.0f;
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
    held_id_ = 0;
    speed_ = 2.0f;
}

MoveTool::MoveTool(Vector3 position, float scale) : position_(position), scale_(scale_), holding_distance_(2.0f) {
    model_.push_back(std::make_unique<Cube>(Vector3{0.0f,0.0f,0.0f}, Vector3{1.0f,1.0f,2.0f}, 0.2f, YELLOW));
    model_.push_back(std::make_unique<Cube>(Vector3{0.0f,0.0f,1.5f*0.2f}, Vector3{1.0f,1.0f,1.0f}, 0.2f, LIGHTGRAY));
    held_id_ = 0;
    speed_ = 2.0f;
}

void MoveTool::use(std::map<std::string, std::shared_ptr<Event>>& event_buffer, const MainCamera& camera, std::shared_ptr<Player> user, std::shared_ptr<World> world, const std::vector<bool>& keybinds, float dt) {
    if (in_use()) {
        std::shared_ptr<Object3d> held_item = world->get_objects().at(held_id_);
        if (keybinds[7])
            holding_distance_ += 0.1f;
        else if (keybinds[8])
            holding_distance_ -= 0.1f;
        float target_x = camera.get_position().x + camera.get_direction().x * holding_distance_;
        float target_y = camera.get_position().y + camera.get_direction().y * holding_distance_;
        float target_z = camera.get_position().z + camera.get_direction().z * holding_distance_;
        float move_magnitude = std::sqrt(std::pow(held_item->get_x() - target_x,2) + std::pow(held_item->get_y() - target_y,2) + std::pow(held_item->get_z() - target_z,2));
        Vector3 move_direction = {(-held_item->get_x() + target_x)/move_magnitude, (-held_item->get_y() + target_y)/move_magnitude, (-held_item->get_z() + target_z)/move_magnitude};
        float move_distance = speed_*dt*(move_magnitude);

        if (move_magnitude <= move_distance) {
            held_item->set_x(target_x);
            held_item->set_y(target_y);
            held_item->set_z(target_z);
        } else {
            held_item->set_x(held_item->get_x() + move_direction.x*move_distance);
            held_item->set_y(held_item->get_y() + move_direction.y*move_distance);
            held_item->set_z(held_item->get_z() + move_direction.z*move_distance);
        }
        if (event_buffer.find("ObjectMoveEvent") != event_buffer.end()) {
            std::dynamic_pointer_cast<ObjectMoveEvent>(event_buffer["ObjectMoveEvent"])->update(held_id_,Vector3{held_item->get_x(), held_item->get_y(), held_item->get_z()});
        } else {
            ObjectMoveEvent move_event = ObjectMoveEvent(std::map<uint32_t, Vector3>{});
            move_event.update(held_id_,Vector3{held_item->get_x(), held_item->get_y(), held_item->get_z()});
            event_buffer["ObjectMoveEvent"] = std::make_shared<ObjectMoveEvent>(move_event);
        }
    }
    if (!keybinds[6])
        return;
    if (in_use()) {
        held_id_ = 0;
    } else {
        Ray ray = Ray{camera.get_position(), camera.get_direction()};
        uint32_t nearest = 0;
        float min_distance = std::numeric_limits<float>::infinity();
        for (const auto& p : world->get_objects()) {
            if (p.second.get() == this) {
                continue;
            }
            RayCollision c = GetRayCollisionBox(ray, p.second->get_bounding_box());
            if (c.hit) {
                float d = c.distance;
                if (d < min_distance) {
                    nearest = p.first;
                    min_distance = d;
                }
            }
        }
        if (nearest != 0) {
            held_id_ = nearest;
            std::shared_ptr<Object3d> held_item = world->get_objects().at(held_id_);
            float target_x = camera.get_position().x + camera.get_direction().x * holding_distance_;
            float target_y = camera.get_position().y + camera.get_direction().y * holding_distance_;
            float target_z = camera.get_position().z + camera.get_direction().z * holding_distance_;
            float move_magnitude = std::sqrt(std::pow(held_item->get_x() - target_x,2) + std::pow(held_item->get_y() - target_y,2) + std::pow(held_item->get_z() - target_z,2));
            Vector3 move_direction = {(-held_item->get_x() + target_x)/move_magnitude, (-held_item->get_y() + target_y)/move_magnitude, (-held_item->get_z() + target_z)/move_magnitude};
            float move_distance = speed_*dt*(move_magnitude);

            if (move_magnitude <= move_distance) {
                held_item->set_x(target_x);
                held_item->set_y(target_y);
                held_item->set_z(target_z);
            } else {
                held_item->set_x(held_item->get_x() + move_direction.x*move_distance);
                held_item->set_y(held_item->get_y() + move_direction.y*move_distance);
                held_item->set_z(held_item->get_z() + move_direction.z*move_distance);
            }
            if (event_buffer.find("ObjectMoveEvent") != event_buffer.end()) {
                std::dynamic_pointer_cast<ObjectMoveEvent>(event_buffer["ObjectMoveEvent"])->update(held_id_,Vector3{held_item->get_x(), held_item->get_y(), held_item->get_z()});
            } else {
                ObjectMoveEvent move_event = ObjectMoveEvent(std::map<uint32_t, Vector3>{});
                move_event.update(held_id_,Vector3{held_item->get_x(), held_item->get_y(), held_item->get_z()});
                event_buffer["ObjectMoveEvent"] = std::make_shared<ObjectMoveEvent>(move_event);
            }
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
    return held_id_ != 0;
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