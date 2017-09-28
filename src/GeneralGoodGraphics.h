#ifndef GENERALGOODGRAPHICSHEADER
#define GENERALGOODGRAPHICSHEADER

	#if ISUSINGEXTENDED == 0
		int FixX(int x);
		int FixY(int y);
	#endif

	#if ISUSINGEXTENDED == 0
		#if DOFIXCOORDS == 1
			void FixCoords(int* _x, int* _y){
				*_x = FixX(*_x);
				*_y = FixY(*_y);
			}
			#define EASYFIXCOORDS(x, y) FixCoords(x,y)
		#else
			#define EASYFIXCOORDS(x,y)
		#endif
	#endif

	// Renderer stuff
	#if RENDERER == REND_SDL
		#define CrossTexture SDL_Texture
		#include <SDL2/SDL.h>
		#include <SDL2/SDL_image.h>
		//The window we'll be rendering to
		SDL_Window* mainWindow;
		
		//The window renderer
		SDL_Renderer* mainWindowRenderer;
	#endif
	#if RENDERER == REND_VITA2D
		#include <vita2d.h>
		// CROSS TYPES
		#define CrossTexture vita2d_texture
	#endif

	// _windowWidth and _windowHeight are recommendations for the Window size. Will be ignored on Android, Vita, etc.
	void initGraphics(int _windowWidth, int _windowHeight, int* _storeWindowWidth, int* _storeWindowHeight){
		#if RENDERER == REND_SDL
			SDL_Init(SDL_INIT_VIDEO);
			// If platform is Android, make the window fullscreen and store the screen size in the arguments.
			#if SUBPLATFORM == SUB_ANDROID
				SDL_DisplayMode displayMode;
				if( SDL_GetCurrentDisplayMode( 0, &displayMode ) == 0 ){
					mainWindow = SDL_CreateWindow( "HappyWindo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, displayMode.w, displayMode.h, SDL_WINDOW_SHOWN );
				}else{
					printf("Failed to get display mode....\n");
				}
				*_storeWindowWidth=displayMode.w;
				*_storeWindowHeight=displayMode.h;
			#else
				mainWindow = SDL_CreateWindow( "HappyWindo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, _windowWidth, _windowHeight, SDL_WINDOW_SHOWN );
				*_storeWindowWidth=_windowWidth;
				*_storeWindowHeight=_windowHeight;
			#endif
			if (USEVSYNC){
				mainWindowRenderer = SDL_CreateRenderer( mainWindow, -1, SDL_RENDERER_PRESENTVSYNC);
			}else{
				mainWindowRenderer = SDL_CreateRenderer( mainWindow, -1, SDL_RENDERER_ACCELERATED);
			}
			showErrorIfNull(mainWindowRenderer);
			IMG_Init( IMG_INIT_PNG );
			SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
		#else
			#error Hi, Nathan here. I have to make graphics init function for this renderer. RENDERER
		#endif
	}

	void StartDrawing(){
		#if RENDERER == REND_VITA2D
			vita2d_start_drawing();
			vita2d_clear_screen();
		#elif RENDERER == REND_SDL
			SDL_RenderClear(mainWindowRenderer);
		#elif RENDERER == REND_SF2D
			sf2d_start_frame(GFX_TOP, GFX_LEFT);
		#endif
	}
	
	void EndDrawing(){
		#if RENDERER == REND_VITA2D
			vita2d_end_drawing();
			vita2d_swap_buffers();
			vita2d_wait_rendering_done();
		#elif RENDERER == REND_SDL
			SDL_RenderPresent(mainWindowRenderer);
		#elif RENDERER == REND_SF2D
			sf2d_end_frame();
			sf2d_swapbuffers();
		#endif
	}
	/*
	=================================================
	== IMAGES
	=================================================
	*/
	CrossTexture* LoadPNG(char* path){
		#if RENDERER==REND_VITA2D
			return vita2d_load_PNG_file(path);
		#elif RENDERER==REND_SDL
			// Real one we'll return
			SDL_Texture* _returnTexture;
			// Load temp and sho error
			SDL_Surface* _tempSurface = IMG_Load(path);
			showErrorIfNull(_tempSurface);
			// Make good
			_returnTexture = SDL_CreateTextureFromSurface( mainWindowRenderer, _tempSurface );
			showErrorIfNull(_returnTexture);
			// Free memori
			SDL_FreeSurface(_tempSurface);
			return _returnTexture;
		#elif RENDERER==REND_SF2D
			return sfil_load_PNG_file(path,SF2D_PLACE_RAM);
		#endif
	}

	void FreeTexture(CrossTexture* passedTexture){
		#if RENDERER == REND_VITA2D
			vita2d_wait_rendering_done();
			sceDisplayWaitVblankStart();
			vita2d_free_texture(passedTexture);
			passedTexture=NULL;
		#elif RENDERER == REND_SDL
			SDL_DestroyTexture(passedTexture);
			passedTexture=NULL;
		#elif RENDERER == REND_SF2D
			sf2d_free_texture(passedTexture);
			passedTexture=NULL;
		#endif
	}
	/*
	/////////////////////////////////////////////////////
	////// CROSS PLATFORM DRAW FUNCTIONS
	////////////////////////////////////////////////////
	*/
	void SetClearColor(int r, int g, int b, int a){
		if (a!=255){
			printf("You're a moron\n");
		}
		#if RENDERER == REND_SDL
			SDL_SetRenderDrawColor( mainWindowRenderer, r, g, b, a );
		#elif RENDERER == REND_VITA2D
			vita2d_set_clear_color(RGBA8(r, g, b, a));
		#elif RENDERER == REND_SF2D
			sf2d_set_clear_color(RGBA8(r, g, b, a));
		#endif
	}

	int GetTextureWidth(CrossTexture* passedTexture){
		#if RENDERER == REND_VITA2D
			return vita2d_texture_get_width(passedTexture);
		#elif RENDERER == REND_SDL
			int w, h;
			SDL_QueryTexture(passedTexture, NULL, NULL, &w, &h);
			return w;
		#elif RENDERER == REND_SF2D
			return passedTexture->width;
		#endif
	}
	
	int GetTextureHeight(CrossTexture* passedTexture){
		#if RENDERER == REND_VITA2D
			return vita2d_texture_get_height(passedTexture);
		#elif RENDERER == REND_SDL
			int w, h;
			SDL_QueryTexture(passedTexture, NULL, NULL, &w, &h);
			return h;
		#elif RENDERER == REND_SF2D
			return passedTexture->height;
		#endif
	}

	void DrawRectangle(int x, int y, int w, int h, int r, int g, int b, int a){
		EASYFIXCOORDS(&x,&y);
		#if RENDERER == REND_VITA2D
			vita2d_draw_rectangle(x,y,w,h,RGBA8(r,g,b,a));
		#elif RENDERER == REND_SDL
			unsigned char oldr;
			unsigned char oldg;
			unsigned char oldb;
			unsigned char olda;
			SDL_GetRenderDrawColor(mainWindowRenderer,&oldr,&oldg,&oldb,&olda);
			SDL_SetRenderDrawColor(mainWindowRenderer,r,g,b,a);
			SDL_Rect tempRect;
			tempRect.x=x;
			tempRect.y=y;
			tempRect.w=w;
			tempRect.h=h;
			SDL_RenderFillRect(mainWindowRenderer,&tempRect);
			SDL_SetRenderDrawColor(mainWindowRenderer,oldr,oldg,oldb,olda);
		#elif RENDERER == REND_SF2D
			sf2d_draw_rectangle(x,y,w,h,RGBA8(r,g,b,a));
		#endif
	}
	
	void DrawTexture(CrossTexture* passedTexture, int _destX, int _destY){
		EASYFIXCOORDS(&_destX,&_destY);
		#if RENDERER == REND_VITA2D
			vita2d_draw_texture(passedTexture,_destX,_destY);
		#elif RENDERER == REND_SDL
			SDL_Rect _srcRect;
			SDL_Rect _destRect;
	
			SDL_QueryTexture(passedTexture, NULL, NULL, &(_srcRect.w), &(_srcRect.h));
	
			_destRect.w=_srcRect.w;
			_destRect.h=_srcRect.h;
	
			_destRect.x=_destX;
			_destRect.y=_destY;
			
			_srcRect.x=0;
			_srcRect.y=0;
		
			SDL_RenderCopy(mainWindowRenderer, passedTexture, &_srcRect, &_destRect );
		#elif RENDERER == REND_SF2D
			sf2d_draw_texture(passedTexture,_destX,_destY);
		#endif
	}
	
	////////////////// ALPHA
		void DrawTextureAlpha(CrossTexture* passedTexture, int _destX, int _destY, unsigned char alpha){
			EASYFIXCOORDS(&_destX,&_destY);
			#if RENDERER == REND_VITA2D
				vita2d_draw_texture_tint(passedTexture,_destX,_destY,RGBA8(255,255,255,alpha));
			#elif RENDERER == REND_SDL
				unsigned char oldAlpha;
				SDL_GetTextureAlphaMod(passedTexture, &oldAlpha);
				SDL_SetTextureAlphaMod(passedTexture, alpha);
				SDL_Rect _srcRect;
				SDL_Rect _destRect;
		
				SDL_QueryTexture(passedTexture, NULL, NULL, &(_srcRect.w), &(_srcRect.h));
		
				_destRect.w=_srcRect.w;
				_destRect.h=_srcRect.h;
		
				_destRect.x=_destX;
				_destRect.y=_destY;
				
				_srcRect.x=0;
				_srcRect.y=0;
			
				SDL_RenderCopy(mainWindowRenderer, passedTexture, &_srcRect, &_destRect );
				SDL_SetTextureAlphaMod(passedTexture, oldAlpha);
			#elif RENDERER == REND_SF2D
				sf2d_draw_texture(passedTexture,_destX,_destY);
			#endif
		}
		void DrawTextureScaleAlpha(CrossTexture* passedTexture, int destX, int destY, float texXScale, float texYScale, unsigned char alpha){
			EASYFIXCOORDS(&destX,&destY);
			#if RENDERER == REND_VITA2D
				vita2d_draw_texture_tint_scale(passedTexture,destX,destY,texXScale,texYScale,RGBA8(255,255,255,alpha));
			#elif RENDERER == REND_SDL
				unsigned char oldAlpha;
				SDL_GetTextureAlphaMod(passedTexture, &oldAlpha);
				SDL_SetTextureAlphaMod(passedTexture, alpha);
				SDL_Rect _srcRect;
				SDL_Rect _destRect;
				SDL_QueryTexture(passedTexture, NULL, NULL, &(_srcRect.w), &(_srcRect.h));
				
				_srcRect.x=0;
				_srcRect.y=0;
			
				_destRect.w=(_srcRect.w*texXScale);
				_destRect.h=(_srcRect.h*texYScale);
		
				_destRect.x=destX;
				_destRect.y=destY;
		
				SDL_RenderCopy(mainWindowRenderer, passedTexture, &_srcRect, &_destRect );
				SDL_SetTextureAlphaMod(passedTexture, oldAlpha);
			#elif RENDERER == REND_SF2D
				sf2d_draw_texture_scale(passedTexture,destX,destY,texXScale,texYScale);
			#endif
		}

	void DrawTexturePartScale(CrossTexture* passedTexture, int destX, int destY, int texX, int texY, int texW, int texH, float texXScale, float texYScale){
		EASYFIXCOORDS(&destX,&destY);
		#if RENDERER == REND_VITA2D
			vita2d_draw_texture_part_scale(passedTexture,destX,destY,texX,texY,texW, texH, texXScale, texYScale);
		#elif RENDERER == REND_SDL
			SDL_Rect _srcRect;
			SDL_Rect _destRect;
			_srcRect.w=texW;
			_srcRect.h=texH;
			
			_srcRect.x=texX;
			_srcRect.y=texY;
		
			_destRect.w=_srcRect.w*texXScale;
			_destRect.h=_srcRect.h*texYScale;
			//printf("Dest dimensionds is %dx%d;%.6f;%.6f\n",_destRect.w,_destRect.h,texXScale,texYScale);
			_destRect.x=destX;
			_destRect.y=destY;
	
			SDL_RenderCopy(mainWindowRenderer, passedTexture, &_srcRect, &_destRect );
		#elif RENDERER==REND_SF2D
			sf2d_draw_texture_part_scale(passedTexture,destX,destY,texX,texY,texW, texH, texXScale, texYScale);
		#endif
	}

	void DrawTextureScaleTint(CrossTexture* passedTexture, int destX, int destY, float texXScale, float texYScale, unsigned char r, unsigned char g, unsigned char b){
		EASYFIXCOORDS(&destX,&destY);
		#if RENDERER == REND_VITA2D
			vita2d_draw_texture_tint_scale(passedTexture,destX,destY,texXScale,texYScale,RGBA8(r,g,b,255));
		#elif RENDERER == REND_SDL
			unsigned char oldr;
			unsigned char oldg;
			unsigned char oldb;
			SDL_GetTextureColorMod(passedTexture,&oldr,&oldg,&oldb);
			SDL_SetTextureColorMod(passedTexture, r,g,b);
			SDL_Rect _srcRect;
			SDL_Rect _destRect;
			SDL_QueryTexture(passedTexture, NULL, NULL, &(_srcRect.w), &(_srcRect.h));
			
			_srcRect.x=0;
			_srcRect.y=0;
		
			_destRect.w=(_srcRect.w*texXScale);
			_destRect.h=(_srcRect.h*texYScale);
	
			_destRect.x=destX;
			_destRect.y=destY;
	
			SDL_RenderCopy(mainWindowRenderer, passedTexture, &_srcRect, &_destRect );
			SDL_SetTextureColorMod(passedTexture, oldr, oldg, oldb);
		#elif RENDERER == REND_SF2D
			sf2d_draw_texture_tint_scale(passedTexture,destX,destY,texXScale,texYScale);
		#endif
	}

	void DrawTexturePartScaleTint(CrossTexture* passedTexture, int destX, int destY, int texX, int texY, int texW, int texH, float texXScale, float texYScale, unsigned char r, unsigned char g, unsigned b){
		EASYFIXCOORDS(&destX,&destY);
		#if RENDERER == REND_VITA2D
			vita2d_draw_texture_tint_part_scale(passedTexture,destX,destY,texX,texY,texW, texH, texXScale, texYScale,RGBA8(r,g,b,255));
		#elif RENDERER == REND_SDL
			unsigned char oldr;
			unsigned char oldg;
			unsigned char oldb;
			SDL_GetTextureColorMod(passedTexture,&oldr,&oldg,&oldb);
			SDL_SetTextureColorMod(passedTexture, r,g,b);
			SDL_Rect _srcRect;
			SDL_Rect _destRect;
			_srcRect.w=texW;
			_srcRect.h=texH;
			
			_srcRect.x=texX;
			_srcRect.y=texY;
		
			_destRect.w=_srcRect.w*texXScale;
			_destRect.h=_srcRect.h*texYScale;
	
			_destRect.x=destX;
			_destRect.y=destY;
	
			SDL_RenderCopy(mainWindowRenderer, passedTexture, &_srcRect, &_destRect );
			SDL_SetTextureColorMod(passedTexture, oldr, oldg, oldb);
		#elif RENDERER==REND_SF2D
			sf2d_draw_texture_part_scale(passedTexture,destX,destY,texX,texY,texW, texH, texXScale, texYScale);
		#endif
	}
	
	void DrawTextureScale(CrossTexture* passedTexture, int destX, int destY, float texXScale, float texYScale){
		EASYFIXCOORDS(&destX,&destY);
		#if RENDERER == REND_VITA2D
			vita2d_draw_texture_scale(passedTexture,destX,destY,texXScale,texYScale);
		#elif RENDERER == REND_SDL
			SDL_Rect _srcRect;
			SDL_Rect _destRect;
			SDL_QueryTexture(passedTexture, NULL, NULL, &(_srcRect.w), &(_srcRect.h));
			
			_srcRect.x=0;
			_srcRect.y=0;
		
			_destRect.w=(_srcRect.w*texXScale);
			_destRect.h=(_srcRect.h*texYScale);
	
			_destRect.x=destX;
			_destRect.y=destY;
	
			SDL_RenderCopy(mainWindowRenderer, passedTexture, &_srcRect, &_destRect );
		#elif RENDERER == REND_SF2D
			sf2d_draw_texture_scale(passedTexture,destX,destY,texXScale,texYScale);
		#endif
	}

	void DrawTextureScaleSize(CrossTexture* passedTexture, int destX, int destY, float texXScale, float texYScale){
		EASYFIXCOORDS(&destX,&destY);
		#if RENDERER == REND_VITA2D
			vita2d_draw_texture_scale(passedTexture,destX,destY,texXScale/(double)GetTextureWidth(passedTexture),texYScale/(double)GetTextureHeight(passedTexture));
		#elif RENDERER == REND_SDL
			SDL_Rect _srcRect;
			SDL_Rect _destRect;
			SDL_QueryTexture(passedTexture, NULL, NULL, &(_srcRect.w), &(_srcRect.h));
			
			_srcRect.x=0;
			_srcRect.y=0;
		
			_destRect.w=(texXScale);
			_destRect.h=(texYScale);
	
			_destRect.x=destX;
			_destRect.y=destY;
	
			SDL_RenderCopy(mainWindowRenderer, passedTexture, &_srcRect, &_destRect );
		#elif RENDERER == REND_SF2D
			sf2d_draw_texture_scale(passedTexture,destX,destY,texXScale,texYScale);
		#endif
	}
	
	// TODO MAKE ROTATE ON WINDOWS
	void DrawTexturePartScaleRotate(CrossTexture* texture, int x, int y, float tex_x, float tex_y, float tex_w, float tex_h, float x_scale, float y_scale, float rad){
		EASYFIXCOORDS(&x,&y);
		#if RENDERER == REND_VITA2D
			vita2d_draw_texture_part_scale_rotate(texture,x,y,tex_x,tex_y,tex_w,tex_h,x_scale,y_scale,rad);
		#elif RENDERER == REND_SDL
			DrawTexturePartScale(texture,x,y,tex_x,tex_y,tex_w,tex_h,x_scale,y_scale);
		#elif RENDERER == REND_SF2D
			sf2d_draw_texture_part_rotate_scale(texture,x,y,rad,tex_x,tex_y,tex_w,tex_h,x_scale,y_scale);
		#endif
	}

#endif