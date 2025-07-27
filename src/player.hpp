#pragma once
#include <memory>
#include <map>
#include <string>
#include <vector>

#include "raylib.h"

#include "cube.hpp"
#include "event.hpp"
#include "object3d.hpp"

class World;
class MainCamera;

class Player : public std::enable_shared_from_this<Player> {
public:
    Player(std::string username, Vector3 position);
    Player(std::string data);

    void draw(std::string current_user, const MainCamera& camera) const;
    bool move(MainCamera& camera, const std::vector<bool>& keybinds, float dt);
    void set_position(float x, float y, float z);
    void add_to_model(std::unique_ptr<Object3d>&& object);

    void update(std::map<std::string, std::shared_ptr<Event>>& event_buffer, MainCamera& camera, std::shared_ptr<World> world, const std::vector<bool>& keybinds, float dt);

    uint32_t try_pickup(MainCamera& camera, std::shared_ptr<World> world, const std::vector<bool>& keybinds) const;
    void set_item(std::shared_ptr<Item> item);
    std::shared_ptr<Item> drop_item(std::shared_ptr<World> world);
    void use_item(std::map<std::string, std::shared_ptr<Event>>& event_buffer, const MainCamera& camera, std::shared_ptr<World> world, const std::vector<bool>& keybinds, float dt);

    void on_join();
    void on_disconnect();
    bool is_online() const;

    std::string get_username();
    const Cube& get_hitbox();
    Vector3 get_position() const;

    std::string to_string() const;
    
private:
    std::string username_;
    float speed_;
    float pickup_range_;
    bool online_;

    Cube hitbox_;
    std::vector<std::unique_ptr<Object3d>> model_;
    std::shared_ptr<Item> selected_item_;
};