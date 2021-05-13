#pragma once

// game.h

#ifndef CIV4_GAME_H
#define CIV4_GAME_H

//#include "CvStructs.h"
#include "CvDeal.h"
#include "CvRandom.h"

class CvPlot;
class CvCity;
class CvReplayMessage;
class CvReplayInfo;

typedef std::vector<const CvReplayMessage*> ReplayMessageList;

class CvGame
{

public:

	DllExport CvGame();
	DllExport virtual ~CvGame();

	DllExport void init(HandicapTypes eHandicap);
	DllExport void reset(HandicapTypes eHandicap, bool bConstructorCall = false);

protected:

	void uninit();

public:

	DllExport void setInitialItems();
	DllExport void regenerateMap();

	DllExport void initDiplomacy();
	DllExport void initFreeState();
	DllExport void initFreeUnits();

	DllExport void assignStartingPlots();
	DllExport void normalizeStartingPlots();

	DllExport void update();
	DllExport void updateScore(bool bForce = false);

	DllExport void updateColoredPlots();
	DllExport void updateBlockadedPlots();

	DllExport void updatePlotGroups();
	DllExport void updateBuildingCommerce();
	DllExport void updateCitySight(bool bIncrement);
	DllExport void updateTradeRoutes();

	DllExport void updateSelectionList();
	DllExport void updateTestEndTurn();

	DllExport void testExtendedGame();

	DllExport CvUnit* getPlotUnit(const CvPlot* pPlot, int iIndex);
	DllExport void getPlotUnits(const CvPlot *pPlot, std::vector<CvUnit *> &plotUnits);

	DllExport void cycleCities(bool bForward = true, bool bAdd = false);
	DllExport void cycleSelectionGroups(bool bClear, bool bForward = true, bool bWorkers = false);
	DllExport bool cyclePlotUnits(CvPlot* pPlot, bool bForward = true, bool bAuto = false, int iCount = -1);

	DllExport void selectionListMove(CvPlot* pPlot, bool bAlt, bool bShift, bool bCtrl);
	DllExport void selectionListGameNetMessage(int eMessage, int iData2 = -1, int iData3 = -1, int iData4 = -1, int iFlags = 0, bool bAlt = false, bool bShift = false);
	DllExport void selectedCitiesGameNetMessage(int eMessage, int iData2 = -1, int iData3 = -1, int iData4 = -1, bool bOption = false, bool bAlt = false, bool bShift = false, bool bCtrl = false);
	DllExport void cityPushOrder(CvCity* pCity, OrderTypes eOrder, int iData, bool bAlt = false, bool bShift = false, bool bCtrl = false);

	DllExport void selectUnit(CvUnit* pUnit, bool bClear, bool bToggle = false, bool bSound = false);
	DllExport void selectGroup(CvUnit* pUnit, bool bShift, bool bCtrl, bool bAlt);
	DllExport void selectAll(CvPlot* pPlot);

	DllExport bool selectionListIgnoreBuildingDefense();

	DllExport bool canHandleAction(int iAction, CvPlot* pPlot = NULL, bool bTestVisible = false, bool bUseCache = false);
	DllExport void setupActionCache();
	DllExport void handleAction(int iAction);

	DllExport bool canDoControl(ControlTypes eControl);
	DllExport void doControl(ControlTypes eControl);

	DllExport void implementDeal(PlayerTypes eWho, PlayerTypes eOtherWho, CLinkList<TradeData>* pOurList, CLinkList<TradeData>* pTheirList, bool bForce = false);
	DllExport void verifyDeals();

	DllExport void getGlobeviewConfigurationParameters(TeamTypes eTeam, bool& bStarsVisible, bool& bWorldIsRound);

	DllExport int getSymbolID(int iSymbol);

	int getProductionPerPopulation(HurryTypes eHurry);

	int getAdjustedPopulationPercent(VictoryTypes eVictory) const;
	int getAdjustedLandPercent(VictoryTypes eVictory) const;

	bool isTeamVote(VoteTypes eVote) const;
	bool isChooseElection(VoteTypes eVote) const;
	bool isTeamVoteEligible(TeamTypes eTeam, VoteSourceTypes eVoteSource) const;
	int countVote(const VoteTriggeredData& kData, PlayerVoteTypes eChoice) const;
	int countPossibleVote(VoteTypes eVote, VoteSourceTypes eVoteSource) const;
	TeamTypes findHighestVoteTeam(const VoteTriggeredData& kData) const;
	int getVoteRequired(VoteTypes eVote, VoteSourceTypes eVoteSource) const;
	TeamTypes getSecretaryGeneral(VoteSourceTypes eVoteSource) const;
	bool canHaveSecretaryGeneral(VoteSourceTypes eVoteSource) const;
	void clearSecretaryGeneral(VoteSourceTypes eVoteSource);
	void updateSecretaryGeneral();

	DllExport int countCivPlayersAlive() const;
	DllExport int countCivPlayersEverAlive() const;
	DllExport int countCivTeamsAlive() const;
	DllExport int countCivTeamsEverAlive() const;
	DllExport int countHumanPlayersAlive() const;
	DllExport int countHumanPlayersEverAlive() const;

	int countTotalCivPower();
	int countTotalNukeUnits();
	int countKnownTechNumTeams(TechTypes eTech);
	int getNumFreeBonuses(BuildingTypes eBuilding);

	DllExport int countReligionLevels(ReligionTypes eReligion);
	DllExport int calculateReligionPercent(ReligionTypes eReligion) const;
	DllExport int countCorporationLevels(CorporationTypes eCorporation);
	void replaceCorporation(CorporationTypes eCorporation1, CorporationTypes eCorporation2);

	DllExport int goldenAgeLength() const;
	DllExport int victoryDelay(VictoryTypes eVictory) const;
	DllExport int getImprovementUpgradeTime(ImprovementTypes eImprovement) const;

	DllExport bool canTrainNukes() const;
	DllExport EraTypes getCurrentEra() const;

	DllExport TeamTypes getActiveTeam();
	DllExport CivilizationTypes getActiveCivilizationType();

	DllExport unsigned int getLastEndTurnMessageSentTime();
	DllExport bool isNetworkMultiPlayer() const;
	DllExport bool isGameMultiPlayer() const;
	DllExport bool isTeamGame() const;

	DllExport bool isModem();
	DllExport void setModem(bool bModem);

	DllExport void reviveActivePlayer();

	DllExport int getNumHumanPlayers();

	DllExport int getEndTurnMessagesSent();
	DllExport void incrementEndTurnMessagesSent();

	DllExport int getGameTurn();
	DllExport void setGameTurn(int iNewValue);
	DllExport void incrementGameTurn();
	DllExport int getTurnYear(int iGameTurn);
	DllExport int getGameTurnYear();

	DllExport int getElapsedGameTurns() const;
	DllExport void incrementElapsedGameTurns();

	DllExport int getMaxTurns() const;
	DllExport void setMaxTurns(int iNewValue);
	DllExport void changeMaxTurns(int iChange);

	DllExport int getMaxCityElimination() const;
	DllExport void setMaxCityElimination(int iNewValue);

	DllExport int getNumAdvancedStartPoints() const;
	DllExport void setNumAdvancedStartPoints(int iNewValue);

	DllExport int getStartTurn() const;
	DllExport void setStartTurn(int iNewValue);

	DllExport int getStartYear() const;
	DllExport void setStartYear(int iNewValue);

	DllExport int getEstimateEndTurn() const;
	DllExport void setEstimateEndTurn(int iNewValue);

	DllExport int getTurnSlice() const;
	DllExport int getMinutesPlayed() const;
	DllExport void setTurnSlice(int iNewValue);
	DllExport void changeTurnSlice(int iChange);

	DllExport int getCutoffSlice() const;
	DllExport void setCutoffSlice(int iNewValue);
	DllExport void changeCutoffSlice(int iChange);

	DllExport int getTurnSlicesRemaining();
	DllExport void resetTurnTimer();
	DllExport void incrementTurnTimer(int iNumTurnSlices);
	DllExport int getMaxTurnLen();

	DllExport int getTargetScore() const;
	DllExport void setTargetScore(int iNewValue);

	DllExport int getNumGameTurnActive();
	DllExport int countNumHumanGameTurnActive();
	DllExport void changeNumGameTurnActive(int iChange);

	DllExport int getNumCities() const;
	DllExport int getNumCivCities() const;
	DllExport void changeNumCities(int iChange);

	DllExport int getTotalPopulation() const;
	DllExport void changeTotalPopulation(int iChange);

	DllExport int getTradeRoutes() const;
	DllExport void changeTradeRoutes(int iChange);

	DllExport int getFreeTradeCount() const;
	DllExport bool isFreeTrade() const;
	DllExport void changeFreeTradeCount(int iChange);

	DllExport int getNoNukesCount() const;
	DllExport bool isNoNukes() const;
	DllExport void changeNoNukesCount(int iChange);

	DllExport int getSecretaryGeneralTimer(VoteSourceTypes eVoteSource) const;
	DllExport void setSecretaryGeneralTimer(VoteSourceTypes eVoteSource, int iNewValue);
	DllExport void changeSecretaryGeneralTimer(VoteSourceTypes eVoteSource, int iChange);

	DllExport int getVoteTimer(VoteSourceTypes eVoteSource) const;
	DllExport void setVoteTimer(VoteSourceTypes eVoteSource, int iNewValue);
	DllExport void changeVoteTimer(VoteSourceTypes eVoteSource, int iChange);

	DllExport int getNukesExploded() const;
	DllExport void changeNukesExploded(int iChange);

	DllExport int getMaxPopulation() const;
	DllExport int getMaxLand() const;
	DllExport int getMaxTech() const;
	DllExport int getMaxWonders() const;
	DllExport int getInitPopulation() const;
	DllExport int getInitLand() const;
	DllExport int getInitTech() const;
	DllExport int getInitWonders() const;
	DllExport void initScoreCalculation();

	DllExport int getAIAutoPlay();
	DllExport void setAIAutoPlay(int iNewValue);
	DllExport void changeAIAutoPlay(int iChange);

	DllExport unsigned int getInitialTime();
	DllExport void setInitialTime(unsigned int uiNewValue);

	bool isScoreDirty() const;
	void setScoreDirty(bool bNewValue);

	DllExport bool isCircumnavigated() const;
	DllExport void makeCircumnavigated();
	bool circumnavigationAvailable() const;

	DllExport bool isDiploVote(VoteSourceTypes eVoteSource) const;
	int getDiploVoteCount(VoteSourceTypes eVoteSource) const;
	DllExport void changeDiploVote(VoteSourceTypes eVoteSource, int iChange);
	DllExport bool canDoResolution(VoteSourceTypes eVoteSource, const VoteSelectionSubData& kData) const;
	bool isValidVoteSelection(VoteSourceTypes eVoteSource, const VoteSelectionSubData& kData) const;

	DllExport bool isDebugMode() const;
	DllExport void toggleDebugMode();
	DllExport void updateDebugModeCache();

	DllExport int getPitbossTurnTime() const;
	DllExport void setPitbossTurnTime(int iHours);

	DllExport bool isHotSeat() const;
	DllExport bool isPbem() const;
	DllExport bool isPitboss() const;
	DllExport bool isSimultaneousTeamTurns() const;

	DllExport bool isFinalInitialized() const;
	DllExport void setFinalInitialized(bool bNewValue);

	DllExport bool getPbemTurnSent() const;
	DllExport void setPbemTurnSent(bool bNewValue);

	DllExport bool getHotPbemBetweenTurns() const;
	DllExport void setHotPbemBetweenTurns(bool bNewValue);

	DllExport bool isPlayerOptionsSent() const;
	DllExport void sendPlayerOptions(bool bForce = false);

	DllExport PlayerTypes getActivePlayer() const;
	DllExport void setActivePlayer(PlayerTypes eNewValue, bool bForceHotSeat = false);
	DllExport void updateUnitEnemyGlow();

	DllExport HandicapTypes getHandicapType() const;
	DllExport void setHandicapType(HandicapTypes eHandicap);

	DllExport PlayerTypes getPausePlayer();
	DllExport bool isPaused();
	DllExport void setPausePlayer(PlayerTypes eNewValue);

	DllExport UnitTypes getBestLandUnit();
	DllExport int getBestLandUnitCombat();
	DllExport void setBestLandUnit(UnitTypes eNewValue);

	DllExport TeamTypes getWinner() const;
	DllExport VictoryTypes getVictory() const;
	DllExport void setWinner(TeamTypes eNewWinner, VictoryTypes eNewVictory);

	DllExport GameStateTypes getGameState();
	DllExport void setGameState(GameStateTypes eNewValue);

	DllExport EraTypes getStartEra() const;

	DllExport CalendarTypes getCalendar() const;

	DllExport GameSpeedTypes getGameSpeedType() const;

	DllExport int getEndTurnMessagesReceived(int iIndex);
	DllExport void incrementEndTurnMessagesReceived(int iIndex);

	DllExport PlayerTypes getRankPlayer(int iRank);
	DllExport void setRankPlayer(int iRank, PlayerTypes ePlayer);

	DllExport int getPlayerRank(PlayerTypes ePlayer);
	DllExport void setPlayerRank(PlayerTypes ePlayer, int iRank);

	DllExport int getPlayerScore(PlayerTypes ePlayer);
	DllExport void setPlayerScore(PlayerTypes ePlayer, int iScore);

	DllExport TeamTypes getRankTeam(int iRank);
	DllExport void setRankTeam(int iRank, TeamTypes eTeam);

	DllExport int getTeamRank(TeamTypes eTeam);
	DllExport void setTeamRank(TeamTypes eTeam, int iRank);

	DllExport int getTeamScore(TeamTypes eTeam) const;
	DllExport void setTeamScore(TeamTypes eTeam, int iScore);

	DllExport bool isOption(GameOptionTypes eIndex) const;
	DllExport void setOption(GameOptionTypes eIndex, bool bEnabled);

	DllExport bool isMPOption(MultiplayerOptionTypes eIndex) const;
	DllExport void setMPOption(MultiplayerOptionTypes eIndex, bool bEnabled);

	DllExport bool isForcedControl(ForceControlTypes eIndex) const;
	DllExport void setForceControl(ForceControlTypes eIndex, bool bEnabled);

	DllExport int getUnitCreatedCount(UnitTypes eIndex);
	DllExport void incrementUnitCreatedCount(UnitTypes eIndex);

	DllExport int getUnitClassCreatedCount(UnitClassTypes eIndex);
	DllExport bool isUnitClassMaxedOut(UnitClassTypes eIndex, int iExtra = 0);
	DllExport void incrementUnitClassCreatedCount(UnitClassTypes eIndex);

	DllExport int getBuildingClassCreatedCount(BuildingClassTypes eIndex);
	DllExport bool isBuildingClassMaxedOut(BuildingClassTypes eIndex, int iExtra = 0);
	DllExport void incrementBuildingClassCreatedCount(BuildingClassTypes eIndex);

	DllExport int getProjectCreatedCount(ProjectTypes eIndex);
	DllExport bool isProjectMaxedOut(ProjectTypes eIndex, int iExtra = 0);
	DllExport void incrementProjectCreatedCount(ProjectTypes eIndex, int iExtra = 1);

	DllExport int getForceCivicCount(CivicTypes eIndex) const;
	DllExport bool isForceCivic(CivicTypes eIndex) const;
	DllExport bool isForceCivicOption(CivicOptionTypes eCivicOption) const;
	DllExport void changeForceCivicCount(CivicTypes eIndex, int iChange);

	DllExport PlayerVoteTypes getVoteOutcome(VoteTypes eIndex) const;
	DllExport bool isVotePassed(VoteTypes eIndex) const;
	DllExport void setVoteOutcome(const VoteTriggeredData& kData, PlayerVoteTypes eNewValue);

	DllExport bool isVictoryValid(VictoryTypes eIndex) const;
	DllExport void setVictoryValid(VictoryTypes eIndex, bool bValid);

	DllExport bool isSpecialUnitValid(SpecialUnitTypes eIndex);
	DllExport void makeSpecialUnitValid(SpecialUnitTypes eIndex);

	DllExport bool isSpecialBuildingValid(SpecialBuildingTypes eIndex);
	DllExport void makeSpecialBuildingValid(SpecialBuildingTypes eIndex, bool bAnnounce = false);

	DllExport bool isNukesValid() const;
	DllExport void makeNukesValid(bool bValid = true);

	DllExport bool isInAdvancedStart() const;

	DllExport void setVoteChosen(int iSelection, int iVoteId);

	DllExport int getReligionGameTurnFounded(ReligionTypes eIndex);
	DllExport bool isReligionFounded(ReligionTypes eIndex);
	void makeReligionFounded(ReligionTypes eIndex, PlayerTypes ePlayer);

	bool isReligionSlotTaken(ReligionTypes eReligion) const;
	void setReligionSlotTaken(ReligionTypes eReligion, bool bTaken);

	DllExport CvCity* getHolyCity(ReligionTypes eIndex);
	DllExport void setHolyCity(ReligionTypes eIndex, CvCity* pNewValue, bool bAnnounce);

	DllExport int getCorporationGameTurnFounded(CorporationTypes eIndex);
	DllExport bool isCorporationFounded(CorporationTypes eIndex);
	DllExport void makeCorporationFounded(CorporationTypes eIndex, PlayerTypes ePlayer);

	DllExport CvCity* getHeadquarters(CorporationTypes eIndex);
	DllExport void setHeadquarters(CorporationTypes eIndex, CvCity* pNewValue, bool bAnnounce);

	DllExport PlayerVoteTypes getPlayerVote(PlayerTypes eOwnerIndex, int iVoteId) const;
	DllExport void setPlayerVote(PlayerTypes eOwnerIndex, int iVoteId, PlayerVoteTypes eNewValue);
	DllExport void castVote(PlayerTypes eOwnerIndex, int iVoteId, PlayerVoteTypes ePlayerVote);

	DllExport const CvWString & getName();
	DllExport void setName(const TCHAR* szName);

	// Script data needs to be a narrow string for pickling in Python
	std::string getScriptData() const;
	void setScriptData(std::string szNewValue);

	DllExport bool isDestroyedCityName(CvWString& szName) const;
	DllExport void addDestroyedCityName(const CvWString& szName);

	DllExport bool isGreatPersonBorn(CvWString& szName) const;
	DllExport void addGreatPersonBornName(const CvWString& szName);

	DllExport int getIndexAfterLastDeal();
	DllExport int getNumDeals();
	DllExport CvDeal* getDeal(int iID);
	DllExport CvDeal* addDeal();
	DllExport void deleteDeal(int iID);
	// iteration
	DllExport CvDeal* firstDeal(int *pIterIdx, bool bRev=false);
	DllExport CvDeal* nextDeal(int *pIterIdx, bool bRev=false);

	VoteSelectionData* getVoteSelection(int iID) const;
	VoteSelectionData* addVoteSelection(VoteSourceTypes eVoteSource);
	void deleteVoteSelection(int iID);

	VoteTriggeredData* getVoteTriggered(int iID) const;
	VoteTriggeredData* addVoteTriggered(const VoteSelectionData& kData, int iChoice);
	VoteTriggeredData* addVoteTriggered(VoteSourceTypes eVoteSource, const VoteSelectionSubData& kOptionData);
	void deleteVoteTriggered(int iID);

	DllExport CvRandom& getMapRand();
	DllExport int getMapRandNum(int iNum, const char* pszLog);

	DllExport CvRandom& getSorenRand();
	DllExport int getSorenRandNum(int iNum, const char* pszLog);

	DllExport int calculateSyncChecksum();
	DllExport int calculateOptionsChecksum();

	DllExport void addReplayMessage(ReplayMessageTypes eType = NO_REPLAY_MESSAGE, PlayerTypes ePlayer = NO_PLAYER, CvWString pszText = L"",
		int iPlotX = -1, int iPlotY = -1, ColorTypes eColor = NO_COLOR);
	DllExport void clearReplayMessageMap();
	DllExport int getReplayMessageTurn(uint i) const;
	DllExport ReplayMessageTypes getReplayMessageType(uint i) const;
	DllExport int getReplayMessagePlotX(uint i) const;
	DllExport int getReplayMessagePlotY(uint i) const;
	DllExport PlayerTypes getReplayMessagePlayer(uint i) const;
	DllExport LPCWSTR getReplayMessageText(uint i) const;
	DllExport uint getNumReplayMessages() const;
	DllExport ColorTypes getReplayMessageColor(uint i) const;

	DllExport virtual void read(FDataStreamBase* pStream);
	DllExport virtual void write(FDataStreamBase* pStream);
	DllExport virtual void writeReplay(FDataStreamBase& stream, PlayerTypes ePlayer);

	DllExport virtual void AI_init() = 0;
	DllExport virtual void AI_reset() = 0;
	DllExport virtual void AI_makeAssignWorkDirty() = 0;
	DllExport virtual void AI_updateAssignWork() = 0;
	DllExport virtual int AI_combatValue(UnitTypes eUnit) = 0;

	CvReplayInfo* getReplayInfo() const;
	DllExport void setReplayInfo(CvReplayInfo* pReplay);
	void saveReplay(PlayerTypes ePlayer);

	bool hasSkippedSaveChecksum() const;

	void addPlayer(PlayerTypes eNewPlayer, LeaderHeadTypes eLeader, CivilizationTypes eCiv);

	bool testVictory(VictoryTypes eVictory, TeamTypes eTeam, bool* pbEndScore = NULL) const;

	bool isCompetingCorporation(CorporationTypes eCorporation1, CorporationTypes eCorporation2) const;

	int getShrineBuildingCount(ReligionTypes eReligion = NO_RELIGION);
	BuildingTypes getShrineBuilding(int eIndex, ReligionTypes eReligion = NO_RELIGION);
	void changeShrineBuilding(BuildingTypes eBuilding, ReligionTypes eReligion, bool bRemove = false);

	bool culturalVictoryValid();
	int culturalVictoryNumCultureCities();
	CultureLevelTypes culturalVictoryCultureLevel();

	int getPlotExtraYield(int iX, int iY, YieldTypes eYield) const;
	void setPlotExtraYield(int iX, int iY, YieldTypes eYield, int iCost);
	void removePlotExtraYield(int iX, int iY);

	int getPlotExtraCost(int iX, int iY) const;
	void changePlotExtraCost(int iX, int iY, int iCost);
	void removePlotExtraCost(int iX, int iY);

	ReligionTypes getVoteSourceReligion(VoteSourceTypes eVoteSource) const;
	void setVoteSourceReligion(VoteSourceTypes eVoteSource, ReligionTypes eReligion, bool bAnnounce = false);

	bool isEventActive(EventTriggerTypes eTrigger) const;
	DllExport void initEvents();

	bool isCivEverActive(CivilizationTypes eCivilization) const;
	bool isLeaderEverActive(LeaderHeadTypes eLeader) const;
	bool isUnitEverActive(UnitTypes eUnit) const;
	bool isBuildingEverActive(BuildingTypes eBuilding) const;

	void processBuilding(BuildingTypes eBuilding, int iChange);

protected:

	int m_iEndTurnMessagesSent;
	int m_iElapsedGameTurns;
	int m_iStartTurn;
	int m_iStartYear;
	int m_iEstimateEndTurn;
	int m_iTurnSlice;
	int m_iCutoffSlice;
	int m_iNumGameTurnActive;
	int m_iNumCities;
	int m_iTotalPopulation;
	int m_iTradeRoutes;
	int m_iFreeTradeCount;
	int m_iNoNukesCount;
	int m_iNukesExploded;
	int m_iMaxPopulation;
	int m_iMaxLand;
	int m_iMaxTech;
	int m_iMaxWonders;
	int m_iInitPopulation;
	int m_iInitLand;
	int m_iInitTech;
	int m_iInitWonders;
	int m_iAIAutoPlay;

	unsigned int m_uiInitialTime;

	bool m_bScoreDirty;
	bool m_bCircumnavigated;
	bool m_bDebugMode;
	bool m_bDebugModeCache;
	bool m_bFinalInitialized;
	bool m_bPbemTurnSent;
	bool m_bHotPbemBetweenTurns;
	bool m_bPlayerOptionsSent;
	bool m_bNukesValid;

	HandicapTypes m_eHandicap;
	PlayerTypes m_ePausePlayer;
	UnitTypes m_eBestLandUnit;
	TeamTypes m_eWinner;
	VictoryTypes m_eVictory;
	GameStateTypes m_eGameState;
	PlayerTypes m_eEventPlayer;

	CvString m_szScriptData;

	int* m_aiEndTurnMessagesReceived;
	int* m_aiRankPlayer;        // Ordered by rank...
	int* m_aiPlayerRank;        // Ordered by player ID...
	int* m_aiPlayerScore;       // Ordered by player ID...
	int* m_aiRankTeam;						// Ordered by rank...
	int* m_aiTeamRank;						// Ordered by team ID...
	int* m_aiTeamScore;						// Ordered by team ID...

	int* m_paiUnitCreatedCount;
	int* m_paiUnitClassCreatedCount;
	int* m_paiBuildingClassCreatedCount;
	int* m_paiProjectCreatedCount;
	int* m_paiForceCivicCount;
	PlayerVoteTypes* m_paiVoteOutcome;
	int* m_paiReligionGameTurnFounded;
	int* m_paiCorporationGameTurnFounded;
	int* m_aiSecretaryGeneralTimer;
	int* m_aiVoteTimer;
	int* m_aiDiploVote;

	bool* m_pabSpecialUnitValid;
	bool* m_pabSpecialBuildingValid;
	bool* m_abReligionSlotTaken;

	IDInfo* m_paHolyCity;
	IDInfo* m_paHeadquarters;

	int** m_apaiPlayerVote;

	std::vector<CvWString> m_aszDestroyedCities;
	std::vector<CvWString> m_aszGreatPeopleBorn;

	FFreeListTrashArray<CvDeal> m_deals;
	FFreeListTrashArray<VoteSelectionData> m_voteSelections;
	FFreeListTrashArray<VoteTriggeredData> m_votesTriggered;

	CvRandom m_mapRand;
	CvRandom m_sorenRand;

	ReplayMessageList m_listReplayMessages;
	CvReplayInfo* m_pReplayInfo;

	int m_iNumSessions;

	std::vector<PlotExtraYield> m_aPlotExtraYields;
	std::vector<PlotExtraCost> m_aPlotExtraCosts;
	stdext::hash_map<VoteSourceTypes, ReligionTypes> m_mapVoteSourceReligions;
	std::vector<EventTriggerTypes> m_aeInactiveTriggers;

	// CACHE: cache frequently used values
	int		m_iShrineBuildingCount;
	int*	m_aiShrineBuilding;
	int*	m_aiShrineReligion;

	int		m_iNumCultureVictoryCities;
	int		m_eCultureVictoryCultureLevel;

	void doTurn();
	void doDeals();
	void doGlobalWarming();
	void doHolyCity();
	void doHeadquarters();
	void doDiploVote();
	void doVoteResults();
	void doVoteSelection();

	void createBarbarianCities();
	void createBarbarianUnits();
	void createAnimals();

	void verifyCivics();

	void updateWar();
	void updateMoves();
	void updateTimers();
	void updateTurnTimer();

	void testAlive();
	void testVictory();

	void processVote(const VoteTriggeredData& kData, int iChange);

	int getTeamClosenessScore(int** aaiDistances, int* aiStartingLocs);
	void normalizeStartingPlotLocations();
	void normalizeAddRiver();
	void normalizeRemovePeaks();
	void normalizeAddLakes();
	void normalizeRemoveBadFeatures();
	void normalizeRemoveBadTerrain();
	void normalizeAddFoodBonuses();
	void normalizeAddGoodTerrain();
	void normalizeAddExtras();

	void showEndGameSequence();

	CvPlot* normalizeFindLakePlot(PlayerTypes ePlayer);

	void doUpdateCacheOnTurn();
};

#endif
