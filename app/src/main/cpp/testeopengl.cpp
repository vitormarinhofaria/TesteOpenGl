#include <cstdio>
#include <ctime>
#include <vector>
#include <string>
#include <mutex>
#include <queue>
#include <functional>
#include <filesystem>

#include <jni.h>

#include <android/log.h>
#include <android/bitmap.h>
#include <android/asset_manager.h>

#include <GLES3/gl31.h>
#include <unistd.h>
#include "glm/glm/glm.hpp"
#include "glm/glm/ext.hpp"
#include "tinywav/tinywav.h"

#include "stbi_image_write.h"
#include "testeopengl.h"

const char *frag = "#version 310 es\n"
                   "precision mediump float;\n"
                   "out vec4 FragColor;\n"
                   "in vec2 TexCoord;\n"
                   "uniform sampler2D textureSamp;\n"
                   "void main()\n"
                   "{\n"
                   "    FragColor = texture(textureSamp, TexCoord);\n"
                   "}\n";
const char *vert = "#version 310 es\n"
                   "precision mediump float;\n"
                   "layout (location = 0) in vec3 aPos;\n"
                   "layout (location = 1) in vec2 aTexCoord;\n"
                   "out vec2 TexCoord;\n"
                   "uniform mat4 transform;\n"
                   "void main()\n"
                   "{\n"
                   "    gl_Position = transform * vec4(aPos, 1.0);\n"
                   "    TexCoord = aTexCoord;\n"
                   "}\n";

std::string BasePath = "";

double getNow() {
    timespec ts{};
    clock_gettime(CLOCK_REALTIME, &ts);
    return 1000.0 * ts.tv_sec + (double) ts.tv_nsec / 1e6;
}

double lastTime = 0;
State *G_State = nullptr;

glm::mat4 camera;
glm::mat4 view;
float ratio = 1920.0f / 1080.0f;
glm::mat4 proj = glm::ortho(0.0f, 1920.0f, 1080.0f, 0.0f, -1000.0f, 1000.0f);;
glm::mat4 model;

jobject Fragment;
jclass FragmentClass;
jmethodID Fragment_setFrameState;

extern "C"
JNIEXPORT void JNICALL
Java_com_example_testeopengl_NativeRenderer_onSurfaceChangedNative(JNIEnv *env, jclass clazz,
                                                                   jobject gl, jint width,
                                                                   jint height) {
    // TODO: implement onSurfaceChangedNative()
    LOG("Called on Surface Changed: w %d h %d", width, height);
    G_State->ScreenDimensions.x = width;
    G_State->ScreenDimensions.y = height;
    //proj = glm::ortho(ratio * -1.0f, ratio, ratio * -1.0f, ratio, -1000.0f, 1000.0f);
    //G_State->framebuffer = Framebuffer(1920, 1080);
}

int quadEnt = -1;

std::vector<int16_t> musicBytes = {};

static glm::vec3 modelScale = {1.0f, 1.0f, 1.0f};

TinyWav tw;
static const char *musicFile = "gen.wav";
extern "C"
JNIEXPORT void JNICALL
Java_com_example_testeopengl_NativeRenderer_onSurfaceCreatedNative(JNIEnv *env, jclass clazz,
                                                                   jobject gl, jobject config) {
    delete G_State;
    G_State = new State();
    quadEnt = G_State->newEntity();

    uint8_t dumbImg[] = {0, 0, 255, 255, 0, 0, 0, 255, 0, 0, 0, 0};
    G_State->textures[quadEnt] = std::move(
            Texture2D(RefImg{.Data = dumbImg, .Dimensions = {.x = 4, .y = 1}, .GL_Format= GL_RGB}));
    G_State->shaders[quadEnt] = std::move(Shader(vert, frag));

    view = glm::lookAt(glm::vec3{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
    model = glm::scale(glm::mat4(1.0f), modelScale);

    tinywav_open_read(&tw, musicFile, TW_INTERLEAVED, false);
    int16_t samples[2 * 44100];

    while (auto read = tinywav_read_f(&tw, samples, 441)) {
        for (auto i = 0; i < read; i++) {
            musicBytes.push_back(samples[i]);
        }
    }
    tinywav_close_read(&tw);
    lastTime = getNow();
}

void saveFrame(int frameNum, JNIEnv* env) {
    auto bufferSize = G_State->framebuffer.GetPixelsBufferSize();
    auto *buffer = (unsigned char *) malloc(bufferSize);
    G_State->framebuffer.GetPixels(buffer);
    stbi_flip_vertically_on_write(true);

    std::string fileName;
    fileName.append("Frames/img");
    fileName.append(std::to_string(frameNum));
    fileName.append(".png");

    auto sr = stbi_write_png(fileName.c_str(), G_State->framebuffer.ScreenSize.x,
                   G_State->framebuffer.ScreenSize.y, 3,
                   buffer, G_State->framebuffer.ScreenSize.x * 3);
    free(buffer);
    env->CallVoidMethod(Fragment, Fragment_setFrameState, (jint)frameNum);
    LOG("Frame %d sr is %d %s", frameNum, sr, fileName.c_str());
}

std::mutex drawMutex{};
std::queue<std::function<void(JNIEnv *)>> execQ;

float r, g, b = 0;
size_t musicPtr = 0;
extern "C"
JNIEXPORT void JNICALL
Java_com_example_testeopengl_NativeRenderer_onDrawFrameNative(JNIEnv *env, jclass clazz,
                                                              jobject gl) {
    if (!drawMutex.try_lock()) {
        return;
    }
    while (!execQ.empty()) {
        auto d = execQ.front();
        d(env);
        execQ.pop();
    }

    double now = getNow();
    double delta = now - lastTime;

    static bool reversing = false;
    float colorDelta = 0.002f;
    if (r >= 1.0 && !reversing) {
        reversing = true;
    }
    if (reversing && r <= 0.0f) {
        reversing = false;
    }
    if (reversing)
        colorDelta = -colorDelta;

    r += (colorDelta * static_cast<float>(delta));

    glViewport(0, 0, 1920, 1080);
    G_State->framebuffer.Bind();

    glClearColor(r, 0.5, 0.2, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    if (musicPtr == musicBytes.size()) { musicPtr = 0; }
    glm::vec3 modScale = {1.0f, 1.0f, 1.0f};
    if (musicPtr < musicBytes.size()) {
        uint64_t sum = 0;
        for(auto i = 0; i < (44100/24); i++){
            auto nval = abs(musicBytes[musicPtr]);
            sum += nval;
            musicPtr += 1;
        }
        auto nval = sum / (44100/24);
        modScale.x = (float) nval * 0.0001f;
        modScale.y = (float) nval * 0.0001f;
        modScale.z = 1.0f;
        //musicPtr += 1;
    }

    glm::mat4 modelTrans = glm::mat4(1.0f);
    modelTrans = glm::translate(modelTrans, glm::vec3{0.0, 0.0f, 0.0f});
    modelTrans = glm::scale(modelTrans, modScale);

    //glm::mat4 mvp = proj * view * modelTrans;
    glm::mat4 mvp = proj * view * modelTrans;

    G_State->shaders[quadEnt].Bind();
    G_State->shaders[quadEnt].SetUniform("transform", modelTrans);
    G_State->textures[quadEnt].Bind(0);
    G_State->quads[quadEnt].Draw();

    glViewport(0, 0, G_State->ScreenDimensions.x, G_State->ScreenDimensions.y);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, G_State->framebuffer.GetColorAttachment());
    G_State->screenShader.Bind();
    G_State->screenQuad.Draw();

    lastTime = getNow();
    static uint64_t frameCount = 0;
    static bool taken = false;
    if (frameCount < 1104) {
        saveFrame(frameCount, env);
    }
    if (frameCount == 1105 || frameCount == 1) {
        auto saveMp4_Id = env->GetStaticMethodID(clazz, "saveMp4", "()V");
        env->CallStaticVoidMethod(clazz, saveMp4_Id);
    }
    frameCount++;
    drawMutex.unlock();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_testeopengl_NativeRenderer_setTexture(JNIEnv *env, jclass clazz, jobject bitmap) {
    // TODO: implement setTexture()
    drawMutex.lock();
    AndroidBitmapInfo info{};
    AndroidBitmap_getInfo(env, bitmap, &info);
    void *buffer;
    AndroidBitmap_lockPixels(env, bitmap, &buffer);
    std::vector<GLbyte> dest{};
    auto bufferSize = (info.height * info.width) * (sizeof(GLbyte) * 4);
    dest.resize(bufferSize);
    std::memcpy(dest.data(), buffer, bufferSize);
    AndroidBitmap_unlockPixels(env, bitmap);
    execQ.push([=](JNIEnv *jvm) {
        RefImg img{.Data = (uint8_t *) dest.data(), .Dimensions = {.x = static_cast<int>(info.width), .y = static_cast<int>(info.height)}, .GL_Format = GL_RGBA};
        G_State->textures[quadEnt] = Texture2D(img);
    });
    drawMutex.unlock();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_testeopengl_NativeRenderer_setRawTexture(JNIEnv *env, jclass clazz, jint w, jint h,
                                                          jbyteArray bytes) {

}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_testeopengl_NativeRenderer_saveMusic(JNIEnv *env, jclass clazz,
                                                      jbyteArray music_bytes) {
    auto length = env->GetArrayLength(music_bytes);
    auto bytes = env->GetByteArrayElements(music_bytes, nullptr);
    FILE *file = fopen(std::string("./").append(musicFile).c_str(), "wb+");
    if (!file) {
        LOG("error: %d", errno);
        perror("failed opening file to write");
    }
    fwrite(bytes, sizeof(signed char), length, file);
    fclose(file);
    env->ReleaseByteArrayElements(music_bytes, bytes, 0);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_testeopengl_NativeRenderer_setBasePath(JNIEnv *env, jclass clazz,
                                                        jstring base_path) {
    auto path = env->GetStringUTFChars(base_path, nullptr);
    BasePath = path;
    int chRes = chdir(path);
    env->ReleaseStringUTFChars(base_path, path);
    //LOG("CHDIR is %s", std::filesystem::current_path().c_str());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_testeopengl_NativeRenderer_takePrint(JNIEnv *env, jclass clazz) {
    drawMutex.lock();
    execQ.push([](JNIEnv *) {
        auto bufferSize = G_State->framebuffer.GetPixelsBufferSize();
        auto *buffer = (unsigned char *) malloc(bufferSize);
        G_State->framebuffer.GetPixels(buffer);
        stbi_flip_vertically_on_write(true);
        stbi_write_png("image.png", G_State->framebuffer.ScreenSize.x,
                       G_State->framebuffer.ScreenSize.y, 3,
                       buffer, G_State->framebuffer.ScreenSize.x * 3);
        free(buffer);
    });
    drawMutex.unlock();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_testeopengl_NativeRenderer_SetScreenResolution(JNIEnv *env, jclass clazz,
                                                                jint width, jint height) {

}

int State::newEntity() {
    this->shaders.emplace_back();
    this->textures.emplace_back();
    this->quads.emplace_back();
    return this->shaders.size() - 1;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_testeopengl_NativeRenderer_SetFragment(JNIEnv *env, jclass clazz, jobject fragInstance) {
    FragmentClass = env->FindClass("com/example/testeopengl/FirstFragment");
    Fragment = env->NewGlobalRef(fragInstance);
    Fragment_setFrameState = env->GetMethodID(FragmentClass, "setFrameState", "(I)V");
}