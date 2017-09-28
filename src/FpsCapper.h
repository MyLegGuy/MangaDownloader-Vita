#ifndef HAPPYFPSCAP
#define HAPPYFPSCAP

#define MILISECONDSPERFRAME 16.667

// The milisecodnds at the start of the frame.
u64 frameStartMiliseconds;
u64 numberOfFrames;
u64 tempHold;

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

#endif