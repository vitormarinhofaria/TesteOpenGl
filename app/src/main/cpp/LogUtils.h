//
// Created by vitor on 08/09/2022.
//

#ifndef TESTEOPENGL_LOGUTILS_H
#define TESTEOPENGL_LOGUTILS_H
#pragma once
#include <android/log.h>

namespace{
}
static const char* __TESTEOPENGL_CPP_logTag__ = "TESTEOPENGL_CPP";
#define LOG(...) __android_log_print(ANDROID_LOG_DEBUG, __TESTEOPENGL_CPP_logTag__, __VA_ARGS__)

#endif //TESTEOPENGL_LOGUTILS_H
