#pragma once
#pragma comment(lib, "BakkesMod.lib")

#define ENABLE_GUI 1

#define _HAS_STD_BYTE 0
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include <vector>
#include <map>
#include <set>
#include <filesystem>
#if ENABLE_GUI
#include "bakkesmod/plugin/pluginwindow.h"
#include "vendor/imgui/imgui.h"
#endif
#include "vendor/json.hpp"
#include "Canvas.h"


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

class DejaVu : public BakkesMod::Plugin::BakkesModPlugin
#if ENABLE_GUI
, public BakkesMod::Plugin::PluginWindow
#endif
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
// GUI

#if ENABLE_GUI
	void Render() override;
	std::string GetMenuName() override;
	std::string GetMenuTitle() override;
	void SetImGuiContext(uintptr_t ctx) override;
	bool ShouldBlockInput() override;
	bool IsActiveOverlay() override;
	void OnOpen() override;
	void OnClose() override;
#endif

private:
	bool isWindowOpen = false;
	bool shouldBlockInput = false;
	std::string menuTitle = "Deja Vu";
	const char* playlists[2] = {"option 1", "option 2"};
	bool selected1 = false;
	bool isScoreboardOpen = false;

private:
	std::shared_ptr<bool> enabled;
	std::shared_ptr<bool> trackOpponents;
	std::shared_ptr<bool> trackTeammates;
	std::shared_ptr<bool> trackGrouped;
	std::shared_ptr<bool> enabledVisuals;
	std::shared_ptr<bool> toggleWithScoreboard;
	std::shared_ptr<bool> enabledDebug;
	std::shared_ptr<bool> enabledLog;
	std::shared_ptr<bool> showMetCount;
	std::shared_ptr<int> scale;
	std::shared_ptr<float> alpha;
	std::shared_ptr<float> xPos;
	std::shared_ptr<float> yPos;
	std::shared_ptr<float> width;

	std::shared_ptr<int> textColorR;
	std::shared_ptr<int> textColorG;
	std::shared_ptr<int> textColorB;

	std::shared_ptr<bool> enabledBackground;

	std::shared_ptr<int> backgroundColorR;
	std::shared_ptr<int> backgroundColorG;
	std::shared_ptr<int> backgroundColorB;

	json data;
	MMRWrapper mmrWrapper;
	bool gameIsOver = false;

	std::map<std::string, PriWrapper> currentMatchPRIs;
	std::map<std::string, std::set<std::string>> matchPRIsMetList;
	std::vector<RenderData> blueTeamRenderData;
	std::vector<RenderData> orangeTeamRenderData;
	std::vector<std::string> playerNotes;

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

	ServerWrapper GetCurrentServer();
	PriWrapper GetLocalPlayerPRI();

	void CleanUpJson();

	template <class T>
	CVarWrapper RegisterCVar(
		const char* name,
		const char* description,
		T defaultValue,
		std::shared_ptr<T>& bindTo,
		bool searchable = true,
		bool hasMin = false,
		float min = 0,
		bool hasMax = false,
		float max = 0,
		bool saveToCfg = true
	);

};
