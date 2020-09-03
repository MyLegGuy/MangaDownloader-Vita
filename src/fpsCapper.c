#ifndef HAPPYFPSCAP
#define HAPPYFPSCAP

#include <goodbrew/config.h>
#include <goodbrew/images.h>
#include <goodbrew/base.h>

#define MILISECONDSPERFRAME 16.667

// The milisecodnds at the start of the frame.
u64 frameStartMiliseconds;
u64 numberOfFrames;
u64 tempHold;

#if GBPLAT != GB_VITA
	void FpsCapStart(){
		frameStartMiliseconds = getMilli();
	}
	void FpsCapWait(){
		// I just hope I only use this at the end of a frame....
		numberOfFrames=numberOfFrames+1;
		tempHold = getMilli();
		// LIMIT FPS
		if (tempHold-frameStartMiliseconds<MILISECONDSPERFRAME){
			wait( MILISECONDSPERFRAME - (tempHold-frameStartMiliseconds));
		}
	}
#else
	void FpsCapStart(){
	}
	void FpsCapWait(){
	}
#endif

#endif
