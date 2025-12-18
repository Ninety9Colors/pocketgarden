#pragma once
#include <memory>
#include <map>
#include <cstdint>
#include <string>
#include <vector>

#include "raylib.h"

#include "object/consistent/cube.hpp"
#include "event/event.hpp"
#include "object/object3d.hpp"

class World;

class Player {
public:
    Player();
    Player(std::string username, Vector3 position);
    Player(std::string data);
    ~Player();

    void draw(std::string current_user) const;
    bool move(Vector3 direction, const std::vector<bool>& keybinds, float dt);
    void set_position(Vector3 position);
    void add_to_model(std::unique_ptr<Object3d>&& object);
    void set_shader(std::shared_ptr<Shader> shader);
    std::shared_ptr<Shader> get_shader() const;

    void update(std::map<std::string, std::shared_ptr<Event>>& event_buffer, MainCamera& camera, std::shared_ptr<World> world, const std::vector<bool>& keybinds, float dt);

    uint32_t try_pickup(Vector3 direction, std::shared_ptr<World> world, const std::vector<bool>& keybinds) const;
    void set_item(std::shared_ptr<Item> item);
    std::shared_ptr<Item> drop_item(std::map<std::string, std::shared_ptr<Event>>& event_buffer, const MainCamera& camera, std::shared_ptr<World> world, const std::vector<bool>& keybinds, float dt);
    void use_item(std::map<std::string, std::shared_ptr<Event>>& event_buffer, const MainCamera& camera, std::shared_ptr<World> world, const std::vector<bool>& keybinds, float dt);

    void on_join();
    void on_disconnect();
    bool is_online() const;

    std::string get_username() const;
    const Cube& get_hitbox();
    Vector3 get_position() const;

    std::string to_string() const;
private:
    std::string username_;
    float speed_;
    float pickup_range_;
    bool online_;
    
    // Update these everytime position is updated:
    Vector3 position_; // Center of the bottom face of the hitbox
    Cube hitbox_;
    Cube head_;
    std::unique_ptr<Item> selected_item_;
};