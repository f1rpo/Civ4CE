// city.h

#ifndef CIV4_CITY_H
#define CIV4_CITY_H

#include "CvDLLEntity.h"
#include "LinkedList.h"
//#include "CvStructs.h"
//#include "CvEnums.h"

class CvPlot;
class CvPlotGroup;
class CvArea;
class CvGenericBuilding;

typedef std::vector<bool> RowType;
typedef std::vector<RowType> PlotType;

class CvCity : public CvDLLEntity
{

public:

	enum SaveBits
	{
		SAVEDATA_LAST_DEFENSE_DAMAGE = 0x01
	};

	CvCity();
	virtual ~CvCity();

	void init(int iID, PlayerTypes eOwner, int iX, int iY);
	void uninit();
	void reset(int iID = 0, PlayerTypes eOwner = NO_PLAYER, int iX = 0, int iY = 0, bool bConstructorCall = false);
	void setupGraphical();

	DllExport void kill();																								// Exposed to Python

	void doTurn();

	bool isCitySelected();
	DllExport bool canBeSelected() const;
	DllExport void updateSelectedCity();

	void updateYield();

	void updateVisibility();

	void createGreatPeople(UnitTypes eGreatPersonUnit, bool bIncrementThreshold);		// Exposed to Python

	DllExport void doTask(TaskTypes eTask, int iData1 = -1, int iData2 = -1, bool bOption = false);		// Exposed to Python

	DllExport void chooseProduction(UnitTypes eTrainUnit = NO_UNIT, BuildingTypes eConstructBuilding = NO_BUILDING, ProjectTypes eCreateProject = NO_PROJECT, bool bFinish = false, bool bFront = false);		// Exposed to Python

	DllExport int getCityPlotIndex(const CvPlot* pPlot) const;				// Exposed to Python 
	CvPlot* getCityIndexPlot(int iIndex);															// Exposed to Python

	bool canWork(CvPlot* pPlot);																			// Exposed to Python
	void verifyWorkingPlot(int iIndex);
	void verifyWorkingPlots();
	void clearWorkingOverride(int iIndex);														// Exposed to Python
	int countNumImprovedPlots();																			// Exposed to Python
	int countNumWaterPlots();																					// Exposed to Python

	int findPopulationRank();																					// Exposed to Python
	int findBaseYieldRateRank(YieldTypes eYield);											// Exposed to Python
	DllExport int findYieldRateRank(YieldTypes eYield);								// Exposed to Python					
	DllExport int findCommerceRateRank(CommerceTypes eCommerce);			// Exposed to Python					

	UnitTypes allUpgradesAvailable(UnitTypes eUnit, int iUpgradeCount = 0);						// Exposed to Python
	DllExport bool isWorldWondersMaxed();																							// Exposed to Python
	DllExport bool isTeamWondersMaxed();																							// Exposed to Python
	DllExport bool isNationalWondersMaxed();																					// Exposed to Python
	DllExport bool isBuildingsMaxed();																								// Exposed to Python

	DllExport bool canTrain(UnitTypes eUnit, bool bContinue = false, bool bTestVisible = false);					// Exposed to Python 
	DllExport bool canConstruct(BuildingTypes eBuilding, bool bContinue = false, bool bTestVisible = false, bool bIgnoreCost = false);	// Exposed to Python  
	DllExport bool canCreate(ProjectTypes eProject, bool bContinue = false, bool bTestVisible = false);		// Exposed to Python 
	DllExport bool canMaintain(ProcessTypes eProcess, bool bContinue = false);														// Exposed to Python  
	bool canJoin();																													// Exposed to Python

	DllExport int getFoodTurnsLeft();																				// Exposed to Python
	DllExport bool isProduction();																					// Exposed to Python
	bool isProductionLimited();																							// Exposed to Python
	bool isProductionUnit();																								// Exposed to Python
	bool isProductionBuilding();																						// Exposed to Python
	bool isProductionProject();																							// Exposed to Python
	DllExport bool isProductionProcess();																		// Exposed to Python

	bool canContinueProduction(OrderData order);														// Exposed to Python
	int getProductionExperience(UnitTypes eUnit = NO_UNIT);									// Exposed to Python
	void addProductionExperience(CvUnit* pUnit, bool bConscript = false);		// Exposed to Python

	DllExport UnitTypes getProductionUnit();																// Exposed to Python
	UnitAITypes getProductionUnitAI();																			// Exposed to Python
	DllExport BuildingTypes getProductionBuilding();												// Exposed to Python
	DllExport ProjectTypes getProductionProject();													// Exposed to Python
	DllExport ProcessTypes getProductionProcess();													// Exposed to Python
	DllExport const wchar* getProductionName();															// Exposed to Python
	DllExport const wchar* getProductionNameKey();													// Exposed to Python
	DllExport const int getGeneralProductionTurnsLeft();										// Exposed to Python

	bool isFoodProduction();																								// Exposed to Python
	bool isFoodProduction(UnitTypes eUnit);																	// Exposed to Python
	int getFirstUnitOrder(UnitTypes eUnit);																	// Exposed to Python
	int getFirstBuildingOrder(BuildingTypes eBuilding);											// Exposed to Python
	int getFirstProjectOrder(ProjectTypes eProject);												// Exposed to Python
	int getNumTrainUnitAI(UnitAITypes eUnitAI);															// Exposed to Python

	DllExport int getProduction();																						// Exposed to Python
	DllExport int getProductionNeeded();																			// Exposed to Python
	DllExport int getProductionTurnsLeft();																		// Exposed to Python 
	DllExport int getProductionTurnsLeft(UnitTypes eUnit, int iNum);					// Exposed to Python
	DllExport int getProductionTurnsLeft(BuildingTypes eBuilding, int iNum);	// Exposed to Python
	DllExport int getProductionTurnsLeft(ProjectTypes eProject, int iNum);		// Exposed to Python
	int getProductionTurnsLeft(int iProductionNeeded, int iProduction, int iFirstProductionDifference, int iProductionDifference);
	void setProduction(int iNewValue);																			// Exposed to Python
	void changeProduction(int iChange);																			// Exposed to Python

	int getProductionModifier();																						// Exposed to Python
	int getProductionModifier(UnitTypes eUnit);															// Exposed to Python
	int getProductionModifier(BuildingTypes eBuilding);											// Exposed to Python
	int getProductionModifier(ProjectTypes eProject);												// Exposed to Python

	int getOverflowProductionDifference(int iProductionNeeded, int iProduction, int iProductionModifier, int iDiff, int iModifiedProduction);
	int getProductionDifference(int iProductionNeeded, int iProduction, int iProductionModifier, bool bFoodProduction, bool bOverflow);
	DllExport int getCurrentProductionDifference(bool bIgnoreFood, bool bOverflow);				// Exposed to Python
	int getExtraProductionDifference(int iExtra);																					// Exposed to Python

	DllExport bool canHurry(HurryTypes eHurry, bool bTestVisible = false);		// Exposed to Python
	void hurry(HurryTypes eHurry);																						// Exposed to Python

	DllExport UnitTypes getConscriptUnit();																// Exposed to Python
	DllExport int getConscriptPopulation();																// Exposed to Python
	int conscriptMinCityPopulation();																			// Exposed to Python
	int flatConscriptAngerLength();																				// Exposed to Python
	DllExport bool canConscript();																				// Exposed to Python
	void conscript();																											// Exposed to Python

	int getBonusHealth(BonusTypes eBonus);																// Exposed to Python - getBonusHealth
	int getBonusHappiness(BonusTypes eBonus);															// Exposed to Python - getBonusHappiness
	int getBonusPower(BonusTypes eBonus, bool bDirty);										// Exposed to Python 
	int getBonusYieldRateModifier(YieldTypes eIndex, BonusTypes eBonus);	// Exposed to Python 

	void processBonus(BonusTypes eBonus, int iChange);
	void processBuilding(BuildingTypes eBuilding, int iChange, bool bObsolete = false);
	void processProcess(ProcessTypes eProcess, int iChange);
	void processSpecialist(SpecialistTypes eSpecialist, int iChange);

	DllExport HandicapTypes getHandicapType();												// Exposed to Python
	DllExport CivilizationTypes getCivilizationType();								// Exposed to Python
	LeaderHeadTypes getPersonalityType();															// Exposed to Python
	ArtStyleTypes getArtStyleType() const;														// Exposed to Python
	DllExport CitySizeTypes getCitySizeType();												// Exposed to Python

	bool hasTrait(TraitTypes eTrait);																	// Exposed to Python
	bool isBarbarian();																								// Exposed to Python
	bool isHuman();																										// Exposed to Python
	DllExport bool isVisible(TeamTypes eTeam, bool bDebug);						// Exposed to Python

	DllExport bool isCapital();																				// Exposed to Python
	bool isCoastal();																									// Exposed to Python
	DllExport bool isDisorder();																			// Exposed to Python				 
	DllExport bool isHolyCity(ReligionTypes eIndex);									// Exposed to Python				
	DllExport bool isHolyCity();																			// Exposed to Python				

	int getOvercrowdingPercentAnger(int iExtra = 0);									// Exposed to Python
	int getNoMilitaryPercentAnger();																	// Exposed to Python 
	int getCulturePercentAnger();																			// Exposed to Python
	int getReligionPercentAnger();																		// Exposed to Python
	int getHurryPercentAnger();																				// Exposed to Python
	int getConscriptPercentAnger();																		// Exposed to Python
	int getWarWearinessPercentAnger();																// Exposed to Python
	int getLargestCityHappiness();																		// Exposed to Python
	int unhappyLevel(int iExtra = 0);																	// Exposed to Python 
	DllExport int happyLevel();																				// Exposed to Python				
	DllExport int angryPopulation(int iExtra = 0);										// Exposed to Python

	int visiblePopulation();
	DllExport int totalFreeSpecialists();															// Exposed to Python				 
	int extraPopulation();																						// Exposed to Python
	int extraSpecialists();																						// Exposed to Python
	int extraFreeSpecialists();																				// Exposed to Python

	DllExport int unhealthyPopulation(bool bNoAngry = false, int iExtra = 0);	// Exposed to Python
	int totalGoodBuildingHealth();																		// Exposed to Python
	DllExport int totalBadBuildingHealth();														// Exposed to Python
	DllExport int goodHealth();																				// Exposed to Python
	DllExport int badHealth(bool bNoAngry = false, int iExtra = 0);		// Exposed to Python
	DllExport int healthRate(bool bNoAngry = false, int iExtra = 0);	// Exposed to Python
	int foodConsumption(bool bNoAngry = false, int iExtra = 0);				// Exposed to Python
	DllExport int foodDifference(bool bBottom = true);								// Exposed to Python
	DllExport int growthThreshold();																	// Exposed to Python

	int productionLeft();																							// Exposed to Python
	int hurryCost(bool bExtra);																				// Exposed to Python
	DllExport int hurryGold(HurryTypes eHurry);												// Exposed to Python
	DllExport int hurryPopulation(HurryTypes eHurry);									// Exposed to Python
	int hurryProduction(HurryTypes eHurry);														// Exposed to Python
	int flatHurryAngerLength();																				// Exposed to Python
	int hurryAngerLength(HurryTypes eHurry);													// Exposed to Python
	int maxHurryPopulation();																					// Exposed to Python

	int cultureDistance(int iDX, int iDY);														// Exposed to Python
	DllExport int cultureStrength(PlayerTypes ePlayer);								// Exposed to Python					 
	DllExport int cultureGarrison(PlayerTypes ePlayer);								// Exposed to Python					 
																																		
	DllExport bool hasBuilding(BuildingTypes eIndex);									// Exposed to Python					
	DllExport bool hasActiveBuilding(BuildingTypes eIndex);						// Exposed to Python
	bool hasActiveWorldWonder();																			// Exposed to Python

	int getReligionCount();																						// Exposed to Python  

	DllExport int getID() const;																			// Exposed to Python
	DllExport int getIndex() const;
	DllExport IDInfo getIDInfo() const;
	void setID(int iID);

	DllExport int getX() const;																			// Exposed to Python
#ifdef _USRDLL
	inline int getX_INLINE() const
	{
		return m_iX;
	}
#endif
	DllExport int getY() const;																			// Exposed to Python
#ifdef _USRDLL
	inline int getY_INLINE() const
	{
		return m_iY;
	}
#endif	
	bool at(int iX, int iY);																				// Exposed to Python
	bool at(CvPlot* pPlot);																					// Exposed to Python - atPlot
	DllExport CvPlot* plot() const;																	// Exposed to Python
	CvPlotGroup* plotGroup(PlayerTypes ePlayer) const;
	bool isConnectedTo(CvCity* pCity);															// Exposed to Python
	DllExport bool isConnectedToCapital(PlayerTypes ePlayer = NO_PLAYER);			// Exposed to Python
	DllExport CvArea* area() const;																						// Exposed to Python
	CvArea* waterArea() const;																			// Exposed to Python

	DllExport CvPlot* getRallyPlot();																// Exposed to Python
	void setRallyPlot(CvPlot* pPlot);

	int getGameTurnFounded();																				// Exposed to Python
	void setGameTurnFounded(int iNewValue);

	int getGameTurnAcquired();																			// Exposed to Python
	void setGameTurnAcquired(int iNewValue);

	DllExport int getPopulation() const;														// Exposed to Python
	DllExport void setPopulation(int iNewValue);										// Exposed to Python
	DllExport void changePopulation(int iChange);										// Exposed to Python

	long getRealPopulation() const;																	// Exposed to Python

	int getHighestPopulation();																			// Exposed to Python 
	void setHighestPopulation(int iNewValue);

	int getWorkingPopulation();																			// Exposed to Python
	void changeWorkingPopulation(int iChange);														

	int getSpecialistPopulation();																	// Exposed to Python
	void changeSpecialistPopulation(int iChange);													

	int getNumGreatPeople();																				// Exposed to Python
	void changeNumGreatPeople(int iChange);															

	int getBaseGreatPeopleRate();																		// Exposed to Python
	int getGreatPeopleRate();																				// Exposed to Python
	int getTotalGreatPeopleRateModifier();													// Exposed to Python
	void changeBaseGreatPeopleRate(int iChange);										// Exposed to Python

	int getGreatPeopleRateModifier();																// Exposed to Python
	void changeGreatPeopleRateModifier(int iChange);

	DllExport int getGreatPeopleProgress();													// Exposed to Python
	void changeGreatPeopleProgress(int iChange);										// Exposed to Python

	int getNumWorldWonders();																				// Exposed to Python
	void changeNumWorldWonders(int iChange);

	int getNumTeamWonders();																				// Exposed to Python
	void changeNumTeamWonders(int iChange);

	int getNumNationalWonders();																		// Exposed to Python
	void changeNumNationalWonders(int iChange);

	int getNumBuildings();																					// Exposed to Python
	void changeNumBuildings(int iChange);

	int getGovernmentCenterCount();																	
	DllExport bool isGovernmentCenter();														// Exposed to Python
	void changeGovernmentCenterCount(int iChange);													

	DllExport int getMaintenance();																	// Exposed to Python
	void updateMaintenance();
	DllExport int calculateDistanceMaintenance();										// Exposed to Python
	DllExport int calculateNumCitiesMaintenance();									// Exposed to Python
	int calculateBaseMaintenance();																	// Exposed to Python
	DllExport int getMaintenanceModifier();													// Exposed to Python
	void changeMaintenanceModifier(int iChange);													

	int getWarWearinessModifier();																	// Exposed to Python
	void changeWarWearinessModifier(int iChange);													

	int getHealRate();																							// Exposed to Python
	void changeHealRate(int iChange);

	int getFreshWaterGoodHealth();																	// Exposed to Python
	DllExport int getFreshWaterBadHealth();													// Exposed to Python
	void updateFreshWaterHealth();

	int getFeatureGoodHealth();																			// Exposed to Python
	DllExport int getFeatureBadHealth();														// Exposed to Python
	void updateFeatureHealth();

	int getBuildingGoodHealth();																		// Exposed to Python
	int getBuildingBadHealth();																			// Exposed to Python
	int getBuildingHealth(BuildingTypes eBuilding);									// Exposed to Python
	void changeBuildingGoodHealth(int iChange);
	void changeBuildingBadHealth(int iChange);

	int getPowerGoodHealth();																				// Exposed to Python 
	DllExport int getPowerBadHealth();															// Exposed to Python 
	void updatePowerHealth();

	int getBonusGoodHealth();																				// Exposed to Python  
	DllExport int getBonusBadHealth();															// Exposed to Python 
	void changeBonusGoodHealth(int iChange);
	void changeBonusBadHealth(int iChange);

	int getMilitaryHappiness();																			// Exposed to Python
	int getMilitaryHappinessUnits();																// Exposed to Python
	void changeMilitaryHappinessUnits(int iChange);

	int getBuildingGoodHappiness();																	// Exposed to Python 
	int getBuildingBadHappiness();																	// Exposed to Python 
	int getBuildingHappiness(BuildingTypes eBuilding);							// Exposed to Python
	void changeBuildingGoodHappiness(int iChange);
	void changeBuildingBadHappiness(int iChange);

	int getExtraBuildingGoodHappiness();														// Exposed to Python
	int getExtraBuildingBadHappiness();															// Exposed to Python
	void updateExtraBuildingHappiness();

	int getFeatureGoodHappiness();																	// Exposed to Python
	int getFeatureBadHappiness();																		// Exposed to Python
	void updateFeatureHappiness();

	int getBonusGoodHappiness();																		// Exposed to Python  
	int getBonusBadHappiness();																			// Exposed to Python  
	void changeBonusGoodHappiness(int iChange);
	void changeBonusBadHappiness(int iChange);

	int getReligionGoodHappiness();																	// Exposed to Python
	int getReligionBadHappiness();																	// Exposed to Python
	int getReligionHappiness(ReligionTypes eReligion);							// Exposed to Python
	void updateReligionHappiness();

	int getExtraHappiness();																				// Exposed to Python
	void changeExtraHappiness(int iChange);													// Exposed to Python

	int getHurryAngerTimer();																				// Exposed to Python
	void changeHurryAngerTimer(int iChange);												// Exposed to Python

	int getConscriptAngerTimer();																		// Exposed to Python
	void changeConscriptAngerTimer(int iChange);										// Exposed to Python

	int getNoUnhappinessCount();
	bool isNoUnhappiness();																					// Exposed to Python
	void changeNoUnhappinessCount(int iChange);

	int getNoUnhealthyPopulationCount();
	bool isNoUnhealthyPopulation();																	// Exposed to Python
	void changeNoUnhealthyPopulationCount(int iChange);

	int getBuildingOnlyHealthyCount();
	bool isBuildingOnlyHealthy();																		// Exposed to Python
	void changeBuildingOnlyHealthyCount(int iChange);

	DllExport int getFood();																				// Exposed to Python
	void setFood(int iNewValue);																		// Exposed to Python
	void changeFood(int iChange);																		// Exposed to Python

	int getFoodKept();																							// Exposed to Python
	void setFoodKept(int iNewValue);
	void changeFoodKept(int iChange);

	int getMaxFoodKeptPercent();																		// Exposed to Python
	void changeMaxFoodKeptPercent(int iChange);

	int getOverflowProduction();																		// Exposed to Python
	void setOverflowProduction(int iNewValue);											// Exposed to Python
	void changeOverflowProduction(int iChange, int iProductionModifier);

	int getMilitaryProductionModifier();														// Exposed to Python
	void changeMilitaryProductionModifier(int iChange);												

	int getSpaceProductionModifier();																// Exposed to Python
	void changeSpaceProductionModifier(int iChange);

	int getExtraTradeRoutes();																			// Exposed to Python
	void changeExtraTradeRoutes(int iChange);

	int getTradeRouteModifier();																		// Exposed to Python
	void changeTradeRouteModifier(int iChange);

	int getBuildingDefense();																				// Exposed to Python
	void changeBuildingDefense(int iChange);

	int getFreeExperience();																				// Exposed to Python
	void changeFreeExperience(int iChange);															

	int getCurrAirlift();																						// Exposed to Python
	void setCurrAirlift(int iNewValue);
	void changeCurrAirlift(int iChange);

	int getMaxAirlift();																						// Exposed to Python
	void changeMaxAirlift(int iChange);

	int getAirModifier();																						// Exposed to Python
	void changeAirModifier(int iChange);

	int getNukeModifier();																					// Exposed to Python
	void changeNukeModifier(int iChange);

	int getFreeSpecialist();																				// Exposed to Python  
	void changeFreeSpecialist(int iChange);

	int getPowerCount();
	bool isPower();																									// Exposed to Python
	bool isAreaCleanPower();																				// Exposed to Python
	int getDirtyPowerCount();
	bool isDirtyPower();																						// Exposed to Python
	void changePowerCount(int iChange, bool bDirty);

	int getDefenseDamage();																					// Exposed to Python
	void changeDefenseDamage(int iChange);													// Exposed to Python

	int getLastDefenseDamage();																			// Exposed to Python
	void setLastDefenseDamage(int iNewValue);

	bool isBombardable(TeamTypes eTeam);														// Exposed to Python
	int getNaturalDefense();																				// Exposed to Python
	int getTotalDefense(bool bIgnoreBuilding);											// Exposed to Python
	int getDefenseModifier(bool bIgnoreBuilding);										// Exposed to Python

	DllExport int getOccupationTimer();															// Exposed to Python
	DllExport bool isOccupation();																	// Exposed to Python 
	void setOccupationTimer(int iNewValue);													// Exposed to Python
	void changeOccupationTimer(int iChange);												// Exposed to Python

	int getCitySizeBoost() const;
	void setCitySizeBoost(int iBoost);

	bool isNeverLost();																							// Exposed to Python
	void setNeverLost(bool bNewValue);															// Exposed to Python

	bool isBombarded();																							// Exposed to Python
	void setBombarded(bool bNewValue);															// Exposed to Python

	bool isDrafted();																								// Exposed to Python
	void setDrafted(bool bNewValue);																// Exposed to Python

	bool isAirliftTargeted();																				// Exposed to Python
	void setAirliftTargeted(bool bNewValue);												// Exposed to Python

	DllExport bool isWeLoveTheKingDay();														// Exposed to Python 
	void setWeLoveTheKingDay(bool bNewValue);

	DllExport bool isCitizensAutomated();														// Exposed to Python 
	void setCitizensAutomated(bool bNewValue);											// Exposed to Python 

	DllExport bool isProductionAutomated();													// Exposed to Python
	void setProductionAutomated(bool bNewValue);										// Exposed to Python 

	/* allows you to programatically specify a cities walls rather than having them be generated automagically */
	DllExport bool isWallOverride() const; 
	DllExport void setWallOverride(bool bOverride);

	DllExport bool isInfoDirty();
	DllExport void setInfoDirty(bool bNewValue);

	DllExport bool isLayoutDirty();
	DllExport void setLayoutDirty(bool bNewValue);

	DllExport PlayerTypes getOwner() const;																// Exposed to Python
#ifdef _USRDLL
	inline PlayerTypes getOwnerINLINE() const
	{
		return m_eOwner;
	}
#endif
	DllExport TeamTypes getTeam() const;																	// Exposed to Python

	PlayerTypes getPreviousOwner() const;																	// Exposed to Python
	void setPreviousOwner(PlayerTypes eNewValue);

	PlayerTypes getOriginalOwner() const;																	// Exposed to Python
	void setOriginalOwner(PlayerTypes eNewValue);

	CultureLevelTypes getCultureLevel() const;														// Exposed to Python
	DllExport int getCultureThreshold();																	// Exposed to Python
	void setCultureLevel(CultureLevelTypes eNewValue);
	void updateCultureLevel();

	int getSeaPlotYield(YieldTypes eIndex);																// Exposed to Python
	void changeSeaPlotYield(YieldTypes eIndex, int iChange);

	int getBaseYieldRate(YieldTypes eIndex);															// Exposed to Python
	int getBaseYieldRateModifier(YieldTypes eIndex, int iExtra = 0);			// Exposed to Python
	DllExport int getYieldRate(YieldTypes eIndex);												// Exposed to Python
	void setBaseYieldRate(YieldTypes eIndex, int iNewValue);
	void changeBaseYieldRate(YieldTypes eIndex, int iChange);

	int getYieldRateModifier(YieldTypes eIndex);													// Exposed to Python
	void changeYieldRateModifier(YieldTypes eIndex, int iChange);

	int getPowerYieldRateModifier(YieldTypes eIndex);											// Exposed to Python 
	void changePowerYieldRateModifier(YieldTypes eIndex, int iChange);

	int getBonusYieldRateModifier(YieldTypes eIndex);											// Exposed to Python 
	void changeBonusYieldRateModifier(YieldTypes eIndex, int iChange);

	int getTradeYield(YieldTypes eIndex);																	// Exposed to Python
	int totalTradeModifier();																							// Exposed to Python
	int calculateTradeProfit(CvCity* pCity);															// Exposed to Python
	int calculateTradeYield(YieldTypes eIndex, int iTradeProfit);					// Exposed to Python
	void setTradeYield(YieldTypes eIndex, int iNewValue);

	int getExtraSpecialistYield(YieldTypes eIndex);																				// Exposed to Python
	int getExtraSpecialistYield(YieldTypes eIndex, SpecialistTypes eSpecialist);					// Exposed to Python
	void updateExtraSpecialistYield(YieldTypes eYield);
	void updateExtraSpecialistYield();

	DllExport int getCommerceRate(CommerceTypes eIndex);									// Exposed to Python
	int getCommerceFromPercent(CommerceTypes eIndex, int iYieldRate);			// Exposed to Python
	int getBaseCommerceRate(CommerceTypes eIndex);												// Exposed to Python
	int getTotalCommerceRateModifier(CommerceTypes eIndex);								// Exposed to Python
	void updateCommerce(CommerceTypes eIndex);
	void updateCommerce();

	int getProductionToCommerceModifier(CommerceTypes eIndex);						// Exposed to Python
	void changeProductionToCommerceModifier(CommerceTypes eIndex, int iChange);

	int getBuildingCommerce(CommerceTypes eIndex);																				// Exposed to Python
	int getBuildingCommerceByBuilding(CommerceTypes eIndex, BuildingTypes eBuilding);			// Exposed to Python
	void updateBuildingCommerce();

	int getSpecialistCommerce(CommerceTypes eIndex);											// Exposed to Python
	void changeSpecialistCommerce(CommerceTypes eIndex, int iChange);			// Exposed to Python

	int getReligionCommerce(CommerceTypes eIndex);																				// Exposed to Python
	int getReligionCommerceByReligion(CommerceTypes eIndex, ReligionTypes eReligion);			// Exposed to Python
	void updateReligionCommerce(CommerceTypes eIndex);
	void updateReligionCommerce();

	int getCommerceRateModifier(CommerceTypes eIndex);										// Exposed to Python
	void changeCommerceRateModifier(CommerceTypes eIndex, int iChange);

	int getCommerceHappinessPer(CommerceTypes eIndex);										// Exposed to Python
	int getCommerceHappinessByType(CommerceTypes eIndex);									// Exposed to Python
	int getCommerceHappiness();																						// Exposed to Python
	void changeCommerceHappinessPer(CommerceTypes eIndex, int iChange);

	int getDomainFreeExperience(DomainTypes eIndex);											// Exposed to Python
	void changeDomainFreeExperience(DomainTypes eIndex, int iChange);

	int getDomainProductionModifier(DomainTypes eIndex);									// Exposed to Python
	void changeDomainProductionModifier(DomainTypes eIndex, int iChange);

	DllExport int getCulture(PlayerTypes eIndex);													// Exposed to Python
	int countTotalCulture();																							// Exposed to Python
	PlayerTypes findHighestCulture();																			// Exposed to Python
	int calculateCulturePercent(PlayerTypes eIndex);											// Exposed to Python
	int calculateTeamCulturePercent(TeamTypes eIndex);										// Exposed to Python
	void setCulture(PlayerTypes eIndex, int iNewValue, bool bPlots);			// Exposed to Python
	DllExport void changeCulture(PlayerTypes eIndex, int iChange, bool bPlots);		// Exposed to Python

	int getNumRevolts(PlayerTypes eIndex);
	void changeNumRevolts(PlayerTypes eIndex, int iChange);

	bool isTradeRoute(PlayerTypes eIndex);																	// Exposed to Python
	void setTradeRoute(PlayerTypes eIndex, bool bNewValue);

	bool isEverOwned(PlayerTypes eIndex);																		// Exposed to Python
	void setEverOwned(PlayerTypes eIndex, bool bNewValue);

	DllExport bool isRevealed(TeamTypes eIndex, bool bDebug);								// Exposed to Python
	void setRevealed(TeamTypes eIndex, bool bNewValue);											// Exposed to Python

	DllExport const CvWString getName(uint uiForm = 0) const;								// Exposed to Python
	DllExport const wchar* getNameKey() const;															// Exposed to Python
	DllExport void setName(const wchar* szNewValue, bool bFound = false);		// Exposed to Python
	void doFoundMessage();

	// Script data needs to be a narrow string for pickling in Python
	std::string getScriptData();																						// Exposed to Python
	void setScriptData(std::string szNewValue);															// Exposed to Python

	int getFreeBonus(BonusTypes eIndex);																		// Exposed to Python
	void changeFreeBonus(BonusTypes eIndex, int iChange);

	int getNumBonuses(BonusTypes eIndex);																		// Exposed to Python
	DllExport bool hasBonus(BonusTypes eIndex);															// Exposed to Python
	void changeNumBonuses(BonusTypes eIndex, int iChange);

	DllExport int getBuildingProduction(BuildingTypes eIndex);							// Exposed to Python
	void setBuildingProduction(BuildingTypes eIndex, int iNewValue);				// Exposed to Python
	void changeBuildingProduction(BuildingTypes eIndex, int iChange);				// Exposed to Python

	int getBuildingProductionTime(BuildingTypes eIndex);										// Exposed to Python
	void setBuildingProductionTime(BuildingTypes eIndex, int iNewValue);		// Exposed to Python
	void changeBuildingProductionTime(BuildingTypes eIndex, int iChange);		// Exposed to Python

	DllExport int getProjectProduction(ProjectTypes eIndex);								// Exposed to Python
	void setProjectProduction(ProjectTypes eIndex, int iNewValue);					// Exposed to Python
	void changeProjectProduction(ProjectTypes eIndex, int iChange);					// Exposed to Python

	int getBuildingOriginalOwner(BuildingTypes eIndex);											// Exposed to Python
	int getBuildingOriginalTime(BuildingTypes eIndex);											// Exposed to Python

	DllExport int getUnitProduction(UnitTypes eIndex);											// Exposed to Python
	void setUnitProduction(UnitTypes eIndex, int iNewValue);								// Exposed to Python
	void changeUnitProduction(UnitTypes eIndex, int iChange);								// Exposed to Python

	int getUnitProductionTime(UnitTypes eIndex);														// Exposed to Python
	void setUnitProductionTime(UnitTypes eIndex, int iNewValue);						// Exposed to Python
	void changeUnitProductionTime(UnitTypes eIndex, int iChange);						// Exposed to Python

	int getGreatPeopleUnitRate(UnitTypes eIndex);														// Exposed to Python
	void setGreatPeopleUnitRate(UnitTypes eIndex, int iNewValue);
	void changeGreatPeopleUnitRate(UnitTypes eIndex, int iChange);

	DllExport int getGreatPeopleUnitProgress(UnitTypes eIndex);							// Exposed to Python
	void setGreatPeopleUnitProgress(UnitTypes eIndex, int iNewValue);				// Exposed to Python
	void changeGreatPeopleUnitProgress(UnitTypes eIndex, int iChange);			// Exposed to Python

	DllExport int getSpecialistCount(SpecialistTypes eIndex);								// Exposed to Python
	void setSpecialistCount(SpecialistTypes eIndex, int iNewValue);
	void changeSpecialistCount(SpecialistTypes eIndex, int iChange);
	void alterSpecialistCount(SpecialistTypes eIndex, int iChange);					// Exposed to Python

	DllExport int getMaxSpecialistCount(SpecialistTypes eIndex);						// Exposed to Python
	bool isSpecialistValid(SpecialistTypes eIndex, int iExtra = 0);					// Exposed to Python
	void changeMaxSpecialistCount(SpecialistTypes eIndex, int iChange);

	DllExport int getForceSpecialistCount(SpecialistTypes eIndex);					// Exposed to Python
	bool isSpecialistForced();																							// Exposed to Python
	void setForceSpecialistCount(SpecialistTypes eIndex, int iNewValue);		// Exposed to Python
	void changeForceSpecialistCount(SpecialistTypes eIndex, int iChange);		// Exposed to Python

	int getFreeSpecialistCount(SpecialistTypes eIndex);											// Exposed to Python
	void setFreeSpecialistCount(SpecialistTypes eIndex, int iNewValue);			// Exposed to Python
	void changeFreeSpecialistCount(SpecialistTypes eIndex, int iChange);		// Exposed to Python

	int getReligionInfluence(ReligionTypes eIndex);													// Exposed to Python
	void changeReligionInfluence(ReligionTypes eIndex, int iChange);				// Exposed to Python

	int getCurrentStateReligionHappiness();																	// Exposed to Python
	int getStateReligionHappiness(ReligionTypes eIndex);										// Exposed to Python
	void changeStateReligionHappiness(ReligionTypes eIndex, int iChange);		// Exposed to Python

	int getUnitCombatFreeExperience(UnitCombatTypes eIndex);								// Exposed to Python
	void changeUnitCombatFreeExperience(UnitCombatTypes eIndex, int iChange);

	int getFreePromotionCount(PromotionTypes eIndex);												// Exposed to Python
	bool isFreePromotion(PromotionTypes eIndex);														// Exposed to Python
	void changeFreePromotionCount(PromotionTypes eIndex, int iChange);

	DllExport bool isWorkingPlot(int iIndex) const;													// Exposed to Python
	bool isWorkingPlot(const CvPlot* pPlot) const;													// Exposed to Python
	void setWorkingPlot(int iIndex, bool bNewValue);
	void setWorkingPlot(CvPlot* pPlot, bool bNewValue);
	void alterWorkingPlot(int iIndex);																			// Exposed to Python

	bool isHasRealBuilding(BuildingTypes eIndex);														// Exposed to Python
	DllExport void setHasRealBuilding(BuildingTypes eIndex, bool bNewValue);		// Exposed to Python
	void setHasRealBuildingTimed(BuildingTypes eIndex, bool bNewValue, bool bFirst, PlayerTypes eOriginalOwner, int iOriginalTime);

	bool isFreeBuilding(BuildingTypes eIndex);															// Exposed to Python
	void setFreeBuilding(BuildingTypes eIndex, bool bNewValue);

	DllExport bool isHasReligion(ReligionTypes eIndex);
	DllExport void setHasReligion(ReligionTypes eIndex, bool bNewValue, bool bAnnounce, bool bArrows = true);

	CvCity* getTradeCity(int iIndex);																				// Exposed to Python
	int getTradeRoutes();																										// Exposed to Python
	void clearTradeRoutes();
	void updateTradeRoutes();

	void clearOrderQueue();																														// Exposed to Python
	DllExport void pushOrder(OrderTypes eOrder, int iData1, int iData2, bool bSave, bool bPop, bool bAppend, bool bForce = false);		// Exposed to Python
	DllExport void popOrder(int iNum, bool bFinish = false, bool bChoose = false);		// Exposed to Python
	void startHeadOrder();
	void stopHeadOrder();
	DllExport int getOrderQueueLength();																		// Exposed to Python
	DllExport OrderData* getOrderFromQueue(int iIndex);											// Exposed to Python
	DllExport CLLNode<OrderData>* nextOrderQueueNode(CLLNode<OrderData>* pNode);
	DllExport CLLNode<OrderData>* headOrderQueueNode();
	CLLNode<OrderData>* tailOrderQueueNode();

	// fill the kVisible array with buildings that you want shown in city, as well as the number of generics
	// This function is called whenever CvCity::setLayoutDirty() is called
	DllExport void getVisibleBuildings(std::list<BuildingTypes>& kVisible, int& iNumGenerics);
	
	// Fill the kEffectNames array with references to effects in the CIV4EffectInfos.xml to have a
	// city play a given set of effects. This is called whenever the interface updates the city billboard
	// or when the zoom level changes
	DllExport void getVisibleEffects(ZoomLevelTypes eCurrentZoom, std::vector<const TCHAR*>& kEffectNames);


	// Billboard appearance controls
	DllExport void getCityBillboardSizeIconColors(NiColorA& kDotColor, NiColorA& kTextColor);
	DllExport const TCHAR* getCityBillboardProductionIcon();
	
	// Exposed to Python
	DllExport void setWallOverridePoints(const std::vector< std::pair<float, float> >& kPoints); /* points are given in world space ... i.e. PlotXToPointX, etc */
	DllExport const std::vector< std::pair<float, float> >& getWallOverridePoints() const;
	
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
	virtual bool AI_isEmphasizeAvoidGrowth() = 0;
	virtual bool AI_isAssignWorkDirty() = 0;
	virtual CvCity* AI_getRouteToCity() const = 0;
	virtual void AI_setAssignWorkDirty(bool bNewValue) = 0;
	virtual bool AI_isChooseProductionDirty() = 0;
	virtual void AI_setChooseProductionDirty(bool bNewValue) = 0;
	virtual bool AI_isEmphasize(EmphasizeTypes eIndex) = 0;											// Exposed to Python
	virtual void AI_setEmphasize(EmphasizeTypes eIndex, bool bNewValue) = 0;
	virtual int AI_getBestBuildValue(int iIndex) = 0;
	virtual int AI_totalBestBuildValue(CvArea* pArea) = 0;
	virtual int AI_countBestBuilds(CvArea* pArea) = 0;													// Exposed to Python
	virtual BuildTypes AI_getBestBuild(int iIndex) = 0;

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
	int m_iHealRate;
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
	int m_iMilitaryHappinessUnits;
	int m_iBuildingGoodHappiness;
	int m_iBuildingBadHappiness;
	int m_iExtraBuildingGoodHappiness;
	int m_iExtraBuildingBadHappiness;
	int m_iFeatureGoodHappiness;
	int m_iFeatureBadHappiness;
	int m_iBonusGoodHappiness;
	int m_iBonusBadHappiness;
	int m_iReligionGoodHappiness;
	int m_iReligionBadHappiness;
	int m_iExtraHappiness;
	int m_iNoUnhappinessCount;
	int m_iNoUnhealthyPopulationCount;
	int m_iBuildingOnlyHealthyCount;
	int m_iFood;
	int m_iFoodKept;
	int m_iMaxFoodKeptPercent;
	int m_iOverflowProduction;
	int m_iMilitaryProductionModifier;
	int m_iSpaceProductionModifier;
	int m_iExtraTradeRoutes;
	int m_iTradeRouteModifier;
	int m_iBuildingDefense;
	int m_iFreeExperience;
	int m_iCurrAirlift;
	int m_iMaxAirlift;
	int m_iAirModifier;
	int m_iNukeModifier;
	int m_iFreeSpecialist;
	int m_iPowerCount;
	int m_iDirtyPowerCount;
	int m_iDefenseDamage;
	int m_iLastDefenseDamage;
	int m_iOccupationTimer;
	int m_iCitySizeBoost;

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

	PlayerTypes m_eOwner;
	PlayerTypes m_ePreviousOwner;
	PlayerTypes m_eOriginalOwner;
	CultureLevelTypes m_eCultureLevel;

	int m_aiSeaPlotYield[NUM_YIELD_TYPES];
	int m_aiBaseYieldRate[NUM_YIELD_TYPES];
	int m_aiYieldRateModifier[NUM_YIELD_TYPES];
	int m_aiPowerYieldRateModifier[NUM_YIELD_TYPES];
	int m_aiBonusYieldRateModifier[NUM_YIELD_TYPES];
	int m_aiTradeYield[NUM_YIELD_TYPES];
	int m_aiExtraSpecialistYield[NUM_YIELD_TYPES];
	int m_aiCommerceRate[NUM_COMMERCE_TYPES];
	int m_aiProductionToCommerceModifier[NUM_COMMERCE_TYPES];
	int m_aiBuildingCommerce[NUM_COMMERCE_TYPES];
	int m_aiSpecialistCommerce[NUM_COMMERCE_TYPES];
	int m_aiReligionCommerce[NUM_COMMERCE_TYPES];
	int m_aiCommerceRateModifier[NUM_COMMERCE_TYPES];
	int m_aiCommerceHappinessPer[NUM_COMMERCE_TYPES];
	int m_aiDomainFreeExperience[NUM_DOMAIN_TYPES];
	int m_aiDomainProductionModifier[NUM_DOMAIN_TYPES];
	int m_aiCulture[MAX_PLAYERS];
	int m_aiNumRevolts[MAX_PLAYERS];

	bool m_abEverOwned[MAX_PLAYERS];
	bool m_abTradeRoute[MAX_PLAYERS];
	bool m_abRevealed[MAX_TEAMS];

	CvWString m_szName;
	CvString m_szScriptData;

	int* m_paiFreeBonus;
	int* m_paiNumBonuses;
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
	int* m_paiReligionInfluence;
	int* m_paiStateReligionHappiness;
	int* m_paiUnitCombatFreeExperience;
	int* m_paiFreePromotionCount;

	bool* m_pabWorkingPlot;
	bool* m_pabHasRealBuilding;
	bool* m_pabFreeBuilding;
	bool* m_pabHasReligion;

	IDInfo* m_paTradeCities;

	CLinkList<OrderData> m_orderQueue;

	std::vector< std::pair < float, float> > m_kWallOverridePoints;

	void doGrowth();
	void doCulture();
	void doPlotCulture(bool bUpdate);
	void doProduction();
	void doDecay();
	void doReligion();
	void doGreatPeople();
	void doMeltdown();

	virtual bool AI_addBestCitizen(bool bWorkers, bool bSpecialists) = 0;
	virtual bool AI_removeWorstCitizen(SpecialistTypes eIgnoreSpecialist = NO_SPECIALIST) = 0;
};

#endif
