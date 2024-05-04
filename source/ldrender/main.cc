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
#include <fstream>
#include <vector>
#include <algorithm>
#include <cmath>

#include "ldraw.hh"

using namespace ldrender;

class Image {
private:
    int width, height;
    std::vector<std::vector<float>> zBuffer; // Z-buffer
    std::vector<uint32_t> pixels;

public:
    Image(int w, int h) : width(w), height(h), pixels(w * h), zBuffer(w, std::vector<float>(h, std::numeric_limits<float>::lowest())) {}

    void setPixel(int x, int y, float z, uint32_t color) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            if (z > zBuffer[x][y]) {
                pixels[y * width + x] = color;
                zBuffer[x][y] = z;
            }
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

void drawLine(Image &img, int x0, int y0, float z0, int x1, int y1, float z1, uint32_t color) {
    bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
    if (steep) {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }

    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
        std::swap(z0, z1); // Swap z values accordingly
    }

    int dx = x1 - x0;
    int dy = std::abs(y1 - y0);
    float dz = (z1 - z0) / static_cast<float>(dx);
    int error = dx / 2;
    int ystep = (y0 < y1) ? 1 : -1;
    int y = y0;
    float z = z0;

    for (int x = x0; x <= x1; x++) {
        if (steep) {
            img.setPixel(y, x, z, color);
        } else {
            img.setPixel(x, y, z, color);
        }
        error -= dy;
        z += dz; // Increment z along the line
        if (error < 0) {
            y += ystep;
            error += dx;
        }
    }
}

void fillTriangle(Image &img, const LDrawTri &tri) {
    // Find bounding box
    int minX = std::min({tri.position1.x, tri.position2.x, tri.position3.x});
    int minY = std::min({tri.position1.y, tri.position2.y, tri.position3.y});
    int maxX = std::max({tri.position1.x, tri.position2.x, tri.position3.x});
    int maxY = std::max({tri.position1.y, tri.position2.y, tri.position3.y});

    // Iterate over pixels in the bounding box
    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            // Barycentric coordinates to determine if point is inside the triangle
            float alpha = ((tri.position2.y - tri.position3.y) * (x - tri.position3.x) + (tri.position3.x - tri.position2.x) * (y - tri.position3.y)) /
                          ((tri.position2.y - tri.position3.y) * (tri.position1.x - tri.position3.x) + (tri.position3.x - tri.position2.x) * (tri.position1.y - tri.position3.y));
            float beta = ((tri.position3.y - tri.position1.y) * (x - tri.position3.x) + (tri.position1.x - tri.position3.x) * (y - tri.position3.y)) /
                         ((tri.position2.y - tri.position3.y) * (tri.position1.x - tri.position3.x) + (tri.position3.x - tri.position2.x) * (tri.position1.y - tri.position3.y));
            float gamma = 1.0f - alpha - beta;

            if (alpha >= 0 && beta >= 0 && gamma >= 0) {
                // Interpolate z value
                float z = alpha * tri.position1.z + beta * tri.position2.z + gamma * tri.position3.z;
                img.setPixel(x, y, z, tri.color->main);
            }
        }
    }
}

void fillQuad(Image &img, const LDrawQuad &quad) {
    fillTriangle(img, LDrawTri(quad.color, quad.position1, quad.position2, quad.position3));
    fillTriangle(img, LDrawTri(quad.color, quad.position3, quad.position4, quad.position1));
}

int main() {
    Image img(1920, 1080);

    LDraw test("ldraw");
    test.loadFromFile("model.ldr");

    for (LDrawLine line : test.buildLines()) {
        drawLine(img, line.position1.x + 50, line.position1.y + 250, line.position1.z, line.position2.x + 50, line.position2.y + 250, line.position2.z, line.color->edge);
    }

    for (LDrawTri tri : test.buildTris()) {
        tri.position1.x += 50;
        tri.position2.x += 50;
        tri.position3.x += 50;

        tri.position1.y += 250;
        tri.position2.y += 250;
        tri.position3.y += 250;
        fillTriangle(img, tri);
    }

    for (LDrawQuad quad : test.buildQuads()) {
        quad.position1.x += 50;
        quad.position2.x += 50;
        quad.position3.x += 50;
        quad.position4.x += 50;

        quad.position1.y += 250;
        quad.position2.y += 250;
        quad.position3.y += 250;
        quad.position4.y += 250;

        fillQuad(img, quad);
    }

    if (img.saveBMP("output.bmp")) {
        std::cout << "File saved successfully!" << std::endl;
    } else {
        std::cerr << "Failed to save file." << std::endl;
    }

    return 0;
}
