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

#define EXT_ESP "esp"
#define EXT_BSA "bsa"

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

#define FATAL(fmt, ...) CONSOLE_CLEAR_SCREEN(); \
                        CONSOLE_SET_COLOR(CONSOLE_COLOR_RED); \
                        printf("Fatal error: " ); \
                        CONSOLE_SET_COLOR(CONSOLE_COLOR_YELLOW); \
                        printf(fmt, ##__VA_ARGS__); \
                        CONSOLE_MOVE_DOWN(3); \
                        CONSOLE_MOVE_LEFT(99); \
                        CONSOLE_SET_COLOR(CONSOLE_COLOR_GREEN); \
                        printf("Press (+) to exit.\n")

#define FATAL_CODE(code, fmt, ...) FATAL(fmt " (code %d)", ##__VA_ARGS__, code)

#define PANIC() FATAL("Panic @ %s:%d\n\nPlease contact the developer", __FILE__, __LINE__)

#define RC_SUCCESS(rc) (rc == 0)
#define RC_FAILURE(rc) (rc != 0)

#define DO_OR_DIE(rc, task, fail_fmt, ...)  if ((rc = RC_FAILURE((task)))) { \
                                                FATAL_CODE(rc, fail_fmt, ##__VA_ARGS__); \
                                                return -1; \
                                            }

enum class ModStatus {
    ENABLED,
    DISABLED,
    PARTIAL
};

enum class ModFileType {
    BSA,
    ESP,
    UNKNOWN
};

struct ModFile {
    ModFileType type;
    std::string base_name;
    std::string suffix;
};

struct SkyrimMod {
    const std::string base_name;
    bool has_esp;
    bool esp_enabled;
    std::vector<std::string> bsa_suffixes;
    std::map<std::string, int> enabled_bsps;

    SkyrimMod(const std::string &base_name):
            base_name(base_name),
            has_esp(false),
            esp_enabled(false),
            bsa_suffixes(),
            enabled_bsps() {
    }
};

static const std::vector<std::string> g_archive_types_1 = {"", "Animations", "Meshes", "Sounds"};
static const std::vector<std::string> g_archive_types_2 = {"Textures", "Voices"};
static const std::vector<std::string> g_archive_types_3 = {"Animations"};

static std::map<std::string, std::shared_ptr<SkyrimMod>> g_mod_list;
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

ModFile parseModFileName(std::string file_name) {
    size_t dot_index = file_name.find_last_of('.');
    if (dot_index == std::string::npos) {
        return {ModFileType::UNKNOWN};
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
        return {ModFileType::UNKNOWN};
    }

    return {ext == EXT_BSA ? ModFileType::BSA : ModFileType::ESP, base, suffix};
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
        ModFile mod_file = parseModFileName(file_name);

        if (mod_file.type == ModFileType::UNKNOWN) {
            continue;
        } 

        std::shared_ptr<SkyrimMod> mod;
        auto mod_it = g_mod_list.find(mod_file.base_name);
        if (mod_it != g_mod_list.cend()) {
            mod = mod_it->second;
        } else {
            mod = std::shared_ptr<SkyrimMod>(new SkyrimMod(mod_file.base_name));
            g_mod_list.insert(g_mod_list.end(), std::pair(mod_file.base_name, mod));
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
    auto archive_sec_it = g_skyrim_ini.sections.find(INI_SECTION_ARCHIVE);
    if (archive_sec_it == g_skyrim_ini.sections.cend()) {
        return 0;
    }

    decltype(g_skyrim_ini)::Section sec = archive_sec_it->second;
    auto archive_list_it = sec.find(std::string(key));
    if (archive_list_it == sec.cend()) {
        return 0;
    }

    std::string archive_list_str = archive_list_it->second;
    std::vector<std::string> archive_list = split(archive_list_str, ",");
    
    for (std::string archive_file : archive_list) {
        ModFile mod_file = parseModFileName(archive_file);
        if (mod_file.type != ModFileType::BSA) {
            continue;
        }

        if (mod_file.base_name == "Skyrim") {
            continue;
        }

        bool good_suffix = false;
        for (std::string expected_suffix : expected_suffixes) {
            if (mod_file.suffix == expected_suffix) {
                good_suffix = true;
            }
        }
        if (!good_suffix) {
            continue;
        }

        auto mod_it = g_mod_list.find(mod_file.base_name);
        if (mod_it == g_mod_list.cend()) {
            continue;
        }

        std::shared_ptr<SkyrimMod> mod = mod_it->second;

        mod->enabled_bsps[mod_file.suffix] += 1;
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

ModStatus getModStatus(SkyrimMod const &mod) {
    bool esp_status = mod.has_esp ? mod.esp_enabled : true;
    
    ModStatus bsa_status;
    if (mod.bsa_suffixes.size() > 0 && mod.enabled_bsps.size() == 0) {
        bsa_status = ModStatus::DISABLED;
    } else if (mod.bsa_suffixes.size() != mod.enabled_bsps.size()) {
        bsa_status = ModStatus::PARTIAL;
    } else {
        auto anim_it = mod.enabled_bsps.find("Animations");
        if (anim_it != mod.enabled_bsps.cend()) {
            // check if Animations BSA is present in both sResourceArchiveList and sResourceToLoadInMemoryList
            bsa_status = anim_it->second == 2 ? ModStatus::ENABLED : ModStatus::PARTIAL;
        }
    }

    switch (bsa_status) {
        case ModStatus::PARTIAL:
            return ModStatus::PARTIAL;
        case ModStatus::DISABLED:
            return esp_status ? ModStatus::PARTIAL : ModStatus::DISABLED;
        case ModStatus::ENABLED:
            return esp_status ? ModStatus::ENABLED : ModStatus::PARTIAL;
        default:
            PANIC();
            return ModStatus::DISABLED;
    }
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

    printf("Identified %lu mods\n", g_mod_list.size());

    CONSOLE_MOVE_DOWN(3);
    printf("Mod listing:\n\n");
    for (auto it = g_mod_list.cbegin(); it != g_mod_list.cend(); it++) {
        ModStatus status = getModStatus(*(it->second.get()));
        const char *status_str;
        switch (status) {
            case ModStatus::ENABLED:
                status_str = "Enabled";
            case ModStatus::DISABLED:
                status_str = "Disabled";
            case ModStatus::PARTIAL:
                status_str = "Partial";
        }
        printf("  - %s (%s)\n", it->second->base_name.c_str(), status_str);
    }

    if ((rc = parseInis()) != 0) {
        return rc;
    }

    return 0;
}

int main(int argc, char **argv) {
    consoleInit(NULL);

    int init_status = initialize();

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
