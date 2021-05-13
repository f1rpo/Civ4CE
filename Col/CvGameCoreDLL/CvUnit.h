#pragma once

// unit.h

#ifndef CIV4_UNIT_H
#define CIV4_UNIT_H

#include "CvDLLEntity.h"
//#include "CvEnums.h"
//#include "CvStructs.h"

#pragma warning( disable: 4251 )		// needs to have dll-interface to be used by clients of class

class CvPlot;
class CvArea;
class CvUnitInfo;
class CvSelectionGroup;
class FAStarNode;
class CvArtInfoUnit;

struct DllExport CombatDetails
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
	int iHillsAttackModifier;
	int iHillsDefenseModifier;
	int iFeatureAttackModifier;
	int iFeatureDefenseModifier;
	int iTerrainAttackModifier;
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
	int iKamikazeModifier;
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

	void reloadEntity();
	void init(int iID, UnitTypes eUnit, UnitAITypes eUnitAI, PlayerTypes eOwner, int iX, int iY, DirectionTypes eFacingDirection);
	void uninit();
	void reset(int iID = 0, UnitTypes eUnit = NO_UNIT, PlayerTypes eOwner = NO_PLAYER, bool bConstructorCall = false);
	void setupGraphical();

	void convert(CvUnit* pUnit);
	DllExport void kill(bool bDelay, PlayerTypes ePlayer = NO_PLAYER);

	DllExport void NotifyEntity(MissionTypes eMission);

	void doTurn();

	void updateCombat(bool bQuick = false);
	void updateAirCombat(bool bQuick = false);
	void updateAirStrike(CvPlot* pPlot, bool bQuick, bool bFinish);

	bool isActionRecommended(int iAction);

	bool isBetterDefenderThan(const CvUnit* pDefender, const CvUnit* pAttacker) const;

	DllExport bool canDoCommand(CommandTypes eCommand, int iData1, int iData2, bool bTestVisible = false, bool bTestBusy = true);
	DllExport void doCommand(CommandTypes eCommand, int iData1, int iData2);

	FAStarNode* getPathLastNode() const;
	CvPlot* getPathEndTurnPlot() const;
	bool generatePath(const CvPlot* pToPlot, int iFlags = 0, bool bReuse = false, int* piPathTurns = NULL) const;

	bool canEnterTerritory(TeamTypes eTeam, bool bIgnoreRightOfPassage = false) const;
	bool canEnterArea(TeamTypes eTeam, const CvArea* pArea, bool bIgnoreRightOfPassage = false) const;
	DllExport TeamTypes getDeclareWarMove(const CvPlot* pPlot) const;
	bool canMoveInto(const CvPlot* pPlot, bool bAttack = false, bool bDeclareWar = false, bool bIgnoreLoad = false) const;
	bool canMoveOrAttackInto(const CvPlot* pPlot, bool bDeclareWar = false) const;
	bool canMoveThrough(const CvPlot* pPlot) const;
	void attack(CvPlot* pPlot, bool bQuick);
	void attackForDamage(CvUnit *pDefender, int attackerDamageChange, int defenderDamageChange);
	void fightInterceptor(const CvPlot* pPlot, bool bQuick);
	void move(CvPlot* pPlot, bool bShow);
	bool jumpToNearestValidPlot();

	bool canAutomate(AutomateTypes eAutomate) const;
	void automate(AutomateTypes eAutomate);

	bool canScrap() const;
	void scrap();

	bool canGift(bool bTestVisible = false, bool bTestTransport = true);
	void gift(bool bTestTransport = true);

	bool canLoadUnit(const CvUnit* pUnit, const CvPlot* pPlot) const;
	void loadUnit(CvUnit* pUnit);

	bool canLoad(const CvPlot* pPlot) const;
	void load();
	bool shouldLoadOnMove(const CvPlot* pPlot) const;

	bool canUnload() const;
	void unload();

	bool canUnloadAll() const;
	void unloadAll();

	bool canHold(const CvPlot* pPlot) const;
	DllExport bool canSleep(const CvPlot* pPlot) const;
	DllExport bool canFortify(const CvPlot* pPlot) const;
	bool canAirPatrol(const CvPlot* pPlot) const;
	void airCircle(bool bStart);

	bool canSeaPatrol(const CvPlot* pPlot) const;

	bool canHeal(const CvPlot* pPlot) const;
	bool canSentry(const CvPlot* pPlot) const;

	int healRate(const CvPlot* pPlot) const;
	DllExport int healTurns(const CvPlot* pPlot) const;
	void doHeal();

	bool canAirlift(const CvPlot* pPlot) const;
	bool canAirliftAt(const CvPlot* pPlot, int iX, int iY) const;
	bool airlift(int iX, int iY);

	DllExport bool isNukeVictim(const CvPlot* pPlot, TeamTypes eTeam) const;
	bool canNuke(const CvPlot* pPlot) const;
	bool canNukeAt(const CvPlot* pPlot, int iX, int iY) const;
	bool nuke(int iX, int iY);

	bool canRecon(const CvPlot* pPlot) const;
	bool canReconAt(const CvPlot* pPlot, int iX, int iY) const;
	bool recon(int iX, int iY);

	bool canAirBomb(const CvPlot* pPlot) const;
	bool canAirBombAt(const CvPlot* pPlot, int iX, int iY) const;
	bool airBomb(int iX, int iY);

	CvCity* bombardTarget(const CvPlot* pPlot) const;
	bool canBombard(const CvPlot* pPlot) const;
	bool bombard();

	bool canParadrop(const CvPlot* pPlot) const;
	bool canParadropAt(const CvPlot* pPlot, int iX, int iY) const;
	bool paradrop(int iX, int iY);

	bool canPillage(const CvPlot* pPlot) const;
	bool pillage();

	bool canPlunder(const CvPlot* pPlot, bool bTestVisible = false) const;
	bool plunder();
	void updatePlunder(int iChange, bool bUpdatePlotGroups);

	DllExport int sabotageCost(const CvPlot* pPlot) const;
	DllExport int sabotageProb(const CvPlot* pPlot, ProbabilityTypes eProbStyle = PROBABILITY_REAL) const;
	DllExport bool canSabotage(const CvPlot* pPlot, bool bTestVisible = false) const;
	bool sabotage();

	DllExport int destroyCost(const CvPlot* pPlot) const;
	DllExport int destroyProb(const CvPlot* pPlot, ProbabilityTypes eProbStyle = PROBABILITY_REAL) const;
	DllExport bool canDestroy(const CvPlot* pPlot, bool bTestVisible = false) const;
	bool destroy();

	DllExport int stealPlansCost(const CvPlot* pPlot) const;
	DllExport int stealPlansProb(const CvPlot* pPlot, ProbabilityTypes eProbStyle = PROBABILITY_REAL) const;
	DllExport bool canStealPlans(const CvPlot* pPlot, bool bTestVisible = false) const;
	bool stealPlans();

	bool canFound(const CvPlot* pPlot, bool bTestVisible = false) const;
	bool found();

	bool canSpread(const CvPlot* pPlot, ReligionTypes eReligion, bool bTestVisible = false) const;
	bool spread(ReligionTypes eReligion);

	bool canSpreadCorporation(const CvPlot* pPlot, CorporationTypes eCorporation, bool bTestVisible = false) const;
	bool spreadCorporation(CorporationTypes eCorporation);
	int spreadCorporationCost(CorporationTypes eCorporation, CvCity* pCity) const;

	bool canJoin(const CvPlot* pPlot, SpecialistTypes eSpecialist) const;
	bool join(SpecialistTypes eSpecialist);

	bool canConstruct(const CvPlot* pPlot, BuildingTypes eBuilding, bool bTestVisible = false) const;
	bool construct(BuildingTypes eBuilding);

	DllExport TechTypes getDiscoveryTech() const;
	DllExport int getDiscoverResearch(TechTypes eTech) const;
	DllExport bool canDiscover(const CvPlot* pPlot) const;
	bool discover();

	int getMaxHurryProduction(CvCity* pCity) const;
	DllExport int getHurryProduction(const CvPlot* pPlot) const;
	DllExport bool canHurry(const CvPlot* pPlot, bool bTestVisible = false) const;
	bool hurry();

	DllExport int getTradeGold(const CvPlot* pPlot) const;
	DllExport bool canTrade(const CvPlot* pPlot, bool bTestVisible = false) const;
	bool trade();

	DllExport int getGreatWorkCulture(const CvPlot* pPlot) const;
	DllExport bool canGreatWork(const CvPlot* pPlot) const;
	bool greatWork();

	DllExport int getEspionagePoints(const CvPlot* pPlot) const;
	DllExport bool canInfiltrate(const CvPlot* pPlot, bool bTestVisible = false) const;
	bool infiltrate();

	bool canEspionage(const CvPlot* pPlot, bool bTestVisible = false) const;
	bool espionage(EspionageMissionTypes eMission, int iData);
	bool testSpyIntercepted(PlayerTypes eTargetPlayer, int iModifier = 0);
	int getSpyInterceptPercent(TeamTypes eTargetTeam) const;
	bool isIntruding() const;

	bool canGoldenAge(const CvPlot* pPlot, bool bTestVisible = false) const;
	bool goldenAge();

	DllExport bool canBuild(const CvPlot* pPlot, BuildTypes eBuild, bool bTestVisible = false) const;
	bool build(BuildTypes eBuild);

	bool canPromote(PromotionTypes ePromotion, int iLeaderUnitId) const;
	void promote(PromotionTypes ePromotion, int iLeaderUnitId);

	int canLead(const CvPlot* pPlot, int iUnitId) const;
	bool lead(int iUnitId);

	int canGiveExperience(const CvPlot* pPlot) const;
	bool giveExperience();
	int getStackExperienceToGive(int iNumUnits) const;

	DllExport int upgradePrice(UnitTypes eUnit) const;
	bool upgradeAvailable(UnitTypes eFromUnit, UnitClassTypes eToUnitClass, int iCount = 0) const;
	DllExport bool canUpgrade(UnitTypes eUnit, bool bTestVisible = false) const;
	DllExport bool isReadyForUpgrade() const;
	bool hasUpgrade(bool bSearch = false) const;
	bool hasUpgrade(UnitTypes eUnit, bool bSearch = false) const;
	CvCity* getUpgradeCity(bool bSearch = false) const;
	CvCity* getUpgradeCity(UnitTypes eUnit, bool bSearch = false, int* iSearchValue = NULL) const;
	void upgrade(UnitTypes eUnit);

	HandicapTypes getHandicapType() const;
	DllExport CivilizationTypes getCivilizationType() const;
	const wchar* getVisualCivAdjective(TeamTypes eForTeam) const;
	DllExport SpecialUnitTypes getSpecialUnitType() const;
	UnitTypes getCaptureUnitType(CivilizationTypes eCivilization) const;
	DllExport UnitCombatTypes getUnitCombatType() const;
	DllExport DomainTypes getDomainType() const;
	DllExport InvisibleTypes getInvisibleType() const;
	DllExport int getNumSeeInvisibleTypes() const;
	DllExport InvisibleTypes getSeeInvisibleType(int i) const;

	int flavorValue(FlavorTypes eFlavor) const;

	bool isBarbarian() const;
	bool isHuman() const;

	int visibilityRange() const;

	DllExport int baseMoves() const;
	int maxMoves() const;
	DllExport int movesLeft() const;
	DllExport bool canMove() const;
	DllExport bool hasMoved() const;

	DllExport int airRange() const;
	DllExport int nukeRange() const;

	bool canBuildRoute() const;
	DllExport BuildTypes getBuildType() const;
	DllExport int workRate(bool bMax) const;

	bool isAnimal() const;
	bool isNoBadGoodies() const;
	bool isOnlyDefensive() const;
	bool isNoCapture() const;
	bool isRivalTerritory() const;
	bool isMilitaryHappiness() const;
	bool isInvestigate() const;
	bool isCounterSpy() const;
	bool isSpy() const;
	DllExport bool isFound() const;
	bool isGoldenAge() const;
	bool canCoexistWithEnemyUnit(TeamTypes eTeam) const;

	DllExport bool isFighting() const;
	DllExport bool isAttacking() const;
	DllExport bool isDefending() const;
	bool isCombat() const;

	DllExport int maxHitPoints() const;
	DllExport int currHitPoints() const;
	DllExport bool isHurt() const;
	DllExport bool isDead() const;

	void setBaseCombatStr(int iCombat);
	DllExport int baseCombatStr() const;
	DllExport int maxCombatStr(const CvPlot* pPlot, const CvUnit* pAttacker, CombatDetails* pCombatDetails = NULL) const;
	DllExport int currCombatStr(const CvPlot* pPlot, const CvUnit* pAttacker, CombatDetails* pCombatDetails = NULL) const;
	DllExport int currFirepower(const CvPlot* pPlot, const CvUnit* pAttacker) const;
	int currEffectiveStr(const CvPlot* pPlot, const CvUnit* pAttacker, CombatDetails* pCombatDetails = NULL) const;
	DllExport float maxCombatStrFloat(const CvPlot* pPlot, const CvUnit* pAttacker) const;
	DllExport float currCombatStrFloat(const CvPlot* pPlot, const CvUnit* pAttacker) const;

	DllExport bool canFight() const;
	bool canAttack() const;
	bool canDefend(const CvPlot* pPlot = NULL) const;
	bool canSiege(TeamTypes eTeam) const;

	DllExport int airBaseCombatStr() const;
	DllExport int airMaxCombatStr(const CvUnit* pOther) const;
	DllExport int airCurrCombatStr(const CvUnit* pOther) const;
	DllExport float airMaxCombatStrFloat(const CvUnit* pOther) const;
	DllExport float airCurrCombatStrFloat(const CvUnit* pOther) const;
	int combatLimit() const;
	int airCombatLimit() const;
	DllExport bool canAirAttack() const;
	DllExport bool canAirDefend(const CvPlot* pPlot = NULL) const;
	int airCombatDamage(const CvUnit* pDefender) const;
	int rangeCombatDamage(const CvUnit* pDefender) const;
	CvUnit* bestInterceptor(const CvPlot* pPlot) const;
	CvUnit* bestSeaPillageInterceptor(const CvPlot* pPlot) const;

	DllExport bool isAutomated() const;
	DllExport bool isWaiting() const;
	DllExport bool isFortifyable() const;
	DllExport int fortifyModifier() const;

	DllExport int experienceNeeded() const;
	int attackXPValue() const;
	int defenseXPValue() const;
	int maxXPValue() const;

	DllExport int firstStrikes() const;
	DllExport int chanceFirstStrikes() const;
	DllExport int maxFirstStrikes() const;
	DllExport bool isRanged() const;

	DllExport bool alwaysInvisible() const;
	DllExport bool immuneToFirstStrikes() const;
	DllExport bool noDefensiveBonus() const;
	bool ignoreBuildingDefense() const;
	bool canMoveImpassable() const;
	bool canMoveAllTerrain() const;
	bool flatMovementCost() const;
	bool ignoreTerrainCost() const;
	bool isNeverInvisible() const;
	DllExport bool isInvisible(TeamTypes eTeam, bool bDebug, bool bCheckCargo = true) const;
	bool isNukeImmune() const;

	DllExport int maxInterceptionProbability() const;
	DllExport int currInterceptionProbability() const;
	int evasionProbability() const;
	DllExport int withdrawalProbability() const;

	DllExport int collateralDamage() const;
	int collateralDamageLimit() const;
	int collateralDamageMaxUnits() const;

	DllExport int cityAttackModifier() const;
	DllExport int cityDefenseModifier() const;
	DllExport int animalCombatModifier() const;
	DllExport int hillsAttackModifier() const;
	DllExport int hillsDefenseModifier() const;
	DllExport int terrainAttackModifier(TerrainTypes eTerrain) const;
	DllExport int terrainDefenseModifier(TerrainTypes eTerrain) const;
	DllExport int featureAttackModifier(FeatureTypes eFeature) const;
	DllExport int featureDefenseModifier(FeatureTypes eFeature) const;
	DllExport int unitClassAttackModifier(UnitClassTypes eUnitClass) const;
	DllExport int unitClassDefenseModifier(UnitClassTypes eUnitClass) const;
	DllExport int unitCombatModifier(UnitCombatTypes eUnitCombat) const;
	DllExport int domainModifier(DomainTypes eDomain) const;

	DllExport int bombardRate() const;
	int airBombBaseRate() const;
	int airBombCurrRate() const;

	DllExport SpecialUnitTypes specialCargo() const;
	DomainTypes domainCargo() const;
	DllExport int cargoSpace() const;
	DllExport void changeCargoSpace(int iChange);
	bool isFull() const;
	int cargoSpaceAvailable(SpecialUnitTypes eSpecialCargo = NO_SPECIALUNIT, DomainTypes eDomainCargo = NO_DOMAIN) const;
	DllExport bool hasCargo() const;
	bool canCargoAllMove() const;
	bool canCargoEnterArea(TeamTypes eTeam, const CvArea* pArea, bool bIgnoreRightOfPassage) const;
	int getUnitAICargo(UnitAITypes eUnitAI) const;

	DllExport int getID() const;
	int getIndex() const;
	DllExport IDInfo getIDInfo() const;
	void setID(int iID);

	int getGroupID() const;
	bool isInGroup() const;
	DllExport bool isGroupHead() const;
	DllExport CvSelectionGroup* getGroup() const;
	bool canJoinGroup(const CvPlot* pPlot, CvSelectionGroup* pSelectionGroup) const;
	DllExport void joinGroup(CvSelectionGroup* pSelectionGroup, bool bRemoveSelected = false, bool bRejoin = true);

	DllExport int getHotKeyNumber();
	void setHotKeyNumber(int iNewValue);

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
	void setXY(int iX, int iY, bool bGroup = false, bool bUpdate = true, bool bShow = false, bool bCheckPlotVisible = false);
	bool at(int iX, int iY) const;
	DllExport bool atPlot(const CvPlot* pPlot) const;
	DllExport CvPlot* plot() const;
	int getArea() const;
	CvArea* area() const;
	bool onMap() const;

	int getLastMoveTurn() const;
	void setLastMoveTurn(int iNewValue);

	CvPlot* getReconPlot() const;
	void setReconPlot(CvPlot* pNewValue);

	int getGameTurnCreated() const;
	void setGameTurnCreated(int iNewValue);

	DllExport int getDamage() const;
	void setDamage(int iNewValue, PlayerTypes ePlayer = NO_PLAYER, bool bNotifyEntity = true);
	void changeDamage(int iChange, PlayerTypes ePlayer = NO_PLAYER);

	int getMoves() const;
	DllExport void setMoves(int iNewValue);
	void changeMoves(int iChange);
	void finishMoves();

	DllExport int getExperience() const;
	DllExport void setExperience(int iNewValue, int iMax = -1);
	void changeExperience(int iChange, int iMax = -1, bool bFromCombat = false, bool bInBorders = false, bool bUpdateGlobal = false);

	DllExport int getLevel() const;
	void setLevel(int iNewValue);
	void changeLevel(int iChange);

	DllExport int getCargo() const;
	void changeCargo(int iChange);

	CvPlot* getAttackPlot() const;
	void setAttackPlot(const CvPlot* pNewValue, bool bAirCombat);
	bool isAirCombat() const;

	DllExport int getCombatTimer() const;
	void setCombatTimer(int iNewValue);
	void changeCombatTimer(int iChange);

	int getCombatFirstStrikes() const;
	void setCombatFirstStrikes(int iNewValue);
	void changeCombatFirstStrikes(int iChange);

	DllExport int getCombatDamage() const;
	void setCombatDamage(int iNewValue);

	int getFortifyTurns() const;
	void setFortifyTurns(int iNewValue);
	void changeFortifyTurns(int iChange);

	int getBlitzCount() const;
	DllExport bool isBlitz() const;
	void changeBlitzCount(int iChange);

	int getAmphibCount() const;
	DllExport bool isAmphib() const;
	void changeAmphibCount(int iChange);

	int getRiverCount() const;
	DllExport bool isRiver() const;
	void changeRiverCount(int iChange);

	DllExport int getEnemyRouteCount() const;
	DllExport bool isEnemyRoute() const;
	void changeEnemyRouteCount(int iChange);

	int getAlwaysHealCount() const;
	DllExport bool isAlwaysHeal() const;
	void changeAlwaysHealCount(int iChange);

	int getHillsDoubleMoveCount() const;
	DllExport bool isHillsDoubleMove() const;
	void changeHillsDoubleMoveCount(int iChange);

	int getImmuneToFirstStrikesCount() const;
	void changeImmuneToFirstStrikesCount(int iChange);

	DllExport int getExtraVisibilityRange() const;
	void changeExtraVisibilityRange(int iChange);

	int getExtraMoves() const;
	void changeExtraMoves(int iChange);

	DllExport int getExtraMoveDiscount() const;
	void changeExtraMoveDiscount(int iChange);

	int getExtraAirRange() const;
	void changeExtraAirRange(int iChange);

	int getExtraIntercept() const;
	void changeExtraIntercept(int iChange);

	int getExtraEvasion() const;
	void changeExtraEvasion(int iChange);

	int getExtraFirstStrikes() const;
	void changeExtraFirstStrikes(int iChange);

	int getExtraChanceFirstStrikes() const;
	void changeExtraChanceFirstStrikes(int iChange);

	int getExtraWithdrawal() const;
	void changeExtraWithdrawal(int iChange);

	int getExtraCollateralDamage() const;
	void changeExtraCollateralDamage(int iChange);

	int getExtraBombardRate() const;
	void changeExtraBombardRate(int iChange);

	DllExport int getExtraEnemyHeal() const;
	void changeExtraEnemyHeal(int iChange);

	DllExport int getExtraNeutralHeal() const;
	void changeExtraNeutralHeal(int iChange);

	DllExport int getExtraFriendlyHeal() const;
	void changeExtraFriendlyHeal(int iChange);

	DllExport int getSameTileHeal() const;
	void changeSameTileHeal(int iChange);

	DllExport int getAdjacentTileHeal() const;
	void changeAdjacentTileHeal(int iChange);

	DllExport int getExtraCombatPercent() const;
	void changeExtraCombatPercent(int iChange);

	int getExtraCityAttackPercent() const;
	void changeExtraCityAttackPercent(int iChange);

	int getExtraCityDefensePercent() const;
	void changeExtraCityDefensePercent(int iChange);

	int getExtraHillsAttackPercent() const;
	void changeExtraHillsAttackPercent(int iChange);

	int getExtraHillsDefensePercent() const;
	void changeExtraHillsDefensePercent(int iChange);

	int getRevoltProtection() const;
	void changeRevoltProtection(int iChange);

	int getCollateralDamageProtection() const;
	void changeCollateralDamageProtection(int iChange);

	int getPillageChange() const;
	void changePillageChange(int iChange);

	int getUpgradeDiscount() const;
	void changeUpgradeDiscount(int iChange);

	int getExperiencePercent() const;
	void changeExperiencePercent(int iChange);

	int getKamikazePercent() const;
	void changeKamikazePercent(int iChange);

	DllExport DirectionTypes getFacingDirection(bool checkLineOfSightProperty) const;
	void setFacingDirection(DirectionTypes facingDirection);
	void rotateFacingDirectionClockwise();
	void rotateFacingDirectionCounterClockwise();

	bool isSuicide() const;
	int getDropRange() const;

	bool isMadeAttack() const;
	DllExport void setMadeAttack(bool bNewValue);

	bool isMadeInterception() const;
	DllExport void setMadeInterception(bool bNewValue);

	DllExport bool isPromotionReady() const;
	DllExport void setPromotionReady(bool bNewValue);
	void testPromotionReady();

	bool isDelayedDeath() const;
	void startDelayedDeath();
	bool doDelayedDeath();

	bool isCombatFocus() const;

	DllExport bool isInfoBarDirty() const;
	DllExport void setInfoBarDirty(bool bNewValue);

	bool isBlockading() const;
	void setBlockading(bool bNewValue);
	void collectBlockadeGold();

	DllExport PlayerTypes getOwner() const;
#ifdef _USRDLL
	inline PlayerTypes getOwnerINLINE() const
	{
		return m_eOwner;
	}
#endif
	DllExport PlayerTypes getVisualOwner(TeamTypes eForTeam = NO_TEAM) const;
	DllExport PlayerTypes getCombatOwner(TeamTypes eForTeam, const CvPlot* pPlot) const;
	DllExport TeamTypes getTeam() const;

	PlayerTypes getCapturingPlayer() const;
	void setCapturingPlayer(PlayerTypes eNewValue);

	DllExport const UnitTypes getUnitType() const;
	DllExport CvUnitInfo &getUnitInfo() const;
	UnitClassTypes getUnitClassType() const;

	DllExport const UnitTypes getLeaderUnitType() const;
	void setLeaderUnitType(UnitTypes leaderUnitType);

	DllExport CvUnit* getCombatUnit() const;
	void setCombatUnit(CvUnit* pUnit, bool bAttacking = false);

	DllExport CvUnit* getTransportUnit() const;
	DllExport bool isCargo() const;
	void setTransportUnit(CvUnit* pTransportUnit);

	int getExtraDomainModifier(DomainTypes eIndex) const;
	void changeExtraDomainModifier(DomainTypes eIndex, int iChange);

	DllExport const CvWString getName(uint uiForm = 0) const;
	DllExport const wchar* getNameKey() const;
	DllExport const CvWString getNameNoDesc() const;
	DllExport void setName(const CvWString szNewValue);

	// Script data needs to be a narrow string for pickling in Python
	std::string getScriptData() const;
	void setScriptData(std::string szNewValue);

	int getTerrainDoubleMoveCount(TerrainTypes eIndex) const;
	DllExport bool isTerrainDoubleMove(TerrainTypes eIndex) const;
	void changeTerrainDoubleMoveCount(TerrainTypes eIndex, int iChange);

	int getFeatureDoubleMoveCount(FeatureTypes eIndex) const;
	DllExport bool isFeatureDoubleMove(FeatureTypes eIndex) const;
	void changeFeatureDoubleMoveCount(FeatureTypes eIndex, int iChange);

	int getExtraTerrainAttackPercent(TerrainTypes eIndex) const;
	void changeExtraTerrainAttackPercent(TerrainTypes eIndex, int iChange);
	int getExtraTerrainDefensePercent(TerrainTypes eIndex) const;
	void changeExtraTerrainDefensePercent(TerrainTypes eIndex, int iChange);
	int getExtraFeatureAttackPercent(FeatureTypes eIndex) const;
	void changeExtraFeatureAttackPercent(FeatureTypes eIndex, int iChange);
	int getExtraFeatureDefensePercent(FeatureTypes eIndex) const;
	void changeExtraFeatureDefensePercent(FeatureTypes eIndex, int iChange);

	int getExtraUnitCombatModifier(UnitCombatTypes eIndex) const;
	void changeExtraUnitCombatModifier(UnitCombatTypes eIndex, int iChange);

	bool canAcquirePromotion(PromotionTypes ePromotion) const;
	bool canAcquirePromotionAny() const;
	bool isPromotionValid(PromotionTypes ePromotion) const;
	DllExport bool isHasPromotion(PromotionTypes eIndex) const;
	DllExport void setHasPromotion(PromotionTypes eIndex, bool bNewValue);

	DllExport int getSubUnitCount() const;
	DllExport int getSubUnitsAlive() const;
	int getSubUnitsAlive(int iDamage) const;

	bool isTargetOf(const CvUnit& attacker) const;

	DllExport bool isEnemy(TeamTypes eTeam, const CvPlot* pPlot = NULL) const;
	DllExport bool isPotentialEnemy(TeamTypes eTeam, const CvPlot* pPlot = NULL) const;

	bool canRangeStrike() const;
	bool canRangeStrikeAt(const CvPlot* pPlot, int iX, int iY) const;
	bool rangeStrike(int iX, int iY);

	int getTriggerValue(EventTriggerTypes eTrigger, const CvPlot* pPlot, bool bCheckPlot) const;
	bool canApplyEvent(EventTypes eEvent) const;
	void applyEvent(EventTypes eEvent);

	int getImmobileTimer() const;
	void setImmobileTimer(int iNewValue);
	void changeImmobileTimer(int iChange);

	bool potentialWarAction(const CvPlot* pPlot) const;
	bool willRevealByMove(const CvPlot* pPlot) const;

	bool isAlwaysHostile(const CvPlot* pPlot) const;

	bool verifyStackValid();

	DllExport const CvArtInfoUnit* getArtInfo(int i, EraTypes eEra) const;
	DllExport const TCHAR* getButton() const;

	void read(FDataStreamBase* pStream);
	void write(FDataStreamBase* pStream);

	virtual void AI_init(UnitAITypes eUnitAI) = 0;
	virtual void AI_uninit() = 0;
	virtual void AI_reset(UnitAITypes eUnitAI = NO_UNITAI) = 0;
	virtual bool AI_update() = 0;
	virtual bool AI_follow() = 0;
	virtual void AI_upgrade() = 0;
	virtual void AI_promote() = 0;
	virtual int AI_groupFirstVal() = 0;
	virtual int AI_groupSecondVal() = 0;
	virtual int AI_attackOdds(const CvPlot* pPlot, bool bPotentialEnemy) const = 0;
	virtual bool AI_bestCityBuild(CvCity* pCity, CvPlot** ppBestPlot = NULL, BuildTypes* peBestBuild = NULL, CvPlot* pIgnorePlot = NULL, CvUnit* pUnit = NULL) = 0;
	virtual bool AI_isCityAIType() const = 0;
	virtual UnitAITypes AI_getUnitAIType() const = 0;
	virtual void AI_setUnitAIType(UnitAITypes eNewValue) = 0;
    virtual int AI_sacrificeValue(const CvPlot* pPlot) const = 0;

protected:

	int m_iID;
	int m_iGroupID;
	int m_iHotKeyNumber;
	int m_iX;
	int m_iY;
	int m_iLastMoveTurn;
	int m_iReconX;
	int m_iReconY;
	int m_iGameTurnCreated;
	int m_iDamage;
	int m_iMoves;
	int m_iExperience;
	int m_iLevel;
	int m_iCargo;
	int m_iCargoCapacity;
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
	int m_iExtraAirRange;
	int m_iExtraIntercept;
	int m_iExtraEvasion;
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
	int m_iExtraHillsAttackPercent;
	int m_iExtraHillsDefensePercent;
	int m_iRevoltProtection;
	int m_iCollateralDamageProtection;
	int m_iPillageChange;
	int m_iUpgradeDiscount;
	int m_iExperiencePercent;
	int m_iKamikazePercent;
	int m_iBaseCombat;
	DirectionTypes m_eFacingDirection;
	int m_iImmobileTimer;

	bool m_bMadeAttack;
	bool m_bMadeInterception;
	bool m_bPromotionReady;
	bool m_bDeathDelay;
	bool m_bCombatFocus;
	bool m_bInfoBarDirty;
	bool m_bBlockading;
	bool m_bAirCombat;

	PlayerTypes m_eOwner;
	PlayerTypes m_eCapturingPlayer;
	UnitTypes m_eUnitType;
	UnitTypes m_eLeaderUnitType;
	CvUnitInfo *m_pUnitInfo;

	IDInfo m_combatUnit;
	IDInfo m_transportUnit;

	int* m_aiExtraDomainModifier;

	CvWString m_szName;
	CvString m_szScriptData;

	bool* m_pabHasPromotion;

	int* m_paiTerrainDoubleMoveCount;
	int* m_paiFeatureDoubleMoveCount;
	int* m_paiExtraTerrainAttackPercent;
	int* m_paiExtraTerrainDefensePercent;
	int* m_paiExtraFeatureAttackPercent;
	int* m_paiExtraFeatureDefensePercent;
	int* m_paiExtraUnitCombatModifier;

	bool canAdvance(const CvPlot* pPlot, int iThreshold) const;
	void collateralCombat(const CvPlot* pPlot, CvUnit* pSkipUnit = NULL);
	void flankingStrikeCombat(const CvPlot* pPlot, int iAttackerStrength, int iAttackerFirepower, CvUnit* pSkipUnit = NULL);

	bool interceptTest(const CvPlot* pPlot);
	CvUnit* airStrikeTarget(const CvPlot* pPlot) const;
	bool canAirStrike(const CvPlot* pPlot) const;
	bool airStrike(CvPlot* pPlot);

	int planBattle( CvBattleDefinition & kBattleDefinition ) const;
	int computeUnitsToDie( const CvBattleDefinition & kDefinition, bool bRanged, BattleUnitTypes iUnit ) const;
	bool verifyRoundsValid( const CvBattleDefinition & battleDefinition ) const;
	void increaseBattleRounds( CvBattleDefinition & battleDefinition ) const;
	int computeWaveSize( bool bRangedRound, int iAttackerMax, int iDefenderMax ) const;

	void getDefenderCombatValues(CvUnit& kDefender, const CvPlot* pPlot, int iOurStrength, int iOurFirepower, int& iTheirOdds, int& iTheirStrength, int& iOurDamage, int& iTheirDamage, CombatDetails* pTheirDetails = NULL) const;

	bool isCombatVisible(const CvUnit* pDefender) const;
	void resolveCombat(CvUnit* pDefender, CvPlot* pPlot, CvBattleDefinition& kBattle);
	void resolveAirCombat(CvUnit* pInterceptor, CvPlot* pPlot, CvAirMissionDefinition& kBattle);
};

#endif
