Beyond the Sword 3.17 Unofficial Patch

Version 0.23

CivFanatics Forum: http://forums.civfanatics.com/forumdisplay.php?f=299
SourceForge Project: http://sourceforge.net/projects/civ4btspatch/

This is a modification of the core DLL for the game Civiliztion 4: 
Beyond the Sword. It is a community effort to add fixes and improvements 
beyond the latest Official patch release with intent to fix obvious bugs 
but not drastically alter gameplay.

Primary Authors: Solver, Dresden
Contributors: DanF5771, jdog5000, Bhruic, ViterboKnight, ruff_hi, 
              EmperorFool, grumbler, and the many CFC & Apolyton forum
              members who have made bug reports and/or provided saves.

Source is included but contains only those files which have changed from
the official 3.17 release. All changes should be documented in the code.
The Makefile provided is a slightly-altered version of the 3.17 Makefile
provided by Refar with his Visual Studio debugging setup instructions. 
Ref: http://forums.civfanatics.com/downloads.php?do=file&id=10018

======================================================================
Changes from version 0.21
* Diplomacy summary will no longer claim that a civ is the worst enemy of a human team.
    Ref: http://forums.civfanatics.com/showthread.php?p=7423499#post7423499
    CvGameTextMgr::getOtherRelationsString()
* Fixed problem regarding lightly-damaged AI planes always choosing not to attack. [jdog5000]
    Ref: http://forums.civfanatics.com/showthread.php?p=7431555#post7431555
    CvUnitAI::AI_attackAirMove()
* Groups on sentry now awake based on farthest-seeing unit's vision rather than head unit's vision. [Pep]
    Ref: http://forums.civfanatics.com/showthread.php?t=286855
    CvSelectionGroup::sentryAlert()
* Hidden Nationality units can now enter friendly rival territory (without open borders) when starting move in a city. [Pep]
    Ref: http://forums.civfanatics.com/showthread.php?t=286367
    CvUnit::canEnterTerritory()
* Fixed bug where a disloyal AP member could capture a loyal member's city and retain extra hammer bonuses in that city.
    Ref: http://forums.civfanatics.com/showthread.php?p=7387707#post7387707
    CvPlayer::acquireCity()
* Fixed bug with colonies who occupy recycled player slots showing the old leader or civ names.
    Ref: 
    CvGame::addPlayer()

Changes between version 0.21 and version 0.19.1
* Fixed bug where AI Great Engineers would hurry wonders in rival cities. [DanF5771]
    Ref: http://forums.civfanatics.com/showthread.php?t=287128
    CvUnitAI::AI_switchHurry()
* Fixed bug where some AI unit types mistakenly ignored the value of first-strike promotions. [ViterboKnight]
    Ref: http://forums.civfanatics.com/showthread.php?t=292614
    CvUnitAI::AI_promotionValue()
* Fixed team-counting bug that would cause some mapscripts to fail. [DanF5771]
    Ref: http://forums.civfanatics.com/showthread.php?t=287260
    CvGame::countCivTeamsEverAlive()
* Fixed bug where Glance screen would include part of player's name in attitude calculation. [EmperorFool/BUG]
    Ref: http://forums.civfanatics.com/showthread.php?t=270358
    (Python) CvExoticForeignAdvisor.py
* Fixed rare bug where Info screen Score graph would break if using debug mode and viewing an unmet AI.
    Ref: http://forums.civfanatics.com/showthread.php?p=7316414#post7316414
    (Python) CvInfoScreen.py
* Fixed bug with CyUnit::getRevoltProtection() returning the wrong value.
    CyUnit::getRevoltProtection()
* Fixed bug in player closeness calculations causing the AI to be blind to its actual proximity to other players. [jdog5000/BetterAI]
    Ref: http://forums.civfanatics.com/showthread.php?p=6951257#post6951257
    CvPlayerAI::AI_playerCloseness()
* Modified how closeness is used for determining who to attack, different settings for regular and Aggressive AI. [jdog5000/BetterAI]
	Ref: http://forums.civfanatics.com/showthread.php?p=6951257#post6951257
	CvTeamAI::AI_startWarVal()
* Spies really no longer interrupt their mission when moving next to an enemy unit.
    Ref: http://forums.civfanatics.com/showthread.php?p=7221054#post7221054
    CvSelectionGroup::doTurn()
* Modified AI civic valuation to more accurately consider state religion preference.
    Ref: http://forums.civfanatics.com/showthread.php?t=289173
    CvPlayerAI::AI_civicValue()
* Certain situations which invalidated Secretary General/AP Resident will now force the next vote to be an election.
    Ref: http://forums.civfanatics.com/showthread.php?t=296328
    CvGame::doVoteResults(); CvGame::clearSecretaryGeneral()
* Limited which production modifiers affect gold from production overflow.
    Ref: http://forums.civfanatics.com/showthread.php?t=295048
    CvCity::popOrder()
* Building or removing a fort will now force a plotgroup update to verify resource connections.
    Ref: http://forums.civfanatics.com/showthread.php?t=295332
    CvPlot::setImprovementType()
* Obsolete resources no longer considered worthless in trade; their value is now controlled by the BONUS_OBSOLETE_VALUE_MODIFIER Global Define.
  In the supplied GlobalDefines.xml, this is 150 meaning a 50% markup; 100 would be the same value as before obsolete, 0 would mean worthless.
    Ref: http://forums.civfanatics.com/showthread.php?t=295589
    CvPlayerAI::AI_baseBonusVal(); GlobalDefines.xml
* AI will now only redline its actual Favorite Civic with DENIAL_FAVORITE_CIVIC rather than all of them. [DanF5771]
    Ref: http://forums.civfanatics.com/showthread.php?p=6997817#post6997817
    CvPlayerAI::AI_civicTrade()
* Conquistadors now ignore city walls just like the Cuirassiers which they replace.
    Ref: http://forums.civfanatics.com/showthread.php?p=7323414#post7323414
    (XML) CvUnitInfos.xml
* Fixed Team Battleground to distribute hills better and workaround team-counting problem. [ruff_hi]
    Ref: http://forums.civfanatics.com/showthread.php?t=287808
    (PublicMaps) Team_Battleground.py
* Tweaked plane retreat logic to avoid unnecessary mission check.
    CvUnitAI::AI_attackAirMove()
* Added jdog5000's AIAutoPlay changes (SDK only) to help with testing.
    Ref: http://forums.civfanatics.com/showthread.php?t=174812
    CvGame::setAIAutoPlay(); CvGame::setWinnder(); CvPlayer::reset(); CvPlayer::updateHuman()
    CvPlayer class: added m_bDisableHuman, setHumanDisabled() and isHumanDisabled()
* Added global context functions isUnofficialPatch() and getUnofficialPatchVersion() which are exposed to Python.
    CvGlobals::isUnofficialPatch(); CvGlobals::getUnofficialPatchVersion(); 
    CyGlobalContext::isUnofficialPatch(); CyGlobalContext::getUnofficialPatchVersion(); CyGlobalContextPythonInterface1()
* Added (English-only) patch version string to flag hover text. Idea borrowed from BetterAI mod.
    CvDLLWidgetData::parseFlagHelp(); UnofficialPatch.h

Changes between version 0.19.1 and version 0.19
* Fixed problem with AI Bombers not attacking. Fix is from Better AI mod [jdog5000]
    Ref: http://forums.civfanatics.com/showpost.php?p=7080987&postcount=20
    CvUnitAI::AI_attackAirMove()
* Fixed collateral damage calculation related to defensive modifiers like Bunkers & Drill 2+ units. [DanF5771]
    Ref: http://forums.civfanatics.com/showpost.php?p=6981159&postcount=2
    CvUnit::collateralCombat()
* Fixed bug where AI uses player's ID instead of attitude when evaluating trade deals. [DanF5771]
    Ref: http://forums.civfanatics.com/showpost.php?p=7022936&postcount=14
    CvPlayerAI::AI_civicTrade(); CvPlayerAI::AI_bonusTrade; CvPlayerAI::AI_religionTrade
* Fixed bug where AI misinterprets the civicPercentAnger attribute when evaluating trade deals.
    Ref: http://forums.civfanatics.com/showpost.php?p=7124523&postcount=22
    CvPlayerAI::AI_civicTrade()
* Fixed espionage spread culture mission to insert the listed 5% of culture rather than the current .05%
    Ref: http://forums.civfanatics.com/showthread.php?p=7127772#post7127772
    CvPlayer::doEspionageMission()
* AI will change religion during Golden Ages.
    CvPlayerAI::AI_doReligion()
* Re-enabled first-pass randomizer in start location forestation code to allow potential resource placement as per Bhruic's patch for 3.13
    Ref: http://forums.civfanatics.com/showpost.php?p=6199072&postcount=1310
    CvGame::normalizeAddExtras()
* Foreign advisor no longer shows Favorite Civics when playing under Random Personalities
    (Python) CvExoticForeignAdvisor.py

Changes between 0.19 and the Official 3.17 patch (order is bottom to top):
* AI plane/ship retreating logic when city is in danger extended to Forts
* Vassal's spies no longer caught in master's territory 
    CvUnit::isIntruding()
* AI logic on banning nukes tweaked to account for AI leader personality and the global situation
    CvPlayerAI::AI_diploVote()
* Undid galley production changes from Better BtS AI (see **)
* Non-native collateral damage tweaked (thanks guys)
    CvUnit::collateralCombat()
* Civilopedia will now display "replaced by" lines for buildings that get replaced by UBs
    CvGameTextMgr::setBuildingHelp()
* AI guided missiles will lean towards destroying resource-producing tiles as opposed to improvements like Towns
    CvUnitAI::AI_airBombPlots()
* Damaged AI attack planes may choose to continue attacking if no defending interceptors are around
    CvUnitAI::AI_attackAirMove()
* Rivers running through deserts will result in floodplains
    CvGame::normalizeAddRiver()
* Start location tweaks per SevenSpirits
    CvGame::normalizeAddExtras(); CvGame::normalizeAddFoodBonuses()
* Workers not as lazy, per Bhruic's patch
    CvUnitAI::AI_workerMove()
* Glance screen reactivated
    (Python) CvExoticForeignAdvisor.py
* AI will no longer settle Great People in cities that are in danger of being captured
    CvUnitAI::AI_join()
* AI will try to retreat air assets from endangered cities
    CvUnitAI::AI_attackAirMove(); CvUnitAI::AI_defenseAirMove()
* AI will no longer refuse to retreat damaged planes
    CvUnitAI::AI_attackAirMove()
* AI will change civics during Golden Ages
    CvPlayerAI::AI_doCivics()
* Revised collateral damage formula for non-native collateral units, if modded in
    CvUnit::collateralCombat()
* Fixed bug in the map generator with water starts
    CvGame::normalizeAddExtras()
* AI may now try to use Warlords to create super-medic units
    CvUnitAI::AI_promotionValue(); CvUnitAI::AI_lead()
* Fixed bug that prevented random events that target a unit from triggering
    CvPlayer::pickTriggerUnit(); CvUnit::getTriggerValue()
* Access to the Barrage line removed from armored units. If units with access to Barrage and no inherent collateral damage are modded in, they will work.
    (XML) CvPromotionInfos.xml
* Fixed AI defender bug for newly captured cities.
    CvCityAI::AI_neededDefenders()
** Fixed overproduction of transports in land wars courtesy of better BtS AI team.
* Made the AI more likely to use missiles on cruisers/subs to strike against enemy improvements.
    CvUnitAI::AI_missileAirMove()
* Made AI ships prioritize retreating if docked in a city which is in the danger of imminent capture by enemy.
    CvUnitAI::AI_attackSeaMove(); CvUnitAI::AI_reserveSeaMove(); CvUnitAI::AI_escortSeaMove();
    CvUnitAI::AI_exploreSeaMove(); CvUnitAI::AI_carrierSeaMove(); CvUnitAI::AI_missileCarrierSeaMove()
* Civilopedia and mouseover help for units that cause collateral damage will now show the collateral damage limit (50% for Catapults, etc.)
    CvGameTextMgr::setUnitHelp(); CvGameTextMgr::setBasicUnitHelp()
* Somewhat experimental AI change: Aggressive AI now considerably less likely to want peace if it poses more of an immediate threat to enemy cities than it currently faces itself.
    CvTeamAI::AI_endWarVal()
* Fixed unavailability of foreign civ demographics if playing with espionage disabled.
    (Python) CvInfoScreen.py
* Barrage promotions made working again on Tanks and other units with no base collateral ability
    CvUnit::collateralCombat()
