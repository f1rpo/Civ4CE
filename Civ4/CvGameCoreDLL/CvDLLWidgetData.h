#ifndef _CVDLLWIDGETDATA_H_
#define _CVDLLWIDGETDATA_H_

//#include "CvEnums.h"

class CvDLLWidgetData
{

public:

	DllExport static CvDLLWidgetData& getInstance();
	DllExport static void freeInstance();

	//	This will parse the help for the widget
	DllExport void parseHelp(CvWString &szBuffer, CvWidgetDataStruct &widgetDataStruct);

	//	This will execute the action for the widget
	DllExport bool executeAction( CvWidgetDataStruct &widgetDataStruct );

	//	This will execute an alternate action for the widget
	DllExport bool executeAltAction( CvWidgetDataStruct &widgetDataStruct );

	DllExport bool isLink(const CvWidgetDataStruct &widgetDataStruct) const;

	//	Actions to be executed
	void doPlotList(CvWidgetDataStruct &widgetDataStruct);
	void doRenameCity();
	void doRenameUnit();
	void doCreateGroup();
	void doDeleteGroup();
	void doTrain(CvWidgetDataStruct &widgetDataStruct);
	void doConstruct(CvWidgetDataStruct &widgetDataStruct);
	void doCreate(CvWidgetDataStruct &widgetDataStruct);
	void doMaintain(CvWidgetDataStruct &widgetDataStruct);
	void doHurry(CvWidgetDataStruct &widgetDataStruct);
	void doConscript();
	void doAction(CvWidgetDataStruct &widgetDataStruct);
	void doChangeSpecialist(CvWidgetDataStruct &widgetDataStruct);
	void doResearch(CvWidgetDataStruct &widgetDataStruct);
	void doChangePercent(CvWidgetDataStruct &widgetDataStruct);
	void doCityTab(CvWidgetDataStruct &widgetDataStruct);
	void doContactCiv(CvWidgetDataStruct &widgetDataStruct);
	void doConvert(CvWidgetDataStruct &widgetDataStruct);
	void doAutomateCitizens();
	void doAutomateProduction();
	void doEmphasize(CvWidgetDataStruct &widgetDataStruct);
	void doCancelCivics();
	void applyCityEdit();
	void doUnitModel();
	void doFlag();
	void doSelected(CvWidgetDataStruct &widgetDataStruct);
	void doPediaTechJump(CvWidgetDataStruct &widgetDataStruct);
	void doPediaUnitJump(CvWidgetDataStruct &widgetDataStruct);
	void doPediaBuildingJump(CvWidgetDataStruct &widgetDataStruct);
	void doPediaBack();
	void doPediaForward();
	void doPediaBonusJump(CvWidgetDataStruct &widgetDataStruct, bool bData2 = false);
	void doPediaSpecialistJump(CvWidgetDataStruct &widgetDataStruct);
	void doPediaProjectJump(CvWidgetDataStruct &widgetDataStruct);
	void doPediaReligionJump(CvWidgetDataStruct &widgetDataStruct);
	void doPediaTerrainJump(CvWidgetDataStruct &widgetDataStruct);
	void doPediaFeatureJump(CvWidgetDataStruct &widgetDataStruct);
	void doPediaMain(CvWidgetDataStruct &widgetDataStruct);
	void doPediaPromotionJump(CvWidgetDataStruct &widgetDataStruct);
	void doPediaUnitCombatJump(CvWidgetDataStruct &widgetDataStruct);
	void doPediaImprovementJump(CvWidgetDataStruct &widgetDataStruct, bool bData2 = false);
	void doPediaCivicJump(CvWidgetDataStruct &widgetDataStruct);
	void doPediaCivilizationJump(CvWidgetDataStruct &widgetDataStruct);
	void doPediaLeaderJump(CvWidgetDataStruct &widgetDataStruct);
	void doPediaDescription(CvWidgetDataStruct &widgetDataStruct);
	void doGotoTurnEvent(CvWidgetDataStruct &widgetDataStruct);
	void doForeignAdvisor(CvWidgetDataStruct &widgetDataStruct);
	void doPediaConstructJump(CvWidgetDataStruct &widgetDataStruct);
	void doPediaTrainJump(CvWidgetDataStruct &widgetDataStruct);
	void doPediaBuildJump(CvWidgetDataStruct &widgetDataStruct);
	void doDealKill(CvWidgetDataStruct &widgetDataStruct);
	void doRefreshMilitaryAdvisor(CvWidgetDataStruct &widgetDataStruct);
	void doMenu( void );

	//	Help parsing
	void parsePlotListHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseCityNameHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseTrainHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseConstructHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseCreateHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseMaintainHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseHurryHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseConscriptHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseActionHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseCitizenHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseDisabledCitizenHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseAngryCitizenHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseChangeSpecialistHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseResearchHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseTechTreeHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseChangePercentHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseContactCivHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseScoreHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseConvertHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseRevolutionHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
//	void parsePopupQueue(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseAutomateCitizensHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseAutomateProductionHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseEmphasizeHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseTradeItem(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseUnitModelHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseFlagHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseMaintenanceHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseNationalityHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseHealthHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseHappinessHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parsePopulationHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseProductionHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseCultureHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseGreatPeopleHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseSelectedHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseBuildingHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseProjectHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseTerrainHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseFeatureHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseTechEntryHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseTechPrereqHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseTechTreePrereq(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer, bool bTechTreeInfo);
	void parseObsoleteHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseObsoleteBonusString(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseObsoleteSpecialHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseMoveHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseFreeUnitHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseFeatureProductionHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseWorkerRateHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseTradeRouteHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseHealthRateHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseHappinessRateHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseFreeTechHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseLOSHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseMapCenterHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseMapRevealHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseMapTradeHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseTechTradeHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseGoldTradeHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseOpenBordersHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseDefensivePactHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parsePermanentAllianceHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseBuildBridgeHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseIrrigationHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseIgnoreIrrigationHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseWaterWorkHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseBuildHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseDomainExtraMovesHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseAdjustHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseTerrainTradeHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseSpecialBuildingHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseYieldChangeHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseBonusRevealHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseCivicRevealHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseProcessInfoHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseFoundReligionHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseFinanceNumUnits(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseFinanceUnitCost(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseFinanceAwaySupply(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseFinanceCityMaint(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseFinanceCivicUpkeep(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseFinanceForeignIncome(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseFinanceInflatedCosts(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseFinanceGrossIncome(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseFinanceNetGold(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseFinanceGoldReserve(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseUnitHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parsePediaBack(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parsePediaForward(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseBonusHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseReligionHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseReligionHelpCity(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parsePromotionHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseUnitCombatHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseImprovementHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseCivicHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseCivilizationHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseLeaderHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseDescriptionHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer, bool bMinimal);
	void parseCloseScreenHelp(CvWString &szBuffer);
	void parseKillDealHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseProductionModHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseLeaderheadHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseLeaderLineHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);
	void parseCommerceModHelp(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer);

protected:
	
	static CvDLLWidgetData* m_pInst;

};

#endif//_CVDLLWIDGETDATA_H_
