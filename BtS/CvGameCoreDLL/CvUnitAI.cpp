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

// for testing only
#ifndef FINAL_RELEASE
#define DEBUG_OUT_OF_SYNCS
#endif

#define FOUND_RANGE				(7)

//#define DEBUG_WAR_ACTIONS
//this line changed so i can add another commit message :P

// Public Functions...

CvUnitAI::CvUnitAI()
{
	AI_reset();
}


CvUnitAI::~CvUnitAI()
{
	AI_uninit();
}


void CvUnitAI::AI_init(UnitAITypes eUnitAI)
{
	AI_reset(eUnitAI);

	//--------------------------------
	// Init other game data
	AI_setBirthmark(GC.getGameINLINE().getSorenRandNum(10000, "AI Unit Birthmark"));

	FAssertMsg(AI_getUnitAIType() != NO_UNITAI, "AI_getUnitAIType() is not expected to be equal with NO_UNITAI");
	area()->changeNumAIUnits(getOwnerINLINE(), AI_getUnitAIType(), 1);
	GET_PLAYER(getOwnerINLINE()).AI_changeNumAIUnits(AI_getUnitAIType(), 1);
}


void CvUnitAI::AI_uninit()
{
}


void CvUnitAI::AI_reset(UnitAITypes eUnitAI)
{
	AI_uninit();

	m_iBirthmark = 0;

	m_eUnitAIType = eUnitAI;
	
	m_iLastJoinGroupTurn = -1;
	m_iAutomatedAbortTurn = -1;
}

// AI_update returns true when we should abort the loop and wait until next slice
bool CvUnitAI::AI_update()
{
	PROFILE_FUNC();

	CvUnit* pTransportUnit;

	FAssertMsg(canMove(), "canMove is expected to be true");
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

	if (getDomainType() == DOMAIN_LAND)
	{
		if (plot()->isWater())
		{
			getGroup()->pushMission(MISSION_SKIP);
			return false;
		}
		else
		{
			pTransportUnit = getTransportUnit();

			if (pTransportUnit != NULL)
			{
				if (pTransportUnit->getGroup()->hasMoved() || (pTransportUnit->getGroup()->headMissionQueueNode() != NULL))
				{
					getGroup()->pushMission(MISSION_SKIP);
					return false;
				}
			}
		}
	}

	if (AI_afterAttack())
	{
		return false;
	}

	if (getGroup()->isAutomated())
	{
		switch (getGroup()->getAutomateType())
		{
		case AUTOMATE_BUILD:
			if (AI_getUnitAIType() == UNITAI_WORKER)
			{
				AI_workerMove();
			}
			else if (AI_getUnitAIType() == UNITAI_WORKER_SEA)
			{
				AI_workerSeaMove();
			}
			else
			{
				FAssert(false);
			}
			break;

		case AUTOMATE_NETWORK:
			AI_networkAutomated();
			// XXX else wake up???
			break;

		case AUTOMATE_CITY:
			AI_cityAutomated();
			// XXX else wake up???
			break;

		case AUTOMATE_EXPLORE:
			// stagger explorer units a bit, so they do not all go at start of turn and cause more lag
			if (!GC.getGameINLINE().isNetworkMultiPlayer())
			{
				static int s_iLastAutomatedTurn = -1;
				static int s_iLastAutomatedTurnSlice = 0;
				
				int iCurrentTurn = GC.getGameINLINE().getElapsedGameTurns();
				int iCurrentTurnSlice = GC.getGameINLINE().getTurnSlice();

				if (iCurrentTurn == s_iLastAutomatedTurn && iCurrentTurnSlice == s_iLastAutomatedTurnSlice)
				{
					// bail, wait a slice
					return true;
				}

				s_iLastAutomatedTurn = iCurrentTurn;
				s_iLastAutomatedTurnSlice = iCurrentTurnSlice;
			}

			switch (getDomainType())
			{
			case DOMAIN_SEA:
				AI_exploreSeaMove();
				break;

			case DOMAIN_AIR:
				// if we are cargo (on a carrier), hold if the carrier is not done moving yet
				pTransportUnit = getTransportUnit();
				if (pTransportUnit != NULL)
				{
					if (pTransportUnit->isAutomated() && pTransportUnit->canMove() && pTransportUnit->getGroup()->getActivityType() != ACTIVITY_HOLD)
					{
						getGroup()->pushMission(MISSION_SKIP);
						break;
					}
				}
				
				AI_exploreAirMove();
				break;

			case DOMAIN_LAND:
				AI_exploreMove();
				break;

			default:
				FAssert(false);
				break;
			}
			
			// if we have air cargo (we are a carrier), and we done moving, explore with the aircraft as well
			if (hasCargo() && domainCargo() == DOMAIN_AIR && (!canMove() || getGroup()->getActivityType() == ACTIVITY_HOLD))
			{
				CvPlot* pPlot = plot();
				CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();
				while (pUnitNode != NULL && isAutomated())
				{
					CvUnit* pCargoUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = pPlot->nextUnitNode(pUnitNode);

					if (pCargoUnit->getTransportUnit() == this && pCargoUnit->getDomainType() == DOMAIN_AIR)
					{
						if (pCargoUnit->canMove())
						{
							pCargoUnit->getGroup()->setAutomateType(AUTOMATE_EXPLORE);
							pCargoUnit->getGroup()->setActivityType(ACTIVITY_AWAKE);
						}
					}
				}
			}
			break;

		default:
			FAssert(false);
			break;
		}
	
		// if no longer automated, then we want to bail
		return !getGroup()->isAutomated();
	}
	else
	{
		switch (AI_getUnitAIType())
		{
		case UNITAI_UNKNOWN:
			getGroup()->pushMission(MISSION_SKIP);
			break;

		case UNITAI_ANIMAL:
			AI_animalMove();
			break;

		case UNITAI_SETTLE:
			AI_settleMove();
			break;

		case UNITAI_WORKER:
			AI_workerMove();
			break;

		case UNITAI_ATTACK:
			if (isBarbarian())
			{
				AI_barbAttackMove();
			}
			else
			{
				AI_attackMove();
			}
			break;

		case UNITAI_ATTACK_CITY:
			AI_attackCityMove();
			break;

		case UNITAI_COLLATERAL:
			AI_collateralMove();
			break;

		case UNITAI_PILLAGE:
			AI_pillageMove();
			break;

		case UNITAI_RESERVE:
			AI_reserveMove();
			break;

		case UNITAI_COUNTER:
			AI_counterMove();
			break;

		case UNITAI_CITY_DEFENSE:
			AI_cityDefenseMove();
			break;

		case UNITAI_CITY_COUNTER:
		case UNITAI_CITY_SPECIAL:
			AI_cityDefenseExtraMove();
			break;

		case UNITAI_EXPLORE:
			AI_exploreMove();
			break;

		case UNITAI_MISSIONARY:
			AI_missionaryMove();
			break;

		case UNITAI_PROPHET:
			AI_prophetMove();
			break;

		case UNITAI_ARTIST:
			AI_artistMove();
			break;

		case UNITAI_SCIENTIST:
			AI_scientistMove();
			break;

		case UNITAI_GENERAL:
			AI_generalMove();
			break;

		case UNITAI_MERCHANT:
			AI_merchantMove();
			break;

		case UNITAI_ENGINEER:
			AI_engineerMove();
			break;

		case UNITAI_SPY:
			AI_spyMove();
			break;

		case UNITAI_ICBM:
			AI_ICBMMove();
			break;

		case UNITAI_WORKER_SEA:
			AI_workerSeaMove();
			break;

		case UNITAI_ATTACK_SEA:
			if (isBarbarian())
			{
				AI_barbAttackSeaMove();
			}
			else
			{
				AI_attackSeaMove();
			}
			break;

		case UNITAI_RESERVE_SEA:
			AI_reserveSeaMove();
			break;

		case UNITAI_ESCORT_SEA:
			AI_escortSeaMove();
			break;

		case UNITAI_EXPLORE_SEA:
			AI_exploreSeaMove();
			break;

		case UNITAI_ASSAULT_SEA:
			AI_assaultSeaMove();
			break;

		case UNITAI_SETTLER_SEA:
			AI_settlerSeaMove();
			break;

		case UNITAI_MISSIONARY_SEA:
			AI_missionarySeaMove();
			break;

		case UNITAI_SPY_SEA:
			AI_spySeaMove();
			break;

		case UNITAI_CARRIER_SEA:
			AI_carrierSeaMove();
			break;

		case UNITAI_ATTACK_AIR:
			AI_attackAirMove();
			break;

		case UNITAI_DEFENSE_AIR:
			AI_defenseAirMove();
			break;

		case UNITAI_CARRIER_AIR:
			AI_carrierAirMove();
			break;

		default:
			FAssert(false);
			break;
		}
	}

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

	if (atWar(getTeam(), plot()->getTeam()))
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

	if (isFound())
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
	case UNITAI_ANIMAL:
		FAssert(false);
		break;

	case UNITAI_SETTLE:
		return 21;
		break;

	case UNITAI_WORKER:
		return 20;
		break;

	case UNITAI_ATTACK:
		if (collateralDamage() > 0)
		{
			return 17;
		}
		else if (withdrawalProbability() > 0)
		{
			return 15;
		}
		else
		{
			return 13;
		}
		break;

	case UNITAI_ATTACK_CITY:
		if (bombardRate() > 0)
		{
			return 19;
		}
		else if (collateralDamage() > 0)
		{
			return 18;
		}
		else if (withdrawalProbability() > 0)
		{
			return 16;
		}
		else
		{
			return 14;
		}
		break;

	case UNITAI_COLLATERAL:
		return 7;
		break;

	case UNITAI_PILLAGE:
		return 12;
		break;

	case UNITAI_RESERVE:
		return 6;
		break;

	case UNITAI_COUNTER:
		return 5;
		break;

	case UNITAI_CITY_DEFENSE:
		return 4;
		break;

	case UNITAI_CITY_COUNTER:
		return 3;
		break;

	case UNITAI_CITY_SPECIAL:
		return 2;
		break;

	case UNITAI_EXPLORE:
		return 8;
		break;

	case UNITAI_MISSIONARY:
		return 10;
		break;

	case UNITAI_PROPHET:
	case UNITAI_ARTIST:
	case UNITAI_SCIENTIST:
	case UNITAI_GENERAL:
	case UNITAI_MERCHANT:
	case UNITAI_ENGINEER:
		return 11;
		break;

	case UNITAI_SPY:
		return 9;
		break;

	case UNITAI_ICBM:
		break;

	case UNITAI_WORKER_SEA:
		return 6;
		break;

	case UNITAI_ATTACK_SEA:
		return 3;
		break;

	case UNITAI_RESERVE_SEA:
		return 2;
		break;

	case UNITAI_ESCORT_SEA:
		return 1;
		break;

	case UNITAI_EXPLORE_SEA:
		return 4;
		break;

	case UNITAI_ASSAULT_SEA:
		return 10;
		break;

	case UNITAI_SETTLER_SEA:
		return 9;
		break;

	case UNITAI_MISSIONARY_SEA:
		return 7;
		break;

	case UNITAI_SPY_SEA:
		return 8;
		break;

	case UNITAI_CARRIER_SEA:
		return 5;
		break;

	case UNITAI_ATTACK_AIR:
	case UNITAI_DEFENSE_AIR:
	case UNITAI_CARRIER_AIR:
		break;

	default:
		FAssert(false);
		break;
	}

	return 0;
}


int CvUnitAI::AI_groupSecondVal()
{
	return ((getDomainType() == DOMAIN_AIR) ? airBaseCombatStr() : baseCombatStr());
}


// Returns attack odds out of 100 (the higher, the better...)
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

	if (pDefender == NULL)
	{
		return 100;
	}

	iOurStrength = ((getDomainType() == DOMAIN_AIR) ? airCurrCombatStr() : currCombatStr(NULL, NULL));
	iOurFirepower = ((getDomainType() == DOMAIN_AIR) ? iOurStrength : currFirepower(NULL, NULL));

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

	iDamageToUs = max(1,((GC.getDefineINT("COMBAT_DAMAGE") * (iTheirFirepower + iStrengthFactor)) / (iOurFirepower + iStrengthFactor)));
	iDamageToThem = max(1,((GC.getDefineINT("COMBAT_DAMAGE") * (iOurFirepower + iStrengthFactor)) / (iTheirFirepower + iStrengthFactor)));

	iNeededRoundsUs = (pDefender->currHitPoints() + iDamageToThem - 1 ) / iDamageToThem;
	iNeededRoundsThem = (currHitPoints() + iDamageToUs - 1 ) / iDamageToUs;

	if (getDomainType() != DOMAIN_AIR)
	{
		iNeededRoundsUs = max(1, iNeededRoundsUs - (iBaseOdds * ((pDefender->immuneToFirstStrikes() ? 0 : (firstStrikes() + chanceFirstStrikes()/2))) / 100));
		iNeededRoundsThem = max(1, iNeededRoundsThem - ((100 - iBaseOdds) * ((immuneToFirstStrikes() ? 0 : (pDefender->firstStrikes() + pDefender->chanceFirstStrikes()/2))) / 100));
	}

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
	iOdds += ((100 - iOdds) * withdrawalProbability()) / 100;
	iOdds += GET_PLAYER(getOwnerINLINE()).AI_getAttackOddsChange();

	return max(1, min(iOdds, 99));
}


// Returns true if the unit found a build for this city...
bool CvUnitAI::AI_bestCityBuild(CvCity* pCity, CvPlot** ppBestPlot, BuildTypes* peBestBuild, CvPlot* pIgnorePlot)
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	BuildTypes eBuild;
	bool bBuild;
	int iValue;
	int iBestValue;
	int iI;

	iBestValue = 0;
	if (ppBestPlot != NULL)
	{
		*ppBestPlot = NULL;
	}
	if (peBestBuild != NULL)
	{
		*peBestBuild = NO_BUILD;
	}

	bBuild = false;

	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		pLoopPlot = plotCity(pCity->getX_INLINE(), pCity->getY_INLINE(), iI);

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
								if (canBuild(pLoopPlot, eBuild))
								{
									if (!(pLoopPlot->isVisibleEnemyUnit(getOwnerINLINE())))
									{
										// XXX take advantage of range (warning... this could lead to some units doing nothing...)
										int iWorkerCount = GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_BUILD, getGroup());
                                        if (iWorkerCount <= 2)
                                        {
                                            iValue /= max(1, (1 + iWorkerCount));


                                            if (generatePath(pLoopPlot, 0, true))
                                            {
                                                iBestValue = iValue;
                                                if (ppBestPlot != NULL)
                                                {
                                                    *ppBestPlot = pLoopPlot;
                                                }
                                                if (peBestBuild != NULL)
                                                {
                                                    *peBestBuild = eBuild;
                                                }

                                                bBuild = true;
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

	return bBuild;
}


bool CvUnitAI::AI_isCityAIType() const
{
	return ((AI_getUnitAIType() == UNITAI_CITY_DEFENSE) ||
		      (AI_getUnitAIType() == UNITAI_CITY_COUNTER) ||
					(AI_getUnitAIType() == UNITAI_CITY_SPECIAL) ||
						(AI_getUnitAIType() == UNITAI_RESERVE));
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
	FAssertMsg(eNewValue != NO_UNITAI, "NewValue is not assigned a valid value");

	if (AI_getUnitAIType() != eNewValue)
	{
		area()->changeNumAIUnits(getOwnerINLINE(), AI_getUnitAIType(), -1);
		GET_PLAYER(getOwnerINLINE()).AI_changeNumAIUnits(AI_getUnitAIType(), -1);

		m_eUnitAIType = eNewValue;

		area()->changeNumAIUnits(getOwnerINLINE(), AI_getUnitAIType(), 1);
		GET_PLAYER(getOwnerINLINE()).AI_changeNumAIUnits(AI_getUnitAIType(), 1);

		joinGroup(NULL);
	}
}

int CvUnitAI::AI_sacrificeValue(const CvPlot* pPlot) const
{
    int iValue;
    int iCollateralDamageValue = 0;
    if (pPlot != NULL)
    {
        int iPossibleTargets = min((pPlot->getNumVisibleEnemyDefenders(getOwnerINLINE()) - 1), collateralDamageMaxUnits());

        if (iPossibleTargets > 0)
        {
            iCollateralDamageValue = collateralDamage();
            iCollateralDamageValue += max(0, iCollateralDamageValue - 100);
            iCollateralDamageValue *= iPossibleTargets;
            iCollateralDamageValue /= 5;
        }
    }

	if (getDomainType() == DOMAIN_AIR) {
		// Mildly stupid sacrifice value - favors interceptors since they deal damage to intercepting aircraft
		// Devalues units that are expensive and/or injured
		// Feel free to make this smarter...  ~Penjo 19Nov2006
		iValue = 128 * (100 + currInterceptionProbability());
		iValue /= max(1, (1 + GC.getUnitInfo(getUnitType()).getProductionCost()));
		iValue /= getDamage() + 1;
	} else {
		iValue  = 128 * (currEffectiveStr(pPlot, ((pPlot == NULL) ? NULL : this)));
		iValue *= (100 + iCollateralDamageValue);
		iValue /= (100 + cityDefenseModifier());
		iValue *= (100 + withdrawalProbability());

		iValue /= max(1, (1 + GC.getUnitInfo(getUnitType()).getProductionCost()));
		iValue /= (10 + getExperience());
	}

    return iValue;
}

// Protected Functions...

void CvUnitAI::AI_animalMove()
{
	PROFILE_FUNC();

	if (GC.getGameINLINE().getSorenRandNum(100, "Animal Attack") < GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAnimalAttackProb())
	{
		if (AI_anyAttack(1, 0))
		{
			return;
		}
	}

	if (AI_heal())
	{
		return;
	}

	if (AI_patrol())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_settleMove()
{
	PROFILE_FUNC();

	if (GET_PLAYER(getOwnerINLINE()).getNumCities() == 0)
	{
		if (canFound(plot()))
		{
			getGroup()->pushMission(MISSION_FOUND);
			return;
		}
	}

	int iAreaBestFoundValue = area()->getBestFoundValue(getOwnerINLINE());
	
	int iOtherBestFoundValue = 0;
	//Compare found values.
	CvArea* pLoopArea;
	int iLoop;
	for(pLoopArea = GC.getMapINLINE().firstArea(&iLoop); pLoopArea != NULL; pLoopArea = GC.getMapINLINE().nextArea(&iLoop))
	{
		iOtherBestFoundValue = max(iOtherBestFoundValue, pLoopArea->getBestFoundValue(getOwnerINLINE()));		
	}
	
	if ((iOtherBestFoundValue * 100) > (iAreaBestFoundValue * 125))
	{
		if (plot()->getOwnerINLINE() == getOwnerINLINE())
		{
			if (AI_load(UNITAI_SETTLER_SEA, MISSIONAI_LOAD_SETTLER, NO_UNITAI, -1, -1, -1, 0, MOVE_SAFE_TERRITORY))
			{
				return;
			}
		}
	}

	if (!(getGroup()->canDefend()))
	{
		if (AI_retreatToCity())
		{
			return;
		}
	}

	if (plot()->isCity() && (plot()->getOwnerINLINE() == getOwnerINLINE()))
	{
		if ((GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot()) > 0) || (GC.getGameINLINE().getMaxCityElimination() > 0))
		{
			if (getGroup()->getNumUnits() < 3)
			{
				getGroup()->pushMission(MISSION_SKIP);
				return;
			}
		}
	}

	if (iAreaBestFoundValue > 0)
	{
		if (AI_foundRange(FOUND_RANGE))
		{
			return;
		}

		if (AI_found())
		{
			return;
		}
	}

	if (plot()->getOwnerINLINE() == getOwnerINLINE())
	{
		if (AI_load(UNITAI_SETTLER_SEA, MISSIONAI_LOAD_SETTLER, NO_UNITAI, -1, -1, -1, 0, MOVE_NO_ENEMY_TERRITORY))
		{
			return;
		}
	}

	if (AI_exploreRange(5))
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

	CvCity* pCity;
	bool bCanRoute;
	bool bNextCity;

	bCanRoute = canBuildRoute();
	bNextCity = false;
	int iI;

	// XXX could be trouble...
	if (plot()->getOwnerINLINE() != getOwnerINLINE())
	{
		if (AI_retreatToCity())
		{
			return;
		}
	}

	if (!(isHuman()))
	{
		if (plot()->getOwnerINLINE() == getOwnerINLINE())
		{
			if (AI_load(UNITAI_SETTLER_SEA, MISSIONAI_LOAD_SETTLER, UNITAI_SETTLE, 2, -1, -1, 0, MOVE_SAFE_TERRITORY))
			{
				return;
			}
		}
	}

    if (!(getGroup()->canDefend()))
	{
		if (GET_PLAYER(getOwnerINLINE()).AI_getUnitDanger(this, 3) > 0)
		{
			if (AI_retreatToCity()) // XXX maybe not do this??? could be working productively somewhere else...
			{
				return;
			}
		}
	}


	if (bCanRoute)
	{
		BonusTypes eNonObsoleteBonus = plot()->getNonObsoleteBonusType(getTeam());
		if (NO_BONUS != eNonObsoleteBonus)
		{
			if (!(plot()->isConnectedToCapital()))
			{
				ImprovementTypes eImprovement = plot()->getImprovementType();
				if (NO_IMPROVEMENT != eImprovement && GC.getImprovementInfo(eImprovement).isImprovementBonusTrade(eNonObsoleteBonus))
				{
					if (AI_connectPlot(plot()))
					{
						return;
					}
				}
			}
		}
	}

    if (AI_improveBonus(20))
	{
		return;
	}

	if (bCanRoute)
	{
		if (AI_connectCity())
		{
			return;
		}
	}

	pCity = NULL;

	if (plot()->getOwnerINLINE() == getOwnerINLINE())
	{
		pCity = plot()->getWorkingCity();
	}
	if (pCity != NULL)
	{
		bool bMoreBuilds = false;
		for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
		{
			CvPlot* pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);
			if ((iI != CITY_HOME_PLOT) && (pLoopPlot != NULL))
			{
				if (pLoopPlot->getWorkingCity() == pCity)
				{
					if (pLoopPlot->isBeingWorked())
					{
						if (pLoopPlot->getImprovementType() == NO_IMPROVEMENT)
						{
							if (pCity->AI_getBestBuildValue(iI) > 0)
							{
								ImprovementTypes eImprovement;
								eImprovement = (ImprovementTypes)GC.getBuildInfo((BuildTypes)pCity->AI_getBestBuild(iI)).getImprovement();
								if (eImprovement != NO_IMPROVEMENT)
								{
									bMoreBuilds = true;
									break;
								}
							}
						}
					}
				}
			}
		}
		
		if (bMoreBuilds)
		{
			if (AI_improveCity(pCity))
			{
				return;
			}
		}
		
	}

	if ((pCity == NULL) || (pCity->countNumImprovedPlots() > (pCity->getPopulation()+1)))
	{
		if (AI_improveBonus())
		{
			return;
		}

		if (pCity == NULL)
		{
			pCity = GC.getMapINLINE().findCity(getX_INLINE(), getY_INLINE(), getOwnerINLINE()); // XXX do team???
		}

		if (AI_nextCityToImprove(pCity))
		{
			return;
		}

		bNextCity = true;
	}

	if (pCity != NULL)
	{
		if (AI_improveCity(pCity))
		{
			return;
		}
	}

	if (!bNextCity)
	{
		if (AI_improveBonus())
		{
			return;
		}

		if (AI_nextCityToImprove(pCity))
		{
			return;
		}
	}

	if (bCanRoute)
	{
		if (AI_routeTerritory(true))
		{
			return;
		}

		if (AI_connectBonus(false))
		{
			return;
		}

		if (AI_routeCity())
		{
			return;
		}
	}

	if (AI_irrigateTerritory())
	{
		return;
	}

	if (bCanRoute)
	{
		if (AI_routeTerritory())
		{
			return;
		}
	}

	if (!(isHuman()))
	{
		if (AI_nextCityToImproveAirlift())
		{
			return;
		}
	}

	if (!(isHuman()) && (AI_getUnitAIType() == UNITAI_WORKER))
	{
		if (GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(area(), UNITAI_WORKER) > GET_PLAYER(getOwnerINLINE()).AI_neededWorkers(area()))
		{
			if (GC.getGameINLINE().getElapsedGameTurns() > 10)
			{
				if (GET_PLAYER(getOwnerINLINE()).calculateUnitCost() > 0)
				{
					scrap();
					return;
				}
			}
		}
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


void CvUnitAI::AI_barbAttackMove()
{
	PROFILE_FUNC();

	if (AI_guardCity(false, true, 1))
	{
		return;
	}

	if (plot()->isGoody())
	{
		if (plot()->plotCount(PUF_isUnitAIType, UNITAI_ATTACK, -1, getOwnerINLINE()) == 1)
		{
			getGroup()->pushMission(MISSION_SKIP);
			return;
		}
	}

	if (GC.getGameINLINE().getSorenRandNum(2, "AI Barb") == 0)
	{
		if (AI_pillageRange(1))
		{
			return;
		}
	}

	if (AI_anyAttack(1, 20))
	{
		return;
	}

	if (GC.getGameINLINE().isOption(GAMEOPTION_RAGING_BARBARIANS))
	{
		if (AI_pillageRange(4))
		{
			return;
		}

		if (AI_cityAttack(3, 10))
		{
			return;
		}

		if (area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE)
		{
			if (AI_targetCity())
			{
				return;
			}
		}
	}
	else if (GC.getGameINLINE().getNumCivCities() > (GC.getGameINLINE().countCivPlayersAlive() * 3))
	{
		if (AI_cityAttack(1, 15))
		{
			return;
		}

		if (AI_pillageRange(3))
		{
			return;
		}

		if (AI_cityAttack(2, 10))
		{
			return;
		}

		if (area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE)
		{
			if (AI_targetCity())
			{
				return;
			}
		}
	}
	else if (GC.getGameINLINE().getNumCivCities() > (GC.getGameINLINE().countCivPlayersAlive() * 2))
	{
		if (AI_pillageRange(2))
		{
			return;
		}

		if (AI_cityAttack(1, 10))
		{
			return;
		}
	}

	if (AI_heal())
	{
		return;
	}

	if (AI_guardCity(false, true, 2))
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


void CvUnitAI::AI_attackMove()
{
	PROFILE_FUNC();

	{
		PROFILE("CvUnitAI::AI_attackMove() 1");

		if (AI_guardCity(true))
		{
			return;
		}

		if (AI_heal(30, 1))
		{
			return;
		}

		if (AI_group(UNITAI_SETTLE, /*iMaxGroup*/ 1, -1, -1, false, false, false, /*iMaxPath*/ 3, true))
		{
			return;
		}

		if (AI_group(UNITAI_SETTLE, /*iMaxGroup*/ 2, -1, -1, false, false, false, /*iMaxPath*/ 3, true))
		{
			return;
		}

		if (AI_guardCityAirlift())
		{
			return;
		}

		if (AI_guardCity(false, true, 1))
		{
			return;
		}

		//join any city attacks in progress
		if (plot()->getOwnerINLINE() != getOwnerINLINE())
		{
			if (AI_groupMergeRange(UNITAI_ATTACK_CITY, 1, true, true))
			{
				return;
			}
		}
		
		if (AI_cityAttack(1, 55))
		{
			return;
		}

		if (AI_anyAttack(1, 65))
		{
			return;
		}

		if (collateralDamage() > 0)
		{
			if (AI_anyAttack(1, 45, 3))
			{
				return;
			}
		}

		if (!noDefensiveBonus())
		{
			if (AI_guardCity(false, false))
			{
				return;
			}
		}

		if (plot()->getOwnerINLINE() == getOwnerINLINE())
		{
			if (AI_load(UNITAI_SETTLER_SEA, MISSIONAI_LOAD_SETTLER, UNITAI_SETTLE, 3, -1, -1, -1, MOVE_SAFE_TERRITORY, 3))
			{
				return;
			}

			if (AI_load(UNITAI_ASSAULT_SEA, MISSIONAI_LOAD_ASSAULT, NO_UNITAI, -1, -1, -1, -1, MOVE_SAFE_TERRITORY, 4))
			{
				return;
			}

			if (GET_PLAYER(getOwnerINLINE()).AI_unitTargetMissionAIs(this, MISSIONAI_GROUP) > 0)
			{
				getGroup()->pushMission(MISSION_SKIP);
				return;
			}
		}

		if (AI_pillageRange(1, 20))
		{
			return;
		}

		if (AI_heal(50, 3))
		{
			return;
		}

		if (AI_cityAttack(1, 35))
		{
			return;
		}

		if (AI_anyAttack(1, 45))
		{
			return;
		}

		if (AI_pillageRange(3, 20))
		{
			return;
		}

		if (AI_goody(3))
		{
			return;
		}
	}

	{
		PROFILE("CvUnitAI::AI_attackMove() 2");

		if (AI_cityAttack(4, 30))
		{
			return;
		}

		if (AI_anyAttack(2, 40))
		{
			return;
		}

		if (!atWar(getTeam(), plot()->getTeam()))
		{
			if (AI_heal())
			{
				return;
			}
		}

		if ((GET_PLAYER(getOwnerINLINE()).AI_getNumAIUnits(UNITAI_CITY_DEFENSE) > 0) || (GET_TEAM(getTeam()).getAtWarCount(true) > 0))
		{
			if (AI_group(UNITAI_ATTACK_CITY, /*iMaxGroup*/ 1, /*iMaxOwnUnitAI*/ 1, -1, true, true, true, /*iMaxPath*/ 5))
			{
				return;
			}

			if (AI_group(UNITAI_ATTACK, /*iMaxGroup*/ 1, /*iMaxOwnUnitAI*/ 1, -1, true, true, true, /*iMaxPath*/ 4))
			{
				return;
			}

			if (AI_group(UNITAI_ATTACK, /*iMaxGroup*/ 1, /*iMaxOwnUnitAI*/ 1, -1, false, true, true, /*iMaxPath*/ 3))
			{
				return;
			}

			if (AI_group(UNITAI_ATTACK, /*iMaxGroup*/ 1, /*iMaxOwnUnitAI*/ 1, -1, false, false, true, /*iMaxPath*/ 2))
			{
				return;
			}
		}

		if ((area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE) ||
			  (atWar(getTeam(), plot()->getTeam())))
		{
			if (AI_targetCity())
			{
				return;
			}
		}

		if (AI_heal())
		{
			return;
		}

		if (AI_targetBarbCity())
		{
			return;
		}

		if (AI_guardCity(false, true, 3))
		{
			return;
		}

		if (GET_PLAYER(getOwnerINLINE()).getNumCities() > 1)
		{
			if (area()->getAreaAIType(getTeam()) != AREAAI_DEFENSIVE)
			{
				if (area()->getNumUnrevealedTiles(getTeam()) > 0)
				{
					if (GET_PLAYER(getOwnerINLINE()).AI_areaMissionAIs(area(), MISSIONAI_EXPLORE, getGroup()) < (GET_PLAYER(getOwnerINLINE()).AI_neededExplorers(area()) + 1))
					{
						if (AI_exploreRange(3))
						{
							return;
						}

						if (AI_explore())
						{
							return;
						}
					}
				}
			}
		}

		if (AI_protect(35))
		{
			return;
		}

		if (AI_offensiveAirlift())
		{
			return;
		}

		if (AI_defend())
		{
			return;
		}

		if (AI_travelToUpgradeCity())
		{
			return;
		}

		if (AI_fogbust())
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
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_attackCityMove()
{
	PROFILE_FUNC();
	
	// force heal if we in our own city and damaged
	// can we remove this or call AI_heal here?
	if ((getGroup()->getNumUnits() == 1) && (getDamage() > 0) && 
        plot()->getOwnerINLINE() == getOwnerINLINE() && plot()->isCity())
    {
        getGroup()->pushMission(MISSION_HEAL);
		return;
    }
	
//    if (!getGroup()->getMissionType() == MISSION_MOVE_TO)
//    {
//        if (AI_moveIntoCity(1))
//        {
//            return;        
//        }
//    }

	if (plot()->isCity())
	{
		if ((GC.getGame().getGameTurn() - plot()->getPlotCity()->getGameTurnAcquired()) <= 1)
		{
			getGroup()->AI_seperateNonAI(UNITAI_ATTACK_CITY);
		}
	}

	if (AI_guardCity(false, true, 1))
	{
		return;
	}


	if (AI_groupMergeRange(UNITAI_ATTACK_CITY, 0, true, true))
	{
		return;
	}


	//stack attack
	if (getGroup()->getNumUnits() > 1)
	{
		if (AI_stackAttackCity(1, 250, true))
		{
			return;
		}
	}
	
	if (AI_bombardCity())
	{
		return;
	}
	
	//stack attack
	if (getGroup()->getNumUnits() > 1)
	{
		if (AI_stackAttackCity(1, 100, true))
		{
			return;
		}
	}

	if (AI_groupMergeRange(UNITAI_ATTACK_CITY, 2, true, true))
	{
		return;
	}

	if (AI_heal(30, 1))
	{
		return;
	}

	if (plot()->getOwnerINLINE() == getOwnerINLINE())
	{
		if (AI_load(UNITAI_ASSAULT_SEA, MISSIONAI_LOAD_ASSAULT, NO_UNITAI, -1, -1, -1, -1, MOVE_SAFE_TERRITORY, 4))
		{
			return;
		}

		int iTargetCount = GET_PLAYER(getOwnerINLINE()).AI_unitTargetMissionAIs(this, MISSIONAI_GROUP);
		if ((iTargetCount * 5) > getGroup()->getNumUnits())
		{
			if (GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot()) > 0)
			{
				getGroup()->pushMission(MISSION_SKIP);
				return;
			}
		}
	}

	if (AI_cityAttack(1, 30))
	{
		return;
	}

	if (collateralDamage() > 0)
	{
		if (AI_anyAttack(1, 45, 3))
		{
			return;
		}
	}

	if (AI_anyAttack(1, 60))
	{
		return;
	}
	
	bool bAtWar = atWar(getTeam(), plot()->getTeam());
	
	if (bAtWar && (getGroup()->getNumUnits() <= 2))
	{
		if (AI_pillageRange(3, 11))
		{
			return;
		}

		if (AI_pillageRange(1))
		{
			return;
		}
	}

	if (AI_heal(50, 3))
	{
		return;
	}

	if (!bAtWar)
	{
		if (AI_heal())
		{
			return;
		}

		if ((getGroup()->getNumUnits() == 1) && (getTeam() != plot()->getTeam()))
		{
			if (AI_retreatToCity())
			{
				return;
			}
		}
	}

	if (!noDefensiveBonus())
	{
		if (AI_guardCity(false, false))
		{
			return;
		}
	}

	if (AI_group(UNITAI_ATTACK_CITY, -1, -1, -1, true, true, true, /*iMaxPath*/ 10, /*bAllowRegrouping*/ false))
	{
		return;
	}

//	if ((area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE) ||
//		  (atWar(getTeam(), plot()->getTeam())) ||
//			((area()->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE) && (getGroup()->getNumUnits() >= AI_stackOfDoomExtra())))
	if ((getGroup()->getNumUnits() >= AI_stackOfDoomExtra()))
	{
		if (AI_targetCity())
		{
			return;
		}
	}

	if (AI_heal())
	{
		return;
	}
	
	if (AI_moveToStagingCity())
	{
		return;
	}

	if (AI_targetBarbCity())
	{
		return;
	}

	if (AI_offensiveAirlift())
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


void CvUnitAI::AI_collateralMove()
{
	PROFILE_FUNC();

	if (AI_leaveAttack(1, 20, 100))
	{
		return;
	}
	
	if (AI_guardCity(false, true, 1))
	{
		return;
	}

	if (AI_heal(30, 1))
	{
		return;
	}

	if (AI_cityAttack(1, 35))
	{
		return;
	}

	if (AI_anyAttack(1, 45, 3))
	{
		return;
	}

	if (AI_anyAttack(1, 55, 2))
	{
		return;
	}

	if (AI_anyAttack(1, 35, 3))
	{
		return;
	}

	if (AI_anyAttack(1, 30, 4))
	{
		return;
	}

	if (AI_anyAttack(1, 20, 5))
	{
		return;
	}

	if (AI_heal())
	{
		return;
	}

	if (!noDefensiveBonus())
	{
		if (AI_guardCity(false, false))
		{
			return;
		}
	}

	if (AI_anyAttack(2, 55, 3))
	{
		return;
	}

	if (AI_cityAttack(2, 50))
	{
		return;
	}

	if (AI_anyAttack(2, 60))
	{
		return;
	}

	if (AI_protect(50))
	{
		return;
	}

//    if (AI_group(UNITAI_ATTACK_CITY, -1, -1, -1, /*bIgnoreFaster*/ true))
//	{
//		return;
//	}

	if (AI_guardCity(false, true, 3))
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


void CvUnitAI::AI_pillageMove()
{
	PROFILE_FUNC();

	if (AI_guardCity(false, true, 1))
	{
		return;
	}

	if (AI_heal(30, 1))
	{
		return;
	}

	//join any city attacks in progress
	if (plot()->getOwnerINLINE() != getOwnerINLINE())
	{
		if (AI_groupMergeRange(UNITAI_ATTACK_CITY, 1, true, true))
		{
			return;
		}
	}
	
	if (AI_cityAttack(1, 55))
	{
		return;
	}

	if (AI_anyAttack(1, 65))
	{
		return;
	}
	

	if (!noDefensiveBonus())
	{
		if (AI_guardCity(false, false))
		{
			return;
		}
	}
	
	if (AI_pillageRange(3, 11))
	{
		return;
	}

	if (AI_pillageRange(1))
	{
		return;
	}

	if (plot()->getOwnerINLINE() == getOwnerINLINE())
	{
		if (AI_load(UNITAI_ASSAULT_SEA, MISSIONAI_LOAD_ASSAULT, UNITAI_ATTACK, -1, -1, -1, -1, MOVE_SAFE_TERRITORY, 4))
		{
			return;
		}
	}

	if (AI_heal(50, 3))
	{
		return;
	}

	if (!atWar(getTeam(), plot()->getTeam()))
	{
		if (AI_heal())
		{
			return;
		}
	}

	if (AI_group(UNITAI_PILLAGE, /*iMaxGroup*/ 1, /*iMaxOwnUnitAI*/ 1, -1, /*bIgnoreFaster*/ true, false, false, /*iMaxPath*/ 3))
	{
		return;
	}

	if ((area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE) || atWar(getTeam(), plot()->getTeam()))
	{
		if (AI_pillage(20))
		{
			return;
		}
	}

	if (AI_heal())
	{
		return;
	}

	if (AI_guardCity(false, true, 3))
	{
		return;
	}

	if (AI_offensiveAirlift())
	{
		return;
	}

	if (AI_travelToUpgradeCity())
	{
		return;
	}

	if (AI_fogbust())
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


void CvUnitAI::AI_reserveMove()
{
	PROFILE_FUNC();

	if (AI_leaveAttack(2, 55, 130))
	{
		return;
	}
	
	if (plot()->getOwnerINLINE() == getOwnerINLINE())
	{
		if (AI_load(UNITAI_SETTLER_SEA, MISSIONAI_LOAD_SETTLER, UNITAI_SETTLE, -1, -1, 1, -1, MOVE_SAFE_TERRITORY))
		{
			return;
		}
	}
	
	if (AI_guardCity(true))
	{
		return;
	}

	if (AI_guardCityAirlift())
	{
		return;
	}

	if (AI_guardCity(false, true, 1))
	{
		return;
	}

	if (AI_guardBonus(10))
	{
		return;
	}

	if (AI_heal(30, 1))
	{
		return;
	}

	if (AI_cityAttack(1, 55))
	{
		return;
	}

	if (AI_anyAttack(1, 60))
	{
		return;
	}

	if (AI_heal())
	{
		return;
	}

	if (!noDefensiveBonus())
	{
		if (AI_guardCity(false, false))
		{
			return;
		}
	}

//	if (AI_group(UNITAI_COLLATERAL, /*iMaxGroup*/ 1, -1, -1, false, false, false, /*iMaxPath*/ 4))
//	{
//		return;
//	}

	if (AI_cityAttack(3, 45))
	{
		return;
	}

	if (AI_anyAttack(3, 50))
	{
		return;
	}

	if (AI_protect(45))
	{
		return;
	}

	if (AI_guardCity(false, true, 3))
	{
		return;
	}

	if (AI_defend())
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

	if (AI_guardCity(false, true, 1))
	{
		return;
	}

	if (plot()->getOwnerINLINE() == getOwnerINLINE())
	{
		if (AI_load(UNITAI_ASSAULT_SEA, MISSIONAI_LOAD_ASSAULT, UNITAI_ATTACK_CITY, -1, -1, -1, -1, MOVE_SAFE_TERRITORY, 4))
		{
			return;
		}

		if (AI_load(UNITAI_ASSAULT_SEA, MISSIONAI_LOAD_ASSAULT, UNITAI_ATTACK, -1, -1, -1, -1, MOVE_SAFE_TERRITORY, 4))
		{
			return;
		}
	}

	if (!noDefensiveBonus())
	{
		if (AI_guardCity(false, false))
		{
			return;
		}
	}

	if (AI_cityAttack(1, 30))
	{
		return;
	}

	if (AI_anyAttack(1, 40))
	{
		return;
	}

	if (AI_group(UNITAI_ATTACK_CITY, /*iMaxGroup*/ 2, -1, -1, false, /*bIgnoreOwnUnitType*/ true, /*bStackOfDoom*/ true, /*iMaxPath*/ 6))
	{
		return;
	}

	if (AI_group(UNITAI_ATTACK, /*iMaxGroup*/ 2, -1, -1, false, /*bIgnoreOwnUnitType*/ true, /*bStackOfDoom*/ true, /*iMaxPath*/ 5))
	{
		return;
	}

//	if (AI_group(UNITAI_PILLAGE, /*iMaxGroup*/ 1, -1, -1, false, /*bIgnoreOwnUnitType*/ true, false, /*iMaxPath*/ 4))
//	{
//		return;
//	}

//	if (AI_pillageRange(2))
//	{
//		return;
//	}

	if (AI_guardCity(false, true, 3))
	{
		return;
	}

	if (AI_heal())
	{
		return;
	}

	if (AI_offensiveAirlift())
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


void CvUnitAI::AI_cityDefenseMove()
{
	PROFILE_FUNC();
	
	
	
#ifdef NO_WAY_THIS_IS_DEFINED
	// debugging
	if (true)
	{
		CvSelectionGroup* pThisGroup = getGroup();
		int iCount = pThisGroup->getNumUnits();
		CvUnit* pHeadUnit = pThisGroup->getHeadUnit();
		CvCity* pCity = plot()->getPlotCity();

		if (pThisGroup == NULL)
			iCount *= 1;
	}
#endif

	if (AI_leaveAttack(1, 70, 175))
	{
		return;
	}

	if (AI_chokeDefend())
	{
		return;
	}

	if (AI_guardCityBestDefender())
	{
		return;
	}

	if (AI_guardCity(true))
	{
		return;
	}
	

	if (AI_group(UNITAI_SETTLE, /*iMaxGroup*/ 1, -1, -1, false, false, false, /*iMaxPath*/ 2, /*bAllowRegrouping*/ true))
	{
		return;
	}

	if (AI_group(UNITAI_SETTLE, /*iMaxGroup*/ 2, -1, -1, false, false, false, /*iMaxPath*/ 2, /*bAllowRegrouping*/ true))
	{
		return;
	}

	if (plot()->getOwnerINLINE() == getOwnerINLINE())
	{
		if (AI_load(UNITAI_SETTLER_SEA, MISSIONAI_LOAD_SETTLER, UNITAI_SETTLE, -1, -1, 1, -1, MOVE_SAFE_TERRITORY))
		{
			return;
		}
	}

	if (AI_guardCityAirlift())
	{
		return;
	}

	if (AI_guardCity(false, true, 1))
	{
		return;
	}

	if (plot()->getOwnerINLINE() == getOwnerINLINE())
	{
		if (AI_load(UNITAI_SETTLER_SEA, MISSIONAI_LOAD_SETTLER, UNITAI_SETTLE, 3, -1, -1, -1, MOVE_SAFE_TERRITORY))
		{
			// does this ever occur? the previous settler check is less strict, this one should never be true (I think)
			FAssertMsg(false, "unexpected settler load (non-fatal)");
			return;
		}
	}

	if (AI_guardCity(false, true))
	{
		return;
	}

//    if (AI_group(UNITAI_ATTACK_CITY, -1, 3, -1, /*bIgnoreFaster*/ true))
//	{
//		return;
//	}

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


void CvUnitAI::AI_cityDefenseExtraMove()
{
	PROFILE_FUNC();

	CvCity* pCity;

	if (AI_leaveAttack(2, 55, 150))
	{
		return;
	}
	
	if (AI_chokeDefend())
	{
		return;
	}

	if (AI_guardCityBestDefender())
	{
		return;
	}

	if (AI_guardCity(true))
	{
		return;
	}

	if (AI_group(UNITAI_SETTLE, /*iMaxGroup*/ 1, -1, -1, false, false, false, /*iMaxPath*/ 2, /*bAllowRegrouping*/ true))
	{
		return;
	}

	if (AI_group(UNITAI_SETTLE, /*iMaxGroup*/ 2, -1, -1, false, false, false, /*iMaxPath*/ 2, /*bAllowRegrouping*/ true))
	{
		return;
	}

	pCity = plot()->getPlotCity();

	if ((pCity != NULL) && (pCity->getOwnerINLINE() == getOwnerINLINE())) // XXX check for other team?
	{
		if (plot()->plotCount(PUF_canDefendGroupHead, -1, -1, getOwnerINLINE(), NO_TEAM, PUF_isUnitAIType, AI_getUnitAIType()) == 1)
		{
			getGroup()->pushMission(MISSION_SKIP);
			return;
		}
	}

	if (AI_guardCityAirlift())
	{
		return;
	}

	if (AI_guardCity(false, true, 1))
	{
		return;
	}

	if (plot()->getOwnerINLINE() == getOwnerINLINE())
	{
		if (AI_load(UNITAI_SETTLER_SEA, MISSIONAI_LOAD_SETTLER, UNITAI_SETTLE, 3, -1, -1, -1, MOVE_SAFE_TERRITORY, 3))
		{
			return;
		}
	}

	if (AI_guardCity(false, true))
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


void CvUnitAI::AI_exploreMove()
{
	PROFILE_FUNC();
	
	bool bCanAttackOrPillage = (!isHuman() || GC.getGameINLINE().isGameMultiPlayer());
	
	if (bCanAttackOrPillage)
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

	if (AI_heal())
	{
		return;
	}

	if (bCanAttackOrPillage)
	{
		if (AI_pillageRange(1))
		{
			return;
		}

		if (AI_cityAttack(3, 80))
		{
			return;
		}
	}

	if (AI_goody(4))
	{
		return;
	}

	if (AI_exploreRange(3))
	{
		return;
	}

	if (bCanAttackOrPillage)
	{
		if (AI_pillageRange(3))
		{
			return;
		}
	}

	if (AI_explore())
	{
		return;
	}

	if (bCanAttackOrPillage)
	{
		if (AI_pillage())
		{
			return;
		}
	}

	if (!isHuman())
	{
		if (AI_travelToUpgradeCity())
		{
			return;
		}
	}

	if (AI_fogbust())
	{
		return;
	}

	if (!isHuman() && (AI_getUnitAIType() == UNITAI_EXPLORE))
	{
		if (GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(area(), UNITAI_EXPLORE) > GET_PLAYER(getOwnerINLINE()).AI_neededExplorers(area()))
		{
			if (GET_PLAYER(getOwnerINLINE()).calculateUnitCost() > 0)
			{
				scrap();
				return;
			}
		}
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


void CvUnitAI::AI_missionaryMove()
{
	PROFILE_FUNC();

	if (AI_spreadReligion())
	{
		return;
	}

	if (AI_load(UNITAI_MISSIONARY_SEA, MISSIONAI_LOAD_SPECIAL, NO_UNITAI, -1, -1, -1, 0, MOVE_SAFE_TERRITORY))
	{
		return;
	}

	if (AI_load(UNITAI_MISSIONARY_SEA, MISSIONAI_LOAD_SPECIAL, NO_UNITAI, -1, -1, -1, 0, MOVE_NO_ENEMY_TERRITORY))
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


void CvUnitAI::AI_prophetMove()
{
	PROFILE_FUNC();

	if (AI_construct(1))
	{
		return;
	}

	if (AI_discover(true, true))
	{
		return;
	}

	if (AI_construct(3))
	{
		return;
	}
	
	int iGoldenAgeValue = (GET_PLAYER(getOwnerINLINE()).AI_calculateGoldenAgeValue() / (GET_PLAYER(getOwnerINLINE()).unitsRequiredForGoldenAge()));
	int iDiscoverValue = max(1, getDiscoverResearch(NO_TECH));

	if (((iGoldenAgeValue * 100) / iDiscoverValue) > 60)
	{
        if (AI_goldenAge())
        {
            return;
        }

        if (iDiscoverValue > iGoldenAgeValue)
        {
            if (AI_discover())
            {
                return;
            }
            if (GET_PLAYER(getOwnerINLINE()).getUnitClassCount(getUnitClassType()) > 1)
            {
                if (AI_join())
                {
                    return;
                }
            }
        }
	}
	else
	{
		if (AI_discover())
		{
			return;
		}

		if (AI_join())
		{
			return;
		}
	}

	if ((GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot(), 2) > 0) ||
		  (getGameTurnCreated() < (GC.getGameINLINE().getGameTurn() - 25)))
	{
		if (AI_discover())
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


void CvUnitAI::AI_artistMove()
{
	PROFILE_FUNC();
	
	if (AI_artistCultureVictoryMove())
	{
	    return;
	}

	if (AI_discover(true, true))
	{
		return;
	}

	if (AI_greatWork())
	{
		return;
	}

	int iGoldenAgeValue = (GET_PLAYER(getOwnerINLINE()).AI_calculateGoldenAgeValue() / (GET_PLAYER(getOwnerINLINE()).unitsRequiredForGoldenAge()));
	int iDiscoverValue = max(1, getDiscoverResearch(NO_TECH));

	if (((iGoldenAgeValue * 100) / iDiscoverValue) > 60)
	{
        if (AI_goldenAge())
        {
            return;
        }

        if (iDiscoverValue > iGoldenAgeValue)
        {
            if (AI_discover())
            {
                return;
            }
            if (GET_PLAYER(getOwnerINLINE()).getUnitClassCount(getUnitClassType()) > 1)
            {
                if (AI_join())
                {
                    return;
                }
            }
        }
	}
	else
	{
		if (AI_discover())
		{
			return;
		}

		if (AI_join())
		{
			return;
		}
	}

	if ((GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot(), 2) > 0) ||
		  (getGameTurnCreated() < (GC.getGameINLINE().getGameTurn() - 25)))
	{
		if (AI_discover())
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


void CvUnitAI::AI_scientistMove()
{
	PROFILE_FUNC();

	if (AI_discover(true, true))
	{
		return;
	}

	if (AI_construct(1))
	{
		return;
	}
	if (GET_PLAYER(getOwnerINLINE()).getCurrentEra() < 3)
	{
		if (AI_join(2))
		{
			return;
		}		
	}
	
	if (GET_PLAYER(getOwnerINLINE()).getCurrentEra() <= (GC.getNumEraInfos() / 2))
	{
		if (AI_construct())
		{
			return;
		}
	}

	int iGoldenAgeValue = (GET_PLAYER(getOwnerINLINE()).AI_calculateGoldenAgeValue() / (GET_PLAYER(getOwnerINLINE()).unitsRequiredForGoldenAge()));
	int iDiscoverValue = max(1, getDiscoverResearch(NO_TECH));

	if (((iGoldenAgeValue * 100) / iDiscoverValue) > 60)
	{
        if (AI_goldenAge())
        {
            return;
        }

        if (iDiscoverValue > iGoldenAgeValue)
        {
            if (AI_discover())
            {
                return;
            }
            if (GET_PLAYER(getOwnerINLINE()).getUnitClassCount(getUnitClassType()) > 1)
            {
                if (AI_join())
                {
                    return;
                }
            }
        }
	}
	else
	{
		if (AI_discover())
		{
			return;
		}

		if (AI_join())
		{
			return;
		}
	}

	if ((GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot(), 2) > 0) ||
		  (getGameTurnCreated() < (GC.getGameINLINE().getGameTurn() - 25)))
	{
		if (AI_discover())
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


void CvUnitAI::AI_generalMove()
{
	PROFILE_FUNC();

	std::vector<UnitAITypes> aeUnitAITypes;
	int iDanger = GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot(), 2);
	
	bool bOffenseWar = (area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE);
	
	
	if (iDanger > 0)
	{
		aeUnitAITypes.clear();
		aeUnitAITypes.push_back(UNITAI_ATTACK);
		aeUnitAITypes.push_back(UNITAI_COUNTER);
		if (AI_lead(aeUnitAITypes))
		{
			return;
		}
	}
	
	if (AI_construct(1))
	{
		return;
	}
	if (AI_join(1))
	{
		return;
	}
	
	if (bOffenseWar && (GC.getGameINLINE().getSorenRandNum(2, "AI General Lead") == 0))
	{
		aeUnitAITypes.clear();
		aeUnitAITypes.push_back(UNITAI_ATTACK_CITY);
		if (AI_lead(aeUnitAITypes))
		{
			return;
		}
	}
	
	
	if (AI_join(2))
	{
		return;
	}
	
	if (AI_construct(2))
	{
		return;
	}
	if (AI_join(4))
	{
		return;
	}
	
	if (GC.getGameINLINE().getSorenRandNum(3, "AI General Construct") == 0)
	{
		if (AI_construct())
		{
			return;
		}
	}

	if (AI_join())
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


void CvUnitAI::AI_merchantMove()
{
	PROFILE_FUNC();

	if (AI_discover(true, true))
	{
		return;
	}

	int iGoldenAgeValue = (GET_PLAYER(getOwnerINLINE()).AI_calculateGoldenAgeValue() / (GET_PLAYER(getOwnerINLINE()).unitsRequiredForGoldenAge()));
	int iDiscoverValue = max(1, getDiscoverResearch(NO_TECH));

	if (AI_trade(iGoldenAgeValue * 2))
	{
	    return;
	}

	if (((iGoldenAgeValue * 100) / iDiscoverValue) > 60)
	{
        if (AI_goldenAge())
        {
            return;
        }

        if (AI_trade(iGoldenAgeValue))
        {
            return;
        }

        if (iDiscoverValue > iGoldenAgeValue)
        {
            if (AI_discover())
            {
                return;
            }
            if (GET_PLAYER(getOwnerINLINE()).getUnitClassCount(getUnitClassType()) > 1)
            {
                if (AI_join())
                {
                    return;
                }
            }
        }
	}
	else
	{
		if (AI_discover())
		{
			return;
		}

		if (AI_join())
		{
			return;
		}
	}

	if ((GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot(), 2) > 0) ||
		  (getGameTurnCreated() < (GC.getGameINLINE().getGameTurn() - 25)))
	{
		if (AI_discover())
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


void CvUnitAI::AI_engineerMove()
{
	PROFILE_FUNC();

	if (AI_switchHurry())
	{
		return;
	}

	if (AI_hurry())
	{
		return;
	}

	if (AI_discover(true, true))
	{
		return;
	}

	int iGoldenAgeValue = (GET_PLAYER(getOwnerINLINE()).AI_calculateGoldenAgeValue() / (GET_PLAYER(getOwnerINLINE()).unitsRequiredForGoldenAge()));
	int iDiscoverValue = max(1, getDiscoverResearch(NO_TECH));

	if (((iGoldenAgeValue * 100) / iDiscoverValue) > 60)
	{
        if (AI_goldenAge())
        {
            return;
        }

        if (iDiscoverValue > iGoldenAgeValue)
        {
            if (AI_discover())
            {
                return;
            }
            if (GET_PLAYER(getOwnerINLINE()).getUnitClassCount(getUnitClassType()) > 1)
            {
                if (AI_join())
                {
                    return;
                }
            }
        }
	}
	else
	{
		if (AI_discover())
		{
			return;
		}

		if (AI_join())
		{
			return;
		}
	}

	if ((GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot(), 2) > 0) ||
		  (getGameTurnCreated() < (GC.getGameINLINE().getGameTurn() - 25)))
	{
		if (AI_discover())
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


void CvUnitAI::AI_spyMove()
{
	if (GET_PLAYER(getOwnerINLINE()).getGold() > ((GET_PLAYER(getOwnerINLINE()).AI_goldTarget() * 2) / 3))
	{
		if (GET_PLAYER(getOwnerINLINE()).AI_totalMissionAIs(MISSIONAI_ATTACK_SPY, getGroup()) == 0)
		{
			if (AI_sabotageSpy())
			{
				return;
			}

			if (AI_destroySpy())
			{
				return;
			}

			if (AI_load(UNITAI_SPY_SEA, MISSIONAI_ATTACK_SPY, NO_UNITAI, -1, -1, -1, 0, MOVE_NO_ENEMY_TERRITORY))
			{
				return;
			}

			if (AI_pickupTargetSpy())
			{
				return;
			}
		}
	}

	if (AI_guardSpy())
	{
		return;
	}

	if (GET_PLAYER(getOwnerINLINE()).getGold() > GET_PLAYER(getOwnerINLINE()).AI_goldTarget())
	{
		if (AI_sabotageSpy())
		{
			return;
		}

		if (AI_destroySpy())
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


void CvUnitAI::AI_ICBMMove()
{
	if (AI_nuke())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_workerSeaMove()
{
	PROFILE_FUNC();

	CvCity* pCity;
	
	int iI;

	if (!(getGroup()->canDefend()))
	{
		if (GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot()) > 0)
		{
			if (AI_retreatToCity())
			{
				return;
			}
		}
	}

	if (AI_improveBonus(20))
	{
		return;
	}

	if (AI_improveBonus(10))
	{
		return;
	}

	if (AI_improveBonus())
	{
		return;
	}
	
	if (isHuman())
	{
		FAssert(isAutomated());
		if (plot()->getBonusType() != NO_BONUS)
		{
			if ((plot()->getOwner() == getOwner()) || (!plot()->isOwned()))
			{
				getGroup()->pushMission(MISSION_SKIP);
				return;
			}
		}
		
		for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
		{
			CvPlot* pLoopPlot = plotDirection(getX_INLINE(), getY_INLINE(), (DirectionTypes)iI);
			if (pLoopPlot != NULL)
			{
				if (pLoopPlot->getBonusType() != NO_BONUS)
				{
					if (pLoopPlot->isValidDomainForLocation(getDomainType()))
					{
						getGroup()->pushMission(MISSION_SKIP);
						return;						
					}					
				}
			}
		}
	}
	

	if (!(isHuman()) && (AI_getUnitAIType() == UNITAI_WORKER_SEA))
	{
		pCity = plot()->getPlotCity();

		if (pCity != NULL)
		{
			if (pCity->getOwnerINLINE() == getOwnerINLINE())
			{
				if (pCity->AI_neededSeaWorkers() == 0)
				{
					if (GC.getGameINLINE().getElapsedGameTurns() > 10)
					{
						if (GET_PLAYER(getOwnerINLINE()).calculateUnitCost() > 0)
						{
							scrap();
							return;
						}
					}
				}
			}
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


void CvUnitAI::AI_barbAttackSeaMove()
{
	PROFILE_FUNC();

	if (GC.getGameINLINE().getSorenRandNum(2, "AI Barb") == 0)
	{
		if (AI_pillageRange(1))
		{
			return;
		}
	}

	if (AI_anyAttack(2, 25))
	{
		return;
	}

	if (AI_pillageRange(4))
	{
		return;
	}

	if (AI_heal())
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


void CvUnitAI::AI_attackSeaMove()
{
	PROFILE_FUNC();

	CvArea* pWaterArea;

	if (AI_heal(30, 1))
	{
		return;
	}

	if (AI_anyAttack(2, 40))
	{
		return;
	}

	if (AI_heal(50, 3))
	{
		return;
	}

	if (AI_pillageRange(4))
	{
		return;
	}

	if (AI_heal())
	{
		return;
	}

	if (AI_group(UNITAI_ATTACK_SEA, /*iMaxGroup*/ 1, -1, -1, false, false, false, /*iMaxPath*/ 3))
	{
		return;
	}

	if (AI_blockade())
	{
		return;
	}

	if (AI_pillage())
	{
		return;
	}

	pWaterArea = plot()->waterArea();

	if (pWaterArea != NULL)
	{
		if (pWaterArea->getNumUnrevealedTiles(getTeam()) > 0)
		{
			if (GET_PLAYER(getOwnerINLINE()).AI_areaMissionAIs(pWaterArea, MISSIONAI_EXPLORE, getGroup()) < (GET_PLAYER(getOwnerINLINE()).AI_neededExplorers(pWaterArea) + 1))
			{
				if (AI_exploreRange(4))
				{
					return;
				}

				if (AI_explore())
				{
					return;
				}
			}
		}
	}

	if (AI_protect(35))
	{
		return;
	}

	if (AI_travelToUpgradeCity())
	{
		return;
	}

	if (AI_fogbust())
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


void CvUnitAI::AI_reserveSeaMove()
{
	PROFILE_FUNC();

	if (AI_guardBonus(10))
	{
		return;
	}

	if (AI_heal(30, 1))
	{
		return;
	}

	if (AI_anyAttack(1, 55))
	{
		return;
	}

	if (AI_heal(50, 3))
	{
		return;
	}

	if (AI_anyAttack(3, 45))
	{
		return;
	}

	if (AI_heal())
	{
		return;
	}

	if (!isNeverInvisible())
	{
		if (AI_anyAttack(5, 35))
		{
			return;
		}
	}

	if (AI_protect(40))
	{
		return;
	}

	if (AI_travelToUpgradeCity())
	{
		return;
	}

	if (AI_fogbust())
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


void CvUnitAI::AI_escortSeaMove()
{
	PROFILE_FUNC();

	// if we have cargo, possibly convert to UNITAI_ASSAULT_SEA (this will most often happen with galleons)
	// note, this should not happen when we are not the group head, so escort galleons are fine joining a group, just not as head
	if (hasCargo() && (getUnitAICargo(UNITAI_ATTACK_CITY) > 0 || getUnitAICargo(UNITAI_ATTACK) > 0))
	{
		// non-zero AI_unitValue means that UNITAI_ASSAULT_SEA is valid for this unit (that is the check used everywhere)
		if (GET_PLAYER(getOwnerINLINE()).AI_unitValue(getUnitType(), UNITAI_ASSAULT_SEA, NULL) > 0)
		{
			// save old group, so we can merge it back in
			CvSelectionGroup* pOldGroup = getGroup();

			// this will remove this unit from the current group
			AI_setUnitAIType(UNITAI_ASSAULT_SEA);

			// merge back the rest of the group into the new group
			CvSelectionGroup* pNewGroup = getGroup();
			if (pOldGroup != pNewGroup)
			{
				pOldGroup->mergeIntoGroup(pNewGroup);
			}

			// perform assault sea action
			AI_assaultSeaMove();
			return;
		}
	}
	
	if (AI_heal(30, 1))
	{
		return;
	}

	if (AI_anyAttack(1, 55))
	{
		return;
	}

	if (AI_group(UNITAI_ASSAULT_SEA, -1, /*iMaxOwnUnitAI*/ 0, -1, /*bIgnoreFaster*/ true, false, false, /*iMaxPath*/ 3))
	{
		return;
	}

	if (AI_heal(50, 3))
	{
		return;
	}

	if (AI_pillageRange(2))
	{
		return;
	}

	if (AI_group(UNITAI_CARRIER_SEA, -1, /*iMaxOwnUnitAI*/ 0, -1, /*bIgnoreFaster*/ true))
	{
		return;
	}

	if (AI_group(UNITAI_ASSAULT_SEA, -1, /*iMaxOwnUnitAI*/ 2, /*iMinUnitAI*/ 3, /*bIgnoreFaster*/ true))
	{
		return;
	}

	if (AI_group(UNITAI_ASSAULT_SEA, -1, /*iMaxOwnUnitAI*/ 1, /*iMinUnitAI*/ 2, /*bIgnoreFaster*/ true))
	{
		return;
	}

	if (AI_group(UNITAI_ASSAULT_SEA, -1, /*iMaxOwnUnitAI*/ 0, /*iMinUnitAI*/ 1, /*bIgnoreFaster*/ true))
	{
		return;
	}

	if (AI_group(UNITAI_ASSAULT_SEA, -1, /*iMaxOwnUnitAI*/ 5, /*iMinUnitAI*/ 3, /*bIgnoreFaster*/ true))
	{
		return;
	}

	if (AI_group(UNITAI_ASSAULT_SEA, -1, /*iMaxOwnUnitAI*/ 3, /*iMinUnitAI*/ 2, /*bIgnoreFaster*/ true))
	{
		return;
	}

	if (AI_group(UNITAI_ASSAULT_SEA, -1, /*iMaxOwnUnitAI*/ 1, /*iMinUnitAI*/ 1, /*bIgnoreFaster*/ true))
	{
		return;
	}

	if (AI_group(UNITAI_CARRIER_SEA, -1, /*iMaxOwnUnitAI*/ 3, /*iMinUnitAI*/ 2, /*bIgnoreFaster*/ true))
	{
		return;
	}

	if (AI_group(UNITAI_CARRIER_SEA, -1, /*iMaxOwnUnitAI*/ 1, -1, /*bIgnoreFaster*/ true))
	{
		return;
	}

	if (AI_heal())
	{
		return;
	}

	if (AI_travelToUpgradeCity())
	{
		return;
	}

	if (AI_fogbust())
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

	bool bCanAttackOrPillage = (!isHuman() || GC.getGameINLINE().isGameMultiPlayer());
		
	if (bCanAttackOrPillage)
	{
		if (AI_anyAttack(1, 70))
		{
			return;
		}
	}

	if (AI_heal())
	{
		return;
	}

	if (bCanAttackOrPillage)
	{
		if (AI_pillageRange(1))
		{
			return;
		}
	}
	
	if (AI_exploreRange(4))
	{
		return;
	}

	if (bCanAttackOrPillage)
	{
		if (AI_pillageRange(4))
		{
			return;
		}
	}

	if (AI_explore())
	{
		return;
	}

	if (bCanAttackOrPillage)
	{
		if (AI_pillage())
		{
			return;
		}
	}

	if (!isHuman())
	{
		if (AI_travelToUpgradeCity())
		{
			return;
		}
	}

	if (AI_fogbust())
	{
		return;
	}

	if (!isHuman() && (AI_getUnitAIType() == UNITAI_EXPLORE_SEA))
	{
		CvArea* pWaterArea = plot()->waterArea();

		if (pWaterArea != NULL)
		{
			if (GET_PLAYER(getOwnerINLINE()).AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_EXPLORE_SEA) > GET_PLAYER(getOwnerINLINE()).AI_neededExplorers(pWaterArea))
			{
				if (GET_PLAYER(getOwnerINLINE()).calculateUnitCost() > 0)
				{
					scrap();
					return;
				}
			}
		}
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


void CvUnitAI::AI_assaultSeaMove()
{
	PROFILE_FUNC();

	FAssert(AI_getUnitAIType() == UNITAI_ASSAULT_SEA);

	bool bLandWar = false;
	bool bIsCity = plot()->isCity();
	if (bIsCity)
	{
		AreaAITypes eAreaAIType = area()->getAreaAIType(getTeam());
		bLandWar = ((eAreaAIType == AREAAI_OFFENSIVE) || (eAreaAIType == AREAAI_DEFENSIVE) || (eAreaAIType == AREAAI_MASSING));

		const int iMissionAICount = 5;
		MissionAITypes aeMissionAI[iMissionAICount] = 
			{MISSIONAI_GROUP, MISSIONAI_LOAD_ASSAULT, MISSIONAI_LOAD_SETTLER, MISSIONAI_LOAD_SPECIAL, MISSIONAI_ATTACK_SPY};
		
		if (GET_PLAYER(getOwnerINLINE()).AI_unitTargetMissionAIs(this, aeMissionAI, iMissionAICount) > 0)
		{
			// if we are at/preparing land war, and we have cargo, unload, even if full, since we are waiting for some unit to get here
			if (bLandWar && getGroup()->hasCargo())
			{
				getGroup()->unloadAll();
			}
			
			getGroup()->pushMission(MISSION_SKIP);
			return;
		}
	}

	if (getGroup()->isFull() || !(getGroup()->hasCargo()))
	{
		if (getGroup()->isFull())
		{
			if (AI_assaultSeaTransport())
			{
				return;
			}
		}
		else
		{
			if (AI_pillageRange(2))
			{
				return;
			}
		}

		if (AI_group(UNITAI_ASSAULT_SEA, -1, /*iMaxOwnUnitAI*/ 2))
		{
			return;
		}

		if (AI_group(UNITAI_ASSAULT_SEA, -1, /*iMaxOwnUnitAI*/ 1))
		{
			FAssertMsg(false, "somehow there was a group that didnt fit max of 2, but max of 1 was ok");
			return;
		}
	}
	
	// if we just made a dropoff, bombard the city if we can
	if (AI_seaBombardRange(2))
	{
		return;
	}

	// if we are in a city, and at/preparing land war, and we have cargo, unload, even if full, since AI_assaultSeaTransport already had its chance
	if (bIsCity && bLandWar && getGroup()->hasCargo())
	{
		getGroup()->unloadAll();
		getGroup()->pushMission(MISSION_SKIP);
		return;
	}


	if (AI_pickup(UNITAI_ATTACK_CITY))
	{
		return;
	}

	if (AI_pickup(UNITAI_ATTACK))
	{
		return;
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


void CvUnitAI::AI_settlerSeaMove()
{
	PROFILE_FUNC();

	if ((isFull()) ||
			((getUnitAICargo(UNITAI_CITY_DEFENSE) > 0) &&
			 (getUnitAICargo(UNITAI_WORKER) > 0) &&
			 (GET_PLAYER(getOwnerINLINE()).AI_unitTargetMissionAIs(this, MISSIONAI_LOAD_SETTLER) == 0)))
	{
		if (AI_settlerSeaTransport())
		{
			return;
		}
	}
	else if (!(getGroup()->hasCargo()))
	{
		if (AI_pillageRange(3))
		{
			return;
		}
	}

	if (AI_pickup(UNITAI_SETTLE))
	{
		return;
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


void CvUnitAI::AI_missionarySeaMove()
{
	PROFILE_FUNC();

	if (getUnitAICargo(UNITAI_MISSIONARY) > 0)
	{
		if (AI_specialSeaTransportMissionary())
		{
			return;
		}
	}
	else if (!(getGroup()->hasCargo()))
	{
		if (AI_pillageRange(4))
		{
			return;
		}
	}

	if (AI_pickup(UNITAI_MISSIONARY))
	{
		return;
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


void CvUnitAI::AI_spySeaMove()
{
	PROFILE_FUNC();

	CvCity* pCity;

	if (getUnitAICargo(UNITAI_SPY) > 0)
	{
		if (AI_specialSeaTransportSpy())
		{
			return;
		}

		pCity = plot()->getPlotCity();

		if (pCity != NULL)
		{
			if (pCity->getOwnerINLINE() == getOwnerINLINE())
			{
				getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_ATTACK_SPY, pCity->plot());
				return;
			}
		}
	}
	else if (!(getGroup()->hasCargo()))
	{
		if (AI_pillageRange(5))
		{
			return;
		}
	}

	if (AI_pickup(UNITAI_SPY))
	{
		return;
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


void CvUnitAI::AI_carrierSeaMove()
{
	if (AI_heal(50))
	{
		return;
	}

	if (!atWar(getTeam(), plot()->getTeam()))
	{
		if (GET_PLAYER(getOwnerINLINE()).AI_unitTargetMissionAIs(this, MISSIONAI_GROUP) > 0)
		{
			getGroup()->pushMission(MISSION_SKIP);
			return;
		}
	}

	if (getCargo() > 0)
	{
		if (AI_carrierSeaTransport())
		{
			return;
		}

		if (AI_blockade())
		{
			return;
		}

		if (AI_shadow(UNITAI_ASSAULT_SEA))
		{
			return;
		}
	}

	if (AI_group(UNITAI_CARRIER_SEA, -1, /*iMaxOwnUnitAI*/ 1))
	{
		return;
	}

	if (AI_travelToUpgradeCity())
	{
		return;
	}

	if (AI_fogbust())
	{
		return;
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


void CvUnitAI::AI_attackAirMove()
{
	CvCity* pCity;

	if (getDamage() > 0)
	{
		getGroup()->pushMission(MISSION_SKIP);
		return;
	}

	pCity = plot()->getPlotCity();

	if (pCity != NULL)
	{
		if (pCity->AI_isDanger())
		{
			if (!(pCity->AI_isDefended()))
			{
				if (AI_airOffensiveCity())
				{
					return;
				}
			}
		}
	}

	if (GC.getGameINLINE().getSorenRandNum(5, "AI Air Attack Move") == 0)
	{
		if (AI_airBomb())
		{
			return;
		}
	}

	if (AI_airStrike())
	{
		return;
	}

	if (AI_airBomb())
	{
		return;
	}

	if (AI_travelToUpgradeCity())
	{
		return;
	}

	if (AI_airFogbust())
	{
		return;
	}

	if (AI_airOffensiveCity())
	{
		return;
	}

	if (canAirDefend())
	{
		getGroup()->pushMission(MISSION_AIRPATROL);
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_defenseAirMove()
{
	CvCity* pCity;

	if (getDamage() > 0)
	{
		getGroup()->pushMission(MISSION_SKIP);
		return;
	}

	if (GC.getGameINLINE().getSorenRandNum(2, "AI Air Defense Move") == 0)
	{
		pCity = plot()->getPlotCity();

		if ((pCity != NULL) && pCity->AI_isDanger())
		{
			if (AI_airStrike())
			{
				return;
			}

			if (AI_airBomb())
			{
				return;
			}
		}
		else
		{
			if (AI_airBomb())
			{
				return;
			}

			if (AI_airStrike())
			{
				return;
			}
		}

		if (AI_travelToUpgradeCity())
		{
			return;
		}

		if (AI_airFogbust())
		{
			return;
		}
	}

	if (AI_airDefensiveCity())
	{
		return;
	}

	if (canAirDefend())
	{
		getGroup()->pushMission(MISSION_AIRPATROL);
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_carrierAirMove()
{
	// XXX maybe protect land troops?

	if (getDamage() > 0)
	{
		getGroup()->pushMission(MISSION_SKIP);
		return;
	}

	if (isCargo())
	{
		if (GC.getGameINLINE().getSorenRandNum(2, "AI Air Carrier Move") == 0)
		{
			if (AI_airStrike())
			{
				return;
			}

			if (AI_airBomb())
			{
				return;
			}

			if (AI_airExplore())
			{
				return;
			}

			if (AI_travelToUpgradeCity())
			{
				return;
			}

			if (AI_airFogbust())
			{
				return;
			}
		}
	}

	if (AI_airCarrier())
	{
		return;
	}

	if (AI_airDefensiveCity())
	{
		return;
	}

	if (canAirDefend())
	{
		getGroup()->pushMission(MISSION_AIRPATROL);
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_exploreAirMove()
{
	if (AI_airExplore())
	{
		return;
	}

	if (!isHuman())
	{
		if (AI_travelToUpgradeCity())
		{
			return;
		}
	}

	if (AI_airFogbust())
	{
		return;
	}

	if (canAirDefend())
	{
		getGroup()->pushMission(MISSION_AIRPATROL);
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

	if (AI_improveBonus(20))
	{
		return;
	}

	if (AI_improveBonus(10))
	{
		return;
	}

	if (AI_connectBonus())
	{
		return;
	}

	if (AI_connectCity())
	{
		return;
	}

	if (AI_improveBonus())
	{
		return;
	}

	if (AI_routeTerritory(true))
	{
		return;
	}

	if (AI_connectBonus(false))
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
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


// XXX make sure we include any new UnitAITypes...
int CvUnitAI::AI_promotionValue(PromotionTypes ePromotion)
{
	int iValue;
	int iTemp;
	int iExtra;
	int iI;

	iValue = 0;

	if (GC.getPromotionInfo(ePromotion).isLeader())
	{
		// Don't consume the leader as a regular promotion
		return 0;
	}

	if (GC.getPromotionInfo(ePromotion).isBlitz() && baseMoves() > 1)
	{
		if (AI_getUnitAIType() == UNITAI_RESERVE)
		{
			iValue += 10;
		}
		else
		{
			iValue += 2;
		}
	}

	if (GC.getPromotionInfo(ePromotion).isAmphib())
	{
		if ((AI_getUnitAIType() == UNITAI_ATTACK) ||
			  (AI_getUnitAIType() == UNITAI_ATTACK_CITY))
		{
			iValue += 5;
		}
		else
		{
			iValue++;
		}
	}

	if (GC.getPromotionInfo(ePromotion).isRiver())
	{
		if ((AI_getUnitAIType() == UNITAI_ATTACK) ||
			  (AI_getUnitAIType() == UNITAI_ATTACK_CITY))
		{
			iValue += 5;
		}
		else
		{
			iValue++;
		}
	}

	if (GC.getPromotionInfo(ePromotion).isEnemyRoute())
	{
		if (AI_getUnitAIType() == UNITAI_PILLAGE)
		{
			iValue += 40;
		}
		else if ((AI_getUnitAIType() == UNITAI_ATTACK) ||
			       (AI_getUnitAIType() == UNITAI_ATTACK_CITY))
		{
			iValue += 20;
		}
		else
		{
			iValue += 4;
		}
	}

	if (GC.getPromotionInfo(ePromotion).isAlwaysHeal())
	{
		if ((AI_getUnitAIType() == UNITAI_ATTACK) ||
			  (AI_getUnitAIType() == UNITAI_ATTACK_CITY) ||
				(AI_getUnitAIType() == UNITAI_PILLAGE) ||
				(AI_getUnitAIType() == UNITAI_COUNTER) ||
				(AI_getUnitAIType() == UNITAI_ATTACK_SEA) ||
				(AI_getUnitAIType() == UNITAI_ESCORT_SEA))
		{
			iValue += 10;
		}
		else
		{
			iValue += 8;
		}
	}

	if (GC.getPromotionInfo(ePromotion).isHillsDoubleMove())
	{
		if (AI_getUnitAIType() == UNITAI_EXPLORE)
		{
			iValue += 20;
		}
		else
		{
			iValue += 10;
		}
	}

	if (GC.getPromotionInfo(ePromotion).isImmuneToFirstStrikes() 
		&& !immuneToFirstStrikes())
	{
		if ((AI_getUnitAIType() == UNITAI_ATTACK_CITY))
		{
			iValue += 12;			
		}
		else if ((AI_getUnitAIType() == UNITAI_ATTACK))
		{
			iValue += 8;
		}
		else
		{
			iValue += 4;
		}
	}

	iTemp = GC.getPromotionInfo(ePromotion).getVisibilityChange();
	if ((AI_getUnitAIType() == UNITAI_EXPLORE_SEA) || 
		(AI_getUnitAIType() == UNITAI_EXPLORE))
	{
		iValue += (iTemp * 40);
	}
	else
	{
		iValue += (iTemp * 1);
	}

	iTemp = GC.getPromotionInfo(ePromotion).getMovesChange();
	if ((AI_getUnitAIType() == UNITAI_ATTACK_SEA) ||
		  (AI_getUnitAIType() == UNITAI_RESERVE_SEA) ||
		  (AI_getUnitAIType() == UNITAI_ESCORT_SEA) ||
			(AI_getUnitAIType() == UNITAI_EXPLORE_SEA) ||
			(AI_getUnitAIType() == UNITAI_ASSAULT_SEA) ||
			(AI_getUnitAIType() == UNITAI_SETTLER_SEA) ||
			(AI_getUnitAIType() == UNITAI_PILLAGE) ||
			(AI_getUnitAIType() == UNITAI_ATTACK))
	{
		iValue += (iTemp * 20);
	}
	else
	{
		iValue += (iTemp * 4);
	}

	iTemp = GC.getPromotionInfo(ePromotion).getMoveDiscountChange();
	if (AI_getUnitAIType() == UNITAI_PILLAGE)
	{
		iValue += (iTemp * 10);
	}
	else
	{
		iValue += (iTemp * 2);
	}

	iTemp = GC.getPromotionInfo(ePromotion).getFirstStrikesChange() * 2;
	iTemp += GC.getPromotionInfo(ePromotion).getChanceFirstStrikesChange();
	if ((AI_getUnitAIType() == UNITAI_RESERVE) ||
		  (AI_getUnitAIType() == UNITAI_COUNTER) ||
			(AI_getUnitAIType() == UNITAI_CITY_DEFENSE) ||
			(AI_getUnitAIType() == UNITAI_CITY_COUNTER) ||
			(AI_getUnitAIType() == UNITAI_CITY_SPECIAL) ||
			(AI_getUnitAIType() == UNITAI_ATTACK))
	{
		iTemp *= 8;
		iExtra = getExtraChanceFirstStrikes() + getExtraFirstStrikes() * 2;
		iTemp *= 100 + iExtra * 15;
		iTemp /= 100;
	}
	else
	{
		iValue += (iTemp * 5);
	}


	iTemp = GC.getPromotionInfo(ePromotion).getWithdrawalChange();
	if (iTemp != 0)
	{
		iExtra = (GC.getUnitInfo(getUnitType()).getWithdrawalProbability() + (getExtraWithdrawal() * 4));
		iTemp *= (100 + iExtra);
		iTemp /= 100;
		if ((AI_getUnitAIType() == UNITAI_ATTACK_CITY))
		{
			iValue += (iTemp * 4) / 3;
		}
		else if ((AI_getUnitAIType() == UNITAI_COLLATERAL) ||
			  (AI_getUnitAIType() == UNITAI_RESERVE) ||
			  (AI_getUnitAIType() == UNITAI_RESERVE_SEA) ||
			  getLeaderUnitType() != NO_UNIT)
		{
			iValue += iTemp * 1;
		}
		else
		{
			iValue += (iTemp / 4);
		}
	}

	iTemp = GC.getPromotionInfo(ePromotion).getCollateralDamageChange();
	if (iTemp != 0)
	{
		iExtra = (getExtraCollateralDamage());//collateral has no strong synergy (not like retreat)
		iTemp *= (100 + iExtra);
		iTemp /= 100;
		
		if (AI_getUnitAIType() == UNITAI_COLLATERAL)
		{
			iValue += (iTemp * 1);
		}
		else if (AI_getUnitAIType() == UNITAI_ATTACK_CITY)
		{
			iValue += ((iTemp * 2) / 3);
		}
		else
		{
			iValue += (iTemp / 8);
		}
	}

	iTemp = GC.getPromotionInfo(ePromotion).getBombardRateChange();
	if (AI_getUnitAIType() == UNITAI_ATTACK_CITY)
	{
		iValue += (iTemp * 1);
	}
	else
	{
		iValue += (iTemp / 8);
	}

	iTemp = GC.getPromotionInfo(ePromotion).getEnemyHealChange();	
	if ((AI_getUnitAIType() == UNITAI_ATTACK) ||
		  (AI_getUnitAIType() == UNITAI_ATTACK_SEA))
	{
		iValue += (iTemp / 4);
	}
	else
	{
		iValue += (iTemp / 8);
	}

	iTemp = GC.getPromotionInfo(ePromotion).getNeutralHealChange();
	iValue += (iTemp / 8);

	iTemp = GC.getPromotionInfo(ePromotion).getFriendlyHealChange();
	if ((AI_getUnitAIType() == UNITAI_CITY_DEFENSE) ||
		  (AI_getUnitAIType() == UNITAI_CITY_COUNTER) ||
		  (AI_getUnitAIType() == UNITAI_CITY_SPECIAL))
	{
		iValue += (iTemp / 4);
	}
	else
	{
		iValue += (iTemp / 8);
	}


    if (getDamage() > 0)
    {
        iTemp = GC.getPromotionInfo(ePromotion).getSameTileHealChange() + getSameTileHeal();
        iExtra = getSameTileHeal();
        
        iTemp *= (100 + iExtra * 5);
        iTemp /= 100;
        
        if (iTemp > 0)
        {
            if (healRate(plot()) < iTemp)
            {
                iValue += iTemp * ((getGroup()->getNumUnits() > 4) ? 4 : 2);
            }
            else
            {
                iValue += (iTemp / 8);
            }
        }

        iTemp = GC.getPromotionInfo(ePromotion).getAdjacentTileHealChange();
        iExtra = getAdjacentTileHeal();
        iTemp *= (100 + iExtra * 5);
        iTemp /= 100;
        if (getSameTileHeal() >= iTemp)
        {
            iValue += (iTemp * ((getGroup()->getNumUnits() > 9) ? 4 : 2));
        }
        else
        {
            iValue += (iTemp / 4);
        }
    }

	iTemp = GC.getPromotionInfo(ePromotion).getCombatPercent();
	if ((AI_getUnitAIType() == UNITAI_ATTACK) ||
		(AI_getUnitAIType() == UNITAI_COUNTER) ||
		(AI_getUnitAIType() == UNITAI_CITY_COUNTER) ||
		  (AI_getUnitAIType() == UNITAI_ATTACK_SEA) ||
		  (AI_getUnitAIType() == UNITAI_RESERVE_SEA) ||
			(AI_getUnitAIType() == UNITAI_ESCORT_SEA) ||
			(AI_getUnitAIType() == UNITAI_CARRIER_SEA))
	{
		iValue += (iTemp * 2);
	}
	else
	{
		iValue += (iTemp * 1);
	}

	iTemp = GC.getPromotionInfo(ePromotion).getCityAttackPercent();
	if (iTemp != 0)
	{
		iExtra = (GC.getUnitInfo(getUnitType()).getCityAttackModifier() + (getExtraCityAttackPercent() * 2));
		iTemp *= (100 + iExtra);
		iTemp /= 100;
		if (AI_getUnitAIType() == UNITAI_ATTACK_CITY)
		{
			iValue += (iTemp * 1);
		}
		else 
		{
			iValue -= iTemp / 4;
		}
	}

	iTemp = GC.getPromotionInfo(ePromotion).getCityDefensePercent();
	if (iTemp != 0)
	{
		if ((AI_getUnitAIType() == UNITAI_CITY_DEFENSE) ||
			  (AI_getUnitAIType() == UNITAI_CITY_SPECIAL))
		{
			iExtra = GC.getUnitInfo(getUnitType()).getCityDefenseModifier() + (getExtraCityDefensePercent() * 2);
			iValue += ((iTemp * (100 + iExtra)) / 100);
		}
		else
		{
			iValue += (iTemp / 4);
		}
	}

	iTemp = GC.getPromotionInfo(ePromotion).getHillsAttackPercent();
	if (iTemp != 0)
	{
		iExtra = getExtraHillsAttackPercent();
		iTemp *= (100 + iExtra * 2);
		iTemp /= 100;
		if ((AI_getUnitAIType() == UNITAI_ATTACK) ||
			(AI_getUnitAIType() == UNITAI_COUNTER))
		{
			iValue += (iTemp / 4);
		}
		else
		{
			iValue += (iTemp / 16);
		}
	}

	iTemp = GC.getPromotionInfo(ePromotion).getHillsDefensePercent();
	if (iTemp != 0)
	{
		iExtra = (GC.getUnitInfo(getUnitType()).getHillsDefenseModifier() + (getExtraHillsDefensePercent() * 2)); 
		iTemp *= (100 + iExtra);
		iTemp /= 100;
		if (AI_getUnitAIType() == UNITAI_CITY_DEFENSE)
		{
			if (plot()->isCity() && plot()->isHills())
			{
				iValue += (iTemp * 4) / 3;
			}
		}
		else if (AI_getUnitAIType() == UNITAI_COUNTER)
		{
			if (plot()->isHills())
			{
				iValue += (iTemp / 4);
			}
			else
			{
				iValue++;
			}
		}
		else
		{
			iValue += (iTemp / 16);
		}
	}

	iTemp = GC.getPromotionInfo(ePromotion).getRevoltProtection();
	if ((AI_getUnitAIType() == UNITAI_CITY_DEFENSE) ||
		(AI_getUnitAIType() == UNITAI_CITY_COUNTER) ||
		(AI_getUnitAIType() == UNITAI_CITY_SPECIAL))
	{
		if (iTemp > 0)
		{
			PlayerTypes eOwner = plot()->calculateCulturalOwner();
			if (eOwner != NO_PLAYER && GET_PLAYER(eOwner).getTeam() != GET_PLAYER(getOwnerINLINE()).getTeam())
			{
				iValue += (iTemp / 2);
			}
		}
	}

	iTemp = GC.getPromotionInfo(ePromotion).getCollateralDamageProtection();
	if ((AI_getUnitAIType() == UNITAI_CITY_DEFENSE) ||
		(AI_getUnitAIType() == UNITAI_CITY_COUNTER) ||
		(AI_getUnitAIType() == UNITAI_CITY_SPECIAL))
	{
		iValue += (iTemp / 3);
	}
	else if ((AI_getUnitAIType() == UNITAI_ATTACK) ||
		(AI_getUnitAIType() == UNITAI_COUNTER))
	{
		iValue += (iTemp / 4);
	}
	else
	{
		iValue += (iTemp / 8);
	}

	iTemp = GC.getPromotionInfo(ePromotion).getPillageChange();
	if (AI_getUnitAIType() == UNITAI_PILLAGE ||
		AI_getUnitAIType() == UNITAI_ATTACK_SEA)
	{
		iValue += (iTemp / 4);
	}
	else
	{
		iValue += (iTemp / 16);
	}

	iTemp = GC.getPromotionInfo(ePromotion).getUpgradeDiscount();
	iValue += (iTemp / 16);

	iTemp = GC.getPromotionInfo(ePromotion).getExperiencePercent();
	if ((AI_getUnitAIType() == UNITAI_ATTACK) ||
		(AI_getUnitAIType() == UNITAI_ATTACK_SEA) ||
		(AI_getUnitAIType() == UNITAI_RESERVE_SEA) ||
		(AI_getUnitAIType() == UNITAI_ESCORT_SEA) ||
		(AI_getUnitAIType() == UNITAI_CARRIER_SEA))
	{
		iValue += (iTemp * 1);
	}
	else
	{
		iValue += (iTemp / 2);
	}

	iTemp = GC.getPromotionInfo(ePromotion).getKamikazePercent();
	if (AI_getUnitAIType() == UNITAI_ATTACK_CITY)
	{
		iValue += (iTemp / 16);
	}
	else
	{
		iValue += (iTemp / 64);
	}

	for (iI = 0; iI < GC.getNumTerrainInfos(); iI++)
	{
		iTemp = GC.getPromotionInfo(ePromotion).getTerrainDefensePercent(iI);
		if (iTemp != 0)
		{
			iExtra =  getExtraTerrainDefensePercent((TerrainTypes)iI);
			iTemp *= (100 + iExtra);
			iTemp /= 100;
			if (AI_getUnitAIType() == UNITAI_COUNTER)
			{
				if (plot()->getTerrainType() == (TerrainTypes)iI)
				{
					iValue += (iTemp / 4);
				}
				else
				{
					iValue++;
				}
			}
			else
			{
				iValue += (iTemp / 16);
			}
		}

		if (GC.getPromotionInfo(ePromotion).getTerrainDoubleMove(iI))
		{
			if (AI_getUnitAIType() == UNITAI_EXPLORE)
			{
				iValue += 20;
			}
			else if ((AI_getUnitAIType() == UNITAI_ATTACK) || (AI_getUnitAIType() == UNITAI_PILLAGE))
			{
				iValue += 10;
			}
			else
			{
			    iValue += 1;
			}
		}
	}

	for (iI = 0; iI < GC.getNumFeatureInfos(); iI++)
	{
		iTemp = GC.getPromotionInfo(ePromotion).getFeatureDefensePercent(iI);;
		if (iTemp != 0)
		{
			iExtra = getExtraFeatureDefensePercent((FeatureTypes)iI);
			iTemp *= (100 + iExtra * 2);
			iTemp /= 100;
			
			if (!noDefensiveBonus())
			{
				if (AI_getUnitAIType() == UNITAI_COUNTER)
				{
					if (plot()->getFeatureType() == (FeatureTypes)iI)
					{
						iValue += (iTemp / 4);
					}
					else
					{
						iValue++;
					}
				}
				else
				{
					iValue += (iTemp / 16);
				}
			}
		}

		if (GC.getPromotionInfo(ePromotion).getFeatureDoubleMove(iI))
		{
			if (AI_getUnitAIType() == UNITAI_EXPLORE)
			{
				iValue += 20;
			}
			else if ((AI_getUnitAIType() == UNITAI_ATTACK) || (AI_getUnitAIType() == UNITAI_PILLAGE))
			{
				iValue += 10;
			}
			else
			{
			    iValue += 1;
			}
		}
	}

    int iOtherCombat = 0; 
    int iSameCombat = 0;
    
    for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
    {
        if ((UnitCombatTypes)iI == getUnitCombatType())
        {
            iSameCombat += unitCombatModifier((UnitCombatTypes)iI);
        }
        else
        {
            iOtherCombat += unitCombatModifier((UnitCombatTypes)iI);
        }
    }

	for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
	{
		iTemp = GC.getPromotionInfo(ePromotion).getUnitCombatModifierPercent(iI);
		int iCombatWeight = 0;
        //Fighting their own kind
        if ((UnitCombatTypes)iI == getUnitCombatType())
        {
            if (iSameCombat >= iOtherCombat)
            {
                iCombatWeight = 70;//"axeman takes formation"
            }
            else
            {
                iCombatWeight = 30;
            }
        }
        else
        {
            //fighting other kinds
            if (unitCombatModifier((UnitCombatTypes)iI) > 10)
            {
                iCombatWeight = 70;//"spearman takes formation"
            }
            else
            {
                iCombatWeight = 30;
            }
        }
		if ((AI_getUnitAIType() == UNITAI_COUNTER) || (AI_getUnitAIType() == UNITAI_CITY_COUNTER))
		{
		    iValue += (iTemp * iCombatWeight) / 75;

		}
		else if ((AI_getUnitAIType() == UNITAI_ATTACK) ||
			       (AI_getUnitAIType() == UNITAI_RESERVE))
		{
			iValue += (iTemp * iCombatWeight) / 100;
		}
		else if ((AI_getUnitAIType() == UNITAI_ATTACK) ||
			       (AI_getUnitAIType() == UNITAI_RESERVE))
		{
			iValue += (iTemp * iCombatWeight) / 150;
		}
		else
		{
			iValue += (iTemp * iCombatWeight) / 400;
		}
	}

	for (iI = 0; iI < NUM_DOMAIN_TYPES; iI++)
	{
		//WTF? why float and cast to int?
		//iTemp = ((int)((GC.getPromotionInfo(ePromotion).getDomainModifierPercent(iI) + getExtraDomainModifier((DomainTypes)iI)) * 100.0f));
		iTemp = GC.getPromotionInfo(ePromotion).getDomainModifierPercent(iI);
		if (AI_getUnitAIType() == UNITAI_COUNTER)
		{
			iValue += (iTemp * 1);
		}
		else if ((AI_getUnitAIType() == UNITAI_ATTACK) ||
			       (AI_getUnitAIType() == UNITAI_RESERVE))
		{
			iValue += (iTemp / 2);
		}
		else
		{
			iValue += (iTemp / 8);
		}
	}

	if (iValue > 0)
	{
		iValue += GC.getGameINLINE().getSorenRandNum(15, "AI Promote");
	}

	return iValue;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_shadow(UnitAITypes eUnitAI)
{
	PROFILE_FUNC();

	CvUnit* pLoopUnit;
	CvUnit* pBestUnit;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iLoop;

	iBestValue = MAX_INT;
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
							if (GET_PLAYER(getOwnerINLINE()).AI_unitTargetMissionAIs(pLoopUnit, MISSIONAI_SHADOW, getGroup()) == 0)
							{
								if (!(pLoopUnit->plot()->isVisibleEnemyUnit(getOwnerINLINE())))
								{
									if (generatePath(pLoopUnit->plot(), 0, true, &iPathTurns))
									{
										//if (iPathTurns <= iMaxPath) XXX
										{
											iValue = iPathTurns;

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


// Returns true if a group was joined or a mission was pushed...
bool CvUnitAI::AI_group(UnitAITypes eUnitAI, int iMaxGroup, int iMaxOwnUnitAI, int iMinUnitAI, 
						bool bIgnoreFaster, bool bIgnoreOwnUnitType, bool bStackOfDoom, int iMaxPath, bool bAllowRegrouping)
{
	PROFILE_FUNC();

	CvUnit* pLoopUnit;
	CvUnit* pBestUnit;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iLoop;
	
	//if we have already joined a group this turn, don't bother.
	if (m_iLastJoinGroupTurn == GC.getGame().getGameTurn())
	{
		return false;
	}

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

	iBestValue = MAX_INT;
	pBestUnit = NULL;

	CvSelectionGroup* pThisGroup = getGroup();
	for(pLoopUnit = GET_PLAYER(getOwnerINLINE()).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER(getOwnerINLINE()).nextUnit(&iLoop))
	{
		if (pLoopUnit != this)
		{
			CvSelectionGroup* pLoopGroup = pLoopUnit->getGroup();
			if (pLoopGroup != pThisGroup)
			{
				CvPlot* pPlot = pLoopUnit->plot();
				if (AI_plotValid(pPlot))
				{
					if (((iMaxPath > 0) || (pPlot == plot())) && (pLoopUnit->isGroupHead()))
					{
						if (!(pLoopUnit->isCargo()))
						{
							if (pLoopUnit->AI_getUnitAIType() == eUnitAI)
							{
								if (!atWar(pPlot->getTeam(), getTeam()))
								{
									// do not attempt to join groups which are healing this turn
									// (healing is cleared every turn for automated groups, so we know we pushed a heal this turn)
									// do not join groups that are guarding cities
									if (pLoopGroup->getActivityType() != ACTIVITY_HEAL && pLoopGroup->AI_getMissionAIType() != MISSIONAI_GUARD_CITY && canJoinGroup(pPlot, pLoopGroup))
									{
										if ((iMaxGroup == -1) || ((pLoopGroup->getNumUnits() + GET_PLAYER(getOwnerINLINE()).AI_unitTargetMissionAIs(pLoopUnit, MISSIONAI_GROUP, getGroup())) <= (iMaxGroup + ((bStackOfDoom) ? AI_stackOfDoomExtra() : 0))))
										{
											if ((iMaxOwnUnitAI == -1) || (pLoopGroup->countNumUnitAIType(AI_getUnitAIType()) <= (iMaxOwnUnitAI + ((bStackOfDoom) ? AI_stackOfDoomExtra() : 0))))
											{
												if ((iMinUnitAI == -1) || (pLoopGroup->countNumUnitAIType(eUnitAI) >= iMinUnitAI))
												{
													if (!bIgnoreFaster || (pLoopGroup->baseMoves() <= baseMoves()))
													{
														if (!bIgnoreOwnUnitType || (pLoopUnit->getUnitType() != getUnitType()))
														{
															if (!(pPlot->isVisibleEnemyUnit(getOwnerINLINE())))
															{
																if (generatePath(pPlot, 0, true, &iPathTurns))
																{
																	if (iPathTurns <= iMaxPath)
																	{
																		iValue = 1000 * (iPathTurns + 1);
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
			}
		}
	}

	if (pBestUnit != NULL)
	{
		if (atPlot(pBestUnit->plot()))
		{
#ifdef DEBUG_AI_UPDATE_GROUPS
			// debugging
			if (true)
			{
				CvSelectionGroup* pBestGroup = pBestUnit->getGroup();
				CvSelectionGroup* pCurrentGroup = getGroup();
				CvWString szUnitName = GC.getUnitInfo(m_eUnitType).getDescription();
				CvWString szBestUnitName = GC.getUnitInfo(pBestUnit->getUnitType()).getDescription();
				DEBUGLOG("AI_group:%S(%d) leaving group(%d) [size %d%s] to switch into %S(%d)'s group(%d) [size %d%s]\n", szUnitName.GetCString(), shortenID(m_iID), 
					shortenID(m_iGroupID), pCurrentGroup->getNumUnits(), pCurrentGroup->AI_isForceSeparate() ? ",WILL_SEPARATE" : "",
					szBestUnitName.GetCString(), shortenID(pBestUnit->getID()), 
					shortenID(pBestGroup->getID()), pBestGroup->getNumUnits(), pBestGroup->AI_isForceSeparate() ? ",WILL_SEPARATE" : "");
			}
			CvCity* pCity = plot()->getPlotCity();
#endif

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

bool CvUnitAI::AI_groupMergeRange(UnitAITypes eUnitAI, int iMaxRange, bool bBiggerOnly, bool bAllowRegrouping)
{
	PROFILE_FUNC();
	
	if (m_iLastJoinGroupTurn == GC.getGame().getGameTurn())
	{
		return false;
	}

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
	
	// cached values
	CvPlot* pPlot = plot();
	CvSelectionGroup* pGroup = getGroup();
	int iX = pPlot->getX_INLINE(), iY = pPlot->getY_INLINE();
	
	// best match
	CvUnit* pBestUnit = NULL;
	int iBestValue = MAX_INT;
	int iBestRange = -1;
	
	// iterate over plots at each range
	for (int iRange = 0; iRange <= iMaxRange && iBestValue == MAX_INT; iRange++)
	{
		int iDX, iDY;
		for (int iIndex = 0; rangeIndexToXY(iRange, iIndex, iDX, iDY); iIndex++)
		{
			CvPlot* pLoopPlot = plotXY(plot()->getX_INLINE(), plot()->getY_INLINE(), iDX, iDY);
			if (pLoopPlot != NULL && pLoopPlot->getArea() == pPlot->getArea())
			{
				CLLNode<IDInfo>* pUnitNode = pLoopPlot->headUnitNode();
				while (pUnitNode != NULL)
				{
					CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);
					
					CvSelectionGroup* pLoopGroup = pLoopUnit->getGroup();
					if ((pLoopUnit != this) && (pLoopGroup != pGroup) && pLoopUnit->isGroupHead() && !(pLoopUnit->isCargo()))
					{
						// do not merge with groups that are headed to group somewhere else, or are guarding a city
						// do not attempt to join groups which are healing this turn
						// (healing is cleared every turn for automated groups, so we know we pushed a heal this turn)
						MissionAITypes eLoopGroupMissionAI = pLoopGroup->AI_getMissionAIType();
						if (eLoopGroupMissionAI != MISSIONAI_GROUP && eLoopGroupMissionAI != MISSIONAI_GUARD_CITY &&
							pLoopGroup->getActivityType() != ACTIVITY_HEAL)
						{
							if (pLoopUnit->AI_getUnitAIType() == eUnitAI)
							{
								if (canJoinGroup(pPlot, pLoopGroup))
								{
									if (!bBiggerOnly || (pLoopGroup->getNumUnits() >= pGroup->getNumUnits()))
									{
										int iPathTurns;
										if (generatePath(pPlot, 0, true, &iPathTurns))
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
		}
	}

	if (pBestUnit != NULL)
	{
		if (atPlot(pBestUnit->plot()))
		{
#ifdef DEBUG_AI_UPDATE_GROUPS
			// debugging
			if (true)
			{
				CvSelectionGroup* pBestGroup = pBestUnit->getGroup();
				CvSelectionGroup* pCurrentGroup = getGroup();
				CvWString szUnitName = GC.getUnitInfo(m_eUnitType).getDescription();
				CvWString szBestUnitName = GC.getUnitInfo(pBestUnit->getUnitType()).getDescription();
				DEBUGLOG("AI_groupMergeRange:%S(%d)'s group(%d) [size %d%s] merging into %S(%d)'s group(%d) [size %d%s]\n", szUnitName.GetCString(), shortenID(m_iID), 
					shortenID(m_iGroupID), pCurrentGroup->getNumUnits(), pCurrentGroup->AI_isForceSeparate() ? ",WILL_SEPARATE" : "",
					szBestUnitName.GetCString(), shortenID(pBestUnit->getID()), 
					shortenID(pBestGroup->getID()), pBestGroup->getNumUnits(), pBestGroup->AI_isForceSeparate() ? ",WILL_SEPARATE" : "");
			}
			CvCity* pCity = plot()->getPlotCity();
#endif

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

	// do not load transports if we are already in a land war
	AreaAITypes eAreaAIType = area()->getAreaAIType(getTeam());
	bool bLandWar = ((eAreaAIType == AREAAI_OFFENSIVE) || (eAreaAIType == AREAAI_DEFENSIVE) || (eAreaAIType == AREAAI_MASSING));
	if (bLandWar)
	{
		return false;
	}

	iBestValue = MAX_INT;
	pBestUnit = NULL;
	
	const int iLoadMissionAICount = 4;
	MissionAITypes aeLoadMissionAI[iLoadMissionAICount] = {MISSIONAI_LOAD_ASSAULT, MISSIONAI_LOAD_SETTLER, MISSIONAI_LOAD_SPECIAL, MISSIONAI_ATTACK_SPY};

	int iCurrentGroupSize = getGroup()->getNumUnits();

	for(pLoopUnit = GET_PLAYER(getOwnerINLINE()).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER(getOwnerINLINE()).nextUnit(&iLoop))
	{
		if (pLoopUnit != this)
		{
			if (AI_plotValid(pLoopUnit->plot()))
			{
				if (canLoadUnit(pLoopUnit, pLoopUnit->plot()))
				{
					// special case ASSAULT_SEA UnitAI, so that, if a unit is marked escort, but can load units, it will load them
					// transport units might have been built as escort, this most commonly happens with galleons
					UnitAITypes eLoopUnitAI = pLoopUnit->AI_getUnitAIType();
					if (eLoopUnitAI == eUnitAI || (eUnitAI == UNITAI_ASSAULT_SEA && eLoopUnitAI == UNITAI_ESCORT_SEA))
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
												if (!(pLoopUnit->plot()->isVisibleEnemyUnit(getOwnerINLINE())))
												{
													if (generatePath(pLoopUnit->plot(), iFlags, true, &iPathTurns))
													{
														if (iPathTurns <= iMaxPath)
														{
															// prefer a transport that can hold as much of our group as possible 
															iValue = (max(0, iCurrentGroupSize - iCargoSpaceAvailable) * 5) + iPathTurns;

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
#ifdef DEBUG_AI_UPDATE_GROUPS
		CvSelectionGroup* pCurrentGroup = getGroup();
		CvSelectionGroup* pBestGroup = pBestUnit->getGroup();
		CvWString szUnitName = GC.getUnitInfo(m_eUnitType).getDescription();
		CvWString szBestUnitName = GC.getUnitInfo(pBestUnit->getUnitType()).getDescription();
		CvCity* pCity = plot()->getPlotCity();
#endif

		if (atPlot(pBestUnit->plot()))
		{
#ifdef DEBUG_AI_UPDATE_GROUPS
			DEBUGLOG("AI_load:%S(%d) in group(%d) [size %d%s] loading onto %S(%d)'s group(%d) [size %d%s]\n", 
				szUnitName.GetCString(), shortenID(m_iID), 
				shortenID(m_iGroupID), iCurrentGroupSize, pCurrentGroup->AI_isForceSeparate() ? ",WILL_SEPARATE" : "",
				szBestUnitName.GetCString(), shortenID(pBestUnit->getID()), 
				shortenID(pBestGroup->getID()), pBestGroup->getNumUnits(), pBestGroup->AI_isForceSeparate() ? ",WILL_SEPARATE" : "");
#endif

			getGroup()->setTransportUnit(pBestUnit); // XXX is this dangerous (not pushing a mission...) XXX air units?
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

#ifdef DEBUG_AI_UPDATE_GROUPS
			DEBUGLOG("AI_load:%S(%d) split from group (%d) [size %d], to group(%d) [size %d%s] headed to to load %S(%d)'s group(%d) [size %d%s]\n", 
				szUnitName.GetCString(), shortenID(m_iID), 
				shortenID(pCurrentGroup->getID()), iCurrentGroupSize,
				shortenID(pSplitGroup->getID()), pSplitGroup->getNumUnits(), pSplitGroup->AI_isForceSeparate() ? ",WILL_SEPARATE" : "",
				szBestUnitName.GetCString(), shortenID(pBestUnit->getID()), 
				shortenID(pBestGroup->getID()), pBestGroup->getNumUnits(), pBestGroup->AI_isForceSeparate() ? ",WILL_SEPARATE" : "");
#endif

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
bool CvUnitAI::AI_guardCity(bool bLeave, bool bSearch, int iMaxPath)
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvCity* pCity;
	CvCity* pLoopCity;
	CvUnit* pLoopUnit;
	CvPlot* pPlot;
	CvPlot* pBestPlot;
	CvPlot* pBestGuardPlot;
	bool bDefend;
	int iExtra;
	int iCount;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iLoop;

	FAssert(getDomainType() == DOMAIN_LAND);
	FAssert(canDefend());

	pPlot = plot();
	pCity = pPlot->getPlotCity();

	if ((pCity != NULL) && (pCity->getOwnerINLINE() == getOwnerINLINE())) // XXX check for other team?
	{
		if (bLeave && !(pCity->AI_isDanger()))
		{
			iExtra = 1;
		}
		else
		{
			iExtra = (bSearch ? 0 : -GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(pPlot, 2));
		}

		bDefend = false;

		if (pPlot->plotCount(PUF_canDefendGroupHead, -1, -1, getOwnerINLINE()) == 1) // XXX check for other team's units?
		{
			bDefend = true;
		}
		else if (!(pCity->AI_isDefended(((AI_isCityAIType()) ? -1 : 0) + iExtra))) // XXX check for other team's units?
		{
			if (AI_isCityAIType())
			{
				bDefend = true;
			}
			else
			{
				iCount = 0;

				pUnitNode = pPlot->headUnitNode();

				while (pUnitNode != NULL)
				{
					pLoopUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = pPlot->nextUnitNode(pUnitNode);

					if (pLoopUnit->getOwnerINLINE() == getOwnerINLINE())
					{
						if (pLoopUnit->isGroupHead())
						{
							if (!(pLoopUnit->isCargo()))
							{
								if (pLoopUnit->canDefend())
								{
									if (!(pLoopUnit->AI_isCityAIType()))
									{
										if (!(pLoopUnit->isHurt()))
										{
											if (pLoopUnit->isWaiting())
											{
												FAssert(pLoopUnit != this);
												iCount++;
											}
										}
									}
								}
							}
						}
					}
				}

				if (!(pCity->AI_isDefended(iCount + iExtra))) // XXX check for other team's units?
				{
					bDefend = true;
				}
			}
		}

		if (bDefend)
		{
			// split up group if we are going to defend, so rest of group has opportunity to do something else
			if (getGroup()->getNumUnits() > 1)
			{
				getGroup()->AI_separate();	// will change group
			}

#ifdef DEBUG_AI_UPDATE_GROUPS
			CvWString szUnitName = GC.getUnitInfo(m_eUnitType).getDescription();
			DEBUGLOG("AI_guardCity:%S(%d) guarding %S\n", szUnitName.GetCString(), shortenID(m_iID), pCity->getName().GetCString());
#endif

			// the plot in this mission has to be NULL or it will get counted below in the bSearch part
			getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_GUARD_CITY, NULL);
			return true;
		}
	}

	if (bSearch)
	{
		iBestValue = MAX_INT;
		pBestPlot = NULL;
		pBestGuardPlot = NULL;

		for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
		{
			if (AI_plotValid(pLoopCity->plot()))
			{
				if (!(pLoopCity->AI_isDefended((!AI_isCityAIType()) ? pLoopCity->plot()->plotCount(PUF_canDefendGroupHead, -1, -1, getOwnerINLINE(), NO_TEAM, PUF_isNotCityAIType) : 0)))	// XXX check for other team's units?
				{
					if (!(pLoopCity->plot()->isVisibleEnemyUnit(getOwnerINLINE())))
					{
						if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopCity->plot(), MISSIONAI_GUARD_CITY, getGroup()) < 2)
						{
							if (!atPlot(pLoopCity->plot()) && generatePath(pLoopCity->plot(), 0, true, &iPathTurns))
							{
								if (iPathTurns <= iMaxPath)
								{
									iValue = iPathTurns;

									if (iValue < iBestValue)
									{
										iBestValue = iValue;
										pBestPlot = getPathEndTurnPlot();
										pBestGuardPlot = pLoopCity->plot();
										FAssert(!atPlot(pBestPlot));
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
			}
		}

		if ((pBestPlot != NULL) && (pBestGuardPlot != NULL))
		{
			FAssert(!atPlot(pBestPlot));
			// split up group if we are going to defend, so rest of group has opportunity to do something else
			if (getGroup()->getNumUnits() > 1)
			{
				getGroup()->AI_separate();	// will change group
			}

#ifdef DEBUG_AI_UPDATE_GROUPS
			CvWString szUnitName = GC.getUnitInfo(m_eUnitType).getDescription();
			DEBUGLOG("AI_guardCity:%S(%d) moving to guard %S\n", szUnitName.GetCString(), shortenID(m_iID), pBestGuardPlot->getPlotCity()->getName().GetCString());
#endif

			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_GUARD_CITY, pBestGuardPlot);
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_guardCityAirlift()
{
	PROFILE_FUNC();

	CvCity* pCity;
	CvCity* pLoopCity;
	CvPlot* pBestPlot;
	int iValue;
	int iBestValue;
	int iLoop;

	if (getGroup()->getNumUnits() > 1)
	{
		return false;
	}

	pCity = plot()->getPlotCity();

	if (pCity == NULL)
	{
		return false;
	}

	if (pCity->getMaxAirlift() == 0)
	{
		return false;
	}

	iBestValue = 0;
	pBestPlot = NULL;

	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		if (pLoopCity != pCity)
		{
			if (canAirliftAt(pCity->plot(), pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE()))
			{
				if (!(pLoopCity->AI_isDefended((!AI_isCityAIType()) ? pLoopCity->plot()->plotCount(PUF_canDefendGroupHead, -1, -1, getOwnerINLINE(), NO_TEAM, PUF_isNotCityAIType) : 0)))	// XXX check for other team's units?
				{
					iValue = pLoopCity->getPopulation();

					if (pLoopCity->AI_isDanger())
					{
						iValue *= 2;
					}

					if (iValue > iBestValue)
					{
						iBestValue = iValue;
						pBestPlot = pLoopCity->plot();
						FAssert(pLoopCity != pCity);
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		getGroup()->pushMission(MISSION_AIRLIFT, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
		return true;
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_guardBonus(int iMinValue)
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	CvPlot* pBestGuardPlot;
	ImprovementTypes eImprovement;
	BonusTypes eNonObsoleteBonus;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iI;

	iBestValue = 0;
	pBestPlot = NULL;
	pBestGuardPlot = NULL;

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (AI_plotValid(pLoopPlot))
		{
			if (pLoopPlot->getOwnerINLINE() == getOwnerINLINE())
			{
				eNonObsoleteBonus = pLoopPlot->getNonObsoleteBonusType(getTeam());

				if (eNonObsoleteBonus != NO_BONUS)
				{
					eImprovement = pLoopPlot->getImprovementType();

					if ((eImprovement != NO_IMPROVEMENT) && GC.getImprovementInfo(eImprovement).isImprovementBonusTrade(eNonObsoleteBonus))
					{
						iValue = GET_PLAYER(getOwnerINLINE()).AI_bonusVal(eNonObsoleteBonus);

						iValue += max(0, 200 * GC.getBonusInfo(eNonObsoleteBonus).getAIObjective());

						if (pLoopPlot->getPlotGroupConnectedBonus(getOwnerINLINE(), eNonObsoleteBonus) == 1)
						{
							iValue *= 2;
						}

						if (iValue > iMinValue)
						{
							if (!(pLoopPlot->isVisibleEnemyUnit(getOwnerINLINE())))
							{
								if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_GUARD_BONUS, getGroup()) == 0)
								{
									if (generatePath(pLoopPlot, 0, true, &iPathTurns))
									{
										iValue *= 1000;

										iValue /= (iPathTurns + 1);

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
bool CvUnitAI::AI_guardSpy()
{
	PROFILE_FUNC();

	CvCity* pLoopCity;
	CvPlot* pBestPlot;
	CvPlot* pBestGuardPlot;
	int iValue;
	int iBestValue;
	int iLoop;

	iBestValue = 0;
	pBestPlot = NULL;
	pBestGuardPlot = NULL;

	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		if (AI_plotValid(pLoopCity->plot()))
		{
			if (!(pLoopCity->plot()->isVisibleEnemyUnit(getOwnerINLINE())))
			{
				if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopCity->plot(), MISSIONAI_GUARD_SPY, getGroup()) == 0)
				{
					if (generatePath(pLoopCity->plot(), 0, true))
					{
						iValue = (pLoopCity->getPopulation() * 10);

						iValue += pLoopCity->getProduction();

						if (pLoopCity->isProductionUnit())
						{
							if (isLimitedUnitClass((UnitClassTypes)(GC.getUnitInfo(pLoopCity->getProductionUnit()).getUnitClassType())))
							{
								iValue *= 4;
							}
						}
						else if (pLoopCity->isProductionBuilding())
						{
							if (isLimitedWonderClass((BuildingClassTypes)(GC.getBuildingInfo(pLoopCity->getProductionBuilding()).getBuildingClassType())))
							{
								iValue *= 5;
							}
						}
						else if (pLoopCity->isProductionProject())
						{
							if (isLimitedProject(pLoopCity->getProductionProject()))
							{
								iValue *= 6;
							}
						}

						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							pBestPlot = getPathEndTurnPlot();
							pBestGuardPlot = pLoopCity->plot();
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
			getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_GUARD_SPY, pBestGuardPlot);
			return true;
		}
		else
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_GUARD_SPY, pBestGuardPlot);
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_destroySpy()
{
	PROFILE_FUNC();

	CvCity* pLoopCity;
	CvCity* pBestCity;
	CvPlot* pBestPlot;
	int iValue;
	int iBestValue;
	int iLoop;
	int iI;

	iBestValue = 0;
	pBestPlot = NULL;
	pBestCity = NULL;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() != getTeam())
			{
				if (GET_PLAYER(getOwnerINLINE()).AI_getAttitude((PlayerTypes)iI) <= ATTITUDE_ANNOYED)
				{
					for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
					{
						if (AI_plotValid(pLoopCity->plot()))
						{
							if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopCity->plot(), MISSIONAI_ATTACK_SPY, getGroup()) == 0)
							{
								if (generatePath(pLoopCity->plot(), 0, true))
								{
									iValue = (pLoopCity->getPopulation() * 2);

									iValue += pLoopCity->getYieldRate(YIELD_PRODUCTION);

									if (atPlot(pLoopCity->plot()))
									{
										iValue *= 4;
										iValue /= 3;
									}

									if (iValue > iBestValue)
									{
										iBestValue = iValue;
										pBestPlot = getPathEndTurnPlot();
										pBestCity = pLoopCity;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestCity != NULL))
	{
		if (atPlot(pBestCity->plot()))
		{
			if (canDestroy(pBestCity->plot()))
			{
				if (pBestCity->getProduction() > ((pBestCity->getProductionNeeded() * 2) / 3))
				{
					if (pBestCity->isProductionUnit())
					{
						if (isLimitedUnitClass((UnitClassTypes)(GC.getUnitInfo(pBestCity->getProductionUnit()).getUnitClassType())))
						{
							getGroup()->pushMission(MISSION_DESTROY);
							return true;
						}
					}
					else if (pBestCity->isProductionBuilding())
					{
						if (isLimitedWonderClass((BuildingClassTypes)(GC.getBuildingInfo(pBestCity->getProductionBuilding()).getBuildingClassType())))
						{
							getGroup()->pushMission(MISSION_DESTROY);
							return true;
						}
					}
					else if (pBestCity->isProductionProject())
					{
						if (isLimitedProject(pBestCity->getProductionProject()))
						{
							getGroup()->pushMission(MISSION_DESTROY);
							return true;
						}
					}
				}
			}

			getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_ATTACK_SPY, pBestCity->plot());
			return true;
		}
		else
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_ATTACK_SPY, pBestCity->plot());
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_sabotageSpy()
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	CvPlot* pBestSabotagePlot;
	bool abPlayerAngry[MAX_PLAYERS];
	ImprovementTypes eImprovement;
	BonusTypes eNonObsoleteBonus;
	int iValue;
	int iBestValue;
	int iI;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		abPlayerAngry[iI] = false;

		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() != getTeam())
			{
				if (GET_PLAYER(getOwnerINLINE()).AI_getAttitude((PlayerTypes)iI) <= ATTITUDE_ANNOYED)
				{
					abPlayerAngry[iI] = true;
				}
			}
		}
	}

	iBestValue = 0;
	pBestPlot = NULL;
	pBestSabotagePlot = NULL;

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (AI_plotValid(pLoopPlot))
		{
			if (pLoopPlot->isOwned())
			{
				if (pLoopPlot->getTeam() != getTeam())
				{
					if (abPlayerAngry[pLoopPlot->getOwnerINLINE()])
					{
						eNonObsoleteBonus = pLoopPlot->getNonObsoleteBonusType(pLoopPlot->getTeam());

						if (eNonObsoleteBonus != NO_BONUS)
						{
							eImprovement = pLoopPlot->getImprovementType();

							if ((eImprovement != NO_IMPROVEMENT) && GC.getImprovementInfo(eImprovement).isImprovementBonusTrade(eNonObsoleteBonus))
							{
								if (canSabotage(pLoopPlot))
								{
									iValue = GET_PLAYER(pLoopPlot->getOwnerINLINE()).AI_bonusVal(eNonObsoleteBonus);

									if (pLoopPlot->isConnectedToCapital() && (pLoopPlot->getPlotGroupConnectedBonus(pLoopPlot->getOwnerINLINE(), eNonObsoleteBonus) == 1))
									{
										iValue *= 3;
									}

									if (iValue > 25)
									{
										if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_ATTACK_SPY, getGroup()) == 0)
										{
											if (generatePath(pLoopPlot, 0, true))
											{
												if (iValue > iBestValue)
												{
													iBestValue = iValue;
													pBestPlot = getPathEndTurnPlot();
													pBestSabotagePlot = pLoopPlot;
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

	if ((pBestPlot != NULL) && (pBestSabotagePlot != NULL))
	{
		if (atPlot(pBestSabotagePlot))
		{
			getGroup()->pushMission(MISSION_SABOTAGE);
			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_ATTACK_SPY, pBestSabotagePlot);
			return true;
		}
	}

	return false;
}


bool CvUnitAI::AI_pickupTargetSpy()
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

	pCity = plot()->getPlotCity();

	if (pCity != NULL)
	{
		if (pCity->getOwnerINLINE() == getOwnerINLINE())
		{
			if (pCity->isCoastal(GC.getDefineINT("MIN_WATER_SIZE_FOR_OCEAN")))
			{
				getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_ATTACK_SPY, pCity->plot());
				return true;
			}
		}
	}

	iBestValue = MAX_INT;
	pBestPlot = NULL;
	pBestPickupPlot = NULL;

	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		if (AI_plotValid(pLoopCity->plot()))
		{
			if (pLoopCity->isCoastal(GC.getDefineINT("MIN_WATER_SIZE_FOR_OCEAN")))
			{
				if (!(pLoopCity->plot()->isVisibleEnemyUnit(getOwnerINLINE())))
				{
					if (!atPlot(pLoopCity->plot()) && generatePath(pLoopCity->plot(), 0, true, &iPathTurns))
					{
						iValue = iPathTurns;

						if (iValue < iBestValue)
						{
							iBestValue = iValue;
							pBestPlot = getPathEndTurnPlot();
							pBestPickupPlot = pLoopCity->plot();
							FAssert(!atPlot(pBestPlot));
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestPickupPlot != NULL))
	{
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_ATTACK_SPY, pBestPickupPlot);
		return true;
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
						if (AI_anyAttack(1, 65, max(0, (iPlotDanger - 1))))
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
	
    if (getGroup()->getNumUnits() == 1)
	{
	    if (getDamage() > 0)
        {
        	
            if (plot()->isCity() || (healTurns(plot()) <= iMaxPath))
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
	
	iMaxPath = min(iMaxPath, 2);

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
#ifdef DEBUG_AI_UPDATE_GROUPS
			// debugging
			if (true)
			{
				CvSelectionGroup* pCurrentGroup = getGroup();
				CvWString szUnitName = GC.getUnitInfo(m_eUnitType).getDescription();
				DEBUGLOG("AI_heal:%S(%d) leaving group(%d) [size %d%s] to heal solo\n", szUnitName.GetCString(), shortenID(m_iID), 
					shortenID(m_iGroupID), pCurrentGroup->getNumUnits(), pCurrentGroup->AI_isForceSeparate() ? ",WILL_SEPARATE" : "");
			}
#endif
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
	
		if (healRate(plot()) >= 10)
	    {
            pGroup->pushMission(MISSION_SKIP);
            return true;
	    }
	    else if (AI_moveIntoCity(2))
		{
			return true;
		}
	}
	
	return bPushedMission;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_afterAttack()
{
	if (!isMadeAttack())
	{
		return false;
	}

	if (isBlitz())
	{
		return false;
	}

	if (getDomainType() == DOMAIN_LAND)
	{
		if (AI_guardCity(false, true, 1))
		{
			return true;
		}
	}

	if (AI_pillageRange(1))
	{
		return true;
	}

	if (AI_retreatToCity(false, false, 1))
	{
		return true;
	}

	if (AI_hide())
	{
		return true;
	}

	if (AI_goody(1))
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
bool CvUnitAI::AI_goldenAge()
{
	if (canGoldenAge(plot()))
	{
		getGroup()->pushMission(MISSION_GOLDEN_AGE);
		return true;
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_spreadReligion()
{
	PROFILE_FUNC();

	CvCity* pLoopCity;
	CvPlot* pBestPlot;
	CvPlot* pBestSpreadPlot;
	ReligionTypes eReligion;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iPlayerMultiplierPercent;
	int iLoop;
	int iI;


    bool bCultureVictory = GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_CULTURE2);
	eReligion = NO_RELIGION;

	if (eReligion == NO_RELIGION)
	{
		if (GET_PLAYER(getOwnerINLINE()).getStateReligion() != NO_RELIGION)
		{
			if (GC.getUnitInfo(getUnitType()).getReligionSpreads(GET_PLAYER(getOwnerINLINE()).getStateReligion()))
			{
				eReligion = GET_PLAYER(getOwnerINLINE()).getStateReligion();
			}
		}
	}

	if (eReligion == NO_RELIGION)
	{
		for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
		{
			if (bCultureVictory || GET_TEAM(getTeam()).hasHolyCity((ReligionTypes)iI))
			{
				if (GC.getUnitInfo(getUnitType()).getReligionSpreads((ReligionTypes)iI))
				{
					eReligion = ((ReligionTypes)iI);
					break;
				}
			}
		}
	}

	if (eReligion == NO_RELIGION)
	{
		return false;
	}

	bool bHasHolyCity = GET_TEAM(getTeam()).hasHolyCity(eReligion);
	bool bHasAnyHolyCity = bHasHolyCity;
	if (!bHasAnyHolyCity)
	{
		for (iI = 0; !bHasAnyHolyCity && iI < GC.getNumReligionInfos(); iI++)
		{
			bHasAnyHolyCity = GET_TEAM(getTeam()).hasHolyCity((ReligionTypes)iI);
		}
	}

	iBestValue = 0;
	pBestPlot = NULL;
	pBestSpreadPlot = NULL;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
		    iPlayerMultiplierPercent = 100;

			if (GET_PLAYER((PlayerTypes)iI).getTeam() != getTeam())
			{
				if (!bCultureVictory || (eReligion == GET_PLAYER(getOwnerINLINE()).getStateReligion()))
				{
					if (GET_PLAYER((PlayerTypes)iI).getStateReligion() == NO_RELIGION)
					{
						if (0 == (GET_PLAYER((PlayerTypes)iI).getNonStateReligionHappiness()))
						{
							iPlayerMultiplierPercent += 600;
						}
					}
					else if (GET_PLAYER((PlayerTypes)iI).getStateReligion() == eReligion)
					{
						iPlayerMultiplierPercent += 300;
					}
					else
					{
						if (GET_PLAYER((PlayerTypes)iI).hasHolyCity(GET_PLAYER((PlayerTypes)iI).getStateReligion()))
						{
							iPlayerMultiplierPercent += 50;
						}
						else
						{
							iPlayerMultiplierPercent += 300;
						}
					}
					
					int iReligionCount = GET_PLAYER((PlayerTypes)iI).countTotalHasReligion();
					int iCityCount = GET_PLAYER(getOwnerINLINE()).getNumCities();
					//magic formula to produce normalized adjustment factor based on religious infusion
					int iAdjustment = (100 * (iCityCount + 1));
					iAdjustment /= ((iCityCount + 1) + iReligionCount);
					iAdjustment = (((iAdjustment - 25) * 4) / 3);
	                
					iAdjustment = (max(10, iAdjustment));
	                
					iPlayerMultiplierPercent *= iAdjustment;
					iPlayerMultiplierPercent /= 100;

					// if we have a holy city, but not this holy city, then we will likely switch
					// religions soon, so try to spread this religion internally, not externally
					if (bHasAnyHolyCity && !bHasHolyCity)
					{
						iPlayerMultiplierPercent /= 10;
					}
				}
			}

			for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
			{

				if (AI_plotValid(pLoopCity->plot()))
				{
					if (canSpread(pLoopCity->plot(), eReligion))
					{
						if (!(pLoopCity->plot()->isVisibleEnemyUnit(getOwnerINLINE())))
						{
							if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopCity->plot(), MISSIONAI_SPREAD, getGroup()) == 0)
							{
								if (generatePath(pLoopCity->plot(), 0, true, &iPathTurns))
								{
									iValue = (7 + (pLoopCity->getPopulation() * 4));

									bool bOurCity = false;
									if (pLoopCity->getOwnerINLINE() == getOwnerINLINE())
									{
										iValue *= (bCultureVictory ? 16 : 4);
										bOurCity = true;
									}
									else if (pLoopCity->getTeam() == getTeam())
									{
										iValue *= 3;
										bOurCity = true;
									}
									else
									{
									    iValue *= iPlayerMultiplierPercent;
									    iValue /= 100;
									}
									
									int iCityReligionCount = pLoopCity->getReligionCount();
									int iReligionCountFactor = iCityReligionCount;

									if (bOurCity)
									{
										// count cities with no religion the same as cities with 2 religions
										// prefer a city with exactly 1 religion already
										if (iCityReligionCount == 0)
										{
											iReligionCountFactor = 2;
										}
										else if (iCityReligionCount == 1)
										{
											iValue *= 2;
										}
									}
									else
									{
										// absolutely prefer cities with zero religions
										if (iCityReligionCount == 0)
										{
											iValue *= 2;
										}

										// not our city, so prefer the lowest number of religions (increment so no divide by zero)
										iReligionCountFactor++;
									}

									iValue /= iReligionCountFactor;

									FAssert(iPathTurns > 0);

									iValue *= 1000;

									iValue /= (iPathTurns + 2);

									if (iValue > iBestValue)
									{
										iBestValue = iValue;
										pBestPlot = getPathEndTurnPlot();
										pBestSpreadPlot = pLoopCity->plot();
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
			getGroup()->pushMission(MISSION_SPREAD, eReligion);
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


// Returns true if a mission was pushed...
bool CvUnitAI::AI_discover(bool bThisTurnOnly, bool bFirstResearchOnly)
{
	TechTypes eDiscoverTech;
	bool bIsFirstTech;
	int iPercentWasted = 0;

	if (canDiscover(plot()))
	{
		eDiscoverTech = getDiscoveryTech();
		bIsFirstTech = (GC.getGameINLINE().AI_isFirstTech(eDiscoverTech));

        if (bFirstResearchOnly && !bIsFirstTech)
        {
            return false;
        }

		iPercentWasted = (100 - ((getDiscoverResearch(eDiscoverTech) * 100) / getDiscoverResearch(NO_TECH)));
		FAssert(((iPercentWasted >= 0) && (iPercentWasted <= 100)));


        if (getDiscoverResearch(eDiscoverTech) >= GET_TEAM(getTeam()).getResearchLeft(eDiscoverTech))
        {
            if ((iPercentWasted < 51) && bFirstResearchOnly && bIsFirstTech)
            {
                getGroup()->pushMission(MISSION_DISCOVER);
                return true;
            }

            if (iPercentWasted < (bIsFirstTech ? 31 : 11))
            {
                //I need a good way to assess if the tech is actually valuable...
                //but don't have one.
                getGroup()->pushMission(MISSION_DISCOVER);
                return true;
            }
        }
        else if (bThisTurnOnly)
        {
            return false;
        }

        if (iPercentWasted <= 11)
        {
            if (GET_PLAYER(getOwnerINLINE()).getCurrentResearch() == eDiscoverTech)
            {
                getGroup()->pushMission(MISSION_DISCOVER);
                return true;
            }
        }
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
	TeamTypes eOurTeam = kOwner.getTeam();

	bool bNeedLeader = false;
	for (int iI = 0; iI < MAX_CIV_TEAMS; iI++)
	{
		CvTeamAI& kLoopTeam = GET_TEAM((TeamTypes)iI);
		if (kLoopTeam.isAtWar(eOurTeam))
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
							if (!(pLoopUnit->plot()->isVisibleEnemyUnit(getOwnerINLINE())))
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
// iMaxCounts = 1 would mean join a city if there's no existing joined GP of that type.
bool CvUnitAI::AI_join(int iMaxCount)
{
	PROFILE_FUNC();

	CvCity* pLoopCity;
	CvPlot* pBestPlot;
	SpecialistTypes eBestSpecialist;
	int iValue;
	int iBestValue;
	int iLoop;
	int iI;
	int iCount;

	iBestValue = 0;
	pBestPlot = NULL;
	eBestSpecialist = NO_SPECIALIST;
	iCount = 0;

	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		if (AI_plotValid(pLoopCity->plot()))
		{
			if (!(pLoopCity->plot()->isVisibleEnemyUnit(getOwnerINLINE())))
			{
				if (generatePath(pLoopCity->plot(), 0, true))
				{
					for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
					{
						bool bDoesJoin = false;
						SpecialistTypes eSpecialist = (SpecialistTypes)iI;
						if (GC.getUnitInfo(getUnitType()).getGreatPeoples(eSpecialist))
						{
							bDoesJoin = true;
						}
						if (bDoesJoin)
						{
							iCount += pLoopCity->getSpecialistCount(eSpecialist);
							if (iCount >= iMaxCount)
							{
								return false;
							}
						}
						
						if (canJoin(pLoopCity->plot(), ((SpecialistTypes)iI)))
						{
							iValue = pLoopCity->AI_specialistValue(((SpecialistTypes)iI), pLoopCity->AI_avoidGrowth(), false);

							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								pBestPlot = getPathEndTurnPlot();
								eBestSpecialist = ((SpecialistTypes)iI);
							}
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (eBestSpecialist != NO_SPECIALIST))
	{
		if (atPlot(pBestPlot))
		{
			getGroup()->pushMission(MISSION_JOIN, eBestSpecialist);
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
// iMaxCounts = 1 would mean construct only if there are no existing buildings 
//   constructed by this GP type.
bool CvUnitAI::AI_construct(int iMaxCount)
{
	PROFILE_FUNC();

	CvCity* pLoopCity;
	CvPlot* pBestPlot;
	CvPlot* pBestConstructPlot;
	BuildingTypes eBestBuilding;
	int iValue;
	int iBestValue;
	int iLoop;
	int iI;
	int iCount;

	iBestValue = 0;
	pBestPlot = NULL;
	pBestConstructPlot = NULL;
	eBestBuilding = NO_BUILDING;
	iCount = 0;
	
	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		if (AI_plotValid(pLoopCity->plot()))
		{
			if (!(pLoopCity->plot()->isVisibleEnemyUnit(getOwnerINLINE())))
			{
				if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopCity->plot(), MISSIONAI_CONSTRUCT, getGroup()) == 0)
				{
					if (generatePath(pLoopCity->plot(), 0, true))
					{
						for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
						{
							BuildingTypes eBuilding = (BuildingTypes)iI;
							bool bDoesBuild = false;
							if ((GC.getUnitInfo(getUnitType()).getForceBuildings(eBuilding))
								|| (GC.getUnitInfo(getUnitType()).getBuildings(eBuilding)))
							{
								bDoesBuild = true;
							}
							
							if (bDoesBuild && (pLoopCity->hasBuilding(eBuilding)))
							{
								iCount++;
								if (iCount >= iMaxCount)
								{
									return false;
								}
							}
							
							if (bDoesBuild && canConstruct(pLoopCity->plot(), eBuilding))
							{
								iValue = pLoopCity->AI_buildingValue(eBuilding);

								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									pBestPlot = getPathEndTurnPlot();
									pBestConstructPlot = pLoopCity->plot();
									eBestBuilding = ((BuildingTypes)iI);
								}
							}
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestConstructPlot != NULL) && (eBestBuilding != NO_BUILDING))
	{
		if (atPlot(pBestConstructPlot))
		{
			getGroup()->pushMission(MISSION_CONSTRUCT, eBestBuilding);
			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_CONSTRUCT, pBestConstructPlot);
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_switchHurry()
{
	CvCity* pCity;
	BuildingTypes eBestBuilding;
	int iValue;
	int iBestValue;
	int iI;

	pCity = plot()->getPlotCity();

	if (pCity == NULL)
	{
		return false;
	}

	iBestValue = 0;
	eBestBuilding = NO_BUILDING;

	for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
	{
		if (isWorldWonderClass((BuildingClassTypes)(GC.getBuildingInfo((BuildingTypes)iI).getBuildingClassType())))
		{
			if (pCity->canConstruct((BuildingTypes)iI))
			{
				if (pCity->getBuildingProduction((BuildingTypes)iI) == 0)
				{
					if (getMaxHurryProduction(pCity) >= GET_PLAYER(getOwnerINLINE()).getProductionNeeded((BuildingTypes)iI))
					{
						iValue = pCity->AI_buildingValue((BuildingTypes)iI);

						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							eBestBuilding = ((BuildingTypes)iI);
						}
					}
				}
			}
		}
	}

	if (eBestBuilding != NO_BUILDING)
	{
		pCity->pushOrder(ORDER_CONSTRUCT, eBestBuilding, -1, false, false, false);

		if (pCity->getProductionBuilding() == eBestBuilding)
		{
			if (canHurry(plot()))
			{
				getGroup()->pushMission(MISSION_HURRY);
				return true;
			}
		}

		FAssert(false);
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_hurry()
{
	PROFILE_FUNC();

	CvCity* pLoopCity;
	CvPlot* pBestPlot;
	CvPlot* pBestHurryPlot;
	bool bHurry;
	int iTurnsLeft;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iLoop;

	iBestValue = 0;
	pBestPlot = NULL;
	pBestHurryPlot = NULL;

	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		if (AI_plotValid(pLoopCity->plot()))
		{
			if (canHurry(pLoopCity->plot()))
			{
				if (!(pLoopCity->plot()->isVisibleEnemyUnit(getOwnerINLINE())))
				{
					if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopCity->plot(), MISSIONAI_HURRY, getGroup()) == 0)
					{
						if (generatePath(pLoopCity->plot(), 0, true, &iPathTurns))
						{
							bHurry = false;

							if (pLoopCity->isProductionBuilding())
							{
								if (isWorldWonderClass((BuildingClassTypes)(GC.getBuildingInfo(pLoopCity->getProductionBuilding()).getBuildingClassType())))
								{
									bHurry = true;
								}
							}

							if (bHurry)
							{
								iTurnsLeft = pLoopCity->getProductionTurnsLeft();

								iTurnsLeft -= iPathTurns;

								if (iTurnsLeft > 8)
								{
									iValue = iTurnsLeft;

									if (iValue > iBestValue)
									{
										iBestValue = iValue;
										pBestPlot = getPathEndTurnPlot();
										pBestHurryPlot = pLoopCity->plot();
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestHurryPlot != NULL))
	{
		if (atPlot(pBestHurryPlot))
		{
			getGroup()->pushMission(MISSION_HURRY);
			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_HURRY, pBestHurryPlot);
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_greatWork()
{
	PROFILE_FUNC();

	CvCity* pLoopCity;
	CvPlot* pBestPlot;
	CvPlot* pBestGreatWorkPlot;
	int iValue;
	int iBestValue;
	int iLoop;

	iBestValue = 0;
	pBestPlot = NULL;
	pBestGreatWorkPlot = NULL;

	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		if (AI_plotValid(pLoopCity->plot()))
		{
			if (canGreatWork(pLoopCity->plot()))
			{
				if (!(pLoopCity->plot()->isVisibleEnemyUnit(getOwnerINLINE())))
				{
					if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopCity->plot(), MISSIONAI_GREAT_WORK, getGroup()) == 0)
					{
						if (generatePath(pLoopCity->plot(), 0, true))
						{
							iValue = pLoopCity->AI_calculateCulturePressure(true);
							iValue -= ((100 * pLoopCity->getCulture(pLoopCity->getOwnerINLINE())) / max(1, getGreatWorkCulture(pLoopCity->plot())));
							if (iValue > 0)
							{
								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									pBestPlot = getPathEndTurnPlot();
									pBestGreatWorkPlot = pLoopCity->plot();
								}
							}
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestGreatWorkPlot != NULL))
	{
		if (atPlot(pBestGreatWorkPlot))
		{
			getGroup()->pushMission(MISSION_GREAT_WORK);
			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_GREAT_WORK, pBestGreatWorkPlot);
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_offensiveAirlift()
{
	PROFILE_FUNC();

	CvCity* pCity;
	CvCity* pTargetCity;
	CvCity* pLoopCity;
	CvPlot* pBestPlot;
	int iValue;
	int iBestValue;
	int iLoop;

	if (getGroup()->getNumUnits() > 1)
	{
		return false;
	}

	if (area()->getTargetCity(getOwnerINLINE()) != NULL)
	{
		return false;
	}

	pCity = plot()->getPlotCity();

	if (pCity == NULL)
	{
		return false;
	}

	if (pCity->getMaxAirlift() == 0)
	{
		return false;
	}

	iBestValue = 0;
	pBestPlot = NULL;

	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		if (pLoopCity->area() != pCity->area())
		{
			if (canAirliftAt(pCity->plot(), pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE()))
			{
				pTargetCity = pLoopCity->area()->getTargetCity(getOwnerINLINE());

				if (pTargetCity != NULL)
				{
					iValue = 10000;

					iValue *= (GET_PLAYER(getOwnerINLINE()).AI_militaryWeight(pLoopCity->area()) + 10);
					iValue /= (GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(pLoopCity->area(), AI_getUnitAIType()) + 10);

					iValue += max(1, ((GC.getMapINLINE().maxStepDistance() * 2) - GC.getMapINLINE().calculatePathDistance(pLoopCity->plot(), pTargetCity->plot())));

					if (iValue > iBestValue)
					{
						iBestValue = iValue;
						pBestPlot = pLoopCity->plot();
						FAssert(pLoopCity != pCity);
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		getGroup()->pushMission(MISSION_AIRLIFT, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
		return true;
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
				if (pLoopPlot->isVisibleEnemyUnit(getOwnerINLINE()))
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

// value of a plot for fogbusting
int CvUnitAI::AI_fogbustPlotValue(const CvPlot* pPlot, int iNumObservers) const
{
	int iValue = 0;

	int iNewVisibilityCount = pPlot->getVisibilityCount(getTeam()) - iNumObservers;
	if (iNewVisibilityCount <= 0)
	{
		iValue++;

		if (!pPlot->isOwned())
		{
			iValue++;
		}

		if (!pPlot->isImpassable())
		{
			iValue *= 4;

			if ((getDomainType() == DOMAIN_SEA) || pPlot->getArea() == getArea())
			{
				iValue *= 2;
			}
		}
	}

	return iValue;
}

// given a CvPlotRegion (assumed to be not visible yet), calculates the fogbusting value
// if pbAdjacentOurTerritory != NULL, returns whether the region is adjacent
// if pVisibleRegion != NULL it is used to check adjacent, rather than plotRegion
int CvUnitAI::AI_fogbustRegionValue(const CvPlotRegion& plotRegion, bool* pbAdjacentOurTerritory, const CvPlotRegion* pVisibleRegion) const
{
	PROFILE_FUNC();

	int iTotalValue = 0;

	bool bAdjacentOurTerritory = false;
	
	PlayerTypes ePlayer = getOwnerINLINE();

	for (CvPlotRegionConstIterator it = plotRegion.begin(); it != plotRegion.end(); it++)
	{
		XYCoords xy(getXYCoords(it));
		CvPlot* pLoopPlot = GC.getMapINLINE().plotSorenINLINE(xy.iX, xy.iY);

		iTotalValue += AI_fogbustPlotValue(pLoopPlot, /*iNumObservers*/ 0);

		if (pLoopPlot->getOwnerINLINE() == ePlayer)
		{
			bAdjacentOurTerritory = true;
		}
	}
	
	// check the plots just outside our visibility for plots that we own (range 2, since owned plots have a 1 visibility)
	if (pbAdjacentOurTerritory != NULL && !bAdjacentOurTerritory)
	{
		CvPlotRegion adjacentPlotRegion;
		findAdjacentPlots((pVisibleRegion != NULL) ? (*pVisibleRegion) : plotRegion, adjacentPlotRegion, /*bIncludeOriginal*/ false, /*iRange*/ 2);
		for (CvPlotRegionConstIterator it = adjacentPlotRegion.begin(); !bAdjacentOurTerritory && it != adjacentPlotRegion.end(); it++)
		{
			XYCoords xy(getXYCoords(it));
			CvPlot* pLoopPlot = GC.getMapINLINE().plotSorenINLINE(xy.iX, xy.iY);

			if (pLoopPlot->getOwnerINLINE() == ePlayer)
			{
				bAdjacentOurTerritory = true;
			}
		}
	}

	if (pbAdjacentOurTerritory != NULL)
	{
		*pbAdjacentOurTerritory = bAdjacentOurTerritory;
	}

	return iTotalValue;

}

// given a CvPlotDataRegion, calculates the fogbusting value (if bRegionIsVisible true, uses data value as number of observers)
// if pbAdjacentOurTerritory != NULL, returns whether the region is adjacent
// if pVisibleRegion != NULL it is used to check adjacent, rather than plotRegion
int CvUnitAI::AI_fogbustDataRegionValue(const CvPlotDataRegion& plotRegion, bool bRegionIsVisible, bool* pbAdjacentOurTerritory, const CvPlotDataRegion* pVisibleRegion) const
{
	PROFILE_FUNC();

	int iTotalValue = 0;

	bool bAdjacentOurTerritory = false;
	
	PlayerTypes ePlayer = getOwnerINLINE();

	for (CvPlotDataRegionConstIterator it = plotRegion.begin(); it != plotRegion.end(); it++)
	{
		XYCoords xy(getXYCoords(it));
		CvPlot* pLoopPlot = GC.getMapINLINE().plotSorenINLINE(xy.iX, xy.iY);

		int iNumObservers = bRegionIsVisible ? getRegionValue(it) : 0;
		iTotalValue +=  AI_fogbustPlotValue(pLoopPlot, iNumObservers);

		if (pLoopPlot->getOwnerINLINE() == ePlayer)
		{
			bAdjacentOurTerritory = true;
		}
	}
	
	// check the plots just outside our visibility for plots that we own (range 2, since owned plots have a 1 visibility)
	if (pbAdjacentOurTerritory != NULL && !bAdjacentOurTerritory)
	{
		CvPlotDataRegion adjacentPlotRegion;
		findAdjacentPlots((pVisibleRegion != NULL) ? (*pVisibleRegion) : plotRegion, adjacentPlotRegion, /*bIncludeOriginal*/ false, /*iRange*/ 2);
		for (CvPlotDataRegionConstIterator it = adjacentPlotRegion.begin(); !bAdjacentOurTerritory && it != adjacentPlotRegion.end(); it++)
		{
			XYCoords xy(getXYCoords(it));
			CvPlot* pLoopPlot = GC.getMapINLINE().plotSorenINLINE(xy.iX, xy.iY);

			if (pLoopPlot->getOwnerINLINE() == ePlayer)
			{
				bAdjacentOurTerritory = true;
			}
		}
	}

	if (pbAdjacentOurTerritory != NULL)
	{
		*pbAdjacentOurTerritory = bAdjacentOurTerritory;
	}

	return iTotalValue;
}

// determines the change in fogbusting plot values by moving to new plot, returns whether new plot is adjacent our territory
bool CvUnitAI::AI_fogbustPlotDifference(const CvPlot* pNewPlot, const CvPlotDataRegion& visiblePlotRegion, int& iFogbustingLostValue, int& iFogbustingGainedValue) const
{
	PROFILE_FUNC();

	bool bAdjacentOurTerritory = false;
	
	PlayerTypes ePlayer = getOwnerINLINE();

	// get list of visible plots at new location
	CvPlotDataRegion newVisiblePlotRegion;
	getGroup()->buildVisibilityRegion(newVisiblePlotRegion, pNewPlot);

	// loop until finished with both (both are in sorted order)
	CvPlotDataRegionConstIterator itOld = visiblePlotRegion.begin();
	CvPlotDataRegionConstIterator itNew = newVisiblePlotRegion.begin();
	while (true)
	{
		bool bOldValid = (itOld != visiblePlotRegion.end());
		bool bNewValid = (itNew != newVisiblePlotRegion.end());

		// if done with both, then stop
		if (!bOldValid && !bNewValid)
		{
			break;
		}
		
		XYCoords oldXY, newXY;
		if (bOldValid)
		{
			oldXY = getXYCoords(itOld);
		}
		
		if (bNewValid)
		{
			newXY = getXYCoords(itNew);
		}
		
		// if old valid and either new finished or old less, then it is not in the new list
		if (bOldValid && (!bNewValid || oldXY < newXY))
		{
			CvPlot* pLoopPlot = GC.getMapINLINE().plotSorenINLINE(oldXY.iX, oldXY.iY);
			iFogbustingLostValue +=  AI_fogbustPlotValue(pLoopPlot, getRegionValue(itOld));

			// increment old
			itOld++;
		}
		// if new valid and either old finished or new less, then it is not in the old list
		else if (bNewValid && (!bOldValid || newXY < oldXY))
		{
			CvPlot* pLoopPlot = GC.getMapINLINE().plotSorenINLINE(newXY.iX, newXY.iY);
			iFogbustingGainedValue +=  AI_fogbustPlotValue(pLoopPlot, /*iNumObservers*/ 0);

			// check for our territory
			if (pLoopPlot->getOwnerINLINE() == ePlayer)
			{
				bAdjacentOurTerritory = true;
			}
			
			// increment new
			itNew++;
		}
		// otherwise they must be equal, it is in both
		else
		{
			FAssert(bOldValid);
			FAssert(bNewValid);
			FAssert(oldXY == newXY);

			// check for our territory
			CvPlot* pLoopPlot = GC.getMapINLINE().plotSorenINLINE(newXY.iX, newXY.iY);
			if (pLoopPlot->getOwnerINLINE() == ePlayer)
			{
				bAdjacentOurTerritory = true;
			}

			// increment both
			itOld++;
			itNew++;
		}
	}

	// check the plots just outside our visibility for plots that we own (range 2, since owned plots have a 1 visibility)
	if (!bAdjacentOurTerritory)
	{
		CvPlotDataRegion adjacentPlotRegion;
		findAdjacentPlots(newVisiblePlotRegion, adjacentPlotRegion, /*bIncludeOriginal*/ false, /*iRange*/ 2);
		for (CvPlotDataRegionConstIterator it = adjacentPlotRegion.begin(); !bAdjacentOurTerritory && it != adjacentPlotRegion.end(); it++)
		{
			XYCoords xy(getXYCoords(it));
			CvPlot* pLoopPlot = GC.getMapINLINE().plotSorenINLINE(xy.iX, xy.iY);

			if (pLoopPlot->getOwnerINLINE() == ePlayer)
			{
				bAdjacentOurTerritory = true;
			}
		}
	}

	return bAdjacentOurTerritory;
}

// Returns true if a mission was pushed...
bool CvUnitAI::AI_fogbust()
{
	PROFILE_FUNC();

	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	CvPlot* pBestFogbustPlot = NULL;

	PlayerTypes ePlayer = getOwnerINLINE();
	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
	TeamTypes eTeam = kPlayer.getTeam();
	CvSelectionGroup* pGroup = getGroup();
	
	bool bSeaUnit = (getDomainType() == DOMAIN_SEA);
	int iArea = getArea();
	
	// build region of all the plots visible to our group
	CvPlotDataRegion visiblePlotRegion;
	pGroup->buildVisibilityRegion(visiblePlotRegion);

	// are we already busting fog?
	bool bOurTerritoryInRange;
	int iFogbustingCurrentValue = AI_fogbustDataRegionValue(visiblePlotRegion, /*bRegionIsVisible*/ true, &bOurTerritoryInRange);

	// count enemy plots in visible range
	int iEnemyPlots = 0;
	for (CvPlotDataRegionConstIterator it = visiblePlotRegion.begin(); it != visiblePlotRegion.end(); it++)
	{
		XYCoords xy(getXYCoords(it));
		CvPlot* pLoopPlot = GC.getMapINLINE().plotSorenINLINE(xy.iX, xy.iY);

		if (pLoopPlot->getArea() == iArea && pLoopPlot->isVisibleEnemyUnit(ePlayer))
		{
			iEnemyPlots++;
		}
	}

	// if automated human and there are enemies about, wake up, unless we already woke up this turn and the human automated us again 
	int iGameTurn = GC.getGame().getGameTurn();
	if (isAutomated() && iEnemyPlots > 0 && m_iAutomatedAbortTurn != iGameTurn)
	{
		m_iAutomatedAbortTurn = iGameTurn;
		pGroup->setAutomateType(NO_AUTOMATE);
		pGroup->setActivityType(ACTIVITY_AWAKE);
		return true;
	}

	// if we are fogbusting some plots or enemy nearby, check adjacent plots for even better fogbusting
	if (iFogbustingCurrentValue > 0 || iEnemyPlots > 0)
	{
		// if we cannot find a better plot, stay in place rather than searching other cities
		if ((bOurTerritoryInRange && iFogbustingCurrentValue > 0) || iEnemyPlots > 0)
		{
			pBestFogbustPlot = pBestPlot = plot();
			iBestValue = iFogbustingCurrentValue;
		}
		
		// if there are enemies about, we will choose the best defensive plot
		int iCurrCombatStr = 0;				
		if (iEnemyPlots > 0)
		{
			FAssert(pBestPlot != NULL);

			// little hack to get combat str to include value if we stay put
			// we cannot use changeFortifyTurns() because that updates the UI
			bool bChangedFortifyTurns = false;
			if (!hasMoved())
			{
				if (m_iFortifyTurns < GC.getDefineINT("MAX_FORTIFY_TURNS"))
				{
					m_iFortifyTurns++;
					bChangedFortifyTurns = true;
				}
			}

			iCurrCombatStr = currCombatStr(pBestPlot, NULL);

			if (bChangedFortifyTurns)
			{
				m_iFortifyTurns--;
			}
		}
		
		for (int iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
		{
			DirectionTypes eDirection = (DirectionTypes) iI;
			
			// check non-impassable adjacent plots that do not have enemies in them
			CvPlot* pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), eDirection);
			if (pAdjacentPlot != NULL && !pAdjacentPlot->isImpassable() && !pAdjacentPlot->isVisibleEnemyUnit(ePlayer))
			{
				// count up fogbusting gain and loss
				int iFogbustingLostValue = 0;
				int iFogbustingGainedValue = 0;
				bool bNewOurTerritoryInRange = AI_fogbustPlotDifference(pAdjacentPlot, visiblePlotRegion, iFogbustingLostValue, iFogbustingGainedValue);
				
				// is this new plot in range of our territory? (or enemy present)
				if (bNewOurTerritoryInRange || (iEnemyPlots > 0))
				{
					int iFogbustingChange = iFogbustingGainedValue - iFogbustingLostValue;
					int iValue = iFogbustingCurrentValue + iFogbustingChange;

					// if start not in range and we fogbust at all, or we gain at least one plot (or there are enemies about), 
					// consider this as our best choice
					if ((!bOurTerritoryInRange && iValue > 0) || iFogbustingChange > 0 || iEnemyPlots > 0)
					{
						int iPathTurns = 0;
						if (canMoveInto(pAdjacentPlot, /*bAttack*/ false, /*bDeclareWar*/ false, /*bIgnoreLoad*/ true) &&
							generatePath(pAdjacentPlot, 0, true, &iPathTurns))
						{
							// sea units might have to path around land, to avoid long distance shuffling, devalue these
							if (iPathTurns > 3)
							{
								iValue /= 3;
							}

							// if there is an enemy in our fogbusting area, move to the best defensive bonus
							if (iEnemyPlots > 0)
							{
								int iAdjacentPlotCombatStr = currCombatStr(pAdjacentPlot, NULL);
								int iCombatDifference = iAdjacentPlotCombatStr - iCurrCombatStr;

								// if adjacent plot is better defensively (not checking for fogbusting in this case)
								// or equal and we gain significant fogbusting ability
								if ((iCombatDifference > 0) || 
									(iCombatDifference == 0 && ((iFogbustingChange >= 32 && bNewOurTerritoryInRange) || (iValue > 0 && iFogbustingCurrentValue == 0))))
								{
									iValue += (3 * iCombatDifference);
								}
								// adjacent plot is worse defensively, reject it
								else
								{
									iValue = 0;
								}
							}

#ifdef DEBUG_OUT_OF_SYNCS
	if (GC.getLogging())
	{
		if (iValue > 0 && iValue == iBestValue)
		{
			TCHAR szOut[1024];
			sprintf(szOut, "%S(%S)[%d]: AI fogbust (%d, %d), %d\n",
				getName().GetCString(),
				kPlayer.getName(),
				GC.getGameINLINE().getTurnSlice(),
				pAdjacentPlot->getX_INLINE(),
				pAdjacentPlot->getY_INLINE(),
				iValue);
			gDLL->messageControlLog(szOut);
		}
	}
#endif

							// if better, or equal and random chance choose this one
							if (iValue > iBestValue || (iValue > 0 && iValue == iBestValue && GC.getGameINLINE().getSorenRandNum(2, "AI fogbust")))
							{
								iBestValue = iValue;
								pBestFogbustPlot = pBestPlot = pAdjacentPlot;
							}
						}
					}
				}
			}
		}
	}
	
	// if there was nothing good nearby, try the plot we were headed to last time
	if (pBestPlot == NULL)
	{
		MissionAITypes eLastMissionAI = pGroup->AI_getMissionAIType();
		if (eLastMissionAI == MISSIONAI_EXPLORE)
		{
			CvPlot* pLastMissionPlot = pGroup->AI_getMissionAIPlot();
			if (pLastMissionPlot != NULL && !atPlot(pLastMissionPlot))
			{
				if (AI_plotValid(pLastMissionPlot))
				{
					// count up fogbusting gain and loss
					int iFogbustingLostValue = 0;
					int iFogbustingGainedValue = 0;
					bool bNewOurTerritoryInRange = AI_fogbustPlotDifference(pLastMissionPlot, visiblePlotRegion, iFogbustingLostValue, iFogbustingGainedValue);

					// if still a decent choice
					int iValue = iFogbustingCurrentValue + iFogbustingGainedValue - iFogbustingLostValue;
					if (iValue > 0 && bNewOurTerritoryInRange)
					{
						// can we still get there?
						int iPathTurns = 0;
						if (generatePath(pLastMissionPlot, 0, true, &iPathTurns))
						{
							iBestValue = iValue;
							pBestFogbustPlot = pLastMissionPlot;
							pBestPlot = getPathEndTurnPlot();
						}
					}
				}
			}
		}
	}

	// if we still have not found a good spot
	if (pBestPlot == NULL)
	{
		int iRange = pGroup->visibilityRange();
		
		// find a close city (slightly randomize the distance, so we do not always pick the absolute closest)
		int iClosestDistance = MAX_INT;
		CvCity* pLoopCity = NULL;
		CvCity* pStartCity = NULL;
		int iLoop, iStartLoop = 0;
		for (pLoopCity = kPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kPlayer.nextCity(&iLoop))
		{
			if (bSeaUnit || pLoopCity->getArea() == getArea())
			{
				int iDistance = stepDistance(getX_INLINE(), getY_INLINE(), pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE());
				
#ifdef DEBUG_OUT_OF_SYNCS
	if (GC.getLogging())
	{
		if (true)
		{
			TCHAR szOut[1024];
			sprintf(szOut, "%S(%S)[%d]: AI_Fogbust distance adjustment (%d, %d)\n",
				getName().GetCString(),
				kPlayer.getName(),
				GC.getGameINLINE().getTurnSlice(),
				pLoopCity->getX_INLINE(),
				pLoopCity->getY_INLINE());
			gDLL->messageControlLog(szOut);
		}
	}
#endif

				// do slight randomization
				int iAdjustment = GC.getGameINLINE().getSorenRandNum(64, "AI_Fogbust distance adjustment");
				iDistance *= 97 + iAdjustment;
				iDistance /= 128;

				if (iDistance < iClosestDistance)
				{
					iClosestDistance = iDistance;
					pStartCity = pLoopCity;
					iStartLoop = iLoop;
				}
			}
		}
		
		// loop through all the cities, starting with start city, stopping when we find a good match
		pLoopCity = pStartCity;
		iLoop = iStartLoop;
		while (pLoopCity != NULL)
		{
			if (bSeaUnit || pLoopCity->getArea() == getArea())
			{
#ifdef DEBUG_OUT_OF_SYNCS
	if (GC.getLogging())
	{
		if (true)
		{
			TCHAR szOut[1024];
			sprintf(szOut, "%S(%S)[%d]: AI_Fogbust direction (%d, %d)\n",
				getName().GetCString(),
				kPlayer.getName(),
				GC.getGameINLINE().getTurnSlice(),
				pLoopCity->getX_INLINE(),
				pLoopCity->getY_INLINE());
			gDLL->messageControlLog(szOut);
		}
	}
#endif

				// for each city, check each of the 8 sides in random order to see if there is a good fogbusting spot
				int iDirectionRand = GC.getGameINLINE().getSorenRandNum(32, "AI_Fogbust direction");
				for (int iI = 0; iI < NUM_DIRECTION_TYPES && iBestValue <= 0; iI++)
				{
					// shuffle the direction
					DirectionTypes eDirection = directionShuffle((DirectionTypes) iI, iDirectionRand);

					int iDistanceToGo = pLoopCity->getCultureLevel() + iRange + 1;
					
					// move up to iDistanceToGo from center, if possible
					CvPlot* pPreviousCheckPlot = NULL;
					CvPlot* pCheckPlot = NULL;
					CvPlot* pNextPlot = pLoopCity->plot();
					while (0 < iDistanceToGo-- &&
						(pNextPlot = plotDirection(pNextPlot->getX_INLINE(), pNextPlot->getY_INLINE(), eDirection)) != NULL)
					{
						// remember the last plot (only on our area for land units)
						if (bSeaUnit || (pCheckPlot != NULL && pCheckPlot->getArea() == getArea()))
						{
							pPreviousCheckPlot = pCheckPlot;
						}

						pCheckPlot = pNextPlot;
					}
					
					// try to find good plot (trying previous, left and right, double left and double right plots)
					for (int iPass = 0; iPass <= 5; iPass++)
					{
						// if non-sea unit and wrong area, or cannot move into
						if (pCheckPlot != NULL && ((!bSeaUnit && pCheckPlot->getArea() != getArea()) || 
							!canMoveInto(pCheckPlot, /*bAttack*/ false, /*bDeclareWar*/ false, /*bIgnoreLoad*/ true)))
						{
							pCheckPlot = NULL;
						}

						// if good plot, we do not need to try any more
						if (pCheckPlot != NULL && !pCheckPlot->isVisibleEnemyUnit(ePlayer))
						{
							break;
						}
						// if bad plot, try another
						else
						{
							switch (iPass)
							{
							// first try the previous plot
							case 0:
								pCheckPlot = pPreviousCheckPlot;
								break;
							
							// next try the left plot
							case 1:
								pPreviousCheckPlot = (pPreviousCheckPlot != NULL) ? pPreviousCheckPlot : pCheckPlot;
								if (pPreviousCheckPlot != NULL)
								{
									pCheckPlot = plotDirection(pPreviousCheckPlot->getX_INLINE(), pPreviousCheckPlot->getY_INLINE(), GC.getTurnLeftDirection(eDirection));
								}
								break;

							// next try the right plot
							case 2:
								if (pPreviousCheckPlot != NULL)
								{
									pCheckPlot = plotDirection(pPreviousCheckPlot->getX_INLINE(), pPreviousCheckPlot->getY_INLINE(), GC.getTurnRightDirection(eDirection));
								}
								break;

							// next try the double left plot
							case 3:
								if (pPreviousCheckPlot != NULL)
								{
									CvPlot* pStepOnePlot = plotDirection(pPreviousCheckPlot->getX_INLINE(), pPreviousCheckPlot->getY_INLINE(), GC.getTurnLeftDirection(eDirection));
									if (pStepOnePlot != NULL)
									{
										pCheckPlot = plotDirection(pStepOnePlot->getX_INLINE(), pStepOnePlot->getY_INLINE(), GC.getTurnLeftDirection(GC.getTurnLeftDirection(eDirection)));
									}
								}
								break;

							// next try the double right plot
							case 4:
								if (pPreviousCheckPlot != NULL)
								{
									CvPlot* pStepOnePlot = plotDirection(pPreviousCheckPlot->getX_INLINE(), pPreviousCheckPlot->getY_INLINE(), GC.getTurnRightDirection(eDirection));
									if (pStepOnePlot != NULL)
									{
										pCheckPlot = plotDirection(pStepOnePlot->getX_INLINE(), pStepOnePlot->getY_INLINE(), GC.getTurnRightDirection(GC.getTurnRightDirection(eDirection)));
									}
								}
								break;

							// thats it, nothing found
							case 5:
								pCheckPlot = NULL;
							}
						}
					}

					// if we found a good plot
					if (pCheckPlot != NULL)
					{
						// count up fogbusting gain and loss
						int iFogbustingLostValue = 0;
						int iFogbustingGainedValue = 0;
						bool bNewOurTerritoryInRange = AI_fogbustPlotDifference(pCheckPlot, visiblePlotRegion, iFogbustingLostValue, iFogbustingGainedValue);

						// are any other groups headed to the that exact plot, or the area?
						int iClosestTargetRange;
						int iMissionCount = kPlayer.AI_plotTargetMissionAIs(pCheckPlot, MISSIONAI_EXPLORE, iClosestTargetRange, pGroup);
						iClosestTargetRange = min(iClosestTargetRange, 7);

						// if at least one plot, consider this as our best choice
						int iValue = iFogbustingCurrentValue + iFogbustingGainedValue - iFogbustingLostValue;
						if (iValue > 0 && bNewOurTerritoryInRange)
						{
							int iPathTurns = 0;
							if (atPlot(pCheckPlot) || generatePath(pCheckPlot, 0, true, &iPathTurns))
							{
								iValue = (iValue * 64 * iClosestTargetRange) / (8 + iPathTurns);
								iValue /= ((16 * iMissionCount) + 1);
								iValue = (iValue > 0) ? iValue : 1;
								
#ifdef DEBUG_OUT_OF_SYNCS
	if (GC.getLogging())
	{
		if (iValue > 0 && iValue == iBestValue)
		{
			TCHAR szOut[1024];
			sprintf(szOut, "%S(%S)[%d]: AI_Fogbust (%d, %d) %d\n",
				getName().GetCString(),
				kPlayer.getName(),
				GC.getGameINLINE().getTurnSlice(),
				pCheckPlot->getX_INLINE(),
				pCheckPlot->getY_INLINE(),
				iValue);
			gDLL->messageControlLog(szOut);
		}
	}
#endif
								// if better, or equal and random chance choose this one
								if (iValue > iBestValue || (iValue > 0 && iValue == iBestValue && GC.getGameINLINE().getSorenRandNum(2, "AI fogbust")))
								{
									iBestValue = iValue;
									pBestFogbustPlot = pCheckPlot;
									pBestPlot = getPathEndTurnPlot();
								}
							}
						}
					}
				}
			}
			
			// if we found a good match, stop
			if (iBestValue > 0)
			{
				break;
			}
			
			// increment to next city
			pLoopCity = kPlayer.nextCity(&iLoop);
			
			// if we at end, start back at beginning
			if (pLoopCity == NULL)
			{
				pLoopCity = kPlayer.firstCity(&iLoop);
			}

			// if we back to start city, stop
			if (pLoopCity == pStartCity)
			{
				pLoopCity = NULL;
			}
		}
	}

	if (pBestPlot != NULL)
	{
		if (atPlot(pBestPlot))
		{
			getGroup()->pushMission(MISSION_SKIP, 0, 0, 0, false, false, MISSIONAI_EXPLORE, pBestPlot);
			return true;
		}
		else
		{
#ifdef DEBUG_OUT_OF_SYNCS
	if (GC.getLogging())
	{
		if (true)
		{
			TCHAR szOut[1024];
			sprintf(szOut, "%S(%S)[%d]: AI fogbust MISSION TO-> (%d, %d), %d\n",
				getName().GetCString(),
				kPlayer.getName(),
				GC.getGameINLINE().getTurnSlice(),
				pBestPlot->getX_INLINE(),
				pBestPlot->getY_INLINE(),
				iBestValue);
			gDLL->messageControlLog(szOut);
		}
	}
#endif

			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_EXPLORE, pBestFogbustPlot);
			return true;
		}
	}

	return false;
}

// Returns true if a mission was pushed...
bool CvUnitAI::AI_airFogbust()
{
	PROFILE_FUNC();

	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;

	TeamTypes eTeam = getTeam();
	int iArea = getArea();
	
	// we will abort if an enemy is present and we are automated
	CvUnitAI* pCarrierUnit = static_cast<CvUnitAI*>(getTransportUnit());
	int iGameTurn = GC.getGame().getGameTurn();
	bool bAbortIfEnemyFound = isAutomated() && m_iAutomatedAbortTurn != iGameTurn;
	if (!bAbortIfEnemyFound && pCarrierUnit != NULL)
	{
		bAbortIfEnemyFound = pCarrierUnit->isAutomated() && pCarrierUnit->m_iAutomatedAbortTurn != iGameTurn;
	}
	bool bEnemyFound = false;

	// randomly check a direction, stopping when we find a good one
	int iReconVisibilityRange = ((airRange() + 1) / 2);
	CvPlot* pLaunchPlot = plot();

#ifdef DEBUG_OUT_OF_SYNCS
	if (GC.getLogging())
	{
		if (true)
		{
			TCHAR szOut[1024];
			sprintf(szOut, "%S(%S)[%d]: AI_airFogbust direction (%d, %d)\n",
				getName().GetCString(),
				GET_PLAYER(getOwnerINLINE()).getName(),
				GC.getGameINLINE().getTurnSlice(),
				getX_INLINE(),
				getY_INLINE());
			gDLL->messageControlLog(szOut);
		}
	}
#endif

	int iDirectionRand = GC.getGameINLINE().getSorenRandNum(32, "AI_airFogbust direction");
	for (int iI = 0; iI < NUM_DIRECTION_TYPES && !bEnemyFound && iBestValue <= 0; iI++)
	{
		// shuffle the direction
		DirectionTypes eDirection = directionShuffle((DirectionTypes) iI, iDirectionRand);

		int iNX = GC.getPlotDirectionX()[eDirection];
		int iNY = GC.getPlotDirectionY()[eDirection];

		// diagonal directions are a shorter stepdistance, because plotdistance is used for recon
		int iRange = airRange();
		iRange = ((abs(iNX) + abs(iNY)) <= 1) ? iRange : (((2 * iRange) + 1) / 3);

		// find the furthest valid plot in this direction
		CvPlot* pCheckPlot = NULL;
		do
		{
			int iDX = iNX * iRange;
			int iDY = iNY * iRange;

			pCheckPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);
		}
		while (pCheckPlot == NULL && --iRange > 0);

		if (pCheckPlot != NULL && canReconAt(pLaunchPlot, pCheckPlot->getX_INLINE(), pCheckPlot->getY_INLINE()))
		{
			// build region of visible plots at recon location
			CvPlotRegion reconPlotRegion;
			pCheckPlot->addVisiblePlots(reconPlotRegion, iReconVisibilityRange, eTeam);

			// find fogbusting value for this recon location
			int iValue = AI_fogbustRegionValue(reconPlotRegion);

			// check for visible enemy if we will abort
			if (bAbortIfEnemyFound)
			{
				for (CvPlotRegionConstIterator it = reconPlotRegion.begin(); it != reconPlotRegion.end(); it++)
				{
					XYCoords xy(getXYCoords(it));
					CvPlot* pPlotToReveal = GC.getMapINLINE().plotSorenINLINE(xy.iX, xy.iY);

					// abort if necessary
					if (pPlotToReveal->getArea() == iArea && pPlotToReveal->isVisibleEnemyUnit(getOwnerINLINE()))
					{
						bEnemyFound = true;
						break;
					}
				}
			}

			// abort if enemy found
			if (bEnemyFound)
			{
				break;
			}

#ifdef DEBUG_OUT_OF_SYNCS
	if (GC.getLogging())
	{
		if (iValue > 0 && iValue == iBestValue)
		{
			TCHAR szOut[1024];
			sprintf(szOut, "%S(%S)[%d]: AI_airFogbust (%d, %d) %d\n",
				getName().GetCString(),
				GET_PLAYER(getOwnerINLINE()).getName(),
				GC.getGameINLINE().getTurnSlice(),
				pCheckPlot->getX_INLINE(),
				pCheckPlot->getY_INLINE(),
				iValue);
			gDLL->messageControlLog(szOut);
		}
	}
#endif

			// if better, or equal and random chance choose this one
			if (iValue > iBestValue || (iValue > 0 && iValue == iBestValue && GC.getGameINLINE().getSorenRandNum(2, "AI_airFogbust")))
			{
				iBestValue = iValue;
				pBestPlot = pCheckPlot;
			}
		}
	}

	bool bPushedMission = false;
	if (!bEnemyFound && pBestPlot != NULL)
	{
		getGroup()->pushMission(MISSION_RECON, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
		bPushedMission = true;

		if (bAbortIfEnemyFound)
		{
			// build region of visible plots at recon location
			CvPlotRegion reconPlotRegion;
			pBestPlot->addVisiblePlots(reconPlotRegion, iReconVisibilityRange, eTeam);

			// check again for enemies, post recon
			for (CvPlotRegionConstIterator it = reconPlotRegion.begin(); it != reconPlotRegion.end(); it++)
			{
				XYCoords xy(getXYCoords(it));
				CvPlot* pPlotToReveal = GC.getMapINLINE().plotSorenINLINE(xy.iX, xy.iY);

				// abort if necessary
				if (pPlotToReveal->getArea() == iArea && pPlotToReveal->isVisibleEnemyUnit(getOwnerINLINE()))
				{
					bEnemyFound = true;
					break;
				}
			}
		}
	}

	// if automated human and there are enemies about, wake up, unless we already woke up this turn and the human automated us again 
	if (bEnemyFound)
	{
		m_iAutomatedAbortTurn = iGameTurn;
		getGroup()->setAutomateType(NO_AUTOMATE);
		getGroup()->setActivityType(ACTIVITY_AWAKE);
		if (pCarrierUnit != NULL && pCarrierUnit->m_iAutomatedAbortTurn != iGameTurn)
		{
			pCarrierUnit->getGroup()->setAutomateType(NO_AUTOMATE);
			pCarrierUnit->getGroup()->setActivityType(ACTIVITY_AWAKE);
		}

		bPushedMission = true;
	}

	return bPushedMission;
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
				if (!(pAdjacentPlot->isVisibleEnemyUnit(getOwnerINLINE())))
				{
					if (generatePath(pAdjacentPlot, 0, true))
					{
						iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Patrol"));

						if (isBarbarian())
						{
							if (!(pAdjacentPlot->isOwned()))
							{
								iValue += 20000;
							}

							if (!(pAdjacentPlot->isAdjacentOwned()))
							{
								iValue += 10000;
							}
						}
						else
						{
							if (pAdjacentPlot->isRevealedGoody(getTeam()))
							{
								iValue += 100000;
							}

							if (pAdjacentPlot->getOwnerINLINE() == getOwnerINLINE())
							{
								iValue += 10000;
							}
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
						if (!(pLoopPlot->isVisibleEnemyUnit(getOwnerINLINE())))
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
						if (!(pLoopPlot->isVisibleEnemyUnit(getOwnerINLINE())))
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

									iValue += pLoopPlot->defenseModifier(getTeam(), false);

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
						if (!(pLoopPlot->isVisibleEnemyUnit(getOwnerINLINE())))
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

									iValue += pLoopPlot->defenseModifier(getTeam(), false);

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

CvPlot* CvUnitAI::AI_getBestGoodyPlot(CvPlot* pStartPlot, int iRange, CvPlot** pThisTurnPlot, bool bNoRandom)
{
	PROFILE_FUNC();

	PlayerTypes ePlayer = getOwnerINLINE();
	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
	TeamTypes eTeam = kPlayer.getTeam();
	CvSelectionGroup* pGroup = getGroup();

	int iSearchRange = AI_searchRange(iRange);

	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	CvPlot* pBestGoodyPlot = NULL;
	int iUnitsHeadedToBestGoody = 0;
	int iBestPathTurns = 0;

	int iArea = pStartPlot->getArea();

	// search in expanding range for a goody, stop after going 1 range further than the first one we find
	int iStartX = pStartPlot->getX_INLINE();
	int iStartY = pStartPlot->getY_INLINE();
	int iFoundGoodyRange = MAX_INT;
	for (int iGoodyRange = 1; (iGoodyRange - 1) <= iFoundGoodyRange && iGoodyRange <= iSearchRange; iGoodyRange++)
	{
		// search every plot at this range for a goody, pick the best one
		int iGoodyDX, iGoodyDY;
		for (int iIndex = 0; rangeIndexToXY(iGoodyRange, iIndex, iGoodyDX, iGoodyDY); iIndex++)
		{
			// is the plot on the same area?
			CvPlot* pCheckPlot = plotXY(iStartX, iStartY, iGoodyDX, iGoodyDY);
			if (pCheckPlot != NULL && pCheckPlot->getArea() == iArea)
			{
				// is there a goody?
				if (pCheckPlot->isRevealedGoody(eTeam))
				{
					bool bIsVisibleEnemy = pCheckPlot->isVisibleEnemyUnit(ePlayer);
					int iAttackOdds = (bIsVisibleEnemy) ? pGroup->AI_attackOdds(pCheckPlot, /*bPotentialEnemy*/ false) : 128;
					
					// consider it if there is no enemy or if attack odds more than 85%
					if (!bIsVisibleEnemy || (iAttackOdds > 85))
					{
						FAssert(!atPlot(pCheckPlot));
						
						// can we get there?
						int iPathTurns;
						if (pGroup->generatePath(pStartPlot, pCheckPlot, 0, true, &iPathTurns))
						{
							iFoundGoodyRange = iGoodyRange;

							int iUnitsHeadedToGoody = kPlayer.AI_plotTargetMissionAIs(pCheckPlot, MISSIONAI_EXPLORE, pGroup, 0);

							int iValue = 200000 / ((iPathTurns + 1) * ((3 * iUnitsHeadedToGoody) + 1));

#ifdef DEBUG_OUT_OF_SYNCS
	if (GC.getLogging())
	{
		if (!bNoRandom && iValue > 0 && iValue == iBestValue)
		{
			TCHAR szOut[1024];
			sprintf(szOut, "%S(%S)[%d]: AI_getBestGoodyPlot (%d, %d) %d\n",
				getName().GetCString(),
				GET_PLAYER(getOwnerINLINE()).getName(),
				GC.getGameINLINE().getTurnSlice(),
				pCheckPlot->getX_INLINE(),
				pCheckPlot->getY_INLINE(),
				iValue);
			gDLL->messageControlLog(szOut);
		}
	}
#endif

							if (iValue > iBestValue || (!bNoRandom && iValue > 0 && iValue == iBestValue && GC.getGameINLINE().getSorenRandNum(2, "AI_getBestGoodyPlot")))
							{
								iBestValue = iValue;
								pBestPlot = getPathEndTurnPlot();
								pBestGoodyPlot = pCheckPlot;
								iUnitsHeadedToBestGoody = iUnitsHeadedToGoody;
								iBestPathTurns = iPathTurns;
							}
						}
					}
				}
			}
		}
	}

	// bail if another unit is headed there and we cannot get there in this turn
	if (iUnitsHeadedToBestGoody > 0 && pBestPlot != pBestGoodyPlot)
	{
		pBestGoodyPlot = pBestPlot = NULL;
	}

	if (pThisTurnPlot != NULL)
	{
		*pThisTurnPlot = pBestPlot;
	}

	return pBestGoodyPlot;
}

// Returns true if a mission was pushed...
bool CvUnitAI::AI_goody(int iRange)
{
	PROFILE_FUNC();

	CvPlot* pBestPlot = NULL;
	CvPlot* pBestGoodyPlot = AI_getBestGoodyPlot(plot(), iRange, &pBestPlot);

	if ((pBestPlot != NULL) && (pBestGoodyPlot != NULL))
	{
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_EXPLORE, pBestGoodyPlot);
		return true;
	}

	return false;
}

int CvUnitAI::AI_explorePlotValue(const CvPlot* pPlot, bool bAddRandom, bool bIgnoreGoodies) const
{
	PROFILE_FUNC();

	// check cache if we not generating a random modifier
	if (!bAddRandom)
	{
		// try to use cached value (it is -1 if there is no cached value)
		int iCachedValue = GC.getMapINLINE().getCachedExploreValue(pPlot, this);
		if (iCachedValue >= 0)
		{
#ifdef DEBUG_OUT_OF_SYNCS
			if (GC.getLogging())
			{
				TCHAR szOut[1024];
				sprintf(szOut, "%S(%S)[%d]: AI_explorePlotValue (%d, %d) using cache-> %d\n",
					getName().GetCString(),
					GET_PLAYER(getOwnerINLINE()).getName(),
					GC.getGameINLINE().getTurnSlice(),
					pPlot->getX_INLINE(),
					pPlot->getY_INLINE(),
					iCachedValue);
				gDLL->messageControlLog(szOut);
			}
#endif

			return iCachedValue;
		}
	}
	
	// calculate the value
	int iValue = 0;
	int iExtraValue = 0;

	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());
	TeamTypes eTeam = kPlayer.getTeam();
	DomainTypes eDomain = getDomainType();

	CvPlot* pStartPlot = plot();
	bool bStartOnRiver = pStartPlot->isRiver();
	bool bDestinationRiver = false;

	if (!bIgnoreGoodies && pPlot->isRevealedGoody(getTeam()))
	{
		iValue += 200000;
	}

	BonusTypes ePlotBonus = NO_BONUS;
	int iBonusValue = 0;

	if (!(pPlot->isRevealed(getTeam(), false)))
	{
		iValue += 10000;
		
		// try to explore other civs
		if (pPlot->isOwned() && pPlot->getOwnerINLINE() != getOwnerINLINE())
		{
			iValue += 2000;
		}
	}
	else
	{
		// check for bonus resource
		ePlotBonus = pPlot->getNonObsoleteBonusType(getTeam());
		if (ePlotBonus != NO_BONUS)
		{
			iBonusValue = 10 * kPlayer.AI_bonusVal(ePlotBonus);
		}
		
		// try to follow rivers when possible
		if (pPlot->isRiver())
		{
			iExtraValue += 2000 + (pStartPlot->getRiverID() == pPlot->getRiverID()) ? 5000 : 0;
			bDestinationRiver = true;
		}
	}
	
	// add value to all plots that would be visible from this one
	int iRange = (eDomain != DOMAIN_AIR) ? visibilityRange() : ((airRange() + 1) / 2);
	CvPlotRegion visiblePlotRegion;
	pPlot->addVisiblePlots(visiblePlotRegion, iRange, eTeam);

	for (CvPlotRegionIterator it = visiblePlotRegion.begin(); it != visiblePlotRegion.end(); it++)
	{
		XYCoords xy(getXYCoords(it));
		CvPlot* pLoopPlot = GC.getMapINLINE().plotSorenINLINE(xy.iX, xy.iY);

		if (!(pLoopPlot->isRevealed(getTeam(), false)))
		{
			iValue += 1000;

			// explore nearby bonus resources (note this is not perfect, only catches bonuses in middle)
			if (ePlotBonus != NO_BONUS)
			{
				iValue += iBonusValue;
			}

			// explore coasts
			if (pPlot->isWater() != pLoopPlot->isWater())
			{
				iValue += 3500;
			}

			// try to explore other civs
			if (pLoopPlot->isOwned() && pLoopPlot->getOwnerINLINE() != getOwnerINLINE())
			{
				iValue += 2000;
			}
		}
		else
		{
			// try to follow rivers when possible
			if (pLoopPlot->isRiver())
			{
				iExtraValue += 2000 + (pStartPlot->getRiverID() == pLoopPlot->getRiverID()) ? 2000 : 0;
				bDestinationRiver = true;
			}
		}					
	}

	if (iValue > 0)
	{
		PROFILE("AI_explorePlotValue 3");

		// random factor
		if (bAddRandom)
		{
			iValue += GC.getGameINLINE().getSorenRandNum(3000, "AI explorePlotValue");
		}

		// add bonus values (for rivers, etc)
		iValue += iExtraValue;
		
		// if not following river, explore area around capital first
		if (!bDestinationRiver && eDomain == DOMAIN_LAND && kPlayer.getNumCities() == 1)
		{
			CvCity* pCapital = kPlayer.getCapitalCity();
			if (pCapital != NULL && pCapital->getArea() == getArea())
			{
				CvPlot* pCapitalPlot = pCapital->plot();

				int iDistance = plotDistance(pCapitalPlot->getX_INLINE(), pCapitalPlot->getY_INLINE(), pPlot->getX_INLINE(), pPlot->getY_INLINE());
				if (iDistance > 8)
				{
					iValue /= (iDistance > 16) ? 5 : 3;
				}
			}
		}
		
		// circumnavigation 
		if (eDomain == DOMAIN_SEA && GC.getGameINLINE().circumnavigationAvailable())
		{
			// try to go all one direction in x axis (reproducable randomness based on unitID)
			if (GC.getMapINLINE().isWrapXINLINE())
			{
				int iPreferDirection = ((getID() % 2) < 1) ? -1 : 1;
				int iDX = dxWrap(pPlot->getX_INLINE() - pStartPlot->getX_INLINE());
				int iModifier = iPreferDirection * iDX;

				if (iModifier > 0)
				{
					iValue *= min(2 + iModifier, 6);
					iValue /= 2;
				}
				else if (iModifier < 0)
				{
					iValue *= 11;
					iValue /= 16;
				}
			}

			// try to go all one direction in y axis (reproducable randomness based on unitID)
			if (GC.getMapINLINE().isWrapYINLINE())
			{
				int iPreferDirection = ((getID() % 4) < 2) ? -1 : 1;
				int iDY = dyWrap(pPlot->getY_INLINE() - pStartPlot->getY_INLINE());
				int iModifier = iPreferDirection * iDY;

				if (iModifier > 0)
				{
					iValue *= min(2 + iModifier, 6);
					iValue /= 2;
				}
				else if (iModifier < 0)
				{
					iValue *= 11;
					iValue /= 16;
				}
			}
		}
	}
	
	// save to cache if we not generating a random modifier
	if (!bAddRandom)
	{
		// update cache
		GC.getMapINLINE().setCachedExploreValue(pPlot, this, iValue);
	}

	return iValue;
}

int CvUnitAI::AI_exploreTurnPathValue(void) const
{
	int iValue = 0;
	int iPlotCount = 0;
	
	// add up the values for every plot we touch the first turn
	// look at CvSelectionGroup::getPathEndTurnPlot for logic comparison
	FAStarNode* pNode = getPathLastNode();
	while (pNode != NULL)
	{
		// m_iData2 is number of turns to get there
		if (pNode->m_iData2 == 1)
		{
			iValue += AI_explorePlotValue(GC.getMapINLINE().plotSorenINLINE(pNode->m_iX, pNode->m_iY));
			iPlotCount++;
		}

		pNode = pNode->m_pParent;
	}
	
	// reduce path value based on the number of plots added
	if (iPlotCount > 0)
	{
		iValue /= (iPlotCount + 2) / 3;
	}

	return iValue;
}

// Returns true if a mission was pushed...
bool CvUnitAI::AI_explore()
{
	PROFILE_FUNC();

	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());
	TeamTypes eTeam = kPlayer.getTeam();
	CvSelectionGroup* pGroup = getGroup();
	
	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	CvPlot* pBestExplorePlot = NULL;

	// first try the plot we were headed to last time
	MissionAITypes eLastMissionAI = pGroup->AI_getMissionAIType();
	if (eLastMissionAI == MISSIONAI_EXPLORE)
	{
		CvPlot* pLastMissionPlot = pGroup->AI_getMissionAIPlot();
		if (pLastMissionPlot != NULL)
		{
			if (!atPlot(pLastMissionPlot) && AI_plotValid(pLastMissionPlot))
			{
				int iValue = AI_explorePlotValue(pLastMissionPlot, /*bAddRandom*/ false);
				if (iValue > 0)
				{
					if (!(pLastMissionPlot->isVisibleEnemyUnit(getOwnerINLINE())))
					{
						if (kPlayer.AI_plotTargetMissionAIs(pLastMissionPlot, MISSIONAI_EXPLORE, pGroup, 3) == 0)
						{
							if (generatePath(pLastMissionPlot, MOVE_NO_ENEMY_TERRITORY, true))
							{
								iBestValue = iValue;
								pBestPlot = getPathEndTurnPlot();
								pBestExplorePlot = pLastMissionPlot;
							}
						}
					}
				}
			}
		}
	}

	
	if (pBestPlot == NULL)
	{
		int iVisibilityRange = pGroup->visibilityRange() + 1;
		int iCheckDiameter = (iVisibilityRange * 2) + 1;

		int iNumPlots = GC.getMapINLINE().numPlotsINLINE();
		std::vector<bool> abCheckedPlot;
		abCheckedPlot.resize(iNumPlots, false);

		// constants when to break off search
		const int iMaxMoreToCheck = 3;
		const int iMaxGoodOnes = 3;

		// loop through the plots in random order, if the plot is unexplored (or a goody), search all plots in range
		// once one good match is found, only search iMaxMoreToCheck more unexplored areas
		int iMoreToCheck = -1;

#ifdef DEBUG_OUT_OF_SYNCS
	if (GC.getLogging())
	{
		if (true)
		{
			TCHAR szOut[1024];
			sprintf(szOut, "%S(%S)[%d]: AI_explore (%d, %d)\n",
				getName().GetCString(),
				kPlayer.getName(),
				GC.getGameINLINE().getTurnSlice(),
				getX_INLINE(),
				getY_INLINE());
			gDLL->messageControlLog(szOut);
		}
	}
#endif

		int iRand1 = GC.getGameINLINE().getSorenRandNum(iNumPlots, "AI_explore rand1");
		int iRand2 = GC.getGameINLINE().getSorenRandNum(iNumPlots, "AI_explore rand2");
		for (int iI = 0; iI < iNumPlots; iI++)
		{
			PROFILE("AI_explore 1");

			// if we have done enough after finding a good one, stop
			if (iMoreToCheck == 0)
			{
				break;
			}

			int iShuffledIndex = intShuffle(iI, iNumPlots, iRand1, iRand2);
			CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iShuffledIndex);
			if (pLoopPlot != NULL && (!pLoopPlot->isRevealed(eTeam, false) || pLoopPlot->isRevealedGoody(eTeam)))
			{		
				// after we find iMaxGoodOnes good ones stop looking for pLoopPlot
				int iGoodOnes = 0;
				
				// in random order (but with center first), check the plots that can pontentially see this plot
				// stop when we find one good one in this area
				int iCenterX = pLoopPlot->getX_INLINE();
				int iCenterY = pLoopPlot->getY_INLINE();
				for (int iYRange = -1; iYRange < iCheckDiameter && iGoodOnes <= iMaxGoodOnes; iYRange++)
				{
					int iDY = 0;
					
					// shuffle if we not doing the center one
					if (iYRange >= 0)
					{
						iDY = intShuffle(iYRange, iCheckDiameter, iRand1, iRand2) - iVisibilityRange;
					}

					for (int iXRange = 0; iXRange < iCheckDiameter && iGoodOnes <= iMaxGoodOnes; iXRange++)
					{
						int iDX = 0;
						
						// shuffle if we not doing the center one
						if (iYRange >= 0)
						{
							iDX = intShuffle(iXRange, iCheckDiameter, iRand2, iRand1) - iVisibilityRange;
							
							// if the shuffle resulted in the center one, skip, we already did it
							if (iDX == 0 && iDY == 0)
							{
								continue;
							}
						}
						// doing center, do not need to loop here, just check this one and then break out of inner loop
						else
						{
							iXRange = iCheckDiameter;
						}

						CvPlot* pCheckPlot = plotXY(iCenterX, iCenterY, iDX, iDY);
						if (pCheckPlot != NULL)
						{
							// if we have not already checked this plot and it is 'valid'
							int iCheckPlotNum = GC.getMapINLINE().plotNumINLINE(pCheckPlot->getX_INLINE(), pCheckPlot->getY_INLINE());
							if (!abCheckedPlot[iCheckPlotNum] && AI_plotValid(pCheckPlot))
							{
								abCheckedPlot[iCheckPlotNum] = true;
								
								int iValue = AI_explorePlotValue(pCheckPlot, /*bAddRandom*/ false);
								if (iValue > 0)
								{
									if (!(pCheckPlot->isVisibleEnemyUnit(getOwnerINLINE())))
									{
										if (kPlayer.AI_plotTargetMissionAIs(pCheckPlot, MISSIONAI_EXPLORE, pGroup, 3) == 0)
										{
											int iPathTurns;
											if (!atPlot(pCheckPlot) && generatePath(pCheckPlot, MOVE_NO_ENEMY_TERRITORY, true, &iPathTurns))
											{
												iGoodOnes++;
									
												iValue /= (iPathTurns + 1);

												// add the value of the plots we actually moving to this turn
												iValue += AI_exploreTurnPathValue();
												
												if (iValue > iBestValue)
												{
													if (iMoreToCheck < 0)
													{
														iMoreToCheck = iMaxMoreToCheck;
													}
													
													iBestValue = iValue;
													pBestPlot = getPathEndTurnPlot();
													pBestExplorePlot = pCheckPlot;
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
#ifdef DEBUG_OUT_OF_SYNCS
	if (GC.getLogging())
	{
		if (true)
		{
			TCHAR szOut[1024];
			sprintf(szOut, "%S(%S)[%d]: AI_explore MISSION TO-> (%d, %d), %d\n",
				getName().GetCString(),
				kPlayer.getName(),
				GC.getGameINLINE().getTurnSlice(),
				pBestPlot->getX_INLINE(),
				pBestPlot->getY_INLINE(),
				iBestValue);
			gDLL->messageControlLog(szOut);
		}
	}
#endif
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

	CvPlot* pBestPlot = NULL;
	CvPlot* pBestExplorePlot = AI_getBestExplorePlot(iRange, &pBestPlot);

	if ((pBestPlot != NULL) && (pBestExplorePlot != NULL))
	{
#ifdef DEBUG_OUT_OF_SYNCS
	if (GC.getLogging())
	{
		if (true)
		{
			TCHAR szOut[1024];
			sprintf(szOut, "%S(%S)[%d]: AI_exploreRange MISSION TO-> (%d, %d)\n",
				getName().GetCString(),
				GET_PLAYER(getOwnerINLINE()).getName(),
				GC.getGameINLINE().getTurnSlice(),
				pBestPlot->getX_INLINE(),
				pBestPlot->getY_INLINE());
			gDLL->messageControlLog(szOut);
		}
	}
#endif
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), MOVE_NO_ENEMY_TERRITORY, false, false, MISSIONAI_EXPLORE, pBestExplorePlot);
		return true;
	}

	return false;
}

CvPlot* CvUnitAI::AI_getBestExplorePlot(int iRange, CvPlot** pThisTurnPlot, bool bNoRandom)
{
	PROFILE_FUNC();

	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());

	int iSearchRange = AI_searchRange(iRange);

	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	CvPlot* pBestExplorePlot = NULL;
	
	// search in expanding range for best explore plot, stop after going 2 range further than the first good one we find
	int iFoundRange = MAX_INT;
	int iStartX = getX_INLINE();
	int iStartY = getY_INLINE();
	for (int iCheckRange = 1; (iCheckRange - 2) <= iFoundRange && iCheckRange <= iSearchRange; iCheckRange++)
	{
		int iDX, iDY;
		for (int iIndex = 0; rangeIndexToXY(iCheckRange, iIndex, iDX, iDY); iIndex++)
		{
			PROFILE("AI_getBestExplorePlot 1");

			CvPlot* pLoopPlot = plotXY(iStartX, iStartY, iDX, iDY);
			if (pLoopPlot != NULL)
			{
				if (AI_plotValid(pLoopPlot))
				{
					int iValue = AI_explorePlotValue(pLoopPlot);

					if (iValue > 0)
					{
						if (!(pLoopPlot->isVisibleEnemyUnit(getOwnerINLINE())))
						{
							PROFILE("AI_getBestExplorePlot 2");

							if (kPlayer.AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_EXPLORE, getGroup(), 3) == 0)
							{
								PROFILE("AI_getBestExplorePlot 3");
								
								int iPathTurns;
								if (!atPlot(pLoopPlot) && generatePath(pLoopPlot, MOVE_NO_ENEMY_TERRITORY, true, &iPathTurns))
								{
									// if we can get there in one turn, value higher
									if (iPathTurns == 1)
									{
										iValue *= 16;
										iValue /= 11;
									}
									
									if (iPathTurns <= iRange)
									{										
										// add the value of the plots we actually moving to this turn
										iValue += AI_exploreTurnPathValue();
										
#ifdef DEBUG_OUT_OF_SYNCS
	if (GC.getLogging())
	{
		if (!bNoRandom && iValue > 0 && iValue == iBestValue)
		{
			TCHAR szOut[1024];
			sprintf(szOut, "%S(%S)[%d]: AI_getBestExplorePlot (%d, %d), %d\n",
				getName().GetCString(),
				kPlayer.getName(),
				GC.getGameINLINE().getTurnSlice(),
				pBestExplorePlot->getX_INLINE(),
				pBestExplorePlot->getY_INLINE(),
				iValue);
			gDLL->messageControlLog(szOut);
		}
	}
#endif
	
										if (iValue > iBestValue || (!bNoRandom && iValue > 0 && iValue == iBestValue && GC.getGameINLINE().getSorenRandNum(2, "AI_getBestExplorePlot")))
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
				// otherwise, if this is 1 distance, and we are land cargo, is this a good place to drop off?
				// note, under normal conditions, AI_getBestExplorePlot is not called when isCargo() is true
				//else if ((max(abs(iDX), abs(iDY)) <= 1) && isCargo() && getDomainType() == DOMAIN_LAND && !pLoopPlot->isWater())
				//{
				//	CvPlot* pExplorePlot = pLoopPlot;
				//	TeamTypes eTeam = kPlayer.getTeam();

				//	int iValue = pLoopPlot->isRevealedGoody(eTeam) ? 200000 : 0;
				//	
				//	// if no goody right there on the coast
				//	if (iValue == 0)
				//	{
				//		CvArea* pArea = pLoopPlot->area(); 
				//		int iAreaExporers = kPlayer.AI_areaMissionAIs(pArea, MISSIONAI_EXPLORE, getGroup());
				//		int iAreaNeededExplorers = kPlayer.AI_neededExplorers(pArea);
				//		
				//		iValue = (iAreaNeededExplorers - iAreaExporers) * 10000;
				//		
				//		// if area does not need explorers and has none, is there a goody nearby?
				//		bool bFoundGoody = false;
				//		if (iAreaNeededExplorers == 0 && iAreaExporers == 0)
				//		{
				//			pExplorePlot = pCargoUnit->AI_getBestGoodyPlot(pLoopPlot, 5);
				//		}
				//	}

				//	if (iValue > iBestValue || (iValue > 0 && iValue == iBestValue && GC.getGameINLINE().getSorenRandNum(2, "AI_getBestExplorePlot")))
				//	{
				//		if (canMoveInto(pLoopPlot, /*bAttack*/ false, /*bDeclareWar*/ false, /*bIgnoreLoad*/ true))
				//		{
				//			iBestValue = iValue;
				//			pBestPlot = pLoopPlot;
				//			pBestExplorePlot = pExplorePlot;
				//		}
				//	}
				//}
			}
		}
	}

	if (pThisTurnPlot != NULL)
	{
		*pThisTurnPlot = pBestPlot;
	}

	return pBestExplorePlot;
}

// Returns true if a mission was pushed...
bool CvUnitAI::AI_airExplore()
{
	PROFILE_FUNC();

	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;

	// check in all 8 directions
	for (int iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
	{
		DirectionTypes eDirection = (DirectionTypes) iI;
		int iNX = GC.getPlotDirectionX()[eDirection];
		int iNY = GC.getPlotDirectionY()[eDirection];

		// try once at full range and once at half range
		for (int iRangeDivisor = 1; iRangeDivisor <= 2; iRangeDivisor++)
		{
			// diagonal directions are a shorter stepdistance, because plotdistance is used for recon
			int iRange = airRange() / iRangeDivisor;
			iRange = ((abs(iNX) + abs(iNY)) <= 1) ? iRange : (((2 * iRange) + 1) / 3);

			// find the furthest valid plot in this direction
			CvPlot* pCheckPlot = NULL;
			do
			{
				int iDX = iNX * iRange;
				int iDY = iNY * iRange;

				pCheckPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);
			}
			while (pCheckPlot == NULL && --iRange > 0);

			if (pCheckPlot != NULL && canReconAt(plot(), pCheckPlot->getX_INLINE(), pCheckPlot->getY_INLINE()))
			{
				int iValue = AI_explorePlotValue(pCheckPlot, /*bAddRandom*/ false, /*bIgnoreGoodies*/ true);

#ifdef DEBUG_OUT_OF_SYNCS
	if (GC.getLogging())
	{
		if (iValue > 0 && iValue == iBestValue)
		{
			TCHAR szOut[1024];
			sprintf(szOut, "%S(%S)[%d]: AI_airExplore (%d, %d), %d\n",
				getName().GetCString(),
				GET_PLAYER(getOwnerINLINE()).getName(),
				GC.getGameINLINE().getTurnSlice(),
				pCheckPlot->getX_INLINE(),
				pCheckPlot->getY_INLINE(),
				iValue);
			gDLL->messageControlLog(szOut);
		}
	}
#endif
				
				// if better, or equal and random chance choose this one
				if (iValue > iBestValue || (iValue > 0 && iValue == iBestValue && GC.getGameINLINE().getSorenRandNum(2, "AI_airExplore")))
				{
					iBestValue = iValue;
					pBestPlot = pCheckPlot;
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		getGroup()->pushMission(MISSION_RECON, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
		return true;
	}

	return false;
}

// Returns true if a mission was pushed...
bool CvUnitAI::AI_targetCity()
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
			if (!atPlot(pTargetCity->plot()) && generatePath(pTargetCity->plot(), 0, true))
			{
				pBestCity = pTargetCity;
			}
		}
	}

	if (pBestCity == NULL)
	{
		for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
				{
					if (AI_plotValid(pLoopCity->plot()) && AI_potentialEnemy(GET_PLAYER((PlayerTypes)iI).getTeam(), pLoopCity->plot()))
					{
						if (!atPlot(pLoopCity->plot()) && generatePath(pLoopCity->plot(), 0, true, &iPathTurns))
						{
							iValue = GET_PLAYER(getOwnerINLINE()).AI_targetCityValue(pLoopCity, false);

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

		for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
		{
			pAdjacentPlot = plotDirection(pBestCity->getX_INLINE(), pBestCity->getY_INLINE(), ((DirectionTypes)iI));

			if (pAdjacentPlot != NULL)
			{
				if (AI_plotValid(pAdjacentPlot))
				{
					if (!(pAdjacentPlot->isVisibleEnemyUnit(getOwnerINLINE())))
					{
						if (generatePath(pAdjacentPlot, 0, true, &iPathTurns))
						{
							iValue = max(0, (pAdjacentPlot->defenseModifier(getTeam(), false) + 100));

							if (!(pAdjacentPlot->isRiverCrossing(directionXY(pAdjacentPlot, pBestCity->plot()))))
							{
								iValue += (12 * -(GC.getRIVER_ATTACK_MODIFIER()));
							}
							
                            if (!atWar(getTeam(), pAdjacentPlot->getTeam()))
                            {
                                iValue += 100;                                
							}

							iValue = max(1, iValue);

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

		if (pBestPlot != NULL)
		{
			FAssert(!(pBestCity->at(pBestPlot))); // no suicide missions...
			if (atPlot(pBestPlot))
			{
				getGroup()->pushMission(MISSION_SKIP);
				return true;
			}
			else
			{
#ifdef DEBUG_WAR_ACTIONS
				// debugging
				if (true)
				{
					CvSelectionGroup* pCurrentGroup = getGroup();
					CvWString szUnitName = GC.getUnitInfo(m_eUnitType).getDescription();
					DEBUGLOG("AI_targetCity: %S(%d) headed to %S. Group(%d) [size %d%s]\n", szUnitName.GetCString(), shortenID(m_iID), 
						pBestCity->getName().GetCString(),
						shortenID(m_iGroupID), pCurrentGroup->getNumUnits(), pCurrentGroup->AI_isForceSeparate() ? ",WILL_SEPARATE" : "");
				}
#endif
				getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
				return true;
			}
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_targetBarbCity()
{
	PROFILE_FUNC();

	CvCity* pLoopCity;
	CvCity* pBestCity;
	CvPlot* pAdjacentPlot;
	CvPlot* pBestPlot;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iLoop;
	int iI;

	if (isBarbarian())
	{
		return false;
	}

	iBestValue = 0;
	pBestCity = NULL;

	for (pLoopCity = GET_PLAYER(BARBARIAN_PLAYER).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(BARBARIAN_PLAYER).nextCity(&iLoop))
	{
		if (AI_plotValid(pLoopCity->plot()))
		{
			if (pLoopCity->isRevealed(getTeam(), false))
			{
				if (!atPlot(pLoopCity->plot()) && generatePath(pLoopCity->plot(), 0, true, &iPathTurns))
				{
					if (iPathTurns < 10)
					{
						iValue = GET_PLAYER(getOwnerINLINE()).AI_targetCityValue(pLoopCity, false);

						iValue *= 1000;

						iValue /= (iPathTurns + 1);

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

		for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
		{
			pAdjacentPlot = plotDirection(pBestCity->getX_INLINE(), pBestCity->getY_INLINE(), ((DirectionTypes)iI));

			if (pAdjacentPlot != NULL)
			{
				if (AI_plotValid(pAdjacentPlot))
				{
					if (!(pAdjacentPlot->isVisibleEnemyUnit(getOwnerINLINE())))
					{
						if (generatePath(pAdjacentPlot, 0, true, &iPathTurns))
						{
							iValue = max(0, (pAdjacentPlot->defenseModifier(getTeam(), false) + 100));

							if (!(pAdjacentPlot->isRiverCrossing(directionXY(pAdjacentPlot, pBestCity->plot()))))
							{
								iValue += (10 * -(GC.getRIVER_ATTACK_MODIFIER()));
							}

							iValue = max(1, iValue);

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

		if (pBestPlot != NULL)
		{
			FAssert(!(pBestCity->at(pBestPlot))); // no suicide missions...
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
		
		// do not bombard cities if we have overwelming odds
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
			pLoopPlot	= plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (AI_plotValid(pLoopPlot))
				{
					if (pLoopPlot->isCity() || (pLoopPlot->isCity(true) && pLoopPlot->isVisibleEnemyUnit(getOwnerINLINE())))
					{
						if (AI_potentialEnemy(pLoopPlot->getTeam(), pLoopPlot))
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
			pLoopPlot	= plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (AI_plotValid(pLoopPlot))
				{
					if (pLoopPlot->isVisibleEnemyUnit(getOwnerINLINE()) || (pLoopPlot->isCity() && AI_potentialEnemy(pLoopPlot->getTeam(), pLoopPlot)))
					{
						if (!atPlot(pLoopPlot) && ((bFollow) ? canMoveInto(pLoopPlot, true) : (generatePath(pLoopPlot, 0, true, &iPathTurns) && (iPathTurns <= iRange))))
						{
							if (pLoopPlot->getNumVisibleEnemyDefenders(getOwnerINLINE()) >= iMinStack)
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
    	int iEnemyStrength = GET_PLAYER(getOwnerINLINE()).AI_getEnemyPlotStrength(plot(), iRange, false, false);
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
					if (pLoopPlot->isVisibleEnemyUnit(getOwnerINLINE()) || (pLoopPlot->isCity() && AI_potentialEnemy(pLoopPlot->getTeam(), pLoopPlot)))
					{
						if (!atPlot(pLoopPlot) && (generatePath(pLoopPlot, 0, true, &iPathTurns) && (iPathTurns <= iRange)))
						{
							if (pLoopPlot->getNumVisibleEnemyDefenders(getOwnerINLINE()) > 0)
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
bool CvUnitAI::AI_blockade()
{
	PROFILE_FUNC();

	CvCity* pCity;
	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	CvPlot* pBestBlockadePlot;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iI;

	iBestValue = 0;
	pBestPlot = NULL;
	pBestBlockadePlot = NULL;

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (AI_plotValid(pLoopPlot))
		{
			if (potentialWarAction(pLoopPlot))
			{
				pCity = pLoopPlot->getWorkingCity();

				if (pCity != NULL)
				{
					if (pCity->isCoastal(GC.getDefineINT("MIN_WATER_SIZE_FOR_OCEAN")))
					{
						if (!(pCity->isBarbarian()))
						{
							if (!(pLoopPlot->isVisibleEnemyUnit(getOwnerINLINE())))
							{
								if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_BLOCKADE, getGroup(), 2) == 0)
								{
									if (generatePath(pLoopPlot, 0, true, &iPathTurns))
									{
										iValue = 1;

										iValue += min(pCity->getPopulation(), pCity->countNumWaterPlots());

										iValue += GET_PLAYER(getOwnerINLINE()).AI_adjacentPotentialAttackers(pCity->plot());

										iValue += (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pCity->plot(), MISSIONAI_ASSAULT, getGroup(), 2) * 3);

										if (canBombard(pLoopPlot))
										{
											iValue *= 2;
										}

										iValue *= 1000;

										iValue /= (iPathTurns + 1);

										// if not at war with this plot owner, then devalue plot if we already inside this owner's borders
										// (because declaring war will pop us some unknown distance away)
										if (!atWar(getTeam(), pLoopPlot->getTeam()) && plot()->getTeam() == pLoopPlot->getTeam())
										{
											iValue /= 10;
										}

										if (iValue > iBestValue)
										{
											iBestValue = iValue;
											pBestPlot = getPathEndTurnPlot();
											pBestBlockadePlot = pLoopPlot;
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

	if ((pBestPlot != NULL) && (pBestBlockadePlot != NULL))
	{
		if (atPlot(pBestBlockadePlot) && !atWar(getTeam(), pBestBlockadePlot->getTeam()))
		{
			getGroup()->groupDeclareWar(pBestBlockadePlot, true);
		}
		
		if (atPlot(pBestBlockadePlot))
		{
			if (canBombard(plot()))
			{
				getGroup()->pushMission(MISSION_BOMBARD, -1, -1, 0, false, false, MISSIONAI_BLOCKADE, pBestBlockadePlot);
			}

			getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, (getGroup()->getLengthMissionQueue() > 0), false, MISSIONAI_BLOCKADE, pBestBlockadePlot);
			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_BLOCKADE, pBestBlockadePlot);
			return true;
		}
	}

	return false;
}

// Returns true if a mission was pushed...
bool CvUnitAI::AI_seaBombardRange(int iMaxRange)
{
	PROFILE_FUNC();
	
	// cached values
	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());
	TeamTypes eTeam = kPlayer.getTeam();
	CvPlot* pPlot = plot();
	CvSelectionGroup* pGroup = getGroup();
	int iX = pPlot->getX_INLINE(), iY = pPlot->getY_INLINE();
	
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
	
	// iterator
	for (int iRange = 0; iRange <= iMaxRange; iRange++)
	{
		int iDX, iDY;
		for (int iIndex = 0; rangeIndexToXY(iRange, iIndex, iDX, iDY); iIndex++)
		{
			CvPlot* pLoopPlot = plotXY(plot()->getX_INLINE(), plot()->getY_INLINE(), iDX, iDY);
			if (pLoopPlot != NULL)
			{
				CvCity* pBombardCity = bombardTarget(pLoopPlot);

				if (pBombardCity != NULL && atWar(eTeam, pBombardCity->getTeam()) && pBombardCity->getDefenseDamage() < ((GC.getMAX_CITY_DEFENSE_DAMAGE()*5)/6))
				{
					int iPathTurns;
					if (generatePath(pLoopPlot, 0, true, &iPathTurns))
					{
						int iValue = 1;
						
						iValue += (kPlayer.AI_plotTargetMissionAIs(pBombardCity->plot(), MISSIONAI_ASSAULT, pGroup, 2) * 3);

						iValue *= 1000;

						iValue /= (iPathTurns + 1);

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
	
	if ((pBestPlot != NULL) && (pBestBombardPlot != NULL))
	{
		if (atPlot(pBestBombardPlot))
		{
			// if we are at the plot from which to bombard, and we have a unit that can bombard this turn, do it
			if (bBombardUnitCanBombardNow && pGroup->canBombard(pBestBombardPlot))
			{
				getGroup()->pushMission(MISSION_BOMBARD, -1, -1, 0, false, false, MISSIONAI_BLOCKADE, pBestBombardPlot);

				// if city bombarded enough, wake up any units that were waiting to bombard this city
				CvCity* pBombardCity = bombardTarget(pBestBombardPlot); // is NULL if city cannot be bombarded any more
				if (pBombardCity == NULL || pBombardCity->getDefenseDamage() < ((GC.getMAX_CITY_DEFENSE_DAMAGE()*5)/6))
				{
					kPlayer.AI_wakePlotTargetMissionAIs(pBestBombardPlot, MISSIONAI_BLOCKADE, getGroup());
				}
			}
			// otherwise, skip until next turn, when we will surely bombard
			else
			{
				getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_BLOCKADE, pBestBombardPlot);
			}

			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_BLOCKADE, pBestBombardPlot);
			return true;
		}
	}

	return false;
}



// Returns true if a mission was pushed...
bool CvUnitAI::AI_pillage(int iBonusValueThreshold)
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	CvPlot* pBestPillagePlot;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iI;

	iBestValue = 0;
	pBestPlot = NULL;
	pBestPillagePlot = NULL;

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (AI_plotValid(pLoopPlot) && !(pLoopPlot->isBarbarian()))
		{
			if (potentialWarAction(pLoopPlot))
			{
			    CvCity * pWorkingCity = pLoopPlot->getWorkingCity();

			    if (pWorkingCity != NULL)
			    {
                    if (!(pWorkingCity == area()->getTargetCity(getOwnerINLINE())) && canPillage(pLoopPlot))
                    {
                        if (!(pLoopPlot->isVisibleEnemyUnit(getOwnerINLINE())))
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
									if (!atWar(getTeam(), pLoopPlot->getTeam()) && plot()->getTeam() == pLoopPlot->getTeam())
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
		if (atPlot(pBestPillagePlot) && !atWar(getTeam(), pBestPillagePlot->getTeam()))
		{
			//getGroup()->groupDeclareWar(pBestPillagePlot, true);
			// rather than declare war, just find something else to do, since we may already be deep in enemy territory
			return false;
		}
		
		if (atPlot(pBestPillagePlot))
		{
			if (atWar(getTeam(), pBestPillagePlot->getTeam()))
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


// Returns true if a mission was pushed...
bool CvUnitAI::AI_pillageRange(int iRange, int iBonusValueThreshold)
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
				if (AI_plotValid(pLoopPlot) && !(pLoopPlot->isBarbarian()))
				{
					if (potentialWarAction(pLoopPlot))
					{
                        CvCity * pWorkingCity = pLoopPlot->getWorkingCity();

                        if (pWorkingCity != NULL)
                        {
                            if (!(pWorkingCity == area()->getTargetCity(getOwnerINLINE())) && canPillage(pLoopPlot))
                            {
                                if (!(pLoopPlot->isVisibleEnemyUnit(getOwnerINLINE())))
                                {
                                    if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_PILLAGE, getGroup()) == 0)
                                    {
                                        if (generatePath(pLoopPlot, 0, true, &iPathTurns))
                                        {
                                            if (getPathLastNode()->m_iData1 == 0)
                                            {
                                                iPathTurns++;
                                            }

                                            if (iPathTurns <= iRange)
                                            {
                                                iValue = AI_pillageValue(pLoopPlot);

                                                iValue *= 1000;

                                                iValue /= (iPathTurns + 1);

												// if not at war with this plot owner, then devalue plot if we already inside this owner's borders
												// (because declaring war will pop us some unknown distance away)
												if (!atWar(getTeam(), pLoopPlot->getTeam()) && plot()->getTeam() == pLoopPlot->getTeam())
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
		if (atPlot(pBestPillagePlot) && !atWar(getTeam(), pBestPillagePlot->getTeam()))
		{
			//getGroup()->groupDeclareWar(pBestPillagePlot, true);
			// rather than declare war, just find something else to do, since we may already be deep in enemy territory
			return false;
		}
		
		if (atPlot(pBestPillagePlot))
		{
			if (atWar(getTeam(), pBestPillagePlot->getTeam()))
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


// Returns true if a mission was pushed...
bool CvUnitAI::AI_found()
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	CvPlot* pBestFoundPlot;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iI;

	iBestValue = 0;
	pBestPlot = NULL;
	pBestFoundPlot = NULL;

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (AI_plotValid(pLoopPlot) && (pLoopPlot != plot() || GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(pLoopPlot, 1) <= pLoopPlot->plotCount(PUF_canDefend, -1, -1, getOwnerINLINE())))
		{
			if (canFound(pLoopPlot))
			{
				iValue = pLoopPlot->getFoundValue(getOwnerINLINE());

				if (iValue > 0)
				{
					if (!(pLoopPlot->isVisibleEnemyUnit(getOwnerINLINE())))
					{
						if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_FOUND, getGroup(), 3) == 0)
						{
							if (generatePath(pLoopPlot, MOVE_SAFE_TERRITORY, true, &iPathTurns))
							{
								iValue *= 1000;

								iValue /= (iPathTurns + 1);

								if (iValue > iBestValue)
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

	if ((pBestPlot != NULL) && (pBestFoundPlot != NULL))
	{
		if (atPlot(pBestFoundPlot))
		{
			getGroup()->pushMission(MISSION_FOUND, -1, -1, 0, false, false, MISSIONAI_FOUND, pBestFoundPlot);
			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), MOVE_SAFE_TERRITORY, false, false, MISSIONAI_FOUND, pBestFoundPlot);
			return true;
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
	int iValue;
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
						iValue = pLoopPlot->getFoundValue(getOwnerINLINE());

						if (iValue > iBestValue)
						{
							if (!(pLoopPlot->isVisibleEnemyUnit(getOwnerINLINE())))
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


// Returns true if a mission was pushed...
bool CvUnitAI::AI_assaultSeaTransport()
{
	PROFILE_FUNC();

	CvCity* pCity;
	CvPlot* pLoopPlot;
	CvPlot* pAdjacentPlot;
	CvPlot* pBestPlot;
	CvPlot* pBestAssaultPlot;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iI, iJ;

	bool bIsAttackCity = (getUnitAICargo(UNITAI_ATTACK_CITY) > 0);
	
	FAssert(getCargo() > 0);
	FAssert(bIsAttackCity || getUnitAICargo(UNITAI_ATTACK) > 0);

	if (!canCargoAllMove())
	{
		return false;
	}
	
	// make sure attack city transports in cities are defended by at least one non-transport unit, if we can build one
	if (bIsAttackCity)
	{
		pCity = plot()->getPlotCity();
		if (pCity != NULL && pCity->AI_bestUnitAI(UNITAI_ESCORT_SEA) != NO_UNIT)
		{
			if (getGroup()->countNumUnitAIType(UNITAI_ESCORT_SEA) < 1)
			{
				return false;
			}
		}
	}

	iBestValue = 0;
	pBestPlot = NULL;
	pBestAssaultPlot = NULL;

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (pLoopPlot->isCoastalLand())
		{
			if (pLoopPlot->isOwned())
			{
				if (!(pLoopPlot->isBarbarian()) || GET_PLAYER(getOwnerINLINE()).isMinorCiv())
				{
					if (isPotentialEnemy(getTeam(), pLoopPlot->getTeam()))
					{
						if (pLoopPlot->area()->getCitiesPerPlayer(pLoopPlot->getOwnerINLINE()) > 0)
						{
							if (generatePath(pLoopPlot, 0, true, &iPathTurns))
							{
								iValue = 1;

								iValue += (AI_pillageValue(pLoopPlot) * 10);

								iValue += (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_ASSAULT, getGroup()) * 100);

								pCity = pLoopPlot->getPlotCity();

								if (pCity == NULL)
								{
									for (iJ = 0; iJ < NUM_DIRECTION_TYPES; iJ++)
									{
										pAdjacentPlot = plotDirection(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), ((DirectionTypes)iJ));

										if (pAdjacentPlot != NULL)
										{
											pCity = pAdjacentPlot->getPlotCity();

											if (pCity != NULL)
											{
												if (pCity->getOwnerINLINE() == pLoopPlot->getOwnerINLINE())
												{
													break;
												}
												else
												{
													pCity = NULL;
												}
											}
										}
									}
								}

								if (pCity != NULL)
								{
									FAssert(isPotentialEnemy(getTeam(), pCity->getTeam()));

									iValue += 2000;
									iValue += (GET_PLAYER(getOwnerINLINE()).AI_adjacentPotentialAttackers(pCity->plot()) * 200);

									if (iPathTurns == 1)
									{
										iValue += GC.getGameINLINE().getSorenRandNum(2000, "AI Assault");
									}
								}

								FAssert(iPathTurns > 0);

								if (iPathTurns == 1)
								{
									iValue *= 3;
								}

								iValue /= (pLoopPlot->getNumVisibleEnemyDefenders(getOwnerINLINE()) + 1);

								iValue /= max(1, (4 - pLoopPlot->area()->getCitiesPerPlayer(pLoopPlot->getOwnerINLINE())));

								if (pLoopPlot->isCity())
								{
									iValue /= 10;
								}

								iValue *= 1000;

								// if more than 3 turns to get there, then put some randomness into our preference of distance
								// +/- 33%
								if (iPathTurns > 3)
								{
									int iPathAdjustment = GC.getGameINLINE().getSorenRandNum(67, "AI Assault Target");

									iPathTurns *= 66 + iPathAdjustment;
									iPathTurns /= 100;
								}

								iValue /= (iPathTurns + 1);

								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									pBestPlot = getPathEndTurnPlot();
									pBestAssaultPlot = pLoopPlot;
								}
							}
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestAssaultPlot != NULL))
	{
		if ((pBestPlot == pBestAssaultPlot) || (stepDistance(pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), pBestAssaultPlot->getX_INLINE(), pBestAssaultPlot->getY_INLINE()) == 1))
		{
			if (atPlot(pBestAssaultPlot))
			{
				getGroup()->unloadAll(); // XXX is this dangerous (not pushing a mission...) XXX air units?
				return true;
			}
			else
			{
				getGroup()->pushMission(MISSION_MOVE_TO, pBestAssaultPlot->getX_INLINE(), pBestAssaultPlot->getY_INLINE(), 0, false, false, MISSIONAI_ASSAULT, pBestAssaultPlot);
				return true;
			}
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_ASSAULT, pBestAssaultPlot);
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_settlerSeaTransport()
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pLoopPlot;
	CvPlot* pPlot;
	CvPlot* pBestPlot;
	CvPlot* pBestFoundPlot;
	bool bValid;
	int iValue;
	int iBestValue;
	int iI;

	FAssert(getCargo() > 0);
	FAssert(getUnitAICargo(UNITAI_SETTLE) > 0);

	if (!canCargoAllMove())
	{
		return false;
	}

	pPlot = plot();

	iBestValue = 0;
	pBestPlot = NULL;
	pBestFoundPlot = NULL;

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (pLoopPlot->isCoastalLand())
		{
			iValue = pLoopPlot->getFoundValue(getOwnerINLINE());

			if (iValue > iBestValue)
			{
				bValid = false;

				pUnitNode = pPlot->headUnitNode();

				while (pUnitNode != NULL)
				{
					pLoopUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = pPlot->nextUnitNode(pUnitNode);

					if (pLoopUnit->getTransportUnit() == this)
					{
						if (pLoopUnit->canFound(pLoopPlot))
						{
							bValid = true;
							break;
						}
					}
				}

				if (bValid)
				{
					if (!(pLoopPlot->isVisibleEnemyUnit(getOwnerINLINE())))
					{
						if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_FOUND, getGroup(), 4) == 0)
						{
							if (generatePath(pLoopPlot, 0, true))
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

	if ((pBestPlot != NULL) && (pBestFoundPlot != NULL))
	{
		if ((pBestPlot == pBestFoundPlot) || (stepDistance(pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), pBestFoundPlot->getX_INLINE(), pBestFoundPlot->getY_INLINE()) == 1))
		{
			if (atPlot(pBestFoundPlot))
			{
				unloadAll(); // XXX is this dangerous (not pushing a mission...) XXX air units?
				return true;
			}
			else
			{
				getGroup()->pushMission(MISSION_MOVE_TO, pBestFoundPlot->getX_INLINE(), pBestFoundPlot->getY_INLINE(), 0, false, false, MISSIONAI_FOUND, pBestFoundPlot);
				return true;
			}
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_FOUND, pBestFoundPlot);
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
	int iI, iJ;

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

				for (iJ = 0; iJ < GC.getNumReligionInfos(); iJ++)
				{
					if (pMissionaryUnit->canSpread(pLoopPlot, ((ReligionTypes)iJ)))
					{
						if (GET_PLAYER(getOwnerINLINE()).getStateReligion() == ((ReligionTypes)iJ))
						{
							iValue += 3;
						}

						if (GET_PLAYER(getOwnerINLINE()).hasHolyCity((ReligionTypes)iJ))
						{
							iValue++;
						}
					}
				}

				if (iValue > 0)
				{
					if (!(pLoopPlot->isVisibleEnemyUnit(getOwnerINLINE())))
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

								if (pCity->getReligionCount() == 0)
								{
									iValue *= 2;
								}

								iValue /= (pCity->getReligionCount() + 1);

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
		if ((pBestPlot == pBestSpreadPlot) || (stepDistance(pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), pBestSpreadPlot->getX_INLINE(), pBestSpreadPlot->getY_INLINE()) == 1))
		{
			if (atPlot(pBestSpreadPlot))
			{
				unloadAll(); // XXX is this dangerous (not pushing a mission...) XXX air units?
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
bool CvUnitAI::AI_specialSeaTransportSpy()
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	CvPlot* pBestSpyPlot;
	PlayerTypes eBestPlayer;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iI;

	FAssert(getCargo() > 0);
	FAssert(getUnitAICargo(UNITAI_SPY) > 0);

	if (!canCargoAllMove())
	{
		return false;
	}

	iBestValue = 0;
	eBestPlayer = NO_PLAYER;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() != getTeam())
			{
				if (GET_PLAYER(getOwnerINLINE()).AI_getAttitude((PlayerTypes)iI) <= ATTITUDE_ANNOYED)
				{
					iValue = GET_PLAYER((PlayerTypes)iI).getTotalPopulation();

					if (iValue > iBestValue)
					{
						iBestValue = iValue;
						eBestPlayer = ((PlayerTypes)iI);
					}
				}
			}
		}
	}

	if (eBestPlayer == NO_PLAYER)
	{
		return false;
	}

	pBestPlot = NULL;
	pBestSpyPlot = NULL;

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (pLoopPlot->isCoastalLand())
		{
			if (pLoopPlot->getOwnerINLINE() == eBestPlayer)
			{
				iValue = pLoopPlot->area()->getCitiesPerPlayer(eBestPlayer);

				if (iValue > 0)
				{
					if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_ATTACK_SPY, getGroup(), 4) == 0)
					{
						if (generatePath(pLoopPlot, 0, true, &iPathTurns))
						{
							iValue *= 1000;

							iValue /= (iPathTurns + 1);

							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								pBestPlot = getPathEndTurnPlot();
								pBestSpyPlot = pLoopPlot;
							}
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestSpyPlot != NULL))
	{
		if ((pBestPlot == pBestSpyPlot) || (stepDistance(pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), pBestSpyPlot->getX_INLINE(), pBestSpyPlot->getY_INLINE()) == 1))
		{
			if (atPlot(pBestSpyPlot))
			{
				unloadAll(); // XXX is this dangerous (not pushing a mission...) XXX air units?
				return true;
			}
			else
			{
				getGroup()->pushMission(MISSION_MOVE_TO, pBestSpyPlot->getX_INLINE(), pBestSpyPlot->getY_INLINE(), 0, false, false, MISSIONAI_ATTACK_SPY, pBestSpyPlot);
				return true;
			}
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_ATTACK_SPY, pBestSpyPlot);
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_carrierSeaTransport()
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pPlot;
	CvPlot* pLoopPlot;
	CvPlot* pLoopPlotAir;
	CvPlot* pBestPlot;
	CvPlot* pBestCarrierPlot;
	int iMaxAirRange;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iDX, iDY;
	int iI;

	// XXX maybe protect land troops?

	iMaxAirRange = 0;

	pPlot = plot();

	pUnitNode = pPlot->headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pPlot->nextUnitNode(pUnitNode);

		if (pLoopUnit->getTransportUnit() == this)
		{
			iMaxAirRange = max(iMaxAirRange, pLoopUnit->airRange());
		}
	}

	if (iMaxAirRange == 0)
	{
		return false;
	}

	iBestValue = 0;
	pBestPlot = NULL;
	pBestCarrierPlot = NULL;

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (AI_plotValid(pLoopPlot))
		{
			if (pLoopPlot->isAdjacentToLand())
			{
				iValue = 0;

				for (iDX = -(iMaxAirRange); iDX <= iMaxAirRange; iDX++)
				{
					for (iDY = -(iMaxAirRange); iDY <= iMaxAirRange; iDY++)
					{
						pLoopPlotAir = plotXY(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), iDX, iDY);

						if (pLoopPlotAir != NULL)
						{
							if (plotDistance(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), pLoopPlotAir->getX_INLINE(), pLoopPlotAir->getY_INLINE()) <= iMaxAirRange)
							{
								if (!(pLoopPlotAir->isBarbarian()))
								{
									if (potentialWarAction(pLoopPlotAir))
									{
										if (pLoopPlotAir->isCity())
										{
											iValue++;
										}

										if (pLoopPlotAir->getImprovementType() != NO_IMPROVEMENT)
										{
											iValue += 2;
										}
									}
								}
							}
						}
					}
				}

				if (iValue > 0)
				{
					if (!(pLoopPlot->isVisibleEnemyUnit(getOwnerINLINE())))
					{
						if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_CARRIER, getGroup(), 4) == 0)
						{
							if (generatePath(pLoopPlot, 0, true, &iPathTurns))
							{
								iValue *= 1000;

								iValue += (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_ASSAULT, getGroup(), 2) * 1000);

								iValue /= (iPathTurns + 1);

								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									pBestPlot = getPathEndTurnPlot();
									pBestCarrierPlot = pLoopPlot;
								}
							}
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestCarrierPlot != NULL))
	{
		if (atPlot(pBestCarrierPlot))
		{
			getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_CARRIER, pBestCarrierPlot);
			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_CARRIER, pBestCarrierPlot);
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_connectPlot(CvPlot* pPlot, int iRange)
{
	PROFILE_FUNC();

	CvCity* pLoopCity;
	int iLoop;

	FAssert(canBuildRoute());

	if ((pPlot->getOwnerINLINE() == getOwnerINLINE()) && (!(pPlot->isVisibleEnemyUnit(getOwnerINLINE()))))
	{
		if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pPlot, MISSIONAI_BUILD, getGroup(), iRange) == 0)
		{
			if (generatePath(pPlot, MOVE_SAFE_TERRITORY, true))
			{
				for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
				{
					if (!(pPlot->isConnectedTo(pLoopCity)))
					{
						FAssertMsg(pPlot->getPlotCity() != pLoopCity, "pPlot->getPlotCity() is not expected to be equal with pLoopCity");

						if (plot()->getPlotGroup(getOwnerINLINE()) == pLoopCity->plot()->getPlotGroup(getOwnerINLINE()))
						{
							getGroup()->pushMission(MISSION_ROUTE_TO, pPlot->getX_INLINE(), pPlot->getY_INLINE(), MOVE_SAFE_TERRITORY, false, false, MISSIONAI_BUILD, pPlot);

							return true;
						}
					}
				}

				for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
				{
					if (!(pPlot->isConnectedTo(pLoopCity)))
					{
						FAssertMsg(pPlot->getPlotCity() != pLoopCity, "pPlot->getPlotCity() is not expected to be equal with pLoopCity");

						if (!(pLoopCity->plot()->isVisibleEnemyUnit(getOwnerINLINE())))
						{
							if (generatePath(pLoopCity->plot(), MOVE_SAFE_TERRITORY, true))
							{
								if (atPlot(pPlot)) // need to test before moving...
								{
									getGroup()->pushMission(MISSION_ROUTE_TO, pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE(), MOVE_SAFE_TERRITORY, false, false, MISSIONAI_BUILD, pPlot);
								}
								else
								{
									getGroup()->pushMission(MISSION_ROUTE_TO, pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE(), MOVE_SAFE_TERRITORY, false, false, MISSIONAI_BUILD, pPlot);
									getGroup()->pushMission(MISSION_ROUTE_TO, pPlot->getX_INLINE(), pPlot->getY_INLINE(), MOVE_SAFE_TERRITORY, (getGroup()->getLengthMissionQueue() > 0), false, MISSIONAI_BUILD, pPlot);
								}

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


// Returns true if a mission was pushed...
bool CvUnitAI::AI_improveCity(CvCity* pCity)
{
	PROFILE_FUNC();

	CvPlot* pBestPlot;
	BuildTypes eBestBuild;
	MissionTypes eMission;

	if (AI_bestCityBuild(pCity, &pBestPlot, &eBestBuild))
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
		}

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
			iValue = pLoopCity->AI_totalBestBuildValue(area());

			if (iValue > 0)
			{
				if (AI_bestCityBuild(pLoopCity, &pPlot, &eBuild))
				{
					FAssert(pPlot != NULL);
					FAssert(eBuild != NO_BUILD);

					iValue *= (max(0, (pLoopCity->getPopulation() - pLoopCity->countNumImprovedPlots())) + 1);

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
						FAssert(!atPlot(pBestPlot));
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


// Returns true if a mission was pushed...
bool CvUnitAI::AI_nextCityToImproveAirlift()
{
	PROFILE_FUNC();

	CvCity* pCity;
	CvCity* pLoopCity;
	CvPlot* pBestPlot;
	int iValue;
	int iBestValue;
	int iLoop;

	if (getGroup()->getNumUnits() > 1)
	{
		return false;
	}

	pCity = plot()->getPlotCity();

	if (pCity == NULL)
	{
		return false;
	}

	if (pCity->getMaxAirlift() == 0)
	{
		return false;
	}

	iBestValue = 0;
	pBestPlot = NULL;

	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		if (pLoopCity != pCity)
		{
			if (canAirliftAt(pCity->plot(), pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE()))
			{
				iValue = pLoopCity->AI_totalBestBuildValue(pLoopCity->area());

				if (iValue > iBestValue)
				{
					iBestValue = iValue;
					pBestPlot = pLoopCity->plot();
					FAssert(pLoopCity != pCity);
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		getGroup()->pushMission(MISSION_AIRLIFT, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
		return true;
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_irrigateTerritory()
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	ImprovementTypes eImprovement;
	BuildTypes eBuild;
	BuildTypes eBestBuild;
	BuildTypes eBestTempBuild;
	BonusTypes eNonObsoleteBonus;
	bool bValid;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iBestTempBuildValue;
	int iI, iJ;

	iBestValue = 0;
	eBestBuild = NO_BUILD;
	pBestPlot = NULL;

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (AI_plotValid(pLoopPlot))
		{
			if (pLoopPlot->getOwnerINLINE() == getOwnerINLINE()) // XXX team???
			{
				if (pLoopPlot->getWorkingCity() == NULL)
				{
					eImprovement = pLoopPlot->getImprovementType();

					if ((eImprovement == NO_IMPROVEMENT) || !(GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_SAFE_AUTOMATION) && !(eImprovement == (GC.getDefineINT("RUINS_IMPROVEMENT")))))
					{
						if ((eImprovement == NO_IMPROVEMENT) || !(GC.getImprovementInfo(eImprovement).isCarriesIrrigation()))
						{
							eNonObsoleteBonus = pLoopPlot->getNonObsoleteBonusType(getTeam());

							if ((eImprovement == NO_IMPROVEMENT) || (eNonObsoleteBonus == NO_BONUS) || !(GC.getImprovementInfo(eImprovement).isImprovementBonusTrade(eNonObsoleteBonus)))
							{
								if (pLoopPlot->isIrrigationAvailable(true))
								{
									iBestTempBuildValue = MAX_INT;
									eBestTempBuild = NO_BUILD;

									for (iJ = 0; iJ < GC.getNumBuildInfos(); iJ++)
									{
										eBuild = ((BuildTypes)iJ);
                                        FAssertMsg(eBuild < GC.getNumBuildInfos(), "Invalid Build");

										if (GC.getBuildInfo(eBuild).getImprovement() != NO_IMPROVEMENT)
										{
											if (GC.getImprovementInfo((ImprovementTypes)(GC.getBuildInfo(eBuild).getImprovement())).isCarriesIrrigation())
											{
												if (canBuild(pLoopPlot, eBuild))
												{
													iValue = 10000;

													iValue /= (GC.getBuildInfo(eBuild).getTime() + 1);

													// XXX feature production???

													if (iValue < iBestTempBuildValue)
													{
														iBestTempBuildValue = iValue;
														eBestTempBuild = eBuild;
													}
												}
											}
										}
									}

									if (eBestTempBuild != NO_BUILD)
									{
										bValid = true;

										if (GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_LEAVE_FORESTS))
										{
											if (pLoopPlot->getFeatureType() != NO_FEATURE)
											{
												if (GC.getBuildInfo(eBestTempBuild).isFeatureRemove(pLoopPlot->getFeatureType()))
												{
													if (GC.getFeatureInfo(pLoopPlot->getFeatureType()).getYieldChange(YIELD_PRODUCTION) > 0)
													{
														bValid = false;
													}
												}
											}
										}

										if (bValid)
										{
											if (!(pLoopPlot->isVisibleEnemyUnit(getOwnerINLINE())))
											{
												if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_BUILD, getGroup(), 1) == 0)
												{
													if (generatePath(pLoopPlot, 0, true, &iPathTurns)) // XXX should this actually be at the top of the loop? (with saved paths and all...)
													{
														iValue = 10000;

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


// Returns true if a mission was pushed...
bool CvUnitAI::AI_improveBonus(int iMinValue)
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	ImprovementTypes eImprovement;
	BuildTypes eBuild;
	BuildTypes eBestBuild;
	BuildTypes eBestTempBuild;
	BonusTypes eNonObsoleteBonus;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iBestTempBuildValue;
	int iI, iJ;
	bool bBestBuildIsRoute;

	bool bCanRoute;
	bool bIsConnected;

	iBestValue = 0;
	eBestBuild = NO_BUILD;
	pBestPlot = NULL;

	bCanRoute = canBuildRoute();

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (AI_plotValid(pLoopPlot))
		{
			if (pLoopPlot->getOwnerINLINE() == getOwnerINLINE()) // XXX team???
			{
				
				
				eNonObsoleteBonus = pLoopPlot->getNonObsoleteBonusType(getTeam());

				if (eNonObsoleteBonus != NO_BONUS)
				{
				    bIsConnected = pLoopPlot->isConnectedToCapital(getOwnerINLINE());
                    if ((pLoopPlot->getWorkingCity() != NULL) || (bIsConnected || bCanRoute))
                    {
                        eImprovement = pLoopPlot->getImprovementType();

                        bool bDoImprove = false;
                        
                        if (eImprovement == NO_IMPROVEMENT)
                        {
                            bDoImprove = true;
                        }
                        else if (eImprovement == (ImprovementTypes)(GC.getDefineINT("RUINS_IMPROVEMENT")))
                        {
                            bDoImprove = true;
                        }
                        else if (!GC.getImprovementInfo(eImprovement).isImprovementBonusTrade(eNonObsoleteBonus))
                        {
                            if (GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_SAFE_AUTOMATION))
                            {
                                if (GET_PLAYER(getOwnerINLINE()).getNumTradeableBonuses(eNonObsoleteBonus) == 0)
                                {
                                    if (!pLoopPlot->isIrrigated())
                                    {
                                        if (((ImprovementTypes)GC.getImprovementInfo(eImprovement).getImprovementPillage()) == NO_IMPROVEMENT)
                                        {
                                            bDoImprove = true;
                                        }
                                    }
                                }
                            }
                        }

                        iBestTempBuildValue = MAX_INT;
                        eBestTempBuild = NO_BUILD;

                        if (bDoImprove)
                        {
                            for (iJ = 0; iJ < GC.getNumBuildInfos(); iJ++)
                            {
                                eBuild = ((BuildTypes)iJ);

                                if (GC.getBuildInfo(eBuild).getImprovement() != NO_IMPROVEMENT)
                                {
                                    if (GC.getImprovementInfo((ImprovementTypes) GC.getBuildInfo(eBuild).getImprovement()).isImprovementBonusTrade(eNonObsoleteBonus))
                                    {
                                        if (canBuild(pLoopPlot, eBuild))
                                        {
                                            iValue = 10000;

                                            iValue /= (GC.getBuildInfo(eBuild).getTime() + 1);

                                            // XXX feature production???

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
                        if (eBestTempBuild == NO_BUILD)
                        {
                        	bDoImprove = false;
                        }

                        if ((eBestTempBuild != NO_BUILD) || (bCanRoute && !bIsConnected))
                        {
                        	iValue = GET_PLAYER(getOwnerINLINE()).AI_bonusVal(eNonObsoleteBonus);
                                        
							if (bDoImprove)
							{
								eImprovement = (ImprovementTypes)GC.getBuildInfo(eBestTempBuild).getImprovement();
								FAssert(eImprovement != NO_IMPROVEMENT);
								//iValue += (GC.getImprovementInfo((ImprovementTypes) GC.getBuildInfo(eBestTempBuild).getImprovement()))
								iValue += 5 * pLoopPlot->calculateImprovementYieldChange(eImprovement, YIELD_FOOD, getOwner(), false);
								iValue += 5 * pLoopPlot->calculateNatureYield(YIELD_FOOD, getTeam(), (pLoopPlot->getFeatureType() == NO_FEATURE) ? true : GC.getBuildInfo(eBestTempBuild).isFeatureRemove(pLoopPlot->getFeatureType()));
							}

							iValue += max(0, 100 * GC.getBonusInfo(eNonObsoleteBonus).getAIObjective());

							if (GET_PLAYER(getOwnerINLINE()).getNumTradeableBonuses(eNonObsoleteBonus) == 0)
							{
								iValue *= 2;
							}
							
							int iMaxWorkers = 1;
							if ((eBestTempBuild != NO_BUILD) && (!GC.getBuildInfo(eBestTempBuild).isKill()))
							{
								//allow teaming.
								iMaxWorkers = min(3, (iValue / 10));
							}
							
							if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_BUILD, getGroup()) < iMaxWorkers)
                        	{
								if (bDoImprove)
								{
									if (generatePath(pLoopPlot, 0, true, &iPathTurns)) // XXX should this actually be at the top of the loop? (with saved paths and all...)
									{
										if (iValue > iMinValue)
										{
											iValue *= 1000;

											iValue /= (iPathTurns + 1);

											if (pLoopPlot->isCityRadius())
											{
												iValue *= 2;
											}

											if (iValue > iBestValue)
											{
												iBestValue = iValue;
												eBestBuild = eBestTempBuild;
												pBestPlot = pLoopPlot;
												bBestBuildIsRoute = false;
											}
										}
									}
								}
								else
								{
									FAssert(bCanRoute && !bIsConnected);
									eImprovement = pLoopPlot->getImprovementType();
									if ((eImprovement != NO_IMPROVEMENT) && (GC.getImprovementInfo(eImprovement).isImprovementBonusTrade(eNonObsoleteBonus)))
									{
										if (generatePath(pLoopPlot, MOVE_SAFE_TERRITORY, true, &iPathTurns))
										{
											iValue *= 1000;
											iValue /= (iPathTurns + 1);
											
											if (iValue > iBestValue)
											{
												iBestValue = iValue;
												eBestBuild = NO_BUILD;
												pBestPlot = pLoopPlot;
												bBestBuildIsRoute = true;
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
		if (eBestBuild != NO_BUILD)
		{
			FAssertMsg(!bBestBuildIsRoute, "BestBuild should not be a route");
			FAssertMsg(eBestBuild < GC.getNumBuildInfos(), "BestBuild is assigned a corrupt value");

			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_BUILD, pBestPlot);
			getGroup()->pushMission(MISSION_BUILD, eBestBuild, -1, 0, (getGroup()->getLengthMissionQueue() > 0), false, MISSIONAI_BUILD, pBestPlot);

			return true;
		}
		else if (bBestBuildIsRoute)
		{
			if (AI_connectPlot(pBestPlot))
			{
				return true;
			}
			/*else
			{
				// the plot may be connected, but not connected to capital, if capital is not on same area, or if civ has no capital (like barbarians)
				FAssertMsg(false, "Expected that a route could be built to eBestPlot");
			}*/
		}
		else
		{
			FAssert(false);
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_connectBonus(bool bTestTrade)
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	BonusTypes eNonObsoleteBonus;
	int iI;

	// XXX how do we make sure that we can build roads???

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (AI_plotValid(pLoopPlot))
		{
			if (pLoopPlot->getOwnerINLINE() == getOwnerINLINE()) // XXX team???
			{
				eNonObsoleteBonus = pLoopPlot->getNonObsoleteBonusType(getTeam());

				if (eNonObsoleteBonus != NO_BONUS)
				{
					if (!(pLoopPlot->isConnectedToCapital()))
					{
						if (!bTestTrade || ((pLoopPlot->getImprovementType() != NO_IMPROVEMENT) && (GC.getImprovementInfo(pLoopPlot->getImprovementType()).isImprovementBonusTrade(eNonObsoleteBonus))))
						{
							if (AI_connectPlot(pLoopPlot))
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


// Returns true if a mission was pushed...
bool CvUnitAI::AI_connectCity()
{
	PROFILE_FUNC();

	CvCity* pLoopCity;
	int iLoop;

	// XXX how do we make sure that we can build roads???

    pLoopCity = plot()->getWorkingCity();
    if (pLoopCity != NULL)
    {
        if (AI_plotValid(pLoopCity->plot()))
        {
            if (!(pLoopCity->isConnectedToCapital()))
            {
                if (AI_connectPlot(pLoopCity->plot(), 1))
                {
                    return true;
                }
            }
        }
    }

	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		if (AI_plotValid(pLoopCity->plot()))
		{
			if (!(pLoopCity->isConnectedToCapital()))
			{
				if (AI_connectPlot(pLoopCity->plot(), 1))
				{
					return true;
				}
			}
		}
	}

	return false;
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
				if (!(pLoopCity->plot()->isVisibleEnemyUnit(getOwnerINLINE())))
				{
					if (generatePath(pLoopCity->plot(), MOVE_SAFE_TERRITORY, true))
					{
						if (!(pRouteToCity->plot()->isVisibleEnemyUnit(getOwnerINLINE())))
						{
							if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pRouteToCity->plot(), MISSIONAI_BUILD, getGroup()) == 0)
							{
								if (generatePath(pRouteToCity->plot(), MOVE_SAFE_TERRITORY, true))
								{
									getGroup()->pushMission(MISSION_ROUTE_TO, pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE(), MOVE_SAFE_TERRITORY, false, false, MISSIONAI_BUILD, pRouteToCity->plot());
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
							if (!(pLoopPlot->isVisibleEnemyUnit(getOwnerINLINE())))
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
		bool bCanAirliftUnit = (getDomainType() == DOMAIN_LAND);
		bool bShouldSkipToUpgrade = (getDomainType() != DOMAIN_AIR);

		// if we at the upgrade city, stop, wait to get upgraded
		if (pUpgradeCity->plot() == pPlot)
		{
			if (!bShouldSkipToUpgrade)
			{
				return false;
			}
			
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
		bool bCanPathToUpgradeCity = generatePath(pUpgradeCity->plot(), 0, true, &iUpgradeCityPathTurns);
		CvPlot* pThisTurnPlot = bCanPathToUpgradeCity ? getPathEndTurnPlot() : NULL;
		
		// if we close to upgrade city, head there 
		if (bCanPathToUpgradeCity && (pClosestCity == pUpgradeCity || iUpgradeCityPathTurns < 4))
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pThisTurnPlot->getX_INLINE(), pThisTurnPlot->getY_INLINE());
			return true;
		}
		
		// check for better airlift choice
		if (bCanAirliftUnit && pClosestCity != NULL && pClosestCity->getMaxAirlift() > 0)
		{
			// if we at the closest city, then do the airlift, or wait
			if (bAtClosestCity)
			{
				// can we do the airlift this turn?
				if (canAirliftAt(pClosestCity->plot(), pUpgradeCity->getX_INLINE(), pUpgradeCity->getY_INLINE()))
				{
					getGroup()->pushMission(MISSION_AIRLIFT, pUpgradeCity->getX_INLINE(), pUpgradeCity->getY_INLINE());
					return true;
				}
				// wait to do it next turn
				else
				{
					getGroup()->pushMission(MISSION_SKIP);
					return true;
				}
			}
			
			int iClosestCityPathTurns;
			bool bCanPathToClosestCity = generatePath(pClosestCity->plot(), 0, true, &iClosestCityPathTurns);
			
			// is the closest city closer pathing? If so, move toward closest city
			if (bCanPathToClosestCity && iClosestCityPathTurns < iUpgradeCityPathTurns)
			{
				pThisTurnPlot = getPathEndTurnPlot();
				getGroup()->pushMission(MISSION_MOVE_TO, pThisTurnPlot->getX_INLINE(), pThisTurnPlot->getY_INLINE());
				return true;
			}
		}
		
		// did not have better airlift choice, go ahead and path to the upgrade city
		if (bCanPathToUpgradeCity)
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pThisTurnPlot->getX_INLINE(), pThisTurnPlot->getY_INLINE());
			return true;
		}
	}

	return false;
}

// Returns true if a mission was pushed...
bool CvUnitAI::AI_retreatToCity(bool bPrimary, bool bAirlift, int iMaxPath)
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
	int iBestDanger = iCurrentDanger;

	pCity = plot()->getPlotCity();


	if (0 == iCurrentDanger)
	{
		if (pCity != NULL)
		{
			if (pCity->getOwnerINLINE() == getOwnerINLINE())
			{
				if (!bPrimary || GET_PLAYER(getOwnerINLINE()).AI_isPrimaryArea(pCity->area()))
				{
					if (!bAirlift || (pCity->getMaxAirlift() > 0))
					{
						if (!(pCity->plot()->isVisibleEnemyUnit(getOwnerINLINE())))
						{
#ifdef DEBUG_AI_UPDATE_GROUPS
							// debugging
							if (true)
							{
								CvSelectionGroup* pCurrentGroup = getGroup();
								CvWString szUnitName = GC.getUnitInfo(m_eUnitType).getDescription();
								DEBUGLOG("AI_retreatToCity:%S(%d) already in city, will skip. Group(%d) [size %d%s]\n", szUnitName.GetCString(), shortenID(m_iID), 
									shortenID(m_iGroupID), pCurrentGroup->getNumUnits(), pCurrentGroup->AI_isForceSeparate() ? ",WILL_SEPARATE" : "");
							}
#endif
							getGroup()->pushMission(MISSION_SKIP);
							return true;
						}
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
					if (!bAirlift || (pLoopCity->getMaxAirlift() > 0))
					{
						if (!(pLoopCity->plot()->isVisibleEnemyUnit(getOwnerINLINE())))
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
						if (!bAirlift || (pCity->getMaxAirlift() > 0))
						{
							if (!(pCity->plot()->isVisibleEnemyUnit(getOwnerINLINE())))
							{
								getGroup()->pushMission(MISSION_SKIP);
								return true;
							}
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

	pCity = plot()->getPlotCity();

	if (pCity != NULL)
	{
		if (pCity->getOwnerINLINE() == getOwnerINLINE())
		{
			if (pCity->plot()->plotCount(PUF_isUnitAIType, eUnitAI, -1, getOwnerINLINE()) > 0)
			{
				getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_PICKUP, pCity->plot());
				return true;
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
			if (GET_PLAYER(getOwnerINLINE()).AI_isPrimaryArea(pLoopCity->area()))
			{
				iValue = pLoopCity->plot()->plotCount(PUF_isUnitAIType, eUnitAI, -1, getOwnerINLINE());

				if (iValue > 0)
				{
					iValue *= 10;

					iValue += pLoopCity->getPopulation();

					if (!(pLoopCity->plot()->isVisibleEnemyUnit(getOwnerINLINE())))
					{
						if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopCity->plot(), MISSIONAI_PICKUP, getGroup()) == 0)
						{
							if (!atPlot(pLoopCity->plot()) && generatePath(pLoopCity->plot(), 0, true, &iPathTurns))
							{
								iValue *= 1000;

								iValue /= (iPathTurns + 1);

								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									pBestPlot = getPathEndTurnPlot();
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


// Returns true if a mission was pushed...
bool CvUnitAI::AI_airOffensiveCity()
{
	PROFILE_FUNC();

	CvCity* pNearestEnemyCity;
	CvCity* pLoopCity;
	CvPlot* pBestPlot;
	int iValue;
	int iBestValue;
	int iLoop;
	int iI;

	FAssert(canAirAttack());

	iBestValue = 0;
	pBestPlot = NULL;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() == getTeam())
			{
				for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
				{
					if (pLoopCity->AI_isDefended())
					{
						if (atPlot(pLoopCity->plot()) || canMoveInto(pLoopCity->plot()))
						{
							// XXX is in danger?

							iValue = (pLoopCity->getPopulation() + 10);

							if (pLoopCity->getOwnerINLINE() == getOwnerINLINE())
							{
								iValue *= 3;
								iValue /= 2;
							}

							if (pLoopCity->AI_isDanger())
							{
								iValue *= 2;
							}

							iValue *= 1000;

							pNearestEnemyCity = GC.getMapINLINE().findCity(pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE(), NO_PLAYER, NO_TEAM, false, false, getTeam());

							if (pNearestEnemyCity != NULL)
							{
								iValue /= (plotDistance(pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE(), pNearestEnemyCity->getX_INLINE(), pNearestEnemyCity->getY_INLINE()) + 1);
							}

							iValue /= ((pLoopCity->plot()->plotCount(PUF_canAirAttack, -1, -1, NO_PLAYER, getTeam()) + ((atPlot(pLoopCity->plot()) && canAirAttack()) ? -1 : 0)) + 1);

							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								pBestPlot = pLoopCity->plot();
							}
						}
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		if (!atPlot(pBestPlot))
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_airDefensiveCity()
{
	PROFILE_FUNC();

	CvCity* pCity;
	CvCity* pLoopCity;
	CvPlot* pBestPlot;
	int iValue;
	int iBestValue;
	int iLoop;

	FAssert(getDomainType() == DOMAIN_AIR);
	FAssert(canAirDefend());

	if (canAirDefend())
	{
		pCity = plot()->getPlotCity();

		if (pCity != NULL)
		{
			if (pCity->getOwnerINLINE() == getOwnerINLINE())
			{
				if (!(pCity->AI_isAirDefended(-1)))
				{
					getGroup()->pushMission(MISSION_AIRPATROL);
					return true;
				}
			}
		}
	}

	iBestValue = 0;
	pBestPlot = NULL;

	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		if (canAirDefend(pLoopCity->plot()))
		{
			if (!(pLoopCity->AI_isAirDefended()))
			{
				if (!atPlot(pLoopCity->plot()) && canMoveInto(pLoopCity->plot()))
				{
					// XXX is in danger?

					iValue = pLoopCity->getPopulation();

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
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
		return true;
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_airCarrier()
{
	PROFILE_FUNC();

	CvUnit* pLoopUnit;
	CvUnit* pBestUnit;
	int iValue;
	int iBestValue;
	int iLoop;

	if (getCargo() > 0)
	{
		return false;
	}

	if (isCargo())
	{
		if (canAirDefend())
		{
			getGroup()->pushMission(MISSION_AIRPATROL);
			return true;
		}
		else
		{
			getGroup()->pushMission(MISSION_SKIP);
			return true;
		}
	}

	iBestValue = 0;
	pBestUnit = NULL;

	for(pLoopUnit = GET_PLAYER(getOwnerINLINE()).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER(getOwnerINLINE()).nextUnit(&iLoop))
	{
		if (canLoadUnit(pLoopUnit, pLoopUnit->plot()))
		{
			iValue = 10;

			if (!(pLoopUnit->plot()->isCity()))
			{
				iValue += 20;
			}

			if (pLoopUnit->plot()->isOwned())
			{
				if (atWar(getTeam(), pLoopUnit->plot()->getTeam()))
				{
					iValue += 20;
				}
			}
			else
			{
				iValue += 10;
			}

			iValue /= (pLoopUnit->getCargo() + 1);

			if (iValue > iBestValue)
			{
				iBestValue = iValue;
				pBestUnit = pLoopUnit;
			}
		}
	}

	if (pBestUnit != NULL)
	{
		if (atPlot(pBestUnit->plot()))
		{
			setTransportUnit(pBestUnit); // XXX is this dangerous (not pushing a mission...) XXX air units?
			return true;
		}
		else
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestUnit->getX_INLINE(), pBestUnit->getY_INLINE());
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_airStrike()
{
	PROFILE_FUNC();

	CvUnit* pDefender;
	CvUnit* pInterceptor;
	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	int iSearchRange;
	int iDamage;
	int iPotentialAttackers;
	int iInterceptProb;
	int iValue;
	int iBestValue;
	int iDX, iDY;

	iSearchRange = airRange();

	iBestValue = 0;
	pBestPlot = NULL;

	for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (canAirStrike(pLoopPlot))
				{
					iPotentialAttackers = GET_PLAYER(getOwnerINLINE()).AI_adjacentPotentialAttackers(pLoopPlot);

					if ((iPotentialAttackers > 0) || pLoopPlot->isAdjacentTeam(getTeam()))
					{
						pDefender = pLoopPlot->getBestDefender(NO_PLAYER, getOwnerINLINE(), this, true);

						FAssert(pDefender != NULL);
						FAssert(pDefender->canDefend());

						// XXX factor in air defenses...

						iDamage = airCombatDamage(pDefender);

						iValue = max(0, (min((pDefender->getDamage() + iDamage), airCombatLimit()) - pDefender->getDamage()));

						iValue += ((((iDamage * collateralDamage()) / 100) * min((pLoopPlot->getNumVisibleEnemyDefenders(getOwnerINLINE()) - 1), collateralDamageMaxUnits())) / 2);

						iValue *= (3 + iPotentialAttackers);
						iValue /= 4;

						pInterceptor = bestInterceptor(pLoopPlot);

						if (pInterceptor != NULL)
						{
							iInterceptProb = pInterceptor->currInterceptionProbability();

							iInterceptProb *= max(0, (100 - evasionProbability()));
							iInterceptProb /= 100;

							iValue *= max(0, (100 - iInterceptProb));
							iValue /= 100;
						}

						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							pBestPlot = pLoopPlot;
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
bool CvUnitAI::AI_airBomb()
{
	PROFILE_FUNC();

	CvCity* pCity;
	CvUnit* pInterceptor;
	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	int iSearchRange;
	int iPotentialAttackers;
	int iInterceptProb;
	int iValue;
	int iBestValue;
	int iDX, iDY;

	iSearchRange = airRange();

	iBestValue = 0;
	pBestPlot = NULL;

	for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			pLoopPlot	= plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (canAirBombAt(plot(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE()))
				{
					iValue = 0;

					pCity = pLoopPlot->getPlotCity();

					if (pCity != NULL)
					{
						iPotentialAttackers = GET_PLAYER(getOwnerINLINE()).AI_adjacentPotentialAttackers(pLoopPlot);

						if (iPotentialAttackers > 1)
						{
							iValue += max(0, (min((pCity->getDefenseDamage() + airBombCurrRate()), GC.getMAX_CITY_DEFENSE_DAMAGE()) - pCity->getDefenseDamage()));

							iValue *= 5;

							if (pCity->AI_isDanger())
							{
								iValue *= 2;
							}

							if (pCity == pCity->area()->getTargetCity(getOwnerINLINE()))
							{
								iValue *= 3;
							}
						}
					}
					else
					{
						iValue += AI_pillageValue(pLoopPlot);

						iValue *= GC.getGameINLINE().getSorenRandNum(20, "AI Air Bomb");
					}

					pInterceptor = bestInterceptor(pLoopPlot);

					if (pInterceptor != NULL)
					{
						iInterceptProb = pInterceptor->currInterceptionProbability();

						iInterceptProb *= max(0, (100 - evasionProbability()));
						iInterceptProb /= 100;

						iValue *= max(0, (100 - iInterceptProb));
						iValue /= 100;
					}

					if (iValue > iBestValue)
					{
						iBestValue = iValue;
						pBestPlot = pLoopPlot;
						FAssert(!atPlot(pBestPlot));
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		getGroup()->pushMission(MISSION_AIRBOMB, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
		return true;
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_nuke()
{
	PROFILE_FUNC();

	CvCity* pLoopCity;
	CvCity* pBestCity;
	int iValue;
	int iBestValue;
	int iLoop;
	int iI;

	pBestCity = NULL;

	iBestValue = 0;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive() && !GET_PLAYER((PlayerTypes)iI).isBarbarian())
		{
			if (atWar(getTeam(), GET_PLAYER((PlayerTypes)iI).getTeam()))
			{
				if (GET_PLAYER(getOwnerINLINE()).AI_getAttitude((PlayerTypes)iI) == ATTITUDE_FURIOUS)
				{
					for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
					{
						if (canNukeAt(plot(), pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE()))
						{
							iValue = AI_nukeValue(pLoopCity);

							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								pBestCity = pLoopCity;
								FAssert(pBestCity->getTeam() != getTeam());
							}
						}
					}
				}
			}
		}
	}

	if (pBestCity != NULL)
	{
		getGroup()->pushMission(MISSION_NUKE, pBestCity->getX_INLINE(), pBestCity->getY_INLINE());
		return true;
	}

	return false;
}


bool CvUnitAI::AI_trade(int iValueThreshold)
{
	CvCity* pLoopCity;
	CvPlot* pBestPlot;
	CvPlot* pBestTradePlot;

	int iPathTurns;
	int iValue;
	int iBestValue;
	int iLoop;
	int iI;


	iBestValue = 0;
	pBestPlot = NULL;
	pBestTradePlot = NULL;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
			{
				if (AI_plotValid(pLoopCity->plot()))
				{
                    if (getTeam() != pLoopCity->getTeam())
				    {
                        iValue = getTradeGold(pLoopCity->plot());

                        if ((iValue >= iValueThreshold) && canTrade(pLoopCity->plot(), true))
                        {
                            if (!(pLoopCity->plot()->isVisibleEnemyUnit(getOwnerINLINE())))
                            {
                                if (generatePath(pLoopCity->plot(), 0, true, &iPathTurns))
                                {
                                    FAssert(iPathTurns > 0);

                                    iValue /= (4 + iPathTurns);

                                    if (iValue > iBestValue)
                                    {
                                        iBestValue = iValue;
                                        pBestPlot = getPathEndTurnPlot();
                                        pBestTradePlot = pLoopCity->plot();
                                    }
                                }

                            }
                        }
				    }
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestTradePlot != NULL))
	{
		if (atPlot(pBestTradePlot))
		{
			getGroup()->pushMission(MISSION_TRADE);
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
					iValue /= (4 + iPathTurns);

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
			return false;
		}
		else
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
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
		return isPotentialEnemy(getTeam(), eTeam);
	}
	else
	{
		return atWar(getTeam(), eTeam);
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

			if (pPlot->defenseModifier(getTeam(), false) >= 50 && pPlot->isRoute() && pPlot->getTeam() == getTeam())
			{
				return true;
			}
		}
	}

	return false;
}


int CvUnitAI::AI_pillageValue(CvPlot* pPlot, int iBonusValueThreshold)
{
	CvPlot* pAdjacentPlot;
	ImprovementTypes eImprovement;
	BonusTypes eNonObsoleteBonus;
	int iValue;
	int iTempValue;
	int iBonusValue;
	int iI;

	FAssert(canPillage(pPlot) || canAirBombAt(plot(), pPlot->getX_INLINE(), pPlot->getY_INLINE()) || (getCargo() > 0));

	if (!(pPlot->isOwned()))
	{
		return 0;
	}
	
	iBonusValue = 0;
	eNonObsoleteBonus = pPlot->getNonObsoleteBonusType(pPlot->getTeam());
	if (eNonObsoleteBonus != NO_BONUS)
	{
		iBonusValue = (GET_PLAYER(pPlot->getOwnerINLINE()).AI_bonusVal(eNonObsoleteBonus));
	}
	
	if (iBonusValueThreshold > 0)
	{
		if (eNonObsoleteBonus == NO_BONUS)
		{
			return 0;
		}
		else if (iBonusValue < iBonusValueThreshold)
		{
			return 0;
		}
	}

	iValue = 0;

	if (getDomainType() != DOMAIN_AIR)
	{
		if (pPlot->isRoute())
		{
			iValue++;
			if (eNonObsoleteBonus != NO_BONUS)
			{
				iValue += iBonusValue * 4;
			}

			for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
			{
				pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

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
				}
			}
		}
	}

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
			iValue += (pPlot->calculateImprovementYieldChange(eImprovement, YIELD_PRODUCTION, pPlot->getOwnerINLINE()) * 4);
			iValue += (pPlot->calculateImprovementYieldChange(eImprovement, YIELD_COMMERCE, pPlot->getOwnerINLINE()) * 3);
		}

		if (getDomainType() != DOMAIN_AIR)
		{
			iValue += GC.getImprovementInfo(eImprovement).getPillageGold();
		}

		if (eNonObsoleteBonus != NO_BONUS)
		{
			if (GC.getImprovementInfo(eImprovement).isImprovementBonusTrade(eNonObsoleteBonus))
			{
				iTempValue = iBonusValue * 4;

				if (pPlot->isConnectedToCapital() && (pPlot->getPlotGroupConnectedBonus(pPlot->getOwnerINLINE(), eNonObsoleteBonus) == 1))
				{
					iTempValue *= 2;
				}

				iValue += iTempValue;
			}
		}
	}

	return iValue;
}


int CvUnitAI::AI_nukeValue(CvCity* pCity)
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	int iValue;
	int iDX, iDY;

	FAssertMsg(pCity != NULL, "City is not assigned a valid value");

	iValue = 1;

	iValue += GC.getGameINLINE().getSorenRandNum((pCity->getPopulation() + 1), "AI Nuke City Value");

	iValue += ((pCity->getPopulation() * (100 + pCity->calculateCulturePercent(pCity->getOwnerINLINE()))) / 100);

	iValue += -(GET_PLAYER(getOwnerINLINE()).AI_getAttitudeVal(pCity->getOwnerINLINE()) / 3);

	for (iDX = -(nukeRange()); iDX <= nukeRange(); iDX++)
	{
		for (iDY = -(nukeRange()); iDY <= nukeRange(); iDY++)
		{
			pLoopPlot	= plotXY(pCity->getX_INLINE(), pCity->getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (pLoopPlot->getImprovementType() != NO_IMPROVEMENT)
				{
					iValue++;
				}

				if (pLoopPlot->getBonusType() != NO_BONUS)
				{
					iValue++;
				}
			}
		}
	}

	if (!(pCity->isEverOwned(getOwnerINLINE())))
	{
		iValue *= 3;
		iValue /= 2;
	}

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
		if (pPlot->isWater())
		{
			return true;
		}
		else if (pPlot->isFriendlyCity(getTeam()) && pPlot->isCoastalLand())
		{
			return true;
		}
		break;

	case DOMAIN_AIR:
		FAssert(false);
		break;

	case DOMAIN_LAND:
		if (pPlot->getArea() == getArea())
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
			iFinalOddsThreshold += max(0, (pCity->getDefenseDamage() - pCity->getLastDefenseDamage() - (GC.getDefineINT("CITY_DEFENSE_DAMAGE_HEAL_RATE") * 2)));
		}
	}

	if (pPlot->getNumVisiblePotentialEnemyDefenders(getOwnerINLINE()) == 1)
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

	iFinalOddsThreshold *= 6;
	iFinalOddsThreshold /= (3 + GET_PLAYER(getOwnerINLINE()).AI_adjacentPotentialAttackers(pPlot, true) + ((stepDistance(getX_INLINE(), getY_INLINE(), pPlot->getX_INLINE(), pPlot->getY_INLINE()) > 1) ? 1 : 0) + ((AI_isCityAIType()) ? 2 : 0));

	return range(iFinalOddsThreshold, 0, 100);
}


int CvUnitAI::AI_stackOfDoomExtra()
{
	return ((AI_getBirthmark() % (1 + GET_PLAYER(getOwnerINLINE()).getCurrentEra() * 2)) + 4);
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
					if (pLoopPlot->isCity() || (pLoopPlot->isCity(true) && pLoopPlot->isVisibleEnemyUnit(getOwnerINLINE())))
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
				if (AI_plotValid(pLoopPlot) && (!atWar(getTeam(),pLoopPlot->getTeam())))
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

//bolsters the culture of the weakest city.
//returns true if a mission is pushed.
bool CvUnitAI::AI_artistCultureVictoryMove()
{
    bool bGreatWork = false;
    bool bJoin = true;

    if (!GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_CULTURE1))
    {
        return false;        
    }
    
    if (GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_CULTURE3))
    {
        //Great Work
        bGreatWork = true;
    }
    
	int iCultureCitiesNeeded = GC.getGameINLINE().culturalVictoryNumCultureCities();
	FAssertMsg(iCultureCitiesNeeded > 0, "CultureVictory Strategy should not be true");

	CvCity* pLoopCity;
	CvPlot* pBestPlot;
	CvCity* pBestCity;
	SpecialistTypes eBestSpecialist;
	int iLoop, iValue, iBestValue;

	pBestPlot = NULL;
	eBestSpecialist = NO_SPECIALIST;

	pBestCity = NULL;
	
	iBestValue = 0;
	iLoop = 0;
	
	int iTargetCultureRank = iCultureCitiesNeeded;
	while (iTargetCultureRank > 0 && pBestCity == NULL)
	{
		for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
		{
			if (AI_plotValid(pLoopCity->plot()))
			{
				// instead of commerce rate rank should use the culture on tile...
				if (pLoopCity->findCommerceRateRank(COMMERCE_CULTURE) == iTargetCultureRank)
				{
					// if the city is a fledgling, probably building culture, try next higher city
					if (pLoopCity->getCultureLevel() < 2)
					{
						break;
					}
					
					// if we cannot path there, try the next higher culture city
					if (!generatePath(pLoopCity->plot(), 0, true))
					{
						break;
					}

					pBestCity = pLoopCity;
					pBestPlot = pLoopCity->plot();
					if (bGreatWork)
					{
						if (canGreatWork(pBestPlot))
						{
							break;
						}
					}

					for (int iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
					{
						if (canJoin(pBestPlot, ((SpecialistTypes)iI)))
						{
							iValue = pLoopCity->AI_specialistValue(((SpecialistTypes)iI), pLoopCity->AI_avoidGrowth(), false);

							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								eBestSpecialist = ((SpecialistTypes)iI);
							}
						}
					}

					if (eBestSpecialist == NO_SPECIALIST)
					{
						bJoin = false;
						if (canGreatWork(pBestPlot))
						{
							bGreatWork = true; 
							break;                        
						}
						bGreatWork = false;
					}
					break;
				}
			}
		}
	
		iTargetCultureRank--;
	}


	FAssertMsg(bGreatWork || bJoin, "This wasn't a Great Artist");
	
	if (pBestCity == NULL)
	{
	    //should try to airlift there...
	    return false;
	}


    if (atPlot(pBestPlot))
    {
        if (bGreatWork)
        {
            getGroup()->pushMission(MISSION_GREAT_WORK);
            return true;
        }
        if (bJoin)
        {
            getGroup()->pushMission(MISSION_JOIN, eBestSpecialist);
            return true;
        }
        FAssert(false);
        return false;		    
    }
    else
    {
        FAssert(!atPlot(pBestPlot));
        getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
        return true;
    }


	return false;
}    



void CvUnitAI::read(FDataStreamBase* pStream)
{
	CvUnit::read(pStream);

	uint uiFlag=0;
	pStream->Read(&uiFlag);	// flags for expansion

	pStream->Read(&m_iBirthmark);

	pStream->Read((int*)&m_eUnitAIType);
}


void CvUnitAI::write(FDataStreamBase* pStream)
{
	CvUnit::write(pStream);

	uint uiFlag=0;
	pStream->Write(uiFlag);		// flag for expansion

	pStream->Write(m_iBirthmark);

	pStream->Write(m_eUnitAIType);
}

// Private Functions...
