// This file only is public domain.
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
	void removeFromLinkedList(NathanLinkedList** _startingList, int _removeIndex){
		int i;
		NathanLinkedList* _lastList=NULL;
		NathanLinkedList* _tempInterationList = *_startingList;
		for(i=1;;i++){
			if (i==_removeIndex){
				if ((*_startingList)->nextEntry!=NULL){
					if (_lastList!=NULL){
						_lastList->nextEntry = (_tempInterationList)->nextEntry;
						free((_tempInterationList)->memory);
						free((_tempInterationList));
					}else{ // is first entry
						NathanLinkedList* _newFirstEntry = (*_startingList)->nextEntry;
						free((*_startingList)->memory);
						free((*_startingList));
						(*_startingList) = _newFirstEntry;
					}
				}else{
					printf("Can't delete only entry.\n");
				}
				break;
			}
			_lastList=(_tempInterationList);
			if ((_tempInterationList)->nextEntry!=NULL){
				(_tempInterationList) = ((_tempInterationList)->nextEntry);
			}else{
				break;
			}
		}
		return;
	}
	// Returns index if found, returns 0 otherwise
	int searchLinkedList(NathanLinkedList* _startingList, char* _searchTerm){
		int i;
		for(i=1;;i++){
			if (strcmp(_startingList->memory,_searchTerm)==0){
				return i;
			}
			if (_startingList->nextEntry!=NULL){
				_startingList = (_startingList->nextEntry);
			}else{
				break;
			}
		}
		return 0;
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