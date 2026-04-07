#include "voxel_thinning.hpp"

#include <array>

#include "logging.hpp"

static int LUT[256] {};
static const std::vector<std::array<int,3>> DIRECTIONS {{0,-1,0},{0,1,0},{1,0,0},{-1,0,0},{0,0,1},{0,0,-1}};
static const std::vector<std::array<int,3>> DIRECTIONS_26 {{-1,-1,-1},{0,-1,-1},{1,-1,-1},
                                                        {-1, 0,-1},{0, 0,-1},{1, 0,-1},
                                                        {-1, 1,-1},{0, 1,-1},{1, 1,-1},
                                                        {-1,-1, 0},{0,-1, 0},{1,-1, 0},
                                                        {-1, 0, 0},          {1, 0, 0},
                                                        {-1, 1, 0},{0, 1, 0},{1, 1, 0},
                                                        {-1,-1, 1},{0,-1, 1},{1,-1, 1},
                                                        {-1, 0, 1},{0, 0, 1},{1, 0, 1},
                                                        {-1, 1, 1},{0, 1, 1},{1, 1, 1}};

static bool is_border(int i, int j, int k, const std::vector<std::vector<std::vector<double>>>& voxels, const std::array<int,3>& direction) {
    int x = i+direction[0];
    int y = j+direction[1];
    int z = k+direction[2];
    int n = voxels.size();
    int m = voxels[0].size();
    int q = voxels[0][0].size();
    if (x < 0 || x >= n || y < 0 || y >= m || z < 0 || z >= q)
        return true; // boundary of volume is treated as border
    return voxels[x][y][z] > 0.0;
}

static bool is_endpoint(int i, int j, int k, const std::vector<std::vector<std::vector<double>>>& voxels) {
    int n = voxels.size();
    int m = voxels[0].size();
    int q = voxels[0][0].size();
    int count = -1;
    for (int a = std::max(0,i-1); a <= std::min(n-1,i+1);a++) {
        for (int b = std::max(0,j-1); b <= std::min(m-1,j+1);b++) {
            for (int c = std::max(0,k-1); c <= std::min(q-1,k+1);c++) {
                count += voxels[a][b][c] < 0.0;
            }
        }
    }
    return count <= 1;
}

static int euler_number(int a, int b, int c, const std::vector<std::vector<std::vector<double>>>& voxels) {
    int index = 0;
    if (voxels[a][b][c] < 0.0)
        index |= 1;
    if (voxels[a+1][b][c] < 0.0)
        index |= 2;
    if (voxels[a][b+1][c] < 0.0)
        index |= 4;
    if (voxels[a+1][b+1][c] < 0.0)
        index |= 8;
    if (voxels[a][b][c+1] < 0.0)
        index |= 16;
    if (voxels[a+1][b][c+1] < 0.0)
        index |= 32;
    if (voxels[a][b+1][c+1] < 0.0)
        index |= 64;
    if (voxels[a+1][b+1][c+1] < 0.0)
        index |= 128;
    return LUT[index];
}

static bool is_euler_invariant(int i, int j, int k, std::vector<std::vector<std::vector<double>>>& voxels) {
    int n = voxels.size();
    int m = voxels[0].size();
    int q = voxels[0][0].size();
    int current_sum = 0;
    for (int a = std::max(0,i-1); a <= std::min(n-2,i);a++) {
        for (int b = std::max(0,j-1); b <= std::min(m-2,j);b++) {
            for (int c = std::max(0,k-1); c <= std::min(q-2,k);c++) {
                current_sum += euler_number(a,b,c,voxels);
            }
        }
    }
    return current_sum == 0;
}
static void mark_neighbors(int comp, int a, int b, int c, int i, int j, int k, std::array<std::array<std::array<int,3>,3>,3>& component_number, std::vector<std::vector<std::vector<double>>>& voxels) {
    int n = voxels.size();
    int m = voxels[0].size();
    int q = voxels[0][0].size();
    for (const auto& d : DIRECTIONS_26) {
        int dx = d[0];
        int dy = d[1];
        int dz = d[2];
        int x = a+dx;
        int y = b+dy;
        int z = c+dz;
        if (x < 0 || x < i-1 ||
            y < 0 || y < j-1 ||
            z < 0 || z < k-1 ||
            x >= n || x > i+1 ||
            y >= m || y > j+1 ||
            z >= q || z > k+1 ||
            voxels[x][y][z] > 0.0) continue;
        int index_i = x-i+1;
        int index_j = y-j+1;
        int index_k = z-k+1;
        if (component_number[index_i][index_j][index_k] != 0) continue;
        component_number[index_i][index_j][index_k] = comp;
        mark_neighbors(comp,x,y,z,i,j,k,component_number,voxels);
    }
}
static bool is_simple(int i, int j, int k, std::vector<std::vector<std::vector<double>>>& voxels) {
    int n = voxels.size();
    int m = voxels[0].size();
    int q = voxels[0][0].size();
    std::array<std::array<std::array<int,3>,3>,3> component_number {};
    int comp = 1;
    voxels[i][j][k] = 1.0;
    for (int a = std::max(0,i-1); a <= std::min(n-1,i+1);a++) {
        for (int b = std::max(0,j-1); b <= std::min(m-1,j+1);b++) {
            for (int c = std::max(0,k-1); c <= std::min(q-1,k+1);c++) {
                if (voxels[a][b][c] > 0.0 || a==i&&b==j&&c==k) continue;
                int index_i = a-i+1;
                int index_j = b-j+1;
                int index_k = c-k+1;
                if (component_number[index_i][index_j][index_k] == 0) {
                    component_number[index_i][index_j][index_k] = comp;
                    mark_neighbors(comp,a,b,c,i,j,k,component_number,voxels);
                    comp++;
                }
            }
        }
    }
    voxels[i][j][k] = -1.0;
    return comp == 2;
}

void fill_lut() {
    LUT[1]  =  1;
    LUT[3]  = -1;
    LUT[5]  = -1;
    LUT[7]  =  1;
    LUT[9]  = -3;
    LUT[11] = -1;
    LUT[13] = -1;
    LUT[15] =  1;
    LUT[17] = -1;
    LUT[19] =  1;
    LUT[21] =  1;
    LUT[23] = -1;
    LUT[25] =  3;
    LUT[27] =  1;
    LUT[29] =  1;
    LUT[31] = -1;
    LUT[33] = -3;
    LUT[35] = -1;
    LUT[37] =  3;
    LUT[39] =  1;
    LUT[41] =  1;
    LUT[43] = -1;
    LUT[45] =  3;
    LUT[47] =  1;
    LUT[49] = -1;
    LUT[51] =  1;

    LUT[53] =  1;
    LUT[55] = -1;
    LUT[57] =  3;
    LUT[59] =  1;
    LUT[61] =  1;
    LUT[63] = -1;
    LUT[65] = -3;
    LUT[67] =  3;
    LUT[69] = -1;
    LUT[71] =  1;
    LUT[73] =  1;
    LUT[75] =  3;
    LUT[77] = -1;
    LUT[79] =  1;
    LUT[81] = -1;
    LUT[83] =  1;
    LUT[85] =  1;
    LUT[87] = -1;
    LUT[89] =  3;
    LUT[91] =  1;
    LUT[93] =  1;
    LUT[95] = -1;
    LUT[97] =  1;
    LUT[99] =  3;
    LUT[101] =  3;
    LUT[103] =  1;

    LUT[105] =  5;
    LUT[107] =  3;
    LUT[109] =  3;
    LUT[111] =  1;
    LUT[113] = -1;
    LUT[115] =  1;
    LUT[117] =  1;
    LUT[119] = -1;
    LUT[121] =  3;
    LUT[123] =  1;
    LUT[125] =  1;
    LUT[127] = -1;
    LUT[129] = -7;
    LUT[131] = -1;
    LUT[133] = -1;
    LUT[135] =  1;
    LUT[137] = -3;
    LUT[139] = -1;
    LUT[141] = -1;
    LUT[143] =  1;
    LUT[145] = -1;
    LUT[147] =  1;
    LUT[149] =  1;
    LUT[151] = -1;
    LUT[153] =  3;
    LUT[155] =  1;

    LUT[157] =  1;
    LUT[159] = -1;
    LUT[161] = -3;
    LUT[163] = -1;
    LUT[165] =  3;
    LUT[167] =  1;
    LUT[169] =  1;
    LUT[171] = -1;
    LUT[173] =  3;
    LUT[175] =  1;
    LUT[177] = -1;
    LUT[179] =  1;
    LUT[181] =  1;
    LUT[183] = -1;
    LUT[185] =  3;
    LUT[187] =  1;
    LUT[189] =  1;
    LUT[191] = -1;
    LUT[193] = -3;
    LUT[195] =  3;
    LUT[197] = -1;
    LUT[199] =  1;
    LUT[201] =  1;
    LUT[203] =  3;
    LUT[205] = -1;
    LUT[207] =  1;

    LUT[209] = -1;
    LUT[211] =  1;
    LUT[213] =  1;
    LUT[215] = -1;
    LUT[217] =  3;
    LUT[219] =  1;
    LUT[221] =  1;
    LUT[223] = -1;
    LUT[225] =  1;
    LUT[227] =  3;
    LUT[229] =  3;
    LUT[231] =  1;
    LUT[233] =  5;
    LUT[235] =  3;
    LUT[237] =  3;
    LUT[239] =  1;
    LUT[241] = -1;
    LUT[243] =  1;
    LUT[245] =  1;
    LUT[247] = -1;
    LUT[249] =  3;
    LUT[251] =  1;
    LUT[253] =  1;
    LUT[255] = -1;
}

int thin_voxels_once(std::vector<std::vector<std::vector<double>>>& voxels) {
    int n = voxels.size();
    int m = voxels[0].size();
    int q = voxels[0][0].size();
    std::vector<std::array<int,3>> remove_list {};
    int total_removed = 0;
    for (const auto& d : DIRECTIONS) {
        int dx = d[0];
        int dy = d[1];
        int dz = d[2];
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                for (int k = 0; k < q; k++) {
                    if (voxels[i][j][k] < 0.0&&
                        is_border(i,j,k,voxels,d)&&
                        !is_endpoint(i,j,k,voxels)&&
                        is_euler_invariant(i,j,k,voxels)&&
                        is_simple(i,j,k,voxels))
                        remove_list.push_back({i,j,k});
                    if (voxels[i][j][k] < 0.0) {
                        bool border   = is_border(i,j,k,voxels,d);
                        bool endpoint = is_endpoint(i,j,k,voxels);
                        bool euler    = is_euler_invariant(i,j,k,voxels);
                        bool simple   = is_simple(i,j,k,voxels);
                    }
                }
            }
        }
        INFO("Removing " + std::to_string(remove_list.size()) + " voxels...");
        total_removed += remove_list.size();
        for (const auto& a : remove_list) {
            int x = a[0];
            int y = a[1];
            int z = a[2];
            voxels[x][y][z] = 1.0;
        }
        if (remove_list.size() > 0)
            remove_list.clear();
    }
    return total_removed;
}

void thin_voxels(std::vector<std::vector<std::vector<double>>>& voxels) {
    fill_lut();
    int n = voxels.size();
    int m = voxels[0].size();
    int q = voxels[0][0].size();

    int fg_count = 0;
    for (int i=0;i<n;i++)
        for (int j=0;j<m;j++)
            for (int k=0;k<q;k++)
                fg_count += voxels[i][j][k] < 0.0;
    INFO("Foreground voxels: " + std::to_string(fg_count));

    bool changed = true;
    int total_removed = 0;

    while (changed) {
        int num = thin_voxels_once(voxels);
        changed = num > 0;
        total_removed += changed;
    }
    INFO("Complete, removed " + std::to_string(total_removed) + " voxels");
}