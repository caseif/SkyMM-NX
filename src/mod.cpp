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

#include "console_helper.hpp"
#include "error_defs.hpp"
#include "mod.hpp"
#include "string_helper.hpp"

#include <algorithm>
#include <memory>
#include <string>

static ModList g_mod_list;

ModFile ModFile::fromFileName(std::string const &file_name) {
    size_t dot_index = file_name.find_last_of('.');
    if (dot_index == std::string::npos) {
        return {ModFileType::UNKNOWN};
    }

    std::string base = file_name.substr(0, dot_index);
    std::string ext = file_name.substr(dot_index + 1);
    std::string suffix;

    base = trim(base);
    ext = trim(ext);

    if (ext == EXT_BSA) {
        size_t dash_index = base.rfind(" - ");
        if (dash_index == std::string::npos) {
            suffix = "";
        } else {
            suffix = base.substr(dash_index + 3);
            base = base.substr(0, dash_index);
        }
    } else if (ext == EXT_ESP || ext == EXT_ESM) {
        suffix = "";
    } else {
        return {ModFileType::UNKNOWN};
    }

    ModFileType type;
    if (ext == EXT_ESM) {
        type = ModFileType::ESM;
    } else if (ext == EXT_ESP) {
        type = ModFileType::ESP;
    } else if (ext == EXT_BSA) {
        type = ModFileType::BSA;
    } else {
        type = ModFileType::UNKNOWN;
    }

    return {type, base, suffix};
}

ModStatus SkyrimMod::getStatus(void) {
    bool esp_status = has_esp ? esp_enabled : true;
    
    ModStatus bsa_status;
    if (bsa_suffixes.size() > 0 && enabled_bsas.size() == 0) {
        bsa_status = ModStatus::DISABLED;
    } else if (bsa_suffixes.size() != enabled_bsas.size()) {
        bsa_status = ModStatus::PARTIAL;
    } else {
        bool bad_anims = false;
        for (auto bsa_pair : enabled_bsas) {
            if (bsa_pair.first.find(SUFFIX_ANIMATIONS) == 0 && bsa_pair.second != 2) {
                bsa_status = ModStatus::PARTIAL;
                bad_anims = true;
                break;
            }
        }

        if (!bad_anims) {
            bsa_status = ModStatus::ENABLED;
        }
    }

    switch (bsa_status) {
        case ModStatus::PARTIAL:
            return ModStatus::PARTIAL;
        case ModStatus::DISABLED:
            return (esp_status && has_esp) ? ModStatus::PARTIAL : ModStatus::DISABLED;
        case ModStatus::ENABLED:
            return esp_status ? ModStatus::ENABLED : (bsa_suffixes.empty() ? ModStatus::DISABLED : ModStatus::PARTIAL);
        default:
            PANIC();
            return ModStatus::DISABLED;
    }
}

void SkyrimMod::enable(void) {
    enabled_bsas.clear();
    for (std::string bsa : bsa_suffixes) {
        int count = bsa.find(SUFFIX_ANIMATIONS) == 0 ? 2 : 1;
        enabled_bsas.insert(std::pair(bsa, count));
    }

    if (has_esp) {
        esp_enabled = true;
    }
}

void SkyrimMod::disable(void) {
    enabled_bsas.clear();
    esp_enabled = false;
}

void SkyrimMod::loadSooner(void) {
    for (auto it = g_mod_list.begin(); it != g_mod_list.end(); it++) {
        if ((*it)->base_name == this->base_name) {
            if (it == g_mod_list.begin()) {
                // mod is already first, nothing to do
                return;
            } else {
                // swap it with the previous mod
                std::iter_swap(it, it - 1);
                return;
            }
        }
    }
}

void SkyrimMod::loadLater(void) {
    for (auto it = g_mod_list.begin(); it != g_mod_list.end(); it++) {
        if ((*it)->base_name == this->base_name) {
            if (it == g_mod_list.end() - 1) {
                // mod is already last, nothing to do
                return;
            } else {
                // swap it with the next mod
                std::iter_swap(it, it + 1);
                return;
            }
        }
    }
}

ModList &getGlobalModList(void) {
    return g_mod_list;
}

std::shared_ptr<SkyrimMod> find_mod(ModList const &mod_list, std::string const &name) {
    std::shared_ptr<SkyrimMod> mod;
    for (std::shared_ptr<SkyrimMod> entry : mod_list) {
        if (entry->base_name == name) {
            mod = entry;
            break;
        }
    }
    return mod;
}
