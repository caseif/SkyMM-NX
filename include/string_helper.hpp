#pragma once

#include <algorithm>
#include <string>
#include <vector>

std::string trim(std::string &str) {
    std::string sc = str;
    sc.erase(sc.begin(), std::find_if(sc.begin(), sc.end(), [](int ch) { return !std::isspace(ch); }));
    sc.erase(std::find_if(sc.rbegin(), sc.rend(), [](int ch) { return !std::isspace(ch); }).base(), sc.end());
    return str;
}

std::vector<std::string> split(std::string str, std::string delim) {
    std::vector<std::string> res;
    size_t pos = 0;
    std::string token;

    while ((pos = str.find(delim)) != std::string::npos) {
        token = str.substr(0, pos);
        res.insert(res.end(), token);
        str.erase(0, pos + delim.length());
    }
    res.insert(res.end(), str);

    return res;
}