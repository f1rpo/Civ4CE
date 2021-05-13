#pragma once

#ifndef CvDLLEventReporterIFaceBase_h
#define CvDLLEventReporterIFaceBase_h

//
// abstract class containing event reporting functions that the DLL needs
// Creator- Mustafa Thamer
//

class CvUnit;
class CvCity;
class CvPlot;
class CvSelectionGroup;

class CvDLLEventReporterIFaceBase
{
public:
	virtual void genericEvent(const char* szEventName, void* pythonArgs=NULL) = 0;
	virtual void mouseEvent(int evt, const POINT& ptCursor) = 0;
	virtual void kbdEvent(int evt, int key) = 0;

	virtual void gameEnd() = 0;

	virtual void beginGameTurn(int iGameTurn) = 0;
	virtual void endGameTurn(int iGameTurn) = 0;

	virtual void beginPlayerTurn(int iGameTurn, PlayerTypes) = 0;
	virtual void endPlayerTurn(int iGameTurn, PlayerTypes) = 0;
	virtual void firstContact(TeamTypes eTeamID1, TeamTypes eTeamID2) = 0;
	virtual void combatResult(CvUnit* pWinner, CvUnit* pLoser) = 0;
	virtual void improvementBuilt(int iImprovementType, int iX, int iY) = 0;
	virtual void improvementDestroyed(int iImprovementType, int iPlayer, int iX, int iY) = 0;
	virtual void routeBuilt(int RouteType, int iX, int iY) = 0;

	virtual void plotRevealed(CvPlot *pPlot, TeamTypes eTeam) = 0;
	virtual void plotFeatureRemoved(CvPlot *pPlot, FeatureTypes eFeature, CvCity* pCity) = 0;
	virtual void plotPicked(CvPlot *pPlot) = 0;
	virtual void gotoPlotSet(CvPlot *pPlot, PlayerTypes ePlayer) = 0;

	virtual void cityBuilt(CvCity *pCity) = 0;
	virtual void cityRazed(CvCity *pCity, PlayerTypes ePlayer) = 0;
	virtual void cityAcquired(PlayerTypes eOldOwner, PlayerTypes ePlayer, CvCity* pCity, bool bConquest, bool bTrade) = 0;
	virtual void cityAcquiredAndKept(PlayerTypes ePlayer, CvCity* pCity) = 0;
	virtual void cityLost(CvCity *pCity) = 0;
	virtual void cultureExpansion( CvCity *pCity, PlayerTypes ePlayer) = 0;
	virtual void cityGrowth(CvCity *pCity, PlayerTypes ePlayer) = 0;
	virtual void cityDoTurn(CvCity *pCity, PlayerTypes ePlayer) = 0;
	virtual void cityBuildingUnit(CvCity* pCity, UnitTypes eUnitType) = 0;
	virtual void cityBuildingBuilding(CvCity* pCity, BuildingTypes eBuildingType) = 0;
	virtual void cityRename(CvCity* pCity) = 0;
	virtual void createTradeRoute(PlayerTypes ePlayer) = 0;
	virtual void editTradeRoute(PlayerTypes ePlayer, int iRoute) = 0;
	virtual void cityHurry(CvCity* pCity, HurryTypes eHurry) = 0;

	virtual void selectionGroupPushMission(CvSelectionGroup* pSelectionGroup, MissionTypes eMission) = 0;
	virtual void unitMove(CvPlot* pPlot, CvUnit* pUnit, CvPlot* pOldPlot) = 0;
	virtual void unitSetXY(CvPlot* pPlot, CvUnit* pUnit) = 0;
	virtual void unitCreated(CvUnit *pUnit) = 0;
	virtual void unitBuilt(CvCity *pCity, CvUnit *pUnit) = 0;
	virtual void unitKilled(CvUnit *pUnit, PlayerTypes eAttacker) = 0;
	virtual void unitLost(CvUnit *pUnit) = 0;
	virtual void unitPromoted(CvUnit *pUnit, PromotionTypes ePromotion) = 0;
	virtual void unitSelected(CvUnit *pUnit) = 0;
	virtual void missionaryConvertedUnit(CvUnit *pUnit) = 0;
	virtual void unitRename(CvUnit* pUnit) = 0;
	virtual void unitPillage(CvUnit* pUnit, ImprovementTypes eImprovement, RouteTypes eRoute, PlayerTypes ePlayer) = 0;
	virtual void unitGifted(CvUnit* pUnit, PlayerTypes eGiftingPlayer, CvPlot* pPlotLocation) = 0;
	virtual void unitBuildImprovement(CvUnit* pUnit, BuildTypes eBuild, bool bFinished) = 0;

	virtual void goodyReceived(PlayerTypes ePlayer, CvPlot *pGoodyPlot, CvUnit *pGoodyUnit, GoodyTypes eGoodyType) = 0;
	virtual void buildingBuilt(CvCity *pCity, BuildingTypes eBuilding) = 0;
	virtual void changeWar(bool bWar, TeamTypes eTeam, TeamTypes eOtherTeam) = 0;

	virtual void setPlayerAlive(PlayerTypes ePlayerID, bool bNewValue) = 0;
	virtual void playerGoldTrade(PlayerTypes eFromPlayer, PlayerTypes eToPlayer, int iAmount) = 0;

	virtual void chat(char *szString) = 0;

	virtual void victory(TeamTypes eNewWinner, VictoryTypes eNewVictory) = 0;

	virtual void yieldSoldToEurope(PlayerTypes ePlayer, YieldTypes eYield, int iAmount) = 0;
	virtual void yieldBoughtFromEurope(PlayerTypes ePlayer, YieldTypes eYield, int iAmount) = 0;
	virtual void unitBoughtFromEurope(PlayerTypes ePlayer, int iUnitId) = 0;
	virtual void unitTravelStateChanged(PlayerTypes ePlayer, UnitTravelStates eState, int iUnitId) = 0;
	virtual void emmigrantAtDocks(PlayerTypes ePlayer, int iUnitId) = 0;
	virtual void populationJoined(PlayerTypes ePlayer, int iCityId, int iUnitId) = 0;
	virtual void populationUnjoined(PlayerTypes ePlayer, int iCityId, int iUnitId) = 0;
	virtual void unitLearned(PlayerTypes ePlayer, int iUnitId) = 0;
	virtual void yieldProduced(PlayerTypes ePlayer, int iCityId, YieldTypes eYield) = 0;
};

#endif	// CvDLLEventReporterIFaceBase_h
