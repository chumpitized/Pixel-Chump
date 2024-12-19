#include "raylib.h"
#include "raymath.h"
#include "canvas.h"
#include "palette.h"
#include "tools.h"
#include "buttons.h"

#include <vector>
#include <iostream>
#include <queue>
#include <cmath>

using namespace std;

int titleXOffset = canvasXOffset;
int titleYOffset = 20;
int fontSize = 40;

Vector2 mousePosition;

struct Interaction {
	Tool tool;
	Canvas canvas;
	Palette palette;

	//Select Tool
	Vector2 prevMousePos;
	Vector2 selectOrigin;
	vector<Vector2> selectRectStack;
	vector<Pixel> selectStack;
	bool selected = false;
	bool dragging = false;

	//Pen-Line
	Vector2 prevStroke;

	//Line Tool
	Vector2 lineOrigin;
	vector<Vector2> lineStack;
	int xDir[8] = {0, 1, 0, -1, 1, 1, -1, -1};
	int yDir[8] = {1, 0, -1, 0, 1, -1, 1, -1};

	Vector2 rectOrigin;
	vector<Vector2> rectStack;

	int lastPositionInCanvas[2] = {1, 1};
	vector<vector<Pixel>> historyStack;
	vector<Pixel> historyTransaction;

	Color hoverColor;

	Interaction() {

	}

	void Draw() {
		canvas.Draw();
		canvas.DrawMini();
		palette.Draw(tool.storedColor);

		getCanvasCoords();
		std::string formattedCoords = ((string)"(" + to_string(lastPositionInCanvas[0]).c_str() + (string)"," + to_string(lastPositionInCanvas[1]).c_str() + (string)")").c_str();
		DrawText(formattedCoords.c_str(), canvasXOffset, canvasYOffset + canvasSize + 10, 20, GetColor(0xfff4e0ff));

		getHoverColor();
		DrawRectangle(canvasXOffset + canvasSize - pixelSize, canvasYOffset + canvasSize + 10, pixelSize, 20, hoverColor); 
	}

	void flipHoriImage() {
		canvas.HorizontalFlip();
	}

	void flipVertImage() {
		canvas.VerticalFlip();
	}

	void changeToFill() {
		tool.currentTool = Fill;
	}

	void changeToPen() {
		tool.currentTool = Pen;
	}

	void changeToLine() {
		tool.currentTool = Line;
	}

	void changeToRect() {
		tool.currentTool = Rect;
	}

	void changeToRectFill() {
		tool.currentTool = RectFill;
	}

	void changeToSelect() {
		tool.currentTool = Select;
	}


	bool inCanvasPixels(Vector2 location) {
		int x = location.x;
		int y = location.y;

		return x >= 0 && x < canvasSizeInPixels && y >= 0 && y < canvasSizeInPixels;
	}

	void Export() {
		double uniqueTime = GetTime();
		std::string stringTime = (string)"myart" + to_string(uniqueTime).c_str() + (string)".png";
		canvas.ExportCanvas(stringTime.c_str());
	}

	void getCanvasCoords() {
		Vector2 mousePos = GetMousePosition();
		if (tool.inCanvas(mousePos)) {
			int x = (mousePos.x - canvasXOffset) / pixelSize;
			int y = (mousePos.y - canvasYOffset) / pixelSize;
			lastPositionInCanvas[0] = x + 1;
			lastPositionInCanvas[1] = abs(y - (canvasSize / pixelSize));
		}
	}

	void getHoverColor() {
		Vector2 mousePos = GetMousePosition();
		if (tool.inCanvas(mousePos)) {
			int x = (mousePos.x - canvasXOffset) / pixelSize;
			int y = (mousePos.y - canvasYOffset) / pixelSize;
			
			Pixel pixel = canvas.pixels[y][x];

			if (pixel.uncommittedColor.a == 0) hoverColor = pixel.color;
			else hoverColor = pixel.uncommittedColor;
		}

	}

	void Undo() {
		if (IsKeyPressed(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Z) && !historyStack.empty()) {
			vector<Pixel> transaction = historyStack.back();
			historyStack.pop_back();

			for (auto pixel : transaction) {
				int x = pixel.location.x;
				int y = pixel.location.y;

				canvas.pixels[y][x] = pixel;
			}

		}

		else if (IsKeyPressed(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Z) && historyStack.empty()) cout << "history is empty" << endl;
	}

	void handleMouseClick() {
		Vector2 mousePos = GetMousePosition();

		if (tool.inPalette(mousePos, palette.colors) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			int x = (mousePos.x - paletteXOffset) / swatchSize;
			int y = (mousePos.y - paletteYOffset) / swatchSize;
					
			tool.storedColor = palette.colors[y][x];
			return;
		} 
		
		//we do this to prevent from calling some released conditions below (which do not require that the cursor be within the canvas) BAD
		if (tool.inPalette(mousePos, palette.colors)) return;

		switch (tool.currentTool) {

			//If you switch to another tool before finalizing the selection / movement, you will NOT get the intended behavior where the overlying uncommitted color merges
			//with the pixel color. This will cause issues when you attempt to draw over that pixel and can't (because you are drawing under the uncommitted color).
			case Select: {
				
				if (!selected) {
					if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
						if (tool.inCanvas(mousePos)) {
							int x = (mousePos.x - canvasXOffset) / pixelSize;
							int y = (mousePos.y - canvasYOffset) / pixelSize;
				
							selectOrigin = Vector2{(float)x, (float)y};
						}
					}
			
					if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
						if (tool.inCanvas(mousePos)) {
							int x = (mousePos.x - canvasXOffset) / pixelSize;
							int y = (mousePos.y - canvasYOffset) / pixelSize;

							Vector2 destination = Vector2{(float)x, (float)y};

							if (Vector2Equals(selectOrigin, destination)) break;

							canvas.flushUncommitted();

							selectRectStack.clear();
							selectRectStack.push_back(selectOrigin);
							selectRectStack.push_back(destination);

							int selectWidth = selectOrigin.x - destination.x;
							int selectHeight = selectOrigin.y - destination.y;

							for (int i = 1; i <= abs(selectWidth); i++) {
								float x = selectWidth < 0 ? selectOrigin.x + i : selectOrigin.x - i;

								selectRectStack.push_back(Vector2{x, selectOrigin.y});
								if (x != destination.x) selectRectStack.push_back(Vector2{x, destination.y});
							}

							for (int i = 1; i <= abs(selectHeight); i++) {
								float y = selectHeight < 0 ? selectOrigin.y + i : selectOrigin.y - i;

								selectRectStack.push_back(Vector2{selectOrigin.x, y});

								if (y != destination.y) selectRectStack.push_back(Vector2{destination.x, y});
							}

							for (auto point : selectRectStack) {
								int x = point.x;
								int y = point.y;
								Pixel& pixel = canvas.pixels[y][x];
								if (pixel.isEmpty) pixel.uncommittedColor = BLACK;
								else pixel.uncommittedColor = RAYWHITE;
							}
						}
					}

					//This pushes the selected pixels into the selection stack
					if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
						if (tool.inCanvas(mousePos)) {
							int x = (mousePos.x - canvasXOffset) / pixelSize;
							int y = (mousePos.y - canvasYOffset) / pixelSize;
				
							Vector2 destination = Vector2{(float)x, (float)y};

							selectStack.clear();

							int selectWidth = selectOrigin.x - destination.x;
							int selectHeight = selectOrigin.y - destination.y;
				
							for (int i = 0; i <= abs(selectHeight); i++) {
								float y = selectHeight < 0 ? selectOrigin.y + i : selectOrigin.y - i;

								for (int j = 0; j <= abs(selectWidth); j++) {
									float x = selectWidth < 0 ? selectOrigin.x + j : selectOrigin.x - j;

									Pixel pixel = canvas.pixels[y][x];

									if (!pixel.isEmpty) {
										cout << "did this get hit???" << endl;
										selectStack.push_back(canvas.pixels[y][x]);
									}

								}
							}

							for (Pixel& pixel : selectStack) {
								float x = pixel.location.x;
								float y = pixel.location.y;

								if (pixel.isEmpty) continue;
								else {
									Pixel& canvasPixel = canvas.pixels[y][x];
									canvasPixel.uncommittedColor = canvasPixel.color;
									canvasPixel.color = canvasPixel.colorless;
									canvasPixel.isEmpty = true;
								}
							}

							selected = true;
						}
					}
				} else {

					if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
						int x = (mousePos.x - canvasXOffset) / pixelSize;
						int y = (mousePos.y - canvasYOffset) / pixelSize;
				
						Vector2 mousePosInPixels = Vector2{(float)x, (float)y};
						bool inSelection = false;

						for (Pixel pixel : selectStack) {
							if (Vector2Equals(mousePosInPixels, pixel.location)) {
								inSelection = true;
								break;
							}
						}

						if (inSelection) {
							dragging = true;
							prevMousePos = mousePos;
						} else {
							for (Pixel& pixel : selectStack) {
								if (inCanvasPixels(pixel.location)) {

									int x = pixel.location.x;
									int y = pixel.location.y;
									
									//We need to honor the original colorless color of the underlying pixel.
									Color origColorless = canvas.pixels[y][x].colorless;
									pixel.colorless = origColorless;
									canvas.pixels[y][x] = pixel;

								}
							}
							canvas.flushUncommitted();
							selectRectStack.clear();
							selectStack.clear();
							selected = false;
							selectOrigin = Vector2{(float)x, (float)y};
						}

						break;

					}

					if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && dragging) {

						int currX = (mousePos.x) / pixelSize;
						int currY = (mousePos.y) / pixelSize;

						int prevX = (prevMousePos.x) / pixelSize;
						int prevY = (prevMousePos.y) / pixelSize;

						Vector2 currMousePosPixels = Vector2{(float)currX, (float)currY};
						Vector2 prevMousePosPixels = Vector2{(float)prevX, (float)prevY};


						if (Vector2Equals(currMousePosPixels, prevMousePosPixels)) return;

						int differenceX = currMousePosPixels.x - prevMousePosPixels.x;
						int differenceY = currMousePosPixels.y - prevMousePosPixels.y;

						canvas.flushUncommitted();

						for (Pixel& pixel : selectStack) {
							pixel.location.x += differenceX;
							pixel.location.y += differenceY;

							float x = pixel.location.x;
							float y = pixel.location.y;

							if (inCanvasPixels(pixel.location)) {						
								canvas.pixels[y][x].uncommittedColor = pixel.color;
							}

						}

						prevMousePos = mousePos;
					}


					if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
						dragging = false;
					}


				}


				break;

			}




			case RectFill: {
				if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
					if (tool.inCanvas(mousePos)) {
						int x = (mousePos.x - canvasXOffset) / pixelSize;
						int y = (mousePos.y - canvasYOffset) / pixelSize;

						rectOrigin = Vector2{(float)x, (float)y};
					}
				}
				
				if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
					if (tool.inCanvas(mousePos)) {
						int x = (mousePos.x - canvasXOffset) / pixelSize;
						int y = (mousePos.y - canvasYOffset) / pixelSize;

						Vector2 destination = Vector2{(float)x, (float)y};

						if (Vector2Equals(rectOrigin, destination)) break;

						canvas.flushUncommitted();

						rectStack.clear();

						int rectWidth = rectOrigin.x - destination.x;
						int rectHeight = rectOrigin.y - destination.y;


						for (int i = 0; i <= abs(rectHeight); i++) {
							float y = rectHeight < 0 ? rectOrigin.y + i : rectOrigin.y - i;

							for (int j = 0; j <= abs(rectWidth); j++) {
								float x = rectWidth < 0 ? rectOrigin.x + j : rectOrigin.x - j;

								rectStack.push_back(Vector2{x, y});
							}
						}

						for (auto point : rectStack) {
							int x = point.x;
							int y = point.y;
							Pixel& pixel = canvas.pixels[y][x];
							pixel.uncommittedColor = tool.storedColor;
						}
					}
				}

				if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
					for (auto point : rectStack) {
						int x = point.x;
						int y = point.y;
						Pixel& pixel = canvas.pixels[y][x];
						pixel.uncommittedColor.a = 0;
						historyTransaction.push_back(pixel);
						pixel.color = tool.storedColor;
						pixel.isEmpty = false;
					}

				}

				break;
			}

			case Rect: {
				if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
					if (tool.inCanvas(mousePos)) {
						int x = (mousePos.x - canvasXOffset) / pixelSize;
						int y = (mousePos.y - canvasYOffset) / pixelSize;

						rectOrigin = Vector2{(float)x, (float)y};
					}
				}
				
				if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
					if (tool.inCanvas(mousePos)) {
						int x = (mousePos.x - canvasXOffset) / pixelSize;
						int y = (mousePos.y - canvasYOffset) / pixelSize;

						Vector2 destination = Vector2{(float)x, (float)y};

						if (Vector2Equals(rectOrigin, destination)) break;

						canvas.flushUncommitted();

						rectStack.clear();
						rectStack.push_back(rectOrigin);
						rectStack.push_back(destination);

						int rectWidth = rectOrigin.x - destination.x;
						int rectHeight = rectOrigin.y - destination.y;

						for (int i = 1; i <= abs(rectWidth); i++) {
							float x = rectWidth < 0 ? rectOrigin.x + i : rectOrigin.x - i;

							rectStack.push_back(Vector2{x, rectOrigin.y});
							if (x != destination.x) rectStack.push_back(Vector2{x, destination.y});
						}

						for (int i = 1; i <= abs(rectHeight); i++) {
							float y = rectHeight < 0 ? rectOrigin.y + i : rectOrigin.y - i;

							rectStack.push_back(Vector2{rectOrigin.x, y});

							if (y != destination.y) rectStack.push_back(Vector2{destination.x, y});
						}

						for (auto point : rectStack) {
							int x = point.x;
							int y = point.y;
							Pixel& pixel = canvas.pixels[y][x];
							pixel.uncommittedColor = tool.storedColor;
						}
					}
				}

				if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
					for (auto point : rectStack) {
						int x = point.x;
						int y = point.y;
						Pixel& pixel = canvas.pixels[y][x];
						pixel.uncommittedColor.a = 0;
						if (!ColorIsEqual(pixel.color, tool.storedColor)) historyTransaction.push_back(pixel);
						pixel.color = tool.storedColor;
						pixel.isEmpty = false;
					}

				}

				break;
			}

			case Line: {
				if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
					if (tool.inCanvas(mousePos)) {
						int x = (mousePos.x - canvasXOffset) / pixelSize;
						int y = (mousePos.y - canvasYOffset) / pixelSize;

						lineOrigin = Vector2{(float)x, (float)y};
					}
				}

				if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
					if (tool.inCanvas(mousePos)) {
						int x = (mousePos.x - canvasXOffset) / pixelSize;
						int y = (mousePos.y - canvasYOffset) / pixelSize;

						Vector2 destination = Vector2{(float)x, (float)y};

						if (Vector2Equals(lineOrigin, destination)) break;

						canvas.flushUncommitted();

						lineStack.clear();
						lineStack.push_back(lineOrigin);

						int lineLength = max(abs(lineOrigin.x - destination.x), abs(lineOrigin.y - destination.y));

						for (int i = 0; i < lineLength; i++) {

							Vector2 prev = lineStack.back();
							bool added = false;
							Vector2 candidate;

							for (int j = 0; j < 8; j++) {
								int newX = prev.x + xDir[j];
								int newY = prev.y + yDir[j];
										
								Vector2 curr = Vector2{(float)newX, (float)newY};

								float prevToDestination = Vector2Distance(prev, destination);

								float candidateToDestination = Vector2Distance(candidate, destination);
								float candidateToOrigin = Vector2Distance(candidate, lineOrigin);
							
								float currToDestination = Vector2Distance(curr, destination);
								float currToOrigin = Vector2Distance(curr, lineOrigin);

								if (currToDestination > prevToDestination) continue;

								if (!added) {
									added = true;
									candidate = curr;
									continue;
								}


								if ((currToOrigin + currToDestination) < (candidateToOrigin + candidateToDestination)) {
										candidate = curr;
								}
							}

							lineStack.push_back(candidate);

						}

						for (auto point : lineStack) {
							int x = point.x;
							int y = point.y;
							canvas.pixels[y][x].uncommittedColor = tool.storedColor;
						}
					}
				}

				if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
					for (auto point : lineStack) {
						int x = point.x;
						int y = point.y;

						Pixel& pixel = canvas.pixels[y][x];

						pixel.uncommittedColor = (Color){0, 0, 0, 0};
						historyTransaction.push_back(pixel);
						pixel.color = tool.storedColor;
						pixel.isEmpty = false;
					}
					
				}

				break;
			}

			case Pen: {
				if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
					if (tool.inCanvas(mousePos)) {
						int x = (mousePos.x - canvasXOffset) / pixelSize;
						int y = (mousePos.y - canvasYOffset) / pixelSize;

						prevStroke = Vector2{(float)x, (float)y};

						Pixel& pixel = canvas.pixels[y][x];
						
						if (!ColorIsEqual(pixel.color, tool.storedColor)) {
							historyTransaction.push_back(pixel);

							pixel.color = tool.storedColor;
							pixel.isEmpty = false;
						}
					}
					break;
				}


				if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
					cout << "pen" << endl;

					bool broken = true;

					if (tool.inCanvas(mousePos)) {
												
						int x = (mousePos.x - canvasXOffset) / pixelSize;
						int y = (mousePos.y - canvasYOffset) / pixelSize;

						Vector2 posInCanvas = Vector2{(float)x, (float)y};
						
						for (int i = 0; i < 8; i++) {
							Vector2 adj = Vector2{posInCanvas.x + xDir[i], posInCanvas.y + yDir[i]};

							if (Vector2Equals(adj, prevStroke)) {
								broken = false;
								break;
							}
						}

						if (broken) {
							lineStack.clear();
							lineStack.push_back(prevStroke);

							int lineLength = max(abs(prevStroke.x - posInCanvas.x), abs(prevStroke.y - posInCanvas.y));

							for (int i = 0; i < lineLength; i++) {
								Vector2 prev = lineStack.back();
								bool added = false;
								Vector2 candidate;

								for (int j = 0; j < 8; j++) {
									int newX = prev.x + xDir[j];
									int newY = prev.y + yDir[j];

									Vector2 curr = Vector2{(float)newX, (float)newY};

									float prevToDestination = Vector2Distance(prev, posInCanvas);

									float candidateToDestination = Vector2Distance(candidate, posInCanvas);
									float candidateToOrigin = Vector2Distance(candidate, prevStroke);
							
									float currToDestination = Vector2Distance(curr, posInCanvas);
									float currToOrigin = Vector2Distance(curr, prevStroke);

									if (currToDestination > prevToDestination) continue;

									if (!added) {
										added = true;
										candidate = curr;
										continue;
									}


									if ((currToOrigin + currToDestination) < (candidateToOrigin + candidateToDestination)) {
										candidate = curr;
									}
								}

								lineStack.push_back(candidate);
							}

							for (auto point : lineStack) {
								int x = point.x;
								int y = point.y;

								Pixel& pixel = canvas.pixels[y][x];

								if (!ColorIsEqual(pixel.color, tool.storedColor)) historyTransaction.push_back(pixel);
								pixel.color = tool.storedColor;
								pixel.isEmpty = false;
							}

							prevStroke = posInCanvas;
							break;
						}

						Pixel& pixel = canvas.pixels[y][x];
						
						if (!ColorIsEqual(pixel.color, tool.storedColor)) {
							historyTransaction.push_back(pixel);

							pixel.color = tool.storedColor;
							pixel.isEmpty = false;
						}

						prevStroke = posInCanvas;
					}
				}
				break;
			}

			case Fill: {
				if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
					if (tool.inCanvas(mousePos)) {
						int x = (mousePos.x - canvasXOffset) / pixelSize;
						int y = (mousePos.y - canvasYOffset) / pixelSize;
						int canvasPixels = canvas.pixels.size();
						
						Pixel& selectedPixel 		= canvas.pixels[y][x];
						bool fillPixelColor 		= !selectedPixel.isEmpty;

						historyTransaction.push_back(selectedPixel);

						Color selectedPixelColor 	= selectedPixel.color;
						canvas.pixels[y][x].color 	= tool.storedColor;
						canvas.pixels[y][x].isEmpty = false;
					
						int xDir[4] = {0, 1, 0, -1};
						int yDir[4] = {1, 0, -1, 0};
					
						queue<pair<int,int>> que;
						que.push(pair{x, y});

						while (!que.empty()) {
							pair<int, int> curr = que.front();
							que.pop();
					
							for (int i = 0; i < 4; i++) {
								int newX = curr.first + xDir[i];
								int newY = curr.second + yDir[i];
					
								if (newX < canvasPixels && newX >= 0 && newY < canvasPixels && newY >= 0) {
					
									if (fillPixelColor) {
										if (ColorIsEqual(canvas.pixels[newY][newX].color, selectedPixelColor) && !ColorIsEqual(selectedPixelColor, tool.storedColor)) {
											que.push(pair{newX, newY});

											Pixel& pixel = canvas.pixels[newY][newX];
											
											historyTransaction.push_back(pixel);

											pixel.color = tool.storedColor;
											pixel.isEmpty = false;
										}
									}
									else {
										if (canvas.pixels[newY][newX].isEmpty) {
											que.push(pair{newX, newY});
											
											Pixel& pixel = canvas.pixels[newY][newX];

											historyTransaction.push_back(pixel);

											pixel.color = tool.storedColor;
											pixel.isEmpty = false;
										}
									}
								}
							}
						}
					}
				}
				break;
			}
		}

		if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {

			if (tool.inCanvas(mousePos)) {
				int x = (mousePos.x - canvasXOffset) / pixelSize;
				int y = (mousePos.y - canvasYOffset) / pixelSize;

				Pixel& pixel = canvas.pixels[y][x];

				if (!pixel.isEmpty) { 

					historyTransaction.push_back(pixel);

					pixel.color = pixel.colorless;
					pixel.isEmpty = true;
					return;
				}
			}
		}

		if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
			if (!historyTransaction.empty()) {
				cout << "committing history transaction" << endl;
				historyStack.push_back(historyTransaction);
				historyTransaction.clear();
			}
		}

		if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
			if (!historyTransaction.empty()) {
				cout << "committing history transaction" << endl;
				historyStack.push_back(historyTransaction);
				historyTransaction.clear();
			}
		}



	}
};

int main(void) {
	
	SetTargetFPS(60);
	InitWindow(windowSizeX, windowSizeY, "Pixel Chump");

	Interaction interaction = Interaction(); 

	while(!WindowShouldClose()) {
		BeginDrawing();

		ClearBackground((Color){23, 23, 23, 255});

		DrawFPS(0, 0);
		DrawText("Pixel Chump", titleXOffset, titleYOffset, fontSize, RAYWHITE);

		interaction.Undo();
		interaction.handleMouseClick();
		interaction.Draw();

		if (IsKeyPressed(KEY_V)) {
			cout << "Changed to Fill Tool!!" << endl;
			interaction.changeToFill();
		} 

		if (IsKeyPressed(KEY_B)) {
			cout << "Changed to Pen!!!!" << endl;
			interaction.changeToPen();

		}

		if (IsKeyPressed(KEY_M)) {
			cout << "Changed to Select!!!" << endl;
			interaction.changeToSelect();
		}


		if (IsKeyPressed(KEY_L)) {
			cout << "Changed to Line!!!" << endl;
			interaction.changeToLine();
		}

		if (IsKeyPressed(KEY_P)) {
			interaction.changeToRect();
			cout << "Changed to Rect!!!" << endl;
		}

		if (IsKeyPressed(KEY_F)) {
			interaction.changeToRectFill();
			cout << "Changed to Rect Fill!!!" << endl;
		}

		if (IsKeyPressed(KEY_S) && IsKeyPressed(KEY_LEFT_CONTROL)) {
			interaction.Export();
		}


		if (IsKeyPressed(KEY_H)) {
			interaction.flipHoriImage();
		}

		if (IsKeyPressed(KEY_J)) {
			interaction.flipVertImage();
		}

		EndDrawing();
	}

	CloseWindow();

	return 0;
}

