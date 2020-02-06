#include "DejaVu.h"
#include <iomanip>
#include <sstream>
#include <fstream>

#include "vendor\easyloggingpp-9.96.7\src\easylogging++.h"

#define DEV 1

/**
 * TODO
 * - Add option to show total record across all playlists
 * - IMGUI stuff
 * - Don't show * if youre showing the met count
 */

INITIALIZE_EASYLOGGINGPP

BAKKESMOD_PLUGIN(DejaVu, "Deja Vu", "1.3.2", 0)

template <class T>
CVarWrapper DejaVu::RegisterCVar(
	const char* name,
	const char* description,
	T defaultValue,
	std::shared_ptr<T>& bindTo,
	bool searchable,
	bool hasMin,
	float min,
	bool hasMax,
	float max,
	bool saveToCfg
)
{
	bindTo = std::make_shared<T>(defaultValue);
	CVarWrapper cvar = this->cvarManager->registerCvar(
		name,
		std::to_string(defaultValue),
		description,
		searchable,
		hasMin,
		min,
		hasMax,
		max,
		saveToCfg
	);
	cvar.bindTo(bindTo);

	return cvar;
}

template <>
CVarWrapper DejaVu::RegisterCVar(
	const char* name,
	const char* description,
	std::string defaultValue,
	std::shared_ptr<std::string>& bindTo,
	bool searchable,
	bool hasMin,
	float min,
	bool hasMax,
	float max,
	bool saveToCfg
)
{
	bindTo = std::make_shared<std::string>(defaultValue);
	CVarWrapper cvar = this->cvarManager->registerCvar(
		name,
		defaultValue,
		description,
		searchable,
		hasMin,
		min,
		hasMax,
		max,
		saveToCfg
	);
	cvar.bindTo(bindTo);

	return cvar;
}

void SetupLogger(std::string logPath, bool enabled)
{
	el::Configurations defaultConf;
	defaultConf.setToDefault();
	defaultConf.setGlobally(el::ConfigurationType::Filename, logPath);
	defaultConf.setGlobally(el::ConfigurationType::Format, "%datetime %level [%fbase:%line] %msg");
	defaultConf.setGlobally(el::ConfigurationType::Enabled, enabled ? "true" : "false");
	el::Loggers::reconfigureLogger("default", defaultConf);
}

void DejaVu::HookAndLogEvent(std::string eventName)
{
	this->gameWrapper->HookEvent(eventName, std::bind(&DejaVu::LogChatbox, this, std::placeholders::_1));
}

void DejaVu::CleanUpJson()
{
	for (auto player : this->data["players"].items())
	{
		json::value_type playerData = player.value();
		for (auto playlistData : playerData["playlistData"].items())
		{
			bool containsRecords = playlistData.value().contains("records");
			bool recordsIsNull = containsRecords && playlistData.value()["records"].is_null();

			if (!containsRecords || recordsIsNull)
				this->data["players"][player.key()]["playlistData"].erase(playlistData.key());
		}
	}
	WriteData();
}

void DejaVu::onLoad()
{
	// At end of match in unranked when people leave and get replaced by bots the event fires and for some reason IsInOnlineGame turns back on
	// Check 1v1. Player added event didn't fire after joining last
	// Add debug
	this->mmrWrapper = this->gameWrapper->GetMMRWrapper();

	this->cvarManager->registerNotifier("dejavu_reload", [this](const std::vector<std::string>& commands) {
		LoadData();
	}, "Reloads the json data from file", PERMISSION_ALL);

	this->cvarManager->registerNotifier("dejavu_track_current", [this](const std::vector<std::string>& commands) {
		HandlePlayerAdded("dejavu_track_current");
	}, "Tracks current lobby", PERMISSION_ONLINE);

	this->cvarManager->registerNotifier("dejavu_test", [this](const std::vector<std::string>& commands) {
		this->gameWrapper->SetTimeout([this](GameWrapper* gameWrapper) {
			Log("test after 5");
		}, 5);
	}, "test", PERMISSION_ALL);

	this->cvarManager->registerNotifier("dejavu_cleanup", [this](const std::vector<std::string>& commands) {
		CleanUpJson();
	}, "Cleans up the json", PERMISSION_ALL);

	RegisterCVar("cl_dejavu_enable", "Enables plugin", true, this->enabled);

	RegisterCVar("cl_dejavu_track_opponents", "Track players if opponents", true, this->trackOpponents);
	RegisterCVar("cl_dejavu_track_teammates", "Track players if teammates", true, this->trackTeammates);
	RegisterCVar("cl_dejavu_track_grouped", "Track players if in party", true, this->trackGrouped);
	RegisterCVar("cl_dejavu_show_metcount", "Show the met count instead of your record", true, this->showMetCount);

	RegisterCVar("cl_dejavu_visuals", "Enables visuals", true, this->enabledVisuals);

	auto debugCVar = RegisterCVar("cl_dejavu_debug", "Enables debug view. Useful for choosing colors", false, this->enabledDebug);
	debugCVar.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		bool val = cvar.getBoolValue();

		if (this->gameWrapper->IsInOnlineGame()) {
			if (val)
				cvar.setValue(false);
			return;
		}

		this->blueTeamRenderData.clear();
		this->orangeTeamRenderData.clear();

		if (val) {
			this->blueTeamRenderData.push_back({ "0", "Blue Player 1", 5, { 5, 5 } });
			this->blueTeamRenderData.push_back({ "0", "Blue Player 2", 15, { 15, 15 } });
			this->blueTeamRenderData.push_back({ "0", "Blue Player 3 with a loooonngggg name", 999, { 999, 999 } });
			this->orangeTeamRenderData.push_back({ "0", "Orange Player 1", 5, { 5, 5 } });
			this->orangeTeamRenderData.push_back({ "0", "Orange Player 2", 15, { 15, 15 } });
			//this->orangeTeamRenderData.push_back({ "0", "Orange Player 3", 999, { 999, 999 } });
		}
	});

	RegisterCVar("cl_dejavu_scale", "Scale of visuals", 1, this->scale);
	
	RegisterCVar("cl_dejavu_alpha", "Alpha of display", 0.75f, this->alpha, true, true, 0.0f, true, 1.0f);
	
	RegisterCVar("cl_dejavu_xpos", "X position of display", 0.0f, this->xPos, true, true, 0.0f, true, 1.0f);
	RegisterCVar("cl_dejavu_ypos", "Y position of display", 1.0f, this->yPos, true, true, 0.0f, true, 1.0f);
	RegisterCVar("cl_dejavu_width", "Width of display", 0.0f, this->width, true, true, 0.0f, true, 1.0f);

	RegisterCVar("cl_dejavu_text_color_r", "Text color: Red", 255, this->textColorR);
	RegisterCVar("cl_dejavu_text_color_g", "Text color: Green", 255, this->textColorG);
	RegisterCVar("cl_dejavu_text_color_b", "Text color: Blue", 255, this->textColorB);

	RegisterCVar("cl_dejavu_background", "Enables background", true, this->enabledBackground);

	RegisterCVar("cl_dejavu_background_color_r", "Background color: Red", 0, this->backgroundColorR);
	RegisterCVar("cl_dejavu_background_color_g", "Background color: Green", 0, this->backgroundColorG);
	RegisterCVar("cl_dejavu_background_color_b", "Background color: Blue", 0, this->backgroundColorB);

	auto logCVar = RegisterCVar("cl_dejavu_log", "Enables logging", false, this->enabledLog);
	logCVar.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		bool val = cvar.getBoolValue();

		SetupLogger(this->logPath.string(), val);
	});

	// I guess this doesn't fire for "you"
	this->gameWrapper->HookEvent("Function TAGame.GameEvent_TA.EventPlayerAdded", std::bind(&DejaVu::HandlePlayerAdded, this, std::placeholders::_1));
	this->gameWrapper->HookEvent("Function GameEvent_TA.Countdown.BeginState", std::bind(&DejaVu::HandlePlayerAdded, this, std::placeholders::_1));
	this->gameWrapper->HookEvent("Function TAGame.Team_TA.EventScoreUpdated", std::bind(&DejaVu::HandlePlayerAdded, this, std::placeholders::_1));
	// TODO: Look for event like "spawning" so that when you join an in progress match it will gather data

	this->gameWrapper->HookEvent("Function TAGame.GameEvent_TA.EventPlayerRemoved", std::bind(&DejaVu::HandlePlayerRemoved, this, std::placeholders::_1));

	// Don't think this one ever actually works
	this->gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.InitGame", bind(&DejaVu::HandleGameStart, this, std::placeholders::_1));
	// Fires when joining a game
	this->gameWrapper->HookEvent("Function OnlineGameJoinGame_X.JoiningBase.IsJoiningGame", std::bind(&DejaVu::HandleGameStart, this, std::placeholders::_1));
	// Fires when the match is first initialized
	this->gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnAllTeamsCreated", std::bind(&DejaVu::HandleGameStart, this, std::placeholders::_1));

	this->gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.EventMatchEnded", std::bind(&DejaVu::HandleGameEnd, this, std::placeholders::_1));
	this->gameWrapper->HookEvent("Function TAGame.GFxShell_TA.LeaveMatch", std::bind(&DejaVu::HandleGameLeave, this, std::placeholders::_1));

	this->gameWrapper->UnregisterDrawables();
	this->gameWrapper->RegisterDrawable(bind(&DejaVu::RenderDrawable, this, std::placeholders::_1));

	//std::string eventsToLog[] = {
		//"Function TAGame.GameEvent_Soccar_TA.EndGame",
		//"Function TAGame.GameEvent_Soccar_TA.EndRound",
		//"Function TAGame.GameEvent_Soccar_TA.EventMatchEnded",
		//"Function TAGame.GameEvent_Soccar_TA.EventGameEnded",
		//"Function TAGame.GameEvent_Soccar_TA.EventGameWinnerSet",
		//"Function TAGame.GameEvent_Soccar_TA.EventMatchWinnerSet",
		//"Function TAGame.GameEvent_Soccar_TA.HasWinner",
		//"Function TAGame.GameEvent_Soccar_TA.EventEndGameCountDown",
		//"Function TAGame.GameEvent_Soccar_TA.EventGameTimeUpdated",
		//"Function TAGame.GameEvent_Soccar_TA.Finished.BeginState",
		//"Function TAGame.GameEvent_Soccar_TA.Finished.OnFinished",
		//"Function TAGame.GameEvent_Soccar_TA.FinishEvent",
		//"Function TAGame.GameEvent_Soccar_TA.HasWinner",
		//"Function TAGame.GameEvent_Soccar_TA.OnGameWinnerSet",
		//"Function TAGame.GameEvent_Soccar_TA.SetMatchWinner",
		//"Function TAGame.GameEvent_Soccar_TA.SubmitMatch",
		//"Function TAGame.GameEvent_Soccar_TA.SubmitMatchComplete",
		//"Function TAGame.GameEvent_Soccar_TA.WaitForEndRound",
		//"Function TAGame.GameEvent_Soccar_TA.EventActiveRoundChanged",
		//"Function TAGame.GameEvent_Soccar_TA.GetWinningTeam",
		//"Function TAGame.GameEvent_TA.EventGameStateChanged",
		//"Function TAGame.GameEvent_TA.Finished.EndState",
		//"Function TAGame.GameEvent_TA.Finished.BeginState",
		//"Function TAGame.GameEvent_Soccar_TA.Destroyed",
		//"Function TAGame.GameEvent_TA.IsFinished",
	//};

	//for (std::string eventName : eventsToLog)
	//{
	//	HookAndLogEvent(eventName);
	//}

	/* 
		Goal Scored event: "Function TAGame.Team_TA.EventScoreUpdated"
		Game Start event: "Function TAGame.GameEvent_Soccar_TA.InitGame"
		Game End event: "Function TAGame.GameEvent_Soccar_TA.EventMatchEnded"
		Function TAGame.GameEvent_Soccar_TA.Destroyed
		Function TAGame.GameEvent_Soccar_TA.EventMatchEnded
		Function GameEvent_TA.Countdown.BeginState
		Function TAGame.GameEvent_Soccar_TA.InitField

		Function TAGame.GameEvent_Soccar_TA.InitGame
		Function TAGame.GameEvent_Soccar_TA.InitGame

		Function TAGame.GameEvent_Soccar_TA.EventGameWinnerSet
		Function TAGame.GameEvent_Soccar_TA.EventMatchWinnerSet
	*/

	LoadData();

	this->gameWrapper->SetTimeout([this](GameWrapper* gameWrapper) {
		LOG(INFO) << "---DEJAVU LOADED---";
	}, 5);

#if DEV
	this->cvarManager->executeCommand("exec tmp.cfg");
#endif
}

void DejaVu::onUnload()
{
	LOG(INFO) << "---DEJAVU UNLOADED---";
	WriteData();

#if DEV
	this->cvarManager->backupCfg("./bakkesmod/cfg/tmp.cfg");
#endif

#if ENABLE_GUI
	if (this->isWindowOpen)
		this->cvarManager->executeCommand("togglemenu " + GetMenuName());
#endif
}

void DejaVu::Log(std::string msg)
{
	this->cvarManager->log(msg);
}

void DejaVu::LogError(std::string msg)
{
	this->cvarManager->log("ERROR: " + msg);
}

void DejaVu::LogChatbox(std::string msg)
{
	this->gameWrapper->LogToChatbox(msg);
	LOG(INFO) << msg;
}

void DejaVu::LoadData()
{
	// Upgrade old file path
	if (std::filesystem::exists(this->mainFile)) {
		Log("Upgrading old file path");
		std::filesystem::create_directories(this->dataDir);
		std::filesystem::rename(this->mainFile, this->mainPath);
		std::filesystem::rename(this->bakFile, this->bakPath);
	}

	std::ifstream in(this->mainPath);
	if (in.fail()) {
		LogError("Failed to open file");
		LogError(strerror(errno));
		this->data["players"] = json::object();
		WriteData();
		in.open(this->mainPath);
	}

	try {
		in >> this->data;
	}
	catch (const nlohmann::detail::exception& e) {
		in.close();
		LogError("Failed to parse json");
		LogError(e.what());
	}

	if (!this->data.contains("players")) {
		Log("Data doesn't contain players");
		this->data["players"] = json::object();
		WriteData();
	}

	Log("Successfully loaded existing data");

	in.close();
}


void DejaVu::WriteData()
{
	LOG(INFO) << "WriteData";
	std::filesystem::create_directories(this->dataDir);

	std::ofstream out(this->tmpPath);
	try {
		out << this->data.dump(4, ' ', false, json::error_handler_t::replace) << std::endl;
		out.close();
		std::error_code err;
		std::filesystem::remove(this->bakPath, err);
		if (std::filesystem::exists(this->mainPath)) {
			std::filesystem::rename(this->mainPath, this->bakPath, err);
			if (err.value() != 0) {
				LogError("Could not backup player counter");
				LogError(err.message());
				return;
			}
		}
		std::filesystem::rename(this->tmpPath, this->mainPath, err);
		if (err.value() != 0) {
			LogError("Could not move temp file to main");
			LogError(err.message());
			std::filesystem::rename(this->bakPath, this->mainPath, err);
			return;
		}
	}
	catch (const nlohmann::detail::exception& e) {
		LogError("failed to serialize json");
		LogError(e.what());
	}
	catch (...) {
		LogError("failed to serialize json (unknown)");
	}
}

ServerWrapper DejaVu::GetCurrentServer()
{
	if (this->gameWrapper->IsInReplay())
		return this->gameWrapper->GetGameEventAsReplay();
	else if (this->gameWrapper->IsInOnlineGame())
		return this->gameWrapper->GetOnlineGame();
	else if (this->gameWrapper->IsInFreeplay())
		return this->gameWrapper->GetGameEventAsServer();
	else if (this->gameWrapper->IsInCustomTraining())
		return this->gameWrapper->GetGameEventAsServer();
	else if (this->gameWrapper->IsSpectatingInOnlineGame())
		return this->gameWrapper->GetOnlineGame();
	else
		return NULL;
}

PriWrapper DejaVu::GetLocalPlayerPRI()
{
	auto server = GetCurrentServer();
	if (server.IsNull())
		return NULL;
	auto player = server.GetLocalPrimaryPlayer();
	if (player.IsNull())
		return NULL;
	return player.GetPRI();
}

void DejaVu::HandlePlayerAdded(std::string eventName)
{
	if (!IsInRealGame())
		return;
	LOG(INFO) << "HandlePlayerAdded: " << eventName;
	if (this->gameIsOver)
		return;
	ServerWrapper server = this->gameWrapper->GetOnlineGame();
	LOG(INFO) << "server is null: " << (server.IsNull() ? "true" : "false");
	MMRWrapper mw = this->gameWrapper->GetMMRWrapper();
	ArrayWrapper<PriWrapper> pris = server.GetPRIs();

	int len = pris.Count();

	bool needsSave = false;

	for (int i = 0; i < len; i++)
	{
		PriWrapper player = pris.Get(i);

		PlayerControllerWrapper localPlayerCont = server.GetLocalPrimaryPlayer();
		if (!localPlayerCont.IsNull())
		{
			PriWrapper localPlayer = localPlayerCont.GetPRI();
			if (!localPlayer.IsNull())
			{

				bool isTeammate = player.GetTeamNum() == localPlayer.GetTeamNum();
				unsigned long long myLeaderID = localPlayer.GetPartyLeader().ID;
				bool isInMyGroup = myLeaderID != 0 && player.GetPartyLeader().ID == myLeaderID;

				if (isTeammate && !*this->trackTeammates)
				{
					continue;
				}

				if (!isTeammate && !*this->trackOpponents)
				{
					continue;
				}

				if (isInMyGroup && !*this->trackGrouped)
				{
					continue;
				}

				SteamID steamID = player.GetUniqueId();
				LOG(INFO) << "steamID: " << steamID.ID;

				std::string steamIDStr = std::to_string(steamID.ID);

				SteamID localSteamID = localPlayer.GetUniqueId();

				// Skip self
				if (steamID.ID == localSteamID.ID) {
					continue;
				//} else
				//{
				//	this->gameWrapper->LogToChatbox(std::to_string(steamID.ID) + " != " + std::to_string(localSteamID.ID));
				}

				std::string playerName = player.GetPlayerName().ToString();

				// Bots
				if (steamID.ID == 0)
				{
					//playerName = "[BOT]";
					continue;
				}

				int curPlaylist = mw.GetCurrentPlaylist();

				//GetAndSetMetMMR(localSteamID, curPlaylist, steamID);

				//GetAndSetMetMMR(steamID, curPlaylist, steamID);

				// Only do met count logic if we haven't yet
				if (this->currentMatchPRIsMetList.count(steamIDStr) == 0)
				{
					this->currentMatchPRIsMetList.emplace(steamIDStr, player);
					int metCount;
					if (!this->data["players"].contains(steamIDStr))
					{
						metCount = 1;
						this->data["players"][steamIDStr] = json({
							{ "metCount", metCount },
							{ "name", playerName },
							//{ "playerMetMMR", { { std::to_string(curPlaylist), -1 } } },
							//{ "otherMetMMR", { { std::to_string(curPlaylist), -1 } } },
						});
					} else
					{
						json& playerData = this->data["players"][steamIDStr];
						metCount = playerData["metCount"].get<int>();
						metCount++;
						playerData["metCount"] = metCount;
						playerData["name"] = playerName;
					}
					needsSave = true;
				}
				AddPlayerToRenderData(player);

			}
			else {
				LOG(INFO) << "localPlayer is null";
			}

		}
		else {
			LOG(INFO) << "localPlayerController is null";
		}

	}

	if (needsSave)
		WriteData();

}

void DejaVu::AddPlayerToRenderData(PriWrapper player)
{
	auto steamID = player.GetUniqueId().ID;
	std::string steamIDStr = std::to_string(steamID);
	// If we've already added him to the list, return
	if (this->currentMatchPRIs.count(steamIDStr) != 0)
		return;
	auto server = GetCurrentServer();
	auto myTeamNum = server.GetLocalPrimaryPlayer().GetPRI().GetTeamNum();
	auto theirTeamNum = player.GetTeamNum();
	unsigned char spectating = -1;
	if (myTeamNum == spectating || theirTeamNum == spectating)
	{
		LOG(INFO) << "No team set for " << player.GetPlayerName().ToString() << ", retrying in 5 seconds: " << (int)myTeamNum << ":" << (int)theirTeamNum;
		// No team set. Try again in a couple seconds
		this->gameWrapper->SetTimeout([this](GameWrapper* gameWrapper) {
			HandlePlayerAdded("NoTeamSetRetry");
		}, 5);
		return;
	}
	// Team set, so we all good
	this->currentMatchPRIs.emplace(steamIDStr, player);

	LOG(INFO) << "adding player: " << player.GetPlayerName().ToString();

	int metCount = this->data["players"][steamIDStr]["metCount"].get<int>();
	bool sameTeam = theirTeamNum == myTeamNum;
	Record record = GetRecord(steamIDStr, server.GetPlaylist().GetPlaylistId(), sameTeam ? Side::Same : Side::Other);
	LOG(INFO) << "player team num: " << std::to_string(theirTeamNum);
	std::string playerName = player.GetPlayerName().ToString();
	if (theirTeamNum == 0)
		this->blueTeamRenderData.push_back({ steamIDStr, playerName, metCount, record });
	else
		this->orangeTeamRenderData.push_back({ steamIDStr, playerName, metCount, record });
}

void DejaVu::RemovePlayerFromRenderData(PriWrapper player)
{
	LOG(INFO) << "Removing player: " << player.GetPlayerName().ToString();
	std::string steamID = std::to_string(player.GetUniqueId().ID);
	LOG(INFO) << "Player SteamID: " << steamID;
	this->blueTeamRenderData.erase(std::remove_if(this->blueTeamRenderData.begin(), this->blueTeamRenderData.end(), [steamID](const RenderData& data) {
		return data.id == steamID;
	}), this->blueTeamRenderData.end());
	this->orangeTeamRenderData.erase(std::remove_if(this->orangeTeamRenderData.begin(), this->orangeTeamRenderData.end(), [steamID](const RenderData& data) {
		return data.id == steamID;
	}), this->orangeTeamRenderData.end());
}

void DejaVu::HandlePlayerRemoved(std::string eventName)
{
	if (!IsInRealGame())
		return;
	LOG(INFO) << eventName;

	auto server = GetCurrentServer();

	auto pris = server.GetPRIs();
	for (auto it = this->currentMatchPRIs.begin(); it != this->currentMatchPRIs.end(); )
	{
		std::string playerID = it->first;
		auto player = it->second;

		bool isInGame = false;
		for (int i = 0; i < pris.Count(); i++)
		{
			auto playerInGame = pris.Get(i);
			std::string playerInGameID = std::to_string(playerInGame.GetUniqueId().ID);
			if (playerID == playerInGameID)
				isInGame = true;
		}
		if (!isInGame)
		{
			LOG(INFO) << "Player is no longer in game: " << player.GetPlayerName().ToString();
			it = this->currentMatchPRIs.erase(it);
			RemovePlayerFromRenderData(player);
		}
		else
			++it;
	}
}

void DejaVu::HandleGameStart(std::string eventName)
{
	LOG(INFO) << eventName;
	Reset();
	this->cvarManager->getCvar("cl_dejavu_debug").setValue(false);
}

void DejaVu::HandleGameEnd(std::string eventName)
{
	LOG(INFO) << eventName;
	SetRecord();
	WriteData();
	Reset();
	this->gameIsOver = true;
}

void DejaVu::HandleGameLeave(std::string eventName)
{
	LOG(INFO) << eventName;
	WriteData();
	Reset();
}

void DejaVu::Reset()
{
	this->gameIsOver = false;
	this->currentMatchPRIs.clear();
	this->currentMatchPRIsMetList.clear();
	this->blueTeamRenderData.clear();
	this->orangeTeamRenderData.clear();
}

void DejaVu::GetAndSetMetMMR(SteamID steamID, int playlist, SteamID idToSet)
{
	this->gameWrapper->SetTimeout([this, steamID, playlist, idToSet](GameWrapper* gameWrapper) {
		float mmrValue = this->mmrWrapper.GetPlayerMMR(steamID, playlist);
		// For some reason getting a lot of these values 100.01998901367188
		if (mmrValue < 0 && !this->mmrWrapper.IsSynced(steamID, playlist)) {
			Log("Not synced yet: " + std::to_string(mmrValue) + "|" + std::to_string(this->mmrWrapper.IsSyncing(steamID)));
			GetAndSetMetMMR(steamID, playlist, idToSet);
			return;
		}

		json& player = this->data["players"][std::to_string(idToSet.ID)];
		std::string keyToSet = (steamID.ID == idToSet.ID) ? "otherMetMMR" : "playerMetMMR";
		if (!player.contains(keyToSet))
		{
			player[keyToSet] = json::object();
		}
		player[keyToSet][std::to_string(playlist)] = mmrValue;
		WriteData();
	}, 5);
}

Record DejaVu::GetRecord(SteamID steamID, int playlist, Side side)
{
	return GetRecord(std::to_string(steamID.ID), playlist, side);
}

Record DejaVu::GetRecord(std::string steamID, int playlist, Side side)
{
	std::string sideStr;
	if (side == Side::Same)
		sideStr = "with";
	else if (side == Side::Other)
		sideStr = "against";
	else
		return { 0, 0 };

	json playerData = this->data["players"][steamID];
	if (!playerData.contains("playlistData"))
		return { 0, 0 };
	json data = playerData["playlistData"];
	if (!data.contains(std::to_string(playlist)))
		return { 0, 0 };
	json recordJson = data[std::to_string(playlist)]["records"];
	if (recordJson.contains(sideStr))
		return recordJson[sideStr].get<Record>();
	return { 0, 0 };
}

void DejaVu::SetRecord()
{
	if (!IsInRealGame())
		return;

	auto server = this->gameWrapper->GetOnlineGame();
	if (server.IsNull())
		return;

	auto winningTeam = server.GetWinningTeam();
	if (winningTeam.IsNull())
		return;

	auto localPlayer = server.GetLocalPrimaryPlayer();
	if (localPlayer.IsNull())
		return;
	auto localPRI = localPlayer.GetPRI();
	if (localPRI.IsNull())
		return;

	bool myTeamWon = winningTeam.GetTeamNum() == localPRI.GetTeamNum();
	Log(myTeamWon ? "YOU WON!" : "YOU LOST!");
	SteamID myID = localPRI.GetUniqueId();
	auto players = server.GetPRIs();
	for (int i = 0; i < players.Count(); i++)
	{
		auto player = players.Get(i);
		SteamID steamID = player.GetUniqueId();
		if (steamID.ID == myID.ID)
			continue;
		std::string playerID = std::to_string(steamID.ID);

		bool sameTeam = player.GetTeamNum() == localPRI.GetTeamNum();

		int playlist = server.GetPlaylist().GetPlaylistId();
		Record record = GetRecord(steamID, playlist, sameTeam ? Side::Same : Side::Other);
		if (myTeamWon)
			record.wins++;
		else
			record.losses++;

		std::string sideStr;
		if (sameTeam)
			sideStr = "with";
		else
			sideStr = "against";

		this->data["players"][std::to_string(steamID.ID)]["playlistData"][std::to_string(playlist)]["records"][sideStr] = record;
	}
}

bool DejaVu::IsInRealGame()
{
	return this->gameWrapper->IsInOnlineGame() && !this->gameWrapper->IsInReplay();
}

char spacings[] = { 15, 30, 45, 60 };

void DejaVu::RenderDrawable(CanvasWrapper canvas)
{
	static bool haveSetContext = false;
	if (!haveSetContext)
	{
		Canvas::SetContext(canvas);
		haveSetContext = true;
	}

	Canvas::CanvasTableOptions tableOptions;
	tableOptions.bgColor = Canvas::Color(*this->backgroundColorR, *this->backgroundColorG, *this->backgroundColorB);
	tableOptions.fgColor = Canvas::Color(*this->textColorR, *this->textColorG, *this->textColorB);
	tableOptions.borderOptions.borderColor = tableOptions.fgColor;
	tableOptions.borderOptions.borders = Canvas::TableBorder::ALL;
	//Canvas::SetPosition(Vector2{ 50, 50 });
	//Canvas::BeginTable({
	//	{ Canvas::Alignment::RIGHT },
	//	{ Canvas::Alignment::CENTER },
	//	{ Canvas::Alignment::LEFT },
	//	//{ Canvas::Alignment::LEFT },
	//}, tableOptions);
	//Canvas::Row({ "Right", "Center", "Left" });
	//Canvas::Row({ "Adam", "5", "Red", "This is out of bounds so won't be shown" });
	//Canvas::Row({ "Alex", "12345678" });
	//Canvas::Row({ "A$gyifdd", "&%@", ",.|[]" });
	//Canvas::Row({ "Long text to show auto sizing", "one", "Pretty neatoooo" });
	//Canvas::Row({ "Alex", "543" });
	//Canvas::EndTable();

	//int pad = 15;
	//Canvas::SetPosition(pos);
	//Canvas::DrawString("ABCDEFGHIJKLMNOPQRSTUVWXYZA");
	//pos.Y += pad;
	//Canvas::SetPosition(pos);
	//Canvas::DrawString("Aabcdefghijklmnopqrstuvwxyz");
	//pos.Y += pad;
	//Canvas::SetPosition(pos);
	//Canvas::DrawString("A`B1234567890-=");
	//pos.Y += pad;
	//Canvas::SetPosition(pos);
	//Canvas::DrawString("A~!@#$%^&*()_+");
	//pos.Y += pad;
	//Canvas::SetPosition(pos);
	//Canvas::DrawString("A[O[O]O\\O;O'O,O.O/O");
	//pos.Y += pad;
	//Canvas::SetPosition(pos);
	//Canvas::DrawString("O{O}O|O:O\"O<O>O?O");
	//pos.Y += pad;
	//Canvas::SetPosition(pos);
	//Canvas::DrawString("__ __()_(_)Aggggg");

	bool inGame = IsInRealGame();
	bool noData = this->blueTeamRenderData.size() == 0 && this->orangeTeamRenderData.size() == 0;
	if (
		(!*this->enabled || !*this->enabledVisuals || !inGame || noData) && !*this->enabledDebug
		)
		return;

	int spacing = *this->scale * 15;

	Vector2 size = canvas.GetSize();
	Vector2 padding{ 5, 5 };

	int yOffset = 3;

	int blueSize = this->blueTeamRenderData.size();
	int orangeSize = this->orangeTeamRenderData.size();

	bool renderPlayerBlue = false;
	bool renderPlayerOrange = false;
	auto pri = GetLocalPlayerPRI();
	if (!pri.IsNull())
	{
		auto isBlueTeam = pri.GetTeamNum() == 0;
		if (isBlueTeam)
		{
			blueSize++;
			orangeSize = max(orangeSize, 1);
		}
		else
		{
			orangeSize++;
			blueSize = max(blueSize, 1);
		}
		renderPlayerBlue = isBlueTeam;
		renderPlayerOrange = !isBlueTeam;
	}

	if (*this->enabledDebug)
	{
		renderPlayerOrange = true;
		orangeSize++;
	}



	float totalHeight = (blueSize + orangeSize) * spacing + padding.Y * 4 + yOffset;

	float minWidth = 200;
	float height = blueSize * spacing + padding.Y * 2;

	float width = ((size.X - 200 * *this->scale) * *this->width) + 200 * *this->scale;

	float maxX = size.X - width;
	float maxY = size.Y - totalHeight;

	//Canvas::Rect rect{(*this->xPos * maxX), (*this->yPos * maxY), width, height};
	//rect = RenderUI(rect, this->blueTeamRenderData, renderPlayerBlue, false);
	//rect.Y += rect.Height + yOffset;
	//height = orangeSize * spacing + padding.Y * 2;
	//rect.Height = height;
	//RenderUI(rect, this->orangeTeamRenderData, renderPlayerOrange, false);

	static int prevRenderHeight = 0;
	maxY = size.Y - prevRenderHeight;
	height = blueSize * spacing + padding.Y * 2;
	Canvas::Rect rect2{(*this->xPos * maxX), (*this->yPos * maxY), width, height};
	rect2 = RenderUI(rect2, this->blueTeamRenderData, renderPlayerBlue, true);
	prevRenderHeight = rect2.Height;
	rect2.Y += rect2.Height + yOffset;
	rect2 = RenderUI(rect2, this->orangeTeamRenderData, renderPlayerOrange, true);
	prevRenderHeight += rect2.Height + yOffset;
}

Canvas::Rect DejaVu::RenderUI(Canvas::Rect area, const std::vector<RenderData>& renderData, bool renderPlayer, bool newCanvas)
{
	char alphaVal = *this->alpha * 255;
	//char currentCharWidth = charWidths[(*this->scale) - 1];
	Vector2 padding{ 5, 5 };
	int spacing = *this->scale * 15;

	if (!newCanvas)
	{

		if (*this->enabledBackground)
		{
			Canvas::SetColor(*this->backgroundColorR, *this->backgroundColorG, *this->backgroundColorB, alphaVal);
			Canvas::DrawRect(Vector2{ area.X, area.Y }, { area.X + area.Width, area.Y + area.Height });
		}

		int yPos = area.Y + padding.Y;
		Canvas::SetColor(*this->textColorR, *this->textColorG, *this->textColorB, alphaVal);

		if (!renderPlayer && renderData.size() == 0)
		{
			Canvas::SetPosition(Vector2{ area.X + padding.X, yPos });
			Canvas::DrawString("Waiting...", *this->scale, *this->scale);
			return area;
		}

		if (renderPlayer) {
			Canvas::SetPosition(Vector2{ area.X + padding.X, yPos });
			Canvas::DrawString("You", *this->scale, *this->scale);
			yPos += spacing;
		}

		for (auto const& playerRenderData : renderData)
		{
			std::string playerName = playerRenderData.name;
			//auto widthOfNamePx = playerName.length() * currentCharWidth;
			auto widthOfNamePx = Canvas::GetStringWidth(playerName) * (*this->scale);
			Record record = playerRenderData.record;
			int recordWidth = Canvas::GetStringWidth(
				*this->showMetCount ?
				std::to_string(playerRenderData.metCount) :
				std::to_string(record.wins) + ":" + std::to_string(record.losses)
			);
			int remainingPixels = area.Width - (padding.X * 2) - (*this->scale * (recordWidth + Canvas::GetCharWidth('*') + (Canvas::GetCharWidth('.') * 3)));
			if (widthOfNamePx >= remainingPixels) {
				// truncate
				int characters = 0;
				int pixels = 0;
				for (char ch : playerName)
				{
					int width = *this->scale * Canvas::GetCharWidth(ch);
					if ((pixels + width) > remainingPixels)
						break;
					pixels += width;
					characters++;
				}
				playerName = playerName.substr(0, characters) + "...";
			}
			// Append * when we are showing the record, the record is 0:0, and we have met them before
			if (!*this->showMetCount && playerRenderData.metCount > 1 && (record.wins == 0 && record.losses == 0))
				playerName += "*";
			Canvas::SetPosition(Vector2{ area.X + padding.X, yPos });
			Canvas::DrawString(playerName, *this->scale, *this->scale);

			if (*this->showMetCount) {
				Canvas::SetPosition(Vector2{ area.X + area.Width - padding.X - *this->scale * Canvas::GetStringWidth(std::to_string(playerRenderData.metCount)), yPos });
				Canvas::DrawString(std::to_string(playerRenderData.metCount), *this->scale, *this->scale);
			}
			else {
				std::string recordStr = std::to_string(record.wins) + ":" + std::to_string(record.losses);
				Canvas::SetPosition(Vector2{ area.X + area.Width - padding.X - *this->scale * Canvas::GetStringWidth(recordStr), yPos });
				Canvas::DrawString(recordStr, *this->scale, *this->scale);
			}

			yPos += spacing;
		}
		return area;
	}
	else
	{
		Canvas::SetPosition(Vector2{ area.X, area.Y });
		Canvas::BeginAlpha(alphaVal);
		//Canvas::BeginScale(*this->scale);
		Canvas::BeginTable({
			{ Canvas::Alignment::LEFT },
			{ Canvas::Alignment::RIGHT, 10, std::nullopt, *this->showMetCount ? 35 : 65 },
		}, {
			Canvas::Color(*this->backgroundColorR, *this->backgroundColorG, *this->backgroundColorB, *this->enabledBackground ? 255 : 0),
			Canvas::Color(*this->textColorR, *this->textColorG, *this->textColorB),
			area.Width,
			//{ Canvas::Color::Green, Canvas::TableBorder::ALL },
			{},
			{0, 0, 5, 2}
		});
		if (renderPlayer) {
			Canvas::Row({ { "You" }, { "" } });
		}
		for (auto const& playerRenderData : renderData)
		{
			std::string playerName = playerRenderData.name;
			auto widthOfNamePx = Canvas::GetStringWidth(playerName) * (*this->scale);
			Record record = playerRenderData.record;
			// Append * when we are showing the record, the record is 0:0, and we have met them before
			if (!*this->showMetCount && playerRenderData.metCount > 1 && (record.wins == 0 && record.losses == 0))
				playerName += "*";

			std::stringstream data;
			if (*this->showMetCount) {
				data << playerRenderData.metCount;
			}
			else {
				data << record.wins << ":" << record.losses;
			}
			Canvas::Row({ { playerName }, { data.str() } });
			//Canvas::Row({ { playerName }, { "999" } });
		}
		Canvas::Rect r = Canvas::EndTable();
		//Canvas::EndScale();
		Canvas::EndAlpha();
		return r;
	}
}

