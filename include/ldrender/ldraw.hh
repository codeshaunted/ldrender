// codeshaunted - ldrender
// include/ldrender/ldraw.hh
// contains LDraw definitions
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

#ifndef LDRENDER_LDRAW_HH
#define LDRENDER_LDRAW_HH

#include <string>
#include <unordered_map>
#include <vector>

namespace ldrender {

class Vector3 {
    public:
        float x, y, z;
        Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
        Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
};

class TransformMatrix {
    public:
        TransformMatrix();
        TransformMatrix(float x, float y, float z, float a, float b, float c, float d, float e, float f, float g, float h, float i);
        float* operator[](int i);
        TransformMatrix operator*(TransformMatrix& other); // matrix multiply resulting in another TransformMatrix
        Vector3 operator*(Vector3& other); // matrix multiply resulting in another Vector3 (neglecting last row)
    private:
        float data[4][4];
};

class LDraw;

struct LDrawSubFile {
    int color;
    TransformMatrix transform;
    LDraw* model;
};

struct LDrawLine {
    int color;
    Vector3 position1;
    Vector3 position2;
};

struct LDrawTri {
    int color;
    Vector3 position1;
    Vector3 position2;
    Vector3 position3;
};

struct LDrawQuad {
    int color;
    Vector3 position1;
    Vector3 position2;
    Vector3 position3;
    Vector3 position4;
};

struct LDrawOptLine {

};

struct LDrawColor {
    std::string name;
    uint32_t value;
    uint32_t edge;
    // TODO: add support for more color bullshit?
};

class LDraw {
    public:
        LDraw(std::string library_path);
        ~LDraw();
        bool wasLoaded();
        void loadFromData(std::string model_data);
        void loadFromFile(std::string file_path);
        std::vector<LDrawLine> buildLines();
        std::vector<LDrawTri> buildTris();
        std::vector<LDrawQuad> buildQuads();
    private:
        std::string library_path;
        bool was_loaded = false;
        bool is_root_model = false;
        static std::unordered_map<std::string, LDraw*> loaded_models;
        static std::unordered_map<int, LDrawColor*> color_map;
        std::vector<LDrawSubFile> subfiles;
        std::vector<LDrawLine> lines;
        std::vector<LDrawTri> tris;
        std::vector<LDrawQuad> quads;
        std::vector<LDrawOptLine> optlines;
        LDraw();
        void loadLDConfig();
};

} // namespace ldrender

#endif // LDRENDER_LDRAW_HH