#include <frame_root.hpp>

frame_root::frame_root()
{
	this->setTitle("SkyMM-NX");
	this->setFooterText(VERSION);
	this->setIcon("romfs:/images/icon.jpg");

	this->addTab("Mods", new tab_mods());
	//   this->addSeparator();
	//   this->addTab("Settings", new tab_general_settings());
	//   this->addTab("About", new tab_about());
}

bool frame_root::onCancel()
{

	auto *lastFocus = brls::Application::getCurrentFocus();

	bool onCancel = TabFrame::onCancel();

	if (lastFocus == brls::Application::getCurrentFocus())
	{
		if (g_dirty && !g_dirty_warned)
		{
			g_status_msg = "Press (+) to exit without saving changes";
			g_tmp_status = true;
			g_dirty_warned = true;
		}
		else
		{
			brls::Application::quit();
		}
	}

	return onCancel;
}