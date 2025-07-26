#include <cassert>
#include <cmath>
#include <iostream>

#include "maincamera.hpp"
#include "move_tool.hpp"
#include "player.hpp"
#include "util.hpp"
#include "world.hpp"

Player::Player(std::string username, Vector3 position) : username_(username) {
    speed_ = 4.0f;
    online_ = false;
    hitbox_ = Cube(Vector3{0.0f,0.0f,0.0f}, Vector3{1.0f, 2.0f, 1.0f}, 1.0f, WHITE);
    set_position(position.x, position.y, position.z);
    auto head = std::make_unique<Cube>(Vector3{0, 1.0f, 0}, Vector3{0.5f, 0.5f, 0.5f}, 1.0f,PINK);
    model_.push_back(std::move(head));
    selected_item_ = nullptr;
};

Player::Player(std::string data) {
    speed_ = 4.0f;
    std::vector<std::string> split = split_string(data);
    assert(split[0] == "Player" && split.size() == 8);
    username_ = split[1];
    hitbox_ = Cube(Vector3{0.0f,0.0f,0.0f}, Vector3{1.0f, 2.0f, 1.0f}, 1.0f, WHITE);
    set_position(std::stof(split[2]), std::stof(split[3]), std::stof(split[4]));
    online_ = std::stoi(split[5]) == 1;
    if (split[6] != "null_item") {
        if (get_first_word(split[6]) == "MoveTool") {
            selected_item_ = std::make_shared<MoveTool>(split[6]);
        }
    }
    std::vector<std::string> model_objects = split_string(split[7]);
    for (std::string object_data : model_objects) {
        if (get_first_word(object_data) == "Cube") {
            add_to_model(std::make_unique<Cube>(object_data));
        }
    }
};

void Player::draw(std::string current_user, int camera_mode) const {
    if ((camera_mode == CAMERA_CUSTOM && username_ == current_user) || !online_)
        return;
    for (const auto& object : model_)
        object->draw_offset(get_position().x, get_position().y, get_position().z);
    hitbox_.draw_outline();
}

bool Player::move(MainCamera& camera, const std::vector<bool>& keybinds, float dt) {
    assert(keybinds.size() >= 4);
    if (camera.get_mode() == CAMERA_FREE || !online_ || (!keybinds[0] && !keybinds[1] && !keybinds[2] && !keybinds[3])) {
        return false;
    }
    Vector3 direction = camera.get_direction();
    Vector3 left = Vector3{direction.z, 0.0f, -direction.x};
    float dx = (keybinds[0]*direction.x-keybinds[2]*direction.x-keybinds[3]*left.x+keybinds[1]*left.x);
    float dz = (keybinds[0]*direction.z-keybinds[2]*direction.z-keybinds[3]*left.z+keybinds[1]*left.z);
    float magnitude = sqrt(pow(dx,2.0) + pow(dz,2.0));
    if (magnitude == 0) return false;
    dx = dx/magnitude*dt*speed_;
    dz = dz/magnitude*dt*speed_;

    hitbox_.set_x(hitbox_.get_x()+dx);
    hitbox_.set_z(hitbox_.get_z()+dz);
    return true;
}

bool Player::pickup(std::shared_ptr<World> world, const std::vector<bool>& keybinds) {
    return false;
}

void Player::set_position(float x, float y, float z) {
    hitbox_.set_x(x);
    hitbox_.set_y(y+0.5f);
    hitbox_.set_z(z);
}

void Player::add_to_model(std::unique_ptr<Object3d>&& object) {
    model_.push_back(std::move(object));
}

void Player::update(std::map<std::string, std::shared_ptr<Event>>& event_buffer, MainCamera& camera, std::shared_ptr<World> world, const std::vector<bool>& keybinds, float dt) {
    bool moved = move(camera, keybinds, dt);
    bool picked_up = pickup(world, keybinds);
    use_item(event_buffer, camera, world, keybinds, dt);
    if (moved) {
        event_buffer["PlayerMoveEvent"] = std::make_unique<PlayerMoveEvent>(shared_from_this());
    }
    if (picked_up) {

    }
}

void Player::pickup_item(std::shared_ptr<Item> item, std::shared_ptr<World> world) {
    if (item == selected_item_)
        return;
    drop_item(world);
    //world->remove_object(item);
    selected_item_ = item;
}
void Player::drop_item(std::shared_ptr<World> world) {
    if (selected_item_ == nullptr)
        return;
    std::shared_ptr<Item> item = selected_item_;
    selected_item_ = nullptr;
    item->set_x(get_position().x);
    item->set_y(get_position().y);
    item->set_z(get_position().z);
    world->load_object(item);
}
void Player::use_item(std::map<std::string, std::shared_ptr<Event>>& event_buffer, const MainCamera& camera, std::shared_ptr<World> world, const std::vector<bool>& keybinds, float dt) {
    if (selected_item_ == nullptr)
        return;
    selected_item_->use(event_buffer, camera, shared_from_this(), world, keybinds, dt);
}

void Player::on_join() {
    online_ = true;
}

void Player::on_disconnect() {
    online_ = false;
}

bool Player::is_online() const {
    return online_;
}

const Cube& Player::get_hitbox() {
    return hitbox_;
}

std::string Player::get_username() {
    return username_;
}

Vector3 Player::get_position() const {
    return Vector3{hitbox_.get_x(), hitbox_.get_y()-0.5f, hitbox_.get_z()};
}

std::string Player::to_string() const {
    std::string item_string = (selected_item_ == nullptr) ? "null_item" : selected_item_->to_string();
    std::string result = "Player " + 
        username_ + " " +
        std::to_string(get_position().x) + " " + std::to_string(get_position().y) + " " + std::to_string(get_position().z) + " "
        + std::to_string((int)online_) + " (" + item_string + ")(";
    for (const auto& object : model_) {
        result += "(" + object->to_string() + ")";
    }
    result += ")";
    return result;
}