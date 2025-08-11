#include <vector>
#include <cassert>

#include "parameter.hpp"
#include "util.hpp"

ParameterMap::ParameterMap() : parameters_{} {}
ParameterMap::ParameterMap(std::string data) {
    std::vector<std::string> split = split_string(data);
    assert(split[0] == "ParameterMap");
    std::vector<std::string> parameter_split = split_string(split[1]);
    for (std::string s : parameter_split) {
        std::vector<std::string> a = split_string(s);
        Parameter p {std::stof(a[1]), std::stof(a[2]), std::stof(a[3])};
        parameters_[a[1]] = p;
    }
}

void ParameterMap::set_parameter(std::string name, float value) {
    parameters_[name].value = value;
}

void ParameterMap::set_parameter(std::string name, Parameter parameter) {
    parameters_[name] = parameter;
}

const Parameter ParameterMap::get_parameter(std::string name) const {
    return parameters_.at(name);
}

std::string ParameterMap::to_string() const {
    std::string result = "ParameterMap ( ";
    for (const auto& p : parameters_) {
        result += "(" + p.first + " " +
        std::to_string(p.second.min) + " " + std::to_string(p.second.value) + " " + std::to_string(p.second.max) + ")";
    }
    result += ")";
    return result;
}