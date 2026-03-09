#include "marching_cubes.hpp"

#include <array>
#include <limits>
#include "raylib.h"

std::vector<std::vector<std::vector<float>>> voxelize_pcd(std::vector<std::array<float,3>> coordinates, std::vector<std::array<unsigned char,3>> colors, float voxel_size) {
    Vector3 bottom_left {std::numeric_limits<float>::infinity(),std::numeric_limits<float>::infinity(),std::numeric_limits<float>::infinity()};
    Vector3 top_right {-std::numeric_limits<float>::infinity(),-std::numeric_limits<float>::infinity(),-std::numeric_limits<float>::infinity()};
    // defining the bounding box ^
    for (const std::array<float,3>& a : coordinates) {
        bottom_left.x = std::min(bottom_left.x,a[0]);
        bottom_left.y = std::min(bottom_left.y,a[1]);
        bottom_left.z = std::min(bottom_left.z,a[2]);

        top_right.x = std::max(top_right.x,a[0]);
        top_right.y = std::max(top_right.y,a[1]);
        top_right.z = std::max(top_right.z,a[2]);
    }
    int n_x = (top_right.x-bottom_left.x)/voxel_size + 1.0f;
    int n_y = (top_right.y-bottom_left.y)/voxel_size + 1.0f;
    int n_z = (top_right.z-bottom_left.z)/voxel_size + 1.0f;
    std::vector<std::vector<std::vector<float>>>
}