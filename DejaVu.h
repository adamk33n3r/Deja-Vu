#pragma once
#pragma comment(lib, "BakkesMod.lib")

#define DEV 0

#define _HAS_STD_BYTE 0
#include <vector>
#include <map>
#include <set>
#include <filesystem>
#include <optional>

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "vendor/json.hpp"

#include "Canvas.h"
#include "CVar2WayBinding.h"


using json = nlohmann::json;

#define PLAYLISTS \
X(NONE, -1) \
X(Duel, 1) \
X(Doubles, 2) \
X(Standard, 3) \
X(Chaos, 4) \
X(PrivateMatch, 6) \
X(OfflineSeason, 7) \
X(OfflineSplitscreen, 8) \
X(Training, 9) \
X(RankedDuel, 10) \
X(RankedDoubles, 11) \
X(RankedSoloStandard, 12) \
X(RankedStandard, 13) \
X(MutatorMashup, 14) \
X(SnowDay, 15) \
X(RocketLabs, 16) \
X(Hoops, 17) \
X(Rumble, 18) \
X(Workshop, 19) \
X(TrainingEditor, 20) \
X(CustomTraining, 21) \
X(Tournament, 22) \
X(Dropshot, 23) \
X(RankedHoops, 27) \
X(RankedRumble, 28) \
X(RankedDropshot, 29) \
X(RankedSnowDay, 30)

#define X(playlist, id) playlist = id,
enum class Playlist
{
	PLAYLISTS
};
#undef X

#define X(playlist, id) { Playlist::playlist, #playlist },
static std::map<Playlist, std::string> PlaylistNames
{
	PLAYLISTS
};
#undef X

struct PlaylistFilter {
	std::string name;
	int playlistID;
};

enum class Side {
	Same,
	Other,
};

struct Record {
	int wins;
	int losses;
};


struct Rect {
	int X, Y, Width, Height;
};

struct RenderData {
	std::string id;
	std::string name;
	int metCount;
	Record record;
};

inline void to_json(json& j, const Record& record) {
	j = json{ {"wins", record.wins}, { "losses", record.losses } };
}

inline void from_json(const json& j, Record& record) {
	j.at("wins").get_to(record.wins);
	j.at("losses").get_to(record.losses);
}

class DejaVu : public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginWindow
{
public:
	DejaVu() : mmrWrapper(MMRWrapper(NULL)) {}

	virtual void onLoad() override;
	virtual void onUnload() override;

	void HandlePlayerAdded(std::string eventName);
	void HandlePlayerRemoved(std::string eventName);
	void HandleGameStart(std::string eventName);
	void HandleGameEnd(std::string eventName);
	void HandleGameLeave(std::string eventName);
	void RenderDrawable(CanvasWrapper canvas);
	void OpenScoreboard(std::string eventName);
	void CloseScoreboard(std::string eventName);
	void LaunchQuickNoteModal();

#pragma region GUI

	void Render() override;
	void RenderEditNoteModal();
	std::string GetMenuName() override;
	std::string GetMenuTitle() override;
	void SetImGuiContext(uintptr_t ctx) override;
	bool ShouldBlockInput() override;
	bool IsActiveOverlay() override;
	void OnOpen() override;
	void OnClose() override;
	void OpenMenu();
	void CloseMenu();
	void ToggleMenu();

private:
	bool isWindowOpen = false;
	bool shouldBlockInput = false;
	std::string menuTitle = "Deja Vu";
	bool openQuickNote = false;
	std::string playersNoteToEdit = "";

#pragma endregion GUI

#pragma region cvars
private:

	CVar2WayBinding<bool> enabled =                CVar2WayBinding<bool>("cl_dejavu_enabled", true, "Enables plugin");
	CVar2WayBinding<bool> trackOpponents =         CVar2WayBinding<bool>("cl_dejavu_track_opponents", true, "Track players if opponents");
	CVar2WayBinding<bool> trackTeammates =         CVar2WayBinding<bool>("cl_dejavu_track_teammates", true, "Track players if teammates");
	CVar2WayBinding<bool> trackGrouped =           CVar2WayBinding<bool>("cl_dejavu_track_grouped", true, "Track players if in party");
	CVar2WayBinding<bool> enabledVisuals =         CVar2WayBinding<bool>("cl_dejavu_visuals", true, "Enables visuals");
	CVar2WayBinding<bool> toggleWithScoreboard =   CVar2WayBinding<bool>("cl_dejavu_toggle_with_scoreboard", false, "Toggle with scoreboard (instead of always on)");
	CVar2WayBinding<bool> showNotes =              CVar2WayBinding<bool>("cl_dejavu_show_player_notes", false, "Show player notes in the visuals");
	CVar2WayBinding<bool> enabledDebug =           CVar2WayBinding<bool>("cl_dejavu_debug", false, "Enables debug view. Useful when changing visual settings");
	CVar2WayBinding<bool> enabledLog =             CVar2WayBinding<bool>("cl_dejavu_log", false, "Enables logging");
	CVar2WayBinding<bool> showMetCount =           CVar2WayBinding<bool>("cl_dejavu_show_metcount", true, "Show the met count instead of your record");
	CVar2WayBinding<int> scale =                   CVar2WayBinding<int>("cl_dejavu_scale", 1, "Scale of visuals", true, true, 0.0f, true, 4.0f);
	CVar2WayBinding<float> alpha =                 CVar2WayBinding<float>("cl_dejavu_alpha", 0.75f, "Alpha of visuals", true, true, 0.0f, true, 1.0f);
	CVar2WayBinding<float> xPos =                  CVar2WayBinding<float>("cl_dejavu_xpos", 0.0f, "X position of visuals", true, true, 0.0f, true, 1.0f);
	CVar2WayBinding<float> yPos =                  CVar2WayBinding<float>("cl_dejavu_ypos", 1.0f, "Y position of visuals", true, true, 0.0f, true, 1.0f);
	CVar2WayBinding<float> width =                 CVar2WayBinding<float>("cl_dejavu_width", 0.0f, "Width of visuals", true, true, 0.0f, true, 1.0f);

	// DEPRECATED
	[[deprecated("Use textColor instead")]]
	CVar2WayBinding<int> textColorR =              CVar2WayBinding<int>("cl_dejavu_text_color_r", 0xff, "Text color: Red", false);
	// DEPRECATED
	[[deprecated("Use textColor instead")]]
	CVar2WayBinding<int> textColorG =              CVar2WayBinding<int>("cl_dejavu_text_color_g", 0x00, "Text color: Green", false);
	// DEPRECATED
	[[deprecated("Use textColor instead")]]
	CVar2WayBinding<int> textColorB =              CVar2WayBinding<int>("cl_dejavu_text_color_b", 0x00, "Text color: Blue", false);
	CVar2WayBinding<LinearColor> textColor =       CVar2WayBinding<LinearColor>("cl_dejavu_text_color", LinearColor{ 0xff, 0xff, 0xff, 0xff }, "Text color");

	CVar2WayBinding<bool> enabledBackground =      CVar2WayBinding<bool>("cl_dejavu_background", true, "Enables background");

	// DEPRECATED
	[[deprecated("Use backgroundColor instead")]]
	CVar2WayBinding<int> backgroundColorR =        CVar2WayBinding<int>("cl_dejavu_background_color_r", 0x00, "Background color: Red", false);
	// DEPRECATED
	[[deprecated("Use backgroundColor instead")]]
	CVar2WayBinding<int> backgroundColorG =        CVar2WayBinding<int>("cl_dejavu_background_color_g", 0x00, "Background color: Green", false);
	// DEPRECATED
	[[deprecated("Use backgroundColor instead")]]
	CVar2WayBinding<int> backgroundColorB =        CVar2WayBinding<int>("cl_dejavu_background_color_b", 0x00, "Background color: Blue", false);
	CVar2WayBinding<LinearColor> backgroundColor = CVar2WayBinding<LinearColor>("cl_dejavu_background_color", LinearColor{ 0x00, 0x00, 0x00, 0xff }, "Background color");

	CVar2WayBinding<bool> hasUpgradedColors =      CVar2WayBinding<bool>("cl_dejavu_has_upgraded_colors", false, "Flag for upgrading colors", true);
#pragma endregion cvars

	json data;
	MMRWrapper mmrWrapper;
	bool gameIsOver = false;
	bool isScoreboardOpen = false;

	std::map<std::string, PriWrapper> currentMatchPRIs;
	std::map<std::string, std::set<std::string>> matchesMetLists;
	std::vector<RenderData> blueTeamRenderData;
	std::vector<RenderData> orangeTeamRenderData;
	std::vector<std::string> playerIDsToDisplay;

	inline static auto mainFile = "player_counter.json";
	inline static auto tmpFile  = "player_counter.json.tmp";
	inline static auto bakFile  = "player_counter.json.bak";
	inline static auto logFile  = "dejavu.log";

	inline static std::filesystem::path dataDir;
	inline static std::filesystem::path mainPath;
	inline static std::filesystem::path tmpPath;
	inline static std::filesystem::path bakPath;
	inline static std::filesystem::path logPath;

	void Log(std::string msg);
	void LogError(std::string msg);
	void LogChatbox(std::string msg);
	void LoadData();
	void WriteData();
	void Reset();
	void GetAndSetMetMMR(SteamID steamID, int playlist, SteamID idToSet);
	Record GetRecord(UniqueIDWrapper uniqueID, int playlist, Side side);
	Record GetRecord(std::string uniqueID, Playlist playlist, Side side);
	Record GetRecord(std::string uniqueID, int playlist, Side side);
	void SetRecord();
	void RenderUI(const std::vector<RenderData>& renderData, const Canvas::CanvasTableOptions& tableOptions, const std::vector<Canvas::CanvasColumnOptions>& columnOptions, const bool renderPlayer);
	void AddPlayerToRenderData(PriWrapper player);
	void RemovePlayerFromRenderData(PriWrapper player);
	bool IsInRealGame();
	void HookAndLogEvent(std::string eventName);

	std::optional<std::string> GetMatchGUID();
	ServerWrapper GetCurrentServer();
	PriWrapper GetLocalPlayerPRI();

	void CleanUpJson();

	template <class T>
	CVarWrapper RegisterCVar(
		const char* name,
		const char* description,
		//T defaultValue,
		std::shared_ptr<T>& bindTo,
		bool searchable = true,
		bool hasMin = false,
		float min = 0,
		bool hasMax = false,
		float max = 0,
		bool saveToCfg = true
	);

};
