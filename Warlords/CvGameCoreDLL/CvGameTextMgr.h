//  $Header:
//------------------------------------------------------------------------------------------------
//
//  FILE:    CvGameTextMgr.h
//
//  AUTHOR:  Jesse Smith  --  10/2004
//
//  PURPOSE: Group of functions to manage CIV Game Text
//
//------------------------------------------------------------------------------------------------
//  Copyright (c) 2004 Firaxis Games, Inc. All rights reserved.
//------------------------------------------------------------------------------------------------
#ifndef CIV4_GAME_TEXT_MGR_H
#define CIV4_GAME_TEXT_MGR_H

#include "CvInfos.h"
//#include "CvEnums.h"

#pragma warning( disable: 4251 )	// needs to have dll-interface to be used by clients of class

class CvCity;
class CvDeal;
class CvPopupInfo;
class CvPlayer;

//
// Class:		CvGameTextMgr
// Purpose:		Manages Game Text...
class CvGameTextMgr
{
	friend class CvGlobals;
public:
	// singleton accessor
	DllExport static CvGameTextMgr& GetInstance();

	DllExport CvGameTextMgr();
	DllExport virtual ~CvGameTextMgr();

	DllExport void Initialize();
	DllExport void DeInitialize();
	DllExport void Reset();

	DllExport int getCurrentLanguage();

	DllExport void setTimeStr(CvWString& szString, int iGameTurn, bool bSave);
	DllExport void setDateStr(CvWString& szString, int iGameTurn, bool bSave, CalendarTypes eCalendar, int iStartYear, GameSpeedTypes eSpeed);
	DllExport void setInterfaceTime(CvWString& szString, PlayerTypes ePlayer);
	DllExport void setGoldStr(CvWString& szString, PlayerTypes ePlayer);
	DllExport void setResearchStr(CvWString& szString, PlayerTypes ePlayer);
	DllExport void setOOSSeeds(CvWString& szString, PlayerTypes ePlayer);
	DllExport void setNetStats(CvWString& szString, PlayerTypes ePlayer);
	DllExport void setMinimizePopupHelp(CvWString& szString, const CvPopupInfo & info);

	DllExport void setUnitHelp(CvWString &szString, const CvUnit* pUnit, bool bOneLine = false, bool bShort = false);
	DllExport void setPlotListHelp(CvWString &szString, CvPlot* pPlot, bool bOneLine, bool bShort);
	DllExport bool setCombatPlotHelp(CvWString &szString, CvPlot* pPlot);
	DllExport void setPlotHelp(CvWString &szString, CvPlot* pPlot);
	DllExport void setCityBarHelp(CvWString &szString, CvCity* pCity);
	DllExport void setScoreHelp(CvWString &szString, PlayerTypes ePlayer);

	DllExport void parseTraits(CvWString &szHelpString, TraitTypes eTrait, CivilizationTypes eCivilization = NO_CIVILIZATION, bool bDawnOfMan = false);
	DllExport void parseLeaderTraits(CvWString &szInfoText, LeaderHeadTypes eLeader = NO_LEADER, CivilizationTypes eCivilization = NO_CIVILIZATION, bool bDawnOfMan = false, bool bCivilopediaText = false);
	DllExport void parseLeaderShortTraits(CvWString &szInfoText, LeaderHeadTypes eLeader);
	DllExport void parseCivInfos(CvWString &szHelpString, CivilizationTypes eCivilization, bool bDawnOfMan = false);
	DllExport void parseSpecialistHelp(CvWString &szHelpString, SpecialistTypes eSpecialist, CvCity* pCity, bool bCivilopediaText = false);
	DllExport void parseFreeSpecialistHelp(CvWString &szHelpString, const CvCity& kCity);
	DllExport void parsePromotionHelp(CvWString &szBuffer, PromotionTypes ePromotion, const wchar* pcNewline = NEWLINE);
	DllExport void parseCivicInfo(CvWString &szBuffer, CivicTypes eCivic, bool bCivilopediaText = false, bool bPlayerContext = false, bool bSkipName = false);
	DllExport void parseLeaderHeadHelp(CvWString &szBuffer, PlayerTypes eThisPlayer, PlayerTypes eOtherPlayer);
	DllExport void parseLeaderLineHelp(CvWString &szBuffer, PlayerTypes eThisPlayer, PlayerTypes eOtherPlayer);
	DllExport void parseGreatPeopleHelp(CvWString &szBuffer, CvCity& city);
	DllExport void parseGreatGeneralHelp(CvWString &szBuffer, CvPlayer& kPlayer);

	DllExport void setTechHelp(CvWString &szBuffer, TechTypes eTech, bool bCivilopediaText = false, bool bPlayerContext = false, bool bStrategyText = false, bool bTreeInfo = true, TechTypes eFromTech = NO_TECH);
	DllExport void setBasicUnitHelp(CvWString &szBuffer, UnitTypes eUnit, bool bCivilopediaText = false);
	DllExport void setUnitHelp(CvWString &szBuffer, UnitTypes eUnit, bool bCivilopediaText = false, bool bStrategyText = false, bool bTechChooserText = false, CvCity* pCity = NULL);
	DllExport void setBuildingHelp(CvWString &szBuffer, BuildingTypes eBuilding, bool bCivilopediaText = false, bool bStrategyText = false, bool bTechChooserText = false, CvCity* pCity = NULL);
	DllExport void setProjectHelp(CvWString &szBuffer, ProjectTypes eProject, bool bCivilopediaText = false, CvCity* pCity = NULL);
	DllExport void setProcessHelp(CvWString &szBuffer, ProcessTypes eProcess);
	DllExport void setGoodHealthHelp(CvWString &szBuffer, CvCity& city);
	DllExport void setBadHealthHelp(CvWString &szBuffer, CvCity& city);
	DllExport void setAngerHelp(CvWString &szBuffer, CvCity& city);
	DllExport void setHappyHelp(CvWString &szBuffer, CvCity& city);
	DllExport void setYieldChangeHelp(CvWString &szBuffer, const CvWString& szStart, const CvWString& szSpace, const CvWString& szEnd, const int* piYieldChange, bool bPercent = false, bool bNewLine = true);
	DllExport void setCommerceChangeHelp(CvWString &szBuffer, const CvWString& szStart, const CvWString& szSpace, const CvWString& szEnd, const int* piCommerceChange, bool bPercent = false, bool bNewLine = true);
	DllExport void setBonusHelp(CvWString &szBuffer, BonusTypes eBonus, bool bCivilopediaText = false);
	DllExport void setReligionHelp(CvWString &szBuffer, ReligionTypes eReligion, bool bCivilopedia = false);
	DllExport void setReligionHelpCity(CvWString &szBuffer, ReligionTypes eReligion, CvCity *pCity, bool bCityScreen = false, bool bForceReligion = false, bool bForceState = false, bool bNoStateReligion = false);
	DllExport void setPromotionHelp(CvWString &szBuffer, PromotionTypes ePromotion, bool bCivilopediaText = false);
	DllExport void setUnitCombatHelp(CvWString &szBuffer, UnitCombatTypes eUnitCombat);
	DllExport void setImprovementHelp(CvWString &szBuffer, ImprovementTypes eImprovement, bool bCivilopediaText = false);
	DllExport void setTerrainHelp(CvWString &szBuffer, TerrainTypes eTerrain, bool bCivilopediaText = false);
	DllExport void setFeatureHelp(CvWString &szBuffer, FeatureTypes eFeature, bool bCivilopediaText = false);
	DllExport void setProductionHelp(CvWString &szBuffer, CvCity& city);
	DllExport void setCommerceHelp(CvWString &szBuffer, CvCity& city, CommerceTypes eCommerceType);
	DllExport void setYieldHelp(CvWString &szBuffer, CvCity& city, YieldTypes eYieldType);
	DllExport void setConvertHelp(CvWString& szBuffer, PlayerTypes ePlayer, ReligionTypes eReligion);
	DllExport void setRevolutionHelp(CvWString& szBuffer, PlayerTypes ePlayer);
	DllExport void setVassalRevoltHelp(CvWString& szBuffer, TeamTypes eMaster, TeamTypes eVassal);

	DllExport void buildObsoleteString( CvWString& szBuffer, int iItem, bool bList = false, bool bPlayerContext = false );
	DllExport void buildObsoleteBonusString( CvWString& szBuffer, int iItem, bool bList = false, bool bPlayerContext = false);
	DllExport void buildObsoleteSpecialString( CvWString& szBuffer, int iItem, bool bList = false, bool bPlayerContext = false );
	DllExport void buildMoveString( CvWString& szBuffer, TechTypes eTech, bool bList = false, bool bPlayerContext = false );
	DllExport void buildFreeUnitString( CvWString& szBuffer, TechTypes eTech, bool bList = false, bool bPlayerContext = false );
	DllExport void buildFeatureProductionString( CvWString& szBuffer, TechTypes eTech, bool bList = false, bool bPlayerContext = false );
	DllExport void buildWorkerRateString( CvWString& szBuffer, TechTypes eTech, bool bList = false, bool bPlayerContext = false );
	DllExport void buildTradeRouteString( CvWString& szBuffer, TechTypes eTech, bool bList = false, bool bPlayerContext = false );
	DllExport void buildHealthRateString( CvWString& szBuffer, TechTypes eTech, bool bList = false, bool bPlayerContext = false );
	DllExport void buildHappinessRateString( CvWString& szBuffer, TechTypes eTech, bool bList = false, bool bPlayerContext = false );
	DllExport void buildFreeTechString( CvWString& szBuffer, TechTypes eTech, bool bList = false, bool bPlayerContext = false );
	DllExport void buildLOSString( CvWString& szBuffer, TechTypes eTech, bool bList = false, bool bPlayerContext = false );
	DllExport void buildMapCenterString( CvWString& szBuffer, TechTypes eTech, bool bList = false, bool bPlayerContext = false );
	DllExport void buildMapRevealString( CvWString& szBuffer, TechTypes eTech, bool bList = false );
	DllExport void buildMapTradeString( CvWString& szBuffer, TechTypes eTech, bool bList = false, bool bPlayerContext = false );
	DllExport void buildTechTradeString( CvWString& szBuffer, TechTypes eTech, bool bList = false, bool bPlayerContext = false );
	DllExport void buildGoldTradeString( CvWString& szBuffer, TechTypes eTech, bool bList = false, bool bPlayerContext = false );
	DllExport void buildOpenBordersString( CvWString& szBuffer, TechTypes eTech, bool bList = false, bool bPlayerContext = false );
	DllExport void buildDefensivePactString( CvWString& szBuffer, TechTypes eTech, bool bList = false, bool bPlayerContext = false );
	DllExport void buildPermanentAllianceString( CvWString& szBuffer, TechTypes eTech, bool bList = false, bool bPlayerContext = false );
	DllExport void buildVassalStateString( CvWString& szBuffer, TechTypes eTech, bool bList = false, bool bPlayerContext = false );
	DllExport void buildBridgeString( CvWString& szBuffer, TechTypes eTech, bool bList = false, bool bPlayerContext = false );
	DllExport void buildIrrigationString( CvWString& szBuffer, TechTypes eTech, bool bList = false, bool bPlayerContext = false );
	DllExport void buildIgnoreIrrigationString( CvWString& szBuffer, TechTypes eTech, bool bList = false, bool bPlayerContext = false );
	DllExport void buildWaterWorkString( CvWString &szBuffer, TechTypes eTech, bool bList = false, bool bPlayerContext = false );
	DllExport void buildImprovementString( CvWString& szBuffer, TechTypes eTech, int iImprovement, bool bList = false, bool bPlayerContext = false );
	DllExport void buildDomainExtraMovesString( CvWString& szBuffer, TechTypes eTech, int iCommerceType, bool bList = false, bool bPlayerContext = false );
	DllExport void buildAdjustString( CvWString& szBuffer, TechTypes eTech, int iCommerceType, bool bList = false, bool bPlayerContext = false );
	DllExport void buildTerrainTradeString( CvWString& szBuffer, TechTypes eTech, int iTerrainType, bool bList = false, bool bPlayerContext = false );
	DllExport void buildSpecialBuildingString( CvWString& szBuffer, TechTypes eTech, int iBuildingType, bool bList = false, bool bPlayerContext = false );
	DllExport void buildYieldChangeString( CvWString& szBuffer, TechTypes eTech, int iTieldType, bool bList = false, bool bPlayerContext = false );
	DllExport bool buildBonusRevealString( CvWString& szBuffer, TechTypes eTech, int iBonusType, bool bFirst, bool bList = false, bool bPlayerContext = false );
	DllExport bool buildCivicRevealString( CvWString& szBuffer, TechTypes eTech, int iCivicType, bool bFirst, bool bList = false, bool bPlayerContext = false );
	DllExport bool buildProcessInfoString( CvWString& szBuffer, TechTypes eTech, int iProcessType, bool bFirst, bool bList = false, bool bPlayerContext = false );
	DllExport bool buildFoundReligionString( CvWString& szBuffer, TechTypes eTech, int iReligionType, bool bFirst, bool bList = false, bool bPlayerContext = false );
	DllExport bool buildPromotionString( CvWString& szBuffer, TechTypes eTech, int iPromotionType, bool bFirst, bool bList = false, bool bPlayerContext = false );
	DllExport void buildHintsList(CvWString& szBuffer);

	DllExport void buildCityBillboardIconString( CvWString& szBuffer, CvCity* pCity);
	DllExport void buildCityBillboardCityNameString( CvWString& szBuffer, CvCity* pCity);
	DllExport void buildCityBillboardProductionString( CvWString& szBuffer, CvCity* pCity);
	DllExport void buildCityBillboardCitySizeString( CvWString& szBuffer, CvCity* pCity, const NiColorA& kColor);

	DllExport void buildSingleLineTechTreeString(CvWString &szBuffer, TechTypes eTech, bool bPlayerContext);
	DllExport void buildTechTreeString(CvWString &szBuffer, TechTypes eTech, bool bPlayerContext, TechTypes eFromTech);

	DllExport void getAttitudeString(CvWString& szBuffer, PlayerTypes ePlayer, PlayerTypes eTargetPlayer);
	DllExport void getTradeString(CvWString& szBuffer, const TradeData& tradeData, PlayerTypes ePlayer1, PlayerTypes ePlayer2);
	DllExport void getDealString(CvWString& szString, CvDeal& deal, PlayerTypes ePlayerPerspective = NO_PLAYER);
	DllExport void getActiveDealsString(CvWString& szString, PlayerTypes eThisPlayer, PlayerTypes eOtherPlayer);

	DllExport void buildFinanceInflationString(CvWString& szDetails, PlayerTypes ePlayer);
	DllExport void buildFinanceUnitCostString(CvWString& szDetails, PlayerTypes ePlayer);
	DllExport void buildFinanceAwaySupplyString(CvWString& szDetails, PlayerTypes ePlayer);
	DllExport void buildFinanceCityMaintString(CvWString& szDetails, PlayerTypes ePlayer);
	DllExport void buildFinanceCivicUpkeepString(CvWString& szDetails, PlayerTypes ePlayer);
	DllExport void buildFinanceForeignIncomeString(CvWString& szDetails, PlayerTypes ePlayer);

private:

	struct ltstr
	{
		bool operator()(const wchar* s1, const wchar* s2) const
		{
			return wcscmp(s1, s2) < 0;
		}
	};
	
	int m_iNumDiplomacyTextInfos;
	CvDiplomacyTextInfo* m_paDiplomacyTextInfo;
	std::vector<int*> m_apbPromotion;
};

// Singleton Accessor
#define GAMETEXT CvGameTextMgr::GetInstance()

#endif
