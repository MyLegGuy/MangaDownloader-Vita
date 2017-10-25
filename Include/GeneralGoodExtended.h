#ifndef GENERALGOODEXTENDED_H
#define GENERALGOODEXTENDED_H

#if PLATFORM != PLAT_VITA
	enum SceCtrlPadButtons {
		SCE_CTRL_SELECT      = 0,	//!< Select button.
		SCE_CTRL_L3          = 1,	//!< L3 button.
		SCE_CTRL_R3          = 2,	//!< R3 button.
		SCE_CTRL_START       = 3,	//!< Start button.
		SCE_CTRL_UP          = 4,	//!< Up D-Pad button.
		SCE_CTRL_RIGHT       = 5,	//!< Right D-Pad button.
		SCE_CTRL_DOWN        = 6,	//!< Down D-Pad button.
		SCE_CTRL_LEFT        = 7,	//!< Left D-Pad button.
		SCE_CTRL_LTRIGGER    = 8,	//!< Left trigger.
		SCE_CTRL_RTRIGGER    = 9,	//!< Right trigger.
		SCE_CTRL_L1          = 10,	//!< L1 button.
		SCE_CTRL_R1          = 11,	//!< R1 button.
		SCE_CTRL_TRIANGLE    = 12,	//!< Triangle button.
		SCE_CTRL_CIRCLE      = 13,	//!< Circle button.
		SCE_CTRL_CROSS       = 14,	//!< Cross button.
		SCE_CTRL_SQUARE      = 15,	//!< Square button.
		SCE_CTRL_INTERCEPTED = 16,  //!< Input not available because intercepted by another application
		SCE_CTRL_VOLUP       = 17,	//!< Volume up button.
		SCE_CTRL_VOLDOWN     = 18	//!< Volume down button.
		//int SCE_TOUCH = 19;
	};
#endif

extern char tempPathFixBuffer[256];
extern char* DATAFOLDER;

// For FixPath argument
#define TYPE_UNDEFINED 0
#define TYPE_DATA 1
#define TYPE_EMBEDDED 2

void ControlsEnd();
void ControlsResetEmpty();
void ControlsResetFull();
void ControlsReset();
void ControlsStart();
void FixCoords(int* _x, int* _y);
void FixPath(char* filename,char _buffer[], char type);
void GenerateDefaultDataDirectory(char** _dataDirPointer, char _useUma0);
signed char IsDown(int value);
#if SUBPLATFORM == SUB_ANDROID
	void itoa(int _num, char* _buffer, int _uselessBase);
#endif
void MakeDataDirectory();
signed char WasJustPressedRegardless(int value);
signed char WasJustPressed(int value);
signed char WasJustReleased(int value);
 
#endif /* GENERALGOODEXTENDED_H */