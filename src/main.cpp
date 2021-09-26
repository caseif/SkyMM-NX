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

#include "main.hpp"

bool g_dirty = false;
bool g_dirty_warned = false;

std::string g_status_msg = VERSION;
std::string g_status_msg_old = VERSION;
bool g_tmp_status = false;

ModList g_mod_list_tmp;

std::string g_plugins_header;

int g_scroll_dir = 0;
u64 g_last_scroll_time = 0;
bool g_scroll_initial_cooldown;

bool g_edit_load_order = false;

bool g_plugin = false;

u64 _nanotime(void)
{
	return armTicksToNs(armGetSystemTick());
}

int discoverMods()
{
	DIR *dir = opendir(getRomfsPath(SKYRIM_DATA_DIR).c_str());

	if (!dir)
	{
		FATAL(("sky/fatal/no_data"_i18n).c_str(), getBaseRomfsPath());
		return -1;
	}

	std::vector<std::string> files;

	struct dirent *ent;
	while ((ent = readdir(dir)))
	{
		if (ent->d_type != DT_REG)
		{
			continue;
		}

		files.insert(files.end(), ent->d_name);
	}

	closedir(dir);
	brls::Logger::debug("Found {} mod files\n", files.size());

	for (std::string file_name : files)
	{
		ModFile mod_file = ModFile::fromFileName(file_name);

		if (mod_file.type == ModFileType::UNKNOWN)
		{
			continue;
		}

		std::shared_ptr<SkyrimMod> mod = find_mod(g_mod_list_tmp, mod_file.base_name);
		if (!mod)
		{
			mod = std::shared_ptr<SkyrimMod>(new SkyrimMod(mod_file.base_name));
			// everything gets loaded into a temp buffer so we can rebuild it with the proper order later
			g_mod_list_tmp.insert(g_mod_list_tmp.end(), mod);
		}

		if (mod_file.type == ModFileType::ESP)
		{
			mod->has_esp = true;
		}
		else if (mod_file.type == ModFileType::ESM)
		{
			mod->has_esp = true;
			mod->is_master = true;
		}
		else if (mod_file.type == ModFileType::BSA)
		{
			mod->bsa_suffixes.insert(mod->bsa_suffixes.end(), mod_file.suffix);
		}
		else
		{
			PANIC();
			return -1;
		}
	}

	return 0;
}

int processPluginsFile()
{
	std::ifstream plugins_stream = std::ifstream(getRomfsPath(SKYRIM_PLUGINS_FILE), std::ios::in);
	if (!plugins_stream.good())
	{
		FATAL(("sky/fatal/plugin"_i18n).c_str());
		return -1;
	}

	bool in_header = true;
	std::stringstream header_stream;
	std::string line;
	while (std::getline(plugins_stream, line))
	{
		if (line.length() == 0 || line.at(0) == '#')
		{
			if (in_header)
			{
				header_stream << line << '\n';
			}
			continue;
		}

		if (in_header)
		{
			g_plugins_header = header_stream.str();
			in_header = false;
		}

		bool enable = line.at(0) == '*';

		std::string file_name = enable ? line.substr(1) : line;
		ModFile file_def = ModFile::fromFileName(file_name);
		if (file_def.type != ModFileType::ESP && file_def.type != ModFileType::ESM)
		{
			continue;
		}

		std::shared_ptr<SkyrimMod> mod = find_mod(getGlobalModList(), file_def.base_name);
		if (!mod)
		{
			mod = find_mod(g_mod_list_tmp, file_def.base_name);
			if (mod)
			{
				getGlobalModList().insert(getGlobalModList().end(), mod);
			}
			else
			{
				continue;
			}
		}

		mod->esp_enabled = enable;
	}

	return 0;
}

int writePluginsFile(void)
{
	std::ofstream plugins_stream = std::ofstream(getRomfsPath(SKYRIM_PLUGINS_FILE),
												 std::ios::out | std::ios::trunc | std::ios::binary);

	if (!plugins_stream.good())
	{
		FATAL(("sky/fatal/plugin_atm"_i18n).c_str());
		return -1;
	}

	// write header that we loaded earlier
	plugins_stream << g_plugins_header;

	for (std::shared_ptr<SkyrimMod> mod : getGlobalModList())
	{
		if (mod->has_esp)
		{
			if (mod->esp_enabled)
			{
				plugins_stream << '*';
			}
			if (mod->is_master)
			{
				plugins_stream << mod->base_name << ".esm";
			}
			else
			{
				plugins_stream << mod->base_name << ".esp";
			}
			plugins_stream << '\n';
		}
	}
	if (g_plugin)
	{
		std::ofstream plugins_stream_smm = std::ofstream(smmModPathForCfwPath(getRomfsPath(SKYRIM_PLUGINS_FILE)),
														 std::ios::out | std::ios::trunc | std::ios::binary);

		if (!plugins_stream_smm.good())
		{
			FATAL(("sky/fatal/plugin_smm"_i18n).c_str());
			return -1;
		}

		// write header that we loaded earlier
		plugins_stream_smm << g_plugins_header;

		for (std::shared_ptr<SkyrimMod> mod : getGlobalModList())
		{
			if (mod->has_esp)
			{
				if (mod->esp_enabled)
				{
					plugins_stream_smm << '*';
				}
				if (mod->is_master)
				{
					plugins_stream_smm << mod->base_name << ".esm";
				}
				else
				{
					plugins_stream_smm << mod->base_name << ".esp";
				}
				plugins_stream_smm << '\n';
			}
		}
	}

	return 0;
}

int initialize(void)
{
	int rc;

	brls::Logger::debug("Discovering available mods...\n");

	if (RC_FAILURE(rc = discoverMods()))
	{
		return rc;
	}

	if (RC_FAILURE(rc = processPluginsFile()))
	{
		return rc;
	}

	if (RC_FAILURE(rc = parseInis(getGlobalModList(), g_mod_list_tmp)))
	{
		return rc;
	}

	for (std::shared_ptr<SkyrimMod> mod : g_mod_list_tmp)
	{
		std::string mod_name = mod->base_name;
		if (!find_mod(getGlobalModList(), mod_name))
		{
			getGlobalModList().insert(getGlobalModList().end(), mod);
		}
	}

	brls::Logger::debug("Identified {} mods\n", getGlobalModList().size());

	brls::Logger::debug("Mod listing:\n\n");
	for (auto it = getGlobalModList().cbegin(); it != getGlobalModList().cend(); it++)
	{
		ModStatus status = (*it)->getStatus();
		const char *status_str;
		switch (status)
		{
		case ModStatus::ENABLED:
			status_str = "Enabled";
			break;
		case ModStatus::DISABLED:
			status_str = "Disabled";
			break;
		case ModStatus::PARTIAL:
			status_str = "Partial";
			break;
		default:
			PANIC();
			return -1;
		}
		brls::Logger::debug("  - {} ({})\n", (*it)->base_name.c_str(), status_str);
	}

	return 0;
}

void clearTempEffects(void)
{
	g_dirty_warned = false;

	if (g_tmp_status)
	{
		g_status_msg = VERSION;
		g_tmp_status = false;
	}
}

int main(int argc, char **argv)
{
	Result rc;
	rc = nsInitialize();
	if (R_FAILED(rc))
	{
		brls::Logger::debug("nsInitialize Failed");
	}
	brls::i18n::loadTranslations();
	brls::Logger::setLogLevel(brls::LogLevel::DEBUG);
	if (not brls::Application::init("SkyMM-NX"))
	{
		FATAL(("sky/fatal/brls_init"_i18n).c_str());
	}
	frame_root *gui;

	int init_status = initialize();
	if (RC_SUCCESS(init_status))
	{
		gui = new frame_root();
		brls::Application::pushView(gui);
	}
	else
	{
		gui = new frame_root();
	}
	gui->registerAction(
		"sky/hints/save"_i18n, brls::Key::PLUS, [gui] {
			writePluginsFile();
			writeIniChanges();
			g_dirty = false;

			g_status_msg = "sky/msg/save"_i18n;
			g_tmp_status = true;
			return true;
		});
	while (brls::Application::mainLoop())
	{
		if (g_status_msg_old != g_status_msg)
		{
			g_status_msg_old = g_status_msg;
			gui->setFooterText(g_status_msg);
		}
	}
	return 0;
}
