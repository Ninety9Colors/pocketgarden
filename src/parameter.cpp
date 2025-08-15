#include <algorithm>
#include <vector>
#include <cassert>
#include <random>

#include "parameter.hpp"
#include "util.hpp"

Parameter::Parameter() : min(0), value(0), max(0) {}
Parameter::Parameter(float min, float value, float max) : min(min), value(value), max(max) {}

void Parameter::seed_gaussian(std::mt19937_64& rng) {
    std::normal_distribution dist((max+min)/2.0f,(max-min)/6.0f);
    value = std::clamp<float>(dist(rng),min,max);
}
void Parameter::seed_uniform(std::mt19937_64& rng) {
    std::uniform_real_distribution dist(min,max);
    value = dist(rng);
}
void Parameter::seed_hsv_gaussian(float hue_min, float hue_max, std::mt19937_64& rng) {
    std::normal_distribution dist_gaussian(0.5f,0.5f/3.0f);
    std::uniform_real_distribution dist_uniform(hue_min,hue_max);
    min = std::fmodf(dist_uniform(rng),360.0f);
    value = std::clamp<float>(dist_gaussian(rng),0.0f,1.0f);
    max = std::clamp<float>(dist_gaussian(rng),0.0f,1.0f);
}
void Parameter::seed_hsv_uniform(float hue_min, float hue_max, std::mt19937_64& rng) {
    std::normal_distribution dist_sv(0.5f,0.5f/3.0f);
    std::normal_distribution dist_hue((hue_max+hue_min)/2.0f,(hue_max-hue_min)/6.0f);
    min = std::fmodf(dist_hue(rng),360.0f);
    value = std::clamp<float>(dist_sv(rng),0.0f,1.0f);
    max = std::clamp<float>(dist_sv(rng),0.0f,1.0f);
}

ParameterMap::ParameterMap() : parameters_{} {}
ParameterMap::ParameterMap(std::string data) {
    std::vector<std::string> split = split_string(data);
    assert(split[0] == "ParameterMap");
    std::vector<std::string> parameter_split = split_string(split[1]);
    for (std::string s : parameter_split) {
        std::vector<std::string> a = split_string(s);
        Parameter p {std::stof(a[1]), std::stof(a[2]), std::stof(a[3])};
        parameters_[a[0]] = p;
    }
}

void ParameterMap::set_parameter(std::string name, float value) {
    parameters_[name].value = value;
}

void ParameterMap::set_parameter(std::string name, Parameter parameter) {
    parameters_[name] = parameter;
}

const Parameter ParameterMap::get_parameter(std::string name) const {
    try {
        return parameters_.at(name);
    } catch (...) {
        return Parameter{};
    }
}

void ParameterMap::seed_gaussian(std::string name, std::mt19937_64& rng) {
    parameters_.at(name).seed_gaussian(rng);
}
void ParameterMap::seed_uniform(std::string name, std::mt19937_64& rng) {
    parameters_.at(name).seed_uniform(rng);
}
void ParameterMap::seed_hsv_gaussian(std::string name, float hue_min, float hue_max, std::mt19937_64& rng) {
    parameters_.at(name).seed_hsv_gaussian(hue_min,hue_max,rng);
}
void ParameterMap::seed_hsv_uniform(std::string name, float hue_min, float hue_max, std::mt19937_64& rng) {
    parameters_.at(name).seed_hsv_uniform(hue_min,hue_max,rng);
}

std::string ParameterMap::to_string() const {
    std::string result = "ParameterMap (";
    for (const auto& p : parameters_) {
        result += "(" + p.first + " " +
        std::to_string(p.second.min) + " " + std::to_string(p.second.value) + " " + std::to_string(p.second.max) + ")";
    }
    result += ")";
    return result;
}