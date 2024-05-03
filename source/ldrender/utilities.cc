// codeshaunted - ldrender
// source/ldrender/utilities.cc
// contains utitilies definitions
// Copyright 2024 codeshaunted
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org / licenses / LICENSE - 2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissionsand
// limitations under the License.

#include "utilities.hh"

#include <algorithm>
#include <sstream>

namespace ldrender {

std::string Utilities::toLowercaseString(const std::string& string) {
    std::string result;
    for (char c : string) {
        result += std::tolower(c);
    }

    return result;
}

std::string Utilities::trimString(const std::string& string) {
    auto left = std::find_if_not(string.begin(), string.end(), [](unsigned char c) { return std::isspace(c); });
    auto right = std::find_if_not(string.rbegin(), string.rend(), [](unsigned char c) { return std::isspace(c); }).base();

    if (right <= left) {
        return "";
    }

    return std::string(left, right);
}

std::vector<std::string> Utilities::splitStringByWhitespace(const std::string& string, int max_splits) {
    std::vector<std::string> result;
    std::stringstream stream(string);
    std::string token;

    int i = 1;
    while (stream >> token) {
        result.push_back(token);
        if (i == max_splits - 1) {
            std::stringstream temp_stream;
            temp_stream << stream.rdbuf();
            result.push_back(temp_stream.str());
            return result;
        }
        ++i;
    }

    return result;
}

} // namespace ldrender