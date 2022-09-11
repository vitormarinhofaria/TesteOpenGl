//
// Created by vitor on 08/09/2022.
//
#pragma once

#include <cstdint>

#include <GLES3/gl31.h>
#include "glm/glm/glm.hpp"

#include "Types.h"

class Framebuffer {
public:
    Framebuffer(int32_t width, int32_t height);
    void GetPixels(uint8_t* pixelsDest) const;
    size_t GetPixelsBufferSize() const;
    GLuint GetColorAttachment() const;
    void Bind() const;
    Framebuffer();

private:
    GLuint FBO;
    GLuint RBO;
    GLuint ColorBuffer;
public:
    Vec2<int32_t> ScreenSize;
};