#pragma once
#include <bakkesmod\wrappers\canvaswrapper.h>
#include <vector>
#include <string>

// Macro from wingdi.h
#undef GetCharWidth

#define MAKE_COLOR(r, g, b) (r << 24) | (g << 16) | (b << 8)

namespace Canvas {
	struct Color {
		unsigned int r = 0;
		unsigned int g = 255;
		unsigned int b = 0;

		const static Color WHITE;
		const static Color BLACK;
		const static Color RED;
		const static Color GREEN;
		const static Color BLUE;
	};

	enum class Alignment {
		LEFT = -1,
		CENTER = 0,
		RIGHT = 1,
	};

	struct CanvasColumnOptions {
		Alignment alignment = Alignment::LEFT;
		int width = 0; // 0 for auto
	};

	struct CanvasTableOptions {
		Color textColor = Color::WHITE;
		bool background = true;
		Color backgroundColor = Color::BLACK;
		bool borders = false;
		Color borderColor = Color::WHITE;
		int width = 0; // 0 for auto
		int padding = 5;
	};

	struct CanvasTableContext {
		int totalCols = 0;
		//int currentColumn = 0;
		CanvasTableOptions tableOptions;
		std::vector<CanvasColumnOptions> columnOptions;
		std::vector<std::vector<std::string>> rows;
		int verticalPadding = 3;
	};

	struct CanvasContext {
		CanvasWrapper canvas;
		bool ctxSet;
		unsigned int characterWidths[256];
		unsigned int characterHeight = 14 /* top padding */ + 2;
		unsigned int scale = 1;
		char alpha = (char)255;
		CanvasTableContext tableContext;

		CanvasContext() : canvas(NULL) {
			ctxSet = false;

	#pragma region characterWidths
			characterWidths['A'] = 9;
			characterWidths['B'] = 8;
			characterWidths['C'] = 9;
			characterWidths['D'] = 9;
			characterWidths['E'] = 8;
			characterWidths['F'] = 8;
			characterWidths['G'] = 9;
			characterWidths['H'] = 9;
			characterWidths['I'] = 5;
			characterWidths['J'] = 6; // Right next to prev char
			characterWidths['K'] = 8;
			characterWidths['L'] = 7;
			characterWidths['M'] = 11;
			characterWidths['N'] = 9;
			characterWidths['O'] = 10;
			characterWidths['P'] = 8;
			characterWidths['Q'] = 10;
			characterWidths['R'] = 8;
			characterWidths['S'] = 9;
			characterWidths['T'] = 9;
			characterWidths['U'] = 9;
			characterWidths['V'] = 9;
			characterWidths['W'] = 13;
			characterWidths['X'] = 9;
			characterWidths['Y'] = 9;
			characterWidths['Z'] = 9;

			characterWidths['a'] = 8;
			characterWidths['b'] = 8;
			characterWidths['c'] = 8;
			characterWidths['d'] = 8;
			characterWidths['e'] = 8;
			characterWidths['f'] = 6;
			characterWidths['g'] = 8; // Right next to prev char. Weird black pixel top left
			characterWidths['h'] = 8;
			characterWidths['i'] = 3;
			characterWidths['j'] = 4; // Right next to prev char
			characterWidths['k'] = 7;
			characterWidths['l'] = 3;
			characterWidths['m'] = 11;
			characterWidths['n'] = 8;
			characterWidths['o'] = 8;
			characterWidths['p'] = 8;
			characterWidths['q'] = 8;
			characterWidths['r'] = 5;
			characterWidths['s'] = 7;
			characterWidths['t'] = 6;
			characterWidths['u'] = 8;
			characterWidths['v'] = 8;
			characterWidths['w'] = 11;
			characterWidths['x'] = 7;
			characterWidths['y'] = 8;
			characterWidths['z'] = 7;

			characterWidths['`'] = 8;
			characterWidths['1'] = 8;
			characterWidths['2'] = 8;
			characterWidths['3'] = 8;
			characterWidths['4'] = 8;
			characterWidths['5'] = 8;
			characterWidths['6'] = 8;
			characterWidths['7'] = 8;
			characterWidths['8'] = 8;
			characterWidths['9'] = 8;
			characterWidths['0'] = 8;
			characterWidths['-'] = 7;
			characterWidths['='] = 9;

			characterWidths['~'] = 12;
			characterWidths['!'] = 4;
			characterWidths['@'] = 13;
			characterWidths['#'] = 10;
			characterWidths['$'] = 8;
			characterWidths['%'] = 13;
			characterWidths['^'] = 11;
			characterWidths['&'] = 10;
			characterWidths['*'] = 8;
			characterWidths['('] = 6;
			characterWidths[')'] = 6;
			characterWidths['_'] = 8; // Right next to prev char
			characterWidths['+'] = 9;

			characterWidths['['] = 6;
			characterWidths[']'] = 6;
			characterWidths['\\'] = 6; // Right next to prev char
			characterWidths[';'] = 6;
			characterWidths['\''] = 3;
			characterWidths[','] = 5;
			characterWidths['.'] = 5;
			characterWidths['/'] = 6; // Right next to prev char

			characterWidths['{'] = 8;
			characterWidths['}'] = 8;
			characterWidths['|'] = 7;
			characterWidths[':'] = 6;
			characterWidths['"'] = 5;
			characterWidths['<'] = 9;
			characterWidths['>'] = 9;
			characterWidths['?'] = 7;

			characterWidths[' '] = 5;
	#pragma endregion characterWidths
		}
	};

	void SetContext(CanvasWrapper canvas);
	bool IsContextSet();
	int GetCharHeight();
	int GetCharWidth(unsigned char ch);
	int GetStringWidth(std::string str);

	// Forward methods
	void SetColor(char red, char green, char blue, char alpha);
	void SetPosition(Vector2 pos);
	Vector2 GetPosition();
	void DrawBox(Vector2 size);
	void FillBox(Vector2 size);
	void FillTriangle(Vector2 p1, Vector2 p2, Vector2 p3, LinearColor color);
	void DrawLine(Vector2 start, Vector2 end);
	void DrawLine(Vector2 start, Vector2 end, float width);
	void DrawRect(Vector2 start, Vector2 end);
	Vector2 Project(Vector location);
	Vector2 GetSize();

	void SetPosition(Vector2F pos);
	Vector2F GetPositionFloat();
	void DrawBox(Vector2F size);
	void FillBox(Vector2F size);
	void FillTriangle(Vector2F p1, Vector2F p2, Vector2F p3, LinearColor color);
	void DrawString(std::string text);
	void DrawString(std::string text, float xScale, float yScale);
	void DrawLine(Vector2F start, Vector2F end);
	void DrawLine(Vector2F start, Vector2F end, float width);
	void DrawRect(Vector2F start, Vector2F end);
	Vector2F ProjectF(Vector location);

	// Helpers
	void SetGlobalAlpha(char alpha);
	void SetColor(Color color);
	void SetColor(Color color, char alpha);
	void SetScale(unsigned int scale);
	void SetPosition(int x, int y);
	void SetPosition(float x, float y);
	void DrawRect(Vector2 size);
	void BeginTable(CanvasTableOptions tableOptions = {});
	void Columns(std::vector<CanvasColumnOptions> columns);
	void Row(const std::vector<std::string>& rowData);
	void EndTable();
	int GetTableRowHeight();

}

