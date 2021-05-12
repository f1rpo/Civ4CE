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
	m_pbEmphasize = NULL;

	AI_reset();
}


CvCityAI::~CvCityAI()
{
	AI_uninit();
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
		if (GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(area(), UNITAI_WORKER) == 0)
		{
			if (GET_PLAYER(getOwnerINLINE()).AI_neededWorkers(area()) > 0)
			{
				if (AI_chooseUnit(UNITAI_WORKER))
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
		if (getCommerceRate(COMMERCE_CULTURE) == 0)
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

	if (getBaseYieldRate(YIELD_PRODUCTION) >= 4)
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
			//if (GET_PLAYER(getOwnerINLINE()).getNumCities() > 2) XXX
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

		if (getBaseYieldRate(YIELD_PRODUCTION) >= 4)
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
	aiUnitAIVal[UNITAI_CITY_COUNTER] += ((iAreaCities + 1) / 2);
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
			if ((GET_PLAYER(getOwnerINLINE()).getNumCities() > 3) || (area()->getNumUnownedTiles() < 10))
			{
				aiUnitAIVal[UNITAI_WORKER_SEA] += AI_neededSeaWorkers();

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
				aiUnitAIVal[UNITAI_COUNTER] += ((iMilitaryWeight / ((bLandWar || bAssault) ? 11 : 18)) + ((bPrimaryArea && !bAreaAlone) ? 1 : 0));

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
	aiUnitAIVal[UNITAI_PILLAGE] *= 4;
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
					if (!isFoodProduction(eLoopUnit) || (getPopulation() > 1))
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

								iValue += getUnitProduction(eLoopUnit);

								iValue *= (GET_PLAYER(getOwnerINLINE()).getNumCities() * 2);
								iValue /= (GET_PLAYER(getOwnerINLINE()).getUnitClassCountPlusMaking((UnitClassTypes)iI) + GET_PLAYER(getOwnerINLINE()).getNumCities() + 1);

								FAssert((MAX_INT / 1000) > iValue);
								iValue *= 1000;
								iValue /= max(1, (getProductionTurnsLeft(eLoopUnit, 0) + 10));

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

						if (GC.getBuildingInfo(eLoopBuilding).getFreeBuilding() != NO_BUILDING)
						{
							iValue += (AI_buildingValue(((BuildingTypes)(GC.getBuildingInfo(eLoopBuilding).getFreeBuilding())), iFocusFlags) * (GET_PLAYER(getOwnerINLINE()).getNumCities() - GET_PLAYER(getOwnerINLINE()).getBuildingClassCountPlusMaking((BuildingClassTypes)(GC.getBuildingInfo((BuildingTypes)GC.getBuildingInfo(eLoopBuilding).getFreeBuilding()).getBuildingClassType()))));
						}

						if (iValue > 0)
						{
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

							iTurnsLeft = getProductionTurnsLeft(eLoopBuilding, 0);

							if (iTurnsLeft <= GC.getGameINLINE().AI_turnsPercent(iMaxTurns, GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getConstructPercent()))
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
	int iHasMetCount;
	int iBaseMaintenance;
	int iFoodDifference;
	int iAngryPopulation;
	int iBadHealth;
	int iValue;
	int iTempValue;
	int iPass;
	int iI, iJ;

	eStateReligion = GET_PLAYER(getOwnerINLINE()).getStateReligion();

	bAreaAlone = GET_PLAYER(getOwnerINLINE()).AI_isAreaAlone(area());
	bProvidesPower = (GC.getBuildingInfo(eBuilding).isPower() || ((GC.getBuildingInfo(eBuilding).getPowerBonus() != NO_BONUS) && hasBonus((BonusTypes)(GC.getBuildingInfo(eBuilding).getPowerBonus()))));

	iHasMetCount = GET_TEAM(getTeam()).getHasMetCivCount(true);
	iBaseMaintenance = calculateBaseMaintenance(); // XXX is this slow?
	iFoodDifference = foodDifference(false);
	iAngryPopulation = angryPopulation(1);
	FAssert(iAngryPopulation >= 0);
	iBadHealth = max(0, -(healthRate(1)));

	if (GC.getBuildingInfo(eBuilding).isCapital())
	{
		FAssert(false);
		return 0;
	}

	if (GC.getBuildingInfo(eBuilding).getObsoleteTech() != NO_TECH)
	{
		FAssertMsg(!(GET_TEAM(getTeam()).isHasTech((TechTypes)(GC.getBuildingInfo(eBuilding).getObsoleteTech()))), "Team expected to not have the tech that obsoletes eBuilding");
		if (GET_PLAYER(getOwnerINLINE()).canResearch((TechTypes)(GC.getBuildingInfo(eBuilding).getObsoleteTech())))
		{
			return 0;
		}
	}

	if (GC.getBuildingInfo(eBuilding).getSpecialBuildingType() != NO_SPECIALBUILDING)
	{
		if (GC.getSpecialBuildingInfo((SpecialBuildingTypes) GC.getBuildingInfo(eBuilding).getSpecialBuildingType()).getObsoleteTech() != NO_TECH)
		{
			FAssertMsg(!(GET_TEAM(getTeam()).isHasTech((TechTypes)(GC.getSpecialBuildingInfo((SpecialBuildingTypes) GC.getBuildingInfo(eBuilding).getSpecialBuildingType()).getObsoleteTech()))), "Team expected to not have the tech that obsoletes eBuilding");
			if (GET_PLAYER(getOwnerINLINE()).canResearch((TechTypes)(GC.getSpecialBuildingInfo((SpecialBuildingTypes) GC.getBuildingInfo(eBuilding).getSpecialBuildingType()).getObsoleteTech())))
			{
				return 0;
			}
		}
	}

	for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
	{
		if (GC.getBuildingInfo(eBuilding).getReligionChange(iI) > 0)
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
						iValue += (max(0, min(((GC.getBuildingInfo(eBuilding).getDefenseModifier() + getBuildingDefense()) - getNaturalDefense() - 10), GC.getBuildingInfo(eBuilding).getDefenseModifier())) / 4);
					}
				}

				iValue += ((GC.getBuildingInfo(eBuilding).getAllCityDefenseModifier() * GET_PLAYER(getOwnerINLINE()).getNumCities()) / 5);
			}

			if ((iFocusFlags & BUILDINGFOCUS_HAPPY) || (iPass > 0))
			{
				if (GC.getBuildingInfo(eBuilding).isNoUnhappiness())
				{
					iValue += ((iAngryPopulation + getPopulation()) * 10);
				}

				iValue += (min(GC.getBuildingInfo(eBuilding).getHappiness(), iAngryPopulation) * 10);
				iValue += (GC.getBuildingInfo(eBuilding).getAreaHappiness() * area()->getCitiesPerPlayer(getOwnerINLINE()) * 8);
				iValue += (GC.getBuildingInfo(eBuilding).getGlobalHappiness() * GET_PLAYER(getOwnerINLINE()).getNumCities() * 8);

				iValue += (min(-(((GC.getBuildingInfo(eBuilding).getWarWearinessModifier() * GET_PLAYER(getOwnerINLINE()).getWarWearinessPercentAnger()) / 100) / GC.getDefineINT("PERCENT_ANGER_DIVISOR")), iAngryPopulation) * 8);
				iValue += (-(((GC.getBuildingInfo(eBuilding).getGlobalWarWearinessModifier() * GET_PLAYER(getOwnerINLINE()).getWarWearinessPercentAnger() / 100) / GC.getDefineINT("PERCENT_ANGER_DIVISOR"))) * GET_PLAYER(getOwnerINLINE()).getNumCities());

				if (GC.getBuildingInfo(eBuilding).getReligionType() == eStateReligion)
				{
					iValue += (min(GC.getBuildingInfo(eBuilding).getStateReligionHappiness(), iAngryPopulation) * 8);
				}

				for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
				{
					iValue += (min(((GC.getBuildingInfo(eBuilding).getCommerceHappiness(iI) * GET_PLAYER(getOwnerINLINE()).getCommercePercent((CommerceTypes)iI)) / 100), iAngryPopulation) * 8);
				}

				for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
				{
					if (hasBonus((BonusTypes)iI))
					{
						iValue += (min(GC.getBuildingInfo(eBuilding).getBonusHappinessChanges(iI), iAngryPopulation) * 8);
					}
				}

				for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
				{
					if (GC.getBuildingInfo(eBuilding).getBuildingHappinessChanges(iI) != 0)
					{
						iValue += (GC.getBuildingInfo(eBuilding).getBuildingHappinessChanges(iI) * GET_PLAYER(getOwnerINLINE()).countNumBuildings((BuildingTypes)iI) * 8);
					}
				}
			}

			if ((iFocusFlags & BUILDINGFOCUS_HEALTHY) || (iPass > 0))
			{
				if (bProvidesPower)
				{
					if (isDirtyPower() && !(GC.getBuildingInfo(eBuilding).isDirtyPower()))
					{
						iValue += (min(-(GC.getDefineINT("DIRTY_POWER_HEALTH_CHANGE")), iBadHealth) * 8);
					}
				}

				if (GC.getBuildingInfo(eBuilding).isNoUnhealthyPopulation())
				{
					iValue += (min(unhealthyPopulation(), iBadHealth) * 6);
				}

				if (GC.getBuildingInfo(eBuilding).isBuildingOnlyHealthy())
				{
					iValue += (min(-(getBuildingBadHealth()), iBadHealth) * 6);
				}

				iValue += (min(GC.getBuildingInfo(eBuilding).getHealth(), iBadHealth) * 8);
				iValue += (GC.getBuildingInfo(eBuilding).getAreaHealth() * area()->getCitiesPerPlayer(getOwnerINLINE()) * 6);
				iValue += (GC.getBuildingInfo(eBuilding).getGlobalHealth() * GET_PLAYER(getOwnerINLINE()).getNumCities() * 6);

				for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
				{
					if (hasBonus((BonusTypes)iI))
					{
						iValue += (min(GC.getBuildingInfo(eBuilding).getBonusHealthChanges(iI), iBadHealth) * 6);
					}
				}
			}

			if ((iFocusFlags & BUILDINGFOCUS_EXPERIENCE) || (iPass > 0))
			{
				iValue += (GC.getBuildingInfo(eBuilding).getFreeExperience() * ((iHasMetCount > 0) ? 8 : 4));

				for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
				{
					iValue += (GC.getBuildingInfo(eBuilding).getUnitCombatFreeExperience(iI) * ((iHasMetCount > 0) ? 4 : 2));
				}

				for (iI = 0; iI < NUM_DOMAIN_TYPES; iI++)
				{
					iValue += (GC.getBuildingInfo(eBuilding).getDomainFreeExperience(iI) * ((iHasMetCount > 0) ? 6 : 3));
				}
			}

			if ((iFocusFlags & BUILDINGFOCUS_MAINTENANCE) || (iPass > 0))
			{
				iValue += ((iBaseMaintenance - ((iBaseMaintenance * max(0, (GC.getBuildingInfo(eBuilding).getMaintenanceModifier() + 100))) / 100)) * 15);
			}

			if ((iFocusFlags & BUILDINGFOCUS_SPECIALIST) || (iPass > 0))
			{
				for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
				{
					if (GC.getBuildingInfo(eBuilding).getSpecialistCount(iI) > 0)
					{
						iTempValue = AI_specialistValue(((SpecialistTypes)iI), false, false);

						iTempValue *= (50 + (10 * GC.getBuildingInfo(eBuilding).getSpecialistCount(iI)));
						iTempValue /= 100;

						iValue += (iTempValue / 100);
					}
				}
			}

			if (iPass > 0)
			{
				if (GC.getBuildingInfo(eBuilding).isAreaCleanPower())
				{
					iValue += (area()->getCitiesPerPlayer(getOwnerINLINE()) * 10); // XXX count cities without clean power???
				}

				if (GC.getBuildingInfo(eBuilding).isGovernmentCenter())
				{
					FAssert(!(GC.getBuildingInfo(eBuilding).isCapital()));
					iValue += (calculateDistanceMaintenance() * area()->getCitiesPerPlayer(getOwnerINLINE()));
				}

				if (GC.getBuildingInfo(eBuilding).isMapCentering())
				{
					iValue++;
				}

				if (GC.getBuildingInfo(eBuilding).getFreeBonus() != NO_BONUS)
				{
					iValue += (GET_PLAYER(getOwnerINLINE()).AI_bonusVal((BonusTypes)(GC.getBuildingInfo(eBuilding).getFreeBonus())) * 
						         ((GET_PLAYER(getOwnerINLINE()).getNumTradeableBonuses((BonusTypes)(GC.getBuildingInfo(eBuilding).getFreeBonus())) == 0) ? 2 : 1) * 
						         (GET_PLAYER(getOwnerINLINE()).getNumCities() + GC.getBuildingInfo(eBuilding).getNumFreeBonuses()));
				}

				if (GC.getBuildingInfo(eBuilding).getFreePromotion() != NO_PROMOTION)
				{
					iValue += ((iHasMetCount > 0) ? 10 : 5); // XXX some sort of promotion value???
				}

				if (GC.getBuildingInfo(eBuilding).getCivicOption() != NO_CIVICOPTION)
				{
					for (iI = 0; iI < GC.getNumCivicInfos(); iI++)
					{
						if (GC.getCivicInfo((CivicTypes)iI).getCivicOptionType() == GC.getBuildingInfo(eBuilding).getCivicOption())
						{
							if (!(GET_PLAYER(getOwnerINLINE()).canDoCivics((CivicTypes)iI)))
							{
								iValue += (GET_PLAYER(getOwnerINLINE()).AI_civicValue((CivicTypes)iI) / 10);
							}
						}
					}
				}

				iValue += ((GC.getBuildingInfo(eBuilding).getGreatPeopleRateModifier() * getBaseGreatPeopleRate()) / 16);
				iValue += ((GC.getBuildingInfo(eBuilding).getGlobalGreatPeopleRateModifier() * GET_PLAYER(getOwnerINLINE()).getNumCities()) / 8);

				iValue += (-(GC.getBuildingInfo(eBuilding).getAnarchyModifier()) / 4);

				iValue += (-(GC.getBuildingInfo(eBuilding).getGlobalHurryModifier()) * 2);

				iValue += (GC.getBuildingInfo(eBuilding).getGlobalFreeExperience() * GET_PLAYER(getOwnerINLINE()).getNumCities() * ((iHasMetCount > 0) ? 6 : 3));

				if (iFoodDifference > 1)
				{
					iValue += ((GC.getBuildingInfo(eBuilding).getFoodKept() * iFoodDifference) / 25);
				}

				iValue += (GC.getBuildingInfo(eBuilding).getAirlift() * getPopulation() * 4);

				iValue += (-(GC.getBuildingInfo(eBuilding).getAirModifier()) / ((iHasMetCount > 0) ? 4 : 8));

				iValue += (-(GC.getBuildingInfo(eBuilding).getNukeModifier()) / ((iHasMetCount > 0) ? 10 : 20));

				iValue += (GC.getBuildingInfo(eBuilding).getFreeSpecialist() * 16);
				iValue += (GC.getBuildingInfo(eBuilding).getAreaFreeSpecialist() * area()->getCitiesPerPlayer(getOwnerINLINE()) * 12);
				iValue += (GC.getBuildingInfo(eBuilding).getGlobalFreeSpecialist() * GET_PLAYER(getOwnerINLINE()).getNumCities() * 12);

				iValue += ((GC.getBuildingInfo(eBuilding).getWorkerSpeedModifier() * GET_PLAYER(getOwnerINLINE()).AI_getNumAIUnits(UNITAI_WORKER)) / 10);

				if (iHasMetCount > 0)
				{
					iValue += (GC.getBuildingInfo(eBuilding).getMilitaryProductionModifier() / 3);
				}

				iValue += (GC.getBuildingInfo(eBuilding).getSpaceProductionModifier() / 5);
				iValue += ((GC.getBuildingInfo(eBuilding).getGlobalSpaceProductionModifier() * GET_PLAYER(getOwnerINLINE()).getNumCities()) / 20);

				iValue += (GC.getBuildingInfo(eBuilding).getTradeRoutes() * ((10 * max(0, (totalTradeModifier() + 100))) / 100));
				iValue += (GC.getBuildingInfo(eBuilding).getCoastalTradeRoutes() * GET_PLAYER(getOwnerINLINE()).countNumCoastalCities() * 8);
				iValue += (GC.getBuildingInfo(eBuilding).getGlobalTradeRoutes() * GET_PLAYER(getOwnerINLINE()).getNumCities() * 8);

				if (GC.getBuildingInfo(eBuilding).getGreatPeopleUnitClass() != NO_UNITCLASS)
				{
					iValue++; // XXX improve this for diversity...
				}

				iValue += (GC.getBuildingInfo(eBuilding).getGreatPeopleRateChange() * 20);

				if (!bAreaAlone)
				{
					iValue += (GC.getBuildingInfo(eBuilding).getHealRateChange() / 2);
				}

				iValue += (GC.getBuildingInfo(eBuilding).getGlobalPopulationChange() * GET_PLAYER(getOwnerINLINE()).getNumCities() * 4);

				iValue += (GC.getBuildingInfo(eBuilding).getFreeTechs() * 40);

				for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
				{
					if (GC.getBuildingInfo(eBuilding).getFreeSpecialistCount(iI) > 0)
					{
						iValue += ((AI_specialistValue(((SpecialistTypes)iI), false, false) * GC.getBuildingInfo(eBuilding).getFreeSpecialistCount(iI)) / 50);
					}
				}

				for (iI = 0; iI < NUM_DOMAIN_TYPES; iI++)
				{
					iValue += (GC.getBuildingInfo(eBuilding).getDomainProductionModifier(iI) / 5);
				}

				for (iI = 0; iI < GC.getNumUnitInfos(); iI++)
				{
					if (GC.getUnitInfo((UnitTypes)iI).getPrereqBuilding() == eBuilding)
					{
						if (GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(area(), ((UnitAITypes)(GC.getUnitInfo((UnitTypes)iI).getDefaultUnitAIType()))) == 0)
						{
							iValue += area()->getCitiesPerPlayer(getOwnerINLINE());
						}

						iValue++;
					}
				}

				for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
				{
					if (GET_PLAYER(getOwnerINLINE()).getBuildingClassPrereqBuilding(((BuildingTypes)iI), ((BuildingClassTypes)(GC.getBuildingInfo(eBuilding).getBuildingClassType()))) <= GET_PLAYER(getOwnerINLINE()).getNumCities())
					{
						if (GET_PLAYER(getOwnerINLINE()).getBuildingClassPrereqBuilding(((BuildingTypes)iI), ((BuildingClassTypes)(GC.getBuildingInfo(eBuilding).getBuildingClassType()))) > GET_PLAYER(getOwnerINLINE()).getBuildingClassCountPlusMaking((BuildingClassTypes)(GC.getBuildingInfo(eBuilding).getBuildingClassType())))
						{
							iValue += (GET_PLAYER(getOwnerINLINE()).getNumCities() * 3);
						}
					}
				}
			}

			if (iPass > 0)
			{
				for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
				{
					iTempValue = 0;

					iValue += ((GC.getBuildingInfo(eBuilding).getTradeRouteModifier() * getTradeYield((YieldTypes)iI)) / 12);

					if (GC.getBuildingInfo(eBuilding).getSeaPlotYieldChange(iI) > 0)
					{
						iTempValue += (GC.getBuildingInfo(eBuilding).getSeaPlotYieldChange(iI) * countNumWaterPlots() * 4);
					}
					iTempValue += (GC.getBuildingInfo(eBuilding).getGlobalSeaPlotYieldChange(iI) * GET_PLAYER(getOwnerINLINE()).countNumCoastalCities() * 8);
					iTempValue += (GC.getBuildingInfo(eBuilding).getYieldChange(iI) * 6);
					iTempValue += ((GC.getBuildingInfo(eBuilding).getYieldModifier(iI) * getBaseYieldRate((YieldTypes)iI)) / 10);
					iTempValue += ((GC.getBuildingInfo(eBuilding).getPowerYieldModifier(iI) * getBaseYieldRate((YieldTypes)iI)) / ((bProvidesPower || isPower()) ? 12 : 15));
					iTempValue += ((GC.getBuildingInfo(eBuilding).getAreaYieldModifier(iI) * area()->getCitiesPerPlayer(getOwnerINLINE())) / 3);
					iTempValue += ((GC.getBuildingInfo(eBuilding).getGlobalYieldModifier(iI) * GET_PLAYER(getOwnerINLINE()).getNumCities()) / 3);

					if (bProvidesPower && !isPower())
					{
						iTempValue += ((getPowerYieldRateModifier((YieldTypes)iI) * getBaseYieldRate((YieldTypes)iI)) / 12);
					}

					for (iJ = 0; iJ < GC.getNumSpecialistInfos(); iJ++)
					{
						iTempValue += ((GC.getBuildingInfo(eBuilding).getSpecialistYieldChange(iJ, iI) * GET_PLAYER(getOwnerINLINE()).getTotalPopulation()) / 5);
					}

					for (iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
					{
						if (hasBonus((BonusTypes)iJ))
						{
							iTempValue += ((GC.getBuildingInfo(eBuilding).getBonusYieldModifier(iJ, iI) * getBaseYieldRate((YieldTypes)iI)) / 12);
						}
					}

					iTempValue *= GET_PLAYER(getOwnerINLINE()).AI_yieldWeight((YieldTypes)iI);
					iTempValue /= 100;

					iValue += iTempValue;
				}
			}
			else
			{
				if (iFocusFlags & BUILDINGFOCUS_FOOD)
				{
					if (GC.getBuildingInfo(eBuilding).getSeaPlotYieldChange(YIELD_FOOD) > 0)
					{
						iValue += (GC.getBuildingInfo(eBuilding).getSeaPlotYieldChange(YIELD_FOOD) * countNumWaterPlots() * 4);
					}
				}

				if (iFocusFlags & BUILDINGFOCUS_PRODUCTION)
				{
					iValue += ((GC.getBuildingInfo(eBuilding).getYieldModifier(YIELD_PRODUCTION) * getBaseYieldRate(YIELD_PRODUCTION)) / 10);
					iValue += ((GC.getBuildingInfo(eBuilding).getPowerYieldModifier(YIELD_PRODUCTION) * getBaseYieldRate(YIELD_PRODUCTION)) / ((bProvidesPower || isPower()) ? 12 : 15));

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

					iTempValue += (GC.getBuildingInfo(eBuilding).getCommerceChange(iI) * 4);
					iTempValue += (GC.getBuildingInfo(eBuilding).getObsoleteSafeCommerceChange(iI) * 4);

					if (GC.getBuildingInfo(eBuilding).getCommerceChangeDoubleTime(iI) > 0)
					{
						if ((GC.getBuildingInfo(eBuilding).getCommerceChange(iI) > 0) || (GC.getBuildingInfo(eBuilding).getObsoleteSafeCommerceChange(iI) > 0))
						{
							iTempValue += (1000 / GC.getBuildingInfo(eBuilding).getCommerceChangeDoubleTime(iI));
						}
					}

					iTempValue += ((GC.getBuildingInfo(eBuilding).getCommerceModifier(iI) * getBaseCommerceRate((CommerceTypes)iI)) / 15);
					iTempValue += ((GC.getBuildingInfo(eBuilding).getGlobalCommerceModifier(iI) * GET_PLAYER(getOwnerINLINE()).getNumCities()) / 4);
					iTempValue += ((GC.getBuildingInfo(eBuilding).getSpecialistExtraCommerce(iI) * GET_PLAYER(getOwnerINLINE()).getTotalPopulation()) / 3);

					if (eStateReligion != NO_RELIGION)
					{
						iTempValue += (GC.getBuildingInfo(eBuilding).getStateReligionCommerce(iI) * GET_PLAYER(getOwnerINLINE()).getHasReligionCount(eStateReligion) * 3);
					}

					if (GC.getBuildingInfo(eBuilding).getGlobalReligionCommerce() != NO_RELIGION)
					{
						iTempValue += (GC.getReligionInfo((ReligionTypes)(GC.getBuildingInfo(eBuilding).getGlobalReligionCommerce())).getGlobalReligionCommerce(iI) * GC.getGameINLINE().countReligionLevels((ReligionTypes)(GC.getBuildingInfo(eBuilding).getGlobalReligionCommerce())) * 2);
					}

					if (GC.getBuildingInfo(eBuilding).isCommerceFlexible(iI))
					{
						if (!(GET_PLAYER(getOwnerINLINE()).isCommerceFlexible((CommerceTypes)iI)))
						{
							iTempValue += 40;
						}
					}

					if (GC.getBuildingInfo(eBuilding).isCommerceChangeOriginalOwner(iI))
					{
						if ((GC.getBuildingInfo(eBuilding).getCommerceChange(iI) > 0) || (GC.getBuildingInfo(eBuilding).getObsoleteSafeCommerceChange(iI) > 0))
						{
							iTempValue++;
						}
					}

					iTempValue *= GET_PLAYER(getOwnerINLINE()).AI_commerceWeight(((CommerceTypes)iI), this);
					iTempValue /= 100;

					iValue += iTempValue;
				}

				for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
				{
					if (GC.getBuildingInfo(eBuilding).getReligionChange(iI) > 0)
					{
						if (GET_TEAM(getTeam()).hasHolyCity((ReligionTypes)iI))
						{
							iValue += (GC.getBuildingInfo(eBuilding).getReligionChange(iI) * ((eStateReligion == iI) ? 10 : 1));
						}
					}
				}
			}
			else
			{
				if (iFocusFlags & BUILDINGFOCUS_GOLD)
				{
					iValue += ((GC.getBuildingInfo(eBuilding).getCommerceModifier(COMMERCE_GOLD) * getBaseCommerceRate(COMMERCE_GOLD)) / 15);
				}

				if (iFocusFlags & BUILDINGFOCUS_RESEARCH)
				{
					iValue += ((GC.getBuildingInfo(eBuilding).getCommerceModifier(COMMERCE_RESEARCH) * getBaseCommerceRate(COMMERCE_RESEARCH)) / 15);
				}

				if (iFocusFlags & BUILDINGFOCUS_CULTURE)
				{
					iValue += (GC.getBuildingInfo(eBuilding).getCommerceChange(COMMERCE_CULTURE) * 4);
					iValue += (GC.getBuildingInfo(eBuilding).getObsoleteSafeCommerceChange(COMMERCE_CULTURE) * 4);
				}
			}

			if (iPass > 0)
			{
				iValue += GC.getBuildingInfo(eBuilding).getAIWeight();

				if (iValue > 0)
				{
					if (!isHuman())
					{
						for (iI = 0; iI < GC.getNumFlavorTypes(); iI++)
						{
							iValue += (GET_PLAYER(getOwnerINLINE()).AI_getFlavorValue((FlavorTypes)iI) * GC.getBuildingInfo(eBuilding).getFlavorValue(iI));
						}
					}
				}
			}
		}
	}

	if (GC.getBuildingInfo(eBuilding).getObsoleteTech() != NO_TECH)
	{
		iValue++;
		iValue /= 2;
	}

	if (GC.getBuildingInfo(eBuilding).getSpecialBuildingType() != NO_SPECIALBUILDING)
	{
		if (GC.getSpecialBuildingInfo((SpecialBuildingTypes)(GC.getBuildingInfo(eBuilding).getSpecialBuildingType())).getObsoleteTech() != NO_TECH)
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


void CvCityAI::AI_updateBestBuild()
{
	CvPlot* pLoopPlot;
	int iI;

	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		m_aiBestBuildValue[iI] = 0;
		m_aeBestBuild[iI] = NO_BUILD;

		if (iI != CITY_HOME_PLOT)
		{
			pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);

			if (pLoopPlot != NULL)
			{
				AI_bestPlotBuild(pLoopPlot, &(m_aiBestBuildValue[iI]), &(m_aeBestBuild[iI]));
				FAssert(AI_getBestBuildValue(iI) >= 0);
			}
		}
	}
}

// Protected Functions...

void CvCityAI::AI_doDraft()
{
	PROFILE_FUNC();

	bool bWait;

	FAssert(!isHuman());

	if (canConscript())
	{
		if (!AI_isDefended() && AI_isDanger())
		{
			bWait = false;

			if (!bWait)
			{
				if (getConscriptAngerTimer() > 0)
				{
					bWait = true;
				}
			}

			if (!bWait)
			{
				if (getConscriptPopulation() > (getPopulation() / 4))
				{
					bWait = true;
				}
			}

			if (!bWait)
			{
				conscript();
			}
		}
	}
}


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
					if ((getCommerceRate(COMMERCE_CULTURE) == 0) || (plot()->calculateCulturePercent(getOwnerINLINE()) < 40))
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
					if (!AI_foodAvailable())
					{
						iMinTurns = min(iMinTurns, 10);
					}
				}
			}

			if (eProductionBuilding != NO_BUILDING)
			{
				if (GC.getBuildingInfo(eProductionBuilding).getFreeExperience() > 0)
				{
					if (bDanger)
					{
						iMinTurns = min(iMinTurns, 3);
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

			if (eProductionUnit != NO_UNIT)
			{
				if (GC.getUnitInfo(eProductionUnit).getDomainType() == DOMAIN_LAND)
				{
					if (GC.getUnitInfo(eProductionUnit).getCombat() > 0)
					{
						if (bDanger)
						{
							iMinTurns = min(iMinTurns, 3);
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
				}
			}

			if (eProductionUnitAI == UNITAI_WORKER_SEA)
			{
				if (AI_neededSeaWorkers() > 0)
				{
					iMinTurns = min(iMinTurns, 5);
				}
			}

			// XXX adjust for game speed?
			if (getProductionTurnsLeft() > iMinTurns)
			{
				bWait = false;

				if (!bWait)
				{
					if (iHurryAngerLength > 0)
					{
						if (getHurryAngerTimer() > 0)
						{
							bWait = true;
						}
					}
				}

				if (!bWait)
				{
					if (iHurryPopulation > (getPopulation() / 3))
					{
						bWait = true;
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


void CvCityAI::AI_doEmphasize()
{
	PROFILE_FUNC();

	FAssert(!isHuman());

	BuildingTypes eProductionBuilding;
	ProjectTypes eProductionProject;
	bool bFirstTech;
	bool bEmphasize;
	int iPopulationRank;
	int iI;

	eProductionBuilding = getProductionBuilding();
	eProductionProject = getProductionProject();

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

		if (GC.getEmphasizeInfo((EmphasizeTypes)iI).getYieldChange(YIELD_PRODUCTION) > 0)
		{
			if (((eProductionProject != NO_PROJECT) && (AI_projectValue(eProductionProject) > 0)) ||
				  ((eProductionBuilding != NO_BUILDING) && isWorldWonderClass((BuildingClassTypes)(GC.getBuildingInfo(eProductionBuilding).getBuildingClassType()))) ||
					(AI_isDanger()))
			{
				bEmphasize = true;
			}
		}

		if (GC.getEmphasizeInfo((EmphasizeTypes)iI).getYieldChange(YIELD_COMMERCE) > 0)
		{
			if (bFirstTech)
			{
				if (GC.getGameINLINE().getElapsedGameTurns() > 60)
				{
					bEmphasize = true;
				}
			}
		}

		if (GC.getEmphasizeInfo((EmphasizeTypes)iI).getCommerceChange(COMMERCE_RESEARCH) > 0)
		{
			if (bFirstTech)
			{
				if (iPopulationRank < ((GET_PLAYER(getOwnerINLINE()).getNumCities() / 4) + 1))
				{
					bEmphasize = true;
				}
			}
		}

		if (GC.getEmphasizeInfo((EmphasizeTypes)iI).isGreatPeople())
		{
			if (iPopulationRank < ((GET_PLAYER(getOwnerINLINE()).getNumCities() / 5) + 1))
			{
				bEmphasize = true;
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
	int iBestPlot;
	int iValue;
	int iBestValue;
	int iI;

	bAvoidGrowth = AI_avoidGrowth();

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
							iValue = AI_plotValue(pLoopPlot, bAvoidGrowth, false);

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
					iValue = AI_plotValue(pLoopPlot, bAvoidGrowth, true);

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
	int iBestPlot;
	int iValue;
	int iBestValue;
	int iI;

	bAvoidGrowth = AI_avoidGrowth();

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
						iValue = AI_plotValue(pLoopPlot, bAvoidGrowth, true, true);

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
	return ((piYields[YIELD_FOOD] > GC.getFOOD_CONSUMPTION_PER_POPULATION()) || (piYields[YIELD_PRODUCTION] > 0) || (piYields[YIELD_COMMERCE] > 0));
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


int CvCityAI::AI_yieldValue(short* piYields, bool bAvoidGrowth, bool bRemove, bool bIgnoreFood)
{
	PROFILE_FUNC();

	int aiYields[NUM_YIELD_TYPES];
	int iNewCityYield;
	int iYieldCommerce;
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
			iNewCityYield = ((bRemove) ? (getBaseYieldRate((YieldTypes)iI) - piYields[iI]) : (getBaseYieldRate((YieldTypes)iI) + piYields[iI]));

			iNewCityYield = ((iNewCityYield * getBaseYieldRateModifier((YieldTypes)iI)) / 100);

			// The yield of the citizen in question is the difference of total yields
			// to account for rounding of modifiers
			aiYields[iI] = ((bRemove) ? (getYieldRate((YieldTypes)iI) - iNewCityYield) : (iNewCityYield - getYieldRate((YieldTypes)iI)));
		}
	}

	iYieldCommerce = aiYields[YIELD_COMMERCE];
	aiYields[YIELD_COMMERCE] = 0;

	for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		aiYields[YIELD_COMMERCE] += ((getCommerceFromPercent(((CommerceTypes)iI), iYieldCommerce) * getTotalCommerceRateModifier((CommerceTypes)iI)) / 100);
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
			iValue += (min(-(iFoodShortage), aiYields[YIELD_FOOD]) * 256);
		}

		if (!isFoodProduction())
		{
			if (!bAvoidGrowth)
			{
				iGrowthTarget = (iFoodShortage - 1 - ((getPopulation() < 12) ? 1 : 0) - ((getPopulation() < 6) ? 1 : 0) - ((getPopulation() == 1) ? 1 : 0) - ((AI_isEmphasizeYield(YIELD_FOOD)) ? 2 : 0));

				if (iGrowthTarget < 0)
				{
					iValue += (min(-(iGrowthTarget), aiYields[YIELD_FOOD]) * 128);
				}
			}

			iValue += (aiYields[YIELD_FOOD] * ((bAvoidGrowth) ? 1 : 8));
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

	if (!AI_isEmphasizeYield(YIELD_FOOD))
	{
		if (!AI_potentialPlot(piYields))
		{
			iValue /= 16;
		}
	}

	return iValue;
}


int CvCityAI::AI_plotValue(CvPlot* pPlot, bool bAvoidGrowth, bool bRemove, bool bIgnoreFood)
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
				aiYields[iI] += iYieldDiff;

				iValue -= (iYieldDiff * 50);
			}
		}
	}

	iValue += (AI_yieldValue(aiYields, bAvoidGrowth, bRemove, bIgnoreFood) * 100);

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


void CvCityAI::AI_bestPlotBuild(CvPlot* pPlot, int* piBestValue, BuildTypes* peBestBuild)
{
	PROFILE_FUNC();

	CvCity* pCity;
	CvPlot* pPlotA;
	CvPlot* pPlotB;
	CvPlot* pLoopPlot;
	int aiFinalYields[NUM_YIELD_TYPES];
	ImprovementTypes eImprovement;
	ImprovementTypes eFinalImprovement;
	BuildTypes eBuild;
	BuildTypes eBestBuild;
	BuildTypes eBestTempBuild;
	BonusTypes eBonus;
	BonusTypes eNonObsoleteBonus;
	bool bLowFood;
	bool bLowProduction;
	bool bLowBestProduction;
	bool bEmphasizeFood;
	bool bEmphasizeProduction;
	bool bEmphasizeCommerce;
	bool bEmphasizeIrrigation;
	bool bNeedFood;
	bool bIgnoreFeature;
	bool bHasBonusImprovement;
	bool bValid;
	int iGoodFoodPlots;
	int iGoodProductionPlots;
	int iGoodBestProductionPlots;
	int iTotalPlots;
	int iValue;
	int iBestValue;
	int iBestTempBuildValue;
	int iCount;
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
		iGoodFoodPlots = 0;
		iGoodProductionPlots = 0;
		iGoodBestProductionPlots = 0;
		iTotalPlots = 0;

		for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
		{
			if (iI != CITY_HOME_PLOT)
			{
				pLoopPlot = getCityIndexPlot(iI);

				if (pLoopPlot != NULL)
				{
					if ((pLoopPlot->calculateNatureYield(YIELD_FOOD, getTeam(), true) >= GC.getFOOD_CONSUMPTION_PER_POPULATION()) || (isCoastal() && pLoopPlot->isWater()))
					{
						iGoodFoodPlots++;
					}

					if (pLoopPlot->calculateNatureYield(YIELD_PRODUCTION, getTeam(), true) > 0)
					{
						iGoodProductionPlots++;
					}

					if (pLoopPlot->calculateBestNatureYield(YIELD_PRODUCTION, getTeam()) > 0)
					{
						iGoodBestProductionPlots++;
					}

					iTotalPlots++;
				}
			}
		}

		bLowFood = (iGoodFoodPlots < ((iTotalPlots * 2) / 3));
		bLowProduction = (iGoodProductionPlots < (iTotalPlots / 3));
		bLowBestProduction = (iGoodBestProductionPlots < (iTotalPlots / 2));

		bEmphasizeFood = (!(GET_PLAYER(getOwnerINLINE()).AI_isCommercePlot(pPlot)) || (bLowFood && (pPlot->isHills() || pPlot->isFreshWater())));
		bEmphasizeProduction = (!bLowFood && pPlot->isHills());
		bEmphasizeCommerce = GET_PLAYER(getOwnerINLINE()).AI_isCommercePlot(pPlot);

		bNeedFood = false;

		iCount = 0;

		for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
		{
			if (iI != CITY_HOME_PLOT)
			{
				pLoopPlot = getCityIndexPlot(iI);

				if (pLoopPlot != NULL)
				{
					if (!(pLoopPlot->isWater()))
					{
						if (!(pLoopPlot->isImpassable()))
						{
							if (pLoopPlot->getBonusType() == NO_BONUS)
							{
								if (pLoopPlot == pPlot)
								{
									if (((iCount + 3) % 4) == 0)
									{
										if (bLowProduction && !bLowFood)
										{
											bEmphasizeProduction = true;
										}
									}

									if (((iCount + 2) % 8) == 0)
									{
										bEmphasizeFood = true;
										bNeedFood = true;
									}

									break;
								}

								iCount++;
							}
						}
					}
				}
			}
		}

		bEmphasizeIrrigation = bEmphasizeFood;

		// XXX can we figure out if fresh water is available on this area?
		if ((area()->getNumTiles() > 40) || (area()->getNumRiverEdges() > 0))
		{
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
						if (GET_PLAYER(getOwnerINLINE()).canBuild(pPlot, eBuild, true))
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

					// XXX what about Health or Environmentalism?
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
									else if (bLowBestProduction)
									{
										if (pPlot->isHills())
										{
											bValid = false;
										}
										else if (bLowProduction || !bEmphasizeIrrigation)
										{
											//if (pPlot->calculateNatureYield(YIELD_FOOD, getTeam()) >= GC.getFOOD_CONSUMPTION_PER_POPULATION())
											if (pPlot->calculateNatureYield(YIELD_PRODUCTION, getTeam(), true) == 0)
											{
												bValid = false;
											}
										}
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

				iValue = 0;

				iValue += (max(0, GC.getImprovementInfo(eFinalImprovement).getYieldChange(YIELD_FOOD)) * 4);
				iValue += (max(0, GC.getImprovementInfo(eFinalImprovement).getYieldChange(YIELD_PRODUCTION)) * 3);
				iValue += (max(0, GC.getImprovementInfo(eFinalImprovement).getYieldChange(YIELD_COMMERCE)) * 2);

				if (bEmphasizeIrrigation)
				{
					if (GC.getImprovementInfo(eFinalImprovement).isCarriesIrrigation())
					{
						iValue += 19;
					}
				}

				if (eBonus != NO_BONUS)
				{
					if (eNonObsoleteBonus != NO_BONUS)
					{
						if (GC.getImprovementInfo(eFinalImprovement).isImprovementBonusTrade(eNonObsoleteBonus))
						{
							iValue += (GET_PLAYER(getOwnerINLINE()).AI_bonusVal(eNonObsoleteBonus) * 10);
							iValue += 100;
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

				if (iValue > 0)
				{
					for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
					{
						aiFinalYields[iJ] = (pPlot->calculateNatureYield(((YieldTypes)iJ), getTeam(), bIgnoreFeature) + pPlot->calculateImprovementYieldChange(eFinalImprovement, ((YieldTypes)iJ), getOwnerINLINE(), true));
					}

					iValue += (aiFinalYields[YIELD_FOOD] * (((bEmphasizeFood) ? 26 : 19) + ((bLowFood) ? 8 : 0) + ((bNeedFood) ? 14 : 0)));
					iValue += (aiFinalYields[YIELD_PRODUCTION] * ((bEmphasizeProduction) ? 35 : 15));
					iValue += (aiFinalYields[YIELD_COMMERCE] * ((bEmphasizeCommerce) ? 8 : 7));

					iValue += (min(0, (aiFinalYields[YIELD_FOOD] - GC.getFOOD_CONSUMPTION_PER_POPULATION())) * ((bLowFood) ? 21 : 12));

					if (!isHuman())
					{
						iValue *= max(0, (GC.getLeaderHeadInfo(getPersonalityType()).getImprovementWeightModifier(eFinalImprovement) + 100));
						iValue /= 100;
					}

					if (pPlot->getImprovementType() == NO_IMPROVEMENT)
					{
						iValue += ((pPlot->isBeingWorked()) ? 150 : 100);

						if (eBestTempBuild != NO_BUILD)
						{
							if (pPlot->getFeatureType() != NO_FEATURE)
							{
								if (GC.getBuildInfo(eBestTempBuild).isFeatureRemove(pPlot->getFeatureType()))
								{
									iValue += pPlot->getFeatureProduction(eBestTempBuild, getTeam(), &pCity);
									FAssert(pCity == this);
								}
							}
						}
					}

					// XXX feature production?
					// XXX what about cottage/villages??? (don't want to chop them up if turns have been invested...)

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
