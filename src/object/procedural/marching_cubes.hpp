#pragma once

#include <raylib.h>
#include <vector>

std::pair<std::pair<std::vector<std::vector<std::vector<double>>>,std::vector<std::vector<std::vector<Color>>>>,std::array<double,3>> voxelize_pcd(std::vector<std::array<double,3>> coordinates, std::vector<std::array<unsigned char,3>> colors, double voxel_size);
std::vector<Mesh> march_cubes(std::vector<std::vector<std::vector<double>>> grid, std::vector<std::vector<std::vector<Color>>> color_grid, std::array<double,3> bottom_left_voxel, double voxel_size, double iso_value);