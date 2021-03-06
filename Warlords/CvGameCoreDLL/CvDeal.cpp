// CvDeal.cpp

#include "CvGameCoreDLL.h"
#include "CvGlobals.h"
#include "CvGameAI.h"
#include "CvTeamAI.h"
#include "CvPlayerAI.h"
#include "CvMap.h"
#include "CvPlot.h"
#include "CvGameCoreUtils.h"
#include "CvGameTextMgr.h"
#include "CvDLLInterfaceIFaceBase.h"

// Public Functions...

CvDeal::CvDeal()
{
	reset();
}


CvDeal::~CvDeal()
{
	uninit();
}


void CvDeal::init(int iID, PlayerTypes eFirstPlayer, PlayerTypes eSecondPlayer)
{
	//--------------------------------
	// Init saved data
	reset(iID, eFirstPlayer, eSecondPlayer);

	//--------------------------------
	// Init non-saved data

	//--------------------------------
	// Init other game data
	setInitialGameTurn(GC.getGameINLINE().getGameTurn());
}


void CvDeal::uninit()
{
	m_firstTrades.clear();
	m_secondTrades.clear();
}


// FUNCTION: reset()
// Initializes data members that are serialized.
void CvDeal::reset(int iID, PlayerTypes eFirstPlayer, PlayerTypes eSecondPlayer)
{
	//--------------------------------
	// Uninit class
	uninit();

	m_iID = iID;
	m_iInitialGameTurn = 0;

	m_eFirstPlayer = eFirstPlayer;
	m_eSecondPlayer = eSecondPlayer;
}


void CvDeal::kill(bool bKillTeam)
{
	if ((getLengthFirstTrades() > 0) || (getLengthSecondTrades() > 0))
	{
		CvWString szString;
		CvWString szDealString;
		CvWString szCancelString = gDLL->getText("TXT_KEY_POPUP_DEAL_CANCEL");

		szDealString.clear();
		GAMETEXT.getDealString(szDealString, *this, getFirstPlayer());
		szString.Format(L"%s: %s", szCancelString.GetCString(), szDealString.GetCString());
		gDLL->getInterfaceIFace()->addMessage((PlayerTypes)getFirstPlayer(), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szString, "AS2D_DEAL_CANCELLED");

		szDealString.clear();
		GAMETEXT.getDealString(szDealString, *this, getSecondPlayer());
		szString.Format(L"%s: %s", szCancelString.GetCString(), szDealString.GetCString());
		gDLL->getInterfaceIFace()->addMessage((PlayerTypes)getSecondPlayer(), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szString, "AS2D_DEAL_CANCELLED");
	}

	CLLNode<TradeData>* pNode;

	for (pNode = headFirstTradesNode(); (pNode != NULL); pNode = nextFirstTradesNode(pNode))
	{
		endTrade(pNode->m_data, getFirstPlayer(), getSecondPlayer(), bKillTeam);
	}

	for (pNode = headSecondTradesNode(); (pNode != NULL); pNode = nextSecondTradesNode(pNode))
	{
		endTrade(pNode->m_data, getSecondPlayer(), getFirstPlayer(), bKillTeam);
	}

	GC.getGameINLINE().deleteDeal(getID());
}


void CvDeal::addTrades(CLinkList<TradeData>* pFirstList, CLinkList<TradeData>* pSecondList, bool bCheckAllowed)
{
	CLLNode<TradeData>* pNode;
	bool bAlliance;
	bool bSave;
	int iValue;

	if (isVassalTrade(pFirstList) && isVassalTrade(pSecondList))
	{
		return;
	}

	if (pFirstList != NULL)
	{
		for (pNode = pFirstList->head(); pNode; pNode = pFirstList->next(pNode))
		{
			if (bCheckAllowed)
			{
				if (!(GET_PLAYER(getFirstPlayer()).canTradeItem(getSecondPlayer(), pNode->m_data)))
				{
					return;
				}
			}
		}
	}

	if (pSecondList != NULL)
	{
		for (pNode = pSecondList->head(); pNode; pNode = pSecondList->next(pNode))
		{
			if (bCheckAllowed && !(GET_PLAYER(getSecondPlayer()).canTradeItem(getFirstPlayer(), pNode->m_data)))
			{
				return;
			}
		}
	}

	if (atWar(GET_PLAYER(getFirstPlayer()).getTeam(), GET_PLAYER(getSecondPlayer()).getTeam()))
	{
		GET_TEAM(GET_PLAYER(getFirstPlayer()).getTeam()).makePeace(GET_PLAYER(getSecondPlayer()).getTeam(), !isVassalTrade(pFirstList) && !isVassalTrade(pSecondList));
	}
	else
	{
		if (!isPeaceDealBetweenOthers(pFirstList, pSecondList))
		{
			if ((pSecondList != NULL) && (pSecondList->getLength() > 0))
			{
				iValue = GET_PLAYER(getFirstPlayer()).AI_dealVal(getSecondPlayer(), pSecondList, true);

				if ((pFirstList != NULL) && (pFirstList->getLength() > 0))
				{
					GET_PLAYER(getFirstPlayer()).AI_changePeacetimeTradeValue(getSecondPlayer(), iValue);
				}
				else
				{
					GET_PLAYER(getFirstPlayer()).AI_changePeacetimeGrantValue(getSecondPlayer(), iValue);
				}
			}
			if ((pFirstList != NULL) && (pFirstList->getLength() > 0))
			{
				iValue = GET_PLAYER(getSecondPlayer()).AI_dealVal(getFirstPlayer(), pFirstList, true);

				if ((pSecondList != NULL) && (pSecondList->getLength() > 0))
				{
					GET_PLAYER(getSecondPlayer()).AI_changePeacetimeTradeValue(getFirstPlayer(), iValue);
				}
				else
				{
					GET_PLAYER(getSecondPlayer()).AI_changePeacetimeGrantValue(getFirstPlayer(), iValue);
				}
			}
		}
	}

	if (pFirstList != NULL)
	{
		for (pNode = pFirstList->head(); pNode; pNode = pFirstList->next(pNode))
		{
			bSave = startTrade(pNode->m_data, getFirstPlayer(), getSecondPlayer());

			if (bSave)
			{
				insertAtEndFirstTrades(pNode->m_data);
			}
		}
	}

	if (pSecondList != NULL)
	{
		for (pNode = pSecondList->head(); pNode; pNode = pSecondList->next(pNode))
		{
			bSave = startTrade(pNode->m_data, getSecondPlayer(), getFirstPlayer());

			if (bSave)
			{
				insertAtEndSecondTrades(pNode->m_data);
			}
		}
	}

	bAlliance = false;

	if (pFirstList != NULL)
	{
		for (pNode = pFirstList->head(); pNode; pNode = pFirstList->next(pNode))
		{
			if (pNode->m_data.m_eItemType == TRADE_PERMANENT_ALLIANCE)
			{
				bAlliance = true;
			}
		}
	}

	if (pSecondList != NULL)
	{
		for (pNode = pSecondList->head(); pNode; pNode = pSecondList->next(pNode))
		{
			if (pNode->m_data.m_eItemType == TRADE_PERMANENT_ALLIANCE)
			{
				bAlliance = true;
			}
		}
	}

	if (bAlliance)
	{
		if (GET_PLAYER(getFirstPlayer()).getTeam() < GET_PLAYER(getSecondPlayer()).getTeam())
		{
			GET_TEAM(GET_PLAYER(getFirstPlayer()).getTeam()).addTeam(GET_PLAYER(getSecondPlayer()).getTeam());
		}
		else if (GET_PLAYER(getSecondPlayer()).getTeam() < GET_PLAYER(getFirstPlayer()).getTeam())
		{
			GET_TEAM(GET_PLAYER(getSecondPlayer()).getTeam()).addTeam(GET_PLAYER(getFirstPlayer()).getTeam());
		}
	}
}


void CvDeal::doTurn()
{
	int iValue;

	if (!isPeaceDeal())
	{
		if (getLengthSecondTrades() > 0)
		{
			iValue = (GET_PLAYER(getFirstPlayer()).AI_dealVal(getSecondPlayer(), getSecondTrades()) / GC.getDefineINT("PEACE_TREATY_LENGTH"));

			if (getLengthFirstTrades() > 0)
			{
				GET_PLAYER(getFirstPlayer()).AI_changePeacetimeTradeValue(getSecondPlayer(), iValue);
			}
			else
			{
				GET_PLAYER(getFirstPlayer()).AI_changePeacetimeGrantValue(getSecondPlayer(), iValue);
			}
		}

		if (getLengthFirstTrades() > 0)
		{
			iValue = (GET_PLAYER(getSecondPlayer()).AI_dealVal(getFirstPlayer(), getFirstTrades()) / GC.getDefineINT("PEACE_TREATY_LENGTH"));

			if (getLengthSecondTrades() > 0)
			{
				GET_PLAYER(getSecondPlayer()).AI_changePeacetimeTradeValue(getFirstPlayer(), iValue);
			}
			else
			{
				GET_PLAYER(getSecondPlayer()).AI_changePeacetimeGrantValue(getFirstPlayer(), iValue);
			}
		}
	}
}


// XXX probably should have some sort of message for the user or something...
void CvDeal::verify()
{
	CLLNode<TradeData>* pNode;
	bool bCancelDeal;

	bCancelDeal = false;

	for (pNode = headFirstTradesNode(); (pNode != NULL); pNode = nextFirstTradesNode(pNode))
	{
		if (pNode->m_data.m_eItemType == TRADE_RESOURCES)
		{
			// XXX embargoes?
			if ((GET_PLAYER(getFirstPlayer()).getNumTradeableBonuses((BonusTypes)(pNode->m_data.m_iData)) < 0) ||
				  !(GET_PLAYER(getFirstPlayer()).canTradeNetworkWith(getSecondPlayer())))
			{
				bCancelDeal = true;
			}
		}
	}

	for (pNode = headSecondTradesNode(); (pNode != NULL); pNode = nextSecondTradesNode(pNode))
	{
		if (pNode->m_data.m_eItemType == TRADE_RESOURCES)
		{
			// XXX embargoes?
			if ((GET_PLAYER(getSecondPlayer()).getNumTradeableBonuses((BonusTypes)(pNode->m_data.m_iData)) < 0) ||
				  !(GET_PLAYER(getSecondPlayer()).canTradeNetworkWith(getFirstPlayer())))
			{
				bCancelDeal = true;
			}
		}
	}

	if (isCancelable(NO_PLAYER))
	{
		if (isPeaceDeal())
		{
			bCancelDeal = true;
		}
	}

	if (bCancelDeal)
	{
		kill();
	}
}


bool CvDeal::isPeaceDeal() const
{
	CLLNode<TradeData>* pNode;

	for (pNode = headFirstTradesNode(); (pNode != NULL); pNode = nextFirstTradesNode(pNode))
	{
		if (pNode->m_data.m_eItemType == TRADE_PEACE_TREATY)
		{
			return true;
		}
	}

	for (pNode = headSecondTradesNode(); (pNode != NULL); pNode = nextSecondTradesNode(pNode))
	{
		if (pNode->m_data.m_eItemType == TRADE_PEACE_TREATY)
		{
			return true;
		}
	}

	return false;
}

bool CvDeal::isVassalDeal() const
{
	return (isVassalTrade(&m_firstTrades) || isVassalTrade(&m_secondTrades));
}

bool CvDeal::isVassalTrade(const CLinkList<TradeData>* pList)
{
	if (pList)
	{
		for (CLLNode<TradeData>* pNode = pList->head(); pNode != NULL; pNode = pList->next(pNode))
		{
			if (pNode->m_data.m_eItemType == TRADE_VASSAL || pNode->m_data.m_eItemType == TRADE_SURRENDER)
			{
				return true;
			}
		}
	}

	return false;
}


bool CvDeal::isUncancelableVassalDeal(PlayerTypes eByPlayer, CvWString* pszReason) const
{
	CLLNode<TradeData>* pNode;

	for (pNode = headFirstTradesNode(); (pNode != NULL); pNode = nextFirstTradesNode(pNode))
	{
		if (pNode->m_data.m_eItemType == TRADE_SURRENDER || pNode->m_data.m_eItemType == TRADE_VASSAL)
		{
			if (eByPlayer == getSecondPlayer())
			{
				if (pszReason)
				{
					*pszReason += gDLL->getText("TXT_KEY_MISC_DEAL_NO_CANCEL_EVER");
				}

				return true;
			}
		}

		if (pNode->m_data.m_eItemType == TRADE_SURRENDER)
		{
			CvTeam& kVassal = GET_TEAM(GET_PLAYER(getFirstPlayer()).getTeam());
			TeamTypes eMaster = GET_PLAYER(getSecondPlayer()).getTeam();
			
			if (!kVassal.canVassalRevolt(eMaster))
			{
				if (pszReason)
				{
					GAMETEXT.setVassalRevoltHelp(*pszReason, eMaster, GET_PLAYER(getFirstPlayer()).getTeam());
				}

				return true;
			}
		}
	}

	for (pNode = headSecondTradesNode(); (pNode != NULL); pNode = nextSecondTradesNode(pNode))
	{
		if (pNode->m_data.m_eItemType == TRADE_SURRENDER || pNode->m_data.m_eItemType == TRADE_VASSAL)
		{
			if (eByPlayer == getFirstPlayer())
			{
				if (pszReason)
				{
					*pszReason += gDLL->getText("TXT_KEY_MISC_DEAL_NO_CANCEL_EVER");
				}

				return true;
			}
		}

		if (pNode->m_data.m_eItemType == TRADE_SURRENDER)
		{
			CvTeam& kVassal = GET_TEAM(GET_PLAYER(getSecondPlayer()).getTeam());
			TeamTypes eMaster = GET_PLAYER(getFirstPlayer()).getTeam();
			
			if (!kVassal.canVassalRevolt(eMaster))
			{
				if (pszReason)
				{
					if (pszReason)
					{
						GAMETEXT.setVassalRevoltHelp(*pszReason, eMaster, GET_PLAYER(getFirstPlayer()).getTeam());
					}
				}

				return true;
			}
		}
	}

	return false;
}

bool CvDeal::isVassalTributeDeal(const CLinkList<TradeData>* pList)
{
	bool bResources = false;

	for (CLLNode<TradeData>* pNode = pList->head(); pNode != NULL; pNode = pList->next(pNode))
	{
		if (pNode->m_data.m_eItemType == TRADE_RESOURCES)
		{
			if (!bResources)
			{
				bResources = true;
			}
			else
			{
				bResources = false;
				break;
			}
		}
		else
		{
			bResources = false;
			break;
		}
	}

	return bResources;
}

bool CvDeal::isPeaceDealBetweenOthers(CLinkList<TradeData>* pFirstList, CLinkList<TradeData>* pSecondList) const
{
	CLLNode<TradeData>* pNode;

	if (pFirstList != NULL)
	{
		for (pNode = pFirstList->head(); pNode; pNode = pFirstList->next(pNode))
		{
			if (pNode->m_data.m_eItemType == TRADE_PEACE)
			{
				return true;
			}
		}
	}

	if (pSecondList != NULL)
	{
		for (pNode = pSecondList->head(); pNode; pNode = pSecondList->next(pNode))
		{
			if (pNode->m_data.m_eItemType == TRADE_PEACE)
			{
				return true;
			}
		}
	}

	return false;
}


int CvDeal::getID() const
{
	return m_iID;
}


void CvDeal::setID(int iID)
{
	m_iID = iID;
}


int CvDeal::getInitialGameTurn() const
{
	return m_iInitialGameTurn;
}


void CvDeal::setInitialGameTurn(int iNewValue)
{
	m_iInitialGameTurn = iNewValue;
}


PlayerTypes CvDeal::getFirstPlayer() const
{
	return m_eFirstPlayer;
}


PlayerTypes CvDeal::getSecondPlayer() const
{
	return m_eSecondPlayer;
}


void CvDeal::clearFirstTrades()
{
	m_firstTrades.clear();
}


void CvDeal::insertAtEndFirstTrades(TradeData trade)
{
	m_firstTrades.insertAtEnd(trade);
}


CLLNode<TradeData>* CvDeal::nextFirstTradesNode(CLLNode<TradeData>* pNode) const
{
	return m_firstTrades.next(pNode);
}


int CvDeal::getLengthFirstTrades() const
{
	return m_firstTrades.getLength();
}


CLLNode<TradeData>* CvDeal::headFirstTradesNode() const
{
	return m_firstTrades.head();
}


const CLinkList<TradeData>* CvDeal::getFirstTrades() const
{
	return &(m_firstTrades);
}


void CvDeal::clearSecondTrades()
{
	m_secondTrades.clear();
}


void CvDeal::insertAtEndSecondTrades(TradeData trade)
{
	m_secondTrades.insertAtEnd(trade);
}


CLLNode<TradeData>* CvDeal::nextSecondTradesNode(CLLNode<TradeData>* pNode) const
{
	return m_secondTrades.next(pNode);
}


int CvDeal::getLengthSecondTrades() const
{
	return m_secondTrades.getLength();
}


CLLNode<TradeData>* CvDeal::headSecondTradesNode() const
{
	return m_secondTrades.head();
}


const CLinkList<TradeData>* CvDeal::getSecondTrades() const
{
	return &(m_secondTrades);
}


void CvDeal::write(FDataStreamBase* pStream)
{
	uint uiFlag=0;
	pStream->Write(uiFlag);		// flag for expansion

	pStream->Write(m_iID);
	pStream->Write(m_iInitialGameTurn);

	pStream->Write(m_eFirstPlayer);
	pStream->Write(m_eSecondPlayer);

	m_firstTrades.Write(pStream);
	m_secondTrades.Write(pStream);
}

void CvDeal::read(FDataStreamBase* pStream)
{
	uint uiFlag=0;
	pStream->Read(&uiFlag);	// flags for expansion

	pStream->Read(&m_iID);
	pStream->Read(&m_iInitialGameTurn);

	pStream->Read((int*)&m_eFirstPlayer);
	pStream->Read((int*)&m_eSecondPlayer);

	m_firstTrades.Read(pStream);
	m_secondTrades.Read(pStream);
}

// Protected Functions...

// Returns true if the trade should be saved...
bool CvDeal::startTrade(TradeData trade, PlayerTypes eFromPlayer, PlayerTypes eToPlayer)
{
	CivicTypes* paeNewCivics;
	CvCity* pCity;
	CvPlot* pLoopPlot;
	bool bSave;
	int iI;

	bSave = false;

	switch (trade.m_eItemType)
	{
	case TRADE_TECHNOLOGIES:
		GET_TEAM(GET_PLAYER(eToPlayer).getTeam()).setHasTech(((TechTypes)trade.m_iData), true, eToPlayer, true, true);
		GET_TEAM(GET_PLAYER(eToPlayer).getTeam()).setNoTradeTech(((TechTypes)trade.m_iData), true);

		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if (GET_PLAYER((PlayerTypes)iI).getTeam() == GET_PLAYER(eToPlayer).getTeam())
				{
					GET_PLAYER((PlayerTypes)iI).AI_changeMemoryCount(eFromPlayer, MEMORY_TRADED_TECH_TO_US, 1);
				}
				else
				{
					if (GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).isHasMet(GET_PLAYER(eToPlayer).getTeam()))
					{
						GET_PLAYER((PlayerTypes)iI).AI_changeMemoryCount(eToPlayer, MEMORY_RECEIVED_TECH_FROM_ANY, 1);
					}
				}
			}
		}
		break;

	case TRADE_RESOURCES:
		GET_PLAYER(eFromPlayer).changeBonusExport(((BonusTypes)trade.m_iData), 1);
		GET_PLAYER(eToPlayer).changeBonusImport(((BonusTypes)trade.m_iData), 1);
		bSave = true;
		break;

	case TRADE_CITIES:
		pCity = GET_PLAYER(eFromPlayer).getCity(trade.m_iData);
		if (pCity != NULL)
		{
			pCity->doTask(TASK_GIFT, eToPlayer);
		}
		break;

	case TRADE_GOLD:
		GET_PLAYER(eFromPlayer).changeGold(-(trade.m_iData));
		GET_PLAYER(eToPlayer).changeGold(trade.m_iData);
		GET_PLAYER(eFromPlayer).AI_changeGoldTradedTo(eToPlayer, trade.m_iData);
		break;

	case TRADE_GOLD_PER_TURN:
		GET_PLAYER(eFromPlayer).changeGoldPerTurnByPlayer(eToPlayer, -(trade.m_iData));
		GET_PLAYER(eToPlayer).changeGoldPerTurnByPlayer(eFromPlayer, trade.m_iData);
		bSave = true;
		break;

	case TRADE_MAPS:
		for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
		{
			pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

			if (pLoopPlot->isRevealed(GET_PLAYER(eFromPlayer).getTeam(), false))
			{
				pLoopPlot->setRevealed(GET_PLAYER(eToPlayer).getTeam(), true, false, GET_PLAYER(eFromPlayer).getTeam());
			}
		}
		break;

	case TRADE_SURRENDER:
	case TRADE_VASSAL:
		if (trade.m_iData == 0)
		{
			startTeamTrade(trade.m_eItemType, GET_PLAYER(eFromPlayer).getTeam(), GET_PLAYER(eToPlayer).getTeam(), false);
			GET_TEAM(GET_PLAYER(eFromPlayer).getTeam()).setVassal(GET_PLAYER(eToPlayer).getTeam(), true, TRADE_SURRENDER == trade.m_eItemType);
		}
		else
		{
			bSave = true;
		}


		break;

	case TRADE_PEACE:
		GET_TEAM(GET_PLAYER(eFromPlayer).getTeam()).makePeace((TeamTypes)trade.m_iData);
		break;

	case TRADE_WAR:
		GET_TEAM(GET_PLAYER(eFromPlayer).getTeam()).declareWar(((TeamTypes)trade.m_iData), true);

		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if (GET_PLAYER((PlayerTypes)iI).getTeam() == ((TeamTypes)trade.m_iData))
				{
					GET_PLAYER((PlayerTypes)iI).AI_changeMemoryCount(eToPlayer, MEMORY_HIRED_WAR_ALLY, 1);
				}
			}
		}
		break;

	case TRADE_EMBARGO:
		GET_PLAYER(eFromPlayer).stopTradingWithTeam((TeamTypes)trade.m_iData);

		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if (GET_PLAYER((PlayerTypes)iI).getTeam() == ((TeamTypes)trade.m_iData))
				{
					GET_PLAYER((PlayerTypes)iI).AI_changeMemoryCount(eToPlayer, MEMORY_HIRED_TRADE_EMBARGO, 1);
				}
			}
		}
		break;

	case TRADE_CIVIC:
		paeNewCivics = new CivicTypes[GC.getNumCivicOptionInfos()];

		for (iI = 0; iI < GC.getNumCivicOptionInfos(); iI++)
		{
			paeNewCivics[iI] = GET_PLAYER(eFromPlayer).getCivics((CivicOptionTypes)iI);
		}

		paeNewCivics[GC.getCivicInfo((CivicTypes)trade.m_iData).getCivicOptionType()] = ((CivicTypes)trade.m_iData);

		GET_PLAYER(eFromPlayer).revolution(paeNewCivics, true);

		if (GET_PLAYER(eFromPlayer).AI_getCivicTimer() < GC.getDefineINT("PEACE_TREATY_LENGTH"))
		{
			GET_PLAYER(eFromPlayer).AI_setCivicTimer(GC.getDefineINT("PEACE_TREATY_LENGTH"));
		}

		SAFE_DELETE_ARRAY(paeNewCivics);
		break;

	case TRADE_RELIGION:
		GET_PLAYER(eFromPlayer).convert((ReligionTypes)trade.m_iData);

		if (GET_PLAYER(eFromPlayer).AI_getReligionTimer() < GC.getDefineINT("PEACE_TREATY_LENGTH"))
		{
			GET_PLAYER(eFromPlayer).AI_setReligionTimer(GC.getDefineINT("PEACE_TREATY_LENGTH"));
		}
		break;

	case TRADE_OPEN_BORDERS:
		if (trade.m_iData == 0)
		{
			startTeamTrade(TRADE_OPEN_BORDERS, GET_PLAYER(eFromPlayer).getTeam(), GET_PLAYER(eToPlayer).getTeam(), true);
			GET_TEAM(GET_PLAYER(eFromPlayer).getTeam()).setOpenBorders(((TeamTypes)(GET_PLAYER(eToPlayer).getTeam())), true);
		}
		else
		{
			bSave = true;
		}
		break;

	case TRADE_DEFENSIVE_PACT:
		if (trade.m_iData == 0)
		{
			startTeamTrade(TRADE_DEFENSIVE_PACT, GET_PLAYER(eFromPlayer).getTeam(), GET_PLAYER(eToPlayer).getTeam(), true);
			GET_TEAM(GET_PLAYER(eFromPlayer).getTeam()).setDefensivePact(((TeamTypes)(GET_PLAYER(eToPlayer).getTeam())), true);
		}
		else
		{
			bSave = true;
		}
		break;

	case TRADE_PERMANENT_ALLIANCE:
		break;

	case TRADE_PEACE_TREATY:
		GET_TEAM(GET_PLAYER(eFromPlayer).getTeam()).setForcePeace(((TeamTypes)(GET_PLAYER(eToPlayer).getTeam())), true);
		bSave = true;
		break;

	default:
		FAssert(false);
		break;
	}

	return bSave;
}


void CvDeal::endTrade(TradeData trade, PlayerTypes eFromPlayer, PlayerTypes eToPlayer, bool bTeam)
{
	int iI, iJ;

	switch (trade.m_eItemType)
	{
	case TRADE_TECHNOLOGIES:
		FAssert(false);
		break;

	case TRADE_RESOURCES:
		GET_PLAYER(eToPlayer).changeBonusImport(((BonusTypes)trade.m_iData), -1);
		GET_PLAYER(eFromPlayer).changeBonusExport(((BonusTypes)trade.m_iData), -1);
		break;

	case TRADE_CITIES:
	case TRADE_GOLD:
		FAssert(false);
		break;

	case TRADE_GOLD_PER_TURN:
		GET_PLAYER(eFromPlayer).changeGoldPerTurnByPlayer(eToPlayer, trade.m_iData);
		GET_PLAYER(eToPlayer).changeGoldPerTurnByPlayer(eFromPlayer, -(trade.m_iData));
		break;

	case TRADE_MAPS:
	case TRADE_PEACE:
	case TRADE_WAR:
	case TRADE_EMBARGO:
	case TRADE_CIVIC:
	case TRADE_RELIGION:
		FAssert(false);
		break;

	case TRADE_VASSAL:
		GET_TEAM(GET_PLAYER(eFromPlayer).getTeam()).setVassal(((TeamTypes)(GET_PLAYER(eToPlayer).getTeam())), false, false);
		if (bTeam)
		{
			endTeamTrade(TRADE_VASSAL, GET_PLAYER(eFromPlayer).getTeam(), GET_PLAYER(eToPlayer).getTeam());
		}
		break;

	case TRADE_SURRENDER:
		GET_TEAM(GET_PLAYER(eFromPlayer).getTeam()).setVassal(((TeamTypes)(GET_PLAYER(eToPlayer).getTeam())), false, true);
		if (bTeam)
		{
			endTeamTrade(TRADE_SURRENDER, GET_PLAYER(eFromPlayer).getTeam(), GET_PLAYER(eToPlayer).getTeam());
		}
		break;

	case TRADE_OPEN_BORDERS:
		GET_TEAM(GET_PLAYER(eFromPlayer).getTeam()).setOpenBorders(((TeamTypes)(GET_PLAYER(eToPlayer).getTeam())), false);
		if (bTeam)
		{
			endTeamTrade(TRADE_OPEN_BORDERS, GET_PLAYER(eFromPlayer).getTeam(), GET_PLAYER(eToPlayer).getTeam());
		}

		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if (GET_PLAYER((PlayerTypes)iI).getTeam() == GET_PLAYER(eToPlayer).getTeam())
				{
					for (iJ = 0; iJ < MAX_PLAYERS; iJ++)
					{
						if (GET_PLAYER((PlayerTypes)iJ).isAlive())
						{
							if (GET_PLAYER((PlayerTypes)iJ).getTeam() == GET_PLAYER(eFromPlayer).getTeam())
							{
								GET_PLAYER((PlayerTypes)iI).AI_changeMemoryCount(((PlayerTypes)iJ), MEMORY_CANCELLED_OPEN_BORDERS, 1);
							}
						}
					}
				}
			}
		}
		break;

	case TRADE_DEFENSIVE_PACT:
		GET_TEAM(GET_PLAYER(eFromPlayer).getTeam()).setDefensivePact(((TeamTypes)(GET_PLAYER(eToPlayer).getTeam())), false);
		if (bTeam)
		{
			endTeamTrade(TRADE_DEFENSIVE_PACT, GET_PLAYER(eFromPlayer).getTeam(), GET_PLAYER(eToPlayer).getTeam());
		}
		break;

	case TRADE_PERMANENT_ALLIANCE:
		FAssert(false);
		break;

	case TRADE_PEACE_TREATY:
		GET_TEAM(GET_PLAYER(eFromPlayer).getTeam()).setForcePeace(((TeamTypes)(GET_PLAYER(eToPlayer).getTeam())), false);
		break;

	default:
		FAssert(false);
		break;
	}
}

void CvDeal::startTeamTrade(TradeableItems eItem, TeamTypes eFromTeam, TeamTypes eToTeam, bool bDual)
{
	for (int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		CvPlayer& kLoopFromPlayer = GET_PLAYER((PlayerTypes)iI);
		if (kLoopFromPlayer.isAlive() )
		{
			if (kLoopFromPlayer.getTeam() == eFromTeam)
			{
				for (int iJ = 0; iJ < MAX_PLAYERS; iJ++)
				{
					CvPlayer& kLoopToPlayer = GET_PLAYER((PlayerTypes)iJ);
					if (kLoopToPlayer.isAlive())
					{
						if (kLoopToPlayer.getTeam() == eToTeam)
						{
							TradeData item;
							setTradeItem(&item, eItem, 1);
							CLinkList<TradeData> ourList;
							ourList.insertAtEnd(item);
							CLinkList<TradeData> theirList;
							if (bDual)
							{
								theirList.insertAtEnd(item);
							}
							GC.getGame().implementDeal((PlayerTypes)iI, (PlayerTypes)iJ, &ourList, &theirList);
						}
					}
				}
			}
		}
	}
}

void CvDeal::endTeamTrade(TradeableItems eItem, TeamTypes eFromTeam, TeamTypes eToTeam)
{
	CvDeal* pLoopDeal;
	int iLoop;
	CLLNode<TradeData>* pNode;

	for (pLoopDeal = GC.getGameINLINE().firstDeal(&iLoop); pLoopDeal != NULL; pLoopDeal = GC.getGameINLINE().nextDeal(&iLoop))
	{
		if (pLoopDeal != this)
		{
			bool bValid = true;

			if (GET_PLAYER(pLoopDeal->getFirstPlayer()).getTeam() == eFromTeam && GET_PLAYER(pLoopDeal->getSecondPlayer()).getTeam() == eToTeam)
			{
				if (pLoopDeal->getFirstTrades())
				{
					for (pNode = pLoopDeal->getFirstTrades()->head(); pNode; pNode = pLoopDeal->getFirstTrades()->next(pNode))
					{
						if (pNode->m_data.m_eItemType == eItem)
						{
							bValid = false;
						}
					}
				}
			}

			if (bValid && GET_PLAYER(pLoopDeal->getFirstPlayer()).getTeam() == eToTeam && GET_PLAYER(pLoopDeal->getSecondPlayer()).getTeam() == eFromTeam)
			{
				if (pLoopDeal->getSecondTrades() != NULL)
				{
					for (pNode = pLoopDeal->getSecondTrades()->head(); pNode; pNode = pLoopDeal->getSecondTrades()->next(pNode))
					{
						if (pNode->m_data.m_eItemType == eItem)
						{
							bValid = false;
						}
					}
				}
			}

			if (!bValid)
			{
				pLoopDeal->kill(false);
			}
		}
	}
}

bool CvDeal::isCancelable(PlayerTypes eByPlayer, CvWString* pszReason)
{
	if (isUncancelableVassalDeal(eByPlayer, pszReason))
	{
		return false;
	}

	int iTurns = turnsToCancel(eByPlayer);
	if (pszReason)
	{
		if (iTurns > 0)
		{
			*pszReason = gDLL->getText("TXT_KEY_MISC_DEAL_NO_CANCEL", iTurns);
		}
	}

	return (iTurns <= 0);
}

int CvDeal::turnsToCancel(PlayerTypes eByPlayer)
{
	return (getInitialGameTurn() + GC.getDefineINT("PEACE_TREATY_LENGTH") - GC.getGameINLINE().getGameTurn());
}
