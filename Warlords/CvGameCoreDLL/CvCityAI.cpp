// cityAI.cpp

#include "CvGameCoreDLL.h"
#include "CvGlobals.h"
#include "CvGameCoreUtils.h"
#include "CvCityAI.h"
#include "CvGameAI.h"
#include "CvPlot.h"
#include "CvArea.h"
#include "CvPlayerAI.h"
#include "CvTeamAI.h"
#include "CyCity.h"
#include "CyArgsList.h"
#include "CvInfos.h"
#include "FProfiler.h"

#include "CvDLLPythonIFaceBase.h"
#include "CvDLLInterfaceIFaceBase.h"
#include "CvDLLFAStarIFaceBase.h"


#define BUILDINGFOCUS_FOOD					(0x00000001)
#define BUILDINGFOCUS_PRODUCTION		(0x00000002)
#define BUILDINGFOCUS_GOLD					(0x00000004)
#define BUILDINGFOCUS_RESEARCH			(0x00000008)
#define BUILDINGFOCUS_CULTURE				(0x00000010)
#define BUILDINGFOCUS_DEFENSE				(0x00000020)
#define BUILDINGFOCUS_HAPPY					(0x00000040)
#define BUILDINGFOCUS_HEALTHY				(0x00000080)
#define BUILDINGFOCUS_EXPERIENCE		(0x00000100)
#define BUILDINGFOCUS_MAINTENANCE		(0x00000200)
#define BUILDINGFOCUS_SPECIALIST		(0x00000400)


// Public Functions...

CvCityAI::CvCityAI()
{
	m_aiEmphasizeYieldCount = new int[NUM_YIELD_TYPES];
	m_aiEmphasizeCommerceCount = new int[NUM_COMMERCE_TYPES];

	m_pbEmphasize = NULL;

	AI_reset();
}


CvCityAI::~CvCityAI()
{
	AI_uninit();

	SAFE_DELETE_ARRAY(m_aiEmphasizeYieldCount);
	SAFE_DELETE_ARRAY(m_aiEmphasizeCommerceCount);
}


void CvCityAI::AI_init()
{
	AI_reset();

	//--------------------------------
	// Init other game data
	AI_assignWorkingPlots();

	AI_updateBestBuild();
}


void CvCityAI::AI_uninit()
{
	SAFE_DELETE_ARRAY(m_pbEmphasize);
}


// FUNCTION: AI_reset()
// Initializes data members that are serialized.
void CvCityAI::AI_reset()
{
	int iI;

	AI_uninit();

	m_iEmphasizeAvoidGrowthCount = 0;
	m_iEmphasizeGreatPeopleCount = 0;

	m_bAssignWorkDirty = false;
	m_bChooseProductionDirty = false;

	m_routeToCity.reset();

	for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		m_aiEmphasizeYieldCount[iI] = 0;
	}

	for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		m_aiEmphasizeCommerceCount[iI] = 0;
	}

	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		m_aiBestBuildValue[iI] = NO_BUILD;
	}

	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		m_aeBestBuild[iI] = NO_BUILD;
	}

	FAssertMsg(m_pbEmphasize == NULL, "m_pbEmphasize not NULL!!!");
	FAssertMsg(GC.getNumEmphasizeInfos() > 0,  "GC.getNumEmphasizeInfos() is not greater than zero but an array is being allocated in CvCityAI::AI_reset");
	m_pbEmphasize = new bool[GC.getNumEmphasizeInfos()];
	for (iI = 0; iI < GC.getNumEmphasizeInfos(); iI++)
	{
		m_pbEmphasize[iI] = false;
	}
}


void CvCityAI::AI_doTurn()
{
	PROFILE_FUNC();

	int iI;

	if (!isHuman())
	{
		for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
		{
			setForceSpecialistCount(((SpecialistTypes)iI), 0);
		}
	}

	AI_assignWorkingPlots();

	AI_updateBestBuild();

	AI_updateRouteToCity();

	if (isHuman())
	{
		return;
	}

	AI_doDraft();

	AI_doHurry();

	AI_doEmphasize();
}


// XXX this shouldn't be called as much and optimized with the new (always work every tile) system...
void CvCityAI::AI_assignWorkingPlots()
{
	PROFILE_FUNC();

	CvPlot* pHomePlot;
	int iI;

	if (!isHuman() || isCitizensAutomated())
	{
		for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
		{
			setWorkingPlot(iI, false);
		}
	}

	verifyWorkingPlots();

	for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
	{
		if (!isHuman() || isCitizensAutomated() || (getSpecialistCount((SpecialistTypes)iI) < getForceSpecialistCount((SpecialistTypes)iI)))
		{
			setSpecialistCount(((SpecialistTypes)iI), getForceSpecialistCount((SpecialistTypes)iI));
		}
	}

	for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
	{
		if (!isSpecialistValid((SpecialistTypes)iI))
		{
			if (getSpecialistCount((SpecialistTypes)iI) > getMaxSpecialistCount((SpecialistTypes)iI))
			{
				setSpecialistCount(((SpecialistTypes)iI), getMaxSpecialistCount((SpecialistTypes)iI));
			}
		}
	}

	pHomePlot = getCityIndexPlot(CITY_HOME_PLOT);

	if (pHomePlot != NULL)
	{
		setWorkingPlot(CITY_HOME_PLOT, ((getPopulation() > 0) && canWork(pHomePlot)));
	}

	while (extraPopulation() < 0)
	{
		if (!AI_removeWorstCitizen())
		{
			FAssert(false);
			break;
		}
	}

	FAssertMsg(extraSpecialists() >= 0, "extraSpecialists() is expected to be non-negative (invalid Index)");

	while (extraPopulation() > 0)
	{
		if (!AI_addBestCitizen(true, !isSpecialistForced()))
		{
			break;
		}
	}

	while (extraSpecialists() > 0)
	{
		if (!AI_addBestCitizen(false, true))
		{
			break;
		}
	}

	if (!isHuman() || isCitizensAutomated())
	{
		AI_juggleCitizens();
	}

	FAssert((getWorkingPopulation() + getSpecialistPopulation()) <= (totalFreeSpecialists() + getPopulation()));

	AI_setAssignWorkDirty(false);

	if ((getOwnerINLINE() == GC.getGameINLINE().getActivePlayer()) && isCitySelected())
	{
		gDLL->getInterfaceIFace()->setDirty(CitizenButtons_DIRTY_BIT, true);
	}
}


void CvCityAI::AI_updateAssignWork()
{
	if (!isHuman())
	{
		return;
	}

	if (AI_isAssignWorkDirty())
	{
		AI_assignWorkingPlots();
	}
}


bool CvCityAI::AI_avoidGrowth()
{
	PROFILE_FUNC();

	if (AI_isEmphasizeAvoidGrowth())
	{
		return true;
	}

	if (!AI_isEmphasizeYield(YIELD_FOOD))
	{
		if (angryPopulation((isHuman()) ? 0 : 1) > 0)
		{
			return true;
		}
	}

	return false;
}


bool CvCityAI::AI_ignoreGrowth()
{
	PROFILE_FUNC();

	if (!AI_isEmphasizeYield(YIELD_FOOD))
	{
		if (!AI_foodAvailable((isHuman()) ? 0 : 1))
		{
			return true;
		}
	}

	return false;
}


int CvCityAI::AI_specialistValue(SpecialistTypes eSpecialist, bool bAvoidGrowth, bool bRemove)
{
	PROFILE_FUNC();

	short aiYields[NUM_YIELD_TYPES];
	int iTempValue;
	int iCommerce;
	int iGreatPeopleRate;
	int iValue;
	int iI;
	int iProductionRank = findYieldRateRank(YIELD_PRODUCTION);
	int iNumCities = GET_PLAYER(getOwnerINLINE()).getNumCities();

	for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		aiYields[iI] = GET_PLAYER(getOwnerINLINE()).specialistYield(eSpecialist, ((YieldTypes)iI));
	}

	iValue = AI_yieldValue(aiYields, bAvoidGrowth, bRemove);

	for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		iCommerce = GET_PLAYER(getOwnerINLINE()).specialistCommerce(eSpecialist, ((CommerceTypes)iI));

		if (iCommerce != 0)
		{
			iCommerce = ((iCommerce * getTotalCommerceRateModifier((CommerceTypes)iI)) / 100);

			iTempValue = (iCommerce * 4);

			if (AI_isEmphasizeCommerce((CommerceTypes)iI))
			{
				iTempValue += (iCommerce * 16);
			}

			iTempValue *= GET_PLAYER(getOwnerINLINE()).AI_commerceWeight(((CommerceTypes)iI), this);
			iTempValue /= 100;

			iValue += iTempValue;
		}
	}

	iGreatPeopleRate = GC.getSpecialistInfo(eSpecialist).getGreatPeopleRateChange();

	if (iGreatPeopleRate != 0)
	{
		iGreatPeopleRate = ((iGreatPeopleRate * getTotalGreatPeopleRateModifier()) / 100);

		iTempValue = (iGreatPeopleRate * 4);

		if (AI_isEmphasizeGreatPeople())
		{
			iTempValue += (iGreatPeopleRate * 16);
		}

		iValue += iTempValue;
	}

	int iExperience = GC.getSpecialistInfo(eSpecialist).getExperience();
	if (0 != iExperience)
	{
		iExperience += getFreeExperience();

		int iHasMetCount = GET_TEAM(getTeam()).getHasMetCivCount(true);

		iValue += (iExperience * ((iHasMetCount > 0) ? 4 : 2));
		if (iProductionRank <= iNumCities/2 + 1)
		{
			iValue += iExperience *  4;
		}
	}

	return (iValue * 100);
}


void CvCityAI::AI_chooseProduction()
{
	PROFILE_FUNC();

	CvArea* pWaterArea;
	UnitTypes eProductionUnit;
	bool bWasFoodProduction;
	bool bHasMetHuman;
	bool bLandWar;
	bool bAssault;
	bool bPrimaryArea;
	bool bFinancialTrouble;
	bool bDanger;
	bool bChooseUnit;
	int iProductionRank;
	int iNeededMissionaries;
	int iI;

	AI_setChooseProductionDirty(false);

	if (isProduction())
	{
		if (getProduction() > 0)
		{
			if (getProductionTurnsLeft() <= 3)
			{
				return;
			}

			if (plot()->getNumDefenders(getOwnerINLINE()) == 0)
			{
				eProductionUnit = getProductionUnit();

				if (eProductionUnit != NO_UNIT)
				{
					if (GC.getUnitInfo(eProductionUnit).getCombat() > 0)
					{
						return;
					}
				}
			}
		}

		clearOrderQueue();
	}

	// allow python to handle it
	CyCity* pyCity = new CyCity(this);
	CyArgsList argsList;
	argsList.add(gDLL->getPythonIFace()->makePythonObject(pyCity));	// pass in city class
	long lResult=0;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "AI_chooseProduction", argsList.makeFunctionArgs(), &lResult);
	delete pyCity;	// python fxn must not hold on to this pointer
	if (lResult == 1)
	{
		return;
	}

	pWaterArea = waterArea();

	bWasFoodProduction = isFoodProduction();
	bHasMetHuman = GET_TEAM(getTeam()).hasMetHuman();
	bLandWar = ((area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE) || (area()->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE) || (area()->getAreaAIType(getTeam()) == AREAAI_MASSING));
	bAssault = (area()->getAreaAIType(getTeam()) == AREAAI_ASSAULT);
	bPrimaryArea = GET_PLAYER(getOwnerINLINE()).AI_isPrimaryArea(area());
	bFinancialTrouble = GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble();
	bDanger = AI_isDanger();

	iProductionRank = findYieldRateRank(YIELD_PRODUCTION);

	bChooseUnit = false;

	clearOrderQueue();

	if (bWasFoodProduction)
	{
		AI_assignWorkingPlots();
	}

	if (plot()->getNumDefenders(getOwnerINLINE()) == 0) // XXX check for other team's units?
	{
		if (AI_chooseUnit(UNITAI_CITY_DEFENSE))
		{
			return;
		}

		if (AI_chooseUnit(UNITAI_CITY_COUNTER))
		{
			return;
		}

		if (AI_chooseUnit(UNITAI_CITY_SPECIAL))
		{
			return;
		}

		if (AI_chooseUnit(UNITAI_ATTACK))
		{
			return;
		}
	}

	if (isHuman())
	{
		if (GET_PLAYER(getOwnerINLINE()).AI_totalUnitAIs(UNITAI_ATTACK) < 2)
		{
			if (AI_chooseUnit(UNITAI_ATTACK))
			{
				return;
			}
		}
	}
	else
	{
		if (bHasMetHuman)
		{
			if (!AI_isDefended(plot()->plotCount(PUF_isUnitAIType, UNITAI_ATTACK, -1, getOwnerINLINE()))) // XXX check for other team's units?
			{
				if (AI_chooseDefender())
				{
					return;
				}

				if (AI_chooseUnit(UNITAI_ATTACK))
				{
					return;
				}
			}
		}

		if (plot()->plotCheck(PUF_isUnitAIType, UNITAI_SETTLE, -1, getOwnerINLINE()) != NULL)
		{
			if (!AI_isDefended(-2))
			{
				if (AI_chooseDefender())
				{
					return;
				}
			}
		}
	}

	if (!bLandWar && !bDanger)
	{
		int iExistingWorkers = (GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(area(), UNITAI_WORKER));
		int iNeededWorkers = (GET_PLAYER(getOwnerINLINE()).AI_neededWorkers(area()));
		if ((iExistingWorkers == 0 && iNeededWorkers > 0) || iExistingWorkers < (iNeededWorkers + 1) / 2)
		{
			if (AI_chooseUnit(UNITAI_WORKER))
			{
				return;
			}
		}
	}

	if (pWaterArea != NULL)
	{
		if (GET_PLAYER(getOwnerINLINE()).AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_WORKER_SEA) == 0)
		{
			if (AI_neededSeaWorkers() > 0)
			{
				if (AI_chooseUnit(UNITAI_WORKER_SEA))
				{
					return;
				}
			}
		}
	}

	if (isHuman() || bHasMetHuman)
	{
		if (plot()->plotCheck(PUF_isUnitAIType, UNITAI_CITY_DEFENSE, -1, getOwnerINLINE()) == NULL)
		{
			if (AI_chooseUnit(UNITAI_CITY_DEFENSE))
			{
				return;
			}
		}
	}

	if (bPrimaryArea)
	{
		if (GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(area(), UNITAI_ATTACK) == 0)
		{
			if (AI_chooseUnit(UNITAI_ATTACK))
			{
				return;
			}
		}
	}

	if (!bLandWar && !bDanger)
	{
		if (GC.getGameINLINE().getElapsedGameTurns() < 50)
		{
			if (GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(area(), UNITAI_EXPLORE) < (GET_PLAYER(getOwnerINLINE()).AI_neededExplorers(area())))
			{
				if (AI_chooseUnit(UNITAI_EXPLORE))
				{
					return;
				}
			}
		}
	}

	if (bDanger)
	{
		if (GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(area(), UNITAI_ATTACK) < (area()->getCitiesPerPlayer(getOwnerINLINE()) + 2))
		{
			if (AI_chooseUnit(UNITAI_ATTACK))
			{
				return;
			}
		}
	}

	if (bAssault)
	{
		if (pWaterArea != NULL)
		{
			if (bPrimaryArea)
			{
				if (GET_PLAYER(getOwnerINLINE()).AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_ASSAULT_SEA) == 0)
				{
					if (AI_chooseUnit(UNITAI_ASSAULT_SEA))
					{
						return;
					}
				}

				if (plot()->plotCount(PUF_isUnitAIType, UNITAI_ASSAULT_SEA, -1, getOwnerINLINE()) == 1)
				{
					if (AI_chooseUnit(UNITAI_ASSAULT_SEA))
					{
						return;
					}
				}

				if (GET_PLAYER(getOwnerINLINE()).AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_ESCORT_SEA) < (GET_PLAYER(getOwnerINLINE()).AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_ASSAULT_SEA) + GET_PLAYER(getOwnerINLINE()).AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_CARRIER_SEA)))
				{
					if (AI_chooseUnit(UNITAI_ESCORT_SEA))
					{
						return;
					}
				}
			}
		}
	}

	if (!bLandWar)
	{
		if (GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(area(), UNITAI_SETTLE) == 0)
		{
			if (!bFinancialTrouble)
			{
				if (area()->getBestFoundValue(getOwnerINLINE()) > 0)
				{
					if ((GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(area(), UNITAI_CITY_DEFENSE) + GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(area(), UNITAI_ATTACK)) < (AI_neededDefenders() + area()->getCitiesPerPlayer(getOwnerINLINE())))
					{
						if (AI_chooseUnit(UNITAI_CITY_DEFENSE))
						{
							return;
						}

						if (AI_chooseUnit(UNITAI_ATTACK))
						{
							return;
						}
					}

					if (AI_chooseUnit(UNITAI_SETTLE))
					{
						return;
					}
				}
			}
		}

		if (pWaterArea != NULL)
		{
			if (bPrimaryArea)
			{
				if (GET_PLAYER(getOwnerINLINE()).AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_SETTLER_SEA) == 0)
				{
					if (GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(area(), UNITAI_SETTLE) > 0)
					{
						if ((area()->getBestFoundValue(getOwnerINLINE()) == 0) || (GC.getGameINLINE().getSorenRandNum(10, "AI Build Settler Sea") == 0))
						{
							if (AI_chooseUnit(UNITAI_SETTLER_SEA))
							{
								return;
							}
						}
					}
				}
			}

			if (GET_PLAYER(getOwnerINLINE()).AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_MISSIONARY_SEA) == 0)
			{
				if (GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(area(), UNITAI_MISSIONARY) > 0)
				{
					if (AI_chooseUnit(UNITAI_MISSIONARY_SEA))
					{
						return;
					}
				}
			}
		}
	}

	if (!isBarbarian())
	{
		if (getCommerceRateTimes100(COMMERCE_CULTURE) == 0)
		{
			if (AI_chooseBuilding(BUILDINGFOCUS_CULTURE, 30))
			{
				return;
			}
		}
	}

	if (!isHuman())
	{
		if (!AI_isDefended())
		{
			if (AI_chooseDefender())
			{
				return;
			}
		}

		if (getPopulation() > 6)
		{
			if (!AI_isAirDefended())
			{
				if (AI_chooseUnit(UNITAI_DEFENSE_AIR))
				{
					return;
				}
			}
		}
	}

	if (bDanger || (bHasMetHuman && (GC.getGameINLINE().getSorenRandNum(30, "AI Build Defense") < getPopulation())))
	{
		if (AI_chooseBuilding(BUILDINGFOCUS_DEFENSE, 20))
		{
			return;
		}
	}

	if (AI_chooseBuilding(BUILDINGFOCUS_FOOD, 80))
	{
		return;
	}

	if (getBaseYieldRate(YIELD_PRODUCTION) >= 8)
	{
		if (AI_chooseBuilding(BUILDINGFOCUS_PRODUCTION, 30))
		{
			return;
		}
	}

	if (iProductionRank <= ((GET_PLAYER(getOwnerINLINE()).getNumCities() / 3) + 1))
	{
		if (AI_chooseProject())
		{
			return;
		}
	}

	if (!bLandWar)
	{
		if (getMaintenance() >= 8)
		{
			if (AI_chooseBuilding(BUILDINGFOCUS_MAINTENANCE, 40))
			{
				return;
			}
		}

		if (angryPopulation(1) > 0)
		{
			if (AI_chooseBuilding(BUILDINGFOCUS_HAPPY, 40))
			{
				return;
			}
		}

		if (healthRate(1) < 0)
		{
			if (AI_chooseBuilding(BUILDINGFOCUS_HEALTHY, 40))
			{
				return;
			}
		}

		if (!AI_foodAvailable(2))
		{
			if (AI_chooseBuilding(BUILDINGFOCUS_FOOD, 60))
			{
				return;
			}
		}

		if (getMaintenance() >= 6)
		{
			if (AI_chooseBuilding(BUILDINGFOCUS_MAINTENANCE, 60))
			{
				return;
			}
		}

		if (plot()->calculateCulturePercent(getOwnerINLINE()) < 50)
		{
			if (AI_chooseBuilding(BUILDINGFOCUS_CULTURE, 60))
			{
				return;
			}
		}

		if (GC.getDefineINT("DEFAULT_SPECIALIST") != NO_SPECIALIST)
		{
			if (getSpecialistCount((SpecialistTypes)(GC.getDefineINT("DEFAULT_SPECIALIST"))) > 0)
			{
				if (AI_chooseBuilding(BUILDINGFOCUS_SPECIALIST, 60))
				{
					return;
				}
			}
		}

		if (bFinancialTrouble)
		{
			if (getBaseCommerceRate(COMMERCE_GOLD) >= 12)
			{
				if (AI_chooseBuilding(BUILDINGFOCUS_GOLD, 40))
				{
					return;
				}
			}
		}

		if (!(GET_PLAYER(getOwnerINLINE()).AI_avoidScience()))
		{
			if (getBaseCommerceRate(COMMERCE_RESEARCH) >= 12)
			{
				if (AI_chooseBuilding(BUILDINGFOCUS_RESEARCH, 40))
				{
					return;
				}
			}
		}

		if (!bDanger)
		{
			if (countNumImprovedPlots() < (getPopulation() / 2))
			{
				if (plot()->plotCheck(PUF_isUnitAIType, UNITAI_WORKER, -1, getOwnerINLINE()) == NULL)
				{
					if (GET_PLAYER(getOwnerINLINE()).AI_neededWorkers(area()) > (GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(area(), UNITAI_WORKER) * 2))
					{
						if (AI_chooseUnit(UNITAI_WORKER))
						{
							return;
						}
					}
				}
			}
		}

		if (pWaterArea != NULL)
		{
			if (bPrimaryArea)
			{
				if (GET_PLAYER(getOwnerINLINE()).AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_EXPLORE_SEA) < min(1, GET_PLAYER(getOwnerINLINE()).AI_neededExplorers(pWaterArea)))
				{
					if (AI_chooseUnit(UNITAI_EXPLORE_SEA))
					{
						return;
					}
				}
			}
		}

		if (getBaseYieldRate(YIELD_PRODUCTION) >= 8)
		{
			if (AI_chooseBuilding(BUILDINGFOCUS_PRODUCTION, 80))
			{
				return;
			}
		}

		if (GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(area(), UNITAI_SETTLE) == 0)
		{
			if (!bFinancialTrouble)
			{
				if (bPrimaryArea && (GET_PLAYER(getOwnerINLINE()).findBestFoundValue() > 0))
				{
					if (AI_chooseUnit(UNITAI_SETTLE))
					{
						return;
					}
				}
			}
		}
	}

	if (AI_chooseBuilding(BUILDINGFOCUS_EXPERIENCE, 6))
	{
		return;
	}

	if (iProductionRank <= ((GET_PLAYER(getOwnerINLINE()).getNumCities() / 2) + 1))
	{
		if (AI_chooseBuilding(BUILDINGFOCUS_EXPERIENCE, 40))
		{
			return;
		}
	}

	if (!isHuman())
	{
		if (plot()->plotCheck(PUF_isUnitAIType, UNITAI_CITY_COUNTER, -1, getOwnerINLINE()) == NULL)
		{
			if (AI_chooseUnit(UNITAI_CITY_COUNTER))
			{
				return;
			}
		}
	}

	if (bPrimaryArea)
	{
		if (GET_PLAYER(getOwnerINLINE()).getNumCities() > 2)
		{
			if (GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(area(), UNITAI_RESERVE) == 0)
			{
				if (AI_chooseUnit(UNITAI_RESERVE))
				{
					return;
				}
			}
		}
	}

	if (pWaterArea != NULL)
	{
		if (bPrimaryArea)
		{
			if (plot()->plotCount(PUF_isUnitAIType, UNITAI_ASSAULT_SEA, -1, getOwnerINLINE()) > 0)
			{
				if (plot()->plotCount(PUF_isUnitAIType, UNITAI_ESCORT_SEA, -1, getOwnerINLINE()) == 0)
				{
					if (AI_chooseUnit(UNITAI_ESCORT_SEA))
					{
						return;
					}
				}
			}
		}
	}

	if (!bDanger)
	{
		if (GC.getGameINLINE().getSorenRandNum(100, "AI Build Missionary") < ((bLandWar) ? 5 : 20))
		{
			for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
			{
				iNeededMissionaries = GET_PLAYER(getOwnerINLINE()).AI_neededMissionaries(area(), ((ReligionTypes)iI));

				if (iNeededMissionaries > 0)
				{
					if (iNeededMissionaries > GET_PLAYER(getOwnerINLINE()).countReligionSpreadUnits(area(), ((ReligionTypes)iI)))
					{
						if (AI_chooseUnit(UNITAI_MISSIONARY))
						{
							return;
						}
					}
				}
			}
		}
	}

	if (GC.getGameINLINE().getSorenRandNum(10, "AI Build Spy") == 0)
	{
		if (GET_PLAYER(getOwnerINLINE()).AI_totalUnitAIs(UNITAI_SPY) < ((GET_PLAYER(getOwnerINLINE()).getNumCities() / 4) + 1))
		{
			if (AI_chooseUnit(UNITAI_SPY))
			{
				return;
			}
		}
	}

	if (!bLandWar)
	{
		if (pWaterArea != NULL)
		{
			if (GET_PLAYER(getOwnerINLINE()).AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_SPY_SEA) == 0)
			{
				if (GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(area(), UNITAI_SPY) > 0)
				{
					if (AI_chooseUnit(UNITAI_SPY_SEA))
					{
						return;
					}
				}
			}
		}
	}

	if ((bLandWar) ||
		  ((GET_PLAYER(getOwnerINLINE()).getNumCities() <= 3) && (GC.getGameINLINE().getElapsedGameTurns() < 60)) ||
		  (GC.getGameINLINE().getSorenRandNum(100, "AI Build Unit Production") < AI_buildUnitProb()) ||
			(isHuman() && (getGameTurnFounded() == GC.getGameINLINE().getGameTurn())))
	{
		if (AI_chooseUnit())
		{
			return;
		}

		bChooseUnit = true;
	}

	if (AI_chooseProject())
	{
		return;
	}

	if (AI_chooseBuilding())
	{
		return;
	}

	if (!bChooseUnit)
	{
		if (AI_chooseUnit())
		{
			return;
		}
	}

	if (AI_chooseProcess())
	{
		return;
	}
}


UnitTypes CvCityAI::AI_bestUnit(bool bAsync, AdvisorTypes eIgnoreAdvisor, UnitAITypes* peBestUnitAI)
{
	CvArea* pWaterArea;
	int aiUnitAIVal[NUM_UNITAI_TYPES];
	UnitTypes eUnit;
	UnitTypes eBestUnit;
	bool bWarPlan;
	bool bDefense;
	bool bLandWar;
	bool bAssault;
	bool bPrimaryArea;
	bool bAreaAlone;
	bool bFinancialTrouble;
	bool bWarPossible;
	bool bDanger;
	int iHasMetCount;
	int iMilitaryWeight;
	int iAreaCities;
	int iCoastalCities;
	int iBestValue;
	int iI;

	if (peBestUnitAI != NULL)
	{
		*peBestUnitAI = NO_UNITAI;
	}

	pWaterArea = waterArea();

	bWarPlan = (GET_TEAM(getTeam()).getAnyWarPlanCount(true) > 0);
	bDefense = (area()->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE);
	bLandWar = (bDefense || (area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE) || (area()->getAreaAIType(getTeam()) == AREAAI_MASSING));
	bAssault = (area()->getAreaAIType(getTeam()) == AREAAI_ASSAULT);
	bPrimaryArea = GET_PLAYER(getOwnerINLINE()).AI_isPrimaryArea(area());
	bAreaAlone = GET_PLAYER(getOwnerINLINE()).AI_isAreaAlone(area());
	bFinancialTrouble = GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble();
	bWarPossible = GET_TEAM(getTeam()).AI_isWarPossible();
	bDanger = AI_isDanger();

	iHasMetCount = GET_TEAM(getTeam()).getHasMetCivCount(true);
	iMilitaryWeight = GET_PLAYER(getOwnerINLINE()).AI_militaryWeight(area());
	iAreaCities = area()->getCitiesPerPlayer(getOwnerINLINE());
	if (pWaterArea != NULL)
	{
		iCoastalCities = GET_PLAYER(getOwnerINLINE()).countNumCoastalCitiesByArea(pWaterArea);
	}

	for (iI = 0; iI < NUM_UNITAI_TYPES; iI++)
	{
		aiUnitAIVal[iI] = 0;
	}

	if (!bFinancialTrouble && ((bPrimaryArea) ? (GET_PLAYER(getOwnerINLINE()).findBestFoundValue() > 0) : (area()->getBestFoundValue(getOwnerINLINE()) > 0)))
	{
		aiUnitAIVal[UNITAI_SETTLE]++;
	}

	aiUnitAIVal[UNITAI_WORKER] += GET_PLAYER(getOwnerINLINE()).AI_neededWorkers(area());

	aiUnitAIVal[UNITAI_ATTACK] += ((iMilitaryWeight / ((bWarPlan || bLandWar || bAssault) ? 7 : 12)) + ((bPrimaryArea && bWarPossible) ? 2 : 0) + 1);

	aiUnitAIVal[UNITAI_CITY_DEFENSE] += (iAreaCities + 1);
	aiUnitAIVal[UNITAI_CITY_COUNTER] += ((5 * (iAreaCities + 1)) / 8);
	aiUnitAIVal[UNITAI_CITY_SPECIAL] += ((iAreaCities + 1) / 2);

	if (bWarPossible)
	{
		aiUnitAIVal[UNITAI_ATTACK_CITY] += ((iMilitaryWeight / ((bWarPlan || bLandWar || bAssault) ? 10 : 17)) + ((bPrimaryArea) ? 1 : 0));
		aiUnitAIVal[UNITAI_COUNTER] += ((iMilitaryWeight / ((bWarPlan || bLandWar || bAssault) ? 13 : 22)) + ((bPrimaryArea) ? 1 : 0));

		aiUnitAIVal[UNITAI_DEFENSE_AIR] += (GET_PLAYER(getOwnerINLINE()).getNumCities() + 1);
		aiUnitAIVal[UNITAI_CARRIER_AIR] += GET_PLAYER(getOwnerINLINE()).AI_countCargoSpace(UNITAI_CARRIER_SEA);

		if (bPrimaryArea)
		{
			aiUnitAIVal[UNITAI_ICBM] += max((GET_PLAYER(getOwnerINLINE()).getTotalPopulation() / 25), ((GC.getGameINLINE().countCivPlayersAlive() + GC.getGameINLINE().countTotalNukeUnits()) / (GC.getGameINLINE().countCivPlayersAlive() + 1)));
		}
	}

	if (isBarbarian())
	{
		aiUnitAIVal[UNITAI_ATTACK] *= 2;
	}
	else
	{
		if (!bLandWar)
		{
			aiUnitAIVal[UNITAI_EXPLORE] += GET_PLAYER(getOwnerINLINE()).AI_neededExplorers(area());
		}

		if (pWaterArea != NULL)
		{
			aiUnitAIVal[UNITAI_WORKER_SEA] += AI_neededSeaWorkers();

			if ((GET_PLAYER(getOwnerINLINE()).getNumCities() > 3) || (area()->getNumUnownedTiles() < 10))
			{
				if (bPrimaryArea)
				{
					aiUnitAIVal[UNITAI_EXPLORE_SEA] += GET_PLAYER(getOwnerINLINE()).AI_neededExplorers(pWaterArea);
				}

				if (bPrimaryArea && (GET_PLAYER(getOwnerINLINE()).findBestFoundValue() > 0) && (pWaterArea->getNumTiles() > 300))
				{
					aiUnitAIVal[UNITAI_SETTLER_SEA]++;
				}

				if (bPrimaryArea && (GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(area(), UNITAI_MISSIONARY) > 0) && (pWaterArea->getNumTiles() > 400))
				{
					aiUnitAIVal[UNITAI_MISSIONARY_SEA]++;
				}

				if (bPrimaryArea && (GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(area(), UNITAI_SPY) > 0) && (pWaterArea->getNumTiles() > 500))
				{
					aiUnitAIVal[UNITAI_SPY_SEA]++;
				}

				if (bWarPossible)
				{
					aiUnitAIVal[UNITAI_ATTACK_SEA] += min((pWaterArea->getNumTiles() / 150), ((((iCoastalCities * 2) + (iMilitaryWeight / 9)) / ((bAssault) ? 4 : 6)) + ((bPrimaryArea) ? 1 : 0)));
					aiUnitAIVal[UNITAI_RESERVE_SEA] += min((pWaterArea->getNumTiles() / 200), ((((iCoastalCities * 2) + (iMilitaryWeight / 7)) / 5) + ((bPrimaryArea) ? 1 : 0)));
					aiUnitAIVal[UNITAI_ESCORT_SEA] += (GET_PLAYER(getOwnerINLINE()).AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_ASSAULT_SEA) + (GET_PLAYER(getOwnerINLINE()).AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_CARRIER_SEA) * 2));
					aiUnitAIVal[UNITAI_ASSAULT_SEA] += min((pWaterArea->getNumTiles() / 250), ((((iCoastalCities * 2) + (iMilitaryWeight / 6)) / ((bAssault) ? 5 : 8)) + ((bPrimaryArea) ? 1 : 0)));
					aiUnitAIVal[UNITAI_CARRIER_SEA] += min((pWaterArea->getNumTiles() / 350), ((((iCoastalCities * 2) + (iMilitaryWeight / 8)) / 7) + ((bPrimaryArea) ? 1 : 0)));
				}
			}
		}

		if ((iHasMetCount > 0) && bWarPossible)
		{
			if (bLandWar || bAssault || !bFinancialTrouble || (GET_PLAYER(getOwnerINLINE()).calculateUnitCost() == 0))
			{
				aiUnitAIVal[UNITAI_ATTACK] += ((iMilitaryWeight / ((bLandWar || bAssault) ? 9 : 16)) + ((bPrimaryArea && !bAreaAlone) ? 1 : 0));
				aiUnitAIVal[UNITAI_ATTACK_CITY] += ((iMilitaryWeight / ((bLandWar || bAssault) ? 7 : 15)) + ((bPrimaryArea && !bAreaAlone) ? 1 : 0));
				aiUnitAIVal[UNITAI_COLLATERAL] += ((iMilitaryWeight / ((bDefense) ? 8 : 14)) + ((bPrimaryArea && !bAreaAlone) ? 1 : 0));
				aiUnitAIVal[UNITAI_PILLAGE] += ((iMilitaryWeight / ((bLandWar || bAssault) ? 10 : 19)) + ((bPrimaryArea && !bAreaAlone) ? 1 : 0));
				aiUnitAIVal[UNITAI_RESERVE] += ((iMilitaryWeight / ((bLandWar) ? 12 : 17)) + ((bPrimaryArea && !bAreaAlone) ? 1 : 0));
				aiUnitAIVal[UNITAI_COUNTER] += ((iMilitaryWeight / ((bLandWar || bAssault) ? 9 : 16)) + ((bPrimaryArea && !bAreaAlone) ? 1 : 0));

				aiUnitAIVal[UNITAI_ATTACK_AIR] += (GET_PLAYER(getOwnerINLINE()).getNumCities() + 1);

				if (pWaterArea != NULL)
				{
					if ((GET_PLAYER(getOwnerINLINE()).getNumCities() > 3) || (area()->getNumUnownedTiles() < 10))
					{
						aiUnitAIVal[UNITAI_ATTACK_SEA] += min((pWaterArea->getNumTiles() / 100), ((((iCoastalCities * 2) + (iMilitaryWeight / 10)) / ((bAssault) ? 5 : 7)) + ((bPrimaryArea) ? 1 : 0)));
						aiUnitAIVal[UNITAI_RESERVE_SEA] += min((pWaterArea->getNumTiles() / 150), ((((iCoastalCities * 2) + (iMilitaryWeight / 11)) / 8) + ((bPrimaryArea) ? 1 : 0)));
					}
				}
			}
		}
	}

	// XXX this should account for air and heli units too...
	for (iI = 0; iI < NUM_UNITAI_TYPES; iI++)
	{
		if (GET_PLAYER(getOwnerINLINE()).AI_unitAIDomainType((UnitAITypes)iI) == DOMAIN_SEA)
		{
			if (pWaterArea != NULL)
			{
				aiUnitAIVal[iI] -= GET_PLAYER(getOwnerINLINE()).AI_totalWaterAreaUnitAIs(pWaterArea, ((UnitAITypes)iI));
			}
		}
		else if ((GET_PLAYER(getOwnerINLINE()).AI_unitAIDomainType((UnitAITypes)iI) == DOMAIN_AIR) || (iI == UNITAI_ICBM))
		{
			aiUnitAIVal[iI] -= GET_PLAYER(getOwnerINLINE()).AI_totalUnitAIs((UnitAITypes)iI);
		}
		else
		{
			aiUnitAIVal[iI] -= GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(area(), ((UnitAITypes)iI));
		}
	}

	aiUnitAIVal[UNITAI_SETTLE] *= ((bDanger) ? 8 : 20);
	aiUnitAIVal[UNITAI_WORKER] *= ((bDanger) ? 2 : 7);
	aiUnitAIVal[UNITAI_ATTACK] *= 3;
	aiUnitAIVal[UNITAI_ATTACK_CITY] *= 4;
	aiUnitAIVal[UNITAI_COLLATERAL] *= 5;
	aiUnitAIVal[UNITAI_PILLAGE] *= 3;
	aiUnitAIVal[UNITAI_RESERVE] *= 3;
	aiUnitAIVal[UNITAI_COUNTER] *= 3;
	aiUnitAIVal[UNITAI_CITY_DEFENSE] *= 2;
	aiUnitAIVal[UNITAI_CITY_COUNTER] *= 2;
	aiUnitAIVal[UNITAI_CITY_SPECIAL] *= 2;
	aiUnitAIVal[UNITAI_EXPLORE] *= ((bDanger) ? 6 : 15);
	aiUnitAIVal[UNITAI_ICBM] *= 18;
	aiUnitAIVal[UNITAI_WORKER_SEA] *= ((bDanger) ? 3 : 10);
	aiUnitAIVal[UNITAI_ATTACK_SEA] *= 5;
	aiUnitAIVal[UNITAI_RESERVE_SEA] *= 4;
	aiUnitAIVal[UNITAI_ESCORT_SEA] *= 20;
	aiUnitAIVal[UNITAI_EXPLORE_SEA] *= 18;
	aiUnitAIVal[UNITAI_ASSAULT_SEA] *= 14;
	aiUnitAIVal[UNITAI_SETTLER_SEA] *= 16;
	aiUnitAIVal[UNITAI_MISSIONARY_SEA] *= 12;
	aiUnitAIVal[UNITAI_SPY_SEA] *= 10;
	aiUnitAIVal[UNITAI_CARRIER_SEA] *= 8;
	aiUnitAIVal[UNITAI_ATTACK_AIR] *= 6;
	aiUnitAIVal[UNITAI_DEFENSE_AIR] *= 3;
	aiUnitAIVal[UNITAI_CARRIER_AIR] *= 15;

	for (iI = 0; iI < NUM_UNITAI_TYPES; iI++)
	{
		aiUnitAIVal[iI] *= max(0, (GC.getLeaderHeadInfo(getPersonalityType()).getUnitAIWeightModifier(iI) + 100));
		aiUnitAIVal[iI] /= 100;
	}

	iBestValue = 0;
	eBestUnit = NO_UNIT;

	for (iI = 0; iI < NUM_UNITAI_TYPES; iI++)
	{
		if (aiUnitAIVal[iI] > 0)
		{
			if (bAsync)
			{
				aiUnitAIVal[iI] += GC.getASyncRand().get(iMilitaryWeight, "AI Best UnitAI ASYNC");
			}
			else
			{
				aiUnitAIVal[iI] += GC.getGameINLINE().getSorenRandNum(iMilitaryWeight, "AI Best UnitAI");
			}

			if (aiUnitAIVal[iI] > iBestValue)
			{
				eUnit = AI_bestUnitAI(((UnitAITypes)iI), bAsync, eIgnoreAdvisor);

				if (eUnit != NO_UNIT)
				{
					iBestValue = aiUnitAIVal[iI];
					eBestUnit = eUnit;
					if (peBestUnitAI != NULL)
					{
						*peBestUnitAI = ((UnitAITypes)iI);
					}
				}
			}
		}
	}

	return eBestUnit;
}


UnitTypes CvCityAI::AI_bestUnitAI(UnitAITypes eUnitAI, bool bAsync, AdvisorTypes eIgnoreAdvisor)
{
	UnitTypes eLoopUnit;
	UnitTypes eBestUnit;
	int iValue;
	int iBestValue;
	int iOriginalValue;
	int iBestOriginalValue;
	int iI;

	FAssertMsg(eUnitAI != NO_UNITAI, "UnitAI is not assigned a valid value");

	iBestOriginalValue = 0;

	for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
	{
		eLoopUnit = ((UnitTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI)));

		if (eLoopUnit != NO_UNIT)
		{
			if ((eIgnoreAdvisor == NO_ADVISOR) || (GC.getUnitInfo(eLoopUnit).getAdvisorType() != eIgnoreAdvisor))
			{
				if (!isHuman() || (GC.getUnitInfo(eLoopUnit).getDefaultUnitAIType() == eUnitAI))
				{
					if (!isFoodProduction(eLoopUnit) || (getPopulation() > 2))
					{
						if (canTrain(eLoopUnit))
						{
							iOriginalValue = GET_PLAYER(getOwnerINLINE()).AI_unitValue(eLoopUnit, eUnitAI, area());

							if (iOriginalValue > iBestOriginalValue)
							{
								iBestOriginalValue = iOriginalValue;
							}
						}
					}
				}
			}
		}
	}

	iBestValue = 0;
	eBestUnit = NO_UNIT;

	for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
	{
		eLoopUnit = ((UnitTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI)));

		if (eLoopUnit != NO_UNIT)
		{
			if ((eIgnoreAdvisor == NO_ADVISOR) || (GC.getUnitInfo(eLoopUnit).getAdvisorType() != eIgnoreAdvisor))
			{
				if (!isHuman() || (GC.getUnitInfo(eLoopUnit).getDefaultUnitAIType() == eUnitAI))
				{
					if (!isFoodProduction(eLoopUnit) || (getPopulation() > 1))
					{
						if (canTrain(eLoopUnit))
						{
							iValue = GET_PLAYER(getOwnerINLINE()).AI_unitValue(eLoopUnit, eUnitAI, area());

							if (iValue > ((iBestOriginalValue * 2) / 3))
							{
								iValue *= (getProductionExperience(eLoopUnit) + 10);
								iValue /= 10;

								if (bAsync)
								{
									iValue *= (GC.getASyncRand().get(50, "AI Best Unit ASYNC") + 100);
									iValue /= 100;
								}
								else
								{
									iValue *= (GC.getGameINLINE().getSorenRandNum(50, "AI Best Unit") + 100);
									iValue /= 100;
								}


								int iBestHappy = 0;
								for (int iHurry = 0; iHurry < GC.getNumHurryInfos(); ++iHurry)
								{
									if (canHurryUnit((HurryTypes)iHurry, eLoopUnit, true))
									{
										int iHappy = AI_getHappyFromHurry((HurryTypes)iHurry, eLoopUnit, true);
										if (iHappy > iBestHappy)
										{
											iBestHappy = iHappy;
										}
									}
								}

								if (0 == iBestHappy)
								{
									iValue += getUnitProduction(eLoopUnit);
								}

								iValue *= (GET_PLAYER(getOwnerINLINE()).getNumCities() * 2);
								iValue /= (GET_PLAYER(getOwnerINLINE()).getUnitClassCountPlusMaking((UnitClassTypes)iI) + GET_PLAYER(getOwnerINLINE()).getNumCities() + 1);

								FAssert((MAX_INT / 1000) > iValue);
								iValue *= 1000;

								if (0 == iBestHappy)
								{
									iValue /= max(1, (getProductionTurnsLeft(eLoopUnit, 0) + 10));
								}
								else
								{
									iValue *= (10 + 2 * iBestHappy);
									iValue /= 100;
								}

								iValue = max(1, iValue);

								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									eBestUnit = eLoopUnit;
								}
							}
						}
					}
				}
			}
		}
	}

	return eBestUnit;
}


BuildingTypes CvCityAI::AI_bestBuilding(int iFocusFlags, int iMaxTurns, bool bAsync, AdvisorTypes eIgnoreAdvisor)
{
	BuildingTypes eLoopBuilding;
	BuildingTypes eBestBuilding;
	bool bAreaAlone;
	int iProductionRank;
	int iTurnsLeft;
	int iValue;
	int iTempValue;
	int iBestValue;
	int iI;

	bAreaAlone = GET_PLAYER(getOwnerINLINE()).AI_isAreaAlone(area());

	iProductionRank = findYieldRateRank(YIELD_PRODUCTION);

	iBestValue = 0;
	eBestBuilding = NO_BUILDING;

	for (iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
	{
		if (!(GET_PLAYER(getOwnerINLINE()).isBuildingClassMaxedOut(((BuildingClassTypes)iI), GC.getBuildingClassInfo((BuildingClassTypes)iI).getExtraPlayerInstances())))
		{
			eLoopBuilding = ((BuildingTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iI)));

			if (eLoopBuilding != NO_BUILDING)
			{
				if ((eIgnoreAdvisor == NO_ADVISOR) || (GC.getBuildingInfo(eLoopBuilding).getAdvisorType() != eIgnoreAdvisor))
				{
					if (canConstruct(eLoopBuilding))
					{
						iValue = AI_buildingValue(eLoopBuilding, iFocusFlags);

						if (GC.getBuildingInfo(eLoopBuilding).getFreeBuildingClass() != NO_BUILDINGCLASS)
						{
							BuildingTypes eFreeBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(GC.getBuildingInfo(eLoopBuilding).getFreeBuildingClass());
							if (NO_BUILDING != eFreeBuilding)
							{
								iValue += (AI_buildingValue(eFreeBuilding, iFocusFlags) * (GET_PLAYER(getOwnerINLINE()).getNumCities() - GET_PLAYER(getOwnerINLINE()).getBuildingClassCountPlusMaking((BuildingClassTypes)GC.getBuildingInfo(eLoopBuilding).getFreeBuildingClass())));
							}
						}

						if (iValue > 0)
						{
							iTurnsLeft = getProductionTurnsLeft(eLoopBuilding, 0);

							if (isWorldWonderClass((BuildingClassTypes)iI))
							{
								if (iProductionRank <= min(3, ((GET_PLAYER(getOwnerINLINE()).getNumCities() + 2) / 3)))
								{
									if (bAsync)
									{
										iTempValue = GC.getASyncRand().get(GC.getLeaderHeadInfo(getPersonalityType()).getWonderConstructRand(), "Wonder Construction Rand ASYNC");
									}
									else
									{
										iTempValue = GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getWonderConstructRand(), "Wonder Construction Rand");
									}

									if (bAreaAlone)
									{
										iTempValue *= 2;
									}

									iValue += iTempValue;

									if (AI_isEmphasizeYield(YIELD_FOOD))
									{
										// Building the Wonder will no longer emphasize food, so we will gain production
										iTurnsLeft *= 2;
										iTurnsLeft /= 3;
									}
								}
							}

							if (bAsync)
							{
								iValue *= (GC.getASyncRand().get(25, "AI Best Building ASYNC") + 100);
								iValue /= 100;
							}
							else
							{
								iValue *= (GC.getGameINLINE().getSorenRandNum(25, "AI Best Building") + 100);
								iValue /= 100;
							}

							iValue += getBuildingProduction(eLoopBuilding);

							bool bValid = (iTurnsLeft <= GC.getGameINLINE().AI_turnsPercent(iMaxTurns, GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getConstructPercent()));
							if (!bValid)
							{
								for (int iHurry = 0; iHurry < GC.getNumHurryInfos(); ++iHurry)
								{
									if (canHurryBuilding((HurryTypes)iHurry, eLoopBuilding, true))
									{
										if (AI_getHappyFromHurry((HurryTypes)iHurry, eLoopBuilding, true) > 0)
										{
											bValid = true;
											break;
										}
									}
								}
							}

							if (bValid)
							{
								FAssert((MAX_INT / 1000) > iValue);
								iValue *= 1000;
								iValue /= max(1, (iTurnsLeft + 10));

								iValue = max(1, iValue);

								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									eBestBuilding = eLoopBuilding;
								}
							}
						}
					}
				}
			}
		}
	}

	return eBestBuilding;
}


// XXX should some of these count cities, buildings, etc. based on teams (because wonders are shared...)
// XXX in general, this function needs to be more sensitive to what makes this city unique (more likely to build airports if there already is a harbor...)
int CvCityAI::AI_buildingValue(BuildingTypes eBuilding, int iFocusFlags)
{
	PROFILE_FUNC();

	ReligionTypes eStateReligion;
	bool bAreaAlone;
	bool bProvidesPower;
	bool bCanPopRush;
	int iHasMetCount;
	int iBaseMaintenance;
	int iFoodDifference;
	int iAngryPopulation;
	int iBadHealth;
	int iValue;
	int iTempValue;
	int iPass;
	int iI, iJ;
	int iProductionRank;
	int iNumCities;

	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
	CvBuildingInfo& kBuilding = GC.getBuildingInfo(eBuilding);

	eStateReligion = kOwner.getStateReligion();

	bAreaAlone = kOwner.AI_isAreaAlone(area());
	bProvidesPower = (kBuilding.isPower() || ((kBuilding.getPowerBonus() != NO_BONUS) && hasBonus((BonusTypes)(kBuilding.getPowerBonus()))));

	iHasMetCount = GET_TEAM(getTeam()).getHasMetCivCount(true);
	iBaseMaintenance = calculateBaseMaintenanceTimes100()/100; // XXX is this slow?
	iFoodDifference = foodDifference(false);
	iAngryPopulation = angryPopulation(1);
	FAssert(iAngryPopulation >= 0);
	iBadHealth = max(0, -(healthRate(1)));
	iProductionRank = findBaseYieldRateRank(YIELD_PRODUCTION);
	iNumCities = kOwner.getNumCities();

	bCanPopRush = false;
	for (iI = 0; iI < GC.getNumHurryInfos(); ++iI)
	{
		if (GC.getHurryInfo((HurryTypes)iI).getProductionPerPopulation() > 0 && GET_PLAYER(getOwnerINLINE()).canHurry((HurryTypes)iI))
		{
			bCanPopRush = true;
			break;
		}
	}

	if (kBuilding.isCapital())
	{
		FAssert(false);
		return 0;
	}

	if (kBuilding.getObsoleteTech() != NO_TECH)
	{
		FAssertMsg(!(GET_TEAM(getTeam()).isHasTech((TechTypes)(kBuilding.getObsoleteTech()))), "Team expected to not have the tech that obsoletes eBuilding");
		if (kOwner.canResearch((TechTypes)(kBuilding.getObsoleteTech())))
		{
			return 0;
		}
	}

	if (kBuilding.getSpecialBuildingType() != NO_SPECIALBUILDING)
	{
		if (GC.getSpecialBuildingInfo((SpecialBuildingTypes) kBuilding.getSpecialBuildingType()).getObsoleteTech() != NO_TECH)
		{
			FAssertMsg(!(GET_TEAM(getTeam()).isHasTech((TechTypes)(GC.getSpecialBuildingInfo((SpecialBuildingTypes) kBuilding.getSpecialBuildingType()).getObsoleteTech()))), "Team expected to not have the tech that obsoletes eBuilding");
			if (kOwner.canResearch((TechTypes)(GC.getSpecialBuildingInfo((SpecialBuildingTypes) kBuilding.getSpecialBuildingType()).getObsoleteTech())))
			{
				return 0;
			}
		}
	}

	for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
	{
		if (kBuilding.getReligionChange(iI) > 0)
		{
			if (!(GET_TEAM(getTeam()).hasHolyCity((ReligionTypes)iI)))
			{
				return 0;
			}
		}
	}

	iValue = 0;

	for (iPass = 0; iPass < 2; iPass++)
	{
		if ((iFocusFlags == 0) || (iValue > 0) || (iPass == 0))
		{
			if ((iFocusFlags & BUILDINGFOCUS_DEFENSE) || (iPass > 0))
			{
				if (!bAreaAlone)
				{
					if ((GC.getGameINLINE().getBestLandUnit() == NO_UNIT) || !(GC.getUnitInfo(GC.getGameINLINE().getBestLandUnit()).isIgnoreBuildingDefense()))
					{
						iValue += (max(0, min(((kBuilding.getDefenseModifier() + getBuildingDefense()) - getNaturalDefense() - 10), kBuilding.getDefenseModifier())) / 4);
					}
				}

				iValue += kBuilding.getBombardDefenseModifier() / 8;

				iValue += ((kBuilding.getAllCityDefenseModifier() * iNumCities) / 5);
			}

			if ((iFocusFlags & BUILDINGFOCUS_HAPPY) || (iPass > 0))
			{
				int iBestHappy = 0;
				for (iI = 0; iI < GC.getNumHurryInfos(); iI++)
				{
					if (canHurryBuilding((HurryTypes)iI, eBuilding, true))
					{
						int iHappyFromHurry = AI_getHappyFromHurry((HurryTypes)iI, eBuilding, true);
						if (iHappyFromHurry > iBestHappy)
						{
							iBestHappy = iHappyFromHurry;
						}
					}
				}
				iValue += iBestHappy * 20;

				if (kBuilding.isNoUnhappiness())
				{
					iValue += ((iAngryPopulation + getPopulation()) * 10);
				}

				iValue += (min(kBuilding.getHappiness(), iAngryPopulation) * 10);
				iValue += (kBuilding.getAreaHappiness() * area()->getCitiesPerPlayer(getOwnerINLINE()) * 8);
				iValue += (kBuilding.getGlobalHappiness() * iNumCities * 8);

				iValue += (min(-(((kBuilding.getWarWearinessModifier() * kOwner.getWarWearinessPercentAnger()) / 100) / GC.getDefineINT("PERCENT_ANGER_DIVISOR")), iAngryPopulation) * 8);
				iValue += (-(((kBuilding.getGlobalWarWearinessModifier() * kOwner.getWarWearinessPercentAnger() / 100) / GC.getDefineINT("PERCENT_ANGER_DIVISOR"))) * iNumCities);
				iValue -= (kBuilding.getHurryAngerModifier() * getHurryPercentAnger()) / 100;

				if (kBuilding.getReligionType() == eStateReligion)
				{
					iValue += (min(kBuilding.getStateReligionHappiness(), iAngryPopulation) * 8);
				}

				for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
				{
					iValue += (min(((kBuilding.getCommerceHappiness(iI) * kOwner.getCommercePercent((CommerceTypes)iI)) / 100), iAngryPopulation) * 8);
				}

				for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
				{
					if (hasBonus((BonusTypes)iI))
					{
						iValue += (min(kBuilding.getBonusHappinessChanges(iI), iAngryPopulation) * 8);
					}
				}

				for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
				{
					if (kBuilding.getBuildingHappinessChanges(iI) != 0)
					{
						iValue += (kBuilding.getBuildingHappinessChanges(iI) * kOwner.countNumBuildings((BuildingTypes)iI) * 8);
					}
				}
			}

			if ((iFocusFlags & BUILDINGFOCUS_HEALTHY) || (iPass > 0))
			{
				if (bProvidesPower)
				{
					if (isDirtyPower() && !(kBuilding.isDirtyPower()))
					{
						iValue += (min(-(GC.getDefineINT("DIRTY_POWER_HEALTH_CHANGE")), iBadHealth) * 8);
					}
				}

				if (kBuilding.isNoUnhealthyPopulation())
				{
					iValue += (min(unhealthyPopulation(), iBadHealth) * 6);
				}

				if (kBuilding.isBuildingOnlyHealthy())
				{
					iValue += (min(-(getBuildingBadHealth()), iBadHealth) * 6);
				}

				iValue += (min(kBuilding.getHealth(), iBadHealth) * 8);
				iValue += (kBuilding.getAreaHealth() * area()->getCitiesPerPlayer(getOwnerINLINE()) * 6);
				iValue += (kBuilding.getGlobalHealth() * iNumCities * 6);

				for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
				{
					if (hasBonus((BonusTypes)iI))
					{
						iValue += (min(kBuilding.getBonusHealthChanges(iI), iBadHealth) * 6);
					}
				}
			}

			if ((iFocusFlags & BUILDINGFOCUS_EXPERIENCE) || (iPass > 0))
			{
				iValue += (kBuilding.getFreeExperience() * ((iHasMetCount > 0) ? 10 : 5));

				for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
				{
					if (canTrain((UnitCombatTypes)iI))
					{
						iValue += (kBuilding.getUnitCombatFreeExperience(iI) * ((iHasMetCount > 0) ? 6 : 3));
					}
				}

				for (iI = 0; iI < NUM_DOMAIN_TYPES; iI++)
				{
					iValue += (kBuilding.getDomainFreeExperience(iI) * ((iHasMetCount > 0) ? 8 : 4));
				}
			}

			if ((iFocusFlags & BUILDINGFOCUS_MAINTENANCE) || (iPass > 0))
			{
				iValue += ((iBaseMaintenance - ((iBaseMaintenance * max(0, (kBuilding.getMaintenanceModifier() + 100))) / 100)) * 8);
			}

			if ((iFocusFlags & BUILDINGFOCUS_SPECIALIST) || (iPass > 0))
			{
				for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
				{
					if (kBuilding.getSpecialistCount(iI) > 0)
					{
						iTempValue = AI_specialistValue(((SpecialistTypes)iI), false, false);

						iTempValue *= (50 + (10 * kBuilding.getSpecialistCount(iI)));
						iTempValue /= 100;

						iValue += (iTempValue / 100);
					}
				}
			}

			if (iPass > 0)
			{
				if (kBuilding.isAreaCleanPower())
				{
					iValue += (area()->getCitiesPerPlayer(getOwnerINLINE()) * 10); // XXX count cities without clean power???
				}

				if (kBuilding.getDomesticGreatGeneralRateModifier() != 0)
				{
					iValue += (kBuilding.getDomesticGreatGeneralRateModifier() / 10);
				}

				if (kBuilding.isAreaBorderObstacle())
				{
					iValue += (area()->getCitiesPerPlayer(getOwnerINLINE()));
				}

				if (kBuilding.isGovernmentCenter())
				{
					FAssert(!(kBuilding.isCapital()));
					iValue += (calculateDistanceMaintenance() * area()->getCitiesPerPlayer(getOwnerINLINE()));
				}

				if (kBuilding.isMapCentering())
				{
					iValue++;
				}

				if (kBuilding.getFreeBonus() != NO_BONUS)
				{
					iValue += (kOwner.AI_bonusVal((BonusTypes)(kBuilding.getFreeBonus())) *
						         ((kOwner.getNumTradeableBonuses((BonusTypes)(kBuilding.getFreeBonus())) == 0) ? 2 : 1) *
						         (iNumCities + kBuilding.getNumFreeBonuses()));
				}

				if (kBuilding.getFreePromotion() != NO_PROMOTION)
				{
					iValue += ((iHasMetCount > 0) ? 10 : 5); // XXX some sort of promotion value???
				}

				if (kBuilding.getCivicOption() != NO_CIVICOPTION)
				{
					for (iI = 0; iI < GC.getNumCivicInfos(); iI++)
					{
						if (GC.getCivicInfo((CivicTypes)iI).getCivicOptionType() == kBuilding.getCivicOption())
						{
							if (!(kOwner.canDoCivics((CivicTypes)iI)))
							{
								iValue += (kOwner.AI_civicValue((CivicTypes)iI) / 10);
							}
						}
					}
				}

				iValue += ((kBuilding.getGreatPeopleRateModifier() * getBaseGreatPeopleRate()) / 16);
				iValue += ((kBuilding.getGlobalGreatPeopleRateModifier() * iNumCities) / 8);

				iValue += (-(kBuilding.getAnarchyModifier()) / 4);

				iValue += (-(kBuilding.getGlobalHurryModifier()) * 2);

				iValue += (kBuilding.getGlobalFreeExperience() * iNumCities * ((iHasMetCount > 0) ? 6 : 3));

				if (bCanPopRush)
				{
					iValue += kBuilding.getFoodKept() / 10;
				}

				iValue += (kBuilding.getAirlift() * getPopulation() * 4);

				iValue += (-(kBuilding.getAirModifier()) / ((iHasMetCount > 0) ? 9 : 18));

				iValue += (-(kBuilding.getNukeModifier()) / ((iHasMetCount > 0) ? 10 : 20));

				iValue += (kBuilding.getFreeSpecialist() * 16);
				iValue += (kBuilding.getAreaFreeSpecialist() * area()->getCitiesPerPlayer(getOwnerINLINE()) * 12);
				iValue += (kBuilding.getGlobalFreeSpecialist() * iNumCities * 12);

				iValue += ((kBuilding.getWorkerSpeedModifier() * kOwner.AI_getNumAIUnits(UNITAI_WORKER)) / 10);

				if (iHasMetCount > 0)
				{
					iValue += (kBuilding.getMilitaryProductionModifier() / 3);

					if (iProductionRank <= max(3, (GET_PLAYER(getOwnerINLINE()).getNumCities() / 2)))
					{
						iValue += kBuilding.getMilitaryProductionModifier() / 3;
					}
				}

				iValue += (kBuilding.getSpaceProductionModifier() / 5);
				iValue += ((kBuilding.getGlobalSpaceProductionModifier() * iNumCities) / 20);

				iValue += (kBuilding.getTradeRoutes() * ((10 * max(0, (totalTradeModifier() + 100))) / 100));
				iValue += (kBuilding.getCoastalTradeRoutes() * kOwner.countNumCoastalCities() * 8);
				iValue += (kBuilding.getGlobalTradeRoutes() * iNumCities * 8);

				if (kBuilding.getGreatPeopleUnitClass() != NO_UNITCLASS)
				{
					iValue++; // XXX improve this for diversity...
				}

				iValue += (kBuilding.getGreatPeopleRateChange() * 20);

				if (!bAreaAlone)
				{
					iValue += (kBuilding.getHealRateChange() / 2);
				}

				iValue += (kBuilding.getGlobalPopulationChange() * iNumCities * 4);

				iValue += (kBuilding.getFreeTechs() * 80);

				for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
				{
					if (kBuilding.getFreeSpecialistCount(iI) > 0)
					{
						iValue += ((AI_specialistValue(((SpecialistTypes)iI), false, false) * kBuilding.getFreeSpecialistCount(iI)) / 50);
					}
				}

				for (iI = 0; iI < NUM_DOMAIN_TYPES; iI++)
				{
					iValue += (kBuilding.getDomainProductionModifier(iI) / 5);
					if (iProductionRank <= max(3, (GET_PLAYER(getOwnerINLINE()).getNumCities() / 2)))
					{
						iValue += (kBuilding.getDomainProductionModifier(iI) / 5);
					}
				}

				for (iI = 0; iI < GC.getNumUnitInfos(); iI++)
				{
					if (GC.getUnitInfo((UnitTypes)iI).getPrereqBuilding() == eBuilding)
					{
						if (kOwner.AI_totalAreaUnitAIs(area(), ((UnitAITypes)(GC.getUnitInfo((UnitTypes)iI).getDefaultUnitAIType()))) == 0)
						{
							iValue += area()->getCitiesPerPlayer(getOwnerINLINE());
						}

						iValue++;
					}
				}

				for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
				{
					if (kOwner.getBuildingClassPrereqBuilding(((BuildingTypes)iI), ((BuildingClassTypes)(kBuilding.getBuildingClassType()))) <= iNumCities)
					{
						if (kOwner.getBuildingClassPrereqBuilding(((BuildingTypes)iI), ((BuildingClassTypes)(kBuilding.getBuildingClassType()))) > kOwner.getBuildingClassCountPlusMaking((BuildingClassTypes)(kBuilding.getBuildingClassType())))
						{
							iValue += (iNumCities * 3);
						}
					}
				}
			}

			if (iPass > 0)
			{
				for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
				{
					iTempValue = 0;

					iValue += ((kBuilding.getTradeRouteModifier() * getTradeYield((YieldTypes)iI)) / 12);

					if (kBuilding.getSeaPlotYieldChange(iI) > 0)
					{
						iTempValue += (kBuilding.getSeaPlotYieldChange(iI) * countNumWaterPlots() * 4);
					}
					iTempValue += (kBuilding.getGlobalSeaPlotYieldChange(iI) * kOwner.countNumCoastalCities() * 8);
					iTempValue += (kBuilding.getYieldChange(iI) * 6);
					iTempValue += ((kBuilding.getYieldModifier(iI) * getBaseYieldRate((YieldTypes)iI)) / 10);
					iTempValue += ((kBuilding.getPowerYieldModifier(iI) * getBaseYieldRate((YieldTypes)iI)) / ((bProvidesPower || isPower()) ? 12 : 15));
					iTempValue += ((kBuilding.getAreaYieldModifier(iI) * area()->getCitiesPerPlayer(getOwnerINLINE())) / 3);
					iTempValue += ((kBuilding.getGlobalYieldModifier(iI) * iNumCities) / 3);

					if (bProvidesPower && !isPower())
					{
						iTempValue += ((getPowerYieldRateModifier((YieldTypes)iI) * getBaseYieldRate((YieldTypes)iI)) / 12);
					}

					for (iJ = 0; iJ < GC.getNumSpecialistInfos(); iJ++)
					{
						iTempValue += ((kBuilding.getSpecialistYieldChange(iJ, iI) * kOwner.getTotalPopulation()) / 5);
					}

					for (iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
					{
						if (hasBonus((BonusTypes)iJ))
						{
							iTempValue += ((kBuilding.getBonusYieldModifier(iJ, iI) * getBaseYieldRate((YieldTypes)iI)) / 12);
						}
					}

					iTempValue *= kOwner.AI_yieldWeight((YieldTypes)iI);
					iTempValue /= 100;

					iValue += iTempValue;
				}
			}
			else
			{
				if (iFoodDifference > 1)
				{
					iValue += ((kBuilding.getFoodKept() * iFoodDifference) / 25);
				}

				if (iFocusFlags & BUILDINGFOCUS_FOOD)
				{
					if (kBuilding.getSeaPlotYieldChange(YIELD_FOOD) > 0)
					{
						iValue += (kBuilding.getSeaPlotYieldChange(YIELD_FOOD) * countNumWaterPlots() * 4);
					}
				}

				if (iFocusFlags & BUILDINGFOCUS_PRODUCTION)
				{
					iValue += ((kBuilding.getYieldModifier(YIELD_PRODUCTION) * getBaseYieldRate(YIELD_PRODUCTION)) / 10);
					iValue += ((kBuilding.getPowerYieldModifier(YIELD_PRODUCTION) * getBaseYieldRate(YIELD_PRODUCTION)) / ((bProvidesPower || isPower()) ? 12 : 15));

					if (bProvidesPower && !isPower())
					{
						iValue += ((getPowerYieldRateModifier(YIELD_PRODUCTION) * getBaseYieldRate(YIELD_PRODUCTION)) / 12);
					}
				}
			}

			if (iPass > 0)
			{
				for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
				{
					iTempValue = 0;

					iTempValue += (kBuilding.getCommerceChange(iI) * 4);
					iTempValue += (kBuilding.getObsoleteSafeCommerceChange(iI) * 4);

					if (kBuilding.getCommerceChangeDoubleTime(iI) > 0)
					{
						if ((kBuilding.getCommerceChange(iI) > 0) || (kBuilding.getObsoleteSafeCommerceChange(iI) > 0))
						{
							iTempValue += (1000 / kBuilding.getCommerceChangeDoubleTime(iI));
						}
					}

					iTempValue += ((kBuilding.getCommerceModifier(iI) * getBaseCommerceRate((CommerceTypes)iI)) / 15);
					iTempValue += ((kBuilding.getGlobalCommerceModifier(iI) * iNumCities) / 4);
					iTempValue += ((kBuilding.getSpecialistExtraCommerce(iI) * kOwner.getTotalPopulation()) / 3);

					if (eStateReligion != NO_RELIGION)
					{
						iTempValue += (kBuilding.getStateReligionCommerce(iI) * kOwner.getHasReligionCount(eStateReligion) * 3);
					}

					if (kBuilding.getGlobalReligionCommerce() != NO_RELIGION)
					{
						iTempValue += (GC.getReligionInfo((ReligionTypes)(kBuilding.getGlobalReligionCommerce())).getGlobalReligionCommerce(iI) * GC.getGameINLINE().countReligionLevels((ReligionTypes)(kBuilding.getGlobalReligionCommerce())) * 2);
					}

					if (kBuilding.isCommerceFlexible(iI))
					{
						if (!(kOwner.isCommerceFlexible((CommerceTypes)iI)))
						{
							iTempValue += 40;
						}
					}

					if (kBuilding.isCommerceChangeOriginalOwner(iI))
					{
						if ((kBuilding.getCommerceChange(iI) > 0) || (kBuilding.getObsoleteSafeCommerceChange(iI) > 0))
						{
							iTempValue++;
						}
					}

					iTempValue *= kOwner.AI_commerceWeight(((CommerceTypes)iI), this);
					iTempValue /= 100;

					iValue += iTempValue;
				}

				for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
				{
					if (kBuilding.getReligionChange(iI) > 0)
					{
						if (GET_TEAM(getTeam()).hasHolyCity((ReligionTypes)iI))
						{
							iValue += (kBuilding.getReligionChange(iI) * ((eStateReligion == iI) ? 10 : 1));
						}
					}
				}
			}
			else
			{
				if (iFocusFlags & BUILDINGFOCUS_GOLD)
				{
					iValue += ((kBuilding.getCommerceModifier(COMMERCE_GOLD) * getBaseCommerceRate(COMMERCE_GOLD)) / 15);
				}

				if (iFocusFlags & BUILDINGFOCUS_RESEARCH)
				{
					iValue += ((kBuilding.getCommerceModifier(COMMERCE_RESEARCH) * getBaseCommerceRate(COMMERCE_RESEARCH)) / 15);
				}

				if (iFocusFlags & BUILDINGFOCUS_CULTURE)
				{
					iValue += (kBuilding.getCommerceChange(COMMERCE_CULTURE) * 4);
					iValue += (kBuilding.getObsoleteSafeCommerceChange(COMMERCE_CULTURE) * 4);
				}
			}

			if (iPass > 0)
			{
				iValue += kBuilding.getAIWeight();

				if (iValue > 0)
				{
					if (!isHuman())
					{
						for (iI = 0; iI < GC.getNumFlavorTypes(); iI++)
						{
							iValue += (kOwner.AI_getFlavorValue((FlavorTypes)iI) * kBuilding.getFlavorValue(iI));
						}
					}
				}
			}
		}
	}

	if (kBuilding.getObsoleteTech() != NO_TECH)
	{
		iValue++;
		iValue /= 2;
	}

	if (kBuilding.getSpecialBuildingType() != NO_SPECIALBUILDING)
	{
		if (GC.getSpecialBuildingInfo((SpecialBuildingTypes)(kBuilding.getSpecialBuildingType())).getObsoleteTech() != NO_TECH)
		{
			iValue++;
			iValue /= 2;
		}
	}

	return max(0, iValue);
}


ProjectTypes CvCityAI::AI_bestProject()
{
	ProjectTypes eBestProject;
	int iProductionRank;
	int iTurnsLeft;
	int iValue;
	int iBestValue;
	int iI;

	iProductionRank = findYieldRateRank(YIELD_PRODUCTION);

	iBestValue = 0;
	eBestProject = NO_PROJECT;

	for (iI = 0; iI < GC.getNumProjectInfos(); iI++)
	{
		if (canCreate((ProjectTypes)iI))
		{
			iValue = AI_projectValue((ProjectTypes)iI);

			if ((GC.getProjectInfo((ProjectTypes)iI).getEveryoneSpecialUnit() != NO_SPECIALUNIT) ||
				  (GC.getProjectInfo((ProjectTypes)iI).getEveryoneSpecialBuilding() != NO_SPECIALBUILDING))
			{
				if (GC.getGameINLINE().getSorenRandNum(100, "Project Everyone") == 0)
				{
					iValue++;
				}
			}

			if (iValue > 0)
			{
				iValue += getProjectProduction((ProjectTypes)iI);

				iTurnsLeft = getProductionTurnsLeft(((ProjectTypes)iI), 0);

				if ((iTurnsLeft <= GC.getGameINLINE().AI_turnsPercent(10, GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getCreatePercent())) || !(GET_TEAM(getTeam()).isHuman()))
				{
					if ((iTurnsLeft <= GC.getGameINLINE().AI_turnsPercent(20, GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getCreatePercent())) || (iProductionRank <= max(3, (GET_PLAYER(getOwnerINLINE()).getNumCities() / 2))))
					{
						if (iProductionRank == 1)
						{
							iValue += iTurnsLeft;
						}
						else
						{
							FAssert((MAX_INT / 1000) > iValue);
							iValue *= 1000;
							iValue /= max(1, (iTurnsLeft + 10));
						}

						iValue = max(1, iValue);

						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							eBestProject = ((ProjectTypes)iI);
						}
					}
				}
			}
		}
	}

	return eBestProject;
}


int CvCityAI::AI_projectValue(ProjectTypes eProject)
{
	int iValue;
	int iI;

	iValue = 0;

	if (GC.getProjectInfo(eProject).getNukeInterception() > 0)
	{
		if (GC.getGameINLINE().canTrainNukes())
		{
			iValue += (GC.getProjectInfo(eProject).getNukeInterception() / 10);
		}
	}

	if (GC.getProjectInfo(eProject).getTechShare() > 0)
	{
		if (GC.getProjectInfo(eProject).getTechShare() >= GET_TEAM(getTeam()).getHasMetCivCount(true))
		{
			iValue += (20 / GC.getProjectInfo(eProject).getTechShare());
		}
	}

	for (iI = 0; iI < GC.getNumVictoryInfos(); iI++)
	{
		if (GC.getGameINLINE().isVictoryValid((VictoryTypes)iI))
		{
			iValue += (max(0, (GC.getProjectInfo(eProject).getVictoryThreshold(iI) - GET_TEAM(getTeam()).getProjectCount(eProject))) * 20);
		}
	}

	for (iI = 0; iI < GC.getNumProjectInfos(); iI++)
	{
		iValue += (max(0, (GC.getProjectInfo((ProjectTypes)iI).getProjectsNeeded(eProject) - GET_TEAM(getTeam()).getProjectCount(eProject))) * 10);
	}

	return iValue;
}


ProcessTypes CvCityAI::AI_bestProcess()
{
	ProcessTypes eBestProcess;
	int iValue;
	int iBestValue;
	int iI;

	iBestValue = 0;
	eBestProcess = NO_PROCESS;

	for (iI = 0; iI < GC.getNumProcessInfos(); iI++)
	{
		if (canMaintain((ProcessTypes)iI))
		{
			iValue = AI_processValue((ProcessTypes)iI);

			if (iValue > iBestValue)
			{
				iBestValue = iValue;
				eBestProcess = ((ProcessTypes)iI);
			}
		}
	}

	return eBestProcess;
}


int CvCityAI::AI_processValue(ProcessTypes eProcess)
{
	int iValue;
	int iTempValue;
	int iI;

	iValue = 0;

	if (GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble())
	{
		iValue += GC.getProcessInfo(eProcess).getProductionToCommerceModifier(COMMERCE_GOLD);
	}

	if (plot()->calculateCulturePercent(getOwnerINLINE()) < 50)
	{
		iValue += GC.getProcessInfo(eProcess).getProductionToCommerceModifier(COMMERCE_CULTURE);
	}

	for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		iTempValue = GC.getProcessInfo(eProcess).getProductionToCommerceModifier(iI);

		iTempValue *= GET_PLAYER(getOwnerINLINE()).AI_commerceWeight(((CommerceTypes)iI), this);
		iTempValue /= 100;

		iValue += iTempValue;
	}

	return iValue;
}


int CvCityAI::AI_neededSeaWorkers()
{
	CvArea* pWaterArea;

	pWaterArea = waterArea();

	if (pWaterArea != NULL)
	{
		return GET_PLAYER(getOwnerINLINE()).countUnimprovedBonuses(pWaterArea, plot());
	}

	return 0;
}


bool CvCityAI::AI_isDefended(int iExtra)
{
	PROFILE_FUNC();

	return ((plot()->plotCount(PUF_canDefendGroupHead, -1, -1, getOwnerINLINE(), NO_TEAM, PUF_isCityAIType) + iExtra) >= AI_neededDefenders()); // XXX check for other team's units?
}


bool CvCityAI::AI_isAirDefended(int iExtra)
{
	PROFILE_FUNC();

	return ((plot()->plotCount(PUF_canAirDefend, -1, -1, getOwnerINLINE(), NO_TEAM, PUF_isDomainType, DOMAIN_AIR) + iExtra) >= AI_neededAirDefenders()); // XXX check for other team's units?
}


int CvCityAI::AI_neededDefenders()
{
	int iDefenders;

	if (!(GET_TEAM(getTeam()).AI_isWarPossible()))
	{
		return 1;
	}

	iDefenders = 1;

	iDefenders += max(0, ((getPopulation() - 2) / 7));

	if (isCapital() && (GET_PLAYER(getOwnerINLINE()).getNumCities() > 2))
	{
		iDefenders++;
	}

	if (GET_TEAM(getTeam()).hasMetHuman())
	{
		iDefenders++;
	}

	if (getGameTurnAcquired() != getGameTurnFounded())
	{
		if ((getGameTurnAcquired() + 10) > GC.getGameINLINE().getGameTurn())
		{
			iDefenders++;
		}
	}

	if (GC.getGameINLINE().getMaxCityElimination() > 0)
	{
		iDefenders++;
	}

	return iDefenders;
}


int CvCityAI::AI_neededAirDefenders()
{
	int iDefenders;

	if (!(GET_TEAM(getTeam()).AI_isWarPossible()))
	{
		return 0;
	}

	iDefenders = 1;

	iDefenders += max(0, ((getPopulation() - 3) / 11));

	if (isCapital() && (GET_PLAYER(getOwnerINLINE()).getNumCities() > 1))
	{
		iDefenders++;
	}

	if (GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot()) > 0)
	{
		iDefenders += 2;
	}

	return iDefenders;
}


bool CvCityAI::AI_isDanger()
{
	return GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot(), 2, false);
}


int CvCityAI::AI_getEmphasizeAvoidGrowthCount()
{
	return m_iEmphasizeAvoidGrowthCount;
}


bool CvCityAI::AI_isEmphasizeAvoidGrowth()
{
	return (AI_getEmphasizeAvoidGrowthCount() > 0);
}


int CvCityAI::AI_getEmphasizeGreatPeopleCount()
{
	return m_iEmphasizeGreatPeopleCount;
}


bool CvCityAI::AI_isEmphasizeGreatPeople()
{
	return (AI_getEmphasizeGreatPeopleCount() > 0);
}


bool CvCityAI::AI_isAssignWorkDirty()
{
	return m_bAssignWorkDirty;
}


void CvCityAI::AI_setAssignWorkDirty(bool bNewValue)
{
	m_bAssignWorkDirty = bNewValue;
}


bool CvCityAI::AI_isChooseProductionDirty()
{
	return m_bChooseProductionDirty;
}


void CvCityAI::AI_setChooseProductionDirty(bool bNewValue)
{
	m_bChooseProductionDirty = bNewValue;
}


CvCity* CvCityAI::AI_getRouteToCity() const
{
	return getCity(m_routeToCity);
}


void CvCityAI::AI_updateRouteToCity()
{
	CvCity* pLoopCity;
	CvCity* pBestCity;
	int iValue;
	int iBestValue;
	int iLoop;
	int iI;

	gDLL->getFAStarIFace()->ForceReset(&GC.getRouteFinder());

	iBestValue = MAX_INT;
	pBestCity = NULL;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).getTeam() == getTeam())
		{
			for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
			{
				if (pLoopCity != this)
				{
					if (pLoopCity->area() == area())
					{
						if (!(gDLL->getFAStarIFace()->GeneratePath(&GC.getRouteFinder(), getX_INLINE(), getY_INLINE(), pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE(), false, getOwnerINLINE(), true)))
						{
							iValue = plotDistance(getX_INLINE(), getY_INLINE(), pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE());

							if (iValue < iBestValue)
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
		m_routeToCity = pBestCity->getIDInfo();
	}
	else
	{
		m_routeToCity.reset();
	}
}


int CvCityAI::AI_getEmphasizeYieldCount(YieldTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_YIELD_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiEmphasizeYieldCount[eIndex];
}


bool CvCityAI::AI_isEmphasizeYield(YieldTypes eIndex)
{
	return (AI_getEmphasizeYieldCount(eIndex) > 0);
}


int CvCityAI::AI_getEmphasizeCommerceCount(CommerceTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_COMMERCE_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
	return (m_aiEmphasizeCommerceCount[eIndex] > 0);
}


bool CvCityAI::AI_isEmphasizeCommerce(CommerceTypes eIndex)
{
	return (AI_getEmphasizeCommerceCount(eIndex) > 0);
}


bool CvCityAI::AI_isEmphasize(EmphasizeTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumEmphasizeInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	FAssertMsg(m_pbEmphasize != NULL, "m_pbEmphasize is not expected to be equal with NULL");
	return m_pbEmphasize[eIndex];
}


void CvCityAI::AI_setEmphasize(EmphasizeTypes eIndex, bool bNewValue)
{
	int iI;

	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumEmphasizeInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (AI_isEmphasize(eIndex) != bNewValue)
	{
		m_pbEmphasize[eIndex] = bNewValue;

		if (GC.getEmphasizeInfo(eIndex).isAvoidGrowth())
		{
			m_iEmphasizeAvoidGrowthCount += ((AI_isEmphasize(eIndex)) ? 1 : -1);
			FAssert(AI_getEmphasizeAvoidGrowthCount() >= 0);
		}

		if (GC.getEmphasizeInfo(eIndex).isGreatPeople())
		{
			m_iEmphasizeGreatPeopleCount += ((AI_isEmphasize(eIndex)) ? 1 : -1);
			FAssert(AI_getEmphasizeGreatPeopleCount() >= 0);
		}

		for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
		{
			if (GC.getEmphasizeInfo(eIndex).getYieldChange(iI))
			{
				m_aiEmphasizeYieldCount[iI] += ((AI_isEmphasize(eIndex)) ? 1 : -1);
				FAssert(AI_getEmphasizeYieldCount((YieldTypes)iI) >= 0);
			}
		}

		for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
		{
			if (GC.getEmphasizeInfo(eIndex).getCommerceChange(iI))
			{
				m_aiEmphasizeCommerceCount[iI] += ((AI_isEmphasize(eIndex)) ? 1 : -1);
				FAssert(AI_getEmphasizeCommerceCount((CommerceTypes)iI) >= 0);
			}
		}

		AI_assignWorkingPlots();

		if ((getOwnerINLINE() == GC.getGameINLINE().getActivePlayer()) && isCitySelected())
		{
			gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
		}
	}
}


int CvCityAI::AI_getBestBuildValue(int iIndex)
{
	FAssertMsg(iIndex >= 0, "iIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(iIndex < NUM_CITY_PLOTS, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiBestBuildValue[iIndex];
}


int CvCityAI::AI_totalBestBuildValue(CvArea* pArea)
{
	CvPlot* pLoopPlot;
	int iTotalValue;
	int iI;

	iTotalValue = 0;

	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		if (iI != CITY_HOME_PLOT)
		{
			pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);

			if (pLoopPlot != NULL)
			{
				if (pLoopPlot->area() == pArea)
				{
					if ((pLoopPlot->getImprovementType() == NO_IMPROVEMENT) || !(GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_SAFE_AUTOMATION)))
					{
						iTotalValue += AI_getBestBuildValue(iI);
					}
				}
			}
		}
	}

	return iTotalValue;
}


BuildTypes CvCityAI::AI_getBestBuild(int iIndex)
{
	FAssertMsg(iIndex >= 0, "iIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(iIndex < NUM_CITY_PLOTS, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aeBestBuild[iIndex];
}


int CvCityAI::AI_countBestBuilds(CvArea* pArea)
{
	CvPlot* pLoopPlot;
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		if (iI != CITY_HOME_PLOT)
		{
			pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);

			if (pLoopPlot != NULL)
			{
				if (pLoopPlot->area() == pArea)
				{
					if (AI_getBestBuild(iI) != NO_BUILD)
					{
						iCount++;
					}
				}
			}
		}
	}

	return iCount;
}


// Improved worker AI provided by Blake - thank you!
void CvCityAI::AI_updateBestBuild()
{
	CvPlot* pLoopPlot;
	int iI, iJ;
    int aiFinalYields[NUM_YIELD_TYPES];


	int iBonusFoodSurplus = 0;
	int iBonusFoodDeficit = 0;
	int iFeatureFoodSurplus = 0;
	int iHillFoodDeficit = 0;
	int iFoodTotal = GC.getYieldInfo(YIELD_FOOD).getMinCity();
	BonusTypes eBonus;
	int iFoodPriority = 0;
	int iCommercePriority = 0;
	int iProductionPriority = 0;



	int iGoodTileCount = 0;

	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		if (iI != CITY_HOME_PLOT)
		{
			pLoopPlot = getCityIndexPlot(iI);

			if (NULL != pLoopPlot && pLoopPlot->getWorkingCity() == this)
			{
				for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
				{
					//by default we'll use the current value
					aiFinalYields[iJ] = pLoopPlot->getYield((YieldTypes)iJ);

					//determine if the tile is being improved.
					if (m_aiBestBuildValue[iI] > 0)
					{
						//check if the tile is being improved
						if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_BUILD) > 0)
						{
							//we can't really assume this.. but lets assume the BestBuild is what is being built
							BuildTypes eBuild = m_aeBestBuild[iI];
							ImprovementTypes eImprovement = (ImprovementTypes)GC.getBuildInfo(eBuild).getImprovement();
							if (eImprovement != NO_IMPROVEMENT)
							{
								bool bIgnoreFeature = false;
								if (pLoopPlot->getFeatureType() != NO_FEATURE)
								{
									if (GC.getBuildInfo(eBuild).isFeatureRemove(pLoopPlot->getFeatureType()))
									{
										bIgnoreFeature = true;
									}
								}

								aiFinalYields[iJ] = (pLoopPlot->calculateNatureYield(((YieldTypes)iJ), getTeam(), bIgnoreFeature) + pLoopPlot->calculateImprovementYieldChange(eImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false));
							}
						}
					}
				}

				iFoodTotal += ((aiFinalYields[YIELD_FOOD] >= GC.getFOOD_CONSUMPTION_PER_POPULATION()) ? aiFinalYields[YIELD_FOOD] : 0);

				eBonus = pLoopPlot->getBonusType(getTeam());

				if (((aiFinalYields[YIELD_FOOD]*10) + (aiFinalYields[YIELD_PRODUCTION]*6) + (aiFinalYields[YIELD_COMMERCE]*4)) > 21)
				{
					iGoodTileCount++;
				}

				if (eBonus != NO_BONUS)
				{
					if (pLoopPlot->getImprovementType() != NO_IMPROVEMENT)
					{
						int iNetFood = ((pLoopPlot->getYield(YIELD_FOOD)) - GC.getFOOD_CONSUMPTION_PER_POPULATION());
						iBonusFoodSurplus += max(0, iNetFood);
						iBonusFoodDeficit += max(0, -iNetFood);
					}
				}
				if ((pLoopPlot->getFeatureType()) != NO_FEATURE)
				{
					iFeatureFoodSurplus += max (0, pLoopPlot->calculateNatureYield(YIELD_FOOD, getTeam()) - GC.getFOOD_CONSUMPTION_PER_POPULATION());
				}
				if ((pLoopPlot->isHills()))
				{
					iHillFoodDeficit += max (0, GC.getFOOD_CONSUMPTION_PER_POPULATION() - pLoopPlot->calculateNatureYield(YIELD_FOOD, getTeam()));
				}
			}
		}
	}

	int iFoodEaten = getPopulation() * GC.getFOOD_CONSUMPTION_PER_POPULATION();
	int iBonusFoodDiff = (iBonusFoodSurplus + iFeatureFoodSurplus - iBonusFoodDeficit);

	if ((iBonusFoodDiff < 3) || (iFoodTotal < (2 + iFoodEaten)))
	{
		iFoodPriority = min(1, 1 - (iBonusFoodDiff / 3));
	}

	if ((iFoodTotal < (iBonusFoodDeficit + iFoodEaten + (3 + getPopulation() / 6))) && (iGoodTileCount < getPopulation()))
	{
		iFoodPriority += 1;
	}

	if ((iBonusFoodSurplus + iFeatureFoodSurplus > 5) && ((iBonusFoodDeficit + iHillFoodDeficit) > 5))
	{
		if ((iBonusFoodDeficit + iHillFoodDeficit) > 8)
		{
			//probably a good candidate for a wonder pump
			iFoodPriority = 1;
			iProductionPriority = 1;
			iCommercePriority = -1;
		}
	}

	if (((iBonusFoodSurplus + iFeatureFoodSurplus) > 1) && ((iBonusFoodDeficit + iHillFoodDeficit) < 5))
	{
		iCommercePriority = 1;
	}

	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		m_aiBestBuildValue[iI] = 0;
		m_aeBestBuild[iI] = NO_BUILD;

		if (iI != CITY_HOME_PLOT)
		{
			pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);

			if (NULL != pLoopPlot && pLoopPlot->getWorkingCity() == this)
			{
				AI_bestPlotBuild(pLoopPlot, &(m_aiBestBuildValue[iI]), &(m_aeBestBuild[iI]), iFoodPriority, iProductionPriority, iCommercePriority);
				FAssert(AI_getBestBuildValue(iI) >= 0);
			}
		}
	}
}

// Protected Functions...

// Better drafting strategy by Blake - thank you!
void CvCityAI::AI_doDraft()
{
	PROFILE_FUNC();

	FAssert(!isHuman());

	if (canConscript())
	{
		bool bLandWar = ((area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE) || (area()->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE) || (area()->getAreaAIType(getTeam()) == AREAAI_MASSING));
		bool bDanger = (!AI_isDefended() && AI_isDanger());
		int iConscriptPop = getConscriptPopulation();
		int iHappyDiff = GC.getDefineINT("CONSCRIPT_POP_ANGER") - iConscriptPop;

		if (bLandWar && 0 == angryPopulation(iHappyDiff))
		{
			bool bWait = false;

			if (!bDanger)
			{
				if (!bWait)
				{
					if (getConscriptAngerTimer() > 0)
					{
						bWait = true;
					}
				}

				if (!bWait)
				{
					if (3 * (getPopulation() - iConscriptPop) < getHighestPopulation() * 2)
					{
						bWait = true;
					}
				}
			}

			if (!bWait)
			{
				conscript();
			}
		}
	}
}

// Better pop-rushing strategy by Blake - thank you!
void CvCityAI::AI_doHurry()
{
	PROFILE_FUNC();

	CvArea* pWaterArea;
	UnitTypes eProductionUnit;
	UnitAITypes eProductionUnitAI;
	BuildingTypes eProductionBuilding;
	int iHurryAngerLength;
	int iHurryPopulation;
	int iMinTurns;
	bool bDanger;
	bool bWait;
	bool bEssential;
	int iI, iJ;

	FAssert(!isHuman());

	if (getProduction() == 0)
	{
		return;
	}

	pWaterArea = waterArea();

	eProductionUnit = getProductionUnit();
	eProductionUnitAI = getProductionUnitAI();
	eProductionBuilding = getProductionBuilding();

	bDanger = AI_isDanger();

	for (iI = 0; iI < GC.getNumHurryInfos(); iI++)
	{
		if (canHurry((HurryTypes)iI))
		{
			iHurryAngerLength = hurryAngerLength((HurryTypes)iI);
			iHurryPopulation = hurryPopulation((HurryTypes)iI);

			iMinTurns = MAX_INT;
			bEssential = false;

			// Whip to eliminate unhappiness - thank you Blake!
			if (getProduction() > 0)
			{
				if (AI_getHappyFromHurry((HurryTypes)iI) > 0)
				{
					hurry((HurryTypes)iI);
					break;
				}
			}

			if ((iHurryAngerLength == 0) && (iHurryPopulation == 0))
			{
				if (GET_PLAYER(getOwnerINLINE()).AI_avoidScience())
				{
					if (GET_PLAYER(getOwnerINLINE()).getGold() > GET_PLAYER(getOwnerINLINE()).AI_goldTarget())
					{
						iMinTurns = min(iMinTurns, 10);
					}
				}
			}

			if (eProductionBuilding != NO_BUILDING)
			{
				if (isWorldWonderClass((BuildingClassTypes)(GC.getBuildingInfo(eProductionBuilding).getBuildingClassType())))
				{
					iMinTurns = min(iMinTurns, 10);
				}
			}

			if (eProductionBuilding != NO_BUILDING)
			{
				if (GC.getBuildingInfo(eProductionBuilding).getDefenseModifier() > 0)
				{
					if (bDanger)
					{
						iMinTurns = min(iMinTurns, 3);
						bEssential = true;
					}
				}
			}

			if (eProductionBuilding != NO_BUILDING)
			{
				if (GC.getBuildingInfo(eProductionBuilding).getBombardDefenseModifier() > 0)
				{
					if (bDanger)
					{
						iMinTurns = min(iMinTurns, 3);
						bEssential = true;
					}
				}
			}

			if (eProductionBuilding != NO_BUILDING)
			{
				if (GC.getBuildingInfo(eProductionBuilding).getYieldModifier(YIELD_PRODUCTION) > 0)
				{
					if (getBaseYieldRate(YIELD_PRODUCTION) >= 6)
					{
						iMinTurns = min(iMinTurns, 10);
					}
				}
			}

			if (eProductionBuilding != NO_BUILDING)
			{
				if ((GC.getBuildingInfo(eProductionBuilding).getCommerceChange(COMMERCE_CULTURE) > 0) ||
						(GC.getBuildingInfo(eProductionBuilding).getObsoleteSafeCommerceChange(COMMERCE_CULTURE) > 0))
				{
					if ((getCommerceRateTimes100(COMMERCE_CULTURE) == 0) || (plot()->calculateCulturePercent(getOwnerINLINE()) < 40))
					{
						iMinTurns = min(iMinTurns, 10);
					}
				}
			}

			if (eProductionBuilding != NO_BUILDING)
			{
				if (GC.getBuildingInfo(eProductionBuilding).getHappiness() > 0)
				{
					if (angryPopulation() > 0)
					{
						iMinTurns = min(iMinTurns, 10);
					}
				}
			}

			if (eProductionBuilding != NO_BUILDING)
			{
				if (GC.getBuildingInfo(eProductionBuilding).getHealth() > 0)
				{
					if (healthRate() < 0)
					{
						iMinTurns = min(iMinTurns, 10);
					}
				}
			}

			if (eProductionBuilding != NO_BUILDING)
			{
				if (GC.getBuildingInfo(eProductionBuilding).getSeaPlotYieldChange(YIELD_FOOD) > 0)
				{
					iMinTurns = min(iMinTurns, 10);
					bEssential = true;
				}
			}

			if (eProductionBuilding != NO_BUILDING)
			{
				if (GC.getBuildingInfo(eProductionBuilding).getFreeExperience() > 0)
				{
					if (bDanger)
					{
						iMinTurns = min(iMinTurns, 3);
						bEssential = true;
					}
				}
			}

			if (eProductionBuilding != NO_BUILDING)
			{
				if (GC.getBuildingInfo(eProductionBuilding).getMaintenanceModifier() < 0)
				{
					if (getMaintenance() >= 10)
					{
						iMinTurns = min(iMinTurns, 10);
					}
				}
			}

			if (eProductionBuilding != NO_BUILDING)
			{
				if (GC.getDefineINT("DEFAULT_SPECIALIST") != NO_SPECIALIST)
				{
					if (getSpecialistCount((SpecialistTypes)(GC.getDefineINT("DEFAULT_SPECIALIST"))) > 0)
					{
						for (iJ = 0; iJ < GC.getNumSpecialistInfos(); iJ++)
						{
							if (GC.getBuildingInfo(eProductionBuilding).getSpecialistCount(iJ) > 0)
							{
								iMinTurns = min(iMinTurns, 10);
								break;
							}
						}
					}
				}
			}

			if (eProductionBuilding != NO_BUILDING)
			{
				if (GC.getBuildingInfo(eProductionBuilding).getCommerceModifier(COMMERCE_GOLD) > 0)
				{
					if (GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble())
					{
						if (getBaseCommerceRate(COMMERCE_GOLD) >= 16)
						{
							iMinTurns = min(iMinTurns, 10);
						}
					}
				}
			}

			if (eProductionBuilding != NO_BUILDING)
			{
				if (GC.getBuildingInfo(eProductionBuilding).getCommerceModifier(COMMERCE_RESEARCH) > 0)
				{
					if (!(GET_PLAYER(getOwnerINLINE()).AI_avoidScience()))
					{
						if (getBaseCommerceRate(COMMERCE_RESEARCH) >= 16)
						{
							iMinTurns = min(iMinTurns, 10);
						}
					}
				}
			}

			if (eProductionBuilding != NO_BUILDING)
			{
				if (GC.getBuildingInfo(eProductionBuilding).getFoodKept() > 0)
				{
					iMinTurns = min(iMinTurns, 5);
					bEssential = true;
				}
			}

			if (eProductionUnit != NO_UNIT)
			{
				if (GC.getUnitInfo(eProductionUnit).getDomainType() == DOMAIN_LAND)
				{
					if (GC.getUnitInfo(eProductionUnit).getCombat() > 0)
					{
						if (bDanger)
						{
							iMinTurns = min(iMinTurns, 3);
							bEssential = true;
						}
					}
				}
			}

			if (eProductionUnitAI == UNITAI_CITY_DEFENSE)
			{
				if (plot()->plotCheck(PUF_isUnitAIType, UNITAI_SETTLE, -1, getOwnerINLINE()) != NULL)
				{
					if (!AI_isDefended(-2)) // XXX check for other team's units?
					{
						iMinTurns = min(iMinTurns, 5);
					}
				}
			}

			if (eProductionUnitAI == UNITAI_SETTLE)
			{
				if (area()->getNumAIUnits(getOwnerINLINE(), UNITAI_SETTLE) == 0)
				{
					if (!(GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble()))
					{
						if (area()->getBestFoundValue(getOwnerINLINE()) > 0)
						{
							iMinTurns = min(iMinTurns, 5);
							bEssential = true;
						}
					}
				}
			}

			if (eProductionUnitAI == UNITAI_SETTLER_SEA)
			{
				if (pWaterArea != NULL)
				{
					if (pWaterArea->getNumAIUnits(getOwnerINLINE(), UNITAI_SETTLER_SEA) == 0)
					{
						if (area()->getNumAIUnits(getOwnerINLINE(), UNITAI_SETTLE) > 0)
						{
							iMinTurns = min(iMinTurns, 5);
						}
					}
				}
			}

			if (eProductionUnitAI == UNITAI_WORKER)
			{
				if (GET_PLAYER(getOwnerINLINE()).AI_neededWorkers(area()) > (area()->getNumAIUnits(getOwnerINLINE(), UNITAI_WORKER) * 2))
				{
					iMinTurns = min(iMinTurns, 5);
					bEssential = true;
				}
			}

			if (eProductionUnitAI == UNITAI_WORKER_SEA)
			{
				if (AI_neededSeaWorkers() > 0)
				{
					iMinTurns = min(iMinTurns, 5);
					bEssential = true;
				}
			}

			// adjust for game speed
			if (NO_UNIT != getProductionUnit())
			{
				iMinTurns *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent();
			}
			else if (NO_BUILDING != getProductionBuilding())
			{
				iMinTurns *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getConstructPercent();
			}
			else if (NO_PROJECT != getProductionProject())
			{
				iMinTurns *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getCreatePercent();
			}
			else
			{
				iMinTurns *= 100;
			}

			iMinTurns /= 100;

			if (getProductionTurnsLeft() > iMinTurns)
			{
				bWait = false;

				if (!bWait)
				{
					if (iHurryAngerLength > 0)
					{
						//is the whip just too small or the population just too reduced to bother?
						if ((iHurryPopulation < 1 + GC.getDefineINT("HURRY_POP_ANGER") && !bEssential) || ((getPopulation() - iHurryPopulation) <= max(3, (getHighestPopulation() / 2))))
						{
							bWait = true;
						}
						else
						{
							//sometimes it's worth whipping even with existing anger
							if (getHurryAngerTimer() > 1)
							{
								if (!bEssential)
								{
									bWait = true;
								}
								else if (GC.getDefineINT("HURRY_POP_ANGER") == iHurryPopulation && angryPopulation() > 0)
								{
									//ideally we'll whip something more expensive
									bWait = true;
								}
							}
						}

						//if the city is just lame then don't whip the poor thing
						//(it'll still get whipped when unhappy/unhealthy)
						if (!bWait && !bEssential)
						{
							int iFoodSurplus = 0;
							CvPlot * pLoopPlot;

							for (iJ = 0; iJ < NUM_CITY_PLOTS; iJ++)
							{
								if (iJ != CITY_HOME_PLOT)
								{
									pLoopPlot = getCityIndexPlot(iJ);

									if (pLoopPlot != NULL)
									{
										if (pLoopPlot->getWorkingCity() == this)
										{
											iFoodSurplus += max(0, pLoopPlot->getYield(YIELD_FOOD) - GC.getFOOD_CONSUMPTION_PER_POPULATION());
										}
									}
								}
							}

							if (iFoodSurplus < 3)
							{
								bWait = true;
							}
						}
					}
				}

				if (!bWait)
				{
					hurry((HurryTypes)iI);
					break;
				}
			}
		}
	}
}


// Improved use of emphasize by Blake, to go with his whipping strategy - thank you!
void CvCityAI::AI_doEmphasize()
{
	PROFILE_FUNC();

	FAssert(!isHuman());

	bool bFirstTech;
	bool bEmphasize;
	int iPopulationRank;
	bool bGrow;
	bool bUrgentBuild;
	int iI;

	bool bPopRush = false; 
	for (iI = 0; iI < GC.getNumHurryInfos(); ++iI) 
	{ 
		if (canHurry((HurryTypes)iI))
		{
			if (AI_getHappyFromHurry((HurryTypes)iI) > 0)
			{
				bPopRush = true;
				break;
			}
		}
	}

	bGrow = (0 == angryPopulation(bPopRush ? 0 : 1)); 

	bUrgentBuild = AI_isDanger();
	if (!bUrgentBuild)
	{
		ProjectTypes eProductionProject = getProductionProject();
		bUrgentBuild = (eProductionProject != NO_PROJECT && AI_projectValue(eProductionProject) > 0);
	}
	if (!bUrgentBuild)
	{
		BuildingTypes eProductionBuilding = getProductionBuilding();
		bUrgentBuild = (eProductionBuilding != NO_BUILDING && isWorldWonderClass((BuildingClassTypes)(GC.getBuildingInfo(eProductionBuilding).getBuildingClassType())));
	}
	if (!bUrgentBuild)
	{
		UnitTypes eProductionUnit = getProductionUnit();
		bUrgentBuild = (eProductionUnit != NO_UNIT && GC.getUnitInfo(eProductionUnit).isFound());
	}

	if (GET_PLAYER(getOwnerINLINE()).getCurrentResearch() != NO_TECH)
	{
		bFirstTech = GC.getGameINLINE().AI_isFirstTech(GET_PLAYER(getOwnerINLINE()).getCurrentResearch());
	}
	else
	{
		bFirstTech = false;
	}

	iPopulationRank = findPopulationRank();

	for (iI = 0; iI < GC.getNumEmphasizeInfos(); iI++)
	{
		bEmphasize = false;

		if (GC.getEmphasizeInfo((EmphasizeTypes)iI).getYieldChange(YIELD_FOOD) > 0)
		{
			if (bGrow && !bUrgentBuild && !bFirstTech)
			{
				bEmphasize = true;
			}
		}

		if (GC.getEmphasizeInfo((EmphasizeTypes)iI).getYieldChange(YIELD_PRODUCTION) > 0)
		{
			if (!bFirstTech || bUrgentBuild)
			{
				bEmphasize = true;
			}
		}

		if (GC.getEmphasizeInfo((EmphasizeTypes)iI).getYieldChange(YIELD_COMMERCE) > 0)
		{
			if (!bUrgentBuild)
			{
				bEmphasize = true;
			}
		}

		if (GC.getEmphasizeInfo((EmphasizeTypes)iI).getCommerceChange(COMMERCE_RESEARCH) > 0)
		{
			if (bFirstTech && !bUrgentBuild)
			{
				if (iPopulationRank < ((GET_PLAYER(getOwnerINLINE()).getNumCities() / 4) + 1))
				{
					bEmphasize = true;
				}
			}
		}

		if (GC.getEmphasizeInfo((EmphasizeTypes)iI).isGreatPeople())
		{
			if (!bUrgentBuild)
			{
				if (iPopulationRank < ((GET_PLAYER(getOwnerINLINE()).getNumCities() / 5) + 1))
				{
					bEmphasize = true;
				}
			}
		}

		AI_setEmphasize(((EmphasizeTypes)iI), bEmphasize);
	}
}


bool CvCityAI::AI_chooseUnit(UnitAITypes eUnitAI)
{
	UnitTypes eBestUnit;

	if (eUnitAI != NO_UNITAI)
	{
		eBestUnit = AI_bestUnitAI(eUnitAI);
	}
	else
	{
		eBestUnit = AI_bestUnit(false, NO_ADVISOR, &eUnitAI);
	}

	if (eBestUnit != NO_UNIT)
	{
		pushOrder(ORDER_TRAIN, eBestUnit, eUnitAI, false, false, false);
		return true;
	}

	return false;
}


bool CvCityAI::AI_chooseDefender()
{
	if (plot()->plotCheck(PUF_isUnitAIType, UNITAI_CITY_SPECIAL, -1, getOwnerINLINE()) == NULL)
	{
		if (AI_chooseUnit(UNITAI_CITY_SPECIAL))
		{
			return true;
		}
	}

	if (plot()->plotCheck(PUF_isUnitAIType, UNITAI_CITY_COUNTER, -1, getOwnerINLINE()) == NULL)
	{
		if (AI_chooseUnit(UNITAI_CITY_COUNTER))
		{
			return true;
		}
	}

	if (AI_chooseUnit(UNITAI_CITY_DEFENSE))
	{
		return true;
	}

	return false;
}


bool CvCityAI::AI_chooseBuilding(int iFocusFlags, int iMaxTurns)
{
	BuildingTypes eBestBuilding;

	eBestBuilding = AI_bestBuilding(iFocusFlags, iMaxTurns);

	if (eBestBuilding != NO_BUILDING)
	{
		pushOrder(ORDER_CONSTRUCT, eBestBuilding, -1, false, false, false);
		return true;
	}

	return false;
}


bool CvCityAI::AI_chooseProject()
{
	ProjectTypes eBestProject;

	eBestProject = AI_bestProject();

	if (eBestProject != NO_PROJECT)
	{
		pushOrder(ORDER_CREATE, eBestProject, -1, false, false, false);
		return true;
	}

	return false;
}


bool CvCityAI::AI_chooseProcess()
{
	ProcessTypes eBestProcess;

	eBestProcess = AI_bestProcess();

	if (eBestProcess != NO_PROCESS)
	{
		pushOrder(ORDER_MAINTAIN, eBestProcess, -1, false, false, false);
		return true;
	}

	return false;
}


// Returns true if a worker was added to a plot...
bool CvCityAI::AI_addBestCitizen(bool bWorkers, bool bSpecialists)
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	SpecialistTypes eBestSpecialist;
	bool bAvoidGrowth;
	bool bIgnoreGrowth;
	int iBestPlot;
	int iValue;
	int iBestValue;
	int iI;

	bAvoidGrowth = AI_avoidGrowth();
	bIgnoreGrowth = AI_ignoreGrowth();

	iBestValue = 0;
	eBestSpecialist = NO_SPECIALIST;
	iBestPlot = -1;

	if (bSpecialists)
	{
		for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
		{
			if (isSpecialistValid(((SpecialistTypes)iI), 1))
			{
				iValue = AI_specialistValue(((SpecialistTypes)iI), bAvoidGrowth, false);

				if (iValue >= iBestValue)
				{
					iBestValue = iValue;
					eBestSpecialist = ((SpecialistTypes)iI);
					iBestPlot = -1;
				}
			}
		}
	}

	if (bWorkers)
	{
		for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
		{
			if (iI != CITY_HOME_PLOT)
			{
				if (!isWorkingPlot(iI))
				{
					pLoopPlot = getCityIndexPlot(iI);

					if (pLoopPlot != NULL)
					{
						if (canWork(pLoopPlot))
						{
							iValue = AI_plotValue(pLoopPlot, bAvoidGrowth, false, false, bIgnoreGrowth);

							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								eBestSpecialist = NO_SPECIALIST;
								iBestPlot = iI;
							}
						}
					}
				}
			}
		}
	}

	if (eBestSpecialist != NO_SPECIALIST)
	{
		changeSpecialistCount(eBestSpecialist, 1);
		return true;
	}
	else if (iBestPlot != -1)
	{
		setWorkingPlot(iBestPlot, true);
		return true;
	}

	return false;
}


// Returns true if a worker was removed from a plot...
bool CvCityAI::AI_removeWorstCitizen(SpecialistTypes eIgnoreSpecialist)
{
	CvPlot* pLoopPlot;
	SpecialistTypes eBestSpecialist;
	bool bAvoidGrowth;
	bool bIgnoreGrowth;
	int iBestPlot;
	int iValue;
	int iBestValue;
	int iI;

	if (extraFreeSpecialists() < 0)
	{
		if (GC.getDefineINT("DEFAULT_SPECIALIST") != NO_SPECIALIST)
		{
			if (eIgnoreSpecialist != GC.getDefineINT("DEFAULT_SPECIALIST"))
			{
				if (getSpecialistCount((SpecialistTypes)(GC.getDefineINT("DEFAULT_SPECIALIST"))) > getForceSpecialistCount((SpecialistTypes)(GC.getDefineINT("DEFAULT_SPECIALIST"))))
				{
					changeSpecialistCount(((SpecialistTypes)(GC.getDefineINT("DEFAULT_SPECIALIST"))), -1);
					return true;
				}
			}
		}
	}

	bAvoidGrowth = AI_avoidGrowth();
	bIgnoreGrowth = AI_ignoreGrowth();

	iBestValue = MAX_INT;
	eBestSpecialist = NO_SPECIALIST;
	iBestPlot = -1;

	if (extraFreeSpecialists() < 0)
	{
		for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
		{
			if (eIgnoreSpecialist != iI)
			{
				if (getSpecialistCount((SpecialistTypes)iI) > getForceSpecialistCount((SpecialistTypes)iI))
				{
					iValue = AI_specialistValue(((SpecialistTypes)iI), bAvoidGrowth, true);

					if (iValue < iBestValue)
					{
						iBestValue = iValue;
						eBestSpecialist = ((SpecialistTypes)iI);
						iBestPlot = -1;
					}
				}
			}
		}
	}

	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		if (iI != CITY_HOME_PLOT)
		{
			if (isWorkingPlot(iI))
			{
				pLoopPlot = getCityIndexPlot(iI);

				if (pLoopPlot != NULL)
				{
					iValue = AI_plotValue(pLoopPlot, bAvoidGrowth, true, false, bIgnoreGrowth);

					if (iValue < iBestValue)
					{
						iBestValue = iValue;
						eBestSpecialist = NO_SPECIALIST;
						iBestPlot = iI;
					}
				}
			}
		}
	}

	if (eBestSpecialist != NO_SPECIALIST)
	{
		changeSpecialistCount(eBestSpecialist, -1);
		return true;
	}
	else if (iBestPlot != -1)
	{
		setWorkingPlot(iBestPlot, false);
		return true;
	}

	if (extraFreeSpecialists() < 0)
	{
		for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
		{
			if (getSpecialistCount((SpecialistTypes)iI) > 0)
			{
				iValue = AI_specialistValue(((SpecialistTypes)iI), bAvoidGrowth, true);

				if (iValue < iBestValue)
				{
					iBestValue = iValue;
					eBestSpecialist = ((SpecialistTypes)iI);
					iBestPlot = -1;
				}
			}
		}
	}

	if (eBestSpecialist != NO_SPECIALIST)
	{
		changeSpecialistCount(eBestSpecialist, -1);
		return true;
	}

	return false;
}


void CvCityAI::AI_juggleCitizens()
{
	CvPlot* pLoopPlot;
	bool bAvoidGrowth;
	bool bIgnoreGrowth;
	int iBestPlot;
	int iValue;
	int iBestValue;
	int iI;

	bAvoidGrowth = AI_avoidGrowth();
	bIgnoreGrowth = AI_ignoreGrowth();

	int iCount = 0; // XXX

	while (true)
	{
		iBestValue = MAX_INT;
		iBestPlot = -1;

		for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
		{
			if (iI != CITY_HOME_PLOT)
			{
				if (isWorkingPlot(iI))
				{
					pLoopPlot = getCityIndexPlot(iI);

					if (pLoopPlot != NULL)
					{
						iValue = AI_plotValue(pLoopPlot, bAvoidGrowth, true, true, bIgnoreGrowth);

						if (iValue < iBestValue)
						{
							iBestValue = iValue;
							iBestPlot = iI;
						}
					}
				}
			}
		}

		if (iBestPlot == -1)
		{
			break;
		}

		setWorkingPlot(iBestPlot, false);

		if (AI_addBestCitizen(true, true))
		{
			if (isWorkingPlot(iBestPlot))
			{
				break;
			}
		}

		iCount++;
		if (iCount > NUM_CITY_PLOTS)
		{
			FAssertMsg(false, "infinite loop");
			break; // XXX
		}
	}
}


bool CvCityAI::AI_potentialPlot(short* piYields)
{
	int iNetFood = piYields[YIELD_FOOD] - GC.getFOOD_CONSUMPTION_PER_POPULATION();

	if (iNetFood <= 0)
	{
 		if (piYields[YIELD_FOOD] == 0)
		{
			if (piYields[YIELD_PRODUCTION] + piYields[YIELD_COMMERCE] < 2)
			{
				return false;
			}
		}
		else
		{
			if (piYields[YIELD_PRODUCTION] + piYields[YIELD_COMMERCE] == 0)
			{
				return false;
			}
		}
	}

	return true;
}


bool CvCityAI::AI_foodAvailable(int iExtra)
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	bool abPlotAvailable[NUM_CITY_PLOTS];
	int iFoodCount;
	int iPopulation;
	int iBestPlot;
	int iValue;
	int iBestValue;
	int iI;

	iFoodCount = 0;

	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		abPlotAvailable[iI] = false;
	}

	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		pLoopPlot = getCityIndexPlot(iI);

		if (pLoopPlot != NULL)
		{
			if (iI == CITY_HOME_PLOT)
			{
				iFoodCount += pLoopPlot->getYield(YIELD_FOOD);
			}
			else if ((pLoopPlot->getWorkingCity() == this) && AI_potentialPlot(pLoopPlot->getYield()))
			{
				abPlotAvailable[iI] = true;
			}
		}
	}

	iPopulation = (getPopulation() + iExtra);

	while (iPopulation > 0)
	{
		iBestValue = 0;
		iBestPlot = CITY_HOME_PLOT;

		for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
		{
			if (abPlotAvailable[iI])
			{
				iValue = getCityIndexPlot(iI)->getYield(YIELD_FOOD);

				if (iValue > iBestValue)
				{
					iBestValue = iValue;
					iBestPlot = iI;
				}
			}
		}

		if (iBestPlot != CITY_HOME_PLOT)
		{
			iFoodCount += iBestValue;
			abPlotAvailable[iBestPlot] = false;
		}
		else
		{
			break;
		}

		iPopulation--;
	}

	for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
	{
		iFoodCount += (GC.getSpecialistInfo((SpecialistTypes)iI).getYieldChange(YIELD_FOOD) * getFreeSpecialistCount((SpecialistTypes)iI));
	}

	if (iFoodCount < foodConsumption(false, iExtra))
	{
		return false;
	}

	return true;
}


int CvCityAI::AI_yieldValue(short* piYields, bool bAvoidGrowth, bool bRemove, bool bIgnoreFood, bool bIgnoreGrowth)
{
	PROFILE_FUNC();

	int aiYields[NUM_YIELD_TYPES];
	int iNewCityYield;
	int iOldCityYield;
	int iFoodShortage;
	int iGrowthTarget;
	int iValue;
	int iI;

	for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		if (piYields[iI] == 0)
		{
			aiYields[iI] = 0;
		}
		else
		{
			// Get yield for city after adding/removing the citizen in question
			iOldCityYield = getBaseYieldRate((YieldTypes)iI);
			iNewCityYield = (bRemove ? (iOldCityYield - piYields[iI]) : (iOldCityYield + piYields[iI]));

			int iModifier = getBaseYieldRateModifier((YieldTypes)iI);
			if (iI == YIELD_PRODUCTION)
			{
				iModifier += getProductionModifier();
			}

			iNewCityYield = (iNewCityYield * iModifier) / 100;
			iOldCityYield = (iOldCityYield * iModifier) / 100;

			if (iI == YIELD_COMMERCE)
			{
				int iNewCommerceComponent = 0;
				int iOldCommerceComponent = 0;
				for (int iJ = 0; iJ < NUM_COMMERCE_TYPES; iJ++)
				{
					iNewCommerceComponent += (getCommerceFromPercent((CommerceTypes)iJ, iNewCityYield) * getTotalCommerceRateModifier((CommerceTypes)iJ)) / 100;
					iOldCommerceComponent += (getCommerceFromPercent((CommerceTypes)iJ, iOldCityYield) * getTotalCommerceRateModifier((CommerceTypes)iJ)) / 100;
				}

				aiYields[iI] = (bRemove ? (iOldCommerceComponent - iNewCommerceComponent) : (iNewCommerceComponent - iOldCommerceComponent));
			}
			else
			{
				// The yield of the citizen in question is the difference of total yields
				// to account for rounding of modifiers
				aiYields[iI] = (bRemove ? (iOldCityYield - iNewCityYield) : (iNewCityYield - iOldCityYield));
			}

		}
	}

	iValue = 0;

	if (!bIgnoreFood)
	{
		iFoodShortage = (foodDifference(false) - ((bRemove) ? aiYields[YIELD_FOOD] : 0));

		if (bAvoidGrowth)
		{
			if (!isHuman())
			{
				iFoodShortage += max(0, (getFood() - (growthThreshold() / 2)));
			}
		}

		if (iFoodShortage < 0)
		{
			if (max(0, extraPopulation() * aiYields[YIELD_FOOD]) >= -iFoodShortage)
			{
				if (aiYields[YIELD_FOOD] <= -iFoodShortage)
				{
					iValue += aiYields[YIELD_FOOD] * 2048;
				}
				else
				{
					iValue += (-iFoodShortage * 2048) + (aiYields[YIELD_FOOD] + iFoodShortage) * 32;
				}
			}
			else
			{
				iValue += aiYields[YIELD_FOOD] * 32;
			}
		}

		if (!isFoodProduction())
		{
			if (!bAvoidGrowth)
			{
				if (!bIgnoreGrowth)
				{
					iGrowthTarget = iFoodShortage - 1;
					if (AI_isEmphasizeYield(YIELD_FOOD))
					{
						iGrowthTarget -= 2;
					}

					if (healthRate(false, 1) == 0 && angryPopulation(1) == 0)
					{
						if (getPopulation() == 1)
						{
							iGrowthTarget -= 3;
						}
						else if (getPopulation() < 6)
						{
							iGrowthTarget -= 2;
						}
						else if (getPopulation() < 12)
						{
							--iGrowthTarget;
						}
					}

					if (iGrowthTarget < 0)
					{
						iValue += (min(-(iGrowthTarget), aiYields[YIELD_FOOD]) * 128);
					}
				}

				iValue += (aiYields[YIELD_FOOD] * 8);
			}
		}
	}

	iValue += ((((isFoodProduction()) ? aiYields[YIELD_FOOD] : 0) + aiYields[YIELD_PRODUCTION]) * 16);

	iValue += (aiYields[YIELD_COMMERCE] * 4);

	if (AI_isEmphasizeYield(YIELD_FOOD))
	{
		if (!isFoodProduction())
		{
			iValue += (aiYields[YIELD_FOOD] * 64);
		}
	}

	if (AI_isEmphasizeYield(YIELD_PRODUCTION))
	{
		if (isFoodProduction())
		{
			iValue += (aiYields[YIELD_FOOD] * 32);
		}

		iValue += (aiYields[YIELD_PRODUCTION] * 32);
	}

	if (AI_isEmphasizeYield(YIELD_COMMERCE))
	{
		iValue += (aiYields[YIELD_COMMERCE] * 16);
	}

	if (!AI_isEmphasizeYield(YIELD_FOOD) || isFoodProduction())
	{
		if (!AI_potentialPlot(piYields))
		{
			iValue /= 16;
		}
	}

	return iValue;
}


int CvCityAI::AI_plotValue(CvPlot* pPlot, bool bAvoidGrowth, bool bRemove, bool bIgnoreFood, bool bIgnoreGrowth)
{
	PROFILE_FUNC();

	short aiYields[NUM_YIELD_TYPES];
	ImprovementTypes eCurrentImprovement;
	ImprovementTypes eFinalImprovement;
	int iYieldDiff;
	int iValue;
	int iI;

	iValue = 0;

	for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		aiYields[iI] = pPlot->getYield((YieldTypes)iI);
	}

	eCurrentImprovement = pPlot->getImprovementType();

	if (eCurrentImprovement != NO_IMPROVEMENT)
	{
		eFinalImprovement = finalImprovementUpgrade(eCurrentImprovement);

		if ((eFinalImprovement != NO_IMPROVEMENT) && (eFinalImprovement != eCurrentImprovement))
		{
			for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
			{
				iYieldDiff = (pPlot->calculateImprovementYieldChange(eFinalImprovement, ((YieldTypes)iI), getOwnerINLINE()) - pPlot->calculateImprovementYieldChange(eCurrentImprovement, ((YieldTypes)iI), getOwnerINLINE()));
				aiYields[iI] += (iYieldDiff + 1)/ 2;
			}
		}
	}

	iValue += (AI_yieldValue(aiYields, bAvoidGrowth, bRemove, bIgnoreFood, bIgnoreGrowth) * 100);

	if (eCurrentImprovement != NO_IMPROVEMENT)
	{
		if (pPlot->getBonusType(getTeam()) == NO_BONUS) // XXX double-check CvGame::doFeature that the checks are the same...
		{
			for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
			{
				if (GET_TEAM(getTeam()).isHasTech((TechTypes)(GC.getBonusInfo((BonusTypes) iI).getTechReveal())))
				{
					if (GC.getImprovementInfo(eCurrentImprovement).getImprovementBonusDiscoverRand(iI) > 0)
					{
						iValue++;
					}
				}
			}
		}
	}

	if ((pPlot->getImprovementType() != NO_IMPROVEMENT) && (GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementUpgrade() != NO_IMPROVEMENT))
	{
		iValue += 2;

		iValue += pPlot->getUpgradeProgress();

		if (GET_PLAYER(getOwnerINLINE()).AI_isCommercePlot(pPlot))
		{
			iValue++;
		}
	}
	else if (pPlot->getImprovementType() == NO_IMPROVEMENT)
	{
		iValue++;
	}

	return iValue;
}


int CvCityAI::AI_experienceWeight()
{
	return ((getProductionExperience() + getDomainFreeExperience(DOMAIN_SEA)) * 2);
}


int CvCityAI::AI_buildUnitProb()
{
	int iProb;

	iProb = (GC.getLeaderHeadInfo(getPersonalityType()).getBuildUnitProb() + AI_experienceWeight());

	if (GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble())
	{
		iProb /= 2;
	}

	return iProb;
}



// Improved worker AI provided by Blake - thank you!
void CvCityAI::AI_bestPlotBuild(CvPlot* pPlot, int* piBestValue, BuildTypes* peBestBuild, int iFoodPriority, int iProductionPriority, int iCommercePriority)
{
	PROFILE_FUNC();

	CvCity* pCity;
	int aiFinalYields[NUM_YIELD_TYPES];
	int aiDiffYields[NUM_YIELD_TYPES];
	ImprovementTypes eImprovement;
	ImprovementTypes eFinalImprovement;
	BuildTypes eBuild;
	BuildTypes eBestBuild;
	BuildTypes eBestTempBuild;
	BonusTypes eBonus;
	BonusTypes eNonObsoleteBonus;

	bool bEmphasizeIrrigation;
	bool bIgnoreFeature;
	bool bHasBonusImprovement;
	bool bValid;
	int iValue;
	int iBestValue;
	int iBestTempBuildValue;
	int iI, iJ;

	if (piBestValue != NULL)
	{
		*piBestValue = 0;
	}
	if (peBestBuild != NULL)
	{
		*peBestBuild = NO_BUILD;
	}

	if (pPlot->getWorkingCity() != this)
	{
		return;
	}

	FAssertMsg(pPlot->getOwnerINLINE() == getOwnerINLINE(), "pPlot must be owned by this city's owner");

	eBonus = pPlot->getBonusType(getTeam());
	eNonObsoleteBonus = pPlot->getNonObsoleteBonusType(getTeam());

	bHasBonusImprovement = false;

	if (eNonObsoleteBonus != NO_BONUS)
	{
		if (pPlot->getImprovementType() != NO_IMPROVEMENT)
		{
			if (GC.getImprovementInfo(pPlot->getImprovementType()).isImprovementBonusTrade(eNonObsoleteBonus))
			{
				bHasBonusImprovement = true;
			}
		}
	}

	iBestValue = 0;
	eBestBuild = NO_BUILD;

	if (!bHasBonusImprovement)
	{
		bEmphasizeIrrigation = false;

		for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
		{
			CvPlot* pAdjacentPlot = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), ((DirectionTypes)iI));

			if (NULL != pAdjacentPlot && pAdjacentPlot->getOwnerINLINE() == getOwnerINLINE() && pAdjacentPlot->isCityRadius())
			{
				if (!pAdjacentPlot->isIrrigationAvailable() && pAdjacentPlot->isPotentialIrrigation())
				{
					bEmphasizeIrrigation = true;
					break;
				}
			}
		}

		// XXX can we figure out if fresh water is available on this area?
		if ((area()->getNumTiles() > 40) || (area()->getNumRiverEdges() > 0))
		{
			CvPlot* pPlotA;
			CvPlot* pPlotB;

			if (!bEmphasizeIrrigation)
			{
				pPlotA = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), DIRECTION_NORTH);
				pPlotB = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), DIRECTION_SOUTH);
				if (((pPlotA == NULL) || !(pPlotA->canHavePotentialIrrigation()) || (pPlotA->getBonusType() != NO_BONUS)) &&
					((pPlotB == NULL) || !(pPlotB->canHavePotentialIrrigation()) || (pPlotB->getBonusType() != NO_BONUS)))
				{
					bEmphasizeIrrigation = true;
				}
			}

			if (!bEmphasizeIrrigation)
			{
				pPlotA = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), DIRECTION_WEST);
				pPlotB = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), DIRECTION_EAST);
				if (((pPlotA == NULL) || !(pPlotA->canHavePotentialIrrigation()) || (pPlotA->getBonusType() != NO_BONUS)) &&
					((pPlotB == NULL) || !(pPlotB->canHavePotentialIrrigation()) || (pPlotB->getBonusType() != NO_BONUS)))
				{
					bEmphasizeIrrigation = true;
				}
			}
		}


		for (iI = 0; iI < GC.getNumImprovementInfos(); iI++)
		{
			eImprovement = ((ImprovementTypes)iI);

			iBestTempBuildValue = 0;
			eBestTempBuild = NO_BUILD;

			bIgnoreFeature = false;
			bValid = false;

			if (eImprovement == pPlot->getImprovementType())
			{
				bValid = true;
			}
			else
			{
				for (iJ = 0; iJ < GC.getNumBuildInfos(); iJ++)
				{
					eBuild = ((BuildTypes)iJ);

					if (GC.getBuildInfo(eBuild).getImprovement() == eImprovement)
					{

						if (GET_PLAYER(getOwnerINLINE()).canBuild(pPlot, eBuild, false))
						{
							iValue = 10000;

							iValue /= (GC.getBuildInfo(eBuild).getTime() + 1);

							// XXX feature production???

							if (iValue > iBestTempBuildValue)
							{
								iBestTempBuildValue = iValue;
								eBestTempBuild = eBuild;
							}
						}
					}
				}

				if (eBestTempBuild != NO_BUILD)
				{
					bValid = true;

					if (pPlot->getFeatureType() != NO_FEATURE)
					{
						if (GC.getBuildInfo(eBestTempBuild).isFeatureRemove(pPlot->getFeatureType()))
						{
							bIgnoreFeature = true;

							if (GC.getFeatureInfo(pPlot->getFeatureType()).getYieldChange(YIELD_PRODUCTION) > 0)
							{
								if (eNonObsoleteBonus == NO_BONUS)
								{
									if (GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_LEAVE_FORESTS))
									{
										bValid = false;
									}
									else if (healthRate() < 0 && GC.getFeatureInfo(pPlot->getFeatureType()).getHealthPercent() > 0)
									{
										bValid = false;
									}
									else if (GET_PLAYER(getOwnerINLINE()).getFeatureHappiness(pPlot->getFeatureType()) > 0)
									{
										bValid = false;
									}
								}
							}
						}
					}
				}
			}

			if (bValid)
			{
				eFinalImprovement = finalImprovementUpgrade(eImprovement);

				if (eFinalImprovement == NO_IMPROVEMENT)
				{
					eFinalImprovement = eImprovement;
				}

				int aiTempYields[NUM_YIELD_TYPES];
				for (iJ = 0; iJ < NUM_YIELD_TYPES; ++iJ)
				{
					aiTempYields[iJ] = GC.getImprovementInfo(eFinalImprovement).getYieldChange(iJ);

					if (pPlot->canHavePotentialIrrigation())
					{
						aiTempYields[iJ] += GC.getImprovementInfo(eFinalImprovement).getIrrigatedYieldChange(iJ);
					}

					if (NO_BONUS != eBonus)
					{
						aiTempYields[iJ] += GC.getImprovementInfo(eFinalImprovement).getImprovementBonusYield(eBonus, iJ);
					}
				}

				iValue = 0;
				iValue += (aiTempYields[YIELD_FOOD] * 10);
				iValue += (aiTempYields[YIELD_PRODUCTION] * 6);
				iValue += (aiTempYields[YIELD_COMMERCE] * 2);

				if (iValue > 0)
				{
					if (eBonus != NO_BONUS)
					{
						if (eNonObsoleteBonus != NO_BONUS)
						{
							if (GC.getImprovementInfo(eFinalImprovement).isImprovementBonusTrade(eNonObsoleteBonus))
							{
								iValue += (GET_PLAYER(getOwnerINLINE()).AI_bonusVal(eNonObsoleteBonus) * 10);
								iValue += 200;
							}
							else
							{
								iValue -= 60;
							}
						}
					}
					else
					{
						for (iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
						{
							if (GC.getImprovementInfo(eFinalImprovement).getImprovementBonusDiscoverRand(iJ) > 0)
							{
								iValue++;
							}
						}
					}


					for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
					{
						aiFinalYields[iJ] = (pPlot->calculateNatureYield(((YieldTypes)iJ), getTeam(), bIgnoreFeature) + pPlot->calculateImprovementYieldChange(eFinalImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false));
						aiDiffYields[iJ] = (aiFinalYields[iJ] - pPlot->getYield(((YieldTypes)iJ)));
					}


					iValue += ((aiDiffYields[YIELD_FOOD] * (100 + 45 * iFoodPriority)));
					iValue += ((aiDiffYields[YIELD_PRODUCTION] * (60 + 27 * iProductionPriority)));
					iValue += ((aiDiffYields[YIELD_COMMERCE] * (25 + 12 * iCommercePriority)));

					// this is mainly to make it improve better tiles first
					//flood plain > grassland > plain > tundra
					iValue += (aiFinalYields[YIELD_FOOD] * 10);
					iValue += (aiFinalYields[YIELD_PRODUCTION] * 6);
					iValue += (aiFinalYields[YIELD_COMMERCE] * 4);

					if (aiFinalYields[YIELD_FOOD] >= GC.getFOOD_CONSUMPTION_PER_POPULATION())
					{
						//this is a food yielding tile
						if (iFoodPriority > 0)
						{
							iValue *= (2 + iFoodPriority);
							iValue /= 2;
						}
					}
					else if (aiDiffYields[YIELD_FOOD] <= 0)
					{
						if (iFoodPriority >= 0)
						{
							iValue *= 2;
							iValue /= (3 + iFoodPriority);
							//if the city needs food there's little sense in improving tiles
							//which can't be worked.
						}
					}

					if (bEmphasizeIrrigation && GC.getImprovementInfo(eFinalImprovement).isCarriesIrrigation())
					{
						iValue += 500;
					}

					if (!isHuman())
					{
						iValue *= max(0, (GC.getLeaderHeadInfo(getPersonalityType()).getImprovementWeightModifier(eFinalImprovement) + 200));
						iValue /= 200;
					}

					if (pPlot->getImprovementType() == NO_IMPROVEMENT)
					{
						if (pPlot->isBeingWorked())
						{
							iValue += 1;
						}

						if (eBestTempBuild != NO_BUILD)
						{
							if (pPlot->getFeatureType() != NO_FEATURE)
							{
								if (GC.getBuildInfo(eBestTempBuild).isFeatureRemove(pPlot->getFeatureType()))
								{
									iValue += pPlot->getFeatureProduction(eBestTempBuild, getTeam(), &pCity);
									FAssert(pCity == this);

									if (healthRate(false,3) < 0)
									{
										//avoid chopping forests if needed for health
										iValue -= (GC.getFeatureInfo(pPlot->getFeatureType()).getHealthPercent() * (pPlot->isBeingWorked() ? 1 : 2));
									}
								}
							}
						}
					}
					else
					{
						// cottage/villages (don't want to chop them up if turns have been invested)
						if (GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementPillage() != NO_IMPROVEMENT)
						{
							iValue -= (GC.getImprovementInfo(pPlot->getImprovementType()).getPillageGold() * 4);
							if (GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementUpgrade() != NO_IMPROVEMENT)
							{
								iValue -= (pPlot->getUpgradeProgress() * 40) / max(1, GC.getGameINLINE().getImprovementUpgradeTime(pPlot->getImprovementType()));
							}
							else
							{
								iValue -= 50;
							}
						}

						if (eNonObsoleteBonus == NO_BONUS)
						{
							iValue -= 50;
							iValue *= 2;
							iValue /= 3;
						}
					}

					if (iValue > iBestValue)
					{
						iBestValue = iValue;
						eBestBuild = eBestTempBuild;
					}
				}
			}
		}
	}

	if (eBestBuild == NO_BUILD)
	{
		if (pPlot->getFeatureType() != NO_FEATURE)
		{
			if ((GC.getFeatureInfo(pPlot->getFeatureType()).getHealthPercent() < 0) ||
				((GC.getFeatureInfo(pPlot->getFeatureType()).getYieldChange(YIELD_FOOD) + GC.getFeatureInfo(pPlot->getFeatureType()).getYieldChange(YIELD_PRODUCTION) + GC.getFeatureInfo(pPlot->getFeatureType()).getYieldChange(YIELD_COMMERCE)) < 0))
			{
				for (iI = 0; iI < GC.getNumBuildInfos(); iI++)
				{
					eBuild = ((BuildTypes)iI);

					if (GC.getBuildInfo(eBuild).getImprovement() == NO_IMPROVEMENT)
					{
						if (GC.getBuildInfo(eBuild).isFeatureRemove(pPlot->getFeatureType()))
						{
							if (GET_PLAYER(getOwnerINLINE()).canBuild(pPlot, eBuild))
							{
								iValue = 10000;

								iValue += (pPlot->getFeatureProduction(eBuild, getTeam(), &pCity) * 100);
								FAssert(pCity == this);

								iValue /= (GC.getBuildInfo(eBuild).getFeatureTime(pPlot->getFeatureType()) + 1);

								// XXX feature production???
								if (bHasBonusImprovement)
								{
									//clear jumbo of the jungle
									iValue += 500;
								}

								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									eBestBuild = eBuild;
								}
							}
						}
					}
				}
			}
		}
	}

	if (eBestBuild != NO_BUILD)
	{
		FAssertMsg(iBestValue > 0, "iBestValue is expected to be greater than 0");

		if (piBestValue != NULL)
		{
			*piBestValue = iBestValue;
		}
		if (peBestBuild != NULL)
		{
			*peBestBuild = eBestBuild;
		}
	}
}

int CvCityAI::AI_getHappyFromHurry(HurryTypes eHurry)
{
	return AI_getHappyFromHurry(hurryPopulation(eHurry));
}

int CvCityAI::AI_getHappyFromHurry(int iHurryPopulation)
{
	int iHappyDiff = iHurryPopulation - GC.getDefineINT("HURRY_POP_ANGER");
	if (iHappyDiff > 0)
	{
		if (getHurryAngerTimer() <= 1)
		{
			if (2 * angryPopulation(1) - healthRate(false, 1) > 1)
			{
				return iHappyDiff;
			}
		}
	}

	return 0;
}

int CvCityAI::AI_getHappyFromHurry(HurryTypes eHurry, UnitTypes eUnit, bool bIgnoreNew)
{
	return AI_getHappyFromHurry(getHurryPopulation(eHurry, getHurryCost(true, eUnit, bIgnoreNew)));
}

int CvCityAI::AI_getHappyFromHurry(HurryTypes eHurry, BuildingTypes eBuilding, bool bIgnoreNew)
{
	return AI_getHappyFromHurry(getHurryPopulation(eHurry, getHurryCost(true, eBuilding, bIgnoreNew)));
}


//
//
//
void CvCityAI::read(FDataStreamBase* pStream)
{
	CvCity::read(pStream);

	uint uiFlag=0;
	pStream->Read(&uiFlag);	// flags for expansion

	pStream->Read(&m_iEmphasizeAvoidGrowthCount);
	pStream->Read(&m_iEmphasizeGreatPeopleCount);

	pStream->Read(&m_bAssignWorkDirty);
	pStream->Read(&m_bChooseProductionDirty);

	pStream->Read((int*)&m_routeToCity.eOwner);
	pStream->Read(&m_routeToCity.iID);

	pStream->Read(NUM_YIELD_TYPES, m_aiEmphasizeYieldCount);
	pStream->Read(NUM_COMMERCE_TYPES, m_aiEmphasizeCommerceCount);
	pStream->Read(NUM_CITY_PLOTS, m_aiBestBuildValue);

	pStream->Read(NUM_CITY_PLOTS, (int*)m_aeBestBuild);

	pStream->Read(GC.getNumEmphasizeInfos(), m_pbEmphasize);
}

//
//
//
void CvCityAI::write(FDataStreamBase* pStream)
{
	CvCity::write(pStream);

	uint uiFlag=0;
	pStream->Write(uiFlag);		// flag for expansion

	pStream->Write(m_iEmphasizeAvoidGrowthCount);
	pStream->Write(m_iEmphasizeGreatPeopleCount);

	pStream->Write(m_bAssignWorkDirty);
	pStream->Write(m_bChooseProductionDirty);

	pStream->Write(m_routeToCity.eOwner);
	pStream->Write(m_routeToCity.iID);

	pStream->Write(NUM_YIELD_TYPES, m_aiEmphasizeYieldCount);
	pStream->Write(NUM_COMMERCE_TYPES, m_aiEmphasizeCommerceCount);
	pStream->Write(NUM_CITY_PLOTS, m_aiBestBuildValue);

	pStream->Write(NUM_CITY_PLOTS, (int*)m_aeBestBuild);

	pStream->Write(GC.getNumEmphasizeInfos(), m_pbEmphasize);
}
