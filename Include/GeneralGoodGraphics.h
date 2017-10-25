#ifndef GENERALGOODGRAPHICS_H
#define GENERALGOODGRAPHICS_H
 
void DrawRectangle(int x, int y, int w, int h, int r, int g, int b, int a);
void EndDrawing();
void FixCoords(int* _x, int* _y);
void initGraphics(int _windowWidth, int _windowHeight, int* _storeWindowWidth, int* _storeWindowHeight);
void SetClearColor(int r, int g, int b, int a);
void StartDrawing();
 
#endif /* GENERALGOODGRAPHICS_H */