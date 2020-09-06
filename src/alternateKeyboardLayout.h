struct keyboardState;
struct keyEntry{
	char* label;
	void (*onAction)(struct keyboardState* k,void*);
	void* data; // given to onAction
	double ratioWidth;
};
struct layout{
	struct keyEntry** keys;
	int numRows;
	int* rowLengths;
};
void setEnLayout(struct layout* l);
void setEnUpperLayout(struct layout* l);
void freeLayout(struct layout* l);
void setNumsLayout(struct layout* l);
