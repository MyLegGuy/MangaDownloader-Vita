#ifndef GENERALGOODEXTENDED_H
#define GENERALGOODEXTENDED_H

#if PLATFORM == PLAT_COMPUTER
	enum SceCtrlPadButtons {
		SCE_CTRL_SELECT      = 0,
		SCE_CTRL_L3          = 1,
		SCE_CTRL_R3          = 2,
		SCE_CTRL_START       = 3,
		SCE_CTRL_UP          = 4,
		SCE_CTRL_RIGHT       = 5,
		SCE_CTRL_DOWN        = 6,
		SCE_CTRL_LEFT        = 7,
		SCE_CTRL_LTRIGGER    = 8,
		SCE_CTRL_RTRIGGER    = 9,
		SCE_CTRL_L1          = 10,
		SCE_CTRL_R1          = 11,
		SCE_CTRL_TRIANGLE    = 12,
		SCE_CTRL_CIRCLE      = 13,
		SCE_CTRL_CROSS       = 14,
		SCE_CTRL_SQUARE      = 15,
		SCE_CTRL_INTERCEPTED = 16,
		SCE_CTRL_VOLUP       = 17,
		SCE_CTRL_VOLDOWN     = 18,
		SCE_TOUCH 			 = 19,
		SCE_ANDROID_BACK	 = 20,
		SCE_MOUSE_SCROLL	 = 21
	};
#endif
#if PLATFORM == PLAT_VITA
	#include <psp2/ctrl.h>
	#include <psp2/kernel/processmgr.h>
	#include <psp2/rtc.h>
	#include <psp2/types.h>
	#include <psp2/display.h>
	#include <psp2/touch.h>
	#include <psp2/io/fcntl.h>
	#include <psp2/io/dirent.h>
	#include <psp2/power.h>
#endif

#if PLATFORM == PLAT_3DS
	#define SCE_CTRL_CROSS KEY_A
	#define SCE_CTRL_CIRCLE KEY_B
	#define SCE_CTRL_UP KEY_UP
	#define SCE_CTRL_DOWN KEY_DOWN
	#define SCE_CTRL_LEFT KEY_LEFT
	#define SCE_CTRL_RIGHT KEY_RIGHT
	#define SCE_CTRL_TRIANGLE KEY_X
	#define SCE_CTRL_SQUARE KEY_Y
	#define SCE_CTRL_START KEY_START
	#define SCE_CTRL_SELECT KEY_SELECT
	#define SCE_CTRL_LTRIGGER KEY_L
	#define SCE_CTRL_RTRIGGER KEY_R
#endif

#if PLATFORM == PLAT_SWITCH
	#include <switch.h>

	#define SCE_CTRL_CROSS KEY_A
	#define SCE_CTRL_CIRCLE KEY_B
	#define SCE_CTRL_TRIANGLE KEY_X
	#define SCE_CTRL_SQUARE KEY_Y
	#define SCE_CTRL_START KEY_PLUS
	#define SCE_CTRL_SELECT KEY_MINUS
	//
	#define SCE_CTRL_LTRIGGER KEY_ZL
	#define SCE_CTRL_RTRIGGER KEY_ZR
	//
	#define SCE_CTRL_UP KEY_UP
	#define SCE_CTRL_DOWN KEY_DOWN
	#define SCE_CTRL_LEFT KEY_LEFT
	#define SCE_CTRL_RIGHT KEY_RIGHT

	#define ctrlDta u64
#else
	#define ctrlDta int
#endif

#if RENDERER == REND_SDL
	#include <SDL2/SDL_keycode.h>
	// Keysyms, lets you use the entire keyboard
	extern SDL_Keycode lastSDLPressedKey;
	extern char lastClickWasRight;
#endif

extern char tempPathFixBuffer[256];
extern char* DATAFOLDER;

extern unsigned char isSkipping;
extern signed char InputValidity;

extern int screenHeight;
extern int screenWidth;

extern int touchX;
extern int touchY;
extern int mouseScroll;

// For FixPath argument
#define TYPE_UNDEFINED 0
#define TYPE_DATA 1
#define TYPE_EMBEDDED 2

void controlsEnd();
void controlsResetEmpty();
void controlsStart();
void fixCoords(int* _x, int* _y);
void fixPath(char* filename,char _buffer[], char type);
void generateDefaultDataDirectory(char** _dataDirPointer, char _useUma0);
#if SUBPLATFORM == SUB_ANDROID
	void itoa(int _num, char* _buffer, int _uselessBase);
#endif
void makeDataDirectory();
ctrlDta isDown(int value);
ctrlDta wasJustPressedRegardless(ctrlDta value);
ctrlDta wasJustPressed(ctrlDta value);
ctrlDta wasJustReleased(ctrlDta value);
char* getFixPathString(char type);
 
#endif /* GENERALGOODEXTENDED_H */