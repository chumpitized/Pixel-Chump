#include "raylib.h"
#include "canvas.h"

struct Button {
	const char *name;
	Texture texture;

	Button(const char *name, const char *imagePath);
	~Button();
	void Draw(int x, int y);
};
