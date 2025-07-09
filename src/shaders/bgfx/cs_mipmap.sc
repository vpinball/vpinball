// license:GPLv3+

#include "bgfx_compute.sh"
#include "bgfx_shader.sh"

SAMPLER2D(u_Source, 0);
IMAGE2D_WO(u_Destination, rgba32f, 1);

uniform vec4 u_MipMapOptions;
#define isSRGB      (u_MipMapOptions.x != 0.0)
#define filterMode   u_MipMapOptions.y
#define srcMipLevel  u_MipMapOptions.z

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

float srgbToLinear(float f) {
    if (f <= 0.04045) // 0.03928 ?
        return f * (1.0/12.92);
    else
        return pow(f * (1.0/1.055) + (0.055/1.055), 2.4);
}

vec3 srgbToLinear(vec3 c) {
    return vec3(srgbToLinear(c.r), srgbToLinear(c.g), srgbToLinear(c.b));
}

float linearToSrgb(float f) {
    if (f <= 0.0031308)
        return 12.92 * f;
    else
        return 1.055 * pow(f, 1.0/2.4) - 0.055;
}

vec3 linearToSrgb(vec3 c) {
    return vec3(linearToSrgb(c.r), linearToSrgb(c.g), linearToSrgb(c.b));
}

NUM_THREADS(8, 8, 1)
void main() {
    ivec2 dstCoord = ivec2(gl_GlobalInvocationID.xy);
    CONST(ivec2 dstSize) = imageSize(u_Destination);
    if (any(greaterThanEqual(dstCoord, dstSize)))
        return;
    CONST(int srcMip) = int(srcMipLevel);

    vec4 color = vec4_splat(0.0);

    if (filterMode == 0.0) {
        ivec2 srcCoord = dstCoord * 2;
        color += texelFetch      (u_Source, srcCoord, srcMip             );
        color += texelFetchOffset(u_Source, srcCoord, srcMip, ivec2(0, 1));
        color += texelFetchOffset(u_Source, srcCoord, srcMip, ivec2(1, 0));
        color += texelFetchOffset(u_Source, srcCoord, srcMip, ivec2(1, 1));
        color /= 4.0;
    }
    else if (filterMode == 1.0) {
        ivec2 srcCoord = dstCoord * 2 - ivec2(1, 1);
        for (int dy = 0; dy < 4; ++dy)
            for (int dx = 0; dx < 4; ++dx)
                color += g_kaiserKernel4[dx][dy] * texelFetchOffset(u_Source, srcCoord, srcMip, ivec2(dx, dy));
    }
    else if (filterMode == 2.0) {
        ivec2 srcCoord = dstCoord * 2 - ivec2(2, 2);
        for (int dy = 0; dy < 6; ++dy)
            for (int dx = 0; dx < 6; ++dx)
                color += g_kaiserKernel6[dx][dy] * texelFetchOffset(u_Source, srcCoord, srcMip, ivec2(dx, dy));
    }
    else if (filterMode == 3.0) {
        ivec2 srcCoord = dstCoord * 2 - ivec2(2, 2);
        for (int dy = 0; dy < 6; ++dy)
            for (int dx = 0; dx < 6; ++dx)
                color += g_kaiserKernel6s[dx][dy] * texelFetchOffset(u_Source, srcCoord, srcMip, ivec2(dx, dy));
    }
    
    if (isSRGB)
        color.rgb = linearToSrgb(color.rgb);
    imageStore(u_Destination, dstCoord, color);
}
