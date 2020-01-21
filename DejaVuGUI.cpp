#include "DejaVu.h"
#include "vendor\easyloggingpp-9.96.7\src\easylogging++.h"

#if ENABLE_GUI
void DejaVu::Render()
{

	if (!this->isWindowOpen) {
		cvarManager->executeCommand("togglemenu " + GetMenuName());

		return;
	}

	//ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
	//ImGui::SetNextWindowSize(ImVec2(256, 384), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(ImVec2(55 + 250 + 55 + 250 + 80 + 100, 600), ImVec2(FLT_MAX, FLT_MAX));

	ImGuiWindowFlags windowFlags = 0
		| ImGuiWindowFlags_MenuBar
		| ImGuiWindowFlags_ResizeFromAnySide
		//| ImGuiWindowFlags_NoCollapse
	;

	if (!ImGui::Begin(GetMenuTitle().c_str(), &this->isWindowOpen, windowFlags))
	{
		// Early out if the window is collapsed, as an optimization.
		this->shouldBlockInput = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
		ImGui::End();
		return;
	}

	//ImGui::SetNextWindowSizeConstraints(ImVec2(55 + 250 + 55 + 250 + 80 + 100, 600), ImVec2(FLT_MAX, FLT_MAX));
	//ImGui::Begin(menuTitle.c_str(), &isWindowOpen, ImGuiWindowFlags_NoCollapse);
	//const char* playlists[] = {"asdf", "fff"};


	static const char* items[]{"Apple", "Banana", "Orange"};
	static int selected = -1;

	//ImGui::ListBox("Fruit", &this->gui_selectedPlaylist, this->playlists, 2);
	//ImGui::ListBox("Playlist Filter", &this->gui_selectedPlaylist, this->playlists, IM_ARRAYSIZE(this->playlists));
	//ImGui::Combo("Playlist Filter", &this->gui_selectedPlaylist, this->playlists, IM_ARRAYSIZE(this->playlists));

	static PlaylistFilter playlistFilters[]{
		{"Duel", 1},
		{"Doubles", 2},
		{"Standard", 3},
	};

	const char* preview = (selected >= 0) ? playlistFilters[selected].name.c_str() : "Select...";

	if (ImGui::BeginCombo("Playlist Filter", preview))
	{
		if (ImGui::Selectable("None", selected == -1))
			selected = -1;
		for (int i = 0; i < IM_ARRAYSIZE(playlistFilters); ++i) {
			bool isSelected = selected == i;
			if (ImGui::Selectable(playlistFilters[i].name.c_str(), isSelected))
				selected = i;
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	ImGui::BeginChild("#LoadedPluginsTab", ImVec2(55 + 250 + 55 + 250, -ImGui::GetFrameHeightWithSpacing()));


	//ImGui::ListBoxHeader()
	ImGui::Columns(4, "dejavu_stats"); // 4-ways, with border
	//ImGui::SetColumnWidth(0, 55);
	//ImGui::SetColumnWidth(1, 250);
	//ImGui::SetColumnWidth(2, 55);
	//ImGui::SetColumnWidth(3, 250);
	ImGui::Separator();
	ImGui::Text("Name"); ImGui::NextColumn();
	ImGui::Text("Met Count"); ImGui::NextColumn();
	ImGui::Text("Total Record With"); ImGui::NextColumn();
	ImGui::Text("Total Record Against"); ImGui::NextColumn();
	ImGui::Separator();

	int i = 0;
	for (auto player : this->data["players"].items())
	{
		std::string steamID = player.key();
		json::value_type playerData = player.value();
		int metCount = playerData["metCount"].get<int>();
		std::string name = playerData["name"].get<std::string>();

		// Skip if doesn't have selected playlist data
		if (!playerData["playlistData"].contains(std::to_string(27)) || playerData["playlistData"]["27"]["records"].is_null())
			continue;

		auto sameRecord = GetRecord(steamID, 27, Side::Same);
		auto otherRecord = GetRecord(steamID, 27, Side::Other);

		ImGui::Text(name.c_str()); ImGui::NextColumn();
		ImGui::Text(std::to_string(metCount).c_str()); ImGui::NextColumn();
		std::ostringstream sameRecordFormatted;
		sameRecordFormatted << sameRecord.wins << ":" << sameRecord.losses;
		ImGui::Text(sameRecordFormatted.str().c_str()); ImGui::NextColumn();
		std::ostringstream otherRecordFormatted;
		otherRecordFormatted << otherRecord.wins << ":" << otherRecord.losses;
		ImGui::Text(otherRecordFormatted.str().c_str()); ImGui::NextColumn();
		ImGui::Separator();
		i++;
		if (i > 10)
			break;
	}

	//if (ImGui::BeginMenuBar())
	//{
	//	if (ImGui::BeginMenu("File"))
	//	{
	//		if (ImGui::MenuItem("Open..", "Ctrl+O")) { /* Do stuff */ }
	//		if (ImGui::MenuItem("Save", "Ctrl+S")) { /* Do stuff */ }
	//		if (ImGui::MenuItem("Close", "Ctrl+W")) { this->isWindowOpen = false; }
	//		ImGui::EndMenu();
	//	}
	//	ImGui::EndMenuBar();
	//}
	ImGui::EndChild();


	this->shouldBlockInput = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
	ImGui::End();
}

std::string DejaVu::GetMenuName()
{
	return "dejavu";
}

std::string DejaVu::GetMenuTitle()
{
	return this->menuTitle;
}

void DejaVu::SetImGuiContext(uintptr_t ctx)
{
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

bool DejaVu::ShouldBlockInput()
{
	return this->shouldBlockInput;
}

bool DejaVu::IsActiveOverlay()
{
	return true;
}

void DejaVu::OnOpen()
{
	this->isWindowOpen = true;
	this->cvarManager->getCvar("cl_dejavu_log").setValue(true);
}

void DejaVu::OnClose()
{
	this->isWindowOpen = false;
}
#endif
