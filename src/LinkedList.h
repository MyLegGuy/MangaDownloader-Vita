#ifndef NATHANLINKEDLISTHEADER
#define NATHANLINKEDLISTHEADER
	#include <stdlib.h>
	typedef struct NathanLinkedList_t{
		void* memory;
		struct NathanLinkedList_t* nextEntry;
	}NathanLinkedList;
	// 1 based
	NathanLinkedList* getLinkedList(NathanLinkedList* _startingList,int num){
		NathanLinkedList* listOn = _startingList;
		int i;
		for (i=0;i<num-1;i++){
			listOn=(listOn->nextEntry);
		}
		return listOn;
	}
	// 1 based
	int getLinkedListLength(NathanLinkedList* _startingList){
		int i;
		for (i=1;;i++){
			if (_startingList->nextEntry!=NULL){
				_startingList = (_startingList->nextEntry);
			}else{
				break;
			}
		}
		return i;
	}
	// Adds to the end. Will return what you pass to it if what you passed points to NULL for memory
	NathanLinkedList* addToLinkedList(NathanLinkedList* _startingList){
		if (_startingList->memory==NULL){
			return _startingList;
		}else{
			NathanLinkedList* tempList = calloc(1,sizeof(NathanLinkedList));
			NathanLinkedList* listOn = (getLinkedList(_startingList,getLinkedListLength(_startingList)));
			if (listOn->nextEntry){
				// ??? There's already another entry???
				printf("Problem add to list. Maybe we'll loose some entries.");
			}
			listOn->nextEntry=tempList;
			return tempList;
		}
	}
	#define freeLinkedList(x) freeLinkedListSpecific(x,1)
	void freeLinkedListSpecific(NathanLinkedList* _startingList, char _shouldFreeMemory){
		int i;
		NathanLinkedList* _currentListToFree = _startingList;
		NathanLinkedList* _nextListToFree = _startingList;
		int _cachedListLength = getLinkedListLength(_startingList);
		for (i=0;i<_cachedListLength;i++){
			if (_shouldFreeMemory){
				free(_currentListToFree->memory);
			}
			_nextListToFree = _currentListToFree->nextEntry;
			free(_currentListToFree);
			_currentListToFree = _nextListToFree;
		}
	}
	void** linkedListToArray(NathanLinkedList* _passedList){
		int _passedListLength = getLinkedListLength(_passedList);
		void** _returnArray = calloc(1,sizeof(void*)*_passedListLength);
		int i;
		for (i=0;i<_passedListLength;i++){
			_returnArray[i] = _passedList->memory;
			if (_passedList->nextEntry){
				_passedList = _passedList->nextEntry;
			}
		}
		freeLinkedListSpecific(_passedList,0);
		return _returnArray;
	}
#endif