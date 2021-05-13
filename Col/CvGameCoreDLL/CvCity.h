#pragma once
// city.h
#ifndef CIV4_CITY_H
#define CIV4_CITY_H
#include "CvDLLEntity.h"
#include "LinkedList.h"

class CvPlot;
class CvArea;
class CvGenericBuilding;
class CvInfoBase;

class CvCity : public CvDLLEntity
{
public:
	CvCity();
	virtual ~CvCity();
	void init(int iID, PlayerTypes eOwner, int iX, int iY, bool bBumpUnits);
	void uninit();
	void reset(int iID = 0, PlayerTypes eOwner = NO_PLAYER, int iX = 0, int iY = 0, bool bConstructorCall = false);
	void setupGraphical();
	DllExport void kill();

	void doTurn();
	bool isCitySelected() const;
	DllExport bool canBeSelected() const;
	DllExport void updateSelectedCity();
	void updateYield();
	void updateVisibility();
	void createGreatGeneral(UnitTypes eGreatGeneralUnit, bool bIncrementExperience);

	DllExport void doTask(TaskTypes eTask, int iData1 = -1, int iData2 = -1, bool bOption = false, bool bAlt = false, bool bShift = false, bool bCtrl = false);

	DllExport void chooseProduction(UnitTypes eTrainUnit = NO_UNIT, BuildingTypes eConstructBuilding = NO_BUILDING, bool bFinish = false, bool bFront = false);

	DllExport int getCityPlotIndex(const CvPlot* pPlot) const;
	CvPlot* getCityIndexPlot(int iIndex) const;

	bool canWork(const CvPlot* pPlot) const;
	void verifyWorkingPlot(int iPlotIndex);
	void verifyWorkingPlots();
	int getNumAvailableWorkPlots() const;
	void clearWorkingOverride(int iIndex);
	int countNumImprovedPlots(ImprovementTypes eImprovement = NO_IMPROVEMENT, bool bPotential = false) const;
	int countNumWaterPlots() const;
	int countNumRiverPlots() const;

	int getNumProfessionBuildingSlots(ProfessionTypes eProfession) const;
	bool isAvailableProfessionSlot(ProfessionTypes eProfession, const CvUnit* pUnit) const;
	
	int professionCount(ProfessionTypes eProfession) const;

	int findPopulationRank() const;
	int findBaseYieldRateRank(YieldTypes eYield) const;
	DllExport int findYieldRateRank(YieldTypes eYield) const;

	UnitTypes allUpgradesAvailable(UnitTypes eUnit, int iUpgradeCount = 0) const;

	DllExport bool canTrain(UnitTypes eUnit, bool bContinue = false, bool bTestVisible = false, bool bIgnoreCost = false, bool bIgnoreUpgrades = false) const;
	DllExport bool canTrain(UnitCombatTypes eUnitCombat) const;
	DllExport bool canConstruct(BuildingTypes eBuilding, bool bContinue = false, bool bTestVisible = false, bool bIgnoreCost = false) const;
	DllExport bool canConvince(FatherPointTypes eFatherPoint, bool bContinue = false, bool bTestVisible = false) const;

	DllExport int getFoodTurnsLeft() const;
	DllExport bool isProduction() const;
	bool isProductionUnit() const;
	bool isProductionBuilding() const;
	bool isProductionConvince() const;

	bool canContinueProduction(OrderData order) const;
	int getProductionExperience(UnitTypes eUnit = NO_UNIT) const;
	void addProductionExperience(CvUnit* pUnit);

	DllExport UnitTypes getProductionUnit() const;
	UnitAITypes getProductionUnitAI() const;
	DllExport BuildingTypes getProductionBuilding() const;
	FatherPointTypes getProductionFatherPoint() const;
	const CvInfoBase* getProductionInfo() const;
	DllExport const wchar* getProductionName() const;
	DllExport const wchar* getProductionNameKey() const;
	DllExport int getGeneralProductionTurnsLeft() const;

	int getFirstUnitOrder(UnitTypes eUnit) const;
	int getFirstBuildingOrder(BuildingTypes eBuilding) const;
	int getNumTrainUnitAI(UnitAITypes eUnitAI) const;

	DllExport int getProduction() const;
	DllExport int getProductionNeeded(YieldTypes eYield) const;
	DllExport int getYieldProductionNeeded(UnitTypes eUnit, YieldTypes eYield) const;
	DllExport int getYieldProductionNeeded(BuildingTypes eBuilding, YieldTypes eYield) const;
	DllExport int getProductionTurnsLeft() const;
	DllExport int getProductionTurnsLeft(UnitTypes eUnit, int iNum) const;
	DllExport int getProductionTurnsLeft(BuildingTypes eBuilding, int iNum) const;
	int getProductionTurnsLeft(int iProductionNeeded, int iProduction, int iFirstProductionDifference, int iProductionDifference) const;
	void setProduction(int iNewValue);
	void changeProduction(int iChange);

	int getProductionModifier() const;
	int getProductionModifier(UnitTypes eUnit) const;
	int getProductionModifier(BuildingTypes eBuilding) const;

	DllExport int getCurrentProductionDifference(bool bOverflow) const;

	DllExport bool canHurry(HurryTypes eHurry, bool bTestVisible = false) const;
	void hurry(HurryTypes eHurry);

	void processBuilding(BuildingTypes eBuilding, int iChange);
	DllExport HandicapTypes getHandicapType() const;
	DllExport CivilizationTypes getCivilizationType() const;
	LeaderHeadTypes getPersonalityType() const;
	ArtStyleTypes getArtStyleType() const;
	DllExport CitySizeTypes getCitySizeType() const;

	bool hasTrait(TraitTypes eTrait) const;
	bool isHuman() const;
	bool isNative() const;
	DllExport bool isVisible(TeamTypes eTeam, bool bDebug) const;

	DllExport bool isCapital() const;
	bool isCoastal(int iMinWaterSize) const;
	DllExport bool isDisorder() const;

	int extraPopulation() const;

	int foodConsumption(int iExtra = 0) const;
	DllExport int foodDifference() const;
	DllExport int growthThreshold() const;

	int productionLeft() const;
	int hurryCost(bool bExtra, bool bIgnoreNew) const;
	int getHurryCostModifier(bool bIgnoreNew = false) const;
	DllExport int hurryGold(HurryTypes eHurry) const;
	DllExport int hurryPopulation(HurryTypes eHurry) const;
	int hurryProduction(HurryTypes eHurry) const;
	int maxHurryPopulation() const;
	int hurryYield(HurryTypes eHurry, YieldTypes eYield) const;

	int cultureDistance(int iDX, int iDY) const;

	bool isHasBuildingClass(BuildingClassTypes eIndex) const;
	DllExport bool isHasBuilding(BuildingTypes eIndex) const;
	int getProfessionOutput(ProfessionTypes eProfession, const CvUnit* pUnit, BuildingTypes* peBuilding = NULL) const;
	int getProfessionInput(ProfessionTypes eProfession, const CvUnit* pUnit) const;
	BuildingTypes getYieldBuilding(YieldTypes eYield) const;
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
	bool at(CvPlot* pPlot) const;
	DllExport CvPlot* plot() const;
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
	void updatePopulation(int iOldPopulation);
	long getRealPopulation() const;

	int getHighestPopulation() const;
	void setHighestPopulation(int iNewValue);

	int getWorksWaterCount() const;
	bool isWorksWater() const;
	void changeWorksWaterCount(int iChange);

	int getHealRate() const;
	void changeHealRate(int iChange);
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
	int getMilitaryProductionModifier() const;
	void changeMilitaryProductionModifier(int iChange);

	int getBuildingDefense() const;
	void changeBuildingDefense(int iChange);
	int getBuildingBombardDefense() const;
	void changeBuildingBombardDefense(int iChange);
	int getFreeExperience() const;
	void changeFreeExperience(int iChange);

	int getDefenseDamage() const;
	void changeDefenseDamage(int iChange);
	void changeDefenseModifier(int iChange);

	int getLastDefenseDamage() const;
	void setLastDefenseDamage(int iNewValue);
	bool isBombardable(const CvUnit* pUnit) const;
	int getTotalDefense() const;
	int getDefenseModifier() const;

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

	void setAllCitizensAutomated(bool bAutomated);
	void setCitizenAutomated(int iUnitID);

	DllExport bool isProductionAutomated() const;
	void setProductionAutomated(bool bNewValue, bool bClear);

	/* allows you to programatically specify a cities walls rather than having them be generated automagically */
	DllExport bool isWallOverride() const;
	DllExport void setWallOverride(bool bOverride);
	DllExport bool isBillboardDirty() const;
	DllExport void setBillboardDirty(bool bNewValue);
	DllExport bool isLayoutDirty() const;
	DllExport void setLayoutDirty(bool bNewValue);

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
	void setCultureLevel(CultureLevelTypes eNewValue);
	void updateCultureLevel();
	int getSeaPlotYield(YieldTypes eIndex) const;
	void changeSeaPlotYield(YieldTypes eIndex, int iChange);
	int getRiverPlotYield(YieldTypes eIndex) const;
	void changeRiverPlotYield(YieldTypes eIndex, int iChange);
	int getBaseRawYieldProduced(YieldTypes eIndex) const;
	int getRawYieldProduced(YieldTypes eIndex) const;
	int getRawYieldConsumed(YieldTypes eIndex) const;
	int getBaseYieldRateModifier(YieldTypes eIndex, int iExtra = 0) const;
	int getYieldRate(YieldTypes eIndex) const;
	int getCultureRate() const;
	void setYieldRateDirty();
	int getYieldRateModifier(YieldTypes eIndex) const;
	void changeYieldRateModifier(YieldTypes eIndex, int iChange);
	int getDomainFreeExperience(DomainTypes eIndex) const;
	void changeDomainFreeExperience(DomainTypes eIndex, int iChange);
	int getDomainProductionModifier(DomainTypes eIndex) const;
	void changeDomainProductionModifier(DomainTypes eIndex, int iChange);
	DllExport int getCulture(PlayerTypes eIndex) const;
	int countTotalCulture() const;
	PlayerTypes findHighestCulture() const;
	int calculateCulturePercent(PlayerTypes eIndex) const;
	int calculateTeamCulturePercent(TeamTypes eIndex) const;
	void setCulture(PlayerTypes eIndex, int iNewValue, bool bPlots);
	DllExport void changeCulture(PlayerTypes eIndex, int iChange, bool bPlots);

	int getYieldStored(YieldTypes eYield) const;
	void setYieldStored(YieldTypes eYield, int iValue);
	void changeYieldStored(YieldTypes eYield, int iChange);
	int getYieldRushed(YieldTypes eYield) const;
	void changeYieldRushed(YieldTypes eYield, int iChange);
	void calculateNetYields(int aiYields[], int* aiProducedYields = NULL, int* aiConsumedYields = NULL, bool bPrintWarning = false) const;
	int calculateNetYield(YieldTypes eYield) const;
	int calculateActualYieldProduced(YieldTypes eYield) const;
	int calculateActualYieldConsumed(YieldTypes eYield) const;
	int getOverflowYieldSellPercent() const;

	bool isEverOwned(PlayerTypes eIndex) const;
	void setEverOwned(PlayerTypes eIndex, bool bNewValue);
	DllExport bool isRevealed(TeamTypes eIndex, bool bDebug) const;
	DllExport void setRevealed(TeamTypes eIndex, bool bNewValue);

	DllExport const CvWString getName(uint uiForm = 0) const;
	DllExport const wchar* getNameKey() const;
	DllExport void setName(const wchar* szNewValue, bool bFound = false);
	void doFoundMessage();
	// Script data needs to be a narrow string for pickling in Python
	std::string getScriptData() const;
	void setScriptData(std::string szNewValue);

	DllExport int getBuildingProduction(BuildingTypes eIndex) const;
	void setBuildingProduction(BuildingTypes eIndex, int iNewValue);
	void changeBuildingProduction(BuildingTypes eIndex, int iChange);

	int getBuildingProductionTime(BuildingTypes eIndex) const;
	void setBuildingProductionTime(BuildingTypes eIndex, int iNewValue);
	void changeBuildingProductionTime(BuildingTypes eIndex, int iChange);

	int getBuildingOriginalOwner(BuildingTypes eIndex) const;
	int getBuildingOriginalTime(BuildingTypes eIndex) const;

	DllExport int getUnitProduction(UnitTypes eIndex) const;
	void setUnitProduction(UnitTypes eIndex, int iNewValue);
	void changeUnitProduction(UnitTypes eIndex, int iChange);

	int getUnitProductionTime(UnitTypes eIndex) const;
	void setUnitProductionTime(UnitTypes eIndex, int iNewValue);
	void changeUnitProductionTime(UnitTypes eIndex, int iChange);

	int getUnitCombatFreeExperience(UnitCombatTypes eIndex) const;
	void changeUnitCombatFreeExperience(UnitCombatTypes eIndex, int iChange);
	int getFreePromotionCount(PromotionTypes eIndex) const;
	bool isFreePromotion(PromotionTypes eIndex) const;
	void changeFreePromotionCount(PromotionTypes eIndex, int iChange);
	CvUnit* getUnitWorkingPlot(int iPlotIndex) const;
	DllExport bool isUnitWorkingPlot(int iPlotIndex) const;
	bool isUnitWorkingAnyPlot(const CvUnit* pUnit) const;
	DllExport CvUnit* getUnitWorkingPlot(const CvPlot* pPlot) const;
	bool isUnitWorkingPlot(const CvPlot* pPlot) const;
	void clearUnitWorkingPlot(int iPlotIndex);
	void clearUnitWorkingPlot(CvPlot* pPlot);
	void alterUnitWorkingPlot(int iPlotIndex, int iUnitId, bool bAskProfession);
	DllExport void alterUnitProfession(int iUnitId, ProfessionTypes eProfession);
	DllExport void ejectToTransport(int iUnitId, int iTransportId);
	void replaceCitizen(int iUnitId, int iReplacedUnitId, bool bAskProfession);
	bool isHasRealBuilding(BuildingTypes eIndex) const;
	DllExport void setHasRealBuilding(BuildingTypes eIndex, bool bNewValue);
	void setHasRealBuildingTimed(BuildingTypes eIndex, bool bNewValue, bool bFirst, PlayerTypes eOriginalOwner, int iOriginalTime);
	bool isValidBuildingLocation(BuildingTypes eIndex) const;
	bool isHasFreeBuilding(BuildingTypes eIndex) const;
	void setHasFreeBuilding(BuildingTypes eIndex, bool bNewValue);
	bool isHasConceptualBuilding(BuildingTypes eIndex) const;
	bool isDominantSpecialBuilding(BuildingTypes eIndex) const;
	void clearOrderQueue();
	DllExport void pushOrder(OrderTypes eOrder, int iData1, int iData2, bool bSave, bool bPop, bool bAppend, bool bForce = false);
	DllExport void popOrder(int iNum, bool bFinish = false, bool bChoose = false);
	bool processRequiredYields(int iNum);
	bool checkRequiredYields(OrderTypes eOrder, int iData1) const;
	void checkCompletedBuilds(YieldTypes eYield, int iChange);
	void getOrdersWaitingForYield(std::vector< std::pair<OrderTypes, int> >& aOrders, YieldTypes eYield, bool bYieldsComplete, int iChange) const;
	void startHeadOrder();
	void stopHeadOrder();
	DllExport int getOrderQueueLength() const;
	DllExport OrderData* getOrderFromQueue(int iIndex);
	DllExport CLLNode<OrderData>* nextOrderQueueNode(CLLNode<OrderData>* pNode) const;
	DllExport CLLNode<OrderData>* headOrderQueueNode() const;
	CLLNode<OrderData>* tailOrderQueueNode() const;
	// fill the kVisible array with buildings that you want shown in city, as well as the number of generics
	// This function is called whenever CvCity::setLayoutDirty() is called
	DllExport void getVisibleBuildings(std::list<BuildingTypes>& kVisible, int& iNumGenerics) const;
	// Fill the kEffectNames array with references to effects in the CIV4EffectInfos.xml to have a
	// city play a given set of effects. This is called whenever the interface updates the city billboard
	// or when the zoom level changes
	DllExport void getVisibleEffects(ZoomLevelTypes eCurrentZoom, std::vector<const TCHAR*>& kEffectNames) const;

	// Billboard appearance controls
	DllExport void getCityBillboardSizeIconColors(NiColorA& kDotColor, NiColorA& kTextColor) const;
	DllExport const TCHAR* getCityBillboardProductionIcon() const;
	DllExport bool getCityBillboardTopBarValues(float& fStored, float& fRate, float& fRateExtra) const;
	DllExport bool getCityBillboardBottomBarValues(float& fStored, float& fRate, float& fRateExtra) const;

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
	DllExport PlayerTypes getLiberationPlayer(bool bConquest) const;
	void liberate(bool bConquest);

	int getMaxYieldCapacity() const;
	bool isAutoRaze() const;
	void setScoutVisited(TeamTypes eTeam, bool bVisited);
	DllExport bool isScoutVisited(TeamTypes eTeam) const;
	GoodyTypes getGoodyType(const CvUnit* pUnit) const;
	int doGoody(CvUnit* pUnit, GoodyTypes eGoody);
	PlayerTypes getMissionaryPlayer() const;
	CivilizationTypes getMissionaryCivilization() const;
	void setMissionaryPlayer(PlayerTypes ePlayer);
	int getMissionaryRate() const;
	void setMissionaryRate(int iRate);

	DllExport int getRebelPercent() const;
	DllExport int getRebelSentiment() const;
	DllExport void setRebelSentiment(int iValue);

	int getTeachUnitMultiplier() const;
	void setTeachUnitMultiplier(int iModifier);
	UnitClassTypes bestTeachUnitClass();

	int getEducationThresholdMultiplier() const;
	void setEducationThresholdMultiplier(int iModifier);
	int educationThreshold() const;

	CvUnit* ejectBestDefender(CvUnit* pCurrentBest, CvUnit* pAttacker);
	CvUnit* getBestDefender(ProfessionTypes* peProfession, CvUnit* pCurrentBest, const CvUnit* pAttacker) const;

	bool canTradeAway(PlayerTypes eToPlayer) const;

	void read(FDataStreamBase* pStream);
	void write(FDataStreamBase* pStream);
	virtual void AI_init() = 0;
	virtual void AI_reset() = 0;
	virtual void AI_doTurn() = 0;
	virtual void AI_doNative() = 0;
	virtual void AI_assignWorkingPlots() = 0;
	virtual void AI_updateAssignWork() = 0;
	virtual bool AI_avoidGrowth() const = 0;
	virtual void AI_setAvoidGrowth(bool bNewValue) = 0;
	virtual void AI_chooseProduction() = 0;
	virtual UnitTypes AI_bestUnit(bool bAsync = false, UnitAITypes* peBestUnitAI = NULL, bool bPickAny = false) const = 0;
	virtual UnitTypes AI_bestUnitAI(UnitAITypes eUnitAI, bool bAsync = false) const = 0;
	virtual BuildingTypes AI_bestBuilding(int iFocusFlags = 0, int iMaxTurns = MAX_INT, bool bAsync = false) const = 0;
	virtual int AI_buildingValue(BuildingTypes eBuilding, int iFocusFlags = 0) const = 0;
	virtual int AI_neededSeaWorkers() const = 0;
	virtual bool AI_isDefended(int iExtra = 0) const = 0;
	virtual bool AI_isDanger() const = 0;
	virtual int AI_neededDefenders() const = 0;
	virtual int AI_numDefenders(bool bDefenseOnly = true, bool bIncludePotential = true) const = 0;
	virtual int AI_minDefenders() const = 0;
	virtual bool AI_isEmphasizeAvoidGrowth() const = 0;
	virtual bool AI_isAssignWorkDirty() const = 0;
	virtual CvCity* AI_getRouteToCity() const = 0;
	virtual void AI_setAssignWorkDirty(bool bNewValue) = 0;
	virtual bool AI_isChooseProductionDirty() const = 0;
	virtual void AI_setChooseProductionDirty(bool bNewValue) = 0;
	virtual bool AI_isEmphasize(EmphasizeTypes eIndex) const = 0;
	virtual void AI_setEmphasize(EmphasizeTypes eIndex, bool bNewValue) = 0;
	virtual int AI_getEmphasizeYieldCount(YieldTypes eIndex) const = 0;
	virtual int AI_getBestBuildValue(int iIndex) const = 0;
	virtual int AI_totalBestBuildValue(CvArea* pArea) const = 0;
	virtual int AI_countBestBuilds(CvArea* pArea) const = 0;
	virtual BuildTypes AI_getBestBuild(int iIndex) const = 0;
	virtual void AI_updateBestBuild() = 0;
	virtual int AI_cityValue() const = 0;
	virtual int AI_clearFeatureValue(int iIndex) = 0;
	virtual int AI_calculateCulturePressure() const = 0;
	virtual int AI_calculateWaterWorldPercent() const = 0;
	virtual int AI_playerCloseness(PlayerTypes eIndex, int iMaxDistance = 7) const = 0;
	virtual int AI_cityThreat(bool bDangerPercent = false) const = 0;
	virtual BuildingTypes AI_bestAdvancedStartBuilding(int iPass) const = 0;
	virtual int AI_getWorkersHave() const = 0;
	virtual int AI_getWorkersNeeded() const = 0;
	virtual void AI_changeWorkersHave(int iChange) = 0;
	virtual bool AI_removeWorstPopulationUnit(bool bDelete) = 0;
	virtual CvUnit* AI_bestPopulationUnit(UnitAITypes eUnitAI, ProfessionTypes eProfession = NO_PROFESSION) = 0;
	virtual ProfessionTypes AI_bestPlotProfession(const CvUnit* pUnit, const CvPlot* pPlot) const = 0;
	virtual int AI_bestProfessionPlot(ProfessionTypes eProfession, const CvUnit* pUnit) const = 0;
	virtual int AI_professionValue(ProfessionTypes eProfession, const CvUnit* pUnit, const CvPlot* pPlot, const CvUnit* pDisplaceUnit) const = 0;
	virtual int AI_professionBasicOutput(ProfessionTypes eProfession, UnitTypes eUnit, const CvPlot* pPlot) const = 0;
	virtual int AI_unitJoinCityValue(CvUnit* pUnit, ProfessionTypes* peNewProfession) const = 0;
	virtual int AI_unitJoinReplaceValue(CvUnit* pUnit, CvUnit** pReplaceUnit = NULL) const = 0;
	virtual bool AI_canMakeGift() const = 0;
	virtual int AI_getGiftTimer() const = 0;
	virtual void AI_setGiftTimer(int iNewValue) = 0;
	virtual void AI_changeGiftTimer(int iChange) = 0;
	virtual int AI_maxGoldTrade(PlayerTypes ePlayer) const = 0;
	virtual int AI_estimateYieldValue(YieldTypes eYield, int iAmount) const = 0;
	virtual int AI_getTargetSize() const = 0;
	virtual void AI_setTargetSize(int iNewValue) = 0;
	virtual int AI_getYieldOutputWeight(YieldTypes eYield) const = 0;
	virtual void AI_setYieldOutputWeight(YieldTypes eYield, int iNewValue) = 0;
	virtual int AI_getNeededYield(YieldTypes eYield) const = 0;
	virtual void AI_setNeededYield(YieldTypes eYield, int iNewValue) = 0;
	virtual int AI_getTradeBalance(YieldTypes eYield) const = 0;
	virtual void AI_changeTradeBalance(YieldTypes eYield, int iAmount) = 0;
	virtual int AI_getYieldAdvantage(YieldTypes eYield) const = 0;
	virtual void AI_setYieldAdvantage(YieldTypes eYield, int iNewValue) = 0;
	virtual void AI_assignDesiredYield() = 0;
	virtual YieldTypes AI_getDesiredYield() const = 0;
	virtual int AI_getFoodGatherable(int iPop, int iPlotFoodThreshold) const = 0;
	virtual bool AI_shouldImportYield(YieldTypes eYield) const = 0;
	virtual bool AI_shouldExportYield(YieldTypes eYield) const = 0;
	virtual int AI_getTransitYield(YieldTypes eYield) const = 0;
	virtual bool AI_isPort() const = 0;
	virtual void AI_setPort(bool iNewValue) = 0;
	virtual int AI_getRequiredYieldLevel(YieldTypes eYield) = 0;
	virtual void AI_educateStudent(int iUnitId) = 0;
	virtual bool AI_isWorkforceHack() = 0;
	virtual void AI_setWorkforceHack(bool bNewValue) = 0;
	virtual int AI_calculateAlarm(PlayerTypes eIndex) const = 0;

	void invalidatePopulationRankCache();
	void invalidateYieldRankCache(YieldTypes eYield = NO_YIELD);
	int getBestYieldAmountAvailable(ProfessionTypes eProfession, const CvUnit* pUnit) const;
	void addPopulationUnit(CvUnit* pUnit, ProfessionTypes eProfession);
	bool removePopulationUnit(CvUnit* pUnit, bool bDelete, ProfessionTypes eProfession);
	CvUnit* removeUnitType(UnitTypes eUnit, ProfessionTypes eProfession);
	void removeNonCityPopulationUnits();
	int getPopulationUnitId(int iPlotIndex) const;
	DllExport CvUnit* getPopulationUnitById(int iUnitId) const;
	CvUnit* getPopulationUnitByIndex(int iUnitIndex) const;
	int getPopulationUnitIndex(CvUnit *pUnit) const;
	CvPlot* getPlotWorkedByUnit(const CvUnit* pUnit) const;
	CvUnit* createYieldUnit(YieldTypes eYield, PlayerTypes ePlayer, int iYieldAmount);
	UnitClassTypes getTeachUnitClass() const;
	void setTeachUnitClass(UnitClassTypes eUnitClass);
	void ejectTeachUnits();

	bool canProduceYield(YieldTypes eYield);

	bool educateStudent(int iUnitId, UnitTypes eUnit);
	int getSpecialistTuition(UnitTypes eUnit) const;

	bool isExport(YieldTypes eYield) const;
	void addExport(YieldTypes eYield, bool bUpdateRoutes = true);
	void removeExport(YieldTypes eYield, bool bUpdateRoutes = true);
	void updateExport(YieldTypes eYield);
	void updateExports();
	bool isImport(YieldTypes eYield) const;
	void addImport(YieldTypes eYield, bool bUpdateRoutes = true);
	void removeImport(YieldTypes eYield, bool bUpdateRoutes = true);
	void updateImport(YieldTypes eYield);
	void updateImports();
	void removeTradeRoutes();
	void setMaintainLevel(YieldTypes eYield, int iMaintainLevel);
	int getMaintainLevel(YieldTypes eYield) const;

protected:
	int m_iID;
	int m_iX;
	int m_iY;
	int m_iRallyX;
	int m_iRallyY;
	int m_iGameTurnFounded;
	int m_iGameTurnAcquired;
	int m_iHighestPopulation;
	int m_iWorkingPopulation;
	int m_iNumBuildings;
	int m_iWorksWaterCount;
	int m_iHealRate;
	int m_iFoodKept;
	int m_iMaxFoodKeptPercent;
	int m_iOverflowProduction;
	int m_iMilitaryProductionModifier;
	int m_iBuildingDefense;
	int m_iBuildingBombardDefense;
	int m_iFreeExperience;
	int m_iDefenseDamage;
	int m_iLastDefenseDamage;
	int m_iOccupationTimer;
	int m_iCultureUpdateTimer;
	int m_iCitySizeBoost;
	int m_iHammers;
	int m_iMissionaryRate;
	int m_iRebelSentiment;
	int m_iTeachUnitMultiplier;
	int m_iEducationThresholdMultiplier;

	bool m_bNeverLost;
	bool m_bBombarded;
	bool m_bProductionAutomated;
	bool m_bWallOverride;
	bool m_bInfoDirty;
	bool m_bLayoutDirty;
	PlayerTypes m_eOwner;
	PlayerTypes m_ePreviousOwner;
	PlayerTypes m_eOriginalOwner;
	CultureLevelTypes m_eCultureLevel;
	UnitClassTypes m_eTeachUnitClass;
	PlayerTypes m_eMissionaryPlayer;
	int* m_aiSeaPlotYield;
	int* m_aiRiverPlotYield;
	int* m_aiYieldRateModifier;
	int* m_aiYieldStored;
	int* m_aiYieldRushed;
	int* m_aiDomainFreeExperience;
	int* m_aiDomainProductionModifier;
	int* m_aiCulture;
	bool* m_abEverOwned;
	bool* m_abRevealed;
	bool* m_abScoutVisited;
	CvWString m_szName;
	CvString m_szScriptData;
	int* m_paiBuildingProduction;
	int* m_paiBuildingProductionTime;
	int* m_paiBuildingOriginalOwner;
	int* m_paiBuildingOriginalTime;
	int* m_paiUnitProduction;
	int* m_paiUnitProductionTime;
	int* m_aiSpecialistWeights;
	int* m_paiUnitCombatFreeExperience;
	int* m_paiFreePromotionCount;
	bool* m_pabHasRealBuilding;
	bool* m_pabHasFreeBuilding;
	int* m_paiWorkingPlot;
	IDInfo* m_paTradeCities;
	mutable CLinkList<OrderData> m_orderQueue;
	std::vector< std::pair < float, float> > m_kWallOverridePoints;
	std::vector<EventTypes> m_aEventsOccured;
	std::vector<BuildingYieldChange> m_aBuildingYieldChange;
	std::vector<CvUnit*> m_aPopulationUnits;
	std::set<YieldTypes> m_setImports;
	std::set<YieldTypes> m_setExports;
	std::map<YieldTypes, int> m_mapMaintain;
	// CACHE: cache frequently used values
	mutable int	m_iPopulationRank;
	mutable bool m_bPopulationRankValid;
	int*	m_aiBaseYieldRank;
	bool*	m_abBaseYieldRankValid;
	int*	m_aiYieldRank;
	bool*	m_abYieldRankValid;

	void doGrowth();
	void doYields();
	void doCulture();
	void doPlotCulture(bool bUpdate, PlayerTypes ePlayer, int iCultureRate);
	void doSpecialists();
	void doProduction(bool bAllowNoProduction);
	void doDecay();
	void doMissionaries();
	void doRebelSentiment();
	bool doCheckProduction();
	void doCheat(bool bAlt, bool bShift, bool bCtrl);
	int getExtraProductionDifference(int iExtra, int iModifier) const;
	int getHurryCostModifier(UnitTypes eUnit, bool bIgnoreNew) const;
	int getHurryCostModifier(BuildingTypes eBuilding, bool bIgnoreNew) const;
	int getHurryCostModifier(int iBaseModifier, int iProduction, bool bIgnoreNew) const;
	int getHurryCost(bool bExtra, int iProductionLeft, int iHurryModifier, int iModifier) const;
	int getHurryPopulation(HurryTypes eHurry, int iHurryCost) const;
	int getHurryYieldDeficit(HurryTypes eHurry, YieldTypes eYield) const;
	int getHurryYieldNeeded(HurryTypes eHurry, YieldTypes eYield) const;
	int getProductionDifference(int iProductionModifier, bool bOverflow, bool bUseStoredHammers) const;
	int getStoredProductionDifference() const;

	void setUnitWorkingPlot(int iPlotIndex, int iUnitId);
	void setUnitWorkingPlot(const CvPlot* pPlot, int iUnitId);
	int getNextFreeUnitId() const;
	virtual bool AI_addBestCitizen() = 0;
	virtual bool AI_removeWorstCitizen() = 0;
};
#endif
