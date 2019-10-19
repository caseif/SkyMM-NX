#pragma once

#include <map>
#include <string>
#include <vector>

#define EXT_ESP "esp"
#define EXT_BSA "bsa"

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

    static ModFile fromFileName(std::string &file_name);
};

struct SkyrimMod {
    const std::string base_name;
    bool has_esp;
    bool esp_enabled;
    std::vector<std::string> bsa_suffixes;
    std::map<std::string, int> enabled_bsas;

    SkyrimMod(const std::string &base_name):
            base_name(base_name),
            has_esp(false),
            esp_enabled(false),
            bsa_suffixes(),
            enabled_bsas() {
    }

    ModStatus getStatus(void);
};