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

#include <cstdio>

#include <switch.h>

#define CONSOLE_LINES 44

#define _PRINT_ESC(s) printf(CONSOLE_ESC(s))
#define _EXPAND(a) a

#define CONSOLE_ATTR_NONE 0
#define CONSOLE_ATTR_BOLD 1
#define CONSOLE_ATTR_LIGHT 2
#define CONSOLE_ATTR_UNDERSCORE 4
#define CONSOLE_ATTR_BLINK 5
#define CONSOLE_ATTR_REVERSE 7
#define CONSOLE_ATTR_CONCEALED 8
#define CONSOLE_ATTR_STRIKETHROUGH 9

#define CONSOLE_COLOR_FG_BLACK 30
#define CONSOLE_COLOR_FG_RED 31
#define CONSOLE_COLOR_FG_GREEN 32
#define CONSOLE_COLOR_FG_YELLOW 33
#define CONSOLE_COLOR_FG_BLUE 34
#define CONSOLE_COLOR_FG_MAGENTA 35
#define CONSOLE_COLOR_FG_CYAN 36
#define CONSOLE_COLOR_FG_WHITE 37

#define CONSOLE_COLOR_BG_BLACK 40
#define CONSOLE_COLOR_BG_RED 41
#define CONSOLE_COLOR_BG_GREEN 42
#define CONSOLE_COLOR_BG_YELLOW 43
#define CONSOLE_COLOR_BG_BLUE 44
#define CONSOLE_COLOR_BG_MAGENTA 45
#define CONSOLE_COLOR_BG_CYAN 46
#define CONSOLE_COLOR_BG_WHITE 47

#define CONSOLE_SET_ATTRS(a) _PRINT_ESC(_EXPAND(a)m)

#define CONSOLE_SET_COLOR(color) CONSOLE_SET_ATTRS(color)

#define CONSOLE_CLEAR_SCREEN() _PRINT_ESC(2J)
#define CONSOLE_CLEAR_LINE() _PRINT_ESC(K)

#define CONSOLE_SET_POS(x, y) _PRINT_ESC(_EXPAND(x);_EXPAND(y)H)
#define CONSOLE_MOVE_UP(lines) _PRINT_ESC(_EXPAND(lines)A)
#define CONSOLE_MOVE_DOWN(lines) _PRINT_ESC(_EXPAND(lines)B)
#define CONSOLE_MOVE_RIGHT(cols) _PRINT_ESC(_EXPAND(cols)C)
#define CONSOLE_MOVE_LEFT(cols) _PRINT_ESC(_EXPAND(cols)D)

#define CONSOLE_PUSH_POS() _PRINT_ESC(s)
#define CONSOLE_POP_POS() _PRINT_ESC(u)
