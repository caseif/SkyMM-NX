#pragma once

#include <inipp/inipp.h>

#define INI_SECTION_ARCHIVE "Archive"
#define INI_ARCHIVE_LIST_1 "sResourceArchiveList"
#define INI_ARCHIVE_LIST_2 "sResourceArchiveList2"
#define INI_ARCHIVE_LIST_3 "sArchiveToLoadInMemoryList"

typedef inipp::Ini<char> StdIni;

int readIniFile(std::string &path, StdIni &ini);

int readIniFile(const char *path, StdIni &ini);

int processIniDefs(StdIni &ini, const char *key, const std::vector<std::string> &expected_suffixes);

int parseInis(void);