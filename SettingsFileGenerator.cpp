#include "pch.h"
#include "DejaVu.h"
#include <fstream>

#define blank setFile << '\n'
#define text(x) setFile << "9|" << x; blank
#define sameline setFile << "7"; blank
#define checkbox(x, y) setFile << "1|" << x << "|" << y; blank
#define separator setFile << "8"; blank
#define greyedStart(x) setFile << "10|" << x; blank
#define greyedEnd setFile << "11"; blank
#define slider(label, var, min, max) setFile << "4|" << label << "|" << var << "|" << min << "|" << max; blank
#define color(label, var) setFile << "13|" << label << "|" << var; blank

void DejaVu::GenerateSettingsFile()
{
	std::ofstream setFile(this->gameWrapper->GetBakkesModPath() / "plugins" / "settings" / "DejaVu.set");

	setFile << "Deja Vu\n";
	checkbox("Enable plugin", CVAR_ENABLED);

	blank;
	separator;
	blank;

	text("Tracking settings");
	checkbox("Track teammates", CVAR_TRACK_TEAMMATES);
	sameline;
	checkbox("Track opponents", CVAR_TRACK_OPPONENTS);
	sameline;
	checkbox("Track your party (applies to both sides)", CVAR_TRACK_GROUPED);

	blank;
	separator;
	blank;

	text("Visual Settings");
	checkbox("Enable visuals", CVAR_VISUALS);
	blank;
	greyedStart(CVAR_VISUALS);
	checkbox("Show preview", CVAR_DEBUG);
	checkbox("Show met count", CVAR_SHOW_MET_COUNT);
	sameline;
	checkbox("Show record", CVAR_SHOW_RECORD);
	sameline;
	checkbox("Show player notes", CVAR_SHOW_PLAYER_NOTES);

	blank;
	separator;
	blank;

	text("Position and size");
	checkbox("Toggle with scoreboard (instead of always on)", CVAR_TOGGLE_WITH_SCOREBOARD);
	slider("Scale", CVAR_SCALE, 1, 4);
	slider("X Pos", CVAR_XPOS, 0, 1);
	slider("Y Pos", CVAR_YPOS, 0, 1);
	slider("Width", CVAR_WIDTH, 0, 1);

	separator;
	text("Colors");
	checkbox("Enable background", CVAR_BACKGROUND);
	color("Text color", CVAR_TEXT_COLOR);
	sameline;
	greyedStart(CVAR_BACKGROUND);
	color("Background color", CVAR_BACKGROUND_COLOR);
	greyedEnd;
	slider("Alpha", CVAR_ALPHA, 0, 1);
	blank;

	greyedEnd;

	setFile.close();

	cvarManager->executeCommand("cl_settings_refreshplugins");
}
