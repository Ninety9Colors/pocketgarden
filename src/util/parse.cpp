#include "util/parse.hpp"

#include <fstream>
#include <sstream>
#include <iostream>

std::vector<std::pair<std::array<float,3>,std::array<unsigned char,3>>> parse_xyz(std::string filename) {
    std::ifstream file(filename);
    if (!file)
        return {};
    std::string line {};
    std::vector<std::pair<std::array<float,3>,std::array<unsigned char,3>>> result {};
    while (std::getline(file,line)) {
        std::stringstream line_stream(line);
        float x,y,z;
        unsigned char a,b,c;
        line_stream >> x >> y >> z >> a >> b >> c;
        result.push_back({{x,y,z},{a,b,c}});
        // std::cout << x << "," << y << "," << z << " - " << a << "," << b << ',' << c << "\n";
    }
    return result;
}