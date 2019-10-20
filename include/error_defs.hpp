/*
 * This file is a part of SkyrimNXModManager.
 * Copyright (c) 2019, Max Roncace <mproncace@gmail.com>
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

#include "console_helper.hpp"

#define FATAL(fmt, ...) CONSOLE_CLEAR_SCREEN(); \
                        CONSOLE_SET_COLOR(CONSOLE_COLOR_FG_RED); \
                        printf("Fatal error: " ); \
                        CONSOLE_SET_COLOR(CONSOLE_COLOR_FG_YELLOW); \
                        printf(fmt, ##__VA_ARGS__); \
                        CONSOLE_MOVE_DOWN(3); \
                        CONSOLE_MOVE_LEFT(99); \
                        CONSOLE_SET_COLOR(CONSOLE_COLOR_FG_GREEN); \
                        printf("Press (+) to exit.\n"); \
                        g_fatal_occurred = true

#define FATAL_CODE(code, fmt, ...) FATAL(fmt " (code %d)", ##__VA_ARGS__, code)

#define PANIC() FATAL("Panic @ %s:%d\n\nPlease contact the developer", __FILE__, __LINE__)

#define RC_SUCCESS(rc) (rc == 0)
#define RC_FAILURE(rc) (rc != 0)

#define DO_OR_DIE(rc, task, fail_fmt, ...)  if ((rc = RC_FAILURE((task)))) { \
                                                FATAL_CODE(rc, fail_fmt, ##__VA_ARGS__); \
                                                return -1; \
                                            }

static bool g_fatal_occurred = false;

inline bool fatal_occurred(void) {
    return g_fatal_occurred;
}
