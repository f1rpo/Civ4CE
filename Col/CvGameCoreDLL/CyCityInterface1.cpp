#include "CvGameCoreDLL.h"
#include "CyCity.h"
#include "CyUnit.h"
#include "CyPlot.h"
#include "CyArea.h"
#include "CvInfos.h"

//# include <boost/python/manage_new_object.hpp>
//# include <boost/python/return_value_policy.hpp>

//
// published python interface for CyCity
//

void CyCityPythonInterface1(python::class_<CyCity>& x)
{
	OutputDebugString("Python Extension Module - CyCityPythonInterface1\n");

	x
		.def("isNone", &CyCity::isNone, "void () - is the instance valid?")
		.def("kill", &CyCity::kill, "void () - kill the city")
		.def("doTask", &CyCity::doTask, "void (int eTaskTypes, int iData1, int iData2, bool bOption) - Enacts the TaskType passed")
		.def("chooseProduction", &CyCity::chooseProduction, "void (int /*UnitTypes*/ eTrainUnit, int /*BuildingTypes*/ eConstructBuilding, bool bFinish, bool bFront) - Chooses production for a city")

		.def("createGreatGeneral", &CyCity::createGreatGeneral, "void (int /*UnitTypes*/ eGreatGeneralUnit, bool bIncrementThreshold) - Creates a great person in this city and whether it should increment the threshold to the next level")

		.def("getCityPlotIndex", &CyCity::getCityPlotIndex, "int (CyPlot* pPlot)")
		.def("getCityIndexPlot", &CyCity::getCityIndexPlot, python::return_value_policy<python::manage_new_object>(), "CyPlot* (int iIndex)")
		.def("canWork", &CyCity::canWork, "bool (CyPlot*) - can the city work the plot?")
		.def("clearWorkingOverride", &CyCity::clearWorkingOverride, "void (int iIndex)")
		.def("countNumImprovedPlots", &CyCity::countNumImprovedPlots, "int ()")
		.def("countNumWaterPlots", &CyCity::countNumWaterPlots, "int ()")
		.def("countNumRiverPlots", &CyCity::countNumRiverPlots, "int ()")

		.def("findPopulationRank", &CyCity::findPopulationRank, "int ()")
		.def("findBaseYieldRateRank", &CyCity::findBaseYieldRateRank, "int (int /*YieldTypes*/ eYield)")
		.def("findYieldRateRank", &CyCity::findYieldRateRank, "int (int /*YieldTypes*/ eYield)")

		.def("allUpgradesAvailable", &CyCity::allUpgradesAvailable, "int UnitTypes (int eUnit, int iUpgradeCount)")
		.def("canTrain", &CyCity::canTrain, "bool (int eUnit, bool bContinue, bool bTestVisible)")
		.def("canConstruct", &CyCity::canConstruct, "bool (int eBuilding, bool bContinue, bool bTestVisible, bool bIgnoreCost)")
		.def("canConvince", &CyCity::canConvince, "bool (int eFatherPointType, bool bContinue, bool bTestVisible)")
		.def("getFoodTurnsLeft", &CyCity::getFoodTurnsLeft, "int () - how many food turns remain?")
		.def("isProduction", &CyCity::isProduction, "bool () - is city producing?")
		.def("isProductionUnit", &CyCity::isProductionUnit, "bool () - is city training a unit?")
		.def("isProductionBuilding", &CyCity::isProductionBuilding, "bool () - is city constructing a building?")

		.def("canContinueProduction", &CyCity::canContinueProduction, "bool (OrderData order)")
		.def("getProductionExperience", &CyCity::getProductionExperience, "int (int /*UnitTypes*/ eUnit)")
		.def("addProductionExperience", &CyCity::addProductionExperience, "void (CyUnit* pUnit)")

		.def("getProductionUnit", &CyCity::getProductionUnit, "UnitID () - ID for unit that is being trained")
		.def("getProductionUnitAI", &CyCity::getProductionUnitAI, "int eUnitAIType ()")
		.def("getProductionBuilding", &CyCity::getProductionBuilding, "BuildingID () - ID for building that is under construction")
		.def("getProductionName", &CyCity::getProductionName, "str () - description of item that the city is working on")
		.def("getGeneralProductionTurnsLeft", &CyCity::getGeneralProductionTurnsLeft, "int - # of production turns left for the top order node item in a city...")
		.def("getProductionNameKey", &CyCity::getProductionNameKey, "str () - description of item that the city is working on")
		.def("getFirstUnitOrder", &CyCity::getFirstUnitOrder, "int (int /*UnitTypes*/ eUnit)")
		.def("getFirstBuildingOrder", &CyCity::getFirstBuildingOrder, "int (int /*BuildingTypes*/ eBuilding)")
		.def("getProduction", &CyCity::getProduction, "int () - returns the current production towards whatever is top of this city's OrderQueue")
		.def("getProductionNeeded", &CyCity::getProductionNeeded, "int (int /*YieldTypes*/) - # of production needed to complete construction")
		.def("getProductionTurnsLeft", &CyCity::getProductionTurnsLeft, "int () - # of turns remaining until item is completed")
		.def("getUnitProductionTurnsLeft", &CyCity::getUnitProductionTurnsLeft, "int (UnitID, int iNum) - # of turns remaining to complete UnitID")
		.def("getBuildingProductionTurnsLeft", &CyCity::getBuildingProductionTurnsLeft, "int (BuildingID, int iNum) - # of turns remaining to complete UnitID")
		.def("setProduction", &CyCity::setProduction, "void (int iNewValue)")
		.def("changeProduction", &CyCity::changeProduction, "void (int iChange)")
		.def("getProductionModifier", &CyCity::getProductionModifier, "int () - multiplier (if any) for item being produced")
		.def("getCurrentProductionDifference", &CyCity::getCurrentProductionDifference, "int (bool bOverflow)")
		.def("getUnitProductionModifier", &CyCity::getUnitProductionModifier, "int (UnitID) - production multiplier for UnitID")
		.def("getBuildingProductionModifier", &CyCity::getBuildingProductionModifier, "int (BuildingID) - production multiplier for BuildingID")

		.def("canHurry", &CyCity::canHurry, "bool (HurryTypes eHurry, bool bTestVisible = 0) - can player eHurry in this city?")
		.def("hurry", &CyCity::hurry, "void (HurryTypes eHurry) - forces the city to rush production using eHurry")
		.def("getHandicapType", &CyCity::getHandicapType, "HandicapType () - owners difficulty level")
		.def("getCivilizationType", &CyCity::getCivilizationType, "CivilizationID () - owners CivilizationID")
		.def("getPersonalityType", &CyCity::getPersonalityType, "int /*LeaderHeadTypes*/ ()")
		.def("getArtStyleType", &CyCity::getArtStyleType, "int /*ArtStyleTypes*/ ()")
		.def("getCitySizeType", &CyCity::getCitySizeType, "int /*CitySizeTypes*/ ()")

		.def("hasTrait", &CyCity::hasTrait, "bool (TraitID) - does owner have TraitID?")
		.def("isHuman", &CyCity::isHuman, "bool () - is owner human?")
		.def("isVisible", &CyCity::isVisible, "bool (int /*TeamTypes*/ eTeam, bool bDebug)")

		.def("isCapital", &CyCity::isCapital, "bool () - is city the owners capital?")
		.def("isCoastal", &CyCity::isCoastal, "bool (int) - is the city on the coast?")
		.def("isDisorder", &CyCity::isDisorder, "bool () - is the city in disorder?")
		.def("extraPopulation", &CyCity::extraPopulation, "int () - # of extra/available citizens")
		.def("foodConsumption", &CyCity::foodConsumption, "int (int iExtra)")
		.def("foodDifference", &CyCity::foodDifference, "int () - result of getYieldRate(Food) - foodConsumption()")
		.def("growthThreshold", &CyCity::growthThreshold, "int () - value needed for growth")
		.def("productionLeft", &CyCity::productionLeft, "int ()")
		.def("hurryCost", &CyCity::hurryCost, "int (bool bExtra, bool bIgnoreNew)")
		.def("hurryGold", &CyCity::hurryGold, "int (HurryID) - total value of gold when hurrying")
		.def("hurryPopulation", &CyCity::hurryPopulation, "int (HurryID) - value of each pop when hurrying")
		.def("hurryProduction", &CyCity::hurryProduction, "int (HurryID)")
		.def("maxHurryPopulation", &CyCity::maxHurryPopulation, "int ()")

		.def("cultureDistance", &CyCity::cultureDistance, "int (iDX, iDY) - culture distance")
		.def("isHasBuilding", &CyCity::isHasBuilding, "bool (int iBuildingID)")
		.def("getProfessionOutput", &CyCity::getProfessionOutput, "int (int /*ProfessionTypes*/ iProfession, CyUnit* pUnit)")

		.def("getID", &CyCity::getID, "int () - index ID # for the city - use with pPlayer.getCity(ID) to obtain city instance")
		.def("getX", &CyCity::getX, "int () - X coordinate for the cities plot")
		.def("getY", &CyCity::getY, "int () - Y coordinate for the cities plot")
		.def("at", &CyCity::at, "bool (iX, iY) - is the city at (iX, iY) ?")
		.def("atPlot", &CyCity::atPlot, "bool (CyPlot) - is pPlot the cities plot?")
		.def("plot", &CyCity::plot, python::return_value_policy<python::manage_new_object>(), "CyPlot () - returns cities plot instance")
		.def("area", &CyCity::area, python::return_value_policy<python::manage_new_object>(), "CyArea() () - returns CyArea instance for location of city")
		.def("waterArea", &CyCity::waterArea, python::return_value_policy<python::manage_new_object>(), "CyArea* ()")
		.def("getRallyPlot", &CyCity::getRallyPlot, python::return_value_policy<python::manage_new_object>(), "CyPlot () - returns city's rally plot instance")
		.def("getGameTurnFounded", &CyCity::getGameTurnFounded, "int () - GameTurn the city was founded")

		.def("getGameTurnAcquired", &CyCity::getGameTurnAcquired, "int ()")
		.def("getPopulation", &CyCity::getPopulation, "int () - total city population")
		.def("setPopulation", &CyCity::setPopulation, "void (int iNewValue) - sets the city population to iNewValue")
		.def("changePopulation", &CyCity::changePopulation, "void (int iChange) - adjusts the city population by iChange")
		.def("getRealPopulation", &CyCity::getRealPopulation, "int () - total city population in \"real\" numbers")
		.def("getHighestPopulation", &CyCity::getHighestPopulation, "int () ")
		.def("setHighestPopulation", &CyCity::setHighestPopulation, "void (iNewValue)")
		.def("isWorksWater", &CyCity::isWorksWater, "bool () - is city able to work water plots?")
		.def("changeHealRate", &CyCity::changeHealRate, "void (int iChange) - changes the heal rate of this city to iChange")

		.def("getFood", &CyCity::getFood, "int () - stored food")
		.def("setFood", &CyCity::setFood, "void (iNewValue) - set stored food to iNewValue")
		.def("changeFood", &CyCity::changeFood, "void (iChange) - adjust stored food by iChange")
		.def("getFoodKept", &CyCity::getFoodKept, "int ()")
		.def("getMaxFoodKeptPercent", &CyCity::getMaxFoodKeptPercent, "int ()")
		.def("getOverflowProduction", &CyCity::getOverflowProduction, "int () - value of overflow production")
		.def("setOverflowProduction", &CyCity::setOverflowProduction, "void (iNewValue) - set overflow production to iNewValue")
		.def("getMilitaryProductionModifier", &CyCity::getMilitaryProductionModifier, "int () - value of adjustments to military production")
		.def("getBuildingDefense", &CyCity::getBuildingDefense, "int () - building defense")
		.def("getBuildingBombardDefense", &CyCity::getBuildingBombardDefense, "int () - building defense")
		.def("getFreeExperience", &CyCity::getFreeExperience, "int () - # of free experience newly trained units receive")
		.def("getDefenseDamage", &CyCity::getDefenseDamage, "int () - value of damage city defenses can receive")
		.def("changeDefenseDamage", &CyCity::changeDefenseDamage, "void (iChange) - adjust damage value by iChange")
		.def("isBombardable", &CyCity::isBombardable, "bool (CyUnit* pUnit)")
		.def("getTotalDefense", &CyCity::getTotalDefense, "int ()")
		.def("getDefenseModifier", &CyCity::getDefenseModifier, "int ()")

		.def("getOccupationTimer", &CyCity::getOccupationTimer, "int () - total # of turns remaining on occupation timer")
		.def("isOccupation", &CyCity::isOccupation, "bool () - is the city under occupation?")
		.def("setOccupationTimer", &CyCity::setOccupationTimer, "void (iNewValue) - set the Occupation Timer to iNewValue")
		.def("changeOccupationTimer", &CyCity::changeOccupationTimer, "void (iChange) - adjusts the Occupation Timer by iChange")
		.def("getCultureUpdateTimer", &CyCity::getCultureUpdateTimer, "int () - Culture Update Timer")
		.def("changeCultureUpdateTimer", &CyCity::changeCultureUpdateTimer, "void (iChange) - adjusts the Culture Update Timer by iChange")
		.def("isNeverLost", &CyCity::isNeverLost, "bool ()")
		.def("setNeverLost", &CyCity::setNeverLost, "void (iNewValue)")
		.def("isBombarded", &CyCity::isBombarded, "bool ()")
		.def("setBombarded", &CyCity::setBombarded, "void (iNewValue)")
		.def("setAllCitizensAutomated", &CyCity::setAllCitizensAutomated, "void (bool bAutomated)")
		.def("isProductionAutomated", &CyCity::isProductionAutomated, "bool () - is production under automation?")
		.def("setProductionAutomated", &CyCity::setProductionAutomated, "void (bool bNewValue) - set city production automation to bNewValue")
		.def("isWallOverride", &CyCity::isWallOverride, "bool isWallOverride()")
		.def("setWallOverride", &CyCity::setWallOverride, "setWallOverride(bool bOverride)")
		.def("setCitySizeBoost", &CyCity::setCitySizeBoost, "setCitySizeBoost(int iBoost)")
		.def("getOwner", &CyCity::getOwner, "int /*PlayerTypes*/ ()")
		.def("getTeam", &CyCity::getTeam, "int /*TeamTypes*/ ()")
		.def("getPreviousOwner", &CyCity::getPreviousOwner, "int /*PlayerTypes*/ ()")
		.def("getOriginalOwner", &CyCity::getOriginalOwner, "int /*PlayerTypes*/ ()")
		.def("getCultureLevel", &CyCity::getCultureLevel, "int /*CultureLevelTypes*/ ()")
		.def("getCultureThreshold", &CyCity::getCultureThreshold)
		.def("getSeaPlotYield", &CyCity::getSeaPlotYield, "int (int /*YieldTypes*/) - total YieldType for water plots")
		.def("getRiverPlotYield", &CyCity::getRiverPlotYield, "int (int /*YieldTypes*/) - total YieldType for river plots")

		.def("getBaseRawYieldProduced", &CyCity::getBaseRawYieldProduced, "int (int /*YieldTypes*/) - Yield Bonus produced in all cities")
		.def("getRawYieldProduced", &CyCity::getRawYieldProduced, "int (int /*YieldTypes*/) - Yield added to the city each turn")
		.def("getRawYieldConsumed", &CyCity::getRawYieldConsumed, "int (int /*YieldTypes*/) - Yield remoced from the city each turn")

		.def("getBaseYieldRateModifier", &CyCity::getBaseYieldRateModifier)
		.def("getYieldRate", &CyCity::getYieldRate, "int (int /*YieldTypes*/) - total value of YieldType")
		.def("getCultureRate", &CyCity::getCultureRate, "int ()")
		.def("getYieldRateModifier", &CyCity::getYieldRateModifier, "int (int /*YieldTypes*/) - yield rate modifier for YieldType")

		.def("getDomainFreeExperience", &CyCity::getDomainFreeExperience, "int (int /*DomainTypes*/)")
		.def("getDomainProductionModifier", &CyCity::getDomainProductionModifier, "int (int /*DomainTypes*/)")
		.def("getCulture", &CyCity::getCulture, "int /*PlayerTypes*/ ()")
		.def("countTotalCulture", &CyCity::countTotalCulture, "int ()")
		.def("findHighestCulture", &CyCity::findHighestCulture, "PlayerTypes ()")
		.def("calculateCulturePercent", &CyCity::calculateCulturePercent, "int (int eIndex)")
		.def("calculateTeamCulturePercent", &CyCity::calculateTeamCulturePercent, "int /*TeamTypes*/ ()")
		.def("setCulture", &CyCity::setCulture, "void (int PlayerTypes eIndex`, bool bPlots)")
		.def("changeCulture", &CyCity::changeCulture, "void (int PlayerTypes eIndex, int iChange, bool bPlots)")

		.def("getYieldStored", &CyCity::getYieldStored, "int /*YieldTypes*/ ()")
		.def("setYieldStored", &CyCity::setYieldStored, "void (int YieldTypes eIndex, int)")
		.def("changeYieldStored", &CyCity::changeYieldStored, "void (int YieldTypes, int)")
		.def("getYieldRushed", &CyCity::getYieldRushed, "int /*YieldTypes*/ ()")
		.def("calculateNetYield", &CyCity::calculateNetYield, "int (int /*YieldTypes*/)")
		.def("calculateActualYieldProduced", &CyCity::calculateActualYieldProduced, "int (int /*YieldTypes*/)")
		.def("calculateActualYieldConsumed", &CyCity::calculateActualYieldConsumed, "int (int /*YieldTypes*/)")

		.def("isEverOwned", &CyCity::isEverOwned, "bool ()")

		.def("isRevealed", &CyCity::isRevealed, "bool (int /*TeamTypes*/ eIndex, bool bDebug)")
		.def("setRevealed", &CyCity::setRevealed, "void (int /*TeamTypes*/ eIndex, bool bNewValue)")
		.def("getName", &CyCity::getName, "string () - city name")
		.def("getNameForm", &CyCity::getNameForm, "string () - city name")
		.def("getNameKey", &CyCity::getNameKey, "string () - city name")
		.def("setName", &CyCity::setName, "void (TCHAR szNewValue, bool bFound) - sets the name to szNewValue")
		.def("getBuildingProduction", &CyCity::getBuildingProduction, "int (BuildingID) - current production towards BuildingID")
		.def("setBuildingProduction", &CyCity::setBuildingProduction, "void (BuildingID, iNewValue) - set progress towards BuildingID as iNewValue")
		.def("changeBuildingProduction", &CyCity::changeBuildingProduction, "void (BuildingID, iChange) - adjusts progress towards BuildingID by iChange")
		.def("getBuildingProductionTime", &CyCity::getBuildingProductionTime, "int (int eIndex)")
		.def("setBuildingProductionTime", &CyCity::setBuildingProductionTime, "int (int eIndex, int iNewValue)")
		.def("changeBuildingProductionTime", &CyCity::changeBuildingProductionTime, "int (int eIndex, int iChange)")
		.def("getBuildingOriginalOwner", &CyCity::getBuildingOriginalOwner, "int (BuildingType) - index of original building owner")
		.def("getBuildingOriginalTime", &CyCity::getBuildingOriginalTime, "int (BuildingType) - original build date")
		.def("getUnitProduction", &CyCity::getUnitProduction, "int (UnitID) - gets current production towards UnitID")
		.def("setUnitProduction", &CyCity::setUnitProduction, "void (UnitID, iNewValue) - sets production towards UnitID as iNewValue")
		.def("changeUnitProduction", &CyCity::changeUnitProduction, "void (UnitID, iChange) - adjusts production towards UnitID by iChange")

		.def("getUnitCombatFreeExperience", &CyCity::getUnitCombatFreeExperience, "int (int /*UnitCombatTypes*/ eIndex)")
		.def("getFreePromotionCount", &CyCity::getFreePromotionCount, "int (int /*PromotionTypes*/ eIndex)")
		.def("isFreePromotion", &CyCity::isFreePromotion, "bool (int /*PromotionTypes*/ eIndex)")

		.def("isHasRealBuilding", &CyCity::isHasRealBuilding, "bool (BuildingID)")
		.def("setHasRealBuilding", &CyCity::setHasRealBuilding, "(BuildingID, bool)")
		.def("isHasFreeBuilding", &CyCity::isHasFreeBuilding, "bool (BuildingID)")

		.def("clearOrderQueue", &CyCity::clearOrderQueue, "void ()")
		.def("pushOrder", &CyCity::pushOrder, "void (OrderTypes eOrder, int iData1, int iData2, bool bSave, bool bPop, bool bAppend, bool bForce)")
		.def("popOrder", &CyCity::popOrder, "int (int iNum, bool bFinish, bool bChoose)")
		.def("getOrderQueueLength", &CyCity::getOrderQueueLength, "void ()")
		.def("getOrderFromQueue", &CyCity::getOrderFromQueue, python::return_value_policy<python::manage_new_object>(), "OrderData* (int iIndex)")

		.def("setWallOverridePoints", &CyCity::setWallOverridePoints, "setWallOverridePoints(const python::tuple& kPoints)")
		.def("getWallOverridePoints", &CyCity::getWallOverridePoints, "python::tuple getWallOverridePoints()")

		.def("AI_avoidGrowth", &CyCity::AI_avoidGrowth, "bool ()")
		.def("AI_isEmphasize", &CyCity::AI_isEmphasize, "bool (int iEmphasizeType)")
		.def("AI_getEmphasizeYieldCount", &CyCity::AI_getEmphasizeYieldCount, "int (int iEmphasizeType)")
		.def("AI_countBestBuilds", &CyCity::AI_countBestBuilds, "int (CyArea* pArea)")
		.def("AI_cityValue", &CyCity::AI_cityValue, "int ()")

		.def("getScriptData", &CyCity::getScriptData, "str () - Get stored custom data (via pickle)")
		.def("setScriptData", &CyCity::setScriptData, "void (str) - Set stored custom data (via pickle)")

		.def("getBuildingYieldChange", &CyCity::getBuildingYieldChange, "int (int /*BuildingClassTypes*/ eBuildingClass, int /*YieldTypes*/ eYield)")
		.def("setBuildingYieldChange", &CyCity::setBuildingYieldChange, "void (int /*BuildingClassTypes*/ eBuildingClass, int /*YieldTypes*/ eYield, int iChange)")

		.def("getLiberationPlayer", &CyCity::getLiberationPlayer, "int ()")
		.def("liberate", &CyCity::liberate, "void ()")

		.def("isScoutVisited", &CyCity::isScoutVisited, "bool isScoutVisited(int /*TeamTypes*/ eTeam)")
		.def("getMaxYieldCapacity", &CyCity::getMaxYieldCapacity, "int ()")

		.def("getPopulationUnitByIndex", &CyCity::getPopulationUnitByIndex, python::return_value_policy<python::manage_new_object>(), "CyUnit* getPopulationUnitByIndex(int)")
		.def("getPopulationUnitById", &CyCity::getPopulationUnitById, python::return_value_policy<python::manage_new_object>(), "CyUnit* getPopulationUnitById(int)")
		.def("getPopulationUnitIndex", &CyCity::getPopulationUnitIndex, "int (CyUnit* pUnit)")
		.def("getTeachUnitClass", &CyCity::getTeachUnitClass, "int /*UnitClass*/()")
		.def("getRebelPercent", &CyCity::getRebelPercent, "int ()")
		.def("getRebelSentiment", &CyCity::getRebelSentiment, "int ()")
		.def("setRebelSentiment", &CyCity::setRebelSentiment, "void (int)")

		.def("getUnitWorkingPlot", &CyCity::getUnitWorkingPlot, python::return_value_policy<python::manage_new_object>(), "CyUnit* getUnitWorkingPlot(int /*PlotIndex*/)")
		.def("addPopulationUnit", &CyCity::addPopulationUnit, "void addPopulationUnit(CyUnit* pUnit, int /*ProfessionTypes*/ eProfession)")
		.def("removePopulationUnit", &CyCity::removePopulationUnit, "removePopulationUnit(CyUnit* pUnit, bool bDelete, int /*ProfessionTypes*/ eProfession)")

		.def("isExport", &CyCity::isExport, "bool isExport(int /*YieldTypes*/ eYield)")
		.def("addExport", &CyCity::addExport, "void addExport(int /*YieldTypes*/ eYield)")
		.def("removeExport", &CyCity::removeExport, "void removeExport(int /*YieldTypes*/ eYield)")
		.def("isImport", &CyCity::isImport, "bool isImport(int /*YieldTypes*/ eYield) const")
		.def("addImport", &CyCity::addImport, "void addImport(int /*YieldTypes*/ eYield)")
		.def("removeImport", &CyCity::removeImport, "void removeImport(int /*YieldTypes*/ eYield)")
		.def("getMaintainLevel", &CyCity::getMaintainLevel, "int getMaintainLevel(int /*YieldTypes*/ eYield)")

		.def("isOrderWaitingForYield", &CyCity::isOrderWaitingForYield, "python::tuple isOrderWaitingForYield(int /*YieldTypes*/ eYield)")
		;
}
