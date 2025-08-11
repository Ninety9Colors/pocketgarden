#include <assert.h>
#include <iostream>
#include <limits>
#include <cstdint>
#include <cmath>
#include "move_tool.hpp"

#include "raylib.h"

#include "cube.hpp"
#include "maincamera.hpp"
#include "util.hpp"

MoveTool::MoveTool() : Item(), holding_distance_(2.0f) {
    held_id_ = 0;
    speed_ = 2.0f;
    material_.maps[MATERIAL_MAP_DIFFUSE].color = GREEN;
    UnloadMesh(mesh_);
    mesh_ = GenMeshCylinder(0.25f,0.5f,10);
    update_matrix();
}

MoveTool::MoveTool(std::string data) : Item() {
    std::vector<std::string> split = split_string(data);
    assert(split[0] == "MoveTool" && split.size() == 10);
    position_ = Vector3{std::stof(split[1]), std::stof(split[2]), std::stof(split[3])};
    holding_distance_ = std::stof(split[4]);
    scale_ = std::stof(split[5]);
    quaternion_ = Quaternion{std::stof(split[6]),std::stof(split[7]),std::stof(split[8]),std::stof(split[9])};
    held_id_ = 0;
    speed_ = 2.0f;
    material_.maps[MATERIAL_MAP_DIFFUSE].color = GREEN;
    UnloadMesh(mesh_);
    mesh_ = GenMeshCylinder(0.25f,0.5f,10);
    update_matrix();
}

MoveTool::MoveTool(Vector3 position, float scale) : Item(position,scale), holding_distance_(2.0f) {
    held_id_ = 0;
    speed_ = 2.0f;
    material_.maps[MATERIAL_MAP_DIFFUSE].color = GREEN;
    UnloadMesh(mesh_);
    mesh_ = GenMeshCylinder(0.25f,0.5f,10);
    update_matrix();
}

void MoveTool::use(std::map<std::string, std::shared_ptr<Event>>& event_buffer, const MainCamera& camera, std::shared_ptr<Player> user, std::shared_ptr<World> world, const std::vector<bool>& keybinds, float dt) {
    constexpr float epsilon = 0.02f;
    if (in_use()) {
        if (world->get_objects().find(held_id_) == world->get_objects().end()) {
            held_id_ = 0;
        } else {
            std::shared_ptr<Object3d> held_item = world->get_objects().at(held_id_);
            if (keybinds[7])
                holding_distance_ += 0.5f;
            else if (keybinds[8])
                holding_distance_ -= 0.5f;
            Vector3 pos = held_item->get_position();
            float x = pos.x;
            float y = pos.y;
            float z = pos.z;
            float target_x = camera.get_position().x + camera.get_direction().x * holding_distance_;
            float target_y = std::max(0.0f,camera.get_position().y + camera.get_direction().y * holding_distance_);
            float target_z = camera.get_position().z + camera.get_direction().z * holding_distance_;
            if ((x <= (target_x-epsilon) || x >= (target_x+epsilon)) || (y <= (target_y-epsilon) || y >= (target_y+epsilon)) || (z <= (target_z-epsilon) || z >= (target_z+epsilon))) {
                float move_magnitude = std::sqrt(std::pow(x - target_x,2) + std::pow(y - target_y,2) + std::pow(z - target_z,2));
                Vector3 move_direction = {(target_x-x)/move_magnitude, (target_y-y)/move_magnitude, (target_z-z)/move_magnitude};
                float move_distance = speed_*dt*(move_magnitude);

                if (move_magnitude <= epsilon) {
                    held_item->set_position(Vector3{target_x, target_y, target_z});
                } else {
                    held_item->set_position(Vector3{x + move_direction.x*move_distance, y + move_direction.y*move_distance, z + move_direction.z*move_distance});
                }
                if (event_buffer.find("ObjectMoveEvent") != event_buffer.end()) {
                    std::dynamic_pointer_cast<ObjectMoveEvent>(event_buffer["ObjectMoveEvent"])->add(held_id_,held_item->get_position());
                } else {
                    ObjectMoveEvent move_event = ObjectMoveEvent(std::map<uint32_t, Vector3>{}, user->get_username());
                    move_event.add(held_id_,held_item->get_position());
                    event_buffer["ObjectMoveEvent"] = std::make_shared<ObjectMoveEvent>(move_event);
                }
            }
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
                std::cout << "Hitting object " << p.second->to_string() << "\n";
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
            Vector3 pos = held_item->get_position();
            float x = pos.x;
            float y = pos.y;
            float z = pos.z;
            float target_x = camera.get_position().x + camera.get_direction().x * holding_distance_;
            float target_y = std::max(0.0f,camera.get_position().y + camera.get_direction().y * holding_distance_);
            float target_z = camera.get_position().z + camera.get_direction().z * holding_distance_;
            if ((x >= (target_x-epsilon) && x <= (target_x+epsilon)) && (y >= (target_y-epsilon) && y <= (target_y+epsilon)) && (z >= (target_z-epsilon) && z <= (target_z+epsilon)))
                return;
            float move_magnitude = std::sqrt(std::pow(x - target_x,2) + std::pow(y - target_y,2) + std::pow(z - target_z,2));
            Vector3 move_direction = {(target_x-x)/move_magnitude, (target_y-y)/move_magnitude, (target_z-z)/move_magnitude};
            float move_distance = speed_*dt*(move_magnitude);

            if (move_magnitude <= epsilon) {
                held_item->set_position(Vector3{target_x, target_y, target_z});
            } else {
                held_item->set_position(Vector3{x + move_direction.x*move_distance, y + move_direction.y*move_distance, z + move_direction.z*move_distance});
            }
            if (event_buffer.find("ObjectMoveEvent") != event_buffer.end()) {
                std::dynamic_pointer_cast<ObjectMoveEvent>(event_buffer["ObjectMoveEvent"])->add(held_id_,held_item->get_position());
            } else {
                ObjectMoveEvent move_event = ObjectMoveEvent(std::map<uint32_t, Vector3>{}, user->get_username());
                move_event.add(held_id_,held_item->get_position());
                event_buffer["ObjectMoveEvent"] = std::make_shared<ObjectMoveEvent>(move_event);
            }
        }
    }
}

void MoveTool::prepare_drop(std::map<std::string, std::shared_ptr<Event>>& event_buffer, const MainCamera& camera, std::shared_ptr<Player> user, std::shared_ptr<World> world, const std::vector<bool>& keybinds, float dt) {
    held_id_ = 0;
}

bool MoveTool::in_use() const {
    return held_id_ != 0;
}

std::string MoveTool::to_string() const {
    std::string result = "MoveTool " + 
        std::to_string(position_.x) + " " + std::to_string(position_.y) + " " + std::to_string(position_.z) + " " +
        std::to_string(holding_distance_) + " " +
        std::to_string(scale_) + " " +
        std::to_string(quaternion_.x) + " " + std::to_string(quaternion_.y) + " " + std::to_string(quaternion_.z) + " " + std::to_string(quaternion_.w);
    return result;
}