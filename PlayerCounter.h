#pragma once
#pragma comment(lib, "BakkesMod.lib")
#define _HAS_STD_BYTE 0
#include "bakkesmod\plugin\bakkesmodplugin.h"
#include "vendor\json.hpp"
#include <vector>
#include <filesystem>

using json = nlohmann::json;

enum class Playlist
{
	Duel = 1,
	Doubles = 2,
	Standard = 3,
	Chaos = 4,
	PrivateMatch = 6,
	OfflineSeason = 7,
	OfflineSplitscreen = 8,
	Training = 9,
	RankedDuel = 10,
	RankedDoubles = 11,
	RankedSoloStandard = 12,
	RankedStandard = 13,
	MutatorMashup = 14,
	SnowDay = 15,
	RocketLabs = 16,
	Hoops = 17,
	Rumble = 18,
	Workshop = 19,
	TrainingEditor = 20,
	CustomTraining = 21,
	Tournament = 22,
	Dropshot = 23,
	RankedHoops = 27,
	RankedRumble = 28,
	RankedDropshot = 29,
	RankedSnowDay = 30
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

void to_json(json& j, const Record& record) {
	j = json{ {"wins", record.wins}, { "losses", record.losses } };
}

void from_json(const json& j, Record& record) {
	j.at("wins").get_to(record.wins);
	j.at("losses").get_to(record.losses);
}

class PlayerCounter : public BakkesMod::Plugin::BakkesModPlugin
{
public:
	PlayerCounter() : mmrWrapper(MMRWrapper(0)) {}

	virtual void onLoad() override;
	virtual void onUnload() override;

	void HandlePlayerAdded(std::string eventName);
	void HandlePlayerRemoved(std::string eventName);
	void HandleGameStart(std::string eventName);
	void HandleGameEnd(std::string eventName);
	void HandleGameLeave(std::string eventName);
	void RenderDrawable(CanvasWrapper canvas);

private:
	std::shared_ptr<bool> enabled;
	std::shared_ptr<bool> trackOpponents;
	std::shared_ptr<bool> trackTeammates;
	std::shared_ptr<bool> trackGrouped;
	std::shared_ptr<bool> enabledVisuals;
	std::shared_ptr<bool> enabledDebug;
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
	bool gameIsOver;

	std::map<std::string, PriWrapper> currentMatchPRIs;
	std::map<std::string, PriWrapper> currentMatchPRIsAll;
	std::vector<RenderData> blueTeamRenderData;
	std::vector<RenderData> orangeTeamRenderData;

	inline static auto mainFile = "player_counter.json";
	inline static auto tmpFile  = "player_counter.json.tmp";
	inline static auto bakFile  = "player_counter.json.bak";
	inline static auto logFile  = "dejavu.log";

	inline static auto dataDir = std::filesystem::path("bakkesmod/data/dejavu");
	inline static auto mainPath = std::filesystem::path(dataDir).append(mainFile);
	inline static auto tmpPath = std::filesystem::path(dataDir).append(tmpFile);
	inline static auto bakPath = std::filesystem::path(dataDir).append(bakFile);
	inline static auto logPath = std::filesystem::path(dataDir).append(logFile);

	void Log(std::string msg);
	void LogError(std::string msg);
	void LoadData();
	void WriteData();
	void Reset();
	void GetAndSetMetMMR(SteamID steamID, int playlist, SteamID idToSet);
	Record GetRecord(SteamID steamID, int playlist, Side side);
	Record GetRecord(std::string steamID, int playlist, Side side);
	void SetRecord();
	ServerWrapper GetCurrentServer();
	Rect RenderUI(CanvasWrapper& canvas, Rect area, const std::vector<RenderData>& renderData);
	void AddPlayerToRenderData(PriWrapper player);
	void RemovePlayerFromRenderData(PriWrapper player);

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
