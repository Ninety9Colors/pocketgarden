#include <assert.h>
#include <limits>
#include <cstdint>
#include <cmath>

#include "raylib.h"
#include "raymath.h"

#include "logging.hpp"
#include "object/consistent/rotate_tool.hpp"
#include "object/consistent/cube.hpp"
#include "player/maincamera.hpp"
#include "game.hpp"


RotateTool::RotateTool() : Item(), rotate_speed_(PI/2), axis_{1.0f,0.0f,0.0f}, held_id_(0) {
    generate_mesh();
    update_matrix();
}

RotateTool::RotateTool(const json& j) {
    from_json(j);
    generate_mesh();
    update_matrix();
}

RotateTool::RotateTool(Quaternion quaternion, Vector3 position, float scale) : Item(quaternion, position,scale), rotate_speed_(PI/2), axis_{1.0f,0.0f,0.0f}, held_id_(0) {
    generate_mesh();
    update_matrix();
}

void RotateTool::generate_mesh() {
    material_.maps[MATERIAL_MAP_DIFFUSE].color = BLUE;
    mesh_ = GenMeshCube(0.25f,0.25f,0.25f);
}

void RotateTool::from_json(const json& j) {
    object_type_ = j.at("object_type");
    position_ = {j.at("position")["x"],j.at("position")["y"],j.at("position")["z"]};
    axis_ = {j.at("axis")["x"],j.at("axis")["y"],j.at("axis")["z"]};
    scale_ = j.at("scale");
    held_id_ = j.at("held_id");
    rotate_speed_ = j.at("rotate_speed");
    quaternion_ = {j.at("quaternion")["x"],j.at("quaternion")["y"],j.at("quaternion")["z"],j.at("quaternion")["w"]};
}

json RotateTool::to_json() const {
    json j = {
        {"type","RotateTool"},
        {"object_type",object_type_},
        {"position",{{"x",position_.x},{"y",position_.y},{"z",position_.z}}},
        {"axis",{{"x",axis_.x},{"y",axis_.y},{"z",axis_.z}}},
        {"scale",scale_},
        {"held_id",held_id_},
        {"rotate_speed",rotate_speed_},
        {"quaternion",{{"x",quaternion_.x},{"y",quaternion_.y},{"z",quaternion_.z},{"w",quaternion_.w}}}
    };
    return j;
}

void RotateTool::use(Game& game, std::string username, const std::vector<bool>& keybinds, float dt) {
    if (held_id_ != 0) {
        if (!game.get_world().contains_object(held_id_)) {
            WARN("Tried to rotate nonexistent id " + std::to_string(held_id_) + ", resetting to 0");
            held_id_ = 0;
        } else {
            if (keybinds[12]) { // "r" key
                float a = axis_.x;
                axis_.x = axis_.z;
                axis_.z = axis_.y;
                axis_.y = a;
            }
            if (keybinds[10]) // q
                game.get_world().rotate_object_axis(held_id_,axis_,rotate_speed_*dt);
            else if (keybinds[11]) // e
                game.get_world().rotate_object_axis(held_id_,axis_,-rotate_speed_*dt);
            if (keybinds[10] || keybinds[11])
                game.queue_event_send(std::make_unique<ObjectRotateEvent>(held_id_,game.get_world().get_object_quaternion(held_id_),username));
        }
    }
    if (!keybinds[6]) // mouse left click
        return;
    uint32_t nearest = game.get_world().raycast_nearest({game.get_camera().get_position(), game.get_camera().get_direction()});
    INFO("Player " + username + " clicked with RotateTool and found id: " + std::to_string(nearest));
    held_id_ = nearest;
}

void RotateTool::on_drop(Game& game, std::string username, const std::vector<bool>& keybinds, float dt) {
    held_id_ = 0;
}

void RotateTool::draw(Game& game, Material material) const {
    Item::draw(game,material);
    if (held_id_ == 0) return;
    Vector3 pos = game.get_world().get_object_position(held_id_);
    BoundingBox bb = game.get_world().get_object_bounding_box(held_id_);
    DrawLine3D(pos, Vector3Add(pos,axis_*Vector3Distance(bb.max, bb.min)*2), WHITE);
}
void RotateTool::draw(Game& game, Matrix transform, Material material) const {
    Item::draw(game,transform,material);
    if (held_id_ == 0) return;
    Vector3 pos = game.get_world().get_object_position(held_id_);
    BoundingBox bb = game.get_world().get_object_bounding_box(held_id_);
    DrawLine3D(pos, Vector3Add(pos,axis_*Vector3Distance(bb.max, bb.min)*2), WHITE);
}