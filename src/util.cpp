#include "util.hpp"

std::vector<std::string> split_string(const std::string& data) {
    std::vector<std::string> split;
    int l = 0;
    int r = 0;
    while (r < data.size()) {
        while (r < data.size() && data[r] != ' ' && data[r] != '(')
            r++;
        if (data[r] == '(') {
            int opened_parentheses = 1;
            l = r+1;
            while (r < data.size() && opened_parentheses > 0) {
                r++;
                opened_parentheses += -1*(data[r]==')') + 1*(data[r] == '(');
            }
        }
        split.push_back(data.substr(l, r - l)); // capture word
        while (r < data.size() && (data[r] == ' ' || data[r] == ')'))
            r++; // skip consecutive spaces
        l = r;
    }
    return std::move(split);
}

std::string get_first_word(const std::string& data) {
    int l = 0;
    int r = 0;
    while (r < data.size() && data[r] != ' ' && data[r] != '(') {
        r++;
    }
    if (data[r] == '(') {
        int opened_parentheses = 1;
        l = r+1;
        while (r < data.size() && opened_parentheses > 0) {
            r++;
            opened_parentheses += -1*(data[r]==')') + 1*(data[r] == '(');
        }
    }
    std::string result = data.substr(l, r - l);
    return result;
}

std::string get_without_first_word(const std::string& data) {
    int r = 0;
    while (r < data.size() && data[r] != ' ' && data[r] != '(') {
        r++;
    }
    if (data[r] == '(') {
        int opened_parentheses = 1;
        while (r < data.size() && opened_parentheses > 0) {
            r++;
            opened_parentheses += -1*(data[r]==')') + 1*(data[r] == '(');
        }
    }
    r++;
    std::string result = data.substr(r, r - data.size());
    return result;
}
