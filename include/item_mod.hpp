#pragma once

#include <borealis.hpp>
#include <borealis/list.hpp>
#include "mod.hpp"
#include "main.hpp"
#include "global.hpp"
using namespace brls;

class ModListItem : public ListItem
{

public:
	ModListItem(std::string label, std::string description = "", std::string subLabel = "") : ListItem(label, description, subLabel){};
	void onFocusLost();
	void onFocusGained();
};
