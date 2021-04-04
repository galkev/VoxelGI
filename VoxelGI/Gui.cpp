#include "Gui.h"

namespace VoxelGI
{
	std::string Gui::filename = "guisettings.xml";
	bool Gui::onLoad = false;
	bool Gui::onSave = false;
	bool Gui::saveRequested = false;

	ticpp::Document Gui::doc;
	ticpp::Element* Gui::curElement = nullptr;
}