#include "pch.h"
#include <fstream>

#include "DejaVu.h"

#define blank setFile << '\n'
#define text(x) setFile << "9|" << x; blank
#define sameline setFile << "7"; blank
#define checkbox(x, y) setFile << "1|" << x << "|" << y; blank
#define separator setFile << "8"; blank
#define greyedStart(x) setFile << "10|" << x; blank
#define greyedEnd setFile << "11"; blank
#define slider(label, var, min, max) setFile << "4|" << label << "|" << var << "|" << min << "|" << max; blank
#define color(label, var) setFile << "13|" << label << "|" << var; blank
#define dropdown(label, var, options) setFile << "6|" << label << "|" << var << "|" << options; blank

#define KEYBINDS "None@None&F1@F1&F2@F2&F3@F3&F4@F4&F5@F5&F6@F6&F7@F7&F8@F8&F9@F9&F10@F10&F11@F11&F12@F12&A@A&B@B&C@C&D@D&E@E&F@F&G@G&H@H&I@I&J@J&K@K&L@L&M@M&N@N&O@O&P@P&Q@Q&R@R&S@S&T@T&U@U&V@V&W@W&X@X&Y@Y&Z@Z&Escape@Escape&Tab@Tab&Tilde@Tilde&ScrollLock@ScrollLock&Pause@Pause&one@one&two@two&three@three&four@four&five@five&six@six&seven@seven&eight@eight&nine@nine&zero@zero&Underscore@Underscore&Equals@Equals&Backslash@Backslash&LeftBracket@LeftBracket&RightBracket@RightBracket&Enter@Enter&CapsLock@CapsLock&Semicolon@Semicolon&Quote@Quote&LeftShift@LeftShift&Comma@Comma&Period@Period&Slash@Slash&RightShift@RightShift&LeftControl@LeftControl&LeftAlt@LeftAlt&SpaceBar@SpaceBar&RightAlt@RightAlt&RightControl@RightControl&Left@Left&Up@Up&Down@Down&Right@Right&Home@Home&End@End&Insert@Insert&PageUp@PageUp&Delete@Delete&PageDown@PageDown&NumLock@NumLock&Divide@Divide&Multiply@Multiply&Subtract@Subtract&Add@Add&NumPadOne@NumPadOne&NumPadTwo@NumPadTwo&NumPadThree@NumPadThree&NumPadFour@NumPadFour&NumPadFive@NumPadFive&NumPadSix@NumPadSix&NumPadSeven@NumPadSeven&NumPadEight@NumPadEight&NumPadNine@NumPadNine&NumPadZero@NumPadZero&Decimal@Decimal&LeftMouseButton@LeftMouseButton&RightMouseButton@RightMouseButton&ThumbMouseButton@ThumbMouseButton&ThumbMouseButton2@ThumbMouseButton2&MouseScrollUp@MouseScrollUp&MouseScrollDown@MouseScrollDown&MouseX@MouseX&MouseY@MouseY&XboxTypeS_LeftThumbStick@XboxTypeS_LeftThumbStick&XboxTypeS_RightThumbStick@XboxTypeS_RightThumbStick&XboxTypeS_DPad_Up@XboxTypeS_DPad_Up&XboxTypeS_DPad_Left@XboxTypeS_DPad_Left&XboxTypeS_DPad_Right@XboxTypeS_DPad_Right&XboxTypeS_DPad_Down@XboxTypeS_DPad_Down&XboxTypeS_Back@XboxTypeS_Back&XboxTypeS_Start@XboxTypeS_Start&XboxTypeS_Y@XboxTypeS_Y&XboxTypeS_X@XboxTypeS_X&XboxTypeS_B@XboxTypeS_B&XboxTypeS_A@XboxTypeS_A&XboxTypeS_LeftShoulder@XboxTypeS_LeftShoulder&XboxTypeS_RightShoulder@XboxTypeS_RightShoulder&XboxTypeS_LeftTrigger@XboxTypeS_LeftTrigger&XboxTypeS_RightTrigger@XboxTypeS_RightTrigger&XboxTypeS_LeftTriggerAxis@XboxTypeS_LeftTriggerAxis&XboxTypeS_RightTriggerAxis@XboxTypeS_RightTriggerAxis&XboxTypeS_LeftX@XboxTypeS_LeftX&XboxTypeS_LeftY@XboxTypeS_LeftY&XboxTypeS_RightX@XboxTypeS_RightX&XboxTypeS_RightY@XboxTypeS_RightY"

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
	color("Text color", CVAR_TEXT_COLOR);
	checkbox("Enable borders", CVAR_BORDERS);
	sameline;
	greyedStart(CVAR_BORDERS);
	color("Border color", CVAR_BORDER_COLOR);
	greyedEnd;
	checkbox("Enable background", CVAR_BACKGROUND);
	sameline;
	greyedStart(CVAR_BACKGROUND);
	color("Background color", CVAR_BACKGROUND_COLOR);
	greyedEnd;
	slider("Alpha", CVAR_ALPHA, 0, 1);
	blank;

	greyedEnd;

	separator;
	text("Bindings (this creates bindings in the Bindings tab)");
	dropdown("Main GUI", CVAR_KEYBIND_MAIN_GUI, KEYBINDS);
	dropdown("Quick Note", CVAR_KEYBIND_QUICK_NOTE, KEYBINDS);

	separator;
	text(std::string("Created with love by adamk33n3r (v") + PluginVersion + ")");

	setFile.close();

	cvarManager->executeCommand("cl_settings_refreshplugins");
}
