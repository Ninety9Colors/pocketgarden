#include <assert.h>
#include <iostream>
#include <limits>
#include <cstdint>
#include <cmath>
#include "rotate_tool.hpp"

#include "raylib.h"

#include "cube.hpp"
#include "maincamera.hpp"
#include "util.hpp"

RotateTool::RotateTool() : Item(), rotate_speed_(PI/2), axis_{1.0f,0.0f,0.0f} {
    held_id_ = 0;
    material_.maps[MATERIAL_MAP_DIFFUSE].color = BLUE;
    UnloadMesh(mesh_);
    mesh_ = GenMeshCube(0.25f,0.25f,0.25f);
    update_matrix();
}

RotateTool::RotateTool(std::string data) : Item() {
    std::vector<std::string> split = split_string(data);
    assert(split[0] == "RotateTool" && split.size() == 13);
    position_ = Vector3{std::stof(split[1]), std::stof(split[2]), std::stof(split[3])};
    rotate_speed_ = std::stof(split[4]);
    axis_ = Vector3{std::stof(split[5]),std::stof(split[6]),std::stof(split[7])};
    scale_ = std::stof(split[8]);
    quaternion_ = Quaternion{std::stof(split[9]),std::stof(split[10]),std::stof(split[11]),std::stof(split[12])};
    held_id_ = 0;
    material_.maps[MATERIAL_MAP_DIFFUSE].color = BLUE;
    UnloadMesh(mesh_);
    mesh_ = GenMeshCube(0.25f,0.25f,0.25f);
    update_matrix();
}

RotateTool::RotateTool(Vector3 position, float scale) : Item(position,scale), rotate_speed_(PI/2), axis_{1.0f,0.0f,0.0f} {
    held_id_ = 0;
    material_.maps[MATERIAL_MAP_DIFFUSE].color = BLUE;
    UnloadMesh(mesh_);
    mesh_ = GenMeshCube(0.25f,0.25f,0.25f);
    update_matrix();
}

void RotateTool::use(std::map<std::string, std::shared_ptr<Event>>& event_buffer, const MainCamera& camera, std::shared_ptr<Player> user, std::shared_ptr<World> world, const std::vector<bool>& keybinds, float dt) {
    if (in_use()) {
        if (world->get_objects().find(held_id_) == world->get_objects().end()) {
            held_id_ = 0;
        } else {
            std::shared_ptr<Object3d> held_item = world->get_objects().at(held_id_);
            if (keybinds[10])
                held_item->rotate_axis(axis_,rotate_speed_*dt);
            else if (keybinds[11])
                held_item->rotate_axis(axis_,-rotate_speed_*dt);
            // if (event_buffer.find("ObjectMoveEvent") != event_buffer.end()) {
            //     std::dynamic_pointer_cast<ObjectMoveEvent>(event_buffer["ObjectMoveEvent"])->add(held_id_,held_item->get_position());
            // } else {
            //     ObjectMoveEvent move_event = ObjectMoveEvent(std::map<uint32_t, Vector3>{}, user->get_username());
            //     move_event.add(held_id_,held_item->get_position());
            //     event_buffer["ObjectMoveEvent"] = std::make_shared<ObjectMoveEvent>(move_event);
            // }
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
            if (keybinds[10])
                held_item->rotate_axis(axis_,rotate_speed_*dt);
            else if (keybinds[11])
                held_item->rotate_axis(axis_,-rotate_speed_*dt);
        }
    }
}

void RotateTool::prepare_drop(std::map<std::string, std::shared_ptr<Event>>& event_buffer, const MainCamera& camera, std::shared_ptr<Player> user, std::shared_ptr<World> world, const std::vector<bool>& keybinds, float dt) {
    held_id_ = 0;
}

bool RotateTool::in_use() const {
    return held_id_ != 0;
}

std::string RotateTool::to_string() const {
    std::string result = "RotateTool " + 
        std::to_string(position_.x) + " " + std::to_string(position_.y) + " " + std::to_string(position_.z) + " " +
        std::to_string(rotate_speed_) + " " +
        std::to_string(axis_.x) + " " + std::to_string(axis_.y) + " " + std::to_string(axis_.z) + " " +
        std::to_string(scale_) + " " +
        std::to_string(quaternion_.x) + " " + std::to_string(quaternion_.y) + " " + std::to_string(quaternion_.z) + " " + std::to_string(quaternion_.w);
    return result;
}