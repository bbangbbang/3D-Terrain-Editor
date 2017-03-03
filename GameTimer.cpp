#include "GameTimer.h"

GameTimer::GameTimer()
: mPrevTime(0), mCurTime(0), mStopped(false), mStopTime(0)
, mPauseTime(0), mStartTime(0)
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	mSecondsPerCount = 1.0 / countsPerSec;
}


void GameTimer::Reset()
{
	__int64 t;
	QueryPerformanceCounter((LARGE_INTEGER*)&t);

	mPrevTime = t;
	mCurTime = t;

	mStartTime = t;
	mStopTime = 0;
	mPauseTime = 0;
	mStopped = false;
}

void GameTimer::Start()
{
	if (mStopped)
	{
		__int64 t;
		QueryPerformanceCounter((LARGE_INTEGER*)&t);

		mStartTime = t;
		mPauseTime += t - mStopTime;
		mPrevTime = t;
		mCurTime = t;
		mStopped = false;
	}
}

void GameTimer::Stop()
{
	if (!mStopped) {
		mStopped = true;

		__int64 t;
		QueryPerformanceCounter((LARGE_INTEGER*)&t);
		mStopTime = t;
		mDeltaTime = 0;
	}
}

void GameTimer::Tick(){

	if (!mStopped){

		__int64 t;
		QueryPerformanceCounter((LARGE_INTEGER*)&t);

		mCurTime = t;
		mDeltaTime =(mCurTime - mPrevTime) * mSecondsPerCount;
		mPrevTime = mCurTime;

		if (mDeltaTime < 0.0)
		{
			mDeltaTime = 0.0;
		}
	}
}

float GameTimer::DeltaTime()
{
	return mDeltaTime;
}

float GameTimer::TotalTime()
{
	__int64 t;
	if (mStopped)
		t = mStopTime;
	else
		t = mCurTime;

	
	return (t - mStartTime - mPauseTime) * mSecondsPerCount;
}