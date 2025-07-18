#include "player.hpp"

Player::Player(std::string username, Vector3 position) : username_(username) {
    online_ = false;
    hitbox_ = Cube(position, Vector3{1.0f, 2.0f, 1.0f}, Color{0,0,0,0});
    auto head = std::make_unique<Cube>(Vector3{position.x, position.y+1.0f, position.z}, Vector3{0.5f, 0.5f, 0.5f}, PINK);
    model_.push_back(std::move(head));
};

void Player::draw(std::string current_user, int camera_mode) const {
    if ((camera_mode == CAMERA_CUSTOM && username_ == current_user) || !online_)
        return;
    for (const auto& object : model_)
        object->draw();
}

void Player::on_join() {
    online_ = true;
}

void Player::on_disconnect() {
    online_ = false;
}

const Cube& Player::get_hitbox() {
    return hitbox_;
}

std::string Player::get_username() {
    return username_;
}

