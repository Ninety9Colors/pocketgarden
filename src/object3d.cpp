#include "object3d.hpp"

std::shared_ptr<Shader> Object3d::get_shader() {
    return shader_;
}

void Object3d::set_quaternion(Quaternion quaternion) {
    quaternion_ = quaternion;
}

Quaternion Object3d::get_quaternion() {
    return quaternion_;
}

void Object3d::set_position(Vector3 position) {
    position_ = position;
}

Vector3 Object3d::get_position() const {
    return position_;
}