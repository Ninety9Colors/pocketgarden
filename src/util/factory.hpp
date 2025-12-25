#pragma once
#include <memory>

#include "json.hpp"
using json = nlohmann::json;

class Event;
class Object3d;

std::unique_ptr<Object3d> parse_object(const json& j);
std::unique_ptr<Event> parse_event(const json& j);