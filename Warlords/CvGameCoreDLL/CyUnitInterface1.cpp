#include "CvGameCoreDLL.h"
#include "CyUnit.h"
#include "CyCity.h"
#include "CyPlot.h"
#include "CyArea.h"
#include "CySelectionGroup.h"

//# include <boost/python/manage_new_object.hpp>
//# include <boost/python/return_value_policy.hpp>

//
// published python interface for CyUnit
//

void CyUnitPythonInterface1(python::class_<CyUnit>& x)
{
	OutputDebugString("Python Extension Module - CyUnitPythonInterface1\n");

	x
		.def("isNone", &CyUnit::isNone, "bool () - Is this a valid unit instance?")
		.def("convert", &CyUnit::convert, "void (CyUnit* pUnit)")
		.def("kill", &CyUnit::kill, "void (bool bDelay, int /*PlayerTypes*/ ePlayer)")
		.def("NotifyEntity", &CyUnit::NotifyEntity, "void (int EntityEventType)")
		
		.def("isActionRecommended", &CyUnit::isActionRecommended, "int (int i)")
		.def("isBetterDefenderThan", &CyUnit::isBetterDefenderThan, "bool (CyUnit* pDefender, CyUnit* pAttacker)")

		.def("canDoCommand", &CyUnit::canDoCommand, "bool (eCommand, iData1, iData2, bTestVisible = False) - can the unit perform eCommand?")
		.def("doCommand", &CyUnit::doCommand, "void (eCommand, iData1, iData2) - force the unit to perform eCommand")

		.def("getPathEndTurnPlot", &CyUnit::getPathEndTurnPlot, python::return_value_policy<python::manage_new_object>(), "CyPlot* ()")
		.def("generatePath", &CyUnit::generatePath, "bool (CyPlot* pToPlot, int iFlags = 0, bool bReuse = false, int* piPathTurns = NULL)")

		.def("canEnterTerritory", &CyUnit::canEnterTerritory, "bool (int (TeamTypes) eTeam, bool bIgnoreRightOfPassage)")
		.def("getDeclareWarMove", &CyUnit::getDeclareWarMove, "TeamTypes (CyPlot* pPlot)")
		.def("canMoveInto", &CyUnit::canMoveInto, "bool (CyPlot* pPlot, bool bAttack, bool bDeclareWar, bool bIgnoreLoad)")
		.def("canMoveOrAttackInto", &CyUnit::canMoveOrAttackInto, "bool (CyPlot* pPlot, bool bDeclareWar)")
		.def("canMoveThrough", &CyUnit::canMoveThrough, "bool (CyPlot* pPlot)")
		.def("jumpToNearestValidPlot", &CyUnit::jumpToNearestValidPlot, "void ()")

		.def("canAutomate", &CyUnit::canAutomate, "bool (int (AutomateTypes) eAutomate)")
		.def("canScrap", &CyUnit::canScrap, "bool ()")
		.def("canGift", &CyUnit::canGift, "bool (bool bTestVisible)")
		.def("canLoadUnit", &CyUnit::canLoadUnit, "bool (CyUnit* pUnit, CyPlot* pPlot)")
		.def("canLoad", &CyUnit::canLoad, "bool ()")
		.def("canUnload", &CyUnit::canUnload, "bool ()")
		.def("canUnloadAll", &CyUnit::canUnloadAll, "bool ()")
		.def("canHold", &CyUnit::canHold, "bool (CyPlot* pPlot)")
		.def("canSleep", &CyUnit::canSleep, "bool (CyPlot* pPlot)")
		.def("canFortify", &CyUnit::canFortify, "bool (CyPlot* pPlot)")
		.def("canPatrol", &CyUnit::canAirPatrol, "bool (CyPlot* pPlot)")
		.def("canHeal", &CyUnit::canHeal, "bool (CyPlot* pPlot)")
		.def("canSentry", &CyUnit::canSentry, "bool (CyPlot* pPlot)")

		.def("canAirlift", &CyUnit::canAirlift, "bool (CyPlot* pPlot)")
		.def("canAirliftAt", &CyUnit::canAirliftAt, "bool (CyPlot* pPlot, int iX, int iY)")

		.def("isNukeVictim", &CyUnit::isNukeVictim, "bool (CyPlot* pPlot, int eTeam)")
		.def("canNuke", &CyUnit::canNuke, "bool ()")
		.def("canNukeAt", &CyUnit::canNukeAt, "bool (CyPlot* pPlot, int iX, int iY)")

		.def("canRecon", &CyUnit::canRecon, "bool ()")
		.def("canReconAt", &CyUnit::canReconAt, "bool (CyPlot* pPlot, int iX, int iY)")

		.def("canAirBomb", &CyUnit::canAirBomb, "bool ()")
		.def("canAirBombAt", &CyUnit::canAirBombAt, "bool (CyPlot* pPlot, int iX, int iY)")

		.def("bombardTarget", &CyUnit::bombardTarget, python::return_value_policy<python::manage_new_object>(), "CyCity* (CyPlot* pPlot)")
		.def("canBombard", &CyUnit::canBombard, "bool (CyPlot* pPlot)")

		.def("canPillage", &CyUnit::canPillage, "bool (CyPlot* pPlot)")
		.def("sabotageCost", &CyUnit::sabotageCost, "int (CyPlot* pPlot)")
		.def("sabotageProb", &CyUnit::sabotageProb, "int (CyPlot* pPlot, int /*ProbabilityTypes*/ eProbStyle)")
		.def("canSabotage", &CyUnit::canSabotage, "bool (CyPlot* pPlot, bool bTestVisible)")
		.def("destroyCost", &CyUnit::destroyCost, "int (CyPlot* pPlot)")
		.def("destroyProb", &CyUnit::destroyProb, "int (CyPlot* pPlot, int /*ProbabilityTypes*/ eProbStyle)")
		.def("canDestroy", &CyUnit::canDestroy, "bool (CyPlot* pPlot, bool bTestVisible)")
		.def("stealPlansCost", &CyUnit::stealPlansCost, "int (CyPlot* pPlot)")
		.def("stealPlansProb", &CyUnit::stealPlansProb, "int (CyPlot* pPlot, int /*ProbabilityTypes*/ eProbStyle)")
		.def("canStealPlans", &CyUnit::canStealPlans, "bool (CyPlot* pPlot, bool bTestVisible)")

		.def("canFound", &CyUnit::canFound, "bool (CyPlot* pPlot, bool bTestVisible)")
		.def("canSpread", &CyUnit::canSpread, "bool (CyPlot* pPlot, int /*ReligionTypes*/ eReligion, bool bTestVisible)")
		.def("canJoin", &CyUnit::canJoin, "bool (CyPlot* pPlot, int (SpecialistTypes) eSpecialist)")
		.def("canConstruct", &CyUnit::canConstruct, "bool (CyPlot* pPlot, int (BuildingTypes) eBuilding)")

		.def("getDiscoveryTech", &CyUnit::getDiscoveryTech, "int /*TechTypes*/ ()")
		.def("getDiscoverResearch", &CyUnit::getDiscoverResearch, "int (int /*TechTypes*/ eTech)")
		.def("canDiscover", &CyUnit::canDiscover, "bool (CyPlot* pPlot)")
		.def("getMaxHurryProduction", &CyUnit::getMaxHurryProduction, "int (CyCity* pCity)")
		.def("getHurryProduction", &CyUnit::getHurryProduction, "int (CyPlot* pPlot)")
		.def("canHurry", &CyUnit::canHurry, "bool (CyPlot* pPlot, bool bTestVisible)")
		.def("getTradeGold", &CyUnit::getTradeGold, "int (CyPlot* pPlot)")
		.def("canTrade", &CyUnit::canTrade, "bool (CyPlot* pPlot, bool bTestVisible)")
		.def("getGreatWorkCulture", &CyUnit::getGreatWorkCulture, "int (CyPlot* pPlot)")
		.def("canGreatWork", &CyUnit::canGreatWork, "bool (CyPlot* pPlot)")
		.def("canGoldenAge", &CyUnit::canGoldenAge, "bool (CyPlot* pPlot, bool bTestVisible)")
		.def("canBuild", &CyUnit::canBuild, "bool (CyPlot* pPlot, int (BuildTypes) eBuild, bool bTestVisible)")

		.def("canPromote", &CyUnit::canPromote, "bool (ePromotion)")
		.def("promote", &CyUnit::promote, "bool (ePromotion)")

		.def("upgradePrice", &CyUnit::upgradePrice, "int (UnitTypes eUnit)")
		.def("upgradeAvailable", &CyUnit::upgradeAvailable, "bool (int /*UnitTypes*/ eFromUnit, int /*UnitClassTypes*/ eToUnitClass, int iCount)")
		.def("canUpgrade", &CyUnit::canUpgrade, "bool (int /*UnitTypes*/ eUnit, bool bTestVisible)")

		.def("getHandicapType", &CyUnit::getHandicapType, "int ()")
		.def("getCivilizationType", &CyUnit::getCivilizationType, "int ()")
		.def("getSpecialUnitType", &CyUnit::getSpecialUnitType, "int ()")
		.def("getCaptureUnitType", &CyUnit::getCaptureUnitType, "int (int /*CivilizationTypes*/ eCivilization)")
		.def("getUnitCombatType", &CyUnit::getUnitCombatType, "int ()")
		.def("getDomainType", &CyUnit::getDomainType, "int ()")
		.def("getInvisibleType", &CyUnit::getInvisibleType, "int (InvisibleTypes) ()")
		.def("getSeeInvisibleType", &CyUnit::getSeeInvisibleType, "int (InvisibleTypes) ()")

		.def("flavorValue", &CyUnit::flavorValue, "int (FlavorTypes eFlavor)")
		.def("isBarbarian", &CyUnit::isBarbarian, "bool ()")
		.def("isHuman", &CyUnit::isHuman, "bool ()")
		.def("visibilityRange", &CyUnit::visibilityRange, "int ()")
		.def("baseMoves", &CyUnit::baseMoves, "int ()")
		.def("movesLeft", &CyUnit::movesLeft, "int ()")

		.def("maxMoves", &CyUnit::maxMoves, "int ()")
		.def("canMove", &CyUnit::canMove, "bool ()")
		.def("hasMoved", &CyUnit::hasMoved, "bool ()")
		.def("airRange", &CyUnit::airRange, "int ()")
		.def("nukeRange", &CyUnit::nukeRange, "int ()")

		.def("canBuildRoute", &CyUnit::canBuildRoute, "bool ()")
		.def("getBuildType", &CyUnit::getBuildType, "int (BuildTypes) ()")
		.def("workRate", &CyUnit::workRate, "int (bool bMax)")

		.def("isAnimal", &CyUnit::isAnimal, "bool ()")
		.def("isNoBadGoodies", &CyUnit::isNoBadGoodies, "bool ()")
		.def("isOnlyDefensive", &CyUnit::isOnlyDefensive, "bool ()")
		.def("isNoCapture", &CyUnit::isNoCapture, "bool ()")
		.def("isRivalTerritory", &CyUnit::isRivalTerritory, "bool ()")
		.def("isMilitaryHappiness", &CyUnit::isMilitaryHappiness, "bool ()")
		.def("isInvestigate", &CyUnit::isInvestigate, "bool ()")
		.def("isCounterSpy", &CyUnit::isCounterSpy, "bool ()")
		.def("isFound", &CyUnit::isFound, "bool ()")
		.def("isGoldenAge", &CyUnit::isGoldenAge, "bool ()")

		.def("isFighting", &CyUnit::isFighting, "bool ()")
		.def("isAttacking", &CyUnit::isAttacking, "bool ()")
		.def("isDefending", &CyUnit::isDefending, "bool ()")
		.def("isCombat", &CyUnit::isCombat, "bool ()")
		.def("maxHitPoints", &CyUnit::maxHitPoints, "bool ()")
		.def("currHitPoints", &CyUnit::currHitPoints, "bool ()")
		.def("isHurt", &CyUnit::isHurt, "bool ()")
		.def("isDead", &CyUnit::isDead, "bool ()")
		.def("baseCombatStr", &CyUnit::baseCombatStr, "int ()")
		.def("maxCombatStr", &CyUnit::maxCombatStr, "int (CyPlot* pPlot, CyUnit* pAttacker)")
		.def("currCombatStr", &CyUnit::currCombatStr, "int (CyPlot* pPlot, CyUnit* pAttacker)")
		.def("currFirepower", &CyUnit::currFirepower, "int (CyPlot* pPlot, CyUnit* pAttacker)")
		.def("maxCombatStrFloat", &CyUnit::maxCombatStrFloat, "float (CyPlot* pPlot, CyUnit* pAttacker)")
		.def("currCombatStrFloat", &CyUnit::currCombatStrFloat, "float (CyPlot* pPlot, CyUnit* pAttacker)")

		.def("canFight", &CyUnit::canFight, "bool ()")
		.def("canAttack", &CyUnit::canAttack, "bool ()")
		.def("canDefend", &CyUnit::canDefend, "bool (CyPlot* pPlot)")
		.def("canSiege", &CyUnit::canSiege, "bool (int /*TeamTypes*/ eTeam)")

		.def("airBaseCombatStr", &CyUnit::airBaseCombatStr, "int ()")
		.def("airMaxCombatStr", &CyUnit::airMaxCombatStr, "int ()")
		.def("airCurrCombatStr", &CyUnit::airCurrCombatStr, "int ()")
		.def("airMaxCombatStrFloat", &CyUnit::airMaxCombatStrFloat, "float ()")
		.def("airCurrCombatStrFloat", &CyUnit::airCurrCombatStrFloat, "float ()")
		.def("airCombatLimit", &CyUnit::airCombatLimit, "int ()")
		.def("canAirAttack", &CyUnit::canAirAttack, "bool ()")
		.def("canAirDefend", &CyUnit::canAirDefend, "bool (CyPlot*)")
		.def("airCombatDamage", &CyUnit::airCombatDamage, "int (CyUnit* pDefender)")
		.def("bestInterceptor", &CyUnit::bestInterceptor, python::return_value_policy<python::manage_new_object>(), "CyUnit* (CyPlot*)")

		.def("isAutomated", &CyUnit::isAutomated, "bool ()")
		.def("isWaiting", &CyUnit::isWaiting, "bool ()")
		.def("isFortifyable", &CyUnit::isFortifyable, "bool ()")
		.def("fortifyModifier", &CyUnit::fortifyModifier, "int ()")
		.def("experienceNeeded", &CyUnit::experienceNeeded, "int ()") 
		.def("attackXPValue", &CyUnit::attackXPValue, "int ()") 
		.def("defenseXPValue", &CyUnit::defenseXPValue, "int ()")
		.def("maxXPValue", &CyUnit::maxXPValue, "int ()")
		.def("firstStrikes", &CyUnit::firstStrikes, "int ()")
		.def("chanceFirstStrikes", &CyUnit::chanceFirstStrikes, "int ()")
		.def("maxFirstStrikes", &CyUnit::maxFirstStrikes, "int ()")
		.def("isRanged", &CyUnit::isRanged, "bool ()")
		.def("alwaysInvisible", &CyUnit::alwaysInvisible, "bool ()")

		.def("immuneToFirstStrikes", &CyUnit::immuneToFirstStrikes, "bool ()")
		.def("noDefensiveBonus", &CyUnit::noDefensiveBonus, "bool ()")
		.def("ignoreBuildingDefense", &CyUnit::ignoreBuildingDefense, "bool ()")
		.def("canMoveImpassable", &CyUnit::canMoveImpassable, "bool ()")
		.def("flatMovementCost", &CyUnit::flatMovementCost, "bool ()")
		.def("ignoreTerrainCost", &CyUnit::ignoreTerrainCost, "bool ()")
		.def("isNeverInvisible", &CyUnit::isNeverInvisible, "bool ()")
		.def("isInvisible", &CyUnit::isInvisible, "bool (int (TeamTypes) eTeam, bool bDebug)")
		.def("isNukeImmune", &CyUnit::isNukeImmune, "bool ()")

		.def("maxInterceptionProbability", &CyUnit::maxInterceptionProbability, "int ()")
		.def("currInterceptionProbability", &CyUnit::currInterceptionProbability, "int ()")
		.def("evasionProbability", &CyUnit::evasionProbability, "int ()")
		.def("withdrawalProbability", &CyUnit::withdrawalProbability, "int ()")
		.def("collateralDamage", &CyUnit::collateralDamage, "int ()")
		.def("collateralDamageLimit", &CyUnit::collateralDamageLimit, "int ()")
		.def("collateralDamageMaxUnits", &CyUnit::collateralDamageMaxUnits, "int ()")

		.def("cityAttackModifier", &CyUnit::cityAttackModifier, "int ()")
		.def("cityDefenseModifier", &CyUnit::cityDefenseModifier, "int ()")
		.def("animalCombatModifier", &CyUnit::animalCombatModifier, "int ()")
		.def("hillsDefenseModifier", &CyUnit::hillsDefenseModifier, "int ()")
		.def("terrainDefenseModifier", &CyUnit::terrainDefenseModifier, "int (int /*TerrainTypes*/ eTerrain)")
		.def("featureDefenseModifier", &CyUnit::featureDefenseModifier, "int (int /*FeatureTypes*/ eFeature)")
		.def("unitClassAttackModifier", &CyUnit::unitClassAttackModifier, "int (int /*UnitClassTypes*/ eUnitClass)")
		.def("unitClassDefenseModifier", &CyUnit::unitClassDefenseModifier, "int (int /*UnitClassTypes*/ eUnitClass)")
		.def("unitCombatModifier", &CyUnit::unitCombatModifier, "int (int /*UnitCombatTypes*/ eUnitCombat)")
		.def("domainModifier", &CyUnit::domainModifier, "int (int /*DomainTypes*/ eDomain)") 

		.def("bombardRate", &CyUnit::bombardRate, "int ()") 
		.def("airBombBaseRate", &CyUnit::airBombBaseRate, "int ()")
		.def("airBombCurrRate", &CyUnit::airBombCurrRate, "int ()")

		.def("specialCargo", &CyUnit::specialCargo, "int ()") 
		.def("domainCargo", &CyUnit::domainCargo, "int ()") 
		.def("cargoSpace", &CyUnit::cargoSpace, "int ()")
		.def("isFull", &CyUnit::isFull, "bool ()")
		.def("cargoSpaceAvailable", &CyUnit::cargoSpaceAvailable, "int ()")
		.def("hasCargo", &CyUnit::hasCargo, "bool ()")
		.def("canCargoAllMove", &CyUnit::canCargoAllMove, "bool ()")
		.def("getUnitAICargo", &CyUnit::getUnitAICargo, "int (int (UnitAITypes) eUnitAI)")
		.def("getID", &CyUnit::getID, "int ()")

		.def("getGroupID", &CyUnit::getGroupID, "int ()")
		.def("isInGroup", &CyUnit::isInGroup, "bool ()")
		.def("isGroupHead", &CyUnit::isGroupHead, "bool ()")
		.def("getGroup", &CyUnit::getGroup, python::return_value_policy<python::manage_new_object>(), "CySelectionGroup* ()")

		.def("getHotKeyNumber", &CyUnit::getHotKeyNumber, "int () - returns the HotKey number for this unit")
		.def("setHotKeyNumber", &CyUnit::setHotKeyNumber, "void (int iNewValue)")

		.def("getX", &CyUnit::getX, "int ()")
		.def("getY", &CyUnit::getY, "int ()")
		.def("setXY", &CyUnit::setXY, "int (int iX, int iY)")
		.def("at", &CyUnit::at, "bool (int iX, int iY)")
		.def("atPlot", &CyUnit::atPlot, "bool (CyPlot* pPlot)")
		.def("plot", &CyUnit::plot, python::return_value_policy<python::manage_new_object>(), "CyPlot* ()")
		.def("area", &CyUnit::area, python::return_value_policy<python::manage_new_object>(), "CyArea* ()")
		.def("getReconPlot", &CyUnit::getReconPlot, python::return_value_policy<python::manage_new_object>(), "CyPlot* ()")
		.def("getReconRange", &CyUnit::getReconRange, "int ()")
		.def("setReconPlot", &CyUnit::setReconPlot, "void (CyPlot, int)")				 

		.def("getGameTurnCreated", &CyUnit::getGameTurnCreated, "int ()")

		.def("getDamage", &CyUnit::getDamage, "int ()")
		.def("setDamage", &CyUnit::setDamage, "void (int iNewValue, int /*PlayerTypes*/ ePlayer)")
		.def("changeDamage", &CyUnit::changeDamage, "void (int iChange, int /*PlayerTypes*/ ePlayer)")
		.def("getMoves", &CyUnit::getMoves, "int ()")
		.def("setMoves", &CyUnit::setMoves, "void (int iNewValue)")
		.def("changeMoves", &CyUnit::changeMoves, "void (int iChange)")
		.def("finishMoves", &CyUnit::finishMoves, "void ()")
		.def("getExperience", &CyUnit::getExperience, "int ()")
		.def("setExperience", &CyUnit::setExperience, "void (int iNewValue)")
		.def("changeExperience", &CyUnit::changeExperience, "void (int iChange)")
		.def("getLevel", &CyUnit::getLevel, "int ()")
		.def("setLevel", &CyUnit::setLevel)
		.def("changeLevel", &CyUnit::changeLevel)
		.def("getCargo", &CyUnit::getCargo, "int ()")
		.def("getFortifyTurns", &CyUnit::getFortifyTurns, "int ()")
		.def("getBlitzCount", &CyUnit::getBlitzCount, "int ()")
		.def("isBlitz", &CyUnit::isBlitz, "bool ()")
		.def("getAmphibCount", &CyUnit::getAmphibCount, "int ()")
		.def("isAmphib", &CyUnit::isAmphib, "bool ()")
		.def("getRiverCount", &CyUnit::getRiverCount, "int ()")
		.def("isRiver", &CyUnit::isRiver, "bool ()")
		.def("isEnemyRoute", &CyUnit::isEnemyRoute, "bool ()")
		.def("isAlwaysHeal", &CyUnit::isAlwaysHeal, "bool ()")
		.def("isHillsDoubleMove", &CyUnit::isHillsDoubleMove, "bool ()")

		.def("getExtraVisibilityRange", &CyUnit::getExtraVisibilityRange, "int ()")
		.def("getExtraMoves", &CyUnit::getExtraMoves, "int ()")
		.def("getExtraMoveDiscount", &CyUnit::getExtraMoveDiscount, "int ()")
		.def("getExtraFirstStrikes", &CyUnit::getExtraFirstStrikes, "int ()")
		.def("getExtraChanceFirstStrikes", &CyUnit::getExtraChanceFirstStrikes, "int ()")
		.def("getExtraWithdrawal", &CyUnit::getExtraWithdrawal, "int ()")
		.def("getExtraCollateralDamage", &CyUnit::getExtraCollateralDamage, "int ()")
		.def("getExtraEnemyHeal", &CyUnit::getExtraEnemyHeal, "int ()")
		.def("getExtraNeutralHeal", &CyUnit::getExtraNeutralHeal, "int ()")
		.def("getExtraFriendlyHeal", &CyUnit::getExtraFriendlyHeal, "int ()")

		.def("getSameTileHeal", &CyUnit::getSameTileHeal, "int ()")
		.def("getAdjacentTileHeal", &CyUnit::getAdjacentTileHeal, "int ()")

		.def("getExtraCombatPercent", &CyUnit::getExtraCombatPercent, "int ()")
		.def("getExtraCityAttackPercent", &CyUnit::getExtraCityAttackPercent, "int ()")
		.def("getExtraCityDefensePercent", &CyUnit::getExtraCityDefensePercent, "int ()")
		.def("getExtraHillsDefensePercent", &CyUnit::getExtraHillsDefensePercent, "int ()")

		.def("isMadeAttack", &CyUnit::isMadeAttack, "bool ()")
		.def("setMadeAttack", &CyUnit::setMadeAttack, "void (int iNewValue)")
		.def("isMadeInterception", &CyUnit::isMadeInterception, "bool ()")
		.def("setMadeInterception", &CyUnit::setMadeInterception, "void (int iNewValue)")

		.def("isPromotionReady", &CyUnit::isPromotionReady, "bool ()")
		.def("setPromotionReady", &CyUnit::setPromotionReady, "void (int iNewValue)")
		.def("getOwner", &CyUnit::getOwner, "int ()")
		.def("getTeam", &CyUnit::getTeam, "int ()")

		.def("getUnitType", &CyUnit::getUnitType, "int ()")
		.def("getUnitClassType", &CyUnit::getUnitClassType, "int ()")

		.def("getTransportUnit", &CyUnit::getTransportUnit, python::return_value_policy<python::manage_new_object>(), "CyUnit* ()")
		.def("isCargo", &CyUnit::isCargo, "bool ()")

		.def("getExtraDomainModifier", &CyUnit::getExtraDomainModifier, "int ()")

		.def("getName", &CyUnit::getName, "str () - Returns the name of a unit along with its type description in parens if using a custom name")
		.def("getNameForm", &CyUnit::getNameForm, "str (int iForm)")
		.def("getNameKey", &CyUnit::getNameKey, "str ()")
		.def("getNameNoDesc", &CyUnit::getNameNoDesc, "str () - Returns the name of a unit without any description afterwards")
		.def("setName", &CyUnit::setName, "void (str)")
		.def("getScriptData", &CyUnit::getScriptData, "str ()")
		.def("setScriptData", &CyUnit::setScriptData, "void (str)")

		.def("isTerrainDoubleMove", &CyUnit::isTerrainDoubleMove, "bool (TerrainType)")
		.def("isFeatureDoubleMove", &CyUnit::isFeatureDoubleMove, "bool (FeatureType)")

		.def("getExtraTerrainDefensePercent", &CyUnit::getExtraTerrainDefensePercent, "int ()")
		.def("getExtraFeatureDefensePercent", &CyUnit::getExtraFeatureDefensePercent, "int ()")
		.def("getExtraUnitCombatModifier", &CyUnit::getExtraUnitCombatModifier, "int ()")

		.def("canAcquirePromotion", &CyUnit::canAcquirePromotion, "bool (int /*PromotionTypes*/ ePromotion)")
		.def("canAcquirePromotionAny", &CyUnit::canAcquirePromotionAny, "bool ()")
		.def("isHasPromotion", &CyUnit::isHasPromotion, "bool (int /*PromotionTypes*/ ePromotion)")
		.def("setHasPromotion", &CyUnit::setHasPromotion, "void (int (PromotionTypes) eIndex, bool bNewValue)")
		.def("IsSelected", &CyUnit::IsSelected)

		.def("getUnitAIType", &CyUnit::getUnitAIType, "int UnitAIType () - returns the int value of the UnitAIType")
		.def("setUnitAIType", &CyUnit::setUnitAIType, "void UnitAIType (int iUnitAIType) - sets the unit's UnitAIType")

		// Python Helper Functions
		.def("centerCamera", &CyUnit::centerCamera, "void () - Centers the Camera on the unit")
		;
}