#pragma once

// playerAI.h

#ifndef CIV4_PLAYER_AI_H
#define CIV4_PLAYER_AI_H

#include "CvPlayer.h"
#include "AI_defines.h"

class CvEventTriggerInfo;

class CvPlayerAI : public CvPlayer
{

public:

	CvPlayerAI();
	virtual ~CvPlayerAI();

  // inlined for performance reasons
#ifdef _USRDLL
  static CvPlayerAI& getPlayer(PlayerTypes ePlayer) 
  {
	  FAssertMsg(ePlayer != NO_PLAYER, "Player is not assigned a valid value");
	  FAssertMsg(ePlayer < MAX_PLAYERS, "Player is not assigned a valid value");
	  return m_aPlayers[ePlayer]; 
  }
#endif
	DllExport static CvPlayerAI& getPlayerNonInl(PlayerTypes ePlayer);

	static void initStatics();
	static void freeStatics();
	DllExport static bool areStaticsInitialized();

	void AI_init();
	void AI_uninit();
	void AI_reset();

	int AI_getFlavorValue(FlavorTypes eFlavor);

	void AI_doTurnPre();
	void AI_doTurnPost();
	void AI_doTurnUnitsPre();
	void AI_doTurnUnitsPost();

	void AI_doPeace();

	void AI_updateFoundValues(bool bStartingLoc = false);
	void AI_updateAreaTargets();

	int AI_movementPriority(CvSelectionGroup* pGroup);
	void AI_unitUpdate();

	void AI_makeAssignWorkDirty();
	void AI_assignWorkingPlots();
	void AI_updateAssignWork();

	void AI_makeProductionDirty();

	void AI_conquerCity(CvCity* pCity);

	bool AI_acceptUnit(CvUnit* pUnit);
	bool AI_captureUnit(UnitTypes eUnit, CvPlot* pPlot);

	DomainTypes AI_unitAIDomainType(UnitAITypes eUnitAI);

	int AI_yieldWeight(YieldTypes eYield);
	int AI_commerceWeight(CommerceTypes eCommerce, CvCity* pCity = NULL);

	int AI_foundValue(int iX, int iY, int iMinRivalRange = -1, bool bStartingLoc = false);

	bool AI_isAreaAlone(CvArea* pArea);
	bool AI_isCapitalAreaAlone();
	bool AI_isPrimaryArea(CvArea* pArea);

	int AI_militaryWeight(CvArea* pArea);

	int AI_targetCityValue(CvCity* pCity, bool bRandomize, bool bIgnoreAttackers = false);
	CvCity* AI_findTargetCity(CvArea* pArea);

	bool AI_isCommercePlot(CvPlot* pPlot);
	int AI_getPlotDanger(CvPlot* pPlot, int iRange = -1, bool bTestMoves = true);
	int AI_getUnitDanger(CvUnit* pUnit, int iRange = -1, bool bTestMoves = true, bool bAnyDanger = true);
	int AI_getSpyDanger(CvPlot* pPlot, int iRange, bool bTestMoves = true);
	int AI_getWaterDanger(CvPlot* pPlot, int iRange, bool bTestMoves = true);

	bool AI_avoidScience();
	bool AI_isFinancialTrouble();
	int AI_goldTarget();

	TechTypes AI_bestTech(int iMaxPathLength = 1, bool bIgnoreCost = false, bool bAsync = false, TechTypes eIgnoreTech = NO_TECH, AdvisorTypes eIgnoreAdvisor = NO_ADVISOR);
	void AI_chooseFreeTech();
	void AI_chooseResearch();

	DllExport DiploCommentTypes AI_getGreeting(PlayerTypes ePlayer);
	bool AI_isWillingToTalk(PlayerTypes ePlayer);
	bool AI_demandRebukedSneak(PlayerTypes ePlayer);
	bool AI_demandRebukedWar(PlayerTypes ePlayer);
	DllExport bool AI_hasTradedWithTeam(TeamTypes eTeam);

	AttitudeTypes AI_getAttitude(PlayerTypes ePlayer, bool bForced = true);
	int AI_getAttitudeVal(PlayerTypes ePlayer, bool bForced = true);
	static AttitudeTypes AI_getAttitude(int iAttitudeVal);

	int AI_calculateStolenCityRadiusPlots(PlayerTypes ePlayer) const;
	int AI_getCloseBordersAttitude(PlayerTypes ePlayer);

	int AI_getWarAttitude(PlayerTypes ePlayer);
	int AI_getPeaceAttitude(PlayerTypes ePlayer);
	int AI_getSameReligionAttitude(PlayerTypes ePlayer);
	int AI_getDifferentReligionAttitude(PlayerTypes ePlayer);
	int AI_getBonusTradeAttitude(PlayerTypes ePlayer);
	int AI_getOpenBordersAttitude(PlayerTypes ePlayer);
	int AI_getDefensivePactAttitude(PlayerTypes ePlayer);
	int AI_getRivalDefensivePactAttitude(PlayerTypes ePlayer);
	int AI_getRivalVassalAttitude(PlayerTypes ePlayer);
	int AI_getShareWarAttitude(PlayerTypes ePlayer);
	int AI_getFavoriteCivicAttitude(PlayerTypes ePlayer);
	int AI_getTradeAttitude(PlayerTypes ePlayer);
	int AI_getRivalTradeAttitude(PlayerTypes ePlayer);
	int AI_getMemoryAttitude(PlayerTypes ePlayer, MemoryTypes eMemory);
	int AI_getColonyAttitude(PlayerTypes ePlayer);

	PlayerVoteTypes AI_diploVote(const VoteSelectionSubData& kVoteData, VoteSourceTypes eVoteSource, bool bPropose);

	int AI_dealVal(PlayerTypes ePlayer, const CLinkList<TradeData>* pList, bool bIgnoreAnnual = false, int iExtra = 1);
	bool AI_goldDeal(const CLinkList<TradeData>* pList);
	bool AI_considerOffer(PlayerTypes ePlayer, const CLinkList<TradeData>* pTheirList, const CLinkList<TradeData>* pOurList, int iChange = 1);
	bool AI_counterPropose(PlayerTypes ePlayer, const CLinkList<TradeData>* pTheirList, const CLinkList<TradeData>* pOurList, CLinkList<TradeData>* pTheirInventory, CLinkList<TradeData>* pOurInventory, CLinkList<TradeData>* pTheirCounter, CLinkList<TradeData>* pOurCounter);

	DllExport int AI_maxGoldTrade(PlayerTypes ePlayer);

	DllExport int AI_maxGoldPerTurnTrade(PlayerTypes ePlayer);
	int AI_goldPerTurnTradeVal(int iGoldPerTurn);

	int AI_bonusVal(BonusTypes eBonus, int iChange = 1);
	int AI_baseBonusVal(BonusTypes eBonus);
	int AI_bonusTradeVal(BonusTypes eBonus, PlayerTypes ePlayer, int iChange);
	DenialTypes AI_bonusTrade(BonusTypes eBonus, PlayerTypes ePlayer);
	int AI_corporationBonusVal(BonusTypes eBonus);

	int AI_cityTradeVal(CvCity* pCity);
	DenialTypes AI_cityTrade(CvCity* pCity, PlayerTypes ePlayer);

	int AI_stopTradingTradeVal(TeamTypes eTradeTeam, PlayerTypes ePlayer);
	DenialTypes AI_stopTradingTrade(TeamTypes eTradeTeam, PlayerTypes ePlayer);

	int AI_civicTradeVal(CivicTypes eCivic, PlayerTypes ePlayer);
	DenialTypes AI_civicTrade(CivicTypes eCivic, PlayerTypes ePlayer);

	int AI_religionTradeVal(ReligionTypes eReligion, PlayerTypes ePlayer);
	DenialTypes AI_religionTrade(ReligionTypes eReligion, PlayerTypes ePlayer);

	int AI_unitImpassableCount(UnitTypes eUnit);
	int AI_unitValue(UnitTypes eUnit, UnitAITypes eUnitAI, CvArea* pArea);
	int AI_totalUnitAIs(UnitAITypes eUnitAI);
	int AI_totalAreaUnitAIs(CvArea* pArea, UnitAITypes eUnitAI);
	int AI_totalWaterAreaUnitAIs(CvArea* pArea, UnitAITypes eUnitAI);
	int AI_countCargoSpace(UnitAITypes eUnitAI);

	int AI_neededExplorers(CvArea* pArea);
	int AI_neededWorkers(CvArea* pArea);
	int AI_neededMissionaries(CvArea* pArea, ReligionTypes eReligion);
	int AI_neededExecutives(CvArea* pArea, CorporationTypes eCorporation);
	
	int AI_missionaryValue(CvArea* pArea, ReligionTypes eReligion, PlayerTypes* peBestPlayer = NULL);
	int AI_executiveValue(CvArea* pArea, CorporationTypes eCorporation, PlayerTypes* peBestPlayer = NULL);
	
	int AI_corporationValue(CorporationTypes eCorporation, CvCity* pCity = NULL);
	
	int AI_adjacentPotentialAttackers(CvPlot* pPlot, bool bTestCanMove = false);
	int AI_totalMissionAIs(MissionAITypes eMissionAI, CvSelectionGroup* pSkipSelectionGroup = NULL);
	int AI_areaMissionAIs(CvArea* pArea, MissionAITypes eMissionAI, CvSelectionGroup* pSkipSelectionGroup = NULL);
	int AI_plotTargetMissionAIs(CvPlot* pPlot, MissionAITypes eMissionAI, CvSelectionGroup* pSkipSelectionGroup = NULL, int iRange = 0);
	int AI_plotTargetMissionAIs(CvPlot* pPlot, MissionAITypes eMissionAI, int& iClosestTargetRange, CvSelectionGroup* pSkipSelectionGroup = NULL, int iRange = 0);
	int AI_plotTargetMissionAIs(CvPlot* pPlot, MissionAITypes* aeMissionAI, int iMissionAICount, int& iClosestTargetRange, CvSelectionGroup* pSkipSelectionGroup = NULL, int iRange = 0);
	int AI_unitTargetMissionAIs(CvUnit* pUnit, MissionAITypes eMissionAI, CvSelectionGroup* pSkipSelectionGroup = NULL);
	int AI_unitTargetMissionAIs(CvUnit* pUnit, MissionAITypes* aeMissionAI, int iMissionAICount, CvSelectionGroup* pSkipSelectionGroup = NULL);
	int AI_enemyTargetMissionAIs(MissionAITypes eMissionAI, CvSelectionGroup* pSkipSelectionGroup = NULL);
	int AI_enemyTargetMissionAIs(MissionAITypes* aeMissionAI, int iMissionAICount, CvSelectionGroup* pSkipSelectionGroup = NULL);
	int AI_wakePlotTargetMissionAIs(CvPlot* pPlot, MissionAITypes eMissionAI, CvSelectionGroup* pSkipSelectionGroup = NULL);
	

	CivicTypes AI_bestCivic(CivicOptionTypes eCivicOption);
	int AI_civicValue(CivicTypes eCivic);

	ReligionTypes AI_bestReligion();
	int AI_religionValue(ReligionTypes eReligion);

	EspionageMissionTypes AI_bestPlotEspionage(CvPlot* pSpyPlot, PlayerTypes& eTargetPlayer, CvPlot*& pPlot, int& iData);
	int AI_espionageVal(PlayerTypes eTargetPlayer, EspionageMissionTypes eMission, CvPlot* pPlot, int iData);

	int AI_getPeaceWeight();
	void AI_setPeaceWeight(int iNewValue);

	int AI_getEspionageWeight();
	void AI_setEspionageWeight(int iNewValue);

	int AI_getAttackOddsChange();
	void AI_setAttackOddsChange(int iNewValue);

	int AI_getCivicTimer();
	void AI_setCivicTimer(int iNewValue);
	void AI_changeCivicTimer(int iChange);

	int AI_getReligionTimer();
	void AI_setReligionTimer(int iNewValue);
	void AI_changeReligionTimer(int iChange);

	int AI_getExtraGoldTarget() const;
	void AI_setExtraGoldTarget(int iNewValue);

	int AI_getNumTrainAIUnits(UnitAITypes eIndex);
	void AI_changeNumTrainAIUnits(UnitAITypes eIndex, int iChange);

	int AI_getNumAIUnits(UnitAITypes eIndex);
	void AI_changeNumAIUnits(UnitAITypes eIndex, int iChange);

	int AI_getSameReligionCounter(PlayerTypes eIndex);
	void AI_changeSameReligionCounter(PlayerTypes eIndex, int iChange);

	int AI_getDifferentReligionCounter(PlayerTypes eIndex);
	void AI_changeDifferentReligionCounter(PlayerTypes eIndex, int iChange);

	int AI_getFavoriteCivicCounter(PlayerTypes eIndex);
	void AI_changeFavoriteCivicCounter(PlayerTypes eIndex, int iChange);

	int AI_getBonusTradeCounter(PlayerTypes eIndex);
	void AI_changeBonusTradeCounter(PlayerTypes eIndex, int iChange);

	int AI_getPeacetimeTradeValue(PlayerTypes eIndex);
	void AI_changePeacetimeTradeValue(PlayerTypes eIndex, int iChange);

	int AI_getPeacetimeGrantValue(PlayerTypes eIndex);
	void AI_changePeacetimeGrantValue(PlayerTypes eIndex, int iChange);

	int AI_getGoldTradedTo(PlayerTypes eIndex);
	void AI_changeGoldTradedTo(PlayerTypes eIndex, int iChange);

	int AI_getAttitudeExtra(PlayerTypes eIndex);
	void AI_setAttitudeExtra(PlayerTypes eIndex, int iNewValue);
	void AI_changeAttitudeExtra(PlayerTypes eIndex, int iChange);

	bool AI_isFirstContact(PlayerTypes eIndex);
	void AI_setFirstContact(PlayerTypes eIndex, bool bNewValue);

	int AI_getContactTimer(PlayerTypes eIndex1, ContactTypes eIndex2);
	void AI_changeContactTimer(PlayerTypes eIndex1, ContactTypes eIndex2, int iChange);

	int AI_getMemoryCount(PlayerTypes eIndex1, MemoryTypes eIndex2);
	void AI_changeMemoryCount(PlayerTypes eIndex1, MemoryTypes eIndex2, int iChange);

	int AI_calculateGoldenAgeValue();

	void AI_doCommerce();

	EventTypes AI_chooseEvent(int iTriggeredId);
	virtual void AI_launch(VictoryTypes eVictory);

	int AI_getCultureVictoryStage();
	
	int AI_cultureVictoryTechValue(TechTypes eTech);
	
	bool AI_isDoStrategy(int iStrategy);
	void AI_forceUpdateStrategies();

	void AI_nowHasTech(TechTypes eTech);
	
    int AI_countDeadlockedBonuses(CvPlot* pPlot);
    
    int AI_getOurPlotStrength(CvPlot* pPlot, int iRange, bool bDefensiveBonuses, bool bTestMoves);
    int AI_getEnemyPlotStrength(CvPlot* pPlot, int iRange, bool bDefensiveBonuses, bool bTestMoves);

	int AI_goldToUpgradeAllUnits(int iExpThreshold = 0);

	int AI_goldTradeValuePercent();
	
	int AI_averageYieldMultiplier(YieldTypes eYield);
	int AI_averageCommerceMultiplier(CommerceTypes eCommerce);
	int AI_averageGreatPeopleMultiplier();
	int AI_averageCommerceExchange(CommerceTypes eCommerce);
	
	int AI_playerCloseness(PlayerTypes eIndex, int iMaxDistance);
	
	int AI_getTotalCityThreat();
	int AI_getTotalFloatingDefenseNeeded();
	
	
	int AI_getTotalAreaCityThreat(CvArea* pArea);
	int AI_countNumAreaHostileUnits(CvArea* pArea, bool bPlayer, bool bTeam, bool bNeutral, bool bHostile);
	int AI_getTotalFloatingDefendersNeeded(CvArea* pArea);
	int AI_getTotalFloatingDefenders(CvArea* pArea);

	RouteTypes AI_bestAdvancedStartRoute(CvPlot* pPlot, int* piYieldValue = NULL);
	UnitTypes AI_bestAdvancedStartUnitAI(CvPlot* pPlot, UnitAITypes eUnitAI);
	CvPlot* AI_advancedStartFindCapitalPlot();
	
	bool AI_advancedStartPlaceExploreUnits(bool bLand);
	void AI_advancedStartRevealRadius(CvPlot* pPlot, int iRadius);
	bool AI_advancedStartPlaceCity(CvPlot* pPlot);
	bool AI_advancedStartDoRoute(CvPlot* pFromPlot, CvPlot* pToPlot);
	void AI_advancedStartRouteTerritory();
	void AI_doAdvancedStart(bool bNoExit = false);
	
	int AI_getMinFoundValue();
	
	void AI_recalculateFoundValues(int iX, int iY, int iInnerRadius, int iOuterRadius);
	
	void AI_updateCitySites(int iMinFoundValueThreshold, int iMaxSites);
	void AI_invalidateCitySites(int iMinFoundValueThreshold);
	bool AI_isPlotCitySite(CvPlot* pPlot);
	int AI_getNumAreaCitySites(int iAreaID, int& iBestValue);
	int AI_getNumAdjacentAreaCitySites(int iWaterAreaID, int iExcludeArea, int& iBestValue);
	
	int AI_getNumCitySites();
	CvPlot* AI_getCitySite(int iIndex);
	
	int AI_bestAreaUnitAIValue(UnitAITypes eUnitAI, CvArea* pArea, UnitTypes* peBestUnitType = NULL);
	int AI_bestCityUnitAIValue(UnitAITypes eUnitAI, CvCity* pCity, UnitTypes* peBestUnitType = NULL);
	
	int AI_calculateTotalBombard(DomainTypes eDomain);
	
	int AI_getUnitClassWeight(UnitClassTypes eUnitClass);
	int AI_getUnitCombatWeight(UnitCombatTypes eUnitCombat);
	int AI_calculateUnitAIViability(UnitAITypes eUnitAI, DomainTypes eDomain);
	
	void AI_updateBonusValue();
	void AI_updateBonusValue(BonusTypes eBonus);
	
	int AI_getAttitudeWeight(PlayerTypes ePlayer);

	ReligionTypes AI_chooseReligion();
	
	int AI_getPlotAirbaseValue(CvPlot* pPlot);
	int AI_getPlotCanalValue(CvPlot* pPlot);

	// for serialization
  virtual void read(FDataStreamBase* pStream);
  virtual void write(FDataStreamBase* pStream);

protected:

	static CvPlayerAI* m_aPlayers;

	int m_iPeaceWeight;
	int m_iEspionageWeight;
	int m_iAttackOddsChange;
	int m_iCivicTimer;
	int m_iReligionTimer;
	int m_iExtraGoldTarget;
	
	int m_iStrategyHash;
	int m_iStrategyHashCacheTurn;
	
	
	int m_iAveragesCacheTurn;
	
	int m_iAverageGreatPeopleMultiplier;
	
	int *m_aiAverageYieldMultiplier;
	int *m_aiAverageCommerceMultiplier;
	int *m_aiAverageCommerceExchange;
	
	int m_iUpgradeUnitsCacheTurn;
	int m_iUpgradeUnitsCachedExpThreshold;
	int m_iUpgradeUnitsCachedGold;
	
	
	int *m_aiNumTrainAIUnits;
	int *m_aiNumAIUnits;
	int* m_aiSameReligionCounter;
	int* m_aiDifferentReligionCounter;
	int* m_aiFavoriteCivicCounter;
	int* m_aiBonusTradeCounter;
	int* m_aiPeacetimeTradeValue;
	int* m_aiPeacetimeGrantValue;
	int* m_aiGoldTradedTo;
	int* m_aiAttitudeExtra;
	int* m_aiBonusValue;
	int* m_aiUnitClassWeights;
	int* m_aiUnitCombatWeights;

	mutable int* m_aiCloseBordersAttitudeCache;


	bool* m_abFirstContact;

	int** m_aaiContactTimer;
	int** m_aaiMemoryCount;
	
	std::vector<int> m_aiAICitySites;
	
	bool m_bWasFinancialTrouble;
	int m_iTurnLastProductionDirty;

	void AI_doCounter();
	void AI_doMilitary();
	void AI_doResearch();
	void AI_doCivics();
	void AI_doReligion();
	void AI_doDiplo();
	void AI_doSplit();
	void AI_doCheckFinancialTrouble();
	
	bool AI_disbandUnit(int iExpThreshold, bool bObsolete);
	
	int AI_getStrategyHash();
	void AI_calculateAverages();
	
	int AI_getHappinessWeight(int iHappy, int iExtraPop);
	int AI_getHealthWeight(int iHealth, int iExtraPop);
	
	void AI_convertUnitAITypesForCrush();
	int AI_eventValue(EventTypes eEvent, const EventTriggeredData& kTriggeredData);
		
	void AI_doEnemyUnitData();
	void AI_invalidateCloseBordersAttitudeCache();
	
	friend class CvGameTextMgr;
};

// helper for accessing static functions
#ifdef _USRDLL
#define GET_PLAYER CvPlayerAI::getPlayer
#else
#define GET_PLAYER CvPlayerAI::getPlayerNonInl
#endif

#endif
