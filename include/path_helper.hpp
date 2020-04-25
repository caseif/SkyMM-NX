/*
 * This file is a part of SkyMM-NX.
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

#include <string>

#define SKYRIM_ROMFS_DIR "sdmc:/atmosphere/contents/01000A10041EA000/romfs"
#define SKYRIM_ROMFS_DIR_OLD "sdmc:/atmosphere/titles/01000A10041EA000/romfs"
#define SKYRIM_DATA_DIR "Data"
#define SKYRIM_INI_FILE "Skyrim.ini"
#define SKYRIM_INI_LANG_FILE_PREFIX "Skyrim_"
#define SKYRIM_PLUGINS_FILE "Plugins"

#define LANG_CODE_MAX_LEN 6

std::string getRomfsPath(std::string &partial);

std::string getRomfsPath(const char *partial);

const char *getBaseRomfsPath(void);
