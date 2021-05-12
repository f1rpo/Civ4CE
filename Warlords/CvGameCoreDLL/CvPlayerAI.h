// playerAI.h

#ifndef CIV4_PLAYER_AI_H
#define CIV4_PLAYER_AI_H

#include "CvPlayer.h"

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

	int AI_targetCityValue(CvCity* pCity, bool bRandomize);
	CvCity* AI_findTargetCity(CvArea* pArea);

	bool AI_isCommercePlot(CvPlot* pPlot);
	int AI_getPlotDanger(CvPlot* pPlot, int iRange = -1, bool bTestMoves = true);

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

	AttitudeTypes AI_getAttitude(PlayerTypes ePlayer);
	int AI_getAttitudeVal(PlayerTypes ePlayer);

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
	int AI_getShareWarAttitude(PlayerTypes ePlayer);
	int AI_getFavoriteCivicAttitude(PlayerTypes ePlayer);
	int AI_getTradeAttitude(PlayerTypes ePlayer);
	int AI_getRivalTradeAttitude(PlayerTypes ePlayer);
	int AI_getMemoryAttitude(PlayerTypes ePlayer, MemoryTypes eMemory);

	int AI_diploVote(VoteTypes eVote);

	int AI_dealVal(PlayerTypes ePlayer, CLinkList<TradeData>* pList, bool bIgnoreAnnual = false);
	bool AI_goldDeal(CLinkList<TradeData>* pList);
	bool AI_considerOffer(PlayerTypes ePlayer, CLinkList<TradeData>* pTheirList, CLinkList<TradeData>* pOurList);
	bool AI_counterPropose(PlayerTypes ePlayer, CLinkList<TradeData>* pTheirList, CLinkList<TradeData>* pOurList, CLinkList<TradeData>* pTheirInventory, CLinkList<TradeData>* pOurInventory, CLinkList<TradeData>* pTheirCounter, CLinkList<TradeData>* pOurCounter);

	DllExport int AI_maxGoldTrade(PlayerTypes ePlayer);

	DllExport int AI_maxGoldPerTurnTrade(PlayerTypes ePlayer);
	int AI_goldPerTurnTradeVal(int iGoldPerTurn);

	int AI_bonusVal(BonusTypes eBonus);
	int AI_bonusTradeVal(BonusTypes eBonus, PlayerTypes ePlayer);
	DenialTypes AI_bonusTrade(BonusTypes eBonus, PlayerTypes ePlayer);

	int AI_cityTradeVal(CvCity* pCity);
	DenialTypes AI_cityTrade(CvCity* pCity, PlayerTypes ePlayer);

	int AI_stopTradingTradeVal(TeamTypes eTradeTeam, PlayerTypes ePlayer);
	DenialTypes AI_stopTradingTrade(TeamTypes eTradeTeam, PlayerTypes ePlayer);

	int AI_civicTradeVal(CivicTypes eCivic, PlayerTypes ePlayer);
	DenialTypes AI_civicTrade(CivicTypes eCivic, PlayerTypes ePlayer);

	int AI_religionTradeVal(ReligionTypes eReligion, PlayerTypes ePlayer);
	DenialTypes AI_religionTrade(ReligionTypes eReligion, PlayerTypes ePlayer);

	bool AI_unitImpassable(UnitTypes eUnit);
	int AI_unitValue(UnitTypes eUnit, UnitAITypes eUnitAI, CvArea* pArea);
	int AI_totalUnitAIs(UnitAITypes eUnitAI);
	int AI_totalAreaUnitAIs(CvArea* pArea, UnitAITypes eUnitAI);
	int AI_totalWaterAreaUnitAIs(CvArea* pArea, UnitAITypes eUnitAI);
	int AI_countCargoSpace(UnitAITypes eUnitAI);

	int AI_neededExplorers(CvArea* pArea);
	int AI_neededWorkers(CvArea* pArea);
	int AI_neededMissionaries(CvArea* pArea, ReligionTypes eReligion);

	int AI_adjacentPotentialAttackers(CvPlot* pPlot, bool bTestCanMove = false);
	int AI_totalMissionAIs(MissionAITypes eMissionAI, CvSelectionGroup* pSkipSelectionGroup = NULL);
	int AI_areaMissionAIs(CvArea* pArea, MissionAITypes eMissionAI, CvSelectionGroup* pSkipSelectionGroup = NULL);
	int AI_plotTargetMissionAIs(CvPlot* pPlot, MissionAITypes eMissionAI, CvSelectionGroup* pSkipSelectionGroup = NULL, int iRange = 0);
	int AI_unitTargetMissionAIs(CvUnit* pUnit, MissionAITypes eMissionAI, CvSelectionGroup* pSkipSelectionGroup = NULL);

	CivicTypes AI_bestCivic(CivicOptionTypes eCivicOption);
	int AI_civicValue(CivicTypes eCivic);

	ReligionTypes AI_bestReligion();
	int AI_religionValue(ReligionTypes eReligion);

	int AI_getPeaceWeight();
	void AI_setPeaceWeight(int iNewValue);

	int AI_getAttackOddsChange();
	void AI_setAttackOddsChange(int iNewValue);

	int AI_getCivicTimer();
	void AI_setCivicTimer(int iNewValue);
	void AI_changeCivicTimer(int iChange);

	int AI_getReligionTimer();
	void AI_setReligionTimer(int iNewValue);
	void AI_changeReligionTimer(int iChange);

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

  // for serialization
  virtual void read(FDataStreamBase* pStream);
  virtual void write(FDataStreamBase* pStream);

protected:

	static CvPlayerAI* m_aPlayers;

	int m_iPeaceWeight;
	int m_iAttackOddsChange;
	int m_iCivicTimer;
	int m_iReligionTimer;

	int m_aiNumTrainAIUnits[NUM_UNITAI_TYPES];
	int m_aiNumAIUnits[NUM_UNITAI_TYPES];
	int m_aiSameReligionCounter[MAX_PLAYERS];
	int m_aiDifferentReligionCounter[MAX_PLAYERS];
	int m_aiFavoriteCivicCounter[MAX_PLAYERS];
	int m_aiBonusTradeCounter[MAX_PLAYERS];
	int m_aiPeacetimeTradeValue[MAX_PLAYERS];
	int m_aiPeacetimeGrantValue[MAX_PLAYERS];
	int m_aiGoldTradedTo[MAX_PLAYERS];
	int m_aiAttitudeExtra[MAX_PLAYERS];

	bool m_abFirstContact[MAX_PLAYERS];

	int m_aaiContactTimer[MAX_PLAYERS][NUM_CONTACT_TYPES];
	int m_aaiMemoryCount[MAX_PLAYERS][NUM_MEMORY_TYPES];

	void AI_doCounter();
	void AI_doMilitary();
	void AI_doResearch();
	void AI_doCommerce();
	void AI_doCivics();
	void AI_doReligion();
	void AI_doDiplo();

};

// helper for accessing static functions
#ifdef _USRDLL
#define GET_PLAYER CvPlayerAI::getPlayer
#else
#define GET_PLAYER CvPlayerAI::getPlayerNonInl
#endif

#endif
