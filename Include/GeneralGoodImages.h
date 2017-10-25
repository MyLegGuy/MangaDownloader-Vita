#ifndef GENERALGOODIMAGES_H
#define GENERALGOODIMAGES_H

#if RENDERER == REND_SDL
	#define CrossTexture SDL_Texture
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_image.h>	
#endif
#if RENDERER == REND_VITA2D
	#include <vita2d.h>
#endif

void DrawTexturePartScaleRotate(CrossTexture* texture, int x, int y, float tex_x, float tex_y, float tex_w, float tex_h, float x_scale, float y_scale, float rad);
void DrawTexturePartScaleTint(CrossTexture* passedTexture, int destX, int destY, int texX, int texY, int texW, int texH, float texXScale, float texYScale, unsigned char r, unsigned char g, unsigned b);
void DrawTexturePartScale(CrossTexture* passedTexture, int destX, int destY, int texX, int texY, int texW, int texH, float texXScale, float texYScale);
void DrawTextureScaleSize(CrossTexture* passedTexture, int destX, int destY, float texXScale, float texYScale);
void DrawTextureScaleTint(CrossTexture* passedTexture, int destX, int destY, float texXScale, float texYScale, unsigned char r, unsigned char g, unsigned char b);
void DrawTextureScale(CrossTexture* passedTexture, int destX, int destY, float texXScale, float texYScale);
void DrawTexture(CrossTexture* passedTexture, int _destX, int _destY);
void FreeTexture(CrossTexture* passedTexture);
int GetTextureHeight(CrossTexture* passedTexture);
int GetTextureWidth(CrossTexture* passedTexture);
CrossTexture* LoadJPG(char* path);
CrossTexture* LoadPNG(char* path);
CrossTexture* LoadPNGBuffer(void* _passedBuffer, int _passedBufferSize);
CrossTexture* LoadJPGBuffer(void* _passedBuffer, int _passedBufferSize);

#endif /* GENERALGOODGRAPHICS_H */