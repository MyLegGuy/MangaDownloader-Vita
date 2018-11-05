#ifndef HAPPYFPSCAP
#define HAPPYFPSCAP

#include <GeneralGoodConfig.h>
#include <GeneralGood.h>

#define MILISECONDSPERFRAME 16.667

// The milisecodnds at the start of the frame.
u64 frameStartMiliseconds;
u64 numberOfFrames;
u64 tempHold;

#if PLATFORM != PLAT_VITA
	void FpsCapStart(){
		frameStartMiliseconds = getTicks();
	}
	void FpsCapWait(){
		// I just hope I only use this at the end of a frame....
		numberOfFrames=numberOfFrames+1;
		tempHold = getTicks();
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