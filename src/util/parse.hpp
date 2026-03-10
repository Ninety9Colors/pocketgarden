#pragma once

#include <vector>
#include <array>
#include <string>

std::pair<std::vector<std::array<double,3>>,std::vector<std::array<unsigned char,3>>> parse_xyz(std::string filename);