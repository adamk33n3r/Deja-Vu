#include "PlayerCounter.h"
#include <iomanip>
#include <sstream>
#include <fstream>

BAKKESMOD_PLUGIN(PlayerCounter, "Deja Vu", "1.0.3", 0)

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

	this->enabled = std::make_shared<bool>(true);
	this->cvarManager->registerCvar("cl_dejavu_enable", "1", "Enables plugin").bindTo(this->enabled);

	this->trackOpponents = std::make_shared<bool>(true);
	this->cvarManager->registerCvar("cl_dejavu_track_opponents", "1", "Enables tracking opponents").bindTo(this->trackOpponents);
	this->trackTeammates = std::make_shared<bool>(true);
	this->cvarManager->registerCvar("cl_dejavu_track_teammates", "1", "Enables tracking teammates").bindTo(this->trackTeammates);
	this->trackGrouped = std::make_shared<bool>(true);
	this->cvarManager->registerCvar("cl_dejavu_track_grouped", "1", "Track players if grouped").bindTo(this->trackGrouped);

	this->enabledVisuals = std::make_shared<bool>(true);
	this->cvarManager->registerCvar("cl_dejavu_visuals", "1", "Enables visuals").bindTo(this->enabledVisuals);

	this->enabledDebug = std::make_shared<bool>(false);
	auto debugCVar = this->cvarManager->registerCvar("cl_dejavu_debug", "0", "Enables debug view. Useful for choosing colors");
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
	debugCVar.bindTo(this->enabledDebug);

	this->scale = std::make_shared<int>(1);
	this->cvarManager->registerCvar("cl_dejavu_scale", "1", "Scale of visuals").bindTo(this->scale);
	
	this->alpha = std::make_shared<float>(0.75f);
	this->cvarManager->registerCvar("cl_dejavu_alpha", "0.75", "Alpha of display", true, true, 0.0f, true, 1.0f).bindTo(this->alpha);
	
	this->xPos = std::make_shared<float>(0.0f);
	this->cvarManager->registerCvar("cl_dejavu_xpos", "0.0", "X position of display", true, true, 0.0f, true, 1.0f).bindTo(this->xPos);
	this->yPos = std::make_shared<float>(1.0f);
	this->cvarManager->registerCvar("cl_dejavu_ypos", "1.0", "Y position of display", true, true, 0.0f, true, 1.0f).bindTo(this->yPos);

	this->textColorR = std::make_shared<int>(255);
	this->cvarManager->registerCvar("cl_dejavu_text_color_r", "255", "Text color: Red").bindTo(this->textColorR);
	this->textColorG = std::make_shared<int>(255);
	this->cvarManager->registerCvar("cl_dejavu_text_color_g", "255", "Text color: Green").bindTo(this->textColorG);
	this->textColorB = std::make_shared<int>(255);
	this->cvarManager->registerCvar("cl_dejavu_text_color_b", "255", "Text color: Blue").bindTo(this->textColorB);

	this->enabledBackground = std::make_shared<bool>(true);
	this->cvarManager->registerCvar("cl_dejavu_background", "1", "Enables background").bindTo(this->enabledBackground);

	this->backgroundColorR = std::make_shared<int>(0);
	this->cvarManager->registerCvar("cl_dejavu_background_color_r", "0", "Background color: Red").bindTo(this->backgroundColorR);
	this->backgroundColorG = std::make_shared<int>(0);
	this->cvarManager->registerCvar("cl_dejavu_background_color_g", "0", "Background color: Green").bindTo(this->backgroundColorG);
	this->backgroundColorB = std::make_shared<int>(0);
	this->cvarManager->registerCvar("cl_dejavu_background_color_b", "0", "Background color: Blue").bindTo(this->backgroundColorB);

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
	std::ifstream in("player_counter.json");
	if (in.fail()) {
		Log("Failed to open file");
		Log(strerror(errno));
		this->data["players"] = json::object();
		WriteData();
		in.open("player_counter.json");
	}

	try {
		in >> this->data;
	}
	catch (const nlohmann::detail::exception& e) {
		in.close();
		Log("Failed to parse json");
		Log(e.what());
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
	std::ofstream out("player_counter.json.tmp");
	try {
		out << this->data.dump(4, ' ', false, json::error_handler_t::replace) << std::endl;
		out.close();
		remove("player_counter.json.bak");
		int err = rename("player_counter.json", "player_counter.json.bak");
		if (err != 0) {
			LogError("Could not backup player counter");
			return;
		}
		err = rename("player_counter.json.tmp", "player_counter.json");
		if (err != 0) {
			LogError("Could not move temp file to main");
			err = rename("player_counter.json.bak", "player_counter.json");
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
	this->gameWrapper->LogToChatbox(eventName);
	if (!this->gameWrapper->IsInOnlineGame())
		return;
	ServerWrapper server = this->gameWrapper->GetOnlineGame();
	if (server.GetGameTime() <= 0)
		return;
	//server.GetGameWinner().
	MMRWrapper mw = this->gameWrapper->GetMMRWrapper();
	ArrayWrapper<PriWrapper> pris = server.GetPRIs();
	int i = 0;
	int len = pris.Count();
	Log("track teammates: " + std::to_string(*this->trackTeammates));
	Log("track opponents: " + std::to_string(*this->trackOpponents));
	Log("track group: " + std::to_string(*this->trackGrouped));

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
				Log("player teamnum: " + std::to_string(player.GetTeamNum()));
				Log("my teamnum: " + std::to_string(localPlayer.GetTeamNum()));
				Log("isTeammate: " + std::string(isTeammate ? "true" : "false"));
				Log("player leader: " + std::to_string(player.GetPartyLeader().ID));
				Log("my leader: " + std::to_string(localPlayer.GetPartyLeader().ID));
				Log("isGrouped: " + std::string(isInMyGroup ? "true" : "false"));

				if (isTeammate && !*this->trackTeammates)
				{
					Log("skipping because teammate");
					i++;
					continue;
				}

				if (!isTeammate && !*this->trackOpponents)
				{
					Log("skipping because opponent");
					i++;
					continue;
				}

				if (isInMyGroup && !*this->trackGrouped)
				{
					Log("skipping because in my group");
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
					Log("Player ID already seen this match");
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
	if (
		!*this->enabledVisuals &&
		(!*this->enabled || !*this->enabledDebug || !this->gameWrapper->IsInOnlineGame() || this->currentMatchMetCounts.size() == 0)
	)
		return;

	float alphaVal = *this->alpha;
	Vector2 size = canvas.GetSize();
	Vector2 padding{ 5, 5 };

	int spacing = spacings[(*this->scale)-1];

	float width = 200 * *this->scale;
	float height = this->currentMatchMetCounts.size() * spacing + padding.Y * 2;

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
		std::string metCount = std::to_string(val.second);

		canvas.SetPosition(Vector2{ area.X + padding.X, yPos });
		canvas.DrawString(playerName, *this->scale, *this->scale);
		canvas.SetPosition(Vector2{ area.X + area.Width - padding.X - (charWidths[(*this->scale)-1] * (int)metCount.size()), yPos });
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

		//Log("Finally synced!");
		//gameWrapper->LogToChatbox("Finally Synced!");

		//float mmrValue = this->mmrWrapper.GetPlayerMMR(steamID, playlist);
		//Log(std::to_string(idToSet.ID));
		json& player = this->data["players"][std::to_string(idToSet.ID)];
		//Log(player.dump(2));
		std::string keyToSet = (steamID.ID == idToSet.ID) ? "otherMetMMR" : "playerMetMMR";
		if (!player.contains(keyToSet))
		{
			player[keyToSet] = json::object();
		}
		player[keyToSet][std::to_string(playlist)] = mmrValue;
		//Log(player.dump(2));
		WriteData();
	}, 5);
}
