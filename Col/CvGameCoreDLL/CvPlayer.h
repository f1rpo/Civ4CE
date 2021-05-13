#pragma once

// player.h

#ifndef CIV4_PLAYER_H
#define CIV4_PLAYER_H

#include "CvCityAI.h"
#include "CvUnitAI.h"
#include "CvSelectionGroupAI.h"
#include "CvPlotGroup.h"
#include "LinkedList.h"
#include "CvTalkingHeadMessage.h"

class CvDiploParameters;
class CvPopupInfo;
class CvEventTriggerInfo;

typedef std::list<CvTalkingHeadMessage> CvMessageQueue;
typedef std::list<CvPopupInfo*> CvPopupQueue;
typedef std::list<CvDiploParameters*> CvDiploQueue;
typedef stdext::hash_map<int, int> CvTurnScoreMap;
typedef stdext::hash_map<EventTypes, EventTriggeredData> CvEventMap;
typedef std::vector< std::pair<UnitCombatTypes, PromotionTypes> > UnitCombatPromotionArray;
typedef std::vector< std::pair<UnitClassTypes, PromotionTypes> > UnitClassPromotionArray;
typedef std::vector< std::pair<CivilizationTypes, LeaderHeadTypes> > CivLeaderArray;

class CvPlayer
{
public:
	CvPlayer();
	virtual ~CvPlayer();

	DllExport void init(PlayerTypes eID);
	DllExport void setupGraphical();
	DllExport void reset(PlayerTypes eID = NO_PLAYER, bool bConstructorCall = false);

protected:

	void uninit();

public:

	void initFreeState();
	void initFreeUnits();
	void addFreeUnitAI(UnitAITypes eUnitAI, int iCount);
	void addFreeUnit(UnitTypes eUnit, UnitAITypes eUnitAI = NO_UNITAI);

	int startingPlotRange() const;
	bool startingPlotWithinRange(CvPlot* pPlot, PlayerTypes ePlayer, int iRange, int iPass) const;
	int startingPlotDistanceFactor(CvPlot* pPlot, PlayerTypes ePlayer, int iRange) const;
	int findStartingArea() const;
	CvPlot* findStartingPlot(bool bRandomize = false);

	CvPlotGroup* initPlotGroup(CvPlot* pPlot);

	DllExport CvCity* initCity(int iX, int iY, bool bBumpUnits, bool bUpdatePlotGroups);
	void acquireCity(CvCity* pCity, bool bConquest, bool bTrade, bool bUpdatePlotGroups);
	void killCities();
	CvWString getNewCityName() const;
	void getCivilizationCityName(CvWString& szBuffer, CivilizationTypes eCivilization) const;
	bool isCityNameValid(CvWString& szName, bool bTestDestroyed = true) const;

	DllExport CvUnit* initUnit(UnitTypes eUnit, int iX, int iY, UnitAITypes eUnitAI = NO_UNITAI, DirectionTypes eFacingDirection = NO_DIRECTION);
	void disbandUnit(bool bAnnounce);
	void killUnits();

	DllExport CvSelectionGroup* cycleSelectionGroups(CvUnit* pUnit, bool bForward, bool bWorkers, bool* pbWrap);

	bool hasTrait(TraitTypes eTrait) const;
	DllExport bool isHuman() const;
	DllExport void updateHuman();
	DllExport bool isBarbarian() const;

	DllExport const wchar* getName(uint uiForm = 0) const;
	DllExport const wchar* getNameKey() const;
	DllExport const wchar* getCivilizationDescription(uint uiForm = 0) const;
	DllExport const wchar* getCivilizationDescriptionKey() const;
	DllExport const wchar* getCivilizationShortDescription(uint uiForm = 0) const;
	DllExport const wchar* getCivilizationShortDescriptionKey() const;
	DllExport const wchar* getCivilizationAdjective(uint uiForm = 0) const;
	DllExport const wchar* getCivilizationAdjectiveKey() const;
	DllExport CvWString getFlagDecal() const;
	DllExport bool isWhiteFlag() const;
	DllExport const wchar* getStateReligionName(uint uiForm = 0) const;
	DllExport const wchar* getStateReligionKey() const;
	DllExport const CvWString getBestAttackUnitName(uint uiForm = 0) const;
	DllExport const CvWString getWorstEnemyName() const;
	const wchar* getBestAttackUnitKey() const;
	DllExport ArtStyleTypes getArtStyleType() const;
	DllExport const TCHAR* getUnitButton(UnitTypes eUnit) const;

	void doTurn();
	void doTurnUnits();

	void verifyCivics();

	void updatePlotGroups();

	void updateYield();
	void updateMaintenance();
	void updatePowerHealth();
	void updateExtraBuildingHappiness();
	void updateExtraBuildingHealth();
	void updateFeatureHappiness();
	void updateReligionHappiness();
	void updateExtraSpecialistYield();
	void updateCommerce(CommerceTypes eCommerce);
	void updateCommerce();
	void updateBuildingCommerce();
	void updateReligionCommerce();
	void updateCorporation();
	void updateCityPlotYield();
	void updateCitySight(bool bIncrement, bool bUpdatePlotGroups);
	void updateTradeRoutes();
	void updatePlunder(int iChange, bool bUpdatePlotGroups);

	void updateTimers();

	DllExport bool hasReadyUnit(bool bAny = false) const;
	DllExport bool hasAutoUnit() const;
	DllExport bool hasBusyUnit() const;

	DllExport void chooseTech(int iDiscover = 0, CvWString szText = "", bool bFront = false);

	int calculateScore(bool bFinal = false, bool bVictory = false);

	int findBestFoundValue() const;

	DllExport int upgradeAllPrice(UnitTypes eUpgradeUnit, UnitTypes eFromUnit);

	int countReligionSpreadUnits(CvArea* pArea, ReligionTypes eReligion) const;
	int countCorporationSpreadUnits(CvArea* pArea, CorporationTypes eCorporation) const;
	int countNumCoastalCities() const;
	int countNumCoastalCitiesByArea(CvArea* pArea) const;
	int countTotalCulture() const;
	int countOwnedBonuses(BonusTypes eBonus) const;
	int countUnimprovedBonuses(CvArea* pArea, CvPlot* pFromPlot = NULL) const;
	int countCityFeatures(FeatureTypes eFeature) const;
	int countNumBuildings(BuildingTypes eBuilding) const;
	DllExport int countNumCitiesConnectedToCapital() const;
	int countPotentialForeignTradeCities(CvArea* pIgnoreArea = NULL) const;
	int countPotentialForeignTradeCitiesConnected() const;

	DllExport bool canContact(PlayerTypes ePlayer) const;
	void contact(PlayerTypes ePlayer);
	DllExport void handleDiploEvent(DiploEventTypes eDiploEvent, PlayerTypes ePlayer, int iData1, int iData2);
	bool canTradeWith(PlayerTypes eWhoTo) const;
	bool canReceiveTradeCity() const;
	DllExport bool canTradeItem(PlayerTypes eWhoTo, TradeData item, bool bTestDenial = false);
	DllExport DenialTypes getTradeDenial(PlayerTypes eWhoTo, TradeData item);
	bool canTradeNetworkWith(PlayerTypes ePlayer) const;
	int getNumAvailableBonuses(BonusTypes eBonus) const;
	DllExport int getNumTradeableBonuses(BonusTypes eBonus) const;
	int getNumTradeBonusImports(PlayerTypes ePlayer) const;
	bool hasBonus(BonusTypes eBonus) const;

	bool isTradingWithTeam(TeamTypes eTeam, bool bIncludeCancelable) const;
	bool canStopTradingWithTeam(TeamTypes eTeam, bool bContinueNotTrading = false) const;
	void stopTradingWithTeam(TeamTypes eTeam);
	void killAllDeals();

	void findNewCapital();
	DllExport int getNumGovernmentCenters() const;

	DllExport bool canRaze(CvCity* pCity) const;
	void raze(CvCity* pCity);
	void disband(CvCity* pCity);

	bool canReceiveGoody(CvPlot* pPlot, GoodyTypes eGoody, CvUnit* pUnit) const;
	void receiveGoody(CvPlot* pPlot, GoodyTypes eGoody, CvUnit* pUnit);
	void doGoody(CvPlot* pPlot, CvUnit* pUnit);

	DllExport bool canFound(int iX, int iY, bool bTestVisible = false) const;
	void found(int iX, int iY);

	DllExport bool canTrain(UnitTypes eUnit, bool bContinue = false, bool bTestVisible = false, bool bIgnoreCost = false) const;
	bool canConstruct(BuildingTypes eBuilding, bool bContinue = false, bool bTestVisible = false, bool bIgnoreCost = false) const;
	bool canCreate(ProjectTypes eProject, bool bContinue = false, bool bTestVisible = false) const;
	bool canMaintain(ProcessTypes eProcess, bool bContinue = false) const;
	bool isProductionMaxedUnitClass(UnitClassTypes eUnitClass) const;
	bool isProductionMaxedBuildingClass(BuildingClassTypes eBuildingClass, bool bAcquireCity = false) const;
	bool isProductionMaxedProject(ProjectTypes eProject) const;
	DllExport int getProductionNeeded(UnitTypes eUnit) const;
	DllExport int getProductionNeeded(BuildingTypes eBuilding) const;
	DllExport int getProductionNeeded(ProjectTypes eProject) const;
	int getProductionModifier(UnitTypes eUnit) const;
	int getProductionModifier(BuildingTypes eBuilding) const;
	int getProductionModifier(ProjectTypes eProject) const;

	DllExport int getBuildingClassPrereqBuilding(BuildingTypes eBuilding, BuildingClassTypes ePrereqBuildingClass, int iExtra = 0) const;
	void removeBuildingClass(BuildingClassTypes eBuildingClass);
	void processBuilding(BuildingTypes eBuilding, int iChange, CvArea* pArea);

	int getBuildCost(const CvPlot* pPlot, BuildTypes eBuild) const;
	bool canBuild(const CvPlot* pPlot, BuildTypes eBuild, bool bTestEra = false, bool bTestVisible = false) const;
	RouteTypes getBestRoute(CvPlot* pPlot = NULL) const;
	int getImprovementUpgradeRate() const;

	int calculateTotalYield(YieldTypes eYield) const;
	int calculateTotalExports(YieldTypes eYield) const;
	int calculateTotalImports(YieldTypes eYield) const;

	int calculateTotalCityHappiness() const;
	int calculateTotalCityUnhappiness() const;

	int calculateTotalCityHealthiness() const;
	int calculateTotalCityUnhealthiness() const;

	int calculateUnitCost(int& iFreeUnits, int& iFreeMilitaryUnits, int& iPaidUnits, int& iPaidMilitaryUnits, int& iBaseUnitCost, int& iMilitaryCost, int& iExtraCost) const;
	int calculateUnitCost() const;
	int calculateUnitSupply(int& iPaidUnits, int& iBaseSupplyCost) const;
	int calculateUnitSupply() const;
	int calculatePreInflatedCosts() const;
	int calculateInflationRate() const;
	int calculateInflatedCosts() const;

	int calculateBaseNetGold() const;
	int calculateBaseNetResearch(TechTypes eTech = NO_TECH) const;
	int calculateResearchModifier(TechTypes eTech) const;
	int calculateGoldRate() const;
	int calculateResearchRate(TechTypes eTech = NO_TECH) const;
	int calculateTotalCommerce() const;

	bool isResearch() const;
	DllExport bool canEverResearch(TechTypes eTech) const;
	DllExport bool canResearch(TechTypes eTech, bool bTrade = false) const;
	DllExport TechTypes getCurrentResearch() const;
	bool isCurrentResearchRepeat() const;
	bool isNoResearchAvailable() const;
	DllExport int getResearchTurnsLeft(TechTypes eTech, bool bOverflow) const;

	bool isCivic(CivicTypes eCivic) const;
	bool canDoCivics(CivicTypes eCivic) const;
	DllExport bool canRevolution(CivicTypes* paeNewCivics) const;
	DllExport void revolution(CivicTypes* paeNewCivics, bool bForce = false);
	int getCivicPercentAnger(CivicTypes eCivic, bool bIgnore = false) const;

	bool canDoReligion(ReligionTypes eReligion) const;
	bool canChangeReligion() const;
	DllExport bool canConvert(ReligionTypes eReligion) const;
	DllExport void convert(ReligionTypes eReligion);
	bool hasHolyCity(ReligionTypes eReligion) const;
	int countHolyCities() const;
	DllExport void foundReligion(ReligionTypes eReligion, ReligionTypes eSlotReligion, bool bAward);

	bool hasHeadquarters(CorporationTypes eCorporation) const;
	int countHeadquarters() const;
	int countCorporations(CorporationTypes eCorporation) const;
	void foundCorporation(CorporationTypes eCorporation);

	DllExport int getCivicAnarchyLength(CivicTypes* paeNewCivics) const;
	DllExport int getReligionAnarchyLength() const;

	DllExport int unitsRequiredForGoldenAge() const;
	int unitsGoldenAgeCapable() const;
	DllExport int unitsGoldenAgeReady() const;
	void killGoldenAgeUnits(CvUnit* pUnitAlive);

	DllExport int greatPeopleThreshold(bool bMilitary = false) const;
	int specialistYield(SpecialistTypes eSpecialist, YieldTypes eYield) const;
	int specialistCommerce(SpecialistTypes eSpecialist, CommerceTypes eCommerce) const;

	DllExport CvPlot* getStartingPlot() const;
	DllExport void setStartingPlot(CvPlot* pNewValue, bool bUpdateStartDist);

	DllExport int getTotalPopulation() const;
	int getAveragePopulation() const;
	void changeTotalPopulation(int iChange);
	long getRealPopulation() const;
	int getReligionPopulation(ReligionTypes eReligion) const;

	int getTotalLand() const;
	void changeTotalLand(int iChange);

	int getTotalLandScored() const;
	void changeTotalLandScored(int iChange);

	DllExport int getGold() const;
	DllExport void setGold(int iNewValue);
	DllExport void changeGold(int iChange);

	int getGoldPerTurn() const;

	DllExport int getAdvancedStartPoints() const;
	DllExport void setAdvancedStartPoints(int iNewValue);
	DllExport void changeAdvancedStartPoints(int iChange);

	int getEspionageSpending(TeamTypes eAgainstTeam) const;
	DllExport bool canDoEspionageMission(EspionageMissionTypes eMission, PlayerTypes eTargetPlayer, const CvPlot* pPlot, int iExtraData, const CvUnit* pUnit) const;
	int getEspionageMissionBaseCost(EspionageMissionTypes eMission, PlayerTypes eTargetPlayer, const CvPlot* pPlot, int iExtraData, const CvUnit* pSpyUnit) const;
	int getEspionageMissionCost(EspionageMissionTypes eMission, PlayerTypes eTargetPlayer, const CvPlot* pPlot = NULL, int iExtraData = -1, const CvUnit* pSpyUnit = NULL) const;
	int getEspionageMissionCostModifier(EspionageMissionTypes eMission, PlayerTypes eTargetPlayer, const CvPlot* pPlot = NULL, int iExtraData = -1, const CvUnit* pSpyUnit = NULL) const;
	bool doEspionageMission(EspionageMissionTypes eMission, PlayerTypes eTargetPlayer, CvPlot* pPlot, int iExtraData, CvUnit* pUnit);

	int getEspionageSpendingWeightAgainstTeam(TeamTypes eIndex) const;
	void setEspionageSpendingWeightAgainstTeam(TeamTypes eIndex, int iValue);
	DllExport void changeEspionageSpendingWeightAgainstTeam(TeamTypes eIndex, int iChange);

	bool canStealTech(PlayerTypes eTarget, TechTypes eTech) const;
	bool canForceCivics(PlayerTypes eTarget, CivicTypes eCivic) const;
	bool canForceReligion(PlayerTypes eTarget, ReligionTypes eReligion) const;
	bool canSpyDestroyUnit(PlayerTypes eTarget, CvUnit& kUnit) const;
	bool canSpyBribeUnit(PlayerTypes eTarget, CvUnit& kUnit) const;
	bool canSpyDestroyBuilding(PlayerTypes eTarget, BuildingTypes eBuilding) const;
	bool canSpyDestroyProject(PlayerTypes eTarget, ProjectTypes eProject) const;

	DllExport void doAdvancedStartAction(AdvancedStartActionTypes eAction, int iX, int iY, int iData, bool bAdd);
	DllExport int getAdvancedStartUnitCost(UnitTypes eUnit, bool bAdd, CvPlot* pPlot = NULL);
	DllExport int getAdvancedStartCityCost(bool bAdd, CvPlot* pPlot = NULL);
	DllExport int getAdvancedStartPopCost(bool bAdd, CvCity* pCity = NULL);
	DllExport int getAdvancedStartCultureCost(bool bAdd, CvCity* pCity = NULL);
	DllExport int getAdvancedStartBuildingCost(BuildingTypes eBuilding, bool bAdd, CvCity* pCity = NULL);
	DllExport int getAdvancedStartImprovementCost(ImprovementTypes eImprovement, bool bAdd, CvPlot* pPlot = NULL);
	DllExport int getAdvancedStartRouteCost(RouteTypes eRoute, bool bAdd, CvPlot* pPlot = NULL);
	DllExport int getAdvancedStartTechCost(TechTypes eTech, bool bAdd);
	DllExport int getAdvancedStartVisibilityCost(bool bAdd, CvPlot* pPlot = NULL);

	DllExport int getGoldenAgeTurns() const;
	DllExport bool isGoldenAge() const;
	void changeGoldenAgeTurns(int iChange);
	int getGoldenAgeLength() const;

	int getNumUnitGoldenAges() const;
	void changeNumUnitGoldenAges(int iChange);

	int getAnarchyTurns() const;
	DllExport bool isAnarchy() const;
	void changeAnarchyTurns(int iChange);

	int getStrikeTurns() const;
	void changeStrikeTurns(int iChange);

	int getMaxAnarchyTurns() const;
	void updateMaxAnarchyTurns();

	int getAnarchyModifier() const;
	void changeAnarchyModifier(int iChange);

	int getGoldenAgeModifier() const;
	void changeGoldenAgeModifier(int iChange);

	int getHurryModifier() const;
	void changeHurryModifier(int iChange);

	void createGreatPeople(UnitTypes eGreatPersonUnit, bool bIncrementThreshold, bool bIncrementExperience, int iX, int iY);

	int getGreatPeopleCreated() const;
	void incrementGreatPeopleCreated();

	int getGreatGeneralsCreated() const;
	void incrementGreatGeneralsCreated();

	int getGreatPeopleThresholdModifier() const;
	void changeGreatPeopleThresholdModifier(int iChange);

	int getGreatGeneralsThresholdModifier() const;
	void changeGreatGeneralsThresholdModifier(int iChange);

	int getGreatPeopleRateModifier() const;
	void changeGreatPeopleRateModifier(int iChange);

	int getGreatGeneralRateModifier() const;
	void changeGreatGeneralRateModifier(int iChange);

	int getDomesticGreatGeneralRateModifier() const;
	void changeDomesticGreatGeneralRateModifier(int iChange);

	int getStateReligionGreatPeopleRateModifier() const;
	void changeStateReligionGreatPeopleRateModifier(int iChange);

	int getMaxGlobalBuildingProductionModifier() const;
	void changeMaxGlobalBuildingProductionModifier(int iChange);

	int getMaxTeamBuildingProductionModifier() const;
	void changeMaxTeamBuildingProductionModifier(int iChange);

	int getMaxPlayerBuildingProductionModifier() const;
	void changeMaxPlayerBuildingProductionModifier(int iChange);

	int getFreeExperience() const;
	void changeFreeExperience(int iChange);

	int getFeatureProductionModifier() const;
	void changeFeatureProductionModifier(int iChange);

	int getWorkerSpeedModifier() const;
	void changeWorkerSpeedModifier(int iChange);

	int getImprovementUpgradeRateModifier() const;
	void changeImprovementUpgradeRateModifier(int iChange);

	int getMilitaryProductionModifier() const;
	void changeMilitaryProductionModifier(int iChange);

	int getSpaceProductionModifier() const;
	void changeSpaceProductionModifier(int iChange);

	int getCityDefenseModifier() const;
	void changeCityDefenseModifier(int iChange);

	int getNumNukeUnits() const;
	void changeNumNukeUnits(int iChange);

	int getNumOutsideUnits() const;
	void changeNumOutsideUnits(int iChange);

	int getBaseFreeUnits() const;
	void changeBaseFreeUnits(int iChange);

	int getBaseFreeMilitaryUnits() const;
	void changeBaseFreeMilitaryUnits(int iChange);

	int getFreeUnitsPopulationPercent() const;
	void changeFreeUnitsPopulationPercent(int iChange);

	int getFreeMilitaryUnitsPopulationPercent() const;
	void changeFreeMilitaryUnitsPopulationPercent(int iChange);

	int getGoldPerUnit() const;
	void changeGoldPerUnit(int iChange);

	int getGoldPerMilitaryUnit() const;
	void changeGoldPerMilitaryUnit(int iChange);

	int getExtraUnitCost() const;
	void changeExtraUnitCost(int iChange);

	int getNumMilitaryUnits() const;
	void changeNumMilitaryUnits(int iChange);

	int getHappyPerMilitaryUnit() const;
	void changeHappyPerMilitaryUnit(int iChange);

	int getMilitaryFoodProductionCount() const;
	bool isMilitaryFoodProduction() const;
	void changeMilitaryFoodProductionCount(int iChange);

	int getHighestUnitLevel() const;
	void setHighestUnitLevel(int iNewValue);

	int getConscriptCount() const;
	void setConscriptCount(int iNewValue);
	void changeConscriptCount(int iChange);

	DllExport int getMaxConscript() const;
	void changeMaxConscript(int iChange);

	DllExport int getOverflowResearch() const;
	void setOverflowResearch(int iNewValue);
	void changeOverflowResearch(int iChange);

	int getNoUnhealthyPopulationCount() const;
	bool isNoUnhealthyPopulation() const;
	void changeNoUnhealthyPopulationCount(int iChange);

	int getExpInBorderModifier() const;
	void changeExpInBorderModifier(int iChange);

	int getBuildingOnlyHealthyCount() const;
	bool isBuildingOnlyHealthy() const;
	void changeBuildingOnlyHealthyCount(int iChange);

	int getDistanceMaintenanceModifier() const;
	void changeDistanceMaintenanceModifier(int iChange);

	int getNumCitiesMaintenanceModifier() const;
	void changeNumCitiesMaintenanceModifier(int iChange);

	int getCorporationMaintenanceModifier() const;
	void changeCorporationMaintenanceModifier(int iChange);

	int getTotalMaintenance() const;
	void changeTotalMaintenance(int iChange);

	int getUpkeepModifier() const;
	void changeUpkeepModifier(int iChange);

	int getLevelExperienceModifier() const;
	void changeLevelExperienceModifier(int iChange);

	DllExport int getExtraHealth() const;
	void changeExtraHealth(int iChange);

	int getBuildingGoodHealth() const;
	void changeBuildingGoodHealth(int iChange);

	int getBuildingBadHealth() const;
	void changeBuildingBadHealth(int iChange);

	int getExtraHappiness() const;
	void changeExtraHappiness(int iChange);

	int getBuildingHappiness() const;
	void changeBuildingHappiness(int iChange);

	int getLargestCityHappiness() const;
	void changeLargestCityHappiness(int iChange);

	int getWarWearinessPercentAnger() const;
	void updateWarWearinessPercentAnger();
	int getModifiedWarWearinessPercentAnger(int iWarWearinessPercentAnger) const;

	int getWarWearinessModifier() const;
	void changeWarWearinessModifier(int iChange);

	int getFreeSpecialist() const;
	void changeFreeSpecialist(int iChange);

	int getNoForeignTradeCount() const;
	bool isNoForeignTrade() const;
	void changeNoForeignTradeCount(int iChange);

	int getNoCorporationsCount() const;
	bool isNoCorporations() const;
	void changeNoCorporationsCount(int iChange);

	int getNoForeignCorporationsCount() const;
	bool isNoForeignCorporations() const;
	void changeNoForeignCorporationsCount(int iChange);

	int getCoastalTradeRoutes() const;
	void changeCoastalTradeRoutes(int iChange);

	int getTradeRoutes() const;
	void changeTradeRoutes(int iChange);

	DllExport int getRevolutionTimer() const;
	void setRevolutionTimer(int iNewValue);
	void changeRevolutionTimer(int iChange);

	int getConversionTimer() const;
	void setConversionTimer(int iNewValue);
	void changeConversionTimer(int iChange);

	int getStateReligionCount() const;
	bool isStateReligion() const;
	void changeStateReligionCount(int iChange);

	int getNoNonStateReligionSpreadCount() const;
	DllExport bool isNoNonStateReligionSpread() const;
	void changeNoNonStateReligionSpreadCount(int iChange);

	DllExport int getStateReligionHappiness() const;
	void changeStateReligionHappiness(int iChange);

	int getNonStateReligionHappiness() const;
	void changeNonStateReligionHappiness(int iChange);

	int getStateReligionUnitProductionModifier() const;
	void changeStateReligionUnitProductionModifier(int iChange);

	int getStateReligionBuildingProductionModifier() const;
	void changeStateReligionBuildingProductionModifier(int iChange);

	int getStateReligionFreeExperience() const;
	void changeStateReligionFreeExperience(int iChange);

	DllExport CvCity* getCapitalCity() const;
	void setCapitalCity(CvCity* pNewCapitalCity);

	int getCitiesLost() const;
	void changeCitiesLost(int iChange);

	int getWinsVsBarbs() const;
	void changeWinsVsBarbs(int iChange);

	DllExport int getAssets() const;
	void changeAssets(int iChange);

	DllExport int getPower() const;
	void changePower(int iChange);

	DllExport int getPopScore(bool bCheckVassal = true) const;
	void changePopScore(int iChange);
	DllExport int getLandScore(bool bCheckVassal = true) const;
	void changeLandScore(int iChange);
	DllExport int getTechScore() const;
	void changeTechScore(int iChange);
	DllExport int getWondersScore() const;
	void changeWondersScore(int iChange);

	int getCombatExperience() const;
	void setCombatExperience(int iExperience);
	void changeCombatExperience(int iChange);

	DllExport bool isConnected() const;
	DllExport int getNetID() const;
	DllExport void setNetID(int iNetID);
	DllExport void sendReminder();

	uint getStartTime() const;
	DllExport void setStartTime(uint uiStartTime);
	DllExport uint getTotalTimePlayed() const;

	bool isMinorCiv() const;

	DllExport bool isAlive() const;
	DllExport bool isEverAlive() const;
	void setAlive(bool bNewValue);
	void verifyAlive();

	DllExport bool isTurnActive() const;
	DllExport void setTurnActive(bool bNewValue, bool bDoTurn = true);

	bool isAutoMoves() const;
	DllExport void setAutoMoves(bool bNewValue);
	DllExport void setTurnActiveForPbem(bool bActive);

	DllExport bool isPbemNewTurn() const;
	DllExport void setPbemNewTurn(bool bNew);

	bool isEndTurn() const;
	DllExport void setEndTurn(bool bNewValue);

	DllExport bool isTurnDone() const;

	bool isExtendedGame() const;
	DllExport void makeExtendedGame();

	bool isFoundedFirstCity() const;
	void setFoundedFirstCity(bool bNewValue);

	DllExport bool isStrike() const;
	void setStrike(bool bNewValue);

	DllExport PlayerTypes getID() const;

	DllExport HandicapTypes getHandicapType() const;

	DllExport CivilizationTypes getCivilizationType() const;

	DllExport LeaderHeadTypes getLeaderType() const;

	LeaderHeadTypes getPersonalityType() const;
	void setPersonalityType(LeaderHeadTypes eNewValue);

	DllExport EraTypes getCurrentEra() const;
	void setCurrentEra(EraTypes eNewValue);

	ReligionTypes getLastStateReligion() const;
	DllExport ReligionTypes getStateReligion() const;
	void setLastStateReligion(ReligionTypes eNewValue);

	PlayerTypes getParent() const;
	void setParent(PlayerTypes eParent);

	DllExport TeamTypes getTeam() const;
	void setTeam(TeamTypes eTeam);
	void updateTeamType();

	DllExport PlayerColorTypes getPlayerColor() const;
	DllExport int getPlayerTextColorR() const;
	DllExport int getPlayerTextColorG() const;
	DllExport int getPlayerTextColorB() const;
	DllExport int getPlayerTextColorA() const;

	int getSeaPlotYield(YieldTypes eIndex) const;
	void changeSeaPlotYield(YieldTypes eIndex, int iChange);

	int getYieldRateModifier(YieldTypes eIndex) const;
	void changeYieldRateModifier(YieldTypes eIndex, int iChange);

	int getCapitalYieldRateModifier(YieldTypes eIndex) const;
	void changeCapitalYieldRateModifier(YieldTypes eIndex, int iChange);

	int getExtraYieldThreshold(YieldTypes eIndex) const;
	void updateExtraYieldThreshold(YieldTypes eIndex);

	int getTradeYieldModifier(YieldTypes eIndex) const;
	void changeTradeYieldModifier(YieldTypes eIndex, int iChange);

	int getFreeCityCommerce(CommerceTypes eIndex) const;
	void changeFreeCityCommerce(CommerceTypes eIndex, int iChange);

	int getCommercePercent(CommerceTypes eIndex) const;
	void setCommercePercent(CommerceTypes eIndex, int iNewValue);
	DllExport void changeCommercePercent(CommerceTypes eIndex, int iChange);

	int getCommerceRate(CommerceTypes eIndex) const;
	void changeCommerceRate(CommerceTypes eIndex, int iChange);

	int getCommerceRateModifier(CommerceTypes eIndex) const;
	void changeCommerceRateModifier(CommerceTypes eIndex, int iChange);

	int getCapitalCommerceRateModifier(CommerceTypes eIndex) const;
	void changeCapitalCommerceRateModifier(CommerceTypes eIndex, int iChange);

	int getStateReligionBuildingCommerce(CommerceTypes eIndex) const;
	void changeStateReligionBuildingCommerce(CommerceTypes eIndex, int iChange);

	int getSpecialistExtraCommerce(CommerceTypes eIndex) const;
	void changeSpecialistExtraCommerce(CommerceTypes eIndex, int iChange);

	int getCommerceFlexibleCount(CommerceTypes eIndex) const;
	bool isCommerceFlexible(CommerceTypes eIndex) const;
	void changeCommerceFlexibleCount(CommerceTypes eIndex, int iChange);

	int getGoldPerTurnByPlayer(PlayerTypes eIndex) const;
	void changeGoldPerTurnByPlayer(PlayerTypes eIndex, int iChange);

	bool isFeatAccomplished(FeatTypes eIndex) const;
	void setFeatAccomplished(FeatTypes eIndex, bool bNewValue);

	DllExport bool isOption(PlayerOptionTypes eIndex) const;
	DllExport void setOption(PlayerOptionTypes eIndex, bool bNewValue);

	DllExport bool isLoyalMember(VoteSourceTypes eVoteSource) const;
	DllExport void setLoyalMember(VoteSourceTypes eVoteSource, bool bNewValue);

	DllExport bool isPlayable() const;
	DllExport void setPlayable(bool bNewValue);

	int getBonusExport(BonusTypes eIndex) const;
	void changeBonusExport(BonusTypes eIndex, int iChange);

	int getBonusImport(BonusTypes eIndex) const;
	void changeBonusImport(BonusTypes eIndex, int iChange);

	int getImprovementCount(ImprovementTypes eIndex) const;
	void changeImprovementCount(ImprovementTypes eIndex, int iChange);

	int getFreeBuildingCount(BuildingTypes eIndex) const;
	bool isBuildingFree(BuildingTypes eIndex) const;
	void changeFreeBuildingCount(BuildingTypes eIndex, int iChange);

	int getExtraBuildingHappiness(BuildingTypes eIndex) const;
	void changeExtraBuildingHappiness(BuildingTypes eIndex, int iChange);
	int getExtraBuildingHealth(BuildingTypes eIndex) const;
	void changeExtraBuildingHealth(BuildingTypes eIndex, int iChange);

	int getFeatureHappiness(FeatureTypes eIndex) const;
	void changeFeatureHappiness(FeatureTypes eIndex, int iChange);

	int getUnitClassCount(UnitClassTypes eIndex) const;
	bool isUnitClassMaxedOut(UnitClassTypes eIndex, int iExtra = 0) const;
	void changeUnitClassCount(UnitClassTypes eIndex, int iChange);
	int getUnitClassMaking(UnitClassTypes eIndex) const;
	void changeUnitClassMaking(UnitClassTypes eIndex, int iChange);
	int getUnitClassCountPlusMaking(UnitClassTypes eIndex) const;

	int getBuildingClassCount(BuildingClassTypes eIndex) const;
	bool isBuildingClassMaxedOut(BuildingClassTypes eIndex, int iExtra = 0) const;
	void changeBuildingClassCount(BuildingClassTypes eIndex, int iChange);
	int getBuildingClassMaking(BuildingClassTypes eIndex) const;
	void changeBuildingClassMaking(BuildingClassTypes eIndex, int iChange);
	int getBuildingClassCountPlusMaking(BuildingClassTypes eIndex) const;

	int getHurryCount(HurryTypes eIndex) const;
	DllExport bool canHurry(HurryTypes eIndex) const;
	bool canPopRush();
	void changeHurryCount(HurryTypes eIndex, int iChange);

	int getSpecialBuildingNotRequiredCount(SpecialBuildingTypes eIndex) const;
	bool isSpecialBuildingNotRequired(SpecialBuildingTypes eIndex) const;
	void changeSpecialBuildingNotRequiredCount(SpecialBuildingTypes eIndex, int iChange);

	int getHasCivicOptionCount(CivicOptionTypes eIndex) const;
	bool isHasCivicOption(CivicOptionTypes eIndex) const;
	void changeHasCivicOptionCount(CivicOptionTypes eIndex, int iChange);

	int getNoCivicUpkeepCount(CivicOptionTypes eIndex) const;
	bool isNoCivicUpkeep(CivicOptionTypes eIndex) const;
	void changeNoCivicUpkeepCount(CivicOptionTypes eIndex, int iChange);

	int getHasReligionCount(ReligionTypes eIndex) const;
	int countTotalHasReligion() const;
	int findHighestHasReligionCount() const;
	void changeHasReligionCount(ReligionTypes eIndex, int iChange);

	int getHasCorporationCount(CorporationTypes eIndex) const;
	int countTotalHasCorporation() const;
	void changeHasCorporationCount(CorporationTypes eIndex, int iChange);
	bool isActiveCorporation(CorporationTypes eIndex) const;

	int getUpkeepCount(UpkeepTypes eIndex) const;
	void changeUpkeepCount(UpkeepTypes eIndex, int iChange);

	int getSpecialistValidCount(SpecialistTypes eIndex) const;
	DllExport bool isSpecialistValid(SpecialistTypes eIndex) const;
	void changeSpecialistValidCount(SpecialistTypes eIndex, int iChange);

	DllExport bool isResearchingTech(TechTypes eIndex) const;
	void setResearchingTech(TechTypes eIndex, bool bNewValue);

	DllExport CivicTypes getCivics(CivicOptionTypes eIndex) const;
	int getSingleCivicUpkeep(CivicTypes eCivic, bool bIgnoreAnarchy = false) const;
	int getCivicUpkeep(CivicTypes* paeCivics = NULL, bool bIgnoreAnarchy = false) const;
	void setCivics(CivicOptionTypes eIndex, CivicTypes eNewValue);

	int getSpecialistExtraYield(SpecialistTypes eIndex1, YieldTypes eIndex2) const;
	void changeSpecialistExtraYield(SpecialistTypes eIndex1, YieldTypes eIndex2, int iChange);

	int getImprovementYieldChange(ImprovementTypes eIndex1, YieldTypes eIndex2) const;
	void changeImprovementYieldChange(ImprovementTypes eIndex1, YieldTypes eIndex2, int iChange);

	void updateGroupCycle(CvUnit* pUnit);
	void removeGroupCycle(int iID);
	CLLNode<int>* deleteGroupCycleNode(CLLNode<int>* pNode);
	CLLNode<int>* nextGroupCycleNode(CLLNode<int>* pNode) const;
	CLLNode<int>* previousGroupCycleNode(CLLNode<int>* pNode) const;
	CLLNode<int>* headGroupCycleNode() const;
	CLLNode<int>* tailGroupCycleNode() const;

	int findPathLength(TechTypes eTech, bool bCost = true) const;
	int getQueuePosition(TechTypes eTech) const;
	DllExport void clearResearchQueue();
	DllExport bool pushResearch(TechTypes eTech, bool bClear = false);
	void popResearch(TechTypes eTech);
	int getLengthResearchQueue() const;
	CLLNode<TechTypes>* nextResearchQueueNode(CLLNode<TechTypes>* pNode) const;
	CLLNode<TechTypes>* headResearchQueueNode() const;
	CLLNode<TechTypes>* tailResearchQueueNode() const;

	void addCityName(const CvWString& szName);
	int getNumCityNames() const;
	CvWString getCityName(int iIndex) const;
	CLLNode<CvWString>* nextCityNameNode(CLLNode<CvWString>* pNode) const;
	CLLNode<CvWString>* headCityNameNode() const;

	// plot groups iteration
	CvPlotGroup* firstPlotGroup(int *pIterIdx, bool bRev=false) const;
	CvPlotGroup* nextPlotGroup(int *pIterIdx, bool bRev=false) const;
	int getNumPlotGroups() const;
	CvPlotGroup* getPlotGroup(int iID) const;
	CvPlotGroup* addPlotGroup();
	void deletePlotGroup(int iID);

	// city iteration
	DllExport CvCity* firstCity(int *pIterIdx, bool bRev=false) const;
	DllExport CvCity* nextCity(int *pIterIdx, bool bRev=false) const;
	DllExport int getNumCities() const;
	DllExport CvCity* getCity(int iID) const;
	CvCity* addCity();
	void deleteCity(int iID);

	// unit iteration
	DllExport CvUnit* firstUnit(int *pIterIdx, bool bRev=false) const;
	DllExport CvUnit* nextUnit(int *pIterIdx, bool bRev=false) const;
	DllExport int getNumUnits() const;
	DllExport CvUnit* getUnit(int iID) const;
	CvUnit* addUnit();
	void deleteUnit(int iID);

	// selection groups iteration
	DllExport CvSelectionGroup* firstSelectionGroup(int *pIterIdx, bool bRev=false) const;
	DllExport CvSelectionGroup* nextSelectionGroup(int *pIterIdx, bool bRev=false) const;
	int getNumSelectionGroups() const;
	CvSelectionGroup* getSelectionGroup(int iID) const;
	CvSelectionGroup* addSelectionGroup();
	void deleteSelectionGroup(int iID);

	// pending triggers iteration
	EventTriggeredData* firstEventTriggered(int *pIterIdx, bool bRev=false) const;
	EventTriggeredData* nextEventTriggered(int *pIterIdx, bool bRev=false) const;
	int getNumEventsTriggered() const;
	EventTriggeredData* getEventTriggered(int iID) const;
	EventTriggeredData* addEventTriggered();
	void deleteEventTriggered(int iID);
	EventTriggeredData* initTriggeredData(EventTriggerTypes eEventTrigger, bool bFire = false, int iCityId = -1, int iPlotX = INVALID_PLOT_COORD, int iPlotY = INVALID_PLOT_COORD, PlayerTypes eOtherPlayer = NO_PLAYER, int iOtherPlayerCityId = -1, ReligionTypes eReligion = NO_RELIGION, CorporationTypes eCorporation = NO_CORPORATION, int iUnitId = -1, BuildingTypes eBuilding = NO_BUILDING);
	int getEventTriggerWeight(EventTriggerTypes eTrigger) const;

	DllExport void addMessage(const CvTalkingHeadMessage& message);
	void showMissedMessages();
	void clearMessages();
	DllExport const CvMessageQueue& getGameMessages() const;
	DllExport void expireMessages();
	DllExport void addPopup(CvPopupInfo* pInfo, bool bFront = false);
	void clearPopups();
	DllExport CvPopupInfo* popFrontPopup();
	DllExport const CvPopupQueue& getPopups() const;
	DllExport void addDiplomacy(CvDiploParameters* pDiplo);
	void clearDiplomacy();
	DllExport const CvDiploQueue& getDiplomacy() const;
	DllExport CvDiploParameters* popFrontDiplomacy();
	DllExport void showSpaceShip();
	DllExport void clearSpaceShipPopups();
	DllExport void cheatSpaceShipParts();

	int getScoreHistory(int iTurn) const;
	void updateScoreHistory(int iTurn, int iBestScore);

	int getEconomyHistory(int iTurn) const;
	void updateEconomyHistory(int iTurn, int iBestEconomy);
	int getIndustryHistory(int iTurn) const;
	void updateIndustryHistory(int iTurn, int iBestIndustry);
	int getAgricultureHistory(int iTurn) const;
	void updateAgricultureHistory(int iTurn, int iBestAgriculture);
	int getPowerHistory(int iTurn) const;
	void updatePowerHistory(int iTurn, int iBestPower);
	int getCultureHistory(int iTurn) const;
	void updateCultureHistory(int iTurn, int iBestCulture);
	int getEspionageHistory(int iTurn) const;
	void updateEspionageHistory(int iTurn, int iBestEspionage);

	// Script data needs to be a narrow string for pickling in Python
	std::string getScriptData() const;
	void setScriptData(std::string szNewValue);

	DllExport const CvString& getPbemEmailAddress() const;
	DllExport void setPbemEmailAddress(const char* szAddress);
	DllExport const CvString& getSmtpHost() const;
	DllExport void setSmtpHost(const char* szHost);

	const EventTriggeredData* getEventOccured(EventTypes eEvent) const;
	bool isTriggerFired(EventTriggerTypes eEventTrigger) const;
	void setEventOccured(EventTypes eEvent, const EventTriggeredData& kEventTriggered, bool bOthers = true);
	void resetEventOccured(EventTypes eEvent, bool bAnnounce = true);
	void setTriggerFired(const EventTriggeredData& kTriggeredData, bool bOthers = true, bool bAnnounce = true);
	void resetTriggerFired(EventTriggerTypes eEventTrigger);
	void trigger(EventTriggerTypes eEventTrigger);
	void trigger(const EventTriggeredData& kData);
	DllExport void applyEvent(EventTypes eEvent, int iTriggeredId, bool bUpdateTrigger = true);
	bool canDoEvent(EventTypes eEvent, const EventTriggeredData& kTriggeredData) const;
	TechTypes getBestEventTech(EventTypes eEvent, PlayerTypes eOtherPlayer) const;
	int getEventCost(EventTypes eEvent, PlayerTypes eOtherPlayer, bool bRandom) const;
	bool canTrigger(EventTriggerTypes eTrigger, PlayerTypes ePlayer, ReligionTypes eReligion) const;
	const EventTriggeredData* getEventCountdown(EventTypes eEvent) const;
	void setEventCountdown(EventTypes eEvent, const EventTriggeredData& kEventTriggered);
	void resetEventCountdown(EventTypes eEvent);

	bool isFreePromotion(UnitCombatTypes eUnitCombat, PromotionTypes ePromotion) const;
	void setFreePromotion(UnitCombatTypes eUnitCombat, PromotionTypes ePromotion, bool bFree);
	bool isFreePromotion(UnitClassTypes eUnitCombat, PromotionTypes ePromotion) const;
	void setFreePromotion(UnitClassTypes eUnitCombat, PromotionTypes ePromotion, bool bFree);

	PlayerVoteTypes getVote(int iId) const;
	void setVote(int iId, PlayerVoteTypes ePlayerVote);

	int getUnitExtraCost(UnitClassTypes eUnitClass) const;
	void setUnitExtraCost(UnitClassTypes eUnitClass, int iCost);

	DllExport bool splitEmpire(int iAreaId);
	bool canSplitEmpire() const;
	bool canSplitArea(int iAreaId) const;
	PlayerTypes getSplitEmpirePlayer(int iAreaId) const;
	bool getSplitEmpireLeaders(CivLeaderArray& aLeaders) const;

	DllExport void launch(VictoryTypes victoryType);

	bool hasShrine(ReligionTypes eReligion);
	int getVotes(VoteTypes eVote, VoteSourceTypes eVoteSource) const;
	void processVoteSourceBonus(VoteSourceTypes eVoteSource, bool bActive);
	bool canDoResolution(VoteSourceTypes eVoteSource, const VoteSelectionSubData& kData) const;
	bool canDefyResolution(VoteSourceTypes eVoteSource, const VoteSelectionSubData& kData) const;
	void setDefiedResolution(VoteSourceTypes eVoteSource, const VoteSelectionSubData& kData);
	void setEndorsedResolution(VoteSourceTypes eVoteSource, const VoteSelectionSubData& kData);
	bool isFullMember(VoteSourceTypes eVoteSource) const;
	bool isVotingMember(VoteSourceTypes eVoteSource) const;

	void invalidatePopulationRankCache();
	void invalidateYieldRankCache(YieldTypes eYield = NO_YIELD);
	void invalidateCommerceRankCache(CommerceTypes eCommerce = NO_COMMERCE);

	PlayerTypes pickConqueredCityOwner(const CvCity& kCity) const;
	bool canHaveTradeRoutesWith(PlayerTypes ePlayer) const;

	void forcePeace(PlayerTypes ePlayer);

	bool canSpiesEnterBorders(PlayerTypes ePlayer) const;
	int getNewCityProductionValue() const;

	int getGrowthThreshold(int iPopulation) const;

	void verifyUnitStacksValid();

	virtual void AI_init() = 0;
	virtual void AI_reset() = 0;
	virtual void AI_doTurnPre() = 0;
	virtual void AI_doTurnPost() = 0;
	virtual void AI_doTurnUnitsPre() = 0;
	virtual void AI_doTurnUnitsPost() = 0;
	virtual void AI_updateFoundValues(bool bStartingLoc = false) = 0;
	virtual void AI_unitUpdate() = 0;
	virtual void AI_makeAssignWorkDirty() = 0;
	virtual void AI_assignWorkingPlots() = 0;
	virtual void AI_updateAssignWork() = 0;
	virtual void AI_makeProductionDirty() = 0;
	virtual void AI_conquerCity(CvCity* pCity) = 0;
	virtual int AI_foundValue(int iX, int iY, int iMinUnitRange = -1, bool bStartingLoc = false) = 0;
	virtual bool AI_isCommercePlot(CvPlot* pPlot) = 0;
	virtual int AI_getPlotDanger(CvPlot* pPlot, int iRange = -1, bool bTestMoves = true) = 0;
	virtual bool AI_isFinancialTrouble() = 0;
	virtual TechTypes AI_bestTech(int iMaxPathLength = 1, bool bIgnoreCost = false, bool bAsync = false, TechTypes eIgnoreTech = NO_TECH, AdvisorTypes eIgnoreAdvisor = NO_ADVISOR) = 0;
	virtual void AI_chooseFreeTech() = 0;
	virtual void AI_chooseResearch() = 0;
	virtual bool AI_isWillingToTalk(PlayerTypes ePlayer) = 0;
	virtual bool AI_demandRebukedSneak(PlayerTypes ePlayer) = 0;
	virtual bool AI_demandRebukedWar(PlayerTypes ePlayer) = 0;
	virtual AttitudeTypes AI_getAttitude(PlayerTypes ePlayer, bool bForced = true) = 0;
	virtual PlayerVoteTypes AI_diploVote(const VoteSelectionSubData& kVoteData, VoteSourceTypes eVoteSource, bool bPropose) = 0;
	virtual int AI_dealVal(PlayerTypes ePlayer, const CLinkList<TradeData>* pList, bool bIgnoreAnnual = false, int iExtra = 0) = 0;
	virtual bool AI_considerOffer(PlayerTypes ePlayer, const CLinkList<TradeData>* pTheirList, const CLinkList<TradeData>* pOurList, int iChange = 1) = 0;
	virtual bool AI_counterPropose(PlayerTypes ePlayer, const CLinkList<TradeData>* pTheirList, const CLinkList<TradeData>* pOurList, CLinkList<TradeData>* pTheirInventory, CLinkList<TradeData>* pOurInventory, CLinkList<TradeData>* pTheirCounter, CLinkList<TradeData>* pOurCounter) = 0;
	virtual int AI_bonusVal(BonusTypes eBonus, int iChange = 0) = 0;
	virtual int AI_bonusTradeVal(BonusTypes eBonus, PlayerTypes ePlayer, int iChange = 0) = 0;
	virtual DenialTypes AI_bonusTrade(BonusTypes eBonus, PlayerTypes ePlayer) = 0;
	virtual int AI_cityTradeVal(CvCity* pCity) = 0;
	virtual DenialTypes AI_cityTrade(CvCity* pCity, PlayerTypes ePlayer) = 0;
	virtual DenialTypes AI_stopTradingTrade(TeamTypes eTradeTeam, PlayerTypes ePlayer) = 0;
	virtual DenialTypes AI_civicTrade(CivicTypes eCivic, PlayerTypes ePlayer) = 0;
	virtual DenialTypes AI_religionTrade(ReligionTypes eReligion, PlayerTypes ePlayer) = 0;
	virtual int AI_unitValue(UnitTypes eUnit, UnitAITypes eUnitAI, CvArea* pArea) = 0;
	virtual int AI_totalUnitAIs(UnitAITypes eUnitAI) = 0;
	virtual int AI_totalAreaUnitAIs(CvArea* pArea, UnitAITypes eUnitAI) = 0;
	virtual int AI_totalWaterAreaUnitAIs(CvArea* pArea, UnitAITypes eUnitAI) = 0;
	virtual int AI_plotTargetMissionAIs(CvPlot* pPlot, MissionAITypes eMissionAI, CvSelectionGroup* pSkipSelectionGroup = NULL, int iRange = 0) = 0;
	virtual int AI_unitTargetMissionAIs(CvUnit* pUnit, MissionAITypes eMissionAI, CvSelectionGroup* pSkipSelectionGroup = NULL) = 0;
	virtual int AI_civicValue(CivicTypes eCivic) = 0;
	virtual int AI_getNumAIUnits(UnitAITypes eIndex) = 0;
	virtual void AI_changePeacetimeTradeValue(PlayerTypes eIndex, int iChange) = 0;
	virtual void AI_changePeacetimeGrantValue(PlayerTypes eIndex, int iChange) = 0;
	virtual int AI_getAttitudeExtra(PlayerTypes eIndex) = 0;
	virtual void AI_setAttitudeExtra(PlayerTypes eIndex, int iNewValue) = 0;
	virtual void AI_changeAttitudeExtra(PlayerTypes eIndex, int iChange) = 0;
	virtual void AI_setFirstContact(PlayerTypes eIndex, bool bNewValue) = 0;
	virtual int AI_getMemoryCount(PlayerTypes eIndex1, MemoryTypes eIndex2) = 0;
	virtual void AI_changeMemoryCount(PlayerTypes eIndex1, MemoryTypes eIndex2, int iChange) = 0;
	virtual void AI_doCommerce() = 0;
	virtual EventTypes AI_chooseEvent(int iTriggeredId) = 0;
	virtual void AI_launch(VictoryTypes eVictory) = 0;
	virtual void AI_doAdvancedStart(bool bNoExit = false) = 0;
	virtual void AI_updateBonusValue() = 0;
	virtual void AI_updateBonusValue(BonusTypes eBonus) = 0;
	virtual ReligionTypes AI_chooseReligion() = 0;
	virtual int AI_getExtraGoldTarget() const = 0;
	virtual void AI_setExtraGoldTarget(int iNewValue) = 0;

protected:

	int m_iStartingX;
	int m_iStartingY;
	int m_iTotalPopulation;
	int m_iTotalLand;
	int m_iTotalLandScored;
	int m_iGold;
	int m_iGoldPerTurn;
	int m_iAdvancedStartPoints;
	int m_iGoldenAgeTurns;
	int m_iNumUnitGoldenAges;
	int m_iStrikeTurns;
	int m_iAnarchyTurns;
	int m_iMaxAnarchyTurns;
	int m_iAnarchyModifier;
	int m_iGoldenAgeModifier;
	int m_iGlobalHurryModifier;
	int m_iGreatPeopleCreated;
	int m_iGreatGeneralsCreated;
	int m_iGreatPeopleThresholdModifier;
	int m_iGreatGeneralsThresholdModifier;
	int m_iGreatPeopleRateModifier;
	int m_iGreatGeneralRateModifier;
	int m_iDomesticGreatGeneralRateModifier;
	int m_iStateReligionGreatPeopleRateModifier;
	int m_iMaxGlobalBuildingProductionModifier;
	int m_iMaxTeamBuildingProductionModifier;
	int m_iMaxPlayerBuildingProductionModifier;
	int m_iFreeExperience;
	int m_iFeatureProductionModifier;
	int m_iWorkerSpeedModifier;
	int m_iImprovementUpgradeRateModifier;
	int m_iMilitaryProductionModifier;
	int m_iSpaceProductionModifier;
	int m_iCityDefenseModifier;
	int m_iNumNukeUnits;
	int m_iNumOutsideUnits;
	int m_iBaseFreeUnits;
	int m_iBaseFreeMilitaryUnits;
	int m_iFreeUnitsPopulationPercent;
	int m_iFreeMilitaryUnitsPopulationPercent;
	int m_iGoldPerUnit;
	int m_iGoldPerMilitaryUnit;
	int m_iExtraUnitCost;
	int m_iNumMilitaryUnits;
	int m_iHappyPerMilitaryUnit;
	int m_iMilitaryFoodProductionCount;
	int m_iConscriptCount;
	int m_iMaxConscript;
	int m_iHighestUnitLevel;
	int m_iOverflowResearch;
	int m_iNoUnhealthyPopulationCount;
	int m_iExpInBorderModifier;
	int m_iBuildingOnlyHealthyCount;
	int m_iDistanceMaintenanceModifier;
	int m_iNumCitiesMaintenanceModifier;
	int m_iCorporationMaintenanceModifier;
	int m_iTotalMaintenance;
	int m_iUpkeepModifier;
	int m_iLevelExperienceModifier;
	int m_iExtraHealth;
	int m_iBuildingGoodHealth;
	int m_iBuildingBadHealth;
	int m_iExtraHappiness;
	int m_iBuildingHappiness;
	int m_iLargestCityHappiness;
	int m_iWarWearinessPercentAnger;
	int m_iWarWearinessModifier;
	int m_iFreeSpecialist;
	int m_iNoForeignTradeCount;
	int m_iNoCorporationsCount;
	int m_iNoForeignCorporationsCount;
	int m_iCoastalTradeRoutes;
	int m_iTradeRoutes;
	int m_iRevolutionTimer;
	int m_iConversionTimer;
	int m_iStateReligionCount;
	int m_iNoNonStateReligionSpreadCount;
	int m_iStateReligionHappiness;
	int m_iNonStateReligionHappiness;
	int m_iStateReligionUnitProductionModifier;
	int m_iStateReligionBuildingProductionModifier;
	int m_iStateReligionFreeExperience;
	int m_iCapitalCityID;
	int m_iCitiesLost;
	int m_iWinsVsBarbs;
	int m_iAssets;
	int m_iPower;
	int m_iPopulationScore;
	int m_iLandScore;
	int m_iTechScore;
	int m_iWondersScore;
	int m_iCombatExperience;
	int m_iPopRushHurryCount;
	int m_iInflationModifier;

	uint m_uiStartTime;  // XXX save these?

	bool m_bAlive;
	bool m_bEverAlive;
	bool m_bTurnActive;
	bool m_bAutoMoves;
	bool m_bEndTurn;
	bool m_bPbemNewTurn;
	bool m_bExtendedGame;
	bool m_bFoundedFirstCity;
	bool m_bStrike;
	bool m_bHuman;

	PlayerTypes m_eID;
	LeaderHeadTypes m_ePersonalityType;
	EraTypes m_eCurrentEra;
	ReligionTypes m_eLastStateReligion;
	PlayerTypes m_eParent;
	TeamTypes m_eTeamType;

	int* m_aiSeaPlotYield;
	int* m_aiYieldRateModifier;
	int* m_aiCapitalYieldRateModifier;
	int* m_aiExtraYieldThreshold;
	int* m_aiTradeYieldModifier;
	int* m_aiFreeCityCommerce;
	int* m_aiCommercePercent;
	int* m_aiCommerceRate;
	int* m_aiCommerceRateModifier;
	int* m_aiCapitalCommerceRateModifier;
	int* m_aiStateReligionBuildingCommerce;
	int* m_aiSpecialistExtraCommerce;
	int* m_aiCommerceFlexibleCount;
	int* m_aiGoldPerTurnByPlayer;
	int* m_aiEspionageSpendingWeightAgainstTeam;

	bool* m_abFeatAccomplished;
	bool* m_abOptions;

	CvString m_szScriptData;

	int* m_paiBonusExport;
	int* m_paiBonusImport;
	int* m_paiImprovementCount;
	int* m_paiFreeBuildingCount;
	int* m_paiExtraBuildingHappiness;
	int* m_paiExtraBuildingHealth;
	int** m_paiExtraBuildingYield;
	int** m_paiExtraBuildingCommerce;
	int* m_paiFeatureHappiness;
	int* m_paiUnitClassCount;
	int* m_paiUnitClassMaking;
	int* m_paiBuildingClassCount;
	int* m_paiBuildingClassMaking;
	int* m_paiHurryCount;
	int* m_paiSpecialBuildingNotRequiredCount;
	int* m_paiHasCivicOptionCount;
	int* m_paiNoCivicUpkeepCount;
	int* m_paiHasReligionCount;
	int* m_paiHasCorporationCount;
	int* m_paiUpkeepCount;
	int* m_paiSpecialistValidCount;

	bool* m_pabResearchingTech;
	bool* m_pabLoyalMember;

	std::vector<EventTriggerTypes> m_triggersFired;

	CivicTypes* m_paeCivics;

	int** m_ppaaiSpecialistExtraYield;
	int** m_ppaaiImprovementYieldChange;

	CLinkList<int> m_groupCycle;

	CLinkList<TechTypes> m_researchQueue;

	CLinkList<CvWString> m_cityNames;

	FFreeListTrashArray<CvPlotGroup> m_plotGroups;

	FFreeListTrashArray<CvCityAI> m_cities;

	FFreeListTrashArray<CvUnitAI> m_units;

	FFreeListTrashArray<CvSelectionGroupAI> m_selectionGroups;

	FFreeListTrashArray<EventTriggeredData> m_eventsTriggered;
	CvEventMap m_mapEventsOccured;
	CvEventMap m_mapEventCountdown;
	UnitCombatPromotionArray m_aFreeUnitCombatPromotions;
	UnitClassPromotionArray m_aFreeUnitClassPromotions;

	std::vector< std::pair<int, PlayerVoteTypes> > m_aVote;
	std::vector< std::pair<UnitClassTypes, int> > m_aUnitExtraCosts;

	CvMessageQueue m_listGameMessages;
	CvPopupQueue m_listPopups;
	CvDiploQueue m_listDiplomacy;

	CvTurnScoreMap m_mapScoreHistory;
	CvTurnScoreMap m_mapEconomyHistory;
	CvTurnScoreMap m_mapIndustryHistory;
	CvTurnScoreMap m_mapAgricultureHistory;
	CvTurnScoreMap m_mapPowerHistory;
	CvTurnScoreMap m_mapCultureHistory;
	CvTurnScoreMap m_mapEspionageHistory;

	void doGold();
	void doResearch();
	void doEspionagePoints();
	void doWarnings();
	void doEvents();

	bool checkExpireEvent(EventTypes eEvent, const EventTriggeredData& kTriggeredData) const;
	void expireEvent(EventTypes eEvent, const EventTriggeredData& kTriggeredData, bool bFail);
	bool isValidTriggerReligion(const CvEventTriggerInfo& kTrigger, CvCity* pCity, ReligionTypes eReligion) const;
	bool isValidTriggerCorporation(const CvEventTriggerInfo& kTrigger, CvCity* pCity, CorporationTypes eCorporation) const;
	CvCity* pickTriggerCity(EventTriggerTypes eTrigger) const;
	CvUnit* pickTriggerUnit(EventTriggerTypes eTrigger, CvPlot* pPlot, bool bPickPlot) const;
	bool isValidEventTech(TechTypes eTech, EventTypes eEvent, PlayerTypes eOtherPlayer) const;

	void verifyGoldCommercePercent();

	void processCivics(CivicTypes eCivic, int iChange);

	// for serialization
	virtual void read(FDataStreamBase* pStream);
	virtual void write(FDataStreamBase* pStream);

	void doUpdateCacheOnTurn();
	int getResearchTurnsLeftTimes100(TechTypes eTech, bool bOverflow) const;
};

#endif
