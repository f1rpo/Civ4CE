// team.cpp

#include "CvGameCoreDLL.h"
#include "CvPlayerAI.h"
#include "CvDefines.h"
#include "CvGameCoreUtils.h"
#include "CvGlobals.h"
#include "CvTeamAI.h"
#include "CvPlayerAI.h"
#include "CvGameAI.h"
#include "CvMap.h"
#include "CvPlot.h"
#include "CvTeam.h"
#include "CvDLLInterfaceIFaceBase.h"
#include "CvDLLEventReporterIFaceBase.h"
#include "CvDLLEngineIFaceBase.h"
#include "CvArtFileMgr.h"
#include "CvDiploParameters.h"
#include "CvInfos.h"
#include "CvPopupInfo.h"
#include "CvDLLPythonIFaceBase.h"
#include "CyArgsList.h"
#include "FProfiler.h"

// Public Functions...

CvTeam::CvTeam()
{
	m_paiRouteChange = NULL;
	m_paiProjectCount = NULL;
	m_paiProjectMaking = NULL;
	m_paiUnitClassCount = NULL;
	m_paiBuildingClassCount = NULL;
	m_paiObsoleteBuildingCount = NULL;
	m_paiResearchProgress = NULL;
	m_paiTechCount = NULL;
	m_paiTerrainTradeCount = NULL;

	m_pabHasTech = NULL;
	m_pabNoTradeTech = NULL;

	m_ppaaiImprovementYieldChange = NULL;

	reset((TeamTypes)0, true);
}


CvTeam::~CvTeam()
{
	uninit();
}


void CvTeam::init(TeamTypes eID)
{
	//--------------------------------
	// Init saved data
	reset(eID);

	//--------------------------------
	// Init non-saved data

	//--------------------------------
	// Init other game data
	AI_init();
}


void CvTeam::uninit()
{
	int iI;

	SAFE_DELETE_ARRAY(m_paiRouteChange);
	SAFE_DELETE_ARRAY(m_paiProjectCount);
	SAFE_DELETE_ARRAY(m_paiProjectMaking);
	SAFE_DELETE_ARRAY(m_paiUnitClassCount);
	SAFE_DELETE_ARRAY(m_paiBuildingClassCount);
	SAFE_DELETE_ARRAY(m_paiObsoleteBuildingCount);
	SAFE_DELETE_ARRAY(m_paiResearchProgress);
	SAFE_DELETE_ARRAY(m_paiTechCount);
	SAFE_DELETE_ARRAY(m_paiTerrainTradeCount);

	SAFE_DELETE_ARRAY(m_pabHasTech);
	SAFE_DELETE_ARRAY(m_pabNoTradeTech);

	if (m_ppaaiImprovementYieldChange != NULL)
	{
		for (iI = 0; iI < GC.getNumImprovementInfos(); iI++)
		{
			SAFE_DELETE_ARRAY(m_ppaaiImprovementYieldChange[iI]);
		}
		SAFE_DELETE_ARRAY(m_ppaaiImprovementYieldChange);
	}
}


// FUNCTION: reset()
// Initializes data members that are serialized.
void CvTeam::reset(TeamTypes eID, bool bConstructorCall)
{
	int iI, iJ;

	//--------------------------------
	// Uninit class
	uninit();

	m_iNumMembers = 0;
	m_iAliveCount = 0;
	m_iEverAliveCount = 0;
	m_iNumCities = 0;
	m_iTotalPopulation = 0;
	m_iTotalLand = 0;
	m_iNukeInterception = 0;
	m_iForceTeamVoteEligibilityCount = 0;
	m_iExtraWaterSeeFromCount = 0;
	m_iMapTradingCount = 0;
	m_iTechTradingCount = 0;
	m_iGoldTradingCount = 0;
	m_iOpenBordersTradingCount = 0;
	m_iDefensivePactTradingCount = 0;
	m_iPermanentAllianceTradingCount = 0;
	m_iBridgeBuildingCount = 0;
	m_iIrrigationCount = 0;
	m_iIgnoreIrrigationCount = 0;
	m_iWaterWorkCount = 0;

	m_bMapCentering = false;

	m_eID = eID;

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		m_aiStolenVisibilityTimer[iI] = 0;
		m_aiWarWeariness[iI] = 0;
		m_aiTechShareCount[iI] = 0;
	}

	for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		m_aiCommerceFlexibleCount[iI] = 0;
	}

	for (iI = 0; iI < NUM_DOMAIN_TYPES; iI++)
	{
		m_aiExtraMoves[iI] = 0;
	}

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		m_abHasMet[iI] = false;
		m_abAtWar[iI] = false;
		m_abPermanentWarPeace[iI] = false;
		m_abOpenBorders[iI] = false;
		m_abDefensivePact[iI] = false;
		m_abForcePeace[iI] = false;
	}

	if (!bConstructorCall)
	{
		FAssertMsg(m_paiRouteChange==NULL, "about to leak memory, CvTeam::m_paiRouteChange");
		m_paiRouteChange = new int[GC.getNumRouteInfos()];
		for (iI = 0; iI < GC.getNumRouteInfos(); iI++)
		{
			m_paiRouteChange[iI] = 0;
		}

		FAssertMsg(m_paiProjectCount==NULL, "about to leak memory, CvPlayer::m_paiProjectCount");
		m_paiProjectCount = new int [GC.getNumProjectInfos()];
		FAssertMsg(m_paiProjectMaking==NULL, "about to leak memory, CvPlayer::m_paiProjectMaking");
		m_paiProjectMaking = new int [GC.getNumProjectInfos()];
		for (iI = 0; iI < GC.getNumProjectInfos(); iI++)
		{
			m_paiProjectCount[iI] = 0;
			m_paiProjectMaking[iI] = 0;
		}

		FAssertMsg(m_paiUnitClassCount==NULL, "about to leak memory, CvTeam::m_paiUnitClassCount");
		m_paiUnitClassCount = new int [GC.getNumUnitClassInfos()];
		for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
		{
			m_paiUnitClassCount[iI] = 0;
		}

		FAssertMsg(m_paiBuildingClassCount==NULL, "about to leak memory, CvTeam::m_paiBuildingClassCount");
		m_paiBuildingClassCount = new int [GC.getNumBuildingClassInfos()];
		for (iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
		{
			m_paiBuildingClassCount[iI] = 0;
		}

		FAssertMsg(m_paiObsoleteBuildingCount==NULL, "about to leak memory, CvTeam::m_paiObsoleteBuildingCount");
		m_paiObsoleteBuildingCount = new int[GC.getNumBuildingInfos()];
		for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
		{
			m_paiObsoleteBuildingCount[iI] = 0;
		}

		FAssertMsg(m_paiResearchProgress==NULL, "about to leak memory, CvPlayer::m_paiResearchProgress");
		m_paiResearchProgress = new int [GC.getNumTechInfos()];
		FAssertMsg(m_paiTechCount==NULL, "about to leak memory, CvPlayer::m_paiTechCount");
		m_paiTechCount = new int [GC.getNumTechInfos()];
		for (iI = 0; iI < GC.getNumTechInfos(); iI++)
		{
			m_paiResearchProgress[iI] = 0;
			m_paiTechCount[iI] = 0;
		}

		FAssertMsg(m_paiTerrainTradeCount==NULL, "about to leak memory, CvTeam::m_paiTerrainTradeCount");
		m_paiTerrainTradeCount = new int[GC.getNumTerrainInfos()];
		for (iI = 0; iI < GC.getNumTerrainInfos(); iI++)
		{
			m_paiTerrainTradeCount[iI] = 0;
		}

		FAssertMsg(m_pabHasTech==NULL, "about to leak memory, CvTeam::m_pabHasTech");
		m_pabHasTech = new bool[GC.getNumTechInfos()];
		FAssertMsg(m_pabNoTradeTech==NULL, "about to leak memory, CvTeam::m_pabNoTradeTech");
		m_pabNoTradeTech = new bool[GC.getNumTechInfos()];
		for (iI = 0; iI < GC.getNumTechInfos(); iI++)
		{
			m_pabHasTech[iI] = false;
			m_pabNoTradeTech[iI] = false;
		}

		FAssertMsg(m_ppaaiImprovementYieldChange==NULL, "about to leak memory, CvTeam::m_ppaaiImprovementYieldChange");
		m_ppaaiImprovementYieldChange = new int*[GC.getNumImprovementInfos()];
		for (iI = 0; iI < GC.getNumImprovementInfos(); iI++)
		{
			m_ppaaiImprovementYieldChange[iI] = new int[NUM_YIELD_TYPES];
			for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
			{
				m_ppaaiImprovementYieldChange[iI][iJ] = 0;
			}
		}

		AI_reset();
	}
}


void CvTeam::addTeam(TeamTypes eTeam)
{
	CLLNode<TradeData>* pNode;
	CvDeal* pLoopDeal;
	CvPlot* pLoopPlot;
	CvWString szBuffer;
	bool bValid;
	int iLoop;
	int iI, iJ;

	FAssert(eTeam != NO_TEAM);
	FAssert(eTeam != getID());

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if ((GET_PLAYER((PlayerTypes)iI).getTeam() != getID()) && (GET_PLAYER((PlayerTypes)iI).getTeam() != eTeam))
			{
				if (GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).isHasMet(getID()) && GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).isHasMet(eTeam))
				{
					szBuffer = gDLL->getText("TXT_KEY_MISC_PLAYER_PERMANENT_ALLIANCE", getName().GetCString(), GET_TEAM(eTeam).getName().GetCString());
					gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_THEIRALLIANCE", MESSAGE_TYPE_MINOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"));
				}
			}
		}
	}

	szBuffer = gDLL->getText("TXT_KEY_MISC_PLAYER_PERMANENT_ALLIANCE", getName().GetCString(), GET_TEAM(eTeam).getName().GetCString());
	GC.getGameINLINE().addReplayMessage(REPLAY_MESSAGE_MAJOR_EVENT, getLeaderID(), szBuffer, -1, -1, (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"));


	for (pLoopDeal = GC.getGameINLINE().firstDeal(&iLoop); pLoopDeal != NULL; pLoopDeal = GC.getGameINLINE().nextDeal(&iLoop))
	{
		if (((GET_PLAYER(pLoopDeal->getFirstPlayer()).getTeam() == getID()) && (GET_PLAYER(pLoopDeal->getSecondPlayer()).getTeam() == eTeam)) ||
			  ((GET_PLAYER(pLoopDeal->getFirstPlayer()).getTeam() == eTeam) && (GET_PLAYER(pLoopDeal->getSecondPlayer()).getTeam() == getID())))
		{
			bValid = true;

			if (pLoopDeal->getFirstTrades() != NULL)
			{
				for (pNode = pLoopDeal->getFirstTrades()->head(); pNode; pNode = pLoopDeal->getFirstTrades()->next(pNode))
				{
					if ((pNode->m_data.m_eItemType == TRADE_OPEN_BORDERS) ||
						  (pNode->m_data.m_eItemType == TRADE_DEFENSIVE_PACT) ||
						  (pNode->m_data.m_eItemType == TRADE_PEACE_TREATY))
					{
						bValid = false;
					}
				}
			}

			if (pLoopDeal->getSecondTrades() != NULL)
			{
				for (pNode = pLoopDeal->getSecondTrades()->head(); pNode; pNode = pLoopDeal->getSecondTrades()->next(pNode))
				{
					if ((pNode->m_data.m_eItemType == TRADE_OPEN_BORDERS) ||
						  (pNode->m_data.m_eItemType == TRADE_DEFENSIVE_PACT) ||
						  (pNode->m_data.m_eItemType == TRADE_PEACE_TREATY))
					{
						bValid = false;
					}
				}
			}

			if (!bValid)
			{
				pLoopDeal->kill();
			}
		}
	}

	shareItems(eTeam);
	GET_TEAM(eTeam).shareItems(getID());

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		if ((iI != getID()) && (iI != eTeam))
		{
			if (GET_TEAM((TeamTypes)iI).isAlive())
			{
				if (GET_TEAM(eTeam).isHasMet((TeamTypes)iI))
				{
					meet(((TeamTypes)iI), false);
				}
				else if (isHasMet((TeamTypes)iI))
				{
					GET_TEAM(eTeam).meet(((TeamTypes)iI), false);
				}
			}
		}
	}

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		if ((iI != getID()) && (iI != eTeam))
		{
			if (GET_TEAM((TeamTypes)iI).isAlive())
			{
				if (GET_TEAM(eTeam).isAtWar((TeamTypes)iI))
				{
					declareWar(((TeamTypes)iI), false);
				}
				else if (isAtWar((TeamTypes)iI))
				{
					GET_TEAM(eTeam).declareWar(((TeamTypes)iI), false);
				}
			}
		}
	}

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		if ((iI != getID()) && (iI != eTeam))
		{
			if (GET_TEAM((TeamTypes)iI).isAlive())
			{
				if (GET_TEAM(eTeam).isPermanentWarPeace((TeamTypes)iI))
				{
					setPermanentWarPeace(((TeamTypes)iI), true);
				}
				else if (isPermanentWarPeace((TeamTypes)iI))
				{
					GET_TEAM(eTeam).setPermanentWarPeace(((TeamTypes)iI), true);
				}
			}
		}
	}

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		if ((iI != getID()) && (iI != eTeam))
		{
			if (GET_TEAM((TeamTypes)iI).isAlive())
			{
				if (GET_TEAM(eTeam).isOpenBorders((TeamTypes)iI))
				{
					setOpenBorders(((TeamTypes)iI), true);
					GET_TEAM((TeamTypes)iI).setOpenBorders(getID(), true);
				}
				else if (isOpenBorders((TeamTypes)iI))
				{
					GET_TEAM(eTeam).setOpenBorders(((TeamTypes)iI), true);
					GET_TEAM((TeamTypes)iI).setOpenBorders(eTeam, true);
				}
			}
		}
	}

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		if ((iI != getID()) && (iI != eTeam))
		{
			if (GET_TEAM((TeamTypes)iI).isAlive())
			{
				if (GET_TEAM(eTeam).isDefensivePact((TeamTypes)iI))
				{
					setDefensivePact(((TeamTypes)iI), true);
					GET_TEAM((TeamTypes)iI).setDefensivePact(getID(), true);
				}
				else if (isDefensivePact((TeamTypes)iI))
				{
					GET_TEAM(eTeam).setDefensivePact(((TeamTypes)iI), true);
					GET_TEAM((TeamTypes)iI).setDefensivePact(eTeam, true);
				}
			}
		}
	}

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		if ((iI != getID()) && (iI != eTeam))
		{
			if (GET_TEAM((TeamTypes)iI).isAlive())
			{
				if (GET_TEAM(eTeam).isForcePeace((TeamTypes)iI))
				{
					setForcePeace(((TeamTypes)iI), true);
					GET_TEAM((TeamTypes)iI).setForcePeace(getID(), true);
				}
				else if (isForcePeace((TeamTypes)iI))
				{
					GET_TEAM(eTeam).setForcePeace(((TeamTypes)iI), true);
					GET_TEAM((TeamTypes)iI).setForcePeace(eTeam, true);
				}
			}
		}
	}

	shareCounters(eTeam);
	GET_TEAM(eTeam).shareCounters(getID());

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).getTeam() == eTeam)
		{
			GET_PLAYER((PlayerTypes)iI).setTeam(getID());
		}
	}

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		pLoopPlot->changeVisibilityCount(getID(), pLoopPlot->getVisibilityCount(eTeam), NO_INVISIBLE);

		for (iJ = 0; iJ < GC.getNumInvisibleInfos(); iJ++)
		{
			pLoopPlot->changeInvisibleVisibilityCount(getID(), ((InvisibleTypes)iJ), pLoopPlot->getInvisibleVisibilityCount(eTeam, ((InvisibleTypes)iJ)));
		}

		if (pLoopPlot->isRevealed(eTeam, false))
		{
			pLoopPlot->setRevealed(getID(), true, false, eTeam);
		}
	}

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		if ((iI != getID()) && (iI != eTeam))
		{
			GET_TEAM((TeamTypes)iI).setWarWeariness(getID(), ((GET_TEAM((TeamTypes)iI).getWarWeariness(getID()) + GET_TEAM((TeamTypes)iI).getWarWeariness(eTeam)) / 2));
			GET_TEAM((TeamTypes)iI).setStolenVisibilityTimer(getID(), ((GET_TEAM((TeamTypes)iI).getStolenVisibilityTimer(getID()) + GET_TEAM((TeamTypes)iI).getStolenVisibilityTimer(eTeam)) / 2));
			GET_TEAM((TeamTypes)iI).AI_setAtWarCounter(getID(), ((GET_TEAM((TeamTypes)iI).AI_getAtWarCounter(getID()) + GET_TEAM((TeamTypes)iI).AI_getAtWarCounter(eTeam)) / 2));
			GET_TEAM((TeamTypes)iI).AI_setAtPeaceCounter(getID(), ((GET_TEAM((TeamTypes)iI).AI_getAtPeaceCounter(getID()) + GET_TEAM((TeamTypes)iI).AI_getAtPeaceCounter(eTeam)) / 2));
			GET_TEAM((TeamTypes)iI).AI_setHasMetCounter(getID(), ((GET_TEAM((TeamTypes)iI).AI_getHasMetCounter(getID()) + GET_TEAM((TeamTypes)iI).AI_getHasMetCounter(eTeam)) / 2));
			GET_TEAM((TeamTypes)iI).AI_setDefensivePactCounter(getID(), ((GET_TEAM((TeamTypes)iI).AI_getDefensivePactCounter(getID()) + GET_TEAM((TeamTypes)iI).AI_getDefensivePactCounter(eTeam)) / 2));
			GET_TEAM((TeamTypes)iI).AI_setShareWarCounter(getID(), ((GET_TEAM((TeamTypes)iI).AI_getShareWarCounter(getID()) + GET_TEAM((TeamTypes)iI).AI_getShareWarCounter(eTeam)) / 2));
			GET_TEAM((TeamTypes)iI).AI_setWarSuccess(getID(), ((GET_TEAM((TeamTypes)iI).AI_getWarSuccess(getID()) + GET_TEAM((TeamTypes)iI).AI_getWarSuccess(eTeam)) / 2));
			GET_TEAM((TeamTypes)iI).AI_setEnemyPeacetimeTradeValue(getID(), ((GET_TEAM((TeamTypes)iI).AI_getEnemyPeacetimeTradeValue(getID()) + GET_TEAM((TeamTypes)iI).AI_getEnemyPeacetimeTradeValue(eTeam)) / 2));
			GET_TEAM((TeamTypes)iI).AI_setEnemyPeacetimeGrantValue(getID(), ((GET_TEAM((TeamTypes)iI).AI_getEnemyPeacetimeGrantValue(getID()) + GET_TEAM((TeamTypes)iI).AI_getEnemyPeacetimeGrantValue(eTeam)) / 2));

			if (GET_TEAM((TeamTypes)iI).isAlive())
			{
				GET_TEAM((TeamTypes)iI).AI_setWarPlan(getID(), NO_WARPLAN);
				GET_TEAM((TeamTypes)iI).AI_setWarPlan(eTeam, NO_WARPLAN);
			}
		}
	}

	AI_updateWorstEnemy();

	AI_updateAreaStragies();

	GC.getGameINLINE().updateScore(true);
}


void CvTeam::shareItems(TeamTypes eTeam)
{
	CvCity* pLoopCity;
	int iLoop;
	int iI, iJ, iK;

	FAssert(eTeam != NO_TEAM);
	FAssert(eTeam != getID());

	for (iI = 0; iI < GC.getNumTechInfos(); iI++)
	{
		if (GET_TEAM(eTeam).isHasTech((TechTypes)iI))
		{
			setHasTech(((TechTypes)iI), true, NO_PLAYER, true, false);
		}
	}

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() == eTeam)
			{
				for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
				{
					for (iJ = 0; iJ < GC.getNumBuildingInfos(); iJ++)
					{
						if (pLoopCity->hasBuilding((BuildingTypes)iJ))
						{
							if (GC.getBuildingInfo((BuildingTypes)iJ).isTeamShare())
							{
								for (iK = 0; iK < MAX_PLAYERS; iK++)
								{
									if (GET_PLAYER((PlayerTypes)iK).getTeam() == getID())
									{
										GET_PLAYER((PlayerTypes)iK).processBuilding(((BuildingTypes)iJ), 1, pLoopCity->area());
									}
								}
							}

							processBuilding(((BuildingTypes)iJ), 1);
						}
					}
				}
			}
		}
	}
}


void CvTeam::shareCounters(TeamTypes eTeam)
{
	int iI;

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		if ((iI != getID()) && (iI != eTeam))
		{
			if (GET_TEAM(eTeam).getWarWeariness((TeamTypes)iI) > getWarWeariness((TeamTypes)iI))
			{
				setWarWeariness(((TeamTypes)iI), GET_TEAM(eTeam).getWarWeariness((TeamTypes)iI));
			}

			if (GET_TEAM(eTeam).getStolenVisibilityTimer((TeamTypes)iI) > getStolenVisibilityTimer((TeamTypes)iI))
			{
				setStolenVisibilityTimer(((TeamTypes)iI), GET_TEAM(eTeam).getStolenVisibilityTimer((TeamTypes)iI));
			}

			if (GET_TEAM(eTeam).AI_getAtWarCounter((TeamTypes)iI) > AI_getAtWarCounter((TeamTypes)iI))
			{
				AI_setAtWarCounter(((TeamTypes)iI), GET_TEAM(eTeam).AI_getAtWarCounter((TeamTypes)iI));
			}

			if (GET_TEAM(eTeam).AI_getAtPeaceCounter((TeamTypes)iI) > AI_getAtPeaceCounter((TeamTypes)iI))
			{
				AI_setAtPeaceCounter(((TeamTypes)iI), GET_TEAM(eTeam).AI_getAtPeaceCounter((TeamTypes)iI));
			}

			if (GET_TEAM(eTeam).AI_getHasMetCounter((TeamTypes)iI) > AI_getHasMetCounter((TeamTypes)iI))
			{
				AI_setHasMetCounter(((TeamTypes)iI), GET_TEAM(eTeam).AI_getHasMetCounter((TeamTypes)iI));
			}

			if (GET_TEAM(eTeam).AI_getOpenBordersCounter((TeamTypes)iI) > AI_getOpenBordersCounter((TeamTypes)iI))
			{
				AI_setOpenBordersCounter(((TeamTypes)iI), GET_TEAM(eTeam).AI_getOpenBordersCounter((TeamTypes)iI));
			}

			if (GET_TEAM(eTeam).AI_getDefensivePactCounter((TeamTypes)iI) > AI_getDefensivePactCounter((TeamTypes)iI))
			{
				AI_setDefensivePactCounter(((TeamTypes)iI), GET_TEAM(eTeam).AI_getDefensivePactCounter((TeamTypes)iI));
			}

			if (GET_TEAM(eTeam).AI_getShareWarCounter((TeamTypes)iI) > AI_getShareWarCounter((TeamTypes)iI))
			{
				AI_setShareWarCounter(((TeamTypes)iI), GET_TEAM(eTeam).AI_getShareWarCounter((TeamTypes)iI));
			}

			if (GET_TEAM(eTeam).AI_getWarSuccess((TeamTypes)iI) > AI_getWarSuccess((TeamTypes)iI))
			{
				AI_setWarSuccess(((TeamTypes)iI), GET_TEAM(eTeam).AI_getWarSuccess((TeamTypes)iI));
			}

			if (GET_TEAM(eTeam).AI_getEnemyPeacetimeTradeValue((TeamTypes)iI) > AI_getEnemyPeacetimeTradeValue((TeamTypes)iI))
			{
				AI_setEnemyPeacetimeTradeValue(((TeamTypes)iI), GET_TEAM(eTeam).AI_getEnemyPeacetimeTradeValue((TeamTypes)iI));
			}

			if (GET_TEAM(eTeam).AI_getEnemyPeacetimeGrantValue((TeamTypes)iI) > AI_getEnemyPeacetimeGrantValue((TeamTypes)iI))
			{
				AI_setEnemyPeacetimeGrantValue(((TeamTypes)iI), GET_TEAM(eTeam).AI_getEnemyPeacetimeGrantValue((TeamTypes)iI));
			}

			GET_TEAM(eTeam).AI_setWarPlan(((TeamTypes)iI), NO_WARPLAN);
		}
	}

	for (iI = 0; iI < GC.getNumProjectInfos(); iI++)
	{
		if (GET_TEAM(eTeam).getProjectCount((ProjectTypes)iI) > getProjectCount((ProjectTypes)iI))
		{
			changeProjectCount(((ProjectTypes)iI), (GET_TEAM(eTeam).getProjectCount((ProjectTypes)iI) - getProjectCount((ProjectTypes)iI)));
		}

		changeProjectMaking(((ProjectTypes)iI), GET_TEAM(eTeam).getProjectMaking((ProjectTypes)iI));
	}

	for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
	{
		changeUnitClassCount(((UnitClassTypes)iI), GET_TEAM(eTeam).getUnitClassCount((UnitClassTypes)iI));
	}

	for (iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
	{
		changeBuildingClassCount(((BuildingClassTypes)iI), GET_TEAM(eTeam).getBuildingClassCount((BuildingClassTypes)iI));
	}

	for (iI = 0; iI < GC.getNumTechInfos(); iI++)
	{
		if (GET_TEAM(eTeam).getResearchProgress((TechTypes)iI) > getResearchProgress((TechTypes)iI))
		{
			setResearchProgress(((TechTypes)iI), GET_TEAM(eTeam).getResearchProgress((TechTypes)iI), getLeaderID());
		}

		if (GET_TEAM(eTeam).isNoTradeTech((TechTypes)iI))
		{
			setNoTradeTech(((TechTypes)iI), true);
		}
	}
}


void CvTeam::processBuilding(BuildingTypes eBuilding, int iChange)
{
	changeForceTeamVoteEligibilityCount((GC.getBuildingInfo(eBuilding).isForceTeamVoteEligible()) ? iChange : 0);

	if (GC.getBuildingInfo(eBuilding).isMapCentering())
	{
		if (iChange > 0)
		{
			setMapCentering(true);
		}
	}
}


void CvTeam::doTurn()
{
	PROFILE("CvTeam::doTurn()")

	int iCount;
	int iPossibleCount;
	int iI, iJ;

	FAssertMsg(isAlive(), "isAlive is expected to be true");

	AI_doTurnPre();

	if (isBarbarian())
	{
		for (iI = 0; iI < GC.getNumTechInfos(); iI++)
		{
			if (!isHasTech((TechTypes)iI))
			{
				iCount = 0;
				iPossibleCount = 0;

				for (iJ = 0; iJ < MAX_CIV_TEAMS; iJ++)
				{
					if (GET_TEAM((TeamTypes)iJ).isAlive())
					{
						if (GET_TEAM((TeamTypes)iJ).isHasTech((TechTypes)iI))
						{
							iCount++;
						}

						iPossibleCount++;
					}
				}

				if (iCount > 0)
				{
					FAssertMsg(iPossibleCount > 0, "iPossibleCount is expected to be greater than 0");

					changeResearchProgress(((TechTypes)iI), ((getResearchCost((TechTypes)iI) * ((GC.getDefineINT("BARBARIAN_FREE_TECH_PERCENT") * iCount) / iPossibleCount)) / 100), getLeaderID());
				}
			}
		}
	}

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		if (GET_TEAM((TeamTypes)iI).isAlive())
		{
			if (getStolenVisibilityTimer((TeamTypes)iI) > 0)
			{
				changeStolenVisibilityTimer(((TeamTypes)iI), -1);
			}
		}
	}

	for (iI = 0; iI < GC.getNumTechInfos(); iI++)
	{
		setNoTradeTech(((TechTypes)iI), false);
	}

	doWarWeariness();

	testCircumnavigated();

	AI_doTurnPost();
}


void CvTeam::updateYield()
{
	int iI;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
			{
				GET_PLAYER((PlayerTypes)iI).updateYield();
			}
		}
	}
}


void CvTeam::updatePowerHealth()
{
	int iI;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
			{
				GET_PLAYER((PlayerTypes)iI).updatePowerHealth();
			}
		}
	}
}


void CvTeam::updateCommerce()
{
	int iI;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
			{
				GET_PLAYER((PlayerTypes)iI).updateCommerce();
			}
		}
	}
}


bool CvTeam::canChangeWarPeace(TeamTypes eTeam)
{
	if (GC.getGameINLINE().isOption(GAMEOPTION_NO_CHANGING_WAR_PEACE))
	{
		return false;
	}

	if (isPermanentWarPeace(eTeam) || GET_TEAM(eTeam).isPermanentWarPeace(getID()))
	{
		return false;
	}

	return true;
}


bool CvTeam::canDeclareWar(TeamTypes eTeam)
{
	if (eTeam == getID())
	{
		return false;
	}

	if (!(isAlive()) || !(GET_TEAM(eTeam).isAlive()))
	{
		return false;
	}

	if (isAtWar(eTeam))
	{
		return false;
	}

	if (!isHasMet(eTeam))
	{
		return false;
	}

	if (isForcePeace(eTeam))
	{
		return false;
	}

	if (!canChangeWarPeace(eTeam))
	{
		return false;
	}

	if (GC.getGameINLINE().isOption(GAMEOPTION_ALWAYS_PEACE))
	{
		return false;
	}

	return true;
}


void CvTeam::declareWar(TeamTypes eTeam, bool bNewDiplo)
{
	PROFILE_FUNC();

	CLLNode<TradeData>* pNode;
	CvDiploParameters* pDiplo;
	CvDeal* pLoopDeal;
	CvWString szBuffer;
	bool bCancelDeal;
	int iLoop;
	int iI, iJ;

	FAssertMsg(eTeam != NO_TEAM, "eTeam is not assigned a valid value");
	FAssertMsg(eTeam != getID(), "eTeam is not expected to be equal with getID()");

	if (!isAtWar(eTeam))
	{
		FAssertMsg((isHuman() || isMinorCiv() || GET_TEAM(eTeam).isMinorCiv() || isBarbarian() || GET_TEAM(eTeam).isBarbarian() || AI_isSneakAttackReady(eTeam) || (GET_TEAM(eTeam).getAtWarCount(true) > 0) || !(GC.getGameINLINE().isFinalInitialized()) || gDLL->GetWorldBuilderMode()), "Possible accidently AI war!!!");

		for (pLoopDeal = GC.getGameINLINE().firstDeal(&iLoop); pLoopDeal != NULL; pLoopDeal = GC.getGameINLINE().nextDeal(&iLoop))
		{
			if (((GET_PLAYER(pLoopDeal->getFirstPlayer()).getTeam() == getID()) && (GET_PLAYER(pLoopDeal->getSecondPlayer()).getTeam() == eTeam)) ||
					((GET_PLAYER(pLoopDeal->getFirstPlayer()).getTeam() == eTeam) && (GET_PLAYER(pLoopDeal->getSecondPlayer()).getTeam() == getID())))
			{
				pLoopDeal->kill();
			}
		}

		FAssertMsg(eTeam != getID(), "eTeam is not expected to be equal with getID()");
		setAtWar(eTeam, true);
		GET_TEAM(eTeam).setAtWar(getID(), true);

		meet(eTeam, false);

		AI_setAtPeaceCounter(eTeam, 0);
		GET_TEAM(eTeam).AI_setAtPeaceCounter(getID(), 0);

		AI_setShareWarCounter(eTeam, 0);
		GET_TEAM(eTeam).AI_setShareWarCounter(getID(), 0);

		FAssert(!(AI_isSneakAttackPreparing(eTeam)));
		if ((AI_getWarPlan(eTeam) == NO_WARPLAN) || AI_isSneakAttackPreparing(eTeam))
		{
			if (isHuman())
			{
				AI_setWarPlan(eTeam, WARPLAN_TOTAL);
			}
			else if (isMinorCiv() || isBarbarian() || (GET_TEAM(eTeam).getAtWarCount(true) == 1))
			{
				AI_setWarPlan(eTeam, WARPLAN_LIMITED);
			}
			else
			{
				AI_setWarPlan(eTeam, WARPLAN_DOGPILE);
			}
		}
		GET_TEAM(eTeam).AI_setWarPlan(getID(), ((isBarbarian() || isMinorCiv()) ? WARPLAN_ATTACKED : WARPLAN_ATTACKED_RECENT));

		for (iI = 0; iI < MAX_TEAMS; iI++)
		{
			if (GET_TEAM((TeamTypes)iI).isAlive())
			{
				if (GET_TEAM(eTeam).AI_isSneakAttackPreparing((TeamTypes)iI))
				{
					GET_TEAM(eTeam).AI_setWarPlan(((TeamTypes)iI), NO_WARPLAN);
				}
			}
		}

		GC.getMapINLINE().verifyUnitValidPlot();

		GC.getGameINLINE().AI_makeAssignWorkDirty();

		if ((getID() == GC.getGameINLINE().getActiveTeam()) || (eTeam == GC.getGameINLINE().getActiveTeam()))
		{
			gDLL->getInterfaceIFace()->setDirty(Score_DIRTY_BIT, true);
		}

		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				for (iJ = 0; iJ < MAX_PLAYERS; iJ++)
				{
					if (GET_PLAYER((PlayerTypes)iJ).isAlive())
					{
						if ((GET_PLAYER((PlayerTypes)iI).getTeam() == getID()) && (GET_PLAYER((PlayerTypes)iJ).getTeam() == eTeam))
						{
							GET_PLAYER((PlayerTypes)iI).AI_setFirstContact(((PlayerTypes)iJ), true);
							GET_PLAYER((PlayerTypes)iJ).AI_setFirstContact(((PlayerTypes)iI), true);
						}
					}
				}
			}
		}

		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				for (iJ = 0; iJ < MAX_PLAYERS; iJ++)
				{
					if (GET_PLAYER((PlayerTypes)iJ).isAlive())
					{
						if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
						{
							if (GET_PLAYER((PlayerTypes)iJ).getTeam() == eTeam)
							{
								GET_PLAYER((PlayerTypes)iJ).AI_changeMemoryCount(((PlayerTypes)iI), MEMORY_DECLARED_WAR, 1);
							}
							else if (GET_PLAYER((PlayerTypes)iJ).getTeam() != getID())
							{
								if (GET_TEAM(GET_PLAYER((PlayerTypes)iJ).getTeam()).isHasMet(eTeam))
								{
									if (GET_TEAM(GET_PLAYER((PlayerTypes)iJ).getTeam()).AI_getAttitude(eTeam) >= ATTITUDE_PLEASED)
									{
										GET_PLAYER((PlayerTypes)iJ).AI_changeMemoryCount(((PlayerTypes)iI), MEMORY_DECLARED_WAR_ON_FRIEND, 1);
									}
								}
							}
						}
					}
				}
			}
		}

		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if ((GET_PLAYER((PlayerTypes)iI).getTeam() == getID()) || (GET_PLAYER((PlayerTypes)iI).getTeam() == eTeam))
				{
					GET_PLAYER((PlayerTypes)iI).updateWarWearinessPercentAnger();
				}
			}
		}

		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if ((GET_PLAYER((PlayerTypes)iI).getTeam() == getID()) || (GET_PLAYER((PlayerTypes)iI).getTeam() == eTeam))
				{
					GET_PLAYER((PlayerTypes)iI).updatePlotGroups();
				}
			}
		}

		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if ((GET_PLAYER((PlayerTypes)iI).getTeam() == getID()) || (GET_PLAYER((PlayerTypes)iI).getTeam() == eTeam))
				{
					GET_PLAYER((PlayerTypes)iI).updateTradeRoutes();
				}
			}
		}

		if (GC.getGameINLINE().isFinalInitialized() && !(gDLL->GetWorldBuilderMode()))
		{
			if (bNewDiplo)
			{
				if (!isHuman())
				{
					for (iI = 0; iI < MAX_PLAYERS; iI++)
					{
						if (GET_PLAYER((PlayerTypes)iI).isAlive())
						{
							if (GET_PLAYER((PlayerTypes)iI).getTeam() == eTeam)
							{
								if (GET_PLAYER(getLeaderID()).canContact((PlayerTypes)iI))
								{
									if (GET_PLAYER((PlayerTypes)iI).isHuman())
									{
										pDiplo = new CvDiploParameters(getLeaderID());
										FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
										pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_DECLARE_WAR"));
										pDiplo->setAIContact(true);
										gDLL->beginDiplomacy(pDiplo, ((PlayerTypes)iI));
									}
								}
							}
						}
					}
				}
			}

			if (!isBarbarian() && !(GET_TEAM(eTeam).isBarbarian()) &&
				  !isMinorCiv() && !(GET_TEAM(eTeam).isMinorCiv()))
			{
				for (iI = 0; iI < MAX_PLAYERS; iI++)
				{
					if (GET_PLAYER((PlayerTypes)iI).isAlive())
					{
						if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
						{
							szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_DECLARED_WAR_ON", GET_TEAM(eTeam).getName().GetCString());
							gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_DECLAREWAR", MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_WARNING_TEXT"));
						}
						else if (GET_PLAYER((PlayerTypes)iI).getTeam() == eTeam)
						{
							szBuffer = gDLL->getText("TXT_KEY_MISC_DECLARED_WAR_ON_YOU", getName().GetCString());
							gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_DECLAREWAR", MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_WARNING_TEXT"));
						}
						else if (GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).isHasMet(getID()) && GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).isHasMet(eTeam))
						{
							szBuffer = gDLL->getText("TXT_KEY_MISC_SOMEONE_DECLARED_WAR", getName().GetCString(), GET_TEAM(eTeam).getName().GetCString());
							gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_THEIRDECLAREWAR", MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_WARNING_TEXT"));
						}
					}
				}

				szBuffer = gDLL->getText("TXT_KEY_MISC_SOMEONE_DECLARES_WAR", getName().GetCString(), GET_TEAM(eTeam).getName().GetCString());
				GC.getGameINLINE().addReplayMessage(REPLAY_MESSAGE_MAJOR_EVENT, getLeaderID(), szBuffer, -1, -1, (ColorTypes)GC.getInfoTypeForString("COLOR_WARNING_TEXT"));
			}
		}

		if (!(GET_TEAM(eTeam).isMinorCiv()))
		{
			for (iI = 0; iI < MAX_PLAYERS; iI++)
			{
				if (GET_PLAYER((PlayerTypes)iI).isAlive())
				{
					if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
					{
						for (pLoopDeal = GC.getGameINLINE().firstDeal(&iLoop); pLoopDeal != NULL; pLoopDeal = GC.getGameINLINE().nextDeal(&iLoop))
						{
							bCancelDeal = false;

							if ((pLoopDeal->getFirstPlayer() == ((PlayerTypes)iI)) || (pLoopDeal->getSecondPlayer() == ((PlayerTypes)iI)))
							{
								for (pNode = pLoopDeal->headFirstTradesNode(); (pNode != NULL); pNode = pLoopDeal->nextFirstTradesNode(pNode))
								{
									if (pNode->m_data.m_eItemType == TRADE_DEFENSIVE_PACT)
									{
										bCancelDeal = true;
									}
								}

								for (pNode = pLoopDeal->headSecondTradesNode(); (pNode != NULL); pNode = pLoopDeal->nextSecondTradesNode(pNode))
								{
									if (pNode->m_data.m_eItemType == TRADE_DEFENSIVE_PACT)
									{
										bCancelDeal = true;
									}
								}
							}

							if (bCancelDeal)
							{
								pLoopDeal->kill();
							}
						}
					}
				}
			}
		}

		cancelDefensivePacts();

		for (iI = 0; iI < MAX_TEAMS; iI++)
		{
			if (GET_TEAM((TeamTypes)iI).isAlive())
			{
				if (GET_TEAM((TeamTypes)iI).isDefensivePact(eTeam))
				{
					GET_TEAM((TeamTypes)iI).declareWar(getID(), bNewDiplo);
				}
			}
		}

		GET_TEAM(eTeam).cancelDefensivePacts();
	}
}

void CvTeam::makePeace(TeamTypes eTeam)
{
	CvWString szBuffer;
	int iI;

	FAssertMsg(eTeam != NO_TEAM, "eTeam is not assigned a valid value");
	FAssertMsg(eTeam != getID(), "eTeam is not expected to be equal with getID()");

	if (isAtWar(eTeam))
	{
		FAssertMsg(eTeam != getID(), "eTeam is not expected to be equal with getID()");
		setAtWar(eTeam, false);
		GET_TEAM(eTeam).setAtWar(getID(), false);

		AI_setAtWarCounter(eTeam, 0);
		GET_TEAM(eTeam).AI_setAtWarCounter(getID(), 0);

		AI_setWarSuccess(eTeam, 0);
		GET_TEAM(eTeam).AI_setWarSuccess(getID(), 0);

		AI_setWarPlan(eTeam, NO_WARPLAN);
		GET_TEAM(eTeam).AI_setWarPlan(getID(), NO_WARPLAN);

		GC.getMapINLINE().verifyUnitValidPlot();

		GC.getGameINLINE().AI_makeAssignWorkDirty();

		if ((getID() == GC.getGameINLINE().getActiveTeam()) || (eTeam == GC.getGameINLINE().getActiveTeam()))
		{
			gDLL->getInterfaceIFace()->setDirty(Score_DIRTY_BIT, true);
		}

		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if ((GET_PLAYER((PlayerTypes)iI).getTeam() == getID()) || (GET_PLAYER((PlayerTypes)iI).getTeam() == eTeam))
				{
					GET_PLAYER((PlayerTypes)iI).updateWarWearinessPercentAnger();
				}
			}
		}

		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if ((GET_PLAYER((PlayerTypes)iI).getTeam() == getID()) || (GET_PLAYER((PlayerTypes)iI).getTeam() == eTeam))
				{
					GET_PLAYER((PlayerTypes)iI).updatePlotGroups();
				}
			}
		}

		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if ((GET_PLAYER((PlayerTypes)iI).getTeam() == getID()) || (GET_PLAYER((PlayerTypes)iI).getTeam() == eTeam))
				{
					GET_PLAYER((PlayerTypes)iI).updateTradeRoutes();
				}
			}
		}

		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
				{
					szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_MADE_PEACE_WITH", GET_TEAM(eTeam).getName().GetCString());
					gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_MAKEPEACE", MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"));
				}
				else if (GET_PLAYER((PlayerTypes)iI).getTeam() == eTeam)
				{
					szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_MADE_PEACE_WITH", getName().GetCString());
					gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_MAKEPEACE", MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"));
				}
				else if (GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).isHasMet(getID()) && GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).isHasMet(eTeam))
				{
					szBuffer = gDLL->getText("TXT_KEY_MISC_SOMEONE_MADE_PEACE", getName().GetCString(), GET_TEAM(eTeam).getName().GetCString());
					gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_THEIRMAKEPEACE", MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"));
				}
			}
		}

		szBuffer = gDLL->getText("TXT_KEY_MISC_SOMEONE_MADE_PEACE", getName().GetCString(), GET_TEAM(eTeam).getName().GetCString());
		GC.getGameINLINE().addReplayMessage(REPLAY_MESSAGE_MAJOR_EVENT, getLeaderID(), szBuffer, -1, -1, (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"));
	}
}


bool CvTeam::canContact(TeamTypes eTeam)
{
	int iI, iJ;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
			{
				for (iJ = 0; iJ < MAX_PLAYERS; iJ++)
				{
					if (GET_PLAYER((PlayerTypes)iJ).isAlive())
					{
						if (GET_PLAYER((PlayerTypes)iJ).getTeam() == eTeam)
						{
							if (GET_PLAYER((PlayerTypes)iI).canContact((PlayerTypes)iJ))
							{
								return true;
							}
						}
					}
				}
			}
		}
	}

	return false;
}


void CvTeam::meet(TeamTypes eTeam, bool bNewDiplo)
{
	if (!isHasMet(eTeam))
	{
		makeHasMet(eTeam, bNewDiplo);
		GET_TEAM(eTeam).makeHasMet(getID(), bNewDiplo);
	}
}


void CvTeam::signOpenBorders(TeamTypes eTeam)
{
	CLinkList<TradeData> ourList;
	CLinkList<TradeData> theirList;
	TradeData item;

	FAssert(eTeam != NO_TEAM);
	FAssert(eTeam != getID());

	if (!isAtWar(eTeam) && (getID() != eTeam))
	{
		setTradeItem(&item, TRADE_OPEN_BORDERS);

		if (GET_PLAYER(getLeaderID()).canTradeItem(GET_TEAM(eTeam).getLeaderID(), item) && GET_PLAYER(GET_TEAM(eTeam).getLeaderID()).canTradeItem(getLeaderID(), item))
		{
			ourList.clear();
			theirList.clear();

			ourList.insertAtEnd(item);
			theirList.insertAtEnd(item);

			GC.getGameINLINE().implementDeal(getLeaderID(), (GET_TEAM(eTeam).getLeaderID()), &ourList, &theirList);
		}
	}
}


void CvTeam::signDefensivePact(TeamTypes eTeam)
{
	CLinkList<TradeData> ourList;
	CLinkList<TradeData> theirList;
	TradeData item;

	FAssert(eTeam != NO_TEAM);
	FAssert(eTeam != getID());

	if (!isAtWar(eTeam) && (getID() != eTeam))
	{
		setTradeItem(&item, TRADE_DEFENSIVE_PACT);

		if (GET_PLAYER(getLeaderID()).canTradeItem(GET_TEAM(eTeam).getLeaderID(), item) && GET_PLAYER(GET_TEAM(eTeam).getLeaderID()).canTradeItem(getLeaderID(), item))
		{
			ourList.clear();
			theirList.clear();

			ourList.insertAtEnd(item);
			theirList.insertAtEnd(item);

			GC.getGameINLINE().implementDeal(getLeaderID(), (GET_TEAM(eTeam).getLeaderID()), &ourList, &theirList);
		}
	}
}


int CvTeam::getAssets()
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
			{
				iCount += GET_PLAYER((PlayerTypes)iI).getAssets();
			}
		}
	}

	return iCount;
}


int CvTeam::getPower()
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
			{
				iCount += GET_PLAYER((PlayerTypes)iI).getPower();
			}
		}
	}

	return iCount;
}


int CvTeam::getDefensivePower()
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if ((GET_PLAYER((PlayerTypes)iI).getTeam() == getID()) || isDefensivePact(GET_PLAYER((PlayerTypes)iI).getTeam()))
			{
				iCount += GET_PLAYER((PlayerTypes)iI).getPower();
			}
		}
	}

	return iCount;
}


int CvTeam::getNumNukeUnits()
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
			{
				iCount += GET_PLAYER((PlayerTypes)iI).getNumNukeUnits();
			}
		}
	}

	return iCount;
}


int CvTeam::getAtWarCount(bool bIgnoreMinors)
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
	{
		if (GET_TEAM((TeamTypes)iI).isAlive())
		{
			if (!bIgnoreMinors || !(GET_TEAM((TeamTypes)iI).isMinorCiv()))
			{
				if (isAtWar((TeamTypes)iI))
				{
					FAssert(iI != getID());
					FAssert(!(AI_isSneakAttackPreparing((TeamTypes)iI)));
					iCount++;
				}
			}
		}
	}

	return iCount;
}


int CvTeam::getWarPlanCount(WarPlanTypes eWarPlan, bool bIgnoreMinors)
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
	{
		if (GET_TEAM((TeamTypes)iI).isAlive())
		{
			if (!bIgnoreMinors || !(GET_TEAM((TeamTypes)iI).isMinorCiv()))
			{
				if (AI_getWarPlan((TeamTypes)iI) == eWarPlan)
				{
					FAssert(iI != getID());
					iCount++;
				}
			}
		}
	}

	return iCount;
}


int CvTeam::getAnyWarPlanCount(bool bIgnoreMinors)
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
	{
		if (GET_TEAM((TeamTypes)iI).isAlive())
		{
			if (!bIgnoreMinors || !(GET_TEAM((TeamTypes)iI).isMinorCiv()))
			{
				if (AI_getWarPlan((TeamTypes)iI) != NO_WARPLAN)
				{
					FAssert(iI != getID());
					iCount++;
				}
			}
		}
	}

	FAssert(iCount >= getAtWarCount(bIgnoreMinors));

	return iCount;
}


int CvTeam::getChosenWarCount(bool bIgnoreMinors)
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
	{
		if (GET_TEAM((TeamTypes)iI).isAlive())
		{
			if (!bIgnoreMinors || !(GET_TEAM((TeamTypes)iI).isMinorCiv()))
			{
				if (AI_isChosenWar((TeamTypes)iI))
				{
					FAssert(iI != getID());
					iCount++;
				}
			}
		}
	}

	return iCount;
}


int CvTeam::getHasMetCivCount(bool bIgnoreMinors)
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
	{
		if (GET_TEAM((TeamTypes)iI).isAlive())
		{
			if (iI != getID())
			{
				if (!bIgnoreMinors || !(GET_TEAM((TeamTypes)iI).isMinorCiv()))
				{
					if (isHasMet((TeamTypes)iI))
					{
						FAssert(iI != getID());
						iCount++;
					}
				}
			}
		}
	}

	return iCount;
}


bool CvTeam::hasMetHuman()
{
	int iI;

	for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
	{
		if (GET_TEAM((TeamTypes)iI).isAlive())
		{
			if (iI != getID())
			{
				if (GET_TEAM((TeamTypes)iI).isHuman())
				{
					if (isHasMet((TeamTypes)iI))
					{
						FAssert(iI != getID());
						return true;
					}
				}
			}
		}
	}

	return false;
}


int CvTeam::getDefensivePactCount()
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
	{
		if (GET_TEAM((TeamTypes)iI).isAlive())
		{
			if (iI != getID())
			{
				if (isDefensivePact((TeamTypes)iI))
				{
					FAssert(iI != getID());
					iCount++;
				}
			}
		}
	}

	return iCount;
}


int CvTeam::getUnitClassMaking(UnitClassTypes eUnitClass)
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
			{
				iCount += GET_PLAYER((PlayerTypes)iI).getUnitClassMaking(eUnitClass);
			}
		}
	}

	return iCount;
}


int CvTeam::getUnitClassCountPlusMaking(UnitClassTypes eIndex)
{
	return (getUnitClassCount(eIndex) + getUnitClassMaking(eIndex));
}


int CvTeam::getBuildingClassMaking(BuildingClassTypes eBuildingClass)
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
			{
				iCount += GET_PLAYER((PlayerTypes)iI).getBuildingClassMaking(eBuildingClass);
			}
		}
	}

	return iCount;
}


int CvTeam::getBuildingClassCountPlusMaking(BuildingClassTypes eIndex)
{
	return (getBuildingClassCount(eIndex) + getBuildingClassMaking(eIndex));
}


int CvTeam::getHasReligionCount(ReligionTypes eReligion)
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
			{
				iCount += GET_PLAYER((PlayerTypes)iI).getHasReligionCount(eReligion);
			}
		}
	}

	return iCount;
}


int CvTeam::countTotalCulture()
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
			{
				iCount += GET_PLAYER((PlayerTypes)iI).countTotalCulture();
			}
		}
	}

	return iCount;
}


int CvTeam::countNumUnitsByArea(CvArea* pArea)
{
	PROFILE_FUNC();

	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
			{
				iCount += pArea->getUnitsPerPlayer((PlayerTypes)iI);
			}
		}
	}

	return iCount;
}


int CvTeam::countNumCitiesByArea(CvArea* pArea)
{
	PROFILE_FUNC();

	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
			{
				iCount += pArea->getCitiesPerPlayer((PlayerTypes)iI);
			}
		}
	}

	return iCount;
}


int CvTeam::countTotalPopulationByArea(CvArea* pArea)
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
			{
				iCount += pArea->getPopulationPerPlayer((PlayerTypes)iI);
			}
		}
	}

	return iCount;
}


int CvTeam::countPowerByArea(CvArea* pArea)
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
			{
				iCount += pArea->getPower((PlayerTypes)iI);
			}
		}
	}

	return iCount;
}


int CvTeam::countEnemyPowerByArea(CvArea* pArea)
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() != getID())
			{
				if (isAtWar(GET_PLAYER((PlayerTypes)iI).getTeam()))
				{
					iCount += pArea->getPower((PlayerTypes)iI);
				}
			}
		}
	}

	return iCount;
}


int CvTeam::countNumAIUnitsByArea(CvArea* pArea, UnitAITypes eUnitAI)
{
	PROFILE_FUNC();

	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
			{
				iCount += pArea->getNumAIUnits(((PlayerTypes)iI), eUnitAI);
			}
		}
	}

	return iCount;
}


int CvTeam::countEnemyDangerByArea(CvArea* pArea)
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (pLoopPlot != NULL)
		{
			if (pLoopPlot->area() == pArea)
			{
				if (pLoopPlot->getTeam() == getID())
				{
					iCount += pLoopPlot->plotCount(PUF_canDefendEnemy, getLeaderID(), -1, NO_PLAYER, NO_TEAM, PUF_isVisible, getLeaderID());
				}
			}
		}
	}

	return iCount;
}


int CvTeam::getResearchCost(TechTypes eTech)
{
	int iCost;

	FAssertMsg(eTech != NO_TECH, "Tech is not assigned a valid value");

	iCost = GC.getTechInfo(eTech).getResearchCost();

	iCost *= GC.getHandicapInfo(getHandicapType()).getResearchPercent();
	iCost /= 100;

	iCost *= GC.getWorldInfo(GC.getMapINLINE().getWorldSize()).getResearchPercent();
	iCost /= 100;

	iCost *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getResearchPercent();
	iCost /= 100;

	iCost *= GC.getEraInfo(GC.getGameINLINE().getStartEra()).getResearchPercent();
	iCost /= 100;

	iCost *= max(0, ((GC.getDefineINT("TECH_COST_EXTRA_TEAM_MEMBER_MODIFIER") * (getNumMembers() - 1)) + 100));
	iCost /= 100;

	return max(1, iCost);
}


int CvTeam::getResearchLeft(TechTypes eTech)
{
	return max(0, (getResearchCost(eTech) - getResearchProgress(eTech)));
}


bool CvTeam::hasHolyCity(ReligionTypes eReligion)
{
	CvCity* pHolyCity;

	FAssertMsg(eReligion != NO_RELIGION, "Religion is not assigned a valid value");

	pHolyCity = GC.getGameINLINE().getHolyCity(eReligion);

	if (pHolyCity != NULL)
	{
		return (pHolyCity->getTeam() == getID());
	}

	return false;
}


bool CvTeam::isHuman()
{
	PROFILE_FUNC();

	int iI;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
		{
			if (GET_PLAYER((PlayerTypes)iI).isHuman())
			{
				return true;
			}
		}
	}

	return false;
}


bool CvTeam::isBarbarian()
{
	return (getID() == BARBARIAN_TEAM);
}


bool CvTeam::isMinorCiv()
{
	bool bValid;
	int iI;

	bValid = false;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
		{
			if (GET_PLAYER((PlayerTypes)iI).isMinorCiv())
			{
				bValid = true;
			}
			else
			{
				return false;
			}
		}
	}

	return bValid;
}


PlayerTypes CvTeam::getLeaderID()
{
	int iI;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
			{
				return ((PlayerTypes)iI);
			}
		}
	}

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
		{
			return ((PlayerTypes)iI);
		}
	}

	return NO_PLAYER;
}


PlayerTypes CvTeam::getSecretaryID()
{
	int iI;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).isHuman())
			{
				if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
				{
					return ((PlayerTypes)iI);
				}
			}
		}
	}

	return getLeaderID();
}


HandicapTypes CvTeam::getHandicapType()
{
	int iGameHandicap;
	int iCount;
	int iI;

	iGameHandicap = 0;
	iCount = 0;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
			{
				iGameHandicap += GET_PLAYER((PlayerTypes)iI).getHandicapType();
				iCount++;
			}
		}
	}

	if (iCount > 0)
	{
		FAssertMsg((iGameHandicap / iCount) >= 0, "(iGameHandicap / iCount) is expected to be non-negative (invalid Index)");
		return ((HandicapTypes)(iGameHandicap / iCount));
	}
	else
	{
		return ((HandicapTypes)(GC.getDefineINT("STANDARD_HANDICAP")));
	}
}


CvWString CvTeam::getName()
{
	CvWString szBuffer;
	bool bFirst;
	int iI;

	bFirst = true;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
		{
			setListHelp(szBuffer, L"", GET_PLAYER((PlayerTypes)iI).getName(), L"/", bFirst);
			bFirst = false;
		}
	}

	return szBuffer;
}


int CvTeam::getNumMembers()
{
	return m_iNumMembers;
}


void CvTeam::changeNumMembers(int iChange)
{
	m_iNumMembers = (m_iNumMembers + iChange);
	FAssert(getNumMembers() >= 0);
}


int CvTeam::getAliveCount()
{
	return m_iAliveCount;
}


int CvTeam::isAlive()
{
	return (getAliveCount() > 0);
}


void CvTeam::changeAliveCount(int iChange)
{
	m_iAliveCount = (m_iAliveCount + iChange);
	FAssert(getAliveCount() >= 0);
}


int CvTeam::getEverAliveCount()
{
	return m_iEverAliveCount;
}


int CvTeam::isEverAlive()
{
	return (getEverAliveCount() > 0);
}


void CvTeam::changeEverAliveCount(int iChange)
{
	m_iEverAliveCount = (m_iEverAliveCount + iChange);
	FAssert(getEverAliveCount() >= 0);
}


int CvTeam::getNumCities()														
{
	return m_iNumCities;
}


void CvTeam::changeNumCities(int iChange)							
{
	m_iNumCities = (m_iNumCities + iChange);
	FAssert(getNumCities() >= 0);
}


int CvTeam::getTotalPopulation()											
{
	return m_iTotalPopulation;
}


void CvTeam::changeTotalPopulation(int iChange)	
{
	m_iTotalPopulation = (m_iTotalPopulation + iChange);
	FAssert(getTotalPopulation() >= 0);
}


int CvTeam::getTotalLand()
{
	return m_iTotalLand;
}


void CvTeam::changeTotalLand(int iChange)														
{
	m_iTotalLand = (m_iTotalLand + iChange);
	FAssert(getTotalLand() >= 0);
}


int CvTeam::getNukeInterception()
{
	return m_iNukeInterception;
}


void CvTeam::changeNukeInterception(int iChange)
{
	m_iNukeInterception = (m_iNukeInterception + iChange);
	FAssert(getNukeInterception() >= 0);
}


int CvTeam::getForceTeamVoteEligibilityCount()
{
	return m_iForceTeamVoteEligibilityCount;
}


bool CvTeam::isForceTeamVoteEligible()
{
	return ((getForceTeamVoteEligibilityCount() > 0) && !isMinorCiv());
}


void CvTeam::changeForceTeamVoteEligibilityCount(int iChange)
{
	m_iForceTeamVoteEligibilityCount = (m_iForceTeamVoteEligibilityCount + iChange);
	FAssert(getForceTeamVoteEligibilityCount() >= 0);
}


int CvTeam::getExtraWaterSeeFromCount()
{
	return m_iExtraWaterSeeFromCount;
}


bool CvTeam::isExtraWaterSeeFrom()
{
	return (getExtraWaterSeeFromCount() > 0);
}


void CvTeam::changeExtraWaterSeeFromCount(int iChange)
{
	if (iChange != 0)
	{
		GC.getMapINLINE().updateSight(false);

		m_iExtraWaterSeeFromCount = (m_iExtraWaterSeeFromCount + iChange);
		FAssert(getExtraWaterSeeFromCount() >= 0);

		GC.getMapINLINE().updateSight(true);
	}
}


int CvTeam::getMapTradingCount()
{
	return m_iMapTradingCount;
}


bool CvTeam::isMapTrading()														
{
	return (getMapTradingCount() > 0);
}


void CvTeam::changeMapTradingCount(int iChange)						 
{
	m_iMapTradingCount = (m_iMapTradingCount + iChange);
	FAssert(getMapTradingCount() >= 0);
}


int CvTeam::getTechTradingCount()
{
	return m_iTechTradingCount;
}


bool CvTeam::isTechTrading()													 
{
	return (getTechTradingCount() > 0);
}


void CvTeam::changeTechTradingCount(int iChange)						 
{
	m_iTechTradingCount = (m_iTechTradingCount + iChange);
	FAssert(getTechTradingCount() >= 0);
}


int CvTeam::getGoldTradingCount()
{
	return m_iGoldTradingCount;
}


bool CvTeam::isGoldTrading()													 
{
	return (getGoldTradingCount() > 0);
}


void CvTeam::changeGoldTradingCount(int iChange)						 
{
	m_iGoldTradingCount = (m_iGoldTradingCount + iChange);
	FAssert(getGoldTradingCount() >= 0);
}


int CvTeam::getOpenBordersTradingCount()
{
	return m_iOpenBordersTradingCount;
}


bool CvTeam::isOpenBordersTrading()
{
	return (getOpenBordersTradingCount() > 0);
}


void CvTeam::changeOpenBordersTradingCount(int iChange)
{
	m_iOpenBordersTradingCount = (m_iOpenBordersTradingCount + iChange);
	FAssert(getOpenBordersTradingCount() >= 0);
}


int CvTeam::getDefensivePactTradingCount()
{
	return m_iDefensivePactTradingCount;
}


bool CvTeam::isDefensivePactTrading()
{
	return (getDefensivePactTradingCount() > 0);
}


void CvTeam::changeDefensivePactTradingCount(int iChange)
{
	m_iDefensivePactTradingCount = (m_iDefensivePactTradingCount + iChange);
	FAssert(getDefensivePactTradingCount() >= 0);
}


int CvTeam::getPermanentAllianceTradingCount()
{
	return m_iPermanentAllianceTradingCount;
}


bool CvTeam::isPermanentAllianceTrading()
{
	if (!(GC.getGameINLINE().isOption(GAMEOPTION_PERMANENT_ALLIANCES)))
	{
		return false;
	}

	return (getPermanentAllianceTradingCount() > 0);
}


void CvTeam::changePermanentAllianceTradingCount(int iChange)
{
	m_iPermanentAllianceTradingCount = (m_iPermanentAllianceTradingCount + iChange);
	FAssert(getPermanentAllianceTradingCount() >= 0);
}


int CvTeam::getBridgeBuildingCount()
{
	return m_iBridgeBuildingCount;
}


bool CvTeam::isBridgeBuilding()												
{
	return (getBridgeBuildingCount() > 0);
}


void CvTeam::changeBridgeBuildingCount(int iChange)				 
{
	if (iChange != 0)
	{
		m_iBridgeBuildingCount = (m_iBridgeBuildingCount + iChange);
		FAssert(getBridgeBuildingCount() >= 0);

		if (GC.IsGraphicsInitialized())
		{
			gDLL->getEngineIFace()->MarkBridgesDirty();
		}
	}
}


int CvTeam::getIrrigationCount()
{
	return m_iIrrigationCount;
}


bool CvTeam::isIrrigation()
{
	return (getIrrigationCount() > 0);
}


void CvTeam::changeIrrigationCount(int iChange)
{
	if (iChange != 0)
	{
		m_iIrrigationCount = (m_iIrrigationCount + iChange);
		FAssert(getIrrigationCount() >= 0);

		GC.getMapINLINE().updateIrrigated();
	}
}


int CvTeam::getIgnoreIrrigationCount()
{
	return m_iIgnoreIrrigationCount;
}


bool CvTeam::isIgnoreIrrigation()
{
	return (getIgnoreIrrigationCount() > 0);
}


void CvTeam::changeIgnoreIrrigationCount(int iChange)
{
	m_iIgnoreIrrigationCount = (m_iIgnoreIrrigationCount + iChange);
	FAssert(getIgnoreIrrigationCount() >= 0);
}


int CvTeam::getWaterWorkCount()
{
	return m_iWaterWorkCount;
}


bool CvTeam::isWaterWork()
{
	return (getWaterWorkCount() > 0);
}


void CvTeam::changeWaterWorkCount(int iChange)
{
	if (iChange != 0)
	{
		m_iWaterWorkCount = (m_iWaterWorkCount + iChange);
		FAssert(getWaterWorkCount() >= 0);

		AI_makeAssignWorkDirty();
	}
}


bool CvTeam::isMapCentering()
{
	return m_bMapCentering;
}


void CvTeam::setMapCentering(bool bNewValue)
{
	if (isMapCentering() != bNewValue)
	{
		m_bMapCentering = bNewValue;

		if (getID() == GC.getGameINLINE().getActiveTeam())
		{ 
			gDLL->getInterfaceIFace()->setDirty(MinimapSection_DIRTY_BIT, true);
		}
	}
}


TeamTypes CvTeam::getID() const
{
	return m_eID;
}


int CvTeam::getStolenVisibilityTimer(TeamTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "iIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_TEAMS, "iIndex is expected to be within maximum bounds (invalid Index)");

	return m_aiStolenVisibilityTimer[eIndex];
}


bool CvTeam::isStolenVisibility(TeamTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "iIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_TEAMS, "iIndex is expected to be within maximum bounds (invalid Index)");

	return (getStolenVisibilityTimer(eIndex) > 0);
}


void CvTeam::setStolenVisibilityTimer(TeamTypes eIndex, int iNewValue)
{
	CvPlot* pLoopPlot;
	bool bOldStolenVisibility;
	int iI;

	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_TEAMS, "eIndex is expected to be within maximum bounds (invalid Index)");

	if (getStolenVisibilityTimer(eIndex) != iNewValue)
	{
		bOldStolenVisibility = isStolenVisibility(eIndex);

		m_aiStolenVisibilityTimer[eIndex] = iNewValue;
		FAssert(getStolenVisibilityTimer(eIndex) >= 0);

		if (bOldStolenVisibility != isStolenVisibility(eIndex))
		{
			for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
			{
				pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

				if (pLoopPlot->isVisible(eIndex, false))
				{
					pLoopPlot->changeStolenVisibilityCount(getID(), ((isStolenVisibility(eIndex)) ? 1 : -1));
				}
			}
		}
	}
}


void CvTeam::changeStolenVisibilityTimer(TeamTypes eIndex, int iChange)
{
	setStolenVisibilityTimer(eIndex, (getStolenVisibilityTimer(eIndex) + iChange));
}


int CvTeam::getWarWeariness(TeamTypes eIndex)								 
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_TEAMS, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiWarWeariness[eIndex];
}


void CvTeam::setWarWeariness(TeamTypes eIndex, int iNewValue)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_TEAMS, "eIndex is expected to be within maximum bounds (invalid Index)");
	m_aiWarWeariness[eIndex] = max(0, iNewValue);
}


void CvTeam::changeWarWeariness(TeamTypes eIndex, int iChange)
{
	setWarWeariness(eIndex, (getWarWeariness(eIndex) + iChange));
}


int CvTeam::getTechShareCount(int iIndex)
{
	FAssertMsg(iIndex >= 0, "iIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(iIndex < MAX_TEAMS, "iIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiTechShareCount[iIndex];
}


bool CvTeam::isTechShare(int iIndex)
{
	return (getTechShareCount(iIndex) > 0);
}


void CvTeam::changeTechShareCount(int iIndex, int iChange)
{
	FAssertMsg(iIndex >= 0, "iIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(iIndex < MAX_TEAMS, "iIndex is expected to be within maximum bounds (invalid Index)");

	if (iChange != 0)
	{
		m_aiTechShareCount[iIndex] = (m_aiTechShareCount[iIndex] + iChange);
		FAssert(getTechShareCount(iIndex) >= 0);

		if (isTechShare(iIndex))
		{
			updateTechShare();
		}
	}
}


int CvTeam::getCommerceFlexibleCount(CommerceTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_COMMERCE_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiCommerceFlexibleCount[eIndex];
}


bool CvTeam::isCommerceFlexible(CommerceTypes eIndex)
{
	return (getCommerceFlexibleCount(eIndex) > 0);
}


void CvTeam::changeCommerceFlexibleCount(CommerceTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_COMMERCE_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");

	if (iChange != 0)
	{
		m_aiCommerceFlexibleCount[eIndex] = (m_aiCommerceFlexibleCount[eIndex] + iChange);
		FAssert(getCommerceFlexibleCount(eIndex) >= 0);

		if (getID() == GC.getGameINLINE().getActiveTeam())
		{
			gDLL->getInterfaceIFace()->setDirty(PercentButtons_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(GameData_DIRTY_BIT, true);
		}
	}
}


int CvTeam::getExtraMoves(DomainTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_DOMAIN_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiExtraMoves[eIndex];
}


void CvTeam::changeExtraMoves(DomainTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_DOMAIN_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
	m_aiExtraMoves[eIndex] = (m_aiExtraMoves[eIndex] + iChange);
	FAssert(getExtraMoves(eIndex) >= 0);
}


bool CvTeam::isHasMet(TeamTypes eIndex)															 
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_TEAMS, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_abHasMet[eIndex];
	FAssert((eIndex != getID()) || isHasMet(eIndex));
}


void CvTeam::makeHasMet(TeamTypes eIndex, bool bNewDiplo)
{
	CvDiploParameters* pDiplo;
	int iI;

	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_TEAMS, "eIndex is expected to be within maximum bounds (invalid Index)");

	if (!isHasMet(eIndex))
	{
		m_abHasMet[eIndex] = true;

		updateTechShare();

		if (GET_TEAM(eIndex).isHuman())
		{
			for (iI = 0; iI < MAX_PLAYERS; iI++)
			{
				if (GET_PLAYER((PlayerTypes)iI).isAlive())
				{
					if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
					{
						if (!(GET_PLAYER((PlayerTypes)iI).isHuman()))
						{
							GET_PLAYER((PlayerTypes)iI).clearResearchQueue();
							GET_PLAYER((PlayerTypes)iI).AI_makeProductionDirty();
						}
					}
				}
			}
		}

		if ((getID() == GC.getGameINLINE().getActiveTeam()) || (eIndex == GC.getGameINLINE().getActiveTeam()))
		{
			gDLL->getInterfaceIFace()->setDirty(Score_DIRTY_BIT, true);
		}

		if (GC.getGameINLINE().isOption(GAMEOPTION_ALWAYS_WAR))
		{
			if (isHuman())
			{
				if (getID() != eIndex)
				{
					declareWar(eIndex, false);
				}
			}
		}
		else
		{
			if (GC.getGameINLINE().isFinalInitialized() && !(gDLL->GetWorldBuilderMode()))
			{
				if (bNewDiplo)
				{
					if (!isHuman())
					{
						if (!isAtWar(eIndex))
						{
							for (iI = 0; iI < MAX_PLAYERS; iI++)
							{
								if (GET_PLAYER((PlayerTypes)iI).isAlive())
								{
									if (GET_PLAYER((PlayerTypes)iI).getTeam() == eIndex)
									{
										if (GET_PLAYER(getLeaderID()).canContact((PlayerTypes)iI))
										{
											if (GET_PLAYER((PlayerTypes)iI).isHuman())
											{
												pDiplo = new CvDiploParameters(getLeaderID());
												FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
												pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_FIRST_CONTACT"));
												pDiplo->setAIContact(true);
												gDLL->beginDiplomacy(pDiplo, ((PlayerTypes)iI));
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

		// report event to Python, along with some other key state
		gDLL->getEventReporterIFace()->firstContact(getID(), eIndex);
	}
}


bool CvTeam::isAtWar(TeamTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_TEAMS, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_abAtWar[eIndex];
}


void CvTeam::setAtWar(TeamTypes eIndex, bool bNewValue)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_TEAMS, "eIndex is expected to be within maximum bounds (invalid Index)");
	m_abAtWar[eIndex] = bNewValue;
}


bool CvTeam::isPermanentWarPeace(TeamTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_TEAMS, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_abPermanentWarPeace[eIndex];
}


void CvTeam::setPermanentWarPeace(TeamTypes eIndex, bool bNewValue)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_TEAMS, "eIndex is expected to be within maximum bounds (invalid Index)");
	m_abPermanentWarPeace[eIndex] = bNewValue;
}


bool CvTeam::isFreeTrade(TeamTypes eIndex)
{
	if (isAtWar(eIndex))
	{
		return false;
	}

	if (!isHasMet(eIndex))
	{
		return false;
	}

	return (isOpenBorders(eIndex) || GC.getGameINLINE().isFreeTrade());
}


bool CvTeam::isOpenBorders(TeamTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_TEAMS, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_abOpenBorders[eIndex];
}


void CvTeam::setOpenBorders(TeamTypes eIndex, bool bNewValue)
{
	bool bOldFreeTrade;
	int iI;

	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_TEAMS, "eIndex is expected to be within maximum bounds (invalid Index)");

	if (isOpenBorders(eIndex) != bNewValue)
	{
		bOldFreeTrade = isFreeTrade(eIndex);

		m_abOpenBorders[eIndex] = bNewValue;

		AI_setOpenBordersCounter(eIndex, 0);

		GC.getMapINLINE().verifyUnitValidPlot();

		if ((getID() == GC.getGameINLINE().getActiveTeam()) || (eIndex == GC.getGameINLINE().getActiveTeam()))
		{
			gDLL->getInterfaceIFace()->setDirty(Score_DIRTY_BIT, true);
		}

		if (bOldFreeTrade != isFreeTrade(eIndex))
		{
			for (iI = 0; iI < MAX_PLAYERS; iI++)
			{
				if (GET_PLAYER((PlayerTypes)iI).isAlive())
				{
					if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
					{
						GET_PLAYER((PlayerTypes)iI).updateTradeRoutes();
					}
				}
			}
		}
	}
}


bool CvTeam::isDefensivePact(TeamTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_TEAMS, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_abDefensivePact[eIndex];
}


void CvTeam::setDefensivePact(TeamTypes eIndex, bool bNewValue)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_TEAMS, "eIndex is expected to be within maximum bounds (invalid Index)");

	if (isDefensivePact(eIndex) != bNewValue)
	{
		m_abDefensivePact[eIndex] = bNewValue;

		if ((getID() == GC.getGameINLINE().getActiveTeam()) || (eIndex == GC.getGameINLINE().getActiveTeam()))
		{
			gDLL->getInterfaceIFace()->setDirty(Score_DIRTY_BIT, true);
		}

		if (bNewValue && !GET_TEAM(eIndex).isDefensivePact(getID()))
		{
			CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_PLAYERS_SIGN_DEFENSIVE_PACT", getName().GetCString(), GET_TEAM(eIndex).getName().GetCString());
			GC.getGameINLINE().addReplayMessage(REPLAY_MESSAGE_MAJOR_EVENT, getLeaderID(), szBuffer, -1, -1, (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"));
		}
	}
}


bool CvTeam::isForcePeace(TeamTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_TEAMS, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_abForcePeace[eIndex];
}


void CvTeam::setForcePeace(TeamTypes eIndex, bool bNewValue)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_TEAMS, "eIndex is expected to be within maximum bounds (invalid Index)");
	m_abForcePeace[eIndex] = bNewValue;
}


int CvTeam::getRouteChange(RouteTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumRouteInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiRouteChange[eIndex];
}


void CvTeam::changeRouteChange(RouteTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumRouteInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_paiRouteChange[eIndex] = (m_paiRouteChange[eIndex] + iChange);
}


int CvTeam::getProjectCount(ProjectTypes eIndex)																	 
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumProjectInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiProjectCount[eIndex];
}


bool CvTeam::isProjectMaxedOut(ProjectTypes eIndex, int iExtra)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumProjectInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (!isTeamProject(eIndex))
	{
		return false;
	}

	FAssertMsg(getProjectCount(eIndex) <= GC.getProjectInfo(eIndex).getMaxTeamInstances(), "Current Project count is expected to not exceed the maximum number of instances for this project");

	return ((getProjectCount(eIndex) + iExtra) >= GC.getProjectInfo(eIndex).getMaxTeamInstances());
}


void CvTeam::changeProjectCount(ProjectTypes eIndex, int iChange)
{
	CvWString szBuffer;
	bool bChangeProduction;
	int iOldProjectCount;
	int iI, iJ;

	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumProjectInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (iChange != 0)
	{
		iOldProjectCount = getProjectCount(eIndex);

		m_paiProjectCount[eIndex] = (m_paiProjectCount[eIndex] + iChange);
		FAssert(getProjectCount(eIndex) >= 0);

		changeNukeInterception(GC.getProjectInfo(eIndex).getNukeInterception() * iChange);

		if ((GC.getProjectInfo(eIndex).getTechShare() > 0) && (GC.getProjectInfo(eIndex).getTechShare() <= MAX_TEAMS))
		{
			changeTechShareCount((GC.getProjectInfo(eIndex).getTechShare() - 1), iChange);
		}

		if (iChange > 0)
		{
			if (GC.getProjectInfo(eIndex).getEveryoneSpecialUnit() != NO_SPECIALUNIT)
			{
				GC.getGameINLINE().makeSpecialUnitValid((SpecialUnitTypes)(GC.getProjectInfo(eIndex).getEveryoneSpecialUnit()));
			}

			if (GC.getProjectInfo(eIndex).getEveryoneSpecialBuilding() != NO_SPECIALBUILDING)
			{
				GC.getGameINLINE().makeSpecialBuildingValid((SpecialBuildingTypes)(GC.getProjectInfo(eIndex).getEveryoneSpecialBuilding()));
			}

			for (iI = 0; iI < MAX_PLAYERS; iI++)
			{
				if (GET_PLAYER((PlayerTypes)iI).isAlive())
				{
					if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
					{
						if (!(GET_PLAYER((PlayerTypes)iI).isHuman()))
						{
							bChangeProduction = false;

							for (iJ = 0; iJ < GC.getNumProjectInfos(); iJ++)
							{
								if ((getProjectCount(eIndex) >= GC.getProjectInfo((ProjectTypes)iJ).getProjectsNeeded(eIndex)) &&
									  (iOldProjectCount < GC.getProjectInfo((ProjectTypes)iJ).getProjectsNeeded(eIndex)))
								{
									bChangeProduction = true;
									break;
								}
							}

							if (bChangeProduction)
							{
								GET_PLAYER((PlayerTypes)iI).AI_makeProductionDirty();
							}
						}
					}
				}
			}

			if (GC.getGameINLINE().isFinalInitialized() && !(gDLL->GetWorldBuilderMode()))
			{
				szBuffer = gDLL->getText("TXT_KEY_MISC_COMPLETES_PROJECT", getName().GetCString(), GC.getProjectInfo(eIndex).getTextKeyWide());
				GC.getGameINLINE().addReplayMessage(REPLAY_MESSAGE_MAJOR_EVENT, getLeaderID(), szBuffer, -1, -1, (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"));

				for (iI = 0; iI < MAX_PLAYERS; iI++)
				{
					if (GET_PLAYER((PlayerTypes)iI).isAlive())
					{
						szBuffer = gDLL->getText("TXT_KEY_MISC_SOMEONE_HAS_COMPLETED", getName().GetCString(), GC.getProjectInfo(eIndex).getTextKeyWide());
						gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_PROJECT_COMPLETED", MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"));
					}
				}
			}
		}
	}
}


int CvTeam::getProjectMaking(ProjectTypes eIndex)																	 
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumProjectInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiProjectMaking[eIndex];
}


void CvTeam::changeProjectMaking(ProjectTypes eIndex, int iChange)										
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumProjectInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_paiProjectMaking[eIndex] = (m_paiProjectMaking[eIndex] + iChange);
	FAssert(getProjectMaking(eIndex) >= 0);
}


int CvTeam::getUnitClassCount(UnitClassTypes eIndex)											
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumUnitClassInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiUnitClassCount[eIndex];
}


bool CvTeam::isUnitClassMaxedOut(UnitClassTypes eIndex, int iExtra)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumUnitClassInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (!isTeamUnitClass(eIndex))
	{
		return false;
	}

	FAssertMsg(getUnitClassCount(eIndex) <= GC.getUnitClassInfo(eIndex).getMaxTeamInstances(), "The current unit class count is expected not to exceed the maximum number of instances allowed for this team");

	return ((getUnitClassCount(eIndex) + iExtra) >= GC.getUnitClassInfo(eIndex).getMaxTeamInstances());
}


void CvTeam::changeUnitClassCount(UnitClassTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumUnitClassInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_paiUnitClassCount[eIndex] = (m_paiUnitClassCount[eIndex] + iChange);
	FAssert(getUnitClassCount(eIndex) >= 0);
}


int CvTeam::getBuildingClassCount(BuildingClassTypes eIndex)											
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumBuildingClassInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiBuildingClassCount[eIndex];
}


bool CvTeam::isBuildingClassMaxedOut(BuildingClassTypes eIndex, int iExtra)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumBuildingClassInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (!isTeamWonderClass(eIndex))
	{
		return false;
	}

	FAssertMsg(getBuildingClassCount(eIndex) <= GC.getBuildingClassInfo(eIndex).getMaxTeamInstances(), "The current building class count is expected not to exceed the maximum number of instances allowed for this team");

	return ((getBuildingClassCount(eIndex) + iExtra) >= GC.getBuildingClassInfo(eIndex).getMaxTeamInstances());
}


void CvTeam::changeBuildingClassCount(BuildingClassTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumBuildingClassInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_paiBuildingClassCount[eIndex] = (m_paiBuildingClassCount[eIndex] + iChange);
	FAssert(getBuildingClassCount(eIndex) >= 0);
}


int CvTeam::getObsoleteBuildingCount(BuildingTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumBuildingInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiObsoleteBuildingCount[eIndex];
}


bool CvTeam::isObsoleteBuilding(BuildingTypes eIndex)				
{
	return (getObsoleteBuildingCount(eIndex) > 0);
}


void CvTeam::changeObsoleteBuildingCount(BuildingTypes eIndex, int iChange)
{
	CvCity* pLoopCity;
	bool bOldObsoleteBuilding;
	int iLoop;
	int iI;

	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumBuildingInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (iChange != 0)
	{
		bOldObsoleteBuilding = isObsoleteBuilding(eIndex);

		m_paiObsoleteBuildingCount[eIndex] = (m_paiObsoleteBuildingCount[eIndex] + iChange);
		FAssert(getObsoleteBuildingCount(eIndex) >= 0);

		if (bOldObsoleteBuilding != isObsoleteBuilding(eIndex))
		{
			for (iI = 0; iI < MAX_PLAYERS; iI++)
			{
				if (GET_PLAYER((PlayerTypes)iI).isAlive())
				{
					if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
					{
						for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
						{
							if (pLoopCity->hasBuilding(eIndex))
							{
								pLoopCity->processBuilding(eIndex, ((isObsoleteBuilding(eIndex)) ? -1 : 1), true);
							}
						}
					}
				}
			}
		}
	}
}


int CvTeam::getResearchProgress(TechTypes eIndex)							
{
	if (eIndex != NO_TECH)
	{
		return m_paiResearchProgress[eIndex];
	}
	else
	{
		return 0;
	}
}


void CvTeam::setResearchProgress(TechTypes eIndex, int iNewValue, PlayerTypes ePlayer)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumTechInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	FAssertMsg(ePlayer >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(ePlayer < MAX_PLAYERS, "ePlayer is expected to be within maximum bounds (invalid Index)");

	if (getResearchProgress(eIndex) != iNewValue)
	{
		m_paiResearchProgress[eIndex] = iNewValue;
		FAssert(getResearchProgress(eIndex) >= 0);

		if (getID() == GC.getGameINLINE().getActiveTeam())
		{
			gDLL->getInterfaceIFace()->setDirty(GameData_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(Score_DIRTY_BIT, true);
		}

		if (getResearchProgress(eIndex) >= getResearchCost(eIndex))
		{
			int iOverflow = (100 * (getResearchProgress(eIndex) - getResearchCost(eIndex))) / GET_PLAYER(ePlayer).calculateResearchModifier(eIndex);
			GET_PLAYER(ePlayer).changeOverflowResearch(iOverflow);
			setHasTech(eIndex, true, ePlayer, true, true);
			if (!(GC.getGameINLINE().isMPOption(MPOPTION_SIMULTANEOUS_TURNS)))
			{
				setNoTradeTech(eIndex, true);
			}
		}
	}
}


void CvTeam::changeResearchProgress(TechTypes eIndex, int iChange, PlayerTypes ePlayer)
{
	setResearchProgress(eIndex, (getResearchProgress(eIndex) + iChange), ePlayer);
}


int CvTeam::getTechCount(TechTypes eIndex)							
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumTechInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiTechCount[eIndex];
}


int CvTeam::getTerrainTradeCount(TerrainTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumTerrainInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiTerrainTradeCount[eIndex];
}


bool CvTeam::isTerrainTrade(TerrainTypes eIndex)
{
	return (getTerrainTradeCount(eIndex) > 0);
}


void CvTeam::changeTerrainTradeCount(TerrainTypes eIndex, int iChange)
{
	int iI;

	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumTerrainInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (iChange != 0)
	{
		m_paiTerrainTradeCount[eIndex] = (m_paiTerrainTradeCount[eIndex] + iChange);
		FAssert(getTerrainTradeCount(eIndex) >= 0);

		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
				{
					GET_PLAYER((PlayerTypes)iI).updatePlotGroups();
				}
			}
		}
	}
}


bool CvTeam::isHasTech(TechTypes eIndex)
{
	if (eIndex == NO_TECH)
	{
		return true;
	}

	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumTechInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	FAssertMsg(m_pabHasTech != NULL, "m_pabHasTech is not expected to be equal with NULL");
	return m_pabHasTech[eIndex];
}

void CvTeam::announceTechToPlayers(TechTypes eIndex)
{
	bool bSound = (GC.getGameINLINE().isNetworkMultiPlayer() || gDLL->getInterfaceIFace()->noTechSplash());

	for (int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
			{
				CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_DISCOVERED_TECH", GC.getTechInfo(eIndex).getTextKeyWide());

				gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), false, (bSound ? GC.getDefineINT("EVENT_MESSAGE_TIME") : -1), szBuffer, (bSound ? GC.getTechInfo(eIndex).getSoundMP() : NULL), MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_TECH_TEXT"));
			}
		}
	}
}

void CvTeam::setHasTech(TechTypes eIndex, bool bNewValue, PlayerTypes ePlayer, bool bFirst, bool bAnnounce)
{
	PROFILE_FUNC();

	CvCity* pCapitalCity;
	CvCity* pCity;
	CvPlot* pLoopPlot;
	CvWString szBuffer;
	CivicOptionTypes eCivicOptionType;
	CivicTypes eCivicType;
	PlayerTypes eBestPlayer;
	BonusTypes eBonus;
	UnitTypes eFreeUnit;
	bool bReligionFounded;
	bool bFirstBonus;
	int iValue;
	int iBestValue;
	int iI, iJ, iK;

	if (eIndex == NO_TECH)
	{
		return;
	}

	if (ePlayer == NO_PLAYER)
	{
		ePlayer = getLeaderID();
	}

	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumTechInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	FAssertMsg(ePlayer >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(ePlayer < MAX_PLAYERS, "ePlayer is expected to be within maximum bounds (invalid Index)");

	if (isHasTech(eIndex) != bNewValue)
	{
		if (GC.getTechInfo(eIndex).isRepeat())
		{
			m_paiTechCount[eIndex]++;

			setResearchProgress(eIndex, 0, ePlayer);

			// report event to Python
			gDLL->getEventReporterIFace()->techAcquired(eIndex, getID(), ePlayer, bAnnounce && 1 == m_paiTechCount[eIndex]);

			if (1 == m_paiTechCount[eIndex])
			{
				if (bAnnounce)
				{
					if (GC.getGameINLINE().isFinalInitialized() && !(gDLL->GetWorldBuilderMode()))
					{
						announceTechToPlayers(eIndex);
					}
				}
			}
		}
		else
		{
			for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
			{
				PROFILE("CvTeam::setHasTech() 1");

				pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

				if (pLoopPlot->getBonusType() != NO_BONUS)
				{
					if (pLoopPlot->getTeam() == getID())
					{
						if ((GC.getBonusInfo(pLoopPlot->getBonusType()).getTechReveal() == eIndex) ||
							  ((GC.getBonusInfo(pLoopPlot->getBonusType()).getTechCityTrade() == eIndex) && (pLoopPlot->isCity())) ||
								(GC.getBonusInfo(pLoopPlot->getBonusType()).getTechObsolete() == eIndex))
						{
							pLoopPlot->updatePlotGroupBonus(false);
						}
					}
				}
			}
			m_pabHasTech[eIndex] = bNewValue;
			for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
			{
				PROFILE("CvTeam::setHasTech() 2");

				pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

				if (pLoopPlot->getBonusType() != NO_BONUS)
				{
					if (pLoopPlot->getTeam() == getID())
					{
						if ((GC.getBonusInfo(pLoopPlot->getBonusType()).getTechReveal() == eIndex) ||
							  ((GC.getBonusInfo(pLoopPlot->getBonusType()).getTechCityTrade() == eIndex) && (pLoopPlot->isCity())) ||
							  (GC.getBonusInfo(pLoopPlot->getBonusType()).getTechObsolete() == eIndex))
						{
							pLoopPlot->updatePlotGroupBonus(true);
						}
					}
				}
			}
		}

		processTech(eIndex, ((bNewValue) ? 1 : -1));

		if (isHasTech(eIndex))
		{
			PROFILE("CvTeam::setHasTech() 3");

			for (iI = 0; iI < MAX_PLAYERS; iI++)
			{
				if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
				{
					if (GET_PLAYER((PlayerTypes)iI).getCurrentEra() < GC.getTechInfo(eIndex).getEra())
					{
						GET_PLAYER((PlayerTypes)iI).setCurrentEra((EraTypes)(GC.getTechInfo(eIndex).getEra()));
					}
				}
			}

			if (GC.getTechInfo(eIndex).isMapVisible())
			{
				GC.getMapINLINE().setRevealedPlots(getID(), true, true);
			}

			// report event to Python, along with some other key state
			gDLL->getEventReporterIFace()->techAcquired(eIndex, getID(), ePlayer, bAnnounce);

			bReligionFounded = false;
			bFirstBonus = false;

			if (bFirst)
			{
				if (GC.getGameINLINE().countKnownTechNumTeams(eIndex) == 1)
				{
					CyArgsList argsList;
					argsList.add(getID());
					argsList.add(ePlayer);
					argsList.add(eIndex);
					argsList.add(bFirst);
					long lResult=0;
					gDLL->getPythonIFace()->callFunction(PYGameModule, "doHolyCityTech", argsList.makeFunctionArgs(), &lResult);
					if (lResult != 1)
					{
						for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
						{
							if (GC.getReligionInfo((ReligionTypes)iI).getTechPrereq() == eIndex)
							{
								if (!(GC.getGameINLINE().isReligionFounded((ReligionTypes)iI)))
								{
									iBestValue = MAX_INT;
									eBestPlayer = NO_PLAYER;

									for (iJ = 0; iJ < MAX_PLAYERS; iJ++)
									{
										if (GET_PLAYER((PlayerTypes)iJ).isAlive())
										{
											if (GET_PLAYER((PlayerTypes)iJ).getTeam() == getID())
											{
												iValue = 10;

												iValue += GC.getGameINLINE().getSorenRandNum(10, "Found Religion (Player)");

												for (iK = 0; iK < GC.getNumReligionInfos(); iK++)
												{
													iValue += (GET_PLAYER((PlayerTypes)iJ).getHasReligionCount((ReligionTypes)iK) * 10);
												}

												if (GET_PLAYER((PlayerTypes)iJ).getCurrentResearch() != eIndex)
												{
													iValue *= 10;
												}

												if (iValue < iBestValue)
												{
													iBestValue = iValue;
													eBestPlayer = ((PlayerTypes)iJ);
												}
											}
										}
									}

									if (eBestPlayer != NO_PLAYER)
									{
										GET_PLAYER(eBestPlayer).foundReligion((ReligionTypes)iI);
										bReligionFounded = true;
										bFirstBonus = true;
									}
								}
							}
						}
					}
				}
			}

			for (iI = 0; iI < MAX_PLAYERS; iI++)
			{
				if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
				{
					if (GET_PLAYER((PlayerTypes)iI).isResearchingTech(eIndex))
					{
						GET_PLAYER((PlayerTypes)iI).popResearch(eIndex);
					}
				}
			}

			if (bFirst)
			{
				if (GC.getGameINLINE().countKnownTechNumTeams(eIndex) == 1)
				{
					if (GC.getTechInfo(eIndex).getFirstFreeUnitClass() != NO_UNITCLASS)
					{
						bFirstBonus = true;

						eFreeUnit = ((UnitTypes)(GC.getCivilizationInfo(GET_PLAYER(ePlayer).getCivilizationType()).getCivilizationUnits(GC.getTechInfo(eIndex).getFirstFreeUnitClass())));

						if (eFreeUnit != NULL)
						{
							pCapitalCity = GET_PLAYER(ePlayer).getCapitalCity();

							if (pCapitalCity != NULL)
							{
								pCapitalCity->createGreatPeople(eFreeUnit, false);
							}
						}
					}

					if (GC.getTechInfo(eIndex).getFirstFreeTechs() > 0)
					{
						bFirstBonus = true;

						if (!isHuman())
						{
							for (iI = 0; iI < GC.getTechInfo(eIndex).getFirstFreeTechs(); iI++)
							{
								GET_PLAYER(ePlayer).AI_chooseFreeTech();
							}
						}
						else if (ePlayer == GC.getGameINLINE().getActivePlayer())
						{
							szBuffer = gDLL->getText("TXT_KEY_MISC_FIRST_TECH_CHOOSE_FREE", GC.getTechInfo(eIndex).getTextKeyWide());
							GET_PLAYER(ePlayer).chooseTech(GC.getTechInfo(eIndex).getFirstFreeTechs(), szBuffer.GetCString());
						}

						for (iI = 0; iI < MAX_PLAYERS; iI++)
						{
							if (GET_PLAYER((PlayerTypes)iI).isAlive())
							{
								if (isHasMet(GET_PLAYER((PlayerTypes)iI).getTeam()))
								{
									szBuffer = gDLL->getText("TXT_KEY_MISC_SOMEONE_FIRST_TO_TECH", GET_PLAYER(ePlayer).getNameKey(), GC.getTechInfo(eIndex).getTextKeyWide());
								}
								else
								{
									szBuffer = gDLL->getText("TXT_KEY_MISC_UNKNOWN_FIRST_TO_TECH", GC.getTechInfo(eIndex).getTextKeyWide());
								}
								gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_FIRSTTOTECH", MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"));
							}
						}

						szBuffer = gDLL->getText("TXT_KEY_MISC_SOMEONE_FIRST_TO_TECH", GET_PLAYER(ePlayer).getName(), GC.getTechInfo(eIndex).getTextKeyWide());
						GC.getGameINLINE().addReplayMessage(REPLAY_MESSAGE_MAJOR_EVENT, ePlayer, szBuffer, -1, -1, (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"));
					}

					if (bFirstBonus)
					{
						for (iI = 0; iI < MAX_PLAYERS; iI++)
						{
							if (GET_PLAYER((PlayerTypes)iI).isAlive())
							{
								if (!(GET_PLAYER((PlayerTypes)iI).isHuman()))
								{
									if (GET_PLAYER((PlayerTypes)iI).isResearchingTech(eIndex))
									{
										GET_PLAYER((PlayerTypes)iI).clearResearchQueue();
									}
								}
							}
						}
					}
				}
			}

			if (bAnnounce)
			{
				if (GC.getGameINLINE().isFinalInitialized() && !(gDLL->GetWorldBuilderMode()))
				{
					announceTechToPlayers(eIndex);

					for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
					{
						pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

						if (pLoopPlot->getTeam() == getID())
						{
							eBonus = pLoopPlot->getBonusType();

							if (eBonus != NO_BONUS)
							{
								if (GC.getBonusInfo(eBonus).getTechReveal() == eIndex)
								{
									pCity = GC.getMapINLINE().findCity(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), NO_PLAYER, getID(), false);

									if (pCity != NULL)
									{
										szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_DISCOVERED_BONUS", GC.getBonusInfo(eBonus).getTextKeyWide(), pCity->getNameKey());
										gDLL->getInterfaceIFace()->addMessage(pLoopPlot->getOwnerINLINE(), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_DISCOVERBONUS", MESSAGE_TYPE_INFO, GC.getBonusInfo(eBonus).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_WHITE"), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), true, true);
									}
								}
							}
						}
					}

					for (iI = 0; iI < MAX_PLAYERS; iI++)
					{
						if (GET_PLAYER((PlayerTypes)iI).isAlive())
						{
							if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
							{
								if (GET_PLAYER((PlayerTypes)iI).isHuman())
								{
									if (!bReligionFounded || (GET_PLAYER((PlayerTypes)iI).getLastStateReligion() != NO_RELIGION) || (iI != ePlayer))
									{
										if (GET_PLAYER((PlayerTypes)iI).canRevolution(NULL))
										{
											eCivicOptionType = NO_CIVICOPTION;
											eCivicType = NO_CIVIC;

											for (iJ = 0; iJ < GC.getNumCivicOptionInfos(); iJ++)
											{
												if (!(GET_PLAYER((PlayerTypes)iI).isHasCivicOption((CivicOptionTypes)iJ)))
												{
													for (iK = 0; iK < GC.getNumCivicInfos(); iK++)
													{
														if (GC.getCivicInfo((CivicTypes)iK).getCivicOptionType() == iJ)
														{
															if (GC.getCivicInfo((CivicTypes)iK).getTechPrereq() == eIndex)
															{
																eCivicOptionType = ((CivicOptionTypes)iJ);
																eCivicType = ((CivicTypes)iK);
															}
														}
													}
												}
											}

											if ((eCivicOptionType != NO_CIVICOPTION) && (eCivicType != NO_CIVIC))
											{
												CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_CHANGECIVIC);
												if (NULL != pInfo)
												{
													pInfo->setData1(eCivicOptionType);
													pInfo->setData2(eCivicType);
													gDLL->getInterfaceIFace()->addPopup(pInfo, (PlayerTypes)iI);
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

			for (iI = 0; iI < MAX_TEAMS; iI++)
			{
				if (GET_TEAM((TeamTypes)iI).isAlive())
				{
					if (iI != getID())
					{
						GET_TEAM((TeamTypes)iI).updateTechShare(eIndex);
					}
				}
			}
		}

		if (bNewValue)
		{
			if (bAnnounce)
			{
				if (GC.getGameINLINE().isFinalInitialized() && !(gDLL->GetWorldBuilderMode()))
				{
					FAssert(ePlayer != NO_PLAYER);
					if (GET_PLAYER(ePlayer).isResearch() && (GET_PLAYER(ePlayer).getCurrentResearch() == NO_TECH))
					{
						szBuffer = gDLL->getText("TXT_KEY_MISC_WHAT_TO_RESEARCH_NEXT");
						GET_PLAYER(ePlayer).chooseTech(0, szBuffer);
					}
				}
			}
		}

		if (getID() == GC.getGameINLINE().getActiveTeam())
		{
			gDLL->getInterfaceIFace()->setDirty(MiscButtons_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(ResearchButtons_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(GlobeLayer_DIRTY_BIT, true);
		}
	}
}


bool CvTeam::isNoTradeTech(TechTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumTechInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_pabNoTradeTech[eIndex];
}


void CvTeam::setNoTradeTech(TechTypes eIndex, bool bNewValue)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumTechInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_pabNoTradeTech[eIndex] = bNewValue;
}


int CvTeam::getImprovementYieldChange(ImprovementTypes eIndex1, YieldTypes eIndex2)
{
	FAssertMsg(eIndex1 >= 0, "eIndex1 is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex1 < GC.getNumImprovementInfos(), "eIndex1 is expected to be within maximum bounds (invalid Index)");
	FAssertMsg(eIndex2 >= 0, "eIndex2 is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex2 < NUM_YIELD_TYPES, "eIndex2 is expected to be within maximum bounds (invalid Index)");
	return m_ppaaiImprovementYieldChange[eIndex1][eIndex2];
}


void CvTeam::changeImprovementYieldChange(ImprovementTypes eIndex1, YieldTypes eIndex2, int iChange)
{
	FAssertMsg(eIndex1 >= 0, "eIndex1 is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex1 < GC.getNumImprovementInfos(), "eIndex1 is expected to be within maximum bounds (invalid Index)");
	FAssertMsg(eIndex2 >= 0, "eIndex2 is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex2 < NUM_YIELD_TYPES, "eIndex2 is expected to be within maximum bounds (invalid Index)");

	if (iChange != 0)
	{
		m_ppaaiImprovementYieldChange[eIndex1][eIndex2] = (m_ppaaiImprovementYieldChange[eIndex1][eIndex2] + iChange);
		FAssert(getImprovementYieldChange(eIndex1, eIndex2) >= 0);

		updateYield();
	}
}

// Protected Functions...

void CvTeam::doWarWeariness()
{
	int iI;

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		if (getWarWeariness((TeamTypes)iI) > 0)
		{
			changeWarWeariness(((TeamTypes)iI), GC.getDefineINT("WW_DECAY_RATE"));

			if (!(GET_TEAM((TeamTypes)iI).isAlive()) || !isAtWar((TeamTypes)iI))
			{
				setWarWeariness(((TeamTypes)iI), ((getWarWeariness((TeamTypes)iI) * GC.getDefineINT("WW_DECAY_PEACE_PERCENT")) / 100));
			}
		}
	}
}


void CvTeam::updateTechShare(TechTypes eTech)
{
	int iBestShare;
	int iCount;
	int iI;

	if (isHasTech(eTech))
	{
		return;
	}

	iBestShare = MAX_INT;

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		if (isTechShare(iI))
		{
			iBestShare = min(iBestShare, (iI + 1));
		}
	}

	if (iBestShare != MAX_INT)
	{
		iCount = 0;

		for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
		{
			if (GET_TEAM((TeamTypes)iI).isAlive())
			{
				if (GET_TEAM((TeamTypes)iI).isHasTech(eTech))
				{
					if (isHasMet((TeamTypes)iI))
					{
						FAssertMsg(iI != getID(), "iI is not expected to be equal with getID()");
						iCount++;
					}
				}
			}
		}

		if (iCount >= iBestShare)
		{
			setHasTech(eTech, true, NO_PLAYER, true, true);
		}
	}
}


void CvTeam::updateTechShare()
{
	int iI;

	for (iI = 0; iI < GC.getNumTechInfos(); iI++)
	{
		updateTechShare((TechTypes)iI);
	}
}


void CvTeam::testCircumnavigated()
{
	CvPlot* pPlot;
	CvWString szBuffer;
	bool bFoundVisible;
	int iX, iY;

	if (isBarbarian())
	{
		return;
	}

	if (GC.getGameINLINE().isCircumnavigated())
	{
		return;
	}

	if (GC.getDefineINT("CIRCUMNAVIGATE_FREE_MOVES") == 0)
	{
		return;
	}

	if (!(GC.getMapINLINE().isWrapXINLINE()) && !(GC.getMapINLINE().isWrapYINLINE()))
	{
		return;
	}

	if (GC.getMapINLINE().getLandPlots() > ((GC.getMapINLINE().numPlotsINLINE() * 2) / 3))
	{
		return;
	}

	if (GC.getMapINLINE().isWrapXINLINE())
	{
		for (iX = 0; iX < GC.getMapINLINE().getGridWidthINLINE(); iX++)
		{
			bFoundVisible = false;

			for (iY = 0; iY < GC.getMapINLINE().getGridHeightINLINE(); iY++)
			{
				pPlot = GC.getMapINLINE().plotSorenINLINE(iX, iY);

				if (pPlot->isRevealed(getID(), false))
				{
					bFoundVisible = true;
					break;
				}
			}

			if (!bFoundVisible)
			{
				return;
			}
		}
	}

	if (GC.getMapINLINE().isWrapYINLINE())
	{
		for (iY = 0; iY < GC.getMapINLINE().getGridHeightINLINE(); iY++)
		{
			bFoundVisible = false;

			for (iX = 0; iX < GC.getMapINLINE().getGridWidthINLINE(); iX++)
			{
				pPlot = GC.getMapINLINE().plotSorenINLINE(iX, iY);

				if (pPlot->isRevealed(getID(), false))
				{
					bFoundVisible = true;
					break;
				}
			}

			if (!bFoundVisible)
			{
				return;
			}
		}
	}

	GC.getGameINLINE().makeCircumnavigated();

	if (GC.getGameINLINE().getElapsedGameTurns() > 0)
	{
		if (GC.getDefineINT("CIRCUMNAVIGATE_FREE_MOVES") != 0)
		{
			changeExtraMoves(DOMAIN_SEA, GC.getDefineINT("CIRCUMNAVIGATE_FREE_MOVES"));

			for (int iI = 0; iI < MAX_PLAYERS; iI++)
			{
				if (GET_PLAYER((PlayerTypes)iI).isAlive())
				{
					if (getID() == GET_PLAYER((PlayerTypes)iI).getTeam())
					{
						szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_CIRC_GLOBE", GC.getDefineINT("CIRCUMNAVIGATE_FREE_MOVES"));
					}
					else if (isHasMet(GET_PLAYER((PlayerTypes)iI).getTeam()))
					{
						szBuffer = gDLL->getText("TXT_KEY_MISC_SOMEONE_CIRC_GLOBE", getName().GetCString());
					}
					else
					{
						szBuffer = gDLL->getText("TXT_KEY_MISC_UNKNOWN_CIRC_GLOBE");
					}
					gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_GLOBECIRCUMNAVIGATED", MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"));
				}
			}

			szBuffer = gDLL->getText("TXT_KEY_MISC_SOMEONE_CIRC_GLOBE", getName().GetCString());
			GC.getGameINLINE().addReplayMessage(REPLAY_MESSAGE_MAJOR_EVENT, getLeaderID(), szBuffer, -1, -1, (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"));
		}
	}
}


void CvTeam::processTech(TechTypes eTech, int iChange)
{
	PROFILE_FUNC();

	CvCity* pCity;
	CvPlot* pLoopPlot;
	BonusTypes eBonus;
	int iI, iJ;

	if (GC.getTechInfo(eTech).isExtraWaterSeeFrom())
	{
		changeExtraWaterSeeFromCount(iChange);
	}

	if (GC.getTechInfo(eTech).isMapCentering())
	{
		if (iChange > 0)
		{
			setMapCentering(true);
		}
	}

	if (GC.getTechInfo(eTech).isMapTrading())
	{
		changeMapTradingCount(iChange);
	}

	if (GC.getTechInfo(eTech).isTechTrading())
	{
		changeTechTradingCount(iChange);
	}

	if (GC.getTechInfo(eTech).isGoldTrading())
	{
		changeGoldTradingCount(iChange);
	}

	if (GC.getTechInfo(eTech).isOpenBordersTrading())
	{
		changeOpenBordersTradingCount(iChange);
	}

	if (GC.getTechInfo(eTech).isDefensivePactTrading())
	{
		changeDefensivePactTradingCount(iChange);
	}

	if (GC.getTechInfo(eTech).isPermanentAllianceTrading())
	{
		changePermanentAllianceTradingCount(iChange);
	}

	if (GC.getTechInfo(eTech).isBridgeBuilding())
	{
		changeBridgeBuildingCount(iChange);
	}

	if (GC.getTechInfo(eTech).isIrrigation())
	{
		changeIrrigationCount(iChange);
	}

	if (GC.getTechInfo(eTech).isIgnoreIrrigation())
	{
		changeIgnoreIrrigationCount(iChange);
	}

	if (GC.getTechInfo(eTech).isWaterWork())
	{
		changeWaterWorkCount(iChange);
	}

	for (iI = 0; iI < GC.getNumRouteInfos(); iI++)
	{
		changeRouteChange(((RouteTypes)iI), (GC.getRouteInfo((RouteTypes) iI).getTechMovementChange(eTech) * iChange));
	}

	for (iI = 0; iI < NUM_DOMAIN_TYPES; iI++)
	{
		changeExtraMoves(((DomainTypes)iI), (GC.getTechInfo(eTech).getDomainExtraMoves(iI) * iChange));
	}

	for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		if (GC.getTechInfo(eTech).isCommerceFlexible(iI))
		{
			changeCommerceFlexibleCount(((CommerceTypes)iI), iChange);
		}
	}

	for (iI = 0; iI < GC.getNumTerrainInfos(); iI++)
	{
		if (GC.getTechInfo(eTech).isTerrainTrade(iI))
		{
			changeTerrainTradeCount(((TerrainTypes)iI), iChange);
		}
	}

	for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
	{
		if (GC.getBuildingInfo((BuildingTypes) iI).getObsoleteTech() == eTech)
		{
			changeObsoleteBuildingCount(((BuildingTypes)iI), iChange);
		}

		if (GC.getBuildingInfo((BuildingTypes) iI).getSpecialBuildingType() != NO_SPECIALBUILDING)
		{
			if (GC.getSpecialBuildingInfo((SpecialBuildingTypes) GC.getBuildingInfo((BuildingTypes) iI).getSpecialBuildingType()).getObsoleteTech() == eTech)
			{
				changeObsoleteBuildingCount(((BuildingTypes)iI), iChange);
			}
		}
	}

	for (iI = 0; iI < GC.getNumImprovementInfos(); iI++)
	{
		for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
		{
			changeImprovementYieldChange(((ImprovementTypes)iI), ((YieldTypes)iJ), (GC.getImprovementInfo((ImprovementTypes)iI).getTechYieldChanges(eTech, iJ) * iChange));
		}
	}

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).getTeam() == getID())
		{
			GET_PLAYER((PlayerTypes)iI).changeFeatureProductionModifier(GC.getTechInfo(eTech).getFeatureProductionModifier() * iChange);
			GET_PLAYER((PlayerTypes)iI).changeWorkerSpeedModifier(GC.getTechInfo(eTech).getWorkerSpeedModifier() * iChange);
			GET_PLAYER((PlayerTypes)iI).changeTradeRoutes(GC.getTechInfo(eTech).getTradeRoutes() * iChange);
			GET_PLAYER((PlayerTypes)iI).changeExtraHealth(GC.getTechInfo(eTech).getHealth() * iChange);
			GET_PLAYER((PlayerTypes)iI).changeExtraHappiness(GC.getTechInfo(eTech).getHappiness() * iChange);

			GET_PLAYER((PlayerTypes)iI).changeAssets(GC.getTechInfo(eTech).getAssetValue() * iChange);
			GET_PLAYER((PlayerTypes)iI).changePower(GC.getTechInfo(eTech).getPowerValue() * iChange);
			GET_PLAYER((PlayerTypes)iI).changeTechScore(getTechScore(eTech) * iChange);
		}
	}

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		eBonus = pLoopPlot->getBonusType();

		if (eBonus != NO_BONUS)
		{
			if (GC.getBonusInfo(eBonus).getTechReveal() == eTech)
			{
				pLoopPlot->updateYield();
				pLoopPlot->setLayoutDirty(true);
			}
		}
	}

	for (iI = 0; iI < GC.getNumBuildInfos(); iI++)
	{
		if (GC.getBuildInfo((BuildTypes) iI).getTechPrereq() == eTech)
		{
			if (GC.getBuildInfo((BuildTypes) iI).getRoute() != NO_ROUTE)
			{
				for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
				{
					pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

					pCity = pLoopPlot->getPlotCity();

					if (pCity != NULL)
					{
						if (pCity->getTeam() == getID())
						{
							pLoopPlot->updateCityRoute();
						}
					}
				}
			}
		}
	}
}


void CvTeam::cancelDefensivePacts()
{
	CLLNode<TradeData>* pNode;
	CvDeal* pLoopDeal;
	bool bCancelDeal;
	int iLoop;

	for (pLoopDeal = GC.getGameINLINE().firstDeal(&iLoop); pLoopDeal != NULL; pLoopDeal = GC.getGameINLINE().nextDeal(&iLoop))
	{
		bCancelDeal = false;

		if ((GET_PLAYER(pLoopDeal->getFirstPlayer()).getTeam() == getID()) ||
			  (GET_PLAYER(pLoopDeal->getSecondPlayer()).getTeam() == getID()))
		{
			for (pNode = pLoopDeal->headFirstTradesNode(); (pNode != NULL); pNode = pLoopDeal->nextFirstTradesNode(pNode))
			{
				if (pNode->m_data.m_eItemType == TRADE_DEFENSIVE_PACT)
				{
					bCancelDeal = true;
				}
			}

			for (pNode = pLoopDeal->headSecondTradesNode(); (pNode != NULL); pNode = pLoopDeal->nextSecondTradesNode(pNode))
			{
				if (pNode->m_data.m_eItemType == TRADE_DEFENSIVE_PACT)
				{
					bCancelDeal = true;
				}
			}
		}

		if (bCancelDeal)
		{
			pLoopDeal->kill();
		}
	}
}


void CvTeam::read(FDataStreamBase* pStream)
{
	int iI;

	// Init data before load
	reset();

	uint uiFlag=0;
	pStream->Read(&uiFlag);	// flags for expansion

	pStream->Read(&m_iNumMembers);
	pStream->Read(&m_iAliveCount);
	pStream->Read(&m_iEverAliveCount);
	pStream->Read(&m_iNumCities);
	pStream->Read(&m_iTotalPopulation);
	pStream->Read(&m_iTotalLand);
	pStream->Read(&m_iNukeInterception);
	pStream->Read(&m_iForceTeamVoteEligibilityCount);
	pStream->Read(&m_iExtraWaterSeeFromCount);
	pStream->Read(&m_iMapTradingCount);
	pStream->Read(&m_iTechTradingCount);
	pStream->Read(&m_iGoldTradingCount);
	pStream->Read(&m_iOpenBordersTradingCount);
	pStream->Read(&m_iDefensivePactTradingCount);
	pStream->Read(&m_iPermanentAllianceTradingCount);
	pStream->Read(&m_iBridgeBuildingCount);
	pStream->Read(&m_iIrrigationCount);
	pStream->Read(&m_iIgnoreIrrigationCount);
	pStream->Read(&m_iWaterWorkCount);

	pStream->Read(&m_bMapCentering);

	pStream->Read((int*)&m_eID);

	pStream->Read(MAX_TEAMS, m_aiStolenVisibilityTimer);
	pStream->Read(MAX_TEAMS, m_aiWarWeariness);
	pStream->Read(MAX_TEAMS, m_aiTechShareCount);
	pStream->Read(NUM_COMMERCE_TYPES, m_aiCommerceFlexibleCount);
	pStream->Read(NUM_DOMAIN_TYPES, m_aiExtraMoves);

	pStream->Read(MAX_TEAMS, m_abHasMet);
	pStream->Read(MAX_TEAMS, m_abAtWar);
	pStream->Read(MAX_TEAMS, m_abPermanentWarPeace);
	pStream->Read(MAX_TEAMS, m_abOpenBorders);
	pStream->Read(MAX_TEAMS, m_abDefensivePact);
	pStream->Read(MAX_TEAMS, m_abForcePeace);

	pStream->Read(GC.getNumRouteInfos(), m_paiRouteChange);
	pStream->Read(GC.getNumProjectInfos(), m_paiProjectCount);
	pStream->Read(GC.getNumProjectInfos(), m_paiProjectMaking);
	pStream->Read(GC.getNumUnitClassInfos(), m_paiUnitClassCount);
	pStream->Read(GC.getNumBuildingClassInfos(), m_paiBuildingClassCount);
	pStream->Read(GC.getNumBuildingInfos(), m_paiObsoleteBuildingCount);
	pStream->Read(GC.getNumTechInfos(), m_paiResearchProgress);
	pStream->Read(GC.getNumTechInfos(), m_paiTechCount);
	pStream->Read(GC.getNumTerrainInfos(), m_paiTerrainTradeCount);

	pStream->Read(GC.getNumTechInfos(), m_pabHasTech);
	pStream->Read(GC.getNumTechInfos(), m_pabNoTradeTech);

	for (iI=0;iI<GC.getNumImprovementInfos();iI++)
	{
		pStream->Read(NUM_YIELD_TYPES, m_ppaaiImprovementYieldChange[iI]);
	}
}


void CvTeam::write(FDataStreamBase* pStream)
{
	int iI;

	uint uiFlag=0;
	pStream->Write(uiFlag);		// flag for expansion

	pStream->Write(m_iNumMembers);
	pStream->Write(m_iAliveCount);
	pStream->Write(m_iEverAliveCount);
	pStream->Write(m_iNumCities);
	pStream->Write(m_iTotalPopulation);
	pStream->Write(m_iTotalLand);
	pStream->Write(m_iNukeInterception);
	pStream->Write(m_iForceTeamVoteEligibilityCount);
	pStream->Write(m_iExtraWaterSeeFromCount);
	pStream->Write(m_iMapTradingCount);
	pStream->Write(m_iTechTradingCount);
	pStream->Write(m_iGoldTradingCount);
	pStream->Write(m_iOpenBordersTradingCount);
	pStream->Write(m_iDefensivePactTradingCount);
	pStream->Write(m_iPermanentAllianceTradingCount);
	pStream->Write(m_iBridgeBuildingCount);
	pStream->Write(m_iIrrigationCount);
	pStream->Write(m_iIgnoreIrrigationCount);
	pStream->Write(m_iWaterWorkCount);

	pStream->Write(m_bMapCentering);

	pStream->Write(m_eID);

	pStream->Write(MAX_TEAMS, m_aiStolenVisibilityTimer);
	pStream->Write(MAX_TEAMS, m_aiWarWeariness);
	pStream->Write(MAX_TEAMS, m_aiTechShareCount);
	pStream->Write(NUM_COMMERCE_TYPES, m_aiCommerceFlexibleCount);
	pStream->Write(NUM_DOMAIN_TYPES, m_aiExtraMoves);

	pStream->Write(MAX_TEAMS, m_abHasMet);
	pStream->Write(MAX_TEAMS, m_abAtWar);
	pStream->Write(MAX_TEAMS, m_abPermanentWarPeace);
	pStream->Write(MAX_TEAMS, m_abOpenBorders);
	pStream->Write(MAX_TEAMS, m_abDefensivePact);
	pStream->Write(MAX_TEAMS, m_abForcePeace);

	pStream->Write(GC.getNumRouteInfos(), m_paiRouteChange);
	pStream->Write(GC.getNumProjectInfos(), m_paiProjectCount);
	pStream->Write(GC.getNumProjectInfos(), m_paiProjectMaking);
	pStream->Write(GC.getNumUnitClassInfos(), m_paiUnitClassCount);
	pStream->Write(GC.getNumBuildingClassInfos(), m_paiBuildingClassCount);
	pStream->Write(GC.getNumBuildingInfos(), m_paiObsoleteBuildingCount);
	pStream->Write(GC.getNumTechInfos(), m_paiResearchProgress);
	pStream->Write(GC.getNumTechInfos(), m_paiTechCount);
	pStream->Write(GC.getNumTerrainInfos(), m_paiTerrainTradeCount);

	pStream->Write(GC.getNumTechInfos(), m_pabHasTech);
	pStream->Write(GC.getNumTechInfos(), m_pabNoTradeTech);

	for (iI=0;iI<GC.getNumImprovementInfos();iI++)
	{
		pStream->Write(NUM_YIELD_TYPES, m_ppaaiImprovementYieldChange[iI]);
	}
}
