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

	Changes:
	Tons of code was shifted around or removed.
	File was renamed
	More headers are included.
*/

// Returns malloc'd string if user entered something. NULL if they canceled.
//char* userKeyboardInput(char* _defaultText, char* _title, int _maxInputLength)


#ifndef KEYBOARDCODEHEADER
#define KEYBOARDCODEHEADER
	
	#if PLATFORM == PLAT_VITA

		#include <stdio.h>
		#include <string.h>
		#include <stdlib.h>
		
		#include <psp2/appmgr.h>
		#include <psp2/apputil.h>
		#include <psp2/types.h>
		#include <psp2/kernel/processmgr.h>
		#include <psp2/message_dialog.h>
		#include <psp2/ime_dialog.h>
		#include <psp2/display.h>
		#include <psp2/apputil.h>
		
		#include <vita2d.h>
		
		#define SCE_IME_DIALOG_MAX_TITLE_LENGTH	(128)
		#define SCE_IME_DIALOG_MAX_TEXT_LENGTH	(512)
		
		#define IME_DIALOG_RESULT_NONE 0
		#define IME_DIALOG_RESULT_RUNNING 1
		#define IME_DIALOG_RESULT_FINISHED 2
		#define IME_DIALOG_RESULT_CANCELED 3
		
		void utf16_to_utf8(uint16_t *src, uint8_t *dst) {
			int i;
			for (i = 0; src[i]; i++) {
				if ((src[i] & 0xFF80) == 0) {
					*(dst++) = src[i] & 0xFF;
				} else if((src[i] & 0xF800) == 0) {
					*(dst++) = ((src[i] >> 6) & 0xFF) | 0xC0;
					*(dst++) = (src[i] & 0x3F) | 0x80;
				} else if((src[i] & 0xFC00) == 0xD800 && (src[i + 1] & 0xFC00) == 0xDC00) {
					*(dst++) = (((src[i] + 64) >> 8) & 0x3) | 0xF0;
					*(dst++) = (((src[i] >> 2) + 16) & 0x3F) | 0x80;
					*(dst++) = ((src[i] >> 4) & 0x30) | 0x80 | ((src[i + 1] << 2) & 0xF);
					*(dst++) = (src[i + 1] & 0x3F) | 0x80;
					i += 1;
				} else {
					*(dst++) = ((src[i] >> 12) & 0xF) | 0xE0;
					*(dst++) = ((src[i] >> 6) & 0x3F) | 0x80;
					*(dst++) = (src[i] & 0x3F) | 0x80;
				}
			}
		
			*dst = '\0';
		}
		
		void utf8_to_utf16(uint8_t *src, uint16_t *dst) {
			int i;
			for (i = 0; src[i];) {
				if ((src[i] & 0xE0) == 0xE0) {
					*(dst++) = ((src[i] & 0x0F) << 12) | ((src[i + 1] & 0x3F) << 6) | (src[i + 2] & 0x3F);
					i += 3;
				} else if ((src[i] & 0xC0) == 0xC0) {
					*(dst++) = ((src[i] & 0x1F) << 6) | (src[i + 1] & 0x3F);
					i += 2;
				} else {
					*(dst++) = src[i];
					i += 1;
				}
			}
		
			*dst = '\0';
		}
		char _isKeyboardFirstTime=1;
		char* userKeyboardInput(char* _defaultText, char* _title, int _maxInputLength){
			if (_isKeyboardFirstTime==1){
				sceAppUtilInit(&(SceAppUtilInitParam){}, &(SceAppUtilBootParam){});
				sceCommonDialogSetConfigParam(&(SceCommonDialogConfigParam){});
				_isKeyboardFirstTime=0;
			}
		
			uint16_t ime_title_utf16[SCE_IME_DIALOG_MAX_TITLE_LENGTH];
			uint16_t ime_initial_text_utf16[SCE_IME_DIALOG_MAX_TEXT_LENGTH];
			uint16_t ime_input_text_utf16[SCE_IME_DIALOG_MAX_TEXT_LENGTH + 1];
			uint8_t ime_input_text_utf8[SCE_IME_DIALOG_MAX_TEXT_LENGTH + 1];
		
			// init function
			// This will convert our utf8 input strings to utf16 strings and put them in buffers
			utf8_to_utf16((uint8_t *)_title, ime_title_utf16);
			utf8_to_utf16((uint8_t *)_defaultText, ime_initial_text_utf16);
			
			SceImeDialogParam param;
			sceImeDialogParamInit(&param);
			
			param.sdkVersion = 0x03150021,
			param.supportedLanguages = 0x0001FFFF;
			param.languagesForced = SCE_TRUE;
			param.type = SCE_IME_TYPE_BASIC_LATIN;
			param.title = ime_title_utf16;
			param.maxTextLength = _maxInputLength;
			param.initialText = ime_initial_text_utf16;
			param.inputTextBuffer = ime_input_text_utf16;
			
			sceImeDialogInit(&param);
		
			char* _resultText=NULL;
		
			// Loops, waiting for dialog to finish
			while (1){
				vita2d_start_drawing();
				vita2d_clear_screen();
		
				SceCommonDialogStatus status = sceImeDialogGetStatus();
		
				if (status == IME_DIALOG_RESULT_FINISHED) {
					SceImeDialogResult result;
					memset(&result, 0, sizeof(SceImeDialogResult));
					sceImeDialogGetResult(&result);
		
					if (result.button == SCE_IME_DIALOG_BUTTON_CLOSE) {
						status = IME_DIALOG_RESULT_CANCELED;
						//break;
					} else {
						// Convert result to utf8 and put result in _resultText
						utf16_to_utf8(ime_input_text_utf16, ime_input_text_utf8);
						_resultText = malloc(strlen((char*)ime_input_text_utf8)+1);
						strcpy(_resultText,(char*)ime_input_text_utf8);
					}
		
					sceImeDialogTerm();
					break;
				}
		
				vita2d_end_drawing();
				vita2d_common_dialog_update();
				vita2d_swap_buffers();
				sceDisplayWaitVblankStart();
			}
			return _resultText;
		}

	#elif PLATFORM == PLAT_COMPUTER
		// Should return string. malloc'd
		// Can return NULL, meaning user canceled
		char* userKeyboardInput(char* _startingString, char* _uselessTitle, int _uselessMaxLength){
			printf("Input string:\n");
			char* _tempBuffer=NULL;
			size_t _tempIntBuffer;
			getline(&_tempBuffer,&_tempIntBuffer,stdin);
			removeNewline(_tempBuffer);
			return _tempBuffer;
		}
	#else
		#warning MAKE USER STRING INPUT FUNCTION
		char* userKeyboardInput(char* _startingString, char* _uselessTitle, int _uselessMaxLength){
			return strdup("dummy");
		}
	#endif

#endif