#pragma once

#ifndef CyCity_h
#define CyCity_h
//
// Python wrapper class for CvCity 
// 

//#include "CvEnums.h"
#include <string>
# include <boost/python/tuple.hpp>
namespace python = boost::python;

struct OrderData;
class CvCity;
class CyPlot;
class CyArea;
class CyUnit;
class CyCity
{
public:
	CyCity();
	DllExport CyCity(CvCity* pCity);		// Call from C++
	CvCity* getCity() { return m_pCity;	}	// Call from C++
	bool isNone() { return (m_pCity==NULL); }
	void kill();

	void createGreatGeneral(int /*UnitTypes*/ eGreatGeneralUnit, bool bIncrementExperience);
	
	void doTask(int /*TaskTypes*/ eTask, int iData1, int iData2, bool bOption);
	void chooseProduction(int /*UnitTypes*/ eTrainUnit, int /*BuildingTypes*/ eConstructBuilding, bool bFinish, bool bFront);
	int getCityPlotIndex(CyPlot* pPlot);
	CyPlot* getCityIndexPlot(int iIndex);
	bool canWork(CyPlot* pPlot);
	void clearWorkingOverride(int iIndex);
	int countNumImprovedPlots();
	int countNumWaterPlots();
	int countNumRiverPlots();

	int findPopulationRank();
	int findBaseYieldRateRank(int /*YieldTypes*/ eYield);
	int findYieldRateRank(int /*YieldTypes*/ eYield);

	int /*UnitTypes*/ allUpgradesAvailable(int /*UnitTypes*/ eUnit, int iUpgradeCount);

	bool canTrain( int iUnit, bool bContinue, bool bTestVisible);
	bool canConstruct( int iBuilding, bool bContinue, bool bTestVisible, bool bIgnoreCost);
	bool canConvince( int iFatherPointType, bool bContinue, bool bTestVisible);
	int getFoodTurnsLeft();
	bool isProduction();
	bool isProductionUnit();
	bool isProductionBuilding();

	bool canContinueProduction(OrderData order);
	int getProductionExperience(int /*UnitTypes*/ eUnit);
	void addProductionExperience(CyUnit* pUnit);

	int /*UnitTypes*/ getProductionUnit();
	int /*UnitAITypes*/ getProductionUnitAI();
	int /*BuildingTypes*/ getProductionBuilding();
	std::wstring getProductionName();
	std::wstring getProductionNameKey();
	int getGeneralProductionTurnsLeft();
	int getFirstUnitOrder(int /*UnitTypes*/ eUnit);
	int getNumTrainUnitAI(int /*UnitAITypes*/ eUnitAI);
	int getFirstBuildingOrder(int /*BuildingTypes*/ eBuilding);
	int getProduction();
	int getProductionNeeded(int /*YieldTypes*/ eYield);
	int getProductionTurnsLeft();
	int getUnitProductionTurnsLeft(int /*UnitTypes*/ iUnit, int iNum);
	int getBuildingProductionTurnsLeft(int /*BuildingTypes*/ iBuilding, int iNum);								
	void setProduction(int iNewValue);
	void changeProduction(int iChange);
	int getProductionModifier();
	int getCurrentProductionDifference(bool bOverflow);
	int getUnitProductionModifier(int /*UnitTypes*/ iUnit);
	int getBuildingProductionModifier(int /*BuildingTypes*/ iBuilding);

	bool canHurry(int /*HurryTypes*/ iHurry, bool bTestVisible);
	void hurry(int /*HurryTypes*/ iHurry);
	int /* HandicapTypes */ getHandicapType();
	int /* CivilizationTypes */ getCivilizationType();
	int /*LeaderHeadTypes*/ getPersonalityType();
	int /*ArtStyleTypes*/ getArtStyleType();
	int /*CitySizeTypes*/ getCitySizeType();

	bool hasTrait(int /*TraitTypes*/ iTrait);
	bool isHuman();
	bool isVisible(int /*TeamTypes*/ eTeam, bool bDebug);

	bool isCapital();
	bool isCoastal(int iMinWaterSize);
	bool isDisorder();
	int extraPopulation();
	int foodConsumption(int iExtra);												 
	int foodDifference();
	int growthThreshold();
	int productionLeft();
	int hurryCost(bool bExtra, bool bIgnoreNew);																						 
	int hurryGold(int /*HurryTypes*/ iHurry);
	int hurryPopulation(int /*HurryTypes*/ iHurry);
	int hurryProduction(int /*HurryTypes*/ iHurry);
	int maxHurryPopulation();

	int cultureDistance(int iDX, int iDY);
	bool isHasBuilding(int /*BuildingTypes*/ iIndex);
	int getProfessionOutput(int /*ProfessionTypes*/ iIndex, CyUnit* pUnit);
	int getID();
	int getX();
	int getY();
	bool at(int iX, int iY);
	bool atPlot(CyPlot* pPlot);
	CyPlot* plot();
	CyArea* area();
	CyArea* waterArea();
	CyPlot* getRallyPlot();

	int getGameTurnFounded();
	int getGameTurnAcquired();
	int getPopulation();
	void setPopulation(int iNewValue);
	void changePopulation(int iChange);
	long getRealPopulation();

	int getHighestPopulation();
	void setHighestPopulation(int iNewValue);
	bool isWorksWater();
	void changeHealRate(int iChange);

	int getFood();
	void setFood(int iNewValue);
	void changeFood(int iChange);
	int getFoodKept();
	int getMaxFoodKeptPercent();
	int getOverflowProduction();
	void setOverflowProduction(int iNewValue);
	int getMilitaryProductionModifier();
	int getBuildingDefense();
	int getBuildingBombardDefense();
	int getFreeExperience();
	int getDefenseDamage();
	void changeDefenseDamage(int iChange);
	bool isBombardable(CyUnit* pUnit);
	int getTotalDefense();
	int getDefenseModifier();

	int getOccupationTimer();
	bool isOccupation();
	void setOccupationTimer(int iNewValue);
	void changeOccupationTimer(int iChange);
	int getCultureUpdateTimer();
	void changeCultureUpdateTimer(int iChange);
	bool isNeverLost();
	void setNeverLost(int iNewValue);
	bool isBombarded();
	void setBombarded(int iNewValue);
	void setAllCitizensAutomated(bool bAutomated);
	bool isProductionAutomated();
	void setProductionAutomated(bool bNewValue);
	bool isWallOverride() const;
	void setWallOverride(bool bOverride);
	void setCitySizeBoost(int iBoost);
	int /*PlayerTypes*/getOwner();
	int /*TeamTypes*/getTeam();
	int /*PlayerTypes*/getPreviousOwner();
	int /*PlayerTypes*/getOriginalOwner();
	int /*CultureLevelTypes*/ getCultureLevel();
	int getCultureThreshold();
	int getSeaPlotYield(int /*YieldTypes*/ eIndex);
	int getRiverPlotYield(int /*YieldTypes*/ eIndex);

	int getBaseRawYieldProduced(int /*YieldTypes*/ eIndex);
	int getRawYieldProduced(int /*YieldTypes*/ eIndex);
	int getRawYieldConsumed(int /*YieldTypes*/ eIndex);

	int getBaseYieldRateModifier(int /*YieldTypes*/ eIndex, int iExtra);
	int getYieldRate(int /*YieldTypes*/ eIndex);
	int getCultureRate();
	int getYieldRateModifier(int /*YieldTypes*/ eIndex);

	int getDomainFreeExperience(int /*DomainTypes*/ eIndex);
	int getDomainProductionModifier(int /*DomainTypes*/ eIndex);
	int getCulture(int /*PlayerTypes*/ eIndex);
	int countTotalCulture();
	PlayerTypes findHighestCulture();
	int calculateCulturePercent(int eIndex);
	int calculateTeamCulturePercent(int /*TeamTypes*/ eIndex);
	void setCulture(int /*PlayerTypes*/ eIndex, int iNewValue, bool bPlots);
	void changeCulture(int /*PlayerTypes*/ eIndex, int iChange, bool bPlots);

	int getYieldStored(int /*YieldTypes*/ eYield) const;
	void setYieldStored(int /*YieldTypes*/ eYield, int iValue);
	void changeYieldStored(int /*YieldTypes*/ eYield, int iChange);
	int getYieldRushed(int /*YieldTypes*/ eYield) const;
	int calculateNetYield(int /*YieldTypes*/ eYield);
	int calculateActualYieldProduced(int /*YieldTypes*/ eYield) const;
	int calculateActualYieldConsumed(int /*YieldTypes*/ eYield) const;

	bool isEverOwned(int /*PlayerTypes*/ eIndex);

	bool isRevealed(int /*TeamTypes*/ eIndex, bool bDebug);	
	void setRevealed(int /*TeamTypes*/ eIndex, bool bNewValue);	
	std::wstring getName();
	std::wstring getNameForm(int iForm);
	std::wstring getNameKey();
	void setName(std::wstring szNewValue, bool bFound);
	int getBuildingProduction(int /*BuildingTypes*/ iIndex);
	void setBuildingProduction(int /*BuildingTypes*/ iIndex, int iNewValue);
	void changeBuildingProduction(int /*BuildingTypes*/ iIndex, int iChange);
	int getBuildingProductionTime(int /*BuildingTypes*/ eIndex);
	void setBuildingProductionTime(int /*BuildingTypes*/ eIndex, int iNewValue);
	void changeBuildingProductionTime(int /*BuildingTypes*/ eIndex, int iChange);
	int getBuildingOriginalOwner(int /*BuildingTypes*/ iIndex);
	int getBuildingOriginalTime(int /*BuildingTypes*/ iIndex);
	int getUnitProduction(int iIndex);
	void setUnitProduction(int iIndex, int iNewValue);
	void changeUnitProduction(int /*UnitTypes*/ iIndex, int iChange);

	int getUnitCombatFreeExperience(int /*UnitCombatTypes*/ eIndex);	
	int getFreePromotionCount(int /*PromotionTypes*/ eIndex);	
	bool isFreePromotion(int /*PromotionTypes*/ eIndex);

	bool isHasRealBuilding(int /*BuildingTypes*/ iIndex);
	void setHasRealBuilding(int /*BuildingTypes*/ iIndex, bool bNewValue);
	bool isHasFreeBuilding(int /*BuildingTypes*/ iIndex);

	void clearOrderQueue();
	void pushOrder(OrderTypes eOrder, int iData1, int iData2, bool bSave, bool bPop, bool bAppend, bool bForce);
	void popOrder(int iNum, bool bFinish, bool bChoose);
	int getOrderQueueLength();
	OrderData* getOrderFromQueue(int iIndex);

	void setWallOverridePoints(const python::tuple& kPoints); /* points are given in world space ... i.e. PlotXToPointX, etc */
	python::tuple getWallOverridePoints() const;

	int getBuildingYieldChange(int /*BuildingClassTypes*/ eBuildingClass, int /*YieldTypes*/ eYield) const;
	void setBuildingYieldChange(int /*BuildingClassTypes*/ eBuildingClass, int /*YieldTypes*/ eYield, int iChange);

	int getLiberationPlayer(bool bConquest);
	void liberate(bool bConquest);

	bool isScoutVisited(int /*TeamTypes*/ eTeam) const;
	int getMaxYieldCapacity();

	int AI_getEmphasizeYieldCount(int /*YieldTypes*/ eYield) const;
	bool AI_avoidGrowth();
	bool AI_isEmphasize(int iEmphasizeType);
	int AI_countBestBuilds(CyArea* pArea);
	int AI_cityValue();

	std::string getScriptData() const;
	void setScriptData(std::string szNewValue);

	CyUnit* getPopulationUnitByIndex(int iUnitIndex);
	CyUnit* getPopulationUnitById(int iUnitID);
	int getPopulationUnitIndex(CyUnit*);
	CyUnit* getUnitWorkingPlot(int iPlotIndex);
	void addPopulationUnit(CyUnit* pUnit, int /*ProfessionTypes*/ eProfession);
	bool removePopulationUnit(CyUnit* pUnit, bool bDelete, int /*ProfessionTypes*/ eProfession);

	int getTeachUnitClass();
	int getRebelPercent();
	int getRebelSentiment() const;
	void setRebelSentiment(int iValue);

	bool isExport(int /*YieldTypes*/ eYield) const;
	void addExport(int /*YieldTypes*/ eYield);
	void removeExport(int /*YieldTypes*/ eYield);
	bool isImport(int /*YieldTypes*/ eYield) const;
	void addImport(int /*YieldTypes*/ eYield);
	void removeImport(int /*YieldTypes*/ eYield);
	int getMaintainLevel(int /*YieldTypes*/ eYield) const;

	python::tuple isOrderWaitingForYield(int /*YieldTypes*/ eYield);

private:
	CvCity* m_pCity;
};

#endif	// CyCity_h
