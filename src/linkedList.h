#ifndef NATHANLINKEDLISTHEADER
#define NATHANLINKEDLISTHEADER

typedef struct NathanLinkedList_t{
	void* memory;
	struct NathanLinkedList_t* nextEntry;
}NathanLinkedList;

#define freeLinkedList(x) freeLinkedListSpecific(x,1)
NathanLinkedList* addToLinkedList(NathanLinkedList* _startingList);
void freeLinkedListSpecific(NathanLinkedList* _startingList, char _shouldFreeMemory);
int getLinkedListLength(NathanLinkedList* _startingList);
NathanLinkedList* getLinkedList(NathanLinkedList* _startingList,int num);
void** linkedListToArray(NathanLinkedList* _passedList);
void removeFromLinkedList(NathanLinkedList** _startingList, int _removeIndex);
int searchLinkedList(NathanLinkedList* _startingList, char* _searchTerm);
#endif