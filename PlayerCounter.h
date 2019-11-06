#pragma once
#pragma comment(lib, "BakkesMod.lib")
#include "bakkesmod\plugin\bakkesmodplugin.h"
#include "json.hpp"
#include <vector>

using json = nlohmann::json;

enum class Playlists {
	Duel = 1,
	Doubles = 2,
	Standard = 3,
	Chaos = 4,
	Ranked_Duel = 10,
	Ranked_Doubles = 11,
	Ranked_Solo_Standard = 12,
	Ranked_Standard = 13,
	Mutator_Mashup = 14,
	Snow_Day = 15,
	Rocket_Labs = 16,
	Hoops = 17,
	Rumble = 18,
	Dropshot = 23,
};

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

	std::shared_ptr<int> textColorR;
	std::shared_ptr<int> textColorG;
	std::shared_ptr<int> textColorB;

	std::shared_ptr<bool> enabledBackground;

	std::shared_ptr<int> backgroundColorR;
	std::shared_ptr<int> backgroundColorG;
	std::shared_ptr<int> backgroundColorB;

	json data;
	std::vector<std::string> currentMatchIDs;
	MMRWrapper mmrWrapper;
	std::map<std::string, int> currentMatchMetCounts;

	void Log(std::string msg);
	void LogError(std::string msg);
	void LoadData();
	void WriteData();
	void GetAndSetMetMMR(SteamID steamID, int playlist, SteamID idToSet);

};
