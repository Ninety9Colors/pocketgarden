#pragma once
#include <memory>
#include <map>
#include <cstdint>
#include <string>
#include <vector>
#include <deque>

#include "raylib.h"

#include "object/consistent/cube.hpp"
#include "event/event.hpp"
#include "object/object3d.hpp"

class World;

class Player {
public:
    Player();
    Player(std::string username, Vector3 position);
    Player(const json& j);
    Player(const Player& rhs);
    Player& operator=(Player rhs) {swap(*this,rhs);return *this;}
    ~Player() {};

    void draw(Game& game) const;
    void move(Vector3 direction, const std::vector<bool>& keybinds, float dt);
    void set_position(Vector3 position);
    
    void set_item(std::unique_ptr<Object3d> item);
    std::unique_ptr<Object3d> drop_item(Game& game, std::string username, const std::vector<bool>& keybinds, float dt);
    void use_item(Game& game, const std::vector<bool>& keybinds, float dt);

    void set_online(bool online);
    bool is_online() const;

    std::string get_username() const;
    const Cube& get_hitbox();
    Vector3 get_position() const;
    float get_pickup_range() const;

    json to_json() const;
    void from_json(const json& j);

    friend void swap(Player& a, Player& b);
private:
    std::string username_;
    float speed_;
    float pickup_range_;
    bool online_;
    
    // Update these everytime position is updated:
    Vector3 position_; // Center of the bottom face of the hitbox
    Cube hitbox_;
    Cube head_;
    std::unique_ptr<Object3d> selected_item_;
};