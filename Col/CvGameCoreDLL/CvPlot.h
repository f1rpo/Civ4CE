#pragma once

// CvPlot.h

#ifndef CIV4_PLOT_H
#define CIV4_PLOT_H

//#include "CvStructs.h"
#include "LinkedList.h"
#include <bitset>

#pragma warning( disable: 4251 )		// needs to have dll-interface to be used by clients of class

class CvArea;
class CvMap;
class CvPlotBuilder;
class CvRoute;
class CvRiver;
class CvCity;
class CvFeature;
class CvUnit;
class CvSymbol;
class CvFlagEntity;

typedef bool (*ConstPlotUnitFunc)( const CvUnit* pUnit, int iData1, int iData2);
typedef bool (*PlotUnitFunc)(CvUnit* pUnit, int iData1, int iData2);

class CvPlot
{

public:
	CvPlot();
	virtual ~CvPlot();

	void init(int iX, int iY);
	void uninit();
	void reset(int iX = 0, int iY = 0, bool bConstructorCall=false);
	void setupGraphical();
	DllExport void erase();

	DllExport float getPointX() const;
	DllExport float getPointY() const;
	DllExport NiPoint3 getPoint() const;

	DllExport TeamTypes getTeam() const;

	void doTurn();

	void doImprovement();

	void updateCulture(bool bBumpUnits);

	void updateFog();
	void updateVisibility();

	void updateSymbolDisplay();
	void updateSymbolVisibility();
	void updateSymbols();

	void updateMinimapColor();

	void updateCenterUnit();

	void verifyUnitValidPlot();

    bool isAdjacentToPlot(CvPlot* pPlot) const;
	bool isAdjacentToArea(int iAreaID) const;
	bool isAdjacentToArea(const CvArea* pArea) const;
	bool shareAdjacentArea( const CvPlot* pPlot) const;
	bool isAdjacentToLand() const;
	bool isCoastalLand(int iMinWaterSize = -1) const;
	bool isAdjacentWaterPassable(CvPlot* pPlot) const;

	bool isVisibleWorked() const;
	bool isWithinTeamCityRadius(TeamTypes eTeam, PlayerTypes eIgnorePlayer = NO_PLAYER) const;

	DllExport bool isLake() const;

	DllExport bool isRiverMask() const;
	DllExport bool isRiverCrossingFlowClockwise(DirectionTypes eDirection) const;
	bool isRiverSide() const;
	bool isRiver() const;
	bool isRiverConnection(DirectionTypes eDirection) const;

	CvPlot* getNearestLandPlotInternal(int iDistance) const;
	int getNearestLandArea() const;
	CvPlot* getNearestLandPlot() const;

	int seeFromLevel(TeamTypes eTeam) const;
	int seeThroughLevel() const;
	void changeAdjacentSight(TeamTypes eTeam, int iRange, bool bIncrement, CvUnit* pUnit);
	bool canSeePlot(CvPlot *plot, TeamTypes eTeam, int iRange, DirectionTypes eFacingDirection) const;
	bool canSeeDisplacementPlot(TeamTypes eTeam, int dx, int dy, int originalDX, int originalDY, bool firstPlot, bool outerRing) const;
	bool shouldProcessDisplacementPlot(int dx, int dy, int range, DirectionTypes eFacingDirection) const;
	void updateSight(bool bIncrement);
	void updateSeeFromSight(bool bIncrement);
	DllExport bool canHaveBonus(BonusTypes eBonus, bool bIgnoreLatitude = false) const;
	DllExport bool canHaveImprovement(ImprovementTypes eImprovement, TeamTypes eTeam = NO_TEAM, bool bPotential = false) const;

	bool canBuild(BuildTypes eBuild, PlayerTypes ePlayer = NO_PLAYER, bool bTestVisible = false) const;
	int getBuildTime(BuildTypes eBuild) const;
	DllExport int getBuildTurnsLeft(BuildTypes eBuild, int iNowExtra, int iThenExtra) const;
	DllExport int getFeatureYield(BuildTypes eBuild, YieldTypes eYield, TeamTypes eTeam, CvCity** ppCity) const;

	DllExport CvUnit* getBestDefender(PlayerTypes eOwner, PlayerTypes eAttackingPlayer = NO_PLAYER, const CvUnit* pAttacker = NULL, bool bTestAtWar = false, bool bTestPotentialEnemy = false, bool bTestCanMove = false) const;
	int AI_sumStrength(PlayerTypes eOwner, PlayerTypes eAttackingPlayer = NO_PLAYER, DomainTypes eDomainType = NO_DOMAIN, bool bDefensiveBonuses = true, bool bTestAtWar = false, bool bTestPotentialEnemy = false) const;
	CvUnit* getSelectedUnit() const;
	int getUnitPower(PlayerTypes eOwner = NO_PLAYER) const;

	DllExport int defenseModifier(TeamTypes eDefender, bool bHelp = false) const;
	int movementCost(const CvUnit* pUnit, const CvPlot* pFromPlot) const;

	bool isAdjacentOwned() const;
	bool isAdjacentPlayer(PlayerTypes ePlayer, bool bLandOnly = false) const;
	bool isAdjacentTeam(TeamTypes eTeam, bool bLandOnly = false) const;
	bool isWithinCultureRange(PlayerTypes ePlayer) const;
	int getNumCultureRangeCities(PlayerTypes ePlayer) const;

	DllExport PlayerTypes calculateCulturalOwner() const;
	void plotAction(PlotUnitFunc func, int iData1 = -1, int iData2 = -1, PlayerTypes eOwner = NO_PLAYER, TeamTypes eTeam = NO_TEAM);
	DllExport int plotCount(ConstPlotUnitFunc funcA, int iData1A = -1, int iData2A = -1, PlayerTypes eOwner = NO_PLAYER, TeamTypes eTeam = NO_TEAM, ConstPlotUnitFunc funcB = NULL, int iData1B = -1, int iData2B = -1) const;
	CvUnit* plotCheck(ConstPlotUnitFunc funcA, int iData1A = -1, int iData2A = -1, PlayerTypes eOwner = NO_PLAYER, TeamTypes eTeam = NO_TEAM, ConstPlotUnitFunc funcB = NULL, int iData1B = -1, int iData2B = -1) const;
	bool isOwned() const;

	DllExport bool isVisible(TeamTypes eTeam, bool bDebug) const;
	DllExport bool isActiveVisible(bool bDebug) const;
	bool isVisibleToCivTeam() const;
	bool isVisibleToWatchingHuman() const;
	bool isAdjacentVisible(TeamTypes eTeam, bool bDebug) const;
	bool isAdjacentNonvisible(TeamTypes eTeam) const;

	DllExport bool isGoody(TeamTypes eTeam = NO_TEAM) const;
	bool isRevealedGoody(TeamTypes eTeam = NO_TEAM) const;
	void removeGoody();

	DllExport bool isCity(bool bCheckImprovement = false, TeamTypes eForTeam = NO_TEAM) const;
	DllExport bool isFriendlyCity(const CvUnit& kUnit, bool bCheckImprovement) const;
	bool isEnemyCity(const CvUnit& kUnit) const;

	bool isOccupation() const;
	DllExport bool isBeingWorked() const;

	bool isUnit() const;
	bool isVisibleEnemyDefender(const CvUnit* pUnit) const;
	CvUnit *getVisibleEnemyDefender(PlayerTypes ePlayer) const;
	int getNumDefenders(PlayerTypes ePlayer) const;
	int getNumVisibleEnemyDefenders(const CvUnit* pUnit) const;
	int getNumVisiblePotentialEnemyDefenders(const CvUnit* pUnit) const;
	DllExport bool isVisibleEnemyUnit(PlayerTypes ePlayer) const;
	bool isVisibleEnemyUnit(const CvUnit* pUnit) const;
	bool isVisibleOtherUnit(PlayerTypes ePlayer) const;
	DllExport bool isFighting() const;

	DllExport bool canHaveFeature(FeatureTypes eFeature) const;

	DllExport bool isRoute() const;
	bool isValidRoute(const CvUnit* pUnit) const;
	bool isValidDomainForLocation(const CvUnit& unit) const;
	bool isValidDomainForAction(const CvUnit& unit) const;
	bool isValidDomainForAction(UnitTypes eUnit) const;
	DllExport bool isImpassable() const;

	DllExport int getX() const;
#ifdef _USRDLL
	inline int getX_INLINE() const
	{
		return m_iX;
	}
#endif
	DllExport int getY() const;
#ifdef _USRDLL
	inline int getY_INLINE() const
	{
		return m_iY;
	}
#endif
	bool at(int iX, int iY) const;
	int getLatitude() const;
	int getFOWIndex() const;
	DllExport CvArea* area() const;
	CvArea* waterArea() const;
	CvArea* secondWaterArea() const;
	DllExport int getArea() const;
	void setArea(int iNewValue);

	DllExport int getFeatureVariety() const;

	int getOwnershipDuration() const;
	bool isOwnershipScore() const;
	void setOwnershipDuration(int iNewValue);
	void changeOwnershipDuration(int iChange);

	int getImprovementDuration() const;
	void setImprovementDuration(int iNewValue);
	void changeImprovementDuration(int iChange);

	DllExport int getUpgradeProgress() const;
	DllExport int getUpgradeTimeLeft(ImprovementTypes eImprovement, PlayerTypes ePlayer) const;
	void setUpgradeProgress(int iNewValue);
	void changeUpgradeProgress(int iChange);

	int getForceUnownedTimer() const;
	bool isForceUnowned() const;
	void setForceUnownedTimer(int iNewValue);
	void changeForceUnownedTimer(int iChange);

	int getCityRadiusCount() const;
	int isCityRadius() const;
	void changeCityRadiusCount(int iChange);
	bool isStartingPlot() const;
	void setStartingPlot(bool bNewValue);

	DllExport bool isNOfRiver() const;
	DllExport void setNOfRiver(bool bNewValue, CardinalDirectionTypes eRiverDir);

	DllExport bool isWOfRiver() const;
	DllExport void setWOfRiver(bool bNewValue, CardinalDirectionTypes eRiverDir);

	DllExport CardinalDirectionTypes getRiverNSDirection() const;
	DllExport CardinalDirectionTypes getRiverWEDirection() const;
	EuropeTypes getEurope() const;
	void setEurope(EuropeTypes eEurope);
	EuropeTypes getNearestEurope() const;
	bool isEuropeAccessable() const;

	CvPlot* getInlandCorner() const;
	bool hasCoastAtSECorner() const;
	bool isPotentialCityWork() const;
	bool isPotentialCityWorkForArea(CvArea* pArea) const;
	void updatePotentialCityWork();
	bool isShowCitySymbols() const;
	void updateShowCitySymbols();
	bool isFlagDirty() const;
	void setFlagDirty(bool bNewValue);

	DllExport PlayerTypes getOwner() const;
#ifdef _USRDLL
	inline PlayerTypes getOwnerINLINE() const
	{
		return (PlayerTypes)m_eOwner;
	}
#endif
	void setOwner(PlayerTypes eNewValue, bool bCheckUnits);
	DllExport PlotTypes getPlotType() const;
	DllExport bool isWater() const;
	DllExport bool isEurope() const;
	bool isFlatlands() const;
	DllExport bool isHills() const;
	DllExport bool isPeak() const;
	DllExport void setPlotType(PlotTypes eNewValue, bool bRecalculate = true, bool bRebuildGraphics = true);

	DllExport TerrainTypes getTerrainType() const;
	DllExport void setTerrainType(TerrainTypes eNewValue, bool bRecalculate = true, bool bRebuildGraphics = true);

	DllExport FeatureTypes getFeatureType() const;
	DllExport void setFeatureType(FeatureTypes eNewValue, int iVariety = -1);
	DllExport BonusTypes getBonusType() const;
	DllExport void setBonusType(BonusTypes eNewValue);

	DllExport ImprovementTypes getImprovementType() const;
	DllExport void setImprovementType(ImprovementTypes eNewValue);

	DllExport RouteTypes getRouteType() const;
	DllExport void setRouteType(RouteTypes eNewValue);
	void updateCityRoute();
	DllExport CvCity* getPlotCity() const;
	void setPlotCity(CvCity* pNewValue);
	DllExport CvCity* getWorkingCity() const;
	void updateWorkingCity();
	CvCity* getWorkingCityOverride() const;
	void setWorkingCityOverride( const CvCity* pNewValue);
	int getRiverID() const;
	void setRiverID(int iNewValue);

	int getMinOriginalStartDist() const;
	void setMinOriginalStartDist(int iNewValue);
	int getRiverCrossingCount() const;
	void changeRiverCrossingCount(int iChange);
	short* getYield();
	DllExport int getYield(YieldTypes eIndex) const;
	int calculateNatureYield(YieldTypes eIndex, TeamTypes eTeam, bool bIgnoreFeature = false) const;
	int calculateBestNatureYield(YieldTypes eIndex, TeamTypes eTeam) const;
	int calculateTotalBestNatureYield(TeamTypes eTeam) const;
	int calculateImprovementYieldChange(ImprovementTypes eImprovement, YieldTypes eYield, PlayerTypes ePlayer, bool bOptimal = false) const;
	int calculateYield(YieldTypes eIndex, bool bDisplay) const;
	int calculatePotentialYield(YieldTypes eIndex, const CvUnit* pUnit, bool bDisplay) const;
	int calculatePotentialYield(YieldTypes eYield, PlayerTypes ePlayer, ImprovementTypes eImprovement, bool bIgnoreFeature, RouteTypes eRoute, UnitTypes eUnit, bool bDisplay) const;
	int calculatePotentialProfessionYieldAmount(ProfessionTypes eProfession, const CvUnit* pUnit, bool bDisplay) const;
	bool hasYield() const;
	void updateYield(bool bUpdateCity);
	int calculateMaxYield(YieldTypes eYield) const;
	int getYieldWithBuild(BuildTypes eBuild, YieldTypes eYield, bool bWithUpgrade) const;
	DllExport int getCulture(PlayerTypes eIndex) const;
	DllExport int countTotalCulture() const;
	int countFriendlyCulture(TeamTypes eTeam) const;
	TeamTypes findHighestCultureTeam() const;
	PlayerTypes findHighestCulturePlayer() const;
	DllExport int calculateCulturePercent(PlayerTypes eIndex) const;
	int calculateTeamCulturePercent(TeamTypes eIndex) const;
	void setCulture(PlayerTypes eIndex, int iNewValue, bool bUpdate);
	void changeCulture(PlayerTypes eIndex, int iChange, bool bUpdate);
	int getBuyCultureAmount(PlayerTypes ePlayer) const;
	int getBuyPrice(PlayerTypes ePlayer) const;
	int getFoundValue(PlayerTypes eIndex);
	bool isBestAdjacentFound(PlayerTypes eIndex);
	void setFoundValue(PlayerTypes eIndex, int iNewValue);
	int getPlayerCityRadiusCount(PlayerTypes eIndex) const;
	bool isPlayerCityRadius(PlayerTypes eIndex) const;
	void changePlayerCityRadiusCount(PlayerTypes eIndex, int iChange);

	int getVisibilityCount(TeamTypes eTeam) const;
	void changeVisibilityCount(TeamTypes eTeam, int iChange, InvisibleTypes eSeeInvisible);

	DllExport PlayerTypes getRevealedOwner(TeamTypes eTeam, bool bDebug) const;
	DllExport TeamTypes getRevealedTeam(TeamTypes eTeam, bool bDebug) const;
	void setRevealedOwner(TeamTypes eTeam, PlayerTypes eNewValue);
	void updateRevealedOwner(TeamTypes eTeam);
	DllExport bool isRiverCrossing(DirectionTypes eIndex) const;
	void updateRiverCrossing(DirectionTypes eIndex);
	void updateRiverCrossing();
	DllExport bool isRevealed(TeamTypes eTeam, bool bDebug) const;
	DllExport void setRevealed(TeamTypes eTeam, bool bNewValue, bool bTerrainOnly, TeamTypes eFromTeam);
	bool isAdjacentRevealed(TeamTypes eTeam) const;
	bool isAdjacentNonrevealed(TeamTypes eTeam) const;

	ImprovementTypes getRevealedImprovementType(TeamTypes eTeam, bool bDebug) const;
	void setRevealedImprovementType(TeamTypes eTeam, ImprovementTypes eNewValue);

	RouteTypes getRevealedRouteType(TeamTypes eTeam, bool bDebug) const;
	void setRevealedRouteType(TeamTypes eTeam, RouteTypes eNewValue);

	int getBuildProgress(BuildTypes eBuild) const;
	bool changeBuildProgress(BuildTypes eBuild, int iChange, TeamTypes eTeam = NO_TEAM);

	void updateFeatureSymbolVisibility();
	void updateFeatureSymbol(bool bForce = false, bool bBuildTileArt = true);

	DllExport bool isLayoutDirty() const;							// The plot layout contains bonuses and improvements --- it is, like the city layout, passively computed by LSystems
	DllExport void setLayoutDirty(bool bDirty);
	DllExport bool isLayoutStateDifferent() const;
	DllExport void setLayoutStateToCurrent();
	bool updatePlotBuilder();

	DllExport void getVisibleImprovementState(ImprovementTypes& eType, bool& bWorked);				// determines how the improvement state is shown in the engine
	DllExport void getVisibleBonusState(BonusTypes& eType, bool& bImproved, bool& bWorked);		// determines how the bonus state is shown in the engine
	DllExport bool shouldUsePlotBuilder();
	DllExport CvPlotBuilder* getPlotBuilder() { return m_pPlotBuilder; }

	DllExport CvRoute* getRouteSymbol() const;
	void updateRouteSymbol(bool bForce = false, bool bAdjacent = false);

	DllExport CvRiver* getRiverSymbol() const;
	void updateRiverSymbol(bool bForce = false, bool bAdjacent = false);

	CvFeature* getFeatureSymbol() const;

	DllExport CvFlagEntity* getFlagSymbol() const;
	DllExport CvFlagEntity* getFlagSymbolOffset() const;
	DllExport void updateFlagSymbol();

	DllExport CvUnit* getCenterUnit() const;
	DllExport CvUnit* getDebugCenterUnit() const;
	void setCenterUnit(CvUnit* pNewValue);
	int getCultureRangeCities(PlayerTypes eOwnerIndex, int iRangeIndex) const;
	bool isCultureRangeCity(PlayerTypes eOwnerIndex, int iRangeIndex) const;
	void changeCultureRangeCities(PlayerTypes eOwnerIndex, int iRangeIndex, int iChange);
	int getInvisibleVisibilityCount(TeamTypes eTeam, InvisibleTypes eInvisible) const;
	bool isInvisibleVisible(TeamTypes eTeam, InvisibleTypes eInvisible) const;
	void changeInvisibleVisibilityCount(TeamTypes eTeam, InvisibleTypes eInvisible, int iChange);

	DllExport int getNumUnits() const;
	DllExport CvUnit* getUnitByIndex(int iIndex) const;
	void addUnit(CvUnit* pUnit, bool bUpdate = true);
	void removeUnit(CvUnit* pUnit, bool bUpdate = true);
	DllExport CLLNode<IDInfo>* nextUnitNode(CLLNode<IDInfo>* pNode) const;
	DllExport CLLNode<IDInfo>* prevUnitNode(CLLNode<IDInfo>* pNode) const;
	DllExport CLLNode<IDInfo>* headUnitNode() const;
	DllExport CLLNode<IDInfo>* tailUnitNode() const;

	// Script data needs to be a narrow string for pickling in Python
	CvString getScriptData() const;
	void setScriptData(const char* szNewValue);

	bool canTrigger(EventTriggerTypes eTrigger, PlayerTypes ePlayer) const;
	bool canApplyEvent(EventTypes eEvent) const;
	void applyEvent(EventTypes eEvent);

	bool canTrain(UnitTypes eUnit, bool bContinue, bool bTestVisible) const;
	bool isValidYieldChanges(UnitTypes eUnit) const;

	void setDistanceToOcean(int iNewValue);
	int getDistanceToOcean() const;

	CvPlot* findNearbyOceanPlot(int iRandomization = 25);

	int getCrumbs() const;
	void addCrumbs(int iQuantity);

	DllExport const char* getResourceLayerIcon(ResourceLayerOptions eOption, CvWStringBuffer& szHelp, PlotIndicatorVisibilityFlags& eVisibilityFlag, ColorTypes& eColor) const;
	DllExport CvUnit* getUnitLayerUnit(UnitLayerOptionTypes eOption, CvWStringBuffer& szHelp, PlotIndicatorVisibilityFlags& eVisibilityFlag, ColorTypes& eColor, bool& bTestEnemyVisibility) const;

	void read(FDataStreamBase* pStream);
	void write(FDataStreamBase* pStream);

protected:

	void updateImpassable();

	short m_iX;
	short m_iY;
	int m_iArea;
	mutable CvArea *m_pPlotArea;
	short m_iFeatureVariety;
	short m_iOwnershipDuration;
	short m_iImprovementDuration;
	short m_iUpgradeProgress;
	short m_iForceUnownedTimer;
	short m_iCityRadiusCount;
	int m_iRiverID;
	short m_iMinOriginalStartDist;
	short m_iRiverCrossingCount;
	short m_iDistanceToOcean;
	short m_iCrumbs;

	bool m_bStartingPlot:1;
	bool m_bHills:1;
	bool m_bNOfRiver:1;
	bool m_bWOfRiver:1;
	bool m_bPotentialCityWork:1;
	bool m_bShowCitySymbols:1;
	bool m_bFlagDirty:1;
	bool m_bPlotLayoutDirty:1;
	bool m_bLayoutStateWorked:1;
	bool m_bImpassable:1;

	char /*PlayerTypes*/ m_eOwner;
	short /*PlotTypes*/ m_ePlotType;
	short /*TerrainTypes*/ m_eTerrainType;
	short /*FeatureTypes*/ m_eFeatureType;
	short /*BonusTypes*/ m_eBonusType;
	short /*ImprovementTypes*/ m_eImprovementType;
	short /*RouteTypes*/ m_eRouteType;
	char /*CardinalDirectionTypes*/ m_eRiverNSDirection;
	char /*CardinalDirectionTypes*/ m_eRiverWEDirection;
	char /*EuropeTypes*/ m_eEurope;

	IDInfo m_plotCity;
	IDInfo m_workingCity;
	IDInfo m_workingCityOverride;

	short* m_aiYield;
	int* m_aiCulture;
	int* m_aiFoundValue;
	char* m_aiPlayerCityRadiusCount;
	short* m_aiVisibilityCount;
	char* m_aiRevealedOwner;

	bool* m_abRiverCrossing;	// bit vector
	bool* m_abRevealed;

	short* /*ImprovementTypes*/ m_aeRevealedImprovementType;
	short* /*RouteTypes*/ m_aeRevealedRouteType;

	char* m_szScriptData;

	short* m_paiBuildProgress;

	CvFeature* m_pFeatureSymbol;
	CvRoute* m_pRouteSymbol;
	CvRiver* m_pRiverSymbol;
	CvFlagEntity* m_pFlagSymbol;
	CvFlagEntity* m_pFlagSymbolOffset;
	CvUnit* m_pCenterUnit;

	CvPlotBuilder* m_pPlotBuilder;		// builds bonuses and improvements

	char** m_apaiCultureRangeCities;
	short** m_apaiInvisibleVisibilityCount;

	CLinkList<IDInfo> m_units;

	void doFeature();

	void processArea(CvArea* pArea, int iChange);
	void doImprovementUpgrade();

	ColorTypes plotMinimapColor();

	// added so under cheat mode we can access protected stuff
	friend class CvGameTextMgr;
};

#endif
