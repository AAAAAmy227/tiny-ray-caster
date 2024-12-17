#include "poly-trc.h"
#include <iterator>

void Window::to_ppm(std::string filename) {
  std::ofstream
        ofs(filename, std::ios::binary); // binary mode is necessary for PPM
    ofs << "P6\n" << w << " " << h << "\n" << 255 << "\n";
    for (size_t i = 0; i < w * h; i++) {
        uint8_t r, g, b, a;
        unpack_colors(buffer[i], r, g, b, a);
        ofs << r << g << b;
    }
    ofs.close();
}

uint32_t pack_colors(const uint8_t r, const uint8_t g, const uint8_t b,
                     const uint8_t a) {
  return (a << 24) + (b << 16) + (g << 8) + r; // 0xAABBGGRR
                     }

void unpack_colors(const uint32_t color, uint8_t &r, uint8_t &g, uint8_t &b,
                   uint8_t &a) {
  r = (color >> 0) & 255;
  g = (color >> 8) & 255;
  b = (color >> 16) & 255;
  a = (color >> 24) & 255;
}

void BaseView::draw_rectangle(const size_t x, const size_t y, const size_t rec_w,
                    const size_t rec_h, const uint32_t color) {
  for (size_t i = 0; i < rec_w; i++)
    for (size_t j = 0; j < rec_h; j++) {
      size_t cx = x + i;
      size_t cy = y + j;
      //assert(cx < w && cy < h);
      if(cx >= w || cy >= h) continue;
      window.buffer[cx + cy * w] = color;
    }
}

void BaseView::reset_origin(const size_t x, const size_t y) {
  this->o_x = x; //won't cause chaos?
  this->o_y = y;
}

void MiniMap::render() {
  for (size_t i = 0; i < grid_w; i++)
    for (size_t j = 0; j < grid_h; j++) {
      if (matrix[i + j * grid_w] == '0')
        continue;
      draw_rectangle(i * cell_w, j * cell_h, cell_w,
                     cell_h, pack_colors(255, 255, 255));
    }
  for (auto &player : window.players) {
    draw_player(player);
    draw_radar(player);
  }
}

void MiniMap::draw_player(Player &player) {
  size_t px = player.x * cell_w;
  size_t py = player.y * cell_h;
  draw_rectangle(px, py, 5, 5, pack_colors(255, 255, 255));
}

float MiniMap::draw_laser(Player &player, float angle, const uint32_t color) {
  float l =0;
  for (; l < 20; l += 0.05) {
    float cx = player.x + l * cos(angle); // logic coordinates
    float cy = player.y + l * sin(angle);
    size_t pix_x = int(cx * cell_w); // pixel coordinates
    size_t pix_y = int(cy * cell_h);
    window.buffer[pix_x + pix_y * w] = color;
    if (matrix[int(cx) + int(cy) * grid_w] == '1') {
      break;
    }
  }
  return l;
}

void MiniMap::draw_radar(Player &player) {
    float player_ca = player.a;
    for(size_t i=0;i<player.fpv->w;i++) {

        float dis = draw_laser(player, player_ca, pack_colors(255, 255, 255));
        player.fpv->draw_FPV(dis);
        player_ca += player.fov / player.fpv->w;

    }
}

void FPV::draw_FPV(float dis) {
  for (size_t i = o_x; i < o_x+w; i++) {
    draw_rectangle(i,h/2*(1-1/dis),1,h/dis,pack_colors(255,255,0));
  }
}

Player::Player(Window &window, float x, float y, float a, float fov,
               uint32_t color)
    : window(window),x(x), y(y), a(a), fov(fov), color(color) {}

int main() {
  const size_t map_w = 16;
  const size_t map_h = 16;
  const char map[] = "1111111111111111"
                     "1000000000000001"
                     "1000000000000001"
                     "1000111111000001"
                     "1000100000000001"
                     "1000100011111111"
                     "1000100000000001"
                     "1000100000000001"
                     "1000111111100001"
                     "1000001000100001"
                     "1000001000111111"
                     "1000000000100001"
                     "1000111111100001"
                     "1000100000000001"
                     "1000000000000001"
                     "1111111111111111";

  // 1: wall
  assert(sizeof(map) == map_w * map_h + 1);
  Window window;
  MiniMap minimap(window, map);
  Player player1(window);
  Player player2(window);
  FPV fpv1(window, &player1);
  FPV fpv2(window, &player2);
  player1.fpv = &fpv1;
  player2.fpv = &fpv2;
  window.players.push_back(player1);
  window.players.push_back(player2);
  minimap.render();
  window.to_ppm();
}