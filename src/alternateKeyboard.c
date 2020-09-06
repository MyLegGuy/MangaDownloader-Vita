#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <goodbrew/config.h>
#include <goodbrew/graphics.h>
#include <goodbrew/controls.h>
#include <goodbrew/text.h>
#include "alternateKeyboardLayout.h"

extern crossFont* mainFont;
extern int currentTextHeight;

#define KEYCOLOR 32,32,32
#define OUTLINECOLOR 255,255,255
#define PRESSEDKEYCOLOR 0,255,0

struct keyboardState{
	int initialPressedRow; // -1 if none right now
	int initialPressedKey;
	int curPressedRow;
	int curPressedKey;
	char doReleaseAnim;

	char* curStr;
	int curStrSize;
	int curStrUsed;

	char isCapsLock;
	struct layout l;
	char isDone;
};

static void myZeroBuff( void *v, size_t n ){
	#ifdef __GNU_LIBRARY__
	explicit_bzero(v,n);
	#else
	volatile unsigned char *p = v;
	while( n-- ) *p++ = 0;
	#endif
}
extern struct keyEntry QWERTYRow0;
char isCapsRightNow(struct layout* l){
	return (l->keys[0]==&QWERTYRow0);
}
void prepareLayoutSwap(struct keyboardState* k){
	k->curPressedRow=-1;
	k->initialPressedRow=-1;
	freeLayout(&k->l);
}
void backspacePress(struct keyboardState* k, void* str){
	if (k->curStrUsed!=0){
		k->curStr[--k->curStrUsed]='\0';
	}
}
void toggleShift(struct keyboardState* k, void* str){
	char _isUpper=isCapsRightNow(&k->l);
	prepareLayoutSwap(k);
	if (_isUpper){
		setEnLayout(&k->l);
	}else{
		setEnUpperLayout(&k->l);
	}
}
void toggleCapsLock(struct keyboardState* k, void* str){
	toggleShift(k,str);
	k->isCapsLock=isCapsRightNow(&k->l);
}
void keyboardSubmit(struct keyboardState* k, void* str){
	if (k->curStrUsed==0 && !k->curStr){ // in order to allow empty submission...
		k->curStr=malloc(1);
	}
	k->curStr[k->curStrUsed]='\0';
	k->isDone=1;
}
void inputStr(struct keyboardState* k, void* str){
	int _appendLen = strlen(str);
	if (k->curStrUsed+_appendLen>k->curStrSize){
		k->curStrSize+=40;
		char* _newBuff = malloc(k->curStrSize+1);
		memcpy(_newBuff,k->curStr,k->curStrUsed);
		myZeroBuff(k->curStr,k->curStrUsed);
		free(k->curStr);
		k->curStr=_newBuff;
	}
	memcpy(&(k->curStr[k->curStrUsed]),str,_appendLen);
	k->curStrUsed+=_appendLen;
	{ // tmp test
		k->curStr[k->curStrUsed]='\0';
		printf("%s\n",k->curStr);
	}
	if (isCapsRightNow(&k->l) && !k->isCapsLock){
		prepareLayoutSwap(k);
		setEnLayout(&k->l);
	}
}

double getRowTotalRatio(struct layout* l, int r){
	double _totalRatio=0;
	for (int j=0;j<l->rowLengths[r];++j){
		_totalRatio+=l->keys[r][j].ratioWidth;
	}
	return _totalRatio;
}
void getPressedKey(struct layout* l, int w, int h, int touchX, int touchY, int* _retRow, int* _retKey){
	int _rowHeight = h/l->numRows;
	int _curRow=touchY/_rowHeight;
	*_retRow=_curRow;
	double _totalRatio=getRowTotalRatio(l,_curRow);
	int _curX=0;
	for (int j=0;j<l->rowLengths[_curRow];++j){
		int _curW=(l->keys[_curRow][j].ratioWidth/_totalRatio)*w;
		_curX+=_curW;
		if (touchX<_curX){
			*_retKey=j;
			return;
		}
	}
	*_retKey=0;
	return;
}
void gdrawStrInBox(const char* s, int x, int y, int w, int h){
	y+=(h-currentTextHeight)/2;
	gbDrawTextfCenter(mainFont,x,y,255,255,255,255,w,"%s",s);
}
void gdrawRectCol(int x, int y, int w, int h, int borderPx, unsigned char borderR, unsigned char borderG, unsigned char borderB, unsigned char r, unsigned char g, unsigned char b){
	drawRectangle(x,y,w,h,borderR,borderG,borderB,255);
	drawRectangle(x+borderPx,y+borderPx,w-borderPx*2,h-borderPx*2,r,g,b,255);
}
char* showVirtualKeyboard(char _isNumsOnly){
	int w=getScreenWidth();
	int _totalHeight=getScreenHeight();
	struct keyboardState state;
	state.initialPressedRow=-1;
	state.curPressedRow=-1;
	state.curPressedKey=-1;
	state.doReleaseAnim=0;
	state.curStr=NULL;
	state.curStrSize=0;
	state.curStrUsed=0;
	state.isCapsLock=0;
	state.isDone=0;

	if (_isNumsOnly){
		setNumsLayout(&state.l);
	}else{
		setEnLayout(&state.l);
	}
	
	while(!state.isDone){
		int _yReserved=currentTextHeight;
		int _touchH=_totalHeight-_yReserved;
		controlsStart();
		// controls
		if (state.initialPressedRow==-1){
			if (isDown(BUTTON_TOUCH) && touchY>_yReserved){
				getPressedKey(&state.l,w,_touchH,touchX,touchY-_yReserved,&state.initialPressedRow,&state.initialPressedKey);
				state.curPressedRow=state.initialPressedRow;
				state.curPressedKey=state.initialPressedKey;
			}
		}else{
			if (!isDown(BUTTON_TOUCH)){ // we've just released it
				// if the state.curPressedRow showed that we were on the same key that we initially pushed
				if (state.curPressedRow==state.initialPressedRow && state.curPressedKey==state.initialPressedKey){
					// do it
					struct keyEntry* c = &(state.l.keys[state.curPressedRow][state.curPressedKey]);
					c->onAction(&state,c->data);
					state.doReleaseAnim=1;
				}
				state.initialPressedRow=-1;
			}else{
				getPressedKey(&state.l,w,_touchH,touchX,touchY-_yReserved,&state.curPressedRow,&state.curPressedKey);
			}
		}
		if (wasJustPressed(BUTTON_B)){
			myZeroBuff(state.curStr,state.curStrSize);
			free(state.curStr);
			state.curStr=NULL;
			state.curStrSize=0;
			state.isDone=1;
		}
		if (wasJustPressed(BUTTON_L)){
			inputStr(&state," ");
		}
		if (wasJustPressed(BUTTON_A) || wasJustPressed(BUTTON_START)){
			keyboardSubmit(&state, NULL);
		}
		if (wasJustPressed(BUTTON_Y)){
			myZeroBuff(state.curStr,state.curStrSize);
			state.curStrUsed=0;
		}
		controlsEnd();
		startDrawing();
		//
		int _rowHeight = _touchH/state.l.numRows;
		int _borderThick=_rowHeight/50;
		// draw rows bottom up so unused pixels go to the top;
		int _curY = _touchH-_rowHeight+_yReserved;
		for (int i=state.l.numRows-1;i>=0;--i,_curY-=_rowHeight){
			double _totalRatio=getRowTotalRatio(&state.l,i);
			int _curX=0;
			for (int j=0;j<state.l.rowLengths[i];++j){
				int _curW=(state.l.keys[i][j].ratioWidth/_totalRatio)*w;
				//
				unsigned char bgr, bgg, bgb;
				if (state.curPressedRow!=-1 && i==state.curPressedRow && j==state.curPressedKey){
					if (state.initialPressedRow==state.curPressedRow && state.initialPressedKey==state.curPressedKey){
						bgr=255;
						bgg=149;
						bgb=252;
					}else if (state.doReleaseAnim){
						state.doReleaseAnim=0;
						state.curPressedRow=-1;
						bgr=0;
						bgg=255;
						bgb=0;
					}else{
						goto normcolor;
					}
				}else{
				normcolor:
					bgr=32;
					bgg=32;
					bgb=32;
				}
				//
				gdrawRectCol(_curX,_curY,_curW,_rowHeight, _borderThick, OUTLINECOLOR, bgr,bgg,bgb);
				gdrawStrInBox(state.l.keys[i][j].label, _curX, _curY, _curW, _rowHeight);
				_curX+=_curW;
			}
		}
		gbDrawTextf(mainFont,0,0,255,255,255,255,"%d char%c",state.curStrUsed,state.curStrUsed==1 ? ' ' : 's');
		endDrawing();
	}
	freeLayout(&state.l);
	if (state.curStr){
		state.curStr[state.curStrUsed]='\0';
	}
	return state.curStr;
}
void inputRawByte(struct keyboardState* k, void* _nullthingie){
	char* _input = showVirtualKeyboard(1);
	if (_input){
		int _val = atoi(_input);
		if (_val!=0){
			char* _fakeStr = malloc(2);
			_fakeStr[0]=_val;
			_fakeStr[1]='\0';
			inputStr(k, _fakeStr);
			myZeroBuff(_fakeStr,2);
			free(_fakeStr);
		}
		myZeroBuff(_input,strlen(_input));
		free(_input);
	}
}
char* alternateKeyboard(){
	return showVirtualKeyboard(0);
}
