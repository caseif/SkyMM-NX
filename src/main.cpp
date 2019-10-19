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

static bool g_dirty = false;
static bool g_dirty_warned = false;

static std::string g_status_msg = "";
static bool g_tmp_status = false;

static ModList g_mod_list_tmp;

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

        std::shared_ptr<SkyrimMod> mod = find_mod(g_mod_list_tmp, mod_file.base_name);
        if (!mod) {
            mod = std::shared_ptr<SkyrimMod>(new SkyrimMod(mod_file.base_name));
            // everything gets loaded into a temp buffer so we can rebuild it with the proper order later
            g_mod_list_tmp.insert(g_mod_list_tmp.end(), mod);
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

        std::shared_ptr<SkyrimMod> mod = find_mod(getGlobalModList(), file_def.base_name);
        if (!mod) {
            mod = find_mod(g_mod_list_tmp, file_def.base_name);
            if (mod) {
                getGlobalModList().insert(getGlobalModList().end(), mod);
            } else {
                continue;
            }
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

    if (RC_FAILURE(rc = processPluginsFile())) {
        return rc;
    }

    if (RC_FAILURE(rc = parseInis(getGlobalModList(), g_mod_list_tmp))) {
        return rc;
    }

    for (std::shared_ptr<SkyrimMod> mod : g_mod_list_tmp) {
        std::string mod_name = mod->base_name;
        if (!find_mod(getGlobalModList(), mod_name)) {
            getGlobalModList().insert(getGlobalModList().end(), mod);
        }
    }

    printf("Identified %lu mods\n", getGlobalModList().size());

    CONSOLE_MOVE_DOWN(3);
    printf("Mod listing:\n\n");
    for (auto it = getGlobalModList().cbegin(); it != getGlobalModList().cend(); it++) {
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

static void redrawFooter() {
    CONSOLE_SET_POS(39, 0);
    CONSOLE_CLEAR_LINE();
    printf(HRULE);
    CONSOLE_MOVE_LEFT(255);
    CONSOLE_MOVE_DOWN(1);

    CONSOLE_MOVE_DOWN(1);
    CONSOLE_CLEAR_LINE();
    if (!g_status_msg.empty()) {
        CONSOLE_SET_ATTRS(CONSOLE_ATTR_NONE);
        CONSOLE_SET_COLOR(CONSOLE_COLOR_FG_YELLOW);
        printf(g_status_msg.c_str());
        CONSOLE_SET_COLOR(CONSOLE_COLOR_FG_WHITE);
        CONSOLE_SET_ATTRS(CONSOLE_ATTR_BOLD);
    }
    CONSOLE_MOVE_LEFT(255);

    CONSOLE_MOVE_DOWN(1);
    CONSOLE_CLEAR_LINE();

    CONSOLE_MOVE_DOWN(1);
    CONSOLE_CLEAR_LINE();
    CONSOLE_SET_COLOR(CONSOLE_COLOR_FG_GREEN);
    printf("(Up/Down) Navigate  |  (A) Toggle Mod  |  (Y) (hold) Change Load Order");
    CONSOLE_MOVE_LEFT(255);
    CONSOLE_MOVE_DOWN(1);
    printf("(-) Save Changes    |  (+) Exit");
    CONSOLE_SET_COLOR(CONSOLE_COLOR_FG_WHITE);
}

static void clearTempEffects(void) {
    g_dirty_warned = false;

    if (g_tmp_status) {
        g_status_msg = "";
        g_tmp_status = false;
        redrawFooter();
    }
}

int main(int argc, char **argv) {
    consoleInit(NULL);

    ModGui gui = ModGui(getGlobalModList(), HEADER_HEIGHT, CONSOLE_LINES - HEADER_HEIGHT - FOOTER_HEIGHT);

    int init_status = initialize();
    if (RC_SUCCESS(init_status)) {
        CONSOLE_CLEAR_SCREEN();

        redrawHeader();
        gui.redraw();
        redrawFooter();
    }

    bool edit_load_order = false;

    while (appletMainLoop()) {
        hidScanInput();

        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        u64 kUp = hidKeysUp(CONTROLLER_P1_AUTO);

        if (kDown & KEY_PLUS) {
            if (g_dirty && !g_dirty_warned) {
                g_status_msg = "Press (+) to exit without saving changes";
                g_tmp_status = true;
                g_dirty_warned = true;
                redrawFooter();
            } else {
                break;
            }
        }

        if (RC_FAILURE(init_status)) {
            consoleUpdate(NULL);
            continue;
        }

        if (kDown & g_key_edit_lo) {
            edit_load_order = true;
            g_status_msg = "Editing load order";
            redrawFooter();
        }
        
        if (kUp & g_key_edit_lo) {
            edit_load_order = false;
            g_status_msg = "";
            redrawFooter();
        }

        if (kDown & KEY_DOWN) {
            if (edit_load_order) {
                if (gui.getSelectedIndex() < getGlobalModList().size() - 1) {
                    gui.getSelectedMod()->loadLater();
                    g_dirty = true;
                }
            }

            gui.scrollSelection(1);

            clearTempEffects();
        } else if (kDown & KEY_UP) {
            if (edit_load_order) {
                if (gui.getSelectedIndex() > 0) {
                    gui.getSelectedMod()->loadSooner();
                    g_dirty = true;
                }
            }

            gui.scrollSelection(-1);

            clearTempEffects();
        } else if (kDown & KEY_A) {
            std::shared_ptr<SkyrimMod> mod = gui.getSelectedMod();
            switch (mod->getStatus()) {
                case ModStatus::ENABLED:
                    mod->disable();
                    break;
                case ModStatus::PARTIAL:
                case ModStatus::DISABLED:
                    mod->enable();
                    break;
                default:
                    PANIC();
            }
            g_dirty = true;

            gui.redrawCurrentRow();

            clearTempEffects();
        }

        if (kDown & KEY_MINUS) {
            //TODO: save changes
            g_dirty = false;

            g_status_msg = "Wrote changes to SDMC!";
            g_tmp_status = true;
            redrawFooter();
        }

        consoleUpdate(NULL);
    }

    consoleExit(NULL);
    return 0;
}
