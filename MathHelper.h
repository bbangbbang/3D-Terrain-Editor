#pragma once
#include <math.h>

class MathHelper
{
public:
	static const float	PI;

	static float Clamp(float x, float low, float high)
	{
		return (high < x ? high : (low > x) ? low : x);
	}

	static bool CheckPowerOfTwo(int nNum)
	{
		int nResult = nNum & (nNum - 1);

		if (nResult == 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	static int CalNumToPowerOfTwo(int nNum) {

		float f = log2((float)nNum) + 0.5f;
		return pow(2.0f, (int)f);
	}
};