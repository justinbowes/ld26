//
//  xpl_rand_perlin.c
//  p1
//
//  Created by Justin Bowes on 2013-02-18.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#include "xpl_rand.h"


static int perlin_grad3[12][3] = {
    {1,1,0},
    {-1,1,0},
    {1,-1,0},
    {-1,-1,0},
    {1,0,1},
    {-1,0,1},
    {1,0,-1},
    {-1,0,-1},
    {0,1,1},
    {0,-1,1},
    {0,1,-1},
    {0,-1,-1}
};

static int perlin_perm[] = {151,160,137,91,90,15,
    131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
    190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
    88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
    77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
    102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
    135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
    5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
    223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
    129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
    251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
    49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
    // repeats
    151,160,137,91,90,15,
    131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
    190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
    88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
    77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
    102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
    135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
    5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
    223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
    129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
    251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
    49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

XPLINLINE int ifloor(float x) {
    return (x > 0) ? (int)x : (int)x - 1;
}

XPLINLINE float p_dot(int *g, float x, float y, float z) {
    return g[0] * x + g[1] * y + g[2] * z;
}

XPLINLINE float p_fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float xpl_perlin_noise2(xvec2 vec) {
    return xpl_perlin_noise3(xvec3_extend(vec, 0.f));
}

float xpl_perlin_noise3(xvec3 vec) {
    xivec3 cell = {{ ifloor(vec.x), ifloor(vec.y), ifloor(vec.z) }};
    xvec3 offset = {{ vec.x - cell.x, vec.y - cell.y, vec.z - cell.z }};
    cell = xivec3_set(cell.x & 0xFF, cell.y & 0xFF, cell.z & 0xFF);
    
    int g[] = {
        perlin_perm[cell.x +        perlin_perm[cell.y +        perlin_perm[cell.z]]]       % 12,
        perlin_perm[cell.x +        perlin_perm[cell.y +        perlin_perm[cell.z + 1]]]   % 12,
        perlin_perm[cell.x +        perlin_perm[cell.y + 1 +    perlin_perm[cell.z]]]       % 12,
        perlin_perm[cell.x +        perlin_perm[cell.y + 1 +    perlin_perm[cell.z + 1]]]   % 12,
        perlin_perm[cell.x + 1 +    perlin_perm[cell.y +        perlin_perm[cell.z]]]       % 12,
        perlin_perm[cell.x + 1 +    perlin_perm[cell.y +        perlin_perm[cell.z + 1]]]   % 12,
        perlin_perm[cell.x + 1 +    perlin_perm[cell.y + 1 +    perlin_perm[cell.z]]]       % 12,
        perlin_perm[cell.x + 1 +    perlin_perm[cell.y + 1 +    perlin_perm[cell.z + 1]]]   % 12
    };
    
    float n[] = {
        p_dot(perlin_grad3[g[0]],   offset.x,       offset.y,       offset.z),
        p_dot(perlin_grad3[g[1]],   offset.x - 1,   offset.y,       offset.z),
        p_dot(perlin_grad3[g[2]],   offset.x,       offset.y - 1,   offset.z),
        p_dot(perlin_grad3[g[3]],   offset.x - 1,   offset.y - 1,   offset.z),
        p_dot(perlin_grad3[g[4]],   offset.x,       offset.y,       offset.z - 1),
        p_dot(perlin_grad3[g[5]],   offset.x - 1,   offset.y,       offset.z - 1),
        p_dot(perlin_grad3[g[6]],   offset.x,       offset.y - 1,   offset.z - 1),
        p_dot(perlin_grad3[g[7]],   offset.x - 1,   offset.y - 1,   offset.z - 1)
    };
    
    xvec3 fade = {{
        p_fade(offset.x),
        p_fade(offset.y),
        p_fade(offset.z)
    }};
    
    float nx[] = {
        xmix(n[0], n[1], fade.x),
        xmix(n[4], n[5], fade.x),
        xmix(n[2], n[3], fade.x),
        xmix(n[6], n[7], fade.x)
    };
    
    float nxy[] = {
        xmix(nx[0], nx[2], fade.y),
        xmix(nx[1], nx[3], fade.y)
    };
    
    float nxyz = xmix(nxy[0], nxy[1], fade.z);
    
    return nxyz;
}

