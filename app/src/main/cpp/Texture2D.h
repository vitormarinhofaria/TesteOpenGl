//
// Created by vitor on 08/09/2022.
//

#ifndef TESTEOPENGL_TEXTURE2D_H
#define TESTEOPENGL_TEXTURE2D_H

#include <cstdint>

#include <GLES3/gl3.h>

#include "Types.h"

class Texture2D {
public:
    Texture2D(RefImg imgData);
    Texture2D();
    Texture2D(const Texture2D& old) = delete;
    Texture2D(Texture2D&& old) noexcept;
    Texture2D& operator=(Texture2D&& old) noexcept;
    void Bind(uint32_t slot) const;
    ~Texture2D();
private:
    GLuint Id;
};


#endif //TESTEOPENGL_TEXTURE2D_H
