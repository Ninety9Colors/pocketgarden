#include <cassert>
#include <cstdint>
#include <cmath>
#include <iostream>

#include "object/consistent/move_tool.hpp"
#include "object/consistent/sun_tool.hpp"
#include "object/consistent/rotate_tool.hpp"
#include "object/procedural/tapered_petal.hpp"
#include "player/player.hpp"
#include "world/world.hpp"
#include "object/procedural/lily_flower.hpp"
#include "util/factory.hpp"

#include "raymath.h"
#include "logging.hpp"

constexpr float DEFAULT_PLAYER_SPEED = 4.0f;
constexpr float DEFAULT_PLAYER_PICKUP_RANGE= 3.0f;
const Vector3 HITBOX_OFFSET = {0.0f,1.0f,0.0f};
const Vector3 HEAD_OFFSET = {0.0f,1.5f,0.0f};
const Vector3 ITEM_OFFSET = {0.0f,2.0f,0.0f};

Player::Player() : position_(0.0f,0.0f,0.0f), selected_item_(nullptr), online_(false), pickup_range_(DEFAULT_PLAYER_PICKUP_RANGE), speed_(DEFAULT_PLAYER_SPEED), username_("No Username"), hitbox_({0.0f,0.0f,0.0f,1.0f},HITBOX_OFFSET, {1.0f, 2.0f, 1.0f}, 1.0f, WHITE), head_({0.0f,0.0f,0.0f,1.0f},HEAD_OFFSET, Vector3{0.5f, 0.5f, 0.5f}, 1.0f,PINK) {};
Player::Player(std::string username, Vector3 position) : position_(position), selected_item_(nullptr), online_(false), pickup_range_(DEFAULT_PLAYER_PICKUP_RANGE), speed_(DEFAULT_PLAYER_SPEED), username_(username), hitbox_({0.0f,0.0f,0.0f,1.0f},Vector3Add(position_,HITBOX_OFFSET), {1.0f, 2.0f, 1.0f}, 1.0f, WHITE), head_({0.0f,0.0f,0.0f,1.0f},Vector3Add(position_,HEAD_OFFSET), Vector3{0.5f, 0.5f, 0.5f}, 1.0f,PINK) {
    DEBUG("Constructed player with username: " + username);
};
Player::Player(const json& j) : hitbox_({0.0f,0.0f,0.0f,1.0f},HITBOX_OFFSET, {1.0f, 2.0f, 1.0f}, 1.0f, WHITE), head_({0.0f,0.0f,0.0f,1.0f},HEAD_OFFSET, Vector3{0.5f, 0.5f, 0.5f}, 1.0f,PINK) {from_json(j);}
Player::Player(const Player& rhs) : position_(rhs.position_), selected_item_((rhs.selected_item_ == nullptr) ? nullptr : std::move(parse_object(rhs.selected_item_->to_json()))), online_(rhs.online_), pickup_range_(rhs.pickup_range_), speed_(rhs.speed_), username_(rhs.username_), hitbox_(rhs.hitbox_), head_(rhs.head_) {}

void Player::from_json(const json& j) {
    username_ = j.at("username");
    position_ = Vector3{j.at("position")["x"],j.at("position")["y"],j.at("position")["z"]};
    online_ = j.at("online");
    selected_item_ = parse_object(j.at("item"));
    speed_ = j.at("speed");
    pickup_range_ = j.at("pickup_range");
    set_position(position_);
}

json Player::to_json() const {
    json j = {
        {"type","Player"},
        {"username",username_},
        {"position",{{"x",position_.x},{"y",position_.y},{"z",position_.z}}},
        {"online",online_},
        {"item",(selected_item_==nullptr ? json{{"type","null_item"}} : selected_item_->to_json())},
        {"speed",speed_},
        {"pickup_range",pickup_range_}
    };
    return j;
}

void Player::draw(Game& game) const {
    std::string current_user = game.get_current_username();
    if (username_ != current_user && online_)
        head_.draw(game);
    if (selected_item_ != nullptr && online_)
        selected_item_->draw(game);
}

void Player::move(Vector3 direction, const std::vector<bool>& keybinds, float dt) {
    Vector3 left = Vector3{direction.z, 0.0f, -direction.x};
    float dx = (keybinds[0]*direction.x-keybinds[2]*direction.x-keybinds[3]*left.x+keybinds[1]*left.x);
    float dz = (keybinds[0]*direction.z-keybinds[2]*direction.z-keybinds[3]*left.z+keybinds[1]*left.z);
    float magnitude = sqrt(pow(dx,2.0) + pow(dz,2.0));
    if (magnitude == 0) return;
    dx = dx/magnitude*dt*speed_;
    dz = dz/magnitude*dt*speed_;

    set_position(Vector3Add(position_,{dx,0.0f,dz}));
}

void Player::set_position(Vector3 position) {
    position_ = position;
    hitbox_.set_position(Vector3Add(position_,HITBOX_OFFSET));
    head_.set_position(Vector3Add(position_,HEAD_OFFSET));
    if (selected_item_ != nullptr)
        selected_item_->set_position(Vector3Add(position_,ITEM_OFFSET));
}

void Player::set_item(std::unique_ptr<Object3d> item) {
    if (item == nullptr || item.get() == selected_item_.get())
        return;
    INFO("Player " + username_ + " set selected item to type: " + std::string(item->to_json()["type"]));
    selected_item_ = std::move(item);
    selected_item_->set_position(Vector3Add(position_,ITEM_OFFSET));
}

std::unique_ptr<Object3d> Player::drop_item(Game& game, std::string username, const std::vector<bool>& keybinds, float dt) {
    if (selected_item_ == nullptr)
        return nullptr;
    std::unique_ptr<Object3d> item = std::move(selected_item_);
    selected_item_ = nullptr;
    item->on_drop(game,username_,keybinds,dt);
    item->set_position(get_position());
    return std::move(item);
}
void Player::use_item(Game& game, const std::vector<bool>& keybinds, float dt) {
    if (selected_item_ == nullptr)
        return;
    selected_item_->use(game,username_, keybinds, dt);
}

void Player::set_online(bool online) {
    online_ = online;
}
bool Player::is_online() const {
    return online_;
}

const Cube& Player::get_hitbox() {
    return hitbox_;
}

std::string Player::get_username() const {
    return username_;
}

Vector3 Player::get_position() const {
    return position_;
}

float Player::get_pickup_range() const {
    return pickup_range_;
}

void swap(Player& a, Player& b) {
    using std::swap;
    swap(a.username_,b.username_);
    swap(a.speed_,b.speed_);
    swap(a.pickup_range_,b.pickup_range_);
    swap(a.online_,b.online_);
    swap(a.position_,b.position_);
    swap(a.hitbox_,b.hitbox_);
    swap(a.head_,b.head_);
    swap(a.selected_item_,b.selected_item_);
}