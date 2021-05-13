#pragma once

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

//sign function taken from FirePlace - JW
template<class T> __forceinline T getSign( T x ) { return (( x < 0 ) ? T(-1) : x > 0 ? T(1) : T(0)); };

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

inline int dxWrap(int iDX)
{
	return wrapCoordDifference(iDX, GC.getMapINLINE().getGridWidthINLINE(), GC.getMapINLINE().isWrapXINLINE());
}

inline int dyWrap(int iDY)
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
inline int plotDistance(int iX1, int iY1, int iX2, int iY2)
{
	int iDX;
	int iDY;

	iDX = xDistance(iX1, iX2);
	iDY = yDistance(iY1, iY2);

	return (std::max(iDX, iDY) + (std::min(iDX, iDY) / 2));
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
inline int stepDistance(int iX1, int iY1, int iX2, int iY2)
{
	return std::max(xDistance(iX1, iX2), yDistance(iY1, iY2));
}

inline CvPlot* plotDirection(int iX, int iY, DirectionTypes eDirection)
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

inline CvPlot* plotCardinalDirection(int iX, int iY, CardinalDirectionTypes eCardinalDirection)
{
	return GC.getMapINLINE().plotINLINE((iX + GC.getPlotCardinalDirectionX()[eCardinalDirection]), (iY + GC.getPlotCardinalDirectionY()[eCardinalDirection]));
}

inline CvPlot* plotXY(int iX, int iY, int iDX, int iDY)
{
	return GC.getMapINLINE().plotINLINE((iX + iDX), (iY + iDY));
}

inline DirectionTypes directionXY(int iDX, int iDY)
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

inline DirectionTypes directionXY(const CvPlot* pFromPlot, const CvPlot* pToPlot)
{
	return directionXY(dxWrap(pToPlot->getX_INLINE() - pFromPlot->getX_INLINE()), dyWrap(pToPlot->getY_INLINE() - pFromPlot->getY_INLINE()));
}

DllExport CvPlot* plotCity(int iX, int iY, int iIndex);
DllExport int plotCityXY(int iDX, int iDY);
DllExport int plotCityXY(const CvCity* pCity, const CvPlot* pPlot);

DllExport CardinalDirectionTypes getOppositeCardinalDirection(CardinalDirectionTypes eDir);
DllExport DirectionTypes cardinalDirectionToDirection(CardinalDirectionTypes eCard);
DllExport bool isCardinalDirection(DirectionTypes eDirection);
DllExport DirectionTypes estimateDirection(int iDX, int iDY);
DllExport DirectionTypes estimateDirection(const CvPlot* pFromPlot, const CvPlot* pToPlot);
DllExport float directionAngle(DirectionTypes eDirection);

DllExport bool atWar(TeamTypes eTeamA, TeamTypes eTeamB);
DllExport bool isPotentialEnemy(TeamTypes eOurTeam, TeamTypes eTheirTeam);

DllExport CvCity* getCity(IDInfo city);
DllExport CvUnit* getUnit(IDInfo unit);

DllExport bool isBeforeUnitCycle(const CvUnit* pFirstUnit, const CvUnit* pSecondUnit);
DllExport bool isPromotionValid(PromotionTypes ePromotion, UnitTypes eUnit, bool bLeader);

DllExport int getPopulationAsset(int iPopulation);
DllExport int getLandPlotsAsset(int iLandPlots);
DllExport int getPopulationPower(int iPopulation);
DllExport int getPopulationScore(int iPopulation);
DllExport int getLandPlotsScore(int iLandPlots);
DllExport int getTechScore(TechTypes eTech);
DllExport int getWonderScore(BuildingClassTypes eWonderClass);

DllExport ImprovementTypes finalImprovementUpgrade(ImprovementTypes eImprovement, int iCount = 0);

DllExport int getWorldSizeMaxConscript(CivicTypes eCivic);

DllExport bool isReligionTech(TechTypes eTech);

DllExport bool isTechRequiredForUnit(TechTypes eTech, UnitTypes eUnit);
DllExport bool isTechRequiredForBuilding(TechTypes eTech, BuildingTypes eBuilding);
DllExport bool isTechRequiredForProject(TechTypes eTech, ProjectTypes eProject);

DllExport bool isWorldUnitClass(UnitClassTypes eUnitClass);
DllExport bool isTeamUnitClass(UnitClassTypes eUnitClass);
DllExport bool isNationalUnitClass(UnitClassTypes eUnitClass);
DllExport bool isLimitedUnitClass(UnitClassTypes eUnitClass);

DllExport bool isWorldWonderClass(BuildingClassTypes eBuildingClass);
DllExport bool isTeamWonderClass(BuildingClassTypes eBuildingClass);
DllExport bool isNationalWonderClass(BuildingClassTypes eBuildingClass);
DllExport bool isLimitedWonderClass(BuildingClassTypes eBuildingClass);
DllExport int limitedWonderClassLimit(BuildingClassTypes eBuildingClass);

DllExport bool isWorldProject(ProjectTypes eProject);
DllExport bool isTeamProject(ProjectTypes eProject);
DllExport bool isLimitedProject(ProjectTypes eProject);

DllExport __int64 getBinomialCoefficient(int iN, int iK);
DllExport int getCombatOdds(CvUnit* pAttacker, CvUnit* pDefender);

DllExport int getEspionageModifier(TeamTypes eOurTeam, TeamTypes eTargetTeam);

DllExport void setTradeItem(TradeData* pItem, TradeableItems eItemType = TRADE_ITEM_NONE, int iData = 0);

DllExport bool isPlotEventTrigger(EventTriggerTypes eTrigger);

DllExport TechTypes getDiscoveryTech(UnitTypes eUnit, PlayerTypes ePlayer);

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
DllExport void setListHelp(CvWStringBuffer& szBuffer, const wchar* szStart, const wchar* szItem, const wchar* szSeparator, bool bFirst);

// PlotUnitFunc's...
DllExport bool PUF_isGroupHead( const CvUnit* pUnit, int iData1 = -1, int iData2 = -1);
DllExport bool PUF_isPlayer( const CvUnit* pUnit, int iData1, int iData2 = -1);
DllExport bool PUF_isTeam( const CvUnit* pUnit, int iData1, int iData2 = -1);
DllExport bool PUF_isCombatTeam(const CvUnit* pUnit, int iData1, int iData2);
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
DllExport bool PUF_isSpy( const CvUnit* pUnit, int iData1 = -1, int iData2 = -1);
DllExport bool PUF_isUnitType( const CvUnit* pUnit, int iData1, int iData2 = -1);
DllExport bool PUF_isDomainType( const CvUnit* pUnit, int iData1, int iData2 = -1);
DllExport bool PUF_isUnitAIType( const CvUnit* pUnit, int iData1, int iData2 = -1);
DllExport bool PUF_isCityAIType( const CvUnit* pUnit, int iData1, int iData2 = -1);
DllExport bool PUF_isNotCityAIType( const CvUnit* pUnit, int iData1, int iData2 = -1);
DllExport bool PUF_isSelected( const CvUnit* pUnit, int iData1 = -1, int iData2 = -1);
DllExport bool PUF_makeInfoBarDirty(CvUnit* pUnit, int iData1 = -1, int iData2 = -1);
bool PUF_isNoMission(const CvUnit* pUnit, int iData1 = -1, int iData2 = -1);
bool PUF_isFiniteRange(const CvUnit* pUnit, int iData1 = -1, int iData2 = -1);

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

DllExport int getTurnMonthForGame(int iGameTurn, int iStartYear, CalendarTypes eCalendar, GameSpeedTypes eSpeed);
DllExport int getTurnYearForGame(int iGameTurn, int iStartYear, CalendarTypes eCalendar, GameSpeedTypes eSpeed);

DllExport void boolsToString(const bool* pBools, int iNumBools, CvString* szOut);
DllExport void stringToBools(const char* szString, int* iNumBools, bool** ppBools);

void getDirectionTypeString(CvWString& szString, DirectionTypes eDirectionType);
void getCardinalDirectionTypeString(CvWString& szString, CardinalDirectionTypes eDirectionType);
void getActivityTypeString(CvWString& szString, ActivityTypes eActivityType);
void getMissionTypeString(CvWString& szString, MissionTypes eMissionType);
void getMissionAIString(CvWString& szString, MissionAITypes eMissionAI);
void getUnitAIString(CvWString& szString, UnitAITypes eUnitAI);

#endif
