#include "DejaVu.h"
#include "vendor\easyloggingpp-9.96.7\src\easylogging++.h"
#include "vendor/imgui/imgui.h"
#include "vendor/imgui/imgui_stdlib.h"
#include <algorithm>
#undef max

#if ENABLE_GUI
void DejaVu::Render()
{

	if (!this->isWindowOpen) {
		cvarManager->executeCommand("togglemenu " + GetMenuName());

		return;
	}

	if (this->openQuickNote)
	{
#if DEV
		std::set<std::string> matchMetList = { "0" };
		auto curMatchGUID = GetMatchGUID();
		if (curMatchGUID.has_value())
			matchMetList = this->matchesMetLists[curMatchGUID.value()];
#else
		auto curMatchGUID = GetMatchGUID();
		if (!curMatchGUID.has_value())
		{
			this->openQuickNote = false;
			return;
		}

		const std::set<std::string>& matchMetList = this->matchesMetLists[curMatchGUID.value()];
#endif DEV
			
		ImGui::OpenPopup("LaunchQuickNoteModal");
		ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 4), ImGuiCond_Appearing);
		if (ImGui::BeginPopupModal("LaunchQuickNoteModal"))
		{
			float reserveHeight = ImGui::GetTextLineHeightWithSpacing() + ImGui::GetStyle().FramePadding.y * 2;
			ImGui::BeginChild("#dejavu_quick_note", ImVec2(0, -reserveHeight));
			ImGui::Columns(2, "Quick Note Edit");
			ImGui::SetColumnWidth(0, std::max(ImGui::GetColumnWidth(0), 200.0f));

			ImGui::Separator();
			ImGui::Text("Name"); ImGui::NextColumn();
			ImGui::Text("Player Note"); ImGui::NextColumn();
			ImGui::Separator();

			for (const auto& uniqueID : matchMetList)
			{
				auto& playerData = this->data["players"][uniqueID];
				ImGui::Text(playerData["name"].get<std::string>().c_str()); ImGui::NextColumn();

				float buttonPos = ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x;
				if (!playerData.contains("note"))
					playerData["note"] = "";
				float buttonWidth = 2 * ImGui::GetStyle().FramePadding.x + ImGui::CalcTextSize("Edit").x;
				ImGui::BeginChild((std::string("#note") + uniqueID).c_str(), ImVec2(ImGui::GetColumnWidth() - buttonWidth - 2 * ImGui::GetStyle().ItemSpacing.x, ImGui::GetFrameHeightWithSpacing()), false, ImGuiWindowFlags_NoScrollbar);
				ImGui::TextWrapped(playerData["note"].get<std::string>().c_str());
				ImGui::EndChild();
				ImGui::SameLine();
				ImGui::SetCursorPosX(buttonPos - buttonWidth);
				if (ImGui::Button((std::string("Edit##") + uniqueID).c_str(), ImVec2(0, ImGui::GetFrameHeightWithSpacing())))
				{
					ImGui::OpenPopup("Edit note");
					this->playersNoteToEdit = uniqueID;
				}

				ImGui::NextColumn();

			}

			RenderEditNoteModal();
			ImGui::EndChild();

			int escIdx = ImGui::GetIO().KeyMap[ImGuiKey_Escape];
			if (ImGui::Button("Close", ImVec2(120, 0)) || (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && escIdx >= 0 && ImGui::IsKeyPressed(escIdx)))
			{
				this->openQuickNote = false;
				ImGui::CloseCurrentPopup();
				CloseMenu();
			}

			ImGui::EndPopup();
		}

		this->shouldBlockInput = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
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

	static char nameFilter[65] = "";
	ImGui::InputText("Name", nameFilter, IM_ARRAYSIZE(nameFilter), ImGuiInputTextFlags_AutoSelectAll);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::BeginChild("#DejaVuDataDisplay", ImVec2(0, ImGui::GetTextLineHeight() + ImGui::GetStyle().ItemSpacing.y * 2 + ImGui::GetStyle().FramePadding.y), false, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysUseWindowPadding);

	//ImGui::ListBoxHeader()
	ImGui::Columns(5, "dejavu_stats");
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
	ImGui::Text("Player Note"); ImGui::NextColumn();
	ImGui::Separator();
	ImGui::EndChild();

	//ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().WindowPadding.y);
	ImGui::BeginChild("#DejaVuDataDisplayBody", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysUseWindowPadding);
	ImGui::Columns(5, "dejavu_stats_body");
	ImGui::Separator();

	std::string selectedPlaylistIDStr = std::to_string(static_cast<int>(selectedPlaylist));

	auto nameFilterView = std::string_view(nameFilter);

	int i = 0;
	this->playerIDsToDisplay.resize(this->data["players"].size());
	for (auto& player : this->data["players"].items())
	{
		if (selectedPlaylist != Playlist::NONE && (!player.value()["playlistData"].contains(selectedPlaylistIDStr) || player.value()["playlistData"][selectedPlaylistIDStr]["records"].is_null()))
			continue;
		std::string name;
		try {
			name = player.value()["name"].get<std::string>();
		}
		catch (const std::exception& e)
		{
			this->gameWrapper->Toast("DejaVu Error", "Check console/log for details");
			this->cvarManager->log(e.what());
			LOG(INFO) << e.what();
			continue;
		}
		bool nameFound = std::search(name.begin(), name.end(), nameFilterView.begin(), nameFilterView.end(), [](char ch1, char ch2) {
			return std::toupper(ch1) == std::toupper(ch2);
		}) == name.end();
		if (nameFilterView.size() > 0 && nameFound)
			continue;
		this->playerIDsToDisplay[i++] = player.key();
	}
	this->playerIDsToDisplay.resize(i);
	ImGuiListClipper clipper(this->playerIDsToDisplay.size());
	while (clipper.Step())
	{
		for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
		{
			std::string uniqueID = this->playerIDsToDisplay[i];
			auto& playerData = this->data["players"][uniqueID];

			int metCount = playerData["metCount"].get<int>();
			std::string name = playerData["name"].get<std::string>();

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

			float buttonPos = ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x;
			if (!playerData.contains("note"))
				playerData["note"] = "";
			float buttonWidth = 2 * ImGui::GetStyle().FramePadding.x + ImGui::CalcTextSize("Edit").x;
			ImGui::BeginChild((std::string("#note") + uniqueID).c_str(), ImVec2(ImGui::GetColumnWidth() - buttonWidth - 2 * ImGui::GetStyle().ItemSpacing.x, ImGui::GetFrameHeightWithSpacing()), false, ImGuiWindowFlags_NoScrollbar);
			ImGui::TextWrapped(playerData["note"].get<std::string>().c_str());
			ImGui::EndChild();
			ImGui::SameLine();
			ImGui::SetCursorPosX(buttonPos - buttonWidth);
			if (ImGui::Button((std::string("Edit##") + uniqueID).c_str(), ImVec2(0, ImGui::GetFrameHeightWithSpacing())))
			{
				ImGui::OpenPopup("Edit note");
				this->playersNoteToEdit = uniqueID;
			}
			ImGui::NextColumn();
			ImGui::Separator();
		}
	}


	RenderEditNoteModal();

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
	ImGui::PopStyleVar();
	ImGui::EndChild();


	this->shouldBlockInput = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
	ImGui::End();
}

void DejaVu::RenderEditNoteModal()
{
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x / 3, ImGui::GetIO().DisplaySize.y / 3), ImGuiCond_Appearing);
	if (ImGui::BeginPopupModal("Edit note"))
	{
		if (false && this->playersNoteToEdit.empty())
		{
			ImGui::CloseCurrentPopup();
		}
		else
		{
			json::value_type playerData = this->data["players"][this->playersNoteToEdit];
			if (!playerData.contains("note"))
				playerData["note"] = "";
			auto playerNote = playerData["note"].get_ptr<std::string*>();

			ImVec2 textSize(
				ImGui::GetWindowWidth() - 2 * ImGui::GetStyle().WindowPadding.x,
				ImGui::GetWindowHeight() - 2 * ImGui::GetStyle().WindowPadding.y - ImGui::GetTextLineHeightWithSpacing() - 4 * ImGui::GetStyle().FramePadding.y - 4 * ImGui::GetStyle().ItemSpacing.y
			);
			if (ImGui::InputTextMultiline("##note", playerNote, textSize))
				this->data["players"][this->playersNoteToEdit]["note"] = *playerNote;
			//if (ImGui::IsAnyWindowFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
			if (ImGui::IsWindowAppearing())
				ImGui::SetKeyboardFocusHere();

			int escIdx = ImGui::GetIO().KeyMap[ImGuiKey_Escape];
			if (ImGui::Button("OK", ImVec2(120, 0)) || (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && escIdx >= 0 && ImGui::IsKeyPressed(escIdx)))
				ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
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

void DejaVu::LaunchQuickNoteModal()
{
	this->openQuickNote = true;
	if (!this->isWindowOpen)
		cvarManager->executeCommand("togglemenu " + GetMenuName());
}

void DejaVu::OnOpen()
{
	this->isWindowOpen = true;
	this->cvarManager->getCvar("cl_dejavu_log").setValue(true);
	this->playerIDsToDisplay.resize(this->data["players"].size());
}

void DejaVu::OnClose()
{
	this->isWindowOpen = false;
	WriteData();
}

void DejaVu::OpenMenu()
{
	if (!this->isWindowOpen)
		ToggleMenu();
}

void DejaVu::CloseMenu()
{
	if (this->isWindowOpen)
		ToggleMenu();
}

void DejaVu::ToggleMenu()
{
	cvarManager->executeCommand("togglemenu " + GetMenuName());
}
#endif
