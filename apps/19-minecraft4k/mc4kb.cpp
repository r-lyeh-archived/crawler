// mc4kb.cpp: Minecraft4k
// This version by Anthony C. Hay - http://howtowriteaprogram.blogspot.co.uk/
// Based on Markus Persson's Minecraft4k - http://jsdo.it/notch/dB1E
// This code is public domain - use at your own risk

#include "gem.h"
#include <cstdlib>


// the block world map is stored in a cube of side mapdim; each map entry
// determines the colour of the corresponding block
const int mappow = 6;
const int mapdim = 1 << mappow;
const int mapmask = mapdim - 1;

// these are the image dimentions used in Minecraft4k
const int width = 428;
const int height = 240;
const int scale = 2;


// return map index of block at given int co-ordinates
inline int mapindex(int x, int y, int z)
{
    return (z & mapmask) << (mappow << 1) | (y & mapmask) << mappow | (x & mapmask);
}

// return map index of block at given float co-ordinates
inline int mapindex(float x, float y, float z)
{
    return mapindex(static_cast<int>(x), static_cast<int>(y), static_cast<int>(z));
}

// return frame buffer index of pixel at given co-ordinates; (0, 0) is top left
inline int framebufindex(int x, int y)
{
    // the OpenGL frame buffer origin is at botom left
    return width * (height - y - 1) + x;
}

// return a random number between 0 and 1
inline float frand()
{
    return static_cast<float>(rand()) / RAND_MAX;
}

// return pixel of given colour
inline uint32_t rgba(uint32_t r, uint32_t g, uint32_t b)
{
    return 0xFF000000 | b << 16 | g << 8 | r;
}


// algorithmicly generate bricks and wood and water and ground and ...
std::vector<uint32_t> generate_textures()
{
    std::vector<uint32_t> texmap(16 * 16 * 3 * 16);
    
    for (int i = 1; i < 16; ++i) {
        float br = 255.0f - rand() % 96;
        for (int y = 0; y < 16 * 3; ++y) {
            for (int x = 0; x < 16; ++x) {
                uint32_t color = 0x966C4A; // ground
                
                if (i == 4) // stone
                    color = 0x7F7F7F;
                
                if (i != 4 || rand() % 3 == 0)
                    br = 255.0f - rand() % 96;
                
                if ((i == 1 && y < (((x * x * 3 + x * 81) >> 2) & 3) + 18))
                    color = 0x6AAA40; // grass
                else if ((i == 1 && y < (((x * x * 3 + x * 81) >> 2) & 3) + 19))
                    br = br * 2 / 3;
                
                if (i == 7) { // wood
                    color = 0x675231;
                    if (x > 0 && x < 15 && ((y > 0 && y < 15) || (y > 32 && y < 47))) {
                        color = 0xBC9862;
                        float xd = x - 7.0f;
                        float yd = (y & 15) - 7.0f;
                        if (xd < 0)
                            xd = 1 - xd;
                        if (yd < 0)
                            yd = 1 - yd;
                        if (yd > xd)
                            xd = yd;
                        
                        br = 196.0f - rand() % 32 + static_cast<int>(xd) % 3 * 32;
                    }
                    else if (rand() % 2 == 0)
                        br = br * (150 - (x & 1) * 100) / 100;
                }
                
                if (i == 5) { // brick
                    color = 0xB53A15;
                    if ((x + (y >> 2) * 4) % 8 == 0 || y % 4 == 0)
                        color = 0xBCAFA5;
                }
                
                if (i == 9) // water
                    color = 0x4040ff;
                
                uint32_t brr = static_cast<uint32_t>(br);
                if (y >= 32)
                    brr /= 2;
                
                if (i == 8) { // leaves
                    color = 0x50D937;
                    if (rand() % 2 == 0) {
                        color = 0;
                        brr = 255;
                    }
                }
                
                uint32_t col = (((color >> 16) & 0xff) * brr / 255) << 16
                             | (((color >> 8) & 0xff) * brr / 255) << 8
                             | (((color) & 0xff) * brr / 255);
                texmap[x + y * 16 + i * 256 * 3] = col;
            }
        }
    }
    
    return texmap;
}


// create the world map
std::vector<uint32_t> generate_map()
{
    std::vector<uint32_t> map(mapdim * mapdim * mapdim);
    
    for (int x = 0; x < mapdim; x++) {
        for (int y = 0; y < mapdim; y++) {
            for (int z = 0; z < mapdim; z++) {
                float yd = (y - 32.5f) * 0.4f;
                float zd = (z - 32.5f) * 0.4f;
                if (frand() > sqrt(sqrt(yd * yd + zd * zd)) - 0.8 || frand() < 0.6)
                    map[mapindex(x,y,z)] = 0; // there won't be a block here
                else
                    map[mapindex(x,y,z)] = rand() % 16; // assign a block type (or none)
            }
        }
    }
    
    return map;
}


struct render_info { // bundle of data used in render_minecraft()
    std::vector<uint32_t> map;
    std::vector<uint32_t> texmap;
    
    render_info()
    : map(generate_map()), texmap(generate_textures())
    {
    }
};


// render the next frame into the given 'frame_buf'
void render_minecraft(void * private_renderer_data, uint32_t * frame_buf)
{
    const render_info * info = reinterpret_cast<render_info *>(private_renderer_data);
    const uint32_t * map = &info->map[0];
    const uint32_t * texmap = &info->texmap[0];
    const float pi = 3.14159265f;
    
    const float dx = static_cast<float>(clock() % (CLOCKS_PER_SEC * 10)) / (CLOCKS_PER_SEC * 10);
    const float xRot = sin(dx * pi * 2) * 0.4f + pi / 2;
    const float yRot = cos(dx * pi * 2) * 0.4f;
    const float yCos = cos(yRot);
    const float ySin = sin(yRot);
    const float xCos = cos(xRot);
    const float xSin = sin(xRot);
    const float ox = 32.5f + dx * 64;
    const float oy = 32.5f;
    const float oz = 32.5f;
    
    for (int x = 0; x < width; ++x) {
        float ___xd = (x - width / 2.0f) / height;

        for (int y = 0; y < height; ++y) {
            const float __yd = (y - height / 2.0f) / height;
            const float __zd = 1;
            
            const float ___zd = __zd * yCos + __yd * ySin;
            const float _yd = __yd * yCos - __zd * ySin;
            
            const float _xd = ___xd * xCos + ___zd * xSin;
            const float _zd = ___zd * xCos - ___xd * xSin;
            
            uint32_t col = 0;
            uint32_t br = 255;
            float ddist = 0;
            float closest = 32;
            
            for (int d = 0; d < 3; ++d) {
                const float dimLength = d == 0 ? _xd : d == 1 ? _yd : _zd;
                float ll = 1 / (dimLength < 0 ? -dimLength : dimLength);
                float xd = (_xd) * ll;
                float yd = (_yd) * ll;
                float zd = (_zd) * ll;
                float initial;
                switch (d) {
                    case 0: initial = ox - static_cast<int>(ox); break;
                    case 1: initial = oy - static_cast<int>(oy); break;
                    default: initial = oz - static_cast<int>(oz); break;
                }
                if (dimLength > 0)
                    initial = 1 - initial;
                float dist = ll * initial;
                float xp = ox + xd * initial;
                float yp = oy + yd * initial;
                float zp = oz + zd * initial;
                if (dimLength < 0)
                    --(d == 0 ? xp : d == 1 ? yp : zp);
                
                while (dist < closest) {
                    uint32_t tex = map[mapindex(xp, yp, zp)];
                    
                    if (tex > 0) {
                        uint32_t u = (uint32_t)((xp + zp) * 16) & 15;
                        uint32_t v = ((uint32_t)(yp * 16) & 15) + 16;
                        if (d == 1) {
                            u = (uint32_t)(xp * 16) & 15;
                            v = ((uint32_t)(zp * 16) & 15);
                            if (yd < 0)
                                v += 32;
                        }
                        
                        uint32_t cc = texmap[u + v * 16 + tex * 256 * 3];
                        if (cc > 0) {
                            col = cc;
                            ddist = static_cast<float>(255 - static_cast<int>(dist / 32 * 255));
                            br = 255 * (255 - ((d + 2) % 3) * 50) / 255;
                            closest = dist;
                        }
                    }
                    xp += xd;
                    yp += yd;
                    zp += zd;
                    dist += ll;
                }
            }
            
            const uint32_t r = static_cast<uint32_t>(((col >> 16) & 0xff) * br * ddist / (255 * 255));
            const uint32_t g = static_cast<uint32_t>(((col >> 8) & 0xff) * br * ddist / (255 * 255));
            const uint32_t b = static_cast<uint32_t>(((col) & 0xff) * br * ddist / (255 * 255));
            
            frame_buf[framebufindex(x, y)] = rgba(r, g, b);
        }
    }
}


int main()
{
    render_info info;
    gem graphics(width, height, scale);
    graphics.run(render_minecraft, &info);
}
