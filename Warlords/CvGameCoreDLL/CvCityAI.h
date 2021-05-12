// cityAI.h

#ifndef CIV4_CITY_AI_H
#define CIV4_CITY_AI_H

#include "CvCity.h"

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
	DllExport int AI_buildingValue(BuildingTypes eBuilding, int iFocusFlags = 0);

	DllExport ProjectTypes AI_bestProject();
	DllExport int AI_projectValue(ProjectTypes eProject);

	DllExport ProcessTypes AI_bestProcess();
	DllExport int AI_processValue(ProcessTypes eProcess);

	DllExport int AI_neededSeaWorkers();

	DllExport bool AI_isDefended(int iExtra = 0);
	DllExport bool AI_isAirDefended(int iExtra = 0);
	DllExport bool AI_isDanger();

	DllExport int AI_neededDefenders();
	DllExport int AI_neededAirDefenders();

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

	DllExport int AI_getBestBuildValue(int iIndex);
	DllExport int AI_totalBestBuildValue(CvArea* pArea);

	DllExport BuildTypes AI_getBestBuild(int iIndex);
	DllExport int AI_countBestBuilds(CvArea* pArea);
	DllExport void AI_updateBestBuild();

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
	int m_aiBestBuildValue[NUM_CITY_PLOTS];

	BuildTypes m_aeBestBuild[NUM_CITY_PLOTS];

	bool* m_pbEmphasize;

	void AI_doDraft();
	void AI_doHurry();
	void AI_doEmphasize();

	bool AI_chooseUnit(UnitAITypes eUnitAI = NO_UNITAI);
	bool AI_chooseDefender();
	bool AI_chooseBuilding(int iFocusFlags = 0, int iMaxTurns = MAX_INT);
	bool AI_chooseProject();
	bool AI_chooseProcess();

	bool AI_addBestCitizen(bool bWorkers, bool bSpecialists);
	bool AI_removeWorstCitizen(SpecialistTypes eIgnoreSpecialist = NO_SPECIALIST);
	void AI_juggleCitizens();

	bool AI_potentialPlot(short* piYields);
	bool AI_foodAvailable(int iExtra = 0);
	int AI_yieldValue(short* piYields, bool bAvoidGrowth, bool bRemove, bool bIgnoreFood = false, bool bIgnoreGrowth = false);
	int AI_plotValue(CvPlot* pPlot, bool bAvoidGrowth, bool bRemove, bool bIgnoreFood = false, bool bIgnoreGrowth = false);

	int AI_experienceWeight();
	int AI_buildUnitProb();

	void AI_bestPlotBuild(CvPlot* pPlot, int* piBestValue, BuildTypes* peBestBuild);

};

#endif
