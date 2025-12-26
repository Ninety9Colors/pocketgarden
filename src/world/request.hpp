#pragma once

#include <optional>
#include <string>

#include "json.hpp"
using json = nlohmann::json;

std::optional<json> get_url(std::string url);