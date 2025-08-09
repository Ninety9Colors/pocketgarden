#pragma once
#include <string>
#include <map>
#include <memory>
#include <cstdint>
#include <vector>

#include "raylib.h"

class Player;
class World;
class MainCamera;
class Event;

class Object3d {
public:
    void set_id(uint32_t id) {id_ = id;}
    uint32_t get_id() const {return id_;}

    virtual void draw() const = 0;
    virtual void draw_offset(float x, float y, float z) const = 0;
    virtual void set_shader(std::shared_ptr<Shader> shader) = 0;
    virtual std::shared_ptr<Shader> get_shader();

    virtual void set_quaternion(Quaternion quaternion);
    virtual Quaternion get_quaternion();

    virtual void set_position(Vector3 position);
    virtual Vector3 get_position() const;

    virtual BoundingBox get_bounding_box() const = 0;

    virtual std::string to_string() const = 0;

    virtual ~Object3d() {};
protected:
    std::shared_ptr<Shader> shader_;
    Quaternion quaternion_;
    Vector3 position_;
private:
    uint32_t id_ = 0;
};

class Item : public Object3d {
public:
    virtual void use(std::map<std::string, std::shared_ptr<Event>>& event_buffer, const MainCamera& camera, std::shared_ptr<Player> user, std::shared_ptr<World> world, const std::vector<bool>& keybinds, float dt) = 0;
    virtual void prepare_drop(std::map<std::string, std::shared_ptr<Event>>& event_buffer, const MainCamera& camera, std::shared_ptr<Player> user, std::shared_ptr<World> world, const std::vector<bool>& keybinds, float dt) = 0;
    virtual ~Item() {};
};