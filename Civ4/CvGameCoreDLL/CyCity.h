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

	void createGreatPeople(int /*UnitTypes*/ eGreatPersonUnit, bool bIncrementThreshold);
	
	void doTask(int /*TaskTypes*/ eTask, int iData1, int iData2, bool bOption);
	void chooseProduction(int /*UnitTypes*/ eTrainUnit, int /*BuildingTypes*/ eConstructBuilding, int /*ProjectTypes*/ eCreateProject, bool bFinish, bool bFront);
	int getCityPlotIndex(CyPlot* pPlot);
	CyPlot* getCityIndexPlot(int iIndex);
	bool canWork(CyPlot* pPlot);
	void clearWorkingOverride(int iIndex);
	int countNumImprovedPlots();
	int countNumWaterPlots();

	int findPopulationRank();
	int findBaseYieldRateRank(int /*YieldTypes*/ eYield);
	int findYieldRateRank(int /*YieldTypes*/ eYield);
	int findCommerceRateRank(int /*CommerceTypes*/ eCommerce);

	int /*UnitTypes*/ allUpgradesAvailable(int /*UnitTypes*/ eUnit, int iUpgradeCount);
	bool isWorldWondersMaxed();
	bool isTeamWondersMaxed();
	bool isNationalWondersMaxed();
	bool isBuildingsMaxed();

	bool canTrain( int iUnit, bool bContinue, bool bTestVisible);
	bool canConstruct( int iBuilding, bool bContinue, bool bTestVisible, bool bIgnoreCost);
	bool canCreate( int iProject, bool bContinue, bool bTestVisible );
	bool canMaintain( int iProcess, bool bContinue );
	bool canJoin();
	int getFoodTurnsLeft();
	bool isProduction();
	bool isProductionLimited();
	bool isProductionUnit();
	bool isProductionBuilding();
	bool isProductionProject();																	 
	bool isProductionProcess();

	bool canContinueProduction(OrderData order);
	int getProductionExperience(int /*UnitTypes*/ eUnit);
	void addProductionExperience(CyUnit* pUnit, bool bConscript);

	int /*UnitTypes*/ getProductionUnit();
	int /*UnitAITypes*/ getProductionUnitAI();
	int /*BuildingTypes*/ getProductionBuilding();
	int /*ProjectTypes*/ getProductionProject();
	int /*ProcessTypes*/ getProductionProcess();
	std::wstring getProductionName();
	std::wstring getProductionNameKey();
	int getGeneralProductionTurnsLeft();
	bool isFoodProduction();
	int getFirstUnitOrder(int /*UnitTypes*/ eUnit);
	int getFirstProjectOrder(int /*ProjectTypes*/ eProject);
	int getNumTrainUnitAI(int /*UnitAITypes*/ eUnitAI);
	int getFirstBuildingOrder(int /*BuildingTypes*/ eBuilding);
	bool isUnitFoodProduction(int /*UnitTypes*/ iUnit);
	int getProduction();
	int getProductionNeeded();
	int getProductionTurnsLeft();
	int getUnitProductionTurnsLeft(int /*UnitTypes*/ iUnit, int iNum);
	int getBuildingProductionTurnsLeft(int /*BuildingTypes*/ iBuilding, int iNum);								
	int getProjectProductionTurnsLeft(int /*ProjectTypes*/ eProject, int iNum);								 
	void setProduction(int iNewValue);
	void changeProduction(int iChange);
	int getProductionModifier();
	int getCurrentProductionDifference(bool bIgnoreFood, bool bOverflow);
	int getUnitProductionModifier(int /*UnitTypes*/ iUnit);
	int getBuildingProductionModifier(int /*BuildingTypes*/ iBuilding);
	int getProjectProductionModifier(int /*ProjectTypes*/ eProject);											 

	int getExtraProductionDifference(int iExtra);

	bool canHurry(int /*HurryTypes*/ iHurry, bool bTestVisible);
	void hurry(int /*HurryTypes*/ iHurry);
	int /*UnitTypes*/ getConscriptUnit();
	int getConscriptPopulation();
	int conscriptMinCityPopulation();
	int flatConscriptAngerLength();
	bool canConscript();
	void conscript();
	int getBonusHealth(int /*BonusTypes*/ iBonus);
	int getBonusHappiness(int /*BonusTypes*/ iBonus);
	int getBonusPower(int /*BonusTypes*/ eBonus, bool bDirty);										 
	int getBonusYieldRateModifier(int /*YieldTypes*/ eIndex, int /*BonusTypes*/ eBonus);	 
	int /* HandicapTypes */ getHandicapType();
	int /* CivilizationTypes */ getCivilizationType();
	int /*LeaderHeadTypes*/ getPersonalityType();
	int /*ArtStyleTypes*/ getArtStyleType();
	int /*CitySizeTypes*/ getCitySizeType();

	bool hasTrait(int /*TraitTypes*/ iTrait);
	bool isBarbarian();
	bool isHuman();
	bool isVisible(int /*TeamTypes*/ eTeam, bool bDebug);

	bool isCapital();
	bool isCoastal();
	bool isDisorder();
	bool isHolyCityByType(int /*ReligionTypes*/ iIndex);
	bool isHolyCity();
	int getOvercrowdingPercentAnger(int iExtra);
	int getNoMilitaryPercentAnger();
	int getCulturePercentAnger();
	int getReligionPercentAnger();
	int getWarWearinessPercentAnger();
	int getLargestCityHappiness();
	int unhappyLevel(int iExtra);
	int happyLevel();
	int angryPopulation(int iExtra);
	int totalFreeSpecialists();
	int extraPopulation();
	int extraSpecialists();
	int extraFreeSpecialists();
	int unhealthyPopulation(bool bNoAngry, int iExtra);										 
	int totalGoodBuildingHealth();																		 
	int totalBadBuildingHealth();																			 
	int goodHealth();
	int badHealth(bool bNoAngry);
	int healthRate(bool bNoAngry, int iExtra);															 
	int foodConsumption(bool bNoAngry, int iExtra);												 
	int foodDifference(bool bBottom);
	int growthThreshold();
	int productionLeft();
	int hurryCost(bool bExtra);																						 
	int hurryGold(int /*HurryTypes*/ iHurry);
	int hurryPopulation(int /*HurryTypes*/ iHurry);
	int hurryProduction(int /*HurryTypes*/ iHurry);
	int flatHurryAngerLength();
	int hurryAngerLength(int /*HurryTypes*/ iHurry);
	int maxHurryPopulation();

	int cultureDistance(int iDX, int iDY);
	int cultureStrength(int /*PlayerTypes*/ ePlayer);
	int cultureGarrison(int /*PlayerTypes*/ ePlayer);	
	bool hasBuilding(int /*BuildingTypes*/ iIndex);
	bool hasActiveBuilding(int /*BuildingTypes*/ iIndex);
	int getID();
	int getX();
	int getY();
	bool at(int iX, int iY);
	bool atPlot(CyPlot* pPlot);
	CyPlot* plot();
	bool isConnectedTo(CyCity* pCity);
	bool isConnectedToCapital(int /*PlayerTypes*/ ePlayer);
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
	int getWorkingPopulation();
	int getSpecialistPopulation();
	int getNumGreatPeople();
	int getBaseGreatPeopleRate();
	int getGreatPeopleRate();
	int getTotalGreatPeopleRateModifier();
	void changeBaseGreatPeopleRate(int iChange);
	int getGreatPeopleRateModifier();
	int getGreatPeopleProgress();
	void changeGreatPeopleProgress(int iChange);
	int getNumWorldWonders();
	int getNumTeamWonders();
	int getNumNationalWonders();
	int getNumBuildings();
	bool isGovernmentCenter();
	int getMaintenance();
	int calculateDistanceMaintenance();														 
	int calculateNumCitiesMaintenance();													 
	int getMaintenanceModifier();
	int getWarWearinessModifier();
	void changeHealRate(int iChange);
	int getFreshWaterGoodHealth();
	int getFreshWaterBadHealth();
	int getBuildingGoodHealth();
	int getBuildingBadHealth();
	int getFeatureGoodHealth();
	int getFeatureBadHealth();
	int getBuildingHealth(int iBuilding);
	int getPowerGoodHealth();
	int getPowerBadHealth();
	int getBonusGoodHealth();
	int getBonusBadHealth();
	int getMilitaryHappiness();
	int getMilitaryHappinessUnits();
	int getBuildingGoodHappiness();
	int getBuildingBadHappiness();
	int getBuildingHappiness(int iBuilding);
	int getExtraBuildingGoodHappiness();
	int getExtraBuildingBadHappiness();
	int getFeatureGoodHappiness();
	int getFeatureBadHappiness();
	int getBonusGoodHappiness();
	int getBonusBadHappiness();
	int getReligionGoodHappiness();
	int getReligionBadHappiness();
	int getReligionHappiness(int iReligion);
	int getExtraHappiness();
	void changeExtraHappiness(int iChange);
	int getHurryAngerTimer();
	void changeHurryAngerTimer(int iChange);
	int getConscriptAngerTimer();
	void changeConscriptAngerTimer(int iChange);
	bool isNoUnhappiness();
	bool isNoUnhealthyPopulation();

	bool isBuildingOnlyHealthy();

	int getFood();
	void setFood(int iNewValue);
	void changeFood(int iChange);
	int getFoodKept();
	int getMaxFoodKeptPercent();
	int getOverflowProduction();
	void setOverflowProduction(int iNewValue);
	int getMilitaryProductionModifier();
	int getSpaceProductionModifier();
	int getExtraTradeRoutes();
	int getTradeRouteModifier();
	int getBuildingDefense();
	int getFreeExperience();
	int getCurrAirlift();
	int getMaxAirlift();
	int getAirModifier();
	int getNukeModifier();
	int getFreeSpecialist();
	bool isPower();
	bool isAreaCleanPower();
	bool isDirtyPower();
	int getDefenseDamage();
	void changeDefenseDamage(int iChange);
	bool isBombardable(int iTeam);
	int getNaturalDefense();
	int getTotalDefense(bool bIgnoreBuilding);
	int getDefenseModifier(bool bIgnoreBuilding);

	int visiblePopulation(void);

	int getOccupationTimer();
	bool isOccupation();
	void setOccupationTimer(int iNewValue);
	void changeOccupationTimer(int iChange);
	bool isNeverLost();
	void setNeverLost(int iNewValue);
	bool isBombarded();
	void setBombarded(int iNewValue);
	bool isDrafted();
	void setDrafted(int iNewValue);
	bool isAirliftTargeted();
	void setAirliftTargeted(int iNewValue);
	bool isCitizensAutomated();
	void setCitizensAutomated(bool bNewValue);
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
	int getBaseYieldRate(int /*YieldTypes*/ eIndex);
	int getBaseYieldRateModifier(int /*YieldTypes*/ eIndex, int iExtra);
	int getYieldRate(int /*YieldTypes*/ eIndex);
	int getYieldRateModifier(int /*YieldTypes*/ eIndex);
	int getTradeYield(int /*YieldTypes*/ eIndex);
	int totalTradeModifier();

	int calculateTradeProfit(CyCity* pCity);
	int calculateTradeYield(int /*YieldTypes*/ eIndex, int iTradeProfit);

	int getExtraSpecialistYield(int /*YieldTypes*/ eIndex);
	int getExtraSpecialistYieldOfType(int /*YieldTypes*/ eIndex, int /*SpecialistTypes*/ eSpecialist);

	int getCommerceRate(int /*CommerceTypes*/ eIndex);
	int getCommerceFromPercent(int /*CommerceTypes*/ eIndex, int iYieldRate);
	int getBaseCommerceRate(int /*CommerceTypes*/ eIndex);
	int getTotalCommerceRateModifier(int /*CommerceTypes*/ eIndex);
	int getProductionToCommerceModifier(int /*CommerceTypes*/ eIndex);
	int getBuildingCommerce(int /*CommerceTypes*/ eIndex);
	int getBuildingCommerceByBuilding(int /*CommerceTypes*/ eIndex, int /*BuildingTypes*/ iBuilding);	
	int getSpecialistCommerce(int /*CommerceTypes*/ eIndex);
	void changeSpecialistCommerce(int /*CommerceTypes*/ eIndex, int iChange);
	int getReligionCommerce(int /*CommerceTypes*/ eIndex);
	int getReligionCommerceByReligion(int /*CommerceTypes*/ eIndex, int /*ReligionTypes*/ iReligion);
	int getCommerceRateModifier(int /*CommerceTypes*/ eIndex);
	int getCommerceHappinessPer(int /*CommerceTypes*/ eIndex);
	int getCommerceHappinessByType(int /*CommerceTypes*/ eIndex);
	int getCommerceHappiness();
	int getDomainFreeExperience(int /*DomainTypes*/ eIndex);
	int getDomainProductionModifier(int /*DomainTypes*/ eIndex);
	int getCulture(int /*PlayerTypes*/ eIndex);
	int countTotalCulture();
	PlayerTypes findHighestCulture();
	int calculateCulturePercent(int eIndex);
	int calculateTeamCulturePercent(int /*TeamTypes*/ eIndex);
	void setCulture(int /*PlayerTypes*/ eIndex, int iNewValue, bool bPlots);
	void changeCulture(int /*PlayerTypes*/ eIndex, int iChange, bool bPlots);

	bool isTradeRoute(int /*PlayerTypes*/ eIndex);
	bool isEverOwned(int /*PlayerTypes*/ eIndex);

	bool isRevealed(int /*TeamTypes*/ eIndex, bool bDebug);	
	void setRevealed(int /*TeamTypes*/ eIndex, bool bNewValue);	
	std::wstring getName();
	std::wstring getNameForm(int iForm);
	std::wstring getNameKey();
	void setName(std::wstring szNewValue, bool bFound);
	int getFreeBonus(int /*BonusTypes*/ eIndex);
	int getNumBonuses(int /*BonusTypes*/ iBonus);
	bool hasBonus(int /*BonusTypes */ iBonus);
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
	int getGreatPeopleUnitRate(int /*UnitTypes*/ iIndex);
	int getGreatPeopleUnitProgress(int /*UnitTypes*/ iIndex);
	void setGreatPeopleUnitProgress(int /*UnitTypes*/ iIndex, int iNewValue);
	void changeGreatPeopleUnitProgress(int /*UnitTypes*/ iIndex, int iChange);
	int getSpecialistCount(int /*SpecialistTypes*/ eIndex);
	void alterSpecialistCount(int /*SpecialistTypes*/ eIndex, int iChange);
	int getMaxSpecialistCount(int /*SpecialistTypes*/ eIndex);
	bool isSpecialistValid(int /*SpecialistTypes*/ eIndex, int iExtra);
	int getForceSpecialistCount(int /*SpecialistTypes*/ eIndex);
	bool isSpecialistForced();
	void setForceSpecialistCount(int /*SpecialistTypes*/ eIndex, int iNewValue);
	void changeForceSpecialistCount(int /*SpecialistTypes*/ eIndex, int iChange);
	int getFreeSpecialistCount(int /*SpecialistTypes*/ eIndex);
	void setFreeSpecialistCount(int /*SpecialistTypes*/ eIndex, int iNewValue);
	void changeFreeSpecialistCount(int /*SpecialistTypes*/ eIndex, int iChange);
	int getReligionInfluence(int /*ReligionTypes*/ iIndex);
	void changeReligionInfluence(int /*ReligionTypes*/ iIndex, int iChange);

	int getCurrentStateReligionHappiness();
	int getStateReligionHappiness(int /*ReligionTypes*/ eIndex);	
	void changeStateReligionHappiness(int /*ReligionTypes*/ eIndex, int iChange);

	int getUnitCombatFreeExperience(int /*UnitCombatTypes*/ eIndex);	
	int getFreePromotionCount(int /*PromotionTypes*/ eIndex);	
	bool isFreePromotion(int /*PromotionTypes*/ eIndex);

	bool isWorkingPlotByIndex(int iIndex);
	bool isWorkingPlot(CyPlot* pPlot);
	void alterWorkingPlot(int iIndex);
	bool isHasRealBuilding(int /*BuildingTypes*/ iIndex);
	void setHasRealBuilding(int /*BuildingTypes*/ iIndex, bool bNewValue);
	bool isFreeBuilding(int /*BuildingTypes*/ iIndex);
	bool isHasReligion(int /*ReligionTypes*/ iIndex);
	void setHasReligion(int /*ReligionTypes*/ iIndex, bool bNewValue, bool bAnnounce, bool bArrows);
	CyCity* getTradeCity(int iIndex);
	int getTradeRoutes();

	void clearOrderQueue();
	void pushOrder(OrderTypes eOrder, int iData1, int iData2, bool bSave, bool bPop, bool bAppend, bool bForce);
	void popOrder(int iNum, bool bFinish, bool bChoose);
	int getOrderQueueLength();
	OrderData* getOrderFromQueue(int iIndex);

	void setWallOverridePoints(const python::tuple& kPoints); /* points are given in world space ... i.e. PlotXToPointX, etc */
	python::tuple getWallOverridePoints() const;

	bool AI_isEmphasize(int iEmphasizeType);
	int AI_countBestBuilds(CyArea* pArea);

	std::string getScriptData() const;
	void setScriptData(std::string szNewValue);

private:
	CvCity* m_pCity;
};

#endif	// CyCity_h