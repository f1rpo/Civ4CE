// utils.h

#ifndef CIV4_GAMECORE_UTILS_H
#define CIV4_GAMECORE_UTILS_H


//#include "CvStructs.h"
#include "CvGlobals.h"
#include "CvMap.h"

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


DllExport CvPlot* plotCity(int iX, int iY, int iIndex);																			// Exposed to Python
DllExport int plotCityXY(int iDX, int iDY);																									// Exposed to Python
DllExport int plotCityXY(const CvCity* pCity, const CvPlot* pPlot);													// Exposed to Python

DllExport CardinalDirectionTypes getOppositeCardinalDirection(CardinalDirectionTypes eDir);	// Exposed to Python 
DllExport DirectionTypes cardinalDirectionToDirection(CardinalDirectionTypes eCard);				// Exposed to Python
DllExport bool isCardinalDirection(DirectionTypes eDirection);															// Exposed to Python
DllExport DirectionTypes estimateDirection(int iDX, int iDY);																// Exposed to Python
DllExport float directionAngle(DirectionTypes eDirection);

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
