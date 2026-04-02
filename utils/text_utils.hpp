#pragma once

#include <sstream>
#include <string>
#include <vector>

inline std::vector<std::string> split_whitespace(const std::string &s)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream ss(s);

    while (ss >> token)
    {
        tokens.push_back(token);
    }

    return tokens;
}
