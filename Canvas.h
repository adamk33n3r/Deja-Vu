#pragma once
#include <bakkesmod\wrappers\canvaswrapper.h>

// Macro from wingdi.h
#undef GetCharWidth

namespace Canvas {
	struct Color {
		unsigned char r = 0;
		unsigned char g = 255;
		unsigned char b = 0;
		unsigned char a = 255;

		const static Color WHITE;
		const static Color BLACK;
		const static Color RED;
		const static Color GREEN;
		const static Color BLUE;

		operator LinearColor()
		{
			return LinearColor{ (float)this->r, (float)this->g, (float)this->b, (float)this->a };
		}
	};

	inline Color to_color(LinearColor lColor)
	{
		return Color{ (unsigned char)lColor.R, (unsigned char)lColor.G, (unsigned char)lColor.B, (unsigned char)lColor.A };
	}

	enum class Alignment {
		LEFT = -1,
		CENTER = 0,
		RIGHT = 1,
	};

	struct CanvasColumnOptions {
		Alignment alignment = Alignment::LEFT;
		float width = 0; // 0 for auto
	};

	struct CanvasTableOptions {
		Color textColor = Color::WHITE;
		bool background = true;
		Color backgroundColor = Color::BLACK;
		bool borders = false;
		Color borderColor = Color::WHITE;
		float width = 0; // 0 for auto
		float padding = 5;
	};

	struct CanvasTableContext {
		int totalCols = 0;
		CanvasTableOptions tableOptions;
		std::vector<CanvasColumnOptions> columnOptions;
		std::vector<std::vector<std::string>> rows;
		float verticalPadding = 3;
	};

	struct CanvasContext {
		CanvasWrapper canvas;
		bool ctxSet;
		float scale = 1.0f;
		char alpha = (char)255;
		CanvasTableContext tableContext;

		CanvasContext() : canvas(NULL) {
			ctxSet = false;
		}
	};

	void SetContext(CanvasWrapper canvas);
	bool IsContextSet();
	float GetCharHeight();
	float GetStringWidth(std::string str);

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
	char GetGlobalAlpha();
	void SetGlobalAlpha(char alpha);
	void SetColor(Color color);
	void SetColor(Color color, char alpha);
	void SetScale(float scale);
	void SetPosition(int x, int y);
	void SetPosition(float x, float y);
	void DrawRect(Vector2 size);
	void BeginTable(CanvasTableOptions tableOptions = {});
	void Columns(std::vector<CanvasColumnOptions> columns);
	void Row(const std::vector<std::string>& rowData);
	void EndTable();
	float GetTableRowHeight();

}

