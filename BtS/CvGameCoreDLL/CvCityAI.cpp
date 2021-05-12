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
#include "CvExtraSaveData.h"
#include "FProfiler.h"

#include "CvDLLPythonIFaceBase.h"
#include "CvDLLInterfaceIFaceBase.h"
#include "CvDLLFAStarIFaceBase.h"


#define BUILDINGFOCUS_FOOD					(1 << 1)
#define BUILDINGFOCUS_PRODUCTION			(1 << 2)
#define BUILDINGFOCUS_GOLD					(1 << 3)
#define BUILDINGFOCUS_RESEARCH				(1 << 4)
#define BUILDINGFOCUS_CULTURE				(1 << 5)
#define BUILDINGFOCUS_DEFENSE				(1 << 6)
#define BUILDINGFOCUS_HAPPY					(1 << 7)
#define BUILDINGFOCUS_HEALTHY				(1 << 8)
#define BUILDINGFOCUS_EXPERIENCE			(1 << 9)
#define BUILDINGFOCUS_MAINTENANCE			(1 << 10)
#define BUILDINGFOCUS_SPECIALIST			(1 << 11)
#define BUILDINGFOCUS_BIGCULTURE			(1 << 12)
#define BUILDINGFOCUS_WORLDWONDER			(1 << 13)
#define BUILDINGFOCUS_DOMAINSEA				(1 << 14)
#define BUILDINGFOCUS_WONDEROK				(1 << 15)


// for testing only
#ifndef FINAL_RELEASE
//#define TESTING_EXTRA_SAVE_DATA
#define DEBUG_OUT_OF_SYNCS
#define DEBUG_AI_FOOD_ADJUST
#define DEBUG_AI_FOOD_ADJUST_LOWEST_FOOD
#endif

// our own version number on our extra data
// if we want to reorder what we save, change this number
// adding to the list should be fine
#ifdef TESTING_EXTRA_SAVE_DATA

#define CITYEXTRADATA_CURRENTVERSION		0x4000
enum CityExtraDataIndicies
{
	CITYEXTRADATA_VERSION = 0,
	CITYEXTRADATA_XCOORD,
	CITYEXTRADATA_YCOORD,
};
#else

#define CITYEXTRADATA_CURRENTVERSION		1
enum CityExtraDataIndicies
{
	CITYEXTRADATA_VERSION = 0,
	//CITYEXTRADATA_UNUSED1,
	//CITYEXTRADATA_UNUSED2,
};
#endif

//#define DEBUG_CITY_BUILDS
#ifdef DEBUG_CITY_BUILDS
inline void getBuildingFocusString(CvWString& szString, int iFocusFlags)
{
	bool bFirst = true; szString = "";
	if (iFocusFlags & BUILDINGFOCUS_FOOD) {szString+=L"food"; if (!bFirst){szString+=L", ";} bFirst = false;}
	if (iFocusFlags & BUILDINGFOCUS_PRODUCTION) {szString+=L"production"; if (!bFirst){szString+=L", ";} bFirst = false;}
	if (iFocusFlags & BUILDINGFOCUS_GOLD) {szString+=L"gold"; if (!bFirst){szString+=L", ";} bFirst = false;}
	if (iFocusFlags & BUILDINGFOCUS_RESEARCH) {szString+=L"research"; if (!bFirst){szString+=L", ";} bFirst = false;}
	if (iFocusFlags & BUILDINGFOCUS_CULTURE) {szString+=L"culture"; if (!bFirst){szString+=L", ";} bFirst = false;}
	if (iFocusFlags & BUILDINGFOCUS_DEFENSE) {szString+=L"defense"; if (!bFirst){szString+=L", ";} bFirst = false;}
	if (iFocusFlags & BUILDINGFOCUS_HAPPY) {szString+=L"happy"; if (!bFirst){szString+=L", ";} bFirst = false;}
	if (iFocusFlags & BUILDINGFOCUS_HEALTHY) {szString+=L"healthy"; if (!bFirst){szString+=L", ";} bFirst = false;}
	if (iFocusFlags & BUILDINGFOCUS_EXPERIENCE) {szString+=L"experience"; if (!bFirst){szString+=L", ";} bFirst = false;}
	if (iFocusFlags & BUILDINGFOCUS_MAINTENANCE) {szString+=L"maintenance"; if (!bFirst){szString+=L", ";} bFirst = false;}
	if (iFocusFlags & BUILDINGFOCUS_SPECIALIST) {szString+=L"specialist"; if (!bFirst){szString+=L", ";} bFirst = false;}
	if (iFocusFlags & BUILDINGFOCUS_BIGCULTURE) {szString+=L"big culture"; if (!bFirst){szString+=L", ";} bFirst = false;}
	if (iFocusFlags & BUILDINGFOCUS_WORLDWONDER) {szString+=L"world wonder"; if (!bFirst){szString+=L", ";} bFirst = false;}
	if (iFocusFlags & BUILDINGFOCUS_DOMAINSEA) {szString+=L"domain sea"; if (!bFirst){szString+=L", ";} bFirst = false;}
	if (bFirst) {szString+=L"none";}
}
#endif


// Public Functions...

CvCityAI::CvCityAI()
{
	m_aiEmphasizeYieldCount = new int[NUM_YIELD_TYPES];
	m_aiEmphasizeCommerceCount = new int[NUM_COMMERCE_TYPES];
	m_bForceEmphasizeCulture = false;
	m_aiSpecialYieldMultiplier = new int[NUM_YIELD_TYPES];
	m_aiPlayerCloseness = new int[MAX_PLAYERS];

	m_pbEmphasize = NULL;

	AI_reset();
}


CvCityAI::~CvCityAI()
{
	AI_uninit();

	SAFE_DELETE_ARRAY(m_aiEmphasizeYieldCount);
	SAFE_DELETE_ARRAY(m_aiEmphasizeCommerceCount);
	SAFE_DELETE_ARRAY(m_aiSpecialYieldMultiplier);
	SAFE_DELETE_ARRAY(m_aiPlayerCloseness);

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
	m_bForceEmphasizeCulture = false;


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
	
	for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		m_aiSpecialYieldMultiplier[iI] = 0;
	}
	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		m_aiPlayerCloseness[iI] = 0;
	}
	m_iCachePlayerClosenessTurn = -1;
	m_iCachePlayerClosenessDistance = -1;
	
	m_iNeededFloatingDefenders = -1;
	m_iNeededFloatingDefendersCacheTurn = -1;

	FAssertMsg(m_pbEmphasize == NULL, "m_pbEmphasize not NULL!!!");
	FAssertMsg(GC.getNumEmphasizeInfos() > 0,  "GC.getNumEmphasizeInfos() is not greater than zero but an array is being allocated in CvCityAI::AI_reset");
	m_pbEmphasize = new bool[GC.getNumEmphasizeInfos()];
	for (iI = 0; iI < GC.getNumEmphasizeInfos(); iI++)
	{
		m_pbEmphasize[iI] = false;
	}

#ifdef NEW_CITY_GOVERNOR
	m_governorValues.bInitialized = false;
	m_governorCitizens.clear();
#endif

#ifdef COMPARE_NEW_CITY_GOVERNOR
	m_bDisableTest = false;
#endif
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
	
    if (!isHuman())
	{
	    AI_stealPlots();
	}

	AI_assignWorkingPlots();

	AI_updateBestBuild();

	AI_updateRouteToCity();

	if (isHuman())
	{
	    if (isProductionAutomated())
	    {
	        AI_doHurry();	        
	    }
		return;
	}
	
	AI_doPanic();

	AI_doDraft();

	AI_doHurry();

	AI_doEmphasize();
}


// XXX this shouldn't be called as much and optimized with the new (always work every tile) system...
void CvCityAI::AI_assignWorkingPlots()
{
	PROFILE_FUNC();

#ifdef NEW_CITY_GOVERNOR
	test_AI_AssignWorkingPlots();
#endif

#ifdef USE_NEW_CITY_GOVERNOR
	return;
#endif

	CvPlot* pHomePlot;
	int iI;
	
	// remove all assigned plots if we automated
	if (!isHuman() || isCitizensAutomated())
	{
		for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
		{
			setWorkingPlot(iI, false);
		}
	}
	
	//update the special yield multiplier to be current
	AI_updateSpecialYieldMultiplier();
	
	// remove any plots we can no longer work for any reason
	verifyWorkingPlots();
	
	// if forcing specialists, try to make all future specialists of the same type
	bool bIsSpecialistForced = false;
	int iTotalForcedSpecialists = 0;

	// make sure at least the forced amount of specialists are assigned
	for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
	{
		int iForcedSpecialistCount = getForceSpecialistCount((SpecialistTypes)iI);
		if (iForcedSpecialistCount > 0)
		{
			bIsSpecialistForced = true;
			iTotalForcedSpecialists += iForcedSpecialistCount;
		}
		
		if (!isHuman() || isCitizensAutomated() || (getSpecialistCount((SpecialistTypes)iI) < iForcedSpecialistCount))
		{
			setSpecialistCount(((SpecialistTypes)iI), iForcedSpecialistCount);
		}
	}
	
	// if we have more specialists of any type than this city can have, reduce to the max
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
	
	// always work the home plot (center)
	pHomePlot = getCityIndexPlot(CITY_HOME_PLOT);
	if (pHomePlot != NULL)
	{
		setWorkingPlot(CITY_HOME_PLOT, ((getPopulation() > 0) && canWork(pHomePlot)));
	}
	
	// keep removing the worst citizen until we are not over the limit
	while (extraPopulation() < 0)
	{
		if (!AI_removeWorstCitizen())
		{
			FAssert(false);
			break;
		}
	}
	
	// extraSpecialists() is less than extraPopulation()
	FAssertMsg(extraSpecialists() >= 0, "extraSpecialists() is expected to be non-negative (invalid Index)");
	
#ifdef COMPARE_NEW_CITY_GOVERNOR
	// dont check all the calls to AI_addBestCitizen and AI_removeWorstCitizen called past this point
	m_bDisableTest = true;
#endif

	// do we have population unassigned
	while (extraPopulation() > 0)
	{
		// (AI_addBestCitizen now handles forced specialist logic)
		if (!AI_addBestCitizen(/*bWorkers*/ true, /*bSpecialists*/ true))
		{
			break;
		}
	}

	// if forcing specialists, assign any other specialists that we must place based on forced specialists
	int iInitialExtraSpecialists = extraSpecialists();
	int iExtraSpecialists = iInitialExtraSpecialists;
	if (bIsSpecialistForced && iExtraSpecialists > 0)
	{
		FAssertMsg(iTotalForcedSpecialists > 0, "zero or negative total forced specialists");
		for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
		{
			if (isSpecialistValid((SpecialistTypes)iI, 1))
			{
				int iForcedSpecialistCount = getForceSpecialistCount((SpecialistTypes)iI);
				if (iForcedSpecialistCount > 0)
				{
					int iSpecialistCount = getSpecialistCount((SpecialistTypes)iI);
					int iMaxSpecialistCount = getMaxSpecialistCount((SpecialistTypes)iI);

					int iSpecialistsToAdd = ((iInitialExtraSpecialists * iForcedSpecialistCount) + (iTotalForcedSpecialists/2)) / iTotalForcedSpecialists;
					if (iExtraSpecialists < iSpecialistsToAdd)
					{
						iSpecialistsToAdd = iExtraSpecialists;
					}
					
					iSpecialistCount += iSpecialistsToAdd;
					iExtraSpecialists -= iSpecialistsToAdd;

					// if we cannot fit that many, then add as many as we can
					if (iSpecialistCount > iMaxSpecialistCount && !GET_PLAYER(getOwnerINLINE()).isSpecialistValid((SpecialistTypes)iI))
					{
						iExtraSpecialists += iSpecialistCount - iMaxSpecialistCount;
						iSpecialistCount = iMaxSpecialistCount;
					}

					setSpecialistCount((SpecialistTypes)iI, iSpecialistCount);
				}
			}
		}
	}
	FAssertMsg(iExtraSpecialists >= 0, "added too many specialists");

	// if we still have population to assign, assign specialists
	while (extraSpecialists() > 0)
	{
		if (!AI_addBestCitizen(/*bWorkers*/ false, /*bSpecialists*/ true))
		{
			break;
		}
	}
	
	// if automated, look for better choices than the current ones
	if (!isHuman() || isCitizensAutomated())
	{
		AI_juggleCitizens();
	}
	
	// at this point, we should not be over the limit
	FAssert((getWorkingPopulation() + getSpecialistPopulation()) <= (totalFreeSpecialists() + getPopulation()));

	AI_setAssignWorkDirty(false);

	if ((getOwnerINLINE() == GC.getGameINLINE().getActivePlayer()) && isCitySelected())
	{
		gDLL->getInterfaceIFace()->setDirty(CitizenButtons_DIRTY_BIT, true);
	}

#ifdef COMPARE_NEW_CITY_GOVERNOR
	m_bDisableTest = false;

	AI_compareGovernorCitizens();
#endif
}

#ifdef NEW_CITY_GOVERNOR

void CvCityAI::test_AI_AssignWorkingPlots()
{
	PROFILE_FUNC();

	//update the special yield multiplier to be current
	AI_updateSpecialYieldMultiplier();

	// calculate the modifiers to each yield type
	AI_calculateGovernorValues(m_governorValues);

	// build and sort the list of plots and specialists
	bool bGovernorActive = (!isHuman() || isCitizensAutomated());
	AI_buildGovernorCitizens(m_governorCitizens, m_governorValues, /*bReassign*/ bGovernorActive);
		
	// first pass, count the number of citizens already assigned, check forced specialists
	CvGovernorWork governorWork;
	AI_initializeGovernorWork(governorWork, m_governorCitizens, /*bDoForced*/ true);

	// are we working more population than we have?
	int iAvailableToWork = governorWork.availableToWork();
	if (iAvailableToWork < 0)
	{
		AI_workLessCitizens(governorWork, m_governorCitizens, -iAvailableToWork);
	}
	// do we need to work some more?
	int iUnassignedWorkers = governorWork.unassignedWorkers();
	if (iUnassignedWorkers > 0)
	{
		AI_workMoreCitizens(governorWork, m_governorCitizens, iUnassignedWorkers);
	}
	
	// at this point, we should have assigned exactly the right amount
	FAssert((governorWork.availableToWork() == 0 && governorWork.availableSpecialists() == 0));

	// adjust for growth and starvation
	AI_foodAdjustCitizens(governorWork, m_governorCitizens, m_governorValues);

	// at this point, we should still have assigned exactly the right amount
	FAssert((governorWork.availableToWork() == 0 && governorWork.availableSpecialists() == 0));

	// actually work the citizens
	AI_workGovernorCitizens(m_governorCitizens);
		
	// at this point, we should not be over the limit
	FAssert((getWorkingPopulation() + getSpecialistPopulation()) <= (totalFreeSpecialists() + getPopulation()));

	// assign is done
	AI_setAssignWorkDirty(false);
}

bool CvCityAI::test_AI_AddBestCitizen(bool bWorkers, bool bSpecialists)
{
	PROFILE_FUNC();

	// calculate the modifiers to each yield type
	AI_calculateGovernorValues(m_governorValues);

	// build and sort the list of plots and specialists
	AI_buildGovernorCitizens(m_governorCitizens, m_governorValues, /*bReassign*/ false, /*bCapWorkers*/ !bWorkers, /*bCapSpecialists*/ !bSpecialists);

	// first pass, count the number of citizens already assigned, check forced specialists
	CvGovernorWork governorWork;
	AI_initializeGovernorWork(governorWork, m_governorCitizens, /*bDoForced*/ true);

	// add one citizen
	int iChangeCount = AI_workMoreCitizens(governorWork, m_governorCitizens, 1);
	FAssert(iChangeCount <= 1);
		
	// adjust for growth and starvation
	AI_foodAdjustChangedCitizen(governorWork, m_governorCitizens, m_governorValues);

	// actually work the citizens
	AI_workGovernorCitizens(m_governorCitizens);

	return (iChangeCount > 0);
}

bool CvCityAI::test_AI_removeWorstCitizen(SpecialistTypes eIgnoreSpecialist)
{
	PROFILE_FUNC();

	// calculate the modifiers to each yield type
	AI_calculateGovernorValues(m_governorValues);

	// build and sort the list of plots and specialists
	AI_buildGovernorCitizens(m_governorCitizens, m_governorValues, /*bReassign*/ false);

	// first pass, count the number of citizens already assigned, check forced specialists
	CvGovernorWork governorWork;
	AI_initializeGovernorWork(governorWork, m_governorCitizens, /*bDoForced*/ true);

	// ensure that we do not remove the ignore specialist, by marking it forced
	if (eIgnoreSpecialist != NO_SPECIALIST)
	{
		for (CvCitizensIterator it = m_governorCitizens.begin(); it != m_governorCitizens.end(); ++it)
		{
			// is this our ignore specialist? 
			CvGovernorCitizen* pCitizen = getCitizen(it);
			if (pCitizen->bIsSpecialist && pCitizen->iIndex == eIgnoreSpecialist)
			{
				// set forced to the current value
				int iForcedDelta = (pCitizen->iCount - pCitizen->iForcedCount);
				pCitizen->iForcedCount += iForcedDelta;
				governorWork.iTotalForcedSpecialists += iForcedDelta;

				// finished (only one of each specialist)
				break;
			}
		}
	}

	// remove one citizen
	int iChangeCount = AI_workLessCitizens(governorWork, m_governorCitizens, 1);
	FAssert(iChangeCount <= 1);
	
	// adjust for growth and starvation
	AI_foodAdjustChangedCitizen(governorWork, m_governorCitizens, m_governorValues);

	// actually work the citizens
	AI_workGovernorCitizens(m_governorCitizens);

	return (iChangeCount > 0);
}

// counts the workers already assigned, assigns forced specialists, returns forced specialist count
void CvCityAI::AI_initializeGovernorWork(CvGovernorWork &governorWork, CvSortedCitizens& governorCitizens, bool bDoForced)
{
	PROFILE_FUNC();

	// counters to keep track what needs to be done
	governorWork.iWorkablePopulation = getPopulation() - angryPopulation();
	governorWork.iWorkablePlots = 0;
	governorWork.iTotalFreeSpecialists = totalFreeSpecialists();
	governorWork.iWorkingPlots = 0;
	governorWork.iWorkingSpecialists = 0;
	governorWork.iTotalForcedSpecialists = 0;

	// first pass, count the number of citizens already assigned, check forced specialists
	for (CvCitizensIterator it = governorCitizens.begin(); it != governorCitizens.end(); ++it)
	{
		CvGovernorCitizen* pCitizen = getCitizen(it);

		// specialist
		if (pCitizen->bIsSpecialist)
		{
			int iCount = pCitizen->iCount;
			int iForcedCount = pCitizen->iForcedCount;
			if (iForcedCount > 0)
			{
				if (bDoForced && iCount < iForcedCount)
				{
					iForcedCount = min(iForcedCount, pCitizen->iMax);
					pCitizen->iCount = iForcedCount;
				}

				governorWork.iTotalForcedSpecialists += iForcedCount;
			}

			governorWork.iWorkingSpecialists += pCitizen->iCount;
		}
		// plot
		else
		{
			governorWork.iWorkingPlots += pCitizen->iCount;
			
			governorWork.iWorkablePlots += pCitizen->iMax;
		}
	}

	// only used in food recalcs (for now), availableToWork() or unassignedWorkers() are probably better choices most of the time
	governorWork.iPopulationAssignable = governorWork.availableToWork();
	governorWork.iFreeSpecialistsAssignable = max(0, governorWork.iTotalFreeSpecialists - governorWork.iWorkingSpecialists);
}

int CvCityAI::AI_workMoreCitizens(CvGovernorWork &governorWork, CvSortedCitizens& governorCitizens, int iCountToAdd, bool bIgnoreForced)
{
	PROFILE_FUNC();

	// how many we have left to remove
	int iPopulationToAdd = iCountToAdd;

	// if we have forced specialists, then we will only assign of those types
	bool bOnlyForced = (!bIgnoreForced && governorWork.isForcedSpecialists()); 

	// little macros to make more readable	
	#define GET_MAX_ADD(P_CITIZEN)																	\
		(	((P_CITIZEN)->bIsSpecialist) ?															\
			((P_CITIZEN)->iMax - (P_CITIZEN)->iCount) :												\
			(min(((P_CITIZEN)->iMax - (P_CITIZEN)->iCount), governorWork.availableToWork()))	)
	
	#define ADD_CITIZEN(P_CITIZEN, I_ADD_COUNT)					\
	{	int iAddAddCount = (I_ADD_COUNT);						\
		CvGovernorCitizen* pAddCitizen = (P_CITIZEN);			\
		pAddCitizen->iCount += iAddAddCount;					\
		iPopulationToAdd -= iAddAddCount;						\
		if (pAddCitizen->bIsSpecialist)							\
			governorWork.iWorkingSpecialists += iAddAddCount;	\
		else													\
			governorWork.iWorkingPlots += iAddAddCount;	}

	// start from the best citizen, working downward (list is sorted worst to best, so do in reverse order)
	for (CvCitizensReverseIterator ir = governorCitizens.rbegin(); iPopulationToAdd > 0 && ir != governorCitizens.rend(); ++ir)
	{
		CvGovernorCitizen* pCitizen = getCitizen(ir);

		// can we work more of this citizen?
		int iAddCount = GET_MAX_ADD(pCitizen);
		if (iAddCount > 0)
		{
			// is this one we should add?
			if (!bOnlyForced || !pCitizen->bIsSpecialist || pCitizen->iForcedCount > 0)
			{
				// if only adding one, then simply add it and move to next citizen
				if (iAddCount == 1 || iPopulationToAdd == 1)
				{
					ADD_CITIZEN(pCitizen, 1);
				}
				// we want to add more than one, so look ahead, to see if there are other citizens with close values
				else
				{
					// any value within 95% is considered close enough
					int iMinValue = ((95 * getCitizenValue(ir)) + 99) / 100;
					
					// how many are there with close values we can add?
					int iCloseCount = 1;
					int iTotalAddCount = iAddCount;
					int iHighestAddCount = iAddCount;
					CvCitizensReverseIterator irNext = ir;
					while ((iCloseCount < iPopulationToAdd) && ++irNext != governorCitizens.rend() && getCitizenValue(irNext) >= iMinValue)
					{
						// can we work more of this citizen?
						CvGovernorCitizen* pNextCitizen = getCitizen(irNext);
						int iNextAddCount = GET_MAX_ADD(pNextCitizen);
						if (iNextAddCount > 0)
						{
							// is this one we should add?
							if (!bOnlyForced || !pNextCitizen->bIsSpecialist || pNextCitizen->iForcedCount > 0)
							{
								++iCloseCount;				
									
								iTotalAddCount += iNextAddCount;
								if (iNextAddCount > iHighestAddCount)
								{
									iHighestAddCount = iNextAddCount;
								}
							}
						}
					}

					// if there were none close, just add them all to this one
					if (iCloseCount == 1)
					{
						ADD_CITIZEN(pCitizen, min(iAddCount, iPopulationToAdd));
					}
					// otherwise, loop over them all
					else
					{
						// if adding the max from every one will not finish us, then do that all at once
						// otherwise will only add one at a time
						bool bAllAtOnce = (iTotalAddCount < iPopulationToAdd);

						while (iPopulationToAdd > 0 && iHighestAddCount-- > 0)
						{
							CvCitizensReverseIterator irNext = ir;
							do
							{
								// can we work more of this citizen?
								CvGovernorCitizen* pNextCitizen = getCitizen(irNext);
								iAddCount = GET_MAX_ADD(pNextCitizen);
								if (iAddCount > 0)
								{
									// is this one we should add?
									if (!bOnlyForced || !pNextCitizen->bIsSpecialist || pNextCitizen->iForcedCount > 0)
									{
										ADD_CITIZEN(pNextCitizen, bAllAtOnce ? iAddCount : 1);
									}
								}
							}
							while (iPopulationToAdd > 0 && ++irNext != governorCitizens.rend() && getCitizenValue(irNext) >= iMinValue);

							// if all at once, then done, stop loop
							if (bAllAtOnce)
							{
								break;
							}
						}
					}
				}
			}
		}
	}
	
	// if we did not add enough, try again allowing non-forced specialists
	if (bOnlyForced && iPopulationToAdd > 0)
	{
		iPopulationToAdd -= AI_workMoreCitizens(governorWork, governorCitizens, iPopulationToAdd, /*bIgnoreForced*/ true);
	}
	
	// return the number we added
	return (iCountToAdd - iPopulationToAdd);
}

int CvCityAI::AI_workLessCitizens(CvGovernorWork &governorWork, CvSortedCitizens& governorCitizens, int iCountToRemove)
{
	PROFILE_FUNC();

	// how many we have left to remove
	int iPopulationToRemove = iCountToRemove;

	// special case check, if we have more forced specialists than we can possibly assign, we will have to remove some
	bool bRemoveForced = (governorWork.iTotalForcedSpecialists > (governorWork.iWorkingPlots + governorWork.iWorkingSpecialists - iCountToRemove));

	// little macro to make more readable	
	#define REMOVE_CITIZEN(P_CITIZEN, I_REMOVE_COUNT)					\
	{	int iRemoveRemoveCount = (I_REMOVE_COUNT);						\
		CvGovernorCitizen* pRemoveCitizen = (P_CITIZEN);				\
		pRemoveCitizen->iCount -= iRemoveRemoveCount;					\
		iPopulationToRemove -= iRemoveRemoveCount;						\
		if (pRemoveCitizen->bIsSpecialist)								\
			governorWork.iWorkingSpecialists -= iRemoveRemoveCount;		\
		else															\
			governorWork.iWorkingPlots -= iRemoveRemoveCount;	}

	// start from the worst citizen, working upward (list is sorted worst to best)
	for (CvCitizensIterator it = governorCitizens.begin(); iPopulationToRemove > 0 && it != governorCitizens.end(); ++it)
	{
		CvGovernorCitizen* pCitizen = getCitizen(it);

		// are we working this citizen?
		int iCount = pCitizen->iCount;
		if (iCount > 0)
		{
			// is this one we should remove?
			if (bRemoveForced || iCount > pCitizen->iForcedCount)
			{
				// if only removing one, then simply remove it and move to next citizen
				int iRemoveCount = bRemoveForced ? iCount : (iCount - pCitizen->iForcedCount); 
				if (iRemoveCount == 1 || iPopulationToRemove == 1)
				{
					REMOVE_CITIZEN(pCitizen, 1);
				}
				// we want to remove more than one, so look ahead, to see if there are other citizens with close values
				else
				{
					// any value within 95% is considered close enough
					int iMaxValue = (105 * getCitizenValue(it)) / 100;
					
					// how many are there with close values we can remove?
					int iCloseCount = 1;
					int iTotalRemoveCount = iRemoveCount;
					int iHighestRemoveCount = iRemoveCount;
					CvCitizensIterator itNext = it;
					while ((iCloseCount < iPopulationToRemove) && ++itNext != governorCitizens.end() && getCitizenValue(itNext) <= iMaxValue)
					{
						// is this citizen worked
						CvGovernorCitizen* pNextCitizen = getCitizen(itNext);
						if (pNextCitizen->iCount > 0)
						{
							int iNextRemoveCount = bRemoveForced ? pNextCitizen->iCount : (pNextCitizen->iCount - pNextCitizen->iForcedCount); 
							if (iNextRemoveCount > 0)
							{
								++iCloseCount;				
								
								iTotalRemoveCount += iNextRemoveCount;
								if (iNextRemoveCount > iHighestRemoveCount)
								{
									iHighestRemoveCount = iNextRemoveCount;
								}
							}
						}
					}

					// if there were none close, just remove them all from this one
					if (iCloseCount == 1)
					{
						REMOVE_CITIZEN(pCitizen, min(iRemoveCount, iPopulationToRemove));
					}
					// otherwise, loop over them all
					else
					{
						// if removing the max from every one will not finish us, then do that all at once
						// otherwise will only remove one at a time
						bool bAllAtOnce = (iTotalRemoveCount < iPopulationToRemove);

						while (iPopulationToRemove > 0 && iHighestRemoveCount-- > 0)
						{
							CvCitizensIterator itNext = it;
							do
							{
								// is this citizen worked
								CvGovernorCitizen* pNextCitizen = getCitizen(itNext);
								if (pNextCitizen->iCount > 0)
								{
									iRemoveCount = bRemoveForced ? pNextCitizen->iCount : (pNextCitizen->iCount - pCitizen->iForcedCount); 
									if (iRemoveCount > 0)
									{
										REMOVE_CITIZEN(pNextCitizen, bAllAtOnce ? iRemoveCount : 1);
									}
								}
							}
							while (iPopulationToRemove > 0 && ++itNext != governorCitizens.end() && getCitizenValue(itNext) <= iMaxValue);

							// if all at once, then done, stop loop
							if (bAllAtOnce)
							{
								break;
							}
						}
					}
				}
			}
		}
	}
	
	// return the number we removed
	return (iCountToRemove - iPopulationToRemove);
}
int CvCityAI::AI_foodAdjustCitizens(CvGovernorWork &governorWork, CvSortedCitizens& governorCitizens, CvGovernorValues& governorValues)
{
	PROFILE_FUNC();

	// how many were changed? (return result)
	int iTotalChangeCount = 0;
	
	int iCurrentFood = AI_countCitizenFood(governorCitizens);
	
	int iFoodConsumption = foodConsumption();
	int iFoodDifference = iCurrentFood - iFoodConsumption;
	
	// is our food difference too low? (shoot for min of iConsumtionPerPop if we want to grow, and at least zero so we dont starve)
	int iConsumtionPerPop = GC.getFOOD_CONSUMPTION_PER_POPULATION();
	bool bEncourageGrowth = (!isFoodProduction() && (governorWork.iWorkablePlots > governorWork.iWorkingPlots) && (governorValues.bGrowPastHappy || governorValues.iHappyLevel > 0)) ? true : false;
	if (iFoodDifference < 0 || bEncourageGrowth)
	{
		int iMaxFood = AI_countMaximumCitizenFood(governorWork, governorCitizens);

		// can we avoid starvation by working the highest food possible? (if so continue, if not, do not bother)
		if ((iMaxFood - iFoodConsumption) >= 0 && iMaxFood > iCurrentFood)
		{
			// most of the time, 1 pass should be sufficient, never do more than one pass unless starving
			int iPass = 0;
			while ((iFoodDifference < 0 || bEncourageGrowth) && ++iPass < 16)
			{
#ifdef DEBUG_AI_FOOD_ADJUST
				DEBUGLOG("%S:foodAdjustCitizens: %d [max %d] - %d (%d); Pass %d\n", getName().GetCString(),
					iCurrentFood, iMaxFood, iFoodConsumption, iFoodDifference, iPass);
#endif

				FAssert(iPass < 3);
			
				// search iterator will be the first citizen with a higher value than our first swap
				bool bRemovedFirst = false;
				CvCitizensReverseIterator irFirstRemove;

				// keep track of the lowest food value we working when possible
				int iLowestFood = -1;
				CvCitizensReverseIterator irLowestFood;
				
				// start from the best citizen, working downward (list is sorted worst to best, so do in reverse order)
				// looking for a citizen that is not worked, we will then search for a better citizen that is less food to swap
				for (CvCitizensReverseIterator ir = governorCitizens.rbegin(); (bEncourageGrowth || iFoodDifference < 0) && ir != governorCitizens.rend(); ++ir)
				{
					iTotalChangeCount += swapHigherFoodCitizen(ir, iFoodDifference, bEncourageGrowth, iLowestFood, irLowestFood, governorWork, governorCitizens);
				}

				// only add food past starving on the first pass
				bEncourageGrowth = false;
			}
		}
	}
	
	return iTotalChangeCount;
}

inline bool isCitizenValueCloseEnough(int iFoodDifference, int iLowerCitizenValue, int iHigherCitizenValue, int iFoodChange)
{
	bool bCloseEnough = false;
	
	if (iFoodChange > 0)
	{
		bCloseEnough = (iFoodDifference < 0);
		if (!bCloseEnough)
		{
			int iMaxPercentage = max(1, 10 - iFoodDifference);
			iMaxPercentage = (iMaxPercentage * iMaxPercentage) / max(2, 5 - iFoodChange);
			bCloseEnough = (iHigherCitizenValue <= (iLowerCitizenValue * (100 + iMaxPercentage)) / 100);
		}
	}

	return bCloseEnough;
}

int CvCityAI::swapHigherFoodCitizen(CvCitizensReverseIterator ir, int& iFoodDifference, bool bEncourageGrowth, int& iLowestFood, CvCitizensReverseIterator& irLowestFood, CvGovernorWork &governorWork, CvSortedCitizens& governorCitizens)
{
	PROFILE_FUNC();

	int iTotalChangeCount = 0;
	
	CvGovernorCitizen* pCitizen = getCitizen(ir);
	int iFood = pCitizen->iFood;

	int iCount = pCitizen->iCount;
	int iCannotChangeCount = min(iCount, max(pCitizen->iInitialCount, pCitizen->iForcedCount));
	int iCanChangeCount = iCount - iCannotChangeCount;
	int iCanChangeMax = pCitizen->iMax - iCannotChangeCount;

	// is this the worst food we can remove? (once we remove one, do not look at lower values for low food)
	if (iCanChangeCount > 0 && (iLowestFood < 0 || iFood <= iLowestFood))
	{
		iLowestFood = iFood;
		irLowestFood = ir;

#ifdef DEBUG_AI_FOOD_ADJUST_LOWEST_FOOD
		CvGovernorCitizen* pLowestCitizen = getCitizen(irLowestFood);
		CvWString szLowestString;
		if (pLowestCitizen->bIsSpecialist) { szLowestString.Format(L"%s[%d]", GC.getSpecialistInfo((SpecialistTypes)pLowestCitizen->iIndex).getDescription(), pLowestCitizen->iCount); }
		else { szLowestString.Format(L"plot(%d)", pLowestCitizen->iIndex); }
		DEBUGLOG("  lowest food: %df; %S\n", iLowestFood, szLowestString.GetCString());
#endif
	}

	// if we are at the top of the list, then there are no better ones we can swap in
	// (since we always start lower and look for higher food)
	if (ir == governorCitizens.rbegin())
	{
		return 0;
	}
		
	// is this one we can increase food by swapping in?
	int iMoveCount = (iCanChangeMax - iCanChangeCount);
	if (iFood > 0 && iFood > iLowestFood && iMoveCount > 0)
	{
		// remember original lowest food, we will not set lowest food to a value higher than this, will keep at -1
		int iOriginalLowestFood = iLowestFood;
		
		// remove points to the citizen we will remove one (or more) of
		CvCitizensReverseIterator irRemove = governorCitizens.rend();
		
		// if we are one greater than the lowest food, start with the cached lowest food
		if ((iFood == iLowestFood + 1) && isCitizenValueCloseEnough(iFoodDifference, getCitizenValue(ir), getCitizenValue(irLowestFood), 1))
		{
			irRemove = irLowestFood;
		}
		
		// when/if we search for the worst citizen we are working that has less food starting just above this one
		CvCitizensReverseIterator irSearch = ir;

		// if we at start, we in trouble. should never happen
		FAssert(irSearch != governorCitizens.rbegin());

		// while we still have more of this one we can potentially work, and not done searching
		while ((iFoodDifference < 0 || bEncourageGrowth) && iMoveCount > 0 && irSearch != governorCitizens.rbegin())
		{
			// search for the worst citizen we are working that has less food
			if (irRemove == governorCitizens.rend())
			{
				int iLowestFoodFound = -1;
				CvCitizensReverseIterator irLowestFoundFood = governorCitizens.rend();
				
				// do the search
				bool bDone = false;
				while (!bDone)
				{
					// first decrement, then check to see whether we are at the beginning, if so, this is the last one to check
					bDone = (--irSearch == governorCitizens.rbegin());

					CvGovernorCitizen* pSearchCitizen = getCitizen(irSearch);

					iCount = pSearchCitizen->iCount;
					iCannotChangeCount = min(iCount, max(pSearchCitizen->iInitialCount, pSearchCitizen->iForcedCount));
					iCanChangeCount = iCount - iCannotChangeCount;
					// can we remove at least one citizen from this one?
					if (iCanChangeCount > 0)
					{
						int iSearchFood = pSearchCitizen->iFood;
						
						// is this one low enough food? then we done
						if (isCitizenValueCloseEnough(iFoodDifference, getCitizenValue(ir), getCitizenValue(irSearch), iFood - iSearchFood))
						{
							irRemove = irSearch;
							break;
						}
						// otherwise, update lowest food found
						else if (iLowestFoodFound < 0 || iSearchFood < iLowestFoodFound)
						{
							iLowestFoodFound = iSearchFood;
							irLowestFoundFood = irSearch;
						}
					}
				}

				// update lowest food cached value, if we can
				if ((iLowestFood < 0 && (bDone || iLowestFoodFound <= iOriginalLowestFood)) || iLowestFoodFound < iLowestFood)
				{
					iLowestFood = iLowestFoodFound;
					irLowestFood = irLowestFoundFood;

#ifdef DEBUG_AI_FOOD_ADJUST_LOWEST_FOOD
					CvGovernorCitizen* pLowestCitizen = getCitizen(irLowestFood);
					CvWString szLowestString;
					if (pLowestCitizen->bIsSpecialist) { szLowestString.Format(L"%s[%d]", GC.getSpecialistInfo((SpecialistTypes)pLowestCitizen->iIndex).getDescription(), pLowestCitizen->iCount); }
					else { szLowestString.Format(L"plot(%d)", pLowestCitizen->iIndex); }
					DEBUGLOG("  lowest food: %df; %S\n", iLowestFood, szLowestString.GetCString());
#endif
				}
			}
			
			// did we find a match?
			if (irRemove != governorCitizens.rend())
			{
				CvGovernorCitizen* pRemoveCitizen = getCitizen(irRemove);

				iCount = pRemoveCitizen->iCount;
				iCannotChangeCount = min(iCount, max(pRemoveCitizen->iInitialCount, pRemoveCitizen->iForcedCount));
				iCanChangeCount = iCount - iCannotChangeCount;
				
				FAssert(iCanChangeCount > 0);

				// how many will we move?
				int iChangeCount = min(iCanChangeCount, iMoveCount);

				// if we are at the cached lowest food, and we cannot reduce this item any further
				// then 'clear' the cache, so we do not try to use it again later
				if (irRemove == irLowestFood && iCanChangeCount <= iChangeCount)
				{
					iLowestFood = -1;
				}
				
				// check to see if we would be removing a free specialist
				if (!pCitizen->bIsSpecialist && pRemoveCitizen->bIsSpecialist)
				{
					iChangeCount = min(iChangeCount, governorWork.iWorkingSpecialists - governorWork.iTotalFreeSpecialists);

					// if we would be removing a free specialist, bail, continue through search loop
					if (iChangeCount <= 0)
					{
						irRemove = governorCitizens.rend();
						continue;
					}
				}
			
				// do the swap
#ifdef DEBUG_AI_FOOD_ADJUST
				CvWString szRemoveString, szAddString;
				if (pRemoveCitizen->bIsSpecialist) { szRemoveString.Format(L"%s[%d]", GC.getSpecialistInfo((SpecialistTypes)pRemoveCitizen->iIndex).getDescription(), pRemoveCitizen->iCount); }
				else { szRemoveString.Format(L"plot(%d)", pRemoveCitizen->iIndex); }
				if (pCitizen->bIsSpecialist) { szAddString.Format(L"%s[%d]", GC.getSpecialistInfo((SpecialistTypes)pCitizen->iIndex).getDescription(), pCitizen->iCount); }
				else { szAddString.Format(L"plot(%d)", pCitizen->iIndex); }
				DEBUGLOG("  swap: remove %S (%df) and add %S (%df) [%d%%,%d,%d]\n", 
					szRemoveString.GetCString(), pRemoveCitizen->iFood,
					szAddString.GetCString(), iFood,
					(getCitizenValue(irRemove) * 100) / getCitizenValue(ir), iChangeCount, iFoodDifference);
#endif
				pCitizen->iCount += iChangeCount;
				pRemoveCitizen->iCount -= iChangeCount;

				FAssert(pRemoveCitizen->iFood < iFood);
				iFoodDifference += (iFood - pRemoveCitizen->iFood) * iChangeCount;
				
				// are both not the same, either both specialist or both not specialist?
				if (pCitizen->bIsSpecialist != pRemoveCitizen->bIsSpecialist)
				{
					// we are adding a specialist
					if (pCitizen->bIsSpecialist)
					{
						governorWork.iWorkingSpecialists += iChangeCount;
						governorWork.iWorkingPlots -= iChangeCount;
					}
					// we are removing a specialist
					else
					{
						governorWork.iWorkingSpecialists -= iChangeCount;
						governorWork.iWorkingPlots += iChangeCount;
					}
				}

				// check to see if there is a more valueble, lower food one than the one we just swapped, to swap for that one
				int iRecursiveChangeCount = swapHigherFoodCitizen(irRemove, iFoodDifference, bEncourageGrowth, iLowestFood, irLowestFood, governorWork, governorCitizens);
				
				// the change count is the max of the two counts (so we do not count a double swap as two)
				iTotalChangeCount += max(iChangeCount, iRecursiveChangeCount);

				// clear remove, so we can search again
				irRemove = governorCitizens.rend();

				// recalc move count (recursive call to swapHigherFoodCitizen might have changed it)
				iMoveCount = (pCitizen->iMax - pCitizen->iCount);
			}
		}
	}

	return iTotalChangeCount;
}

bool CvCityAI::AI_foodAdjustChangedCitizen(CvGovernorWork &governorWork, CvSortedCitizens& governorCitizens, CvGovernorValues& governorValues)
{
	PROFILE_FUNC();

	bool bWasChanged = false;

	int iCurrentFood = AI_countCitizenFood(governorCitizens);
	
	int iFoodConsumption = foodConsumption();
	int iFoodDifference = iCurrentFood - iFoodConsumption;

	// is our food difference too low? (shoot for min of iConsumtionPerPop if we want to grow, and at least zero so we dont starve)
	int iConsumtionPerPop = GC.getFOOD_CONSUMPTION_PER_POPULATION();
	int iMinDifference = (!isFoodProduction() && (governorValues.bGrowPastHappy || governorValues.iHappyLevel > 0)) ? iConsumtionPerPop : 0;
	if (iFoodDifference < iMinDifference)
	{
		// find the changed citizen
		bool bFoundCitizen = false;
		CvCitizensReverseIterator irCitizen;
		int iCitizenFood = -1;
		int iOriginalCitizenFood = -1;
		int iOriginalCitizenValue = 0;
		bool bIsRemove = false;

		// start from the best citizen, working downward (list is sorted worst to best, so do in reverse order)
		for (CvCitizensReverseIterator ir = governorCitizens.rbegin(); iFoodDifference < iMinDifference && ir != governorCitizens.rend(); ++ir)
		{
			CvGovernorCitizen* pCitizen = getCitizen(ir);
			int iFood = pCitizen->iFood;

			int iCount = pCitizen->iCount;
			int iCannotChangeCount = min(iCount, max(pCitizen->iInitialCount, pCitizen->iForcedCount));
			int iCanChangeCount = iCount - iCannotChangeCount;
			int iCanChangeMax = pCitizen->iMax - iCannotChangeCount;

			// still searching for the one that was changed
			if (!bFoundCitizen)
			{
				if (pCitizen->iCount != pCitizen->iInitialCount)
				{
					bFoundCitizen = true;
					irCitizen = ir;
					iOriginalCitizenFood = iCitizenFood = iFood;
					iOriginalCitizenValue = getCitizenValue(irCitizen);
					bIsRemove = (pCitizen->iInitialCount > pCitizen->iCount);
				}
			}
			// found the changed one, now look for higher food (lower value) one
			else if (iFood > iCitizenFood && isCitizenValueCloseEnough(iFoodDifference, getCitizenValue(ir), iOriginalCitizenValue, iFood - iOriginalCitizenFood))
			{
				// can we swap this one
				if (bIsRemove ? (pCitizen->iCount > max(pCitizen->iInitialCount, pCitizen->iForcedCount)) : (pCitizen->iCount < pCitizen->iMax))
				{
					CvGovernorCitizen* pOldCitizen = getCitizen(irCitizen);

					// swap
					int iChange = (bIsRemove) ? -1 : 1;
					pCitizen->iCount += iChange;
					pOldCitizen->iCount -= iChange;

					// update difference
					iFoodDifference += iFood - iCitizenFood;
					bWasChanged = true;

					// remember new one
					irCitizen = ir;
					iCitizenFood = iFood;
				}
			}
		}
	}

	return bWasChanged;
}


int CvCityAI::AI_countCitizenFood(CvSortedCitizens& governorCitizens)
{
	int iTotalFood = 0;
	
	for (CvCitizensIterator it = governorCitizens.begin(); it != governorCitizens.end(); ++it)
	{
		CvGovernorCitizen* pCitizen = getCitizen(it);

		int iFood = pCitizen->iFood;
		if (iFood > 0)
		{
			int iCount = pCitizen->iCount;
			if (iCount > 0)
			{
				iTotalFood += iFood * iCount;
			}

			int iFreeCount = pCitizen->iFreeCount;
			if (iFreeCount > 0)
			{
				iTotalFood += iFood * iFreeCount;
			}
		}
	}

	return iTotalFood;
}

int CvCityAI::AI_countMaximumCitizenFood(CvGovernorWork &governorWork, CvSortedCitizens& governorCitizens)
{
	PROFILE_FUNC();

	int iMaxFood = 0;
	
	// make a new list, food sorted
	CvSortedCitizens foodSortedCitizens;

	// build a list of entries we can assign, sorted by food (do not bother with entries that have zero food)
	int iPopulationToAssign = governorWork.iWorkablePopulation;
	int iFreeSpecialistsLeft = governorWork.iTotalFreeSpecialists;
	for (CvCitizensIterator it = governorCitizens.begin(); it != governorCitizens.end(); ++it)
	{
		CvGovernorCitizen* pCitizen = getCitizen(it);

		if (pCitizen->iFood > 0)
		{
			// copy the citizen so we can make changes
			CvGovernorCitizen citizen = *pCitizen;

			// if there is intial or forced, remove that portion (counting it below)
			int iInitialCount = min(pCitizen->iInitialCount, pCitizen->iCount);
			int iForcedCount = min(pCitizen->iForcedCount, pCitizen->iCount);
			int iCannotChangeCount = max(iInitialCount, iForcedCount);
			citizen.iCount -= iCannotChangeCount;
			citizen.iMax -= iCannotChangeCount;

			// if there is food here we cannot reassign, count it now
			if (iCannotChangeCount > 0 || citizen.iFreeCount > 0)
			{
				iMaxFood += citizen.iFood * (iCannotChangeCount + citizen.iFreeCount);
			}

			// lower pop to add based on these
			if (iCannotChangeCount > 0)
			{
				// if this a specialist, first remove from the free specialists
				if (citizen.bIsSpecialist && iFreeSpecialistsLeft > 0)
				{
					int iToRemove = min(iCannotChangeCount, iFreeSpecialistsLeft);
					iFreeSpecialistsLeft -= iToRemove;
					iCannotChangeCount -= iToRemove;
				}

				// remove from pop to assign
				iPopulationToAssign -= iCannotChangeCount;
			}

			// if we can assign some changable amount of this citizen
			if (citizen.iMax > 0)
			{
				foodSortedCitizens.insert(std::make_pair(citizen.iFood, citizen));
			}
		}
	}

	// remember the values from this recalc
	governorWork.iPopulationAssignable = iPopulationToAssign;
	governorWork.iFreeSpecialistsAssignable = iFreeSpecialistsLeft;

	// grab the highest food items
	for (CvCitizensReverseIterator ir = foodSortedCitizens.rbegin(); (iPopulationToAssign > 0 || iFreeSpecialistsLeft > 0) && ir != foodSortedCitizens.rend(); ++ir)
	{
		CvGovernorCitizen* pCitizen = getCitizen(ir);

		// if this a specialist, first remove from the free specialists
		if (pCitizen->bIsSpecialist && iFreeSpecialistsLeft > 0)
		{
			int iFreeAssignCount = min(pCitizen->iMax, iFreeSpecialistsLeft);
			iFreeSpecialistsLeft -= iFreeAssignCount;
			pCitizen->iMax -= iFreeAssignCount;

			FAssert(pCitizen->iFood > 0);
			iMaxFood += pCitizen->iFood * iFreeAssignCount;
		}
		
		if (iPopulationToAssign > 0 && pCitizen->iMax > 0)
		{
			int iAssignCount = min(pCitizen->iMax, iPopulationToAssign);
			iPopulationToAssign -= iAssignCount;
			
			FAssert(pCitizen->iFood > 0);
			iMaxFood += pCitizen->iFood * iAssignCount;
		}
	}

	return iMaxFood;
}

void CvCityAI::AI_calculateGovernorValues()
{
	AI_calculateGovernorValues(m_governorValues);
}

void CvCityAI::AI_calculateGovernorValues(CvGovernorValues& governorValues)
{
	PROFILE_FUNC();

	PlayerTypes ePlayer = getOwnerINLINE();
	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
	
	bool bEmphasizeFood = AI_isEmphasizeYield(YIELD_FOOD);
	bool bEmphasizeProduction = AI_isEmphasizeYield(YIELD_PRODUCTION);
	bool bEmphasizeAvoidGrowth = AI_isEmphasizeAvoidGrowth();
	bool bFoodIsProduction = isFoodProduction();
	bool bCanPopRush = kPlayer.canPopRush();
	bool bIsNoUnhappiness = isNoUnhappiness();

	int iNumCities = kPlayer.getNumCities();
	int iPopulation = getPopulation();
	int iConsumtionPerPop = GC.getFOOD_CONSUMPTION_PER_POPULATION();
	
	// calculate modifiers
	int iExtraProductionModifier = 0;
	int iBaseProductionModifier = 100;
	for (int iYield = 0; iYield < NUM_YIELD_TYPES; iYield++)
	{
		governorValues.yieldModifiers.aiYield[iYield] = getBaseYieldRateModifier((YieldTypes) iYield);

		if (iYield == YIELD_PRODUCTION)
		{
			iBaseProductionModifier = governorValues.yieldModifiers.aiYield[YIELD_PRODUCTION];
			iExtraProductionModifier = getProductionModifier();
			governorValues.yieldModifiers.aiYield[YIELD_PRODUCTION] += iExtraProductionModifier;
		}
	}

	for (int iCommerce = 0; iCommerce < NUM_COMMERCE_TYPES; iCommerce++)
	{
		governorValues.yieldModifiers.aiCommerce[iCommerce] = getTotalCommerceRateModifier((CommerceTypes) iCommerce);
	}

	governorValues.yieldModifiers.iGPPRate = getTotalGreatPeopleRateModifier();
	governorValues.yieldModifiers.iBonusExperience = 100;
	
	// set initial yield values, based on constant weights (default any new types to commerce weight)
	for (int iYield = 0; iYield < NUM_YIELD_TYPES; iYield++)
	{
		governorValues.yieldValues.aiYield[iYield] = iGovernor_CommerceWeight;
	}
	governorValues.yieldValues.aiYield[YIELD_FOOD] = iGovernor_FoodWeight;
	governorValues.iExtraFoodValue = 0;
	governorValues.yieldValues.aiYield[YIELD_PRODUCTION] = iGovernor_ProductionWeight;
	governorValues.yieldValues.aiYield[YIELD_COMMERCE] = iGovernor_CommerceWeight;

	for (int iCommerce = 0; iCommerce < NUM_COMMERCE_TYPES; iCommerce++)
	{
		governorValues.yieldValues.aiCommerce[iCommerce] = iGovernor_CommerceWeight;
	}

	governorValues.yieldValues.iGPPRate = iGovernor_CommerceWeight / 2;
	governorValues.yieldValues.iBonusExperience = 2;

	
	// now do the real work, modifying these values based on the situation
	int iHealthLevel = goodHealth() - badHealth(/*bNoAngry*/ false, 1);
	int iHappyLevel = (bIsNoUnhappiness ? max(3, iHealthLevel + 5) : happyLevel() - unhappyLevel(0));
	
	governorValues.iHappyLevel = iHappyLevel;
	governorValues.bGrowPastHappy = false;

	// if food is production, we will set food value to the same as production value
	if (!bFoodIsProduction)
	{
		// if emphasizing food, value food high in all cases, no need to do other checks
		if (bEmphasizeFood)
		{
			governorValues.yieldValues.aiYield[YIELD_FOOD] *= iGovernor_EmphasizeMultiplier;
			governorValues.bGrowPastHappy = true;
		}
		// food value: if food isnt production, then adjust for growth
		else
		{
			// value food much less if we avoiding growth
			if (bEmphasizeAvoidGrowth)
			{
				governorValues.yieldValues.aiYield[YIELD_FOOD] /= 3;
				governorValues.bGrowPastHappy = false;
			}
			// otherwise, do some checks based on our situation
			else
			{
				// if unhealthy, value food less
				if (iHealthLevel < 0)
				{
					governorValues.yieldValues.aiYield[YIELD_FOOD] *= (iHealthLevel < -4) ? 60 : 80;
					governorValues.yieldValues.aiYield[YIELD_FOOD] /= 100;
				}

				// if we smallish, and have room to grow, value food more, adjusted by unhealth
				if (iHappyLevel > 0 && iPopulation < 15)
				{
					bool bUnhealthy = (iHealthLevel < 0);
					governorValues.yieldValues.aiYield[YIELD_FOOD] += (iGovernor_FoodWeight * (bUnhealthy ? 4 : 5)) / 8;
				}
				
				// if we are at or above the happy cap, value food less (50% if exactly at cap, 30% if over cap)
				if (iHappyLevel <= 0)
				{
					int iAdjustment = ((iHappyLevel == 0) ? 5 : 3);
					governorValues.yieldValues.aiYield[YIELD_FOOD] *= iAdjustment;
					governorValues.yieldValues.aiYield[YIELD_FOOD] /= 10;
				}
			}
		}
	}

	governorValues.yieldValues.aiYield[YIELD_FOOD] *= 100 + AI_specialYieldMultiplier(YIELD_FOOD);
	governorValues.yieldValues.aiYield[YIELD_FOOD] /= kPlayer.AI_averageYieldMultiplier(YIELD_FOOD);

	// extra food value (slavery, conscription)
	if (!bFoodIsProduction)
	{
		int iFoodToGrow = max(1, (growthThreshold() * (100 - getMaxFoodKeptPercent())) / 100);
		int iFoodConsumption = foodConsumption();
		int iProductionTurnsLeft = getProductionTurnsLeft();

		// if we can pop rush, value food more
		if (bCanPopRush)
		{
			int iBestProductionPerPopulation = 0;
			int iHurryPopulation = 0;
			for (int iHurry = 0; iHurry < GC.getNumHurryInfos(); iHurry++)
			{
				if (kPlayer.getHurryCount((HurryTypes) iHurry) > 0)
				{
					int iProductionPerPopulation = GC.getHurryInfo((HurryTypes) iHurry).getProductionPerPopulation();
					if (iProductionPerPopulation > iBestProductionPerPopulation)
					{
						iBestProductionPerPopulation = iProductionPerPopulation;
						iHurryPopulation = hurryPopulation((HurryTypes) iHurry);
					}
				}
			}
			
			// productionWeight * slaveryProductionPerPop/foodToGrow * (2*pop/foodConsumption) * (100/rushCostModifier) * (125% if globe theater)
			int iHurryCostModifier = getHurryCostModifier(/*bIgnoreNew*/ true);
			int iSlaveryValue = iGovernor_ProductionWeight * iBestProductionPerPopulation * iConsumtionPerPop * iPopulation * 100;
			iSlaveryValue *= bIsNoUnhappiness ? 125 : 100;
			iSlaveryValue /= iFoodToGrow * max(1, iFoodConsumption) * iHurryCostModifier * 100;
			
			// if we have to grow too much to actually use slavery on this build, then value slavery bonus much less
			int iExtraPopNeeded = max(0, (iHurryPopulation * 2) - iPopulation);
			iSlaveryValue /= max(1, iExtraPopNeeded - 5);

			// if we have whipped more than twice, reduce slavery value
			int iFullLengthHurryCount = getHurryAngerTimer() / flatHurryAngerLength();
			if (iFullLengthHurryCount > 1)
			{
				iSlaveryValue /= (iFullLengthHurryCount * iFullLengthHurryCount);
			}

			governorValues.iExtraFoodValue += iSlaveryValue;
		}

		// if we can draft, value food more
		int iMaxConscript;
		UnitTypes eConscriptUnit;
		bool bCanConscript = ((iMaxConscript = kPlayer.getMaxConscript()) > 0 && (eConscriptUnit = getConscriptUnit()) != NO_UNIT);
		if (bCanConscript)
		{
			int iConscriptProduction = kPlayer.getProductionNeeded(eConscriptUnit);
			int iConscriptPopulation = getConscriptPopulation();
			int iConscriptValue = iGovernor_ProductionWeight * iConscriptProduction;
			iConscriptValue *= bIsNoUnhappiness ? 3 : range(iNumCities, 1, 12);
			iConscriptValue /= iConscriptPopulation * iFoodToGrow * iConsumtionPerPop;
			iConscriptValue /= bIsNoUnhappiness ? 1 : (iMaxConscript * 2);
			
			governorValues.iExtraFoodValue += iConscriptValue;
		}
		
		// iExtraFoodValue is food working as production because of slavery or conscription 
		if (bEmphasizeProduction)
		{
			governorValues.iExtraFoodValue *= iGovernor_EmphasizeMultiplier;
		}
		
		governorValues.iExtraFoodValue *= 100 + AI_specialYieldMultiplier(YIELD_PRODUCTION);
		governorValues.iExtraFoodValue /= kPlayer.AI_averageYieldMultiplier(YIELD_PRODUCTION);
	}

	// production
	governorValues.yieldValues.aiYield[YIELD_PRODUCTION] *= 100 + AI_specialYieldMultiplier(YIELD_PRODUCTION);

	if (!bFoodIsProduction)
	{
		// normalize the production... this allows the system to account for rounding
		// and such while preventing an "out to lunch smoking weed" scenario with
		// unusually high transient production modifiers.
		// Other yields don't have transient bonuses in quite the same way.

		governorValues.yieldValues.aiYield[YIELD_PRODUCTION] *= iBaseProductionModifier;
		governorValues.yieldValues.aiYield[YIELD_PRODUCTION] /= (iBaseProductionModifier + iExtraProductionModifier);
	}
	
	if (bEmphasizeProduction)
	{
		governorValues.yieldValues.aiYield[YIELD_PRODUCTION] *= iGovernor_EmphasizeMultiplier;
	}

	governorValues.yieldValues.aiYield[YIELD_PRODUCTION] /= kPlayer.AI_averageYieldMultiplier(YIELD_PRODUCTION);

	// if food is production, then food is worth the same as production
	if (bFoodIsProduction)
	{
		governorValues.yieldValues.aiYield[YIELD_FOOD] = governorValues.yieldValues.aiYield[YIELD_PRODUCTION];
	}
	
	// commerce

	// commerce is split into its components, so value overall commerce as zero (should not be used anyway)
	governorValues.yieldValues.aiYield[YIELD_COMMERCE] = 0;
	
	// do each commerce component
	bool bEmphasizeCommerce = AI_isEmphasizeYield(YIELD_COMMERCE);
	for (int iCommerce = 0; iCommerce < NUM_COMMERCE_TYPES; iCommerce++)
	{
		// special culture value
		if (iCommerce == COMMERCE_CULTURE)
		{
			int iCultureRateRank = findCommerceRateRank(COMMERCE_CULTURE);
			int iCulturalVictoryNumCultureCities = GC.getGameINLINE().culturalVictoryNumCultureCities();
			bool bTopCultureCity = (iCultureRateRank <= iCulturalVictoryNumCultureCities + 1);

			if (kPlayer.AI_isDoStrategy(AI_STRATEGY_CULTURE3))
			{
				governorValues.yieldValues.aiCommerce[COMMERCE_CULTURE] *= bTopCultureCity ? 6 : 3;
			}
			else if (kPlayer.AI_isDoStrategy(AI_STRATEGY_CULTURE2))
			{
				governorValues.yieldValues.aiCommerce[COMMERCE_CULTURE] *= bTopCultureCity ? 3 : 2;
			}
			else
			{
				// do we have culture pressure?
				int iCulturePercent = calculateCulturePercent(ePlayer);
				if (iCulturePercent < 55)
				{
					governorValues.yieldValues.aiCommerce[COMMERCE_CULTURE] *= (2 * (105 - iCulturePercent));
					governorValues.yieldValues.aiCommerce[COMMERCE_CULTURE] /= 100;
				}
				else if (iCulturePercent > 95 && !bTopCultureCity)
				{
					governorValues.yieldValues.aiCommerce[COMMERCE_CULTURE] /= 2;
				}
			}
		}

		
		bool bEmphasizeThisCommerce = AI_isEmphasizeCommerce((CommerceTypes) iCommerce);
		if (bEmphasizeCommerce || bEmphasizeThisCommerce)
		{
			governorValues.yieldValues.aiCommerce[iCommerce] *= (bEmphasizeCommerce && bEmphasizeThisCommerce) ? ((3 * iGovernor_EmphasizeMultiplier) / 2) : iGovernor_EmphasizeMultiplier;
		}

		governorValues.yieldValues.aiCommerce[iCommerce] *= kPlayer.AI_commerceWeight((CommerceTypes) iCommerce);
		governorValues.yieldValues.aiCommerce[iCommerce] *= kPlayer.AI_averageCommerceExchange((CommerceTypes) iCommerce);
		governorValues.yieldValues.aiCommerce[iCommerce] *= 100 + AI_specialYieldMultiplier(YIELD_COMMERCE);
		governorValues.yieldValues.aiCommerce[iCommerce] /= (100 * 100 * kPlayer.AI_averageYieldMultiplier(YIELD_COMMERCE));
	}

	// GPP

	if (AI_isEmphasizeGreatPeople())
	{
		governorValues.yieldValues.iGPPRate *= iGovernor_EmphasizeMultiplier;
	}

	// if non-human, value based on this cities progress toward a GPP
	if (!isHuman())
	{
		int iProgress = getGreatPeopleProgress();
		if (iProgress > 0)
		{
			int iThreshold = kPlayer.greatPeopleThreshold();
			governorValues.yieldValues.iGPPRate *= (iProgress * iProgress);
			governorValues.yieldValues.iGPPRate /= (iThreshold * iThreshold);
		}
	}
	
	// experience
	int iHasMetCount = GET_TEAM(kPlayer.getTeam()).getHasMetCivCount(true);
	if (iHasMetCount > 0)
	{
		governorValues.yieldValues.iBonusExperience *= 2;
	}

	int iProductionRank = findYieldRateRank(YIELD_PRODUCTION);
	if (iProductionRank <= iNumCities/2 + 1)
	{
		governorValues.yieldValues.iBonusExperience += 4;
	}
	
	governorValues.yieldValues.iBonusExperience += (getMilitaryProductionModifier() * 8) / 100;

	// we are intialized
	governorValues.bInitialized = true;
}

void CvCityAI::AI_buildGovernorCitizens(bool bReassign, bool bCapWorkers, bool bCapSpecialists)
{
	// do calc if needed
	if (!m_governorValues.bInitialized)
	{
		AI_calculateGovernorValues(m_governorValues);
	}

	AI_buildGovernorCitizens(m_governorCitizens, m_governorValues, bReassign, bCapWorkers, bCapSpecialists);
}

void CvCityAI::AI_buildGovernorCitizens(CvSortedCitizens& governorCitizens, CvGovernorValues& governorValues, bool bReassign, bool bCapWorkers, bool bCapSpecialists)
{
	PROFILE_FUNC();

	PlayerTypes ePlayer = getOwnerINLINE();
	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);

	// clear the list of sorted citizens
	governorCitizens.clear();
	
	// template
	int iValue;
	CvGovernorCitizen citizen;
	
	// plots
	for (int iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		CvPlot* pLoopPlot = getCityIndexPlot(iI);
		if (pLoopPlot != NULL)
		{
			if (canWork(pLoopPlot))
			{
				// all plots have this citizen part in common
				citizen.iFood = pLoopPlot->getYield(YIELD_FOOD);
				citizen.iIndex = iI;
				citizen.bIsSpecialist = false;

				if (iI == CITY_HOME_PLOT)
				{
					// arbitrary value for home plot (no need to calculate it)
					iValue = 1;
					
					// finish citizen info
					citizen.iInitialCount = 0;
					citizen.iForcedCount = 0;
					citizen.iCount = 0;
					citizen.iMax = 0;
					citizen.iFreeCount = (getPopulation() > 0) ? 1 : 0;
				}
				else
				{
					// get value
					iValue = AI_plotValue(pLoopPlot, governorValues);
					
					// finish citizen info
					citizen.iInitialCount = (!bReassign && isWorkingPlot(iI)) ? 1 : 0;
					citizen.iForcedCount = 0;
					citizen.iCount = citizen.iInitialCount;
					citizen.iMax = (bCapWorkers) ? citizen.iCount: 1;
					citizen.iFreeCount = 0;
				}

				// add to the sorted citizens
				governorCitizens.insert(std::make_pair(iValue, citizen));
			}
			// also add plots we were working but can no longer work (so they will be removed)
			else if (isWorkingPlot(iI))
			{
				iValue = 0;
				
				// set up citizen
				citizen.iFood = 0;

				citizen.iIndex = iI;
				citizen.bIsSpecialist = false;

				citizen.iInitialCount = 0;
				citizen.iForcedCount = 0;
				citizen.iCount = 0;
				citizen.iMax = 0;

				citizen.iFreeCount = 0;

				// add to the sorted citizens
				governorCitizens.insert(std::make_pair(iValue, citizen));
			}
		}
	}

	//  specialists
	SpecialistTypes eDefaultSpecialist = (SpecialistTypes) GC.getDefineINT("DEFAULT_SPECIALIST");
	for (int iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
	{
		SpecialistTypes eSpecialist = (SpecialistTypes) iI;

		// get value
		iValue = AI_specialistValue(eSpecialist, governorValues);
		
		// set up citizen
		citizen.iFood = kPlayer.specialistYield(eSpecialist, YIELD_FOOD);
		
		citizen.iIndex = eSpecialist;
		citizen.bIsSpecialist = true;

		citizen.iInitialCount = bReassign ? 0 : getSpecialistCount(eSpecialist);
		citizen.iForcedCount = getForceSpecialistCount(eSpecialist);
		citizen.iCount = citizen.iInitialCount;
		citizen.iMax = (!bCapSpecialists) ? ((eSpecialist == eDefaultSpecialist || kPlayer.isSpecialistValid(eSpecialist)) ? MAX_SHORT : getMaxSpecialistCount(eSpecialist)) : citizen.iCount;

		citizen.iFreeCount = getFreeSpecialistCount(eSpecialist);

		// add to the sorted citizens
		governorCitizens.insert(std::make_pair(iValue, citizen));
	}
}

void CvCityAI::AI_workGovernorCitizens(CvSortedCitizens& governorCitizens)
{
#ifdef USE_NEW_CITY_GOVERNOR
	// actually work (or stop working) each citizen in our list
	for (CvCitizensIterator it = governorCitizens.begin(); it != governorCitizens.end(); ++it)
	{
		CvGovernorCitizen* pCitizen = getCitizen(it);

		// specialists
		if (pCitizen->bIsSpecialist)
		{
			setSpecialistCount((SpecialistTypes) pCitizen->iIndex, pCitizen->iCount);
		}
		// non-specialists
		else
		{
			setWorkingPlot(pCitizen->iIndex, (pCitizen->iCount > 0 || pCitizen->iFreeCount > 0));
		}
	}

	if ((getOwnerINLINE() == GC.getGameINLINE().getActivePlayer()) && isCitySelected())
	{
		gDLL->getInterfaceIFace()->setDirty(CitizenButtons_DIRTY_BIT, true);
	}
#endif
}


int CvCityAI::AI_plotValue(CvPlot* pPlot)
{
	// do calc if needed
	if (!m_governorValues.bInitialized)
	{
		AI_calculateGovernorValues(m_governorValues);
	}

	return AI_plotValue(pPlot, m_governorValues);
}

int CvCityAI::AI_plotValue(CvPlot* pPlot, CvGovernorValues& governorValues)
{
	PROFILE_FUNC();
	
	// cache some stuff
	PlayerTypes ePlayer = getOwnerINLINE();
	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);

	// return result
	int iValue = 0;
	
	// store all the yield info here
	CvCompleteYield completeYield;
	for (int iYield = 0; iYield < NUM_YIELD_TYPES; iYield++)
	{
		// value food in a non-linear fasion unless food is production (first 2 85%, 2nd two 100%, all after that 115%)
		if (iYield == YIELD_FOOD && !isFoodProduction())
		{
			int iConsumtionPerPop = GC.getFOOD_CONSUMPTION_PER_POPULATION();

			int iFoodYield = pPlot->getYield(YIELD_FOOD);
			int iLowestTier = min(iFoodYield, iConsumtionPerPop);
			int iMiddleTier = min(iFoodYield - iLowestTier, iConsumtionPerPop);
			int iHighestTier = max(0, iFoodYield - iMiddleTier - iLowestTier);

			completeYield.aiYield[YIELD_FOOD] = (iLowestTier * 85) + (iMiddleTier * 100) + (iHighestTier * 115);
		}
		else
		{
			completeYield.aiYield[iYield] = 100 * pPlot->getYield((YieldTypes) iYield);
		}
	}
	for (int iCommerce = 0; iCommerce < NUM_COMMERCE_TYPES; iCommerce++)
	{
		completeYield.aiCommerce[iCommerce] = 0;
	}
	completeYield.iGPPRate = 0;
	completeYield.iBonusExperience = 0;

	// add some value if there is improvement that can upgrade
	int iTotalDiff = 0;
	ImprovementTypes eCurrentImprovement = pPlot->getImprovementType();
	if (eCurrentImprovement != NO_IMPROVEMENT)
	{
		//in the case that improvements upgrade, use 50% of the next 2 upgrade levels for the yield calculations.
		ImprovementTypes eUpgradeImprovement = (ImprovementTypes)GC.getImprovementInfo(eCurrentImprovement).getImprovementUpgrade();
		if (eUpgradeImprovement != NO_IMPROVEMENT)
		{
			ImprovementTypes eUpgradeImprovement2 = (ImprovementTypes)GC.getImprovementInfo(eUpgradeImprovement).getImprovementUpgrade();
			if (eUpgradeImprovement2 != NO_IMPROVEMENT)
			{
				eUpgradeImprovement = eUpgradeImprovement2;				
			}
		}
		ImprovementTypes eFinalImprovement = eUpgradeImprovement;
		
		if ((eFinalImprovement != NO_IMPROVEMENT) && (eFinalImprovement != eCurrentImprovement))
		{
			for (int iYield = 0; iYield < NUM_YIELD_TYPES; iYield++)
			{
				int iYieldDiff = (pPlot->calculateImprovementYieldChange(eFinalImprovement, ((YieldTypes) iYield), ePlayer) - pPlot->calculateImprovementYieldChange(eCurrentImprovement, ((YieldTypes) iYield), ePlayer));
				completeYield.aiYield[iYield] += 50 * iYieldDiff;
				iTotalDiff += iYieldDiff;
			}
		}
	}
	
	
	int iYieldValue = AI_completeYieldValue(completeYield, governorValues);
#ifdef WE_DONT_NEED_THIS_DO_WE
	// unless we are emph food (and also food not production)
	if (!(AI_isEmphasizeYield(YIELD_FOOD) && !isFoodProduction()))
	{
		// if this plot is super bad (less than 2 food and less than combined 2 prod/commerce
		if (!AI_potentialPlot(aiYields))
		{
			// undervalue it even more!
			iYieldValue /= 16;
		}
	}
#endif
	iValue += iYieldValue;
	
	// add value for improvements that can spawn a bonus resource
	if (eCurrentImprovement != NO_IMPROVEMENT)
	{
		TeamTypes eTeam = kPlayer.getTeam();
		CvTeam& kTeam = GET_TEAM(eTeam);

		if (pPlot->getBonusType(eTeam) == NO_BONUS) // XXX double-check CvGame::doFeature that the checks are the same...
		{
			for (int iI = 0; iI < GC.getNumBonusInfos(); iI++)
			{
				if (kTeam.isHasTech((TechTypes)(GC.getBonusInfo((BonusTypes) iI).getTechReveal())))
				{
					if (GC.getImprovementInfo(eCurrentImprovement).getImprovementBonusDiscoverRand(iI) > 0)
					{
						iValue += 32;
					}
				}
			}
		}
	}
	
	// adjust value based on upgrade time
	if ((eCurrentImprovement != NO_IMPROVEMENT) && (GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementUpgrade() != NO_IMPROVEMENT))
	{
		// all improvements that can upgrade get a boost
		iValue += 128;

		int iUpgradeTime = (GC.getGameINLINE().getImprovementUpgradeTime(eCurrentImprovement));
		if (iUpgradeTime > 0) //assert this?
		{
			int iUpgradePenalty = (128 * (iUpgradeTime - pPlot->getUpgradeProgress()));
			iUpgradePenalty *= (iTotalDiff * 4);
			iUpgradePenalty /= max(1, GC.getGameSpeedInfo(GC.getGame().getGameSpeedType()).getImprovementPercent());
			iValue -= iUpgradePenalty;
		}		
	}

	return iValue;
}

int CvCityAI::AI_specialistValue(SpecialistTypes eSpecialist)
{
	// do calc if needed
	if (!m_governorValues.bInitialized)
	{
		AI_calculateGovernorValues(m_governorValues);
	}

	return AI_specialistValue(eSpecialist, m_governorValues);
}

int CvCityAI::AI_specialistValue(SpecialistTypes eSpecialist, CvGovernorValues& governorValues)
{
	PROFILE_FUNC();

	// cache some stuff
	PlayerTypes ePlayer = getOwnerINLINE();
	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);

	// store all the yield info here
	CvCompleteYield completeYield;
	for (int iYield = 0; iYield < NUM_YIELD_TYPES; iYield++)
	{
		completeYield.aiYield[iYield] = 100 * kPlayer.specialistYield(eSpecialist, ((YieldTypes) iYield));
	}
	for (int iCommerce = 0; iCommerce < NUM_COMMERCE_TYPES; iCommerce++)
	{
		completeYield.aiCommerce[iCommerce] = 100 * kPlayer.specialistCommerce(eSpecialist, ((CommerceTypes) iCommerce));
	}
	completeYield.iGPPRate = 100 * GC.getSpecialistInfo(eSpecialist).getGreatPeopleRateChange();
	completeYield.iBonusExperience = 100 * GC.getSpecialistInfo(eSpecialist).getExperience();

	// devalue GPPRate for humans if the GPPRate for this type is zero in this city
	if (completeYield.iGPPRate != 0)
	{
		UnitTypes iGreatPeopleType = (UnitTypes) GC.getSpecialistInfo(eSpecialist).getGreatPeopleUnitClass();
		if (iGreatPeopleType != NO_UNIT && isHuman() && (getGreatPeopleUnitRate(iGreatPeopleType) == 0) && (getForceSpecialistCount(eSpecialist) == 0))
		{
			completeYield.iGPPRate /= 4;			
		}
	}

	int iValue = AI_completeYieldValue(completeYield, governorValues);
    
	// extra value to spawn some special gp for this city
	if (completeYield.iGPPRate != 0)
	{
		if (!isHuman() && (kPlayer.getCurrentEra() <= (GC.getNumEraInfos() / 2)))
        {
            // try to spawn a prophet for any shrines we have yet to build
            bool bNeedProphet = false;
            int iBestSpreadValue = 0;
			
			for (int iJ = 0; iJ < GC.getNumReligionInfos(); iJ++)
            {
                ReligionTypes eReligion = (ReligionTypes) iJ;

                if (isHolyCity(eReligion) && !hasShrine(eReligion))
                {
					CvCivilizationInfo* pCivilizationInfo = &GC.getCivilizationInfo(getCivilizationType());
					
					int iUnitClass = GC.getSpecialistInfo(eSpecialist).getGreatPeopleUnitClass();
                    FAssert(iUnitClass != NO_UNITCLASS);
                    
					UnitTypes eGreatPeopleUnit = (UnitTypes) pCivilizationInfo->getCivilizationUnits(iUnitClass);
					if (eGreatPeopleUnit != NO_UNIT)
					{
						// note, for normal XML, this count will be one (there is only 1 shrine building for each religion)
						int	shrineBuildingCount = GC.getGameINLINE().getShrineBuildingCount(eReligion);
						for (int iI = 0; iI < shrineBuildingCount; iI++)
						{
							int eBuilding = (int) GC.getGameINLINE().getShrineBuilding(iI, eReligion);
							
							// if this unit builds or forceBuilds this building
							if (GC.getUnitInfo(eGreatPeopleUnit).getBuildings(eBuilding) || GC.getUnitInfo(eGreatPeopleUnit).getForceBuildings(eBuilding))
							{
								bNeedProphet = true;
								iBestSpreadValue = max(iBestSpreadValue, GC.getGameINLINE().countReligionLevels(eReligion));
							}
						}
				
					}
				}
			}

			if (bNeedProphet)
            {

				iValue += (iBestSpreadValue * completeYield.iGPPRate * governorValues.yieldValues.iGPPRate * governorValues.yieldModifiers.iGPPRate) / 100;
            }
		}
	}
#ifdef IS_THIS_STILL_NEEDED
	// specialist has no GPP rate, probably generic citizen, which we devalue
	else
	{
		SpecialistTypes eGenericCitizen = (SpecialistTypes) GC.getDefineINT("DEFAULT_SPECIALIST");
		
		// are we the generic specialist?
		if (eSpecialist == eGenericCitizen)
		{
			iValue *= 60;
			iValue /= 100;
		}
	}
#endif
	
	return iValue;
}

int CvCityAI::AI_completeYieldValue(CvCompleteYield& completeYield)
{
	return AI_completeYieldValue(completeYield, m_governorValues);
}

int CvCityAI::AI_completeYieldValue(CvCompleteYield& completeYield, CvGovernorValues& governorValues)
{
	PROFILE_FUNC();

	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());
	
	// modify yields by governorValues.yieldModifiers / 100
	for (int iYield = 0; iYield < NUM_YIELD_TYPES; iYield++)
	{
		completeYield.aiYield[iYield] *= governorValues.yieldModifiers.aiYield[iYield];
		completeYield.aiYield[iYield] /= 100;
	}
	
	completeYield.iGPPRate *= governorValues.yieldModifiers.iGPPRate;
	completeYield.iGPPRate /= 100;

	completeYield.iBonusExperience *= governorValues.yieldModifiers.iBonusExperience;
	completeYield.iBonusExperience /= 100;

	// convert the commerce to its components before doing the governorValues.yieldModifiers.aiCommerce modification
	int iTotalCommerce = completeYield.aiYield[YIELD_COMMERCE];
	if (iTotalCommerce != 0)
	{
		for (int iCommerce = 0; iCommerce < NUM_COMMERCE_TYPES; iCommerce++)
		{
			completeYield.aiCommerce[iCommerce] += ((iTotalCommerce * kPlayer.getCommercePercent((CommerceTypes) iCommerce))) / 100;
		}

		completeYield.aiYield[YIELD_COMMERCE] = 0;
	}
	
	// now modify commerce yields by governorValues.yieldModifiers.aiCommerce / 100
	for (int iCommerce = 0; iCommerce < NUM_COMMERCE_TYPES; iCommerce++)
	{
		completeYield.aiCommerce[iCommerce] *= governorValues.yieldModifiers.aiCommerce[iCommerce];
		completeYield.aiCommerce[iCommerce] /= 100;
	}
	
	// if production is process, convert it now (post modifiers)
	int iProduction = completeYield.aiYield[YIELD_PRODUCTION];
	if (iProduction != 0)
	{
		if (isProductionProcess())
		{
			completeYield.aiYield[YIELD_PRODUCTION] = 0;
			
			CvProcessInfo& kProcessInfo = GC.getProcessInfo(getProductionProcess());
			for (int iCommerce = 0; iCommerce < NUM_COMMERCE_TYPES; iCommerce++)
			{
				completeYield.aiCommerce[iCommerce] += (iProduction * kProcessInfo.getProductionToCommerceModifier(iCommerce)) / 100;
			}
		}
	}			

	// calculate the value
	int iValue = 0;

	for (int iYield = 0; iYield < NUM_YIELD_TYPES; iYield++)
	{
		iValue += completeYield.aiYield[iYield] * governorValues.yieldValues.aiYield[iYield];
	}

	// only count food over the citizen cost for slavery (rough approximation of food that will put us over break even)
	int iConsumtionPerPop = GC.getFOOD_CONSUMPTION_PER_POPULATION();
	iValue += max(0, completeYield.aiYield[YIELD_FOOD] - iConsumtionPerPop) * governorValues.iExtraFoodValue;

	for (int iCommerce = 0; iCommerce < NUM_COMMERCE_TYPES; iCommerce++)
	{
		iValue += completeYield.aiCommerce[iCommerce] * governorValues.yieldValues.aiCommerce[iCommerce];
	}

	iValue += completeYield.iGPPRate * governorValues.yieldValues.iGPPRate;
	iValue += completeYield.iBonusExperience * governorValues.yieldValues.iBonusExperience;
	
	return iValue;
}

#endif	// NEW_CITY_GOVERNOR

#ifdef COMPARE_NEW_CITY_GOVERNOR
void CvCityAI::AI_compareGovernorCitizens()
{
#ifdef FASSERT_ENABLE
	std::set<int> differentPlots;
	std::set<int> differentSpecialists;

	bool bIsFoodProduction = isFoodProduction();

	// compare
	int iDifferentPlots = 0;
	int iDifferentSpecialists = 0;
	int iFoodDifference = 0;
	int iProductionDelta = 0;
	int iCommerceDelta = 0;
	for (CvCitizensIterator it = m_governorCitizens.begin(); it != m_governorCitizens.end(); ++it)
	{
		CvGovernorCitizen* pCitizen = getCitizen(it);

		// specialist
		if (pCitizen->bIsSpecialist)
		{
			if (getSpecialistCount((SpecialistTypes) pCitizen->iIndex) != pCitizen->iCount)
			{
				if (m_lastDifferentSpecialists.find(pCitizen->iIndex) == m_lastDifferentSpecialists.end())
				{
					iDifferentSpecialists++;
				}

				differentSpecialists.insert(pCitizen->iIndex);
			}
		}
		// plot
		else
		{
			int iPlotDelta = (pCitizen->iCount + pCitizen->iFreeCount) - (isWorkingPlot(pCitizen->iIndex) ? 1 : 0);
			if (iPlotDelta != 0)
			{
				CvPlot* pPlot = getCityIndexPlot(pCitizen->iIndex);
				if (pPlot != NULL)
				{
					iFoodDifference += bIsFoodProduction ? 0 : (iPlotDelta * pPlot->getYield(YIELD_FOOD));
					iProductionDelta += iPlotDelta * ((bIsFoodProduction ? pPlot->getYield(YIELD_FOOD) : 0) + pPlot->getYield(YIELD_PRODUCTION));
					iCommerceDelta += iPlotDelta * pPlot->getYield(YIELD_COMMERCE);
				}
				
				if (m_lastDifferentPlots.find(pCitizen->iIndex) == m_lastDifferentPlots.end())
				{
					iDifferentPlots++;
				}

				differentPlots.insert(pCitizen->iIndex);
			}
		}
	}

	m_lastDifferentPlots = differentPlots;
	m_lastDifferentSpecialists = differentSpecialists;

	TCHAR szAssertMessage[1024];
	sprintf(szAssertMessage, "%S assigned different plots (%d) [%d,%d,%d] and/or specialists (%d)", getName().GetCString(), iDifferentPlots, iFoodDifference, iProductionDelta, iCommerceDelta, iDifferentSpecialists);
	FAssertMsg((iDifferentPlots == 0 || (iFoodDifference == 0 && iProductionDelta == 0 && iCommerceDelta == 0)) && (iDifferentSpecialists == 0), szAssertMessage);

#endif	// FASSERT_ENABLE
}
#endif	// COMPARE_NEW_CITY_GOVERNOR

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
		int iExtra = (isHuman()) ? 0 : 1;
		int iHappinessLevel = happyLevel() - unhappyLevel(iExtra);
		
		// ignore military unhappy, since we assume it will be fixed quickly, grow anyway
		if (getMilitaryHappinessUnits() == 0)
		{
			iHappinessLevel += ((GC.getDefineINT("NO_MILITARY_PERCENT_ANGER") * (getPopulation() + 1)) / GC.getDefineINT("PERCENT_ANGER_DIVISOR"));
		}
		
		// if we can pop rush, we want to grow one over the cap
		if (GET_PLAYER(getOwnerINLINE()).canPopRush())
		{
			iHappinessLevel++;
		}

		
		// if we have angry citizens
		if (iHappinessLevel < 0)
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

#ifdef USE_NEW_CITY_GOVERNOR
	return AI_specialistValue(eSpecialist);
#endif

	short aiYields[NUM_YIELD_TYPES];
	int iTempValue;
	int iCommerce;
	int iGreatPeopleRate;
	int iValue;
	int iI, iJ;
	int iNumCities = GET_PLAYER(getOwnerINLINE()).getNumCities();
	int iModifier;

    int iCultureModifier = 1;
	if (GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_CULTURE3))
	{
		iCultureModifier = 5;
	}
	else if (GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_CULTURE2))
	{
		iCultureModifier = 3;
	}
    
	for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		aiYields[iI] = GET_PLAYER(getOwnerINLINE()).specialistYield(eSpecialist, ((YieldTypes)iI));
	}
	
//	if (AI_specialYieldMultiplier(YIELD_PRODUCTION) < 25)
//	{
//		if (aiYields[YIELD_PRODUCTION] > 0)
//		{
//			aiYields[YIELD_PRODUCTION] /= iCultureModifier;
//		}
//	}

	iValue = AI_yieldValue(aiYields, bAvoidGrowth, bRemove);
    
	for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		iCommerce = GET_PLAYER(getOwnerINLINE()).specialistCommerce(eSpecialist, ((CommerceTypes)iI));

		if (iCommerce != 0)
		{
		    iModifier = 100;
			iModifier = getTotalCommerceRateModifier((CommerceTypes) iI);
			
			iTempValue = iCommerce * 6;
			iTempValue *= iModifier;
			iTempValue *= GET_PLAYER(getOwnerINLINE()).AI_averageCommerceExchange((CommerceTypes)iI);
			iTempValue /= 10000;

			int iCommerceWeight = GET_PLAYER(getOwnerINLINE()).AI_commerceWeight(((CommerceTypes)iI), this);
			
			// if this commerce type is specifically emphasized
			bool bIsEmphasizeThisCommerceType = AI_isEmphasizeCommerce((CommerceTypes)iI);
			if (bIsEmphasizeThisCommerceType)
			{
				iTempValue += (iCommerce * 40);
			}
			
			// if emp all commerce, then value based on the player's commerce percent
			if (AI_isEmphasizeYield(YIELD_COMMERCE))
			{
				int iCommercePercent = GET_PLAYER(getOwnerINLINE()).getCommercePercent((CommerceTypes) iI);

				// do not count twice, if emph both this commerce type and general commerce
				if (!bIsEmphasizeThisCommerceType)
				{
					iTempValue += ((iCommerce * 40) * iCommercePercent) / 100;
				}
				
				// if the weight is so low, its less than half this commerce slider value, use a high percentage instead
				// (otherwise, artists never get picked)
				if (iCommerceWeight < iCommercePercent / 2)
				{
					iCommerceWeight = iCommercePercent + 25;
				}
			}
			
			if ((CommerceTypes)iI == COMMERCE_CULTURE)
			{
			    if (iCultureModifier == 1)
			    {
                    int iCulturePercent = calculateCulturePercent(getOwnerINLINE());
					
					// boost artists if our culture is under 55% (but do not lower them if its over that)
					if (iCulturePercent < 55)
					{
						iTempValue *= (2 * (105 - iCulturePercent));
						iTempValue /= 100;
					}
					if (iCulturePercent > 95)
					{
						iTempValue = 0;						
					}
			    }
			}
			else
			{
			    iTempValue /= iCultureModifier;
			}
			
			iTempValue *= iCommerceWeight;
			iTempValue /= 100;
			//since multipliers have been allowed back in...
			iTempValue *= (100 + AI_specialYieldMultiplier(YIELD_COMMERCE));
			iTempValue /= 100;

			iValue += iTempValue;
		}
	}

	iGreatPeopleRate = GC.getSpecialistInfo(eSpecialist).getGreatPeopleRateChange();

	if (iGreatPeopleRate != 0)
	{
		int iGPPValue = 4;
		
		
		//iGreatPeopleRate = ((iGreatPeopleRate * getTotalGreatPeopleRateModifier()) / 100);
		UnitTypes iGreatPeopleType = (UnitTypes)GC.getSpecialistInfo(eSpecialist).getGreatPeopleUnitClass();
		
		iTempValue = (iGreatPeopleRate * 4);
		
		if (isHuman() && (getGreatPeopleUnitRate(iGreatPeopleType) == 0)
			&& (getForceSpecialistCount(eSpecialist) == 0))
		{
			iTempValue -= (iGreatPeopleRate * 4);			
		}

		if (!isHuman())
		{
			int iProgress = getGreatPeopleProgress();
			if (iProgress > 0)
			{
				int iThreshold = GET_PLAYER(getOwnerINLINE()).greatPeopleThreshold();
				iTempValue += ((iGreatPeopleRate * 16 * iProgress * iProgress) / (iThreshold * iThreshold));
			}
		}

		if (AI_isEmphasizeGreatPeople())
		{
			iTempValue += (iGreatPeopleRate * 20);
		}

        if (!isHuman() && (GET_PLAYER(getOwnerINLINE()).getCurrentEra() <= (GC.getNumEraInfos() / 2)))
        {
            // try to spawn a prophet for any shrines we have yet to build
            bool bNeedProphet = false;
            int iBestSpreadValue = 0;

			
			for (iJ = 0; iJ < GC.getNumReligionInfos(); iJ++)
            {
                ReligionTypes eReligion = (ReligionTypes) iJ;

                if (isHolyCity(eReligion) && !hasShrine(eReligion))
                {
					CvCivilizationInfo* pCivilizationInfo = &GC.getCivilizationInfo(getCivilizationType());
					
					int iUnitClass = GC.getSpecialistInfo(eSpecialist).getGreatPeopleUnitClass();
                    FAssert(iUnitClass != NO_UNITCLASS);
                    
					UnitTypes eGreatPeopleUnit = (UnitTypes) pCivilizationInfo->getCivilizationUnits(iUnitClass);
					if (eGreatPeopleUnit != NO_UNIT)
					{
						// note, for normal XML, this count will be one (there is only 1 shrine building for each religion)
						int	shrineBuildingCount = GC.getGameINLINE().getShrineBuildingCount(eReligion);
						for (int iI = 0; iI < shrineBuildingCount; iI++)
						{
							int eBuilding = (int) GC.getGameINLINE().getShrineBuilding(iI, eReligion);
							
							// if this unit builds or forceBuilds this building
							if (GC.getUnitInfo(eGreatPeopleUnit).getBuildings(eBuilding) || GC.getUnitInfo(eGreatPeopleUnit).getForceBuildings(eBuilding))
							{
								bNeedProphet = true;
								iBestSpreadValue = max(iBestSpreadValue, GC.getGameINLINE().countReligionLevels(eReligion));
							}
						}
				
					}
				}
			}

			if (bNeedProphet)
            {
                iTempValue += ((iGreatPeopleRate * iBestSpreadValue));
                if (AI_isEmphasizeGreatPeople())
                {
                    iTempValue += (iGreatPeopleRate * iBestSpreadValue);
                }
            }
		}
		iTempValue *= 100;
		iTempValue /= GET_PLAYER(getOwnerINLINE()).AI_averageGreatPeopleMultiplier();
		
		iValue += iTempValue;
	}

	else
	{
		SpecialistTypes eGenericCitizen = (SpecialistTypes) GC.getDefineINT("DEFAULT_SPECIALIST");
		
		// are we the generic specialist?
		if (eSpecialist == eGenericCitizen)
		{
			iValue *= 60;
			iValue /= 100;
		}
	}
	
	int iExperience = GC.getSpecialistInfo(eSpecialist).getExperience();
	if (0 != iExperience)
	{
		int iProductionRank = findYieldRateRank(YIELD_PRODUCTION);
		int iHasMetCount = GET_TEAM(getTeam()).getHasMetCivCount(true);

		iValue += (iExperience * ((iHasMetCount > 0) ? 4 : 2));
		if (iProductionRank <= iNumCities/2 + 1)
		{
			iValue += iExperience *  4;
		}
		iValue += ((getMilitaryProductionModifier() * iExperience * 8) / 100);
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
	bool bDefenseWar;
	bool bPrimaryArea;
	bool bFinancialTrouble;
	bool bDanger;
	bool bChooseUnit;
	int iProductionRank;
	int iNeededMissionaries;
	int iCulturePressure;
	int iI;

	bDanger = AI_isDanger();

	if (isProduction())
	{
		if (getProduction() > 0)
		{
			// if less than 3 turns left, keep building current item
			if (getProductionTurnsLeft() <= 3)
			{
				return;
			}
			
			// if building a combat unit, and we have no defenders, keep building it
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
			
			// if we are building a wonder, do not cancel, keep building it (if no danger)
			BuildingTypes eProductionBuilding = getProductionBuilding();
			if (!bDanger && eProductionBuilding != NO_BUILDING && 
				isLimitedWonderClass((BuildingClassTypes) GC.getBuildingInfo(eProductionBuilding).getBuildingClassType()))
			{
				return;
			}
			
			//if we are killing our growth to train this, then finish it.
			if (!bDanger && isFoodProduction())
			{
				return;
			}
		}

		clearOrderQueue();
	}
	
	// only clear the dirty bit if we actually do a check, multiple items might be queued
	AI_setChooseProductionDirty(false);

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

    if (isHuman() && isProductionAutomated())
    {
        AI_buildGovernorChooseProduction();
        return;
    }

	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());
	
	pWaterArea = waterArea();

	bWasFoodProduction = isFoodProduction();
	bHasMetHuman = GET_TEAM(getTeam()).hasMetHuman();
	bLandWar = ((area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE) || (area()->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE) || (area()->getAreaAIType(getTeam()) == AREAAI_MASSING));
	bDefenseWar = (area()->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE);
	bAssault = (area()->getAreaAIType(getTeam()) == AREAAI_ASSAULT);
	bPrimaryArea = kPlayer.AI_isPrimaryArea(area());
	bFinancialTrouble = kPlayer.AI_isFinancialTrouble();
	iCulturePressure = AI_calculateCulturePressure();
	int iNumCitiesInArea = area()->getCitiesPerPlayer(getOwnerINLINE());
	bool bImportantCity = false; //be very careful about setting this.
	bool bBigCultureCity = false;
	int iCultureRateRank = findCommerceRateRank(COMMERCE_CULTURE);
    int iCulturalVictoryNumCultureCities = GC.getGameINLINE().culturalVictoryNumCultureCities();
    
    bool bGetBetterUnits = GET_PLAYER(getOwner()).AI_isDoStrategy(AI_STRATEGY_GET_BETTER_UNITS);
    bool bAggressiveAI = GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI);

	int iUnitCostPercentage = (kPlayer.calculateUnitCost() * 100) / max(1, kPlayer.calculatePreInflatedCosts());

	int iSeaAssaultUnits = -1;
	int iBuildUnitProb = AI_buildUnitProb();
    
    int iExistingWorkers = kPlayer.AI_totalAreaUnitAIs(area(), UNITAI_WORKER);
    int iNeededWorkers = kPlayer.AI_neededWorkers(area());
    
    int iAreaBestFoundValue = area()->getBestFoundValue(getOwnerINLINE());
    
	int iEconomyFlags = 0;
	iEconomyFlags |= BUILDINGFOCUS_GOLD;
	iEconomyFlags |= BUILDINGFOCUS_RESEARCH;
	iEconomyFlags |= BUILDINGFOCUS_MAINTENANCE;
	iEconomyFlags |= BUILDINGFOCUS_HAPPY;
	iEconomyFlags |= BUILDINGFOCUS_HEALTHY;
	iEconomyFlags |= BUILDINGFOCUS_SPECIALIST;

	if (iNumCitiesInArea > 2)
	{	
		if (kPlayer.AI_isDoStrategy(AI_STRATEGY_CULTURE2))
		{
			if (iCultureRateRank <= iCulturalVictoryNumCultureCities + 1)
			{
				bBigCultureCity = true;

				// if we do not have enough cities, then the highest culture city will not get special attention
				if (iCultureRateRank > 1 || (kPlayer.getNumCities() > (iCulturalVictoryNumCultureCities + 1)))
				{
					bImportantCity = true;
				}
            }
        }
	}

	iProductionRank = findYieldRateRank(YIELD_PRODUCTION);

	int iBaseGoldRate = getBaseCommerceRate(COMMERCE_GOLD);
	int iBaseResearchRate = getBaseCommerceRate(COMMERCE_RESEARCH);

	bChooseUnit = false;
	
	int iWaterPercent = AI_calculateWaterWorldPercent();
	
#ifdef DEBUG_OUT_OF_SYNCS
	if (GC.getLogging())
	{
		TCHAR szOut[1024];
		sprintf(szOut, "%S(%S)[%d]: fp=%d, lw=%d, as=%d, dw=%d, pa=%d, da=%d, pr=%d, cp=%d, "
			"ca=%d, ic=%d, bc=%d, cr=%d, cv=%d, bu=%d, ag=%d, ucp=%d, ew=%d, nw=%d, bgr=%d, brr=%d, wp=%d\n\n", 
			getName().GetCString(),
			kPlayer.getName(),
			GC.getGameINLINE().getTurnSlice(),
			bWasFoodProduction,
			bLandWar,
			bAssault,
			bDefenseWar,
			bPrimaryArea,
			bFinancialTrouble,
			bDanger,
			iProductionRank,
			iCulturePressure,

			iNumCitiesInArea,
			bImportantCity,
			bBigCultureCity,
			iCultureRateRank ,
			iCulturalVictoryNumCultureCities,
			bGetBetterUnits,
			bAggressiveAI,
			iUnitCostPercentage,
			iExistingWorkers,
			iNeededWorkers,
			iBaseGoldRate,
			iBaseResearchRate,
			iWaterPercent);
		gDLL->messageControlLog(szOut);
	}
#endif
	
	clearOrderQueue();

	if (bWasFoodProduction)
	{
		AI_assignWorkingPlots();
	}
	
    // if we need to pop borders, then do that immediately if we have drama and can do it
	if (getCultureLevel() <= (CultureLevelTypes) 1)
	{
        if (AI_chooseProcess(COMMERCE_CULTURE))
        {
            return;
        }
	}

	// if we just captured this city, dont build a unit first
	if (isOccupation())
	{
		// pick granary or lighthouse, any duration
		if (AI_chooseBuilding(BUILDINGFOCUS_FOOD))
		{
			return;
		}

		// try picking forge, etc, any duration
		if (AI_chooseBuilding(BUILDINGFOCUS_PRODUCTION))
		{
			return;
		}
		
		// just pick any building, any duration
		if (AI_chooseBuilding())
		{
			return;
		}
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
	
	if (isBarbarian())
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
    
    //do a check for one tile island type thing?
    //this can be overridden by "wait and grow more"
    if (!bDanger && (iExistingWorkers == 0) && isCapital())
    {
		if ((AI_countNumBonuses(NO_BONUS, /*bIncludeOurs*/ true, /*bIncludeNeutral*/ true, -1, /*bLand*/ true, /*bWater*/ false) > 0) || 
			(getPopulation() > 3))
    	{
			if (AI_chooseUnit(UNITAI_WORKER))
			{
				return;
			}
    	}
		if (AI_neededSeaWorkers() > 0)
		{
			if (AI_chooseUnit(UNITAI_WORKER_SEA))
			{
				return;
			}
		}
    }
    
    if (AI_chooseBuilding(BUILDINGFOCUS_EXPERIENCE, 6, 33))
    {
    	return;
    }
	

	if (isHuman())
	{
		if (kPlayer.AI_totalUnitAIs(UNITAI_ATTACK) < 2)
		{
			if (AI_chooseUnit(UNITAI_ATTACK))
			{
				return;
			}
		}
	}
	else
	{
		//minimal defense.
		if (plot()->plotCount(PUF_isUnitAIType, UNITAI_CITY_DEFENSE, -1, getOwnerINLINE()) <= plot()->plotCount(PUF_isUnitAIType, UNITAI_SETTLE, -1, getOwnerINLINE()))
		{
			if (AI_chooseUnit(UNITAI_CITY_DEFENSE))
			{
				return;
			}
		}
	}
	
	int iNeededFloatingDefenders = isBarbarian() ?  0 : GET_PLAYER(getOwner()).AI_getTotalFloatingDefendersNeeded(area());
 	int iTotalFloatingDefenders = isBarbarian() ? 0 : GET_PLAYER(getOwner()).AI_getTotalFloatingDefenders(area());
	
	if (iTotalFloatingDefenders < ((iNeededFloatingDefenders + 1) / (bGetBetterUnits ? 3 : 2)))
	{
		if (AI_chooseFloatingDefender(iNeededFloatingDefenders))
		{
			return;
		}
	}

	if (!(iExistingWorkers == 0) && !bImportantCity)
	{
        if (!bDanger && (iExistingWorkers < ((iNeededWorkers + 1) / 2)))
		{
			if (AI_chooseUnit(UNITAI_WORKER))
			{
				return;
			}
		}
	}
	
	    //do a check for one tile island type thing?
    //this can be overridden by "wait and grow more"
    if (bDanger && (iExistingWorkers == 0) && isCapital())
    {
	if ((AI_countNumBonuses(NO_BONUS, /*bIncludeOurs*/ true, /*bIncludeNeutral*/ true, -1, /*bLand*/ true, /*bWater*/ false) > 0) || 
		(getPopulation() > 3))
    	{
			if (AI_chooseUnit(UNITAI_WORKER))
			{
				return;
			}
    	}
		if (AI_neededSeaWorkers() > 0)
		{
			if (AI_chooseUnit(UNITAI_WORKER_SEA))
			{
				return;
			}
		}
    }

	if (pWaterArea != NULL)
	{
		if (kPlayer.AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_WORKER_SEA) == 0)
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

	if (AI_countNumBonuses(NO_BONUS, /*bIncludeOurs*/ false, /*bIncludeNeutral*/ true, /*iOtherCultureThreshold*/ -1, /*bLand*/ true, /*bWater*/ true) > 0)
	{
		if (getCommerceRate(COMMERCE_CULTURE) == 0)
		{
			AI_chooseBuilding(BUILDINGFOCUS_CULTURE, 30);
		}
	}
	
	//oppurunistic wonder build (1)
	if (!bDanger && (!hasActiveWorldWonder()))
	{
		int iWonderTime = GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getWonderConstructRand(), "Wonder Construction Rand");
		iWonderTime /= 5;
		iWonderTime += 7;
		if (AI_chooseBuilding(BUILDINGFOCUS_WORLDWONDER, iWonderTime))
		{
			if (kPlayer.AI_totalAreaUnitAIs(area(), UNITAI_RESERVE) <= iNumCitiesInArea)
			{
				AI_chooseUnit(UNITAI_RESERVE);
			}
			return;
		}
	}
	
	if ((iAreaBestFoundValue > (bDanger ? 2000 : 1400)) && !bFinancialTrouble)
	{
		if (kPlayer.AI_totalAreaUnitAIs(area(), UNITAI_SETTLE) <= (iNumCitiesInArea / 4))
		{
			if (AI_chooseUnit(UNITAI_SETTLE))
			{
				if (kPlayer.AI_totalAreaUnitAIs(area(), UNITAI_CITY_DEFENSE) <= iNumCitiesInArea)
				{
					if (!AI_chooseUnit(UNITAI_CITY_DEFENSE))
					{
						AI_chooseUnit(UNITAI_RESERVE);
					}
				}
				return;
			}
		}
	}
	
	//this is needed to build the cathedrals quickly
	//also very good for giving cultural cities first dibs on wonders
	//kind of a bug but with bImportantCity set it'll cause important cities
	//to snap up wonders - this wasn't intended logic but it works well and I
	//don't see the need to add a cultural victory check.
    if (bImportantCity && (iCultureRateRank <= iCulturalVictoryNumCultureCities))
    {
        if (iCultureRateRank == iCulturalVictoryNumCultureCities)
        {
            if (AI_chooseBuilding(BUILDINGFOCUS_BIGCULTURE | BUILDINGFOCUS_CULTURE | BUILDINGFOCUS_WONDEROK, 40))
            {
                return;
            }
        }
        
        else if (GC.getGameINLINE().getSorenRandNum(((iCultureRateRank == 1) ? 4 : 1) + iCulturalVictoryNumCultureCities * 2 , "AI Build up Culture") < iCultureRateRank)
        {
            if (AI_chooseBuilding(BUILDINGFOCUS_BIGCULTURE | BUILDINGFOCUS_CULTURE | BUILDINGFOCUS_WONDEROK, 40))
            {
                return;
            }
        }
    }

	// don't build frivilious things if this is an imporant city unless we at war
    if (!bImportantCity || bLandWar || bAssault)
    {
        if (bPrimaryArea)
        {
            if (kPlayer.AI_totalAreaUnitAIs(area(), UNITAI_ATTACK) == 0)
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
                if (kPlayer.AI_totalAreaUnitAIs(area(), UNITAI_EXPLORE) < (kPlayer.AI_neededExplorers(area())))
                {
                    if (AI_chooseUnit(UNITAI_EXPLORE))
                    {
                        return;
                    }
                }
            }
        }
        
        // if we already choose to build a sea transport, then finish the stack
		if (bAssault)
        {
			if (iSeaAssaultUnits == -1)
			{
				iSeaAssaultUnits = plot()->plotCount(PUF_isUnitAIType, UNITAI_ASSAULT_SEA, -1, getOwnerINLINE());
			}

			if (iSeaAssaultUnits > 0 && GC.getGameINLINE().getSorenRandNum(100, "AI Finish Sea Stack") < 33)
			{
				// if there are transports waiting on units, build a unit
				int iTransportGroupsNeedingUnits = kPlayer.AI_plotTargetMissionAIs(plot(), MISSIONAI_PICKUP);

				if (iTransportGroupsNeedingUnits > 0 && GC.getGameINLINE().getSorenRandNum(100, "AI Finish Sea Stack") < 50)
				{
					if (GC.getGameINLINE().getSorenRandNum(100, "AI Assault Choose Unit") < 70)
					{
						if (AI_chooseUnit(UNITAI_ATTACK_CITY))
						{
							return;
						}
					}

					if (AI_chooseUnit(UNITAI_COUNTER))
					{
						return;
					}
	                
					if (AI_chooseUnit(UNITAI_ATTACK))
					{
						return;
					}
				}
				
				if (AI_chooseBuilding(BUILDINGFOCUS_DOMAINSEA, 25))
				{
					return;
				}

				int iSeaEscortUnits = plot()->plotCount(PUF_isUnitAIType, UNITAI_ESCORT_SEA, -1, getOwnerINLINE());

				if (iSeaAssaultUnits > iSeaEscortUnits)
				{
                    if (AI_chooseUnit(UNITAI_ESCORT_SEA))
                    {
                        return;
                    }
				}

				if (GC.getGameINLINE().getSorenRandNum(100, "AI Sea Attack") < 25)
				{
					if (AI_chooseUnit(UNITAI_ATTACK_SEA))
					{
						return;
					}
				}

				if (iSeaAssaultUnits < 4)
				{
					if (AI_chooseUnit(UNITAI_ASSAULT_SEA))
					{
						return;
					}
				}
			}
        }

        if (bAssault && (GC.getGameINLINE().getSorenRandNum(100, "AI Choose Sea Unit") < iWaterPercent))
        {
            if (pWaterArea != NULL)
            {
                if (bPrimaryArea)
                {
					if (kPlayer.AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_ATTACK_SEA) == 0 || 
						GC.getGameINLINE().getSorenRandNum(100, "AI Sea Attack") < 10)
					{
						if (AI_chooseUnit(UNITAI_ATTACK_SEA))
						{
							AI_chooseBuilding(BUILDINGFOCUS_DOMAINSEA, 15);
							return;
						}
					}

                    if (kPlayer.AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_ASSAULT_SEA) == 0)
                    {
                        if (AI_chooseUnit(UNITAI_ASSAULT_SEA))
                        {
                            AI_chooseBuilding(BUILDINGFOCUS_DOMAINSEA, 15);
							return;
                        }
                    }

                    if (kPlayer.AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_ESCORT_SEA) < (kPlayer.AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_ASSAULT_SEA) + kPlayer.AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_CARRIER_SEA)))
                    {
                        if (AI_chooseUnit(UNITAI_ESCORT_SEA))
                        {
                            AI_chooseBuilding(BUILDINGFOCUS_DOMAINSEA, 15);
							return;
                        }
                    }
                }
            }
        }
        
        if (bDefenseWar)
        {
        	if (GC.getGameINLINE().getSorenRandNum(100, "AI Train Defensive Unit (Panic)") > (bGetBetterUnits ? 50 : 35))
        	{
        		switch (GC.getGameINLINE().getSorenRandNum(4, "AI Choose Defense Panic Unit"))
        		{
        			case 0:
					if (AI_chooseUnit(UNITAI_RESERVE))
					{
						break;
					}
        			
        			case 1:
					if (AI_chooseUnit(UNITAI_COUNTER))
					{
						break;
					}
					
					case 2:
					if (AI_chooseUnit(UNITAI_COLLATERAL))
					{
						break;
					}
					
					case 3:
					if (AI_chooseUnit(UNITAI_ATTACK))
					{
						break;
					}
					break;//lack of other breaks deliberate
        		}
        	}
        }
        
        if (AI_chooseBuilding(BUILDINGFOCUS_FOOD, 60))
        {
        	return;
        }
        
        //oppurunistic wonder build
        if (!bDanger && (!hasActiveWorldWonder() || (kPlayer.getNumCities() > 3)))
        {
        	int iWonderTime = GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getWonderConstructRand(), "Wonder Construction Rand");
        	iWonderTime /= 5;
        	iWonderTime += 8;
			if (AI_chooseBuilding(BUILDINGFOCUS_WORLDWONDER, iWonderTime))
			{
				return;
			}
        }
        
		//essential economic builds
		if (AI_chooseBuilding(iEconomyFlags, 10, 25))
		{
			return;
		}

        if (!bFinancialTrouble && !bGetBetterUnits && kPlayer.AI_isDoStrategy(AI_STRATEGY_DAGGER))
        {
            if (GC.getGameINLINE().getSorenRandNum(100, "AI Dagger Train Unit") > (bAggressiveAI ? 60 - iBuildUnitProb : 80 - iBuildUnitProb))
            {
                if (iUnitCostPercentage < 20)
                {
					if (iWaterPercent > 50 && !bLandWar)
					{
						if (GC.getGameINLINE().getSorenRandNum(50, "AI Dagger Sea Unit") < (iWaterPercent / 2))
						{
							if (GC.getGameINLINE().getSorenRandNum(100, "AI Sea Attack") < 10)
							{
								AI_chooseUnit(UNITAI_ATTACK_SEA);
							}
							else
							{
								AI_chooseUnit(UNITAI_ASSAULT_SEA);
							}
							AI_chooseBuilding(BUILDINGFOCUS_DOMAINSEA, 20);
						}
					}
					int iPillagerCount = (kPlayer.AI_totalAreaUnitAIs(area(), UNITAI_PILLAGE));
					if (iPillagerCount <= ((iNumCitiesInArea + 1) / 2))
					{
						if (GC.getGameINLINE().getSorenRandNum(iPillagerCount + 2, "AI Dagger Choose Pillager") == 0)
						{
							if (AI_chooseUnit(UNITAI_PILLAGE))
							{
								return;							
							}
						}
					}
					
					if (GC.getGameINLINE().getSorenRandNum(100, "AI Dagger Choose Unit") > 40)
					{
						if (AI_chooseUnit(UNITAI_ATTACK_CITY))
						{
							AI_chooseBuilding(BUILDINGFOCUS_EXPERIENCE, 20);
							return;
						}
					}
					
					if (AI_chooseUnit(UNITAI_COUNTER))
					{
						AI_chooseBuilding(BUILDINGFOCUS_EXPERIENCE, 20);
						return;
					}
					
					if (AI_chooseUnit(UNITAI_ATTACK_CITY))
					{
						AI_chooseBuilding(BUILDINGFOCUS_EXPERIENCE, 20);
						return;
					}
				}
            }
        }
	}

 	// don't build frivilious things if this is an imporant city
    if (!bImportantCity)
    {
       if (!bLandWar)
        {
            if (kPlayer.AI_totalAreaUnitAIs(area(), UNITAI_SETTLE) == 0)
            {
                if (!bFinancialTrouble)
                {
                    if (iAreaBestFoundValue > 0)
                    {
                        if (AI_chooseUnit(UNITAI_SETTLE))
                        {
                            return;
                        }
                    }
                }
            }
        }
    }
    
	if (kPlayer.AI_isDoStrategy(AI_STRATEGY_MISSIONARY) && (iNumCitiesInArea > 1))
	{
		ReligionTypes eStateReligion = kPlayer.getStateReligion();
		if ((eStateReligion != NO_RELIGION) && isHasReligion(eStateReligion))
		{
			if (GC.getGameINLINE().getSorenRandNum(100, "AI Build Missionary") < (bLandWar ? 25 : 50))
			{
				iNeededMissionaries = kPlayer.AI_neededMissionaries(area(), eStateReligion);
				if (iNeededMissionaries > 0)
				{
					if (iNeededMissionaries > kPlayer.countReligionSpreadUnits(area(), eStateReligion))
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
		
	if (iTotalFloatingDefenders < iNeededFloatingDefenders)
	{
		if (GC.getGameINLINE().getSorenRandNum(100, "AI Build Floating Defender") < 50)
		{
			if (AI_chooseFloatingDefender(iNeededFloatingDefenders))
			{
				return;
			}
		}
	}
	if (bLandWar)
	{
		if (kPlayer.AI_totalAreaUnitAIs(area(), UNITAI_SETTLE) == 0)
		{
			if (!bFinancialTrouble)
			{
				if (iAreaBestFoundValue  > 500)
				{
					if (AI_chooseUnit(UNITAI_SETTLE))
					{
						return;
					}
				}
			}
		}
	}
	if (!bLandWar)
	{		
		if ((iCulturePressure > 90) || kPlayer.AI_isDoStrategy(AI_STRATEGY_CULTURE2))
		{
			if (AI_chooseBuilding(BUILDINGFOCUS_CULTURE, 20))
			{
				return;
			}
		}

		if (pWaterArea != NULL)
		{
			if (bPrimaryArea && !bFinancialTrouble)
			{
				if (kPlayer.AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_SETTLER_SEA) == 0)
				{
					if (kPlayer.AI_totalAreaUnitAIs(area(), UNITAI_SETTLE) > 0)
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

			if (kPlayer.AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_MISSIONARY_SEA) == 0)
			{
				if (kPlayer.AI_totalAreaUnitAIs(area(), UNITAI_MISSIONARY) > 0)
				{
					if (AI_chooseUnit(UNITAI_MISSIONARY_SEA))
					{
						return;
					}
				}
			}
		}
	}
	
	if ((iProductionRank <= ((kPlayer.getNumCities() > 8) ? 3 : 2))
		&& (getPopulation() > 3))
	{
		int iWonderRand = 8 + GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getWonderConstructRand(), "Wonder Construction Rand");
		
		// increase chance of going for an early wonder
		if (GC.getGameINLINE().getElapsedGameTurns() < 100 && iNumCitiesInArea > 1)
		{
			iWonderRand *= 35;
			iWonderRand /= 100;
		}
		else if (iNumCitiesInArea >= 3)
		{
			iWonderRand *= 30;
			iWonderRand /= 100;
		}
		else
		{
			iWonderRand *= 25;
			iWonderRand /= 100;
		}
		
		if (bAggressiveAI)
		{
			iWonderRand *= 2;
			iWonderRand /= 3;
		}
		
		int iWonderRoll = GC.getGameINLINE().getSorenRandNum(100, "Wonder Build Rand");
		
		if (iWonderRoll < iWonderRand)
		{
			int iWonderMaxTurns = 20 + ((iWonderRand - iWonderRoll) * 2);
			if (bLandWar)
			{
				iWonderMaxTurns /= 2;
			}
			
			if (AI_chooseBuilding(BUILDINGFOCUS_WORLDWONDER, iWonderMaxTurns))
			{
#ifdef DEBUG_CITY_BUILDS
			DEBUGLOG("%S: picked wonder of %d normalized turns (%d/%d)\n", getName().GetCString(), iWonderMaxTurns, 
				iWonderRoll, iWonderRand);
#endif
			   return;
			}

#ifdef DEBUG_CITY_BUILDS
			DEBUGLOG("%S: failed to find wonder of %d normalized turns (%d/%d)\n", getName().GetCString(), iWonderMaxTurns, 
				iWonderRoll, iWonderRand);
#endif
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

	if (bDanger || (GC.getGameINLINE().getSorenRandNum(30, "AI Build Defense") < getPopulation()))
	{
	    if (!bDanger)
	    {
            if (AI_chooseBuilding(BUILDINGFOCUS_EXPERIENCE, 20))
            {
                return;
            }
	    }

		if (AI_chooseBuilding(BUILDINGFOCUS_DEFENSE, 20))
		{
			return;
		}
	}

	if (AI_chooseBuilding(BUILDINGFOCUS_PRODUCTION, 20, 4))
	{
		return;
	}

	if (iProductionRank <= ((kPlayer.getNumCities() / 3) + 1))
	{
		if (AI_chooseProject())
		{
			return;
		}
	}

    if (!bDanger)
	{
	    if  (kPlayer.AI_isDoStrategy(AI_STRATEGY_CULTURE2) || (GC.getGameINLINE().getSorenRandNum(100, "AI Build Missionary") < (isHolyCity() ? 30 : 15)))
	    {
            for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
            {
                if (isHasReligion((ReligionTypes)iI))
                {
                    iNeededMissionaries = kPlayer.AI_neededMissionaries(area(), ((ReligionTypes)iI));
                    if (iNeededMissionaries > 0)
                    {
                        if (iNeededMissionaries > kPlayer.countReligionSpreadUnits(area(), ((ReligionTypes)iI)))
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
	}
	
	
	//20 means 5g or ~2 happiness...
	if (AI_chooseBuilding(iEconomyFlags, 15, 20))
	{
		return;
	}


	if (!bLandWar)
	{
		if (AI_chooseBuilding(iEconomyFlags, 40, 8))
		{
			return;
		}

		if (iCulturePressure > 50)
		{
			if (AI_chooseBuilding(BUILDINGFOCUS_CULTURE, 60))
			{
				return;
			}
		}

		if (!bDanger)
		{
			if (countNumImprovedPlots() < (getPopulation() / 2))
			{
				if (plot()->plotCheck(PUF_isUnitAIType, UNITAI_WORKER, -1, getOwnerINLINE()) == NULL)
				{
					if (kPlayer.AI_neededWorkers(area()) > (kPlayer.AI_totalAreaUnitAIs(area(), UNITAI_WORKER) * 2))
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
				if (kPlayer.AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_EXPLORE_SEA) < min(1, kPlayer.AI_neededExplorers(pWaterArea)))
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

		if (kPlayer.AI_totalAreaUnitAIs(area(), UNITAI_SETTLE) == 0)
		{

			if (bPrimaryArea && (kPlayer.findBestFoundValue() > (bFinancialTrouble ? 2000 : 0)))
			{
				if (AI_chooseUnit(UNITAI_SETTLE))
				{
					return;
				}

			}
		}
	}

	if (iProductionRank <= ((kPlayer.getNumCities() / 2) + 1))
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

	// we do a similar check lower, in the landwar case
	if (!bLandWar && bFinancialTrouble)
	{
		if (AI_chooseBuilding(BUILDINGFOCUS_GOLD))
		{
			return;
		}
	}

	if (bPrimaryArea)
	{
		if (kPlayer.getNumCities() > 2)
		{
			if (kPlayer.AI_totalAreaUnitAIs(area(), UNITAI_RESERVE) == 0)
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
			if (iSeaAssaultUnits == -1)
			{
				iSeaAssaultUnits = plot()->plotCount(PUF_isUnitAIType, UNITAI_ASSAULT_SEA, -1, getOwnerINLINE());
			}

			int iUnitsToEscort = iSeaAssaultUnits + plot()->plotCount(PUF_isUnitAIType, UNITAI_CARRIER_SEA, -1, getOwnerINLINE());
			if (iUnitsToEscort > 0)
			{
				if (plot()->plotCount(PUF_isUnitAIType, UNITAI_ESCORT_SEA, -1, getOwnerINLINE()) < (2 * iUnitsToEscort))
				{
					if (AI_chooseUnit(UNITAI_ESCORT_SEA))
					{
						return;
					}
				}
			}

			if (GC.getGameINLINE().getSorenRandNum(100, "AI Sea Attack") < 8)
			{
				if (AI_chooseUnit(UNITAI_ATTACK_SEA))
				{
					return;
				}
			}
		}
	}

	if (GC.getGameINLINE().getSorenRandNum(10, "AI Build Spy") == 0)
	{
		if (kPlayer.AI_totalUnitAIs(UNITAI_SPY) < ((kPlayer.getNumCities() / 4) + 1))
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
			if (kPlayer.AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_SPY_SEA) == 0)
			{
				if (kPlayer.AI_totalAreaUnitAIs(area(), UNITAI_SPY) > 0)
				{
					if (AI_chooseUnit(UNITAI_SPY_SEA))
					{
						return;
					}
				}
			}
		}
	}

	if (iUnitCostPercentage < 10)
	{
		if ((bLandWar) ||
			  ((kPlayer.getNumCities() <= 3) && (GC.getGameINLINE().getElapsedGameTurns() < 60)) ||
			  (GC.getGameINLINE().getSorenRandNum(100, "AI Build Unit Production") < AI_buildUnitProb()) ||
				(isHuman() && (getGameTurnFounded() == GC.getGameINLINE().getGameTurn())))
		{
			if (AI_chooseUnit())
			{
				return;
			}

			bChooseUnit = true;
		}
	}

	if (AI_chooseProject())
	{
		return;
	}

	// we did the same check above if not landwar, do not repeat it here
	if (bLandWar && bFinancialTrouble)
	{
		if (AI_chooseBuilding(BUILDINGFOCUS_GOLD))
		{
			return;
		}
	}
	
	if (iUnitCostPercentage < 15)
	{
		if (!AI_isDefended())
		{
			if (AI_chooseUnit(UNITAI_CITY_DEFENSE))
			{
				return;
			}
		}
	}

	if (AI_chooseBuilding())
	{
		return;
	}
	
	if (!bChooseUnit)
	{
		if (iUnitCostPercentage < 20)
		{
			if (AI_chooseUnit())
			{
				return;
			}
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
	int iNumCitiesInArea = area()->getCitiesPerPlayer(getOwnerINLINE());

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

	aiUnitAIVal[UNITAI_CITY_DEFENSE] += (iNumCitiesInArea + 1);
	aiUnitAIVal[UNITAI_CITY_COUNTER] += ((5 * (iNumCitiesInArea + 1)) / 8);
	aiUnitAIVal[UNITAI_CITY_SPECIAL] += ((iNumCitiesInArea + 1) / 2);

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
	int iI, iJ, iK;
	
	FAssertMsg(eUnitAI != NO_UNITAI, "UnitAI is not assigned a valid value");
	
	bool bGrowMore = false;

	if (GET_PLAYER(getOwnerINLINE()).getNumCities() <= 2)
	{
		bGrowMore = ((getPopulation() < 3) && (AI_countGoodTiles(true, false, 100) >= getPopulation()));
	}
	else
	{
		bGrowMore = ((getPopulation() < 3) || (AI_countGoodTiles(true, false, 100) >= getPopulation()));
	}
	if (!bGrowMore && (getPopulation() < 6) && (AI_countGoodTiles(true, false, 80) >= getPopulation()))
	{
		if ((getFood() - (getFoodKept() / 2)) >= (growthThreshold() / 2))
		{
			if ((angryPopulation(1) == 0) && (healthRate(false, 1) == 0))
			{
				bGrowMore = true;                
			}
		}
	}
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
				    
					if (!(bGrowMore && isFoodProduction(eLoopUnit)))
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
				    
					if (!(bGrowMore && isFoodProduction(eLoopUnit)))
					{
						if (canTrain(eLoopUnit))
						{
							iValue = GET_PLAYER(getOwnerINLINE()).AI_unitValue(eLoopUnit, eUnitAI, area());

							if (iValue > ((iBestOriginalValue * 2) / 3))
							{
								iValue *= (getProductionExperience(eLoopUnit) + 10);
								iValue /= 10;

                                //free promotions. slow?
                                //only 1 promotion per source is counted (ie protective isn't counted twice)
                                int iPromotionValue = 0;
                                //buildings
                                for (iJ = 0; iJ < GC.getNumPromotionInfos(); iJ++)
                                {
                                    if (GC.getUnitInfo(eLoopUnit).getFreePromotions((PromotionTypes)iJ) || isFreePromotion((PromotionTypes)iJ))
                                    {
                                        if ((GC.getUnitInfo(eLoopUnit).getUnitCombatType() != NO_UNITCOMBAT) && GC.getPromotionInfo((PromotionTypes)iJ).getUnitCombat(GC.getUnitInfo(eLoopUnit).getUnitCombatType()))
                                        {
                                            iPromotionValue += 10;
                                            break;
                                        }
                                    }
                                }

                                //special to the unit
                                for (iJ = 0; iJ < GC.getNumPromotionInfos(); iJ++)
                                {
                                    if (GC.getUnitInfo(eLoopUnit).getFreePromotions(iJ))
                                    {
                                        iPromotionValue += 10;
                                        break;
                                    }
                                }

                                //traits
                                for (iJ = 0; iJ < GC.getNumTraitInfos(); iJ++)
                                {
                                    if (hasTrait((TraitTypes)iJ))
                                    {
                                        for (iK = 0; iK < GC.getNumPromotionInfos(); iK++)
                                        {
                                            if (GC.getTraitInfo((TraitTypes) iJ).isFreePromotion(iK))
                                            {
                                                if ((GC.getUnitInfo(eLoopUnit).getUnitCombatType() != NO_UNITCOMBAT) && GC.getTraitInfo((TraitTypes) iJ).isFreePromotionUnitCombat(GC.getUnitInfo(eLoopUnit).getUnitCombatType()))
                                                {
                                                    iPromotionValue += 10;
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }

                                iValue *= (iPromotionValue + 100);
                                iValue /= 100;

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
	return AI_bestBuildingThreshold(iFocusFlags, iMaxTurns, /*iMinThreshold*/ 0, bAsync, eIgnoreAdvisor);
}

BuildingTypes CvCityAI::AI_bestBuildingThreshold(int iFocusFlags, int iMaxTurns, int iMinThreshold, bool bAsync, AdvisorTypes eIgnoreAdvisor)
{
	BuildingTypes eLoopBuilding;
	BuildingTypes eBestBuilding;
	bool bAreaAlone;
	int iProductionRank;
	int iTurnsLeft;
	int iValue;
	int iTempValue;
	int iBestValue;
	int iI, iJ;

	bAreaAlone = GET_PLAYER(getOwnerINLINE()).AI_isAreaAlone(area());

	iProductionRank = findYieldRateRank(YIELD_PRODUCTION);

	iBestValue = 0;
	eBestBuilding = NO_BUILDING;

#ifdef DEBUG_CITY_BUILDS
	bool bFirst = true;
	CvWString szFocusString; 
	getBuildingFocusString(szFocusString, iFocusFlags);
	
	DEBUGLOG("%S: AI_bestBuilding(%S,%d): ", getName().GetCString(), szFocusString.GetCString(), 
		GC.getGameINLINE().AI_turnsPercent(iMaxTurns, GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getConstructPercent()));
#endif


	for (iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
	{
		if (!(GET_PLAYER(getOwnerINLINE()).isBuildingClassMaxedOut(((BuildingClassTypes)iI), GC.getBuildingClassInfo((BuildingClassTypes)iI).getExtraPlayerInstances())))
		{
			eLoopBuilding = ((BuildingTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iI)));

			if ((eLoopBuilding != NO_BUILDING) &&
			(!isProductionAutomated() || !(isWorldWonderClass((BuildingClassTypes)iI) || isNationalWonderClass((BuildingClassTypes)iI))))
			{
				//don't build wonders?
				if (((iFocusFlags == 0) || (iFocusFlags & BUILDINGFOCUS_WONDEROK) || (iFocusFlags & BUILDINGFOCUS_WORLDWONDER)) || 
					!(isWorldWonderClass((BuildingClassTypes)iI) || 
					 isTeamWonderClass((BuildingClassTypes)iI) || 
					  isNationalWonderClass((BuildingClassTypes)iI) ||
					   isLimitedWonderClass((BuildingClassTypes)iI)))
				{
					if ((eIgnoreAdvisor == NO_ADVISOR) || (GC.getBuildingInfo(eLoopBuilding).getAdvisorType() != eIgnoreAdvisor))
					{
						if (canConstruct(eLoopBuilding))
						{
							iValue = AI_buildingValueThreshold(eLoopBuilding, iFocusFlags, iMinThreshold);

							if (GC.getBuildingInfo(eLoopBuilding).getFreeBuildingClass() != NO_BUILDINGCLASS)
							{
								BuildingTypes eFreeBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(GC.getBuildingInfo(eLoopBuilding).getFreeBuildingClass());
								if (NO_BUILDING != eFreeBuilding)
								{
									iValue += (AI_buildingValue(eFreeBuilding, iFocusFlags) * (GET_PLAYER(getOwnerINLINE()).getNumCities() - GET_PLAYER(getOwnerINLINE()).getBuildingClassCountPlusMaking((BuildingClassTypes)GC.getBuildingInfo(eLoopBuilding).getFreeBuildingClass())));
								}
							}
							if (isProductionAutomated())
							{
								for (iJ = 0; iJ < GC.getNumBuildingClassInfos(); iJ++)
								{
									if (GC.getBuildingInfo(eLoopBuilding).getPrereqNumOfBuildingClass(iJ) > 0)
									{
										iValue = 0;
										break;
									}
								}
							}


							if (iValue > 0)
							{
								iTurnsLeft = getProductionTurnsLeft(eLoopBuilding, 0);

#ifdef DEBUG_CITY_BUILDS
								DEBUGLOG("%s%S(%d/%d)", (bFirst ? "" : ", "), 
									GC.getBuildingInfo(eLoopBuilding).getDescription(), iValue, iTurnsLeft);

								bFirst = false;
#endif

								if (isWorldWonderClass((BuildingClassTypes)iI))
								{
									int iMultiplier = 100 + getProductionModifier(eLoopBuilding);
									if (iProductionRank <= min(3, ((GET_PLAYER(getOwnerINLINE()).getNumCities() + 2) / 3)))
									{
										if (bAsync)
										{
											iTempValue = ((iMultiplier * GC.getASyncRand().get(GC.getLeaderHeadInfo(getPersonalityType()).getWonderConstructRand(), "Wonder Construction Rand ASYNC")) / 100);
										}
										else
										{
											iTempValue = ((iMultiplier * GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getWonderConstructRand(), "Wonder Construction Rand")) / 100);
										}
										
										if (bAreaAlone)
										{
											iTempValue *= 2;
										}

	#ifdef DEBUG_CITY_BUILDS
										DEBUGLOG("[+%d]", iTempValue);
	#endif

										iValue += iTempValue;
									}
									iTurnsLeft *= 150;
									iTurnsLeft /= (100 + iMultiplier);
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
									iValue /= max(1, (iTurnsLeft + 3));

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
	}

#ifdef DEBUG_CITY_BUILDS
	DEBUGLOG("\n");
#endif

	return eBestBuilding;
}


int CvCityAI::AI_buildingValue(BuildingTypes eBuilding, int iFocusFlags)
{
	return AI_buildingValueThreshold(eBuilding, iFocusFlags, 0);
}

// XXX should some of these count cities, buildings, etc. based on teams (because wonders are shared...)
// XXX in general, this function needs to be more sensitive to what makes this city unique (more likely to build airports if there already is a harbor...)
int CvCityAI::AI_buildingValueThreshold(BuildingTypes eBuilding, int iFocusFlags, int iThreshold)
{
	PROFILE_FUNC();

	int iTempValue;
	int iPass;
	int iI, iJ;

	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
	CvBuildingInfo& kBuilding = GC.getBuildingInfo(eBuilding);
	BuildingClassTypes eBuildingClass = (BuildingClassTypes) kBuilding.getBuildingClassType();
	int iLimitedWonderLimit = limitedWonderClassLimit(eBuildingClass);
	bool bIsLimitedWonder = (iLimitedWonderLimit >= 0);

	ReligionTypes eStateReligion = kOwner.getStateReligion();

	bool bAreaAlone = kOwner.AI_isAreaAlone(area());
	bool bProvidesPower = (kBuilding.isPower() || ((kBuilding.getPowerBonus() != NO_BONUS) && hasBonus((BonusTypes)(kBuilding.getPowerBonus()))));

	int iHasMetCount = GET_TEAM(getTeam()).getHasMetCivCount(true);
	int iBaseMaintenance = calculateBaseMaintenanceTimes100()/100; // XXX is this slow?
	int iFoodDifference = foodDifference(false);

	int iHappinessLevel = happyLevel() - unhappyLevel(1);
	int iAngryPopulation = range(-iHappinessLevel, 0, (getPopulation() + 1));
	int iHealthLevel = goodHealth() - badHealth(/*bNoAngry*/ false, 1);
	int iBadHealth = max(0, -iHealthLevel);

	int iHappyModifier = (iHappinessLevel >= iHealthLevel && iHappinessLevel <= 6) ? 2 : 1;
	int iHealthModifier = (iHealthLevel > iHappinessLevel && iHealthLevel <= 4) ? 2 : 1;
	
	int iTotalPopulation = kOwner.getTotalPopulation();
	int iNumCities = kOwner.getNumCities();
	int iNumCitiesInArea = area()->getCitiesPerPlayer(getOwnerINLINE());
	
	int aiYieldRank[NUM_YIELD_TYPES];
	for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		aiYieldRank[iI] = findBaseYieldRateRank((YieldTypes) iI);
	}

	int aiCommerceRank[NUM_COMMERCE_TYPES];
	for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		aiCommerceRank[iI] = findCommerceRateRank((CommerceTypes) iI);
	}

	bool bIsHighProductionCity = (aiYieldRank[YIELD_PRODUCTION] <= max(3, (iNumCities / 2)));
	
	int iCultureRank = findCommerceRateRank(COMMERCE_CULTURE);
	int iCulturalVictoryNumCultureCities = GC.getGameINLINE().culturalVictoryNumCultureCities();

	bool bFinancialTrouble = GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble();

	bool bCulturalVictory1 = GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_CULTURE1);
	bool bCulturalVictory2 = GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_CULTURE2);
	bool bCulturalVictory3 = GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_CULTURE3);
	
	bool bCanPopRush = GET_PLAYER(getOwnerINLINE()).canPopRush();

	if (kBuilding.isCapital())
	{
		FAssert(false);
		return 0;
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

	int iValue = 0;

	for (iPass = 0; iPass < 2; iPass++)
	{
		if ((iFocusFlags == 0) || (iValue > 0) || (iPass == 0))
		{
		    
		    if ((iFocusFlags & BUILDINGFOCUS_WORLDWONDER) || (iPass > 0))
		    {
		        if (isWorldWonderClass(eBuildingClass))
		        {
                    if (aiYieldRank[YIELD_PRODUCTION] <= 3)
                    {
						iValue++;
                    }
		        }
		    }
		    
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
				iValue += iBestHappy * 10;

				if (kBuilding.isNoUnhappiness())
				{
					iValue += ((iAngryPopulation * 10) + getPopulation());
				}
				
				int iBuildingHappiness = kBuilding.getHappiness();
				if (iBuildingHappiness != 0)
				{
					iValue += (min(iBuildingHappiness, iAngryPopulation) * 10) 
						+ (max(0, iBuildingHappiness - iAngryPopulation) * iHappyModifier);
				}

				iValue += (kBuilding.getAreaHappiness() * iNumCitiesInArea * 8);
				iValue += (kBuilding.getGlobalHappiness() * iNumCities * 8);

				int iWarWearinessPercentAnger = kOwner.getWarWearinessPercentAnger();
				int iWarWearinessModifer = kBuilding.getWarWearinessModifier();
				if (iWarWearinessModifer > 0)
				{
					iValue += (min(-(((iWarWearinessModifer * iWarWearinessPercentAnger) / 100) / GC.getDefineINT("PERCENT_ANGER_DIVISOR")), iAngryPopulation) * 8);
					iValue += (-iWarWearinessModifer * iHappyModifier) / 16;
				}
				
				int iGlobalWarWearinessModifer = kBuilding.getGlobalWarWearinessModifier();
				if (iGlobalWarWearinessModifer > 0)
				{
					iValue += (-(((iGlobalWarWearinessModifer * iWarWearinessPercentAnger / 100) / GC.getDefineINT("PERCENT_ANGER_DIVISOR"))) * iNumCities);
					iValue += (-iGlobalWarWearinessModifer * iHappyModifier) / 16;
				}

				iValue += (-kBuilding.getHurryAngerModifier() * getHurryPercentAnger()) / 100;

				int iStateReligionHappiness = kBuilding.getStateReligionHappiness();
				if (kBuilding.getReligionType() == eStateReligion && iStateReligionHappiness != 0)
				{
					iValue += (min(iStateReligionHappiness, iAngryPopulation) * 8)
						+ (max(0, iStateReligionHappiness - iAngryPopulation) * iHappyModifier);
				}

				for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
				{
					iValue += (min(((kBuilding.getCommerceHappiness(iI) * kOwner.getCommercePercent((CommerceTypes)iI)) / 100), iAngryPopulation) * 8);
					iValue += (kBuilding.getCommerceHappiness(iI) * iHappyModifier) / 4;
				}

				for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
				{
					if (hasBonus((BonusTypes)iI))
					{
						int iBonusHappinessChange = kBuilding.getBonusHappinessChanges(iI);
						iValue += (min(iBonusHappinessChange, iAngryPopulation) * 8)
							+ (max(0, iBonusHappinessChange - iAngryPopulation) * iHappyModifier);
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
					int iUnhealthyPopulation = unhealthyPopulation();
					iValue += (min(iUnhealthyPopulation, iBadHealth) * 6)
						+ (max(0, iUnhealthyPopulation - iBadHealth) * iHealthModifier);
				}

				if (kBuilding.isBuildingOnlyHealthy())
				{
					int iBuildingBadHealth = -getBuildingBadHealth();
					iValue += (min(iBuildingBadHealth, iBadHealth) * 6)
						+ ((max(0, iBuildingBadHealth - iBadHealth) + 1) * iHealthModifier);
				}

				int iBuildingHealth = kBuilding.getHealth();
				if (iBuildingHealth != 0)
				{
					iValue += (min(iBuildingHealth, iBadHealth) * 6)
						+ (max(0, iBuildingHealth - iBadHealth) * iHealthModifier);
				}

				iValue += (kBuilding.getAreaHealth() * iNumCitiesInArea * 4);
				iValue += (kBuilding.getGlobalHealth() * iNumCities * 4);

				for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
				{
					if (hasBonus((BonusTypes)iI))
					{
						int iBonusHealthChange = kBuilding.getBonusHealthChanges(iI);
						iValue += (min(iBonusHealthChange, iBadHealth) * 6)
							+ (max(0, iBonusHealthChange - iBadHealth) * iHealthModifier);
					}
				}
			}

			if ((iFocusFlags & BUILDINGFOCUS_EXPERIENCE) || (iPass > 0))
			{
				iValue += (kBuilding.getFreeExperience() * ((iHasMetCount > 0) ? 12 : 6));

				for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
				{
					if (canTrain((UnitCombatTypes)iI))
					{
						iValue += (kBuilding.getUnitCombatFreeExperience(iI) * ((iHasMetCount > 0) ? 6 : 3));
					}
				}

				for (iI = 0; iI < NUM_DOMAIN_TYPES; iI++)
				{
					int iDomainExpValue = 0;
					if (iI == DOMAIN_SEA)
					{
						iDomainExpValue = 7;
					} 
					else if (iI == DOMAIN_LAND)
					{
						iDomainExpValue = 12;
					}
					else
					{
						iDomainExpValue = 6;
					}
					iValue += (kBuilding.getDomainFreeExperience(iI) * ((iHasMetCount > 0) ? iDomainExpValue : iDomainExpValue / 2));
				}
			}
			
			// since this duplicates BUILDINGFOCUS_EXPERIENCE checks, do not repeat on pass 1
			if ((iFocusFlags & BUILDINGFOCUS_DOMAINSEA))
			{
				iValue += (kBuilding.getFreeExperience() * ((iHasMetCount > 0) ? 16 : 8));

				for (int iUnitIndex = 0; iUnitIndex < GC.getNumUnitInfos(); iUnitIndex++)
				{
					CvUnitInfo& kUnitInfo = GC.getUnitInfo((UnitTypes) iUnitIndex);
					int iCombatType = kUnitInfo.getUnitCombatType();
					if (kUnitInfo.getDomainType() == DOMAIN_SEA && canTrain((UnitTypes) iUnitIndex) && iCombatType != NO_UNITCOMBAT)
					{
						iValue += (kBuilding.getUnitCombatFreeExperience(iCombatType) * ((iHasMetCount > 0) ? 6 : 3));
					}
				}

				iValue += (kBuilding.getDomainFreeExperience(DOMAIN_SEA) * ((iHasMetCount > 0) ? 16 : 8));

				iValue += (kBuilding.getDomainProductionModifier(DOMAIN_SEA) / 4);
			}

			if ((iFocusFlags & BUILDINGFOCUS_MAINTENANCE) || (iFocusFlags & BUILDINGFOCUS_GOLD) || (iPass > 0))
			{
				iTempValue = ((iBaseMaintenance - ((iBaseMaintenance * max(0, (kBuilding.getMaintenanceModifier() + 100))) / 100)) * 6);
				if (bFinancialTrouble)
				{
					iTempValue *= 2;
				}

				iValue += iTempValue;
			}

			if ((iFocusFlags & BUILDINGFOCUS_SPECIALIST) || (iPass > 0))
			{
				for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
				{
					if (kBuilding.getSpecialistCount(iI) > 0)
					{
						iTempValue = AI_specialistValue(((SpecialistTypes)iI), false, false);

						iTempValue *= (20 + (40 * kBuilding.getSpecialistCount(iI)));
						iTempValue /= 100;

						iValue += (iTempValue / 100);
					}
				}
			}
			
			if ((iFocusFlags & (BUILDINGFOCUS_GOLD | BUILDINGFOCUS_RESEARCH)) || iPass > 0)
			{
				// trade routes
				iTempValue = ((kBuilding.getTradeRoutes() * ((8 * max(0, (totalTradeModifier() + 100))) / 100))
								* (getPopulation() / 5 + 1));
				int iGlobalTradeValue = (((6 * iTotalPopulation) / 5) / iNumCities);
				iTempValue += (kBuilding.getCoastalTradeRoutes() * kOwner.countNumCoastalCities() * iGlobalTradeValue);
				iTempValue += (kBuilding.getGlobalTradeRoutes() * iNumCities * iGlobalTradeValue);
				
				if (bFinancialTrouble)
				{
					iTempValue *= 2;
				}
					
				if (kOwner.isNoForeignTrade())
				{
					iTempValue /= 3;
				}
				

				iValue += iTempValue;
			}

			if (iPass > 0)
			{
				if (kBuilding.isAreaCleanPower())
				{
					iValue += (iNumCitiesInArea * 10); // XXX count cities without clean power???
				}

				if (kBuilding.getDomesticGreatGeneralRateModifier() != 0)
				{
					iValue += (kBuilding.getDomesticGreatGeneralRateModifier() / 10);
				}

				if (kBuilding.isAreaBorderObstacle())
				{
					iValue += (iNumCitiesInArea);
				}

				if (kBuilding.isGovernmentCenter())
				{
					FAssert(!(kBuilding.isCapital()));
					iValue += (calculateDistanceMaintenance() * iNumCitiesInArea);
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
				
				int iGreatPeopleRateModifier = kBuilding.getGreatPeopleRateModifier();
				if (iGreatPeopleRateModifier > 0)
				{
					int iGreatPeopleRate = getBaseGreatPeopleRate();
					const int kTargetGPRate = 10;

					// either not a wonder, or a wonder and our GP rate is at least the target rate
					if (!bIsLimitedWonder || iGreatPeopleRate >= kTargetGPRate)
					{
						iValue += ((iGreatPeopleRateModifier * iGreatPeopleRate) / 16);
					}
					// otherwise, this is a limited wonder (aka National Epic), we _really_ do not want to build this here
					// subtract from the value (if this wonder has a lot of other stuff, we still might build it)
					else
					{
						iValue -= ((iGreatPeopleRateModifier * (kTargetGPRate - iGreatPeopleRate)) / 12);
					}
				}

				iValue += ((kBuilding.getGlobalGreatPeopleRateModifier() * iNumCities) / 8);

				iValue += (-(kBuilding.getAnarchyModifier()) / 4);

				iValue += (-(kBuilding.getGlobalHurryModifier()) * 2);

				iValue += (kBuilding.getGlobalFreeExperience() * iNumCities * ((iHasMetCount > 0) ? 6 : 3));

				if (bCanPopRush)
				{
					iValue += kBuilding.getFoodKept() / 2;
				}

				iValue += (kBuilding.getAirlift() * getPopulation() * 3 + 10);

				iValue += (-(kBuilding.getAirModifier()) / ((iHasMetCount > 0) ? 9 : 18));

				iValue += (-(kBuilding.getNukeModifier()) / ((iHasMetCount > 0) ? 10 : 20));

				iValue += (kBuilding.getFreeSpecialist() * 16);
				iValue += (kBuilding.getAreaFreeSpecialist() * iNumCitiesInArea * 12);
				iValue += (kBuilding.getGlobalFreeSpecialist() * iNumCities * 12);

				iValue += ((kBuilding.getWorkerSpeedModifier() * kOwner.AI_getNumAIUnits(UNITAI_WORKER)) / 10);

				int iMilitaryProductionModifier = kBuilding.getMilitaryProductionModifier();
				if (iHasMetCount > 0 && iMilitaryProductionModifier > 0)
				{
					// either not a wonder, or a wonder and we are a high production city
					if (!bIsLimitedWonder || bIsHighProductionCity)
					{
						iValue += (iMilitaryProductionModifier / 4);

						// if a wonder, then pick one of the best cities
						if (bIsLimitedWonder)
						{
							// if one of the top 3 production cities, give a big boost
							if (aiYieldRank[YIELD_PRODUCTION] <= (2 + iLimitedWonderLimit))
							{
								iValue += (2 * iMilitaryProductionModifier) / (2 + aiYieldRank[YIELD_PRODUCTION]);
							}
						}
						// otherwise, any of the top half of cities will do
						else if (bIsHighProductionCity)
						{
							iValue += iMilitaryProductionModifier / 4;
						}
						iValue += ((iMilitaryProductionModifier * (getFreeExperience() + getSpecialistFreeExperience())) / 10);
					}
					// otherwise, this is a limited wonder (aka Heroic Epic), we _really_ do not want to build this here
					// subtract from the value (if this wonder has a lot of other stuff, we still might build it)
					else
					{
						iValue -= (iMilitaryProductionModifier * aiYieldRank[YIELD_PRODUCTION]) / 5;
					}
				}

				iValue += (kBuilding.getSpaceProductionModifier() / 5);
				iValue += ((kBuilding.getGlobalSpaceProductionModifier() * iNumCities) / 20);
				

				if (kBuilding.getGreatPeopleUnitClass() != NO_UNITCLASS)
				{
					iValue++; // XXX improve this for diversity...
				}
				
				// prefer to build great people buildings in places that already have some GP points
				iValue += (kBuilding.getGreatPeopleRateChange() * 10) * (1 + (getBaseGreatPeopleRate() / 2));

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
					
					if (bIsHighProductionCity)
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
							iValue += iNumCitiesInArea;
						}

						iValue++;

						ReligionTypes eReligion = (ReligionTypes)(GC.getUnitInfo((UnitTypes)iI).getPrereqReligion());
						if (eReligion != NO_RELIGION)
						{
						    //encouragement to get some minimal ability to train special units
						    if (bCulturalVictory1 || isHolyCity(eReligion) || isCapital())
						    {
						        iValue += (2 + iNumCitiesInArea);
                            }
                            
                            if (bCulturalVictory2 && GC.getUnitInfo((UnitTypes)iI).getReligionSpreads(eReligion))
                            {
                                //this gives a very large extra value if the religion is (nearly) unique
                                //to no extra value for a fully spread religion.
                                //I'm torn between huge boost and enough to bias towards the best monastery type.
                                int iReligionCount = GET_PLAYER(getOwnerINLINE()).getHasReligionCount(eReligion);
                                iValue += (100 * (iNumCities - iReligionCount)) / (iNumCities * (iReligionCount + 1));
                            }
						}
					}
				}
				
				// is this building needed to build other buildings?
				for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
				{
					int iPrereqBuildings = kOwner.getBuildingClassPrereqBuilding(((BuildingTypes) iI), eBuildingClass);

					// if we need some of us to build iI building, and we dont need more than we have cities
					if (iPrereqBuildings > 0 && iPrereqBuildings <= iNumCities)
					{
						// do we need more than what we are currently building?
						if (iPrereqBuildings > kOwner.getBuildingClassCountPlusMaking(eBuildingClass))
						{
							iValue += (iNumCities * 3);

							if (bCulturalVictory1)
							{
								BuildingTypes eLoopBuilding = (BuildingTypes) iI;
								CvBuildingInfo& kLoopBuilding = GC.getBuildingInfo(eLoopBuilding);
								int iLoopBuildingCultureModifier = kLoopBuilding.getCommerceModifier(COMMERCE_CULTURE);
								if (iLoopBuildingCultureModifier > 0)
								{
									int iLoopBuildingsBuilt = kOwner.getBuildingClassCount((BuildingClassTypes) kLoopBuilding.getBuildingClassType());
									
									// if we have less than the number needed in culture cities
									//		OR we are one of the top cities and we do not have the building
									if (iLoopBuildingsBuilt < iCulturalVictoryNumCultureCities || 
										(iCultureRank <= iCulturalVictoryNumCultureCities && !hasBuilding(eLoopBuilding)))
									{
										iValue += iLoopBuildingCultureModifier;

										if (bCulturalVictory3)
										{
											iValue += iLoopBuildingCultureModifier * 2;
										}
									}
								}
							}
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

					if (iFoodDifference > 0)
					{
						iValue += kBuilding.getFoodKept() / 2;
					}

					if (kBuilding.getSeaPlotYieldChange(iI) > 0)
					{
					    iTempValue += kBuilding.getSeaPlotYieldChange(iI) * AI_buildingSpecialYieldChangeValue(eBuilding, (YieldTypes)iI);
					}
					iTempValue += (kBuilding.getGlobalSeaPlotYieldChange(iI) * kOwner.countNumCoastalCities() * 8);
					iTempValue += (kBuilding.getYieldChange(iI) * 6);
					iTempValue += ((kBuilding.getYieldModifier(iI) * getBaseYieldRate((YieldTypes)iI)) / 10);
					iTempValue += ((kBuilding.getPowerYieldModifier(iI) * getBaseYieldRate((YieldTypes)iI)) / ((bProvidesPower || isPower()) ? 12 : 15));
					iTempValue += ((kBuilding.getAreaYieldModifier(iI) * iNumCitiesInArea) / 3);
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

					if (bFinancialTrouble && iI == YIELD_COMMERCE)
					{
						iTempValue *= 2;
					}

					iTempValue *= kOwner.AI_yieldWeight((YieldTypes)iI);
					iTempValue /= 100;

					// if this is a limited wonder, and we are not one of the top 4 in this category, subtract the value
					// we do _not_ want to build this here (unless the value was small anyway)
					if (bIsLimitedWonder && (aiYieldRank[iI] > (3 + iLimitedWonderLimit)))
					{
						iTempValue *= -1;
					}

					iValue += iTempValue;
				}
			}
			else
			{
				if (iFocusFlags & BUILDINGFOCUS_FOOD)
				{

					iValue += kBuilding.getFoodKept();

					if (kBuilding.getSeaPlotYieldChange(YIELD_FOOD) > 0)
					{
					    
					    iTempValue = kBuilding.getSeaPlotYieldChange(YIELD_FOOD) * AI_buildingSpecialYieldChangeValue(eBuilding, YIELD_FOOD);
					    if ((iTempValue < 8) && (getPopulation() > 3))
					    {
					        // don't bother
					    }
					    else
					    {
                            iValue += ((iTempValue * 4) / max(2, iFoodDifference));
					    }
					}
				}

				if (iFocusFlags & BUILDINGFOCUS_PRODUCTION)
				{
					iTempValue = ((kBuilding.getYieldModifier(YIELD_PRODUCTION) * getBaseYieldRate(YIELD_PRODUCTION)) / 20);
					iTempValue += ((kBuilding.getPowerYieldModifier(YIELD_PRODUCTION) * getBaseYieldRate(YIELD_PRODUCTION)) / ((bProvidesPower || isPower()) ? 24 : 30));

					if (bProvidesPower && !isPower())
					{
						iTempValue += ((getPowerYieldRateModifier(YIELD_PRODUCTION) * getBaseYieldRate(YIELD_PRODUCTION)) / 12);
					}
					
					// if this is a limited wonder, and we are not one of the top 4 in this category, subtract the value
					// we do _not_ want to build this here (unless the value was small anyway)
					if (bIsLimitedWonder && (aiYieldRank[iI] > (3 + iLimitedWonderLimit)))
					{
						iTempValue *= -1;
					}

					iValue += iTempValue;
				}

				if (iFocusFlags & BUILDINGFOCUS_GOLD)
				{
					iTempValue = ((kBuilding.getYieldModifier(YIELD_COMMERCE) * getBaseYieldRate(YIELD_COMMERCE)));
					iTempValue *= kOwner.getCommercePercent(COMMERCE_GOLD);
					
					if (bFinancialTrouble)
					{
						iTempValue *= 2;
					}
					
					iTempValue /= 3000;

					// if this is a limited wonder, and we are not one of the top 4 in this category, subtract the value
					// we do _not_ want to build this here (unless the value was small anyway)
					if (bIsLimitedWonder && (aiYieldRank[iI] > (3 + iLimitedWonderLimit)))
					{
						iTempValue *= -1;
					}

					iValue += iTempValue;
				}
			}

			if (iPass > 0)
			{
				for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
				{
					iTempValue = 0;

					iTempValue += (kBuilding.getCommerceChange(iI) * 4);
					iTempValue += (kBuilding.getObsoleteSafeCommerceChange(iI) * 4);
					
					if ((CommerceTypes)iI == COMMERCE_CULTURE)
					{
					    if (bCulturalVictory1)
					    {
					        iTempValue *= 2;					        
					    }
					}

					if (kBuilding.getCommerceChangeDoubleTime(iI) > 0)
					{
						if ((kBuilding.getCommerceChange(iI) > 0) || (kBuilding.getObsoleteSafeCommerceChange(iI) > 0))
						{
							iTempValue += (1000 / kBuilding.getCommerceChangeDoubleTime(iI));
						}
					}
					
					// add value for a commerce modifier
					int iCommerceModifier = kBuilding.getCommerceModifier(iI);
					int iBaseCommerceRate = getBaseCommerceRate((CommerceTypes) iI);
					int iCommerceMultiplierValue = iCommerceModifier * iBaseCommerceRate;
					if (((CommerceTypes) iI) == COMMERCE_CULTURE && iCommerceModifier != 0)
					{
						if (bCulturalVictory1)
						{							
							// if this is one of our top culture cities, then we want to build this here first!
							if (iCultureRank <= iCulturalVictoryNumCultureCities)
							{					        
								iCommerceMultiplierValue /= 8;					        
								
								// if we at culture level 3, then these need to get built asap
								if (bCulturalVictory3)
								{
									// its most important to build in the lowest rate city, but important everywhere
									iCommerceMultiplierValue += max(100, 500 - iBaseCommerceRate) * iCommerceModifier;					        
								}
							}
							else
							{
								int iCountBuilt = kOwner.getBuildingClassCountPlusMaking(eBuildingClass);

								// do we have enough buildings to build extras?
								bool bHaveEnough = true;

								// if its limited and the limit is less than the number we need in culture cities, do not build here
								if (bIsLimitedWonder && (iLimitedWonderLimit <= iCulturalVictoryNumCultureCities))
								{
									bHaveEnough = false;
								}

								for (iJ = 0; bHaveEnough && iJ < GC.getNumBuildingClassInfos(); iJ++)
								{
									// count excess the number of prereq buildings which do not have this building built for yet
									int iPrereqBuildings = kOwner.getBuildingClassPrereqBuilding(eBuilding, (BuildingClassTypes) iJ, -iCountBuilt);
									
									// do we not have enough built (do not count ones in progress)
									if (iPrereqBuildings > 0 && kOwner.getBuildingClassCount((BuildingClassTypes) iJ) <  iPrereqBuildings)
									{
										bHaveEnough = false;
									}
								}

								// if we have enough and our rank is close to the top, then possibly build here too
								if (bHaveEnough && (iCultureRank - iCulturalVictoryNumCultureCities) <= 3)
								{
									iCommerceMultiplierValue /= 12;
								}
								// otherwise, we really do not want to build this here
								else
								{
									iCommerceMultiplierValue /= 30;
								}
							}
						}
						else
						{
							iCommerceMultiplierValue /= 15;

							// increase priority if we need culture oppressed city
							iCommerceMultiplierValue *= (100 - calculateCulturePercent(getOwnerINLINE()));
						}
					}
					else
					{
 						iCommerceMultiplierValue /= 15;
					}
					iTempValue += iCommerceMultiplierValue;

					iTempValue += ((kBuilding.getGlobalCommerceModifier(iI) * iNumCities) / 4);
					iTempValue += ((kBuilding.getSpecialistExtraCommerce(iI) * kOwner.getTotalPopulation()) / 3);

					if (eStateReligion != NO_RELIGION)
					{
						iTempValue += (kBuilding.getStateReligionCommerce(iI) * kOwner.getHasReligionCount(eStateReligion) * 3);
					}

					if (kBuilding.getGlobalReligionCommerce() != NO_RELIGION)
					{
						iTempValue += (GC.getReligionInfo((ReligionTypes)(kBuilding.getGlobalReligionCommerce())).getGlobalReligionCommerce(iI) * GC.getGameINLINE().countReligionLevels((ReligionTypes)(kBuilding.getGlobalReligionCommerce())) * 2);
						if (eStateReligion == (ReligionTypes)(kBuilding.getGlobalReligionCommerce()))
						{
						    iTempValue += 10;
						}
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

					if (bFinancialTrouble && iI == COMMERCE_GOLD)
					{
						iTempValue *= 2;
					}

					iTempValue *= kOwner.AI_commerceWeight(((CommerceTypes)iI), this);
					iTempValue = (iTempValue + 99) / 100;

					// if this is a limited wonder, and we are not one of the top 4 in this category, subtract the value
					// we do _not_ want to build this here (unless the value was small anyway)
					if (bIsLimitedWonder && ((aiCommerceRank[iI] > (3 + iLimitedWonderLimit)))
						|| (bCulturalVictory1 && (iI == COMMERCE_CULTURE) && (aiCommerceRank[iI] == 1)))
					{
						iTempValue *= -1;

						// for culture, just set it to zero, not negative, just about every wonder gives culture
						if (iI == COMMERCE_CULTURE)
						{
							iTempValue = 0;
						}
					}

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
					iTempValue = ((kBuilding.getCommerceModifier(COMMERCE_GOLD) * getBaseCommerceRate(COMMERCE_GOLD)) / 40);

					if (bFinancialTrouble)
					{
						iTempValue *= 2;
					}

					// if this is a limited wonder, and we are not one of the top 4 in this category, subtract the value
					// we do _not_ want to build this here (unless the value was small anyway)
					if (bIsLimitedWonder && (aiCommerceRank[COMMERCE_GOLD] > (3 + iLimitedWonderLimit)))
					{
						iTempValue *= -1;
					}

					iValue += iTempValue;
				}

				if (iFocusFlags & BUILDINGFOCUS_RESEARCH)
				{
					iTempValue = ((kBuilding.getCommerceModifier(COMMERCE_RESEARCH) * getBaseCommerceRate(COMMERCE_RESEARCH)) / 40);

					// if this is a limited wonder, and we are not one of the top 4 in this category, subtract the value
					// we do _not_ want to build this here (unless the value was small anyway)
					if (bIsLimitedWonder && (aiCommerceRank[COMMERCE_RESEARCH] > (3 + iLimitedWonderLimit)))
					{
						iTempValue *= -1;
					}

					iValue += iTempValue;
				}

				if (iFocusFlags & BUILDINGFOCUS_CULTURE)
				{
					iTempValue = (kBuilding.getCommerceChange(COMMERCE_CULTURE) * 3);
					iTempValue += (kBuilding.getObsoleteSafeCommerceChange(COMMERCE_CULTURE) * 3);

					// if this is a limited wonder, and we are not one of the top 4 in this category, 
					// do not count the culture value
					// we probably do not want to build this here (but we might)
					if (bIsLimitedWonder && (aiCommerceRank[COMMERCE_CULTURE] > (3 + iLimitedWonderLimit)))
					{
						iTempValue  = 0;
					}

					iValue += iTempValue;
				}
				
                if (iFocusFlags & BUILDINGFOCUS_BIGCULTURE)
				{
					iTempValue = (kBuilding.getCommerceModifier(COMMERCE_CULTURE) / 5);

					// if this is a limited wonder, and we are not one of the top 4 in this category, 
					// do not count the culture value
					// we probably do not want to build this here (but we might)
					if (bIsLimitedWonder && (aiCommerceRank[COMMERCE_CULTURE] > (3 + iLimitedWonderLimit)))
					{
						iTempValue  = 0;
					}

					iValue += iTempValue;
				}
			}
			
			if ((iThreshold > 0) && (iPass == 0))
			{
				if (iValue < iThreshold)
				{
					iValue = 0;
				}
			}

			if (iPass > 0 && !isHuman())
			{
				iValue += kBuilding.getAIWeight();
				if (iValue > 0)
				{
					for (iI = 0; iI < GC.getNumFlavorTypes(); iI++)
					{
						iValue += (kOwner.AI_getFlavorValue((FlavorTypes)iI) * kBuilding.getFlavorValue(iI));
					}
				}
			}
		}
	}
	
	// obsolete checks
	bool bCanResearchObsoleteTech = false;
	int iErasUntilObsolete = MAX_INT;
	if (kBuilding.getObsoleteTech() != NO_TECH)
	{
		TechTypes eObsoleteTech = (TechTypes) kBuilding.getObsoleteTech();
		FAssertMsg(eObsoleteTech == NO_TECH || !(GET_TEAM(getTeam()).isHasTech(eObsoleteTech)), "Team expected to not have the tech that obsoletes eBuilding");
		iErasUntilObsolete = GC.getTechInfo(eObsoleteTech).getEra() - kOwner.getCurrentEra();
		bCanResearchObsoleteTech = kOwner.canResearch(eObsoleteTech);
	}

	if (kBuilding.getSpecialBuildingType() != NO_SPECIALBUILDING)
	{
		TechTypes eSpecialBldgObsoleteTech = (TechTypes) GC.getSpecialBuildingInfo((SpecialBuildingTypes)(kBuilding.getSpecialBuildingType())).getObsoleteTech();
		FAssertMsg(eSpecialBldgObsoleteTech == NO_TECH || !(GET_TEAM(getTeam()).isHasTech(eSpecialBldgObsoleteTech)), "Team expected to not have the tech that obsoletes eBuilding");
		if (eSpecialBldgObsoleteTech != NO_TECH)
		{
			int iSpecialBldgErasUntilObsolete = GC.getTechInfo(eSpecialBldgObsoleteTech).getEra() - kOwner.getCurrentEra();

			if (iSpecialBldgErasUntilObsolete < iErasUntilObsolete)
			{
				iErasUntilObsolete = iSpecialBldgErasUntilObsolete;
			}

			if (!bCanResearchObsoleteTech)
			{
				bCanResearchObsoleteTech = kOwner.canResearch(eSpecialBldgObsoleteTech);
			}
		}
	}
	
	// tech path method commented out, it would be more accurate if we want to be so.
	//int iObsoleteTechPathLength = 0;
	//if (kBuilding.getObsoleteTech() != NO_TECH)
	//{
	//	iObsoleteTechPathLength = findPathLength(kBuilding.getObsoleteTech(), false);
	//}

	//if (kBuilding.getSpecialBuildingType() != NO_SPECIALBUILDING)
	//{
	//	TechTypes eSpecialBldgObsoleteTech = GC.getSpecialBuildingInfo((SpecialBuildingTypes)(kBuilding.getSpecialBuildingType())).getObsoleteTech();
	//	int iSpecialBldgObsoleteTechPathLength = findPathLength(eSpecialBldgObsoleteTech, false);

	//	if (iSpecialBldgObsoleteTechPathLength < iObsoleteTechPathLength)
	//	{
	//		iObsoleteTechPathLength = iSpecialBldgObsoleteTechPathLength;
	//	}
	//}
	
	// if we can research obsolete tech, then this almost no value
	if (bCanResearchObsoleteTech)
	{
		iValue = min(16, (iValue + 31) / 32);
	}
	// if we are going obsolete in the next era, the current era, or even previous eras...
	else if (iErasUntilObsolete < 2)
	{
		// do not care about obsolete if we are going cultural and there is a culture benefit post obsolesence
		if (!bCulturalVictory1 || (kBuilding.getObsoleteSafeCommerceChange(COMMERCE_CULTURE) <= 0))
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
	return AI_bestProcess(NO_COMMERCE);
}

ProcessTypes CvCityAI::AI_bestProcess(CommerceTypes eCommerceType)
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
			iValue = AI_processValue((ProcessTypes)iI, eCommerceType);

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
	return AI_processValue(eProcess, NO_COMMERCE);
}

int CvCityAI::AI_processValue(ProcessTypes eProcess, CommerceTypes eCommerceType)
{
	int iValue;
	int iTempValue;
	int iI;
	bool bValid = (eCommerceType == NO_COMMERCE);
	
	iValue = 0;

	if (GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble())
	{
		iValue += GC.getProcessInfo(eProcess).getProductionToCommerceModifier(COMMERCE_GOLD);
	}

	// if we own less than 50%, or we need to pop borders
	if ((plot()->calculateCulturePercent(getOwnerINLINE()) < 50) || (getCultureLevel() <= (CultureLevelTypes) 1))
	{
		iValue += GC.getProcessInfo(eProcess).getProductionToCommerceModifier(COMMERCE_CULTURE);
	}

	for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		iTempValue = GC.getProcessInfo(eProcess).getProductionToCommerceModifier(iI);
		if (!bValid && ((CommerceTypes)iI == eCommerceType) && (iTempValue > 0))
		{
			bValid = true;
		    iTempValue *= 2;
		}

		iTempValue *= GET_PLAYER(getOwnerINLINE()).AI_commerceWeight(((CommerceTypes)iI), this);
		
		iTempValue /= 100;

		iValue += iTempValue;
	}

	return (bValid ? iValue : 0);
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
	PROFILE_FUNC();
	int iDefenders;
	bool bOffenseWar = ((area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE) || (area()->getAreaAIType(getTeam()) == AREAAI_MASSING));
	bool bDefenseWar = ((area()->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE));
	int iNumCities = GET_PLAYER(getOwnerINLINE()).getNumCities();
	

	if (!(GET_TEAM(getTeam()).AI_isWarPossible()))
	{
		return 1;
	}
	
	if (isBarbarian())
	{
		iDefenders = 2 + ((getPopulation() - 2) / 7);
		return iDefenders;
	}
		
		

	iDefenders = 1;
	
	if (hasActiveWorldWonder() || isCapital())
	{
		iDefenders++;
	}
	
	if (!GET_PLAYER(getOwner()).AI_isDoStrategy(AI_STRATEGY_CRUSH))
	{
		iDefenders += AI_neededFloatingDefenders();
	}
	else
	{
		iDefenders += (AI_neededFloatingDefenders() + 2) / 4;
	}
	
	
	if (bDefenseWar)
	{
		if (!(plot()->isHills()))
		{
			iDefenders++;
		}
	}

	if (isOccupation())
	{
		iDefenders ++;
	}

	if (GC.getGameINLINE().getMaxCityElimination() > 0)
	{
		iDefenders++;
	}

	return iDefenders;
}

int CvCityAI::AI_neededFloatingDefenders()
{
	if (m_iNeededFloatingDefendersCacheTurn != GC.getGame().getGameTurn())
	{
		AI_updateNeededFloatingDefenders();
	}
	return m_iNeededFloatingDefenders;	
}

void CvCityAI::AI_updateNeededFloatingDefenders()
{
	int iDefenders = 0;
	
	int iFloatingDefenders = GET_PLAYER(getOwnerINLINE()).AI_getTotalFloatingDefendersNeeded(area());
		
	int iTotalThreat = max(1, GET_PLAYER(getOwnerINLINE()).AI_getTotalAreaCityThreat(area()));
	
	iFloatingDefenders *= AI_cityThreat();
	iFloatingDefenders += (iTotalThreat / 2);
	iFloatingDefenders /= iTotalThreat;
	
	m_iNeededFloatingDefenders = iFloatingDefenders;
	m_iNeededFloatingDefendersCacheTurn = GC.getGame().getGameTurn();
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

void CvCityAI::AI_forceEmphasizeCulture(bool bNewValue)
{
	if (m_bForceEmphasizeCulture != bNewValue)
	{
		m_bForceEmphasizeCulture = bNewValue;

		m_aiEmphasizeCommerceCount[COMMERCE_CULTURE] += (bNewValue ? 1 : -1);
		FAssert(m_aiEmphasizeCommerceCount[COMMERCE_CULTURE] >= 0);
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
					if ((pLoopPlot->getImprovementType() == NO_IMPROVEMENT) || !(GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_SAFE_AUTOMATION) && !(pLoopPlot->getImprovementType() == (GC.getDefineINT("RUINS_IMPROVEMENT")))))
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
	int iProductionTotal = GC.getYieldInfo(YIELD_PRODUCTION).getMinCity();
	BonusTypes eBonus;
	int iFoodMultiplier = 100;
	int iCommerceMultiplier = 100;
	int iProductionMultiplier = 100;
	int iWorkerCount = 0;

	bool bChop = false;

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
					    int iCount = (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_BUILD));
					    iWorkerCount += iCount;
						//check if the tile is being improved
						if (iCount > 0)
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


				eBonus = pLoopPlot->getBonusType(getTeam());

				if (((aiFinalYields[YIELD_FOOD]*10) + (aiFinalYields[YIELD_PRODUCTION]*6) + (aiFinalYields[YIELD_COMMERCE]*4)) > 21)
				{
					iGoodTileCount++;
                    iFoodTotal += aiFinalYields[YIELD_FOOD];
                    if (aiFinalYields[YIELD_PRODUCTION] > 1)
                    {	
                    	iProductionTotal += aiFinalYields[YIELD_PRODUCTION];
                    }
				}

				if (eBonus != NO_BONUS)
				{
                    int iNetFood = (aiFinalYields[YIELD_FOOD] - GC.getFOOD_CONSUMPTION_PER_POPULATION());
                    iBonusFoodSurplus += max(0, iNetFood);
                    iBonusFoodDeficit += max(0, -iNetFood);
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
	int iBonusFoodDiff = ((iBonusFoodSurplus + iFeatureFoodSurplus) - iBonusFoodDeficit);

	int iTargetSize = min(iGoodTileCount, getPopulation()+(happyLevel()-unhappyLevel()));
	iTargetSize = min(iTargetSize, 1 + getPopulation() + goodHealth() - badHealth());

	if (iBonusFoodDiff < 3)
	{
		iFoodMultiplier += 10 * (3 - iBonusFoodDiff);
	}

	if (iFoodTotal < ((iTargetSize + ((getPopulation() < iTargetSize) ? 1 : 0)) * GC.getFOOD_CONSUMPTION_PER_POPULATION()))
	{
		iFoodMultiplier += 30;
	}

	if (iProductionTotal < 10)
	{
	    iProductionMultiplier += (80 - 8 * iProductionTotal);
	}
	int iProductionTarget = 1 + (min(getPopulation(), (iTargetSize * 2) / 3) * 2);
	
	if (iProductionTotal < iProductionTarget)
	{
	    iProductionMultiplier += 8 * (iProductionTarget - iProductionTotal);
	}
	
	int iCurrentEra = GET_PLAYER(getOwnerINLINE()).getCurrentEra();
	int iMaxEra = max(1, GC.getNumEraInfos() - 1);
	
	iProductionMultiplier += (60 * (iCurrentEra * iCurrentEra)) / (iMaxEra * iMaxEra);

	if ((iBonusFoodSurplus + iFeatureFoodSurplus > 5) && ((iBonusFoodDeficit + iHillFoodDeficit) > 5))
	{
		if ((iBonusFoodDeficit + iHillFoodDeficit) > 8)
		{
			//probably a good candidate for a wonder pump
			iProductionMultiplier += 40;
			iCommerceMultiplier += (GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble()) ? 0 : -40;
		}
	}
	
	int iProductionAdvantage = 100 * AI_yieldMultiplier(YIELD_PRODUCTION);
	iProductionAdvantage /= GET_PLAYER(getOwnerINLINE()).AI_averageYieldMultiplier(YIELD_PRODUCTION);
	iProductionAdvantage *= GET_PLAYER(getOwnerINLINE()).AI_averageYieldMultiplier(YIELD_COMMERCE);
	iProductionAdvantage /= AI_yieldMultiplier(YIELD_COMMERCE);
	
	//now we normalize the effect by # of cities
	
	int iNumCities = GET_PLAYER(getOwnerINLINE()).getNumCities();
	FAssert(iNumCities > 0);//superstisious?
	
	//in short in an OCC the relative multipliers should *never* make a difference
	//so this indeed equals "100" for the iNumCities == 0 case.
	iProductionAdvantage = ((iProductionAdvantage * (iNumCities - 1) + 200) / (iNumCities + 1));
	
	iProductionMultiplier *= iProductionAdvantage;
	iProductionMultiplier /= 100;
	
	iCommerceMultiplier *= 100;
	iCommerceMultiplier /= iProductionAdvantage;
	
	int iGreatPeopleAdvantage = 100 * getTotalGreatPeopleRateModifier();
	iGreatPeopleAdvantage /= GET_PLAYER(getOwnerINLINE()).AI_averageGreatPeopleMultiplier();
	iGreatPeopleAdvantage = ((iGreatPeopleAdvantage * (iNumCities - 1) + 200) / (iNumCities + 1));
	iGreatPeopleAdvantage += 200; //gpp multipliers are larger than others so lets not go overboard
	iGreatPeopleAdvantage /= 3;
	
	//With great people we want to slightly increase food priority at the expense of commerce
	//this gracefully handles both wonder and specialist based GPP...
	iCommerceMultiplier *= 100;
	iCommerceMultiplier /= iGreatPeopleAdvantage;
	iFoodMultiplier *= iGreatPeopleAdvantage;
	iFoodMultiplier /= 100;	
	
	// if leader flavor likes production, increase production, reduce commerce
	if (GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_PRODUCTION))
	{
		iProductionMultiplier += 10;
		iCommerceMultiplier -= 16;
	}

    if (!bChop)
	{
		ProjectTypes eProductionProject = getProductionProject();
		bChop = (eProductionProject != NO_PROJECT && AI_projectValue(eProductionProject) > 0);
	}
	if (!bChop)
	{
		BuildingTypes eProductionBuilding = getProductionBuilding();
		bChop = (eProductionBuilding != NO_BUILDING && isWorldWonderClass((BuildingClassTypes)(GC.getBuildingInfo(eProductionBuilding).getBuildingClassType())));
	}
	if (!bChop)
	{
		UnitTypes eProductionUnit = getProductionUnit();
		bChop = (eProductionUnit != NO_UNIT && GC.getUnitInfo(eProductionUnit).isFoodProduction());
	}
	if (!bChop)
	{
        bChop = ((area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE) || (area()->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE) || (area()->getAreaAIType(getTeam()) == AREAAI_MASSING));
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
				AI_bestPlotBuild(pLoopPlot, &(m_aiBestBuildValue[iI]), &(m_aeBestBuild[iI]), iFoodMultiplier, iProductionMultiplier, iCommerceMultiplier, bChop);
				m_aiBestBuildValue[iI] *= 4;
				m_aiBestBuildValue[iI] /= (4 + iWorkerCount);
				FAssert(AI_getBestBuildValue(iI) >= 0);
			}
		}
	}
}

// Protected Functions...

// Better drafting strategy by Blake - thank you!
void CvCityAI::AI_doDraft(bool bForce)
{
	PROFILE_FUNC();

	FAssert(!isHuman());
	if (isBarbarian())
	{
		return;
	}

	if (canConscript())
	{
	    if (GC.getUnitInfo(getConscriptUnit()).getCombat() > 5)
        {
			if (bForce)
			{
				conscript();
				return;
        	}
        	
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
}

// Better pop-rushing strategy by Blake - thank you!
void CvCityAI::AI_doHurry(bool bForce)
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
	bool bGrowth;
	int iI, iJ;

	FAssert(!isHuman() || isProductionAutomated());
	
	if (isBarbarian())
	{
		return;
	}

	if ((getProduction() == 0) && !bForce)
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
			if (bForce)
			{
				hurry((HurryTypes)iI);
				break;
			}
			iHurryAngerLength = hurryAngerLength((HurryTypes)iI);
			iHurryPopulation = hurryPopulation((HurryTypes)iI);

			iMinTurns = MAX_INT;
			bEssential = false;
			bGrowth = false;

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
						if (getCommerceRateTimes100(COMMERCE_CULTURE) == 0)
						{
						    bEssential = true;
						    iMinTurns = min(iMinTurns, 5);
						    if (AI_countNumBonuses(NO_BONUS, false, true, 2, true, true) > 0)
						    {
						    	bGrowth = true;						    	
						    }
						}
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
					
					if (AI_buildingSpecialYieldChangeValue(eProductionBuilding, YIELD_FOOD) > (getPopulation() * 2))
					{
						bEssential = true;
						bGrowth = true;
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
						bEssential = true;
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
					bGrowth = true;
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
							bGrowth = true;
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
					bGrowth = true;
				}
			}

			if (eProductionUnitAI == UNITAI_WORKER_SEA)
			{
				if (AI_neededSeaWorkers() > 0)
				{
					iMinTurns = min(iMinTurns, 5);
					bEssential = true;
					bGrowth = true;
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

			//this overrides everything.
			if (bGrowth)
			{
				int iHurryGold = hurryGold((HurryTypes)iI);
				if ((iHurryGold > 0) && ((iHurryGold * 16) > GET_PLAYER(getOwnerINLINE()).getGold()))
				{
					hurry((HurryTypes)iI);
					break;					
				}
				if (AI_countGoodTiles((healthRate(0) == 0), false, 100) <= (getPopulation() - iHurryPopulation))
				{
					hurry((HurryTypes)iI);
					break;					
				}
			}
			if (AI_countGoodTiles((healthRate(0) == 0), false, 100) <= (getPopulation() - iHurryPopulation))
			{
				int iProductionTurnsLeft = getProductionTurnsLeft();
				if (iProductionTurnsLeft > iMinTurns)
				{
					bWait = isHuman();
					
					if ((iHurryPopulation * 3) > (iProductionTurnsLeft * 2))
					{
						bWait = true;
					}

					if (!bWait)
					{
						if (iHurryAngerLength > 0)
						{
							//is the whip just too small or the population just too reduced to bother?
							if (!bEssential && ((iHurryPopulation < (1 + GC.getDefineINT("HURRY_POP_ANGER"))) || ((getPopulation() - iHurryPopulation) <= max(3, (getHighestPopulation() / 2)))))
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
}


// Improved use of emphasize by Blake, to go with his whipping strategy - thank you!
void CvCityAI::AI_doEmphasize()
{
	PROFILE_FUNC();

	FAssert(!isHuman());

	bool bFirstTech;
	bool bEmphasize;
	bool bCultureVictory = GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_CULTURE2);
	int iI;
	
	int iBadHealth = -healthRate();

	if (GET_PLAYER(getOwnerINLINE()).getCurrentResearch() != NO_TECH)
	{
		bFirstTech = GC.getGameINLINE().AI_isFirstTech(GET_PLAYER(getOwnerINLINE()).getCurrentResearch());
	}
	else
	{
		bFirstTech = false;
	}

	int iPopulationRank = findPopulationRank();
	int iCultureRateRank = findCommerceRateRank(COMMERCE_CULTURE);

	int iCulturalVictoryNumCultureCities = GC.getGameINLINE().culturalVictoryNumCultureCities();

	for (iI = 0; iI < GC.getNumEmphasizeInfos(); iI++)
	{
		bEmphasize = false;

		if (GC.getEmphasizeInfo((EmphasizeTypes)iI).getYieldChange(YIELD_FOOD) > 0)
		{

		}

		if (GC.getEmphasizeInfo((EmphasizeTypes)iI).getYieldChange(YIELD_PRODUCTION) > 0)
		{

		}
		
		if (AI_specialYieldMultiplier(YIELD_PRODUCTION) < 50)
		{
			if (GC.getEmphasizeInfo((EmphasizeTypes)iI).getYieldChange(YIELD_COMMERCE) > 0)
			{
				if (bFirstTech)
				{
					bEmphasize = true;
				}
			}

			if (GC.getEmphasizeInfo((EmphasizeTypes)iI).getCommerceChange(COMMERCE_RESEARCH) > 0)
			{
				if (bFirstTech && !bCultureVictory)
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
					if (!bCultureVictory || iCultureRateRank > (iCulturalVictoryNumCultureCities + 1))
					{
						bEmphasize = true;
					}
				}
			}
		}

		AI_setEmphasize(((EmphasizeTypes)iI), bEmphasize);
	}
	
	// force emphasize culture for cultural victories
	// there is no emphasize type for this, so we have to force it
	
//    bEmphasize = false;
//	if (!bGrow)
//	{
//		
//		// emphasize culture in our top 4 cities (1 more than what is needed for cultural victory)
//		if (bCultureVictory && iCultureRateRank <= (iCulturalVictoryNumCultureCities + 1))
//		{
//			bEmphasize = true;
//		}
//	}
//	AI_forceEmphasizeCulture(bEmphasize);
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
#ifdef DEBUG_CITY_BUILDS
		DEBUGLOG("%S: AI_chooseUnit(%S): choose %S\n", getName().GetCString(), GC.getUnitAIInfo(eUnitAI).getDescription(), GC.getUnitInfo(eBestUnit).getDescription());
#endif

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

bool CvCityAI::AI_chooseFloatingDefender(int iTotalNeededDefenders)
{
 	int iCount;
 	
	iCount = GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(area(), UNITAI_CITY_DEFENSE);
	if (iCount < (iTotalNeededDefenders / 5))
	{
		if (AI_chooseUnit(UNITAI_CITY_DEFENSE))
		{
			return true;
		}
	}
	iTotalNeededDefenders -= iCount;
	
	iCount = GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(area(), UNITAI_COLLATERAL);
	if (iCount <= (iTotalNeededDefenders / 4))
	{
		if (AI_chooseUnit(UNITAI_COLLATERAL))
		{
			return true;
		}
	}
	iTotalNeededDefenders -= iCount;
	
	
	iCount = GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(area(), UNITAI_CITY_COUNTER);
	if (iCount < (iTotalNeededDefenders / 3))
	{
		if (AI_chooseUnit(UNITAI_CITY_COUNTER))
		{
			return true;
		}
	}
	iTotalNeededDefenders -= iCount;

	
	if (AI_chooseUnit(UNITAI_RESERVE))
	{
		return true;
	}
	
	return false;
}


bool CvCityAI::AI_chooseBuilding(int iFocusFlags, int iMaxTurns, int iMinThreshold)
{
	BuildingTypes eBestBuilding;

	eBestBuilding = AI_bestBuildingThreshold(iFocusFlags, iMaxTurns, iMinThreshold);

	if (eBestBuilding != NO_BUILDING)
	{
#ifdef DEBUG_CITY_BUILDS
		CvWString szFocusString; 
		getBuildingFocusString(szFocusString, iFocusFlags);

		DEBUGLOG("%S: AI_chooseBuilding(%S,%d): choose %S\n", getName().GetCString(), szFocusString.GetCString(), iMaxTurns, GC.getBuildingInfo(eBestBuilding).getDescription());
#endif

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
#ifdef DEBUG_CITY_BUILDS
		DEBUGLOG("%S: AI_chooseProject: choose %S\n", getName().GetCString(), GC.getProjectInfo(eBestProject).getDescription());
#endif

		pushOrder(ORDER_CREATE, eBestProject, -1, false, false, false);
		return true;
	}

	return false;
}


bool CvCityAI::AI_chooseProcess(CommerceTypes eCommerceType)
{
	ProcessTypes eBestProcess;

	eBestProcess = AI_bestProcess(eCommerceType);

	if (eBestProcess != NO_PROCESS)
	{
#ifdef DEBUG_CITY_BUILDS
		DEBUGLOG("%S: AI_chooseProcess: choose %S\n", getName().GetCString(), GC.getProcessInfo(eBestProcess).getDescription());
#endif

		pushOrder(ORDER_MAINTAIN, eBestProcess, -1, false, false, false);
		return true;
	}

	return false;
}


// Returns true if a worker was added to a plot...
bool CvCityAI::AI_addBestCitizen(bool bWorkers, bool bSpecialists)
{
	PROFILE_FUNC();

#ifdef USE_NEW_CITY_GOVERNOR
	return test_AI_AddBestCitizen(bWorkers, bSpecialists);
#endif

#ifdef COMPARE_NEW_CITY_GOVERNOR
	bool bTestSuccess = true;
	if (!m_bDisableTest)
	{
		bTestSuccess = test_AI_AddBestCitizen(bWorkers, bSpecialists);
	}
#endif

	bool bAvoidGrowth = AI_avoidGrowth();
	bool bIgnoreGrowth = AI_ignoreGrowth();
	bool bIsSpecialistForced = false;

	int iBestSpecialistValue = 0;
	SpecialistTypes eBestSpecialist = NO_SPECIALIST;
	SpecialistTypes eBestForcedSpecialist = NO_SPECIALIST;

	if (bSpecialists)
	{
		// count the total forced specialists
		int iTotalForcedSpecialists = 0;
		for (int iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
		{
			int iForcedSpecialistCount = getForceSpecialistCount((SpecialistTypes)iI);
			if (iForcedSpecialistCount > 0)
			{
				bIsSpecialistForced = true;
				iTotalForcedSpecialists += iForcedSpecialistCount;
			}
		}
		
		// if forcing any specialists, find the best one that we can still assign
		if (bIsSpecialistForced)
		{
			int iBestForcedValue = MIN_INT;
			
			int iTotalSpecialists = 1 + getSpecialistPopulation();
			for (int iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
			{
				if (isSpecialistValid((SpecialistTypes)iI, 1))
				{
					int iForcedSpecialistCount = getForceSpecialistCount((SpecialistTypes)iI);
					if (iForcedSpecialistCount > 0)
					{
						int iSpecialistCount = getSpecialistCount((SpecialistTypes)iI);

						// the value is based on how close we are to our goal ratio forced/total
						int iForcedValue = ((iForcedSpecialistCount * 128) / iTotalForcedSpecialists) -  ((iSpecialistCount * 128) / iTotalSpecialists);
						if (iForcedValue >= iBestForcedValue)
						{
							int iSpecialistValue = AI_specialistValue((SpecialistTypes)iI, bAvoidGrowth, false);
							
							// if forced value larger, or if equal, does this specialist have a higher value
							if (iForcedValue > iBestForcedValue || iSpecialistValue > iBestSpecialistValue)
							{					
								iBestForcedValue = iForcedValue;
								iBestSpecialistValue = iSpecialistValue;
								eBestForcedSpecialist = ((SpecialistTypes)iI);
								eBestSpecialist = eBestForcedSpecialist;
							}
						}
					}
				}
			}
		}
		
		// if we do not have a best specialist yet, then just find the one with the best value
		if (eBestSpecialist == NO_SPECIALIST)
		{
			for (int iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
			{
				if (isSpecialistValid((SpecialistTypes)iI, 1))
				{
					int iValue = AI_specialistValue(((SpecialistTypes)iI), bAvoidGrowth, false);
					if (iValue >= iBestSpecialistValue)
					{
						iBestSpecialistValue = iValue;
						eBestSpecialist = ((SpecialistTypes)iI);
					}
				}
			}
		}
	}

	int iBestPlotValue = 0;
	int iBestPlot = -1;
	if (bWorkers)
	{
		for (int iI = 0; iI < NUM_CITY_PLOTS; iI++)
		{
			if (iI != CITY_HOME_PLOT)
			{
				if (!isWorkingPlot(iI))
				{
					CvPlot* pLoopPlot = getCityIndexPlot(iI);

					if (pLoopPlot != NULL)
					{
						if (canWork(pLoopPlot))
						{
							int iValue = AI_plotValue(pLoopPlot, bAvoidGrowth, /*bRemove*/ false, /*bIgnoreFood*/ false, bIgnoreGrowth);

							if (iValue > iBestPlotValue)
							{
								iBestPlotValue = iValue;
								iBestPlot = iI;
							}
						}
					}
				}
			}
		}
	}
	
	// if we found a plot to work
	if (iBestPlot != -1)
	{
		// if the best plot value is better than the best specialist, or if we forcing and we could not assign a forced specialst
		if (iBestPlotValue > iBestSpecialistValue || (bIsSpecialistForced && eBestForcedSpecialist == NO_SPECIALIST))
		{
			// do not work the specialist
			eBestSpecialist = NO_SPECIALIST;
		}
	}
	
	bool bSuccess = false;
	if (eBestSpecialist != NO_SPECIALIST)
	{
		changeSpecialistCount(eBestSpecialist, 1);
		bSuccess = true;
	}
	else if (iBestPlot != -1)
	{
		setWorkingPlot(iBestPlot, true);
		bSuccess = true;
	}

#ifdef COMPARE_NEW_CITY_GOVERNOR
	if (!m_bDisableTest)
	{
		FAssertMsg(bTestSuccess == bSuccess, "AI_AddBestCitizen and test_AI_AddBestCitizen had different success");
		AI_compareGovernorCitizens();
	}
#endif

	return bSuccess;
}


// Returns true if a worker was removed from a plot...
bool CvCityAI::AI_removeWorstCitizen(SpecialistTypes eIgnoreSpecialist)
{
#ifdef USE_NEW_CITY_GOVERNOR
	return test_AI_removeWorstCitizen(eIgnoreSpecialist);
#endif

#ifdef COMPARE_NEW_CITY_GOVERNOR
	bool bTestSuccess = true;
	if (!m_bDisableTest)
	{
		bTestSuccess = test_AI_removeWorstCitizen(eIgnoreSpecialist);
	}
#endif

	CvPlot* pLoopPlot;
	SpecialistTypes eWorstSpecialist;
	bool bAvoidGrowth;
	bool bIgnoreGrowth;
	int iWorstPlot;
	int iValue;
	int iWorstValue;
	int iI;
	
	bool bSuccess = false;

	// if we are using more specialists than the free ones we get
	if (extraFreeSpecialists() < 0)
	{
		// does generic 'citizen' specialist exist?
		if (GC.getDefineINT("DEFAULT_SPECIALIST") != NO_SPECIALIST)
		{
			// is ignore something other than generic citizen?
			if (eIgnoreSpecialist != GC.getDefineINT("DEFAULT_SPECIALIST"))
			{
				// do we have at least one more generic citizen than we are forcing?
				if (getSpecialistCount((SpecialistTypes)(GC.getDefineINT("DEFAULT_SPECIALIST"))) > getForceSpecialistCount((SpecialistTypes)(GC.getDefineINT("DEFAULT_SPECIALIST"))))
				{
					// remove the extra generic citzen
					changeSpecialistCount(((SpecialistTypes)(GC.getDefineINT("DEFAULT_SPECIALIST"))), -1);
					bSuccess = true;
				}
			}
		}
	}
	
	// if we did not just remove the extra generic citizen
	if (!bSuccess)
	{
		bAvoidGrowth = AI_avoidGrowth();
		bIgnoreGrowth = AI_ignoreGrowth();

		iWorstValue = MAX_INT;
		eWorstSpecialist = NO_SPECIALIST;
		iWorstPlot = -1;

		// if we are using more specialists than the free ones we get
		if (extraFreeSpecialists() < 0)
		{
			for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
			{
				if (eIgnoreSpecialist != iI)
				{
					if (getSpecialistCount((SpecialistTypes)iI) > getForceSpecialistCount((SpecialistTypes)iI))
					{
						iValue = AI_specialistValue(((SpecialistTypes)iI), bAvoidGrowth, /*bRemove*/ true);

						if (iValue < iWorstValue)
						{
							iWorstValue = iValue;
							eWorstSpecialist = ((SpecialistTypes)iI);
							iWorstPlot = -1;
						}
					}
				}
			}
		}
		
		// check all the plots we working
		for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
		{
			if (iI != CITY_HOME_PLOT)
			{
				if (isWorkingPlot(iI))
				{
					pLoopPlot = getCityIndexPlot(iI);

					if (pLoopPlot != NULL)
					{
						iValue = AI_plotValue(pLoopPlot, bAvoidGrowth, /*bRemove*/ true, /*bIgnoreFood*/ false, bIgnoreGrowth);

						if (iValue < iWorstValue)
						{
							iWorstValue = iValue;
							eWorstSpecialist = NO_SPECIALIST;
							iWorstPlot = iI;
						}
					}
				}
			}
		}

		if (eWorstSpecialist != NO_SPECIALIST)
		{
			changeSpecialistCount(eWorstSpecialist, -1);
			bSuccess = true;
		}
		else if (iWorstPlot != -1)
		{
			setWorkingPlot(iWorstPlot, false);
			bSuccess = true;
		}
	
		// if we still have not removed one, then try again, but do not ignore the one we were told to ignore
		if (!bSuccess)
		{
			if (extraFreeSpecialists() < 0)
			{
				for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
				{
					if (getSpecialistCount((SpecialistTypes)iI) > 0)
					{
						iValue = AI_specialistValue(((SpecialistTypes)iI), bAvoidGrowth, /*bRemove*/ true);

						if (iValue < iWorstValue)
						{
							iWorstValue = iValue;
							eWorstSpecialist = ((SpecialistTypes)iI);
							iWorstPlot = -1;
						}
					}
				}
			}

			if (eWorstSpecialist != NO_SPECIALIST)
			{
				changeSpecialistCount(eWorstSpecialist, -1);
				bSuccess = true;
			}
		}
	}

#ifdef COMPARE_NEW_CITY_GOVERNOR
	if (!m_bDisableTest)
	{
		FAssertMsg(bTestSuccess == bSuccess, "AI_removeWorstCitizen and test_AI_removeWorstCitizen had different success");
		AI_compareGovernorCitizens();
	}
#endif

	return bSuccess;
}


void CvCityAI::AI_juggleCitizens()
{
	bool bAvoidGrowth = AI_avoidGrowth();
	bool bIgnoreGrowth = AI_ignoreGrowth();
	bool bIsSpecialistForced = isSpecialistForced();
	
	// one at a time, remove the worst citizen, then add the best citizen
	// until we add back the same one we removed
	for (int iPass = 0; iPass < 2; iPass++)
	{
		bool bCompletedChecks = false;
		int iCount = 0;

		int iFirstWorstPlot = -1;

		while (!bCompletedChecks)
		{
			int iLowestValue = MAX_INT;
			int iWorstPlot = -1;
			int iValue;

			for (int iI = 0; iI < NUM_CITY_PLOTS; iI++)
			{
				if (iI != CITY_HOME_PLOT)
				{
					if (isWorkingPlot(iI))
					{
						CvPlot* pLoopPlot = getCityIndexPlot(iI);

						if (pLoopPlot != NULL)
						{
							if (!bCompletedChecks)
							{
								iValue = AI_plotValue(pLoopPlot, bAvoidGrowth, /*bRemove*/ true, /*bIgnoreFood*/ false, bIgnoreGrowth, (iPass == 0));

								// use <= so that we pick the last one that is lowest, to avoid infinite loop with AI_addBestCitizen
								if (iValue <= iLowestValue)
								{
									iLowestValue = iValue;
									iWorstPlot = iI;
								}
							}
						}
					}
				}
			}
			// if no worst plot, or we looped back around and are trying to remove the first plot we removed, stop
			if (iWorstPlot == -1 || iFirstWorstPlot == iWorstPlot)
			{
				bCompletedChecks = true;
			}

			if (!bCompletedChecks)
			{
				// if this the first worst plot, remember it
				if (iFirstWorstPlot == -1)
				{
					iFirstWorstPlot = iWorstPlot;
				}

				setWorkingPlot(iWorstPlot, false);

				if (AI_addBestCitizen(true, true))
				{
					if (isWorkingPlot(iWorstPlot))
					{
						bCompletedChecks = true;
					}
				}
			}

			iCount++;
			if (iCount > (NUM_CITY_PLOTS + 1))
			{
				FAssertMsg(false, "infinite loop(!!!!!)");
				break; // XXX
			}
		}

		if ((iPass == 0) && (foodDifference(false) >= 0))
		{
			//good enough, the starvation code
			break;
		}
	}
}


bool CvCityAI::AI_potentialPlot(short* piYields)
{
	int iNetFood = piYields[YIELD_FOOD] - GC.getFOOD_CONSUMPTION_PER_POPULATION();

	if (iNetFood < 0)
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


int CvCityAI::AI_yieldValue(short* piYields, bool bAvoidGrowth, bool bRemove, bool bIgnoreFood, bool bIgnoreGrowth, bool bIgnoreStarvation)
{
	PROFILE_FUNC();

	int aiYields[NUM_YIELD_TYPES];
	int iCommerceYields[NUM_COMMERCE_TYPES];

	int iExtraProductionModifier = 0;
	int iBaseProductionModifier = 100;
	
	bool bEmphasizeFood = AI_isEmphasizeYield(YIELD_FOOD);
	bool bFoodIsProduction = isFoodProduction();
	bool bCanPopRush = GET_PLAYER(getOwnerINLINE()).canPopRush();
	
	for (int iJ = 0; iJ < NUM_COMMERCE_TYPES; iJ++)
	{
		iCommerceYields[iJ] = 0;
	}

	for (int iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		if (piYields[iI] == 0)
		{
			aiYields[iI] = 0;
		}
		else
		{
			// Get yield for city after adding/removing the citizen in question
			int iOldCityYield = getBaseYieldRate((YieldTypes)iI);
			int iNewCityYield = (bRemove ? (iOldCityYield - piYields[iI]) : (iOldCityYield + piYields[iI]));
			
			int iModifier = getBaseYieldRateModifier((YieldTypes)iI);
			if (iI == YIELD_PRODUCTION)
			{
				iBaseProductionModifier = iModifier;
				iExtraProductionModifier = getProductionModifier();
				iModifier += iExtraProductionModifier;
			}

			iNewCityYield = (iNewCityYield * iModifier) / 100;
			iOldCityYield = (iOldCityYield * iModifier) / 100;

			// The yield of the citizen in question is the difference of total yields
			// to account for rounding of modifiers
			aiYields[iI] = (bRemove ? (iOldCityYield - iNewCityYield) : (iNewCityYield - iOldCityYield));
			
			if (iI == YIELD_COMMERCE)
			{
				for (int iJ = 0; iJ < NUM_COMMERCE_TYPES; iJ++)
				{
				    iModifier = getTotalCommerceRateModifier((CommerceTypes)iJ);
				    
                    int iNewCommerceComponent = (getCommerceFromPercent((CommerceTypes)iJ, iNewCityYield) * iModifier) / 100;
                    int iOldCommerceComponent = (getCommerceFromPercent((CommerceTypes)iJ, iOldCityYield) * iModifier) / 100;

					iCommerceYields[iJ] += (bRemove ? (iOldCommerceComponent - iNewCommerceComponent) : (iNewCommerceComponent - iOldCommerceComponent));
				}
			}
			
			if (iI == YIELD_PRODUCTION)
			{
				if (isProductionProcess())
				{
					aiYields[iI] = 0;
					for (int iJ = 0; iJ < NUM_COMMERCE_TYPES; iJ++)
					{
						iCommerceYields[iJ] += ((GC.getProcessInfo(getProductionProcess()).getProductionToCommerceModifier(iJ)) * (bRemove ? (iOldCityYield - iNewCityYield) : (iNewCityYield - iOldCityYield))) / 100;
					}
				}
			}			
		}
	}
	
	// should not really use this much, but making it accurate
	aiYields[YIELD_COMMERCE] = 0;
	for (int iJ = 0; iJ < NUM_COMMERCE_TYPES; iJ++)
	{
		aiYields[YIELD_COMMERCE] += iCommerceYields[iJ];
	}

	int iValue = 0;
	int iSlaveryValue = 0;
	
	int iFoodGrowthValue = 0;
	
	if (!bIgnoreFood && aiYields[YIELD_FOOD] != 0)
	{
		// tiny food factor, to insure that even when we dont want to grow, 
		// we still prefer more food if everything else is equal
		iValue += (aiYields[YIELD_FOOD] * 1);

		int iFoodPerTurn = (foodDifference(false) - ((bRemove) ? aiYields[YIELD_FOOD] : 0));
		int iFoodLevel = getFood();
		int iFoodToGrow = growthThreshold();
		int iHealthLevel = goodHealth() - badHealth(/*bNoAngry*/ false, 1);
		int iHappinessLevel = (isNoUnhappiness() ? max(3, iHealthLevel + 5) : happyLevel() - unhappyLevel(0));
		int iPopulation = getPopulation();
		int iWorkingPopulation = getWorkingPopulation();
		int	iExtraPopulationThatCanWork = min(iPopulation - range(-iHappinessLevel, 0, iPopulation) + min(0, extraFreeSpecialists()) , NUM_CITY_PLOTS) - getWorkingPopulation() + ((bRemove) ? 1 : 0);
		int iConsumtionPerPop = GC.getFOOD_CONSUMPTION_PER_POPULATION();

		int iAdjustedFoodDifference = getYieldRate(YIELD_FOOD) - ((iPopulation + min(0, iHappinessLevel)) * iConsumtionPerPop);
		
		// if we not human, allow us to starve to half full if avoiding growth
		if (!bIgnoreStarvation)
		{
			int iStarvingAllowance = 0;
			if (bAvoidGrowth && !isHuman())
			{
				iStarvingAllowance = max(0, (iFoodLevel - max(1, ((9 * iFoodToGrow) / 10))));
			}
			
			// if still starving
			if ((iFoodPerTurn + iStarvingAllowance) < 0)
			{
				// if working plots all like this one will save us from starving
				if (max(0, iExtraPopulationThatCanWork * aiYields[YIELD_FOOD]) >= -iFoodPerTurn)
				{
					// if this is high food, then we want to pick it first, this will allow us to pick some great non-food later
					int iHighFoodThreshold = iConsumtionPerPop + 1;				
					if (iFoodPerTurn <= -iHighFoodThreshold && aiYields[YIELD_FOOD] >= iHighFoodThreshold)
					{
						// value all the food that will contribute to not starving
						iValue += 2048 * min(aiYields[YIELD_FOOD], -iFoodPerTurn);
					}
					else
					{
						// give a huge boost to this plot, but not based on how much food it has
						// ie, if working a bunch of 1f 7h plots will stop us from starving, then do not force working unimproved 2f plot
						iValue += 4096;
					}
				}
				else
				{
					// value food high(32), but not forced
					iValue += aiYields[YIELD_FOOD] * 32;
				}
			}
		}
		
		// if food isnt production, then adjust for growth
		if (!bFoodIsProduction)
		{
			if (!bAvoidGrowth)
			{
				// only do relative checks on food if we want to grow AND we not emph food
				// the emp food case will just give a big boost to all food under all circumstances
				if (!bIgnoreGrowth && !bEmphasizeFood)
				{
					// also avail: iFoodLevel, iFoodToGrow
					
					// adjust iFoodPerTurn assuming that we work plots all equal to iConsumtionPerPop
					// that way it is our guesstimate of how much excess food we will have
					iFoodPerTurn += (iExtraPopulationThatCanWork * iConsumtionPerPop);
					
					// we have less than 10 extra happy, do some checks to see if we can increase it
					if (iHappinessLevel < 10)
					{
						// if we have anger becase no military, do not count it, on the assumption that it will 
						// be remedied soon, and that we still want to grow
						if (getMilitaryHappinessUnits() == 0)
						{
							iHappinessLevel += ((GC.getDefineINT("NO_MILITARY_PERCENT_ANGER") * (iPopulation + 1)) / GC.getDefineINT("PERCENT_ANGER_DIVISOR"));
						}

//						// if we can pop rush, let us grow one over the limit
//						if (bCanPopRush)
//						{
//							iHappinessLevel++;
//						}

						// currently we can at most increase happy by 2 in the following checks
						const int kMaxHappyIncrease = 2;

						// if happy is large enough so that it will be over zero after we do the checks
						int iNewFoodPerTurn = iFoodPerTurn + aiYields[YIELD_FOOD] - iConsumtionPerPop;
						if ((iHappinessLevel + kMaxHappyIncrease) > 0 && iNewFoodPerTurn > 0)
						{
							int iApproxTurnsToGrow = (iNewFoodPerTurn > 0) ? ((iFoodToGrow - iFoodLevel) / iNewFoodPerTurn) : MAX_INT;

							// do we have hurry anger?
							int iHurryAngerTimer = getHurryAngerTimer();
							if (iHurryAngerTimer > 0)
							{
								int iTurnsUntilAngerIsReduced = iHurryAngerTimer % flatHurryAngerLength();
								
								// angry population is bad but if we'll recover by the time we grow...
								if (iTurnsUntilAngerIsReduced <= iApproxTurnsToGrow)
								{
									iHappinessLevel++;
								}
							}

							// do we have conscript anger?
							int iConscriptAngerTimer = getConscriptAngerTimer();
							if (iConscriptAngerTimer > 0)
							{
								int iTurnsUntilAngerIsReduced = iConscriptAngerTimer % flatConscriptAngerLength();
								
								// angry population is bad but if we'll recover by the time we grow...
								if (iTurnsUntilAngerIsReduced <= iApproxTurnsToGrow)
								{
									iHappinessLevel++;
								}
							}
						}
					}

					int iPopToGrow = max(0, iHappinessLevel);
					iPopToGrow = min(iPopToGrow, AI_countGoodTiles(iHealthLevel > 0, true, 50) + ((bRemove) ? 1 : 0));

					// if we do not have unhappy, fill food bar to 85%
					bool bFillingBar = false;
					if (iHappinessLevel == 0)
					{
						if (iFoodLevel + iFoodPerTurn + aiYields[YIELD_FOOD] < 	((85 * iFoodToGrow) / 100))
						{
							iPopToGrow = 1;
							bFillingBar = true;
						}
					}
					
					// if we will push into growth we do not want (and we cannot pop rush), devalue our food
					if (iPopToGrow <= 0 && !bCanPopRush)
					{
						if (iFoodLevel + iFoodPerTurn + aiYields[YIELD_FOOD] >= iFoodToGrow)
						{
							iValue -= (aiYields[YIELD_FOOD] * 4);
						}
					}

					// if we want to grow
					if (iPopToGrow > 0)
					{
						int iUnhealthLosses = max(0, iPopToGrow - iHealthLevel);
						
						// will mulitply this by factors
						iFoodGrowthValue = aiYields[YIELD_FOOD];

						// (range 1-25) - we want to grow more if we have a lot of growth to do
						// factor goes up like this: 0:1, 1:8, 2:9, 3:10, 4:11, 5:13, 6:14, 7:15, 8:16, 9:17, ... 17:25
						int iFactorPopToGrow;

						if (iPopToGrow < 1 || bFillingBar)
							iFactorPopToGrow = 1;
						else if (iPopToGrow < 7)
							iFactorPopToGrow = 17 + 3 * iPopToGrow;
						else
							iFactorPopToGrow = 41;
						
						if (AI_isEmphasizeYield(YIELD_PRODUCTION) || AI_isEmphasizeYield(YIELD_COMMERCE) || AI_isEmphasizeGreatPeople())
						{
							iFactorPopToGrow *= 2;
						}
						// cap iFactorPopToGrow if this is a very poor tile
						if (iFactorPopToGrow > 1 &&
							aiYields[YIELD_FOOD] <= iConsumtionPerPop && (aiYields[YIELD_PRODUCTION] + aiYields[YIELD_COMMERCE]) < 1)
						{
							iFactorPopToGrow = 1;
						}
						iFoodGrowthValue *= iFactorPopToGrow;
						
						// (range 51-100) - we will grown slightly slower if we running unhealthy
						if (iUnhealthLosses > 0)
						{
							int iFactorUnhealth = 26 + max(0, 49 - iUnhealthLosses);
							iFoodGrowthValue *= iFactorUnhealth;
							iFoodGrowthValue /= 100;
						}
						//If we already grow somewhat fast, devalue further food
						//Remember growth acceleration is not dependent on food eaten per 
						//pop, 4f twice as fast as 2f twice as fast as 1f...
						if (iFoodPerTurn > 2)
						{
							iFoodGrowthValue *= 6;
							iFoodGrowthValue /= 4 + min(8, iFoodPerTurn);
						}
					}
				}
			}
			
			//very high food override
			if (!(bAvoidGrowth && isHuman()))
			{
				//very high food override
				int iTempValue = max(0, 30 * aiYields[YIELD_FOOD] - 15 * iConsumtionPerPop);
				iTempValue *= max(0, 3 * iConsumtionPerPop) - iAdjustedFoodDifference;
				iTempValue /= 3 * iConsumtionPerPop;
				iFoodGrowthValue += iTempValue;
			
				//Slavery Override
				if (bCanPopRush && (iHappinessLevel > 0) && (iHealthLevel > 0))
				{
					iSlaveryValue = 30 * 14 * aiYields[YIELD_FOOD];
					iSlaveryValue /= max(10, (growthThreshold() * (100 - getMaxFoodKeptPercent())));
					
					iSlaveryValue *= 100;
					iSlaveryValue /= getHurryCostModifier(true);
					
					iSlaveryValue *= iConsumtionPerPop * 2;
					iSlaveryValue /= iConsumtionPerPop * 2 + max(0, iAdjustedFoodDifference);
				}
			}
		}
	}
	
	
	int iProductionValue = 0;
	int iCommerceValue = 0;
	int iFoodValue = iFoodGrowthValue;
	// if food is production, the count it
	int adjustedYIELD_PRODUCTION = (((bFoodIsProduction) ? aiYields[YIELD_FOOD] : 0) + aiYields[YIELD_PRODUCTION]);
			
	// value production medium(15)
	iProductionValue += (adjustedYIELD_PRODUCTION * 15);
	
	// value commerce low(5 was 4)

	for (int iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		int iCommerceWeight = GET_PLAYER(getOwnerINLINE()).AI_commerceWeight((CommerceTypes)iI);
		iCommerceValue += (iCommerceWeight * (iCommerceYields[iI] * 6) * GET_PLAYER(getOwnerINLINE()).AI_averageCommerceExchange((CommerceTypes)iI)) / 10000;
	}

	if (bEmphasizeFood)
	{
		if (!bFoodIsProduction)
		{
			// value food extremely high(180)
			iFoodValue += (aiYields[YIELD_FOOD] * 180);
		}
	}
	
	if (AI_isEmphasizeYield(YIELD_PRODUCTION))
	{
		// value production high(80)
		iProductionValue += (adjustedYIELD_PRODUCTION * 80);
	}
	
	//Slavery translation
	if ((iSlaveryValue > adjustedYIELD_PRODUCTION) && (iSlaveryValue > iFoodValue))
	{
		//treat the food component as production
		iFoodValue = 0;
	}
	else
	{
		//treat it as just food
		iSlaveryValue = 0;
	}

	if (AI_isEmphasizeYield(YIELD_COMMERCE))
	{
		for (int iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
		{
			iCommerceValue += ((iCommerceYields[iI] * 40) * GET_PLAYER(getOwnerINLINE()).AI_averageCommerceExchange((CommerceTypes)iI)) / 100;
		}
	}

	for (int iJ = 0; iJ < NUM_COMMERCE_TYPES; iJ++)
	{
		if (AI_isEmphasizeCommerce((CommerceTypes) iJ))
		{
			// value the part of our commerce that goes to our emphasis medium (40)
			iCommerceValue += (iCommerceYields[iJ] * 40);
		}
	}
	
	//Lets have some fun with the multipliers, this basically bluntens the impact of
	//massive bonuses.....
	
	//normalize the production... this allows the system to account for rounding
	//and such while preventing an "out to lunch smoking weed" scenario with
	//unusually high transient production modifiers.
	//Other yields don't have transient bonuses in quite the same way.

	if (isFoodProduction())
	{
		iProductionValue *= 100 + AI_specialYieldMultiplier(YIELD_PRODUCTION);
		iProductionValue /= 100;		
	}
	else
	{
		iProductionValue *= iBaseProductionModifier;
		iProductionValue /= (iBaseProductionModifier + iExtraProductionModifier);
		
		iProductionValue += iSlaveryValue;
		iProductionValue *= (100 + AI_specialYieldMultiplier(YIELD_PRODUCTION));
		
		iProductionValue /= GET_PLAYER(getOwnerINLINE()).AI_averageYieldMultiplier(YIELD_PRODUCTION);
	}
	iValue += iProductionValue;
	
	
	iCommerceValue *= (100 + AI_specialYieldMultiplier(YIELD_COMMERCE));
	iCommerceValue /= GET_PLAYER(getOwnerINLINE()).AI_averageYieldMultiplier(YIELD_COMMERCE);
	iValue += iCommerceValue;
//	
	iFoodValue *= 100;
	iFoodValue /= GET_PLAYER(getOwnerINLINE()).AI_averageYieldMultiplier(YIELD_FOOD);
	iValue += iFoodValue;

	
	return iValue;
}

int CvCityAI::AI_plotValue(CvPlot* pPlot, bool bAvoidGrowth, bool bRemove, bool bIgnoreFood, bool bIgnoreGrowth, bool bIgnoreStarvation)
{
	PROFILE_FUNC();

#ifdef USE_NEW_CITY_GOVERNOR
	return AI_plotValue(pPlot);
#endif

	short aiYields[NUM_YIELD_TYPES];
	ImprovementTypes eCurrentImprovement;
	ImprovementTypes eFinalImprovement;
	int iYieldDiff;
	int iValue;
	int iI;
	int iTotalDiff;

	iValue = 0;
	iTotalDiff = 0;

	for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		aiYields[iI] = pPlot->getYield((YieldTypes)iI);
	}

	eCurrentImprovement = pPlot->getImprovementType();

	if (eCurrentImprovement != NO_IMPROVEMENT)
	{
		//eFinalImprovement = finalImprovementUpgrade(eCurrentImprovement);
		
		//in the case that improvements upgrade, use 2 upgrade levels higher for the
		//yield calculations.
		ImprovementTypes eUpgradeImprovement = (ImprovementTypes)GC.getImprovementInfo(eCurrentImprovement).getImprovementUpgrade();
		if (eUpgradeImprovement != NO_IMPROVEMENT)
		{
			ImprovementTypes eUpgradeImprovement2 = (ImprovementTypes)GC.getImprovementInfo(eUpgradeImprovement).getImprovementUpgrade();
			if (eUpgradeImprovement2 != NO_IMPROVEMENT)
			{
				eUpgradeImprovement = eUpgradeImprovement2;				
			}
		}
		eFinalImprovement = eUpgradeImprovement;
		
		if ((eFinalImprovement != NO_IMPROVEMENT) && (eFinalImprovement != eCurrentImprovement))
		{
			for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
			{
				iYieldDiff = (pPlot->calculateImprovementYieldChange(eFinalImprovement, ((YieldTypes)iI), getOwnerINLINE()) - pPlot->calculateImprovementYieldChange(eCurrentImprovement, ((YieldTypes)iI), getOwnerINLINE()));
				aiYields[iI] += iYieldDiff;
				iValue -= iYieldDiff * 50;
				iTotalDiff += iYieldDiff;
			}
		}
	}
	
	
	int iYieldValue = (AI_yieldValue(aiYields, bAvoidGrowth, bRemove, bIgnoreFood, bIgnoreGrowth, bIgnoreStarvation) * 100);
	// unless we are emph food (and also food not production)
	if (!(AI_isEmphasizeYield(YIELD_FOOD) && !isFoodProduction()))
		// if this plot is super bad (less than 2 food and less than combined 2 prod/commerce
		if (!AI_potentialPlot(aiYields))
			// undervalue it even more!
			iYieldValue /= 16;
	iValue += iYieldValue;

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
						iValue += 35;
					}
				}
			}
		}
	}

	if ((eCurrentImprovement != NO_IMPROVEMENT) && (GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementUpgrade() != NO_IMPROVEMENT))
	{
		iValue += 200;
		int iUpgradeTime = (GC.getGameINLINE().getImprovementUpgradeTime(eCurrentImprovement));
		if (iUpgradeTime > 0) //assert this?
		{
			int iUpgradePenalty = (100 * (iUpgradeTime - pPlot->getUpgradeProgress()));
			iUpgradePenalty *= (iTotalDiff * 5);
			iUpgradePenalty /= max(1, GC.getGameSpeedInfo(GC.getGame().getGameSpeedType()).getImprovementPercent());
			iValue -= iUpgradePenalty;
		}		
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



//Improved worker AI provided by Blake - thank you!
void CvCityAI::AI_bestPlotBuild(CvPlot* pPlot, int* piBestValue, BuildTypes* peBestBuild, int iFoodPriority, int iProductionPriority, int iCommercePriority, bool bChop)
{
	PROFILE_FUNC();

	CvCity* pCity;
	int aiFinalYields[NUM_YIELD_TYPES];
	int aiDiffYields[NUM_YIELD_TYPES];
	int aiBestDiffYields[NUM_YIELD_TYPES];
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

        CvPlot* pAdjacantPlot;
        CvPlot* pAdjacantPlot2;
        BonusTypes eTempBonus;

        //It looks unwieldly but the code has to be rigid to avoid "worker ADD"
        //where they keep connecting then disconnecting a crops resource or building
        //multiple farms to connect a single crop resource.
        //isFreshWater is used to ensure invalid plots are pruned early, the inner loop
        //really doesn't run that often.

        //using logic along the lines of "Will irrigating me make crops wet"
        //wont really work... it does have to "am i the tile the crops want to be irrigated"

        //I optimized through the use of "isIrrigated" which is just checking a bool...
        //once everything is nicely irrigated, this code should be really fast...
        if ((pPlot->isIrrigated()) || (pPlot->isFreshWater() && pPlot->canHavePotentialIrrigation()))
        {
            for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
            {
                pAdjacantPlot = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), ((DirectionTypes)iI));

                if ((pAdjacantPlot != NULL) && (pAdjacantPlot->getOwner() == getOwner()) && (pAdjacantPlot->isCityRadius()))
                {
                    if (!pAdjacantPlot->isFreshWater())
                    {
                        //check for a city? cities can conduct irrigation and that effect is quite
                        //useful... so I think irrigate cities.
                        if (pAdjacantPlot->isPotentialIrrigation())
                        {
                            CvPlot* eBestIrrigationPlot = NULL;


                            for (iJ = 0; iJ < NUM_DIRECTION_TYPES; iJ++)
                            {
                                pAdjacantPlot2 = plotDirection(pAdjacantPlot->getX_INLINE(), pAdjacantPlot->getY_INLINE(), ((DirectionTypes)iJ));
                                if ((pAdjacantPlot2 != NULL) && (pAdjacantPlot2->getOwner() == getOwner()))
                                {
	                                eTempBonus = pAdjacantPlot2->getNonObsoleteBonusType(getTeam());
                                    if (pAdjacantPlot->isIrrigated())
                                    {
                                        //the irrigation has to be coming from somewhere
                                        if (pAdjacantPlot2->isIrrigated())
                                        {
                                            //if we find a tile which is already carrying irrigation
                                            //then lets not replace that one...
                                            eBestIrrigationPlot = pAdjacantPlot2;

                                            if ((pAdjacantPlot2->isCity()) || (eTempBonus != NO_BONUS) || (!pAdjacantPlot2->isCityRadius()))
                                            {
                                            	if (pAdjacantPlot2->isFreshWater())
                                            	{
                                                	//these are all ideal for irrigation chains so stop looking.
                                                	break;
                                            	}
                                            }
                                        }

                                    }
                                    else
                                    {
                                        if (pAdjacantPlot2->getNonObsoleteBonusType(getTeam()) == NO_BONUS)
                                        {
                                            if (pAdjacantPlot2->canHavePotentialIrrigation() && pAdjacantPlot2->isIrrigationAvailable())
                                            {
                                                //could use more sophisticated logic
                                                //however this would rely on things like smart irrigation chaining
                                                //of out-of-city plots
                                                eBestIrrigationPlot = pAdjacantPlot2;
                                                break;
                                            }
                                        }
                                    }
                                }
                            }

                            if (pPlot == eBestIrrigationPlot)
                            {
                                bEmphasizeIrrigation = true;
                                break;
                            }
                        }
                    }
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
				iValue += (aiTempYields[YIELD_COMMERCE] * 4);


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

                    iValue *= 2;
					for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
					{
						aiFinalYields[iJ] = 2*(pPlot->calculateNatureYield(((YieldTypes)iJ), getTeam(), bIgnoreFeature));
						aiFinalYields[iJ] += (pPlot->calculateImprovementYieldChange(eFinalImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false));
						aiFinalYields[iJ] += (pPlot->calculateImprovementYieldChange(eImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false));
						aiDiffYields[iJ] = (aiFinalYields[iJ] - (2 * pPlot->getYield(((YieldTypes)iJ))));
					}


					iValue += (aiDiffYields[YIELD_FOOD] * ((100 * iFoodPriority) / 100));
					iValue += (aiDiffYields[YIELD_PRODUCTION] * ((60 * iProductionPriority) / 100));
					iValue += (aiDiffYields[YIELD_COMMERCE] * ((40 * iCommercePriority) / 100));


					// this is mainly to make it improve better tiles first
					//flood plain > grassland > plain > tundra
					iValue += (aiFinalYields[YIELD_FOOD] * 10);
					iValue += (aiFinalYields[YIELD_PRODUCTION] * 6);
					iValue += (aiFinalYields[YIELD_COMMERCE] * 4);

					iValue /= 2;

                    for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
					{
					    aiFinalYields[iJ] /= 2;
					    aiDiffYields[iJ] /= 2;
					}



					if (aiFinalYields[YIELD_FOOD] >= GC.getFOOD_CONSUMPTION_PER_POPULATION())
					{
						//this is a food yielding tile
						if (iFoodPriority > 100)
						{
							iValue *= iFoodPriority;
							iValue /= 100;							
						}
						if (iCommercePriority > 100)
						{
							iValue *= 100 + (((iCommercePriority - 100) * aiDiffYields[YIELD_COMMERCE]) / 4);
							iValue /= 100;
						}
					}
					else if (aiFinalYields[YIELD_FOOD] < GC.getFOOD_CONSUMPTION_PER_POPULATION())
					{
						if (iFoodPriority > 100)
						{
						    if (eNonObsoleteBonus == NO_BONUS)
						    {
                                iValue *= 75;
                                iValue /= iFoodPriority;
                                //if the city needs food there's little sense in improving tiles
                                //which can't be worked.
						    }
						}
						else
						{
							if ((aiDiffYields[YIELD_PRODUCTION] > 0) && (aiFinalYields[YIELD_FOOD]+aiFinalYields[YIELD_PRODUCTION] > 3))
							{
								//value booster for mines on hills
								iValue *= (100 + 25 * aiDiffYields[YIELD_PRODUCTION]);
								iValue /= 100;
							}
						}

						if ((iFoodPriority < 100) && (iProductionPriority > 100))
						{
						    iValue *= (200 + iProductionPriority);
						    iValue /= 200;
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
							iValue *= 5;
							iValue /= 4;
							iValue += 1;
						}

						if (eBestTempBuild != NO_BUILD)
						{
							if (pPlot->getFeatureType() != NO_FEATURE)
							{
								if (GC.getBuildInfo(eBestTempBuild).isFeatureRemove(pPlot->getFeatureType()))
								{
									iValue += pPlot->getFeatureProduction(eBestTempBuild, getTeam(), &pCity) * 2;
									FAssert(pCity == this);

									//avoid chopping forests if needed for health
									iValue -= ((GC.getFeatureInfo(pPlot->getFeatureType()).getHealthPercent() * (1 + max(1, -healthRate(false, 4)))) / (pPlot->isBeingWorked() ? 4 : 2));

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
							    iValue /= 2;
								iValue -= 50;
							}
						}

						if (eNonObsoleteBonus == NO_BONUS)
						{
							if (isWorkingPlot(pPlot))
							{
                                iValue -= 70;
                                iValue *= 2;
                                iValue /= 3;
							}
						}
					}

					if (iValue > iBestValue)
					{
						iBestValue = iValue;
						eBestBuild = eBestTempBuild;
						
						for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
						{
							aiBestDiffYields[iJ] = aiDiffYields[iJ];
						}
					}
				}
			}
		}
	}

    //clear jungle
	//if (eBestBuild == NO_BUILD)
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
								iValue = 100;
								iValue += (pPlot->getFeatureProduction(eBuild, getTeam(), &pCity) * 10);
								iValue += ((GC.getFeatureInfo(pPlot->getFeatureType()).getHealthPercent() * -healthRate(false, 1)));
								FAssert(pCity == this);
								
								iValue *= 500;
								iValue /= (GC.getBuildInfo(eBuild).getFeatureTime(pPlot->getFeatureType()) + 100);

								// XXX feature production???
								if (bHasBonusImprovement)
								{
									//clear jumbo of the jungle
									iValue += 500;
								}

								if ((iValue > iBestValue) || ((iValue > 0) && (eBestBuild == NO_BUILD)))
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

	//Chop - maybe integrate this better with the other feature-clear code tho the logic
	//is kinda different
    if (bChop && (eBonus == NO_BONUS) && (pPlot->getFeatureType() != NO_FEATURE) &&
        (pPlot->getImprovementType() == NO_IMPROVEMENT) && !(GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_LEAVE_FORESTS)))
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
                        iValue = (pPlot->getFeatureProduction(eBuild, getTeam(), &pCity)) * 10;
                        FAssert(pCity == this);

                        if (iValue > 0)
                        {
                            int iFeatureHealthPercent = GC.getFeatureInfo(pPlot->getFeatureType()).getHealthPercent();
							if (iFeatureHealthPercent > 0)
							{
								int iHippyFactor = ((iFeatureHealthPercent * -healthRate(false, 5)));
								iHippyFactor += (pPlot->isBeingWorked() ? 300 : 0);

								iValue *= (400 - iHippyFactor);
								iValue /= 400;
								iValue = max(0, iValue);
							}

                            if (iValue > 0)
                            {
								if (GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_DAGGER))
								{
									iValue += 20;
									iValue *= 2;
								}
                                iValue *= 500;
                                iValue /= max(1, (GC.getBuildInfo(eBuild).getFeatureTime(pPlot->getFeatureType()) + 100));

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


    for (iI = 0; iI < GC.getNumRouteInfos(); iI++)
    {
        RouteTypes eRoute = (RouteTypes)iI;
        RouteTypes eOldRoute = pPlot->getRouteType();

        if (eRoute != eOldRoute)
        {
        	int iTempValue = 0;
            if (pPlot->getImprovementType() != NO_IMPROVEMENT)
            {
                if ((eOldRoute == NO_ROUTE) || (GC.getRouteInfo(eRoute).getValue() > GC.getRouteInfo(eOldRoute).getValue()))
                {
                    iTempValue += ((GC.getImprovementInfo(pPlot->getImprovementType()).getRouteYieldChanges(eRoute, YIELD_FOOD)) * 100);
                    iTempValue += ((GC.getImprovementInfo(pPlot->getImprovementType()).getRouteYieldChanges(eRoute, YIELD_PRODUCTION)) * 60);
                    iTempValue += ((GC.getImprovementInfo(pPlot->getImprovementType()).getRouteYieldChanges(eRoute, YIELD_COMMERCE)) * 40);
                }
				//road up bonuses if sort of bored.
				if ((eOldRoute == NO_ROUTE) && (eBonus != NO_BONUS))
				{
					iTempValue += (pPlot->isConnectedToCapital() ? 10 : 30);
				}
            }
                                
			if (iTempValue > 0)
			{
				for (iJ = 0; iJ < GC.getNumBuildInfos(); iJ++)
				{
					eBuild = ((BuildTypes)iJ);
					if (GC.getBuildInfo(eBuild).getRoute() == eRoute)
					{
						if (GET_PLAYER(getOwnerINLINE()).canBuild(pPlot, eBuild, false))
						{
							//the value multiplier is based on the default time...
							iValue = iTempValue * 5 * 300;
							iValue /= GC.getBuildInfo(eBuild).getTime();

							if ((iValue > iBestValue) || ((iValue > 0) && (eBestBuild == NO_BUILD)))
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



	if (eBestBuild != NO_BUILD)
	{
		FAssertMsg(iBestValue > 0, "iBestValue is expected to be greater than 0");
		
		//Now modify the priority for this build.
		if (GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble())
		{
			if (GC.getBuildInfo(eBestBuild).getImprovement() != NO_IMPROVEMENT)
			{
				iBestValue += (iBestValue * aiBestDiffYields[YIELD_COMMERCE]) / 4;
				iBestValue = max(1, iBestValue);
			}
			
		}

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


bool CvCityAI::AI_doPanic()
{
	
	bool bLandWar = ((area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE) || (area()->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE) || (area()->getAreaAIType(getTeam()) == AREAAI_MASSING));
	
	if (bLandWar)
	{
		int iOurDefense = GET_PLAYER(getOwnerINLINE()).AI_getOurPlotStrength(plot(), 0, true, false);
		int iEnemyOffense = GET_PLAYER(getOwnerINLINE()).AI_getEnemyPlotStrength(plot(), 2, false, false);
		int iRatio = (100 * iEnemyOffense) / (max(1, iOurDefense));

		if (iRatio > 100)
		{
			UnitTypes eProductionUnit = getProductionUnit();

			if (eProductionUnit != NO_UNIT)
			{
				if (getProduction() > 0)
				{
					if (GC.getUnitInfo(eProductionUnit).getCombat() > 0)
					{
						AI_doHurry(true);
						return true;
					}
				}
			}
			else
			{
				if ((GC.getGame().getSorenRandNum(2, "AI choose panic unit") == 0) && AI_chooseUnit(UNITAI_CITY_COUNTER))
				{
					AI_doHurry((iRatio > 140));	
				}
				else if (AI_chooseUnit(UNITAI_CITY_DEFENSE))
				{
					AI_doHurry((iRatio > 140));
				}
				else if (AI_chooseUnit(UNITAI_ATTACK))
				{
					AI_doHurry((iRatio > 140));
				}
			}
		}
	}
	return false;
}




int CvCityAI::AI_calculateCulturePressure(bool bGreatWork)
{
    CvPlot* pLoopPlot;
    BonusTypes eNonObsoleteBonus;
    int iValue;
    int iTempValue;
    int iI;

    iValue = 0;
    for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
    {
        pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);
		if (pLoopPlot != NULL)
		{
		    if (pLoopPlot->getOwnerINLINE() == NO_PLAYER)
		    {
		        iValue++;
		    }
		    else
		    {
                iTempValue = pLoopPlot->calculateCulturePercent(getOwnerINLINE());
                if (iTempValue == 100)
                {
                    //do nothing
                }
                else if ((iTempValue == 0) || (iTempValue > 75))
                {
                    iValue++;
                }
                else
                {
                    iTempValue = (100 - iTempValue);
                    FAssert(iTempValue > 0);
                    FAssert(iTempValue <= 100);

                    if (iI != CITY_HOME_PLOT)
                    {
                        iTempValue *= 4;
                        iTempValue /= NUM_CITY_PLOTS;
                    }

                    eNonObsoleteBonus = pLoopPlot->getNonObsoleteBonusType(getTeam());

                    if (eNonObsoleteBonus != NO_BONUS)
                    {
                        iTempValue += (GET_PLAYER(getOwnerINLINE()).AI_bonusVal(eNonObsoleteBonus) * ((GET_PLAYER(getOwnerINLINE()).getNumTradeableBonuses(eNonObsoleteBonus) == 0) ? 4 : 2));
                    }

                    if ((iTempValue > 80) && (pLoopPlot->getOwnerINLINE() == getID()))
                    {
                        //captured territory special case
                        iTempValue *= (100 - iTempValue);
                        iTempValue /= 100;
                    }

                    if (pLoopPlot->getTeam() == getTeam())
                    {
                        iTempValue /= (bGreatWork ? 10 : 2);
                    }
                    else
                    {
                        iTempValue *= 2;
                        if (bGreatWork)
                        {
                            if (GET_PLAYER(getOwnerINLINE()).AI_getAttitude(pLoopPlot->getOwnerINLINE()) == ATTITUDE_FRIENDLY)
                            {
                                iValue /= 10;
                            }
                        }
                    }

                    iValue += iTempValue;
                }
            }
		}
    }


    return iValue;
}


void CvCityAI::AI_buildGovernorChooseProduction()
{
	PROFILE_FUNC();

	CvArea* pWaterArea;
	bool bWasFoodProduction;
	bool bDanger;
	int iCulturePressure;

	bDanger = AI_isDanger();


	// only clear the dirty bit if we actually do a check, multiple items might be queued
	AI_setChooseProductionDirty(false);

	pWaterArea = waterArea();

	bWasFoodProduction = isFoodProduction();
	iCulturePressure = AI_calculateCulturePressure();
	int iNumCitiesInArea = area()->getCitiesPerPlayer(getOwnerINLINE());
	int iMinValueDivisor = 1;
	if (getPopulation() < 3)
	{
		iMinValueDivisor = 3;
	}
	else if (getPopulation() < 7)
	{
		iMinValueDivisor = 2;
	}
	
	
	clearOrderQueue();

	if (bWasFoodProduction)
	{
		AI_assignWorkingPlots();
	}
	
    // if we need to pop borders, then do that immediately if we have drama and can do it
	if ((getCultureLevel() <= (CultureLevelTypes)1) && ((getCommerceRate(COMMERCE_CULTURE) < 2) || (iCulturePressure > 0)))
	{
        if (AI_chooseProcess(COMMERCE_CULTURE))
        {
            return;
        }
	}
	
	//workboat
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

	if ((AI_countNumBonuses(NO_BONUS, false, true, 10, true, true) > 0)
		&& (getPopulation() > AI_countNumBonuses(NO_BONUS, true, false, -1, true, true)))
	{
		if (getCommerceRate(COMMERCE_CULTURE) == 0)
		{
			AI_chooseBuilding(BUILDINGFOCUS_CULTURE);
			return;
		}
	}

    // pick granary or lighthouse, any duration
    if (AI_chooseBuilding(BUILDINGFOCUS_FOOD))
    {
        return;
    }
    
    if (angryPopulation(1) > 1)
    {
        if (AI_chooseBuilding(BUILDINGFOCUS_HAPPY, 40))
        {
            return;
        }
    }
    
	if (AI_chooseBuilding(BUILDINGFOCUS_PRODUCTION, 30, 10 / iMinValueDivisor))
    {
        return;
    }
    
	if (AI_chooseBuilding(BUILDINGFOCUS_EXPERIENCE, 8, 33))
    {
        return;
    }

    
	if (((getCommerceRateTimes100(COMMERCE_CULTURE) == 0) && (iCulturePressure != 0))
        || (iCulturePressure > 100))
    {
        if (AI_chooseBuilding(BUILDINGFOCUS_CULTURE, 30))
        {
            return;
        }
    }

    
	int iEconomyFlags = 0;
	iEconomyFlags |= BUILDINGFOCUS_GOLD;
	iEconomyFlags |= BUILDINGFOCUS_RESEARCH;
	iEconomyFlags |= BUILDINGFOCUS_MAINTENANCE;
	iEconomyFlags |= BUILDINGFOCUS_HAPPY;
	iEconomyFlags |= BUILDINGFOCUS_HEALTHY;
	iEconomyFlags |= BUILDINGFOCUS_SPECIALIST;
	
	//20 means 5g or ~2 happiness...
	if (AI_chooseBuilding(iEconomyFlags, 20, 20 / iMinValueDivisor))
	{
		return;
	}
	
	int iExistingWorkers = GET_PLAYER(getOwner()).AI_totalAreaUnitAIs(area(), UNITAI_WORKER);
    int iNeededWorkers = GET_PLAYER(getOwner()).AI_neededWorkers(area());
    
	if (!bDanger && (iExistingWorkers < ((iNeededWorkers + 1) / 2)))
	{
		if (AI_chooseUnit(UNITAI_WORKER))
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
    
	if (AI_chooseBuilding(iEconomyFlags, 40, 15 / iMinValueDivisor))
	{
		return;
	}
	
	if (AI_chooseBuilding(iEconomyFlags | BUILDINGFOCUS_CULTURE, 10, 10 / iMinValueDivisor))
	{
		return;
	}


	if (AI_chooseProcess())
	{
		return;
	}

	if (AI_chooseBuilding())
	{
		return;
	}
	
    if (AI_chooseUnit())
	{
		return;
	}
}

int CvCityAI::AI_calculateWaterWorldPercent()
{
    int iI;
    int iWaterPercent = 0;
    int iTeamCityCount = 0;
	int iOtherCityCount = 0;
	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		if (GET_TEAM((TeamTypes)iI).isAlive())
		{
			if (iI == getTeam() || GET_TEAM((TeamTypes)iI).isVassal(getTeam())
				|| GET_TEAM(getTeam()).isVassal((TeamTypes)iI))
			{
				iTeamCityCount += GET_TEAM((TeamTypes)iI).countNumCitiesByArea(area());
			}
			else
			{
				iOtherCityCount += GET_TEAM((TeamTypes)iI).countNumCitiesByArea(area());
			}
		}
	}

    if (iOtherCityCount == 0)
    {
        iWaterPercent = 100;
    }
    else
    {
        iWaterPercent = 100 - ((iTeamCityCount + iOtherCityCount) * 100) / max(1, (GC.getGame().getNumCities()));
    }
    
    iWaterPercent *= 50;
    iWaterPercent /= 100;
    
    iWaterPercent += (50 * (2 + iTeamCityCount)) / (2 + iTeamCityCount + iOtherCityCount);
    
    iWaterPercent = max(1, iWaterPercent);
    
    
    return iWaterPercent;
}

bool CvCityAI::AI_isCityRole(int iFlag) const
{
    FAssertMsg(m_iCityRoleFlags & AI_CITY_ROLE_VALID, "City Role not set! Mod just loaded?");
    return (m_iCityRoleFlags && iFlag);
    
}

void CvCityAI::AI_setCityRole(int iOnFlags, int iOffFlags)
{
    m_iCityRoleFlags |= iOnFlags;
    m_iCityRoleFlags &= ~iOffFlags;
}

void CvCityAI::AI_resetCityRoles()
{
    m_iCityRoleFlags = AI_CITY_ROLE_VALID;
}


//The magic value is basically "Look at this plot, is it worth working"
//-50 or lower means the plot is worthless in a "workers kill yourself" kind of way.
//-50 to -1 means the plot isn't worth growing to work - might be okay with emphasize though.
//Between 0 and 50 means it is marginal.
//50-100 means it's okay.
//Above 100 means it's definitely decent - seriously question ever not working it.
//This function deliberately doesn't use emphasize settings.
int CvCityAI::AI_getPlotMagicValue(CvPlot* pPlot, bool bHealthy)
{
    short aiYields[NUM_YIELD_TYPES];
    ImprovementTypes eCurrentImprovement;
    ImprovementTypes eFinalImprovement;
    int iI;
    int iYieldDiff;
    
    int iValue = 0;
    
    FAssert(pPlot != NULL);

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
    
    int iPopEats = GC.getFOOD_CONSUMPTION_PER_POPULATION();
    iPopEats += (bHealthy ? 0 : 1);


    iValue = ((aiYields[YIELD_FOOD] * 100 + aiYields[YIELD_PRODUCTION]*55 + aiYields[YIELD_COMMERCE]*40) - iPopEats * 102);
    return iValue;
}

//useful for deciding whether or not to grow... or whether the city needs terrain
//improvement.
//if healthy is false it assumes bad health conditions.
int CvCityAI::AI_countGoodTiles(bool bHealthy, bool bUnworkedOnly, int iThreshold)
{
    CvPlot* pLoopPlot;
    int iI;
    int iCount;
    
    iCount = 0;
    for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
    {
        pLoopPlot = plotCity(getX_INLINE(),getY_INLINE(), iI);
        if ((iI != CITY_HOME_PLOT) && (pLoopPlot != NULL))
        {
            if (pLoopPlot->getWorkingCity() == this)
            {
                if (!bUnworkedOnly || !(pLoopPlot->isBeingWorked()))
                {
                    if (AI_getPlotMagicValue(pLoopPlot, bHealthy) > iThreshold)
                    {
                        iCount++;
                    }                    
                }
            }
        }
    }
    return iCount;
}

//0 is normal
//higher than zero means special.
int CvCityAI::AI_getCityImportance(bool bEconomy, bool bMilitary)
{
    int iValue = 0;
    if (GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_CULTURE2))
    {
        int iCultureRateRank = findCommerceRateRank(COMMERCE_CULTURE);
        int iCulturalVictoryNumCultureCities = GC.getGameINLINE().culturalVictoryNumCultureCities();
        
        if (iCultureRateRank <= iCulturalVictoryNumCultureCities)
        {
            iValue += 100;
        
            if ((getCultureLevel() < (GC.getNumCultureLevelInfos() - 1)))
            {
                iValue += !bMilitary ? 100 : 0;
            }
            else
            {
                iValue += bMilitary ? 100 : 0;
            }
        }
    }
    
    return iValue;
}

void CvCityAI::AI_stealPlots()
{
    PROFILE_FUNC();
    CvPlot* pLoopPlot;
    CvCityAI* pWorkingCity;
    int iI;
    int iOtherImportance;
    
    int iImportance = AI_getCityImportance(true, false);

    for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
    {
        pLoopPlot = plotCity(getX_INLINE(),getY_INLINE(),iI);
        
        if (pLoopPlot != NULL)
        {
            if (iImportance > 0)
            {
                if (pLoopPlot->getOwnerINLINE() == getOwnerINLINE())
                {
                    pWorkingCity = static_cast<CvCityAI*>(pLoopPlot->getWorkingCity());
                    if ((pWorkingCity != this) && (pWorkingCity != NULL))
                    {
                        FAssert(pWorkingCity->getOwnerINLINE() == getOwnerINLINE());
                        iOtherImportance = pWorkingCity->AI_getCityImportance(true, false);
                        if (iImportance > iOtherImportance)
                        {
                            pLoopPlot->setWorkingCityOverride(this);
                        }
                    }
                }
            }
                
            if (pLoopPlot->getWorkingCityOverride() == this)
            {
                if (pLoopPlot->getOwnerINLINE() != getOwnerINLINE())
                {
                    pLoopPlot->setWorkingCityOverride(NULL);                    
                }
            }
        }
    }
}




// +1/+3/+5 plot based on base food yield (1/2/3)
// +4 if being worked.
// +4 if a bonus.
// Unworked ocean ranks very lowly. Unworked lake ranks at 3. Worked lake at 7.
// Worked bonus in ocean ranks at like 11
int CvCityAI::AI_buildingSpecialYieldChangeValue(BuildingTypes eBuilding, YieldTypes eYield)
{
    int iI;
    CvPlot* pLoopPlot;
    int iValue = 0;
    CvBuildingInfo& kBuilding = GC.getBuildingInfo(eBuilding);
    
    int iYieldChange = kBuilding.getSeaPlotYieldChange(eYield);
    if (iYieldChange > 0)
    {
        int iWaterCount = 0;
        int iWorkedWaterCount = 0;
        for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
        {
            if (iI != CITY_HOME_PLOT)
            {
                pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);
                if ((pLoopPlot != NULL) && (pLoopPlot->getWorkingCity() == this))
                {
                    if (pLoopPlot->isWater())
                    {
                        iWaterCount++;
                        iValue += max(0, (pLoopPlot->calculateNatureYield(YIELD_FOOD, getTeam(), false)) * 2 - 1);
                        iValue += (pLoopPlot->isBeingWorked() ? 4 : 0);
                        iValue += ((pLoopPlot->getBonusType(getTeam()) != NO_BONUS) ? 4 : 0);
                    }
                }
            }
        }
    }
     
    

    return iValue;
}


int CvCityAI::AI_yieldMultiplier(YieldTypes eYield)
{
	PROFILE_FUNC();
	
	int iMultiplier = getBaseYieldRateModifier(eYield);
	
	if (eYield == YIELD_PRODUCTION)
	{
		iMultiplier += (getMilitaryProductionModifier() / 2);
	}
	
	if (eYield == YIELD_COMMERCE)
	{
		iMultiplier += (getCommerceRateModifier(COMMERCE_RESEARCH) * 60) / 100;
		iMultiplier += (getCommerceRateModifier(COMMERCE_GOLD) * 35) / 100;
		iMultiplier += (getCommerceRateModifier(COMMERCE_CULTURE) * 15) / 100;
	}
	
	return iMultiplier;	
}
//this should be called before doing governor stuff.
void CvCityAI::AI_updateSpecialYieldMultiplier()
{
	PROFILE_FUNC();
	
	for (int iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		m_aiSpecialYieldMultiplier[iI] = 0;
	}
	
	UnitTypes eProductionUnit = getProductionUnit();
	if (eProductionUnit != NO_UNIT)
	{
		if (GC.getUnitInfo(eProductionUnit).getDefaultUnitAIType() == UNITAI_WORKER_SEA)
		{
			m_aiSpecialYieldMultiplier[YIELD_PRODUCTION] += 50;
			m_aiSpecialYieldMultiplier[YIELD_COMMERCE] -= 50;				
		}
		if ((GC.getUnitInfo(eProductionUnit).getDefaultUnitAIType() == UNITAI_WORKER) ||
			(GC.getUnitInfo(eProductionUnit).getDefaultUnitAIType() == UNITAI_SETTLE))
		
		{
			m_aiSpecialYieldMultiplier[YIELD_COMMERCE] -= 50;			
		}
	}

	BuildingTypes eProductionBuilding = getProductionBuilding();
	if (eProductionBuilding != NO_BUILDING)
	{
		if (isWorldWonderClass((BuildingClassTypes)(GC.getBuildingInfo(eProductionBuilding).getBuildingClassType()))
			|| isProductionProject())
		{
			m_aiSpecialYieldMultiplier[YIELD_PRODUCTION] += 50;
			m_aiSpecialYieldMultiplier[YIELD_COMMERCE] -= 25;					
		}
		m_aiSpecialYieldMultiplier[YIELD_PRODUCTION] += max(-25, GC.getBuildingInfo(eProductionBuilding).getFoodKept());
	}
	
	// non-human production value increase
	if (!isHuman())
	{
		AreaAITypes eAreaAIType = area()->getAreaAIType(getTeam());
		
		if ((GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_DAGGER) && getPopulation() >= 4)
			|| (eAreaAIType == AREAAI_OFFENSIVE) || (eAreaAIType == AREAAI_DEFENSIVE) 
			|| (eAreaAIType == AREAAI_MASSING) || (eAreaAIType == AREAAI_ASSAULT))
		{
			m_aiSpecialYieldMultiplier[YIELD_PRODUCTION] += 10;
			if (!GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble())
			{
				m_aiSpecialYieldMultiplier[YIELD_COMMERCE] -= 40;
			}
		}
	}
}

int CvCityAI::AI_specialYieldMultiplier(YieldTypes eYield)
{
	return m_aiSpecialYieldMultiplier[eYield];
}


int CvCityAI::AI_countNumBonuses(BonusTypes eBonus, bool bIncludeOurs, bool bIncludeNeutral, int iOtherCultureThreshold, bool bLand, bool bWater)
{
    CvPlot* pLoopPlot;
    BonusTypes eLoopBonus;
    int iI;
    int iCount = 0;
    for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
    {
        pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);
        
        if (pLoopPlot != NULL)
        {
        	if ((pLoopPlot->area() == area()) || (bWater && pLoopPlot->isWater()))
        	{
				eLoopBonus = pLoopPlot->getBonusType(getTeam());
				if (eLoopBonus != NO_BONUS)
				{
					if ((eBonus == NO_BONUS) || (eBonus == eLoopBonus))
					{
						if (bIncludeOurs && (pLoopPlot->getOwnerINLINE() == getOwnerINLINE()) && (pLoopPlot->getWorkingCity() == this))
						{
							iCount++;                    
						}
						else if (bIncludeNeutral && (!pLoopPlot->isOwned()))
						{
							iCount++;
						}
						else if ((iOtherCultureThreshold > 0) && (pLoopPlot->isOwned() && (pLoopPlot->getOwnerINLINE() != getOwnerINLINE())))
						{
							if ((pLoopPlot->getCulture(pLoopPlot->getOwnerINLINE()) - pLoopPlot->getCulture(getOwnerINLINE())) < iOtherCultureThreshold)
							{
								iCount++;
							}                        
						}
					}
				}
        	}
        }
    }
    
    
    return iCount;    
    
}

int CvCityAI::AI_playerCloseness(PlayerTypes eIndex, int iMaxDistance)
{
	FAssert(GET_PLAYER(eIndex).isAlive());
	FAssert(eIndex != getID());
	
	if ((m_iCachePlayerClosenessTurn != GC.getGame().getGameTurn())
		|| (m_iCachePlayerClosenessDistance != iMaxDistance))
	{
		AI_cachePlayerCloseness(iMaxDistance);
	}
	
	return m_aiPlayerCloseness[eIndex];	
}

void CvCityAI::AI_cachePlayerCloseness(int iMaxDistance)
{
	CvCity* pLoopCity;
	int iI;
	int iLoop;
	int iValue;
	int iTempValue;
	int iBestValue;
	
	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive() && 
			((GET_TEAM(getTeam()).isHasMet(GET_PLAYER((PlayerTypes)iI).getTeam()))))
		{
			iValue = 0;
			iBestValue = 0;
			for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
			{
				int iDistance = stepDistance(getX_INLINE(), getY_INLINE(), pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE());
				if (area() != pLoopCity->area())
				{
					iDistance += 1;
					iDistance /= 2;
				}
				if (iDistance <= iMaxDistance)
				{
					iTempValue = 20 + pLoopCity->getPopulation() * 2;
					iTempValue *= (1 + (iMaxDistance - iDistance));
					iTempValue /= (1 + iMaxDistance);
					
					//reduce for small islands.
					int iAreaCityCount = pLoopCity->area()->getNumCities();
					iTempValue *= min(iAreaCityCount, 5);
					iTempValue /= 5;
					if (iAreaCityCount < 3)
					{
						iTempValue /= 2;
					}
					
					if (pLoopCity->isBarbarian())
					{
						iTempValue /= 4;
					}
					
					iValue += iTempValue;					
					iBestValue = max(iBestValue, iTempValue);
				}
			}
			m_aiPlayerCloseness[iI] = (iBestValue + iValue / 4);
		}
	}
	
	m_iCachePlayerClosenessTurn = GC.getGame().getGameTurn();	
	m_iCachePlayerClosenessDistance = iMaxDistance;
}

int CvCityAI::AI_cityThreat(bool bDangerPercent)
{
	PROFILE_FUNC();
	int iValue = 0;
	int iDangerPercentValue = 0;
	bool bCrushStrategy = GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_CRUSH);
	for (int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if ((iI != getOwner()) && GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			int iTempValue = AI_playerCloseness((PlayerTypes)iI, DEFAULT_PLAYER_CLOSENESS);
			if (iTempValue > 0)
			{
				if ((bCrushStrategy) && (GET_TEAM(getTeam()).AI_getWarPlan(GET_PLAYER((PlayerTypes)iI).getTeam()) != NO_WARPLAN))
				{
					iTempValue *= 400;						
				}
				else if (atWar(getTeam(), GET_PLAYER((PlayerTypes)iI).getTeam()))
				{
					iTempValue *= 300;
				}
				else
				{
					switch (GET_PLAYER(getOwnerINLINE()).AI_getAttitude((PlayerTypes)iI))
					{
					case ATTITUDE_FURIOUS:
						iTempValue *= 180;
						break;

					case ATTITUDE_ANNOYED:
						iTempValue *= 130;
						break;

					case ATTITUDE_CAUTIOUS:
						iTempValue *= 100;
						break;

					case ATTITUDE_PLEASED:
						iTempValue *= 50;
						break;

					case ATTITUDE_FRIENDLY:
						iTempValue *= 20;
						break;

					default:
						FAssert(false);
						break;
					}
					if (bCrushStrategy)
					{
						iValue /= 2;
					}
				}
				iTempValue /= 100;
				iValue += iTempValue;
			}
		}
	}
	
	if (isCoastal(GC.getDefineINT("MIN_WATER_SIZE_FOR_OCEAN")))
	{
		int iCurrentEra = GET_PLAYER(getOwnerINLINE()).getCurrentEra();
		iValue +=  ((10 * iCurrentEra) / 3) - 6; //there are better ways to do this
	}
	
	iValue += getNumWorldWonders() * 5;
	
	if (GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_CULTURE3))
	{
		iValue += 10;
		iValue += getCommerceRateModifier(COMMERCE_CULTURE) / 20;
		if (getCultureLevel() >= (GC.getNumCultureLevelInfos() - 2))
		{
			iValue += 20;
			if (getCultureLevel() >= (GC.getNumCultureLevelInfos() - 1))
			{
				iValue += 30;
			}
		}
	}
	
	iValue += 2 * GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot(), 3, false);
	
	return iValue;
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
	
	//Start Hack
	pStream->Read(NUM_CITY_PLOTS, m_aiBestBuildValue);
	m_iCityRoleFlags = m_aiBestBuildValue[CITY_HOME_PLOT];
	m_aiBestBuildValue[CITY_HOME_PLOT] = 0;
	//End Hack

	pStream->Read(NUM_CITY_PLOTS, (int*)m_aeBestBuild);

	pStream->Read(GC.getNumEmphasizeInfos(), m_pbEmphasize);

	CvExtraSaveData& extraSaveData = GC.getMapINLINE().getExtraSaveDataClass();
	int iVersion;
	if (extraSaveData.getSaveData(EXTRASAVEDATA_CITY, m_iID, m_eOwner, CITYEXTRADATA_VERSION, iVersion))
	{
		if (iVersion == CITYEXTRADATA_CURRENTVERSION)
		{
#ifdef TESTING_EXTRA_SAVE_DATA
			int iValue;
			if (extraSaveData.getSaveData(EXTRASAVEDATA_CITY, m_iID, m_eOwner, CITYEXTRADATA_XCOORD, iValue))
			{
				FAssertMsg(iValue == m_iX, "TESTING_EXTRA_SAVE_DATA integrity check failed");
			}
			if (extraSaveData.getSaveData(EXTRASAVEDATA_CITY, m_iID, m_eOwner, CITYEXTRADATA_YCOORD, iValue))
			{
				FAssertMsg(iValue == m_iY, "TESTING_EXTRA_SAVE_DATA integrity check failed");
			}
#else
			//int iValue;
			//if (extraSaveData.getSaveData(EXTRASAVEDATA_CITY, m_iID, m_eOwner, CITYEXTRADATA_UNUSED1, iValue))
			//{
			//	m_unused1 = iValue;
			//}
			//if (extraSaveData.getSaveData(EXTRASAVEDATA_CITY, m_iID, m_eOwner, CITYEXTRADATA_UNUSED2, iValue))
			//{
			//	m_unused2 = iValue;
			//}
#endif
		}
	}
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
	
	//Start Hack
	m_aiBestBuildValue[CITY_HOME_PLOT] = m_iCityRoleFlags;
	pStream->Write(NUM_CITY_PLOTS, m_aiBestBuildValue);
	m_aiBestBuildValue[CITY_HOME_PLOT] = 0;
	//End Hack
	
	pStream->Write(NUM_CITY_PLOTS, (int*)m_aeBestBuild);

	pStream->Write(GC.getNumEmphasizeInfos(), m_pbEmphasize);
}

void CvCityAI::writeExtraSaveData(CvExtraSaveData& extraSaveData)
{
	int iVersion = CITYEXTRADATA_CURRENTVERSION;
	extraSaveData.setSaveData(EXTRASAVEDATA_CITY, m_iID, m_eOwner, CITYEXTRADATA_VERSION, iVersion);

#ifdef TESTING_EXTRA_SAVE_DATA
	extraSaveData.setSaveData(EXTRASAVEDATA_CITY, m_iID, m_eOwner, CITYEXTRADATA_XCOORD, m_iX);
	extraSaveData.setSaveData(EXTRASAVEDATA_CITY, m_iID, m_eOwner, CITYEXTRADATA_YCOORD, m_iY);
#else
	//int iUnused = 0;
	//extraSaveData.setSaveData(EXTRASAVEDATA_CITY, m_iID, m_eOwner, CITYEXTRADATA_UNUSED1, iUnused);
	
	//int iUnused = 0;
	//extraSaveData.setSaveData(EXTRASAVEDATA_CITY, m_iID, m_eOwner, CITYEXTRADATA_UNUSED2, iUnused);
#endif
}

