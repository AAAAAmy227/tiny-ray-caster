#include<iostream>
#include<fstream>
#include<cstdint>
#include<vector>
#include<string>
#include<cassert>
uint32_t pack_colors(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
{
    return (a << 24) + (b << 16) + (g << 8) + r; // 0xAABBGGRR 
}

void unpack_colors(uint32_t color, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a)
{
    r = (color >> 0)&255;
    g = (color >> 8)&255;
    b = (color >> 16)&255;
    a = (color >> 24)&255;
}

void drop_ppm(std::string filename,std::vector<uint32_t> buffer,uint16_t w, uint16_t h)
{
    assert(buffer.size()==w*h);
    std::ofstream ofs(filename,std::ios::binary);
    ofs << "P6\n"<<w<<" "<<h<<"\n"<<255<<"\n";
    for(uint32_t i=0;i<w*h;i++)
    {
        uint8_t r,g,b,a;
        unpack_colors(buffer[i],r,g,b,a);
        ofs<<r<<g<<b;
    }
}
int main()
{
    std::vector<uint32_t> map;
    uint16_t w=512,h=512;
    for(uint16_t i=0;i<w;i++)
    for(uint16_t j=0;j<h;j++)
    {
        map.push_back(pack_colors(255*static_cast<float>(i/w),255*static_cast<float>(j/h),255*static_cast<float>(i*j/(w*h))));
    }
    drop_ppm("test.ppm",map,w,h);
}