// cityAI.h

#ifndef CIV4_CITY_AI_H
#define CIV4_CITY_AI_H

#include "CvCity.h"

#ifndef FINAL_RELEASE
// only one or the other of the following should be defined, not both
//#define COMPARE_NEW_CITY_GOVERNOR
//#define USE_NEW_CITY_GOVERNOR
#endif

#ifdef COMPARE_NEW_CITY_GOVERNOR
#define NEW_CITY_GOVERNOR
#include <set>
#endif

#ifdef USE_NEW_CITY_GOVERNOR
#define NEW_CITY_GOVERNOR
#endif

#ifdef NEW_CITY_GOVERNOR
#include <map>

const int iGovernor_FoodWeight = 80;
const int iGovernor_ProductionWeight = 65;
const int iGovernor_CommerceWeight = 40;
const int iGovernor_EmphasizeMultiplier = 3;

struct CvCompleteYield
{
	int aiYield[NUM_YIELD_TYPES];
	int aiCommerce[NUM_COMMERCE_TYPES];
	int iGPPRate;
	int iBonusExperience;
};

struct CvGovernorValues
{
	CvCompleteYield yieldModifiers;
	CvCompleteYield yieldValues;
	int iExtraFoodValue;
	
	bool bInitialized;

	bool bGrowPastHappy;
	int iHappyLevel;
};

struct CvGovernorCitizen
{
	short iFood;
	
	short iIndex;
	bool bIsSpecialist;
	
	short iInitialCount;
	short iForcedCount;
	short iCount;
	short iMax;

	short iFreeCount;
};

struct CvGovernorWork
{
	int iWorkablePopulation;
	int iWorkablePlots;
	int iTotalFreeSpecialists;
	int iWorkingPlots;
	int iWorkingSpecialists;

	int iPopulationAssignable;
	int iFreeSpecialistsAssignable;

	int iTotalForcedSpecialists;

	inline int availableToWork() { return (iWorkablePopulation - iWorkingPlots + min(0, iTotalFreeSpecialists - iWorkingSpecialists)); };
	inline int unassignedWorkers() { return (iWorkablePopulation - iWorkingPlots + iTotalFreeSpecialists - iWorkingSpecialists); };

	inline int maxSpecialists() { return (iWorkablePopulation + iTotalFreeSpecialists); };
	inline int availableSpecialists() { return (maxSpecialists() - iWorkingPlots - iWorkingSpecialists); };
	
	inline bool isForcedSpecialists() { return (iTotalForcedSpecialists > 0); };
};

typedef std::multimap<int,CvGovernorCitizen> CvSortedCitizens;
typedef CvSortedCitizens::iterator CvCitizensIterator;
typedef CvSortedCitizens::reverse_iterator CvCitizensReverseIterator;

template <class _CitizenIter>
inline const int getCitizenValue(_CitizenIter itCitizen)
{
	return itCitizen->first;
}

template <class _CitizenIter>
inline CvGovernorCitizen* getCitizen(_CitizenIter itCitizen)
{
	return &itCitizen->second;
}

#endif

class CvExtraSaveData;

class CvCityAI : public CvCity
{

public:

	DllExport CvCityAI();
	DllExport virtual ~CvCityAI();

	DllExport void AI_init();
	DllExport void AI_uninit();
	DllExport void AI_reset();

	DllExport void AI_doTurn();

	DllExport void AI_assignWorkingPlots();
	DllExport void AI_updateAssignWork();

	DllExport bool AI_avoidGrowth();
	DllExport bool AI_ignoreGrowth();
	DllExport int AI_specialistValue(SpecialistTypes eSpecialist, bool bAvoidGrowth, bool bRemove);

	DllExport void AI_chooseProduction();

	DllExport UnitTypes AI_bestUnit(bool bAsync = false, AdvisorTypes eIgnoreAdvisor = NO_ADVISOR, UnitAITypes* peBestUnitAI = NULL);
	DllExport UnitTypes AI_bestUnitAI(UnitAITypes eUnitAI, bool bAsync = false, AdvisorTypes eIgnoreAdvisor = NO_ADVISOR);

	DllExport BuildingTypes AI_bestBuilding(int iFocusFlags = 0, int iMaxTurns = 0, bool bAsync = false, AdvisorTypes eIgnoreAdvisor = NO_ADVISOR);
	BuildingTypes AI_bestBuildingThreshold(int iFocusFlags = 0, int iMaxTurns = 0, int iMinThreshold = 0, bool bAsync = false, AdvisorTypes eIgnoreAdvisor = NO_ADVISOR);
	
	DllExport int AI_buildingValue(BuildingTypes eBuilding, int iFocusFlags = 0);
	int AI_buildingValueThreshold(BuildingTypes eBuilding, int iFocusFlags = 0, int iThreshold = 0);

	DllExport ProjectTypes AI_bestProject();
	DllExport int AI_projectValue(ProjectTypes eProject);

	DllExport ProcessTypes AI_bestProcess();
	ProcessTypes AI_bestProcess(CommerceTypes eCommerceType);
	DllExport int AI_processValue(ProcessTypes eProcess);
	int AI_processValue(ProcessTypes eProcess, CommerceTypes eCommerceType);

	DllExport int AI_neededSeaWorkers();

	DllExport bool AI_isDefended(int iExtra = 0);
	DllExport bool AI_isAirDefended(int iExtra = 0);
	DllExport bool AI_isDanger();

	DllExport int AI_neededDefenders();
	DllExport int AI_neededAirDefenders();
	int AI_neededFloatingDefenders();
	void AI_updateNeededFloatingDefenders();

	DllExport int AI_getEmphasizeAvoidGrowthCount();
	DllExport bool AI_isEmphasizeAvoidGrowth();

	DllExport int AI_getEmphasizeGreatPeopleCount();
	DllExport bool AI_isEmphasizeGreatPeople();

	DllExport bool AI_isAssignWorkDirty();
	DllExport void AI_setAssignWorkDirty(bool bNewValue);

	DllExport bool AI_isChooseProductionDirty();
	DllExport void AI_setChooseProductionDirty(bool bNewValue);

	DllExport CvCity* AI_getRouteToCity() const;
	DllExport void AI_updateRouteToCity();

	DllExport int AI_getEmphasizeYieldCount(YieldTypes eIndex);
	DllExport bool AI_isEmphasizeYield(YieldTypes eIndex);

	DllExport int AI_getEmphasizeCommerceCount(CommerceTypes eIndex);
	DllExport bool AI_isEmphasizeCommerce(CommerceTypes eIndex);

	DllExport bool AI_isEmphasize(EmphasizeTypes eIndex);
	DllExport void AI_setEmphasize(EmphasizeTypes eIndex, bool bNewValue);
	void AI_forceEmphasizeCulture(bool bNewValue);

	DllExport int AI_getBestBuildValue(int iIndex);
	DllExport int AI_totalBestBuildValue(CvArea* pArea);

	DllExport BuildTypes AI_getBestBuild(int iIndex);
	DllExport int AI_countBestBuilds(CvArea* pArea);
	DllExport void AI_updateBestBuild();

    int AI_calculateCulturePressure(bool bGreatWork = false);
    
    int AI_calculateWaterWorldPercent();
    
    bool AI_isCityRole(int iFlag) const;

    void AI_setCityRole(int iOnFlags, int iOffFlags = 0);
    void AI_resetCityRoles();
    int AI_getCityImportance(bool bEconomy, bool bMilitary);
    
    int AI_yieldMultiplier(YieldTypes eYield);
    void AI_updateSpecialYieldMultiplier();
    int AI_specialYieldMultiplier(YieldTypes eYield);
    
    int AI_countNumBonuses(BonusTypes eBonus, bool bIncludeOurs, bool bIncludeNeutral, int iOtherCultureThreshold, bool bLand = true, bool bWater = true);

	int AI_playerCloseness(PlayerTypes eIndex, int iMaxDistance);
	int AI_cityThreat(bool bDangerPercent = false);
	
	DllExport void read(FDataStreamBase* pStream);
	DllExport void write(FDataStreamBase* pStream);

protected:

	int m_iEmphasizeAvoidGrowthCount;
	int m_iEmphasizeGreatPeopleCount;

	bool m_bAssignWorkDirty;
	bool m_bChooseProductionDirty;

	IDInfo m_routeToCity;

	int* m_aiEmphasizeYieldCount;
	int* m_aiEmphasizeCommerceCount;
	bool m_bForceEmphasizeCulture;

	int m_aiBestBuildValue[NUM_CITY_PLOTS];

	BuildTypes m_aeBestBuild[NUM_CITY_PLOTS];

	bool* m_pbEmphasize;
	
	int m_iCityRoleFlags;
	int* m_aiSpecialYieldMultiplier;
	
	int m_iCachePlayerClosenessTurn;
	int m_iCachePlayerClosenessDistance;
	int* m_aiPlayerCloseness;
	
	int m_iNeededFloatingDefenders;
	int m_iNeededFloatingDefendersCacheTurn;
	

	void AI_doDraft(bool bForce = false);
	void AI_doHurry(bool bForce = false);
	void AI_doEmphasize();
	int AI_getHappyFromHurry(HurryTypes eHurry);
	int AI_getHappyFromHurry(HurryTypes eHurry, UnitTypes eUnit, bool bIgnoreNew);
	int AI_getHappyFromHurry(HurryTypes eHurry, BuildingTypes eBuilding, bool bIgnoreNew);
	int AI_getHappyFromHurry(int iHurryPopulation);
	bool AI_doPanic();

	bool AI_chooseUnit(UnitAITypes eUnitAI = NO_UNITAI);
	bool AI_chooseDefender();
	bool AI_chooseFloatingDefender(int iTotalNeededDefenders);
	bool AI_chooseBuilding(int iFocusFlags = 0, int iMaxTurns = MAX_INT, int iMinThreshold = 0);
	bool AI_chooseProject();
	bool AI_chooseProcess(CommerceTypes eCommerceType = NO_COMMERCE);

	bool AI_addBestCitizen(bool bWorkers, bool bSpecialists);
	bool AI_removeWorstCitizen(SpecialistTypes eIgnoreSpecialist = NO_SPECIALIST);
	void AI_juggleCitizens();

	bool AI_potentialPlot(short* piYields);
	bool AI_foodAvailable(int iExtra = 0);
	int AI_yieldValue(short* piYields, bool bAvoidGrowth, bool bRemove, bool bIgnoreFood = false, bool bIgnoreGrowth = false, bool bIgnoreStarvation = false);
	int AI_unifiedYieldValue(short* piYields, short* piCommerce, bool bAvoidGrowth, bool bRemove, bool bIgnoreFood, bool bIgnoreGrowth);
	int AI_plotValue(CvPlot* pPlot, bool bAvoidGrowth, bool bRemove, bool bIgnoreFood = false, bool bIgnoreGrowth = false, bool bIgnoreStarvation = false);

#ifdef NEW_CITY_GOVERNOR
	CvGovernorValues m_governorValues;
	CvSortedCitizens m_governorCitizens;

	void test_AI_AssignWorkingPlots();
	
	bool test_AI_AddBestCitizen(bool bWorkers, bool bSpecialists);
	bool test_AI_removeWorstCitizen(SpecialistTypes eIgnoreSpecialist = NO_SPECIALIST);

	void AI_initializeGovernorWork(CvGovernorWork &governorWork, CvSortedCitizens& governorCitizens, bool bDoForced = false);
	int AI_workMoreCitizens(CvGovernorWork &governorWork, CvSortedCitizens& governorCitizens, int iCountToAdd, bool bIgnoreForced = false);
	int AI_workLessCitizens(CvGovernorWork &governorWork, CvSortedCitizens& governorCitizens, int iCountToRemove);

	int AI_foodAdjustCitizens(CvGovernorWork &governorWork, CvSortedCitizens& governorCitizens, CvGovernorValues& governorValues);
	int swapHigherFoodCitizen(CvCitizensReverseIterator ir, int& iFoodDifference, bool bEncourageGrowth, int& iLowestFood, CvCitizensReverseIterator& irLowestFood, CvGovernorWork &governorWork, CvSortedCitizens& governorCitizens);
	bool AI_foodAdjustChangedCitizen(CvGovernorWork &governorWork, CvSortedCitizens& governorCitizens, CvGovernorValues& governorValues);

	int AI_countCitizenFood(CvSortedCitizens& governorCitizens);
	int AI_countMaximumCitizenFood(CvGovernorWork &governorWork, CvSortedCitizens& governorCitizens);

	void AI_calculateGovernorValues();
	void AI_calculateGovernorValues(CvGovernorValues& governorValues);
	void AI_buildGovernorCitizens(bool bReassign = false, bool bCapWorkers = false, bool bCapSpecialists = false);
	void AI_buildGovernorCitizens(CvSortedCitizens& governorCitizens, CvGovernorValues& governorValues, bool bReassign = false, bool bCapWorkers = false, bool bCapSpecialists = false);
	void AI_workGovernorCitizens(CvSortedCitizens& governorCitizens);

	int AI_plotValue(CvPlot* pPlot);
	int AI_plotValue(CvPlot* pPlot, CvGovernorValues& governorValues);
	int AI_specialistValue(SpecialistTypes eSpecialist);
	int AI_specialistValue(SpecialistTypes eSpecialist, CvGovernorValues& governorValues);
	int AI_completeYieldValue(CvCompleteYield& completeYield);
	int AI_completeYieldValue(CvCompleteYield& completeYield, CvGovernorValues& governorValues);

#ifdef COMPARE_NEW_CITY_GOVERNOR
	// the following are only for testing
	bool m_bDisableTest;
	std::set<int> m_lastDifferentPlots;
	std::set<int> m_lastDifferentSpecialists;
	void AI_compareGovernorCitizens();
#endif
#endif

	int AI_experienceWeight();
	int AI_buildUnitProb();

	void AI_bestPlotBuild(CvPlot* pPlot, int* piBestValue, BuildTypes* peBestBuild, int iFoodPriority, int iProductionPriority, int iCommercePriority, bool bChop);
	
	void AI_buildGovernorChooseProduction();
	
	int AI_getPlotMagicValue(CvPlot* pPlot, bool bHealthy);
	int AI_countGoodTiles(bool bHealthy, bool bUnworkedOnly, int iThreshold = 50);
	
	void AI_stealPlots();
	
	int AI_buildingSpecialYieldChangeValue(BuildingTypes kBuilding, YieldTypes eYield);
	
	void AI_cachePlayerCloseness(int iMaxDistance);

	void writeExtraSaveData(CvExtraSaveData& extraSaveData);
	friend CvExtraSaveData;

	// added so under cheat mode we can call protected functions for testing
	friend class CvGameTextMgr;
};

#endif
