// unitAI.h

#ifndef CIV4_UNIT_AI_H
#define CIV4_UNIT_AI_H

#include "CvUnit.h"
#include "CvPlotRegion.h"

class CvCity;

class CvUnitAI : public CvUnit
{

public:

  CvUnitAI();
  virtual ~CvUnitAI();

  void AI_init(UnitAITypes eUnitAI);
	void AI_uninit();
	void AI_reset(UnitAITypes eUnitAI = NO_UNITAI);

	bool AI_update();
	bool AI_follow();

	void AI_upgrade();

	void AI_promote();

	int AI_groupFirstVal();
	int AI_groupSecondVal();

	int AI_attackOdds(const CvPlot* pPlot, bool bPotentialEnemy) const;

	bool AI_bestCityBuild(CvCity* pCity, CvPlot** ppBestPlot = NULL, BuildTypes* peBestBuild = NULL, CvPlot* pIgnorePlot = NULL);

	bool AI_isCityAIType() const;

	int AI_getBirthmark() const;
	void AI_setBirthmark(int iNewValue);

	UnitAITypes AI_getUnitAIType() const;
	void AI_setUnitAIType(UnitAITypes eNewValue);

	int AI_sacrificeValue(const CvPlot* pPlot) const;
	
	CvPlot* AI_getBestExplorePlot(int iRange, CvPlot** pThisTurnPlot = NULL, bool bNoRandom = false);
	CvPlot* AI_getBestGoodyPlot(CvPlot* pStartPlot, int iRange, CvPlot** pThisTurnPlot = NULL, bool bNoRandom = false);

	void read(FDataStreamBase* pStream);
	void write(FDataStreamBase* pStream);

protected:

	int m_iBirthmark;

	UnitAITypes m_eUnitAIType;

	int m_iAutomatedAbortTurn;

	void AI_animalMove();
	void AI_settleMove();
	void AI_workerMove();
	void AI_barbAttackMove();
	void AI_attackMove();
	void AI_attackCityMove();
	void AI_collateralMove();
	void AI_pillageMove();
	void AI_reserveMove();
	void AI_counterMove();
	void AI_cityDefenseMove();
	void AI_cityDefenseExtraMove();
	void AI_exploreMove();
	void AI_missionaryMove();
	void AI_prophetMove();
	void AI_artistMove();
	void AI_scientistMove();
	void AI_generalMove();
	void AI_merchantMove();
	void AI_engineerMove();
	void AI_spyMove();
	void AI_ICBMMove();
	void AI_workerSeaMove();
	void AI_barbAttackSeaMove();
	void AI_attackSeaMove();
	void AI_reserveSeaMove();
	void AI_escortSeaMove();
	void AI_exploreSeaMove();
	void AI_assaultSeaMove();
	void AI_settlerSeaMove();
	void AI_missionarySeaMove();
	void AI_spySeaMove();
	void AI_carrierSeaMove();
	void AI_attackAirMove();
	void AI_defenseAirMove();
	void AI_carrierAirMove();
	void AI_exploreAirMove();

	void AI_networkAutomated();
	void AI_cityAutomated();

	int AI_promotionValue(PromotionTypes ePromotion);

	bool AI_shadow(UnitAITypes eUnitAI);
	bool AI_group(UnitAITypes eUnitAI, int iMaxGroup = -1, int iMaxOwnUnitAI = -1, int iMinUnitAI = -1, bool bIgnoreFaster = false, bool bIgnoreOwnUnitType = false, bool bStackOfDoom = false, int iMaxPath = MAX_INT, bool bAllowRegrouping = false);
	bool AI_load(UnitAITypes eUnitAI, MissionAITypes eMissionAI, UnitAITypes eTransportedUnitAI = NO_UNITAI, int iMinCargo = -1, int iMinCargoSpace = -1, int iMaxCargoSpace = -1, int iMaxCargoOurUnitAI = -1, int iFlags = 0, int iMaxPath = MAX_INT);
	bool AI_guardCityBestDefender();
	bool AI_guardCity(bool bLeave = false, bool bSearch = false, int iMaxPath = MAX_INT);
	bool AI_guardCityAirlift();
	bool AI_guardBonus(int iMinValue = 0);
	bool AI_guardSpy();
	bool AI_destroySpy();
	bool AI_sabotageSpy();
	bool AI_pickupTargetSpy();
	bool AI_chokeDefend();
	bool AI_heal(int iDamagePercent = 0, int iMaxPath = MAX_INT);
	bool AI_afterAttack();
	bool AI_goldenAge();
	bool AI_spreadReligion();
	bool AI_discover(bool bThisTurnOnly = false, bool bFirstResearchOnly = false);
	bool AI_lead(std::vector<UnitAITypes>& aeAIUnitTypes);
	bool AI_join(int iMaxCount = MAX_INT);
	bool AI_construct(int iMaxCount = MAX_INT);
	bool AI_switchHurry();
	bool AI_hurry();
	bool AI_greatWork();
	bool AI_offensiveAirlift();
	bool AI_protect(int iOddsThreshold);
	int AI_fogbustPlotValue(const CvPlot* pPlot, int iNumObservers) const;
	int AI_fogbustRegionValue(const CvPlotRegion& plotRegion, bool* pbAdjacentOurTerritory = NULL, const CvPlotRegion* pVisibleRegion = NULL) const;
	int AI_fogbustDataRegionValue(const CvPlotDataRegion& plotRegion, bool bRegionIsVisible = true, bool* pbAdjacentOurTerritory = NULL, const CvPlotDataRegion* pVisibleRegion = NULL) const;
	bool AI_fogbustPlotDifference(const CvPlot* pNewPlot, const CvPlotDataRegion& visiblePlotRegion, int& iFogbustingLostValue, int& iFogbustingGainedValue) const;
	bool AI_fogbust();
	bool AI_airFogbust();
	bool AI_patrol();
	bool AI_defend();
	bool AI_safety();
	bool AI_hide();
	bool AI_goody(int iRange);
	int AI_explorePlotValue(const CvPlot* pPlot, bool bAddRandom = false, bool bIgnoreGoodies = false) const;
	int AI_exploreTurnPathValue(void) const;
	bool AI_explore();
	bool AI_exploreRange(int iRange);
	bool AI_airExplore();
	bool AI_targetCity();
	bool AI_targetBarbCity();
	bool AI_bombardCity();
	bool AI_cityAttack(int iRange, int iOddsThreshold, bool bFollow = false);
	bool AI_anyAttack(int iRange, int iOddsThreshold, int iMinStack = 0, bool bFollow = false);
	bool AI_leaveAttack(int iRange, int iThreshold, int iStrengthThreshold);
	bool AI_blockade();
	bool AI_seaBombardRange(int iMaxRange);
	bool AI_pillage(int iBonusValueThreshold = 0);
	bool AI_pillageRange(int iRange, int iBonusValueThreshold = 0);
	bool AI_found();
	bool AI_foundRange(int iRange, bool bFollow = false);
	bool AI_assaultSeaTransport();
	bool AI_settlerSeaTransport();
	bool AI_specialSeaTransportMissionary();
	bool AI_specialSeaTransportSpy();
	bool AI_carrierSeaTransport();
	bool AI_connectPlot(CvPlot* pPlot, int iRange = 0);
	bool AI_improveCity(CvCity* pCity);
	bool AI_nextCityToImprove(CvCity* pCity);
	bool AI_nextCityToImproveAirlift();
	bool AI_irrigateTerritory();
	bool AI_improveBonus(int iMinValue = 0);
	bool AI_connectBonus(bool bTestTrade = true);
	bool AI_connectCity();
	bool AI_routeCity();
	bool AI_routeTerritory(bool bImprovementOnly = false);
	bool AI_travelToUpgradeCity();
	bool AI_retreatToCity(bool bPrimary = false, bool bAirlift = false, int iMaxPath = MAX_INT);
	bool AI_pickup(UnitAITypes eUnitAI);
	bool AI_airOffensiveCity();
	bool AI_airDefensiveCity();
	bool AI_airCarrier();
	bool AI_airStrike();
	bool AI_airBomb();
	bool AI_nuke();
	bool AI_trade(int iValueThreshold);
	bool AI_moveToStagingCity();

	bool AI_followBombard();

	bool AI_potentialEnemy(TeamTypes eTeam, const CvPlot* pPlot = NULL);

	bool AI_defendPlot(CvPlot* pPlot);
	int AI_pillageValue(CvPlot* pPlot, int iBonusValueThreshold = 0);
	int AI_nukeValue(CvCity* pCity);

	int AI_searchRange(int iRange);
	bool AI_plotValid(CvPlot* pPlot);

	int AI_finalOddsThreshold(CvPlot* pPlot, int iOddsThreshold);

	int AI_stackOfDoomExtra();

    bool AI_stackAttackCity(int iRange, int iPowerThreshold, bool bFollow = true);
    bool AI_moveIntoCity(int iRange);
    bool AI_groupMergeRange(UnitAITypes eUnitAI, int iRange, bool bBiggerOnly = true, bool bAllowRegrouping = false);
    
    bool AI_artistCultureVictoryMove();

	// added so under cheat mode we can call protected functions for testing
	friend class CvGameTextMgr;
#ifdef FASSERT_ENABLE
	friend class CvGame;
#endif
};

#endif
