// team.h

#ifndef CIV4_TEAM_H
#define CIV4_TEAM_H

//#include "CvEnums.h"

class CvArea;

class CvTeam
{

public:

	CvTeam();
	virtual ~CvTeam();

	DllExport void init(TeamTypes eID);
	DllExport void reset(TeamTypes eID = (TeamTypes)0, bool bConstructorCall = false);

protected:

	void uninit();

public:

	void addTeam(TeamTypes eTeam);																								// Exposed to Python
	void shareItems(TeamTypes eTeam);
	void shareCounters(TeamTypes eTeam);
	void processBuilding(BuildingTypes eBuilding, int iChange);

	void doTurn();

	void updateYield();
	void updatePowerHealth();
	void updateCommerce();

	bool canChangeWarPeace(TeamTypes eTeam);																			// Exposed to Python
	DllExport bool canDeclareWar(TeamTypes eTeam);																// Exposed to Python
	DllExport void declareWar(TeamTypes eTeam, bool bNewDiplo);										// Exposed to Python
	DllExport void makePeace(TeamTypes eTeam);																		// Exposed to Python
	bool canContact(TeamTypes eTeam);																							// Exposed to Python
	void meet(TeamTypes eTeam, bool bNewDiplo);																		// Exposed to Python
	void signOpenBorders(TeamTypes eTeam);																				// Exposed to Python
	void signDefensivePact(TeamTypes eTeam);																			// Exposed to Python

	int getAssets();																															// Exposed to Python
	int getPower();																																// Exposed to Python
	int getDefensivePower();																											// Exposed to Python
	int getNumNukeUnits();																												// Exposed to Python

	int getAtWarCount(bool bIgnoreMinors);																				// Exposed to Python
	int getWarPlanCount(WarPlanTypes eWarPlan, bool bIgnoreMinors);								// Exposed to Python
	int getAnyWarPlanCount(bool bIgnoreMinors);																		// Exposed to Python
	int getChosenWarCount(bool bIgnoreMinors);																		// Exposed to Python
	int getHasMetCivCount(bool bIgnoreMinors);																		// Exposed to Python
	bool hasMetHuman();																														// Exposed to Python
	int getDefensivePactCount();																									// Exposed to Python

	int getUnitClassMaking(UnitClassTypes eUnitClass);														// Exposed to Python
	int getUnitClassCountPlusMaking(UnitClassTypes eIndex);												// Exposed to Python
	int getBuildingClassMaking(BuildingClassTypes eBuildingClass);								// Exposed to Python
	int getBuildingClassCountPlusMaking(BuildingClassTypes eIndex);								// Exposed to Python
	int getHasReligionCount(ReligionTypes eReligion);															// Exposed to Python

	int countTotalCulture();																											// Exposed to Python

	int countNumUnitsByArea(CvArea* pArea);																				// Exposed to Python
	int countNumCitiesByArea(CvArea* pArea);																			// Exposed to Python
	int countTotalPopulationByArea(CvArea* pArea);																// Exposed to Python
	int countPowerByArea(CvArea* pArea);																					// Exposed to Python
	int countEnemyPowerByArea(CvArea* pArea);																			// Exposed to Python
	int countNumAIUnitsByArea(CvArea* pArea, UnitAITypes eUnitAI);								// Exposed to Python
	int countEnemyDangerByArea(CvArea* pArea);																		// Exposed to Python

	DllExport int getResearchCost(TechTypes eTech);																// Exposed to Python
	DllExport int getResearchLeft(TechTypes eTech);																// Exposed to Python

	bool hasHolyCity(ReligionTypes eReligion);																		// Exposed to Python

	bool isHuman();																																// Exposed to Python
	bool isBarbarian();																														// Exposed to Python
	bool isMinorCiv();																														// Exposed to Python
	PlayerTypes getLeaderID();																										// Exposed to Python
	PlayerTypes getSecretaryID();																									// Exposed to Python
	HandicapTypes getHandicapType();																							// Exposed to Python
	DllExport CvWString getName();																								// Exposed to Python

	DllExport int getNumMembers();																								// Exposed to Python
	DllExport void changeNumMembers(int iChange);

	int getAliveCount();
	DllExport int isAlive();																											// Exposed to Python
	void changeAliveCount(int iChange);

	int getEverAliveCount();
	int isEverAlive();																														// Exposed to Python
	void changeEverAliveCount(int iChange);

	int getNumCities();																														// Exposed to Python
	void changeNumCities(int iChange);							

	int getTotalPopulation();																											// Exposed to Python
	void changeTotalPopulation(int iChange);	

	int getTotalLand();																														// Exposed to Python  
	void changeTotalLand(int iChange);														

	int getNukeInterception();																										// Exposed to Python
	void changeNukeInterception(int iChange);																			// Exposed to Python

	int getForceTeamVoteEligibilityCount();																				// Exposed to Python	
	bool isForceTeamVoteEligible();																								// Exposed to Python	
	void changeForceTeamVoteEligibilityCount(int iChange);												// Exposed to Python	
																																								
	int getExtraWaterSeeFromCount();																							// Exposed to Python	
	bool isExtraWaterSeeFrom();																										// Exposed to Python	
	void changeExtraWaterSeeFromCount(int iChange);																// Exposed to Python	
																																								
	int getMapTradingCount();																											// Exposed to Python	
	bool isMapTrading();																													// Exposed to Python	
	void changeMapTradingCount(int iChange);																			// Exposed to Python	
																																								
	int getTechTradingCount();																										// Exposed to Python	
	bool isTechTrading();																													// Exposed to Python	
	void changeTechTradingCount(int iChange);																			// Exposed to Python	
																																								
	int getGoldTradingCount();																										// Exposed to Python	
	bool isGoldTrading();																													// Exposed to Python	
	void changeGoldTradingCount(int iChange);																			// Exposed to Python	
																																								
	int getOpenBordersTradingCount();																							// Exposed to Python	
	DllExport bool isOpenBordersTrading();																				// Exposed to Python	
	void changeOpenBordersTradingCount(int iChange);															// Exposed to Python	
																																								
	int getDefensivePactTradingCount();																						// Exposed to Python	
	bool isDefensivePactTrading();																								// Exposed to Python						
	void changeDefensivePactTradingCount(int iChange);														// Exposed to Python	
																																									
	int getPermanentAllianceTradingCount();																				// Exposed to Python	
	bool isPermanentAllianceTrading();																						// Exposed to Python						
	void changePermanentAllianceTradingCount(int iChange);												// Exposed to Python	
																																									
	int getBridgeBuildingCount();																									// Exposed to Python	
	DllExport bool isBridgeBuilding();																						// Exposed to Python						
	void changeBridgeBuildingCount(int iChange);																	// Exposed to Python	
																																								
	int getIrrigationCount();																											// Exposed to Python	
	DllExport bool isIrrigation();																								// Exposed to Python	
	void changeIrrigationCount(int iChange);																			// Exposed to Python	
																																								
	int getIgnoreIrrigationCount();																								// Exposed to Python	
	DllExport bool isIgnoreIrrigation();																					// Exposed to Python	
	void changeIgnoreIrrigationCount(int iChange);																// Exposed to Python	
																																								
	int getWaterWorkCount();																											// Exposed to Python	
	DllExport bool isWaterWork();																									// Exposed to Python	
	void changeWaterWorkCount(int iChange);																				// Exposed to Python	
																																								
	DllExport bool isMapCentering();																							// Exposed to Python	
	void setMapCentering(bool bNewValue);																					// Exposed to Python	
																																								
	TeamTypes getID() const;																											// Exposed to Python	

	int getStolenVisibilityTimer(TeamTypes eIndex);
	bool isStolenVisibility(TeamTypes eIndex);																		// Exposed to Python
	void setStolenVisibilityTimer(TeamTypes eIndex, int iNewValue);
	void changeStolenVisibilityTimer(TeamTypes eIndex, int iChange);

	int getWarWeariness(TeamTypes eIndex);																				// Exposed to Python
	void setWarWeariness(TeamTypes eIndex, int iNewValue);												// Exposed to Python
	void changeWarWeariness(TeamTypes eIndex, int iChange);												// Exposed to Python

	int getTechShareCount(int iIndex);																						// Exposed to Python
	bool isTechShare(int iIndex);																									// Exposed to Python
	void changeTechShareCount(int iIndex, int iChange);														// Exposed to Python

	int getCommerceFlexibleCount(CommerceTypes eIndex);														// Exposed to Python
	bool isCommerceFlexible(CommerceTypes eIndex);																// Exposed to Python
	void changeCommerceFlexibleCount(CommerceTypes eIndex, int iChange);					// Exposed to Python

	int getExtraMoves(DomainTypes eIndex);																				// Exposed to Python
	DllExport void changeExtraMoves(DomainTypes eIndex, int iChange);							// Exposed to Python

	DllExport bool isHasMet(TeamTypes eIndex);																		// Exposed to Python
	void makeHasMet(TeamTypes eIndex, bool bNewDiplo);

	DllExport bool isAtWar(TeamTypes eIndex);																			// Exposed to Python
	DllExport void setAtWar(TeamTypes eIndex, bool bNewValue);

	bool isPermanentWarPeace(TeamTypes eIndex);																		// Exposed to Python
	void setPermanentWarPeace(TeamTypes eIndex, bool bNewValue);									// Exposed to Python

	DllExport bool isFreeTrade(TeamTypes eIndex);																	// Exposed to Python
	DllExport bool isOpenBorders(TeamTypes eIndex);																// Exposed to Python
	void setOpenBorders(TeamTypes eIndex, bool bNewValue);

	DllExport bool isDefensivePact(TeamTypes eIndex);															// Exposed to Python
	void setDefensivePact(TeamTypes eIndex, bool bNewValue);

	DllExport bool isForcePeace(TeamTypes eIndex);																// Exposed to Python
	void setForcePeace(TeamTypes eIndex, bool bNewValue);

	int getRouteChange(RouteTypes eIndex);																				// Exposed to Python
	void changeRouteChange(RouteTypes eIndex, int iChange);												// Exposed to Python

	DllExport int getProjectCount(ProjectTypes eIndex);														// Exposed to Python
	bool isProjectMaxedOut(ProjectTypes eIndex, int iExtra = 0);									// Exposed to Python
	void changeProjectCount(ProjectTypes eIndex, int iChange);										// Exposed to Python

	int getProjectMaking(ProjectTypes eIndex);																		// Exposed to Python
	void changeProjectMaking(ProjectTypes eIndex, int iChange);

	int getUnitClassCount(UnitClassTypes eIndex);																	// Exposed to Python
	bool isUnitClassMaxedOut(UnitClassTypes eIndex, int iExtra = 0);							// Exposed to Python
	void changeUnitClassCount(UnitClassTypes eIndex, int iChange);

	int getBuildingClassCount(BuildingClassTypes eIndex);													// Exposed to Python
	bool isBuildingClassMaxedOut(BuildingClassTypes eIndex, int iExtra = 0);			// Exposed to Python
	void changeBuildingClassCount(BuildingClassTypes eIndex, int iChange);

	int getObsoleteBuildingCount(BuildingTypes eIndex);
	bool isObsoleteBuilding(BuildingTypes eIndex);																// Exposed to Python
	void changeObsoleteBuildingCount(BuildingTypes eIndex, int iChange);

	DllExport int getResearchProgress(TechTypes eIndex);																						// Exposed to Python
	void setResearchProgress(TechTypes eIndex, int iNewValue, PlayerTypes ePlayer);									// Exposed to Python
	void changeResearchProgress(TechTypes eIndex, int iChange, PlayerTypes ePlayer);								// Exposed to Python

	DllExport int getTechCount(TechTypes eIndex);																										// Exposed to Python

	int getTerrainTradeCount(TerrainTypes eIndex);
	bool isTerrainTrade(TerrainTypes eIndex);																												// Exposed to Python
	void changeTerrainTradeCount(TerrainTypes eIndex, int iChange);

	DllExport bool isHasTech(TechTypes eIndex);																																			// Exposed to Python
	DllExport void setHasTech(TechTypes eIndex, bool bNewValue, PlayerTypes ePlayer, bool bFirst, bool bAnnounce);	// Exposed to Python

	bool isNoTradeTech(TechTypes eIndex);																														// Exposed to Python
	void setNoTradeTech(TechTypes eIndex, bool bNewValue);																					// Exposed to Python

	int getImprovementYieldChange(ImprovementTypes eIndex1, YieldTypes eIndex2);										// Exposed to Python 
	void changeImprovementYieldChange(ImprovementTypes eIndex1, YieldTypes eIndex2, int iChange);		// Exposed to Python 

	virtual void AI_init() = 0;
	virtual void AI_reset() = 0;
	virtual void AI_doTurnPre() = 0;
	virtual void AI_doTurnPost() = 0;
	virtual void AI_makeAssignWorkDirty() = 0;
	virtual void AI_updateAreaStragies(bool bTargets = true) = 0;
	virtual bool AI_shareWar(TeamTypes eTeam) = 0;																									// Exposed to Python
	virtual void AI_updateWorstEnemy() = 0;
	virtual int AI_getAtWarCounter(TeamTypes eIndex) = 0;
	virtual void AI_setAtWarCounter(TeamTypes eIndex, int iNewValue) = 0;
	virtual int AI_getAtPeaceCounter(TeamTypes eIndex) = 0;
	virtual void AI_setAtPeaceCounter(TeamTypes eIndex, int iNewValue) = 0;
	virtual int AI_getHasMetCounter(TeamTypes eIndex) = 0;
	virtual void AI_setHasMetCounter(TeamTypes eIndex, int iNewValue) = 0;
	virtual int AI_getOpenBordersCounter(TeamTypes eIndex) = 0;
	virtual void AI_setOpenBordersCounter(TeamTypes eIndex, int iNewValue) = 0;
	virtual int AI_getDefensivePactCounter(TeamTypes eIndex) = 0;
	virtual void AI_setDefensivePactCounter(TeamTypes eIndex, int iNewValue) = 0;
	virtual int AI_getShareWarCounter(TeamTypes eIndex) = 0;
	virtual void AI_setShareWarCounter(TeamTypes eIndex, int iNewValue) = 0;
	virtual int AI_getWarSuccess(TeamTypes eIndex) = 0;
	virtual void AI_setWarSuccess(TeamTypes eIndex, int iNewValue) = 0;
	virtual void AI_changeWarSuccess(TeamTypes eIndex, int iChange) = 0;
	virtual int AI_getEnemyPeacetimeTradeValue(TeamTypes eIndex) = 0;
	virtual void AI_setEnemyPeacetimeTradeValue(TeamTypes eIndex, int iNewValue) = 0;
	virtual int AI_getEnemyPeacetimeGrantValue(TeamTypes eIndex) = 0;
	virtual void AI_setEnemyPeacetimeGrantValue(TeamTypes eIndex, int iNewValue) = 0;
	virtual WarPlanTypes AI_getWarPlan(TeamTypes eIndex) = 0;
	virtual bool AI_isChosenWar(TeamTypes eIndex) = 0;
	virtual bool AI_isSneakAttackPreparing(TeamTypes eIndex) = 0;
	virtual bool AI_isSneakAttackReady(TeamTypes eIndex) = 0;
	virtual void AI_setWarPlan(TeamTypes eIndex, WarPlanTypes eNewValue) = 0;

protected:

	int m_iNumMembers;
	int m_iAliveCount;
	int m_iEverAliveCount;
	int m_iNumCities;
	int m_iTotalPopulation;
	int m_iTotalLand;
	int m_iNukeInterception;
	int m_iForceTeamVoteEligibilityCount;
	int m_iExtraWaterSeeFromCount;
	int m_iMapTradingCount;
	int m_iTechTradingCount;
	int m_iGoldTradingCount;
	int m_iOpenBordersTradingCount;
	int m_iDefensivePactTradingCount;
	int m_iPermanentAllianceTradingCount;
	int m_iBridgeBuildingCount;
	int m_iIrrigationCount;
	int m_iIgnoreIrrigationCount;
	int m_iWaterWorkCount;

	bool m_bMapCentering;

	TeamTypes m_eID;

	int m_aiStolenVisibilityTimer[MAX_TEAMS];
	int m_aiWarWeariness[MAX_TEAMS];
	int m_aiTechShareCount[MAX_TEAMS];
	int m_aiCommerceFlexibleCount[NUM_COMMERCE_TYPES];
	int m_aiExtraMoves[NUM_DOMAIN_TYPES];

	bool m_abAtWar[MAX_TEAMS];
	bool m_abHasMet[MAX_TEAMS];
	bool m_abPermanentWarPeace[MAX_TEAMS];
	bool m_abOpenBorders[MAX_TEAMS];
	bool m_abDefensivePact[MAX_TEAMS];
	bool m_abForcePeace[MAX_TEAMS];

	int* m_paiRouteChange;
	int* m_paiProjectCount;
	int* m_paiProjectMaking;
	int* m_paiUnitClassCount;
	int* m_paiBuildingClassCount;
	int* m_paiObsoleteBuildingCount;
	int* m_paiResearchProgress;
	int* m_paiTechCount;
	int* m_paiTerrainTradeCount;

	bool* m_pabHasTech;
	bool* m_pabNoTradeTech;

	int** m_ppaaiImprovementYieldChange;

	void doWarWeariness();

	void updateTechShare(TechTypes eTech);
	void updateTechShare();

	void testCircumnavigated();

	void processTech(TechTypes eTech, int iChange);

	void cancelDefensivePacts();
	void announceTechToPlayers(TechTypes eIndex);

	virtual void read(FDataStreamBase* pStream);
	virtual void write(FDataStreamBase* pStream);

};

#endif
