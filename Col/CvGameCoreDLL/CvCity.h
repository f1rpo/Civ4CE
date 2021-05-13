#pragma once

// city.h

#ifndef CIV4_CITY_H
#define CIV4_CITY_H

#include "CvDLLEntity.h"
#include "LinkedList.h"

class CvPlot;
class CvPlotGroup;
class CvArea;
class CvGenericBuilding;

class CvCity : public CvDLLEntity
{

public:
	CvCity();
	virtual ~CvCity();

	void init(int iID, PlayerTypes eOwner, int iX, int iY, bool bBumpUnits, bool bUpdatePlotGroups);
	void uninit();
	void reset(int iID = 0, PlayerTypes eOwner = NO_PLAYER, int iX = 0, int iY = 0, bool bConstructorCall = false);
	void setupGraphical();

	DllExport void kill(bool bUpdatePlotGroups);

	void doTurn();

	bool isCitySelected();
	DllExport bool canBeSelected() const;
	DllExport void updateSelectedCity();

	void updateYield();

	void updateVisibility();

	void createGreatPeople(UnitTypes eGreatPersonUnit, bool bIncrementThreshold, bool bIncrementExperience);

	DllExport void doTask(TaskTypes eTask, int iData1 = -1, int iData2 = -1, bool bOption = false, bool bAlt = false, bool bShift = false, bool bCtrl = false);

	DllExport void chooseProduction(UnitTypes eTrainUnit = NO_UNIT, BuildingTypes eConstructBuilding = NO_BUILDING, ProjectTypes eCreateProject = NO_PROJECT, bool bFinish = false, bool bFront = false);

	DllExport int getCityPlotIndex(const CvPlot* pPlot) const;
	CvPlot* getCityIndexPlot(int iIndex) const;

	bool canWork(CvPlot* pPlot) const;
	void verifyWorkingPlot(int iIndex);
	void verifyWorkingPlots();
	void clearWorkingOverride(int iIndex);
	int countNumImprovedPlots(ImprovementTypes eImprovement = NO_IMPROVEMENT, bool bPotential = false) const;
	int countNumWaterPlots() const;
	int countNumRiverPlots() const;

	int findPopulationRank() const;
	int findBaseYieldRateRank(YieldTypes eYield) const;
	DllExport int findYieldRateRank(YieldTypes eYield) const;
	DllExport int findCommerceRateRank(CommerceTypes eCommerce) const;

	UnitTypes allUpgradesAvailable(UnitTypes eUnit, int iUpgradeCount = 0) const;
	DllExport bool isWorldWondersMaxed() const;
	DllExport bool isTeamWondersMaxed() const;
	DllExport bool isNationalWondersMaxed() const;
	DllExport bool isBuildingsMaxed() const;

	DllExport bool canTrain(UnitTypes eUnit, bool bContinue = false, bool bTestVisible = false, bool bIgnoreCost = false, bool bIgnoreUpgrades = false) const;
	DllExport bool canTrain(UnitCombatTypes eUnitCombat) const;
	DllExport bool canConstruct(BuildingTypes eBuilding, bool bContinue = false, bool bTestVisible = false, bool bIgnoreCost = false) const;
	DllExport bool canCreate(ProjectTypes eProject, bool bContinue = false, bool bTestVisible = false) const;
	DllExport bool canMaintain(ProcessTypes eProcess, bool bContinue = false) const;
	bool canJoin() const;

	DllExport int getFoodTurnsLeft() const;
	DllExport bool isProduction() const;
	bool isProductionLimited() const;
	bool isProductionUnit() const;
	bool isProductionBuilding() const;
	bool isProductionProject() const;
	DllExport bool isProductionProcess() const;

	bool canContinueProduction(OrderData order);
	int getProductionExperience(UnitTypes eUnit = NO_UNIT);
	void addProductionExperience(CvUnit* pUnit, bool bConscript = false);

	DllExport UnitTypes getProductionUnit() const;
	UnitAITypes getProductionUnitAI() const;
	DllExport BuildingTypes getProductionBuilding() const;
	DllExport ProjectTypes getProductionProject() const;
	DllExport ProcessTypes getProductionProcess() const;
	DllExport const wchar* getProductionName() const;
	DllExport const wchar* getProductionNameKey() const;
	DllExport int getGeneralProductionTurnsLeft() const;

	bool isFoodProduction() const;
	bool isFoodProduction(UnitTypes eUnit) const;
	int getFirstUnitOrder(UnitTypes eUnit) const;
	int getFirstBuildingOrder(BuildingTypes eBuilding) const;
	int getFirstProjectOrder(ProjectTypes eProject) const;
	int getNumTrainUnitAI(UnitAITypes eUnitAI) const;

	DllExport int getProduction() const;
	DllExport int getProductionNeeded() const;
	DllExport int getProductionNeeded(UnitTypes eUnit) const;
	DllExport int getProductionNeeded(BuildingTypes eBuilding) const;
	DllExport int getProductionNeeded(ProjectTypes eProject) const;
	DllExport int getProductionTurnsLeft() const;
	DllExport int getProductionTurnsLeft(UnitTypes eUnit, int iNum) const;
	DllExport int getProductionTurnsLeft(BuildingTypes eBuilding, int iNum) const;
	DllExport int getProductionTurnsLeft(ProjectTypes eProject, int iNum) const;
	int getProductionTurnsLeft(int iProductionNeeded, int iProduction, int iFirstProductionDifference, int iProductionDifference) const;
	void setProduction(int iNewValue);
	void changeProduction(int iChange);

	int getProductionModifier() const;
	int getProductionModifier(UnitTypes eUnit) const;
	int getProductionModifier(BuildingTypes eBuilding) const;
	int getProductionModifier(ProjectTypes eProject) const;

	int getOverflowProductionDifference(int iProductionNeeded, int iProduction, int iProductionModifier, int iDiff, int iModifiedProduction) const;
	int getProductionDifference(int iProductionNeeded, int iProduction, int iProductionModifier, bool bFoodProduction, bool bOverflow) const;
	DllExport int getCurrentProductionDifference(bool bIgnoreFood, bool bOverflow) const;
	int getExtraProductionDifference(int iExtra) const;

	DllExport bool canHurry(HurryTypes eHurry, bool bTestVisible = false) const;
	void hurry(HurryTypes eHurry);

	DllExport UnitTypes getConscriptUnit() const;
	CvUnit* initConscriptedUnit();
	DllExport int getConscriptPopulation() const;
	int conscriptMinCityPopulation() const;
	int flatConscriptAngerLength() const;
	DllExport bool canConscript() const;
	void conscript();

	int getBonusHealth(BonusTypes eBonus) const;																 - getBonusHealth
	int getBonusHappiness(BonusTypes eBonus) const;															 - getBonusHappiness
	int getBonusPower(BonusTypes eBonus, bool bDirty) const;
	int getBonusYieldRateModifier(YieldTypes eIndex, BonusTypes eBonus) const;

	void processBonus(BonusTypes eBonus, int iChange);
	void processBuilding(BuildingTypes eBuilding, int iChange, bool bObsolete = false);
	void processProcess(ProcessTypes eProcess, int iChange);
	void processSpecialist(SpecialistTypes eSpecialist, int iChange);

	DllExport HandicapTypes getHandicapType() const;
	DllExport CivilizationTypes getCivilizationType() const;
	LeaderHeadTypes getPersonalityType() const;
	ArtStyleTypes getArtStyleType() const;
	DllExport CitySizeTypes getCitySizeType() const;

	bool hasTrait(TraitTypes eTrait) const;
	bool isBarbarian() const;
	bool isHuman() const;
	DllExport bool isVisible(TeamTypes eTeam, bool bDebug) const;

	DllExport bool isCapital() const;
	bool isCoastal(int iMinWaterSize) const;
	DllExport bool isDisorder() const;
	DllExport bool isHolyCity(ReligionTypes eIndex) const;
	DllExport bool isHolyCity() const;
	DllExport bool isHeadquarters(CorporationTypes eIndex) const;
	DllExport bool isHeadquarters() const;
	void setHeadquarters(CorporationTypes eIndex);

	int getOvercrowdingPercentAnger(int iExtra = 0) const;
	int getNoMilitaryPercentAnger() const;
	int getCulturePercentAnger() const;
	int getReligionPercentAnger() const;
	int getHurryPercentAnger(int iExtra = 0) const;
	int getConscriptPercentAnger(int iExtra = 0) const;
	int getDefyResolutionPercentAnger(int iExtra = 0) const;
	int getWarWearinessPercentAnger() const;
	int getLargestCityHappiness() const;
	int getVassalHappiness() const;
	int getVassalUnhappiness() const;
	int unhappyLevel(int iExtra = 0) const;
	DllExport int happyLevel() const;
	DllExport int angryPopulation(int iExtra = 0) const;

	int visiblePopulation() const;
	DllExport int totalFreeSpecialists() const;
	int extraPopulation() const;
	int extraSpecialists() const;
	int extraFreeSpecialists() const;

	DllExport int unhealthyPopulation(bool bNoAngry = false, int iExtra = 0) const;
	int totalGoodBuildingHealth() const;
	DllExport int totalBadBuildingHealth() const;
	DllExport int goodHealth() const;
	DllExport int badHealth(bool bNoAngry = false, int iExtra = 0) const;
	DllExport int healthRate(bool bNoAngry = false, int iExtra = 0) const;
	int foodConsumption(bool bNoAngry = false, int iExtra = 0) const;
	DllExport int foodDifference(bool bBottom = true) const;
	DllExport int growthThreshold() const;

	int productionLeft() const;
	int hurryCost(bool bExtra) const;
	int getHurryCostModifier(bool bIgnoreNew = false) const;
	DllExport int hurryGold(HurryTypes eHurry) const;
	DllExport int hurryPopulation(HurryTypes eHurry) const;
	int hurryProduction(HurryTypes eHurry) const;
	int flatHurryAngerLength() const;
	int hurryAngerLength(HurryTypes eHurry) const;
	int maxHurryPopulation() const;

	int cultureDistance(int iDX, int iDY) const;
	DllExport int cultureStrength(PlayerTypes ePlayer) const;
	DllExport int cultureGarrison(PlayerTypes ePlayer) const;

	DllExport int getNumBuilding(BuildingTypes eIndex) const;
	DllExport int getNumActiveBuilding(BuildingTypes eIndex) const;
	bool hasActiveWorldWonder() const;

	int getReligionCount() const;
	int getCorporationCount() const;

	DllExport int getID() const;
	DllExport int getIndex() const;
	DllExport IDInfo getIDInfo() const;
	void setID(int iID);

	DllExport int getX() const;
#ifdef _USRDLL
	inline int getX_INLINE() const
	{
		return m_iX;
	}
#endif
	DllExport int getY() const;
#ifdef _USRDLL
	inline int getY_INLINE() const
	{
		return m_iY;
	}
#endif
	bool at(int iX, int iY) const;
	bool at(CvPlot* pPlot) const;																					 - atPlot
	DllExport CvPlot* plot() const;
	CvPlotGroup* plotGroup(PlayerTypes ePlayer) const;
	bool isConnectedTo(CvCity* pCity) const;
	DllExport bool isConnectedToCapital(PlayerTypes ePlayer = NO_PLAYER) const;
	int getArea() const;
	DllExport CvArea* area() const;
	CvArea* waterArea() const;

	DllExport CvPlot* getRallyPlot() const;
	void setRallyPlot(CvPlot* pPlot);

	int getGameTurnFounded() const;
	void setGameTurnFounded(int iNewValue);

	int getGameTurnAcquired() const;
	void setGameTurnAcquired(int iNewValue);

	DllExport int getPopulation() const;
	DllExport void setPopulation(int iNewValue);
	DllExport void changePopulation(int iChange);

	long getRealPopulation() const;

	int getHighestPopulation() const;
	void setHighestPopulation(int iNewValue);

	int getWorkingPopulation() const;
	void changeWorkingPopulation(int iChange);

	int getSpecialistPopulation() const;
	void changeSpecialistPopulation(int iChange);

	int getNumGreatPeople() const;
	void changeNumGreatPeople(int iChange);

	int getBaseGreatPeopleRate() const;
	int getGreatPeopleRate() const;
	int getTotalGreatPeopleRateModifier() const;
	void changeBaseGreatPeopleRate(int iChange);

	int getGreatPeopleRateModifier() const;
	void changeGreatPeopleRateModifier(int iChange);

	DllExport int getGreatPeopleProgress() const;
	void changeGreatPeopleProgress(int iChange);

	int getNumWorldWonders() const;
	void changeNumWorldWonders(int iChange);

	int getNumTeamWonders() const;
	void changeNumTeamWonders(int iChange);

	int getNumNationalWonders() const;
	void changeNumNationalWonders(int iChange);

	int getNumBuildings() const;
	void changeNumBuildings(int iChange);

	int getGovernmentCenterCount() const;
	DllExport bool isGovernmentCenter() const;
	void changeGovernmentCenterCount(int iChange);

	DllExport int getMaintenance() const;
	int getMaintenanceTimes100() const;
	void updateMaintenance();
	DllExport int calculateDistanceMaintenance() const;
	DllExport int calculateNumCitiesMaintenance() const;
	DllExport int calculateColonyMaintenance() const;
	DllExport int calculateCorporationMaintenance() const;
	int calculateDistanceMaintenanceTimes100() const;
	int calculateNumCitiesMaintenanceTimes100() const;
	int calculateColonyMaintenanceTimes100() const;
	int calculateCorporationMaintenanceTimes100(CorporationTypes eCorporation) const;
	int calculateCorporationMaintenanceTimes100() const;
	int calculateBaseMaintenanceTimes100() const;
	DllExport int getMaintenanceModifier() const;
	void changeMaintenanceModifier(int iChange);

	int getWarWearinessModifier() const;
	void changeWarWearinessModifier(int iChange);

	int getHurryAngerModifier() const;
	void changeHurryAngerModifier(int iChange);

	int getHealRate() const;
	void changeHealRate(int iChange);

	DllExport int getEspionageHealthCounter() const;
	void changeEspionageHealthCounter(int iChange);

	DllExport int getEspionageHappinessCounter() const;
	void changeEspionageHappinessCounter(int iChange);

	int getFreshWaterGoodHealth() const;
	DllExport int getFreshWaterBadHealth() const;
	void updateFreshWaterHealth();

	int getFeatureGoodHealth() const;
	DllExport int getFeatureBadHealth() const;
	void updateFeatureHealth();

	int getBuildingGoodHealth() const;
	int getBuildingBadHealth() const;
	int getBuildingHealth(BuildingTypes eBuilding) const;
	void changeBuildingGoodHealth(int iChange);
	void changeBuildingBadHealth(int iChange);

	int getPowerGoodHealth() const;
	DllExport int getPowerBadHealth() const;
	void updatePowerHealth();

	int getBonusGoodHealth() const;
	DllExport int getBonusBadHealth() const;
	void changeBonusGoodHealth(int iChange);
	void changeBonusBadHealth(int iChange);

	int getMilitaryHappiness() const;
	int getMilitaryHappinessUnits() const;
	void changeMilitaryHappinessUnits(int iChange);

	int getBuildingGoodHappiness() const;
	int getBuildingBadHappiness() const;
	int getBuildingHappiness(BuildingTypes eBuilding) const;
	void changeBuildingGoodHappiness(int iChange);
	void changeBuildingBadHappiness(int iChange);

	int getExtraBuildingGoodHappiness() const;
	int getExtraBuildingBadHappiness() const;
	void updateExtraBuildingHappiness();

	int getExtraBuildingGoodHealth() const;
	int getExtraBuildingBadHealth() const;
	void updateExtraBuildingHealth();

	int getFeatureGoodHappiness() const;
	int getFeatureBadHappiness() const;
	void updateFeatureHappiness();

	int getBonusGoodHappiness() const;
	int getBonusBadHappiness() const;
	void changeBonusGoodHappiness(int iChange);
	void changeBonusBadHappiness(int iChange);

	int getReligionGoodHappiness() const;
	int getReligionBadHappiness() const;
	int getReligionHappiness(ReligionTypes eReligion) const;
	void updateReligionHappiness();

	int getExtraHappiness() const;
	void changeExtraHappiness(int iChange);

	int getExtraHealth() const;
	void changeExtraHealth(int iChange);

	int getHurryAngerTimer() const;
	void changeHurryAngerTimer(int iChange);

	int getConscriptAngerTimer() const;
	void changeConscriptAngerTimer(int iChange);

	int getDefyResolutionAngerTimer() const;
	void changeDefyResolutionAngerTimer(int iChange);
	int flatDefyResolutionAngerLength() const;

	int getHappinessTimer() const;
	void changeHappinessTimer(int iChange);

	int getNoUnhappinessCount() const;
	bool isNoUnhappiness() const;
	void changeNoUnhappinessCount(int iChange);

	int getNoUnhealthyPopulationCount() const;
	bool isNoUnhealthyPopulation() const;
	void changeNoUnhealthyPopulationCount(int iChange);

	int getBuildingOnlyHealthyCount() const;
	bool isBuildingOnlyHealthy() const;
	void changeBuildingOnlyHealthyCount(int iChange);

	DllExport int getFood() const;
	void setFood(int iNewValue);
	void changeFood(int iChange);

	int getFoodKept() const;
	void setFoodKept(int iNewValue);
	void changeFoodKept(int iChange);

	int getMaxFoodKeptPercent() const;
	void changeMaxFoodKeptPercent(int iChange);

	int getOverflowProduction() const;
	void setOverflowProduction(int iNewValue);
	void changeOverflowProduction(int iChange, int iProductionModifier);

	int getFeatureProduction()const;
	void setFeatureProduction(int iNewValue);
	void changeFeatureProduction(int iChange);

	int getMilitaryProductionModifier() const;
	void changeMilitaryProductionModifier(int iChange);

	int getSpaceProductionModifier() const;
	void changeSpaceProductionModifier(int iChange);

	int getExtraTradeRoutes() const;
	void changeExtraTradeRoutes(int iChange);

	int getTradeRouteModifier() const;
	void changeTradeRouteModifier(int iChange);

	int getForeignTradeRouteModifier() const;
	void changeForeignTradeRouteModifier(int iChange);

	int getBuildingDefense() const;
	void changeBuildingDefense(int iChange);

	int getBuildingBombardDefense() const;
	void changeBuildingBombardDefense(int iChange);

	int getFreeExperience() const;
	void changeFreeExperience(int iChange);

	int getCurrAirlift() const;
	void setCurrAirlift(int iNewValue);
	void changeCurrAirlift(int iChange);

	int getMaxAirlift() const;
	void changeMaxAirlift(int iChange);

	int getAirModifier() const;
	void changeAirModifier(int iChange);

	DllExport int getAirUnitCapacity(TeamTypes eTeam) const;
	void changeAirUnitCapacity(int iChange);

	int getNukeModifier() const;
	void changeNukeModifier(int iChange);

	int getFreeSpecialist() const;
	void changeFreeSpecialist(int iChange);

	int getPowerCount() const;
	bool isPower() const;
	bool isAreaCleanPower() const;
	int getDirtyPowerCount() const;
	bool isDirtyPower() const;
	void changePowerCount(int iChange, bool bDirty);

	bool isAreaBorderObstacle() const;

	int getDefenseDamage() const;
	void changeDefenseDamage(int iChange);
	void changeDefenseModifier(int iChange);

	int getLastDefenseDamage() const;
	void setLastDefenseDamage(int iNewValue);

	bool isBombardable(const CvUnit* pUnit) const;
	int getNaturalDefense() const;
	int getTotalDefense(bool bIgnoreBuilding) const;
	int getDefenseModifier(bool bIgnoreBuilding) const;

	int getOccupationTimer() const;
	bool isOccupation() const;
	void setOccupationTimer(int iNewValue);
	void changeOccupationTimer(int iChange);

	int getCultureUpdateTimer() const;
	void setCultureUpdateTimer(int iNewValue);
	void changeCultureUpdateTimer(int iChange);

	int getCitySizeBoost() const;
	void setCitySizeBoost(int iBoost);

	bool isNeverLost() const;
	void setNeverLost(bool bNewValue);

	bool isBombarded() const;
	void setBombarded(bool bNewValue);

	bool isDrafted() const;
	void setDrafted(bool bNewValue);

	bool isAirliftTargeted() const;
	void setAirliftTargeted(bool bNewValue);

	DllExport bool isWeLoveTheKingDay() const;
	void setWeLoveTheKingDay(bool bNewValue);

	DllExport bool isCitizensAutomated() const;
	void setCitizensAutomated(bool bNewValue);

	DllExport bool isProductionAutomated() const;
	void setProductionAutomated(bool bNewValue, bool bClear);

	/* allows you to programatically specify a cities walls rather than having them be generated automagically */
	DllExport bool isWallOverride() const;
	DllExport void setWallOverride(bool bOverride);

	DllExport bool isInfoDirty() const;
	DllExport void setInfoDirty(bool bNewValue);

	DllExport bool isLayoutDirty() const;
	DllExport void setLayoutDirty(bool bNewValue);

	bool isPlundered() const;
	void setPlundered(bool bNewValue);

	DllExport PlayerTypes getOwner() const;
#ifdef _USRDLL
	inline PlayerTypes getOwnerINLINE() const
	{
		return m_eOwner;
	}
#endif
	DllExport TeamTypes getTeam() const;

	PlayerTypes getPreviousOwner() const;
	void setPreviousOwner(PlayerTypes eNewValue);

	PlayerTypes getOriginalOwner() const;
	void setOriginalOwner(PlayerTypes eNewValue);

	CultureLevelTypes getCultureLevel() const;
	DllExport int getCultureThreshold() const;
	static int getCultureThreshold(CultureLevelTypes eLevel);
	void setCultureLevel(CultureLevelTypes eNewValue, bool bUpdatePlotGroups);
	void updateCultureLevel(bool bUpdatePlotGroups);

	int getSeaPlotYield(YieldTypes eIndex) const;
	void changeSeaPlotYield(YieldTypes eIndex, int iChange);

	int getRiverPlotYield(YieldTypes eIndex) const;
	void changeRiverPlotYield(YieldTypes eIndex, int iChange);

	int getBaseYieldRate(YieldTypes eIndex) const;
	int getBaseYieldRateModifier(YieldTypes eIndex, int iExtra = 0) const;
	DllExport int getYieldRate(YieldTypes eIndex) const;
	void setBaseYieldRate(YieldTypes eIndex, int iNewValue);
	void changeBaseYieldRate(YieldTypes eIndex, int iChange);

	int getYieldRateModifier(YieldTypes eIndex) const;
	void changeYieldRateModifier(YieldTypes eIndex, int iChange);

	int getPowerYieldRateModifier(YieldTypes eIndex) const;
	void changePowerYieldRateModifier(YieldTypes eIndex, int iChange);

	int getBonusYieldRateModifier(YieldTypes eIndex) const;
	void changeBonusYieldRateModifier(YieldTypes eIndex, int iChange);

	int getTradeYield(YieldTypes eIndex) const;
	int totalTradeModifier(CvCity* pOtherCity = NULL) const;
	int getPopulationTradeModifier() const;
	int getPeaceTradeModifier(TeamTypes eTeam) const;
	int getBaseTradeProfit(CvCity* pCity) const;
	int calculateTradeProfit(CvCity* pCity) const;
	int calculateTradeYield(YieldTypes eIndex, int iTradeProfit) const;
	void setTradeYield(YieldTypes eIndex, int iNewValue);

	int getExtraSpecialistYield(YieldTypes eIndex) const;
	int getExtraSpecialistYield(YieldTypes eIndex, SpecialistTypes eSpecialist) const;
	void updateExtraSpecialistYield(YieldTypes eYield);
	void updateExtraSpecialistYield();

	DllExport int getCommerceRate(CommerceTypes eIndex) const;
	int getCommerceRateTimes100(CommerceTypes eIndex) const;
	int getCommerceFromPercent(CommerceTypes eIndex, int iYieldRate) const;
	int getBaseCommerceRate(CommerceTypes eIndex) const;
	int getBaseCommerceRateTimes100(CommerceTypes eIndex) const;
	int getTotalCommerceRateModifier(CommerceTypes eIndex) const;
	void updateCommerce(CommerceTypes eIndex);
	void updateCommerce();

	int getProductionToCommerceModifier(CommerceTypes eIndex) const;
	void changeProductionToCommerceModifier(CommerceTypes eIndex, int iChange);

	int getBuildingCommerce(CommerceTypes eIndex) const;
	int getBuildingCommerceByBuilding(CommerceTypes eIndex, BuildingTypes eBuilding) const;
	void updateBuildingCommerce();

	int getSpecialistCommerce(CommerceTypes eIndex) const;
	void changeSpecialistCommerce(CommerceTypes eIndex, int iChange);

	int getReligionCommerce(CommerceTypes eIndex) const;
	int getReligionCommerceByReligion(CommerceTypes eIndex, ReligionTypes eReligion) const;
	void updateReligionCommerce(CommerceTypes eIndex);
	void updateReligionCommerce();

	void setCorporationYield(YieldTypes eIndex, int iNewValue);
	int getCorporationCommerce(CommerceTypes eIndex) const;
	int getCorporationCommerceByCorporation(CommerceTypes eIndex, CorporationTypes eCorporation) const;
	int getCorporationYield(YieldTypes eIndex) const;
	int getCorporationYieldByCorporation(YieldTypes eIndex, CorporationTypes eCorporation) const;
	void updateCorporation();
	void updateCorporationCommerce(CommerceTypes eIndex);
	void updateCorporationYield(YieldTypes eIndex);
	void updateCorporationBonus();

	int getCommerceRateModifier(CommerceTypes eIndex) const;
	void changeCommerceRateModifier(CommerceTypes eIndex, int iChange);

	int getCommerceHappinessPer(CommerceTypes eIndex) const;
	int getCommerceHappinessByType(CommerceTypes eIndex) const;
	int getCommerceHappiness() const;
	void changeCommerceHappinessPer(CommerceTypes eIndex, int iChange);

	int getDomainFreeExperience(DomainTypes eIndex) const;
	void changeDomainFreeExperience(DomainTypes eIndex, int iChange);

	int getDomainProductionModifier(DomainTypes eIndex) const;
	void changeDomainProductionModifier(DomainTypes eIndex, int iChange);

	DllExport int getCulture(PlayerTypes eIndex) const;
	int getCultureTimes100(PlayerTypes eIndex) const;
	int countTotalCultureTimes100() const;
	PlayerTypes findHighestCulture() const;
	int calculateCulturePercent(PlayerTypes eIndex) const;
	int calculateTeamCulturePercent(TeamTypes eIndex) const;
	void setCulture(PlayerTypes eIndex, int iNewValue, bool bPlots, bool bUpdatePlotGroups);
	void setCultureTimes100(PlayerTypes eIndex, int iNewValue, bool bPlots, bool bUpdatePlotGroups);
	DllExport void changeCulture(PlayerTypes eIndex, int iChange, bool bPlots, bool bUpdatePlotGroups);
	void changeCultureTimes100(PlayerTypes eIndex, int iChange, bool bPlots, bool bUpdatePlotGroups);

	int getNumRevolts(PlayerTypes eIndex) const;
	void changeNumRevolts(PlayerTypes eIndex, int iChange);
	int getRevoltTestProbability() const;

	bool isTradeRoute(PlayerTypes eIndex) const;
	void setTradeRoute(PlayerTypes eIndex, bool bNewValue);

	bool isEverOwned(PlayerTypes eIndex) const;
	void setEverOwned(PlayerTypes eIndex, bool bNewValue);

	DllExport bool isRevealed(TeamTypes eIndex, bool bDebug) const;
	DllExport void setRevealed(TeamTypes eIndex, bool bNewValue);

	DllExport bool getEspionageVisibility(TeamTypes eTeam) const;
	void setEspionageVisibility(TeamTypes eTeam, bool bVisible, bool bUpdatePlotGroups);
	void updateEspionageVisibility(bool bUpdatePlotGroups);

	DllExport const CvWString getName(uint uiForm = 0) const;
	DllExport const wchar* getNameKey() const;
	DllExport void setName(const wchar* szNewValue, bool bFound = false);
	void doFoundMessage();

	// Script data needs to be a narrow string for pickling in Python
	std::string getScriptData() const;
	void setScriptData(std::string szNewValue);

	int getFreeBonus(BonusTypes eIndex) const;
	void changeFreeBonus(BonusTypes eIndex, int iChange);

	int getNumBonuses(BonusTypes eIndex) const;
	DllExport bool hasBonus(BonusTypes eIndex) const;
	void changeNumBonuses(BonusTypes eIndex, int iChange);

	int getNumCorpProducedBonuses(BonusTypes eIndex) const;
	bool isCorporationBonus(BonusTypes eBonus) const;
	bool isActiveCorporation(CorporationTypes eCorporation) const;

	DllExport int getBuildingProduction(BuildingTypes eIndex) const;
	void setBuildingProduction(BuildingTypes eIndex, int iNewValue);
	void changeBuildingProduction(BuildingTypes eIndex, int iChange);

	int getBuildingProductionTime(BuildingTypes eIndex) const;
	void setBuildingProductionTime(BuildingTypes eIndex, int iNewValue);
	void changeBuildingProductionTime(BuildingTypes eIndex, int iChange);

	DllExport int getProjectProduction(ProjectTypes eIndex) const;
	void setProjectProduction(ProjectTypes eIndex, int iNewValue);
	void changeProjectProduction(ProjectTypes eIndex, int iChange);

	int getBuildingOriginalOwner(BuildingTypes eIndex) const;
	int getBuildingOriginalTime(BuildingTypes eIndex) const;

	DllExport int getUnitProduction(UnitTypes eIndex) const;
	void setUnitProduction(UnitTypes eIndex, int iNewValue);
	void changeUnitProduction(UnitTypes eIndex, int iChange);

	int getUnitProductionTime(UnitTypes eIndex) const;
	void setUnitProductionTime(UnitTypes eIndex, int iNewValue);
	void changeUnitProductionTime(UnitTypes eIndex, int iChange);

	int getGreatPeopleUnitRate(UnitTypes eIndex) const;
	void setGreatPeopleUnitRate(UnitTypes eIndex, int iNewValue);
	void changeGreatPeopleUnitRate(UnitTypes eIndex, int iChange);

	DllExport int getGreatPeopleUnitProgress(UnitTypes eIndex) const;
	void setGreatPeopleUnitProgress(UnitTypes eIndex, int iNewValue);
	void changeGreatPeopleUnitProgress(UnitTypes eIndex, int iChange);

	DllExport int getSpecialistCount(SpecialistTypes eIndex) const;
	void setSpecialistCount(SpecialistTypes eIndex, int iNewValue);
	void changeSpecialistCount(SpecialistTypes eIndex, int iChange);
	void alterSpecialistCount(SpecialistTypes eIndex, int iChange);

	DllExport int getMaxSpecialistCount(SpecialistTypes eIndex) const;
	bool isSpecialistValid(SpecialistTypes eIndex, int iExtra = 0) const;
	void changeMaxSpecialistCount(SpecialistTypes eIndex, int iChange);

	DllExport int getForceSpecialistCount(SpecialistTypes eIndex) const;
	bool isSpecialistForced() const;
	void setForceSpecialistCount(SpecialistTypes eIndex, int iNewValue);
	void changeForceSpecialistCount(SpecialistTypes eIndex, int iChange);

	int getFreeSpecialistCount(SpecialistTypes eIndex) const;
	void setFreeSpecialistCount(SpecialistTypes eIndex, int iNewValue);
	void changeFreeSpecialistCount(SpecialistTypes eIndex, int iChange);
	int getAddedFreeSpecialistCount(SpecialistTypes eIndex) const;

	int getImprovementFreeSpecialists(ImprovementTypes eIndex) const;
	void changeImprovementFreeSpecialists(ImprovementTypes eIndex, int iChange);

	int getReligionInfluence(ReligionTypes eIndex) const;
	void changeReligionInfluence(ReligionTypes eIndex, int iChange);

	int getCurrentStateReligionHappiness() const;
	int getStateReligionHappiness(ReligionTypes eIndex) const;
	void changeStateReligionHappiness(ReligionTypes eIndex, int iChange);

	int getUnitCombatFreeExperience(UnitCombatTypes eIndex) const;
	void changeUnitCombatFreeExperience(UnitCombatTypes eIndex, int iChange);

	int getFreePromotionCount(PromotionTypes eIndex) const;
	bool isFreePromotion(PromotionTypes eIndex) const;
	void changeFreePromotionCount(PromotionTypes eIndex, int iChange);

	int getSpecialistFreeExperience() const;
	void changeSpecialistFreeExperience(int iChange);

	int getEspionageDefenseModifier() const;
	void changeEspionageDefenseModifier(int iChange);

	DllExport bool isWorkingPlot(int iIndex) const;
	bool isWorkingPlot(const CvPlot* pPlot) const;
	void setWorkingPlot(int iIndex, bool bNewValue);
	void setWorkingPlot(CvPlot* pPlot, bool bNewValue);
	void alterWorkingPlot(int iIndex);

	int getNumRealBuilding(BuildingTypes eIndex) const;
	DllExport void setNumRealBuilding(BuildingTypes eIndex, int iNewValue);
	void setNumRealBuildingTimed(BuildingTypes eIndex, int iNewValue, bool bFirst, PlayerTypes eOriginalOwner, int iOriginalTime);

	bool isValidBuildingLocation(BuildingTypes eIndex) const;

	int getNumFreeBuilding(BuildingTypes eIndex) const;
	void setNumFreeBuilding(BuildingTypes eIndex, int iNewValue);

	DllExport bool isHasReligion(ReligionTypes eIndex) const;
	DllExport void setHasReligion(ReligionTypes eIndex, bool bNewValue, bool bAnnounce, bool bArrows = true);

	DllExport bool isHasCorporation(CorporationTypes eIndex) const;
	DllExport void setHasCorporation(CorporationTypes eIndex, bool bNewValue, bool bAnnounce, bool bArrows = true);

	CvCity* getTradeCity(int iIndex) const;
	int getTradeRoutes() const;
	void clearTradeRoutes();
	void updateTradeRoutes();

	void clearOrderQueue();
	DllExport void pushOrder(OrderTypes eOrder, int iData1, int iData2, bool bSave, bool bPop, bool bAppend, bool bForce = false);
	DllExport void popOrder(int iNum, bool bFinish = false, bool bChoose = false);
	void startHeadOrder();
	void stopHeadOrder();
	DllExport int getOrderQueueLength();
	DllExport OrderData* getOrderFromQueue(int iIndex);
	DllExport CLLNode<OrderData>* nextOrderQueueNode(CLLNode<OrderData>* pNode) const;
	DllExport CLLNode<OrderData>* headOrderQueueNode() const;
	CLLNode<OrderData>* tailOrderQueueNode() const;

	// fill the kVisible array with buildings that you want shown in city, as well as the number of generics
	// This function is called whenever CvCity::setLayoutDirty() is called
	DllExport void getVisibleBuildings(std::list<BuildingTypes>& kVisible, int& iNumGenerics);

	// Fill the kEffectNames array with references to effects in the CIV4EffectInfos.xml to have a
	// city play a given set of effects. This is called whenever the interface updates the city billboard
	// or when the zoom level changes
	DllExport void getVisibleEffects(ZoomLevelTypes eCurrentZoom, std::vector<const TCHAR*>& kEffectNames);


	// Billboard appearance controls
	DllExport void getCityBillboardSizeIconColors(NiColorA& kDotColor, NiColorA& kTextColor) const;
	DllExport const TCHAR* getCityBillboardProductionIcon() const;


	DllExport void setWallOverridePoints(const std::vector< std::pair<float, float> >& kPoints); /* points are given in world space ... i.e. PlotXToPointX, etc */
	DllExport const std::vector< std::pair<float, float> >& getWallOverridePoints() const;

	int getTriggerValue(EventTriggerTypes eTrigger) const;
	bool canApplyEvent(EventTypes eEvent, const EventTriggeredData& kTriggeredData) const;
	void applyEvent(EventTypes eEvent, const EventTriggeredData& kTriggeredData, bool bClear);
	bool isEventOccured(EventTypes eEvent) const;
	void setEventOccured(EventTypes eEvent, bool bOccured);

	int getBuildingYieldChange(BuildingClassTypes eBuildingClass, YieldTypes eYield) const;
	void setBuildingYieldChange(BuildingClassTypes eBuildingClass, YieldTypes eYield, int iChange);
	void changeBuildingYieldChange(BuildingClassTypes eBuildingClass, YieldTypes eYield, int iChange);
	int getBuildingCommerceChange(BuildingClassTypes eBuildingClass, CommerceTypes eCommerce) const;
	void setBuildingCommerceChange(BuildingClassTypes eBuildingClass, CommerceTypes eCommerce, int iChange);
	void changeBuildingCommerceChange(BuildingClassTypes eBuildingClass, CommerceTypes eCommerce, int iChange);
	int getBuildingHappyChange(BuildingClassTypes eBuildingClass) const;
	void setBuildingHappyChange(BuildingClassTypes eBuildingClass, int iChange);
	int getBuildingHealthChange(BuildingClassTypes eBuildingClass) const;
	void setBuildingHealthChange(BuildingClassTypes eBuildingClass, int iChange);

	DllExport PlayerTypes getLiberationPlayer(bool bConquest) const;
	void liberate(bool bConquest);

	void changeNoBonusCount(BonusTypes eBonus, int iChange);
	int getNoBonusCount(BonusTypes eBonus) const;
	bool isNoBonus(BonusTypes eBonus) const;

	void read(FDataStreamBase* pStream);
	void write(FDataStreamBase* pStream);

	virtual void AI_init() = 0;
	virtual void AI_reset() = 0;
	virtual void AI_doTurn() = 0;
	virtual void AI_assignWorkingPlots() = 0;
	virtual void AI_updateAssignWork() = 0;
	virtual bool AI_avoidGrowth() = 0;
	virtual int AI_specialistValue(SpecialistTypes eSpecialist, bool bAvoidGrowth, bool bRemove) = 0;
	virtual void AI_chooseProduction() = 0;
	virtual UnitTypes AI_bestUnit(bool bAsync = false, AdvisorTypes eIgnoreAdvisor = NO_ADVISOR, UnitAITypes* peBestUnitAI = NULL) = 0;
	virtual UnitTypes AI_bestUnitAI(UnitAITypes eUnitAI, bool bAsync = false, AdvisorTypes eIgnoreAdvisor = NO_ADVISOR) = 0;
	virtual BuildingTypes AI_bestBuilding(int iFocusFlags = 0, int iMaxTurns = MAX_INT, bool bAsync = false, AdvisorTypes eIgnoreAdvisor = NO_ADVISOR) = 0;
	virtual int AI_buildingValue(BuildingTypes eBuilding, int iFocusFlags = 0) = 0;
	virtual int AI_projectValue(ProjectTypes eProject) = 0;
	virtual int AI_neededSeaWorkers() = 0;
	virtual bool AI_isDefended(int iExtra = 0) = 0;
	virtual bool AI_isAirDefended(int iExtra = 0) = 0;
	virtual bool AI_isDanger() = 0;
	virtual int AI_neededDefenders() = 0;
	virtual int AI_neededAirDefenders() = 0;
	virtual int AI_minDefenders() = 0;
	virtual bool AI_isEmphasizeAvoidGrowth() = 0;
	virtual bool AI_isAssignWorkDirty() = 0;
	virtual CvCity* AI_getRouteToCity() const = 0;
	virtual void AI_setAssignWorkDirty(bool bNewValue) = 0;
	virtual bool AI_isChooseProductionDirty() = 0;
	virtual void AI_setChooseProductionDirty(bool bNewValue) = 0;
	virtual bool AI_isEmphasize(EmphasizeTypes eIndex) = 0;
	virtual void AI_setEmphasize(EmphasizeTypes eIndex, bool bNewValue) = 0;
	virtual int AI_getBestBuildValue(int iIndex) = 0;
	virtual int AI_totalBestBuildValue(CvArea* pArea) = 0;
	virtual int AI_countBestBuilds(CvArea* pArea) = 0;
	virtual BuildTypes AI_getBestBuild(int iIndex) = 0;
	virtual void AI_updateBestBuild() = 0;
	virtual int AI_cityValue() const = 0;
	virtual int AI_clearFeatureValue(int iIndex) = 0;

	virtual int AI_calculateCulturePressure(bool bGreatWork = false) = 0;
	virtual int AI_calculateWaterWorldPercent() = 0;
	virtual int AI_countNumBonuses(BonusTypes eBonus, bool bIncludeOurs, bool bIncludeNeutral, int iOtherCultureThreshold, bool bLand = true, bool bWater = true) = 0;
	virtual int AI_yieldMultiplier(YieldTypes eYield) = 0;
	virtual int AI_playerCloseness(PlayerTypes eIndex, int iMaxDistance = 7) = 0;
	virtual int AI_cityThreat(bool bDangerPercent = false) = 0;
	virtual BuildingTypes AI_bestAdvancedStartBuilding(int iPass) = 0;

	virtual void AI_tryToWorkPlot(CvPlot* pPlot) = 0;

	virtual int AI_getWorkersHave() = 0;
	virtual int AI_getWorkersNeeded() = 0;
	virtual void AI_changeWorkersHave(int iChange) = 0;

	bool hasShrine(ReligionTypes eReligion);
	void processVoteSourceBonus(VoteSourceTypes eVoteSource, bool bActive);

	void invalidatePopulationRankCache();
	void invalidateYieldRankCache(YieldTypes eYield = NO_YIELD);
	void invalidateCommerceRankCache(CommerceTypes eCommerce = NO_COMMERCE);

	int getBestYieldAvailable(YieldTypes eYield) const;

protected:

	int m_iID;
	int m_iX;
	int m_iY;
	int m_iRallyX;
	int m_iRallyY;
	int m_iGameTurnFounded;
	int m_iGameTurnAcquired;
	int m_iPopulation;
	int m_iHighestPopulation;
	int m_iWorkingPopulation;
	int m_iSpecialistPopulation;
	int m_iNumGreatPeople;
	int m_iBaseGreatPeopleRate;
	int m_iGreatPeopleRateModifier;
	int m_iGreatPeopleProgress;
	int m_iNumWorldWonders;
	int m_iNumTeamWonders;
	int m_iNumNationalWonders;
	int m_iNumBuildings;
	int m_iGovernmentCenterCount;
	int m_iMaintenance;
	int m_iMaintenanceModifier;
	int m_iWarWearinessModifier;
	int m_iHurryAngerModifier;
	int m_iHealRate;
	int m_iEspionageHealthCounter;
	int m_iEspionageHappinessCounter;
	int m_iFreshWaterGoodHealth;
	int m_iFreshWaterBadHealth;
	int m_iFeatureGoodHealth;
	int m_iFeatureBadHealth;
	int m_iBuildingGoodHealth;
	int m_iBuildingBadHealth;
	int m_iPowerGoodHealth;
	int m_iPowerBadHealth;
	int m_iBonusGoodHealth;
	int m_iBonusBadHealth;
	int m_iHurryAngerTimer;
	int m_iConscriptAngerTimer;
	int m_iDefyResolutionAngerTimer;
	int m_iHappinessTimer;
	int m_iMilitaryHappinessUnits;
	int m_iBuildingGoodHappiness;
	int m_iBuildingBadHappiness;
	int m_iExtraBuildingGoodHappiness;
	int m_iExtraBuildingBadHappiness;
	int m_iExtraBuildingGoodHealth;
	int m_iExtraBuildingBadHealth;
	int m_iFeatureGoodHappiness;
	int m_iFeatureBadHappiness;
	int m_iBonusGoodHappiness;
	int m_iBonusBadHappiness;
	int m_iReligionGoodHappiness;
	int m_iReligionBadHappiness;
	int m_iExtraHappiness;
	int m_iExtraHealth;
	int m_iNoUnhappinessCount;
	int m_iNoUnhealthyPopulationCount;
	int m_iBuildingOnlyHealthyCount;
	int m_iFood;
	int m_iFoodKept;
	int m_iMaxFoodKeptPercent;
	int m_iOverflowProduction;
	int m_iFeatureProduction;
	int m_iMilitaryProductionModifier;
	int m_iSpaceProductionModifier;
	int m_iExtraTradeRoutes;
	int m_iTradeRouteModifier;
	int m_iForeignTradeRouteModifier;
	int m_iBuildingDefense;
	int m_iBuildingBombardDefense;
	int m_iFreeExperience;
	int m_iCurrAirlift;
	int m_iMaxAirlift;
	int m_iAirModifier;
	int m_iAirUnitCapacity;
	int m_iNukeModifier;
	int m_iFreeSpecialist;
	int m_iPowerCount;
	int m_iDirtyPowerCount;
	int m_iDefenseDamage;
	int m_iLastDefenseDamage;
	int m_iOccupationTimer;
	int m_iCultureUpdateTimer;
	int m_iCitySizeBoost;
	int m_iSpecialistFreeExperience;
	int m_iEspionageDefenseModifier;

	bool m_bNeverLost;
	bool m_bBombarded;
	bool m_bDrafted;
	bool m_bAirliftTargeted;
	bool m_bWeLoveTheKingDay;
	bool m_bCitizensAutomated;
	bool m_bProductionAutomated;
	bool m_bWallOverride;
	bool m_bInfoDirty;
	bool m_bLayoutDirty;
	bool m_bPlundered;

	PlayerTypes m_eOwner;
	PlayerTypes m_ePreviousOwner;
	PlayerTypes m_eOriginalOwner;
	CultureLevelTypes m_eCultureLevel;

	int* m_aiSeaPlotYield;
	int* m_aiRiverPlotYield;
	int* m_aiBaseYieldRate;
	int* m_aiYieldRateModifier;
	int* m_aiPowerYieldRateModifier;
	int* m_aiBonusYieldRateModifier;
	int* m_aiTradeYield;
	int* m_aiCorporationYield;
	int* m_aiExtraSpecialistYield;
	int* m_aiCommerceRate;
	int* m_aiProductionToCommerceModifier;
	int* m_aiBuildingCommerce;
	int* m_aiSpecialistCommerce;
	int* m_aiReligionCommerce;
	int* m_aiCorporationCommerce;
	int* m_aiCommerceRateModifier;
	int* m_aiCommerceHappinessPer;
	int* m_aiDomainFreeExperience;
	int* m_aiDomainProductionModifier;
	int* m_aiCulture;
	int* m_aiNumRevolts;

	bool* m_abEverOwned;
	bool* m_abTradeRoute;
	bool* m_abRevealed;
	bool* m_abEspionageVisibility;

	CvWString m_szName;
	CvString m_szScriptData;

	int* m_paiNoBonus;
	int* m_paiFreeBonus;
	int* m_paiNumBonuses;
	int* m_paiNumCorpProducedBonuses;
	int* m_paiProjectProduction;
	int* m_paiBuildingProduction;
	int* m_paiBuildingProductionTime;
	int* m_paiBuildingOriginalOwner;
	int* m_paiBuildingOriginalTime;
	int* m_paiUnitProduction;
	int* m_paiUnitProductionTime;
	int* m_paiGreatPeopleUnitRate;
	int* m_paiGreatPeopleUnitProgress;
	int* m_paiSpecialistCount;
	int* m_paiMaxSpecialistCount;
	int* m_paiForceSpecialistCount;
	int* m_paiFreeSpecialistCount;
	int* m_paiImprovementFreeSpecialists;
	int* m_paiReligionInfluence;
	int* m_paiStateReligionHappiness;
	int* m_paiUnitCombatFreeExperience;
	int* m_paiFreePromotionCount;
	int* m_paiNumRealBuilding;
	int* m_paiNumFreeBuilding;

	bool* m_pabWorkingPlot;
	bool* m_pabHasReligion;
	bool* m_pabHasCorporation;

	IDInfo* m_paTradeCities;

	mutable CLinkList<OrderData> m_orderQueue;

	std::vector< std::pair < float, float> > m_kWallOverridePoints;

	std::vector<EventTypes> m_aEventsOccured;
	std::vector<BuildingYieldChange> m_aBuildingYieldChange;
	std::vector<BuildingCommerceChange> m_aBuildingCommerceChange;
	BuildingChangeArray m_aBuildingHappyChange;
	BuildingChangeArray m_aBuildingHealthChange;

	// CACHE: cache frequently used values
	mutable int	m_iPopulationRank;
	mutable bool m_bPopulationRankValid;
	int*	m_aiBaseYieldRank;
	bool*	m_abBaseYieldRankValid;
	int*	m_aiYieldRank;
	bool*	m_abYieldRankValid;
	int*	m_aiCommerceRank;
	bool*	m_abCommerceRankValid;

	void doGrowth();
	void doCulture();
	void doPlotCulture(bool bUpdate, PlayerTypes ePlayer, int iCultureRate);
	void doProduction(bool bAllowNoProduction);
	void doDecay();
	void doReligion();
	void doGreatPeople();
	void doMeltdown();
	bool doCheckProduction();

	int getExtraProductionDifference(int iExtra, UnitTypes eUnit) const;
	int getExtraProductionDifference(int iExtra, BuildingTypes eBuilding) const;
	int getExtraProductionDifference(int iExtra, ProjectTypes eProject) const;
	int getExtraProductionDifference(int iExtra, int iModifier) const;
	int getHurryCostModifier(UnitTypes eUnit, bool bIgnoreNew) const;
	int getHurryCostModifier(BuildingTypes eBuilding, bool bIgnoreNew) const;
	int getHurryCostModifier(int iBaseModifier, int iProduction, bool bIgnoreNew) const;
	int getHurryCost(bool bExtra, UnitTypes eUnit, bool bIgnoreNew) const;
	int getHurryCost(bool bExtra, BuildingTypes eBuilding, bool bIgnoreNew) const;
	int getHurryCost(bool bExtra, int iProductionLeft, int iHurryModifier, int iModifier) const;
	int getHurryPopulation(HurryTypes eHurry, int iHurryCost) const;
	int getHurryGold(HurryTypes eHurry, int iHurryCost) const;
	bool canHurryUnit(HurryTypes eHurry, UnitTypes eUnit, bool bIgnoreNew) const;
	bool canHurryBuilding(HurryTypes eHurry, BuildingTypes eBuilding, bool bIgnoreNew) const;

	virtual bool AI_addBestCitizen(bool bWorkers, bool bSpecialists, int* piBestPlot = NULL, SpecialistTypes* peBestSpecialist = NULL) = 0;
	virtual bool AI_removeWorstCitizen(SpecialistTypes eIgnoreSpecialist = NO_SPECIALIST) = 0;
};

#endif
