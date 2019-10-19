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

#include "error_defs.hpp"
#include "ini_helper.hpp"
#include "mod.hpp"
#include "path_defs.hpp"
#include "string_helper.hpp"

#include <inipp/inipp.h>
#include <switch.h>

#include <fstream>
#include <memory>

static const std::vector<std::string> g_archive_types_1 = {"", "Animations", "Meshes", "Sounds"};
static const std::vector<std::string> g_archive_types_2 = {"Textures", "Voices"};
static const std::vector<std::string> g_archive_types_3 = {"Animaini_tions"};

static StdIni g_skyrim_ini;
static StdIni g_skyrim_lang_ini;

static inline const char *get_language_code(SetLanguage &lang) {
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

int readIniFile(std::string &path, StdIni &ini) {
    std::ifstream ini_stream = std::ifstream(path, std::ios::in);
    if (!ini_stream.good()) {
        FATAL("Failed to read file at %s", path.c_str());
        return -1;
    }

    ini.parse(ini_stream);

    return 0;
}

int readIniFile(const char *path, StdIni &ini) {
    std::string path_str = std::string(path);
    return readIniFile(path_str, ini);
}

int processIniDefs(ModList &final_mod_list, ModList &temp_mod_list, StdIni &ini, const char *key,
        const std::vector<std::string> &expected_suffixes) {
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

        std::shared_ptr<SkyrimMod> mod = find_mod(final_mod_list, mod_file.base_name);
        if (!mod) {
            mod = find_mod(temp_mod_list, mod_file.base_name);
            if (mod) {
                final_mod_list.insert(final_mod_list.end(), mod);
            } else {
                continue;
            }
        }

        mod->enabled_bsas[mod_file.suffix] += 1;
    }

    return 0;
}

int parseInis(ModList &final_mod_list, ModList &temp_mod_list) {
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

    processIniDefs(final_mod_list, temp_mod_list, g_skyrim_ini, INI_ARCHIVE_LIST_1, g_archive_types_1);
    processIniDefs(final_mod_list, temp_mod_list, g_skyrim_ini, INI_ARCHIVE_LIST_3, g_archive_types_3);
    processIniDefs(final_mod_list, temp_mod_list, g_skyrim_lang_ini, INI_ARCHIVE_LIST_2, g_archive_types_2);

    return 0;
}