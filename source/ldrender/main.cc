// codeshaunted - ldrender
// source/ldrender/main.cc
// contains entry point
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

#include <iostream>

#include "ldraw.hh"

using namespace ldrender;

#include <fstream>
#include <vector>
#include <algorithm> // for std::swap
#include <cmath> // for std::round

class Image {
private:
    int width, height;
    std::vector<uint32_t> pixels;

public:
    Image(int w, int h) : width(w), height(h), pixels(w * h) {}

    void setPixel(int x, int y, uint32_t color) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            pixels[y * width + x] = color;
        }
    }

    bool saveBMP(const std::string &filename) {
        std::ofstream file(filename, std::ios::binary);
        if (!file) return false;

        uint32_t fileSize = 54 + 3 * width * height;
        uint32_t reserved = 0;
        uint32_t headerSize = 54;
        uint32_t pixelDataOffset = 54;
        uint32_t dibHeaderSize = 40;
        uint16_t planes = 1;
        uint16_t bitsPerPixel = 24;
        uint32_t compression = 0;
        uint32_t rawBitmapSize = 3 * width * height;
        int32_t hRes = 2835;
        int32_t vRes = 2835;
        uint32_t numColors = 0;
        uint32_t importantColors = 0;

        file.put('B').put('M');
        file.write((char*)&fileSize, 4);
        file.write((char*)&reserved, 4);
        file.write((char*)&pixelDataOffset, 4);

        file.write((char*)&dibHeaderSize, 4);
        file.write((char*)&width, 4);
        file.write((char*)&height, 4);
        file.write((char*)&planes, 2);
        file.write((char*)&bitsPerPixel, 2);
        file.write((char*)&compression, 4);
        file.write((char*)&rawBitmapSize, 4);
        file.write((char*)&hRes, 4);
        file.write((char*)&vRes, 4);
        file.write((char*)&numColors, 4);
        file.write((char*)&importantColors, 4);

        for (int y = height - 1; y >= 0; y--) {
            for (int x = 0; x < width; x++) {
                uint32_t pixel = pixels[y * width + x];
                uint8_t r = (pixel >> 16) & 0xFF;
                uint8_t g = (pixel >> 8) & 0xFF;
                uint8_t b = pixel & 0xFF;
                file.put(b).put(g).put(r);
            }
        }

        return true;
    }
};

void drawLine(Image &img, int x0, int y0, int x1, int y1, uint32_t color) {
    bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
    if (steep) {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }

    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int dx = x1 - x0;
    int dy = std::abs(y1 - y0);
    int error = dx / 2;
    int ystep = (y0 < y1) ? 1 : -1;
    int y = y0;

    for (int x = x0; x <= x1; x++) {
        if (steep) {
            img.setPixel(y, x, color);
        } else {
            img.setPixel(x, y, color);
        }
        error -= dy;
        if (error < 0) {
            y += ystep;
            error += dx;
        }
    }
}

Vector3 project(const Vector3 &v) {
    // Simple perspective projection
    float factor = 500 / (v.z + 1000); // Assuming the camera is -1000 units in z
    return Vector3(v.x * factor + 400, v.y * factor + 300, v.z);
}

void drawCube(Image &img, const std::vector<Vector3> &vertices, const std::vector<int> &indices, uint32_t color) {
    for (size_t i = 0; i < indices.size(); i += 2) {
        Vector3 v0 = project(vertices[indices[i]]);
        Vector3 v1 = project(vertices[indices[i + 1]]);
        drawLine(img, std::round(v0.x), std::round(v0.y), std::round(v1.x), std::round(v1.y), color);
    }
}

int main() {
    Image img(800, 600);

    std::vector<Vector3> vertices = {
        Vector3(-100, -100, -100),
        Vector3(100, -100, -100),
        Vector3(100, 100, -100),
        Vector3(-100, 100, -100),
        Vector3(-100, -100, 100),
        Vector3(100, -100, 100),
        Vector3(100, 100, 100),
        Vector3(-100, 100, 100)
    };

    std::vector<int> indices = {
        0, 1, 1, 2, 2, 3, 3, 0, // Bottom face
        4, 5, 5, 6, 6, 7, 7, 4, // Top face
        0, 4, 1, 5, 2, 6, 3, 7  // Vertical edges
    };

    //drawCube(img, vertices, indices, 0xFFFFFF);

    LDraw test;
    test.loadFromFile("model.ldr");

    for (LDrawLine line : test.buildLines()) {
        drawLine(img, line.position1.x + 50, line.position1.y + 250, line.position2.x + 50, line.position2.y + 250, 0xFFFFFF);
        //std::cout << "(" << line.position1.x << ", " <<  line.position1.y << ") to (" << line.position2.x << ", " << line.position2.y << ")" << std::endl;
    }

    if (img.saveBMP("output.bmp")) {
        std::cout << "File saved successfully!" << std::endl;
    } else {
        std::cerr << "Failed to save file." << std::endl;
    }

    return 0;
}
