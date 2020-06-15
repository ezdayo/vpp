/**
 *
 * @file      vpp/log.hpp
 *
 * @brief     These are some C/C++ preprocessor MACRO for logging information
 *
 *            This file is part of the VPP framework (see link).
 *
 * @author    Olivier Stoltz-Douchet <ezdayo@gmail.com>
 *
 * @copyright (c) 2019-2020 Olivier Stoltz-Douchet
 * @license   http://opensource.org/licenses/MIT MIT
 * @link      https://github.com/ezdayo/vpp
 *
 **/

#pragma once

#ifndef LOGTAG
#pragma message("Define LOGTAG to match the name of your application!!!")
#define LOGTAG "my-app"
#endif

#ifdef __ANDROID__ /* On Android */
#include <android/log.h>
#include <cstdio>

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOGTAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOGTAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOGTAG, __VA_ARGS__)

#else /* On Linux */
#include <cstdio>

extern FILE *STDE;
extern FILE *STDW;
extern FILE *STDO;

#define LOGE(...)                                    \
    fprintf(STDE, "[E] " LOGTAG ": " __VA_ARGS__); \
    fprintf(STDE, "\n")
#define LOGW(...)                                    \
    fprintf(STDW, "[W] " LOGTAG ": " __VA_ARGS__); \
    fprintf(STDW, "\n")
#define LOGI(...)                                    \
    fprintf(STDO, "[I] " LOGTAG ": " __VA_ARGS__); \
    fprintf(STDO, "\n")

#endif

#include <cassert>

#ifdef NDEBUG
#define LOGD(...) ((void)0)
#define LOGD_IF_NOT(condition, ...) ((void)0)
#define ASSERT(condition, ...) ((void)0)
#else
#define LOGD(...) LOGW(__VA_ARGS__)
#define LOGD_IF_NOT(condition, ...) \
    if ((!(condition)))             \
    {                               \
        LOGD(__VA_ARGS__);          \
    }
#define ASSERT(condition, ...) \
    if ((!(condition)))        \
    {                          \
        LOGE(__VA_ARGS__);     \
    };                         \
    assert(condition)
#endif
