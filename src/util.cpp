#include "util.hpp"

std::vector<std::string> split_string(const std::string& raw) {
    std::vector<std::string> split;
    int l = 0;
    int r = 0;
    while (r < raw.size()) {
        while (r < raw.size() && raw[r] != ' ')
            r++;
        split.push_back(raw.substr(l, r - l)); // capture word
        while (r < raw.size() && raw[r] == ' ')
            r++; // skip consecutive spaces
        l = r;
    }
    return std::move(split);
}
