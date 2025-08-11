#include "object3d.hpp"

#include "raylib.h"

class TaperedPetal : public ParameterObject {
public:
    TaperedPetal();
    TaperedPetal(float scale);
    TaperedPetal(Vector3 position, float scale);
    TaperedPetal(std::string data);

    void draw() const override;
    void generate_mesh() override;
    std::string to_string() const override;

    void set_slices(std::pair<int,int> slices);
private:
    float X(float u, float v) const;
    float Y(float u, float v) const;
    float Z(float u, float v) const;
    void initialize_parameters() override;
    std::pair<int,int> slices_;
    float grid_[10][5][3];
};