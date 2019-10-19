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
#include "mod.hpp"
#include "string_helper.hpp"

#include "inipp/inipp.h"

#include <switch.h>

#include <algorithm>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <cstdio>
#include <dirent.h>

#define SKYRIM_ROMFS_DIR "sdmc:/atmosphere/titles/01000A10041EA000/romfs"
#define SKYRIM_DATA_DIR SKYRIM_ROMFS_DIR "/Data"
#define SKYRIM_INI_FILE SKYRIM_ROMFS_DIR "/Skyrim.ini"
#define SKYRIM_INI_LANG_FILE SKYRIM_ROMFS_DIR "/Skyrim_%s.ini"
#define SKYRIM_PLUGINS_FILE SKYRIM_ROMFS_DIR "/Plugins"

#define INI_SECTION_ARCHIVE "Archive"
#define INI_ARCHIVE_LIST_1 "sResourceArchiveList"
#define INI_ARCHIVE_LIST_2 "sResourceArchiveList2"
#define INI_ARCHIVE_LIST_3 "sArchiveToLoadInMemoryList"

#define LANG_CODE_MAX_LEN 6

static const std::vector<std::string> g_archive_types_1 = {"", "Animations", "Meshes", "Sounds"};
static const std::vector<std::string> g_archive_types_2 = {"Textures", "Voices"};
static const std::vector<std::string> g_archive_types_3 = {"Animaini_tions"};

static std::vector<std::shared_ptr<SkyrimMod>> g_mod_list;
static std::map<std::string, std::shared_ptr<SkyrimMod>> g_mod_map;
static inipp::Ini<char> g_skyrim_ini;
static inipp::Ini<char> g_skyrim_lang_ini;

inline const char *get_language_code(SetLanguage &lang) {
    switch (lang) {
        case SetLanguage_JA:
            return "ja";
        case SetLanguage_ENUS:
            return "en";
        case SetLanguage_FR:
            return "fr";
        case SetLanguage_DE:
            return "de";
        case SetLanguage_IT:
            return "it";
        case SetLanguage_ES:
            return "es";
        case SetLanguage_ZHCN:
            return "zhhant";
        case SetLanguage_KO:
            return "en";
        case SetLanguage_NL:
            return "en";
        case SetLanguage_PT:
            return "en";
        case SetLanguage_RU:
            return "ru";
        case SetLanguage_ZHTW:
            return "zhhant";
        case SetLanguage_ENGB:
            return "en";
        case SetLanguage_FRCA:
            return "fr";
        case SetLanguage_ES419:
            return "es";
        case 15:
            return "zhhant";
        case 16:
            return "zhhant";
        default:
            return "en";
    }
}

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

        std::shared_ptr<SkyrimMod> mod;
        auto mod_it = g_mod_map.find(mod_file.base_name);
        if (mod_it != g_mod_map.cend()) {
            mod = mod_it->second;
        } else {
            mod = std::shared_ptr<SkyrimMod>(new SkyrimMod(mod_file.base_name));
            g_mod_list.insert(g_mod_list.end(), mod);
            g_mod_map.insert(g_mod_map.end(), std::pair(mod_file.base_name, mod));
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

template <typename CharT>
int readIniFile(std::string &path, inipp::Ini<CharT> &ini) {
    std::ifstream ini_stream = std::ifstream(path, std::ios::in);
    if (!ini_stream.good()) {
        FATAL("Failed to read file at %s", path.c_str());
        return -1;
    }

    ini.parse(ini_stream);

    return 0;
}

template <typename CharT>
int readIniFile(const char *path, inipp::Ini<CharT> &ini) {
    std::string path_str = std::string(path);
    return readIniFile(path_str, ini);
}

template <typename CharT>
int processIniDefs(inipp::Ini<CharT> &ini, const char *key, const std::vector<std::string> &expected_suffixes) {
    auto archive_sec_it = ini.sections.find(INI_SECTION_ARCHIVE);
    if (archive_sec_it == ini.sections.cend()) {
        return 0;
    }

    auto sec = archive_sec_it->second;
    auto archive_list_it = sec.find(std::string(key));
    if (archive_list_it == sec.cend()) {
        return 0;
    }

    std::string archive_list_str = archive_list_it->second;
    std::vector<std::string> archive_list = split(archive_list_str, ",");
    
    for (std::string archive_file : archive_list) {
        ModFile mod_file = ModFile::fromFileName(archive_file);
        if (mod_file.type != ModFileType::BSA) {
            continue;
        }

        if (mod_file.base_name == "Skyrim") {
            continue;
        }

        bool good_suffix = false;
        for (std::string expected_suffix : expected_suffixes) {
            if (mod_file.suffix.find_last_of(expected_suffix, expected_suffix.size())) {
                good_suffix = true;
                break;
            }
        }
        if (!good_suffix) {
            continue;
        }

        auto mod_it = g_mod_map.find(mod_file.base_name);
        if (mod_it == g_mod_map.cend()) {
            continue;
        }

        std::shared_ptr<SkyrimMod> mod = mod_it->second;

        mod->enabled_bsas[mod_file.suffix] += 1;
    }

    return 0;
}

int parseInis() {
    int rc;

    u64 lang_code;

    DO_OR_DIE(rc, setInitialize(), "Failed to initialize settings");

    DO_OR_DIE(rc, setGetSystemLanguage(&lang_code), "Failed to get system language");

    SetLanguage lang;
    DO_OR_DIE(rc, setMakeLanguage(lang_code, reinterpret_cast<s32*>(&lang)), "Failed to convert language code");

    const char *skyrim_lang_code = get_language_code(lang);

    char ini_lang_file[sizeof(SKYRIM_INI_LANG_FILE) - 2 + LANG_CODE_MAX_LEN + 1];
    sprintf(ini_lang_file, SKYRIM_INI_LANG_FILE, skyrim_lang_code);

    DO_OR_DIE(rc, readIniFile(SKYRIM_INI_FILE, g_skyrim_ini), "Failed to read Skyrim.ini");
    DO_OR_DIE(rc, readIniFile(ini_lang_file, g_skyrim_lang_ini), "Failed to read Skyrim_%s.ini", skyrim_lang_code);

    processIniDefs(g_skyrim_ini, INI_ARCHIVE_LIST_1, g_archive_types_1);
    processIniDefs(g_skyrim_ini, INI_ARCHIVE_LIST_3, g_archive_types_3);
    processIniDefs(g_skyrim_lang_ini, INI_ARCHIVE_LIST_2, g_archive_types_2);

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

        auto mod_it = g_mod_map.find(file_def.base_name);
        if (mod_it == g_mod_map.cend()) {
            continue;
        }

        std::shared_ptr<SkyrimMod> mod = mod_it->second;
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

    printf("Identified %lu mods\n", g_mod_list.size());

    CONSOLE_MOVE_DOWN(3);
    printf("Mod listing:\n\n");
    for (auto it = g_mod_list.cbegin(); it != g_mod_list.cend(); it++) {
        ModStatus status = (it->get())->getStatus();
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
        printf("  - %s (%s)\n", it->get()->base_name.c_str(), status_str);
    }

    return 0;
}

int main(int argc, char **argv) {
    consoleInit(NULL);

    ModGui *gui;

    int init_status = initialize();
    if (RC_SUCCESS(init_status)) {
        gui = new ModGui(g_mod_list, 0, 50);
        CONSOLE_CLEAR_SCREEN();
        gui->redraw();
    }

    while (appletMainLoop()) {
        hidScanInput();

        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

        if (kDown & KEY_PLUS) {
            break;
        }

        if (RC_FAILURE(init_status)) {
            consoleUpdate(NULL);
            continue;
        }

        if (kDown & KEY_DOWN) {
            gui->scrollSelection(1);
        } else if (kDown & KEY_UP) {
            gui->scrollSelection(-1);
        } else if (kDown & KEY_A) {
            //TODO: toggle mod
        }

        consoleUpdate(NULL);
    }

    consoleExit(NULL);
    return 0;
}
