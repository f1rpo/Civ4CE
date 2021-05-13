// unitAI.cpp

#include "CvGameCoreDLL.h"
#include "CvUnitAI.h"
#include "CvMap.h"
#include "CvArea.h"
#include "CvPlot.h"
#include "CvGlobals.h"
#include "CvGameAI.h"
#include "CvTeamAI.h"
#include "CvPlayerAI.h"
#include "CvGameCoreUtils.h"
#include "CvRandom.h"
#include "CyUnit.h"
#include "CyArgsList.h"
#include "CvDLLPythonIFaceBase.h"
#include "CvInfos.h"
#include "FProfiler.h"
#include "FAStarNode.h"

// interface uses
#include "CvDLLInterfaceIFaceBase.h"
#include "CvDLLFAStarIFaceBase.h"

#define FOUND_RANGE				(7)

#define MOVE_PRIORITY_MAX 			2000
#define MOVE_PRIORITY_HIGH 			1500
#define MOVE_PRIORITY_MEDIUM		1000
#define MOVE_PRIORITY_LOW			500
#define MOVE_PRIORITY_MIN			1

// Public Functions...

CvUnitAI::CvUnitAI() :
	m_eUnitAIType(NO_UNITAI)
{
	AI_reset();
}


CvUnitAI::~CvUnitAI()
{
	AI_uninit();
}


void CvUnitAI::AI_init()
{
	AI_reset();

	//--------------------------------
	// Init other game data
	AI_setBirthmark(GC.getGameINLINE().getSorenRandNum(10000, "AI Unit Birthmark"));
}


void CvUnitAI::AI_uninit()
{
}


void CvUnitAI::AI_reset()
{
	AI_uninit();

	m_iBirthmark = 0;
	m_iMovePriority = 0;
	m_iLastAIChangeTurn = GC.getGameINLINE().getGameTurn();
	
	m_eUnitAIType = NO_UNITAI;
	m_eUnitAIState = UNITAI_STATE_DEFAULT;
	m_eOldProfession = NO_PROFESSION;
	m_eIdealProfessionCache = INVALID_PROFESSION;
	
	m_iAutomatedAbortTurn = -1;
	
}

// AI_update returns true when we should abort the loop and wait until next slice
bool CvUnitAI::AI_update()
{
	PROFILE_FUNC();
	
	//FAssertMsg(getUnitTravelState() != NO_UNIT_TRAVEL_STATE || canMove(), "canMove is expected to be true");
	FAssertMsg(isGroupHead(), "isGroupHead is expected to be true"); // XXX is this a good idea???

	// allow python to handle it
	CyUnit* pyUnit = new CyUnit(this);
	CyArgsList argsList;
	argsList.add(gDLL->getPythonIFace()->makePythonObject(pyUnit));	// pass in unit class
	long lResult=0;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "AI_unitUpdate", argsList.makeFunctionArgs(), &lResult);
	delete pyUnit;	// python fxn must not hold on to this pointer
	if (lResult == 1)
	{
		return false;
	}
	
	if (getUnitTravelState() != NO_UNIT_TRAVEL_STATE)
	{
		AI_europeUpdate();
		return false;
	}

	int iOldMovePriority = AI_getMovePriority();

	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
	if (!AI_afterAttack())
	{
		if (getGroup()->isAutomated() && (getGroup()->getAutomateType() != AUTOMATE_FULL))
		{
			switch (getGroup()->getAutomateType())
			{
			case AUTOMATE_BUILD:
				AI_workerMove();
				break;

			case AUTOMATE_CITY:
				AI_cityAutomated();
				if (!isInGroup())
				{
					return true;
				}
				break;

			case AUTOMATE_EXPLORE:
				switch (getDomainType())
				{
				case DOMAIN_SEA:
					AI_exploreSeaMove();
					break;

				case DOMAIN_LAND:
					if (canSpeakWithChief(NULL))
					{
						AI_scoutMove();
					}
					else
					{
						AI_exploreMove();
					}
					break;

				default:
					FAssert(false);
					break;
				}
				break;

			case AUTOMATE_SAIL:
				AI_sailToEurope();
				break;
				
			case AUTOMATE_TRANSPORT_ROUTES:
				AI_transportMoveRoutes();
				break;
			
			case AUTOMATE_TRANSPORT_FULL:
				AI_transportMoveFull();
				break;
			
			case AUTOMATE_FULL:
				FAssert(false);
				break;
			default:
				FAssert(false);
				break;
			}
		
			if (getGroup() == NULL)
			{
				return true;
			}
			
			// if no longer automated, then we want to bail
			return (!getGroup()->isAutomated());
		}
		else if (canMove() || isCargo())
		{
			FAssert(getGroup() != NULL);
			switch (AI_getUnitAIType())
			{
			case UNITAI_UNKNOWN:
				getGroup()->pushMission(MISSION_SKIP);
				break;
				
			case UNITAI_COLONIST:
				AI_colonistMove();
				break;
			
			case UNITAI_SETTLER:
				AI_settlerMove();
				break;
			
			case UNITAI_WORKER:
				AI_workerMove();
				break;
				
			case UNITAI_MISSIONARY:
				AI_missionaryMove();
				break;
				
			case UNITAI_SCOUT:
				AI_scoutMove();
				break;
				
			case UNITAI_WAGON:
				AI_transportMoveFull();
				break;
				
			case UNITAI_TREASURE:
				AI_treasureMove();
				break;
			
			case UNITAI_YIELD:
				AI_yieldUhMove();
				break;
				
			case UNITAI_GENERAL:
				AI_generalMove();
				break;
							
			case UNITAI_DEFENSIVE:
				if (kOwner.isNative())
				{
					AI_defensiveBraveMove();
				}
				else if (kOwner.AI_isKing())
				{
					AI_imperialSoldierMove();
				}
				else
				{
					AI_defensiveMove();
				}
				break;
				
			case UNITAI_OFFENSIVE:
				if (kOwner.isNative())
				{
					AI_offensiveBraveMove();
				}
				else if (kOwner.AI_isKing())
				{
					AI_imperialCannonMove();
				}
				else
				{
					AI_offensiveMove();
				}
				break;
				
			case UNITAI_COUNTER:
				if (kOwner.isNative())
				{
					AI_counterBraveMove();
				}
				else if (kOwner.AI_isKing())
				{
					AI_imperialMountedMove();
				}
				else
				{
					AI_counterMove();
				}
				break;
				
			case UNITAI_TRANSPORT_SEA:
				AI_transportSeaMove();
				break;
			
				
			case UNITAI_ASSAULT_SEA:
				AI_assaultSeaMove();
				break;
				
			case UNITAI_COMBAT_SEA:
				if (kOwner.AI_isKing())
				{
					AI_imperialShipMove();
				}
				else
				{
					AI_combatSeaMove();
				}
				break;
				
			case UNITAI_PIRATE_SEA:
				AI_pirateMove();
				break;
				
			default:
				FAssert(false);
				break;
			}
		}
	}

	if (isDead() || isDelayedDeath() || getGroup() == NULL)
	{
		return true;
	}

	if (!isHuman())
	{
		if (AI_getMovePriority() == iOldMovePriority)
		{
			if (canMove() && (getGroup()->getActivityType() == ACTIVITY_MISSION || getGroup()->getActivityType() == ACTIVITY_AWAKE))
			{
				AI_setMovePriority(AI_getMovePriority() - (MOVE_PRIORITY_MAX / 10));
			}
			else
			{
				AI_setMovePriority(0);
			}
		}
	}
	return false;
}

// AI_update returns true when we should abort the loop and wait until next slice
bool CvUnitAI::AI_europeUpdate()
{
	PROFILE_FUNC();

	if (getDomainType() == DOMAIN_LAND)
	{
		return false;//XXX maybe units should load onto ships...
	}

	if (getUnitTravelTimer() > 0)
	{
		return false;
	}
	else
	{
		if (getGroup()->isAutomated() && (getGroup()->getAutomateType() == AUTOMATE_FULL))
		{
			if (getUnitTravelState() == UNIT_TRAVEL_STATE_IN_EUROPE)
			{
				AI_europe();
				return false;
			}
		}
	}

	if (getGroup()->isAutomated() && (getGroup()->getAutomateType() != AUTOMATE_FULL))
	{

	}
	else
	{
		if (isHurt() && (healRate(plot()) > 0))
		{
			return false;
		}
		switch (AI_getUnitAIType())
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
		case UNITAI_DEFENSIVE:
		case UNITAI_OFFENSIVE:
		case UNITAI_COUNTER:
		    break;

		case UNITAI_TRANSPORT_SEA:
			AI_europe();
		    break;
		    
		case UNITAI_ASSAULT_SEA:			
		case UNITAI_COMBAT_SEA:
			if (GET_PLAYER(getOwnerINLINE()).AI_isKing())
			{
				if (hasCargo())
				{
					crossOcean(UNIT_TRAVEL_STATE_FROM_EUROPE);
				}
			}
			else
			{
				crossOcean(UNIT_TRAVEL_STATE_FROM_EUROPE);
			}
				
			break;
		case UNITAI_PIRATE_SEA:
			crossOcean(UNIT_TRAVEL_STATE_FROM_EUROPE);
		    break;
			
		default:
			FAssert(false);
			break;
		}
	}
	
	AI_setMovePriority(0);
	return false;
}


// Returns true if took an action or should wait to move later...
bool CvUnitAI::AI_follow()
{
	if (AI_followBombard())
	{
		return true;
	}

	if (AI_cityAttack(1, 65, true))
	{
		return true;
	}

	if (isEnemy(plot()->getTeam()))
	{
		if (canPillage(plot()))
		{
			getGroup()->pushMission(MISSION_PILLAGE);
			return true;
		}
	}

	if (AI_anyAttack(1, 70, 2, true))
	{
		return true;
	}

	if (canFound(NULL))
	{
		if (area()->getBestFoundValue(getOwnerINLINE()) > 0)
		{
			if (AI_foundRange(FOUND_RANGE, true))
			{
				return true;
			}
		}
	}

	return false;
}


// XXX what if a unit gets stuck b/c of it's UnitAIType???
// XXX is this function costing us a lot? (it's recursive...)
void CvUnitAI::AI_upgrade()
{
	PROFILE_FUNC();

	FAssertMsg(!isHuman(), "isHuman did not return false as expected");
	FAssertMsg(AI_getUnitAIType() != NO_UNITAI, "AI_getUnitAIType() is not expected to be equal with NO_UNITAI");

	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());
	UnitAITypes eUnitAI = AI_getUnitAIType();
	CvArea* pArea = area();

	int iCurrentValue = kPlayer.AI_unitValue(getUnitType(), eUnitAI, pArea);
	
	for (int iPass = 0; iPass < 2; iPass++)
	{
		int iBestValue = 0;
		UnitTypes eBestUnit = NO_UNIT;

		for (int iI = 0; iI < GC.getNumUnitInfos(); iI++)
		{
			if ((iPass > 0) || GC.getUnitInfo((UnitTypes)iI).getUnitAIType(AI_getUnitAIType()))
			{
				int iNewValue = kPlayer.AI_unitValue(((UnitTypes)iI), eUnitAI, pArea);
				if ((iPass == 0 || iNewValue > 0) && iNewValue > iCurrentValue)
				{
					if (canUpgrade((UnitTypes)iI))
					{
						int iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Upgrade"));

						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							eBestUnit = ((UnitTypes)iI);
						}
					}
				}
			}
		}

		if (eBestUnit != NO_UNIT)
		{
			upgrade(eBestUnit);
			doDelayedDeath();
			return;
		}
	}
}


void CvUnitAI::AI_promote()
{
	PROFILE_FUNC();

	PromotionTypes eBestPromotion;
	int iValue;
	int iBestValue;
	int iI;

	iBestValue = 0;
	eBestPromotion = NO_PROMOTION;

	for (iI = 0; iI < GC.getNumPromotionInfos(); iI++)
	{
		if (canPromote((PromotionTypes)iI, -1))
		{
			iValue = AI_promotionValue((PromotionTypes)iI);

			if (iValue > iBestValue)
			{
				iBestValue = iValue;
				eBestPromotion = ((PromotionTypes)iI);
			}
		}
	}

	if (eBestPromotion != NO_PROMOTION)
	{
		promote(eBestPromotion, -1);
		AI_promote();
	}
}


int CvUnitAI::AI_groupFirstVal()
{
	switch (AI_getUnitAIType())
	{
	case UNITAI_UNKNOWN:
		FAssert(false);
		break;
		
	case UNITAI_COLONIST:
		return 21;
		break;
		
	case UNITAI_SETTLER:
		return 5;
		break;
	
	case UNITAI_WORKER:
		return 6;
		break;
		
	case UNITAI_MISSIONARY:
		return 1;
		break;
	
	case UNITAI_SCOUT:
		return 24;
		break;
		
	case UNITAI_WAGON:
		return 25;
		break;
		
	case UNITAI_TREASURE:
		return 26;
		break;
	case UNITAI_YIELD:
		return 1;
		break;
	
	case UNITAI_GENERAL:
		return 30;
		break;
	
	case UNITAI_DEFENSIVE:
		return 14;
		break;
		
	case UNITAI_OFFENSIVE:
		return 16;
		break;
		
	case UNITAI_COUNTER:
		return 18;
		break;
		
	case UNITAI_TRANSPORT_SEA:
		return 20;
		break;

	case UNITAI_ASSAULT_SEA:
		return 14;
		break;
		
	case UNITAI_COMBAT_SEA:
		return 18;
		break;
		
	case UNITAI_PIRATE_SEA:
		return 16;
		break;
		
	default:
		FAssert(false);
		break;
	}

	return 0;
}


int CvUnitAI::AI_groupSecondVal()
{
	return (baseCombatStr());
}


// Returns attack odds out of 100 (the higher, the better...)
// Withdrawal odds included in returned value
int CvUnitAI::AI_attackOdds(const CvPlot* pPlot, bool bPotentialEnemy) const
{
	CvUnit* pDefender;
	int iOurStrength;
	int iTheirStrength;
	int iOurFirepower;
	int iTheirFirepower;
	int iBaseOdds;
	int iStrengthFactor;
	int iDamageToUs;
	int iDamageToThem;
	int iNeededRoundsUs;
	int iNeededRoundsThem;

	pDefender = pPlot->getBestDefender(NO_PLAYER, getOwnerINLINE(), this, !bPotentialEnemy, bPotentialEnemy);

	CvCity* pCity = pPlot->getPlotCity();
	if (pCity != NULL)
	{
		pDefender = pCity->getBestDefender(NULL, pDefender, this);
	}
					
	if (pDefender == NULL)
	{
		return 100;
	}

	iOurStrength = currCombatStr(NULL, NULL);
	iOurFirepower = currFirepower(NULL, NULL);

	if (iOurStrength == 0)
	{
		return 1;
	}

	iTheirStrength = pDefender->currCombatStr(pPlot, this);
	iTheirFirepower = pDefender->currFirepower(pPlot, this);


	FAssert((iOurStrength + iTheirStrength) > 0);
	FAssert((iOurFirepower + iTheirFirepower) > 0);

	iBaseOdds = (100 * iOurStrength) / (iOurStrength + iTheirStrength);
	if (iBaseOdds == 0)
	{
		return 1;
	}

	iStrengthFactor = ((iOurFirepower + iTheirFirepower + 1) / 2);

	iDamageToUs = std::max(1,((GC.getDefineINT("COMBAT_DAMAGE") * (iTheirFirepower + iStrengthFactor)) / (iOurFirepower + iStrengthFactor)));
	iDamageToThem = std::max(1,((GC.getDefineINT("COMBAT_DAMAGE") * (iOurFirepower + iStrengthFactor)) / (iTheirFirepower + iStrengthFactor)));

	iNeededRoundsUs = (std::max(0, pDefender->currHitPoints()) + iDamageToThem - 1 ) / iDamageToThem;
	iNeededRoundsThem = (std::max(0, currHitPoints()) + iDamageToUs - 1 ) / iDamageToUs;

	iNeededRoundsUs = std::max(1, iNeededRoundsUs);
	iNeededRoundsThem = std::max(1, iNeededRoundsThem);

	int iRoundsDiff = iNeededRoundsUs - iNeededRoundsThem;
	if (iRoundsDiff > 0)
	{
		iTheirStrength *= (1 + iRoundsDiff);
	}
	else
	{
		iOurStrength *= (1 - iRoundsDiff);
	}

	int iOdds = (((iOurStrength * 100) / (iOurStrength + iTheirStrength)));
	iOdds += ((100 - iOdds) * (std::min(100, iOdds * 2) * withdrawalProbability() / 100)) / 100;
	iOdds += GET_PLAYER(getOwnerINLINE()).AI_getAttackOddsChange();

	return std::max(1, std::min(iOdds, 99));
}


// Returns true if the unit found a build for this city...
bool CvUnitAI::AI_bestCityBuild(CvCity* pCity, CvPlot** ppBestPlot, BuildTypes* peBestBuild, CvPlot* pIgnorePlot, CvUnit* pUnit)
{
	PROFILE_FUNC();

	BuildTypes eBuild;
	int iValue;
	int iBestValue;
	int iI;

	iBestValue = 0;
	BuildTypes eBestBuild = NO_BUILD;
	CvPlot* pBestPlot = NULL;

	
	for (int iPass = 0; iPass < 2; iPass++)
	{
		for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
		{
			CvPlot* pLoopPlot = plotCity(pCity->getX_INLINE(), pCity->getY_INLINE(), iI);

			if (pLoopPlot != NULL)
			{
				if (AI_plotValid(pLoopPlot))
				{
					if (pLoopPlot != pIgnorePlot)
					{
						if ((pLoopPlot->getImprovementType() == NO_IMPROVEMENT) || !(GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_SAFE_AUTOMATION) && !(pLoopPlot->getImprovementType() == (GC.getDefineINT("RUINS_IMPROVEMENT")))))
						{
							iValue = pCity->AI_getBestBuildValue(iI);

							if (iValue > iBestValue)
							{
								eBuild = pCity->AI_getBestBuild(iI);
								FAssertMsg(eBuild < GC.getNumBuildInfos(), "Invalid Build");

								if (eBuild != NO_BUILD)
								{
									if (0 == iPass)
									{
										iBestValue = iValue;
										pBestPlot = pLoopPlot;
										eBestBuild = eBuild;
									}
									else if (canBuild(pLoopPlot, eBuild))
									{
										if (!(pLoopPlot->isVisibleEnemyUnit(this)))
										{
											int iPathTurns = 0;
											if (generatePath(pLoopPlot, 0, true, &iPathTurns))
											{
												// XXX take advantage of range (warning... this could lead to some units doing nothing...)
												int iMaxWorkers = 1;
												if (getPathLastNode()->m_iData1 == 0)
												{
													iPathTurns++;
												}
												else if (iPathTurns <= 1)
												{
													iMaxWorkers = AI_calculatePlotWorkersNeeded(pLoopPlot, eBuild);
												}
												if (pUnit != NULL)
												{
													if (pUnit->plot()->isCity() && iPathTurns == 1 && getPathLastNode()->m_iData1 > 0)
													{
														iMaxWorkers += 10;
													}
												}	
												if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_BUILD, getGroup()) < iMaxWorkers)
												{
													//XXX this could be improved greatly by
													//looking at the real build time and other factors
													//when deciding whether to stack.
													iValue /= iPathTurns;

													iBestValue = iValue;
													pBestPlot = pLoopPlot;
													eBestBuild = eBuild;
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
		
		if (0 == iPass)
		{
			if (eBestBuild != NO_BUILD)
			{
				FAssert(pBestPlot != NULL);
				int iPathTurns;
				if ((generatePath(pBestPlot, 0, true, &iPathTurns)) && canBuild(pBestPlot, eBestBuild)
					&& !(pBestPlot->isVisibleEnemyUnit(this)))
				{
					int iMaxWorkers = 1;
					if (pUnit != NULL)
					{
						if (pUnit->plot()->isCity())
						{
							iMaxWorkers += 10;
						}
					}	
					if (getPathLastNode()->m_iData1 == 0)
					{
						iPathTurns++;
					}
					else if (iPathTurns <= 1)
					{
						iMaxWorkers = AI_calculatePlotWorkersNeeded(pBestPlot, eBestBuild);
					}
					int iWorkerCount = GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pBestPlot, MISSIONAI_BUILD, getGroup());
					if (iWorkerCount < iMaxWorkers)
					{
						//Good to go.
						break;
					}
				}
				eBestBuild = NO_BUILD;
				iBestValue = 0;
			}			
		}
	}
	
	if (NO_BUILD != eBestBuild)
	{
		FAssert(NULL != pBestPlot);
		if (ppBestPlot != NULL)
		{
			*ppBestPlot = pBestPlot;
		}
		if (peBestBuild != NULL)
		{
			*peBestBuild = eBestBuild;
		}
	}


	return (NO_BUILD != eBestBuild);
}


bool CvUnitAI::AI_isCityAIType() const
{
	return (AI_getUnitAIType() == UNITAI_DEFENSIVE);
}


int CvUnitAI::AI_getBirthmark() const
{
	return m_iBirthmark;
}


void CvUnitAI::AI_setBirthmark(int iNewValue)
{
	m_iBirthmark = iNewValue;
}


UnitAITypes CvUnitAI::AI_getUnitAIType() const
{
	return m_eUnitAIType;
}


// XXX make sure this gets called...
void CvUnitAI::AI_setUnitAIType(UnitAITypes eNewValue)
{
	if (AI_getUnitAIType() != eNewValue)
	{
		bool bOnMap = (getX_INLINE() != INVALID_PLOT_COORD) && (getY_INLINE() != INVALID_PLOT_COORD);

		if (AI_getUnitAIType() != NO_UNITAI)
		{
			if (bOnMap)
			{
				area()->changeNumAIUnits(getOwnerINLINE(), AI_getUnitAIType(), -1);
			}
			GET_PLAYER(getOwnerINLINE()).AI_changeNumAIUnits(AI_getUnitAIType(), -1);
		}

		m_eUnitAIType = eNewValue;

		if (bOnMap)
		{
			area()->changeNumAIUnits(getOwnerINLINE(), AI_getUnitAIType(), 1);
		
			joinGroup(NULL);

			if (getTransportUnit() != NULL)
			{
				getGroup()->setActivityType(ACTIVITY_SLEEP);
			}
		}
		GET_PLAYER(getOwnerINLINE()).AI_changeNumAIUnits(AI_getUnitAIType(), 1);
		
		int iCurrentTurn = GC.getGameINLINE().getGameTurn();
		if (getGameTurnCreated() != iCurrentTurn)
		{
			m_iLastAIChangeTurn = iCurrentTurn;
		}
	}
}

UnitAIStates CvUnitAI::AI_getUnitAIState() const
{
	return m_eUnitAIState;
}

void CvUnitAI::AI_setUnitAIState(UnitAIStates eNewValue)
{

	if (AI_getUnitAIState() != eNewValue)
	{
		m_eUnitAIState = eNewValue;
	}
	
	int iCurrentTurn = GC.getGameINLINE().getGameTurn();
	if (getGameTurnCreated() != iCurrentTurn)
	{
		m_iLastAIChangeTurn = iCurrentTurn;
	}
}

int CvUnitAI::AI_sacrificeValue(const CvPlot* pPlot) const
{
	int iDenominator = 100 + cityDefenseModifier();
	int iValue  = 128 * (currEffectiveStr(pPlot, ((pPlot == NULL) ? NULL : this))) + iDenominator - 1;  // round up
	iValue /= iDenominator;

	iValue *= 100 + withdrawalProbability();		
	int iCostValue = 0;
	for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
	{
		iCostValue += m_pUnitInfo->getYieldCost(iYield);
	}
	iDenominator = std::max(1, (1 + iCostValue));
	iCostValue += iDenominator - 1;  // round up
	iValue /= iDenominator;

	iDenominator = 10 + getExperience();
	iValue += iDenominator - 1;  // round up
	iValue /= iDenominator;

    return iValue;
}

// Protected Functions...
void CvUnitAI::AI_colonistMove()
{
	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
	if ((kOwner.getNumCities() == 0) && (kOwner.AI_getNumAIUnits(UNITAI_SETTLER) == 0))
	{
		if (canFound(NULL))
		{
			AI_setUnitAIType(UNITAI_SETTLER);
			AI_settlerMove();
			return;
		}
	}
	
	if (isCargo())
	{
		if (AI_joinOptimalCity())
		{
			return;
		}
		if (AI_joinCity())
		{
			return;
		}
		getGroup()->pushMission(MISSION_SKIP);
		return;
	}
	
	if ((area()->getCitiesPerPlayer(getOwnerINLINE()) == 0) && (area()->getNumAIUnits(getOwnerINLINE(), UNITAI_SETTLER) == 0))
	{
		if (area()->getBestFoundValue(getOwnerINLINE()) > 0)
		{
			AI_setUnitAIType(UNITAI_SETTLER);
			AI_settlerMove();
			return;		
		}
	}
	
	int iDanger = GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot(), 3);
	
	if (AI_changeUnitAIType(99))
	{
		return;
	}
	
	
	if ((m_pUnitInfo->getLearnTime() >= 0) && (getProfession() == GC.getCivilizationInfo(getCivilizationType()).getDefaultProfession()))
	{
		if (GC.getGameINLINE().AI_gameCompletePercent() < 25)
		{
		if (AI_learn())
		{
			return;
		}
	}
	}
	
	if (AI_joinOptimalCity())
	{
		return;
	}
	
	if (area()->getBestFoundValue(getOwnerINLINE()) > 1500 && area()->getNumAIUnits(getOwnerINLINE(), UNITAI_SETTLER) == 0)
	{
		if (kOwner.AI_getPopulation() / kOwner.getNumCities() > 4)
		{
			AI_setUnitAIType(UNITAI_SETTLER);
			AI_settlerMove();
			return;
		}
	}

	if (AI_joinCity())
	{
		return;
	}

	if (AI_changeUnitAIType(49))
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}

void CvUnitAI::AI_settlerMove()
{
	PROFILE_FUNC();

	bool bDanger = GET_PLAYER(getOwnerINLINE()).AI_getUnitDanger(this, 2, false, false);
	int iMinFoundValue = (GC.getGame().getGameTurn() > 20) ? 1 : 2;
	
	if (isNative())
	{
		if (AI_foundRange(7))
		{
			return;
		}
		if (AI_found())
		{
			return;
		}
		if (GC.getGame().getGameTurn() - AI_getLastAIChangeTurn() > 10)
		{
			AI_setUnitAIType(UNITAI_DEFENSIVE);
			return;
		}
	}

	if (isCargo())
	{
		if (AI_found(iMinFoundValue))
		{
			return;
		}

		getGroup()->pushMission(MISSION_SKIP);
		return;
	}
	
	if (!isCargo() && (GET_PLAYER(getOwnerINLINE()).getNumCities() == 0) && (plot()->getNearestEurope() != NO_EUROPE))
	{
		if (canFound(plot()))
		{
			found();
			return;
		}
	}
	
	if (bDanger)
	{
		if (canJoinCity(plot()))
		{
			joinCity();
			return;
		}
		
		if (AI_retreatToCity())
		{
			return;
		}
	}
	
	if (!isCargo())
	{
		if (AI_found(iMinFoundValue))
		{
			return;
		}
	}
	
	CvCity* pCity;
	bool bCanRoute;

	bCanRoute = canBuildRoute();

	pCity = NULL;

	if (plot()->getOwnerINLINE() == getOwnerINLINE())
	{
		pCity = plot()->getPlotCity();
		if (pCity == NULL)
		{
			pCity = plot()->getWorkingCity();
		}
	}
	
	if (pCity != NULL)
	{
		if ((pCity->AI_getWorkersNeeded() > 0) && (plot()->isCity() || (pCity->AI_getWorkersNeeded() < ((1 + pCity->AI_getWorkersHave() * 2) / 3))))
		{
			if (AI_improveCity(pCity))
			{
				return;
			}
		}
	}
	
	if (AI_improveLocalPlot(2, pCity))
	{
		return;		
	}



	if (pCity != NULL)
	{
		if (AI_improveCity(pCity))
		{
			return;
		}
	}


	if (AI_nextCityToImprove(pCity))
	{
		return;
	}
		
	if (AI_improveLocalPlot(3, NULL))
	{
		return;		
	}
	
	if (canJoinCity(plot()))
	{
		joinCity();
		return;
	}

	if (AI_retreatToCity(false, true))
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}

void CvUnitAI::AI_workerMove()
{
	PROFILE_FUNC();
	
	if (AI_breakAutomation())
	{
		return;
	}
	
	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
	CvCity* pCity = (plot()->getOwnerINLINE() == getOwnerINLINE()) ? plot()->getPlotCity() : NULL;
	bool bDanger = GET_PLAYER(getOwnerINLINE()).AI_getUnitDanger(this, 2, false, false);
	if (bDanger)
	{
		if (pCity != NULL)
		{
			if (canJoinCity(plot()))
			{
				joinCity();
				return;
			}
		}
		if (AI_retreatToCity())
		{
			return;
		}
		if (AI_safety())
		{
			return;
		}
	}
	
	if ((kOwner.getNumCities() == 0) && (kOwner.AI_getNumAIUnits(UNITAI_SETTLER) == 0))
	{
		if (canFound(NULL))
		{
			AI_setUnitAIType(UNITAI_SETTLER);
			AI_settlerMove();
			return;
		}
	}
	
	if (!isHuman())
	{
		if ((pCity != NULL) && (pCity->getOwnerINLINE() == getOwnerINLINE()))
		{
			if (AI_betterJob())
			{
				return;
			}
		}
	}
	
	if (isCargo())
	{
		if (AI_unloadWhereNeeded())
		{
			return;
		}

		getGroup()->pushMission(MISSION_SKIP);
		return;
	}


	bool bCanRoute;

	bCanRoute = canBuildRoute();

	pCity = getHomeCity();

	if (pCity == NULL)
	{
		if (plot()->getOwnerINLINE() == getOwnerINLINE())
		{
			pCity = plot()->getWorkingCity();
		}
	}
	
	if (pCity != NULL)
	{
		if (pCity->AI_getWorkersNeeded() > 0)
		{
			if (AI_improveCity(pCity))
			{
				if (getHomeCity() != pCity)
				{
					setHomeCity(pCity);
				}
				return;
			}
		}
	}

	if (AI_nextCityToImprove(pCity))
	{
		return;
	}

	
	if (pCity != NULL)
	{
		if (pCity->AI_getWorkersNeeded() == 0)
		{
			if (AI_improveCity(pCity))
			{
				if (getHomeCity() != pCity)
				{
					setHomeCity(pCity);
				}
				return;
			}
		}
	}
	
	AI_setUnitAIType(UNITAI_COLONIST);
	return;
}

void CvUnitAI::AI_missionaryMove()
{
	PROFILE_FUNC();
	
	if (AI_breakAutomation())
	{
		return;
	}
	
	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
	CvCity* pCity = (plot()->getOwnerINLINE() == getOwnerINLINE()) ? plot()->getPlotCity() : NULL;
	bool bDanger = GET_PLAYER(getOwnerINLINE()).AI_getUnitDanger(this, 2, false, false);
	if (bDanger)
	{
		if (pCity != NULL)
		{
			if (canJoinCity(plot()))
			{
				joinCity();
				return;
			}
		}
		if (AI_retreatToCity())
		{
			return;
		}
		if (AI_safety())
		{
			return;
		}
	}
		
	if (!isHuman() && pCity != NULL)
	{
		if (AI_betterJob())
		{
			return;
		}
	}
	
	if (isCargo())
	{
		if (AI_unloadWhereNeeded())
		{
			return;
		}
	}
	
	//Cheesey exploit?
	if (canLearn())
	{
		UnitTypes eTeachUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(plot()->getPlotCity()->getTeachUnitClass());
		if (getUnitInfo().getMissionaryRateModifier() < GC.getUnitInfo(eTeachUnit).getMissionaryRateModifier())
		{
			learn();
			return;
		}
	}
	
	if (AI_spreadReligion())
	{
		return;
	}
	
	if (area()->getNumUnrevealedTiles(getTeam()) > 0)
	{
		if (AI_exploreOpenBorders(4))
		{
			return;
		}
		
		if (AI_explore(true))
		{
			return;
		}
	}
	
	//We should try and convert profession to something else
	//When this is possible.

	if (AI_retreatToCity())
	{
		AI_setUnitAIType(UNITAI_COLONIST);
		GET_PLAYER(getOwnerINLINE()).AI_changeNumRetiredAIUnits(UNITAI_MISSIONARY, 1);
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}

void CvUnitAI::AI_scoutMove()
{
	CvCity* pCity = plot()->getPlotCity();
	
	if (AI_breakAutomation())
	{
		return;
	}
	
	if (isCargo())
	{
		if (canUnload())
		{
			if (plot()->area()->getNumUnrevealedTiles(getTeam()) > 0)
			{
				unload();
				return;
			}
		}
		if (AI_exploreFromShip(6))
		{
			return;
		}
		
		if (AI_unloadWhereNeeded())
		{
			return;
		}

		if (canUnload())
		{
			unload();
			return;
		}
		
		AI_setUnitAIType(UNITAI_COLONIST);
		GET_PLAYER(getOwnerINLINE()).AI_changeNumRetiredAIUnits(UNITAI_SCOUT, 1);

		
		return;
	}
	
	if (!isHuman())
	{
		if ((pCity != NULL) && (pCity->getOwnerINLINE() == getOwnerINLINE()))
		{
			if (AI_betterJob())
			{
				return;
			}
		}
	}
	
	if (isCargo())
	{
		if (AI_unloadWhereNeeded())
		{
			return;
		}
		getGroup()->pushMission(MISSION_SKIP);
		return;
	}
	
	if (AI_heal())
	{
		return;
	}

	if (pCity != NULL)
	{
		if (!isHuman())
		{
			if (GET_PLAYER(getOwnerINLINE()).AI_professionSuitability(getUnitType(), getProfession()) < 100)
			{
				if (canClearSpecialty())
				{
					clearSpecialty();
					return;
				}
			}
		}
		if (!pCity->isScoutVisited(getTeam()))
		{
			if (canSpeakWithChief(plot()))
			{
				speakWithChief();
				FAssert(pCity->isScoutVisited(getTeam()));
				return;
			}
		}
		if (canLearn())
		{
			UnitTypes eTeachUnit = getLearnUnitType(plot());
			
			if (GET_PLAYER(getOwnerINLINE()).AI_professionSuitability(eTeachUnit, getProfession()) > 100)
			{
				learn();
				return;
			}
		}
	}
	
	if (AI_exploreRange(4))
	{
		return;
	}
	
	if (AI_goody())
	{
		return;
	}
		
	if (area()->getNumUnrevealedTiles(getTeam()) > 0)
	{		
		if (AI_explore())
		{
			return;
		}
	}
	
	if (!isHuman())
	{
		bool bRequestPickup = false;
		CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
		CvArea* pLoopArea;
		int iLoop;
		for(pLoopArea = GC.getMapINLINE().firstArea(&iLoop); pLoopArea != NULL; pLoopArea = GC.getMapINLINE().nextArea(&iLoop))
		{
			if (!pLoopArea->isWater())
			{
				if ((pLoopArea->getNumUnrevealedTiles(getTeam()) > 7) && pLoopArea->getNumAIUnits(getOwnerINLINE(), UNITAI_SCOUT) == 0)
				{
					bRequestPickup = true;
					break;
				}
			}
		}
		
		if (bRequestPickup)
		{
			if (AI_requestPickup())
			{
				return;
			}
		}
		
		{	//Become a Colonist
			if (m_pUnitInfo->getUnitAIType(UNITAI_COLONIST))
			{
				AI_setUnitAIType(UNITAI_COLONIST);
				return;
			}
		}
	}
	else
	{
		FAssert(isAutomated());
		if (plot()->getOwnerINLINE() == getOwnerINLINE())
		{
			getGroup()->setAutomateType(NO_AUTOMATE);
			return;
		}
	}
	
	if (AI_retreatToCity())
	{
		return;
	}
	
	getGroup()->pushMission(MISSION_SKIP);
	return;
}

void CvUnitAI::AI_treasureMove()
{
	PROFILE_FUNC();
	
	if (isCargo())
	{
		getGroup()->pushMission(MISSION_SKIP);
		return;
	}
	
	bool bDanger = GET_PLAYER(getOwnerINLINE()).AI_getUnitDanger(this, 2, false, false);
	if (bDanger)
	{
		if (AI_retreatToCity())
		{
			return;
		}
		
		if (AI_safety())
		{
			return;
		}
	}

	CvCity* pCity = (plot()->getOwnerINLINE() == getOwnerINLINE()) ? plot()->getPlotCity() : NULL;
	
	bool bAtWar = GET_TEAM(getTeam()).getAnyWarPlanCount();
	
	if (pCity != NULL)
	{
		if ((GC.getGameINLINE().getGameTurn() < 90) || bAtWar)
		{
			if (canKingTransport())
			{
				kingTransport(true);	
				return;
			}
		}
	}
	

	if (AI_treasureRetreat(MAX_INT))
	{
		return;
	}
		
	if (AI_requestPickup())
	{
		return;
	}
	
	if (AI_safety())
	{
		return;
	}
	
	getGroup()->pushMission(MISSION_SKIP);
	return;
}

void CvUnitAI::AI_yieldUhMove()
{
	FAssert(isCargo());
	
	if (AI_getUnitAIState() == UNITAI_STATE_SELL_TO_NATIVES)
	{
		if (AI_yieldNativeDestination())
		{
			return;
		}
		if (GC.getGameINLINE().getGameTurn() - getGameTurnCreated() > 6)
		{
			GET_PLAYER(getOwnerINLINE()).AI_clearStrategy(STRATEGY_SELL_TO_NATIVES);
			AI_setUnitAIState(UNITAI_STATE_PURCHASED);
		}
		else
		{
			AI_setMovePriority(0);
			return;
		}		
	}
	if (AI_yieldDestination())
	{
		AI_setMovePriority(MOVE_PRIORITY_MIN);
		return;
	}
	
	AI_setMovePriority(0);
	return;	
}

void CvUnitAI::AI_generalMove()
{
	PROFILE_FUNC();

	std::vector<UnitAITypes> aeUnitAITypes;

	aeUnitAITypes.clear();
	aeUnitAITypes.push_back(UNITAI_DEFENSIVE);
	aeUnitAITypes.push_back(UNITAI_OFFENSIVE);
	aeUnitAITypes.push_back(UNITAI_COUNTER);

	if (AI_lead(aeUnitAITypes))
	{
		return;
	}
	
	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}

void CvUnitAI::AI_defensiveMove()
{
	PROFILE_FUNC();
	
	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
	bool bDanger = kOwner.AI_getPlotDanger(plot(), 2, false);
	if ((kOwner.getNumCities() == 0) && (kOwner.AI_getNumAIUnits(UNITAI_SETTLER) == 0))
	{
		if (canFound(NULL))
		{
			AI_setUnitAIType(UNITAI_SETTLER);
			AI_settlerMove();
			return;
		}
	}
	
	CvCity* pCity = plot()->getPlotCity();
	if ((pCity != NULL) && (pCity->getOwnerINLINE() != getOwnerINLINE()))
	{
		pCity = NULL;		
	}
	if (kOwner.AI_isStrategy(STRATEGY_REVOLUTION) && bDanger)
	{
		if ((pCity != NULL) && bDanger)
		{
			getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_GUARD_CITY);
			return;
		}
	}
	
	if ((!isHuman() && getProfession() != NO_PROFESSION) && !bDanger)
	{
		if (pCity != NULL)
		{
			if (isCargo() || pCity->getPopulation() < 3)
			{
				if (canJoinCity(plot()))
				{
					pCity->addPopulationUnit(this, NO_PROFESSION);
					return;
				}
			}
					
			if ((getFortifyTurns() == 0) || (GC.getGameINLINE().getSorenRandNum(10, "AI better job")  == 0))
			{
				if (AI_betterJob())
				{
					return;
				}
				
				if (canClearSpecialty())
				{
					clearSpecialty();
					return;
				}
			}
		}
	}
	
	if (isCargo())
	{
		if (AI_guardCityMinDefender())
		{
			return;
		}
		
		if (AI_unloadWhereNeeded())
		{
			return;
		}
		
		getGroup()->pushMission(MISSION_SKIP);
		return;
	}
		
	if ((GC.getGame().getGameTurn() < 10) && (area()->getNumAIUnits(getOwnerINLINE(), UNITAI_SCOUT) == 0))
	{
		if (bDanger)
		{
			if (AI_guardCityBestDefender())
			{
				return;
			}
		}
			
		if (AI_goodyRange(2))
		{
			return;
		}
		if (AI_exploreRange(4))
		{
			return;
		}
		
		if (AI_explore())
		{
			return;
		}
	}
	
	CvPlot* pMissionPlot = getGroup()->AI_getMissionAIPlot();
	
	if (AI_guardCityBestDefender())
	{
		return;
	}
	
	if ((bDanger) && !(kOwner.AI_isStrategy(STRATEGY_REVOLUTION)))
	{
		if (AI_leaveAttack(1, 50, 120))
		{
			return;
		}
	}
	
	if (AI_guardCityMinDefender())
	{
		return;
	}
	
	if ((GC.getGameINLINE().getGameTurn() == getGameTurnCreated()) || (GC.getGameINLINE().getSorenRandNum(5, "AI shuffle defenders") == 0) || AI_isOnMission())
	{
		if (AI_guardCity(true))
		{
			return;
		}
	}
	else
	{
		if (!bDanger)
		{
			if (AI_joinCityDefender())
			{
				return;
			}
		}
		if (AI_guardCity(false))
		{
			return;
		}
	}

	if (!(kOwner.AI_isStrategy(STRATEGY_REVOLUTION)))
	{
		if (AI_smartAttack(1, 70, 98, NULL))
		{
			return;
		}
	}

	if (AI_heal())
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}

void CvUnitAI::AI_offensiveMove()
{
	PROFILE_FUNC();
	
	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
	bool bDanger = kOwner.AI_getPlotDanger(plot(), 2, false, true);
	
	if (!isHuman() && getProfession() != NO_PROFESSION)
	{
		CvCity* pCity = plot()->getPlotCity();
		if ((pCity != NULL) && (pCity->getOwnerINLINE() == getOwnerINLINE()))
		{
			if (isCargo())
			{
				if (canJoinCity(plot()))
				{
					pCity->addPopulationUnit(this, NO_PROFESSION);
					return;
				}
			}
					
			if (!bDanger && ((getFortifyTurns() == 0) || (GC.getGameINLINE().getSorenRandNum(10, "AI better job")  == 0)))
			{
				if (AI_betterJob())
				{
					return;
				}
				if (canClearSpecialty())
				{
					clearSpecialty();
					return;
				}
			}
		}
	}
	
	if (isCargo())
	{
		if (AI_guardCityMinDefender())
		{
			return;
		}
		
		if (AI_unloadWhereNeeded())
		{
			return;
		}
		
		getGroup()->pushMission(MISSION_SKIP);
		return;
	}
		
	CvPlot* pMissionPlot = getGroup()->AI_getMissionAIPlot();
	
	if (bDanger)
	{
		if (AI_anyAttack(1, 90))
		{
			return;
		}
		
		if (AI_leaveAttack(1, 30, 120))
		{
			return;
		}
	}
	
	if (isHurt())
	{
		if (AI_heal())
		{
			return;
		}
	}

	if (bDanger)
	{
		if (AI_bombardCity())
		{
			return;
		}

		if (AI_anyAttack(2, 60))
		{
			return;
		}
	}
		
	if (AI_guardCityCounter())
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}

void CvUnitAI::AI_counterMove()
{
	PROFILE_FUNC();
	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
	bool bDanger = kOwner.AI_getPlotDanger(plot(), 2, false, true);
	
	if (!isHuman() && getProfession() != NO_PROFESSION)
	{
		CvCity* pCity = plot()->getPlotCity();
		if ((pCity != NULL) && (pCity->getOwnerINLINE() == getOwnerINLINE()))
		{
			if (isCargo())
			{
				if (canJoinCity(plot()))
				{
					pCity->addPopulationUnit(this, NO_PROFESSION);
					return;
				}
			}
					
			if (!bDanger && ((getFortifyTurns() == 0) || (GC.getGameINLINE().getSorenRandNum(10, "AI better job")  == 0)))
			{
				if (AI_betterJob())
				{
					return;
				}
				if (canClearSpecialty())
				{
					clearSpecialty();
					return;
				}
			}
		}
	}
	
	if (isCargo())
	{
		if (AI_guardCityMinDefender())
		{
			return;
		}
		
		if (AI_unloadWhereNeeded())
		{
			return;
		}
		
		getGroup()->pushMission(MISSION_SKIP);
		return;
	}
		
	CvPlot* pMissionPlot = getGroup()->AI_getMissionAIPlot();
	
	if (bDanger)
	{
		if (AI_anyAttack(3, 60))
		{
			return;
		}
		
		if (!(kOwner.AI_isStrategy(STRATEGY_REVOLUTION)))
		{
			if (AI_leaveAttack(1, 30, 120))
			{
				return;
			}
		}
		
		if (isHurt())
		{
			if (AI_heal())
			{
				return;
			}
		}
		
		if (bDanger && !(kOwner.AI_isStrategy(STRATEGY_REVOLUTION)))
		{
			if (AI_counter(6))
			{
				return;
			}
		}
		
		if (AI_guardCityCounter())
		{
			return;
		}
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}

void CvUnitAI::AI_defensiveBraveMove()
{
	FAssert(canMove());
	bool bAtWar = GET_TEAM(getTeam()).getAnyWarPlanCount();
	
	if (GC.getGameINLINE().getSorenRandNum(10, "AI upgrade unit profession") == 0)
	{
		AI_upgradeProfession();
	}
	
	AreaAITypes eAreaAI = area()->getAreaAIType(getTeam());
	
	FAssert(eAreaAI == AREAAI_NEUTRAL || eAreaAI == AREAAI_OFFENSIVE || eAreaAI == AREAAI_BALANCED || eAreaAI == AREAAI_DEFENSIVE);	
	
	CvCity* pCity = (plot()->getOwner() == getOwner()) ? plot()->getPlotCity() : NULL;
	
	CvPlayerAI& kOwner = GET_PLAYER(getOwner());
	
	bool bDanger = kOwner.AI_getPlotDanger(plot(), 2, false);
				
	if (pCity != NULL)
	{
		if (AI_guardCityBestDefender())
		{
			return;
		}
		
		if (bDanger)
		{
			if (AI_leaveAttack(1, 20 + GC.getGameINLINE().getSorenRandNum(30, "Native leave city attack"), 120))
			{
				return;
			}
		}
			
		if (pCity->AI_isDefended(-2))
		{
			if (AI_joinCityBrave())
			{
				return;
			}
		}
		
		if (getHomeCity() == NULL)
		{
			setHomeCity(pCity);
		}
		else if (pCity != getHomeCity())
		{
			if (getHomeCity()->AI_isDefended())
			{
				if (!pCity->AI_isDefended())
				{
					setHomeCity(pCity);
				}
			}
		}			
	
		if (pCity == getHomeCity())
		{
			if (pCity->AI_isDefended(-1))
			{
				if (AI_joinCityBrave())
				{
					return;
				}

				if ((pCity->AI_getGiftTimer() == 0) && pCity->AI_canMakeGift())
				{
					int iRand = GC.getLeaderHeadInfo(GET_PLAYER(getOwnerINLINE()).getPersonalityType()).getContactRand(CONTACT_YIELD_GIFT);
					iRand += std::max(0, 40 - GC.getGameINLINE().getGameTurn());
					if (iRand > GC.getGame().getSorenRandNum(100 * plot()->plotCount(PUF_isUnitAIType, UNITAI_DEFENSIVE, -1, getOwner()), "AI native bear gifts"))
					{
						AI_setUnitAIState(UNITAI_STATE_BEARING_GIFTS);
						pCity->AI_setGiftTimer(GC.getLeaderHeadInfo(GET_PLAYER(getOwnerINLINE()).getPersonalityType()).getContactDelay(CONTACT_YIELD_GIFT));
					}
				}
				else if (GC.getGame().getSorenRandNum(10, "AI native start wandering") == 0)
				{
					if (kOwner.AI_countNumHomedUnits(pCity, NO_UNITAI, UNITAI_STATE_WANDER) < 2)
					{
						AI_setUnitAIState(UNITAI_STATE_WANDER);				
						//fall through.
					}
				}
			}
		}
	}
		
	if (AI_getUnitAIState() == UNITAI_STATE_BEARING_GIFTS)
	{
		if (plot()->isCity() && (!GET_PLAYER(plot()->getOwner()).isNative()))
		{
			//Make gift and do diplomacy
			GET_PLAYER(getOwnerINLINE()).AI_nativeYieldGift(this);
			AI_setUnitAIState(UNITAI_STATE_RETURN_HOME);
			
			finishMoves();
			return;
		}
		else
		{
			if (AI_travelToEuropeColony(ATTITUDE_CAUTIOUS, ATTITUDE_FRIENDLY, 5)) //XXX XMLize the range?
			{
				return;
			}
			
			AI_setUnitAIState(UNITAI_STATE_RETURN_HOME);
		}
	}
		
	if (AI_getUnitAIState() == UNITAI_STATE_RETURN_HOME)
	{
		if (plot()->isCity() && (plot()->getPlotCity() == getHomeCity()))
		{
			AI_setUnitAIState(UNITAI_STATE_DEFAULT);
		}
		else if (AI_guardHomeColony())
		{
			return;
		}
	}
		
	if (AI_getUnitAIState() == UNITAI_STATE_WANDER)
	{
		if (AI_bravePatrol())
		{
			return;
		}
		if ((kOwner.AI_cityDistance(plot()) > 6) || (GC.getGameINLINE().getGameTurn() - AI_getLastAIChangeTurn() > 8))
		{
			AI_setUnitAIState(UNITAI_STATE_RETURN_HOME);
		}
	}
	
	if (kOwner.getCitiesLost() > 0)
	{
		if (kOwner.AI_getNumAIUnits(UNITAI_SETTLER) == 0)
		{
			if (area()->getBestFoundValue(getOwnerINLINE()) > 0)
			{
				if (kOwner.getTotalPopulation() >= 3 * kOwner.getNumCities())
				{
					if (pCity == NULL || pCity->AI_isDefended(-1))
					{
						AI_setUnitAIType(UNITAI_SETTLER);
						AI_settlerMove();
						return;
					}
				}
			}
		}
	}
		
	if (AI_guardHomeColony())
	{
		return;
	}
		
	if (AI_findNewHomeColony())
	{
		return;
	}
	
	if (getHomeCity() == NULL)
	{
		kill(true);//One way to deal with the homeless problem.
		return;
	}
		
	getGroup()->pushMission(MISSION_SKIP);
	return;
}

void CvUnitAI::AI_offensiveBraveMove()
{
	FAssert(canMove());
	bool bAtWar = GET_TEAM(getTeam()).getAnyWarPlanCount();
	
	if (GC.getGameINLINE().getSorenRandNum(10, "AI upgrade unit profession") == 0)
	{
		AI_upgradeProfession();
	}
	
	AreaAITypes eAreaAI = area()->getAreaAIType(getTeam());
	
	FAssert(eAreaAI == AREAAI_NEUTRAL || eAreaAI == AREAAI_OFFENSIVE || eAreaAI == AREAAI_BALANCED || eAreaAI == AREAAI_DEFENSIVE);	
	
	CvCity* pCity = (plot()->getOwner() == getOwner()) ? plot()->getPlotCity() : NULL;
	
	CvPlayerAI& kOwner = GET_PLAYER(getOwner());
				
	const int iLoiterDistance = 2;
	
	bool bDanger = kOwner.AI_getPlotDanger(plot(), 2, true, true) > 0;
	
	if (eAreaAI == AREAAI_OFFENSIVE || eAreaAI == AREAAI_BALANCED || eAreaAI == AREAAI_DEFENSIVE)
	{
		if (pCity != NULL)
		{
			if (bDanger)
			{
				if (AI_leaveAttack(2, 1 + GC.getGameINLINE().getSorenRandNum(30, "AI native leave attack"), 120))
				{
					return;
				}
			}
			
			if (pCity->AI_isDefended(-1))
			{
				AI_setUnitAIState(UNITAI_STATE_ADVANCING);
			}
			else
			{
				AI_setUnitAIType(UNITAI_DEFENSIVE);
				getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_GUARD_CITY, NULL);
				return;
			}
		}
		
		if (AI_getUnitAIState() == UNITAI_STATE_RETREATING)
		{
			if (!bDanger)
			{
				AI_setUnitAIState(UNITAI_STATE_ADVANCING);
			}
			else
			{
				if (AI_pillageRange(0))
				{
					return;
				}
				if (AI_anyAttack(1, 49))
				{
					return;
				}
				
				if (AI_guardHomeColony())
				{
					return;
				}
				
				if (AI_findNewHomeColony())
				{
					return;
				}
			}
		}
		
		if (AI_getUnitAIState() == UNITAI_STATE_GROUPING)
		{
			if (AI_anyAttack(1, 30))
			{
				return;
			}
			
			if (AI_pillageRange(1))
			{
				return;
			}
			if (AI_loiter(iLoiterDistance, iLoiterDistance, true))
			{
				return;				
			}
		}
		
		if (AI_getUnitAIState() == UNITAI_STATE_CHARGING)
		{			
			if (AI_anyAttack(1, 40))
			{
				return;
			}
			if (AI_pillageRange(0))
			{
				return;
			}
			if (AI_cityAttack(1, 10))
			{
				return;
			}
			if (AI_advance(true))
			{
				return;
			}
		}
		
		if (AI_getUnitAIState() == UNITAI_STATE_ADVANCING)
		{
			if (AI_anyAttack(1, 30))
			{
				return;
			}
			
			if (AI_pillageRange(1))
			{
				return;
			}
			
			CvTeamAI& kTeam = GET_TEAM(getTeam());
			if (AI_advance(true))
			{
				if (kTeam.AI_enemyCityDistance(plot()) <= iLoiterDistance)
				{
					AI_setUnitAIState(UNITAI_STATE_GROUPING);
				}
				return;
			}
			
			if (kTeam.AI_enemyCityDistance(plot()) <= iLoiterDistance)
			{
				AI_setUnitAIState(UNITAI_STATE_GROUPING);
			}
		}
	}
	else
	{
		AI_setUnitAIType(UNITAI_DEFENSIVE);
		return;
	}
			
	if (AI_guardHomeColony())
	{
		return;
	}
		
	if (AI_findNewHomeColony())
	{
		return;
	}
	
	if (getHomeCity() == NULL)
	{
		kill(true);//One way to deal with the homeless problem.
		return;
	}
		
	getGroup()->pushMission(MISSION_SKIP);
	return;
}

void CvUnitAI::AI_counterBraveMove()
{
	FAssert(canMove());
	bool bAtWar = GET_TEAM(getTeam()).getAnyWarPlanCount();
	
	if (GC.getGameINLINE().getSorenRandNum(10, "AI upgrade unit profession") == 0)
	{
		AI_upgradeProfession();
	}
	
	AreaAITypes eAreaAI = area()->getAreaAIType(getTeam());
	
	FAssert(eAreaAI == AREAAI_NEUTRAL || eAreaAI == AREAAI_OFFENSIVE || eAreaAI == AREAAI_BALANCED || eAreaAI == AREAAI_DEFENSIVE);	
	
	CvCity* pCity = (plot()->getOwner() == getOwner()) ? plot()->getPlotCity() : NULL;
	
	CvPlayerAI& kOwner = GET_PLAYER(getOwner());
				
	const int iLoiterDistance = 2;
	
	bool bDanger = kOwner.AI_getPlotDanger(plot(), 2, true, true) > 0;
	
	if (eAreaAI == AREAAI_OFFENSIVE || eAreaAI == AREAAI_BALANCED || eAreaAI == AREAAI_DEFENSIVE)
	{
		if (pCity != NULL)
		{
			if (bDanger)
			{
				if (AI_leaveAttack(2, 10 + GC.getGameINLINE().getSorenRandNum(30, "AI native leave attack"), 120))
				{
					return;
				}
			}
			
			if (!pCity->AI_isDefended(-1))
			{
				AI_setUnitAIType(UNITAI_DEFENSIVE);
				getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_GUARD_CITY, NULL);
				return;
			}
		}

		if (AI_pillageRange(1))
		{
			return;
		}
		
		if (AI_anyAttack(2, 20 + GC.getGameINLINE().getSorenRandNum(20, "AI counter attack")))
		{
			return;
		}
		
		if (AI_counter(5))
		{
			return;
		}
	}
	else
	{
		AI_setUnitAIType(UNITAI_DEFENSIVE);
		return;
	}
			
	if (AI_guardHomeColony())
	{
		return;
	}
		
	if (AI_findNewHomeColony())
	{
		return;
	}
	
	if (getHomeCity() == NULL)
	{
		kill(true);//One way to deal with the homeless problem.
		return;
	}
		
	getGroup()->pushMission(MISSION_SKIP);
	return;
}

void CvUnitAI::AI_transportMove()
{
	PROFILE_FUNC();
	
	CvCity* pCity = NULL;
	if (plot()->getPlotCity() != NULL)
	{
		if (plot()->getOwner() == getOwner())
		{
			pCity = plot()->getPlotCity();
		}
	}
	
	getGroup()->pushMission(MISSION_SKIP);
	return;	
}

void CvUnitAI::AI_transportMoveRoutes()
{
	
	if (AI_breakAutomation())
	{
		return;
	}
	
	if (AI_getUnitAIState() == UNITAI_STATE_SAIL)
	{
		if (AI_sailToEurope())
		{
			return;
		}
	}
	
	if (getGroup()->AI_tradeRoutes())
	{
		return;
	}
	
	if (AI_getUnitAIState() == UNITAI_STATE_SAIL)
	{
		if (AI_sailToEurope())
		{
			return;
		}
	}
	
	if (AI_retreatToCity())
	{
		return;
	}
	
	getGroup()->pushMission(MISSION_SKIP);
	return;
}

void CvUnitAI::AI_transportMoveFull()
{

	if (AI_breakAutomation())
	{
		return;
	}
	
	bool bEenish = false;
	if (AI_getUnitAIState() == UNITAI_STATE_SAIL)
	{
		if (AI_sailToEurope())
		{
			return;
		}
		bEenish = true;
	}
	
	if (getGroup()->AI_tradeRoutes())
	{
		return;
	}
	
	if (AI_getUnitAIState() == UNITAI_STATE_SAIL)
	{
		FAssertMsg(!bEenish, "Now this is confusing!!!");
		if (AI_sailToEurope())
		{
			return;
		}
	}
	
	if (AI_retreatToCity())
	{
		return;
	}
	
	getGroup()->pushMission(MISSION_SKIP);
	return;
	
}

void CvUnitAI::AI_imperialShipMove()
{
	
	CvPlayerAI& kOwner = GET_PLAYER(getOwner());
	int iMovePriority = AI_getMovePriority();
    bool bHasCargo = getGroup()->hasCargo();
    
	CvCity* pPlotCity = plot()->getPlotCity();
	if (pPlotCity != NULL && pPlotCity->getOwnerINLINE() != getOwnerINLINE())
	{
		pPlotCity = NULL;
	}
	
	if (GET_TEAM(getTeam()).getAnyWarPlanCount() == 0)
	{
		if (pPlotCity != NULL)
		{
			getGroup()->pushMission(MISSION_SKIP);
			return;
		}
		if (AI_retreatToCity())
		{
			return;
		}
		getGroup()->pushMission(MISSION_SKIP);
		return;
	}
	
	if (getGroup()->AI_getMissionAIType() == MISSIONAI_EXPLORE)
    {
    	if (AI_anyAttack(1, 1))
    	{
    		return;
    	}
    	
    	if (GC.getGame().getSorenRandNum(10, "AI wander aimlessly") != 0)
    	{
			if (AI_wanderAroundAimlessly())
			{
				return;
			}
    	}
    }
    
    //Move into range and bombard.
	if (bHasCargo)
	{
		if (AI_imperialSeaAssault())
		{
			return;
		}
	}
	else
	{
		if (isHurt())
		{
			if (AI_anyAttack(1, 70))
			{
				return;
			}

			if (AI_sailToEurope())
			{
				return;
			}			
		}
		if (AI_anyAttack(1, 30))
		{
			return;
		}
	}

	if (!kOwner.AI_isStrategy(STRATEGY_BUILDUP))
	{
		if (plot()->isAdjacentToLand())
		{
			if (canAttack())
			{
				if (AI_seaBombardRange(1))
				{
					return;
				}
			}

			if (AI_blockade(2))
			{
				return;
			}
		}
	}

	if (!bHasCargo)
	{
		if (kOwner.getNumEuropeUnits() > 0)
		{
			if (AI_sailToEurope())
			{
				if (kOwner.AI_isStrategy(STRATEGY_SMALL_WAVES) && (kOwner.AI_getStrategyDuration(STRATEGY_SMALL_WAVES) > 5))
				{
					kOwner.AI_clearStrategy(STRATEGY_SMALL_WAVES);
					kOwner.AI_setStrategy(STRATEGY_BUILDUP);
				}
				return;
			}
		}
	}
	
	if (AI_wanderAroundAimlessly())
	{
		return;
	}
    
    getGroup()->pushMission(MISSION_SKIP);
    return;
}

void CvUnitAI::AI_imperialSoldierMove()
{
	if (isCargo())
	{
		if (AI_anyAttack(1, 50))
		{
			return;
		}
		if (AI_disembark(false))
		{
			return;
		}
		
		getGroup()->pushMission(MISSION_SKIP);
		return;
	}
	
	CvCity* pPlotCity = plot()->getPlotCity();
	if (pPlotCity != NULL && pPlotCity->getOwnerINLINE() != getOwnerINLINE())
	{
		pPlotCity = NULL;
	}
	
	if (GET_TEAM(getTeam()).getAnyWarPlanCount() == 0)
    {
    	if (pPlotCity != NULL)
    	{
    		getGroup()->pushMission(MISSION_SKIP);
    		return;
    	}
    	if (AI_retreatToCity())
    	{
    		return;
    	}
    	getGroup()->pushMission(MISSION_SKIP);
    	return;
    }
    
	if (pPlotCity != NULL)
	{
		if (!pPlotCity->AI_isDefended(-1) || (isHurt()))
		{
			if (AI_smartAttack(1, 50, 95, NULL))
			{
				return;
			}
			getGroup()->pushMission(MISSION_SKIP);
			return;
		}
	}
	
	if (isHurt())
	{
		if (AI_retreat(4))
		{
			return;
		}
	}
	
	if (AI_anyAttack(1, 30))
	{
		return;
	}
	
	if (baseMoves() > 1)
	{
		if (AI_pillageRange(1))
		{
			return;
		}
	}
	
	int iAttackThreshold = GET_PLAYER(getOwnerINLINE()).AI_shouldAttackAdjacentCity(plot()) ? 0 : 30;
	if (AI_cityAttack(1, iAttackThreshold))
	{
		return;
	}
	
	if (AI_advance(false))
	{
		return;
	}
	
	if (AI_advance(true))
	{
		return;
	}
	
    if (AI_retreatToCity(false))
    {
        return;
    }
    
    getGroup()->pushMission(MISSION_SKIP);
    return;
}

void CvUnitAI::AI_imperialMountedMove()
{
	if (isCargo())
	{
		if (AI_anyAttack(1, 50))
		{
			return;
		}
		if (AI_disembark(false))
		{
			return;
		}
		
		getGroup()->pushMission(MISSION_SKIP);
		return;
	}
	
	CvCity* pPlotCity = plot()->getPlotCity();
	if (pPlotCity != NULL && pPlotCity->getOwnerINLINE() != getOwnerINLINE())
	{
		pPlotCity = NULL;
	}
	
	if (GET_TEAM(getTeam()).getAnyWarPlanCount() == 0)
    {
    	if (pPlotCity != NULL)
    	{
    		getGroup()->pushMission(MISSION_SKIP);
    		return;
    	}
    	if (AI_retreatToCity())
    	{
    		return;
    	}
    	getGroup()->pushMission(MISSION_SKIP);
    	return;
    }
    
	if (pPlotCity != NULL)
	{
		if (!pPlotCity->AI_isDefended(-1) || (isHurt()))
		{
			if (AI_smartAttack(1, 50, 95, NULL))
			{
				return;
			}
			getGroup()->pushMission(MISSION_SKIP);
			return;
		}
	}
	
	if (isHurt())
	{
		if (AI_retreat(4))
		{
			return;
		}
	}

	if (AI_anyAttack(1, 30))
	{
		return;
	}
	
	if (baseMoves() > 1)
	{
		if (AI_pillageRange(1))
		{
			return;
		}
	}
	
	int iAttackThreshold = GET_PLAYER(getOwnerINLINE()).AI_shouldAttackAdjacentCity(plot()) ? 0 : 40;
	if (AI_cityAttack(1, iAttackThreshold))
	{
		return;
	}
	
	if (AI_advance(false))
	{
		return;
	}
	
	if (AI_advance(true))
	{
		return;
	}
	
    if (AI_retreatToCity(false))
    {
        return;
    }
    
    getGroup()->pushMission(MISSION_SKIP);
    return;
}
void CvUnitAI::AI_imperialCannonMove()
{
	
	if (isCargo())
	{
		if (AI_disembark(false))
		{
			return;
		}
		
		getGroup()->pushMission(MISSION_SKIP);
		return;
	}
	
	CvCity* pPlotCity = plot()->getPlotCity();
	if (pPlotCity != NULL && pPlotCity->getOwnerINLINE() != getOwnerINLINE())
	{
		pPlotCity = NULL;
	}
	
	if (GET_TEAM(getTeam()).getAnyWarPlanCount() == 0)
    {
    	if (pPlotCity != NULL)
    	{
    		getGroup()->pushMission(MISSION_SKIP);
    		return;
    	}
    	if (AI_retreatToCity())
    	{
    		return;
    	}
    	getGroup()->pushMission(MISSION_SKIP);
    	return;
    }
    
	if (pPlotCity != NULL)
	{
		if (!pPlotCity->AI_isDefended(-1) || (isHurt()))
		{
			if (AI_smartAttack(1, 50, 95, NULL))
			{
				return;
			}
			getGroup()->pushMission(MISSION_SKIP);
			return;
		}
	}
	
	if (isHurt())
	{
		if (AI_retreat(4))
		{
			return;
		}
	}
	
	if (AI_bombardCity())
	{
		return;
	}
		
	if (isHurt())
	{
		if (AI_retreat(4))
		{
			return;
		}
	}
		
	int iAttackThreshold = GET_PLAYER(getOwnerINLINE()).AI_shouldAttackAdjacentCity(plot()) ? 0 : 30;
	
	if (AI_cityAttack(1, iAttackThreshold))
	{
		return;
	}
		
	if (AI_anyAttack(1, 50))
	{
		return;
	}
	
	if (AI_advance(false))
	{
		return;
	}
	
	if (AI_advance(true))
	{
		return;
	}	

    if (AI_retreatToCity(false))
    {
        return;
    }
    
    getGroup()->pushMission(MISSION_SKIP);
    return;
}
	
void CvUnitAI::AI_transportSeaMove()
{
	PROFILE_FUNC();

	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
	bool bEmpty = !getGroup()->hasCargo();

	CvCity* pCity = NULL;
	if (plot()->getPlotCity() != NULL)
	{
		if (AI_tradeWithCity())
		{
			return;
		}
		
		if (plot()->getOwner() == getOwner())
		{
			pCity = plot()->getPlotCity();
		}
	}
	

	int iGoodsCount = 0;
	UnitAIStates eStartingState = AI_getUnitAIState();
	if (AI_getUnitAIState() == UNITAI_STATE_SAIL)
	{
		if (AI_sailToEurope(false))
		{
			return;
		}
		if (AI_continueMission(-1, MISSIONAI_SAIL_TO_EUROPE, MOVE_BUST_FOG))
		{
			return;
		}
	
		if (kOwner.AI_totalUnitAIs(UNITAI_TREASURE) > 0)
		{
			if (AI_respondToPickup(2, UNITAI_TREASURE))
			{
				return;
			}
		}
		
		if (pCity != NULL)
		{
			AI_collectGoods();
		}
	
		if (kOwner.AI_totalUnitAIs(UNITAI_TREASURE) > 0)
		{
			if (AI_respondToPickup(10, UNITAI_TREASURE))
			{
				return;
			}
		}

		if (isFull())
		{
			if (AI_sailToEurope())
			{
				return;
			}
		}
		else
		{		
		if (AI_travelToPort(15, 4))
		{
			return;
		}
		}
		
		if (hasCargo() || (kOwner.getNumEuropeUnits() > 0))
		{
			if (AI_sailToEurope())
			{
				return;
			}
		}
		AI_setUnitAIState(UNITAI_STATE_DEFAULT);
	}
	
	if (AI_deliverUnits())
	{
		return;
	}

	if (AI_getUnitAIState() == UNITAI_STATE_PICKUP)
	{
		if (AI_respondToPickup(2))
		{
			return;
		}
		
		if (AI_deliverUnits())
		{
			return;
		}
		
		if (bEmpty)
		{
			AI_setUnitAIState(UNITAI_STATE_DEFAULT);
		}
	}
	
	if (bEmpty)
	{
		if (AI_respondToPickup(5))
		{
			AI_setUnitAIState(UNITAI_STATE_PICKUP);
			return;
		}
	}
	
	int iNativeSaleGoods = 0;
	int iSettlerCount = 0;
	if (!bEmpty)
	{
		CLLNode<IDInfo>* pUnitNode;
		CvUnit* pLoopUnit;
		CvPlot* pPlot;
		int iCount;

		iCount = 0;

		pPlot = plot();

		pUnitNode = pPlot->headUnitNode();

		while (pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = pPlot->nextUnitNode(pUnitNode);

			if (pLoopUnit->getTransportUnit() == this)
			{
				if (pLoopUnit->getYield() != NO_YIELD)
				{
					FAssert(pLoopUnit->getYieldStored() > 0);
					iGoodsCount++;
					if (pLoopUnit->AI_getUnitAIState() == UNITAI_STATE_SELL_TO_NATIVES)
					{
						iNativeSaleGoods++;
					}
				}
				if (pLoopUnit->AI_getUnitAIType() == UNITAI_SETTLER)
				{
					iSettlerCount++;
				}
			}
		}
	}
	
	if (kOwner.AI_totalUnitAIs(UNITAI_TREASURE) > 0)
	{
		if (AI_respondToPickup(MAX_INT, UNITAI_TREASURE))
		{
			AI_setUnitAIState(UNITAI_STATE_SAIL);
			return;
		}

	}
	
	if (bEmpty && GC.getGameINLINE().getSorenRandNum(100, "AI Respond to Pickup 1") < 25)
	{
		if (AI_respondToPickup())
		{
			AI_setUnitAIState(UNITAI_STATE_PICKUP);
			return;
		}
	}

	bool bRoutes = (AI_getUnitAIType() == UNITAI_TRANSPORT_SEA);
	if (iNativeSaleGoods == 0 && iGoodsCount > 0)
	{
		if (getGroup()->AI_tradeRoutes())
		{
			return;
		}
		bRoutes = false;
	}

	if (iSettlerCount > 0 || iNativeSaleGoods > 0 || ((kOwner.getNumEuropeUnits() == 0) && (kOwner.AI_countYieldWaiting() < 3)))
	{
		if (AI_exploreCoast(2))
		{
			return;
		}
		if (AI_exploreOcean(1))
		{
			return;
		}
		if (AI_exploreDeep())
		{
			return;
		}
	}
	
	int iCargoWaiting = kOwner.AI_countYieldWaiting();
	if (iCargoWaiting < (cargoSpace() * GC.getGameINLINE().getCargoYieldCapacity()))
	{
		if (bRoutes)
		{
			if (AI_isObsoleteTradeShip())
			{
				if (getGroup()->AI_tradeRoutes())
				{
					return;
				}
				bRoutes = false;
			}
		}
	}
	
	if (pCity != NULL)
	{
		if (AI_collectGoods())
		{
			AI_setUnitAIState(UNITAI_STATE_SAIL);
			return;
		}
	}
	
	if (!isFull())
	{
		if (AI_travelToPort(40))
		{
			AI_setUnitAIState(UNITAI_STATE_SAIL);
			return;
		}
	}
	
	if (iNativeSaleGoods > 0)
	{
		if (AI_exploreCoast(2))
		{
			return;
		}
		if (AI_exploreDeep())
		{
			return;
		}
	}
	if ((GET_PLAYER(getOwnerINLINE()).getNumEuropeUnits() > 0) || isFull())
	{
		FAssert(AI_getUnitAIState() != UNITAI_STATE_SAIL);
		AI_setUnitAIState(UNITAI_STATE_SAIL);
		return;
	}
	
	CvArea* pArea = area();
	if (!pArea->isWater())
	{
		pArea = plot()->waterArea();		
	}
	FAssert(pArea != NULL);
	if (pArea != NULL && pArea->getNumTiles() - pArea->getNumRevealedTiles(getTeam()) > 0)
	{
		if (AI_exploreCoast(2))
		{
			return;
		}
		if (AI_exploreDeep())
		{
			return;
		}
	}
	
	if (bRoutes)
	{
		if (getGroup()->AI_tradeRoutes())
		{
			return;
		}
	}

	if (AI_retreatToCity(true))
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}

void CvUnitAI::AI_assaultSeaMove()
{
	AI_transportSeaMove();
	return;
}

void CvUnitAI::AI_combatSeaMove()
{
	if (AI_anyAttack(2, 49))
	{
		return;
	}
	
	if (isHurt())
	{
		if (AI_heal())
		{
			return;
		}
		
		if (AI_retreatToCity())
		{
			return;
		}
	}
	
	if (plot()->getDistanceToOcean() == 0)
	{
		if (AI_moveTowardsVictimCity())
		{
			return;
		}
	}

	if (AI_moveTowardsOcean(1))
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;	
}

void CvUnitAI::AI_pirateMove()
{
	if (AI_anyAttack(2, 49))
	{
		return;
	}
	
	if (isHurt())
	{
		if (AI_heal())
		{
			return;
		}
		
		if (AI_retreatToCity())
		{
			return;
		}
	}
	
	if (AI_shouldRun())
	{
		AI_setUnitAIState(UNITAI_STATE_RETREATING);
	}
	
	if (AI_getUnitAIState() == UNITAI_STATE_RETREATING)
	{
		if (AI_retreatToCity())
		{
			if (plot()->isCity())
			{
				AI_setUnitAIState(UNITAI_STATE_DEFAULT);
				getGroup()->pushMission(MISSION_SKIP);
				return;
			}
			return;
		}
	}
	
	if (AI_continueMission(0, MISSIONAI_PIRACY, 0))
	{
		return;
	}
	
	if (plot()->getDistanceToOcean() == 0)
	{
		if (AI_moveTowardsVictimCity())
		{
			return;
		}
	}

	if (AI_moveTowardsOcean(1))
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;	
}




void CvUnitAI::AI_exploreMove()
{
	PROFILE_FUNC();
	
	if (AI_breakAutomation())
	{
		return;
	}

	if (!isHuman() && canAttack())
	{
		if (AI_cityAttack(1, 60))
		{
			return;
		}

		if (AI_anyAttack(1, 70))
		{
			return;
		}
	}

	if (getDamage() > 0)
	{
		getGroup()->pushMission(MISSION_HEAL);
		return;
	}

	if (AI_goody())
	{
		return;
	}

	if (AI_exploreRange(3))
	{
		return;
	}

	if (AI_patrol())
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}

void CvUnitAI::AI_exploreSeaMove()
{
	PROFILE_FUNC();
	
	if (AI_breakAutomation())
	{
		return;
	}
	
	if (AI_exploreCoast(2))
	{
		return;
	}
	
	if (AI_exploreOcean(1))
	{
		return;
	}
	
	if (AI_exploreDeep())
	{
		return;
	}
	
	getGroup()->pushMission(MISSION_SKIP);
	return;
}

void CvUnitAI::AI_networkAutomated()
{
	FAssertMsg(canBuildRoute(), "canBuildRoute is expected to be true");

	if (!(getGroup()->canDefend()))
	{
		if (GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot()) > 0)
		{
			if (AI_retreatToCity()) // XXX maybe not do this??? could be working productively somewhere else...
			{
				return;
			}
		}
	}

	if (AI_routeTerritory(true))
	{
		return;
	}

	if (AI_routeCity())
	{
		return;
	}

	if (AI_routeTerritory())
	{
		return;
	}
	
	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_cityAutomated()
{
	CvCity* pCity;

	if (!(getGroup()->canDefend()))
	{
		if (GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot()) > 0)
		{
			if (AI_retreatToCity()) // XXX maybe not do this??? could be working productively somewhere else...
			{
				return;
			}
		}
	}

	pCity = NULL;

	if (plot()->getOwnerINLINE() == getOwnerINLINE())
	{
		pCity = plot()->getWorkingCity();
	}

	if (pCity == NULL)
	{
		pCity = GC.getMapINLINE().findCity(getX_INLINE(), getY_INLINE(), getOwnerINLINE()); // XXX do team???
	}

	if (pCity != NULL)
	{
		if (AI_improveCity(pCity))
		{
			return;
		}
	}
	
	if (AI_retreatToCity())
	{
		if (canJoinCity(plot()))
		{
			joinCity();	
		}
		return;
	}
	
	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}

bool CvUnitAI::AI_travelToEuropeColony(int iMinAttitude, int iMaxAttitude, int iRange)
{
	int iX, iY;
	if (iRange == -1)
	{
		iRange = 5;
	}
	
	CvPlot* pBestPlot = NULL;
	int iBestValue = 0;
	
	CvPlayer& kOwner = GET_PLAYER(getOwner());
	
	for (iX = -iRange; iX <= iRange; iX++)
	{
		for (iY = -iRange; iY <= iRange; iY++)
		{
			//Should this always be centered on the home city?
			CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iX, iY);
			
			if (NULL != pLoopPlot)
			{
				if (pLoopPlot->isCity())
				{
					CvPlayer& kLoopPlayer = GET_PLAYER(pLoopPlot->getOwner());
					
					if (!kLoopPlayer.isNative())
					{
						int iAttitude = kOwner.AI_getAttitude(kLoopPlayer.getID());
						if (iAttitude >= iMinAttitude && iAttitude <= iMaxAttitude)
						{
							int iPathTurns;
							
							if (generatePath(pLoopPlot, 0, true, &iPathTurns))
							{
								int iValue = 5000;
								iValue += GC.getGame().getSorenRandNum(10000, "AI target colony");
								
								iValue /= 3 + iPathTurns;
								
								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									pBestPlot = pLoopPlot;
								}								
							}
						}
					}
				}
			}
		}
	}
	
	if (pBestPlot != NULL)
	{
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
		return true;
	}
	
	return false;	
}

bool CvUnitAI::AI_guardHomeColony()
{
	if (getHomeCity() != NULL)
	{
		CvPlot* pHomePlot = getHomeCity()->plot();
		if (atPlot(pHomePlot))
		{
				getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_GUARD_CITY, NULL);
				return true;
			}
			else
			{
				if (generatePath(pHomePlot, 0, true))
				{
					getGroup()->pushMission(MISSION_MOVE_TO, pHomePlot->getX_INLINE(), pHomePlot->getY_INLINE(), 0, false, false, MISSIONAI_GUARD_CITY, NULL);
					return true;				
				}
			}
		}
		return false;
	}

bool CvUnitAI::AI_findNewHomeColony()
{
	int iBestValue = MAX_INT;
	CvCity* pBestCity = NULL;
	
	CvPlayer& kPlayer = GET_PLAYER(getOwnerINLINE());
	int iLoop;
	for (CvCity* pLoopCity = kPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kPlayer.nextCity(&iLoop))
	{
		int iPathTurns;
		if (generatePath(pLoopCity->plot(), 0, true, &iPathTurns))
		{
			int iValue = iPathTurns * (25 + GC.getGameINLINE().getSorenRandNum(75, "AI Find New Home"));
			
			if (iValue < iBestValue)
			{
				iBestValue = iValue;
				pBestCity = pLoopCity;
			}
		}
	}
	
	if (pBestCity != NULL)
	{
		setHomeCity(pBestCity);
		if (generatePath(getHomeCity()->plot(), 0, true))
		{
			getGroup()->pushMission(MISSION_MOVE_TO, getHomeCity()->getX_INLINE(), getHomeCity()->getY_INLINE());
			return true;
		}
	}

	return false;
}

bool CvUnitAI::AI_europeBuyNativeYields()
{
	bool bPurchased = false;
	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
	CvPlayer& kPlayerEurope = GET_PLAYER(kOwner.getParent());
	
	
	std::vector<int> yields;
	
	yields.push_back(YIELD_MUSKETS);
	if (GC.getGameINLINE().getSorenRandNum(2, "AI buy horses") == 0)
	{
		yields.push_back(YIELD_HORSES);
	}
	
	GC.getGameINLINE().getSorenRand().shuffleArray(yields, NULL);
	
	for (uint i = 0; i < yields.size(); ++i)
	{
		YieldTypes eLoopYield = (YieldTypes)yields[i];
		if (kOwner.isYieldEuropeTradable(eLoopYield))
		{
			int iAmount = getLoadYieldAmount(eLoopYield);
			int iPrice = iAmount * kPlayerEurope.getYieldSellPrice(eLoopYield);
			
			int iAvailableGold = kOwner.getGold() - 50;
			
			if (iPrice > iAvailableGold)
			{
				iAmount = iAvailableGold / kPlayerEurope.getYieldSellPrice(eLoopYield);
			}
			if (iAmount > 0)
			{
				CvUnit* pYieldUnit = kOwner.buyYieldUnitFromEurope(eLoopYield, iAmount, this);
				if (pYieldUnit != NULL)
				{
					pYieldUnit->AI_setUnitAIState(UNITAI_STATE_SELL_TO_NATIVES);
				}
				bPurchased = true;
			}
		}
	}
	
	return bPurchased;
}



bool CvUnitAI::AI_europeBuyYields()
{
	
	
	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
	CvPlayer& kPlayerEurope = GET_PLAYER(kOwner.getParent());
	
	int aiYields[NUM_YIELD_TYPES];
	for (int iYield = 0; iYield < NUM_YIELD_TYPES; iYield++)
	{
		aiYields[iYield] = 0;
	}
	
	int aiTotalYields[NUM_YIELD_TYPES];
	kOwner.calculateTotalYields(aiTotalYields);
	
	for (int iYield = 0; iYield < NUM_YIELD_TYPES; iYield++)
	{
		YieldTypes eLoopYield = (YieldTypes)iYield;
		if (kOwner.AI_shouldBuyFromEurope(eLoopYield))
		{
			if (kOwner.isYieldEuropeTradable(eLoopYield))
			{
				int iLoop;
				CvCity* pLoopCity;
				for (pLoopCity = kOwner.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kOwner.nextCity(&iLoop))
				{	
					int iNeeded = pLoopCity->getMaintainLevel(eLoopYield);
					if (pLoopCity->getYieldStored(eLoopYield) < iNeeded)
					{			
						aiYields[eLoopYield] += iNeeded;
					}
				}
			}
		}
	}

	std::vector<int> yieldRandomizer(NUM_YIELD_TYPES);
	for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
	{
		yieldRandomizer[iYield] = iYield;
	}
	
	GC.getGameINLINE().getSorenRand().shuffleArray(yieldRandomizer, NULL);

	for (int iPass = 0; iPass < 2; ++iPass)
	{
		for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
		{
			YieldTypes eLoopYield = (YieldTypes)yieldRandomizer[iYield];
			if (aiYields[eLoopYield] > 0)
			{
				int iMax = getLoadYieldAmount(eLoopYield);
				int iMin = std::min(iMax, aiYields[eLoopYield]);
				
				
				int iMinPrice = iMin * kPlayerEurope.getYieldSellPrice(eLoopYield);
				int iMaxPrice = iMax * kPlayerEurope.getYieldSellPrice(eLoopYield);
				
				int iAmount = 0;
				int iAvailableGold = kOwner.getGold() - 50;
				
				if (iMinPrice > iAvailableGold)
				{
					return false;
				}
				if (iMaxPrice <= iAvailableGold)
				{
					iAmount = iMax;
				}
				else
				{
					iAmount = iMin;
				}
				if (iAmount > 0)
				{
					CvUnit* pYieldUnit = kOwner.buyYieldUnitFromEurope(eLoopYield, iAmount, this);
					if (pYieldUnit != NULL)
					{
						pYieldUnit->AI_setUnitAIState(UNITAI_STATE_PURCHASED);
						aiYields[eLoopYield] -= iAmount;
					}
					if (isFull())
					{
						break;
					}
				}
			}
		}
	}
	
	return false;
}


bool CvUnitAI::AI_europe()
{
	
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pPlot;
	int iCount;

	iCount = 0;

	pPlot = plot();

	pUnitNode = pPlot->headUnitNode();

	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
	
	//Sell to Europe
	std::vector<CvUnit*> apUnits;
	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pPlot->nextUnitNode(pUnitNode);

		if (pLoopUnit->getTransportUnit() == this)
		{
			YieldTypes eYield = pLoopUnit->getYield();
			if (pLoopUnit->isGoods() || pLoopUnit->getUnitInfo().isTreasure())
			{
				if ((NO_YIELD == eYield) || kOwner.isYieldEuropeTradable(eYield))
				{
					apUnits.push_back(pLoopUnit);
				}
			}
		}
	}

	for (uint i = 0; i < apUnits.size(); ++i)
	{
		kOwner.sellYieldUnitToEurope(apUnits[i], apUnits[i]->getYieldStored(), 0);
	}
	
	kOwner.AI_doEurope();
	
	//Pick up units from Europe (FIFO)
	std::deque<CvUnit*> aUnits;
	for (int i = 0; i < kOwner.getNumEuropeUnits(); ++i)
	{
		aUnits.push_back(kOwner.getEuropeUnit(i));
	}

	while (!aUnits.empty() && !isFull())
	{
		CvUnit* pUnit = aUnits.front();
		aUnits.pop_front();
		if (pUnit->canLoadUnit(this, plot(), false))
		{
			kOwner.loadUnitFromEurope(pUnit, this);
			if (getGroup()->getAutomateType() == AUTOMATE_FULL)
			{
				FAssert(pUnit->getGroup() != NULL);
				pUnit->getGroup()->setAutomateType(AUTOMATE_FULL);
			}
		}
	}
	
	if (kOwner.AI_isStrategy(STRATEGY_SELL_TO_NATIVES) && GC.getGameINLINE().getSorenRandNum(2, "AI sell to natives") == 0)
	{
		if ((AI_getUnitAIType() == UNITAI_TRANSPORT_SEA) || (AI_getUnitAIType() == UNITAI_ASSAULT_SEA && GET_TEAM(getTeam()).getAnyWarPlanCount() == 0))
		{
			if (cargoSpace() < 5)
			{
				AI_europeBuyNativeYields();
			}
		}
	}
	
	AI_europeBuyYields();
	
	FAssert(plot()->isEurope());
	FAssert(canCrossOcean(plot(), UNIT_TRAVEL_STATE_FROM_EUROPE));

	crossOcean(UNIT_TRAVEL_STATE_FROM_EUROPE);
	finishMoves();
	return true;
}

bool CvUnitAI::AI_europeAssaultSea()
{
	
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pPlot;
	int iCount;

	iCount = 0;

	pPlot = plot();

	pUnitNode = pPlot->headUnitNode();

	CvPlayer& kOwner = GET_PLAYER(getOwnerINLINE());
	
	//Sell to Europe
	std::vector<CvUnit*> apUnits;
	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pPlot->nextUnitNode(pUnitNode);

		if (pLoopUnit->getTransportUnit() == this)
		{
			YieldTypes eYield = pLoopUnit->getYield();
			if (NO_YIELD != eYield)
			{
				if (kOwner.isYieldEuropeTradable(eYield))
				{
					apUnits.push_back(pLoopUnit);
				}
			}
		}
	}

	for (uint i = 0; i < apUnits.size(); ++i)
	{
		kOwner.sellYieldUnitToEurope(apUnits[i], apUnits[i]->getYieldStored(), 0);
	}
	
	//Pick up units from Europe (FIFO)
	while (kOwner.getNumEuropeUnits() > 0)
	{
		if (isFull())
		{
			break;
		}
		
		CvUnit* pUnit = kOwner.getEuropeUnit(0);
		kOwner.loadUnitFromEurope(pUnit, this);
		if (getGroup()->getAutomateType() == AUTOMATE_FULL)
		{
			FAssert(pUnit->getGroup() != NULL);
			pUnit->getGroup()->setAutomateType(AUTOMATE_FULL);
		}
	}
	
	FAssert(plot()->isEurope());
	FAssert(canCrossOcean(plot(), UNIT_TRAVEL_STATE_FROM_EUROPE));

	crossOcean(UNIT_TRAVEL_STATE_FROM_EUROPE);
	finishMoves();
	return true;
}

bool CvUnitAI::AI_sailToEurope(bool bMove)
{
	
	if (canCrossOcean(plot(), UNIT_TRAVEL_STATE_TO_EUROPE))
	{
		crossOcean(UNIT_TRAVEL_STATE_TO_EUROPE);
		if (AI_getUnitAIState() == UNITAI_STATE_SAIL)
		{
			AI_setUnitAIState(UNITAI_STATE_DEFAULT);
		}

		if (getGroup()->getAutomateType() == AUTOMATE_SAIL)
		{
			getGroup()->setAutomateType(NO_AUTOMATE);
		}
		return true;
	}
	
	if (!bMove)
	{
		return false;
	}
	
	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	CvPlot* pBestMissionPlot = NULL;
	
	for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);
		
		if (AI_plotValid(pLoopPlot) && !pLoopPlot->isVisibleEnemyDefender(this))
		{
			if (pLoopPlot->isRevealed(getTeam(), false))
			{
				if (canCrossOcean(pLoopPlot, UNIT_TRAVEL_STATE_TO_EUROPE))
				{
					int iPathTurns;
					if (generatePath(pLoopPlot, MOVE_BUST_FOG, true, &iPathTurns))
					{
						int iValue = 10000;
						iValue /= 100 + getPathCost();
						
						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							pBestPlot = getPathEndTurnPlot();
							pBestMissionPlot = pLoopPlot;
						}
					}
				}
			}
		}
	}

	
	if (pBestPlot != NULL)
	{
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), MOVE_BUST_FOG, false, false, MISSIONAI_SAIL_TO_EUROPE, pBestMissionPlot);
		if (plot()->isEurope())
		{
			if (canCrossOcean(plot(), UNIT_TRAVEL_STATE_TO_EUROPE))
			{
				crossOcean(UNIT_TRAVEL_STATE_TO_EUROPE);
				if (AI_getUnitAIState() == UNITAI_STATE_SAIL)
				{
					AI_setUnitAIState(UNITAI_STATE_DEFAULT);
				}
				if (getGroup()->getAutomateType() == AUTOMATE_SAIL)
				{
					getGroup()->setAutomateType(NO_AUTOMATE);
				}
			}
		}
		return true;
	}
	return false;
}

bool CvUnitAI::AI_travelToPort(int iMinPercent, int iMaxPath)
{
	PROFILE_FUNC();
	//Later on I'll make it so it can completely full yield units, for now though not.
	FAssert(!isFull());
	
	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	
	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
	CvPlayerAI& kEuropePlayer = GET_PLAYER(kOwner.getParent());
	CvCity* pLoopCity;
	int iLoop;
	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		if (!atPlot(pLoopCity->plot()) && AI_plotValid(pLoopCity->plot()))
		{
			if (!(pLoopCity->plot()->isVisibleEnemyUnit(this)))
			{
				if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopCity->plot(), MISSIONAI_TRANSPORT_SEA, getGroup(), 1) == 0)
				{
					
					int iPathTurns;
					if (generatePath(pLoopCity->plot(), 0, true, &iPathTurns))
					{
						if (iPathTurns <= iMaxPath)
						{
							
							int iBestYieldValue = 0;
							//Nothing too fancy, just find the best yield and best quantity.
							for (int i = 0; i < NUM_YIELD_TYPES; i++)
							{
								YieldTypes eYield = (YieldTypes)i;
								if (kOwner.isYieldEuropeTradable(eYield))
								{
									if (kOwner.AI_isYieldForSale(eYield))
									{
										int iStored = pLoopCity->getYieldStored(eYield);
										if (iStored >= ((GC.getGameINLINE().getCargoYieldCapacity() * iMinPercent) / 100 ))
										{
											int iYieldValue = iStored * kEuropePlayer.getYieldBuyPrice(eYield);
											iBestYieldValue = std::max(iYieldValue, iBestYieldValue);
										}
									}
								}
							}
						
							if (iBestYieldValue > 0)
							{
								int iValue = (100000 * iBestYieldValue);
								
								iValue /= 100 + getPathCost();								
								iValue /= 3 + pLoopCity->plot()->getDistanceToOcean();
								
								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									pBestPlot = getPathEndTurnPlot();
								}
							}
						}
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_TRANSPORT_SEA, pBestPlot);
		return true;
	}

	return false;
}
	
bool CvUnitAI::AI_collectGoods()
{
	bool bLoaded = false;
	CvCity* pCity = plot()->getPlotCity();

	FAssert(pCity != NULL);
	FAssert(pCity->getOwner() == getOwner()); // team?
	
	CvPlayerAI& kOwner = GET_PLAYER(getOwner());
	CvPlayerAI& kEuropePlayer = GET_PLAYER(kOwner.getParent());
	
	//First try and load any additional cargo.
	for (int i = 0; i < NUM_YIELD_TYPES; i++)
	{
		YieldTypes eYield = (YieldTypes)i;
		if (kOwner.isYieldEuropeTradable(eYield))
		{
			if (kOwner.AI_isYieldForSale(eYield))
			{
				YieldTypes eYield = (YieldTypes)i;
				int iYieldStored = getLoadedYieldAmount(eYield) % GC.getGameINLINE().getCargoYieldCapacity();
				
				if (iYieldStored > 0)
				{
					if (pCity->getYieldStored(eYield) > 0)
					{
						loadYield(eYield, false);
						bLoaded = true;
					}
				}
			}
		}
	}
	
	//Now load full units (or all which is available)
	while (!isFull())
	{
		YieldTypes eBestYield = NO_YIELD;
		int iBestYieldValue = 0;

		for (int i = 0; i < NUM_YIELD_TYPES; i++)
		{
			YieldTypes eYield = (YieldTypes)i;
			if (kOwner.isYieldEuropeTradable(eYield))
			{
				if (kOwner.AI_isYieldForSale(eYield))
				{
					int iStored = pCity->getYieldStored(eYield) - pCity->getMaintainLevel(eYield);
					if (iStored > (GC.getGameINLINE().getCargoYieldCapacity() / 10))
					{
						int iYieldValue = iStored * kEuropePlayer.getYieldBuyPrice(eYield);
						if (iYieldValue > iBestYieldValue)
						{
							iBestYieldValue =iYieldValue;
						eBestYield = eYield;
					}
				}
			}
		}
		}
		
		if (eBestYield == NO_YIELD)
		{
			break;
		}
		
		loadYield(eBestYield, false);
		bLoaded = true;
	}
	
	return bLoaded;
}

bool CvUnitAI::AI_deliverUnits()
{
	
	CvUnit* pColonistUnit = NULL;
	
	int iBestValue = 0;
	CvPlot* pBestDestination = NULL;
	CvPlot* pBestMissionPlot = NULL;
	
	{
		CLLNode<IDInfo>* pUnitNode;
		CvUnit* pLoopUnit;
		CvPlot* pPlot;
		int iCount;

		iCount = 0;

		pPlot = plot();

		pUnitNode = pPlot->headUnitNode();

		while (pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = pPlot->nextUnitNode(pUnitNode);

			if (pLoopUnit->getTransportUnit() == this)
			{
				CvPlot* pDestination = NULL;
				CvPlot* pMissionPlot = NULL;
				
				if (pLoopUnit->getGroup()->AI_getMissionAIType() != NO_MISSIONAI)
				{
					CvPlot* pDestination = pLoopUnit->getGroup()->AI_getMissionAIPlot();

					if ((pDestination != NULL) && (pDestination != plot())) //Don't humor foolishness x[.
					{
						int iPathTurns;
						
						if (generatePath(pDestination, 0, true, &iPathTurns))
						{
							int iValue = 100000;
							iValue /= 100 + getPathCost();
							
							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								pBestDestination = canMoveInto(pDestination) ? pDestination : getGroup()->getPathSecondLastPlot();	
							}
						}
					}
				}
			}
		}
	}
	
	if (iBestValue > 0)
	{
		getGroup()->pushMission(MISSION_MOVE_TO, pBestDestination->getX_INLINE(), pBestDestination->getY_INLINE());

		bool bWaitForCargo = AI_wakeCargo(NO_UNITAI, AI_getMovePriority() + 1);

		if (bWaitForCargo)
		{
			if (canMove())
			{
				AI_setMovePriority(AI_getMovePriority() - 1);
			}
		}
		return true;
	}
	return false;
}

//Returns true if any units are loaded.
bool CvUnitAI::AI_loadUnits(UnitAITypes eUnitAI, MissionAITypes eMissionAI)
{
	CvPlot * pPlot = plot();
	CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();

	int iCount = 0;
	while (pUnitNode != NULL)
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pPlot->nextUnitNode(pUnitNode);
		
		if (!pLoopUnit->isCargo())
		{
			if ((eUnitAI == NO_UNITAI) || (pLoopUnit->AI_getUnitAIType() == eUnitAI))
			{
				if ((eMissionAI == NO_MISSIONAI) || (pLoopUnit->getGroup()->AI_getMissionAIType() == eMissionAI))
				{
					if (pLoopUnit->canLoadUnit(this, plot(), true))
					{
						pLoopUnit->loadUnit(this);
						iCount++;
						
						if (!isHuman())
						{
							GET_PLAYER(getOwnerINLINE()).AI_removeUnitFromMoveQueue(pLoopUnit);
						}
					}
				}
			}
		}
	}
	return (iCount > 0);
}

bool CvUnitAI::AI_wakeCargo(UnitAITypes eUnitAI, int iPriority)
{
	bool bWaitForCargo = false;
	CvPlot* pPlot = plot();

	CLLNode<IDInfo>*  pUnitNode = plot()->headUnitNode();
	while (pUnitNode != NULL)
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = plot()->nextUnitNode(pUnitNode);
			
		if (pLoopUnit->isCargo() && (pLoopUnit->getTransportUnit()->getGroup() == getGroup()))
		{
			if (eUnitAI == NO_UNITAI || pLoopUnit->AI_getUnitAIType() == eUnitAI)
			{
				if (pLoopUnit->canMove())
				{
				pLoopUnit->AI_setMovePriority(AI_getMovePriority() + 1);
				}
				bWaitForCargo = true;
			}
		}
	}
	return bWaitForCargo;
}

//This function is used for units on board transports, essentially they may decide
//where they want to go.
//The mission MUST be one with a plot destination.
CvPlot* CvUnitAI::AI_determineDestination(CvPlot** ppMissionPlot, MissionTypes* peMission, MissionAITypes* peMissionAI)
{
	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	
	CvUnit* pTransport = getTransportUnit();
	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());
	FAssert(pTransport != NULL);

	if (AI_getUnitAIType() == UNITAI_SCOUT)
	{
		
		for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
		{
			CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);
			CvArea* pArea = pLoopPlot->area();
			if (!pArea->isWater() && pArea->getNumUnrevealedTiles(getTeam()) > 0)
			{
				if (pLoopPlot->isRevealed(getTeam(), false))
				{
					int iUnrevealedCount = 0;
					int iWaterCount = 0;
					for (int iJ = 0; iJ < NUM_DIRECTION_TYPES; iJ++)
					{
						CvPlot* pLoopPlot2 = plotDirection(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), (DirectionTypes)iJ);
						
						if (pLoopPlot2 != NULL)
						{
							if (!pLoopPlot2->isWater())
							{
								if (!pLoopPlot2->isRevealed(getTeam(), false))
								{
									iUnrevealedCount++;
									if (iWaterCount > 0)
									{
										break;
									}
								}
							}
							else
							{
								iWaterCount++;
								if (iUnrevealedCount > 0)
								{
									break;
								}
							}
						}
					}
					if ((iUnrevealedCount > 0) && (iWaterCount > 0))
					{
						if (kPlayer.AI_areaMissionAIs(pArea, MISSIONAI_EXPLORE, pTransport->getGroup()) == 0)
						{
							int iPathTurns;
							if (pTransport->generatePath(pLoopPlot, 0, true, &iPathTurns))
							{
								int iValue = 100000 / (100 + getPathCost());
								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									pBestPlot = pTransport->canMoveInto(pLoopPlot) ? pLoopPlot : pTransport->getGroup()->getPathSecondLastPlot();
									if (ppMissionPlot != NULL)
									{
										*ppMissionPlot = pLoopPlot;
									}
									if (peMissionAI != NULL)
									{
										*peMissionAI = MISSIONAI_EXPLORE;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	else if (AI_getUnitAIType() == UNITAI_COLONIST)
	{
		for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
		{
			CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);
			
			if ((pTransport == NULL && pLoopPlot->getArea() == getArea()) ||
				(pTransport != NULL && pLoopPlot->isCoastalLand()))
			{
				int iValue = 10000 * kPlayer.AI_estimatedColonistIncome(pLoopPlot, this);
				if (kPlayer.AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_FOUND, pTransport->getGroup(), 4) == 0)
				{
					int iPathTurns;
					if (pTransport->generatePath(pLoopPlot, 0, true, &iPathTurns))
					{
						iValue *= 1000;
						iValue /= pTransport->maxMoves() * 100 + pTransport->getPathCost();
						
						iValue /= 3 + pLoopPlot->getDistanceToOcean();
						
						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							pBestPlot = pTransport->canMoveInto(pLoopPlot) ? pLoopPlot : pTransport->getGroup()->getPathSecondLastPlot();
							if (ppMissionPlot != NULL)
							{
								*ppMissionPlot = pLoopPlot;
							}
							if (peMissionAI != NULL)
							{
								*peMissionAI = MISSIONAI_FOUND;
							}
						}
					}
				}
			}
		}
	}
	
	if (pBestPlot == NULL)
	{		
		int iLoop;
		CvCity* pLoopCity;
		for (pLoopCity = kPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kPlayer.nextCity(&iLoop))
		{
			CvPlot* pLoopPlot = pLoopCity->plot();
			int iPathTurns = 0;
			if (generatePath(pLoopPlot, 0, true, &iPathTurns))
			{
				int iValue = 10000 * pLoopCity->getPopulation();
				iValue *= 10 + pLoopCity->plot()->getDistanceToOcean();
				iValue /= 100 + pTransport->getPathCost();
				
				if (iValue > iBestValue)
				{
					iBestValue = iValue;
					pBestPlot = pTransport->canMoveInto(pLoopPlot) ? pLoopPlot : pTransport->getGroup()->getPathSecondLastPlot();
					if (ppMissionPlot != NULL)
					{
						*ppMissionPlot = pLoopPlot;
					}
					if (peMissionAI != NULL)
					{
						*peMissionAI = NO_MISSIONAI;
					}
				}				
			}
		}
	}
	
	return pBestPlot;
}
	
bool CvUnitAI::AI_moveFromTransport(CvPlot* pHintPlot)
{
    int iBestValue = 0;
    CvPlot* pBestPlot = NULL;
    
    for (int i = 0; i < NUM_DIRECTION_TYPES; ++i)
    {
        CvPlot* pLoopPlot = plotDirection(getX_INLINE(), getY_INLINE(), (DirectionTypes)i);
        if (pLoopPlot != NULL)
        {
        
            int iValue = 0;
            for (int iDirection = 0; iDirection < NUM_DIRECTION_TYPES; iDirection++)
            {
                CvPlot* pDirectionPlot = plotDirection(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), (DirectionTypes)iDirection);
                if (pDirectionPlot != NULL)
                {
                    if (pDirectionPlot->isCity())
                    {
                        if (atWar(getTeam(), pDirectionPlot->getTeam()))
                        {
                            iValue += pDirectionPlot->getPlotCity()->getPopulation() * 100;
                        }
                    }
                }
            }
            
            if (pLoopPlot->isCity())
            {
                if (pLoopPlot->getTeam() == getTeam() && !pLoopPlot->getPlotCity()->AI_isDefended())
                {
                    iValue += pLoopPlot->getPlotCity()->getPopulation() * 100;
                }
                else if (atWar(getTeam(), pLoopPlot->getTeam()))
                {
					if (pLoopPlot->getNumVisibleEnemyDefenders(this) == 0)
					{
						iValue += pLoopPlot->getPlotCity()->getPopulation() * 150;
					}
                }
            } 
            else if (iValue > 0)
            {
				iValue += pLoopPlot->defenseModifier(getTeam());
            }

			if (iValue > 0)
			{
				if (generatePath(pLoopPlot, 0, true))
				{
					if (iValue > iBestValue)
					{
						pBestPlot = pLoopPlot;
						iBestValue = iValue;
					}
				}
            }
        }
    }
    
    if (pBestPlot != NULL)
    {
        getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0);
        return true;
    }
    return false;    
}

bool CvUnitAI::AI_attackFromTransport(CvPlot* pHintPlot, int iLowOddsThreshold, int iHighOddsThreshold)
{
    if (AI_smartAttack(1, iLowOddsThreshold, iHighOddsThreshold, pHintPlot))
    {
        return true;
    }
    return false;
}
	
int CvUnitAI::AI_getMovePriority() const
{
	if (isDelayedDeath())
	{
		return 0;
	}
	return m_iMovePriority;
}

void CvUnitAI::AI_setMovePriority(int iNewValue)
{
	m_iMovePriority = iNewValue;

	if (AI_getMovePriority() <= 0)
	{
		GET_PLAYER(getOwnerINLINE()).AI_removeUnitFromMoveQueue(this);
	}
	else
	{
		GET_PLAYER(getOwnerINLINE()).AI_addUnitToMoveQueue(this);        
	}
}

bool CvUnitAI::AI_hasAIChanged(int iNumTurns)
{
	if (getGameTurnCreated() == AI_getLastAIChangeTurn())
	{
		return false;
	}
	
	if (GC.getGameINLINE().getGameTurn() - AI_getLastAIChangeTurn() < iNumTurns)
	{
		return true;
	}
	return false;
}

int CvUnitAI::AI_getLastAIChangeTurn()
{
	return m_iLastAIChangeTurn;
}

void CvUnitAI::AI_doInitialMovePriority()
{
	if (!shouldUnitMove(this))
	{
		return;
	}
	int iMovePriority = 0;
	
	if (getGroup()->getActivityType() == ACTIVITY_MISSION)
	{
		return;
	}
	
	if (getUnitTravelState() == NO_UNIT_TRAVEL_STATE)
	{
		if (GET_PLAYER(getOwnerINLINE()).AI_isKing())
		{
			if (getDomainType() == DOMAIN_SEA)
			{
				iMovePriority = 1400;
			}
			else if (getDomainType() == DOMAIN_LAND)
			{
				iMovePriority = 700;
				
				
				if (!isCargo())
				{
					iMovePriority += 50;				
				}
				if (canBombard(plot()))
				{
					iMovePriority += 100;
				}

				iMovePriority += baseCombatStr();
			}
		}
		else
		{
			if (isCargo())
			{
				if (canMove())
				{
					iMovePriority = MOVE_PRIORITY_MAX;
				}
				else if (getYield() != NO_YIELD)
				{
					if (AI_getUnitAIState() == UNITAI_STATE_PURCHASED)
					{
						iMovePriority = MOVE_PRIORITY_MAX;
					}
					else if (AI_getUnitAIState() == UNITAI_STATE_SELL_TO_NATIVES)
					{
						iMovePriority = MOVE_PRIORITY_MAX;
					}
				}
			}
			else if (canMove())
			{
				if (getDomainType() == DOMAIN_SEA)
				{
					iMovePriority = MOVE_PRIORITY_HIGH;
				}
				else
				{
					if (AI_getUnitAIType() == UNITAI_SETTLER)
					{
						iMovePriority = MOVE_PRIORITY_MAX;
					}
					else
					{
						iMovePriority = MOVE_PRIORITY_MEDIUM;
					}
				}
			}
		}
	}
	else if (getUnitTravelState() == UNIT_TRAVEL_STATE_IN_EUROPE)
	{
		if (getDomainType() == DOMAIN_SEA)
		{		
			iMovePriority = MOVE_PRIORITY_MIN;
		}
	}
	
	AI_setMovePriority(iMovePriority);
}

void CvUnitAI::AI_doFound()
{
	doFound(false);
}

ProfessionTypes CvUnitAI::AI_getOldProfession() const
{
	return m_eOldProfession;
}

void CvUnitAI::AI_setOldProfession(ProfessionTypes eProfession)
{
	m_eOldProfession = eProfession;
}

ProfessionTypes CvUnitAI::AI_getIdealProfession() const
{
	if (m_eIdealProfessionCache == INVALID_PROFESSION)
	{
		m_eIdealProfessionCache = GET_PLAYER(getOwnerINLINE()).AI_idealProfessionForUnit(getUnitType());
	}
	return m_eIdealProfessionCache;	
}
	
// XXX make sure we include any new UnitAITypes...
int CvUnitAI::AI_promotionValue(PromotionTypes ePromotion)
{
	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
	CvPromotionInfo& kPromotion = GC.getPromotionInfo(ePromotion);
	
	int iValue = 0;
	
	UnitAITypes eUnitAI = AI_getUnitAIType();
	
	if (kPromotion.getVisibilityChange() != 0)
	{
		if ((eUnitAI == UNITAI_SCOUT) || (getDomainType() == DOMAIN_SEA))
		{
			iValue += 25 * kPromotion.getVisibilityChange();
		}
	}
		
	if (kPromotion.getMovesChange() != 0)
	{
		if (eUnitAI == UNITAI_TRANSPORT_SEA)
		{
			iValue += 50 * kPromotion.getMovesChange();
		}
		else if (eUnitAI != UNITAI_DEFENSIVE)
		{
			iValue += 25 * kPromotion.getMovesChange();
		}
	}
	if (kPromotion.getMoveDiscountChange() > 0)
	{
		if (maxMoves() > 1)
		{
			iValue += (eUnitAI == UNITAI_SCOUT) ? 25 : 10;
		}
	}
	if (kPromotion.getWithdrawalChange() != 0)
	{
		if (eUnitAI == UNITAI_OFFENSIVE)
		{
			iValue += kPromotion.getWithdrawalChange() * 150;
			iValue /= (100 - withdrawalProbability());
		}
	}
	if (kPromotion.getCargoChange() != 0)
	{
		if (eUnitAI == UNITAI_TRANSPORT_SEA || eUnitAI == UNITAI_WAGON)
		{
			iValue += kPromotion.getCargoChange() * 50;
		}
	}
	if (kPromotion.getBombardRateChange() != 0)
	{
		if ((eUnitAI == UNITAI_COMBAT_SEA) || (eUnitAI == UNITAI_OFFENSIVE))
		{
			iValue += kPromotion.getBombardRateChange();
		}
	}
	if (kPromotion.getEnemyHealChange() != 0)
	{
		if (eUnitAI == UNITAI_OFFENSIVE)
		{
			iValue += kPromotion.getEnemyHealChange();
		}
	}
	if (kPromotion.getNeutralHealChange() != 0)
	{
		if (eUnitAI == UNITAI_OFFENSIVE || eUnitAI == UNITAI_COUNTER)
		{
			iValue += kPromotion.getNeutralHealChange();
		}		
	}
	if (kPromotion.getFriendlyHealChange() != 0)
	{
		iValue += kPromotion.getFriendlyHealChange();
	}
	if (kPromotion.getSameTileHealChange() != 0)
	{
		int iTotalUnits = 0;
		if (kOwner.AI_countPromotions(ePromotion, plot(), 2, &iTotalUnits) == 0)
		{
			if (iTotalUnits > 3)
			{
				iValue += 10 + iTotalUnits;
			}
		}
	}
	if (kPromotion.getAdjacentTileHealChange() != 0)
	{
		int iTotalUnits = 0;
		if (kOwner.AI_countPromotions(ePromotion, plot(), 2, &iTotalUnits) == 0)
		{
			if (iTotalUnits > 10)
			{
				iValue += 30;
			}
		}
	}
	
	if (kPromotion.getCombatPercent() != 0)
	{
		if (eUnitAI == UNITAI_COUNTER)
		{
			iValue += kPromotion.getCombatPercent() * 2;
		}
		else
		{
			iValue += kPromotion.getCombatPercent();
		}
	}
	
	if (kPromotion.getCityAttackPercent() != 0)
	{
		if (eUnitAI == UNITAI_OFFENSIVE)
		{
			iValue += kPromotion.getCityAttackPercent() * (10 + cityAttackModifier()) / 10;
		}
	}
	
	if (kPromotion.getCityDefensePercent() != 0)
	{
		if (eUnitAI == UNITAI_DEFENSIVE)
		{
			iValue += kPromotion.getCityDefensePercent() * (10 + cityDefenseModifier()) / 10;
		}		
	}
	if (kPromotion.getHillsAttackPercent() != 0)
	{
		if ((eUnitAI == UNITAI_COUNTER) && (kOwner.AI_countPromotions(ePromotion, plot(), 1) == 0))
		{
			iValue += kPromotion.getHillsAttackPercent() / 2;			
		}
	}
	if (kPromotion.getHillsDefensePercent() != 0)
	{
		if ((eUnitAI == UNITAI_COUNTER) && kOwner.AI_countPromotions(ePromotion, plot(), 1) == 0)
		{
			iValue += kPromotion.getHillsDefensePercent() / 2;			
		}
	}
	if (kPromotion.getPillageChange() != 0)
	{
		if (eUnitAI == UNITAI_OFFENSIVE)
		{
			iValue += kPromotion.getPillageChange() / 4;
		}
	}
	
	if (kPromotion.getExperiencePercent() != 0)
	{
		if (eUnitAI == UNITAI_OFFENSIVE || eUnitAI == UNITAI_DEFENSIVE)
		{
			iValue += kPromotion.getExperiencePercent() / 4;
		}
	}

	if (kPromotion.isBlitz())
	{
		if ((eUnitAI != UNITAI_DEFENSIVE) && canAttack())
		{
			iValue += ((getMoves()) - 1) * 20;
		}
	}
	
	if (kPromotion.isAmphib())
	{
		if (eUnitAI == UNITAI_OFFENSIVE)
		{
			iValue += 4;
		}
	}
	
	if (kPromotion.isRiver())
	{
		if (eUnitAI == UNITAI_OFFENSIVE)
		{
			iValue += 4;
		}
	}
	
	if (kPromotion.isEnemyRoute())
	{
		iValue += 10;
	}
	
	if (kPromotion.isAlwaysHeal())
	{
		if (eUnitAI != UNITAI_DEFENSIVE)
		{
			iValue += 25;
		}
	}
	
	if (isHillsDoubleMove())
	{
		if (eUnitAI == UNITAI_COUNTER)
		{
			iValue += 20;
		}
	}

	if (eUnitAI == UNITAI_COUNTER)
	{
		if (kOwner.AI_countPromotions(ePromotion, plot(), 1) == 0)
		{
			for (int i = 0; i < GC.getNumTerrainInfos(); ++i)
			{
				int iPercent = kPromotion.getTerrainAttackPercent(i) + kPromotion.getTerrainDefensePercent(i);
				if (kPromotion.getTerrainDoubleMove(i))
				{
					iPercent += 10;
				}
				if (iPercent > 0)
				{
					iPercent *= 20 + terrainAttackModifier((TerrainTypes)i) + terrainDefenseModifier((TerrainTypes)i);
					iPercent /= 20;
					
					int iCount = 0;
					for (int j = 0; j < NUM_DIRECTION_TYPES; ++j)
					{
						CvPlot* pLoopPlot = plotDirection(getX_INLINE(), getY_INLINE(), (DirectionTypes)j);
						if (pLoopPlot != NULL)
						{
							if (pLoopPlot->getTerrainType() == i)
							{
								++iCount;
							}
						}
					}
					if (iCount > 0)
					{
						iValue += iPercent / 3 + iCount * iPercent / NUM_DIRECTION_TYPES;
					}
				}
			}

			for (int i = 0; i < GC.getNumFeatureInfos(); ++i)
			{
				int iPercent = kPromotion.getFeatureAttackPercent(i) + kPromotion.getFeatureDefensePercent(i);
				if (kPromotion.getFeatureDoubleMove(i))
				{
					iPercent += 10;
				}
				if (iPercent > 0)
				{
					iPercent *= 20 + featureAttackModifier((FeatureTypes)i) + featureDefenseModifier((FeatureTypes)i);
					iPercent /= 20;
					
					int iCount = 0;
					for (int j = 0; j < NUM_DIRECTION_TYPES; ++j)
					{
						CvPlot* pLoopPlot = plotDirection(getX_INLINE(), getY_INLINE(), (DirectionTypes)j);
						if (pLoopPlot != NULL)
						{
							if (pLoopPlot->getFeatureType() == i)
							{
								++iCount;
							}
						}
					}
					if (iCount > 0)
					{
						iValue += iPercent / 3 + iCount * iPercent / NUM_DIRECTION_TYPES;
					}
				}
			}
		}
	}
	
	for (int i = 0; i < GC.getNumUnitClassInfos(); ++i)
	{
		if (kPromotion.getUnitClassAttackModifier(i) > 0)
		{
			if (eUnitAI == UNITAI_COUNTER)
			{
				iValue += kPromotion.getUnitClassAttackModifier(i) / 3;
			}
		}
		if (kPromotion.getUnitClassDefenseModifier(i) > 0)
		{
			if (eUnitAI == UNITAI_DEFENSIVE)
			{
				iValue += kPromotion.getUnitClassDefenseModifier(i) / 3;
			}
		}
	}
	if (eUnitAI == UNITAI_COUNTER || eUnitAI == UNITAI_DEFENSIVE)
	{
		for (int i = 0; i < GC.getNumUnitCombatInfos(); ++i)
		{
			iValue += kPromotion.getUnitCombatModifierPercent(i) / 2;
		}
	}
	for (int i = 0; i < NUM_DOMAIN_TYPES; ++i)
	{
		if (kPromotion.getDomainModifierPercent(i) > 0)
		{
			if (getDomainType() == i)
			{
				iValue += kPromotion.getDomainModifierPercent(i);
			}
		}
	}

	if (iValue > 0)
	{
		iValue += GC.getGameINLINE().getSorenRandNum(15, "AI Promote");
	}
	return iValue;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_shadow(UnitAITypes eUnitAI, int iMax, int iMaxRatio, bool bWithCargoOnly)
{
	PROFILE_FUNC();

	CvUnit* pLoopUnit;
	CvUnit* pBestUnit;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iLoop;

	iBestValue = 0;
	pBestUnit = NULL;

	for(pLoopUnit = GET_PLAYER(getOwnerINLINE()).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER(getOwnerINLINE()).nextUnit(&iLoop))
	{
		if (pLoopUnit != this)
		{
			if (AI_plotValid(pLoopUnit->plot()))
			{
				if (pLoopUnit->isGroupHead())
				{
					if (!(pLoopUnit->isCargo()))
					{
						if (pLoopUnit->AI_getUnitAIType() == eUnitAI)
						{
							if (pLoopUnit->getGroup()->baseMoves() <= getGroup()->baseMoves())
							{
								if (!bWithCargoOnly || pLoopUnit->getGroup()->hasCargo())
								{
									int iShadowerCount = GET_PLAYER(getOwnerINLINE()).AI_unitTargetMissionAIs(pLoopUnit, MISSIONAI_SHADOW, getGroup());
									if (((-1 == iMax) || (iShadowerCount < iMax)) &&
										 ((-1 == iMaxRatio) || (iShadowerCount == 0) || (((100 * iShadowerCount) / std::max(1, pLoopUnit->getGroup()->countNumUnitAIType(eUnitAI))) <= iMaxRatio)))
									{
										if (!(pLoopUnit->plot()->isVisibleEnemyUnit(this)))
										{
											if (generatePath(pLoopUnit->plot(), 0, true, &iPathTurns))
											{
												//if (iPathTurns <= iMaxPath) XXX
												{
													iValue = 1 + pLoopUnit->getGroup()->getCargo();
													iValue *= 1000;
													iValue /= 1 + iPathTurns;

													if (iValue > iBestValue)
													{
														iBestValue = iValue;
														pBestUnit = pLoopUnit;
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

	if (pBestUnit != NULL)
	{
		if (atPlot(pBestUnit->plot()))
		{
			getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_SHADOW, NULL, pBestUnit);
			return true;
		}
		else
		{
			getGroup()->pushMission(MISSION_MOVE_TO_UNIT, pBestUnit->getOwnerINLINE(), pBestUnit->getID(), 0, false, false, MISSIONAI_SHADOW, NULL, pBestUnit);
			return true;
		}
	}

	return false;
}


bool CvUnitAI::AI_unloadWhereNeeded(int iMaxPath)
{
	CvPlayer& kOwner = GET_PLAYER(getOwnerINLINE());
	
	CvUnit* pTransportUnit = getTransportUnit();
	
	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	CvPlot* pBestJoinPlot = NULL;
	
	int iLoop;
	CvCity* pCity;

	for (pCity = kOwner.firstCity(&iLoop); pCity != NULL; pCity = kOwner.nextCity(&iLoop))
	{
		CvPlot* pLoopPlot = pCity->plot();

		{
			int iPathTurns = 0;
			
			if (atPlot(pLoopPlot) || pTransportUnit->generatePath(pLoopPlot, MOVE_SAFE_TERRITORY, true, &iPathTurns))
			{
				if (iPathTurns < iMaxPath)
				{
					int iValue = 1000;
					
					CvArea* pArea = pLoopPlot->area();
					
					iValue *= 1 + pArea->getNumCities();
					iValue /= 1 + pArea->getNumAIUnits(getOwnerINLINE(), AI_getUnitAIType());				
					
					if (iValue > iBestValue)
					{
						iBestValue = iValue;
						pBestPlot = atPlot(pLoopPlot) ? pLoopPlot : pTransportUnit->getPathEndTurnPlot();
						pBestJoinPlot = pLoopPlot;
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestJoinPlot != NULL))
	{
		if (atPlot(pBestJoinPlot))
		{
			unload();
			return true;
		}

		getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_FOUND, pBestJoinPlot);
		AI_setMovePriority(1);
		return true;

	}
	return false;
}

bool CvUnitAI::AI_betterJob()
{
	
	CvCity* pCity = plot()->getPlotCity();
	if (pCity == NULL)
	{
		return false;
	}
	if (pCity->getOwnerINLINE() != getOwnerINLINE())
	{
		return false;
	}

	
	
	pCity->AI_setWorkforceHack(true);
	
	
	ProfessionTypes eOriginalProfession = getProfession();
	std::vector<CvUnit*> units;
	int iOriginalMovePriority = AI_getMovePriority();
	UnitAITypes eOriginalAI = AI_getUnitAIType();
	
	bool bJoined=false;
	if (canJoinCity(plot()))
	{
		bJoined=true;
		pCity->addPopulationUnit(this, NO_PROFESSION);
	}

	CvPlot * pPlot = plot();
	CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();
	while (pUnitNode != NULL)
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pPlot->nextUnitNode(pUnitNode);
		
		if (pLoopUnit != this && pLoopUnit->getOwnerINLINE() == getOwnerINLINE())
		{
			if (!pLoopUnit->AI_hasAIChanged(5) && pLoopUnit->canJoinCity(pPlot))
			{
				if (pLoopUnit->canHaveProfession(eOriginalProfession, true) && canHaveProfession(pLoopUnit->getProfession(), true))
				{
					units.push_back(pLoopUnit);
				}
			}
		}
	}
	
	for (int i = 0; i < pCity->getPopulation(); ++i)
	{
		CvUnit* pLoopUnit = pCity->getPopulationUnitByIndex(i);
		if (pLoopUnit != this && pLoopUnit->getProfession() != NO_PROFESSION)
		{
			if (pLoopUnit != this)
			{
				if (!pLoopUnit->AI_hasAIChanged(5))
				{
					if (pLoopUnit->canHaveProfession(eOriginalProfession, true) && canHaveProfession(pLoopUnit->getProfession(), true))
					{
						units.push_back(pLoopUnit);
					}
				}
			}
		}
	}
	
	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
	int iSuitability = kOwner.AI_professionSuitability(this, eOriginalProfession, plot());
	
	int iBestValue = 0;
	CvUnit* pBestUnit = NULL;
	
	for (uint i = 0; i < units.size(); ++i)
	{
		CvUnit* pLoopUnit = units[i];
		CvPlot* pLoopPlot = GC.getProfessionInfo(pLoopUnit->getProfession()).isWorkPlot() ? pCity->getPlotWorkedByUnit(pLoopUnit) : plot();
		int iOtherSuitability = kOwner.AI_professionSuitability(pLoopUnit, eOriginalProfession, pLoopPlot);
		int iValue = 0;

		if ((iSuitability < 100) && (iOtherSuitability >= 100))
		{
			iValue = iOtherSuitability - iSuitability;
		}

		int iOriginalValue = iSuitability + kOwner.AI_professionSuitability(pLoopUnit, pLoopUnit->getProfession(), pLoopPlot);
		
		int iNewValue = kOwner.AI_professionSuitability(this, pLoopUnit->getProfession(), pLoopPlot) + kOwner.AI_professionSuitability(pLoopUnit, eOriginalProfession, pLoopPlot);
		
		iValue = std::max(iValue, iNewValue - iOriginalValue);
		
		if (iValue > iBestValue)
		{
			iBestValue = iValue;
			pBestUnit = pLoopUnit;
		}
	}
	
	if (bJoined)
	{
		pCity->removePopulationUnit(this, false, eOriginalProfession);
		AI_setUnitAIType(eOriginalAI);
		AI_setMovePriority(iOriginalMovePriority);
	}
	
	if (pBestUnit != NULL)
	{
		kOwner.AI_swapUnitJobs(this, pBestUnit);
		m_iLastAIChangeTurn = GC.getGameINLINE().getGameTurn();
	}
	
	pCity->AI_setWorkforceHack(false);
	
	if (pBestUnit != NULL && hasMoved() && getGroup() != NULL)
	{
		getGroup()->pushMission(MISSION_SKIP);
		return true;
	}
	
	return (pBestUnit == NULL) ? false : true;	
}
	
bool CvUnitAI::AI_upgradeProfession()
{
	if (getProfession() == NO_PROFESSION)
	{
		return false;
	}
	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
	ProfessionTypes eBestProfession = NO_PROFESSION;
	CvProfessionInfo& kCurrentProfession = GC.getProfessionInfo(getProfession());
	
	int iBestValue = 0;
	
	for (int i = 0; i < GC.getNumProfessionInfos(); ++i)
	{
		ProfessionTypes eLoopProfession = (ProfessionTypes)i;
		if (eLoopProfession != getProfession())
		{
			if (kCurrentProfession.isCitizen() == GC.getProfessionInfo(eLoopProfession).isCitizen())
			{
				if (canHaveProfession(eLoopProfession, false, plot()))
				{
					int iValue = kOwner.AI_professionValue(eLoopProfession, AI_getUnitAIType());
					if (iValue > iBestValue)
					{
						eBestProfession = eLoopProfession;
						iBestValue = iValue;
					}
				}
			}
		}
	}
	
	if (eBestProfession == NO_PROFESSION)
	{
		return false;
	}
	
	if (iBestValue <= kOwner.AI_professionValue(getProfession(), AI_getUnitAIType()))
	{
		return false;
	}
	
	UnitAITypes eCurrentAI = AI_getUnitAIType();
	setProfession(eBestProfession);
	AI_setUnitAIType(eCurrentAI);
	return true;
}

bool CvUnitAI::AI_changeUnitAIType(int iMinMultiplier)
{
	CvCity* pCity = plot()->getPlotCity();
	if (pCity == NULL || pCity->getOwnerINLINE() != getOwnerINLINE())
	{
		return false;
	}
	
	int iBestValue = 0;
	ProfessionTypes eBestProfession = NO_PROFESSION;
	UnitAITypes eBestUnitAI = NO_UNITAI;
	
	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
	
	for (int iUnitAI = 0; iUnitAI < NUM_UNITAI_TYPES; ++iUnitAI)
	{
		UnitAITypes eLoopUnitAI = (UnitAITypes) iUnitAI;

		if (eLoopUnitAI != AI_getUnitAIType())
		{
		
			int iMultiplier = kOwner.AI_unitAIValueMultipler(eLoopUnitAI);
			
			if (iMultiplier > iMinMultiplier)
			{
				for (int iProfession = 0; iProfession < GC.getNumProfessionInfos(); ++iProfession)
				{
					ProfessionTypes eLoopProfession = (ProfessionTypes)iProfession;
					
					if (canHaveProfession(eLoopProfession, false, plot()))
					{
						int iProfessionValue = kOwner.AI_professionValue(eLoopProfession, eLoopUnitAI);
						if (iProfessionValue > 0)
						{
							iProfessionValue = kOwner.AI_professionSuitability(this, eLoopProfession, plot(), eLoopUnitAI);
							iProfessionValue /= 100;
							if (iProfessionValue > iMinMultiplier)
							{
								int iValue = iProfessionValue * iMultiplier;
								
								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									eBestProfession = eLoopProfession;
									eBestUnitAI = eLoopUnitAI;	
								}
							}
						}
					}
				}
			}
		}
	}
	
	if (eBestUnitAI != NO_UNITAI)
	{
		setProfession(eBestProfession);
		AI_setUnitAIType(eBestUnitAI);
		return true;
	}
	return false;
}
	

bool CvUnitAI::AI_advance(bool bAttack)
{
	
	int iBestValue = MAX_INT;
	CvPlot* pBestPlot = NULL;
	CvTeamAI& kTeam = GET_TEAM(getTeam());
	for (int i = 0; i < NUM_DIRECTION_TYPES; ++i)
	{
		CvPlot* pDirectionPlot = plotDirection(getX_INLINE(), getY_INLINE(), (DirectionTypes)i);
		if (pDirectionPlot != NULL)
		{
			if (bAttack ? canMoveOrAttackInto(pDirectionPlot, false) : canMoveInto(pDirectionPlot, false))
			{
				int iValue = kTeam.AI_enemyCityDistance(pDirectionPlot);
				if (iValue >= 0)
				{
					iValue *= 100;
					iValue -= GC.getGameINLINE().getSorenRandNum(50, "AI best advance plot");
					iValue -= std::min(50, pDirectionPlot->defenseModifier(getTeam()));
					
					if (iValue < iBestValue)
					{
						iBestValue = iValue;
						pBestPlot = pDirectionPlot;
					}
				}
			}
		}
	}
	
	if (pBestPlot != NULL)
	{
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false);
		return true;
	}
	return false;
}
		
bool CvUnitAI::AI_loiter(int iMinDistance, int iMaxDistance, bool bAttack)
{
	int iBestValue = MAX_INT;
	CvPlot* pBestPlot = NULL;
	CvTeamAI& kTeam = GET_TEAM(getTeam());
	for (int iX = -1; iX <= 1; ++iX)
	{
		for (int iY = -1; iY <= 1; ++iY)
		{
			CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iX, iY);
			if (pLoopPlot != NULL)
			{
				if (atPlot(pLoopPlot) || (bAttack ? canMoveOrAttackInto(pLoopPlot, false) : canMoveInto(pLoopPlot, false)))
				{
					int iEnemyDistance = kTeam.AI_enemyCityDistance(pLoopPlot);
					if (iEnemyDistance >= 0)
					{
						int iValue = 0;
						if (iEnemyDistance < iMinDistance)
						{
							iValue = iMinDistance - iEnemyDistance;
						}
						else if (iEnemyDistance > iMaxDistance)
						{
							iValue = iEnemyDistance - iMaxDistance;
						}
						iValue *= 100;
						iValue += 100;
						iValue -= GC.getGameINLINE().getSorenRandNum(50, "AI best loiter plot");
						iValue -= std::min(50, pLoopPlot->defenseModifier(getTeam()));
						
						if (iValue < iBestValue)
						{
							iBestValue = iValue;
							pBestPlot = pLoopPlot;
						}
					}
				}
			}
		}
	}
	
	if (pBestPlot != NULL)
	{
		if (atPlot(pBestPlot))
		{
			getGroup()->pushMission(MISSION_SKIP);
		}
		else
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false);
		}
		return true;
	}
	return false;
}

bool CvUnitAI::AI_retreat(int iMaxDistance)
{
	int iRange = iMaxDistance;
	
	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	
	for (int iX = -iRange; iX <= iRange; iX++)
	{
		for (int iY = -iRange; iY <= iRange; iY++)
		{
			CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iX, iY);
			if ((pLoopPlot != NULL) && !atPlot(pLoopPlot))
			{
				if (pLoopPlot->isCity() && pLoopPlot->getOwnerINLINE() == getOwnerINLINE())
				{
					if (AI_plotValid(pLoopPlot))
					{
						int iPathTurns = 0;
						if (generatePath(pLoopPlot, 0, true, &iPathTurns))
						{
							int iValue = pLoopPlot->getPlotCity()->getHighestPopulation() * 100;
							
							iValue /= iPathTurns + 1;
							
							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								pBestPlot = pLoopPlot;
							}
						}
					}
				}
			}
		}
	}
	
	if (pBestPlot != NULL)
	{
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false);
		return true;
	}
	return false;
}

bool CvUnitAI::AI_counter(int iTether)
{
	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	CvTeamAI& kTeam = GET_TEAM(getTeam());
	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());

	for (int i = 0; i < NUM_DIRECTION_TYPES; ++i)
	{
		CvPlot* pDirectionPlot = plotDirection(getX_INLINE(), getY_INLINE(), (DirectionTypes)i);
		if (pDirectionPlot != NULL)
		{
			if (canMoveInto(pDirectionPlot, false))
			{
				int iFriendlyCityDistance = kOwner.AI_cityDistance(pDirectionPlot);
				
				if (iFriendlyCityDistance <= iTether)
				{
					int iValue = 1 + GC.getGameINLINE().getSorenRandNum(100, "AI counter patrol");
					
					int iEnemyUnitDistance = kTeam.AI_enemyUnitDistance(pDirectionPlot);
					int iEnemyCityDistance = kTeam.AI_enemyCityDistance(pDirectionPlot);
					
					int iDefense = pDirectionPlot->defenseModifier(getTeam());
					
					if (iEnemyUnitDistance == 1)
					{
						if (iDefense == 0)
						{
							iValue /= 10;							
						}
						iValue *= 100 + 4 * iDefense;
						
						iValue /= 2 + iFriendlyCityDistance;
					}
					else
					{
						iValue *= 100;
						iValue /= 1 + std::max(0, iEnemyUnitDistance);
						iValue /= 3 + iEnemyCityDistance;
					}
					
					if (iValue > iBestValue)
					{
						iBestValue = iValue;
						pBestPlot = pDirectionPlot;
					}
				}
			}
		}
	}
	
	if (pBestPlot != NULL)
	{
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false);
		return true;
	}
	return false;
}

bool CvUnitAI::AI_shouldRun()
{
	int iRange = 3;
	
	for (int iX = -iRange; iX <= iRange; ++iX)
	{
		for (int iY = -iRange; iY <= iRange; ++iY)
		{
			CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iX, iY);
			if (pLoopPlot != NULL && pLoopPlot->isVisible(getTeam(), false))
			{
				for (int i = 0; i < pLoopPlot->getNumUnits(); ++i)
				{
					CvUnit* pLoopUnit = pLoopPlot->getUnitByIndex(i);
					if (isEnemy(pLoopUnit->getTeam()))
					{
						if (pLoopUnit->canAttack() && pLoopUnit->getDomainType() == getDomainType())
						{
							
							int iTheirStrength = pLoopUnit->currCombatStr(NULL, NULL);
							int iTheirFirepower = pLoopUnit->currFirepower(NULL, NULL);

							if (iTheirStrength == 0)
							{
								return 1;
							}

							int iOurStrength = currCombatStr(pLoopPlot, pLoopUnit);
							int iOurFirepower = currFirepower(pLoopPlot, pLoopUnit);

							if (iTheirStrength * iTheirFirepower > iOurStrength * iOurFirepower)
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

//Fast routine for revealing water
bool CvUnitAI::AI_seaPatrol()
{
	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	
	for (int iDirection = 0; iDirection < NUM_DIRECTION_TYPES; iDirection++)
	{
		CvPlot* pLoopPlot = plotDirection(getX_INLINE(), getY_INLINE(), (DirectionTypes)iDirection);
		int iPathTurns;
		if (generatePath(pLoopPlot, 0, true, &iPathTurns))
		{
			if (iPathTurns <= 1)
			{
				int iValue = 150 / (1 + abs(pLoopPlot->getDistanceToOcean() - 3));
				iValue += GC.getGameINLINE().getSorenRandNum(50, "AI sea patrol");
				
				if (!isCardinalDirection((DirectionTypes)iDirection))
				{
					iValue *= 4;
					iValue /= 3;
				}
				
				if (pLoopPlot->isAdjacentToLand())
				{
					iValue /= 2;
				}
				
				if (iValue > iBestValue)
				{
					iBestValue = iValue;
					pBestPlot = pLoopPlot;
				}
			}
		}
	}
	
	if (pBestPlot != NULL)
	{
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_PIRACY, pBestPlot);
	}
	return false;
}

bool CvUnitAI::AI_moveTowardsOcean(int iRange)
{
	int iBestValue = MAX_INT;
	CvPlot* pBestPlot = NULL;
	for (int iX = -iRange; iX <= iRange; iX++)
	{
		for (int iY = -iRange; iY <= iRange; iY++)
		{
			if (iX != 0 || iY != 0)
			{
				CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iX, iY);
				if (pLoopPlot != NULL)
				{
					if (pLoopPlot->getDistanceToOcean() < plot()->getDistanceToOcean())
					{
						int iPathTurns;
						if (generatePath(pLoopPlot, MOVE_BUST_FOG, true, &iPathTurns) && iPathTurns <= 1)
						{
							int iPlotValue = 100 * pLoopPlot->getDistanceToOcean();
							iPlotValue += GC.getGameINLINE().getSorenRandNum(50, "AI move to ocean");
							
							if (iPlotValue < iBestValue)
							{
								iBestValue = iPlotValue;
								pBestPlot = pLoopPlot;
							}
						}
					}
				}
			}
		}
	}
	
	if (pBestPlot != NULL)
	{
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), MOVE_BUST_FOG);
		return true;
	}
	return false;
}

bool CvUnitAI::AI_moveTowardsVictimCity()
{
	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	CvPlot* pBestMissionPlot = NULL;
	
	for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);
		
		CvCity* pCity = pLoopPlot->getPlotCity();
		if (pCity != NULL)
		{
			if (pCity->getTeam() == getTeam() || isEnemy(pCity->getTeam()))
			{
				if (!pCity->isNative())
				{
					if (pCity->isCoastal(GC.getMIN_WATER_SIZE_FOR_OCEAN()))
					{
						int iCityValue = 1 + GC.getGameINLINE().getSorenRandNum(pCity->getPopulation() * 1000, "AI Piracy City Target");
						for (int iDirection = 0; iDirection < NUM_DIRECTION_TYPES; iDirection++)
						{
							CvPlot* pDirectionPlot = plotDirection(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), (DirectionTypes)iDirection);
							if ((pDirectionPlot != NULL) && pDirectionPlot->isWater())
							{
								int iPathTurns;
								if (generatePath(pDirectionPlot, MOVE_THROUGH_ENEMY, true, &iPathTurns))
								{
									int iValue = iCityValue / (pDirectionPlot->getDistanceToOcean() + 1);
									iValue += pLoopPlot->getCrumbs();
									iValue /= 2 + std::max(3, iPathTurns);
									if (iPathTurns < 3)
									{
										iValue /= 5 - iPathTurns;
									}
									CvPlayer& kCityOwner = GET_PLAYER(pCity->getOwnerINLINE());
									if (kCityOwner.countNumCoastalCitiesByArea(area()) == 1)
									{
										iValue *= 3;
										iValue /= 2;
									}
									
									if (iValue > iBestValue)
									{
										iBestValue = iValue;
										pBestPlot = getGroup()->getPathFirstPlot();
										pBestMissionPlot = pDirectionPlot;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	
	if (pBestPlot != NULL)
	{
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_PIRACY, pBestMissionPlot);
		return true;
	}

	return false;
}



bool CvUnitAI::AI_spreadReligion()
{
	PROFILE_FUNC();
	
	FAssert(GC.getProfessionInfo(getProfession()).getMissionaryRate() > 0);
	
	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	CvPlot* pBestSpreadPlot = NULL;
	
	//We can optimize some by skipping some code if the civilization is unique to this team
	int iTeamCivCount = 0;
	int iOtherTeamCivCount = 0;
	for (int iPlayer = 0; iPlayer < MAX_PLAYERS; iPlayer++)
	{
		CvPlayerAI& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);
		if (kLoopPlayer.isAlive() && !kLoopPlayer.isEurope() && !kLoopPlayer.isNative())
		{
			if (kLoopPlayer.getCivilizationType() == getCivilizationType())
			{
				if (kLoopPlayer.getTeam() == getTeam())
				{
					iTeamCivCount++;
				}
				else
				{
					iOtherTeamCivCount++;
				}
			}
		}
	}
	
	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
	for (int iPlayer = 0; iPlayer < MAX_PLAYERS; iPlayer++)
	{
		CvPlayerAI& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);
		if (kLoopPlayer.isAlive())
		{
			if (kLoopPlayer.canHaveMission(getCivilizationType()))
			{
				if (kLoopPlayer.AI_getAttitude(getOwnerINLINE()) >= ATTITUDE_CAUTIOUS)
				{
					int iLoop;
					for (CvCity* pLoopCity = kLoopPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kLoopPlayer.nextCity(&iLoop))
					{
						CvPlot* pLoopPlot = pLoopCity->plot();
						if (pLoopPlot->isRevealed(getTeam(), false))
						{
							if (pLoopPlot->getArea() == getArea())
							{
								if ((pLoopCity->getMissionaryCivilization() != getCivilizationType()) && (pLoopPlot->calculateCulturePercent(pLoopPlot->getOwnerINLINE()) == 100))
								{
									if (kOwner.AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_SPREAD, getGroup()) == 0)
									{
										bool bValid = (pLoopCity->getMissionaryCivilization() == NO_CIVILIZATION) || (iOtherTeamCivCount == 0);
										int iModifier = 100;
										
										
										if (!bValid)
										{
											int iProb;
											pLoopCity->calculateConvertRecipient(getTeam(), &iProb);
											if (iProb > 50)
											{
												iModifier = 2 * (iProb - 50);
												bValid = true;
											}
										}

										if (bValid)
										{
											int iPathTurns;
											if (generatePath(pLoopPlot, MOVE_NO_ENEMY_TERRITORY, true, &iPathTurns))
											{
												if (iPathTurns < 10)
												{
													int iValue = pLoopCity->getPopulation() * iModifier;
													
													if (pLoopPlot->getCulture(getOwnerINLINE()) > 0)
													{
														iValue /= 4;
													}
													
													iValue *= 100 + GC.getGameINLINE().getSorenRandNum(25, "AI best spread plot");
													iValue /= 400 + getPathCost();
													if (iValue > iBestValue)
													{
														iBestValue = iValue;
														pBestSpreadPlot = pLoopPlot;
														pBestPlot = getPathEndTurnPlot();
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
		

	if ((pBestPlot != NULL) && (pBestSpreadPlot != NULL))
	{
		if (atPlot(pBestSpreadPlot))
		{
			FAssert(canEstablishMission());
			establishMission();
			kOwner.AI_changeNumRetiredAIUnits(UNITAI_MISSIONARY, 1);
			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_SPREAD, pBestSpreadPlot);
			return true;
		}
	}

	return false;
}

bool CvUnitAI::AI_learn(int iRange)
{
	FAssert(getUnitInfo().getLearnTime() >= 0);
	
	CvCity* pCity = plot()->getPlotCity();
	if (pCity != NULL)
	{
		if (canLearn())
		{
			learn();
			return true;
		}
	}
	
	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
	
	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	
	for (int iX = -iRange; iX <= iRange; iX++)
	{
		for (int iY = -iRange; iY <= iRange; iY++)
		{
			CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iX, iY);
			if (pLoopPlot != NULL)
			{
				CvCity* pPlotCity = pLoopPlot->getPlotCity();
				if (pPlotCity != NULL)
				{
					if (pPlotCity->getArea() == getArea())
					{
						if (GET_PLAYER(pPlotCity->getOwnerINLINE()).isNative() && pPlotCity->isScoutVisited(getTeam()))
						{
							if (!isEnemy(pPlotCity->getTeam()))
							{
								if (!pLoopPlot->isVisibleEnemyUnit(this))
								{
									UnitTypes eLearnUnitType = getLearnUnitType(pLoopPlot);
									if (eLearnUnitType != NO_UNIT)
									{
										int iProfessionSuitability = 100;
										if (AI_getOldProfession() != NO_PROFESSION)
										{
											iProfessionSuitability = kOwner.AI_professionSuitability(eLearnUnitType, AI_getOldProfession());											
										}
										if (iProfessionSuitability > 100 || kOwner.getUnitClassCountPlusMaking(pPlotCity->getTeachUnitClass()) == 0)
										{
											if (iProfessionSuitability > 100 || kOwner.AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_LEARN, getGroup()) == 0)
											{
												int iPathTurns;
												if (generatePath(pLoopPlot, MOVE_NO_ENEMY_TERRITORY, true, &iPathTurns))
												{
													int iValue = iProfessionSuitability;
													if (iProfessionSuitability > 100)
													{
														iValue *= 2;
													}

													iValue /= 5 + iPathTurns;
													
													if (iValue > iBestValue)
													{
														iBestValue = iValue;
														pBestPlot = pLoopPlot;
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
	
	if (pBestPlot != NULL)
	{
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), MOVE_NO_ENEMY_TERRITORY, false, false, MISSIONAI_LEARN, pBestPlot);
		return true;
	}
	return false;
}

bool CvUnitAI::AI_requestPickup(int iMaxPath)
{
	PROFILE_FUNC();

	//Are we ready to load?
	CvPlayerAI& kOwner= GET_PLAYER(getOwnerINLINE());
	
	if (getGroup()->AI_getMissionAIType() == MISSIONAI_AWAIT_PICKUP)
	{
		CvPlot* pMissionPlot =  getGroup()->AI_getMissionAIPlot();
		FAssert(pMissionPlot != NULL);
		
		if (kOwner.AI_plotTargetMissionAIs(pMissionPlot, MISSIONAI_PICKUP, NULL, 1) > 0)
		{
			if (AI_continueMission(0, MISSIONAI_AWAIT_PICKUP, MOVE_NO_ENEMY_TERRITORY, false))
			{
				return true;
			}
		}
	}
	
	CvPlot* pStartingPlot = GET_PLAYER(getOwnerINLINE()).getStartingPlot();
	if (pStartingPlot == NULL || !pStartingPlot->isWater())
	{
		return false;
	}
	EuropeTypes eMainEurope = pStartingPlot->getNearestEurope();
	if (eMainEurope == NO_EUROPE)
	{
		return false;
	}
	
	
	CvPlot* pBestPlot = NULL;
	CvPlot* pBestMissionPlot = NULL;
	int iBestValue = MAX_INT;
	
	MissionAITypes paMissionAIs[] = {MISSIONAI_PICKUP, MISSIONAI_AWAIT_PICKUP};
	int iMissionAICount = 2;
	
	for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);
		
		if (pLoopPlot->getArea() == getArea())
		{
			CvArea* pWaterArea = pLoopPlot->waterArea();
			if (pWaterArea != NULL && pWaterArea->getNumAIUnits(getOwnerINLINE(), UNITAI_TRANSPORT_SEA) > 0 && pLoopPlot->getNearestEurope() == eMainEurope)
			{
				int iOceanDist = pLoopPlot->getDistanceToOcean();
				
				if (iOceanDist < 20)
			{
				if (atPlot(pLoopPlot) || generatePath(pLoopPlot, 0, true))
				{
					int iValue = 300 + atPlot(pLoopPlot) ? 0 : getPathCost();
					int iClosestTargetRange;
					int iCount = kOwner.AI_plotTargetMissionAIs(pLoopPlot, paMissionAIs, iMissionAICount, iClosestTargetRange, getGroup(), 1);
					iValue /= 1 + iCount;
					
						if (pLoopPlot->isCity())
					{
							iValue /= 2;
							}
						
						iValue *= iOceanDist;
					if (iValue < iBestValue)
					{
						iBestValue = iValue;
						pBestPlot = atPlot(pLoopPlot) ? pLoopPlot : getPathEndTurnPlot();
						pBestMissionPlot = pLoopPlot;								
					}
				}
			}
		}
	}
	}
	
	if (pBestPlot != NULL)
	{
		if (atPlot(pBestPlot))
		{
			getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_AWAIT_PICKUP, pBestMissionPlot);
		}
		else
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_AWAIT_PICKUP, pBestMissionPlot);
		}
		return true;
	}
	return false;
}

//This is very fast when no units need picking up.
bool CvUnitAI::AI_respondToPickup(int iMaxPath, UnitAITypes eUnitAI)
{
	PROFILE_FUNC();
	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	CvPlot* pBestMissionPlot = NULL;
	
	CvArea* pWaterArea = plot()->waterArea();
	int iLoop;
	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
	for(CvSelectionGroup* pLoopSelectionGroup = kOwner.firstSelectionGroup(&iLoop); pLoopSelectionGroup; pLoopSelectionGroup = kOwner.nextSelectionGroup(&iLoop))
	{
		if (pLoopSelectionGroup->getNumUnits() > 0)
		{
			if (pLoopSelectionGroup->AI_getMissionAIType() == MISSIONAI_AWAIT_PICKUP)
			{
				if ((eUnitAI == NO_UNITAI) || (pLoopSelectionGroup->getHeadUnit()->AI_getUnitAIType() == eUnitAI))
				{
					int iMaxPathTurns = iMaxPath;
					CvUnit* pHeadUnit = pLoopSelectionGroup->getHeadUnit();
					FAssert(pHeadUnit != NULL);

					CvPlot* pMissionPlot = pLoopSelectionGroup->AI_getMissionAIPlot();
					if (pMissionPlot != NULL)
					{
						if ((stepDistance(pMissionPlot->getX_INLINE(), pMissionPlot->getY_INLINE(), getX_INLINE(), getY_INLINE()) / std::max(1, maxMoves())) <= iMaxPathTurns)
						{

							bool bValid = true;
							
							{
								CvPlot * pPlot = plot();
								CLLNode<IDInfo>* pUnitNode = pLoopSelectionGroup->headUnitNode();

								int iCount = 0;
								while (pUnitNode != NULL)
								{
									CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
									pUnitNode = pLoopSelectionGroup->nextUnitNode(pUnitNode);
									
									if (!pLoopUnit->canLoadUnit(this, plot(), true))
									{
										bValid = false;
										break;							
									}
								}
							}
								
							if (bValid)
							{
								CvPlot* pLoopPlot = NULL;
								int iPathTurns = 0;
								if ((atPlot(pMissionPlot) || (canMoveInto(pMissionPlot) && generatePath(pMissionPlot, MOVE_NO_ENEMY_TERRITORY, true, &iPathTurns))) && (iPathTurns <= iMaxPathTurns))
								{
									pLoopPlot = pMissionPlot;
								}
								else
								{
									int iBestDirectionValue = MAX_INT;
									for (int iDirection = 0; iDirection < NUM_DIRECTION_TYPES; iDirection++)
									{
										CvPlot* pDirectionPlot = plotDirection(pMissionPlot->getX_INLINE(), pMissionPlot->getY_INLINE(), (DirectionTypes)iDirection);
										if (pDirectionPlot != NULL)
										{
											if (pDirectionPlot->getArea() == pWaterArea->getID())
											{
												if (generatePath(pDirectionPlot, MOVE_NO_ENEMY_TERRITORY, true, &iPathTurns))
												{
													if (iPathTurns <= iMaxPathTurns)
													{
														int iValue = getPathCost();

														if (iValue < iBestDirectionValue)
														{
															iBestDirectionValue = iValue;
															pLoopPlot = pDirectionPlot;
														}
													}
												}
											}
										}
									}
								}
								if (pLoopPlot != NULL)
								{
									int iValue = 500;
									if (pHeadUnit->AI_getUnitAIType() == UNITAI_TREASURE)
									{
										iValue += pHeadUnit->getYieldStored();
									}
									
									iValue *= 1000;
									iValue /= 100 + ((iPathTurns == 0) ? 0 : getPathCost());
									
									int iDistance = stepDistance(pLoopSelectionGroup->getX(), pLoopSelectionGroup->getY(), pMissionPlot->getX_INLINE(), pMissionPlot->getY_INLINE());
									
									iValue *= 100;												
									iValue /= 100 + ((100 * iDistance) / pLoopSelectionGroup->baseMoves());
									if (pHeadUnit->AI_getUnitAIType() == UNITAI_TREASURE)
									{
										iValue *= 3;
									}
									
									if (iValue > iBestValue)
									{
										iBestValue = iValue;
										pBestPlot = pLoopPlot;
										pBestMissionPlot = pLoopPlot;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	
	if (pBestPlot != NULL)
	{
		if (atPlot(pBestPlot))
		{
			int iOldCargo = getCargo();
			if (AI_pickupAdjacantUnits())
			{
				getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_PICKUP, pBestMissionPlot);
				return true;
			}
			if (getCargo() > iOldCargo)
			{
				return true;
			}
			return false;
		}
		else
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), MOVE_NO_ENEMY_TERRITORY, false, false, MISSIONAI_PICKUP, pBestMissionPlot);
			return true;
		}
	}
	return false;
}

bool CvUnitAI::AI_pickupAdjacantUnits()
{
	std::vector<CvUnit*> units;
	
	for (int iX = -1; iX <= 1; ++iX)
	{
		for (int iY = -1; iY <= 1; ++iY)
		{
			CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iX, iY);
			if (pLoopPlot != NULL)
			{
				CLLNode<IDInfo>* pUnitNode = pLoopPlot->headUnitNode();

				int iCount = 0;
				while (pUnitNode != NULL)
				{
					CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);
					
					if ((pLoopUnit->getGroup()->AI_getMissionAIType() == MISSIONAI_AWAIT_PICKUP) && pLoopUnit->canLoadUnit(this, plot(), true))
					{
						units.push_back(pLoopUnit);	
					}
				}
			}
		}
	}
	
	if (units.empty())
	{
		return false;
	}
	
	while (!units.empty() && !isFull())
		{
		CvUnit* pLoopUnit = units.back();
		units.pop_back();
		
		pLoopUnit->joinGroup(NULL);
		pLoopUnit->AI_setMovePriority(AI_getMovePriority() + 1);
		if (pLoopUnit->atPlot(plot()))
		{
			pLoopUnit->loadUnit(this);
		}
		else
		{
			pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, getX_INLINE(), getY_INLINE());
		}
	}
	return false;
}

bool CvUnitAI::AI_continueMission(int iAbortDistance, MissionAITypes eValidMissionAI, int iFlags, bool bStepwise)
{
	CvPlot* pMissionPlot = getGroup()->AI_getMissionAIPlot();
	MissionAITypes eMissionAI = getGroup()->AI_getMissionAIType();
	if (pMissionPlot == NULL || eMissionAI == NO_MISSIONAI)
	{
		return false;
	}
	
	if (eMissionAI != NO_MISSIONAI && eMissionAI != eValidMissionAI)
	{
		return false;
	}
	
	if (stepDistance(getX_INLINE(), getY_INLINE(), pMissionPlot->getX_INLINE(), pMissionPlot->getY_INLINE()) <= iAbortDistance)
	{
		return false;
	}
	
	if (atPlot(pMissionPlot))
	{
		return false;
	}
	else if (generatePath(pMissionPlot, iFlags, true))
	{	
		CvPlot* pNextPlot = bStepwise ? getGroup()->getPathFirstPlot() : getGroup()->getPathEndTurnPlot();
		if ((pNextPlot == NULL) || atPlot(pNextPlot))
		{
			return false;
		}

		getGroup()->pushMission(MISSION_MOVE_TO, pNextPlot->getX_INLINE(), pNextPlot->getY_INLINE(), iFlags, false, false, eMissionAI, pMissionPlot);
		return true;
	}
	
	return false;
}

bool CvUnitAI::AI_breakAutomation()
{
	if (!isAutomated())
	{
		return false;
	}
	
	bool bBreak = false;
	if ((getGroup()->getAutomateType() == AUTOMATE_TRANSPORT_ROUTES) || (getGroup()->getAutomateType() == AUTOMATE_TRANSPORT_FULL))
	{
		CLLNode<IDInfo>* pUnitNode = plot()->headUnitNode();
		CvUnit* pLoopUnit;
		while (pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = plot()->nextUnitNode(pUnitNode);
			if (pLoopUnit->isCargo())
			{
				if (pLoopUnit->getTransportUnit()->getGroup() == getGroup())
				{
					if ((pLoopUnit->getYield() == NO_YIELD) || (pLoopUnit->AI_getUnitAIState() == UNITAI_STATE_PURCHASED))
					{
						bBreak = true;
					}
				}
			}
		}
	}
	
	if (!bBreak && GET_PLAYER(getOwnerINLINE()).AI_getUnitDanger(this, 2, false, false))
	{
		bBreak = true;
	}
	
	if (bBreak)
	{
		getGroup()->setAutomateType(NO_AUTOMATE);
		return true;
	}
	return false;
}

// Returns true if a group was joined or a mission was pushed...
bool CvUnitAI::AI_group(UnitAITypes eUnitAI, int iMaxGroup, int iMaxOwnUnitAI, int iMinUnitAI, bool bIgnoreFaster, bool bIgnoreOwnUnitType, bool bStackOfDoom, int iMaxPath, bool bAllowRegrouping)
{
	PROFILE_FUNC();

	CvUnit* pLoopUnit;
	CvUnit* pBestUnit;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iLoop;

	// if we are on a transport, then do not regroup
	if (isCargo())
	{
		return false;
	}

	if (!bAllowRegrouping)
	{
		if (getGroup()->getNumUnits() > 1)
		{
			return false;
		}
	}
	
	if ((getDomainType() == DOMAIN_LAND) && !m_pUnitInfo->isCanMoveAllTerrain())
	{
		if (area()->getNumAIUnits(getOwnerINLINE(), eUnitAI) == 0)
		{
			return false;
		}
	}

	if (!AI_canGroupWithAIType(eUnitAI))
	{
		return false;
	}

	iBestValue = MAX_INT;
	pBestUnit = NULL;

	CvSelectionGroup* pThisGroup = getGroup();
	for(pLoopUnit = GET_PLAYER(getOwnerINLINE()).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER(getOwnerINLINE()).nextUnit(&iLoop))
	{
		CvSelectionGroup* pLoopGroup = pLoopUnit->getGroup();
		CvPlot* pPlot = pLoopUnit->plot();
		if (AI_plotValid(pPlot))
		{
			if (iMaxPath > 0 || pPlot == plot())
			{
				if (!isEnemy(pPlot->getTeam()))
				{
					if (AI_allowGroup(pLoopUnit, eUnitAI))
					{
						if ((iMaxGroup == -1) || ((pLoopGroup->getNumUnits() + GET_PLAYER(getOwnerINLINE()).AI_unitTargetMissionAIs(pLoopUnit, MISSIONAI_GROUP, getGroup())) <= (iMaxGroup + ((bStackOfDoom) ? AI_stackOfDoomExtra() : 0))))
						{
							if ((iMaxOwnUnitAI == -1) || (pLoopGroup->countNumUnitAIType(AI_getUnitAIType()) <= (iMaxOwnUnitAI + ((bStackOfDoom) ? AI_stackOfDoomExtra() : 0))))
							{
								if ((iMinUnitAI == -1) || (pLoopGroup->countNumUnitAIType(eUnitAI) >= iMinUnitAI))
								{
									if (!bIgnoreFaster || (pLoopUnit->getGroup()->baseMoves() <= baseMoves()))
									{
										if (!(pLoopUnit->plot()->isVisibleEnemyUnit(this)))
										{
											if (!bIgnoreOwnUnitType || (pLoopUnit->getUnitType() != getUnitType()))
											{
												if (!(pPlot->isVisibleEnemyUnit(this)))
												{
													if (generatePath(pPlot, 0, true, &iPathTurns))
													{
														if (iPathTurns <= iMaxPath)
														{
															iValue = 1000 * (iPathTurns + 1);
															iValue *= 4 + pLoopGroup->getCargo();
															iValue /= pLoopGroup->getNumUnits();


															if (iValue < iBestValue)
															{
																iBestValue = iValue;
																pBestUnit = pLoopUnit;
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

	if (pBestUnit != NULL)
	{
		if (atPlot(pBestUnit->plot()))
		{
			joinGroup(pBestUnit->getGroup());
			return true;
		}
		else
		{
			getGroup()->pushMission(MISSION_MOVE_TO_UNIT, pBestUnit->getOwnerINLINE(), pBestUnit->getID(), 0, false, false, MISSIONAI_GROUP, NULL, pBestUnit);
			return true;
		}
	}

	return false;
}

bool CvUnitAI::AI_groupMergeRange(UnitAITypes eUnitAI, int iMaxRange, bool bBiggerOnly, bool bAllowRegrouping, bool bIgnoreFaster)
{
	PROFILE_FUNC();


 	// if we are on a transport, then do not regroup
	if (isCargo())
	{
		return false;
	}

   if (!bAllowRegrouping)
	{
		if (getGroup()->getNumUnits() > 1)
		{
			return false;
		}
	}
	
	if ((getDomainType() == DOMAIN_LAND) && !m_pUnitInfo->isCanMoveAllTerrain())
	{
		if (area()->getNumAIUnits(getOwnerINLINE(), eUnitAI) == 0)
		{
			return false;
		}
	}
	
	if (!AI_canGroupWithAIType(eUnitAI))
	{
		return false;
	}
	
	// cached values
	CvPlot* pPlot = plot();
	CvSelectionGroup* pGroup = getGroup();
	
	// best match
	CvUnit* pBestUnit = NULL;
	int iBestValue = MAX_INT;
	// iterate over plots at each range
	for (int iDX = -(iMaxRange); iDX <= iMaxRange; iDX++)
	{
		for (int iDY = -(iMaxRange); iDY <= iMaxRange; iDY++)
		{
			CvPlot* pLoopPlot = plotXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iDX, iDY);
			
			if (pLoopPlot != NULL && pLoopPlot->getArea() == pPlot->getArea())
			{
				CLLNode<IDInfo>* pUnitNode = pLoopPlot->headUnitNode();
				while (pUnitNode != NULL)
				{
					CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);
					
					CvSelectionGroup* pLoopGroup = pLoopUnit->getGroup();

					if (AI_allowGroup(pLoopUnit, eUnitAI))
					{
						if (!bIgnoreFaster || (pLoopUnit->getGroup()->baseMoves() <= baseMoves()))
						{
							if (!bBiggerOnly || (pLoopGroup->getNumUnits() >= pGroup->getNumUnits()))
							{
								int iPathTurns;
								if (generatePath(pLoopPlot, 0, true, &iPathTurns))
								{
									if (iPathTurns <= (iMaxRange + 2))
									{
										int iValue = 1000 * (iPathTurns + 1);
										iValue /= pLoopGroup->getNumUnits();

										if (iValue < iBestValue)
										{
											iBestValue = iValue;
											pBestUnit = pLoopUnit;
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

	if (pBestUnit != NULL)
	{
		if (atPlot(pBestUnit->plot()))
		{
			pGroup->mergeIntoGroup(pBestUnit->getGroup());
			return true;
		}
		else
		{
			pGroup->pushMission(MISSION_MOVE_TO_UNIT, pBestUnit->getOwnerINLINE(), pBestUnit->getID(), 0, false, false, MISSIONAI_GROUP, NULL, pBestUnit);
			return true;
		}
	}

	return false;
}

// Returns true if we loaded onto a transport or a mission was pushed...
bool CvUnitAI::AI_load(UnitAITypes eUnitAI, MissionAITypes eMissionAI, UnitAITypes eTransportedUnitAI, int iMinCargo, int iMinCargoSpace, int iMaxCargoSpace, int iMaxCargoOurUnitAI, int iFlags, int iMaxPath)
{
	PROFILE_FUNC();

	CvUnit* pLoopUnit;
	CvUnit* pBestUnit;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iLoop;

	// XXX what to do about groups???
	/*if (getGroup()->getNumUnits() > 1)
	{
		return false;
	}*/

	if (getCargo() > 0)
	{
		return false;
	}

	if (isCargo())
	{
		getGroup()->pushMission(MISSION_SKIP);
		return true;
	}
	
	if ((getDomainType() == DOMAIN_LAND) && !m_pUnitInfo->isCanMoveAllTerrain())
	{
		if (area()->getNumAIUnits(getOwnerINLINE(), eUnitAI) == 0)
		{
			return false;
		}
	}

	// do not load transports if we are already in a land war
	AreaAITypes eAreaAIType = area()->getAreaAIType(getTeam());
	bool bLandWar = ((eAreaAIType == AREAAI_OFFENSIVE) || (eAreaAIType == AREAAI_DEFENSIVE) || (eAreaAIType == AREAAI_MASSING));
	if (bLandWar && (eMissionAI != MISSIONAI_LOAD_SETTLER))
	{
		return false;
	}

	iBestValue = MAX_INT;
	pBestUnit = NULL;
	
	const int iLoadMissionAICount = 4;
	MissionAITypes aeLoadMissionAI[iLoadMissionAICount] = {MISSIONAI_LOAD_ASSAULT, MISSIONAI_LOAD_SETTLER, MISSIONAI_LOAD_SPECIAL};

	int iCurrentGroupSize = getGroup()->getNumUnits();

	for(pLoopUnit = GET_PLAYER(getOwnerINLINE()).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER(getOwnerINLINE()).nextUnit(&iLoop))
	{
		if (pLoopUnit != this)
		{
			if (AI_plotValid(pLoopUnit->plot()))
			{
				if (canLoadUnit(pLoopUnit, pLoopUnit->plot(), true))
				{

					UnitAITypes eLoopUnitAI = pLoopUnit->AI_getUnitAIType();
					if (eLoopUnitAI == eUnitAI)
					{
						int iCargoSpaceAvailable = pLoopUnit->cargoSpaceAvailable(getSpecialUnitType(), getDomainType());
						iCargoSpaceAvailable -= GET_PLAYER(getOwnerINLINE()).AI_unitTargetMissionAIs(pLoopUnit, aeLoadMissionAI, iLoadMissionAICount, getGroup());
						if (iCargoSpaceAvailable > 0)
						{
							if ((eTransportedUnitAI == NO_UNITAI) || (pLoopUnit->getUnitAICargo(eTransportedUnitAI) > 0))
							{
								if ((iMinCargo == -1) || (pLoopUnit->getCargo() >= iMinCargo))
								{
									if ((iMinCargoSpace == -1) || (pLoopUnit->cargoSpaceAvailable() >= iMinCargoSpace))
									{
										if ((iMaxCargoSpace == -1) || (pLoopUnit->cargoSpaceAvailable() <= iMaxCargoSpace))
										{
											if ((iMaxCargoOurUnitAI == -1) || (pLoopUnit->getUnitAICargo(AI_getUnitAIType()) <= iMaxCargoOurUnitAI))
											{
												{
													if (!(pLoopUnit->plot()->isVisibleEnemyUnit(this)))
													{
														CvPlot* pUnitTargetPlot = pLoopUnit->getGroup()->AI_getMissionAIPlot();
														if ((pUnitTargetPlot == NULL) || (pUnitTargetPlot->getTeam() == getTeam()) || (!pUnitTargetPlot->isOwned() || !isPotentialEnemy(pUnitTargetPlot->getTeam(), pUnitTargetPlot)))
														{
															if (generatePath(pLoopUnit->plot(), iFlags, true, &iPathTurns))
															{
																if (iPathTurns <= iMaxPath)
																{
																	// prefer a transport that can hold as much of our group as possible 
																	iValue = (std::max(0, iCurrentGroupSize - iCargoSpaceAvailable) * 5) + iPathTurns;

																	if (iValue < iBestValue)
																	{
																		iBestValue = iValue;
																		pBestUnit = pLoopUnit;
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

	if (pBestUnit != NULL)
	{
		if (atPlot(pBestUnit->plot()))
		{
			getGroup()->setTransportUnit(pBestUnit); // XXX is this dangerous (not pushing a mission...)
			return true;
		}
		else
		{
			int iCargoSpaceAvailable = pBestUnit->cargoSpaceAvailable(getSpecialUnitType(), getDomainType());
			FAssertMsg(iCargoSpaceAvailable > 0, "best unit has no space");

			// split our group to fit on the transport
			CvSelectionGroup* pSplitGroup = getGroup()->splitGroup(iCargoSpaceAvailable, this);
			FAssertMsg(pSplitGroup != NULL, "splitGroup failed");
			FAssertMsg(m_iGroupID == pSplitGroup->getID(), "splitGroup failed to put unit in the new group");

			if (pSplitGroup != NULL)
			{
				pSplitGroup->pushMission(MISSION_MOVE_TO_UNIT, pBestUnit->getOwnerINLINE(), pBestUnit->getID(), iFlags, false, false, eMissionAI, NULL, pBestUnit);
				return true;
			}
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_guardCityBestDefender()
{
	CvCity* pCity;
	CvPlot* pPlot;

	pPlot = plot();
	pCity = pPlot->getPlotCity();

	if (pCity != NULL)
	{
		if (pCity->getOwnerINLINE() == getOwnerINLINE())
		{
			if (pPlot->getBestDefender(getOwnerINLINE()) == this)
			{
				getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_GUARD_CITY, NULL);
				return true;
			}
		}
	}

	return false;
}

// Returns true if a mission was pushed...
bool CvUnitAI::AI_guardCity(bool bAll, int iMaxPath)
{
	PROFILE_FUNC();
	
	bool bIncludePotential = !GET_PLAYER(getOwnerINLINE()).isNative();
	if (bIncludePotential && area()->getAreaAIType(getTeam()) != AREAAI_NEUTRAL)
	{
		bIncludePotential = false;
	}
		

	FAssert(getDomainType() == DOMAIN_LAND);
	FAssert(canFight());

	CvPlot* pPlot = plot();
	CvCity* pCity = pPlot->getPlotCity();
	
	if (!bAll)
	{
		if ((pCity != NULL) && (pCity->getOwnerINLINE() == getOwnerINLINE())) // XXX check for other team?
		{
			int iExtra = 1;
			int iDanger = GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(pPlot, 2);
			iExtra += -iDanger;

			int iNeededDefenders = pCity->AI_neededDefenders();
			int iDefenders = pCity->AI_numDefenders(true, bIncludePotential) + iExtra;

			if (iNeededDefenders < iDefenders)
			{
				getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_GUARD_CITY, NULL);
				return true;
			}
		}
	}

	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	CvPlot* pBestGuardPlot = NULL;
	
	int iLoop;
	CvCity* pLoopCity;
	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		CvPlot* pLoopPlot = pLoopCity->plot();
		if (AI_plotValid(pLoopPlot))
		{
			int iExtra = 0;//(!AI_isCityAIType() ? pLoopPlot->plotCount(PUF_canDefendGroupHead, -1, -1, getOwnerINLINE(), NO_TEAM, PUF_isNotCityAIType) : 0);
			if (atPlot(pLoopPlot) && AI_isCityAIType())
			{
				iExtra -= 1;
			}
			int iIncoming = GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_GUARD_CITY, getGroup());

			int iNeededDefenders = pLoopCity->AI_neededDefenders();
			int iDefenders = std::max(0, pLoopCity->AI_numDefenders(true, bIncludePotential) + iExtra + iIncoming);
			
			FAssert(iDefenders >= 0);
			
			if (bAll || (iDefenders < iNeededDefenders))
			{
				if (!(pLoopPlot->isVisibleEnemyUnit(this)))
				{
					if ((GC.getGame().getGameTurn() - pLoopCity->getGameTurnAcquired() < 10) || iIncoming < 3)
					{
						int iPathTurns = 0;
						if (atPlot(pLoopPlot) || generatePath(pLoopPlot, 0, true, &iPathTurns))
						{
							if (iPathTurns <= iMaxPath)
							{
								int iValue = (1000 * iNeededDefenders) / (iDefenders + 1);
								
								if (iDefenders >= iNeededDefenders)
								{
									iValue /= 2;
								}

								if (iDefenders == 0)
								{
									iValue *= 4;
								}
								
								iValue /= 5 + iPathTurns;

								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									pBestPlot = atPlot(pLoopPlot) ? pLoopPlot : getPathEndTurnPlot();
									pBestGuardPlot = pLoopPlot;
								}
							}
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestGuardPlot != NULL))
	{
		if (atPlot(pBestGuardPlot))
		{
			getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_GUARD_CITY, NULL);
		}
		else
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_GUARD_CITY, pBestGuardPlot);
		}
		if (GET_PLAYER(getOwnerINLINE()).isNative())
		{
			setHomeCity(pBestGuardPlot->getPlotCity());
		}

		return true;				
	}

	return false;
}

bool CvUnitAI::AI_guardCityMinDefender()
{
	bool bIncludePotential = !GET_PLAYER(getOwnerINLINE()).isNative();
	if (bIncludePotential && area()->getAreaAIType(getTeam()) != AREAAI_NEUTRAL)
	{
		bIncludePotential = false;
	}
	
	CvUnit* pTransportUnit = getTransportUnit();

	bool bForceTransport = false;
	if (plot()->isCity() && (plot()->getOwnerINLINE() != getOwnerINLINE()))
	{
		bForceTransport = true;
	}
	
	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	bool bBestIsTransport = false;
	
	int iLoop;
	bool bTransportPath = false;
	for (CvCity* pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		CvPlot* pLoopPlot = pLoopCity->plot();
		if (!atPlot(pLoopPlot) && AI_plotValid(pLoopPlot))
		{
			int iDefenders = pLoopCity->AI_numDefenders(true,bIncludePotential);
			
			if (iDefenders == 0)
			{
				int iIncoming = GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_GUARD_CITY, getGroup());
				
				if (iIncoming == 0)
				{
					bool bTransport = (pTransportUnit != NULL);
					if (!bForceTransport && canUnload() && (pLoopPlot->area() == area()))
					{
						bTransport = false;
					}
					int iPathTurns = 0;
					bool bValid;
					if (bTransport)
					{
						bValid = pTransportUnit->generatePath(pLoopPlot, 0, bTransportPath, &iPathTurns);
					}
					else
					{
						bValid = generatePath(pLoopPlot, MOVE_NO_ENEMY_TERRITORY, !bTransportPath, &iPathTurns);
					}
					bTransportPath = bTransport;
					if (bValid)
					{
						int iValue = (pLoopCity->AI_neededDefenders() + pLoopCity->getPopulation()) * 1000;
						iValue /= 1 + iPathTurns;
						
						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							pBestPlot = pLoopPlot;
							bBestIsTransport = bTransport;
						}				
					}
				}
			}
		}
	}
	
	if (pBestPlot != NULL)
	{
		if (bBestIsTransport)
		{
			getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_FOUND, pBestPlot);
			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), MOVE_NO_ENEMY_TERRITORY, false, false, MISSIONAI_FOUND, pBestPlot);
			return true;
		}
	}
	return false;
}

bool CvUnitAI::AI_guardCityCounter(int iMaxPath)
{
	CvUnit* pTransportUnit = getTransportUnit();

	bool bForceTransport = false;
	if (plot()->isCity() && (plot()->getOwnerINLINE() != getOwnerINLINE()))
	{
		bForceTransport = true;
	}
	
	CvTeamAI& kTeam = GET_TEAM(getTeam());
	int iCurrentDistance = kTeam.AI_enemyUnitDistance(plot());
	if (iCurrentDistance < 2)
	{
		return false;
	}
	
	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	bool bBestIsTransport = false;
	
	int iLoop;
	CvCity* pLoopCity;
	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		CvPlot* pLoopPlot = pLoopCity->plot();
		if (!atPlot(pLoopPlot) && AI_plotValid(pLoopPlot))
		{
			int iEnemyDistance = kTeam.AI_enemyUnitDistance(pLoopPlot);
			if (iEnemyDistance != -1)
			{
				int iValue = 1000 / (1 + iEnemyDistance);
				
				if (iEnemyDistance < iCurrentDistance)
				{
					iValue *= 2;
				}
				
				int iPathTurns = 0;
				if (atPlot(pLoopPlot) || generatePath(pLoopPlot, 0, true, &iPathTurns))
				{
					if (iPathTurns < iMaxPath)
					{
						iValue /= 3 + iPathTurns;
					
						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							pBestPlot = pLoopPlot;
						}
					}
				}
			}
		}
	}
	
	if (pBestPlot != NULL)
	{
		if (atPlot(pBestPlot))
		{
			return false;
		}
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
		return true;
	}
	return false;
}

int CvUnitAI::AI_getPlotDefendersNeeded(CvPlot* pPlot, int iExtra)
{
	int iNeeded = iExtra;

	int iDefense = pPlot->defenseModifier(getTeam());

	iNeeded += (iDefense + 25) / 50;

	if (iNeeded == 0)
	{
		return 0;
	}

	int iNumHostiles = 0;
	int iNumPlots = 0;

	int iRange = 2;
	for (int iX = -iRange; iX <= iRange; iX++)
	{
		for (int iY = -iRange; iY <= iRange; iY++)
		{
			CvPlot* pLoopPlot = plotXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iX, iY);
			if (pLoopPlot != NULL)
			{
				iNumHostiles += pLoopPlot->getNumVisibleEnemyDefenders(this);
				if ((pLoopPlot->getTeam() != getTeam()) || pLoopPlot->isCoastalLand())
				{
				    iNumPlots++;
                    if (isEnemy(pLoopPlot->getTeam()))
                    {
                        iNumPlots += 4;
                    }
				}
			}
		}
	}

	if ((iNumHostiles == 0) && (iNumPlots < 4))
	{
		if (iNeeded > 1)
		{
			iNeeded = 1;
		}
		else
		{
			iNeeded = 0;
		}
	}

	return iNeeded;
}

bool CvUnitAI::AI_guardFort(bool bSearch)
{
	PROFILE_FUNC();
	
	if (plot()->getOwnerINLINE() == getOwnerINLINE())
	{
		ImprovementTypes eImprovement = plot()->getImprovementType();
		if (eImprovement != NO_IMPROVEMENT)
		{
			if (GC.getImprovementInfo(eImprovement).isActsAsCity())
			{
				if (plot()->plotCount(PUF_isCityAIType, -1, -1, getOwnerINLINE()) <= AI_getPlotDefendersNeeded(plot(), 0))
				{
					getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_GUARD_BONUS, plot());
					return true;
				}
			}
		}
	}
	
	if (!bSearch)
	{
		return false;
	}

	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	CvPlot* pBestGuardPlot = NULL;

	for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (AI_plotValid(pLoopPlot) && !atPlot(pLoopPlot))
		{
			if (pLoopPlot->getOwnerINLINE() == getOwnerINLINE())
			{
				ImprovementTypes eImprovement = pLoopPlot->getImprovementType();
				if (eImprovement != NO_IMPROVEMENT)
				{
					if (GC.getImprovementInfo(eImprovement).isActsAsCity())
					{
						int iValue = AI_getPlotDefendersNeeded(pLoopPlot, 0);

						if (iValue > 0)
						{
							if (!(pLoopPlot->isVisibleEnemyUnit(this)))
							{
								if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_GUARD_BONUS, getGroup()) < iValue)
								{
									int iPathTurns;
									if (generatePath(pLoopPlot, 0, true, &iPathTurns))
									{
										iValue *= 1000;

										iValue /= (iPathTurns + 2);

										if (iValue > iBestValue)
										{
											iBestValue = iValue;
											pBestPlot = getPathEndTurnPlot();
											pBestGuardPlot = pLoopPlot;
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

	if ((pBestPlot != NULL) && (pBestGuardPlot != NULL))
	{
		if (atPlot(pBestGuardPlot))
		{
			getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_GUARD_BONUS, pBestGuardPlot);
			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_GUARD_BONUS, pBestGuardPlot);
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_chokeDefend()
{
	CvCity* pCity;
	int iPlotDanger;

	FAssert(AI_isCityAIType());

	// XXX what about amphib invasions?

	pCity = plot()->getPlotCity();

	if (pCity != NULL)
	{
		if (pCity->getOwnerINLINE() == getOwnerINLINE())
		{
			if (pCity->AI_neededDefenders() > 1)
			{
				if (pCity->AI_isDefended(pCity->plot()->plotCount(PUF_canDefendGroupHead, -1, -1, getOwnerINLINE(), NO_TEAM, PUF_isNotCityAIType)))
				{
					iPlotDanger = GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot(), 3);

					if (iPlotDanger <= 4)
					{
						if (AI_anyAttack(1, 65, std::max(0, (iPlotDanger - 1))))
						{
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_heal(int iDamagePercent, int iMaxPath)
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pEntityNode;
	std::vector<CvUnit*> aeDamagedUnits;
	CvSelectionGroup* pGroup;
	CvUnit* pLoopUnit;
	int iTotalDamage;
	int iTotalHitpoints;
	int iHurtUnitCount;
	bool bRetreat;
	
	pGroup = getGroup();
	
	if (iDamagePercent == 0)
	{
	    iDamagePercent = 10;	    
	}	

	bRetreat = false;
	
	if (getDomainType() == DOMAIN_SEA)
	{
		if (plot()->getDistanceToOcean() < 10)
		{
			if (AI_sailToEurope())
			{
				return true;
			}
		}
	}
	
    if (getGroup()->getNumUnits() == 1)
	{
	    if (getDamage() > 0)
        {
        	
            if (plot()->isCity() || (healTurns(plot()) == 1))
            {
                if (!(isAlwaysHeal()))
                {
                    getGroup()->pushMission(MISSION_HEAL);
                    return true;
                }
            }
        }
        return false;
	}
	
	iMaxPath = std::min(iMaxPath, 2);

	pEntityNode = getGroup()->headUnitNode();

    iTotalDamage = 0;
    iTotalHitpoints = 0;
    iHurtUnitCount = 0;
	while (pEntityNode != NULL)
	{
		pLoopUnit = ::getUnit(pEntityNode->m_data);
		FAssert(pLoopUnit != NULL);
		pEntityNode = pGroup->nextUnitNode(pEntityNode);

		int iDamageThreshold = (pLoopUnit->maxHitPoints() * iDamagePercent) / 100;

		if (NO_UNIT != getLeaderUnitType())
		{
			iDamageThreshold /= 2;
		}
		
		if (pLoopUnit->getDamage() > 0)
		{
		    iHurtUnitCount++;
		}
		iTotalDamage += pLoopUnit->getDamage();
		iTotalHitpoints += pLoopUnit->maxHitPoints();
		

		if (pLoopUnit->getDamage() > iDamageThreshold)
		{
			bRetreat = true;

			if (!(pLoopUnit->hasMoved()))
			{
				if (!(pLoopUnit->isAlwaysHeal()))
				{
					if (pLoopUnit->healTurns(pLoopUnit->plot()) <= iMaxPath)
					{
					    aeDamagedUnits.push_back(pLoopUnit);
					}
				}
			}
		}
	}
	if (iHurtUnitCount == 0)
	{
	    return false;
	}
	
	bool bPushedMission = false;
    if (plot()->isCity() && (plot()->getOwnerINLINE() == getOwnerINLINE()))
	{
		FAssertMsg(((int) aeDamagedUnits.size()) <= iHurtUnitCount, "damaged units array is larger than our hurt unit count");

		for (unsigned int iI = 0; iI < aeDamagedUnits.size(); iI++)
		{
			CvUnit* pUnitToHeal = aeDamagedUnits[iI];
			pUnitToHeal->joinGroup(NULL);
			pUnitToHeal->getGroup()->pushMission(MISSION_HEAL);

			// note, removing the head unit from a group will force the group to be completely split if non-human
			if (pUnitToHeal == this)
			{
				bPushedMission = true;
			}

			iHurtUnitCount--;
		}
	}
	
	if ((iHurtUnitCount * 2) > pGroup->getNumUnits())
	{
		FAssertMsg(pGroup->getNumUnits() > 0, "group now has zero units");
	
	    if (AI_moveIntoCity(2))
		{
			return true;
		}
		else if (healRate(plot()) > 10)
	    {
            pGroup->pushMission(MISSION_HEAL);
            return true;
	    }
	}
	
	return bPushedMission;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_afterAttack()
{
	if (isHuman())
	{
		return false;
	}

	if (!isMadeAttack())
	{
		return false;
	}
	
	if (!canFight())
	{
		return false;
	}

	if (isBlitz())
	{
		return false;
	}

	if (getDomainType() == DOMAIN_LAND)
	{
		if (AI_guardCity(false))
		{
			return true;
		}
	}

	if (AI_pillageRange(1))
	{
		return true;
	}

	if (AI_retreatToCity(false, 1))
	{
		return true;
	}

	if (AI_hide())
	{
		return true;
	}

	if (AI_pillageRange(2))
	{
		return true;
	}

	if (AI_defend())
	{
		return true;
	}

	if (AI_safety())
	{
		return true;
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_lead(std::vector<UnitAITypes>& aeUnitAITypes)
{
	PROFILE_FUNC();

	FAssertMsg(!isHuman(), "isHuman did not return false as expected");
	FAssertMsg(AI_getUnitAIType() != NO_UNITAI, "AI_getUnitAIType() is not expected to be equal with NO_UNITAI");
	FAssert(NO_PLAYER != getOwnerINLINE());

	CvPlayer& kOwner = GET_PLAYER(getOwnerINLINE());

	bool bNeedLeader = false;
	for (int iI = 0; iI < MAX_TEAMS; iI++)
	{
		CvTeamAI& kLoopTeam = GET_TEAM((TeamTypes)iI);
		if (isEnemy((TeamTypes)iI))
		{
			if (kLoopTeam.countNumUnitsByArea(area()) > 0)
			{
				bNeedLeader = true;
				break;
			}
		}
	}

	CvUnit* pBestUnit = NULL;
	CvPlot* pBestPlot = NULL;

	if (bNeedLeader)
	{
		int iBestStrength = 0;
		int iLoop;
		for (CvUnit* pLoopUnit = kOwner.firstUnit(&iLoop); pLoopUnit; pLoopUnit = kOwner.nextUnit(&iLoop))
		{
			for (uint iI = 0; iI < aeUnitAITypes.size(); iI++)
			{
				if (pLoopUnit->AI_getUnitAIType() == aeUnitAITypes[iI] || NO_UNITAI == aeUnitAITypes[iI])
				{
					if (canLead(pLoopUnit->plot(), pLoopUnit->getID()))
					{
						if (AI_plotValid(pLoopUnit->plot()))
						{
							if (!(pLoopUnit->plot()->isVisibleEnemyUnit(this)))
							{
								if (generatePath(pLoopUnit->plot(), 0, true))
								{
									// pick the unit with the highest current strength
									int iCombatStrength = pLoopUnit->currCombatStr(NULL, NULL);
									if (iCombatStrength > iBestStrength)
									{
										iBestStrength = iCombatStrength;
										pBestUnit = pLoopUnit;
										pBestPlot = getPathEndTurnPlot();
									}
								}
							}
						}
					}
					break;
				}
			}
		}
	}

	if (pBestPlot)
	{
		if (atPlot(pBestPlot) && pBestUnit)
		{
			getGroup()->pushMission(MISSION_LEAD, pBestUnit->getID());
			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
			return true;
		}
	}

	return false;
}

// Returns true if a mission was pushed...
bool CvUnitAI::AI_protect(int iOddsThreshold)
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	int iValue;
	int iBestValue;
	int iI;

	iBestValue = 0;
	pBestPlot = NULL;

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (AI_plotValid(pLoopPlot))
		{
			if (pLoopPlot->getOwnerINLINE() == getOwnerINLINE())
			{
				if (pLoopPlot->isVisibleEnemyUnit(this))
				{
					if (!atPlot(pLoopPlot) && generatePath(pLoopPlot, 0, true))
					{
						iValue = getGroup()->AI_attackOdds(pLoopPlot, true);

						if (iValue >= AI_finalOddsThreshold(pLoopPlot, iOddsThreshold))
						{
							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								pBestPlot = getPathEndTurnPlot();
								FAssert(!atPlot(pBestPlot));
							}
						}
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
		return true;
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_bravePatrol()
{
	PROFILE_FUNC();
	FAssert(canMove());

	CvPlot* pAdjacentPlot;
	CvPlot* pBestPlot;
	int iValue;
	int iBestValue;
	int iI;

	iBestValue = 0;
	pBestPlot = NULL;
	
	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());

	int iOldDistance = kOwner.AI_cityDistance(plot());

	for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
	{
		pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

		if (pAdjacentPlot != NULL)
		{
			if (AI_plotValid(pAdjacentPlot))
			{
				if (!(pAdjacentPlot->isVisibleEnemyUnit(this)))
				{
						iValue = 0;

						if (pAdjacentPlot->isRevealedGoody(getTeam()))
						{
							iValue -= 10000;
						}
						
						if (getHomeCity() != NULL)
						{
							int iDistance = kOwner.AI_cityDistance(plot());
							
							if (iDistance > iOldDistance)
							{
								iValue += 2500;
							}
							else if (iDistance == iOldDistance)
							{
								iValue += 1000;
							}
						}

						if (iValue > 0)
						{
							iValue = (1 + GC.getGameINLINE().getSorenRandNum(iValue, "AI Patrol"));

						if (iValue > iBestValue)
						{
							if (generatePath(pAdjacentPlot, 0, true))
							{
							iBestValue = iValue;
							pBestPlot = getPathEndTurnPlot();
							FAssert(!atPlot(pBestPlot));
						}
					}
				}
			}
		}
	}
	}

	if (pBestPlot != NULL)
	{
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
		return true;
	}

	return false;
}

// Returns true if a mission was pushed...
bool CvUnitAI::AI_patrol()
{
	PROFILE_FUNC();

	CvPlot* pAdjacentPlot;
	CvPlot* pBestPlot;
	int iValue;
	int iBestValue;
	int iI;

	iBestValue = 0;
	pBestPlot = NULL;

	for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
	{
		pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

		if (pAdjacentPlot != NULL)
		{
			if (AI_plotValid(pAdjacentPlot))
			{
				if (!(pAdjacentPlot->isVisibleEnemyUnit(this)))
				{
					if (generatePath(pAdjacentPlot, 0, true))
					{
						iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Patrol"));

							if (pAdjacentPlot->isRevealedGoody(getTeam()))
							{
								iValue += 100000;
							}

							if (pAdjacentPlot->getOwnerINLINE() == getOwnerINLINE())
							{
								iValue += 10000;
							}

						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							pBestPlot = getPathEndTurnPlot();
							FAssert(!atPlot(pBestPlot));
						}
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
		return true;
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_defend()
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	int iSearchRange;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iDX, iDY;

	if (AI_defendPlot(plot()))
	{
		getGroup()->pushMission(MISSION_SKIP);
		return true;
	}

	iSearchRange = AI_searchRange(1);

	iBestValue = 0;
	pBestPlot = NULL;

	for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			pLoopPlot	= plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (AI_plotValid(pLoopPlot))
				{
					if (AI_defendPlot(pLoopPlot))
					{
						if (!(pLoopPlot->isVisibleEnemyUnit(this)))
						{
							if (!atPlot(pLoopPlot) && generatePath(pLoopPlot, 0, true, &iPathTurns))
							{
								if (iPathTurns <= 1)
								{
									iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Defend"));

									if (iValue > iBestValue)
									{
										iBestValue = iValue;
										pBestPlot = pLoopPlot;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
		return true;
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_safety()
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvUnit* pHeadUnit;
	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	int iSearchRange;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iCount;
	int iPass;
	int iDX, iDY;

	iSearchRange = AI_searchRange(1);

	iBestValue = 0;
	pBestPlot = NULL;

	for (iPass = 0; iPass < 2; iPass++)
	{
		for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
		{
			for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
			{
				pLoopPlot	= plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

				if (pLoopPlot != NULL)
				{
					if (AI_plotValid(pLoopPlot))
					{
						if (!(pLoopPlot->isVisibleEnemyUnit(this)))
						{
							if (generatePath(pLoopPlot, ((iPass > 0) ? MOVE_IGNORE_DANGER : 0), true, &iPathTurns))
							{
								if (iPathTurns <= 1)
								{
									iCount = 0;

									pUnitNode = pLoopPlot->headUnitNode();

									while (pUnitNode != NULL)
									{
										pLoopUnit = ::getUnit(pUnitNode->m_data);
										pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);

										if (pLoopUnit->getOwnerINLINE() == getOwnerINLINE())
										{
											if (pLoopUnit->canDefend())
											{
												pHeadUnit = pLoopUnit->getGroup()->getHeadUnit();
												FAssert(pHeadUnit != NULL);
												FAssert(getGroup()->getHeadUnit() == this);

												if (pHeadUnit != this)
												{
													if (pHeadUnit->isWaiting() || !(pHeadUnit->canMove()))
													{
														FAssert(pLoopUnit != this);
														FAssert(pHeadUnit != getGroup()->getHeadUnit());
														iCount++;
													}
												}
											}
										}
									}

									iValue = (iCount * 100);

									iValue += pLoopPlot->defenseModifier(getTeam());

									if (atPlot(pLoopPlot))
									{
										iValue += 50;
									}
									else
									{
										iValue += GC.getGameINLINE().getSorenRandNum(50, "AI Safety");
									}

									if (iValue > iBestValue)
									{
										iBestValue = iValue;
										pBestPlot = pLoopPlot;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		if (atPlot(pBestPlot))
		{
			getGroup()->pushMission(MISSION_SKIP);
			return true;
		}
		else
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), ((iPass > 0) ? MOVE_IGNORE_DANGER : 0));
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_hide()
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvUnit* pHeadUnit;
	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	bool bValid;
	int iSearchRange;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iCount;
	int iDX, iDY;
	int iI;

	if (getInvisibleType() == NO_INVISIBLE)
	{
		return false;
	}

	iSearchRange = AI_searchRange(1);

	iBestValue = 0;
	pBestPlot = NULL;

	for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			pLoopPlot	= plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (AI_plotValid(pLoopPlot))
				{
					bValid = true;

					for (iI = 0; iI < MAX_TEAMS; iI++)
					{
						if (GET_TEAM((TeamTypes)iI).isAlive())
						{
							if (pLoopPlot->isInvisibleVisible(((TeamTypes)iI), getInvisibleType()))
							{
								bValid = false;
								break;
							}
						}
					}

					if (bValid)
					{
						if (!(pLoopPlot->isVisibleEnemyUnit(this)))
						{
							if (generatePath(pLoopPlot, 0, true, &iPathTurns))
							{
								if (iPathTurns <= 1)
								{
									iCount = 1;

									pUnitNode = pLoopPlot->headUnitNode();

									while (pUnitNode != NULL)
									{
										pLoopUnit = ::getUnit(pUnitNode->m_data);
										pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);

										if (pLoopUnit->getOwnerINLINE() == getOwnerINLINE())
										{
											if (pLoopUnit->canDefend())
											{
												pHeadUnit = pLoopUnit->getGroup()->getHeadUnit();
												FAssert(pHeadUnit != NULL);
												FAssert(getGroup()->getHeadUnit() == this);

												if (pHeadUnit != this)
												{
													if (pHeadUnit->isWaiting() || !(pHeadUnit->canMove()))
													{
														FAssert(pLoopUnit != this);
														FAssert(pHeadUnit != getGroup()->getHeadUnit());
														iCount++;
													}
												}
											}
										}
									}

									iValue = (iCount * 100);

									iValue += pLoopPlot->defenseModifier(getTeam());

									if (atPlot(pLoopPlot))
									{
										iValue += 50;
									}
									else
									{
										iValue += GC.getGameINLINE().getSorenRandNum(50, "AI Hide");
									}

									if (iValue > iBestValue)
									{
										iBestValue = iValue;
										pBestPlot = pLoopPlot;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		if (atPlot(pBestPlot))
		{
			getGroup()->pushMission(MISSION_SKIP);
			return true;
		}
		else
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_goody()
{
	PROFILE_FUNC();

	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	CvPlot* pBestExplorePlot = NULL;

	for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		PROFILE("AI_explore 1");

		CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (AI_plotValid(pLoopPlot))
		{
			int iValue = 0;

			if (pLoopPlot->isRevealedGoody(getTeam()))
			{
				iValue += 10000;
			}
			
			if (pLoopPlot->isCity())
			{
				if (!pLoopPlot->getPlotCity()->isScoutVisited(getTeam()))
				{
					if (canSpeakWithChief(pLoopPlot))
					{
						iValue += 10000;
					}
				}
			}

			if (iValue > 0)
			{
				if (!(pLoopPlot->isVisibleEnemyUnit(this)))
				{
					if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_EXPLORE, getGroup(), 3) == 0)
					{
						int iPathTurns = 0;
						if (!atPlot(pLoopPlot) && generatePath(pLoopPlot, MOVE_NO_ENEMY_TERRITORY, true, &iPathTurns))
						{
							iValue += GC.getGameINLINE().getSorenRandNum(250 * abs(xDistance(getX_INLINE(), pLoopPlot->getX_INLINE())) + abs(yDistance(getY_INLINE(), pLoopPlot->getY_INLINE())), "AI explore");

							iValue /= 3 + std::max(1, iPathTurns);

							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								pBestPlot = getPathEndTurnPlot();
								pBestExplorePlot = pLoopPlot;
							}
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestExplorePlot != NULL))
	{
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), MOVE_NO_ENEMY_TERRITORY, false, false, MISSIONAI_EXPLORE, pBestExplorePlot);
		return true;
	}

	return false;

}

bool CvUnitAI::AI_goodyRange(int iRange)
{
	PROFILE_FUNC();

	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	CvPlot* pBestExplorePlot = NULL;

	for (int iX = -iRange; iX <= iRange; iX++)
	{
		for (int iY = -iRange; iY <= iRange; iY++)
		{
			CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iX, iY);

			if (pLoopPlot != NULL && AI_plotValid(pLoopPlot))
			{
				int iValue = 0;

				if (pLoopPlot->isRevealedGoody(getTeam()))
				{
					iValue += 10000;
				}
				
				if (pLoopPlot->isCity())
				{
					if (!pLoopPlot->getPlotCity()->isScoutVisited(getTeam()))
					{
						if (canSpeakWithChief(pLoopPlot))
						{
							iValue += 10000;
						}
					}
				}

				if (iValue > 0)
				{
					if (!(pLoopPlot->isVisibleEnemyUnit(this)))
					{
						if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_EXPLORE, getGroup(), 1) == 0)
						{
							int iPathTurns = 0;
							if (!atPlot(pLoopPlot) && generatePath(pLoopPlot, MOVE_NO_ENEMY_TERRITORY, true, &iPathTurns))
							{
								iValue += GC.getGameINLINE().getSorenRandNum(250 * abs(xDistance(getX_INLINE(), pLoopPlot->getX_INLINE())) + abs(yDistance(getY_INLINE(), pLoopPlot->getY_INLINE())), "AI explore");

								iValue /= 3 + std::max(1, iPathTurns);

								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									pBestPlot = getPathEndTurnPlot();
									pBestExplorePlot = pLoopPlot;
								}
							}
						}
					}
				}
			}
		}
	}
	

	if ((pBestPlot != NULL) && (pBestExplorePlot != NULL))
	{
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), MOVE_NO_ENEMY_TERRITORY, false, false, MISSIONAI_EXPLORE, pBestExplorePlot);
		return true;
	}

	return false;
	
}

bool CvUnitAI::AI_isValidExplore(CvPlot* pPlot)
{
	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
	if (kOwner.getNumCities() == 0 || kOwner.AI_getNumAIUnits(AI_getUnitAIType()) > 1)
	{
		return true;
	}
	int iMaxDistance = MAX_INT;
	int iGameTurn = GC.getGameINLINE().getGameTurn();
	if (iGameTurn < 30)
	{
		iMaxDistance = 10 + iGameTurn / 5;

		if (getDomainType() == DOMAIN_SEA)
		{
			iMaxDistance += iGameTurn / 2;
		}
	}
	
	if (getDomainType() == DOMAIN_SEA || pPlot->area()->getCitiesPerPlayer(getOwnerINLINE()) > 0) 
	{
		if (kOwner.AI_cityDistance(pPlot) > iMaxDistance)
		{
			return false;	
		}
	}
	
	return true;
}
	
int CvUnitAI::AI_explorePlotValue(CvPlot* pPlot, bool bImportantOnly)
{
	int iValue = 0;
	if (pPlot->isRevealedGoody(getTeam()))
	{
		iValue += 50000;
	}
	
	if (pPlot->isCity())
	{
		if (!pPlot->getPlotCity()->isScoutVisited(getTeam()))
		{
			if (canSpeakWithChief(pPlot))
			{
				iValue += 50000;
			}
		}
	}
	
	if (iValue == 0)
	{
		if (!AI_isValidExplore(pPlot))
		{
			return 0;
		}
	}
	
	if (!bImportantOnly || iValue > 0)
	{
		for (int i = 0; i < NUM_DIRECTION_TYPES; i++)
		{
			CvPlot* pAdjacentPlot = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), ((DirectionTypes)i));

			if (pAdjacentPlot != NULL)
			{
				if (!(pAdjacentPlot->isRevealed(getTeam(), false)))
				{
					iValue += 1000;
					if (pPlot->isWater() != pAdjacentPlot->isWater())
					{
						iValue += 1000;
					}
					
					if (pAdjacentPlot->isRiver())
					{
						iValue += 500;
					}
				}
			}
		}
	}
	
	return iValue;
}

// Returns true if a mission was pushed...
bool CvUnitAI::AI_explore(bool bFavorOpenBorders)
{
	PROFILE_FUNC();

	CvPlot* pBestPlot;
	CvPlot* pBestExplorePlot;
	int iPathTurns;
	int iBestValue;
	int iI;

	iBestValue = 0;
	pBestPlot = NULL;
	pBestExplorePlot = NULL;
	
	bool bNoContact = (GC.getGameINLINE().countCivTeamsAlive() > GET_TEAM(getTeam()).getHasMetCivCount());

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		PROFILE("AI_explore 1");

		CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (AI_plotValid(pLoopPlot))
		{
			int iValue = AI_explorePlotValue(pLoopPlot, false);

			if (iValue > 0)
			{
				if (!(pLoopPlot->isVisibleEnemyUnit(this)))
				{
					if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_EXPLORE, getGroup(), 3) == 0)
					{
						if (!atPlot(pLoopPlot) && generatePath(pLoopPlot, MOVE_NO_ENEMY_TERRITORY, true, &iPathTurns))
						{
							iValue += GC.getGameINLINE().getSorenRandNum(250 * abs(xDistance(getX_INLINE(), pLoopPlot->getX_INLINE())) + abs(yDistance(getY_INLINE(), pLoopPlot->getY_INLINE())), "AI explore");

							if (pLoopPlot->isAdjacentToLand())
							{
								iValue += 10000;
							}

							if (bFavorOpenBorders && pLoopPlot->isOwned())
							{
								iValue *= 2;									
							}
							
							iValue /= 3 + std::max(1, iPathTurns);

							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								pBestPlot = pLoopPlot->isRevealedGoody(getTeam()) ? getPathEndTurnPlot() : pLoopPlot;
								pBestExplorePlot = pLoopPlot;
							}
						}
					}
				}
			}		
		}
	}

	if ((pBestPlot != NULL) && (pBestExplorePlot != NULL))
	{
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), MOVE_NO_ENEMY_TERRITORY, false, false, MISSIONAI_EXPLORE, pBestExplorePlot);
		return true;
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_exploreRange(int iRange)
{
	PROFILE_FUNC();

	if (canSpeakWithChief(plot()))
	{
		if (!plot()->getPlotCity()->isScoutVisited(getTeam()))
		{
			speakWithChief();
			return true;
		}
	}

	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	CvPlot* pBestExplorePlot = NULL;
	
	
	for (int iX = -iRange; iX <= iRange; iX++)
	{
		for (int iY = -iRange; iY <= iRange; iY++)
		{
			CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iX, iY);
			if ((pLoopPlot != NULL) && !atPlot(pLoopPlot))
			{
				if (pLoopPlot->isRevealed(getTeam(), false))
				{
					int iValue = AI_explorePlotValue(pLoopPlot, false);
					
					if (iValue > 0)
					{
						if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_EXPLORE, getGroup(), 3) == 0)
						{
							if (generatePath(pLoopPlot, MOVE_BUST_FOG, true))
							{
								if (getPathLastNode()->m_iData1 == 0)
								{
									iValue += pLoopPlot->seeFromLevel(getTeam()) * 500;
								}
								iValue *= 100;
								iValue += GC.getGame().getSorenRandNum(100, "AI Explore Range");
								iValue /= 100 + getPathCost();
								
								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									pBestExplorePlot = pLoopPlot;
									pBestPlot = getPathEndTurnPlot();
								}
							}
						}
					}
				}
			}
		}
	}
	
	if (pBestPlot != NULL)
	{
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), MOVE_BUST_FOG, false, false, MISSIONAI_EXPLORE, pBestExplorePlot);
		return true;
	}
	return false;
}

bool CvUnitAI::AI_exploreFromShip(int iMaxPath)
{
	PROFILE_FUNC();

	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	CvPlot* pBestExplorePlot = NULL;
	
	CvUnit* pTransportUnit = getTransportUnit();
	if (pTransportUnit == NULL)
	{
		FAssertMsg(false, "No transport unit???");
		return false;
	}
	
	int iRange = iMaxPath * pTransportUnit->baseMoves();
	bool bTransportPath = false;
	for (int iX = -iRange; iX <= iRange; iX++)
	{
		for (int iY = -iRange; iY <= iRange; iY++)
		{
			CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iX, iY);
			if ((pLoopPlot != NULL) && !atPlot(pLoopPlot) && !pLoopPlot->isWater())
			{
				if (pLoopPlot->isRevealed(getTeam(), false))
				{
					int iValue = AI_explorePlotValue(pLoopPlot, false);
					
					CvArea* pArea = pLoopPlot->area();
					iValue += std::max(40, pArea->getNumUnrevealedTiles(getTeam())) * 50;
					if (pArea->getNumAIUnits(getOwnerINLINE(), UNITAI_SCOUT) == 0)
					{
						iValue *= 5;
					}
					
					if (iValue > 0)
					{
						if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_EXPLORE, getGroup(), 3) == 0)
						{
							bool bTransport = (plotDistance(getX_INLINE(), getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE()) > 1);
							int iPathTurns = 0;
							bool bValid;
							if (bTransport)
							{
								bValid = pTransportUnit->generatePath(pLoopPlot, 0, bTransportPath, &iPathTurns);
							}
							else
							{
								bValid = generatePath(pLoopPlot, 0, !bTransportPath, &iPathTurns);
							}

							if (bValid)
							{
								if (iPathTurns < iMaxPath)
								{
									iValue *= 100;
									iValue /= 100 + getPathCost();
									
									if (iValue > iBestValue)
									{
										iBestValue = iValue;
										pBestExplorePlot = pLoopPlot;
										pBestPlot = bTransport ? pTransportUnit->getPathEndTurnPlot() : pLoopPlot;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	
	if (pBestPlot != NULL)
	{
		if (plotDistance(getX_INLINE(), getY_INLINE(), pBestExplorePlot->getX_INLINE(), pBestExplorePlot->getY_INLINE()) == 1)
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestExplorePlot->getX_INLINE(), pBestExplorePlot->getY_INLINE(), 0, false, false, MISSIONAI_EXPLORE, pBestExplorePlot);
			return true;
		}
		else
		{
			getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_EXPLORE, pBestExplorePlot);
			return true;
		}
	}
	return false;
}

bool CvUnitAI::AI_exploreCoast(int iRange)
{
	PROFILE_FUNC();

	CvPlot* pBestPlot = NULL;
	CvPlot* pBestExplorePlot = NULL;
	int iBestValue = 0;

	for (int iDX = -iRange; iDX <= iRange; iDX++)
	{
		for (int iDY = -iRange; iDY <= iRange; iDY++)
		{
			CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (AI_plotValid(pLoopPlot) && !atPlot(pLoopPlot) && AI_isValidExplore(pLoopPlot))
				{
					int iValue = 0;
					int iLandCount = 0;
					int iUnrevealedLandCount = 0;
					int iUnrevealedCount = 0;
					int iImpassableCount = 0;
					
					if (!pLoopPlot->isVisibleEnemyDefender(this))
					{
						for (int iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
						{
							CvPlot* pAdjacentPlot = plotDirection(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), ((DirectionTypes)iI));

							if (pAdjacentPlot != NULL)
							{
								if (!pAdjacentPlot->isRevealed(getTeam(), false))
								{
									iUnrevealedCount++;
									if (pLoopPlot->isRevealed(getTeam(), false))
									{
										if (!pLoopPlot->isWater())
										{
											iUnrevealedLandCount++;
										}
									}
								}
								if (!pAdjacentPlot->isWater())
								{
									iLandCount++;
								}
							}
						}
					}
					
					if ((iUnrevealedCount > 2) || (iUnrevealedLandCount > 0))
					{
						iValue += iLandCount * ((iUnrevealedCount == 1) ? 50 : 100);
					}
					
					if (iValue > 0)
					{
						iValue *= 1000;
						iValue += GC.getGame().getSorenRandNum(100, "AI explore");
						
						iValue *= 4;
						iValue /= 4 + pLoopPlot->getDistanceToOcean();
					}
					
					if (iValue > 0)
					{
						int iPathTurns;
						if (generatePath(pLoopPlot, MOVE_BUST_FOG, true, &iPathTurns))
						{
							if (iPathTurns <= 2)
							{
								//iValue /= 1 + stepDistance(iDX, iDY, 0, 0);
								iValue *= 10000;
								iValue /= 100 + getPathCost();
								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									pBestExplorePlot = pLoopPlot;
									pBestPlot = getGroup()->getPathFirstPlot();
								}
							}
						}
					}
				}
			}
		}
	}
	
	if (pBestPlot != NULL)
	{
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), MOVE_BUST_FOG, false, false, MISSIONAI_EXPLORE, pBestExplorePlot);
		return true;
	}

	return false;
}

bool CvUnitAI::AI_exploreOcean(int iRange)
{
	PROFILE_FUNC();

	CvPlot* pBestPlot = NULL;
	CvPlot* pBestExplorePlot = NULL;
	int iBestValue = 0;

	for (int iDX = -iRange; iDX <= iRange; iDX++)
	{
		for (int iDY = -iRange; iDY <= iRange; iDY++)
		{
			CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (AI_plotValid(pLoopPlot) && !atPlot(pLoopPlot) && AI_isValidExplore(pLoopPlot))
				{
					int iValue = 0;
					int iUnrevealedCount = 0;
					int iImpassableCount = 0;
					int iEuropeCount = 0;
					
					if (!pLoopPlot->isVisibleEnemyDefender(this))
					{
						if (pLoopPlot->isRevealed(getTeam(), true))
						{
							for (int iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
							{
								CvPlot* pAdjacentPlot = plotDirection(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), ((DirectionTypes)iI));

								if (pAdjacentPlot != NULL)
								{
									if (!pAdjacentPlot->isRevealed(getTeam(), false))
									{
										iUnrevealedCount++;
									}
									if (pLoopPlot->isEurope() && pAdjacentPlot->isEurope())
									{
										iEuropeCount++;
									}

									if (pAdjacentPlot->isImpassable())
									{
										iImpassableCount++;
									}
								}
							}
						}
					}
					
					if (iUnrevealedCount > 2)
					{
						iValue += iUnrevealedCount * 100;
						iValue /= 1 + iImpassableCount;
						iValue /= 1 + iEuropeCount;
					}
					
					if (GC.getGameINLINE().getGameTurn() < 10)
					{
						CvPlot* pStartingPlot = GET_PLAYER(getOwnerINLINE()).getStartingPlot();
						if (pStartingPlot != NULL)
						{
							if ((std::abs(pLoopPlot->getX_INLINE() - pStartingPlot->getX_INLINE()) <= 1) || (std::abs(pLoopPlot->getY_INLINE() - pStartingPlot->getY_INLINE()) <= 1))
							{
								iValue *= 2;
							}
							
						}
					}
					
					if (iValue > 0)
					{
						iValue *= 1000;
						iValue += GC.getGame().getSorenRandNum(100, "AI explore");
					}
					
					
					if (iValue > iBestValue)
					{
						if (generatePath(pLoopPlot, MOVE_BUST_FOG, true))
						{
							iBestValue = iValue;
							pBestExplorePlot = pLoopPlot;
							pBestPlot = getPathEndTurnPlot();
						}
					}
				}
			}
		}
	}
	
	if (pBestPlot != NULL)
	{
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), MOVE_BUST_FOG, false, false, MISSIONAI_EXPLORE, pBestExplorePlot);
		return true;
	}

	return false;
}

bool CvUnitAI::AI_exploreDeep()
{
	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	CvPlot* pBestExplorePlot = NULL;
	
	for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (AI_plotValid(pLoopPlot) && !atPlot(pLoopPlot) && AI_isValidExplore(pLoopPlot))
		{
			int iValue = 0;
			int iUnrevealedCount = 0;
			int iLandCount = 0;
			int iWaterCount = 0;
			int iImpassableCount = 0;
			int iEuropeCount = 0;
			if (pLoopPlot->isRevealed(getTeam(), false) && !pLoopPlot->isVisibleEnemyDefender(this))
			{
				for (int iJ = 0; iJ < NUM_DIRECTION_TYPES; iJ++)
				{
					PROFILE("AI_explore 2");

					CvPlot* pAdjacentPlot = plotDirection(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), ((DirectionTypes)iJ));
					
					if (pAdjacentPlot != NULL)
					{
						if (!(pAdjacentPlot->isRevealed(getTeam(), false)))
						{
							iUnrevealedCount++;
						}
						if (pAdjacentPlot->isWater())
						{
							iWaterCount++;
							if (pAdjacentPlot->isImpassable())
							{
								iImpassableCount++;
							}
							if (pAdjacentPlot->isEurope() && pLoopPlot->isEurope())
							{
								iEuropeCount++;
							}
						}
						else
						{
							iLandCount++;
						}
					}
				}
				
				if ((iUnrevealedCount > 2) || ((iUnrevealedCount > 0) && (iLandCount > 0)))
				{
					iValue += iLandCount * 100;
					iValue += iWaterCount * 30;
					
					if (iLandCount == 0)
					{
						iValue /= 1 + 2 * iImpassableCount;
					}
					iValue /= 1 + iEuropeCount;
				}

				if (iValue > 0)
				{
					if (!(pLoopPlot->isVisibleEnemyUnit(this)))
					{
						if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_EXPLORE, getGroup(), 3) == 0)
						{
							int iPathTurns;
							if (generatePath(pLoopPlot, MOVE_BUST_FOG, true, &iPathTurns))
							{
								iValue += GC.getGameINLINE().getSorenRandNum(10, "AI find land deep search");
								
								iValue *= 5;
								iValue /= 5 + pLoopPlot->getDistanceToOcean();
								
								iValue *= 10000;

								iValue /= 100 + getPathCost();

								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									pBestPlot = getPathEndTurnPlot();
									pBestExplorePlot = pLoopPlot;
								}
							}
						}
					}
				}
			}		
		}
	}

	if ((pBestPlot != NULL) && (pBestExplorePlot != NULL))
	{
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), MOVE_BUST_FOG, false, false, MISSIONAI_EXPLORE, pBestExplorePlot);
		return true;
	}

	return false;
}

bool CvUnitAI::AI_exploreOpenBorders(int iRange)
{
	PROFILE_FUNC();

	CvPlot* pBestPlot = NULL;
	CvPlot* pBestExplorePlot = NULL;
	int iBestValue = 0;
	CvTeamAI& kTeam = GET_TEAM(getTeam());
	for (int iDX = -iRange; iDX <= iRange; iDX++)
	{
		for (int iDY = -iRange; iDY <= iRange; iDY++)
		{
			CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (AI_plotValid(pLoopPlot) && !atPlot(pLoopPlot) && AI_isValidExplore(pLoopPlot))
				{
					if (!pLoopPlot->isVisibleEnemyDefender(this))
					{
						int iValue = 0;
						for (int iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
						{
							CvPlot* pAdjacentPlot = plotDirection(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), ((DirectionTypes)iI));

							if (pAdjacentPlot != NULL)
							{
								if (!pAdjacentPlot->isRevealed(getTeam(), false))
								{
									iValue += 100;
									if (pAdjacentPlot->isWater())
									{
										iValue += 50;
									}
									if (pAdjacentPlot->isRiver())
									{
										iValue += 50;
									}
									if (pAdjacentPlot->isOwned() && !isEnemy(pAdjacentPlot->getTeam()))
									{
										iValue += 200;
										if (kTeam.isOpenBorders(pAdjacentPlot->getTeam()))
										{
											iValue += 300;
										}
									}
								}
							}
						}

						if (iValue > 0)
						{
							iValue *= 100;
							iValue += GC.getGame().getSorenRandNum(100, "AI explore");
						}
						
						if (iValue > 0)
						{
							int iPathTurns;
							if (generatePath(pLoopPlot, MOVE_BUST_FOG | MOVE_NO_ENEMY_TERRITORY, true, &iPathTurns))
							{
								iValue *= 10000;
								iValue /= 100 + getPathCost();
								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									pBestExplorePlot = pLoopPlot;
									pBestPlot = getGroup()->getPathFirstPlot();
								}
							}
						}
					}
				}
			}
		}
	}
	
	if (pBestPlot != NULL)
	{
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), MOVE_BUST_FOG | MOVE_NO_ENEMY_TERRITORY, false, false, MISSIONAI_EXPLORE, pBestExplorePlot);
		return true;
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_targetCity(int iFlags)
{
	PROFILE_FUNC();

	CvCity* pTargetCity;
	CvCity* pLoopCity;
	CvCity* pBestCity;
	CvPlot* pAdjacentPlot;
	CvPlot* pBestPlot;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iLoop;
	int iI;

	iBestValue = 0;
	pBestCity = NULL;

	pTargetCity = area()->getTargetCity(getOwnerINLINE());

	if (pTargetCity != NULL)
	{
		if (AI_potentialEnemy(pTargetCity->getTeam(), pTargetCity->plot()))
		{
			if (!atPlot(pTargetCity->plot()) && generatePath(pTargetCity->plot(), iFlags, true))
			{
				pBestCity = pTargetCity;
			}
		}
	}

	if (pBestCity == NULL)
	{
		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
				{
					if (AI_plotValid(pLoopCity->plot()) && AI_potentialEnemy(GET_PLAYER((PlayerTypes)iI).getTeam(), pLoopCity->plot()))
					{
						if (!atPlot(pLoopCity->plot()) && generatePath(pLoopCity->plot(), iFlags, true, &iPathTurns))
						{
							iValue = 0;

							iValue = GET_PLAYER(getOwnerINLINE()).AI_targetCityValue(pLoopCity, false, false);

							iValue *= 1000;
							
							if ((area()->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE))
							{
								if (pLoopCity->calculateCulturePercent(getOwner()) < 75)
								{
									iValue /= 2;
								}
							}

							iValue /= (4 + iPathTurns*iPathTurns);

							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								pBestCity = pLoopCity;
							}
						}
					}
				}
			}
		}
	}

	if (pBestCity != NULL)
	{
		iBestValue = 0;
		pBestPlot = NULL;

		if (0 == (iFlags & MOVE_THROUGH_ENEMY))
		{
			for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
			{
				pAdjacentPlot = plotDirection(pBestCity->getX_INLINE(), pBestCity->getY_INLINE(), ((DirectionTypes)iI));

				if (pAdjacentPlot != NULL)
				{
					if (AI_plotValid(pAdjacentPlot))
					{
						if (!(pAdjacentPlot->isVisibleEnemyUnit(this)))
						{
							if (generatePath(pAdjacentPlot, iFlags, true, &iPathTurns))
							{
								iValue = std::max(0, (pAdjacentPlot->defenseModifier(getTeam()) + 100));

								if (!(pAdjacentPlot->isRiverCrossing(directionXY(pAdjacentPlot, pBestCity->plot()))))
								{
									iValue += (12 * -(GC.getRIVER_ATTACK_MODIFIER()));
								}

								if (!isEnemy(pAdjacentPlot->getTeam(), pAdjacentPlot))
								{
									iValue += 100;                                
								}

								iValue = std::max(1, iValue);

								iValue *= 1000;

								iValue /= (iPathTurns + 1);

								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									pBestPlot = getPathEndTurnPlot();
								}
							}
						}
					}
				}
			}
		}


		else
		{
			pBestPlot =  pBestCity->plot();
		}

		if (pBestPlot != NULL)
		{
			FAssert(!(pBestCity->at(pBestPlot)) || 0 != (iFlags & MOVE_THROUGH_ENEMY)); // no suicide missions...
			if (!atPlot(pBestPlot))
			{
				getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), iFlags);
				return true;
			}
		}
	}

	return false;
}

bool CvUnitAI::AI_targetCityNative(int iFlags)
{
	PROFILE_FUNC();


	//Native logic is pretty simple overall.
	//They move up to a city. Then hang around if extortion. If not extortion, they attack.

	CvCity* pTargetCity;
	CvCity* pLoopCity;
	CvCity* pBestCity;
	CvPlot* pBestPlot;
	CvPlot* pAdjacentPlot;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iLoop;
	int iI;

	iBestValue = 0;
	pBestCity = NULL;

	pTargetCity = area()->getTargetCity(getOwnerINLINE());

	if (pTargetCity != NULL)
	{
		if (AI_potentialEnemy(pTargetCity->getTeam(), pTargetCity->plot()))
		{
			if (!atPlot(pTargetCity->plot()) && generatePath(pTargetCity->plot(), iFlags, true))
			{
				pBestCity = pTargetCity;
			}
		}
	}

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
			{
				if (AI_plotValid(pLoopCity->plot()) && AI_potentialEnemy(GET_PLAYER((PlayerTypes)iI).getTeam(), pLoopCity->plot()))
				{
					if (!atPlot(pLoopCity->plot()) && generatePath(pLoopCity->plot(), iFlags, true, &iPathTurns))
					{
						iValue = 0;

						iValue = GET_PLAYER(getOwnerINLINE()).AI_targetCityValue(pLoopCity, false, false);

						iValue *= 1000;
						
						if (pLoopCity == pBestCity)
						{
							iValue *= 2;
						}
						
						iValue /= (4 + iPathTurns*iPathTurns);

						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							pBestCity = pLoopCity;
						}
					}
				}
			}
		}
	}

	if (pBestCity != NULL)
	{
		iBestValue = 0;
		pBestPlot = NULL;

		if (0 == (iFlags & MOVE_THROUGH_ENEMY))
		{
			for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
			{
				pAdjacentPlot = plotDirection(pBestCity->getX_INLINE(), pBestCity->getY_INLINE(), ((DirectionTypes)iI));

				if (pAdjacentPlot != NULL)
				{
					if (AI_plotValid(pAdjacentPlot))
					{
						if (!(pAdjacentPlot->isVisibleEnemyUnit(this)))
						{
							if (generatePath(pAdjacentPlot, iFlags, true, &iPathTurns))
							{
								iValue = std::max(0, (pAdjacentPlot->defenseModifier(getTeam()) + 100));

								if (!(pAdjacentPlot->isRiverCrossing(directionXY(pAdjacentPlot, pBestCity->plot()))))
								{
									iValue += (12 * -(GC.getRIVER_ATTACK_MODIFIER()));
								}

								if (!isEnemy(pAdjacentPlot->getTeam(), pAdjacentPlot))
								{
									iValue += 100;                                
								}

								iValue = std::max(1, iValue);

								iValue *= 1000;

								iValue /= (iPathTurns + 1);

								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									pBestPlot = getPathEndTurnPlot();
								}
							}
						}
					}
				}
			}
		}
		else
		{
			pBestPlot =  pBestCity->plot();
		}

		if (pBestPlot != NULL)
		{
			FAssert(!(pBestCity->at(pBestPlot)) || 0 != (iFlags & MOVE_THROUGH_ENEMY)); // no suicide missions...
			if (!atPlot(pBestPlot))
			{
				getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), iFlags);
				return true;
			}
		}
	}

	return false;
}

bool CvUnitAI::AI_extortCity()
{
	int iBestValue = 0;
	CvCity* pBestCity = NULL;
	
	for (int iDirection = 0; iDirection < NUM_DIRECTION_TYPES; ++iDirection)
	{
		CvPlot* pLoopPlot = plotDirection(getX_INLINE(), getY_INLINE(), (DirectionTypes)iDirection);
		if (pLoopPlot != NULL)
		{
			if (pLoopPlot->isCity())
			{
				if (AI_potentialEnemy(pLoopPlot->getTeam(), pLoopPlot))
				{
					CvTeamAI& kPlotTeam = GET_TEAM(pLoopPlot->getTeam());
					
					if (GET_TEAM(getTeam()).AI_getWarPlan(pLoopPlot->getTeam()) == WARPLAN_EXTORTION)
					{
						int iValue = GC.getGame().getSorenRandNum(100, "AI choose extort city");
						
						if (iValue > iBestValue)
						{
							iValue = iBestValue;
							pBestCity = pLoopPlot->getPlotCity();
						}
					}					
				}
			}
		}
	}
	
	if (pBestCity != NULL)
	{
		getGroup()->pushMission(MISSION_SKIP);
		return true;
	}
	return false;
}

// Returns true if a mission was pushed...
bool CvUnitAI::AI_bombardCity()
{
	CvCity* pBombardCity;

	if (canBombard(plot()))
	{
		pBombardCity = bombardTarget(plot());
		FAssertMsg(pBombardCity != NULL, "BombardCity is not assigned a valid value");

		// do not bombard cities with no defenders
		int iDefenderStrength = pBombardCity->plot()->AI_sumStrength(NO_PLAYER, getOwnerINLINE(), DOMAIN_LAND, /*bDefensiveBonuses*/ true, /*bTestAtWar*/ true, false);
		if (iDefenderStrength == 0)
		{
			return false;
		}
		
		// do not bombard cities if we have overwhelming odds
		int iAttackOdds = getGroup()->AI_attackOdds(pBombardCity->plot(), /*bPotentialEnemy*/ true);
		if (iAttackOdds > 95)
		{
			return false;
		}
		
		// could also do a compare stacks call here if we wanted, the downside of that is that we may just have a lot more units
		// we may not want to suffer high casualties just to save a turn
		//getGroup()->AI_compareStacks(pBombardCity->plot(), /*bPotentialEnemy*/ true, /*bCheckCanAttack*/ true, /*bCheckCanMove*/ true);
		//int iOurStrength = pBombardCity->plot()->AI_sumStrength(getOwnerINLINE(), NO_PLAYER, DOMAIN_LAND, false, false, false)
		
		if (pBombardCity->getDefenseDamage() < ((GC.getMAX_CITY_DEFENSE_DAMAGE() * 3) / 4))
		{
			getGroup()->pushMission(MISSION_BOMBARD);
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_cityAttack(int iRange, int iOddsThreshold, bool bFollow)
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	int iSearchRange;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iDX, iDY;

	FAssert(canMove());

	if (bFollow || isCargo())
	{
		iSearchRange = 1;
	}
	else
	{
		iSearchRange = AI_searchRange(iRange);
	}

	iBestValue = 0;
	pBestPlot = NULL;

	for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			pLoopPlot	= plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (AI_plotValid(pLoopPlot))
				{
					if (pLoopPlot->isCity() || (pLoopPlot->isCity(true, getTeam()) && pLoopPlot->isVisibleEnemyUnit(this)))
					{
						if (AI_potentialEnemy(pLoopPlot->getTeam(), pLoopPlot) && (GET_TEAM(getTeam()).AI_getWarPlan(pLoopPlot->getTeam()) != WARPLAN_EXTORTION))
						{
							if (!atPlot(pLoopPlot) && ((bFollow) ? canMoveInto(pLoopPlot, true) : (generatePath(pLoopPlot, 0, true, &iPathTurns) && (iPathTurns <= iRange))))
							{
								iValue = getGroup()->AI_attackOdds(pLoopPlot, true);

								if (iValue >= AI_finalOddsThreshold(pLoopPlot, iOddsThreshold))
								{
									if (iValue > iBestValue)
									{
										iBestValue = iValue;
										pBestPlot = ((bFollow) ? pLoopPlot : getPathEndTurnPlot());
										FAssert(!atPlot(pBestPlot));
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), ((bFollow) ? MOVE_DIRECT_ATTACK : 0));
		return true;
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_anyAttack(int iRange, int iOddsThreshold, int iMinStack, bool bFollow)
{
	PROFILE_FUNC();

	FAssert(canMove());

	int iSearchRange = bFollow ? 1 : AI_searchRange(iRange);

	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	CvPlot* pBombardPlot = NULL;
	int iBestBombardValue = 0;

	for (int iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (int iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			CvPlot* pLoopPlot = ::plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (AI_plotValid(pLoopPlot) && pLoopPlot->isVisible(getTeam(), false))
				{
					bool bCanBombard = canBombard(pLoopPlot);
					if (bCanBombard || pLoopPlot->isVisibleEnemyUnit(this) || (pLoopPlot->isCity() && AI_potentialEnemy(pLoopPlot->getTeam(), pLoopPlot) && (GET_TEAM(getTeam()).AI_getWarPlan(pLoopPlot->getTeam()) != WARPLAN_EXTORTION)))
					{
						int iPathTurns;
						if (!atPlot(pLoopPlot) && ((bFollow) ? canMoveInto(pLoopPlot, true) : (generatePath(pLoopPlot, 0, true, &iPathTurns) && (iPathTurns <= iRange))))
						{
							if (pLoopPlot->getNumVisibleEnemyDefenders(this) >= iMinStack)
							{
								int iValue = getGroup()->AI_attackOdds(pLoopPlot, true);

								if (iValue >= AI_finalOddsThreshold(pLoopPlot, iOddsThreshold))
								{
									if (iValue > iBestValue)
									{
										iBestValue = iValue;
										pBestPlot = ((bFollow) ? pLoopPlot : getPathEndTurnPlot());
										FAssert(!atPlot(pBestPlot));
									}
								}

							}

							if (bCanBombard)
							{
								CvCity* pCity = bombardTarget(pLoopPlot);
								FAssert(pCity != NULL);
								if (pCity != NULL)
								{
									int iValue = getGroup()->AI_attackOdds(pCity->plot(), true);
									if (iValue > iBestBombardValue)
									{
										iBestBombardValue = iValue;
										pBombardPlot = ((bFollow) ? pLoopPlot : getPathEndTurnPlot());
										FAssert(!atPlot(pBombardPlot));
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), ((bFollow) ? MOVE_DIRECT_ATTACK : 0));
		return true;
	}
	else if (AI_bombardCity())
	{
		return true;
	}
	else if (pBombardPlot != NULL)
	{
		FAssert(!atPlot(pBombardPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBombardPlot->getX_INLINE(), pBombardPlot->getY_INLINE(), ((bFollow) ? MOVE_DIRECT_ATTACK : 0));
		return true;
	}

	return false;
}

//Attack only if the odds fall between two thresholds.
//This is to prevent "baiting" and is intended to be used with multiple passes.
bool CvUnitAI::AI_smartAttack(int iRange, int iLowOddsThreshold, int iHighOddsThreshold, CvPlot* pHintPlot)
{
    PROFILE_FUNC();

	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	int iSearchRange;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iDX, iDY;

	FAssert(canMove());

	iSearchRange = isCargo() ? 1 : AI_searchRange(iRange);

	iBestValue = 0;
	pBestPlot = NULL;

	for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			pLoopPlot	= plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (canMoveOrAttackInto(pLoopPlot, true))
				{
					if (pLoopPlot->isVisibleEnemyUnit(this) || (pLoopPlot->isCity() && AI_potentialEnemy(pLoopPlot->getTeam(), pLoopPlot) && (GET_TEAM(getTeam()).AI_getWarPlan(pLoopPlot->getTeam()) != WARPLAN_EXTORTION)))
					{
						if (!atPlot(pLoopPlot) && (generatePath(pLoopPlot, 0, true, &iPathTurns) && (iPathTurns <= iRange)))
						{
							{
								iValue = getGroup()->AI_attackOdds(pLoopPlot, true);
								
								if (iValue >= iLowOddsThreshold)
								{
								    if (pLoopPlot->isCity() ||  (iValue <= iHighOddsThreshold))
                                    {
                                        if (pHintPlot == pLoopPlot)
                                        {
                                            iValue *= 3;
                                            iValue /= 2;
                                        }
                                        if (iValue > iBestValue)
                                        {
                                            iBestValue = iValue;
                                            pBestPlot =  getPathEndTurnPlot();
                                            FAssert(!atPlot(pBestPlot));
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

	if (pBestPlot != NULL)
	{
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0);
		return true;
	}
    return false;
}

// Returns true if a mission was pushed...
bool CvUnitAI::AI_leaveAttack(int iRange, int iOddsThreshold, int iStrengthThreshold)
{
	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	CvCity* pCity;
	int iSearchRange;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iDX, iDY;

	FAssert(canMove());

	iSearchRange = iRange;
	
	iBestValue = 0;
	pBestPlot = NULL;
	
	
	pCity = plot()->getPlotCity();
	
	if ((pCity != NULL) && (pCity->getOwner() == getOwner()))
	{
		int iOurStrength = GET_PLAYER(getOwnerINLINE()).AI_getOurPlotStrength(plot(), 0, false, false);
    	int iEnemyStrength = GET_PLAYER(getOwnerINLINE()).AI_getEnemyPlotStrength(plot(), 2, false, false);
		if (iEnemyStrength > 0)
		{
    		if (((iOurStrength * 100) / iEnemyStrength) < iStrengthThreshold)
    		{
    			return false;    		    		
    		}
    		if (plot()->plotCount(PUF_canDefendGroupHead, -1, -1, getOwnerINLINE()) <= getGroup()->getNumUnits())
    		{
    			return false;    		
    		}
		}
	}

	for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (AI_plotValid(pLoopPlot))
				{
					if (pLoopPlot->isVisibleEnemyUnit(this) || (pLoopPlot->isCity() && AI_potentialEnemy(pLoopPlot->getTeam(), pLoopPlot)))
					{
						if (!atPlot(pLoopPlot) && (generatePath(pLoopPlot, 0, true, &iPathTurns) && (iPathTurns <= iRange)))
						{
							//if (pLoopPlot->getNumVisibleEnemyDefenders(this) > 0)
							{
								iValue = getGroup()->AI_attackOdds(pLoopPlot, true);

								if (iValue >= AI_finalOddsThreshold(pLoopPlot, iOddsThreshold))
								{
									if (iValue > iBestValue)
									{
										iBestValue = iValue;
										pBestPlot = getPathEndTurnPlot();
										FAssert(!atPlot(pBestPlot));
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0);
		return true;
	}

	return false;
	
}

// Returns true if a mission was pushed...
bool CvUnitAI::AI_seaBombardRange(int iMaxRange)
{
	PROFILE_FUNC();

	// cached values
	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());
	CvPlot* pPlot = plot();
	CvSelectionGroup* pGroup = getGroup();

	// can any unit in this group bombard?
	bool bHasBombardUnit = false;
	bool bBombardUnitCanBombardNow = false;
	CLLNode<IDInfo>* pUnitNode = pGroup->headUnitNode();
	while (pUnitNode != NULL && !bBombardUnitCanBombardNow)
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pGroup->nextUnitNode(pUnitNode);

		if (pLoopUnit->bombardRate() > 0)
		{
			bHasBombardUnit = true;

			if (pLoopUnit->canMove() && !pLoopUnit->isMadeAttack())
			{
				bBombardUnitCanBombardNow = true;
			}
		}
	}

	if (!bHasBombardUnit)
	{
		return false;
	}

	// best match
	CvPlot* pBestPlot = NULL;
	CvPlot* pBestBombardPlot = NULL;
	int iBestValue = 0;

	// iterate over plots at each range
	for (int iDX = -(iMaxRange); iDX <= iMaxRange; iDX++)
	{
		for (int iDY = -(iMaxRange); iDY <= iMaxRange; iDY++)
		{
			CvPlot* pLoopPlot = plotXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL && AI_plotValid(pLoopPlot))
			{
				CvCity* pBombardCity = bombardTarget(pLoopPlot);

				if (pBombardCity != NULL && isEnemy(pBombardCity->getTeam(), pLoopPlot) && pBombardCity->getDefenseDamage() < GC.getMAX_CITY_DEFENSE_DAMAGE())
				{
					int iPathTurns;
					if (generatePath(pLoopPlot, 0, true, &iPathTurns))
					{
						int iValue = 1;

						iValue += (kPlayer.AI_plotTargetMissionAIs(pBombardCity->plot(), MISSIONAI_ASSAULT, NULL, 2) * 3);
						iValue += (kPlayer.AI_adjacentPotentialAttackers(pBombardCity->plot(), true));

						if (iValue > 0)
						{
							iValue *= 1000;

							iValue /= getPathCost() + 50;

							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								pBestPlot = getPathEndTurnPlot();
								pBestBombardPlot = pLoopPlot;
							}
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestBombardPlot != NULL))
	{
		if (atPlot(pBestBombardPlot))
		{
			// if we are at the plot from which to bombard, and we have a unit that can bombard this turn, do it
			if (bBombardUnitCanBombardNow && pGroup->canBombard(pBestBombardPlot))
			{
				getGroup()->pushMission(MISSION_BOMBARD, -1, -1, 0, false, false, MISSIONAI_BOMBARD, pBestBombardPlot);
			}
			// otherwise, skip until next turn, when we will surely bombard
			else
			{
				getGroup()->pushMission(MISSION_SKIP);
			}

			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_BOMBARD, pBestBombardPlot);
			return true;
		}
	}

	return false;
}

bool CvUnitAI::AI_blockade(int iRange)
{
	PROFILE_FUNC();

	int iMaxRange = iRange;

	// best match
	CvPlot* pBestPlot = NULL;
	int iBestValue = 0;

	// iterate over plots at each range
	for (int iDX = -(iMaxRange); iDX <= iMaxRange; iDX++)
	{
		for (int iDY = -(iMaxRange); iDY <= iMaxRange; iDY++)
		{
			CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL && AI_plotValid(pLoopPlot))
			{
				CvCity* pBlockadeCity = pLoopPlot->getWorkingCity();
				if (pBlockadeCity != NULL && isEnemy(pBlockadeCity->getTeam(), pLoopPlot) && pBlockadeCity->getDefenseDamage() < GC.getMAX_CITY_DEFENSE_DAMAGE())
				{
					int iPathTurns;
					if (atPlot(pLoopPlot) || generatePath(pLoopPlot, 0, true, &iPathTurns))
					{
						int iValue = atPlot(pLoopPlot) ? 1200 : 1000;
						if (pLoopPlot->getBonusType() != NO_BONUS)
						{
							iValue += 1000;
						}
						iValue /= 1 + pLoopPlot->plotCount(PUF_canDefend, -1, -1, getOwnerINLINE());
						iValue += GC.getGameINLINE().getSorenRandNum(100, "AI blockade plot");
						
						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							pBestPlot = getPathEndTurnPlot();
						}
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		if (atPlot(pBestPlot))
		{
			if (canBombard(pBestPlot))
			{
				getGroup()->pushMission(MISSION_BOMBARD);
			}
			else
			{
				getGroup()->pushMission(MISSION_SKIP);
			}
			return true;
		}
		else
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
			return true;
		}
	}

	return false;	
}



// Returns true if a mission was pushed...
bool CvUnitAI::AI_pillage()
{
	PROFILE_FUNC();

	if (canPillage(plot()) && AI_canPillage(*plot()))
	{
		pillage();
		return true;
	}

	CvPlot* pBestPillagePlot = NULL;
	int iPathTurns;
	int iValue;
	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;

	for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (AI_plotValid(pLoopPlot))
		{
			if (potentialWarAction(pLoopPlot))
			{
			    CvCity * pWorkingCity = pLoopPlot->getWorkingCity();

			    if (pWorkingCity != NULL)
			    {
                    if (!(pWorkingCity == area()->getTargetCity(getOwnerINLINE())) && canPillage(pLoopPlot))
                    {
                        if (!(pLoopPlot->isVisibleEnemyUnit(this)))
                        {
                            if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_PILLAGE, getGroup(), 1) == 0)
                            {
                                if (generatePath(pLoopPlot, 0, true, &iPathTurns))
                                {
                                    iValue = AI_pillageValue(pLoopPlot);

                                    iValue *= 1000;

                                    iValue /= (iPathTurns + 1);

									// if not at war with this plot owner, then devalue plot if we already inside this owner's borders
									// (because declaring war will pop us some unknown distance away)
									if (!isEnemy(pLoopPlot->getTeam()) && plot()->getTeam() == pLoopPlot->getTeam())
									{
										iValue /= 10;
									}

                                    if (iValue > iBestValue)
                                    {
                                        iBestValue = iValue;
                                        pBestPlot = getPathEndTurnPlot();
                                        pBestPillagePlot = pLoopPlot;
                                    }
                                }
                            }
                        }
                    }
			    }
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestPillagePlot != NULL))
	{
		if (atPlot(pBestPillagePlot) && !isEnemy(pBestPillagePlot->getTeam()))
		{
			//getGroup()->groupDeclareWar(pBestPillagePlot, true);
			// rather than declare war, just find something else to do, since we may already be deep in enemy territory
			return false;
		}
		
		if (atPlot(pBestPillagePlot))
		{
			if (isEnemy(pBestPillagePlot->getTeam()))
			{
				getGroup()->pushMission(MISSION_PILLAGE, -1, -1, 0, false, false, MISSIONAI_PILLAGE, pBestPillagePlot);
				return true;
			}
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_PILLAGE, pBestPillagePlot);
			return true;
		}
	}

	return false;
}

bool CvUnitAI::AI_canPillage(CvPlot& kPlot) const
{
	if (isEnemy(kPlot.getTeam(), &kPlot))
	{
		return true;
	}

	if (!kPlot.isOwned())
	{
		return true;
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_pillageRange(int iRange, bool bSafe)
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	CvPlot* pBestPillagePlot;
	int iSearchRange;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iDX, iDY;

	if (canPillage(plot()) && AI_canPillage(*plot()))
	{
		pillage();
		return true;
	}

	iSearchRange = AI_searchRange(iRange);

	iBestValue = 0;
	pBestPlot = NULL;
	pBestPillagePlot = NULL;

	for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (AI_plotValid(pLoopPlot))
				{
					if ((potentialWarAction(pLoopPlot)) || (pLoopPlot->getCrumbs() > 0))
					{
                        CvCity * pWorkingCity = pLoopPlot->getWorkingCity();

                        if (canPillage(pLoopPlot))
                        {
                            if (GET_PLAYER(getOwnerINLINE()).isNative() || ((pWorkingCity != NULL) && !(pWorkingCity == area()->getTargetCity(getOwnerINLINE()))))
                            {
                                if (!(pLoopPlot->isVisibleEnemyUnit(this)))
                                {
                                    if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_PILLAGE, getGroup()) == 0)
                                    {
                                        if (generatePath(pLoopPlot, 0, true, &iPathTurns))
                                        {
                                        	bool bDanger = false;
                                            if (getPathLastNode()->m_iData1 == 0)
                                            {
                                            	bDanger = true;
                                                iPathTurns++;
                                            }
                                            
                                            if (bDanger)
                                            {
                                            	if (pLoopPlot->defenseModifier(getTeam()) > 0 && !noDefensiveBonus())
                                            	{
                                            		bDanger = false;
                                            	}
                                            }

                                            if ((!bDanger || !bSafe) && (iPathTurns <= iRange))
                                            {
                                                iValue = AI_pillageValue(pLoopPlot);

                                                iValue *= 1000;

                                                iValue /= (iPathTurns + 1);

												// if not at war with this plot owner, then devalue plot if we already inside this owner's borders
												// (because declaring war will pop us some unknown distance away)
												if (pLoopPlot->isOwned() && !isEnemy(pLoopPlot->getTeam()) && plot()->getTeam() == pLoopPlot->getTeam())
												{
													iValue /= 10;
												}

                                                if (iValue > iBestValue)
                                                {
                                                    iBestValue = iValue;
                                                    pBestPlot = getPathEndTurnPlot();
                                                    pBestPillagePlot = pLoopPlot;
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

	if ((pBestPlot != NULL) && (pBestPillagePlot != NULL))
	{
		if (atPlot(pBestPillagePlot) && !isEnemy(pBestPillagePlot->getTeam()))
		{
			//getGroup()->groupDeclareWar(pBestPillagePlot, true);
			// rather than declare war, just find something else to do, since we may already be deep in enemy territory
			return false;
		}
		
		if (atPlot(pBestPillagePlot))
		{
			if (isNative() || isEnemy(pBestPillagePlot->getTeam()))
			{
				getGroup()->pushMission(MISSION_PILLAGE, -1, -1, 0, false, false, MISSIONAI_PILLAGE, pBestPillagePlot);
				return true;
			}
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_PILLAGE, pBestPillagePlot);
			return true;
		}
	}

	return false;
}

bool CvUnitAI::AI_maraud(int iRange)
{
	PROFILE_FUNC();
	//The basic goal of the maraud function, is to penetrate deeper into hostile
	//territory, but staying away from cities.
	
	int iSearchRange = iRange;//AI_searchRange(iRange);

	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	CvPlot* pBestMaraudPlot = NULL;
	
	CvCity* pNearestCity = GC.getMapINLINE().findCity(getX_INLINE(), getY_INLINE(), getOwnerINLINE(), NO_TEAM, true);
	
	if (pNearestCity == NULL)
	{
		return false;
	}
	
	int iCurrentDist = stepDistance(getX_INLINE(), getY_INLINE(), pNearestCity->getX_INLINE(), pNearestCity->getY_INLINE());

	for (int iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (int iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);	
			
			if ((pLoopPlot != NULL) && !atPlot(pLoopPlot))
			{
				if (AI_potentialEnemy(pLoopPlot->getTeam(), pLoopPlot))
				{
					int iPathTurns;
					if (generatePath(pLoopPlot, 0, true, &iPathTurns))
					{
						int iNewDist = stepDistance(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), pNearestCity->getX_INLINE(), pNearestCity->getY_INLINE());
						
						int iValue = 200 + GC.getGame().getSorenRandNum((iNewDist - iCurrentDist) * 100, "AI maraud");
						
						if (canPillage(pLoopPlot))
						{
							iValue += 100;
						}

						for (int iDirection = 0; iDirection < NUM_DIRECTION_TYPES; ++iDirection)
						{
							CvPlot* pDirectionPlot = plotDirection(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), (DirectionTypes)iDirection);
							if (pDirectionPlot != NULL)
							{
								if (pDirectionPlot->isOwned() && AI_potentialEnemy(pDirectionPlot->getTeam(), pDirectionPlot))
								{
									if (pDirectionPlot->isCity())
									{
										iValue -= 200;
									}
									else if (pDirectionPlot->getRouteType() != NO_ROUTE)
									{
										iValue -= 25;
									}
								}
							}
						}
						
						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							pBestMaraudPlot = pLoopPlot;
							pBestPlot = getPathEndTurnPlot();
						}
					}
				}
			}
		}
	}
	
	if (pBestPlot != NULL)
	{
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_PILLAGE, pBestMaraudPlot);
		return true;
	}
	
	return false;
}

bool CvUnitAI::AI_hostileShuffle()
{
	int iBestAdjacentPlotValue = 0;
	CvPlot* pBestAdjacentPlot = NULL;
	for (int iDirection = 0; iDirection < NUM_DIRECTION_TYPES; ++iDirection)
	{
		CvPlot* pDirectionPlot = plotDirection(getX_INLINE(), getY_INLINE(), (DirectionTypes)iDirection);
		if (pDirectionPlot != NULL)
		{
			if (isEnemy(pDirectionPlot->getTeam(), pDirectionPlot))
			{
				if (pDirectionPlot->isCity())
				{
					int iValue =  GET_PLAYER(getOwnerINLINE()).AI_targetCityValue(pDirectionPlot->getPlotCity(), false, false);
					if (iValue > iBestAdjacentPlotValue)
					{
						pBestAdjacentPlot = pDirectionPlot;
						iBestAdjacentPlotValue = iValue;
					}
				}
			}
		}
	}
	if (pBestAdjacentPlot == NULL)
	{
		return false;
	}
	
	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	for (int iX = -1; iX < 1; ++iX)
	{
		for (int iY = -1; iY <= 1; ++iY)
		{
			CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iX, iY);
			if (pLoopPlot != NULL)
			{
				if (stepDistance(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), pBestAdjacentPlot->getX_INLINE(), pBestAdjacentPlot->getY_INLINE()) <= 1)
				{
					if (atPlot(pLoopPlot) || canMoveInto(pLoopPlot))
					{
						int iValue = pLoopPlot->defenseModifier(getTeam());
						if (!pLoopPlot->isRiverCrossing(directionXY(pLoopPlot, pBestAdjacentPlot)))
						{
							iValue += 50;
						}
						if (pLoopPlot->isBeingWorked())
						{
							iValue += 25;
						}
						iValue += GC.getGameINLINE().getSorenRandNum(75, "AI hostile shuffle");						
						
						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							pBestPlot = pLoopPlot;
						}
					}
				}
			}
		}
	}
	
	if (pBestPlot != NULL)
	{
		if (atPlot(pBestPlot))
		{
			getGroup()->pushMission(MISSION_SKIP);
		}
		else
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
		}
		return true;
	}
	return false;
}

bool CvUnitAI::AI_wanderAroundAimlessly()
{
	//I dedicate this function to all seekers of anything in life.
	
	CvPlot* pMissionPlot = getGroup()->AI_getMissionAIPlot();
	
	CvMap& kMap = GC.getMap();
	while ((pMissionPlot == NULL) || atPlot(pMissionPlot))
	{
		pMissionPlot = kMap.plot(GC.getGameINLINE().getSorenRandNum(kMap.getGridWidthINLINE(), "AI wander X"), GC.getGameINLINE().getSorenRandNum(kMap.getGridWidthINLINE(), "AI wander Y"));
	}
	
	int iCurrentDistance = plotDistance(getX_INLINE(), getY_INLINE(), pMissionPlot->getX_INLINE(), pMissionPlot->getY_INLINE());
		
	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	if (pMissionPlot != NULL)
	{
		for (int i = 0; i < NUM_DIRECTION_TYPES; ++i)
		{
			CvPlot* pLoopPlot = plotDirection(getX_INLINE(), getY_INLINE(), (DirectionTypes)i);
			if (pLoopPlot != NULL)
			{
				int iDistance = plotDistance(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), pMissionPlot->getX_INLINE(), pMissionPlot->getY_INLINE());
				if (iDistance < iCurrentDistance)
				{
					if (AI_plotValid(pLoopPlot))
					{
						int iValue = GC.getGameINLINE().getSorenRandNum(100, "AI wander aimlessly");
						
						for (int j = 0; j < NUM_DIRECTION_TYPES; ++j)
						{
							CvPlot* pDirectionPlot = plotDirection(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), (DirectionTypes)i);
							if (pDirectionPlot != NULL)
							{
								if (!pDirectionPlot->isVisible(getTeam(), false))
								{
									iValue += 5;
								}
							}
						}
						
						if ((iValue > iBestValue) && generatePath(pLoopPlot))
						{
							iBestValue = iValue;
							pBestPlot = pLoopPlot;
						}
					}
				}
			}
		}
	}
	
	if (pBestPlot == NULL)
	{
		return false;
	}
	
	getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_EXPLORE, pMissionPlot);
	return true;
}

int CvUnitAI::AI_foundValue(CvPlot* pPlot)
{
	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());

	int iValue = 0;
	if ((kOwner.getNumCities() == 0) && (GC.getGameINLINE().getGameTurn() < 20))
	{
		iValue = kOwner.AI_foundValue(pPlot->getX_INLINE(), pPlot->getY_INLINE());
	}
	else
	{
		iValue = pPlot->getFoundValue(getOwnerINLINE());
	}
	
	ProfessionTypes eProfession = AI_getIdealProfession();
	if (eProfession != NO_PROFESSION)
	{
		CvProfessionInfo& kProfession = GC.getProfessionInfo(eProfession);
		if (kProfession.isWorkPlot())
		{
			YieldTypes eYieldProduced = (YieldTypes)kProfession.getYieldProduced();
			FAssert(eYieldProduced != NO_YIELD);
			
			if (eYieldProduced != NO_YIELD)
			{
			
				bool bHasBonus = false;
				int iBestAmount = 0;
				
				for (int iI = 0; iI < NUM_CITY_PLOTS; ++iI)
				{
					if (iI != CITY_HOME_PLOT)
					{
						CvPlot* pLoopPlot = plotCity(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iI);
						if (pLoopPlot != NULL && (!pLoopPlot->isOwned() || GET_PLAYER(pLoopPlot->getOwnerINLINE()).isNative()))
						{
							iBestAmount = std::max(iBestAmount, pLoopPlot->calculateBestNatureYield(eYieldProduced, getTeam()));
							if (pLoopPlot->getBonusType() != NO_BONUS)
							{
								if (GC.getBonusInfo(pLoopPlot->getBonusType()).getYieldChange(eYieldProduced) > 0)
								{
									bHasBonus = true;
								}
							}
						}
					}
				}
				
				if (iBestAmount > 0)
				{
					iValue += 20 * kOwner.AI_yieldValue(eYieldProduced, true, iBestAmount + 1);
					if (bHasBonus)
					{
						iValue *= 4;
						iValue /= 3;
					}
				}
				else
				{
					iValue *= 2;
					iValue /= 3;
				}
			}
		}
	}
	
	
	return iValue;	
}

// Returns true if a mission was pushed...
bool CvUnitAI::AI_found(int iMinValue)
{
	PROFILE_FUNC();
	
	if (!canFound(NULL))
	{
		return false;
	}
	
	CvUnit* pTransportUnit = getTransportUnit();
	
	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());

	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	CvPlot* pBestFoundPlot = NULL;
	bool bBestIsTransport = false;
	bool bTransportPath = false;
	
	for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);
		if (pLoopPlot->isRevealed(getTeam(), false) && ((pTransportUnit != NULL) || AI_plotValid(pLoopPlot)))
		{
			if (canFound(pLoopPlot))
			{
				int iValue = AI_foundValue(pLoopPlot);

				if (iValue > iMinValue)
				{
					if (!(pLoopPlot->isVisibleEnemyUnit(this)))
					{
						if (kOwner.AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_FOUND, getGroup(), 0) == 0)
						{
							int iPathTurns = 0;
							bool bValid = atPlot(pLoopPlot);
							bool bTransport = false;

							if (!bValid)
							{
								bTransport = pTransportUnit != NULL;
								if (bTransport && canMove())
								{
									if (stepDistance(getX_INLINE(), getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE()) <= 1)
									{
										bTransport = false;
									}
								}
								if (bTransport)
								{
									bValid = pTransportUnit->generatePath(pLoopPlot, 0, bTransportPath, &iPathTurns);
								}
								else
								{
									bValid = generatePath(pLoopPlot, 0, !bTransportPath, &iPathTurns);
								}
								bTransportPath = bTransport;
							}

							if (bValid)
							{
								iValue *= 10;
								iValue /= 5 + iPathTurns;
								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									pBestPlot = atPlot(pLoopPlot) ? pLoopPlot : (bTransport ? pTransportUnit->getPathEndTurnPlot() : getPathEndTurnPlot());
									pBestFoundPlot = pLoopPlot;
									bBestIsTransport = bTransport;
								}
							}
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestFoundPlot != NULL))
	{
		if (bBestIsTransport)
		{
			getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_FOUND, pBestFoundPlot);
			return true;
		}
		else
		{
			if (atPlot(pBestFoundPlot))
			{
				getGroup()->pushMission(MISSION_FOUND, -1, -1, 0, false, false, MISSIONAI_FOUND, pBestFoundPlot);
				return true;
			}
			else
			{
				FAssert(!atPlot(pBestPlot));
				getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_FOUND, pBestFoundPlot);
				return true;
			}
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_foundRange(int iRange, bool bFollow)
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	CvPlot* pBestFoundPlot;
	int iSearchRange;
	int iPathTurns;
	int iBestValue;
	int iDX, iDY;

	iSearchRange = AI_searchRange(iRange);

	iBestValue = 0;
	pBestPlot = NULL;
	pBestFoundPlot = NULL;

	for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			pLoopPlot	= plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (AI_plotValid(pLoopPlot) && (pLoopPlot != plot() || GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(pLoopPlot, 1) <= pLoopPlot->plotCount(PUF_canDefend, -1, -1, getOwnerINLINE())))
				{
					if (canFound(pLoopPlot))
					{
						int iValue = AI_foundValue(pLoopPlot);

						if (iValue > iBestValue)
						{
							if (!(pLoopPlot->isVisibleEnemyUnit(this)))
							{
								if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_FOUND, getGroup(), 3) == 0)
								{
									if (generatePath(pLoopPlot, MOVE_SAFE_TERRITORY, true, &iPathTurns))
									{
										if (iPathTurns <= iRange)
										{
											iBestValue = iValue;
											pBestPlot = getPathEndTurnPlot();
											pBestFoundPlot = pLoopPlot;
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

	if ((pBestPlot != NULL) && (pBestFoundPlot != NULL))
	{
		if (atPlot(pBestFoundPlot))
		{
			getGroup()->pushMission(MISSION_FOUND, -1, -1, 0, false, false, MISSIONAI_FOUND, pBestFoundPlot);
			return true;
		}
		else if (!bFollow)
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), MOVE_SAFE_TERRITORY, false, false, MISSIONAI_FOUND, pBestFoundPlot);
			return true;
		}
	}

	return false;
}
bool CvUnitAI::AI_joinCityBrave()
{
	CvCity* pCity = plot()->getPlotCity();
	if (pCity == NULL || pCity->getOwnerINLINE() != getOwnerINLINE())
	{
		return false;
	}
	
	int iProduction = pCity->getRawYieldProduced(YIELD_FOOD);
	int iConsumption = pCity->getRawYieldConsumed(YIELD_FOOD);

	if (((iProduction / 2) + 1) >= (iConsumption + GC.getFOOD_CONSUMPTION_PER_POPULATION()))
	{
		if (canJoinCity(plot()))
		{
			joinCity();
			pCity->AI_setTargetSize(pCity->getPopulation());
			AI_setMovePriority(0);
			return true;
		}
	}
	return false;
}
						
bool CvUnitAI::AI_joinCity(int iMaxPath)
{
	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
	
	CvUnit* pTransportUnit = getTransportUnit();
	
	bool bForceTransport = false;
	if (plot()->isCity() && (plot()->getOwnerINLINE() != getOwnerINLINE()))
	{
		bForceTransport = true;
	}
	
	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	CvPlot* pBestJoinPlot = NULL;
	ProfessionTypes eBestProfession = NO_PROFESSION;
	
	ProfessionTypes eOptimalProfession = kOwner.AI_idealProfessionForUnit(getUnitType());
	
	int iLoop;
	bool bTransportPath = false;
	for (CvCity* pCity = kOwner.firstCity(&iLoop); pCity != NULL; pCity = kOwner.nextCity(&iLoop))
	{
		CvPlot* pLoopPlot = pCity->plot();

		if (canJoinCity(pLoopPlot, true))
		{
			int iPathTurns = 0;
			bool bTransport = pTransportUnit != NULL;
			
			if (!bForceTransport && canUnload() && (pLoopPlot->area() == area()))
			{
				bTransport = false;
			}

			bool bValid = atPlot(pLoopPlot);
				
			if (!bValid)
			{
				if (bTransport)
				{
					bValid = pTransportUnit->generatePath(pLoopPlot, 0, bTransportPath, &iPathTurns);
				}
				else
				{
					bValid = generatePath(pLoopPlot, MOVE_NO_ENEMY_TERRITORY, !bTransportPath, &iPathTurns);
				}
				bTransportPath = bTransport;
			}
				
			if (bValid && iPathTurns < iMaxPath)
				{
					ProfessionTypes eProfession;
					int iValue = pCity->AI_unitJoinCityValue(this, &eProfession);
					
					iValue *= 100;
					iValue /= 100;
					
					int iIncoming = kOwner.AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_FOUND, getGroup());
					int iSizeGap = pCity->AI_getTargetSize() - (pCity->getPopulation() + iIncoming, 0);
					if (iSizeGap > 0)
					{
						int iModifier = iSizeGap * (eOptimalProfession == NO_PROFESSION) ? 20 : 5;
						iModifier += (pCity->getPopulation() + iIncoming == 1) ? 100 : 25;
													
						iValue *= 100 + iModifier;
						iValue /= 100;
					}
					
					iValue *= 100;
					iValue /= 100 + 10 * iPathTurns;

					if (atPlot(pLoopPlot))
					{
						iValue *= 100 + std::max(0, (15 - pCity->getPopulation()) * 2) + ((eOptimalProfession == NO_PROFESSION) ? 25 : 10);
						iValue /= 100;
					}
					
					if (iValue > iBestValue)
					{
						iBestValue = iValue;
						pBestPlot = atPlot(pLoopPlot) ? pLoopPlot : pLoopPlot;
						pBestJoinPlot = pLoopPlot;
						eBestProfession = eProfession;
					}
				}
			}
		}

	
	if (pBestJoinPlot == NULL)
	{
		CvCity* pPlotCity = plot()->getPlotCity();
		if (pPlotCity != NULL && (pPlotCity->getOwnerINLINE() == getOwnerINLINE()))
		{
			if (canUnload())
			{
				unload();
				return true;
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestJoinPlot != NULL))
	{
		if (atPlot(pBestJoinPlot))
		{
			if (!canJoinCity(plot(), true))
			{
				if (getTransportUnit() != NULL)
				{
					unload();
				}
				getGroup()->pushMission(MISSION_SKIP);				
				return true;			
			}
			
			AI_setMovePriority(0);
			plot()->getPlotCity()->addPopulationUnit(this, eBestProfession);
			return true;
		}
		else if ((pTransportUnit != NULL) && !(!bForceTransport && canUnload() && (pBestJoinPlot->area() == area())))
		{
			getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_FOUND, pBestJoinPlot);
			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), MOVE_NO_ENEMY_TERRITORY, false, false, MISSIONAI_FOUND, pBestJoinPlot);
			return true;
		}
	}
	return false;
}
	

bool CvUnitAI::AI_joinCityReplace(int iMaxPath)
{
	CvPlayer& kOwner = GET_PLAYER(getOwnerINLINE());
	
	CvUnit* pTransportUnit = getTransportUnit();
	
	bool bIsExpert = false;
	for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
	{
		if (getUnitInfo().getYieldModifier(iYield) > 0 || getUnitInfo().getYieldChange(iYield) > 0 || getUnitInfo().getBonusYieldChange(iYield) > 0)
		{
			bIsExpert = true;
			break;
		}
	}
	if (!bIsExpert)
	{
		return false;
	}
	
	
	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	CvPlot* pBestJoinPlot = NULL;
	
	
	int iLoop;
	bool bTransportPath = false;
	for (CvCity* pCity = kOwner.firstCity(&iLoop); pCity != NULL; pCity = kOwner.nextCity(&iLoop))
	{
		CvPlot* pLoopPlot = pCity->plot();

		if (canJoinCity(pLoopPlot, true))
		{
			int iPathTurns = 0;
			bool bTransport = pTransportUnit != NULL;
			
			if (canUnload() && (pLoopPlot->area() == area()))
			{
				bTransport = false;
			}
				
			bool bValid = atPlot(pLoopPlot);

			if (!bValid)
			{
				if (bTransport)
				{
					bValid = pTransportUnit->generatePath(pLoopPlot, 0, bTransportPath, &iPathTurns);
				}
				else
			{
					bValid = generatePath(pLoopPlot, MOVE_NO_ENEMY_TERRITORY, !bTransportPath, &iPathTurns);
				}
				bTransportPath = bTransport;
			}
				
			if (bValid && iPathTurns < iMaxPath)
				{
					int iValue = pCity->AI_unitJoinReplaceValue(this);

					if (iValue > 0 && atPlot(pLoopPlot))
					{
						iValue++;
					}

					iValue -= std::min(50, 5 + 5 * iPathTurns);
					
					if (iValue > iBestValue)
					{
						iBestValue = iValue;
						pBestPlot = atPlot(pLoopPlot) ? pLoopPlot :  getPathEndTurnPlot();
						pBestJoinPlot = pLoopPlot;
					}
				}
			}
		}

	
	if (pBestJoinPlot == NULL)
	{
		CvCity* pPlotCity = plot()->getPlotCity();
		if (pPlotCity != NULL && (pPlotCity->getOwnerINLINE() == getOwnerINLINE()))
		{
			if (canUnload())
			{
				unload();
				return true;
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestJoinPlot != NULL))
	{
		if (atPlot(pBestJoinPlot))
		{
			if (!canJoinCity(plot()))
			{
				getGroup()->pushMission(MISSION_SKIP);
				return true;			
			}
			
			if (AI_betterJob())
			{
				return true;
			}
			return true;
		}
		else if ((pTransportUnit != NULL) && !(canUnload() && (pBestJoinPlot->area() == area())))
		{
			getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_FOUND, pBestJoinPlot);
			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), MOVE_NO_ENEMY_TERRITORY, false, false, MISSIONAI_FOUND, pBestJoinPlot);
			return true;
		}
	}
	return false;
}

bool CvUnitAI::AI_joinOptimalCity()
{
	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
	
	CvUnit* pTransportUnit = getTransportUnit();
	
	bool bForceTransport = false;
	if (plot()->isCity() && (plot()->getOwnerINLINE() != getOwnerINLINE()))
	{
		bForceTransport = true;
	}
	
	ProfessionTypes eOptimalProfession = kOwner.AI_idealProfessionForUnit(getUnitType());
	
	if (eOptimalProfession == NO_PROFESSION)
	{
		return false;
	}
	
	
	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	CvPlot* pBestJoinPlot = NULL;
	
	
	int iLoop;
	bool bTransportPath = false;
	for (CvCity* pCity = kOwner.firstCity(&iLoop); pCity != NULL; pCity = kOwner.nextCity(&iLoop))
	{
		CvPlot* pLoopPlot = pCity->plot();

		if (canJoinCity(pLoopPlot, true))
		{
			int iPathTurns = 0;
			bool bTransport = pTransportUnit != NULL;
			
			if (!bForceTransport && canUnload() && (pLoopPlot->area() == area()))
			{
				bTransport = false;
			}
				
			bool bValid = atPlot(pLoopPlot);

			if (!bValid)
			{
				if (bTransport)
				{
					bValid = pTransportUnit->generatePath(pLoopPlot, 0, bTransportPath, &iPathTurns);
				}
				else
			{
					bValid = generatePath(pLoopPlot, MOVE_NO_ENEMY_TERRITORY, !bTransportPath, &iPathTurns);
				}
				bTransportPath = bTransport;
			}
				
			if (bValid)
				{
					int iValue = kOwner.AI_professionBasicValue(eOptimalProfession, getUnitType(), pCity);

					if (iValue > 0)
					{
						if (AI_getIdealProfession() != NO_PROFESSION)
						{
							bool bUnique = true;
							YieldTypes eYieldProduced = (YieldTypes)GC.getProfessionInfo(AI_getIdealProfession()).getYieldProduced();
							if (eYieldProduced == YIELD_HAMMERS || eYieldProduced == YIELD_BELLS)
							{
								for (int i = 0; i < pCity->getPopulation(); ++i)
								{
									CvUnit* pLoopUnit = pCity->getPopulationUnitByIndex(i);							
									if (pLoopUnit->AI_getIdealProfession() == AI_getIdealProfession())
									{
										bUnique = false;
										break;
									}
								}
							if (bUnique)
							{
								if (pCity->AI_getYieldAdvantage(YIELD_EDUCATION) >= 100)
								{
									iValue *= 2;
								}
							}
						}
						}
						
						iValue += 50;
						iValue -= iPathTurns;

						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							pBestPlot = pLoopPlot;
							pBestJoinPlot = pLoopPlot;
						}
					}
				}
			}
		}

	
	if (pBestJoinPlot == NULL)
	{
		CvCity* pPlotCity = plot()->getPlotCity();
		if (pPlotCity != NULL && (pPlotCity->getOwnerINLINE() == getOwnerINLINE()))
		{
			if (canUnload())
			{
				unload();
				return true;
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestJoinPlot != NULL))
	{
		if (atPlot(pBestJoinPlot))
		{
			if (canJoinCity(plot()))
			{
				AI_setMovePriority(0);
				joinCity();
				return true;
			}
			else
			{
				getGroup()->pushMission(MISSION_SKIP);
				return true;			
			}
			
			if (AI_betterJob())
			{
				return true;
			}
			return true;
		}
		else if ((pTransportUnit != NULL) && !(!bForceTransport && canUnload() && (pBestJoinPlot->area() == area())))
		{
			getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_FOUND, pBestJoinPlot);
			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), MOVE_NO_ENEMY_TERRITORY, false, false, MISSIONAI_FOUND, pBestJoinPlot);
			return true;
		}
	}
	return false;
}

bool CvUnitAI::AI_joinCityDefender()
{
	CvCity* pCity = plot()->getPlotCity();
	if (pCity == NULL || pCity->getOwnerINLINE() != getOwnerINLINE())
	{
		return false;
	}
	if (getProfession() == NO_PROFESSION)
	{
		return false;
	}
	if (!canJoinCity(plot()))
	{
		return false;
	}
	
	CvProfessionInfo& kProfession = GC.getProfessionInfo(getProfession());
	
	int iCapacity = pCity->getMaxYieldCapacity();
	for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
	{
		int iAmount = GET_PLAYER(getOwnerINLINE()).getYieldEquipmentAmount(getProfession(), (YieldTypes)iYield);
		if (iAmount > 0)
		{
			if ((pCity->getYieldStored((YieldTypes)iYield) + (iAmount * 2)) > iCapacity)
			{
				return false;
			}
		}
	}

	AI_setMovePriority(0);
	joinCity();
	return true;
}

bool CvUnitAI::AI_yieldDestination(int iMaxPath)
{
	CvPlayer& kOwner = GET_PLAYER(getOwnerINLINE());
	
	CvCity* pPlotCity = plot()->getPlotCity();
	if (pPlotCity != NULL)
	{
		AI_setMovePriority(0);
		unload();
		return true;
	}
	
	CvUnit* pTransportUnit = getTransportUnit();
	FAssert(pTransportUnit != NULL);
	if (pTransportUnit == NULL)
	{
		return false;
	}
	
	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	
	int iLoop;
	CvCity* pCity;
	for (pCity = kOwner.firstCity(&iLoop); pCity != NULL; pCity = kOwner.nextCity(&iLoop))
	{
		CvPlot* pLoopPlot = pCity->plot();
		if (!atPlot(pLoopPlot))
		{
			int iPathTurns;
			if (pTransportUnit->generatePath(pLoopPlot, 0, true, &iPathTurns))
			{
				if (iPathTurns < iMaxPath)
				{
					int iValue = 10 + 3 * std::max(0, pCity->getMaintainLevel(getYield()) - pCity->getYieldStored(getYield()));
					iValue += pCity->getMaxYieldCapacity() - pCity->getYieldStored(getYield());
					
					iValue = iValue / (iPathTurns + 3);
					
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
		getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_FOUND, pBestPlot);
		return true;
	}
	return false;
}

bool CvUnitAI::AI_yieldNativeDestination(int iMaxPath)
{
	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
	CvUnit* pTransportUnit = getTransportUnit();
	FAssert(pTransportUnit != NULL);
	if (pTransportUnit == NULL)
	{
		return false;
	}
	
	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		CvPlayerAI& kLoopPlayer = GET_PLAYER((PlayerTypes)i);
		if (kLoopPlayer.isAlive() && kLoopPlayer.isNative() && !atWar(getTeam(), kLoopPlayer.getTeam()))
		{
			if (kLoopPlayer.AI_maxGoldTrade(getOwnerINLINE()) > 0)
			{
				if (kOwner.AI_getAttitude((PlayerTypes)i) >= ATTITUDE_CAUTIOUS)
				{
					int iLoop;
					CvCity* pCity;
					for (pCity = kLoopPlayer.firstCity(&iLoop); pCity != NULL; pCity = kLoopPlayer.nextCity(&iLoop))
					{
						CvPlot* pLoopPlot = pCity->plot();
						
						if (!atPlot(pLoopPlot) && pLoopPlot->isRevealed(getTeam(), false))
						{

							int iPathTurns = 0;
							if (pTransportUnit->generatePath(pLoopPlot, 0, true, &iPathTurns))
							{
								if (iPathTurns < iMaxPath)
								{
									int iSellPrice = kLoopPlayer.AI_yieldTradeVal(getYield(), getTransportUnit()->getIDInfo(), getOwnerINLINE());
									iSellPrice = std::min(iSellPrice, kLoopPlayer.AI_maxGoldTrade(getOwnerINLINE()));
									int iBuyPrice = (GET_PLAYER(kOwner.getParent()).getYieldSellPrice(getYield()) - 1) * getYieldStored();
									
									if (iSellPrice >= iBuyPrice)
									{
										int iValue = 100 * iSellPrice / (iPathTurns + 2);
										{
										
											if (iValue > iBestValue)
											{
												iBestValue = iValue;
												pBestPlot = pLoopPlot;
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
	
	if (pBestPlot != NULL)
	{
		getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_TRANSPORT_SEA, pBestPlot);
		return true;
	}
	return false;
}

bool CvUnitAI::AI_tradeWithCity()
{
	bool bSaleMade = false;
	CvPlayer& kOwner = GET_PLAYER(getOwnerINLINE());
	
	CvCity* pPlotCity = plot()->getPlotCity();
	if (pPlotCity != NULL)
	{
		CvPlayerAI& kPlotPlayer = GET_PLAYER(pPlotCity->getOwner());
		if (kPlotPlayer.isNative())
		{
			if (canTradeYield(plot()))
			{
				CLLNode<IDInfo>* pUnitNode = plot()->headUnitNode();
				//Sell to city.
				std::vector<CvUnit*> apUnits;
				while (pUnitNode != NULL)
				{
					CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = plot()->nextUnitNode(pUnitNode);

					if (pLoopUnit->getTransportUnit() == this)
					{
						YieldTypes eYield = pLoopUnit->getYield();
						if (pLoopUnit->isGoods())
						{
							if (pLoopUnit->AI_getUnitAIState() == UNITAI_STATE_SELL_TO_NATIVES)
							{
								apUnits.push_back(pLoopUnit);
							}
						}
					}
				}
				
				for (uint i = 0; i < apUnits.size(); ++i)
				{
					int iGold = std::min(kPlotPlayer.AI_maxGoldTrade(getOwnerINLINE()), kPlotPlayer.AI_yieldTradeVal(apUnits[i]->getYield(), getIDInfo(), getOwnerINLINE()));
					
					CLinkList<TradeData> theirList;
					
					CLinkList<TradeData> ourList;

					TradeData item;
					setTradeItem(&item, TRADE_YIELD, apUnits[i]->getYield(), &getIDInfo());
					ourList.insertAtEnd(item);
					
					setTradeItem(&item, TRADE_GOLD, iGold, &getIDInfo());
					theirList.insertAtEnd(item);

					GC.getGameINLINE().implementDeal(getOwnerINLINE(), pPlotCity->getOwnerINLINE(), &ourList, &theirList);
					bSaleMade = true;
				}
			}
		}
	}
	return bSaleMade;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_settlerSeaFerry()
{
	PROFILE_FUNC();

	FAssert(getCargo() > 0);
	FAssert(getUnitAICargo(UNITAI_SETTLER) > 0);

	if (!canCargoAllMove())
	{
		return false;
	}

	CvArea* pWaterArea = plot()->waterArea();
	FAssertMsg(pWaterArea != NULL, "Ship out of water?");

	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	
	CvCity* pLoopCity;
	int iLoop;
	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		int iValue = pLoopCity->AI_getWorkersNeeded();
		if (iValue > 0)
		{
			iValue -= GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopCity->plot(), MISSIONAI_FOUND, getGroup());
			if (iValue > 0)
			{
				int iPathTurns;
				if (generatePath(pLoopCity->plot(), 0, true, &iPathTurns))
				{
					iValue += std::max(0, (GET_PLAYER(getOwnerINLINE()).AI_neededWorkers(pLoopCity->area()) - GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(pLoopCity->area(), UNITAI_WORKER)));
					iValue *= 1000;
					iValue /= 4 + iPathTurns;
					if (atPlot(pLoopCity->plot()))
					{
						iValue += 100;
					}
					else
					{
						iValue += GC.getGame().getSorenRandNum(100, "AI settler sea ferry");
					}
					if (iValue > iBestValue)
					{
						iBestValue = iValue;
						pBestPlot = pLoopCity->plot();							
					}
				}
			}
		}
	}
	
	if (pBestPlot != NULL)
	{
		if (atPlot(pBestPlot))
		{
			unloadAll(); // XXX is this dangerous (not pushing a mission...)
			return true;
		}
		else
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_FOUND, pBestPlot);
			return true;
		}
	}
	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_specialSeaTransportMissionary()
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvCity* pCity;
	CvUnit* pMissionaryUnit;
	CvUnit* pLoopUnit;
	CvPlot* pLoopPlot;
	CvPlot* pPlot;
	CvPlot* pBestPlot;
	CvPlot* pBestSpreadPlot;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iI;

	FAssert(getCargo() > 0);
	FAssert(getUnitAICargo(UNITAI_MISSIONARY) > 0);

	if (!canCargoAllMove())
	{
		return false;
	}

	pPlot = plot();

	pMissionaryUnit = NULL;

	pUnitNode = pPlot->headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pPlot->nextUnitNode(pUnitNode);

		if (pLoopUnit->getTransportUnit() == this)
		{
			if (pLoopUnit->AI_getUnitAIType() == UNITAI_MISSIONARY)
			{
				pMissionaryUnit = pLoopUnit;
				break;
			}
		}
	}

	if (pMissionaryUnit == NULL)
	{
		return false;
	}

	iBestValue = 0;
	pBestPlot = NULL;
	pBestSpreadPlot = NULL;

	// XXX what about non-coastal cities?
	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (pLoopPlot->isCoastalLand())
		{
			pCity = pLoopPlot->getPlotCity();

			if (pCity != NULL)
			{
				iValue = 0;

				if (iValue > 0)
				{
					if (!(pLoopPlot->isVisibleEnemyUnit(this)))
					{
						if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_SPREAD, getGroup()) == 0)
						{
							if (generatePath(pLoopPlot, 0, true, &iPathTurns))
							{
								iValue *= pCity->getPopulation();

								if (pCity->getOwnerINLINE() == getOwnerINLINE())
								{
									iValue *= 4;
								}
								else if (pCity->getTeam() == getTeam())
								{
									iValue *= 3;
								}

								FAssert(iPathTurns > 0);

								if (iPathTurns == 1)
								{
									iValue *= 2;
								}

								iValue *= 1000;

								iValue /= (iPathTurns + 1);

								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									pBestPlot = getPathEndTurnPlot();
									pBestSpreadPlot = pLoopPlot;
								}
							}
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestSpreadPlot != NULL))
	{
		FAssert(!(pBestPlot->isImpassable()) || canMoveImpassable());

		if ((pBestPlot == pBestSpreadPlot) || (stepDistance(pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), pBestSpreadPlot->getX_INLINE(), pBestSpreadPlot->getY_INLINE()) == 1))
		{
			if (atPlot(pBestSpreadPlot))
			{
				unloadAll(); // XXX is this dangerous (not pushing a mission...)
				return true;
			}
			else
			{
				getGroup()->pushMission(MISSION_MOVE_TO, pBestSpreadPlot->getX_INLINE(), pBestSpreadPlot->getY_INLINE(), 0, false, false, MISSIONAI_SPREAD, pBestSpreadPlot);
				return true;
			}
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_SPREAD, pBestSpreadPlot);
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_connectPlot(CvPlot* pPlot, int iRange)
{
	PROFILE_FUNC();

	FAssert(canBuildRoute());

	if (!(pPlot->isVisibleEnemyUnit(this)))
	{
		if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pPlot, MISSIONAI_BUILD, getGroup(), iRange) == 0)
		{
			if (generatePath(pPlot, MOVE_SAFE_TERRITORY, true))
			{
				getGroup()->pushMission(MISSION_ROUTE_TO, pPlot->getX_INLINE(), pPlot->getY_INLINE(), MOVE_SAFE_TERRITORY, false, false, MISSIONAI_BUILD, pPlot);
				return true;
			}
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_improveCity(CvCity* pCity)
{
	PROFILE_FUNC();

	CvPlot* pBestPlot;
	BuildTypes eBestBuild;
	MissionTypes eMission;
	
	if (plot()->getWorkingCity() != pCity)
	{
		if (canBuildRoute())
		{
			getGroup()->pushMission(MISSION_ROUTE_TO, pCity->getX_INLINE(), pCity->getY_INLINE(), 0, false, false, MISSIONAI_BUILD, pCity->plot());
		}
		else
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pCity->getX_INLINE(), pCity->getY_INLINE(), 0, false, false);
		}
		return true;
	}
	
	if (plot()->getWorkingCity() == pCity)
	{
		BuildTypes eBuild = pCity->AI_getBestBuild(pCity->getCityPlotIndex(plot()));
		if (eBuild != NO_BUILD)
		{
			if (canBuild(plot(), eBuild, true))
			{ 
				getGroup()->pushMission(MISSION_BUILD, eBuild, -1, 0, false, false, MISSIONAI_BUILD, plot());
				return true;
			}
		}
	}

	if (AI_bestCityBuild(pCity, &pBestPlot, &eBestBuild, NULL, this))
	{
		FAssertMsg(pBestPlot != NULL, "BestPlot is not assigned a valid value");
		FAssertMsg(eBestBuild != NO_BUILD, "BestBuild is not assigned a valid value");
		FAssertMsg(eBestBuild < GC.getNumBuildInfos(), "BestBuild is assigned a corrupt value");
		if ((plot()->getWorkingCity() != pCity) || (GC.getBuildInfo(eBestBuild).getRoute() != NO_ROUTE))
		{
			eMission = MISSION_ROUTE_TO;
		}
		else
		{
			eMission = MISSION_MOVE_TO;
			if (NULL != pBestPlot && generatePath(pBestPlot) && (getPathLastNode()->m_iData2 == 1) && (getPathLastNode()->m_iData1 == 0))
			{
				if (pBestPlot->getRouteType() != NO_ROUTE)
				{
					eMission = MISSION_ROUTE_TO;
				}
			}
			else if (plot()->getRouteType() == NO_ROUTE)
			{
				int iPlotMoveCost = 0;
				iPlotMoveCost = ((plot()->getFeatureType() == NO_FEATURE) ? GC.getTerrainInfo(plot()->getTerrainType()).getMovementCost() : GC.getFeatureInfo(plot()->getFeatureType()).getMovementCost());

				if (plot()->isHills())
				{
					iPlotMoveCost += GC.getHILLS_EXTRA_MOVEMENT();
				}
				if (plot()->isPeak())
				{
					iPlotMoveCost += GC.getPEAK_EXTRA_MOVEMENT();
				}
				if (iPlotMoveCost > 1)
				{
					eMission = MISSION_ROUTE_TO;
				}
			}
		}
		
		eBestBuild = AI_betterPlotBuild(pBestPlot, eBestBuild);

		getGroup()->pushMission(eMission, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_BUILD, pBestPlot);
		getGroup()->pushMission(MISSION_BUILD, eBestBuild, -1, 0, (getGroup()->getLengthMissionQueue() > 0), false, MISSIONAI_BUILD, pBestPlot);

		return true;
	}

	return false;
}

bool CvUnitAI::AI_improveLocalPlot(int iRange, CvCity* pIgnoreCity)
{
	
	int iX, iY;
	
	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	BuildTypes eBestBuild = NO_BUILD;
	
	for (iX = -iRange; iX <= iRange; iX++)
	{
		for (iY = -iRange; iY <= iRange; iY++)
		{
			CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iX, iY);
			if ((pLoopPlot != NULL) && (pLoopPlot->isCityRadius()))
			{
				CvCity* pCity = pLoopPlot->getWorkingCity();
				if ((NULL != pCity) && (pCity->getOwnerINLINE() == getOwnerINLINE()))
				{
					if ((NULL == pIgnoreCity) || (pCity != pIgnoreCity))
					{
						if (AI_plotValid(pLoopPlot))
						{
							int iIndex = pCity->getCityPlotIndex(pLoopPlot);
							if (iIndex != CITY_HOME_PLOT)
							{
								if (((NULL == pIgnoreCity) || ((pCity->AI_getWorkersNeeded() > 0) && (pCity->AI_getWorkersHave() < (1 + pCity->AI_getWorkersNeeded() * 2 / 3)))) && (pCity->AI_getBestBuild(iIndex) != NO_BUILD))
								{
									if (canBuild(pLoopPlot, pCity->AI_getBestBuild(iIndex)))
									{
										bool bAllowed = true;

										if (GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_SAFE_AUTOMATION))
										{
											if (pLoopPlot->getImprovementType() != NO_IMPROVEMENT && pLoopPlot->getImprovementType() != GC.getDefineINT("RUINS_IMPROVEMENT"))
											{
												bAllowed = false;
											}
										}

										if (bAllowed)
										{
											if (pLoopPlot->getImprovementType() != NO_IMPROVEMENT && GC.getBuildInfo(pCity->AI_getBestBuild(iIndex)).getImprovement() != NO_IMPROVEMENT)
											{
												bAllowed = false;
											}
										}

										if (bAllowed)
										{
											int iValue = pCity->AI_getBestBuildValue(iIndex);
											int iPathTurns;
											if (generatePath(pLoopPlot, 0, true, &iPathTurns))
											{
												int iMaxWorkers = 1;
												if (plot() == pLoopPlot)
												{
													iValue *= 3;
													iValue /= 2;
												}
												else if (getPathLastNode()->m_iData1 == 0)
												{
													iPathTurns++;
												}
												else if (iPathTurns <= 1)
												{
													iMaxWorkers = AI_calculatePlotWorkersNeeded(pLoopPlot, pCity->AI_getBestBuild(iIndex));
												}

												if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_BUILD, getGroup()) < iMaxWorkers)
												{
													iValue *= 1000;
													iValue /= 1 + iPathTurns;

													if (iValue > iBestValue)
													{
														iBestValue = iValue;
														pBestPlot = pLoopPlot;
														eBestBuild = pCity->AI_getBestBuild(iIndex);
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
	
	if (pBestPlot != NULL)
	{
	    FAssertMsg(eBestBuild != NO_BUILD, "BestBuild is not assigned a valid value");
	    FAssertMsg(eBestBuild < GC.getNumBuildInfos(), "BestBuild is assigned a corrupt value");

		FAssert(pBestPlot->getWorkingCity() != NULL);
		if (NULL != pBestPlot->getWorkingCity())
		{
			pBestPlot->getWorkingCity()->AI_changeWorkersHave(+1);

			if (plot()->getWorkingCity() != NULL)
			{
				plot()->getWorkingCity()->AI_changeWorkersHave(-1);
			}
		}
		MissionTypes eMission = MISSION_MOVE_TO;

		int iPathTurns;
		if (generatePath(pBestPlot, 0, true, &iPathTurns) && (getPathLastNode()->m_iData2 == 1) && (getPathLastNode()->m_iData1 == 0))
		{
			if (pBestPlot->getRouteType() != NO_ROUTE)
			{
				eMission = MISSION_ROUTE_TO;
			}				
		}
		else if (plot()->getRouteType() == NO_ROUTE)
		{
			int iPlotMoveCost = 0;
			iPlotMoveCost = ((plot()->getFeatureType() == NO_FEATURE) ? GC.getTerrainInfo(plot()->getTerrainType()).getMovementCost() : GC.getFeatureInfo(plot()->getFeatureType()).getMovementCost());

			if (plot()->isHills())
			{
				iPlotMoveCost += GC.getHILLS_EXTRA_MOVEMENT();
			}
			if (plot()->isPeak())
			{
				iPlotMoveCost += GC.getPEAK_EXTRA_MOVEMENT();
			}
			if (iPlotMoveCost > 1)
			{
				eMission = MISSION_ROUTE_TO;
			}
		}
		
		eBestBuild = AI_betterPlotBuild(pBestPlot, eBestBuild);

		getGroup()->pushMission(eMission, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_BUILD, pBestPlot);
		getGroup()->pushMission(MISSION_BUILD, eBestBuild, -1, 0, (getGroup()->getLengthMissionQueue() > 0), false, MISSIONAI_BUILD, pBestPlot);
		return true;
	}
	
	return false;
}

// Returns true if a mission was pushed...
bool CvUnitAI::AI_nextCityToImprove(CvCity* pCity)
{
	PROFILE_FUNC();

	CvCity* pLoopCity;
	CvPlot* pPlot;
	CvPlot* pBestPlot;
	BuildTypes eBuild;
	BuildTypes eBestBuild;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iLoop;

	iBestValue = 0;
	eBestBuild = NO_BUILD;
	pBestPlot = NULL;

	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		if (pLoopCity != pCity)
		{
			int iWorkersNeeded = pLoopCity->AI_getWorkersNeeded();
			int iWorkersHave = pLoopCity->AI_getWorkersHave();
			
			iValue = std::max(0, iWorkersNeeded - iWorkersHave) * 100;
			iValue += iWorkersNeeded * 10;
			iValue *= (iWorkersNeeded + 1);
			iValue /= (iWorkersHave + 1);

			if (iValue > 0)
			{
				if (AI_bestCityBuild(pLoopCity, &pPlot, &eBuild, NULL, this))
				{
					FAssert(pPlot != NULL);
					FAssert(eBuild != NO_BUILD);

					iValue *= 1000;

					if (pLoopCity->isCapital())
					{
					    iValue *= 2;
					}

					generatePath(pPlot, 0, true, &iPathTurns);
					iValue /= (iPathTurns + 1);

					if (iValue > iBestValue)
					{
						iBestValue = iValue;
						eBestBuild = eBuild;
						pBestPlot = pPlot;
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		gDLL->getFAStarIFace()->ForceReset(&GC.getRouteFinder());
		
		CvCity* pThisCity = plot()->getWorkingCity();
		CvCity* pOtherCity = pBestPlot->getWorkingCity();
		
		if (pOtherCity != NULL)
		{
			setHomeCity(pOtherCity);
		}
		
		if (pThisCity != NULL && pOtherCity != NULL)
		{
			if (!(gDLL->getFAStarIFace()->GeneratePath(&GC.getRouteFinder(), pThisCity->getX_INLINE(), pThisCity->getY_INLINE(), pOtherCity->getX_INLINE(), pOtherCity->getY_INLINE(), false, getOwnerINLINE(), true)))
			{
				getGroup()->pushMission(MISSION_MOVE_TO, pThisCity->getX_INLINE(), pThisCity->getY_INLINE(), 0, false, false, MISSIONAI_BUILD, pOtherCity->plot());
				getGroup()->pushMission(MISSION_ROUTE_TO, pOtherCity->getX_INLINE(), pOtherCity->getY_INLINE(), 0, (getGroup()->getLengthMissionQueue() > 0), false, MISSIONAI_BUILD, pOtherCity->plot());
				return true;
			}
		}
		
		eBestBuild = AI_betterPlotBuild(pBestPlot, eBestBuild);

		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_BUILD, pBestPlot);
		getGroup()->pushMission(MISSION_BUILD, eBestBuild, -1, 0, (getGroup()->getLengthMissionQueue() > 0), false, MISSIONAI_BUILD, pBestPlot);
		return true;
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_fortTerritory(bool bCanal)
{
	int iBestValue = 0;
	BuildTypes eBestBuild = NO_BUILD;
	CvPlot* pBestPlot = NULL;

	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
	for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (AI_plotValid(pLoopPlot))
		{
			if (pLoopPlot->getOwnerINLINE() == getOwnerINLINE()) // XXX team???
			{
				if (pLoopPlot->getImprovementType() == NO_IMPROVEMENT)
				{
					int iValue = 0;
					iValue += bCanal ? kOwner.AI_getPlotCanalValue(pLoopPlot) : 0;

					if (iValue > 0)
					{
						int iBestTempBuildValue = MAX_INT;
						BuildTypes eBestTempBuild = NO_BUILD;

						for (int iJ = 0; iJ < GC.getNumBuildInfos(); iJ++)
						{
							BuildTypes eBuild = ((BuildTypes)iJ);
							FAssertMsg(eBuild < GC.getNumBuildInfos(), "Invalid Build");

							if (GC.getBuildInfo(eBuild).getImprovement() != NO_IMPROVEMENT)
							{
								if (GC.getImprovementInfo((ImprovementTypes)(GC.getBuildInfo(eBuild).getImprovement())).isActsAsCity())
								{
								    if (GC.getImprovementInfo((ImprovementTypes)(GC.getBuildInfo(eBuild).getImprovement())).getDefenseModifier() > 0)
								    {
                                        if (canBuild(pLoopPlot, eBuild))
                                        {
                                            iValue = 10000;

                                            iValue /= (GC.getBuildInfo(eBuild).getTime() + 1);

                                            if (iValue < iBestTempBuildValue)
                                            {
                                                iBestTempBuildValue = iValue;
                                                eBestTempBuild = eBuild;
                                            }
                                        }
                                    }
								}
							}
						}

						if (eBestTempBuild != NO_BUILD)
						{
							if (!(pLoopPlot->isVisibleEnemyUnit(this)))
							{
								bool bValid = true;

								if (GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_LEAVE_FORESTS))
								{
									if (pLoopPlot->getFeatureType() != NO_FEATURE)
									{
										if (GC.getBuildInfo(eBestTempBuild).isFeatureRemove(pLoopPlot->getFeatureType()))
										{
											bValid = false;
										}
									}
								}

								if (bValid)
								{
									if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_BUILD, getGroup(), 3) == 0)
									{
										int iPathTurns;
										if (generatePath(pLoopPlot, 0, true, &iPathTurns))
										{
											iValue *= 1000;
											iValue /= (iPathTurns + 1);

											if (iValue > iBestValue)
											{
												iBestValue = iValue;
												eBestBuild = eBestTempBuild;
												pBestPlot = pLoopPlot;
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

	if (pBestPlot != NULL)
	{
		FAssertMsg(eBestBuild != NO_BUILD, "BestBuild is not assigned a valid value");
		FAssertMsg(eBestBuild < GC.getNumBuildInfos(), "BestBuild is assigned a corrupt value");

		getGroup()->pushMission(MISSION_ROUTE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_BUILD, pBestPlot);
		getGroup()->pushMission(MISSION_BUILD, eBestBuild, -1, 0, (getGroup()->getLengthMissionQueue() > 0), false, MISSIONAI_BUILD, pBestPlot);

		return true;
	}
	return false;
}

//returns true if a mission is pushed
//if eBuild is NO_BUILD, assumes a route is desired.
bool CvUnitAI::AI_improvePlot(CvPlot* pPlot, BuildTypes eBuild)
{
	FAssert(pPlot != NULL);
	
	if (eBuild != NO_BUILD)
	{
		FAssertMsg(eBuild < GC.getNumBuildInfos(), "BestBuild is assigned a corrupt value");
		
		eBuild = AI_betterPlotBuild(pPlot, eBuild);
		if (!atPlot(pPlot))
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pPlot->getX_INLINE(), pPlot->getY_INLINE(), 0, false, false, MISSIONAI_BUILD, pPlot);
		}
		getGroup()->pushMission(MISSION_BUILD, eBuild, -1, 0, (getGroup()->getLengthMissionQueue() > 0), false, MISSIONAI_BUILD, pPlot);

		return true;
	}
	else if (canBuildRoute())
	{
		if (AI_connectPlot(pPlot))
		{
			return true;
		}
	}
	
	return false;
	
}

BuildTypes CvUnitAI::AI_betterPlotBuild(CvPlot* pPlot, BuildTypes eBuild)
{
	FAssert(pPlot != NULL);
	FAssert(eBuild != NO_BUILD);
	bool bBuildRoute = false;
	bool bClearFeature = false;
	
	FeatureTypes eFeature = pPlot->getFeatureType();
	
	CvBuildInfo& kOriginalBuildInfo = GC.getBuildInfo(eBuild);
	
	if (kOriginalBuildInfo.getRoute() != NO_ROUTE)
	{
		return eBuild;
	}
	
	int iWorkersNeeded = AI_calculatePlotWorkersNeeded(pPlot, eBuild);
	
	if ((pPlot->getBonusType() == NO_BONUS) && (pPlot->getWorkingCity() != NULL))
	{
		iWorkersNeeded = std::max(1, std::min(iWorkersNeeded, pPlot->getWorkingCity()->AI_getWorkersHave()));
	}
	
	if (eFeature != NO_FEATURE)
	{
		CvFeatureInfo& kFeatureInfo = GC.getFeatureInfo(eFeature);
		if (kOriginalBuildInfo.isFeatureRemove(eFeature))
		{
			if ((kOriginalBuildInfo.getImprovement() == NO_IMPROVEMENT) || (!pPlot->isBeingWorked() || (kFeatureInfo.getYieldChange(YIELD_FOOD)) <= 0))
			{
				bClearFeature = true;
			}
		}
		
		if ((kFeatureInfo.getMovementCost() > 1) && (iWorkersNeeded > 1))
		{
			bBuildRoute = true;
		}
	}
	
	if (pPlot->getBonusType() != NO_BONUS)
	{
		bBuildRoute = true;
	}
	else if (pPlot->isHills())
	{
		if ((GC.getHILLS_EXTRA_MOVEMENT() > 0) && (iWorkersNeeded > 1))
		{
			bBuildRoute = true;
		}
	}
	
	if (pPlot->getRouteType() != NO_ROUTE)
	{
		bBuildRoute = false;
	}
	
	BuildTypes eBestBuild = NO_BUILD;
	int iBestValue = 0;
	for (int iBuild = 0; iBuild < GC.getNumBuildInfos(); iBuild++)
	{
		BuildTypes eBuild = ((BuildTypes)iBuild);
		CvBuildInfo& kBuildInfo = GC.getBuildInfo(eBuild);
		
		
		RouteTypes eRoute = (RouteTypes)kBuildInfo.getRoute();
		if ((bBuildRoute && (eRoute != NO_ROUTE)) || (bClearFeature && kBuildInfo.isFeatureRemove(eFeature)))
		{
			if (canBuild(pPlot, eBuild))
			{
				int iValue = 10000;
				
				if (bBuildRoute && (eRoute != NO_ROUTE))
				{
					iValue *= (1 + GC.getRouteInfo(eRoute).getValue());
					iValue /= 2;
					
					if (pPlot->getBonusType() != NO_BONUS)
					{
						iValue *= 2;
					}
					
					if (pPlot->getWorkingCity() != NULL)
					{
						iValue *= 2 + iWorkersNeeded + ((pPlot->isHills() && (iWorkersNeeded > 1)) ? 2 * GC.getHILLS_EXTRA_MOVEMENT() : 0);
						iValue /= 3;
					}
					ImprovementTypes eImprovement = (ImprovementTypes)kOriginalBuildInfo.getImprovement();
					if (eImprovement != NO_IMPROVEMENT)
					{
						int iRouteMultiplier = ((GC.getImprovementInfo(eImprovement).getRouteYieldChanges(eRoute, YIELD_FOOD)) * 100);
						iValue *= 100 + iRouteMultiplier;
						iValue /= 100;
					}
				}

				iValue /= (kBuildInfo.getTime() + 1);

				if (iValue > iBestValue)
				{
					iBestValue = iValue;
					eBestBuild = eBuild;
				}
			}
		}
	}
	
	if (eBestBuild == NO_BUILD)
	{
		return eBuild;
	}
	return eBestBuild;	
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_routeCity()
{
	PROFILE_FUNC();

	CvCity* pRouteToCity;
	CvCity* pLoopCity;
	int iLoop;

	FAssert(canBuildRoute());

	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		if (AI_plotValid(pLoopCity->plot()))
		{
			pRouteToCity = pLoopCity->AI_getRouteToCity();

			if (pRouteToCity != NULL)
			{
				if (!(pLoopCity->plot()->isVisibleEnemyUnit(this)))
				{
					if (generatePath(pLoopCity->plot(), MOVE_SAFE_TERRITORY, true))
					{
						if (!(pRouteToCity->plot()->isVisibleEnemyUnit(this)))
						{
							if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pRouteToCity->plot(), MISSIONAI_BUILD, getGroup()) == 0)
							{
								if (generatePath(pRouteToCity->plot(), MOVE_SAFE_TERRITORY, true))
								{
									getGroup()->pushMission(MISSION_MOVE_TO, pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE(), MOVE_SAFE_TERRITORY, false, false, MISSIONAI_BUILD, pRouteToCity->plot());
									getGroup()->pushMission(MISSION_ROUTE_TO, pRouteToCity->getX_INLINE(), pRouteToCity->getY_INLINE(), MOVE_SAFE_TERRITORY, (getGroup()->getLengthMissionQueue() > 0), false, MISSIONAI_BUILD, pRouteToCity->plot());

									return true;
								}
							}
						}
					}
				}
			}
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_routeTerritory(bool bImprovementOnly)
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	ImprovementTypes eImprovement;
	RouteTypes eBestRoute;
	bool bValid;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iI, iJ;

	// XXX how do we make sure that we can build roads???

	FAssert(canBuildRoute());

	iBestValue = 0;
	pBestPlot = NULL;

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (AI_plotValid(pLoopPlot))
		{
			if (pLoopPlot->getOwnerINLINE() == getOwnerINLINE()) // XXX team???
			{
				eBestRoute = GET_PLAYER(getOwnerINLINE()).getBestRoute(pLoopPlot);

				if (eBestRoute != NO_ROUTE)
				{
					if (eBestRoute != pLoopPlot->getRouteType())
					{
						if (bImprovementOnly)
						{
							bValid = false;

							eImprovement = pLoopPlot->getImprovementType();

							if (eImprovement != NO_IMPROVEMENT)
							{
								for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
								{
									if (GC.getImprovementInfo(eImprovement).getRouteYieldChanges(eBestRoute, iJ) > 0)
									{
										bValid = true;
										break;
									}
								}
							}
						}
						else
						{
							bValid = true;
						}

						if (bValid)
						{
							if (!(pLoopPlot->isVisibleEnemyUnit(this)))
							{
								if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_BUILD, getGroup(), 1) == 0)
								{
									if (generatePath(pLoopPlot, MOVE_SAFE_TERRITORY, true, &iPathTurns))
									{
										iValue = 10000;

										iValue /= (iPathTurns + 1);

										if (iValue > iBestValue)
										{
											iBestValue = iValue;
											pBestPlot = pLoopPlot;
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

	if (pBestPlot != NULL)
	{
		getGroup()->pushMission(MISSION_ROUTE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), MOVE_SAFE_TERRITORY, false, false, MISSIONAI_BUILD, pBestPlot);
		return true;
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_travelToUpgradeCity()
{
	// is there a city which can upgrade us?
	CvCity* pUpgradeCity = getUpgradeCity(/*bSearch*/ true);
	if (pUpgradeCity != NULL)
	{
		// cache some stuff
		CvPlot* pPlot = plot();
		bool bSeaUnit = (getDomainType() == DOMAIN_SEA);

		// if we at the upgrade city, stop, wait to get upgraded
		if (pUpgradeCity->plot() == pPlot)
		{
			getGroup()->pushMission(MISSION_SKIP);
			return true;
		}

		// find the closest city
		CvCity* pClosestCity = pPlot->getPlotCity();
		bool bAtClosestCity = (pClosestCity != NULL);
		if (pClosestCity == NULL)
		{
			pClosestCity = pPlot->getWorkingCity();
		}
		if (pClosestCity == NULL)
		{
			pClosestCity = GC.getMapINLINE().findCity(getX_INLINE(), getY_INLINE(), NO_PLAYER, getTeam(), true, bSeaUnit);
		}

		// can we path to the upgrade city?
		int iUpgradeCityPathTurns;
		CvPlot* pThisTurnPlot = NULL;
		bool bCanPathToUpgradeCity = generatePath(pUpgradeCity->plot(), 0, true, &iUpgradeCityPathTurns);
		if (bCanPathToUpgradeCity)
		{
			pThisTurnPlot = getPathEndTurnPlot();
		}
		
		// if we close to upgrade city, head there 
		if (NULL != pThisTurnPlot && NULL != pClosestCity && (pClosestCity == pUpgradeCity || iUpgradeCityPathTurns < 4))
		{
			FAssert(!atPlot(pThisTurnPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pThisTurnPlot->getX_INLINE(), pThisTurnPlot->getY_INLINE());
			return true;
		}
		
		// path to the upgrade city
		if (NULL != pThisTurnPlot)
		{
			FAssert(!atPlot(pThisTurnPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pThisTurnPlot->getX_INLINE(), pThisTurnPlot->getY_INLINE());
			return true;
		}
	}

	return false;
}

// Returns true if a mission was pushed...
bool CvUnitAI::AI_retreatToCity(bool bPrimary, int iMaxPath)
{
	PROFILE_FUNC();

	CvCity* pCity;
	CvCity* pLoopCity;
	CvPlot* pBestPlot = NULL;
	int iPathTurns;
	int iValue;
	int iBestValue = MAX_INT;
	int iPass;
	int iLoop;
	int iCurrentDanger = GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot());

	pCity = plot()->getPlotCity();


	if (0 == iCurrentDanger)
	{
		if (pCity != NULL)
		{
			if (pCity->getOwnerINLINE() == getOwnerINLINE())
			{
				if (!bPrimary || GET_PLAYER(getOwnerINLINE()).AI_isPrimaryArea(pCity->area()))
				{
					if (!(pCity->plot()->isVisibleEnemyUnit(this)))
					{
						FAssert(isInGroup());
						getGroup()->pushMission(MISSION_SKIP);
						return true;
					}
				}
			}
		}
	}

	for (iPass = 0; iPass < 4; iPass++)
	{
		for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
		{
			if (AI_plotValid(pLoopCity->plot()))
			{
				if (!bPrimary || GET_PLAYER(getOwnerINLINE()).AI_isPrimaryArea(pLoopCity->area()))
				{
					if (!(pLoopCity->plot()->isVisibleEnemyUnit(this)))
					{
						if (!atPlot(pLoopCity->plot()) && generatePath(pLoopCity->plot(), ((iPass > 1) ? MOVE_IGNORE_DANGER : 0), true, &iPathTurns))
						{
							if (iPathTurns <= ((iPass == 2) ? 1 : iMaxPath))
							{
								if ((iPass > 0) || (getGroup()->canFight() || GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(pLoopCity->plot()) < iCurrentDanger))
								{
									iValue = iPathTurns;

									if (iValue < iBestValue)
									{
										iBestValue = iValue;
										pBestPlot = getPathEndTurnPlot();
										FAssert(!atPlot(pBestPlot));
									}
								}
							}
						}
					}
				}
			}
		}

		if (pBestPlot != NULL)
		{
			break;
		}
		else if (iPass == 0)
		{
			if (pCity != NULL)
			{
				if (pCity->getOwnerINLINE() == getOwnerINLINE())
				{
					if (!bPrimary || GET_PLAYER(getOwnerINLINE()).AI_isPrimaryArea(pCity->area()))
					{
						if (!(pCity->plot()->isVisibleEnemyUnit(this)))
						{
							getGroup()->pushMission(MISSION_SKIP);
							return true;
						}
					}
				}
			}
		}

		if (getGroup()->alwaysInvisible())
		{
			break;
		}
	}

	if (pBestPlot != NULL)
	{
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), ((iPass > 0) ? MOVE_IGNORE_DANGER : 0));
		return true;
	}

	if (pCity != NULL)
	{
		if (pCity->getTeam() == getTeam())
		{
			getGroup()->pushMission(MISSION_SKIP);
			return true;
		}
	}

	return false;
}

bool CvUnitAI::AI_treasureRetreat(int iMaxPathTurns)
{
	PROFILE_FUNC();

	
	int iBestValue = MAX_INT;
	CvPlot* pBestMissionPlot = NULL;
	CvPlot* pBestPlot = NULL;
	
	CvPlot* pStartingPlot = GET_PLAYER(getOwnerINLINE()).getStartingPlot();
	if (pStartingPlot == NULL || !pStartingPlot->isWater())
	{
		return false;
	}
	EuropeTypes eMainEurope = pStartingPlot->getNearestEurope();
	if (eMainEurope == NO_EUROPE)
	{
		return false;
	}

	for (int iPass = 0; iPass < 2; iPass++)
	{
		int iLoop;
		for (CvCity* pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
		{
			CvPlot* pLoopPlot = pLoopCity->plot();
			if (AI_plotValid(pLoopPlot))
			{
				if (!(pLoopPlot->isVisibleEnemyUnit(this)))
				{
					if (pLoopPlot->getNearestEurope() == eMainEurope)
					{
						int iPathTurns = 0;
						if (atPlot(pLoopPlot) || generatePath(pLoopPlot, ((iPass > 0) ? MOVE_IGNORE_DANGER : 0), true, &iPathTurns))
						{
							if (iPathTurns <= iMaxPathTurns)
							{
								int iValue = 3 + pLoopPlot->getDistanceToOcean();
								
								iValue *= 3 + iPathTurns;

								if (iValue < iBestValue)
								{
									iBestValue = iValue;
									pBestPlot = atPlot(pLoopPlot) ? pLoopPlot : getPathEndTurnPlot();
									pBestMissionPlot = pLoopPlot;
								}
							}
						}
					}
				}
			}
		}

		if (pBestPlot != NULL)
		{
			break;
		}

		if (getGroup()->alwaysInvisible())
		{
			break;
		}
	}

	if (pBestPlot != NULL)
	{
		if (atPlot(pBestPlot))
		{
			getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_AWAIT_PICKUP, pBestMissionPlot);
			return true;
		}
		else
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), ((iPass > 0) ? MOVE_IGNORE_DANGER : 0), false, false, MISSIONAI_AWAIT_PICKUP, pBestMissionPlot);
			return true;
		}
	}

	return false;
}

// Returns true if a mission was pushed...
bool CvUnitAI::AI_pickup(UnitAITypes eUnitAI)
{
	PROFILE_FUNC();

	CvCity* pCity;
	CvCity* pLoopCity;
	CvPlot* pBestPlot;
	CvPlot* pBestPickupPlot;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iLoop;

	FAssert(cargoSpace() > 0);
	if (0 == cargoSpace())
	{
		return false;
	}

	pCity = plot()->getPlotCity();

	if (pCity != NULL)
	{
		if (pCity->getOwnerINLINE() == getOwnerINLINE())
		{
			bool bAnyLoaded = false;
			int iCount = 0;
			
			CvPlot* pPlot = pCity->plot();
			CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();

			while (pUnitNode != NULL)
			{
				CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
				pUnitNode = pPlot->nextUnitNode(pUnitNode);		
				
				if (pLoopUnit->AI_getUnitAIType() == eUnitAI)
				{
					if (cargoSpace() >= pLoopUnit->getUnitInfo().getRequiredTransportSize())
					{
						iCount++;
					}
				}
			}
			
			if (iCount > 0)
			{
				if (iCount > 0)
				{
					if (AI_loadUnits(eUnitAI, MISSIONAI_AWAIT_PICKUP))
					{
						return true;
					}
				}
				if ((AI_getUnitAIType() != UNITAI_ASSAULT_SEA) || pCity->AI_isDefended(-1))
				{
					getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_PICKUP, pCity->plot());
					return true;
				}
			}
		}
	}

	iBestValue = 0;
	pBestPlot = NULL;
	pBestPickupPlot = NULL;

	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		if (AI_plotValid(pLoopCity->plot()))
		{
			
			if (!GET_PLAYER(getOwnerINLINE()).AI_unitAIIsCombat(eUnitAI) || pLoopCity->AI_isDefended(-1))
			{
				//int iCount = pLoopCity->plot()->plotCount(PUF_isUnitAIType, eUnitAI, -1, getOwnerINLINE());
				int iCount = 0;
				CvPlot* pPlot = pLoopCity->plot();
				CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();

				while (pUnitNode != NULL)
				{
					CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = pPlot->nextUnitNode(pUnitNode);		
					
					if (pLoopUnit->AI_getUnitAIType() == eUnitAI)
					{
						if (cargoSpace() >= pLoopUnit->getUnitInfo().getRequiredTransportSize())
						{
							iCount++;
						}
					}
				}
				
				iValue = iCount * 10;
				
				if (pLoopCity->getProductionUnitAI() == eUnitAI)
				{
					CvUnitInfo& kUnitInfo = GC.getUnitInfo(pLoopCity->getProductionUnit());
					iValue++;
					iCount++;
				}

				if (iValue > 0)
				{
					iValue += pLoopCity->getPopulation();

					if (!(pLoopCity->plot()->isVisibleEnemyUnit(this)))
					{
						if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopCity->plot(), MISSIONAI_PICKUP, getGroup()) < ((iCount + (cargoSpace() - 1)) / cargoSpace()))
						{
							if (!atPlot(pLoopCity->plot()) && generatePath(pLoopCity->plot(), 0, true, &iPathTurns))
							{
								iValue *= 1000;

								iValue /= (iPathTurns + 1);

								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									pBestPlot = pLoopCity->plot();
									pBestPickupPlot = pLoopCity->plot();
								}
							}
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestPickupPlot != NULL))
	{
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_PICKUP, pBestPickupPlot);
		return true;
	}

	return false;
}

bool CvUnitAI::AI_moveToStagingCity()
{
	CvCity* pLoopCity;
	CvPlot* pBestPlot;

	int iPathTurns;
	int iValue;
	int iBestValue;
	int iLoop;

	iBestValue = 0;
	pBestPlot = NULL;
	
	int iWarCount = 0;
	TeamTypes eTargetTeam = NO_TEAM;
	CvTeam& kTeam = GET_TEAM(getTeam());
	for (int iI = 0; iI < MAX_TEAMS; iI++)
	{
		if ((iI != getTeam()) && GET_TEAM((TeamTypes)iI).isAlive())
		{
			if (kTeam.AI_isSneakAttackPreparing((TeamTypes)iI))
			{
				eTargetTeam = (TeamTypes)iI;
				iWarCount++;
			}			
		}		
	}
	if (iWarCount > 1)
	{
		eTargetTeam = NO_TEAM;
	}
	

	for (pLoopCity = GET_PLAYER(getOwner()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwner()).nextCity(&iLoop))
	{
		if (AI_plotValid(pLoopCity->plot()))
		{
			iValue = pLoopCity->AI_cityThreat();
			if (iValue > 0)
			{
				if (generatePath(pLoopCity->plot(), 0, true, &iPathTurns))
				{
					iValue *= 1000;
					iValue /= (5 + iPathTurns);
					if ((pLoopCity->plot() != plot()) && pLoopCity->isVisible(eTargetTeam, false))
					{
						iValue /= 2;
					}

					if (iValue > iBestValue)
					{
						iBestValue = iValue;
						pBestPlot = getPathEndTurnPlot();
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		if (atPlot(pBestPlot))
		{
			getGroup()->pushMission(MISSION_SKIP);
			return true;
		}
		else
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
			return true;
		}
	}

	return false;
}

bool CvUnitAI::AI_disembark(bool bEnemyCity)
{
	CvTeamAI& kTeam = GET_TEAM(getTeam());
	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	for (int i = 0; i < NUM_DIRECTION_TYPES; ++i)
	{
		CvPlot* pLoopPlot = plotDirection(getX_INLINE(), getY_INLINE(), (DirectionTypes)i);
		
		if ((pLoopPlot != NULL) && (!bEnemyCity || !pLoopPlot->isEnemyCity(*this)))
		{
			int iValue = 0;
			
			int iEnemyDistance = kTeam.AI_enemyCityDistance(pLoopPlot);
			
			if (iEnemyDistance != -1)
			{
				int iOdds = pLoopPlot->isVisibleEnemyDefender(this) ? AI_attackOdds(pLoopPlot, false) : 100;

				iValue = 10000 / std::max(1, iEnemyDistance);
	
				iValue *= iOdds;
				iValue /= 100;
				
				iValue += 1 + GC.getGameINLINE().getSorenRandNum(100, "AI disembark");
				if (generatePath(pLoopPlot, 0, true))
				{
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
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
		return true;
	}
	return false;
}

bool CvUnitAI::AI_imperialSeaAssault()
{
    int iBestValue = 0;
    CvPlot* pBestPlot = NULL;
    CvPlot* pBestAssaultPlot = NULL;

    CvTeamAI& kTeam = GET_TEAM(getTeam());
    
	CvPlot* pTargetPlot = NULL;
	
	if (GET_PLAYER(getOwnerINLINE()).AI_isStrategy(STRATEGY_CONCENTRATED_ATTACK))
	{
		pTargetPlot = GC.getMapINLINE().plotByIndexINLINE(GET_PLAYER(getOwnerINLINE()).AI_getStrategyData(STRATEGY_CONCENTRATED_ATTACK));
	}
    
    for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);
		
		if (!pLoopPlot->isWater() && !pLoopPlot->isImpassable())
		{
            int iWaterCount = 0;
            CvPlot* pCityPlot = NULL;
            int iBestCityValue = 0;
            
			int iValue = 0;

			int iCityDistance = kTeam.AI_enemyCityDistance(pLoopPlot);
			
			if ((iCityDistance != -1) && (iCityDistance < 8))
			{
				iValue = 8 - iCityDistance;
			
				if (iValue > 0)
				{
					iValue += 4;
					iValue *= 100;
					int iPathTurns = 0;
					if (generatePath(pLoopPlot, MOVE_THROUGH_ENEMY, true, &iPathTurns))
					{
						iValue *= 2 + GC.getGame().getSorenRandNum(50, "AI imperial sea assualt1") + GC.getGame().getSorenRandNum(50, "AI imperial sea assualt2");
						
						if (pTargetPlot != NULL)
						{
							iValue *= 3;
							iValue /= std::max(3, stepDistance(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), pTargetPlot->getX_INLINE(), pTargetPlot->getY_INLINE()));
						}
						
						iValue *= 100;
						iValue /= getPathCost() + baseMoves() * 350;
						
						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							pBestAssaultPlot = pLoopPlot;
							pBestPlot = getGroup()->getPathSecondLastPlot();
						}
					}
				}
			}
		}
	}
	
	if (pBestPlot != NULL)
	{
	    if (atPlot(pBestPlot))
	    {
	    	if (AI_wakeCargo(NO_UNITAI, AI_getMovePriority() + 1))
	    	{
				return true;	    		
	    	}
			return false;
	    }
	    else
	    {
            getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_ASSAULT, pBestAssaultPlot);
			if (atPlot(pBestPlot))
			{
				AI_wakeCargo(NO_UNITAI, AI_getMovePriority() + 1);
			}
            return true;
	    }
	}
	return false;
}


// Returns true if a mission was pushed or we should wait for another unit to bombard...
bool CvUnitAI::AI_followBombard()
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pAdjacentPlot1;
	CvPlot* pAdjacentPlot2;
	int iI, iJ;

	if (canBombard(plot()))
	{
		getGroup()->pushMission(MISSION_BOMBARD);
		return true;
	}

	if (getDomainType() == DOMAIN_LAND)
	{
		for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
		{
			pAdjacentPlot1 = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

			if (pAdjacentPlot1 != NULL)
			{
				if (pAdjacentPlot1->isCity())
				{
					if (AI_potentialEnemy(pAdjacentPlot1->getTeam(), pAdjacentPlot1))
					{
						for (iJ = 0; iJ < NUM_DIRECTION_TYPES; iJ++)
						{
							pAdjacentPlot2 = plotDirection(pAdjacentPlot1->getX_INLINE(), pAdjacentPlot1->getY_INLINE(), ((DirectionTypes)iJ));

							if (pAdjacentPlot2 != NULL)
							{
								pUnitNode = pAdjacentPlot2->headUnitNode();

								while (pUnitNode != NULL)
								{
									pLoopUnit = ::getUnit(pUnitNode->m_data);
									pUnitNode = pAdjacentPlot2->nextUnitNode(pUnitNode);

									if (pLoopUnit->getOwnerINLINE() == getOwnerINLINE())
									{
										if (pLoopUnit->canBombard(pAdjacentPlot2))
										{
											if (pLoopUnit->isGroupHead())
											{
												if (pLoopUnit->getGroup() != getGroup())
												{
													if (pLoopUnit->getGroup()->readyToMove())
													{
														return true;
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

	return false;
}


// Returns true if the unit has found a potential enemy...
bool CvUnitAI::AI_potentialEnemy(TeamTypes eTeam, const CvPlot* pPlot)
{
	if (getGroup()->AI_isDeclareWar(pPlot))
	{
		return isPotentialEnemy(eTeam, pPlot);
	}
	else
	{
		return isEnemy(eTeam, pPlot);
	}
}


// Returns true if this plot needs some defense...
bool CvUnitAI::AI_defendPlot(CvPlot* pPlot)
{
	CvCity* pCity;

	if (!canDefend(pPlot))
	{
		return false;
	}

	pCity = pPlot->getPlotCity();

	if (pCity != NULL)
	{
		if (pCity->getOwnerINLINE() == getOwnerINLINE())
		{
			if (pCity->AI_isDanger())
			{
				return true;
			}
		}
	}
	else
	{
		if (pPlot->plotCount(PUF_canDefendGroupHead, -1, -1, getOwnerINLINE()) <= ((atPlot(pPlot)) ? 1 : 0))
		{
			if (pPlot->plotCount(PUF_cannotDefend, -1, -1, getOwnerINLINE()) > 0)
			{
				return true;
			}

//			if (pPlot->defenseModifier(getTeam()) >= 50 && pPlot->isRoute() && pPlot->getTeam() == getTeam())
//			{
//				return true;
//			}
		}
	}

	return false;
}


int CvUnitAI::AI_pillageValue(CvPlot* pPlot)
{
	FAssert(canPillage(pPlot) || (getGroup()->getCargo() > 0));

	CvPlayer& kOwner = GET_PLAYER(getOwnerINLINE());
	if (!kOwner.isNative() && !(pPlot->isOwned()))
	{
		return 0;
	}
		
	int iValue = 0;

	if (pPlot->isRoute())
	{
		iValue++;

		for (int iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
		{
			CvPlot* pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

			if (pAdjacentPlot != NULL && pAdjacentPlot->getTeam() == pPlot->getTeam())
			{
				if (pAdjacentPlot->isCity())
				{
					iValue += 10;
				}

				if (!(pAdjacentPlot->isRoute()))
				{
					if (!(pAdjacentPlot->isWater()) && !(pAdjacentPlot->isImpassable()))
					{
						iValue += 2;
					}
				}
				
				if ((pPlot->getCrumbs() > 0) && isNative() && isEnemy(pAdjacentPlot->getTeam()))
				{
					iValue += 5;
				}
			}
		}
	}

	ImprovementTypes eImprovement;
	if (pPlot->getImprovementDuration() > ((pPlot->isWater()) ? 20 : 5))
	{
		eImprovement = pPlot->getImprovementType();
	}
	else
	{
		eImprovement = pPlot->getRevealedImprovementType(getTeam(), false);
	}

	if (eImprovement != NO_IMPROVEMENT)
	{
		if (pPlot->getWorkingCity() != NULL)
		{
			iValue += (pPlot->calculateImprovementYieldChange(eImprovement, YIELD_FOOD, pPlot->getOwnerINLINE()) * 5);
		}

		iValue += GC.getImprovementInfo(eImprovement).getPillageGold();
	}
	
	iValue *= 100 + 2 * pPlot->defenseModifier(getTeam());
	iValue /= 100;

	return iValue;
}


int CvUnitAI::AI_searchRange(int iRange)
{
	if (iRange == 0)
	{
		return 0;
	}

	if (flatMovementCost() || (getDomainType() == DOMAIN_SEA))
	{
		return (iRange * baseMoves());
	}
	else
	{
		return ((iRange + 1) * (baseMoves() + 1));
	}
}


// XXX at some point test the game with and without this function...
bool CvUnitAI::AI_plotValid(CvPlot* pPlot)
{
	PROFILE_FUNC();

	switch (getDomainType())
	{
	case DOMAIN_SEA:
		if (pPlot->isWater() || m_pUnitInfo->isCanMoveAllTerrain())
		{
			return true;
		}
		else if (pPlot->isFriendlyCity(*this, true) && pPlot->isCoastalLand())
		{
			return true;
		}
		break;

	case DOMAIN_LAND:
		if (pPlot->getArea() == getArea() || m_pUnitInfo->isCanMoveAllTerrain())
		{
			return true;
		}
		break;

	case DOMAIN_IMMOBILE:
		FAssert(false);
		break;

	default:
		FAssert(false);
		break;
	}

	return false;
}


int CvUnitAI::AI_finalOddsThreshold(CvPlot* pPlot, int iOddsThreshold)
{
	PROFILE_FUNC();

	CvCity* pCity;

	int iFinalOddsThreshold;

	iFinalOddsThreshold = iOddsThreshold;

	pCity = pPlot->getPlotCity();

	if (pCity != NULL)
	{
		if (pCity->getDefenseDamage() < ((GC.getMAX_CITY_DEFENSE_DAMAGE() * 3) / 4))
		{
			iFinalOddsThreshold += std::max(0, (pCity->getDefenseDamage() - pCity->getLastDefenseDamage() - (GC.getDefineINT("CITY_DEFENSE_DAMAGE_HEAL_RATE") * 2)));
		}
	}

	if (pPlot->getNumVisiblePotentialEnemyDefenders(this) == 1)
	{
		if (pCity != NULL)
		{
			iFinalOddsThreshold *= 2;
			iFinalOddsThreshold /= 3;
		}
		else
		{
			iFinalOddsThreshold *= 7;
			iFinalOddsThreshold /= 8;
		}
	}
	
	if ((getDomainType() == DOMAIN_SEA) && !getGroup()->hasCargo())
	{
		iFinalOddsThreshold *= 3;
		iFinalOddsThreshold /= 2 + getGroup()->getNumUnits();
	}
	else
	{
		iFinalOddsThreshold *= 6;
		iFinalOddsThreshold /= (3 + GET_PLAYER(getOwnerINLINE()).AI_adjacentPotentialAttackers(pPlot, true) + ((stepDistance(getX_INLINE(), getY_INLINE(), pPlot->getX_INLINE(), pPlot->getY_INLINE()) > 1) ? 1 : 0) + ((AI_isCityAIType()) ? 2 : 0));
	}

	return range(iFinalOddsThreshold, 1, 99);
}


int CvUnitAI::AI_stackOfDoomExtra()
{
	return ((AI_getBirthmark() % (1 + GET_PLAYER(getOwnerINLINE()).getCurrentEra())) + 4);
}

bool CvUnitAI::AI_stackAttackCity(int iRange, int iPowerThreshold, bool bFollow)
{
    PROFILE_FUNC();
	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	int iSearchRange;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iDX, iDY;

	FAssert(canMove());

	if (bFollow)
	{
		iSearchRange = 1;
	}
	else
	{
		iSearchRange = AI_searchRange(iRange);
	}

	iBestValue = 0;
	pBestPlot = NULL;

	for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (AI_plotValid(pLoopPlot))
				{
					if (pLoopPlot->isCity() || (pLoopPlot->isCity(true) && pLoopPlot->isVisibleEnemyUnit(this)))
					{
						if (AI_potentialEnemy(pLoopPlot->getTeam(), pLoopPlot))
						{
							if (!atPlot(pLoopPlot) && ((bFollow) ? canMoveInto(pLoopPlot, /*bAttack*/ true, /*bDeclareWar*/ true) : (generatePath(pLoopPlot, 0, true, &iPathTurns) && (iPathTurns <= iRange))))
							{
								iValue = getGroup()->AI_compareStacks(pLoopPlot, /*bPotentialEnemy*/ true, /*bCheckCanAttack*/ true, /*bCheckCanMove*/ true);

								if (iValue >= iPowerThreshold)
								{
									if (iValue > iBestValue)
									{
										iBestValue = iValue;
										pBestPlot = ((bFollow) ? pLoopPlot : getPathEndTurnPlot());
										FAssert(!atPlot(pBestPlot));
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), ((bFollow) ? MOVE_DIRECT_ATTACK : 0));
		return true;
	}

	return false;
}

bool CvUnitAI::AI_moveIntoCity(int iRange)
{
    PROFILE_FUNC();

	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	int iSearchRange = iRange;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iDX, iDY;

	FAssert(canMove());

	iBestValue = 0;
	pBestPlot = NULL;
	
	if (plot()->isCity())
	{
	    return false;
	}

	iSearchRange = AI_searchRange(iRange);

	for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);
			
			if (pLoopPlot != NULL)
			{
				if (AI_plotValid(pLoopPlot) && (!isEnemy(pLoopPlot->getTeam(), pLoopPlot)))
				{
					if (pLoopPlot->isCity() || (pLoopPlot->isCity(true)))
					{
                        if (canMoveInto(pLoopPlot, false) && (generatePath(pLoopPlot, 0, true, &iPathTurns) && (iPathTurns <= 1)))
                        {
                            iValue = 1;
                            if (pLoopPlot->getPlotCity() != NULL)
                            {
                                 iValue += pLoopPlot->getPlotCity()->getPopulation();
                            }
                            
                            if (iValue > iBestValue)
                            {
                                iBestValue = iValue;
                                pBestPlot = getPathEndTurnPlot();
                                FAssert(!atPlot(pBestPlot));
                            }
						}
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
		return true;
	}

	return false;
}

bool CvUnitAI::AI_poach()
{
	CvPlot* pLoopPlot;
	int iX, iY;
	
	int iBestPoachValue = 0;
	CvPlot* pBestPoachPlot = NULL;
	TeamTypes eBestPoachTeam = NO_TEAM;
	
	if (!GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI))
	{
		return false;
	}
	
	if (GET_TEAM(getTeam()).getNumMembers() > 1)
	{
		return false;
	}
	
	int iNoPoachRoll = GET_PLAYER(getOwnerINLINE()).AI_totalUnitAIs(UNITAI_WORKER);
	iNoPoachRoll += GET_PLAYER(getOwnerINLINE()).getNumCities();
	iNoPoachRoll = std::max(0, (iNoPoachRoll - 1) / 2);
	if (GC.getGameINLINE().getSorenRandNum(iNoPoachRoll, "AI Poach") > 0)
	{
		return false;
	}

	if (GET_TEAM(getTeam()).getAnyWarPlanCount() > 0)
	{
		return false;
	}
	
	FAssert(canAttack());
	
	
	
	int iRange = 1;
	//Look for a unit which is non-combat
	//and has a capture unit type
	for (iX = -iRange; iX <= iRange; iX++)
	{
		for (iY = -iRange; iY <= iRange; iY++)
		{
			if (iX != 0 && iY != 0)
			{
				pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iX, iY);
				if ((pLoopPlot != NULL) && (pLoopPlot->getTeam() != getTeam()) && pLoopPlot->isVisible(getTeam(), false))
				{
					int iPoachCount = 0;
					int iDefenderCount = 0;
					CvUnit* pPoachUnit = NULL;
					CLLNode<IDInfo>* pUnitNode = pLoopPlot->headUnitNode();
					while (pUnitNode != NULL)
					{
						CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
						pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);
						if ((pLoopUnit->getTeam() != getTeam())
							&& GET_TEAM(getTeam()).canDeclareWar(pLoopUnit->getTeam()))
						{
							if (!pLoopUnit->canDefend())
							{
								if (pLoopUnit->getCaptureUnitType(GET_PLAYER(getOwnerINLINE()).getCivilizationType()) != NO_UNIT)
								{
									iPoachCount++;
									pPoachUnit = pLoopUnit;						
								}
							}
							else
							{
								iDefenderCount++;
							}
						}
					}
					
					if (pPoachUnit != NULL)
					{
						if (iDefenderCount == 0)
						{
							int iValue = iPoachCount * 100;
							iValue -= iNoPoachRoll * 25;
							if (iValue > iBestPoachValue)
							{
								iBestPoachValue = iValue;
								pBestPoachPlot = pLoopPlot;
								eBestPoachTeam = pPoachUnit->getTeam();
							}
						}
					}
				}
			}
		}
	}
	
	if (pBestPoachPlot != NULL)
	{
		//No war roll.
		if (!GET_TEAM(getTeam()).AI_performNoWarRolls(eBestPoachTeam))
		{
			GET_TEAM(getTeam()).declareWar(eBestPoachTeam, true, WARPLAN_LIMITED);
			
			FAssert(!atPlot(pBestPoachPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPoachPlot->getX_INLINE(), pBestPoachPlot->getY_INLINE(), MOVE_DIRECT_ATTACK);
			return true;
		}
		
	}
	
	return false;
}

bool CvUnitAI::AI_choke(int iRange)
{
	CvPlot* pBestPlot = NULL;
	int iBestValue = 0;
	for (int iX = -iRange; iX <= iRange; iX++)
	{
		for (int iY = -iRange; iY <= iRange; iY++)
		{
			CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iX, iY);
			if (pLoopPlot != NULL)
			{
				if (isEnemy(pLoopPlot->getTeam()))
				{
					CvCity* pWorkingCity = pLoopPlot->getWorkingCity();
					if ((pWorkingCity != NULL) && (pWorkingCity->getTeam() == pLoopPlot->getTeam()))
					{
						int iValue = -15;
						
						iValue += pLoopPlot->calculatePotentialYield(YIELD_FOOD, NULL, false) * 10;
						
						if (noDefensiveBonus())
						{
							iValue *= std::max(0, ((baseCombatStr() * 120) - GC.getGame().getBestLandUnitCombat()));
						}
						else
						{
							iValue *= pLoopPlot->defenseModifier(getTeam());
						}
						
						if (iValue > 0)
						{
							iValue *= 10;
							
							iValue /= std::max(1, (pLoopPlot->getNumDefenders(getOwnerINLINE()) + ((pLoopPlot == plot()) ? 0 : 1)));
							
							if (generatePath(pLoopPlot, 0, true))
							{
								pBestPlot = getPathEndTurnPlot();
								iBestValue = iValue;
							}
						}
					}
				}
			}
		}
	}
	if (pBestPlot != NULL)
	{
		if (atPlot(pBestPlot))
		{
			getGroup()->pushMission(MISSION_SKIP);
			return true;
		}
		else
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX(), pBestPlot->getY());
			return true;
		}
	}
	
		
	
	return false;
}

bool CvUnitAI::AI_solveBlockageProblem(CvPlot* pDestPlot, bool bDeclareWar)
{
	FAssert(pDestPlot != NULL);
	

	if (pDestPlot != NULL)
	{
		FAStarNode* pStepNode;
		
		CvPlot* pSourcePlot = plot();

		if (gDLL->getFAStarIFace()->GeneratePath(&GC.getStepFinder(), pSourcePlot->getX_INLINE(), pSourcePlot->getY_INLINE(), pDestPlot->getX_INLINE(), pDestPlot->getY_INLINE(), false, 0, true))
		{
			pStepNode = gDLL->getFAStarIFace()->GetLastNode(&GC.getStepFinder());

			while (pStepNode != NULL)
			{
				CvPlot* pStepPlot = GC.getMapINLINE().plotSorenINLINE(pStepNode->m_iX, pStepNode->m_iY);
				if (canMoveOrAttackInto(pStepPlot) && generatePath(pStepPlot, 0, true))
				{
					if (bDeclareWar && pStepNode->m_pPrev != NULL)
					{
						CvPlot* pPlot = GC.getMapINLINE().plotSorenINLINE(pStepNode->m_pPrev->m_iX, pStepNode->m_pPrev->m_iY);
						if (pPlot->getTeam() != NO_TEAM)
						{
							if (!canMoveInto(pPlot, true, true))
							{
								if (!isPotentialEnemy(pPlot->getTeam(), pPlot))
								{									
									CvTeamAI& kTeam = GET_TEAM(getTeam());
									if (kTeam.canDeclareWar(pPlot->getTeam()))
									{
										WarPlanTypes eWarPlan = WARPLAN_LIMITED;
										WarPlanTypes eExistingWarPlan = kTeam.AI_getWarPlan(pDestPlot->getTeam());
										if (eExistingWarPlan != NO_WARPLAN)
										{
											if ((eExistingWarPlan == WARPLAN_TOTAL) || (eExistingWarPlan == WARPLAN_PREPARING_TOTAL))
											{
												eWarPlan = WARPLAN_TOTAL;
											}
											
											if (!kTeam.isAtWar(pDestPlot->getTeam()))
											{
												kTeam.AI_setWarPlan(pDestPlot->getTeam(), NO_WARPLAN);
											}
										}
										kTeam.AI_setWarPlan(pPlot->getTeam(), eWarPlan, true);
										return (AI_targetCity());
									}
								}
							}
						}
					}
					if (pStepPlot->isVisibleEnemyUnit(this))
					{
						FAssert(canAttack());
						CvPlot* pBestPlot = pStepPlot;
						//To prevent puppeteering attempt to barge through
						//if quite close
						if (getPathLastNode()->m_iData2 > 3)
						{
							pBestPlot = getPathEndTurnPlot();
						}

						FAssert(!atPlot(pBestPlot));
						getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), MOVE_DIRECT_ATTACK);
						return true;						
					}
				}
				pStepNode = pStepNode->m_pParent;
			}
		}
	}
	
	return false;
}

int CvUnitAI::AI_calculatePlotWorkersNeeded(CvPlot* pPlot, BuildTypes eBuild)
{
	int iBuildTime = pPlot->getBuildTime(eBuild) - pPlot->getBuildProgress(eBuild);
	int iWorkRate = workRate(true);
	
	if (iWorkRate <= 0)
	{
		FAssert(false);
		return 1;
	}
	int iTurns = iBuildTime / iWorkRate;
	
	if (iBuildTime > (iTurns * iWorkRate))
	{
		iTurns++;
	}
	
	int iNeeded = std::max(1, (iTurns + 2) / 3);
	
	if (pPlot->getBonusType() != NO_BONUS)
	{
		iNeeded *= 2;		
	}
	return iNeeded;
	
}

bool CvUnitAI::AI_canGroupWithAIType(UnitAITypes eUnitAI) const
{
	return true;
}



bool CvUnitAI::AI_allowGroup(const CvUnit* pUnit, UnitAITypes eUnitAI) const
{
	CvSelectionGroup* pGroup = pUnit->getGroup();
	CvPlot* pPlot = pUnit->plot();

	if (pUnit == this)
	{
		return false;
	}

	if (!pUnit->isGroupHead())
	{
		return false;
	}

	if (pGroup == getGroup())
	{
		return false;
	}

	if (pUnit->isCargo())
	{
		return false;
	}

	if (pUnit->AI_getUnitAIType() != eUnitAI)
	{
		return false;
	}

	switch (pGroup->AI_getMissionAIType())
	{
	case MISSIONAI_GUARD_CITY:
		// do not join groups that are guarding cities
		// intentional fallthrough
	case MISSIONAI_LOAD_SETTLER:
	case MISSIONAI_LOAD_ASSAULT:
	case MISSIONAI_LOAD_SPECIAL:
		// do not join groups that are loading into transports (we might not fit and get stuck in loop forever)
		return false;
		break;
	default:
		break;
	}

	if (pGroup->getActivityType() == ACTIVITY_HEAL)
	{
		// do not attempt to join groups which are healing this turn
		// (healing is cleared every turn for automated groups, so we know we pushed a heal this turn)
		return false;
	}

	if (!canJoinGroup(pPlot, pGroup))
	{
		return false;
	}

	if (eUnitAI == UNITAI_COLONIST)
	{
		if (GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(pPlot, 3) > 0)
		{
			return false;
		}
	}
	else if (eUnitAI == UNITAI_ASSAULT_SEA)
	{
		if (!pGroup->hasCargo())
		{
			return false;
		}
	}

	if ((getGroup()->getHeadUnitAI() == UNITAI_DEFENSIVE))
	{
		if (plot()->isCity() && (plot()->getTeam() == getTeam()) && plot()->getBestDefender(getOwnerINLINE())->getGroup() == getGroup())
		{
			return false;
		}
	}

	if (plot()->getOwnerINLINE() == getOwnerINLINE())
	{
		CvPlot* pTargetPlot = pGroup->AI_getMissionAIPlot();

		if (pTargetPlot != NULL)
		{
			if (pTargetPlot->isOwned())
			{
				if (isPotentialEnemy(pTargetPlot->getTeam(), pTargetPlot))
				{
					//Do not join groups which have debarked on an offensive mission
					return false;
				}
			}
		}
	}

	if (pUnit->getInvisibleType() != NO_INVISIBLE)
	{
		if (getInvisibleType() == NO_INVISIBLE)
		{
			return false;
		}
	}

	return true;
}

bool CvUnitAI::AI_isOnMission()
{
	CvPlot* pMissionPlot = getGroup()->AI_getMissionAIPlot();
	if (pMissionPlot != NULL)
	{
		if (!atPlot(pMissionPlot))
		{
			return true;
		}
	}
	return false;	
}

bool CvUnitAI::AI_isObsoleteTradeShip()
{
	CvPlayer& kOwner = GET_PLAYER(getOwnerINLINE());
	
	int iLoop;
	CvUnit* pLoopUnit;
	for (pLoopUnit = kOwner.firstUnit(&iLoop); pLoopUnit; pLoopUnit = kOwner.nextUnit(&iLoop))
	{
		if (pLoopUnit->AI_getUnitAIType() == AI_getUnitAIType())
		{
			if (cargoSpace() < pLoopUnit->cargoSpace())
			{
				return true;
			}
			else if (cargoSpace() == pLoopUnit->cargoSpace())
			{
				if (getGameTurnCreated() > pLoopUnit->getGameTurnCreated())
				{
					return true;
				}				
			}
			
		}
	}
	return false;
}
		
	

void CvUnitAI::read(FDataStreamBase* pStream)
{
	CvUnit::read(pStream);

	uint uiFlag=0;
	pStream->Read(&uiFlag);	// flags for expansion

	pStream->Read(&m_iBirthmark);
	pStream->Read(&m_iMovePriority);
	if (uiFlag > 0)
	{
		pStream->Read(&m_iLastAIChangeTurn);
	}
	pStream->Read((int*)&m_eUnitAIType);
	pStream->Read((int*)&m_eUnitAIState);
	pStream->Read((int*)&m_eOldProfession);
	pStream->Read((int*)&m_eIdealProfessionCache);
	pStream->Read(&m_iAutomatedAbortTurn);
}


void CvUnitAI::write(FDataStreamBase* pStream)
{
	CvUnit::write(pStream);

	uint uiFlag=1;
	pStream->Write(uiFlag);		// flag for expansion

	pStream->Write(m_iBirthmark);
	pStream->Write(m_iMovePriority);
	pStream->Write(m_iLastAIChangeTurn);
	pStream->Write(m_eUnitAIType);
	pStream->Write(m_eUnitAIState);
	pStream->Write(m_eOldProfession);
	pStream->Write(m_eIdealProfessionCache);
	pStream->Write(m_iAutomatedAbortTurn);
	
}

// Private Functions...
