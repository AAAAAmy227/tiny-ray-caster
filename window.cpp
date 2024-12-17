//write window management

//#pragma once
#include <cassert>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
//random number generator
#include <random>
class Screen;
class Window;
class ColorUtil;

class ColorUtil{
    public:
    static uint32_t pack_colors(const uint8_t r, const uint8_t g, const uint8_t b,
                      const uint8_t a = 255) {
        return (a << 24) + (b << 16) + (g << 8) + r; // 0xAABBGGRR
    }
    static void unpack_colors(const uint32_t color, uint8_t &r, uint8_t &g, uint8_t &b,
                    uint8_t &a) {
        r = (color >> 0) & 255;
        g = (color >> 8) & 255;
        b = (color >> 16) & 255;
        a = (color >> 24) & 255;
    }
};


class Screen{
    
    public:
    size_t w; // width
    size_t h; // height
    std::vector<Window*> windows;
    std::vector<uint32_t> buffer;

    Screen(size_t w = 1024, size_t h = 512)
        : w(w), h(h), buffer(w * h), windows() {};

    void to_ppm(std::string filename = "./screen.ppm") {
        std::ofstream ofs(filename, std::ios::binary); // binary mode is necessary for PPM
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
    Screen* screen;
    size_t o_x; // origin
    size_t o_y;
    size_t w;
    size_t h;
    Window(Screen* screen, size_t o_x = 0, size_t o_y = 0,size_t w = 512, size_t h = 512)
        : screen(screen), o_x(o_x), o_y(o_y), w(w), h(h) {
            screen->windows.push_back(this);
        };
    uint32_t& access_virtual_buffer(size_t x, size_t y,bool& err){
        size_t global_index= (x+o_x) + (y+o_y) * screen->w;
        if(global_index >= screen->w * screen->h)
        {
            err = true;
            return screen->buffer[0]; // or segmenation fault
        }
        return screen->buffer[global_index];
    }

    void draw_rectangle_in_window(const size_t x, const size_t y, const size_t rec_w,
                        const size_t rec_h, const uint32_t color) { //treat (o_x,o_y) as the origin
      for (size_t i = 0; i < rec_w; i++)
        for (size_t j = 0; j < rec_h; j++) {
            bool err = false;
          size_t cx = x + i;
          size_t cy = y + j;
          //assert(cx < w && cy < h);
          if(cx >= w || cy >= h) continue;
          uint32_t& virtual_buffer = access_virtual_buffer(cx, cy,err);
          if(err) continue;
            virtual_buffer = color;
        }
    }
    void reset_origin(const size_t x,
                      const size_t y) { // treat (x,y) as the new origin
      this->o_x = x; // won't cause chaos?
      this->o_y = y;
    }

    virtual void render() = 0; //render here basically means updating the buffer
};


class PureWindow : public Window{
    public:
    uint32_t color;
    PureWindow(Screen* screen, uint32_t color,size_t o_x = 0, size_t o_y = 0,size_t w = 512, size_t h = 512 )
        : Window(screen,o_x,o_y,w,h), color(color) {};
    void render() override {
        for (size_t i = 0; i < w; i++)
            for (size_t j = 0; j < h; j++) {
                draw_rectangle_in_window(i,j,1,1,ColorUtil::pack_colors(255,255,255));
            }
    }
};

class GradientWindow : public Window{
    public:
    GradientWindow(Screen* screen,size_t o_x = 0, size_t o_y = 0,size_t w = 512, size_t h = 512 )
        : Window(screen,o_x,o_y,w,h) {};
    void render() override {
        for (size_t i = 0; i < w; i++)
            for (size_t j = 0; j < h; j++) {
                draw_rectangle_in_window(i,j,1,1,ColorUtil::pack_colors(i%256,0,j%256));
            }
    }
};

class NoiseWindow : public Window{
    public:
    NoiseWindow(Screen* screen,size_t o_x = 0, size_t o_y = 0,size_t w = 512, size_t h = 512 )
        : Window(screen,o_x,o_y,w,h) {};
    void render() override {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        for (size_t i = 0; i < w; i++)
            for (size_t j = 0; j < h; j++) {
                draw_rectangle_in_window(i,j,1,1,ColorUtil::pack_colors(dis(gen),dis(gen),dis(gen)));
            }
    }
};
int main()
{
    Screen screen(512,512);
    PureWindow pure_window(&screen,ColorUtil::pack_colors(255,0,0),0,0,256,256);
    GradientWindow gradient_window(&screen,256,0,256,256);
    NoiseWindow noise_window(&screen,0,256,256,256);
    for(auto window:screen.windows)
    {
        window->render();
    }
    screen.to_ppm();
    return 0;
}

