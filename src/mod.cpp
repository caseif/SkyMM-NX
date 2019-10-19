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
#include "mod.hpp"
#include "string_helper.hpp"

#include <memory>
#include <string>

static std::vector<std::shared_ptr<SkyrimMod>> g_mod_list;

ModFile ModFile::fromFileName(std::string &file_name) {
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
    } else if (ext == EXT_ESP) {
        suffix = "";
    } else {
        return {ModFileType::UNKNOWN};
    }

    return {ext == EXT_BSA ? ModFileType::BSA : ModFileType::ESP, base, suffix};
}

ModStatus SkyrimMod::getStatus(void) {
    bool esp_status = this->has_esp ? this->esp_enabled : true;
    
    ModStatus bsa_status;
    if (this->bsa_suffixes.size() > 0 && this->enabled_bsas.size() == 0) {
        bsa_status = ModStatus::DISABLED;
    } else if (this->bsa_suffixes.size() != this->enabled_bsas.size()) {
        bsa_status = ModStatus::PARTIAL;
    } else {
        auto anim_it = this->enabled_bsas.find("Animations");
        if (anim_it != this->enabled_bsas.cend()) {
            // check if Animations BSA is present in both sResourceArchiveList and sResourceToLoadInMemoryList
            bsa_status = anim_it->second == 2 ? ModStatus::ENABLED : ModStatus::PARTIAL;
        } else {
            bsa_status = ModStatus::ENABLED;
        }
    }

    switch (bsa_status) {
        case ModStatus::PARTIAL:
            return ModStatus::PARTIAL;
        case ModStatus::DISABLED:
            return (esp_status && this->has_esp) ? ModStatus::PARTIAL : ModStatus::DISABLED;
        case ModStatus::ENABLED:
            return esp_status ? ModStatus::ENABLED : ModStatus::PARTIAL;
        default:
            PANIC();
            return ModStatus::DISABLED;
    }
}

std::vector<std::shared_ptr<SkyrimMod>> &get_global_mod_list(void) {
    return g_mod_list;
}

std::shared_ptr<SkyrimMod> find_mod(std::string &name) {
    std::shared_ptr<SkyrimMod> mod;
    for (std::shared_ptr<SkyrimMod> entry : get_global_mod_list()) {
        if (entry->base_name == name) {
            mod = entry;
            break;
        }
    }
    return mod;
}
