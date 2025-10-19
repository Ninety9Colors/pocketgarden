#pragma once
#include <string>
#include <map>
#include <memory>
#include <cstdint>
#include <vector>

#include "raylib.h"

#include "object/procedural/parameter.hpp"

class Player;
class World;
class MainCamera;
class Event;

class Object3d {
public:
    Object3d();
    Object3d(float scale);
    Object3d(Vector3 position, float scale);
    Object3d(Quaternion quaternion, Vector3 position, float scale);
    virtual ~Object3d();

    void set_id(uint32_t id) {id_ = id;}
    uint32_t get_id() const {return id_;}

    virtual void draw() const;
    virtual void draw(Matrix transform) const;
    virtual void draw_offset(float x, float y, float z) const;
    virtual void set_shader(std::shared_ptr<Shader> shader);
    virtual std::shared_ptr<Shader> get_shader();

    virtual void set_quaternion(Quaternion quaternion);
    virtual Quaternion get_quaternion();
    virtual void rotate_axis(Vector3 axis, float radians);

    virtual void update_matrix();
    virtual const Matrix& get_matrix() const;

    virtual void set_position(Vector3 position);
    virtual Vector3 get_position() const;

    virtual void set_scale(float scale);
    virtual float get_scale() const;

    virtual BoundingBox get_bounding_box() const;
    virtual BoundingBox get_bounding_box(Matrix transform) const;

    virtual std::string to_string() const = 0;
protected:
    std::shared_ptr<Shader> shader_;
    Quaternion quaternion_;
    Vector3 position_;
    Mesh mesh_;
    Material material_;
    float scale_;
    Matrix transform_;
private:
    uint32_t id_ = 0;
};

class Item : public Object3d {
public:
    Item();
    Item(float scale);
    Item(Vector3 position, float scale);
    Item(Quaternion quaternion, Vector3 position, float scale);
    virtual void use(std::map<std::string, std::shared_ptr<Event>>& event_buffer, const MainCamera& camera, std::shared_ptr<Player> user, std::shared_ptr<World> world, const std::vector<bool>& keybinds, float dt) = 0;
    virtual void prepare_drop(std::map<std::string, std::shared_ptr<Event>>& event_buffer, const MainCamera& camera, std::shared_ptr<Player> user, std::shared_ptr<World> world, const std::vector<bool>& keybinds, float dt) = 0;
    virtual ~Item() {};
};

class ParameterObject : public Object3d {
public:
    ParameterObject();
    ParameterObject(float scale);
    ParameterObject(Vector3 position, float scale);
    ParameterObject(Quaternion quaternion, Vector3 position, float scale);

    virtual void generate_mesh() = 0;
    
    void set_parameters(ParameterMap map);
    void set_parameter(std::string name, float value);
    const Parameter get_parameter(std::string name) const;

    virtual ~ParameterObject() {};
protected:
    virtual void initialize_parameters() = 0;

    ParameterMap parameter_map_; 
};