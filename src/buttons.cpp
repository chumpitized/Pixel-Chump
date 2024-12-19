#include "raylib.h"
#include "buttons.h"

Button::Button(const char *name, const char *imagePath) {
	this->name = name;
	Image image = LoadImage(imagePath);
	texture = LoadTextureFromImage(image);
	UnloadImage(image);
}

Button::~Button() {
	UnloadTexture(texture);
}

void Button::Draw(int x, int y) {
	DrawTexture(texture, x, y, LIGHTGRAY);
}

