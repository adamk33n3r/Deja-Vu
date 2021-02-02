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

	static Playlist selectedPlaylist = Playlist::NONE;

	//ImGui::ListBox("Fruit", &this->gui_selectedPlaylist, this->playlists, 2);
	//ImGui::ListBox("Playlist Filter", &this->gui_selectedPlaylist, this->playlists, IM_ARRAYSIZE(this->playlists));
	//ImGui::Combo("Playlist Filter", &this->gui_selectedPlaylist, this->playlists, IM_ARRAYSIZE(this->playlists));

	const char* preview = (selectedPlaylist != Playlist::NONE) ? PlaylistNames[selectedPlaylist].c_str() : "Select...";

	if (ImGui::BeginCombo("Playlist Filter", preview))
	{
		for (auto it = PlaylistNames.begin(); it != PlaylistNames.end(); ++it)
		{
			bool isSelected = selectedPlaylist == it->first;
			if (ImGui::Selectable(it->second.c_str(), isSelected))
				selectedPlaylist = it->first;
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	static char nameFilter[61] = "";
	ImGui::InputText("Name", nameFilter, IM_ARRAYSIZE(nameFilter), ImGuiInputTextFlags_AutoSelectAll);

	//commented out line 60 to test scrolling option 
	//ImGui::BeginChild("#DejaVuDataDisplay", ImVec2(55 + 250 + 55 + 250 + 55, -ImGui::GetFrameHeightWithSpacing()));
	ImGui::BeginChild("##ScrollingRegion", ImVec2(0, ImGui::GetFontSize() * 20), false, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar);

	//ImGui::ListBoxHeader()
	ImGui::Columns(5, "dejavu_stats"); // 5-ways, with border
	//ImGui::SetColumnWidth(0, 55);
	//ImGui::SetColumnWidth(1, 250);
	//ImGui::SetColumnWidth(2, 55);
	//ImGui::SetColumnWidth(3, 250);
	//ImGui::SetColumnWidth(4, 55);
	ImGui::Separator();
	ImGui::Text("Name"); ImGui::NextColumn();
	ImGui::Text("Met Count"); ImGui::NextColumn();
	ImGui::Text("Total Record With"); ImGui::NextColumn();
	ImGui::Text("Total Record Against"); ImGui::NextColumn();
	ImGui::Text("Player Notes"); ImGui::NextColumn();
	ImGui::Separator();

	std::string selectedPlaylistIDStr = std::to_string(static_cast<int>(selectedPlaylist));

	int i = 0;
	auto nameFilterView = std::string_view(nameFilter);
	for (auto& player : this->data["players"].items())
	{
		std::string uniqueID = player.key();
		json::value_type playerData = player.value();
		int metCount = playerData["metCount"].get<int>();
		std::string name = playerData["name"].get<std::string>();

		// Skip if doesn't have selected playlist data
		if (selectedPlaylist != Playlist::NONE && (!playerData["playlistData"].contains(selectedPlaylistIDStr) || playerData["playlistData"][selectedPlaylistIDStr]["records"].is_null()))
			continue;

		bool nameFound = std::search(name.begin(), name.end(), nameFilterView.begin(), nameFilterView.end(), [](char ch1, char ch2) {
			return std::toupper(ch1) == std::toupper(ch2);
		}) == name.end();
		if (nameFilterView.size() > 0 && nameFound)
			continue;

		auto sameRecord = GetRecord(uniqueID, selectedPlaylist, Side::Same);
		auto otherRecord = GetRecord(uniqueID, selectedPlaylist, Side::Other);

		ImGui::Text(name.c_str()); ImGui::NextColumn();
		ImGui::Text(std::to_string(metCount).c_str()); ImGui::NextColumn();
		std::ostringstream sameRecordFormatted;
		sameRecordFormatted << sameRecord.wins << ":" << sameRecord.losses;
		ImGui::Text(sameRecordFormatted.str().c_str()); ImGui::NextColumn();
		std::ostringstream otherRecordFormatted;
		otherRecordFormatted << otherRecord.wins << ":" << otherRecord.losses;
		ImGui::Text(otherRecordFormatted.str().c_str()); ImGui::NextColumn();
		std::vector<std::string> playerNotes(this->data.size());
		ImGui::InputText((std::string("##input text") + std::to_string(i)).c_str(), str1, IM_ARRAYSIZE(str1)); ImGui::NextColumn();
		ImGui::Separator();
		i++;
		//"create them beforehand, and save them, and reuse them...I believe in you" (when window is opened, create
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
	ImGui::GetIO().ConfigWindowsResizeFromEdges = true;
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
	this->playerNotes.resize(this->data.size());
}

void DejaVu::OnClose()
{
	this->isWindowOpen = false;
}
#endif
