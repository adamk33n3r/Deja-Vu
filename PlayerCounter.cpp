#include "PlayerCounter.h"
#include <iomanip>
#include <sstream>
#include <fstream>

BAKKESMOD_PLUGIN(PlayerCounter, "Deja Vu", "1.1.2", 0)

template <class T>
CVarWrapper PlayerCounter::RegisterCVar(
	const char* name,
	const char* description,
	T defaultValue,
	std::shared_ptr<T>& bindTo,
	bool searchable,
	bool hasMin,
	float min,
	float hasMax,
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
CVarWrapper PlayerCounter::RegisterCVar(
	const char* name,
	const char* description,
	std::string defaultValue,
	std::shared_ptr<std::string>& bindTo,
	bool searchable,
	bool hasMin,
	float min,
	float hasMax,
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

void PlayerCounter::onLoad()
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
	}, "Reloads the json data from file", PERMISSION_ONLINE);

	RegisterCVar("cl_dejavu_enable", "Enables plugin", true, this->enabled);

	RegisterCVar("cl_dejavu_track_opponents", "Track players if opponents", true, this->trackOpponents);
	RegisterCVar("cl_dejavu_track_teammates", "Track players if teammates", true, this->trackTeammates);
	RegisterCVar("cl_dejavu_track_grouped", "Track players if in party", true, this->trackGrouped);

	RegisterCVar("cl_dejavu_visuals", "Enables visuals", true, this->enabledVisuals);

	auto debugCVar = RegisterCVar("cl_dejavu_debug", "Enables debug view. Useful for choosing colors", false, this->enabledDebug);
	debugCVar.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		bool val = cvar.getBoolValue();

		if (this->gameWrapper->IsInOnlineGame()) {
			if (val)
				cvar.setValue(false);
			return;
		}

		this->currentMatchMetCounts.clear();

		if (val) {
			this->currentMatchMetCounts["Player Name 1"] = 5;
			this->currentMatchMetCounts["Player Name 2"] = 15;
			this->currentMatchMetCounts["Player Name 3"] = 999;
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

	this->gameWrapper->HookEvent("Function TAGame.GameEvent_TA.EventPlayerAdded", std::bind(&PlayerCounter::HandlePlayerAdded, this, std::placeholders::_1));
	//this->gameWrapper->HookEvent("Function TAGame.GameEvent_TA.EventPlayerRemoved", std::bind(&PlayerCounter::HandlePlayerRemoved, this, std::placeholders::_1));
	this->gameWrapper->HookEvent("Function GameEvent_TA.Countdown.BeginState", std::bind(&PlayerCounter::HandlePlayerAdded, this, std::placeholders::_1));
	this->gameWrapper->HookEvent("Function TAGame.Team_TA.EventScoreUpdated", std::bind(&PlayerCounter::HandlePlayerAdded, this, std::placeholders::_1));

	this->gameWrapper->HookEvent("Function TAGame.GameEvent_TA.InitGame", bind(&PlayerCounter::HandleGameStart, this, std::placeholders::_1));
	this->gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.InitGame", bind(&PlayerCounter::HandleGameStart, this, std::placeholders::_1));
	this->gameWrapper->HookEvent("Function TAGame.GameEvent_Team_TA.InitGame", bind(&PlayerCounter::HandleGameStart, this, std::placeholders::_1));
	this->gameWrapper->HookEvent("Function OnlineGameJoinGame_X.JoiningBase.IsJoiningGame", std::bind(&PlayerCounter::HandleGameStart, this, std::placeholders::_1));

	this->gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.EventMatchEnded", std::bind(&PlayerCounter::HandleGameEnd, this, std::placeholders::_1));
	this->gameWrapper->HookEvent("Function TAGame.GFxShell_TA.LeaveMatch", std::bind(&PlayerCounter::HandleGameEnd, this, std::placeholders::_1));
	//this->gameWrapper->HookEvent("Function TAGame.GFxShell_TA.ExitToMainMenu", std::bind(&PlayerCounter::Log, this, std::placeholders::_1));
	//this->gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed", std::bind(&PlayerCounter::Log, this, std::placeholders::_1));

	this->gameWrapper->UnregisterDrawables();
	this->gameWrapper->RegisterDrawable(bind(&PlayerCounter::RenderDrawable, this, std::placeholders::_1));


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
	*/

	LoadData();
}

void PlayerCounter::onUnload()
{
	WriteData();
}

void PlayerCounter::Log(std::string msg)
{
	this->cvarManager->log(msg);
}

void PlayerCounter::LogError(std::string msg)
{
	this->cvarManager->log("ERROR: " + msg);
}

void PlayerCounter::LoadData()
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


void PlayerCounter::WriteData()
{
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

void PlayerCounter::HandlePlayerAdded(std::string eventName)
{
	if (!this->gameWrapper->IsInOnlineGame())
		return;
	ServerWrapper server = this->gameWrapper->GetOnlineGame();
	Log("Game time is: " + std::to_string(server.GetGameTime()));
	if (server.GetGameTime() <= 0) {
		return;
	}
	MMRWrapper mw = this->gameWrapper->GetMMRWrapper();
	ArrayWrapper<PriWrapper> pris = server.GetPRIs();
	int i = 0;
	int len = pris.Count();

	while (i < len)
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
					i++;
					continue;
				}

				if (!isTeammate && !*this->trackOpponents)
				{
					i++;
					continue;
				}

				if (isInMyGroup && !*this->trackGrouped)
				{
					i++;
					continue;
				}

				SteamID steamID = player.GetUniqueId();
				// Bots
				if (steamID.ID == 0)
				{
					i++;
					continue;
				}

				std::string steamIDStr = std::to_string(steamID.ID);
				std::string playerName = player.GetPlayerName().ToString();
				int curPlaylist = mw.GetCurrentPlaylist();

				if (std::find(this->currentMatchIDs.begin(), this->currentMatchIDs.end(), steamIDStr) != this->currentMatchIDs.end())
				{
					i++;
					continue;
				}

				SteamID localSteamID = localPlayer.GetUniqueId();

				// Skip self
				if (steamID.ID == localSteamID.ID) {
					i++;
					continue;
				//} else
				//{
				//	this->gameWrapper->LogToChatbox(std::to_string(steamID.ID) + " != " + std::to_string(localSteamID.ID));
				}

				GetAndSetMetMMR(localSteamID, curPlaylist, steamID);

				GetAndSetMetMMR(steamID, curPlaylist, steamID);

				this->currentMatchIDs.push_back(steamIDStr);

				int metCount;
				if (!this->data["players"].contains(steamIDStr))
				{
					metCount = 1;
					this->data["players"][steamIDStr] = json({
						{ "metCount", metCount },
						{ "name", playerName },
						{ "playerMetMMR", { { std::to_string(curPlaylist), -1 } } },
						{ "otherMetMMR", { { std::to_string(curPlaylist), -1 } } },
					});
				} else
				{
					json& playerData = this->data["players"][steamIDStr];
					metCount = playerData["metCount"].get<int>();
					metCount++;
					playerData["metCount"] = metCount;
					playerData["name"] = playerName;
				}

				//this->gameWrapper->LogToChatbox(playerName + "|" + std::to_string(metCount));
				this->currentMatchMetCounts[playerName] = metCount;


			} else
			{
				//this->gameWrapper->LogToChatbox("localPlayer is null");
			}

		} else
		{
			//this->gameWrapper->LogToChatbox("localPlayerCont is null");
		}

	}

	i++;

	WriteData();

}

void PlayerCounter::HandleGameStart(std::string eventName)
{
	this->currentMatchIDs.clear();
	this->currentMatchMetCounts.clear();
	this->cvarManager->getCvar("cl_dejavu_debug").setValue(false);
}

void PlayerCounter::HandleGameEnd(std::string eventName)
{
	WriteData();
	this->currentMatchIDs.clear();
	this->currentMatchMetCounts.clear();
}

struct Rect
{
	int X, Y, Width, Height;
};

char charWidths[] = { 8, 16, 25, 32 };
char spacings[] = { 15, 30, 45, 60 };

void PlayerCounter::RenderDrawable(CanvasWrapper canvas)
{
	bool inGame = this->gameWrapper->IsInOnlineGame();
	bool mapEmpty = this->currentMatchMetCounts.size() == 0;
	if (
		(!*this->enabled || !*this->enabledVisuals || !inGame || mapEmpty) && !*this->enabledDebug
	)
		return;

	float alphaVal = *this->alpha;
	Vector2 size = canvas.GetSize();
	Vector2 padding{ 5, 5 };

	int spacing = spacings[(*this->scale)-1];

	float minWidth = 200;
	char currentCharWidth = charWidths[(*this->scale) - 1];
	float height = this->currentMatchMetCounts.size() * spacing + padding.Y * 2;

	float width = ((size.X - 200 * *this->scale) * *this->width) + 200 * *this->scale;

	float maxX = size.X - width;
	float maxY = size.Y - height;

	Rect area{(*this->xPos * maxX), (*this->yPos * maxY), width, height};

	if (*this->enabledBackground)
	{
		canvas.SetColor(*this->backgroundColorR, *this->backgroundColorG, *this->backgroundColorB, 255 * alphaVal);
		canvas.DrawRect(Vector2{ area.X, area.Y }, { area.X + area.Width, area.Y + area.Height });
	}

	int yPos = area.Y + padding.Y;
	canvas.SetColor(*this->textColorR, *this->textColorG, *this->textColorB, 255 * alphaVal);
	for (auto const& val : this->currentMatchMetCounts)
	{
		std::string playerName = val.first;
		auto widthOfNamePx = playerName.length() * currentCharWidth;
		if (widthOfNamePx >= width) {
			// truncate
			playerName = playerName.substr(0, (width / currentCharWidth) - 4 - 3) + "...";
		}
		std::string metCount = std::to_string(val.second);

		canvas.SetPosition(Vector2{ area.X + padding.X, yPos });
		canvas.DrawString(playerName, *this->scale, *this->scale);
		canvas.SetPosition(Vector2{ area.X + area.Width - padding.X - (currentCharWidth * (int)metCount.size()), yPos });
		canvas.DrawString(metCount, *this->scale, *this->scale);
		yPos += spacing;
	}
}

void PlayerCounter::GetAndSetMetMMR(SteamID steamID, int playlist, SteamID idToSet)
{
	this->gameWrapper->SetTimeout([this, steamID, playlist, idToSet](GameWrapper* gameWrapper) {
		float mmrValue = this->mmrWrapper.GetPlayerMMR(steamID, playlist);
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


