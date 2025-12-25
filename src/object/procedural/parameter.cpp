#include <algorithm>
#include <vector>
#include <cassert>
#include <random>

#include "json.hpp"
using json = nlohmann::json;

#include "object/procedural/parameter.hpp"
#include "logging.hpp"

Parameter::Parameter() : min(0), value(0), max(0) {}
Parameter::Parameter(const json& j) {from_json(j);}
Parameter::Parameter(float min, float value, float max) : min(min), value(value), max(max) {}

void Parameter::from_json(const json& j) {
    min = j.at("min");
    value = j.at("value");
    max = j.at("max");
}

json Parameter::to_json() const {
    json j = {
        {"min",min},
        {"value",value},
        {"max",max}
    };
    return j;
}

ParameterMap::ParameterMap() : parameters_{} {}
ParameterMap::ParameterMap(const json& j) {from_json(j);}

void ParameterMap::from_json(const json& j) {
    for (const auto& p : j.at("parameters").items())
        parameters_[p.key()] = Parameter{p.value()};
}

json ParameterMap::to_json() const {
    json j = {
        {"type","ParameterMap"},
        {"parameters",{}}
    };
    for (const auto& p : parameters_)
        j.at("parameters")[p.first] = p.second.to_json();
    return j;
}

bool ParameterMap::contains_parameter(std::string name) const {
    return parameters_.find(name) != parameters_.end();
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
        CRITICAL("Could not find parameter " + name + "! Returning default");
        return Parameter{};
    }
}