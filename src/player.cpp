#include <cassert>
#include <cmath>
#include <iostream>

#include "player.hpp"
#include "util.hpp"

Player::Player(std::string username, Vector3 position) : username_(username) {
    speed_ = 4.0f;
    online_ = false;
    hitbox_ = Cube(Vector3{0.0f,0.0f,0.0f}, Vector3{1.0f, 2.0f, 1.0f}, 1.0f, WHITE);
    set_position(position.x, position.y, position.z);
    auto head = std::make_unique<Cube>(Vector3{0, 1.0f, 0}, Vector3{0.5f, 0.5f, 0.5f}, 1.0f,PINK);
    model_.push_back(std::move(head));
};

Player::Player(std::string data) {
    speed_ = 4.0f;
    std::vector<std::string> split = split_string(data);
    assert(split[0] == "Player" && split.size() == 7);
    username_ = split[1];
    hitbox_ = Cube(Vector3{0.0f,0.0f,0.0f}, Vector3{1.0f, 2.0f, 1.0f}, 1.0f, WHITE);
    set_position(std::stof(split[2]), std::stof(split[3]), std::stof(split[4]));
    online_ = std::stoi(split[5]) == 1;
    std::vector<std::string> model_objects = split_string(split[6]);
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

bool Player::move(float dt, const Vector3& direction, int camera_mode, const std::vector<bool>& keybinds) {
    assert(keybinds.size() >= 4);
    if (camera_mode == CAMERA_FREE || !online_ || (!keybinds[0] && !keybinds[1] && !keybinds[2] && !keybinds[3])) {
        return false;
    }
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

void Player::set_position(float x, float y, float z) {
    hitbox_.set_x(x);
    hitbox_.set_y(y+0.5f);
    hitbox_.set_z(z);
}

void Player::add_to_model(std::unique_ptr<Object3d>&& object) {
    model_.push_back(std::move(object));
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
    std::string result = "Player " + 
        username_ + " " +
        std::to_string(get_position().x) + " " + std::to_string(get_position().y) + " " + std::to_string(get_position().z) + " "
        + std::to_string((int)online_) + " (";
    for (const auto& object : model_) {
        result += "(" + object->to_string() + ")";
    }
    result += ")";
    return result;
}