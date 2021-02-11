#pragma once

#include <borealis.hpp>
#include <tab_mods.hpp>
#include <global.hpp>
#include "mod.hpp"

class frame_root : public brls::TabFrame
{

public:
	frame_root();

	bool onCancel() override;
};
