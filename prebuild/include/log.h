/*
 * Copyright (C) 2005 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//
// C/C++ logging functions.  See the logging documentation for API details.
//
// We'd like these to be available from C code (in case we import some from
// somewhere), so this has a C interface.
//
// The output will be correct when the log file is shared between multiple
// threads and/or multiple processes so long as the operating system
// supports O_APPEND.  These calls have mutex-protected data structures
// and so are NOT reentrant.  Do not use LOG in a signal handler.
//
#ifndef _LIBS_CUTILS_LOG_H
#define _LIBS_CUTILS_LOG_H

#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DEBUG
#ifndef DEBUG
#define ALOGV(...)   ((void)0)
#define ALOGI(...)   ((void)0)
#define ALOGD(...)   ((void)0)
#define ALOGW(...)   ((void)0)
#define ALOGE(...)   ((void)0)
#else
#define ALOGV(...) { do { printf("V:");printf(__VA_ARGS__);printf("\n"); } while(0); }
#define ALOGI(...) { do { printf("I:");printf(__VA_ARGS__);printf("\n"); } while(0); }
#define ALOGW(...) { do { printf("W:");printf(__VA_ARGS__);printf("\n"); } while(0); }
#define ALOGD(...) { do { printf("D:");printf(__VA_ARGS__);printf("\n"); } while(0); }
#define ALOGE(...) { do { printf("E:");printf(__VA_ARGS__);printf("\n"); } while(0); }
#endif
#ifdef __cplusplus
}
#endif

#endif // _LIBS_CUTILS_LOG_H
