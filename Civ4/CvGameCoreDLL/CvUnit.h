// unit.h

#ifndef CIV4_UNIT_H
#define CIV4_UNIT_H

#include "CvDLLEntity.h"
//#include "CvEnums.h"
//#include "CvStructs.h"

#pragma warning( disable: 4251 )		// needs to have dll-interface to be used by clients of class

class CvPlot;
class CvArea;
class CvSelectionGroup;
class FAStarNode;

//Added ST
struct DllExport CombatDetails					// Exposed to Python
{
	int iExtraCombatPercent;
	int iAnimalCombatModifierTA;
	int iAIAnimalCombatModifierTA;
	int iAnimalCombatModifierAA;
	int iAIAnimalCombatModifierAA;
	int iBarbarianCombatModifierTB;
	int iAIBarbarianCombatModifierTB;
	int iBarbarianCombatModifierAB;
	int iAIBarbarianCombatModifierAB;
	int iPlotDefenseModifier;
	int iFortifyModifier;
	int iCityDefenseModifier;
	int iHillsDefenseModifier;
	int iFeatureDefenseModifier;
	int iTerrainDefenseModifier;
	int iCityAttackModifier;
	int iDomainDefenseModifier;
	int iCityBarbarianDefenseModifier;
	int iClassDefenseModifier;
	int iClassAttackModifier;
	int iCombatModifierT;
	int iCombatModifierA;
	int iDomainModifierA;
	int iDomainModifierT;
	int iAnimalCombatModifierA;
	int iAnimalCombatModifierT;
	int iRiverAttackModifier;
	int iAmphibAttackModifier;
	int iModifierTotal;
	int iBaseCombatStr;
	int iCombat;
	int iMaxCombatStr;
	int iCurrHitPoints;
	int iMaxHitPoints;
	int iCurrCombatStr;
	PlayerTypes eOwner;
	std::wstring sUnitName;
};

class CvUnit : public CvDLLEntity
{

public:

	CvUnit();
	virtual ~CvUnit();

	void init(int iID, UnitTypes eUnit, UnitAITypes eUnitAI, PlayerTypes eOwner, int iX, int iY);
	void uninit();
	void reset(int iID = 0, UnitTypes eUnit = NO_UNIT, PlayerTypes eOwner = NO_PLAYER, bool bConstructorCall = false);
	void setupGraphical();

	void convert(CvUnit* pUnit);																																	// Exposed to Python
	DllExport void kill(bool bDelay, PlayerTypes ePlayer = NO_PLAYER);														// Exposed to Python

	DllExport void NotifyEntity(MissionTypes eMission);

	void doTurn();

	void updateCombat(bool bQuick = false);

	bool isActionRecommended(int iAction);

	bool isBetterDefenderThan(const CvUnit* pDefender, const CvUnit* pAttacker) const;						// Exposed to Python 

	DllExport bool canDoCommand(CommandTypes eCommand, int iData1, int iData2, bool bTestVisible = false);	// Exposed to Python
	DllExport void doCommand(CommandTypes eCommand, int iData1, int iData2);																// Exposed to Python

	FAStarNode* getPathLastNode() const;
	CvPlot* getPathEndTurnPlot() const;																																						// Exposed to Python
	bool generatePath(const CvPlot* pToPlot, int iFlags = 0, bool bReuse = false, int* piPathTurns = NULL) const;	// Exposed to Python

	bool canEnterTerritory(TeamTypes eTeam, bool bIgnoreRightOfPassage = false) const;						// Exposed to Python
	DllExport TeamTypes getDeclareWarMove(const CvPlot* pPlot) const;															// Exposed to Python
	bool canMoveInto(const CvPlot* pPlot, bool bAttack = false, bool bDeclareWar = false, bool bIgnoreLoad = false) const;	// Exposed to Python
	bool canMoveOrAttackInto(const CvPlot* pPlot, bool bDeclareWar = false) const;								// Exposed to Python
	bool canMoveThrough(const CvPlot* pPlot) const;																								// Exposed to Python
	void attack(CvPlot* pPlot, bool bQuick);
	void move(CvPlot* pPlot, bool bShow);
	void jumpToNearestValidPlot();																																// Exposed to Python

	bool canAutomate(AutomateTypes eAutomate) const;																							// Exposed to Python
	void automate(AutomateTypes eAutomate);

	bool canScrap() const;																																				// Exposed to Python
	void scrap();

	bool canGift(bool bTestVisible = false, bool bTestTransport = true);																											// Exposed to Python 
	void gift(bool bTestTransport = true);

	bool canLoadUnit(const CvUnit* pUnit, const CvPlot* pPlot) const;															// Exposed to Python
	void loadUnit(CvUnit* pUnit);

	bool canLoad(const CvPlot* pPlot) const;																											// Exposed to Python
	void load();

	bool canUnload() const;																																				// Exposed to Python
	void unload();

	bool canUnloadAll() const;																																		// Exposed to Python
	void unloadAll();

	bool canHold(const CvPlot* pPlot) const;																											// Exposed to Python
	DllExport bool canSleep(const CvPlot* pPlot) const;																						// Exposed to Python
	DllExport bool canFortify(const CvPlot* pPlot) const;																					// Exposed to Python
	bool canAirPatrol(const CvPlot* pPlot) const;																									// Exposed to Python
	void airCircle(bool bStart);

	bool canHeal(const CvPlot* pPlot) const;																											// Exposed to Python
	bool canSentry(const CvPlot* pPlot) const;																										// Exposed to Python

	int healRate(const CvPlot* pPlot) const;
	DllExport int healTurns(const CvPlot* pPlot) const;
	void doHeal();

	bool canAirlift(const CvPlot* pPlot) const;																										// Exposed to Python
	bool canAirliftAt(const CvPlot* pPlot, int iX, int iY) const;																	// Exposed to Python
	bool airlift(int iX, int iY);

	bool isNukeVictim(const CvPlot* pPlot, TeamTypes eTeam) const;																// Exposed to Python
	bool canNuke(const CvPlot* pPlot) const;																											// Exposed to Python
	bool canNukeAt(const CvPlot* pPlot, int iX, int iY) const;																		// Exposed to Python
	bool nuke(int iX, int iY);

	bool canRecon(const CvPlot* pPlot) const;																											// Exposed to Python
	bool canReconAt(const CvPlot* pPlot, int iX, int iY) const;																		// Exposed to Python
	bool recon(int iX, int iY);

	bool canAirBomb(const CvPlot* pPlot) const;																										// Exposed to Python
	bool canAirBombAt(const CvPlot* pPlot, int iX, int iY) const;																	// Exposed to Python
	bool airBomb(int iX, int iY);

	CvCity* bombardTarget(const CvPlot* pPlot) const;																							// Exposed to Python
	bool canBombard(const CvPlot* pPlot) const;																										// Exposed to Python
	bool bombard();

	bool canPillage(const CvPlot* pPlot) const;																										// Exposed to Python
	bool pillage();

	DllExport int sabotageCost(const CvPlot* pPlot) const;																									// Exposed to Python
	DllExport int sabotageProb(const CvPlot* pPlot, ProbabilityTypes eProbStyle = PROBABILITY_REAL) const;	// Exposed to Python
	DllExport bool canSabotage(const CvPlot* pPlot, bool bTestVisible = false) const;												// Exposed to Python
	bool sabotage();

	DllExport int destroyCost(const CvPlot* pPlot) const;																										// Exposed to Python
	DllExport int destroyProb(const CvPlot* pPlot, ProbabilityTypes eProbStyle = PROBABILITY_REAL) const;		// Exposed to Python
	DllExport bool canDestroy(const CvPlot* pPlot, bool bTestVisible = false) const;												// Exposed to Python
	bool destroy();

	DllExport int stealPlansCost(const CvPlot* pPlot) const;																									// Exposed to Python
	DllExport int stealPlansProb(const CvPlot* pPlot, ProbabilityTypes eProbStyle = PROBABILITY_REAL) const;	// Exposed to Python
	DllExport bool canStealPlans(const CvPlot* pPlot, bool bTestVisible = false) const;												// Exposed to Python
	bool stealPlans();

	bool canFound(const CvPlot* pPlot, bool bTestVisible = false) const;																		// Exposed to Python
	bool found();

	bool canSpread(const CvPlot* pPlot, ReligionTypes eReligion, bool bTestVisible = false) const;					// Exposed to Python
	bool spread(ReligionTypes eReligion);

	bool canJoin(const CvPlot* pPlot, SpecialistTypes eSpecialist) const;																		// Exposed to Python
	bool join(SpecialistTypes eSpecialist);

	bool canConstruct(const CvPlot* pPlot, BuildingTypes eBuilding, bool bTestVisible = false) const;				// Exposed to Python
	bool construct(BuildingTypes eBuilding);

	DllExport TechTypes getDiscoveryTech() const;																														// Exposed to Python
	DllExport int getDiscoverResearch(TechTypes eTech) const;																								// Exposed to Python
	DllExport bool canDiscover(const CvPlot* pPlot) const;																									// Exposed to Python
	bool discover();

	int getMaxHurryProduction(CvCity* pCity) const;																													// Exposed to Python
	DllExport int getHurryProduction(const CvPlot* pPlot) const;																						// Exposed to Python
	DllExport bool canHurry(const CvPlot* pPlot, bool bTestVisible = false) const;													// Exposed to Python
	bool hurry();

	DllExport int getTradeGold(const CvPlot* pPlot) const;																									// Exposed to Python
	DllExport bool canTrade(const CvPlot* pPlot, bool bTestVisible = false) const;													// Exposed to Python
	bool trade();

	DllExport int getGreatWorkCulture(const CvPlot* pPlot) const;																						// Exposed to Python
	DllExport bool canGreatWork(const CvPlot* pPlot) const;																									// Exposed to Python
	bool greatWork();

	bool canGoldenAge(const CvPlot* pPlot, bool bTestVisible = false) const;																// Exposed to Python
	bool goldenAge();

	DllExport bool canBuild(const CvPlot* pPlot, BuildTypes eBuild, bool bTestVisible = false) const;				// Exposed to Python
	bool build(BuildTypes eBuild);

	bool canPromote(PromotionTypes ePromotion) const;																												// Exposed to Python 
	void promote(PromotionTypes ePromotion);																																// Exposed to Python 

	DllExport int upgradePrice(UnitTypes eUnit) const;																											// Exposed to Python
	bool upgradeAvailable(UnitTypes eFromUnit, UnitClassTypes eToUnitClass, int iCount = 0) const;					// Exposed to Python
	DllExport bool canUpgrade(UnitTypes eUnit, bool bTestVisible = false) const;														// Exposed to Python
	void upgrade(UnitTypes eUnit);

	HandicapTypes getHandicapType() const;																// Exposed to Python		
	DllExport CivilizationTypes getCivilizationType() const;							// Exposed to Python								
	DllExport SpecialUnitTypes getSpecialUnitType() const;								// Exposed to Python								 
	UnitTypes getCaptureUnitType(CivilizationTypes eCivilization) const;	// Exposed to Python								
	DllExport UnitCombatTypes getUnitCombatType() const;									// Exposed to Python								
	DllExport DomainTypes getDomainType() const;													// Exposed to Python								
	DllExport InvisibleTypes getInvisibleType() const;										// Exposed to Python								
	DllExport InvisibleTypes getSeeInvisibleType() const;									// Exposed to Python								
																																				
	int flavorValue(FlavorTypes eFlavor) const;														// Exposed to Python		

	bool isBarbarian() const;																							// Exposed to Python
	bool isHuman() const;																									// Exposed to Python

	int visibilityRange() const;																					// Exposed to Python

	DllExport int baseMoves() const;																			// Exposed to Python
	int maxMoves() const;																									// Exposed to Python
	DllExport int movesLeft() const;																			// Exposed to Python			
	DllExport bool canMove() const;																				// Exposed to Python			
	DllExport bool hasMoved() const;																			// Exposed to Python			
																																				
	DllExport int airRange() const;																				// Exposed to Python			
	DllExport int nukeRange() const;																			// Exposed to Python			 

	bool canBuildRoute() const;																						// Exposed to Python
	DllExport BuildTypes getBuildType() const;														// Exposed to Python
	DllExport int workRate(bool bMax) const;															// Exposed to Python

	bool isAnimal() const;																								// Exposed to Python
	bool isNoBadGoodies() const;																					// Exposed to Python
	bool isOnlyDefensive() const;																					// Exposed to Python
	bool isNoCapture() const;																							// Exposed to Python 
	bool isRivalTerritory() const;																				// Exposed to Python 
	bool isMilitaryHappiness() const;																			// Exposed to Python
	bool isInvestigate() const;																						// Exposed to Python
	bool isCounterSpy() const;																						// Exposed to Python
	DllExport bool isFound() const;																				// Exposed to Python
	bool isGoldenAge() const;																							// Exposed to Python

	DllExport bool isFighting() const;																		// Exposed to Python						
	DllExport bool isAttacking() const;																		// Exposed to Python						
	DllExport bool isDefending() const;																		// Exposed to Python						
	bool isCombat() const;																								// Exposed to Python						
																																				
	DllExport int maxHitPoints() const;																		// Exposed to Python						
	DllExport int currHitPoints() const;																	// Exposed to Python						
	DllExport bool isHurt() const;																				// Exposed to Python						
	DllExport bool isDead() const;																				// Exposed to Python						

	DllExport int baseCombatStr() const;																																										// Exposed to Python
	DllExport int maxCombatStr(const CvPlot* pPlot, const CvUnit* pAttacker, CombatDetails* pCombatDetails = NULL) const;		// Exposed to Python
	DllExport int currCombatStr(const CvPlot* pPlot, const CvUnit* pAttacker, CombatDetails* pCombatDetails = NULL) const;	// Exposed to Python
	DllExport int currFirepower(const CvPlot* pPlot, const CvUnit* pAttacker) const;																				// Exposed to Python
	DllExport float maxCombatStrFloat(const CvPlot* pPlot, const CvUnit* pAttacker) const;																	// Exposed to Python
	DllExport float currCombatStrFloat(const CvPlot* pPlot, const CvUnit* pAttacker) const;																	// Exposed to Python

	DllExport bool canFight() const;																									// Exposed to Python
	bool canAttack() const;																														// Exposed to Python
	bool canDefend(const CvPlot* pPlot = NULL) const;																	// Exposed to Python
	bool canSiege(TeamTypes eTeam) const;																							// Exposed to Python

	DllExport int airBaseCombatStr() const;																						// Exposed to Python
	DllExport int airMaxCombatStr() const;																						// Exposed to Python
	DllExport int airCurrCombatStr() const;																						// Exposed to Python
	DllExport float airMaxCombatStrFloat() const;																			// Exposed to Python
	DllExport float airCurrCombatStrFloat() const;																		// Exposed to Python
	int airCombatLimit() const;																												// Exposed to Python
	DllExport bool canAirAttack() const;																							// Exposed to Python
	DllExport bool canAirDefend(const CvPlot* pPlot = NULL) const;										// Exposed to Python
	int airCombatDamage(const CvUnit* pDefender) const;																// Exposed to Python
	CvUnit* bestInterceptor(const CvPlot* pPlot) const;																// Exposed to Python

	DllExport bool isAutomated() const;																								// Exposed to Python
	DllExport bool isWaiting() const;																									// Exposed to Python 
	DllExport bool isFortifyable() const;																							// Exposed to Python
	DllExport int fortifyModifier() const;																						// Exposed to Python

	DllExport int experienceNeeded() const;																						// Exposed to Python
	int attackXPValue() const;																												// Exposed to Python	
	int defenseXPValue() const;																												// Exposed to Python	
	int maxXPValue() const;																														// Exposed to Python	

	DllExport int firstStrikes() const;																								// Exposed to Python
	DllExport int chanceFirstStrikes() const;																					// Exposed to Python 
	DllExport int maxFirstStrikes() const;																						// Exposed to Python 
	DllExport bool isRanged() const;																									// Exposed to Python

	DllExport bool alwaysInvisible() const;																						// Exposed to Python
	DllExport bool immuneToFirstStrikes() const;																			// Exposed to Python
	DllExport bool noDefensiveBonus() const;																					// Exposed to Python
	bool ignoreBuildingDefense() const;																								// Exposed to Python
	bool canMoveImpassable() const;																										// Exposed to Python
	bool flatMovementCost() const;																										// Exposed to Python
	bool ignoreTerrainCost() const;																										// Exposed to Python
	bool isNeverInvisible() const;																										// Exposed to Python
	DllExport bool isInvisible(TeamTypes eTeam, bool bDebug) const;										// Exposed to Python
	bool isNukeImmune() const;																												// Exposed to Python

	DllExport int maxInterceptionProbability() const;																	// Exposed to Python
	DllExport int currInterceptionProbability() const;																// Exposed to Python
	int evasionProbability() const;																										// Exposed to Python
	DllExport int withdrawalProbability() const;																			// Exposed to Python

	DllExport int collateralDamage() const;																						// Exposed to Python
	int collateralDamageLimit() const;																								// Exposed to Python
	int collateralDamageMaxUnits() const;																							// Exposed to Python

	DllExport int cityAttackModifier() const;																					// Exposed to Python
	DllExport int cityDefenseModifier() const;																				// Exposed to Python
	DllExport int animalCombatModifier() const;																				// Exposed to Python
	DllExport int hillsDefenseModifier() const;																				// Exposed to Python
	DllExport int terrainDefenseModifier(TerrainTypes eTerrain) const;								// Exposed to Python
	DllExport int featureDefenseModifier(FeatureTypes eFeature) const;								// Exposed to Python
	DllExport int unitClassAttackModifier(UnitClassTypes eUnitClass) const;						// Exposed to Python
	DllExport int unitClassDefenseModifier(UnitClassTypes eUnitClass) const;					// Exposed to Python
	DllExport int unitCombatModifier(UnitCombatTypes eUnitCombat) const;							// Exposed to Python
	DllExport int domainModifier(DomainTypes eDomain) const;													// Exposed to Python

	DllExport int bombardRate() const;																								// Exposed to Python
	int airBombBaseRate() const;																											// Exposed to Python
	int airBombCurrRate() const;																											// Exposed to Python

	DllExport SpecialUnitTypes specialCargo() const;																	// Exposed to Python
	DomainTypes domainCargo() const;																									// Exposed to Python
	DllExport int cargoSpace() const;																									// Exposed to Python
	bool isFull() const;																															// Exposed to Python
	int cargoSpaceAvailable(SpecialUnitTypes eSpecialCargo = NO_SPECIALUNIT, DomainTypes eDomainCargo = NO_DOMAIN) const;	// Exposed to Python
	DllExport bool hasCargo() const;																									// Exposed to Python
	bool canCargoAllMove() const;																											// Exposed to Python
	int getUnitAICargo(UnitAITypes eUnitAI) const;																		// Exposed to Python

	DllExport int getID() const;																											// Exposed to Python
	int getIndex() const;
	DllExport IDInfo getIDInfo() const;
	void setID(int iID);

	int getGroupID() const;																														// Exposed to Python
	bool isInGroup() const;																														// Exposed to Python
	DllExport bool isGroupHead() const;																								// Exposed to Python
	DllExport CvSelectionGroup* getGroup() const;																			// Exposed to Python
	bool canJoinGroup(const CvPlot* pPlot, CvSelectionGroup* pSelectionGroup) const;
	DllExport void joinGroup(CvSelectionGroup* pSelectionGroup, bool bRemoveSelected = false, bool bRejoin = true);

	DllExport int getHotKeyNumber();																													// Exposed to Python
	void setHotKeyNumber(int iNewValue);																											// Exposed to Python

	DllExport int getX() const;																																// Exposed to Python
#ifdef _USRDLL
	inline int getX_INLINE() const
	{
		return m_iX;
	}
#endif
	DllExport int getY() const;																																// Exposed to Python
#ifdef _USRDLL
	inline int getY_INLINE() const
	{
		return m_iY;
	}
#endif
	void setXY(int iX, int iY, bool bGroup = false, bool bUpdate = true, bool bShow = false);	// Exposed to Python
	bool at(int iX, int iY) const;																														// Exposed to Python
	DllExport bool atPlot(const CvPlot* pPlot) const;																					// Exposed to Python
	DllExport CvPlot* plot() const;																														// Exposed to Python
	CvArea* area() const;																																			// Exposed to Python
	bool onMap() const;

	int getLastMoveTurn() const;
	void setLastMoveTurn(int iNewValue);

	CvPlot* getReconPlot() const;																															// Exposed to Python 
	int getReconRange() const;																																// Exposed to Python
	void setReconPlot(CvPlot* pNewValue, int iRange = 0);																			// Exposed to Python

	int getGameTurnCreated() const;																														// Exposed to Python
	void setGameTurnCreated(int iNewValue);

	DllExport int getDamage() const;																													// Exposed to Python
	void setDamage(int iNewValue, PlayerTypes ePlayer = NO_PLAYER);														// Exposed to Python
	void changeDamage(int iChange, PlayerTypes ePlayer = NO_PLAYER);													// Exposed to Python

	int getMoves() const;																																			// Exposed to Python
	DllExport void setMoves(int iNewValue);																										// Exposed to Python
	void changeMoves(int iChange);																														// Exposed to Python
	void finishMoves();																																				// Exposed to Python

	DllExport int getExperience() const;																											// Exposed to Python
	DllExport void setExperience(int iNewValue, int iMax = -1);																// Exposed to Python
	void changeExperience(int iChange, int iMax = -1);																				// Exposed to Python

	DllExport int getLevel() const;																														// Exposed to Python					
	void setLevel(int iNewValue);
	void changeLevel(int iChange);

	DllExport int getCargo() const;																														// Exposed to Python					
	void changeCargo(int iChange);

	CvPlot* getAttackPlot() const;
	void setAttackPlot(const CvPlot* pNewValue);

	DllExport int getCombatTimer() const;
	void setCombatTimer(int iNewValue);
	void changeCombatTimer(int iChange);

	int getCombatFirstStrikes() const;			
	void setCombatFirstStrikes(int iNewValue);			
	void changeCombatFirstStrikes(int iChange);			

	DllExport int getCombatDamage() const;			
	void setCombatDamage(int iNewValue);			

	int getFortifyTurns() const;																															// Exposed to Python
	void setFortifyTurns(int iNewValue);
	void changeFortifyTurns(int iChange);

	int getBlitzCount() const;			
	DllExport bool isBlitz() const;																														// Exposed to Python					
	void changeBlitzCount(int iChange);																												
																																														
	int getAmphibCount() const;																																
	DllExport bool isAmphib() const;																													// Exposed to Python					
	void changeAmphibCount(int iChange);																											
																																														
	int getRiverCount() const;																																
	DllExport bool isRiver() const;																														// Exposed to Python					
	void changeRiverCount(int iChange);																												
																																														
	DllExport int getEnemyRouteCount() const;																									
	DllExport bool isEnemyRoute() const;																											// Exposed to Python					
	void changeEnemyRouteCount(int iChange);																									
																																														
	int getAlwaysHealCount() const;																														
	DllExport bool isAlwaysHeal() const;																											// Exposed to Python					
	void changeAlwaysHealCount(int iChange);																									
																																														
	int getHillsDoubleMoveCount() const;																											
	DllExport bool isHillsDoubleMove() const;																									// Exposed to Python					
	void changeHillsDoubleMoveCount(int iChange);																							
																																														
	int getImmuneToFirstStrikesCount() const;																									
	void changeImmuneToFirstStrikesCount(int iChange);																				
																																														
	DllExport int getExtraVisibilityRange() const;																						// Exposed to Python					
	void changeExtraVisibilityRange(int iChange);

	int getExtraMoves() const;																																// Exposed to Python
	void changeExtraMoves(int iChange);

	DllExport int getExtraMoveDiscount() const;																								// Exposed to Python
	void changeExtraMoveDiscount(int iChange);

	int getExtraFirstStrikes() const;																													// Exposed to Python
	void changeExtraFirstStrikes(int iChange);

	int getExtraChanceFirstStrikes() const;																										// Exposed to Python
	void changeExtraChanceFirstStrikes(int iChange);

	int getExtraWithdrawal() const;																														// Exposed to Python
	void changeExtraWithdrawal(int iChange);

	int getExtraCollateralDamage() const;																											// Exposed to Python
	void changeExtraCollateralDamage(int iChange);

	int getExtraBombardRate() const;																													// Exposed to Python
	void changeExtraBombardRate(int iChange);

	DllExport int getExtraEnemyHeal() const;																									// Exposed to Python					
	void changeExtraEnemyHeal(int iChange);																										
																																														
	DllExport int getExtraNeutralHeal() const;																								// Exposed to Python					
	void changeExtraNeutralHeal(int iChange);																									
																																														
	DllExport int getExtraFriendlyHeal() const;																								// Exposed to Python					
	void changeExtraFriendlyHeal(int iChange);																								
																																														
	DllExport int getSameTileHeal() const;																										// Exposed to Python					
	void changeSameTileHeal(int iChange);																											
																																														
	DllExport int getAdjacentTileHeal() const;																								// Exposed to Python					
	void changeAdjacentTileHeal(int iChange);																									
																																														
	DllExport int getExtraCombatPercent() const;																							// Exposed to Python					
	void changeExtraCombatPercent(int iChange);

	int getExtraCityAttackPercent() const;																										// Exposed to Python
	void changeExtraCityAttackPercent(int iChange);

	int getExtraCityDefensePercent() const;																										// Exposed to Python
	void changeExtraCityDefensePercent(int iChange);

	int getExtraHillsDefensePercent() const;																									// Exposed to Python
	void changeExtraHillsDefensePercent(int iChange);

	bool isMadeAttack() const;																																// Exposed to Python
	DllExport void setMadeAttack(bool bNewValue);																							// Exposed to Python

	bool isMadeInterception() const;																													// Exposed to Python
	DllExport void setMadeInterception(bool bNewValue);																				// Exposed to Python

	DllExport bool isPromotionReady() const;																									// Exposed to Python
	DllExport void setPromotionReady(bool bNewValue);																					// Exposed to Python
	void testPromotionReady();

	bool isDelayedDeath() const;
	void startDelayedDeath();
	bool doDelayedDeath();

	bool isCombatFocus() const;

	DllExport bool isInfoBarDirty() const;
	DllExport void setInfoBarDirty(bool bNewValue);

	DllExport PlayerTypes getOwner() const;																									// Exposed to Python
#ifdef _USRDLL
	inline PlayerTypes getOwnerINLINE() const
	{
		return m_eOwner;
	}
#endif
	DllExport TeamTypes getTeam() const;																										// Exposed to Python

	PlayerTypes getCapturingPlayer() const;
	void setCapturingPlayer(PlayerTypes eNewValue);

	DllExport const UnitTypes getUnitType() const;																					// Exposed to Python
	UnitClassTypes getUnitClassType() const;																								// Exposed to Python

	DllExport CvUnit* getCombatUnit() const;
	void setCombatUnit(CvUnit* pUnit, bool bAttacking = false);

	DllExport CvUnit* getTransportUnit() const;																							// Exposed to Python
	DllExport bool isCargo() const;																													// Exposed to Python
	void setTransportUnit(CvUnit* pTransportUnit);

	int getExtraDomainModifier(DomainTypes eIndex) const;																		// Exposed to Python
	void changeExtraDomainModifier(DomainTypes eIndex, int iChange);

	DllExport const CvWString getName(uint uiForm = 0) const;																// Exposed to Python
	DllExport const wchar* getNameKey() const;																							// Exposed to Python
	DllExport const CvWString getNameNoDesc() const;																				// Exposed to Python
	DllExport void setName(const CvWString szNewValue);																			// Exposed to Python

	// Script data needs to be a narrow string for pickling in Python
	std::string getScriptData() const;																											// Exposed to Python
	void setScriptData(std::string szNewValue);																							// Exposed to Python

	int getTerrainDoubleMoveCount(TerrainTypes eIndex) const;
	DllExport bool isTerrainDoubleMove(TerrainTypes eIndex) const;													// Exposed to Python 
	void changeTerrainDoubleMoveCount(TerrainTypes eIndex, int iChange);

	int getFeatureDoubleMoveCount(FeatureTypes eIndex) const;
	DllExport bool isFeatureDoubleMove(FeatureTypes eIndex) const;													// Exposed to Python 
	void changeFeatureDoubleMoveCount(FeatureTypes eIndex, int iChange);

	int getExtraTerrainDefensePercent(TerrainTypes eIndex) const;														// Exposed to Python
	void changeExtraTerrainDefensePercent(TerrainTypes eIndex, int iChange);						

	int getExtraFeatureDefensePercent(FeatureTypes eIndex) const;														// Exposed to Python
	void changeExtraFeatureDefensePercent(FeatureTypes eIndex, int iChange);

	int getExtraUnitCombatModifier(UnitCombatTypes eIndex) const;														// Exposed to Python
	void changeExtraUnitCombatModifier(UnitCombatTypes eIndex, int iChange);

	bool canAcquirePromotion(PromotionTypes ePromotion) const;															// Exposed to Python 
	bool canAcquirePromotionAny() const;																										// Exposed to Python
	DllExport bool isHasPromotion(PromotionTypes eIndex) const;															// Exposed to Python
	DllExport void setHasPromotion(PromotionTypes eIndex, bool bNewValue);									// Exposed to Python

	DllExport int getSubUnitCount() const;
	DllExport int getSubUnitsAlive() const;
	int getSubUnitsAlive(int iDamage) const;

	void read(FDataStreamBase* pStream);
	void write(FDataStreamBase* pStream);

	virtual void AI_init(UnitAITypes eUnitAI) = 0;
	virtual void AI_uninit() = 0;
	virtual void AI_reset(UnitAITypes eUnitAI = NO_UNITAI) = 0;
	virtual void AI_update() = 0;
	virtual bool AI_follow() = 0;
	virtual void AI_upgrade() = 0;
	virtual void AI_promote() = 0;
	virtual int AI_groupFirstVal() = 0;
	virtual int AI_groupSecondVal() = 0;
	virtual int AI_attackOdds(const CvPlot* pPlot, bool bPotentialEnemy) const = 0;
	virtual bool AI_bestCityBuild(CvCity* pCity, CvPlot** ppBestPlot = NULL, BuildTypes* peBestBuild = NULL, CvPlot* pIgnorePlot = NULL) = 0;
	virtual bool AI_isCityAIType() const = 0;
	virtual UnitAITypes AI_getUnitAIType() const = 0;																				// Exposed to Python
	virtual void AI_setUnitAIType(UnitAITypes eNewValue) = 0;

protected:

	int m_iID;
	int m_iGroupID;
	int m_iHotKeyNumber;
	int m_iX;
	int m_iY;
	int m_iLastMoveTurn;
	int m_iReconX;
	int m_iReconY;
	int m_iReconRange;
	int m_iGameTurnCreated;
	int m_iDamage;
	int m_iMoves;
	int m_iExperience;
	int m_iLevel;
	int m_iCargo;
	int m_iAttackPlotX;
	int m_iAttackPlotY;
	int m_iCombatTimer;
	int m_iCombatFirstStrikes;
	int m_iCombatDamage;
	int m_iFortifyTurns;
	int m_iBlitzCount;
	int m_iAmphibCount;
	int m_iRiverCount;
	int m_iEnemyRouteCount;
	int m_iAlwaysHealCount;
	int m_iHillsDoubleMoveCount;
	int m_iImmuneToFirstStrikesCount;
	int m_iExtraVisibilityRange;
	int m_iExtraMoves;
	int m_iExtraMoveDiscount;
	int m_iExtraFirstStrikes;
	int m_iExtraChanceFirstStrikes;
	int m_iExtraWithdrawal;
	int m_iExtraCollateralDamage;
	int m_iExtraBombardRate;
	int m_iExtraEnemyHeal;
	int m_iExtraNeutralHeal;
	int m_iExtraFriendlyHeal;
	int m_iSameTileHeal;
	int m_iAdjacentTileHeal;
	int m_iExtraCombatPercent;
	int m_iExtraCityAttackPercent;
	int m_iExtraCityDefensePercent;
	int m_iExtraHillsDefensePercent;

	bool m_bMadeAttack;
	bool m_bMadeInterception;
	bool m_bPromotionReady;
	bool m_bDeathDelay;
	bool m_bCombatFocus;
	bool m_bInfoBarDirty;

	PlayerTypes m_eOwner;
	PlayerTypes m_eCapturingPlayer;
	UnitTypes m_eUnitType;

	IDInfo m_combatUnit;
	IDInfo m_transportUnit;

	int m_aiExtraDomainModifier[NUM_DOMAIN_TYPES];

	CvWString m_szName;
	CvString m_szScriptData;

	bool* m_pabHasPromotion;

	int* m_paiTerrainDoubleMoveCount;
	int* m_paiFeatureDoubleMoveCount;
	int* m_paiExtraTerrainDefensePercent;
	int* m_paiExtraFeatureDefensePercent;
	int* m_paiExtraUnitCombatModifier;

	bool canAdvance(const CvPlot* pPlot, int iThreshold) const;
	void collateralCombat(const CvPlot* pPlot, CvUnit* pSkipUnit = NULL);

	bool interceptTest(const CvPlot* pPlot);
	CvUnit* airStrikeTarget(const CvPlot* pPlot) const;
	bool canAirStrike(const CvPlot* pPlot) const;
	void airStrike(CvPlot* pPlot);

	int planBattle( CvBattleDefinition & kBattleDefinition ) const;
	int computeUnitsToDie( const CvBattleDefinition & kDefinition, bool bRanged, int iUnit ) const;
	bool verifyRoundsValid( const CvBattlePlanVector & vctBattlePlan ) const;
	void increaseBattleRounds( CvBattleDefinition & vctBattlePlan ) const;
	int computeWaveSize( bool bRangedRound, int iAttackerMax, int iDefenderMax ) const;
};

#endif
