#pragma once
#include <string>
#include <vector>

class Object3d {
public:
    virtual void draw() const = 0;
    virtual void draw_outline() const = 0;
    virtual void draw_offset(float x, float y, float z) const = 0;
    virtual void draw_outline_offset(float x, float y, float z) const = 0;

    virtual void set_x(float new_x) = 0;
    virtual void set_y(float new_y) = 0;
    virtual void set_z(float new_z) = 0;
    virtual float get_x() const = 0;
    virtual float get_y() const = 0;
    virtual float get_z() const = 0;

    virtual std::string to_string() const = 0;

    virtual ~Object3d() {};
};

class Item : public Object3d {
public:
    virtual void use(const std::vector<bool>& keybinds) = 0;
    virtual ~Item() {};
};