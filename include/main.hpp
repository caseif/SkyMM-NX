#pragma once

#include "mod.hpp"

#include <memory>
#include <vector>

static std::vector<std::shared_ptr<SkyrimMod>> g_mod_list;

inline std::vector<std::shared_ptr<SkyrimMod>> &get_global_mod_list(void) {
    return g_mod_list;
}

std::shared_ptr<SkyrimMod> find_mod(std::string name);
