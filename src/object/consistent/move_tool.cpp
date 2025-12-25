#include <assert.h>
#include <limits>
#include <cstdint>
#include <cmath>

#include "raylib.h"

#include "logging.hpp"
#include "object/consistent/cube.hpp"
#include "object/consistent/move_tool.hpp"
#include "player/maincamera.hpp"

MoveTool::MoveTool() : Item(), holding_distance_(2.0f), held_id_(0), speed_(2.0f) {
    generate_mesh();
    update_matrix();
}
MoveTool::MoveTool(const json& j) {
    from_json(j);
    generate_mesh();
    update_matrix();
}

MoveTool::MoveTool(Quaternion quaternion, Vector3 position, float scale) : Item(quaternion, position,scale), holding_distance_(2.0f), held_id_(0), speed_(2.0f) {
    generate_mesh();
    update_matrix();
}

void MoveTool::generate_mesh() {
    material_.maps[MATERIAL_MAP_DIFFUSE].color = GREEN;
    mesh_ = GenMeshCylinder(0.25f,0.5f,10);
}

json MoveTool::to_json() const {
    json j = {
        {"type","MoveTool"},
        {"object_type",object_type_},
        {"position",{{"x",position_.x},{"y",position_.y},{"z",position_.z}}},
        {"held_id",held_id_},
        {"scale",scale_},
        {"holding_distance",holding_distance_},
        {"speed",speed_},
        {"quaternion",{{"x",quaternion_.x},{"y",quaternion_.y},{"z",quaternion_.z},{"w",quaternion_.w}}}
    };
    return j;
}
void MoveTool::from_json(const json& j) {
    object_type_ = j.at("object_type");
    position_ = {j.at("position")["x"],j.at("position")["y"],j.at("position")["z"]};
    held_id_ = j.at("held_id");
    scale_ = j.at("scale");
    quaternion_ = {j.at("quaternion")["x"],j.at("quaternion")["y"],j.at("quaternion")["z"],j.at("quaternion")["w"]};
    speed_ = j.at("speed");
    holding_distance_ = j.at("holding_distance");
    generate_mesh();
}

void MoveTool::use(Game& game, std::string username, const std::vector<bool>& keybinds, float dt) {
    constexpr float epsilon = 0.02f; // distance at which where the object stops moving toward pointed location
    if (held_id_ != 0) {
        if (!game.get_world().contains_object(held_id_)) {
            WARN("Tried to move nonexistent id " + std::to_string(held_id_) + ", resetting to 0");
            held_id_ = 0;
        } else {
            if (keybinds[7])
                holding_distance_ += 0.5f;
            else if (keybinds[8])
                holding_distance_ -= 0.5f;
            Vector3 pos = game.get_world().get_object_position(held_id_);
            float x = pos.x;
            float y = pos.y;
            float z = pos.z;
            MainCamera& camera = game.get_camera();
            float target_x = camera.get_position().x + camera.get_direction().x * holding_distance_;
            float target_y = std::max(0.0f,camera.get_position().y + camera.get_direction().y * holding_distance_);
            float target_z = camera.get_position().z + camera.get_direction().z * holding_distance_;
            if (pos.x != target_x || pos.y != target_y || pos.z != target_z) {
                float move_magnitude = std::sqrt(std::pow(x - target_x,2) + std::pow(y - target_y,2) + std::pow(z - target_z,2));
                Vector3 move_direction = {(target_x-x)/move_magnitude, (target_y-y)/move_magnitude, (target_z-z)/move_magnitude};
                float move_distance = speed_*dt*(move_magnitude);
                if (move_magnitude <= epsilon) {
                    game.get_world().move_object(held_id_,Vector3{target_x, target_y, target_z});
                } else {
                    game.get_world().move_object(held_id_,Vector3{x + move_direction.x*move_distance, y + move_direction.y*move_distance, z + move_direction.z*move_distance});
                }
                game.queue_event_send(std::make_unique<ObjectMoveEvent>(held_id_,game.get_world().get_object_position(held_id_),username));
            }
        }
    }
    if (!keybinds[6]) // mouse left click
        return;
    if (held_id_ != 0) {
        held_id_ = 0;
        return;
    }
    uint32_t nearest = game.get_world().raycast_nearest({game.get_camera().get_position(), game.get_camera().get_direction()});
    INFO("Player " + username + " clicked with MoveTool and found id: " + std::to_string(nearest));
    held_id_ = nearest;
}

void MoveTool::on_drop(Game& game, std::string username, const std::vector<bool>& keybinds, float dt) {
    held_id_ = 0;
}