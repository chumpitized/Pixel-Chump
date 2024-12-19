#include "raylib.h"
#include <vector>
#include "canvas.h"

using namespace std;

const int swatchSize = 32;
const int paletteXOffset = miniCanvasXOffset;
const int paletteYOffset = canvasYOffset + miniCanvasSize + 20;

struct Palette {
	vector<vector<Color>> colors;
	Palette();
	~Palette();

	void Draw(Color selectedColor);
};