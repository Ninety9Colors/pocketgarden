#pragma once
#include <string>
#include <map>

struct Parameter {
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

    std::string to_string() const;
private:
    std::map<std::string, Parameter> parameters_;
};