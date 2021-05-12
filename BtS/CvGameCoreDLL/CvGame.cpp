// game.cpp

#include "CvGameCoreDLL.h"
#include "CvGameCoreUtils.h"
#include "CvGameCoreUtils.h"
#include "CvGame.h"
#include "CvGameAI.h"
#include "CvMap.h"
#include "CvPlot.h"
#include "CvPlayerAI.h"
#include "CvRandom.h"
#include "CvTeamAI.h"
#include "CvGlobals.h"
#include "CvInitCore.h"
#include "CvMapGenerator.h"
#include "CvArtFileMgr.h"
#include "CvDiploParameters.h"
#include "CvReplayMessage.h"
#include "CyArgsList.h"
#include "CvInfos.h"
#include "CvPopupInfo.h"
#include "FProfiler.h"
#include "CvReplayInfo.h"
#include "CyPlot.h"
#include "CvGameTextMgr.h"

// interface uses
#include "CvDLLInterfaceIFaceBase.h"
#include "CvDLLEngineIFaceBase.h"
#include "CvDLLEventReporterIFaceBase.h"
#include "CvDLLPythonIFaceBase.h"

// for testing only
#ifndef FINAL_RELEASE
//#define DEBUG_OUT_OF_SYNCS
#endif


// Public Functions...

CvGame::CvGame()
{
	int iI;

	m_aiEndTurnMessagesReceived = new int[MAX_PLAYERS];
	m_aiRankPlayer = new int[MAX_PLAYERS];        // Ordered by rank...
	m_aiPlayerRank = new int[MAX_PLAYERS];        // Ordered by player ID...
	m_aiPlayerScore = new int[MAX_PLAYERS];       // Ordered by player ID...
	m_aiRankTeam = new int[MAX_TEAMS];						// Ordered by rank...
	m_aiTeamRank = new int[MAX_TEAMS];						// Ordered by team ID...
	m_aiTeamScore = new int[MAX_TEAMS];						// Ordered by team ID...

	m_apaiPlayerVote = new int*[MAX_CIV_PLAYERS];

	m_paiUnitCreatedCount = NULL;
	m_paiUnitClassCreatedCount = NULL;
	m_paiBuildingClassCreatedCount = NULL;
	m_paiProjectCreatedCount = NULL;
	m_paiForceCivicCount = NULL;
	m_paiVoteOutcome = NULL;
	m_paiReligionGameTurnFounded = NULL;

	m_pabSpecialUnitValid = NULL;
	m_pabSpecialBuildingValid = NULL;
	m_pabVoteTriggered = NULL;

	m_paHolyCity = NULL;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		m_apaiPlayerVote[iI] = NULL;
	}

	m_pReplayInfo = NULL;

	constructCache();

	reset(NO_HANDICAP, true);
}


CvGame::~CvGame()
{
	uninit();

	destructCache();

	SAFE_DELETE_ARRAY(m_aiEndTurnMessagesReceived);
	SAFE_DELETE_ARRAY(m_aiRankPlayer);
	SAFE_DELETE_ARRAY(m_aiPlayerRank);
	SAFE_DELETE_ARRAY(m_aiPlayerScore);
	SAFE_DELETE_ARRAY(m_aiRankTeam);
	SAFE_DELETE_ARRAY(m_aiTeamRank);
	SAFE_DELETE_ARRAY(m_aiTeamScore);
	SAFE_DELETE_ARRAY(m_apaiPlayerVote);
}

void CvGame::init(HandicapTypes eHandicap)
{
	bool bValid;
	int iStartTurn;
	int iEstimateEndTurn;
	int iI;

	//--------------------------------
	// Init saved data
	reset(eHandicap);

	//--------------------------------
	// Init containers
	m_deals.init();

	m_mapRand.init(GC.getInitCore().getMapRandSeed() % 73637381);
	m_sorenRand.init(GC.getInitCore().getSyncRandSeed() % 52319761);

	//--------------------------------
	// Init non-saved data

	//--------------------------------
	// Init other game data

	// Turn off all MP options if it's a single player game
	if (GC.getInitCore().getType() == GAME_SP_NEW ||
		GC.getInitCore().getType() == GAME_SP_SCENARIO)
	{
		for (iI = 0; iI < NUM_MPOPTION_TYPES; ++iI)
		{
			setMPOption((MultiplayerOptionTypes)iI, false);
		}
	}

	// If this is a hot seat game, simultaneous turns is always off
	if (isHotSeat() || isPbem())
	{
		setMPOption(MPOPTION_SIMULTANEOUS_TURNS, false);
	}
	// If we didn't set a time in the Pitboss, turn timer off
	if (isPitboss() && getPitbossTurnTime() == 0)
	{
		setMPOption(MPOPTION_TURN_TIMER, false);
	}

	if (isMPOption(MPOPTION_SHUFFLE_TEAMS))
	{
		int aiTeams[MAX_CIV_PLAYERS];

		int iNumPlayers = 0;
		for (int i = 0; i < MAX_CIV_PLAYERS; i++)
		{
			if (GC.getInitCore().getSlotStatus((PlayerTypes)i) == SS_TAKEN)
			{
				aiTeams[iNumPlayers] = GC.getInitCore().getTeam((PlayerTypes)i);
				++iNumPlayers;
			}
		}

		for (int i = 0; i < iNumPlayers; i++)
		{
			int j = (getSorenRand().get(iNumPlayers - i, NULL) + i);

			if (i != j)
			{
				int iTemp = aiTeams[i];
				aiTeams[i] = aiTeams[j];
				aiTeams[j] = iTemp;
			}
		}

		iNumPlayers = 0;
		for (int i = 0; i < MAX_CIV_PLAYERS; i++)
		{
			if (GC.getInitCore().getSlotStatus((PlayerTypes)i) == SS_TAKEN)
			{
				GC.getInitCore().setTeam((PlayerTypes)i, (TeamTypes)aiTeams[iNumPlayers]);
				++iNumPlayers;
			}
		}
	}

	if (isOption(GAMEOPTION_LOCK_MODS))
	{
		if (isGameMultiPlayer())
		{
			setOption(GAMEOPTION_LOCK_MODS, false);
		}
		else
		{
			static const int iPasswordSize = 8;
			char szRandomPassword[iPasswordSize];
			for (int i = 0; i < iPasswordSize-1; i++)
			{
				szRandomPassword[i] = getSorenRandNum(128, NULL);
			}
			szRandomPassword[iPasswordSize-1] = 0;

			GC.getInitCore().setAdminPassword(szRandomPassword);
		}
	}

	if (getGameTurn() == 0)
	{
		iStartTurn = 0;

		for (iI = 0; iI < GC.getGameSpeedInfo(getGameSpeedType()).getNumTurnIncrements(); iI++)
		{
			iStartTurn += GC.getGameSpeedInfo(getGameSpeedType()).getGameTurnInfo(iI).iNumGameTurnsPerIncrement;
		}

		iStartTurn *= GC.getEraInfo(getStartEra()).getStartPercent();
		iStartTurn /= 100;

		setGameTurn(iStartTurn);
	}

	setStartTurn(getGameTurn());

	if (getMaxTurns() == 0)
	{
		iEstimateEndTurn = 0;

		for (iI = 0; iI < GC.getGameSpeedInfo(getGameSpeedType()).getNumTurnIncrements(); iI++)
		{
			iEstimateEndTurn += GC.getGameSpeedInfo(getGameSpeedType()).getGameTurnInfo(iI).iNumGameTurnsPerIncrement;
		}

		setEstimateEndTurn(iEstimateEndTurn);

		if (getEstimateEndTurn() > getGameTurn())
		{
			bValid = false;

			for (iI = 0; iI < GC.getNumVictoryInfos(); iI++)
			{
				if (isVictoryValid((VictoryTypes)iI))
				{
					if (GC.getVictoryInfo((VictoryTypes)iI).isEndScore())
					{
						bValid = true;
						break;
					}
				}
			}

			if (bValid)
			{
				setMaxTurns(getEstimateEndTurn() - getGameTurn());
			}
		}
	}
	else
	{
		setEstimateEndTurn(getGameTurn() + getMaxTurns());
	}

	setStartYear(GC.getDefineINT("START_YEAR"));

	for (iI = 0; iI < GC.getNumSpecialUnitInfos(); iI++)
	{
		if (GC.getSpecialUnitInfo((SpecialUnitTypes)iI).isValid())
		{
			makeSpecialUnitValid((SpecialUnitTypes)iI);
		}
	}

	for (iI = 0; iI < GC.getNumSpecialBuildingInfos(); iI++)
	{
		if (GC.getSpecialBuildingInfo((SpecialBuildingTypes)iI).isValid())
		{
			makeSpecialBuildingValid((SpecialBuildingTypes)iI);
		}
	}

	AI_init();

	initCache();
}

//
// Set initial items (units, techs, etc...)
//
void CvGame::setInitialItems()
{
	PROFILE_FUNC();

	initFreeState();
	assignStartingPlots();
	normalizeStartingPlots();
	initFreeUnits();
}


void CvGame::regenerateMap()
{
	int iI;

	if (GC.getInitCore().getWBMapScript())
	{
		return;
	}

	setFinalInitialized(false);

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		GET_PLAYER((PlayerTypes)iI).killUnits();
	}

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		GET_PLAYER((PlayerTypes)iI).killCities();
	}

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		GET_PLAYER((PlayerTypes)iI).killAllDeals();
	}

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		GET_PLAYER((PlayerTypes)iI).setFoundedFirstCity(false);
		GET_PLAYER((PlayerTypes)iI).setStartingPlot(NULL, false);
	}

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		GC.getMapINLINE().setRevealedPlots(((TeamTypes)iI), false);
	}

	GC.getMapINLINE().erasePlots();

	CvMapGenerator::GetInstance().generateRandomMap();
	CvMapGenerator::GetInstance().addGameElements();

	gDLL->getEngineIFace()->RebuildAllPlots();

	gDLL->resetStatistics();

	setInitialItems();

	initScoreCalculation();
	setFinalInitialized(true);

	GC.getMapINLINE().setupGraphical();
	gDLL->getEngineIFace()->SetDirty(GlobeTexture_DIRTY_BIT, true);
	gDLL->getEngineIFace()->SetDirty(MinimapTexture_DIRTY_BIT, true);

	gDLL->getInterfaceIFace()->setCycleSelectionCounter(1);

	gDLL->getEngineIFace()->AutoSave(true);
}


void CvGame::uninit()
{
	int iI;

	uninitCache();

	SAFE_DELETE_ARRAY(m_paiUnitCreatedCount);
	SAFE_DELETE_ARRAY(m_paiUnitClassCreatedCount);
	SAFE_DELETE_ARRAY(m_paiBuildingClassCreatedCount);
	SAFE_DELETE_ARRAY(m_paiProjectCreatedCount);
	SAFE_DELETE_ARRAY(m_paiForceCivicCount);
	SAFE_DELETE_ARRAY(m_paiVoteOutcome);
	SAFE_DELETE_ARRAY(m_paiReligionGameTurnFounded);

	SAFE_DELETE_ARRAY(m_pabSpecialUnitValid);
	SAFE_DELETE_ARRAY(m_pabSpecialBuildingValid);
	SAFE_DELETE_ARRAY(m_pabVoteTriggered);

	SAFE_DELETE_ARRAY(m_paHolyCity);

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		SAFE_DELETE_ARRAY(m_apaiPlayerVote[iI]);
	}

	m_aszDestroyedCities.clear();
	m_aszGreatPeopleBorn.clear();

	m_deals.uninit();

	m_mapRand.uninit();
	m_sorenRand.uninit();

	clearReplayMessageMap();
	SAFE_DELETE(m_pReplayInfo);
}


// FUNCTION: reset()
// Initializes data members that are serialized.
void CvGame::reset(HandicapTypes eHandicap, bool bConstructorCall)
{
	int iI, iJ;

	//--------------------------------
	// Uninit class
	uninit();

	m_iEndTurnMessagesSent = 0;
	m_iElapsedGameTurns = 0;
	m_iStartTurn = 0;
	m_iStartYear = 0;
	m_iEstimateEndTurn = 0;
	m_iTurnSlice = 0;
	m_iCutoffSlice = 0;
	m_iNumGameTurnActive = 0;
	m_iNumCities = 0;
	m_iTotalPopulation = 0;
	m_iTradeRoutes = 0;
	m_iFreeTradeCount = 0;
	m_iNoNukesCount = 0;
	m_iSecretaryGeneralTimer = 0;
	m_iNukesExploded = 0;
	m_iMaxPopulation = 0;
	m_iMaxLand = 0;
	m_iMaxTech = 0;
	m_iMaxWonders = 0;
	m_iInitPopulation = 0;
	m_iInitLand = 0;
	m_iInitTech = 0;
	m_iInitWonders = 0;
	m_iAIAutoPlay = 0;

	m_uiInitialTime = 0;

	m_bScoreDirty = false;
	m_bCircumnavigated = false;
	m_bDiploVote = false;
	m_bDebugMode = false;
	m_bFinalInitialized = false;
	m_bPbemTurnSent = false;
	m_bHotPbemBetweenTurns = false;
	m_bPlayerOptionsSent = false;

	m_eHandicap = eHandicap;
	m_ePausePlayer = NO_PLAYER;
	m_eBestLandUnit = NO_UNIT;
	m_eWinner = NO_TEAM;
	m_eVictory = NO_VICTORY;
	m_eGameState = GAMESTATE_ON;

	m_szScriptData = "";

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		m_aiEndTurnMessagesReceived[iI] = 0;
		m_aiRankPlayer[iI] = 0;
		m_aiPlayerRank[iI] = 0;
		m_aiPlayerScore[iI] = 0;
	}

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		m_aiRankTeam[iI] = 0;
		m_aiTeamRank[iI] = 0;
		m_aiTeamScore[iI] = 0;
	}

	if (!bConstructorCall)
	{
		FAssertMsg(m_paiUnitCreatedCount==NULL, "about to leak memory, CvGame::m_paiUnitCreatedCount");
		m_paiUnitCreatedCount = new int[GC.getNumUnitInfos()];
		for (iI = 0; iI < GC.getNumUnitInfos(); iI++)
		{
			m_paiUnitCreatedCount[iI] = 0;
		}

		FAssertMsg(m_paiUnitClassCreatedCount==NULL, "about to leak memory, CvGame::m_paiUnitClassCreatedCount");
		m_paiUnitClassCreatedCount = new int[GC.getNumUnitClassInfos()];
		for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
		{
			m_paiUnitClassCreatedCount[iI] = 0;
		}

		FAssertMsg(m_paiBuildingClassCreatedCount==NULL, "about to leak memory, CvGame::m_paiBuildingClassCreatedCount");
		m_paiBuildingClassCreatedCount = new int[GC.getNumBuildingClassInfos()];
		for (iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
		{
			m_paiBuildingClassCreatedCount[iI] = 0;
		}

		FAssertMsg(m_paiProjectCreatedCount==NULL, "about to leak memory, CvGame::m_paiProjectCreatedCount");
		m_paiProjectCreatedCount = new int[GC.getNumProjectInfos()];
		for (iI = 0; iI < GC.getNumProjectInfos(); iI++)
		{
			m_paiProjectCreatedCount[iI] = 0;
		}

		FAssertMsg(m_paiForceCivicCount==NULL, "about to leak memory, CvGame::m_paiForceCivicCount");
		m_paiForceCivicCount = new int[GC.getNumCivicInfos()];
		for (iI = 0; iI < GC.getNumCivicInfos(); iI++)
		{
			m_paiForceCivicCount[iI] = 0;
		}

		FAssertMsg(0 < GC.getNumVoteInfos(), "GC.getNumVoteInfos() is not greater than zero in CvGame::reset");
		FAssertMsg(m_paiVoteOutcome==NULL, "about to leak memory, CvGame::m_paiVoteOutcome");
		m_paiVoteOutcome = new int[GC.getNumVoteInfos()];
		m_pabVoteTriggered = new bool[GC.getNumVoteInfos()];
		for (iI = 0; iI < GC.getNumVoteInfos(); iI++)
		{
			m_paiVoteOutcome[iI] = -1;
			m_pabVoteTriggered[iI] = false;
		}

		FAssertMsg(m_pabSpecialUnitValid==NULL, "about to leak memory, CvGame::m_pabSpecialUnitValid");
		m_pabSpecialUnitValid = new bool[GC.getNumSpecialUnitInfos()];
		for (iI = 0; iI < GC.getNumSpecialUnitInfos(); iI++)
		{
			m_pabSpecialUnitValid[iI] = false;
		}

		FAssertMsg(m_pabSpecialBuildingValid==NULL, "about to leak memory, CvGame::m_pabSpecialBuildingValid");
		m_pabSpecialBuildingValid = new bool[GC.getNumSpecialBuildingInfos()];
		for (iI = 0; iI < GC.getNumSpecialBuildingInfos(); iI++)
		{
			m_pabSpecialBuildingValid[iI] = false;
		}

		FAssertMsg(0 < GC.getNumReligionInfos(), "GC.getNumReligionInfos() is not greater than zero in CvGame::reset");
		FAssertMsg(m_paiReligionGameTurnFounded==NULL, "about to leak memory, CvGame::m_paiReligionGameTurnFounded");
		m_paiReligionGameTurnFounded = new int[GC.getNumReligionInfos()];
		m_paHolyCity = new IDInfo[GC.getNumReligionInfos()];
		for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
		{
			m_paiReligionGameTurnFounded[iI] = -1;
			m_paHolyCity[iI].reset();
		}

		for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
		{
			FAssertMsg(m_apaiPlayerVote[iI]==NULL, "about to leak memory, CvGame::m_apaiPlayerVote");
			FAssertMsg((0 < GC.getNumVoteInfos()), "GC.getNumVoteInfos() is not greater than zero but an array is being allocated in CvGame::reset");
			m_apaiPlayerVote[iI] = new int[GC.getNumVoteInfos()];
			for (iJ = 0; iJ < GC.getNumVoteInfos(); iJ++)
			{
				m_apaiPlayerVote[iI][iJ] = 0;
			}
		}
	}

	m_deals.removeAll();

	m_mapRand.reset();
	m_sorenRand.reset();

	m_iNumSessions = 1;

	if (!bConstructorCall)
	{
		AI_reset();
	}

	resetCache(bConstructorCall);
}


void CvGame::initDiplomacy()
{
	PROFILE_FUNC();

	int iI, iJ;

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		GET_TEAM((TeamTypes)iI).meet(((TeamTypes)iI), false);

		if (GET_TEAM((TeamTypes)iI).isBarbarian() || GET_TEAM((TeamTypes)iI).isMinorCiv())
		{
			for (iJ = 0; iJ < MAX_CIV_TEAMS; iJ++)
			{
				if (iI != iJ)
				{
					GET_TEAM((TeamTypes)iI).declareWar(((TeamTypes)iJ), false);
				}
			}
		}
	}
}


void CvGame::initFreeState()
{
	bool bValid;
	int iI, iJ, iK;

	for (iI = 0; iI < GC.getNumTechInfos(); iI++)
	{
		for (iJ = 0; iJ < MAX_TEAMS; iJ++)
		{
			if (GET_TEAM((TeamTypes)iJ).isAlive())
			{
				bValid = false;

				if (!bValid)
				{
					if ((GC.getHandicapInfo(getHandicapType()).isFreeTechs(iI)) ||
						  (!(GET_TEAM((TeamTypes)iJ).isHuman())&& GC.getHandicapInfo(getHandicapType()).isAIFreeTechs(iI)) ||
						  (GC.getTechInfo((TechTypes)iI).getEra() < getStartEra()))
					{
						bValid = true;
					}
				}

				if (!bValid)
				{
					for (iK = 0; iK < MAX_PLAYERS; iK++)
					{
						if (GET_PLAYER((PlayerTypes)iK).isAlive())
						{
							if (GET_PLAYER((PlayerTypes)iK).getTeam() == iJ)
							{
								if (GC.getCivilizationInfo(GET_PLAYER((PlayerTypes)iK).getCivilizationType()).isCivilizationFreeTechs(iI))
								{
									bValid = true;
									break;
								}
							}
						}
					}
				}

				GET_TEAM((TeamTypes)iJ).setHasTech(((TechTypes)iI), bValid, NO_PLAYER, false, false);
			}
		}
	}

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			GET_PLAYER((PlayerTypes)iI).initFreeState();
		}
	}
}


void CvGame::initFreeUnits()
{
	int iI;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if ((GET_PLAYER((PlayerTypes)iI).getNumUnits() == 0) && (GET_PLAYER((PlayerTypes)iI).getNumCities() == 0))
			{
				GET_PLAYER((PlayerTypes)iI).initFreeUnits();
			}
		}
	}
}


void CvGame::assignStartingPlots()
{
	PROFILE_FUNC();

	CvPlot* pPlot;
	CvPlot* pBestPlot;
	bool bStartFound;
	bool bValid;
	int iRandOffset;
	int iLoopTeam;
	int iLoopPlayer;
	int iHumanSlot;
	int iValue;
	int iBestValue;
	int iI, iJ, iK;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getStartingPlot() == NULL)
			{
				iBestValue = 0;
				pBestPlot = NULL;

				for (iJ = 0; iJ < GC.getMapINLINE().numPlotsINLINE(); iJ++)
				{
					gDLL->callUpdater();	// allow window updates during launch

					pPlot = GC.getMapINLINE().plotByIndexINLINE(iJ);

					if (pPlot->isStartingPlot())
					{
						bValid = true;

						for (iK = 0; iK < MAX_CIV_PLAYERS; iK++)
						{
							if (GET_PLAYER((PlayerTypes)iK).isAlive())
							{
								if (GET_PLAYER((PlayerTypes)iK).getStartingPlot() == pPlot)
								{
									bValid = false;
									break;
								}
							}
						}

						if (bValid)
						{
							iValue = (1 + getSorenRandNum(1000, "Starting Plot"));

							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								pBestPlot = pPlot;
							}
						}
					}
				}

				if (pBestPlot != NULL)
				{
					GET_PLAYER((PlayerTypes)iI).setStartingPlot(pBestPlot, true);
				}
			}
		}
	}

	if (gDLL->getPythonIFace()->pythonAssignStartingPlots() && !gDLL->getPythonIFace()->pythonUsingDefaultImpl())
	{
		return; // Python override
	}

	if (isTeamGame())
	{
		while (true)
		{
			bStartFound = false;

			iRandOffset = getSorenRandNum(countCivTeamsAlive(), "Team Starting Plot");

			for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
			{
				iLoopTeam = ((iI + iRandOffset) % MAX_CIV_TEAMS);

				if (GET_TEAM((TeamTypes)iLoopTeam).isAlive())
				{
					for (iJ = 0; iJ < MAX_CIV_PLAYERS; iJ++)
					{
						if (GET_PLAYER((PlayerTypes)iJ).isAlive())
						{
							if (GET_PLAYER((PlayerTypes)iJ).getTeam() == iLoopTeam)
							{
								if (GET_PLAYER((PlayerTypes)iJ).getStartingPlot() == NULL)
								{
									GET_PLAYER((PlayerTypes)iJ).setStartingPlot(GET_PLAYER((PlayerTypes)iJ).findStartingPlot(), true);
									bStartFound = true;
									break;
								}
							}
						}
					}
				}
			}

			if (!bStartFound)
			{
				break;
			}

			iRandOffset++;
		}
	}
	else if (isGameMultiPlayer())
	{
		iRandOffset = getSorenRandNum(countCivPlayersAlive(), "Player Starting Plot");

		for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
		{
			iLoopPlayer = ((iI + iRandOffset) % MAX_CIV_PLAYERS);

			if (GET_PLAYER((PlayerTypes)iLoopPlayer).isAlive())
			{
				if (GET_PLAYER((PlayerTypes)iLoopPlayer).isHuman())
				{
					if (GET_PLAYER((PlayerTypes)iLoopPlayer).getStartingPlot() == NULL)
					{
						GET_PLAYER((PlayerTypes)iLoopPlayer).setStartingPlot(GET_PLAYER((PlayerTypes)iLoopPlayer).findStartingPlot(), true);
					}
				}
			}
		}

		for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if (!(GET_PLAYER((PlayerTypes)iI).isHuman()))
				{
					if (GET_PLAYER((PlayerTypes)iI).getStartingPlot() == NULL)
					{
						GET_PLAYER((PlayerTypes)iI).setStartingPlot(GET_PLAYER((PlayerTypes)iI).findStartingPlot(), true);
					}
				}
			}
		}
	}
	else
	{
		iHumanSlot = range((((countCivPlayersAlive() - 1) * GC.getHandicapInfo(getHandicapType()).getStartingLocationPercent()) / 100), 0, (countCivPlayersAlive() - 1));

		for (iI = 0; iI < iHumanSlot; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if (!(GET_PLAYER((PlayerTypes)iI).isHuman()))
				{
					if (GET_PLAYER((PlayerTypes)iI).getStartingPlot() == NULL)
					{
						GET_PLAYER((PlayerTypes)iI).setStartingPlot(GET_PLAYER((PlayerTypes)iI).findStartingPlot(), true);
					}
				}
			}
		}

		for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if (GET_PLAYER((PlayerTypes)iI).isHuman())
				{
					if (GET_PLAYER((PlayerTypes)iI).getStartingPlot() == NULL)
					{
						GET_PLAYER((PlayerTypes)iI).setStartingPlot(GET_PLAYER((PlayerTypes)iI).findStartingPlot(), true);
					}
				}
			}
		}

		for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if (GET_PLAYER((PlayerTypes)iI).getStartingPlot() == NULL)
				{
					GET_PLAYER((PlayerTypes)iI).setStartingPlot(GET_PLAYER((PlayerTypes)iI).findStartingPlot(), true);
				}
			}
		}
	}
}

// Swaps starting locations until we have reached the optimal closeness between teams
// (caveat: this isn't quite "optimal" because we could get stuck in local minima, but it's pretty good)

void CvGame::normalizeStartingPlotLocations()
{
	CvPlot* apNewStartPlots[MAX_CIV_PLAYERS];
	int* aaiDistances[MAX_CIV_PLAYERS];
	int aiStartingLocs[MAX_CIV_PLAYERS];
	int iI, iJ;

	// Precalculate distances between all starting positions:
	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			gDLL->callUpdater();	// allow window to update during launch
			aaiDistances[iI] = new int[iI];
			for (iJ = 0; iJ < iI; iJ++)
			{
				aaiDistances[iI][iJ] = 0;
			}
			CvPlot *pPlotI = GET_PLAYER((PlayerTypes)iI).getStartingPlot();	
			if (pPlotI != NULL)
			{
				for (iJ = 0; iJ < iI; iJ++)
				{
					if (GET_PLAYER((PlayerTypes)iJ).isAlive())
					{
						CvPlot *pPlotJ = GET_PLAYER((PlayerTypes)iJ).getStartingPlot();
						if (pPlotJ != NULL)
						{
							int iDist = GC.getMapINLINE().calculatePathDistance(pPlotI, pPlotJ);
							if (iDist == -1)
							{
								// 5x penalty for not being on the same area, or having no passable route
								iDist = 5*plotDistance(pPlotI->getX_INLINE(), pPlotI->getY_INLINE(), pPlotJ->getX_INLINE(), pPlotJ->getY_INLINE());
							}
							aaiDistances[iI][iJ] = iDist;
						}
					}
				}
			}
		}
		else
		{
			aaiDistances[iI] = NULL;
		}
	}

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		aiStartingLocs[iI] = iI; // each player starting in own location
	}

	int iBestScore = getTeamClosenessScore(aaiDistances, aiStartingLocs);
	bool bFoundSwap = true;
	while (bFoundSwap)
	{
		bFoundSwap = false;
		for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				for (iJ = 0; iJ < iI; iJ++)
				{
					if (GET_PLAYER((PlayerTypes)iJ).isAlive())
					{
						int iTemp = aiStartingLocs[iI];
						aiStartingLocs[iI] = aiStartingLocs[iJ];
						aiStartingLocs[iJ] = iTemp;
						int iScore = getTeamClosenessScore(aaiDistances, aiStartingLocs);
						if (iScore < iBestScore)
						{
							iBestScore = iScore;
							bFoundSwap = true;
						}
						else
						{
							// Swap them back:
							iTemp = aiStartingLocs[iI];
							aiStartingLocs[iI] = aiStartingLocs[iJ];
							aiStartingLocs[iJ] = iTemp;
						}
					}
				}
			}
		}
	} 

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		apNewStartPlots[iI] = NULL;
	} 

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (aiStartingLocs[iI] != iI)
			{
				apNewStartPlots[iI] = GET_PLAYER((PlayerTypes)aiStartingLocs[iI]).getStartingPlot();
			}
		}
	}

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (apNewStartPlots[iI] != NULL)
			{
				GET_PLAYER((PlayerTypes)iI).setStartingPlot(apNewStartPlots[iI], false);
			}
		}
	}

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		SAFE_DELETE_ARRAY(aaiDistances[iI]);
	}
}


void CvGame::normalizeAddRiver()
{
	CvPlot* pStartingPlot;
	int iI;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			pStartingPlot = GET_PLAYER((PlayerTypes)iI).getStartingPlot();

			if (pStartingPlot != NULL)
			{
				if (!pStartingPlot->isFreshWater())
				{
					// if we will be able to add a lake, then use old river code
					if (normalizeFindLakePlot((PlayerTypes)iI) != NULL)
					{
						CvMapGenerator::GetInstance().doRiver(pStartingPlot);
					}
					// otherwise, use new river code which is much more likely to succeed
					else
					{
						CvMapGenerator::GetInstance().addRiver(pStartingPlot);
					}
				}
			}
		}
	}
}


void CvGame::normalizeRemovePeaks()
{
	CvPlot* pStartingPlot;
	CvPlot* pLoopPlot;
	int iRange;
	int iDX, iDY;
	int iI;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			pStartingPlot = GET_PLAYER((PlayerTypes)iI).getStartingPlot();

			if (pStartingPlot != NULL)
			{
				iRange = 3;

				for (iDX = -(iRange); iDX <= iRange; iDX++)
				{
					for (iDY = -(iRange); iDY <= iRange; iDY++)
					{
						pLoopPlot = plotXY(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), iDX, iDY);

						if (pLoopPlot != NULL)
						{
							if (pLoopPlot->isPeak())
							{
								pLoopPlot->setPlotType(PLOT_HILLS);
							}
						}
					}
				}
			}
		}
	}
}

void CvGame::normalizeAddLakes()
{
	for (int iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			CvPlot* pLakePlot = normalizeFindLakePlot((PlayerTypes)iI);
			if (pLakePlot != NULL)
			{
				pLakePlot->setPlotType(PLOT_OCEAN);
			}
		}
	}
}

CvPlot* CvGame::normalizeFindLakePlot(PlayerTypes ePlayer)
{
	if (!GET_PLAYER(ePlayer).isAlive())
	{
		return NULL;
	}

	CvPlot* pStartingPlot = GET_PLAYER(ePlayer).getStartingPlot();
	if (pStartingPlot != NULL)
	{
		if (!(pStartingPlot->isFreshWater()))
		{
			for (int iJ = 0; iJ < NUM_CITY_PLOTS; iJ++)
			{
				CvPlot* pLoopPlot = plotCity(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), iJ);

				if (pLoopPlot != NULL)
				{
					if (!(pLoopPlot->isWater()))
					{
						if (!(pLoopPlot->isCoastalLand()))
						{
							if (!(pLoopPlot->isRiver()))
							{
								if (pLoopPlot->getBonusType() == NO_BONUS)
								{
									bool bStartingPlot = false;

									for (int iK = 0; iK < MAX_CIV_PLAYERS; iK++)
									{
										if (GET_PLAYER((PlayerTypes)iK).isAlive())
										{
											if (GET_PLAYER((PlayerTypes)iK).getStartingPlot() == pLoopPlot)
											{
												bStartingPlot = true;
												break;
											}
										}
									}

									if (!bStartingPlot)
									{
										return pLoopPlot;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return NULL;
}


void CvGame::normalizeRemoveBadFeatures()
{
	CvPlot* pStartingPlot;
	CvPlot* pLoopPlot;
	int iI, iJ;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			pStartingPlot = GET_PLAYER((PlayerTypes)iI).getStartingPlot();

			if (pStartingPlot != NULL)
			{
				for (iJ = 0; iJ < NUM_CITY_PLOTS; iJ++)
				{
					pLoopPlot = plotCity(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), iJ);

					if (pLoopPlot != NULL)
					{
						if (pLoopPlot->getFeatureType() != NO_FEATURE)
						{
							if ((GC.getFeatureInfo(pLoopPlot->getFeatureType()).getYieldChange(YIELD_FOOD) <= 0) &&
								(GC.getFeatureInfo(pLoopPlot->getFeatureType()).getYieldChange(YIELD_PRODUCTION) <= 0))
							{
								pLoopPlot->setFeatureType(NO_FEATURE);
							}
						}
					}
				}
			}
			
			int iX, iY;
            int iCityRange = CITY_PLOTS_RADIUS;
            int iExtraRange = 2;
            int iMaxRange = iCityRange + iExtraRange;
			
            for (iX = -iMaxRange; iX <= iMaxRange; iX++)
            {
                for (iY = -iMaxRange; iY <= iMaxRange; iY++)
                {
                    pLoopPlot = plotXY(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), iX, iY);
                    if (pLoopPlot != NULL)
                    {
                        int iDistance = plotDistance(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE());			
                        if (iDistance <= iMaxRange)
                        {
                            if (pLoopPlot->getFeatureType() != NO_FEATURE)
                            {
                                if ((GC.getFeatureInfo(pLoopPlot->getFeatureType()).getYieldChange(YIELD_FOOD) <= 0) &&
                                    (GC.getFeatureInfo(pLoopPlot->getFeatureType()).getYieldChange(YIELD_PRODUCTION) <= 0))
                                {
                                    if (pLoopPlot->isWater())
                                    {
                                        if (pLoopPlot->isAdjacentToLand() || (!(iDistance == iMaxRange) && (getSorenRandNum(2, "Remove Bad Feature") == 0)))
                                        {
                                            pLoopPlot->setFeatureType(NO_FEATURE);
                                        }
                                    }
                                    else
                                    {
                                        if (!(iDistance == iMaxRange) && (getSorenRandNum((2 + (pLoopPlot->getBonusType() == NO_BONUS) ? 0 : 2), "Remove Bad Feature") == 0))
                                        {
                                            pLoopPlot->setFeatureType(NO_FEATURE);                                            
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
	}
}


void CvGame::normalizeRemoveBadTerrain()
{
	CvPlot* pStartingPlot;
	CvPlot* pLoopPlot;
	int iI, iK;
	int iX, iY;
	
	int iTargetFood;
	int iTargetTotal;
	int iPlotFood;
	int iPlotProduction;
	
	
	int iCityRange = CITY_PLOTS_RADIUS;
	int iExtraRange = 1;
	int iMaxRange = iCityRange + iExtraRange;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			pStartingPlot = GET_PLAYER((PlayerTypes)iI).getStartingPlot();

			if (pStartingPlot != NULL)
			{
			    for (iX = -iMaxRange; iX <= iMaxRange; iX++)
			    {
			        for (iY = -iMaxRange; iY <= iMaxRange; iY++)
			        {
			            pLoopPlot = plotXY(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), iX, iY);
                        if (pLoopPlot != NULL)
                        {
                            int iDistance = plotDistance(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE());
                            if (iDistance <= iMaxRange)
                            {
                                if (!(pLoopPlot->isWater()) && ((iDistance <= iCityRange) || (pLoopPlot->isCoastalLand()) || (0 == getSorenRandNum(1 + iDistance - iCityRange, "Map Upgrade Terrain Food"))))
                                {
                                    iPlotFood = GC.getTerrainInfo(pLoopPlot->getTerrainType()).getYield(YIELD_FOOD);
                                    iPlotProduction = GC.getTerrainInfo(pLoopPlot->getTerrainType()).getYield(YIELD_PRODUCTION);
                                    if ((iPlotFood + iPlotProduction) <= 1)
                                    {
                                        iTargetFood = 1;
                                        iTargetTotal = 1;
                                        if (pLoopPlot->getBonusType(GET_PLAYER((PlayerTypes)iI).getTeam()) != NO_BONUS)
                                        {
                                            iTargetFood = 1;
                                            iTargetTotal = 2;
                                        }
                                        else if ((iPlotFood == 1) || (iDistance <= iCityRange))
                                        {
                                            iTargetFood = 1 + getSorenRandNum(2, "Map Upgrade Terrain Food");
                                            iTargetTotal = 2;
                                        }
                                        else
                                        {
                                            iTargetFood = pLoopPlot->isCoastalLand() ? 2 : 1;
                                            iTargetTotal = 2;
                                        }
                                        
                                        for (iK = 0; iK < GC.getNumTerrainInfos(); iK++)
                                        {
                                            if (!(GC.getTerrainInfo((TerrainTypes)iK).isWater()))
                                            {
                                                if ((GC.getTerrainInfo((TerrainTypes)iK).getYield(YIELD_FOOD) >= iTargetFood) && 
                                                    (GC.getTerrainInfo((TerrainTypes)iK).getYield(YIELD_FOOD) + GC.getTerrainInfo((TerrainTypes)iK).getYield(YIELD_PRODUCTION)) == iTargetTotal)
                                                {
                                                    if ((pLoopPlot->getFeatureType() == NO_FEATURE) || GC.getFeatureInfo(pLoopPlot->getFeatureType()).isTerrain(iK))
                                                    {
                                                        pLoopPlot->setTerrainType((TerrainTypes)iK);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
  
			            }
			        }
				}
			}
		}
	}
}


void CvGame::normalizeAddFoodBonuses()
{
	CvPlot* pStartingPlot;
	CvPlot* pLoopPlot;
	BonusTypes eBonus;
	bool bIgnoreLatitude;
	int iFoodBonus;
	int iGoodNatureTileCount;
	int iI, iJ, iK;

	bIgnoreLatitude = false;
	gDLL->getPythonIFace()->pythonIsBonusIgnoreLatitudes(&bIgnoreLatitude);

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			pStartingPlot = GET_PLAYER((PlayerTypes)iI).getStartingPlot();

			if (pStartingPlot != NULL)
			{
				iFoodBonus = 0;
				iGoodNatureTileCount = 0;

				for (iJ = 0; iJ < NUM_CITY_PLOTS; iJ++)
				{
					pLoopPlot = plotCity(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), iJ);

					if (pLoopPlot != NULL)
					{
						eBonus = pLoopPlot->getBonusType(GET_PLAYER((PlayerTypes)iI).getTeam());
						

						if (eBonus != NO_BONUS)
						{
						    if (pLoopPlot->calculateBestNatureYield(YIELD_FOOD, GET_PLAYER((PlayerTypes)iI).getTeam()) >= 2)
						    {
						        iGoodNatureTileCount++;
						    }
							if (GC.getBonusInfo(eBonus).getYieldChange(YIELD_FOOD) > 0)
							{
								if ((GC.getBonusInfo(eBonus).getTechCityTrade() == NO_TECH) || (GC.getTechInfo((TechTypes)(GC.getBonusInfo(eBonus).getTechCityTrade())).getEra() <= getStartEra()))
								{
									if (pLoopPlot->isWater())
									{
										iFoodBonus += 2;
									}
									else
									{
										iFoodBonus += 3;
									}
								}
							}
						}
						else
						{
                            if (pLoopPlot->calculateBestNatureYield(YIELD_FOOD, GET_PLAYER((PlayerTypes)iI).getTeam()) >= 3)
						    {
						        iGoodNatureTileCount++;
						    }
						}
					}
				}
				
				int iTargetFoodBonusCount = 3;
				iTargetFoodBonusCount += (iGoodNatureTileCount == 0) ? 2 : 0;

				for (iJ = 0; iJ < NUM_CITY_PLOTS; iJ++)
				{
					if (iFoodBonus >= iTargetFoodBonusCount)
					{
						break;
					}

					pLoopPlot = plotCity(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), iJ);

					if (pLoopPlot != NULL)
					{
						if (pLoopPlot != pStartingPlot)
						{
							if (pLoopPlot->getBonusType() == NO_BONUS)
							{
								for (iK = 0; iK < GC.getNumBonusInfos(); iK++)
								{
									if (GC.getBonusInfo((BonusTypes)iK).isNormalize())
									{
										if (GC.getBonusInfo((BonusTypes)iK).getYieldChange(YIELD_FOOD) > 0)
										{
											if ((GC.getBonusInfo((BonusTypes)iK).getTechCityTrade() == NO_TECH) || (GC.getTechInfo((TechTypes)(GC.getBonusInfo((BonusTypes)iK).getTechCityTrade())).getEra() <= getStartEra()))
											{
												if (GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).isHasTech((TechTypes)(GC.getBonusInfo((BonusTypes)iK).getTechReveal())))
												{
													if (pLoopPlot->canHaveBonus(((BonusTypes)iK), bIgnoreLatitude))
													{
														pLoopPlot->setBonusType((BonusTypes)iK);
														if (pLoopPlot->isWater())
														{
															iFoodBonus += 2;
														}
														else
														{
															iFoodBonus += 3;
														}
														break;
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}


void CvGame::normalizeAddGoodTerrain()
{
	CvPlot* pStartingPlot;
	CvPlot* pLoopPlot;
	bool bChanged;
	int iGoodPlot;
	int iI, iJ, iK;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			pStartingPlot = GET_PLAYER((PlayerTypes)iI).getStartingPlot();

			if (pStartingPlot != NULL)
			{
				iGoodPlot = 0;

				for (iJ = 0; iJ < NUM_CITY_PLOTS; iJ++)
				{
					pLoopPlot = plotCity(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), iJ);

					if (pLoopPlot != NULL)
					{
						if (pLoopPlot != pStartingPlot)
						{
							if ((pLoopPlot->calculateNatureYield(YIELD_FOOD, GET_PLAYER((PlayerTypes)iI).getTeam()) >= GC.getFOOD_CONSUMPTION_PER_POPULATION()) &&
								  (pLoopPlot->calculateNatureYield(YIELD_PRODUCTION, GET_PLAYER((PlayerTypes)iI).getTeam()) > 0))
							{
								iGoodPlot++;
							}
						}
					}
				}

				for (iJ = 0; iJ < NUM_CITY_PLOTS; iJ++)
				{
					if (iGoodPlot >= 4)
					{
						break;
					}

					pLoopPlot = plotCity(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), iJ);

					if (pLoopPlot != NULL)
					{
						if (pLoopPlot != pStartingPlot)
						{
							if (!(pLoopPlot->isWater()))
							{
								if (!(pLoopPlot->isHills()))
								{
									if (pLoopPlot->getBonusType() == NO_BONUS)
									{
										bChanged = false;

										if (pLoopPlot->calculateNatureYield(YIELD_FOOD, GET_PLAYER((PlayerTypes)iI).getTeam()) < GC.getFOOD_CONSUMPTION_PER_POPULATION())
										{
											for (iK = 0; iK < GC.getNumTerrainInfos(); iK++)
											{
												if (!(GC.getTerrainInfo((TerrainTypes)iK).isWater()))
												{
													if (GC.getTerrainInfo((TerrainTypes)iK).getYield(YIELD_FOOD) >= GC.getFOOD_CONSUMPTION_PER_POPULATION())
													{
														pLoopPlot->setTerrainType((TerrainTypes)iK);
														bChanged = true;
														break;
													}
												}
											}
										}

										if (pLoopPlot->calculateNatureYield(YIELD_PRODUCTION, GET_PLAYER((PlayerTypes)iI).getTeam()) == 0)
										{
											for (iK = 0; iK < GC.getNumFeatureInfos(); iK++)
											{
												if ((GC.getFeatureInfo((FeatureTypes)iK).getYieldChange(YIELD_FOOD) >= 0) &&
													  (GC.getFeatureInfo((FeatureTypes)iK).getYieldChange(YIELD_PRODUCTION) > 0))
												{
													if (GC.getFeatureInfo((FeatureTypes)iK).isTerrain(pLoopPlot->getTerrainType()))
													{
														pLoopPlot->setFeatureType((FeatureTypes)iK);
														bChanged = true;
														break;
													}
												}
											}
										}

										if (bChanged)
										{
											iGoodPlot++;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}


void CvGame::normalizeAddExtras()
{
	CvPlot* pStartingPlot;
	CvPlot* pLoopPlot;
	bool bIgnoreLatitude;
	int iValue;
	int iTotalValue;
	int iBestValue;
	int iWorstValue;
	int iTargetValue;
	int iCount;
	int iPass;
	int iPlayerCount;
	int iI, iJ, iK;
	
	int iOceanFoodCount;
	int iCoastFoodCount;
	bool bLandBias;

	bIgnoreLatitude = false;
	gDLL->getPythonIFace()->pythonIsBonusIgnoreLatitudes(&bIgnoreLatitude);

	iTotalValue = 0;
	iPlayerCount = 0;
	iBestValue = 0;
	iWorstValue = MAX_INT;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			pStartingPlot = GET_PLAYER((PlayerTypes)iI).getStartingPlot();

			if (pStartingPlot != NULL)
			{
				iValue = GET_PLAYER((PlayerTypes)iI).AI_foundValue(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), -1, true);
				iTotalValue += iValue;
                iPlayerCount++;
                
                iBestValue = max(iValue, iBestValue);
                iWorstValue = min(iValue, iWorstValue);
                
			}
		}
	}

	//iTargetValue = (iTotalValue + iBestValue) / (iPlayerCount + 1);
	
	iTargetValue = (iBestValue * 4) / 5;
		

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			gDLL->callUpdater();	// allow window to update during launch
			pStartingPlot = GET_PLAYER((PlayerTypes)iI).getStartingPlot();

			if (pStartingPlot != NULL)
			{
			    
                iCount = 0;
				int iFeatureCount = 0;

				for (iJ = 0; iJ < NUM_CITY_PLOTS; iJ++)
				{
					if (GET_PLAYER((PlayerTypes)iI).AI_foundValue(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), -1, true) >= iTargetValue)
					{
						break;
					}
					


					if (getSorenRandNum((iCount + 2), "Setting Feature Type") <= 1)
					{
						pLoopPlot = plotCity(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), iJ);

						if (pLoopPlot != NULL)
						{
							if (pLoopPlot != pStartingPlot)
							{
								if (pLoopPlot->getBonusType() == NO_BONUS)
								{
									if (pLoopPlot->getFeatureType() == NO_FEATURE)
									{
										for (iK = 0; iK < GC.getNumFeatureInfos(); iK++)
										{
											if ((GC.getFeatureInfo((FeatureTypes)iK).getYieldChange(YIELD_FOOD) + GC.getFeatureInfo((FeatureTypes)iK).getYieldChange(YIELD_PRODUCTION)) > 0)
											{
												if (pLoopPlot->canHaveFeature((FeatureTypes)iK))
												{
													pLoopPlot->setFeatureType((FeatureTypes)iK);
													iCount++;
													break;
												}
											}
										}
									}
									
									iFeatureCount += (pLoopPlot->getFeatureType() != NO_FEATURE) ? 1 : 0;
								}
							}
						}
					}
				}
				
				iCoastFoodCount = 0;
				iOceanFoodCount = 0;
				int iWaterCount = 0;
				for (iJ = 0; iJ < NUM_CITY_PLOTS; iJ++)
				{
					pLoopPlot = plotCity(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), iJ);
					if (pLoopPlot != NULL)
					{
						if (pLoopPlot != pStartingPlot)
						{
							iWaterCount++;
							if (pLoopPlot->isWater())
							{
								if (pLoopPlot->getBonusType() != NO_BONUS)
								{
									if (pLoopPlot->isAdjacentToLand())
									{
										iCoastFoodCount++;
									}
									else
									{
										iOceanFoodCount++;                                        
									}
								}
							}
						}
					}
				}
				
				if (iWaterCount > NUM_CITY_PLOTS / 2)
				{
				    bLandBias = true;
				}

                iCount = 0;

				for (iJ = 0; iJ < NUM_CITY_PLOTS; iJ++)
				{
				    pLoopPlot = plotCity(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), iJ);

                    if ((pLoopPlot != NULL) && (pLoopPlot != pStartingPlot))
                    {
                        if (getSorenRandNum((((iCount * 3) + 1) * ((bLandBias && pLoopPlot->isWater()) ? 2 : 1)), "Placing Bonuses") == 0)
                        {
                            if (GET_PLAYER((PlayerTypes)iI).AI_foundValue(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), -1, true) >= iTargetValue)
                            {
                                break;
                            }

						    bool bCoast = (pLoopPlot->isWater() && pLoopPlot->isAdjacentToLand());
						    bool bOcean = (pLoopPlot->isWater() && !bCoast);
							if ((pLoopPlot != pStartingPlot)
                                && !(bCoast && (iCoastFoodCount > 2))
                                && !(bOcean && (iOceanFoodCount > 2)))
							{
								for (iPass = 0; iPass < 2; iPass++)
								{
									if (pLoopPlot->getBonusType() == NO_BONUS)
									{
										for (iK = 0; iK < GC.getNumBonusInfos(); iK++)
										{
											if (GC.getBonusInfo((BonusTypes)iK).isNormalize())
											{
											    //???no bonuses with negative yields?
												if ((GC.getBonusInfo((BonusTypes)iK).getYieldChange(YIELD_FOOD) >= 0) &&
													  (GC.getBonusInfo((BonusTypes)iK).getYieldChange(YIELD_PRODUCTION) >= 0))
												{
													if ((GC.getBonusInfo((BonusTypes)iK).getTechCityTrade() == NO_TECH) || (GC.getTechInfo((TechTypes)(GC.getBonusInfo((BonusTypes)iK).getTechCityTrade())).getEra() <= getStartEra()))
													{
														if (GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).isHasTech((TechTypes)(GC.getBonusInfo((BonusTypes)iK).getTechReveal())))
														{
															if ((iPass == 0) ? CvMapGenerator::GetInstance().canPlaceBonusAt(((BonusTypes)iK), pLoopPlot->getX(), pLoopPlot->getY(), bIgnoreLatitude) : pLoopPlot->canHaveBonus(((BonusTypes)iK), bIgnoreLatitude))
															{
																pLoopPlot->setBonusType((BonusTypes)iK);
																iCount++;
																iCoastFoodCount += bCoast ? 1 : 0;
																iOceanFoodCount += bOcean ? 1 : 0;
																break;
															}
														}
													}
												}
											}
										}
										
										if (bLandBias && !(pLoopPlot->isWater()) && pLoopPlot->getBonusType() != NO_BONUS)
										{
											if (((iFeatureCount > 4) && (pLoopPlot->getFeatureType() != NO_FEATURE))
												&& ((iCoastFoodCount + iOceanFoodCount) > 2))
											{
												pLoopPlot->setFeatureType(NO_FEATURE);
												if (getSorenRandNum(2, "Clear feature do add bonus") == 0)
												{
													for (iK = 0; iK < GC.getNumBonusInfos(); iK++)
													{
														if (GC.getBonusInfo((BonusTypes)iK).isNormalize())
														{
															//???no bonuses with negative yields?
															if ((GC.getBonusInfo((BonusTypes)iK).getYieldChange(YIELD_FOOD) >= 0) &&
																  (GC.getBonusInfo((BonusTypes)iK).getYieldChange(YIELD_PRODUCTION) >= 0))
															{
																if ((GC.getBonusInfo((BonusTypes)iK).getTechCityTrade() == NO_TECH) || (GC.getTechInfo((TechTypes)(GC.getBonusInfo((BonusTypes)iK).getTechCityTrade())).getEra() <= getStartEra()))
																{
																	if ((iPass == 0) ? CvMapGenerator::GetInstance().canPlaceBonusAt(((BonusTypes)iK), pLoopPlot->getX(), pLoopPlot->getY(), bIgnoreLatitude) : pLoopPlot->canHaveBonus(((BonusTypes)iK), bIgnoreLatitude))
																	{
																		pLoopPlot->setBonusType((BonusTypes)iK);
																		iCount++;
																		iCoastFoodCount += bCoast ? 1 : 0;
																		iOceanFoodCount += bOcean ? 1 : 0;
																		break;
																	}
																}
															}
														}
													}
												}
											}										
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}


void CvGame::normalizeStartingPlots()
{
	PROFILE_FUNC();

	if (!(GC.getInitCore().getWBMapScript()) || GC.getInitCore().getWBMapNoPlayers())
	{
		if (!gDLL->getPythonIFace()->pythonNormalizeStartingPlotLocations()  || gDLL->getPythonIFace()->pythonUsingDefaultImpl())
		{
			normalizeStartingPlotLocations();
		}
	}

	if (GC.getInitCore().getWBMapScript())
	{
		return;
	}

	if (!gDLL->getPythonIFace()->pythonNormalizeAddRiver()  || gDLL->getPythonIFace()->pythonUsingDefaultImpl())
	{
		normalizeAddRiver();
	}

	if (!gDLL->getPythonIFace()->pythonNormalizeRemovePeaks()  || gDLL->getPythonIFace()->pythonUsingDefaultImpl())
	{
		normalizeRemovePeaks();
	}

	if (!gDLL->getPythonIFace()->pythonNormalizeAddLakes()  || gDLL->getPythonIFace()->pythonUsingDefaultImpl())
	{
		normalizeAddLakes();
	}

	if (!gDLL->getPythonIFace()->pythonNormalizeRemoveBadFeatures()  || gDLL->getPythonIFace()->pythonUsingDefaultImpl())
	{
		normalizeRemoveBadFeatures();
	}

	if (!gDLL->getPythonIFace()->pythonNormalizeRemoveBadTerrain()  || gDLL->getPythonIFace()->pythonUsingDefaultImpl())
	{
		normalizeRemoveBadTerrain();
	}

	if (!gDLL->getPythonIFace()->pythonNormalizeAddFoodBonuses()  || gDLL->getPythonIFace()->pythonUsingDefaultImpl())
	{
		normalizeAddFoodBonuses();
	}

	if (!gDLL->getPythonIFace()->pythonNormalizeAddGoodTerrain()  || gDLL->getPythonIFace()->pythonUsingDefaultImpl())
	{
		normalizeAddGoodTerrain();
	}

	if (!gDLL->getPythonIFace()->pythonNormalizeAddExtras()  || gDLL->getPythonIFace()->pythonUsingDefaultImpl())
	{
		normalizeAddExtras();
	}
}

// For each of n teams, let the closeness score for that team be the average distance of an edge between two players on that team.
// This function calculates the closeness score for each team and returns the sum of those n scores.
// The lower the result, the better "clumped" the players' starting locations are.
//
// Note: for the purposes of this function, player i will be assumed to start in the location of player aiStartingLocs[i] 

int CvGame::getTeamClosenessScore(int** aaiDistances, int* aiStartingLocs)
{
	int iScore = 0;

	for (int iTeam = 0; iTeam < MAX_CIV_TEAMS; iTeam++)
	{
		if (GET_TEAM((TeamTypes)iTeam).isAlive())
		{
			int iTeamTotalDist = 0;
			int iNumEdges = 0;
			for (int iPlayer = 0; iPlayer < MAX_CIV_PLAYERS; iPlayer++)
			{
				if (GET_PLAYER((PlayerTypes)iPlayer).isAlive())
				{
					if (GET_PLAYER((PlayerTypes)iPlayer).getTeam() == (TeamTypes)iTeam)
					{
						for (int iOtherPlayer = 0; iOtherPlayer < iPlayer; iOtherPlayer++)
						{
							if (GET_PLAYER((PlayerTypes)iOtherPlayer).getTeam() == (TeamTypes)iTeam)
							{
								// Add the edge between these two players that are on the same team
								iNumEdges++;
								int iPlayerStart = aiStartingLocs[iPlayer];
								int iOtherPlayerStart = aiStartingLocs[iOtherPlayer];

								if (iPlayerStart < iOtherPlayerStart) // Make sure that iPlayerStart > iOtherPlayerStart
								{
									int iTemp = iPlayerStart;
									iPlayerStart = iOtherPlayerStart;
									iOtherPlayerStart = iTemp;
								}
								else if (iPlayerStart == iOtherPlayerStart)
								{
									FAssertMsg(false, "Two players are (hypothetically) assigned to the same starting location!");
								}
								iTeamTotalDist += aaiDistances[iPlayerStart][iOtherPlayerStart];
							}
						}
					}
				}
			}

			int iTeamScore;
			if (iNumEdges == 0)
			{
				iTeamScore = 0;
			}
			else
			{
				iTeamScore = iTeamTotalDist/iNumEdges; // the avg distance between team edges is the team score
			}
			iScore += iTeamScore;
		}
	}
	return iScore;
}


void CvGame::update()
{
	PROFILE("CvGame::update");

	if (!gDLL->GetWorldBuilderMode())
	{
		sendPlayerOptions();

		// sample generic event
		CyArgsList pyArgs;
		pyArgs.add(getTurnSlice());
		gDLL->getEventReporterIFace()->genericEvent("gameUpdate", pyArgs.makeFunctionArgs());

		if (getTurnSlice() == 0)
		{
			gDLL->getEngineIFace()->AutoSave(true);
		}

		if (getNumGameTurnActive() == 0)
		{
			if (!isPbem() || !getPbemTurnSent())
			{
				doTurn();
			}
		}

		updateScore();

		updateWar();

		updateMoves();

		updateTimers();

		updateTurnTimer();

		AI_updateAssignWork();

		testAlive();

		if ((getAIAutoPlay() == 0) && !(gDLL->GetAutorun()) && GAMESTATE_EXTENDED != getGameState())
		{
			if (countHumanPlayersAlive() == 0)
			{
				setGameState(GAMESTATE_OVER);
			}
		}

		changeTurnSlice(1);
	}
}


void CvGame::updateScore(bool bForce)
{
	bool abPlayerScored[MAX_CIV_PLAYERS];
	bool abTeamScored[MAX_CIV_TEAMS];
	int iScore;
	int iBestScore;
	PlayerTypes eBestPlayer;
	TeamTypes eBestTeam;
	int iI, iJ, iK;

	if (!isScoreDirty() && !bForce)
	{
		return;
	}

	setScoreDirty(false);

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		abPlayerScored[iI] = false;
	}

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		iBestScore = MIN_INT;
		eBestPlayer = NO_PLAYER;

		for (iJ = 0; iJ < MAX_CIV_PLAYERS; iJ++)
		{
			if (!abPlayerScored[iJ])
			{
				iScore = GET_PLAYER((PlayerTypes)iJ).calculateScore(false);

				if (iScore >= iBestScore)
				{
					iBestScore = iScore;
					eBestPlayer = (PlayerTypes)iJ;
				}
			}
		}

		abPlayerScored[eBestPlayer] = true;

		setRankPlayer(iI, eBestPlayer);
		setPlayerRank(eBestPlayer, iI);
		setPlayerScore(eBestPlayer, iBestScore);
		GET_PLAYER(eBestPlayer).updateScoreHistory(getGameTurn(), iBestScore);
	}

	for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
	{
		abTeamScored[iI] = false;
	}

	for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
	{
		iBestScore = MIN_INT;
		eBestTeam = NO_TEAM;

		for (iJ = 0; iJ < MAX_CIV_TEAMS; iJ++)
		{
			if (!abTeamScored[iJ])
			{
				iScore = 0;

				for (iK = 0; iK < MAX_CIV_PLAYERS; iK++)
				{
					if (GET_PLAYER((PlayerTypes)iK).getTeam() == iJ)
					{
						iScore += getPlayerScore((PlayerTypes)iK);
					}
				}

				if (iScore >= iBestScore)
				{
					iBestScore = iScore;
					eBestTeam = (TeamTypes)iJ;
				}
			}
		}

		abTeamScored[eBestTeam] = true;

		setRankTeam(iI, eBestTeam);
		setTeamRank(eBestTeam, iI);
		setTeamScore(eBestTeam, iBestScore);
	}
}


void CvGame::updateColoredPlots()
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pSelectedCityNode;
	CLLNode<IDInfo>* pSelectedUnitNode;
	CvCity* pHeadSelectedCity;
	CvCity* pSelectedCity;
	CvCity* pCity;
	CvUnit* pHeadSelectedUnit;
	CvUnit* pSelectedUnit;
	CvPlot* pRallyPlot;
	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	CvPlot* pNextBestPlot;
	long lResult;
	int iMaxAirRange;
	int iRange;
	//int iPathTurns;
	int iDX, iDY;
	int iI;

	gDLL->getEngineIFace()->clearColoredPlots(PLOT_LANDSCAPE_LAYER_BASE);

	if (!gDLL->GetWorldBuilderMode())
	{
		gDLL->getEngineIFace()->clearColoredPlots(PLOT_LANDSCAPE_LAYER_RECOMMENDED_PLOTS);
	}

	lResult = 0;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "updateColoredPlots", NULL, &lResult);
	if (lResult == 1)
	{
		return;
	}

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();

	if (pHeadSelectedCity != NULL)
	{
		if (gDLL->getInterfaceIFace()->isCityScreenUp())
		{
			for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
			{
				if (pHeadSelectedCity->isWorkingPlot(iI))
				{
					pLoopPlot = plotCity(pHeadSelectedCity->getX_INLINE(), pHeadSelectedCity->getY_INLINE(), iI);

					if (pLoopPlot != NULL)
					{
						NiColorA color(GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_WHITE")).getColor());
						color.a = 0.7f;
						gDLL->getEngineIFace()->addColoredPlot(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), color, PLOT_STYLE_CIRCLE, PLOT_LANDSCAPE_LAYER_BASE);
					}
				}
			}
		}
		else
		{
			pSelectedCityNode = gDLL->getInterfaceIFace()->headSelectedCitiesNode();

			while (pSelectedCityNode != NULL)
			{
				pSelectedCity = ::getCity(pSelectedCityNode->m_data);
				pSelectedCityNode = gDLL->getInterfaceIFace()->nextSelectedCitiesNode(pSelectedCityNode);

				if (pSelectedCity != NULL)
				{
					pRallyPlot = pSelectedCity->getRallyPlot();

					if (pRallyPlot != NULL)
					{
						gDLL->getEngineIFace()->addColoredPlot(pRallyPlot->getX_INLINE(), pRallyPlot->getY_INLINE(), GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_YELLOW")).getColor(), PLOT_STYLE_CIRCLE, PLOT_LANDSCAPE_LAYER_BASE);
					}
				}
			}
		}
	}
	else if (pHeadSelectedUnit != NULL)
	{
#ifdef ITS_FULL_OF_STARS
		// testing
		static bool bDrawVisibility = true;
		if (bDrawVisibility)
		{
			TeamTypes eTeam = pHeadSelectedUnit->getTeam();
			int iRange = pHeadSelectedUnit->visibilityRange();

			CvPlotDataRegion visiblePlotDataRegion;
			pHeadSelectedUnit->getGroup()->buildVisibilityRegion(visiblePlotDataRegion);
			for(CvPlotDataRegionIterator it = visiblePlotDataRegion.begin(); it != visiblePlotDataRegion.end(); it++)
			{
				XYCoords xy(getXYCoords(it));
				CvPlot* pLoopPlot = GC.getMapINLINE().plotSorenINLINE(xy.iX, xy.iY);
				FAssert(pLoopPlot != NULL);
				gDLL->getEngineIFace()->addColoredPlot(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_NEGATIVE_TEXT")).getColor(), PLOT_STYLE_BOX_OUTLINE, PLOT_LANDSCAPE_LAYER_RECOMMENDED_PLOTS);
			}

			CvPlotRegion visiblePlotRegion;
			pHeadSelectedUnit->getGroup()->buildVisibilityRegion(visiblePlotRegion);
			for(CvPlotRegionIterator it = visiblePlotRegion.begin(); it != visiblePlotRegion.end(); it++)
			{
				XYCoords xy(getXYCoords(it));
				CvPlot* pLoopPlot = GC.getMapINLINE().plotSorenINLINE(xy.iX, xy.iY);
				FAssert(pLoopPlot != NULL);
				gDLL->getEngineIFace()->addColoredPlot(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_UNIT_TEXT")).getColor(), PLOT_STYLE_BOX_OUTLINE, PLOT_LANDSCAPE_LAYER_RECOMMENDED_PLOTS);
			}

			/*CvPlotDataRegion visiblePlotRegion;
			pHeadSelectedUnit->getGroup()->buildVisibilityRegion(visiblePlotRegion);
			for(CvPlotDataRegionIterator it = visiblePlotRegion.begin(); it != visiblePlotRegion.end(); it++)
			{
				XYCoords xy(getXYCoords(it));
				CvPlot* pLoopPlot = GC.getMapINLINE().plotSorenINLINE(xy.iX, xy.iY);
				FAssert(pLoopPlot != NULL);
				CvUnitAI* pUnit = static_cast<CvUnitAI*>(pHeadSelectedUnit);
				if (pUnit->AI_fogbustPlotValue(pLoopPlot, getRegionValue(it)) > 0)
				{
					gDLL->getEngineIFace()->addColoredPlot(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_UNIT_TEXT")).getColor(), PLOT_STYLE_BOX_OUTLINE, PLOT_LANDSCAPE_LAYER_RECOMMENDED_PLOTS);
				}
			}*/

			/*CvPlotDataRegion visibilityValues;
			pHeadSelectedUnit->plot()->addVisiblePlots(visibilityValues, iRange, 1, eTeam);
			
			if (pHeadSelectedUnit->getDomainType() == DOMAIN_LAND)
			{
				CvPlotDataRegion adjacentPlots;
				bool bIncludeOriginal = (pHeadSelectedUnit->getDomainType() == DOMAIN_AIR);
				int iRange = (pHeadSelectedUnit->getDomainType() == DOMAIN_LAND) ? 2 : 1;
				findAdjacentPlots(visibilityValues, adjacentPlots, bIncludeOriginal, iRange);
				for(CvPlotDataRegionIterator it = adjacentPlots.begin(); it != adjacentPlots.end(); it++)
				{
					CvPlot* pLoopPlot = GC.getMapINLINE().plotSorenINLINE(it->first.iX, it->first.iY);
					FAssert(pLoopPlot != NULL);
					gDLL->getEngineIFace()->addColoredPlot(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_POSITIVE_TEXT")).getColor(), PLOT_STYLE_BOX_OUTLINE, PLOT_LANDSCAPE_LAYER_RECOMMENDED_PLOTS);
				}

				CvPlotDataRegion furtherAdjacentPlots;
				bIncludeOriginal = false;
				iRange = 1;
				findAdjacentPlots(adjacentPlots, furtherAdjacentPlots, bIncludeOriginal, iRange);
				for(CvPlotDataRegionIterator it = furtherAdjacentPlots.begin(); it != furtherAdjacentPlots.end(); it++)
				{
					CvPlot* pLoopPlot = GC.getMapINLINE().plotSorenINLINE(it->first.iX, it->first.iY);
					FAssert(pLoopPlot != NULL);
					gDLL->getEngineIFace()->addColoredPlot(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_NEGATIVE_TEXT")).getColor(), PLOT_STYLE_BOX_OUTLINE, PLOT_LANDSCAPE_LAYER_RECOMMENDED_PLOTS);
				}
			}
			else
			{
				CvPlotRegion adjacentPlots;
				bool bIncludeOriginal = (pHeadSelectedUnit->getDomainType() == DOMAIN_AIR);
				int iRange = (pHeadSelectedUnit->getDomainType() == DOMAIN_LAND) ? 2 : 1;
				findAdjacentPlots(visiblePlotRegion, adjacentPlots, bIncludeOriginal, iRange);
				for(CvPlotRegionIterator it = adjacentPlots.begin(); it != adjacentPlots.end(); it++)
				{
					CvPlot* pLoopPlot = GC.getMapINLINE().plotSorenINLINE(it->iX, it->iY);
					FAssert(pLoopPlot != NULL);
					gDLL->getEngineIFace()->addColoredPlot(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_POSITIVE_TEXT")).getColor(), PLOT_STYLE_BOX_OUTLINE, PLOT_LANDSCAPE_LAYER_RECOMMENDED_PLOTS);
				}

				CvPlotRegion furtherAdjacentPlots;
				bIncludeOriginal = false;
				iRange = 1;
				findAdjacentPlots(adjacentPlots, furtherAdjacentPlots, bIncludeOriginal, iRange);
				for(CvPlotRegionIterator it = furtherAdjacentPlots.begin(); it != furtherAdjacentPlots.end(); it++)
				{
					CvPlot* pLoopPlot = GC.getMapINLINE().plotSorenINLINE(it->iX, it->iY);
					FAssert(pLoopPlot != NULL);
					gDLL->getEngineIFace()->addColoredPlot(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_NEGATIVE_TEXT")).getColor(), PLOT_STYLE_BOX_OUTLINE, PLOT_LANDSCAPE_LAYER_RECOMMENDED_PLOTS);
				}

			}*/

			//CLinkList<CvPlot*> visiblePlotList;
			//pHeadSelectedUnit->plot()->getVisiblePlotsList(visiblePlotList, eTeam, iRange);
			/*for (CLLNode<CvPlot*>* pNode = visiblePlotList.head(); (pNode != NULL); pNode = visiblePlotList.next(pNode))
			{
				CvPlot* pLoopPlot = pNode->m_data;
				CvPlotDataRegionIterator itValue = visibilityValues.find(XYCoords(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE()));
				gDLL->getEngineIFace()->addColoredPlot(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), GC.getColorInfo((ColorTypes)GC.getInfoTypeForString(itValue != visibilityValues.end() ? "COLOR_UNIT_TEXT" : "COLOR_NEGATIVE_TEXT")).getColor(), PLOT_STYLE_BOX_OUTLINE, PLOT_LANDSCAPE_LAYER_RECOMMENDED_PLOTS);
			}*/

			//// check size
			//FAssert(visiblePlotList.getLength() == visiblePlotRegion.size());

			//// compare
			//for (CLLNode<CvPlot*>* pNode = visiblePlotList.head(); (pNode != NULL); pNode = visiblePlotList.next(pNode))
			//{
			//	CvPlotRegionIterator itPlot = visiblePlotRegion.find(XYCoords(pNode->m_data->getX_INLINE(), pNode->m_data->getY_INLINE()));
			//	FAssert(itPlot != visiblePlotRegion.end());
			//}

			/*CLinkList<CvPlot*> furtherPlotList;
			pHeadSelectedUnit->plot()->getVisiblePlotsList(furtherPlotList, eTeam, iRange + 2, &visiblePlotList);
			for (CLLNode<CvPlot*>* pNode = furtherPlotList.head(); (pNode != NULL); pNode = furtherPlotList.next(pNode))
			{
				CvPlot* pLoopPlot = pNode->m_data;
				gDLL->getEngineIFace()->addColoredPlot(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_NEGATIVE_TEXT")).getColor(), PLOT_STYLE_BOX_OUTLINE, PLOT_LANDSCAPE_LAYER_RECOMMENDED_PLOTS);
			}

			pHeadSelectedUnit->plot()->getVisiblePlotsList(furtherPlotList, eTeam, iRange + 1, &visiblePlotList);
			for (CLLNode<CvPlot*>* pNode = furtherPlotList.head(); (pNode != NULL); pNode = furtherPlotList.next(pNode))
			{
				CvPlot* pLoopPlot = pNode->m_data;
				gDLL->getEngineIFace()->addColoredPlot(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_POSITIVE_TEXT")).getColor(), PLOT_STYLE_BOX_OUTLINE, PLOT_LANDSCAPE_LAYER_RECOMMENDED_PLOTS);
			}*/

		}
#endif

		if (gDLL->getGraphicOption(GRAPHICOPTION_CITY_RADIUS))
		{
			if (gDLL->getInterfaceIFace()->canSelectionListFound())
			{
				for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
				{
					pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

					if (pLoopPlot->getOwnerINLINE() == pHeadSelectedUnit->getOwnerINLINE())
					{
						if (pLoopPlot->getWorkingCity() != NULL)
						{
							NiColorA color(GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT")/*(GC.getPlayerColorInfo(GET_PLAYER(pHeadSelectedUnit->getOwnerINLINE()).getPlayerColor()).getColorTypePrimary())*/).getColor());
							color.a = 0.25f;
							gDLL->getEngineIFace()->addColoredPlot(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), color, PLOT_STYLE_BOX_FILL, PLOT_LANDSCAPE_LAYER_BASE);
						}
					}
				}
			}
		}

		if (pHeadSelectedUnit->getDomainType() == DOMAIN_AIR)
		{
			iMaxAirRange = 0;

			pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

			while (pSelectedUnitNode != NULL)
			{
				pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);
				pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);

				if (pSelectedUnit != NULL)
				{
					iMaxAirRange = max(iMaxAirRange, pSelectedUnit->airRange());
				}
			}

			if (iMaxAirRange > 0)
			{
				for (iDX = -(iMaxAirRange); iDX <= iMaxAirRange; iDX++)
				{
					for (iDY = -(iMaxAirRange); iDY <= iMaxAirRange; iDY++)
					{
						pLoopPlot = plotXY(pHeadSelectedUnit->getX_INLINE(), pHeadSelectedUnit->getY_INLINE(), iDX, iDY);

						if (pLoopPlot != NULL)
						{
							if (plotDistance(pHeadSelectedUnit->getX_INLINE(), pHeadSelectedUnit->getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE()) <= iMaxAirRange)
							{
								NiColorA color(GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_WHITE")).getColor());
								color.a = 0.4f;
								gDLL->getEngineIFace()->addColoredPlot(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), color, PLOT_STYLE_TARGET, PLOT_LANDSCAPE_LAYER_BASE);
							}
						}
					}
				}
			}
		}

		FAssert(getActivePlayer() != NO_PLAYER);

		if (!(GET_PLAYER(getActivePlayer()).isOption(PLAYEROPTION_NO_UNIT_RECOMMENDATIONS)))
		{
			bool bFoundRecommendation = false;

			if ((pHeadSelectedUnit->AI_getUnitAIType() == UNITAI_WORKER) || (pHeadSelectedUnit->AI_getUnitAIType() == UNITAI_WORKER_SEA))
			{
				if (pHeadSelectedUnit->plot()->getOwnerINLINE() == pHeadSelectedUnit->getOwnerINLINE())
				{
					pCity = pHeadSelectedUnit->plot()->getWorkingCity();

					if (pCity != NULL)
					{
						if (pHeadSelectedUnit->AI_bestCityBuild(pCity, &pBestPlot))
						{
							bFoundRecommendation = true;

							FAssert(pBestPlot != NULL);
							gDLL->getEngineIFace()->addColoredPlot(pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT")).getColor(), PLOT_STYLE_CIRCLE, PLOT_LANDSCAPE_LAYER_RECOMMENDED_PLOTS);

							if (pHeadSelectedUnit->AI_bestCityBuild(pCity, &pNextBestPlot, NULL, pBestPlot))
							{
								FAssert(pNextBestPlot != NULL);
								gDLL->getEngineIFace()->addColoredPlot(pNextBestPlot->getX_INLINE(), pNextBestPlot->getY_INLINE(), GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT")).getColor(), PLOT_STYLE_CIRCLE, PLOT_LANDSCAPE_LAYER_RECOMMENDED_PLOTS);
							}
						}
					}
				}
			}

			iRange = 4;

			for (iDX = -(iRange); iDX <= iRange; iDX++)
			{
				for (iDY = -(iRange); iDY <= iRange; iDY++)
				{
					pLoopPlot = plotXY(pHeadSelectedUnit->getX_INLINE(), pHeadSelectedUnit->getY_INLINE(), iDX, iDY);

					if (pLoopPlot != NULL)
					{
						if (pLoopPlot->area() == pHeadSelectedUnit->area())
						{
							if (pLoopPlot->isVisible(pHeadSelectedUnit->getTeam(), false))
							{
								if (plotDistance(pHeadSelectedUnit->getX_INLINE(), pHeadSelectedUnit->getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE()) <= iRange)
								{
									if (pHeadSelectedUnit->canFound(pLoopPlot))
									{
										if (pLoopPlot->isBestAdjacentFound(pHeadSelectedUnit->getOwnerINLINE()))
										{
											bFoundRecommendation = true;
											gDLL->getEngineIFace()->addColoredPlot(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT")).getColor(), PLOT_STYLE_CIRCLE, PLOT_LANDSCAPE_LAYER_RECOMMENDED_PLOTS);
										}
									}

									if (pHeadSelectedUnit->isNoBadGoodies())
									{
										if (pLoopPlot->isRevealedGoody(pHeadSelectedUnit->getTeam()))
										{
											bFoundRecommendation = true;
											gDLL->getEngineIFace()->addColoredPlot(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT")).getColor(), PLOT_STYLE_CIRCLE, PLOT_LANDSCAPE_LAYER_RECOMMENDED_PLOTS);
										}
									}
								}
							}
						}
					}
				}
			}

#ifdef GIVE_EXPLORER_SUGGESTIONS
			if (!bFoundRecommendation && (pHeadSelectedUnit->isNoBadGoodies() || pHeadSelectedUnit->getDomainType() == DOMAIN_SEA))
			{
				CvPlot* pBestExplorePlotThisTurn = NULL;
				CvPlot* pBestExplorePlot = pHeadSelectedUnit->AI_getBestExplorePlot((pHeadSelectedUnit->getDomainType() == DOMAIN_SEA) ? 4 : 3, &pBestExplorePlotThisTurn, /*bNoRandom*/ true);
				if (pBestExplorePlotThisTurn != NULL)
				{
					if (pBestExplorePlotThisTurn != pHeadSelectedUnit->plot())
					{
						gDLL->getEngineIFace()->addColoredPlot(pBestExplorePlotThisTurn->getX_INLINE(), pBestExplorePlotThisTurn->getY_INLINE(), GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT")).getColor(), PLOT_STYLE_CIRCLE, PLOT_LANDSCAPE_LAYER_RECOMMENDED_PLOTS);
					}
					else if (pBestExplorePlot != NULL && pBestExplorePlotThisTurn != pBestExplorePlot)
					{
						gDLL->getEngineIFace()->addColoredPlot(pBestExplorePlot->getX_INLINE(), pBestExplorePlot->getY_INLINE(), GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT")).getColor(), PLOT_STYLE_CIRCLE, PLOT_LANDSCAPE_LAYER_RECOMMENDED_PLOTS);
					}

#ifdef FASSERT_ENABLE
					if (pBestExplorePlot != NULL && pBestExplorePlotThisTurn != pBestExplorePlot)
					{
						gDLL->getEngineIFace()->addColoredPlot(pBestExplorePlot->getX_INLINE(), pBestExplorePlot->getY_INLINE(), GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_NEGATIVE_TEXT")).getColor(), PLOT_STYLE_CIRCLE, PLOT_LANDSCAPE_LAYER_RECOMMENDED_PLOTS);
					}
#endif
				}
			}
#endif

		}
	}
}


void CvGame::updatePlotGroups()
{
	PROFILE_FUNC();

	int iI;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			GET_PLAYER((PlayerTypes)iI).updatePlotGroups();
		}
	}
}


void CvGame::updateBuildingCommerce()
{
	int iI;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			GET_PLAYER((PlayerTypes)iI).updateBuildingCommerce();
		}
	}
}


void CvGame::updateCitySight(bool bIncrement)
{
	int iI;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			GET_PLAYER((PlayerTypes)iI).updateCitySight(bIncrement);
		}
	}
}


void CvGame::updateTradeRoutes()
{
	int iI;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			GET_PLAYER((PlayerTypes)iI).updateTradeRoutes();
		}
	}
}


void CvGame::updateSelectionList()
{
	CvUnit* pHeadSelectedUnit;

	if (GET_PLAYER(getActivePlayer()).isOption(PLAYEROPTION_NO_UNIT_CYCLING))
	{
		return;
	}

	pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();

	if ((pHeadSelectedUnit == NULL) || !(pHeadSelectedUnit->getGroup()->readyToSelect(true)))
	{
		if ((gDLL->getInterfaceIFace()->getOriginalPlot() == NULL) || !(cyclePlotUnits(gDLL->getInterfaceIFace()->getOriginalPlot(), true, true, gDLL->getInterfaceIFace()->getOriginalPlotCount())))
		{
			if ((gDLL->getInterfaceIFace()->getSelectionPlot() == NULL) || !(cyclePlotUnits(gDLL->getInterfaceIFace()->getSelectionPlot(), true, true)))
			{
				cycleSelectionGroups(true);
			}
		}

		pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();

		if (pHeadSelectedUnit != NULL)
		{
			if (!(pHeadSelectedUnit->getGroup()->readyToSelect()))
			{
				gDLL->getInterfaceIFace()->clearSelectionList();
			}
		}
	}
}


void CvGame::updateTestEndTurn()
{
	bool bAny;

	bAny = ((gDLL->getInterfaceIFace()->getHeadSelectedUnit() != NULL) && !(GET_PLAYER(getActivePlayer()).isOption(PLAYEROPTION_NO_UNIT_CYCLING)));

	if (GET_PLAYER(getActivePlayer()).isTurnActive())
	{
		if (gDLL->getInterfaceIFace()->isEndTurnMessage())
		{
			if (GET_PLAYER(getActivePlayer()).hasReadyUnit(bAny))
			{
				gDLL->getInterfaceIFace()->setEndTurnMessage(false);
			}
		}
		else
		{
			if (!(GET_PLAYER(getActivePlayer()).hasBusyUnit()) && !(GET_PLAYER(getActivePlayer()).hasReadyUnit(bAny)))
			{
				if (!(gDLL->getInterfaceIFace()->isForcePopup()))
				{
					gDLL->getInterfaceIFace()->setForcePopup(true);
				}
				else
				{
					if (GET_PLAYER(getActivePlayer()).hasAutoUnit())
					{
						if (!(gDLL->shiftKey()))
						{
							gDLL->sendAutoMoves();
						}
					}
					else
					{
						if (GET_PLAYER(getActivePlayer()).isOption(PLAYEROPTION_WAIT_END_TURN) || !(gDLL->getInterfaceIFace()->isHasMovedUnit()) || isHotSeat() || isPbem())
						{
							gDLL->getInterfaceIFace()->setEndTurnMessage(true);
						}
						else
						{
							if (gDLL->getInterfaceIFace()->getEndTurnCounter() > 0)
							{
								gDLL->getInterfaceIFace()->changeEndTurnCounter(-1);
							}
							else
							{
								gDLL->sendTurnComplete();
								gDLL->getInterfaceIFace()->setEndTurnCounter(3); // XXX
							}
						}
					}
				}
			}
		}
	}
}


void CvGame::testExtendedGame()
{
	int iI;

	if (getGameState() != GAMESTATE_OVER)
	{
		return;
	}

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).isHuman())
			{
				if (GET_PLAYER((PlayerTypes)iI).isExtendedGame())
				{
					setGameState(GAMESTATE_EXTENDED);
					break;
				}
			}
		}
	}
}


CvUnit* CvGame::getPlotUnit(const CvPlot* pPlot, int iIndex)
{
	CLLNode<IDInfo>* pUnitNode1;
	CLLNode<IDInfo>* pUnitNode2;
	CvUnit* pLoopUnit1;
	CvUnit* pLoopUnit2;
	int iCount;
	int iPass;

	if (pPlot != NULL)
	{
		iCount = 0;

		for (iPass = 0; iPass < 2; iPass++)
		{
			pUnitNode1 = pPlot->headUnitNode();

			while (pUnitNode1 != NULL)
			{
				pLoopUnit1 = ::getUnit(pUnitNode1->m_data);
				pUnitNode1 = pPlot->nextUnitNode(pUnitNode1);

				if (!(pLoopUnit1->isInvisible(getActiveTeam(), true)))
				{
					if (!(pLoopUnit1->isCargo()))
					{
						if ((pLoopUnit1->getOwner() == getActivePlayer()) == (iPass == 0))
						{
							if (iCount == iIndex)
							{
								return pLoopUnit1;
							}

							iCount++;

							//if ((pLoopUnit1->getTeam() == getActiveTeam()) || isDebugMode())
							{
								if (pLoopUnit1->hasCargo())
								{
									pUnitNode2 = pPlot->headUnitNode();

									while (pUnitNode2 != NULL)
									{
										pLoopUnit2 = ::getUnit(pUnitNode2->m_data);
										pUnitNode2 = pPlot->nextUnitNode(pUnitNode2);

										if (!(pLoopUnit2->isInvisible(getActiveTeam(), true)))
										{
											if (pLoopUnit2->getTransportUnit() == pLoopUnit1)
											{
												if (iCount == iIndex)
												{
													return pLoopUnit2;
												}

												iCount++;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return NULL;
}


void CvGame::cycleCities(bool bForward, bool bAdd)
{
	CvCity* pHeadSelectedCity;
	CvCity* pSelectCity;
	CvCity* pLoopCity;
	int iLoop;

	pSelectCity = NULL;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if ((pHeadSelectedCity != NULL) && ((pHeadSelectedCity->getTeam() == getActiveTeam()) || isDebugMode()))
	{
		iLoop = pHeadSelectedCity->getIndex();
		iLoop += (bForward ? 1 : -1);

		pLoopCity = GET_PLAYER(pHeadSelectedCity->getOwner()).nextCity(&iLoop, !bForward);

		if (pLoopCity == NULL)
		{
			pLoopCity = GET_PLAYER(pHeadSelectedCity->getOwner()).firstCity(&iLoop, !bForward);
		}

		if ((pLoopCity != NULL) && (pLoopCity != pHeadSelectedCity))
		{
			pSelectCity = pLoopCity;
		}
	}
	else
	{
		pSelectCity = GET_PLAYER(getActivePlayer()).firstCity(&iLoop, !bForward);
	}

	if (pSelectCity != NULL)
	{
		if (bAdd)
		{
			gDLL->getInterfaceIFace()->clearSelectedCities();
			gDLL->getInterfaceIFace()->addSelectedCity(pSelectCity);
		}
		else
		{
			gDLL->getInterfaceIFace()->selectCity(pSelectCity);
		}
	}
}


void CvGame::cycleSelectionGroups(bool bClear, bool bForward, bool bWorkers)
{
	CvSelectionGroup* pNextSelectionGroup;
	CvPlot* pPlot;
	CvUnit* pCycleUnit;
	bool bWrap;

	pCycleUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();

	if (pCycleUnit != NULL)
	{
		if (pCycleUnit->getOwner() != getActivePlayer())
		{
			pCycleUnit = NULL;
		}

		pNextSelectionGroup = GET_PLAYER(getActivePlayer()).cycleSelectionGroups(pCycleUnit, bForward, bWorkers, &bWrap);

		if (bWrap)
		{
			if (GET_PLAYER(getActivePlayer()).hasAutoUnit())
			{
				gDLL->sendAutoMoves();
			}
		}
	}
	else
	{
		pPlot = gDLL->getInterfaceIFace()->getLookAtPlot();
		pNextSelectionGroup = GC.getMapINLINE().findSelectionGroup(((pPlot != NULL) ? pPlot->getX() : 0), ((pPlot != NULL) ? pPlot->getY() : 0), getActivePlayer(), true, bWorkers);
	}

	if (pNextSelectionGroup != NULL)
	{
		FAssert(pNextSelectionGroup->getOwner() == getActivePlayer());
		gDLL->getInterfaceIFace()->selectUnit(pNextSelectionGroup->getHeadUnit(), bClear);
	}

	if ((pCycleUnit != gDLL->getInterfaceIFace()->getHeadSelectedUnit()) || ((pCycleUnit != NULL) && pCycleUnit->getGroup()->readyToSelect()))
	{
		gDLL->getInterfaceIFace()->lookAtSelectionPlot();
	}
}


// Returns true if unit was cycled...
bool CvGame::cyclePlotUnits(CvPlot* pPlot, bool bForward, bool bAuto, int iCount)
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pSelectedUnit;
	CvUnit* pLoopUnit;

	FAssertMsg(iCount >= -1, "iCount expected to be >= -1");

	if (iCount == -1)
	{
		pUnitNode = pPlot->headUnitNode();

		while (pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);

			if (pLoopUnit->IsSelected())
			{
				break;
			}

			pUnitNode = pPlot->nextUnitNode(pUnitNode);
		}
	}
	else
	{
		pUnitNode = pPlot->headUnitNode();

		while (pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);

			if ((iCount - 1) == 0)
			{
				break;
			}

			if (iCount > 0)
			{
				iCount--;
			}

			pUnitNode = pPlot->nextUnitNode(pUnitNode);
		}

		if (pUnitNode == NULL)
		{
			pUnitNode = pPlot->tailUnitNode();

			if (pUnitNode != NULL)
			{
				pLoopUnit = ::getUnit(pUnitNode->m_data);
			}
		}
	}

	if (pUnitNode != NULL)
	{
		pSelectedUnit = pLoopUnit;

		while (true)
		{
			if (bForward)
			{
				pUnitNode = pPlot->nextUnitNode(pUnitNode);
				if (pUnitNode == NULL)
				{
					pUnitNode = pPlot->headUnitNode();
				}
			}
			else
			{
				pUnitNode = pPlot->prevUnitNode(pUnitNode);
				if (pUnitNode == NULL)
				{
					pUnitNode = pPlot->tailUnitNode();
				}
			}

			pLoopUnit = ::getUnit(pUnitNode->m_data);

			if (iCount == -1)
			{
				if (pLoopUnit == pSelectedUnit)
				{
					break;
				}
			}

			if (pLoopUnit->getOwner() == getActivePlayer())
			{
				if (bAuto)
				{
					if (pLoopUnit->getGroup()->readyToSelect())
					{
						gDLL->getInterfaceIFace()->selectUnit(pLoopUnit, true);
						return true;
					}
				}
				else
				{
					gDLL->getInterfaceIFace()->insertIntoSelectionList(pLoopUnit, true, false);
					return true;
				}
			}

			if (pLoopUnit == pSelectedUnit)
			{
				break;
			}
		}
	}

	return false;
}


void CvGame::selectionListMove(CvPlot* pPlot, bool bAlt, bool bShift, bool bCtrl)
{
	CLLNode<IDInfo>* pSelectedUnitNode;
	CvUnit* pHeadSelectedUnit;
	CvUnit* pSelectedUnit;
	TeamTypes eRivalTeam;

	if (pPlot == NULL)
	{
		return;
	}

	CyPlot* pyPlot = new CyPlot(pPlot);
	CyArgsList argsList;
	argsList.add(gDLL->getPythonIFace()->makePythonObject(pyPlot));	// pass in plot class
	argsList.add(bAlt);
	argsList.add(bShift);
	argsList.add(bCtrl);
	long lResult=0;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "cannotSelectionListMove", argsList.makeFunctionArgs(), &lResult);
	delete pyPlot;	// python fxn must not hold on to this pointer 
	if (lResult == 1)
	{
		return;
	}

	pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();

	if ((pHeadSelectedUnit == NULL) || (pHeadSelectedUnit->getOwner() != getActivePlayer()))
	{
		return;
	}

	if (bAlt)
	{
		gDLL->getInterfaceIFace()->selectGroup(pHeadSelectedUnit, false, false, true);
	}
	else if (bCtrl)
	{
		gDLL->getInterfaceIFace()->selectGroup(pHeadSelectedUnit, false, true, false);
	}

	pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

	while (pSelectedUnitNode != NULL)
	{
		pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

		eRivalTeam = pSelectedUnit->getDeclareWarMove(pPlot);

		if (eRivalTeam != NO_TEAM)
		{
			CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_DECLAREWARMOVE);
			if (NULL != pInfo)
			{
				pInfo->setData1(eRivalTeam);
				pInfo->setData2(pPlot->getX());
				pInfo->setData3(pPlot->getY());
				pInfo->setOption1(bShift);
				pInfo->setOption2(pPlot->getTeam() != eRivalTeam);
				gDLL->getInterfaceIFace()->addPopup(pInfo);
			}
			return;
		}

		pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
	}

	selectionListGameNetMessage(GAMEMESSAGE_PUSH_MISSION, MISSION_MOVE_TO, pPlot->getX(), pPlot->getY(), 0, false, bShift);
}


void CvGame::selectionListGameNetMessage(int eMessage, int iData2, int iData3, int iData4, int iFlags, bool bAlt, bool bShift)
{
	CLLNode<IDInfo>* pSelectedUnitNode;
	CvUnit* pHeadSelectedUnit;
	CvUnit* pSelectedUnit;

	CyArgsList argsList;
	argsList.add(eMessage);	// pass in plot class
	argsList.add(iData2);
	argsList.add(iData3);
	argsList.add(iData4);
	argsList.add(iFlags);
	argsList.add(bAlt);
	argsList.add(bShift);
	long lResult=0;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "cannotSelectionListGameNetMessage", argsList.makeFunctionArgs(), &lResult);
	if (lResult == 1)
	{
		return;
	}

	pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();

	if (pHeadSelectedUnit != NULL)
	{
		if (pHeadSelectedUnit->getOwner() == getActivePlayer())
		{
			if (eMessage == GAMEMESSAGE_JOIN_GROUP)
			{
				pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

				while (pSelectedUnitNode != NULL)
				{
					pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);
					pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);

					if (bShift)
					{
						gDLL->sendJoinGroup(pSelectedUnit->getID(), FFreeList::INVALID_INDEX);
					}
					else
					{
						if (pSelectedUnit == pHeadSelectedUnit)
						{
							gDLL->sendJoinGroup(pSelectedUnit->getID(), FFreeList::INVALID_INDEX);
						}

						gDLL->sendJoinGroup(pSelectedUnit->getID(), pHeadSelectedUnit->getID());
					}
				}

				if (bShift)
				{
					gDLL->getInterfaceIFace()->selectUnit(pHeadSelectedUnit, true);
				}
			}
			else if (eMessage == GAMEMESSAGE_DO_COMMAND)
			{
				pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

				while (pSelectedUnitNode != NULL)
				{
					pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);
					pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);

					gDLL->sendDoCommand(pSelectedUnit->getID(), ((CommandTypes)iData2), iData3, iData4, bAlt);
				}
			}
			else if ((eMessage == GAMEMESSAGE_PUSH_MISSION) || (eMessage == GAMEMESSAGE_AUTO_MISSION))
			{
				if (!(gDLL->getInterfaceIFace()->mirrorsSelectionGroup()))
				{
					selectionListGameNetMessage(GAMEMESSAGE_JOIN_GROUP);
				}

				if (eMessage == GAMEMESSAGE_PUSH_MISSION)
				{
					gDLL->sendPushMission(pHeadSelectedUnit->getID(), ((MissionTypes)iData2), iData3, iData4, iFlags, bShift);
				}
				else
				{
					gDLL->sendAutoMission(pHeadSelectedUnit->getID());
				}
			}
			else
			{
				FAssert(false);
			}
		}
	}
}


void CvGame::selectedCitiesGameNetMessage(int eMessage, int iData2, int iData3, int iData4, bool bOption, bool bAlt, bool bShift, bool bCtrl)
{
	CLLNode<IDInfo>* pSelectedCityNode;
	CvCity* pSelectedCity;

	pSelectedCityNode = gDLL->getInterfaceIFace()->headSelectedCitiesNode();

	while (pSelectedCityNode != NULL)
	{
		pSelectedCity = ::getCity(pSelectedCityNode->m_data);
		pSelectedCityNode = gDLL->getInterfaceIFace()->nextSelectedCitiesNode(pSelectedCityNode);

		if (pSelectedCity != NULL)
		{
			if (pSelectedCity->getOwner() == getActivePlayer())
			{
				switch (eMessage)
				{
				case GAMEMESSAGE_PUSH_ORDER:
					cityPushOrder(pSelectedCity, ((OrderTypes)iData2), iData3, bAlt, bShift, bCtrl);
					break;

				case GAMEMESSAGE_POP_ORDER:
					if (pSelectedCity->getOrderQueueLength() > 1)
					{
						gDLL->sendPopOrder(pSelectedCity->getID(), iData2);
					}
					break;

				case GAMEMESSAGE_DO_TASK:
					gDLL->sendDoTask(pSelectedCity->getID(), ((TaskTypes)iData2), iData3, iData4, bOption);
					break;

				default:
					FAssert(false);
					break;
				}
			}
		}
	}
}


void CvGame::cityPushOrder(CvCity* pCity, OrderTypes eOrder, int iData, bool bAlt, bool bShift, bool bCtrl)
{
	if (pCity->getProduction() > 0)
	{
		gDLL->sendPushOrder(pCity->getID(), eOrder, iData, bAlt, bShift, !bShift);
	}
	else if ((eOrder == ORDER_TRAIN) && (pCity->getProductionUnit() == iData))
	{
		gDLL->sendPushOrder(pCity->getID(), eOrder, iData, bAlt, !bCtrl, bCtrl);
	}
	else
	{
		gDLL->sendPushOrder(pCity->getID(), eOrder, iData, bAlt, bShift, bCtrl);
	}
}


void CvGame::selectUnit(CvUnit* pUnit, bool bClear, bool bToggle, bool bSound)
{
	CLLNode<IDInfo>* pEntityNode;
	CvSelectionGroup* pSelectionGroup;
	bool bSelectGroup;
	bool bGroup;

	if (gDLL->getInterfaceIFace()->getHeadSelectedUnit() == NULL)
	{
		bSelectGroup = true;
	}
	else if (gDLL->getInterfaceIFace()->getHeadSelectedUnit()->getGroup() != pUnit->getGroup())
	{
		bSelectGroup = true;
	}
	else if (pUnit->IsSelected() && !(gDLL->getInterfaceIFace()->mirrorsSelectionGroup()))
	{
		bSelectGroup = !bToggle;
	}
	else
	{
		bSelectGroup = false;
	}

	gDLL->getInterfaceIFace()->clearSelectedCities();

	if (bClear)
	{
		gDLL->getInterfaceIFace()->clearSelectionList();
		bGroup = false;
	}
	else
	{
		bGroup = gDLL->getInterfaceIFace()->mirrorsSelectionGroup();
	}

	if (bSelectGroup)
	{
		pSelectionGroup = pUnit->getGroup();

		pEntityNode = pSelectionGroup->headUnitNode();

		while (pEntityNode != NULL)
		{
			FAssertMsg(::getUnit(pEntityNode->m_data), "null entity in selection group");
			gDLL->getInterfaceIFace()->insertIntoSelectionList(::getUnit(pEntityNode->m_data), false, bToggle, bGroup, bSound);

			pEntityNode = pSelectionGroup->nextUnitNode(pEntityNode);
		}
	}
	else
	{
		gDLL->getInterfaceIFace()->insertIntoSelectionList(pUnit, false, bToggle, bGroup, bSound);
	}

	gDLL->getInterfaceIFace()->makeSelectionListDirty();
}


void CvGame::selectGroup(CvUnit* pUnit, bool bShift, bool bCtrl, bool bAlt)
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pUnitPlot;
	bool bGroup;

	FAssertMsg(pUnit != NULL, "pUnit == NULL unexpectedly");

	if (bAlt || bCtrl)
	{
		gDLL->getInterfaceIFace()->clearSelectedCities();

		if (!bShift)
		{
			gDLL->getInterfaceIFace()->clearSelectionList();
			bGroup = true;
		}
		else
		{
			bGroup = gDLL->getInterfaceIFace()->mirrorsSelectionGroup();
		}

		pUnitPlot = pUnit->plot();

		pUnitNode = pUnitPlot->headUnitNode();

		while (pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = pUnitPlot->nextUnitNode(pUnitNode);

			if (pLoopUnit->getOwner() == getActivePlayer())
			{
				if (pLoopUnit->canMove())
				{
					if (!isMPOption(MPOPTION_SIMULTANEOUS_TURNS) || getTurnSlice() - pLoopUnit->getLastMoveTurn() > GC.getDefineINT("MIN_TIMER_UNIT_DOUBLE_MOVES"))
					{
						if (bAlt || (pLoopUnit->getUnitType() == pUnit->getUnitType()))
						{
							gDLL->getInterfaceIFace()->insertIntoSelectionList(pLoopUnit, false, false, bGroup);
						}
					}
				}
			}
		}
	}
	else
	{
		gDLL->getInterfaceIFace()->selectUnit(pUnit, !bShift, bShift, true);
	}
}


void CvGame::selectAll(CvPlot* pPlot)
{
	CvUnit* pSelectUnit;
	CvUnit* pCenterUnit;

	pSelectUnit = NULL;

	if (pPlot != NULL)
	{
		pCenterUnit = pPlot->getDebugCenterUnit();

		if ((pCenterUnit != NULL) && (pCenterUnit->getOwner() == getActivePlayer()))
		{
			pSelectUnit = pCenterUnit;
		}
	}

	if (pSelectUnit != NULL)
	{
		gDLL->getInterfaceIFace()->selectGroup(pSelectUnit, false, false, true);
	}
}


bool CvGame::selectionListIgnoreBuildingDefense()
{
	CLLNode<IDInfo>* pSelectedUnitNode;
	CvUnit* pSelectedUnit;
	bool bIgnoreBuilding;
	bool bAttackLandUnit;

	bIgnoreBuilding = false;
	bAttackLandUnit = false;

	pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

	while (pSelectedUnitNode != NULL)
	{
		pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);
		pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);

		if (pSelectedUnit != NULL)
		{
			if (pSelectedUnit->ignoreBuildingDefense())
			{
				bIgnoreBuilding = true;
			}

			if ((pSelectedUnit->getDomainType() == DOMAIN_LAND) && pSelectedUnit->canAttack())
			{
				bAttackLandUnit = true;
			}
		}
	}

	if (!bIgnoreBuilding && !bAttackLandUnit)
	{
		if (GC.getGameINLINE().getBestLandUnit() != NO_UNIT)
		{
			bIgnoreBuilding = GC.getUnitInfo(GC.getGameINLINE().getBestLandUnit()).isIgnoreBuildingDefense();
		}
	}

	return bIgnoreBuilding;
}


bool CvGame::canHandleAction(int iAction, CvPlot* pPlot, bool bTestVisible)
{
	PROFILE_FUNC();

	CvSelectionGroup* pSelectedGroup;
	CvUnit* pHeadSelectedUnit;
	CvPlot* pMissionPlot;
	bool bShift;

	bShift = gDLL->shiftKey();

	CyPlot* pyPlot = new CyPlot(pPlot);
	CyArgsList argsList;
	argsList.add(gDLL->getPythonIFace()->makePythonObject(pyPlot));	// pass in plot class
	argsList.add(iAction);
	argsList.add(bTestVisible);
	long lResult=0;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "cannotHandleAction", argsList.makeFunctionArgs(), &lResult);
	delete pyPlot;	// python fxn must not hold on to this pointer 
	if (lResult == 1)
	{
		return false;
	}

	if (GC.getActionInfo(iAction).getControlType() != NO_CONTROL)
	{
		if (canDoControl((ControlTypes)(GC.getActionInfo(iAction).getControlType())))
		{
			return true;
		}
	}

	if (gDLL->getInterfaceIFace()->isCitySelection())
	{
		return false; // XXX hack!
	}

	pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();

	if (pHeadSelectedUnit != NULL)
	{
		if (pHeadSelectedUnit->getOwner() == getActivePlayer())
		{
			if (gDLL->getInterfaceIFace()->mirrorsSelectionGroup())
			{
				pSelectedGroup = pHeadSelectedUnit->getGroup();

				if (GC.getActionInfo(iAction).getMissionType() != NO_MISSION)
				{
					if (pPlot != NULL)
					{
						pMissionPlot = pPlot;
					}
					else if (bShift)
					{
						pMissionPlot = pSelectedGroup->lastMissionPlot();
					}
					else
					{
						pMissionPlot = NULL;
					}

					if ((pMissionPlot == NULL) || !(pMissionPlot->isVisible(pHeadSelectedUnit->getTeam(), false)))
					{
						pMissionPlot = pSelectedGroup->plot();
					}

					if (pSelectedGroup->canStartMission(GC.getActionInfo(iAction).getMissionType(), GC.getActionInfo(iAction).getMissionData(), -1, pMissionPlot, bTestVisible))
					{
						return true;
					}
				}

				if (GC.getActionInfo(iAction).getCommandType() != NO_COMMAND)
				{
					if (pSelectedGroup->canDoCommand(((CommandTypes)(GC.getActionInfo(iAction).getCommandType())), GC.getActionInfo(iAction).getCommandData(), -1, bTestVisible))
					{
						return true;
					}
				}

				if (gDLL->getInterfaceIFace()->canDoInterfaceMode(((InterfaceModeTypes)GC.getActionInfo(iAction).getInterfaceModeType()), pSelectedGroup))
				{
					return true;
				}
			}
			else
			{
				if (GC.getActionInfo(iAction).getMissionType() != NO_MISSION)
				{
					if (gDLL->getInterfaceIFace()->getSelectionList()->canStartMission(GC.getActionInfo(iAction).getMissionType(), GC.getActionInfo(iAction).getMissionData(), -1, gDLL->getInterfaceIFace()->getSelectionList()->plot(), bTestVisible))
					{
						return true;
					}
				}

				if (GC.getActionInfo(iAction).getCommandType() != NO_COMMAND)
				{
					if (gDLL->getInterfaceIFace()->getSelectionList()->canDoCommand(((CommandTypes)(GC.getActionInfo(iAction).getCommandType())), GC.getActionInfo(iAction).getCommandData(), -1, bTestVisible))
					{
						return true;
					}
				}

				if (gDLL->getInterfaceIFace()->canDoInterfaceMode(((InterfaceModeTypes)GC.getActionInfo(iAction).getInterfaceModeType()), gDLL->getInterfaceIFace()->getSelectionList()))
				{
					return true;
				}
			}
		}
	}

	return false;
}


void CvGame::handleAction(int iAction)
{
	CvUnit* pHeadSelectedUnit;
	bool bAlt;
	bool bShift;
	bool bSkip;

	bAlt = gDLL->altKey();
	bShift = gDLL->shiftKey();

	if (!(gDLL->getInterfaceIFace()->canHandleAction(iAction)))
	{
		return;
	}

	if (GC.getActionInfo(iAction).getControlType() != NO_CONTROL)
	{
		doControl((ControlTypes)(GC.getActionInfo(iAction).getControlType()));
	}

	if (gDLL->getInterfaceIFace()->canDoInterfaceMode((InterfaceModeTypes)GC.getActionInfo(iAction).getInterfaceModeType(), gDLL->getInterfaceIFace()->getSelectionList()))
	{
		pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();

		if (pHeadSelectedUnit != NULL)
		{
			if (GC.getInterfaceModeInfo((InterfaceModeTypes)GC.getActionInfo(iAction).getInterfaceModeType()).getSelectAll())
			{
				gDLL->getInterfaceIFace()->selectGroup(pHeadSelectedUnit, false, false, true);
			}
			else if (GC.getInterfaceModeInfo((InterfaceModeTypes)GC.getActionInfo(iAction).getInterfaceModeType()).getSelectType())
			{
				gDLL->getInterfaceIFace()->selectGroup(pHeadSelectedUnit, false, true, false);
			}
		}

		gDLL->getInterfaceIFace()->setInterfaceMode((InterfaceModeTypes)GC.getActionInfo(iAction).getInterfaceModeType());
	}

	if (GC.getActionInfo(iAction).getMissionType() != NO_MISSION)
	{
		selectionListGameNetMessage(GAMEMESSAGE_PUSH_MISSION, GC.getActionInfo(iAction).getMissionType(), GC.getActionInfo(iAction).getMissionData(), -1, 0, false, bShift);
	}

	if (GC.getActionInfo(iAction).getCommandType() != NO_COMMAND)
	{
		bSkip = false;

		if (GC.getActionInfo(iAction).getCommandType() == COMMAND_LOAD)
		{
			CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_LOADUNIT);
			if (NULL != pInfo)
			{
				gDLL->getInterfaceIFace()->addPopup(pInfo);
				bSkip = true;
			}
		}

		if (!bSkip)
		{
			if (GC.getActionInfo(iAction).isConfirmCommand())
			{
				CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_CONFIRMCOMMAND);
				if (NULL != pInfo)
				{
					pInfo->setData1(iAction);
					pInfo->setOption1(bAlt);
					gDLL->getInterfaceIFace()->addPopup(pInfo);
				}
			}
			else
			{
				selectionListGameNetMessage(GAMEMESSAGE_DO_COMMAND, GC.getActionInfo(iAction).getCommandType(), GC.getActionInfo(iAction).getCommandData(), -1, 0, bAlt);
			}
		}
	}
}


bool CvGame::canDoControl(ControlTypes eControl)
{
	CyArgsList argsList;
	argsList.add(eControl);
	long lResult=0;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "cannotDoControl", argsList.makeFunctionArgs(), &lResult);
	if (lResult == 1)
	{
		return false;
	}

	switch (eControl)
	{
	case CONTROL_SELECTYUNITTYPE:
	case CONTROL_SELECTYUNITALL:
	case CONTROL_SELECT_HEALTHY:
	case CONTROL_SELECTCITY:
	case CONTROL_SELECTCAPITAL:
	case CONTROL_NEXTUNIT:
	case CONTROL_PREVUNIT:
	case CONTROL_CYCLEUNIT:
	case CONTROL_CYCLEUNIT_ALT:
	case CONTROL_CYCLEWORKER:
	case CONTROL_LASTUNIT:
	case CONTROL_FORCEENDTURN:
	case CONTROL_AUTOMOVES:
	case CONTROL_SAVE_GROUP:
	case CONTROL_QUICK_SAVE:
	case CONTROL_QUICK_LOAD:
	case CONTROL_ORTHO_CAMERA:
	case CONTROL_CYCLE_CAMERA_FLYING_MODES:
	case CONTROL_ISOMETRIC_CAMERA_LEFT:
	case CONTROL_ISOMETRIC_CAMERA_RIGHT:
	case CONTROL_FLYING_CAMERA:
	case CONTROL_MOUSE_FLYING_CAMERA:
	case CONTROL_TOP_DOWN_CAMERA:
	case CONTROL_TURN_LOG:
	case CONTROL_CHAT_ALL:
	case CONTROL_CHAT_TEAM:
	case CONTROL_GLOBE_VIEW:
		if (!gDLL->getInterfaceIFace()->isFocused())
		{
			return true;
		}
		break;

	case CONTROL_PING:
	case CONTROL_SIGN:
	case CONTROL_GRID:
	case CONTROL_BARE_MAP:
	case CONTROL_YIELDS:
	case CONTROL_RESOURCE_ALL:
	case CONTROL_GLOBELAYER:
	case CONTROL_SCORES:
		if (!gDLL->getInterfaceIFace()->isFocusedWidget())
		{
			return true;
		}
		break;

	case CONTROL_OPTIONS_SCREEN:
	case CONTROL_DOMESTIC_SCREEN:
	case CONTROL_VICTORY_SCREEN:
	case CONTROL_CIVILOPEDIA:
	case CONTROL_RELIGION_SCREEN:
	case CONTROL_CIVICS_SCREEN:
	case CONTROL_FOREIGN_SCREEN:
	case CONTROL_FINANCIAL_SCREEN:
	case CONTROL_MILITARY_SCREEN:
	case CONTROL_TECH_CHOOSER:
	case CONTROL_DIPLOMACY:
	case CONTROL_HALL_OF_FAME:
	case CONTROL_INFO:
	case CONTROL_DETAILS:
	case CONTROL_SAVE_NORMAL:
	case CONTROL_NEXTCITY:
	case CONTROL_PREVCITY:
		return true;
		break;

	case CONTROL_ADMIN_DETAILS:
		if (gDLL->isFMPMgrHost())
		{
			return true;
		}
		break;

	case CONTROL_CENTERONSELECTION:
		if (gDLL->getInterfaceIFace()->getLookAtPlot() != gDLL->getInterfaceIFace()->getSelectionPlot())
		{
			return true;
		}
		break;

	case CONTROL_LOAD_GAME:
		if (!(isNetworkMultiPlayer()))
		{
			return true;
		}
		break;

	case CONTROL_RETIRE:
		if ((getGameState() == GAMESTATE_ON) || GC.getGameINLINE().isGameMultiPlayer())
		{
			if (GET_PLAYER(getActivePlayer()).isAlive())
			{
				if (GC.getGameINLINE().isPbem() || GC.getGameINLINE().isHotSeat())
				{
					if (!GET_PLAYER(getActivePlayer()).isEndTurn())
					{
						return true;
					}
				}
				else
				{
					return true;
				}
			}
		}
		break;

	case CONTROL_WORLD_BUILDER:
		if (!(isGameMultiPlayer()) && GC.getInitCore().getAdminPassword().empty())
		{
			return true;
		}
		break;

	case CONTROL_ENDTURN:
	case CONTROL_ENDTURN_ALT:
		if (gDLL->getInterfaceIFace()->isEndTurnMessage() && !gDLL->getInterfaceIFace()->isFocused())
		{
			return true;
		}
		break;

	default:
		FAssertMsg(false, "eControl did not match any valid options");
		break;
	}

	return false;
}


void CvGame::doControl(ControlTypes eControl)
{
	CvPopupInfo* pInfo;
	CvCity* pCapitalCity;
	CvUnit* pHeadSelectedUnit;
	CvUnit* pUnit;
	CvPlot* pPlot;

	if (!canDoControl(eControl))
	{
		return;
	}

	switch (eControl)
	{
	case CONTROL_CENTERONSELECTION:
		gDLL->getInterfaceIFace()->lookAtSelectionPlot();
		break;

	case CONTROL_SELECTYUNITTYPE:
		pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();
		if (pHeadSelectedUnit != NULL)
		{
			gDLL->getInterfaceIFace()->selectGroup(pHeadSelectedUnit, false, true, false);
		}
		break;

	case CONTROL_SELECTYUNITALL:
		pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();
		if (pHeadSelectedUnit != NULL)
		{
			gDLL->getInterfaceIFace()->selectGroup(pHeadSelectedUnit, false, false, true);
		}
		break;

	case CONTROL_SELECT_HEALTHY:
		{
			CvSelectionGroup* pGroup = NULL;
			pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();
			gDLL->getInterfaceIFace()->clearSelectionList();
			if (pHeadSelectedUnit != NULL)
			{
				CvPlot* pPlot = pHeadSelectedUnit->plot();
				for (int iI = 0; iI < pPlot->getNumUnits(); iI++)
				{
					pUnit = gDLL->getInterfaceIFace()->getInterfacePlotUnit(pPlot, iI);

					if (pUnit->getOwner() == getActivePlayer())
					{
						if (!isMPOption(MPOPTION_SIMULTANEOUS_TURNS) || getTurnSlice() - pUnit->getLastMoveTurn() > GC.getDefineINT("MIN_TIMER_UNIT_DOUBLE_MOVES"))
						{
							if (pUnit->isHurt())
							{
								if (pGroup)
								{
									pUnit->joinGroup(pGroup);
								}
								else
								{
									pGroup = pUnit->getGroup();
								}

								gDLL->getInterfaceIFace()->insertIntoSelectionList(pUnit, false, false, true, true);
							}
						}
					}
				}
			}
		}
		break;

	case CONTROL_SELECTCITY:
		if (gDLL->getInterfaceIFace()->isCityScreenUp())
		{
			cycleCities();
		}
		else
		{
			gDLL->getInterfaceIFace()->selectLookAtCity();
		}
		break;

	case CONTROL_SELECTCAPITAL:
		pCapitalCity = GET_PLAYER(getActivePlayer()).getCapitalCity();
		if (pCapitalCity != NULL)
		{
			gDLL->getInterfaceIFace()->selectCity(pCapitalCity);
		}
		break;

	case CONTROL_NEXTCITY:
		if (gDLL->getInterfaceIFace()->isCitySelection())
		{
			cycleCities(true, !(gDLL->getInterfaceIFace()->isCityScreenUp()));
		}
		else
		{
			gDLL->getInterfaceIFace()->selectLookAtCity(true);
		}
		gDLL->getInterfaceIFace()->lookAtSelectionPlot();
		break;

	case CONTROL_PREVCITY:
		if (gDLL->getInterfaceIFace()->isCitySelection())
		{
			cycleCities(false, !(gDLL->getInterfaceIFace()->isCityScreenUp()));
		}
		else
		{
			gDLL->getInterfaceIFace()->selectLookAtCity(true);
		}
		gDLL->getInterfaceIFace()->lookAtSelectionPlot();
		break;

	case CONTROL_NEXTUNIT:
		pPlot = gDLL->getInterfaceIFace()->getSelectionPlot();
		if (pPlot != NULL)
		{
			cyclePlotUnits(pPlot);
		}
		break;

	case CONTROL_PREVUNIT:
		pPlot = gDLL->getInterfaceIFace()->getSelectionPlot();
		if (pPlot != NULL)
		{
			cyclePlotUnits(pPlot, false);
		}
		break;

	case CONTROL_CYCLEUNIT:
	case CONTROL_CYCLEUNIT_ALT:
		cycleSelectionGroups(true);
		break;

	case CONTROL_CYCLEWORKER:
		cycleSelectionGroups(true, true, true);
		break;

	case CONTROL_LASTUNIT:
		pUnit = gDLL->getInterfaceIFace()->getLastSelectedUnit();

		if (pUnit != NULL)
		{
			gDLL->getInterfaceIFace()->selectUnit(pUnit, true);
			gDLL->getInterfaceIFace()->lookAtSelectionPlot();
		}
		else
		{
			cycleSelectionGroups(true, false);
		}

		gDLL->getInterfaceIFace()->setLastSelectedUnit(NULL);
		break;

	case CONTROL_ENDTURN:
	case CONTROL_ENDTURN_ALT:
		if (gDLL->getInterfaceIFace()->isEndTurnMessage())
		{
			gDLL->sendTurnComplete();
		}
		break;

	case CONTROL_FORCEENDTURN:
		gDLL->sendTurnComplete();
		break;

	case CONTROL_AUTOMOVES:
		gDLL->sendAutoMoves();
		break;

	case CONTROL_PING:
		gDLL->getInterfaceIFace()->setInterfaceMode(INTERFACEMODE_PING);
		break;

	case CONTROL_SIGN:
		gDLL->getInterfaceIFace()->setInterfaceMode(INTERFACEMODE_SIGN);
		break;

	case CONTROL_GRID:
		gDLL->getEngineIFace()->SetGridMode(!(gDLL->getEngineIFace()->GetGridMode()));
		break;

	case CONTROL_BARE_MAP:
		gDLL->getInterfaceIFace()->toggleBareMapMode();
		break;

	case CONTROL_YIELDS:
		gDLL->getInterfaceIFace()->toggleYieldVisibleMode();
		break;

	case CONTROL_RESOURCE_ALL:
		gDLL->getEngineIFace()->toggleResourceLayer();
		break;

	case CONTROL_GLOBELAYER:
		gDLL->getEngineIFace()->toggleGlobeview();
		break;

	case CONTROL_SCORES:
		gDLL->getInterfaceIFace()->toggleScoresVisible();
		break;

	case CONTROL_LOAD_GAME:
		gDLL->LoadGame();
		break;

	case CONTROL_OPTIONS_SCREEN:
		gDLL->getPythonIFace()->callFunction("CvScreensInterface", "showOptionsScreen");
		break;

	case CONTROL_RETIRE:
		if (!isGameMultiPlayer() || countHumanPlayersAlive() == 1)
		{
			setGameState(GAMESTATE_OVER);
			gDLL->getInterfaceIFace()->setDirty(Soundtrack_DIRTY_BIT, true);
		}
		else
		{
			if (isNetworkMultiPlayer())
			{
				gDLL->sendMPRetire();
				gDLL->getInterfaceIFace()->exitingToMainMenu();
			}
			else
			{
				gDLL->handleRetirement(getActivePlayer());
			}
		}
		break;

	case CONTROL_SAVE_GROUP:
		gDLL->SaveGame(SAVEGAME_GROUP);
		break;

	case CONTROL_SAVE_NORMAL:
		gDLL->SaveGame(SAVEGAME_NORMAL);
		break;

	case CONTROL_QUICK_SAVE:
		if (!(isNetworkMultiPlayer()))	// SP only!
		{
			gDLL->QuickSave();
		}
		break;

	case CONTROL_QUICK_LOAD:
		if (!(isNetworkMultiPlayer()))	// SP only!
		{
			gDLL->QuickLoad();
		}
		break;

	case CONTROL_ORTHO_CAMERA:
		gDLL->getEngineIFace()->SetOrthoCamera(!(gDLL->getEngineIFace()->GetOrthoCamera()));
		break;

	case CONTROL_CYCLE_CAMERA_FLYING_MODES:
		gDLL->getEngineIFace()->CycleFlyingMode(1);
		break;

	case CONTROL_ISOMETRIC_CAMERA_LEFT:
		gDLL->getEngineIFace()->MoveBaseTurnLeft();
		break;

	case CONTROL_ISOMETRIC_CAMERA_RIGHT:
		gDLL->getEngineIFace()->MoveBaseTurnRight();
		break;

	case CONTROL_FLYING_CAMERA:
		gDLL->getEngineIFace()->SetFlying(!(gDLL->getEngineIFace()->GetFlying()));
		break;

	case CONTROL_MOUSE_FLYING_CAMERA:
		gDLL->getEngineIFace()->SetMouseFlying(!(gDLL->getEngineIFace()->GetMouseFlying()));
		break;

	case CONTROL_TOP_DOWN_CAMERA:
		gDLL->getEngineIFace()->SetSatelliteMode(!(gDLL->getEngineIFace()->GetSatelliteMode()));
		break;

	case CONTROL_CIVILOPEDIA:
		gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaShow");
		break;

	case CONTROL_RELIGION_SCREEN:
		gDLL->getPythonIFace()->callFunction(PYScreensModule, "showReligionScreen");
		break;

	case CONTROL_CIVICS_SCREEN:
		gDLL->getPythonIFace()->callFunction(PYScreensModule, "showCivicsScreen");
		break;

	case CONTROL_FOREIGN_SCREEN:
		{
			CyArgsList argsList;
			argsList.add(-1);
			gDLL->getPythonIFace()->callFunction(PYScreensModule, "showForeignAdvisorScreen", argsList.makeFunctionArgs());
		}
		break;

	case CONTROL_FINANCIAL_SCREEN:
		gDLL->getPythonIFace()->callFunction(PYScreensModule, "showFinanceAdvisor");
		break;

	case CONTROL_MILITARY_SCREEN:
		gDLL->getPythonIFace()->callFunction(PYScreensModule, "showMilitaryAdvisor");
		break;

	case CONTROL_TECH_CHOOSER:
		gDLL->getPythonIFace()->callFunction(PYScreensModule, "showTechChooser");
		break;

	case CONTROL_TURN_LOG:
		if (!gDLL->GetWorldBuilderMode())
		{
			gDLL->getInterfaceIFace()->toggleTurnLog();
		}
		break;

	case CONTROL_CHAT_ALL:
		if (!gDLL->GetWorldBuilderMode())
		{
			gDLL->getInterfaceIFace()->showTurnLog(CHATTARGET_ALL);
		}
		break;

	case CONTROL_CHAT_TEAM:
		if (!gDLL->GetWorldBuilderMode())
		{
			gDLL->getInterfaceIFace()->showTurnLog(CHATTARGET_TEAM);
		}
		break;

	case CONTROL_DOMESTIC_SCREEN:
		{
			CyArgsList argsList;
			argsList.add(-1);
			gDLL->getPythonIFace()->callFunction(PYScreensModule, "showDomesticAdvisor", argsList.makeFunctionArgs());
		}
		break;

	case CONTROL_VICTORY_SCREEN:
		gDLL->getPythonIFace()->callFunction(PYScreensModule, "showVictoryScreen");
		break;

	case CONTROL_INFO:
		{
			CyArgsList args;
			args.add(0);
			args.add(getGameState() == GAMESTATE_ON ? 0 : 1);
			gDLL->getPythonIFace()->callFunction(PYScreensModule, "showInfoScreen", args.makeFunctionArgs());
		}
		break;

	case CONTROL_GLOBE_VIEW:
		gDLL->getEngineIFace()->toggleGlobeview();
		break;

	case CONTROL_DETAILS:
		gDLL->getInterfaceIFace()->showDetails();
		break;

	case CONTROL_ADMIN_DETAILS:
		if (GC.getInitCore().getAdminPassword().empty())
		{
			gDLL->getInterfaceIFace()->showAdminDetails();
		}
		else
		{
			CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_ADMIN_PASSWORD);
			if (NULL != pInfo)
			{
				pInfo->setData1((int)CONTROL_ADMIN_DETAILS);
				gDLL->getInterfaceIFace()->addPopup(pInfo, NO_PLAYER, true);
			}
		}
		break;

	case CONTROL_HALL_OF_FAME:
		{
			CyArgsList args;
			args.add(true);
			gDLL->getPythonIFace()->callFunction(PYScreensModule, "showHallOfFame", args.makeFunctionArgs());
		}
		break;

	case CONTROL_WORLD_BUILDER:
		if (GC.getInitCore().getAdminPassword().empty())
		{
			gDLL->getInterfaceIFace()->setWorldBuilder(!(gDLL->GetWorldBuilderMode()));
		}
		else
		{
			CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_ADMIN_PASSWORD);
			if (NULL != pInfo)
			{
				pInfo->setData1((int)CONTROL_WORLD_BUILDER);
				gDLL->getInterfaceIFace()->addPopup(pInfo, NO_PLAYER, true);
			}
		}
		break;

	case CONTROL_DIPLOMACY:
		pInfo = new CvPopupInfo(BUTTONPOPUP_DIPLOMACY);
		if (NULL != pInfo)
		{
			GET_PLAYER(getActivePlayer()).addPopup(pInfo);
		}
		break;

	default:
		FAssertMsg(false, "eControl did not match any valid options");
		break;
	}
}


void CvGame::implementDeal(PlayerTypes eWho, PlayerTypes eOtherWho, CLinkList<TradeData>* pOurList, CLinkList<TradeData>* pTheirList, bool bForce)
{
	FAssertMsg(eWho != NO_PLAYER, "Who is not assigned a valid value");
	FAssertMsg(eOtherWho != NO_PLAYER, "OtherWho is not assigned a valid value");
	FAssertMsg(eWho != eOtherWho, "eWho is not expected to be equal with eOtherWho");

	// if debug mode (ctrl-z under chipotle), then notify the human that a deal was made
	if (GC.getGameINLINE().isDebugMode())
	{
		FAssertMsg(((pOurList->getLength() > 0) || (pTheirList->getLength() > 0)), "Empty trade implemented?!");

		PlayerTypes iActivePlayer = GC.getGameINLINE().getActivePlayer();
		
		if (iActivePlayer != eWho && iActivePlayer != eOtherWho)
		{
			bool bReportTrade = true;
			
			// special check for the triple open borders issue
			CLLNode<TradeData>* pNode = pOurList->head();
			if (pNode != NULL)
			{
				if (pNode->m_data.m_eItemType == TRADE_OPEN_BORDERS && pNode->m_data.m_iData != 0)
				{
					bReportTrade = false;
				}
			}
			
			if (bReportTrade)
			{
				CvWString szString;
				CvWString szDealString;

				szDealString.clear();
				GAMETEXT.getDealString(szDealString, eWho, eOtherWho, pOurList, pTheirList, iActivePlayer);
				szString.Format(L"%s", szDealString.GetCString());
				gDLL->getInterfaceIFace()->addMessage(iActivePlayer, true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szString);
			}
		}
	}

	CvDeal* pDeal = addDeal();
	pDeal->init(pDeal->getID(), eWho, eOtherWho);
	pDeal->addTrades(pOurList, pTheirList, !bForce);

	if ((pDeal->getLengthFirstTrades() == 0) && (pDeal->getLengthSecondTrades() == 0))
	{
		pDeal->kill();
	}
}


void CvGame::verifyDeals()
{
	CvDeal* pLoopDeal;
	int iLoop;

	for(pLoopDeal = firstDeal(&iLoop); pLoopDeal != NULL; pLoopDeal = nextDeal(&iLoop))
	{
		pLoopDeal->verify();
	}
}


/* Globeview configuration control:
If bStarsVisible, then there will be stars visible behind the globe when it is on
If bWorldIsRound, then the world will bend into a globe; otherwise, it will show up as a plane  */
void CvGame::getGlobeviewConfigurationParameters(TeamTypes eTeam, bool& bStarsVisible, bool& bWorldIsRound)
{
	if(GET_TEAM(eTeam).isMapCentering() || isCircumnavigated())
	{
		bStarsVisible = true;
		bWorldIsRound = true;
	}
	else
	{
		bStarsVisible = false;
		bWorldIsRound = false;
	}
}


int CvGame::getSymbolID(int iSymbol)
{
	return gDLL->getInterfaceIFace()->getSymbolID(iSymbol);
}


int CvGame::getAdjustedPopulationPercent(VictoryTypes eVictory)
{
	int iPopulation;
	int iBestPopulation;
	int iNextBestPopulation;
	int iI;

	if (GC.getVictoryInfo(eVictory).getPopulationPercentLead() == 0)
	{
		return 0;
	}

	if (getTotalPopulation() == 0)
	{
		return 100;
	}

	iBestPopulation = 0;
	iNextBestPopulation = 0;

	for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
	{
		if (GET_TEAM((TeamTypes)iI).isAlive())
		{
			iPopulation = GET_TEAM((TeamTypes)iI).getTotalPopulation();

			if (iPopulation > iBestPopulation)
			{
				iNextBestPopulation = iBestPopulation;
				iBestPopulation = iPopulation;
			}
			else if (iPopulation > iNextBestPopulation)
			{
				iNextBestPopulation = iPopulation;
			}
		}
	}

	return min(100, (((iNextBestPopulation * 100) / getTotalPopulation()) + GC.getVictoryInfo(eVictory).getPopulationPercentLead()));
}


int CvGame::getProductionPerPopulation(HurryTypes eHurry)
{
	if (NO_HURRY == eHurry)
	{
		return 0;
	}
	return (GC.getHurryInfo(eHurry).getProductionPerPopulation() * 100) / max(1, GC.getGameSpeedInfo(getGameSpeedType()).getHurryPercent());
}


int CvGame::getAdjustedLandPercent(VictoryTypes eVictory)
{
	int iPercent;

	if (GC.getVictoryInfo(eVictory).getLandPercent() == 0)
	{
		return 0;
	}

	iPercent = GC.getVictoryInfo(eVictory).getLandPercent();

	iPercent -= (countCivTeamsEverAlive() * 2);

	return max(iPercent, GC.getVictoryInfo(eVictory).getMinLandPercent());
}


bool CvGame::isTeamVote(VoteTypes eVote) const
{
	return (GC.getVoteInfo(eVote).isSecretaryGeneral() || GC.getVoteInfo(eVote).isVictory());
}


bool CvGame::isChooseElection(VoteTypes eVote) const
{
	return !(GC.getVoteInfo(eVote).isSecretaryGeneral());
}


bool CvGame::isTeamVoteEligible(TeamTypes eTeam) const
{
	int iExtraEligible;
	int iCount;
	int iI;

	if (GET_TEAM(eTeam).isForceTeamVoteEligible())
	{
		return true;
	}

	iCount = 0;

	for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
	{
		if (GET_TEAM((TeamTypes)iI).isAlive())
		{
			if (GET_TEAM((TeamTypes)iI).isForceTeamVoteEligible())
			{
				iCount++;
			}
		}
	}

	if (iCount >= GC.getDefineINT("TEAM_VOTE_MIN_CANDIDATES"))
	{
		return false;
	}

	iExtraEligible = (GC.getDefineINT("TEAM_VOTE_MIN_CANDIDATES") - iCount);

	FAssertMsg(iExtraEligible > 0, "iExtraEligible is expected to be greater than 0");

	for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
	{
		if (GET_TEAM((TeamTypes)iI).isAlive())
		{
			if (!(GET_TEAM((TeamTypes)iI).isForceTeamVoteEligible()))
			{
				if (iI != eTeam)
				{
					if ((GET_TEAM((TeamTypes)iI).getTotalPopulation() > GET_TEAM(eTeam).getTotalPopulation()) ||
						  ((GET_TEAM((TeamTypes)iI).getTotalPopulation() == GET_TEAM(eTeam).getTotalPopulation()) && (iI < eTeam)))
					{
						iExtraEligible--;
					}
				}
			}
		}
	}

	return (iExtraEligible > 0);
}


int CvGame::countVote(VoteTypes eVote, int iChoice) const
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (getPlayerVote(((PlayerTypes)iI), eVote) == iChoice)
			{
				iCount += GET_PLAYER((PlayerTypes)iI).getTotalPopulation();
			}
		}
	}

	return iCount;
}


int CvGame::countPossibleVote() const
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			iCount += GET_PLAYER((PlayerTypes)iI).getTotalPopulation();
		}
	}

	return iCount;
}


TeamTypes CvGame::findHighestVoteTeam(VoteTypes eVote) const
{
	TeamTypes eBestTeam;
	int iCount;
	int iBestCount;
	int iI;

	if (!(isTeamVote(eVote)))
	{
		return NO_TEAM;
	}

	iBestCount = 0;
	eBestTeam = NO_TEAM;

	for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
	{
		if (GET_TEAM((TeamTypes)iI).isAlive())
		{
			iCount = countVote(eVote, iI);

			if (iCount > iBestCount)
			{
				iBestCount = iCount;
				eBestTeam = ((TeamTypes)iI);
			}
		}
	}

	return eBestTeam;
}



int CvGame::getVoteRequired(VoteTypes eVote) const
{
	return ((countPossibleVote() * GC.getVoteInfo(eVote).getPopulationThreshold()) / 100);
}


TeamTypes CvGame::getSecretaryGeneral() const
{
	int iI;

	for (iI = 0; iI < GC.getNumVoteInfos(); iI++)
	{
		if (GC.getVoteInfo((VoteTypes)iI).isSecretaryGeneral())
		{
			if (isVotePassed((VoteTypes)iI))
			{
				return ((TeamTypes)(getVoteOutcome((VoteTypes)iI)));
			}
		}
	}

	return NO_TEAM;
}


int CvGame::countCivPlayersAlive()
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			iCount++;
		}
	}

	return iCount;
}


int CvGame::countCivPlayersEverAlive()
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isEverAlive())
		{
			iCount++;
		}
	}

	return iCount;
}


int CvGame::countCivTeamsAlive()
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
	{
		if (GET_TEAM((TeamTypes)iI).isAlive())
		{
			iCount++;
		}
	}

	return iCount;
}


int CvGame::countCivTeamsEverAlive()
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
	{
		if (GET_TEAM((TeamTypes)iI).isEverAlive())
		{
			iCount++;
		}
	}

	return iCount;
}


int CvGame::countHumanPlayersAlive()
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).isHuman())
			{
				iCount++;
			}
		}
	}

	return iCount;
}


int CvGame::countHumanPlayersEverAlive()
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isEverAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).isHuman())
			{
				iCount++;
			}
		}
	}

	return iCount;
}


int CvGame::countTotalCivPower()
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			iCount += GET_PLAYER((PlayerTypes)iI).getPower();
		}
	}

	return iCount;
}


int CvGame::countTotalNukeUnits()
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			iCount += GET_PLAYER((PlayerTypes)iI).getNumNukeUnits();
		}
	}

	return iCount;
}


int CvGame::countKnownTechNumTeams(TechTypes eTech)
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		if (GET_TEAM((TeamTypes)iI).isEverAlive())
		{
			if (GET_TEAM((TeamTypes)iI).isHasTech(eTech))
			{
				iCount++;
			}
		}
	}

	return iCount;
}


int CvGame::getNumFreeBonuses(BuildingTypes eBuilding)
{
	if (GC.getBuildingInfo(eBuilding).getNumFreeBonuses() == -1)
	{
		return GC.getWorldInfo(GC.getMapINLINE().getWorldSize()).getNumFreeBuildingBonuses();
	}
	else
	{
		return GC.getBuildingInfo(eBuilding).getNumFreeBonuses();
	}
}


int CvGame::countReligionLevels(ReligionTypes eReligion)
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			iCount += GET_PLAYER((PlayerTypes)iI).getHasReligionCount(eReligion);
		}
	}

	return iCount;
}


int CvGame::calculateReligionPercent(ReligionTypes eReligion)
{
	CvCity* pLoopCity;
	int iCount;
	int iLoop;
	int iI;

	if (getTotalPopulation() == 0)
	{
		return 0;
	}

	iCount = 0;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
			{
				if (pLoopCity->isHasReligion(eReligion))
				{
					iCount += ((pLoopCity->getPopulation() + (pLoopCity->getReligionCount() / 2)) / pLoopCity->getReligionCount());
				}
			}
		}
	}

	return ((iCount * 100) / getTotalPopulation());
}


int CvGame::goldenAgeLength() const
{
	int iLength;

	iLength = GC.getDefineINT("GOLDEN_AGE_LENGTH");

	iLength *= GC.getGameSpeedInfo(getGameSpeedType()).getGoldenAgePercent();
	iLength /= 100;

	return iLength;
}


int CvGame::getImprovementUpgradeTime(ImprovementTypes eImprovement) const
{
	int iTime;

	iTime = GC.getImprovementInfo(eImprovement).getUpgradeTime();

	iTime *= GC.getGameSpeedInfo(getGameSpeedType()).getImprovementPercent();
	iTime /= 100;

	iTime *= GC.getEraInfo(getStartEra()).getImprovementPercent();
	iTime /= 100;

	return iTime;
}


bool CvGame::canTrainNukes() const
{
	int iI, iJ;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			for (iJ = 0; iJ < GC.getNumUnitInfos(); iJ++)
			{
				if (GC.getUnitInfo((UnitTypes)iJ).getNukeRange() != -1)
				{
					if (GET_PLAYER((PlayerTypes)iI).canTrain(((UnitTypes)iJ)))
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}


bool CvGame::showAllMoves(TeamTypes eTeam) const
{
	int iI;

	if (isMPOption(MPOPTION_SIMULTANEOUS_TURNS))
	{
		return false;
	}

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).isHuman())
			{
				if (GET_PLAYER((PlayerTypes)iI).isOption((atWar(GET_PLAYER((PlayerTypes)iI).getTeam(), eTeam)) ? PLAYEROPTION_SHOW_ENEMY_MOVES : PLAYEROPTION_SHOW_FRIENDLY_MOVES))
				{
					return true;
				}
			}
		}
	}

	return false;
}


EraTypes CvGame::getCurrentEra() const
{
	int iEra;
	int iCount;
	int iI;

	iEra = 0;
	iCount = 0;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			iEra += GET_PLAYER((PlayerTypes)iI).getCurrentEra();
			iCount++;
		}
	}

	if (iCount > 0)
	{
		return ((EraTypes)(iEra / iCount));
	}

	return NO_ERA;
}


TeamTypes CvGame::getActiveTeam()
{
	if (getActivePlayer() == NO_PLAYER)
	{
		return NO_TEAM;	
	}
	else
	{
		return (TeamTypes)GET_PLAYER(getActivePlayer()).getTeam();
	}
}


CivilizationTypes CvGame::getActiveCivilizationType()
{
	if (getActivePlayer() == NO_PLAYER)
	{
		return NO_CIVILIZATION;
	}
	else
	{
		return (CivilizationTypes)GET_PLAYER(getActivePlayer()).getCivilizationType();
	}
}


unsigned int CvGame::getLastEndTurnMessageSentTime()												
{
	return (getInitialTime() + (gDLL->getMillisecsPerTurn() * (getEndTurnMessagesSent() - 2)));
}


bool CvGame::isNetworkMultiPlayer()																	 
{
	return GC.getInitCore().getMultiplayer();
}


bool CvGame::isGameMultiPlayer()																	 
{
	return (isNetworkMultiPlayer() || isPbem() || isHotSeat());
}


bool CvGame::isTeamGame()																			 
{
	FAssert(countCivPlayersAlive() >= countCivTeamsAlive());
	return (countCivPlayersAlive() > countCivTeamsAlive());
}


bool CvGame::isModem()
{
	return gDLL->IsModem();
}
void CvGame::setModem(bool bModem)
{
	if (bModem)
	{
		gDLL->ChangeINIKeyValue("CONFIG", "Bandwidth", "modem");
	}
	else
	{
		gDLL->ChangeINIKeyValue("CONFIG", "Bandwidth", "broadband");
	}

	gDLL->SetModem(bModem);
}


void CvGame::reviveActivePlayer()
{
	if (!(GET_PLAYER(getActivePlayer()).isAlive()))
	{
		setAIAutoPlay(0);

		GC.getInitCore().setSlotStatus(getActivePlayer(), SS_TAKEN);
		
		// Let Python handle it
		long lResult=0;
		CyArgsList argsList;
		argsList.add(getActivePlayer());

		gDLL->getPythonIFace()->callFunction(PYGameModule, "doReviveActivePlayer", argsList.makeFunctionArgs(), &lResult);
		if (lResult == 1)
		{
			return;
		}

		GET_PLAYER(getActivePlayer()).initUnit(((UnitTypes)0), 0, 0);
	}
}


int CvGame::getNumHumanPlayers()
{
	return GC.getInitCore().getNumHumans();
}


int CvGame::getEndTurnMessagesSent()
{
	return m_iEndTurnMessagesSent;
}


void CvGame::incrementEndTurnMessagesSent()
{
	m_iEndTurnMessagesSent++;
}


int CvGame::getGameTurn()
{
	return GC.getInitCore().getGameTurn();
}


void CvGame::setGameTurn(int iNewValue)
{
	if (getGameTurn() != iNewValue)
	{
		GC.getInitCore().setGameTurn(iNewValue);
		FAssert(getGameTurn() >= 0);

		updateBuildingCommerce();

		setScoreDirty(true);

		gDLL->getInterfaceIFace()->setDirty(TurnTimer_DIRTY_BIT, true);
		gDLL->getInterfaceIFace()->setDirty(GameData_DIRTY_BIT, true);
	}
}


void CvGame::incrementGameTurn()
{
	setGameTurn(getGameTurn() + 1);
}


int CvGame::getTurnYear(int iGameTurn)
{
	// moved the body of this method to Game Core Utils so we have access for other games than the current one (replay screen in HOF)
	return getTurnYearForGame(iGameTurn, getStartYear(), getCalendar(), getGameSpeedType());
}


int CvGame::getGameTurnYear()
{
	return getTurnYear(getGameTurn());
}


int CvGame::getElapsedGameTurns()
{
	return m_iElapsedGameTurns;
}


void CvGame::incrementElapsedGameTurns()
{
	m_iElapsedGameTurns++;
}


int CvGame::getMaxTurns() const
{
	return GC.getInitCore().getMaxTurns();
}


void CvGame::setMaxTurns(int iNewValue)
{
	GC.getInitCore().setMaxTurns(iNewValue);
	FAssert(getMaxTurns() >= 0);
}


void CvGame::changeMaxTurns(int iChange)
{
	setMaxTurns(getMaxTurns() + iChange);
}


int CvGame::getMaxCityElimination() const
{
	return GC.getInitCore().getMaxCityElimination();
}


void CvGame::setMaxCityElimination(int iNewValue)
{
	GC.getInitCore().setMaxCityElimination(iNewValue);
	FAssert(getMaxCityElimination() >= 0);
}


int CvGame::getStartTurn() const
{
	return m_iStartTurn;
}


void CvGame::setStartTurn(int iNewValue)
{
	m_iStartTurn = iNewValue;
}


int CvGame::getStartYear() const
{
	return m_iStartYear;
}


void CvGame::setStartYear(int iNewValue)
{
	m_iStartYear = iNewValue;
}


int CvGame::getEstimateEndTurn() const
{
	return m_iEstimateEndTurn;
}


void CvGame::setEstimateEndTurn(int iNewValue)
{
	m_iEstimateEndTurn = iNewValue;
}


int CvGame::getTurnSlice() const
{
	return m_iTurnSlice;
}


int CvGame::getMinutesPlayed() const
{
	return (getTurnSlice() / gDLL->getTurnsPerMinute());
}


void CvGame::setTurnSlice(int iNewValue)
{
	m_iTurnSlice = iNewValue;
}


void CvGame::changeTurnSlice(int iChange)
{
	setTurnSlice(getTurnSlice() + iChange);
}


int CvGame::getCutoffSlice() const
{
	return m_iCutoffSlice;
}


void CvGame::setCutoffSlice(int iNewValue)
{
	m_iCutoffSlice = iNewValue;
}


void CvGame::changeCutoffSlice(int iChange)
{
	setCutoffSlice(getCutoffSlice() + iChange);
}


int CvGame::getTurnSlicesRemaining()
{
	return (getCutoffSlice() - getTurnSlice());
}


void CvGame::resetTurnTimer()
{
	// We should only use the turn timer if we are in multiplayer
	if (isMPOption(MPOPTION_TURN_TIMER))
	{
		// Determine how much time we should allow
		int iTurnLen = getMaxTurnLen();
		if (getElapsedGameTurns() == 0 && !isPitboss())
		{
			// Let's allow more time for the initial turn
			TurnTimerTypes eTurnTimer = GC.getInitCore().getTurnTimer();
			FAssertMsg(eTurnTimer >= 0 && eTurnTimer < GC.getNumTurnTimerInfos(), "Invalid TurnTimer selection in InitCore");
			iTurnLen = (iTurnLen * GC.getTurnTimerInfo(eTurnTimer).getFirstTurnMultiplier());
		}
		// Set the current turn slice to start the 'timer'
		setCutoffSlice(getTurnSlice() + iTurnLen);
	}
}

void CvGame::incrementTurnTimer(int iNumTurnSlices)
{
	if (isMPOption(MPOPTION_TURN_TIMER))
	{
		// If the turn timer has expired, we shouldn't increment it as we've sent our turn complete message
		if (getTurnSlice() <= getCutoffSlice())
		{
			changeCutoffSlice(iNumTurnSlices);
		}
	}
}


int CvGame::getMaxTurnLen()
{
	if (isPitboss())
	{
		// Use the user provided input
		// Turn time is in hours
		return ( getPitbossTurnTime() * 3600 * 4);
	}
	else
	{
		int iMaxUnits = 0;
		int iMaxCities = 0;

		// Find out who has the most units and who has the most cities
		// Calculate the max turn time based on the max number of units and cities
		for (int i = 0; i < MAX_CIV_PLAYERS; ++i)
		{
			if (GET_PLAYER((PlayerTypes)i).isAlive())
			{
				if (GET_PLAYER((PlayerTypes)i).getNumUnits() > iMaxUnits)
				{
					iMaxUnits = GET_PLAYER((PlayerTypes)i).getNumUnits();
				}
				if (GET_PLAYER((PlayerTypes)i).getNumCities() > iMaxCities)
				{
					iMaxCities = GET_PLAYER((PlayerTypes)i).getNumCities();
				}
			}
		}

		// Now return turn len based on base len and unit and city bonuses
		TurnTimerTypes eTurnTimer = GC.getInitCore().getTurnTimer();
		FAssertMsg(eTurnTimer >= 0 && eTurnTimer < GC.getNumTurnTimerInfos(), "Invalid TurnTimer Selection in InitCore");
		return ( GC.getTurnTimerInfo(eTurnTimer).getBaseTime() + 
			    (GC.getTurnTimerInfo(eTurnTimer).getCityBonus()*iMaxCities) +
				(GC.getTurnTimerInfo(eTurnTimer).getUnitBonus()*iMaxUnits) );
	}
}


int CvGame::getTargetScore() const
{
	return GC.getInitCore().getTargetScore();
}


void CvGame::setTargetScore(int iNewValue)
{
	GC.getInitCore().setTargetScore(iNewValue);
	FAssert(getTargetScore() >= 0);
}


int CvGame::getNumGameTurnActive()
{
	return m_iNumGameTurnActive;
}


int CvGame::countNumHumanGameTurnActive()
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isHuman())
		{
			if (GET_PLAYER((PlayerTypes)iI).isTurnActive())
			{
				iCount++;
			}
		}
	}

	return iCount;
}


void CvGame::changeNumGameTurnActive(int iChange)
{
	m_iNumGameTurnActive = (m_iNumGameTurnActive + iChange);
	FAssert(getNumGameTurnActive() >= 0);
}


int CvGame::getNumCities()																			
{
	return m_iNumCities;
}


int CvGame::getNumCivCities()
{
	return (getNumCities() - GET_PLAYER(BARBARIAN_PLAYER).getNumCities());
}


void CvGame::changeNumCities(int iChange)
{
	m_iNumCities = (m_iNumCities + iChange);
	FAssert(getNumCities() >= 0);
}


int CvGame::getTotalPopulation() const
{
	return m_iTotalPopulation;
}


void CvGame::changeTotalPopulation(int iChange)
{
	m_iTotalPopulation = (m_iTotalPopulation + iChange);
	FAssert(getTotalPopulation() >= 0);
}


int CvGame::getTradeRoutes() const
{
	return m_iTradeRoutes;
}


void CvGame::changeTradeRoutes(int iChange)
{
	if (iChange != 0)
	{
		m_iTradeRoutes = (m_iTradeRoutes + iChange);
		FAssert(getTradeRoutes() >= 0);

		updateTradeRoutes();
	}
}


int CvGame::getFreeTradeCount() const
{
	return m_iFreeTradeCount;
}


bool CvGame::isFreeTrade() const
{
	return (getFreeTradeCount() > 0);
}


void CvGame::changeFreeTradeCount(int iChange)
{
	bool bOldFreeTrade;

	if (iChange != 0)
	{
		bOldFreeTrade = isFreeTrade();

		m_iFreeTradeCount = (m_iFreeTradeCount + iChange);
		FAssert(getFreeTradeCount() >= 0);

		if (bOldFreeTrade != isFreeTrade())
		{
			updateTradeRoutes();
		}
	}
}


int CvGame::getNoNukesCount() const
{
	return m_iNoNukesCount;
}


bool CvGame::isNoNukes() const
{
	return (getNoNukesCount() > 0);
}


void CvGame::changeNoNukesCount(int iChange)
{
	m_iNoNukesCount = (m_iNoNukesCount + iChange);
	FAssert(getNoNukesCount() >= 0);
}


int CvGame::getSecretaryGeneralTimer() const
{
	return m_iSecretaryGeneralTimer;
}


void CvGame::setSecretaryGeneralTimer(int iNewValue)
{
	m_iSecretaryGeneralTimer = iNewValue;
	FAssert(getSecretaryGeneralTimer() >= 0);
}


void CvGame::changeSecretaryGeneralTimer(int iChange)
{
	setSecretaryGeneralTimer(getSecretaryGeneralTimer() + iChange);
}


int CvGame::getNukesExploded() const
{
	return m_iNukesExploded;
}


void CvGame::changeNukesExploded(int iChange)
{
	m_iNukesExploded = (m_iNukesExploded + iChange);
}


int CvGame::getMaxPopulation() const
{
	return m_iMaxPopulation;
}


int CvGame::getMaxLand() const
{
	return m_iMaxLand;
}


int CvGame::getMaxTech() const
{
	return m_iMaxTech;
}


int CvGame::getMaxWonders() const
{
	return m_iMaxWonders;
}


int CvGame::getInitPopulation() const
{
	return m_iInitPopulation;
}


int CvGame::getInitLand() const
{
	return m_iInitLand;
}


int CvGame::getInitTech() const
{
	return m_iInitTech;
}


int CvGame::getInitWonders() const
{
	return m_iInitWonders;
}


void CvGame::initScoreCalculation()
{
	// initialize score calculation
	int iMaxFood = 0;
	for (int i = 0; i < GC.getMapINLINE().numPlotsINLINE(); i++)
	{
		CvPlot* pPlot = GC.getMapINLINE().plotByIndexINLINE(i);
		if (!pPlot->isWater() || pPlot->isAdjacentToLand())
		{
			iMaxFood += pPlot->calculateBestNatureYield(YIELD_FOOD, NO_TEAM);
		}
	}
	m_iMaxPopulation = getPopulationScore(iMaxFood / max(1, GC.getFOOD_CONSUMPTION_PER_POPULATION()));
	m_iMaxLand = getLandPlotsScore(GC.getMapINLINE().getLandPlots());
	m_iMaxTech = 0;
	for (int i = 0; i < GC.getNumTechInfos(); i++)
	{
		m_iMaxTech += getTechScore((TechTypes)i);
	}
	m_iMaxWonders = 0;
	for (int i = 0; i < GC.getNumBuildingClassInfos(); i++)
	{
		m_iMaxWonders += getWonderScore((BuildingClassTypes)i);
	}

	if (NO_ERA != getStartEra())
	{
		int iNumSettlers = GC.getEraInfo(getStartEra()).getStartingUnitMultiplier();
		m_iInitPopulation = getPopulationScore(iNumSettlers * (GC.getEraInfo(getStartEra()).getFreePopulation() + 1));
		m_iInitLand = getLandPlotsScore(iNumSettlers *  NUM_CITY_PLOTS);
	}
	else
	{
		m_iInitPopulation = 0;
		m_iInitLand = 0;
	}

	m_iInitTech = 0;
	for (int i = 0; i < GC.getNumTechInfos(); i++)
	{
		if (GC.getTechInfo((TechTypes)i).getEra() < getStartEra())
		{
			m_iInitTech += getTechScore((TechTypes)i);
		}
		else
		{
			// count all possible free techs as initial to lower the score from immediate retirement
			for (int iCiv = 0; iCiv < GC.getNumCivilizationInfos(); iCiv++)
			{
				if (GC.getCivilizationInfo((CivilizationTypes)iCiv).isPlayable())
				{
					if (GC.getCivilizationInfo((CivilizationTypes)iCiv).isCivilizationFreeTechs(i))
					{
						m_iInitTech += getTechScore((TechTypes)i);
						break;
					}
				}
			}
		}
	}
	m_iInitWonders = 0;
}


int CvGame::getAIAutoPlay()
{
	return m_iAIAutoPlay;
}


void CvGame::setAIAutoPlay(int iNewValue)
{
	int iOldValue;

	iOldValue = getAIAutoPlay();

	if (iOldValue != iNewValue)
	{
		m_iAIAutoPlay = max(0, iNewValue);

		if ((iOldValue == 0) && (getAIAutoPlay() > 0))
		{
			GET_PLAYER(getActivePlayer()).killUnits();
			GET_PLAYER(getActivePlayer()).killCities();
		}
	}
}


void CvGame::changeAIAutoPlay(int iChange)
{
	setAIAutoPlay(getAIAutoPlay() + iChange);
}


unsigned int CvGame::getInitialTime()
{
	return m_uiInitialTime;
}


void CvGame::setInitialTime(unsigned int uiNewValue)
{
	m_uiInitialTime = uiNewValue;
}


bool CvGame::isScoreDirty() const
{
	return m_bScoreDirty;
}


void CvGame::setScoreDirty(bool bNewValue)
{
	m_bScoreDirty = bNewValue;
}


bool CvGame::isCircumnavigated() const
{
	return m_bCircumnavigated;
}


void CvGame::makeCircumnavigated()
{
	m_bCircumnavigated = true;
}

bool CvGame::circumnavigationAvailable() const
{
	if (isCircumnavigated())
	{
		return false;
	}

	if (GC.getDefineINT("CIRCUMNAVIGATE_FREE_MOVES") == 0)
	{
		return false;
	}

	CvMap& kMap = GC.getMapINLINE();

	if (!(kMap.isWrapXINLINE()) && !(kMap.isWrapYINLINE()))
	{
		return false;
	}

	if (kMap.getLandPlots() > ((kMap.numPlotsINLINE() * 2) / 3))
	{
		return false;
	}

	return true;
}

bool CvGame::isDiploVote() const
{
	return m_bDiploVote;
}


void CvGame::makeDiploVote()
{
	m_bDiploVote = true;
}


bool CvGame::isDebugMode() const
{
	if ((gDLL->getChtLvl() > 0) || (gDLL->GetWorldBuilderMode()))
	{
		return m_bDebugMode;
	}
	else
	{
		return false;
	}
}


void CvGame::toggleDebugMode()
{
	m_bDebugMode = ((m_bDebugMode) ? false : true);

	GC.getMapINLINE().updateVisibility();
	GC.getMapINLINE().updateSymbols();
	GC.getMapINLINE().updateMinimapColor();

	gDLL->getInterfaceIFace()->setDirty(GameData_DIRTY_BIT, true);
	gDLL->getInterfaceIFace()->setDirty(Score_DIRTY_BIT, true);
	gDLL->getInterfaceIFace()->setDirty(MinimapSection_DIRTY_BIT, true);
	gDLL->getInterfaceIFace()->setDirty(UnitInfo_DIRTY_BIT, true);
	gDLL->getInterfaceIFace()->setDirty(CityInfo_DIRTY_BIT, true);
	gDLL->getInterfaceIFace()->setDirty(GlobeLayer_DIRTY_BIT, true);

	//gDLL->getEngineIFace()->SetDirty(GlobeTexture_DIRTY_BIT, true);
	gDLL->getEngineIFace()->SetDirty(MinimapTexture_DIRTY_BIT, true);
	gDLL->getEngineIFace()->SetDirty(FogOfWar_DIRTY_BIT, true);
	gDLL->getEngineIFace()->SetDirty(CultureBorders_DIRTY_BIT, true);

	if (m_bDebugMode)
	{
		gDLL->getEngineIFace()->PushFogOfWar(FOGOFWARMODE_OFF);
	}
	else
	{
		gDLL->getEngineIFace()->PopFogOfWar();
	}
	gDLL->getEngineIFace()->setFogOfWarFromStack();
}


int CvGame::getPitbossTurnTime()
{
	return GC.getInitCore().getPitbossTurnTime();
}


bool CvGame::isHotSeat()
{
	return (GC.getInitCore().getHotseat());
}

bool CvGame::isPbem()
{
	return (GC.getInitCore().getPbem());
}



bool CvGame::isPitboss()
{
	return (GC.getInitCore().getPitboss());
}


bool CvGame::isFinalInitialized()
{
	return m_bFinalInitialized;
}


void CvGame::setFinalInitialized(bool bNewValue)
{
	PROFILE_FUNC();

	int iI;

	if (isFinalInitialized() != bNewValue)
	{
		m_bFinalInitialized = bNewValue;

		if (isFinalInitialized())
		{
			//GC.getMapINLINE().updateSight(false);
			//GC.getMapINLINE().updateSight(true);

			updatePlotGroups();

			GC.getMapINLINE().updateIrrigated();

			for (iI = 0; iI < MAX_TEAMS; iI++)
			{
				if (GET_TEAM((TeamTypes)iI).isAlive())
				{
					GET_TEAM((TeamTypes)iI).AI_updateAreaStragies();
				}
			}
		}
	}
}


bool CvGame::getPbemTurnSent() const
{
	return m_bPbemTurnSent;
}


void CvGame::setPbemTurnSent(bool bNewValue)
{
	m_bPbemTurnSent = bNewValue;
}


bool CvGame::getHotPbemBetweenTurns() const
{
	return m_bHotPbemBetweenTurns;
}


void CvGame::setHotPbemBetweenTurns(bool bNewValue)
{
	m_bHotPbemBetweenTurns = bNewValue;
}


bool CvGame::isPlayerOptionsSent() const
{
	return m_bPlayerOptionsSent;
}


void CvGame::sendPlayerOptions(bool bForce)
{
	int iI;

	if (getActivePlayer() == NO_PLAYER)
	{
		return;
	}

	if (!isPlayerOptionsSent() || bForce)
	{
		m_bPlayerOptionsSent = true;

		for (iI = 0; iI < NUM_PLAYEROPTION_TYPES; iI++)
		{
			gDLL->sendPlayerOption(((PlayerOptionTypes)iI), gDLL->getPlayerOption((PlayerOptionTypes)iI));
		}
	}
}


PlayerTypes CvGame::getActivePlayer() const
{
	return GC.getInitCore().getActivePlayer();
}


void CvGame::setActivePlayer(PlayerTypes eNewValue, bool bForceHotSeat)
{
	PlayerTypes eOldActivePlayer = getActivePlayer();
	if (eOldActivePlayer != eNewValue)
	{
		int iActiveNetId = ((NO_PLAYER != eOldActivePlayer) ? GET_PLAYER(eOldActivePlayer).getNetID() : -1);
		GC.getInitCore().setActivePlayer(eNewValue);

		if (GET_PLAYER(eNewValue).isHuman() && (isHotSeat() || isPbem() || bForceHotSeat))
		{
			gDLL->getPassword(eNewValue);
			setHotPbemBetweenTurns(false);
			gDLL->getInterfaceIFace()->dirtyTurnLog(eNewValue);

			if (NO_PLAYER != eOldActivePlayer)
			{
				int iInactiveNetId = GET_PLAYER(eNewValue).getNetID();
				GET_PLAYER(eNewValue).setNetID(iActiveNetId);
				GET_PLAYER(eOldActivePlayer).setNetID(iInactiveNetId);
			}

			GET_PLAYER(eNewValue).showMissedMessages();

			if (countHumanPlayersAlive() == 1 && isPbem())
			{
				// Nobody else left alive
				GC.getInitCore().setType(GAME_HOTSEAT_NEW);
			}

			if (isHotSeat())
			{
				sendPlayerOptions(true);
			}
		}

		if (GC.IsGraphicsInitialized())
		{
			GC.getMapINLINE().updateFog();
			GC.getMapINLINE().updateVisibility();
			GC.getMapINLINE().updateSymbols();
			GC.getMapINLINE().updateMinimapColor();

			gDLL->getInterfaceIFace()->setEndTurnMessage(false);

			gDLL->getInterfaceIFace()->clearSelectedCities();
			gDLL->getInterfaceIFace()->clearSelectionList();

			gDLL->getInterfaceIFace()->setDirty(PercentButtons_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(ResearchButtons_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(GameData_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(MinimapSection_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(CityInfo_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(UnitInfo_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(Flag_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(GlobeLayer_DIRTY_BIT, true);

			gDLL->getEngineIFace()->SetDirty(CultureBorders_DIRTY_BIT, true);
		}
	}
}


HandicapTypes CvGame::getHandicapType() const
{
	return m_eHandicap;
}

void CvGame::setHandicapType(HandicapTypes eHandicap)
{
	m_eHandicap = eHandicap;
}

PlayerTypes CvGame::getPausePlayer()
{
	return m_ePausePlayer;
}


bool CvGame::isPaused()
{
	return (getPausePlayer() != NO_PLAYER);
}


void CvGame::setPausePlayer(PlayerTypes eNewValue)
{
	m_ePausePlayer = eNewValue;
}


UnitTypes CvGame::getBestLandUnit()
{
	return m_eBestLandUnit;
}


int CvGame::getBestLandUnitCombat()
{
	if (getBestLandUnit() == NO_UNIT)
	{
		return 1;
	}

	return max(1, GC.getUnitInfo(getBestLandUnit()).getCombat());
}


void CvGame::setBestLandUnit(UnitTypes eNewValue)
{
	if (getBestLandUnit() != eNewValue)
	{
		m_eBestLandUnit = eNewValue;

		gDLL->getInterfaceIFace()->setDirty(UnitInfo_DIRTY_BIT, true);
	}
}


TeamTypes CvGame::getWinner() const
{
	return m_eWinner;
}


VictoryTypes CvGame::getVictory() const
{
	return m_eVictory;
}


void CvGame::setWinner(TeamTypes eNewWinner, VictoryTypes eNewVictory)
{
	CvWString szBuffer;

	if ((getWinner() != eNewWinner) || (getVictory() != eNewVictory))
	{
		m_eWinner = eNewWinner;
		m_eVictory = eNewVictory;

		if (getVictory() != NO_VICTORY)
		{
			if (getWinner() != NO_TEAM)
			{
				szBuffer = gDLL->getText("TXT_KEY_GAME_WON", GET_TEAM(getWinner()).getName().GetCString(), GC.getVictoryInfo(getVictory()).getTextKeyWide());
				addReplayMessage(REPLAY_MESSAGE_MAJOR_EVENT, GET_TEAM(getWinner()).getLeaderID(), szBuffer, -1, -1, (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"));
			}

			if ((getAIAutoPlay() > 0) || gDLL->GetAutorun())
			{
				setGameState(GAMESTATE_EXTENDED);
			}
			else
			{
				setGameState(GAMESTATE_OVER);
			}
		}

		gDLL->getInterfaceIFace()->setDirty(Center_DIRTY_BIT, true);

		gDLL->getEventReporterIFace()->victory(eNewWinner, eNewVictory);

		gDLL->getInterfaceIFace()->setDirty(Soundtrack_DIRTY_BIT, true);
	}
}


GameStateTypes CvGame::getGameState()
{
	return m_eGameState;
}


void CvGame::setGameState(GameStateTypes eNewValue)
{
	CvPopupInfo* pInfo;
	int iI;

	if (getGameState() != eNewValue)
	{
		m_eGameState = eNewValue;

		if (eNewValue == GAMESTATE_OVER)
		{
			gDLL->getEventReporterIFace()->gameEnd();

			showEndGameSequence();

			for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
			{
				if (GET_PLAYER((PlayerTypes)iI).isHuman())
				{
					// One more turn?
					pInfo = new CvPopupInfo(BUTTONPOPUP_EXTENDED_GAME);
					if (NULL != pInfo)
					{
						GET_PLAYER((PlayerTypes)iI).addPopup(pInfo);
					}
				}
			}
		}

		gDLL->getInterfaceIFace()->setDirty(Cursor_DIRTY_BIT, true);
	}
}


GameSpeedTypes CvGame::getGameSpeedType() const
{
	return GC.getInitCore().getGameSpeed();
}


EraTypes CvGame::getStartEra() const
{
	return GC.getInitCore().getEra();
}


CalendarTypes CvGame::getCalendar() const
{
	return GC.getInitCore().getCalendar();
}


int CvGame::getEndTurnMessagesReceived(int iIndex)														
{
	FAssertMsg(iIndex >= 0, "iIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(iIndex < MAX_PLAYERS, "iIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiEndTurnMessagesReceived[iIndex];
}


void CvGame::incrementEndTurnMessagesReceived(int iIndex)												
{
	FAssertMsg(iIndex >= 0, "iIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(iIndex < MAX_PLAYERS, "iIndex is expected to be within maximum bounds (invalid Index)");
	m_aiEndTurnMessagesReceived[iIndex]++;
}


PlayerTypes CvGame::getRankPlayer(int iRank)																	
{
	FAssertMsg(iRank >= 0, "iRank is expected to be non-negative (invalid Rank)");
	FAssertMsg(iRank < MAX_PLAYERS, "iRank is expected to be within maximum bounds (invalid Rank)");
	return (PlayerTypes)m_aiRankPlayer[iRank];
}


void CvGame::setRankPlayer(int iRank, PlayerTypes ePlayer)													
{
	FAssertMsg(iRank >= 0, "iRank is expected to be non-negative (invalid Rank)");
	FAssertMsg(iRank < MAX_PLAYERS, "iRank is expected to be within maximum bounds (invalid Rank)");

	if (getRankPlayer(iRank) != ePlayer)
	{
		m_aiRankPlayer[iRank] = ePlayer;

		gDLL->getInterfaceIFace()->setDirty(Score_DIRTY_BIT, true);
	}
}


int CvGame::getPlayerRank(PlayerTypes ePlayer)																	 
{
	FAssertMsg(ePlayer >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(ePlayer < MAX_PLAYERS, "ePlayer is expected to be within maximum bounds (invalid Index)");
	return m_aiPlayerRank[ePlayer];
}
 

void CvGame::setPlayerRank(PlayerTypes ePlayer, int iRank)													
{
	FAssertMsg(ePlayer >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(ePlayer < MAX_PLAYERS, "ePlayer is expected to be within maximum bounds (invalid Index)");
	m_aiPlayerRank[ePlayer] = iRank;
	FAssert(getPlayerRank(ePlayer) >= 0);
}


int CvGame::getPlayerScore(PlayerTypes ePlayer)																	
{
	FAssertMsg(ePlayer >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(ePlayer < MAX_PLAYERS, "ePlayer is expected to be within maximum bounds (invalid Index)");
	return m_aiPlayerScore[ePlayer];
}


void CvGame::setPlayerScore(PlayerTypes ePlayer, int iScore)													
{
	FAssertMsg(ePlayer >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(ePlayer < MAX_PLAYERS, "ePlayer is expected to be within maximum bounds (invalid Index)");

	if (getPlayerScore(ePlayer) != iScore)
	{
		m_aiPlayerScore[ePlayer] = iScore;
		FAssert(getPlayerScore(ePlayer) >= 0);

		gDLL->getInterfaceIFace()->setDirty(Score_DIRTY_BIT, true);
	}
}


TeamTypes CvGame::getRankTeam(int iRank)																		
{
	FAssertMsg(iRank >= 0, "iRank is expected to be non-negative (invalid Rank)");
	FAssertMsg(iRank < MAX_TEAMS, "iRank is expected to be within maximum bounds (invalid Index)");
	return (TeamTypes)m_aiRankTeam[iRank];
}


void CvGame::setRankTeam(int iRank, TeamTypes eTeam)														
{
	FAssertMsg(iRank >= 0, "iRank is expected to be non-negative (invalid Rank)");
	FAssertMsg(iRank < MAX_TEAMS, "iRank is expected to be within maximum bounds (invalid Index)");

	if (getRankTeam(iRank) != eTeam)
	{
		m_aiRankTeam[iRank] = eTeam;

		gDLL->getInterfaceIFace()->setDirty(Score_DIRTY_BIT, true);
	}
}


int CvGame::getTeamRank(TeamTypes eTeam)																		
{
	FAssertMsg(eTeam >= 0, "eTeam is expected to be non-negative (invalid Index)");
	FAssertMsg(eTeam < MAX_TEAMS, "eTeam is expected to be within maximum bounds (invalid Index)");
	return m_aiTeamRank[eTeam];
}


void CvGame::setTeamRank(TeamTypes eTeam, int iRank)														
{
	FAssertMsg(eTeam >= 0, "eTeam is expected to be non-negative (invalid Index)");
	FAssertMsg(eTeam < MAX_TEAMS, "eTeam is expected to be within maximum bounds (invalid Index)");
	m_aiTeamRank[eTeam] = iRank;
	FAssert(getTeamRank(eTeam) >= 0);
}


int CvGame::getTeamScore(TeamTypes eTeam)																	
{
	FAssertMsg(eTeam >= 0, "eTeam is expected to be non-negative (invalid Index)");
	FAssertMsg(eTeam < MAX_TEAMS, "eTeam is expected to be within maximum bounds (invalid Index)");
	return m_aiTeamScore[eTeam];
}


void CvGame::setTeamScore(TeamTypes eTeam, int iScore)		
{
	FAssertMsg(eTeam >= 0, "eTeam is expected to be non-negative (invalid Index)");
	FAssertMsg(eTeam < MAX_TEAMS, "eTeam is expected to be within maximum bounds (invalid Index)");
	m_aiTeamScore[eTeam] = iScore;
	FAssert(getTeamScore(eTeam) >= 0);
}


bool CvGame::isOption(GameOptionTypes eIndex) const
{
	return GC.getInitCore().getOption(eIndex);
}


void CvGame::setOption(GameOptionTypes eIndex, bool bEnabled)
{
	GC.getInitCore().setOption(eIndex, bEnabled);
}


bool CvGame::isMPOption(MultiplayerOptionTypes eIndex) const
{
	return GC.getInitCore().getMPOption(eIndex);
}


void CvGame::setMPOption(MultiplayerOptionTypes eIndex, bool bEnabled)
{
	GC.getInitCore().setMPOption(eIndex, bEnabled);
}


bool CvGame::isForcedControl(ForceControlTypes eIndex) const
{
	return GC.getInitCore().getForceControl(eIndex);
}


void CvGame::setForceControl(ForceControlTypes eIndex, bool bEnabled)
{
	GC.getInitCore().setForceControl(eIndex, bEnabled);
}


int CvGame::getUnitCreatedCount(UnitTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumUnitInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiUnitCreatedCount[eIndex];
}


void CvGame::incrementUnitCreatedCount(UnitTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumUnitInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_paiUnitCreatedCount[eIndex]++;
}


int CvGame::getUnitClassCreatedCount(UnitClassTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumUnitClassInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiUnitClassCreatedCount[eIndex];
}


bool CvGame::isUnitClassMaxedOut(UnitClassTypes eIndex, int iExtra)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumUnitClassInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (!isWorldUnitClass(eIndex))
	{
		return false;
	}

	FAssertMsg(getUnitClassCreatedCount(eIndex) <= GC.getUnitClassInfo(eIndex).getMaxGlobalInstances(), "Index is expected to be within maximum bounds (invalid Index)");

	return ((getUnitClassCreatedCount(eIndex) + iExtra) >= GC.getUnitClassInfo(eIndex).getMaxGlobalInstances());
}


void CvGame::incrementUnitClassCreatedCount(UnitClassTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumUnitClassInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_paiUnitClassCreatedCount[eIndex]++;
}


int CvGame::getBuildingClassCreatedCount(BuildingClassTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumBuildingClassInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiBuildingClassCreatedCount[eIndex];
}


bool CvGame::isBuildingClassMaxedOut(BuildingClassTypes eIndex, int iExtra)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumBuildingClassInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (!isWorldWonderClass(eIndex))
	{
		return false;
	}

	FAssertMsg(getBuildingClassCreatedCount(eIndex) <= GC.getBuildingClassInfo(eIndex).getMaxGlobalInstances(), "Index is expected to be within maximum bounds (invalid Index)");

	return ((getBuildingClassCreatedCount(eIndex) + iExtra) >= GC.getBuildingClassInfo(eIndex).getMaxGlobalInstances());
}


void CvGame::incrementBuildingClassCreatedCount(BuildingClassTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumBuildingClassInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_paiBuildingClassCreatedCount[eIndex]++;
}


int CvGame::getProjectCreatedCount(ProjectTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumProjectInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiProjectCreatedCount[eIndex];
}


bool CvGame::isProjectMaxedOut(ProjectTypes eIndex, int iExtra)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumProjectInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (!isWorldProject(eIndex))
	{
		return false;
	}

	FAssertMsg(getProjectCreatedCount(eIndex) <= GC.getProjectInfo(eIndex).getMaxGlobalInstances(), "Index is expected to be within maximum bounds (invalid Index)");

	return ((getProjectCreatedCount(eIndex) + iExtra) >= GC.getProjectInfo(eIndex).getMaxGlobalInstances());
}


void CvGame::incrementProjectCreatedCount(ProjectTypes eIndex, int iExtra)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumProjectInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_paiProjectCreatedCount[eIndex] += iExtra;
}


int CvGame::getForceCivicCount(CivicTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumCivicInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiForceCivicCount[eIndex];
}


bool CvGame::isForceCivic(CivicTypes eIndex) const
{
	return (getForceCivicCount(eIndex) > 0);
}


bool CvGame::isForceCivicOption(CivicOptionTypes eCivicOption) const
{
	int iI;

	for (iI = 0; iI < GC.getNumCivicInfos(); iI++)
	{
		if (GC.getCivicInfo((CivicTypes)iI).getCivicOptionType() == eCivicOption)
		{
			if (isForceCivic((CivicTypes)iI))
			{
				return true;
			}
		}
	}

	return false;
}


void CvGame::changeForceCivicCount(CivicTypes eIndex, int iChange)
{
	bool bOldForceCivic;

	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumCivicInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (iChange != 0)
	{
		bOldForceCivic = isForceCivic(eIndex);

		m_paiForceCivicCount[eIndex] = (m_paiForceCivicCount[eIndex] + iChange);
		FAssert(getForceCivicCount(eIndex) >= 0);

		if (bOldForceCivic != isForceCivic(eIndex))
		{
			verifyCivics();
		}
	}
}


int CvGame::getVoteOutcome(VoteTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumVoteInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiVoteOutcome[eIndex];
}


bool CvGame::isVotePassed(VoteTypes eIndex) const
{
	if (isTeamVote(eIndex))
	{
		return (getVoteOutcome(eIndex) != -1);
	}
	else
	{
		return (getVoteOutcome(eIndex) == 1);
	}
}


void CvGame::setVoteOutcome(VoteTypes eIndex, int iNewValue)
{
	bool bOldPassed;

	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumVoteInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (getVoteOutcome(eIndex) != iNewValue)
	{
		bOldPassed = isVotePassed(eIndex);

		m_paiVoteOutcome[eIndex] = iNewValue;

		if (bOldPassed != isVotePassed(eIndex))
		{
			processVote(eIndex, ((isVotePassed(eIndex)) ? 1 : -1));
		}
	}
}


int CvGame::getReligionGameTurnFounded(ReligionTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumReligionInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiReligionGameTurnFounded[eIndex];
}


bool CvGame::isReligionFounded(ReligionTypes eIndex)
{
	return (getReligionGameTurnFounded(eIndex) != -1);
}


void CvGame::makeReligionFounded(ReligionTypes eIndex, PlayerTypes ePlayer)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumReligionInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (!isReligionFounded(eIndex))
	{
		FAssertMsg(getGameTurn() != -1, "getGameTurn() is not expected to be equal with -1");
		m_paiReligionGameTurnFounded[eIndex] = getGameTurn();

		gDLL->getEventReporterIFace()->religionFounded(eIndex, ePlayer);
	}
}


bool CvGame::isVictoryValid(VictoryTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumVictoryInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return GC.getInitCore().getVictory(eIndex);
}

void CvGame::setVictoryValid(VictoryTypes eIndex, bool bValid)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumVictoryInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	GC.getInitCore().setVictory(eIndex, bValid);
}


bool CvGame::isSpecialUnitValid(SpecialUnitTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumSpecialUnitInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_pabSpecialUnitValid[eIndex];
}


void CvGame::makeSpecialUnitValid(SpecialUnitTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumSpecialUnitInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_pabSpecialUnitValid[eIndex] = true;
}


bool CvGame::isSpecialBuildingValid(SpecialBuildingTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumSpecialBuildingInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_pabSpecialBuildingValid[eIndex];
}


void CvGame::makeSpecialBuildingValid(SpecialBuildingTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumSpecialBuildingInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_pabSpecialBuildingValid[eIndex] = true;
}


bool CvGame::isVoteTriggered(VoteTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumVoteInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_pabVoteTriggered[eIndex];
}


void CvGame::setVoteTriggered(VoteTypes eIndex, bool bNewValue)
{
	TeamTypes eTeam;
	//bool bPass;
	int iI;

	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumVoteInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (isVoteTriggered(eIndex) != bNewValue)
	{
		m_pabVoteTriggered[eIndex] = bNewValue;

		if (isVoteTriggered(eIndex))
		{
			for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
			{
				setPlayerVote(((PlayerTypes)iI), eIndex, -1);
			}

			for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
			{
				if (GET_PLAYER((PlayerTypes)iI).isAlive())
				{
					if (GET_PLAYER((PlayerTypes)iI).isHuman())
					{
						CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_DIPLOVOTE);
						if (NULL != pInfo)
						{
							pInfo->setData1(eIndex);
							gDLL->getInterfaceIFace()->addPopup(pInfo, (PlayerTypes)iI);
						}
					}
					else
					{
						castVote(((PlayerTypes)iI), eIndex, GET_PLAYER((PlayerTypes)iI).AI_diploVote(eIndex));
					}
				}
			}
		}
		else
		{
			if (isTeamVote(eIndex))
			{
				eTeam = findHighestVoteTeam(eIndex);

				if ((eTeam != NO_TEAM) && (countVote(eIndex, eTeam) >= getVoteRequired(eIndex)))
				{
					setVoteOutcome(eIndex, eTeam);
				}
				else
				{
					setVoteOutcome(eIndex, -1);
				}
			}
			else
			{
				if (countVote(eIndex, 1) >= getVoteRequired(eIndex))
				{
					setVoteOutcome(eIndex, 1);
				}
				else
				{
					setVoteOutcome(eIndex, 0);
				}
			}

			for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
			{
				if (GET_PLAYER((PlayerTypes)iI).isAlive())
				{
					if (GET_PLAYER((PlayerTypes)iI).isHuman())
					{
						CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_DIPLOVOTE_RESULTS);
						if (NULL != pInfo)
						{
							pInfo->setData1(eIndex);
							gDLL->getInterfaceIFace()->addPopup(pInfo, (PlayerTypes)iI);
						}
					}
				}
			}
		}
	}
}


CvCity* CvGame::getHolyCity(ReligionTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumReligionInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return getCity(m_paHolyCity[eIndex]);
}


void CvGame::setHolyCity(ReligionTypes eIndex, CvCity* pNewValue, bool bAnnounce)
{
	CvWString szBuffer;
	CvCity* pOldValue;
	CvCity* pHolyCity;
	int iI;

	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumReligionInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	pOldValue = getHolyCity(eIndex);

	if (pOldValue != pNewValue)
	{
		updateCitySight(false);

		if (pNewValue != NULL)
		{
			m_paHolyCity[eIndex] = pNewValue->getIDInfo();
		}
		else
		{
			m_paHolyCity[eIndex].reset();
		}

		updateCitySight(true);

		if (pOldValue != NULL)
		{
			pOldValue->changeReligionInfluence(eIndex, -(GC.getDefineINT("HOLY_CITY_INFLUENCE")));

			pOldValue->updateReligionCommerce();

			pOldValue->setInfoDirty(true);
		}

		if (getHolyCity(eIndex) != NULL)
		{
			pHolyCity = getHolyCity(eIndex);

			pHolyCity->setHasReligion(eIndex, true, bAnnounce);
			pHolyCity->changeReligionInfluence(eIndex, GC.getDefineINT("HOLY_CITY_INFLUENCE"));

			pHolyCity->updateReligionCommerce();

			pHolyCity->setInfoDirty(true);

			if (bAnnounce)
			{
				if (isFinalInitialized() && !(gDLL->GetWorldBuilderMode()))
				{
					szBuffer = gDLL->getText("TXT_KEY_MISC_REL_FOUNDED", GC.getReligionInfo(eIndex).getTextKeyWide(), pHolyCity->getNameKey());
					addReplayMessage(REPLAY_MESSAGE_MAJOR_EVENT, pHolyCity->getOwnerINLINE(), szBuffer, pHolyCity->getX_INLINE(), pHolyCity->getY_INLINE(), (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"));

					for (iI = 0; iI < MAX_PLAYERS; iI++)
					{
						if (GET_PLAYER((PlayerTypes)iI).isAlive())
						{
							if (pHolyCity->isRevealed(GET_PLAYER((PlayerTypes)iI).getTeam(), false))
							{
								szBuffer = gDLL->getText("TXT_KEY_MISC_REL_FOUNDED", GC.getReligionInfo(eIndex).getTextKeyWide(), pHolyCity->getNameKey());
								gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), false, GC.getDefineINT("EVENT_MESSAGE_TIME_LONG"), szBuffer, GC.getReligionInfo(eIndex).getSound(), MESSAGE_TYPE_MAJOR_EVENT, GC.getReligionInfo(eIndex).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"), pHolyCity->getX_INLINE(), pHolyCity->getY_INLINE());
							}
							else
							{
								szBuffer = gDLL->getText("TXT_KEY_MISC_REL_FOUNDED_UNKNOWN", GC.getReligionInfo(eIndex).getTextKeyWide());
								gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), false, GC.getDefineINT("EVENT_MESSAGE_TIME_LONG"), szBuffer, GC.getReligionInfo(eIndex).getSound(), MESSAGE_TYPE_MAJOR_EVENT, GC.getReligionInfo(eIndex).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"));
							}
						}
					}
				}
			}
		}

		AI_makeAssignWorkDirty();
	}
}


int CvGame::getPlayerVote(PlayerTypes eOwnerIndex, VoteTypes eVoteIndex) const
{
	FAssert(eOwnerIndex >= 0);
	FAssert(eOwnerIndex < MAX_CIV_PLAYERS);
	FAssert(eVoteIndex >= 0);
	FAssert(eVoteIndex < GC.getNumVoteInfos());

	return m_apaiPlayerVote[eOwnerIndex][eVoteIndex];
}


void CvGame::setPlayerVote(PlayerTypes eOwnerIndex, VoteTypes eVoteIndex, int iNewValue)
{
	FAssert(eOwnerIndex >= 0);
	FAssert(eOwnerIndex < MAX_CIV_PLAYERS);
	FAssert(eVoteIndex >= 0);
	FAssert(eVoteIndex < GC.getNumVoteInfos());

	m_apaiPlayerVote[eOwnerIndex][eVoteIndex] = iNewValue;
}


void CvGame::castVote(PlayerTypes eOwnerIndex, VoteTypes eVoteIndex, int iVote)
{
	if (isVoteTriggered(eVoteIndex))
	{
		setPlayerVote(eOwnerIndex, eVoteIndex, iVote);
	}
}


std::string CvGame::getScriptData() const
{
	return m_szScriptData;
}


void CvGame::setScriptData(std::string szNewValue)
{
	m_szScriptData = szNewValue;
}

const CvWString & CvGame::getName()
{
	return GC.getInitCore().getGameName();
}


void CvGame::setName(const TCHAR* szName)
{
	GC.getInitCore().setGameName(szName);
}


bool CvGame::isDestroyedCityName(CvWString& szName) const
{
	std::vector<CvWString>::const_iterator it;

	for (it = m_aszDestroyedCities.begin(); it != m_aszDestroyedCities.end(); it++)
	{
		if (*it == szName)
		{
			return true;
		}
	}

	return false;
}

void CvGame::addDestroyedCityName(const CvWString& szName)
{
	m_aszDestroyedCities.push_back(szName);
}

bool CvGame::isGreatPersonBorn(CvWString& szName) const
{
	std::vector<CvWString>::const_iterator it;

	for (it = m_aszGreatPeopleBorn.begin(); it != m_aszGreatPeopleBorn.end(); it++)
	{
		if (*it == szName)
		{
			return true;
		}
	}

	return false;
}

void CvGame::addGreatPersonBornName(const CvWString& szName)
{
	m_aszGreatPeopleBorn.push_back(szName);
}


// Protected Functions...

void CvGame::doTurn()
{
	PROFILE_BEGIN("CvGame::doTurn()");

	int aiShuffle[MAX_PLAYERS];
	int iLoopPlayer;
	int iI;

	// END OF TURN
	gDLL->getEventReporterIFace()->beginGameTurn( getGameTurn() );

	doUpdateCacheOnTurn();

	updateScore();

	doDeals();

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		if (GET_TEAM((TeamTypes)iI).isAlive())
		{
			GET_TEAM((TeamTypes)iI).doTurn();
		}
	}

	GC.getMapINLINE().doTurn();

	createBarbarianCities();

	createBarbarianUnits();

	doGlobalWarming();

	doHolyCity();

	doDiploVote();

	gDLL->getInterfaceIFace()->setEndTurnMessage(false);
	gDLL->getInterfaceIFace()->setHasMovedUnit(false);

	if (getAIAutoPlay() > 0)
	{
		changeAIAutoPlay(-1);

		if (getAIAutoPlay() == 0)
		{
			reviveActivePlayer();
		}
	}

	// XXX
#ifdef _DEBUG
	if (!isGameMultiPlayer() && (getActivePlayer() != NO_PLAYER))
	{
		if (!(GET_PLAYER(getActivePlayer()).isAlive()))
		{
			if (getGameTurnYear() == 2000)
			{
				reviveActivePlayer();

				if (!isDebugMode())
				{
					toggleDebugMode();
				}
			}
		}
	}
#endif
	// XXX

	gDLL->getEventReporterIFace()->endGameTurn(getGameTurn());

	incrementGameTurn();
	incrementElapsedGameTurns();

	if (isMPOption(MPOPTION_SIMULTANEOUS_TURNS))
	{
		shuffleArray(aiShuffle, MAX_PLAYERS, GC.getGameINLINE().getSorenRand());

		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			iLoopPlayer = aiShuffle[iI];

			if (GET_PLAYER((PlayerTypes)iLoopPlayer).isAlive())
			{
				GET_PLAYER((PlayerTypes)iLoopPlayer).setTurnActive(true);
			}
		}
	}
	else
	{
		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if (isPbem() && GET_PLAYER((PlayerTypes)iI).isHuman())
				{
					if (iI == getActivePlayer())
					{
						// Nobody else left alive
						GC.getInitCore().setType(GAME_HOTSEAT_NEW);
						GET_PLAYER((PlayerTypes)iI).setTurnActive(true);
					}
					else if (!getPbemTurnSent())
					{
						gDLL->sendPbemTurn((PlayerTypes)iI);
					}
				}
				else
				{
					GET_PLAYER((PlayerTypes)iI).setTurnActive(true);
					FAssert(getNumGameTurnActive() == 1);
				}

				break;
			}
		}
	}

	testVictory();

	gDLL->getEngineIFace()->SetDirty(GlobePartialTexture_DIRTY_BIT, true);

	PROFILE_END();

	stopProfilingDLL();

	gDLL->getEngineIFace()->AutoSave();
}


void CvGame::doDeals()
{
	CvDeal* pLoopDeal;
	int iLoop;

	verifyDeals();

	for(pLoopDeal = firstDeal(&iLoop); pLoopDeal != NULL; pLoopDeal = nextDeal(&iLoop))
	{
		pLoopDeal->doTurn();
	}
}


void CvGame::doGlobalWarming()
{
	CvCity* pCity;
	CvPlot* pPlot;
	CvWString szBuffer;
	TerrainTypes eWarmingTerrain;
	bool bChanged;
	int iI;

	eWarmingTerrain = ((TerrainTypes)(GC.getDefineINT("GLOBAL_WARMING_TERRAIN")));

	for (iI = 0; iI < getNukesExploded(); iI++)
	{
		if (getSorenRandNum(100, "Global Warming") < GC.getDefineINT("GLOBAL_WARMING_PROB"))
		{
			pPlot = GC.getMapINLINE().syncRandPlot(RANDPLOT_LAND | RANDPLOT_NOT_CITY);

			if (pPlot != NULL)
			{
				bChanged = false;

				if (pPlot->getTerrainType() != eWarmingTerrain)
				{
					if (pPlot->calculateTotalBestNatureYield(NO_TEAM) > 1)
					{
						pPlot->setTerrainType(eWarmingTerrain);
						bChanged = true;
					}
				}

				if (pPlot->getFeatureType() != NO_FEATURE)
				{
					if (pPlot->getFeatureType() != GC.getDefineINT("NUKE_FEATURE"))
					{
						pPlot->setFeatureType(NO_FEATURE);
						bChanged = true;
					}
				}

				if (bChanged)
				{
					pPlot->setImprovementType(NO_IMPROVEMENT);

					pCity = GC.getMapINLINE().findCity(pPlot->getX_INLINE(), pPlot->getY_INLINE());
					if (pCity != NULL)
					{
						if (pPlot->isVisible(pCity->getTeam(), false))
						{
							szBuffer = gDLL->getText("TXT_KEY_MISC_GLOBAL_WARMING_NEAR_CITY", pCity->getNameKey());
							gDLL->getInterfaceIFace()->addMessage(pCity->getOwnerINLINE(), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_GLOBALWARMING", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
						}
					}
				}
			}
		}
	}
}


void CvGame::doHolyCity()
{
	PlayerTypes eBestPlayer;
	TeamTypes eBestTeam;
	long lResult;
	int iValue;
	int iBestValue;
	int iI, iJ, iK;

	lResult = 0;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "doHolyCity", NULL, &lResult);
	if (lResult == 1)
	{
		return;
	}

	if (getElapsedGameTurns() < 5)
	{
		return;
	}

	for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
	{
		if (!isReligionFounded((ReligionTypes)iI))
		{
			iBestValue = MAX_INT;
			eBestTeam = NO_TEAM;

			for (iJ = 0; iJ < MAX_TEAMS; iJ++)
			{
				if (GET_TEAM((TeamTypes)iJ).isAlive())
				{
					if (GET_TEAM((TeamTypes)iJ).isHasTech((TechTypes)(GC.getReligionInfo((ReligionTypes)iI).getTechPrereq())))
					{
						if (GET_TEAM((TeamTypes)iJ).getNumCities() > 0)
						{
							iValue = getSorenRandNum(10, "Found Religion (Team)");

							if (!(GET_TEAM((TeamTypes)iJ).isHuman()))
							{
								iValue += 10;
							}

							for (iK = 0; iK < GC.getNumReligionInfos(); iK++)
							{
								int iReligionCount = GET_TEAM((TeamTypes)iJ).getHasReligionCount((ReligionTypes)iK);

								if (iReligionCount > 0)
								{
									iValue += iReligionCount * 20;
									if (GC.getReligionInfo((ReligionTypes)iK).getFreeUnitClass() != -1 
										&& GC.getReligionInfo((ReligionTypes)iI).getFreeUnitClass() != -1)
									{
										iValue += 20;
									}
								}
							}

							if (iValue < iBestValue)
							{
								iBestValue = iValue;
								eBestTeam = ((TeamTypes)iJ);
							}
						}
					}
				}
			}

			if (eBestTeam != NO_TEAM)
			{
				iBestValue = MAX_INT;
				eBestPlayer = NO_PLAYER;

				for (iJ = 0; iJ < MAX_PLAYERS; iJ++)
				{
					if (GET_PLAYER((PlayerTypes)iJ).isAlive())
					{
						if (GET_PLAYER((PlayerTypes)iJ).getTeam() == eBestTeam)
						{
							if (GET_PLAYER((PlayerTypes)iJ).getNumCities() > 0)
							{
								iValue = getSorenRandNum(10, "Found Religion (Player)");

								if (!(GET_PLAYER((PlayerTypes)iJ).isHuman()))
								{
									iValue += 10;
								}

								for (iK = 0; iK < GC.getNumReligionInfos(); iK++)
								{
									int iReligionCount = GET_PLAYER((PlayerTypes)iJ).getHasReligionCount((ReligionTypes)iK);

									if (iReligionCount > 0)
									{
										iValue += iReligionCount * 20;
										if (GC.getReligionInfo((ReligionTypes)iK).getFreeUnitClass() != -1 
											&& GC.getReligionInfo((ReligionTypes)iI).getFreeUnitClass() != -1)
										{
											iValue += 20;
										}
									}
								}

								if (iValue < iBestValue)
								{
									iBestValue = iValue;
									eBestPlayer = ((PlayerTypes)iJ);
								}
							}
						}
					}
				}

				if (eBestPlayer != NO_PLAYER)
				{
					GET_PLAYER(eBestPlayer).foundReligion((ReligionTypes)iI);
				}
			}
		}
	}
}


void CvGame::doDiploVote()
{
	TeamTypes eSecretaryGeneral;
	PlayerTypes eSecretaryPlayer;
	VoteTypes eVote;
	int iI;

	for (iI = 0; iI < GC.getNumVoteInfos(); iI++)
	{
		if (GC.getVoteInfo((VoteTypes)iI).isSecretaryGeneral())
		{
			setVoteTriggered(((VoteTypes)iI), false);
		}
	}

	eSecretaryGeneral = getSecretaryGeneral();

	if (eSecretaryGeneral != NO_TEAM)
	{
		eSecretaryPlayer = GET_TEAM(eSecretaryGeneral).getSecretaryID();
	}
	else
	{
		eSecretaryPlayer = NO_PLAYER;
	}

	if (eSecretaryPlayer != NO_PLAYER)
	{
		if (!(GET_PLAYER(eSecretaryPlayer).isHuman()))
		{
			for (iI = 0; iI < GC.getNumVoteInfos(); iI++)
			{
				if (!(GC.getVoteInfo((VoteTypes)iI).isSecretaryGeneral()))
				{
					setVoteTriggered(((VoteTypes)iI), false);
				}
			}
		}
	}

	if (isDiploVote())
	{
		if ((getGameTurn() % GC.getDefineINT("DIPLO_VOTE_TURN_FREQUENCY")) == 0)
		{
			if (getSecretaryGeneralTimer() > 0)
			{
				changeSecretaryGeneralTimer(-1);
			}

			if ((eSecretaryGeneral != NO_TEAM) && (eSecretaryPlayer != NO_PLAYER) && (getSecretaryGeneralTimer() > 0))
			{
				if (GET_PLAYER(eSecretaryPlayer).isHuman())
				{
					CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_CHOOSEELECTION);
					if (NULL != pInfo)
					{
						gDLL->getInterfaceIFace()->addPopup(pInfo, eSecretaryPlayer);
					}
				}
				else
				{
					eVote = GET_TEAM(eSecretaryGeneral).AI_chooseElection();

					if (eVote != NO_VOTE)
					{
						setVoteTriggered(eVote, true);
					}
				}
			}
			else
			{
				setSecretaryGeneralTimer(GC.getDefineINT("DIPLO_VOTE_SECRETARY_GENERAL_INTERVAL"));

				for (iI = 0; iI < GC.getNumVoteInfos(); iI++)
				{
					if (GC.getVoteInfo((VoteTypes)iI).isSecretaryGeneral())
					{
						setVoteTriggered(((VoteTypes)iI), true);
						break;
					}
				}
			}
		}
	}
}


void CvGame::createBarbarianCities()
{
	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	long lResult;
	int iTargetCities;
	int iValue;
	int iBestValue;
	int iI;

	if (getMaxCityElimination() > 0)
	{
		return;
	}

	if (isOption(GAMEOPTION_NO_BARBARIANS))
	{
		return;
	}

	lResult = 0;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "createBarbarianCities", NULL, &lResult);
	if (lResult == 1)
	{
		return;
	}

	if (GC.getEraInfo(getCurrentEra()).isNoBarbCities())
	{
		return;
	}

	if (GC.getHandicapInfo(getHandicapType()).getUnownedTilesPerBarbarianCity() <= 0)
	{
		return;
	}

	if (getNumCivCities() < (countCivPlayersAlive() * 2))
	{
		return;
	}

	if (getElapsedGameTurns() < ((GC.getHandicapInfo(getHandicapType()).getBarbarianCityCreationTurnsElapsed() * GC.getGameSpeedInfo(getGameSpeedType()).getBarbPercent()) / 100))
	{
		return;
	}

	if (getSorenRandNum(100, "Barb City Creation") >= GC.getHandicapInfo(getHandicapType()).getBarbarianCityCreationProb())
	{
		return;
	}

	iBestValue = 0;
	pBestPlot = NULL;

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (!(pLoopPlot->isWater()))
		{
			if (!(pLoopPlot->isVisibleToCivTeam()))
			{
				iTargetCities = pLoopPlot->area()->getNumUnownedTiles();

				if (pLoopPlot->area()->getNumCities() == pLoopPlot->area()->getCitiesPerPlayer(BARBARIAN_PLAYER))
				{
					iTargetCities *= 3;
				}

				iTargetCities /= GC.getHandicapInfo(getHandicapType()).getUnownedTilesPerBarbarianCity();

				if (pLoopPlot->area()->getCitiesPerPlayer(BARBARIAN_PLAYER) < iTargetCities)
				{
					iValue = GET_PLAYER(BARBARIAN_PLAYER).AI_foundValue(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), GC.getDefineINT("MIN_BARBARIAN_CITY_STARTING_DISTANCE"));

					iValue += (100 + getSorenRandNum(50, "Barb City Found"));
					iValue /= 100;

					if (iValue > iBestValue)
					{
						iBestValue = iValue;
						pBestPlot = pLoopPlot;
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		GET_PLAYER(BARBARIAN_PLAYER).found(pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
	}
}


void CvGame::createBarbarianUnits()
{
	CvUnit* pLoopUnit;
	CvArea* pLoopArea;
	CvPlot* pPlot;
	UnitAITypes eBarbUnitAI;
	UnitTypes eBestUnit;
	UnitTypes eLoopUnit;
	bool bAnimals;
	long lResult;
	int iNeededBarbs;
	int iDivisor;
	int iValue;
	int iBestValue;
	int iLoop;
	int iI, iJ;

	if (isOption(GAMEOPTION_NO_BARBARIANS))
	{
		return;
	}

	lResult = 0;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "createBarbarianUnits", NULL, &lResult);
	if (lResult == 1)
	{
		return;
	}

	bAnimals = false;

	if (GC.getEraInfo(getCurrentEra()).isNoBarbUnits())
	{
		bAnimals = true;
	}

	if (getNumCivCities() < ((countCivPlayersAlive() * 3) / 2) && !isOption(GAMEOPTION_ONE_CITY_CHALLENGE))
	{
		bAnimals = true;
	}

	if (getElapsedGameTurns() < ((GC.getHandicapInfo(getHandicapType()).getBarbarianCreationTurnsElapsed() * GC.getGameSpeedInfo(getGameSpeedType()).getBarbPercent()) / 100))
	{
		bAnimals = true;
	}

	if (bAnimals)
	{
		createAnimals();
	}
	else
	{
		for(pLoopArea = GC.getMapINLINE().firstArea(&iLoop); pLoopArea != NULL; pLoopArea = GC.getMapINLINE().nextArea(&iLoop))
		{
			if (pLoopArea->isWater())
			{
				eBarbUnitAI = UNITAI_ATTACK_SEA;
				iDivisor = GC.getHandicapInfo(getHandicapType()).getUnownedWaterTilesPerBarbarianUnit();
			}
			else
			{
				eBarbUnitAI = UNITAI_ATTACK;
				iDivisor = GC.getHandicapInfo(getHandicapType()).getUnownedTilesPerBarbarianUnit();
			}

			if (isOption(GAMEOPTION_RAGING_BARBARIANS))
			{
				iDivisor = max(1, (iDivisor / 2));
			}

			if (iDivisor > 0)
			{
				iNeededBarbs = ((pLoopArea->getNumUnownedTiles() / iDivisor) - pLoopArea->getUnitsPerPlayer(BARBARIAN_PLAYER)); // XXX eventually need to measure how many barbs of eBarbUnitAI we have in this area...

				if (iNeededBarbs > 0)
				{
					iNeededBarbs = ((iNeededBarbs / 4) + 1);

					for (iI = 0; iI < iNeededBarbs; iI++)
					{
						pPlot = GC.getMapINLINE().syncRandPlot((RANDPLOT_NOT_VISIBLE_TO_CIV | RANDPLOT_ADJACENT_LAND | RANDPLOT_PASSIBLE), pLoopArea->getID(), GC.getDefineINT("MIN_BARBARIAN_STARTING_DISTANCE"));

						if (pPlot != NULL)
						{
							eBestUnit = NO_UNIT;
							iBestValue = 0;

							for (iJ = 0; iJ < GC.getNumUnitClassInfos(); iJ++)
							{
								bool bValid = false;
								eLoopUnit = ((UnitTypes)(GC.getCivilizationInfo(GET_PLAYER(BARBARIAN_PLAYER).getCivilizationType()).getCivilizationUnits(iJ)));

								if (eLoopUnit != NO_UNIT)
								{
									CvUnitInfo& kUnit = GC.getUnitInfo(eLoopUnit);

									bValid = (kUnit.getCombat() > 0 && !kUnit.isOnlyDefensive());

									if (bValid)
									{
										if (pLoopArea->isWater() && kUnit.getDomainType() != DOMAIN_SEA)
										{
											bValid = false;
										}
										else if (!pLoopArea->isWater() && kUnit.getDomainType() != DOMAIN_LAND)
										{
											bValid = false;
										}
									}

									if (bValid)
									{
										if (!GET_PLAYER(BARBARIAN_PLAYER).canTrain(eLoopUnit))
										{
											bValid = false;
										}
									}

									if (bValid)
									{
										if (NO_BONUS != kUnit.getPrereqAndBonus())
										{
											if (!GET_TEAM(BARBARIAN_TEAM).isHasTech((TechTypes)GC.getBonusInfo((BonusTypes)kUnit.getPrereqAndBonus()).getTechCityTrade()))
											{
												bValid = false;
											}
										}
									}

									if (bValid)
									{
										bool bFound = false;
										bool bRequires = false;
										for (int i = 0; i < GC.getNUM_UNIT_PREREQ_OR_BONUSES(); ++i)
										{
											TechTypes eTech = (TechTypes)kUnit.getPrereqOrBonuses(i);

											if (NO_TECH != eTech)
											{
												bRequires = true;

												if (GET_TEAM(BARBARIAN_TEAM).isHasTech(eTech))
												{
													bFound = true;
													break;
												}
											}
										}

										if (bRequires && !bFound)
										{
											bValid = false;
										}
									}

									if (bValid)
									{
										iValue = (1 + getSorenRandNum(1000, "Barb Unit Selection"));

										if (kUnit.getUnitAIType(eBarbUnitAI))
										{
											iValue += 200;
										}

										if (iValue > iBestValue)
										{
											eBestUnit = eLoopUnit;
											iBestValue = iValue;
										}
									}
								}
							}

							if (eBestUnit != NO_UNIT)
							{
								GET_PLAYER(BARBARIAN_PLAYER).initUnit(eBestUnit, pPlot->getX_INLINE(), pPlot->getY_INLINE(), eBarbUnitAI);
							}
						}
					}
				}
			}
		}

		for (pLoopUnit = GET_PLAYER(BARBARIAN_PLAYER).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER(BARBARIAN_PLAYER).nextUnit(&iLoop))
		{
			if (pLoopUnit->isAnimal())
			{
				pLoopUnit->kill(false);
				break;
			}
		}
	}
}


void CvGame::createAnimals()
{
	CvArea* pLoopArea;
	CvPlot* pPlot;
	UnitTypes eBestUnit;
	UnitTypes eLoopUnit;
	int iNeededAnimals;
	int iValue;
	int iBestValue;
	int iLoop;
	int iI, iJ;

	if (GC.getEraInfo(getCurrentEra()).isNoAnimals())
	{
		return;
	}

	if (GC.getHandicapInfo(getHandicapType()).getUnownedTilesPerGameAnimal() <= 0)
	{
		return;
	}

	if (getNumCivCities() < countCivPlayersAlive())
	{
		return;
	}

	if (getElapsedGameTurns() < 5)
	{
		return;
	}

	for(pLoopArea = GC.getMapINLINE().firstArea(&iLoop); pLoopArea != NULL; pLoopArea = GC.getMapINLINE().nextArea(&iLoop))
	{
		if (!(pLoopArea->isWater()))
		{
			iNeededAnimals = ((pLoopArea->getNumUnownedTiles() / GC.getHandicapInfo(getHandicapType()).getUnownedTilesPerGameAnimal()) - pLoopArea->getUnitsPerPlayer(BARBARIAN_PLAYER));

			if (iNeededAnimals > 0)
			{
				iNeededAnimals = ((iNeededAnimals / 5) + 1);

				for (iI = 0; iI < iNeededAnimals; iI++)
				{
					pPlot = GC.getMapINLINE().syncRandPlot((RANDPLOT_NOT_VISIBLE_TO_CIV | RANDPLOT_PASSIBLE), pLoopArea->getID(), GC.getDefineINT("MIN_ANIMAL_STARTING_DISTANCE"));

					if (pPlot != NULL)
					{
						eBestUnit = NO_UNIT;
						iBestValue = 0;

						for (iJ = 0; iJ < GC.getNumUnitClassInfos(); iJ++)
						{
							eLoopUnit = ((UnitTypes)(GC.getCivilizationInfo(GET_PLAYER(BARBARIAN_PLAYER).getCivilizationType()).getCivilizationUnits(iJ)));

							if (eLoopUnit != NO_UNIT)
							{
								if (GC.getUnitInfo(eLoopUnit).getUnitAIType(UNITAI_ANIMAL))
								{
									if ((pPlot->getFeatureType() != NO_FEATURE) ? GC.getUnitInfo(eLoopUnit).getFeatureNative(pPlot->getFeatureType()) : GC.getUnitInfo(eLoopUnit).getTerrainNative(pPlot->getTerrainType()))
									{
										iValue = (1 + getSorenRandNum(1000, "Animal Unit Selection"));

										if (iValue > iBestValue)
										{
											eBestUnit = eLoopUnit;
											iBestValue = iValue;
										}
									}
								}
							}
						}

						if (eBestUnit != NO_UNIT)
						{
							GET_PLAYER(BARBARIAN_PLAYER).initUnit(eBestUnit, pPlot->getX_INLINE(), pPlot->getY_INLINE(), UNITAI_ANIMAL);
						}
					}
				}
			}
		}
	}
}


void CvGame::updateWar()
{
	int iI, iJ;

	if (isOption(GAMEOPTION_ALWAYS_WAR))
	{
		for (iI = 0; iI < MAX_TEAMS; iI++)
		{
			if (GET_TEAM((TeamTypes)iI).isAlive())
			{
				if (GET_TEAM((TeamTypes)iI).isHuman())
				{
					for (iJ = 0; iJ < MAX_TEAMS; iJ++)
					{
						if (GET_TEAM((TeamTypes)iJ).isAlive())
						{
							if (!(GET_TEAM((TeamTypes)iJ).isHuman()))
							{
								FAssert(iI != iJ);

								if (GET_TEAM((TeamTypes)iI).isHasMet((TeamTypes)iJ))
								{
									if (!atWar(((TeamTypes)iI), ((TeamTypes)iJ)))
									{
										GET_TEAM((TeamTypes)iI).declareWar(((TeamTypes)iJ), false);
									}
								}
							}
						}
					}
				}
			}
		}
	}
}


void CvGame::updateMoves()
{
	CvSelectionGroup* pLoopSelectionGroup;
	int aiShuffle[MAX_PLAYERS];
	int iLoop;
	int iI;

	if (isMPOption(MPOPTION_SIMULTANEOUS_TURNS))
	{
		shuffleArray(aiShuffle, MAX_PLAYERS, GC.getGameINLINE().getSorenRand());
	}
	else
	{
		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			aiShuffle[iI] = iI;
		}
	}

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		CvPlayer& player = GET_PLAYER((PlayerTypes)(aiShuffle[iI]));

		if (player.isAlive())
		{
			if (player.isTurnActive())
			{
				if (!(player.isAutoMoves()))
				{
					player.AI_unitUpdate();

					if (!(player.isHuman()))
					{
						if (!(player.hasBusyUnit()) && !(player.hasReadyUnit(true)))
						{
							player.setAutoMoves(true);
						}
					}
				}

				if (player.isAutoMoves())
				{
					for(pLoopSelectionGroup = player.firstSelectionGroup(&iLoop); pLoopSelectionGroup; pLoopSelectionGroup = player.nextSelectionGroup(&iLoop))
					{
						pLoopSelectionGroup->autoMission();
					}

					if (!(player.hasBusyUnit()))
					{
						player.setAutoMoves(false);
					}
				}
			}
		}
	}
}


void CvGame::verifyCivics()
{
	int iI;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			GET_PLAYER((PlayerTypes)iI).verifyCivics();
		}
	}
}


void CvGame::updateTimers()
{
	int iI;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			GET_PLAYER((PlayerTypes)iI).updateTimers();
		}
	}
}


void CvGame::updateTurnTimer()
{
	int iI;

	// Are we using a turn timer?
	if (isMPOption(MPOPTION_TURN_TIMER))
	{
		// Has the turn expired?
		if (getTurnSlice() > getCutoffSlice())
		{
			for (iI = 0; iI < MAX_PLAYERS; iI++)
			{
				if (GET_PLAYER((PlayerTypes)iI).isAlive() && GET_PLAYER((PlayerTypes)iI).isTurnActive())
				{
					GET_PLAYER((PlayerTypes)iI).setEndTurn(true);

					if (!(isMPOption(MPOPTION_SIMULTANEOUS_TURNS)))
					{
						break;
					}
				}
			}
		}
	}
}


void CvGame::testAlive()
{
	int iI;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		GET_PLAYER((PlayerTypes)iI).verifyAlive();
	}
}


void CvGame::testVictory()
{
	CvCity* pLoopCity;
	bool bValid;
	bool bFound;
	bool bEndScore;
	long lResult;
	int iThreshold;
	int iCount;
	int iLoop;
	int iI, iJ, iK;

	if (getVictory() != NO_VICTORY)
	{
		return;
	}

	if (getGameState() == GAMESTATE_EXTENDED)
	{
		return;
	}

	updateScore();

	lResult = 1;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "isVictoryTest", NULL, &lResult);
	if (lResult == 0)
	{
		return;
	}

	bEndScore = false;

	for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
	{
		if (GET_TEAM((TeamTypes)iI).isAlive())
		{
			if (!(GET_TEAM((TeamTypes)iI).isMinorCiv()))
			{
				for (iJ = 0; iJ < GC.getNumVictoryInfos(); iJ++)
				{
					if (isVictoryValid((VictoryTypes)iJ))
					{
						bValid = true;

						if (bValid)
						{
							if (GC.getVictoryInfo((VictoryTypes)iJ).isEndScore())
							{
								bEndScore = true;

								if (getMaxTurns() == 0)
								{
									bValid = false;
								}
								else if (getElapsedGameTurns() < getMaxTurns())
								{
									bValid = false;
								}
								else
								{
									bFound = false;

									for (iK = 0; iK < MAX_CIV_TEAMS; iK++)
									{
										if (GET_TEAM((TeamTypes)iK).isAlive())
										{
											if (iK != iI)
											{
												if (getTeamScore((TeamTypes)iK) >= getTeamScore((TeamTypes)iI))
												{
													bFound = true;
													break;
												}
											}
										}
									}

									if (bFound)
									{
										bValid = false;
									}
								}
							}
						}

						if (bValid)
						{
							if (GC.getVictoryInfo((VictoryTypes)iJ).isTargetScore())
							{
								if (getTargetScore() == 0)
								{
									bValid = false;
								}
								else if (getTeamScore((TeamTypes)iI) < getTargetScore())
								{
									bValid = false;
								}
								else
								{
									bFound = false;

									for (iK = 0; iK < MAX_CIV_TEAMS; iK++)
									{
										if (GET_TEAM((TeamTypes)iK).isAlive())
										{
											if (iK != iI)
											{
												if (getTeamScore((TeamTypes)iK) >= getTeamScore((TeamTypes)iI))
												{
													bFound = true;
													break;
												}
											}
										}
									}

									if (bFound)
									{
										bValid = false;
									}
								}
							}
						}

						if (bValid)
						{
							if (GC.getVictoryInfo((VictoryTypes)iJ).isConquest())
							{
								if (GET_TEAM((TeamTypes)iI).getNumCities() == 0)
								{
									bValid = false;
								}
								else
								{
									bFound = false;

									for (iK = 0; iK < MAX_CIV_TEAMS; iK++)
									{
										if (GET_TEAM((TeamTypes)iK).isAlive())
										{
											if (iK != iI && !GET_TEAM((TeamTypes)iK).isVassal((TeamTypes)iI))
											{
												if (GET_TEAM((TeamTypes)iK).getNumCities() > 0)
												{
													bFound = true;
													break;
												}
											}
										}
									}

									if (bFound)
									{
										bValid = false;
									}
								}
							}
						}

						if (bValid)
						{
							if (GC.getVictoryInfo((VictoryTypes)iJ).isDiploVote())
							{
								bFound = false;

								for (iK = 0; iK < GC.getNumVoteInfos(); iK++)
								{
									if (GC.getVoteInfo((VoteTypes)iK).isVictory())
									{
										if (getVoteOutcome((VoteTypes)iK) == ((TeamTypes)iI))
										{
											bFound = true;
											break;
										}
									}
								}

								if (!bFound)
								{
									bValid = false;
								}
							}
						}

						if (bValid)
						{
							if (getAdjustedPopulationPercent((VictoryTypes)iJ) > 0)
							{
								if (100 * GET_TEAM((TeamTypes)iI).getTotalPopulation() < getTotalPopulation() * getAdjustedPopulationPercent((VictoryTypes)iJ))
								{
									bValid = false;
								}
							}
						}

						if (bValid)
						{
							if (getAdjustedLandPercent((VictoryTypes)iJ) > 0)
							{
								if (100 * GET_TEAM((TeamTypes)iI).getTotalLand() < GC.getMapINLINE().getLandPlots() * getAdjustedLandPercent((VictoryTypes)iJ))
								{
									bValid = false;
								}
							}
						}

						if (bValid)
						{
							if (GC.getVictoryInfo((VictoryTypes)iJ).getReligionPercent() > 0)
							{
								bFound = false;

								if (getNumCivCities() > (countCivPlayersAlive() * 2))
								{
									for (iK = 0; iK < GC.getNumReligionInfos(); iK++)
									{
										if (GET_TEAM((TeamTypes)iI).hasHolyCity((ReligionTypes)iK))
										{
											if (calculateReligionPercent((ReligionTypes)iK) >= GC.getVictoryInfo((VictoryTypes)iJ).getReligionPercent())
											{
												bFound = true;
												break;
											}
										}

										if (bFound)
										{
											break;
										}
									}
								}

								if (!bFound)
								{
									bValid = false;
								}
							}
						}

						if (bValid)
						{
							if ((GC.getVictoryInfo((VictoryTypes)iJ).getCityCulture() != NO_CULTURELEVEL) && (GC.getVictoryInfo((VictoryTypes)iJ).getNumCultureCities() > 0))
							{
								iCount = 0;

								for (iK = 0; iK < MAX_CIV_PLAYERS; iK++)
								{
									if (GET_PLAYER((PlayerTypes)iK).isAlive())
									{
										if (GET_PLAYER((PlayerTypes)iK).getTeam() == iI)
										{
											for (pLoopCity = GET_PLAYER((PlayerTypes)iK).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iK).nextCity(&iLoop))
											{
												if (pLoopCity->getCultureLevel() >= GC.getVictoryInfo((VictoryTypes)iJ).getCityCulture())
												{
													iCount++;
												}
											}
										}
									}
								}

								if (iCount < GC.getVictoryInfo((VictoryTypes)iJ).getNumCultureCities())
								{
									bValid = false;
								}
							}
						}

						if (bValid)
						{
							if (GC.getVictoryInfo((VictoryTypes)iJ).getTotalCultureRatio() > 0)
							{
								iThreshold = ((GET_TEAM((TeamTypes)iI).countTotalCulture() * 100) / GC.getVictoryInfo((VictoryTypes)iJ).getTotalCultureRatio());

								bFound = false;

								for (iK = 0; iK < MAX_CIV_TEAMS; iK++)
								{
									if (GET_TEAM((TeamTypes)iK).isAlive())
									{
										if (iK != iI)
										{
											if (GET_TEAM((TeamTypes)iK).countTotalCulture() > iThreshold)
											{
												bFound = true;
												break;
											}
										}
									}
								}

								if (bFound)
								{
									bValid = false;
								}
							}
						}

						if (bValid)
						{
							for (iK = 0; iK < GC.getNumBuildingClassInfos(); iK++)
							{
								if (GC.getBuildingClassInfo((BuildingClassTypes) iK).getVictoryThreshold(iJ) > GET_TEAM((TeamTypes)iI).getBuildingClassCount((BuildingClassTypes)iK))
								{
									bValid = false;
									break;
								}
							}
						}

						if (bValid)
						{
							for (iK = 0; iK < GC.getNumProjectInfos(); iK++)
							{
								if (GC.getProjectInfo((ProjectTypes) iK).getVictoryThreshold(iJ) > GET_TEAM((TeamTypes)iI).getProjectCount((ProjectTypes)iK))
								{
									bValid = false;
									break;
								}
							}
						}

						if (bValid)
						{
							setWinner(((TeamTypes)iI), ((VictoryTypes)iJ));
							break;
						}
					}
				}
			}
		}

		if (getVictory() != NO_VICTORY)
		{
			break;
		}
	}

	if (getVictory() == NO_VICTORY)
	{
		if (getMaxTurns() > 0)
		{
			if (getElapsedGameTurns() >= getMaxTurns())
			{
				if (!bEndScore)
				{
					if ((getAIAutoPlay() > 0) || gDLL->GetAutorun())
					{
						setGameState(GAMESTATE_EXTENDED);
					}
					else
					{
						setGameState(GAMESTATE_OVER);
					}
				}
			}
		}
	}
}


void CvGame::processVote(VoteTypes eVote, int iChange)
{
	int iI;

	changeTradeRoutes(GC.getVoteInfo(eVote).getTradeRoutes() * iChange);
	changeFreeTradeCount((GC.getVoteInfo(eVote).isFreeTrade()) ? iChange : 0);
	changeNoNukesCount((GC.getVoteInfo(eVote).isNoNukes()) ? iChange : 0);

	for (iI = 0; iI < GC.getNumCivicInfos(); iI++)
	{
		changeForceCivicCount(((CivicTypes)iI), ((GC.getVoteInfo(eVote).isForceCivic(iI)) ? iChange : 0));
	}
}


int CvGame::getIndexAfterLastDeal()
{
	return m_deals.getIndexAfterLast();
}


int CvGame::getNumDeals()
{
	return m_deals.getCount();
}


 CvDeal* CvGame::getDeal(int iID)																		
{
	return ((CvDeal *)(m_deals.getAt(iID)));
}


CvDeal* CvGame::addDeal()
{
	return ((CvDeal *)(m_deals.add()));
}


 void CvGame::deleteDeal(int iID)
{
	m_deals.removeAt(iID);
	gDLL->getInterfaceIFace()->setDirty(Foreign_Screen_DIRTY_BIT, true);
}

CvDeal* CvGame::firstDeal(int *pIterIdx, bool bRev)
{
	return !bRev ? m_deals.beginIter(pIterIdx) : m_deals.endIter(pIterIdx);
}


CvDeal* CvGame::nextDeal(int *pIterIdx, bool bRev)
{
	return !bRev ? m_deals.nextIter(pIterIdx) : m_deals.prevIter(pIterIdx);
}


 CvRandom& CvGame::getMapRand()																					
{
	return m_mapRand;
}


int CvGame::getMapRandNum(int iNum, const char* pszLog)
{
	return m_mapRand.get(iNum, pszLog);
}


CvRandom& CvGame::getSorenRand()																					
{
	return m_sorenRand;
}


int CvGame::getSorenRandNum(int iNum, const char* pszLog)
{
	return m_sorenRand.get(iNum, pszLog);
}


int CvGame::calculateSyncChecksum()
{
	PROFILE_FUNC();

	CvUnit* pLoopUnit;
	int iMultiplier;
	int iValue;
	int iLoop;
	int iI, iJ;

	iValue = 0;

	iValue += getMapRand().getSeed();
	iValue += getSorenRand().getSeed();

	iValue += getNumCities();
	iValue += getTotalPopulation();
	iValue += getNumDeals();

	iValue += GC.getMapINLINE().getOwnedPlots();
	iValue += GC.getMapINLINE().getNumAreas();

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isEverAlive())
		{
			iMultiplier = getPlayerScore((PlayerTypes)iI);

			switch (getTurnSlice() % 4)
			{
			case 0:
				iMultiplier += (GET_PLAYER((PlayerTypes)iI).getTotalPopulation() * 543271);
				iMultiplier += (GET_PLAYER((PlayerTypes)iI).getTotalLand() * 327382);
				iMultiplier += (GET_PLAYER((PlayerTypes)iI).getGold() * 107564);
				iMultiplier += (GET_PLAYER((PlayerTypes)iI).getAssets() * 327455);
				iMultiplier += (GET_PLAYER((PlayerTypes)iI).getPower() * 135647);
				iMultiplier += (GET_PLAYER((PlayerTypes)iI).getNumCities() * 436432);
				iMultiplier += (GET_PLAYER((PlayerTypes)iI).getNumUnits() * 324111);
				iMultiplier += (GET_PLAYER((PlayerTypes)iI).getNumSelectionGroups() * 215356);
				break;

			case 1:
				for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
				{
					iMultiplier += (GET_PLAYER((PlayerTypes)iI).calculateTotalYield((YieldTypes)iJ) * 432754);
				}

				for (iJ = 0; iJ < NUM_COMMERCE_TYPES; iJ++)
				{
					iMultiplier += (GET_PLAYER((PlayerTypes)iI).getCommerceRate((CommerceTypes)iJ) * 432789);
				}
				break;

			case 2:
				for (iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
				{
					iMultiplier += (GET_PLAYER((PlayerTypes)iI).getNumAvailableBonuses((BonusTypes)iJ) * 945732);
					iMultiplier += (GET_PLAYER((PlayerTypes)iI).getBonusImport((BonusTypes)iJ) * 326443);
					iMultiplier += (GET_PLAYER((PlayerTypes)iI).getBonusExport((BonusTypes)iJ) * 932211);
				}

				for (iJ = 0; iJ < GC.getNumImprovementInfos(); iJ++)
				{
					iMultiplier += (GET_PLAYER((PlayerTypes)iI).getImprovementCount((ImprovementTypes)iJ) * 883422);
				}

				for (iJ = 0; iJ < GC.getNumBuildingClassInfos(); iJ++)
				{
					iMultiplier += (GET_PLAYER((PlayerTypes)iI).getBuildingClassCountPlusMaking((BuildingClassTypes)iJ) * 954531);
				}

				for (iJ = 0; iJ < GC.getNumUnitClassInfos(); iJ++)
				{
					iMultiplier += (GET_PLAYER((PlayerTypes)iI).getUnitClassCountPlusMaking((UnitClassTypes)iJ) * 754843);
				}

				for (iJ = 0; iJ < NUM_UNITAI_TYPES; iJ++)
				{
					iMultiplier += (GET_PLAYER((PlayerTypes)iI).AI_totalUnitAIs((UnitAITypes)iJ) * 643383);
				}
				break;

			case 3:
				for (pLoopUnit = GET_PLAYER((PlayerTypes)iI).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER((PlayerTypes)iI).nextUnit(&iLoop))
				{
					iMultiplier += (pLoopUnit->getX_INLINE() * 876543);
					iMultiplier += (pLoopUnit->getY_INLINE() * 985310);
					iMultiplier += (pLoopUnit->getDamage() * 736373);
					iMultiplier += (pLoopUnit->getExperience() * 820622);
					iMultiplier += (pLoopUnit->getLevel() * 367291);
				}
				break;
			}

			if (iMultiplier != 0)
			{
				iValue *= iMultiplier;
			}
		}
	}

#ifdef DEBUG_OUT_OF_SYNCS
	if (GC.getLogging())
	{
		TCHAR szOut[1024];
		sprintf(szOut, "SyncChecksum = %d on %d\n", iValue, GC.getGameINLINE().getTurnSlice());
		gDLL->messageControlLog(szOut);

		sprintf(szOut, "m=%d, s=%d, c=%d, p=%d, d=%d, o=%d, a=%d\n", 
			getMapRand().getSeed(),
			getSorenRand().getSeed(),
			getNumCities(),
			getTotalPopulation(),
			getNumDeals(),
			GC.getMapINLINE().getOwnedPlots(),
			GC.getMapINLINE().getNumAreas());
		gDLL->messageControlLog(szOut);

		sprintf(szOut, "m=%d, s=%d, c=%d, p=%d, d=%d, o=%d, a=%d\n", 
			getMapRand().getSeed(),
			getSorenRand().getSeed(),
			getNumCities(),
			getTotalPopulation(),
			getNumDeals(),
			GC.getMapINLINE().getOwnedPlots(),
			GC.getMapINLINE().getNumAreas());
		gDLL->messageControlLog(szOut);

		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isEverAlive())
			{
				sprintf(szOut, "%S: s=%d, p=%d, L=%d, g=%d, a=%d, pw=%d, c=%d, u=%d, sg=%d\n", 
					GET_PLAYER((PlayerTypes)iI).getName(),
					getPlayerScore((PlayerTypes)iI),
					GET_PLAYER((PlayerTypes)iI).getTotalPopulation(),
					GET_PLAYER((PlayerTypes)iI).getTotalLand(),
					GET_PLAYER((PlayerTypes)iI).getGold(),
					GET_PLAYER((PlayerTypes)iI).getAssets(),
					GET_PLAYER((PlayerTypes)iI).getPower(),
					GET_PLAYER((PlayerTypes)iI).getNumCities(),
					GET_PLAYER((PlayerTypes)iI).getNumUnits(),
					GET_PLAYER((PlayerTypes)iI).getNumSelectionGroups());
				gDLL->messageControlLog(szOut);

				sprintf(szOut, " yield: f=%d, p=%d, c=%d; commerce: g=%d, r=%d, c=%d\n", 
					GET_PLAYER((PlayerTypes)iI).calculateTotalYield(YIELD_FOOD),
					GET_PLAYER((PlayerTypes)iI).calculateTotalYield(YIELD_PRODUCTION),
					GET_PLAYER((PlayerTypes)iI).calculateTotalYield(YIELD_COMMERCE),
					GET_PLAYER((PlayerTypes)iI).getCommerceRate(COMMERCE_GOLD),
					GET_PLAYER((PlayerTypes)iI).getCommerceRate(COMMERCE_RESEARCH),
					GET_PLAYER((PlayerTypes)iI).getCommerceRate(COMMERCE_CULTURE));
				gDLL->messageControlLog(szOut);

				sprintf(szOut, " bonuses: ");
				gDLL->messageControlLog(szOut);
				for (iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
				{
					int iAvailableBonuses = GET_PLAYER((PlayerTypes)iI).getNumAvailableBonuses((BonusTypes)iJ);
					int iBonusImport = GET_PLAYER((PlayerTypes)iI).getBonusImport((BonusTypes)iJ);
					int iBonusExport = GET_PLAYER((PlayerTypes)iI).getBonusExport((BonusTypes)iJ);

					if (iAvailableBonuses != 0 || iBonusImport != 0 || iBonusExport != 0)
					{
						sprintf(szOut, "%S: f=%d, p=%d, c=%d; ", GC.getBonusInfo((BonusTypes)iJ).getDescription(), iAvailableBonuses, iBonusImport, iBonusExport);
						gDLL->messageControlLog(szOut);
					}
				}

				sprintf(szOut, "\n imrovements: ");
				gDLL->messageControlLog(szOut);
				for (iJ = 0; iJ < GC.getNumImprovementInfos(); iJ++)
				{
					int iImprovementCount = GET_PLAYER((PlayerTypes)iI).getImprovementCount((ImprovementTypes)iJ);
					if (iImprovementCount != 0)
					{
						sprintf(szOut, "%S=%d, ", GC.getImprovementInfo((ImprovementTypes)iJ).getDescription(), iImprovementCount);
						gDLL->messageControlLog(szOut);
					}
				}

				sprintf(szOut, "\n buildings: ");
				gDLL->messageControlLog(szOut);
				for (iJ = 0; iJ < GC.getNumBuildingClassInfos(); iJ++)
				{
					int iBuildingCount = GET_PLAYER((PlayerTypes)iI).getBuildingClassCountPlusMaking((BuildingClassTypes)iJ);
					if (iBuildingCount != 0)
					{
						sprintf(szOut, "%S=%d, ", GC.getBuildingClassInfo((BuildingClassTypes)iJ).getDescription(), iBuildingCount);
						gDLL->messageControlLog(szOut);
					}
				}

				sprintf(szOut, "\n unitclasses: ");
				gDLL->messageControlLog(szOut);
				for (iJ = 0; iJ < GC.getNumUnitClassInfos(); iJ++)
				{
					int iUnitCount = GET_PLAYER((PlayerTypes)iI).getUnitClassCountPlusMaking((UnitClassTypes)iJ);
					if (iUnitCount != 0)
					{
						sprintf(szOut, "%S=%d, ", GC.getUnitClassInfo((UnitClassTypes)iJ).getDescription(), iUnitCount);
						gDLL->messageControlLog(szOut);
					}
				}

				sprintf(szOut, "\n unitais: ");
				gDLL->messageControlLog(szOut);
				for (iJ = 0; iJ < NUM_UNITAI_TYPES; iJ++)
				{
					int iUnitAICount = GET_PLAYER((PlayerTypes)iI).AI_totalUnitAIs((UnitAITypes)iJ);
					if (iUnitAICount != 0)
					{
						sprintf(szOut, "%S=%d, ", GC.getUnitAIInfo((UnitAITypes)iJ).getDescription(), iUnitAICount);
						gDLL->messageControlLog(szOut);
					}
				}

				sprintf(szOut, "\n units: ");
				gDLL->messageControlLog(szOut);
				for (pLoopUnit = GET_PLAYER((PlayerTypes)iI).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER((PlayerTypes)iI).nextUnit(&iLoop))
				{
					sprintf(szOut, "%S(%d,%d), d=%d, %dexp, l=%d; ", 
						pLoopUnit->getName().GetCString(),
						pLoopUnit->getX_INLINE(),
						pLoopUnit->getY_INLINE(),
						pLoopUnit->getDamage(),
						pLoopUnit->getExperience(),
						pLoopUnit->getLevel());
					gDLL->messageControlLog(szOut);
				}
				
				sprintf(szOut, "\n\n");
				gDLL->messageControlLog(szOut);
			}
		}
	}
#endif

	return iValue;
}


int CvGame::calculateOptionsChecksum()
{
	PROFILE_FUNC();

	int iValue;
	int iI, iJ;

	iValue = 0;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		for (iJ = 0; iJ < NUM_PLAYEROPTION_TYPES; iJ++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isOption((PlayerOptionTypes)iJ))
			{
				iValue += (iI * 943097);
				iValue += (iJ * 281541);
			}
		}
	}

	return iValue;
}


void CvGame::addReplayMessage(ReplayMessageTypes eType, PlayerTypes ePlayer, CvWString pszText, int iPlotX, int iPlotY, ColorTypes eColor)
{
	int iGameTurn = getGameTurn();
	CvReplayMessage* pMessage = new CvReplayMessage(iGameTurn, eType, ePlayer);
	if (NULL != pMessage)
	{
		pMessage->setPlot(iPlotX, iPlotY);
		pMessage->setText(pszText);
		if (NO_COLOR == eColor)
		{
			eColor = (ColorTypes)GC.getInfoTypeForString("COLOR_WHITE");
		}
		pMessage->setColor(eColor);
		m_listReplayMessages.push_back(pMessage);
	}
}

void CvGame::clearReplayMessageMap()
{
	for (ReplayMessageList::const_iterator itList = m_listReplayMessages.begin(); itList != m_listReplayMessages.end(); itList++)
	{
		const CvReplayMessage* pMessage = *itList;
		if (NULL != pMessage)
		{
			delete pMessage;
		}
	}
	m_listReplayMessages.clear();
}

int CvGame::getReplayMessageTurn(uint i) const
{
	if (i >= m_listReplayMessages.size())
	{
		return (-1);
	}
	const CvReplayMessage* pMessage =  m_listReplayMessages[i];
	if (NULL == pMessage)
	{
		return (-1);
	}
	return pMessage->getTurn();
}

ReplayMessageTypes CvGame::getReplayMessageType(uint i) const
{
	if (i >= m_listReplayMessages.size())
	{
		return (NO_REPLAY_MESSAGE);
	}
	const CvReplayMessage* pMessage =  m_listReplayMessages[i];
	if (NULL == pMessage)
	{
		return (NO_REPLAY_MESSAGE);
	}
	return pMessage->getType();
}

int CvGame::getReplayMessagePlotX(uint i) const
{
	if (i >= m_listReplayMessages.size())
	{
		return (-1);
	}
	const CvReplayMessage* pMessage =  m_listReplayMessages[i];
	if (NULL == pMessage)
	{
		return (-1);
	}
	return pMessage->getPlotX();
}

int CvGame::getReplayMessagePlotY(uint i) const
{
	if (i >= m_listReplayMessages.size())
	{
		return (-1);
	}
	const CvReplayMessage* pMessage =  m_listReplayMessages[i];
	if (NULL == pMessage)
	{
		return (-1);
	}
	return pMessage->getPlotY();
}

PlayerTypes CvGame::getReplayMessagePlayer(uint i) const
{
	if (i >= m_listReplayMessages.size())
	{
		return (NO_PLAYER);
	}
	const CvReplayMessage* pMessage =  m_listReplayMessages[i];
	if (NULL == pMessage)
	{
		return (NO_PLAYER);
	}
	return pMessage->getPlayer();
}

LPCWSTR CvGame::getReplayMessageText(uint i) const
{
	if (i >= m_listReplayMessages.size())
	{
		return (NULL);
	}
	const CvReplayMessage* pMessage =  m_listReplayMessages[i];
	if (NULL == pMessage)
	{
		return (NULL);
	}
	return pMessage->getText().GetCString();
}

ColorTypes CvGame::getReplayMessageColor(uint i) const
{
	if (i >= m_listReplayMessages.size())
	{
		return (NO_COLOR);
	}
	const CvReplayMessage* pMessage =  m_listReplayMessages[i];
	if (NULL == pMessage)
	{
		return (NO_COLOR);
	}
	return pMessage->getColor();
}


uint CvGame::getNumReplayMessages() const
{
	return m_listReplayMessages.size();
}

// Private Functions...

void CvGame::read(FDataStreamBase* pStream)
{
	int iI;

	reset(NO_HANDICAP);

	uint uiFlag=0;
	pStream->Read(&uiFlag);	// flags for expansion

	pStream->Read(&m_iEndTurnMessagesSent);
	pStream->Read(&m_iElapsedGameTurns);
	pStream->Read(&m_iStartTurn);
	pStream->Read(&m_iStartYear);
	pStream->Read(&m_iEstimateEndTurn);
	pStream->Read(&m_iTurnSlice);
	pStream->Read(&m_iCutoffSlice);
	pStream->Read(&m_iNumGameTurnActive);
	pStream->Read(&m_iNumCities);
	pStream->Read(&m_iTotalPopulation);
	pStream->Read(&m_iTradeRoutes);
	pStream->Read(&m_iFreeTradeCount);
	pStream->Read(&m_iNoNukesCount);
	pStream->Read(&m_iSecretaryGeneralTimer);
	pStream->Read(&m_iNukesExploded);
	pStream->Read(&m_iMaxPopulation);
	pStream->Read(&m_iMaxLand);
	pStream->Read(&m_iMaxTech);
	pStream->Read(&m_iMaxWonders);
	pStream->Read(&m_iInitPopulation);
	pStream->Read(&m_iInitLand);
	pStream->Read(&m_iInitTech);
	pStream->Read(&m_iInitWonders);
	pStream->Read(&m_iAIAutoPlay);

	// m_uiInitialTime not saved

	pStream->Read(&m_bScoreDirty);
	pStream->Read(&m_bCircumnavigated);
	pStream->Read(&m_bDiploVote);
	// m_bDebugMode not saved
	pStream->Read(&m_bFinalInitialized);
	// m_bPbemTurnSent not saved
	pStream->Read(&m_bHotPbemBetweenTurns);
	// m_bPlayerOptionsSent not saved

	pStream->Read((int*)&m_eHandicap);
	pStream->Read((int*)&m_ePausePlayer);
	pStream->Read((int*)&m_eBestLandUnit);
	pStream->Read((int*)&m_eWinner);
	pStream->Read((int*)&m_eVictory);
	pStream->Read((int*)&m_eGameState);

	pStream->ReadString(m_szScriptData);

	pStream->Read(MAX_PLAYERS, m_aiEndTurnMessagesReceived);
	pStream->Read(MAX_PLAYERS, m_aiRankPlayer);
	pStream->Read(MAX_PLAYERS, m_aiPlayerRank);
	pStream->Read(MAX_PLAYERS, m_aiPlayerScore);
	pStream->Read(MAX_TEAMS, m_aiRankTeam);
	pStream->Read(MAX_TEAMS, m_aiTeamRank);
	pStream->Read(MAX_TEAMS, m_aiTeamScore);

	pStream->Read(GC.getNumUnitInfos(), m_paiUnitCreatedCount);
	pStream->Read(GC.getNumUnitClassInfos(), m_paiUnitClassCreatedCount);
	pStream->Read(GC.getNumBuildingClassInfos(), m_paiBuildingClassCreatedCount);
	pStream->Read(GC.getNumProjectInfos(), m_paiProjectCreatedCount);
	pStream->Read(GC.getNumCivicInfos(), m_paiForceCivicCount);
	pStream->Read(GC.getNumVoteInfos(), m_paiVoteOutcome);
	pStream->Read(GC.getNumReligionInfos(), m_paiReligionGameTurnFounded);

	pStream->Read(GC.getNumSpecialUnitInfos(), m_pabSpecialUnitValid);
	pStream->Read(GC.getNumSpecialBuildingInfos(), m_pabSpecialBuildingValid);
	pStream->Read(GC.getNumVoteInfos(), m_pabVoteTriggered);

	for (iI=0;iI<GC.getNumReligionInfos();iI++)
	{
		pStream->Read((int*)&m_paHolyCity[iI].eOwner);
		pStream->Read(&m_paHolyCity[iI].iID);
	}

	for (iI=0;iI<MAX_CIV_PLAYERS;iI++)
	{
		pStream->Read(GC.getNumVoteInfos(), m_apaiPlayerVote[iI]);
	}

	{
		CvWString szBuffer;
		uint iSize;

		m_aszDestroyedCities.clear();
		pStream->Read(&iSize);
		for (uint i = 0; i < iSize; i++)
		{
			pStream->ReadString(szBuffer);
			m_aszDestroyedCities.push_back(szBuffer);
		}

		m_aszGreatPeopleBorn.clear();
		pStream->Read(&iSize);
		for (uint i = 0; i < iSize; i++)
		{
			pStream->ReadString(szBuffer);
			m_aszGreatPeopleBorn.push_back(szBuffer);
		}
	}

	ReadStreamableFFreeListTrashArray(m_deals, pStream);

	m_mapRand.read(pStream);
	m_sorenRand.read(pStream);

	clearReplayMessageMap();
	ReplayMessageList::_Alloc::size_type iSize;
	pStream->Read(&iSize);
	for (ReplayMessageList::_Alloc::size_type i = 0; i < iSize; i++)
	{
		CvReplayMessage* pMessage = new CvReplayMessage(0);
		if (NULL != pMessage)
		{
			pMessage->read(*pStream);
		}
		m_listReplayMessages.push_back(pMessage);
	}
	// m_pReplayInfo not saved

	pStream->Read(&m_iNumSessions);
	if (!isNetworkMultiPlayer())
	{
		++m_iNumSessions;
	}

	// Get the active player information from the initialization structure
	if (!isGameMultiPlayer())
	{
		for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isHuman())
			{
				setActivePlayer((PlayerTypes)iI);
				break;
			}
		}
		addReplayMessage(REPLAY_MESSAGE_MAJOR_EVENT, getActivePlayer(), gDLL->getText("TXT_KEY_MISC_RELOAD", m_iNumSessions));
	}

	if (isOption(GAMEOPTION_NEW_RANDOM_SEED))
	{
		if (!isNetworkMultiPlayer())
		{
			m_sorenRand.reseed(timeGetTime());
		}
	}

	doUpdateCacheOnTurn();
}


void CvGame::write(FDataStreamBase* pStream)
{
	int iI;

	uint uiFlag=0;
	pStream->Write(uiFlag);		// flag for expansion

	pStream->Write(m_iEndTurnMessagesSent);
	pStream->Write(m_iElapsedGameTurns);
	pStream->Write(m_iStartTurn);
	pStream->Write(m_iStartYear);
	pStream->Write(m_iEstimateEndTurn);
	pStream->Write(m_iTurnSlice);
	pStream->Write(m_iCutoffSlice);
	pStream->Write(m_iNumGameTurnActive);
	pStream->Write(m_iNumCities);
	pStream->Write(m_iTotalPopulation);
	pStream->Write(m_iTradeRoutes);
	pStream->Write(m_iFreeTradeCount);
	pStream->Write(m_iNoNukesCount);
	pStream->Write(m_iSecretaryGeneralTimer);
	pStream->Write(m_iNukesExploded);
	pStream->Write(m_iMaxPopulation);
	pStream->Write(m_iMaxLand);
	pStream->Write(m_iMaxTech);
	pStream->Write(m_iMaxWonders);
	pStream->Write(m_iInitPopulation);
	pStream->Write(m_iInitLand);
	pStream->Write(m_iInitTech);
	pStream->Write(m_iInitWonders);
	pStream->Write(m_iAIAutoPlay);

	// m_uiInitialTime not saved

	pStream->Write(m_bScoreDirty);
	pStream->Write(m_bCircumnavigated);
	pStream->Write(m_bDiploVote);
	// m_bDebugMode not saved
	pStream->Write(m_bFinalInitialized);
	// m_bPbemTurnSent not saved
	pStream->Write(m_bHotPbemBetweenTurns);
	// m_bPlayerOptionsSent not saved

	pStream->Write(m_eHandicap);
	pStream->Write(m_ePausePlayer);
	pStream->Write(m_eBestLandUnit);
	pStream->Write(m_eWinner);
	pStream->Write(m_eVictory);
	pStream->Write(m_eGameState);

	pStream->WriteString(m_szScriptData);

	pStream->Write(MAX_PLAYERS, m_aiEndTurnMessagesReceived);
	pStream->Write(MAX_PLAYERS, m_aiRankPlayer);
	pStream->Write(MAX_PLAYERS, m_aiPlayerRank);
	pStream->Write(MAX_PLAYERS, m_aiPlayerScore);
	pStream->Write(MAX_TEAMS, m_aiRankTeam);
	pStream->Write(MAX_TEAMS, m_aiTeamRank);
	pStream->Write(MAX_TEAMS, m_aiTeamScore);

	pStream->Write(GC.getNumUnitInfos(), m_paiUnitCreatedCount);
	pStream->Write(GC.getNumUnitClassInfos(), m_paiUnitClassCreatedCount);
	pStream->Write(GC.getNumBuildingClassInfos(), m_paiBuildingClassCreatedCount);
	pStream->Write(GC.getNumProjectInfos(), m_paiProjectCreatedCount);
	pStream->Write(GC.getNumCivicInfos(), m_paiForceCivicCount);
	pStream->Write(GC.getNumVoteInfos(), m_paiVoteOutcome);
	pStream->Write(GC.getNumReligionInfos(), m_paiReligionGameTurnFounded);

	pStream->Write(GC.getNumSpecialUnitInfos(), m_pabSpecialUnitValid);
	pStream->Write(GC.getNumSpecialBuildingInfos(), m_pabSpecialBuildingValid);
	pStream->Write(GC.getNumVoteInfos(), m_pabVoteTriggered);

	for (iI=0;iI<GC.getNumReligionInfos();iI++)
	{
		pStream->Write(m_paHolyCity[iI].eOwner);
		pStream->Write(m_paHolyCity[iI].iID);
	}

	for (iI=0;iI<MAX_CIV_PLAYERS;iI++)
	{
		pStream->Write(GC.getNumVoteInfos(), m_apaiPlayerVote[iI]);
	}

	{
		std::vector<CvWString>::iterator it;

		pStream->Write(m_aszDestroyedCities.size());
		for (it = m_aszDestroyedCities.begin(); it != m_aszDestroyedCities.end(); it++)
		{
			pStream->WriteString(*it);
		}

		pStream->Write(m_aszGreatPeopleBorn.size());
		for (it = m_aszGreatPeopleBorn.begin(); it != m_aszGreatPeopleBorn.end(); it++)
		{
			pStream->WriteString(*it);
		}
	}

	WriteStreamableFFreeListTrashArray(m_deals, pStream);

	m_mapRand.write(pStream);
	m_sorenRand.write(pStream);

	ReplayMessageList::_Alloc::size_type iSize = m_listReplayMessages.size();
	pStream->Write(iSize);
	ReplayMessageList::const_iterator it;
	for (it = m_listReplayMessages.begin(); it != m_listReplayMessages.end(); it++)
	{
		const CvReplayMessage* pMessage = *it;
		if (NULL != pMessage)
		{
			pMessage->write(*pStream);
		}
	}
	// m_pReplayInfo not saved

	pStream->Write(m_iNumSessions);
}

void CvGame::writeReplay(FDataStreamBase& stream, PlayerTypes ePlayer)
{
	SAFE_DELETE(m_pReplayInfo);
	m_pReplayInfo = new CvReplayInfo();
	if (m_pReplayInfo)
	{
		m_pReplayInfo->createInfo(ePlayer);

		m_pReplayInfo->write(stream);
	}
}

void CvGame::saveReplay(PlayerTypes ePlayer)
{
	gDLL->getEngineIFace()->SaveReplay(ePlayer);
}


void CvGame::showEndGameSequence()
{
	CvPopupInfo* pInfo;
	CvWString szBuffer;
	int iI;

	long iHours = getMinutesPlayed() / 60;
	long iMinutes = getMinutesPlayed() % 60;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		CvPlayer& player = GET_PLAYER((PlayerTypes)iI);
		if (player.isHuman())
		{
			addReplayMessage(REPLAY_MESSAGE_MAJOR_EVENT, (PlayerTypes)iI, gDLL->getText("TXT_KEY_MISC_TIME_SPENT", iHours, iMinutes));

			pInfo = new CvPopupInfo(BUTTONPOPUP_TEXT);
			if (NULL != pInfo)
			{
				if ((getWinner() != NO_TEAM) && (getVictory() != NO_VICTORY))
				{
					pInfo->setText(gDLL->getText("TXT_KEY_GAME_WON", GET_TEAM(getWinner()).getName().GetCString(), GC.getVictoryInfo(getVictory()).getTextKeyWide()));
				}
				else
				{
					pInfo->setText(gDLL->getText("TXT_KEY_MISC_DEFEAT"));
				}
				player.addPopup(pInfo);
			}

			if (getWinner() == player.getTeam())
			{
				if (!CvString(GC.getVictoryInfo(getVictory()).getMovie()).empty())
				{
					// show movie
					pInfo = new CvPopupInfo(BUTTONPOPUP_PYTHON_SCREEN);
					if (NULL != pInfo)
					{
						pInfo->setText(L"showVictoryMovie");
						pInfo->setData1((int)getVictory());
						player.addPopup(pInfo);
					}
				}
				else if (GC.getVictoryInfo(getVictory()).isDiploVote())
				{
					pInfo = new CvPopupInfo(BUTTONPOPUP_PYTHON_SCREEN);
					if (NULL != pInfo)
					{
						pInfo->setText(L"showUnVictoryScreen");
						player.addPopup(pInfo);
					}
				}
			}

			// show replay
			pInfo = new CvPopupInfo(BUTTONPOPUP_PYTHON_SCREEN);
			if (NULL != pInfo)
			{
				pInfo->setText(L"showReplay"); 
				pInfo->setData1(iI);
				pInfo->setOption1(false); // don't go to HOF on exit
				player.addPopup(pInfo);
			}

			// show top cities / stats
			pInfo = new CvPopupInfo(BUTTONPOPUP_PYTHON_SCREEN);
			if (NULL != pInfo)
			{
				pInfo->setText(L"showInfoScreen");
				pInfo->setData1(0);
				pInfo->setData2(1);
				player.addPopup(pInfo);
			}

			// show Dan
			pInfo = new CvPopupInfo(BUTTONPOPUP_PYTHON_SCREEN);
			if (NULL != pInfo)
			{
				pInfo->setText(L"showDanQuayleScreen");
				player.addPopup(pInfo);
			}

			// show Hall of Fame
			pInfo = new CvPopupInfo(BUTTONPOPUP_PYTHON_SCREEN);
			if (NULL != pInfo)
			{
				pInfo->setText(L"showHallOfFame");
				player.addPopup(pInfo);
			}
		}
	}
}

CvReplayInfo* CvGame::getReplayInfo() const
{
	return m_pReplayInfo;
}

void CvGame::setReplayInfo(CvReplayInfo* pReplay)
{
	SAFE_DELETE(m_pReplayInfo);
	m_pReplayInfo = pReplay;
}

bool CvGame::hasSkippedSaveChecksum() const
{
	return gDLL->hasSkippedSaveChecksum();
}

// CACHE: cache frequently used values
///////////////////////////////////////


int CvGame::getShrineBuildingCount(ReligionTypes eReligion)
{
	int	iShrineBuildingCount = 0;

	if (eReligion == NO_RELIGION)
		iShrineBuildingCount = m_iShrineBuildingCount;
	else for (int iI = 0; iI < m_iShrineBuildingCount; iI++)
		if (m_aiShrineReligion[iI] == eReligion)
			iShrineBuildingCount++;

	return iShrineBuildingCount;
}

BuildingTypes CvGame::getShrineBuilding(int eIndex, ReligionTypes eReligion)
{
	FAssertMsg(eIndex >= 0 && eIndex < m_iShrineBuildingCount, "invalid index to CvGame::getShrineBuilding");

	BuildingTypes eBuilding = NO_BUILDING;

	if (eIndex >= 0 && eIndex < m_iShrineBuildingCount)
	{
		if (eReligion == NO_RELIGION)
			eBuilding = (BuildingTypes) m_aiShrineBuilding[eIndex];
		else for (int iI = 0, iReligiousBuilding = 0; iI < m_iShrineBuildingCount; iI++)
			if (m_aiShrineReligion[iI] == (int) eReligion)
			{
				if (iReligiousBuilding == eIndex)
				{
					// found it
					eBuilding = (BuildingTypes) m_aiShrineBuilding[iI];
					break;
				}

				iReligiousBuilding++;
			}
	}

	return eBuilding;
}

void CvGame::changeShrineBuilding(BuildingTypes eBuilding, ReligionTypes eReligion, bool bRemove)
{
	FAssertMsg(eBuilding >= 0 && eBuilding < GC.getNumBuildingInfos(), "invalid index to CvGame::changeShrineBuilding");
	FAssertMsg(bRemove || m_iShrineBuildingCount < GC.getNumBuildingInfos(), "trying to add too many buildings to CvGame::changeShrineBuilding");

	if (bRemove)
	{
		bool bFound = false;

		for (int iI = 0; iI < m_iShrineBuildingCount; iI++)
		{
			if (!bFound)
			{
				// note, eReligion is not important if we removing, since each building is always one religion
				if (m_aiShrineBuilding[iI] == (int) eBuilding)
					bFound = true;
			}
			
			if (bFound)
			{
				int iToMove = iI + 1;
				if (iToMove < m_iShrineBuildingCount)
				{
					m_aiShrineBuilding[iI] = m_aiShrineBuilding[iToMove];
					m_aiShrineReligion[iI] = m_aiShrineReligion[iToMove];
				}
				else
				{
					m_aiShrineBuilding[iI] = (int) NO_BUILDING;
					m_aiShrineReligion[iI] = (int) NO_RELIGION;
				}
			}

		if (bFound)
			m_iShrineBuildingCount--;

		}
	}
	else if (m_iShrineBuildingCount < GC.getNumBuildingInfos())
	{
		// add this item to the end
		m_aiShrineBuilding[m_iShrineBuildingCount] = eBuilding;
		m_aiShrineReligion[m_iShrineBuildingCount] = eReligion;
		m_iShrineBuildingCount++;
	}
	
}

bool CvGame::culturalVictoryValid()
{
	if (m_iNumCultureVictoryCities > 0)
	{
		return true;
	}

	return false;
}

int CvGame::culturalVictoryNumCultureCities()
{
	return m_iNumCultureVictoryCities;
}

CultureLevelTypes CvGame::culturalVictoryCultureLevel()
{
	if (m_iNumCultureVictoryCities > 0)
	{
		return (CultureLevelTypes) m_eCultureVictoryCultureLevel;
	}
	
	return NO_CULTURELEVEL;
}


void CvGame::constructCache()
{
	// allocate known constants

	// zero everything out, just to be safe
	m_iShrineBuildingCount = 0;
	m_aiShrineBuilding = NULL;
	m_aiShrineReligion = NULL;

	m_iNumCultureVictoryCities = 0;
	
	// note resetCache(/*bConstructorCall*/ true) will be called by CvGame::CvGame
}

void CvGame::destructCache()
{
	uninitCache(); // note, will be called by CvGame::~CvGame as well

	// deallocate anything allocated in constructor
}

void CvGame::initCache()
{
	// init non-saved data

	// note resetCache(/*bConstructorCall*/ false) will be called by CvGame::init

	doUpdateCacheOnTurn();
}

void CvGame::uninitCache()
{
	m_iShrineBuildingCount = 0;

	SAFE_DELETE_ARRAY(m_aiShrineBuilding);
	SAFE_DELETE_ARRAY(m_aiShrineReligion);

	m_iNumCultureVictoryCities = 0;
}

void CvGame::resetCache(bool bConstructorCall)
{
	int	iI;
	
	uninitCache(); // note, will be called by CvGame::reset as well
	
	if (!bConstructorCall)
	{
		FAssertMsg(m_aiShrineBuilding==NULL, "about to leak memory, CvGame::m_aiShrineBuilding");
		FAssertMsg(m_aiShrineReligion==NULL, "about to leak memory, CvGame::m_aiShrineReligion");
		m_aiShrineBuilding = new int[GC.getNumBuildingInfos()];
		m_aiShrineReligion = new int[GC.getNumBuildingInfos()];
		for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
		{
			m_aiShrineBuilding[iI] = (int) NO_BUILDING;
			m_aiShrineReligion[iI] = (int) NO_RELIGION;
		}
	}
}


void CvGame::doUpdateCacheOnTurn()
{
	int	iI;
	
	// reset shrine count
	m_iShrineBuildingCount = 0;

	for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
	{
		CvBuildingInfo&	kBuildingInfo = GC.getBuildingInfo((BuildingTypes) iI);
		
		// if it is for holy city, then its a shrine-thing, add it
		if (kBuildingInfo.getHolyCity() != NO_RELIGION)
		{
			changeShrineBuilding((BuildingTypes) iI, (ReligionTypes) kBuildingInfo.getReligionType());
		}
	}

	// reset cultural victories
	m_iNumCultureVictoryCities = 0;
	for (iI = 0; iI < GC.getNumVictoryInfos(); iI++)
	{
		if (isVictoryValid((VictoryTypes) iI))
		{
			CvVictoryInfo& kVictoryInfo = GC.getVictoryInfo((VictoryTypes) iI);
			if (kVictoryInfo.getCityCulture() > 0)
			{
				int iNumCultureCities = kVictoryInfo.getNumCultureCities();
				if (iNumCultureCities > m_iNumCultureVictoryCities)
				{
					m_iNumCultureVictoryCities = iNumCultureCities;
					m_eCultureVictoryCultureLevel = kVictoryInfo.getCityCulture();
				}
			}
		}
	}

}
