#include "pch.h"
#include "Canvas.h"

#undef max
#define CHECK_CTX assert(GCanvas->ctxSet)
#define FORWARD(method, ...) CHECK_CTX; return GCanvas->canvas.method(__VA_ARGS__)

static Canvas::CanvasContext     GCanvasDefaultContext;
Canvas::CanvasContext*           GCanvas = &GCanvasDefaultContext;

const Canvas::Color Canvas::Color::WHITE = { 255, 255, 255 };
const Canvas::Color Canvas::Color::BLACK = {   0,   0,   0 };
const Canvas::Color Canvas::Color::RED   = { 255,   0,   0 };
const Canvas::Color Canvas::Color::GREEN = {   0, 255,   0 };
const Canvas::Color Canvas::Color::BLUE  = {   0,   0, 255 };

void Canvas::SetContext(CanvasWrapper canvas)
{
	GCanvas->canvas = canvas;
	GCanvas->ctxSet = true;
}

bool Canvas::IsContextSet()
{
	return GCanvas->ctxSet;
}

float Canvas::GetCharHeight()
{
	return GCanvas->canvas.GetStringSize("A", GCanvas->scale, GCanvas->scale).Y;
}

float Canvas::GetStringWidth(std::string str)
{
	return GCanvas->canvas.GetStringSize(str, GCanvas->scale, GCanvas->scale).X;
}

char Canvas::GetGlobalAlpha()
{
	return GCanvas->alpha;
}

void Canvas::SetGlobalAlpha(char alpha)
{
	GCanvas->alpha = alpha;
}

void Canvas::SetColor(Color color)
{
	SetColor(color, GCanvas->alpha);
}

void Canvas::SetColor(Color color, char alpha)
{
	SetColor(color.r, color.g, color.b, alpha);
}

void Canvas::SetScale(float scale)
{
	assert(scale >= 0);
	GCanvas->scale = scale;
}

void Canvas::SetPosition(int x, int y)
{
	SetPosition(Vector2{x, y});
}

void Canvas::SetPosition(float x, float y)
{
	SetPosition(Vector2F{x, y});
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

Vector2 Canvas::GetPosition()
{
	FORWARD(GetPosition);
}

Vector2F Canvas::GetPositionFloat()
{
	FORWARD(GetPositionFloat);
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

void Canvas::DrawString(std::string text)
{
	FORWARD(DrawString, text, GCanvas->scale, GCanvas->scale);
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


void Canvas::DrawRect(Vector2 size)
{
	auto pos = GetPosition();
	DrawRect(pos, pos + size);
}

void Canvas::BeginTable(CanvasTableOptions tableOptions)
{
	CHECK_CTX;
	GCanvas->tableContext.tableOptions = tableOptions;
	GCanvas->tableContext.rows.clear();
}

void Canvas::Columns(std::vector<CanvasColumnOptions> columns)
{
	assert(columns.size() >= 1);
	GCanvas->tableContext.totalCols = (int)columns.size();
	GCanvas->tableContext.columnOptions = columns;
}

void Canvas::Row(const std::vector<std::string>& rowData)
{
	CHECK_CTX;
	GCanvas->tableContext.rows.push_back(rowData);
}

void Canvas::EndTable()
{
	CHECK_CTX;
	assert(GCanvas->tableContext.totalCols >= 1);
	assert(GCanvas->tableContext.rows.size() >= 1);
	Vector2F pos = GetPositionFloat();

	std::vector<float> colSizes(GCanvas->tableContext.totalCols, 0);
	// Fixed width set
	if (GCanvas->tableContext.tableOptions.width > 0)
	{
		for (auto& row : GCanvas->tableContext.rows)
		{
			// Fill empty columns
			while (row.size() < GCanvas->tableContext.totalCols)
			{
				row.push_back("");
			}
		}
		int colNum = 0;
		int numFixedCols = 0;
		float totalSetWidths = 0;
		for (auto& colOpt : GCanvas->tableContext.columnOptions)
		{
			if (colOpt.width > 0)
			{
				totalSetWidths += colSizes[colNum] = colOpt.width;
				numFixedCols++;
			}
			colNum++;
		}
		float remainingWidth = std::max(GCanvas->tableContext.tableOptions.width - totalSetWidths, 0.0f);
		float newColWidth = remainingWidth / (GCanvas->tableContext.totalCols - numFixedCols);
		//float colRemainder = remainingWidth % (GCanvas->tableContext.totalCols - numFixedCols);
		for (auto& colSize : colSizes)
		{
			if (colSize == 0)
				colSize = remainingWidth / (GCanvas->tableContext.totalCols - numFixedCols);
		}
		//colSizes[colSizes.size() - 1] += colRemainder;
	}
	// Expand to fit columns
	else
	{
		for (auto& row : GCanvas->tableContext.rows)
		{
			// Fill empty columns
			while (row.size() < GCanvas->tableContext.totalCols)
			{
				row.push_back("");
			}
			int colNum = 0;
			for (const auto& col : row)
			{
				colSizes[colNum] = std::max({ GetStringWidth(col) + GCanvas->tableContext.tableOptions.padding * 2, colSizes[colNum], 5.0f });
				colNum++;
			}
		}

		int colNum = 0;
		for (auto& colOpt : GCanvas->tableContext.columnOptions)
		{
			if (colOpt.width > 0)
				colSizes[colNum] = colOpt.width;
			colNum++;
		}
	}
	float totalWidth = std::accumulate(colSizes.begin(), colSizes.end(), 0.0f);

	float maxX = pos.X + totalWidth;
	float maxY = pos.Y + GetTableRowHeight() * (int)GCanvas->tableContext.rows.size();

	// Draw background
	if (GCanvas->tableContext.tableOptions.background)
	{
		SetColor(GCanvas->tableContext.tableOptions.backgroundColor);
		DrawRect(Vector2F{ pos.X, pos.Y }, { maxX, maxY + (GCanvas->tableContext.tableOptions.borders ? 0.0f : GCanvas->tableContext.verticalPadding * 2) });
	}
	if (GCanvas->tableContext.tableOptions.borders)
	{
		SetColor(GCanvas->tableContext.tableOptions.borderColor);
		// For some reason vertical lines draw x-1 and horizontal lines draw y-1
		DrawLine(Vector2F{ pos.X + 1, pos.Y }, { pos.X + 1, maxY });
		DrawLine(Vector2F{ pos.X, pos.Y + 0 }, { maxX, pos.Y + 0 });
	}

	// Draw data
	float xPos = pos.X;
	float yPos = pos.Y + 1 + (GCanvas->tableContext.tableOptions.borders ? 0 : 5);
	float ellipsisWidth = GetStringWidth("...");
	for (const auto& row : GCanvas->tableContext.rows)
	{
		// Reset back to start
		xPos = pos.X;
		int i = 0;
		for (auto col : row)
		{
			if (i >= GCanvas->tableContext.totalCols)
				break;
			float colSize = colSizes[i];
			auto& options = GCanvas->tableContext.columnOptions[i];
			float strWidth = GetStringWidth(col);
			// truncate if necessary
			float remainingPixels = (float)(colSize - GCanvas->tableContext.tableOptions.padding * 2);
			// not enough room for the string
			if (strWidth >= remainingPixels)
			{
				int characters = 0;
				float pixels = 0;
				// remove space for the ellipsis for calculation
				remainingPixels -= ellipsisWidth;
				for (char ch : col)
				{
					float width = GetStringWidth({ch});
					//width = 8;
					if ((pixels + width) > remainingPixels)
						break;
					pixels += width;
					characters++;
				}
				col = col.substr(0, characters) + "...";
				strWidth = GetStringWidth(col);
			}
			switch (options.alignment) {
				case Alignment::LEFT:
					SetPosition(Vector2F{ xPos + GCanvas->tableContext.tableOptions.padding, yPos });
					break;
				case Alignment::CENTER: {
					float offset = (colSize - strWidth) / 2;
					SetPosition(Vector2F{ xPos + offset, yPos });
					break;
				}
				case Alignment::RIGHT: {
					float offset = colSize - GCanvas->tableContext.tableOptions.padding - strWidth;
					SetPosition(Vector2F{ xPos + offset, yPos });
					break;
				}
			}
			SetColor(GCanvas->tableContext.tableOptions.textColor);
			DrawString(col, GCanvas->scale, GCanvas->scale);
			xPos += colSize;
			// TODO: make this drawn once per column from top to bottom
			if (GCanvas->tableContext.tableOptions.borders)
			{
				SetColor(GCanvas->tableContext.tableOptions.borderColor);
				// We don't x+1 here because we want it to overlap the edge of the bg
				// Looks like first row line start too high
				DrawLine(Vector2F{ xPos, yPos - 1 }, { xPos, yPos - 1 + GetTableRowHeight() - 1 });
			}
			i++;
		}

		yPos += GetTableRowHeight();
		if (GCanvas->tableContext.tableOptions.borders)
		{
			SetColor(GCanvas->tableContext.tableOptions.borderColor);
			// y-1 here because we went up 1 at the start for the text padding
			DrawLine(Vector2F{ pos.X, yPos - 1 }, { maxX, yPos - 1 });
		}
	}

	// Set position to after the table to make next UI line up
	SetPosition(pos.X, yPos - 1 + (GCanvas->tableContext.tableOptions.borders ? 0 : 2));
}

float Canvas::GetTableRowHeight()
{
	return GetCharHeight() + GCanvas->tableContext.verticalPadding;
}

