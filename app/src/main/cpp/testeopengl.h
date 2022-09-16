//
// Created by vitor on 08/09/2022.
//

#ifndef TESTEOPENGL_TESTEOPENGL_H
#define TESTEOPENGL_TESTEOPENGL_H
#include <jni.h>

#include <vector>

extern "C"{
    #include <libavcodec/avcodec.h>
};

#include "LogUtils.h"
#include "Shader.h"
#include "Texture2D.h"
#include "Types.h"
#include "TexturedQuad2D.h"
#include "Framebuffer.h"

const char *screenVs =
        "#version 310 es\n"
        "precision mediump float;\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec2 aTexCoords;\n"
        "out vec2 TexCoords;\n"
        "void main()\n"
        "{\n"
        "   TexCoords = aTexCoords;\n"
        "   gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
        "}\n";
const char *screenFs =
        "#version 310 es\n"
        "precision mediump float;\n"
        "out vec4 FragColor;\n"
        "in vec2 TexCoords;\n"
        "uniform sampler2D texture0;\n"
        "void main()\n"
        "{\n"
        "   FragColor = texture(texture0, TexCoords);\n"
        "}\n";

struct State {
    Framebuffer framebuffer = Framebuffer(1920, 1080);
    TexturedQuad2D screenQuad = TexturedQuad2D(true);
    Shader screenShader = Shader(screenVs, screenFs);
    Vec2<int> ScreenDimensions;

    std::vector<TexturedQuad2D> quads;
    std::vector<Shader> shaders;
    std::vector<Texture2D> textures;

    int newEntity();
    int entityCount = -1;

    AAssetManager* m_assetManager = nullptr;
};

#endif //TESTEOPENGL_TESTEOPENGL_H
