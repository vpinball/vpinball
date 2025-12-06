// license:GPLv3+

#pragma once

// 2D vector
class alignas(8) Vertex2D
{
public:
   float x;
   float y;

   constexpr Vertex2D() {}
   constexpr Vertex2D(const float _x, const float _y) : x(_x), y(_y) {}
};

class Vertex3Ds final
{
public:
    float x, y, z;

    constexpr Vertex3Ds() {}
    constexpr Vertex3Ds(const float _x, const float _y, const float _z) : x(_x), y(_y), z(_z) {}
};

class alignas(16) Vertex4D final
{
public:
    float x, y, z, w;

    constexpr Vertex4D() {}
    constexpr Vertex4D(const float _x, const float _y, const float _z, const float _w) : x(_x), y(_y), z(_z), w(_w) {}
};

class bool4 final
{
public:
    union
    {
        struct {
            bool x, y, z, w;
        };
        unsigned int xyzw;
    };

    constexpr bool4() {}
    constexpr bool4(const bool _x, const bool _y, const bool _z, const bool _w) : x(_x), y(_y), z(_z), w(_w) {}
};

class bool2 final
{
public:
    bool x, y;

    constexpr bool2() {}
    constexpr bool2(const bool _x, const bool _y) : x(_x), y(_y) {}
};
