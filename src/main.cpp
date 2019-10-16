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

#include "console_helper.hpp"

#include <switch.h>

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <cstdio>
#include <dirent.h>

#define PANIC() CONSOLE_MOVE_DOWN(3); \
                CONSOLE_SET_COLOR(CONSOLE_COLOR_RED); \
                printf("Panic @ %s:%d", __FILE__, __LINE__)

#define EXT_ESP "esp"
#define EXT_BSA "bsa"

#define SKYRIM_ROMFS_DIR "sdmc:/atmosphere/titles/01000A10041EA000/romfs"
#define SKYRIM_DATA_DIR SKYRIM_ROMFS_DIR "/Data"
#define SKYRIM_INI_FILE SKYRIM_ROMFS_DIR "/Skyrim.ini"
#define SKYRIM_INI_LANG_FILE SKYRIM_ROMFS_DIR "/Skyrim_%s.ini"
#define SKYRIM_PLUGINS_FILE SKYRIM_ROMFS_DIR "/Plugins"

struct SkyrimMod {
    const std::string base_name;
    bool has_esp;
    bool esp_enabled;
    std::vector<std::string> bsa_suffixes;
    std::vector<std::string> enabled_bsps;

    SkyrimMod(const std::string &base_name):
            base_name(base_name),
            has_esp(false),
            esp_enabled(false),
            bsa_suffixes(),
            enabled_bsps() {
    }
};

std::vector<std::shared_ptr<SkyrimMod>> g_mod_list;

int discoverMods() {
    DIR *dir = opendir(SKYRIM_DATA_DIR);

    if (!dir) {
        CONSOLE_CLEAR_SCREEN();
        CONSOLE_SET_COLOR(CONSOLE_COLOR_RED);
        printf("No Skyrim data folder found!\n");
        return -1;
    }

    std::vector<std::string> files;

    struct dirent *ent;
    while ((ent = readdir(dir))) {
        if (ent->d_type != DT_REG) {
            continue;
        }

        files.insert(files.end(), ent->d_name);
    }

    closedir(dir);

    printf("Found %lu mod files\n", files.size());

    std::map<std::string, std::shared_ptr<SkyrimMod>> mod_map;

    for (std::string file_name : files) {
        size_t dot_index = file_name.find_last_of('.');
        if (dot_index == std::string::npos) {
            continue;
        }

        std::string base = file_name.substr(0, dot_index);
        std::string ext = file_name.substr(dot_index + 1);
        std::string suffix;

        if (ext == EXT_BSA) {
            size_t dash_index = base.rfind(" - ");
            if (dash_index == std::string::npos) {
                suffix = "";
            } else {
                suffix = base.substr(dash_index + 3);
                base = base.substr(0, dash_index);
            }
        } else if (ext != EXT_ESP) {
            continue;
        }

        std::shared_ptr<SkyrimMod> mod;
        auto mod_it = mod_map.find(base);
        if (mod_it != mod_map.cend()) {
            mod = mod_it->second;
        } else {
            mod = std::shared_ptr<SkyrimMod>(new SkyrimMod(base));
            mod_map.insert(std::pair(base, mod));
        }

        if (ext == EXT_ESP) {
            mod->has_esp = true;
        } else if (ext == EXT_BSA) {
            mod->bsa_suffixes.insert(mod->bsa_suffixes.end(), suffix);
        } else {
            PANIC();
            return -1;
        }
    }

    for (auto it = mod_map.cbegin(); it != mod_map.cend(); it++) {
        g_mod_list.insert(g_mod_list.begin(), it->second);
    }

    return 0;
}

int parseInis() {
    char *sys_lang_code;
    return 0;
}

int initialize(void) {
    int rc;

    CONSOLE_SET_POS(0, 0);
    CONSOLE_CLEAR_SCREEN();
    CONSOLE_SET_ATTRS(CONSOLE_ATTR_BOLD);
    printf("Discovering available mods...\n");
    
    rc = discoverMods();
    if (rc != 0) {
        return rc;
    }

    printf("Identified %lu mods\n", g_mod_list.size());

    std::sort(g_mod_list.begin(), g_mod_list.end(), [](auto &a, auto &b) { return (a->base_name.compare(b->base_name)); });

    CONSOLE_MOVE_DOWN(3);
    printf("Mod listing:\n\n");
    for (std::shared_ptr<SkyrimMod> mod : g_mod_list) {
        printf("  - %s\n", mod->base_name.c_str());
    }

    return 0;
}

int main(int argc, char **argv) {
    consoleInit(NULL);

    int init_status = initialize();

    if (init_status != 0) {
        CONSOLE_MOVE_DOWN(3);
        CONSOLE_SET_COLOR(CONSOLE_COLOR_GREEN);
        printf("Press (+) to exit.\n");
    }

    while (appletMainLoop()) {
        hidScanInput();

        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

        if (kDown & KEY_PLUS) {
            break;
        }

        consoleUpdate(NULL);
    }

    consoleExit(NULL);
    return 0;
}
