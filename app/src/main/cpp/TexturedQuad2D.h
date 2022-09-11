//
// Created by vitor on 08/09/2022.
//

#ifndef TESTEOPENGL_TEXTUREDQUAD2D_H
#define TESTEOPENGL_TEXTUREDQUAD2D_H

#include <cstdint>

#include <GLES3/gl31.h>

#include "Types.h"
#include "Shader.h"

class TexturedQuad2D {
public:
    TexturedQuad2D(bool fullscreen = false);
    void Draw() const;
    ~TexturedQuad2D();
private:
    GLuint VBO, VAO, EBO;
};


#endif //TESTEOPENGL_TEXTUREDQUAD2D_H
