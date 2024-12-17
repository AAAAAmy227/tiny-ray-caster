#pragma once
#include <cassert>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

const double PI = 3.14159265358979323846;

class Window {
private:
  static std::vector<uint32_t> buffer;
  MiniMap *map;
  Player *player;
  friend class Player;
  friend class Map;

public:
  const size_t win_w = 1024;
  const size_t win_h = 512;
  const size_t mul_w; // unit: pixel/grid
  const size_t mul_h;
  std::string filename;
  Window(MiniMap *map, Player *player);
  uint32_t pack_colors(const uint8_t r, const uint8_t g, const uint8_t b,
                       const uint8_t a = 255);

  void unpack_colors(const uint32_t color, uint8_t &r, uint8_t &g, uint8_t &b,
                     uint8_t &a);

  void draw_rectangle(const size_t x, const size_t y, const size_t rec_w,
                      const size_t rec_h, const uint32_t color);
  //(x,y) is the left top corner of the rectangle

  float draw_laser(float angle,
                   const uint32_t color = 0xFFFFFFFF);
  // laser bounces off walls
  // return the distance to the wall (unit: grid)
  virtual void render();
  // Window(Map* map, Player* player);
  // why can't I write the constructor in .cpp? Why does it cause error?
};

class MiniMap : public Window {
public:
  const size_t w;
  const size_t h;
  const char *matrix;
  void render() override;
public:
  MiniMap(size_t w = 16, size_t h = 16, MiniMap *map = nullptr, Player *player = nullptr) 
    : Window(map, player), w(w), h(h), matrix(new char[w * h]) {}
  ~MiniMap() { delete[] matrix; }
};

class FPV : public Window {
  void render() override;

};


class Player {
public:
  float x = 3.456; // unit: grid
  float y = 2.345;
  float a = 1.3; // start angle, the angle between the direction and the x-axis
  float fov = PI / 3; // field of view
  uint32_t color = 0xFFFFFFFF;
  Player(float x = 3.456, float y = 2.345, float a = 1.3, float fov = PI / 3,
         uint32_t color = 0xFFFFFFFF);
  void draw_radar(float fov = PI / 3); // draw radar, the lines of sight
  void draw_FPV(float dis, size_t index); // draw first person view 
  // dis: the distance to the wall
  // index: the index of the current laser
  // num_laser: the number of lasers
  // default: num_laser = window->minimap_w, a laser per pixel
  
  Window *window;
};