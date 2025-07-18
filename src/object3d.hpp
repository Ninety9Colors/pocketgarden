#pragma once

class Object3d {
public:
    virtual void draw() const = 0;
    virtual ~Object3d() {};
};