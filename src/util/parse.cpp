#include "util/parse.hpp"

#include <fstream>
#include <sstream>
#include <iostream>

std::pair<std::vector<std::array<double,3>>,std::vector<std::array<unsigned char,3>>> parse_xyz(std::string filename, bool flip_yz) {
    std::ifstream file(filename);
    if (!file)
        return {};
    std::string line {};
    std::vector<std::array<double,3>> positions {};
    std::vector<std::array<unsigned char,3>> colors {};
    while (std::getline(file,line)) {
        std::stringstream line_stream(line);
        double x,y,z;
        int a,b,c;
        line_stream >> x >> y >> z >> a >> b >> c;
        if (flip_yz)
            std::swap(y,z);
        positions.push_back({x,y,z});
        colors.push_back({(unsigned char)a,(unsigned char)b,(unsigned char)c});
        //std::cout << x << "," << y << "," << z << " - " << std::to_string((int)a) << "," << std::to_string((int)b) << ',' << std::to_string((int)c) << "\n";
    }
    return {positions,colors};
}