#pragma once

#include <borealis.hpp>
#include "console_helper.hpp"
#include "error_defs.hpp"
#include "frame_root.hpp"
#include "ini_helper.hpp"
#include "mod.hpp"
#include "path_helper.hpp"
#include "string_helper.hpp"

#include <inipp/inipp.h>
#include <switch.h>

#include <filesystem>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <cstdio>
#include <dirent.h>

#include <libsmm.h>
using namespace brls::i18n::literals;
#define Q(x) #x
#define QUOTE(x) Q(x)
#define VERSION QUOTE(__VERSION)

extern bool g_dirty;
extern bool g_dirty_warned;

extern std::string g_status_msg;
extern bool g_tmp_status;

extern ModList g_mod_list_tmp;

extern std::string g_plugins_header;

extern int g_scroll_dir;
extern u64 g_last_scroll_time;
extern bool g_scroll_initial_cooldown;

extern bool g_edit_load_order;
extern bool g_plugin;

extern std::string g_sel_mod;
extern std::string g_prev_mod;

extern bool g_dialog_open;

void clearTempEffects(void);
u64 _nanotime(void);