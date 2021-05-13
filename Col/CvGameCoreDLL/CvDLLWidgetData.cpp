#include "CvGameCoreDLL.h"
#include "CvGameCoreUtils.h"
#include "CvCity.h"
#include "CvUnit.h"
#include "CvGlobals.h"
#include "CvGameAI.h"
#include "CvMap.h"
#include "CvPlot.h"
#include "CvTeamAI.h"
#include "CvPlayerAI.h"
#include "CvGameCoreUtils.h"
#include "CyArgsList.h"
#include "CvGameTextMgr.h"
#include "CvDLLInterfaceIFaceBase.h"
#include "CvDLLPythonIFaceBase.h"
#include "CvDLLEngineIFaceBase.h"
#include "CvDLLEventReporterIFaceBase.h"
#include "CvDLLWidgetData.h"
#include "CvPopupInfo.h"
#include "FProfiler.h"

CvDLLWidgetData* CvDLLWidgetData::m_pInst = NULL;

CvDLLWidgetData& CvDLLWidgetData::getInstance()
{
	if (m_pInst == NULL)
	{
		m_pInst = new CvDLLWidgetData;
	}
	return *m_pInst;
}

void CvDLLWidgetData::freeInstance()
{
	delete m_pInst;
	m_pInst = NULL;
}

void CvDLLWidgetData::parseHelp(CvWStringBuffer &szBuffer, CvWidgetDataStruct &widgetDataStruct)
{
	switch (widgetDataStruct.m_eWidgetType)
	{
	case WIDGET_PLOT_LIST:
		parsePlotListHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PLOT_LIST_SHIFT:
		if (widgetDataStruct.m_iData1 != 0)
		{
			szBuffer.assign(gDLL->getText("TXT_KEY_MISC_CTRL_SHIFT", (GC.getDefineINT("MAX_PLOT_LIST_SIZE") - 1)));
		}
		break;

	case WIDGET_CITY_SCROLL:
		break;

	case WIDGET_LIBERATE_CITY:
		parseLiberateCityHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_CITY_NAME:
		parseCityNameHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_UNIT_NAME:
		szBuffer.append(gDLL->getText("TXT_KEY_CHANGE_NAME"));
		break;

	case WIDGET_CREATE_GROUP:
		szBuffer.append(gDLL->getText("TXT_KEY_WIDGET_CREATE_GROUP"));
		break;

	case WIDGET_DELETE_GROUP:
		szBuffer.append(gDLL->getText("TXT_KEY_WIDGET_DELETE_GROUP"));
		break;

	case WIDGET_TRAIN:
		parseTrainHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_CONSTRUCT:
		parseConstructHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_CONVINCE:
		parseConvinceHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HURRY:
		parseHurryHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PLAYER_HURRY:
		parsePlayerHurryHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_MENU_ICON:
		szBuffer.append(gDLL->getText("TXT_KEY_MAIN_MENU"));
		break;

	case WIDGET_ACTION:
		parseActionHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_CITIZEN:
		parseCitizenHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_CONTACT_CIV:
		parseContactCivHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_SCORE_BREAKDOWN:
		parseScoreHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_CREATE_TRADE_ROUTE:
		szBuffer.append(gDLL->getText("TXT_KEY_CREATE_TRADE_ROUTE"));
		break;

	case WIDGET_EDIT_TRADE_ROUTE:
		szBuffer.append(gDLL->getText("TXT_KEY_EDIT_TRADE_ROUTE"));
		break;

	case WIDGET_YIELD_IMPORT_EXPORT:
		parseImportExportHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_EJECT_CITIZEN:
		parseEjectCitizenHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_SHIP_CARGO:
		parseShipCargoUnitHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_DOCK:
		parseEuropeUnitHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_SAIL:
		szBuffer.append(gDLL->getText("TXT_KEY_SAIL"));
		break;

	case WIDGET_GOTO_CITY:
		{
			CvCity* pCity = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCity(widgetDataStruct.m_iData1);
			if (pCity != NULL)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_GO_TO_CITY", pCity->getNameKey()));
			}
		}
		break;

	case WIDGET_ASSIGN_CITIZEN_TO_PLOT:
		break;

	case WIDGET_ZOOM_CITY:
		szBuffer.append(gDLL->getText("TXT_KEY_ZOOM_CITY_HELP"));
		break;

	case WIDGET_END_TURN:
		szBuffer.append(gDLL->getText("TXT_KEY_WIDGET_END_TURN"));
		break;

	case WIDGET_AUTOMATE_CITIZENS:
		parseAutomateCitizensHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_AUTOMATE_PRODUCTION:
		parseAutomateProductionHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_EMPHASIZE:
		parseEmphasizeHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_TRADE_ITEM:
		parseTradeItem(widgetDataStruct, szBuffer);
		break;

	case WIDGET_UNIT_MODEL:
		parseUnitModelHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_DEFENSE:
		parseCityDefenseHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_POPULATION:
		parsePopulationHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_REBEL:
		parseRebelHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_GREAT_GENERAL:
		parseGreatGeneralHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_SELECTED:
		parseSelectedHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_UNIT:
		parseUnitHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_PROFESSION:
		parseProfessionHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_BUILDING:
		parseBuildingHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_BACK:
		// parsePediaBack(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_FORWARD:
		// parsePediaForward(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_BONUS:
		parseBonusHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_MAIN:
		break;

	case WIDGET_PEDIA_JUMP_TO_PROMOTION:
	case WIDGET_HELP_PROMOTION:
		parsePromotionHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_UNIT_PROMOTION:
		parseUnitPromotionHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_YIELD:
		parseCityYieldHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_CHOOSE_EVENT:
		parseEventHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_IMPROVEMENT:
		parseImprovementHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_CIVIC:
		parseCivicHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_CIV:
		parseCivilizationHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_LEADER:
		parseLeaderHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_TERRAIN:
		parseTerrainHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_YIELDS:
		parseYieldHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_FEATURE:
		parseFeatureHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_FATHER:
		parseFatherHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_DESCRIPTION:
		parseDescriptionHelp(widgetDataStruct, szBuffer, false);
		break;

	case WIDGET_CLOSE_SCREEN:
		//parseCloseScreenHelp(szBuffer);
		break;

	case WIDGET_DEAL_KILL:
		parseKillDealHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_DESCRIPTION_NO_HELP:
		//parseDescriptionHelp(widgetDataStruct, szBuffer, true);
		break;

	case WIDGET_MINIMAP_HIGHLIGHT:
		break;

	case WIDGET_PRODUCTION_MOD_HELP:
		parseProductionModHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_LEADERHEAD:
		parseLeaderheadHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_LEADER_LINE:
		parseLeaderLineHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_CITY_UNIT_ASSIGN_PROFESSION:
		parseSpecialBuildingHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_MOVE_CARGO_TO_TRANSPORT:
		parseCityYieldHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_ASSIGN_TRADE_ROUTE:
		parseAssignTradeRoute(widgetDataStruct, szBuffer);
		break;

	case WIDGET_RECEIVE_MOVE_CARGO_TO_TRANSPORT:
		parseReceiveMoveCargoToTransportHelp(widgetDataStruct, szBuffer);
		break;

	}
}

// Protected Functions...
bool CvDLLWidgetData::executeAction( CvWidgetDataStruct &widgetDataStruct )
{
	bool bHandled = false;			//	Right now general bHandled = false;  We can specific case this to true later.  Game will run with this = false;

	switch (widgetDataStruct.m_eWidgetType)
	{

	case WIDGET_PLOT_LIST:
		doPlotList(widgetDataStruct);
		break;

	case WIDGET_PLOT_LIST_SHIFT:
		if (widgetDataStruct.m_iData1 == 0)
		{
			gDLL->getInterfaceIFace()->toggleMultiRowPlotList();
		}
		else
		{
			gDLL->getInterfaceIFace()->changePlotListColumn(widgetDataStruct.m_iData1 * ((gDLL->ctrlKey()) ? (GC.getDefineINT("MAX_PLOT_LIST_SIZE") - 1) : 1));
		}
		break;

	case WIDGET_CITY_SCROLL:
		if ( widgetDataStruct.m_iData1 > 0 )
		{
			GC.getGameINLINE().doControl(CONTROL_NEXTCITY);
		}
		else
		{
			GC.getGameINLINE().doControl(CONTROL_PREVCITY);
		}
		break;

	case WIDGET_LIBERATE_CITY:
		doLiberateCity();
		break;

	case WIDGET_CITY_NAME:
		doRenameCity();
		break;

	case WIDGET_UNIT_NAME:
		doRenameUnit();
		break;

	case WIDGET_CREATE_GROUP:
		doCreateGroup();
		break;

	case WIDGET_DELETE_GROUP:
		doDeleteGroup();
		break;

	case WIDGET_TRAIN:
		doTrain(widgetDataStruct);
		break;

	case WIDGET_CONSTRUCT:
		doConstruct(widgetDataStruct);
		break;

	case WIDGET_CONVINCE:
		doConvince(widgetDataStruct);
		break;

	case WIDGET_HURRY:
		doHurry(widgetDataStruct);
		break;

	case WIDGET_PLAYER_HURRY:
		doPlayerHurry(widgetDataStruct);
		break;

	case WIDGET_MENU_ICON:
		doMenu();
		break;

	case WIDGET_ACTION:
		doAction(widgetDataStruct);
		break;

	case WIDGET_CITIZEN:
		if (widgetDataStruct.m_iData2 != -1)
		{
			doDoubleClickCitizen(widgetDataStruct);
		}
		break;

	case WIDGET_CONTACT_CIV:
		doContactCiv(widgetDataStruct);
		break;

	case WIDGET_END_TURN:
		GC.getGameINLINE().doControl(CONTROL_FORCEENDTURN);
		break;

	case WIDGET_AUTOMATE_CITIZENS:
		doAutomateCitizens(widgetDataStruct);
		break;

	case WIDGET_AUTOMATE_PRODUCTION:
		doAutomateProduction();
		break;

	case WIDGET_EMPHASIZE:
		doEmphasize(widgetDataStruct);
		break;

	case WIDGET_DIPLOMACY_RESPONSE:
		break;

	case WIDGET_TRADE_ITEM:
		break;

	case WIDGET_UNIT_MODEL:
		doUnitModel();
		break;

	case WIDGET_HELP_SELECTED:
		doSelected(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_UNIT:
		if (widgetDataStruct.m_iData2 != 1)
		{
			doPediaUnitJump(widgetDataStruct);
		}
		break;

	case WIDGET_PEDIA_JUMP_TO_PROFESSION:
		doPediaProfessionJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_BUILDING:
		doPediaBuildingJump(widgetDataStruct);
		break;
	case WIDGET_PEDIA_BACK:
		doPediaBack();
		break;
	case WIDGET_PEDIA_FORWARD:
		doPediaForward();
		break;

	case WIDGET_PEDIA_JUMP_TO_BONUS:
		doPediaBonusJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_MAIN:
		doPediaMain(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_PROMOTION:
		doPediaPromotionJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_IMPROVEMENT:
		doPediaImprovementJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_CIVIC:
		doPediaCivicJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_CIV:
		doPediaCivilizationJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_LEADER:
		doPediaLeaderJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_TERRAIN:
		doPediaTerrainJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_YIELDS:
		doPediaYieldJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_FEATURE:
		doPediaFeatureJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_FATHER:
		if (widgetDataStruct.m_iData2 != 1)
		{
			doPediaFatherJump(widgetDataStruct);
		}
		break;

	case WIDGET_PEDIA_DESCRIPTION:
	case WIDGET_PEDIA_DESCRIPTION_NO_HELP:
		doPediaDescription(widgetDataStruct);
		break;

	case WIDGET_TURN_EVENT:
		doGotoTurnEvent(widgetDataStruct);
		break;

	case WIDGET_DEAL_KILL:
		doDealKill(widgetDataStruct);
		break;

	case WIDGET_MINIMAP_HIGHLIGHT:
		doRefreshMilitaryAdvisor(widgetDataStruct);
		break;

	case WIDGET_CHOOSE_EVENT:
		break;

	case WIDGET_ZOOM_CITY:
		break;
	case WIDGET_HELP_YIELD:
	case WIDGET_HELP_PROMOTION:
	case WIDGET_HELP_UNIT_PROMOTION:
	case WIDGET_LEADERHEAD:
	case WIDGET_LEADER_LINE:
	case WIDGET_CLOSE_SCREEN:
	case WIDGET_SCORE_BREAKDOWN:
	case WIDGET_ASSIGN_CITIZEN_TO_PLOT:
		//	Nothing on clicked
		break;
	case WIDGET_CREATE_TRADE_ROUTE:
		doCreateTradeRoute(widgetDataStruct);
		break;
	case WIDGET_EDIT_TRADE_ROUTE:
		doEditTradeRoute(widgetDataStruct);
		break;
	case WIDGET_YIELD_IMPORT_EXPORT:
		doYieldImportExport(widgetDataStruct);
		break;
	case WIDGET_ASSIGN_TRADE_ROUTE:
		doAssignTradeRoute(widgetDataStruct);
		break;
	case WIDGET_GOTO_CITY:
		doGoToCity(widgetDataStruct);
		break;

	}

	return bHandled;
}

//	right clicking action
bool CvDLLWidgetData::executeAltAction( CvWidgetDataStruct &widgetDataStruct )
{
	CvWidgetDataStruct widgetData = widgetDataStruct;

	bool bHandled = true;
	switch (widgetDataStruct.m_eWidgetType)
	{
	case WIDGET_TRAIN:
		doPediaTrainJump(widgetDataStruct);
		break;
	case WIDGET_CONSTRUCT:
		doPediaConstructJump(widgetDataStruct);
		break;
	case WIDGET_PEDIA_JUMP_TO_UNIT:
		doPediaUnitJump(widgetDataStruct);
		break;
	case WIDGET_PEDIA_JUMP_TO_PROFESSION:
		doPediaProfessionJump(widgetDataStruct);
		break;
	case WIDGET_PEDIA_JUMP_TO_BUILDING:
		doPediaBuildingJump(widgetDataStruct);
		break;
	case WIDGET_PEDIA_JUMP_TO_PROMOTION:
		doPediaPromotionJump(widgetDataStruct);
		break;
	case WIDGET_PEDIA_JUMP_TO_FATHER:
		doPediaFatherJump(widgetDataStruct);
		break;
	case WIDGET_LEADERHEAD:
		doContactCiv(widgetDataStruct);
		break;

	default:
		bHandled = false;
		break;
	}

	return (bHandled);
}

bool CvDLLWidgetData::executeDropOn(const CvWidgetDataStruct& destinationWidgetData, const CvWidgetDataStruct& sourceWidgetData)
{
	bool bHandled = true;
	switch (sourceWidgetData.m_eWidgetType)
	{
	case WIDGET_CITIZEN:
		doCityUnitAssignCitizen(destinationWidgetData, sourceWidgetData);
		break;

	case WIDGET_MOVE_CARGO_TO_TRANSPORT:
		doMoveCargoToTransport(destinationWidgetData, sourceWidgetData);
		break;

	case WIDGET_MOVE_CARGO_TO_CITY:
		doMoveCargoToCity(destinationWidgetData, sourceWidgetData);
		break;

	case WIDGET_SHIP_CARGO:
		doMoveShipCargo(destinationWidgetData, sourceWidgetData);
		break;

	case WIDGET_DOCK:
		doMoveDockUnit(destinationWidgetData, sourceWidgetData);
		break;

	case WIDGET_EJECT_CITIZEN:
		doUnitIntoCity(destinationWidgetData, sourceWidgetData);

	default:
		bHandled = false;
		break;
	}

	return (bHandled);
}

//	right clicking action
bool CvDLLWidgetData::executeDoubleClick(const CvWidgetDataStruct& widgetDataStruct)
{
	bool bHandled = true;
	switch (widgetDataStruct.m_eWidgetType)
	{
	case WIDGET_CITIZEN:
		doDoubleClickCitizen(widgetDataStruct);
		break;

	case WIDGET_DOCK:
		doDoubleClickDock(widgetDataStruct);
		break;

	default:
		bHandled = false;
		break;
	}

	return (bHandled);
}

bool CvDLLWidgetData::isLink(const CvWidgetDataStruct &widgetDataStruct) const
{
	bool bLink = false;
	switch (widgetDataStruct.m_eWidgetType)
	{
	case WIDGET_PEDIA_JUMP_TO_BUILDING:
	case WIDGET_PEDIA_JUMP_TO_UNIT:
	case WIDGET_PEDIA_JUMP_TO_PROFESSION:
	case WIDGET_PEDIA_JUMP_TO_PROMOTION:
	case WIDGET_PEDIA_JUMP_TO_BONUS:
	case WIDGET_PEDIA_JUMP_TO_IMPROVEMENT:
	case WIDGET_PEDIA_JUMP_TO_CIVIC:
	case WIDGET_PEDIA_JUMP_TO_CIV:
	case WIDGET_PEDIA_JUMP_TO_LEADER:
	case WIDGET_PEDIA_JUMP_TO_TERRAIN:
	case WIDGET_PEDIA_JUMP_TO_YIELDS:
	case WIDGET_PEDIA_JUMP_TO_FEATURE:
	case WIDGET_PEDIA_JUMP_TO_FATHER:
	case WIDGET_PEDIA_FORWARD:
	case WIDGET_PEDIA_BACK:
	case WIDGET_PEDIA_MAIN:
	case WIDGET_TURN_EVENT:
	case WIDGET_PEDIA_DESCRIPTION:
	case WIDGET_PEDIA_DESCRIPTION_NO_HELP:
	case WIDGET_MINIMAP_HIGHLIGHT:
		bLink = (widgetDataStruct.m_iData1 >= 0);
		break;
	case WIDGET_DEAL_KILL:
		{
			CvDeal* pDeal = GC.getGameINLINE().getDeal(widgetDataStruct.m_iData1);
			bLink = (NULL != pDeal && pDeal->isCancelable(GC.getGameINLINE().getActivePlayer()));
		}
		break;
	case WIDGET_GENERAL:
		bLink = (1 == widgetDataStruct.m_iData1);
		break;
	}
	return (bLink);
}


void CvDLLWidgetData::doPlotList(CvWidgetDataStruct &widgetDataStruct)
{
	PROFILE_FUNC();

	CvUnit* pUnit;
	int iUnitIndex = widgetDataStruct.m_iData1 + gDLL->getInterfaceIFace()->getPlotListColumn() - gDLL->getInterfaceIFace()->getPlotListOffset();

	CvPlot *selectionPlot = gDLL->getInterfaceIFace()->getSelectionPlot();
	pUnit = gDLL->getInterfaceIFace()->getInterfacePlotUnit(selectionPlot, iUnitIndex);

	if (pUnit != NULL)
	{
		if (pUnit->getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
		{
			if(!gDLL->getInterfaceIFace()->isCityScreenUp())
			{
				gDLL->getInterfaceIFace()->selectGroup(pUnit, gDLL->shiftKey(), gDLL->ctrlKey(), gDLL->altKey());
			}
			else
			{
				pUnit->joinCity();
			}
		}
	}
}


void CvDLLWidgetData::doLiberateCity()
{
	GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_DO_TASK, TASK_LIBERATE, 0);

	gDLL->getInterfaceIFace()->clearSelectedCities();
}


void CvDLLWidgetData::doRenameCity()
{
	CvCity* pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	if (pHeadSelectedCity != NULL)
	{
		if (pHeadSelectedCity->getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
		{
			gDLL->getEventReporterIFace()->cityRename(pHeadSelectedCity);
		}
	}
}


void CvDLLWidgetData::doRenameUnit()
{
	CvUnit* pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();
	if (pHeadSelectedUnit != NULL)
	{
		if (pHeadSelectedUnit->getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
		{
			gDLL->getEventReporterIFace()->unitRename(pHeadSelectedUnit);
		}
	}
}


void CvDLLWidgetData::doCreateGroup()
{
	GC.getGameINLINE().selectionListGameNetMessage(GAMEMESSAGE_JOIN_GROUP);
}


void CvDLLWidgetData::doDeleteGroup()
{
	GC.getGameINLINE().selectionListGameNetMessage(GAMEMESSAGE_JOIN_GROUP, -1, -1, -1, 0, false, true);
}


void CvDLLWidgetData::doTrain(CvWidgetDataStruct &widgetDataStruct)
{
	UnitTypes eUnit = (UnitTypes) GC.getCivilizationInfo(GC.getGameINLINE().getActiveCivilizationType()).getCivilizationUnits(widgetDataStruct.m_iData1);

	if (widgetDataStruct.m_iData2 != FFreeList::INVALID_INDEX)
	{
		gDLL->sendPushOrder(widgetDataStruct.m_iData2, ORDER_TRAIN, eUnit, false, false, false);
	}
	else
	{
		GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_PUSH_ORDER, ORDER_TRAIN, eUnit, -1, false, gDLL->altKey(), gDLL->shiftKey(), gDLL->ctrlKey());
	}
}


void CvDLLWidgetData::doConstruct(CvWidgetDataStruct &widgetDataStruct)
{
	BuildingTypes eBuilding;

	eBuilding = ((BuildingTypes)(GC.getCivilizationInfo(GC.getGameINLINE().getActiveCivilizationType()).getCivilizationBuildings(widgetDataStruct.m_iData1)));

	if (widgetDataStruct.m_iData2 != FFreeList::INVALID_INDEX)
	{
		gDLL->sendPushOrder(widgetDataStruct.m_iData2, ORDER_CONSTRUCT, eBuilding, false, false, false);
	}
	else
	{
		GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_PUSH_ORDER, ORDER_CONSTRUCT, eBuilding, -1, false, gDLL->altKey(), gDLL->shiftKey(), gDLL->ctrlKey());
	}
}

void CvDLLWidgetData::doConvince(CvWidgetDataStruct &widgetDataStruct)
{
	FatherPointTypes eFatherPointType = (FatherPointTypes) widgetDataStruct.m_iData1;

	if (widgetDataStruct.m_iData2 != FFreeList::INVALID_INDEX)
	{
		gDLL->sendPushOrder(widgetDataStruct.m_iData2, ORDER_CONVINCE, eFatherPointType, false, false, false);
	}
	else
	{
		GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_PUSH_ORDER, ORDER_CONVINCE, eFatherPointType, -1, false, gDLL->altKey(), gDLL->shiftKey(), gDLL->ctrlKey());
	}
}


void CvDLLWidgetData::doHurry(CvWidgetDataStruct &widgetDataStruct)
{
	GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_DO_TASK, TASK_HURRY, widgetDataStruct.m_iData1);
}


void CvDLLWidgetData::doPlayerHurry(CvWidgetDataStruct &widgetDataStruct)
{
	gDLL->sendPlayerAction(GC.getGameINLINE().getActivePlayer(), PLAYER_ACTION_HURRY, widgetDataStruct.m_iData1, widgetDataStruct.m_iData2, -1);
}


void CvDLLWidgetData::doAction(CvWidgetDataStruct &widgetDataStruct)
{
	GC.getGameINLINE().handleAction(widgetDataStruct.m_iData1);
}

void CvDLLWidgetData::doContactCiv(CvWidgetDataStruct &widgetDataStruct)
{
	if (gDLL->isDiplomacy() || gDLL->isMPDiplomacyScreenUp())
	{
		return;
	}

	//	Do not execute this if we are trying to contact ourselves...
	if (GC.getGameINLINE().getActivePlayer() == widgetDataStruct.m_iData1)
	{
		doFlag();
		return;
	}

	if (gDLL->shiftKey())
	{
		if (GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).isHuman())
		{
			if (widgetDataStruct.m_iData1 != GC.getGameINLINE().getActivePlayer())
			{
				gDLL->getInterfaceIFace()->showTurnLog((ChatTargetTypes)widgetDataStruct.m_iData1);
			}
		}
		return;
	}

	if (gDLL->altKey())
	{
		if (GET_TEAM(GC.getGameINLINE().getActiveTeam()).canDeclareWar(GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).getTeam()))
		{
			gDLL->sendChangeWar(GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).getTeam(), true);
		}
		return;
	}

	GET_PLAYER(GC.getGameINLINE().getActivePlayer()).contact((PlayerTypes)widgetDataStruct.m_iData1);
}

void CvDLLWidgetData::doAutomateCitizens(CvWidgetDataStruct &widgetDataStruct)
{
	CvCity* pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	if (pHeadSelectedCity != NULL)
	{
		if (pHeadSelectedCity->getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
		{
			GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_DO_TASK, TASK_SET_ALL_CITIZENS_AUTOMATED, widgetDataStruct.m_iData1, -1);
		}
	}
}

void CvDLLWidgetData::doAutomateProduction()
{
	CvCity* pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	if (pHeadSelectedCity != NULL)
	{
		if (pHeadSelectedCity->getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
		{
			GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_DO_TASK, TASK_SET_AUTOMATED_PRODUCTION, -1, -1, !pHeadSelectedCity->isProductionAutomated(), gDLL->altKey(), gDLL->shiftKey(), gDLL->ctrlKey());
		}
	}
}

EmphasizeTypes CvDLLWidgetData::getNextEmphasis(CvCity* pCity, YieldTypes eYield)
{
	int iCurrentEmphasis = pCity->AI_getEmphasizeYieldCount(eYield);
	if (iCurrentEmphasis == 0)
	{
		// No current emphasis - return positive emphasis
		for (int i = 0; i < GC.getNumEmphasizeInfos(); i++)
		{
			EmphasizeTypes eLoopEmphasis = (EmphasizeTypes) i;
			int iYieldChange = GC.getEmphasizeInfo(eLoopEmphasis).getYieldChange(eYield);
			if (iYieldChange > 0)
			{
				return eLoopEmphasis;
			}
		}
	}
	else if (iCurrentEmphasis > 0)
	{
		// Positive current emphasis - return negative emphasis
		for (int i = 0; i < GC.getNumEmphasizeInfos(); i++)
		{
			EmphasizeTypes eLoopEmphasis = (EmphasizeTypes) i;
			int iYieldChange = GC.getEmphasizeInfo(eLoopEmphasis).getYieldChange(eYield);
			if (iYieldChange < 0)
			{
				return eLoopEmphasis;
			}
		}
	}

	// Negative current emphasis - return no emphasis
	return NO_EMPHASIZE;
}


void CvDLLWidgetData::doEmphasize(CvWidgetDataStruct &widgetDataStruct)
{
	CvCity* pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	YieldTypes eYield = (YieldTypes) widgetDataStruct.m_iData1;
	if (pHeadSelectedCity != NULL)
	{
		if (eYield == NO_YIELD)
		{
			EmphasizeTypes eEmphasize = (EmphasizeTypes) widgetDataStruct.m_iData2;
			if (eEmphasize != NO_EMPHASIZE)
			{
				GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_DO_TASK, TASK_SET_EMPHASIZE, eEmphasize, -1, !(pHeadSelectedCity->AI_isEmphasize(eEmphasize)));
			}
			else
			{
				for (int iEmphasize = 0; iEmphasize < GC.getNumEmphasizeInfos(); ++iEmphasize)
				{
					GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_DO_TASK, TASK_SET_EMPHASIZE, iEmphasize, -1, false);
				}
			}
		}
		else
		{
			EmphasizeTypes eNextEmphasis = getNextEmphasis(pHeadSelectedCity, eYield);
			for (int i = 0; i < GC.getNumEmphasizeInfos(); i++)
			{
				// turn off all emphasis except eNextEmphasis
				if (GC.getEmphasizeInfo((EmphasizeTypes) i).getYieldChange(eYield) != 0)
				{
					GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_DO_TASK, TASK_SET_EMPHASIZE, i, -1, i == eNextEmphasis);
				}
			}
		}
	}
}

void CvDLLWidgetData::doUnitModel()
{
	if (gDLL->getInterfaceIFace()->isFocused())
	{
		//	Do NOT execute if a screen is up...
		return;
	}

	gDLL->getInterfaceIFace()->lookAtSelectionPlot();
}


void CvDLLWidgetData::doFlag()
{
	GC.getGameINLINE().doControl(CONTROL_SELECTCAPITAL);
}

void CvDLLWidgetData::doSelected(CvWidgetDataStruct &widgetDataStruct)
{
	CvCity* pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	if (pHeadSelectedCity != NULL)
	{
		GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_POP_ORDER, widgetDataStruct.m_iData1);
	}
}
void CvDLLWidgetData::doPediaUnitJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;

	argsList.add(widgetDataStruct.m_iData1);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToUnit", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaProfessionJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;

	argsList.add(widgetDataStruct.m_iData1);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToProfession", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaBuildingJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToBuilding", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaTerrainJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToTerrain", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaYieldJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToYields", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaFeatureJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToFeature", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaFatherJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToFather", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaTrainJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(GC.getCivilizationInfo(GC.getGameINLINE().getActiveCivilizationType()).getCivilizationUnits(widgetDataStruct.m_iData1));

	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToUnit", argsList.makeFunctionArgs());
}


void CvDLLWidgetData::doPediaConstructJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(GC.getCivilizationInfo(GC.getGameINLINE().getActiveCivilizationType()).getCivilizationBuildings(widgetDataStruct.m_iData1));

	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToBuilding", argsList.makeFunctionArgs());
}


void CvDLLWidgetData::doPediaBack()
{
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaBack");
}

void CvDLLWidgetData::doPediaForward()
{
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaForward");
}

void CvDLLWidgetData::doPediaBonusJump(CvWidgetDataStruct &widgetDataStruct, bool bData2)
{
	CyArgsList argsList;
	if (bData2)
	{
		argsList.add(widgetDataStruct.m_iData2);
	}
	else
	{
		argsList.add(widgetDataStruct.m_iData1);
	}
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToBonus", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaMain(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1 < 0 ? 0 : widgetDataStruct.m_iData1);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaMain", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaPromotionJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToPromotion", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaImprovementJump(CvWidgetDataStruct &widgetDataStruct, bool bData2)
{
	CyArgsList argsList;
	if (bData2)
	{
		argsList.add(widgetDataStruct.m_iData2);
	}
	else
	{
		argsList.add(widgetDataStruct.m_iData1);
	}
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToImprovement", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaCivicJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToCivic", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaCivilizationJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToCiv", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaLeaderJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToLeader", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaDescription(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	argsList.add(widgetDataStruct.m_iData2);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaShowHistorical", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaBuildJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;

	ImprovementTypes eImprovement = NO_IMPROVEMENT;
	BuildTypes eBuild = (BuildTypes)widgetDataStruct.m_iData2;
	if (NO_BUILD != eBuild)
	{
		eImprovement = (ImprovementTypes)GC.getBuildInfo(eBuild).getImprovement();
	}

	if (NO_IMPROVEMENT != eImprovement)
	{
		argsList.add(eImprovement);
		gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToImprovement", argsList.makeFunctionArgs());
	}
}

void CvDLLWidgetData::doGotoTurnEvent(CvWidgetDataStruct &widgetDataStruct)
{
	CvPlot* pPlot = GC.getMapINLINE().plotINLINE(widgetDataStruct.m_iData1, widgetDataStruct.m_iData2);

	if (NULL != pPlot && !gDLL->getEngineIFace()->isCameraLocked())
	{
		if (pPlot->isRevealed(GC.getGameINLINE().getActiveTeam(), false))
		{
			gDLL->getEngineIFace()->cameraLookAt(pPlot->getPoint());
		}
	}
}

void CvDLLWidgetData::doMenu( void )
{
	if (!gDLL->isGameInitializing())
	{
		CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_MAIN_MENU);
		gDLL->getInterfaceIFace()->addPopup(pInfo, NO_PLAYER, true);
	}
}

//
//	HELP PARSING FUNCTIONS
//

void CvDLLWidgetData::parsePlotListHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	PROFILE_FUNC();

	CvUnit* pUnit;

	int iUnitIndex = widgetDataStruct.m_iData1 + gDLL->getInterfaceIFace()->getPlotListColumn() - gDLL->getInterfaceIFace()->getPlotListOffset();

	CvPlot *selectionPlot = gDLL->getInterfaceIFace()->getSelectionPlot();
	pUnit = gDLL->getInterfaceIFace()->getInterfacePlotUnit(selectionPlot, iUnitIndex);

	if (pUnit != NULL)
	{
		GAMETEXT.setUnitHelp(szBuffer, pUnit);

		if (pUnit->plot()->plotCount(PUF_isUnitType, pUnit->getUnitType(), -1, pUnit->getOwnerINLINE()) > 1)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_CTRL_SELECT", GC.getUnitInfo(pUnit->getUnitType()).getTextKeyWide()));
		}

		if (pUnit->plot()->plotCount(NULL, -1, -1, pUnit->getOwnerINLINE()) > 1)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_ALT_SELECT"));
		}
	}
}


void CvDLLWidgetData::parseLiberateCityHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	if (pHeadSelectedCity != NULL)
	{
		PlayerTypes ePlayer = pHeadSelectedCity->getLiberationPlayer(false);
		if (NO_PLAYER != ePlayer)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_LIBERATE_CITY_HELP", pHeadSelectedCity->getNameKey(), GET_PLAYER(ePlayer).getNameKey()));
		}
	}
}

void CvDLLWidgetData::parseCityNameHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	if (pHeadSelectedCity != NULL)
	{
		szBuffer.append(pHeadSelectedCity->getName());

		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_CITY_POPULATION", pHeadSelectedCity->getRealPopulation()));

		CvWString szTempBuffer;
		GAMETEXT.setTimeStr(szTempBuffer, pHeadSelectedCity->getGameTurnFounded(), false);
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_CITY_FOUNDED", szTempBuffer.GetCString()));

		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_CHANGE_NAME"));
	}
}



void CvDLLWidgetData::parseTrainHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;
	UnitTypes eUnit;

	if (widgetDataStruct.m_iData2 != FFreeList::INVALID_INDEX)
	{
		pHeadSelectedCity = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCity(widgetDataStruct.m_iData2);
	}
	else
	{
		pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	}

	if (pHeadSelectedCity != NULL)
	{
		eUnit = (UnitTypes)GC.getCivilizationInfo(pHeadSelectedCity->getCivilizationType()).getCivilizationUnits(widgetDataStruct.m_iData1);
		GAMETEXT.setUnitHelp(szBuffer, eUnit, false, widgetDataStruct.m_bOption, pHeadSelectedCity);
	}
}


void CvDLLWidgetData::parseConstructHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;
	BuildingTypes eBuilding;

	if (widgetDataStruct.m_iData2 != FFreeList::INVALID_INDEX)
	{
		pHeadSelectedCity = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCity(widgetDataStruct.m_iData2);
	}
	else
	{
		pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	}

	if (pHeadSelectedCity != NULL)
	{
		eBuilding = (BuildingTypes)GC.getCivilizationInfo(pHeadSelectedCity->getCivilizationType()).getCivilizationBuildings(widgetDataStruct.m_iData1);
		GAMETEXT.setBuildingHelp(szBuffer, eBuilding, false, widgetDataStruct.m_bOption, pHeadSelectedCity);
	}
}

void CvDLLWidgetData::parseConvinceHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.setFatherPointHelp(szBuffer, (FatherPointTypes) widgetDataStruct.m_iData1);
}


void CvDLLWidgetData::parseHurryHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	HurryTypes eHurry = (HurryTypes) widgetDataStruct.m_iData1;
	CvCity* pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		if (!(pHeadSelectedCity->isProductionUnit()) && !(pHeadSelectedCity->isProductionBuilding()))
		{
			szBuffer.append(NEWLINE);
			szBuffer.assign(gDLL->getText("TXT_KEY_MISC_UNIT_BUILDING_HURRY"));
		}
		else
		{
			szBuffer.assign(gDLL->getText("TXT_KEY_MISC_HURRY_PROD", pHeadSelectedCity->getProductionNameKey()));

			int iHurryGold = pHeadSelectedCity->hurryGold(eHurry);
			if (iHurryGold > 0)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_HURRY_GOLD", iHurryGold));
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_FINANCIAL_ADVISOR_TREASURY", GET_PLAYER(pHeadSelectedCity->getOwnerINLINE()).getGold()));
			}

			int iHurryPopulation = pHeadSelectedCity->hurryPopulation(eHurry);
			if (iHurryPopulation > 0)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_HURRY_POP", iHurryPopulation));

				if (iHurryPopulation > pHeadSelectedCity->maxHurryPopulation())
				{
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_MAX_POP_HURRY", pHeadSelectedCity->maxHurryPopulation()));
				}
			}

			for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
			{	
				int iAmountNeeded = pHeadSelectedCity->hurryYield(eHurry, (YieldTypes) iYield);
				if (iAmountNeeded > 0)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HURRY_YIELD", iAmountNeeded, GC.getYieldInfo((YieldTypes) iYield).getChar()));
				}
			}

			bool bFirst = true;
			if (!(GET_PLAYER(pHeadSelectedCity->getOwnerINLINE()).canHurry((HurryTypes)(widgetDataStruct.m_iData1), -1)))
			{
				for (int iI = 0; iI < GC.getNumCivicInfos(); iI++)
				{
					if (GC.getCivicInfo((CivicTypes)iI).isHurry(widgetDataStruct.m_iData1))
					{
						CvWString szTempBuffer = NEWLINE + gDLL->getText("TXT_KEY_REQUIRES");
						setListHelp(szBuffer, szTempBuffer, GC.getCivicInfo((CivicTypes)iI).getDescription(), gDLL->getText("TXT_KEY_OR").c_str(), bFirst);
						bFirst = false;
					}
				}

				for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
				{
					YieldTypes eYield = (YieldTypes) iYield;
					if (GC.getYieldInfo(eYield).isCargo() && !GET_PLAYER(pHeadSelectedCity->getOwnerINLINE()).isYieldEuropeTradable(eYield))
					{
						int iAmountNeeded = pHeadSelectedCity->getProductionNeeded(eYield);
						if (iAmountNeeded > pHeadSelectedCity->getYieldStored(eYield) + pHeadSelectedCity->getYieldRushed(eYield))
						{
							szBuffer.append(NEWLINE + gDLL->getText("TXT_KEY_REQUIRES"));
							szBuffer.append(CvWString::format(L"%d%c", iAmountNeeded, GC.getYieldInfo(eYield).getChar()));
							bFirst = false;
						}
					}
				}
			}

			for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
			{
				YieldTypes eYield = (YieldTypes) iYield;
				if (GC.getYieldInfo(eYield).isCargo() && !GET_PLAYER(pHeadSelectedCity->getOwnerINLINE()).isYieldEuropeTradable(eYield))
				{
					int iAmountNeeded = pHeadSelectedCity->getProductionNeeded(eYield);
					if (iAmountNeeded > pHeadSelectedCity->getYieldStored(eYield) + pHeadSelectedCity->getYieldRushed(eYield))
					{
						szBuffer.append(NEWLINE + gDLL->getText("TXT_KEY_REQUIRES"));
						szBuffer.append(CvWString::format(L"%d%c", iAmountNeeded, GC.getYieldInfo(eYield).getChar()));
						bFirst = false;
					}
				}
			}

			if (!bFirst)
			{
				szBuffer.append(ENDCOLR);
			}
		}
	}
}

void CvDLLWidgetData::parsePlayerHurryHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvPlayer& kPlayer = GET_PLAYER(GC.getGameINLINE().getActivePlayer());
	HurryTypes eHurry = (HurryTypes) widgetDataStruct.m_iData1;
	szBuffer.assign(gDLL->getText("TXT_KEY_MISC_HURRY_PROD", kPlayer.getHurryItemTextKey(eHurry, widgetDataStruct.m_iData2)));

	int iHurryGold = kPlayer.getHurryGold(eHurry, widgetDataStruct.m_iData2);
	if (iHurryGold > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HURRY_GOLD", iHurryGold));
	}
}


void CvDLLWidgetData::parseActionHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CLLNode<IDInfo>* pSelectedUnitNode;
	CvCity* pMissionCity;
	CvUnit* pHeadSelectedUnit;
	CvUnit* pSelectedUnit;
	CvPlot* pMissionPlot;
	CvPlot* pLoopPlot;
	CvWString szTempBuffer;
	CvWString szFirstBuffer;
	ImprovementTypes eImprovement;
	ImprovementTypes eFinalImprovement;
	PlayerTypes eGiftPlayer;
	BuildTypes eBuild;
	RouteTypes eRoute;
	bool bAlt;
	bool bShift;
	bool bValid;
	int iYield;
	int iMovementCost;
	int iFlatMovementCost;
	int iMoves;
	int iFlatMoves;
	int iNowWorkRate;
	int iThenWorkRate;
	int iTurns;
	int iPrice;
	int iLast;
	int iRange;
	int iDX, iDY;
	int iI;

	bAlt = gDLL->altKey();
	bShift = gDLL->shiftKey();

	CvWString szTemp;
	szTemp.Format(SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getActionInfo(widgetDataStruct.m_iData1).getHotKeyDescription().c_str());
	szBuffer.assign(szTemp);

	pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();

	if (pHeadSelectedUnit != NULL)
	{
		if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() != NO_MISSION)
		{
			if (bShift && gDLL->getInterfaceIFace()->mirrorsSelectionGroup())
			{
				pMissionPlot = pHeadSelectedUnit->getGroup()->lastMissionPlot();
			}
			else
			{
				pMissionPlot = pHeadSelectedUnit->plot();
			}

			pMissionCity = pMissionPlot->getPlotCity();

			if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_HEAL)
			{
				iTurns = 0;

				pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

				while (pSelectedUnitNode != NULL)
				{
					pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);
					iTurns = std::max(iTurns, pSelectedUnit->healTurns(pMissionPlot));

					pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
				}

				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_TURN_OR_TURNS", iTurns));
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_PILLAGE)
			{
				if (pMissionPlot->getImprovementType() != NO_IMPROVEMENT)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_ACTION_DESTROY_IMP", GC.getImprovementInfo(pMissionPlot->getImprovementType()).getTextKeyWide()));
				}
				else if (pMissionPlot->getRouteType() != NO_ROUTE)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_ACTION_DESTROY_IMP", GC.getRouteInfo(pMissionPlot->getRouteType()).getTextKeyWide()));
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_FOUND)
			{
				if (!(GET_PLAYER(pHeadSelectedUnit->getOwnerINLINE()).canFound(pMissionPlot->getX_INLINE(), pMissionPlot->getY_INLINE())))
				{
					bValid = true;

					iRange = GC.getMIN_CITY_RANGE();

					for (iDX = -(iRange); iDX <= iRange; iDX++)
					{
						for (iDY = -(iRange); iDY <= iRange; iDY++)
						{
							pLoopPlot	= plotXY(pMissionPlot->getX_INLINE(), pMissionPlot->getY_INLINE(), iDX, iDY);

							if (pLoopPlot != NULL)
							{
								if (pLoopPlot->isCity())
								{
									bValid = false;
								}
							}
						}
					}

					if (!bValid)
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_ACTION_CANNOT_FOUND", GC.getMIN_CITY_RANGE()));
					}
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_JOIN_CITY)
			{
				if (!pHeadSelectedUnit->canJoinCity(pMissionPlot))
				{
					if (pHeadSelectedUnit->hasMoved())
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_ACTION_CANNOT_JOIN_MOVE"));
					}

					CvCity* pCity = pMissionPlot->getPlotCity();
					if (pCity != NULL)
					{
						if (pCity->foodDifference() < 0 && !pHeadSelectedUnit->canJoinStarvingCity(*pCity))
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_ACTION_CANNOT_JOIN_STARVING"));
						}
					}
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_LEAD)
			{
				if (pHeadSelectedUnit->getUnitInfo().getLeaderExperience() > 0)
				{
					int iNumUnits = pHeadSelectedUnit->canGiveExperience(pHeadSelectedUnit->plot());
					if (iNumUnits > 0)
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_ACTION_LEAD_TROOPS", pHeadSelectedUnit->getStackExperienceToGive(iNumUnits)));
					}
				}
				if (pHeadSelectedUnit->getUnitInfo().getLeaderPromotion() != NO_PROMOTION)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_WHEN_LEADING"));
					GAMETEXT.parsePromotionHelp(szBuffer, (PromotionTypes)pHeadSelectedUnit->getUnitInfo().getLeaderPromotion(), L"\n   ");
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_BUILD)
			{
				eBuild = ((BuildTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getMissionData()));
				FAssert(eBuild != NO_BUILD);
				eImprovement = pMissionPlot->getImprovementType();
				ImprovementTypes eBuildImprovement = (ImprovementTypes) GC.getBuildInfo(eBuild).getImprovement();
				if (eBuildImprovement != NO_IMPROVEMENT)
				{
					eImprovement = eBuildImprovement;
				}
				eRoute = pMissionPlot->getRouteType();
				RouteTypes eBuildRoute = (RouteTypes) GC.getBuildInfo(eBuild).getRoute();
				if (eBuildRoute != NO_ROUTE)
				{
					eRoute = eBuildRoute;
				}
				FeatureTypes eFeature = pMissionPlot->getFeatureType();
				bool bIgnoreFeature = (eFeature != NO_FEATURE && GC.getBuildInfo(eBuild).isFeatureRemove(eFeature));
				for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
				{
					iYield = pMissionPlot->calculatePotentialYield((YieldTypes) iI, GC.getGameINLINE().getActivePlayer(), eImprovement, bIgnoreFeature, eRoute, NO_UNIT, false);
					iYield -= pMissionPlot->calculatePotentialYield((YieldTypes) iI, GC.getGameINLINE().getActivePlayer(), pMissionPlot->getImprovementType(), false, pMissionPlot->getRouteType(), NO_UNIT, false);

					if (iYield != 0)
					{
						szTempBuffer.Format(L", %s%d%c", ((iYield > 0) ? L"+" : L""), iYield, GC.getYieldInfo((YieldTypes) iI).getChar());
						szBuffer.append(szTempBuffer);
					}
				}

				bValid = false;

				pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

				while (pSelectedUnitNode != NULL)
				{
					pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

					if (pSelectedUnit->canBuild(pMissionPlot, eBuild))
					{
						bValid = true;
						break;
					}

					pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
				}

				if (!bValid)
				{
					if (eBuildImprovement != NO_IMPROVEMENT)
					{
						if (pMissionPlot->getTeam() != pHeadSelectedUnit->getTeam())
						{
							if (GC.getImprovementInfo(eBuildImprovement).isOutsideBorders())
							{
								if (pMissionPlot->getTeam() != NO_TEAM)
								{
									szBuffer.append(NEWLINE);
									szBuffer.append(gDLL->getText("TXT_KEY_ACTION_NEEDS_OUT_RIVAL_CULTURE_BORDER"));
								}
							}
							else
							{
								szBuffer.append(NEWLINE);
								szBuffer.append(gDLL->getText("TXT_KEY_ACTION_NEEDS_CULTURE_BORDER"));
							}
						}
					}
				}

				if (eBuildImprovement != NO_IMPROVEMENT)
				{
					if (pMissionPlot->getImprovementType() != NO_IMPROVEMENT)
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_ACTION_WILL_DESTROY_IMP", GC.getImprovementInfo(pMissionPlot->getImprovementType()).getTextKeyWide()));
					}
				}

				if (GC.getBuildInfo(eBuild).isKill())
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_ACTION_CONSUME_UNIT"));
				}

				if (pMissionPlot->getFeatureType() != NO_FEATURE)
				{
					if (GC.getBuildInfo(eBuild).isFeatureRemove(pMissionPlot->getFeatureType()))
					{
						CvCity* pCity = NULL;
						for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
						{
							YieldTypes eYield = (YieldTypes) iYield;
							if (GC.getYieldInfo(eYield).isCargo())
							{
								int iYieldProduction = pMissionPlot->getFeatureYield(eBuild, eYield, pHeadSelectedUnit->getTeam(), &pCity);
								if (iYieldProduction > 0)
								{
									szBuffer.append(NEWLINE);
									szBuffer.append(gDLL->getText("TXT_KEY_ACTION_CHANGE_PRODUCTION", iYieldProduction, pCity->getNameKey(), GC.getYieldInfo(eYield).getChar()));
								}
							}
						}

						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_ACTION_REMOVE_FEATURE", GC.getFeatureInfo(pMissionPlot->getFeatureType()).getTextKeyWide()));
					}

				}

				if (eBuildImprovement != NO_IMPROVEMENT)
				{
					if (pMissionPlot->getBonusType() == NO_BONUS)
					{
						iLast = 0;

						FAssert((0 < GC.getNumBonusInfos()) && "GC.getNumBonusInfos() is not greater than zero but an array is being allocated in CvDLLWidgetData::parseActionHelp");
						for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
						{
							if (GC.getImprovementInfo(eBuildImprovement).getImprovementBonusDiscoverRand(iI) > 0)
							{
								szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_ACTION_CHANCE_DISCOVER").c_str());
								szTempBuffer.Format(L"%c", GC.getBonusInfo((BonusTypes) iI).getChar());
								setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", (GC.getImprovementInfo(eBuildImprovement).getImprovementBonusDiscoverRand(iI) != iLast));
								iLast = GC.getImprovementInfo(eBuildImprovement).getImprovementBonusDiscoverRand(iI);
							}
						}
					}

					if (eBuildRoute == NO_ROUTE)
					{
						for (iI = 0; iI < GC.getNumRouteInfos(); iI++)
						{
							if (pMissionPlot->getRouteType() != ((RouteTypes)iI))
							{
								GAMETEXT.setYieldChangeHelp(szBuffer, GC.getRouteInfo((RouteTypes)iI).getDescription(), L": ", L"", GC.getImprovementInfo(eBuildImprovement).getRouteYieldChangesArray((RouteTypes)iI));
							}
						}
					}

					if (GC.getImprovementInfo(eBuildImprovement).getDefenseModifier() != 0)
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_ACTION_DEFENSE_MODIFIER", GC.getImprovementInfo(eBuildImprovement).getDefenseModifier()));
					}

					if (GC.getImprovementInfo(eBuildImprovement).getImprovementUpgrade() != NO_IMPROVEMENT)
					{
						iTurns = pMissionPlot->getUpgradeTimeLeft(eBuildImprovement, pHeadSelectedUnit->getOwnerINLINE());

						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_ACTION_BECOMES_IMP", GC.getImprovementInfo((ImprovementTypes) GC.getImprovementInfo(eBuildImprovement).getImprovementUpgrade()).getTextKeyWide(), iTurns));
					}
				}

				if (eBuildRoute != NO_ROUTE)
				{
					eFinalImprovement = eBuildImprovement;

					if (eFinalImprovement == NO_IMPROVEMENT)
					{
						eFinalImprovement = pMissionPlot->getImprovementType();
					}

					if (eFinalImprovement != NO_IMPROVEMENT)
					{
						GAMETEXT.setYieldChangeHelp(szBuffer, GC.getImprovementInfo(eFinalImprovement).getDescription(), L": ", L"", GC.getImprovementInfo(eFinalImprovement).getRouteYieldChangesArray(eBuildRoute));
					}

					iMovementCost = GC.getRouteInfo(eBuildRoute).getMovementCost();
					iFlatMovementCost = GC.getRouteInfo(eBuildRoute).getFlatMovementCost();

					if (iMovementCost > 0)
					{
						iMoves = (GC.getMOVE_DENOMINATOR() / iMovementCost);

						if ((iMoves * iMovementCost) < GC.getMOVE_DENOMINATOR())
						{
							iMoves++;
						}
					}
					else
					{
						iMoves = GC.getMOVE_DENOMINATOR();
					}

					if (iFlatMovementCost > 0)
					{
						iFlatMoves = (GC.getMOVE_DENOMINATOR() / iFlatMovementCost);

						if ((iFlatMoves * iFlatMovementCost) < GC.getMOVE_DENOMINATOR())
						{
							iFlatMoves++;
						}
					}
					else
					{
						iFlatMoves = GC.getMOVE_DENOMINATOR();
					}

					if ((iMoves > 1) || (iFlatMoves > 1))
					{
						if (iMoves >= iFlatMoves)
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_ACTION_MOVEMENT_COST", iMoves));
						}
						else
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_ACTION_FLAT_MOVEMENT_COST", iFlatMoves));
						}
					}
				}

				iNowWorkRate = 0;
				iThenWorkRate = 0;

				pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

				if (NULL != pHeadSelectedUnit)
				{
					int iCost = GET_PLAYER(pHeadSelectedUnit->getOwnerINLINE()).getBuildCost(pMissionPlot, eBuild);
					if (iCost > 0)
					{
						szBuffer.append(NEWLINE);
						if (GET_PLAYER(pHeadSelectedUnit->getOwnerINLINE()).getGold() < iCost)
						{
							szBuffer.append(gDLL->getText("TXT_KEY_BUILD_CANNOT_AFFORD", iCost, gDLL->getSymbolID(GOLD_CHAR)));
						}
						else
						{
							szBuffer.append(gDLL->getText("TXT_KEY_BUILD_COST", iCost));
						}
					}
				}

				while (pSelectedUnitNode != NULL)
				{
					pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

					iNowWorkRate += pSelectedUnit->workRate(false);
					iThenWorkRate += pSelectedUnit->workRate(true);

					pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
				}

				iTurns = pMissionPlot->getBuildTurnsLeft(eBuild, iNowWorkRate, iThenWorkRate);


				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_ACTION_NUM_TURNS", iTurns));

				if (!isEmpty(GC.getBuildInfo(eBuild).getHelp()))
				{
					szBuffer.append(CvWString::format(L"%s%s", NEWLINE, GC.getBuildInfo(eBuild).getHelp()).c_str());
				}
			}

			if (!isEmpty(GC.getMissionInfo((MissionTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType())).getHelp()))
			{
				szBuffer.append(CvWString::format(L"%s%s", NEWLINE, GC.getMissionInfo((MissionTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType())).getHelp()).c_str());
			}
		}

		if (GC.getActionInfo(widgetDataStruct.m_iData1).getCommandType() != NO_COMMAND)
		{
			if (GC.getActionInfo(widgetDataStruct.m_iData1).getCommandType() == COMMAND_PROMOTION)
			{
				GAMETEXT.parsePromotionHelp(szBuffer, ((PromotionTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getCommandData())));
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getCommandType() == COMMAND_UPGRADE)
			{
				GAMETEXT.setBasicUnitHelp(szBuffer, ((UnitTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getCommandData())));

				if (bAlt && GC.getCommandInfo((CommandTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getCommandType())).getAll())
				{
					iPrice = GET_PLAYER(pHeadSelectedUnit->getOwnerINLINE()).upgradeAllPrice(((UnitTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getCommandData())), pHeadSelectedUnit->getUnitType());
				}
				else
				{
					iPrice = 0;

					pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

					while (pSelectedUnitNode != NULL)
					{
						pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

						if (pSelectedUnit->canUpgrade(((UnitTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getCommandData())), true))
						{
							iPrice += pSelectedUnit->upgradePrice((UnitTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getCommandData()));
						}

						pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
					}
				}

				szTempBuffer.Format(L"%s%d %c", NEWLINE, iPrice, gDLL->getSymbolID(GOLD_CHAR));
				szBuffer.append(szTempBuffer);
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getCommandType() == COMMAND_GIFT)
			{
				eGiftPlayer = pHeadSelectedUnit->plot()->getOwnerINLINE();

				if (eGiftPlayer != NO_PLAYER)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_ACTION_GOES_TO_CIV"));

					szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, GET_PLAYER(eGiftPlayer).getPlayerTextColorR(), GET_PLAYER(eGiftPlayer).getPlayerTextColorG(), GET_PLAYER(eGiftPlayer).getPlayerTextColorB(), GET_PLAYER(eGiftPlayer).getPlayerTextColorA(), GET_PLAYER(eGiftPlayer).getCivilizationShortDescription());
					szBuffer.append(szTempBuffer);

					pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

					while (pSelectedUnitNode != NULL)
					{
						pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

						if (!(GET_PLAYER(eGiftPlayer).AI_acceptUnit(pSelectedUnit)))
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_REFUSE_GIFT", GET_PLAYER(eGiftPlayer).getNameKey()));
							break;
						}

						pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
					}
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getCommandType() == COMMAND_LEARN)
			{
				CvPlot* pPlot = pHeadSelectedUnit->plot();
				if (pPlot != NULL)
				{
					UnitTypes eLearnUnit = pHeadSelectedUnit->getLearnUnitType(pPlot);
					if(eLearnUnit != NO_UNIT)
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_COMMAND_LEARN_HELP_1", GC.getUnitInfo(eLearnUnit).getTextKeyWide()));
					}
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getCommandType() == COMMAND_ESTABLISH_MISSION)
			{
				CvUnit* pMissionary = NULL;
				pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();
				while (pSelectedUnitNode != NULL)
				{
					pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);
					if (pSelectedUnit->canEstablishMission())
					{
						pMissionary = pSelectedUnit;
						break;
					}

					pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
				}


				if (pMissionary != NULL)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_TALK_NATIVES_POPUP_MISSION2", std::min(100, pMissionary->getMissionarySuccessPercent())));
				}
			}

			if (GC.getCommandInfo((CommandTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getCommandType())).getAll())
			{
				szBuffer.append(gDLL->getText("TXT_KEY_ACTION_ALL_UNITS"));
			}

			if (!isEmpty(GC.getCommandInfo((CommandTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getCommandType())).getHelp()))
			{
				szBuffer.append(CvWString::format(L"%s%s", NEWLINE, GC.getCommandInfo((CommandTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getCommandType())).getHelp()).c_str());
			}
		}

		if (GC.getActionInfo(widgetDataStruct.m_iData1).getAutomateType() != NO_AUTOMATE)
		{
			if (!isEmpty(GC.getAutomateInfo((ControlTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getAutomateType())).getHelp()))
			{
				szBuffer.append(CvWString::format(L"%s%s", NEWLINE, GC.getAutomateInfo((ControlTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getAutomateType())).getHelp()).c_str());
			}
		}
	}

	if (GC.getActionInfo(widgetDataStruct.m_iData1).getControlType() != NO_CONTROL)
	{
		if (!isEmpty(GC.getControlInfo((ControlTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getControlType())).getHelp()))
		{
			szBuffer.append(CvWString::format(L"%s%s", NEWLINE, GC.getControlInfo((ControlTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getControlType())).getHelp()).c_str());
		}
	}

	if (GC.getActionInfo(widgetDataStruct.m_iData1).getInterfaceModeType() != NO_INTERFACEMODE)
	{
		if (!isEmpty(GC.getInterfaceModeInfo((InterfaceModeTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getInterfaceModeType())).getHelp()))
		{
			szBuffer.append(CvWString::format(L"%s%s", NEWLINE, GC.getInterfaceModeInfo((InterfaceModeTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getInterfaceModeType())).getHelp()).c_str());
		}
	}
}


void CvDLLWidgetData::parseCitizenHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pCity = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCity(widgetDataStruct.m_iData2);
	if (pCity == NULL)
	{
		pCity =	gDLL->getInterfaceIFace()->getHeadSelectedCity();
	}
	if (pCity != NULL)
	{
		CvUnit* pUnit = pCity->getPopulationUnitById(widgetDataStruct.m_iData1);
		if (NULL == pUnit)
		{
			return;
		}

		GAMETEXT.setCitizenHelp(szBuffer, *pCity, *pUnit);
	}
}

void CvDLLWidgetData::parseContactCivHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	//	Do not execute this if we are trying to contact ourselves...
	if (widgetDataStruct.m_iData1 >= MAX_PLAYERS)
	{
		return;
	}

	CvPlayer& otherPlayer = GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1);

	if (otherPlayer.getCivilizationType() == NO_CIVILIZATION)
	{
		return;
	}

	if (GC.getGameINLINE().getActivePlayer() == otherPlayer.getID())
	{
		parseFlagHelp(widgetDataStruct, szBuffer);
		parseScoreHelp(widgetDataStruct, szBuffer);
		return;
	}

	CvWString szNameKey;
	if (otherPlayer.isEurope())
	{
		szNameKey = "TXT_KEY_GENERIC_KING";
	}
	else
	{
		szNameKey = otherPlayer.getNameKey();
	}
	szBuffer.assign(gDLL->getText("TXT_KEY_MISC_CONTACT_LEADER", szNameKey.GetCString(), otherPlayer.getCivilizationShortDescription()));

	szBuffer.append(NEWLINE);
	GAMETEXT.parsePlayerTraits(szBuffer, otherPlayer.getID());

	if (!(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isHasMet(otherPlayer.getTeam())))
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HAVENT_MET_CIV"));
	}
	else
	{
		if (!otherPlayer.isHuman())
		{
			if (!otherPlayer.AI_isWillingToTalk(GC.getGameINLINE().getActivePlayer()))
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_REFUSES_TO_TALK"));
			}

			szBuffer.append(NEWLINE);
			GAMETEXT.getAttitudeString(szBuffer, otherPlayer.getID(), GC.getGameINLINE().getActivePlayer());

			if (!GC.getGameINLINE().isOption(GAMEOPTION_ALWAYS_WAR) && !otherPlayer.isEurope())
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_CTRL_TRADE"));
			}
		}

		if (otherPlayer.getTeam() != GC.getGameINLINE().getActiveTeam() && !::atWar(GC.getGameINLINE().getActiveTeam(), otherPlayer.getTeam()))
		{
			if (GET_TEAM(GC.getGameINLINE().getActiveTeam()).canDeclareWar(otherPlayer.getTeam()))
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_ALT_DECLARE_WAR"));
			}
			else if (!GET_TEAM(otherPlayer.getTeam()).isParentOf(GC.getGameINLINE().getActiveTeam()))
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_CANNOT_DECLARE_WAR"));
			}
		}
	}

	if (otherPlayer.isHuman())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_SHIFT_SEND_CHAT"));
	}

	if ((gDLL->getChtLvl() > 0) && gDLL->shiftKey())
	{
		CvPlayerAI& player = GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1);
		
		szBuffer.append(CvWString::format(L"\nPlayer %d, Team %d", player.getID(), player.getTeam()));
		szBuffer.append(CvWString::format(L"\n%d%c %d%c", player.getGold(), gDLL->getSymbolID(GOLD_CHAR), GET_TEAM(player.getTeam()).getRebelPercent(), gDLL->getSymbolID(POWER_CHAR)));
		szBuffer.append(CvWString::format(L"\nCities = %d, Units = %d, Pop = %d, AIPop = %d", player.getNumCities(), player.getNumUnits(), player.getTotalPopulation(), player.AI_getPopulation()));
		szBuffer.append(CvWString::format(L"\nIncome = %d, Hurry Spending = %d", player.AI_getTotalIncome(), player.AI_getHurrySpending()));
		for (int i = 0; i < NUM_STRATEGY_TYPES; ++i)
		{
			if (player.AI_isStrategy((StrategyTypes) i))
			{
				szBuffer.append(CvWString::format(L"\nStrategy %d, Duration %d", i, player.AI_getStrategyDuration((StrategyTypes) i)));
			}
		}
		
		//Extra Info
		ProfessionTypes eBuyProfession;
		UnitTypes eBuyProfessionUnit;
		UnitAITypes eBuyProfessionAI;
		int iBuyProfessionValue;
		eBuyProfessionUnit = player.AI_nextBuyProfessionUnit(&eBuyProfession, &eBuyProfessionAI, &iBuyProfessionValue);

		szBuffer.append(CvWString::format(SETCOLR L"\n"));

		UnitAITypes eBuyUnitAI;
		int iBuyUnitValue;
		UnitTypes eBuyUnit = player.AI_nextBuyUnit(&eBuyUnitAI, &iBuyUnitValue);

		if (eBuyProfession != NO_PROFESSION)
		{
			CvWString szTempString;
			getUnitAIString(szTempString, eBuyProfessionAI);
			szBuffer.append(CvWString::format(SETCOLR L"\n%s/ %s/ %s/ %d" ENDCOLR , TEXT_COLOR("COLOR_UNIT_TEXT"), (eBuyProfessionUnit == NO_UNIT) ? L"NONE" : GC.getUnitInfo(eBuyProfessionUnit).getDescription(), GC.getProfessionInfo(eBuyProfession).getDescription(), szTempString.GetCString(), iBuyProfessionValue ));
		}
		if (eBuyUnit != NO_UNIT)
		{
			CvWString szTempString;
			getUnitAIString(szTempString, eBuyUnitAI);
			szBuffer.append(CvWString::format(SETCOLR L"\n%s/ %s/ %d" ENDCOLR , TEXT_COLOR("COLOR_UNIT_TEXT"), GC.getUnitInfo(eBuyUnit).getDescription(), szTempString.GetCString(), iBuyUnitValue ));
		}
		
		if (player.isNative())
		{
			szBuffer.append(CvWString::format(L"\nOverpopulation : %d", player.AI_getOverpopulationPercent()));
		}
		
		CvTeamAI& kTeam = GET_TEAM(player.getTeam());
		for (int i = 0; i < MAX_PLAYERS; ++i)
		{
			CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)i);
						
			if (kLoopPlayer.isAlive())
			{
				TeamTypes eLoopTeam = kLoopPlayer.getTeam();
				if (eLoopTeam != player.getTeam() && GET_TEAM(eLoopTeam).isAlive())
				{
					WarPlanTypes eWarplan = kTeam.AI_getWarPlan(eLoopTeam);
					
					if (eWarplan != NO_WARPLAN)
					{
						CvWStringBuffer szTempString;
						GAMETEXT.getWarplanString(szTempString, eWarplan);
						szBuffer.append(CvWString::format(L"\n%s : %s", kLoopPlayer.getName(), szTempString));
	
					}
				}
			}
		}
	}
}


void CvDLLWidgetData::parseAutomateCitizensHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData1 == 0)
	{
		szBuffer.assign(gDLL->getText("TXT_KEY_MISC_OFF_CITIZEN_AUTO"));
	}
	else
	{
		szBuffer.assign(gDLL->getText("TXT_KEY_MISC_ON_CITIZEN_AUTO"));
	}
}

void CvDLLWidgetData::parseAutomateProductionHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		if (pHeadSelectedCity->isProductionAutomated())
		{
			szBuffer.assign(gDLL->getText("TXT_KEY_MISC_OFF_PROD_AUTO"));
		}
		else
		{
			szBuffer.assign(gDLL->getText("TXT_KEY_MISC_ON_PROD_AUTO"));
		}
	}
}

void CvDLLWidgetData::parseEmphasizeHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	szBuffer.clear();

	CvCity* pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		YieldTypes eYield = (YieldTypes) widgetDataStruct.m_iData1;
		if (eYield != NO_YIELD)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_CLICK_TO"));
			EmphasizeTypes eNextEmphasis = getNextEmphasis(pHeadSelectedCity, eYield);
			if (eNextEmphasis == NO_EMPHASIZE)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_EMPHASIZE_OFF"));
			}
			else
			{
				szBuffer.append(GC.getEmphasizeInfo(eNextEmphasis).getDescription());
			}
		}
		else
		{
			EmphasizeTypes eEmphasize = (EmphasizeTypes) widgetDataStruct.m_iData2;
			if (eEmphasize != NO_EMPHASIZE)
			{
				szBuffer.append(GC.getEmphasizeInfo(eEmphasize).getDescription());
			}
			else
			{
				szBuffer.append(gDLL->getText("TXT_KEY_CLICK_TO"));
				szBuffer.append(gDLL->getText("TXT_KEY_EMPHASIZE_OFF"));
			}
		}
	}
}

void CvDLLWidgetData::parseTradeItem(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvWString szTempBuffer;
	TradeData item;
	PlayerTypes eWhoFrom = NO_PLAYER;
	PlayerTypes eWhoTo = NO_PLAYER;
	PlayerTypes eWhoDenies;

	szBuffer.clear();

	if (widgetDataStruct.m_bOption)
	{
		if ( gDLL->isDiplomacy())
		{
			eWhoFrom = (PlayerTypes) gDLL->getDiplomacyPlayer();
		}
		else if (gDLL->isMPDiplomacyScreenUp())
		{
			eWhoFrom = (PlayerTypes) gDLL->getMPDiplomacyPlayer();
		}
		eWhoTo = GC.getGameINLINE().getActivePlayer();
	}
	else
	{
		eWhoFrom = GC.getGameINLINE().getActivePlayer();
		if ( gDLL->isDiplomacy() )
		{
			eWhoTo = (PlayerTypes) gDLL->getDiplomacyPlayer();
		}
		else if (gDLL->isMPDiplomacyScreenUp())
		{
			eWhoTo = (PlayerTypes) gDLL->getMPDiplomacyPlayer();
		}
	}

	eWhoDenies = eWhoFrom;

	IDInfo kTransport;
	if ( gDLL->isDiplomacy())
	{
		kTransport = gDLL->getDiplomacyTransport();
	}
	else if (gDLL->isMPDiplomacyScreenUp())
	{
		kTransport = gDLL->getMPDiplomacyTransport();
	}

	if ((eWhoFrom != NO_PLAYER) && (eWhoTo != NO_PLAYER))
	{
		//	Data1 is the heading
		switch (widgetDataStruct.m_iData1)
		{
		case TRADE_CITIES:
			szBuffer.assign(gDLL->getText("TXT_KEY_TRADE_CITIES"));
			eWhoDenies = (widgetDataStruct.m_bOption ? eWhoFrom : eWhoTo);
			break;
		case TRADE_PEACE:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_MAKE_PEACE", GET_TEAM(GET_PLAYER(eWhoFrom).getTeam()).getName().GetCString(), GET_TEAM((TeamTypes)widgetDataStruct.m_iData2).getName().GetCString()));
			break;
		case TRADE_WAR:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_MAKE_WAR", GET_TEAM(GET_PLAYER(eWhoFrom).getTeam()).getName().GetCString(), GET_TEAM((TeamTypes)widgetDataStruct.m_iData2).getName().GetCString()));
			break;
		case TRADE_EMBARGO:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_STOP_TRADING", GET_TEAM(GET_PLAYER(eWhoFrom).getTeam()).getName().GetCString(), GET_TEAM((TeamTypes)widgetDataStruct.m_iData2).getName().GetCString()));
			break;
		case TRADE_GOLD:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_GOLD"));
			break;
		case TRADE_YIELD:
			GAMETEXT.setYieldPriceHelp(szBuffer, GC.getGameINLINE().getActivePlayer(), (YieldTypes) widgetDataStruct.m_iData2);
			break;
		case TRADE_MAPS:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_MAPS"));
			break;
		case TRADE_OPEN_BORDERS:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_OPEN_BORDERS"));
			break;
		case TRADE_DEFENSIVE_PACT:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_DEFENSIVE_PACT"));
			break;
		case TRADE_PERMANENT_ALLIANCE:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_PERMANENT_ALLIANCE"));
			break;
		case TRADE_PEACE_TREATY:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_PEACE_TREATY", GC.getDefineINT("PEACE_TREATY_LENGTH")));
			break;
		}

		setTradeItem(&item, ((TradeableItems)(widgetDataStruct.m_iData1)), widgetDataStruct.m_iData2, &kTransport);

		DenialTypes eDenial = GET_PLAYER(eWhoFrom).getTradeDenial(eWhoTo, item);
		if (eDenial != NO_DENIAL)
		{
			szTempBuffer.Format(L"%s: " SETCOLR L"%s" ENDCOLR, GET_PLAYER(eWhoDenies).getName(), TEXT_COLOR("COLOR_WARNING_TEXT"), GC.getDenialInfo(eDenial).getDescription());
			szBuffer.append(NEWLINE);
			szBuffer.append(szTempBuffer);
		}
	}
}


void CvDLLWidgetData::parseUnitModelHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvUnit* pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();
	if (pHeadSelectedUnit != NULL)
	{
		GAMETEXT.setUnitHelp(szBuffer, pHeadSelectedUnit);
	}
}

void CvDLLWidgetData::parseCityDefenseHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	if (pHeadSelectedCity != NULL)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_PLOT_BONUS", pHeadSelectedCity->getTotalDefense()));
	}
}


void CvDLLWidgetData::parseFlagHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvWString szTempBuffer;

	szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getCivilizationInfo(GC.getGameINLINE().getActiveCivilizationType()).getDescription());
	szBuffer.append(szTempBuffer);
	szBuffer.append(NEWLINE);

	GAMETEXT.parseLeaderTraits(szBuffer, GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getLeaderType(), GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCivilizationType());
}

void CvDLLWidgetData::parsePopulationHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		szBuffer.assign(gDLL->getText("TXT_KEY_MISC_FOOD_THRESHOLD", pHeadSelectedCity->getFood(), pHeadSelectedCity->growthThreshold()));
	}
}

void CvDLLWidgetData::parseRebelHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData1 == 1)
	{
		CvCity* pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
		if (pHeadSelectedCity != NULL)
		{
			szBuffer.assign(gDLL->getText("TXT_KEY_MISC_REBEL_HELP", pHeadSelectedCity->getRebelPercent()));
		}
	}
	else
	{
		szBuffer.assign(gDLL->getText("TXT_KEY_MISC_REBEL_HELP", GET_TEAM(GC.getGameINLINE().getActiveTeam()).getRebelPercent()));
	}
}


void CvDLLWidgetData::parseGreatGeneralHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (NO_PLAYER != GC.getGame().getActivePlayer())
	{
		GAMETEXT.parseGreatGeneralHelp(szBuffer, GET_PLAYER(GC.getGame().getActivePlayer()));
	}
}


void CvDLLWidgetData::parseSelectedHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;
	CvUnit* pHeadSelectedUnit;
	OrderData* pOrder;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();

	if (pHeadSelectedCity != NULL)
	{
		pOrder = pHeadSelectedCity->getOrderFromQueue(widgetDataStruct.m_iData1);

		if (pOrder != NULL)
		{
			switch (pOrder->eOrderType)
			{
			case ORDER_TRAIN:
				GAMETEXT.setUnitHelp(szBuffer, ((UnitTypes)(pOrder->iData1)), false, false, pHeadSelectedCity);
				break;
			case ORDER_CONSTRUCT:
				GAMETEXT.setBuildingHelp(szBuffer, ((BuildingTypes)(pOrder->iData1)), false, false, pHeadSelectedCity);
				break;
			case ORDER_CONVINCE:
				GAMETEXT.setFatherPointHelp(szBuffer, (FatherPointTypes) pOrder->iData1);
				break;

			default:
				FAssertMsg(false, "eOrderType did not match valid options");
				break;
			}
		}
	}
}


void CvDLLWidgetData::parseBuildingHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setBuildingHelp(szBuffer, ((BuildingTypes)(widgetDataStruct.m_iData1)), false, false, gDLL->getInterfaceIFace()->getHeadSelectedCity());
	}
}

void CvDLLWidgetData::parseSpecialBuildingHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	if (pCity != NULL)
	{
		for (int iBuildingClass = 0; iBuildingClass < GC.getNumBuildingClassInfos(); ++iBuildingClass)
		{
			BuildingTypes eBuilding = (BuildingTypes) GC.getCivilizationInfo(pCity->getCivilizationType()).getCivilizationBuildings(iBuildingClass);
			if (eBuilding != NO_BUILDING)
			{
				if (GC.getBuildingInfo(eBuilding).getSpecialBuildingType() == widgetDataStruct.m_iData1)
				{
					if (pCity->isHasBuilding(eBuilding))
					{
						GAMETEXT.setBuildingHelp(szBuffer, eBuilding, false, false, pCity);
					}
				}
			}
		}
	}
}

void CvDLLWidgetData::parseTerrainHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setTerrainHelp(szBuffer, (TerrainTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseYieldHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setYieldsHelp(szBuffer, (YieldTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseFeatureHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setFeatureHelp(szBuffer, (FeatureTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseFatherHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setFatherHelp(szBuffer, (FatherTypes)widgetDataStruct.m_iData1, false);
	}
}

void CvDLLWidgetData::parseUnitHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setUnitHelp(szBuffer, (UnitTypes)widgetDataStruct.m_iData1, false, false, gDLL->getInterfaceIFace()->getHeadSelectedCity());
	}
}

void CvDLLWidgetData::parseShipCargoUnitHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvPlayer& pPlayer = GET_PLAYER(GC.getGameINLINE().getActivePlayer());

	CvUnit* pUnit = pPlayer.getUnit(widgetDataStruct.m_iData1);
	if (pUnit != NULL)
	{
		GAMETEXT.setUnitHelp(szBuffer, pUnit, false, true);
	}
}

void CvDLLWidgetData::parseEuropeUnitHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvPlayer& kPlayer = GET_PLAYER(GC.getGameINLINE().getActivePlayer());
	CvUnit* pUnit = kPlayer.getEuropeUnitById(widgetDataStruct.m_iData1);
	if (pUnit != NULL)
{
		GAMETEXT.setUnitHelp(szBuffer, pUnit, false, true);
	}
}

void CvDLLWidgetData::parseProfessionHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setProfessionHelp(szBuffer, (ProfessionTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parsePediaBack(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
//	szBuffer = "Back";
	szBuffer.assign(gDLL->getText("TXT_KEY_MISC_PEDIA_BACK"));
}

void CvDLLWidgetData::parsePediaForward(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
//	szBuffer = "Forward";
	szBuffer.assign(gDLL->getText("TXT_KEY_MISC_PEDIA_FORWARD"));
}

void CvDLLWidgetData::parseBonusHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setBonusHelp(szBuffer, (BonusTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parsePromotionHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setPromotionHelp(szBuffer, (PromotionTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseUnitPromotionHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvUnit* pUnit = ::getUnit(IDInfo((PlayerTypes) widgetDataStruct.m_iData1, widgetDataStruct.m_iData2));
	if (pUnit != NULL)
	{
		GAMETEXT.setUnitPromotionHelp(szBuffer, pUnit);
	}
}

void CvDLLWidgetData::parseCityYieldHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	YieldTypes eYield = (YieldTypes) widgetDataStruct.m_iData1;
	CvPlayer& kPlayer = GET_PLAYER(GC.getGameINLINE().getActivePlayer());
	CvCity* pCity = kPlayer.getCity(widgetDataStruct.m_iData2);
	if (NULL == pCity)
	{
		pCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	}

	if (NULL != pCity)
	{
		GAMETEXT.setYieldHelp(szBuffer, *pCity, eYield);
	}
	else
	{
		GAMETEXT.setYieldPriceHelp(szBuffer, GC.getGameINLINE().getActivePlayer(), eYield);
	}

	PlayerTypes eActivePlayer = GC.getGameINLINE().getActivePlayer();
	for (int i = 0; i < GC.getNumProfessionInfos(); ++i)
	{
		ProfessionTypes eProfession = (ProfessionTypes) i;
		if (GET_PLAYER(eActivePlayer).isProfessionValid(eProfession, NO_UNIT))
		{
			int iNumRequired = GET_PLAYER(eActivePlayer).getYieldEquipmentAmount(eProfession, eYield);
			if (iNumRequired > 0)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_YIELD_NEEDED_FOR_PROFESSION", iNumRequired, GC.getProfessionInfo(eProfession).getTextKeyWide()));
			}
		}
	}
}

void CvDLLWidgetData::parseAssignTradeRoute(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvUnit* pUnit = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getUnit(widgetDataStruct.m_iData1);
	if (pUnit != NULL)
	{
		if (pUnit->getGroup()->isAssignedTradeRoute(widgetDataStruct.m_iData2))
		{
			szBuffer.assign(gDLL->getText("TXT_KEY_UNASSIGN_ROUTE"));
		}
		else
		{
			szBuffer.assign(gDLL->getText("TXT_KEY_ASSIGN_ROUTE"));
		}
	}
}

void CvDLLWidgetData::parseReceiveMoveCargoToTransportHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvUnit* pUnit = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getUnit(widgetDataStruct.m_iData1);
	if (pUnit != NULL)
	{
		GAMETEXT.setUnitHelp(szBuffer, pUnit, false, false);
	}
}

void CvDLLWidgetData::parseEventHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.setEventHelp(szBuffer, (EventTypes)widgetDataStruct.m_iData1, widgetDataStruct.m_iData2, GC.getGameINLINE().getActivePlayer());
}

void CvDLLWidgetData::parseImprovementHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setImprovementHelp(szBuffer, (ImprovementTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseCivicHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.parseCivicInfo(szBuffer, (CivicTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseCivilizationHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.parseCivInfos(szBuffer, (CivilizationTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseLeaderHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != -1)
	{
		GAMETEXT.parseLeaderTraits(szBuffer, (LeaderHeadTypes)widgetDataStruct.m_iData1, (CivilizationTypes)widgetDataStruct.m_iData2);
	}
}

void CvDLLWidgetData::parseCloseScreenHelp(CvWStringBuffer& szBuffer)
{
	szBuffer.assign(gDLL->getText("TXT_KEY_MISC_CLOSE_SCREEN"));
}

void CvDLLWidgetData::parseDescriptionHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer, bool bMinimal)
{
	CivilopediaPageTypes eType = (CivilopediaPageTypes)widgetDataStruct.m_iData1;
	switch (eType)
	{
	case CIVILOPEDIA_PAGE_UNIT:
		{
			UnitTypes eUnit = (UnitTypes)widgetDataStruct.m_iData2;
			if (NO_UNIT != eUnit)
			{
				szBuffer.assign(bMinimal ? GC.getUnitInfo(eUnit).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getUnitInfo(eUnit).getTextKeyWide()));
			}
		}
		break;
	case CIVILOPEDIA_PAGE_PROFESSION:
		{
			ProfessionTypes eProfession = (ProfessionTypes)widgetDataStruct.m_iData2;
			if (NO_PROFESSION != eProfession)
			{
				szBuffer.assign(bMinimal ? GC.getProfessionInfo(eProfession).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getProfessionInfo(eProfession).getTextKeyWide()));
			}
		}
		break;
	case CIVILOPEDIA_PAGE_BUILDING:
		{
			BuildingTypes eBuilding = (BuildingTypes)widgetDataStruct.m_iData2;
			if (NO_BUILDING != eBuilding)
			{
				szBuffer.assign(bMinimal ? GC.getBuildingInfo(eBuilding).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getBuildingInfo(eBuilding).getTextKeyWide()));
			}
		}
		break;
	case CIVILOPEDIA_PAGE_BONUS:
		{
			BonusTypes eBonus = (BonusTypes)widgetDataStruct.m_iData2;
			if (NO_BONUS != eBonus)
			{
				szBuffer.assign(bMinimal ? GC.getBonusInfo(eBonus).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getBonusInfo(eBonus).getTextKeyWide()));
			}
		}
		break;
	case CIVILOPEDIA_PAGE_IMPROVEMENT:
		{
			ImprovementTypes eImprovement = (ImprovementTypes)widgetDataStruct.m_iData2;
			if (NO_IMPROVEMENT != eImprovement)
			{
				szBuffer.assign(bMinimal ? GC.getImprovementInfo(eImprovement).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getImprovementInfo(eImprovement).getTextKeyWide()));
			}
		}
		break;
	case CIVILOPEDIA_PAGE_PROMOTION:
		{
			PromotionTypes ePromo = (PromotionTypes)widgetDataStruct.m_iData2;
			if (NO_PROMOTION != ePromo)
			{
				szBuffer.assign(bMinimal ? GC.getPromotionInfo(ePromo).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getPromotionInfo(ePromo).getTextKeyWide()));
			}
		}
		break;
	case CIVILOPEDIA_PAGE_CIV:
		{
			CivilizationTypes eCiv = (CivilizationTypes)widgetDataStruct.m_iData2;
			if (NO_CIVILIZATION != eCiv)
			{
				szBuffer.assign(bMinimal ? GC.getCivilizationInfo(eCiv).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getCivilizationInfo(eCiv).getTextKeyWide()));
			}
		}
		break;
	case CIVILOPEDIA_PAGE_LEADER:
		{
			LeaderHeadTypes eLeader = (LeaderHeadTypes)widgetDataStruct.m_iData2;
			if (NO_LEADER != eLeader)
			{
				szBuffer.assign(bMinimal ? GC.getLeaderHeadInfo(eLeader).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getLeaderHeadInfo(eLeader).getTextKeyWide()));
			}
		}
		break;
	case CIVILOPEDIA_PAGE_CIVIC:
		{
			CivicTypes eCivic = (CivicTypes)widgetDataStruct.m_iData2;
			if (NO_CIVIC != eCivic)
			{
				szBuffer.assign(bMinimal ? GC.getCivicInfo(eCivic).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getCivicInfo(eCivic).getTextKeyWide()));
			}
		}
		break;
	case CIVILOPEDIA_PAGE_CONCEPT:
		{
			ConceptTypes eConcept = (ConceptTypes)widgetDataStruct.m_iData2;
			if (NO_CONCEPT != eConcept)
			{
				szBuffer.assign(GC.getConceptInfo(eConcept).getDescription());
			}
		}
		break;
	default:
		break;
	}
}

void CvDLLWidgetData::parseKillDealHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
//	szBuffer = "Click to cancel";
	CvWString szTemp;
	szTemp = szBuffer.getCString();
	CvDeal* pDeal = GC.getGameINLINE().getDeal(widgetDataStruct.m_iData1);
	if (NULL != pDeal)
	{
		if (pDeal->isCancelable(GC.getGameINLINE().getActivePlayer(), &szTemp))
		{
			szTemp = gDLL->getText("TXT_KEY_MISC_CLICK_TO_CANCEL");
		}
	}

	szBuffer.assign(szTemp);
}


void CvDLLWidgetData::doDealKill(CvWidgetDataStruct &widgetDataStruct)
{
	CvDeal* pDeal = GC.getGameINLINE().getDeal(widgetDataStruct.m_iData1);
	if (pDeal != NULL)
	{
		if (!pDeal->isCancelable(GC.getGameINLINE().getActivePlayer()))
		{
			CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_TEXT);
			pInfo->setText(gDLL->getText("TXT_KEY_POPUP_CANNOT_CANCEL_DEAL"));
			gDLL->getInterfaceIFace()->addPopup(pInfo, GC.getGameINLINE().getActivePlayer(), true);
		}
		else
		{
			CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_DEAL_CANCELED);
			pInfo->setData1(pDeal->getID());
			pInfo->setOption1(false);
			gDLL->getInterfaceIFace()->addPopup(pInfo, GC.getGameINLINE().getActivePlayer(), true);
		}
	}
}


void CvDLLWidgetData::doRefreshMilitaryAdvisor(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	argsList.add(widgetDataStruct.m_iData2);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "refreshMilitaryAdvisor", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doCityUnitAssignCitizen(const CvWidgetDataStruct& destinationWidgetData, const CvWidgetDataStruct& sourceWidgetData)
{
	// citizen dragged and dropped somewhere
	CvCity* pCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	if (pCity == NULL || pCity->getOwnerINLINE() != GC.getGameINLINE().getActivePlayer())
	{
		return;
	}

	switch (destinationWidgetData.m_eWidgetType)
	{
	case WIDGET_CITY_UNIT_ASSIGN_PROFESSION:
		if (destinationWidgetData.m_iData2 > -1)
		{
			CvUnit* pUnit = pCity->getPopulationUnitById(sourceWidgetData.m_iData1);
			if (pUnit != NULL)
			{
				if (pCity->isAvailableProfessionSlot((ProfessionTypes) destinationWidgetData.m_iData2, pUnit))
				{
					gDLL->sendDoTask(pCity->getID(), TASK_CHANGE_PROFESSION, sourceWidgetData.m_iData1, destinationWidgetData.m_iData2, false, false, false, false);
				}
				else
				{
					for (int i = 0; i < pCity->getPopulation(); ++i)
					{
						CvUnit* pUnit = pCity->getPopulationUnitByIndex(i);
						if (pUnit->getProfession() == destinationWidgetData.m_iData2)
						{
							gDLL->sendDoTask(pCity->getID(), TASK_REPLACE_CITIZEN, sourceWidgetData.m_iData1, pUnit->getID(), false, false, false, false);
							break;
						}
					}
				}
			}
		}
		break;

	case WIDGET_ASSIGN_CITIZEN_TO_PLOT:
		doAssignCitizenToPlot(gDLL->getInterfaceIFace()->getHeadSelectedCity(), destinationWidgetData.m_iData1, sourceWidgetData.m_iData1);
		break;

	case WIDGET_CITIZEN:
		gDLL->sendDoTask(pCity->getID(), TASK_REPLACE_CITIZEN, sourceWidgetData.m_iData1, destinationWidgetData.m_iData1, true, false, false, false);
		break;

	case WIDGET_EJECT_CITIZEN:
		{
			CvUnit* pUnit = pCity->getPopulationUnitById(sourceWidgetData.m_iData1);
			if (pUnit != NULL)
			{
				CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_CHOOSE_PROFESSION, pCity->getID(), pUnit->getID(), 0);
				gDLL->getInterfaceIFace()->addPopup(pInfo, NO_PLAYER, true);
			}
		}
		break;

	case WIDGET_RECEIVE_MOVE_CARGO_TO_TRANSPORT:
		{
			CvCity* pCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
			CvUnit* pUnit = pCity->getPopulationUnitById(sourceWidgetData.m_iData1);
			CvUnit* pTransport = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getUnit(destinationWidgetData.m_iData1);

			if (NULL != pUnit && NULL != pTransport)
			{
				if (pUnit->canLoadUnit(pTransport, pTransport->plot(), true))
				{
					gDLL->sendDoTask(pCity->getID(), TASK_EJECT_TO_TRANSPORT, pUnit->getID(), pTransport->getID(), false, false, false, false);
				}
			}
		}
		break;

	default:
		break;
	}
}

void CvDLLWidgetData::doMoveCargoToCity(const CvWidgetDataStruct& destinationWidgetData, const CvWidgetDataStruct& sourceWidgetData)
{
	// cargo inside the city screen dropped somewhere

	CvCity* pCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	if (pCity == NULL || pCity->getOwnerINLINE() != GC.getGameINLINE().getActivePlayer())
	{
		return;
	}

	switch (destinationWidgetData.m_eWidgetType)
	{
	case WIDGET_HELP_YIELD:
	case WIDGET_RECEIVE_MOVE_CARGO_TO_CITY:
	case WIDGET_MOVE_CARGO_TO_TRANSPORT:
		if (gDLL->shiftKey())
		{
			CvUnit* pUnit = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getUnit(sourceWidgetData.m_iData2);
			if (pUnit != NULL)
			{
				CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_SELECT_YIELD_AMOUNT, pUnit->getYield(), sourceWidgetData.m_iData2, COMMAND_UNLOAD);
				pInfo->setOption1(false);
				gDLL->getInterfaceIFace()->addPopup(pInfo, NO_PLAYER, true);
			}
		}
		else
		{
			gDLL->sendDoCommand(sourceWidgetData.m_iData2, COMMAND_UNLOAD, -1, -1, false);
		}
		break;

	case WIDGET_CITY_UNIT_ASSIGN_PROFESSION:
		{
			int iUnitId = sourceWidgetData.m_iData2;
			CvUnit* pUnit = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getUnit(iUnitId);
			if (pUnit == NULL)
			{
				break;
			}
			ProfessionTypes eProfession = (ProfessionTypes) destinationWidgetData.m_iData2;
			if (eProfession == NO_PROFESSION)
			{
				break;
			}

			if (pUnit->canHaveProfession(eProfession, true, pCity->plot()))
			{
				if (pCity->isAvailableProfessionSlot(eProfession, pUnit))
				{
					gDLL->sendDoCommand(iUnitId, COMMAND_PROFESSION, eProfession, -1, false);
				}
				else
				{
					for (int i = 0; i < pCity->getPopulation(); ++i)
					{
						CvUnit* pLoopUnit = pCity->getPopulationUnitByIndex(i);
						if (pLoopUnit->getProfession() == eProfession)
						{
							gDLL->sendDoTask(pCity->getID(), TASK_REPLACE_CITIZEN, iUnitId, pLoopUnit->getID(), false, false, false, false);
							break;
						}
					}
				}
			}
		}
		break;

	case WIDGET_ASSIGN_CITIZEN_TO_PLOT:
		doAssignCitizenToPlot(gDLL->getInterfaceIFace()->getHeadSelectedCity(), destinationWidgetData.m_iData1, sourceWidgetData.m_iData2);
		break;

	case WIDGET_CITIZEN:
		{
			CvUnit* pUnit = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getUnit(sourceWidgetData.m_iData2);
			if (NULL != pUnit)
			{
				gDLL->sendDoTask(pCity->getID(), TASK_REPLACE_CITIZEN, sourceWidgetData.m_iData2, destinationWidgetData.m_iData1, true, false, false, false);
			}
		}
		break;

	case WIDGET_EJECT_CITIZEN:
		{
			CvUnit* pUnit = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getUnit(sourceWidgetData.m_iData2);
			if (pUnit != NULL)
			{
				gDLL->sendDoCommand(sourceWidgetData.m_iData2, COMMAND_UNLOAD, -1, -1, false);
			}
		}
		break;

	case WIDGET_RECEIVE_MOVE_CARGO_TO_TRANSPORT:
		{
			CvUnit* pUnit = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getUnit(sourceWidgetData.m_iData2);
			CvUnit* pTransport = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getUnit(destinationWidgetData.m_iData1);

			if (NULL != pUnit && NULL != pTransport)
			{
				gDLL->sendDoCommand(sourceWidgetData.m_iData2, COMMAND_LOAD_UNIT, pTransport->getOwnerINLINE(), destinationWidgetData.m_iData1, false);
			}
		}
		break;
	default:
		break;
	}
}

void CvDLLWidgetData::doMoveShipCargo(const CvWidgetDataStruct& destinationWidgetData, const CvWidgetDataStruct& sourceWidgetData)
{
	switch (destinationWidgetData.m_eWidgetType)
	{
	case WIDGET_SHIP_CARGO:
		{
			gDLL->sendPlayerAction(GC.getGameINLINE().getActivePlayer(), PLAYER_ACTION_TRANSFER_UNIT_IN_EUROPE, sourceWidgetData.m_iData1, destinationWidgetData.m_iData1, -1);
			// Move cargo from one ship to another (shift split?)
		}
		break;
	case WIDGET_DOCK:
		{
			CvUnit* pUnit = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getUnit(sourceWidgetData.m_iData1);
			if (pUnit != NULL && pUnit->isCargo())
			{
				if (!pUnit->isGoods())
				{
					gDLL->sendPlayerAction(GC.getGameINLINE().getActivePlayer(), PLAYER_ACTION_UNLOAD_UNIT_TO_EUROPE, sourceWidgetData.m_iData1, destinationWidgetData.m_iData1, -1);
				}
				else
				{
					if (gDLL->shiftKey())
					{
						CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_SELECT_YIELD_AMOUNT, pUnit->getYield(), sourceWidgetData.m_iData1, PLAYER_ACTION_SELL_YIELD_UNIT);
						pInfo->setOption1(false);
						pInfo->setOption2(true);
						gDLL->getInterfaceIFace()->addPopup(pInfo, NO_PLAYER, true);
					}
					else
					{
						gDLL->sendPlayerAction(GC.getGameINLINE().getActivePlayer(), PLAYER_ACTION_SELL_YIELD_UNIT, 0, pUnit->getYieldStored(), sourceWidgetData.m_iData1);
					}			
				}
			}
		}
		break;
	case WIDGET_MOVE_CARGO_TO_TRANSPORT:
		{
			PlayerTypes ePlayer = GC.getGameINLINE().getActivePlayer();
			CvUnit* pUnit = GET_PLAYER(ePlayer).getUnit(sourceWidgetData.m_iData1);
			if (pUnit != NULL && pUnit->isGoods())
			{
				if (gDLL->shiftKey())
				{
					CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_SELECT_YIELD_AMOUNT, pUnit->getYield(), sourceWidgetData.m_iData1, PLAYER_ACTION_SELL_YIELD_UNIT);
					pInfo->setOption1(false);
					pInfo->setOption2(true);
					gDLL->getInterfaceIFace()->addPopup(pInfo, NO_PLAYER, true);
				}
				else
				{
					gDLL->sendPlayerAction(GC.getGameINLINE().getActivePlayer(), PLAYER_ACTION_SELL_YIELD_UNIT, 0, pUnit->getYieldStored(), sourceWidgetData.m_iData1);
				}
			}
		}
		break;

	case WIDGET_SAIL:
		if (sourceWidgetData.m_iData2 == -1)
		{
			gDLL->sendDoCommand(sourceWidgetData.m_iData1, COMMAND_SAIL_TO_EUROPE, destinationWidgetData.m_iData1, -1, false);
		}
		break;

	default:
		break;
	}
}

void CvDLLWidgetData::doMoveDockUnit(const CvWidgetDataStruct& destinationWidgetData, const CvWidgetDataStruct& sourceWidgetData)
{
	switch (destinationWidgetData.m_eWidgetType)
	{
	case WIDGET_SHIP_CARGO:
		{
			gDLL->sendPlayerAction(GC.getGameINLINE().getActivePlayer(), PLAYER_ACTION_LOAD_UNIT_FROM_EUROPE, sourceWidgetData.m_iData1, destinationWidgetData.m_iData1, -1);
		}
		break;

	default:
		break;
	}
}

void CvDLLWidgetData::doUnitIntoCity(const CvWidgetDataStruct& destinationWidgetData, const CvWidgetDataStruct& sourceWidgetData)
{
	// garrison from city screen citizen dragged and dropped somewhere

	CvCity* pCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	if (pCity == NULL || pCity->getOwnerINLINE() != GC.getGameINLINE().getActivePlayer())
	{
		return;
	}

	switch (destinationWidgetData.m_eWidgetType)
	{
	case WIDGET_CITY_UNIT_ASSIGN_PROFESSION:
		{
			int iUnitId = sourceWidgetData.m_iData1;
			CvUnit* pUnit = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getUnit(iUnitId);
			if (pUnit == NULL)
			{
				break;
			}
			ProfessionTypes eProfession = (ProfessionTypes) destinationWidgetData.m_iData2;
			if (eProfession == NO_PROFESSION)
			{
				break;
			}

			if (pUnit->canHaveProfession(eProfession, true, pCity->plot()))
			{
				if (pCity->isAvailableProfessionSlot(eProfession, pUnit))
				{
					gDLL->sendDoCommand(iUnitId, COMMAND_PROFESSION, eProfession, -1, false);
				}
				else
				{
					for (int i = 0; i < pCity->getPopulation(); ++i)
					{
						CvUnit* pLoopUnit = pCity->getPopulationUnitByIndex(i);
						if (pLoopUnit->getProfession() == eProfession)
						{
							gDLL->sendDoTask(pCity->getID(), TASK_REPLACE_CITIZEN, iUnitId, pLoopUnit->getID(), false, false, false, false);
							break;
						}
					}
				}
			}
		}
		break;

	case WIDGET_ASSIGN_CITIZEN_TO_PLOT:
		doAssignCitizenToPlot(gDLL->getInterfaceIFace()->getHeadSelectedCity(), destinationWidgetData.m_iData1, sourceWidgetData.m_iData1);
		break;

	case WIDGET_CITIZEN:
		{
			CvUnit* pUnit = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getUnit(sourceWidgetData.m_iData1);
			if (NULL != pUnit)
			{
				gDLL->sendDoTask(pCity->getID(), TASK_REPLACE_CITIZEN, sourceWidgetData.m_iData1, destinationWidgetData.m_iData1, true, false, false, false);
			}
		}
		break;

	case WIDGET_RECEIVE_MOVE_CARGO_TO_TRANSPORT:
		{
			CvUnit* pUnit = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getUnit(sourceWidgetData.m_iData1);
			CvUnit* pTransport = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getUnit(destinationWidgetData.m_iData1);
			
			if (NULL != pUnit && NULL != pTransport)
			{
				gDLL->sendDoCommand(sourceWidgetData.m_iData1, COMMAND_LOAD_UNIT, pTransport->getOwnerINLINE(), destinationWidgetData.m_iData1, false);
			}
		}
		break;

	default:
		break;
	}
}

void CvDLLWidgetData::doMoveCargoToTransport(const CvWidgetDataStruct& destinationWidgetData, const CvWidgetDataStruct& sourceWidgetData)
{
	// a yield dropped somewhere (could be in Europe or in City)

	switch (destinationWidgetData.m_eWidgetType)
	{
	case WIDGET_RECEIVE_MOVE_CARGO_TO_TRANSPORT:
	case WIDGET_MOVE_CARGO_TO_CITY:
		// City screen
		{
			CvCity* pCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
			if (pCity == NULL || pCity->getOwnerINLINE() != GC.getGameINLINE().getActivePlayer())
			{
				break;
			}

			if (gDLL->shiftKey())
			{
				CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_SELECT_YIELD_AMOUNT, sourceWidgetData.m_iData1, destinationWidgetData.m_iData1, COMMAND_LOAD_YIELD);
				pInfo->setOption1(true);
				gDLL->getInterfaceIFace()->addPopup(pInfo, NO_PLAYER, true);
			}
			else
			{
				gDLL->sendDoCommand(destinationWidgetData.m_iData1, COMMAND_LOAD_YIELD, sourceWidgetData.m_iData1, -1, false);
			}
		}
		break;

	case WIDGET_SHIP_CARGO:
		// Europe
		if (gDLL->shiftKey())
		{
			CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_SELECT_YIELD_AMOUNT, sourceWidgetData.m_iData1, destinationWidgetData.m_iData1, PLAYER_ACTION_BUY_YIELD_UNIT);
			pInfo->setOption1(true);
			gDLL->getInterfaceIFace()->addPopup(pInfo, NO_PLAYER, true);
		}
		else
		{
			PlayerTypes ePlayer = GC.getGameINLINE().getActivePlayer();
			CvUnit* pUnit = GET_PLAYER(ePlayer).getUnit(destinationWidgetData.m_iData1);
			int iAmount = pUnit->getLoadYieldAmount((YieldTypes) sourceWidgetData.m_iData1);
			if (iAmount > 0)
			{
				gDLL->sendPlayerAction(GC.getGameINLINE().getActivePlayer(), PLAYER_ACTION_BUY_YIELD_UNIT, sourceWidgetData.m_iData1, iAmount, destinationWidgetData.m_iData1);
			}
		}
		break;

	default:
		break;
	}
}

void CvDLLWidgetData::doDoubleClickCitizen(const CvWidgetDataStruct& widgetDataStruct)
{
	CvCity* pHeadSelectedCity;
	if (widgetDataStruct.m_iData2 != -1)
	{
		pHeadSelectedCity = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCity(widgetDataStruct.m_iData2);
	}
	else
	{
		pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	}
	if (pHeadSelectedCity != NULL && pHeadSelectedCity->getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
	{
		CvUnit* pUnit = pHeadSelectedCity->getPopulationUnitById(widgetDataStruct.m_iData1);
		if (pUnit != NULL)
		{
			CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_CHOOSE_PROFESSION, pHeadSelectedCity->getID(), pUnit->getID());
			gDLL->getInterfaceIFace()->addPopup(pInfo, NO_PLAYER, true);
		}
	}
}

void CvDLLWidgetData::doDoubleClickDock(const CvWidgetDataStruct& widgetDataStruct)
{
	CvUnit* pUnit = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getEuropeUnitById(widgetDataStruct.m_iData1);
	if (pUnit != NULL)
	{
		CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_CHOOSE_PROFESSION, -1, pUnit->getID(), 0);
		gDLL->getInterfaceIFace()->addPopup(pInfo, NO_PLAYER, true);
	}
}

void CvDLLWidgetData::doCreateTradeRoute(const CvWidgetDataStruct& widgetDataStruct)
{
	if (GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getNumCities() >= 2)
	{
		gDLL->getEventReporterIFace()->createTradeRoute(GC.getGameINLINE().getActivePlayer());
	}
}

void CvDLLWidgetData::doEditTradeRoute(const CvWidgetDataStruct& widgetDataStruct)
{
	if (GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getTradeRoute(widgetDataStruct.m_iData1) != NULL)
	{
		gDLL->getEventReporterIFace()->editTradeRoute(GC.getGameINLINE().getActivePlayer(), widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::doYieldImportExport(const CvWidgetDataStruct& widgetDataStruct)
{
	CvCity* pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	if (pHeadSelectedCity != NULL && pHeadSelectedCity->getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
	{
		CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_YIELD_IMPORT_EXPORT, gDLL->getInterfaceIFace()->getHeadSelectedCity()->getID());
		gDLL->getInterfaceIFace()->addPopup(pInfo, NO_PLAYER, true);
	}
}

void CvDLLWidgetData::doAssignTradeRoute(const CvWidgetDataStruct& widgetDataStruct)
{
	CvUnit* pUnit = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getUnit(widgetDataStruct.m_iData1);
	if (pUnit != NULL)
	{
		gDLL->sendDoCommand(pUnit->getID(), COMMAND_ASSIGN_TRADE_ROUTE, widgetDataStruct.m_iData2, !pUnit->getGroup()->isAssignedTradeRoute(widgetDataStruct.m_iData2), false);
	}
}

void CvDLLWidgetData::parseProductionModHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	if (NULL != pCity)
	{
		GAMETEXT.setProductionHelp(szBuffer, *pCity);
	}
}

void CvDLLWidgetData::parseLeaderheadHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.parseLeaderHeadHelp(szBuffer, (PlayerTypes)widgetDataStruct.m_iData1, (PlayerTypes)widgetDataStruct.m_iData2);
}

void CvDLLWidgetData::parseLeaderLineHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.parseLeaderLineHelp(szBuffer, (PlayerTypes)widgetDataStruct.m_iData1, (PlayerTypes)widgetDataStruct.m_iData2);
}

void CvDLLWidgetData::parseScoreHelp(CvWidgetDataStruct& widgetDataStruct, CvWStringBuffer& szBuffer)
{
	GAMETEXT.setScoreHelp(szBuffer, (PlayerTypes)widgetDataStruct.m_iData1);
}

void CvDLLWidgetData::parseImportExportHelp(CvWidgetDataStruct& widgetDataStruct, CvWStringBuffer& szBuffer)
{
	szBuffer.append(gDLL->getText("TXT_KEY_HELP_EDIT_IMPORTS"));
}

void CvDLLWidgetData::parseEjectCitizenHelp(CvWidgetDataStruct& widgetDataStruct, CvWStringBuffer& szBuffer)
{
	CvCity* pCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	if (pCity != NULL)
	{
		CvUnit* pUnit = GET_PLAYER(pCity->getOwnerINLINE()).getUnit(widgetDataStruct.m_iData1);
		if (pUnit != NULL)
		{
			GAMETEXT.setUnitHelp(szBuffer, pUnit, false, false);
		}
		else
		{
			szBuffer.append(gDLL->getText("TXT_KEY_EJECT_CITIZEN"));
		}
	}
}

void CvDLLWidgetData::doAssignCitizenToPlot(CvCity* pCity, int iPlotIndex, int iUnitId)
{
	if (NULL != pCity)
	{
		if (pCity->getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
		{
			CvUnit* pOtherUnit = pCity->getUnitWorkingPlot(iPlotIndex);
			if (pOtherUnit != NULL)
			{
				gDLL->sendDoTask(pCity->getID(), TASK_REPLACE_CITIZEN, iUnitId, pOtherUnit->getID(), true, false, false, false);
			}
			else
			{
				gDLL->sendDoTask(pCity->getID(), TASK_CHANGE_WORKING_PLOT, iPlotIndex, iUnitId, true, false, false, false);
			}
		}
	}
}

void CvDLLWidgetData::doGoToCity(const CvWidgetDataStruct &widgetDataStruct)
{
	CvCity* pCity = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCity(widgetDataStruct.m_iData1);
	if (pCity != NULL)
	{
		gDLL->getInterfaceIFace()->selectCity(pCity);
	}
}
