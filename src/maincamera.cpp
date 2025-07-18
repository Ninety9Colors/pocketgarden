#include <cmath>

#include <iostream>

#include "maincamera.hpp"
#include <algorithm>

MainCamera::MainCamera() {
    float mag = sqrt(1+0.3*0.3);
    direction_ = {1.0f/mag, 0.0f, 0.3f/mag};

    camera_ = Camera3D{};
    camera_.fovy = 60.0f;
    camera_.position = Vector3{1.0f, 2.0f, 3.0f};
    camera_.target = Vector3{0.0f, 0.0f, 0.0f};
    camera_.up = Vector3{0.0f, 1.0f, 0.0f};
    camera_.projection = CAMERA_PERSPECTIVE;

    camera_mode_ = CAMERA_CUSTOM;

    sensitivity_ = 0.001f;
}

void MainCamera::update(const std::shared_ptr<Player> player, Vector2 mouse_delta) {
    if (camera_mode_ != CAMERA_CUSTOM) {
        UpdateCamera(&camera_,camera_mode_);
        return;
    }
    camera_.position = Vector3{player->get_hitbox().position.x, player->get_hitbox().position.y + 1.0f, player->get_hitbox().position.z};
    camera_.target = Vector3{camera_.position.x + direction_.x, camera_.position.y + direction_.y, camera_.position.z + direction_.z};

    float dpitch = mouse_delta.y*sensitivity_; // 0 when straight on y axis, positive downward
    float dyaw = mouse_delta.x*sensitivity_; // 0 when straight on x axis, positive leftward
    
    float prev_pitch = acosf(direction_.y);
    float new_pitch = prev_pitch + dpitch;
    if (new_pitch > 3.1)
        new_pitch = 3.1;
    if (new_pitch < 0.1)
        new_pitch = 0.1;

    float sideways_vector_magnitude = sqrt(pow(direction_.x,2.0f) + pow(direction_.z, 2));
    Vector3 sideways_vector = Vector3{direction_.x/sideways_vector_magnitude, 0.0f, direction_.z/sideways_vector_magnitude};

    float new_x = sin(new_pitch)*sideways_vector.x;
    float new_y = cos(new_pitch);
    float new_z = sin(new_pitch)*sideways_vector.z;
    float new_magnitude = sqrt(pow(new_x,2) + pow(new_y,2) + pow(new_z,2));

    direction_.x = new_x/new_magnitude;
    direction_.y = new_y/new_magnitude;
    direction_.z = new_z/new_magnitude;

    new_x = direction_.x * cosf(dyaw) - direction_.z * sinf(dyaw);
    new_z = direction_.x * sinf(dyaw) + direction_.z * cosf(dyaw);
    float new_sideways_magnitude = sqrt(pow(new_x,2) + pow(new_z,2));

    direction_.x = new_x/new_sideways_magnitude;
    direction_.z = new_z/new_sideways_magnitude;
}

void MainCamera::toggle_freecam() {
    if (camera_mode_ == CAMERA_CUSTOM) {
        camera_mode_ = CAMERA_FREE;
    } else {
        camera_mode_ = CAMERA_CUSTOM;
    }
}

const Camera3D& MainCamera::get_camera() const {
    return camera_;
}

int MainCamera::get_mode() const {
    return camera_mode_;
}