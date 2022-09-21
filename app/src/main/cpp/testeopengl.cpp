#include <cstdio>
#include <ctime>
#include <vector>
#include <string>
#include <mutex>
#include <queue>
#include <functional>
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>

#include <jni.h>

#include <android/log.h>
#include <android/bitmap.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <GLES3/gl31.h>
#include <unistd.h>
#include "glm/glm/glm.hpp"
#include "glm/glm/ext.hpp"
#include "tinywav/tinywav.h"

#include "stbi_image_write.h"
#include "testeopengl.h"
#include "video_rs.h"
#include "AudioPlayer.h"

#include "AAssetDataSource.h"

#define MINIMP3_FLOAT_OUTPUT
#define MINIMP3_IMPLEMENTATION

#include "minimp3.h"
#include "minimp3_ex.h"

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

///Return time in milliseconds
double getNow() {
    timespec ts{};
    clock_gettime(CLOCK_REALTIME, &ts);
    return (double) ts.tv_nsec / 1e6;
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
jmethodID Fragment_setTextView;

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

std::vector<float> musicBytes = {};
std::vector<float> musicBytesR = {};
std::vector<float> musicBytesL = {};
std::vector<float> filteredBytesL = {};

static glm::vec3 modelScale = {1.0f, 1.0f, 1.0f};

AAssetManager *GAssetManager = nullptr;

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

    lastTime = getNow();

    AudioPlayer::get()->BeginPlay(musicBytesL.data(), musicBytesL.size(), musicBytesR.data());
}

std::fstream videoOutput;

void saveFrame(int frameNum, JNIEnv *env) {
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
    env->CallVoidMethod(Fragment, Fragment_setFrameState, (jint) frameNum, 500);
    LOG("Frame %d sr is %d %s", frameNum, sr, fileName.c_str());
}

std::mutex drawMutex{};
std::queue<std::function<void(JNIEnv *)>> execQ;
constexpr static int frameRate = 24;
constexpr static int secondsMax = 10;
constexpr static int framesMax = frameRate * secondsMax;
constexpr static double frameDeltaTs = (double) secondsMax / (double) framesMax;

float r, g, b = 0;
size_t musicPtr = 0;
bool musicIsPlaying = true;
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
    double deltaTime = now - lastTime;
    lastTime = now;

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

    r += (colorDelta * static_cast<float>(frameDeltaTs));

    glViewport(0, 0, 1920, 1080);
    G_State->framebuffer.Bind();

    glClearColor(r, 0.5, 0.2, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    if (musicPtr == 0 && musicIsPlaying) {
        AudioPlayer::get()->m_audioPlayer->setPlaying(true);
    }
    if (musicPtr == musicBytesL.size()) { musicPtr = 0; }
    glm::vec3 modScale = {1.0f, 1.0f, 1.0f};
    if (musicPtr < musicBytesL.size()) {
        double sum = 0;

        auto samplesPerFrame = size_t(48000.0f * (deltaTime / 1000));
        size_t avgSize = samplesPerFrame / 2;
        musicPtr += samplesPerFrame;
        if (musicPtr >= musicBytesL.size()) musicPtr = 0;

        auto samplesUsed = 0;
        for (auto i = 0; i < avgSize; i++) {
            float val = musicBytesL[musicPtr + i * 2];
            samplesUsed++;
            if (val < 0) {
                val *= -1;
            }
            sum += val;

        }
        //auto nval = abs(musicBytes[musicPtr + ((44100/24)/2)]);

        auto nval = (float) (sum / samplesUsed);
        musicPtr += samplesPerFrame;
        modScale.x = nval * 0.8f;
        modScale.y = nval * 0.8f;
        modScale.z = 1.0f;
    }

    glm::mat4 modelTrans = glm::mat4(1.0f);
    modelTrans = glm::translate(modelTrans, glm::vec3{0.0, 0.0f, 0.0f});
    modelTrans = glm::scale(modelTrans, modScale + glm::vec3{1.0f, 1.0f, 1.0f});

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
    static bool shouldSave = false;
    if (frameCount == 0 && shouldSave) {
        initEncoding();
        //segment.Init(&writer);
    }
    if (frameCount < framesMax && shouldSave) {
        auto beforeFrame = getNow();
        if (true) {
            auto bufferSize = G_State->framebuffer.GetPixelsBufferSize();
            auto *buffer = (unsigned char *) malloc(bufferSize);
            G_State->framebuffer.GetPixels(buffer);
            addFrame(buffer, frameCount);
            delete buffer;

        } else {
            saveFrame(frameCount, env);
        }
        auto afterFrame = getNow();
        LOG("Frame took %f", afterFrame - beforeFrame);
        env->CallVoidMethod(Fragment, Fragment_setFrameState, (jint) frameCount, framesMax);
    }
    //if (frameCount == 1105 || frameCount == 1) {
    if (frameCount == framesMax && shouldSave) {
        //auto saveMp4_Id = env->GetStaticMethodID(clazz, "saveMp4", "()V");
        //env->CallStaticVoidMethod(clazz, saveMp4_Id);
        endEncoding();
        //auto showToast_mId = env->GetMethodID(FragmentClass, "showToastEndEncoding", "()V");
        //env->CallVoidMethod(Fragment, showToast_mId);
        LOG("Ended encoding");
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

void applySmoothOperator(std::vector<float> &buffer) {
    uint32_t iterations = 1;
    const uint32_t totalIterations = buffer.size() / 9;
    for (auto i = 4; i < buffer.size() - 4; i += 8) {
        float sum = 0;
        sum += buffer[i - 4];
        sum += buffer[i - 3];
        sum += buffer[i - 2];
        sum += buffer[i - 1];
        sum += buffer[i];
        sum += buffer[i + 1];
        sum += buffer[i + 2];
        sum += buffer[i + 3];
        sum += buffer[i + 4];

        const float midSample = sum / 9;
        buffer[i] = midSample;
        if (iterations == 1) {
            for (auto x = 0; x < 4; x++) {
                buffer[x] = midSample;
            }
        } else if (iterations != totalIterations) {
            float prevSample = buffer[i - 5];
            const float sampleDiff = midSample - prevSample;
            const float diffRatio = sampleDiff / 4;
            for (auto x = 3; x >= 0; x--) {
                buffer[i - x] = prevSample + diffRatio;
                prevSample = buffer[i - x];
            }
        }

        iterations += 1;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_testeopengl_NativeRenderer_saveMusic(JNIEnv *env, jclass clazz,
                                                      jbyteArray music_bytes) {
//    auto length = env->GetArrayLength(music_bytes);
//    auto *bytes = env->GetByteArrayElements(music_bytes, nullptr);
//    FILE *file = fopen(std::string("./").append(musicFile).c_str(), "wb+");
//    if (!file) {
//        LOG("error: %d", errno);
//        perror("failed opening file to write");
//    }
//    fwrite(bytes, sizeof(signed char), length, file);
//    fclose(file);

    auto *asset = AAssetManager_open(GAssetManager, "twice.mp3", AASSET_MODE_BUFFER);
    auto assetSize = AAsset_getRemainingLength64(asset);
    std::vector<uint8_t> assetBuffer;
    assetBuffer.resize(assetSize);
    auto readResult = AAsset_read(asset, assetBuffer.data(), assetSize);
    AAsset_close(asset);

    mp3dec_t mp3d;
    mp3dec_init(&mp3d);

    mp3dec_t mp3dec;
    mp3dec_file_info_t info{};
    std::vector<uint8_t> recBuffer{};
    recBuffer.resize(MINIMP3_BUF_SIZE * 10);
    auto result = mp3dec_load_buf(&mp3dec, assetBuffer.data(), assetBuffer.size(), &info, nullptr,
                                  nullptr);
    if (result != 0) {
        LOG("Failed to decode MP3. Result is: %d", result);
    }


    musicBytes.clear();
    musicBytes.resize(info.samples * info.channels);
    std::memcpy(musicBytes.data(), info.buffer, info.samples * info.channels);
    free(info.buffer);

    ///Prepare the Left and Right channels buffers for receiving their separated samples
    musicBytesL.reserve(musicBytes.size() / 2);
    musicBytesR.reserve(musicBytes.size() / 2);
    for (auto i = 0; i < musicBytes.size(); i++) {
        if (i % 2 == 0) {
            musicBytesL.push_back(musicBytes[i]);
        } else {
            musicBytesR.push_back(musicBytes[i]);
        }
    }

    ///Clear the main music samples buffer before applying effects to individual channels and joining then
    musicBytes.clear();
    musicBytes.reserve(info.samples * info.channels);

    std::vector<float> originalChanL = musicBytesL;
    std::vector<float> originalChanR = musicBytesR;

    applySmoothOperator(musicBytesL);
    applySmoothOperator(musicBytesR);
    for (auto i = 0; i < musicBytesL.size(); i++) {
        musicBytes.push_back(musicBytesL[i] + originalChanL[i]);
        musicBytes.push_back(musicBytesR[i] + originalChanR[i]);
    }
    filteredBytesL = originalChanL;

    AudioProperties targetProperties{.channelCount = info.channels, .sampleRate = info.hz};
    std::shared_ptr<AAssetDataSource> modAudio{
            AAssetDataSource::fromRaw(musicBytes.data(), musicBytes.size(), targetProperties)};

    AudioPlayer::get()->m_audioPlayer = std::make_unique<Player>(modAudio);
    AudioPlayer::get()->m_audioPlayer->setLooping(true);

//    env->ReleaseByteArrayElements(music_bytes, bytes,
//                                  0);
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

jobject jassetManagerRef;
extern "C"
JNIEXPORT void JNICALL
Java_com_example_testeopengl_NativeRenderer_SetFragment(JNIEnv *env, jclass clazz,
                                                        jobject fragInstance,
                                                        jobject assetManager) {
    loadLib();
    FragmentClass = env->FindClass("com/example/testeopengl/FirstFragment");
    Fragment = env->NewGlobalRef(fragInstance);
    Fragment_setFrameState = env->GetMethodID(FragmentClass, "setFrameState", "(II)V");
    Fragment_setTextView = env->GetMethodID(FragmentClass, "setVersionText",
                                            "(Ljava/lang/String;)V");
    const char *av_info = avcodec_configuration();
    auto jString = env->NewStringUTF(av_info);
    env->CallVoidMethod(Fragment, Fragment_setTextView, jString);
    env->DeleteLocalRef(jString);

    jassetManagerRef = env->NewGlobalRef(assetManager);
    GAssetManager = AAssetManager_fromJava(env, assetManager);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_testeopengl_NativeRenderer_PlayMusic(JNIEnv *env, jclass clazz) {
    musicPtr = 0;
    AudioPlayer::get()->m_audioPlayer->setPlaying(true);
    musicIsPlaying = true;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_testeopengl_NativeRenderer_PauseMusic(JNIEnv *env, jclass clazz) {
    AudioPlayer::get()->m_audioPlayer->setPlaying(false);
    musicPtr = 0;
    musicIsPlaying = false;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_testeopengl_NativeRenderer_ResetMusic(JNIEnv *env, jclass clazz) {
    musicPtr = 0;
    AudioPlayer::get()->m_audioPlayer->resetPlayHead();
}