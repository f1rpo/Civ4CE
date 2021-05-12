// teamAI.h

#ifndef CIV4_TEAM_AI_H
#define CIV4_TEAM_AI_H

#include "CvTeam.h"

class CvTeamAI : public CvTeam
{

public:

	CvTeamAI();
	virtual ~CvTeamAI();

#ifdef _USRDLL
	// inlined for performance reasons, only in the dll
	static CvTeamAI& getTeam(TeamTypes eTeam)
	{
		FAssertMsg(eTeam != NO_TEAM, "eTeam is not assigned a valid value");
		FAssertMsg(eTeam < MAX_TEAMS, "eTeam is not assigned a valid value");
		return m_aTeams[eTeam]; 
	}
#endif
	DllExport static CvTeamAI& getTeamNonInl(TeamTypes eTeam);

	static void initStatics();
	static void freeStatics();

	void AI_init();
	void AI_uninit();
	void AI_reset();

	void AI_doTurnPre();
	void AI_doTurnPost();

	void AI_makeAssignWorkDirty();

	void AI_updateAreaStragies(bool bTargets = true);
	void AI_updateAreaTargets();

	int AI_countFinancialTrouble();
	int AI_countMilitaryWeight(CvArea* pArea);

	bool AI_isAnyCapitalAreaAlone();
	bool AI_isPrimaryArea(CvArea* pArea);
	bool AI_hasCitiesInPrimaryArea(TeamTypes eTeam);
	AreaAITypes AI_calculateAreaAIType(CvArea* pArea, bool bPreparingTotal = false);

	int AI_calculateAdjacentLandPlots(TeamTypes eTeam) const;
	int AI_calculateCapitalProximity(TeamTypes eTeam) const;

	bool AI_isWarPossible();
	bool AI_isLandTarget(TeamTypes eTeam);
	bool AI_shareWar(TeamTypes eTeam);

	AttitudeTypes AI_getAttitude(TeamTypes eTeam);
	int AI_getAttitudeVal(TeamTypes eTeam);
	int AI_getMemoryCount(TeamTypes eTeam, MemoryTypes eMemory);

	VoteTypes AI_chooseElection();

	int AI_startWarVal(TeamTypes eTeam);
	int AI_endWarVal(TeamTypes eTeam);

	int AI_techTradeVal(TechTypes eTech);
	DenialTypes AI_techTrade(TechTypes eTech, TeamTypes eTeam);

	int AI_mapTradeVal(TeamTypes eTeam);
	DenialTypes AI_mapTrade(TeamTypes eTeam);

	int AI_makePeaceTradeVal(TeamTypes ePeaceTeam, TeamTypes eTeam);
	DenialTypes AI_makePeaceTrade(TeamTypes ePeaceTeam, TeamTypes eTeam);

	int AI_declareWarTradeVal(TeamTypes eWarTeam, TeamTypes eTeam);
	DenialTypes AI_declareWarTrade(TeamTypes eWarTeam, TeamTypes eTeam);

	int AI_openBordersTradeVal(TeamTypes eTeam);
	DenialTypes AI_openBordersTrade(TeamTypes eTeam);

	int AI_defensivePactTradeVal(TeamTypes eTeam);
	DenialTypes AI_defensivePactTrade(TeamTypes eTeam);

	DenialTypes AI_permanentAllianceTrade(TeamTypes eTeam);

	TeamTypes AI_getWorstEnemy();
	void AI_updateWorstEnemy();

	int AI_getWarPlanStateCounter(TeamTypes eIndex);
	void AI_setWarPlanStateCounter(TeamTypes eIndex, int iNewValue);
	void AI_changeWarPlanStateCounter(TeamTypes eIndex, int iChange);

	int AI_getAtWarCounter(TeamTypes eIndex);
	void AI_setAtWarCounter(TeamTypes eIndex, int iNewValue);
	void AI_changeAtWarCounter(TeamTypes eIndex, int iChange);

	int AI_getAtPeaceCounter(TeamTypes eIndex);
	void AI_setAtPeaceCounter(TeamTypes eIndex, int iNewValue);
	void AI_changeAtPeaceCounter(TeamTypes eIndex, int iChange);

	int AI_getHasMetCounter(TeamTypes eIndex);
	void AI_setHasMetCounter(TeamTypes eIndex, int iNewValue);
	void AI_changeHasMetCounter(TeamTypes eIndex, int iChange);

	int AI_getOpenBordersCounter(TeamTypes eIndex);
	void AI_setOpenBordersCounter(TeamTypes eIndex, int iNewValue);
	void AI_changeOpenBordersCounter(TeamTypes eIndex, int iChange);

	int AI_getDefensivePactCounter(TeamTypes eIndex);
	void AI_setDefensivePactCounter(TeamTypes eIndex, int iNewValue);
	void AI_changeDefensivePactCounter(TeamTypes eIndex, int iChange);

	int AI_getShareWarCounter(TeamTypes eIndex);
	void AI_setShareWarCounter(TeamTypes eIndex, int iNewValue);
	void AI_changeShareWarCounter(TeamTypes eIndex, int iChange);

	int AI_getWarSuccess(TeamTypes eIndex);
	void AI_setWarSuccess(TeamTypes eIndex, int iNewValue);
	void AI_changeWarSuccess(TeamTypes eIndex, int iChange);

	int AI_getEnemyPeacetimeTradeValue(TeamTypes eIndex);
	void AI_setEnemyPeacetimeTradeValue(TeamTypes eIndex, int iNewValue);
	void AI_changeEnemyPeacetimeTradeValue(TeamTypes eIndex, int iChange);

	int AI_getEnemyPeacetimeGrantValue(TeamTypes eIndex);
	void AI_setEnemyPeacetimeGrantValue(TeamTypes eIndex, int iNewValue);
	void AI_changeEnemyPeacetimeGrantValue(TeamTypes eIndex, int iChange);

	WarPlanTypes AI_getWarPlan(TeamTypes eIndex);
	bool AI_isChosenWar(TeamTypes eIndex);
	bool AI_isSneakAttackPreparing(TeamTypes eIndex);
	bool AI_isSneakAttackReady(TeamTypes eIndex);
	void AI_setWarPlan(TeamTypes eIndex, WarPlanTypes eNewValue);

	virtual void read(FDataStreamBase* pStream);
	virtual void write(FDataStreamBase* pStream);

protected:

	static CvTeamAI* m_aTeams;

	TeamTypes m_eWorstEnemy;

	int m_aiWarPlanStateCounter[MAX_TEAMS];
	int m_aiAtWarCounter[MAX_TEAMS];
	int m_aiAtPeaceCounter[MAX_TEAMS];
	int m_aiHasMetCounter[MAX_TEAMS];
	int m_aiOpenBordersCounter[MAX_TEAMS];
	int m_aiDefensivePactCounter[MAX_TEAMS];
	int m_aiShareWarCounter[MAX_TEAMS];
	int m_aiWarSuccess[MAX_TEAMS];
	int m_aiEnemyPeacetimeTradeValue[MAX_TEAMS];
	int m_aiEnemyPeacetimeGrantValue[MAX_TEAMS];

	WarPlanTypes m_aeWarPlan[MAX_TEAMS];

	int AI_noTechTradeThreshold();
	int AI_techTradeKnownPercent();
	int AI_maxWarRand();
	int AI_maxWarNearbyPowerRatio();
	int AI_maxWarDistantPowerRatio();
	int AI_maxWarMinAdjacentLandPercent();
	int AI_limitedWarRand();
	int AI_limitedWarPowerRatio();
	int AI_dogpileWarRand();
	int AI_makePeaceRand();
	int AI_noWarAttitudeProb(AttitudeTypes eAttitude);

	void AI_doCounter();
	void AI_doWar();

};

// helper for accessing static functions
#ifdef _USRDLL
#define GET_TEAM CvTeamAI::getTeam
#else
#define GET_TEAM CvTeamAI::getTeamNonInl
#endif

#endif
