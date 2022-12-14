
# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.22.1)

# Declares and names the project.

project("testeopengl")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)
# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_subdirectory(glm)
add_library(# Sets the name of the library.
            testeopengl

            # Sets the library as a shared library.
            SHARED

            # Provides a relative path to your source file(s).
            testeopengl.cpp testeopengl.h
            tinywav/tinywav.c tinywav/tinywav.h
            stbi_image_write.h stb_impl.cpp
            Framebuffer.cpp Framebuffer.h
            LogUtils.h
            Types.h Types.cpp
            Shader.cpp Shader.h
            TexturedQuad2D.cpp TexturedQuad2D.h
            Texture2D.h Texture2D.cpp
            video_rs.cpp video_rs.h
            minimp4.h
            AudioPlayer.cpp AudioPlayer.h
            minimp3.h minimp3_ex.h

            oboeDemo/AAssetDataSource.cpp oboeDemo/AAssetDataSource.h
            oboeDemo/DataSource.h
            oboeDemo/NDKExtractor.cpp oboeDemo/NDKExtractor.h
            oboeDemo/Player.cpp oboeDemo/Player.h
        )
target_include_directories(testeopengl PRIVATE include oboeDemo)
# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.
find_package(oboe REQUIRED CONFIG)
find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )
# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.
target_link_directories(testeopengl PRIVATE ${CMAKE_SOURCE_DIR}/libs/${CMAKE_ANDROID_ARCH_ABI})
target_link_libraries( # Specifies the target library.
                       testeopengl

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} GLESv3 jnigraphics glm avformat avcodec avutil oboe::oboe android mediandk andr_lib)

target_include_directories(testeopengl PRIVATE glm)

set_target_properties (testeopengl
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libtestando.so)