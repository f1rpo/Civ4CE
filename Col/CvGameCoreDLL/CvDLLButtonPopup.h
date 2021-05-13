#pragma once

// CvDLLButtonPopup.h

#ifndef CIV4_DLL_BUTTON_POPUP_H
#define CIV4_DLL_BUTTON_POPUP_H

//#include "CvEnums.h"
#include "CvPopupInfo.h"
#include "CvPopupReturn.h"

class CvPopup;
class CvDLLButtonPopup
{

public:
	CvDLLButtonPopup();
	virtual ~CvDLLButtonPopup();

	DllExport static CvDLLButtonPopup& getInstance();
	DllExport static void freeInstance();

	DllExport virtual void OnOkClicked(CvPopup* pPopup, PopupReturn *pPopupReturn, CvPopupInfo &info);
	DllExport virtual void OnAltExecute(CvPopup& popup, const PopupReturn& popupReturn, CvPopupInfo &info);
	DllExport virtual void OnEscape(CvPopup& pPopup, CvPopupInfo &info);
	DllExport virtual void OnFocus(CvPopup* pPopup, CvPopupInfo &info);

	DllExport bool launchButtonPopup(CvPopup* pPopup, CvPopupInfo &info);

private:

	static CvDLLButtonPopup* m_pInst;

	bool launchTextPopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchProductionPopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchChooseYieldBuildPopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchEducationPopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchRazeCityPopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchAlarmPopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchDeclareWarMovePopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchConfirmCommandPopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchConfirmTaskPopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchLoadUnitPopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchLoadCargoPopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchLeadUnitPopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchMainMenuPopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchConfirmMenu(CvPopup *pPopup, CvPopupInfo &info);
	bool launchPythonScreen(CvPopup* pPopup, CvPopupInfo &info);
	bool launchMovie(CvPopup* pPopup, CvPopupInfo &info);
	bool launchPythonPopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchDetailsPopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchAdminPopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchAdminPasswordPopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchCancelDeal(CvPopup* pPopup, CvPopupInfo &info);
	bool launchExtendedGamePopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchDiplomacyPopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchAddBuddyPopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchForcedDisconnectPopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchPitbossDisconnectPopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchKickedPopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchEventPopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchFreeColonyPopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchChooseProfessionPopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchPurchaseEuropeUnitPopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchFoundingFatherPopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchCivicOptionPopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchTradeRoutesPopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchYieldImportExportPopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchPromotionPopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchChooseGoodyPopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchSelectYieldAmountPopup(CvPopup* pPopup, CvPopupInfo &info);
	bool launchTalkNativesPopup(CvPopup* pPopup, CvPopupInfo &info);
};

#endif
