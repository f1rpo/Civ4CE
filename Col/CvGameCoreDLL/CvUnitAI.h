#pragma once

// unitAI.h

#ifndef CIV4_UNIT_AI_H
#define CIV4_UNIT_AI_H

#include "CvUnit.h"

class CvCity;

class CvUnitAI : public CvUnit
{

public:

	CvUnitAI();
	virtual ~CvUnitAI();

	// PatchMod: Treasure Fix START
	CvPlot* AI_canLoadAdjacent(CvPlot* pPlot, bool bTestCity);
	bool AI_loadAdjacent(CvPlot* pPlot, bool bTestCity);
	bool AI_hasTransportOfSize(int iBerthSize);
	// PatchMod: Treasure Fix END

	void AI_init();
	void AI_uninit();
	void AI_reset();

	bool AI_update();
	bool AI_europeUpdate();
	bool AI_follow();

	void AI_upgrade();

	void AI_promote();

	int AI_groupFirstVal();
	int AI_groupSecondVal();

	int AI_attackOdds(const CvPlot* pPlot, bool bPotentialEnemy) const;

	bool AI_bestCityBuild(CvCity* pCity, CvPlot** ppBestPlot = NULL, BuildTypes* peBestBuild = NULL, CvPlot* pIgnorePlot = NULL, CvUnit* pUnit = NULL);

	bool AI_isCityAIType() const;

	int AI_getBirthmark() const;
	void AI_setBirthmark(int iNewValue);

	UnitAITypes AI_getUnitAIType() const;
	void AI_setUnitAIType(UnitAITypes eNewValue);

	UnitAIStates AI_getUnitAIState() const;
	void AI_setUnitAIState(UnitAIStates eNewValue);

	int AI_sacrificeValue(const CvPlot* pPlot) const;

	CvPlot* AI_determineDestination(CvPlot** ppMissionPlot, MissionTypes* peMission, MissionAITypes* peMissionAI);

	bool AI_moveFromTransport(CvPlot* pHintPlot);
	bool AI_attackFromTransport(CvPlot* pHintPlot, int iLowOddsThreshold, int iHighOddsThreshold);
	int AI_getMovePriority() const;
	void AI_setMovePriority(int iNewValue);
	bool AI_hasAIChanged(int iNumTurns);
	int AI_getLastAIChangeTurn();

	void AI_doInitialMovePriority();
	void AI_doFound();
	
	ProfessionTypes AI_getOldProfession() const;
	void AI_setOldProfession(ProfessionTypes eProfession);
	ProfessionTypes AI_getIdealProfession() const;

	void read(FDataStreamBase* pStream);
	void write(FDataStreamBase* pStream);

protected:

	int m_iBirthmark;
	int m_iMovePriority;
	int m_iLastAIChangeTurn;

	UnitAITypes m_eUnitAIType;

	UnitAIStates m_eUnitAIState;
	ProfessionTypes m_eOldProfession;

	int m_iAutomatedAbortTurn;
	
	mutable ProfessionTypes m_eIdealProfessionCache;

	void AI_colonistMove();
	void AI_settlerMove();
	void AI_workerMove();
	void AI_missionaryMove();
	void AI_scoutMove();
	void AI_treasureMove();
	void AI_yieldUhMove();
	void AI_generalMove();
	
	void AI_offensiveMove();
	void AI_defensiveMove();
	void AI_counterMove();
	
	void AI_defensiveBraveMove();
	void AI_offensiveBraveMove();
	void AI_counterBraveMove();
	
	void AI_transportMove();
	void AI_transportMoveRoutes();
	void AI_transportMoveFull();
	void AI_pirateMove();

	void AI_imperialShipMove();
	void AI_imperialSoldierMove();
	void AI_imperialMountedMove();
	void AI_imperialCannonMove();

	void AI_transportSeaMove();
	void AI_assaultSeaMove();
	void AI_combatSeaMove();

	void AI_exploreMove();
	void AI_exploreSeaMove();

	void AI_networkAutomated();
	void AI_cityAutomated();

	bool AI_travelToEuropeColony(int iMinAttitude, int iMaxAttitude, int iRange = -1);
	bool AI_guardHomeColony();
	bool AI_findNewHomeColony();

	bool AI_europeBuyNativeYields();
	bool AI_europeBuyYields();
	bool AI_europe();
	bool AI_europeAssaultSea();


	bool AI_sailToEurope(bool bMove = true);
	bool AI_travelToPort(int iMinPercent = 25, int iMaxPath = MAX_INT);

	int AI_loadYieldValue(CvCity* pCity, YieldTypes eYield, int iAmount, int iTravelTurns = 0);
	int AI_unloadYieldValue(CvCity* pCity, YieldTypes eYield, int iAmount, int iTravelTurns = 0);

	bool AI_collectGoods();

	bool AI_deliverUnits();
	bool AI_loadUnits(UnitAITypes eUnitAI, MissionAITypes eMissionAI);
	
	bool AI_wakeCargo(UnitAITypes eUnitAI, int iPriority);

	int AI_promotionValue(PromotionTypes ePromotion);

	bool AI_seaPatrol();

	bool AI_moveTowardsOcean(int iRange);

	bool AI_moveTowardsVictimCity();
	bool AI_spreadReligion();

	bool AI_learn(int iRange = 8);

	bool AI_requestPickup(int iMaxPath = MAX_INT);
	bool AI_respondToPickup(int iMaxPath = MAX_INT, UnitAITypes eUnitAI = NO_UNITAI);

	bool AI_pickupAdjacantUnits();

	bool AI_continueMission(int iAbortDistance, MissionAITypes eValidMissionAI, int iFlags = 0, bool bStepwise = true);
	bool AI_breakAutomation();

	bool AI_unloadWhereNeeded(int iMaxPath = MAX_INT);

	bool AI_betterJob();
	bool AI_upgradeProfession();
	bool AI_changeUnitAIType(int iMinMultiplier = 0);

	bool AI_advance(bool bAttack);
	bool AI_loiter(int iMinDistance, int iMaxDistance, bool bAttack);
	bool AI_retreat(int iMaxDistance);
	bool AI_counter(int iTether);
	bool AI_shouldRun();

	bool AI_shadow(UnitAITypes eUnitAI, int iMax = -1, int iMaxRatio = -1, bool bWithCargoOnly = true);
	bool AI_group(UnitAITypes eUnitAI, int iMaxGroup = -1, int iMaxOwnUnitAI = -1, int iMinUnitAI = -1, bool bIgnoreFaster = false, bool bIgnoreOwnUnitType = false, bool bStackOfDoom = false, int iMaxPath = MAX_INT, bool bAllowRegrouping = false);
	bool AI_load(UnitAITypes eUnitAI, MissionAITypes eMissionAI, UnitAITypes eTransportedUnitAI = NO_UNITAI, int iMinCargo = -1, int iMinCargoSpace = -1, int iMaxCargoSpace = -1, int iMaxCargoOurUnitAI = -1, int iFlags = 0, int iMaxPath = MAX_INT);
	bool AI_guardCityBestDefender();
	bool AI_guardCityMinDefender();
	bool AI_guardCity(bool bAll = false, int iMaxPath = MAX_INT);
	bool AI_guardCityCounter(int iMaxPath = MAX_INT);
	int AI_getPlotDefendersNeeded(CvPlot* pPlot, int iExtra);
	bool AI_guardFort(bool bSearch = true);
	bool AI_guardCitySite();
	bool AI_chokeDefend();
	bool AI_heal(int iDamagePercent = 0, int iMaxPath = MAX_INT);
	bool AI_afterAttack();
	bool AI_lead(std::vector<UnitAITypes>& aeAIUnitTypes);
	bool AI_protect(int iOddsThreshold);
	bool AI_bravePatrol();
	bool AI_patrol();
	bool AI_defend();
	bool AI_safety();
	bool AI_hide();
	bool AI_goody();
	bool AI_goodyRange(int iRange);
	
	bool AI_isValidExplore(CvPlot* pPlot);
	int AI_explorePlotValue(CvPlot* pPlot, bool bImportantOnly = false);
	bool AI_explore(bool bFavorOpenBorders = false);
	bool AI_exploreRange(int iRange);
	bool AI_exploreFromShip(int iMaxPath);
	bool AI_exploreCoast(int iRange);
	bool AI_exploreOcean(int iRange);
	bool AI_exploreDeep();
	bool AI_exploreOpenBorders(int iRange);
	
	bool AI_targetCity(int iFlags = 0);
	bool AI_targetCityNative(int iFlags = 0);

	bool AI_extortCity();

	bool AI_bombardCity();
	bool AI_cityAttack(int iRange, int iOddsThreshold, bool bFollow = false);
	bool AI_anyAttack(int iRange, int iOddsThreshold, int iMinStack = 0, bool bFollow = false);
	bool AI_smartAttack(int iRange, int iLowOddsThreshold, int iHighOddsThreshold, CvPlot* pHintPlot);
	bool AI_leaveAttack(int iRange, int iThreshold, int iStrengthThreshold);
	bool AI_seaBombardRange(int iMaxRange);
	bool AI_blockade(int iRange);
	bool AI_pillage();
	bool AI_pillageRange(int iRange, bool bSafe = false);
	bool AI_maraud(int iRange);
	bool AI_hostileShuffle();
	bool AI_wanderAroundAimlessly();
	int AI_foundValue(CvPlot* pPlot);
	bool AI_found(int iMinValue = 1);
	bool AI_foundRange(int iRange, bool bFollow = false);
	bool AI_joinCityBrave();
	bool AI_joinCity(int iMaxPath = MAX_INT);
	bool AI_joinCityReplace(int iMaxPath = MAX_INT);
	bool AI_joinOptimalCity();
	bool AI_joinCityDefender();
	bool AI_yieldDestination(int iMaxPath = MAX_INT);
	bool AI_yieldNativeDestination(int iMaxPath = MAX_INT);
	bool AI_tradeWithCity();
	bool AI_settlerSeaFerry();
	bool AI_specialSeaTransportMissionary();
	bool AI_connectPlot(CvPlot* pPlot, int iRange = 0);
	bool AI_improveCity(CvCity* pCity);
	bool AI_improveLocalPlot(int iRange, CvCity* pIgnoreCity);
	bool AI_nextCityToImprove(CvCity* pCity);
	bool AI_fortTerritory(bool bCanal);
	bool AI_improvePlot(CvPlot* pPlot, BuildTypes eBuild);
	BuildTypes AI_betterPlotBuild(CvPlot* pPlot, BuildTypes eBuild);
	bool AI_routeCity();
	bool AI_routeTerritory(bool bImprovementOnly = false);
	bool AI_travelToUpgradeCity();
	bool AI_retreatToCity(bool bPrimary = false, int iMaxPath = MAX_INT);
	bool AI_treasureRetreat(int iMaxPathTurns = MAX_INT);
	bool AI_pickup(UnitAITypes eUnitAI);
	bool AI_moveToStagingCity();

	bool AI_disembark(bool bEnemyCity = true);
	bool AI_imperialSeaAssault();

	bool AI_followBombard();

	bool AI_potentialEnemy(TeamTypes eTeam, const CvPlot* pPlot = NULL);

	bool AI_defendPlot(CvPlot* pPlot);
	int AI_pillageValue(CvPlot* pPlot);
	bool AI_canPillage(CvPlot& kPlot) const;

	int AI_searchRange(int iRange);
	bool AI_plotValid(CvPlot* pPlot);

	int AI_finalOddsThreshold(CvPlot* pPlot, int iOddsThreshold);

	int AI_stackOfDoomExtra();

	bool AI_stackAttackCity(int iRange, int iPowerThreshold, bool bFollow = true);
	bool AI_moveIntoCity(int iRange);

	bool AI_groupMergeRange(UnitAITypes eUnitAI, int iRange, bool bBiggerOnly = true, bool bAllowRegrouping = false, bool bIgnoreFaster = false);

	bool AI_poach();
	bool AI_choke(int iRange = 1);

	bool AI_solveBlockageProblem(CvPlot* pDestPlot, bool bDeclareWar);

	int AI_calculatePlotWorkersNeeded(CvPlot* pPlot, BuildTypes eBuild);

	bool AI_canGroupWithAIType(UnitAITypes eUnitAI) const;
	bool AI_allowGroup(const CvUnit* pUnit, UnitAITypes eUnitAI) const;
	
	bool AI_isOnMission();
	
	bool AI_isObsoleteTradeShip();

	// added so under cheat mode we can call protected functions for testing
	friend class CvGameTextMgr;

};

#endif
