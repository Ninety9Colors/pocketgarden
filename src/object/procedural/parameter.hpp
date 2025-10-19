#pragma once
#include <string>
#include <map>
#include <random>

class Parameter {
public:
    Parameter();
    Parameter(float min, float value, float max);

    void seed_gaussian(std::mt19937_64& rng);
    void seed_uniform(std::mt19937_64& rng);
    void seed_hsv_gaussian(float hue_min, float hue_max, std::mt19937_64& rng);
    void seed_hsv_uniform(float hue_min, float hue_max, std::mt19937_64& rng);

    float min;
    float value;
    float max;
};

class ParameterMap {
public:
    ParameterMap();
    ParameterMap(std::string data);
    void set_parameter(std::string name, float value);
    void set_parameter(std::string name, Parameter parameter);
    const Parameter get_parameter(std::string name) const;

    void seed_gaussian(std::string name, std::mt19937_64& rng);
    void seed_uniform(std::string name, std::mt19937_64& rng);
    void seed_hsv_gaussian(std::string name, float hue_min, float hue_max, std::mt19937_64& rng);
    void seed_hsv_uniform(std::string name, float hue_min, float hue_max, std::mt19937_64& rng);

    std::string to_string() const;
private:
    std::map<std::string, Parameter> parameters_;
};