#pragma once

#include <map>
#include <string>
#include <any>

class Settings {
public:
    static const std::any get(std::string setting_name) {return settings_[setting_name];};
    static std::any set(std::string setting_name, std::any value) {settings_[setting_name] = value;};
private:
    static std::map<std::string, std::any> settings_;
};