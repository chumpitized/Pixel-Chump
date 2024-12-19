#include "raylib.h"
#include "Palette.h"
#include <vector>
#include <iostream>

using namespace std;

Palette::Palette() {
	colors.push_back(vector{
		GetColor(0xfff4e0ff),
		GetColor(0x8fcccbff),
		GetColor(0x449489ff),
		GetColor(0x285763ff),
		GetColor(0x2f2b5cff)
	});
	colors.push_back(vector{
		GetColor(0x4b3b9cff),
		GetColor(0x457cd6ff),
		GetColor(0xf2b63dff),
		GetColor(0xd46e33ff),
		GetColor(0xe34262ff)
	});
	colors.push_back(vector{
		GetColor(0x94353df),
		GetColor(0x57253bff),
		GetColor(0x9c656cff),
		GetColor(0xd1b48cff),
		GetColor(0xb4ba47ff)
	});
	colors.push_back(vector{
		GetColor(0x6d8c32ff),
		GetColor(0x2c1b2eff)
	});
}

Palette::~Palette() {}

void Palette::Draw(Color selectedColor) {
	for (int i = 0; i < colors.size(); i++) {
		int y = (i * swatchSize) + paletteYOffset;

		for (int j = 0; j < colors[i].size(); j++) {
			int x = (j * swatchSize) + paletteXOffset;
			DrawRectangle(x, y, swatchSize, swatchSize, colors[i][j]);

			//Draw active color outline
			if (ColorIsEqual(colors[i][j], selectedColor)) {
				Color paletteColor = colors[i][j];

				DrawRectangleLinesEx(Rectangle{(float)x, (float)y, (float)swatchSize, (float)swatchSize}, 5, (Color){23, 23, 23, 255});
			}
		}
	}
}
