#pragma once
#include <bakkesmod\wrappers\canvaswrapper.h>
#include <vector>
#include <string>
#include <optional>

// Macro from wingdi.h
#undef GetCharWidth

namespace Canvas {
	struct Color {
		char red, green, blue, alpha;
		Color(char red, char green, char blue)
			: red(red), green(green), blue(blue), alpha(255) {}
		Color(char red, char green, char blue, char alpha)
			: red(red), green(green), blue(blue), alpha(alpha) {}
		const static Color White;
		const static Color Black;
		const static Color Red;
		const static Color Green;
		const static Color Blue;
	};

	struct Rect {
		int X, Y, Width, Height;
	};

	struct Bounds {
		int Left, Right, Top, Bottom;
	};

	//enum Color : unsigned int {
	//	COLOR_WHITE = MAKE_COLOR(255, 255, 255),
	//	COLOR_BLACK = MAKE_COLOR(0, 0, 0),
	//	COLOR_RED = MAKE_COLOR(255, 0, 0),
	//	COLOR_GREEN = MAKE_COLOR(0, 255, 0),
	//	COLOR_BLUE = MAKE_COLOR(0, 0, 255),
	//};

	enum class Alignment {
		DEFAULT,
		LEFT,
		CENTER,
		RIGHT,
	};

	enum class TableBorder {
		NONE,
		INNER,
		OUTER,
		ALL,
	};

	struct CanvasBorderOptions {
		Color borderColor = Color::White;
		TableBorder borders = TableBorder::NONE;
	};

	struct CanvasTableOptions {
		Color bgColor = Color::Black;
		Color fgColor = Color::White;
		std::optional<int> width;
		CanvasBorderOptions borderOptions;
		Bounds padding{ 1, 1, 1, 1 };
	};

	struct CanvasColumnOptions {
		Alignment alignment = Alignment::RIGHT;
		int minWidth = 10;
		std::optional<int> maxWidth;
		std::optional<int> width;
	};

	struct CanvasCellOptions {
		std::string text;
		std::optional<Color> textColor;
		std::optional<Alignment> alignment;
	};

	struct CanvasTableContext {
		int totalCols = 0;
		//int currentColumn = 0;
		int padding = 5;
		CanvasTableOptions tableOptions;
		std::vector<std::vector<CanvasCellOptions>> rows;
		std::vector<CanvasColumnOptions> columnOptions;
	};

	struct CanvasContext {
		CanvasWrapper canvas;
		bool ctxSet;
		unsigned int characterWidths[256];
		unsigned int characterHeight = 14 /* top padding */ + 2;
		unsigned int scale = 1;
		CanvasTableContext tableContext;
		char alpha = 255;

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
	void SetColor(Color color);
	void SetColor(Color color, char alpha);
	void SetColor(char red, char green, char blue);
	void SetPosition(int x, int y);
	void SetPosition(float x, float y);
	void BeginAlpha(unsigned char alpha);
	void EndAlpha();
	//void BeginScale(float scale);
	//void EndScale();
	void BeginTable(std::initializer_list<CanvasColumnOptions> columns, CanvasTableOptions options = {});
	void Row(const std::initializer_list<CanvasCellOptions>& rowData);
	Rect EndTable();

}

