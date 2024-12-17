#include "oop_map.h"

Window::Window(Map *map, Player *player)
    : map(map), player(player), mul_w(minimap_w / map->w), mul_h(win_h / map->h) {
  buffer.resize(minimap_w * win_h);
}
// so constants can only be initialized with initializer list?
// TODO: compare a) initializer list b) constructor body c) member
// initialization

uint32_t Window::pack_colors(const uint8_t r, const uint8_t g, const uint8_t b,
                             const uint8_t a) {
  return (a << 24) + (r << 16) + (g << 8) + b;
}

void Window::unpack_colors(const uint32_t color, uint8_t &r, uint8_t &g,
                           uint8_t &b, uint8_t &a) {
  a = (color >> 24) & 0xFF;
  r = (color >> 16) & 0xFF;
  g = (color >> 8) & 0xFF;
  b = color & 0xFF;
}

void Window::draw_rectangle(const size_t x, const size_t y, const size_t rec_w,
                            const size_t rec_h, const uint32_t color) {
  for (size_t i = 0; i < rec_w; i++)
    for (size_t j = 0; j < rec_h; j++) {
      size_t cx = x + i;
      size_t cy = y + j;
      assert(cx < minimap_w && cy < win_h);
      buffer[cx + cy * minimap_w] = color;
    }
}

float Window::draw_laser(float angle,
                         const uint32_t color) {
  for (float player_ca = player->a; player_ca < player->a + angle;
       player_ca += PI / 1000) {
    float cx = player->x;
    float cy = player->y;
    for (float l = 0; l < 20; l += 0.05) {
      cx = player->x + l * cos(player_ca);
      cy = player->y + l * sin(player_ca);
      size_t pix_x = int(cx * mul_w);
      size_t pix_y = int(cy * mul_h);
      buffer[pix_x + pix_y * minimap_w] = color;
      if (Window::map->matrix[int(cx) + int(cy) * Window::map->w] != '0') {  
        // why can't we drop Window:: here?
        break;
      }
    }
  }
}

void Window::whole_window_to_ppm() {
  std::ofstream ofs(filename, std::ios::binary);
  ofs << "P6\n" << minimap_w << " " << win_h << "\n" << 255 << "\n";
  for (size_t i = 0; i < minimap_w * win_h; i++) {
    uint8_t r, g, b, a;
    unpack_colors(buffer[i], r, g, b, a);
    ofs << r << g << b;
  }
  ofs.close();
}

Map::Map(size_t w, size_t h) : w(w), h(h), matrix(new char[w * h]) {}

Player::Player(float x, float y, float a, float fov, uint32_t color)
    : x(x), y(y), a(a), fov(fov), color(color) {}

void Player::draw_radar(float fov) {
  float angle = a;
  for (size_t i = 0; i < window->minimap_w; i++) {
    size_t distance = window->draw_laser(angle);
    angle += fov / float(window->minimap_w);
    Player::draw_FPV(distance, i);
  }
}

void Player::draw_FPV(float distance, size_t i) {
  size_t height = window->win_h / distance;
  size_t start = window->win_h / 2 - height / 2;
  size_t end = window->win_h / 2 + height / 2;
  window->draw_rectangle(i, start, 1, end - start, color);
}


  