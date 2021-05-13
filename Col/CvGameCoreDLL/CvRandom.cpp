// random.cpp

#include "CvGameCoreDLL.h"
#include "CvRandom.h"
#include "CvGlobals.h"
#include "CyArgsList.h"
#include "CvGameAI.h"
#include <numeric>
#include <limits>

#define RANDOM_A      (1103515245)
#define RANDOM_C      (12345)
#define RANDOM_SHIFT  (16)

CvRandom::CvRandom()
{
	reset();
}


CvRandom::~CvRandom()
{
	uninit();
}


void CvRandom::init(unsigned long ulSeed)
{
	//--------------------------------
	// Init saved data
	reset(ulSeed);

	//--------------------------------
	// Init non-saved data
}


void CvRandom::uninit()
{
}


// FUNCTION: reset()
// Initializes data members that are serialized.
void CvRandom::reset(unsigned long ulSeed)
{
	//--------------------------------
	// Uninit class
	uninit();

	m_ulRandomSeed = ulSeed;
}


unsigned short CvRandom::get(unsigned short usNum, const TCHAR* pszLog)
{
	if (pszLog != NULL)
	{
		if (GC.getLogging() && GC.getRandLogging())
		{
			if (GC.getGameINLINE().getTurnSlice() > 0)
			{
				TCHAR szOut[1024];
				sprintf(szOut, "Rand = %d on %d (%s)\n", getSeed(), GC.getGameINLINE().getTurnSlice(), pszLog);
				gDLL->messageControlLog(szOut);
			}
		}
	}

	m_ulRandomSeed = ((RANDOM_A * m_ulRandomSeed) + RANDOM_C);

	unsigned short us = ((unsigned short)((((m_ulRandomSeed >> RANDOM_SHIFT) & MAX_UNSIGNED_SHORT) * ((unsigned long)usNum)) / (MAX_UNSIGNED_SHORT + 1)));

	return us;
}


float CvRandom::getFloat()
{
	return (((float)(get(MAX_UNSIGNED_SHORT))) / ((float)MAX_UNSIGNED_SHORT));
}

/* boxmuller.c           Implements the Polar form of the Box-Muller
Transformation

(c) Copyright 1994, Everett F. Carter Jr.
Permission is granted by the author to use
this software for any application provided this
copyright notice is preserved.

*/

float CvRandom::getGaussian(float fMean, float fStandardDeviation)
{
	float x1, x2, w, y1;
	static float y2;
	static int use_last = 0;

	if (use_last)		        /* use value from previous call */
	{
		y1 = y2;
		use_last = 0;
	}
	else
	{
		do
		{
			x1 = 2.0f * getFloat() - 1.0f;
			x2 = 2.0f * getFloat() - 1.0f;
			w = x1 * x1 + x2 * x2;
		} while ( w >= 1.0f );

		w = sqrt( (-2.0f * log( w ) ) / w );
		y1 = x1 * w;
		y2 = x2 * w;
		use_last = 1;
	}

	return( fMean + y1 * fStandardDeviation );
}

int CvRandom::pickValue(std::vector<int>& aWeights, const TCHAR* pszLog)
{
	int iTotalWeights = std::accumulate(aWeights.begin(), aWeights.end(), 0);
	FAssert(iTotalWeights >= 0);
	FAssert(iTotalWeights <= std::numeric_limits<unsigned short>::max());

	int iValue = get(iTotalWeights, pszLog);
	int iSum = 0;
	for (int i = 0; i < (int)aWeights.size(); ++i)
	{
		iSum += aWeights[i];
		if (iValue < iSum)
		{
			return i;
		}
	}

	FAssert(false);
	return 0;
}

void CvRandom::shuffleArray(std::vector<int>& aNumbers, const TCHAR* pszLog)
{
	for (uint iI = 0; iI < aNumbers.size(); iI++)
	{
		int iJ = (get(aNumbers.size() - iI, pszLog) + iI);

		if (iI != iJ)
		{
			int iTemp = aNumbers[iI];
			aNumbers[iI] = aNumbers[iJ];
			aNumbers[iJ] = iTemp;
		}
	}
}

void CvRandom::shuffleSequence(std::vector<int>& aNumbers, const TCHAR* pszLog)
{
	for (uint i = 0; i < aNumbers.size(); ++i)
	{
		aNumbers[i] = i;
	}
	shuffleArray(aNumbers, pszLog);
}


void CvRandom::reseed(unsigned long ulNewValue)
{
	m_ulRandomSeed = ulNewValue;
}


unsigned long CvRandom::getSeed()
{
	return m_ulRandomSeed;
}


void CvRandom::read(FDataStreamBase* pStream)
{
	reset();

	pStream->Read(&m_ulRandomSeed);
}


void CvRandom::write(FDataStreamBase* pStream)
{
	pStream->Write(m_ulRandomSeed);
}
