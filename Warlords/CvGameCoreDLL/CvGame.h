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

	DllExport void updatePlotGroups();
	DllExport void updateBuildingCommerce();
	DllExport void updateCitySight(bool bIncrement);
	DllExport void updateTradeRoutes();

	DllExport void updateSelectionList();
	DllExport void updateTestEndTurn();

	DllExport void testExtendedGame();

	DllExport CvUnit* getPlotUnit(const CvPlot* pPlot, int iIndex);

	DllExport void cycleCities(bool bForward = true, bool bAdd = false);																				// Exposed to Python
	DllExport void cycleSelectionGroups(bool bClear, bool bForward = true, bool bWorkers = false);							// Exposed to Python
	DllExport bool cyclePlotUnits(CvPlot* pPlot, bool bForward = true, bool bAuto = false, int iCount = -1);		// Exposed to Python

	DllExport void selectionListMove(CvPlot* pPlot, bool bAlt, bool bShift, bool bCtrl);												// Exposed to Python
	DllExport void selectionListGameNetMessage(int eMessage, int iData2 = -1, int iData3 = -1, int iData4 = -1, int iFlags = 0, bool bAlt = false, bool bShift = false);	// Exposed to Python
	DllExport void selectedCitiesGameNetMessage(int eMessage, int iData2 = -1, int iData3 = -1, int iData4 = -1, bool bOption = false, bool bAlt = false, bool bShift = false, bool bCtrl = false);	// Exposed to Python
	DllExport void cityPushOrder(CvCity* pCity, OrderTypes eOrder, int iData, bool bAlt = false, bool bShift = false, bool bCtrl = false);	// Exposed to Python

	DllExport void selectUnit(CvUnit* pUnit, bool bClear, bool bToggle = false, bool bSound = false);
	DllExport void selectGroup(CvUnit* pUnit, bool bShift, bool bCtrl, bool bAlt);
	DllExport void selectAll(CvPlot* pPlot);

	DllExport bool selectionListIgnoreBuildingDefense();

	DllExport bool canHandleAction(int iAction, CvPlot* pPlot = NULL, bool bTestVisible = false);
	DllExport void handleAction(int iAction);

	DllExport bool canDoControl(ControlTypes eControl);
	DllExport void doControl(ControlTypes eControl);

	DllExport void implementDeal(PlayerTypes eWho, PlayerTypes eOtherWho, CLinkList<TradeData>* pOurList, CLinkList<TradeData>* pTheirList);
	DllExport void verifyDeals();

	DllExport void getGlobeviewConfigurationParameters(TeamTypes eTeam, bool& bStarsVisible, bool& bWorldIsRound);

	DllExport int getSymbolID(int iSymbol);																	// Exposed to Python

	int getProductionPerPopulation(HurryTypes eHurry);											// Exposed to Python

	int getAdjustedPopulationPercent(VictoryTypes eVictory);								// Exposed to Python
	int getAdjustedLandPercent(VictoryTypes eVictory);											// Exposed to Python

	DllExport bool isTeamVote(VoteTypes eVote) const;												// Exposed to Python
	DllExport bool isChooseElection(VoteTypes eVote) const;									// Exposed to Python
	DllExport bool isTeamVoteEligible(TeamTypes eTeam) const;								// Exposed to Python
	DllExport int countVote(VoteTypes eVote, int iChoice) const;						// Exposed to Python
	DllExport int countPossibleVote() const;																// Exposed to Python
	DllExport TeamTypes findHighestVoteTeam(VoteTypes eVote) const;					// Exposed to Python
	DllExport int getVoteRequired(VoteTypes eVote) const;										// Exposed to Python
	DllExport TeamTypes getSecretaryGeneral() const;												// Exposed to Python

	DllExport int countCivPlayersAlive();																		// Exposed to Python
	DllExport int countCivPlayersEverAlive();																// Exposed to Python
	DllExport int countCivTeamsAlive();																			// Exposed to Python
	DllExport int countCivTeamsEverAlive();																	// Exposed to Python
	DllExport int countHumanPlayersAlive();																	// Exposed to Python
	DllExport int countHumanPlayersEverAlive();															// Exposed to Python

	int countTotalCivPower();																								// Exposed to Python
	int countTotalNukeUnits();																							// Exposed to Python
	int countKnownTechNumTeams(TechTypes eTech);														// Exposed to Python
	int getNumFreeBonuses(BuildingTypes eBuilding);													// Exposed to Python

	DllExport int countReligionLevels(ReligionTypes eReligion);							// Exposed to Python 
	DllExport int calculateReligionPercent(ReligionTypes eReligion);				// Exposed to Python

	DllExport int goldenAgeLength() const;																					// Exposed to Python
	DllExport int getImprovementUpgradeTime(ImprovementTypes eImprovement) const;		// Exposed to Python

	DllExport bool canTrainNukes() const;																		// Exposed to Python
	DllExport bool showAllMoves(TeamTypes eTeam) const;
	DllExport EraTypes getCurrentEra() const;

	DllExport TeamTypes getActiveTeam();																		// Exposed to Python
	DllExport CivilizationTypes getActiveCivilizationType();								// Exposed to Python

	DllExport unsigned int getLastEndTurnMessageSentTime();
	DllExport bool isNetworkMultiPlayer();																	// Exposed to Python
	DllExport bool isGameMultiPlayer();																			// Exposed to Python
	DllExport bool isTeamGame();																						// Exposed to Python

	DllExport bool isModem();
	DllExport void setModem(bool bModem);

	DllExport void reviveActivePlayer();																		// Exposed to Python

	DllExport int getNumHumanPlayers();																			// Exposed to Python

	DllExport int getEndTurnMessagesSent();
	DllExport void incrementEndTurnMessagesSent();															

	DllExport int getGameTurn();																						// Exposed to Python
	DllExport void setGameTurn(int iNewValue);															// Exposed to Python
	DllExport void incrementGameTurn();
	DllExport int getTurnYear(int iGameTurn);																// Exposed to Python
	DllExport int getGameTurnYear();																				// Exposed to Python

	DllExport int getElapsedGameTurns();																		// Exposed to Python
	DllExport void incrementElapsedGameTurns();

	DllExport int getMaxTurns() const;																			// Exposed to Python
	DllExport void setMaxTurns(int iNewValue);															// Exposed to Python
	DllExport void changeMaxTurns(int iChange);															// Exposed to Python

	DllExport int getMaxCityElimination() const;														// Exposed to Python
	DllExport void setMaxCityElimination(int iNewValue);										// Exposed to Python

	DllExport int getStartTurn() const;																			// Exposed to Python
	DllExport void setStartTurn(int iNewValue);

	DllExport int getStartYear() const;																			// Exposed to Python
	DllExport void setStartYear(int iNewValue);															// Exposed to Python

	DllExport int getEstimateEndTurn() const;																// Exposed to Python
	DllExport void setEstimateEndTurn(int iNewValue);												// Exposed to Python

	DllExport int getTurnSlice() const;																			// Exposed to Python
	DllExport int getMinutesPlayed() const;																	// Exposed to Python
	DllExport void setTurnSlice(int iNewValue);
	DllExport void changeTurnSlice(int iChange);

	DllExport int getCutoffSlice() const;
	DllExport void setCutoffSlice(int iNewValue);
	DllExport void changeCutoffSlice(int iChange);

	DllExport int getTurnSlicesRemaining();
	DllExport void resetTurnTimer();
	DllExport void incrementTurnTimer(int iNumTurnSlices);
	DllExport int getMaxTurnLen();

	DllExport int getTargetScore() const;																		// Exposed to Python
	DllExport void setTargetScore(int iNewValue);														// Exposed to Python

	DllExport int getNumGameTurnActive();																		// Exposed to Python
	DllExport int countNumHumanGameTurnActive();														// Exposed to Python
	DllExport void changeNumGameTurnActive(int iChange);

	DllExport int getNumCities();																						// Exposed to Python
	DllExport int getNumCivCities();																				// Exposed to Python
	DllExport void changeNumCities(int iChange);

	DllExport int getTotalPopulation() const;																// Exposed to Python
	DllExport void changeTotalPopulation(int iChange);

	DllExport int getTradeRoutes() const;																		// Exposed to Python
	DllExport void changeTradeRoutes(int iChange);													// Exposed to Python

	DllExport int getFreeTradeCount() const;																// Exposed to Python
	DllExport bool isFreeTrade() const;																			// Exposed to Python
	DllExport void changeFreeTradeCount(int iChange);												// Exposed to Python

	DllExport int getNoNukesCount() const;																	// Exposed to Python
	DllExport bool isNoNukes() const;																				// Exposed to Python
	DllExport void changeNoNukesCount(int iChange);													// Exposed to Python

	DllExport int getSecretaryGeneralTimer() const;													// Exposed to Python
	DllExport void setSecretaryGeneralTimer(int iNewValue);
	DllExport void changeSecretaryGeneralTimer(int iChange);

	DllExport int getNukesExploded() const;																	// Exposed to Python
	DllExport void changeNukesExploded(int iChange);												// Exposed to Python

	DllExport int getMaxPopulation() const;																	// Exposed to Python
	DllExport int getMaxLand() const;																				// Exposed to Python
	DllExport int getMaxTech() const;																				// Exposed to Python
	DllExport int getMaxWonders() const;																		// Exposed to Python
	DllExport int getInitPopulation() const;																// Exposed to Python
	DllExport int getInitLand() const;																			// Exposed to Python
	DllExport int getInitTech() const;																			// Exposed to Python
	DllExport int getInitWonders() const;																		// Exposed to Python
	DllExport void initScoreCalculation();

	DllExport int getAIAutoPlay();
	DllExport void setAIAutoPlay(int iNewValue);
	DllExport void changeAIAutoPlay(int iChange);

	DllExport unsigned int getInitialTime();
	DllExport void setInitialTime(unsigned int uiNewValue);

	bool isScoreDirty() const;																							// Exposed to Python
	void setScoreDirty(bool bNewValue);																			// Exposed to Python

	DllExport bool isCircumnavigated() const;																// Exposed to Python
	DllExport void makeCircumnavigated();																		// Exposed to Python

	DllExport bool isDiploVote() const;																			// Exposed to Python
	DllExport void makeDiploVote();																					// Exposed to Python

	DllExport bool isDebugMode() const;																			// Exposed to Python
	DllExport void toggleDebugMode();																				// Exposed to Python

	DllExport int getPitbossTurnTime();																			// Exposed to Python

	DllExport bool isHotSeat();																							// Exposed to Python

	DllExport bool isPbem();																								// Exposed to Python

	DllExport bool isPitboss();																							// Exposed to Python

	DllExport bool isFinalInitialized();																		// Exposed to Python
	DllExport void setFinalInitialized(bool bNewValue);

	DllExport bool getPbemTurnSent() const;
	DllExport void setPbemTurnSent(bool bNewValue);

	DllExport bool getHotPbemBetweenTurns() const;
	DllExport void setHotPbemBetweenTurns(bool bNewValue);

	DllExport bool isPlayerOptionsSent() const;
	DllExport void sendPlayerOptions(bool bForce = false);

	DllExport PlayerTypes getActivePlayer() const;																				// Exposed to Python
	DllExport void setActivePlayer(PlayerTypes eNewValue, bool bForceHotSeat = false);		// Exposed to Python

	DllExport HandicapTypes getHandicapType() const;
	DllExport void setHandicapType(HandicapTypes eHandicap);

	DllExport PlayerTypes getPausePlayer();																			// Exposed to Python
	DllExport bool isPaused();																									// Exposed to Python
	DllExport void setPausePlayer(PlayerTypes eNewValue);

	DllExport UnitTypes getBestLandUnit();																			// Exposed to Python
	DllExport int getBestLandUnitCombat();																			// Exposed to Python
	DllExport void setBestLandUnit(UnitTypes eNewValue);

	DllExport TeamTypes getWinner() const;																			// Exposed to Python
	DllExport VictoryTypes getVictory() const;																	// Exposed to Python
	DllExport void setWinner(TeamTypes eNewWinner, VictoryTypes eNewVictory);		// Exposed to Python

	DllExport GameStateTypes getGameState();																		// Exposed to Python
	DllExport void setGameState(GameStateTypes eNewValue);

	DllExport EraTypes getStartEra() const;																			// Exposed to Python

	DllExport CalendarTypes getCalendar() const;																// Exposed to Python

	DllExport GameSpeedTypes getGameSpeedType() const;													// Exposed to Python 

	DllExport int getEndTurnMessagesReceived(int iIndex);
	DllExport void incrementEndTurnMessagesReceived(int iIndex);

	DllExport PlayerTypes getRankPlayer(int iRank);															// Exposed to Python
	DllExport void setRankPlayer(int iRank, PlayerTypes ePlayer);

	DllExport int getPlayerRank(PlayerTypes ePlayer);														// Exposed to Python 
	DllExport void setPlayerRank(PlayerTypes ePlayer, int iRank);

	DllExport int getPlayerScore(PlayerTypes ePlayer);													// Exposed to Python
	DllExport void setPlayerScore(PlayerTypes ePlayer, int iScore);

	DllExport TeamTypes getRankTeam(int iRank);																	// Exposed to Python
	DllExport void setRankTeam(int iRank, TeamTypes eTeam);

	DllExport int getTeamRank(TeamTypes eTeam);																	// Exposed to Python
	DllExport void setTeamRank(TeamTypes eTeam, int iRank);

	DllExport int getTeamScore(TeamTypes eTeam);																// Exposed to Python
	DllExport void setTeamScore(TeamTypes eTeam, int iScore);

	DllExport bool isOption(GameOptionTypes eIndex) const;																// Exposed to Python
	DllExport void setOption(GameOptionTypes eIndex, bool bEnabled);

	DllExport bool isMPOption(MultiplayerOptionTypes eIndex) const;												// Exposed to Python
	DllExport void setMPOption(MultiplayerOptionTypes eIndex, bool bEnabled);

	DllExport bool isForcedControl(ForceControlTypes eIndex) const;												// Exposed to Python
	DllExport void setForceControl(ForceControlTypes eIndex, bool bEnabled);

	DllExport int getUnitCreatedCount(UnitTypes eIndex);																	// Exposed to Python
	DllExport void incrementUnitCreatedCount(UnitTypes eIndex);

	DllExport int getUnitClassCreatedCount(UnitClassTypes eIndex);												// Exposed to Python
	DllExport bool isUnitClassMaxedOut(UnitClassTypes eIndex, int iExtra = 0);						// Exposed to Python
	DllExport void incrementUnitClassCreatedCount(UnitClassTypes eIndex);

	DllExport int getBuildingClassCreatedCount(BuildingClassTypes eIndex);								// Exposed to Python
	DllExport bool isBuildingClassMaxedOut(BuildingClassTypes eIndex, int iExtra = 0);		// Exposed to Python
	DllExport void incrementBuildingClassCreatedCount(BuildingClassTypes eIndex);

	DllExport int getProjectCreatedCount(ProjectTypes eIndex);														// Exposed to Python
	DllExport bool isProjectMaxedOut(ProjectTypes eIndex, int iExtra = 0);								// Exposed to Python
	DllExport void incrementProjectCreatedCount(ProjectTypes eIndex);

	DllExport int getForceCivicCount(CivicTypes eIndex) const;														// Exposed to Python
	DllExport bool isForceCivic(CivicTypes eIndex) const;																	// Exposed to Python
	DllExport bool isForceCivicOption(CivicOptionTypes eCivicOption) const;								// Exposed to Python
	DllExport void changeForceCivicCount(CivicTypes eIndex, int iChange);

	DllExport int getVoteOutcome(VoteTypes eIndex) const;																	// Exposed to Python
	DllExport bool isVotePassed(VoteTypes eIndex) const;																	// Exposed to Python
	DllExport void setVoteOutcome(VoteTypes eIndex, int iNewValue);

	DllExport bool isVictoryValid(VictoryTypes eIndex) const;															// Exposed to Python
	DllExport void setVictoryValid(VictoryTypes eIndex, bool bValid);

	DllExport bool isSpecialUnitValid(SpecialUnitTypes eIndex);														// Exposed to Python  
	DllExport void makeSpecialUnitValid(SpecialUnitTypes eIndex);													// Exposed to Python

	DllExport bool isSpecialBuildingValid(SpecialBuildingTypes eIndex);										// Exposed to Python
	DllExport void makeSpecialBuildingValid(SpecialBuildingTypes eIndex);									// Exposed to Python

	DllExport bool isVoteTriggered(VoteTypes eIndex);																			// Exposed to Python
	DllExport void setVoteTriggered(VoteTypes eIndex, bool bNewValue);

	DllExport int getReligionGameTurnFounded(ReligionTypes eIndex);												// Exposed to Python
	DllExport bool isReligionFounded(ReligionTypes eIndex);																// Exposed to Python
	DllExport void makeReligionFounded(ReligionTypes eIndex, PlayerTypes ePlayer);

	DllExport CvCity* getHolyCity(ReligionTypes eIndex);																	// Exposed to Python
	DllExport void setHolyCity(ReligionTypes eIndex, CvCity* pNewValue, bool bAnnounce);	// Exposed to Python

	DllExport int getPlayerVote(PlayerTypes eOwnerIndex, VoteTypes eVoteIndex) const;			// Exposed to Python
	DllExport void setPlayerVote(PlayerTypes eOwnerIndex, VoteTypes eVoteIndex, int iNewValue);
	DllExport void castVote(PlayerTypes eOwnerIndex, VoteTypes eVoteIndex, int iVote);

	DllExport const CvWString & getName();
	DllExport void setName(const TCHAR* szName);

	// Script data needs to be a narrow string for pickling in Python
	std::string getScriptData() const;																										// Exposed to Python	
	void setScriptData(std::string szNewValue);																						// Exposed to Python	
																																												
	DllExport bool isDestroyedCityName(CvWString& szName) const;													
	DllExport void addDestroyedCityName(const CvWString& szName);													
																																												
	DllExport int getIndexAfterLastDeal();																								// Exposed to Python	
	DllExport int getNumDeals();																													// Exposed to Python	
	DllExport CvDeal* getDeal(int iID);																										// Exposed to Python	
	DllExport CvDeal* addDeal();																													
	DllExport void deleteDeal(int iID);																										
	// iteration																																					
	DllExport CvDeal* firstDeal(int *pIterIdx, bool bRev=false);													// Exposed to Python									
	DllExport CvDeal* nextDeal(int *pIterIdx, bool bRev=false);														// Exposed to Python									
																																												
	DllExport CvRandom& getMapRand();																											// Exposed to Python	
	DllExport int getMapRandNum(int iNum, const char* pszLog);														
																																												
	DllExport CvRandom& getSorenRand();																										// Exposed to Python	
	DllExport int getSorenRandNum(int iNum, const char* pszLog);													
																																												
	DllExport int calculateSyncChecksum();																								// Exposed to Python	
	DllExport int calculateOptionsChecksum();																							// Exposed to Python	

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
	int m_iSecretaryGeneralTimer;
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
	bool m_bDiploVote;
	bool m_bDebugMode;
	bool m_bFinalInitialized;
	bool m_bPbemTurnSent;
	bool m_bHotPbemBetweenTurns;
	bool m_bPlayerOptionsSent;

	HandicapTypes m_eHandicap;
	PlayerTypes m_ePausePlayer;
	UnitTypes m_eBestLandUnit;
	TeamTypes m_eWinner;
	VictoryTypes m_eVictory;
	GameStateTypes m_eGameState;

	CvString m_szScriptData;

	int m_aiEndTurnMessagesReceived[MAX_PLAYERS];
	int m_aiRankPlayer[MAX_PLAYERS];        // Ordered by rank...
	int m_aiPlayerRank[MAX_PLAYERS];        // Ordered by player ID...
	int m_aiPlayerScore[MAX_PLAYERS];       // Ordered by player ID...
	int m_aiRankTeam[MAX_TEAMS];						// Ordered by rank...
	int m_aiTeamRank[MAX_TEAMS];						// Ordered by team ID...
	int m_aiTeamScore[MAX_TEAMS];						// Ordered by team ID...

	int* m_paiUnitCreatedCount;
	int* m_paiUnitClassCreatedCount;
	int* m_paiBuildingClassCreatedCount;
	int* m_paiProjectCreatedCount;
	int* m_paiForceCivicCount;
	int* m_paiVoteOutcome;
	int* m_paiReligionGameTurnFounded;

	bool* m_pabSpecialUnitValid;
	bool* m_pabSpecialBuildingValid;
	bool* m_pabVoteTriggered;

	IDInfo* m_paHolyCity;

	int* m_apaiPlayerVote[MAX_CIV_PLAYERS];

	CLinkList<CvWString> m_destroyedCities;

	FFreeListTrashArray<CvDeal> m_deals;

	CvRandom m_mapRand;
	CvRandom m_sorenRand;

	ReplayMessageList m_listReplayMessages; 
	CvReplayInfo* m_pReplayInfo;

	int m_iNumSessions;

	void doTurn();
	void doDeals();
	void doGlobalWarming();
	void doHolyCity();
	void doDiploVote();

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

	void processVote(VoteTypes eVote, int iChange);

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

};

#endif
