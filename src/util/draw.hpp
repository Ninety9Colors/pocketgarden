#pragma once

#include <vector>
#include "raylib.h"

void draw_mesh_skeleton(Mesh mesh, Matrix transform);
void draw_binary_voxels(const std::vector<std::vector<std::vector<double>>>& coordinates, float size, Mesh mesh);