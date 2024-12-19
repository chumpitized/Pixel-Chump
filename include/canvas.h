#pragma once

#include "raylib.h"
#include <vector>

using namespace std;

struct Pixel {
	Color color;
	Color colorless;
	Color uncommittedColor;
	Vector2 location;
	bool isEmpty;

	Pixel();
	~Pixel();
};

const int windowSizeX = 900;
const int windowSizeY = 800;

const int pixelSize = 21;
const int miniPixelSize = 5;

const int canvasSize = 672;
const int canvasSizeInPixels = canvasSize / pixelSize;
const int canvasXOffset = 28;
const int canvasYOffset = 80;

const int miniCanvasSize = (canvasSize / pixelSize) * miniPixelSize;
const int miniCanvasXOffset = canvasXOffset + canvasSize + 20;

struct Canvas {

public:
	const int size = canvasSize / pixelSize;
	vector<vector<Pixel>> pixels;

	Canvas();
	~Canvas();

	void Draw();
	void flushUncommitted();
	void DrawMini();
	void HorizontalFlip();
	void VerticalFlip();
	void ExportCanvas(const char *filename);
};