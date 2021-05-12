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

typedef std::list<CvTalkingHeadMessage> CvMessageQueue;
typedef std::list<CvPopupInfo*> CvPopupQueue;
typedef std::list<CvDiploParameters*> CvDiploQueue;
typedef stdext::hash_map<int, int> CvTurnScoreMap;

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

	int startingPlotRange();																																									// Exposed to Python
	bool startingPlotWithinRange(CvPlot* pPlot, PlayerTypes ePlayer, int iRange, int iPass);									// Exposed to Python
	int findStartingArea();
	CvPlot* findStartingPlot();

	CvPlotGroup* initPlotGroup(CvPlot* pPlot);													

	DllExport CvCity* initCity(int iX, int iY, bool bBumpUnits = true);																																// Exposed to Python
	void acquireCity(CvCity* pCity, bool bConquest, bool bTrade);																							// Exposed to Python
	void killCities();																																												// Exposed to Python
	CvWString getNewCityName();																																								// Exposed to Python
	void getCivilizationCityName(CvWString& szBuffer, CivilizationTypes eCivilization);
	bool isCityNameValid(CvWString& szName, bool bTestDestroyed = true);

	DllExport CvUnit* initUnit(UnitTypes eUnit, int iX, int iY, UnitAITypes eUnitAI = NO_UNITAI);							// Exposed to Python
	void disbandUnit(bool bAnnounce);																																					// Exposed to Python
	void killUnits();																																													// Exposed to Python

	DllExport CvSelectionGroup* cycleSelectionGroups(CvUnit* pUnit, bool bForward, bool bWorkers, bool* pbWrap);

	bool hasTrait(TraitTypes eTrait);																																			// Exposed to Python						
	DllExport bool isHuman();																																							// Exposed to Python						
	DllExport bool isBarbarian();																																					// Exposed to Python						

	DllExport const wchar* getName(uint uiForm = 0);																											// Exposed to Python
	DllExport const wchar* getNameKey();																																	// Exposed to Python
	DllExport const wchar* getCivilizationDescription(uint uiForm = 0);																		// Exposed to Python
	DllExport const wchar* getCivilizationDescriptionKey();																								// Exposed to Python
	DllExport const wchar* getCivilizationShortDescription(uint uiForm = 0);															// Exposed to Python 
	DllExport const wchar* getCivilizationShortDescriptionKey();																					// Exposed to Python 
	DllExport const wchar* getCivilizationAdjective(uint uiForm = 0);																			// Exposed to Python
	DllExport const wchar* getCivilizationAdjectiveKey();																									// Exposed to Python
	DllExport CvWString getFlagDecal();																																		// Exposed to Python
	DllExport bool isWhiteFlag();																																					// Exposed to Python
	DllExport const wchar* getStateReligionName(uint uiForm = 0);																					// Exposed to Python
	DllExport const wchar* getStateReligionKey();																													// Exposed to Python
	const CvWString getBestAttackUnitName(uint uiForm = 0);																								// Exposed to Python
	const CvWString getWorstEnemyName();																																	// Exposed to Python
	const wchar* getBestAttackUnitKey();																																	// Exposed to Python
	DllExport ArtStyleTypes getArtStyleType();																														// Exposed to Python

	void doTurn();
	void doTurnUnits();

	void verifyCivics();

	void updatePlotGroups();

	void updateYield();
	void updateMaintenance();
	void updatePowerHealth();
	void updateExtraBuildingHappiness();
	void updateFeatureHappiness();
	void updateReligionHappiness();
	void updateExtraSpecialistYield();
	void updateCommerce(CommerceTypes eCommerce);
	void updateCommerce();
	void updateBuildingCommerce();
	void updateReligionCommerce();
	void updateCityPlotYield();
	void updateCitySight(bool bIncrement);
	void updateTradeRoutes();

	void updateTimers();

	DllExport bool hasReadyUnit(bool bAny = false);
	DllExport bool hasAutoUnit();
	DllExport bool hasBusyUnit();

	DllExport void chooseTech(int iDiscover = 0, CvWString szText = "", bool bFront = false);				// Exposed to Python

	int calculateScore(bool bFinal = false, bool bVictory = false);

	int findBestFoundValue();																																				// Exposed to Python

	DllExport int upgradeAllPrice(UnitTypes eUpgradeUnit, UnitTypes eFromUnit);

	int countReligionSpreadUnits(CvArea* pArea, ReligionTypes eReligion);														// Exposed to Python
	int countNumCoastalCities();																																		// Exposed to Python
	int countNumCoastalCitiesByArea(CvArea* pArea);																									// Exposed to Python
	int countTotalCulture();																																				// Exposed to Python
	int countOwnedBonuses(BonusTypes eBonus);																												// Exposed to Python
	int countUnimprovedBonuses(CvArea* pArea, CvPlot* pFromPlot = NULL);														// Exposed to Python
	int countCityFeatures(FeatureTypes eFeature);																										// Exposed to Python
	int countNumBuildings(BuildingTypes eBuilding);																									// Exposed to Python
	DllExport int countNumCitiesConnectedToCapital();																								// Exposed to Python
	int countPotentialForeignTradeCities(CvArea* pIgnoreArea = NULL);																// Exposed to Python
	int countPotentialForeignTradeCitiesConnected();																								// Exposed to Python

	DllExport bool canContact(PlayerTypes ePlayer);																									// Exposed to Python
	void contact(PlayerTypes ePlayer);																															// Exposed to Python
	DllExport void handleDiploEvent(DiploEventTypes eDiploEvent, PlayerTypes ePlayer, int iData1, int iData2);
	bool canTradeWith(PlayerTypes eWhoTo);																													// Exposed to Python
	DllExport bool canTradeItem(PlayerTypes eWhoTo, TradeData item, bool bTestDenial = false);			// Exposed to Python
	DllExport DenialTypes getTradeDenial(PlayerTypes eWhoTo, TradeData item);												// Exposed to Python
	bool canTradeNetworkWith(PlayerTypes ePlayer);																									// Exposed to Python
	int getNumAvailableBonuses(BonusTypes eBonus);																									// Exposed to Python
	DllExport int getNumTradeableBonuses(BonusTypes eBonus);																				// Exposed to Python
	int getNumTradeBonusImports(PlayerTypes ePlayer);																								// Exposed to Python

	bool canStopTradingWithTeam(TeamTypes eTeam);																										// Exposed to Python
	void stopTradingWithTeam(TeamTypes eTeam);																											// Exposed to Python
	void killAllDeals();																																						// Exposed to Python

	void findNewCapital();																																					// Exposed to Python 
	DllExport int getNumGovernmentCenters();																												// Exposed to Python 

	DllExport bool canRaze(CvCity* pCity);																													// Exposed to Python 
	void raze(CvCity* pCity);																																				// Exposed to Python  
	void disband(CvCity* pCity);																																		// Exposed to Python

	bool canReceiveGoody(CvPlot* pPlot, GoodyTypes eGoody, CvUnit* pUnit);													// Exposed to Python
	void receiveGoody(CvPlot* pPlot, GoodyTypes eGoody, CvUnit* pUnit);															// Exposed to Python
	void doGoody(CvPlot* pPlot, CvUnit* pUnit);																											// Exposed to Python

	DllExport bool canFound(int iX, int iY, bool bTestVisible = false);															// Exposed to Python			
	void found(int iX, int iY);																																			// Exposed to Python			

	bool canTrain(UnitTypes eUnit, bool bContinue = false, bool bTestVisible = false);										// Exposed to Python
	bool canConstruct(BuildingTypes eBuilding, bool bContinue = false, bool bTestVisible = false, bool bIgnoreCost = false);	// Exposed to Python
	bool canCreate(ProjectTypes eProject, bool bContinue = false, bool bTestVisible = false);							// Exposed to Python
	bool canMaintain(ProcessTypes eProcess, bool bContinue = false);																			// Exposed to Python
	bool isProductionMaxedUnitClass(UnitClassTypes eUnitClass);																						// Exposed to Python
	bool isProductionMaxedBuildingClass(BuildingClassTypes eBuildingClass, bool bAcquireCity = false);		// Exposed to Python
	bool isProductionMaxedProject(ProjectTypes eProject);																									// Exposed to Python
	DllExport int getProductionNeeded(UnitTypes eUnit);																										// Exposed to Python
	DllExport int getProductionNeeded(BuildingTypes eBuilding);																						// Exposed to Python
	DllExport int getProductionNeeded(ProjectTypes eProject);																							// Exposed to Python

	DllExport int getBuildingClassPrereqBuilding(BuildingTypes eBuilding, BuildingClassTypes ePrereqBuildingClass, int iExtra = 0);	// Exposed to Python
	void removeBuildingClass(BuildingClassTypes eBuildingClass);																		// Exposed to Python
	void processBuilding(BuildingTypes eBuilding, int iChange, CvArea* pArea);

	bool canBuild(const CvPlot* pPlot, BuildTypes eBuild, bool bTestEra = false, bool bTestVisible = false) const;	// Exposed to Python
	RouteTypes getBestRoute(CvPlot* pPlot = NULL) const;																						// Exposed to Python
	int getImprovementUpgradeRate() const;																													// Exposed to Python

	int calculateTotalYield(YieldTypes eYield);																											// Exposed to Python
	int calculateTotalExports(YieldTypes eYield);																										// Exposed to Python
	int calculateTotalImports(YieldTypes eYield);																										// Exposed to Python

	int calculateTotalCityHappiness();																															// Exposed to Python
	int calculateTotalCityUnhappiness();																														// Exposed to Python

	int calculateTotalCityHealthiness();																														// Exposed to Python
	int calculateTotalCityUnhealthiness();																													// Exposed to Python

	int calculateUnitCost(int& iFreeUnits, int& iFreeMilitaryUnits, int& iPaidUnits, int& iPaidMilitaryUnits, int& iBaseUnitCost, int& iMilitaryCost, int& iExtraCost);
	int calculateUnitCost();																																				// Exposed to Python
	int calculateUnitSupply(int& iPaidUnits, int& iBaseSupplyCost);																	// Exposed to Python
	int calculateUnitSupply();																																			// Exposed to Python
	int calculatePreInflatedCosts();																																// Exposed to Python
	int calculateInflationRate();																																		// Exposed to Python
	int calculateInflatedCosts();																																		// Exposed to Python

	int calculateBaseNetGold();
	int calculateBaseNetResearch(TechTypes eTech = NO_TECH);   // Exposed to Python
	int calculateResearchModifier(TechTypes eTech);   // Exposed to Python
	int calculateGoldRate();																																				// Exposed to Python
	int calculateResearchRate(TechTypes eTech = NO_TECH);																						// Exposed to Python

	bool isResearch();																																							// Exposed to Python
	DllExport bool canEverResearch(TechTypes eTech);																								// Exposed to Python
	DllExport bool canResearch(TechTypes eTech, bool bTrade = false);																// Exposed to Python
	DllExport TechTypes getCurrentResearch();																												// Exposed to Python
	bool isCurrentResearchRepeat();																																	// Exposed to Python
	bool isNoResearchAvailable();																																		// Exposed to Python
	DllExport int getResearchTurnsLeft(TechTypes eTech, bool bOverflow);														// Exposed to Python

	bool isCivic(CivicTypes eCivic);																																// Exposed to Python
	bool canDoCivics(CivicTypes eCivic);																														// Exposed to Python
	DllExport bool canRevolution(CivicTypes* paeNewCivics);																					// Exposed to Python
	DllExport void revolution(CivicTypes* paeNewCivics, bool bForce = false);												// Exposed to Python
	int getCivicPercentAnger(CivicTypes eCivic);																										// Exposed to Python

	bool canDoReligion(ReligionTypes eReligion);																										// Exposed to Python
	bool canChangeReligion();																																				// Exposed to Python
	DllExport bool canConvert(ReligionTypes eReligion);																							// Exposed to Python
	DllExport void convert(ReligionTypes eReligion);																								// Exposed to Python
	bool hasHolyCity(ReligionTypes eReligion);																											// Exposed to Python
	int countHolyCities();																																					// Exposed to Python
	void foundReligion(ReligionTypes eReligion);																										// Exposed to Python

	DllExport int getCivicAnarchyLength(CivicTypes* paeNewCivics);																	// Exposed to Python
	DllExport int getReligionAnarchyLength();																												// Exposed to Python

	DllExport int unitsRequiredForGoldenAge();																											// Exposed to Python
	int unitsGoldenAgeCapable();																																		// Exposed to Python
	DllExport int unitsGoldenAgeReady();																														// Exposed to Python
	void killGoldenAgeUnits(CvUnit* pUnitAlive);

	DllExport int greatPeopleThreshold(bool bMilitary = false);																														// Exposed to Python
	int specialistYield(SpecialistTypes eSpecialist, YieldTypes eYield);														// Exposed to Python
	int specialistCommerce(SpecialistTypes eSpecialist, CommerceTypes eCommerce);										// Exposed to Python

	CvPlot* getStartingPlot();																																			// Exposed to Python
	DllExport void setStartingPlot(CvPlot* pNewValue, bool bUpdateStartDist);												// Exposed to Python

	DllExport int getTotalPopulation();																															// Exposed to Python
	int getAveragePopulation();																																			// Exposed to Python
	void changeTotalPopulation(int iChange);
	long getRealPopulation();																																				// Exposed to Python

	int getTotalLand();																																							// Exposed to Python
	void changeTotalLand(int iChange);

	int getTotalLandScored();																																				// Exposed to Python
	void changeTotalLandScored(int iChange);

	DllExport int getGold();																																				// Exposed to Python
	DllExport void setGold(int iNewValue);																													// Exposed to Python
	DllExport void changeGold(int iChange);																													// Exposed to Python

	int getGoldPerTurn();																																						// Exposed to Python

	DllExport int getGoldenAgeTurns();																															// Exposed to Python  
	DllExport bool isGoldenAge();																																		// Exposed to Python 
	void changeGoldenAgeTurns(int iChange);																													// Exposed to Python 

	int getNumUnitGoldenAges();																																			// Exposed to Python 
	void changeNumUnitGoldenAges(int iChange);																											// Exposed to Python 

	int getAnarchyTurns();																																					// Exposed to Python
	DllExport bool isAnarchy();																																			// Exposed to Python
	void changeAnarchyTurns(int iChange);																														// Exposed to Python

	int getStrikeTurns();																																						// Exposed to Python
	void changeStrikeTurns(int iChange);

	int getMaxAnarchyTurns();																																				// Exposed to Python 
	void updateMaxAnarchyTurns();

	int getAnarchyModifier();																																				// Exposed to Python 
	void changeAnarchyModifier(int iChange);

	int getHurryModifier();																																					// Exposed to Python
	void changeHurryModifier(int iChange);

	void createGreatPeople(UnitTypes eGreatPersonUnit, bool bIncrementThreshold, bool bIncrementExperience, int iX, int iY);

	int getGreatPeopleCreated();																																		// Exposed to Python
	void incrementGreatPeopleCreated();

	int getGreatGeneralsCreated();																																		// Exposed to Python
	void incrementGreatGeneralsCreated();

	int getGreatPeopleThresholdModifier();																													// Exposed to Python
	void changeGreatPeopleThresholdModifier(int iChange);										

	int getGreatGeneralsThresholdModifier();																													// Exposed to Python
	void changeGreatGeneralsThresholdModifier(int iChange);										

	int getGreatPeopleRateModifier();																																// Exposed to Python
	void changeGreatPeopleRateModifier(int iChange);

	int getGreatGeneralRateModifier();																																// Exposed to Python
	void changeGreatGeneralRateModifier(int iChange);

	int getDomesticGreatGeneralRateModifier();																																// Exposed to Python
	void changeDomesticGreatGeneralRateModifier(int iChange);

	int getStateReligionGreatPeopleRateModifier();																									// Exposed to Python
	void changeStateReligionGreatPeopleRateModifier(int iChange);

	int getMaxGlobalBuildingProductionModifier();																										// Exposed to Python
	void changeMaxGlobalBuildingProductionModifier(int iChange);

	int getMaxTeamBuildingProductionModifier();																											// Exposed to Python 
	void changeMaxTeamBuildingProductionModifier(int iChange);

	int getMaxPlayerBuildingProductionModifier();																										// Exposed to Python
	void changeMaxPlayerBuildingProductionModifier(int iChange);

	int getFreeExperience();																																				// Exposed to Python
	void changeFreeExperience(int iChange);

	int getFeatureProductionModifier();																															// Exposed to Python
	void changeFeatureProductionModifier(int iChange);

	int getWorkerSpeedModifier();																																		// Exposed to Python
	void changeWorkerSpeedModifier(int iChange);

	int getImprovementUpgradeRateModifier() const;																									// Exposed to Python
	void changeImprovementUpgradeRateModifier(int iChange);

	int getMilitaryProductionModifier() const;																											// Exposed to Python
	void changeMilitaryProductionModifier(int iChange);

	int getSpaceProductionModifier();																																// Exposed to Python  
	void changeSpaceProductionModifier(int iChange);

	int getCityDefenseModifier();																																		// Exposed to Python
	void changeCityDefenseModifier(int iChange);

	int getNumNukeUnits();																																					// Exposed to Python
	void changeNumNukeUnits(int iChange);

	int getNumOutsideUnits();																																				// Exposed to Python
	void changeNumOutsideUnits(int iChange);

	int getBaseFreeUnits();																																					// Exposed to Python
	void changeBaseFreeUnits(int iChange);

	int getBaseFreeMilitaryUnits();																																	// Exposed to Python
	void changeBaseFreeMilitaryUnits(int iChange);

	int getFreeUnitsPopulationPercent();																														// Exposed to Python
	void changeFreeUnitsPopulationPercent(int iChange);

	int getFreeMilitaryUnitsPopulationPercent();																										// Exposed to Python
	void changeFreeMilitaryUnitsPopulationPercent(int iChange);											

	int getGoldPerUnit();																																								// Exposed to Python
	void changeGoldPerUnit(int iChange);															

	int getGoldPerMilitaryUnit();																																				// Exposed to Python
	void changeGoldPerMilitaryUnit(int iChange);

	int getExtraUnitCost();																																							// Exposed to Python 
	void changeExtraUnitCost(int iChange);

	int getNumMilitaryUnits();																																					// Exposed to Python
	void changeNumMilitaryUnits(int iChange);													

	int getHappyPerMilitaryUnit();																																			// Exposed to Python
	void changeHappyPerMilitaryUnit(int iChange);												

	int getMilitaryFoodProductionCount();														
	bool isMilitaryFoodProduction();																																		// Exposed to Python
	void changeMilitaryFoodProductionCount(int iChange);

	int getHighestUnitLevel();																																					// Exposed to Python
	void setHighestUnitLevel(int iNewValue);

	int getConscriptCount();																																						// Exposed to Python
	void setConscriptCount(int iNewValue);																															// Exposed to Python
	void changeConscriptCount(int iChange);																															// Exposed to Python

	DllExport int getMaxConscript();																																		// Exposed to Python
	void changeMaxConscript(int iChange);														

	DllExport int getOverflowResearch();																																// Exposed to Python
	void setOverflowResearch(int iNewValue);																														// Exposed to Python
	void changeOverflowResearch(int iChange);																														// Exposed to Python

	int getNoUnhealthyPopulationCount();
	bool isNoUnhealthyPopulation();																																			// Exposed to Python
	void changeNoUnhealthyPopulationCount(int iChange);

	int getExpInBorderModifier() const;
	void changeExpInBorderModifier(int iChange);

	int getBuildingOnlyHealthyCount();
	bool isBuildingOnlyHealthy();																																				// Exposed to Python
	void changeBuildingOnlyHealthyCount(int iChange);

	int getDistanceMaintenanceModifier();																																// Exposed to Python
	void changeDistanceMaintenanceModifier(int iChange);

	int getNumCitiesMaintenanceModifier();																															// Exposed to Python
	void changeNumCitiesMaintenanceModifier(int iChange);

	int getTotalMaintenance();																																					// Exposed to Python
	void changeTotalMaintenance(int iChange);

	int getUpkeepModifier();																																						// Exposed to Python
	void changeUpkeepModifier(int iChange);

	int getLevelExperienceModifier() const;																																						// Exposed to Python
	void changeLevelExperienceModifier(int iChange);

	DllExport int getExtraHealth();																																			// Exposed to Python
	void changeExtraHealth(int iChange);

	int getBuildingGoodHealth();																																				// Exposed to Python
	void changeBuildingGoodHealth(int iChange);

	int getBuildingBadHealth();																																					// Exposed to Python
	void changeBuildingBadHealth(int iChange);

	int getExtraHappiness();																																						// Exposed to Python
	void changeExtraHappiness(int iChange);

	int getBuildingHappiness();																																					// Exposed to Python
	void changeBuildingHappiness(int iChange);

	int getLargestCityHappiness();																																			// Exposed to Python
	void changeLargestCityHappiness(int iChange);

	int getWarWearinessPercentAnger();																																	// Exposed to Python 
	void updateWarWearinessPercentAnger();

	int getWarWearinessModifier();																																			// Exposed to Python
	void changeWarWearinessModifier(int iChange);

	int getFreeSpecialist();																																						// Exposed to Python
	void changeFreeSpecialist(int iChange);

	int getNoForeignTradeCount();
	bool isNoForeignTrade();																																						// Exposed to Python
	void changeNoForeignTradeCount(int iChange);

	int getCoastalTradeRoutes();																																				// Exposed to Python
	void changeCoastalTradeRoutes(int iChange);																													// Exposed to Python

	int getTradeRoutes();																																								// Exposed to Python
	void changeTradeRoutes(int iChange);																																// Exposed to Python

	DllExport int getRevolutionTimer();																																	// Exposed to Python
	void setRevolutionTimer(int iNewValue);
	void changeRevolutionTimer(int iChange);

	int getConversionTimer();																																						// Exposed to Python
	void setConversionTimer(int iNewValue);
	void changeConversionTimer(int iChange);

	int getStateReligionCount();
	bool isStateReligion();																																							// Exposed to Python
	void changeStateReligionCount(int iChange);

	int getNoNonStateReligionSpreadCount();
	DllExport bool isNoNonStateReligionSpread();																												// Exposed to Python
	void changeNoNonStateReligionSpreadCount(int iChange);

	DllExport int getStateReligionHappiness();																													// Exposed to Python
	void changeStateReligionHappiness(int iChange);

	int getNonStateReligionHappiness();																																	// Exposed to Python
	void changeNonStateReligionHappiness(int iChange);

	int getStateReligionUnitProductionModifier();																												// Exposed to Python 
	void changeStateReligionUnitProductionModifier(int iChange);

	int getStateReligionBuildingProductionModifier();																										// Exposed to Python
	void changeStateReligionBuildingProductionModifier(int iChange);																		// Exposed to Python

	int getStateReligionFreeExperience();																																// Exposed to Python
	void changeStateReligionFreeExperience(int iChange);

	DllExport CvCity* getCapitalCity();																																	// Exposed to Python
	void setCapitalCity(CvCity* pNewCapitalCity);

	int getCitiesLost();																																								// Exposed to Python
	void changeCitiesLost(int iChange);

	int getWinsVsBarbs();																																								// Exposed to Python
	void changeWinsVsBarbs(int iChange);

	DllExport int getAssets();																																					// Exposed to Python
	void changeAssets(int iChange);																																			// Exposed to Python  

	DllExport int getPower();																																						// Exposed to Python
	void changePower(int iChange);

	DllExport int getPopScore(bool bCheckVassal = true);																																				// Exposed to Python
	void changePopScore(int iChange);																																		// Exposed to Python  
	DllExport int getLandScore(bool bCheckVassal = true);																																				// Exposed to Python
	void changeLandScore(int iChange);																																	// Exposed to Python  
	DllExport int getTechScore();																																				// Exposed to Python
	void changeTechScore(int iChange);																																	// Exposed to Python  
	DllExport int getWondersScore();																																		// Exposed to Python
	void changeWondersScore(int iChange);	// Exposed to Python  

	int getCombatExperience() const; 	// Exposed to Python  
	void setCombatExperience(int iExperience);   // Exposed to Python
	void changeCombatExperience(int iChange);   // Exposed to Python

	DllExport bool isConnected();
	DllExport int getNetID();
	DllExport void setNetID(int iNetID);
	DllExport void sendReminder();

	uint getStartTime();
	DllExport void setStartTime(uint uiStartTime);
	DllExport uint getTotalTimePlayed();																																// Exposed to Python			  
																																																			
	bool isMinorCiv();																																									// Exposed to Python			
																																																			
	DllExport bool isAlive();																																						// Exposed to Python			
	DllExport bool isEverAlive();																																				// Exposed to Python			
	void setAlive(bool bNewValue);
	void verifyAlive();

	DllExport bool isTurnActive();																			
	DllExport void setTurnActive(bool bNewValue, bool bDoTurn = true);

	bool isAutoMoves();
	DllExport void setAutoMoves(bool bNewValue);
	DllExport void setTurnActiveForPbem(bool bActive);

	DllExport bool isPbemNewTurn() const;
	DllExport void setPbemNewTurn(bool bNew);

	bool isEndTurn();
	DllExport void setEndTurn(bool bNewValue);

	DllExport bool isTurnDone();

	bool isExtendedGame();																																			// Exposed to Python					
	DllExport void makeExtendedGame();																													
																																															
	bool isFoundedFirstCity();																																	// Exposed to Python					
	void setFoundedFirstCity(bool bNewValue);																										
																																															
	DllExport bool isStrike();																																	// Exposed to Python					
	void setStrike(bool bNewValue);																															
																																															
	DllExport PlayerTypes getID() const;																												// Exposed to Python					
																																															
	DllExport HandicapTypes getHandicapType();																									// Exposed to Python					
																																															
	DllExport CivilizationTypes getCivilizationType();																					// Exposed to Python					
																																															
	DllExport LeaderHeadTypes getLeaderType();																									// Exposed to Python					
																																															
	LeaderHeadTypes getPersonalityType();																												// Exposed to Python									
	void setPersonalityType(LeaderHeadTypes eNewValue);																					// Exposed to Python									
																																																				
	DllExport EraTypes getCurrentEra() const;																										// Exposed to Python									
	void setCurrentEra(EraTypes eNewValue);																											
																																															
	ReligionTypes getLastStateReligion();																												
	DllExport ReligionTypes getStateReligion();																									// Exposed to Python					
	void setLastStateReligion(ReligionTypes eNewValue);																					// Exposed to Python					
																																															
	DllExport TeamTypes getTeam() const;																												// Exposed to Python					
	void setTeam(TeamTypes eTeam);																															
																																																							
	DllExport PlayerColorTypes getPlayerColor();																								// Exposed to Python									
	DllExport int getPlayerTextColorR();																												// Exposed to Python								
	DllExport int getPlayerTextColorG();																												// Exposed to Python									
	DllExport int getPlayerTextColorB();																												// Exposed to Python									
	DllExport int getPlayerTextColorA();																												// Exposed to Python									
																																									
	int getSeaPlotYield(YieldTypes eIndex);																											// Exposed to Python
	void changeSeaPlotYield(YieldTypes eIndex, int iChange);

	int getYieldRateModifier(YieldTypes eIndex);																								// Exposed to Python
	void changeYieldRateModifier(YieldTypes eIndex, int iChange);

	int getCapitalYieldRateModifier(YieldTypes eIndex);																					// Exposed to Python
	void changeCapitalYieldRateModifier(YieldTypes eIndex, int iChange);

	int getExtraYieldThreshold(YieldTypes eIndex);																							// Exposed to Python
	void updateExtraYieldThreshold(YieldTypes eIndex);

	int getTradeYieldModifier(YieldTypes eIndex);																								// Exposed to Python
	void changeTradeYieldModifier(YieldTypes eIndex, int iChange);

	int getFreeCityCommerce(CommerceTypes eIndex);																							// Exposed to Python
	void changeFreeCityCommerce(CommerceTypes eIndex, int iChange);

	int getCommercePercent(CommerceTypes eIndex);																								// Exposed to Python
	void setCommercePercent(CommerceTypes eIndex, int iNewValue);																// Exposed to Python
	DllExport void changeCommercePercent(CommerceTypes eIndex, int iChange);										// Exposed to Python

	int getCommerceRate(CommerceTypes eIndex);																									// Exposed to Python
	void changeCommerceRate(CommerceTypes eIndex, int iChange);

	int getCommerceRateModifier(CommerceTypes eIndex);																					// Exposed to Python
	void changeCommerceRateModifier(CommerceTypes eIndex, int iChange);

	int getCapitalCommerceRateModifier(CommerceTypes eIndex);																		// Exposed to Python
	void changeCapitalCommerceRateModifier(CommerceTypes eIndex, int iChange);

	int getStateReligionBuildingCommerce(CommerceTypes eIndex);																	// Exposed to Python
	void changeStateReligionBuildingCommerce(CommerceTypes eIndex, int iChange);

	int getSpecialistExtraCommerce(CommerceTypes eIndex);																				// Exposed to Python
	void changeSpecialistExtraCommerce(CommerceTypes eIndex, int iChange);

	int getCommerceFlexibleCount(CommerceTypes eIndex);
	bool isCommerceFlexible(CommerceTypes eIndex);																							// Exposed to Python
	void changeCommerceFlexibleCount(CommerceTypes eIndex, int iChange);

	int getGoldPerTurnByPlayer(PlayerTypes eIndex);																							// Exposed to Python
	void changeGoldPerTurnByPlayer(PlayerTypes eIndex, int iChange);

	bool isFeatAccomplished(FeatTypes eIndex);																									// Exposed to Python
	void setFeatAccomplished(FeatTypes eIndex, bool bNewValue);																	// Exposed to Python

	DllExport bool isOption(PlayerOptionTypes eIndex) const;																		// Exposed to Python
	DllExport void setOption(PlayerOptionTypes eIndex, bool bNewValue);													// Exposed to Python

	DllExport bool isPlayable() const;
	DllExport void setPlayable(bool bNewValue);

	int getBonusExport(BonusTypes eIndex);																											// Exposed to Python
	void changeBonusExport(BonusTypes eIndex, int iChange);

	int getBonusImport(BonusTypes eIndex);																											// Exposed to Python
	void changeBonusImport(BonusTypes eIndex, int iChange);

	int getImprovementCount(ImprovementTypes eIndex);																						// Exposed to Python
	void changeImprovementCount(ImprovementTypes eIndex, int iChange);

	int getFreeBuildingCount(BuildingTypes eIndex);
	bool isBuildingFree(BuildingTypes eIndex);																									// Exposed to Python
	void changeFreeBuildingCount(BuildingTypes eIndex, int iChange);

	int getExtraBuildingHappiness(BuildingTypes eIndex);																				// Exposed to Python
	void changeExtraBuildingHappiness(BuildingTypes eIndex, int iChange);

	int getFeatureHappiness(FeatureTypes eIndex);																								// Exposed to Python
	void changeFeatureHappiness(FeatureTypes eIndex, int iChange);

	int getUnitClassCount(UnitClassTypes eIndex);																								// Exposed to Python
	bool isUnitClassMaxedOut(UnitClassTypes eIndex, int iExtra = 0);														// Exposed to Python
	void changeUnitClassCount(UnitClassTypes eIndex, int iChange);
	int getUnitClassMaking(UnitClassTypes eIndex);																							// Exposed to Python
	void changeUnitClassMaking(UnitClassTypes eIndex, int iChange);
	int getUnitClassCountPlusMaking(UnitClassTypes eIndex);																			// Exposed to Python

	int getBuildingClassCount(BuildingClassTypes eIndex);																				// Exposed to Python
	bool isBuildingClassMaxedOut(BuildingClassTypes eIndex, int iExtra = 0);										// Exposed to Python
	void changeBuildingClassCount(BuildingClassTypes eIndex, int iChange);
	int getBuildingClassMaking(BuildingClassTypes eIndex);																			// Exposed to Python 
	void changeBuildingClassMaking(BuildingClassTypes eIndex, int iChange);
	int getBuildingClassCountPlusMaking(BuildingClassTypes eIndex);															// Exposed to Python

	int getHurryCount(HurryTypes eIndex);																												// Exposed to Python
	DllExport bool canHurry(HurryTypes eIndex);																									// Exposed to Python
	void changeHurryCount(HurryTypes eIndex, int iChange);

	int getSpecialBuildingNotRequiredCount(SpecialBuildingTypes eIndex);												// Exposed to Python
	bool isSpecialBuildingNotRequired(SpecialBuildingTypes eIndex);															// Exposed to Python
	void changeSpecialBuildingNotRequiredCount(SpecialBuildingTypes eIndex, int iChange);

	int getHasCivicOptionCount(CivicOptionTypes eIndex);
	bool isHasCivicOption(CivicOptionTypes eIndex);																							// Exposed to Python
	void changeHasCivicOptionCount(CivicOptionTypes eIndex, int iChange);

	int getNoCivicUpkeepCount(CivicOptionTypes eIndex);
	bool isNoCivicUpkeep(CivicOptionTypes eIndex);																							// Exposed to Python
	void changeNoCivicUpkeepCount(CivicOptionTypes eIndex, int iChange);

	int getHasReligionCount(ReligionTypes eIndex);																							// Exposed to Python
	int countTotalHasReligion();																																// Exposed to Python
	int findHighestHasReligionCount();																													// Exposed to Python
	void changeHasReligionCount(ReligionTypes eIndex, int iChange);

	int getUpkeepCount(UpkeepTypes eIndex);																											// Exposed to Python
	void changeUpkeepCount(UpkeepTypes eIndex, int iChange);

	int getSpecialistValidCount(SpecialistTypes eIndex);
	DllExport bool isSpecialistValid(SpecialistTypes eIndex);																		// Exposed to Python					
	void changeSpecialistValidCount(SpecialistTypes eIndex, int iChange);												
																																															
	DllExport bool isResearchingTech(TechTypes eIndex);																					// Exposed to Python					
	void setResearchingTech(TechTypes eIndex, bool bNewValue);																	
																																															
	DllExport bool isTributePaid(PlayerTypes eMaster) const;
	void setTributePaid(PlayerTypes eMaster, bool bNewValue);

	DllExport CivicTypes getCivics(CivicOptionTypes eIndex);																		// Exposed to Python					
	int getSingleCivicUpkeep(CivicTypes eCivic, bool bIgnoreAnarchy = false);										// Exposed to Python					
	int getCivicUpkeep(CivicTypes* paeCivics = NULL, bool bIgnoreAnarchy = false);							// Exposed to Python					
	void setCivics(CivicOptionTypes eIndex, CivicTypes eNewValue);															// Exposed to Python					

	int getSpecialistExtraYield(SpecialistTypes eIndex1, YieldTypes eIndex2);										// Exposed to Python
	void changeSpecialistExtraYield(SpecialistTypes eIndex1, YieldTypes eIndex2, int iChange);

	int getImprovementYieldChange(ImprovementTypes eIndex1, YieldTypes eIndex2);								// Exposed to Python
	void changeImprovementYieldChange(ImprovementTypes eIndex1, YieldTypes eIndex2, int iChange);

	void updateGroupCycle(CvUnit* pUnit);
	void removeGroupCycle(int iID);
	CLLNode<int>* deleteGroupCycleNode(CLLNode<int>* pNode);
	CLLNode<int>* nextGroupCycleNode(CLLNode<int>* pNode);
	CLLNode<int>* previousGroupCycleNode(CLLNode<int>* pNode);
	CLLNode<int>* headGroupCycleNode();
	CLLNode<int>* tailGroupCycleNode();

	int findPathLength(TechTypes eTech, bool bCost = true);																			// Exposed to Python
	int getQueuePosition(TechTypes eTech);																											// Exposed to Python
	DllExport void clearResearchQueue();																												// Exposed to Python
	DllExport bool pushResearch(TechTypes eTech, bool bClear = false);													// Exposed to Python
	void popResearch(TechTypes eTech);																													// Exposed to Python
	int getLengthResearchQueue();																																// Exposed to Python
	CLLNode<TechTypes>* nextResearchQueueNode(CLLNode<TechTypes>* pNode);
	CLLNode<TechTypes>* headResearchQueueNode();
	CLLNode<TechTypes>* tailResearchQueueNode();

	void addCityName(const CvWString& szName);																									// Exposed to Python
	int getNumCityNames() const;																																// Exposed to Python
	CvWString getCityName(int iIndex) const;																										// Exposed to Python
	CLLNode<CvWString>* nextCityNameNode(CLLNode<CvWString>* pNode);
	CLLNode<CvWString>* headCityNameNode();

	// plot groups iteration
	CvPlotGroup* firstPlotGroup(int *pIterIdx, bool bRev=false);
	CvPlotGroup* nextPlotGroup(int *pIterIdx, bool bRev=false);
	int getNumPlotGroups();
	CvPlotGroup* getPlotGroup(int iID);
	CvPlotGroup* addPlotGroup();
	void deletePlotGroup(int iID);

	// city iteration
	DllExport CvCity* firstCity(int *pIterIdx, bool bRev=false);																// Exposed to Python					
	DllExport CvCity* nextCity(int *pIterIdx, bool bRev=false);																	// Exposed to Python					
	DllExport int getNumCities();																																// Exposed to Python					
	DllExport CvCity* getCity(int iID);																													// Exposed to Python					
	CvCity* addCity();																																					
	void deleteCity(int iID);																																		
																																															
	// unit iteration																																						
	DllExport CvUnit* firstUnit(int *pIterIdx, bool bRev=false);																// Exposed to Python					
	DllExport CvUnit* nextUnit(int *pIterIdx, bool bRev=false);																	// Exposed to Python					
	DllExport int getNumUnits();																																// Exposed to Python					
	DllExport CvUnit* getUnit(int iID);																													// Exposed to Python					
	CvUnit* addUnit();																																					
	void deleteUnit(int iID);																																		
																																															
	// selection groups iteration																																
	DllExport CvSelectionGroup* firstSelectionGroup(int *pIterIdx, bool bRev=false);						// Exposed to Python					
	DllExport CvSelectionGroup* nextSelectionGroup(int *pIterIdx, bool bRev=false);							// Exposed to Python					
	int getNumSelectionGroups();																																// Exposed to Python
	CvSelectionGroup* getSelectionGroup(int iID);																								// Exposed to Python
	CvSelectionGroup* addSelectionGroup();
	void deleteSelectionGroup(int iID);

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

	int getScoreHistory(int iTurn) const;																								// Exposed to Python
	void updateScoreHistory(int iTurn, int iBestScore);

	int getEconomyHistory(int iTurn) const;																							// Exposed to Python
	void updateEconomyHistory(int iTurn, int iBestEconomy);
	int getIndustryHistory(int iTurn) const;																						// Exposed to Python
	void updateIndustryHistory(int iTurn, int iBestIndustry);
	int getAgricultureHistory(int iTurn) const;																					// Exposed to Python
	void updateAgricultureHistory(int iTurn, int iBestAgriculture);
	int getPowerHistory(int iTurn) const;																								// Exposed to Python
	void updatePowerHistory(int iTurn, int iBestPower);
	int getCultureHistory(int iTurn) const;																							// Exposed to Python
	void updateCultureHistory(int iTurn, int iBestCulture);

	// Script data needs to be a narrow string for pickling in Python
	std::string getScriptData() const;																									// Exposed to Python
	void setScriptData(std::string szNewValue);																					// Exposed to Python

	DllExport const CvString& getPbemEmailAddress() const;
	DllExport void setPbemEmailAddress(const char* szAddress);
	DllExport const CvString& getSmtpHost() const;
	DllExport void setSmtpHost(const char* szHost);

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
	virtual int AI_foundValue(int iX, int iY, int iMinUnitRange = -1, bool bStartingLoc = false) = 0; // Exposed to Python
	virtual bool AI_isCommercePlot(CvPlot* pPlot) = 0;
	virtual int AI_getPlotDanger(CvPlot* pPlot, int iRange = -1, bool bTestMoves = true) = 0;
	virtual bool AI_isFinancialTrouble() = 0;																											// Exposed to Python
	virtual TechTypes AI_bestTech(int iMaxPathLength = 1, bool bIgnoreCost = false, bool bAsync = false, TechTypes eIgnoreTech = NO_TECH, AdvisorTypes eIgnoreAdvisor = NO_ADVISOR) = 0;
	virtual void AI_chooseFreeTech() = 0;
	virtual void AI_chooseResearch() = 0;
	virtual bool AI_isWillingToTalk(PlayerTypes ePlayer) = 0;
	virtual bool AI_demandRebukedSneak(PlayerTypes ePlayer) = 0;
	virtual bool AI_demandRebukedWar(PlayerTypes ePlayer) = 0;																		// Exposed to Python
	virtual AttitudeTypes AI_getAttitude(PlayerTypes ePlayer, bool bForced = true) = 0;																// Exposed to Python
	virtual int AI_diploVote(VoteTypes eVote) = 0;
	virtual int AI_dealVal(PlayerTypes ePlayer, const CLinkList<TradeData>* pList, bool bIgnoreAnnual = false) = 0;
	virtual bool AI_considerOffer(PlayerTypes ePlayer, const CLinkList<TradeData>* pTheirList, const CLinkList<TradeData>* pOurList) = 0;
	virtual bool AI_counterPropose(PlayerTypes ePlayer, const CLinkList<TradeData>* pTheirList, const CLinkList<TradeData>* pOurList, CLinkList<TradeData>* pTheirInventory, CLinkList<TradeData>* pOurInventory, CLinkList<TradeData>* pTheirCounter, CLinkList<TradeData>* pOurCounter) = 0;
	virtual int AI_bonusVal(BonusTypes eBonus) = 0;
	virtual int AI_bonusTradeVal(BonusTypes eBonus, PlayerTypes ePlayer) = 0;
	virtual DenialTypes AI_bonusTrade(BonusTypes eBonus, PlayerTypes ePlayer) = 0;
	virtual int AI_cityTradeVal(CvCity* pCity) = 0;
	virtual DenialTypes AI_cityTrade(CvCity* pCity, PlayerTypes ePlayer) = 0;
	virtual DenialTypes AI_stopTradingTrade(TeamTypes eTradeTeam, PlayerTypes ePlayer) = 0;
	virtual DenialTypes AI_civicTrade(CivicTypes eCivic, PlayerTypes ePlayer) = 0;
	virtual DenialTypes AI_religionTrade(ReligionTypes eReligion, PlayerTypes ePlayer) = 0;
	virtual int AI_unitValue(UnitTypes eUnit, UnitAITypes eUnitAI, CvArea* pArea) = 0;						// Exposed to Python
	virtual int AI_totalUnitAIs(UnitAITypes eUnitAI) = 0;																					// Exposed to Python
	virtual int AI_totalAreaUnitAIs(CvArea* pArea, UnitAITypes eUnitAI) = 0;											// Exposed to Python
	virtual int AI_totalWaterAreaUnitAIs(CvArea* pArea, UnitAITypes eUnitAI) = 0;									// Exposed to Python
	virtual int AI_plotTargetMissionAIs(CvPlot* pPlot, MissionAITypes eMissionAI, CvSelectionGroup* pSkipSelectionGroup = NULL, int iRange = 0) = 0;
	virtual int AI_unitTargetMissionAIs(CvUnit* pUnit, MissionAITypes eMissionAI, CvSelectionGroup* pSkipSelectionGroup = NULL) = 0;
	virtual int AI_civicValue(CivicTypes eCivic) = 0;
	virtual int AI_getNumAIUnits(UnitAITypes eIndex) = 0;																					// Exposed to Python
	virtual void AI_changePeacetimeTradeValue(PlayerTypes eIndex, int iChange) = 0;
	virtual void AI_changePeacetimeGrantValue(PlayerTypes eIndex, int iChange) = 0;
	virtual int AI_getAttitudeExtra(PlayerTypes eIndex) = 0;																			// Exposed to Python
	virtual void AI_setAttitudeExtra(PlayerTypes eIndex, int iNewValue) = 0;											// Exposed to Python
	virtual void AI_changeAttitudeExtra(PlayerTypes eIndex, int iChange) = 0;											// Exposed to Python
	virtual void AI_setFirstContact(PlayerTypes eIndex, bool bNewValue) = 0;
	virtual int AI_getMemoryCount(PlayerTypes eIndex1, MemoryTypes eIndex2) = 0;
	virtual void AI_changeMemoryCount(PlayerTypes eIndex1, MemoryTypes eIndex2, int iChange) = 0;
	virtual void AI_doCommerce() = 0;
	
protected:

	int m_iStartingX;
	int m_iStartingY;
	int m_iTotalPopulation;
	int m_iTotalLand;
	int m_iTotalLandScored;
	int m_iGold;
	int m_iGoldPerTurn;
	int m_iGoldenAgeTurns;
	int m_iNumUnitGoldenAges;
	int m_iStrikeTurns;
	int m_iAnarchyTurns;
	int m_iMaxAnarchyTurns;
	int m_iAnarchyModifier;
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

	PlayerTypes m_eID;
	LeaderHeadTypes m_ePersonalityType;
	EraTypes m_eCurrentEra;
	ReligionTypes m_eLastStateReligion;

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

	bool* m_abFeatAccomplished;
	bool* m_abOptions;

	CvString m_szScriptData;

	int* m_paiBonusExport;
	int* m_paiBonusImport;
	int* m_paiImprovementCount;
	int* m_paiFreeBuildingCount;
	int* m_paiExtraBuildingHappiness;
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
	int* m_paiUpkeepCount;
	int* m_paiSpecialistValidCount;

	bool* m_pabResearchingTech;
	bool* m_pabTributePaid;

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

	CvMessageQueue m_listGameMessages; 
	CvPopupQueue m_listPopups;
	CvDiploQueue m_listDiplomacy; 

	CvTurnScoreMap m_mapScoreHistory;
	CvTurnScoreMap m_mapEconomyHistory;
	CvTurnScoreMap m_mapIndustryHistory;
	CvTurnScoreMap m_mapAgricultureHistory;
	CvTurnScoreMap m_mapPowerHistory;
	CvTurnScoreMap m_mapCultureHistory;

	void doGold();
	void doResearch();
	void doWarnings();

	void verifyGoldCommercePercent();

	void processCivics(CivicTypes eCivic, int iChange);

	// for serialization
	virtual void read(FDataStreamBase* pStream);
	virtual void write(FDataStreamBase* pStream);

};

#endif
