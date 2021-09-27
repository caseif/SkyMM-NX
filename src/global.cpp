#include "global.hpp"
#include <frame_root.hpp>
#include <tab_mods.hpp>
static tab_mods *_currentTabModBrowserPtr_;

void setModsTabPtr(tab_mods *currentTabModBrowserPtr_)
{
	_currentTabModBrowserPtr_ = currentTabModBrowserPtr_;
}

tab_mods *getModsTabPtr()
{
	return _currentTabModBrowserPtr_;
}
