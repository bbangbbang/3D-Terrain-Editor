#pragma once
#include <windows.h>

class GameTimer
{
public:
	GameTimer();
	
	float DeltaTime();
	float TotalTime();

	void Start();
	void Stop();
	void Reset();
	void Tick();

private:

	__int64 mPrevTime;
	__int64 mCurTime;
	__int64 mStopTime;
	__int64 mPauseTime;
	__int64 mStartTime;

	float mSecondsPerCount;
	float mDeltaTime;
	bool mStopped;
	

};