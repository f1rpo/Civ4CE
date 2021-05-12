//---------------------------------------------------------------------------------------
//
//  *****************   Civilization IV   ********************
//
//  FILE:    CvGameTextMgr.cpp
//
//  AUTHOR:  Jesse Smith / Mustafa Thamer	10/2004 / Jon Shafer 06/2005
//
//  PURPOSE: Interfaces with GameText XML Files to manage the paths of art files
//
//---------------------------------------------------------------------------------------
//  Copyright (c) 2004 Firaxis Games, Inc. All rights reserved.
//---------------------------------------------------------------------------------------

#include "CvGameCoreDLL.h"
#include "CvGameTextMgr.h"
#include "CvGameCoreUtils.h"
#include "CvDLLUtilityIFaceBase.h"
#include "CvDLLInterfaceIFaceBase.h"
#include "CvDLLSymbolIFaceBase.h"
#include "CvInfos.h"
#include "CvXMLLoadUtility.h"
#include "CvCity.h"
#include "CvPlayerAI.h"
#include "CvTeamAI.h"
#include "CvGameAI.h"
#include "CvSelectionGroup.h"
#include "CvMap.h"
#include "CvArea.h"
#include "CvPlot.h"
#include "CvPopupInfo.h"
#include "FProfiler.h"


// For displaying Asserts and error messages
static char* szErrorMsg;

//----------------------------------------------------------------------------
//
//	FUNCTION:	GetInstance()
//
//	PURPOSE:	Get the instance of this class.
//
//----------------------------------------------------------------------------
CvGameTextMgr& CvGameTextMgr::GetInstance()
{
	static CvGameTextMgr gs_GameTextMgr;
	return gs_GameTextMgr;
}

//----------------------------------------------------------------------------
//
//	FUNCTION:	CvGameTextMgr()
//
//	PURPOSE:	Constructor
//
//----------------------------------------------------------------------------
CvGameTextMgr::CvGameTextMgr() :
m_iNumDiplomacyTextInfos(0),
m_paDiplomacyTextInfo(NULL)
{

}

CvGameTextMgr::~CvGameTextMgr()
{
}

//----------------------------------------------------------------------------
//
//	FUNCTION:	Initialize()
//
//	PURPOSE:	Allocates memory
//
//----------------------------------------------------------------------------
void CvGameTextMgr::Initialize()
{

}

//----------------------------------------------------------------------------
//
//	FUNCTION:	DeInitialize()
//
//	PURPOSE:	Clears memory
//
//----------------------------------------------------------------------------
void CvGameTextMgr::DeInitialize()
{
	for(int i=0;i<(int)m_apbPromotion.size();i++)
	{
		delete [] m_apbPromotion[i];
	}
}

//----------------------------------------------------------------------------
//
//	FUNCTION:	Reset()
//
//	PURPOSE:	Accesses CvXMLLoadUtility to clean global text memory and
//				reload the XML files
//
//----------------------------------------------------------------------------
void CvGameTextMgr::Reset()
{
	CvXMLLoadUtility pXML;
	pXML.LoadGlobalText();
}


// Returns the current language
int CvGameTextMgr::getCurrentLanguage()
{
	return gDLL->getCurrentLanguage();
}


void CvGameTextMgr::setDateStr(CvWString& szString, int iGameTurn, bool bSave, CalendarTypes eCalendar, int iStartYear, GameSpeedTypes eSpeed)
{
	CvWString szYearBuffer;
	CvWString szWeekBuffer;

	int iTurnYear = getTurnYearForGame(iGameTurn, iStartYear, eCalendar, eSpeed);

	if (iTurnYear < 0)
	{
		if (bSave)
		{
			szYearBuffer = gDLL->getText("TXT_KEY_TIME_BC_SAVE", CvWString::format(L"%04d", -iTurnYear).GetCString());
		}
		else
		{
			szYearBuffer = gDLL->getText("TXT_KEY_TIME_BC", -(iTurnYear));
		}
	}
	else if (iTurnYear > 0)
	{
		if (bSave)
		{
			szYearBuffer = gDLL->getText("TXT_KEY_TIME_AD_SAVE", CvWString::format(L"%04d", iTurnYear).GetCString());
		}
		else
		{
			szYearBuffer = gDLL->getText("TXT_KEY_TIME_AD", iTurnYear);
		}
	}
	else
	{
		if (bSave)
		{
			szYearBuffer = gDLL->getText("TXT_KEY_TIME_AD_SAVE", L"0001");
		}
		else
		{
			szYearBuffer = gDLL->getText("TXT_KEY_TIME_AD", 1);
		}
	}

	switch (eCalendar)
	{
	case CALENDAR_DEFAULT:
	case CALENDAR_YEARS:
	case CALENDAR_BI_YEARLY:
		szString = szYearBuffer;
		break;

	case CALENDAR_TURNS:
		szString = gDLL->getText("TXT_KEY_TIME_TURN", (iGameTurn + 1));
		break;

	case CALENDAR_SEASONS:
		if (bSave)
		{
			szString = (szYearBuffer + "-" + GC.getSeasonInfo((SeasonTypes)(iGameTurn % GC.getNumSeasonInfos())).getDescription());
		}
		else
		{
			szString = (GC.getSeasonInfo((SeasonTypes)(iGameTurn % GC.getNumSeasonInfos())).getDescription() + CvString(", ") + szYearBuffer);
		}
		break;

	case CALENDAR_MONTHS:
		if (bSave)
		{
			szString = (szYearBuffer + "-" + GC.getMonthInfo((MonthTypes)(iGameTurn % GC.getNumMonthInfos())).getDescription());
		}
		else
		{
			szString = (GC.getMonthInfo((MonthTypes)(iGameTurn % GC.getNumMonthInfos())).getDescription() + CvString(", ") + szYearBuffer);
		}
		break;

	case CALENDAR_WEEKS:
		szWeekBuffer = gDLL->getText("TXT_KEY_TIME_WEEK", ((iGameTurn % GC.getDefineINT("WEEKS_PER_MONTHS")) + 1));

		if (bSave)
		{
			szString = (szYearBuffer + "-" + GC.getMonthInfo((MonthTypes)((iGameTurn / GC.getDefineINT("WEEKS_PER_MONTHS")) % GC.getNumMonthInfos())).getDescription() + "-" + szWeekBuffer);
		}
		else
		{
			szString = (szWeekBuffer + ", " + GC.getMonthInfo((MonthTypes)((iGameTurn / GC.getDefineINT("WEEKS_PER_MONTHS")) % GC.getNumMonthInfos())).getDescription() + ", " + szYearBuffer);
		}
		break;

	default:
		FAssert(false);
	}
}


void CvGameTextMgr::setTimeStr(CvWString& szString, int iGameTurn, bool bSave)
{
	setDateStr(szString, iGameTurn, bSave, GC.getGameINLINE().getCalendar(), GC.getGameINLINE().getStartYear(), GC.getGameINLINE().getGameSpeedType());
}


void CvGameTextMgr::setInterfaceTime(CvWString& szString, PlayerTypes ePlayer)
{
	CvWString szTempBuffer;

	if (GET_PLAYER(ePlayer).isGoldenAge())
	{
		szString.Format(L"%c(%d) ", gDLL->getSymbolID(GOLDEN_AGE_CHAR), GET_PLAYER(ePlayer).getGoldenAgeTurns());
	}
	else
	{
		clear(szString);
	}

	setTimeStr(szTempBuffer, GC.getGameINLINE().getGameTurn(), false);
	szString += CvWString(szTempBuffer);
}


void CvGameTextMgr::setGoldStr(CvWString& szString, PlayerTypes ePlayer)
{
	if (GET_PLAYER(ePlayer).getGold() < 0)
	{
		szString.Format(L"%c: " SETCOLR L"%d" SETCOLR, GC.getCommerceInfo(COMMERCE_GOLD).getChar(), TEXT_COLOR("COLOR_NEGATIVE_TEXT"), GET_PLAYER(ePlayer).getGold());
	}
	else
	{
		szString.Format(L"%c: %d", GC.getCommerceInfo(COMMERCE_GOLD).getChar(), GET_PLAYER(ePlayer).getGold());
	}

	int iGoldRate = GET_PLAYER(ePlayer).calculateGoldRate();
	if (iGoldRate < 0)
	{
		szString += gDLL->getText("TXT_KEY_MISC_NEG_GOLD_PER_TURN", iGoldRate);
	}
	else if (iGoldRate > 0)
	{
		szString += gDLL->getText("TXT_KEY_MISC_POS_GOLD_PER_TURN", iGoldRate);
	}

	if (GET_PLAYER(ePlayer).isStrike())
	{
		szString += gDLL->getText("TXT_KEY_MISC_STRIKE");
	}
}


void CvGameTextMgr::setResearchStr(CvWString& szString, PlayerTypes ePlayer)
{
	CvWString szTempBuffer;

	szString = gDLL->getText("TXT_KEY_MISC_RESEARCH_STRING", GC.getTechInfo(GET_PLAYER(ePlayer).getCurrentResearch()).getTextKeyWide());

	if (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getTechCount(GET_PLAYER(ePlayer).getCurrentResearch()) > 0)
	{
		szTempBuffer.Format(L" %d", (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getTechCount(GET_PLAYER(ePlayer).getCurrentResearch()) + 1));
		szString+=szTempBuffer;
	}

	szTempBuffer.Format(L" (%d)", GET_PLAYER(ePlayer).getResearchTurnsLeft(GET_PLAYER(ePlayer).getCurrentResearch(), true));
	szString+=szTempBuffer;
}


void CvGameTextMgr::setOOSSeeds(CvWString& szString, PlayerTypes ePlayer)
{
	if (GET_PLAYER(ePlayer).isHuman())
	{
		int iNetID = GET_PLAYER(ePlayer).getNetID();
		if (gDLL->isConnected(iNetID))
		{
			szString = gDLL->getText("TXT_KEY_PLAYER_OOS", gDLL->GetSyncOOS(iNetID), gDLL->GetOptionsOOS(iNetID));
		}
	}
}

void CvGameTextMgr::setNetStats(CvWString& szString, PlayerTypes ePlayer)
{
	if (ePlayer != GC.getGameINLINE().getActivePlayer())
	{
		if (GET_PLAYER(ePlayer).isHuman())
		{
			if (gDLL->getInterfaceIFace()->isNetStatsVisible())
			{
				int iNetID = GET_PLAYER(ePlayer).getNetID();
				if (gDLL->isConnected(iNetID))
				{
					szString = gDLL->getText("TXT_KEY_MISC_NUM_MS", gDLL->GetLastPing(iNetID));
				}
				else
				{
					szString = gDLL->getText("TXT_KEY_MISC_DISCONNECTED");
				}
			}
		}
		else
		{
			szString = gDLL->getText("TXT_KEY_MISC_AI");
		}
	}
}


void CvGameTextMgr::setMinimizePopupHelp(CvWString& szString, const CvPopupInfo & info)
{
	CvCity* pCity;
	UnitTypes eTrainUnit;
	BuildingTypes eConstructBuilding;
	ProjectTypes eCreateProject;
	ReligionTypes eReligion;
	CivicTypes eCivic;

	switch (info.getButtonPopupType())
	{
	case BUTTONPOPUP_CHOOSEPRODUCTION:
		pCity = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCity(info.getData1());
		if (pCity != NULL)
		{
			eTrainUnit = NO_UNIT;
			eConstructBuilding = NO_BUILDING;
			eCreateProject = NO_PROJECT;

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

			if (eTrainUnit != NO_UNIT)
			{
				szString += gDLL->getText("TXT_KEY_MINIMIZED_CHOOSE_PRODUCTION_UNIT", GC.getUnitInfo(eTrainUnit).getTextKeyWide(), pCity->getNameKey());
			}
			else if (eConstructBuilding != NO_BUILDING)
			{
				szString += gDLL->getText("TXT_KEY_MINIMIZED_CHOOSE_PRODUCTION_BUILDING", GC.getBuildingInfo(eConstructBuilding).getTextKeyWide(), pCity->getNameKey());
			}
			else if (eCreateProject != NO_PROJECT)
			{
				szString += gDLL->getText("TXT_KEY_MINIMIZED_CHOOSE_PRODUCTION_PROJECT", GC.getProjectInfo(eCreateProject).getTextKeyWide(), pCity->getNameKey());
			}
			else
			{
				szString += gDLL->getText("TXT_KEY_MINIMIZED_CHOOSE_PRODUCTION", pCity->getNameKey());
			}
		}
		break;

	case BUTTONPOPUP_CHANGERELIGION:
		eReligion = ((ReligionTypes)(info.getData1()));
		if (eReligion != NO_RELIGION)
		{
			szString += gDLL->getText("TXT_KEY_MINIMIZED_CHANGE_RELIGION", GC.getReligionInfo(eReligion).getTextKeyWide());
		}
		break;

	case BUTTONPOPUP_CHOOSETECH:
		if (info.getData1() > 0)
		{
			szString += gDLL->getText("TXT_KEY_MINIMIZED_CHOOSE_TECH_FREE");
		}
		else
		{
			szString += gDLL->getText("TXT_KEY_MINIMIZED_CHOOSE_TECH");
		}
		break;

	case BUTTONPOPUP_CHANGECIVIC:
		eCivic = ((CivicTypes)(info.getData2()));
		if (eCivic != NO_CIVIC)
		{
			szString += gDLL->getText("TXT_KEY_MINIMIZED_CHANGE_CIVIC", GC.getCivicInfo(eCivic).getTextKeyWide());
		}
		break;
	}
}


void CvGameTextMgr::setUnitHelp(CvWString &szString, const CvUnit* pUnit, bool bOneLine, bool bShort)
{
	CvWString szTempBuffer;
	BuildTypes eBuild;
	int iCurrMoves;
	int iI;
	bool bFirst;

	szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_UNIT_TEXT"), pUnit->getName().GetCString());
	szString += szTempBuffer;

	szString += (L", ");

	if (pUnit->getDomainType() == DOMAIN_AIR)
	{
		if (pUnit->airBaseCombatStr() > 0)
		{
			if (pUnit->isFighting())
			{
				szTempBuffer.Format(L"?/%d%c, ", pUnit->airBaseCombatStr(), gDLL->getSymbolID(STRENGTH_CHAR));
			}
			else if (pUnit->isHurt())
			{
				szTempBuffer.Format(L"%.1f/%d%c, ", (((float)(pUnit->airBaseCombatStr() * pUnit->currHitPoints())) / ((float)(pUnit->maxHitPoints()))), pUnit->airBaseCombatStr(), gDLL->getSymbolID(STRENGTH_CHAR));
			}
			else
			{
				szTempBuffer.Format(L"%d%c, ", pUnit->airBaseCombatStr(), gDLL->getSymbolID(STRENGTH_CHAR));
			}
			szString += szTempBuffer;
		}
	}
	else
	{
		if (pUnit->canFight())
		{
			if (pUnit->isFighting())
			{
				szTempBuffer.Format(L"?/%d%c, ", pUnit->baseCombatStr(), gDLL->getSymbolID(STRENGTH_CHAR));
			}
			else if (pUnit->isHurt())
			{
				szTempBuffer.Format(L"%.1f/%d%c, ", (((float)(pUnit->baseCombatStr() * pUnit->currHitPoints())) / ((float)(pUnit->maxHitPoints()))), pUnit->baseCombatStr(), gDLL->getSymbolID(STRENGTH_CHAR));
			}
			else
			{
				szTempBuffer.Format(L"%d%c, ", pUnit->baseCombatStr(), gDLL->getSymbolID(STRENGTH_CHAR));
			}
			szString += szTempBuffer;
		}
	}

	iCurrMoves = ((pUnit->movesLeft() / GC.getMOVE_DENOMINATOR()) + (((pUnit->movesLeft() % GC.getMOVE_DENOMINATOR()) > 0) ? 1 : 0));
	if ((pUnit->baseMoves() == iCurrMoves) || (pUnit->getTeam() != GC.getGameINLINE().getActiveTeam()))
	{
		szTempBuffer.Format(L"%d%c", pUnit->baseMoves(), gDLL->getSymbolID(MOVES_CHAR));
	}
	else
	{
		szTempBuffer.Format(L"%d/%d%c", iCurrMoves, pUnit->baseMoves(), gDLL->getSymbolID(MOVES_CHAR));
	}
	szString += (szTempBuffer);

	if (pUnit->airRange() > 0)
	{
		szString += gDLL->getText("TXT_KEY_UNIT_HELP_AIR_RANGE", pUnit->airRange());
	}

	eBuild = pUnit->getBuildType();

	if (eBuild != NO_BUILD)
	{
		szString += L", ";
		szTempBuffer.Format(L"%s (%d)", GC.getBuildInfo(eBuild).getDescription(), pUnit->plot()->getBuildTurnsLeft(eBuild, 0, 0));
		szString += szTempBuffer;
	}

	/*if (!bOneLine)
	{
		if (pUnit->getUnitCombatType() != NO_UNITCOMBAT)
		{
			szTempBuffer.Format(L" (%s)", GC.getUnitCombatInfo(pUnit->getUnitCombatType()).getDescription());
			szString += szTempBuffer;
		}
	}*/

	if (GC.getGameINLINE().isDebugMode())
	{
		FAssertMsg(pUnit->AI_getUnitAIType() != NO_UNITAI, "pUnit's AI type expected to != NO_UNITAI");
		szTempBuffer.Format(L" (%s)", GC.getUnitAIInfo(pUnit->AI_getUnitAIType()).getDescription());
		szString += szTempBuffer;
	}

	if ((pUnit->getTeam() == GC.getGameINLINE().getActiveTeam()) || GC.getGameINLINE().isDebugMode())
	{
		if ((pUnit->getExperience() > 0) && !(pUnit->isFighting()))
		{
			szString += gDLL->getText("TXT_KEY_UNIT_HELP_LEVEL", pUnit->getExperience(), pUnit->experienceNeeded());
		}
	}

	if (pUnit->getOwnerINLINE() != GC.getGameINLINE().getActivePlayer() && !pUnit->isAnimal())
	{
		szString += (L", ");
		szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, GET_PLAYER(pUnit->getOwnerINLINE()).getPlayerTextColorR(), GET_PLAYER(pUnit->getOwnerINLINE()).getPlayerTextColorG(), GET_PLAYER(pUnit->getOwnerINLINE()).getPlayerTextColorB(), GET_PLAYER(pUnit->getOwnerINLINE()).getPlayerTextColorA(), GET_PLAYER(pUnit->getOwnerINLINE()).getName());
		szString += szTempBuffer;
	}

	for (iI = 0; iI < GC.getNumPromotionInfos(); iI++)
	{
		if (pUnit->isHasPromotion((PromotionTypes)iI))
		{
			szTempBuffer.Format(L"<img=%S size=16></img>", GC.getPromotionInfo((PromotionTypes)iI).getButton());
			szString += szTempBuffer;
		}
	}

	if (!bOneLine)
	{
		if (pUnit->cargoSpace() > 0)
		{
			if (pUnit->getTeam() == GC.getGameINLINE().getActiveTeam())
			{
				szTempBuffer = NEWLINE + gDLL->getText("TXT_KEY_UNIT_HELP_CARGO_SPACE", pUnit->getCargo(), pUnit->cargoSpace());
			}
			else
			{
				szTempBuffer = NEWLINE + gDLL->getText("TXT_KEY_UNIT_CARGO_SPACE", pUnit->cargoSpace());
			}
			szString += (szTempBuffer);

			if (pUnit->specialCargo() != NO_SPECIALUNIT)
			{
				szString += gDLL->getText("TXT_KEY_UNIT_CARRIES", GC.getSpecialUnitInfo(pUnit->specialCargo()).getTextKeyWide());
			}
		}

		if (pUnit->fortifyModifier() != 0)
		{
			szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_HELP_FORTIFY_BONUS", pUnit->fortifyModifier());
		}

		if (!bShort)
		{
			if (pUnit->nukeRange() >= 0)
			{
	//			szTempBuffer.Format(L"\n%cCan Nuke Enemy Lands", gDLL->getSymbolID(BULLET_CHAR));
				szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_CAN_NUKE");
			}

			if (pUnit->alwaysInvisible())
			{
	//			szTempBuffer.Format(L"\n%cInvisible to All Units", gDLL->getSymbolID(BULLET_CHAR));
				szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_INVISIBLE_ALL");
			}
			else if (pUnit->getInvisibleType() != NO_INVISIBLE)
			{
	//			szTempBuffer.Format(L"\n%cInvisible to Most Units", gDLL->getSymbolID(BULLET_CHAR));
				szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_INVISIBLE_MOST");
			}

			if ((pUnit->getSeeInvisibleType() != NO_INVISIBLE) && (pUnit->getSeeInvisibleType() != pUnit->getInvisibleType()))
			{
	//			szTempBuffer.Format(L"\n%cCan See %s", gDLL->getSymbolID(BULLET_CHAR), GC.getInvisibleInfo(pUnit->getSeeInvisibleType()).getDescription());
				szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_SEE_INVISIBLE", GC.getInvisibleInfo(pUnit->getSeeInvisibleType()).getTextKeyWide());
			}

			if (pUnit->canMoveImpassable())
			{
				szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_CAN_MOVE_IMPASSABLE");
			}
		}

		if (pUnit->maxFirstStrikes() > 0)
		{
			if (pUnit->firstStrikes() == pUnit->maxFirstStrikes())
			{
				if (pUnit->firstStrikes() == 1)
				{
					szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_ONE_FIRST_STRIKE");
				}
				else
				{
					szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_NUM_FIRST_STRIKES", pUnit->firstStrikes());
				}
			}
			else
			{
				szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_FIRST_STRIKE_CHANCES", pUnit->firstStrikes(), pUnit->maxFirstStrikes());
			}
		}

		if (pUnit->immuneToFirstStrikes())
		{
//			szTempBuffer.Format(L"\n%cImmune to First Strikes", gDLL->getSymbolID(BULLET_CHAR));
			szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_IMMUNE_FIRST_STRIKES");
		}

		if (!bShort)
		{
			if (pUnit->noDefensiveBonus())
			{
	//			szTempBuffer.Format(L"\n%cDoesn't Receive Defensive Bonuses", gDLL->getSymbolID(BULLET_CHAR));
				szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_NO_DEFENSE_BONUSES");
			}

			if (pUnit->flatMovementCost())
			{
				szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_FLAT_MOVEMENT");
			}

			if (pUnit->ignoreTerrainCost())
			{
				szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_IGNORE_TERRAIN");
			}

			if (pUnit->isBlitz())
			{
				//			szTempBuffer.Format(L"\n%cCan Attack Multiple Times per Turn", gDLL->getSymbolID(BULLET_CHAR));
				szString += NEWLINE + gDLL->getText("TXT_KEY_PROMOTION_BLITZ_TEXT");
			}

			if (pUnit->isAmphib())
			{
				//			szTempBuffer.Format(L"\n%cNo Combat Penalty for Attacking from Sea", gDLL->getSymbolID(BULLET_CHAR));
				szString += NEWLINE + gDLL->getText("TXT_KEY_PROMOTION_AMPHIB_TEXT");
			}

			if (pUnit->isRiver())
			{
				//			szTempBuffer.Format(L"\n%cNo Combat Penalty for Crossing River", gDLL->getSymbolID(BULLET_CHAR));
				szString += NEWLINE + gDLL->getText("TXT_KEY_PROMOTION_RIVER_ATTACK_TEXT");
			}

			if (pUnit->isEnemyRoute())
			{
				//			szTempBuffer.Format(L"\n%cCan Use Enemy Roads", gDLL->getSymbolID(BULLET_CHAR));
				szString +=NEWLINE +  gDLL->getText("TXT_KEY_PROMOTION_ENEMY_ROADS_TEXT");
			}

			if (pUnit->isAlwaysHeal())
			{
				//			szTempBuffer.Format(L"\n%cCan Heal while Moving", gDLL->getSymbolID(BULLET_CHAR));
				szString += NEWLINE + gDLL->getText("TXT_KEY_PROMOTION_ALWAYS_HEAL_TEXT");
			}

			if (pUnit->isHillsDoubleMove())
			{
				//			szTempBuffer.Format(L"\n%cDouble Movement in Hills", gDLL->getSymbolID(BULLET_CHAR));
				szString += NEWLINE + gDLL->getText("TXT_KEY_PROMOTION_HILLS_MOVE_TEXT");
			}

			for (iI = 0; iI < GC.getNumTerrainInfos(); iI++)
			{
				if (pUnit->isTerrainDoubleMove((TerrainTypes)iI))
				{
					//				szTempBuffer.Format(L"\n%cDouble Movement in %s", gDLL->getSymbolID(BULLET_CHAR), GC.getTerrainInfo((TerrainTypes) iI).getDescription());
					szString += NEWLINE + gDLL->getText("TXT_KEY_PROMOTION_DOUBLE_MOVE_TEXT", GC.getTerrainInfo((TerrainTypes) iI).getTextKeyWide());
				}
			}

			for (iI = 0; iI < GC.getNumFeatureInfos(); iI++)
			{
				if (pUnit->isFeatureDoubleMove((FeatureTypes)iI))
				{
	//				szTempBuffer.Format(L"\n%cDouble Movement in %s", gDLL->getSymbolID(BULLET_CHAR), GC.getFeatureInfo((FeatureTypes) iI).getDescription());
					szString += NEWLINE + gDLL->getText("TXT_KEY_PROMOTION_DOUBLE_MOVE_TEXT", GC.getFeatureInfo((FeatureTypes) iI).getTextKeyWide());
				}
			}

			if (pUnit->getExtraVisibilityRange() != 0)
			{
	//			szTempBuffer.Format(L"\n%c%s%d Visibility Range", gDLL->getSymbolID(BULLET_CHAR), ((pUnit->getExtraVisibilityRange() > 0) ? "+" : ""), pUnit->getExtraVisibilityRange());
				szString += NEWLINE + gDLL->getText("TXT_KEY_PROMOTION_VISIBILITY_TEXT", pUnit->getExtraVisibilityRange());
			}

			if (pUnit->getExtraMoveDiscount() != 0)
			{
	//			szTempBuffer.Format(L"\n%c%s%d Terrain Movement Cost", gDLL->getSymbolID(BULLET_CHAR), ((-(pUnit->getExtraMoveDiscount()) > 0) ? "+" : ""), -(pUnit->getExtraMoveDiscount()));
				szString += NEWLINE + gDLL->getText("TXT_KEY_PROMOTION_MOVE_DISCOUNT_TEXT", -(pUnit->getExtraMoveDiscount()));
			}

			if (pUnit->getExtraEnemyHeal() != 0)
			{
	//			szTempBuffer.Format(L"\n%cHeals Extra %d%% Damage/Turn in Enemy Lands", gDLL->getSymbolID(BULLET_CHAR), pUnit->getExtraEnemyHeal());
				szString += NEWLINE + gDLL->getText("TXT_KEY_PROMOTION_HEALS_EXTRA_TEXT", pUnit->getExtraEnemyHeal()) + gDLL->getText("TXT_KEY_PROMOTION_ENEMY_LANDS_TEXT");
			}

			if (pUnit->getExtraNeutralHeal() != 0)
			{
	//			szTempBuffer.Format(L"\n%cHeals Extra %d%% Damage/Turn in Neutral Lands", gDLL->getSymbolID(BULLET_CHAR), pUnit->getExtraNeutralHeal());
				szString += NEWLINE + gDLL->getText("TXT_KEY_PROMOTION_HEALS_EXTRA_TEXT", pUnit->getExtraNeutralHeal()) + gDLL->getText("TXT_KEY_PROMOTION_NEUTRAL_LANDS_TEXT");
			}

			if (pUnit->getExtraFriendlyHeal() != 0)
			{
	//			szTempBuffer.Format(L"\n%cHeals Extra %d%% Damage/Turn in Friendly Lands", gDLL->getSymbolID(BULLET_CHAR), pUnit->getExtraFriendlyHeal());
				szString += NEWLINE + gDLL->getText("TXT_KEY_PROMOTION_HEALS_EXTRA_TEXT", pUnit->getExtraFriendlyHeal()) + gDLL->getText("TXT_KEY_PROMOTION_FRIENDLY_LANDS_TEXT");
			}

			if (pUnit->getSameTileHeal() != 0)
			{
	//			szTempBuffer.Format(L"\n%cHeals Units in Same Tile Extra %d%% Damage/Turn", gDLL->getSymbolID(BULLET_CHAR), pUnit->getSameTileHeal());
				szString += NEWLINE + gDLL->getText("TXT_KEY_PROMOTION_HEALS_SAME_TEXT", pUnit->getSameTileHeal()) + gDLL->getText("TXT_KEY_PROMOTION_DAMAGE_TURN_TEXT");
			}

			if (pUnit->getAdjacentTileHeal() != 0)
			{
	//			szTempBuffer.Format(L"\n%cHeals Units in Adjacent Tiles Extra %d%% Damage/Turn", gDLL->getSymbolID(BULLET_CHAR), pUnit->getAdjacentTileHeal());
				szString += NEWLINE + gDLL->getText("TXT_KEY_PROMOTION_HEALS_ADJACENT_TEXT", pUnit->getAdjacentTileHeal()) + gDLL->getText("TXT_KEY_PROMOTION_DAMAGE_TURN_TEXT");
			}
		}

		if (pUnit->currInterceptionProbability() > 0)
		{
			szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_INTERCEPT_AIRCRAFT", pUnit->currInterceptionProbability());
		}

		if (pUnit->evasionProbability() > 0)
		{
			szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_EVADE_INTERCEPTION", pUnit->evasionProbability());
		}

		if (pUnit->withdrawalProbability() > 0)
		{
			if (bShort)
			{
				szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_WITHDRAWL_PROBABILITY_SHORT", pUnit->withdrawalProbability());
			}
			else
			{
				szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_WITHDRAWL_PROBABILITY", pUnit->withdrawalProbability());
			}
		}

		if (pUnit->collateralDamage() > 0)
		{
			if (pUnit->getExtraCollateralDamage() == 0)
			{
				szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_COLLATERAL_DAMAGE");
			}
			else
			{
				szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_COLLATERAL_DAMAGE_EXTRA", pUnit->getExtraCollateralDamage());
			}
		}

		if (pUnit->getCollateralDamageProtection() > 0)
		{
			szString += NEWLINE + gDLL->getText("TXT_KEY_PROMOTION_COLLATERAL_PROTECTION_TEXT", pUnit->getCollateralDamageProtection());
		}

		if (pUnit->getExtraCombatPercent() != 0)
		{
			szString += NEWLINE + gDLL->getText("TXT_KEY_PROMOTION_STRENGTH_TEXT", pUnit->getExtraCombatPercent());
		}

		if (pUnit->cityAttackModifier() == pUnit->cityDefenseModifier())
		{
			if (pUnit->cityAttackModifier() != 0)
			{
				szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_CITY_STRENGTH_MOD", pUnit->cityAttackModifier());
			}
		}
		else
		{
			if (pUnit->cityAttackModifier() != 0)
			{
				szString += NEWLINE + gDLL->getText("TXT_KEY_PROMOTION_CITY_ATTACK_TEXT", pUnit->cityAttackModifier());
			}

			if (pUnit->cityDefenseModifier() != 0)
			{
				szString += NEWLINE + gDLL->getText("TXT_KEY_PROMOTION_CITY_DEFENSE_TEXT", pUnit->cityDefenseModifier());
			}
		}

		if (pUnit->animalCombatModifier() != 0)
		{
			szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_ANIMAL_COMBAT_MOD", pUnit->animalCombatModifier());
		}

		if (pUnit->hillsAttackModifier() == pUnit->hillsDefenseModifier())
		{
			if (pUnit->hillsAttackModifier() != 0)
			{
				szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_HILLS_STRENGTH", pUnit->hillsAttackModifier());
			}
		}
		else
		{
			if (pUnit->hillsAttackModifier() != 0)
			{
				szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_HILLS_ATTACK", pUnit->hillsAttackModifier());
			}

			if (pUnit->hillsDefenseModifier() != 0)
			{
				szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_HILLS_DEFENSE", pUnit->hillsDefenseModifier());
			}
		}

		for (iI = 0; iI < GC.getNumTerrainInfos(); iI++)
		{
			if (pUnit->terrainDefenseModifier((TerrainTypes)iI) != 0)
			{
				szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_DEFENSE", pUnit->terrainDefenseModifier((TerrainTypes)iI), GC.getTerrainInfo((TerrainTypes) iI).getTextKeyWide());
			}
		}

		for (iI = 0; iI < GC.getNumFeatureInfos(); iI++)
		{
			if (pUnit->featureDefenseModifier((FeatureTypes)iI) != 0)
			{
				szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_DEFENSE", pUnit->featureDefenseModifier((FeatureTypes)iI), GC.getFeatureInfo((FeatureTypes) iI).getTextKeyWide());
			}
		}

		for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
		{
			if (GC.getUnitInfo(pUnit->getUnitType()).getUnitClassAttackModifier(iI) == GC.getUnitInfo(pUnit->getUnitType()).getUnitClassDefenseModifier(iI))
			{
				if (GC.getUnitInfo(pUnit->getUnitType()).getUnitClassAttackModifier(iI) != 0)
				{
					szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_MOD_VS_TYPE", GC.getUnitInfo(pUnit->getUnitType()).getUnitClassAttackModifier(iI), GC.getUnitClassInfo((UnitClassTypes)iI).getTextKeyWide());
				}
			}
			else
			{
				if (GC.getUnitInfo(pUnit->getUnitType()).getUnitClassAttackModifier(iI) != 0)
				{
					szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_ATTACK_MOD_VS_CLASS", GC.getUnitInfo(pUnit->getUnitType()).getUnitClassAttackModifier(iI), GC.getUnitClassInfo((UnitClassTypes)iI).getTextKeyWide());
				}

				if (GC.getUnitInfo(pUnit->getUnitType()).getUnitClassDefenseModifier(iI) != 0)
				{
					szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_DEFENSE_MOD_VS_CLASS", GC.getUnitInfo(pUnit->getUnitType()).getUnitClassDefenseModifier(iI), GC.getUnitClassInfo((UnitClassTypes) iI).getTextKeyWide());
				}
			}
		}

		for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
		{
			if (pUnit->unitCombatModifier((UnitCombatTypes)iI) != 0)
			{
				szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_MOD_VS_TYPE", pUnit->unitCombatModifier((UnitCombatTypes)iI), GC.getUnitCombatInfo((UnitCombatTypes) iI).getTextKeyWide());
			}
		}

		for (iI = 0; iI < NUM_DOMAIN_TYPES; iI++)
		{
			if (pUnit->domainModifier((DomainTypes)iI) != 0)
			{
				szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_MOD_VS_TYPE", pUnit->domainModifier((DomainTypes)iI), GC.getDomainInfo((DomainTypes)iI).getTextKeyWide());
			}
		}

		szTempBuffer.clear();
		bFirst = true;
		for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
		{
			if (GC.getUnitInfo(pUnit->getUnitType()).getTargetUnitClass(iI))
			{
				if (bFirst)
				{
					bFirst = false;
				}
				else
				{
					szTempBuffer += L", ";
				}

				szTempBuffer += CvWString::format(L"<link=literal>%s</link>", GC.getUnitClassInfo((UnitClassTypes)iI).getDescription());
			}
		}

		if (!bFirst)
		{
			szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_TARGETS_UNIT_FIRST", szTempBuffer.GetCString());
		}

		szTempBuffer.clear();
		bFirst = true;
		for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
		{
			if (GC.getUnitInfo(pUnit->getUnitType()).getDefenderUnitClass(iI))
			{
				if (bFirst)
				{
					bFirst = false;
				}
				else
				{
					szTempBuffer += L", ";
				}

				szTempBuffer += CvWString::format(L"<link=literal>%s</link>", GC.getUnitClassInfo((UnitClassTypes)iI).getDescription());
			}
		}

		if (!bFirst)
		{
			szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_DEFENDS_UNIT_FIRST", szTempBuffer.GetCString());
		}

		szTempBuffer.clear();
		bFirst = true;
		for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
		{
			if (GC.getUnitInfo(pUnit->getUnitType()).getTargetUnitCombat(iI))
			{
				if (bFirst)
				{
					bFirst = false;
				}
				else
				{
					szTempBuffer += L", ";
				}

				szTempBuffer += CvWString::format(L"<link=literal>%s</link>", GC.getUnitCombatInfo((UnitCombatTypes)iI).getDescription());
			}
		}

		if (!bFirst)
		{
			szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_TARGETS_UNIT_FIRST", szTempBuffer.GetCString());
		}

		szTempBuffer.clear();
		bFirst = true;
		for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
		{
			if (GC.getUnitInfo(pUnit->getUnitType()).getDefenderUnitCombat(iI))
			{
				if (bFirst)
				{
					bFirst = false;
				}
				else
				{
					szTempBuffer += L", ";
				}

				szTempBuffer += CvWString::format(L"<link=literal>%s</link>", GC.getUnitCombatInfo((UnitCombatTypes)iI).getDescription());
			}
		}

		if (!bFirst)
		{
			szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_DEFENDS_UNIT_FIRST", szTempBuffer.GetCString());
		}

		if (pUnit->bombardRate() > 0)
		{
			if (bShort)
			{
				szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_BOMBARD_RATE_SHORT", ((pUnit->bombardRate() * 100) / GC.getMAX_CITY_DEFENSE_DAMAGE()));
			}
			else
			{
				szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_BOMBARD_RATE", ((pUnit->bombardRate() * 100) / GC.getMAX_CITY_DEFENSE_DAMAGE()));
			}
		}

		if (!isEmpty(GC.getUnitInfo(pUnit->getUnitType()).getHelp()))
		{
			szString += NEWLINE;
			szString += (GC.getUnitInfo(pUnit->getUnitType()).getHelp());
		}
	}
}


void CvGameTextMgr::setPlotListHelp(CvWString &szString, CvPlot* pPlot, bool bOneLine, bool bShort)
{
	PROFILE_FUNC();

	CvUnit* pLoopUnit;
	static const uint iMaxNumUnits = 15;
	static std::vector<CvUnit*> apUnits;
	static std::vector<int> aiUnitNumbers;
	static std::vector<int> aiUnitStrength;
	static std::vector<int> aiUnitMaxStrength;

	int iNumVisibleUnits = 0;
	if (pPlot->isVisible(GC.getGameINLINE().getActiveTeam(), GC.getGameINLINE().isDebugMode()))
	{
		for (int i = 0; i < pPlot->getNumUnits(); i++)
		{
			CvUnit* pUnit = pPlot->getUnit(i);
			if (pUnit && !pUnit->isInvisible(GC.getGameINLINE().getActiveTeam(), GC.getGameINLINE().isDebugMode()))
			{
				++iNumVisibleUnits;
			}
		}
	}

	apUnits.erase(apUnits.begin(), apUnits.end());

	if (iNumVisibleUnits > iMaxNumUnits)
	{
		aiUnitNumbers.erase(aiUnitNumbers.begin(), aiUnitNumbers.end());
		aiUnitStrength.erase(aiUnitStrength.begin(), aiUnitStrength.end());
		aiUnitMaxStrength.erase(aiUnitMaxStrength.begin(), aiUnitMaxStrength.end());

		if (m_apbPromotion.size() == 0)
		{
			for (int iI = 0; iI < (GC.getNumUnitInfos() * MAX_PLAYERS); iI++)
			{
				m_apbPromotion.push_back(new int[GC.getNumPromotionInfos()]);
			}
		}

		for (int iI = 0; iI < (GC.getNumUnitInfos() * MAX_PLAYERS); iI++)
		{
			aiUnitNumbers.push_back(0);
			aiUnitStrength.push_back(0);
			aiUnitMaxStrength.push_back(0);
			for (int iJ = 0; iJ < GC.getNumPromotionInfos(); iJ++)
			{
				m_apbPromotion[iI][iJ] = 0;
			}
		}
	}

	int iCount = 0;
	for (int iI = iMaxNumUnits; iI < iNumVisibleUnits; iI++)
	{
		pLoopUnit = gDLL->getInterfaceIFace()->getInterfacePlotUnit(pPlot, iI);

		if (pLoopUnit != NULL && pLoopUnit != pPlot->getCenterUnit())
		{
			apUnits.push_back(pLoopUnit);

			if (iNumVisibleUnits > iMaxNumUnits)
			{
				int iIndex = pLoopUnit->getUnitType() * MAX_PLAYERS + pLoopUnit->getOwner();
				if (aiUnitNumbers[iIndex] == 0)
				{
					++iCount;
				}
				++aiUnitNumbers[iIndex];

				int iBase = (DOMAIN_AIR == pLoopUnit->getDomainType() ? pLoopUnit->airBaseCombatStr() : pLoopUnit->baseCombatStr());
				if (iBase > 0 && pLoopUnit->maxHitPoints() > 0)
				{
					aiUnitMaxStrength[iIndex] += 100 * iBase;
					aiUnitStrength[iIndex] += (100 * iBase * pLoopUnit->currHitPoints()) / pLoopUnit->maxHitPoints();
				}

				for (int iJ = 0; iJ < GC.getNumPromotionInfos(); iJ++)
				{
					if (pLoopUnit->isHasPromotion((PromotionTypes)iJ))
					{
						++m_apbPromotion[iIndex][iJ];
					}
				}
			}
		}
	}


	if (iNumVisibleUnits > 0)
	{
		setUnitHelp(szString, pPlot->getCenterUnit(), iNumVisibleUnits > iMaxNumUnits, true);

		uint iNumShown = min(iMaxNumUnits, iNumVisibleUnits);
		for (uint iI = 0; iI < iNumShown; iI++)
		{
			CvUnit* pLoopUnit = gDLL->getInterfaceIFace()->getInterfacePlotUnit(pPlot, iI);
			if (pLoopUnit != pPlot->getCenterUnit())
			{
				szString += NEWLINE;
				setUnitHelp(szString, pLoopUnit, true, true);
			}
		}

		bool bFirst = true;
		if (iNumVisibleUnits > iMaxNumUnits)
		{
			for (int iI = 0; iI < GC.getNumUnitInfos(); iI++)
			{
				for (int iJ = 0; iJ < MAX_PLAYERS; iJ++)
				{
					int iIndex = iI * MAX_PLAYERS + iJ;

					if (aiUnitNumbers[iIndex] > 0)
					{
						if (iCount < 5 || bFirst)
						{
							szString += NEWLINE;
							bFirst = false;
						}
						else
						{
							szString += L", ";
						}
						szString += CvWString::format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_UNIT_TEXT"), GC.getUnitInfo((UnitTypes)iI).getDescription());

						szString += CvWString::format(L" (%d)", aiUnitNumbers[iIndex]);

						if (aiUnitMaxStrength[iIndex] > 0)
						{
							int iBase = (aiUnitMaxStrength[iIndex] / aiUnitNumbers[iIndex]) / 100;
							int iCurrent = (aiUnitStrength[iIndex] / aiUnitNumbers[iIndex]) / 100;
							int iCurrent100 = (aiUnitStrength[iIndex] / aiUnitNumbers[iIndex]) % 100;
							if (0 == iCurrent100)
							{
								if (iBase == iCurrent)
								{
									szString += CvWString::format(L" %d", iBase);
								}
								else
								{
									szString += CvWString::format(L" %d/%d", iCurrent, iBase);
								}
							}
							else
							{
								szString += CvWString::format(L" %d.%02d/%d", iCurrent, iCurrent100, iBase);
							}
							szString += CvWString::format(L"%c", gDLL->getSymbolID(STRENGTH_CHAR));
						}


						for (int iK = 0; iK < GC.getNumPromotionInfos(); iK++)
						{
							if (m_apbPromotion[iIndex][iK] > 0)
							{
								szString += CvWString::format(L"%d<img=%S size=16></img>", m_apbPromotion[iIndex][iK], GC.getPromotionInfo((PromotionTypes)iK).getButton());
							}
						}

						if (iJ != GC.getGameINLINE().getActivePlayer() && !GC.getUnitInfo((UnitTypes)iI).isAnimal())
						{
							szString += (L", ");
							szString += CvWString::format(SETCOLR L"%s" ENDCOLR, GET_PLAYER((PlayerTypes)iJ).getPlayerTextColorR(), GET_PLAYER((PlayerTypes)iJ).getPlayerTextColorG(), GET_PLAYER((PlayerTypes)iJ).getPlayerTextColorB(), GET_PLAYER((PlayerTypes)iJ).getPlayerTextColorA(), GET_PLAYER((PlayerTypes)iJ).getName());
						}
					}
				}
			}
		}
	}
}


// Returns true if help was given...
bool CvGameTextMgr::setCombatPlotHelp(CvWString &szString, CvPlot* pPlot)
{
	CvUnit* pAttacker;
	CvUnit* pDefender;
	CvWString szTempBuffer;
	CvWString szOffenseOdds;
	CvWString szDefenseOdds;
	bool bValid;
	int iModifier;

	if (gDLL->getInterfaceIFace()->getLengthSelectionList() == 0)
	{
		return false;
	}

	bValid = false;

	switch (gDLL->getInterfaceIFace()->getSelectionList()->getDomainType())
	{
	case DOMAIN_SEA:
		bValid = pPlot->isWater();
		break;

	case DOMAIN_AIR:
		bValid = true;
		break;

	case DOMAIN_LAND:
		bValid = !(pPlot->isWater());
		break;

	case DOMAIN_IMMOBILE:
		break;

	default:
		FAssert(false);
		break;
	}

	if (!bValid)
	{
		return false;
	}

	pAttacker = gDLL->getInterfaceIFace()->getSelectionList()->AI_getBestGroupAttacker(pPlot, false);

	if (pAttacker == NULL)
	{
		pAttacker = gDLL->getInterfaceIFace()->getSelectionList()->AI_getBestGroupAttacker(pPlot, false, true);
	}

	if (pAttacker != NULL)
	{
		pDefender = pPlot->getBestDefender(NO_PLAYER, pAttacker->getOwnerINLINE(), pAttacker, false, NO_TEAM == pAttacker->getDeclareWarMove(pPlot));

		if (pDefender != NULL && pDefender != pAttacker)
		{
			if (pAttacker->getDomainType() != DOMAIN_AIR)
			{
				int iCombatOdds = getCombatOdds(pAttacker, pDefender);
				if (iCombatOdds > 999)
				{
					szTempBuffer = "&gt; 99.9";
				}
				else if (iCombatOdds < 1)
				{
					szTempBuffer = "&lt; 0.1";
				}
				else
				{
					szTempBuffer.Format(L"%.1f", ((float)iCombatOdds) / 10.0f);
				}
				szString += gDLL->getText("TXT_KEY_COMBAT_PLOT_ODDS", szTempBuffer.GetCString());

				if (pAttacker->withdrawalProbability() > 0)
				{
					szTempBuffer.Format(L"%.1f", ((1000 - iCombatOdds) * pAttacker->withdrawalProbability()) / 1000.0f);
					szString += NEWLINE + gDLL->getText("TXT_KEY_COMBAT_PLOT_ODDS_RETREAT", szTempBuffer.GetCString());
				}

				//szTempBuffer.Format(L"AI odds: %d%%", pAttacker->AI_attackOdds(pPlot, true));
				//szString += NEWLINE + szTempBuffer;
			}

			szOffenseOdds.Format(L"%.2f", ((pAttacker->getDomainType() == DOMAIN_AIR) ? pAttacker->airCurrCombatStrFloat() : pAttacker->currCombatStrFloat(NULL, NULL)));
			szDefenseOdds.Format(L"%.2f", pDefender->currCombatStrFloat(pPlot, pAttacker));
			szString += NEWLINE + gDLL->getText("TXT_KEY_COMBAT_PLOT_ODDS_VS", szOffenseOdds.GetCString(), szDefenseOdds.GetCString());

			szString+=L" ";//XXX

			szString += gDLL->getText("TXT_KEY_COLOR_POSITIVE");

			szString+=L" ";//XXX

			iModifier = pAttacker->getExtraCombatPercent();

			if (iModifier != 0)
			{
				szString += NEWLINE + gDLL->getText("TXT_KEY_COMBAT_PLOT_EXTRA_STRENGTH", iModifier);
			}

			iModifier = pAttacker->unitClassAttackModifier(pDefender->getUnitClassType());

			if (iModifier != 0)
			{
				szString += NEWLINE + gDLL->getText("TXT_KEY_COMBAT_PLOT_MOD_VS_TYPE", iModifier, GC.getUnitClassInfo(pDefender->getUnitClassType()).getTextKeyWide());
			}

			if (pDefender->getUnitCombatType() != NO_UNITCOMBAT)
			{
				iModifier = pAttacker->unitCombatModifier(pDefender->getUnitCombatType());

				if (iModifier != 0)
				{
					szString += NEWLINE + gDLL->getText("TXT_KEY_COMBAT_PLOT_MOD_VS_TYPE", iModifier, GC.getUnitCombatInfo(pDefender->getUnitCombatType()).getTextKeyWide());
				}
			}

			iModifier = pAttacker->domainModifier(pDefender->getDomainType());

			if (iModifier != 0)
			{
				szString += NEWLINE + gDLL->getText("TXT_KEY_COMBAT_PLOT_MOD_VS_TYPE", iModifier, GC.getDomainInfo(pDefender->getDomainType()).getTextKeyWide());
			}

			if (pPlot->isCity(true))
			{
				iModifier = pAttacker->cityAttackModifier();

				if (iModifier != 0)
				{
					szString += NEWLINE + gDLL->getText("TXT_KEY_COMBAT_PLOT_CITY_MOD", iModifier);
				}
			}

			if (pPlot->isHills())
			{
				iModifier = pAttacker->hillsAttackModifier();

				if (iModifier != 0)
				{
					szString += NEWLINE + gDLL->getText("TXT_KEY_COMBAT_PLOT_HILLS_MOD", iModifier);
				}
			}

			iModifier = pAttacker->getKamikazePercent();
			if (iModifier != 0)
			{
				szString += NEWLINE + gDLL->getText("TXT_KEY_COMBAT_KAMIKAZE_MOD", iModifier);
			}

			if (pDefender->isAnimal())
			{
				iModifier = -GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAnimalCombatModifier();

				iModifier += GC.getUnitInfo(pAttacker->getUnitType()).getAnimalCombatModifier();

				if (iModifier != 0)
				{
					szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_ANIMAL_COMBAT_MOD", iModifier);
				}
			}

			if (pDefender->isBarbarian())
			{
				iModifier = -GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getBarbarianCombatModifier();

				if (iModifier != 0)
				{
					szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_BARBARIAN_COMBAT_MOD", iModifier);
				}
			}

			if (!(pDefender->immuneToFirstStrikes()))
			{
				if (pAttacker->maxFirstStrikes() > 0)
				{
					if (pAttacker->firstStrikes() == pAttacker->maxFirstStrikes())
					{
						if (pAttacker->firstStrikes() == 1)
						{
							szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_ONE_FIRST_STRIKE");
						}
						else
						{
							szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_NUM_FIRST_STRIKES", pAttacker->firstStrikes());
						}
					}
					else
					{
						szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_FIRST_STRIKE_CHANCES", pAttacker->firstStrikes(), pAttacker->maxFirstStrikes());
					}
				}
			}

			if (pAttacker->isHurt())
			{
				szString += NEWLINE + gDLL->getText("TXT_KEY_COMBAT_PLOT_HP", pAttacker->currHitPoints(), pAttacker->maxHitPoints());
			}

			szString+=gDLL->getText("TXT_KEY_COLOR_REVERT");

			szString+=L" ";//XXX

			szString+=gDLL->getText("TXT_KEY_COLOR_NEGATIVE");

			szString+=L" ";//XXX

			if (!(pAttacker->isRiver()))
			{
				if (pAttacker->plot()->isRiverCrossing(directionXY(pAttacker->plot(), pPlot)))
				{
					iModifier = GC.getRIVER_ATTACK_MODIFIER();

					if (iModifier != 0)
					{
						szString += NEWLINE + gDLL->getText("TXT_KEY_COMBAT_PLOT_RIVER_MOD", -(iModifier));
					}
				}
			}

			if (!(pAttacker->isAmphib()))
			{
				if (!(pPlot->isWater()) && pAttacker->plot()->isWater())
				{
					iModifier = GC.getAMPHIB_ATTACK_MODIFIER();

					if (iModifier != 0)
					{
						szString += NEWLINE + gDLL->getText("TXT_KEY_COMBAT_PLOT_AMPHIB_MOD", -(iModifier));
					}
				}
			}

			iModifier = pDefender->getExtraCombatPercent();

			if (iModifier != 0)
			{
				szString += NEWLINE + gDLL->getText("TXT_KEY_COMBAT_PLOT_EXTRA_STRENGTH", iModifier);
			}

			iModifier = pDefender->unitClassDefenseModifier(pAttacker->getUnitClassType());

			if (iModifier != 0)
			{
				szString += NEWLINE + gDLL->getText("TXT_KEY_COMBAT_PLOT_MOD_VS_TYPE", iModifier, GC.getUnitClassInfo(pAttacker->getUnitClassType()).getTextKeyWide());
			}

			if (pAttacker->getUnitCombatType() != NO_UNITCOMBAT)
			{
				iModifier = pDefender->unitCombatModifier(pAttacker->getUnitCombatType());

				if (iModifier != 0)
				{
					szString += NEWLINE + gDLL->getText("TXT_KEY_COMBAT_PLOT_MOD_VS_TYPE", iModifier, GC.getUnitCombatInfo(pAttacker->getUnitCombatType()).getTextKeyWide());
				}
			}

			iModifier = pDefender->domainModifier(pAttacker->getDomainType());

			if (iModifier != 0)
			{
				szString += NEWLINE + gDLL->getText("TXT_KEY_COMBAT_PLOT_MOD_VS_TYPE", iModifier, GC.getDomainInfo(pAttacker->getDomainType()).getTextKeyWide());
			}

			if (!(pDefender->noDefensiveBonus()))
			{
				iModifier = pPlot->defenseModifier(pDefender->getTeam(), (pAttacker != NULL) ? pAttacker->ignoreBuildingDefense() : true);

				if (iModifier != 0)
				{
					szString += NEWLINE + gDLL->getText("TXT_KEY_COMBAT_PLOT_TILE_MOD", iModifier);
				}
			}

			iModifier = pDefender->fortifyModifier();

			if (iModifier != 0)
			{
				szString += NEWLINE + gDLL->getText("TXT_KEY_COMBAT_PLOT_FORTIFY_MOD", iModifier);
			}

			if (pPlot->isCity(true))
			{
				iModifier = pDefender->cityDefenseModifier();

				if (iModifier != 0)
				{
					szString += NEWLINE + gDLL->getText("TXT_KEY_COMBAT_PLOT_CITY_MOD", iModifier);
				}
			}

			if (pPlot->isHills())
			{
				iModifier = pDefender->hillsDefenseModifier();

				if (iModifier != 0)
				{
					szString += NEWLINE + gDLL->getText("TXT_KEY_COMBAT_PLOT_HILLS_MOD", iModifier);
				}
			}

			if (pPlot->getFeatureType() != NO_FEATURE)
			{
				iModifier = pDefender->featureDefenseModifier(pPlot->getFeatureType());

				if (iModifier != 0)
				{
					szString += NEWLINE + gDLL->getText("TXT_KEY_COMBAT_PLOT_UNIT_MOD", iModifier, GC.getFeatureInfo(pPlot->getFeatureType()).getTextKeyWide());
				}
			}
			else
			{
				iModifier = pDefender->terrainDefenseModifier(pPlot->getTerrainType());

				if (iModifier != 0)
				{
					szString += NEWLINE + gDLL->getText("TXT_KEY_COMBAT_PLOT_UNIT_MOD", iModifier, GC.getTerrainInfo(pPlot->getTerrainType()).getTextKeyWide());
				}
			}

			if (!(pAttacker->immuneToFirstStrikes()))
			{
				if (pDefender->maxFirstStrikes() > 0)
				{
					if (pDefender->firstStrikes() == pDefender->maxFirstStrikes())
					{
						if (pDefender->firstStrikes() == 1)
						{
							szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_ONE_FIRST_STRIKE");
						}
						else
						{
							szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_NUM_FIRST_STRIKES", pDefender->firstStrikes());
						}
					}
					else
					{
						szString += NEWLINE + gDLL->getText("TXT_KEY_UNIT_FIRST_STRIKE_CHANCES", pDefender->firstStrikes(), pDefender->maxFirstStrikes());
					}
				}
			}

			if (pDefender->isHurt())
			{
				szString += NEWLINE + gDLL->getText("TXT_KEY_COMBAT_PLOT_HP", pDefender->currHitPoints(), pDefender->maxHitPoints());
			}

			szString+=gDLL->getText("TXT_KEY_COLOR_REVERT");

			return true;
		}
	}

	return false;
}

// DO NOT REMOVE - needed for font testing - Moose
void createTestFontString(CvWString& szString)
{
	int iI;
	szString = L"!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[×]^_`abcdefghijklmnopqrstuvwxyz\n";
	szString += L"{}~\\ßÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞŸßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ¿¡«»°ŠŒŽšœž™©®€£¢”‘“…’";
	for (iI=0;iI<NUM_YIELD_TYPES;iI++)
		szString = szString + CvWString::format(L"%c", GC.getYieldInfo((YieldTypes) iI).getChar());

	szString += L"\n";
	for (iI=0;iI<NUM_COMMERCE_TYPES;iI++)
		szString = szString + CvWString::format(L"%c", GC.getCommerceInfo((CommerceTypes) iI).getChar());
	szString += L"\n";
	for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
	{
		szString = szString + CvWString::format(L"%c", GC.getReligionInfo((ReligionTypes) iI).getChar());
		szString = szString + CvWString::format(L"%c", GC.getReligionInfo((ReligionTypes) iI).getHolyCityChar());
	}
	szString += L"\n";
	for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
		szString = szString + CvWString::format(L"%c", GC.getBonusInfo((BonusTypes) iI).getChar());
	for (iI=0; iI<MAX_NUM_SYMBOLS; iI++)
		szString = szString + CvWString::format(L"%c", gDLL->getSymbolID(iI));
}

void CvGameTextMgr::setPlotHelp(CvWString& szString, CvPlot* pPlot)
{
	int iI;

	// DO NOT REMOVE - needed for font testing - Moose
	if (gDLL->getTestingFont())
	{
		createTestFontString(szString);
		return;
	}

	CvWString szTempBuffer;
	ImprovementTypes eImprovement;
	PlayerTypes eRevealOwner;
	BonusTypes eBonus;
	bool bShift;
	bool bFound;
	int iDefenseModifier;
	int iYield;
	int iTurns;

	bShift = gDLL->shiftKey();

	if (bShift && (gDLL->getChtLvl() > 0))
	{
		szString+=(GC.getTerrainInfo(pPlot->getTerrainType()).getDescription());

		FAssert((0 < GC.getNumBonusInfos()) && "GC.getNumBonusInfos() is not greater than zero but an array is being allocated in CvInterface::updateHelpStrings");
		for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
		{
			if (pPlot->isPlotGroupConnectedBonus(GC.getGameINLINE().getActivePlayer(), ((BonusTypes)iI)))
			{
				szString+=NEWLINE;
				szString+=(GC.getBonusInfo((BonusTypes)iI).getDescription());
			}
		}

		if (pPlot->getPlotGroup(GC.getGameINLINE().getActivePlayer()) != NULL)
		{
			szTempBuffer.Format(L"\n(%d, %d) group: %d", pPlot->getX_INLINE(), pPlot->getY_INLINE(), pPlot->getPlotGroup(GC.getGameINLINE().getActivePlayer())->getID());
		}
		else
		{
			szTempBuffer.Format(L"\n(%d, %d) group: (-1, -1)", pPlot->getX_INLINE(), pPlot->getY_INLINE());
		}
		szString+=szTempBuffer;

		szTempBuffer.Format(L"\nArea: %d", pPlot->getArea());
		szString+=szTempBuffer;

		char tempChar = 'x';
		if(pPlot->getRiverNSDirection() == CARDINALDIRECTION_NORTH)
		{
			tempChar = 'N';
		}
		else if(pPlot->getRiverNSDirection() == CARDINALDIRECTION_SOUTH)
		{
			tempChar = 'S';
		}
		szTempBuffer.Format(L"\nNSRiverFlow: %c", tempChar);
		szString+=szTempBuffer;

		tempChar = 'x';
		if(pPlot->getRiverWEDirection() == CARDINALDIRECTION_WEST)
		{
			tempChar = 'W';
		}
		else if(pPlot->getRiverWEDirection() == CARDINALDIRECTION_EAST)
		{
			tempChar = 'E';
		}
		szTempBuffer.Format(L"\nWERiverFlow: %c", tempChar);
		szString+=szTempBuffer;

		if(pPlot->getRouteType() != NO_ROUTE)
		{
			szTempBuffer.Format(L"\nRoute: %s", GC.getRouteInfo(pPlot->getRouteType()).getDescription());
			szString+=szTempBuffer;
		}

		if(pPlot->getRouteSymbol() != NULL)
		{
			szTempBuffer.Format(L"\nConnection: %i", gDLL->getRouteIFace()->getConnectionMask(pPlot->getRouteSymbol()));
			szString+=szTempBuffer;
		}

		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if (pPlot->getCulture((PlayerTypes)iI) > 0)
				{
					szTempBuffer.Format(L"\n%s Culture: %d", GET_PLAYER((PlayerTypes)iI).getName(), pPlot->getCulture((PlayerTypes)iI));
					szString+=szTempBuffer;
				}
			}
		}

		szTempBuffer.Format(L"\nFound Value: %d, %d", GET_PLAYER(GC.getGameINLINE().getActivePlayer()).AI_foundValue(pPlot->getX_INLINE(), pPlot->getY_INLINE(), -1 , true), GET_PLAYER(GC.getGameINLINE().getActivePlayer()).AI_foundValue(pPlot->getX_INLINE(), pPlot->getY_INLINE(), -1 , false));
		szString+=szTempBuffer;

		CvCity* pWorkingCity = pPlot->getWorkingCity();
		if (NULL != pWorkingCity)
		{
			for (iI = 0; iI < NUM_CITY_PLOTS; ++iI)
			{
				if (pPlot == plotCity(pWorkingCity->getX_INLINE(), pWorkingCity->getY_INLINE(), iI))
				{
					int iBuildValue = pWorkingCity->AI_getBestBuildValue(iI);
					BuildTypes eBestBuild = pWorkingCity->AI_getBestBuild(iI);

					if (NO_BUILD != eBestBuild)
					{
						szTempBuffer.Format(L"\n Best Build: %s (%d)", GC.getBuildInfo(eBestBuild).getDescription(), iBuildValue);
						szString += szTempBuffer;
					}

					break;
				}
			}
		}
	}
	else
	{
		eRevealOwner = pPlot->getRevealedOwner(GC.getGameINLINE().getActiveTeam(), true);

		if (eRevealOwner != NO_PLAYER)
		{
			if (pPlot->isActiveVisible(true))
			{
				szTempBuffer.Format(L"%d%% " SETCOLR L"%s" ENDCOLR, pPlot->calculateCulturePercent(eRevealOwner), GET_PLAYER(eRevealOwner).getPlayerTextColorR(), GET_PLAYER(eRevealOwner).getPlayerTextColorG(), GET_PLAYER(eRevealOwner).getPlayerTextColorB(), GET_PLAYER(eRevealOwner).getPlayerTextColorA(), GET_PLAYER(eRevealOwner).getCivilizationAdjective());
				szString += szTempBuffer + NEWLINE;
			}
			else
			{
				szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, GET_PLAYER(eRevealOwner).getPlayerTextColorR(), GET_PLAYER(eRevealOwner).getPlayerTextColorG(), GET_PLAYER(eRevealOwner).getPlayerTextColorB(), GET_PLAYER(eRevealOwner).getPlayerTextColorA(), GET_PLAYER(eRevealOwner).getCivilizationDescription());
				szString += szTempBuffer + NEWLINE;
			}
		}

		iDefenseModifier = pPlot->defenseModifier((eRevealOwner != NO_PLAYER ? GET_PLAYER(eRevealOwner).getTeam() : NO_TEAM), true, true);

		if (iDefenseModifier != 0)
		{
			szString += gDLL->getText("TXT_KEY_PLOT_BONUS", iDefenseModifier) + NEWLINE;
		}

		if (pPlot->getTerrainType() != NO_TERRAIN)
		{
			if (pPlot->isPeak())
			{
				szString += gDLL->getText("TXT_KEY_PLOT_PEAK");
			}
			else
			{
				if (pPlot->isWater())
				{
					szTempBuffer.Format(SETCOLR, TEXT_COLOR("COLOR_WATER_TEXT"));
					szString+=szTempBuffer;
				}

				if (pPlot->isHills())
				{
					szString += gDLL->getText("TXT_KEY_PLOT_HILLS");
				}

				if (pPlot->getFeatureType() != NO_FEATURE)
				{
					szTempBuffer.Format(L"%s/", GC.getFeatureInfo(pPlot->getFeatureType()).getDescription());
					szString+=szTempBuffer;
				}

				szString+=(GC.getTerrainInfo(pPlot->getTerrainType()).getDescription());

				if (pPlot->isWater())
				{
					szString+=ENDCOLR;
				}
			}
		}

		if (pPlot->hasYield())
		{
			for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
			{
				iYield = pPlot->calculateYield(((YieldTypes)iI), true);

				if (iYield != 0)
				{
					szTempBuffer.Format(L", %d%c", iYield, GC.getYieldInfo((YieldTypes) iI).getChar());
					szString+=szTempBuffer;
				}
			}
		}

		if (pPlot->isFreshWater())
		{
			szString += NEWLINE + gDLL->getText("TXT_KEY_PLOT_FRESH_WATER");
		}

		if (pPlot->isLake())
		{
			szString += NEWLINE + gDLL->getText("TXT_KEY_PLOT_FRESH_WATER_LAKE");
		}

		if (pPlot->isImpassable())
		{
			szString += NEWLINE + gDLL->getText("TXT_KEY_PLOT_IMPASSABLE");
		}

		if (GC.getGameINLINE().isDebugMode())
		{
			eBonus = pPlot->getBonusType();
		}
		else
		{
			eBonus = pPlot->getBonusType(GC.getGameINLINE().getActiveTeam());
		}

		if (eBonus != NO_BONUS)
		{
			szTempBuffer.Format(L"%c " SETCOLR L"%s" ENDCOLR, GC.getBonusInfo(eBonus).getChar(), TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getBonusInfo(eBonus).getDescription());
			szString += NEWLINE + szTempBuffer;

			if (GC.getBonusInfo(eBonus).getHealth() != 0)
			{
				szTempBuffer.Format(L", +%d%c", abs(GC.getBonusInfo(eBonus).getHealth()), ((GC.getBonusInfo(eBonus).getHealth() > 0) ? gDLL->getSymbolID(HEALTHY_CHAR): gDLL->getSymbolID(UNHEALTHY_CHAR)));
				szString+=szTempBuffer;
			}

			if (GC.getBonusInfo(eBonus).getHappiness() != 0)
			{
				szTempBuffer.Format(L", +%d%c", abs(GC.getBonusInfo(eBonus).getHappiness()), ((GC.getBonusInfo(eBonus).getHappiness() > 0) ? gDLL->getSymbolID(HAPPY_CHAR): gDLL->getSymbolID(UNHAPPY_CHAR)));
				szString+=szTempBuffer;
			}

			if ((pPlot->getImprovementType() == NO_IMPROVEMENT) || !(GC.getImprovementInfo(pPlot->getImprovementType()).isImprovementBonusTrade(eBonus)))
			{
				if (!(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isHasTech((TechTypes)GC.getBonusInfo(eBonus).getTechCityTrade())))
				{
					szString += gDLL->getText("TXT_KEY_PLOT_RESEARCH", GC.getTechInfo((TechTypes) GC.getBonusInfo(eBonus).getTechCityTrade()).getTextKeyWide());
				}

				if (!pPlot->isCity())
				{
					for (iI = 0; iI < GC.getNumBuildInfos(); iI++)
					{
						if (GC.getBuildInfo((BuildTypes) iI).getImprovement() != NO_IMPROVEMENT)
						{
							CvImprovementInfo& kImprovementInfo = GC.getImprovementInfo((ImprovementTypes) GC.getBuildInfo((BuildTypes) iI).getImprovement());
							if (kImprovementInfo.isImprovementBonusTrade(eBonus))
							{
								if (pPlot->canHaveImprovement(((ImprovementTypes)(GC.getBuildInfo((BuildTypes) iI).getImprovement())), GC.getGameINLINE().getActiveTeam(), true))
								{
									if (GET_TEAM(GC.getGameINLINE().getActiveTeam()).isHasTech((TechTypes)GC.getBuildInfo((BuildTypes) iI).getTechPrereq()))
									{
										szString += gDLL->getText("TXT_KEY_PLOT_REQUIRES", kImprovementInfo.getTextKeyWide());
									}
									else if (GC.getBonusInfo(eBonus).getTechCityTrade() != GC.getBuildInfo((BuildTypes) iI).getTechPrereq())
									{
										szString += gDLL->getText("TXT_KEY_PLOT_RESEARCH", GC.getTechInfo((TechTypes) GC.getBuildInfo((BuildTypes) iI).getTechPrereq()).getTextKeyWide());
									}

									bool bFirst = true;

									for (int k = 0; k < NUM_YIELD_TYPES; k++)
									{
										int iYieldChange = kImprovementInfo.getImprovementBonusYield(eBonus, k) + kImprovementInfo.getYieldChange(k);
										if (iYieldChange != 0)
										{
											if (iYieldChange > 0)
											{
												szTempBuffer.Format(L"+%d%c", iYieldChange, GC.getYieldInfo((YieldTypes)k).getChar());
											}
											else
											{
												szTempBuffer.Format(L"%d%c", iYieldChange, GC.getYieldInfo((YieldTypes)k).getChar());
											}
											setListHelp(szString, L"\n", szTempBuffer, L", ", bFirst);
											bFirst = false;
										}
									}

									if (!bFirst)
									{
										szString += gDLL->getText("TXT_KEY_BONUS_WITH_IMPROVEMENT", kImprovementInfo.getTextKeyWide());
									}

									break;
								}
							}
						}
					}
				}
			}
			else if (!(pPlot->isBonusNetwork(GC.getGameINLINE().getActiveTeam())))
			{
				szString += gDLL->getText("TXT_KEY_PLOT_REQUIRES_ROUTE");
			}
		}

		eImprovement = pPlot->getRevealedImprovementType(GC.getGameINLINE().getActiveTeam(), true);

		if (eImprovement != NO_IMPROVEMENT)
		{
			szString+= NEWLINE;
			szString+=(GC.getImprovementInfo(eImprovement).getDescription());

			bFound = false;

			for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
			{
				if (GC.getImprovementInfo(eImprovement).getIrrigatedYieldChange(iI) != 0)
				{
					bFound = true;
					break;
				}
			}

			if (bFound)
			{
				if (pPlot->isIrrigationAvailable())
				{
					szString += gDLL->getText("TXT_KEY_PLOT_IRRIGATED");
				}
				else
				{
					szString += gDLL->getText("TXT_KEY_PLOT_NOT_IRRIGATED");
				}
			}

			if (GC.getImprovementInfo(eImprovement).getImprovementUpgrade() != NO_IMPROVEMENT)
			{
				if ((pPlot->getUpgradeProgress() > 0) || pPlot->isBeingWorked())
				{
					iTurns = pPlot->getUpgradeTimeLeft(eImprovement, eRevealOwner);

					szString += gDLL->getText("TXT_KEY_PLOT_IMP_UPGRADE", iTurns, GC.getImprovementInfo((ImprovementTypes) GC.getImprovementInfo(eImprovement).getImprovementUpgrade()).getTextKeyWide());
				}
				else
				{
					szString += gDLL->getText("TXT_KEY_PLOT_WORK_TO_UPGRADE", GC.getImprovementInfo((ImprovementTypes) GC.getImprovementInfo(eImprovement).getImprovementUpgrade()).getTextKeyWide());
				}
			}
		}

		if (pPlot->getRevealedRouteType(GC.getGameINLINE().getActiveTeam(), true) != NO_ROUTE)
		{
			szString += NEWLINE;
			szString += (GC.getRouteInfo(pPlot->getRevealedRouteType(GC.getGameINLINE().getActiveTeam(), true)).getDescription());
		}
	}
}


void CvGameTextMgr::setCityBarHelp(CvWString &szString, CvCity* pCity)
{
	CvWString szTempBuffer;
	bool bFirst;
	int iFoodDifference;
	int iProductionDiffNoFood;
	int iProductionDiffJustFood;
	int iRate;
	int iI;

	iFoodDifference = pCity->foodDifference();

	szString += pCity->getName();

	if (iFoodDifference <= 0)
	{
		szString += gDLL->getText("TXT_KEY_CITY_BAR_GROWTH", pCity->getFood(), pCity->growthThreshold());
	}
	else
	{
		szString += gDLL->getText("TXT_KEY_CITY_BAR_FOOD_GROWTH", iFoodDifference, pCity->getFood(), pCity->growthThreshold(), pCity->getFoodTurnsLeft());
	}
	if (pCity->getProductionNeeded() != MAX_INT)
	{
		iProductionDiffNoFood = pCity->getCurrentProductionDifference(true, true);
		iProductionDiffJustFood = (pCity->getCurrentProductionDifference(false, true) - iProductionDiffNoFood);

		if (iProductionDiffJustFood > 0)
		{
			szString += gDLL->getText("TXT_KEY_CITY_BAR_FOOD_HAMMER_PRODUCTION", iProductionDiffJustFood, iProductionDiffNoFood, pCity->getProductionName(), pCity->getProduction(), pCity->getProductionNeeded(), pCity->getProductionTurnsLeft());
		}
		else if (iProductionDiffNoFood > 0)
		{
			szString += gDLL->getText("TXT_KEY_CITY_BAR_HAMMER_PRODUCTION", iProductionDiffNoFood, pCity->getProductionName(), pCity->getProduction(), pCity->getProductionNeeded(), pCity->getProductionTurnsLeft());
		}
		else
		{
			szString += gDLL->getText("TXT_KEY_CITY_BAR_PRODUCTION", pCity->getProductionName(), pCity->getProduction(), pCity->getProductionNeeded());
		}
	}

	bFirst = true;

	for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		iRate = pCity->getCommerceRateTimes100((CommerceTypes)iI);

		if (iRate != 0)
		{
			szTempBuffer.Format(L"%d.%02d %c", iRate/100, iRate%100, GC.getCommerceInfo((CommerceTypes)iI).getChar());
			setListHelp(szString, NEWLINE, szTempBuffer, L", ", bFirst);
			bFirst = false;
		}
	}

	iRate = pCity->getGreatPeopleRate();

	if (iRate != 0)
	{
		szTempBuffer.Format(L"%d%c", iRate, gDLL->getSymbolID(GREAT_PEOPLE_CHAR));
		setListHelp(szString, NEWLINE, szTempBuffer, L", ", bFirst);
		bFirst = false;
	}

	if (!bFirst)
	{
		szString += gDLL->getText("TXT_KEY_PER_TURN");
	}

	if (pCity->getCultureLevel() != NO_CULTURELEVEL)
	{
		szString += gDLL->getText("TXT_KEY_CITY_BAR_CULTURE", pCity->getCulture(pCity->getOwnerINLINE()), pCity->getCultureThreshold(), GC.getCultureLevelInfo(pCity->getCultureLevel()).getTextKeyWide());
	}

	if (pCity->getGreatPeopleProgress() > 0)
	{
		szString += gDLL->getText("TXT_KEY_CITY_BAR_GREAT_PEOPLE", pCity->getGreatPeopleProgress(), GET_PLAYER(pCity->getOwnerINLINE()).greatPeopleThreshold(false));
	}

	szString += NEWLINE;

	szString += gDLL->getText("TXT_KEY_CITY_BAR_SELECT", pCity->getNameKey());
	szString += gDLL->getText("TXT_KEY_CITY_BAR_SELECT_CTRL");
	szString += gDLL->getText("TXT_KEY_CITY_BAR_SELECT_ALT");
}


void CvGameTextMgr::parseTraits(CvWString &szHelpString, TraitTypes eTrait, CivilizationTypes eCivilization, bool bDawnOfMan)
{
	CvWString szTempBuffer;
	BuildingTypes eLoopBuilding;
	UnitTypes eLoopUnit;
	int iLast;
	int iI, iJ;
	CvWString szText;

	if (eCivilization == NO_CIVILIZATION)
	{
		eCivilization = GC.getGameINLINE().getActiveCivilizationType();
	}

	// Trait Name
	szText = GC.getTraitInfo(eTrait).getDescription();
	if (bDawnOfMan)
	{
		szTempBuffer.Format(L"%s", szText.GetCString());
	}
	else
	{
		szTempBuffer.Format(NEWLINE SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_ALT_HIGHLIGHT_TEXT"), szText.GetCString());
	}
	szHelpString+=szTempBuffer;

	if (!bDawnOfMan)
	{
		// iHealth
		if (GC.getTraitInfo(eTrait).getHealth() != 0)
		{
			szHelpString += gDLL->getText("TXT_KEY_TRAIT_HEALTH", GC.getTraitInfo(eTrait).getHealth());
		}

		// iHappiness
		if (GC.getTraitInfo(eTrait).getHappiness() != 0)
		{
			szHelpString += gDLL->getText("TXT_KEY_TRAIT_HAPPINESS", GC.getTraitInfo(eTrait).getHappiness());
		}

		// iMaxAnarchy
		if (GC.getTraitInfo(eTrait).getMaxAnarchy() != -1)
		{
			if (GC.getTraitInfo(eTrait).getMaxAnarchy() == 0)
			{
				szHelpString += gDLL->getText("TXT_KEY_TRAIT_NO_ANARCHY");
			}
			else
			{
				szHelpString += gDLL->getText("TXT_KEY_TRAIT_MAX_ANARCHY", GC.getTraitInfo(eTrait).getMaxAnarchy());
			}
		}

		// iUpkeepModifier
		if (GC.getTraitInfo(eTrait).getUpkeepModifier() != 0)
		{
			szHelpString += gDLL->getText("TXT_KEY_TRAIT_CIVIC_UPKEEP_MODIFIER", GC.getTraitInfo(eTrait).getUpkeepModifier());
		}

		// iLevelExperienceModifier
		if (GC.getTraitInfo(eTrait).getLevelExperienceModifier() != 0)
		{
			szHelpString += gDLL->getText("TXT_KEY_TRAIT_CIVIC_LEVEL_MODIFIER", GC.getTraitInfo(eTrait).getLevelExperienceModifier());
		}

		// iGreatPeopleRateModifier
		if (GC.getTraitInfo(eTrait).getGreatPeopleRateModifier() != 0)
		{
			szHelpString += gDLL->getText("TXT_KEY_TRAIT_GREAT_PEOPLE_MODIFIER", GC.getTraitInfo(eTrait).getGreatPeopleRateModifier());
		}

		// iGreatGeneralRateModifier
		if (GC.getTraitInfo(eTrait).getGreatGeneralRateModifier() != 0)
		{
			szHelpString += gDLL->getText("TXT_KEY_TRAIT_GREAT_GENERAL_MODIFIER", GC.getTraitInfo(eTrait).getGreatGeneralRateModifier());
		}

		if (GC.getTraitInfo(eTrait).getDomesticGreatGeneralRateModifier() != 0)
		{
			szHelpString += gDLL->getText("TXT_KEY_TRAIT_DOMESTIC_GREAT_GENERAL_MODIFIER", GC.getTraitInfo(eTrait).getDomesticGreatGeneralRateModifier());
		}

		// Wonder Production Effects
		if ((GC.getTraitInfo(eTrait).getMaxGlobalBuildingProductionModifier() != 0)
			|| (GC.getTraitInfo(eTrait).getMaxTeamBuildingProductionModifier() != 0)
			|| (GC.getTraitInfo(eTrait).getMaxPlayerBuildingProductionModifier() != 0))
		{
			if ((GC.getTraitInfo(eTrait).getMaxGlobalBuildingProductionModifier() == GC.getTraitInfo(eTrait).getMaxTeamBuildingProductionModifier())
				&& 	(GC.getTraitInfo(eTrait).getMaxGlobalBuildingProductionModifier() == GC.getTraitInfo(eTrait).getMaxPlayerBuildingProductionModifier()))
			{
				szHelpString += gDLL->getText("TXT_KEY_TRAIT_WONDER_PRODUCTION_MODIFIER", GC.getTraitInfo(eTrait).getMaxGlobalBuildingProductionModifier());
			}
			else
			{
				if (GC.getTraitInfo(eTrait).getMaxGlobalBuildingProductionModifier() != 0)
				{
					szHelpString += gDLL->getText("TXT_KEY_TRAIT_WORLD_WONDER_PRODUCTION_MODIFIER", GC.getTraitInfo(eTrait).getMaxGlobalBuildingProductionModifier());
				}

				if (GC.getTraitInfo(eTrait).getMaxTeamBuildingProductionModifier() != 0)
				{
					szHelpString += gDLL->getText("TXT_KEY_TRAIT_TEAM_WONDER_PRODUCTION_MODIFIER", GC.getTraitInfo(eTrait).getMaxTeamBuildingProductionModifier());
				}

				if (GC.getTraitInfo(eTrait).getMaxPlayerBuildingProductionModifier() != 0)
				{
					szHelpString += gDLL->getText("TXT_KEY_TRAIT_NATIONAL_WONDER_PRODUCTION_MODIFIER", GC.getTraitInfo(eTrait).getMaxPlayerBuildingProductionModifier());
				}
			}
		}

		// ExtraYieldThresholds
		for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
		{
			if (GC.getTraitInfo(eTrait).getExtraYieldThreshold(iI) > 0)
			{
				szHelpString += gDLL->getText("TXT_KEY_TRAIT_EXTRA_YIELD_THRESHOLDS", GC.getYieldInfo((YieldTypes) iI).getChar(), GC.getTraitInfo(eTrait).getExtraYieldThreshold(iI), GC.getYieldInfo((YieldTypes) iI).getChar());
			}
			// Trade Yield Modifiers
			if (GC.getTraitInfo(eTrait).getTradeYieldModifier(iI) != 0)
			{
				szHelpString += gDLL->getText("TXT_KEY_TRAIT_TRADE_YIELD_MODIFIERS", GC.getTraitInfo(eTrait).getTradeYieldModifier(iI), GC.getYieldInfo((YieldTypes) iI).getChar(), "YIELD");
			}
		}

		// CommerceChanges
		for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
		{
			if (GC.getTraitInfo(eTrait).getCommerceChange(iI) != 0)
			{
				szHelpString += gDLL->getText("TXT_KEY_TRAIT_COMMERCE_CHANGES", 	GC.getTraitInfo(eTrait).getCommerceChange(iI), GC.getCommerceInfo((CommerceTypes) iI).getChar(), "COMMERCE");
			}

			if (GC.getTraitInfo(eTrait).getCommerceModifier(iI) != 0)
			{
				szHelpString += gDLL->getText("TXT_KEY_TRAIT_COMMERCE_MODIFIERS", GC.getTraitInfo(eTrait).getCommerceModifier(iI), GC.getCommerceInfo((CommerceTypes) iI).getChar(), "COMMERCE");
			}
		}

		// Free Promotions
		bool bFoundPromotion = false;
		szTempBuffer.clear();
		for (iI = 0; iI < GC.getNumPromotionInfos(); iI++)
		{
			if (GC.getTraitInfo(eTrait).isFreePromotion(iI))
			{
				if (bFoundPromotion)
				{
					szTempBuffer += L", ";
				}

				szTempBuffer += CvWString::format(L"<link=literal>%s</link>", GC.getPromotionInfo((PromotionTypes) iI).getDescription());
				bFoundPromotion = true;
			}
		}

		if (bFoundPromotion)
		{
			szHelpString += gDLL->getText("TXT_KEY_TRAIT_FREE_PROMOTIONS", szTempBuffer.GetCString());

			for (iJ = 0; iJ < GC.getNumUnitCombatInfos(); iJ++)
			{
				if (GC.getTraitInfo(eTrait).isFreePromotionUnitCombat(iJ))
				{
					szTempBuffer.Format(L"\n        %c<link=literal>%s</link>", gDLL->getSymbolID(BULLET_CHAR), GC.getUnitCombatInfo((UnitCombatTypes)iJ).getDescription());
					szHelpString += szTempBuffer;
				}
			}
		}

		// No Civic Maintenance
		for (iI = 0; iI < GC.getNumCivicOptionInfos(); iI++)
		{
			if (GC.getCivicOptionInfo((CivicOptionTypes) iI).getTraitNoUpkeep(eTrait))
			{
				szHelpString += gDLL->getText("TXT_KEY_TRAIT_NO_UPKEEP", GC.getCivicOptionInfo((CivicOptionTypes)iI).getTextKeyWide());
			}
		}

		// Increase Building/Unit Production Speeds
		iLast = 0;
		for (iI = 0; iI < GC.getNumSpecialUnitInfos(); iI++)
		{
			if (GC.getSpecialUnitInfo((SpecialUnitTypes) iI).getProductionTraits(eTrait) != 0)
			{
				if (GC.getSpecialUnitInfo((SpecialUnitTypes) iI).getProductionTraits(eTrait) == 100)
				{
					szText = gDLL->getText("TXT_KEY_TRAIT_DOUBLE_SPEED");
				}
				else
				{
					szText = gDLL->getText("TXT_KEY_TRAIT_PRODUCTION_MODIFIER", GC.getSpecialUnitInfo((SpecialUnitTypes) iI).getProductionTraits(eTrait));
				}
				setListHelp(szHelpString, szText.GetCString(), GC.getSpecialUnitInfo((SpecialUnitTypes) iI).getDescription(), L", ", (GC.getSpecialUnitInfo((SpecialUnitTypes) iI).getProductionTraits(eTrait) != iLast));
				iLast = GC.getSpecialUnitInfo((SpecialUnitTypes) iI).getProductionTraits(eTrait);
			}
		}

		// Unit Classes
		iLast = 0;
		for (iI = 0; iI < GC.getNumUnitClassInfos();iI++)
		{
			if (eCivilization == NO_CIVILIZATION)
			{
				eLoopUnit = ((UnitTypes)(GC.getUnitClassInfo((UnitClassTypes)iI).getDefaultUnitIndex()));
			}
			else
			{
				eLoopUnit = ((UnitTypes)(GC.getCivilizationInfo(eCivilization).getCivilizationUnits(iI)));
			}

			if (eLoopUnit != NO_UNIT)
			{
				if (GC.getUnitInfo(eLoopUnit).getProductionTraits(eTrait) != 0)
				{
					if (GC.getUnitInfo(eLoopUnit).getProductionTraits(eTrait) == 100)
					{
						szText = gDLL->getText("TXT_KEY_TRAIT_DOUBLE_SPEED");
					}
					else
					{
						szText = gDLL->getText("TXT_KEY_TRAIT_PRODUCTION_MODIFIER", GC.getUnitInfo(eLoopUnit).getProductionTraits(eTrait));
					}
					CvWString szUnit;
					szUnit.Format(L"<link=literal>%s</link>", GC.getUnitInfo(eLoopUnit).getDescription());
					setListHelp(szHelpString, szText.GetCString(), szUnit, L", ", (GC.getUnitInfo(eLoopUnit).getProductionTraits(eTrait) != iLast));
					iLast = GC.getUnitInfo(eLoopUnit).getProductionTraits(eTrait);
				}
			}
		}

		// SpecialBuildings
		iLast = 0;
		for (iI = 0; iI < GC.getNumSpecialBuildingInfos(); iI++)
		{
			if (GC.getSpecialBuildingInfo((SpecialBuildingTypes) iI).getProductionTraits(eTrait) != 0)
			{
				if (GC.getSpecialBuildingInfo((SpecialBuildingTypes) iI).getProductionTraits(eTrait) == 100)
				{
					szText = gDLL->getText("TXT_KEY_TRAIT_DOUBLE_SPEED");
				}
				else
				{
					szText = gDLL->getText("TXT_KEY_TRAIT_PRODUCTION_MODIFIER", GC.getSpecialBuildingInfo((SpecialBuildingTypes) iI).getProductionTraits(eTrait));
				}
				setListHelp(szHelpString, szText.GetCString(), GC.getSpecialBuildingInfo((SpecialBuildingTypes) iI).getDescription(), L", ", (GC.getSpecialBuildingInfo((SpecialBuildingTypes) iI).getProductionTraits(eTrait) != iLast));
				iLast = GC.getSpecialBuildingInfo((SpecialBuildingTypes) iI).getProductionTraits(eTrait);
			}
		}

		// Buildings
		iLast = 0;
		for (iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
		{
			if (eCivilization == NO_CIVILIZATION)
			{
				eLoopBuilding = ((BuildingTypes)(GC.getBuildingClassInfo((BuildingClassTypes)iI).getDefaultBuildingIndex()));
			}
			else
			{
				eLoopBuilding = ((BuildingTypes)(GC.getCivilizationInfo(eCivilization).getCivilizationBuildings(iI)));
			}

			if (eLoopBuilding != NO_BUILDING)
			{
				if (GC.getBuildingInfo(eLoopBuilding).getProductionTraits(eTrait) != 0)
				{
					if (GC.getBuildingInfo(eLoopBuilding).getProductionTraits(eTrait) == 100)
					{
						szText = gDLL->getText("TXT_KEY_TRAIT_DOUBLE_SPEED");
					}
					else
					{
						szText = gDLL->getText("TXT_KEY_TRAIT_PRODUCTION_MODIFIER", GC.getBuildingInfo(eLoopBuilding).getProductionTraits(eTrait));
					}

					CvWString szBuilding;
					szBuilding.Format(L"<link=literal>%s</link>", GC.getBuildingInfo(eLoopBuilding).getDescription());
					setListHelp(szHelpString, szText.GetCString(), szBuilding, L", ", (GC.getBuildingInfo(eLoopBuilding).getProductionTraits(eTrait) != iLast));
					iLast = GC.getBuildingInfo(eLoopBuilding).getProductionTraits(eTrait);
				}
			}
		}

		// Buildings
		iLast = 0;
		for (iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
		{
			if (eCivilization == NO_CIVILIZATION)
			{
				eLoopBuilding = ((BuildingTypes)(GC.getBuildingClassInfo((BuildingClassTypes)iI).getDefaultBuildingIndex()));
			}
			else
			{
				eLoopBuilding = ((BuildingTypes)(GC.getCivilizationInfo(eCivilization).getCivilizationBuildings(iI)));
			}

			if (eLoopBuilding != NO_BUILDING)
			{
				int iHappiness = GC.getBuildingInfo(eLoopBuilding).getHappinessTraits(eTrait);
				if (iHappiness != 0)
				{
					if (iHappiness > 0)
					{
						szText = gDLL->getText("TXT_KEY_TRAIT_BUILDING_HAPPINESS", iHappiness, gDLL->getSymbolID(HAPPY_CHAR));
					}
					else
					{
						szText = gDLL->getText("TXT_KEY_TRAIT_BUILDING_HAPPINESS", -iHappiness, gDLL->getSymbolID(UNHAPPY_CHAR));
					}

					CvWString szBuilding;
					szBuilding.Format(L"<link=literal>%s</link>", GC.getBuildingInfo(eLoopBuilding).getDescription());
					setListHelp(szHelpString, szText.GetCString(), szBuilding, L", ", (iHappiness != iLast));
					iLast = iHappiness;
				}
			}
		}

	}

//	return szHelpString;
}


//
// parseLeaderTraits - SimpleCivPicker							// LOCALIZATION READY
//
void CvGameTextMgr::parseLeaderTraits(CvWString &szHelpString, LeaderHeadTypes eLeader, CivilizationTypes eCivilization, bool bDawnOfMan, bool bCivilopediaText)
{
	CvWString szTempBuffer;	// Formatting
	int iI;

	//	Build help string
	if (eLeader != NO_LEADER)
	{
		if (!bDawnOfMan && !bCivilopediaText)
		{
			szTempBuffer.Format( SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getLeaderHeadInfo(eLeader).getDescription());
			szHelpString+=szTempBuffer;
		}

		FAssert((GC.getNumTraitInfos() > 0) &&
			"GC.getNumTraitInfos() is less than or equal to zero but is expected to be larger than zero in CvSimpleCivPicker::setLeaderText");

		bool bFirst = true;
		for (iI = 0; iI < GC.getNumTraitInfos(); iI++)
		{
			if (GC.getLeaderHeadInfo(eLeader).hasTrait(iI))
			{
				if (!bFirst)
				{
					if (bDawnOfMan)
					{
						szHelpString += L", ";
					}
				}
				else
				{
					bFirst = false;
				}
				parseTraits(szHelpString, ((TraitTypes)iI), eCivilization, bDawnOfMan);
			}
		}
	}
	else
	{
		//	Random leader
		szTempBuffer.Format( SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), gDLL->getText("TXT_KEY_TRAIT_PLAYER_UNKNOWN").c_str());
		szHelpString+=szTempBuffer;
	}

//	return szHelpString;
}

//
// parseLeaderTraits - SimpleCivPicker							// LOCALIZATION READY
//
void CvGameTextMgr::parseLeaderShortTraits(CvWString &szHelpString, LeaderHeadTypes eLeader)
{
	int iI;

	//	Build help string
	if (eLeader != NO_LEADER)
	{
		FAssert((GC.getNumTraitInfos() > 0) &&
			"GC.getNumTraitInfos() is less than or equal to zero but is expected to be larger than zero in CvSimpleCivPicker::setLeaderText");

		bool bFirst = true;
		for (iI = 0; iI < GC.getNumTraitInfos(); iI++)
		{
			if (GC.getLeaderHeadInfo(eLeader).hasTrait(iI))
			{
				if (!bFirst)
				{
					szHelpString += L"/";
				}
				szHelpString += gDLL->getText(GC.getTraitInfo((TraitTypes)iI).getShortDescription());
				bFirst = false;
			}
		}
	}
	else
	{
		//	Random leader
		szHelpString += CvWString("???/???");
	}

	//	return szHelpString;
}

//
// Build Civilization Info Help Text
//
void CvGameTextMgr::parseCivInfos(CvWString &szInfoText, CivilizationTypes eCivilization, bool bDawnOfMan)
{
	wchar szBuffer[1024];
	wchar szTempString[1024];
	CvWString szText;
	UnitTypes eDefaultUnit;
	UnitTypes eUniqueUnit;
	BuildingTypes eDefaultBuilding;
	BuildingTypes eUniqueBuilding;

	if (eCivilization != NO_CIVILIZATION)
	{
		if (!bDawnOfMan)
		{
			// Civ Name
			szText = GC.getCivilizationInfo(eCivilization).getDescription();
			swprintf(szBuffer,  SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), szText.GetCString());
			szInfoText.append(szBuffer);
		}

		// Free Techs
		szText = gDLL->getText("TXT_KEY_FREE_TECHS");
		if (bDawnOfMan)
		{
			swprintf(szBuffer, L"%s: ", szText.GetCString());
		}
		else
		{
			swprintf(szBuffer, NEWLINE SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_ALT_HIGHLIGHT_TEXT"), szText.GetCString());
		}
		szInfoText.append(szBuffer);

		int iCounter = 0;
		for (int iI = 0; iI < GC.getNumTechInfos(); iI++)
		{
			if (GC.getCivilizationInfo(eCivilization).isCivilizationFreeTechs(iI))
			{
				iCounter++;
				// Add Tech
				szText = GC.getTechInfo((TechTypes) iI).getDescription();
				if (bDawnOfMan)
				{
					if (iCounter > 1)
					{
						szInfoText += L", ";
					}
					swprintf(szBuffer, L"%s", szText.GetCString());
					szInfoText.append(szBuffer);
				}
				else
				{
					swprintf(szBuffer, L"%s  %c%s", NEWLINE, gDLL->getSymbolID(BULLET_CHAR), szText.GetCString());
					szInfoText.append(szBuffer);
				}
			}
		}
		if (iCounter == 0)
		{
			iCounter++;
			szText = gDLL->getText("TXT_KEY_FREE_TECHS_NO");
			if (bDawnOfMan)
			{
				swprintf(szBuffer, L"%s", szText.GetCString());
			}
			else
			{
				swprintf(szBuffer, L"%s  %s", NEWLINE, szText.GetCString());
			}
			szInfoText.append(szBuffer);
		}

		// Free Units
		szText = gDLL->getText("TXT_KEY_FREE_UNITS");
		if (bDawnOfMan)
		{
			if (iCounter > 0)
			{
				szInfoText += NEWLINE;
			}
			swprintf(szTempString, L"%s: ", szText.GetCString());
		}
		else
		{
			swprintf(szTempString, NEWLINE SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_ALT_HIGHLIGHT_TEXT"), szText.GetCString());
		}
		szInfoText.append(szTempString);

		iCounter = 0;
		for (int iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
		{
			eDefaultUnit = ((UnitTypes)(GC.getCivilizationInfo(eCivilization).getCivilizationUnits(iI)));
			eUniqueUnit = ((UnitTypes)(GC.getUnitClassInfo((UnitClassTypes) iI).getDefaultUnitIndex()));
			if ((eDefaultUnit != NO_UNIT) && (eUniqueUnit != NO_UNIT))
			{
				if (eDefaultUnit != eUniqueUnit)
				{
					iCounter++;
					// Add Unit
					if (bDawnOfMan)
					{
						if (iCounter > 1)
						{
							szInfoText += L", ";
						}
						swprintf(szBuffer, L"%s - (%s)",
							GC.getUnitInfo(eDefaultUnit).getDescription(),
							GC.getUnitInfo(eUniqueUnit).getDescription());
					}
					else
					{
						swprintf(szBuffer, L"\n  %c%s - (%s)", gDLL->getSymbolID(BULLET_CHAR),
							GC.getUnitInfo(eDefaultUnit).getDescription(),
							GC.getUnitInfo(eUniqueUnit).getDescription());
					}
					szInfoText.append(szBuffer);
				}
			}
		}
		if (iCounter == 0)
		{
			iCounter++;
			szText = gDLL->getText("TXT_KEY_FREE_UNITS_NO");
			if (bDawnOfMan)
			{
				swprintf(szTempString, L"%s", szText.GetCString());
			}
			else
			{
				swprintf(szTempString, L"%s  %s", NEWLINE, szText.GetCString());
			}
			szInfoText.append(szTempString);
		}


		// Free Buildings
		szText = gDLL->getText("TXT_KEY_UNIQUE_BUILDINGS");
		if (bDawnOfMan)
		{
			if (iCounter > 0)
			{
				szInfoText += NEWLINE;
			}
			swprintf(szTempString, L"%s: ", szText.GetCString());
		}
		else
		{
			swprintf(szTempString, NEWLINE SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_ALT_HIGHLIGHT_TEXT"), szText.GetCString());
		}
		szInfoText.append(szTempString);

		iCounter = 0;
		for (int iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
		{
			eDefaultBuilding = ((BuildingTypes)(GC.getCivilizationInfo(eCivilization).getCivilizationBuildings(iI)));
			eUniqueBuilding = ((BuildingTypes)(GC.getBuildingClassInfo((BuildingClassTypes) iI).getDefaultBuildingIndex()));
			if ((eDefaultBuilding != NO_BUILDING) && (eUniqueBuilding != NO_BUILDING))
			{
				if (eDefaultBuilding != eUniqueBuilding)
				{
					iCounter++;
					// Add Building
					if (bDawnOfMan)
					{
						if (iCounter > 1)
						{
							szInfoText += L", ";
						}
						swprintf(szBuffer, L"%s - (%s)",
							GC.getBuildingInfo(eDefaultBuilding).getDescription(),
							GC.getBuildingInfo(eUniqueBuilding).getDescription());
					}
					else
					{
						swprintf(szBuffer, L"\n  %c%s - (%s)", gDLL->getSymbolID(BULLET_CHAR),
							GC.getBuildingInfo(eDefaultBuilding).getDescription(),
							GC.getBuildingInfo(eUniqueBuilding).getDescription());
					}
					szInfoText.append(szBuffer);
				}
			}
		}
		if (iCounter == 0)
		{
			iCounter++;
			szText = gDLL->getText("TXT_KEY_UNIQUE_BUILDINGS_NO");
			if (bDawnOfMan)
			{
				swprintf(szTempString, L"%s", szText.GetCString());
			}
			else
			{
				swprintf(szTempString, L"%s  %s", NEWLINE, szText.GetCString());
			}
			szInfoText.append(szTempString);
		}
	}
	else
	{
		//	This is a random civ, let us know here...
		szText = gDLL->getText("TXT_KEY_CIV_UNKNOWN");
		szInfoText.append(szText);
	}

//	return szInfoText;
}


void CvGameTextMgr::parseSpecialistHelp(CvWString &szHelpString, SpecialistTypes eSpecialist, CvCity* pCity, bool bCivilopediaText)
{
	CvWString szText;
	int aiYields[NUM_YIELD_TYPES];
	int aiCommerces[NUM_COMMERCE_TYPES];
	int iI;

	if (eSpecialist != NO_SPECIALIST)
	{
		if (!bCivilopediaText)
		{
			szHelpString=GC.getSpecialistInfo(eSpecialist).getDescription();
		}

		for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
		{
			if (bCivilopediaText)
			{
				aiYields[iI] = GC.getSpecialistInfo(eSpecialist).getYieldChange(iI);
			}
			else
			{
				aiYields[iI] = GET_PLAYER((pCity != NULL) ? pCity->getOwnerINLINE() : GC.getGameINLINE().getActivePlayer()).specialistYield(eSpecialist, ((YieldTypes)iI));
			}
		}

		setYieldChangeHelp(szHelpString, L"", L"", L"", aiYields);

		for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
		{
			if (bCivilopediaText)
			{
				aiCommerces[iI] = GC.getSpecialistInfo(eSpecialist).getCommerceChange(iI);
			}
			else
			{
				aiCommerces[iI] = GET_PLAYER((pCity != NULL) ? pCity->getOwnerINLINE() : GC.getGameINLINE().getActivePlayer()).specialistCommerce(((SpecialistTypes)eSpecialist), ((CommerceTypes)iI));
			}
		}

		setCommerceChangeHelp(szHelpString, L"", L"", L"", aiCommerces);

		if (GC.getSpecialistInfo(eSpecialist).getExperience() > 0)
		{
			szHelpString += NEWLINE + gDLL->getText("TXT_KEY_SPECIALIST_EXPERIENCE", GC.getSpecialistInfo(eSpecialist).getExperience());
		}

		if (GC.getSpecialistInfo(eSpecialist).getGreatPeopleRateChange() != 0)
		{
			szHelpString += NEWLINE + gDLL->getText("TXT_KEY_SPECIALIST_BIRTH_RATE", GC.getSpecialistInfo(eSpecialist).getGreatPeopleRateChange());
		}

		if (!isEmpty(GC.getSpecialistInfo(eSpecialist).getHelp()) && !bCivilopediaText)
		{
			szHelpString += NEWLINE;
			szHelpString += (GC.getSpecialistInfo(eSpecialist).getHelp());
		}
	}
}

void CvGameTextMgr::parseFreeSpecialistHelp(CvWString &szHelpString, const CvCity& kCity)
{
	for (int iLoopSpecialist = 0; iLoopSpecialist < GC.getNumSpecialistInfos(); iLoopSpecialist++)
	{
		SpecialistTypes eSpecialist = (SpecialistTypes)iLoopSpecialist;
		int iNumSpecialists = kCity.getFreeSpecialistCount(eSpecialist);

		if (iNumSpecialists > 0)
		{
			int aiYields[NUM_YIELD_TYPES];
			int aiCommerces[NUM_COMMERCE_TYPES];

			szHelpString += NEWLINE + CvWString::format(L"%s (%d): ", GC.getSpecialistInfo(eSpecialist).getDescription(), iNumSpecialists);

			for (int iI = 0; iI < NUM_YIELD_TYPES; iI++)
			{
				aiYields[iI] = iNumSpecialists * GET_PLAYER(kCity.getOwnerINLINE()).specialistYield(eSpecialist, ((YieldTypes)iI));
			}

			CvWString szYield;
			setYieldChangeHelp(szYield, L"", L"", L"", aiYields, false, false);
			szHelpString += szYield;

			for (int iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
			{
				aiCommerces[iI] = iNumSpecialists * GET_PLAYER(kCity.getOwnerINLINE()).specialistCommerce(eSpecialist, ((CommerceTypes)iI));
			}

			CvWString szCommerceString;
			setCommerceChangeHelp(szCommerceString, L"", L"", L"", aiCommerces, false, false);
			if (!szYield.empty() && !szCommerceString.empty())
			{
				szHelpString += L", ";

			}
			szHelpString += szCommerceString;

			if (GC.getSpecialistInfo(eSpecialist).getExperience() > 0)
			{
				if (!szYield.empty() || !szYield.empty())
				{
					szHelpString += L", ";
				}
				szHelpString += gDLL->getText("TXT_KEY_SPECIALIST_EXPERIENCE_SHORT", iNumSpecialists * GC.getSpecialistInfo(eSpecialist).getExperience());
			}
		}
	}
}


//
// Promotion Help
//
void CvGameTextMgr::parsePromotionHelp(CvWString &szBuffer, PromotionTypes ePromotion, const wchar* pcNewline)
{
	CvWString szText, szText2;
	int iI;

	if (NO_PROMOTION == ePromotion)
	{
		return;
	}

	if (GC.getPromotionInfo(ePromotion).isBlitz())
	{
		szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_BLITZ_TEXT");
	}

	if (GC.getPromotionInfo(ePromotion).isAmphib())
	{
		szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_AMPHIB_TEXT");
	}

	if (GC.getPromotionInfo(ePromotion).isRiver())
	{
		szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_RIVER_ATTACK_TEXT");
	}

	if (GC.getPromotionInfo(ePromotion).isEnemyRoute())
	{
		szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_ENEMY_ROADS_TEXT");
	}

	if (GC.getPromotionInfo(ePromotion).isAlwaysHeal())
	{
		szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_ALWAYS_HEAL_TEXT");
	}

	if (GC.getPromotionInfo(ePromotion).isHillsDoubleMove())
	{
		szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_HILLS_MOVE_TEXT");
	}

	if (GC.getPromotionInfo(ePromotion).isImmuneToFirstStrikes())
	{
		szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_IMMUNE_FIRST_STRIKES_TEXT");
	}

	for (iI = 0; iI < GC.getNumTerrainInfos(); iI++)
	{
		if (GC.getPromotionInfo(ePromotion).getTerrainDoubleMove(iI))
		{
			szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_DOUBLE_MOVE_TEXT", GC.getTerrainInfo((TerrainTypes) iI).getTextKeyWide());
		}
	}

	for (iI = 0; iI < GC.getNumFeatureInfos(); iI++)
	{
		if (GC.getPromotionInfo(ePromotion).getFeatureDoubleMove(iI))
		{
			szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_DOUBLE_MOVE_TEXT", GC.getFeatureInfo((FeatureTypes) iI).getTextKeyWide());
		}
	}

	if (GC.getPromotionInfo(ePromotion).getVisibilityChange() != 0)
	{
		szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_VISIBILITY_TEXT", GC.getPromotionInfo(ePromotion).getVisibilityChange());
	}

	if (GC.getPromotionInfo(ePromotion).getMovesChange() != 0)
	{
		szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_MOVE_TEXT", GC.getPromotionInfo(ePromotion).getMovesChange());
	}

	if (GC.getPromotionInfo(ePromotion).getMoveDiscountChange() != 0)
	{
		szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_MOVE_DISCOUNT_TEXT", -(GC.getPromotionInfo(ePromotion).getMoveDiscountChange()));
	}

	if (GC.getPromotionInfo(ePromotion).getWithdrawalChange() != 0)
	{
		szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_WITHDRAWAL_TEXT", GC.getPromotionInfo(ePromotion).getWithdrawalChange());
	}

	if (GC.getPromotionInfo(ePromotion).getCollateralDamageChange() != 0)
	{
		szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_COLLATERAL_DAMAGE_TEXT", GC.getPromotionInfo(ePromotion).getCollateralDamageChange());
	}

	if (GC.getPromotionInfo(ePromotion).getBombardRateChange() != 0)
	{
		szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_BOMBARD_TEXT", GC.getPromotionInfo(ePromotion).getBombardRateChange());
	}

	if (GC.getPromotionInfo(ePromotion).getFirstStrikesChange() != 0)
	{
		if (GC.getPromotionInfo(ePromotion).getFirstStrikesChange() == 1)
		{
			szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_FIRST_STRIKE_TEXT", GC.getPromotionInfo(ePromotion).getFirstStrikesChange());
		}
		else
		{
			szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_FIRST_STRIKES_TEXT", GC.getPromotionInfo(ePromotion).getFirstStrikesChange());
		}
	}

	if (GC.getPromotionInfo(ePromotion).getChanceFirstStrikesChange() != 0)
	{
		if (GC.getPromotionInfo(ePromotion).getChanceFirstStrikesChange() == 1)
		{
			szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_FIRST_STRIKE_CHANCE_TEXT", GC.getPromotionInfo(ePromotion).getChanceFirstStrikesChange());
		}
		else
		{
			szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_FIRST_STRIKES_CHANCE_TEXT", GC.getPromotionInfo(ePromotion).getChanceFirstStrikesChange());
		}
	}

	if (GC.getPromotionInfo(ePromotion).getEnemyHealChange() != 0)
	{
		szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_HEALS_EXTRA_TEXT", GC.getPromotionInfo(ePromotion).getEnemyHealChange()) + gDLL->getText("TXT_KEY_PROMOTION_ENEMY_LANDS_TEXT");
	}

	if (GC.getPromotionInfo(ePromotion).getNeutralHealChange() != 0)
	{
		szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_HEALS_EXTRA_TEXT", GC.getPromotionInfo(ePromotion).getNeutralHealChange()) + gDLL->getText("TXT_KEY_PROMOTION_NEUTRAL_LANDS_TEXT");
	}

	if (GC.getPromotionInfo(ePromotion).getFriendlyHealChange() != 0)
	{
		szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_HEALS_EXTRA_TEXT", GC.getPromotionInfo(ePromotion).getFriendlyHealChange()) + gDLL->getText("TXT_KEY_PROMOTION_FRIENDLY_LANDS_TEXT");
	}

	if (GC.getPromotionInfo(ePromotion).getSameTileHealChange() != 0)
	{
		szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_HEALS_SAME_TEXT", GC.getPromotionInfo(ePromotion).getSameTileHealChange()) + gDLL->getText("TXT_KEY_PROMOTION_DAMAGE_TURN_TEXT");
	}

	if (GC.getPromotionInfo(ePromotion).getAdjacentTileHealChange() != 0)
	{
		szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_HEALS_ADJACENT_TEXT", GC.getPromotionInfo(ePromotion).getAdjacentTileHealChange()) + gDLL->getText("TXT_KEY_PROMOTION_DAMAGE_TURN_TEXT");
	}

	if (GC.getPromotionInfo(ePromotion).getCombatPercent() != 0)
	{
		szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_STRENGTH_TEXT", GC.getPromotionInfo(ePromotion).getCombatPercent());
	}

	if (GC.getPromotionInfo(ePromotion).getCityAttackPercent() != 0)
	{
		szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_CITY_ATTACK_TEXT", GC.getPromotionInfo(ePromotion).getCityAttackPercent());
	}

	if (GC.getPromotionInfo(ePromotion).getCityDefensePercent() != 0)
	{
		szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_CITY_DEFENSE_TEXT", GC.getPromotionInfo(ePromotion).getCityDefensePercent());
	}

	if (GC.getPromotionInfo(ePromotion).getHillsAttackPercent() != 0)
	{
		szBuffer += pcNewline + gDLL->getText("TXT_KEY_UNIT_HILLS_ATTACK", GC.getPromotionInfo(ePromotion).getHillsAttackPercent());
	}

	if (GC.getPromotionInfo(ePromotion).getHillsDefensePercent() != 0)
	{
		szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_HILLS_DEFENSE_TEXT", GC.getPromotionInfo(ePromotion).getHillsDefensePercent());
	}

	if (GC.getPromotionInfo(ePromotion).getRevoltProtection() != 0)
	{
		szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_REVOLT_PROTECTION_TEXT", GC.getPromotionInfo(ePromotion).getRevoltProtection());
	}

	if (GC.getPromotionInfo(ePromotion).getCollateralDamageProtection() != 0)
	{
		szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_COLLATERAL_PROTECTION_TEXT", GC.getPromotionInfo(ePromotion).getCollateralDamageProtection());
	}

	if (GC.getPromotionInfo(ePromotion).getPillageChange() != 0)
	{
		szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_PILLAGE_CHANGE_TEXT", GC.getPromotionInfo(ePromotion).getPillageChange());
	}

	if (GC.getPromotionInfo(ePromotion).getUpgradeDiscount() != 0)
	{
		if (100 == GC.getPromotionInfo(ePromotion).getUpgradeDiscount())
		{
			szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_UPGRADE_DISCOUNT_FREE_TEXT");
		}
		else
		{
			szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_UPGRADE_DISCOUNT_TEXT", GC.getPromotionInfo(ePromotion).getUpgradeDiscount());
		}
	}

	if (GC.getPromotionInfo(ePromotion).getExperiencePercent() != 0)
	{
		szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_FASTER_EXPERIENCE_TEXT", GC.getPromotionInfo(ePromotion).getExperiencePercent());
	}

	if (GC.getPromotionInfo(ePromotion).getKamikazePercent() != 0)
	{
		szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_KAMIKAZE_TEXT", GC.getPromotionInfo(ePromotion).getKamikazePercent());
	}

	for (iI = 0; iI < GC.getNumTerrainInfos(); iI++)
	{
		if (GC.getPromotionInfo(ePromotion).getTerrainDefensePercent(iI) != 0)
		{
			szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_DEFENSE_TEXT", GC.getPromotionInfo(ePromotion).getTerrainDefensePercent(iI), GC.getTerrainInfo((TerrainTypes) iI).getTextKeyWide());
		}
	}

	for (iI = 0; iI < GC.getNumFeatureInfos(); iI++)
	{
		if (GC.getPromotionInfo(ePromotion).getFeatureDefensePercent(iI) != 0)
		{
			szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_DEFENSE_TEXT", GC.getPromotionInfo(ePromotion).getFeatureDefensePercent(iI), GC.getFeatureInfo((FeatureTypes) iI).getTextKeyWide());
		}
	}

	for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
	{
		if (GC.getPromotionInfo(ePromotion).getUnitCombatModifierPercent(iI) != 0)
		{
			szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_VERSUS_TEXT", GC.getPromotionInfo(ePromotion).getUnitCombatModifierPercent(iI), GC.getUnitCombatInfo((UnitCombatTypes)iI).getTextKeyWide());
		}
	}

	for (iI = 0; iI < NUM_DOMAIN_TYPES; iI++)
	{
		if (GC.getPromotionInfo(ePromotion).getDomainModifierPercent(iI) != 0)
		{
			szBuffer += pcNewline + gDLL->getText("TXT_KEY_PROMOTION_VERSUS_TEXT", GC.getPromotionInfo(ePromotion).getDomainModifierPercent(iI), GC.getDomainInfo((DomainTypes)iI).getTextKeyWide());
		}
	}
}

//	Function:			parseCivicInfo()
//	Description:	Will parse the civic info help
//	Parameters:		szHelpText -- the text to put it into
//								civicInfo - what to parse
//	Returns:			nothing
void CvGameTextMgr::parseCivicInfo(CvWString &szHelpText, CivicTypes eCivic, bool bCivilopediaText, bool bPlayerContext, bool bSkipName)
{
	CvWString szFirstBuffer;
	bool bFound;
	bool bFirst;
	int iLast;
	int iI, iJ;

	if (NO_CIVIC == eCivic)
	{
		return;
	}

	szHelpText = "";

	FAssert(GC.getGameINLINE().getActivePlayer() != NO_PLAYER || !bPlayerContext);

	if (!bSkipName)
	{
		szHelpText += GC.getCivicInfo(eCivic).getDescription();
	}

	if (!bCivilopediaText)
	{
		if (!bPlayerContext || !(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).canDoCivics(eCivic)))
		{
			if (!bPlayerContext || !(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isHasTech((TechTypes)(GC.getCivicInfo(eCivic).getTechPrereq()))))
			{
				if (GC.getCivicInfo(eCivic).getTechPrereq() != NO_TECH)
				{
					szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_REQUIRES", GC.getTechInfo((TechTypes)GC.getCivicInfo(eCivic).getTechPrereq()).getTextKeyWide());
				}
			}
		}
	}

	// Special Building Not Required...
	for (iI = 0; iI < GC.getNumSpecialBuildingInfos(); iI++)
	{
		if (GC.getCivicInfo(eCivic).isSpecialBuildingNotRequired(iI))
		{
			// XXX "Missionaries"??? - Now in XML
			szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_BUILD_MISSIONARIES", GC.getSpecialBuildingInfo((SpecialBuildingTypes)iI).getTextKeyWide());
		}
	}

	// Valid Specialists...

	bFirst = true;

	for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
	{
		if (GC.getCivicInfo(eCivic).isSpecialistValid(iI))
		{
			szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_CIVIC_UNLIMTED").c_str());
			CvWString szSpecialist;
			szSpecialist.Format(L"<link=literal>%s</link>", GC.getSpecialistInfo((SpecialistTypes)iI).getDescription());
			setListHelp(szHelpText, szFirstBuffer, szSpecialist, L", ", bFirst);
			bFirst = false;
		}
	}

	//	Great People Modifier...
	if (GC.getCivicInfo(eCivic).getGreatPeopleRateModifier() != 0)
	{
		szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_GREAT_PEOPLE_MOD", GC.getCivicInfo(eCivic).getGreatPeopleRateModifier());
	}

	//	Great General Modifier...
	if (GC.getCivicInfo(eCivic).getGreatGeneralRateModifier() != 0)
	{
		szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_GREAT_GENERAL_MOD", GC.getCivicInfo(eCivic).getGreatGeneralRateModifier());
	}

	if (GC.getCivicInfo(eCivic).getDomesticGreatGeneralRateModifier() != 0)
	{
		szHelpText += NEWLINE + gDLL->getText("TXT_KEY_DOMESTIC_GREAT_GENERAL_MODIFIER", GC.getCivicInfo(eCivic).getDomesticGreatGeneralRateModifier());
	}

	//	State Religion Great People Modifier...
	if (GC.getCivicInfo(eCivic).getStateReligionGreatPeopleRateModifier() != 0)
	{
		if (bPlayerContext && (GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getStateReligion() != NO_RELIGION))
		{
			szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_GREAT_PEOPLE_MOD_RELIGION", GC.getCivicInfo(eCivic).getStateReligionGreatPeopleRateModifier(), GC.getReligionInfo(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getStateReligion()).getChar());
		}
		else
		{
			szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_GREAT_PEOPLE_MOD_STATE_RELIGION", GC.getCivicInfo(eCivic).getStateReligionGreatPeopleRateModifier(), gDLL->getSymbolID(RELIGION_CHAR));
		}
	}

	//	Distance Maintenance Modifer...
	if (GC.getCivicInfo(eCivic).getDistanceMaintenanceModifier() != 0)
	{
		if (GC.getCivicInfo(eCivic).getDistanceMaintenanceModifier() <= -100)
		{
			szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_DISTANCE_MAINT");
		}
		else
		{
			szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_DISTANCE_MAINT_MOD", GC.getCivicInfo(eCivic).getDistanceMaintenanceModifier());
		}
	}

	//	Num Cities Maintenance Modifer...
	if (GC.getCivicInfo(eCivic).getNumCitiesMaintenanceModifier() != 0)
	{
		if (GC.getCivicInfo(eCivic).getNumCitiesMaintenanceModifier() <= -100)
		{
			szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_NO_MAINT_NUM_CITIES");
		}
		else
		{
			szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_NO_MAINT_NUM_CITIES_MOD", GC.getCivicInfo(eCivic).getNumCitiesMaintenanceModifier());
		}
	}

	//	Extra Health
	if (GC.getCivicInfo(eCivic).getExtraHealth() != 0)
	{
		szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_EXTRA_HEALTH", abs(GC.getCivicInfo(eCivic).getExtraHealth()), ((GC.getCivicInfo(eCivic).getExtraHealth() > 0) ? gDLL->getSymbolID(HEALTHY_CHAR): gDLL->getSymbolID(UNHEALTHY_CHAR)));
	}

	//	Free Experience
	if (GC.getCivicInfo(eCivic).getFreeExperience() != 0)
	{
		szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_FREE_XP", GC.getCivicInfo(eCivic).getFreeExperience());
	}

	//	Worker speed modifier
	if (GC.getCivicInfo(eCivic).getWorkerSpeedModifier() != 0)
	{
		szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_WORKER_SPEED", GC.getCivicInfo(eCivic).getWorkerSpeedModifier());
	}

	//	Improvement upgrade rate modifier
	if (GC.getCivicInfo(eCivic).getImprovementUpgradeRateModifier() != 0)
	{
		bFirst = true;

		for (iI = 0; iI < GC.getNumImprovementInfos(); iI++)
		{
			if (GC.getImprovementInfo((ImprovementTypes)iI).getImprovementUpgrade() != NO_IMPROVEMENT)
			{
				szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_CIVIC_IMPROVEMENT_UPGRADE", GC.getCivicInfo(eCivic).getImprovementUpgradeRateModifier()).c_str());
				CvWString szImprovement;
				szImprovement.Format(L"<link=literal>%s</link>", GC.getImprovementInfo((ImprovementTypes)iI).getDescription());
				setListHelp(szHelpText, szFirstBuffer, szImprovement, L", ", bFirst);
				bFirst = false;
			}
		}
	}

	//	Military unit production modifier
	if (GC.getCivicInfo(eCivic).getMilitaryProductionModifier() != 0)
	{
		szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_MILITARY_PRODUCTION", GC.getCivicInfo(eCivic).getMilitaryProductionModifier());
	}

	//	Free units population percent
	if ((GC.getCivicInfo(eCivic).getBaseFreeUnits() != 0) || (GC.getCivicInfo(eCivic).getFreeUnitsPopulationPercent() != 0))
	{
		if (bPlayerContext)
		{
			szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_FREE_UNITS", (GC.getCivicInfo(eCivic).getBaseFreeUnits() + ((GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getTotalPopulation() * GC.getCivicInfo(eCivic).getFreeUnitsPopulationPercent()) / 100)));
		}
		else
		{
			szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_UNIT_SUPPORT");
		}
	}

	//	Free military units population percent
	if ((GC.getCivicInfo(eCivic).getBaseFreeMilitaryUnits() != 0) || (GC.getCivicInfo(eCivic).getFreeMilitaryUnitsPopulationPercent() != 0))
	{
		if (bPlayerContext)
		{
			szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_FREE_MILITARY_UNITS", (GC.getCivicInfo(eCivic).getBaseFreeMilitaryUnits() + ((GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getTotalPopulation() * GC.getCivicInfo(eCivic).getFreeMilitaryUnitsPopulationPercent()) / 100)));
		}
		else
		{
			szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_MILITARY_UNIT_SUPPORT");
		}
	}

	//	Happiness per military unit
	if (GC.getCivicInfo(eCivic).getHappyPerMilitaryUnit() != 0)
	{
		szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_UNIT_HAPPINESS", GC.getCivicInfo(eCivic).getHappyPerMilitaryUnit(), ((GC.getCivicInfo(eCivic).getHappyPerMilitaryUnit() > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)));
	}

	//	Military units produced with food
	if (GC.getCivicInfo(eCivic).isMilitaryFoodProduction())
	{
		szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_MILITARY_FOOD");
	}

	//	Conscription
	if (getWorldSizeMaxConscript(eCivic) != 0)
	{
		szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_CONSCRIPTION", getWorldSizeMaxConscript(eCivic));
	}

	//	Population Unhealthiness
	if (GC.getCivicInfo(eCivic).isNoUnhealthyPopulation())
	{
		szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_NO_POP_UNHEALTHY");
	}

	//	Building Unhealthiness
	if (GC.getCivicInfo(eCivic).isBuildingOnlyHealthy())
	{
		szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_NO_BUILDING_UNHEALTHY");
	}

	//	Population Unhealthiness
	if (0 != GC.getCivicInfo(eCivic).getExpInBorderModifier())
	{
		szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_EXPERIENCE_IN_BORDERS", GC.getCivicInfo(eCivic).getExpInBorderModifier());
	}

	//	War Weariness
	if (GC.getCivicInfo(eCivic).getWarWearinessModifier() != 0)
	{
		if (GC.getCivicInfo(eCivic).getWarWearinessModifier() <= -100)
		{
			szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_NO_WAR_WEARINESS");
		}
		else
		{
			szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_EXTRA_WAR_WEARINESS", GC.getCivicInfo(eCivic).getWarWearinessModifier());
		}
	}

	//	Free specialists
	if (GC.getCivicInfo(eCivic).getFreeSpecialist() != 0)
	{
		szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_FREE_SPECIALISTS", GC.getCivicInfo(eCivic).getFreeSpecialist());
	}

	//	Trade routes
	if (GC.getCivicInfo(eCivic).getTradeRoutes() != 0)
	{
		szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_TRADE_ROUTES", GC.getCivicInfo(eCivic).getTradeRoutes());
	}

	//	No Foreign Trade
	if (GC.getCivicInfo(eCivic).isNoForeignTrade())
	{
		szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_NO_FOREIGN_TRADE");
	}

	//	Freedom Anger
	if (GC.getCivicInfo(eCivic).getCivicPercentAnger() != 0)
	{
		szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_FREEDOM_ANGER", GC.getCivicInfo(eCivic).getTextKeyWide());
	}

	if (!(GC.getCivicInfo(eCivic).isStateReligion()))
	{
		bFound = false;

		for (iI = 0; iI < GC.getNumCivicInfos(); iI++)
		{
			if ((GC.getCivicInfo((CivicTypes) iI).getCivicOptionType() == GC.getCivicInfo(eCivic).getCivicOptionType()) && (GC.getCivicInfo((CivicTypes) iI).isStateReligion()))
			{
				bFound = true;
			}
		}

		if (bFound)
		{
			szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_NO_STATE_RELIGION");
		}
	}

	if (GC.getCivicInfo(eCivic).getStateReligionHappiness() != 0)
	{
		if (bPlayerContext && (GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getStateReligion() != NO_RELIGION))
		{
			szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_STATE_RELIGION_HAPPINESS", abs(GC.getCivicInfo(eCivic).getStateReligionHappiness()), ((GC.getCivicInfo(eCivic).getStateReligionHappiness() > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)), GC.getReligionInfo(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getStateReligion()).getChar());
		}
		else
		{
			szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_RELIGION_HAPPINESS", abs(GC.getCivicInfo(eCivic).getStateReligionHappiness()), ((GC.getCivicInfo(eCivic).getStateReligionHappiness() > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)));
		}
	}

	if (GC.getCivicInfo(eCivic).getNonStateReligionHappiness() != 0)
	{
		if (GC.getCivicInfo(eCivic).isStateReligion())
		{
			szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_NON_STATE_REL_HAPPINESS_NO_STATE");
		}
		else
		{
			szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_NON_STATE_REL_HAPPINESS_WITH_STATE", abs(GC.getCivicInfo(eCivic).getNonStateReligionHappiness()), ((GC.getCivicInfo(eCivic).getNonStateReligionHappiness() > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)));
		}
	}

	//	State Religion Unit Production Modifier
	if (GC.getCivicInfo(eCivic).getStateReligionUnitProductionModifier() != 0)
	{
		if (bPlayerContext && (GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getStateReligion() != NO_RELIGION))
		{
			szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_REL_TRAIN_BONUS", GC.getReligionInfo(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getStateReligion()).getChar(), GC.getCivicInfo(eCivic).getStateReligionUnitProductionModifier());
		}
		else
		{
			szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_STATE_REL_TRAIN_BONUS", GC.getCivicInfo(eCivic).getStateReligionUnitProductionModifier());
		}
	}

	//	State Religion Building Production Modifier
	if (GC.getCivicInfo(eCivic).getStateReligionBuildingProductionModifier() != 0)
	{
		if (bPlayerContext && (GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getStateReligion() != NO_RELIGION))
		{
			szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_REL_BUILDING_BONUS", GC.getReligionInfo(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getStateReligion()).getChar(), GC.getCivicInfo(eCivic).getStateReligionBuildingProductionModifier());
		}
		else
		{
			szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_STATE_REL_BUILDING_BONUS", GC.getCivicInfo(eCivic).getStateReligionBuildingProductionModifier());
		}
	}

	//	State Religion Free Experience
	if (GC.getCivicInfo(eCivic).getStateReligionFreeExperience() != 0)
	{
		if (bPlayerContext && (GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getStateReligion() != NO_RELIGION))
		{
			szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_REL_FREE_XP", GC.getCivicInfo(eCivic).getStateReligionFreeExperience(), GC.getReligionInfo(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getStateReligion()).getChar());
		}
		else
		{
			szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_STATE_REL_FREE_XP", GC.getCivicInfo(eCivic).getStateReligionFreeExperience());
		}
	}

	if (GC.getCivicInfo(eCivic).isNoNonStateReligionSpread())
	{
		szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_NO_NON_STATE_SPREAD");
	}

	//	Yield Modifiers
	setYieldChangeHelp(szHelpText, L"", L"", gDLL->getText("TXT_KEY_CIVIC_IN_ALL_CITIES").GetCString(), GC.getCivicInfo(eCivic).getYieldModifierArray(), true);

	//	Capital Yield Modifiers
	setYieldChangeHelp(szHelpText, L"", L"", gDLL->getText("TXT_KEY_CIVIC_IN_CAPITAL").GetCString(), GC.getCivicInfo(eCivic).getCapitalYieldModifierArray(), true);

	//	Trade Yield Modifiers
	setYieldChangeHelp(szHelpText, L"", L"", gDLL->getText("TXT_KEY_CIVIC_FROM_TRADE_ROUTES").GetCString(), GC.getCivicInfo(eCivic).getTradeYieldModifierArray(), true);

	//	Commerce Modifier
	setCommerceChangeHelp(szHelpText, L"", L"", gDLL->getText("TXT_KEY_CIVIC_IN_ALL_CITIES").GetCString(), GC.getCivicInfo(eCivic).getCommerceModifierArray(), true);

	//	Capital Commerce Modifiers
	setCommerceChangeHelp(szHelpText, L"", L"", gDLL->getText("TXT_KEY_CIVIC_IN_CAPITAL").GetCString(), GC.getCivicInfo(eCivic).getCapitalCommerceModifierArray(), true);

	//	Specialist Commerce
	setCommerceChangeHelp(szHelpText, L"", L"", gDLL->getText("TXT_KEY_CIVIC_PER_SPECIALIST").GetCString(), GC.getCivicInfo(eCivic).getSpecialistExtraCommerceArray());

	//	Largest City Happiness
	if (GC.getCivicInfo(eCivic).getLargestCityHappiness() != 0)
	{
		szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_LARGEST_CITIES_HAPPINESS", GC.getCivicInfo(eCivic).getLargestCityHappiness(), ((GC.getCivicInfo(eCivic).getLargestCityHappiness() > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)), GC.getWorldInfo(GC.getMapINLINE().getWorldSize()).getTargetNumCities());
	}

	//	Improvement Yields
	for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		iLast = 0;

		for (iJ = 0; iJ < GC.getNumImprovementInfos(); iJ++)
		{
			if (GC.getCivicInfo(eCivic).getImprovementYieldChanges(iJ, iI) != 0)
			{
				szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_CIVIC_IMPROVEMENT_YIELD_CHANGE", GC.getCivicInfo(eCivic).getImprovementYieldChanges(iJ, iI), GC.getYieldInfo((YieldTypes)iI).getChar()).c_str());
				CvWString szImprovement;
				szImprovement.Format(L"<link=literal>%s</link>", GC.getImprovementInfo((ImprovementTypes)iJ).getDescription());
				setListHelp(szHelpText, szFirstBuffer, szImprovement, L", ", (GC.getCivicInfo(eCivic).getImprovementYieldChanges(iJ, iI) != iLast));
				iLast = GC.getCivicInfo(eCivic).getImprovementYieldChanges(iJ, iI);
			}
		}
	}

	//	Building Happiness
	for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
	{
		if (GC.getCivicInfo(eCivic).getBuildingHappinessChanges(iI) != 0)
		{
			szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_BUILDING_HAPPINESS", GC.getCivicInfo(eCivic).getBuildingHappinessChanges(iI), ((GC.getCivicInfo(eCivic).getBuildingHappinessChanges(iI) > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)), GC.getBuildingInfo((BuildingTypes)iI).getTextKeyWide());
		}
	}

	//	Feature Happiness
	iLast = 0;

	for (iI = 0; iI < GC.getNumFeatureInfos(); iI++)
	{
		if (GC.getCivicInfo(eCivic).getFeatureHappinessChanges(iI) != 0)
		{
			szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_CIVIC_FEATURE_HAPPINESS", GC.getCivicInfo(eCivic).getFeatureHappinessChanges(iI), ((GC.getCivicInfo(eCivic).getFeatureHappinessChanges(iI) > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR))).c_str());
			CvWString szFeature;
			szFeature.Format(L"<link=literal>%s</link>", GC.getFeatureInfo((FeatureTypes)iI).getDescription());
			setListHelp(szHelpText, szFirstBuffer, szFeature, L", ", (GC.getCivicInfo(eCivic).getFeatureHappinessChanges(iI) != iLast));
			iLast = GC.getCivicInfo(eCivic).getFeatureHappinessChanges(iI);
		}
	}

	//	Hurry types
	for (iI = 0; iI < GC.getNumHurryInfos(); iI++)
	{
		if (GC.getCivicInfo(eCivic).isHurry(iI))
		{
			szHelpText += CvWString::format(L"%s%c%s", NEWLINE, gDLL->getSymbolID(BULLET_CHAR), GC.getHurryInfo((HurryTypes)iI).getDescription());
		}
	}

	//	Gold cost per unit
	if (GC.getCivicInfo(eCivic).getGoldPerUnit() != 0)
	{
		szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_SUPPORT_COSTS", (GC.getCivicInfo(eCivic).getGoldPerUnit() > 0), GC.getCommerceInfo(COMMERCE_GOLD).getChar());
	}

	//	Gold cost per military unit
	if (GC.getCivicInfo(eCivic).getGoldPerMilitaryUnit() != 0)
	{
		szHelpText += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_MILITARY_SUPPORT_COSTS", (GC.getCivicInfo(eCivic).getGoldPerMilitaryUnit() > 0), GC.getCommerceInfo(COMMERCE_GOLD).getChar());
	}
}


void CvGameTextMgr::setTechHelp(CvWString &szBuffer, TechTypes eTech, bool bCivilopediaText, bool bPlayerContext, bool bStrategyText, bool bTreeInfo, TechTypes eFromTech)
{
	CvWString szTempBuffer;
	CvWString szFirstBuffer;
	BuildingTypes eLoopBuilding;
	UnitTypes eLoopUnit;
	bool bFirst;
	int iI;

	if (NO_TECH == eTech)
	{
		return;
	}

	//	Tech Name
	if (!bCivilopediaText && (!bTreeInfo || (NO_TECH == eFromTech)))
	{
		szTempBuffer.Format( SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_TECH_TEXT"), GC.getTechInfo(eTech).getDescription());
		szBuffer += szTempBuffer;
	}

	FAssert(GC.getGameINLINE().getActivePlayer() != NO_PLAYER || !bPlayerContext);

	if (bTreeInfo && (NO_TECH != eFromTech))
	{
		buildTechTreeString(szBuffer, eTech, bPlayerContext, eFromTech);
	}

	//	Obsolete Buildings
	for (iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
	{
		if (!bPlayerContext || (GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getBuildingClassCount((BuildingClassTypes)iI) > 0))
		{
			if (GC.getGameINLINE().getActivePlayer() != NO_PLAYER)
			{
				eLoopBuilding = (BuildingTypes)GC.getCivilizationInfo(GC.getGameINLINE().getActiveCivilizationType()).getCivilizationBuildings(iI);
			}
			else
			{
				eLoopBuilding = (BuildingTypes)GC.getBuildingClassInfo((BuildingClassTypes)iI).getDefaultBuildingIndex();
			}

			if (eLoopBuilding != NO_BUILDING)
			{
				//	Obsolete Buildings Check...
				if (GC.getBuildingInfo(eLoopBuilding).getObsoleteTech() == eTech)
				{
					buildObsoleteString(szBuffer, eLoopBuilding, true);
				}
			}
		}
	}

	//	Obsolete Bonuses
	for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
	{
		if (GC.getBonusInfo((BonusTypes)iI).getTechObsolete() == eTech)
		{
			buildObsoleteBonusString(szBuffer, iI, true);
		}
	}

	for (iI = 0; iI < GC.getNumSpecialBuildingInfos(); iI++)
	{
		if (GC.getSpecialBuildingInfo((SpecialBuildingTypes) iI).getObsoleteTech() == eTech)
		{
			buildObsoleteSpecialString(szBuffer, iI, true);
		}
	}

	//	Route movement change...
	buildMoveString(szBuffer, eTech, true, bPlayerContext);

	//	Creates a free unit...
	buildFreeUnitString(szBuffer, eTech, true, bPlayerContext);

	//	Increases feature production...
	buildFeatureProductionString(szBuffer, eTech, true, bPlayerContext);

	//	Increases worker build rate...
	buildWorkerRateString(szBuffer, eTech, true, bPlayerContext);

	//	Trade Routed per city change...
	buildTradeRouteString(szBuffer, eTech, true, bPlayerContext);

	//	Health increase...
	buildHealthRateString(szBuffer, eTech, true, bPlayerContext);

	//	Happiness increase...
	buildHappinessRateString(szBuffer, eTech, true, bPlayerContext);

	//	Free Techs...
	buildFreeTechString(szBuffer, eTech, true, bPlayerContext);

	//	Line of Sight Bonus across water...
	buildLOSString(szBuffer, eTech, true, bPlayerContext);

	//	Centers world map...
	buildMapCenterString(szBuffer, eTech, true, bPlayerContext);

	//	Reveals World Map...
	buildMapRevealString(szBuffer, eTech, true);

	//	Enables map trading...
	buildMapTradeString(szBuffer, eTech, true, bPlayerContext);

	//	Enables tech trading...
	buildTechTradeString(szBuffer, eTech, true, bPlayerContext);

	//	Enables gold trading...
	buildGoldTradeString(szBuffer, eTech, true, bPlayerContext);

	//	Enables open borders...
	buildOpenBordersString(szBuffer, eTech, true, bPlayerContext);

	//	Enables defensive pacts...
	buildDefensivePactString(szBuffer, eTech, true, bPlayerContext);

	//	Enables permanent alliances...
	buildPermanentAllianceString(szBuffer, eTech, true, bPlayerContext);

	//	Enables bridge building...
	buildBridgeString(szBuffer, eTech, true, bPlayerContext);

	//	Can spread irrigation...
	buildIrrigationString(szBuffer, eTech, true, bPlayerContext);

	//	Ignore irrigation...
	buildIgnoreIrrigationString(szBuffer, eTech, true, bPlayerContext);

	//	Coastal work...
	buildWaterWorkString(szBuffer, eTech, true, bPlayerContext);

	//	Enables permanent alliances...
	buildVassalStateString(szBuffer, eTech, true, bPlayerContext);

	//	Build farm, irrigation, etc...
	for (iI = 0; iI < GC.getNumBuildInfos(); iI++)
	{
		buildImprovementString(szBuffer, eTech, iI, true, bPlayerContext);
	}

	//	Extra moves for certain domains...
	for (iI = 0; iI < NUM_DOMAIN_TYPES; iI++)
	{
		buildDomainExtraMovesString(szBuffer, eTech, iI, true, bPlayerContext);
	}

	//	Adjusting culture, science, etc
	for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		buildAdjustString(szBuffer, eTech, iI, true, bPlayerContext);
	}

	//	Enabling trade routes on water...?
	for (iI = 0; iI < GC.getNumTerrainInfos(); iI++)
	{
		buildTerrainTradeString(szBuffer, eTech, iI, true, bPlayerContext);
	}

	//	Special Buildings
	for (iI = 0; iI < GC.getNumSpecialBuildingInfos(); iI++)
	{
		buildSpecialBuildingString(szBuffer, eTech, iI, true, bPlayerContext);
	}

	//	Build farm, mine, etc...
	for (iI = 0; iI < GC.getNumImprovementInfos(); iI++)
	{
		buildYieldChangeString(szBuffer, eTech, iI, true, bPlayerContext);
	}

	bFirst = true;

	for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
	{
		bFirst = buildBonusRevealString(szBuffer, eTech, iI, bFirst, true, bPlayerContext);
	}

	bFirst = true;

	for (iI = 0; iI < GC.getNumCivicInfos(); iI++)
	{
		bFirst = buildCivicRevealString(szBuffer, eTech, iI, bFirst, true, bPlayerContext);
	}

	if (!bCivilopediaText)
	{
		bFirst = true;

		for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
		{
			if (!bPlayerContext || !(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).isProductionMaxedUnitClass((UnitClassTypes)iI)))
			{
				if (GC.getGameINLINE().getActivePlayer() != NO_PLAYER)
				{
					eLoopUnit = (UnitTypes)GC.getCivilizationInfo(GC.getGameINLINE().getActiveCivilizationType()).getCivilizationUnits(iI);
				}
				else
				{
					eLoopUnit = (UnitTypes)GC.getUnitClassInfo((UnitClassTypes)iI).getDefaultUnitIndex();
				}

				if (eLoopUnit != NO_UNIT)
				{
					if (!bPlayerContext || !(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).canTrain(eLoopUnit)))
					{
						if (GC.getUnitInfo(eLoopUnit).getPrereqAndTech() == eTech)
						{
							szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_TECH_CAN_TRAIN").c_str());
							szTempBuffer.Format( SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_UNIT_TEXT"), GC.getUnitInfo(eLoopUnit).getDescription());
							setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", bFirst);
							bFirst = false;
						}
						else
						{
							for (int iJ = 0; iJ < GC.getDefineINT("NUM_UNIT_AND_TECH_PREREQS"); iJ++)
							{
								if (GC.getUnitInfo(eLoopUnit).getPrereqAndTechs(iJ) == eTech)
								{
									szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_TECH_CAN_TRAIN").c_str());
									szTempBuffer.Format( SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_UNIT_TEXT"), GC.getUnitInfo(eLoopUnit).getDescription());
									setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", bFirst);
									bFirst = false;
									break;
								}
							}
						}
					}
				}
			}
		}

		bFirst = true;

		for (iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
		{
			if (!bPlayerContext || !(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).isProductionMaxedBuildingClass((BuildingClassTypes)iI)))
			{
				if (GC.getGameINLINE().getActivePlayer() != NO_PLAYER)
				{
					eLoopBuilding = (BuildingTypes)GC.getCivilizationInfo(GC.getGameINLINE().getActiveCivilizationType()).getCivilizationBuildings(iI);
				}
				else
				{
					eLoopBuilding = (BuildingTypes)GC.getBuildingClassInfo((BuildingClassTypes)iI).getDefaultBuildingIndex();
				}

				if (eLoopBuilding != NO_BUILDING)
				{
					if (!bPlayerContext || !(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).canConstruct(eLoopBuilding, false, true)))
					{
						if (GC.getBuildingInfo(eLoopBuilding).getPrereqAndTech() == eTech)
						{
							szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_TECH_CAN_CONSTRUCT").c_str());
							szTempBuffer.Format( SETCOLR L"<link=literal>%s</link>" ENDCOLR , TEXT_COLOR("COLOR_BUILDING_TEXT"), GC.getBuildingInfo(eLoopBuilding).getDescription());
							setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", bFirst);
							bFirst = false;
						}
						else
						{
							for (int iJ = 0; iJ < GC.getDefineINT("NUM_BUILDING_AND_TECH_PREREQS"); iJ++)
							{
								if (GC.getBuildingInfo(eLoopBuilding).getPrereqAndTechs(iJ) == eTech)
								{
									szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_TECH_CAN_CONSTRUCT").c_str());
									szTempBuffer.Format( SETCOLR L"<link=literal>%s</link>" ENDCOLR , TEXT_COLOR("COLOR_BUILDING_TEXT"), GC.getBuildingInfo(eLoopBuilding).getDescription());
									setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", bFirst);
									bFirst = false;
									break;
								}
							}
						}
					}
				}
			}
		}

		bFirst = true;

		for (iI = 0; iI < GC.getNumProjectInfos(); iI++)
		{
			if (!bPlayerContext || !(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).isProductionMaxedProject((ProjectTypes)iI)))
			{
				if (!bPlayerContext || !(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).canCreate(((ProjectTypes)iI), false, true)))
				{
					if (GC.getProjectInfo((ProjectTypes)iI).getTechPrereq() == eTech)
					{
						szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_TECH_CAN_CREATE").c_str());
						szTempBuffer.Format( SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_PROJECT_TEXT"), GC.getProjectInfo((ProjectTypes)iI).getDescription());
						setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", bFirst);
						bFirst = false;
					}
				}
			}
		}
	}

	bFirst = true;

	for (iI = 0; iI < GC.getNumProcessInfos(); iI++)
	{
		bFirst = buildProcessInfoString(szBuffer, eTech, iI, bFirst, true, bPlayerContext);
	}

	bFirst = true;

	for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
	{
		if (!bPlayerContext || !(GC.getGameINLINE().isReligionFounded((ReligionTypes)iI)))
		{
			bFirst = buildFoundReligionString(szBuffer, eTech, iI, bFirst, true, bPlayerContext);
		}
	}

	bFirst = true;
	for (iI = 0; iI < GC.getNumPromotionInfos(); iI++)
	{
		bFirst = buildPromotionString(szBuffer, eTech, iI, bFirst, true, bPlayerContext);
	}

	if (bTreeInfo && NO_TECH == eFromTech)
	{
		buildSingleLineTechTreeString(szBuffer, eTech, bPlayerContext);
	}

	if (!isEmpty(GC.getTechInfo(eTech).getHelp()))
	{
		szBuffer += CvWString::format(L"%s%s", NEWLINE, GC.getTechInfo(eTech).getHelp()).c_str();
	}

	if (!bCivilopediaText)
	{
		if (GC.getGameINLINE().getActivePlayer() == NO_PLAYER)
		{
			szTempBuffer.Format(L"\n%d%c", GC.getTechInfo(eTech).getResearchCost(), GC.getCommerceInfo(COMMERCE_RESEARCH).getChar());
			szBuffer += szTempBuffer;
		}
		else if (GET_TEAM(GC.getGameINLINE().getActiveTeam()).isHasTech(eTech))
		{
			szTempBuffer.Format(L"\n%d%c", GET_TEAM(GC.getGameINLINE().getActiveTeam()).getResearchCost(eTech), GC.getCommerceInfo(COMMERCE_RESEARCH).getChar());
			szBuffer += szTempBuffer;
		}
		else
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_TECH_NUM_TURNS", GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getResearchTurnsLeft(eTech, (gDLL->ctrlKey() || !(gDLL->shiftKey()))));

			szTempBuffer.Format(L" (%d/%d %c)", GET_TEAM(GC.getGameINLINE().getActiveTeam()).getResearchProgress(eTech), GET_TEAM(GC.getGameINLINE().getActiveTeam()).getResearchCost(eTech), GC.getCommerceInfo(COMMERCE_RESEARCH).getChar());
			szBuffer += szTempBuffer;
		}
	}

	if (bStrategyText)
	{
		if (!isEmpty(GC.getTechInfo(eTech).getStrategy()))
		{
			if ((GC.getGameINLINE().getActivePlayer() == NO_PLAYER) || GET_PLAYER(GC.getGameINLINE().getActivePlayer()).isOption(PLAYEROPTION_ADVISOR_HELP))
			{
				szBuffer += SEPARATOR;
				szBuffer += NEWLINE;
				szBuffer += gDLL->getText("TXT_KEY_SIDS_TIPS");
				szBuffer += L"\"";
				szBuffer += GC.getTechInfo(eTech).getStrategy();
				szBuffer += L"\"";
			}
		}
	}
}


void CvGameTextMgr::setBasicUnitHelp(CvWString &szBuffer, UnitTypes eUnit, bool bCivilopediaText)
{
	CvWString szTempBuffer;
	bool bFirst;
	int iCount;
	int iI;

	if (NO_UNIT == eUnit)
	{
		return;
	}

	if (!bCivilopediaText)
	{
		szBuffer += NEWLINE;

		if (GC.getUnitInfo(eUnit).getDomainType() == DOMAIN_AIR)
		{
			if (GC.getUnitInfo(eUnit).getAirCombat() > 0)
			{
				szTempBuffer.Format(L"%d%c, ", GC.getUnitInfo(eUnit).getAirCombat(), gDLL->getSymbolID(STRENGTH_CHAR));
				szBuffer += szTempBuffer;
			}
		}
		else
		{
			if (GC.getUnitInfo(eUnit).getCombat() > 0)
			{
				szTempBuffer.Format(L"%d%c, ", GC.getUnitInfo(eUnit).getCombat(), gDLL->getSymbolID(STRENGTH_CHAR));
				szBuffer += szTempBuffer;
			}
		}

		szTempBuffer.Format(L"%d%c", GC.getUnitInfo(eUnit).getMoves(), gDLL->getSymbolID(MOVES_CHAR));
		szBuffer += szTempBuffer;

		if (GC.getUnitInfo(eUnit).getAirRange() > 0)
		{
			szBuffer += L", " + gDLL->getText("TXT_KEY_UNIT_AIR_RANGE", GC.getUnitInfo(eUnit).getAirRange());
		}
	}

	if (GC.getUnitInfo(eUnit).getCargoSpace() > 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_CARGO_SPACE", GC.getUnitInfo(eUnit).getCargoSpace());

		if (GC.getUnitInfo(eUnit).getSpecialCargo() != NO_SPECIALUNIT)
		{
			szBuffer += gDLL->getText("TXT_KEY_UNIT_CARRIES", GC.getSpecialUnitInfo((SpecialUnitTypes) GC.getUnitInfo(eUnit).getSpecialCargo()).getTextKeyWide());
		}
	}

	szTempBuffer.Format(L"%s%s ", NEWLINE, gDLL->getText("TXT_KEY_UNIT_CANNOT_ENTER").GetCString());

	bFirst = true;

	for (iI = 0; iI < GC.getNumTerrainInfos(); iI++)
	{
		if (GC.getUnitInfo(eUnit).getTerrainImpassable(iI))
		{
			CvWString szTerrain;
			szTerrain.Format(L"<link=literal>%s</link>", GC.getTerrainInfo((TerrainTypes)iI).getDescription());
			setListHelp(szBuffer, szTempBuffer, szTerrain, L", ", bFirst);
			bFirst = false;
		}
	}

	for (iI = 0; iI < GC.getNumFeatureInfos(); iI++)
	{
		if (GC.getUnitInfo(eUnit).getFeatureImpassable(iI))
		{
			CvWString szFeature;
			szFeature.Format(L"<link=literal>%s</link>", GC.getFeatureInfo((FeatureTypes)iI).getDescription());
			setListHelp(szBuffer, szTempBuffer, szFeature, L", ", bFirst);
			bFirst = false;
		}
	}

	if (GC.getUnitInfo(eUnit).isInvisible())
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_INVISIBLE_ALL");
	}
	else if (GC.getUnitInfo(eUnit).getInvisibleType() != NO_INVISIBLE)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_INVISIBLE_MOST");
	}

	if ((GC.getUnitInfo(eUnit).getSeeInvisibleType() != NO_INVISIBLE) && (bCivilopediaText || (GC.getUnitInfo(eUnit).getSeeInvisibleType() != GC.getUnitInfo(eUnit).getInvisibleType())))
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_SEE_INVISIBLE", GC.getInvisibleInfo((InvisibleTypes) GC.getUnitInfo(eUnit).getSeeInvisibleType()).getTextKeyWide());
	}

	if (GC.getUnitInfo(eUnit).isCanMoveImpassable())
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_CAN_MOVE_IMPASSABLE");
	}


	if (GC.getUnitInfo(eUnit).isNoBadGoodies())
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_NO_BAD_GOODIES");
	}

	if (GC.getUnitInfo(eUnit).isOnlyDefensive())
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_ONLY_DEFENSIVE");
	}

	if (GC.getUnitInfo(eUnit).isNoCapture())
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_CANNOT_CAPTURE");
	}

	if (GC.getUnitInfo(eUnit).isRivalTerritory())
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_EXPLORE_RIVAL");
	}

	if (GC.getUnitInfo(eUnit).isFound())
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_FOUND_CITY");
	}

	if (GC.getUnitInfo(eUnit).isGoldenAge())
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_GOLDEN_AGE");
	}

	if (GC.getUnitInfo(eUnit).getLeaderExperience() > 0)
	{
		if (0 == GC.getDefineINT("WARLORD_EXTRA_EXPERIENCE_PER_UNIT_PERCENT"))
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_LEADER", GC.getUnitInfo(eUnit).getLeaderExperience());
		}
		else
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_LEADER_EXPERIENCE", GC.getUnitInfo(eUnit).getLeaderExperience());
		}
	}

	if (NO_PROMOTION != GC.getUnitInfo(eUnit).getLeaderPromotion())
	{
		szBuffer += CvWString::format(L"%s%c%s", NEWLINE, gDLL->getSymbolID(BULLET_CHAR), gDLL->getText("TXT_KEY_PROMOTION_WHEN_LEADING").GetCString());
		szTempBuffer.clear();
		parsePromotionHelp(szTempBuffer, (PromotionTypes)GC.getUnitInfo(eUnit).getLeaderPromotion(), L"\n   ");
		szBuffer += szTempBuffer;
	}

	if ((GC.getUnitInfo(eUnit).getBaseDiscover() > 0) || (GC.getUnitInfo(eUnit).getDiscoverMultiplier() > 0))
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_DISCOVER_TECH");
	}

	if ((GC.getUnitInfo(eUnit).getBaseHurry() > 0) || (GC.getUnitInfo(eUnit).getHurryMultiplier() > 0))
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_HURRY_PRODUCTION");
	}

	if ((GC.getUnitInfo(eUnit).getBaseTrade() > 0) || (GC.getUnitInfo(eUnit).getTradeMultiplier() > 0))
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_TRADE_MISSION");
	}

	if (GC.getUnitInfo(eUnit).getGreatWorkCulture() > 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_GREAT_WORK", GC.getUnitInfo(eUnit).getGreatWorkCulture());
	}

	iCount = 0;

	for (iI = 0; iI < GC.getNumBuildInfos(); iI++)
	{
		if (GC.getUnitInfo(eUnit).getBuilds(iI))
		{
			iCount++;
		}
	}

	if (iCount > ((GC.getNumBuildInfos() * 3) / 4))
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_IMPROVE_PLOTS");
	}
	else
	{
		bFirst = true;

		for (iI = 0; iI < GC.getNumBuildInfos(); iI++)
		{
			if (GC.getUnitInfo(eUnit).getBuilds(iI))
			{
				szTempBuffer.Format(L"%s%s ", NEWLINE, gDLL->getText("TXT_KEY_UNIT_CAN").c_str());
				setListHelp(szBuffer, szTempBuffer, GC.getBuildInfo((BuildTypes) iI).getDescription(), L", ", bFirst);
				bFirst = false;
			}
		}
	}

	if (GC.getUnitInfo(eUnit).getNukeRange() != -1)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_CAN_NUKE");
	}

	if (GC.getUnitInfo(eUnit).isCounterSpy())
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_EXPOSE_SPIES");
	}

	if ((GC.getUnitInfo(eUnit).getFirstStrikes() + GC.getUnitInfo(eUnit).getChanceFirstStrikes()) > 0)
	{
		if (GC.getUnitInfo(eUnit).getChanceFirstStrikes() == 0)
		{
			if (GC.getUnitInfo(eUnit).getFirstStrikes() == 1)
			{
				szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_ONE_FIRST_STRIKE");
			}
			else
			{
				szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_NUM_FIRST_STRIKES", GC.getUnitInfo(eUnit).getFirstStrikes());
			}
		}
		else
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_FIRST_STRIKE_CHANCES", GC.getUnitInfo(eUnit).getFirstStrikes(), GC.getUnitInfo(eUnit).getFirstStrikes() + GC.getUnitInfo(eUnit).getChanceFirstStrikes());
		}
	}

	if (GC.getUnitInfo(eUnit).isFirstStrikeImmune())
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_IMMUNE_FIRST_STRIKES");
	}

	if (GC.getUnitInfo(eUnit).isNoDefensiveBonus())
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_NO_DEFENSE_BONUSES");
	}

	if (GC.getUnitInfo(eUnit).isFlatMovementCost())
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_FLAT_MOVEMENT");
	}

	if (GC.getUnitInfo(eUnit).isIgnoreTerrainCost())
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_IGNORE_TERRAIN");
	}

	if (GC.getUnitInfo(eUnit).getInterceptionProbability() > 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_INTERCEPT_AIRCRAFT", GC.getUnitInfo(eUnit).getInterceptionProbability());
	}

	if (GC.getUnitInfo(eUnit).getEvasionProbability() > 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_EVADE_INTERCEPTION", GC.getUnitInfo(eUnit).getEvasionProbability());
	}

	if (GC.getUnitInfo(eUnit).getWithdrawalProbability() > 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_WITHDRAWL_PROBABILITY", GC.getUnitInfo(eUnit).getWithdrawalProbability());
	}

	if (GC.getUnitInfo(eUnit).getCollateralDamage() > 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_COLLATERAL_DAMAGE");
	}

	if (GC.getUnitInfo(eUnit).getCityAttackModifier() == GC.getUnitInfo(eUnit).getCityDefenseModifier())
	{
		if (GC.getUnitInfo(eUnit).getCityAttackModifier() != 0)
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_CITY_STRENGTH_MOD", GC.getUnitInfo(eUnit).getCityAttackModifier());
		}
	}
	else
	{
		if (GC.getUnitInfo(eUnit).getCityAttackModifier() != 0)
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_CITY_ATTACK_MOD", GC.getUnitInfo(eUnit).getCityAttackModifier());
		}

		if (GC.getUnitInfo(eUnit).getCityDefenseModifier() != 0)
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_CITY_DEFENSE_MOD", GC.getUnitInfo(eUnit).getCityDefenseModifier());
		}
	}

	if (GC.getUnitInfo(eUnit).getAnimalCombatModifier() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_ANIMAL_COMBAT_MOD", GC.getUnitInfo(eUnit).getAnimalCombatModifier());
	}

	if (GC.getUnitInfo(eUnit).getHillsDefenseModifier() == GC.getUnitInfo(eUnit).getHillsAttackModifier())
	{
		if (GC.getUnitInfo(eUnit).getHillsAttackModifier() != 0)
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_HILLS_STRENGTH", GC.getUnitInfo(eUnit).getHillsAttackModifier());
		}
	}
	else
	{
		if (GC.getUnitInfo(eUnit).getHillsAttackModifier() != 0)
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_HILLS_ATTACK", GC.getUnitInfo(eUnit).getHillsAttackModifier());
		}

		if (GC.getUnitInfo(eUnit).getHillsDefenseModifier() != 0)
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_HILLS_DEFENSE", GC.getUnitInfo(eUnit).getHillsDefenseModifier());
		}
	}

	for (iI = 0; iI < GC.getNumTerrainInfos(); iI++)
	{
		if (GC.getUnitInfo(eUnit).getTerrainDefenseModifier(iI) != 0)
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_DEFENSE", GC.getUnitInfo(eUnit).getTerrainDefenseModifier(iI), GC.getTerrainInfo((TerrainTypes) iI).getTextKeyWide());
		}
	}

	for (iI = 0; iI < GC.getNumFeatureInfos(); iI++)
	{
		if (GC.getUnitInfo(eUnit).getFeatureDefenseModifier(iI) != 0)
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_DEFENSE", GC.getUnitInfo(eUnit).getFeatureDefenseModifier(iI), GC.getFeatureInfo((FeatureTypes) iI).getTextKeyWide());
		}
	}

	for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
	{
		if (GC.getUnitInfo(eUnit).getUnitClassAttackModifier(iI) == GC.getUnitInfo(eUnit).getUnitClassDefenseModifier(iI))
		{
			if (GC.getUnitInfo(eUnit).getUnitClassAttackModifier(iI) != 0)
			{
				szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_MOD_VS_TYPE", GC.getUnitInfo(eUnit).getUnitClassAttackModifier(iI), GC.getUnitClassInfo((UnitClassTypes)iI).getTextKeyWide());
			}
		}
		else
		{
			if (GC.getUnitInfo(eUnit).getUnitClassAttackModifier(iI) != 0)
			{
				szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_ATTACK_MOD_VS_CLASS", GC.getUnitInfo(eUnit).getUnitClassAttackModifier(iI), GC.getUnitClassInfo((UnitClassTypes)iI).getTextKeyWide());
			}

			if (GC.getUnitInfo(eUnit).getUnitClassDefenseModifier(iI) != 0)
			{
				szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_DEFENSE_MOD_VS_CLASS", GC.getUnitInfo(eUnit).getUnitClassDefenseModifier(iI), GC.getUnitClassInfo((UnitClassTypes) iI).getTextKeyWide());
			}
		}
	}

	for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
	{
		if (GC.getUnitInfo(eUnit).getUnitCombatModifier(iI) != 0)
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_MOD_VS_TYPE", GC.getUnitInfo(eUnit).getUnitCombatModifier(iI), GC.getUnitCombatInfo((UnitCombatTypes) iI).getTextKeyWide());
		}
	}

	for (iI = 0; iI < NUM_DOMAIN_TYPES; iI++)
	{
		if (GC.getUnitInfo(eUnit).getDomainModifier(iI) != 0)
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_MOD_VS_TYPE_NO_LINK", GC.getUnitInfo(eUnit).getDomainModifier(iI), GC.getDomainInfo((DomainTypes)iI).getTextKeyWide());
		}
	}

	szTempBuffer.clear();
	bFirst = true;
	for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
	{
		if (GC.getUnitInfo(eUnit).getTargetUnitClass(iI))
		{
			if (bFirst)
			{
				bFirst = false;
			}
			else
			{
				szTempBuffer += L", ";
			}

			szTempBuffer += CvWString::format(L"<link=literal>%s</link>", GC.getUnitClassInfo((UnitClassTypes)iI).getDescription());
		}
	}

	if (!bFirst)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_TARGETS_UNIT_FIRST", szTempBuffer.GetCString());
	}

	szTempBuffer.clear();
	bFirst = true;
	for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
	{
		if (GC.getUnitInfo(eUnit).getDefenderUnitClass(iI))
		{
			if (bFirst)
			{
				bFirst = false;
			}
			else
			{
				szTempBuffer += L", ";
			}

			szTempBuffer += CvWString::format(L"<link=literal>%s</link>", GC.getUnitClassInfo((UnitClassTypes)iI).getDescription());
		}
	}

	if (!bFirst)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_DEFENDS_UNIT_FIRST", szTempBuffer.GetCString());
	}

	szTempBuffer.clear();
	bFirst = true;
	for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
	{
		if (GC.getUnitInfo(eUnit).getTargetUnitCombat(iI))
		{
			if (bFirst)
			{
				bFirst = false;
			}
			else
			{
				szTempBuffer += L", ";
			}

			szTempBuffer += CvWString::format(L"<link=literal>%s</link>", GC.getUnitCombatInfo((UnitCombatTypes)iI).getDescription());
		}
	}

	if (!bFirst)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_TARGETS_UNIT_FIRST", szTempBuffer.GetCString());
	}

	szTempBuffer.clear();
	bFirst = true;
	for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
	{
		if (GC.getUnitInfo(eUnit).getDefenderUnitCombat(iI))
		{
			if (bFirst)
			{
				bFirst = false;
			}
			else
			{
				szTempBuffer += L", ";
			}

			szTempBuffer += CvWString::format(L"<link=literal>%s</link>", GC.getUnitCombatInfo((UnitCombatTypes)iI).getDescription());
		}
	}

	if (!bFirst)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_DEFENDS_UNIT_FIRST", szTempBuffer.GetCString());
	}

	if (GC.getUnitInfo(eUnit).getBombRate() > 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_BOMB_RATE", ((GC.getUnitInfo(eUnit).getBombRate() * 100) / GC.getMAX_CITY_DEFENSE_DAMAGE()));
	}

	if (GC.getUnitInfo(eUnit).getBombardRate() > 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_BOMBARD_RATE", ((GC.getUnitInfo(eUnit).getBombardRate() * 100) / GC.getMAX_CITY_DEFENSE_DAMAGE()));
	}

	bFirst = true;

	for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
	{
		if (GC.getUnitInfo(eUnit).getReligionSpreads(iI))
		{
			szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_UNIT_CAN_SPREAD").c_str());
			CvWString szReligion;
			szReligion.Format(L"<link=literal>%s</link>", GC.getReligionInfo((ReligionTypes) iI).getDescription());
			setListHelp(szBuffer, szTempBuffer, szReligion, L", ", bFirst);
			bFirst = false;
		}
	}

	bFirst = true;

	for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
	{
		if (GC.getUnitInfo(eUnit).getGreatPeoples(iI))
		{
			szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_UNIT_CAN_JOIN").c_str());
			CvWString szSpecialistLink = CvWString::format(L"<link=literal>%s</link>", GC.getSpecialistInfo((SpecialistTypes) iI).getDescription());
			setListHelp(szBuffer, szTempBuffer, szSpecialistLink.GetCString(), L", ", bFirst);
			bFirst = false;
		}
	}

	bFirst = true;

	for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
	{
		if (GC.getUnitInfo(eUnit).getBuildings(iI) || GC.getUnitInfo(eUnit).getForceBuildings(iI))
		{
			szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_UNIT_CAN_CONSTRUCT").c_str());
			CvWString szBuildingLink = CvWString::format(L"<link=literal>%s</link>", GC.getBuildingInfo((BuildingTypes) iI).getDescription());
			setListHelp(szBuffer, szTempBuffer, szBuildingLink.GetCString(), L", ", bFirst);
			bFirst = false;
		}
	}

	bFirst = true;

	for (iI = 0; iI < GC.getNumPromotionInfos(); iI++)
	{
		if (GC.getUnitInfo(eUnit).getFreePromotions(iI))
		{
			szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_UNIT_STARTS_WITH").c_str());
			setListHelp(szBuffer, szTempBuffer, CvWString::format(L"<link=literal>%s</link>", GC.getPromotionInfo((PromotionTypes) iI).getDescription()), L", ", bFirst);
			bFirst = false;
		}
	}

	if (GC.getUnitInfo(eUnit).getExtraCost() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_EXTRA_COST", GC.getUnitInfo(eUnit).getExtraCost());
	}

	if (!isEmpty(GC.getUnitInfo(eUnit).getHelp()))
	{
		szBuffer += NEWLINE;
		szBuffer += GC.getUnitInfo(eUnit).getHelp();
	}
}


void CvGameTextMgr::setUnitHelp(CvWString &szBuffer, UnitTypes eUnit, bool bCivilopediaText, bool bStrategyText, bool bTechChooserText, CvCity* pCity)
{
	CvWString szTempBuffer;
	PlayerTypes ePlayer;
	bool bFirst;
	int iProduction;
	int iI;

	if (NO_UNIT == eUnit)
	{
		return;
	}

	if (pCity != NULL)
	{
		ePlayer = pCity->getOwnerINLINE();
	}
	else
	{
		ePlayer = GC.getGameINLINE().getActivePlayer();
	}

	if (!bCivilopediaText)
	{
		szTempBuffer.Format(SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_UNIT_TEXT"), GC.getUnitInfo(eUnit).getDescription());
		szBuffer += szTempBuffer;

		if (GC.getUnitInfo(eUnit).getUnitCombatType() != NO_UNITCOMBAT)
		{
			szTempBuffer.Format(L" (%s)", GC.getUnitCombatInfo((UnitCombatTypes) GC.getUnitInfo(eUnit).getUnitCombatType()).getDescription());
			szBuffer += szTempBuffer;
		}
	}

	// test for unique unit
	UnitClassTypes eUnitClass = (UnitClassTypes)GC.getUnitInfo(eUnit).getUnitClassType();
	UnitTypes eDefaultUnit = (UnitTypes)GC.getUnitClassInfo(eUnitClass).getDefaultUnitIndex();

	if (NO_UNIT != eDefaultUnit && eDefaultUnit != eUnit)
	{
		for (iI  = 0; iI < GC.getNumCivilizationInfos(); iI++)
		{
			UnitTypes eUniqueUnit = (UnitTypes)GC.getCivilizationInfo((CivilizationTypes)iI).getCivilizationUnits((int)eUnitClass);
			if (eUniqueUnit == eUnit)
			{
				szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIQUE_UNIT", GC.getCivilizationInfo((CivilizationTypes)iI).getTextKeyWide());
			}
		}
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_REPLACES_UNIT", GC.getUnitInfo(eDefaultUnit).getTextKeyWide());
	}

	if (isWorldUnitClass(eUnitClass))
	{
		if (pCity == NULL)
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_WORLD_UNIT_ALLOWED", GC.getUnitClassInfo(eUnitClass).getMaxGlobalInstances());
		}
		else
		{
			szBuffer += gDLL->getText("TXT_KEY_UNIT_WORLD_UNIT_LEFT", (GC.getUnitClassInfo(eUnitClass).getMaxGlobalInstances() - (ePlayer != NO_PLAYER ? GC.getGameINLINE().getUnitClassCreatedCount(eUnitClass) + GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getUnitClassMaking(eUnitClass) : 0)));
		}
	}

	if (isTeamUnitClass(eUnitClass))
	{
		if (pCity == NULL)
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_TEAM_UNIT_ALLOWED", GC.getUnitClassInfo(eUnitClass).getMaxTeamInstances());
		}
		else
		{
			szBuffer += gDLL->getText("TXT_KEY_UNIT_TEAM_UNIT_LEFT", (GC.getUnitClassInfo(eUnitClass).getMaxTeamInstances() - (ePlayer != NO_PLAYER ? GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getUnitClassCountPlusMaking(eUnitClass) : 0)));
		}
	}

	if (isNationalUnitClass(eUnitClass))
	{
		if (pCity == NULL)
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_NATIONAL_UNIT_ALLOWED", GC.getUnitClassInfo(eUnitClass).getMaxPlayerInstances());
		}
		else
		{
			szBuffer += gDLL->getText("TXT_KEY_UNIT_NATIONAL_UNIT_LEFT", (GC.getUnitClassInfo(eUnitClass).getMaxPlayerInstances() - (ePlayer != NO_PLAYER ? GET_PLAYER(ePlayer).getUnitClassCountPlusMaking(eUnitClass) : 0)));
		}
	}

	setBasicUnitHelp(szBuffer, eUnit, bCivilopediaText);

	if ((pCity == NULL) || !(pCity->canTrain(eUnit)))
	{
		if (pCity != NULL)
		{
			if (GC.getGameINLINE().isNoNukes())
			{
				if (GC.getUnitInfo(eUnit).getNukeRange() != -1)
				{
					szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_NO_NUKES");
				}
			}
		}

		if (GC.getUnitInfo(eUnit).getHolyCity() != NO_RELIGION)
		{
			if ((pCity == NULL) || !(pCity->isHolyCity((ReligionTypes)(GC.getUnitInfo(eUnit).getHolyCity()))))
			{
				szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_REQUIRES_HOLY_CITY", GC.getReligionInfo((ReligionTypes)(GC.getUnitInfo(eUnit).getHolyCity())).getChar());
			}
		}

		bFirst = true;

		if (GC.getUnitInfo(eUnit).getSpecialUnitType() != NO_SPECIALUNIT)
		{
			if ((pCity == NULL) || !(GC.getGameINLINE().isSpecialUnitValid((SpecialUnitTypes)(GC.getUnitInfo(eUnit).getSpecialUnitType()))))
			{
				for (iI = 0; iI < GC.getNumProjectInfos(); iI++)
				{
					if (GC.getProjectInfo((ProjectTypes)iI).getEveryoneSpecialUnit() == GC.getUnitInfo(eUnit).getSpecialUnitType())
					{
						szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_REQUIRES").c_str());
						CvWString szProject;
						szProject.Format(L"<link=literal>%s</link>", GC.getProjectInfo((ProjectTypes)iI).getDescription());
						setListHelp(szBuffer, szTempBuffer, szProject, gDLL->getText("TXT_KEY_OR").c_str(), bFirst);
						bFirst = false;
					}
				}
			}
		}

		if (!bFirst)
		{
			szBuffer += ENDCOLR;
		}

		if (!bCivilopediaText)
		{
			if (GC.getUnitInfo(eUnit).getPrereqBuilding() != NO_BUILDING)
			{
				if ((pCity == NULL) || !(pCity->hasBuilding((BuildingTypes)(GC.getUnitInfo(eUnit).getPrereqBuilding()))))
				{
					szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_REQUIRES_STRING", GC.getBuildingInfo((BuildingTypes)(GC.getUnitInfo(eUnit).getPrereqBuilding())).getTextKeyWide());
				}
			}

			if (!bTechChooserText)
			{
				if (GC.getUnitInfo(eUnit).getPrereqAndTech() != NO_TECH)
				{
					if (GC.getGameINLINE().getActivePlayer() == NO_PLAYER || !(GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isHasTech((TechTypes)(GC.getUnitInfo(eUnit).getPrereqAndTech()))))
					{
						szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_REQUIRES_STRING", GC.getTechInfo((TechTypes)(GC.getUnitInfo(eUnit).getPrereqAndTech())).getTextKeyWide());
					}
				}
			}

			bFirst = true;

			for (iI = 0; iI < GC.getDefineINT("NUM_UNIT_AND_TECH_PREREQS"); iI++)
			{
				if (GC.getUnitInfo(eUnit).getPrereqAndTechs(iI) != NO_TECH)
				{
					if (bTechChooserText || GC.getGameINLINE().getActivePlayer() == NO_PLAYER || !(GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isHasTech((TechTypes)(GC.getUnitInfo(eUnit).getPrereqAndTechs(iI)))))
					{
						szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_REQUIRES").c_str());
						setListHelp(szBuffer, szTempBuffer, GC.getTechInfo(((TechTypes)(GC.getUnitInfo(eUnit).getPrereqAndTechs(iI)))).getDescription(), gDLL->getText("TXT_KEY_AND").c_str(), bFirst);
						bFirst = false;
					}
				}
			}

			if (!bFirst)
			{
				szBuffer += ENDCOLR;
			}

			if (GC.getUnitInfo(eUnit).getPrereqAndBonus() != NO_BONUS)
			{
				if ((pCity == NULL) || !(pCity->hasBonus((BonusTypes)GC.getUnitInfo(eUnit).getPrereqAndBonus())))
				{
					szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_REQUIRES_STRING", GC.getBonusInfo((BonusTypes)(GC.getUnitInfo(eUnit).getPrereqAndBonus())).getTextKeyWide());
				}
			}

			bFirst = true;

			for (iI = 0; iI < GC.getNUM_UNIT_PREREQ_OR_BONUSES(); iI++)
			{
				if (GC.getUnitInfo(eUnit).getPrereqOrBonuses(iI) != NO_BONUS)
				{
					if ((pCity == NULL) || !(pCity->hasBonus((BonusTypes)GC.getUnitInfo(eUnit).getPrereqOrBonuses(iI))))
					{
						szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_REQUIRES").c_str());
						setListHelp(szBuffer, szTempBuffer, GC.getBonusInfo((BonusTypes) GC.getUnitInfo(eUnit).getPrereqOrBonuses(iI)).getDescription(), gDLL->getText("TXT_KEY_OR").c_str(), bFirst);
						bFirst = false;
					}
				}
			}

			if (!bFirst)
			{
				szBuffer += ENDCOLR;
			}
		}
	}

	if (!bCivilopediaText && GC.getGameINLINE().getActivePlayer() != NO_PLAYER)
	{
		if (pCity == NULL)
		{
			szTempBuffer.Format(L"%s%d%c", NEWLINE, GET_PLAYER(ePlayer).getProductionNeeded(eUnit), GC.getYieldInfo(YIELD_PRODUCTION).getChar());
			szBuffer += szTempBuffer;
		}
		else
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_TURNS", pCity->getProductionTurnsLeft(eUnit, ((gDLL->ctrlKey() || !(gDLL->shiftKey())) ? 0 : pCity->getOrderQueueLength())), GET_PLAYER(ePlayer).getProductionNeeded(eUnit), GC.getYieldInfo(YIELD_PRODUCTION).getChar());

			iProduction = pCity->getUnitProduction(eUnit);

			if (iProduction > 0)
			{
				szTempBuffer.Format(L" - %d/%d%c", iProduction, GET_PLAYER(ePlayer).getProductionNeeded(eUnit), GC.getYieldInfo(YIELD_PRODUCTION).getChar());
				szBuffer += szTempBuffer;
			}
			else
			{
				szTempBuffer.Format(L" - %d%c", GET_PLAYER(ePlayer).getProductionNeeded(eUnit), GC.getYieldInfo(YIELD_PRODUCTION).getChar());
				szBuffer += szTempBuffer;
			}
		}
	}

	for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
	{
		if (GC.getUnitInfo(eUnit).getBonusProductionModifier(iI) != 0)
		{
			if (pCity != NULL)
			{
				if (pCity->hasBonus((BonusTypes)iI))
				{
					szBuffer += gDLL->getText("TXT_KEY_COLOR_POSITIVE");
				}
				else
				{
					szBuffer += gDLL->getText("TXT_KEY_COLOR_NEGATIVE");
				}
			}
			if (!bCivilopediaText)
			{
				szBuffer += L" (";
			}
			else
			{
				szTempBuffer.Format(L"%s%c", NEWLINE, gDLL->getSymbolID(BULLET_CHAR), szTempBuffer);
				szBuffer += szTempBuffer;
			}
			if (GC.getUnitInfo(eUnit).getBonusProductionModifier(iI) == 100)
			{
				szBuffer += gDLL->getText("TXT_KEY_UNIT_DOUBLE_SPEED", GC.getBonusInfo((BonusTypes)iI).getTextKeyWide());
			}
			else
			{
				szBuffer += gDLL->getText("TXT_KEY_UNIT_BUILDS_FASTER", GC.getUnitInfo(eUnit).getBonusProductionModifier(iI), GC.getBonusInfo((BonusTypes)iI).getTextKeyWide());
			}
			if (!bCivilopediaText)
			{
				szBuffer += L")";
			}
			if (pCity != NULL)
			{
				szBuffer += gDLL->getText("TXT_KEY_COLOR_REVERT");
			}
		}
	}

	if (bStrategyText)
	{
		if (!isEmpty(GC.getUnitInfo(eUnit).getStrategy()))
		{
			if ((ePlayer == NO_PLAYER) || GET_PLAYER(ePlayer).isOption(PLAYEROPTION_ADVISOR_HELP))
			{
				szBuffer += SEPARATOR;
				szBuffer += NEWLINE;
				szBuffer += gDLL->getText("TXT_KEY_SIDS_TIPS");
				szBuffer += L"\"";
				szBuffer += GC.getUnitInfo(eUnit).getStrategy();
				szBuffer += L"\"";
			}
		}
	}
}

void CvGameTextMgr::setBuildingHelp(CvWString &szBuffer, BuildingTypes eBuilding, bool bCivilopediaText, bool bStrategyText, bool bTechChooserText, CvCity* pCity)
{
	CvWString szFirstBuffer;
	CvWString szTempBuffer;
	BuildingTypes eLoopBuilding;
	UnitTypes eGreatPeopleUnit;
	PlayerTypes ePlayer;
	bool bFirst;
	int iProduction;
	int iLast;
	int iI;

	if (NO_BUILDING == eBuilding)
	{
		return;
	}

	if (pCity != NULL)
	{
		ePlayer = pCity->getOwnerINLINE();
	}
	else
	{
		ePlayer = GC.getGameINLINE().getActivePlayer();
	}

	if (!bCivilopediaText)
	{
		szTempBuffer.Format( SETCOLR L"<link=literal>%s</link>" ENDCOLR , TEXT_COLOR("COLOR_BUILDING_TEXT"), GC.getBuildingInfo(eBuilding).getDescription());
		szBuffer += szTempBuffer;

		int iHappiness = GC.getBuildingInfo(eBuilding).getHappiness();
		if (ePlayer != NO_PLAYER)
		{
			if (eBuilding == GC.getCivilizationInfo(GET_PLAYER(ePlayer).getCivilizationType()).getCivilizationBuildings(GC.getBuildingInfo(eBuilding).getBuildingClassType()))
			{
				iHappiness += GET_PLAYER(ePlayer).getExtraBuildingHappiness(eBuilding);
			}
		}

		if (iHappiness != 0)
		{
			szTempBuffer.Format(L", +%d%c", iHappiness, ((iHappiness > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)));
			szBuffer += szTempBuffer;
		}

		if (GC.getBuildingInfo(eBuilding).getHealth() != 0)
		{
			szTempBuffer.Format(L", +%d%c", abs(GC.getBuildingInfo(eBuilding).getHealth()), ((GC.getBuildingInfo(eBuilding).getHealth() > 0) ? gDLL->getSymbolID(HEALTHY_CHAR): gDLL->getSymbolID(UNHEALTHY_CHAR)));
			szBuffer += szTempBuffer;
		}

		setYieldChangeHelp(szBuffer, L", ", L"", L"", GC.getBuildingInfo(eBuilding).getYieldChangeArray(), false, false);

		setCommerceChangeHelp(szBuffer, L", ", L"", L"", GC.getBuildingInfo(eBuilding).getCommerceChangeArray(), false, false);

		setCommerceChangeHelp(szBuffer, L", ", L"", L"", GC.getBuildingInfo(eBuilding).getObsoleteSafeCommerceChangeArray(), false, false); // XXX what if building gives gold... does that make it in the Civilopedia?

		setYieldChangeHelp(szBuffer, L", ", L"", L"", GC.getBuildingInfo(eBuilding).getYieldModifierArray(), true, bCivilopediaText);

		setCommerceChangeHelp(szBuffer, L", ", L"", L"", GC.getBuildingInfo(eBuilding).getCommerceModifierArray(), true, bCivilopediaText);

		if (GC.getBuildingInfo(eBuilding).getGreatPeopleRateChange() != 0)
		{
			szTempBuffer.Format(L", %s%d%c", ((GC.getBuildingInfo(eBuilding).getGreatPeopleRateChange() > 0) ? "+" : ""), GC.getBuildingInfo(eBuilding).getGreatPeopleRateChange(), gDLL->getSymbolID(GREAT_PEOPLE_CHAR));
			szBuffer += szTempBuffer;

			if (GC.getBuildingInfo(eBuilding).getGreatPeopleUnitClass() != NO_UNITCLASS)
			{
				if (ePlayer != NO_PLAYER)
				{
					eGreatPeopleUnit = ((UnitTypes)(GC.getCivilizationInfo(GET_PLAYER(ePlayer).getCivilizationType()).getCivilizationUnits(GC.getBuildingInfo(eBuilding).getGreatPeopleUnitClass())));
				}
				else
				{
					eGreatPeopleUnit = (UnitTypes)GC.getUnitClassInfo((UnitClassTypes)GC.getBuildingInfo(eBuilding).getGreatPeopleUnitClass()).getDefaultUnitIndex();
				}

				if (eGreatPeopleUnit != NO_UNIT)
				{
					szTempBuffer.Format(L" (%s)", GC.getUnitInfo(eGreatPeopleUnit).getDescription());
					szBuffer += szTempBuffer;
				}
			}
		}
	}

	// test for unique building
	BuildingClassTypes eBuildingClass = (BuildingClassTypes)GC.getBuildingInfo(eBuilding).getBuildingClassType();
	BuildingTypes eDefaultBuilding = (BuildingTypes)GC.getBuildingClassInfo(eBuildingClass).getDefaultBuildingIndex();

	if (NO_BUILDING != eDefaultBuilding && eDefaultBuilding != eBuilding)
	{
		for (iI  = 0; iI < GC.getNumCivilizationInfos(); iI++)
		{
			BuildingTypes eUniqueBuilding = (BuildingTypes)GC.getCivilizationInfo((CivilizationTypes)iI).getCivilizationBuildings((int)eBuildingClass);
			if (eUniqueBuilding == eBuilding)
			{
				szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIQUE_BUILDING", GC.getCivilizationInfo((CivilizationTypes)iI).getTextKeyWide());
			}
		}
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_REPLACES_UNIT", GC.getBuildingInfo(eDefaultBuilding).getTextKeyWide());
	}

	if (bCivilopediaText)
	{
		setYieldChangeHelp(szBuffer, L"", L"", L"", GC.getBuildingInfo(eBuilding).getYieldModifierArray(), true, bCivilopediaText);

		setCommerceChangeHelp(szBuffer, L"", L"", L"", GC.getBuildingInfo(eBuilding).getCommerceModifierArray(), true, bCivilopediaText);
	}
	else
	{
		if (isWorldWonderClass((BuildingClassTypes)(GC.getBuildingInfo(eBuilding).getBuildingClassType())))
		{
			if (pCity == NULL || ePlayer == NO_PLAYER)
			{
				szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_WORLD_WONDER_ALLOWED", GC.getBuildingClassInfo((BuildingClassTypes) GC.getBuildingInfo(eBuilding).getBuildingClassType()).getMaxGlobalInstances());
			}
			else
			{
				szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_WORLD_WONDER_LEFT", (GC.getBuildingClassInfo((BuildingClassTypes) GC.getBuildingInfo(eBuilding).getBuildingClassType()).getMaxGlobalInstances() - GC.getGameINLINE().getBuildingClassCreatedCount((BuildingClassTypes)(GC.getBuildingInfo(eBuilding).getBuildingClassType())) - GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getBuildingClassMaking((BuildingClassTypes)(GC.getBuildingInfo(eBuilding).getBuildingClassType()))));
			}
		}

		if (isTeamWonderClass((BuildingClassTypes)(GC.getBuildingInfo(eBuilding).getBuildingClassType())))
		{
			if (pCity == NULL || ePlayer == NO_PLAYER)
			{
				szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_TEAM_WONDER_ALLOWED", GC.getBuildingClassInfo((BuildingClassTypes) GC.getBuildingInfo(eBuilding).getBuildingClassType()).getMaxTeamInstances());
			}
			else
			{
				szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_TEAM_WONDER_LEFT", (GC.getBuildingClassInfo((BuildingClassTypes) GC.getBuildingInfo(eBuilding).getBuildingClassType()).getMaxTeamInstances() - GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getBuildingClassCountPlusMaking((BuildingClassTypes)(GC.getBuildingInfo(eBuilding).getBuildingClassType()))));
			}
		}

		if (isNationalWonderClass((BuildingClassTypes)(GC.getBuildingInfo(eBuilding).getBuildingClassType())))
		{
			if (pCity == NULL || ePlayer == NO_PLAYER)
			{
				szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_NATIONAL_WONDER_ALLOWED", GC.getBuildingClassInfo((BuildingClassTypes) GC.getBuildingInfo(eBuilding).getBuildingClassType()).getMaxPlayerInstances());
			}
			else
			{
				szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_NATIONAL_WONDER_LEFT", (GC.getBuildingClassInfo((BuildingClassTypes) GC.getBuildingInfo(eBuilding).getBuildingClassType()).getMaxPlayerInstances() - GET_PLAYER(ePlayer).getBuildingClassCountPlusMaking((BuildingClassTypes)(GC.getBuildingInfo(eBuilding).getBuildingClassType()))));
			}
		}
	}

	if (GC.getBuildingInfo(eBuilding).getGlobalReligionCommerce() != NO_RELIGION)
	{
		szFirstBuffer = gDLL->getText("TXT_KEY_BUILDING_PER_CITY_WITH", GC.getReligionInfo((ReligionTypes) GC.getBuildingInfo(eBuilding).getGlobalReligionCommerce()).getChar());
		setCommerceChangeHelp(szBuffer, L"", L"", szFirstBuffer, GC.getReligionInfo((ReligionTypes) GC.getBuildingInfo(eBuilding).getGlobalReligionCommerce()).getGlobalReligionCommerceArray());
	}

	if (GC.getBuildingInfo(eBuilding).getFreeBonus() != NO_BONUS)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_PROVIDES", GC.getGameINLINE().getNumFreeBonuses(eBuilding), GC.getBonusInfo((BonusTypes) GC.getBuildingInfo(eBuilding).getFreeBonus()).getTextKeyWide(), GC.getBonusInfo((BonusTypes) GC.getBuildingInfo(eBuilding).getFreeBonus()).getChar());

		if (GC.getBonusInfo((BonusTypes)(GC.getBuildingInfo(eBuilding).getFreeBonus())).getHealth() != 0)
		{
			szTempBuffer.Format(L", +%d%c", abs(GC.getBonusInfo((BonusTypes) GC.getBuildingInfo(eBuilding).getFreeBonus()).getHealth()), ((GC.getBonusInfo((BonusTypes)(GC.getBuildingInfo(eBuilding).getFreeBonus())).getHealth() > 0) ? gDLL->getSymbolID(HEALTHY_CHAR): gDLL->getSymbolID(UNHEALTHY_CHAR)));
			szBuffer += szTempBuffer;
		}

		if (GC.getBonusInfo((BonusTypes)(GC.getBuildingInfo(eBuilding).getFreeBonus())).getHappiness() != 0)
		{
			szTempBuffer.Format(L", +%d%c", abs(GC.getBonusInfo((BonusTypes) GC.getBuildingInfo(eBuilding).getFreeBonus()).getHappiness()), ((GC.getBonusInfo((BonusTypes)(GC.getBuildingInfo(eBuilding).getFreeBonus())).getHappiness() > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)));
			szBuffer += szTempBuffer;
		}
	}

	if (GC.getBuildingInfo(eBuilding).getFreeBuildingClass() != NO_BUILDINGCLASS)
	{
		BuildingTypes eFreeBuilding;
		if (ePlayer != NO_PLAYER)
		{
			eFreeBuilding = ((BuildingTypes)(GC.getCivilizationInfo(GET_PLAYER(ePlayer).getCivilizationType()).getCivilizationBuildings(GC.getBuildingInfo(eBuilding).getFreeBuildingClass())));
		}
		else
		{
			eFreeBuilding = (BuildingTypes)GC.getBuildingClassInfo((BuildingClassTypes)GC.getBuildingInfo(eBuilding).getFreeBuildingClass()).getDefaultBuildingIndex();
		}

		if (NO_BUILDING != eFreeBuilding)
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_FREE_IN_CITY", GC.getBuildingInfo(eFreeBuilding).getTextKeyWide());
		}
	}

	if (GC.getBuildingInfo(eBuilding).getFreePromotion() != NO_PROMOTION)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_FREE_PROMOTION", GC.getPromotionInfo((PromotionTypes)(GC.getBuildingInfo(eBuilding).getFreePromotion())).getTextKeyWide());
	}

	if (GC.getBuildingInfo(eBuilding).getCivicOption() != NO_CIVICOPTION)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_ENABLES_CIVICS", GC.getCivicOptionInfo((CivicOptionTypes)(GC.getBuildingInfo(eBuilding).getCivicOption())).getTextKeyWide());
	}

	if (GC.getBuildingInfo(eBuilding).isPower())
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_PROVIDES_POWER");

		if (GC.getBuildingInfo(eBuilding).isDirtyPower() && (GC.getDefineINT("DIRTY_POWER_HEALTH_CHANGE") != 0))
		{
			szTempBuffer.Format(L" (+%d%c)", abs(GC.getDefineINT("DIRTY_POWER_HEALTH_CHANGE")), ((GC.getDefineINT("DIRTY_POWER_HEALTH_CHANGE") > 0) ? gDLL->getSymbolID(HEALTHY_CHAR): gDLL->getSymbolID(UNHEALTHY_CHAR)));
			szBuffer += szTempBuffer;
		}
	}

	if (GC.getBuildingInfo(eBuilding).isAreaCleanPower())
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_PROVIDES_AREA_CLEAN_POWER");
	}

	if (GC.getBuildingInfo(eBuilding).isAreaBorderObstacle())
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_BORDER_OBSTACLE");
	}

	if (GC.getBuildingInfo(eBuilding).isDiploVote())
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_DIPLO_VOTE");
	}

	if (GC.getBuildingInfo(eBuilding).isForceTeamVoteEligible())
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_ELECTION_ELIGIBILITY");
	}

	if (GC.getBuildingInfo(eBuilding).isCapital())
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_CAPITAL");
	}

	if (GC.getBuildingInfo(eBuilding).isGovernmentCenter())
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_REDUCES_MAINTENANCE");
	}

	if (GC.getBuildingInfo(eBuilding).isGoldenAge())
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_GOLDEN_AGE");
	}

	if (GC.getBuildingInfo(eBuilding).isMapCentering())
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_CENTERS_MAP");
	}

	if (GC.getBuildingInfo(eBuilding).isNoUnhappiness())
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_NO_UNHAPPY");
	}

	if (GC.getBuildingInfo(eBuilding).isNoUnhealthyPopulation())
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_NO_UNHEALTHY_POP");
	}

	if (GC.getBuildingInfo(eBuilding).isBuildingOnlyHealthy())
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_NO_UNHEALTHY_BUILDINGS");
	}

	if (GC.getBuildingInfo(eBuilding).getGreatPeopleRateModifier() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_BIRTH_RATE_MOD", GC.getBuildingInfo(eBuilding).getGreatPeopleRateModifier());
	}

	if (GC.getBuildingInfo(eBuilding).getGreatGeneralRateModifier() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_GENERAL_RATE_MOD", GC.getBuildingInfo(eBuilding).getGreatGeneralRateModifier());
	}

	if (GC.getBuildingInfo(eBuilding).getDomesticGreatGeneralRateModifier() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_DOMESTIC_GREAT_GENERAL_MODIFIER", GC.getBuildingInfo(eBuilding).getDomesticGreatGeneralRateModifier());
	}

	if (GC.getBuildingInfo(eBuilding).getGlobalGreatPeopleRateModifier() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_BIRTH_RATE_MOD_ALL_CITIES", GC.getBuildingInfo(eBuilding).getGlobalGreatPeopleRateModifier());
	}

	if (GC.getBuildingInfo(eBuilding).getAnarchyModifier() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_ANARCHY_MOD", GC.getBuildingInfo(eBuilding).getAnarchyModifier());
	}

	if (GC.getBuildingInfo(eBuilding).getGlobalHurryModifier() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_HURRY_MOD", GC.getBuildingInfo(eBuilding).getGlobalHurryModifier());
	}

	if (GC.getBuildingInfo(eBuilding).getFreeExperience() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_FREE_XP_UNITS", GC.getBuildingInfo(eBuilding).getFreeExperience());
	}

	if (GC.getBuildingInfo(eBuilding).getGlobalFreeExperience() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_FREE_XP_ALL_CITIES", GC.getBuildingInfo(eBuilding).getGlobalFreeExperience());
	}

	if (GC.getBuildingInfo(eBuilding).getFoodKept() > 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_STORES_FOOD", GC.getBuildingInfo(eBuilding).getFoodKept());
	}

	if (GC.getBuildingInfo(eBuilding).getAirlift() > 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_AIRLIFT", GC.getBuildingInfo(eBuilding).getAirlift());
	}

	if (GC.getBuildingInfo(eBuilding).getAirModifier() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_AIR_DAMAGE_MOD", GC.getBuildingInfo(eBuilding).getAirModifier());
	}

	if (GC.getBuildingInfo(eBuilding).getNukeModifier() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_NUKE_DAMAGE_MOD", GC.getBuildingInfo(eBuilding).getNukeModifier());
	}

	if (GC.getBuildingInfo(eBuilding).getNukeExplosionRand() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_NUKE_EXPLOSION_CHANCE");
	}

	if (GC.getBuildingInfo(eBuilding).getFreeSpecialist() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_FREE_SPECIALISTS", GC.getBuildingInfo(eBuilding).getFreeSpecialist());
	}

	if (GC.getBuildingInfo(eBuilding).getAreaFreeSpecialist() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_FREE_SPECIALISTS_CONT", GC.getBuildingInfo(eBuilding).getAreaFreeSpecialist());
	}

	if (GC.getBuildingInfo(eBuilding).getGlobalFreeSpecialist() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_FREE_SPECIALISTS_ALL_CITIES", GC.getBuildingInfo(eBuilding).getGlobalFreeSpecialist());
	}

	if (GC.getBuildingInfo(eBuilding).getMaintenanceModifier() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_MAINT_MOD", GC.getBuildingInfo(eBuilding).getMaintenanceModifier());
	}

	if (GC.getBuildingInfo(eBuilding).getHurryAngerModifier() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_HURRY_ANGER_MOD", GC.getBuildingInfo(eBuilding).getHurryAngerModifier());
	}

	if (GC.getBuildingInfo(eBuilding).getWarWearinessModifier() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_WAR_WEAR_MOD", GC.getBuildingInfo(eBuilding).getWarWearinessModifier());
	}

	if (GC.getBuildingInfo(eBuilding).getGlobalWarWearinessModifier() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_WAR_WEAR_MOD_ALL_CITIES", GC.getBuildingInfo(eBuilding).getGlobalWarWearinessModifier());
	}

	if (GC.getBuildingInfo(eBuilding).getHealRateChange() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_HEAL_MOD", GC.getBuildingInfo(eBuilding).getHealRateChange());
	}

	if (GC.getBuildingInfo(eBuilding).getAreaHealth() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_HEALTH_CHANGE_CONT", abs(GC.getBuildingInfo(eBuilding).getAreaHealth()), ((GC.getBuildingInfo(eBuilding).getAreaHealth() > 0) ? gDLL->getSymbolID(HEALTHY_CHAR): gDLL->getSymbolID(UNHEALTHY_CHAR)));
	}

	if (GC.getBuildingInfo(eBuilding).getGlobalHealth() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_HEALTH_CHANGE_ALL_CITIES", abs(GC.getBuildingInfo(eBuilding).getGlobalHealth()), ((GC.getBuildingInfo(eBuilding).getGlobalHealth() > 0) ? gDLL->getSymbolID(HEALTHY_CHAR): gDLL->getSymbolID(UNHEALTHY_CHAR)));
	}

	if (GC.getBuildingInfo(eBuilding).getAreaHappiness() > 0)
	{
//			szTempBuffer.Format(L"\n%c+%d%c in All Cities on this Continent", gDLL->getSymbolID(BULLET_CHAR), GC.getBuildingInfo(eBuilding).getAreaHappiness(), ((GC.getBuildingInfo(eBuilding).getAreaHappiness() > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)));
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_HAPPY_CHANGE_CONT", GC.getBuildingInfo(eBuilding).getAreaHappiness(), ((GC.getBuildingInfo(eBuilding).getAreaHappiness() > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)));
	}

	if (GC.getBuildingInfo(eBuilding).getGlobalHappiness() > 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_HAPPY_CHANGE_ALL_CITIES", GC.getBuildingInfo(eBuilding).getGlobalHappiness(), ((GC.getBuildingInfo(eBuilding).getGlobalHappiness() > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)));
	}

	if (GC.getBuildingInfo(eBuilding).getStateReligionHappiness() > 0)
	{
		if (GC.getBuildingInfo(eBuilding).getReligionType() != NO_RELIGION)
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_RELIGION_HAPPINESS", GC.getBuildingInfo(eBuilding).getStateReligionHappiness(), ((GC.getBuildingInfo(eBuilding).getStateReligionHappiness() > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)), GC.getReligionInfo((ReligionTypes)(GC.getBuildingInfo(eBuilding).getReligionType())).getChar());
		}
	}

	if (GC.getBuildingInfo(eBuilding).getWorkerSpeedModifier() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_WORKER_MOD", GC.getBuildingInfo(eBuilding).getWorkerSpeedModifier());
	}

	if (GC.getBuildingInfo(eBuilding).getMilitaryProductionModifier() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_MILITARY_MOD", GC.getBuildingInfo(eBuilding).getMilitaryProductionModifier());
	}

	if (GC.getBuildingInfo(eBuilding).getSpaceProductionModifier() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_SPACESHIP_MOD", GC.getBuildingInfo(eBuilding).getSpaceProductionModifier());
	}

	if (GC.getBuildingInfo(eBuilding).getGlobalSpaceProductionModifier() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_SPACESHIP_MOD_ALL_CITIES", GC.getBuildingInfo(eBuilding).getGlobalSpaceProductionModifier());
	}

	if (GC.getBuildingInfo(eBuilding).getTradeRoutes() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_TRADE_ROUTES", GC.getBuildingInfo(eBuilding).getTradeRoutes());
	}

	if (GC.getBuildingInfo(eBuilding).getCoastalTradeRoutes() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_COASTAL_TRADE_ROUTES", GC.getBuildingInfo(eBuilding).getCoastalTradeRoutes());
	}

	if (GC.getBuildingInfo(eBuilding).getGlobalTradeRoutes() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_TRADE_ROUTES_ALL_CITIES", GC.getBuildingInfo(eBuilding).getGlobalTradeRoutes());
	}

	if (GC.getBuildingInfo(eBuilding).getTradeRouteModifier() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_TRADE_ROUTE_MOD", GC.getBuildingInfo(eBuilding).getTradeRouteModifier());
	}

	if (GC.getBuildingInfo(eBuilding).getGlobalPopulationChange() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_GLOBAL_POP", GC.getBuildingInfo(eBuilding).getGlobalPopulationChange());
	}

	if (GC.getBuildingInfo(eBuilding).getFreeTechs() != 0)
	{
		if (GC.getBuildingInfo(eBuilding).getFreeTechs() == 1)
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_FREE_TECH");
		}
		else
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_FREE_TECHS", GC.getBuildingInfo(eBuilding).getFreeTechs());
		}
	}

	if (GC.getBuildingInfo(eBuilding).getDefenseModifier() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_DEFENSE_MOD", GC.getBuildingInfo(eBuilding).getDefenseModifier());
	}

	if (GC.getBuildingInfo(eBuilding).getBombardDefenseModifier() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_BOMBARD_DEFENSE_MOD", -GC.getBuildingInfo(eBuilding).getBombardDefenseModifier());
	}

	if (GC.getBuildingInfo(eBuilding).getAllCityDefenseModifier() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_DEFENSE_MOD_ALL_CITIES", GC.getBuildingInfo(eBuilding).getAllCityDefenseModifier());
	}

	setYieldChangeHelp(szBuffer, gDLL->getText("TXT_KEY_BUILDING_WATER_PLOTS").c_str(), L": ", L"", GC.getBuildingInfo(eBuilding).getSeaPlotYieldChangeArray());

	setYieldChangeHelp(szBuffer, gDLL->getText("TXT_KEY_BUILDING_WATER_PLOTS_ALL_CITIES").c_str(), L": ", L"", GC.getBuildingInfo(eBuilding).getGlobalSeaPlotYieldChangeArray());

	setYieldChangeHelp(szBuffer, L"", L"", gDLL->getText("TXT_KEY_BUILDING_WITH_POWER").c_str(), GC.getBuildingInfo(eBuilding).getPowerYieldModifierArray(), true);

	setYieldChangeHelp(szBuffer, L"", L"", gDLL->getText("TXT_KEY_BUILDING_ALL_CITIES_THIS_CONTINENT").c_str(), GC.getBuildingInfo(eBuilding).getAreaYieldModifierArray(), true);

	setYieldChangeHelp(szBuffer, L"", L"", gDLL->getText("TXT_KEY_BUILDING_ALL_CITIES").c_str(), GC.getBuildingInfo(eBuilding).getGlobalYieldModifierArray(), true);

	setCommerceChangeHelp(szBuffer, L"", L"", gDLL->getText("TXT_KEY_BUILDING_ALL_CITIES").c_str(), GC.getBuildingInfo(eBuilding).getGlobalCommerceModifierArray(), true);

	setCommerceChangeHelp(szBuffer, L"", L"", gDLL->getText("TXT_KEY_BUILDING_PER_SPECIALIST_ALL_CITIES").c_str(), GC.getBuildingInfo(eBuilding).getSpecialistExtraCommerceArray());

	if (ePlayer != NO_PLAYER && GET_PLAYER(ePlayer).getStateReligion() != NO_RELIGION)
	{
		szTempBuffer = gDLL->getText("TXT_KEY_BUILDING_FROM_ALL_REL_BUILDINGS", GC.getReligionInfo(GET_PLAYER(ePlayer).getStateReligion()).getChar());
	}
	else
	{
		szTempBuffer = gDLL->getText("TXT_KEY_BUILDING_STATE_REL_BUILDINGS");
	}
	setCommerceChangeHelp(szBuffer, L"", L"", szTempBuffer, GC.getBuildingInfo(eBuilding).getStateReligionCommerceArray());

	for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		if (GC.getBuildingInfo(eBuilding).getCommerceHappiness(iI) != 0)
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_PER_LEVEL", ((GC.getBuildingInfo(eBuilding).getCommerceHappiness(iI) > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)), (100 / GC.getBuildingInfo(eBuilding).getCommerceHappiness(iI)), GC.getCommerceInfo((CommerceTypes)iI).getChar());
		}

		if (GC.getBuildingInfo(eBuilding).isCommerceFlexible(iI) != 0)
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_ADJUST_COMM_RATE", GC.getCommerceInfo((CommerceTypes) iI).getChar());
		}
	}

	for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
	{
		szFirstBuffer = gDLL->getText("TXT_KEY_BUILDING_FROM_IN_ALL_CITIES", GC.getSpecialistInfo((SpecialistTypes) iI).getTextKeyWide());
		setYieldChangeHelp(szBuffer, L"", L"", szFirstBuffer, GC.getBuildingInfo(eBuilding).getSpecialistYieldChangeArray(iI));
	}

	for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
	{
		szFirstBuffer = gDLL->getText("TXT_KEY_BUILDING_WITH_BONUS", GC.getBonusInfo((BonusTypes) iI).getTextKeyWide());
		setYieldChangeHelp(szBuffer, L"", L"", szFirstBuffer, GC.getBuildingInfo(eBuilding).getBonusYieldModifierArray(iI), true);
	}

	for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
	{
		if (GC.getBuildingInfo(eBuilding).getReligionChange(iI) > 0)
		{
			szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_BUILDING_SPREADS_RELIGION", GC.getReligionInfo((ReligionTypes) iI).getChar()).c_str());
			szBuffer += szTempBuffer;
		}
	}

	for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
	{
		if (GC.getBuildingInfo(eBuilding).getSpecialistCount(iI) > 0)
		{
			if (GC.getBuildingInfo(eBuilding).getSpecialistCount(iI) == 1)
			{
				szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_TURN_CITIZEN_INTO", GC.getSpecialistInfo((SpecialistTypes) iI).getTextKeyWide());
			}
			else
			{
				szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_TURN_CITIZENS_INTO", GC.getBuildingInfo(eBuilding).getSpecialistCount(iI), GC.getSpecialistInfo((SpecialistTypes) iI).getTextKeyWide());
			}
		}

		if (GC.getBuildingInfo(eBuilding).getFreeSpecialistCount(iI) > 0)
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_FREE_SPECIALIST", GC.getBuildingInfo(eBuilding).getFreeSpecialistCount(iI), GC.getSpecialistInfo((SpecialistTypes) iI).getTextKeyWide());
		}
	}

	iLast = 0;

	for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
	{
		if (GC.getBuildingInfo(eBuilding).getBonusHealthChanges(iI) != 0)
		{
			szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_BUILDING_HEALTH_HAPPINESS_CHANGE", GC.getBuildingInfo(eBuilding).getBonusHealthChanges(iI), ((GC.getBuildingInfo(eBuilding).getBonusHealthChanges(iI) > 0) ? gDLL->getSymbolID(HEALTHY_CHAR): gDLL->getSymbolID(UNHEALTHY_CHAR))).c_str());
			szTempBuffer.Format(L"<link=literal>%s</link>", GC.getBonusInfo((BonusTypes)iI).getDescription());
			setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", (GC.getBuildingInfo(eBuilding).getBonusHealthChanges(iI) != iLast));
			iLast = GC.getBuildingInfo(eBuilding).getBonusHealthChanges(iI);
		}
	}

	iLast = 0;

	for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
	{
		if (GC.getBuildingInfo(eBuilding).getBonusHappinessChanges(iI) != 0)
		{
			szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_BUILDING_HEALTH_HAPPINESS_CHANGE", GC.getBuildingInfo(eBuilding).getBonusHappinessChanges(iI), ((GC.getBuildingInfo(eBuilding).getBonusHappinessChanges(iI) > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR))).c_str());
			szTempBuffer.Format(L"<link=literal>%s</link>", GC.getBonusInfo((BonusTypes)iI).getDescription());
			setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", (GC.getBuildingInfo(eBuilding).getBonusHappinessChanges(iI) != iLast));
			iLast = GC.getBuildingInfo(eBuilding).getBonusHappinessChanges(iI);
		}
	}

	for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
	{
		if (GC.getBuildingInfo(eBuilding).getUnitCombatFreeExperience(iI) != 0)
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_FREE_XP", GC.getUnitCombatInfo((UnitCombatTypes)iI).getTextKeyWide(), GC.getBuildingInfo(eBuilding).getUnitCombatFreeExperience(iI));
		}
	}

	for (iI = 0; iI < NUM_DOMAIN_TYPES; iI++)
	{
		if (GC.getBuildingInfo(eBuilding).getDomainFreeExperience(iI) != 0)
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_FREE_XP", GC.getDomainInfo((DomainTypes)iI).getTextKeyWide(), GC.getBuildingInfo(eBuilding).getDomainFreeExperience(iI));
		}
	}

	for (iI = 0; iI < NUM_DOMAIN_TYPES; iI++)
	{
		if (GC.getBuildingInfo(eBuilding).getDomainProductionModifier(iI) != 0)
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_BUILDS_FASTER_DOMAIN", GC.getDomainInfo((DomainTypes)iI).getTextKeyWide(), GC.getBuildingInfo(eBuilding).getDomainProductionModifier(iI));
		}
	}

	bFirst = true;

	for (iI = 0; iI < GC.getNumUnitInfos(); iI++)
	{
		if (GC.getUnitInfo((UnitTypes)iI).getPrereqBuilding() == eBuilding)
		{
			szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_BUILDING_REQUIRED_TO_TRAIN").c_str());
			szTempBuffer.Format( SETCOLR L"<link=literal>%s</link>" ENDCOLR , TEXT_COLOR("COLOR_UNIT_TEXT"), GC.getUnitInfo((UnitTypes)iI).getDescription());
			setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", bFirst);
			bFirst = false;
		}
	}

	iLast = 0;

	for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
	{
		if (GC.getBuildingInfo(eBuilding).getBuildingHappinessChanges(iI) != 0)
		{
			szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_BUILDING_HAPPINESS_CHANGE", GC.getBuildingInfo(eBuilding).getBuildingHappinessChanges(iI),
				((GC.getBuildingInfo(eBuilding).getBuildingHappinessChanges(iI) > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR))).c_str());
			CvWString szBuilding;
			szBuilding.Format(L"<link=literal>%s</link>", GC.getBuildingInfo((BuildingTypes)iI).getDescription());
			setListHelp(szBuffer, szTempBuffer, szBuilding, L", ", (GC.getBuildingInfo(eBuilding).getBuildingHappinessChanges(iI) != iLast));
			iLast = GC.getBuildingInfo(eBuilding).getBuildingHappinessChanges(iI);
		}
	}

	if (GC.getBuildingInfo(eBuilding).getPowerBonus() != NO_BONUS)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_PROVIDES_POWER_WITH", GC.getBonusInfo((BonusTypes)GC.getBuildingInfo(eBuilding).getPowerBonus()).getTextKeyWide());

		if (GC.getBuildingInfo(eBuilding).isDirtyPower() && (GC.getDefineINT("DIRTY_POWER_HEALTH_CHANGE") != 0))
		{
			szTempBuffer.Format(L" (+%d%c)", abs(GC.getDefineINT("DIRTY_POWER_HEALTH_CHANGE")), ((GC.getDefineINT("DIRTY_POWER_HEALTH_CHANGE") > 0) ? gDLL->getSymbolID(HEALTHY_CHAR): gDLL->getSymbolID(UNHEALTHY_CHAR)));
			szBuffer += szTempBuffer;
		}
	}

	bFirst = true;

	for (iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
	{
		if (ePlayer != NO_PLAYER)
		{
			eLoopBuilding = ((BuildingTypes)(GC.getCivilizationInfo(GET_PLAYER(ePlayer).getCivilizationType()).getCivilizationBuildings(iI)));
		}
		else
		{
			eLoopBuilding = (BuildingTypes)GC.getBuildingClassInfo((BuildingClassTypes)iI).getDefaultBuildingIndex();
		}

		if (eLoopBuilding != NO_BUILDING)
		{
			if (GC.getBuildingInfo(eLoopBuilding).isBuildingClassNeededInCity(GC.getBuildingInfo(eBuilding).getBuildingClassType()))
			{
				if ((pCity == NULL) || pCity->canConstruct(eLoopBuilding, false, true))
				{
					szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_BUILDING_REQUIRED_TO_BUILD").c_str());
					szTempBuffer.Format(SETCOLR L"<link=literal>%s</link>" ENDCOLR, TEXT_COLOR("COLOR_BUILDING_TEXT"), GC.getBuildingInfo(eLoopBuilding).getDescription());
					setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", bFirst);
					bFirst = false;
				}
			}
		}
	}

	if (bCivilopediaText)
	{
		if (GC.getBuildingInfo(eBuilding).getGreatPeopleUnitClass() != NO_UNITCLASS)
		{
			if (ePlayer != NO_PLAYER)
			{
				eGreatPeopleUnit = ((UnitTypes)(GC.getCivilizationInfo(GET_PLAYER(ePlayer).getCivilizationType()).getCivilizationUnits(GC.getBuildingInfo(eBuilding).getGreatPeopleUnitClass())));
			}
			else
			{
				eGreatPeopleUnit = (UnitTypes)GC.getUnitClassInfo((UnitClassTypes)GC.getBuildingInfo(eBuilding).getGreatPeopleUnitClass()).getDefaultUnitIndex();
			}

			if (eGreatPeopleUnit!= NO_UNIT)
			{
				szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_LIKELY_TO_GENERATE", GC.getUnitInfo(eGreatPeopleUnit).getTextKeyWide());
			}
		}

		if (GC.getBuildingInfo(eBuilding).getFreeStartEra() != NO_ERA)
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_FREE_START_ERA", GC.getEraInfo((EraTypes)GC.getBuildingInfo(eBuilding).getFreeStartEra()).getTextKeyWide());
		}
	}

	if (!isEmpty(GC.getBuildingInfo(eBuilding).getHelp()))
	{
		szBuffer += NEWLINE;
		szBuffer += GC.getBuildingInfo(eBuilding).getHelp();
	}

	if ((pCity == NULL) || !(pCity->canConstruct(eBuilding)))
	{
		if (GC.getBuildingInfo(eBuilding).getHolyCity() != NO_RELIGION)
		{
			if ((pCity == NULL) || !(pCity->isHolyCity((ReligionTypes)(GC.getBuildingInfo(eBuilding).getHolyCity()))))
			{
				szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_REQUIRES_HOLY_CITY", GC.getReligionInfo((ReligionTypes) GC.getBuildingInfo(eBuilding).getHolyCity()).getChar());
			}
		}

		bFirst = true;

		if (GC.getBuildingInfo(eBuilding).getSpecialBuildingType() != NO_SPECIALBUILDING)
		{
			if ((pCity == NULL) || !(GC.getGameINLINE().isSpecialBuildingValid((SpecialBuildingTypes)(GC.getBuildingInfo(eBuilding).getSpecialBuildingType()))))
			{
				for (iI = 0; iI < GC.getNumProjectInfos(); iI++)
				{
					if (GC.getProjectInfo((ProjectTypes)iI).getEveryoneSpecialBuilding() == GC.getBuildingInfo(eBuilding).getSpecialBuildingType())
					{
						szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_REQUIRES").c_str());
						CvWString szProject;
						szProject.Format(L"<link=literal>%s</link>", GC.getProjectInfo((ProjectTypes)iI).getDescription());
						setListHelp(szBuffer, szTempBuffer, szProject, gDLL->getText("TXT_KEY_OR").c_str(), bFirst);
						bFirst = false;
					}
				}
			}
		}

		if (!bFirst)
		{
			szBuffer += ENDCOLR;
		}

		for (iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
		{
			if (ePlayer == NO_PLAYER && GC.getBuildingInfo(eBuilding).getPrereqNumOfBuildingClass((BuildingClassTypes)iI) > 0)
			{
				eLoopBuilding = (BuildingTypes)GC.getBuildingClassInfo((BuildingClassTypes)iI).getDefaultBuildingIndex();
				szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_BUILDING_REQUIRES_NUM_SPECIAL_BUILDINGS_NO_CITY", GC.getBuildingInfo(eLoopBuilding).getTextKeyWide(), GC.getBuildingInfo(eBuilding).getPrereqNumOfBuildingClass((BuildingClassTypes)iI)).c_str());
			}
			else if (ePlayer != NO_PLAYER && GET_PLAYER(ePlayer).getBuildingClassPrereqBuilding(eBuilding, ((BuildingClassTypes)iI)) > 0)
			{
				if ((pCity == NULL) || (GET_PLAYER(ePlayer).getBuildingClassCount((BuildingClassTypes)iI) < GET_PLAYER(ePlayer).getBuildingClassPrereqBuilding(eBuilding, ((BuildingClassTypes)iI))))
				{
					eLoopBuilding = ((BuildingTypes)(GC.getCivilizationInfo(GET_PLAYER(ePlayer).getCivilizationType()).getCivilizationBuildings(iI)));

					if (eLoopBuilding != NO_BUILDING)
					{
						if (pCity != NULL)
						{
							szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_BUILDING_REQUIRES_NUM_SPECIAL_BUILDINGS", GC.getBuildingInfo(eLoopBuilding).getTextKeyWide(), GET_PLAYER(ePlayer).getBuildingClassCount((BuildingClassTypes)iI), GET_PLAYER(ePlayer).getBuildingClassPrereqBuilding(eBuilding, ((BuildingClassTypes)iI))).c_str());
						}
						else
						{
							szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_BUILDING_REQUIRES_NUM_SPECIAL_BUILDINGS_NO_CITY", GC.getBuildingInfo(eLoopBuilding).getTextKeyWide(), GET_PLAYER(ePlayer).getBuildingClassPrereqBuilding(eBuilding, ((BuildingClassTypes)iI))).c_str());
						}
						szBuffer += szTempBuffer;
					}
				}
			}
			else if (GC.getBuildingInfo(eBuilding).isBuildingClassNeededInCity(iI))
			{
				if (NO_PLAYER != ePlayer)
				{
					eLoopBuilding = ((BuildingTypes)(GC.getCivilizationInfo(GET_PLAYER(ePlayer).getCivilizationType()).getCivilizationBuildings(iI)));
				}
				else
				{
					eLoopBuilding = (BuildingTypes)GC.getBuildingClassInfo((BuildingClassTypes)iI).getDefaultBuildingIndex();
				}

				if (eLoopBuilding != NO_BUILDING)
				{
					if ((pCity == NULL) || !(pCity->hasBuilding(eLoopBuilding)))
					{
						szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_REQUIRES_STRING", GC.getBuildingInfo(eLoopBuilding).getTextKeyWide());
					}
				}
			}
		}

		if (GC.getBuildingInfo(eBuilding).getNumCitiesPrereq() > 0)
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_REQUIRES_NUM_CITIES", GC.getBuildingInfo(eBuilding).getNumCitiesPrereq());
		}

		if (GC.getBuildingInfo(eBuilding).getUnitLevelPrereq() > 0)
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_REQUIRES_UNIT_LEVEL", GC.getBuildingInfo(eBuilding).getUnitLevelPrereq());
		}

		if (GC.getBuildingInfo(eBuilding).getMinLatitude() > 0)
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_MIN_LATITUDE", GC.getBuildingInfo(eBuilding).getMinLatitude());
		}

		if (GC.getBuildingInfo(eBuilding).getMaxLatitude() < 90)
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_MAX_LATITUDE", GC.getBuildingInfo(eBuilding).getMaxLatitude());
		}

		if (bCivilopediaText)
		{
			if (GC.getBuildingInfo(eBuilding).getVictoryPrereq() != NO_VICTORY)
			{
				szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_REQUIRES_VICTORY", GC.getVictoryInfo((VictoryTypes)(GC.getBuildingInfo(eBuilding).getVictoryPrereq())).getTextKeyWide());
			}

			if (GC.getBuildingInfo(eBuilding).getMaxStartEra() != NO_ERA)
			{
				szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_MAX_START_ERA", GC.getEraInfo((EraTypes)GC.getBuildingInfo(eBuilding).getMaxStartEra()).getTextKeyWide());
			}

			if (GC.getBuildingInfo(eBuilding).getNumTeamsPrereq() > 0)
			{
				szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_REQUIRES_NUM_TEAMS", GC.getBuildingInfo(eBuilding).getNumTeamsPrereq());
			}
		}
		else
		{
			if (!bTechChooserText)
			{
				if (GC.getBuildingInfo(eBuilding).getPrereqAndTech() != NO_TECH)
				{
					if (ePlayer == NO_PLAYER || !(GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isHasTech((TechTypes)(GC.getBuildingInfo(eBuilding).getPrereqAndTech()))))
					{
						szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_REQUIRES_STRING", GC.getTechInfo((TechTypes)(GC.getBuildingInfo(eBuilding).getPrereqAndTech())).getTextKeyWide());
					}
				}
			}

			bFirst = true;

			for (iI = 0; iI < GC.getDefineINT("NUM_BUILDING_AND_TECH_PREREQS"); iI++)
			{
				if (GC.getBuildingInfo(eBuilding).getPrereqAndTechs(iI) != NO_TECH)
				{
					if (bTechChooserText || ePlayer == NO_PLAYER || !(GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isHasTech((TechTypes)(GC.getBuildingInfo(eBuilding).getPrereqAndTechs(iI)))))
					{
						szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_REQUIRES").c_str());
						setListHelp(szBuffer, szTempBuffer, GC.getTechInfo(((TechTypes)(GC.getBuildingInfo(eBuilding).getPrereqAndTechs(iI)))).getDescription(), gDLL->getText("TXT_KEY_AND").c_str(), bFirst);
						bFirst = false;
					}
				}
			}

			if (!bFirst)
			{
				szBuffer += ENDCOLR;
			}

			if (GC.getBuildingInfo(eBuilding).getPrereqAndBonus() != NO_BONUS)
			{
				if ((pCity == NULL) || !(pCity->hasBonus((BonusTypes)GC.getBuildingInfo(eBuilding).getPrereqAndBonus())))
				{
					szBuffer += NEWLINE + gDLL->getText("TXT_KEY_UNIT_REQUIRES_STRING", GC.getBonusInfo((BonusTypes)GC.getBuildingInfo(eBuilding).getPrereqAndBonus()).getTextKeyWide());
				}
			}

			bFirst = true;

			for (iI = 0; iI < GC.getNUM_BUILDING_PREREQ_OR_BONUSES(); iI++)
			{
				if (GC.getBuildingInfo(eBuilding).getPrereqOrBonuses(iI) != NO_BONUS)
				{
					if ((pCity == NULL) || !(pCity->hasBonus((BonusTypes)GC.getBuildingInfo(eBuilding).getPrereqOrBonuses(iI))))
					{
						szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_REQUIRES").c_str());
						setListHelp(szBuffer, szTempBuffer, GC.getBonusInfo((BonusTypes)GC.getBuildingInfo(eBuilding).getPrereqOrBonuses(iI)).getDescription(), gDLL->getText("TXT_KEY_OR").c_str(), bFirst);
						bFirst = false;
					}
				}
			}

			if (!bFirst)
			{
				szBuffer += ENDCOLR;
			}
		}
	}

	if (pCity != NULL)
	{
		if (!(GC.getBuildingClassInfo((BuildingClassTypes)(GC.getBuildingInfo(eBuilding).getBuildingClassType())).isNoLimit()))
		{
			if (isWorldWonderClass((BuildingClassTypes)(GC.getBuildingInfo(eBuilding).getBuildingClassType())))
			{
				if (pCity->isWorldWondersMaxed())
				{
					szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_WORLD_WONDERS_PER_CITY", GC.getDefineINT("MAX_WORLD_WONDERS_PER_CITY"));
				}
			}
			else if (isTeamWonderClass((BuildingClassTypes)(GC.getBuildingInfo(eBuilding).getBuildingClassType())))
			{
				if (pCity->isTeamWondersMaxed())
				{
					szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_TEAM_WONDERS_PER_CITY", GC.getDefineINT("MAX_TEAM_WONDERS_PER_CITY"));
				}
			}
			else if (isNationalWonderClass((BuildingClassTypes)(GC.getBuildingInfo(eBuilding).getBuildingClassType())))
			{
				if (pCity->isNationalWondersMaxed())
				{
					szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_NATIONAL_WONDERS_PER_CITY", GC.getDefineINT("MAX_NATIONAL_WONDERS_PER_CITY"));
				}
			}
			else
			{
				if (pCity->isBuildingsMaxed())
				{
					szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_NUM_PER_CITY", GC.getDefineINT("MAX_BUILDINGS_PER_CITY"));
				}
			}
		}
	}

	if ((pCity == NULL) || !(pCity->isHasRealBuilding(eBuilding)))
	{
		if (!bCivilopediaText)
		{
			if (pCity == NULL)
			{
				if (GC.getBuildingInfo(eBuilding).getProductionCost() > 0)
				{
					szTempBuffer.Format(L"\n%d%c", (ePlayer != NO_PLAYER ? GET_PLAYER(ePlayer).getProductionNeeded(eBuilding) : GC.getBuildingInfo(eBuilding).getProductionCost()), GC.getYieldInfo(YIELD_PRODUCTION).getChar());
					szBuffer += szTempBuffer;
				}
			}
			else
			{
				szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_NUM_TURNS", pCity->getProductionTurnsLeft(eBuilding, ((gDLL->ctrlKey() || !(gDLL->shiftKey())) ? 0 : pCity->getOrderQueueLength())));

				iProduction = pCity->getBuildingProduction(eBuilding);

				if (iProduction > 0)
				{
					szTempBuffer.Format(L" - %d/%d%c", iProduction, GET_PLAYER(ePlayer).getProductionNeeded(eBuilding), GC.getYieldInfo(YIELD_PRODUCTION).getChar());
					szBuffer += szTempBuffer;
				}
				else
				{
					szTempBuffer.Format(L" - %d%c", GET_PLAYER(ePlayer).getProductionNeeded(eBuilding), GC.getYieldInfo(YIELD_PRODUCTION).getChar());
					szBuffer += szTempBuffer;
				}
			}
		}

		for (int iI = 0; iI < GC.getNumBonusInfos(); iI++)
		{
			if (GC.getBuildingInfo(eBuilding).getBonusProductionModifier(iI) != 0)
			{
				if (pCity != NULL)
				{
					if (pCity->hasBonus((BonusTypes)iI))
					{
						szBuffer += gDLL->getText("TXT_KEY_COLOR_POSITIVE");
					}
					else
					{
						szBuffer += gDLL->getText("TXT_KEY_COLOR_NEGATIVE");
					}
				}
				if (!bCivilopediaText)
				{
					szBuffer += L" (";
				}
				else
				{
					szTempBuffer.Format(L"\n%c", gDLL->getSymbolID(BULLET_CHAR), szTempBuffer);
					szBuffer += szTempBuffer;
				}
				if (GC.getBuildingInfo(eBuilding).getBonusProductionModifier(iI) == 100)
				{
	//					szTempBuffer.Format(L"Double Speed with %c",);
					szBuffer += gDLL->getText("TXT_KEY_BUILDING_DOUBLE_SPEED_WITH", GC.getBonusInfo((BonusTypes)iI).getTextKeyWide());
				}
				else
				{
	//					szTempBuffer.Format(L"Builds %d%% Faster with %c");
					szBuffer += gDLL->getText("TXT_KEY_BUILDING_BUILDS_FASTER_WITH", GC.getBuildingInfo(eBuilding).getBonusProductionModifier(iI), GC.getBonusInfo((BonusTypes)iI).getTextKeyWide());
				}
	//				szBuffer += szTempBuffer;
				if (!bCivilopediaText)
				{
					szBuffer += L")";
				}
				if (pCity != NULL)
				{
					szBuffer += gDLL->getText("TXT_KEY_COLOR_REVERT");
				}
			}
		}

		if (GC.getBuildingInfo(eBuilding).getObsoleteTech() != NO_TECH)
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_OBSOLETE_WITH", GC.getTechInfo((TechTypes) GC.getBuildingInfo(eBuilding).getObsoleteTech()).getTextKeyWide());

			if (GC.getBuildingInfo(eBuilding).getDefenseModifier() != 0 || GC.getBuildingInfo(eBuilding).getBombardDefenseModifier() != 0)
			{
				szBuffer += gDLL->getText("TXT_KEY_BUILDING_OBSOLETE_EXCEPT");
			}
		}

		if (GC.getBuildingInfo(eBuilding).getSpecialBuildingType() != NO_SPECIALBUILDING)
		{
			if (GC.getSpecialBuildingInfo((SpecialBuildingTypes) GC.getBuildingInfo(eBuilding).getSpecialBuildingType()).getObsoleteTech() != NO_TECH)
			{
				szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BUILDING_OBSOLETE_WITH", GC.getTechInfo((TechTypes) GC.getSpecialBuildingInfo((SpecialBuildingTypes) GC.getBuildingInfo(eBuilding).getSpecialBuildingType()).getObsoleteTech()).getTextKeyWide());
			}
		}
	}

	if (bStrategyText)
	{
		if (!isEmpty(GC.getBuildingInfo(eBuilding).getStrategy()))
		{
			if ((ePlayer == NO_PLAYER) || GET_PLAYER(ePlayer).isOption(PLAYEROPTION_ADVISOR_HELP))
			{
				szBuffer += SEPARATOR;
				szBuffer += NEWLINE;
				szBuffer += gDLL->getText("TXT_KEY_SIDS_TIPS");
				szBuffer += L"\"";
				szBuffer += GC.getBuildingInfo(eBuilding).getStrategy();
				szBuffer += L"\"";
			}
		}
	}
}


void CvGameTextMgr::setProjectHelp(CvWString &szBuffer, ProjectTypes eProject, bool bCivilopediaText, CvCity* pCity)
{
	CvWString szTempBuffer;
	CvWString szFirstBuffer;
	PlayerTypes ePlayer;
	bool bFirst;
	int iProduction;
	int iI;

	if (NO_PROJECT == eProject)
	{
		return;
	}

	if (pCity != NULL)
	{
		ePlayer = pCity->getOwnerINLINE();
	}
	else
	{
		ePlayer = GC.getGameINLINE().getActivePlayer();
	}

	if (!bCivilopediaText)
	{
		szTempBuffer.Format( SETCOLR L"<link=literal>%s</link>" ENDCOLR , TEXT_COLOR("COLOR_PROJECT_TEXT"), GC.getProjectInfo(eProject).getDescription());
		szBuffer += szTempBuffer;

		if (isWorldProject(eProject))
		{
			if (pCity == NULL)
			{
				szBuffer += NEWLINE + gDLL->getText("TXT_KEY_PROJECT_WORLD_NUM_ALLOWED", GC.getProjectInfo(eProject).getMaxGlobalInstances());
			}
			else
			{
				szBuffer += gDLL->getText("TXT_KEY_PROJECT_WORLD_NUM_LEFT", (GC.getProjectInfo(eProject).getMaxGlobalInstances() - GC.getGameINLINE().getProjectCreatedCount(eProject) - GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getProjectMaking(eProject)));
			}
		}

		if (isTeamProject(eProject))
		{
			if (pCity == NULL)
			{
				szBuffer += NEWLINE + gDLL->getText("TXT_KEY_PROJECT_TEAM_NUM_ALLOWED", GC.getProjectInfo(eProject).getMaxTeamInstances());
			}
			else
			{
				szBuffer += gDLL->getText("TXT_KEY_PROJECT_TEAM_NUM_LEFT", (GC.getProjectInfo(eProject).getMaxTeamInstances() - GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getProjectCount(eProject) - GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getProjectMaking(eProject)));
			}
		}
	}

	if (GC.getProjectInfo(eProject).getNukeInterception() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_PROJECT_CHANCE_INTERCEPT_NUKES", GC.getProjectInfo(eProject).getNukeInterception());
	}

	if (GC.getProjectInfo(eProject).getTechShare() != 0)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_PROJECT_TECH_SHARE", GC.getProjectInfo(eProject).getTechShare());
	}

	if (GC.getProjectInfo(eProject).getEveryoneSpecialUnit() != NO_SPECIALUNIT)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_PROJECT_ENABLES_SPECIAL", GC.getSpecialUnitInfo((SpecialUnitTypes)(GC.getProjectInfo(eProject).getEveryoneSpecialUnit())).getTextKeyWide());
	}

	if (GC.getProjectInfo(eProject).getEveryoneSpecialBuilding() != NO_SPECIALBUILDING)
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_PROJECT_ENABLES_SPECIAL", GC.getSpecialBuildingInfo((SpecialBuildingTypes)(GC.getProjectInfo(eProject).getEveryoneSpecialBuilding())).getTextKeyWide());
	}

	for (iI = 0; iI < GC.getNumVictoryInfos(); iI++)
	{
		if (GC.getProjectInfo(eProject).getVictoryThreshold((VictoryTypes)iI) > 0)
		{
			if ((pCity == NULL) || (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getProjectCount(eProject) < GC.getProjectInfo(eProject).getVictoryThreshold((VictoryTypes)iI)))
			{
				if (pCity != NULL)
				{
					szBuffer += NEWLINE + gDLL->getText("TXT_KEY_PROJECT_REQUIRED_TYPE_VICTORY", GC.getVictoryInfo((VictoryTypes)iI).getTextKeyWide(), GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getProjectCount(eProject), GC.getProjectInfo(eProject).getVictoryThreshold((VictoryTypes)iI));
				}
				else
				{
					szBuffer += NEWLINE + gDLL->getText("TXT_KEY_PROJECT_REQUIRED_STRING", GC.getVictoryInfo((VictoryTypes)iI).getTextKeyWide(), GC.getProjectInfo(eProject).getVictoryThreshold((VictoryTypes)iI));
				}
			}
		}
	}

	bFirst = true;

	for (iI = 0; iI < GC.getNumProjectInfos(); iI++)
	{
		if (GC.getProjectInfo((ProjectTypes)iI).getAnyoneProjectPrereq() == eProject)
		{
			szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_PROJECT_REQUIRED_TO_CREATE_ANYONE").c_str());
			szTempBuffer.Format(SETCOLR L"<link=literal>%s</link>" ENDCOLR, TEXT_COLOR("COLOR_PROJECT_TEXT"), GC.getProjectInfo((ProjectTypes)iI).getDescription());
			setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", bFirst);
			bFirst = false;
		}
	}

	bFirst = true;

	for (iI = 0; iI < GC.getNumProjectInfos(); iI++)
	{
		if (GC.getProjectInfo((ProjectTypes)iI).getProjectsNeeded(eProject) > 0)
		{
			if ((pCity == NULL) || pCity->canCreate(((ProjectTypes)iI), false, true))
			{
				szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_PROJECT_REQUIRED_TO_CREATE").c_str());
				szTempBuffer.Format(SETCOLR L"<link=literal>%s</link>" ENDCOLR, TEXT_COLOR("COLOR_PROJECT_TEXT"), GC.getProjectInfo((ProjectTypes)iI).getDescription());
				setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", bFirst);
				bFirst = false;
			}
		}
	}

	if ((pCity == NULL) || !(pCity->canCreate(eProject)))
	{
		if (pCity != NULL)
		{
			if (GC.getGameINLINE().isNoNukes())
			{
				if (GC.getProjectInfo(eProject).getEveryoneSpecialUnit() != NO_SPECIALUNIT)
				{
					for (iI = 0; iI < GC.getNumUnitInfos(); iI++)
					{
						if (GC.getUnitInfo((UnitTypes)iI).getSpecialUnitType() == GC.getProjectInfo(eProject).getEveryoneSpecialUnit())
						{
							if (GC.getUnitInfo((UnitTypes)iI).getNukeRange() != -1)
							{
								szBuffer += NEWLINE + gDLL->getText("TXT_KEY_PROJECT_NO_NUKES");
							}
						}
					}
				}
			}
		}

		if (GC.getProjectInfo(eProject).getAnyoneProjectPrereq() != NO_PROJECT)
		{
			if ((pCity == NULL) || (GC.getGameINLINE().getProjectCreatedCount((ProjectTypes)(GC.getProjectInfo(eProject).getAnyoneProjectPrereq())) == 0))
			{
				szBuffer += NEWLINE + gDLL->getText("TXT_KEY_PROJECT_REQUIRES_ANYONE", GC.getProjectInfo((ProjectTypes)GC.getProjectInfo(eProject).getAnyoneProjectPrereq()).getTextKeyWide());
			}
		}

		for (iI = 0; iI < GC.getNumProjectInfos(); iI++)
		{
			if (GC.getProjectInfo(eProject).getProjectsNeeded(iI) > 0)
			{
				if ((pCity == NULL) || (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getProjectCount((ProjectTypes)iI) < GC.getProjectInfo(eProject).getProjectsNeeded(iI)))
				{
					if (pCity != NULL)
					{
						szBuffer += NEWLINE + gDLL->getText("TXT_KEY_PROJECT_REQUIRES", GC.getProjectInfo((ProjectTypes)iI).getTextKeyWide(), GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getProjectCount((ProjectTypes)iI), GC.getProjectInfo(eProject).getProjectsNeeded(iI));
					}
					else
					{
						szBuffer += NEWLINE + gDLL->getText("TXT_KEY_PROJECT_REQUIRES_NO_CITY", GC.getProjectInfo((ProjectTypes)iI).getTextKeyWide(), GC.getProjectInfo(eProject).getProjectsNeeded(iI));
					}
				}
			}
		}

		if (bCivilopediaText)
		{
			if (GC.getProjectInfo(eProject).getVictoryPrereq() != NO_VICTORY)
			{
//				szTempBuffer.Format(L"\n%cRequires %s Victory", gDLL->getSymbolID(BULLET_CHAR), GC.getVictoryInfo((VictoryTypes)(GC.getProjectInfo(eProject).getVictoryPrereq())).getDescription());
				szBuffer += NEWLINE + gDLL->getText("TXT_KEY_PROJECT_REQUIRES_STRING_VICTORY", GC.getVictoryInfo((VictoryTypes)(GC.getProjectInfo(eProject).getVictoryPrereq())).getTextKeyWide());
//				szBuffer += szTempBuffer;
			}
		}
	}

	if (!bCivilopediaText)
	{
		if (pCity == NULL)
		{
			if (ePlayer != NO_PLAYER)
			{
				szTempBuffer.Format(L"\n%d%c", GET_PLAYER(ePlayer).getProductionNeeded(eProject), GC.getYieldInfo(YIELD_PRODUCTION).getChar());
			}
			else
			{
				szTempBuffer.Format(L"\n%d%c", GC.getProjectInfo(eProject).getProductionCost(), GC.getYieldInfo(YIELD_PRODUCTION).getChar());
			}
			szBuffer += szTempBuffer;
		}
		else
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_PROJECT_NUM_TURNS", pCity->getProductionTurnsLeft(eProject, ((gDLL->ctrlKey() || !(gDLL->shiftKey())) ? 0 : pCity->getOrderQueueLength())));

			iProduction = pCity->getProjectProduction(eProject);

			if (iProduction > 0)
			{
				szTempBuffer.Format(L" - %d/%d%c", iProduction, GET_PLAYER(ePlayer).getProductionNeeded(eProject), GC.getYieldInfo(YIELD_PRODUCTION).getChar());
			}
			else
			{
				szTempBuffer.Format(L" - %d%c", GET_PLAYER(ePlayer).getProductionNeeded(eProject), GC.getYieldInfo(YIELD_PRODUCTION).getChar());
			}
			szBuffer += szTempBuffer;
		}
	}

	for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
	{
		if (GC.getProjectInfo(eProject).getBonusProductionModifier(iI) != 0)
		{
			if (pCity != NULL)
			{
				if (pCity->hasBonus((BonusTypes)iI))
				{
					szBuffer += gDLL->getText("TXT_KEY_COLOR_POSITIVE");
				}
				else
				{
					szBuffer += gDLL->getText("TXT_KEY_COLOR_NEGATIVE");
				}
			}
			if (!bCivilopediaText)
			{
				szBuffer += L" (";
			}
			else
			{
				szTempBuffer.Format(L"%s%c", NEWLINE, gDLL->getSymbolID(BULLET_CHAR), szTempBuffer);
				szBuffer += szTempBuffer;
			}
			if (GC.getProjectInfo(eProject).getBonusProductionModifier(iI) == 100)
			{
//				szTempBuffer.Format(L"Double Speed with %c", GC.getBonusInfo((BonusTypes) iI).getChar());
				szBuffer += gDLL->getText("TXT_KEY_PROJECT_DOUBLE_SPEED_WITH", GC.getBonusInfo((BonusTypes)iI).getTextKeyWide());
			}
			else
			{
//				szTempBuffer.Format(L"Builds %d%% Faster with %c", GC.getProjectInfo(eProject).getBonusProductionModifier(iI), GC.getBonusInfo((BonusTypes) iI).getChar());
				szBuffer += gDLL->getText("TXT_KEY_PROJECT_BUILDS_FASTER_WITH", GC.getProjectInfo(eProject).getBonusProductionModifier(iI), GC.getBonusInfo((BonusTypes)iI).getTextKeyWide());
			}
//			szBuffer += szTempBuffer;
			if (!bCivilopediaText)
			{
				szBuffer += L")";
			}
			if (pCity != NULL)
			{
				szBuffer += gDLL->getText("TXT_KEY_COLOR_REVERT");
			}
		}
	}
}


void CvGameTextMgr::setProcessHelp(CvWString &szBuffer, ProcessTypes eProcess)
{
	int iI;

	szBuffer += GC.getProcessInfo(eProcess).getDescription();

	for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		if (GC.getProcessInfo(eProcess).getProductionToCommerceModifier(iI) != 0)
		{
//			szTempBuffer.Format(L"\n%cConverts %d%% of %c to %c", gDLL->getSymbolID(BULLET_CHAR), GC.getProcessInfo(eProcess).getProductionToCommerceModifier(iI), GC.getYieldInfo(YIELD_PRODUCTION).getChar(), GC.getCommerceInfo((CommerceTypes) iI).getChar());
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_PROCESS_CONVERTS", GC.getProcessInfo(eProcess).getProductionToCommerceModifier(iI), GC.getYieldInfo(YIELD_PRODUCTION).getChar(), GC.getCommerceInfo((CommerceTypes) iI).getChar());
//			szBuffer += szTempBuffer;
		}
	}
}

void CvGameTextMgr::setBadHealthHelp(CvWString &szBuffer, CvCity& city)
{
	CvPlot* pLoopPlot;
	FeatureTypes eFeature;
	int iHealth;
	int iI;

	if (city.badHealth() > 0)
	{
		iHealth = -(city.getFreshWaterBadHealth());
		if (iHealth > 0)
		{
			//				szTempBuffer.Format("+%d%c from Fresh Water\n", iHealth, CIV.getGameFont()->getSymbolID(UNHEALTHY_CHAR));
			szBuffer += gDLL->getText("TXT_KEY_MISC_FROM_FRESH_WATER", iHealth) + NEWLINE;
		}

		iHealth = -(city.getFeatureBadHealth());
		if (iHealth > 0)
		{
			eFeature = NO_FEATURE;

			for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
			{
				pLoopPlot = plotCity(city.getX_INLINE(), city.getY_INLINE(), iI);

				if (pLoopPlot != NULL)
				{
					if (pLoopPlot->getFeatureType() != NO_FEATURE)
					{
						if (GC.getFeatureInfo(pLoopPlot->getFeatureType()).getHealthPercent() < 0)
						{
							if (eFeature == NO_FEATURE)
							{
								eFeature = pLoopPlot->getFeatureType();
							}
							else if (eFeature != pLoopPlot->getFeatureType())
							{
								eFeature = NO_FEATURE;
								break;
							}
						}
					}
				}
			}

			//				szTempBuffer.Format("+%d%c from %s\n", iHealth, CIV.getGameFont()->getSymbolID(UNHEALTHY_CHAR), ((eFeature == NO_FEATURE) ? "Features" : GC.getFeatureInfo(eFeature).getDescription())); // XXX bad help...
			szBuffer += gDLL->getText("TXT_KEY_MISC_FEAT_HEALTH", iHealth, ((eFeature == NO_FEATURE) ? L"TXT_KEY_MISC_FEATURES" : GC.getFeatureInfo(eFeature).getTextKeyWide())) + NEWLINE;
		}

		iHealth = -(city.getPowerBadHealth());
		if (iHealth > 0)
		{
			//				szTempBuffer.Format("+%d%c from Power\n", iHealth, CIV.getGameFont()->getSymbolID(UNHEALTHY_CHAR));
			szBuffer += gDLL->getText("TXT_KEY_MISC_HEALTH_FROM_POWER", iHealth) + NEWLINE;
		}

		iHealth = -(city.getBonusBadHealth());
		if (iHealth > 0)
		{
			//				szTempBuffer.Format("+%d%c from Bonuses\n", iHealth, CIV.getGameFont()->getSymbolID(UNHEALTHY_CHAR));
			szBuffer += gDLL->getText("TXT_KEY_MISC_HEALTH_FROM_BONUSES", iHealth) + NEWLINE;
		}

		iHealth = -(city.totalBadBuildingHealth());
		if (iHealth > 0)
		{
			//				szTempBuffer.Format("+%d%c from Buildings\n", iHealth, CIV.getGameFont()->getSymbolID(UNHEALTHY_CHAR));
			szBuffer += gDLL->getText("TXT_KEY_MISC_HEALTH_FROM_BUILDINGS", iHealth) + NEWLINE;
		}

		iHealth = -(GET_PLAYER(city.getOwnerINLINE()).getExtraHealth());
		if (iHealth > 0)
		{
			//				szTempBuffer.Format("+%d%c from Civilization\n", iHealth, CIV.getGameFont()->getSymbolID(UNHEALTHY_CHAR));
			szBuffer += gDLL->getText("TXT_KEY_MISC_HEALTH_FROM_CIV", iHealth) + NEWLINE;
		}

		iHealth = -(GC.getHandicapInfo(city.getHandicapType()).getHealthBonus());
		if (iHealth > 0)
		{
			//				szTempBuffer.Format("+%d%c from Handicap\n", iHealth, CIV.getGameFont()->getSymbolID(UNHEALTHY_CHAR));
			szBuffer += gDLL->getText("TXT_KEY_MISC_HEALTH_FROM_HANDICAP", iHealth) + NEWLINE;
		}

		iHealth = city.unhealthyPopulation();
		if (iHealth > 0)
		{
			//				szTempBuffer.Format("+%d%c from Population\n", iHealth, CIV.getGameFont()->getSymbolID(UNHEALTHY_CHAR));
			szBuffer += gDLL->getText("TXT_KEY_MISC_HEALTH_FROM_POP", iHealth) + NEWLINE;
		}

		szBuffer += L"-----------------------\n";

		//			szTempBuffer.Format("%d%c Total Unhealthiness", city.badHealth(), CIV.getGameFont()->getSymbolID(UNHEALTHY_CHAR));
		szBuffer += gDLL->getText("TXT_KEY_MISC_TOTAL_UNHEALTHY", city.badHealth());
	}
}

void CvGameTextMgr::setGoodHealthHelp(CvWString &szBuffer, CvCity& city)
{
	CvPlot* pLoopPlot;
	FeatureTypes eFeature;
	int iHealth;
	int iI;

	if (city.goodHealth() > 0)
	{
		iHealth = city.getFreshWaterGoodHealth();
		if (iHealth > 0)
		{
			//				szTempBuffer.Format("+%d%c from Fresh Water\n", iHealth, CIV.getGameFont()->getSymbolID(UNHEALTHY_CHAR));
			szBuffer += gDLL->getText("TXT_KEY_MISC_HEALTH_FROM_FRESH_WATER", iHealth) + NEWLINE;
		}

		iHealth = city.getFeatureGoodHealth();
		if (iHealth > 0)
		{
			eFeature = NO_FEATURE;

			for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
			{
				pLoopPlot = plotCity(city.getX_INLINE(), city.getY_INLINE(), iI);

				if (pLoopPlot != NULL)
				{
					if (pLoopPlot->getFeatureType() != NO_FEATURE)
					{
						if (GC.getFeatureInfo(pLoopPlot->getFeatureType()).getHealthPercent() > 0)
						{
							if (eFeature == NO_FEATURE)
							{
								eFeature = pLoopPlot->getFeatureType();
							}
							else if (eFeature != pLoopPlot->getFeatureType())
							{
								eFeature = NO_FEATURE;
								break;
							}
						}
					}
				}
			}

			//				szTempBuffer.Format("+%d%c from %s\n", iHealth, CIV.getGameFont()->getSymbolID(UNHEALTHY_CHAR), ((eFeature == NO_FEATURE) ? "Features" : GC.getFeatureInfo(eFeature).getDescription())); // XXX bad help...
			szBuffer += gDLL->getText("TXT_KEY_MISC_FEAT_GOOD_HEALTH", iHealth, ((eFeature == NO_FEATURE) ? L"TXT_KEY_MISC_FEATURES" : GC.getFeatureInfo(eFeature).getTextKeyWide())) + NEWLINE;
		}

		iHealth = city.getPowerGoodHealth();
		if (iHealth > 0)
		{
			//				szTempBuffer.Format("+%d%c from Power\n", iHealth, CIV.getGameFont()->getSymbolID(UNHEALTHY_CHAR));
			szBuffer += gDLL->getText("TXT_KEY_MISC_GOOD_HEALTH_FROM_POWER", iHealth) + NEWLINE;
		}

		iHealth = city.getBonusGoodHealth();
		if (iHealth > 0)
		{
			//				szTempBuffer.Format("+%d%c from Bonuses\n", iHealth, CIV.getGameFont()->getSymbolID(UNHEALTHY_CHAR));
			szBuffer += gDLL->getText("TXT_KEY_MISC_GOOD_HEALTH_FROM_BONUSES", iHealth) + NEWLINE;
		}

		iHealth = city.totalGoodBuildingHealth();
		if (iHealth > 0)
		{
			//				szTempBuffer.Format("+%d%c from Buildings\n", iHealth, CIV.getGameFont()->getSymbolID(UNHEALTHY_CHAR));
			szBuffer += gDLL->getText("TXT_KEY_MISC_GOOD_HEALTH_FROM_BUILDINGS", iHealth) + NEWLINE;
		}

		iHealth = GET_PLAYER(city.getOwnerINLINE()).getExtraHealth();
		if (iHealth > 0)
		{
			//				szTempBuffer.Format("+%d%c from Civilization\n", iHealth, CIV.getGameFont()->getSymbolID(UNHEALTHY_CHAR));
			szBuffer += gDLL->getText("TXT_KEY_MISC_GOOD_HEALTH_FROM_CIV", iHealth) + NEWLINE;
		}

		iHealth = GC.getHandicapInfo(city.getHandicapType()).getHealthBonus();
		if (iHealth > 0)
		{
			//				szTempBuffer.Format("+%d%c from Handicap\n", iHealth, CIV.getGameFont()->getSymbolID(UNHEALTHY_CHAR));
			szBuffer += gDLL->getText("TXT_KEY_MISC_GOOD_HEALTH_FROM_HANDICAP", iHealth) + NEWLINE;
		}

		szBuffer += L"-----------------------\n";

		//			szTempBuffer.Format("%d%c Total Unhealthiness", city.badHealth(), CIV.getGameFont()->getSymbolID(UNHEALTHY_CHAR));
		szBuffer += gDLL->getText("TXT_KEY_MISC_TOTAL_HEALTHY", city.goodHealth());
	}
}

void CvGameTextMgr::setAngerHelp(CvWString &szBuffer, CvCity& city)
{
	int iOldAngerPercent;
	int iNewAngerPercent;
	int iOldAnger;
	int iNewAnger;
	int iAnger;
	int iI;

	if (city.isOccupation())
	{
//		szBuffer += "\"Viva la resistance!\"";
		szBuffer += gDLL->getText("TXT_KEY_ANGER_RESISTANCE");
	}
	else if (GET_PLAYER(city.getOwnerINLINE()).isAnarchy())
	{
//		szBuffer += "Our civilization is in Anarchy!";
		szBuffer += gDLL->getText("TXT_KEY_ANGER_ANARCHY");
	}
	else if (city.unhappyLevel() > 0)
	{
		iOldAngerPercent = 0;
		iNewAngerPercent = 0;

		iOldAnger = 0;
		iNewAnger = 0;

		// XXX decomp these???
		iNewAngerPercent += city.getOvercrowdingPercentAnger();
		iNewAnger += (((iNewAngerPercent * city.getPopulation()) / GC.getDefineINT("PERCENT_ANGER_DIVISOR")) - ((iOldAngerPercent * city.getPopulation()) / GC.getDefineINT("PERCENT_ANGER_DIVISOR")));
		iAnger = ((iNewAnger - iOldAnger) + min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer += gDLL->getText("TXT_KEY_ANGER_OVERCROWDING", iAnger) + NEWLINE;
		}
		iOldAngerPercent = iNewAngerPercent;
		iOldAnger = iNewAnger;

		iNewAngerPercent += city.getNoMilitaryPercentAnger();
		iNewAnger += (((iNewAngerPercent * city.getPopulation()) / GC.getDefineINT("PERCENT_ANGER_DIVISOR")) - ((iOldAngerPercent * city.getPopulation()) / GC.getDefineINT("PERCENT_ANGER_DIVISOR")));
		iAnger = ((iNewAnger - iOldAnger) + min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer += gDLL->getText("TXT_KEY_ANGER_MILITARY_PROTECTION", iAnger) + NEWLINE;
		}
		iOldAngerPercent = iNewAngerPercent;
		iOldAnger = iNewAnger;

		iNewAngerPercent += city.getCulturePercentAnger();
		iNewAnger += (((iNewAngerPercent * city.getPopulation()) / GC.getDefineINT("PERCENT_ANGER_DIVISOR")) - ((iOldAngerPercent * city.getPopulation()) / GC.getDefineINT("PERCENT_ANGER_DIVISOR")));
		iAnger = ((iNewAnger - iOldAnger) + min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer += gDLL->getText("TXT_KEY_ANGER_OCCUPIED", iAnger) + NEWLINE;
		}
		iOldAngerPercent = iNewAngerPercent;
		iOldAnger = iNewAnger;

		iNewAngerPercent += city.getReligionPercentAnger();
		iNewAnger += (((iNewAngerPercent * city.getPopulation()) / GC.getDefineINT("PERCENT_ANGER_DIVISOR")) - ((iOldAngerPercent * city.getPopulation()) / GC.getDefineINT("PERCENT_ANGER_DIVISOR")));
		iAnger = ((iNewAnger - iOldAnger) + min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer += gDLL->getText("TXT_KEY_ANGER_RELIGION_FIGHT", iAnger) + NEWLINE;
		}
		iOldAngerPercent = iNewAngerPercent;
		iOldAnger = iNewAnger;

		iNewAngerPercent += city.getHurryPercentAnger();
		iNewAnger += (((iNewAngerPercent * city.getPopulation()) / GC.getDefineINT("PERCENT_ANGER_DIVISOR")) - ((iOldAngerPercent * city.getPopulation()) / GC.getDefineINT("PERCENT_ANGER_DIVISOR")));
		iAnger = ((iNewAnger - iOldAnger) + min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer += gDLL->getText("TXT_KEY_ANGER_OPPRESSION", iAnger) + NEWLINE;
		}
		iOldAngerPercent = iNewAngerPercent;
		iOldAnger = iNewAnger;

		iNewAngerPercent += city.getConscriptPercentAnger();
		iNewAnger += (((iNewAngerPercent * city.getPopulation()) / GC.getDefineINT("PERCENT_ANGER_DIVISOR")) - ((iOldAngerPercent * city.getPopulation()) / GC.getDefineINT("PERCENT_ANGER_DIVISOR")));
		iAnger = ((iNewAnger - iOldAnger) + min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer += gDLL->getText("TXT_KEY_ANGER_DRAFT", iAnger) + NEWLINE;
		}
		iOldAngerPercent = iNewAngerPercent;
		iOldAnger = iNewAnger;

		iNewAngerPercent += city.getWarWearinessPercentAnger();
		iNewAnger += (((iNewAngerPercent * city.getPopulation()) / GC.getDefineINT("PERCENT_ANGER_DIVISOR")) - ((iOldAngerPercent * city.getPopulation()) / GC.getDefineINT("PERCENT_ANGER_DIVISOR")));
		iAnger = ((iNewAnger - iOldAnger) + min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer += gDLL->getText("TXT_KEY_ANGER_WAR_WEAR", iAnger) + NEWLINE;
		}
		iOldAngerPercent = iNewAngerPercent;
		iOldAnger = iNewAnger;

		iNewAnger += max(0, city.getVassalUnhappiness());
		iAnger = ((iNewAnger - iOldAnger) + min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer += gDLL->getText("TXT_KEY_UNHAPPY_VASSAL", iAnger) + NEWLINE;
		}
		iOldAnger = iNewAnger;

		for (iI = 0; iI < GC.getNumCivicInfos(); iI++)
		{
			iNewAngerPercent += GET_PLAYER(city.getOwnerINLINE()).getCivicPercentAnger((CivicTypes)iI);
			iNewAnger += (((iNewAngerPercent * city.getPopulation()) / GC.getDefineINT("PERCENT_ANGER_DIVISOR")) - ((iOldAngerPercent * city.getPopulation()) / GC.getDefineINT("PERCENT_ANGER_DIVISOR")));
			iAnger = ((iNewAnger - iOldAnger) + min(0, iOldAnger));
			if (iAnger > 0)
			{
				szBuffer += gDLL->getText("TXT_KEY_ANGER_DEMAND_CIVIC", iAnger, GC.getCivicInfo((CivicTypes) iI).getTextKeyWide()) + NEWLINE;
			}
			iOldAngerPercent = iNewAngerPercent;
			iOldAnger = iNewAnger;
		}

		iNewAnger -= min(0, city.getLargestCityHappiness());
		iAnger = ((iNewAnger - iOldAnger) + min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer += gDLL->getText("TXT_KEY_ANGER_BIG_CITY", iAnger) + NEWLINE;
		}
		iOldAnger = iNewAnger;

		iNewAnger -= min(0, city.getMilitaryHappiness());
		iAnger = ((iNewAnger - iOldAnger) + min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer += gDLL->getText("TXT_KEY_ANGER_MILITARY_PRESENCE", iAnger) + NEWLINE;
		}
		iOldAnger = iNewAnger;

		iNewAnger -= min(0, city.getCurrentStateReligionHappiness());
		iAnger = ((iNewAnger - iOldAnger) + min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer += gDLL->getText("TXT_KEY_ANGER_STATE_RELIGION", iAnger) + NEWLINE;
		}
		iOldAnger = iNewAnger;

		iNewAnger -= min(0, (city.getBuildingBadHappiness() + city.getExtraBuildingBadHappiness()));
		iAnger = ((iNewAnger - iOldAnger) + min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer += gDLL->getText("TXT_KEY_ANGER_BUILDINGS", iAnger) + NEWLINE;
		}
		iOldAnger = iNewAnger;

		iNewAnger -= min(0, city.getFeatureBadHappiness());
		iAnger = ((iNewAnger - iOldAnger) + min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer += gDLL->getText("TXT_KEY_ANGER_FEATURES", iAnger) + NEWLINE;
		}
		iOldAnger = iNewAnger;

		iNewAnger -= min(0, city.getBonusBadHappiness());
		iAnger = ((iNewAnger - iOldAnger) + min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer += gDLL->getText("TXT_KEY_ANGER_BONUS", iAnger) + NEWLINE;
		}
		iOldAnger = iNewAnger;

		iNewAnger -= min(0, city.getReligionBadHappiness());
		iAnger = ((iNewAnger - iOldAnger) + min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer += gDLL->getText("TXT_KEY_ANGER_RELIGIOUS_FREEDOM", iAnger) + NEWLINE;
		}
		iOldAnger = iNewAnger;

		iNewAnger -= min(0, city.getCommerceHappiness());
		iAnger = ((iNewAnger - iOldAnger) + min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer += gDLL->getText("TXT_KEY_ANGER_BAD_ENTERTAINMENT", iAnger) + NEWLINE;
		}
		iOldAnger = iNewAnger;

		iNewAnger -= min(0, city.area()->getBuildingHappiness(city.getOwnerINLINE()));
		iAnger = ((iNewAnger - iOldAnger) + min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer += gDLL->getText("TXT_KEY_ANGER_BUILDINGS", iAnger) + NEWLINE;
		}
		iOldAnger = iNewAnger;

		iNewAnger -= min(0, GET_PLAYER(city.getOwnerINLINE()).getBuildingHappiness());
		iAnger = ((iNewAnger - iOldAnger) + min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer += gDLL->getText("TXT_KEY_ANGER_BUILDINGS", iAnger) + NEWLINE;
		}
		iOldAnger = iNewAnger;

		iNewAnger -= min(0, (city.getExtraHappiness() + GET_PLAYER(city.getOwnerINLINE()).getExtraHappiness()));
		iAnger = ((iNewAnger - iOldAnger) + min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer += gDLL->getText("TXT_KEY_ANGER_ARGH", iAnger) + NEWLINE;
		}
		iOldAnger = iNewAnger;

		iNewAnger -= min(0, GC.getHandicapInfo(city.getHandicapType()).getHappyBonus());
		iAnger = ((iNewAnger - iOldAnger) + min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer += gDLL->getText("TXT_KEY_ANGER_HANDICAP", iAnger) + NEWLINE;
		}
		iOldAnger = iNewAnger;

		szBuffer += L"-----------------------\n";

		szBuffer += gDLL->getText("TXT_KEY_ANGER_TOTAL_UNHAPPY", iOldAnger);

		FAssert(iOldAnger == city.unhappyLevel());
	}
}


void CvGameTextMgr::setHappyHelp(CvWString &szBuffer, CvCity& city)
{
	int iHappy;
	int iTotalHappy = 0;

	if (city.isOccupation() || GET_PLAYER(city.getOwnerINLINE()).isAnarchy())
	{
		return;
	}
	if (city.happyLevel() > 0)
	{
		iHappy = city.getLargestCityHappiness();
		if (iHappy > 0)
		{
			iTotalHappy += iHappy;
			szBuffer += gDLL->getText("TXT_KEY_HAPPY_BIG_CITY", iHappy) + NEWLINE;
		}

		iHappy = city.getMilitaryHappiness();
		if (iHappy > 0)
		{
			iTotalHappy += iHappy;
			szBuffer += gDLL->getText("TXT_KEY_HAPPY_MILITARY_PRESENCE", iHappy) + NEWLINE;
		}

		iHappy = city.getVassalHappiness();
		if (iHappy > 0)
		{
			iTotalHappy += iHappy;
			szBuffer += gDLL->getText("TXT_KEY_HAPPY_VASSAL", iHappy) + NEWLINE;
		}

		iHappy = city.getCurrentStateReligionHappiness();
		if (iHappy > 0)
		{
			iTotalHappy += iHappy;
			szBuffer += gDLL->getText("TXT_KEY_HAPPY_STATE_RELIGION", iHappy) + NEWLINE;
		}

		iHappy = (city.getBuildingGoodHappiness() + city.getExtraBuildingGoodHappiness());
		if (iHappy > 0)
		{
			iTotalHappy += iHappy;
			szBuffer += gDLL->getText("TXT_KEY_HAPPY_BUILDINGS", iHappy) + NEWLINE;
		}

		iHappy = city.getFeatureGoodHappiness();
		if (iHappy > 0)
		{
			iTotalHappy += iHappy;
			szBuffer += gDLL->getText("TXT_KEY_HAPPY_FEATURES", iHappy) + NEWLINE;
		}

		iHappy = city.getBonusGoodHappiness();
		if (iHappy > 0)
		{
			iTotalHappy += iHappy;
			szBuffer += gDLL->getText("TXT_KEY_HAPPY_BONUS", iHappy) + NEWLINE;
		}

		iHappy = city.getReligionGoodHappiness();
		if (iHappy > 0)
		{
			iTotalHappy += iHappy;
			szBuffer += gDLL->getText("TXT_KEY_HAPPY_RELIGIOUS_FREEDOM", iHappy) + NEWLINE;
		}

		iHappy = city.getCommerceHappiness();
		if (iHappy > 0)
		{
			iTotalHappy += iHappy;
			szBuffer += gDLL->getText("TXT_KEY_HAPPY_ENTERTAINMENT", iHappy) + NEWLINE;
		}

		iHappy = city.area()->getBuildingHappiness(city.getOwnerINLINE());
		if (iHappy > 0)
		{
			iTotalHappy += iHappy;
			szBuffer += gDLL->getText("TXT_KEY_HAPPY_BUILDINGS", iHappy) + NEWLINE;
		}

		iHappy = GET_PLAYER(city.getOwnerINLINE()).getBuildingHappiness();
		if (iHappy > 0)
		{
			iTotalHappy += iHappy;
			szBuffer += gDLL->getText("TXT_KEY_HAPPY_BUILDINGS", iHappy) + NEWLINE;
		}

		iHappy = (city.getExtraHappiness() + GET_PLAYER(city.getOwnerINLINE()).getExtraHappiness());
		if (iHappy > 0)
		{
			iTotalHappy += iHappy;
			szBuffer += gDLL->getText("TXT_KEY_HAPPY_YEAH", iHappy) + NEWLINE;
		}

		iHappy = GC.getHandicapInfo(city.getHandicapType()).getHappyBonus();
		if (iHappy > 0)
		{
			iTotalHappy += iHappy;
			szBuffer += gDLL->getText("TXT_KEY_HAPPY_HANDICAP", iHappy) + NEWLINE;
		}

		szBuffer += L"-----------------------\n";

		szBuffer += gDLL->getText("TXT_KEY_HAPPY_TOTAL_HAPPY", iTotalHappy);

		FAssert(iTotalHappy == city.happyLevel())
	}
}


void CvGameTextMgr::setYieldChangeHelp(CvWString &szBuffer, const CvWString& szStart, const CvWString& szSpace, const CvWString& szEnd, const int* piYieldChange, bool bPercent, bool bNewLine)
{
	CvWString szTempBuffer;
	bool bStarted;
	int iI;

	bStarted = false;

	for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		if (piYieldChange[iI] != 0)
		{
			if (!bStarted)
			{
				if (bNewLine)
				{
					szTempBuffer.Format(L"\n%c", gDLL->getSymbolID(BULLET_CHAR));
				}
				szTempBuffer += CvWString::format(L"%s%s%s%d%s%c",
					szStart.GetCString(),
					szSpace.GetCString(),
					piYieldChange[iI] > 0 ? L"+" : L"",
					piYieldChange[iI],
					bPercent ? L"%" : L"",
					GC.getYieldInfo((YieldTypes)iI).getChar());
			}
			else
			{
				szTempBuffer.Format(L", %s%d%s%c",
					piYieldChange[iI] > 0 ? L"+" : L"",
					piYieldChange[iI],
					bPercent ? L"%" : L"",
					GC.getYieldInfo((YieldTypes)iI).getChar());
			}
			szBuffer += szTempBuffer;

			bStarted = true;
		}
	}

	if (bStarted)
	{
		szBuffer += szEnd;
	}
}

void CvGameTextMgr::setCommerceChangeHelp(CvWString &szBuffer, const CvWString& szStart, const CvWString& szSpace, const CvWString& szEnd, const int* piCommerceChange, bool bPercent, bool bNewLine)
{
	CvWString szTempBuffer;
	bool bStarted;
	int iI;

	bStarted = false;

	for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		if (piCommerceChange[iI] != 0)
		{
			if (!bStarted)
			{
				if (bNewLine)
				{
					szTempBuffer.Format(L"\n%c", gDLL->getSymbolID(BULLET_CHAR));
				}
				szTempBuffer += CvWString::format(L"%s%s%s%d%s%c", szStart.GetCString(), szSpace.GetCString(), ((piCommerceChange[iI] > 0) ? L"+" : L""), piCommerceChange[iI], ((bPercent) ? L"%" : L""), GC.getCommerceInfo((CommerceTypes) iI).getChar());
			}
			else
			{
				szTempBuffer.Format(L", %s%d%s%c", ((piCommerceChange[iI] > 0) ? L"+" : L""), piCommerceChange[iI], ((bPercent) ? L"%" : L""), GC.getCommerceInfo((CommerceTypes) iI).getChar());
			}
			szBuffer += szTempBuffer;

			bStarted = true;
		}
	}

	if (bStarted)
	{
		szBuffer += szEnd;
	}
}

void CvGameTextMgr::setBonusHelp(CvWString &szBuffer, BonusTypes eBonus, bool bCivilopediaText)
{
	CvWString szTempBuffer;

	if (NO_BONUS == eBonus)
	{
		return;
	}
	if (!bCivilopediaText)
	{
		szTempBuffer.Format( SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getBonusInfo(eBonus).getDescription());
		szBuffer += szTempBuffer;

		setYieldChangeHelp(szBuffer, L"", L"", L"", GC.getBonusInfo(eBonus).getYieldChangeArray());

		if (GC.getBonusInfo(eBonus).getTechReveal() != NO_TECH)
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BONUS_REVEALED_BY", GC.getTechInfo((TechTypes)GC.getBonusInfo(eBonus).getTechReveal()).getTextKeyWide());
		}
	}

	ImprovementTypes eImprovement = NO_IMPROVEMENT;
	for (int iLoopImprovement = 0; iLoopImprovement < GC.getNumImprovementInfos(); iLoopImprovement++)
	{
		if (GC.getImprovementInfo((ImprovementTypes)iLoopImprovement).isImprovementBonusMakesValid(eBonus))
		{
			eImprovement = (ImprovementTypes)iLoopImprovement;
			break;
		}
	}

	if (GC.getBonusInfo(eBonus).getHealth() != 0)
	{
		if (GC.getBonusInfo(eBonus).getHealth() > 0)
		{
//			szTempBuffer.Format(L"\nHealthy: +%d%c", GC.getBonusInfo(eBonus).getHealth(), gDLL->getSymbolID(HEALTHY_CHAR));
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BONUS_HEALTHY", GC.getBonusInfo(eBonus).getHealth());
		}
		else
		{
//			szTempBuffer.Format(L"\nUnhealthy: +%d%c", -GC.getBonusInfo(eBonus).getHealth(), gDLL->getSymbolID(UNHEALTHY_CHAR));
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BONUS_UNHEALTHY", -GC.getBonusInfo(eBonus).getHealth());
		}
//		szBuffer += szTempBuffer;
		if (eImprovement != NO_IMPROVEMENT)
		{
//			szTempBuffer.Format(L"(with %s)", GC.getImprovementInfo(eImprovement).getDescription());
			szBuffer += gDLL->getText("TXT_KEY_BONUS_WITH_IMPROVEMENT", GC.getImprovementInfo(eImprovement).getTextKeyWide());
//			szBuffer += szTempBuffer;
		}
	}

	if (GC.getBonusInfo(eBonus).getHappiness() != 0)
	{
		if (GC.getBonusInfo(eBonus).getHappiness() > 0)
		{
//			szTempBuffer.Format(L"\nHappy: +%d%c", GC.getBonusInfo(eBonus).getHappiness(), gDLL->getSymbolID(HAPPY_CHAR));
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BONUS_HAPPY", GC.getBonusInfo(eBonus).getHappiness());
		}
		else
		{
//			szTempBuffer.Format(L"\nUnhappy: +%d%c", -GC.getBonusInfo(eBonus).getHappiness(), gDLL->getSymbolID(UNHAPPY_CHAR));
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BONUS_UNHAPPY", -GC.getBonusInfo(eBonus).getHappiness());
		}
//		szBuffer += szTempBuffer;
		if (eImprovement != NO_IMPROVEMENT)
		{
//			szTempBuffer.Format(L"(with %s)", GC.getImprovementInfo(eImprovement).getDescription());
			szBuffer += gDLL->getText("TXT_KEY_BONUS_WITH_IMPROVEMENT", GC.getImprovementInfo(eImprovement).getTextKeyWide());
//			szBuffer += szTempBuffer;
		}
	}

	CivilizationTypes eCivilization = GC.getGameINLINE().getActiveCivilizationType();

	for (int i = 0; i < GC.getNumBuildingClassInfos(); i++)
	{
		BuildingTypes eLoopBuilding;
		if (eCivilization == NO_CIVILIZATION)
		{
			eLoopBuilding = ((BuildingTypes)(GC.getBuildingClassInfo((BuildingClassTypes)i).getDefaultBuildingIndex()));
		}
		else
		{
			eLoopBuilding = ((BuildingTypes)(GC.getCivilizationInfo(eCivilization).getCivilizationBuildings(i)));
		}

		if (eLoopBuilding != NO_BUILDING)
		{
			CvBuildingInfo& kBuilding = GC.getBuildingInfo(eLoopBuilding);
			if (kBuilding.getBonusHappinessChanges(eBonus) != 0)
			{
				if (kBuilding.getBonusHappinessChanges(eBonus) > 0)
				{
					szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BONUS_HAPPY", kBuilding.getBonusHappinessChanges(eBonus));
				}
				else
				{
					szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BONUS_UNHAPPY", -kBuilding.getBonusHappinessChanges(eBonus));
				}
				szBuffer += gDLL->getText("TXT_KEY_BONUS_WITH_IMPROVEMENT", kBuilding.getTextKeyWide());
			}

			if (kBuilding.getBonusHealthChanges(eBonus) != 0)
			{
				if (kBuilding.getBonusHealthChanges(eBonus) > 0)
				{
					szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BONUS_HEALTHY", kBuilding.getBonusHealthChanges(eBonus));
				}
				else
				{
					szBuffer += NEWLINE + gDLL->getText("TXT_KEY_BONUS_UNHEALTHY", -kBuilding.getBonusHealthChanges(eBonus));
				}
				szBuffer += gDLL->getText("TXT_KEY_BONUS_WITH_IMPROVEMENT", kBuilding.getTextKeyWide());
			}
		}
	}
}

void CvGameTextMgr::setReligionHelp(CvWString &szBuffer, ReligionTypes eReligion, bool bCivilopedia)
{
	UnitTypes eFreeUnit;

	if (NO_RELIGION == eReligion)
	{
		return;
	}
	CvReligionInfo& religion = GC.getReligionInfo(eReligion);

	if (!bCivilopedia)
	{
		szBuffer += CvWString::format(SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), religion.getDescription());
	}

	setCommerceChangeHelp(szBuffer, gDLL->getText("TXT_KEY_RELIGION_HOLY_CITY").c_str(), L": ", L"", religion.getHolyCityCommerceArray());
	setCommerceChangeHelp(szBuffer, gDLL->getText("TXT_KEY_RELIGION_ALL_CITIES").c_str(), L": ", L"", religion.getStateReligionCommerceArray());

	if (!bCivilopedia)
	{
		if (religion.getTechPrereq() != NO_TECH)
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_RELIGION_FOUNDED_FIRST", GC.getTechInfo((TechTypes)religion.getTechPrereq()).getTextKeyWide());
		}
	}

	if (religion.getFreeUnitClass() != NO_UNITCLASS)
	{
		if (GC.getGameINLINE().getActivePlayer() != NO_PLAYER)
		{
			eFreeUnit = ((UnitTypes)(GC.getCivilizationInfo(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCivilizationType()).getCivilizationUnits(religion.getFreeUnitClass())));
		}
		else
		{
			eFreeUnit = (UnitTypes)GC.getUnitClassInfo((UnitClassTypes)religion.getFreeUnitClass()).getDefaultUnitIndex();
		}

		if (eFreeUnit != NO_UNIT)
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_RELIGION_FOUNDER_RECEIVES", GC.getUnitInfo(eFreeUnit).getTextKeyWide());
		}
	}
}

void CvGameTextMgr::setReligionHelpCity(CvWString &szBuffer, ReligionTypes eReligion, CvCity *pCity, bool bCityScreen, bool bForceReligion, bool bForceState, bool bNoStateReligion)
{
	int i;
	CvWString szTempBuffer;
	bool bHandled = false;
	int iCommerce;
	int iHappiness;
	int iProductionModifier;
	int iFreeExperience;
	int iGreatPeopleRateModifier;

	if (pCity == NULL)
	{
		return;
	}

	ReligionTypes eStateReligion = (bNoStateReligion ? NO_RELIGION : GET_PLAYER(pCity->getOwnerINLINE()).getStateReligion());

	if (bCityScreen)
	{
		szBuffer += CvWString::format(SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getReligionInfo(eReligion).getDescription());
		szBuffer += NEWLINE;

		if (!(GC.getGameINLINE().isReligionFounded(eReligion)))
		{
			if (GC.getReligionInfo(eReligion).getTechPrereq() != NO_TECH)
			{
				szBuffer += gDLL->getText("TXT_KEY_RELIGION_FOUNDED_FIRST", GC.getTechInfo((TechTypes)(GC.getReligionInfo(eReligion).getTechPrereq())).getTextKeyWide());
			}
		}
	}

	if (!bForceReligion)
	{
		if (!(pCity->isHasReligion(eReligion)))
		{
			return;
		}
	}

	if (eStateReligion == eReligion || eStateReligion == NO_RELIGION || bForceState)
	{
		for (i = 0; i < NUM_COMMERCE_TYPES; i++)
		{
			iCommerce = GC.getReligionInfo(eReligion).getStateReligionCommerce((CommerceTypes)i);

			if (pCity->isHolyCity(eReligion))
			{
				iCommerce += GC.getReligionInfo(eReligion).getHolyCityCommerce((CommerceTypes)i);
			}

			if (iCommerce != 0)
			{
				if (bHandled)
				{
					szBuffer += L", ";
				}

				szTempBuffer.Format(L"%s%d%c", iCommerce > 0 ? "+" : "", iCommerce, GC.getCommerceInfo((CommerceTypes)i).getChar());
				szBuffer += szTempBuffer;
				bHandled = true;
			}
		}
	}

	if (eStateReligion == eReligion || bForceState)
	{
		iHappiness = (pCity->getStateReligionHappiness(eReligion) + GET_PLAYER(pCity->getOwnerINLINE()).getStateReligionHappiness());

		if (iHappiness != 0)
		{
			if (bHandled)
			{
				szBuffer += L", ";
			}

			szTempBuffer.Format(L"%d%c", iHappiness, ((iHappiness > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)));
			szBuffer += szTempBuffer;
			bHandled = true;
		}

		iProductionModifier = GET_PLAYER(pCity->getOwnerINLINE()).getStateReligionBuildingProductionModifier();
		if (iProductionModifier != 0)
		{
			if (bHandled)
			{
				szBuffer += L", ";
			}

			szBuffer += gDLL->getText("TXT_KEY_RELIGION_BUILDING_PROD_MOD", iProductionModifier);
			bHandled = true;
		}

		iProductionModifier = GET_PLAYER(pCity->getOwnerINLINE()).getStateReligionUnitProductionModifier();
		if (iProductionModifier != 0)
		{
			if (bHandled)
			{
				szBuffer += L", ";
			}

			szBuffer += gDLL->getText("TXT_KEY_RELIGION_UNIT_PROD_MOD", iProductionModifier);
			bHandled = true;
		}

		iFreeExperience = GET_PLAYER(pCity->getOwnerINLINE()).getStateReligionFreeExperience();
		if (iFreeExperience != 0)
		{
			if (bHandled)
			{
				szBuffer += L", ";
			}

			szBuffer += gDLL->getText("TXT_KEY_RELIGION_FREE_XP", iFreeExperience);
			bHandled = true;
		}

		iGreatPeopleRateModifier = GET_PLAYER(pCity->getOwnerINLINE()).getStateReligionGreatPeopleRateModifier();
		if (iGreatPeopleRateModifier != 0)
		{
			if (bHandled)
			{
				szBuffer += L", ";
			}

			szBuffer += gDLL->getText("TXT_KEY_RELIGION_BIRTH_RATE_MOD", iGreatPeopleRateModifier);
			bHandled = true;
		}
	}
}

void CvGameTextMgr::buildObsoleteString(CvWString &szBuffer, int iItem, bool bList, bool bPlayerContext)
{
	CvWString szTempBuffer;

	if (bList)
	{
		szBuffer += NEWLINE;
	}
	szBuffer += gDLL->getText("TXT_KEY_TECH_OBSOLETES", GC.getBuildingInfo((BuildingTypes) iItem).getTextKeyWide());
}

void CvGameTextMgr::buildObsoleteBonusString(CvWString &szBuffer, int iItem, bool bList, bool bPlayerContext)
{
	CvWString szTempBuffer;

	if (bList)
	{
		szBuffer += NEWLINE;
	}
	szBuffer += gDLL->getText("TXT_KEY_TECH_OBSOLETES", GC.getBonusInfo((BonusTypes) iItem).getTextKeyWide());
}

void CvGameTextMgr::buildObsoleteSpecialString(CvWString &szBuffer, int iItem, bool bList, bool bPlayerContext)
{
	CvWString szTempBuffer;

	if (bList)
	{
		szBuffer += NEWLINE;
	}
	szBuffer += gDLL->getText("TXT_KEY_TECH_OBSOLETES_NO_LINK", GC.getSpecialBuildingInfo((SpecialBuildingTypes) iItem).getTextKeyWide());
}

void CvGameTextMgr::buildMoveString(CvWString &szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	int iI;
	int iMoveDiff;

	for (iI = 0; iI < GC.getNumRouteInfos(); iI++)
	{
		iMoveDiff = ((GC.getMOVE_DENOMINATOR() / max(1, (GC.getRouteInfo((RouteTypes) iI).getMovementCost() + ((bPlayerContext) ? GET_TEAM(GC.getGameINLINE().getActiveTeam()).getRouteChange((RouteTypes)iI) : 0)))) - (GC.getMOVE_DENOMINATOR() / max(1, (GC.getRouteInfo((RouteTypes) iI).getMovementCost() + ((bPlayerContext) ? GET_TEAM(GC.getGameINLINE().getActiveTeam()).getRouteChange((RouteTypes)iI) : 0) + GC.getRouteInfo((RouteTypes) iI).getTechMovementChange(eTech)))));

		if (iMoveDiff != 0)
		{
			if (bList)
			{
				szBuffer += NEWLINE;
			}
			szBuffer += gDLL->getText("TXT_KEY_UNIT_MOVEMENT", -(iMoveDiff), GC.getRouteInfo((RouteTypes) iI).getTextKeyWide());
			bList = true;
		}
	}
}

void CvGameTextMgr::buildFreeUnitString(CvWString &szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	UnitTypes eFreeUnit;

	if (GC.getTechInfo(eTech).getFirstFreeUnitClass() != NO_UNITCLASS)
	{
		if (GC.getGameINLINE().getActivePlayer() != NO_PLAYER)
		{
			eFreeUnit = ((UnitTypes)(GC.getCivilizationInfo(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCivilizationType()).getCivilizationUnits(GC.getTechInfo(eTech).getFirstFreeUnitClass())));
		}
		else
		{
			eFreeUnit = (UnitTypes)GC.getUnitClassInfo((UnitClassTypes)GC.getTechInfo(eTech).getFirstFreeUnitClass()).getDefaultUnitIndex();
		}

		if (eFreeUnit != NO_UNIT)
		{
			if (!bPlayerContext || (GC.getGameINLINE().countKnownTechNumTeams(eTech) == 0))
			{
				if (bList)
				{
					szBuffer += NEWLINE;
				}
				szBuffer += gDLL->getText("TXT_KEY_TECH_FIRST_RECEIVES", GC.getUnitInfo(eFreeUnit).getTextKeyWide());
			}
		}
	}
}

void CvGameTextMgr::buildFeatureProductionString(CvWString &szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).getFeatureProductionModifier() != 0)
	{
		if (bList)
		{
			szBuffer += NEWLINE;
		}
		szBuffer += gDLL->getText("TXT_KEY_FEATURE_PRODUCTION_MODIFIER", GC.getTechInfo(eTech).getFeatureProductionModifier());
	}
}

void CvGameTextMgr::buildWorkerRateString(CvWString &szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).getWorkerSpeedModifier() != 0)
	{
		if (bList)
		{
			szBuffer += NEWLINE;
		}
		szBuffer += gDLL->getText("TXT_KEY_UNIT_WORKERS_FASTER", GC.getTechInfo(eTech).getWorkerSpeedModifier());
	}
}

void CvGameTextMgr::buildTradeRouteString(CvWString &szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).getTradeRoutes() != 0)
	{
		if (bList)
		{
			szBuffer += NEWLINE;
		}
		szBuffer += gDLL->getText("TXT_KEY_MISC_TRADE_ROUTES", GC.getTechInfo(eTech).getTradeRoutes());
	}
}

void CvGameTextMgr::buildHealthRateString(CvWString &szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).getHealth() != 0)
	{
		if (bList)
		{
			szBuffer += NEWLINE;
		}
		szBuffer += gDLL->getText("TXT_KEY_MISC_HEALTH_ALL_CITIES", abs(GC.getTechInfo(eTech).getHealth()), ((GC.getTechInfo(eTech).getHealth() > 0) ? gDLL->getSymbolID(HEALTHY_CHAR): gDLL->getSymbolID(UNHEALTHY_CHAR)));
	}
}

void CvGameTextMgr::buildHappinessRateString(CvWString &szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).getHappiness() != 0)
	{
		if (bList)
		{
			szBuffer += NEWLINE;
		}
		szBuffer += gDLL->getText("TXT_KEY_MISC_HAPPINESS_ALL_CITIES", abs(GC.getTechInfo(eTech).getHappiness()), ((GC.getTechInfo(eTech).getHappiness() > 0) ? gDLL->getSymbolID(HAPPY_CHAR): gDLL->getSymbolID(UNHAPPY_CHAR)));
	}
}

void CvGameTextMgr::buildFreeTechString(CvWString &szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).getFirstFreeTechs() > 0)
	{
		if (!bPlayerContext || (GC.getGameINLINE().countKnownTechNumTeams(eTech) == 0))
		{
			if (bList)
			{
				szBuffer += NEWLINE;
			}
			if (GC.getTechInfo(eTech).getFirstFreeTechs() == 1)
			{
				szBuffer += gDLL->getText("TXT_KEY_TECH_FIRST_FREE_TECH");
			}
			else
			{
				szBuffer += gDLL->getText("TXT_KEY_TECH_FIRST_FREE_TECHS", GC.getTechInfo(eTech).getFirstFreeTechs());
			}
		}
	}
}

void CvGameTextMgr::buildLOSString(CvWString &szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).isExtraWaterSeeFrom() && (!bPlayerContext || !(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isExtraWaterSeeFrom())))
	{
		if (bList)
		{
			szBuffer += NEWLINE;
		}
		szBuffer += gDLL->getText("TXT_KEY_UNIT_EXTRA_SIGHT");
	}
}

void CvGameTextMgr::buildMapCenterString(CvWString &szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).isMapCentering() && (!bPlayerContext || !(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isMapCentering())))
	{
		if (bList)
		{
			szBuffer += NEWLINE;
		}
		szBuffer += gDLL->getText("TXT_KEY_MISC_CENTERS_MAP");
	}
}

void CvGameTextMgr::buildMapRevealString(CvWString &szBuffer, TechTypes eTech, bool bList)
{
	if (GC.getTechInfo(eTech).isMapVisible())
	{
		if (bList)
		{
			szBuffer += NEWLINE;
		}
		szBuffer += gDLL->getText("TXT_KEY_MISC_REVEALS_MAP");
	}
}

void CvGameTextMgr::buildMapTradeString(CvWString &szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).isMapTrading() && (!bPlayerContext || !(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isMapTrading())))
	{
		if (bList)
		{
			szBuffer += NEWLINE;
		}
		szBuffer += gDLL->getText("TXT_KEY_MISC_ENABLES_MAP_TRADING");
	}
}

void CvGameTextMgr::buildTechTradeString(CvWString &szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).isTechTrading() && (!bPlayerContext || !(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isTechTrading())))
	{
		if (bList)
		{
			szBuffer += NEWLINE;
		}
		szBuffer += gDLL->getText("TXT_KEY_MISC_ENABLES_TECH_TRADING");
	}
}

void CvGameTextMgr::buildGoldTradeString(CvWString &szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).isGoldTrading() && (!bPlayerContext || !(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isGoldTrading())))
	{
		if (bList)
		{
			szBuffer += NEWLINE;
		}
		szBuffer += gDLL->getText("TXT_KEY_MISC_ENABLES_GOLD_TRADING");
	}
}

void CvGameTextMgr::buildOpenBordersString(CvWString &szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).isOpenBordersTrading() && (!bPlayerContext || !(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isOpenBordersTrading())))
	{
		if (bList)
		{
			szBuffer += NEWLINE;
		}
		szBuffer += gDLL->getText("TXT_KEY_MISC_ENABLES_OPEN_BORDERS");
	}
}

void CvGameTextMgr::buildDefensivePactString(CvWString &szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).isDefensivePactTrading() && (!bPlayerContext || !(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isDefensivePactTrading())))
	{
		if (bList)
		{
			szBuffer += NEWLINE;
		}
		szBuffer += gDLL->getText("TXT_KEY_MISC_ENABLES_DEFENSIVE_PACTS");
	}
}

void CvGameTextMgr::buildPermanentAllianceString(CvWString &szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).isPermanentAllianceTrading() && (!bPlayerContext || (!(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isPermanentAllianceTrading()) && GC.getGameINLINE().isOption(GAMEOPTION_PERMANENT_ALLIANCES))))
	{
		if (bList)
		{
			szBuffer += NEWLINE;
		}
		szBuffer += gDLL->getText("TXT_KEY_MISC_ENABLES_PERM_ALLIANCES");
	}
}

void CvGameTextMgr::buildVassalStateString(CvWString &szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).isVassalStateTrading() && (!bPlayerContext || (!(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isVassalStateTrading()) && GC.getGameINLINE().isOption(GAMEOPTION_NO_VASSAL_STATES))))
	{
		if (bList)
		{
			szBuffer += NEWLINE;
		}
		szBuffer += gDLL->getText("TXT_KEY_MISC_ENABLES_VASSAL_STATES");
	}
}

void CvGameTextMgr::buildBridgeString(CvWString &szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).isBridgeBuilding() && (!bPlayerContext || !(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isBridgeBuilding())))
	{
		if (bList)
		{
			szBuffer += NEWLINE;
		}
		szBuffer += gDLL->getText("TXT_KEY_MISC_ENABLES_BRIDGE_BUILDING");
	}
}

void CvGameTextMgr::buildIrrigationString(CvWString &szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).isIrrigation() && (!bPlayerContext || !(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isIrrigation())))
	{
		if (bList)
		{
			szBuffer += NEWLINE;
		}
		szBuffer += gDLL->getText("TXT_KEY_MISC_SPREAD_IRRIGATION");
	}
}

void CvGameTextMgr::buildIgnoreIrrigationString(CvWString &szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).isIgnoreIrrigation() && (!bPlayerContext || !(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isIgnoreIrrigation())))
	{
		if (bList)
		{
			szBuffer += NEWLINE;
		}
		szBuffer += gDLL->getText("TXT_KEY_MISC_IRRIGATION_ANYWHERE");
	}
}

void CvGameTextMgr::buildWaterWorkString(CvWString &szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).isWaterWork() && (!bPlayerContext || !(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isWaterWork())))
	{
		if (bList)
		{
			szBuffer += NEWLINE;
		}
		szBuffer += gDLL->getText("TXT_KEY_MISC_WATER_WORK");
	}
}

void CvGameTextMgr::buildImprovementString(CvWString &szBuffer, TechTypes eTech, int iImprovement, bool bList, bool bPlayerContext)
{
	bool bTechFound;
	int iJ;

	bTechFound = false;

	if (GC.getBuildInfo((BuildTypes) iImprovement).getTechPrereq() == NO_TECH)
	{
		for (iJ = 0; iJ < GC.getNumFeatureInfos(); iJ++)
		{
			if (GC.getBuildInfo((BuildTypes) iImprovement).getFeatureTech(iJ) == eTech)
			{
				bTechFound = true;
			}
		}
	}
	else
	{
		if (GC.getBuildInfo((BuildTypes) iImprovement).getTechPrereq() == eTech)
		{
			bTechFound = true;
		}
	}

	if (bTechFound)
	{
		if (bList)
		{
			szBuffer += NEWLINE;
		}

		szBuffer += gDLL->getText("TXT_KEY_MISC_CAN_BUILD_IMPROVEMENT", GC.getBuildInfo((BuildTypes) iImprovement).getTextKeyWide());
	}
}

void CvGameTextMgr::buildDomainExtraMovesString(CvWString &szBuffer, TechTypes eTech, int iDomainType, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).getDomainExtraMoves(iDomainType) != 0)
	{
		if (bList)
		{
			szBuffer += NEWLINE;
		}

		szBuffer += gDLL->getText("TXT_KEY_MISC_EXTRA_MOVES", GC.getTechInfo(eTech).getDomainExtraMoves(iDomainType), GC.getDomainInfo((DomainTypes)iDomainType).getTextKeyWide());
	}
}

void CvGameTextMgr::buildAdjustString(CvWString &szBuffer, TechTypes eTech, int iCommerceType, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).isCommerceFlexible(iCommerceType) && (!bPlayerContext || !(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isCommerceFlexible((CommerceTypes)iCommerceType))))
	{
		if (bList)
		{
			szBuffer += NEWLINE;
		}
		szBuffer += gDLL->getText("TXT_KEY_MISC_ADJUST_COMMERCE_RATE", GC.getCommerceInfo((CommerceTypes) iCommerceType).getChar());
	}
}

void CvGameTextMgr::buildTerrainTradeString(CvWString &szBuffer, TechTypes eTech, int iTerrainType, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).isTerrainTrade(iTerrainType) && (!bPlayerContext || !(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isTerrainTrade((TerrainTypes)iTerrainType))))
	{
		if (bList)
		{
			szBuffer += NEWLINE;
		}
		szBuffer += gDLL->getText("TXT_KEY_MISC_ENABLES_ON_TERRAIN", gDLL->getSymbolID(TRADE_CHAR), GC.getTerrainInfo((TerrainTypes) iTerrainType).getTextKeyWide());
	}
}

void CvGameTextMgr::buildSpecialBuildingString(CvWString &szBuffer, TechTypes eTech, int iBuildingType, bool bList, bool bPlayerContext)
{
	if (GC.getSpecialBuildingInfo((SpecialBuildingTypes)iBuildingType).getTechPrereq() == eTech)
	{
		if (bList)
		{
			szBuffer += NEWLINE;
		}
		szBuffer += gDLL->getText("TXT_KEY_MISC_CAN_CONSTRUCT_BUILDING", GC.getSpecialBuildingInfo((SpecialBuildingTypes) iBuildingType).getTextKeyWide());
	}
}

void CvGameTextMgr::buildYieldChangeString(CvWString &szBuffer, TechTypes eTech, int iYieldType, bool bList, bool bPlayerContext)
{
	CvWString szTempBuffer;
	if (bList)
	{
		szTempBuffer.Format(L"<link=literal>%s</link>", GC.getImprovementInfo((ImprovementTypes)iYieldType).getDescription());
	}
	else
	{
		szTempBuffer.Format(L"%c<link=literal>%s</link>", gDLL->getSymbolID(BULLET_CHAR), GC.getImprovementInfo((ImprovementTypes)iYieldType).getDescription());
	}
	GAMETEXT.setYieldChangeHelp(szBuffer, szTempBuffer, L": ", L"", GC.getImprovementInfo((ImprovementTypes)iYieldType).getTechYieldChangesArray(eTech), false, bList);
}

bool CvGameTextMgr::buildBonusRevealString(CvWString &szBuffer, TechTypes eTech, int iBonusType, bool bFirst, bool bList, bool bPlayerContext)
{
	CvWString szTempBuffer;

	if (GC.getBonusInfo((BonusTypes) iBonusType).getTechReveal() == eTech)
	{
		if (bList)
		{
			szBuffer += NEWLINE;
		}
		szTempBuffer.Format( SETCOLR L"<link=literal>%s</link>" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getBonusInfo((BonusTypes) iBonusType).getDescription());
		setListHelp(szBuffer, gDLL->getText("TXT_KEY_MISC_REVEALS").c_str(), szTempBuffer, L", ", bFirst);
		bFirst = false;
	}
	return bFirst;
}

bool CvGameTextMgr::buildCivicRevealString(CvWString &szBuffer, TechTypes eTech, int iCivicType, bool bFirst, bool bList, bool bPlayerContext)
{
	CvWString szTempBuffer;

	if (GC.getCivicInfo((CivicTypes) iCivicType).getTechPrereq() == eTech)
	{
		if (bList && bFirst)
		{
			szBuffer += NEWLINE;
		}
		szTempBuffer.Format( SETCOLR L"<link=literal>%s</link>" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getCivicInfo((CivicTypes) iCivicType).getDescription());
		setListHelp(szBuffer, gDLL->getText("TXT_KEY_MISC_ENABLES").c_str(), szTempBuffer, L", ", bFirst);
		bFirst = false;
	}
	return bFirst;
}

bool CvGameTextMgr::buildProcessInfoString(CvWString &szBuffer, TechTypes eTech, int iProcessType, bool bFirst, bool bList, bool bPlayerContext)
{
	CvWString szTempBuffer;

	if (GC.getProcessInfo((ProcessTypes) iProcessType).getTechPrereq() == eTech)
	{
		if (bList && bFirst)
		{
			szBuffer += NEWLINE;
		}
		szTempBuffer.Format( SETCOLR L"<link=literal>%s</link>" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getProcessInfo((ProcessTypes) iProcessType).getDescription());
		setListHelp(szBuffer, gDLL->getText("TXT_KEY_MISC_CAN_BUILD").c_str(), szTempBuffer, L", ", bFirst);
		bFirst = false;
	}
	return bFirst;
}

bool CvGameTextMgr::buildFoundReligionString(CvWString &szBuffer, TechTypes eTech, int iReligionType, bool bFirst, bool bList, bool bPlayerContext)
{
	CvWString szTempBuffer;

	if (GC.getReligionInfo((ReligionTypes) iReligionType).getTechPrereq() == eTech)
	{
		if (!bPlayerContext || (GC.getGameINLINE().countKnownTechNumTeams(eTech) == 0))
		{
			if (bList && bFirst)
			{
				szBuffer += NEWLINE;
			}
			szTempBuffer.Format( SETCOLR L"<link=literal>%s</link>" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getReligionInfo((ReligionTypes) iReligionType).getDescription());
			setListHelp(szBuffer, gDLL->getText("TXT_KEY_MISC_FIRST_DISCOVER_FOUNDS").c_str(), szTempBuffer, L", ", bFirst);
			bFirst = false;
		}
	}
	return bFirst;
}

bool CvGameTextMgr::buildPromotionString(CvWString &szBuffer, TechTypes eTech, int iPromotionType, bool bFirst, bool bList, bool bPlayerContext)
{
	CvWString szTempBuffer;

	if (GC.getPromotionInfo((PromotionTypes) iPromotionType).getTechPrereq() == eTech)
	{
		if (bList && bFirst)
		{
			szBuffer += NEWLINE;
		}
		szTempBuffer.Format( SETCOLR L"<link=literal>%s</link>" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getPromotionInfo((PromotionTypes) iPromotionType).getDescription());
		setListHelp(szBuffer, gDLL->getText("TXT_KEY_MISC_ENABLES").c_str(), szTempBuffer, L", ", bFirst);
		bFirst = false;
	}
	return bFirst;
}

// Displays a list of derived technologies - no distinction between AND/OR prerequisites
void CvGameTextMgr::buildSingleLineTechTreeString(CvWString &szBuffer, TechTypes eTech, bool bPlayerContext)
{
	CvWString szTempBuffer;	// Formatting

	if (NO_TECH == eTech)
	{
		// you need to specify a tech of origin for this method to do anything
		return;
	}

	bool bFirst = true;
	for (int iI = 0; iI < GC.getNumTechInfos(); iI++)
	{
		bool bTechAlreadyAccessible = false;
		if (bPlayerContext)
		{
			bTechAlreadyAccessible = (GET_TEAM(GC.getGameINLINE().getActiveTeam()).isHasTech((TechTypes)iI) || GET_PLAYER(GC.getGameINLINE().getActivePlayer()).canResearch((TechTypes)iI));
		}
		if (!bTechAlreadyAccessible)
		{
			bool bTechFound = false;

			if (!bTechFound)
			{
				for (int iJ = 0; iJ < GC.getDefineINT("NUM_OR_TECH_PREREQS"); iJ++)
				{
					if (GC.getTechInfo((TechTypes) iI).getPrereqOrTechs(iJ) == eTech)
					{
						bTechFound = true;
						break;
					}
				}
			}

			if (!bTechFound)
			{
				for (int iJ = 0; iJ < GC.getDefineINT("NUM_AND_TECH_PREREQS"); iJ++)
				{
					if (GC.getTechInfo((TechTypes) iI).getPrereqAndTechs(iJ) == eTech)
					{
						bTechFound = true;
						break;
					}
				}
			}

			if (bTechFound)
			{
				szTempBuffer.Format( SETCOLR L"<link=literal>%s</link>" ENDCOLR , TEXT_COLOR("COLOR_TECH_TEXT"), GC.getTechInfo((TechTypes) iI).getDescription());
				setListHelp(szBuffer, gDLL->getText("TXT_KEY_MISC_LEADS_TO").c_str(), szTempBuffer, L", ", bFirst);
				bFirst = false;
			}
		}
	}
}

// Information about other prerequisite technologies to eTech besides eFromTech
void CvGameTextMgr::buildTechTreeString(CvWString &szBuffer, TechTypes eTech, bool bPlayerContext, TechTypes eFromTech)
{
	CvWString szTempBuffer;	// Formatting

	if (NO_TECH == eTech || NO_TECH == eFromTech)
	{
		return;
	}

	szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_TECH_TEXT"), GC.getTechInfo(eTech).getDescription());
	szBuffer += szTempBuffer;

	// Loop through OR prerequisites to make list
	CvWString szOtherOrTechs;
	int nOtherOrTechs = 0;
	bool bOrTechFound = false;
	for (int iJ = 0; iJ < GC.getDefineINT("NUM_OR_TECH_PREREQS"); iJ++)
	{
		TechTypes eTestTech = (TechTypes)GC.getTechInfo(eTech).getPrereqOrTechs(iJ);
		if (eTestTech >= 0)
		{
			bool bTechAlreadyResearched = false;
			if (bPlayerContext)
			{
				bTechAlreadyResearched = GET_TEAM(GC.getGameINLINE().getActiveTeam()).isHasTech(eTestTech);
			}
			if (!bTechAlreadyResearched)
			{
				if (eTestTech == eFromTech)
				{
					bOrTechFound = true;
				}
				else
				{
					szTempBuffer.Format( SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_TECH_TEXT"), GC.getTechInfo(eTestTech).getDescription());
					setListHelp(szOtherOrTechs, L"", szTempBuffer, gDLL->getText("TXT_KEY_OR").c_str(), 0 == nOtherOrTechs);
					nOtherOrTechs++;
				}
			}
		}
	}

	// Loop through AND prerequisites to make list
	CvWString szOtherAndTechs;
	int nOtherAndTechs = 0;
	bool bAndTechFound = false;
	for (int iJ = 0; iJ < GC.getDefineINT("NUM_AND_TECH_PREREQS"); iJ++)
	{
		TechTypes eTestTech = (TechTypes)GC.getTechInfo(eTech).getPrereqAndTechs(iJ);
		if (eTestTech >= 0)
		{
			bool bTechAlreadyResearched = false;
			if (bPlayerContext)
			{
				bTechAlreadyResearched = GET_TEAM(GC.getGameINLINE().getActiveTeam()).isHasTech(eTestTech);
			}
			if (!bTechAlreadyResearched)
			{
				if (eTestTech == eFromTech)
				{
					bAndTechFound = true;
				}
				else
				{
					szTempBuffer.Format( SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_TECH_TEXT"), GC.getTechInfo(eTestTech).getDescription());
					setListHelp(szOtherAndTechs, L"", szTempBuffer, L", ", 0 == nOtherAndTechs);
					nOtherAndTechs++;
				}
			}
		}
	}

	if (bOrTechFound || bAndTechFound)
	{
		if (nOtherAndTechs > 0 || nOtherOrTechs > 0)
		{
			szBuffer += L" ";

			if (nOtherAndTechs > 0)
			{
				szBuffer += gDLL->getText("TXT_KEY_WITH_SPACE");
				szBuffer += szOtherAndTechs;
			}

			if (nOtherOrTechs > 0)
			{
				if (bAndTechFound)
				{
					if (nOtherAndTechs > 0)
					{
						szBuffer += gDLL->getText("TXT_KEY_AND_SPACE");
					}
					else
					{
						szBuffer += gDLL->getText("TXT_KEY_WITH_SPACE");
					}
					szBuffer += szOtherOrTechs;
				}
				else if (bOrTechFound)
				{
					szBuffer += NEWLINE + gDLL->getText("TXT_KEY_MISC_ALTERNATIVELY_DERIVED", GC.getTechInfo(eTech).getTextKeyWide(), szOtherOrTechs.GetCString());
				}
			}
		}
	}
}

void CvGameTextMgr::setPromotionHelp(CvWString &szBuffer, PromotionTypes ePromotion, bool bCivilopediaText)
{
	if (!bCivilopediaText)
	{
		CvWString szTempBuffer;

		if (NO_PROMOTION == ePromotion)
		{
			return;
		}
		CvPromotionInfo& promo = GC.getPromotionInfo(ePromotion);

		szTempBuffer.Format( SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), promo.getDescription());
		szBuffer += szTempBuffer;
	}

	parsePromotionHelp(szBuffer, ePromotion);
}

void CvGameTextMgr::setUnitCombatHelp(CvWString &szBuffer, UnitCombatTypes eUnitCombat)
{
	szBuffer += GC.getUnitCombatInfo(eUnitCombat).getDescription();
}

void CvGameTextMgr::setImprovementHelp(CvWString &szBuffer, ImprovementTypes eImprovement, bool bCivilopediaText)
{
	CvWString szTempBuffer;
	CvWString szFirstBuffer;
	int iTurns;

	if (NO_IMPROVEMENT == eImprovement)
	{
		return;
	}

	CvImprovementInfo& info = GC.getImprovementInfo(eImprovement);
	if (!bCivilopediaText)
	{
		szTempBuffer.Format( SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), info.getDescription());
		szBuffer += szTempBuffer;

		setYieldChangeHelp(szBuffer, L", ", L"", L"", info.getYieldChangeArray(), false, false);

		setYieldChangeHelp(szBuffer, L"", L"", gDLL->getText("TXT_KEY_MISC_WITH_IRRIGATION").c_str(), info.getIrrigatedYieldChangeArray());
		setYieldChangeHelp(szBuffer, L"", L"", gDLL->getText("TXT_KEY_MISC_ON_HILLS").c_str(), info.getHillsYieldChangeArray());
		setYieldChangeHelp(szBuffer, L"", L"", gDLL->getText("TXT_KEY_MISC_ALONG_RIVER").c_str(), info.getRiverSideYieldChangeArray());

		for (int iTech = 0; iTech < GC.getNumTechInfos(); iTech++)
		{
			for (int iYield = 0; iYield < NUM_YIELD_TYPES; iYield++)
			{
				if (0 != info.getTechYieldChanges(iTech, iYield))
				{
					szBuffer += NEWLINE + gDLL->getText("TXT_KEY_IMPROVEMENT_WITH_TECH", info.getTechYieldChanges(iTech, iYield), GC.getYieldInfo((YieldTypes)iYield).getChar(), GC.getTechInfo((TechTypes)iTech).getTextKeyWide());
				}
			}
		}

		//	Civics
		for (int iYield = 0; iYield < NUM_YIELD_TYPES; iYield++)
		{
			for (int iCivic = 0; iCivic < GC.getNumCivicInfos(); iCivic++)
			{
				int iChange = GC.getCivicInfo((CivicTypes)iCivic).getImprovementYieldChanges(eImprovement, iYield);
				if (0 != iChange)
				{
					szTempBuffer.Format( SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getCivicInfo((CivicTypes)iCivic).getDescription());
					szBuffer += NEWLINE + gDLL->getText("TXT_KEY_CIVIC_IMPROVEMENT_YIELD_CHANGE", iChange, GC.getYieldInfo((YieldTypes)iYield).getChar()) + szTempBuffer;
				}
			}
		}
	}

	if (info.isRequiresRiverSide())
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_IMPROVEMENT_REQUIRES_RIVER");
	}
	if (info.isCarriesIrrigation())
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_IMPROVEMENT_CARRIES_IRRIGATION");
	}
	if (bCivilopediaText)
	{
		if (info.isNoFreshWater())
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_IMPROVEMENT_NO_BUILD_FRESH_WATER");
		}
		if (info.isWater())
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_IMPROVEMENT_BUILD_ONLY_WATER");
		}
		if (info.isRequiresFlatlands())
		{
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_IMPROVEMENT_ONLY_BUILD_FLATLANDS");
		}
	}

	if (info.getImprovementUpgrade() != NO_IMPROVEMENT)
	{
		iTurns = GC.getGameINLINE().getImprovementUpgradeTime(eImprovement);

		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_IMPROVEMENT_EVOLVES", GC.getImprovementInfo((ImprovementTypes) info.getImprovementUpgrade()).getTextKeyWide(), iTurns);
	}

	int iLast = -1;
	for (int iBonus = 0; iBonus < GC.getNumBonusInfos(); iBonus++)
	{
		int iRand = info.getImprovementBonusDiscoverRand(iBonus);
		if (iRand > 0)
		{
			szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_IMPROVEMENT_CHANCE_DISCOVER").c_str());
			szTempBuffer.Format(L"%c", GC.getBonusInfo((BonusTypes) iBonus).getChar());
			setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", iRand != iLast);
			iLast = iRand;
		}
	}

	if (info.getDefenseModifier())
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_IMPROVEMENT_DEFENSE_MODIFIER", info.getDefenseModifier());
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_IMPROVEMENT_DEFENSE_MODIFIER_EXTRA");
	}

	if (bCivilopediaText)
	{
		if (info.getPillageGold() > 0)
		{
			//		szTempBuffer.Format(L"\n%cPillage yields %d%c on average", gDLL->getSymbolID(BULLET_CHAR), info.getPillageGold(), GC.getYieldInfo(YIELD_COMMERCE).getChar());
			szBuffer += NEWLINE + gDLL->getText("TXT_KEY_IMPROVEMENT_PILLAGE_YIELDS", info.getPillageGold());
		}
	}
}


void CvGameTextMgr::getDealString(CvWString& szBuffer, CvDeal& deal, PlayerTypes ePlayerPerspective)
{
	PlayerTypes ePlayer1 = deal.getFirstPlayer();
	PlayerTypes ePlayer2 = deal.getSecondPlayer();
	if (NO_PLAYER == ePlayer1 || NO_PLAYER == ePlayer2)
	{
		FAssertMsg(false, "Deal needs two parties");
		return;
	}

	CvWString szDealOne;
	const CLinkList<TradeData>* pTrades = deal.getFirstTrades();
	if (NULL != pTrades && pTrades->getLength() > 0)
	{
		CLLNode<TradeData>* pTradeNode;
		bool bFirst = true;
		for (pTradeNode = pTrades->head(); pTradeNode; pTradeNode = pTrades->next(pTradeNode))
		{
			CvWString szTrade;
			getTradeString(szTrade, pTradeNode->m_data, deal.getFirstPlayer(), deal.getSecondPlayer());
			setListHelp(szDealOne, L"", szTrade, L", ", bFirst);
			bFirst = false;
		}
	}

	CvWString szDealTwo;
	pTrades = deal.getSecondTrades();
	if (NULL != pTrades && pTrades->getLength() > 0)
	{
		CLLNode<TradeData>* pTradeNode;
		bool bFirst = true;
		for (pTradeNode = pTrades->head(); pTradeNode; pTradeNode = pTrades->next(pTradeNode))
		{
			CvWString szTrade;
			getTradeString(szTrade, pTradeNode->m_data, deal.getSecondPlayer(), deal.getFirstPlayer());
			setListHelp(szDealTwo, L"", szTrade, L", ", bFirst);
			bFirst = false;
		}
	}

	if (!szDealOne.empty())
	{
		if (!szDealTwo.empty())
		{
			if (ePlayerPerspective == ePlayer1)
			{
				szBuffer += gDLL->getText("TXT_KEY_MISC_OUR_DEAL", szDealOne.GetCString(), GET_PLAYER(ePlayer2).getNameKey(), szDealTwo.GetCString());
			}
			else if (ePlayerPerspective == ePlayer2)
			{
				szBuffer += gDLL->getText("TXT_KEY_MISC_OUR_DEAL", szDealTwo.GetCString(), GET_PLAYER(ePlayer1).getNameKey(), szDealOne.GetCString());
			}
			else
			{
				szBuffer += gDLL->getText("TXT_KEY_MISC_DEAL", GET_PLAYER(ePlayer1).getNameKey(), szDealOne.GetCString(), GET_PLAYER(ePlayer2).getNameKey(), szDealTwo.GetCString());
			}
		}
		else
		{
			if (ePlayerPerspective == ePlayer1)
			{
				szBuffer += gDLL->getText("TXT_KEY_MISC_DEAL_ONESIDED_OURS", szDealOne.GetCString(), GET_PLAYER(ePlayer2).getNameKey());
			}
			else if (ePlayerPerspective == ePlayer2)
			{
				szBuffer += gDLL->getText("TXT_KEY_MISC_DEAL_ONESIDED_THEIRS", szDealOne.GetCString(), GET_PLAYER(ePlayer1).getNameKey());
			}
			else
			{
				szBuffer += gDLL->getText("TXT_KEY_MISC_DEAL_ONESIDED", GET_PLAYER(ePlayer1).getNameKey(), szDealOne.GetCString(), GET_PLAYER(ePlayer2).getNameKey());
			}
		}
	}
	else if (!szDealTwo.empty())
	{
		if (ePlayerPerspective == ePlayer1)
		{
			szBuffer += gDLL->getText("TXT_KEY_MISC_DEAL_ONESIDED_THEIRS", szDealTwo.GetCString(), GET_PLAYER(ePlayer2).getNameKey());
		}
		else if (ePlayerPerspective == ePlayer2)
		{
			szBuffer += gDLL->getText("TXT_KEY_MISC_DEAL_ONESIDED_OURS", szDealTwo.GetCString(), GET_PLAYER(ePlayer1).getNameKey());
		}
		else
		{
			szBuffer += gDLL->getText("TXT_KEY_MISC_DEAL_ONESIDED", GET_PLAYER(ePlayer2).getNameKey(), szDealTwo.GetCString(), GET_PLAYER(ePlayer1).getNameKey());
		}
	}
}

void CvGameTextMgr::getAttitudeString(CvWString& szBuffer, PlayerTypes ePlayer, PlayerTypes eTargetPlayer)
{
	CvWString szTempBuffer;
	int iAttitudeChange;
	int iPass;
	int iI;
	CvTeam& kTeam = GET_TEAM(GET_PLAYER(ePlayer).getTeam());

	szBuffer += gDLL->getText("TXT_KEY_ATTITUDE_TOWARDS", GC.getAttitudeInfo(GET_PLAYER(ePlayer).AI_getAttitude(eTargetPlayer)).getTextKeyWide(), GET_PLAYER(eTargetPlayer).getNameKey());

	for (int iTeam = 0; iTeam < MAX_TEAMS; iTeam++)
	{
		CvTeam& kLoopTeam = GET_TEAM((TeamTypes)iTeam);
		if (kLoopTeam.isAlive())
		{
			if (NO_PLAYER != eTargetPlayer)
			{
				CvTeam& kTargetTeam = GET_TEAM(GET_PLAYER(eTargetPlayer).getTeam());
				if (kTargetTeam.isHasMet((TeamTypes)iTeam))
				{
					if (kTeam.isVassal((TeamTypes)iTeam))
					{
						szBuffer += NEWLINE + gDLL->getText("TXT_KEY_ATTITUDE_VASSAL_OF", kLoopTeam.getName().GetCString());
						setVassalRevoltHelp(szBuffer, (TeamTypes)iTeam, kTeam.getID());
					}
					else if (kLoopTeam.isVassal(kTeam.getID()))
					{
						szBuffer += NEWLINE + gDLL->getText("TXT_KEY_ATTITUDE_MASTER_OF", kLoopTeam.getName().GetCString());
						//setVassalRevoltHelp(szBuffer, kTeam.getID(), kLoopTeam.getID());
					}
				}
			}
		}
	}

	for (iPass = 0; iPass < 2; iPass++)
	{
		iAttitudeChange = GET_PLAYER(ePlayer).AI_getCloseBordersAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_LAND_TARGET", iAttitudeChange).GetCString());
			szBuffer += NEWLINE + szTempBuffer;
		}

		iAttitudeChange = GET_PLAYER(ePlayer).AI_getWarAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_WAR", iAttitudeChange).GetCString());
			szBuffer += NEWLINE + szTempBuffer;
		}

		iAttitudeChange = GET_PLAYER(ePlayer).AI_getPeaceAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_PEACE", iAttitudeChange).GetCString());
			szBuffer += NEWLINE + szTempBuffer;
		}

		iAttitudeChange = GET_PLAYER(ePlayer).AI_getSameReligionAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_SAME_RELIGION", iAttitudeChange).GetCString());
			szBuffer += NEWLINE + szTempBuffer;
		}

		iAttitudeChange = GET_PLAYER(ePlayer).AI_getDifferentReligionAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_DIFFERENT_RELIGION", iAttitudeChange).GetCString());
			szBuffer += NEWLINE + szTempBuffer;
		}

		iAttitudeChange = GET_PLAYER(ePlayer).AI_getBonusTradeAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_BONUS_TRADE", iAttitudeChange).GetCString());
			szBuffer += NEWLINE + szTempBuffer;
		}

		iAttitudeChange = GET_PLAYER(ePlayer).AI_getOpenBordersAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_OPEN_BORDERS", iAttitudeChange).GetCString());
			szBuffer += NEWLINE + szTempBuffer;
		}

		iAttitudeChange = GET_PLAYER(ePlayer).AI_getDefensivePactAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_DEFENSIVE_PACT", iAttitudeChange).GetCString());
			szBuffer += NEWLINE + szTempBuffer;
		}

		iAttitudeChange = GET_PLAYER(ePlayer).AI_getRivalDefensivePactAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_RIVAL_DEFENSIVE_PACT", iAttitudeChange).GetCString());
			szBuffer += NEWLINE + szTempBuffer;
		}

		iAttitudeChange = GET_PLAYER(ePlayer).AI_getRivalVassalAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_RIVAL_VASSAL", iAttitudeChange).GetCString());
			szBuffer += NEWLINE + szTempBuffer;
		}

		iAttitudeChange = GET_PLAYER(ePlayer).AI_getShareWarAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_SHARE_WAR", iAttitudeChange).GetCString());
			szBuffer += NEWLINE + szTempBuffer;
		}

		iAttitudeChange = GET_PLAYER(ePlayer).AI_getFavoriteCivicAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_FAVORITE_CIVIC", iAttitudeChange).GetCString());
			szBuffer += NEWLINE + szTempBuffer;
		}

		iAttitudeChange = GET_PLAYER(ePlayer).AI_getTradeAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_TRADE", iAttitudeChange).GetCString());
			szBuffer += NEWLINE + szTempBuffer;
		}

		iAttitudeChange = GET_PLAYER(ePlayer).AI_getRivalTradeAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_RIVAL_TRADE", iAttitudeChange).GetCString());
			szBuffer += NEWLINE + szTempBuffer;
		}

		for (iI = 0; iI < NUM_MEMORY_TYPES; iI++)
		{
			iAttitudeChange = GET_PLAYER(ePlayer).AI_getMemoryAttitude(eTargetPlayer, ((MemoryTypes)iI));
			if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
			{
				szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_MEMORY", iAttitudeChange, GC.getMemoryInfo((MemoryTypes)iI).getDescription()).GetCString());
				szBuffer += NEWLINE + szTempBuffer;
			}
		}
	}
}

void CvGameTextMgr::getTradeString(CvWString& szBuffer, const TradeData& tradeData, PlayerTypes ePlayer1, PlayerTypes ePlayer2)
{
	switch (tradeData.m_eItemType)
	{
	case TRADE_GOLD:
		szBuffer += gDLL->getText("TXT_KEY_MISC_GOLD", tradeData.m_iData);
		break;
	case TRADE_GOLD_PER_TURN:
		szBuffer += gDLL->getText("TXT_KEY_MISC_GOLD_PER_TURN", tradeData.m_iData);
		break;
	case TRADE_MAPS:
		szBuffer += gDLL->getText("TXT_KEY_MISC_WORLD_MAP");
		break;
	case TRADE_SURRENDER:
		szBuffer += gDLL->getText("TXT_KEY_MISC_CAPITULATE");
		break;
	case TRADE_VASSAL:
		szBuffer += gDLL->getText("TXT_KEY_MISC_VASSAL");
		break;
	case TRADE_OPEN_BORDERS:
		szBuffer += gDLL->getText("TXT_KEY_MISC_OPEN_BORDERS");
		break;
	case TRADE_DEFENSIVE_PACT:
		szBuffer += gDLL->getText("TXT_KEY_MISC_DEFENSIVE_PACT");
		break;
	case TRADE_PERMANENT_ALLIANCE:
		szBuffer += gDLL->getText("TXT_KEY_MISC_PERMANENT_ALLIANCE");
		break;
	case TRADE_PEACE_TREATY:
		szBuffer += gDLL->getText("TXT_KEY_MISC_PEACE_TREATY", GC.getDefineINT("PEACE_TREATY_LENGTH"));
		break;
	case TRADE_TECHNOLOGIES:
		szBuffer.Format(L"%s", GC.getTechInfo((TechTypes)tradeData.m_iData).getDescription());
		break;
	case TRADE_RESOURCES:
		szBuffer.Format(L"%s", GC.getBonusInfo((BonusTypes)tradeData.m_iData).getDescription());
		break;
	case TRADE_CITIES:
		szBuffer.Format(L"%s", GET_PLAYER(ePlayer1).getCity(tradeData.m_iData)->getName());
		break;
	case TRADE_PEACE:
	case TRADE_WAR:
	case TRADE_EMBARGO:
		szBuffer.Format(L"%s", GET_TEAM((TeamTypes)tradeData.m_iData).getName().GetCString());
		break;
	case TRADE_CIVIC:
		szBuffer.Format(L"%s", GC.getCivicInfo((CivicTypes)tradeData.m_iData).getDescription());
		break;
	case TRADE_RELIGION:
		szBuffer.Format(L"%s", GC.getReligionInfo((ReligionTypes)tradeData.m_iData).getDescription());
		break;
	default:
		FAssert(false);
		break;
	}
}

void CvGameTextMgr::setFeatureHelp(CvWString &szBuffer, FeatureTypes eFeature, bool bCivilopediaText)
{
	if (NO_FEATURE == eFeature)
	{
		return;
	}
	CvFeatureInfo& feature = GC.getFeatureInfo(eFeature);

	int aiYields[NUM_YIELD_TYPES];
	if (!bCivilopediaText)
	{
		szBuffer += feature.getDescription();

		for (int iI = 0; iI < NUM_YIELD_TYPES; iI++)
		{
			aiYields[iI] = feature.getYieldChange(iI);
		}
		setYieldChangeHelp(szBuffer, L"", L"", L"", aiYields);
	}
	for (int iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		aiYields[iI] = feature.getRiverYieldChange(iI);
	}
	setYieldChangeHelp(szBuffer, L"", L"", gDLL->getText("TXT_KEY_TERRAIN_NEXT_TO_RIVER"), aiYields);

	for (int iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		aiYields[iI] = feature.getHillsYieldChange(iI);
	}
	setYieldChangeHelp(szBuffer, L"", L"", gDLL->getText("TXT_KEY_TERRAIN_ON_HILLS"), aiYields);

	if (feature.getMovementCost() != 1)
	{
		szBuffer += gDLL->getText("TXT_KEY_TERRAIN_MOVEMENT_COST", feature.getMovementCost());
	}

	CvWString szHealth;
	szHealth.Format(L"%.2f", 0.01f * abs(feature.getHealthPercent()));
	if (feature.getHealthPercent() > 0)
	{
		szBuffer += gDLL->getText("TXT_KEY_FEATURE_GOOD_HEALTH", szHealth.GetCString());
	}
	else if (feature.getHealthPercent() < 0)
	{
		szBuffer += gDLL->getText("TXT_KEY_FEATURE_BAD_HEALTH", szHealth.GetCString());
	}

	if (feature.getDefenseModifier() != 0)
	{
		szBuffer += gDLL->getText("TXT_KEY_TERRAIN_DEFENSE_MODIFIER", feature.getDefenseModifier());
	}

	if (feature.isAddsFreshWater())
	{
		szBuffer += gDLL->getText("TXT_KEY_FEATURE_ADDS_FRESH_WATER");
	}

	if (feature.isImpassable())
	{
		szBuffer += gDLL->getText("TXT_KEY_TERRAIN_IMPASSABLE");
	}

	if (feature.isNoCity())
	{
		szBuffer += gDLL->getText("TXT_KEY_TERRAIN_NO_CITIES");
	}

	if (feature.isNoImprovement())
	{
		szBuffer += gDLL->getText("TXT_KEY_FEATURE_NO_IMPROVEMENT");
	}

}


void CvGameTextMgr::setTerrainHelp(CvWString &szBuffer, TerrainTypes eTerrain, bool bCivilopediaText)
{
	if (NO_TERRAIN == eTerrain)
	{
		return;
	}
	CvTerrainInfo& terrain = GC.getTerrainInfo(eTerrain);

	int aiYields[NUM_YIELD_TYPES];
	if (!bCivilopediaText)
	{
		szBuffer += terrain.getDescription();

		for (int iI = 0; iI < NUM_YIELD_TYPES; iI++)
		{
			aiYields[iI] = terrain.getYield(iI);
		}
		setYieldChangeHelp(szBuffer, L"", L"", L"", aiYields);
	}
	for (int iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		aiYields[iI] = terrain.getRiverYieldChange(iI);
	}
	setYieldChangeHelp(szBuffer, L"", L"", gDLL->getText("TXT_KEY_TERRAIN_NEXT_TO_RIVER"), aiYields);

	for (int iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		aiYields[iI] = terrain.getHillsYieldChange(iI);
	}
	setYieldChangeHelp(szBuffer, L"", L"", gDLL->getText("TXT_KEY_TERRAIN_ON_HILLS"), aiYields);

	if (terrain.getMovementCost() != 1)
	{
		szBuffer += gDLL->getText("TXT_KEY_TERRAIN_MOVEMENT_COST", terrain.getMovementCost());
	}

	if (terrain.getBuildModifier() != 0)
	{
		szBuffer += gDLL->getText("TXT_KEY_TERRAIN_BUILD_MODIFIER", terrain.getBuildModifier());
	}

	if (terrain.getDefenseModifier() != 0)
	{
		szBuffer += gDLL->getText("TXT_KEY_TERRAIN_DEFENSE_MODIFIER", terrain.getDefenseModifier());
	}

	if (terrain.isImpassable())
	{
		szBuffer += gDLL->getText("TXT_KEY_TERRAIN_IMPASSABLE");
	}
	if (!terrain.isFound())
	{
		szBuffer += gDLL->getText("TXT_KEY_TERRAIN_NO_CITIES");
		bool bFirst = true;
		if (terrain.isFoundCoast())
		{
			szBuffer += gDLL->getText("TXT_KEY_TERRAIN_COASTAL_CITIES");
			bFirst = false;
		}
		if (!bFirst)
		{
			szBuffer += gDLL->getText("TXT_KEY_OR");
		}
		if (terrain.isFoundFreshWater())
		{
			szBuffer += gDLL->getText("TXT_KEY_TERRAIN_FRESH_WATER_CITIES");
			bFirst = false;
		}
	}
}

void CvGameTextMgr::buildFinanceInflationString(CvWString& szBuffer, PlayerTypes ePlayer)
{
	if (NO_PLAYER == ePlayer)
	{
		return;
	}
	CvPlayer& player = GET_PLAYER(ePlayer);

	int iInflationRate = player.calculateInflationRate();
	if (iInflationRate != 0)
	{
		int iPreInflation = player.calculatePreInflatedCosts();
		szBuffer +=  NEWLINE + gDLL->getText("TXT_KEY_FINANCE_ADVISOR_INFLATION", iPreInflation, iInflationRate, iInflationRate, iPreInflation, (iPreInflation * iInflationRate) / 100);
	}
}

void CvGameTextMgr::buildFinanceUnitCostString(CvWString& szBuffer, PlayerTypes ePlayer)
{
	if (NO_PLAYER == ePlayer)
	{
		return;
	}
	CvPlayer& player = GET_PLAYER(ePlayer);

	int iFreeUnits = 0;
	int iFreeMilitaryUnits = 0;
	int iUnits = player.getNumUnits();
	int iMilitaryUnits = player.getNumMilitaryUnits();
	int iPaidUnits = iUnits;
	int iPaidMilitaryUnits = iMilitaryUnits;
	int iMilitaryCost = 0;
	int iBaseUnitCost = 0;
	int iExtraCost = 0;
	int iCost = player.calculateUnitCost(iFreeUnits, iFreeMilitaryUnits, iPaidUnits, iPaidMilitaryUnits, iBaseUnitCost, iMilitaryCost, iExtraCost);
	int iHandicap = iCost-iBaseUnitCost-iMilitaryCost-iExtraCost;

	szBuffer += NEWLINE + gDLL->getText("TXT_KEY_FINANCE_ADVISOR_UNIT_COST", iPaidUnits, iFreeUnits, iBaseUnitCost);

	if (iPaidMilitaryUnits != 0)
	{
		szBuffer += gDLL->getText("TXT_KEY_FINANCE_ADVISOR_UNIT_COST_2", iPaidMilitaryUnits, iFreeMilitaryUnits, iMilitaryCost);
	}
	if (iExtraCost != 0)
	{
		szBuffer += gDLL->getText("TXT_KEY_FINANCE_ADVISOR_UNIT_COST_3", iExtraCost);
	}
	if (iHandicap != 0)
	{
		szBuffer += gDLL->getText("TXT_KEY_FINANCE_ADVISOR_HANDICAP_COST", iHandicap);
	}
	szBuffer += gDLL->getText("TXT_KEY_FINANCE_ADVISOR_UNIT_COST_4", iCost);
}

void CvGameTextMgr::buildFinanceAwaySupplyString(CvWString& szBuffer, PlayerTypes ePlayer)
{
	if (NO_PLAYER == ePlayer)
	{
		return;
	}
	CvPlayer& player = GET_PLAYER(ePlayer);

	int iPaidUnits = 0;
	int iBaseCost = 0;
	int iCost = player.calculateUnitSupply(iPaidUnits, iBaseCost);
	int iHandicap = iCost - iBaseCost;

	CvWString szHandicap;
	if (iHandicap != 0)
	{
		szHandicap = gDLL->getText("TXT_KEY_FINANCE_ADVISOR_HANDICAP_COST", iHandicap);
	}

	szBuffer += NEWLINE + gDLL->getText("TXT_KEY_FINANCE_ADVISOR_SUPPLY_COST", iPaidUnits, GC.getDefineINT("INITIAL_FREE_OUTSIDE_UNITS"), iBaseCost, szHandicap.GetCString(), iCost);
}

void CvGameTextMgr::buildFinanceCityMaintString(CvWString& szBuffer, PlayerTypes ePlayer)
{
	if (NO_PLAYER == ePlayer)
	{
		return;
	}
	int iLoop;
	int iDistanceMaint = 0;
	CvPlayer& player = GET_PLAYER(ePlayer);
	for (CvCity* pLoopCity = player.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = player.nextCity(&iLoop))
	{
		iDistanceMaint += (pLoopCity->calculateDistanceMaintenanceTimes100() * max(0, (pLoopCity->getMaintenanceModifier() + 100))) / 100;
	}
	iDistanceMaint /= 100;

	int iNumCityMaint = player.getTotalMaintenance() - iDistanceMaint;

	szBuffer += NEWLINE + gDLL->getText("TXT_KEY_FINANCE_ADVISOR_CITY_MAINT_COST", iDistanceMaint, iNumCityMaint, player.getTotalMaintenance());
}

void CvGameTextMgr::buildFinanceCivicUpkeepString(CvWString& szBuffer, PlayerTypes ePlayer)
{
	if (NO_PLAYER == ePlayer)
	{
		return;
	}
	CvPlayer& player = GET_PLAYER(ePlayer);
	CvWString szCivicOptionCosts;
	for (int iI = 0; iI < GC.getNumCivicOptionInfos(); iI++)
	{
		CivicTypes eCivic = player.getCivics((CivicOptionTypes)iI);
		if (NO_CIVIC != eCivic)
		{
			CvWString szTemp;
			szTemp.Format(L"%d%c: %s", player.getSingleCivicUpkeep(eCivic), GC.getCommerceInfo(COMMERCE_GOLD).getChar(),  GC.getCivicInfo(eCivic).getDescription());
			szCivicOptionCosts += NEWLINE + szTemp;
		}
	}
	szBuffer += NEWLINE + gDLL->getText("TXT_KEY_FINANCE_ADVISOR_CIVIC_UPKEEP_COST", szCivicOptionCosts.GetCString(), player.getCivicUpkeep());
}

void CvGameTextMgr::buildFinanceForeignIncomeString(CvWString& szBuffer, PlayerTypes ePlayer)
{
	if (NO_PLAYER == ePlayer)
	{
		return;
	}
	CvPlayer& player = GET_PLAYER(ePlayer);

	CvWString szPlayerIncome;
	for (int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		CvPlayer& otherPlayer = GET_PLAYER((PlayerTypes)iI);
		if (otherPlayer.isAlive() && player.getGoldPerTurnByPlayer((PlayerTypes)iI) != 0)
		{
			CvWString szTemp;
			szTemp.Format(L"%d%c: %s", player.getGoldPerTurnByPlayer((PlayerTypes)iI), GC.getCommerceInfo(COMMERCE_GOLD).getChar(), otherPlayer.getCivilizationShortDescription());
			szPlayerIncome += NEWLINE + szTemp;
		}
	}
	if (!szPlayerIncome.empty())
	{
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_FINANCE_ADVISOR_FOREIGN_INCOME", szPlayerIncome.GetCString(), player.getGoldPerTurn());
	}
}

void CvGameTextMgr::setProductionHelp(CvWString &szBuffer, CvCity& city)
{
	FAssertMsg(NO_PLAYER != city.getOwnerINLINE(), "City must have an owner");

	int iPastOverflow = (city.isProductionProcess() ? 0 : city.getOverflowProduction());
	if (iPastOverflow != 0)
	{
		szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_PROD_OVERFLOW", iPastOverflow) + NEWLINE;
	}

	int iFromChops = (city.isProductionProcess() ? 0 : city.getFeatureProduction());
	if (iFromChops != 0)
	{
		szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_PROD_CHOPS", iFromChops) + NEWLINE;
	}

	if (city.getCurrentProductionDifference(false, true) == 0)
	{
		return;
	}

	setYieldHelp(szBuffer, city, YIELD_PRODUCTION);

	int iBaseProduction = city.getBaseYieldRate(YIELD_PRODUCTION) + iPastOverflow + iFromChops;
	int iBaseModifier = city.getBaseYieldRateModifier(YIELD_PRODUCTION);

	UnitTypes eUnit = city.getProductionUnit();
	if (NO_UNIT != eUnit)
	{
		CvUnitInfo& unit = GC.getUnitInfo(eUnit);

		// Domain
		int iDomainMod = city.getDomainProductionModifier((DomainTypes)unit.getDomainType());
		if (0 != iDomainMod)
		{
			szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_PROD_DOMAIN", iDomainMod, GC.getDomainInfo((DomainTypes)unit.getDomainType()).getTextKeyWide()) + NEWLINE;
			iBaseModifier += iDomainMod;
		}

		// Military
		if (unit.isMilitaryProduction())
		{
			int iMilitaryMod = city.getMilitaryProductionModifier() + GET_PLAYER(city.getOwnerINLINE()).getMilitaryProductionModifier();
			if (0 != iMilitaryMod)
			{
				szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_PROD_MILITARY", iMilitaryMod) + NEWLINE;
				iBaseModifier += iMilitaryMod;
			}
		}

		// Bonus
		for (int i = 0; i < GC.getNumBonusInfos(); i++)
		{
			if (city.hasBonus((BonusTypes)i))
			{
				int iBonusMod = unit.getBonusProductionModifier(i);
				if (0 != iBonusMod)
				{
					szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_PROD_MOD_BONUS", iBonusMod, unit.getTextKeyWide(), GC.getBonusInfo((BonusTypes)i).getTextKeyWide()) + NEWLINE;
					iBaseModifier += iBonusMod;
				}
			}
		}

		// Trait
		for (int i = 0; i < GC.getNumTraitInfos(); i++)
		{
			if (city.hasTrait((TraitTypes)i))
			{
				int iTraitMod = unit.getProductionTraits(i);

				if (unit.getSpecialUnitType() != NO_SPECIALUNIT)
				{
					iTraitMod += GC.getSpecialUnitInfo((SpecialUnitTypes) unit.getSpecialUnitType()).getProductionTraits(i);
				}
				if (0 != iTraitMod)
				{
					szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_PROD_TRAIT", iTraitMod, unit.getTextKeyWide(), GC.getTraitInfo((TraitTypes)i).getTextKeyWide()) + NEWLINE;
					iBaseModifier += iTraitMod;
				}
			}
		}

		// Religion
		if (NO_PLAYER != city.getOwnerINLINE() && NO_RELIGION != GET_PLAYER(city.getOwnerINLINE()).getStateReligion())
		{
			if (city.isHasReligion(GET_PLAYER(city.getOwnerINLINE()).getStateReligion()))
			{
				int iReligionMod = GET_PLAYER(city.getOwnerINLINE()).getStateReligionUnitProductionModifier();
				if (0 != iReligionMod)
				{
					szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_PROD_RELIGION", iReligionMod, GC.getReligionInfo(GET_PLAYER(city.getOwnerINLINE()).getStateReligion()).getTextKeyWide()) + NEWLINE;
					iBaseModifier += iReligionMod;
				}
			}
		}
	}

	BuildingTypes eBuilding = city.getProductionBuilding();
	if (NO_BUILDING != eBuilding)
	{
		CvBuildingInfo& building = GC.getBuildingInfo(eBuilding);

		// Bonus
		for (int i = 0; i < GC.getNumBonusInfos(); i++)
		{
			if (city.hasBonus((BonusTypes)i))
			{
				int iBonusMod = building.getBonusProductionModifier(i);
				if (0 != iBonusMod)
				{
					szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_PROD_MOD_BONUS", iBonusMod, building.getTextKeyWide(), GC.getBonusInfo((BonusTypes)i).getTextKeyWide()) + NEWLINE;
					iBaseModifier += iBonusMod;
				}
			}
		}

		// Trait
		for (int i = 0; i < GC.getNumTraitInfos(); i++)
		{
			if (city.hasTrait((TraitTypes)i))
			{
				int iTraitMod = building.getProductionTraits(i);

				if (building.getSpecialBuildingType() != NO_SPECIALBUILDING)
				{
					iTraitMod += GC.getSpecialBuildingInfo((SpecialBuildingTypes) building.getSpecialBuildingType()).getProductionTraits(i);
				}
				if (0 != iTraitMod)
				{
					szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_PROD_TRAIT", iTraitMod, building.getTextKeyWide(), GC.getTraitInfo((TraitTypes)i).getTextKeyWide()) + NEWLINE;
					iBaseModifier += iTraitMod;
				}
			}
		}

		// Wonder
		if (isWorldWonderClass((BuildingClassTypes)(GC.getBuildingInfo(eBuilding).getBuildingClassType())) && NO_PLAYER != city.getOwnerINLINE())
		{
			int iWonderMod = GET_PLAYER(city.getOwnerINLINE()).getMaxGlobalBuildingProductionModifier();
			if (0 != iWonderMod)
			{
				szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_PROD_WONDER", iWonderMod) + NEWLINE;
				iBaseModifier += iWonderMod;
			}
		}

		// Team Wonder
		if (isTeamWonderClass((BuildingClassTypes)(GC.getBuildingInfo(eBuilding).getBuildingClassType())) && NO_PLAYER != city.getOwnerINLINE())
		{
			int iWonderMod = GET_PLAYER(city.getOwnerINLINE()).getMaxTeamBuildingProductionModifier();
			if (0 != iWonderMod)
			{
				szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_PROD_TEAM_WONDER", iWonderMod) + NEWLINE;
				iBaseModifier += iWonderMod;
			}
		}

		// National Wonder
		if (isNationalWonderClass((BuildingClassTypes)(GC.getBuildingInfo(eBuilding).getBuildingClassType())) && NO_PLAYER != city.getOwnerINLINE())
		{
			int iWonderMod = GET_PLAYER(city.getOwnerINLINE()).getMaxPlayerBuildingProductionModifier();
			if (0 != iWonderMod)
			{
				szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_PROD_NATIONAL_WONDER", iWonderMod) + NEWLINE;
				iBaseModifier += iWonderMod;
			}
		}

		// Religion
		if (NO_PLAYER != city.getOwnerINLINE() && NO_RELIGION != GET_PLAYER(city.getOwnerINLINE()).getStateReligion())
		{
			if (city.isHasReligion(GET_PLAYER(city.getOwnerINLINE()).getStateReligion()))
			{
				int iReligionMod = GET_PLAYER(city.getOwnerINLINE()).getStateReligionBuildingProductionModifier();
				if (0 != iReligionMod)
				{
					szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_PROD_RELIGION", iReligionMod, GC.getReligionInfo(GET_PLAYER(city.getOwnerINLINE()).getStateReligion()).getTextKeyWide()) + NEWLINE;
					iBaseModifier += iReligionMod;
				}
			}
		}
	}

	ProjectTypes eProject = city.getProductionProject();
	if (NO_PROJECT != eProject)
	{
		CvProjectInfo& project = GC.getProjectInfo(eProject);

		// Spaceship
		if (project.isSpaceship())
		{
			int iSpaceshipMod = city.getSpaceProductionModifier();
			if (NO_PLAYER != city.getOwnerINLINE())
			{
				iSpaceshipMod += GET_PLAYER(city.getOwnerINLINE()).getSpaceProductionModifier();
			}
			if (0 != iSpaceshipMod)
			{
				szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_PROD_SPACESHIP", iSpaceshipMod) + NEWLINE;
				iBaseModifier += iSpaceshipMod;
			}
		}

		// Bonus
		for (int i = 0; i < GC.getNumBonusInfos(); i++)
		{
			if (city.hasBonus((BonusTypes)i))
			{
				int iBonusMod = project.getBonusProductionModifier(i);
				if (0 != iBonusMod)
				{
					szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_PROD_MOD_BONUS", iBonusMod, project.getTextKeyWide(), GC.getBonusInfo((BonusTypes)i).getTextKeyWide()) + NEWLINE;
					iBaseModifier += iBonusMod;
				}
			}
		}
	}

	int iFoodProduction = (city.isFoodProduction() ? max(0, (city.getYieldRate(YIELD_FOOD) - city.foodConsumption(true))) : 0);
	if (iFoodProduction > 0)
	{
		szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_PROD_FOOD", iFoodProduction, iFoodProduction) + NEWLINE;
	}

	int iModProduction = iFoodProduction + (iBaseModifier * iBaseProduction) / 100;

	FAssertMsg(iModProduction == city.getCurrentProductionDifference(false, true), "Modified Production does not match actual value");

	szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_PROD_FINAL_YIELD", iModProduction) + NEWLINE;

}

void CvGameTextMgr::parseLeaderHeadHelp(CvWString &szBuffer, PlayerTypes eThisPlayer, PlayerTypes eOtherPlayer)
{
	if (NO_PLAYER == eThisPlayer)
	{
		return;
	}

	szBuffer += CvWString::format(L"%s\n", GET_PLAYER(eThisPlayer).getName());

	if (eOtherPlayer != NO_PLAYER)
	{
		CvTeam& thisTeam = GET_TEAM(GET_PLAYER(eThisPlayer).getTeam());

		if (eOtherPlayer != eThisPlayer && thisTeam.isHasMet(GET_PLAYER(eOtherPlayer).getTeam()))
		{
			getAttitudeString(szBuffer, eThisPlayer, eOtherPlayer);

			getActiveDealsString(szBuffer, eThisPlayer, eOtherPlayer);
		}
	}
}


void CvGameTextMgr::parseLeaderLineHelp(CvWString &szBuffer, PlayerTypes eThisPlayer, PlayerTypes eOtherPlayer)
{
	if (NO_PLAYER == eThisPlayer || NO_PLAYER == eOtherPlayer)
	{
		return;
	}
	CvTeam& thisTeam = GET_TEAM(GET_PLAYER(eThisPlayer).getTeam());
	CvTeam& otherTeam = GET_TEAM(GET_PLAYER(eOtherPlayer).getTeam());

	if (thisTeam.getID() == otherTeam.getID())
	{
		szBuffer += gDLL->getText("TXT_KEY_MISC_PERMANENT_ALLIANCE") + NEWLINE;
	}
	else if (thisTeam.isAtWar(otherTeam.getID()))
	{
		szBuffer += gDLL->getText("TXT_KEY_CONCEPT_WAR") + NEWLINE;
	}
	else
	{
		if (thisTeam.isDefensivePact(otherTeam.getID()))
		{
			szBuffer += gDLL->getText("TXT_KEY_MISC_DEFENSIVE_PACT") + NEWLINE;
		}
		if (thisTeam.isOpenBorders(otherTeam.getID()))
		{
			szBuffer += gDLL->getText("TXT_KEY_MISC_OPEN_BORDERS") + NEWLINE;
		}
		if (thisTeam.isVassal(otherTeam.getID()))
		{
			szBuffer += gDLL->getText("TXT_KEY_MISC_VASSAL") + NEWLINE;
		}
	}
}


void CvGameTextMgr::getActiveDealsString(CvWString &szBuffer, PlayerTypes eThisPlayer, PlayerTypes eOtherPlayer)
{
	int iIndex;
	CvDeal* pDeal = GC.getGameINLINE().firstDeal(&iIndex);
	while (NULL != pDeal)
	{
		if ((pDeal->getFirstPlayer() == eThisPlayer && pDeal->getSecondPlayer() == eOtherPlayer)
			|| (pDeal->getFirstPlayer() == eOtherPlayer && pDeal->getSecondPlayer() == eThisPlayer))
		{
			szBuffer += NEWLINE + CvWString::format(L"%c", gDLL->getSymbolID(BULLET_CHAR));
			getDealString(szBuffer, *pDeal, eThisPlayer);
		}
		pDeal = GC.getGameINLINE().nextDeal(&iIndex);
	}
}

void CvGameTextMgr::buildHintsList(CvWString& szBuffer)
{
	for (int i = 0; i < GC.getNumHints(); i++)
	{
		szBuffer += CvWString::format(L"%c%s", gDLL->getSymbolID(BULLET_CHAR), GC.getHints(i).getText()) + NEWLINE + NEWLINE;
	}
}

void CvGameTextMgr::setCommerceHelp(CvWString &szBuffer, CvCity& city, CommerceTypes eCommerceType)
{
	if (NO_COMMERCE == eCommerceType || 0 == city.getCommerceRateTimes100(eCommerceType))
	{
		return;
	}
	CvCommerceInfo& info = GC.getCommerceInfo(eCommerceType);

	if (NO_PLAYER == city.getOwnerINLINE())
	{
		return;
	}
	CvPlayer& owner = GET_PLAYER(city.getOwnerINLINE());

	setYieldHelp(szBuffer, city, YIELD_COMMERCE);

	int iBaseCommerce = city.getBaseYieldRate(YIELD_COMMERCE);
	int iBaseModifier = city.getBaseYieldRateModifier(YIELD_COMMERCE);

	// Slider
	int iBaseCommerceRate = city.getCommerceFromPercent(eCommerceType, city.getYieldRate(YIELD_COMMERCE) * 100);
	CvWString szRate = CvWString::format(L"%d.%02d", iBaseCommerceRate/100, iBaseCommerceRate%100);
	szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_SLIDER_PERCENT_FLOAT", owner.getCommercePercent(eCommerceType), city.getYieldRate(YIELD_COMMERCE), szRate.GetCString(), info.getChar()) + NEWLINE;

	int iSpecialistCommerce = city.getSpecialistCommerce(eCommerceType) + (city.getSpecialistPopulation() + city.getNumGreatPeople()) * owner.getSpecialistExtraCommerce(eCommerceType);
	if (0 != iSpecialistCommerce)
	{
		szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_SPECIALIST_COMMERCE", iSpecialistCommerce, info.getChar(), L"TXT_KEY_CONCEPT_SPECIALISTS") + NEWLINE;
		iBaseCommerceRate += 100*iSpecialistCommerce;
	}

	int iReligionCommerce = city.getReligionCommerce(eCommerceType);
	if (0 != iReligionCommerce)
	{
		szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_RELIGION_COMMERCE", iReligionCommerce, info.getChar()) + NEWLINE;
		iBaseCommerceRate += 100*iReligionCommerce;
	}

	int iBuildingCommerce = city.getBuildingCommerce(eCommerceType);
	if (0 != iBuildingCommerce)
	{
		szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_BUILDING_COMMERCE", iBuildingCommerce, info.getChar()) + NEWLINE;
		iBaseCommerceRate += 100*iBuildingCommerce;
	}

	int iFreeCityCommerce = owner.getFreeCityCommerce(eCommerceType);
	if (0 != iFreeCityCommerce)
	{
		szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_FREE_CITY_COMMERCE", iFreeCityCommerce, info.getChar()) + NEWLINE;
		iBaseCommerceRate += 100*iFreeCityCommerce;
	}


	FAssertMsg(city.getBaseCommerceRateTimes100(eCommerceType) == iBaseCommerceRate, "Base Commerce rate does not agree with actual value");
	//szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_COMMERCE_BASE", info.getTextKeyWide(), iBaseYield, info.getChar()) + NEWLINE;

	int iModifier = 100;

	// Buildings
	int iBuildingMod = 0;
	for (int i = 0; i < GC.getNumBuildingInfos(); i++)
	{
		CvBuildingInfo& infoBuilding = GC.getBuildingInfo((BuildingTypes)i);
		if (city.hasBuilding((BuildingTypes)i) && !GET_TEAM(city.getTeam()).isObsoleteBuilding((BuildingTypes)i))
		{
			iBuildingMod += infoBuilding.getCommerceModifier(eCommerceType);
		}
		for (int j = 0; j < MAX_PLAYERS; j++)
		{
			if (GET_PLAYER((PlayerTypes)j).isAlive())
			{
				if (GET_PLAYER((PlayerTypes)j).getTeam() == owner.getTeam())
				{
					int iLoop;
					for (CvCity* pLoopCity = GET_PLAYER((PlayerTypes)j).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)j).nextCity(&iLoop))
					{
						if (pLoopCity->hasBuilding((BuildingTypes)i) && !GET_TEAM(pLoopCity->getTeam()).isObsoleteBuilding((BuildingTypes)i))
						{
							iBuildingMod += infoBuilding.getGlobalCommerceModifier(eCommerceType);
						}
					}
				}
			}
		}
	}
	if (0 != iBuildingMod)
	{
		szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_YIELD_BUILDINGS", iBuildingMod, info.getChar()) + NEWLINE;
		iModifier += iBuildingMod;
	}


	// Trait
	for (int i = 0; i < GC.getNumTraitInfos(); i++)
	{
		if (city.hasTrait((TraitTypes)i))
		{
			CvTraitInfo& trait = GC.getTraitInfo((TraitTypes)i);
			int iTraitMod = trait.getCommerceModifier(eCommerceType);
			if (0 != iTraitMod)
			{
				szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_COMMERCE_TRAIT", iTraitMod, info.getChar(), trait.getTextKeyWide()) + NEWLINE;
				iModifier += iTraitMod;
			}
		}
	}

	// Capital
	int iCapitalMod = city.isCapital() ? owner.getCapitalCommerceRateModifier(eCommerceType) : 0;
	if (iCapitalMod != 0)
	{
		szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_YIELD_CAPITAL", iCapitalMod, info.getChar()) + NEWLINE;
		iModifier += iCapitalMod;
	}


	// Civics
	int iCivicMod = 0;
	for (int i = 0; i < GC.getNumCivicOptionInfos(); i++)
	{
		if (NO_CIVIC != owner.getCivics((CivicOptionTypes)i))
		{
			iCivicMod += GC.getCivicInfo(owner.getCivics((CivicOptionTypes)i)).getCommerceModifier(eCommerceType);
		}
	}
	if (0 != iCivicMod)
	{
		szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_YIELD_CIVICS", iCivicMod, info.getChar()) + NEWLINE;
		iModifier += iCivicMod;
	}

	int iModYield = (iModifier * iBaseCommerceRate) / 100;

	int iProductionToCommerce = city.getProductionToCommerceModifier(eCommerceType) * city.getYieldRate(YIELD_PRODUCTION);
	if (0 != iProductionToCommerce)
	{
		if (iProductionToCommerce%100 == 0)
		{
			szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_PRODUCTION_TO_COMMERCE", iProductionToCommerce/100, info.getChar()) + NEWLINE;
		}
		else
		{
			szRate = CvWString::format(L"+%d.%02d", iProductionToCommerce/100, iProductionToCommerce%100);
			szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_PRODUCTION_TO_COMMERCE_FLOAT", szRate.GetCString(), info.getChar()) + NEWLINE;
		}
		iModYield += iProductionToCommerce;
	}

	FAssertMsg(iModYield == city.getCommerceRateTimes100(eCommerceType), "Commerce yield does not match actual value");

	CvWString szYield = CvWString::format(L"%d.%02d", iModYield/100, iModYield%100);
	szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_COMMERCE_FINAL_YIELD_FLOAT", info.getTextKeyWide(), szYield.GetCString(), info.getChar()) + NEWLINE;

}

void CvGameTextMgr::setYieldHelp(CvWString &szBuffer, CvCity& city, YieldTypes eYieldType)
{
	FAssertMsg(NO_PLAYER != city.getOwnerINLINE(), "City must have an owner");

	if (NO_YIELD == eYieldType)
	{
		return;
	}
	CvYieldInfo& info = GC.getYieldInfo(eYieldType);

	if (NO_PLAYER == city.getOwnerINLINE())
	{
		return;
	}
	CvPlayer& owner = GET_PLAYER(city.getOwnerINLINE());

	int iBaseProduction = city.getBaseYieldRate(eYieldType);
	szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_BASE_YIELD", info.getTextKeyWide(), iBaseProduction, info.getChar()) + NEWLINE;

	int iBaseModifier = 100;

	// Buildings
	int iBuildingMod = 0;
	for (int i = 0; i < GC.getNumBuildingInfos(); i++)
	{
		CvBuildingInfo& infoBuilding = GC.getBuildingInfo((BuildingTypes)i);
		if (city.hasBuilding((BuildingTypes)i) && !GET_TEAM(city.getTeam()).isObsoleteBuilding((BuildingTypes)i))
		{
			iBuildingMod += infoBuilding.getYieldModifier(eYieldType);
		}
		for (int j = 0; j < MAX_PLAYERS; j++)
		{
			if (GET_PLAYER((PlayerTypes)j).isAlive())
			{
				if (GET_PLAYER((PlayerTypes)j).getTeam() == owner.getTeam())
				{
					int iLoop;
					for (CvCity* pLoopCity = GET_PLAYER((PlayerTypes)j).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)j).nextCity(&iLoop))
					{
						if (pLoopCity->hasBuilding((BuildingTypes)i) && !GET_TEAM(pLoopCity->getTeam()).isObsoleteBuilding((BuildingTypes)i))
						{
							iBuildingMod += infoBuilding.getGlobalYieldModifier(eYieldType);
						}
					}
				}
			}
		}
	}
	if (NULL != city.area())
	{
		iBuildingMod += city.area()->getYieldRateModifier(city.getOwnerINLINE(), eYieldType);
	}
	if (0 != iBuildingMod)
	{
		szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_YIELD_BUILDINGS", iBuildingMod, info.getChar()) + NEWLINE;
		iBaseModifier += iBuildingMod;
	}

	// Power
	if (city.isPower())
	{
		int iPowerMod = city.getPowerYieldRateModifier(eYieldType);
		if (0 != iPowerMod)
		{
			szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_YIELD_POWER", iPowerMod, info.getChar()) + NEWLINE;
			iBaseModifier += iPowerMod;
		}
	}

	// Resources
	int iBonusMod = city.getBonusYieldRateModifier(eYieldType);
	if (0 != iBonusMod)
	{
		szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_YIELD_BONUS", iBonusMod, info.getChar()) + NEWLINE;
		iBaseModifier += iBonusMod;
	}

	// Capital
	if (city.isCapital())
	{
		int iCapitalMod = owner.getCapitalYieldRateModifier(eYieldType);
		if (0 != iCapitalMod)
		{
			szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_YIELD_CAPITAL", iCapitalMod, info.getChar()) + NEWLINE;
			iBaseModifier += iCapitalMod;
		}
	}

	// Civics
	int iCivicMod = 0;
	for (int i = 0; i < GC.getNumCivicOptionInfos(); i++)
	{
		if (NO_CIVIC != owner.getCivics((CivicOptionTypes)i))
		{
			iCivicMod += GC.getCivicInfo(owner.getCivics((CivicOptionTypes)i)).getYieldModifier(eYieldType);
		}
	}
	if (0 != iCivicMod)
	{
		szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_YIELD_CIVICS", iCivicMod, info.getChar()) + NEWLINE;
		iBaseModifier += iCivicMod;
	}

	FAssertMsg((iBaseModifier * iBaseProduction) / 100 == city.getYieldRate(eYieldType), "Yield Modifier in setProductionHelp does not agree with actual value");
}

void CvGameTextMgr::setConvertHelp(CvWString& szBuffer, PlayerTypes ePlayer, ReligionTypes eReligion)
{
	CvWString szReligion = L"TXT_KEY_MISC_NO_STATE_RELIGION";

	if (eReligion != NO_RELIGION)
	{
		szReligion = GC.getReligionInfo(eReligion).getTextKeyWide();
	}

	szBuffer = gDLL->getText("TXT_KEY_MISC_CANNOT_CONVERT_TO", szReligion.GetCString());

	if (GET_PLAYER(ePlayer).isAnarchy())
	{
		szBuffer += gDLL->getText("TXT_KEY_MISC_WHILE_IN_ANARCHY");
	}
	else if (GET_PLAYER(ePlayer).getStateReligion() == eReligion)
	{
		szBuffer += L". " + gDLL->getText("TXT_KEY_MISC_ALREADY_STATE_REL");
	}
	else if (GET_PLAYER(ePlayer).getConversionTimer() > 0)
	{
		szBuffer += gDLL->getText("TXT_KEY_MISC_ANOTHER_REVOLUTION_RECENTLY");
		CvWString szTemp;
		szTemp += L". " + gDLL->getText("TXT_KEY_MISC_WAIT_MORE_TURNS", GET_PLAYER(ePlayer).getConversionTimer());
		szBuffer += szTemp;
	}
}

void CvGameTextMgr::setRevolutionHelp(CvWString& szBuffer, PlayerTypes ePlayer)
{
	szBuffer = gDLL->getText("TXT_KEY_MISC_CANNOT_CHANGE_CIVICS");

	if (GET_PLAYER(ePlayer).isAnarchy())
	{
		szBuffer += gDLL->getText("TXT_KEY_MISC_WHILE_IN_ANARCHY");
	}
	else if (GET_PLAYER(ePlayer).getRevolutionTimer() > 0)
	{
		szBuffer += gDLL->getText("TXT_KEY_MISC_ANOTHER_REVOLUTION_RECENTLY");
		CvWString szTemp;
		szTemp += L" : " + gDLL->getText("TXT_KEY_MISC_WAIT_MORE_TURNS", GET_PLAYER(ePlayer).getRevolutionTimer());
		szBuffer += szTemp;
	}
}

void CvGameTextMgr::setVassalRevoltHelp(CvWString& szBuffer, TeamTypes eMaster, TeamTypes eVassal)
{
	if (NO_TEAM == eMaster || NO_TEAM == eVassal)
	{
		return;
	}

	CvTeam& kMaster = GET_TEAM(eMaster);
	CvTeam& kVassal = GET_TEAM(eVassal);

	int iMasterLand = kMaster.getTotalLand(false);
	int iVassalLand = kVassal.getTotalLand(false);
	if (iMasterLand > 0 && GC.getDefineINT("FREE_VASSAL_LAND_PERCENT") >= 0)
	{
		szBuffer += gDLL->getText("TXT_KEY_MISC_VASSAL_LAND_STATS", (iVassalLand * 100) / iMasterLand, GC.getDefineINT("FREE_VASSAL_LAND_PERCENT"));
	}

	int iMasterPop = kMaster.getTotalPopulation(false);
	int iVassalPop = kVassal.getTotalPopulation(false);
	if (iMasterPop > 0 && GC.getDefineINT("FREE_VASSAL_POPULATION_PERCENT") >= 0)
	{
		szBuffer += gDLL->getText("TXT_KEY_MISC_VASSAL_POPULATION_STATS", (iVassalPop * 100) / iMasterPop, GC.getDefineINT("FREE_VASSAL_POPULATION_PERCENT"));
	}

	if (GC.getDefineINT("VASSAL_REVOLT_OWN_LOSSES_FACTOR") > 0 && kVassal.getVassalPower() > 0)
	{
		szBuffer += gDLL->getText("TXT_KEY_MISC_VASSAL_AREA_LOSS", (iVassalLand * 100) / kVassal.getVassalPower(), GC.getDefineINT("VASSAL_REVOLT_OWN_LOSSES_FACTOR"));
	}

	if (GC.getDefineINT("VASSAL_REVOLT_MASTER_LOSSES_FACTOR") > 0 && kVassal.getMasterPower() > 0)
	{
		szBuffer += gDLL->getText("TXT_KEY_MISC_MASTER_AREA_LOSS", (iMasterLand * 100) / kVassal.getMasterPower(), GC.getDefineINT("VASSAL_REVOLT_MASTER_LOSSES_FACTOR"));
	}
}

void CvGameTextMgr::parseGreatPeopleHelp(CvWString &szBuffer, CvCity& city)
{
	int iTotalGreatPeopleUnitProgress;
	int iI;

	if (NO_PLAYER == city.getOwnerINLINE())
	{
		return;
	}
	CvPlayer& owner = GET_PLAYER(city.getOwnerINLINE());

	//	szBuffer.Format(L"Great Person: %d/%d %c", pHeadSelectedCity->getGreatPeopleProgress(), GET_PLAYER(pHeadSelectedCity->getOwnerINLINE()).greatPeopleThreshold(), CIV.getGameFont()->getSymbolID(GREAT_PEOPLE_CHAR));
	szBuffer = gDLL->getText("TXT_KEY_MISC_GREAT_PERSON", city.getGreatPeopleProgress(), owner.greatPeopleThreshold(false));


	if (city.getGreatPeopleRate() > 0)
	{
		int iGPPLeft = owner.greatPeopleThreshold(false) - city.getGreatPeopleProgress();

		if (iGPPLeft > 0)
		{
			int iTurnsLeft = iGPPLeft / city.getGreatPeopleRate();

			if (iTurnsLeft * city.getGreatPeopleRate() <  iGPPLeft)
			{
				iTurnsLeft++;
			}

			szBuffer += NEWLINE + gDLL->getText("INTERFACE_CITY_TURNS", max(1, iTurnsLeft));
		}
	}

	iTotalGreatPeopleUnitProgress = 0;

	for (iI = 0; iI < GC.getNumUnitInfos(); iI++)
	{
		iTotalGreatPeopleUnitProgress += city.getGreatPeopleUnitProgress((UnitTypes)iI);
	}

	if (iTotalGreatPeopleUnitProgress > 0)
	{
		szBuffer += SEPARATOR;
		szBuffer += NEWLINE + gDLL->getText("TXT_KEY_MISC_PROB");

		for (iI = 0; iI < GC.getNumUnitInfos(); iI++)
		{
			if (city.getGreatPeopleUnitProgress((UnitTypes)iI) > 0)
			{
				szBuffer += CvWString::format(L"%s%s - %d%%", NEWLINE, GC.getUnitInfo((UnitTypes) iI).getDescription(), ((city.getGreatPeopleUnitProgress((UnitTypes)iI) * 100) / iTotalGreatPeopleUnitProgress));
			}
		}
	}

	if (city.getGreatPeopleRate() == 0)
	{
		return;
	}

	szBuffer += SEPARATOR;
	szBuffer += NEWLINE + gDLL->getText("TXT_KEY_MISC_HELP_GREATPEOPLE_BASE_RATE", city.getBaseGreatPeopleRate()) + NEWLINE;

	int iModifier = 100;

	// Buildings
	int iBuildingMod = 0;
	for (int i = 0; i < GC.getNumBuildingInfos(); i++)
	{
		CvBuildingInfo& infoBuilding = GC.getBuildingInfo((BuildingTypes)i);
		if (city.hasBuilding((BuildingTypes)i) && !GET_TEAM(city.getTeam()).isObsoleteBuilding((BuildingTypes)i))
		{
			iBuildingMod += infoBuilding.getGreatPeopleRateModifier();
		}
		for (int j = 0; j < MAX_PLAYERS; j++)
		{
			if (GET_PLAYER((PlayerTypes)j).isAlive())
			{
				if (GET_PLAYER((PlayerTypes)j).getTeam() == owner.getTeam())
				{
					int iLoop;
					for (CvCity* pLoopCity = GET_PLAYER((PlayerTypes)j).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)j).nextCity(&iLoop))
					{
						if (pLoopCity->hasBuilding((BuildingTypes)i) && !GET_TEAM(pLoopCity->getTeam()).isObsoleteBuilding((BuildingTypes)i))
						{
							iBuildingMod += infoBuilding.getGlobalGreatPeopleRateModifier();
						}
					}
				}
			}
		}
	}
	if (0 != iBuildingMod)
	{
		szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_GREATPEOPLE_BUILDINGS", iBuildingMod) + NEWLINE;
		iModifier += iBuildingMod;
	}

	// Civics
	int iCivicMod = 0;
	for (int i = 0; i < GC.getNumCivicOptionInfos(); i++)
	{
		if (NO_CIVIC != owner.getCivics((CivicOptionTypes)i))
		{
			iCivicMod += GC.getCivicInfo(owner.getCivics((CivicOptionTypes)i)).getGreatPeopleRateModifier();
			if (owner.getStateReligion() != NO_RELIGION && city.isHasReligion(owner.getStateReligion()))
			{
				iCivicMod += GC.getCivicInfo(owner.getCivics((CivicOptionTypes)i)).getStateReligionGreatPeopleRateModifier();
			}
		}
	}
	if (0 != iCivicMod)
	{
		szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_GREATPEOPLE_CIVICS", iCivicMod) + NEWLINE;
		iModifier += iCivicMod;
	}

	// Trait
	for (int i = 0; i < GC.getNumTraitInfos(); i++)
	{
		if (city.hasTrait((TraitTypes)i))
		{
			CvTraitInfo& trait = GC.getTraitInfo((TraitTypes)i);
			int iTraitMod = trait.getGreatPeopleRateModifier();
			if (0 != iTraitMod)
			{
				szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_GREATPEOPLE_TRAIT", iTraitMod, trait.getTextKeyWide()) + NEWLINE;
				iModifier += iTraitMod;
			}
		}
	}

	int iModGreatPeople = (iModifier * city.getBaseGreatPeopleRate()) / 100;

	FAssertMsg(iModGreatPeople == city.getGreatPeopleRate(), "Great person rate does not match actual value");

	szBuffer += gDLL->getText("TXT_KEY_MISC_HELP_GREATPEOPLE_FINAL", iModGreatPeople) + NEWLINE;
}


void CvGameTextMgr::parseGreatGeneralHelp(CvWString &szBuffer, CvPlayer& kPlayer)
{
	szBuffer = gDLL->getText("TXT_KEY_MISC_GREAT_GENERAL", kPlayer.getCombatExperience(), kPlayer.greatPeopleThreshold(true));
}


//------------------------------------------------------------------------------------------------

void CvGameTextMgr::buildCityBillboardIconString( CvWString& szBuffer, CvCity* pCity)
{
	szBuffer.clear();
	bool bHasIcons = true;

	// government center icon
	if (pCity->isGovernmentCenter() && !(pCity->isCapital()))
	{
		szBuffer += CvWString::format(L"%c", gDLL->getSymbolID(SILVER_STAR_CHAR));
	}

	// happiness, healthiness, superlative icons
	if (pCity->canBeSelected())
	{
		if (pCity->angryPopulation() > 0)
		{
			szBuffer += CvWString::format(L"%c", gDLL->getSymbolID(UNHAPPY_CHAR));
		}

		if (pCity->healthRate() < 0)
		{
			szBuffer += CvWString::format(L"%c", gDLL->getSymbolID(UNHEALTHY_CHAR));
		}

		if (gDLL->getGraphicOption(GRAPHICOPTION_CITY_DETAIL))
		{
			if (GET_PLAYER(pCity->getOwnerINLINE()).getNumCities() > 2)
			{
				if (pCity->findYieldRateRank(YIELD_PRODUCTION) == 1)
				{
					szBuffer += CvWString::format(L"%c", GC.getYieldInfo(YIELD_PRODUCTION).getChar());
				}
				if (pCity->findCommerceRateRank(COMMERCE_GOLD) == 1)
				{
					szBuffer += CvWString::format(L"%c", GC.getCommerceInfo(COMMERCE_GOLD).getChar());
				}
				if (pCity->findCommerceRateRank(COMMERCE_RESEARCH) == 1)
				{
					szBuffer += CvWString::format(L"%c", GC.getCommerceInfo(COMMERCE_RESEARCH).getChar());
				}
			}
		}

		if (pCity->isConnectedToCapital())
		{
			if (GET_PLAYER(pCity->getOwnerINLINE()).countNumCitiesConnectedToCapital() > 1)
			{
				szBuffer += CvWString::format(L"%c", gDLL->getSymbolID(TRADE_CHAR));
			}
		}
	}

	// religion icons
	for (int iI = 0; iI < GC.getNumReligionInfos(); iI++)
	{
		if (pCity->isHasReligion((ReligionTypes)iI))
		{
			if (pCity->isHolyCity((ReligionTypes)iI))
			{
				szBuffer += CvWString::format(L"%c", GC.getReligionInfo((ReligionTypes) iI).getHolyCityChar());
			}
			else
			{
				szBuffer += CvWString::format(L"%c", GC.getReligionInfo((ReligionTypes) iI).getChar());
			}
		}
	}

	if (pCity->getTeam() == GC.getGameINLINE().getActiveTeam())
	{
		if (pCity->isPower())
		{
			szBuffer += CvWString::format(L"%c", gDLL->getSymbolID(POWER_CHAR));
		}
	}

	// XXX out this in bottom bar???
	if (pCity->isOccupation())
	{
		szBuffer += CvWString::format(L" (%c:%d)", gDLL->getSymbolID(OCCUPATION_CHAR), pCity->getOccupationTimer());
	}

	// defense icon and text
	//if (pCity->getTeam() != GC.getGameINLINE().getActiveTeam())
	{
		if (pCity->isVisible(GC.getGameINLINE().getActiveTeam(), true))
		{
			int iDefenseModifier = pCity->getDefenseModifier(GC.getGameINLINE().selectionListIgnoreBuildingDefense());

			if (iDefenseModifier != 0)
			{
				szBuffer += CvWString::format(L" %c:%s%d%%", gDLL->getSymbolID(DEFENSE_CHAR), ((iDefenseModifier > 0) ? "+" : ""), iDefenseModifier);
			}
		}
	}
}

void CvGameTextMgr::buildCityBillboardCityNameString( CvWString& szBuffer, CvCity* pCity)
{
	szBuffer = pCity->getName();

	if (pCity->canBeSelected())
	{
		if (gDLL->getGraphicOption(GRAPHICOPTION_CITY_DETAIL))
		{
			if (pCity->foodDifference() > 0)
			{
				int iTurns = pCity->getFoodTurnsLeft();

				if ((iTurns > 1) || !(pCity->AI_isEmphasizeAvoidGrowth()))
				{
					if (iTurns < MAX_INT)
					{
						szBuffer += CvWString::format(L" (%d)", iTurns);
					}
				}
			}
		}
	}
}

void CvGameTextMgr::buildCityBillboardProductionString( CvWString& szBuffer, CvCity* pCity)
{
	if (pCity->getOrderQueueLength() > 0)
	{
		szBuffer = pCity->getProductionName();

		if (gDLL->getGraphicOption(GRAPHICOPTION_CITY_DETAIL))
		{
			int iTurns = pCity->getProductionTurnsLeft();

			if (iTurns < MAX_INT)
			{
				szBuffer = szBuffer + CvWString::format(L" (%d)", iTurns);
			}
		}
	}
	else
	{
		szBuffer.clear();
	}
}


void CvGameTextMgr::buildCityBillboardCitySizeString( CvWString& szBuffer, CvCity* pCity, const NiColorA& kColor)
{
#define CAPARAMS(c) (int)((c).r * 255.0f), (int)((c).g * 255.0f), (int)((c).b * 255.0f), (int)((c).a * 255.0f)
	szBuffer.Format(SETCOLR L"%d" ENDCOLR, CAPARAMS(kColor), pCity->getPopulation());
#undef CAPARAMS
}

void CvGameTextMgr::setScoreHelp(CvWString &szString, PlayerTypes ePlayer)
{
	if (NO_PLAYER != ePlayer)
	{
		CvPlayer& player = GET_PLAYER(ePlayer);

		int iPop = player.getPopScore();
		int iMaxPop = GC.getGameINLINE().getMaxPopulation();
		int iPopScore = (GC.getDefineINT("SCORE_POPULATION_FACTOR") * iPop) / iMaxPop;
		int iLand = player.getLandScore();
		int iMaxLand = GC.getGameINLINE().getMaxLand();
		int iLandScore = (GC.getDefineINT("SCORE_LAND_FACTOR") * iLand) / iMaxLand;
		int iTech = player.getTechScore();
		int iMaxTech = GC.getGameINLINE().getMaxTech();
		int iTechScore = (GC.getDefineINT("SCORE_TECH_FACTOR") * iTech) / iMaxTech;
		int iWonders = player.getWondersScore();
		int iMaxWonders = GC.getGameINLINE().getMaxWonders();
		int iWondersScore = (GC.getDefineINT("SCORE_WONDER_FACTOR") * iWonders) / iMaxWonders;
		int iTotalScore = iPopScore + iLandScore + iTechScore + iWondersScore;
		int iVictoryScore = player.calculateScore(true, true);
		if (iTotalScore == player.calculateScore())
		{
			szString = gDLL->getText("TXT_KEY_SCORE_BREAKDOWN",
				iPopScore, iPop, iMaxPop,
				iLandScore, iLand, iMaxLand,
				iTechScore, iTech, iMaxTech,
				iWondersScore, iWonders, iMaxWonders,
				iTotalScore, iVictoryScore);
		}
	}
}
