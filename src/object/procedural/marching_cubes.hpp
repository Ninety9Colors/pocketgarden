#pragma once

#include <vector>

std::vector<std::vector<std::vector<bool>>> voxelize_pcd_binary(std::vector<std::array<float,3>> coordinates, std::vector<std::array<unsigned char,3>> colors, float voxel_size);