//
// Created by vitor on 08/09/2022.
//

#ifndef TESTEOPENGL_TYPES_H
#define TESTEOPENGL_TYPES_H

#include <cstdint>
#include <GLES3/gl31.h>

template<typename T>
struct Vec2{
    T x;
    T y;
};

template<typename T>
struct Vec3{
    T x;
    T y;
    T z;
};

template<typename T>
struct Vec4{
    T x;
    T y;
    T z;
    T w;
};

struct RefImg {
    uint8_t* Data;
    Vec2<int32_t> Dimensions;
    GLint GL_Format;
};

struct Img {
public:
    Img(RefImg src);
    ~Img();
private:
    RefImg m_Data;
};


#endif //TESTEOPENGL_TYPES_H
