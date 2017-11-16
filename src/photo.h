/*
	VitaShell
	Copyright (C) 2015-2017, TheFloW

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

	=================================================================

	THIS IS A MODIFIED VERSION OF THE ORIGINAL SOURCE CODE!
*/

#ifndef __PHOTO_H__
#define __PHOTO_H__

#if RENDERER == REND_SDL
#if PLATFORM == PLAT_COMPUTER
void sceKernelDelayThread(int _noob){
	FpsCapStart();
	FpsCapWait();
}
#endif

void readPad(){
	controlsStart();
	controlsEnd();
}

// My code
#include "photoExtended.h"
char* photoViewer(CrossTexture* _passedTexture, char* _currentRelativeFilename){
	if (_currentRelativeFilename!=NULL){
		char* _tempSafeSpace = malloc(strlen(_currentRelativeFilename)+1);
		strcpy(_tempSafeSpace,_currentRelativeFilename);
		_currentRelativeFilename = _tempSafeSpace;
	}
	char _isSingleImageMode = _passedTexture!=NULL;
	CrossTexture* tex=NULL;
	//char* _currentRelativeFilename;
	if (_isSingleImageMode==1){
		tex=_passedTexture;
	}else{
		int _initialLoadResult = loadNewPage(&tex,&_currentRelativeFilename,0);
		if (_initialLoadResult==LOADNEW_DIDNTLOAD){
			printf("failed to load.\n");
			return NULL;
		}
	}
	while (1){
		FpsCapStart();
		startDrawing();
		drawTexture(tex,0,0);
		endDrawing();
		controlsStart();
		if (wasJustPressed(SCE_CTRL_RIGHT) || wasJustPressed(SCE_CTRL_LEFT)){
			if (_isSingleImageMode==1){
				freeTexture(tex);
				break;
			}
			int _loadResult = loadNewPage(&tex,&_currentRelativeFilename,wasJustPressed(SCE_CTRL_RIGHT) ? 1 : -1);
			if (_loadResult==LOADNEW_LOADEDNEW){
			}else if (_loadResult==LOADNEW_FINISHEDMANGA){
				break;
			}
		}
		if (wasJustPressed(SCE_CTRL_CIRCLE)){
			break;
		}
		controlsEnd();
		FpsCapWait();
	}
	freeTexture(tex);
	return (_currentRelativeFilename);
}


#elif RENDERER == REND_VITA2D


#define ALIGN_CENTER(a, b) (((a)-(b)) / 2)
#define ALIGN_RIGHT(x, w) ((x)-(w))

#define ANALOG_CENTER 128
#define ANALOG_THRESHOLD 64
#define ANALOG_SENSITIVITY 16

enum {
	SCE_CTRL_RIGHT_ANALOG_UP    = 0x00200000,
	SCE_CTRL_RIGHT_ANALOG_RIGHT = 0x00400000,
	SCE_CTRL_RIGHT_ANALOG_DOWN  = 0x00800000,
	SCE_CTRL_RIGHT_ANALOG_LEFT  = 0x01000000,

	SCE_CTRL_LEFT_ANALOG_UP     = 0x02000000,
	SCE_CTRL_LEFT_ANALOG_RIGHT  = 0x04000000,
	SCE_CTRL_LEFT_ANALOG_DOWN   = 0x08000000,
	SCE_CTRL_LEFT_ANALOG_LEFT   = 0x10000000,
/*
	SCE_CTRL_CROSS              = 0x20000000,
	SCE_CTRL_CIRCLE             = 0x40000000,
*/
};

SceCtrlData pad;
uint32_t old_buttons, current_buttons, pressed_buttons, hold_buttons, hold2_buttons, released_buttons;


#define BIG_BUFFER_SIZE 16 * 1024 * 1024

enum PhotoModes {
	MODE_CUSTOM,
	MODE_PERFECT,
	MODE_ORIGINAL,
	MODE_FIT_HEIGHT,
	MODE_FIT_WIDTH,
};

#define ZOOM_MIN 0.1f
#define ZOOM_MAX 100.0f
#define ZOOM_FACTOR 1.02f

#define MOVE_DIVISION 7.0f

#define ZOOM_TEXT_TIME 2 * 1000 * 1000

//int photoViewer(const char *file, int type, FileList *list, FileListEntry *entry, int *base_pos, int *rel_pos);
void readPad();
// My code
#include "photoExtended.h"

// An image normally viewed is horizontal
static int isHorizontal(float rad) {
	return ((int)sinf(rad) == 0) ? 1 : 0;
}

static void photoMode(float *zoom, float width, float height, float rad, int mode) {
	int horizontal = isHorizontal(rad);
	float h = (horizontal ? height : width);
	float w = (horizontal ? width : height);

	switch (mode) {
		case MODE_CUSTOM:
			break;
			
		case MODE_PERFECT: // this is only used for showing image the first time
			if (h > screenHeight) { // first priority, fit height
				*zoom = screenHeight/h;
			} else if (w > screenWidth) { // second priority, fit screen
				*zoom = screenWidth/w;
			} else { // otherwise, original size
				*zoom = 1.0f;
			}

			break;
		
		case MODE_ORIGINAL:
			*zoom = 1.0f;
			break;
			
		case MODE_FIT_HEIGHT:
			*zoom = screenHeight/h;
			break;
			
		case MODE_FIT_WIDTH:
			*zoom = screenWidth/w;
			break;
	}
}

static int getNextZoomMode(float *zoom, float width, float height, float rad, int mode) {
	float next_zoom = ZOOM_MAX, smallest_zoom = ZOOM_MAX;;
	int next_mode = MODE_ORIGINAL, smallest_mode = MODE_ORIGINAL;

	int i = 0;
	while (i < 3) {
		if (mode == MODE_CUSTOM || mode == MODE_PERFECT || mode == MODE_FIT_WIDTH) {
			mode = MODE_ORIGINAL;
		} else {
			mode++;
		}

		float new_zoom = 0.0f;
		photoMode(&new_zoom, width, height, rad, mode);

		if (new_zoom < smallest_zoom) {
			smallest_zoom = new_zoom;
			smallest_mode = mode;
		}

		if (new_zoom > *zoom && new_zoom < next_zoom) {
			next_zoom = new_zoom;
			next_mode = mode;
		}

		i++;
	}

	// Get smallest then
	if (next_zoom == ZOOM_MAX) {
		next_zoom = smallest_zoom;
		next_mode = smallest_mode;
	}

	*zoom = next_zoom;
	return next_mode;
}



static void resetImageInfo(vita2d_texture *tex, float *width, float *height, float *x, float *y, float *rad, float *zoom, int *mode, uint64_t *time) {
	*width = vita2d_texture_get_width(tex);
	*height = vita2d_texture_get_height(tex);

	*x = *width/2.0f;
	*y = *height/2.0f;

	//*rad = 0;
	*zoom = 1.0f;

	*mode = MODE_PERFECT;
	photoMode(zoom, *width, *height, *rad, *mode);

	*time = 0;
}

void readPad() {
	static int hold_n = 0, hold2_n = 0;

	memset(&pad, 0, sizeof(SceCtrlData));
	sceCtrlPeekBufferPositive(0, &pad, 1);

	if (pad.ly < ANALOG_CENTER-ANALOG_THRESHOLD) {
		pad.buttons |= SCE_CTRL_LEFT_ANALOG_UP;
	} else if (pad.ly > ANALOG_CENTER+ANALOG_THRESHOLD) {
		pad.buttons |= SCE_CTRL_LEFT_ANALOG_DOWN;
	}

	if (pad.lx < ANALOG_CENTER-ANALOG_THRESHOLD) {
		pad.buttons |= SCE_CTRL_LEFT_ANALOG_LEFT;
	} else if (pad.lx > ANALOG_CENTER+ANALOG_THRESHOLD) {
		pad.buttons |= SCE_CTRL_LEFT_ANALOG_RIGHT;
	}

	if (pad.ry < ANALOG_CENTER-ANALOG_THRESHOLD) {
		pad.buttons |= SCE_CTRL_RIGHT_ANALOG_UP;
	} else if (pad.ry > ANALOG_CENTER+ANALOG_THRESHOLD) {
		pad.buttons |= SCE_CTRL_RIGHT_ANALOG_DOWN;
	}

	if (pad.rx < ANALOG_CENTER-ANALOG_THRESHOLD) {
		pad.buttons |= SCE_CTRL_RIGHT_ANALOG_LEFT;
	} else if (pad.rx > ANALOG_CENTER+ANALOG_THRESHOLD) {
		pad.buttons |= SCE_CTRL_RIGHT_ANALOG_RIGHT;
	}

	old_buttons = current_buttons;
	current_buttons = pad.buttons;
	pressed_buttons = current_buttons & ~old_buttons;
	released_buttons = ~current_buttons & old_buttons;

	hold_buttons = pressed_buttons;
	hold2_buttons = pressed_buttons;

	if ((old_buttons & current_buttons) == current_buttons) {
		if (hold_n >= 10) {
			hold_buttons = current_buttons;
			hold_n = 6;
		}

		if (hold2_n >= 10) {
			hold2_buttons = current_buttons;
			hold2_n = 10;
		}

		hold_n++;
		hold2_n++;
	} else {
		hold_n = 0;
		hold2_n = 0;
	}
}
// No need to free a texture afterwards if you pass one to it.
// _currentRelativeFilename isn't touched by this function, so you need to free it yourself if you malloc'd it.
char* photoViewer(CrossTexture* _singleTexture, char* _currentRelativeFilename) {
	WriteToDebugFile("photoviewer used");
	if (_currentRelativeFilename!=NULL){
		// Don't want to touch a buffer I don't know about.
		char* _tempSafeSpace = malloc(strlen(_currentRelativeFilename)+1);
		strcpy(_tempSafeSpace,_currentRelativeFilename);
		_currentRelativeFilename = _tempSafeSpace;
	}
	int _halfScreenWidth = screenWidth/2;
	int _halfScreenHeight = screenHeight/2;
	char _isSingleImageMode = _singleTexture!=NULL;
	vita2d_texture* tex=NULL;
	if (_isSingleImageMode==1){
		tex=_singleTexture;
	}else{
		int _initialLoadResult = loadNewPage(&tex,&_currentRelativeFilename,0);
		if (_initialLoadResult==LOADNEW_DIDNTLOAD){
			return NULL;
		}
	}
	if (!tex) {
		return NULL;
	}
	// Variables
	float width = 0.0f, height = 0.0f, x = 0.0f, y = 0.0f, rad = 0.0f, zoom = 1.0f;
	int mode = MODE_PERFECT;
	uint64_t time = 0;
	// Reset image
	resetImageInfo(tex, &width, &height, &x, &y, &rad, &zoom, &mode, &time);
	while (1) {
		readPad();
		// Cancel
		if (pressed_buttons & SCE_CTRL_CIRCLE) {
			break;
		}
		// Rotate
		if (pressed_buttons & SCE_CTRL_LTRIGGER) {
			rad -= M_PI_2;
			if (rad < 0)
				rad += M_TWOPI;
			photoMode(&zoom, width, height, rad, mode);
		} else if (pressed_buttons & SCE_CTRL_RTRIGGER) {
			rad += M_PI_2;
			if (rad >= M_TWOPI)
				rad -= M_TWOPI;
			photoMode(&zoom, width, height, rad, mode);
		}
		int horizontal = isHorizontal(rad);
		// Previous/next image.
		//if ((horizontal==0 && ((pressed_buttons & SCE_CTRL_LEFT) || (pressed_buttons & SCE_CTRL_RIGHT))) || (horizontal==0 && (pressed_buttons & SCE_CTRL_UP || pressed_buttons & SCE_CTRL_DOWN))) {
		if ((horizontal==1 && ((pressed_buttons & SCE_CTRL_RIGHT) || (pressed_buttons & SCE_CTRL_LEFT))) || (horizontal==0 && ((pressed_buttons & SCE_CTRL_DOWN) || (pressed_buttons & SCE_CTRL_UP)))){
			if (_isSingleImageMode==1){
				break;
			}
			signed char _isNextPage = (((horizontal==1 && (pressed_buttons & SCE_CTRL_RIGHT)) || (horizontal==0 && (pressed_buttons & SCE_CTRL_DOWN))));
			if (_isNextPage==0){
				_isNextPage=-1;
			}
			int _loadResult = loadNewPage(&tex,&_currentRelativeFilename,_isNextPage);
			if (_loadResult==LOADNEW_LOADEDNEW){
				resetImageInfo(tex, &width, &height, &x, &y, &rad, &zoom, &mode, &time);
			}else if (_loadResult==LOADNEW_FINISHEDMANGA){
				break;
			}
		}
		// Photo mode
		if (pressed_buttons & SCE_CTRL_CROSS) {
			x = width / 2.0f;
			y = height / 2.0f;
			// Find next mode
			mode = getNextZoomMode(&zoom, width, height, rad, mode);
		}
		// Zoom
		if ((current_buttons & SCE_CTRL_RIGHT_ANALOG_DOWN) || (horizontal==0 && (current_buttons & SCE_CTRL_LEFT))) {
			mode = MODE_CUSTOM;
			zoom /= ZOOM_FACTOR;
		} else if ((current_buttons & SCE_CTRL_RIGHT_ANALOG_UP) || (horizontal==0 && (current_buttons & SCE_CTRL_RIGHT))) {
			mode = MODE_CUSTOM;
			zoom *= ZOOM_FACTOR;
		}
		if (zoom < ZOOM_MIN) {
			zoom = ZOOM_MIN;
		}
		if (zoom > ZOOM_MAX) {
			zoom = ZOOM_MAX;
		}
		// Move
		if (pad.lx < (ANALOG_CENTER - ANALOG_SENSITIVITY) || pad.lx > (ANALOG_CENTER + ANALOG_SENSITIVITY)) {
			float d = ((pad.lx-ANALOG_CENTER) / MOVE_DIVISION) / zoom;
			if (horizontal) {
				x += cosf(rad) * d;
			} else {
				y += -sinf(rad) * d;
			}
		}
		if (pad.ly < (ANALOG_CENTER - ANALOG_SENSITIVITY) || pad.ly > (ANALOG_CENTER + ANALOG_SENSITIVITY)) {
			float d = ((pad.ly-ANALOG_CENTER) / MOVE_DIVISION) / zoom;
			if (horizontal) {
				y += cosf(rad) * d;
			} else {
				x += sinf(rad) * d;
			}
		}
		// Limit
		float w = horizontal ? _halfScreenWidth : _halfScreenHeight;
		float h = horizontal ? _halfScreenHeight : _halfScreenWidth;
		if ((zoom *  width) > 2.0f * w) {
			if (x < (w / zoom)) {
				x = w / zoom;
			} else if (x > (width - w / zoom)) {
				x = width - w / zoom;
			}
		} else {
			x = width / 2.0f;
		}
		if ((zoom * height) > 2.0f * h) {
			if (y < (h / zoom)) {
				y = h / zoom;
			} else if (y > (height - h / zoom)) {
				y = height - h/zoom;
			}
		} else {
			y = height / 2.0f;
		}
		// Start drawing
		//startDrawing(bg_photo_image);
		startDrawing();
		// Photo
		vita2d_draw_texture_scale_rotate_hotspot(tex, _halfScreenWidth, _halfScreenHeight, zoom, zoom, rad, x, y);
		// Zoom text
		//if ((sceKernelGetProcessTimeWide() - time) < ZOOM_TEXT_TIME)
		//	pgf_draw_textf(SHELL_MARGIN_X, screenHeight - 3.0f * SHELL_MARGIN_Y, PHOTO_ZOOM_COLOR, FONT_SIZE, "%.0f%%", zoom * 100.0f);
		// End drawing
		endDrawing();
	}
	// This is the only way to exit.
	freeTexture(tex);
	return _currentRelativeFilename;
}

#endif

#endif