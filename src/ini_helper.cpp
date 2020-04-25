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

#include "error_defs.hpp"
#include "ini_helper.hpp"
#include "mod.hpp"
#include "path_helper.hpp"
#include "string_helper.hpp"

#include <inipp/inipp.h>
#include <switch.h>

#include <algorithm>
#include <fstream>
#include <memory>

static const std::vector<std::string> g_archive_types_1 = {"", "Animations", "Meshes", "Sounds"};
static const std::vector<std::string> g_archive_types_2 = {"Textures", "Voices"};
static const std::vector<std::string> g_archive_types_3 = {"Animations"};

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

static std::string getString(StdIni &ini, std::string section, std::string key) {
    auto sec = ini.sections.find(section);
    if (sec != ini.sections.cend()) {
        auto val_it = sec->second.find(key);
        if (val_it != sec->second.cend()) {
            return val_it->second;
        }
    }
    return "";
}

static int getLanguage(SetLanguage *lang) {
    int rc;
    u64 lang_code;
    DO_OR_DIE(rc, setInitialize(), "Failed to initialize settings");
    DO_OR_DIE(rc, setGetSystemLanguage(&lang_code), "Failed to get system language");
    DO_OR_DIE(rc, setMakeLanguage(lang_code, lang), "Failed to convert language code");
    return 0;
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
    std::string archive_list_str = getString(ini, INI_SECTION_ARCHIVE, key);
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
    SetLanguage lang;
    if (RC_FAILURE(getLanguage(&lang))) {
        return -1;
    }
    const char *skyrim_lang_code = get_language_code(lang);

    std::string ini_lang_base = std::string(SKYRIM_INI_LANG_FILE_PREFIX) + skyrim_lang_code + ".ini";
    std::string ini_lang_file = getRomfsPath(ini_lang_base);

    DO_OR_DIE(rc, readIniFile(getRomfsPath(SKYRIM_INI_FILE).c_str(), g_skyrim_ini), "Failed to read Skyrim.ini");
    DO_OR_DIE(rc, readIniFile(ini_lang_file, g_skyrim_lang_ini), "Failed to read Skyrim_%s.ini", skyrim_lang_code);

    processIniDefs(final_mod_list, temp_mod_list, g_skyrim_ini, INI_ARCHIVE_LIST_1, g_archive_types_1);
    processIniDefs(final_mod_list, temp_mod_list, g_skyrim_ini, INI_ARCHIVE_LIST_3, g_archive_types_3);
    processIniDefs(final_mod_list, temp_mod_list, g_skyrim_lang_ini, INI_ARCHIVE_LIST_2, g_archive_types_2);

    return 0;
}

static int writeFileList(const char *path, StdIni &ini, std::string key,
        std::vector<std::string> const &expected_suffixes) {
    std::vector<ModFile> file_list;

    std::string archive_list_str = getString(ini, INI_SECTION_ARCHIVE, key);
    std::vector<std::string> archive_list = split(archive_list_str, ",");
    std::vector<ModFile> cur_file_list;
    std::transform(archive_list.cbegin(), archive_list.cend(), std::back_inserter(cur_file_list), ModFile::fromFileName);

    for (ModFile file : cur_file_list) {
        if (file.base_name == "Skyrim") {
            file_list.insert(file_list.end(), file);
        }
    }

    for (std::shared_ptr<SkyrimMod> mod : getGlobalModList()) {
        for (std::pair<std::string, int> suffix_pair : mod->enabled_bsas) {
            for (std::string expected_suffix : expected_suffixes) {
                if (suffix_pair.first.find(expected_suffix) == 0) {
                    file_list.insert(file_list.end(), {ModFileType::ESP, mod->base_name, suffix_pair.first});
                    break;
                }
            }
        }
    }

    std::stringstream ss;
    for (auto mf_it = file_list.cbegin(); mf_it != file_list.cend(); mf_it++) {
        ss << mf_it->base_name;
        if (!mf_it->suffix.empty()) {
            ss << " - " << mf_it->suffix;
        }
        ss << ".bsa";

        if (mf_it != file_list.cend() - 1) {
            ss << ", ";
        }
    }

    std::string out_list_str = ss.str();

    auto sec_it = ini.sections.find(INI_SECTION_ARCHIVE);
    std::map<std::string, std::string> sec_map;
    if (sec_it != ini.sections.cend()) {
        sec_map = sec_it->second;
    }

    sec_map.insert_or_assign(key, out_list_str);

    ini.sections.insert_or_assign(INI_SECTION_ARCHIVE, sec_map);

    std::ofstream ini_stream(path, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!ini_stream.good()) {
        FATAL("Failed to open %s", path);
        return -1;
    }

    ini.generate(ini_stream);
    return 0;
}

int writeIniChanges(void) {
    SetLanguage lang;
    if (RC_FAILURE(getLanguage(&lang))) {
        return -1;
    }
    const char *skyrim_lang_code = get_language_code(lang);

    std::string ini_lang_base = std::string(SKYRIM_INI_LANG_FILE_PREFIX) + skyrim_lang_code + ".ini";
    std::string ini_lang_file = getRomfsPath(ini_lang_base);

    writeFileList(getRomfsPath(SKYRIM_INI_FILE).c_str(), g_skyrim_ini, INI_ARCHIVE_LIST_1, g_archive_types_1);
    writeFileList(ini_lang_file.c_str(), g_skyrim_lang_ini, INI_ARCHIVE_LIST_2, g_archive_types_2);
    writeFileList(getRomfsPath(SKYRIM_INI_FILE).c_str(), g_skyrim_ini, INI_ARCHIVE_LIST_3, g_archive_types_3);

    return 0;
}
