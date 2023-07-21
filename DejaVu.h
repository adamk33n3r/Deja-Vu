#pragma once

#define DEV 0

#include <vector>
#include <map>
#include <set>
#include <filesystem>
#include <optional>

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"

#include "Canvas.h"
#include "CVar2WayBinding.h"

using json = nlohmann::json;

#include "Version.h"
constexpr auto PluginVersion = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD) stringify(VERSION_PHASE);

constexpr auto CVAR_ENABLED = "cl_dejavu_enabled";
constexpr auto CVAR_TRACK_OPPONENTS = "cl_dejavu_track_opponents";
constexpr auto CVAR_TRACK_TEAMMATES = "cl_dejavu_track_teammates";
constexpr auto CVAR_TRACK_GROUPED = "cl_dejavu_track_grouped";
constexpr auto CVAR_VISUALS = "cl_dejavu_visuals";
constexpr auto CVAR_TOGGLE_WITH_SCOREBOARD = "cl_dejavu_toggle_with_scoreboard";
constexpr auto CVAR_SHOW_PLAYER_NOTES = "cl_dejavu_show_player_notes";
constexpr auto CVAR_DEBUG = "cl_dejavu_debug";
constexpr auto CVAR_LOG = "cl_dejavu_log";
constexpr auto CVAR_SHOW_MET_COUNT = "cl_dejavu_show_metcount";
constexpr auto CVAR_SHOW_RECORD = "cl_dejavu_show_record";
constexpr auto CVAR_SHOW_ALL_PLAYLISTS_RECORD = "cl_dejavu_show_all_playlists_record";
constexpr auto CVAR_SCALE = "cl_dejavu_scale";
constexpr auto CVAR_ALPHA = "cl_dejavu_alpha";
constexpr auto CVAR_XPOS = "cl_dejavu_xpos";
constexpr auto CVAR_YPOS = "cl_dejavu_ypos";
constexpr auto CVAR_WIDTH = "cl_dejavu_width";
constexpr auto CVAR_TEXT_COLOR_RED = "cl_dejavu_text_color_r";
constexpr auto CVAR_TEXT_COLOR_GREEN = "cl_dejavu_text_color_g";
constexpr auto CVAR_TEXT_COLOR_BLUE = "cl_dejavu_text_color_b";
constexpr auto CVAR_TEXT_COLOR = "cl_dejavu_text_color";
constexpr auto CVAR_BORDERS = "cl_dejavu_borders";
constexpr auto CVAR_BORDER_COLOR = "cl_dejavu_border_color";
constexpr auto CVAR_BACKGROUND = "cl_dejavu_background";
constexpr auto CVAR_BACKGROUND_COLOR_RED = "cl_dejavu_background_color_r";
constexpr auto CVAR_BACKGROUND_COLOR_GREEN = "cl_dejavu_background_color_g";
constexpr auto CVAR_BACKGROUND_COLOR_BLUE = "cl_dejavu_background_color_b";
constexpr auto CVAR_BACKGROUND_COLOR = "cl_dejavu_background_color";
constexpr auto CVAR_HAS_UPGRADED_COLORS = "cl_dejavu_has_upgraded_colors";
constexpr auto CVAR_KEYBIND_MAIN_GUI = "cl_dejavu_keybind_main_gui";
constexpr auto CVAR_KEYBIND_QUICK_NOTE = "cl_dejavu_keybind_quick_note";

#define PLAYLISTS \
X(ANY, -1) \
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
X(RocketLabsOld, 16) \
X(Hoops, 17) \
X(Rumble, 18) \
X(Workshop, 19) \
X(TrainingEditor, 20) \
X(CustomTraining, 21) \
X(Tournament, 22) \
X(Dropshot, 23) \
X(LocalMatch, 24) \
X(ExternalMatchRanked, 26) \
X(RankedHoops, 27) \
X(RankedRumble, 28) \
X(RankedDropshot, 29) \
X(RankedSnowDay, 30) \
X(GhostHunt, 31) \
X(BeachBall, 32) \
X(SpikeRush, 33) \
X(TournamentMatchAutomatic, 34) \
X(RocketLabs, 35) \
X(DropshotRumble, 37) \
X(Heatseeker, 38) \
X(BoomerBall, 41) \
X(HeatseekerDoubles, 43) \
X(WinterBreakaway, 44) \
X(Gridiron, 46) \
X(SuperCube, 47) \
X(TacticalRumble, 48) \
X(SpringLoaded, 49) \
X(SpeedDemon, 50) \
X(RumbleBM, 52) \
X(Knockout, 54) \
X(Thirdwheel, 55)

#define X(playlist, id) playlist = id,
enum class PlaylistID
{
	PLAYLISTS
};
#undef X

#define X(playlist, id) { PlaylistID::playlist, #playlist },
static std::map<PlaylistID, std::string> PlaylistNames
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

enum TEAM : unsigned char {
	TEAM_NOT_SET = (unsigned char)-1,
	TEAM_BLUE = 0,
	TEAM_ORANGE = 1,
};


struct Rect {
	int X, Y, Width, Height;
};

struct RenderData {
	std::string id;
	std::string name;
	int metCount;
	Record record;
	Record allPlaylistsRecord;
	std::string note;
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
	void HandleWinnerSet(std::string eventName);
	void HandleForfeitChanged(std::string eventName);
	void HandleGameTimeUpdate(std::string eventName);
	void GameOver();
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

private:

#pragma region cvars
	CVar2WayBinding<bool> enabled =                CVar2WayBinding<bool>(CVAR_ENABLED, true, "Enables plugin");
	CVar2WayBinding<bool> trackOpponents =         CVar2WayBinding<bool>(CVAR_TRACK_OPPONENTS, true, "Track players if opponents");
	CVar2WayBinding<bool> trackTeammates =         CVar2WayBinding<bool>(CVAR_TRACK_TEAMMATES, true, "Track players if teammates");
	CVar2WayBinding<bool> trackGrouped =           CVar2WayBinding<bool>(CVAR_TRACK_GROUPED, true, "Track players if in party");
	CVar2WayBinding<bool> enabledVisuals =         CVar2WayBinding<bool>(CVAR_VISUALS, true, "Enables visuals");
	CVar2WayBinding<bool> toggleWithScoreboard =   CVar2WayBinding<bool>(CVAR_TOGGLE_WITH_SCOREBOARD, false, "Toggle with scoreboard (instead of always on)");
	CVar2WayBinding<bool> showNotes =              CVar2WayBinding<bool>(CVAR_SHOW_PLAYER_NOTES, false, "Show player notes in the visuals");
	CVar2WayBinding<bool> enabledDebug =           CVar2WayBinding<bool>(CVAR_DEBUG, false, "Enables debug view. Useful when changing visual settings");
	CVar2WayBinding<bool> enabledLog =             CVar2WayBinding<bool>(CVAR_LOG, false, "Enables logging");
	CVar2WayBinding<bool> showMetCount =           CVar2WayBinding<bool>(CVAR_SHOW_MET_COUNT, true, "Show the met count");
	CVar2WayBinding<bool> showRecord =             CVar2WayBinding<bool>(CVAR_SHOW_RECORD, false, "Show the record");
	CVar2WayBinding<bool> showAllPlaylistsRecord = CVar2WayBinding<bool>(CVAR_SHOW_ALL_PLAYLISTS_RECORD, false, "Show record over all playlists");
	CVar2WayBinding<float> scale =                 CVar2WayBinding<float>(CVAR_SCALE, 1.0f, "Scale of visuals", true, true, 0.0f, true, 4.0f);
	CVar2WayBinding<float> alpha =                 CVar2WayBinding<float>(CVAR_ALPHA, 0.75f, "Alpha of visuals", true, true, 0.0f, true, 1.0f);
	CVar2WayBinding<float> xPos =                  CVar2WayBinding<float>(CVAR_XPOS, 0.0f, "X position of visuals", true, true, 0.0f, true, 1.0f);
	CVar2WayBinding<float> yPos =                  CVar2WayBinding<float>(CVAR_YPOS, 1.0f, "Y position of visuals", true, true, 0.0f, true, 1.0f);
	CVar2WayBinding<float> width =                 CVar2WayBinding<float>(CVAR_WIDTH, 0.0f, "Width of visuals", true, true, 0.0f, true, 1.0f);

	// DEPRECATED
	[[deprecated("Use textColor instead")]]
	CVar2WayBinding<int> textColorR =              CVar2WayBinding<int>(CVAR_TEXT_COLOR_RED, 0xff, "Text color: Red", false);
	// DEPRECATED
	[[deprecated("Use textColor instead")]]
	CVar2WayBinding<int> textColorG =              CVar2WayBinding<int>(CVAR_TEXT_COLOR_GREEN, 0x00, "Text color: Green", false);
	// DEPRECATED
	[[deprecated("Use textColor instead")]]
	CVar2WayBinding<int> textColorB =              CVar2WayBinding<int>(CVAR_TEXT_COLOR_BLUE, 0x00, "Text color: Blue", false);
	CVar2WayBinding<LinearColor> textColor =       CVar2WayBinding<LinearColor>(CVAR_TEXT_COLOR, LinearColor{ 0xff, 0xff, 0xff, 0xff }, "Text color");

	CVar2WayBinding<bool> enabledBorders =         CVar2WayBinding<bool>(CVAR_BORDERS, false, "Enables borders");
	CVar2WayBinding<LinearColor> borderColor =     CVar2WayBinding<LinearColor>(CVAR_BORDER_COLOR, LinearColor{ 0xff, 0xff, 0xff, 0xff }, "Border color");

	CVar2WayBinding<bool> enabledBackground =      CVar2WayBinding<bool>(CVAR_BACKGROUND, true, "Enables background");

	// DEPRECATED
	[[deprecated("Use backgroundColor instead")]]
	CVar2WayBinding<int> backgroundColorR =        CVar2WayBinding<int>(CVAR_BACKGROUND_COLOR_RED, 0x00, "Background color: Red", false);
	// DEPRECATED
	[[deprecated("Use backgroundColor instead")]]
	CVar2WayBinding<int> backgroundColorG =        CVar2WayBinding<int>(CVAR_BACKGROUND_COLOR_GREEN, 0x00, "Background color: Green", false);
	// DEPRECATED
	[[deprecated("Use backgroundColor instead")]]
	CVar2WayBinding<int> backgroundColorB =        CVar2WayBinding<int>(CVAR_BACKGROUND_COLOR_BLUE, 0x00, "Background color: Blue", false);
	CVar2WayBinding<LinearColor> backgroundColor = CVar2WayBinding<LinearColor>(CVAR_BACKGROUND_COLOR, LinearColor{ 0x00, 0x00, 0x00, 0xff }, "Background color");
	CVar2WayBinding<bool> hasUpgradedColors =      CVar2WayBinding<bool>(CVAR_HAS_UPGRADED_COLORS, false, "Flag for upgrading colors", true);

	CVar2WayBinding<std::string> mainGUIKeybind =  CVar2WayBinding<std::string>(CVAR_KEYBIND_MAIN_GUI, "None", "Main keybind");
	CVar2WayBinding<std::string> quickNoteKeybind = CVar2WayBinding<std::string>(CVAR_KEYBIND_QUICK_NOTE, "None", "Quick note keybind");
#pragma endregion cvars

	json data;
	MMRWrapper mmrWrapper;
	bool gameIsOver = false;
	bool isAlreadyAddedToStats = false;
	bool isScoreboardOpen = false;
	std::optional<std::string> curMatchGUID;

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
	Record GetRecord(UniqueIDWrapper uniqueID, PlaylistID playlist, Side side);
	Record GetRecord(std::string uniqueID, PlaylistID playlist, Side side);
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
	void GenerateSettingsFile();

	template <class T>
	CVarWrapper RegisterCVar(
		const char* name,
		const char* description,
		std::shared_ptr<T>& bindTo,
		bool searchable = true,
		bool hasMin = false,
		float min = 0,
		bool hasMax = false,
		float max = 0,
		bool saveToCfg = true
	);

};
