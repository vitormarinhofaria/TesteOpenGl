// Write C++ code here.
//
// Do not forget to dynamically load the C++ library into your application.
//
// For instance,
//
// In MainActivity.java:
//    static {
//       System.loadLibrary("testeopengl");
//    }
//
// Or, in MainActivity.kt:
//    companion object {
//      init {
//         System.loadLibrary("testeopengl")
//      }
//    }
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

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stbi_image_write.h"

const char* frag = "#version 310 es\n"
                   "precision mediump float;\n"
                   "out vec4 FragColor;\n"
                   "in vec2 TexCoord;\n"
                   "uniform sampler2D textureSamp;\n"
                   "void main()\n"
                   "{\n"
                   "    FragColor = texture(textureSamp, TexCoord);\n"
                   "}\n";
const char* vert = "#version 310 es\n"
                   "precision mediump float;\n"
                   "layout (location = 0) in vec3 aPos;\n"
                   "layout (location = 1) in vec2 aTexCoord;\n"
                   "out vec2 TexCoord;\n"
                   "uniform mediump mat4 transform;\n"
                   "void main()\n"
                   "{\n"
                   "    gl_Position = transform * vec4(aPos, 1.0);\n"
                   "    TexCoord = aTexCoord;\n"
                   "}\n";

std::string BasePath = "./";
int Width, Height = 0;

double getNow(){
    timespec ts {};
    clock_gettime(CLOCK_REALTIME, &ts);
    return 1000.0 * ts.tv_sec + (double) ts.tv_nsec / 1e6;
}

const char* logTag = "FILE_PICKER";
#define LOG(...) __android_log_print(ANDROID_LOG_DEBUG, logTag, __VA_ARGS__)

double lastTime = 0;

static unsigned int vao, vbo, ebo;

float vertices[] = {
        // positions         // texture coords
        0.5f,  0.5f, 0.0f,  1.0f, 1.0f, // top right
        0.5f, -0.5f, 0.0f,  1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, // bottom left
        -0.5f,  0.5f, 0.0f,  0.0f, 1.0f  // top left
};
unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
};
GLuint createShader(){
    GLuint shader = glCreateProgram();
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertShader, 1, &vert, nullptr);
    glCompileShader(vertShader);
    GLint success = 0;
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
    if(success == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(vertShader, maxLength, &maxLength, &errorLog[0]);
        LOG("error %s", errorLog.data());
        // Provide the infolog in whatever manor you deem best.
        // Exit with failure.
        glDeleteShader(vertShader); // Don't leak the shader.
    }

    glShaderSource(fragShader, 1, &frag, nullptr);
    glCompileShader(fragShader);
    success = 0;
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
    if(success == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(fragShader, maxLength, &maxLength, &errorLog[0]);
        LOG("error %s", errorLog.data());
        // Provide the infolog in whatever manor you deem best.
        // Exit with failure.
        glDeleteShader(fragShader); // Don't leak the shader.
    }

    glAttachShader(shader, vertShader);
    glAttachShader(shader, fragShader);
    glLinkProgram(shader);
    success = 0;
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if(success == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        glGetProgramInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
        LOG("error %s", errorLog.data());
        // Provide the infolog in whatever manor you deem best.
        // Exit with failure.
        glDeleteProgram(shader); // Don't leak the shader.
    }
    //glDeleteShader(vertShader);
    //glDeleteShader(fragShader);
    return shader;
}


GLuint shader = 0;
GLuint tex = 0;

extern "C"
JNIEXPORT void JNICALL
Java_com_example_testeopengl_NativeRenderer_onSurfaceChangedNative(JNIEnv *env, jclass clazz,
                                                                   jobject gl, jint width,
                                                                   jint height) {
    // TODO: implement onSurfaceChangedNative()
}
void setUpFirstRender(){
    glGenVertexArrays(1, &vao);
    auto error = glGetError();

    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    shader = createShader();

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned char img_tex[] = {0, 0, 255, 255, 0, 0, 0, 255, 0, 0, 0, 0};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 4, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, img_tex);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
}
std::vector<int16_t> musicBytes = {};
glm::mat4 camera;
glm::mat4 view;
glm::mat4 proj;
glm::mat4 model;
static glm::vec3 modelScale = {1.0f, 1.0f, 1.0f};
GLint mvpLocation = 0;
TinyWav tw;
static const char* musicFile = "gen.wav";
extern "C"
JNIEXPORT void JNICALL
Java_com_example_testeopengl_NativeRenderer_onSurfaceCreatedNative(JNIEnv *env, jclass clazz,
                                                                   jobject gl, jobject config) {
    setUpFirstRender();

    proj = glm::ortho(0.0f, 350.0f, 240.0f, 0.0f, -1000.0f, 1000.0f);
    view = glm::lookAt(glm::vec3 {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
    model = glm::scale(glm::mat4(1.0f), modelScale);
    mvpLocation = glGetUniformLocation(shader, "transform");
    tinywav_open_read(&tw, musicFile, TW_INTERLEAVED, false);
    int16_t samples[2 * 44100];

    while(auto read = tinywav_read_f(&tw, samples, 441)){
        for(auto i = 0; i < read; i++){
            musicBytes.push_back(samples[i]);
        }
    }
    tinywav_close_read(&tw);
    lastTime = getNow();
}
std::mutex drawMutex {};
std::queue<std::function<void(JNIEnv*)>> execQ;

float r, g, b = 0;
size_t musicPtr = 0;
extern "C"
JNIEXPORT void JNICALL
Java_com_example_testeopengl_NativeRenderer_onDrawFrameNative(JNIEnv *env, jclass clazz,
                                                              jobject gl) {
    if(!drawMutex.try_lock()){
        return;
    }
    while(!execQ.empty()){
        auto d = execQ.front();
        d(env);
        execQ.pop();
    }

    double now = getNow();
    double delta = now - lastTime;

    static bool reversing = false;
    float colorDelta = 0.002f;
    if(r >= 1.0 && !reversing){
        reversing = true;
    }
    if (reversing && r <= 0.0f){
        reversing = false;
    }
    if (reversing)
        colorDelta = -colorDelta;

    r += (colorDelta * static_cast<float>(delta));

    glClearColor(r, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    if(musicPtr == musicBytes.size()){musicPtr = 0;}
    glm::vec3 modScale = {1.0f, 1.0f, 1.0f};
    if(musicPtr < musicBytes.size()){
        auto nval = static_cast<int16_t>(abs(musicBytes[musicPtr]));
//        uint64_t val = musicBytes[musicPtr] + INT16_MAX + 1;
//        float nval = (float)val * 0.000001;
        modScale.x = (float)nval * 0.00001f;
        modScale.y = (float)nval * 0.00001f;
        modScale.z = 1.0f;
        musicPtr+=1;
    }

    glm::mat4 modelTrans = glm::mat4(1.0f);
    modelTrans = glm::translate(modelTrans, glm::vec3{0.0, modelScale.x * 0.2, 0.0f});
    //modelTrans = glm::scale(modelTrans, glm::vec3{1.0f + r * 0.1, 1.0f + r * 0.1, 1.0});
    modelTrans = glm::scale(modelTrans, glm::vec3{1.0f, 1.0f, 1.0f} + modScale);

    //model = glm::scale(model, modelScale);
    glm::mat4 mvp = proj * view * modelTrans;
    mvpLocation = glGetUniformLocation(shader, "transform");
    glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(modelTrans));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUseProgram(shader);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    lastTime = getNow();
    static uint64_t frameCount = 0;
    static bool taken = false;
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
    void* buffer;
    AndroidBitmap_lockPixels(env, bitmap, &buffer);
    std::vector<GLbyte> dest{};
    auto bufferSize = (info.height * info.width)* (sizeof(GLbyte) * 4);
    dest.resize(bufferSize);
    std::memcpy(dest.data(), buffer, bufferSize);
    AndroidBitmap_unlockPixels(env, bitmap);
    execQ.push([=](JNIEnv* jvm){
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, info.width, info.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, dest.data());
        glGenerateMipmap(GL_TEXTURE_2D);
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
    FILE* file = fopen(std::string("./").append(musicFile).c_str(), "wb+");
    if(!file){
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
    BasePath = std::string(path).append("/");
    int chRes = chdir(path);
    env->ReleaseStringUTFChars(base_path, path);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_testeopengl_NativeRenderer_takePrint(JNIEnv *env, jclass clazz) {
    drawMutex.lock();
    execQ.push([](JNIEnv*){
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        auto* buffer = (unsigned char*)malloc(Width * Height * 3);
        glReadPixels(0, 0, Width, Height, GL_RGB, GL_UNSIGNED_BYTE, buffer);
        stbi_flip_vertically_on_write(true);
        stbi_write_png("image.png", Width, Height, 3, buffer, Width * 3);
        free(buffer);
    });
    drawMutex.unlock();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_testeopengl_NativeRenderer_SetScreenResolution(JNIEnv *env, jclass clazz,
                                                                jint width, jint height) {
    Width = width;
    Height = height;
    glViewport(0,0,width,height);
}