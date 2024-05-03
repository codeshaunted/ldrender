// codeshaunted - ldrender
// include/ldrender/utilities.hh
// contains utilities declarations
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

#ifndef LDRENDER_UTILITIES_HH
#define LDRENDER_UTILITIES_HH

#include <string>
#include <vector>

namespace ldrender {

class Utilities {
    public:
        static std::string toLowercaseString(const std::string& string);
        static std::string trimString(const std::string& string);
        static std::vector<std::string> splitStringByWhitespace(const std::string& string, int max_splits = -1);
};

} // namespace ldrender

#endif // LDRENDER_UTILITIES_HH