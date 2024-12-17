#pragma once
#include <cassert>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

const double PI = 3.14159265358979323846;

class Screen;
class Window;
class ColorUtil;
class Player;
class LocalMiniMap;
class FPV;

class ColorUtil {
public:
  static std::vector<uint32_t> colors;
  static uint32_t pack_colors(const uint8_t r, const uint8_t g, const uint8_t b,
                              const uint8_t a = 255) {
    return (a << 24) + (b << 16) + (g << 8) + r; // 0xAABBGGRR
  }
  static void unpack_colors(const uint32_t color, uint8_t &r, uint8_t &g,
                            uint8_t &b, uint8_t &a) {
    r = (color >> 0) & 255;
    g = (color >> 8) & 255;
    b = (color >> 16) & 255;
    a = (color >> 24) & 255;
  }
};

std::vector<uint32_t> ColorUtil::colors;

class Screen {

public:
  size_t w; // width
  size_t h; // height
  std::vector<Window *> windows;
  std::vector<uint32_t> buffer;

  Screen(size_t w = 1024, size_t h = 512)
      : w(w), h(h), buffer(w * h), windows() {};

  void to_ppm(std::string filename = "./screen.ppm") {
    std::ofstream ofs(filename,
                      std::ios::binary); // binary mode is necessary for PPM
    ofs << "P6\n" << w << " " << h << "\n" << 255 << "\n";
    for (size_t i = 0; i < w * h; i++) {
      uint8_t r, g, b, a;
      ColorUtil::unpack_colors(buffer[i], r, g, b, a);
      ofs << r << g << b;
    }
    ofs.close();
  }
};

class Window {
public:
  Screen *screen;
  size_t o_x; // origin
  size_t o_y;
  size_t w;
  size_t h;
  Window(const Window & window)
      : screen(window.screen), o_x(window.o_x), o_y(window.o_y), w(window.w), h(window.h) {
    screen->windows.push_back(this);
  };
  Window(Screen *screen, size_t o_x = 0, size_t o_y = 0, size_t w = 512,
         size_t h = 512)
      : screen(screen), o_x(o_x), o_y(o_y), w(w), h(h) {
    screen->windows.push_back(this);
  };
  uint32_t &access_virtual_buffer(size_t x, size_t y, bool &err) {
    size_t global_index = (x + o_x) + (y + o_y) * screen->w;
    if (global_index >= screen->w * screen->h) {
      err = true;
      return screen->buffer[0]; // or segmenation fault
    }
    return screen->buffer[global_index];
  }
  void
  draw_rectangle_global(const size_t x, const size_t y, const size_t rec_w,
                        const size_t rec_h,
                        const uint32_t color) { // treat (o_x,o_y) as the origin
    for (size_t i = 0; i < rec_w; i++)
      for (size_t j = 0; j < rec_h; j++) {
        bool err = false;
        size_t cx = x + i;
        size_t cy = y + j;
        // assert(cx < w && cy < h);
        if (cx >= w || cy >= h)
          continue;
        screen->buffer[cx + cy * w] = color;
      }
  }
  void draw_rectangle_in_window(
      const size_t x, const size_t y, const size_t rec_w, const size_t rec_h,
      const uint32_t color) { // treat (o_x,o_y) as the origin
    for (size_t i = 0; i < rec_w; i++)
      for (size_t j = 0; j < rec_h; j++) {
        bool err = false;
        size_t cx = x + i;
        size_t cy = y + j;
        // assert(cx < w && cy < h);
        if (cx >= w || cy >= h)
          continue;
        uint32_t &virtual_buffer = access_virtual_buffer(cx, cy, err);
        if (err)
          continue;
        virtual_buffer = color;
      }
  }
  void reset_origin(const size_t x,
                    const size_t y) { // treat (x,y) as the new origin
    this->o_x = x;                    // won't cause chaos?
    this->o_y = y;
  }

  virtual void render(){}; // render here basically means updating the buffer
};

class Player {
public:
  Screen *screen;
  LocalMiniMap *minimap = nullptr;
  FPV *fpv = nullptr;
  float x = 3.456; // unit: grid
  float y = 2.345;
  float a = 1.3; // start angle, the angle between the direction and the x-axis
  float fov = PI / 3; // field of view
  size_t num_laser = 512;
  uint32_t color = 0xFFFFFFFF;
  Player(Screen *screen, float x = 3.456, float y = 2.345, float a = 1.3,
         float fov = PI / 3, uint32_t color = 0xFFFFFFFF)
      : screen(screen), x(x), y(y), a(a), fov(fov), color(color) {};
  // void draw_radar(float fov = PI / 3); // draw radar, the lines of sight
  // void draw_FPV(float dis, size_t index); // draw first person view
  //  dis: the distance to the wall
  //  index: the index of the current laser
  //  num_laser: the number of lasers
  //  default: num_laser = window->minimap_w, a laser per pixel
};

class LocalMiniMap : public Window {
public:
  Player *player;
  const char *matrix;
  size_t grid_w = 16;
  size_t grid_h = 16;
  size_t cell_w;
  size_t cell_h;
  LocalMiniMap(const Window &window, const char *matrix, size_t grid_w = 16,
               size_t grid_h = 16, Player *player = nullptr)
      : Window(window), matrix(matrix), grid_w(grid_w), grid_h(grid_h),
        player(player) {
    cell_w = w / grid_w; // not window's width but the view's width
    cell_h = h / grid_h;
    init_ground();
    init_wall();
  };

  void init_ground() {
    for (size_t i = 0; i < w; i++)
      for (size_t j = 0; j < h; j++) {
        draw_rectangle_in_window(
            i, j, 1, 1,
            ColorUtil::pack_colors(255 * i / float(w), 0, 255 * j / float(h)));
      }
  }
  void init_wall() {
    for (size_t j = 0; j < grid_h; j++)
      for (size_t i = 0; i < grid_w; i++) {
        if (matrix[i + j * grid_w] == '0')
          continue;
        draw_rectangle_in_window(i * cell_w, j * cell_h, cell_w, cell_h,
                                 ColorUtil::colors[matrix[i + j * grid_w] - '0']);
      }
  }

  void draw_player() {
    size_t px = player->x * cell_w;
    size_t py = player->y * cell_h;
    draw_rectangle_in_window(px, py, 5, 5,
                             ColorUtil::pack_colors(255, 255, 255));
  }

  float shoot_laser(float angle, const uint32_t color, uint32_t& brick_color,  bool draw = true) {
    float l = 0;
    for (; l < 20; l += 0.01) { //l+=0.05 will cause sawtooth
      float cx = player->x + l * cos(angle); // logic coordinates
      float cy = player->y + l * sin(angle);
      size_t pix_x = int(cx * cell_w); // pixel coordinates
      size_t pix_y = int(cy * cell_h);
      if (draw)
        draw_rectangle_in_window(pix_x, pix_y, 1, 1, color);
      if (matrix[int(cx) + int(cy) * grid_w] != '0') {
        //draw_rectangle_in_window(pix_x, pix_y, 1, 1, ColorUtil::pack_colors(0, 255, 0));
        brick_color = ColorUtil::colors[matrix[int(cx) + int(cy) * grid_w] - '0'];
        break;
      }
    }
    return l;
  }

  void draw_radar() {
    float player_ca = player->a;
    uint32_t brick_color;
    for (size_t i = 0; i < (player->num_laser); i++) {
      shoot_laser(player_ca, ColorUtil::pack_colors(255, 255, 255),brick_color, true);
      player_ca += player->fov / player->num_laser;
    }
  }

  void render() override {
    draw_player();
    draw_radar();
  }
};

class FPV : public Window {
public:
  Player *player;
  FPV(Window &window, Player *player)
      : Window(window), player(player) {
  };
  void draw_FPV(size_t i,float dis,uint32_t color = ColorUtil::pack_colors(255, 255, 255)) {

      /*draw_rectangle_in_window(i, float(h) / 2.0 * (1.0 - 1.0 / dis), 1, float(h) / dis,
                               ColorUtil::pack_colors(255, 255, 255));*/
      draw_rectangle_in_window(i, float(h) / 2.0 * (1.0 - 1.0 / dis), 1, float(h) / dis,
                               color);
      //printf h/dis
      //printf("h/dis: %zu\n", static_cast<size_t>(h / dis));
    
  }
  void render() override {
    float player_ca = player->a;
    for (size_t i = 0; i < w; i++) {
      uint32_t brick_color;
      float dis = player->minimap->shoot_laser(player_ca, ColorUtil::pack_colors(255, 255, 255),brick_color, false);
      //printf("dis: %f\n", dis);
      draw_FPV(i,dis,brick_color);
      player_ca += player->fov / w;
    }
  }
};

int main()
{
  
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 255);

  for(int i = 0; i < 256; i++) {
    ColorUtil::colors.push_back(ColorUtil::pack_colors(dis(gen), dis(gen), dis(gen)));
  }
  const char matrix[] = "1111111111111111"
                        "1000000000000001"
                        "1000000000000001"
                        "1000555511000001"
                        "1000100000000001"
                        "1000100077711111"
                        "1000100000000001"
                        "1000100000000001"
                        "1000122222200001"
                        "1000003000900001"
                        "1000003000866111"
                        "1000000000800001"
                        "1000144111800001"
                        "1000100000000001"
                        "1000000000000001"
                        "1111111111111111";


  Screen screen(1024, 1024);

  Window window1(&screen, 0, 0, 512, 512);
  Window window2(&screen, 512, 0, 512, 512);
  Player player(&screen, 2.456, 10.345, -0.6, PI / 3, 0xFFFFFFFF);
  LocalMiniMap minimap(window1, matrix, 16, 16, &player);
  FPV fpv(window2, &player);
  player.minimap = &minimap;
  player.fpv = &fpv;

  minimap.render();
  fpv.render(); 

//add a player with different parameters
  Window window3(&screen, 0, 512, 512, 512);
  Window window4(&screen, 512, 512, 512, 512);
  Player player2(&screen, 12.456, 8.345, 3, PI / 3, 0xFFFFFFFF);
  LocalMiniMap minimap2(window3, matrix, 16, 16, &player2);
  FPV fpv2(window4, &player2);
  player2.minimap = &minimap2;
  player2.fpv = &fpv2;
  minimap2.render();
  fpv2.render();

    screen.to_ppm("./screen.ppm");
  return 0;
}