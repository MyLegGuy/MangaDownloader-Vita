#ifndef GENERALGOODTEXT_H
#define GENERALGOODTEXT_H

#if TEXTRENDERER == TEXT_DEBUG
	extern float fontSize;
#endif
#if TEXTRENDERER == TEXT_FONTCACHE
	//int fontSize = 20;
	extern int fontSize;
#endif
#if TEXTRENDERER == TEXT_VITA2D
	extern int fontSize;
#endif
 
void DrawLetterColor(int letterId, int _x, int _y, float size, unsigned char r, unsigned char g, unsigned char b);
void DrawLetter(int letterId, int _x, int _y, float size);
void GoodDrawTextColored(int x, int y, const char* text, float size, unsigned char r, unsigned char g, unsigned char b);
void GoodDrawText(int x, int y, const char* text, float size);
void LoadFont(char* filename);
int TextHeight(float scale);
int TextWidth(float scale, const char* message);

#endif /* GENERALGOODIMAGES_H */