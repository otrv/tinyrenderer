#include "tgaimage.h"
#include <array>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

constexpr int width = 800;
constexpr int height = 800;

constexpr TGAColor white = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green = {0, 255, 0, 255};
constexpr TGAColor red = {0, 0, 255, 255};
constexpr TGAColor blue = {255, 128, 64, 255};
constexpr TGAColor yellow = {0, 200, 255, 255};

void line(int ax, int ay, int bx, int by, TGAImage &framebuffer,
          TGAColor color) {
  bool steep = std::abs(ax - bx) < std::abs(ay - by);
  if (steep) { // If the line is steep we iterate from y
    std::swap(ax, ay);
    std::swap(bx, by);
  }

  if (ax > bx) { // We always draw from left to right regardless of which point
                 // is bigger
    std::swap(ax, bx);
    std::swap(ay, by);
  }

  for (int x = ax; x <= bx; x++) {
    float t = (x - ax) / static_cast<float>(bx - ax); // Normalize
    int y = std::round(ay + (by - ay) * t);

    if (steep) { // If transposed we de-transpose
      framebuffer.set(y, x, color);
    } else {
      framebuffer.set(x, y, color);
    }
  }
}

std::vector<std::string> split(const std::string &str,
                               const std::string &delim) {
  std::vector<std::string> tokens;
  size_t prev = 0, pos = 0;
  while ((pos = str.find(delim, prev)) != std::string::npos) {
    tokens.push_back(str.substr(prev, pos - prev));
    prev = pos + delim.length();
  }
  tokens.push_back(str.substr(prev)); // Add the final part
  return tokens;
}

// Tha vertices are object space, we need to convert them into world space
std::tuple<int, int>
project(std::array<float, 3> v) { // First of all, (x,y) is an orthogonal
                                  // projection of the vector (x,y,z).
  return {(v[0] + 1.) * width /
              2, // Second, since the input models are scaled to have fit in the
                 // [-1,1]^3 world coordinates,
          (v[1] + 1.) * height / 2}; // we want to shift the vector (x,y) and
                                     // then scale it to span the entire screen.
}

struct Obj {
  std::vector<std::array<float, 3>> vertices;
  std::vector<std::array<int, 3>> faces;
};

Obj load_obj(std::string path) {
  Obj obj = {};

  std::ifstream file("./obj/diablo3_pose/diablo3_pose.obj");
  if (!file.is_open()) {
    std::cerr << "Error: Could not open the file." << std::endl;
    return obj;
  }

  std::string file_line;
  while (std::getline(file, file_line)) {
    std::vector<std::string> tokens = split(file_line, " ");

    if (tokens[0] == "v") {
      float x = std::stof(tokens[1]);
      float y = std::stof(tokens[2]);
      float z = std::stof(tokens[3]);
      obj.vertices.push_back({x, y, z});
    } else if (tokens[0] == "f") {
      std::string first = split(tokens[1], "/")[0];
      std::string second = split(tokens[2], "/")[0];
      std::string third = split(tokens[3], "/")[0];

      obj.faces.push_back(
          {std::stoi(first), std::stoi(second), std::stoi(third)});
    } else {
      continue;
    }
  }

  return obj;
}

int main(int argc, char **argv) {
  TGAImage framebuffer(width, height, TGAImage::RGB);

  Obj obj = load_obj("./obj/diablo3_pose/diablo3_pose.obj");

  for (const auto &face : obj.faces) {
    std::array<float, 3> a = obj.vertices[face[0] - 1];
    std::array<float, 3> b = obj.vertices[face[1] - 1];
    std::array<float, 3> c = obj.vertices[face[2] - 1];

    auto [ax, ay] = project(a);
    auto [bx, by] = project(b);
    auto [cx, cy] = project(c);

    line(ax, ay, bx, by, framebuffer, red);
    line(cx, cy, bx, by, framebuffer, red);
    line(cx, cy, ax, ay, framebuffer, red);
    line(ax, ay, cx, cy, framebuffer, red);
  }

  framebuffer.write_tga_file("framebuffer.tga");
  return 0;
}
