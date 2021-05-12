// utils.h

#ifndef CIV4_GAMECORE_UTILS_H
#define CIV4_GAMECORE_UTILS_H


//#include "CvStructs.h"
#include "CvGlobals.h"
#include "CvMap.h"
#include "CvPlotRegion.h"

#ifndef _USRDLL
// use non inline functions when not in the dll
#define getMapINLINE	getMap
#define getGridHeightINLINE	getGridHeight
#define getGridWidthINLINE	getGridWidth
#define isWrapYINLINE	isWrapY
#define isWrapXINLINE	isWrapX
#define plotINLINE	plot
#define getX_INLINE	getX
#define getY_INLINE	getY

#endif

class CvPlot;
class CvCity;
class CvUnit;
class CvString;
class CvRandom;
class FAStarNode;
class FAStar;
class CvInfoBase;


#ifndef SQR
#define SQR(x) ( (x)*(x))
#endif

#undef max
#undef min


// Inline functions.

inline int max(int iA, int iB)
{
	if (iA > iB)
	{
		return iA;
	}
	else
	{
		return iB;
	}
}

inline float max(float fA, float fB)
{
	if (fA > fB)
	{
		return fA;
	}
	else
	{
		return fB;
	}
}

inline int min(int iA, int iB)
{
	if (iA < iB)
	{
		return iA;
	}
	else
	{
		return iB;
	}
}

inline float min(float fA, float fB)
{
	if (fA < fB)
	{
		return fA;
	}
	else
	{
		return fB;
	}
}

inline int range(int iNum, int iLow, int iHigh)
{
	FAssertMsg(iHigh >= iLow, "High should be higher than low");

	if (iNum < iLow)
	{
		return iLow;
	}
	else if (iNum > iHigh)
	{
		return iHigh;
	}
	else
	{
		return iNum;
	}
}

inline float range(float fNum, float fLow, float fHigh)
{
	FAssertMsg(fHigh >= fLow, "High should be higher than low");

	if (fNum < fLow)
	{
		return fLow;
	}
	else if (fNum > fHigh)
	{
		return fHigh;
	}
	else
	{
		return fNum;
	}
}

// on input, iRand should be a random number, preferably of the range 0-31
inline DirectionTypes directionShuffle(DirectionTypes eDirection, int iRand)
{
	FAssert((NUM_DIRECTION_TYPES % 3) != 0 && (NUM_DIRECTION_TYPES % 5) != 0 && (NUM_DIRECTION_TYPES % 7) != 0);

	// if 0 passed in, no shuffle at all
	if (iRand == 0)
	{
		return eDirection;
	}
		
	int iMultiplier;
	switch (iRand  % 4)
	{
	case 0: iMultiplier = 1; break;
	case 1: iMultiplier = 3; break;
	case 2: iMultiplier = 5; break;
	case 3: iMultiplier = 7; break;
	}

	int iIncrement = (iRand / 4) % NUM_DIRECTION_TYPES;
	
	int iDirection = (DirectionTypes) eDirection;
	iDirection = ((iDirection + iIncrement) * iMultiplier) % NUM_DIRECTION_TYPES;

	return ((DirectionTypes) iDirection);
}

// shuffles a range from 0-(iMaxIndex-1), iIndex is the current index, returns a shuffled one
// each iIndex input will result in a unique result for the range, given the same iRand1, iRand2
// on input, iRand1, iRand2 should be a random numbers, preferably of the range 0-(iMaxIndex-1)
inline int intShuffle(int iIndex, int iMaxIndex, int iRand1, int iRand2)
{
	FAssert(iMaxIndex > 1 && iMaxIndex <= MAX_SHORT);

	const int iPrimesCount = 40;
	const int iPrimes[iPrimesCount] = {3, 5, 7, 11, 17, 29, 43, 83, 137, 173, 197, 223, 313, 347, 397, 467, 547, 643, 743, 797, 
		883, 983, 997, 1223, 1367, 1523, 1613, 1823, 1997, 2113, 2347, 2467, 2617, 2797, 2953, 3137, 3313, 3467, 3547, 3613}; 
	
	// increment start random amount forward
	int iIncrement = iRand1 % iMaxIndex;
	iIndex = (iIndex + iIncrement) % iMaxIndex;

	// special case 0
	if (iRand2 == 0)
	{
		return iIndex;
	}

	int iNumMultipliers;
	if (iMaxIndex < 32)
	{
		iNumMultipliers = 4;
	}
	else if (iMaxIndex < 256)
	{
		iNumMultipliers = 8;
	}
	else
	{
		iNumMultipliers = 16;
	}
	
	// special case 0, do not bother with multiplier
	int iMultiplierIndex = iRand2 % iNumMultipliers;
	if (iMultiplierIndex == 0)
	{
		return iIndex;
	}

	// find the prime to use for this iMultiplierIndex
	int iPrimeIndex = 0;
	int iPrimesFound = 0;
	int iMultiplier = 1;
	while (iPrimeIndex < iPrimesCount)
	{
		// get prime to test and increment index
		int iTestPrime = iPrimes[iPrimeIndex++];

		// is this prime not a factor of iMaxIndex? if so its a good one
		if ((iMaxIndex % iTestPrime) != 0)
		{
			// increment found, and check to see if this is the last one
			if (++iPrimesFound == iMultiplierIndex)
			{
				iMultiplier = iTestPrime;
				break;
			}
		}
	}

	FAssertMsg(iPrimeIndex <= iPrimesCount, "ran out of primes");

	// do the multiplier
	iIndex = (iIndex * iMultiplier) % iMaxIndex;
	
	return iIndex;
}

inline int coordDistance(int iFrom, int iTo, int iRange, bool bWrap)
{
	if (bWrap && (abs(iFrom - iTo) > (iRange / 2)))
	{
		return (iRange - abs(iFrom - iTo));
	}

	return abs(iFrom - iTo);
}

inline int wrapCoordDifference(int iDiff, int iRange, bool bWrap)
{
	if (bWrap)
	{
		if (iDiff > (iRange / 2))
		{
			return (iDiff - iRange);
		}
		else if (iDiff < -(iRange / 2))
		{
			return (iDiff + iRange);
		}
	}

	return iDiff;
}

inline int xDistance(int iFromX, int iToX)
{
	return coordDistance(iFromX, iToX, GC.getMapINLINE().getGridWidthINLINE(), GC.getMapINLINE().isWrapXINLINE());
}

inline int yDistance(int iFromY, int iToY)
{
	return coordDistance(iFromY, iToY, GC.getMapINLINE().getGridHeightINLINE(), GC.getMapINLINE().isWrapYINLINE());
}

inline int dxWrap(int iDX)																													// Exposed to Python
{
	return wrapCoordDifference(iDX, GC.getMapINLINE().getGridWidthINLINE(), GC.getMapINLINE().isWrapXINLINE());
}

inline int dyWrap(int iDY)																													// Exposed to Python
{
	return wrapCoordDifference(iDY, GC.getMapINLINE().getGridHeightINLINE(), GC.getMapINLINE().isWrapYINLINE());
}

// 4 | 4 | 3 | 3 | 3 | 4 | 4
// -------------------------
// 4 | 3 | 2 | 2 | 2 | 3 | 4
// -------------------------
// 3 | 2 | 1 | 1 | 1 | 2 | 3
// -------------------------
// 3 | 2 | 1 | 0 | 1 | 2 | 3
// -------------------------
// 3 | 2 | 1 | 1 | 1 | 2 | 3
// -------------------------
// 4 | 3 | 2 | 2 | 2 | 3 | 4
// -------------------------
// 4 | 4 | 3 | 3 | 3 | 4 | 4
//
// Returns the distance between plots according to the pattern above...
inline int plotDistance(int iX1, int iY1, int iX2, int iY2)													// Exposed to Python
{
	int iDX;
	int iDY;

	iDX = xDistance(iX1, iX2);
	iDY = yDistance(iY1, iY2);

	return (max(iDX, iDY) + (min(iDX, iDY) / 2));
}

// 3 | 3 | 3 | 3 | 3 | 3 | 3
// -------------------------
// 3 | 2 | 2 | 2 | 2 | 2 | 3
// -------------------------
// 3 | 2 | 1 | 1 | 1 | 2 | 3
// -------------------------
// 3 | 2 | 1 | 0 | 1 | 2 | 3
// -------------------------
// 3 | 2 | 1 | 1 | 1 | 2 | 3
// -------------------------
// 3 | 2 | 2 | 2 | 2 | 2 | 3
// -------------------------
// 3 | 3 | 3 | 3 | 3 | 3 | 3
//
// Returns the distance between plots according to the pattern above...
inline int stepDistance(int iX1, int iY1, int iX2, int iY2)													// Exposed to Python
{
	return max(xDistance(iX1, iX2), yDistance(iY1, iY2));
}

inline CvPlot* plotDirection(int iX, int iY, DirectionTypes eDirection)							// Exposed to Python
{
	if(eDirection == NO_DIRECTION)
	{
		return GC.getMapINLINE().plotINLINE(iX, iY);
	}
	else
	{
		return GC.getMapINLINE().plotINLINE((iX + GC.getPlotDirectionX()[eDirection]), (iY + GC.getPlotDirectionY()[eDirection]));
	}
}

inline CvPlot* plotCardinalDirection(int iX, int iY, CardinalDirectionTypes eCardinalDirection)	// Exposed to Python
{
	return GC.getMapINLINE().plotINLINE((iX + GC.getPlotCardinalDirectionX()[eCardinalDirection]), (iY + GC.getPlotCardinalDirectionY()[eCardinalDirection]));
}

inline CvPlot* plotXY(int iX, int iY, int iDX, int iDY)																// Exposed to Python
{
	return GC.getMapINLINE().plotINLINE((iX + iDX), (iY + iDY));
}

inline DirectionTypes directionXY(int iDX, int iDY)																		// Exposed to Python
{
	if ((abs(iDX) > DIRECTION_RADIUS) || (abs(iDY) > DIRECTION_RADIUS))
	{
		return NO_DIRECTION;
	}
	else
	{
		return GC.getXYDirection((iDX + DIRECTION_RADIUS), (iDY + DIRECTION_RADIUS));
	}
}

inline DirectionTypes directionXY(const CvPlot* pFromPlot, const CvPlot* pToPlot)			// Exposed to Python
{
	return directionXY(dxWrap(pToPlot->getX_INLINE() - pFromPlot->getX_INLINE()), dyWrap(pToPlot->getY_INLINE() - pFromPlot->getY_INLINE()));
}

inline bool rangeIndexToXY(const int iRange, const int iIndex, int &iDX, int &iDY)
{
	iDX = 0;
	iDY = 0;
	
	if (iRange == 0 && iIndex == 0)
	{
		return true;
	}

	// set index
	if (iRange > 0)
	{
		int iDiameter = (iRange * 2) + 1;
		int iCheck = iIndex;
		
		// top row
		iCheck -= iDiameter;
		if (iCheck < 0)
		{
			iDX = iCheck + iRange + 1;
			iDY = -iRange;
			return true;
		}
		else
		{
			// bottom row
			iCheck -= iDiameter;
			if (iCheck < 0)
			{
				iDX = iCheck + iRange + 1;
				iDY = iRange;
				return true;
			}
			else
			{
				// the columns are 2 shorter, so we do not count the same plot twice
				iDiameter -= 2;
				
				// left column
				iCheck -= iDiameter;
				if (iCheck < 0)
				{
					iDX = -iRange;
					iDY = iCheck + iRange;
					return true;
				}
				else
				{
					// right column
					iCheck -= iDiameter;
					if (iCheck < 0)
					{
						iDX = iRange;
						iDY = iCheck + iRange;
						return true;
					}
					else
					{
						// index out of range, return false
						return false;
					}
				}
			}
		}
	}

	return false;
}

// - | O | - | - | - | - | -		- | - | U | - | - | - | -
// -------------------------		-------------------------
// - | Y | X | - | - | Z | -		- | - | - | O | - | - | -
// -------------------------		-------------------------
// - | - | X | - | - | Y | O		U | - | Z | Y | X | - | -	
// -------------------------		-------------------------
// - | O | Y | C | - | X | -		- | O | Y | C | Y | O | -
// -------------------------		-------------------------
// - | - | Z | - | - | - | -		- | - | X | Y | Z | - | U
// -------------------------		-------------------------
// - | X | - | - | X | Y | -		- | - | - | O | - | - | -
// -------------------------		-------------------------
// O | - | - | - | - | O | -		- | - | - | - | U | - | -
//
// given a collection of plots centered at (iCenterX, iCenterY), and
// a outer plot at (iOuterDX,iOuterDY), returns plot inside and adjacent to start plot, indexed by iIndex
// will return a minimum of one plot (iIndex == 0), and a max of 3 plots (up to iIndex == 2)
// in the above picture given input C and O, the return plots are X (and Y) (and Z)
// if a (iUltraOuterDX, iUltraOuterDY) are provided, they influence which direction gets the lower index when iOuterDX or iOuterDY is zero
// and a direction reversal is prevented
inline CvPlot* getIndexedInnerPlot(int iCenterX, int iCenterY, int iOuterDX, int iOuterDY, int iIndex, int iUltraOuterDX = 0, int iUltraOuterDY = 0)
{
	int iPlotX, iPlotY;
	bool bPlotValid = false;
	
	int iRange = max(abs(iOuterDX), abs(iOuterDY));
	int iXNormal = (iOuterDX < 0) ? -1 : 1;
	int iYNormal = (iOuterDY < 0) ? -1 : 1;

	// handle special ultra outer cases
	int iOuterToUltraDX = 0;
	int iOuterToUltraDY = 0;
	if (iUltraOuterDX != 0 || iUltraOuterDY != 0)
	{
		iOuterToUltraDX = iUltraOuterDX - iOuterDX;
		iOuterToUltraDY = iUltraOuterDY - iOuterDY;

		// if in the middle, set the normal
		if (iOuterDX == 0)
		{
			iXNormal = (iOuterToUltraDX < 0) ? -1 : 1;
		}
		
		if (iOuterDY == 0)
		{
			iYNormal = (iOuterToUltraDY < 0) ? -1 : 1;
		}
	}
	
	CvMap& kMap = GC.getMapINLINE();

	// loop through all possibilities, until a match is found
	int iValidCount = 0;
	for (int iPass = 0; iValidCount <= iIndex && iPass <= 2; iPass++)
	{
		bPlotValid = false;
		// handle tiny sizes
		if (iRange <= 1)
		{
			// if range is 1, then there is only one inner plot, the center (if range zero, then there are no plots inside)
			if (iRange == 1 && iPass == 0)
			{
				iPlotX = iCenterX;
				iPlotY = iCenterY;
				bPlotValid = true;
			}
		}
		else
		{
			// first position is diagonally in both directions (always exists, by definition)
			if (iPass == 0)
			{
				iPlotX = iCenterX + iOuterDX - iXNormal;
				iPlotY = iCenterY + iOuterDY - iYNormal;
				bPlotValid = true;
			}
			// second position is moving straight inside
			else if (iPass == 1)
			{
				// are we horizontally in from the corner? (then directly inside vertically exists)
				if (abs(iOuterDX) < iRange)
				{
					iPlotX = iCenterX + iOuterDX;
					iPlotY = iCenterY + iOuterDY - iYNormal;
					bPlotValid = true;
				}
				// are we vertically in from the corner? (then directly inside horizontally exists)
				else if (abs(iOuterDY) < iRange)
				{
					iPlotX = iCenterX + iOuterDX - iXNormal;
					iPlotY = iCenterY + iOuterDY;
					bPlotValid = true;
				}
			}
			// third position is moving diagonally, contrary direction to the center of this edge
			else if (iPass == 2)
			{
				// are we in horizontally enough? (then inside vertically and contrary horizontally direction exists)
				if (abs(iOuterDX) < (iRange - 1))
				{
					iPlotX = iCenterX + iOuterDX + iXNormal;
					iPlotY = iCenterY + iOuterDY - iYNormal;
					bPlotValid = true;
				}
				// are we in vertically enough? (then inside horizontally and contrary vertically direction exists)
				else if (abs(iOuterDY) < (iRange - 1))
				{
					iPlotX = iCenterX + iOuterDX - iXNormal;
					iPlotY = iCenterY + iOuterDY + iYNormal;
					bPlotValid = true;
				}
			}
		}

		// invalidate direction switches
		if (bPlotValid)
		{
			int iPlotToOuterDX = iOuterDX - iPlotX + iCenterX;
			int iPlotToOuterDY = iOuterDY - iPlotY + iCenterY;
			if ((iOuterToUltraDX > 0 && iPlotToOuterDX < 0) || (iOuterToUltraDX < 0 && iPlotToOuterDX > 0) ||
				(iOuterToUltraDY > 0 && iPlotToOuterDY < 0) || (iOuterToUltraDY < 0 && iPlotToOuterDY > 0))
			{
				bPlotValid = false;
			}
		}

		// wrap coordinates and invalidate plots off the edge
		if (bPlotValid)
		{
			iPlotX = coordRange(iPlotX, kMap.getGridWidthINLINE(), kMap.isWrapXINLINE());
			iPlotY = coordRange(iPlotY, kMap.getGridHeightINLINE(), kMap.isWrapYINLINE());
			if (!kMap.isPlotINLINE(iPlotX, iPlotY))
			{
				bPlotValid = false;
			}
		}

		// count valid plots until we get to iIndex'th one
		if (bPlotValid)
		{
			iValidCount++;
		}
	}
	
	// did we find enough valid plots?
	if (bPlotValid && iValidCount > iIndex)
	{
		return kMap.plotSorenINLINE(iPlotX, iPlotY);
	}

	return NULL;
}

template<class _RegionClass>
void addPlotsAtRange(_RegionClass& plotRegion, int iX, int iY, int iRange, bool bUsePlotDistance = false)
{
	for (int iDY = -(iRange); iDX <= iRange; iDX++)
	{
		for (int iDX = -(iRange); iDY <= iRange; iDY++)
		{
			CvPlot* pLoopPlot = plotXY(iX, iY, iDX, iDY);
			if (pLoopPlot != NULL)
			{
				XYCoords xy(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE());

				// skip if using plot distance and it is too high
				if (bUsePlotDistance && plotDistance(iX, iY, xy.iX, xy.iY) > iRange)
				{
					continue;
				}

				// add the plot to region (note, this can reset the data value of a CvPlotDataRegion to zero)
				adjacentRegion.insert(plotRegion.end(), makeXYCoords(plotRegion.end(), xy));
			}
		}
	}
}

// finds all adjacent plots within iRange for sourceRegion, if bIncludeOriginal true, then the result includes the original region plots
template<class _RegionClass>
void findAdjacentPlots(const _RegionClass& sourceRegion, _RegionClass& adjacentRegion, bool bIncludeOriginal = false, int iRange = 1)
{
	// we are going to perform multiple steps
	// step (1) smear the region horizontally, allowing negative and invalid coordinate values
	// step (2) smear the region vertically, allowing negative and invalid coordinate values
	// step (3) clip and wrap the final result to valid range
	
	// setup result
	if (bIncludeOriginal)
	{
		adjacentRegion = sourceRegion;
	}
	else
	{
		adjacentRegion.clear();
	}

	_RegionClass::const_iterator itSource = sourceRegion.begin();

	// if empty region, bail
	if (itSource == sourceRegion.end())
	{
		return;
	}
	
	//XYCoords topLeft = getXYCoords(itSource);

	// first smear horizontal adjacent plots
	_RegionClass horizontalRegion;
	while (itSource != sourceRegion.end())
	{
		// start this block
		XYCoords blockXY = getXYCoords(itSource);

		// add iRange plots to the left
		for (int iI = iRange; iI > 0; iI--)
		{
			// add left, (hint to end)
			FAssert(sourceRegion.find(XYCoords(blockXY.iX - iI, blockXY.iY)) == sourceRegion.end());
			horizontalRegion.insert(horizontalRegion.end(), makeXYCoords(horizontalRegion.end(), XYCoords(blockXY.iX - iI, blockXY.iY)));
		}

		// loop until we are at the end or find a big enough gap
		XYCoords sourceXY, nextXY;
		while (itSource != sourceRegion.end())
		{
			// find the end of contiguous plots
			_RegionClass::const_iterator itNext = itSource;
			while (++itNext != sourceRegion.end())
			{
				sourceXY = getXYCoords(itSource);
				nextXY = getXYCoords(itNext);
				if (nextXY.iY != blockXY.iY || nextXY.iX != sourceXY.iX + 1)
				{
					break;				
				}

				itSource = itNext;
			}
			
			// are we at end or end of row or is gap big enough to have a skip?
			sourceXY = getXYCoords(itSource);
			nextXY = getXYCoords(itNext);
			if (itNext == sourceRegion.end() || nextXY.iY != blockXY.iY || nextXY.iX > sourceXY.iX + (2 * iRange) + 1)
			{
				// add iRange plots to the right
				for (int iI = 1; iI <= iRange; iI++)
				{
					// add right, (hint to end)
					FAssert(sourceRegion.find(XYCoords(sourceXY.iX + iI, blockXY.iY)) == sourceRegion.end());
					horizontalRegion.insert(horizontalRegion.end(), makeXYCoords(horizontalRegion.end(), XYCoords(sourceXY.iX + iI, blockXY.iY)));
				}

				// increment
				itSource++;

				// stop loop, go to next block
				break;

			}
			
			// fill in the too-small hole, then loop again, gap is less than ((2 * iRange) + 1)
			int iGapSize = nextXY.iX - sourceXY.iX - 1;

			// add iRange plots to the right
			for (int iI = 1; iI <= iGapSize; iI++)
			{
				// add right, (hint to end)
				FAssert(sourceRegion.find(XYCoords(sourceXY.iX + iI, blockXY.iY)) == sourceRegion.end());
				horizontalRegion.insert(horizontalRegion.end(), makeXYCoords(horizontalRegion.end(), XYCoords(sourceXY.iX + iI, blockXY.iY)));
			}

			// increment, search again for a hole or next line
			itSource++;
		}
	}

	// now smear vertical adjacent plots, just do this one brute force for now

	// first smear plots from source
	for (_RegionClass::const_iterator itSource = sourceRegion.begin(); itSource != sourceRegion.end(); itSource++)
	{
		XYCoords sourceXY = getXYCoords(itSource);

		// add iRange plots up
		for (int iI = iRange; iI > 0; iI--)
		{
			// if not in source, then add it to adjacent
			XYCoords adjacentXY(sourceXY.iX, sourceXY.iY - iI);
			if (sourceRegion.find(adjacentXY) == sourceRegion.end())
			{
				adjacentRegion.insert(adjacentRegion.begin(), makeXYCoords(adjacentRegion.begin(), adjacentXY));
			}
		}

		// add iRange plots down
		for (int iI = 1; iI <= iRange; iI++)
		{
			// if not in source, then add it to adjacent
			XYCoords adjacentXY(sourceXY.iX, sourceXY.iY + iI);
			if (sourceRegion.find(adjacentXY) == sourceRegion.end())
			{
				adjacentRegion.insert(adjacentRegion.end(), makeXYCoords(adjacentRegion.end(), adjacentXY));
			}
		}
	}
	
	// now smear plots from horizontal
	for (_RegionClass::const_iterator itHorizontal = horizontalRegion.begin(); itHorizontal != horizontalRegion.end(); itHorizontal++)
	{
		// add the horizontal smear to the destination as well (we do not have to check for presense in source)
		XYCoords sourceXY = getXYCoords(itHorizontal);
		adjacentRegion.insert(adjacentRegion.begin(), makeXYCoords(adjacentRegion.begin(), sourceXY));

		// add iRange plots up
		for (int iI = iRange; iI > 0; iI--)
		{
			// if not in source, then add it to adjacent (note we do not bother to check for duplicates with previous smear)
			XYCoords adjacentXY(sourceXY.iX, sourceXY.iY - iI);
			if (sourceRegion.find(adjacentXY) == sourceRegion.end())
			{
				adjacentRegion.insert(adjacentRegion.begin(), makeXYCoords(adjacentRegion.begin(), adjacentXY));
			}
		}

		// add iRange plots down
		for (int iI = 1; iI <= iRange; iI++)
		{
			// if not in source, then add it to adjacent (note we do not bother to check for duplicates with previous smear)
			XYCoords adjacentXY(sourceXY.iX, sourceXY.iY + iI);
			if (sourceRegion.find(adjacentXY) == sourceRegion.end())
			{
				adjacentRegion.insert(adjacentRegion.end(), makeXYCoords(adjacentRegion.end(), adjacentXY));
			}
		}
	}
	
	// clip/wrap any plots that are off the edge
	_RegionClass::iterator it = adjacentRegion.begin();
	while (it != adjacentRegion.end())
	{
		// increment before we work with it, so we can remove it if necessary
		_RegionClass::iterator itAdjacent = it;
		it++;

		// does plot exist on map?
		XYCoords adjacentXY = getXYCoords(itAdjacent);
		CvPlot* adjacentPlot = GC.getMapINLINE().plotINLINE(adjacentXY.iX, adjacentXY.iY);
		if (adjacentPlot != NULL)
		{
			// did it wrap?
			XYCoords wrapXY(adjacentPlot->getX_INLINE(), adjacentPlot->getY_INLINE());
			if (adjacentXY != wrapXY)
			{
				// add wrapped one, if it is not already in source or destination
				if (sourceRegion.find(wrapXY) == sourceRegion.end())
				{
					if (adjacentRegion.find(wrapXY) == adjacentRegion.end())
					{
						adjacentRegion.insert(makeXYCoords(itAdjacent, wrapXY));
					}
				}
				
				// remove old one
				adjacentRegion.erase(itAdjacent);
			}
		}
		// doesnt exist, remove it
		else
		{
			adjacentRegion.erase(itAdjacent);
		}
	}
}

inline bool getBit(unsigned int bitField, int bit)
{
	if (bit >= 0 && bit < 32)
	{
		return (bitField & (((unsigned int)1) << bit));
	}

	return false;
}

inline void setBit(unsigned int& bitField, int bit)
{
	if (bit >= 0 && bit < 32)
	{
		bitField |= (((unsigned int)1) << bit);
	}
}

inline void clearBit(unsigned int& bitField, int bit)
{
	if (bit >= 0 && bit < 32)
	{
		bitField &= ~(((unsigned int)1) << bit);
	}
}

// to be used for display purposes only! IDs could overlap
inline int shortenID(int iID)
{
	return (iID & 0xFFF) + 1;
	//return (((iID/1000)/1459) + (iID%1000));
}


// these string functions should only be used under chipotle cheat code (not internationalized)
void getCardinalDirectionTypeString(CvWString& szString, CardinalDirectionTypes eDirectionType);
void getDirectionTypeString(CvWString& szString, DirectionTypes eDirectionType);
void getActivityTypeString(CvWString& szString, ActivityTypes eActivityType);
void getMissionTypeString(CvWString& szString, MissionTypes eMissionType);
void getMissionAIString(CvWString& szString, MissionAITypes eMissionAI);
void getUnitAIString(CvWString& szString, UnitAITypes eUnitAI);

#ifdef FASSERT_ENABLE
//#define DEBUG_AI_UPDATE_GROUPS
inline void DEBUGLOG(const char* format, ...)
{
	char temp[256];
	sprintf(temp, "");
	va_list args;
	va_start(args, format);
	vsprintf(temp, format, args);
	va_end(args);
	OutputDebugString(temp);
}
#else
#define DEBUGLOG(x)
#endif

DllExport CvPlot* plotCity(int iX, int iY, int iIndex);																			// Exposed to Python
DllExport int plotCityXY(int iDX, int iDY);																									// Exposed to Python
DllExport int plotCityXY(const CvCity* pCity, const CvPlot* pPlot);													// Exposed to Python

DllExport CardinalDirectionTypes getOppositeCardinalDirection(CardinalDirectionTypes eDir);	// Exposed to Python 
DllExport DirectionTypes cardinalDirectionToDirection(CardinalDirectionTypes eCard);				// Exposed to Python
DllExport bool isCardinalDirection(DirectionTypes eDirection);															// Exposed to Python
DllExport DirectionTypes estimateDirection(int iDX, int iDY);																// Exposed to Python
DllExport float directionAngle(DirectionTypes eDirection);

void addVisiblePlots(CvPlotRegion& visiblePlots, int iX, int iY, int iRange, TeamTypes eTeam);
void addVisiblePlots(CvPlotDataRegion& visiblePlots, int iX, int iY, int iRange, int iObserverCount, TeamTypes eTeam);

void buildVisibilityRegion(CvPlotRegion& visiblePlots, int iX, int iY, const CLinkList<IDInfo>& unitList, TeamTypes eTeam = NO_TEAM);
void buildVisibilityRegion(CvPlotDataRegion& visiblePlots, int iX, int iY, const CLinkList<IDInfo>& unitList, TeamTypes eTeam = NO_TEAM);

DllExport bool atWar(TeamTypes eTeamA, TeamTypes eTeamB);												// Exposed to Python
DllExport bool isPotentialEnemy(TeamTypes eOurTeam, TeamTypes eTheirTeam);			// Exposed to Python

DllExport CvCity* getCity(IDInfo city);	// Exposed to Python
DllExport CvUnit* getUnit(IDInfo unit);	// Exposed to Python

DllExport bool isBeforeUnitCycle(const CvUnit* pFirstUnit, const CvUnit* pSecondUnit);
DllExport bool isPromotionValid(PromotionTypes ePromotion, UnitTypes eUnit);	// Exposed to Python

DllExport int getPopulationAsset(int iPopulation);								// Exposed to Python
DllExport int getLandPlotsAsset(int iLandPlots);									// Exposed to Python
DllExport int getPopulationPower(int iPopulation);								// Exposed to Python
DllExport int getPopulationScore(int iPopulation);								// Exposed to Python
DllExport int getLandPlotsScore(int iLandPlots);									// Exposed to Python
DllExport int getTechScore(TechTypes eTech);											// Exposed to Python
DllExport int getWonderScore(BuildingClassTypes eWonderClass);		// Exposed to Python

DllExport ImprovementTypes finalImprovementUpgrade(ImprovementTypes eImprovement, int iCount = 0);		// Exposed to Python

DllExport int getWorldSizeMaxConscript(CivicTypes eCivic);								// Exposed to Python

DllExport bool isReligionTech(TechTypes eTech);														// Exposed to Python

DllExport bool isTechRequiredForUnit(TechTypes eTech, UnitTypes eUnit);							// Exposed to Python
DllExport bool isTechRequiredForBuilding(TechTypes eTech, BuildingTypes eBuilding);	// Exposed to Python
DllExport bool isTechRequiredForProject(TechTypes eTech, ProjectTypes eProject);		// Exposed to Python

DllExport bool isWorldUnitClass(UnitClassTypes eUnitClass);											// Exposed to Python
DllExport bool isTeamUnitClass(UnitClassTypes eUnitClass);											// Exposed to Python
DllExport bool isNationalUnitClass(UnitClassTypes eUnitClass);									// Exposed to Python
DllExport bool isLimitedUnitClass(UnitClassTypes eUnitClass);										// Exposed to Python

DllExport bool isWorldWonderClass(BuildingClassTypes eBuildingClass);						// Exposed to Python
DllExport bool isTeamWonderClass(BuildingClassTypes eBuildingClass);						// Exposed to Python
DllExport bool isNationalWonderClass(BuildingClassTypes eBuildingClass);				// Exposed to Python
DllExport bool isLimitedWonderClass(BuildingClassTypes eBuildingClass);					// Exposed to Python
int limitedWonderClassLimit(BuildingClassTypes eBuildingClass);

DllExport bool isWorldProject(ProjectTypes eProject);														// Exposed to Python
DllExport bool isTeamProject(ProjectTypes eProject);														// Exposed to Python
DllExport bool isLimitedProject(ProjectTypes eProject);													// Exposed to Python

DllExport __int64 getBinomialCoefficient(int iN, int iK);
DllExport int getCombatOdds(CvUnit* pAttacker, CvUnit* pDefender);							// Exposed to Python

DllExport void setTradeItem(TradeData* pItem, TradeableItems eItemType = TRADE_ITEM_NONE, int iData = 0);

DllExport void clear(wchar* szString);
DllExport void clear(char* szString);
DllExport void clear(std::string& szString);
DllExport void clear(std::wstring& szString);
DllExport void safecpy(char * szDest, const char * szSource, int iMaxLen);
DllExport void safecpy(wchar * szDest, const wchar * szSource, int iMaxLen);
DllExport void safecpy(CvWString& szDest, const CvWString& szSource, int iMaxLen);
DllExport bool isEmpty(const char* szString);
DllExport bool isEmpty(const std::string& szStr);
DllExport bool isEmpty(const wchar* szString);
DllExport bool isEmpty(const std::wstring& szStr);
DllExport void setListHelp(wchar* szBuffer, const wchar* szStart, const wchar* szItem, const wchar* szSeparator, bool bFirst);
DllExport void setListHelp(CvWString& szBuffer, const wchar* szStart, const wchar* szItem, const wchar* szSeparator, bool bFirst);

// PlotUnitFunc's...
DllExport bool PUF_isGroupHead( const CvUnit* pUnit, int iData1 = -1, int iData2 = -1);
DllExport bool PUF_isPlayer( const CvUnit* pUnit, int iData1, int iData2 = -1);
DllExport bool PUF_isTeam( const CvUnit* pUnit, int iData1, int iData2 = -1);
DllExport bool PUF_isOtherPlayer( const CvUnit* pUnit, int iData1, int iData2 = -1);
DllExport bool PUF_isOtherTeam( const CvUnit* pUnit, int iData1, int iData2 = -1);
DllExport bool PUF_isEnemy( const CvUnit* pUnit, int iData1, int iData2 = -1);
DllExport bool PUF_isVisible( const CvUnit* pUnit, int iData1, int iData2 = -1);
DllExport bool PUF_isVisibleDebug( const CvUnit* pUnit, int iData1, int iData2 = -1);
DllExport bool PUF_canSiege( const CvUnit* pUnit, int iData1, int iData2 = -1);
DllExport bool PUF_isPotentialEnemy( const CvUnit* pUnit, int iData1, int iData2 = -1);
DllExport bool PUF_canDeclareWar( const CvUnit* pUnit, int iData1 = -1, int iData2 = -1);
DllExport bool PUF_canDefend( const CvUnit* pUnit, int iData1 = -1, int iData2 = -1);
DllExport bool PUF_cannotDefend( const CvUnit* pUnit, int iData1 = -1, int iData2 = -1);
DllExport bool PUF_canDefendGroupHead( const CvUnit* pUnit, int iData1 = -1, int iData2 = -1);
DllExport bool PUF_canDefendEnemy( const CvUnit* pUnit, int iData1, int iData2 = -1);
DllExport bool PUF_canDefendPotentialEnemy( const CvUnit* pUnit, int iData1, int iData2 = -1);
DllExport bool PUF_canAirAttack( const CvUnit* pUnit, int iData1 = -1, int iData2 = -1);
DllExport bool PUF_canAirDefend( const CvUnit* pUnit, int iData1 = -1, int iData2 = -1);
DllExport bool PUF_isFighting( const CvUnit* pUnit, int iData1, int iData2 = -1);
DllExport bool PUF_isAnimal( const CvUnit* pUnit, int iData1 = -1, int iData2 = -1);
DllExport bool PUF_isMilitaryHappiness( const CvUnit* pUnit, int iData1 = -1, int iData2 = -1);
DllExport bool PUF_isInvestigate( const CvUnit* pUnit, int iData1 = -1, int iData2 = -1);
DllExport bool PUF_isCounterSpy( const CvUnit* pUnit, int iData1 = -1, int iData2 = -1);
DllExport bool PUF_isUnitType( const CvUnit* pUnit, int iData1, int iData2 = -1);
DllExport bool PUF_isDomainType( const CvUnit* pUnit, int iData1, int iData2 = -1);
DllExport bool PUF_isUnitAIType( const CvUnit* pUnit, int iData1, int iData2 = -1);
DllExport bool PUF_isCityAIType( const CvUnit* pUnit, int iData1, int iData2 = -1);
DllExport bool PUF_isNotCityAIType( const CvUnit* pUnit, int iData1, int iData2 = -1);
DllExport bool PUF_isSelected( const CvUnit* pUnit, int iData1 = -1, int iData2 = -1);
DllExport bool PUF_makeInfoBarDirty(CvUnit* pUnit, int iData1 = -1, int iData2 = -1);

// Inet Stuff
DllExport void sendGameStats(wchar* pURL);

// FAStarFunc...
DllExport int potentialIrrigation(FAStarNode* parent, FAStarNode* node, int data, const void* pointer, FAStar* finder);
DllExport int checkFreshWater(FAStarNode* parent, FAStarNode* node, int data, const void* pointer, FAStar* finder);
DllExport int changeIrrigated(FAStarNode* parent, FAStarNode* node, int data, const void* pointer, FAStar* finder);
DllExport int pathDestValid(int iToX, int iToY, const void* pointer, FAStar* finder);
DllExport int pathHeuristic(int iFromX, int iFromY, int iToX, int iToY);
DllExport int pathCost(FAStarNode* parent, FAStarNode* node, int data, const void* pointer, FAStar* finder);
DllExport int pathValid(FAStarNode* parent, FAStarNode* node, int data, const void* pointer, FAStar* finder);
DllExport int pathAdd(FAStarNode* parent, FAStarNode* node, int data, const void* pointer, FAStar* finder);
DllExport int stepDestValid(int iToX, int iToY, const void* pointer, FAStar* finder);
DllExport int stepHeuristic(int iFromX, int iFromY, int iToX, int iToY);
DllExport int stepValid(FAStarNode* parent, FAStarNode* node, int data, const void* pointer, FAStar* finder);
DllExport int stepCost(FAStarNode* parent, FAStarNode* node, int data, const void* pointer, FAStar* finder);
DllExport int stepAdd(FAStarNode* parent, FAStarNode* node, int data, const void* pointer, FAStar* finder);
DllExport int routeValid(FAStarNode* parent, FAStarNode* node, int data, const void* pointer, FAStar* finder);
DllExport int borderValid(FAStarNode* parent, FAStarNode* node, int data, const void* pointer, FAStar* finder);
DllExport int areaValid(FAStarNode* parent, FAStarNode* node, int data, const void* pointer, FAStar* finder);
DllExport int joinArea(FAStarNode* parent, FAStarNode* node, int data, const void* pointer, FAStar* finder);
DllExport int plotGroupValid(FAStarNode* parent, FAStarNode* node, int data, const void* pointer, FAStar* finder);
DllExport int countPlotGroup(FAStarNode* parent, FAStarNode* node, int data, const void* pointer, FAStar* finder);

DllExport int baseYieldToSymbol(int iNumYieldTypes, int iYieldStack);

DllExport bool isPickableName(const TCHAR* szName);

DllExport int* shuffle(int iNum, CvRandom& rand);
DllExport void shuffleArray(int* piShuffle, int iNum, CvRandom& rand);

DllExport int getTurnYearForGame(int iGameTurn, int iStartYear, CalendarTypes eCalendar, GameSpeedTypes eSpeed);

DllExport void boolsToString(const bool* pBools, int iNumBools, CvString* szOut);
DllExport void stringToBools(const char* szString, int* iNumBools, bool** ppBools);

#endif
