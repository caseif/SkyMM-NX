#include "console_helper.hpp"
#include "error_defs.hpp"
#include "mod.hpp"
#include "string_helper.hpp"

#include <string>

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
