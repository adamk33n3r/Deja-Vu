#include "Canvas.h"

#include <numeric>
#include <assert.h>

#define CHECK_CTX assert(GCanvas->ctxSet)
#define FORWARD(method, ...) CHECK_CTX; return GCanvas->canvas.method(__VA_ARGS__)

static Canvas::CanvasContext     GCanvasDefaultContext;
Canvas::CanvasContext*           GCanvas = &GCanvasDefaultContext;

namespace Canvas {
	const Color Color::White(0xff, 0xff, 0xff);
	const Color Color::Black(0x00, 0x00, 0x00);
	const Color Color::Red(0xff, 0x00, 0x00);
	const Color Color::Green(0x00, 0xff, 0x00);
	const Color Color::Blue(0x00, 0x00, 0xff);
}

void Canvas::SetContext(CanvasWrapper canvas)
{
	GCanvas->canvas = canvas;
	GCanvas->ctxSet = true;
}

int Canvas::GetCharHeight()
{
	return GCanvas->characterHeight;
}

int Canvas::GetCharWidth(unsigned char ch)
{
	if (ch >= 256)
		return 8;
	unsigned int width = GCanvas->characterWidths[ch];
	return width == 0 ? 8 : width;
}

int Canvas::GetStringWidth(std::string str)
{
	unsigned int total = 0;
	for (char ch : str)
	{
		total += GetCharWidth(ch);
	}

	return total;
}

void Canvas::SetColor(Color color)
{
	SetColor(color.red, color.green, color.blue, color.alpha);
}

void Canvas::SetColor(Color color, char alpha)
{
	SetColor(color.red, color.green, color.blue, alpha);
}

#pragma region FORWARDS

void Canvas::SetColor(char red, char green, char blue)
{
	FORWARD(SetColor, red, green, blue, 255);
}

void Canvas::SetColor(char red, char green, char blue, char alpha)
{
	FORWARD(SetColor, red, green, blue, alpha * (GCanvas->alpha / (char)255));
}

void Canvas::SetPosition(Vector2 pos)
{
	FORWARD(SetPosition, pos);
}

void Canvas::SetPosition(Vector2F pos)
{
	FORWARD(SetPosition, pos);
}

Vector2F Canvas::GetPositionFloat()
{
	CHECK_CTX;
	return GCanvas->canvas.GetPositionFloat();
}

Vector2 operator*(const Vector2& v, float mult)
{
	return { (int)(v.X * mult), (int)(v.Y * mult) };
}
Vector2F operator*(const Vector2F& v, float mult)
{
	return { v.X * mult, v.Y * mult };
}

void Canvas::DrawBox(Vector2 size)
{
	FORWARD(DrawBox, size * GCanvas->scale);
}

void Canvas::DrawBox(Vector2F size)
{
	FORWARD(DrawBox, size * GCanvas->scale);
}

void Canvas::FillBox(Vector2 size)
{
	FORWARD(FillBox, size * GCanvas->scale);
}

void Canvas::FillBox(Vector2F size)
{
	FORWARD(FillBox, size * GCanvas->scale);
}

void Canvas::FillTriangle(Vector2 p1, Vector2 p2, Vector2 p3, LinearColor color)
{
	// TODO: Isn't REALLY correct with scaling because it moves the triangle...but idk
	FORWARD(FillTriangle, p1 * GCanvas->scale, p2 * GCanvas->scale, p3 * GCanvas->scale, color);
}

void Canvas::FillTriangle(Vector2F p1, Vector2F p2, Vector2F p3, LinearColor color)
{
	// TODO: Isn't REALLY correct with scaling because it moves the triangle...but idk
	FORWARD(FillTriangle, p1 * GCanvas->scale, p2 * GCanvas->scale, p3 * GCanvas->scale, color);
}

Vector2 Canvas::GetPosition()
{
	FORWARD(GetPosition);
}

void Canvas::DrawString(std::string text)
{
	// Force use scale version because non-scale version in SDK is borked
	FORWARD(DrawString, text, GCanvas->scale, GCanvas->scale);
}

void Canvas::DrawString(std::string text, float xScale, float yScale)
{
	FORWARD(DrawString, text, xScale * GCanvas->scale, yScale * GCanvas->scale);
}

void Canvas::DrawLine(Vector2 start, Vector2 end)
{
	FORWARD(DrawLine, start * GCanvas->scale, end * GCanvas->scale);
}

void Canvas::DrawLine(Vector2 start, Vector2 end, float width)
{
	FORWARD(DrawLine, start * GCanvas->scale, end * GCanvas->scale, width * GCanvas->scale);
}

void Canvas::DrawLine(Vector2F start, Vector2F end)
{
	FORWARD(DrawLine, start * GCanvas->scale, end * GCanvas->scale);
}

void Canvas::DrawLine(Vector2F start, Vector2F end, float width)
{
	FORWARD(DrawLine, start * GCanvas->scale, end * GCanvas->scale, width * GCanvas->scale);
}

void Canvas::DrawRect(Vector2 start, Vector2 end)
{
	FORWARD(DrawRect, start, end * GCanvas->scale);
}

void Canvas::DrawRect(Vector2F start, Vector2F end)
{
	FORWARD(DrawRect, start, end * GCanvas->scale);
}

Vector2F Canvas::ProjectF(Vector location)
{
	FORWARD(ProjectF, location);
}

Vector2 Canvas::Project(Vector location)
{
	FORWARD(Project, location);
}

Vector2 Canvas::GetSize()
{
	FORWARD(GetSize);
}
#pragma endregion FORWARDS

// Helpers

void Canvas::SetPosition(int x, int y)
{
	SetPosition(Vector2{ x, y });
}

void Canvas::SetPosition(float x, float y)
{
	SetPosition(Vector2F{ x, y });
}

void Canvas::BeginAlpha(unsigned char alpha)
{
	assert(alpha >= 0 && alpha <= 255);
	GCanvas->alpha = alpha;
}

void Canvas::EndAlpha()
{
	GCanvas->alpha = 255;
}

//void Canvas::BeginScale(float scale)
//{
//	assert(scale > 0);
//	GCanvas->scale = scale;
//}
//
//void Canvas::EndScale()
//{
//	GCanvas->scale = 1.0f;
//}

void Canvas::BeginTable(std::initializer_list<CanvasColumnOptions> columns, CanvasTableOptions options)
{
	CHECK_CTX;
	assert(columns.size() >= 1);
	GCanvas->tableContext.totalCols = columns.size();
	GCanvas->tableContext.rows.clear();
	GCanvas->tableContext.rows.reserve(columns.size());
	GCanvas->tableContext.columnOptions = columns;
	GCanvas->tableContext.tableOptions = options;
}

void Canvas::Row(const std::initializer_list<CanvasCellOptions>& rowData)
{
	CHECK_CTX;
	assert(GCanvas->tableContext.totalCols >= 1);
	GCanvas->tableContext.rows.push_back(rowData);
}

Canvas::Rect Canvas::EndTable()
{
















	//TODO: Add max width to columns and with that add truncating text with ...
	














	CHECK_CTX;
	Vector2 pos = GetPosition();
	Rect usedArea;
	usedArea.X = pos.X;
	usedArea.Y = pos.Y;

	std::vector<int> colSizes(GCanvas->tableContext.totalCols);
	for (auto row : GCanvas->tableContext.rows)
	{
		int colNum = 0;
		for (auto col : row)
		{
			// Don't worry about extra columns
			if (colNum >= GCanvas->tableContext.totalCols)
				break;
			colSizes[colNum] = max(GetStringWidth(col.text) + GCanvas->tableContext.padding * 2, colSizes[colNum]);
			auto columnOption = GCanvas->tableContext.columnOptions[colNum];
			if (columnOption.maxWidth.has_value())
				colSizes[colNum] = min(columnOption.maxWidth.value(), colSizes[colNum]);
			colNum++;
		}
	}
	int totalWidth = GCanvas->tableContext.tableOptions.width.value_or(std::accumulate(colSizes.begin(), colSizes.end(), 0));
	usedArea.Width = totalWidth;

	int maxX = pos.X + totalWidth;
	int maxY = pos.Y + (GetCharHeight() + 2) * GCanvas->tableContext.rows.size() + GCanvas->tableContext.tableOptions.padding.Bottom;

	// Draw background
	SetColor(GCanvas->tableContext.tableOptions.bgColor);
	DrawRect(Vector2{ pos.X, pos.Y }, { maxX, maxY });
	TableBorder& tableBorder = GCanvas->tableContext.tableOptions.borderOptions.borders;
	if (tableBorder == TableBorder::OUTER || tableBorder == TableBorder::ALL)
	{
		SetColor(GCanvas->tableContext.tableOptions.borderOptions.borderColor);
		// TOP
		DrawRect(Vector2{ pos.X, pos.Y }, { maxX, pos.Y + 1 });
		// BOTTOM
		DrawRect(Vector2{ pos.X, maxY - 1 }, { maxX, maxY });
		// LEFT
		DrawRect(Vector2{ pos.X, pos.Y + 1 }, { pos.X + 1, maxY - 1 });
		// RIGHT
		DrawRect(Vector2{ maxX - 1, pos.Y + 1 }, { maxX, maxY - 1 });
	}

	SetPosition(Vector2{ 500, 500 });

	// Draw data
	int yPos = pos.Y + GCanvas->tableContext.tableOptions.padding.Top;
	int rowAcc = 0;
	for (auto row : GCanvas->tableContext.rows)
	{
		int xPos = pos.X;
		int i = 0;
		for (auto col : row)
		{
			if (i >= GCanvas->tableContext.totalCols)
				break;
			int colSize = colSizes[i];
			auto& options = GCanvas->tableContext.columnOptions[i];
			switch (col.alignment.value_or(options.alignment)) {
				case Alignment::LEFT:
					SetPosition(Vector2{ xPos + GCanvas->tableContext.padding, yPos });
					break;
				case Alignment::CENTER: {
					int strWidth = GetStringWidth(col.text);
					int offset = (colSize - strWidth) / 2;
					SetPosition(Vector2{ xPos + offset, yPos });
					break;
				}
				case Alignment::RIGHT: {
					int strWidth = GetStringWidth(col.text);
					int offset = colSize - GCanvas->tableContext.padding - strWidth;
					SetPosition(Vector2{ xPos + offset, yPos });
					break;
				}
				default:
					SetPosition(Vector2{ xPos + GCanvas->tableContext.padding, yPos });
					break;
			}
			//SetPosition(Vector2{ xPos + GCanvas->tableContext.padding, yPos });
					//int strWidth = GetStringWidth(col);
					//int offset = colSize - GCanvas->tableContext.padding - strWidth;
					//SetPosition(Vector2{ xPos + offset, yPos });
			SetColor(col.textColor.value_or(GCanvas->tableContext.tableOptions.fgColor));
			DrawString(col.text);
			xPos += colSize;

			// TODO: make this drawn once per column from top to bottom
			// Right side border of column
			if (i < GCanvas->tableContext.totalCols - 1 && (tableBorder == TableBorder::INNER || tableBorder == TableBorder::ALL))
			{
				bool isInner = tableBorder == TableBorder::INNER;
				SetColor(GCanvas->tableContext.tableOptions.borderOptions.borderColor);
				DrawLine(
					Vector2{ xPos, yPos - (isInner && rowAcc == 0 ? 1 : 0) },
					{ xPos, yPos + GetCharHeight() + 1 + ((isInner && rowAcc == GCanvas->tableContext.rows.size() - 1) ? 1 : 0) }
				);
			}
			i++;
		}

		yPos += GetCharHeight() + 2;
		// Bottom of row border
		if (rowAcc < GCanvas->tableContext.rows.size() - 1 && (tableBorder == TableBorder::INNER || tableBorder == TableBorder::ALL))
		{
			bool isAll = tableBorder == TableBorder::ALL;
			SetColor(GCanvas->tableContext.tableOptions.borderOptions.borderColor);
			DrawLine(Vector2{ pos.X + (isAll ? 1 : 0), yPos }, { maxX - (isAll ? 1 : 0), yPos });
		}

		rowAcc++;
	}
	usedArea.Height = yPos - usedArea.Y;

	return usedArea;
}

