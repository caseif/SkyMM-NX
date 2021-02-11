#include "item_mod.hpp"
#include <math.h>

void ModListItem::onFocusLost()
{
	g_prev_mod = this->getLabel();
	View::onFocusLost();
}
void ModListItem::onFocusGained()
{
	g_sel_mod = this->getLabel();
	View::onFocusGained();
}
