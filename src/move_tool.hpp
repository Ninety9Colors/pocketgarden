#pragma once
#include <memory>
#include <string>
#include <cstdint>
#include <vector>

#include "raylib.h"

#include "object3d.hpp"
#include "maincamera.hpp"
#include "player.hpp"
#include "world.hpp"
#include "event.hpp"

class MoveTool : public Item {
public:
    MoveTool();
    MoveTool(std::string data);
    MoveTool(Vector3 position, float scale);

    void use(std::map<std::string, std::shared_ptr<Event>>& event_buffer, const MainCamera& camera, std::shared_ptr<Player> user, std::shared_ptr<World> world, const std::vector<bool>& keybinds, float dt) override;

    void draw() const override;
    void draw_offset(float x, float y, float z) const override;
    void prepare_drop(std::map<std::string, std::shared_ptr<Event>>& event_buffer, const MainCamera& camera, std::shared_ptr<Player> user, std::shared_ptr<World> world, const std::vector<bool>& keybinds, float dt) override;
    void set_shader(std::shared_ptr<Shader> shader) override;

    BoundingBox get_bounding_box() const override;

    bool in_use() const;

    std::string to_string() const override;
private:
    uint32_t held_id_;
    float holding_distance_;

    std::vector<std::unique_ptr<Object3d>> model_;
    float speed_;
};