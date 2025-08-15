// license:GPLv3+

#include "bgfx_compute.sh"

#if defined(FMT_rgba32f)
    IMAGE2D_RO(u_Source, rgba32f, 0);
    IMAGE2D_WO(u_Destination, rgba32f, 1);
#elif defined(FMT_rgba16f)
    IMAGE2D_RO(u_Source, rgba16f, 0);
    IMAGE2D_WO(u_Destination, rgba16f, 1);
#elif defined(FMT_rgba8) || defined(FMT_srgba8)
    IMAGE2D_RO(u_Source, rgba8, 0);
    IMAGE2D_WO(u_Destination, rgba8, 1);
#endif

#if defined(FMT_srgba8)
float srgbToLinear(float f) {
    if (f <= 0.04045) // 0.03928 ?
        return f * (1.0/12.92);
    else
        return pow(f * (1.0/1.055) + (0.055/1.055), 2.4);
}

vec4 srgbToLinear(vec4 c) {
    return vec4(srgbToLinear(c.r), srgbToLinear(c.g), srgbToLinear(c.b), c.a);
}

float linearToSrgb(float f) {
    if (f <= 0.0031308)
        return 12.92 * f;
    else
        return 1.055 * pow(f, 1.0/2.4) - 0.055;
}

vec4 linearToSrgb(vec4 c) {
    return vec4(linearToSrgb(c.r), linearToSrgb(c.g), linearToSrgb(c.b), c.a);
}

#   define texLoad(src, pos)       srgbToLinear(imageLoad(src, pos))
#   define texStore(dst, pos, col) imageStore(dst, pos, linearToSrgb(col))

#else
#   define texLoad(src, pos)       imageLoad(src, pos)
#   define texStore(dst, pos, col) imageStore(dst, pos, col)

#endif


// 0=2x2 Box, 1=4x4 Kaiser, 2=6x6 Kaiser, 3=6x6 smoother Kaiser => My preference goes to 1 or 2 (0 is too sharp, 3 is too blurry)
//uniform vec4         u_MipMapOptions;
//#define filterMode   u_MipMapOptions.x
#define filterMode 1.0

// 4x4 Kaiser normalized convolution kernel
CONST(float g_kaiserKernel4[4][4]) = {
    { 0.000099, 0.004880, 0.004880, 0.000099 },
    { 0.004880, 0.240141, 0.240141, 0.004880 },
    { 0.004880, 0.240141, 0.240141, 0.004880 },
    { 0.000099, 0.004880, 0.004880, 0.000099 }
};

// 6x6 Kaiser normalized convolution kernel
CONST(float g_kaiserKernel6[6][6]) = {
    { 0.000036, 0.000808, 0.002133, 0.002133, 0.000808, 0.000036 },
    { 0.000808, 0.018410, 0.048624, 0.048624, 0.018410, 0.000808 },
    { 0.002133, 0.048624, 0.128424, 0.128424, 0.048624, 0.002133 },
    { 0.002133, 0.048624, 0.128424, 0.128424, 0.048624, 0.002133 },
    { 0.000808, 0.018410, 0.048624, 0.048624, 0.018410, 0.000808 },
    { 0.000036, 0.000808, 0.002133, 0.002133, 0.000808, 0.000036 }
};

// 6x6 Kaiser normalized convolution kernel (smoother/blurrier)
CONST(float g_kaiserKernel6s[6][6]) = {
    { 0.00579380, 0.01302491, 0.01923979, 0.01923979, 0.01302491, 0.00579380 },
    { 0.01302491, 0.02928103, 0.04325256, 0.04325256, 0.02928103, 0.01302491 },
    { 0.01923979, 0.04325256, 0.06389065, 0.06389065, 0.04325256, 0.01923979 },
    { 0.01923979, 0.04325256, 0.06389065, 0.06389065, 0.04325256, 0.01923979 },
    { 0.01302491, 0.02928103, 0.04325256, 0.04325256, 0.02928103, 0.01302491 },
    { 0.00579380, 0.01302491, 0.01923979, 0.01923979, 0.01302491, 0.00579380 }
};

NUM_THREADS(8, 8, 1)
void main() {
    ivec2 dstCoord = ivec2(gl_GlobalInvocationID.xy);
    CONST(ivec2 dstSize) = imageSize(u_Destination);
    if (any(greaterThanEqual(dstCoord, dstSize)))
        return;

    vec4 color = vec4_splat(0.0);

    if (filterMode == 0.0) {
        ivec2 srcCoord = dstCoord * 2;
        color += texLoad(u_Source, srcCoord              );
        color += texLoad(u_Source, srcCoord + ivec2(0, 1));
        color += texLoad(u_Source, srcCoord + ivec2(1, 0));
        color += texLoad(u_Source, srcCoord + ivec2(1, 1));
        color /= 4.0;
    }
    else if (filterMode == 1.0) {
        ivec2 srcCoord = dstCoord * 2 - ivec2(1, 1);
        for (int dy = 0; dy < 4; ++dy)
            for (int dx = 0; dx < 4; ++dx)
                color += g_kaiserKernel4[dx][dy] * texLoad(u_Source, srcCoord + ivec2(dx, dy));
    }
    else if (filterMode == 2.0) {
        ivec2 srcCoord = dstCoord * 2 - ivec2(2, 2);
        for (int dy = 0; dy < 6; ++dy)
            for (int dx = 0; dx < 6; ++dx)
                color += g_kaiserKernel6[dx][dy] * texLoad(u_Source, srcCoord + ivec2(dx, dy));
    }
    else if (filterMode == 3.0) {
        ivec2 srcCoord = dstCoord * 2 - ivec2(2, 2);
        for (int dy = 0; dy < 6; ++dy)
            for (int dx = 0; dx < 6; ++dx)
                color += g_kaiserKernel6s[dx][dy] * texLoad(u_Source, srcCoord + ivec2(dx, dy));
    }
    
    texStore(u_Destination, dstCoord, color);
}
