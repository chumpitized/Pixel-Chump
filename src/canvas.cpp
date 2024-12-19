#include "canvas.h"
#include "raylib.h"
#include <iostream>

Pixel::Pixel() {
	uncommittedColor = (Color) {0, 0, 0, 0};
}

Pixel::~Pixel() {}

Canvas::Canvas() {
	pixels = vector(size, vector<Pixel>(size, Pixel()));

	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			Color col = (i + j) % 2 == 0 ? GetColor(0xd9d9d9ff) : GetColor(0xa3a3a3ff);
			Pixel& pixel = pixels[i][j];

			pixel.location.x = j;
			pixel.location.y = i;
			pixel.color = col;
			pixel.colorless = col;
			pixel.isEmpty = true;
		}
	}
}

Canvas::~Canvas() {}

void Canvas::Draw() {
	for (int i = 0; i < size; i++) {
		int ypos = (i * pixelSize) + canvasYOffset;

		for (int j = 0; j < size; j++) {
			Color col = pixels[i][j].color;
			Color uncol = pixels[i][j].uncommittedColor;
			int xpos = (j * pixelSize) + canvasXOffset;
			if (uncol.a == 0) {
				DrawRectangle(xpos, ypos, pixelSize, pixelSize, col);
			} else {
				DrawRectangle(xpos, ypos, pixelSize, pixelSize, uncol);
			}
		}
	}

	int outline = 2;

	DrawRectangleLinesEx(Rectangle{(float)canvasXOffset - outline, (float)canvasYOffset - outline, (float)canvasSize + outline*2, (float)canvasSize + outline*2}, outline, GetColor(0x616161ff)); 
}


void Canvas::flushUncommitted() {
	for (int i = 0; i < pixels.size(); i++) {
		for (int j = 0; j < pixels[0].size(); j++) {
			pixels[i][j].uncommittedColor = (Color){0, 0, 0, 0};
		}

	}
}

void Canvas::DrawMini() {
	for (int i = 0; i < size; i++) {
		int ypos = (i * miniPixelSize) + canvasYOffset;

		for (int j = 0; j < size; j++) {
			Color col = pixels[i][j].color;
			Color uncol = pixels[i][j].uncommittedColor;
			int xpos = (j * miniPixelSize) + miniCanvasXOffset;
			if (uncol.a == 0) {
				DrawRectangle(xpos, ypos, miniPixelSize, miniPixelSize, col);
			} else {
				DrawRectangle(xpos, ypos, miniPixelSize, miniPixelSize, uncol);
			}
		}
	}

	int outline = 2;

	DrawRectangleLinesEx(Rectangle{(float)miniCanvasXOffset - outline, (float)canvasYOffset - outline, (float)miniCanvasSize + outline*2, (float)miniCanvasSize + outline*2}, outline, GetColor(0x616161ff)); 
}

void Canvas::HorizontalFlip() {
	for (int i = 0; i < size; i++) {
		for (int j = 0, k = size - 1; j < k; j++, k--) {
			Pixel temp = pixels[i][j];
			pixels[i][j] = pixels[i][k];
			pixels[i][k] = temp;
		}
	}
}

void Canvas::VerticalFlip() {
	for (int i = 0, j = size - 1; i < j; i++, j--) {
		for (int k = 0; k < size; k++) {
			Pixel temp = pixels[i][k];
			pixels[i][k] = pixels[j][k];
			pixels[j][k] = temp;
		}
	}
}

void Canvas::ExportCanvas(const char *filename) {
	int row = pixels.size();
	int col = pixels[0].size();

	Color *colorData = new Color[row * col];

	for (int i = 0; i < row; i++) {
		for (int j = 0; j < col; j++) {
			colorData[i * col + j] = pixels[i][j].color;
		}
	}

	Image art = Image {
		.data = colorData,
		.width = col,
		.height = row,
		.mipmaps = 1,
		.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
	};

	ExportImage(art, filename);

	UnloadImage(art);
}