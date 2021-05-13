// selectionGroupAI.cpp

#include "CvGameCoreDLL.h"
#include "CvSelectionGroupAI.h"
#include "CvPlayerAI.h"
#include "CvMap.h"
#include "CvPlot.h"
#include "CvTeamAI.h"
#include "CvDLLEntityIFaceBase.h"
#include "CvGameCoreUtils.h"
#include "FProfiler.h"
#include "CVInfos.h"
#include "CvTradeRoute.h"

// Public Functions...

CvSelectionGroupAI::CvSelectionGroupAI()
{
	AI_reset();
}


CvSelectionGroupAI::~CvSelectionGroupAI()
{
	AI_uninit();
}


void CvSelectionGroupAI::AI_init()
{
	AI_reset();

	//--------------------------------
	// Init other game data
}


void CvSelectionGroupAI::AI_uninit()
{
}


void CvSelectionGroupAI::AI_reset()
{
	AI_uninit();

	m_iMissionAIX = INVALID_PLOT_COORD;
	m_iMissionAIY = INVALID_PLOT_COORD;

	m_bForceSeparate = false;

	m_eMissionAIType = NO_MISSIONAI;

	m_missionAIUnit.reset();

	m_bGroupAttack = false;
	m_iGroupAttackX = -1;
	m_iGroupAttackY = -1;
}


void CvSelectionGroupAI::AI_separate()
{
	CLLNode<IDInfo>* pEntityNode;
	CvUnit* pLoopUnit;

	pEntityNode = headUnitNode();

	while (pEntityNode != NULL)
	{
		pLoopUnit = ::getUnit(pEntityNode->m_data);
		pEntityNode = nextUnitNode(pEntityNode);

		pLoopUnit->joinGroup(NULL);
		if (pLoopUnit->plot()->getTeam() == getTeam())
		{
			pLoopUnit->getGroup()->pushMission(MISSION_SKIP);
		}
	}
}

void CvSelectionGroupAI::AI_seperateNonAI(UnitAITypes eUnitAI)
{
	CLLNode<IDInfo>* pEntityNode;
	CvUnit* pLoopUnit;

	pEntityNode = headUnitNode();

	while (pEntityNode != NULL)
	{
		pLoopUnit = ::getUnit(pEntityNode->m_data);
		pEntityNode = nextUnitNode(pEntityNode);
		if (pLoopUnit->AI_getUnitAIType() != eUnitAI)
		{
			pLoopUnit->joinGroup(NULL);
			if (pLoopUnit->plot()->getTeam() == getTeam())
			{
				pLoopUnit->getGroup()->pushMission(MISSION_SKIP);
			}
		}
	}
}

void CvSelectionGroupAI::AI_seperateAI(UnitAITypes eUnitAI)
{
	CLLNode<IDInfo>* pEntityNode;
	CvUnit* pLoopUnit;

	pEntityNode = headUnitNode();

	while (pEntityNode != NULL)
	{
		pLoopUnit = ::getUnit(pEntityNode->m_data);
		pEntityNode = nextUnitNode(pEntityNode);
		if (pLoopUnit->AI_getUnitAIType() == eUnitAI)
		{
			pLoopUnit->joinGroup(NULL);
			if (plot()->getTeam() == getTeam())
			{
				pLoopUnit->getGroup()->pushMission(MISSION_SKIP);
			}
		}
	}
}
// Returns true if the group has become busy...
bool CvSelectionGroupAI::AI_update()
{
	CLLNode<IDInfo>* pEntityNode;
	CvUnit* pLoopUnit;
	bool bDead;
	bool bFollow;

	PROFILE_FUNC();

	FAssert(getOwnerINLINE() != NO_PLAYER);

	if (!AI_isControlled())
	{
		return false;
	}

	if (getNumUnits() == 0)
	{
		return false;
	}

	if (isForceUpdate())
	{
		clearMissionQueue(); // XXX ???
		setActivityType(ACTIVITY_AWAKE);
		setForceUpdate(false);

		// if we are in the middle of attacking with a stack, cancel it
		AI_cancelGroupAttack();
	}

	FAssert(!(GET_PLAYER(getOwnerINLINE()).isAutoMoves()));

	int iTempHack = 0; // XXX

	bDead = false;

	bool bFailedAlreadyFighting = false;
	while ((m_bGroupAttack && !bFailedAlreadyFighting) || readyToMove())
	{
		iTempHack++;
		if (iTempHack > 100)
		{
			FAssert(false);
			CvUnit* pHeadUnit = getHeadUnit();
			if (NULL != pHeadUnit)
			{
				if (GC.getLogging())
				{
					TCHAR szOut[1024];
					CvWString szTempString;
					getUnitAIString(szTempString, pHeadUnit->AI_getUnitAIType());
					sprintf(szOut, "Unit stuck in loop: %S(%S)[%d, %d] (%S)", pHeadUnit->getName().GetCString(), GET_PLAYER(pHeadUnit->getOwnerINLINE()).getName(),
						pHeadUnit->getX_INLINE(), pHeadUnit->getY_INLINE(), szTempString.GetCString());
					gDLL->messageControlLog(szOut);
				}

				pHeadUnit->finishMoves();
			}
			break;
		}

		// if we want to force the group to attack, force another attack
		if (m_bGroupAttack)
		{
			m_bGroupAttack = false;

			groupAttack(m_iGroupAttackX, m_iGroupAttackY, MOVE_DIRECT_ATTACK, bFailedAlreadyFighting);
		}
		// else pick AI action
		else
		{
			CvUnit* pHeadUnit = getHeadUnit();

			if (pHeadUnit == NULL || pHeadUnit->isDelayedDeath())
			{
				break;
			}

			resetPath();

			if (pHeadUnit->AI_update())
			{
				// AI_update returns true when we should abort the loop and wait until next slice
				break;
			}
		}

		if (doDelayedDeath())
		{
			bDead = true;
			break;
		}

		// if no longer group attacking, and force separate is true, then bail, decide what to do after group is split up
		// (UnitAI of head unit may have changed)
		if (!m_bGroupAttack && AI_isForceSeparate())
		{
			AI_separate();	// pointers could become invalid...
			return true;
		}
	}

	if (!bDead)
	{
		CvUnit* pHeadUnit = getHeadUnit();
		if (pHeadUnit != NULL)
		{
			if ((pHeadUnit->getUnitTravelState() == UNIT_TRAVEL_STATE_IN_EUROPE) && AI_isControlled())
			{
				pEntityNode = headUnitNode();

				while (pEntityNode != NULL)
				{
					pLoopUnit = ::getUnit(pEntityNode->m_data);
					pEntityNode = nextUnitNode(pEntityNode);

					pLoopUnit->AI_europeUpdate();
				}
			}
		}

		if (!isHuman())
		{
			bFollow = false;

			// if we not group attacking, then check for follow action
			if (!m_bGroupAttack)
			{
				pEntityNode = headUnitNode();

				while ((pEntityNode != NULL) && readyToMove(true))
				{
					pLoopUnit = ::getUnit(pEntityNode->m_data);
					pEntityNode = nextUnitNode(pEntityNode);

					if (pLoopUnit->canMove())
					{
						resetPath();

						if (pLoopUnit->AI_follow())
						{
							bFollow = true;
							break;
						}
					}
				}
			}

			if (doDelayedDeath())
			{
				bDead = true;
			}

			if (!bDead)
			{
				if (!bFollow && readyToMove(true))
				{
					pushMission(MISSION_SKIP);
				}
			}
		}
	}

	if (bDead)
	{
		return true;
	}

	return (isBusy() || isCargoBusy());
}


// Returns attack odds out of 100 (the higher, the better...)
int CvSelectionGroupAI::AI_attackOdds(const CvPlot* pPlot, bool bPotentialEnemy) const
{
	CvUnit* pAttacker;

	FAssert(getOwnerINLINE() != NO_PLAYER);
	
	int iOdds = 0;
	pAttacker = AI_getBestGroupAttacker(pPlot, bPotentialEnemy, iOdds);

	if (pAttacker == NULL)
	{
		return 0;
	}

	if (pPlot->getBestDefender(NO_PLAYER, getOwnerINLINE(), pAttacker, !bPotentialEnemy, bPotentialEnemy) == NULL)
	{
		return 100;
	}

	return iOdds;
}


CvUnit* CvSelectionGroupAI::AI_getBestGroupAttacker(const CvPlot* pPlot, bool bPotentialEnemy, int& iUnitOdds, bool bForce, bool bNoBlitz) const
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvUnit* pBestUnit;
	int iValue;
	int iBestValue;
	int iOdds;
	int iBestOdds;

	iBestValue = 0;
	iBestOdds = 0;
	pBestUnit = NULL;

	pUnitNode = headUnitNode();

	bool bIsHuman = (pUnitNode != NULL) ? GET_PLAYER(::getUnit(pUnitNode->m_data)->getOwnerINLINE()).isHuman() : true;

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if (!pLoopUnit->isDead())
		{
			bool bCanAttack = false;
			bCanAttack = pLoopUnit->canAttack();

			if (bCanAttack && bNoBlitz && pLoopUnit->isBlitz() && pLoopUnit->isMadeAttack())
			{
				bCanAttack = false;
			}

			if (bCanAttack)
			{
				if (bForce || pLoopUnit->canMove())
				{
					if (bForce || pLoopUnit->canMoveInto(pPlot, /*bAttack*/ true, /*bDeclareWar*/ bPotentialEnemy))
					{
						iOdds = pLoopUnit->AI_attackOdds(pPlot, bPotentialEnemy);

						iValue = iOdds;
						FAssertMsg(iValue > 0, "iValue is expected to be greater than 0");

						// if non-human, prefer the last unit that has the best value (so as to avoid splitting the group)
						if (iValue > iBestValue || (!bIsHuman && iValue > 0 && iValue == iBestValue))
						{
							iBestValue = iValue;
							iBestOdds = iOdds;
							pBestUnit = pLoopUnit;
						}
					}
				}
			}
		}
	}

	iUnitOdds = iBestOdds;
	return pBestUnit;
}

CvUnit* CvSelectionGroupAI::AI_getBestGroupSacrifice(const CvPlot* pPlot, bool bPotentialEnemy, bool bForce, bool bNoBlitz) const
{
	int iBestValue = 0;
	CvUnit* pBestUnit = NULL;

	CLLNode<IDInfo>* pUnitNode = headUnitNode();
	while (pUnitNode != NULL)
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if (!pLoopUnit->isDead())
		{
			bool bCanAttack = false;
			bCanAttack = pLoopUnit->canAttack();

			if (bCanAttack && bNoBlitz && pLoopUnit->isBlitz() && pLoopUnit->isMadeAttack())
			{
				bCanAttack = false;
			}

			if (bCanAttack)
			{
				if (bForce || pLoopUnit->canMove())
				{
					if (bForce || pLoopUnit->canMoveInto(pPlot, true))
					{
                        int iValue = pLoopUnit->AI_sacrificeValue(pPlot);
						FAssertMsg(iValue > 0, "iValue is expected to be greater than 0");

						// we want to pick the last unit of highest value, so pick the last unit with a good value
						if (iValue >= iBestValue)
						{
							iBestValue = iValue;
							pBestUnit = pLoopUnit;
						}
					}
				}
			}
		}
	}

	return pBestUnit;
}

// Returns ratio of strengths of stacks times 100
// (so 100 is even ratio, numbers over 100 mean this group is more powerful than the stack on a plot)
int CvSelectionGroupAI::AI_compareStacks(const CvPlot* pPlot, bool bPotentialEnemy, bool bCheckCanAttack, bool bCheckCanMove) const
{
	FAssert(pPlot != NULL);

	int	compareRatio;
	DomainTypes eDomainType = getDomainType();

	// choose based on the plot, not the head unit (mainly for transport carried units)
	if (pPlot->isWater())
		eDomainType = DOMAIN_SEA;
	else
		eDomainType = DOMAIN_LAND;

	compareRatio = AI_sumStrength(pPlot, eDomainType, bCheckCanAttack, bCheckCanMove);
	compareRatio *= 100;

	PlayerTypes eOwner = getOwnerINLINE();
	if (eOwner == NO_PLAYER)
	{
		eOwner = getHeadOwner();
	}
	FAssert(eOwner != NO_PLAYER);

	int defenderSum = pPlot->AI_sumStrength(NO_PLAYER, getOwnerINLINE(), eDomainType, true, !bPotentialEnemy, bPotentialEnemy);
	compareRatio /= std::max(1, defenderSum);

	return compareRatio;
}

int CvSelectionGroupAI::AI_sumStrength(const CvPlot* pAttackedPlot, DomainTypes eDomainType, bool bCheckCanAttack, bool bCheckCanMove) const
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	int	strSum = 0;

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if (!pLoopUnit->isDead())
		{
			bool bCanAttack = pLoopUnit->canAttack();

			if (!bCheckCanAttack || bCanAttack)
			{
				if (!bCheckCanMove || pLoopUnit->canMove())
					if (!bCheckCanMove || pAttackedPlot == NULL || pLoopUnit->canMoveInto(pAttackedPlot, /*bAttack*/ true, /*bDeclareWar*/ true))
						if (eDomainType == NO_DOMAIN || pLoopUnit->getDomainType() == eDomainType)
							strSum += pLoopUnit->currEffectiveStr(pAttackedPlot, pLoopUnit);
			}
		}
	}

	return strSum;
}

void CvSelectionGroupAI::AI_queueGroupAttack(int iX, int iY)
{
	m_bGroupAttack = true;

	m_iGroupAttackX = iX;
	m_iGroupAttackY = iY;
}

inline void CvSelectionGroupAI::AI_cancelGroupAttack()
{
	m_bGroupAttack = false;
}

inline bool CvSelectionGroupAI::AI_isGroupAttack()
{
	return m_bGroupAttack;
}

bool CvSelectionGroupAI::AI_isControlled()
{
	return (!isHuman() || isAutomated());
}


bool CvSelectionGroupAI::AI_isDeclareWar(const CvPlot* pPlot)
{
	FAssert(getHeadUnit() != NULL);

	if (isHuman())
	{
		return false;
	}
	else
	{
		bool bLimitedWar = false;
		if (pPlot != NULL)
		{
			TeamTypes ePlotTeam = pPlot->getTeam();
			if (ePlotTeam != NO_TEAM)
			{
				WarPlanTypes eWarplan = GET_TEAM(getTeam()).AI_getWarPlan(ePlotTeam);
				if (eWarplan == WARPLAN_LIMITED)
				{
					bLimitedWar = true;
				}
			}
		}

		CvUnit* pHeadUnit = getHeadUnit();

		if (pHeadUnit != NULL)
		{
			switch (pHeadUnit->AI_getUnitAIType())
			{
			case UNITAI_UNKNOWN:
			case UNITAI_COLONIST:
			case UNITAI_SETTLER:
			case UNITAI_WORKER:
			case UNITAI_MISSIONARY:
			case UNITAI_SCOUT:
			case UNITAI_WAGON:
			case UNITAI_TREASURE:
			case UNITAI_YIELD:
			case UNITAI_GENERAL:
				return false;
				break;

			case UNITAI_DEFENSIVE:
			case UNITAI_OFFENSIVE:
			case UNITAI_COUNTER:
				return true;
				break;

			case UNITAI_TRANSPORT_SEA:
				return false;
				break;

			case UNITAI_ASSAULT_SEA:
			case UNITAI_COMBAT_SEA:
			case UNITAI_PIRATE_SEA:
				return true;
				break;

			default:
				FAssert(false);
				break;
			}
		}
	}

	return false;
}


CvPlot* CvSelectionGroupAI::AI_getMissionAIPlot()
{
	return GC.getMapINLINE().plotSorenINLINE(m_iMissionAIX, m_iMissionAIY);
}


bool CvSelectionGroupAI::AI_isForceSeparate()
{
	return m_bForceSeparate;
}


void CvSelectionGroupAI::AI_makeForceSeparate()
{
	m_bForceSeparate = true;
}


MissionAITypes CvSelectionGroupAI::AI_getMissionAIType()
{
	return m_eMissionAIType;
}


void CvSelectionGroupAI::AI_setMissionAI(MissionAITypes eNewMissionAI, CvPlot* pNewPlot, CvUnit* pNewUnit)
{
	m_eMissionAIType = eNewMissionAI;

	if (pNewPlot != NULL)
	{
		m_iMissionAIX = pNewPlot->getX_INLINE();
		m_iMissionAIY = pNewPlot->getY_INLINE();
	}
	else
	{
		m_iMissionAIX = INVALID_PLOT_COORD;
		m_iMissionAIY = INVALID_PLOT_COORD;
	}

	if (pNewUnit != NULL)
	{
		m_missionAIUnit = pNewUnit->getIDInfo();
	}
	else
	{
		m_missionAIUnit.reset();
	}
}


CvUnit* CvSelectionGroupAI::AI_getMissionAIUnit()
{
	return getUnit(m_missionAIUnit);
}

bool CvSelectionGroupAI::AI_isFull()
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;

	if (getNumUnits() > 0)
	{
		UnitAITypes eUnitAI = getHeadUnitAI();
		// do two passes, the first pass, we ignore units with speical cargo
		int iSpecialCargoCount = 0;
		int iCargoCount = 0;

		// first pass, count but ignore special cargo units
		pUnitNode = headUnitNode();

		while (pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = nextUnitNode(pUnitNode);
			if (pLoopUnit->AI_getUnitAIType() == eUnitAI)
			{
				if (pLoopUnit->cargoSpace() > 0)
				{
					iCargoCount++;
				}

				if (pLoopUnit->specialCargo() != NO_SPECIALUNIT)
				{
					iSpecialCargoCount++;
				}
				else if (!(pLoopUnit->isFull()))
				{
					return false;
				}
			}
		}

		// if every unit in the group has special cargo, then check those, otherwise, consider ourselves full
		if (iSpecialCargoCount >= iCargoCount)
		{
			pUnitNode = headUnitNode();
			while (pUnitNode != NULL)
			{
				pLoopUnit = ::getUnit(pUnitNode->m_data);
				pUnitNode = nextUnitNode(pUnitNode);

				if (pLoopUnit->AI_getUnitAIType() == eUnitAI)
				{
					if (!(pLoopUnit->isFull()))
					{
						return false;
					}
				}
			}
		}

		return true;
	}

	return false;
}


bool CvSelectionGroupAI::AI_launchAssault(CvPlot* pTargetCityPlot)
{
	std::multimap<int, CvUnit*, std::greater<int> > units;
	std::multimap<int, CvUnit*, std::greater<int> >::iterator units_it;

	CLLNode<IDInfo>* pUnitNode = plot()->headUnitNode();
	CvUnit* pLoopUnit;

    while (pUnitNode != NULL)
    {
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = plot()->nextUnitNode(pUnitNode);

        if (pLoopUnit->isCargo())
        {
            if (pLoopUnit->getTransportUnit()->getGroup() == this)
            {
                int iValue = pLoopUnit->baseCombatStr();
                if (pLoopUnit->canAttack())
                {
                    iValue *= 10;
                }
                iValue *= 100;

                units.insert(std::make_pair(iValue, pLoopUnit));
            }
        }
    }

    if (units.empty())
    {
        return false;
    }

    bool bAction = false;
    for (units_it = units.begin(); units_it != units.end(); ++units_it)
    {
        pLoopUnit = units_it->second;
		if (pLoopUnit->canMove() && pLoopUnit->canAttack())
		{
//			if (pLoopUnit->AI_attackFromTransport(NULL, 40, 80))
//			{
//			    bAction = true;
//			}
            int iPriority = 41;
            pLoopUnit->AI_setMovePriority(iPriority);
		}
    }
//    for (units_it = units.begin(); units_it != units.end(); ++units_it)
//    {
//        pLoopUnit = units_it->second;
//        if (pLoopUnit->canMove())
//        {
//            if (pLoopUnit->AI_moveFromTransport(NULL))
//            {
//                bAction = true;
//            }
//        }
//    }
//    for (units_it = units.begin(); units_it != units.end(); ++units_it)
//    {
//        pLoopUnit = units_it->second;
//		if (pLoopUnit->canMove() && pLoopUnit->canAttack())
//		{
//			if (pLoopUnit->AI_attackFromTransport(NULL, 0, 100))
//			{
//			    bAction = true;
//			}
//		}
//    }
    if (bAction)
    {
	    //pushMission(MISSION_SKIP);
	    return true;
    }

    return false;

}

void CvSelectionGroupAI::AI_groupBombard()
{
 	CLLNode<IDInfo>* pEntityNode = headUnitNode();
	CvUnit* pLoopUnit = NULL;

	while (pEntityNode != NULL)
	{
		pLoopUnit = ::getUnit(pEntityNode->m_data);
		pEntityNode = nextUnitNode(pEntityNode);

		if (pLoopUnit->canBombard(plot()))
		{
		    pLoopUnit->bombard();
		}
	}
}

//The return value is the amount of treasure on board.
int CvSelectionGroupAI::AI_getYieldsLoaded(short* piYields)
{
	if (piYields != NULL)
	{
		for (int i = 0; i < NUM_YIELD_TYPES; ++i)
		{
			piYields[i] = 0;
		}
	}
	int iAmount = 0;

	CLinkList<IDInfo> unitList;
	buildCargoUnitList(unitList);

	CLLNode<IDInfo>* pUnitNode = unitList.head();
	while (pUnitNode != NULL)
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = unitList.next(pUnitNode);

		if (pLoopUnit->getYieldStored() > 0)
		{
			if (pLoopUnit->getUnitInfo().isTreasure())
			{
				iAmount += pLoopUnit->getYieldStored();
			}
			else if (pLoopUnit->getYield() != NO_YIELD)
			{
				if (piYields != NULL)
				{
					piYields[pLoopUnit->getYield()] += pLoopUnit->getYieldStored();
				}
			}
		}
	}
	return iAmount;
}

bool CvSelectionGroupAI::AI_tradeRoutes()
{
	PROFILE_FUNC();

	const IDInfo kEurope(getOwnerINLINE(), CvTradeRoute::EUROPE_CITY_ID);

	CvCity* pPlotCity = plot()->getPlotCity();
	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
	std::set<int>::iterator it;

	std::map<IDInfo, int> cityValues;

	std::vector<CvTradeRoute*> routes;
	std::vector<int> routeValues;

	std::vector<bool> yieldsDelivered(NUM_YIELD_TYPES, false);
	std::vector<bool> yieldsToUnload(NUM_YIELD_TYPES, false);
	std::vector<int> yieldsOnBoard(NUM_YIELD_TYPES, false);

	if (!isHuman() || (getAutomateType() == AUTOMATE_TRANSPORT_FULL))
	{
		std::vector<CvTradeRoute*> aiRoutes;
		kOwner.getTradeRoutes(aiRoutes);
		for (uint i = 0; i < aiRoutes.size(); ++i)
		{
			CvTradeRoute* pRoute = aiRoutes[i];
			CvCity* pSourceCity = ::getCity(pRoute->getSourceCity());
			CvArea* pSourceWaterArea = pSourceCity->waterArea();
			if ((pSourceCity != NULL) && ((getDomainType() != DOMAIN_SEA) || (pSourceWaterArea != NULL)))
			{
				int iSourceArea = (getDomainType() == DOMAIN_SEA) ? pSourceWaterArea->getID() : pSourceCity->getArea();
				if (getDomainType() == DOMAIN_SEA ? plot()->isAdjacentToArea(iSourceArea) : (iSourceArea == getArea()))
				{
					if ((getDomainType() == DOMAIN_SEA) || (pRoute->getDestinationCity() != kEurope))
					{
						routes.push_back(pRoute);
						routeValues.push_back(0);

						yieldsDelivered[pRoute->getYield()] = true;

						if (pPlotCity != NULL && ::getCity(pRoute->getDestinationCity()) == pPlotCity)
						{
							yieldsToUnload[pRoute->getYield()] = true;
						}


						cityValues[pRoute->getSourceCity()] = 0;
						cityValues[pRoute->getDestinationCity()] = 0;
					}
				}
			}
		}
	}
	else
	{
		for (it = m_aTradeRoutes.begin(); it != m_aTradeRoutes.end(); ++it)
		{
			CvTradeRoute* pRoute = kOwner.getTradeRoute(*it);
			CvCity* pSourceCity = ::getCity(pRoute->getSourceCity());
			if (pSourceCity != NULL)
			{
				CvArea* pSourceWaterArea = pSourceCity->waterArea();
				if (getDomainType() != DOMAIN_SEA || pSourceWaterArea != NULL)
				{
					int iSourceArea = (getDomainType() == DOMAIN_SEA) ? pSourceWaterArea->getID() : pSourceCity->getArea();
					if (getDomainType() == DOMAIN_SEA ? plot()->isAdjacentToArea(iSourceArea) : (iSourceArea == getArea()))
					{
						if ((getDomainType() == DOMAIN_SEA) || (pRoute->getDestinationCity() != kEurope))
						{
							routes.push_back(pRoute);
							routeValues.push_back(0);

							yieldsDelivered[pRoute->getYield()] = true;

							if (pPlotCity != NULL && ::getCity(pRoute->getDestinationCity()) == pPlotCity)
							{
								yieldsToUnload[pRoute->getYield()] = true;
							}


							cityValues[pRoute->getSourceCity()] = 0;
							cityValues[pRoute->getDestinationCity()] = 0;
						}
					}
					else
					{
						FAssertMsg(false, "Unexpected : Unit can't run trade route it's assigned to");
					}

				}
			}
		}


	}

	if ((pPlotCity != NULL) && hasCargo())
	{
		std::vector<CvUnit*> units;

		//Unload everything which we should unload here, or can't unload anywhere...
		CLLNode<IDInfo>* pUnitNode = plot()->headUnitNode();
		while (pUnitNode != NULL)
		{
			CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = plot()->nextUnitNode(pUnitNode);

			if (pLoopUnit != NULL)
			{
				YieldTypes eYield = pLoopUnit->getYield();
				CvUnit* pTransport = pLoopUnit->getTransportUnit();

				if ((eYield != NO_YIELD) && pTransport != NULL && (yieldsToUnload[eYield] || !(yieldsDelivered[eYield])))
				{
					if (pTransport->getGroup() == this && pLoopUnit->canUnload())
					{
						units.push_back(pLoopUnit);
					}
				}
			}
		}

		for (uint i = 0; i < units.size(); ++i)
		{
			units[i]->unload();
		}
	}

	short aiYieldsLoaded[NUM_YIELD_TYPES];
	AI_getYieldsLoaded(aiYieldsLoaded);

	bool bNoCargo = true;
	for (int i = 0; i < NUM_YIELD_TYPES; ++i)
	{
		if (aiYieldsLoaded[i] > 0)
		{
			bNoCargo = false;
			break;
		}
	}

	if (!bNoCargo)
	{
		//We need to iterate over every destination city and see if we can unload.
		for (uint i = 0; i < routes.size(); ++i)
		{
			CvCity* pDestinationCity = ::getCity(routes[i]->getDestinationCity());
			if ((pDestinationCity == NULL) || (pDestinationCity != pPlotCity))
			{
				int iRouteValue = kOwner.AI_transferYieldValue(routes[i]->getDestinationCity(), routes[i]->getYield(), aiYieldsLoaded[routes[i]->getYield()]);

				if (iRouteValue > 0)
				{
					cityValues[routes[i]->getDestinationCity()] += iRouteValue;
					routeValues[i] += iRouteValue;
				}
			}
		}
	}

	//We need to iterate over every source city, and see if there's anything which needs moving to the respective destination city.
	//We apply some bias to the city we are presently at, but not too much - sometimes empty runs need to be made...
	//Basically this looks at the entire NEXT trade run (source-city to dest-city), with some bias given towards
	//starting it from pPlotCity as sourceCity.
	//If we are carrying cargo, only count cities where we can unload.
	for (uint i = 0; i < routes.size(); ++i)
	{
		CvCity* pSourceCity = ::getCity(routes[i]->getSourceCity());

		if ((pSourceCity != NULL) && (bNoCargo || (cityValues[routes[i]->getSourceCity()] > 0)))
		{
			CvCity* pDestinationCity = ::getCity(routes[i]->getDestinationCity());
			YieldTypes eYield = routes[i]->getYield();

			int iAmount = pSourceCity->getYieldStored(eYield) - pSourceCity->getMaintainLevel(eYield);

			if (iAmount > 0)
			{

				int iExportValue = kOwner.AI_transferYieldValue(routes[i]->getSourceCity(), routes[i]->getYield(), -iAmount);
				int iImportValue = kOwner.AI_transferYieldValue(routes[i]->getDestinationCity(), routes[i]->getYield(), iAmount);
				int iRouteValue = (iExportValue + iImportValue + 2 * std::min(iExportValue, iImportValue)) / 4;

				if (pSourceCity == pPlotCity)
				{
					cityValues[routes[i]->getDestinationCity()] += 2 * iRouteValue;
				}
				else
				{
					cityValues[routes[i]->getSourceCity()] += iRouteValue;
				}

				routeValues[i] = iRouteValue;
			}
		}
	}

	IDInfo kBestDestination(NO_PLAYER, -1);
	int iBestDestinationValue = 0;

	for (std::map<IDInfo, int>::iterator it = cityValues.begin(); it != cityValues.end(); ++it)
	{
		int iValue = it->second;

		if (iValue > 0)
		{
			CvCity* pCity = ::getCity(it->first);
			if (pCity != NULL)
			{
				FAssert(!atPlot(pCity->plot()));
				if (generatePath(plot(), pCity->plot(), MOVE_NO_ENEMY_TERRITORY, true))
				{
					iValue /= 1 + kOwner.AI_plotTargetMissionAIs(pCity->plot(), MISSIONAI_TRANSPORT, this, 0);
				}
				else
				{
					iValue = 0;
				}
			}

			if (iValue > iBestDestinationValue)
			{
				iBestDestinationValue = iValue;
				kBestDestination = it->first;
			}
		}
	}


	if ((pPlotCity != NULL) && (kBestDestination.eOwner != NO_PLAYER))
	{
		//We need to keep looping and recalculating
		//For example a city might have "101" of an item, we want to move the first 100 but not the 1.
		//But it could also have 200, in which case we might want 2 loads of 100...
		//But it could also have 200 of two resources, and we'd want to move 100 of each...
		while (!isFull())
		{
			int iBestRoute = -1;
			int iBestRouteValue = 0;
			//Now, for any trade routes which this group is assigned to, try to pick up cargo here.
			for (uint i = 0; i < routes.size(); ++i)
			{
				CvCity* pSourceCity = ::getCity(routes[i]->getSourceCity());
				if ((pSourceCity != NULL) && (routes[i]->getDestinationCity() == kBestDestination))
				{
					CvCity* pDestinationCity = ::getCity(routes[i]->getDestinationCity());
					YieldTypes eYield = routes[i]->getYield();

					if ((pPlotCity == pSourceCity))
					{
						int iAmount = pSourceCity->getYieldStored(eYield) - pSourceCity->getMaintainLevel(eYield);

						if (iAmount > 0)
						{
							int iExportValue = kOwner.AI_transferYieldValue(routes[i]->getSourceCity(), routes[i]->getYield(), -iAmount);
							int iImportValue = kOwner.AI_transferYieldValue(routes[i]->getDestinationCity(), routes[i]->getYield(), iAmount);
							int iRouteValue = (iExportValue + iImportValue + 2 * std::min(iExportValue, iImportValue)) / 4;

							if (iRouteValue > iBestRouteValue)
							{
								iBestRouteValue = iRouteValue;
								iBestRoute = i;
							}
						}
					}
				}
			}

			if (iBestRouteValue > 0)
			{
				CLLNode<IDInfo>* pUnitNode = headUnitNode();
				while (pUnitNode != NULL)
				{
					CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = nextUnitNode(pUnitNode);

					if (pLoopUnit != NULL)
					{
						if (pLoopUnit->canLoadYield(plot(), routes[iBestRoute]->getYield(), false))
						{
							pLoopUnit->loadYield(routes[iBestRoute]->getYield(), false);
							break;
						}
					}
				}
			}
			else
			{
				break;
			}
		}
		//XXX fill hold.
	}

	if ((kBestDestination.eOwner == NO_PLAYER) && hasCargo())
	{
		// Transport group is full and can't find any destination
		CvCity* pCity = kOwner.AI_findBestPort();
		if (pCity != NULL && !atPlot(pCity->plot()))
		{
			kBestDestination = pCity->getIDInfo();
		}
	}

	//As a final step, we could consider loading yields which would be useful as parts of delivery runs...
	if (kBestDestination != kEurope)
	{
		CvCity* pBestDestinationCity = ::getCity(kBestDestination);
		if (pBestDestinationCity != NULL)
		{
			FAssert(!atPlot(pBestDestinationCity->plot()));
			pushMission(MISSION_MOVE_TO, pBestDestinationCity->getX_INLINE(), pBestDestinationCity->getY_INLINE(), MOVE_NO_ENEMY_TERRITORY, false, false, MISSIONAI_TRANSPORT, pBestDestinationCity->plot());
			if (atPlot(pBestDestinationCity->plot()))
			{
				//Unload any goods if required (we can always pick them back up if this is an i+e city).
				std::vector<CvUnit*> units;

				CLLNode<IDInfo>* pUnitNode = plot()->headUnitNode();
				CvUnit* pLoopUnit;
				while (pUnitNode != NULL)
				{
					pLoopUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = plot()->nextUnitNode(pUnitNode);
					YieldTypes eYield = pLoopUnit->getYield();

					if ((eYield != NO_YIELD) && pLoopUnit->isCargo())
					{
						if (pLoopUnit->getTransportUnit()->getGroup() == this && pLoopUnit->canUnload())
						{
							units.push_back(pLoopUnit);
						}
					}
				}
				for (uint i = 0; i < units.size(); ++i)
				{
					units[i]->unload();
				}
			}
			
			return true;
		}
	}
	else
	{
		if (isHuman())
		{
			getHeadUnit()->AI_setUnitAIState(UNITAI_STATE_SAIL);
		}
	}

	return false;
}

CvUnit* CvSelectionGroupAI::AI_ejectBestDefender(CvPlot* pDefendPlot)
{
	CLLNode<IDInfo>* pEntityNode;
	CvUnit* pLoopUnit;

	pEntityNode = headUnitNode();

	CvUnit* pBestUnit = NULL;
	int iBestUnitValue = 0;

	while (pEntityNode != NULL)
	{
		pLoopUnit = ::getUnit(pEntityNode->m_data);
		pEntityNode = nextUnitNode(pEntityNode);

		if (!pLoopUnit->noDefensiveBonus())
		{
			int iValue = pLoopUnit->currEffectiveStr(pDefendPlot, NULL) * 100;

			if (pDefendPlot->isCity(true, getTeam()))
			{
				iValue *= 100 + pLoopUnit->cityDefenseModifier();
				iValue /= 100;
			}

			iValue *= 100;
			iValue /= (100 + pLoopUnit->cityAttackModifier() + pLoopUnit->getExtraCityAttackPercent());

			iValue /= 2 + pLoopUnit->getLevel();

			if (iValue > iBestUnitValue)
			{
				iBestUnitValue = iValue;
				pBestUnit = pLoopUnit;
			}
		}
	}

	if (NULL != pBestUnit && getNumUnits() > 1)
	{
		pBestUnit->joinGroup(NULL);
	}

	return pBestUnit;
}


// Protected Functions...

void CvSelectionGroupAI::read(FDataStreamBase* pStream)
{
	CvSelectionGroup::read(pStream);

	uint uiFlag=0;
	pStream->Read(&uiFlag);	// flags for expansion

	pStream->Read(&m_iMissionAIX);
	pStream->Read(&m_iMissionAIY);

	pStream->Read(&m_bForceSeparate);

	pStream->Read((int*)&m_eMissionAIType);

	m_missionAIUnit.read(pStream);

	pStream->Read(&m_bGroupAttack);
	pStream->Read(&m_iGroupAttackX);
	pStream->Read(&m_iGroupAttackY);
}


void CvSelectionGroupAI::write(FDataStreamBase* pStream)
{
	CvSelectionGroup::write(pStream);

	uint uiFlag=0;
	pStream->Write(uiFlag);		// flag for expansion

	pStream->Write(m_iMissionAIX);
	pStream->Write(m_iMissionAIY);

	pStream->Write(m_bForceSeparate);

	pStream->Write(m_eMissionAIType);

	m_missionAIUnit.write(pStream);

	pStream->Write(m_bGroupAttack);
	pStream->Write(m_iGroupAttackX);
	pStream->Write(m_iGroupAttackY);
}

// Private Functions...
