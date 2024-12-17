#pragma once
#include <cassert>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

const double PI = 3.14159265358979323846;

uint32_t pack_colors(const uint8_t r, const uint8_t g, const uint8_t b,
                     const uint8_t a = 255);
void unpack_colors(const uint32_t color, uint8_t &r, uint8_t &g, uint8_t &b,
                   uint8_t &a);

class Player;
class Window;
class BaseView;
class MiniMap;
class FPV;

class Window {
public:
  std::vector<uint32_t> buffer;
  std::vector<Player> players;
  const size_t w;
  const size_t h;
  Window(size_t w = 1024, size_t h = 512)
      : w(w), h(h), buffer(w * h), players() {};
  void to_ppm(std::string filename = "./output.ppm");
};

// for rendering
// each view is a subWindow
class BaseView {
public:
  Window &window;
  size_t o_x = 0;
  size_t o_y = 0; // origin
  size_t w = 512;
  size_t h = 512; // pixel
public:
  // size_t scale = 1; //how many pixels in the window correspond to one unit in
  // the view like minimap suppose height/width remains the same
  BaseView(Window &window) : window(window) {};

  void draw_rectangle(const size_t x, const size_t y, const size_t rec_w,
                      const size_t rec_h, const uint32_t color);
  void reset_origin(const size_t x,
                    const size_t y); // treat (x,y) as the new origin
  virtual void render(){}
};

class Player {
public:
  FPV *fpv = nullptr;
  Window &window;
  float x = 3.456; // unit: grid
  float y = 2.345;
  float a = 1.3; // start angle, the angle between the direction and the x-axis
  float fov = PI / 3; // field of view
  uint32_t color = 0xFFFFFFFF;
  Player(Window &window, float x = 3.456, float y = 2.345, float a = 1.3,
         float fov = PI / 3, uint32_t color = 0xFFFFFFFF);
  // void draw_radar(float fov = PI / 3); // draw radar, the lines of sight
  // void draw_FPV(float dis, size_t index); // draw first person view
  //  dis: the distance to the wall
  //  index: the index of the current laser
  //  num_laser: the number of lasers
  //  default: num_laser = window->minimap_w, a laser per pixel
};

class MiniMap : public BaseView {
public:
  const char *matrix;
  size_t grid_w = 16;
  size_t grid_h = 16;
  size_t cell_w;
  size_t cell_h;
  MiniMap(Window &window, const char *matrix, size_t grid_w = 16,
          size_t grid_h = 16)
      : BaseView(window), matrix(matrix), grid_w(grid_w), grid_h(grid_h) {
    cell_w = w / grid_w; // not window's width but the view's width
    cell_h = h / grid_h;
  };
  void render() override;

private:
  void draw_player(Player &player);
  float draw_laser(Player &player, float angle, const uint32_t color);
  void draw_radar(Player &player); // draw radar, the lines of sight
};

class FPV : public BaseView {
public:
  Player *player;
  FPV(Window &window, Player *player) : BaseView(window), player(player) {
    reset_origin(window.w / 2, 0);
  };
  // void render() override;
public:
  void draw_FPV(float dis); // draw first person view
};
