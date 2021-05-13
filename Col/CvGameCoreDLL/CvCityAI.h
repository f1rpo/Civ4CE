#pragma once

// cityAI.h

#ifndef CIV4_CITY_AI_H
#define CIV4_CITY_AI_H

#include "CvCity.h"

typedef std::vector<std::pair<UnitAITypes, int> > UnitTypeWeightArray;

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
	int AI_minDefenders();
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

	int AI_clearFeatureValue(int iIndex);
	DllExport BuildTypes AI_getBestBuild(int iIndex);
	DllExport int AI_countBestBuilds(CvArea* pArea);
	DllExport void AI_updateBestBuild();

	virtual int AI_cityValue() const;

    int AI_calculateWaterWorldPercent();

    int AI_getCityImportance(bool bEconomy, bool bMilitary);

    int AI_yieldMultiplier(YieldTypes eYield);
    void AI_updateSpecialYieldMultiplier();
    int AI_specialYieldMultiplier(YieldTypes eYield);

    int AI_countNumBonuses(BonusTypes eBonus, bool bIncludeOurs, bool bIncludeNeutral, int iOtherCultureThreshold, bool bLand = true, bool bWater = true);

	int AI_playerCloseness(PlayerTypes eIndex, int iMaxDistance);
	int AI_cityThreat(bool bDangerPercent = false);

	void AI_tryToWorkPlot(CvPlot* pPlot);
	int AI_getWorkersHave();
	int AI_getWorkersNeeded();
	void AI_changeWorkersHave(int iChange);
	BuildingTypes AI_bestAdvancedStartBuilding(int iPass);

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

	int* m_aiSpecialYieldMultiplier;

	int m_iCachePlayerClosenessTurn;
	int m_iCachePlayerClosenessDistance;
	int* m_aiPlayerCloseness;

	int m_iNeededFloatingDefenders;
	int m_iNeededFloatingDefendersCacheTurn;

	int m_iWorkersNeeded;
	int m_iWorkersHave;


	void AI_doDraft(bool bForce = false);
	void AI_doHurry(bool bForce = false);
	void AI_doEmphasize();
	int AI_getHappyFromHurry(HurryTypes eHurry);
	int AI_getHappyFromHurry(HurryTypes eHurry, UnitTypes eUnit, bool bIgnoreNew);
	int AI_getHappyFromHurry(HurryTypes eHurry, BuildingTypes eBuilding, bool bIgnoreNew);
	int AI_getHappyFromHurry(int iHurryPopulation);
	bool AI_doPanic();
	int AI_calculateCulturePressure(bool bGreatWork = false);


	bool AI_chooseUnit(UnitAITypes eUnitAI = NO_UNITAI);
	bool AI_chooseUnit(UnitTypes eUnit, UnitAITypes eUnitAI);

	bool AI_chooseDefender();
	bool AI_chooseLeastRepresentedUnit(UnitTypeWeightArray &allowedTypes);
	bool AI_chooseBuilding(int iFocusFlags = 0, int iMaxTurns = MAX_INT, int iMinThreshold = 0);
	bool AI_chooseProject();
	bool AI_chooseProcess(CommerceTypes eCommerceType = NO_COMMERCE);

	bool AI_bestSpreadUnit(bool bMissionary, bool bExecutive, int iBaseChance, UnitTypes* eBestSpreadUnit, int* iBestSpreadUnitValue);
	bool AI_addBestCitizen(bool bWorkers, bool bSpecialists, int* piBestPlot = NULL, SpecialistTypes* peBestSpecialist = NULL);
	bool AI_removeWorstCitizen(SpecialistTypes eIgnoreSpecialist = NO_SPECIALIST);
	void AI_juggleCitizens();

	bool AI_potentialPlot(short* piYields);
	bool AI_foodAvailable(int iExtra = 0);
	int AI_yieldValue(short* piYields, short* piCommerceYields, bool bAvoidGrowth, bool bRemove, bool bIgnoreFood = false, bool bIgnoreGrowth = false, bool bIgnoreStarvation = false, bool bWorkerOptimization = false);
	int AI_plotValue(CvPlot* pPlot, bool bAvoidGrowth, bool bRemove, bool bIgnoreFood = false, bool bIgnoreGrowth = false, bool bIgnoreStarvation = false);

	int AI_experienceWeight();
	int AI_buildUnitProb();

	void AI_bestPlotBuild(CvPlot* pPlot, int* piBestValue, BuildTypes* peBestBuild, int iFoodPriority, int iProductionPriority, int iCommercePriority, bool bChop, int iHappyAdjust, int iHealthAdjust, int iFoodChange);

	void AI_buildGovernorChooseProduction();

	int AI_getYieldMagicValue(const int* piYieldsTimes100, bool bHealthy);
	int AI_getPlotMagicValue(CvPlot* pPlot, bool bHealthy, bool bWorkerOptimization = false);
	int AI_countGoodTiles(bool bHealthy, bool bUnworkedOnly, int iThreshold = 50, bool bWorkerOptimization = false);
	int AI_countGoodSpecialists(bool bHealthy);
	int AI_calculateTargetCulturePerTurn();

	void AI_stealPlots();

	int AI_buildingSpecialYieldChangeValue(BuildingTypes kBuilding, YieldTypes eYield);

	void AI_cachePlayerCloseness(int iMaxDistance);
	void AI_updateWorkersNeededHere();

	// added so under cheat mode we can call protected functions for testing
	friend class CvGameTextMgr;
};

#endif
