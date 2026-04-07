#include "marching_cubes.hpp"

#include <array>
#include <deque>
#include <limits>
#include <math.h>

#include "raylib.h"
#include "raymath.h"

#include "logging.hpp"

static const std::vector<std::array<int,3>> DIRECTIONS {{0,-1,0},{0,1,0},{1,0,0},{-1,0,0},{0,0,1},{0,0,-1}};

static std::array<double,3> array_3_add(std::array<double,3> a, std::array<double,3> b) {
    return std::array<double,3>{a[0]+b[0],a[1]+b[1],a[2]+b[2]};
}

static Color color_lerp(double iso_value,Color a, Color b, double valp1,double valp2) {
    double mu = (iso_value - valp1) / (valp2 - valp1);
    if (std::fabs(iso_value-valp1) < 0.00001)
      return(a);
    if (std::fabs(iso_value-valp2) < 0.00001)
        return(b);
    if (std::fabs(valp1-valp2) < 0.00001)
        return(a);
    Color result {};
    result.r = a.r + mu * (b.r-a.r);
    result.g = a.g + mu * (b.g-a.g);
    result.b = a.b + mu * (b.b-a.b);
    result.a = 255;
    return result;
}

/*
   Linearly interpolate the position where an isosurface cuts
   an edge between two vertices, each with their own scalar value
*/
static std::array<double,3> VertexInterp(double iso_value,
                                         const std::array<double,3>& p1,
                                         const std::array<double,3>& p2,
                                         double valp1, double valp2) 
{
    // If the iso_value is exactly on a vertex, return that vertex
    if (std::fabs(iso_value - valp1) < 1e-12) return p1;
    if (std::fabs(iso_value - valp2) < 1e-12) return p2;

    // If the edge is almost flat, return the midpoint
    if (std::fabs(valp1 - valp2) < 1e-12) {
        return std::array<double,3>{(p1[0]+p2[0])*0.5,
                                     (p1[1]+p2[1])*0.5,
                                     (p1[2]+p2[2])*0.5};
    }

    // Linear interpolation along the edge
    double mu = (iso_value - valp1) / (valp2 - valp1);
    return std::array<double,3>{ p1[0] + mu*(p2[0]-p1[0]),
                                 p1[1] + mu*(p2[1]-p1[1]),
                                 p1[2] + mu*(p2[2]-p1[2]) };
}

static int edgeTable[256]={
0x0  , 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
0x190, 0x99 , 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
0x230, 0x339, 0x33 , 0x13a, 0x636, 0x73f, 0x435, 0x53c,
0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
0x3a0, 0x2a9, 0x1a3, 0xaa , 0x7a6, 0x6af, 0x5a5, 0x4ac,
0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
0x460, 0x569, 0x663, 0x76a, 0x66 , 0x16f, 0x265, 0x36c,
0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff , 0x3f5, 0x2fc,
0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55 , 0x15c,
0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc ,
0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
0xcc , 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
0x15c, 0x55 , 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
0x2fc, 0x3f5, 0xff , 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
0x36c, 0x265, 0x16f, 0x66 , 0x76a, 0x663, 0x569, 0x460,
0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa , 0x1a3, 0x2a9, 0x3a0,
0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33 , 0x339, 0x230,
0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99 , 0x190,
0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x0   };
static int triTable[256][16] =
{{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1},
{3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1, -1},
{3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1},
{3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1, -1},
{9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1},
{9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
{2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1, -1},
{8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1, -1},
{9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
{4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1, -1},
{3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1, -1},
{1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1, -1},
{4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1, -1},
{4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
{5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1, -1},
{2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1, -1},
{9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
{0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
{2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1, -1},
{10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1},
{4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1, -1},
{5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1, -1},
{5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1},
{9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1, -1},
{0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1, -1},
{1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1},
{10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1, -1},
{8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1, -1},
{2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1},
{7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1, -1},
{2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1, -1},
{11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1, -1},
{5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1},
{11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1},
{11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
{1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1},
{9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1, -1},
{5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1, -1},
{2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
{5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1, -1},
{6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1, -1},
{3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1, -1},
{6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1},
{5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1, -1},
{1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
{10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1, -1},
{6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1, -1},
{8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1, -1},
{7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1},
{3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
{5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1, -1},
{0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1},
{9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1},
{8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1, -1},
{5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1},
{0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1},
{6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1, -1},
{10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1},
{10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1, -1},
{8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1, -1},
{1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1, -1},
{0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1},
{10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1, -1},
{3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1, -1},
{6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1},
{9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1, -1},
{8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1},
{3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1, -1},
{6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1, -1},
{0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1, -1},
{10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1, -1},
{10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1, -1},
{2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1},
{7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1, -1},
{7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1, -1},
{2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1},
{1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1},
{11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1, -1},
{8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1},
{0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1, -1},
{7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
{10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
{2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
{6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1, -1},
{7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1, -1},
{2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1},
{1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1, -1},
{10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1, -1},
{10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1, -1},
{0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1, -1},
{7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1, -1},
{6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1, -1},
{8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1},
{9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1, -1},
{6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1, -1},
{4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1, -1},
{10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1},
{8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1},
{0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1, -1},
{1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1},
{8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1, -1},
{10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1, -1},
{4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1},
{10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
{5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
{11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1, -1},
{9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
{6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1, -1},
{7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1, -1},
{3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1},
{7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1, -1},
{3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1, -1},
{6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1},
{9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1, -1},
{1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1},
{4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1},
{7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1, -1},
{6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1},
{3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1, -1},
{0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1, -1},
{6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1, -1},
{0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1},
{11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1},
{6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1, -1},
{5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1, -1},
{9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1, -1},
{1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1},
{1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1},
{10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1, -1},
{0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1, -1},
{5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1},
{10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1, -1},
{11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1, -1},
{9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1, -1},
{7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1},
{2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1},
{8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1, -1},
{9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1, -1},
{9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1},
{1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1, -1},
{9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1, -1},
{9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1},
{5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1, -1},
{0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1, -1},
{10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1},
{2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1, -1},
{0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1},
{0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1},
{9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1, -1},
{5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1},
{3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1},
{5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1, -1},
{8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1, -1},
{0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1, -1},
{9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1, -1},
{1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1, -1},
{3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1},
{4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1, -1},
{9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1},
{11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1, -1},
{11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1, -1},
{2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1, -1},
{9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1},
{3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1},
{1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1, -1},
{4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1, -1},
{4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1},
{3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1, -1},
{0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1},
{9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1, -1},
{1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}};

static void flood(std::vector<std::vector<std::vector<double>>>& grid_result, const std::vector<std::vector<std::vector<double>>>& grid, int i, int j, int k) {
    std::deque<std::array<int,3>> dfs {};
    dfs.push_back({i,j,k});
    int n_x = grid.size();
    int n_y = grid[0].size();
    int n_z = grid[0][0].size();
    while (!dfs.empty()) {
        int x = dfs.back()[0];
        int y = dfs.back()[1];
        int z = dfs.back()[2];
        dfs.pop_back();
        grid_result[x][y][z] = 1.0;
        for (const auto& d : DIRECTIONS) {
            int x2 = x+d[0];
            int y2 = y+d[1];
            int z2 = z+d[2];
            if (x2 < 0 || x2 >= n_x ||
                y2 < 0 || y2 >= n_y ||
                z2 < 0 || z2 >= n_z ||
                !(grid_result[x2][y2][z2] < 0.0 && grid[x2][y2][z2] > 0.0)) continue;
            dfs.push_back({x2,y2,z2});
        }
    }
}

std::pair<std::pair<std::vector<std::vector<std::vector<double>>>,std::vector<std::vector<std::vector<Color>>>>,std::array<double,3>> voxelize_pcd(std::vector<std::array<double,3>> coordinates, std::vector<std::array<unsigned char,3>> colors, double voxel_size) {
    DEBUG("Voxellizing point cloud distribution of " + std::to_string(coordinates.size()) + " points with voxel size " + std::to_string(voxel_size) + "...");
    std::array<double,3> bottom_left {std::numeric_limits<double>::infinity(),std::numeric_limits<double>::infinity(),std::numeric_limits<double>::infinity()};
    std::array<double,3> top_right {-std::numeric_limits<double>::infinity(),-std::numeric_limits<double>::infinity(),-std::numeric_limits<double>::infinity()};
    // defining the bounding box ^
    for (const std::array<double,3>& a : coordinates) {
        bottom_left[0] = std::min(bottom_left[0],a[0]);
        bottom_left[1] = std::min(bottom_left[1],a[1]);
        bottom_left[2] = std::min(bottom_left[2],a[2]); 

        top_right[0] = std::max(top_right[0],a[0]);
        top_right[1] = std::max(top_right[1],a[1]);
        top_right[2] = std::max(top_right[2],a[2]);
    }
    int n_x = (top_right[0]-bottom_left[0])/voxel_size + 4;
    int n_y = (top_right[1]-bottom_left[1])/voxel_size + 4;
    int n_z = (top_right[2]-bottom_left[2])/voxel_size + 4;
    DEBUG("Dimensions for voxel grid: " + std::to_string(n_x) + "," + std::to_string(n_y) + "," + std::to_string(n_z));
    std::vector<std::vector<std::vector<double>>> grid (n_x,std::vector<std::vector<double>>(n_y,std::vector<double>(n_z,1.0)));
    std::vector<std::vector<std::vector<std::pair<std::array<double,3>,int>>>> color_sum (n_x,std::vector<std::vector<std::pair<std::array<double,3>,int>>>(n_y,std::vector<std::pair<std::array<double,3>,int>>(n_z,std::pair<std::array<double,3>,int>{{0.0,0.0,0.0},0})));
    std::vector<std::vector<std::vector<Color>>> color_result (n_x,std::vector<std::vector<Color>>(n_y,std::vector<Color>(n_z,Color(0,0,0,0))));

    std::array<double,3> bottom_left_voxel = std::array<double,3>{bottom_left[0]-2.0*voxel_size,bottom_left[1]-2.0*voxel_size,bottom_left[2]-2.0*voxel_size};
    // Bottom left corner of the voxel with the smallest coordinates, offset to ensure it contains all the points

    for (int i = 0; i < coordinates.size(); i++) {
        std::array<double,3> point = std::array<double,3>{coordinates[i][0],coordinates[i][1],coordinates[i][2]};
        std::array<double,3> color = std::array<double,3>{colors[i][0],colors[i][1],colors[i][2]};

        int x_index = std::floor((point[0]-bottom_left_voxel[0])/voxel_size);
        int y_index = std::floor((point[1]-bottom_left_voxel[1])/voxel_size);
        int z_index = std::floor((point[2]-bottom_left_voxel[2])/voxel_size);

        grid[x_index][y_index][z_index] = -1.0;
        color_sum[x_index][y_index][z_index].first = std::array<double,3>{color_sum[x_index][y_index][z_index].first[0]+color[0],color_sum[x_index][y_index][z_index].first[1]+color[1],color_sum[x_index][y_index][z_index].first[2]+color[2]};
        color_sum[x_index][y_index][z_index].second++;
    }
    for (int i = 0; i < n_x; i++) {
        for (int j = 0; j < n_y; j++) {
            for (int k = 0; k < n_z; k++) {
                double scale = 1.0/((double)color_sum[i][j][k].second);
                std::array<double,3> color = std::array<double,3>{color_sum[i][j][k].first[0]*scale,color_sum[i][j][k].first[1]*scale,color_sum[i][j][k].first[2]*scale};
                color_result[i][j][k] = Color((int) color[0],(int) color[1],(int) color[2],255);
            }
        }
    }
    int voxel_count_before = 0;
    for (int i = 0; i < n_x; i++)
        for (int j = 0; j < n_y; j++)
            for (int k = 0; k < n_z; k++)
                voxel_count_before += grid[i][j][k] < 0.0;
    // Flood fill exterior to fill hollow insides
    std::vector<std::vector<std::vector<double>>> grid_result (n_x,std::vector<std::vector<double>>(n_y,std::vector<double>(n_z,-1.0)));
    flood(grid_result,grid,0,0,0);
    int voxel_count_after = 0;
    for (int i = 0; i < n_x; i++)
        for (int j = 0; j < n_y; j++)
            for (int k = 0; k < n_z; k++)
                voxel_count_after += grid_result[i][j][k] < 0.0;
    INFO("Voxel grid voxel count after flooding went from " + std::to_string(voxel_count_before) + " to " + std::to_string(voxel_count_after));
    DEBUG("Finished creating voxel grid, ended with voxel grid of size (" + std::to_string(n_x) + "," + std::to_string(n_y) + "," + std::to_string(n_z) + ")");
    return {{grid_result,color_result},bottom_left_voxel};
}

struct MeshData {
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<unsigned short> indices;
    std::vector<unsigned char> colors;
};

std::vector<Mesh> march_cubes(std::vector<std::vector<std::vector<double>>> grid, std::vector<std::vector<std::vector<Color>>> color_grid, std::array<double,3> bottom_left_voxel, double voxel_size, double iso_value) {
    // https://paulbourke.net/geometry/polygonise/

    int n_x = grid.size()-1;
    int n_y = grid[0].size()-1;
    int n_z = grid[0][0].size()-1;
    DEBUG("Marching cubes with voxel dimension (" + std::to_string(n_x) + "," + std::to_string(n_y) + "," + std::to_string(n_z) + ")," +
    " voxel_size " + std::to_string(voxel_size) + ", iso_value " + std::to_string(iso_value) + "...");
    // Number of 8 corner cubic regions, which is why we subtract one

    const int MESH_VOXELS = 15;
    int x_meshes = (n_x + MESH_VOXELS - 1) / MESH_VOXELS;
    int y_meshes = (n_y + MESH_VOXELS - 1) / MESH_VOXELS;
    int z_meshes = (n_z + MESH_VOXELS - 1) / MESH_VOXELS;
    int n_meshes = x_meshes*y_meshes*z_meshes;
    std::vector<MeshData> mesh_data (n_meshes, MeshData{});

    DEBUG("Using " + std::to_string(n_meshes) + " meshes...");

    for (int i = 0; i < n_x; i++) {
        for (int j = 0; j < n_y; j++) {
            for (int k = 0; k < n_z; k++) {
                int mesh_index = (i/MESH_VOXELS) + (j/MESH_VOXELS)*x_meshes + (k/MESH_VOXELS)*y_meshes*x_meshes;
                unsigned int cubeindex = 0;
                // vertex 0 will be the min coordinate corner, vertex 6 will be the max coordinate corner
                if (grid[i][j][k] < iso_value) cubeindex |= 1;
                if (grid[i+1][j][k] < iso_value) cubeindex |= 2;
                if (grid[i+1][j][k+1] < iso_value) cubeindex |= 4;
                if (grid[i][j][k+1] < iso_value) cubeindex |= 8;
                if (grid[i][j+1][k] < iso_value) cubeindex |= 16;
                if (grid[i+1][j+1][k] < iso_value) cubeindex |= 32;
                if (grid[i+1][j+1][k+1] < iso_value) cubeindex |= 64;
                if (grid[i][j+1][k+1] < iso_value) cubeindex |= 128;
                if (cubeindex == 0) continue; // completely inside or outside surface
                // DEBUG("Voxel found with cubeindex " + std::to_string(cubeindex));
                for (int tri = 0; tri <= 5; tri++) {
                    if (triTable[cubeindex][tri*3] == -1) break;
                    for (int edge_index = tri*3; edge_index < tri*3+3; edge_index++) {
                        int edge = triTable[cubeindex][edge_index];
                        std::array<double,3> pos;
                        Color col;
                        if (edge == 0) {
                            std::array<double,3> p1 = array_3_add(bottom_left_voxel, std::array<double,3>{i*voxel_size,j*voxel_size,k*voxel_size});
                            std::array<double,3> p2 = array_3_add(bottom_left_voxel, std::array<double,3>{(i+1)*voxel_size,j*voxel_size,(k)*voxel_size});
                            double valp1 = grid[i][j][k];
                            double valp2 = grid[i+1][j][k];
                            pos = VertexInterp(iso_value,p1,p2,valp1,valp2);

                            Color c1 = color_grid[i][j][k];
                            Color c2 = color_grid[i+1][j][k];
                            if (c1.a != 0 && c2.a != 0)
                                col = color_lerp(iso_value,c1,c2,valp1,valp2);
                            else if (c1.a != 0)
                                col = c1;
                            else if (c2.a != 0)
                                col = c2;
                        } else if (edge == 1) {
                            std::array<double,3> p1 = array_3_add(bottom_left_voxel, std::array<double,3>{(i+1)*voxel_size,j*voxel_size,(k)*voxel_size});
                            std::array<double,3> p2 = array_3_add(bottom_left_voxel, std::array<double,3>{(i+1)*voxel_size,j*voxel_size,(k+1)*voxel_size});
                            double valp1 = grid[i+1][j][k];
                            double valp2 = grid[i+1][j][k+1];
                            pos = VertexInterp(iso_value,p1,p2,valp1,valp2);

                            Color c1 = color_grid[i+1][j][k];
                            Color c2 = color_grid[i+1][j][k+1];
                            if (c1.a != 0 && c2.a != 0)
                                col = color_lerp(iso_value,c1,c2,valp1,valp2);
                            else if (c1.a != 0)
                                col = c1;
                            else if (c2.a != 0)
                                col = c2;
                        } else if (edge == 2) {
                            std::array<double,3> p1 = array_3_add(bottom_left_voxel, std::array<double,3>{(i+1)*voxel_size,j*voxel_size,(k+1)*voxel_size});
                            std::array<double,3> p2 = array_3_add(bottom_left_voxel, std::array<double,3>{(i)*voxel_size,j*voxel_size,(k+1)*voxel_size});
                            double valp1 = grid[i+1][j][k+1];
                            double valp2 = grid[i][j][k+1];
                            pos = VertexInterp(iso_value,p1,p2,valp1,valp2);

                            Color c1 = color_grid[i+1][j][k+1];
                            Color c2 = color_grid[i][j][k+1];
                            if (c1.a != 0 && c2.a != 0)
                                col = color_lerp(iso_value,c1,c2,valp1,valp2);
                            else if (c1.a != 0)
                                col = c1;
                            else if (c2.a != 0)
                                col = c2;
                        } else if (edge == 3) {
                            std::array<double,3> p1 = array_3_add(bottom_left_voxel, std::array<double,3>{i*voxel_size,j*voxel_size,(k+1)*voxel_size});
                            std::array<double,3> p2 = array_3_add(bottom_left_voxel, std::array<double,3>{i*voxel_size,j*voxel_size,k*voxel_size});
                            double valp1 = grid[i][j][k+1];
                            double valp2 = grid[i][j][k];
                            pos = VertexInterp(iso_value,p1,p2,valp1,valp2);

                            Color c1 = color_grid[i][j][k+1];
                            Color c2 = color_grid[i][j][k];
                            if (c1.a != 0 && c2.a != 0)
                                col = color_lerp(iso_value,c1,c2,valp1,valp2);
                            else if (c1.a != 0)
                                col = c1;
                            else if (c2.a != 0)
                                col = c2;
                        } else if (edge == 4) {
                            std::array<double,3> p1 = array_3_add(bottom_left_voxel, std::array<double,3>{i*voxel_size,(j+1)*voxel_size,k*voxel_size});
                            std::array<double,3> p2 = array_3_add(bottom_left_voxel, std::array<double,3>{(i+1)*voxel_size,(j+1)*voxel_size,(k)*voxel_size});
                            double valp1 = grid[i][j+1][k];
                            double valp2 = grid[i+1][j+1][k];
                            pos = VertexInterp(iso_value,p1,p2,valp1,valp2);

                            Color c1 = color_grid[i][j+1][k];
                            Color c2 = color_grid[i+1][j+1][k];
                            if (c1.a != 0 && c2.a != 0)
                                col = color_lerp(iso_value,c1,c2,valp1,valp2);
                            else if (c1.a != 0)
                                col = c1;
                            else if (c2.a != 0)
                                col = c2;
                        } else if (edge == 5) {
                            std::array<double,3> p1 = array_3_add(bottom_left_voxel, std::array<double,3>{(i+1)*voxel_size,(j+1)*voxel_size,(k)*voxel_size});
                            std::array<double,3> p2 = array_3_add(bottom_left_voxel, std::array<double,3>{(i+1)*voxel_size,(j+1)*voxel_size,(k+1)*voxel_size});
                            double valp1 = grid[i+1][j+1][k];
                            double valp2 = grid[i+1][j+1][k+1];
                            pos = VertexInterp(iso_value,p1,p2,valp1,valp2);

                            Color c1 = color_grid[i+1][j+1][k];
                            Color c2 = color_grid[i+1][j+1][k+1];
                            if (c1.a != 0 && c2.a != 0)
                                col = color_lerp(iso_value,c1,c2,valp1,valp2);
                        } else if (edge == 6) {
                            std::array<double,3> p1 = array_3_add(bottom_left_voxel, std::array<double,3>{(i+1)*voxel_size,(j+1)*voxel_size,(k+1)*voxel_size});
                            std::array<double,3> p2 = array_3_add(bottom_left_voxel, std::array<double,3>{(i)*voxel_size,(j+1)*voxel_size,(k+1)*voxel_size});
                            double valp1 = grid[i+1][j+1][k+1];
                            double valp2 = grid[i][j+1][k+1];
                            pos = VertexInterp(iso_value,p1,p2,valp1,valp2);

                            Color c1 = color_grid[i+1][j+1][k+1];
                            Color c2 = color_grid[i][j+1][k+1];
                            if (c1.a != 0 && c2.a != 0)
                                col = color_lerp(iso_value,c1,c2,valp1,valp2);
                            else if (c1.a != 0)
                                col = c1;
                            else if (c2.a != 0)
                                col = c2;
                        } else if (edge == 7) {
                            std::array<double,3> p1 = array_3_add(bottom_left_voxel, std::array<double,3>{(i)*voxel_size,(j+1)*voxel_size,(k+1)*voxel_size});
                            std::array<double,3> p2 = array_3_add(bottom_left_voxel, std::array<double,3>{i*voxel_size,(j+1)*voxel_size,k*voxel_size});
                            double valp1 = grid[i][j+1][k+1];
                            double valp2 = grid[i][j+1][k];
                            pos = VertexInterp(iso_value,p1,p2,valp1,valp2);

                            Color c1 = color_grid[i][j+1][k+1];
                            Color c2 = color_grid[i][j+1][k];
                            if (c1.a != 0 && c2.a != 0)
                                col = color_lerp(iso_value,c1,c2,valp1,valp2);
                        } else if (edge == 8) {
                            std::array<double,3> p1 = array_3_add(bottom_left_voxel, std::array<double,3>{i*voxel_size,j*voxel_size,k*voxel_size});
                            std::array<double,3> p2 = array_3_add(bottom_left_voxel, std::array<double,3>{i*voxel_size,(j+1)*voxel_size,k*voxel_size});
                            double valp1 = grid[i][j][k];
                            double valp2 = grid[i][j+1][k];
                            pos = VertexInterp(iso_value,p1,p2,valp1,valp2);

                            Color c1 = color_grid[i][j][k];
                            Color c2 = color_grid[i][j+1][k];
                            if (c1.a != 0 && c2.a != 0)
                                col = color_lerp(iso_value,c1,c2,valp1,valp2);
                            else if (c1.a != 0)
                                col = c1;
                            else if (c2.a != 0)
                                col = c2;
                        } else if (edge == 9) {
                            std::array<double,3> p1 = array_3_add(bottom_left_voxel, std::array<double,3>{(i+1)*voxel_size,j*voxel_size,(k)*voxel_size});
                            std::array<double,3> p2 = array_3_add(bottom_left_voxel, std::array<double,3>{(i+1)*voxel_size,(j+1)*voxel_size,(k)*voxel_size});
                            double valp1 = grid[i+1][j][k];
                            double valp2 = grid[i+1][j+1][k];
                            pos = VertexInterp(iso_value,p1,p2,valp1,valp2);

                            Color c1 = color_grid[i+1][j][k];
                            Color c2 = color_grid[i+1][j+1][k];
                            if (c1.a != 0 && c2.a != 0)
                                col = color_lerp(iso_value,c1,c2,valp1,valp2);
                            else if (c1.a != 0)
                                col = c1;
                            else if (c2.a != 0)
                                col = c2;
                        } else if (edge == 10) {
                            std::array<double,3> p1 = array_3_add(bottom_left_voxel, std::array<double,3>{(i+1)*voxel_size,j*voxel_size,(k+1)*voxel_size});
                            std::array<double,3> p2 = array_3_add(bottom_left_voxel, std::array<double,3>{(i+1)*voxel_size,(j+1)*voxel_size,(k+1)*voxel_size});
                            double valp1 = grid[i+1][j][k+1];
                            double valp2 = grid[i+1][j+1][k+1];
                            pos = VertexInterp(iso_value,p1,p2,valp1,valp2);

                            Color c1 = color_grid[i+1][j][k+1];
                            Color c2 = color_grid[i+1][j+1][k+1];
                            if (c1.a != 0 && c2.a != 0)
                                col = color_lerp(iso_value,c1,c2,valp1,valp2);
                            else if (c1.a != 0)
                                col = c1;
                            else if (c2.a != 0)
                                col = c2;
                        } else if (edge == 11) {
                            std::array<double,3> p1 = array_3_add(bottom_left_voxel, std::array<double,3>{(i)*voxel_size,j*voxel_size,(k+1)*voxel_size});
                            std::array<double,3> p2 = array_3_add(bottom_left_voxel, std::array<double,3>{(i)*voxel_size,(j+1)*voxel_size,(k+1)*voxel_size});
                            double valp1 = grid[i][j][k+1];
                            double valp2 = grid[i][j+1][k+1];
                            pos = VertexInterp(iso_value,p1,p2,valp1,valp2);

                            Color c1 = color_grid[i][j][k+1];
                            Color c2 = color_grid[i][j+1][k+1];
                            if (c1.a != 0 && c2.a != 0)
                                col = color_lerp(iso_value,c1,c2,valp1,valp2);
                            else if (c1.a != 0)
                                col = c1;
                            else if (c2.a != 0)
                                col = c2;
                        }
                        mesh_data[mesh_index].indices.push_back(mesh_data[mesh_index].vertices.size()/3);
                        mesh_data[mesh_index].vertices.push_back(pos[0]);
                        mesh_data[mesh_index].vertices.push_back(pos[1]);
                        mesh_data[mesh_index].vertices.push_back(pos[2]);
                        mesh_data[mesh_index].colors.push_back(col.r);
                        mesh_data[mesh_index].colors.push_back(col.g);
                        mesh_data[mesh_index].colors.push_back(col.b);
                        mesh_data[mesh_index].colors.push_back(col.a);
                    }
                }
            }
        }
    }
    std::vector<Mesh> result {};
    for (int i = 0; i < n_meshes; i++) {
        if (mesh_data[i].vertices.size() == 0)
            continue;
        Mesh mesh {};
        mesh.triangleCount = mesh_data[i].indices.size()/3;
        mesh.vertexCount = mesh_data[i].vertices.size()/3;
        DEBUG(std::to_string(mesh.triangleCount) + "," + std::to_string(mesh.vertexCount));
        mesh.vertices = (float*)MemAlloc(mesh.vertexCount*sizeof(float)*3);
        mesh.indices = (unsigned short*)MemAlloc(mesh.triangleCount*sizeof(unsigned short)*3);
        mesh.colors = (unsigned char*)MemAlloc(mesh.vertexCount*sizeof(unsigned char)*4);
        for (int j = 0; j < mesh_data[i].vertices.size(); j++)
            mesh.vertices[j] = mesh_data[i].vertices[j];
        for (int j = 0; j < mesh_data[i].indices.size(); j++)
            mesh.indices[j] = mesh_data[i].indices[j];
        for (int j = 0; j < mesh_data[i].colors.size(); j++)
            mesh.colors[j] = mesh_data[i].colors[j];
        result.push_back(mesh);
    }
    DEBUG("Finished marching cubes, mesh count: " + std::to_string(result.size()));
    return result;
}