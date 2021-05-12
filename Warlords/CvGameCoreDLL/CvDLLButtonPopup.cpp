// buttonPopup.cpp

#include "CvGameCoreDLL.h"
#include "CvDLLButtonPopup.h"
#include "CvGlobals.h"
#include "CvPlayerAI.h"
#include "CvGameAI.h"
#include "CvTeamAI.h"
#include "CvMap.h" 
#include "CvPlot.h"
#include "CvArtFileMgr.h"
#include "CyCity.h"
#include "CyArgsList.h"
#include "CvPopupReturn.h"
#include "CvInfos.h"
#include "CvInitCore.h"
#include "CvGameTextMgr.h"
#include "CvDLLPythonIFaceBase.h"
#include "CvDLLInterfaceIFaceBase.h"
#include "CvDLLEntityIFaceBase.h"
#include "CvGameCoreUtils.h"
#include "CvDLLEngineIFaceBase.h"
#include "CvDLLEventReporterIFaceBase.h"

// Public Functions...

#define PASSWORD_DEFAULT (L"*****")

CvDLLButtonPopup* CvDLLButtonPopup::m_pInst = NULL;

CvDLLButtonPopup& CvDLLButtonPopup::getInstance()
{
	if (m_pInst == NULL)
	{
		m_pInst = new CvDLLButtonPopup;
	}
	return *m_pInst;
}

void CvDLLButtonPopup::freeInstance()
{
	delete m_pInst;
	m_pInst = NULL;
}

CvDLLButtonPopup::CvDLLButtonPopup()
{
}


CvDLLButtonPopup::~CvDLLButtonPopup()
{
}

void CvDLLButtonPopup::OnAltExecute(CvPopup& popup, const PopupReturn& popupReturn, CvPopupInfo &info)
{
	CvPopupInfo* pInfo = new CvPopupInfo;
	if (pInfo)
	{
		*pInfo = info;
		gDLL->getInterfaceIFace()->addPopup(pInfo);
		gDLL->getInterfaceIFace()->popupSetAsCancelled(&popup);
	}
}


void CvDLLButtonPopup::OnOkClicked(CvPopup* pPopup, PopupReturn *pPopupReturn, CvPopupInfo &info)
{
	int iExamineCityID;
	int iI;

	switch (info.getButtonPopupType())
	{
	case BUTTONPOPUP_TEXT:
		break;

	case BUTTONPOPUP_CONFIRM_MENU:
		if ( pPopupReturn->getButtonClicked() == 0 )
		{
			switch (info.getData1())
			{
			case 0:
				gDLL->SetDone(true);
				break;
			case 1:
				gDLL->getInterfaceIFace()->exitingToMainMenu();
				break;
			case 2:
				GC.getGameINLINE().doControl(CONTROL_RETIRE);
				break;
			case 3:
				GC.getGameINLINE().regenerateMap();
				break;
			case 4:
				GC.getGameINLINE().doControl(CONTROL_WORLD_BUILDER);
				break;
			}
		}
		break;

	case BUTTONPOPUP_MAIN_MENU:
		if (pPopupReturn->getButtonClicked() == 0)
		{	// exit to desktop
			CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_CONFIRM_MENU);
			if (NULL != pInfo)
			{
				pInfo->setData1(0);
				gDLL->getInterfaceIFace()->addPopup(pInfo, GC.getGameINLINE().getActivePlayer(), true);
			}
		}
		else if (pPopupReturn->getButtonClicked() == 1)
		{	// exit to main menu
			CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_CONFIRM_MENU);
			if (NULL != pInfo)
			{
				pInfo->setData1(1);
				gDLL->getInterfaceIFace()->addPopup(pInfo, GC.getGameINLINE().getActivePlayer(), true);
			}
		}
		else if (pPopupReturn->getButtonClicked() == 2)
		{
			CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_CONFIRM_MENU);
			if (NULL != pInfo)
			{
				pInfo->setData1(2);
				gDLL->getInterfaceIFace()->addPopup(pInfo, GC.getGameINLINE().getActivePlayer(), true);
			}
		}
		else if (pPopupReturn->getButtonClicked() == 3)
		{
			CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_CONFIRM_MENU);
			if (NULL != pInfo)
			{
				pInfo->setData1(3);
				gDLL->getInterfaceIFace()->addPopup(pInfo, GC.getGameINLINE().getActivePlayer(), true);
			}
		}
		else if (pPopupReturn->getButtonClicked() == 4)
		{	// load game
			GC.getGameINLINE().doControl(CONTROL_LOAD_GAME);
		}
		else if (pPopupReturn->getButtonClicked() == 5)
		{	// save game
			GC.getGameINLINE().doControl(CONTROL_SAVE_NORMAL);
		}
		else if (pPopupReturn->getButtonClicked() == 6)
		{	// options
			gDLL->getPythonIFace()->callFunction("CvScreensInterface", "showOptionsScreen");
		}
		else if (pPopupReturn->getButtonClicked() == 7)
		{
			CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_CONFIRM_MENU);
			if (NULL != pInfo)
			{
				pInfo->setData1(4);
				gDLL->getInterfaceIFace()->addPopup(pInfo, GC.getGameINLINE().getActivePlayer(), true);
			}
		}
		else if (pPopupReturn->getButtonClicked() == 8)
		{	// Game details
			GC.getGameINLINE().doControl(CONTROL_ADMIN_DETAILS);
		}
		else if (pPopupReturn->getButtonClicked() == 9)
		{	// player details
			GC.getGameINLINE().doControl(CONTROL_DETAILS);
		}
		// 10 - cancel
		break;

	case BUTTONPOPUP_DECLAREWARMOVE:
		if (pPopupReturn->getButtonClicked() == 0)
		{
			gDLL->sendChangeWar((TeamTypes)info.getData1(), true);
		}
		if (((pPopupReturn->getButtonClicked() == 0) || info.getOption2()) && info.getFlags() == 0)
		{
			GC.getGameINLINE().selectionListGameNetMessage(GAMEMESSAGE_PUSH_MISSION, MISSION_MOVE_TO, info.getData2(), info.getData3(), info.getFlags(), false, info.getOption1());
		}
		break;

	case BUTTONPOPUP_CONFIRMCOMMAND:
		if (pPopupReturn->getButtonClicked() == 0)
		{
			int iAction = info.getData1();
			GC.getGameINLINE().selectionListGameNetMessage(GAMEMESSAGE_DO_COMMAND, GC.getActionInfo(iAction).getCommandType(), GC.getActionInfo(iAction).getCommandData(), -1, 0, info.getOption1());
		}
		break;

	case BUTTONPOPUP_LOADUNIT:
		if (pPopupReturn->getButtonClicked() != 0)
		{
			CLLNode<IDInfo>* pUnitNode;
			CvSelectionGroup* pSelectionGroup;
			CvUnit* pLoopUnit;
			CvPlot* pPlot;
			int iCount;

			pSelectionGroup = gDLL->getInterfaceIFace()->getSelectionList();

			if (NULL != pSelectionGroup)
			{
				pPlot = pSelectionGroup->plot();

				iCount = pPopupReturn->getButtonClicked();

				pUnitNode = pPlot->headUnitNode();

				while (pUnitNode != NULL)
				{
					pLoopUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = pPlot->nextUnitNode(pUnitNode);

					if (pSelectionGroup->canDoCommand(COMMAND_LOAD_UNIT, pLoopUnit->getOwnerINLINE(), pLoopUnit->getID()))
					{
						iCount--;
						if (iCount == 0)
						{
							GC.getGameINLINE().selectionListGameNetMessage(GAMEMESSAGE_DO_COMMAND, COMMAND_LOAD_UNIT, pLoopUnit->getOwnerINLINE(), pLoopUnit->getID());
							break;
						}
					}
				}
			}
		}
		break;

	case BUTTONPOPUP_LEADUNIT:
		if (pPopupReturn->getButtonClicked() != 0)
		{
			CLLNode<IDInfo>* pUnitNode;
			CvSelectionGroup* pSelectionGroup;
			CvUnit* pLoopUnit;
			CvPlot* pPlot;
			int iCount;

			pSelectionGroup = gDLL->getInterfaceIFace()->getSelectionList();

			if (NULL != pSelectionGroup)
			{
				pPlot = pSelectionGroup->plot();

				iCount = pPopupReturn->getButtonClicked();

				pUnitNode = pPlot->headUnitNode();

				while (pUnitNode != NULL)
				{
					pLoopUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = pPlot->nextUnitNode(pUnitNode);

					if (pLoopUnit->canPromote((PromotionTypes) info.getData1(), info.getData2()))
					{
						iCount--;
						if (iCount == 0)
						{
							GC.getGameINLINE().selectionListGameNetMessage(GAMEMESSAGE_PUSH_MISSION, MISSION_LEAD, pLoopUnit->getID());
							break;
						}
					}
				}
			}
		}
		break;

	case BUTTONPOPUP_CHOOSETECH:
		if (pPopupReturn->getButtonClicked() == GC.getNumTechInfos())
		{
			gDLL->getPythonIFace()->callFunction("CvScreensInterface", "showTechChooser");
			GET_PLAYER(GC.getGameINLINE().getActivePlayer()).chooseTech(0, "", true);
		}
		break;

	case BUTTONPOPUP_RAZECITY:
		if (pPopupReturn->getButtonClicked() == 1)
		{
			gDLL->sendDoTask(info.getData1(), TASK_RAZE, -1, -1, false);
		}
		else if (pPopupReturn->getButtonClicked() == 2)
		{
			CvCity* pCity = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCity(info.getData1());
			if (NULL != pCity)
			{
				gDLL->getEventReporterIFace()->cityAcquiredAndKept(GC.getGameINLINE().getActivePlayer(), pCity);
			}

			gDLL->sendDoTask(info.getData1(), TASK_GIFT, info.getData2(), -1, false);
		}
		else if (pPopupReturn->getButtonClicked() == 0)
		{
			CvCity* pCity = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCity(info.getData1());
			if (NULL != pCity)
			{
				pCity->chooseProduction();
				gDLL->getEventReporterIFace()->cityAcquiredAndKept(GC.getGameINLINE().getActivePlayer(), pCity);
			}
		}
		break;

	case BUTTONPOPUP_DISBANDCITY:
		if (pPopupReturn->getButtonClicked() == 1)
		{
			gDLL->sendDoTask(info.getData1(), TASK_DISBAND, -1, -1, false);
		}
		else if (pPopupReturn->getButtonClicked() == 0)
		{
			CvCity* pCity = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCity(info.getData1());
			if (NULL != pCity)
			{
				pCity->chooseProduction();
			}
		}
		break;

	case BUTTONPOPUP_CHOOSEPRODUCTION:
		iExamineCityID = 0;
		iExamineCityID = max(iExamineCityID, GC.getNumUnitInfos());
		iExamineCityID = max(iExamineCityID, GC.getNumBuildingInfos());
		iExamineCityID = max(iExamineCityID, GC.getNumProjectInfos());
		iExamineCityID = max(iExamineCityID, GC.getNumProcessInfos());

		if (pPopupReturn->getButtonClicked() == iExamineCityID)
		{
			CvCity* pCity = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCity(info.getData1());
			if (pCity != NULL)
			{
				gDLL->getInterfaceIFace()->selectCity(pCity, true);
			}
		}
		break;

	case BUTTONPOPUP_CHANGECIVIC:
		if (pPopupReturn->getButtonClicked() == 0)
		{
			CivicTypes* paeNewCivics = new CivicTypes[GC.getNumCivicOptionInfos()];

			for (iI = 0; iI < GC.getNumCivicOptionInfos(); iI++)
			{
				paeNewCivics[iI] = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCivics((CivicOptionTypes)iI);
			}

			paeNewCivics[info.getData1()] = ((CivicTypes)(info.getData2()));

			gDLL->sendUpdateCivics(paeNewCivics);

			SAFE_DELETE(paeNewCivics);
		}
		else if (pPopupReturn->getButtonClicked() == 2)
		{
			gDLL->getPythonIFace()->callFunction(PYScreensModule, "showCivicsScreen");
		}
		break;

	case BUTTONPOPUP_CHANGERELIGION:
		if (pPopupReturn->getButtonClicked() == 0)
		{
			gDLL->sendConvert((ReligionTypes)(info.getData1()));
		}
		break;

	case BUTTONPOPUP_CHOOSEELECTION:
		if (pPopupReturn->getButtonClicked() < GC.getNumVoteInfos())
		{
			gDLL->sendChooseElection((VoteTypes)(pPopupReturn->getButtonClicked()));
		}
		break;

	case BUTTONPOPUP_DIPLOVOTE:
		if (pPopupReturn->getButtonClicked() == MAX_CIV_TEAMS)
		{
			gDLL->sendDiploVote(((VoteTypes)(info.getData1())), -1);
		}
		else
		{
			gDLL->sendDiploVote(((VoteTypes)(info.getData1())), ((TeamTypes)(pPopupReturn->getButtonClicked())));
		}
		break;

	case BUTTONPOPUP_DIPLOVOTE_RESULTS:
		break;

	case BUTTONPOPUP_ALARM:
		break;

	case BUTTONPOPUP_DEAL_CANCELED:
		if (pPopupReturn->getButtonClicked() == 0)
		{
			gDLL->sendKillDeal(info.getData1());
		}
		break;

	case BUTTONPOPUP_PYTHON:
		if (!info.getOnClickedPythonCallback().IsEmpty())
		{
			CyArgsList argsList;
			argsList.add(pPopupReturn->getButtonClicked());
			argsList.add(info.getData1());
			argsList.add(info.getData2());
			argsList.add(info.getData3());
			argsList.add(info.getFlags());
			argsList.add(info.getText());
			argsList.add(info.getOption1());
			argsList.add(info.getOption2());
			gDLL->getPythonIFace()->callFunction(PYScreensModule, info.getOnClickedPythonCallback(), argsList.makeFunctionArgs());
			break;
		}
		break;

	case BUTTONPOPUP_DETAILS:
		{
			// Civ details
			PlayerTypes eID = GC.getInitCore().getActivePlayer();

			CvWString szLeaderName = GC.getInitCore().getLeaderName(eID);
			CvWString szCivDescription = GC.getInitCore().getCivDescription(eID);
			CvWString szCivShortDesc = GC.getInitCore().getCivShortDesc(eID);
			CvWString szCivAdjective = GC.getInitCore().getCivAdjective(eID);
			CvWString szCivPassword = PASSWORD_DEFAULT;
			CvString szEmail = GC.getInitCore().getEmail(eID);
			CvString szSmtpHost = GC.getInitCore().getSmtpHost(eID);

			if (pPopupReturn->getEditBoxString(0) && *(pPopupReturn->getEditBoxString(0)))
			{
				szLeaderName = pPopupReturn->getEditBoxString(0);
			}
			if (pPopupReturn->getEditBoxString(1) && *(pPopupReturn->getEditBoxString(1)))
			{
				szCivDescription = pPopupReturn->getEditBoxString(1);
			}
			if (pPopupReturn->getEditBoxString(2) && *(pPopupReturn->getEditBoxString(2)))
			{
				szCivShortDesc = pPopupReturn->getEditBoxString(2);
			}
			if (pPopupReturn->getEditBoxString(3) && *(pPopupReturn->getEditBoxString(3)))
			{
				szCivAdjective = pPopupReturn->getEditBoxString(3);
			}
			if (GC.getGameINLINE().isHotSeat() || GC.getGameINLINE().isPbem())
			{
				if (pPopupReturn->getEditBoxString(4) && *(pPopupReturn->getEditBoxString(4)))
				{
					szCivPassword = pPopupReturn->getEditBoxString(4);
				}
			}
			if (GC.getGameINLINE().isPitboss() || GC.getGameINLINE().isPbem())
			{
				if (pPopupReturn->getEditBoxString(5) && *(pPopupReturn->getEditBoxString(5)))
				{
					szEmail = CvString(pPopupReturn->getEditBoxString(5));
				}
			}
			if (GC.getGameINLINE().isPbem())
			{
				if (pPopupReturn->getEditBoxString(6) && *(pPopupReturn->getEditBoxString(6)))
				{
					szSmtpHost = CvString(pPopupReturn->getEditBoxString(6));
				}
			}

			/* HRF: All params straight from init core
			gDLL->sendPlayerInfo(
				eID,
				szLeaderName,
				szCivDescription,
				szCivShortDesc,
				szCivAdjective,
				szCivPassword,
				szEmail,
				szSmtpHost,
				GC.getInitCore().getFlagDecal(eID),
				GC.getInitCore().getWhiteFlag(eID),
				GC.getInitCore().getHandicap(eID),
				GC.getInitCore().getCiv(eID),
				GC.getInitCore().getTeam(eID),
				GC.getInitCore().getLeader(eID),
				GC.getInitCore().getColor(eID),
				GC.getInitCore().getArtStyle(eID),
				GC.getInitCore().getSlotAssigned(eID),
				GC.getInitCore().getSlotStatus(eID),
				GC.getInitCore().getReady(eID)
				);
			*/
			GC.getInitCore().setLeaderName(eID, szLeaderName);
			GC.getInitCore().setCivDescription(eID, szCivDescription);
			GC.getInitCore().setCivShortDesc(eID, szCivShortDesc);
			GC.getInitCore().setCivAdjective(eID, szCivAdjective);
			if (szCivPassword != PASSWORD_DEFAULT)
			{
				GC.getInitCore().setCivPassword(eID, szCivPassword);
			}
			GC.getInitCore().setEmail(eID, szEmail);
			GC.getInitCore().setSmtpHost(eID, szSmtpHost);
			gDLL->sendPlayerInfo(eID);

			if (GC.getGameINLINE().isPbem() && pPopupReturn->getButtonClicked() == 0)
			{
				gDLL->sendPbemTurn(NO_PLAYER);
			}

		}
		break;

	case BUTTONPOPUP_ADMIN:
		{
			// Game details
			CvWString szGameName;
			CvWString szAdminPassword = GC.getInitCore().getAdminPassword();
			if (pPopupReturn->getEditBoxString(0) && *(pPopupReturn->getEditBoxString(0)))
			{
				szGameName = pPopupReturn->getEditBoxString(0);
			}
			if (pPopupReturn->getEditBoxString(1) && CvWString(pPopupReturn->getEditBoxString(1)) != PASSWORD_DEFAULT)
			{
				if (*(pPopupReturn->getEditBoxString(1)))
				{
					szAdminPassword = CvWString(gDLL->md5String((char*)CvString(pPopupReturn->getEditBoxString(1)).GetCString()));
				}
				else
				{
					szAdminPassword = L"";
				}
			}
			if (!GC.getGameINLINE().isGameMultiPlayer())
			{
				if (pPopupReturn->getCheckboxBitfield(2) && pPopupReturn->getCheckboxBitfield(2) > 0)
				{
					gDLL->setChtLvl(1);
				}
				else
				{
					gDLL->setChtLvl(0);
				}
			}

			gDLL->sendGameInfo(szGameName, szAdminPassword);

		}
		break;

	case BUTTONPOPUP_ADMIN_PASSWORD:
		{
			CvWString szAdminPassword;
			if (pPopupReturn->getEditBoxString(0) && CvWString(pPopupReturn->getEditBoxString(0)) != PASSWORD_DEFAULT)
			{
				szAdminPassword = pPopupReturn->getEditBoxString(0);
			}
			if (CvWString(gDLL->md5String((char*)CvString(szAdminPassword).GetCString())) == GC.getInitCore().getAdminPassword())
			{
				switch ((ControlTypes)info.getData1())
				{
				case CONTROL_WORLD_BUILDER:
					gDLL->getInterfaceIFace()->setWorldBuilder(!(gDLL->GetWorldBuilderMode()));
					break;
				case CONTROL_ADMIN_DETAILS:
					gDLL->getInterfaceIFace()->showAdminDetails();
					break;
				default:
					break;
				}
			}
			else
			{
				CvPopupInfo* pInfo = new CvPopupInfo();
				if (NULL != pInfo)
				{
					pInfo->setText(gDLL->getText("TXT_KEY_BAD_PASSWORD_DESC"));
					gDLL->getInterfaceIFace()->addPopup(pInfo, NO_PLAYER, true);
				}
			}
		}
		break;

	case BUTTONPOPUP_EXTENDED_GAME:
		if (pPopupReturn->getButtonClicked() == 0)
		{
			if (GC.getGameINLINE().isNetworkMultiPlayer())
			{
				gDLL->sendExtendedGame();
			}
			else
			{
				GC.getGameINLINE().setGameState(GAMESTATE_EXTENDED);
			}
		}
		else if (pPopupReturn->getButtonClicked() == 1)
		{
			// exit to main menu
			if (GC.getGameINLINE().isNetworkMultiPlayer() && GC.getGameINLINE().canDoControl(CONTROL_RETIRE))
			{
				GC.getGameINLINE().doControl(CONTROL_RETIRE);
			}
			else
			{
				gDLL->getInterfaceIFace()->exitingToMainMenu();
			}
		}
		break;

	case BUTTONPOPUP_DIPLOMACY:
		if (pPopupReturn->getButtonClicked() != MAX_CIV_PLAYERS)
		{
			GET_PLAYER(GC.getGameINLINE().getActivePlayer()).contact((PlayerTypes)(pPopupReturn->getButtonClicked()));
		}
		break;

	case BUTTONPOPUP_ADDBUDDY:
		if (pPopupReturn->getButtonClicked() == 0)
		{
			gDLL->AcceptBuddy(CvString(info.getText()).GetCString(), info.getData1());
		}
		else
		{
			gDLL->RejectBuddy(CvString(info.getText()).GetCString(), info.getData1());
		}
		break;

	case BUTTONPOPUP_FORCED_DISCONNECT:
	case BUTTONPOPUP_PITBOSS_DISCONNECT:
	case BUTTONPOPUP_KICKED:
		gDLL->getInterfaceIFace()->exitingToMainMenu();
		break;

	case BUTTONPOPUP_VASSAL_DEMAND_TRIBUTE:
		if (pPopupReturn->getButtonClicked() < GC.getNumBonusInfos())
		{
			PlayerTypes eVassal = (PlayerTypes)info.getData1();
			if (GET_PLAYER(eVassal).isHuman())
			{
				CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_VASSAL_GRANT_TRIBUTE, GC.getGameINLINE().getActivePlayer(), pPopupReturn->getButtonClicked());
				if (NULL != pInfo)
				{
					gDLL->sendPopup(eVassal, pInfo);
				}
			}
			else
			{
				TradeData item;
				setTradeItem(&item, TRADE_RESOURCES, pPopupReturn->getButtonClicked());

				CLinkList<TradeData> ourList;
				CLinkList<TradeData> theirList;
				theirList.insertAtEnd(item);

				if (GET_PLAYER(eVassal).AI_considerOffer(GC.getGameINLINE().getActivePlayer(), &ourList, &theirList))
				{
					gDLL->sendImplementDealMessage(eVassal, &ourList, &theirList);

					CvWString szBuffer = gDLL->getText("TXT_KEY_VASSAL_GRANT_TRIBUTE_ACCEPTED", GET_PLAYER(eVassal).getNameKey(), GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getNameKey(), GC.getBonusInfo((BonusTypes)pPopupReturn->getButtonClicked()).getTextKeyWide());
					gDLL->getInterfaceIFace()->addMessage(GC.getGameINLINE().getActivePlayer(), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer);
				}
				else
				{
					gDLL->sendChangeWar(GET_PLAYER(eVassal).getTeam(), true);
				}
			}
		}
		break;

	case BUTTONPOPUP_VASSAL_GRANT_TRIBUTE:
		if (pPopupReturn->getButtonClicked() == 0)
		{
			TradeData item;
			setTradeItem(&item, TRADE_RESOURCES, info.getData2());

			CLinkList<TradeData> ourList;
			CLinkList<TradeData> theirList;
			ourList.insertAtEnd(item);
			
			gDLL->sendImplementDealMessage((PlayerTypes)info.getData1(), &ourList, &theirList);

			CvWString szBuffer = gDLL->getText("TXT_KEY_VASSAL_GRANT_TRIBUTE_ACCEPTED", GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getNameKey(), GET_PLAYER((PlayerTypes)info.getData1()).getNameKey(), GC.getBonusInfo((BonusTypes)info.getData2()).getTextKeyWide());
			gDLL->getInterfaceIFace()->addMessage((PlayerTypes)info.getData1(), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer);
		}
		else
		{
			gDLL->sendChangeWar(GET_PLAYER((PlayerTypes)info.getData1()).getTeam(), true);
		}

		GET_PLAYER(GC.getGameINLINE().getActivePlayer()).setTributePaid((PlayerTypes)info.getData1(), true);

		break;

	default:
		FAssert(false);
		break;
	}
}

void CvDLLButtonPopup::OnFocus(CvPopup* pPopup, CvPopupInfo &info)
{
	if (gDLL->getInterfaceIFace()->popupIsDying(pPopup))
	{
		return;
	}

	switch (info.getButtonPopupType())
	{
	case BUTTONPOPUP_CHOOSETECH:
		if (info.getData1() == 0)
		{
			if ((GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCurrentResearch() != NO_TECH) || (GC.getGameINLINE().getGameState() == GAMESTATE_OVER))
			{
				gDLL->getInterfaceIFace()->popupSetAsCancelled(pPopup);
			}
		}
		break;

	case BUTTONPOPUP_CHANGERELIGION:
		if (!(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).canChangeReligion()) || (GC.getGameINLINE().getGameState() == GAMESTATE_OVER))
		{
			gDLL->getInterfaceIFace()->popupSetAsCancelled(pPopup);
		}
		break;

	case BUTTONPOPUP_CHOOSEPRODUCTION:
		if (GC.getGameINLINE().getGameState() == GAMESTATE_OVER)
		{
			gDLL->getInterfaceIFace()->popupSetAsCancelled(pPopup);
		}
		else
		{
			PlayerTypes ePlayer = GC.getGameINLINE().getActivePlayer();
			CvCity* pCity = GET_PLAYER(ePlayer).getCity(info.getData1());

			if (NULL == pCity || pCity->getOwnerINLINE() != ePlayer || pCity->isProduction())
			{
				gDLL->getInterfaceIFace()->popupSetAsCancelled(pPopup);
				break;
			}

			gDLL->getInterfaceIFace()->lookAtCityOffset(pCity->getID());
		}
		break;

	case BUTTONPOPUP_RAZECITY:
	case BUTTONPOPUP_DISBANDCITY:
		{
			PlayerTypes ePlayer = GC.getGameINLINE().getActivePlayer();
			CvCity* pCity = GET_PLAYER(ePlayer).getCity(info.getData1());

			if (NULL == pCity || pCity->getOwnerINLINE() != ePlayer)
			{
				gDLL->getInterfaceIFace()->popupSetAsCancelled(pPopup);
				break;
			}

			gDLL->getInterfaceIFace()->lookAtCityOffset(pCity->getID());
		}
		break;

	case BUTTONPOPUP_CHANGECIVIC:
		if (!(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).canRevolution(NULL)) || (GC.getGameINLINE().getGameState() == GAMESTATE_OVER))
		{
			gDLL->getInterfaceIFace()->popupSetAsCancelled(pPopup);
		}
		break;

	case BUTTONPOPUP_PYTHON:
	case BUTTONPOPUP_PYTHON_SCREEN:
		if (!info.getOnFocusPythonCallback().IsEmpty())
		{
			long iResult;
			CyArgsList argsList;
			argsList.add(info.getData1());
			argsList.add(info.getData2());
			argsList.add(info.getData3());
			argsList.add(info.getFlags());
			argsList.add(info.getText());
			argsList.add(info.getOption1());
			argsList.add(info.getOption2());
			gDLL->getPythonIFace()->callFunction(PYScreensModule, info.getOnFocusPythonCallback(), argsList.makeFunctionArgs(), &iResult);
			if (0 != iResult)
			{
				gDLL->getInterfaceIFace()->popupSetAsCancelled(pPopup);
			}
		}
		break;
	}
}

// returns false if popup is not launched
bool CvDLLButtonPopup::launchButtonPopup(CvPopup* pPopup, CvPopupInfo &info)
{
	bool bLaunched = false;

	switch (info.getButtonPopupType())
	{
	case BUTTONPOPUP_TEXT:
		bLaunched = launchTextPopup(pPopup, info);
		break;
	case BUTTONPOPUP_CHOOSEPRODUCTION:
		bLaunched = launchProductionPopup(pPopup, info);
		break;
	case BUTTONPOPUP_CHANGERELIGION:
		bLaunched = launchChangeReligionPopup(pPopup, info);
		break;
	case BUTTONPOPUP_CHOOSEELECTION:
		bLaunched = launchChooseElectionPopup(pPopup, info);
		break;
	case BUTTONPOPUP_DIPLOVOTE:
		bLaunched = launchDiploVotePopup(pPopup, info);
		break;
	case BUTTONPOPUP_DIPLOVOTE_RESULTS:
		bLaunched = launchDiploVoteResults(pPopup, info);
		break;
	case BUTTONPOPUP_RAZECITY:
		bLaunched = launchRazeCityPopup(pPopup, info);
		break;
	case BUTTONPOPUP_DISBANDCITY:
		bLaunched = launchDisbandCityPopup(pPopup, info);
		break;
	case BUTTONPOPUP_CHOOSETECH:
		bLaunched = launchChooseTechPopup(pPopup, info);
		break;
	case BUTTONPOPUP_CHANGECIVIC:
		bLaunched = launchChangeCivicsPopup(pPopup, info);
		break;
	case BUTTONPOPUP_ALARM:
		bLaunched = launchAlarmPopup(pPopup, info);
		break;
	case BUTTONPOPUP_DECLAREWARMOVE:
		bLaunched = launchDeclareWarMovePopup(pPopup, info);
		break;
	case BUTTONPOPUP_CONFIRMCOMMAND:
		bLaunched = launchConfirmCommandPopup(pPopup, info);
		break;
	case BUTTONPOPUP_LOADUNIT:
		bLaunched = launchLoadUnitPopup(pPopup, info);
		break;
	case BUTTONPOPUP_LEADUNIT:
		bLaunched = launchLeadUnitPopup(pPopup, info);
		break;
	case BUTTONPOPUP_MAIN_MENU:
		bLaunched = launchMainMenuPopup(pPopup, info);
		break;
	case BUTTONPOPUP_CONFIRM_MENU:
		bLaunched = launchConfirmMenu(pPopup, info);
		break;
	case BUTTONPOPUP_PYTHON_SCREEN:
		bLaunched = launchPythonScreen(pPopup, info);
		break;
	case BUTTONPOPUP_DEAL_CANCELED:
		bLaunched = launchCancelDeal(pPopup, info);
		break;
	case BUTTONPOPUP_PYTHON:
		bLaunched = launchPythonPopup(pPopup, info);
		break;
	case BUTTONPOPUP_DETAILS:
		bLaunched = launchDetailsPopup(pPopup, info);
		break;
	case BUTTONPOPUP_ADMIN:
		bLaunched = launchAdminPopup(pPopup, info);
		break;
	case BUTTONPOPUP_ADMIN_PASSWORD:
		bLaunched = launchAdminPasswordPopup(pPopup, info);
		break;
	case BUTTONPOPUP_EXTENDED_GAME:
		bLaunched = launchExtendedGamePopup(pPopup, info);
		break;
	case BUTTONPOPUP_DIPLOMACY:
		bLaunched = launchDiplomacyPopup(pPopup, info);
		break;
	case BUTTONPOPUP_ADDBUDDY:
		bLaunched = launchAddBuddyPopup(pPopup, info);
		break;
	case BUTTONPOPUP_FORCED_DISCONNECT:
		bLaunched = launchForcedDisconnectPopup(pPopup, info);
		break;
	case BUTTONPOPUP_PITBOSS_DISCONNECT:
		bLaunched = launchPitbossDisconnectPopup(pPopup, info);
		break;
	case BUTTONPOPUP_KICKED:
		bLaunched = launchKickedPopup(pPopup, info);
		break;
	case BUTTONPOPUP_VASSAL_DEMAND_TRIBUTE:
		bLaunched = launchVassalDemandTributePopup(pPopup, info);
		break;
	case BUTTONPOPUP_VASSAL_GRANT_TRIBUTE:
		bLaunched = launchVassalGrantTributePopup(pPopup, info);
		break;
	default:
		FAssert(false);
		break;
	}
	return (bLaunched);
}


bool CvDLLButtonPopup::launchTextPopup(CvPopup* pPopup, CvPopupInfo &info)
{
	gDLL->getInterfaceIFace()->popupSetBodyString(pPopup, info.getText());
	gDLL->getInterfaceIFace()->popupLaunch(pPopup, true, POPUPSTATE_IMMEDIATE);
	return (true);
}


bool CvDLLButtonPopup::launchProductionPopup(CvPopup* pPopup, CvPopupInfo &info)
{
	CvWString szBuffer;
	CvString szArtFilename; 
	CvWString szTemp;

	CvCity* pCity = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCity(info.getData1());
	if (NULL == pCity || pCity->isProductionAutomated())
	{
		return (false);
	}

	CyCity* pyCity = new CyCity(pCity);
	CyArgsList argsList;
	argsList.add(gDLL->getPythonIFace()->makePythonObject(pyCity));	// pass in plot class
	long lResult=0;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "skipProductionPopup", argsList.makeFunctionArgs(), &lResult);
	delete pyCity;	// python fxn must not hold on to this pointer 
	if (lResult == 1)
	{
		return (false);
	}

	FAssertMsg(pCity->getOwnerINLINE() == GC.getGameINLINE().getActivePlayer(), "City must belong to Active Player");	

	UnitTypes eTrainUnit = NO_UNIT;
	BuildingTypes eConstructBuilding = NO_BUILDING;
	ProjectTypes eCreateProject = NO_PROJECT;
	switch (info.getData2())
	{
	case (ORDER_TRAIN):
		eTrainUnit = (UnitTypes)info.getData3();
		break;
	case (ORDER_CONSTRUCT):
		eConstructBuilding = (BuildingTypes)info.getData3();
		break;
	case (ORDER_CREATE):
		eCreateProject = (ProjectTypes)info.getData3();
		break;
	default:
		break;
	}
	bool bFinish = info.getOption1();

	if (eTrainUnit != NO_UNIT)
	{
		if (bFinish)
		{
			szBuffer = gDLL->getText(((isLimitedUnitClass((UnitClassTypes)(GC.getUnitInfo(eTrainUnit).getUnitClassType()))) ? "TXT_KEY_POPUP_TRAINED_WORK_ON_NEXT_LIMITED" : "TXT_KEY_POPUP_TRAINED_WORK_ON_NEXT"), GC.getUnitInfo(eTrainUnit).getTextKeyWide(), pCity->getNameKey());
		}
		else
		{
			szBuffer = gDLL->getText(((isLimitedUnitClass((UnitClassTypes)(GC.getUnitInfo(eTrainUnit).getUnitClassType()))) ? "TXT_KEY_POPUP_CANNOT_TRAIN_WORK_NEXT_LIMITED" : "TXT_KEY_POPUP_CANNOT_TRAIN_WORK_NEXT"), GC.getUnitInfo(eTrainUnit).getTextKeyWide(), pCity->getNameKey());
		}
		szArtFilename = GC.getUnitInfo(eTrainUnit).getButton();
	}
	else if (eConstructBuilding != NO_BUILDING)
	{
		if (bFinish)
		{
			szBuffer = gDLL->getText(((isLimitedWonderClass((BuildingClassTypes)(GC.getBuildingInfo(eConstructBuilding).getBuildingClassType()))) ? "TXT_KEY_POPUP_CONSTRUCTED_WORK_ON_NEXT_LIMITED" : "TXT_KEY_POPUP_CONSTRUCTED_WORK_ON_NEXT"), GC.getBuildingInfo(eConstructBuilding).getTextKeyWide(), pCity->getNameKey());
		}
		else
		{
			szBuffer = gDLL->getText(((isLimitedWonderClass((BuildingClassTypes)(GC.getBuildingInfo(eConstructBuilding).getBuildingClassType()))) ? "TXT_KEY_POPUP_CANNOT_CONSTRUCT_WORK_NEXT_LIMITED" : "TXT_KEY_POPUP_CANNOT_CONSTRUCT_WORK_NEXT"), GC.getBuildingInfo(eConstructBuilding).getTextKeyWide(), pCity->getNameKey());
		}
		szArtFilename = GC.getBuildingInfo(eConstructBuilding).getButton();
	}
	else if (eCreateProject != NO_PROJECT)
	{
		if (bFinish)
		{
			szBuffer = gDLL->getText(((isLimitedProject(eCreateProject)) ? "TXT_KEY_POPUP_CREATED_WORK_ON_NEXT_LIMITED" : "TXT_KEY_POPUP_CREATED_WORK_ON_NEXT"), GC.getProjectInfo(eCreateProject).getTextKeyWide(), pCity->getNameKey());
		}
		else
		{
			szBuffer = gDLL->getText(((isLimitedProject(eCreateProject)) ? "TXT_KEY_POPUP_CANNOT_CREATE_WORK_NEXT_LIMITED" : "TXT_KEY_POPUP_CANNOT_CREATE_WORK_NEXT"), GC.getProjectInfo(eCreateProject).getTextKeyWide(), pCity->getNameKey());
		}
		szArtFilename = GC.getProjectInfo(eCreateProject).getButton();
	}
	else
	{
		szBuffer = gDLL->getText("TXT_KEY_POPUP_WHAT_TO_BUILD", pCity->getNameKey());
		szArtFilename = ARTFILEMGR.getInterfaceArtInfo("INTERFACE_POPUPBUTTON_PRODUCTION")->getPath();
	}

	gDLL->getInterfaceIFace()->popupSetHeaderString(pPopup, szBuffer, DLL_FONT_LEFT_JUSTIFY);

	pyCity = new CyCity(pCity);
	CyArgsList argsList2;
	argsList2.add(gDLL->getPythonIFace()->makePythonObject(pyCity));	// pass in plot class
	lResult=1;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "showExamineCityButton", argsList2.makeFunctionArgs(), &lResult);
	delete pyCity;	// python fxn must not hold on to this pointer 
	if (lResult == 1)
	{
		int iExamineCityID = 0;
		iExamineCityID = max(iExamineCityID, GC.getNumUnitInfos());
		iExamineCityID = max(iExamineCityID, GC.getNumBuildingInfos());
		iExamineCityID = max(iExamineCityID, GC.getNumProjectInfos());
		iExamineCityID = max(iExamineCityID, GC.getNumProcessInfos());

		gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_POPUP_EXAMINE_CITY").c_str(), ARTFILEMGR.getInterfaceArtInfo("INTERFACE_BUTTONS_CITYSELECTION")->getPath(), WIDGET_GENERAL, iExamineCityID, -1, true, POPUP_LAYOUT_STRETCH, DLL_FONT_LEFT_JUSTIFY);
	}

	UnitTypes eProductionUnit = pCity->getProductionUnit();
	BuildingTypes eProductionBuilding = pCity->getProductionBuilding();
	ProjectTypes eProductionProject = pCity->getProductionProject();
	ProcessTypes eProductionProcess = pCity->getProductionProcess();

	int iNumBuilds = 0;

	pyCity = new CyCity(pCity);
	CyArgsList argsList3;
	argsList3.add(gDLL->getPythonIFace()->makePythonObject(pyCity));	// pass in city class
	lResult=-1;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "getRecommendedUnit", argsList3.makeFunctionArgs(), &lResult);
	eProductionUnit = ((UnitTypes)lResult);
	CyArgsList argsList4; // XXX
	argsList4.add(gDLL->getPythonIFace()->makePythonObject(pyCity));	// pass in city class
	lResult=-1;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "getRecommendedBuilding", argsList4.makeFunctionArgs(), &lResult);
	eProductionBuilding = ((BuildingTypes)lResult);
	delete pyCity;	// python fxn must not hold on to this pointer 

	if (eProductionUnit == NO_UNIT)
	{
		eProductionUnit = pCity->AI_bestUnit(true, ((eProductionBuilding != NO_BUILDING) ? ((AdvisorTypes)(GC.getBuildingInfo(eProductionBuilding).getAdvisorType())) : NO_ADVISOR));
	}

	if (eProductionBuilding == NO_BUILDING)
	{
		eProductionBuilding = pCity->AI_bestBuilding(0, 50, true, ((eProductionUnit != NO_UNIT) ? ((AdvisorTypes)(GC.getUnitInfo(eProductionUnit).getAdvisorType())) : NO_ADVISOR));
	}

	if (eProductionUnit != NO_UNIT)
	{
		int iTurns = pCity->getProductionTurnsLeft(eProductionUnit, 0);
		szBuffer = gDLL->getText("TXT_KEY_POPUP_RECOMMENDED", GC.getUnitInfo(eProductionUnit).getTextKeyWide(), iTurns, GC.getAdvisorInfo((AdvisorTypes)(GC.getUnitInfo(eProductionUnit).getAdvisorType())).getTextKeyWide());
		gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, szBuffer, GC.getUnitInfo(eProductionUnit).getButton(), WIDGET_TRAIN, GC.getUnitInfo(eProductionUnit).getUnitClassType(), pCity->getID(), true, POPUP_LAYOUT_STRETCH, DLL_FONT_LEFT_JUSTIFY );
		iNumBuilds++;
	}

	if (eProductionBuilding != NO_BUILDING)
	{
		int iTurns = pCity->getProductionTurnsLeft(eProductionBuilding, 0);
		szBuffer = gDLL->getText("TXT_KEY_POPUP_RECOMMENDED", GC.getBuildingInfo(eProductionBuilding).getTextKeyWide(), iTurns, GC.getAdvisorInfo((AdvisorTypes)(GC.getBuildingInfo(eProductionBuilding).getAdvisorType())).getTextKeyWide());
		gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, szBuffer, GC.getBuildingInfo(eProductionBuilding).getButton(), WIDGET_CONSTRUCT, GC.getBuildingInfo(eProductionBuilding).getBuildingClassType(), pCity->getID(), true, POPUP_LAYOUT_STRETCH, DLL_FONT_LEFT_JUSTIFY );
		iNumBuilds++;
	}

	if (eProductionProject != NO_PROJECT)
	{
		int iTurns = pCity->getProductionTurnsLeft(eProductionProject, 0);
		szBuffer = gDLL->getText("TXT_KEY_POPUP_RECOMMENDED_NO_ADV", GC.getProjectInfo(eProductionProject).getTextKeyWide(), iTurns);
		gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, szBuffer, GC.getProjectInfo(eProductionProject).getButton(), WIDGET_CREATE, eProductionProject, pCity->getID(), true, POPUP_LAYOUT_STRETCH, DLL_FONT_LEFT_JUSTIFY );
		iNumBuilds++;
	}

	if (eProductionProcess != NO_PROCESS)
	{
		szBuffer = gDLL->getText("TXT_KEY_POPUP_RECOMMENDED_NO_ADV_OR_TURNS", GC.getProcessInfo(eProductionProcess).getTextKeyWide());
		gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, szBuffer, GC.getProcessInfo(eProductionProcess).getButton(), WIDGET_MAINTAIN, eProductionProcess, pCity->getID(), true, POPUP_LAYOUT_STRETCH, DLL_FONT_LEFT_JUSTIFY );
		iNumBuilds++;
	}

	for (int iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
	{
		UnitTypes eLoopUnit = (UnitTypes)GC.getCivilizationInfo(pCity->getCivilizationType()).getCivilizationUnits(iI);

		if (eLoopUnit != NO_UNIT)
		{
			if (eLoopUnit != eProductionUnit)
			{
				if (pCity->canTrain(eLoopUnit))
				{
					int iTurns = pCity->getProductionTurnsLeft(eLoopUnit, 0);
					szBuffer.Format(L"%s (%d)", GC.getUnitInfo(eLoopUnit).getDescription(), iTurns);
					gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, szBuffer, GC.getUnitInfo(eLoopUnit).getButton(), WIDGET_TRAIN, iI, pCity->getID(), true, POPUP_LAYOUT_STRETCH, DLL_FONT_LEFT_JUSTIFY );
					iNumBuilds++;
				}
			}
		}
	}
	for (int iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
	{
		BuildingTypes eLoopBuilding = ((BuildingTypes)(GC.getCivilizationInfo(pCity->getCivilizationType()).getCivilizationBuildings(iI)));

		if (eLoopBuilding != NO_BUILDING)
		{
			if (eLoopBuilding != eProductionBuilding)
			{
				if (pCity->canConstruct(eLoopBuilding))
				{
					int iTurns = pCity->getProductionTurnsLeft(eLoopBuilding, 0);
					szBuffer.Format(L"%s (%d)", GC.getBuildingInfo(eLoopBuilding).getDescription(), iTurns);
					gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, szBuffer, GC.getBuildingInfo(eLoopBuilding).getButton(), WIDGET_CONSTRUCT, iI, pCity->getID(), true, POPUP_LAYOUT_STRETCH, DLL_FONT_LEFT_JUSTIFY);
					iNumBuilds++;
				}
			}
		}
	}
	for (int iI = 0; iI < GC.getNumProjectInfos(); iI++)
	{
		if (iI != eProductionProject)
		{
			if (pCity->canCreate((ProjectTypes)iI))
			{
				int iTurns = pCity->getProductionTurnsLeft((ProjectTypes)iI, 0);
				szBuffer.Format(L"%s (%d)", GC.getProjectInfo((ProjectTypes) iI).getDescription(), iTurns);
				gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, szBuffer, GC.getProjectInfo((ProjectTypes) iI).getButton(), WIDGET_CREATE, iI, pCity->getID(), true, POPUP_LAYOUT_STRETCH, DLL_FONT_LEFT_JUSTIFY);
				iNumBuilds++;
			}
		}
	}
	for (int iI = 0; iI < GC.getNumProcessInfos(); iI++)
	{
		if (iI != eProductionProcess)
		{
			if (pCity->canMaintain((ProcessTypes)iI))
			{
				gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, GC.getProcessInfo((ProcessTypes) iI).getDescription(), GC.getProcessInfo((ProcessTypes) iI).getButton(), WIDGET_MAINTAIN, iI, pCity->getID(), true, POPUP_LAYOUT_STRETCH, DLL_FONT_LEFT_JUSTIFY);
				iNumBuilds++;
			}
		}
	}

	if (0 == iNumBuilds)
	{
		// city cannot build anything, so don't show popup after all
		return (false);
	}

	gDLL->getInterfaceIFace()->popupSetPopupType(pPopup, POPUPEVENT_PRODUCTION, szArtFilename);

	gDLL->getInterfaceIFace()->popupLaunch(pPopup, false, POPUPSTATE_MINIMIZED, 252);

	switch (info.getData2())
	{
	case ORDER_TRAIN:
		gDLL->getInterfaceIFace()->playGeneralSound(GC.getUnitInfo((UnitTypes)info.getData3()).getArtInfo(0, GET_PLAYER(pCity->getOwner()).getCurrentEra())->getTrainSound());
		break;

	case ORDER_CONSTRUCT:
		gDLL->getInterfaceIFace()->playGeneralSound(GC.getBuildingInfo((BuildingTypes)info.getData3()).getConstructSound());
		break;

	case ORDER_CREATE:
		gDLL->getInterfaceIFace()->playGeneralSound(GC.getProjectInfo((ProjectTypes)info.getData3()).getCreateSound());
		break;

	default:
		break;
	}

	return (true);
}


bool CvDLLButtonPopup::launchChangeReligionPopup(CvPopup* pPopup, CvPopupInfo &info)
{
	CvWString szTemp;
	ReligionTypes eReligion = (ReligionTypes)info.getData1();

	if (NO_RELIGION == eReligion)
	{
		FAssert(false);
		return (false);
	}

	CvPlayer& activePlayer = GET_PLAYER(GC.getGameINLINE().getActivePlayer());
	if (!activePlayer.canConvert(eReligion))
	{
		return (false);
	}

	CvWString szBuffer;
	szBuffer = gDLL->getText("TXT_KEY_POPUP_RELIGION_SPREAD", GC.getReligionInfo(eReligion).getTextKeyWide());
	if (activePlayer.getStateReligionHappiness() != 0)
	{
		if (activePlayer.getStateReligionHappiness() > 0)
		{
			szBuffer += gDLL->getText("TXT_KEY_POPUP_CONVERTING_EFFECTS", 
				activePlayer.getStateReligionHappiness(), gDLL->getSymbolID(HAPPY_CHAR), GC.getReligionInfo(eReligion).getChar());
		}
		else
		{
			szBuffer += gDLL->getText("TXT_KEY_POPUP_CONVERTING_EFFECTS", 
				-activePlayer.getStateReligionHappiness(), gDLL->getSymbolID(UNHAPPY_CHAR), GC.getReligionInfo(eReligion).getChar());
		}
	}
	szBuffer += gDLL->getText("TXT_KEY_POPUP_LIKE_TO_CONVERT");
	gDLL->getInterfaceIFace()->popupSetBodyString(pPopup, szBuffer);

	szBuffer = gDLL->getText("TXT_KEY_POPUP_CONVERT_RELIGION");
	int iAnarchyLength = activePlayer.getReligionAnarchyLength();
	if (iAnarchyLength > 0)
	{
		szBuffer += gDLL->getText("TXT_KEY_POPUP_TURNS_OF_ANARCHY", iAnarchyLength);
	}
	gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, szBuffer);
	gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_POPUP_NO_CONVERSION").c_str());
	gDLL->getInterfaceIFace()->popupSetPopupType(pPopup, POPUPEVENT_RELIGION, ARTFILEMGR.getInterfaceArtInfo("INTERFACE_POPUPBUTTON_RELIGION")->getPath());
	gDLL->getInterfaceIFace()->popupLaunch(pPopup, false, POPUPSTATE_MINIMIZED);
	return (true);
}


bool CvDLLButtonPopup::launchChooseElectionPopup(CvPopup* pPopup, CvPopupInfo &info)
{
	CvWString szBuffer;
	gDLL->getInterfaceIFace()->popupSetBodyString(pPopup, gDLL->getText("TXT_KEY_POPUP_CHOOSE_ELECTION", GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getTotalPopulation()));
	for (int iI = 0; iI < GC.getNumVoteInfos(); iI++)
	{
		if (GC.getGameINLINE().isChooseElection((VoteTypes)iI))
		{
			szBuffer = gDLL->getText("TXT_KEY_POPUP_ELECTION_OPTION", GC.getVoteInfo((VoteTypes)iI).getTextKeyWide(), GC.getGameINLINE().getVoteRequired((VoteTypes)iI), GC.getGameINLINE().countPossibleVote());
			if (GC.getGameINLINE().isVotePassed((VoteTypes)iI))
			{
				szBuffer += gDLL->getText("TXT_KEY_POPUP_PASSED");
			}
			gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, szBuffer, NULL, WIDGET_GENERAL, iI);
		}
	}
	gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_NONE").c_str(), NULL, WIDGET_GENERAL, GC.getNumVoteInfos());
	gDLL->getInterfaceIFace()->popupLaunch(pPopup, false, POPUPSTATE_IMMEDIATE);
	return (true);
}


bool CvDLLButtonPopup::launchDiploVotePopup(CvPopup* pPopup, CvPopupInfo &info)
{
	VoteTypes eVote = (VoteTypes)info.getData1();

	if (NO_VOTE == eVote)
	{
		FAssert(false);
		return (false);
	}

	TeamTypes eVassalOfTeam = NO_TEAM;
	bool bEligible = false;

	gDLL->getInterfaceIFace()->popupSetBodyString(pPopup, gDLL->getText("TXT_KEY_POPUP_DIPLO_VOTE", GC.getVoteInfo(eVote).getTextKeyWide(), GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getTotalPopulation(), GC.getGameINLINE().getVoteRequired(eVote), GC.getGameINLINE().countPossibleVote()));
	if (GC.getGameINLINE().isTeamVote(eVote))
	{
		for (int iI = 0; iI < MAX_CIV_TEAMS; iI++)
		{
			if (GET_TEAM((TeamTypes)iI).isAlive())
			{
				if (GC.getGameINLINE().isTeamVoteEligible((TeamTypes)iI))
				{
					if (GET_TEAM(GC.getGameINLINE().getActiveTeam()).isVassal((TeamTypes)iI))
					{
						eVassalOfTeam = (TeamTypes)iI;
						break;
					}
				}
			}
		}

		for (int iI = 0; iI < MAX_CIV_TEAMS; iI++)
		{
			if (GET_TEAM((TeamTypes)iI).isAlive())
			{
				if (GC.getGameINLINE().isTeamVoteEligible((TeamTypes)iI))
				{
					if (eVassalOfTeam == NO_TEAM || eVassalOfTeam == iI || iI == GC.getGameINLINE().getActiveTeam())
					{
						gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, GET_TEAM((TeamTypes)iI).getName().GetCString(), NULL, WIDGET_GENERAL, iI);
						bEligible = true;
					}
				}
			}
		}
	}
	else
	{
		gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_POPUP_YES").c_str(), NULL, WIDGET_GENERAL, 1);
		gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_POPUP_NO").c_str(), NULL, WIDGET_GENERAL, 0);
		bEligible = true;
	}

	if (eVassalOfTeam == NO_TEAM || !bEligible)
	{
		gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_POPUP_ABSTAIN").c_str(), NULL, WIDGET_GENERAL, MAX_CIV_TEAMS);
	}

	gDLL->getInterfaceIFace()->popupLaunch(pPopup, false, POPUPSTATE_IMMEDIATE);
	return (true);
}


bool CvDLLButtonPopup::launchDiploVoteResults(CvPopup* pPopup, CvPopupInfo &info)
{
	VoteTypes eVote = (VoteTypes)info.getData1();

	if (NO_VOTE == eVote)
	{
		FAssert(false);
		return (false);
	}

	CvWString szBuffer;
	if (GC.getGameINLINE().isTeamVote(eVote))
	{
		szBuffer = GC.getVoteInfo(eVote).getDescription();

		TeamTypes eTeam = GC.getGameINLINE().findHighestVoteTeam(eVote);
		if (eTeam != NO_TEAM)
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_POPUP_DIPLOMATIC_VOTING_VICTORY", GET_TEAM(eTeam).getName().GetCString(), GC.getGameINLINE().countVote(eVote, eTeam), GC.getGameINLINE().getVoteRequired(eVote), GC.getGameINLINE().countPossibleVote());
		}
	}
	else
	{
		szBuffer += gDLL->getText(((GC.getGameINLINE().getVoteOutcome(eVote) == 1) ? "TXT_KEY_POPUP_DIPLOMATIC_VOTING_SUCCEEDS" : "TXT_KEY_POPUP_DIPLOMATIC_VOTING_FAILURE"), GC.getVoteInfo(eVote).getTextKeyWide(), GC.getGameINLINE().countVote(eVote, 1), GC.getGameINLINE().getVoteRequired(eVote), GC.getGameINLINE().countPossibleVote());
	}
	if (GC.getGameINLINE().isTeamVote(eVote))
	{
		for (int iI = (MAX_CIV_TEAMS - 1); iI >= -1; iI--)
		{
			for (int iJ = 0; iJ < MAX_CIV_PLAYERS; iJ++)
			{
				if (GET_PLAYER((PlayerTypes)iJ).isAlive())
				{
					if (GC.getGameINLINE().getPlayerVote(((PlayerTypes)iJ), eVote) == iI)
					{
						if (iI != -1)
						{
							szBuffer += NEWLINE + gDLL->getText("TXT_KEY_POPUP_VOTES_FOR", GET_PLAYER((PlayerTypes)iJ).getNameKey(), GET_TEAM((TeamTypes)(GC.getGameINLINE().getPlayerVote(((PlayerTypes)iJ), eVote))).getName().GetCString(), GET_PLAYER((PlayerTypes)iJ).getTotalPopulation());
						}
						else
						{
							szBuffer += NEWLINE + gDLL->getText("TXT_KEY_POPUP_ABSTAINS", GET_PLAYER((PlayerTypes)iJ).getNameKey(), GET_PLAYER((PlayerTypes)iJ).getTotalPopulation());
						}
					}
				}
			}
		}
	}
	else
	{
		for (int iI = 1; iI >= -1; iI--)
		{
			for (int iJ = 0; iJ < MAX_CIV_PLAYERS; iJ++)
			{
				if (GET_PLAYER((PlayerTypes)iJ).isAlive())
				{
					if (GC.getGameINLINE().getPlayerVote(((PlayerTypes)iJ), eVote) == iI)
					{
						if (iI != -1)
						{
							szBuffer += NEWLINE + gDLL->getText("TXT_KEY_POPUP_VOTES_YES_NO", GET_PLAYER((PlayerTypes)iJ).getNameKey(), ((GC.getGameINLINE().getPlayerVote(((PlayerTypes)iJ), eVote) == 1) ? L"TXT_KEY_POPUP_YES" : L"TXT_KEY_POPUP_NO"), GET_PLAYER((PlayerTypes)iJ).getTotalPopulation());
						}
						else
						{
							szBuffer += NEWLINE + gDLL->getText("TXT_KEY_POPUP_ABSTAINS", GET_PLAYER((PlayerTypes)iJ).getNameKey(), GET_PLAYER((PlayerTypes)iJ).getTotalPopulation());
						}
					}
				}
			}
		}
	}
	gDLL->getInterfaceIFace()->popupSetBodyString(pPopup, szBuffer);
	gDLL->getInterfaceIFace()->popupLaunch(pPopup);

	return (true);
}

bool CvDLLButtonPopup::launchRazeCityPopup(CvPopup* pPopup, CvPopupInfo &info)
{
	CvPlayer& player = GET_PLAYER(GC.getGameINLINE().getActivePlayer());

	CvCity* pNewCity = player.getCity(info.getData1());
	if (NULL == pNewCity)
	{
		FAssert(false);
		return (false);
	}

	if (0 != GC.getDefineINT("PLAYER_ALWAYS_RAZES_CITIES"))
	{
		player.raze(pNewCity);
		return false;
	}

	PlayerTypes eHighestCulturePlayer = (PlayerTypes)info.getData2();

	int iCaptureGold = info.getData3();
	bool bRaze = player.canRaze(pNewCity);
	bool bGift = ((eHighestCulturePlayer != NO_PLAYER) 
		&& (eHighestCulturePlayer != player.getID()) 
		&& ((player.getTeam() == GET_PLAYER(eHighestCulturePlayer).getTeam()) || GET_TEAM(player.getTeam()).isOpenBorders(GET_PLAYER(eHighestCulturePlayer).getTeam()) || GET_TEAM(GET_PLAYER(eHighestCulturePlayer).getTeam()).isVassal(player.getTeam())));
	
	CvWString szBuffer;
	if (iCaptureGold > 0)
	{
		szBuffer = gDLL->getText("TXT_KEY_POPUP_GOLD_CITY_CAPTURE", iCaptureGold, pNewCity->getNameKey());
	}
	else
	{
		szBuffer = gDLL->getText("TXT_KEY_POPUP_CITY_CAPTURE_KEEP", pNewCity->getNameKey());
	}
	gDLL->getInterfaceIFace()->popupSetBodyString(pPopup, szBuffer);
	gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_POPUP_KEEP_CAPTURED_CITY").c_str(), NULL, WIDGET_GENERAL, 0);

	if (bRaze)
	{
		gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_POPUP_RAZE_CAPTURED_CITY").c_str(), NULL, WIDGET_GENERAL, 1);
	}
	if (bGift)
	{
		szBuffer = gDLL->getText("TXT_KEY_POPUP_RETURN_ALLIED_CITY", GET_PLAYER(eHighestCulturePlayer).getCivilizationDescriptionKey());
		gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, szBuffer, NULL, WIDGET_GENERAL, 2, eHighestCulturePlayer);
	}
	gDLL->getInterfaceIFace()->popupLaunch(pPopup, false, POPUPSTATE_IMMEDIATE);

	gDLL->getInterfaceIFace()->playGeneralSound("AS2D_CITYCAPTURE");

	return (true);
}

bool CvDLLButtonPopup::launchDisbandCityPopup(CvPopup* pPopup, CvPopupInfo &info)
{
	CvPlayer& player = GET_PLAYER(GC.getGameINLINE().getActivePlayer());

	CvCity* pNewCity = player.getCity(info.getData1());
	if (NULL == pNewCity)
	{
		FAssert(false);
		return (false);
	}

	CvWString szBuffer;
	szBuffer = gDLL->getText("TXT_KEY_POPUP_FLIPPED_CITY_KEEP", pNewCity->getNameKey());
	gDLL->getInterfaceIFace()->popupSetBodyString(pPopup, szBuffer);
	gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_POPUP_KEEP_FLIPPED_CITY").c_str(), NULL, WIDGET_GENERAL, 0);
	gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_POPUP_DISBAND_FLIPPED_CITY").c_str(), NULL, WIDGET_GENERAL, 1);
	gDLL->getInterfaceIFace()->popupLaunch(pPopup, false, POPUPSTATE_IMMEDIATE);

	gDLL->getInterfaceIFace()->playGeneralSound("AS2D_CULTUREFLIP");

	return (true);
}

bool CvDLLButtonPopup::launchChooseTechPopup(CvPopup* pPopup, CvPopupInfo &info)
{
	CyArgsList argsList;
	argsList.add(GC.getGameINLINE().getActivePlayer());
	long lResult=0;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "skipResearchPopup", argsList.makeFunctionArgs(), &lResult);
	if (lResult == 1)
	{
		return false;
	}

	CvPlayer& player = GET_PLAYER(GC.getGameINLINE().getActivePlayer());

	int iDiscover = info.getData1();
	CvWString szHeader = info.getText();
	if (szHeader.empty())
	{
		szHeader = (iDiscover > 0) ? gDLL->getText("TXT_KEY_POPUP_CHOOSE_TECH").c_str() : gDLL->getText("TXT_KEY_POPUP_RESEARCH_NEXT").c_str();
	}
	gDLL->getInterfaceIFace()->popupSetHeaderString(pPopup, szHeader, DLL_FONT_LEFT_JUSTIFY);

	if (iDiscover == 0)
	{
		lResult=1;
		gDLL->getPythonIFace()->callFunction(PYGameModule, "showTechChooserButton", argsList.makeFunctionArgs(), &lResult);
		if (lResult == 1)
		{
			// Allow user to Jump to the Tech Chooser
			gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_POPUP_SEE_BIG_PICTURE").c_str(), ARTFILEMGR.getInterfaceArtInfo("INTERFACE_POPUPBUTTON_TECH")->getPath(), WIDGET_GENERAL, GC.getNumTechInfos(), MAX_INT, true, POPUP_LAYOUT_STRETCH, DLL_FONT_LEFT_JUSTIFY);
			// Note: This button is NOT supposed to close the popup!! 
		}
	}

	TechTypes eBestTech = NO_TECH;
	TechTypes eNextBestTech = NO_TECH;

	lResult = -1;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "getFirstRecommendedTech", argsList.makeFunctionArgs(), &lResult);
	eBestTech = ((TechTypes)lResult);

	if (eBestTech == NO_TECH)
	{
		eBestTech = player.AI_bestTech(1, (iDiscover > 0), true);
	}

	if (eBestTech != NO_TECH)
	{
		argsList.add(eBestTech);
		lResult = -1;
		gDLL->getPythonIFace()->callFunction(PYGameModule, "getSecondRecommendedTech", argsList.makeFunctionArgs(), &lResult);
		eNextBestTech = ((TechTypes)lResult);

		if (eNextBestTech == NO_TECH)
		{
			eNextBestTech = player.AI_bestTech(1, (iDiscover > 0), true, eBestTech, ((AdvisorTypes)(GC.getTechInfo(eBestTech).getAdvisorType())));
		}
	}

	int iNumTechs = 0;
	for (int iPass = 0; iPass < 2; iPass++)
	{
		for (int iI = 0; iI < GC.getNumTechInfos(); iI++)
		{
			if (((iI == eBestTech) || (iI == eNextBestTech)) == (iPass == 0))
			{
				if (player.canResearch((TechTypes)iI))
				{
					CvWString szBuffer;
					szBuffer.Format(L"%s (%d)", GC.getTechInfo((TechTypes)iI).getDescription(), ((iDiscover > 0) ? 0 : player.getResearchTurnsLeft(((TechTypes)iI), true)));

					if ((iI == eBestTech) || (iI == eNextBestTech))
					{
						szBuffer += gDLL->getText("TXT_KEY_POPUP_RECOMMENDED_ONLY_ADV", GC.getAdvisorInfo((AdvisorTypes)(GC.getTechInfo((TechTypes)iI).getAdvisorType())).getTextKeyWide());
					}

					CvString szButton = GC.getTechInfo((TechTypes) iI).getButton();

					for (int iJ = 0; iJ < GC.getNumReligionInfos(); iJ++)
					{
						if (GC.getReligionInfo((ReligionTypes)iJ).getTechPrereq() == iI)
						{
							if (!(GC.getGameINLINE().isReligionFounded((ReligionTypes)iJ)))
							{
								szButton = GC.getReligionInfo((ReligionTypes) iJ).getTechButton();
								break;
							}
						}
					}

					gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, szBuffer, szButton, WIDGET_RESEARCH, iI, iDiscover, true, POPUP_LAYOUT_STRETCH, DLL_FONT_LEFT_JUSTIFY);
					iNumTechs++;
				}
			}
		}
	}
	if (0 == iNumTechs)
	{
		// player cannot research anything, so don't show this popup after all
		return (false);
	}

	gDLL->getInterfaceIFace()->popupSetPopupType(pPopup, POPUPEVENT_TECHNOLOGY, ARTFILEMGR.getInterfaceArtInfo("INTERFACE_POPUPBUTTON_TECH")->getPath());

	gDLL->getInterfaceIFace()->popupLaunch(pPopup, false, ((iDiscover > 0) ? POPUPSTATE_QUEUED : POPUPSTATE_MINIMIZED));

	return (true);
}

bool CvDLLButtonPopup::launchChangeCivicsPopup(CvPopup* pPopup, CvPopupInfo &info)
{
	CivicTypes* paeNewCivics = new CivicTypes[GC.getNumCivicOptionInfos()];
	if (NULL == paeNewCivics)
	{
		return (false);
	}

	CivicOptionTypes eCivicOptionType = (CivicOptionTypes)info.getData1();
	CivicTypes eCivicType = (CivicTypes)info.getData2();
	bool bValid = false;

	if (eCivicType != NO_CIVIC)
	{
		for (int iI = 0; iI < GC.getNumCivicOptionInfos(); iI++)
		{
			if (iI == eCivicOptionType)
			{
				paeNewCivics[iI] = eCivicType;
			}
			else
			{
				paeNewCivics[iI] = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCivics((CivicOptionTypes)iI);
			}
		}

		if (GET_PLAYER(GC.getGameINLINE().getActivePlayer()).canRevolution(paeNewCivics))
		{
			bValid = true;
		}
	}
	else
	{
		bValid = true;
	}

	if (bValid)
	{
		CvWString szBuffer;
		if (eCivicType != NO_CIVIC)
		{
			szBuffer = gDLL->getText("TXT_KEY_POPUP_NEW_CIVIC", GC.getCivicInfo(eCivicType).getTextKeyWide());
			if (!isEmpty(GC.getCivicInfo(eCivicType).getStrategy()))
			{
				CvWString szTemp;
				szTemp.Format(L" (%s)", GC.getCivicInfo(eCivicType).getStrategy());
				szBuffer += szTemp;
			}
			szBuffer += gDLL->getText("TXT_KEY_POPUP_START_REVOLUTION");
			gDLL->getInterfaceIFace()->popupSetBodyString(pPopup, szBuffer);

			szBuffer = gDLL->getText("TXT_KEY_POPUP_YES_START_REVOLUTION");
			int iAnarchyLength = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCivicAnarchyLength(paeNewCivics);
			if (iAnarchyLength > 0)
			{
				szBuffer += gDLL->getText("TXT_KEY_POPUP_TURNS_OF_ANARCHY", iAnarchyLength);
			}
			gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, szBuffer, NULL, WIDGET_GENERAL, 0);
		}
		else
		{
			gDLL->getInterfaceIFace()->popupSetBodyString(pPopup, gDLL->getText("TXT_KEY_POPUP_FIRST_REVOLUTION"));
		}

		gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_POPUP_OLD_WAYS_BEST").c_str(), NULL, WIDGET_GENERAL, 1);
		gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_POPUP_SEE_BIG_PICTURE").c_str(), NULL, WIDGET_GENERAL, 2);
		gDLL->getInterfaceIFace()->popupSetPopupType(pPopup, POPUPEVENT_CIVIC, ARTFILEMGR.getInterfaceArtInfo("INTERFACE_POPUPBUTTON_CIVICS")->getPath());
		gDLL->getInterfaceIFace()->popupLaunch(pPopup, false, POPUPSTATE_MINIMIZED);
	}

	SAFE_DELETE(paeNewCivics);

	return (bValid);
}


bool CvDLLButtonPopup::launchAlarmPopup(CvPopup* pPopup, CvPopupInfo &info)
{
	gDLL->getInterfaceIFace()->playGeneralSound("AS2D_ALARM");

	gDLL->getInterfaceIFace()->popupSetHeaderString(pPopup, gDLL->getText("TXT_KEY_POPUP_ALARM_TITLE").c_str());
	gDLL->getInterfaceIFace()->popupSetBodyString(pPopup, info.getText());
	gDLL->getInterfaceIFace()->popupLaunch(pPopup, true, POPUPSTATE_IMMEDIATE);

	return (true);
}


bool CvDLLButtonPopup::launchDeclareWarMovePopup(CvPopup* pPopup, CvPopupInfo &info)
{
	TeamTypes eRivalTeam = (TeamTypes)info.getData1();
	int iX = info.getData2();
	int iY = info.getData3();

	FAssert(eRivalTeam != NO_TEAM);

	CvPlot* pPlot = GC.getMapINLINE().plotINLINE(iX, iY);

	CvWString szBuffer;
	if ((pPlot != NULL) && (pPlot->getTeam() == eRivalTeam))
	{
		szBuffer = gDLL->getText("TXT_KEY_POPUP_ENTER_LANDS_WAR", GET_PLAYER(pPlot->getOwnerINLINE()).getCivilizationAdjective());

		if (GET_TEAM(GC.getGameINLINE().getActiveTeam()).isOpenBordersTrading())
		{
			szBuffer += gDLL->getText("TXT_KEY_POPUP_ENTER_WITH_OPEN_BORDERS");
		}
	}
	else
	{
		szBuffer = gDLL->getText("TXT_KEY_POPUP_DOES_THIS_MEAN_WAR", GET_TEAM(eRivalTeam).getName().GetCString());
	}
	gDLL->getInterfaceIFace()->popupSetBodyString(pPopup, szBuffer);
	gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_DECLARE_WAR_YES").c_str());
	gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_DECLARE_WAR_NO").c_str());
	gDLL->getInterfaceIFace()->popupLaunch(pPopup, false, POPUPSTATE_IMMEDIATE);

	return (true);
}


bool CvDLLButtonPopup::launchConfirmCommandPopup(CvPopup* pPopup, CvPopupInfo &info)
{
	int iAction = info.getData1();
	CvWString szBuffer;
	szBuffer = gDLL->getText("TXT_KEY_POPUP_ARE_YOU_SURE_ACTION", GC.getActionInfo(iAction).getTextKeyWide());
	gDLL->getInterfaceIFace()->popupSetBodyString(pPopup, szBuffer);
	gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_POPUP_YES").c_str());
	gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_POPUP_NO").c_str());
	gDLL->getInterfaceIFace()->popupLaunch(pPopup, false, POPUPSTATE_IMMEDIATE);

	return (true);
}


bool CvDLLButtonPopup::launchLoadUnitPopup(CvPopup* pPopup, CvPopupInfo &info)
{
	CLLNode<IDInfo>* pUnitNode;
	CvSelectionGroup* pSelectionGroup;
	CvUnit* pLoopUnit;
	CvPlot* pPlot;
	CvWString szBuffer;
	int iCount;
	CvUnit* pFirstUnit = NULL;

	pSelectionGroup = gDLL->getInterfaceIFace()->getSelectionList();

	if (NULL == pSelectionGroup)
	{
		return (false);
	}

	gDLL->getInterfaceIFace()->popupSetBodyString(pPopup, gDLL->getText("TXT_KEY_CHOOSE_TRANSPORT"));

	pPlot = pSelectionGroup->plot();

	iCount = 1;

	pUnitNode = pPlot->headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pPlot->nextUnitNode(pUnitNode);

		if (pSelectionGroup->canDoCommand(COMMAND_LOAD_UNIT, pLoopUnit->getOwnerINLINE(), pLoopUnit->getID()))
		{
			if (!pFirstUnit)
			{
				pFirstUnit = pLoopUnit;
			}
			szBuffer.clear();
			GAMETEXT.setUnitHelp(szBuffer, pLoopUnit, true);
			szBuffer += L", " + gDLL->getText("TXT_KEY_UNIT_HELP_CARGO_SPACE", pLoopUnit->getCargo(), pLoopUnit->cargoSpace());
			gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, szBuffer, NULL, WIDGET_GENERAL, iCount);
			iCount++;
		}
	}

	if (iCount <= 2)
	{
		if (pFirstUnit)
		{
			GC.getGameINLINE().selectionListGameNetMessage(GAMEMESSAGE_DO_COMMAND, COMMAND_LOAD_UNIT, pFirstUnit->getOwnerINLINE(), pFirstUnit->getID());
		}
		return (false);
	}

	gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_NEVER_MIND"), NULL, WIDGET_GENERAL, 0);

	gDLL->getInterfaceIFace()->popupLaunch(pPopup, false, POPUPSTATE_IMMEDIATE);

	return (true);
}


bool CvDLLButtonPopup::launchLeadUnitPopup(CvPopup* pPopup, CvPopupInfo &info)
{
	CLLNode<IDInfo>* pUnitNode;
	CvSelectionGroup* pSelectionGroup;
	CvUnit* pLoopUnit;
	CvPlot* pPlot;
	CvWString szBuffer;
	int iCount;
	CvUnit* pFirstUnit = NULL;

	pSelectionGroup = gDLL->getInterfaceIFace()->getSelectionList();

	if (NULL == pSelectionGroup)
	{
		return (false);
	}

	pPlot = pSelectionGroup->plot();
	if (NULL == pPlot)
	{
		return (false);
	}

	gDLL->getInterfaceIFace()->popupSetBodyString(pPopup, gDLL->getText("TXT_KEY_CHOOSE_UNIT_TO_LEAD"));


	iCount = 1;

	pUnitNode = pPlot->headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pPlot->nextUnitNode(pUnitNode);

		if (pLoopUnit->canPromote((PromotionTypes) info.getData1(), info.getData2()))
		{
			if (!pFirstUnit)
			{
				pFirstUnit = pLoopUnit;
			}
			szBuffer.clear();
			GAMETEXT.setUnitHelp(szBuffer, pLoopUnit, true);
			gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, szBuffer, NULL, WIDGET_GENERAL, iCount);
			iCount++;
		}
	}

	if (iCount <= 2)
	{
		if (pFirstUnit)
		{
			GC.getGameINLINE().selectionListGameNetMessage(GAMEMESSAGE_PUSH_MISSION, MISSION_LEAD, pFirstUnit->getID());
		}
		return (false);
	}

	gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_NEVER_MIND"), NULL, WIDGET_GENERAL, 0);

	gDLL->getInterfaceIFace()->popupLaunch(pPopup, false, POPUPSTATE_IMMEDIATE);

	return (true);
}


bool CvDLLButtonPopup::launchMainMenuPopup(CvPopup* pPopup, CvPopupInfo &info)
{
	gDLL->getInterfaceIFace()->popupSetStyle( pPopup, "Window_NoTitleBar_Style" );

	// 288,72
	gDLL->getInterfaceIFace()->popupAddDDS(pPopup, "resource/temp/civ4_title_small.dds", 192, 48);

	gDLL->getInterfaceIFace()->popupAddSeparator(pPopup);

	gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_POPUP_EXIT_TO_DESKTOP").c_str(), NULL, WIDGET_GENERAL, 0, 0, true, POPUP_LAYOUT_STRETCH, DLL_FONT_CENTER_JUSTIFY);

	// commenting out since you can't exit to main menu and then restart a game
	gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_POPUP_EXIT_TO_MAIN_MENU").c_str(), NULL, WIDGET_GENERAL, 1, 0, true, POPUP_LAYOUT_STRETCH, DLL_FONT_CENTER_JUSTIFY);

	if (GC.getGameINLINE().canDoControl(CONTROL_RETIRE))
	{
		gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_POPUP_RETIRE").c_str(), NULL, WIDGET_GENERAL, 2, 0, true, POPUP_LAYOUT_STRETCH, DLL_FONT_CENTER_JUSTIFY);
	}

	if ((GC.getGameINLINE().getGameTurn() == 0) && !(GC.getGameINLINE().isGameMultiPlayer()) && !(GC.getInitCore().getWBMapScript()))
	{
		// Don't allow if there has already been diplomacy
		bool bShow = true;
		for (int i = 0; bShow && i < MAX_CIV_TEAMS; i++)
		{
			for (int j = i+1; bShow && j < MAX_CIV_TEAMS; j++)
			{
				if (GET_TEAM((TeamTypes)i).isHasMet((TeamTypes)j))
				{
					bShow = false;
				}
			}
		}

		if (bShow)
		{
			gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_POPUP_REGENERATE_MAP").c_str(), NULL, WIDGET_GENERAL, 3, 0, true, POPUP_LAYOUT_STRETCH, DLL_FONT_CENTER_JUSTIFY);
		}
	}

	if (GC.getGameINLINE().canDoControl(CONTROL_LOAD_GAME))
	{
		gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_POPUP_LOAD_GAME").c_str(), NULL, WIDGET_GENERAL, 4, 0, true, POPUP_LAYOUT_STRETCH, DLL_FONT_CENTER_JUSTIFY);
	}
	if (GC.getGameINLINE().canDoControl(CONTROL_SAVE_NORMAL))
	{
		gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_POPUP_SAVE_GAME").c_str(), NULL, WIDGET_GENERAL, 5, 0, true, POPUP_LAYOUT_STRETCH, DLL_FONT_CENTER_JUSTIFY);
	}
	gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_POPUP_OPTIONS").c_str(), NULL, WIDGET_GENERAL, 6, 0, true, POPUP_LAYOUT_STRETCH, DLL_FONT_CENTER_JUSTIFY);

	if (GC.getGameINLINE().canDoControl(CONTROL_WORLD_BUILDER))
	{
		gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_POPUP_ENTER_WB").c_str(), NULL, WIDGET_GENERAL, 7, 0, true, POPUP_LAYOUT_STRETCH, DLL_FONT_CENTER_JUSTIFY);
	}

	if (GC.getGameINLINE().canDoControl(CONTROL_ADMIN_DETAILS))
	{
		gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_POPUP_GAME_DETAILS").c_str(), NULL, WIDGET_GENERAL, 8, 0, true, POPUP_LAYOUT_STRETCH, DLL_FONT_CENTER_JUSTIFY);
	}

	if (GC.getGameINLINE().canDoControl(CONTROL_DETAILS))
	{
		gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_POPUP_DETAILS_TITLE").c_str(), NULL, WIDGET_GENERAL, 9, 0, true, POPUP_LAYOUT_STRETCH, DLL_FONT_CENTER_JUSTIFY);
	}

	gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_POPUP_CANCEL").c_str(), NULL, WIDGET_GENERAL, 10, 0, true, POPUP_LAYOUT_STRETCH, DLL_FONT_CENTER_JUSTIFY);
	gDLL->getInterfaceIFace()->popupLaunch(pPopup, false, POPUPSTATE_IMMEDIATE);

	return (true);
}

bool CvDLLButtonPopup::launchConfirmMenu(CvPopup *pPopup, CvPopupInfo &info)
{
	gDLL->getInterfaceIFace()->popupSetBodyString(pPopup, gDLL->getText("TXT_KEY_POPUP_ARE_YOU_SURE").c_str());
	gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_POPUP_YES").c_str(), NULL, WIDGET_GENERAL, 0);
	gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_POPUP_NO").c_str(), NULL, WIDGET_GENERAL, 1);
	gDLL->getInterfaceIFace()->popupLaunch(pPopup, false, POPUPSTATE_IMMEDIATE);

	return true;
}

bool CvDLLButtonPopup::launchPythonScreen(CvPopup* pPopup, CvPopupInfo &info)
{
	// this is not really a popup, but a Python screen
	// we trick the app into thinking that it's a popup so that we can take advantage of the popup queuing system 

	CyArgsList argsList;
	argsList.add(info.getData1());
	argsList.add(info.getData2());
	argsList.add(info.getData3());
	argsList.add(info.getOption1());
	argsList.add(info.getOption2());
	gDLL->getPythonIFace()->callFunction(PYScreensModule, CvString(info.getText()).GetCString(), argsList.makeFunctionArgs());

	return (false); // return false, so the Popup object is deleted, since it's just a dummy
}

bool CvDLLButtonPopup::launchCancelDeal(CvPopup* pPopup, CvPopupInfo &info)
{
	gDLL->getInterfaceIFace()->popupSetBodyString(pPopup,  gDLL->getText("TXT_KEY_POPUP_CANCEL_DEAL") );

	gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_POPUP_CANCEL_DEAL_YES"), NULL, WIDGET_GENERAL, 0);
	gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_POPUP_CANCEL_DEAL_NO"), NULL, WIDGET_GENERAL, 1);

	gDLL->getInterfaceIFace()->popupLaunch(pPopup, false, POPUPSTATE_IMMEDIATE);

	return true;
}

bool CvDLLButtonPopup::launchPythonPopup(CvPopup* pPopup, CvPopupInfo &info)
{
	gDLL->getInterfaceIFace()->popupSetBodyString(pPopup, info.getText());
	for (int i = 0; i < info.getNumPythonButtons(); i++)
	{
		gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, info.getPythonButtonText(i), info.getPythonButtonArt(i).IsEmpty() ? NULL : info.getPythonButtonArt(i).GetCString());
	}

	gDLL->getInterfaceIFace()->popupSetPopupType(pPopup, POPUPEVENT_WARNING, ARTFILEMGR.getInterfaceArtInfo("INTERFACE_POPUPBUTTON_WARNING")->getPath());
	gDLL->getInterfaceIFace()->popupLaunch(pPopup, false, POPUPSTATE_IMMEDIATE);

	return (true);
}

bool CvDLLButtonPopup::launchDetailsPopup(CvPopup* pPopup, CvPopupInfo &info)
{
	if (!info.getOption1())
	{
		gDLL->getInterfaceIFace()->popupSetHeaderString(pPopup, gDLL->getText("TXT_KEY_POPUP_DETAILS_TITLE"));

		gDLL->getInterfaceIFace()->popupSetBodyString(pPopup, gDLL->getText("TXT_KEY_MENU_LEADER_NAME"));
		gDLL->getInterfaceIFace()->popupCreateEditBox(pPopup, GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getName(), WIDGET_GENERAL, gDLL->getText("TXT_KEY_MENU_LEADER_NAME"), 0, POPUP_LAYOUT_STRETCH, 0, MAX_PLAYERINFO_CHAR_COUNT);
		gDLL->getInterfaceIFace()->popupAddSeparator(pPopup);
		gDLL->getInterfaceIFace()->popupSetBodyString(pPopup, gDLL->getText("TXT_KEY_MENU_CIV_DESC"));
		gDLL->getInterfaceIFace()->popupCreateEditBox(pPopup, GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCivilizationDescription(), WIDGET_GENERAL, gDLL->getText("TXT_KEY_MENU_CIV_DESC"), 1, POPUP_LAYOUT_STRETCH, 0, MAX_PLAYERINFO_CHAR_COUNT);
		gDLL->getInterfaceIFace()->popupAddSeparator(pPopup);
		gDLL->getInterfaceIFace()->popupSetBodyString(pPopup, gDLL->getText("TXT_KEY_MENU_CIV_SHORT_DESC"));
		gDLL->getInterfaceIFace()->popupCreateEditBox(pPopup, GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCivilizationShortDescription(), WIDGET_GENERAL, gDLL->getText("TXT_KEY_MENU_CIV_SHORT_DESC"), 2, POPUP_LAYOUT_STRETCH, 0, MAX_PLAYERINFO_CHAR_COUNT);
		gDLL->getInterfaceIFace()->popupAddSeparator(pPopup);
		gDLL->getInterfaceIFace()->popupSetBodyString(pPopup, gDLL->getText("TXT_KEY_MENU_CIV_ADJ"));
		gDLL->getInterfaceIFace()->popupCreateEditBox(pPopup, GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCivilizationAdjective(), WIDGET_GENERAL, gDLL->getText("TXT_KEY_MENU_CIV_ADJ"), 3, POPUP_LAYOUT_STRETCH, 0, MAX_PLAYERINFO_CHAR_COUNT);
		gDLL->getInterfaceIFace()->popupAddSeparator(pPopup);
	}
	else if (!GC.getInitCore().getCivPassword(GC.getInitCore().getActivePlayer()).empty())
	{
		// the purpose of the popup with the option set to true is to ask for the civ password if it's not set
		return false;
	}
	if (GC.getGameINLINE().isPbem() || GC.getGameINLINE().isHotSeat() || GC.getGameINLINE().isPitboss())
	{
		gDLL->getInterfaceIFace()->popupSetBodyString(pPopup, gDLL->getText("TXT_KEY_MAIN_MENU_PASSWORD"));
		gDLL->getInterfaceIFace()->popupCreateEditBox(pPopup, PASSWORD_DEFAULT, WIDGET_GENERAL, gDLL->getText("TXT_KEY_MAIN_MENU_PASSWORD"), 4, POPUP_LAYOUT_STRETCH, 0, MAX_PASSWORD_CHAR_COUNT);
		gDLL->getInterfaceIFace()->popupAddSeparator(pPopup);
	}
	if ( (GC.getGameINLINE().isPitboss() || GC.getGameINLINE().isPbem()) && !info.getOption1() )
	{
		gDLL->getInterfaceIFace()->popupSetBodyString(pPopup, gDLL->getText("TXT_KEY_POPUP_DETAILS_EMAIL"));
		gDLL->getInterfaceIFace()->popupCreateEditBox(pPopup, CvWString(GC.getInitCore().getEmail(GC.getInitCore().getActivePlayer())), WIDGET_GENERAL, gDLL->getText("TXT_KEY_POPUP_DETAILS_EMAIL"), 5, POPUP_LAYOUT_STRETCH, 0, MAX_PLAYEREMAIL_CHAR_COUNT);
		gDLL->getInterfaceIFace()->popupAddSeparator(pPopup);
	}
	if (GC.getGameINLINE().isPbem() && !info.getOption1())
	{
		gDLL->getInterfaceIFace()->popupSetBodyString(pPopup, gDLL->getText("TXT_KEY_POPUP_DETAILS_SMTP"));
		gDLL->getInterfaceIFace()->popupCreateEditBox(pPopup, CvWString(GC.getInitCore().getSmtpHost(GC.getInitCore().getActivePlayer())), WIDGET_GENERAL, gDLL->getText("TXT_KEY_POPUP_DETAILS_SMTP"), 6, POPUP_LAYOUT_STRETCH, 0, MAX_PLAYEREMAIL_CHAR_COUNT);
		gDLL->getInterfaceIFace()->popupAddSeparator(pPopup);

		if (GC.getGameINLINE().getPbemTurnSent())
		{
			gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_MISC_SEND"), NULL, WIDGET_GENERAL, 0);
		}
	}

	// Disable leader name edit box for internet games
	if (GC.getInitCore().getMultiplayer() && gDLL->isFMPMgrPublic())
	{
		gDLL->getInterfaceIFace()->popupEnableEditBox(pPopup, 0, false);
	}

	gDLL->getInterfaceIFace()->popupLaunch(pPopup, true, POPUPSTATE_IMMEDIATE);
	return (true);
}

bool CvDLLButtonPopup::launchAdminPopup(CvPopup* pPopup, CvPopupInfo &info)
{
	gDLL->getInterfaceIFace()->popupSetHeaderString(pPopup, gDLL->getText("TXT_KEY_POPUP_GAME_DETAILS"));

	gDLL->getInterfaceIFace()->popupSetBodyString(pPopup, gDLL->getText("TXT_KEY_MAIN_MENU_GAME_NAME"));
	gDLL->getInterfaceIFace()->popupCreateEditBox(pPopup, GC.getInitCore().getGameName(), WIDGET_GENERAL, gDLL->getText("TXT_KEY_MAIN_MENU_GAME_NAME"), 0, POPUP_LAYOUT_STRETCH, 0, MAX_GAMENAME_CHAR_COUNT);
	gDLL->getInterfaceIFace()->popupAddSeparator(pPopup);
	gDLL->getInterfaceIFace()->popupSetBodyString(pPopup, gDLL->getText("TXT_KEY_POPUP_ADMIN_PASSWORD"));
	gDLL->getInterfaceIFace()->popupCreateEditBox(pPopup, PASSWORD_DEFAULT, WIDGET_GENERAL, gDLL->getText("TXT_KEY_POPUP_ADMIN_PASSWORD"), 1, POPUP_LAYOUT_STRETCH, 0, MAX_PASSWORD_CHAR_COUNT);
	gDLL->getInterfaceIFace()->popupAddSeparator(pPopup);
	if (!GC.getGameINLINE().isGameMultiPlayer())
	{
		gDLL->getInterfaceIFace()->popupCreateCheckBoxes(pPopup, 1, 2);
		gDLL->getInterfaceIFace()->popupSetCheckBoxText(pPopup, 0, gDLL->getText("TXT_KEY_POPUP_ADMIN_ALLOW_CHEATS"), 2);
		gDLL->getInterfaceIFace()->popupSetCheckBoxState(pPopup, 0, gDLL->getChtLvl() > 0, 2);
	}

	gDLL->getInterfaceIFace()->popupLaunch(pPopup, true, POPUPSTATE_IMMEDIATE);
	return (true);
}


bool CvDLLButtonPopup::launchAdminPasswordPopup(CvPopup* pPopup, CvPopupInfo &info)
{
	gDLL->getInterfaceIFace()->popupSetBodyString(pPopup, gDLL->getText("TXT_KEY_POPUP_ADMIN_PASSWORD"));
	gDLL->getInterfaceIFace()->popupCreateEditBox(pPopup, L"", WIDGET_GENERAL, gDLL->getText("TXT_KEY_POPUP_ADMIN_PASSWORD"), 0, POPUP_LAYOUT_STRETCH, 0, MAX_PASSWORD_CHAR_COUNT);

	gDLL->getInterfaceIFace()->popupLaunch(pPopup, true, POPUPSTATE_IMMEDIATE);
	return (true);
}


bool CvDLLButtonPopup::launchExtendedGamePopup(CvPopup* pPopup, CvPopupInfo &info)
{
	gDLL->getInterfaceIFace()->popupSetHeaderString(pPopup, gDLL->getText("TXT_KEY_EXTENDED_GAME_TITLE"));

	if (GC.getGameINLINE().countHumanPlayersAlive() > 0)
	{
		gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_EXTENDED_GAME_YES"), NULL, WIDGET_GENERAL, 0);
	}
	gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_EXTENDED_GAME_NO_MENU"), NULL, WIDGET_GENERAL, 1);

	gDLL->getInterfaceIFace()->popupLaunch(pPopup, false);
	return (true);
}

bool CvDLLButtonPopup::launchDiplomacyPopup(CvPopup* pPopup, CvPopupInfo &info)
{
	int iCount;
	int iI;

	gDLL->getInterfaceIFace()->popupSetHeaderString(pPopup, gDLL->getText("TXT_KEY_DIPLOMACY_TITLE"));

	iCount = 0;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER(GC.getGameINLINE().getActivePlayer()).canContact((PlayerTypes)iI))
			{
				gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, GET_PLAYER((PlayerTypes)iI).getName(), NULL, WIDGET_GENERAL, iI);
				iCount++;
			}
		}
	}

	if (iCount == 0)
	{
		return (false);
	}

	gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_NEVER_MIND"), NULL, WIDGET_GENERAL, MAX_CIV_PLAYERS);

	gDLL->getInterfaceIFace()->popupLaunch(pPopup, false, POPUPSTATE_IMMEDIATE);
	return (true);
}


bool CvDLLButtonPopup::launchAddBuddyPopup(CvPopup* pPopup, CvPopupInfo &info)
{
	gDLL->getInterfaceIFace()->popupSetHeaderString( pPopup, gDLL->getText("TXT_KEY_SYSTEM_ADD_BUDDY", info.getText().GetCString()) );
	gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_POPUP_YES"), NULL, WIDGET_GENERAL, 0);
	gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_POPUP_NO"), NULL, WIDGET_GENERAL, 1);
	gDLL->getInterfaceIFace()->popupLaunch(pPopup, false);
	return (true);
}

bool CvDLLButtonPopup::launchForcedDisconnectPopup(CvPopup* pPopup, CvPopupInfo &info)
{
	gDLL->getInterfaceIFace()->popupSetHeaderString( pPopup, gDLL->getText("TXT_KEY_MAIN_MENU_FORCED_DISCONNECT_INGAME") );
	gDLL->getInterfaceIFace()->popupLaunch(pPopup, true);
	return (true);
}

bool CvDLLButtonPopup::launchPitbossDisconnectPopup(CvPopup* pPopup, CvPopupInfo &info)
{
	gDLL->getInterfaceIFace()->popupSetHeaderString( pPopup, gDLL->getText("TXT_KEY_PITBOSS_DISCONNECT") );
	gDLL->getInterfaceIFace()->popupLaunch(pPopup, true);
	return (true);
}

bool CvDLLButtonPopup::launchKickedPopup(CvPopup* pPopup, CvPopupInfo &info)
{
	gDLL->getInterfaceIFace()->popupSetHeaderString( pPopup, gDLL->getText("TXT_KEY_POPUP_KICKED") );
	gDLL->getInterfaceIFace()->popupLaunch(pPopup, true);
	return (true);
}

bool CvDLLButtonPopup::launchVassalDemandTributePopup(CvPopup* pPopup, CvPopupInfo &info)
{
	if (info.getData1() == NO_PLAYER)
	{
		return false;
	}

	CvPlayer& kVassal = GET_PLAYER((PlayerTypes)info.getData1());
	if (!GET_TEAM(kVassal.getTeam()).isVassal(GC.getGameINLINE().getActiveTeam()))
	{
		return false;
	}

	int iNumResources = 0;
	if (kVassal.canTradeNetworkWith(GC.getGameINLINE().getActivePlayer()))
	{
		gDLL->getInterfaceIFace()->popupSetBodyString(pPopup, gDLL->getText("TXT_KEY_VASSAL_DEMAND_TRIBUTE", kVassal.getNameKey()));

		if (!kVassal.isTributePaid(GC.getGameINLINE().getActivePlayer()))
		{
			for (int iBonus = 0; iBonus < GC.getNumBonusInfos(); iBonus++)
			{
				if (kVassal.getNumTradeableBonuses((BonusTypes)iBonus) > 0 && GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getNumAvailableBonuses((BonusTypes)iBonus) == 0)
				{
					CvBonusInfo& info = GC.getBonusInfo((BonusTypes)iBonus);
					gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, info.getDescription(), info.getButton(), WIDGET_GENERAL, iBonus, -1, true, POPUP_LAYOUT_STRETCH, DLL_FONT_LEFT_JUSTIFY);
					++iNumResources;
				}
			}
		}
	}

	if (iNumResources > 0)
	{
		gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_NEVER_MIND"), ARTFILEMGR.getInterfaceArtInfo("INTERFACE_BUTTONS_CANCEL")->getPath(), WIDGET_GENERAL, GC.getNumBonusInfos());
	}
	else
	{
		gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_VASSAL_TRIBUTE_NOT_POSSIBLE"), ARTFILEMGR.getInterfaceArtInfo("INTERFACE_BUTTONS_CANCEL")->getPath(), WIDGET_GENERAL, GC.getNumBonusInfos());
	}

	gDLL->getInterfaceIFace()->popupLaunch(pPopup, false, POPUPSTATE_IMMEDIATE);

	return true;
}

bool CvDLLButtonPopup::launchVassalGrantTributePopup(CvPopup* pPopup, CvPopupInfo &info)
{
	if (info.getData1() == NO_PLAYER)
	{
		return false;
	}

	CvPlayer& kMaster = GET_PLAYER((PlayerTypes)info.getData1());
	if (!GET_TEAM(GC.getGameINLINE().getActiveTeam()).isVassal(kMaster.getTeam()))
	{
		return false;
	}

	if (info.getData2() == NO_BONUS)
	{
		return false;
	}

	gDLL->getInterfaceIFace()->popupSetBodyString(pPopup, gDLL->getText("TXT_KEY_VASSAL_GRANT_TRIBUTE", kMaster.getCivilizationDescriptionKey(), kMaster.getNameKey(), GC.getBonusInfo((BonusTypes)info.getData2()).getTextKeyWide()));
	gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_VASSAL_GRANT_TRIBUTE_YES"), NULL, WIDGET_GENERAL, 0);
	gDLL->getInterfaceIFace()->popupAddGenericButton(pPopup, gDLL->getText("TXT_KEY_VASSAL_GRANT_TRIBUTE_NO"), NULL, WIDGET_GENERAL, 1);
	gDLL->getInterfaceIFace()->popupLaunch(pPopup, false, POPUPSTATE_IMMEDIATE);

	return true;
}
