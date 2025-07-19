#include <cassert>
#include <cmath>


#include "player.hpp"

Player::Player(std::string username, Vector3 position) : username_(username) {
    speed_ = 4.0f;
    online_ = false;
    hitbox_ = Cube(Vector3{position.x, position.y + 0.5f, position.z}, Vector3{1.0f, 2.0f, 1.0f}, Color{0,0,0,0});
    auto head = std::make_unique<Cube>(Vector3{position.x, position.y+1.0f, position.z}, Vector3{0.5f, 0.5f, 0.5f}, PINK);
    model_.push_back(std::move(head));
};

void Player::draw(std::string current_user, int camera_mode) const {
    if ((camera_mode == CAMERA_CUSTOM && username_ == current_user) || !online_)
        return;
    for (const auto& object : model_)
        object->draw();
    hitbox_.draw_outline();
}

void Player::move(float dt, const Vector3& direction, int camera_mode, const std::vector<bool>& keys_down) {
    assert(keys_down.size() == 4);
    if (camera_mode == CAMERA_FREE)
        return;
    Vector3 left = Vector3{direction.z, 0.0f, -direction.x};
    float dx = (keys_down[0]*direction.x-keys_down[2]*direction.x-keys_down[3]*left.x+keys_down[1]*left.x);
    float dz = (keys_down[0]*direction.z-keys_down[2]*direction.z-keys_down[3]*left.z+keys_down[1]*left.z);
    float magnitude = sqrt(pow(dx,2.0) + pow(dz,2.0));
    if (magnitude == 0) return;
    dx = dx/magnitude*dt*speed_;
    dz = dz/magnitude*dt*speed_;

    hitbox_.set_x(hitbox_.get_x()+dx);
    hitbox_.set_z(hitbox_.get_z()+dz);
    for (const auto& object : model_) {
        object->set_x(object->get_x()+dx);
        object->set_z(object->get_z()+dz);
    }
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

