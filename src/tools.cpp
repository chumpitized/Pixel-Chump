#include "raylib.h"
#include "Tools.h"
#include <vector>
#include "canvas.h"
#include "palette.h"
 
using namespace std;

Tool::Tool() {
	storedColor = GetColor(0xfff4e0ff);
}

bool Tool::inCanvas(Vector2 location) {
	return location.x > canvasXOffset && location.x < canvasXOffset + canvasSize &&
	location.y > canvasYOffset && location.y < canvasYOffset + canvasSize;
}

bool Tool::inPalette(Vector2 location, vector<vector<Color>>& palette) {
	if (location.x < paletteXOffset || location.y < paletteYOffset) return false;

	int x = (location.x - paletteXOffset) / swatchSize;
	int y = (location.y - paletteYOffset) / swatchSize;

	if (palette.size() > y && palette[y].size() > x) return true;
	else return false;
}
