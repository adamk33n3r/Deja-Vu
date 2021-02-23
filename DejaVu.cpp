#include "pch.h"
#include "DejaVu.h"


/**
 * TODO
 * - Add option to show total record across all playlists
 * - Create 2-way cvar binding
 * - IMGUI stuff
 */

INITIALIZE_EASYLOGGINGPP

BAKKESMOD_PLUGIN(DejaVu, "Deja Vu", PluginVersion, 0)

// to_string overloads for cvars
namespace std {
	inline string to_string(const std::string& str) {
		return str;
	}
	string to_string(const LinearColor& color) {
        char buf[49];
        sprintf(buf, "(%f, %f, %f, %f)", color.R, color.G, color.B, color.A);
        return buf;
	}
	string to_string(const Record& record) {
		return to_string(record.wins) + ":" + to_string(record.losses);
	}
}

template <class T>
CVarWrapper DejaVu::RegisterCVar(
	const char* name,
	const char* description,
	std::shared_ptr<T>& bindTo,
	bool searchable,
	bool hasMin,
	float min,
	bool hasMax,
	float max,
	bool saveToCfg
)
{
	CVarWrapper cvar = this->cvarManager->registerCvar(
		name,
		"",
		description,
		searchable,
		hasMin,
		min,
		hasMax,
		max,
		saveToCfg
	);
	cvar.setValue(*bindTo);
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
	this->gameWrapper->HookEvent(eventName, std::bind(&DejaVu::Log, this, std::placeholders::_1));
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

	this->dataDir = this->gameWrapper->GetDataFolder().append("dejavu");
	this->mainPath = std::filesystem::path(dataDir).append(this->mainFile);
	this->tmpPath = std::filesystem::path(dataDir).append(this->tmpFile);
	this->bakPath = std::filesystem::path(dataDir).append(this->bakFile);
	this->logPath = std::filesystem::path(dataDir).append(this->logFile);

	this->cvarManager->registerNotifier("dejavu_reload", [this](const std::vector<std::string>& commands) {
		LoadData();
	}, "Reloads the json data from file", PERMISSION_ALL);

	this->cvarManager->registerNotifier("dejavu_track_current", [this](const std::vector<std::string>& commands) {
		HandlePlayerAdded("dejavu_track_current");
	}, "Tracks current lobby", PERMISSION_ONLINE);

	this->cvarManager->registerNotifier("dejavu_launch_quick_note", [this](const std::vector<std::string>& commands) {
#if !DEV
		if (IsInRealGame())
#endif !DEV
			LaunchQuickNoteModal();
		}, "Launches the quick note modal", PERMISSION_ONLINE);

#if DEV
	this->cvarManager->registerNotifier("dejavu_test", [this](const std::vector<std::string>& commands) {
		this->gameWrapper->SetTimeout([this](GameWrapper* gameWrapper) {
			Log("test after 5");
		}, 5);
	}, "test", PERMISSION_ALL);

	this->cvarManager->registerNotifier("dejavu_cleanup", [this](const std::vector<std::string>& commands) {
		CleanUpJson();
	}, "Cleans up the json", PERMISSION_ALL);

	this->cvarManager->registerNotifier("dejavu_dump_list", [this](const std::vector<std::string>& commands) {
		if (this->matchesMetLists.size() == 0)
		{
			this->cvarManager->log("No entries in list");
			return;
		}

		for (auto& match : this->matchesMetLists)
		{
			std::string guid = match.first;
			this->cvarManager->log("For match GUID:" + guid);
			auto& set = match.second;
			for (auto& playerID : set)
			{
				this->cvarManager->log("    " + playerID);
			}
		}
	}, "Dumps met list", PERMISSION_ALL);
	this->cvarManager->registerNotifier("dejavu_test_cvar_binding", [this](const std::vector<std::string>& commands) {
		this->enabledDebug = !*this->enabledDebug;
	}, "Tests cvar binding", PERMISSION_ALL);
#endif DEV


#pragma region register cvars

	this->enabled.Register(this->cvarManager);

	this->trackOpponents.Register(this->cvarManager);
	this->trackTeammates.Register(this->cvarManager);
	this->trackGrouped.Register(this->cvarManager);
	this->showMetCount.Register(this->cvarManager);
	this->showRecord.Register(this->cvarManager);

	auto visualCVar = this->enabledVisuals.Register(this->cvarManager);
	visualCVar.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		if (!cvar.getBoolValue())
			this->enabledDebug = false;
	});
	this->toggleWithScoreboard.Register(this->cvarManager);
	this->showNotes.Register(this->cvarManager);

	auto debugCVar = this->enabledDebug.Register(this->cvarManager);
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
			this->blueTeamRenderData.push_back({ "0", "Blue Player 1", 5, { 5, 5 }, "This guy was a great team player" });
			this->blueTeamRenderData.push_back({ "0", "Blue Player 2", 15, { 15, 15 }, "Quick chat spammer" });
			this->blueTeamRenderData.push_back({ "0", "Blue Player 3 with a loooonngggg name", 999, { 999, 999 } });
			this->orangeTeamRenderData.push_back({ "0", "Orange Player 1", 5, { 5, 5 } });
			this->orangeTeamRenderData.push_back({ "0", "Orange Player 2", 15, { 15, 15 }, "Left early" });
			//this->orangeTeamRenderData.push_back({ "0", "Orange Player 3", 999, { 999, 999 } });
		}
	});
#if DEV
	this->enabledDebug = true;
#endif DEV

	this->scale.Register(this->cvarManager);
	
	this->alpha.Register(this->cvarManager);
	
	this->xPos.Register(this->cvarManager);
	this->yPos.Register(this->cvarManager);
	this->width.Register(this->cvarManager);

#pragma warning(suppress : 4996)
	this->textColorR.Register(this->cvarManager);
#pragma warning(suppress : 4996)
	this->textColorG.Register(this->cvarManager);
#pragma warning(suppress : 4996)
	this->textColorB.Register(this->cvarManager);
	this->textColor.Register(this->cvarManager);

	this->enabledBackground.Register(this->cvarManager);

#pragma warning(suppress : 4996)
	this->backgroundColorR.Register(this->cvarManager);
#pragma warning(suppress : 4996)
	this->backgroundColorG.Register(this->cvarManager);
#pragma warning(suppress : 4996)
	this->backgroundColorB.Register(this->cvarManager);
	this->backgroundColor.Register(this->cvarManager);

	this->hasUpgradedColors.Register(this->cvarManager);

	auto logCVar = this->enabledLog.Register(this->cvarManager);
	logCVar.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		bool val = cvar.getBoolValue();

		SetupLogger(this->logPath.string(), val);
	});

#pragma endregion register cvars

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
	// Function TAGame.GFxHUD_TA.HandlePenaltyChanged

	this->gameWrapper->HookEvent("Function TAGame.GFxData_GameEvent_TA.OnOpenScoreboard", std::bind(&DejaVu::OpenScoreboard, this, std::placeholders::_1));
	this->gameWrapper->HookEvent("Function TAGame.GFxData_GameEvent_TA.OnCloseScoreboard", std::bind(&DejaVu::CloseScoreboard, this, std::placeholders::_1));

	this->gameWrapper->UnregisterDrawables();
	this->gameWrapper->RegisterDrawable(bind(&DejaVu::RenderDrawable, this, std::placeholders::_1));

	/*
	HookEventWithCaller<ServerWrapper>("FUNCTION", bind(&CLASSNAME::FUNCTIONNAME, this, placeholders::_1, 2, 3);
	void CLASSNAME::FUNCTIONNAME(ServerWrapper caller, void* params, string funcName)
	{
		bool returnVal = (bool)params;
	}
	*/

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

	GenerateSettingsFile();


	this->gameWrapper->SetTimeout([this](GameWrapper* gameWrapper) {
		if (!*this->hasUpgradedColors)
		{
			LOG(INFO) << "Upgrading colors...";
#pragma warning(suppress : 4996)
			this->textColor = LinearColor{ (float)*this->textColorR, (float)*this->textColorG, (float)*this->textColorB, 0xff };
#pragma warning(suppress : 4996)
			this->backgroundColor = LinearColor{ (float)*this->backgroundColorR, (float)*this->backgroundColorG, (float)*this->backgroundColorB, 0xff };
			this->hasUpgradedColors = true;
			this->cvarManager->executeCommand("writeconfig");
		}

		LOG(INFO) << "---DEJAVU LOADED---";
	}, 0.001f);

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

	if (this->isWindowOpen)
		this->cvarManager->executeCommand("togglemenu " + GetMenuName());
}


void DejaVu::OpenScoreboard(std::string eventName)
{
	this->isScoreboardOpen = true;
}


void DejaVu::CloseScoreboard(std::string eventName)
{
	this->isScoreboardOpen = false;
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

std::optional<std::string> DejaVu::GetMatchGUID()
{
	ServerWrapper server = GetCurrentServer();
	if (server.IsNull())
		return std::nullopt;
	if (server.IsPlayingPrivate())
		return std::nullopt;
	const std::string& curMatchGUID = server.GetMatchGUID();
	if (curMatchGUID == "No worldInfo" || curMatchGUID.length() == 0)
		return std::nullopt;
	return curMatchGUID;
}

ServerWrapper DejaVu::GetCurrentServer()
{
	if (this->gameWrapper->IsInReplay())
		return this->gameWrapper->GetGameEventAsReplay().memory_address;
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
	ServerWrapper server = GetCurrentServer();
	LOG(INFO) << "server is null: " << (server.IsNull() ? "true" : "false");
	if (server.IsNull())
		return;
	if (server.IsPlayingPrivate())
		return;
	std::string matchGUID = server.GetMatchGUID();
	LOG(INFO) << "Match GUID: " << matchGUID;
	// Too early I guess, so bail since we need the match guid for tracking
	if (matchGUID == "No worldInfo")
		return;
	MMRWrapper mw = this->gameWrapper->GetMMRWrapper();
	ArrayWrapper<PriWrapper> pris = server.GetPRIs();

	int len = pris.Count();

	bool needsSave = false;

	for (int i = 0; i < len; i++)
	{
		PriWrapper player = pris.Get(i);
		bool isSpectator = player.GetbIsSpectator();
		if (isSpectator)
		{
			LOG(INFO) << "player is spectator. skipping";
			continue;
		}

		PriWrapper localPlayer = GetLocalPlayerPRI();
		if (!localPlayer.IsNull())
		{

			bool isTeammate = player.GetTeamNum() == localPlayer.GetTeamNum();
			std::string myLeaderID = localPlayer.GetPartyLeaderID().str();
			bool isInMyGroup = myLeaderID != "0" && player.GetPartyLeaderID().str() == myLeaderID;

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

			UniqueIDWrapper uniqueID = player.GetUniqueIdWrapper();

			std::string uniqueIDStr = uniqueID.str();

			UniqueIDWrapper localUniqueID = localPlayer.GetUniqueIdWrapper();
			std::string localUniqueIDStr = localUniqueID.str();

			// Skip self
			if (uniqueIDStr == localUniqueIDStr) {
				continue;
			//} else
			//{
				//this->gameWrapper->LogToChatbox(uniqueID.str() + " != " + localUniqueID.str());
			}

			std::string playerName = player.GetPlayerName().ToString();
			LOG(INFO) << "uniqueID: " << uniqueIDStr << " name: " << playerName;

			// Bots
			if (uniqueIDStr == "0")
			{
				playerName = "[BOT]";
				continue;
			}

			int curPlaylist = mw.GetCurrentPlaylist();

			//GetAndSetMetMMR(localUniqueID, curPlaylist, uniqueID);

			//GetAndSetMetMMR(uniqueID, curPlaylist, uniqueID);

			// Only do met count logic if we haven't yet
			if (this->matchesMetLists[matchGUID].count(uniqueIDStr) == 0)
			{
				LOG(INFO) << "Haven't processed yet: " << playerName;
				this->matchesMetLists[matchGUID].emplace(uniqueIDStr);
				int metCount = 0;
				if (!this->data["players"].contains(uniqueIDStr))
				{
					LOG(INFO) << "Haven't met yet: " << playerName;
					metCount = 1;
					std::time_t now = std::time(0);
					auto dateTime = std::ctime(&now);
					this->data["players"][uniqueIDStr] = json({
						{ "metCount", metCount },
						{ "name", playerName },
						{ "timeMet", dateTime },
					});
				} else
				{
					LOG(INFO) << "Have met before: " << playerName;
					std::time_t now = std::time(0);
					auto dateTime = std::ctime(&now);
					json& playerData = this->data["players"][uniqueIDStr];
					try
					{
						metCount = playerData["metCount"].get<int>();
						metCount++;
					}
					catch (const std::exception& e)
					{
						this->gameWrapper->Toast("DejaVu Error", "Check console/log for details");
						this->cvarManager->log(e.what());
						LOG(INFO) << e.what();
					}
					playerData["metCount"] = metCount;
					playerData["name"] = playerName;
					playerData["updatedAt"] = dateTime;
				}
				needsSave = true;
			}
			AddPlayerToRenderData(player);

		}
		else {
			LOG(INFO) << "localPlayer is null";
		}

	}

	if (needsSave)
		WriteData();

}

void DejaVu::AddPlayerToRenderData(PriWrapper player)
{
	auto uniqueIDStr = player.GetUniqueIdWrapper().str();
	// If we've already added him to the list, return
	if (this->currentMatchPRIs.count(uniqueIDStr) != 0)
		return;
	auto server = GetCurrentServer();
	auto myTeamNum = server.GetLocalPrimaryPlayer().GetPRI().GetTeamNum();
	auto theirTeamNum = player.GetTeamNum();
	if (myTeamNum == TEAM_NOT_SET || theirTeamNum == TEAM_NOT_SET)
	{
		LOG(INFO) << "No team set for " << player.GetPlayerName().ToString() << ", retrying in 5 seconds: " << (int)myTeamNum << ":" << (int)theirTeamNum;
		// No team set. Try again in a couple seconds
		this->gameWrapper->SetTimeout([this](GameWrapper* gameWrapper) {
			HandlePlayerAdded("NoTeamSetRetry");
		}, 5);
		return;
	}
	// Team set, so we all good
	this->currentMatchPRIs.emplace(uniqueIDStr, player);

	LOG(INFO) << "adding player: " << player.GetPlayerName().ToString();

	int metCount = 0;
	try
	{
		metCount = this->data["players"][uniqueIDStr]["metCount"].get<int>();
	}
	catch (const std::exception& e)
	{
		this->gameWrapper->Toast("DejaVu Error", "Check console/log for details");
		this->cvarManager->log(e.what());
		LOG(INFO) << e.what();
	}
	bool sameTeam = theirTeamNum == myTeamNum;
	Record record = GetRecord(uniqueIDStr, server.GetPlaylist().GetPlaylistId(), sameTeam ? Side::Same : Side::Other);
	LOG(INFO) << "player team num: " << std::to_string(theirTeamNum);
	std::string playerName = player.GetPlayerName().ToString();
	std::string playerNote = this->data["players"][uniqueIDStr].value("note", "");
	if (theirTeamNum == TEAM_BLUE)
		this->blueTeamRenderData.push_back({ uniqueIDStr, playerName, metCount, record, playerNote });
	else
		this->orangeTeamRenderData.push_back({ uniqueIDStr, playerName, metCount, record, playerNote });
}

void DejaVu::RemovePlayerFromRenderData(PriWrapper player)
{
	if (player.IsNull())
		return;
	if (!player.GetPlayerName().IsNull())
		LOG(INFO) << "Removing player: " << player.GetPlayerName().ToString();
	std::string steamID = player.GetUniqueIdWrapper().str();
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
			std::string playerInGameID = playerInGame.GetUniqueIdWrapper().str();
			if (playerID == playerInGameID)
				isInGame = true;
		}
		if (!isInGame)
		{
			if (!player.IsNull() && !player.GetPlayerName().IsNull())
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
	// Maybe move this to HandleGameLeave but probably don't need to worry about it
	//this->matchesMetLists.clear();
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

Record DejaVu::GetRecord(UniqueIDWrapper uniqueID, int playlist, Side side)
{
	return GetRecord(uniqueID.str(), playlist, side);
}

Record DejaVu::GetRecord(std::string uniqueID, Playlist playlist, Side side)
{
	return GetRecord(uniqueID, static_cast<int>(playlist), side);
}

Record DejaVu::GetRecord(std::string uniqueID, int playlist, Side side)
{
	std::string sideStr;
	if (side == Side::Same)
		sideStr = "with";
	else if (side == Side::Other)
		sideStr = "against";
	else
		return { 0, 0 };

	json playerData = this->data["players"][uniqueID];
	if (!playerData.contains("playlistData"))
		return { 0, 0 };
	json data = playerData["playlistData"];

	if (playlist == -1)
	{
		Record combinedRecord{};
		for (auto it = data.begin(); it != data.end(); ++it)
		{
			auto temp = GetRecord(uniqueID, std::stoi(it.key()), side);
			combinedRecord.wins += temp.wins;
			combinedRecord.losses += temp.losses;
		}

		return combinedRecord;
	}

	if (!data.contains(std::to_string(playlist)))
		return { 0, 0 };
	json recordJson = data[std::to_string(playlist)]["records"];
	if (recordJson.contains(sideStr))
	{
		try
		{
			return recordJson[sideStr].get<Record>();
		}
		catch (const std::exception& e)
		{
			this->gameWrapper->Toast("DejaVu Error", "Check console/log for details");
			this->cvarManager->log(e.what());
			LOG(INFO) << e.what();
			return { 0, 0 };
		}
	}
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
	UniqueIDWrapper myID = localPRI.GetUniqueIdWrapper();
	auto players = server.GetPRIs();
	for (int i = 0; i < players.Count(); i++)
	{
		auto player = players.Get(i);
		std::string uniqueIDStr = player.GetUniqueIdWrapper().str();
		if (uniqueIDStr == myID.str() || uniqueIDStr == "0")
			continue;

		bool sameTeam = player.GetTeamNum() == localPRI.GetTeamNum();

		int playlist = server.GetPlaylist().GetPlaylistId();
		Record record = GetRecord(uniqueIDStr, playlist, sameTeam ? Side::Same : Side::Other);
		if (myTeamWon)
			record.wins++;
		else
			record.losses++;

		std::string sideStr;
		if (sameTeam)
			sideStr = "with";
		else
			sideStr = "against";

		this->data["players"][uniqueIDStr]["playlistData"][std::to_string(playlist)]["records"][sideStr] = record;
	}
}

bool DejaVu::IsInRealGame()
{
	return this->gameWrapper->IsInOnlineGame() && !this->gameWrapper->IsInReplay() && !this->gameWrapper->IsInFreeplay();
}

static float MetCountColumnWidth;
static float RecordColumnWidth;
void DejaVu::RenderDrawable(CanvasWrapper canvas)
{
	if (!Canvas::IsContextSet())
	{
		Canvas::SetContext(canvas);
		MetCountColumnWidth = Canvas::GetStringWidth("999") + 11;
		RecordColumnWidth = Canvas::GetStringWidth("999:999") + 11;
	}
	Canvas::SetGlobalAlpha((char)(*this->alpha * 255));
	Canvas::SetScale(*this->scale);

	bool inGame = IsInRealGame();
	bool noData = this->blueTeamRenderData.size() == 0 && this->orangeTeamRenderData.size() == 0;
	bool scoreboardIsClosedAndToggleIsOn = *this->toggleWithScoreboard && !this->isScoreboardOpen;
	if (
		(!*this->enabled || !*this->enabledVisuals || !inGame || noData || scoreboardIsClosedAndToggleIsOn) && !*this->enabledDebug
		)
		return;

	int blueSize = (int)this->blueTeamRenderData.size();
	int orangeSize = (int)this->orangeTeamRenderData.size();

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

	float yOffset = 3.0f;
	float totalHeight = (blueSize + orangeSize) * Canvas::GetTableRowHeight() + 7 * 2 + yOffset - 1;
	//                                                                          ^^^^^
	//                                                                    borderless padding

	Vector2 canvasSize = Canvas::GetSize();
	float tableWidth = 150;
	if (*this->showMetCount)
		tableWidth += MetCountColumnWidth;
	if (*this->showRecord)
		tableWidth += RecordColumnWidth;
	if (*this->showNotes)
		tableWidth += 150;
	float width = ((canvasSize.X - tableWidth * *this->scale) * *this->width) + tableWidth * *this->scale;

	float maxX = canvasSize.X - width;
	float maxY = canvasSize.Y - totalHeight;

	std::vector<Canvas::CanvasColumnOptions> columnOptions{
		{ Canvas::Alignment::LEFT },
	};

	//TODO: allow setting a max width for a column (for the name column)
	//TODO: allow toggling table borders (and fix sizing of tables while borders are active)
	if (*this->showMetCount)
		columnOptions.push_back({ Canvas::Alignment::RIGHT, MetCountColumnWidth });

	if (*this->showRecord)
		columnOptions.push_back({ Canvas::Alignment::RIGHT, RecordColumnWidth });

	if (*this->showNotes)
		columnOptions.push_back({ Canvas::Alignment::LEFT });

	Canvas::CanvasTableOptions tableOptions{
		Canvas::to_color(*this->textColor),
		*this->enabledBackground,
		Canvas::to_color(*this->backgroundColor),
		false,
		Canvas::Color::WHITE,
		width,
	};

	Canvas::SetPosition(*this->xPos * maxX, *this->yPos * maxY);
	RenderUI(this->blueTeamRenderData, tableOptions, columnOptions, renderPlayerBlue);

	Canvas::SetPosition(Canvas::GetPositionFloat() + Vector2F{ 0, yOffset });
	RenderUI(this->orangeTeamRenderData, tableOptions, columnOptions, renderPlayerOrange);
}

void DejaVu::RenderUI(const std::vector<RenderData>& renderData, const Canvas::CanvasTableOptions& tableOptions, const std::vector<Canvas::CanvasColumnOptions>& columnOptions, const bool renderPlayer)
{
	Canvas::BeginTable(tableOptions);
	Canvas::Columns(columnOptions);

	if (renderPlayer)
		Canvas::Row({ "You" });
	else if (renderData.size() == 0)
		Canvas::Row({ "Waiting..." });

	for (const auto& player : renderData)
	{
		std::string playerName = player.name;
		// Only show * if not showing met count, we've met them, and record is 0:0
		if (!*this->showMetCount && player.metCount > 1 && (player.record.wins == 0 && player.record.losses == 0))
			playerName += "*";
		std::vector<std::string> rowData{ playerName };
		if (*this->showMetCount)
			rowData.push_back(std::to_string(player.metCount));
		if (*this->showRecord)
			rowData.push_back(std::to_string(player.record));
		if (*this->showNotes)
			rowData.push_back(player.note);
		Canvas::Row(rowData);
	}

	Canvas::EndTable();
}

