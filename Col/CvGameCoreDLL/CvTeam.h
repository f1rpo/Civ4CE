#pragma once

// team.h

#ifndef CIV4_TEAM_H
#define CIV4_TEAM_H

//#include "CvEnums.h"

class CvArea;

class CvTeam
{

public:
	CvTeam();
	virtual ~CvTeam();

	DllExport void init(TeamTypes eID);
	DllExport void reset(TeamTypes eID = (TeamTypes)0, bool bConstructorCall = false);

protected:

	void uninit();

public:

	void addTeam(TeamTypes eTeam);
	void shareItems(TeamTypes eTeam);
	void shareCounters(TeamTypes eTeam);
	void processBuilding(BuildingTypes eBuilding, int iChange);

	void doTurn();

	void updateYield();
	void updatePowerHealth();
	void updateCommerce();

	bool canChangeWarPeace(TeamTypes eTeam, bool bAllowVassal = false) const;
	DllExport bool canDeclareWar(TeamTypes eTeam) const;
	DllExport void declareWar(TeamTypes eTeam, bool bNewDiplo, WarPlanTypes eWarPlan);
	DllExport void makePeace(TeamTypes eTeam, bool bBumpUnits = true);
	bool canContact(TeamTypes eTeam) const;
	void meet(TeamTypes eTeam, bool bNewDiplo);
	void signOpenBorders(TeamTypes eTeam);
	void signDefensivePact(TeamTypes eTeam);
	bool canSignDefensivePact(TeamTypes eTeam);

	int getAssets() const;
	int getPower(bool bIncludeVassals) const;
	int getDefensivePower() const;
	int getEnemyPower() const;
	int getNumNukeUnits() const;
	int getVotes(VoteTypes eVote, VoteSourceTypes eVoteSource) const;
	bool isVotingMember(VoteSourceTypes eVoteSource) const;
	bool isFullMember(VoteSourceTypes eVoteSource) const;

	int getAtWarCount(bool bIgnoreMinors) const;
	int getWarPlanCount(WarPlanTypes eWarPlan, bool bIgnoreMinors) const;
	int getAnyWarPlanCount(bool bIgnoreMinors) const;
	int getChosenWarCount(bool bIgnoreMinors) const;
	int getHasMetCivCount(bool bIgnoreMinors) const;
	bool hasMetHuman() const;
	int getDefensivePactCount(TeamTypes eTeam = NO_TEAM) const;
	int getVassalCount(TeamTypes eTeam = NO_TEAM) const;
	DllExport bool isAVassal() const;
	bool canVassalRevolt(TeamTypes eMaster) const;

	int getUnitClassMaking(UnitClassTypes eUnitClass) const;
	int getUnitClassCountPlusMaking(UnitClassTypes eIndex) const;
	int getBuildingClassMaking(BuildingClassTypes eBuildingClass) const;
	int getBuildingClassCountPlusMaking(BuildingClassTypes eIndex) const;
	int getHasReligionCount(ReligionTypes eReligion) const;
	int getHasCorporationCount(CorporationTypes eCorporation) const;

	int countTotalCulture();

	int countNumUnitsByArea(CvArea* pArea) const;
	int countNumCitiesByArea(CvArea* pArea) const;
	int countTotalPopulationByArea(CvArea* pArea) const;
	int countPowerByArea(CvArea* pArea) const;
	int countEnemyPowerByArea(CvArea* pArea) const;
	int countNumAIUnitsByArea(CvArea* pArea, UnitAITypes eUnitAI) const;
	int countEnemyDangerByArea(CvArea* pArea) const;

	DllExport int getResearchCost(TechTypes eTech) const;
	DllExport int getResearchLeft(TechTypes eTech) const;

	bool hasHolyCity(ReligionTypes eReligion) const;
	bool hasHeadquarters(CorporationTypes eCorporation) const;
	bool hasBonus(BonusTypes eBonus) const;

	bool isHuman() const;
	bool isBarbarian() const;
	bool isMinorCiv() const;
	PlayerTypes getLeaderID() const;
	PlayerTypes getSecretaryID() const;
	HandicapTypes getHandicapType() const;
	DllExport CvWString getName() const;

	DllExport int getNumMembers() const;
	DllExport void changeNumMembers(int iChange);

	DllExport int getAliveCount() const;
	DllExport int isAlive() const;
	void changeAliveCount(int iChange);

	int getEverAliveCount() const;
	int isEverAlive() const;
	void changeEverAliveCount(int iChange);

	int getNumCities() const;
	void changeNumCities(int iChange);

	int getTotalPopulation(bool bCheckVassals = true) const;
	void changeTotalPopulation(int iChange);

	int getTotalLand(bool bCheckVassals = true) const;
	void changeTotalLand(int iChange);

	int getNukeInterception() const;
	void changeNukeInterception(int iChange);

	int getForceTeamVoteEligibilityCount(VoteSourceTypes eVoteSource) const;
	bool isForceTeamVoteEligible(VoteSourceTypes eVoteSource) const;
	void changeForceTeamVoteEligibilityCount(VoteSourceTypes eVoteSource, int iChange);

	int getExtraWaterSeeFromCount() const;
	bool isExtraWaterSeeFrom() const;
	void changeExtraWaterSeeFromCount(int iChange);

	int getMapTradingCount() const;
	bool isMapTrading() const;
	void changeMapTradingCount(int iChange);

	int getTechTradingCount() const;
	bool isTechTrading() const;
	void changeTechTradingCount(int iChange);

	int getGoldTradingCount() const;
	bool isGoldTrading() const;
	void changeGoldTradingCount(int iChange);

	int getOpenBordersTradingCount() const;
	DllExport bool isOpenBordersTrading() const;
	void changeOpenBordersTradingCount(int iChange);

	int getDefensivePactTradingCount() const;
	bool isDefensivePactTrading() const;
	void changeDefensivePactTradingCount(int iChange);

	int getPermanentAllianceTradingCount() const;
	bool isPermanentAllianceTrading() const;
	void changePermanentAllianceTradingCount(int iChange);

	int getVassalTradingCount() const;
	bool isVassalStateTrading() const;
	void changeVassalTradingCount(int iChange);

	int getBridgeBuildingCount() const;
	DllExport bool isBridgeBuilding() const;
	void changeBridgeBuildingCount(int iChange);

	int getIrrigationCount() const;
	DllExport bool isIrrigation() const;
	void changeIrrigationCount(int iChange);

	int getIgnoreIrrigationCount() const;
	DllExport bool isIgnoreIrrigation() const;
	void changeIgnoreIrrigationCount(int iChange);

	int getWaterWorkCount() const;
	DllExport bool isWaterWork() const;
	void changeWaterWorkCount(int iChange);

	int getVassalPower() const;
	void setVassalPower(int iPower);
	int getMasterPower() const;
	void setMasterPower(int iPower);

	int getEnemyWarWearinessModifier() const;
	void changeEnemyWarWearinessModifier(int iChange);
	void changeWarWeariness(TeamTypes eOtherTeam, const CvPlot& kPlot, int iFactor);

	DllExport bool isMapCentering() const;
	void setMapCentering(bool bNewValue);

	TeamTypes getID() const;

	int getStolenVisibilityTimer(TeamTypes eIndex) const;
	bool isStolenVisibility(TeamTypes eIndex) const;
	void setStolenVisibilityTimer(TeamTypes eIndex, int iNewValue);
	void changeStolenVisibilityTimer(TeamTypes eIndex, int iChange);

	int getWarWeariness(TeamTypes eIndex) const;
	void setWarWeariness(TeamTypes eIndex, int iNewValue);
	void changeWarWeariness(TeamTypes eIndex, int iChange);

	int getTechShareCount(int iIndex) const;
	bool isTechShare(int iIndex) const;
	void changeTechShareCount(int iIndex, int iChange);

	int getCommerceFlexibleCount(CommerceTypes eIndex) const;
	bool isCommerceFlexible(CommerceTypes eIndex) const;
	void changeCommerceFlexibleCount(CommerceTypes eIndex, int iChange);

	int getExtraMoves(DomainTypes eIndex) const;
	DllExport void changeExtraMoves(DomainTypes eIndex, int iChange);

	DllExport bool isHasMet(TeamTypes eIndex) const;
	void makeHasMet(TeamTypes eIndex, bool bNewDiplo);

	DllExport bool isAtWar(TeamTypes eIndex) const;
	DllExport void setAtWar(TeamTypes eIndex, bool bNewValue);

	bool isPermanentWarPeace(TeamTypes eIndex) const;
	void setPermanentWarPeace(TeamTypes eIndex, bool bNewValue);

	DllExport bool isFreeTrade(TeamTypes eIndex) const;
	DllExport bool isOpenBorders(TeamTypes eIndex) const;
	void setOpenBorders(TeamTypes eIndex, bool bNewValue);

	DllExport bool isDefensivePact(TeamTypes eIndex) const;
	void setDefensivePact(TeamTypes eIndex, bool bNewValue);

	DllExport bool isForcePeace(TeamTypes eIndex) const;
	void setForcePeace(TeamTypes eIndex, bool bNewValue);

	DllExport bool isVassal(TeamTypes eIndex) const;
	DllExport void setVassal(TeamTypes eIndex, bool bNewValue, bool bCapitulated);

	void assignVassal(TeamTypes eVassal, bool bSurrender) const;
	void freeVassal(TeamTypes eVassal) const;

	bool isCapitulated() const;

	int getRouteChange(RouteTypes eIndex) const;
	void changeRouteChange(RouteTypes eIndex, int iChange);

	DllExport int getProjectCount(ProjectTypes eIndex) const;
	DllExport int getProjectDefaultArtType(ProjectTypes eIndex) const;
	DllExport void setProjectDefaultArtType(ProjectTypes eIndex, int value);
	DllExport int getProjectArtType(ProjectTypes eIndex, int number) const;
	DllExport void setProjectArtType(ProjectTypes eIndex, int number, int value);
	DllExport bool isProjectMaxedOut(ProjectTypes eIndex, int iExtra = 0) const;
	DllExport bool isProjectAndArtMaxedOut(ProjectTypes eIndex) const;
	DllExport void changeProjectCount(ProjectTypes eIndex, int iChange);
	DllExport void finalizeProjectArtTypes();

	int getProjectMaking(ProjectTypes eIndex) const;
	void changeProjectMaking(ProjectTypes eIndex, int iChange);

	int getUnitClassCount(UnitClassTypes eIndex) const;
	bool isUnitClassMaxedOut(UnitClassTypes eIndex, int iExtra = 0) const;
	void changeUnitClassCount(UnitClassTypes eIndex, int iChange);

	int getBuildingClassCount(BuildingClassTypes eIndex) const;
	bool isBuildingClassMaxedOut(BuildingClassTypes eIndex, int iExtra = 0) const;
	void changeBuildingClassCount(BuildingClassTypes eIndex, int iChange);

	int getObsoleteBuildingCount(BuildingTypes eIndex) const;
	bool isObsoleteBuilding(BuildingTypes eIndex) const;
	void changeObsoleteBuildingCount(BuildingTypes eIndex, int iChange);

	DllExport int getResearchProgress(TechTypes eIndex) const;
	void setResearchProgress(TechTypes eIndex, int iNewValue, PlayerTypes ePlayer);
	void changeResearchProgress(TechTypes eIndex, int iChange, PlayerTypes ePlayer);
	int changeResearchProgressPercent(TechTypes eIndex, int iPercent, PlayerTypes ePlayer);

	DllExport int getTechCount(TechTypes eIndex) const;

	int getTerrainTradeCount(TerrainTypes eIndex) const;
	bool isTerrainTrade(TerrainTypes eIndex) const;
	void changeTerrainTradeCount(TerrainTypes eIndex, int iChange);

	int getRiverTradeCount() const;
	bool isRiverTrade() const;
	void changeRiverTradeCount(int iChange);

	DllExport bool isHasTech(TechTypes eIndex) const;
	DllExport void setHasTech(TechTypes eIndex, bool bNewValue, PlayerTypes ePlayer, bool bFirst, bool bAnnounce);

	bool isNoTradeTech(TechTypes eIndex) const;
	void setNoTradeTech(TechTypes eIndex, bool bNewValue);

	int getImprovementYieldChange(ImprovementTypes eIndex1, YieldTypes eIndex2) const;
	void changeImprovementYieldChange(ImprovementTypes eIndex1, YieldTypes eIndex2, int iChange);

	bool isFriendlyTerritory(TeamTypes eTeam) const;

	DllExport int getVictoryCountdown(VictoryTypes eIndex) const;
	DllExport void setVictoryCountdown(VictoryTypes eIndex, int iTurnsLeft);
	void changeVictoryCountdown(VictoryTypes eIndex, int iChange);
	DllExport int getVictoryDelay(VictoryTypes eVictory) const;
	DllExport bool canLaunch(VictoryTypes eVictory) const;
	void setCanLaunch(VictoryTypes eVictory, bool bCan);
	int getLaunchSuccessRate(VictoryTypes eVictory) const;
	void resetVictoryProgress();

	bool isParent(TeamTypes eTeam) const;

	int getEspionagePointsAgainstTeam(TeamTypes eIndex) const;
	void setEspionagePointsAgainstTeam(TeamTypes eIndex, int iValue);
	void changeEspionagePointsAgainstTeam(TeamTypes eIndex, int iChange);

	int getEspionagePointsEver() const;
	void setEspionagePointsEver(int iValue);
	void changeEspionagePointsEver(int iChange);

	int getCounterespionageTurnsLeftAgainstTeam(TeamTypes eIndex) const;
	void setCounterespionageTurnsLeftAgainstTeam(TeamTypes eIndex, int iValue);
	DllExport void changeCounterespionageTurnsLeftAgainstTeam(TeamTypes eIndex, int iChange);

	int getCounterespionageModAgainstTeam(TeamTypes eIndex) const;
	void setCounterespionageModAgainstTeam(TeamTypes eIndex, int iValue);
	DllExport void changeCounterespionageModAgainstTeam(TeamTypes eIndex, int iChange);

	void verifySpyUnitsValidPlot();

	void setForceRevealedBonus(BonusTypes eBonus, bool bRevealed);
	bool isForceRevealedBonus(BonusTypes eBonus) const;

	DllExport int countNumHumanGameTurnActive() const;
	void setTurnActive(bool bNewValue, bool bTurn = true);
	bool isTurnActive() const;

	bool hasShrine(ReligionTypes eReligion);

	virtual void AI_init() = 0;
	virtual void AI_reset() = 0;
	virtual void AI_doTurnPre() = 0;
	virtual void AI_doTurnPost() = 0;
	virtual void AI_makeAssignWorkDirty() = 0;
	virtual void AI_updateAreaStragies(bool bTargets = true) = 0;
	virtual bool AI_shareWar(TeamTypes eTeam) const = 0;
	virtual void AI_updateWorstEnemy() = 0;
	virtual int AI_getAtWarCounter(TeamTypes eIndex) const = 0;
	virtual void AI_setAtWarCounter(TeamTypes eIndex, int iNewValue) = 0;
	virtual int AI_getAtPeaceCounter(TeamTypes eIndex) const = 0;
	virtual void AI_setAtPeaceCounter(TeamTypes eIndex, int iNewValue) = 0;
	virtual int AI_getHasMetCounter(TeamTypes eIndex) const = 0;
	virtual void AI_setHasMetCounter(TeamTypes eIndex, int iNewValue) = 0;
	virtual int AI_getOpenBordersCounter(TeamTypes eIndex) const = 0;
	virtual void AI_setOpenBordersCounter(TeamTypes eIndex, int iNewValue) = 0;
	virtual int AI_getDefensivePactCounter(TeamTypes eIndex) const = 0;
	virtual void AI_setDefensivePactCounter(TeamTypes eIndex, int iNewValue) = 0;
	virtual int AI_getShareWarCounter(TeamTypes eIndex) const = 0;
	virtual void AI_setShareWarCounter(TeamTypes eIndex, int iNewValue) = 0;
	virtual int AI_getWarSuccess(TeamTypes eIndex) const = 0;
	virtual void AI_setWarSuccess(TeamTypes eIndex, int iNewValue) = 0;
	virtual void AI_changeWarSuccess(TeamTypes eIndex, int iChange) = 0;
	virtual int AI_getEnemyPeacetimeTradeValue(TeamTypes eIndex) const = 0;
	virtual void AI_setEnemyPeacetimeTradeValue(TeamTypes eIndex, int iNewValue) = 0;
	virtual int AI_getEnemyPeacetimeGrantValue(TeamTypes eIndex) const = 0;
	virtual void AI_setEnemyPeacetimeGrantValue(TeamTypes eIndex, int iNewValue) = 0;
	virtual WarPlanTypes AI_getWarPlan(TeamTypes eIndex) const = 0;
	virtual bool AI_isChosenWar(TeamTypes eIndex) const = 0;
	virtual bool AI_isSneakAttackPreparing(TeamTypes eIndex) const = 0;
	virtual bool AI_isSneakAttackReady(TeamTypes eIndex) const = 0;
	virtual void AI_setWarPlan(TeamTypes eIndex, WarPlanTypes eNewValue, bool bWar = true) = 0;

protected:

	int m_iNumMembers;
	int m_iAliveCount;
	int m_iEverAliveCount;
	int m_iNumCities;
	int m_iTotalPopulation;
	int m_iTotalLand;
	int m_iNukeInterception;
	int m_iExtraWaterSeeFromCount;
	int m_iMapTradingCount;
	int m_iTechTradingCount;
	int m_iGoldTradingCount;
	int m_iOpenBordersTradingCount;
	int m_iDefensivePactTradingCount;
	int m_iPermanentAllianceTradingCount;
	int m_iVassalTradingCount;
	int m_iBridgeBuildingCount;
	int m_iIrrigationCount;
	int m_iIgnoreIrrigationCount;
	int m_iWaterWorkCount;
	int m_iVassalPower;
	int m_iMasterPower;
	int m_iEnemyWarWearinessModifier;
	int m_iRiverTradeCount;
	int m_iEspionagePointsEver;

	bool m_bMapCentering;
	bool m_bCapitulated;

	TeamTypes m_eID;

	int* m_aiStolenVisibilityTimer;
	int* m_aiWarWeariness;
	int* m_aiTechShareCount;
	int* m_aiCommerceFlexibleCount;
	int* m_aiExtraMoves;
	int* m_aiForceTeamVoteEligibilityCount;

	bool* m_abAtWar;
	bool* m_abHasMet;
	bool* m_abPermanentWarPeace;
	bool* m_abOpenBorders;
	bool* m_abDefensivePact;
	bool* m_abForcePeace;
	bool* m_abVassal;
	bool* m_abCanLaunch;

	int* m_paiRouteChange;
	int* m_paiProjectCount;
	int* m_paiProjectDefaultArtTypes;
	std::vector<int> *m_pavProjectArtTypes;
	int* m_paiProjectMaking;
	int* m_paiUnitClassCount;
	int* m_paiBuildingClassCount;
	int* m_paiObsoleteBuildingCount;
	int* m_paiResearchProgress;
	int* m_paiTechCount;
	int* m_paiTerrainTradeCount;
	int* m_aiVictoryCountdown;

	int* m_aiEspionagePointsAgainstTeam;
	int* m_aiCounterespionageTurnsLeftAgainstTeam;
	int* m_aiCounterespionageModAgainstTeam;

	bool* m_pabHasTech;
	bool* m_pabNoTradeTech;

	int** m_ppaaiImprovementYieldChange;

	std::vector<BonusTypes> m_aeRevealedBonuses;

	void doWarWeariness();

	void updateTechShare(TechTypes eTech);
	void updateTechShare();

	void testCircumnavigated();

	void processTech(TechTypes eTech, int iChange);

	void cancelDefensivePacts();
	void announceTechToPlayers(TechTypes eIndex, bool bPartial = false);

	virtual void read(FDataStreamBase* pStream);
	virtual void write(FDataStreamBase* pStream);
};

#endif
