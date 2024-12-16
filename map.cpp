#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>

const double PI = 3.14159265358979323846;

// a: alpha, transparency
uint32_t pack_colors(const uint8_t r, const uint8_t g, const uint8_t b,
                     const uint8_t a = 255) {
  return (a << 24) + (b << 16) + (g << 8) + r; // 0xAABBGGRR
}

void unpack_colors(const uint32_t color, uint8_t &r, uint8_t &g, uint8_t &b,
                   uint8_t &a) {
  r = (color >> 0) & 255;
  g = (color >> 8) & 255;
  b = (color >> 16) & 255;
  a = (color >> 24) & 255;
}

void drop_ppm(std::string filename, const std::vector<uint32_t> &buffer,
              size_t w, size_t h) {
  assert(buffer.size() == w * h);
  std::ofstream ofs(filename, std::ios::binary);
  ofs << "P6\n" << w << " " << h << "\n" << 255 << "\n";
  for (size_t i = 0; i < w * h; i++) {
    uint8_t r, g, b, a;
    unpack_colors(buffer[i], r, g, b, a);
    ofs << r << g << b;
  }
}

void draw_rectangle(std::vector<uint32_t> &buffer, const size_t img_w,
                    const size_t img_h, const size_t x, const size_t y,
                    const size_t rec_w, const size_t rec_h,
                    const uint32_t color) {
  for (size_t i = 0; i < rec_w; i++)
    for (size_t j = 0; j < rec_h; j++) {
      size_t cx = x + i;
      size_t cy = y + j;
      assert(cx < img_w && cy < img_h);
      buffer[cx + cy * img_w] = color;
    }
}

// void draw_line(std::vector<uint32_t>& buffer, const size_t img_w, const
// size_t img_h,const size_t x, const size_t y, const size_t rec_w, const size_t
// rec_h, const uint32_t color)

void draw_line(float player_a, float player_x, float player_y, const size_t mul_w, const size_t mul_h, std::vector<uint32_t> &buffer, const size_t img_w, const char* map, const size_t map_w)
{
    for (float l = 0; l < 20; l += 0.05)
    {
        float cx = player_x + l * cos(player_a); // logic coordinates
        float cy = player_y + l * sin(player_a);
        size_t pix_x = int(cx * mul_w); // pixel coordinates
        size_t pix_y = int(cy * mul_h);
        buffer[pix_x + pix_y * img_w] = pack_colors(255, 255, 255);
        if (map[int(cx) + int(cy) * map_w] == '1')
        {
            break;
        }
    }
}

void draw_vision(float player_a, float player_x, float player_y, const size_t mul_w, const size_t mul_h, std::vector<uint32_t> &buffer, const size_t img_w, const char* map, const size_t map_w,float angle = PI/3)
{
   for(float player_ca = player_a; player_ca < player_a + angle; player_ca += PI / 1000)
  {
    draw_line(player_ca, player_x, player_y, mul_w, mul_h, buffer, img_w, map, map_w);
  }
}
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
  const size_t img_w = 512;
  const size_t img_h = 512; // idk why but the tutorial uses size_t
  std::vector<uint32_t> buffer(img_w * img_h, 255); // initialize
  const size_t mul_w = img_w / map_w;               // multiplier
  const size_t mul_h = img_h / map_h;

  for (size_t i = 0; i < img_w; i++)
    for (size_t j = 0; j < img_h; j++) {
      buffer[i + j * img_w] =
          pack_colors(255 * i / float(img_w), 0, 255 * j / float(img_h));
    }
  for (size_t i = 0; i < map_w; i++)
    for (size_t j = 0; j < map_h; j++) {
      if (map[i + j * map_w] == '0')
        continue;
      draw_rectangle(buffer, img_w, img_h, i * mul_w, j * mul_h, mul_w, mul_h,
                     pack_colors(255, 255, 255));
    }

  float player_x = 3.456;
  float player_y = 2.345;
  float player_a = 1.523; // angle

  draw_rectangle(buffer, img_w, img_h, player_x * mul_w, player_y * mul_h, 5, 5,
                 pack_colors(255, 255, 255)); //player

    draw_vision(player_a,  player_x, player_y, mul_w, mul_h, buffer, img_w, map, map_w);
  

  drop_ppm("./test.ppm", buffer, img_w, img_h); // add path here
}
