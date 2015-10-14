// mc4ka.cpp: Simplified Minecraft4k
// This version by Anthony C. Hay - http://howtowriteaprogram.blogspot.co.uk/
// Based on work by Salvatore Sanfilippo - https://gist.github.com/4195130
// Which, in turn was based on Markus Persson's Minecraft4k - http://jsdo.it/notch/dB1E
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
                    map[mapindex(x,y,z)] = 0x00FFFFFF & (rand() * rand()); // block colour
            }
        }
    }
    
    return map;
}


// render the next frame into the given 'frame_buf'
void render_blocks(void * private_renderer_data, uint32_t * frame_buf)
{
    // the given void * is our world map
    const uint32_t * map = reinterpret_cast<uint32_t *>(private_renderer_data);
    
    float dx = static_cast<float>(clock() % (CLOCKS_PER_SEC * 10)) / (CLOCKS_PER_SEC * 10);
    float ox = 32.5f + dx * mapdim;
    float oy = 32.5;
    float oz = 32.5;
    
    for (int x = 0; x < width; ++x) {
        const float rotzd = static_cast<float>(x - width / 2) / height;

        for (int y = 0; y < height; ++y) {
            const float rotyd = static_cast<float>(y - height / 2) / height;
            const float rotxd = 1;
            uint32_t col = 0;
            uint32_t br = 255;
            float ddist = 0;
            float closest = 32;

            for (int d = 0; d < 3; d++) {
                const float dimLength = d == 0 ? rotxd : d == 1 ? rotyd : rotzd;
                const float ll = 1 / fabs(dimLength);
                const float xd = rotxd * ll;
                const float yd = rotyd * ll;
                const float zd = rotzd * ll;
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
                        col = tex;
                        ddist = static_cast<float>(255 - static_cast<int>(dist / 32 * 255));
                        br = 255 * (255 - ((d + 2) % 3) * 50) / 255;
                        closest = dist;
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
    std::vector<uint32_t> map(generate_map());
    gem graphics(width, height, scale);
    graphics.run(render_blocks, &map[0]);
}
