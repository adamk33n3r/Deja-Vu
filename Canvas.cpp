#include "Canvas.h"

#include <numeric>
#include <assert.h>

#define CHECK_CTX assert(GCanvas->ctxSet)
#define FORWARD(method, ...) CHECK_CTX; return GCanvas->canvas.method(__VA_ARGS__)

static Canvas::CanvasContext     GCanvasDefaultContext;
Canvas::CanvasContext*           GCanvas = &GCanvasDefaultContext;

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
	SetColor(color, 255);
}

void Canvas::SetColor(Color color, char alpha)
{
	char red = (color & 0xFF000000) >> 24;
	char green = (color & 0x00FF0000) >> 16;
	char blue = (color & 0x0000FF00) >> 8;
	SetColor(red, green, blue, alpha);
}

void Canvas::SetScale(unsigned int scale)
{
	assert(scale >= 1);
	GCanvas->scale = scale;
}

#pragma region FORWARDS

void Canvas::SetColor(char red, char green, char blue, char alpha)
{
	FORWARD(SetColor, red, green, blue, alpha);
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

void Canvas::DrawBox(Vector2 size)
{
	FORWARD(DrawBox, size);
}

void Canvas::DrawBox(Vector2F size)
{
	FORWARD(DrawBox, size);
}

void Canvas::FillBox(Vector2 size)
{
	FORWARD(FillBox, size);
}

void Canvas::FillBox(Vector2F size)
{
	FORWARD(FillBox, size);
}

void Canvas::FillTriangle(Vector2 p1, Vector2 p2, Vector2 p3, LinearColor color)
{
	FORWARD(FillTriangle, p1, p2, p3, color);
}

void Canvas::FillTriangle(Vector2F p1, Vector2F p2, Vector2F p3, LinearColor color)
{
	FORWARD(FillTriangle, p1, p2, p3, color);
}

Vector2 Canvas::GetPosition()
{
	FORWARD(GetPosition);
}

void Canvas::DrawString(std::string text)
{
	FORWARD(DrawString, text);
}

void Canvas::DrawString(std::string text, float xScale, float yScale)
{
	FORWARD(DrawString, text, xScale, yScale);
}

void Canvas::DrawLine(Vector2 start, Vector2 end)
{
	FORWARD(DrawLine, start, end);
}

void Canvas::DrawLine(Vector2 start, Vector2 end, float width)
{
	FORWARD(DrawLine, start, end, width);
}

void Canvas::DrawLine(Vector2F start, Vector2F end)
{
	FORWARD(DrawLine, start, end);
}

void Canvas::DrawLine(Vector2F start, Vector2F end, float width)
{
	FORWARD(DrawLine, start, end, width);
}

void Canvas::DrawRect(Vector2 start, Vector2 end)
{
	FORWARD(DrawRect, start, end);
}

void Canvas::DrawRect(Vector2F start, Vector2F end)
{
	FORWARD(DrawRect, start, end);
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

void Canvas::BeginTable(std::initializer_list<CanvasTableOptions> columns)
{
	CHECK_CTX;
	assert(columns.size() >= 1);
	GCanvas->tableContext.totalCols = columns.size();
	GCanvas->tableContext.rows.clear();
	GCanvas->tableContext.rows.reserve(columns.size());
	GCanvas->tableContext.columnOptions = columns;
}

void Canvas::Row(const std::vector<std::string>& rowData)
{
	CHECK_CTX;
	assert(GCanvas->tableContext.totalCols >= 1);
	GCanvas->tableContext.rows.push_back(rowData);
}

void Canvas::EndTable()
{
	Vector2 pos{ 50, 50 };
	CHECK_CTX;
	int colSize = 50;

	std::vector<int> colSizes(GCanvas->tableContext.totalCols);
	for (auto row : GCanvas->tableContext.rows)
	{
		int colNum = 0;
		for (auto col : row)
		{
			colSizes[colNum] = max(GetStringWidth(col) + GCanvas->tableContext.padding * 2, colSizes[colNum]);
			colNum++;
		}
	}
	int totalWidth = std::accumulate(colSizes.begin(), colSizes.end(), 0);

	int maxX = pos.X + totalWidth;
	int maxY = pos.Y + (GetCharHeight() + 2) * GCanvas->tableContext.rows.size();

	// Draw background
	SetColor(COLOR_BLACK);
	DrawRect(Vector2{ pos.X, pos.Y }, { maxX, maxY });
	SetColor(COLOR_GREEN);
	DrawLine(Vector2{ pos.X, pos.Y }, { pos.X, maxY });
	DrawLine(Vector2{ pos.X-1, pos.Y }, { maxX, pos.Y });
	//DrawLine(Vector2{ maxX, pos.Y }, { maxX, maxY });
	//DrawLine(Vector2{ pos.X, maxY }, { maxX, maxY });

	// Draw data
	int yPos = pos.Y;
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
			switch (options.alignment) {
				case Alignment::LEFT:
					SetPosition(Vector2{ xPos + GCanvas->tableContext.padding, yPos });
					break;
				case Alignment::CENTER: {
					int strWidth = GetStringWidth(col);
					int offset = (colSize - strWidth) / 2;
					SetPosition(Vector2{ xPos + offset, yPos });
					break;
				}
				case Alignment::RIGHT: {
					int strWidth = GetStringWidth(col);
					int offset = colSize - GCanvas->tableContext.padding - strWidth;
					SetPosition(Vector2{ xPos + offset, yPos });
					break;
				}
			}
			DrawString(col);
			xPos += colSize;
			// TODO: make this drawn once per column from top to bottom
			DrawLine(Vector2{ xPos, yPos }, { xPos, maxY });
			i++;
		}

		yPos += GetCharHeight() + 2;
		DrawLine(Vector2{ pos.X, yPos }, { maxX, yPos });
	}
}

