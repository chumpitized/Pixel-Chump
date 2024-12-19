#include "raylib.h"
#include <vector>

using namespace std;

enum ToolType {
	Pen,
	Fill,
	Line,
	Rect,
	RectFill,
	Select
};

struct Tool {
	Color storedColor;
	enum ToolType currentTool = Pen;

	Tool();
	bool inCanvas(Vector2 location);
	bool inPalette(Vector2 location, vector<vector<Color>>& palette);
};
