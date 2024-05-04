// codeshaunted - ldrender
// source/ldrender/ldraw.cc
// contains LDraw declarations
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

#include <iostream> // GET RID OF THIS
#include <fstream>
#include <sstream>

#include "ldraw.hh"
#include "utilities.hh"

namespace ldrender {

TransformMatrix::TransformMatrix() {
    // initialize to identity matrix
    for (size_t i = 0; i < 4; ++i) {
        this->data[i][0] = 0.0f;
        this->data[i][1] = 0.0f;
        this->data[i][2] = 0.0f;
        this->data[i][3] = 0.0f;
        this->data[i][i] = 1.0f;
    }
}

TransformMatrix::TransformMatrix(float x, float y, float z, float a, float b, float c, float d, float e, float f, float g, float h, float i) {
    this->data[0][0] = a;
    this->data[0][1] = b;
    this->data[0][2] = c;
    this->data[0][3] = x;
    
    this->data[1][0] = d;
    this->data[1][1] = e;
    this->data[1][2] = f;
    this->data[1][3] = y;

    this->data[2][0] = g;
    this->data[2][1] = h;
    this->data[2][2] = i;
    this->data[2][3] = z;

    this->data[3][0] = 0.0f;
    this->data[3][1] = 0.0f;
    this->data[3][2] = 0.0f;
    this->data[3][3] = 1.0f;
}

float* TransformMatrix::operator[](int i) {
    return data[i];
}

TransformMatrix TransformMatrix::operator*(TransformMatrix& other) {
    TransformMatrix result;

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.data[i][j] = 0;
            for (int k = 0; k < 4; ++k) {
                result.data[i][j] += this->data[i][k] * other.data[k][j];
            }
        }
    }

    return result;
}

Vector3 TransformMatrix::operator*(Vector3& other) {
    Vector3 result;

    result.x = (this->data[0][0] * other.x) + (this->data[0][1] * other.y) + (this->data[0][2] * other.z) + this->data[0][3];
    result.y = (this->data[1][0] * other.x) + (this->data[1][1] * other.y) + (this->data[1][2] * other.z) + this->data[1][3];
    result.z = (this->data[2][0] * other.x) + (this->data[2][1] * other.y) + (this->data[2][2] * other.z) + this->data[2][3];

    return result;
}

LDraw::LDraw(std::string library_path) {
    this->is_root_model = true;
    this->library_path = library_path;

    this->loadLDConfig();
}

LDraw::~LDraw() {
    if (this->is_root_model) {
        for (auto& model : LDraw::loaded_models) {
            delete model.second;
        }
    }
}

bool LDraw::wasLoaded() {
    return this->was_loaded;
}

void LDraw::loadFromData(std::string model_data) {
    this->was_loaded = true;

    std::stringstream model_data_stream;
    model_data_stream << model_data;
    std::string line_data;
    while (std::getline(model_data_stream, line_data)) {
        line_data = Utilities::trimString(line_data);
        if (line_data.empty()) {
            continue;
        }
        char line_type = line_data.at(0);
        std::vector<std::string> tokens = Utilities::splitStringByWhitespace(line_data);

        bool file_directive = false;
        switch (line_type) {
            case '0': {
                if (tokens.size() > 2) {
                    if (tokens[1] == "FILE") {
                        if (this->is_root_model && this->subfiles.empty() && this->lines.empty() && this->tris.empty() && this->quads.empty() && this->optlines.empty()) {
                            break; // FILE directive for main model, ignore
                        }
                        std::string file_name = Utilities::toLowercaseString(Utilities::trimString(line_data.substr(line_data.find("FILE") + 4)));
                        LDraw* file = nullptr;
                        if (!LDraw::loaded_models.contains(file_name)) {
                            file = new LDraw();
                            LDraw::loaded_models.insert({file_name, file});
                        } else {
                            file = LDraw::loaded_models.at(file_name);
                        }
                        std::stringstream temp_stream;
                        temp_stream << model_data_stream.rdbuf();
                        file->loadFromData(temp_stream.str());
                        file_directive = true;
                    }
                }
                break;
            }    
            case '1': {
                if (tokens.size() > 14) {
                    std::string subfile_name = Utilities::toLowercaseString(Utilities::trimString(Utilities::splitStringByWhitespace(line_data, 15).back()));
                    LDraw* subfile_model = nullptr;
                    if (LDraw::loaded_models.contains(subfile_name)) {
                        subfile_model = LDraw::loaded_models.at(subfile_name);
                    } else {
                        subfile_model = new LDraw();
                        LDraw::loaded_models.insert({subfile_name, subfile_model});
                    }
                    LDrawSubFile subfile(
                        std::stoi(tokens[1]), // color
                        TransformMatrix(
                            std::stof(tokens[2]), // x
                            std::stof(tokens[3]), // y
                            std::stof(tokens[4]), // z
                            std::stof(tokens[5]), // a
                            std::stof(tokens[6]), // b
                            std::stof(tokens[7]), // c
                            std::stof(tokens[8]), // d
                            std::stof(tokens[9]), // e
                            std::stof(tokens[10]), // f
                            std::stof(tokens[11]), // g
                            std::stof(tokens[12]), // h
                            std::stof(tokens[13]) // i
                        ),
                        subfile_model
                    );
                    this->subfiles.push_back(subfile);
                    break;
                }
            }
            case '2': {
                if (tokens.size() == 8) {
                    LDrawLine line(
                        std::stoi(tokens[1]), // color
                        Vector3(
                            std::stof(tokens[2]), // x1
                            std::stof(tokens[3]), // y1
                            std::stof(tokens[4]) // z1
                        ),
                        Vector3(
                            std::stof(tokens[5]), // x2
                            std::stof(tokens[6]), // y2
                            std::stof(tokens[7]) // z2
                        )
                    );
                    this->lines.push_back(line);
                    break;
                }
            }
            case '3': {
                if (tokens.size() == 11) {
                    LDrawTri tri(
                        std::stoi(tokens[1]), // color
                        Vector3(
                            std::stof(tokens[2]), // x1
                            std::stof(tokens[3]), // y1
                            std::stof(tokens[4]) // z1
                        ),
                        Vector3(
                            std::stof(tokens[5]), // x2
                            std::stof(tokens[6]), // y2
                            std::stof(tokens[7]) // z2
                        ),
                        Vector3(
                            std::stof(tokens[8]), // x3
                            std::stof(tokens[9]), // y3
                            std::stof(tokens[10]) // z3
                        )
                    );
                    this->tris.push_back(tri);
                    break;
                }
            }
            case '4': {
                if (tokens.size() == 14) {
                    LDrawQuad quad(
                        std::stoi(tokens[1]), // color
                        Vector3(
                            std::stof(tokens[2]), // x1
                            std::stof(tokens[3]), // y1
                            std::stof(tokens[4]) // z1
                        ),
                        Vector3(
                            std::stof(tokens[5]), // x2
                            std::stof(tokens[6]), // y2
                            std::stof(tokens[7]) // z2
                        ),
                        Vector3(
                            std::stof(tokens[8]), // x3
                            std::stof(tokens[9]), // y3
                            std::stof(tokens[10]) // z3
                        ),
                        Vector3(
                            std::stof(tokens[11]), // x4
                            std::stof(tokens[12]), // y4
                            std::stof(tokens[13]) // z4
                        )
                    );
                    this->quads.push_back(quad);
                    break;
                }
            }
            default:
                break;
        }

        if (file_directive) {
            return;
        }
    }
}

void LDraw::loadFromFile(std::string file_path) {  
    this->was_loaded = true;

    if (this->is_root_model) {
        LDraw::loaded_models.insert({Utilities::toLowercaseString(file_path), this});
    }

    std::ifstream file(file_path);
    if (!file) {
        file = std::ifstream(this->library_path + "/parts/" + file_path);
    }
    if (!file) {
        file = std::ifstream(this->library_path + "/p/" + file_path);
    }
    if (!file) {
        return; // unable to find file, TODO: do something here?
    }

    std::stringstream model_data;
    
    model_data << file.rdbuf();
    file.close();

    this->loadFromData(model_data.str());

    for (auto& model : LDraw::loaded_models) {
        if (!model.second->wasLoaded()) {
            model.second->loadFromFile(model.first);
        }
    }
}

std::vector<LDrawLine> LDraw::buildLines() {
    std::vector<LDrawLine> output_lines;
    for (LDrawLine line : this->lines) {
        output_lines.push_back(line);
    }

    for (LDrawSubFile subfile : subfiles) {
        std::vector<LDrawLine> subfile_lines = subfile.model->buildLines();
        for (LDrawLine subfile_line : subfile_lines) {
            // TODO: ADD SOME SHIT ABOUT COLOR HERE
            subfile_line.position1 = subfile.transform * subfile_line.position1;
            subfile_line.position2 = subfile.transform * subfile_line.position2;

            output_lines.push_back(subfile_line);
        }
    }

    return output_lines;
}

std::vector<LDrawTri> LDraw::buildTris() {
    std::vector<LDrawTri> output_tris;
    for (LDrawTri tri : this->tris) {
        output_tris.push_back(tri);
    }

    for (LDrawSubFile subfile : subfiles) {
        std::vector<LDrawTri> subfile_tris = subfile.model->buildTris();
        for (LDrawTri subfile_tri : subfile_tris) {
            // TODO: ADD SOME SHIT ABOUT COLOR HERE
            subfile_tri.position1 = subfile.transform * subfile_tri.position1;
            subfile_tri.position2 = subfile.transform * subfile_tri.position2;
            subfile_tri.position3 = subfile.transform * subfile_tri.position3;

            output_tris.push_back(subfile_tri);
        }
    }

    return output_tris;
}

std::vector<LDrawQuad> LDraw::buildQuads() {
    std::vector<LDrawQuad> output_quads;
    for (LDrawQuad quad : this->quads) {
        output_quads.push_back(quad);
    }

    for (LDrawSubFile subfile : subfiles) {
        std::vector<LDrawQuad> subfile_quads = subfile.model->buildQuads();
        for (LDrawQuad subfile_quad : subfile_quads) {
            // TODO: ADD SOME SHIT ABOUT COLOR HERE
            subfile_quad.position1 = subfile.transform * subfile_quad.position1;
            subfile_quad.position2 = subfile.transform * subfile_quad.position2;
            subfile_quad.position3 = subfile.transform * subfile_quad.position3;
            subfile_quad.position4 = subfile.transform * subfile_quad.position4;

            output_quads.push_back(subfile_quad);
        }
    }

    return output_quads;
}

std::unordered_map<std::string, LDraw*> LDraw::loaded_models;

std::unordered_map<int, LDrawColor*> LDraw::color_map;

LDraw::LDraw() {

}

void LDraw::loadLDConfig() {
    std::ifstream config_file(this->library_path + "/LDConfig.ldr");
    std::stringstream config_data;
    
    config_data << config_file.rdbuf();
    config_file.close();

    std::string config_line;
    while (std::getline(config_data, config_line)) {
        std::vector<std::string> tokens = Utilities::splitStringByWhitespace(config_line);

        if (tokens.size() > 3) {
            if (tokens[0][0] == '0' && tokens[1] == "!COLOUR") {
                LDrawColor* new_color = new LDrawColor();
                new_color->name = tokens[2];
                int code = 0;

                for (size_t i = 3; i < tokens.size(); ++i) {
                    if (!(i + 1 < tokens.size())) {
                        break;
                    }
                    if (tokens[i] == "CODE") {
                        code = std::stoi(tokens[i + 1]);
                    }
                    if (tokens[i] == "VALUE") {
                        new_color->value = std::stoul(tokens[i + 1].substr(1), nullptr, 16);
                    }
                    if (tokens[i] == "EDGE") {
                        new_color->edge = std::stoul(tokens[i + 1].substr(1), nullptr, 16);
                    }
                }

                this->color_map.insert({code, new_color});
            }
        }
    }
}

} // namespace ldrender