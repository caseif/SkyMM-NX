#include "tab_mods.hpp"
std::string g_prev_mod;
std::string g_sel_mod;
bool g_dialog_open;
tab_mods::tab_mods()
{
	setModsTabPtr(this);
	this->triggerUpdateListItems = false;
	this->triggerUpdateModsDisplayedStatus = false;
	this->frameCounter = -1;
	this->clear();
	// Setup the list
	auto mod_items_list = getGlobalModList();
	for (int i_mod = 0; i_mod < int(mod_items_list.size()); i_mod++)
	{
		std::string selected_mod = mod_items_list[i_mod].get()->base_name;
		brls::Logger::debug("Adding mod: {}", selected_mod.c_str());
		ModListItem *item = new ModListItem(selected_mod, "", "");
		item->getClickEvent()->subscribe([this, item](View *view) {
			if (!g_dialog_open)
			{
				auto *dialog = new brls::Dialog(fmt::format("sky/dialog/enable"_i18n, item->getLabel()));

				dialog->addButton("sky/dialog/yes"_i18n, [item, dialog, this](brls::View *view) {
					std::shared_ptr<SkyrimMod> mod = find_mod(getGlobalModList(), item->getLabel());
					switch (mod->getStatus())
					{
					case ModStatus::ENABLED:
					{
						mod->enable();
						g_dialog_open = false;
						dialog->close();
						break;
					}
					case ModStatus::PARTIAL:
					{
						mod->enable();
						g_dialog_open = false;
						dialog->close();
						break;
					}
					case ModStatus::DISABLED:
					{
						mod->enable();
						g_dialog_open = false;
						dialog->close();
						break;
					}
					default:
						break;
					}
					this->triggerUpdateModsDisplayedStatus = true;
					g_dirty = true;
					clearTempEffects();
				});
				dialog->addButton("sky/dialog/no"_i18n, [dialog](brls::View *view) {
					g_dialog_open = false;
					dialog->close();
				});
				dialog->registerAction("brls/hints/back"_i18n, Key::B, [dialog] { g_dialog_open = false; return dialog->onCancel(); });
				dialog->setCancelable(true);
				g_dialog_open = true;
				dialog->open();
			}

			return true;
		});
		item->updateActionHint(brls::Key::A, "sky/hints/enable"_i18n);

		item->registerAction("sky/hints/disable"_i18n, brls::Key::X, [this, item] {
			if (!g_dialog_open)
			{
				auto *dialog = new brls::Dialog("sky/dialog/disable1"_i18n + item->getLabel() + "sky/dialog/disable2"_i18n);

				dialog->addButton("sky/dialog/yes"_i18n, [item, dialog, this](brls::View *view) {
					std::shared_ptr<SkyrimMod> mod = find_mod(getGlobalModList(), item->getLabel());
					switch (mod->getStatus())
					{
					case ModStatus::ENABLED:
					{
						mod->disable();
						g_dialog_open = false;
						dialog->close();
						break;
					}
					case ModStatus::PARTIAL:
					{
						mod->disable();
						g_dialog_open = false;
						dialog->close();
						break;
					}
					case ModStatus::DISABLED:
					{
						mod->disable();
						g_dialog_open = false;
						dialog->close();
						break;
					}
					default:
						break;
					}
					this->triggerUpdateModsDisplayedStatus = true;
					g_dirty = true;
					clearTempEffects();
				});
				dialog->addButton("sky/dialog/no"_i18n, [dialog](brls::View *view) {
					g_dialog_open = false;
					dialog->close();
				});
				dialog->registerAction("brls/hints/back"_i18n, Key::B, [dialog] { g_dialog_open = false; return dialog->onCancel(); });
				dialog->setCancelable(true);
				g_dialog_open = true;
				dialog->open();
			}
			return true;
		});

		this->addView(item);
		_modsListItems_.push_back(item);
		this->setTriggerUpdateModsDisplayedStatus(true);
	}

	if (mod_items_list.empty())
	{

		auto *emptyListLabel = new brls::ListItem(fmt::format("sky/msg/missing"_i18n, getRomfsPath("Data")));
		emptyListLabel->show([]() {}, false);
		this->addView(emptyListLabel);
	}
	else
	{
		this->registerAction(
			"sky/hints/edit"_i18n, brls::Key::Y, [] {
				if (g_edit_load_order)
				{
					g_status_msg = VERSION;
					g_edit_load_order = false;
				}
				else
				{
					g_status_msg = "sky/msg/edit"_i18n;
					g_edit_load_order = true;
				}
				return true;
			});
	}
}
void tab_mods::updateListItems()
{
	for (auto it = getModsListItems().begin(); it != getModsListItems().end(); it++)
	{
		(*it)->setLabel(getGlobalModList().at(std::distance(getModsListItems().begin(), it)).get()->base_name);
	}
	this->setTriggerUpdateModsDisplayedStatus(true);
}
std::vector<ModListItem *> &tab_mods::getModsListItems()
{
	return _modsListItems_;
}
void tab_mods::onChildFocusLost(View *child)
{
	View::onChildFocusLost(child);
}

void tab_mods::onChildFocusGained(View *child)
{
	if (g_edit_load_order)
	{
		ModList::iterator old_i = getGlobalModList().end();
		ModList::iterator new_i = getGlobalModList().end();
		for (auto it = getGlobalModList().begin(); it != getGlobalModList().end(); it++)
		{
			if ((*it)->base_name == g_prev_mod)
			{
				old_i = it;
			}
			if ((*it)->base_name == g_sel_mod)
			{
				new_i = it;
			}
			if (old_i != getGlobalModList().end() && new_i != getGlobalModList().end())
			{
				break;
			}
		}
		std::iter_swap(old_i, new_i);
		g_dirty = true;
		this->setTriggerUpdateListItems(true);
		g_edit_load_order = false;
		g_status_msg = VERSION;
	}
	ScrollView::onChildFocusGained(child);
}
void tab_mods::draw(NVGcontext *vg, int x, int y, unsigned int width, unsigned int height, brls::Style *style,
					brls::FrameContext *ctx)
{

	ScrollView::draw(vg, x, y, width, height, style, ctx);

	if (this->triggerUpdateModsDisplayedStatus)
	{
		this->updateDisplayedModsStatus();
		this->triggerUpdateModsDisplayedStatus = false;
	}

	if (this->triggerUpdateListItems)
	{
		this->updateListItems();
		this->triggerUpdateListItems = false;
	}
}

void tab_mods::setTriggerUpdateModsDisplayedStatus(bool triggerUpdateModsDisplayedStatus_)
{
	tab_mods::triggerUpdateModsDisplayedStatus = triggerUpdateModsDisplayedStatus_;
}

void tab_mods::setTriggerUpdateListItems(bool triggerUpdateListItems_)
{
	tab_mods::triggerUpdateListItems = triggerUpdateListItems_;
}
void tab_mods::updateDisplayedModsStatus()
{
	for (auto it = getModsListItems().begin(); it != getModsListItems().end(); it++)
	{
		std::string reference_str = (*it)->getLabel();
		std::shared_ptr<SkyrimMod> mod = find_mod(getGlobalModList(), reference_str);
		NVGcolor color;
		switch (mod->getStatus())
		{
		case ModStatus::ENABLED:
			(*it)->setValue("sky/status/enabled"_i18n);
			color = nvgRGB(88, 195, 169);
			break;
		case ModStatus::PARTIAL:
			(*it)->setValue("sky/status/partial"_i18n);
			color = nvgRGB(245 * 0.85, 198 * 0.85, 59 * 0.85);
			break;
		case ModStatus::DISABLED:
			(*it)->setValue("sky/status/disabled"_i18n);
			color = nvgRGB(80, 80, 80);
			break;
		}
		(*it)->setValueActiveColor(color);
	}
}