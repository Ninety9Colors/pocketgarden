#pragma once
#include <string>
#include <map>
#include <memory>
#include <cstdint>
#include <vector>
#include <queue>

#include "raylib.h"
#include "json.hpp"
using json = nlohmann::json;

#include "object/procedural/parameter.hpp"
#include "object/procedural/lsystem.hpp"

class Player;
class World;
class MainCamera;
class Event;
class Game;
class Event;

enum class ObjectType : uint8_t {
    NONE = 0,
    DEFAULT = 1,
    ITEM = 1 << 1
};

class Object3d {
public:
    Object3d();
    Object3d(Quaternion quaternion, Vector3 position, float scale);
    Object3d(const Object3d& rhs) ;
    virtual ~Object3d();

    void set_id(uint32_t id) {id_ = id;}
    uint32_t get_id() const {return id_;}

    virtual void generate_mesh();

    void draw(Game& game) const;
    void draw(Game& game, Matrix transform) const;
    void draw_offset(Game& game, float x, float y, float z) const;
    virtual void draw(Game& game, Material material_) const;
    virtual void draw(Game& game, Matrix transform, Material material_) const;
    virtual void draw_instanced(Game& game, const Matrix* transforms, int matrix_count) const;
    virtual void draw_instanced(Game& game, Material material, const Matrix* transforms, int matrix_count) const;

    virtual void set_quaternion(Quaternion quaternion);
    virtual Quaternion get_quaternion() const;
    virtual void rotate_axis(Vector3 axis, float radians);

    virtual const Matrix& get_matrix() const;

    virtual void set_position(Vector3 position);
    virtual Vector3 get_position() const;

    virtual void set_scale(float scale);
    virtual float get_scale() const;

    virtual BoundingBox get_bounding_box() const;
    virtual BoundingBox get_bounding_box(Matrix transform) const;

    virtual void set_material(Material material);

    uint8_t get_type() const;

    friend void swap(Object3d& a, Object3d& b) noexcept;

    virtual json to_json() const = 0;
    virtual void from_json(const json& j) = 0;

    // Item Methods
    virtual void use(Game& game, const std::string username, const std::vector<bool>& keybinds, float dt);
    virtual void on_drop(Game& game, std::string username, const std::vector<bool>& keybinds, float dt);
protected:
    virtual void update_matrix(); // called automatically

    uint8_t object_type_;

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
    Item(Quaternion quaternion, Vector3 position, float scale);
    virtual ~Item() {};
    virtual void use(Game& game, const std::string username, const std::vector<bool>& keybinds, float dt) = 0;
    virtual void on_drop(Game& game, std::string username, const std::vector<bool>& keybinds, float dt) = 0;
};

class ParameterObject : public Object3d {
public:
    ParameterObject();
    ParameterObject(uint32_t seed);
    ParameterObject(Quaternion quaternion, Vector3 position, float scale);
    ParameterObject(Quaternion quaternion, Vector3 position, float scale, uint32_t seed);
    virtual ~ParameterObject() {};

    // This function should do nothing if parameter map is empty
    // Should be called explicitly after the object is created - lazy loading mesh?
    virtual void generate_mesh() = 0;
    
    void set_parameter_map(ParameterMap map);
    void set_parameter(std::string name, float value);
    void set_parameter(std::string name, Parameter parameter);
    const Parameter get_parameter(std::string name) const;
protected:
    virtual void initialize_parameters() = 0;
    ParameterMap parameter_map_;
    uint32_t seed_;
};