#pragma once

#include <vector>
#include <array>
#include <string>

std::vector<std::pair<std::array<float,3>,std::array<unsigned char,3>>> parse_xyz(std::string filename);