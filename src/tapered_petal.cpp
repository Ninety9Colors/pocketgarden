#include <cassert>
#include <cmath>

#include "tapered_petal.hpp"
#include "util.hpp"

TaperedPetal::TaperedPetal() : ParameterObject(), slices_{10,5} {initialize_parameters();}
TaperedPetal::TaperedPetal(float scale) : ParameterObject(scale), slices_{10,5} {initialize_parameters();}
TaperedPetal::TaperedPetal(Vector3 position, float scale) : ParameterObject(position, scale), slices_{10,5} {initialize_parameters();}
TaperedPetal::TaperedPetal(std::string data) {
    std::vector<std::string> split = split_string(data);
    assert(split[0] == "TaperedPetal");
    position_ = Vector3{std::stof(split[1]), std::stof(split[2]), std::stof(split[3])};
    scale_ = std::stof(split[4]);
    quaternion_ = Quaternion{std::stof(split[5]),std::stof(split[6]),std::stof(split[7]),std::stof(split[8])};
    parameter_map_ = ParameterMap(split[9]);
}

void TaperedPetal::draw() const {
    for (int i = 0; i <= slices_.first; i++) {
        for (int j = 0; j <= slices_.second; j++) {
            DrawSphere(Vector3{grid_[i][j][0],grid_[i][j][1],grid_[i][j][2]},0.005f, PURPLE);
        }
    }
}

void TaperedPetal::generate_mesh() {
    float grid[10][5][3] {0};
    float u_step = parameter_map_.get_parameter("Length").value/(1.0f*slices_.first);
    float v_step = 2.0f*parameter_map_.get_parameter("Width").value/(1.0f*slices_.second);
    for (int i = 0; i <= slices_.first; i++) {
        for (int j = 0; j <= slices_.second; j++) {
            float u = i*u_step;
            float v = j*v_step - parameter_map_.get_parameter("Width").value;
            grid[i][j][0] = X(u,v);
            grid[i][j][1] = Y(u,v);
            grid[i][j][2] = Z(u,v);
            grid_[i][j][0] = X(u,v);
            grid_[i][j][1] = Y(u,v);
            grid_[i][j][2] = Z(u,v);
        }
    }
}

std::string TaperedPetal::to_string() const {
    return "TaperedPetal (" +
    std::to_string(position_.x) + " " + std::to_string(position_.y) + " " + std::to_string(position_.z) + " " +
    std::to_string(scale_) + " " +
    std::to_string(quaternion_.x) + " " + std::to_string(quaternion_.y) + " " + std::to_string(quaternion_.z) + " " + std::to_string(quaternion_.w) + " " + 
    parameter_map_.to_string() + ")";
}

void TaperedPetal::set_slices(std::pair<int,int> slices) {
    slices_ = slices;
}

void TaperedPetal::initialize_parameters() {
    parameter_map_.set_parameter("Sharpness", Parameter{0.0f,0.5f,1.0f});
    parameter_map_.set_parameter("Length", Parameter{0.0f,0.5f,1.0f});
    parameter_map_.set_parameter("Height", Parameter{0.0f,0.25f,0.5f});
    parameter_map_.set_parameter("Curl", Parameter{1.5f,2.25f,3.0f});
    parameter_map_.set_parameter("Width", Parameter{0.0f,0.125f,0.25f});
    parameter_map_.set_parameter("Curvature", Parameter{0.0f,0.175f,0.35f});
}

float TaperedPetal::X(float u, float v) const {
    return u;
}

float TaperedPetal::Y(float u, float v) const {
    float length = parameter_map_.get_parameter("Length").value;
    float height = parameter_map_.get_parameter("Height").value;
    float width = parameter_map_.get_parameter("Width").value;
    float curvature = parameter_map_.get_parameter("Curvature").value;
    float curl = parameter_map_.get_parameter("Curl").value;
    float z = Z(u,v);
    float midpoint = length*curl/3.0f;
    float a = std::sqrt(height)/midpoint;
    float t1 = std::powf((a*(u-midpoint)),2);
    float t2 = std::powf(curvature*z/width,2);
    float t3 = std::powf(midpoint*a,2);
    return -t1 + t2 + t3;
}

float TaperedPetal::Z(float u, float v) const {
    float length = parameter_map_.get_parameter("Length").value;
    float sharpness = parameter_map_.get_parameter("Sharpness").value;
    float temp = std::abs(-4.0f*(u-length/2.0f)*(u-length/2.0f)/(length*length)+1);
    return v * std::powf(temp,1/(4.0f-3.0f*sharpness));
}