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
#include "error_defs.hpp"
#include "gui.hpp"
#include "ini_helper.hpp"
#include "mod.hpp"
#include "path_defs.hpp"
#include "string_helper.hpp"

#include <inipp/inipp.h>
#include <switch.h>

#include <algorithm>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <cstdio>
#include <dirent.h>

#define STRINGIZE0(x) #x
#define STRINGIZE(x) STRINGIZE0(x)

#ifndef __VERSION
#define __VERSION "Unknown"
#endif

#define HEADER_HEIGHT 3
#define FOOTER_HEIGHT 5

#define HRULE "--------------------------------"

static HidControllerKeys g_key_edit_lo = KEY_Y;

int discoverMods() {
    DIR *dir = opendir(SKYRIM_DATA_DIR);

    if (!dir) {
        FATAL("No Skyrim data folder found!");
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

    for (std::string file_name : files) {
        ModFile mod_file = ModFile::fromFileName(file_name);

        if (mod_file.type == ModFileType::UNKNOWN) {
            continue;
        } 

        std::shared_ptr<SkyrimMod> mod = find_mod(mod_file.base_name);
        if (!mod) {
            mod = std::shared_ptr<SkyrimMod>(new SkyrimMod(mod_file.base_name));
            get_global_mod_list().insert(get_global_mod_list().end(), mod);
        }

        if (mod_file.type == ModFileType::ESP) {
            mod->has_esp = true;
        } else if (mod_file.type == ModFileType::BSA) {
            mod->bsa_suffixes.insert(mod->bsa_suffixes.end(), mod_file.suffix);
        } else {
            PANIC();
            return -1;
        }
    }

    return 0;
}

int processPluginsFile() {
    std::ifstream plugins_stream = std::ifstream(SKYRIM_PLUGINS_FILE, std::ios::in);
    if (!plugins_stream.good()) {
        FATAL("Failed to read Plugins file");
        return -1;
    }

    std::string line;
    while (std::getline(plugins_stream, line)) {
        if (line.length() == 0 || line.at(0) != '*') {
            continue;
        }

        std::string file_name = line.substr(1);
        ModFile file_def = ModFile::fromFileName(file_name);
        if (file_def.type != ModFileType::ESP) {
            continue;
        }

        std::shared_ptr<SkyrimMod> mod = find_mod(file_def.base_name);
        if (!mod) {
            continue;
        }

        mod->esp_enabled = true;
    }

    return 0;
}

int initialize(void) {
    int rc;

    CONSOLE_SET_POS(0, 0);
    CONSOLE_CLEAR_SCREEN();
    CONSOLE_SET_ATTRS(CONSOLE_ATTR_BOLD);
    printf("Discovering available mods...\n");

    if (RC_FAILURE(rc = discoverMods())) {
        return rc;
    }

    if (RC_FAILURE(rc = parseInis())) {
        return rc;
    }

    if (RC_FAILURE(rc = processPluginsFile())) {
        return rc;
    }

    printf("Identified %lu mods\n", get_global_mod_list().size());

    CONSOLE_MOVE_DOWN(3);
    printf("Mod listing:\n\n");
    for (auto it = get_global_mod_list().cbegin(); it != get_global_mod_list().cend(); it++) {
        ModStatus status = (*it)->getStatus();
        const char *status_str;
        switch (status) {
            case ModStatus::ENABLED:
                status_str = "Enabled";
                break;
            case ModStatus::DISABLED:
                status_str = "Disabled";
                break;
            case ModStatus::PARTIAL:
                status_str = "Partial";
                break;
            default:
                PANIC();
                return -1;
        }
        printf("  - %s (%s)\n", (*it)->base_name.c_str(), status_str);
    }

    return 0;
}

static void redrawHeader(void) {
    CONSOLE_SET_POS(0, 0);
    CONSOLE_CLEAR_LINE();
    CONSOLE_SET_COLOR(CONSOLE_COLOR_FG_CYAN);
    printf("SkyMM-NX v" STRINGIZE(__VERSION) " by caseif");
    CONSOLE_SET_COLOR(CONSOLE_COLOR_FG_WHITE);

    CONSOLE_MOVE_DOWN(2);
    CONSOLE_MOVE_LEFT(255);
    printf(HRULE);
}

static void redrawFooter(bool edit_lo) {
    CONSOLE_SET_POS(39, 0);
    CONSOLE_CLEAR_LINE();
    printf(HRULE);
    CONSOLE_MOVE_LEFT(255);
    CONSOLE_MOVE_DOWN(1);

    CONSOLE_MOVE_DOWN(1);
    CONSOLE_CLEAR_LINE();
    if (edit_lo) {
        CONSOLE_SET_ATTRS(CONSOLE_ATTR_NONE);
        CONSOLE_SET_COLOR(CONSOLE_COLOR_FG_YELLOW);
        printf("(Editing load order)");
        CONSOLE_SET_COLOR(CONSOLE_COLOR_FG_WHITE);
        CONSOLE_SET_ATTRS(CONSOLE_ATTR_BOLD);
    }
    CONSOLE_MOVE_LEFT(255);

    CONSOLE_MOVE_DOWN(1);
    CONSOLE_CLEAR_LINE();

    CONSOLE_MOVE_DOWN(1);
    CONSOLE_CLEAR_LINE();
    CONSOLE_SET_COLOR(CONSOLE_COLOR_FG_GREEN);
    printf("(Up/Down) Navigate  |  (Y) (hold) Change load order  |  (+) Exit");
    CONSOLE_SET_COLOR(CONSOLE_COLOR_FG_WHITE);
    CONSOLE_MOVE_LEFT(255);
}

int main(int argc, char **argv) {
    consoleInit(NULL);

    ModGui gui = ModGui(get_global_mod_list(), HEADER_HEIGHT, CONSOLE_LINES - HEADER_HEIGHT - FOOTER_HEIGHT);

    int init_status = initialize();
    if (RC_SUCCESS(init_status)) {
        CONSOLE_CLEAR_SCREEN();

        redrawHeader();
        gui.redraw();
        redrawFooter(false);
    }

    bool edit_load_order = false;

    while (appletMainLoop()) {
        hidScanInput();

        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        u64 kUp = hidKeysUp(CONTROLLER_P1_AUTO);

        if (kDown & KEY_PLUS) {
            break;
        }

        if (RC_FAILURE(init_status)) {
            consoleUpdate(NULL);
            continue;
        }

        if (kDown & g_key_edit_lo) {
            edit_load_order = true;
            redrawFooter(true);
        }
        
        if (kUp & g_key_edit_lo) {
            edit_load_order = false;
            redrawFooter(false);
        }

        if (kDown & KEY_DOWN) {
            if (edit_load_order) {
                gui.getSelectedMod()->loadLater();
            }

            gui.scrollSelection(1);
        } else if (kDown & KEY_UP) {
            if (edit_load_order) {
                gui.getSelectedMod()->loadSooner();
            }

            gui.scrollSelection(-1);
        } else if (kDown & KEY_A) {
            //TODO: toggle mod
        }

        consoleUpdate(NULL);
    }

    consoleExit(NULL);
    return 0;
}
