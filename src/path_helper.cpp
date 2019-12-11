#include "path_helper.hpp"

#include <switch.h>
#include <switch/runtime/hosversion.h>

#include <string>

static bool initted = false;
static bool newRomfsPath = false;

static void _init(void) {
    // AMS 0.10.0 for Horizon 9.1.0 changed the RomFS directory
    if (hosversionAtLeast(9, 1, 0)) {
        newRomfsPath = true;
    } else {
        newRomfsPath = false;
    }
}

const char *getBaseRomfsPath(void) {
    if (!initted) {
        _init();
    }

    if (newRomfsPath) {
        return SKYRIM_ROMFS_DIR;
    } else {
        return SKYRIM_ROMFS_DIR_OLD;
    }
}

std::string getRomfsPath(std::string &partial) {
    return getRomfsPath(partial.c_str());
}

std::string getRomfsPath(const char *partial) {
    if (!initted) {
        _init();
    }

    if (newRomfsPath) {
        return std::string(SKYRIM_ROMFS_DIR) + "/" + partial;
    } else {
        return std::string(SKYRIM_ROMFS_DIR_OLD) + "/" + partial;
    }
}
