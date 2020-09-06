#include <stdlib.h>
#include "alternateKeyboardLayout.h"
void inputStr(struct keyboardState* k, void* str);
void keyboardSubmit(struct keyboardState* k, void* str);
void toggleShift(struct keyboardState* k, void* str);
void toggleCapsLock(struct keyboardState* k, void* str);
void backspacePress(struct keyboardState* k, void* str);
void inputRawByte(struct keyboardState* k, void* str);

struct keyEntry qwertyRow0[] = {
	{"`",inputStr,"`",.7},
	{"1",inputStr,"1",1},
	{"2",inputStr,"2",1},
	{"3",inputStr,"3",1},
	{"4",inputStr,"4",1},
	{"5",inputStr,"5",1},
	{"6",inputStr,"6",1},
	{"7",inputStr,"7",1},
	{"8",inputStr,"8",1},
	{"9",inputStr,"9",1},
	{"0",inputStr,"0",1},
	{"-",inputStr,"-",1},
	{"=",inputStr,"=",1},
	{"<-",backspacePress,NULL,2},
};
struct keyEntry qwertyRow1[] = {
	{"->",inputStr,"\t",1.5},
	{"q",inputStr,"q",1},
	{"w",inputStr,"w",1},
	{"e",inputStr,"e",1},
	{"r",inputStr,"r",1},
	{"t",inputStr,"t",1},
	{"y",inputStr,"y",1},
	{"u",inputStr,"u",1},
	{"i",inputStr,"i",1},
	{"o",inputStr,"o",1},
	{"p",inputStr,"p",1},
	{"[",inputStr,"[",1},
	{"]",inputStr,"]",1},
	{"\\",inputStr,"\\",2},
};
struct keyEntry qwertyRow2[] = {
	{"LK",toggleCapsLock,NULL,1.5},
	{"a",inputStr,"a",1},
	{"s",inputStr,"s",1},
	{"d",inputStr,"d",1},
	{"f",inputStr,"f",1},
	{"g",inputStr,"g",1},
	{"h",inputStr,"h",1},
	{"j",inputStr,"j",1},
	{"k",inputStr,"k",1},
	{"l",inputStr,"l",1},
	{";",inputStr,";",1},
	{"'",inputStr,"'",1},
	{"RET",keyboardSubmit,NULL,2},
};
struct keyEntry qwertyRow3[] = {
	{"/\\",toggleShift,NULL,2},
	{"z",inputStr,"z",1},
	{"x",inputStr,"x",1},
	{"c",inputStr,"c",1},
	{"v",inputStr,"v",1},
	{"b",inputStr,"b",1},
	{"n",inputStr,"n",1},
	{"m",inputStr,"m",1},
	{",",inputStr,",",1},
	{".",inputStr,".",1},
	{"/",inputStr,"/",1},
	{"byte",inputRawByte,NULL,1.5},
};

void setEnLayout(struct layout* l){
	l->numRows=4;
	l->keys = malloc(sizeof(struct keyEntry*)*l->numRows);
	l->keys[0]=qwertyRow0;
	l->keys[1]=qwertyRow1;
	l->keys[2]=qwertyRow2;
	l->keys[3]=qwertyRow3;
	l->rowLengths=malloc(sizeof(int)*l->numRows);
	l->rowLengths[0]=sizeof(qwertyRow0)/sizeof(struct keyEntry);
	l->rowLengths[1]=sizeof(qwertyRow1)/sizeof(struct keyEntry);
	l->rowLengths[2]=sizeof(qwertyRow2)/sizeof(struct keyEntry);
	l->rowLengths[3]=sizeof(qwertyRow3)/sizeof(struct keyEntry);
}
struct keyEntry QWERTYRow0[] = {
	{"~",inputStr,"~",.7},
	{"!",inputStr,"!",1},
	{"@",inputStr,"@",1},
	{"#",inputStr,"#",1},
	{"$",inputStr,"$",1},
	{"%",inputStr,"%",1},
	{"^",inputStr,"^",1},
	{"&",inputStr,"&",1},
	{"*",inputStr,"*",1},
	{"(",inputStr,"(",1},
	{")",inputStr,")",1},
	{"_",inputStr,"_",1},
	{"+",inputStr,"+",1},
	{"<-",backspacePress,NULL,2},
};
struct keyEntry QWERTYRow1[] = {
	{"->",inputStr,"\t",1.5},
	{"Q",inputStr,"Q",1},
	{"W",inputStr,"W",1},
	{"E",inputStr,"E",1},
	{"R",inputStr,"R",1},
	{"T",inputStr,"T",1},
	{"Y",inputStr,"Y",1},
	{"U",inputStr,"U",1},
	{"I",inputStr,"I",1},
	{"O",inputStr,"O",1},
	{"P",inputStr,"P",1},
	{"{",inputStr,"{",1},
	{"}",inputStr,"}",1},
	{"|",inputStr,"|",2},
};
struct keyEntry QWERTYRow2[] = {
	{"LK",toggleCapsLock,NULL,1.5},
	{"A",inputStr,"A",1},
	{"S",inputStr,"S",1},
	{"D",inputStr,"D",1},
	{"F",inputStr,"F",1},
	{"G",inputStr,"G",1},
	{"H",inputStr,"H",1},
	{"J",inputStr,"J",1},
	{"K",inputStr,"K",1},
	{"L",inputStr,"L",1},
	{":",inputStr,":",1},
	{"\"",inputStr,"\"",1},
	{"RET",keyboardSubmit,NULL,2},
};
struct keyEntry QWERTYRow3[] = {
	{"/\\",toggleShift,NULL,2},
	{"Z",inputStr,"Z",1},
	{"X",inputStr,"X",1},
	{"C",inputStr,"C",1},
	{"V",inputStr,"V",1},
	{"B",inputStr,"B",1},
	{"N",inputStr,"N",1},
	{"M",inputStr,"M",1},
	{"<",inputStr,"<",1},
	{">",inputStr,">",1},
	{"?",inputStr,"?",1},
	{"byte",inputRawByte,NULL,1.5},
};

void setEnUpperLayout(struct layout* l){
	l->numRows=4;
	l->keys = malloc(sizeof(struct keyEntry*)*l->numRows);
	l->keys[0]=QWERTYRow0;
	l->keys[1]=QWERTYRow1;
	l->keys[2]=QWERTYRow2;
	l->keys[3]=QWERTYRow3;
	l->rowLengths=malloc(sizeof(int)*l->numRows);
	l->rowLengths[0]=sizeof(QWERTYRow0)/sizeof(struct keyEntry);
	l->rowLengths[1]=sizeof(QWERTYRow1)/sizeof(struct keyEntry);
	l->rowLengths[2]=sizeof(QWERTYRow2)/sizeof(struct keyEntry);
	l->rowLengths[3]=sizeof(QWERTYRow3)/sizeof(struct keyEntry);
}

struct keyEntry numsRow0[] = {
	{"7",inputStr,"7",1},
	{"8",inputStr,"8",1},
	{"9",inputStr,"9",1},
};
struct keyEntry numsRow1[] = {
	{"4",inputStr,"4",1},
	{"5",inputStr,"5",1},
	{"6",inputStr,"6",1},
};
struct keyEntry numsRow2[] = {
	{"1",inputStr,"1",1},
	{"2",inputStr,"2",1},
	{"3",inputStr,"3",1},
};
struct keyEntry numsRow3[] = {
	{"0",inputStr,"0",1},
	{"RET",keyboardSubmit,NULL,2},
};

void setNumsLayout(struct layout* l){
	l->numRows=4;
	l->keys = malloc(sizeof(struct keyEntry*)*l->numRows);
	l->keys[0]=numsRow0;
	l->keys[1]=numsRow1;
	l->keys[2]=numsRow2;
	l->keys[3]=numsRow3;
	l->rowLengths=malloc(sizeof(int)*l->numRows);
	l->rowLengths[0]=sizeof(numsRow0)/sizeof(struct keyEntry);
	l->rowLengths[1]=sizeof(numsRow1)/sizeof(struct keyEntry);
	l->rowLengths[2]=sizeof(numsRow2)/sizeof(struct keyEntry);
	l->rowLengths[3]=sizeof(numsRow3)/sizeof(struct keyEntry);
}

void freeLayout(struct layout* l){
	free(l->keys);
	free(l->rowLengths);
}
