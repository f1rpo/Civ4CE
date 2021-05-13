// player.cpp

#include "CvGameCoreDLL.h"
#include "CvGlobals.h"
#include "CvArea.h"
#include "CvGameAI.h"
#include "CvMap.h"
#include "CvPlot.h"
#include "CvRandom.h"
#include "CvTeamAI.h"
#include "CvGameCoreUtils.h"
#include "CvPlayerAI.h"
#include "CvPlayer.h"
#include "CvGameCoreUtils.h"
#include "CvArtFileMgr.h"
#include "CvDiploParameters.h"
#include "CvInitCore.h"
#include "CyArgsList.h"
#include "CvInfos.h"
#include "CvPopupInfo.h"
#include "CvDiploParameters.h"
#include "FProfiler.h"
#include "CvGameTextMgr.h"
#include "CyCity.h"
#include "CyPlot.h"
#include "CvTradeRoute.h"
#include <numeric>

#include "CvDLLInterfaceIFaceBase.h"
#include "CvDLLEntityIFaceBase.h"
#include "CvDLLEngineIFaceBase.h"
#include "CvDLLFAStarIFaceBase.h"
#include "CvDLLEventReporterIFaceBase.h"
#include "CvDLLPythonIFaceBase.h"


// Public Functions...

CvPlayer::CvPlayer()
{
	m_aiSeaPlotYield = new int[NUM_YIELD_TYPES];
	m_aiYieldRateModifier = new int[NUM_YIELD_TYPES];
	m_aiCapitalYieldRateModifier = new int[NUM_YIELD_TYPES];
	m_aiBuildingRequiredYieldModifier = new int[NUM_YIELD_TYPES];
	m_aiCityExtraYield = new int[NUM_YIELD_TYPES];
	m_aiExtraYieldThreshold = new int[NUM_YIELD_TYPES];
	m_aiYieldBuyPrice = new int[NUM_YIELD_TYPES];
	m_aiYieldTradedTotal = new int[NUM_YIELD_TYPES];
	m_aiYieldBoughtTotal = new int[NUM_YIELD_TYPES];
	m_aiTaxYieldModifierCount = new int[NUM_YIELD_TYPES];
	m_aiMissionaryPoints = new int[MAX_PLAYERS];
	m_aiMissionaryThresholdMultiplier = new int[MAX_PLAYERS];

	m_abYieldEuropeTradable = new bool[NUM_YIELD_TYPES];
	m_abFeatAccomplished = new bool[NUM_FEAT_TYPES];
	m_abOptions = new bool[NUM_PLAYEROPTION_TYPES];

	m_paiImprovementCount = NULL;
	m_paiFreeBuildingCount = NULL;
	m_paiUnitClassCount = NULL;
	m_paiUnitClassMaking = NULL;
	m_paiUnitClassImmigrated = NULL;
	m_paiUnitMoveChange = NULL;
	m_paiUnitStrengthModifier = NULL;
	m_paiProfessionCombatChange = NULL;
	m_paiProfessionMoveChange = NULL;
	m_paiBuildingClassCount = NULL;
	m_paiBuildingClassMaking = NULL;
	m_paiHurryCount = NULL;
	m_paiSpecialBuildingNotRequiredCount = NULL;
	m_aiProfessionEquipmentModifier = NULL;
	m_aiTraitCount = NULL;

	m_paeCivics = NULL;

	m_ppiImprovementYieldChange = NULL;
	m_ppiBuildingYieldChange = NULL;

	reset(NO_PLAYER, true);
}


CvPlayer::~CvPlayer()
{
	uninit();

	SAFE_DELETE_ARRAY(m_aiSeaPlotYield);
	SAFE_DELETE_ARRAY(m_aiYieldRateModifier);
	SAFE_DELETE_ARRAY(m_aiCapitalYieldRateModifier);
	SAFE_DELETE_ARRAY(m_aiBuildingRequiredYieldModifier);
	SAFE_DELETE_ARRAY(m_aiCityExtraYield);
	SAFE_DELETE_ARRAY(m_aiExtraYieldThreshold);
	SAFE_DELETE_ARRAY(m_aiYieldBuyPrice);
	SAFE_DELETE_ARRAY(m_aiYieldTradedTotal);
	SAFE_DELETE_ARRAY(m_aiYieldBoughtTotal);
	SAFE_DELETE_ARRAY(m_aiTaxYieldModifierCount);
	SAFE_DELETE_ARRAY(m_aiMissionaryPoints);
	SAFE_DELETE_ARRAY(m_aiMissionaryThresholdMultiplier);
	SAFE_DELETE_ARRAY(m_abYieldEuropeTradable);
	SAFE_DELETE_ARRAY(m_abFeatAccomplished);
	SAFE_DELETE_ARRAY(m_abOptions);
}


void CvPlayer::init(PlayerTypes eID)
{
	//--------------------------------
	// Init saved data
	reset(eID);

	//assign europe civilization as parent
	for (int iParent = 0; iParent < MAX_PLAYERS; ++iParent)
	{
		CvPlayer& kParent = GET_PLAYER((PlayerTypes) iParent);
		if(kParent.getCivilizationType() != NO_CIVILIZATION)
		{
			if(GC.getCivilizationInfo(kParent.getCivilizationType()).getDerivativeCiv() == getCivilizationType())
			{
				setParent((PlayerTypes) iParent);
				break;
			}
		}
	}

	for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
	{
		YieldTypes eYield = (YieldTypes) iYield;
		CvYieldInfo& kYield = GC.getYieldInfo(eYield);

		FAssert(kYield.getBuyPriceHigh() >= kYield.getBuyPriceLow());

		int iBuyPrice = kYield.getBuyPriceLow() + GC.getGameINLINE().getSorenRandNum(kYield.getBuyPriceHigh() - kYield.getBuyPriceLow() + 1, "Yield Price");
		setYieldBuyPrice(eYield, iBuyPrice, false);
	}

	//--------------------------------
	// Init containers
	m_cities.init();

	m_tradeRoutes.reset();
	m_units.reset();
	freeEuropeUnits();

	m_selectionGroups.init();

	m_eventsTriggered.init();

	m_aDocksNextUnits.clear();

	//--------------------------------
	// Init non-saved data
	setupGraphical();

	//--------------------------------
	// Init other game data
	FAssert(getTeam() != NO_TEAM);
	GET_TEAM(getTeam()).changeNumMembers(1);

	if ((GC.getInitCore().getSlotStatus(getID()) == SS_TAKEN) || (GC.getInitCore().getSlotStatus(getID()) == SS_COMPUTER))
	{
		setAlive(true);

		FAssertMsg((GC.getNumTraitInfos() > 0), "GC.getNumTraitInfos() is less than or equal to zero but is expected to be larger than zero in CvPlayer::init");
		for (int iI = 0; iI < GC.getNumTraitInfos(); iI++)
		{
			if (GC.getCivilizationInfo(getCivilizationType()).hasTrait(iI))
			{
				processTrait((TraitTypes) iI, 1);
			}

			if (GC.getLeaderHeadInfo(getLeaderType()).hasTrait(iI))
			{
				processTrait((TraitTypes) iI, 1);
			}
		}

		for (int iI = 0; iI < NUM_YIELD_TYPES; iI++)
		{
			updateExtraYieldThreshold((YieldTypes)iI);
			updateCityExtraYield((YieldTypes) iI);
		}

		for (int iI = 0; iI < GC.getNumCivicOptionInfos(); iI++)
		{
			setCivic(((CivicOptionTypes)iI), ((CivicTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationInitialCivics(iI))));
		}

		for (int iI = 0; iI < GC.getNumEventInfos(); iI++)
		{
			resetEventOccured((EventTypes)iI, false);
		}

		for (int iI = 0; iI < GC.getNumEventTriggerInfos(); iI++)
		{
			resetTriggerFired((EventTriggerTypes)iI);
		}
	}

	AI_init();
}


void CvPlayer::uninit()
{
	int iI;

	SAFE_DELETE_ARRAY(m_paiImprovementCount);
	SAFE_DELETE_ARRAY(m_paiFreeBuildingCount);
	SAFE_DELETE_ARRAY(m_paiUnitClassCount);
	SAFE_DELETE_ARRAY(m_paiUnitClassMaking);
	SAFE_DELETE_ARRAY(m_paiUnitClassImmigrated);
	SAFE_DELETE_ARRAY(m_paiUnitMoveChange);
	SAFE_DELETE_ARRAY(m_paiUnitStrengthModifier);
	SAFE_DELETE_ARRAY(m_paiProfessionCombatChange);
	SAFE_DELETE_ARRAY(m_paiProfessionMoveChange);
	SAFE_DELETE_ARRAY(m_paiBuildingClassCount);
	SAFE_DELETE_ARRAY(m_paiBuildingClassMaking);
	SAFE_DELETE_ARRAY(m_paiHurryCount);
	SAFE_DELETE_ARRAY(m_paiSpecialBuildingNotRequiredCount);
	SAFE_DELETE_ARRAY(m_aiProfessionEquipmentModifier);
	SAFE_DELETE_ARRAY(m_aiTraitCount);

	SAFE_DELETE_ARRAY(m_paeCivics);

	m_triggersFired.clear();

	if (m_ppiImprovementYieldChange != NULL)
	{
		for (iI = 0; iI < GC.getNumImprovementInfos(); iI++)
		{
			SAFE_DELETE_ARRAY(m_ppiImprovementYieldChange[iI]);
		}
		SAFE_DELETE_ARRAY(m_ppiImprovementYieldChange);
	}
	if (m_ppiBuildingYieldChange != NULL)
	{
		for (iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
		{
			SAFE_DELETE_ARRAY(m_ppiBuildingYieldChange[iI]);
		}
		SAFE_DELETE_ARRAY(m_ppiBuildingYieldChange);
	}

	m_groupCycle.clear();
	m_aszCityNames.clear();

	m_cities.uninit();

	m_tradeRoutes.reset();
	m_units.reset();
	freeEuropeUnits();

	m_selectionGroups.uninit();

	m_eventsTriggered.uninit();

	clearMessages();

	clearPopups();

	clearDiplomacy();
}


// FUNCTION: reset()
// Initializes data members that are serialized.
void CvPlayer::reset(PlayerTypes eID, bool bConstructorCall)
{
	int iI, iJ;

	//--------------------------------
	// Uninit class
	uninit();

	m_iStartingX = INVALID_PLOT_COORD;
	m_iStartingY = INVALID_PLOT_COORD;
	m_iTotalPopulation = 0;
	m_iTotalLand = 0;
	m_iTotalLandScored = 0;
	m_iGold = 0;
	m_iAdvancedStartPoints = -1;
	m_iGreatGeneralsCreated = 0;
	m_iGreatGeneralsThresholdModifier = 0;
	m_iGreatGeneralRateModifier = 0;
	m_iDomesticGreatGeneralRateModifier = 0;
	m_iImmigrationThresholdMultiplier = 100;
	m_iRevolutionEuropeUnitThresholdMultiplier = 100;
	m_iKingNumUnitMultiplier = 100;
	m_iNativeAngerModifier = 0;
	m_iFreeExperience = 0;
	m_iWorkerSpeedModifier = 0;
	m_iImprovementUpgradeRateModifier = 0;
	m_iMilitaryProductionModifier = 0;
	m_iCityDefenseModifier = 0;
	m_iHighestUnitLevel = 1;
	m_iFatherOverflowBells = 0;
	m_iExpInBorderModifier = 0;
	m_iLevelExperienceModifier = 0;
	m_iCapitalCityID = FFreeList::INVALID_INDEX;
	m_iCitiesLost = 0;
	m_iAssets = 0;
	m_iPower = 0;
	m_iPopulationScore = 0;
	m_iLandScore = 0;
	m_iFatherScore = 0;
	m_iCombatExperience = 0;
	m_iPopRushHurryCount = 0;
	m_iCrossesStored = 0;
	m_iBellsStored = 0;
	m_iTaxRate = 0;
	m_iNativeCombatModifier = 0;
	m_iDominateNativeBordersCount = 0;
	m_iRevolutionEuropeTradeCount = 0;
	m_iFatherPointMultiplier = 100;
	m_iMissionaryRateModifier = 0;
	m_iMissionarySuccessPercent = 100;
	m_uiStartTime = 0;

	m_bAlive = false;
	m_bEverAlive = false;
	m_bTurnActive = false;
	m_bAutoMoves = false;
	m_bEndTurn = false;
	m_bPbemNewTurn = false;
	m_bExtendedGame = false;
	m_bFoundedFirstCity = false;
	m_bStrike = false;

	m_eID = eID;
	updateTeamType();
	updateHuman();

	if (m_eID != NO_PLAYER)
	{
		m_ePersonalityType = GC.getInitCore().getLeader(m_eID); //??? Is this repeated data???
	}
	else
	{
		m_ePersonalityType = NO_LEADER;
	}
	m_eCurrentEra = ((EraTypes)0);  //??? Is this repeated data???
	m_eParent = NO_PLAYER;
	m_eImmigrationConversion = YIELD_CROSSES;

	for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		m_aiSeaPlotYield[iI] = 0;
		m_aiYieldRateModifier[iI] = 0;
		m_aiCapitalYieldRateModifier[iI] = 0;
		m_aiBuildingRequiredYieldModifier[iI] = 0;
		m_aiCityExtraYield[iI] = 0;
		m_aiExtraYieldThreshold[iI] = 0;
		m_aiYieldBuyPrice[iI] = 0;
		m_aiYieldTradedTotal[iI] = 0;
		m_aiYieldBoughtTotal[iI] = 0;
		m_abYieldEuropeTradable[iI] = true;
		m_aiTaxYieldModifierCount[iI] = 0;
	}

	for (iI = 0; iI < MAX_PLAYERS; ++iI)
	{
		m_aiMissionaryPoints[iI] = 0;
		m_aiMissionaryThresholdMultiplier[iI] = 100;
	}

	for (iI = 0; iI < NUM_FEAT_TYPES; iI++)
	{
		m_abFeatAccomplished[iI] = false;
	}

	for (iI = 0; iI < NUM_PLAYEROPTION_TYPES; iI++)
	{
		m_abOptions[iI] = false;
	}

	m_szScriptData = "";

	if (!bConstructorCall)
	{
		FAssertMsg(0 < GC.getNumImprovementInfos(), "GC.getNumImprovementInfos() is not greater than zero but it is used to allocate memory in CvPlayer::reset");
		FAssertMsg(m_paiImprovementCount==NULL, "about to leak memory, CvPlayer::m_paiImprovementCount");
		m_paiImprovementCount = new int [GC.getNumImprovementInfos()];
		for (iI = 0; iI < GC.getNumImprovementInfos(); iI++)
		{
			m_paiImprovementCount[iI] = 0;
		}

		FAssertMsg(m_paiFreeBuildingCount==NULL, "about to leak memory, CvPlayer::m_paiFreeBuildingCount");
		m_paiFreeBuildingCount = new int [GC.getNumBuildingInfos()];
		for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
		{
			m_paiFreeBuildingCount[iI] = 0;
		}

		FAssertMsg(m_paiUnitClassCount==NULL, "about to leak memory, CvPlayer::m_paiUnitClassCount");
		m_paiUnitClassCount = new int [GC.getNumUnitClassInfos()];
		FAssertMsg(m_paiUnitClassMaking==NULL, "about to leak memory, CvPlayer::m_paiUnitClassMaking");
		m_paiUnitClassMaking = new int [GC.getNumUnitClassInfos()];
		FAssertMsg(m_paiUnitClassImmigrated==NULL, "about to leak memory, CvPlayer::m_paiUnitClassImmigrated");
		m_paiUnitClassImmigrated = new int [GC.getNumUnitClassInfos()];
		FAssertMsg(m_paiUnitMoveChange==NULL, "about to leak memory, CvPlayer::m_paiUnitMoveChange");
		m_paiUnitMoveChange = new int [GC.getNumUnitClassInfos()];
		FAssertMsg(m_paiUnitStrengthModifier==NULL, "about to leak memory, CvPlayer::m_paiUnitStrengthModifier");
		m_paiUnitStrengthModifier = new int [GC.getNumUnitClassInfos()];
		for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
		{
			m_paiUnitClassCount[iI] = 0;
			m_paiUnitClassMaking[iI] = 0;
			m_paiUnitClassImmigrated[iI] = 0;
			m_paiUnitMoveChange[iI] = 0;
			m_paiUnitStrengthModifier[iI] = 0;
		}

		FAssertMsg(m_paiProfessionCombatChange==NULL, "about to leak memory, CvPlayer::m_paiProfessionMoveChange");
		m_paiProfessionCombatChange = new int [GC.getNumProfessionInfos()];
		FAssertMsg(m_paiProfessionMoveChange==NULL, "about to leak memory, CvPlayer::m_paiProfessionMoveChange");
		m_paiProfessionMoveChange = new int [GC.getNumProfessionInfos()];
		for (iI = 0; iI < GC.getNumProfessionInfos(); iI++)
		{
			m_paiProfessionCombatChange[iI] = 0;
			m_paiProfessionMoveChange[iI] = 0;
		}

		FAssertMsg(m_paiBuildingClassCount==NULL, "about to leak memory, CvPlayer::m_paiBuildingClassCount");
		m_paiBuildingClassCount = new int [GC.getNumBuildingClassInfos()];
		FAssertMsg(m_paiBuildingClassMaking==NULL, "about to leak memory, CvPlayer::m_paiBuildingClassMaking");
		m_paiBuildingClassMaking = new int [GC.getNumBuildingClassInfos()];
		for (iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
		{
			m_paiBuildingClassCount[iI] = 0;
			m_paiBuildingClassMaking[iI] = 0;
		}

		FAssertMsg(m_paiHurryCount==NULL, "about to leak memory, CvPlayer::m_paiHurryCount");
		m_paiHurryCount = new int [GC.getNumHurryInfos()];
		for (iI = 0; iI < GC.getNumHurryInfos(); iI++)
		{
			m_paiHurryCount[iI] = GC.getHurryInfo((HurryTypes) iI).isStarting() ? 1 : 0;
		}

		FAssertMsg(m_paiSpecialBuildingNotRequiredCount==NULL, "about to leak memory, CvPlayer::m_paiSpecialBuildingNotRequiredCount");
		m_paiSpecialBuildingNotRequiredCount = new int [GC.getNumSpecialBuildingInfos()];
		for (iI = 0; iI < GC.getNumSpecialBuildingInfos(); iI++)
		{
			m_paiSpecialBuildingNotRequiredCount[iI] = 0;
		}
	
		FAssertMsg(m_paeCivics==NULL, "about to leak memory, CvPlayer::m_paeCivics");
		m_paeCivics = new CivicTypes [GC.getNumCivicOptionInfos()];
		for (iI = 0; iI < GC.getNumCivicOptionInfos(); iI++)
		{
			m_paeCivics[iI] = NO_CIVIC;
		}

		FAssertMsg(m_aiProfessionEquipmentModifier==NULL, "about to leak memory, CvPlayer::m_aiProfessionEquipmentModifier");
		m_aiProfessionEquipmentModifier = new int[GC.getNumProfessionInfos()];
		for (iI = 0; iI < GC.getNumProfessionInfos(); iI++)
		{
			m_aiProfessionEquipmentModifier[iI] = 0;
		}

		FAssertMsg(m_aiTraitCount==NULL, "about to leak memory, CvPlayer::m_aiTraitCount");
		m_aiTraitCount = new int[GC.getNumTraitInfos()];
		for (iI = 0; iI < GC.getNumTraitInfos(); iI++)
		{
			m_aiTraitCount[iI] = 0;
		}

		FAssertMsg(m_ppiImprovementYieldChange==NULL, "about to leak memory, CvPlayer::m_ppiImprovementYieldChange");
		m_ppiImprovementYieldChange = new int*[GC.getNumImprovementInfos()];
		for (iI = 0; iI < GC.getNumImprovementInfos(); iI++)
		{
			m_ppiImprovementYieldChange[iI] = new int[NUM_YIELD_TYPES];
			for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
			{
				m_ppiImprovementYieldChange[iI][iJ] = 0;
			}
		}

		FAssertMsg(m_ppiBuildingYieldChange==NULL, "about to leak memory, CvPlayer::m_ppiBuildingYieldChange");
		m_ppiBuildingYieldChange = new int*[GC.getNumBuildingClassInfos()];
		for (iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
		{
			m_ppiBuildingYieldChange[iI] = new int[NUM_YIELD_TYPES];
			for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
			{
				m_ppiBuildingYieldChange[iI][iJ] = 0;
			}
		}

		m_mapEventsOccured.clear();
		m_mapEventCountdown.clear();
		m_aFreeUnitCombatPromotions.clear();
		m_aFreeUnitClassPromotions.clear();
		m_aEuropeRevolutionUnits.clear();
		m_triggersFired.clear();
		m_aDocksNextUnits.clear();
	}

	m_cities.removeAll();

	m_tradeRoutes.reset();
	m_units.reset();
	freeEuropeUnits();

	m_selectionGroups.removeAll();

	m_eventsTriggered.removeAll();
	m_aszTradeMessages.clear();

	if (!bConstructorCall)
	{
		AI_reset();
	}
}


//////////////////////////////////////
// graphical only setup
//////////////////////////////////////
void CvPlayer::setupGraphical()
{
	if (!GC.IsGraphicsInitialized())
		return;

	CvCity* pLoopCity;
	CvUnit* pLoopUnit;

	// Setup m_cities
	int iLoop;
	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		pLoopCity->setupGraphical();
	}

	// Setup m_units
	for(pLoopUnit = firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = nextUnit(&iLoop))
	{
		if (pLoopUnit->plot() != NULL)
		{
			pLoopUnit->setupGraphical();
		}
	}
}


void CvPlayer::initFreeState()
{
	setGold(0);
	changeGold(GC.getHandicapInfo(getHandicapType()).getStartingGold());
	changeGold(GC.getEraInfo(GC.getGameINLINE().getStartEra()).getStartingGold());
}


void CvPlayer::initFreeUnits()
{
	UnitTypes eLoopUnit;
	int iFreeCount;

	if ((GC.getGameINLINE().isOption(GAMEOPTION_ADVANCED_START) || GC.getCivilizationInfo(getCivilizationType()).getAdvancedStartPoints() > 0) && !GC.getCivilizationInfo(getCivilizationType()).isEurope())
	{
		int iPoints = GC.getCivilizationInfo(getCivilizationType()).getAdvancedStartPoints();

		if (NO_WORLDSIZE != GC.getInitCore().getWorldSize())
		{
			iPoints *= GC.getWorldInfo(GC.getInitCore().getWorldSize()).getAdvancedStartPointsMod();
			iPoints /= 100;
		}

		if (NO_GAMESPEED != GC.getInitCore().getGameSpeed())
		{
			iPoints *= GC.getGameSpeedInfo(GC.getInitCore().getGameSpeed()).getGrowthPercent();
			iPoints /= 100;
		}

		if (GC.getGameINLINE().isOption(GAMEOPTION_ADVANCED_START))
		{
			iPoints += GC.getGameINLINE().getNumAdvancedStartPoints();
		}

		iPoints *= GC.getHandicapInfo(getHandicapType()).getAdvancedStartPointsMod();
		iPoints /= 100;

		if (!isHuman())
		{
			iPoints *= GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIAdvancedStartPercent();
			iPoints /= 100;
		}

		setAdvancedStartPoints(iPoints);

		// Starting visibility
		CvPlot* pStartingPlot = getStartingPlot();
		if (NULL != pStartingPlot)
		{
			for (int iPlotLoop = 0; iPlotLoop < GC.getMapINLINE().numPlots(); ++iPlotLoop)
			{
				CvPlot* pPlot = GC.getMapINLINE().plotByIndex(iPlotLoop);

				if (plotDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE()) <= GC.getDefineINT("ADVANCED_START_SIGHT_RANGE"))
				{
					pPlot->setRevealed(getTeam(), true, false, NO_TEAM);
				}
			}
		}

		if (!GC.getGameINLINE().isOption(GAMEOPTION_ADVANCED_START) && !isHuman() && (getAdvancedStartPoints() >= 0))
		{
			AI_doAdvancedStart();
		}
	}
	else
	{
		CvCivilizationInfo& kCivilizationInfo = GC.getCivilizationInfo(getCivilizationType());
		for (int iI = 0; iI < kCivilizationInfo.getNumCivilizationFreeUnits(); iI++)
		{
			int iLoopUnitClass = kCivilizationInfo.getCivilizationFreeUnitsClass(iI);
			ProfessionTypes eLoopUnitProfession = (ProfessionTypes) kCivilizationInfo.getCivilizationFreeUnitsProfession(iI);
			eLoopUnit = (UnitTypes)kCivilizationInfo.getCivilizationUnits(iLoopUnitClass);

			if (eLoopUnit != NO_UNIT)
			{
				iFreeCount = (GC.getEraInfo(GC.getGameINLINE().getStartEra()).getStartingUnitMultiplier() + ((!isHuman()) ? GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIStartingUnitMultiplier() : 0));

				for (int iJ = 0; iJ < iFreeCount; iJ++)
				{
					//add revolution units to children
					if (GC.getCivilizationInfo(getCivilizationType()).isEurope())
					{
						for (int iPlayer=0;iPlayer<MAX_PLAYERS;iPlayer++)
						{
							CvPlayer& kChild = GET_PLAYER((PlayerTypes) iPlayer);
							if(kChild.isAlive() && (kChild.getParent() == getID()))
							{
								kChild.addRevolutionEuropeUnit(eLoopUnit, eLoopUnitProfession);
							}
						}
					}
					else
					{
						CvUnit* pUnit = addFreeUnit(eLoopUnit, eLoopUnitProfession);
					}
				}
			}
		}

		iFreeCount = GC.getEraInfo(GC.getGameINLINE().getStartEra()).getStartingDefenseUnits();
		iFreeCount += GC.getHandicapInfo(getHandicapType()).getStartingDefenseUnits();

		if (!isHuman())
		{
			iFreeCount += GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIStartingDefenseUnits();
		}

		if (iFreeCount > 0)
		{
			addFreeUnitAI(UNITAI_DEFENSIVE, iFreeCount);
		}

		iFreeCount = GC.getEraInfo(GC.getGameINLINE().getStartEra()).getStartingWorkerUnits();
		iFreeCount += GC.getHandicapInfo(getHandicapType()).getStartingWorkerUnits();

		if (!isHuman())
		{
			iFreeCount += GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIStartingWorkerUnits();
		}

		if (iFreeCount > 0)
		{
			addFreeUnitAI(UNITAI_SETTLER, iFreeCount);
		}

		iFreeCount = GC.getEraInfo(GC.getGameINLINE().getStartEra()).getStartingExploreUnits();
		iFreeCount += GC.getHandicapInfo(getHandicapType()).getStartingExploreUnits();

		if (!isHuman())
		{
			iFreeCount += GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIStartingExploreUnits();
		}

		if (iFreeCount > 0)
		{
			addFreeUnitAI(UNITAI_SCOUT, iFreeCount);
		}
	}
}

void CvPlayer::initImmigration()
{
	FAssert(getParent() != NO_PLAYER);
	m_aDocksNextUnits.clear();
	for (int i = 0; i < GC.getDefineINT("DOCKS_NEXT_UNITS"); ++i)
	{
		m_aDocksNextUnits.push_back(pickBestImmigrant());
	}
}

void CvPlayer::addFreeUnitAI(UnitAITypes eUnitAI, int iCount)
{
	UnitTypes eBestUnit = NO_UNIT;
	ProfessionTypes eBestProfession = NO_PROFESSION;
	int iBestValue = 0;

	for (int iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
	{
		UnitTypes eLoopUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI);

		if (eLoopUnit != NO_UNIT)
		{
			if (canTrain(eLoopUnit))
			{
				int iValue = AI_unitValue(eLoopUnit, eUnitAI, NULL);

				if (iValue > iBestValue)
				{
					eBestUnit = eLoopUnit;
					iBestValue = iValue;
					eBestProfession = (ProfessionTypes) GC.getUnitInfo(eLoopUnit).getDefaultProfession();
				}
			}
		}
	}

	if (eBestUnit != NO_UNIT)
	{
		for (int iI = 0; iI < iCount; iI++)
		{
			addFreeUnit(eBestUnit, eBestProfession, eUnitAI);
		}
	}
}


CvUnit* CvPlayer::addFreeUnit(UnitTypes eUnit, ProfessionTypes eProfession, UnitAITypes eUnitAI)
{
	CvPlot* pStartingPlot = getStartingPlot();
	if (pStartingPlot != NULL)
	{
		CvUnit* pUnit = initUnit(eUnit, eProfession, pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), eUnitAI);
		return pUnit;
	}

	return NULL;
}


int CvPlayer::startingPlotRange() const
{
	int iRange;

	iRange = (GC.getMapINLINE().maxStepDistance() + 40);

	iRange *= GC.getDefineINT("STARTING_DISTANCE_PERCENT");
	iRange /= 100;

	iRange *= (GC.getMapINLINE().getLandPlots() / (GC.getWorldInfo(GC.getMapINLINE().getWorldSize()).getTargetNumCities() * GC.getGameINLINE().countCivPlayersAlive()));
	iRange /= NUM_CITY_PLOTS;

	iRange += std::min(((GC.getMapINLINE().getNumAreas() + 1) / 2), GC.getGameINLINE().countCivPlayersAlive());

	long lResult=0;
	if (gDLL->getPythonIFace()->callFunction(gDLL->getPythonIFace()->getMapScriptModule(), "minStartingDistanceModifier", NULL, &lResult))
	{
		iRange *= std::max<int>(0, (lResult + 100));
		iRange /= 100;
	}

	return std::max(iRange, GC.getDefineINT("MIN_CIV_STARTING_DISTANCE"));
}


int CvPlayer::startingPlotDistanceFactor(CvPlot* pPlot, PlayerTypes ePlayer, int iRange) const
{
	PROFILE_FUNC();
	
	FAssert(ePlayer != getID());

	CvPlot* pStartingPlot;
	
	int iValue = 1000;

	pStartingPlot = getStartingPlot();

	if (pStartingPlot != NULL)
	{
		if (GC.getGameINLINE().isTeamGame())
		{
			if (GET_PLAYER(ePlayer).getTeam() == getTeam())
			{
				iRange *= GC.getDefineINT("OWN_TEAM_STARTING_MODIFIER");
				iRange /= 100;
			}
			else
			{
				iRange *= GC.getDefineINT("RIVAL_TEAM_STARTING_MODIFIER");
				iRange /= 100;
			}
		}

		int iDistance = stepDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE());
		if (pStartingPlot->getArea() != pPlot->getArea())
		{
			iDistance *= 4;            	
			iDistance /= 3;
		}

		iValue *= iDistance;
		iValue /= iRange ;

	}
    
	return std::max(1, iValue);

}


// Returns the id of the best area, or -1 if it doesn't matter:
int CvPlayer::findStartingArea() const
{
	PROFILE_FUNC();

	long result = -1;
	if (gDLL->getPythonIFace()->pythonFindStartingArea(getID(), &result) && !gDLL->getPythonIFace()->pythonUsingDefaultImpl()) // Python override
	{
		if (result == -1 || GC.getMapINLINE().getArea(result) != NULL)
		{
			return result;
		}
		else
		{
			FAssertMsg(false, "python findStartingArea() must return -1 or the ID of a valid area");
		}
	}

	//not useful for water starts
	if(GC.getCivilizationInfo(getCivilizationType()).isWaterStart())
	{
		return -1;
	}

	int iBestValue = 0;
	int iBestArea = -1;
	int iValue;
	int iLoop = 0;

	CvArea *pLoopArea = NULL;

	// find best land area
	for(pLoopArea = GC.getMapINLINE().firstArea(&iLoop); pLoopArea != NULL; pLoopArea = GC.getMapINLINE().nextArea(&iLoop))
	{
		if (!(pLoopArea->isWater()))
		{
			// iNumPlayersOnArea is the number of players starting on the area, plus this player
			int iNumPlayersOnArea = (pLoopArea->getNumStartingPlots() + 1);
			int iTileValue = ((pLoopArea->calculateTotalBestNatureYield() + (pLoopArea->countCoastalLand() * 2) + pLoopArea->getNumRiverEdges() + (pLoopArea->getNumTiles())) + 1);
			iValue = iTileValue / iNumPlayersOnArea;

			iValue *= std::min(NUM_CITY_PLOTS + 1, pLoopArea->getNumTiles() + 1);
			iValue /= (NUM_CITY_PLOTS + 1);

			if (iNumPlayersOnArea <= 2)
			{
				iValue *= 4;
				iValue /= 3;
			}

			if (iValue > iBestValue)
			{
				iBestValue = iValue;
				iBestArea = pLoopArea->getID();
			}
		}
	}

	return iBestArea;
}


CvPlot* CvPlayer::findStartingPlot(bool bRandomize)
{
	PROFILE_FUNC();

	long result = -1;
	if (gDLL->getPythonIFace()->pythonFindStartingPlot(getID(), &result) && !gDLL->getPythonIFace()->pythonUsingDefaultImpl()) // Python override
	{
		CvPlot *pPlot = GC.getMapINLINE().plotByIndexINLINE(result);
		if (pPlot != NULL)
		{
			return pPlot;
		}
		else
		{
			FAssertMsg(false, "python findStartingPlot() returned an invalid plot index!");
		}
	}

	CvPlot* pLoopPlot;
	int iBestArea = -1;
	int iI;

	bool bNew = false;
	if (getStartingPlot() != NULL)
	{
		iBestArea = getStartingPlot()->getArea();
		setStartingPlot(NULL, true);
		bNew = true;
	}

	AI_updateFoundValues(true);//this sets all plots found values to -1
	
	if (!bNew)
	{
		iBestArea = findStartingArea();
	}

	//flood fill land distances O(numPlots)
	std::vector<short> aLandDistances(GC.getMapINLINE().numPlotsINLINE(), MAX_SHORT);

	{
		PROFILE("CvPlayer::findStartingPlot::landDistances");

		//initialize
		std::deque<int> aFillQueue;
		for(int i=0;i<GC.getMapINLINE().numPlotsINLINE();i++)
		{
			CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(i);
			if(!pLoopPlot->isWater())
			{
				aLandDistances[i] = 0;
				aFillQueue.push_back(i);
			}
		}

		//fill
		while(!aFillQueue.empty())
		{
			int iLoopIndex = aFillQueue.front();
			aFillQueue.pop_front();
			CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iLoopIndex);
			int iLoopDistance = aLandDistances[iLoopIndex] + 1;
			for(int iDirection=0;iDirection<NUM_DIRECTION_TYPES;iDirection++)
			{
				CvPlot* pAdjacentPlot = plotDirection(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), (DirectionTypes) iDirection);
				if(pAdjacentPlot != NULL)
				{
					int iAdjacentIndex = GC.getMapINLINE().plotNumINLINE(pAdjacentPlot->getX_INLINE(), pAdjacentPlot->getY_INLINE());
					if(iLoopDistance < aLandDistances[iAdjacentIndex])
					{
						aLandDistances[iAdjacentIndex] = iLoopDistance;
						aFillQueue.push_back(iAdjacentIndex);
					}
				}
			}
		}
	}

	bool bStartOnWater = GC.getCivilizationInfo(getCivilizationType()).isWaterStart();
	std::deque<bool> abEuropeStart;
	if (bStartOnWater)
	{
		for (int i = 0; i < GC.getNumEuropeInfos(); ++i)
		{
			abEuropeStart.push_back(GC.getEuropeInfo((EuropeTypes) i).isStart());
			gDLL->getPythonIFace()->pythonIsEuropeStart((EuropeTypes) i, &abEuropeStart[i]);
		}
	}

	for(int iPass = 0; iPass < 2; iPass++)
	{
		int iBestValue = 0;
		int iBestIndex = -1;
		
		std::vector<int> aiWeights(GC.getMapINLINE().numPlotsINLINE(), 0);
		for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
		{
			int iValue = 0;
			pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

			if ((iBestArea == -1) || (pLoopPlot->getArea() == iBestArea) || (iPass > 0))
			{
				if (bStartOnWater)
				{
					EuropeTypes eEurope = pLoopPlot->getEurope();
					if (eEurope != NO_EUROPE && abEuropeStart[eEurope])
					{
						iValue = 1;
						switch ((CardinalDirectionTypes)GC.getEuropeInfo(eEurope).getCardinalDirection())
						{
						case CARDINALDIRECTION_EAST:
						case CARDINALDIRECTION_WEST:
							iValue *= 1 + std::min(::plotDistance(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), pLoopPlot->getX_INLINE(), 0), startingPlotRange());
							iValue *= 1 + std::min(::plotDistance(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), pLoopPlot->getX_INLINE(), GC.getMapINLINE().getGridHeightINLINE() - 1), startingPlotRange() / 2);
							break;
						case CARDINALDIRECTION_NORTH:
						case CARDINALDIRECTION_SOUTH:
							iValue *= 1 + std::min(::plotDistance(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), 0, pLoopPlot->getY_INLINE()), startingPlotRange());
							iValue *= 1 + std::min(::plotDistance(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), GC.getMapINLINE().getGridWidthINLINE() - 1, pLoopPlot->getY_INLINE()), startingPlotRange() / 2);
							break;
						default:
							break;
						}
						int iMinPlotDistance = startingPlotRange();
						for (int iPlayer = 0; iPlayer < MAX_PLAYERS; iPlayer++)
						{
							CvPlayer& otherPlayer = GET_PLAYER((PlayerTypes) iPlayer);
							if ((iPlayer != getID()) && otherPlayer.isAlive())
							{
								CvPlot* pOtherPlot = otherPlayer.getStartingPlot();
								if(pOtherPlot != NULL)
								{
									if(GC.getCivilizationInfo(otherPlayer.getCivilizationType()).isWaterStart())
									{
										int iPlotDistance = plotDistance(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), pOtherPlot->getX_INLINE(), pOtherPlot->getY_INLINE());
										if (iPlotDistance < iMinPlotDistance)
										{
											iMinPlotDistance = iPlotDistance;
										}
									}
								}
							}
						}
						iValue *= 1 + iMinPlotDistance;

						iValue /= (aLandDistances[iI] + 1);
					}
				}
				else
				{
					//the distance factor is now done inside foundValue
					iValue = pLoopPlot->getFoundValue(getID());
				}

				if (iValue > iBestValue)
				{
					iBestValue = iValue;
					iBestIndex = iI;
				}
			}

			FAssert(iValue >= 0);
			aiWeights[iI] = std::max(0, iValue);
		}

		FAssert(std::accumulate(aiWeights.begin(), aiWeights.end(), 0) > 0);

		if (bRandomize)
		{
			iBestIndex = GC.getGameINLINE().getSorenRand().pickValue(aiWeights, "Randomizing start");
		}

		return GC.getMapINLINE().plotByIndexINLINE(iBestIndex);

		FAssertMsg(iPass != 0, "CvPlayer::findStartingPlot - could not find starting plot in first pass.");
	}

	FAssertMsg(false, "Could not find starting plot.");
	return NULL;
}


CvCity* CvPlayer::initCity(int iX, int iY, bool bBumpUnits)
{
	PROFILE_FUNC();

	CvCity* pCity = addCity();

	FAssertMsg(pCity != NULL, "City is not assigned a valid value");
	FAssertMsg(!(GC.getMapINLINE().plotINLINE(iX, iY)->isCity()), "No city is expected at this plot when initializing new city");

	pCity->init(pCity->getID(), getID(), iX, iY, bBumpUnits);

	return pCity;
}


void CvPlayer::acquireCity(CvCity* pOldCity, bool bConquest, bool bTrade)
{
	CLLNode<IDInfo>* pUnitNode;
	CvCity* pNewCity;
	CvUnit* pLoopUnit;
	CvPlot* pCityPlot;
	CvPlot* pLoopPlot;
	bool* pabHasRealBuilding;
	int* paiBuildingOriginalOwner;
	int* paiBuildingOriginalTime;
	CvWString szBuffer;
	CvWString szName;
	bool abEverOwned[MAX_PLAYERS];
	int aiCulture[MAX_PLAYERS];
	PlayerTypes eOldOwner;
	PlayerTypes eOriginalOwner;
	PlayerTypes eHighestCulturePlayer;
	BuildingTypes eBuilding;
	bool bForceUnowned;
	bool bRecapture;
	bool bRaze;
	bool bGift;
	int iRange;
	int iCaptureGold;
	int iGameTurnFounded;
	int iPopulation;
	int iHighestPopulation;
	int iOccupationTimer;
	int iTeamCulturePercent;
	int iDamage;
	int iDX, iDY;
	int iI;
	CLinkList<IDInfo> oldUnits;

	pCityPlot = pOldCity->plot();

	pUnitNode = pCityPlot->headUnitNode();

	while (pUnitNode != NULL)
	{
		oldUnits.insertAtEnd(pUnitNode->m_data);
		pUnitNode = pCityPlot->nextUnitNode(pUnitNode);
	}

	pUnitNode = oldUnits.head();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = oldUnits.next(pUnitNode);

		if (pLoopUnit && pLoopUnit->getTeam() != getTeam())
		{
			if (pLoopUnit->getDomainType() == DOMAIN_IMMOBILE)
			{
				pLoopUnit->kill(false);
			}
		}
	}

	if (bConquest)
	{
		iRange = pOldCity->getCultureLevel();

		for (iDX = -(iRange); iDX <= iRange; iDX++)
		{
			for (iDY = -(iRange); iDY <= iRange; iDY++)
			{
				if (pOldCity->cultureDistance(iDX, iDY) <= iRange)
				{
					pLoopPlot = plotXY(pOldCity->getX_INLINE(),pOldCity-> getY_INLINE(), iDX, iDY);

					if (pLoopPlot != NULL)
					{
						if (pLoopPlot->getOwnerINLINE() == pOldCity->getOwnerINLINE())
						{
							if (pLoopPlot->getNumCultureRangeCities(pOldCity->getOwnerINLINE()) == 1)
							{
								bForceUnowned = false;

								for (iI = 0; iI < MAX_PLAYERS; iI++)
								{
									if (GET_PLAYER((PlayerTypes)iI).isAlive())
									{
										if ((GET_PLAYER((PlayerTypes)iI).getTeam() != getTeam()) && (GET_PLAYER((PlayerTypes)iI).getTeam() != pOldCity->getTeam()))
										{
											if (pLoopPlot->getNumCultureRangeCities((PlayerTypes)iI) > 0)
											{
												bForceUnowned = true;
												break;
											}
										}
									}
								}

								if (bForceUnowned)
								{
									pLoopPlot->setForceUnownedTimer(2); // XXX
								}
							}
						}
					}
				}
			}
		}
	}

	if (pOldCity->getOriginalOwner() == pOldCity->getOwnerINLINE())
	{
		GET_PLAYER(pOldCity->getOriginalOwner()).changeCitiesLost(1);
	}
	else if (pOldCity->getOriginalOwner() == getID())
	{
		GET_PLAYER(pOldCity->getOriginalOwner()).changeCitiesLost(-1);
	}

	if (bConquest)
	{
		szBuffer = gDLL->getText("TXT_KEY_MISC_CAPTURED_CITY", pOldCity->getNameKey()).GetCString();
		gDLL->getInterfaceIFace()->addMessage(getID(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_CITYCAPTURE", MESSAGE_TYPE_MAJOR_EVENT, ARTFILEMGR.getInterfaceArtInfo("WORLDBUILDER_CITY_EDIT")->getPath(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pOldCity->getX_INLINE(), pOldCity->getY_INLINE(), true, true);

		szName.Format(L"%s (%s)", pOldCity->getName().GetCString(), GET_PLAYER(pOldCity->getOwnerINLINE()).getName());

		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if (iI != getID())
				{
					if (pOldCity->isRevealed(GET_PLAYER((PlayerTypes)iI).getTeam(), false))
					{
						szBuffer = gDLL->getText("TXT_KEY_MISC_CITY_CAPTURED_BY", szName.GetCString(), getCivilizationDescriptionKey());
						gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_CITYCAPTURED", MESSAGE_TYPE_MAJOR_EVENT, ARTFILEMGR.getInterfaceArtInfo("WORLDBUILDER_CITY_EDIT")->getPath(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pOldCity->getX_INLINE(), pOldCity->getY_INLINE(), true, true);
					}
				}
			}
		}

		szBuffer = gDLL->getText("TXT_KEY_MISC_CITY_WAS_CAPTURED_BY", szName.GetCString(), getCivilizationDescriptionKey());
		GC.getGameINLINE().addReplayMessage(REPLAY_MESSAGE_MAJOR_EVENT, getID(), szBuffer, pOldCity->getX_INLINE(), pOldCity->getY_INLINE(), (ColorTypes)GC.getInfoTypeForString("COLOR_WARNING_TEXT"));
	}

	iCaptureGold = 0;

	if (bConquest)
	{
		long lCaptureGold;
		// Use python to determine city capture gold amounts...
		lCaptureGold = 0;

		CyCity* pyOldCity = new CyCity(pOldCity);

		CyArgsList argsList;
		argsList.add(gDLL->getPythonIFace()->makePythonObject(pyOldCity));	// pass in plot class
		
		gDLL->getPythonIFace()->callFunction(PYGameModule, "doCityCaptureGold", argsList.makeFunctionArgs(),&lCaptureGold);
		delete pyOldCity;	// python fxn must not hold on to this pointer

		iCaptureGold = (int)lCaptureGold;
	}

	changeGold(iCaptureGold);

	pabHasRealBuilding = new bool[GC.getNumBuildingInfos()];
	paiBuildingOriginalOwner = new int[GC.getNumBuildingInfos()];
	paiBuildingOriginalTime = new int[GC.getNumBuildingInfos()];

	eOldOwner = pOldCity->getOwnerINLINE();
	eOriginalOwner = pOldCity->getOriginalOwner();
	PlayerTypes eOldPreviousOwner = pOldCity->getPreviousOwner();
	eHighestCulturePlayer = pOldCity->findHighestCulture();
	iGameTurnFounded = pOldCity->getGameTurnFounded();
	iPopulation = pOldCity->getPopulation();
	iHighestPopulation = pOldCity->getHighestPopulation();
	iOccupationTimer = pOldCity->getOccupationTimer();
	szName = pOldCity->getNameKey();
	iDamage = pOldCity->getDefenseDamage();
	int iOldCityId = pOldCity->getID();
	UnitClassTypes eTeachUnitClass = pOldCity->getTeachUnitClass();
	int iEducationThresholdMultiplier = pOldCity->getEducationThresholdMultiplier();

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		abEverOwned[iI] = pOldCity->isEverOwned((PlayerTypes)iI);
		aiCulture[iI] = pOldCity->getCulture((PlayerTypes)iI);
	}

	abEverOwned[getID()] = true;

	for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
	{
		pabHasRealBuilding[iI] = pOldCity->isHasRealBuilding((BuildingTypes)iI);
		paiBuildingOriginalOwner[iI] = pOldCity->getBuildingOriginalOwner((BuildingTypes)iI);
		paiBuildingOriginalTime[iI] = pOldCity->getBuildingOriginalTime((BuildingTypes)iI);
	}

	std::vector<BuildingYieldChange> aBuildingYieldChange;
	for (iI = 0; iI < GC.getNumBuildingClassInfos(); ++iI)
	{
		for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
		{
			BuildingYieldChange kChange;
			kChange.eBuildingClass = (BuildingClassTypes)iI;
			kChange.eYield = (YieldTypes)iYield;
			kChange.iChange = pOldCity->getBuildingYieldChange((BuildingClassTypes)iI, (YieldTypes)iYield);
			if (0 != kChange.iChange)
			{
				aBuildingYieldChange.push_back(kChange);
			}
		}
	}

	int aiYieldsStored[NUM_YIELD_TYPES];
	for (iI = 0; iI < NUM_YIELD_TYPES; ++iI)
	{
		aiYieldsStored[iI] = pOldCity->getYieldStored((YieldTypes) iI);
	}


	//capture all population units
	std::vector<CvUnit *> aOldPopulationUnits;
	for (int i = 0; i < iPopulation; ++i)
	{
		CvUnit* pUnit = pOldCity->getPopulationUnitByIndex(0);
		bool bRemoved = pOldCity->removePopulationUnit(pUnit, false, NO_PROFESSION);
		FAssert(bRemoved);
		aOldPopulationUnits.push_back(pUnit);
		GET_PLAYER(pOldCity->getOwnerINLINE()).getAndRemoveUnit(pUnit->getID());
	}
	FAssert(pOldCity->getPopulation() == 0);

	bRecapture = ((eHighestCulturePlayer != NO_PLAYER) ? (GET_PLAYER(eHighestCulturePlayer).getTeam() == getTeam()) : false);

	pOldCity->kill();

	//acquire old population units
	std::vector<CvUnit*> aNewPopulationUnits;
	for(int i=0;i<(int)aOldPopulationUnits.size();i++)
	{
		CvUnit* pOldUnit = aOldPopulationUnits[i];
		UnitTypes eNewUnitType = (UnitTypes) GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(pOldUnit->getUnitClassType());
		if (eNewUnitType != NO_UNIT)
		{
			CvUnit* pNewUnit = initUnit(eNewUnitType, NO_PROFESSION, pCityPlot->getX_INLINE(), pCityPlot->getY_INLINE(), pOldUnit->AI_getUnitAIType());
			pNewUnit->convert(pOldUnit, true); //kills old unit
			aNewPopulationUnits.push_back(pNewUnit);
		}
		else
		{
			pOldUnit->updateOwnerCache(-1);
			SAFE_DELETE(pOldUnit);
		}
	}

	if (bTrade)
	{
		for (iDX = -1; iDX <= 1; iDX++)
		{
			for (iDY = -1; iDY <= 1; iDY++)
			{
				pLoopPlot	= plotXY(pCityPlot->getX_INLINE(), pCityPlot->getY_INLINE(), iDX, iDY);

				if (pLoopPlot != NULL)
				{
					pLoopPlot->setCulture(eOldOwner, 0, false);
				}
			}
		}
	}

	pNewCity = initCity(pCityPlot->getX_INLINE(), pCityPlot->getY_INLINE(), !bConquest);

	FAssertMsg(pNewCity != NULL, "NewCity is not assigned a valid value");

	pNewCity->setPreviousOwner(eOldOwner);
	pNewCity->setOriginalOwner(eOriginalOwner);
	pNewCity->setGameTurnFounded(iGameTurnFounded);
	pNewCity->setTeachUnitClass(eTeachUnitClass);
	pNewCity->setEducationThresholdMultiplier(iEducationThresholdMultiplier);
	
	for (uint i = 0; i < aNewPopulationUnits.size(); i++)
	{
		pNewCity->addPopulationUnit(aNewPopulationUnits[i], NO_PROFESSION);
	}
	
	pNewCity->setHighestPopulation(iHighestPopulation);
	pNewCity->setName(szName);
	pNewCity->setNeverLost(false);
	pNewCity->changeDefenseDamage(iDamage);

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		pNewCity->setEverOwned(((PlayerTypes)iI), abEverOwned[iI]);
		pNewCity->setCulture(((PlayerTypes)iI), aiCulture[iI], false);
	}

	for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
	{
		bool bSetHasBuilding = false;

		if (pabHasRealBuilding[iI])
		{
			BuildingClassTypes eBuildingClass = (BuildingClassTypes)GC.getBuildingInfo((BuildingTypes)iI).getBuildingClassType();
			eBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(eBuildingClass);

			if (eBuilding != NO_BUILDING)
			{
				if (bTrade || !(GC.getBuildingInfo((BuildingTypes)iI).isNeverCapture()))
				{
					if (pNewCity->isValidBuildingLocation(eBuilding))
					{
						if (!bConquest || bRecapture || pNewCity->isHasRealBuilding(eBuilding) || GC.getGameINLINE().getSorenRandNum(100, "Capture Probability") < GC.getBuildingInfo((BuildingTypes)iI).getConquestProbability())
						{
							bSetHasBuilding = true;
						}
					}
				}

				pNewCity->setHasRealBuildingTimed(eBuilding, bSetHasBuilding, false, ((PlayerTypes)(paiBuildingOriginalOwner[iI])), paiBuildingOriginalTime[iI]);
			}
		}
	}

	for (iI = 0; iI < NUM_YIELD_TYPES; ++iI)
	{
		pNewCity->setYieldStored((YieldTypes) iI, aiYieldsStored[iI]);
	}

	for (std::vector<BuildingYieldChange>::iterator it = aBuildingYieldChange.begin(); it != aBuildingYieldChange.end(); ++it)
	{
		pNewCity->setBuildingYieldChange((*it).eBuildingClass, (*it).eYield, (*it).iChange);
	}

	if (bTrade)
	{
		if (!bRecapture)
		{
			pNewCity->changeOccupationTimer(iOccupationTimer);
		}
	}

	if (bConquest)
	{
		iTeamCulturePercent = pNewCity->calculateTeamCulturePercent(getTeam());

		if (iTeamCulturePercent < GC.getDefineINT("OCCUPATION_CULTURE_PERCENT_THRESHOLD"))
		{
			pNewCity->changeOccupationTimer(((GC.getDefineINT("BASE_OCCUPATION_TURNS") + ((pNewCity->getPopulation() * GC.getDefineINT("OCCUPATION_TURNS_POPULATION_PERCENT")) / 100)) * (100 - iTeamCulturePercent)) / 100);
		}

		GC.getMapINLINE().verifyUnitValidPlot();
	}

	pCityPlot->setRevealed(GET_PLAYER(eOldOwner).getTeam(), true, false, NO_TEAM);

	gDLL->getEventReporterIFace()->cityAcquired(eOldOwner, getID(), pNewCity, bConquest, bTrade);

	SAFE_DELETE_ARRAY(pabHasRealBuilding);
	SAFE_DELETE_ARRAY(paiBuildingOriginalOwner);
	SAFE_DELETE_ARRAY(paiBuildingOriginalTime);

	if (!bTrade)
	{
		TeamTypes eOldPreviousTeam = NO_TEAM;
		if (eOldPreviousOwner != NO_PLAYER)
		{
			eOldPreviousTeam = GET_PLAYER(eOldPreviousOwner).getTeam();
		}

		if (eOldPreviousTeam != getTeam() && eHighestCulturePlayer != getID())
		{
			for (int i = 0; i < GC.getNumFatherPointInfos(); ++i)
			{
				FatherPointTypes ePointType = (FatherPointTypes) i;
				changeFatherPoints(ePointType, GC.getFatherPointInfo(ePointType).getConquerCityPoints());
			}
		}
	}

	if (bConquest)
	{
		CyCity* pyCity = new CyCity(pNewCity);
		CyArgsList argsList;
		argsList.add(getID());	// Player ID
		argsList.add(gDLL->getPythonIFace()->makePythonObject(pyCity));	// pass in city class
		long lResult=0;
		gDLL->getPythonIFace()->callFunction(PYGameModule, "canRazeCity", argsList.makeFunctionArgs(), &lResult);
		delete pyCity;	// python fxn must not hold on to this pointer

		if (lResult == 1)
		{
			//auto raze based on game rules
			if (pNewCity->isAutoRaze())
			{
				if (iCaptureGold > 0)
				{
					szBuffer = gDLL->getText("TXT_KEY_MISC_PILLAGED_CITY", iCaptureGold, pNewCity->getNameKey());
					gDLL->getInterfaceIFace()->addMessage(getID(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_CITYRAZE", MESSAGE_TYPE_MAJOR_EVENT, ARTFILEMGR.getInterfaceArtInfo("WORLDBUILDER_CITY_EDIT")->getPath(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pNewCity->getX_INLINE(), pNewCity->getY_INLINE(), true, true);
				}

				pNewCity->doTask(TASK_RAZE);
			}
			else if (!isHuman())
			{
				AI_conquerCity(pNewCity); // could delete the pointer...
			}
			else
			{
				//popup raze option
				eHighestCulturePlayer = pNewCity->getLiberationPlayer(true);
				bRaze = canRaze(pNewCity);
				bGift = ((eHighestCulturePlayer != NO_PLAYER)
						&& (eHighestCulturePlayer != getID())
						&& ((getTeam() == GET_PLAYER(eHighestCulturePlayer).getTeam())
							|| GET_TEAM(getTeam()).isOpenBorders(GET_PLAYER(eHighestCulturePlayer).getTeam())));

				if (bRaze || bGift)
				{
					CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_RAZECITY);
					pInfo->setData1(pNewCity->getID());
					pInfo->setData2(eHighestCulturePlayer);
					pInfo->setData3(iCaptureGold);
					gDLL->getInterfaceIFace()->addPopup(pInfo, getID());
				}
				else
				{
					pNewCity->chooseProduction();
					gDLL->getEventReporterIFace()->cityAcquiredAndKept(GC.getGameINLINE().getActivePlayer(), pNewCity);
				}
			}
		}
	}
	else 
	{
		if (pNewCity->getPopulation() == 0)
		{
			pNewCity->doTask(TASK_RAZE);
		}
		else if (!bTrade)
		{
			FAssertMsg(false, "No more culture flips");
			gDLL->getEventReporterIFace()->cityAcquiredAndKept(GC.getGameINLINE().getActivePlayer(), pNewCity);
		}
	}

	// Forcing events that deal with the old city not to expire just because we conquered that city
	for (CvEventMap::iterator it = m_mapEventsOccured.begin(); it != m_mapEventsOccured.end(); ++it)
	{
		EventTriggeredData &triggerData = (*it).second;
		if((triggerData.m_eOtherPlayer == eOldOwner) && (triggerData.m_iOtherPlayerCityId == iOldCityId))
		{
			triggerData.m_iOtherPlayerCityId = -1;
		}
	}
}


void CvPlayer::killCities()
{
	int iLoop;
	for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		pLoopCity->kill();
	}
}


const CvWString CvPlayer::getNewCityName() const
{
	for (int i = 0; i < getNumCityNames(); ++i)
	{
		if (isCityNameValid(getCityName(i), true))
		{
			return getCityName(i);
		}
	}

	CvWString szName;
	getCivilizationCityName(szName, getCivilizationType());
	if (!szName.empty())
	{
		return szName;
	}

	// Pick a name from another random civ
	std::vector<CivilizationTypes> aePriorityCivs;
	std::vector<CivilizationTypes> aeOtherCivs;
	int iRandOffset = GC.getGameINLINE().getSorenRandNum(GC.getNumCivilizationInfos(), "Place Units (Player)");
	for (int iI = 0; iI < GC.getNumCivilizationInfos(); iI++)
	{
		CivilizationTypes eLoopCiv = (CivilizationTypes) ((iI + iRandOffset) % GC.getNumCivilizationInfos());
		if (eLoopCiv != getCivilizationType())
		{
			if (GC.getCivilizationInfo(eLoopCiv).isNative() == isNative())
			{
				aePriorityCivs.push_back(eLoopCiv);
			}
			else
			{
				aeOtherCivs.push_back(eLoopCiv);
			}
		}
	}

	for (uint i = 0; i < aePriorityCivs.size(); ++i)
	{
		getCivilizationCityName(szName, aePriorityCivs[i]);
		if (!szName.empty())
		{
			return szName;
		}
	}

	for (uint i = 0; i < aeOtherCivs.size(); ++i)
	{
		getCivilizationCityName(szName, aeOtherCivs[i]);
		if (!szName.empty())
		{
			return szName;
		}
	}

	return L"TXT_KEY_CITY";
}

const CvWString& CvPlayer::addNewCityName()
{
	addCityName(getNewCityName());
	return getCityName(getNumCityNames() - 1);
}


void CvPlayer::getCivilizationCityName(CvWString& szBuffer, CivilizationTypes eCivilization) const
{
	for (int iI = 0; iI < GC.getCivilizationInfo(eCivilization).getNumCityNames(); iI++)
	{
		szBuffer = CvWString(GC.getCivilizationInfo(eCivilization).getCityNames(iI));
		if (isCityNameValid(szBuffer, true))
		{
			return;
		}
	}

	szBuffer.clear();
}


bool CvPlayer::isCityNameValid(const CvWString& szName, bool bTestDestroyed) const
{
	CvCity* pLoopCity;
	int iLoop;

	if (bTestDestroyed)
	{
		if (GC.getGameINLINE().isDestroyedCityName(szName))
		{
			return false;
		}

		for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
		{
			CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);
			for (pLoopCity = kLoopPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kLoopPlayer.nextCity(&iLoop))
			{
				if (pLoopCity->getName() == gDLL->getText(szName))
				{
					return false;
				}
			}
		}
	}
	else
	{
		for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
		{
			if (pLoopCity->getName() == gDLL->getText(szName))
			{
				return false;
			}
		}
	}

	return true;
}


CvUnit* CvPlayer::initUnit(UnitTypes eUnit, ProfessionTypes eProfession, int iX, int iY, UnitAITypes eUnitAI, DirectionTypes eFacingDirection, int iYieldStored)
{
	PROFILE_FUNC();

	FAssertMsg(eUnit != NO_UNIT, "Unit is not assigned a valid value");

	CvUnit* pUnit = addUnit();
	FAssertMsg(pUnit != NULL, "Unit is not assigned a valid value");
	if (NULL != pUnit)
	{
		if (eUnitAI == NO_UNITAI && eProfession != NO_PROFESSION)
		{
			eUnitAI = (UnitAITypes) GC.getProfessionInfo(eProfession).getDefaultUnitAIType();
		}

		if (eUnitAI == NO_UNITAI)
		{
			eUnitAI = (UnitAITypes) GC.getUnitInfo(eUnit).getDefaultUnitAIType();
		}

		pUnit->init(pUnit->getID(), eUnit, eProfession, eUnitAI, getID(), iX, iY, eFacingDirection, iYieldStored);

		if (getID() == GC.getGameINLINE().getActivePlayer())
		{
			if (gDLL->getInterfaceIFace()->getHeadSelectedUnit() == NULL)
			{
				gDLL->getInterfaceIFace()->changeCycleSelectionCounter(1);
			}
		}
	}

	return pUnit;
}

CvUnit* CvPlayer::initEuropeUnit(UnitTypes eUnit, UnitAITypes eUnitAI, DirectionTypes eFacingDirection)
{
	CvUnit* pUnit = initUnit(eUnit, (ProfessionTypes) GC.getUnitInfo(eUnit).getDefaultProfession(), INVALID_PLOT_COORD, INVALID_PLOT_COORD, eUnitAI, eFacingDirection);
	unloadUnitToEurope(pUnit);
	return pUnit;
}

void CvPlayer::killUnits()
{
	while (!m_units.empty())
	{
		m_units.begin()->second->kill(false);
	}
}


// XXX should pUnit be a CvSelectionGroup???
// Returns the next unit in the cycle...
CvSelectionGroup* CvPlayer::cycleSelectionGroups(CvUnit* pUnit, bool bForward, bool* pbWrap)
{
	CLLNode<int>* pSelectionGroupNode;
	CLLNode<int>* pFirstSelectionGroupNode;
	CvSelectionGroup* pLoopSelectionGroup;

	if (pbWrap != NULL)
	{
		*pbWrap = false;
	}

	pSelectionGroupNode = headGroupCycleNode();

	if (pUnit != NULL)
	{
		while (pSelectionGroupNode != NULL)
		{
			if (getSelectionGroup(pSelectionGroupNode->m_data) == pUnit->getGroup())
			{
				if (bForward)
				{
					pSelectionGroupNode = nextGroupCycleNode(pSelectionGroupNode);
				}
				else
				{
					pSelectionGroupNode = previousGroupCycleNode(pSelectionGroupNode);
				}
				break;
			}

			pSelectionGroupNode = nextGroupCycleNode(pSelectionGroupNode);
		}
	}

	if (pSelectionGroupNode == NULL)
	{
		if (bForward)
		{
			pSelectionGroupNode = headGroupCycleNode();
		}
		else
		{
			pSelectionGroupNode = tailGroupCycleNode();
		}

		if (pbWrap != NULL)
		{
			*pbWrap = true;
		}
	}

	if (pSelectionGroupNode != NULL)
	{
		pFirstSelectionGroupNode = pSelectionGroupNode;

		while (true)
		{
			pLoopSelectionGroup = getSelectionGroup(pSelectionGroupNode->m_data);
			FAssertMsg(pLoopSelectionGroup != NULL, "LoopSelectionGroup is not assigned a valid value");

			if (pLoopSelectionGroup->readyToSelect())
			{
				if (pUnit && pLoopSelectionGroup == pUnit->getGroup())
				{
					if (pbWrap != NULL)
					{
						*pbWrap = true;
					}
				}

				return pLoopSelectionGroup;
			}

			if (bForward)
			{
				pSelectionGroupNode = nextGroupCycleNode(pSelectionGroupNode);

				if (pSelectionGroupNode == NULL)
				{
					pSelectionGroupNode = headGroupCycleNode();

					if (pbWrap != NULL)
					{
						*pbWrap = true;
					}
				}
			}
			else
			{
				pSelectionGroupNode = previousGroupCycleNode(pSelectionGroupNode);

				if (pSelectionGroupNode == NULL)
				{
					pSelectionGroupNode = tailGroupCycleNode();

					if (pbWrap != NULL)
					{
						*pbWrap = true;
					}
				}
			}

			if (pSelectionGroupNode == pFirstSelectionGroupNode)
			{
				break;
			}
		}
	}

	return NULL;
}


bool CvPlayer::hasTrait(TraitTypes eTrait) const
{
	return (getTraitCount(eTrait) > 0);
}

int CvPlayer::getTraitCount(TraitTypes eTrait) const
{
	FAssert(eTrait >= 0 && eTrait < GC.getNumTraitInfos());
	return m_aiTraitCount[eTrait];
}

void CvPlayer::changeTraitCount(TraitTypes eTrait, int iChange)
{
	FAssert(eTrait >= 0 && eTrait < GC.getNumTraitInfos());
	m_aiTraitCount[eTrait] += iChange;
	FAssert(getTraitCount(eTrait) >= 0);
}


int CvPlayer::getMercantileFactor() const
{
	int iMercantileFactor = 100;
	for(int iTrait=0;iTrait<GC.getNumTraitInfos();iTrait++)
	{
		TraitTypes eTrait = (TraitTypes) iTrait;
		if (hasTrait(eTrait))
		{
			iMercantileFactor *= (100 + GC.getTraitInfo(eTrait).getMercantileFactor());
			iMercantileFactor /= 100;
		}
	}

	return iMercantileFactor - 100;
}

bool CvPlayer::isHuman() const
{
	return m_bHuman;
}

void CvPlayer::updateHuman()
{
	if (getID() == NO_PLAYER)
	{
		m_bHuman = false;
	}
	else
	{
		m_bHuman = GC.getInitCore().getHuman(getID());
	}
}

bool CvPlayer::isNative() const
{
	CivilizationTypes eCivilizationType = getCivilizationType();
	if(eCivilizationType == NO_CIVILIZATION)
	{
		return false;
	}

	return GC.getCivilizationInfo(eCivilizationType).isNative();
}

bool CvPlayer::isAlwaysOpenBorders() const
{
	if(getCivilizationType() == NO_CIVILIZATION)
	{
		return false;
	}

	return GC.getCivilizationInfo(getCivilizationType()).isOpenBorders();
}

const wchar* CvPlayer::getName(uint uiForm) const
{
	if (isEmpty(gDLL->getPlayerName(getID(), uiForm)) || (GC.getGameINLINE().isMPOption(MPOPTION_ANONYMOUS) && isAlive() && GC.getGameINLINE().getGameState() == GAMESTATE_ON))
	{
		return GC.getLeaderHeadInfo(getLeaderType()).getDescription(uiForm);
	}
	else
	{
		return gDLL->getPlayerName(getID(), uiForm);
	}
}


const wchar* CvPlayer::getNameKey() const
{
	if (isEmpty(gDLL->getPlayerNameKey(getID())) || (GC.getGameINLINE().isMPOption(MPOPTION_ANONYMOUS) && isAlive()))
	{
		return GC.getLeaderHeadInfo(getLeaderType()).getTextKeyWide();
	}
	else
	{
		return gDLL->getPlayerNameKey(getID());
	}
}


const wchar* CvPlayer::getCivilizationDescription(uint uiForm) const
{
	if (isEmpty(gDLL->getCivDescription(getID(), uiForm)))
	{
		return GC.getCivilizationInfo(getCivilizationType()).getDescription(uiForm);
	}
	else
	{
		return gDLL->getCivDescription(getID(), uiForm);
	}
}


const wchar* CvPlayer::getCivilizationDescriptionKey() const
{
	if (isEmpty(gDLL->getCivDescriptionKey(getID())))
	{
		return GC.getCivilizationInfo(getCivilizationType()).getTextKeyWide();
	}
	else
	{
		return gDLL->getCivDescriptionKey(getID());
	}
}


const wchar* CvPlayer::getCivilizationShortDescription(uint uiForm) const
{
	if (isEmpty(gDLL->getCivShortDesc(getID(), uiForm)))
	{
		return GC.getCivilizationInfo(getCivilizationType()).getShortDescription(uiForm);
	}
	else
	{
		return gDLL->getCivShortDesc(getID(), uiForm);
	}
}


const wchar* CvPlayer::getCivilizationShortDescriptionKey() const
{
	if (isEmpty(gDLL->getCivShortDescKey(getID())))
	{
		return GC.getCivilizationInfo(getCivilizationType()).getShortDescriptionKey();
	}
	else
	{
		return gDLL->getCivShortDescKey(getID());
	}
}


const wchar* CvPlayer::getCivilizationAdjective(uint uiForm) const
{
	if (isEmpty(gDLL->getCivAdjective(getID(), uiForm)))
	{
		return GC.getCivilizationInfo(getCivilizationType()).getAdjective(uiForm);
	}
	else
	{
		return gDLL->getCivAdjective(getID(), uiForm);
	}
}

const wchar* CvPlayer::getCivilizationAdjectiveKey() const
{
	if (isEmpty(gDLL->getCivAdjectiveKey(getID())))
	{
		return GC.getCivilizationInfo(getCivilizationType()).getAdjectiveKey();
	}
	else
	{
		return gDLL->getCivAdjectiveKey(getID());
	}
}


const char* CvPlayer::getFlagDecal() const
{
	return GC.getCivilizationInfo(getCivilizationType()).getFlagTexture();
}

bool CvPlayer::isWhiteFlag() const
{
	return GC.getCivilizationInfo(getCivilizationType()).getArtInfo()->isWhiteFlag();
}

bool CvPlayer::isInvertFlag() const
{
	return GC.getCivilizationInfo(getCivilizationType()).getArtInfo()->isInvertFlag();
}

const CvWString CvPlayer::getWorstEnemyName() const
{
	TeamTypes eWorstEnemy;

	eWorstEnemy = GET_TEAM(getTeam()).AI_getWorstEnemy();

	if (eWorstEnemy != NO_TEAM)
	{
		return GET_TEAM(eWorstEnemy).getName();
	}

	return "";
}

ArtStyleTypes CvPlayer::getArtStyleType() const
{
	if (GC.getInitCore().getArtStyle(getID()) == NO_ARTSTYLE)
	{
		return ((ArtStyleTypes)(GC.getCivilizationInfo(getCivilizationType()).getArtStyleType()));
	}
	else
	{
		return GC.getInitCore().getArtStyle(getID());
	}
}

const TCHAR* CvPlayer::getUnitButton(UnitTypes eUnit) const
{
	return GC.getUnitInfo(eUnit).getArtInfo(0, NO_PROFESSION)->getButton();
}

void CvPlayer::doTurn()
{
	PROFILE_FUNC();

	CvCity* pLoopCity;
	int iLoop;

	FAssertMsg(isAlive(), "isAlive is expected to be true");
	FAssertMsg(!hasBusyUnit() || GC.getGameINLINE().isMPOption(MPOPTION_SIMULTANEOUS_TURNS)  || GC.getGameINLINE().isSimultaneousTeamTurns(), "End of turn with busy units in a sequential-turn game");

	gDLL->getEventReporterIFace()->beginPlayerTurn( GC.getGameINLINE().getGameTurn(),  getID());

	doEra();

	doUpdateCacheOnTurn();

	GC.getGameINLINE().verifyDeals();

	AI_doTurnPre();

	AI_assignWorkingPlots();

	doGold();

	doBells();

	doCrosses();

	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		pLoopCity->doTurn();
	}

	verifyCivics();

	doPrices();

	doEvents();

	interceptEuropeUnits();

	updateEconomyHistory(GC.getGameINLINE().getGameTurn(), getGold());
	updateIndustryHistory(GC.getGameINLINE().getGameTurn(), calculateTotalYield(YIELD_HAMMERS));
	updateAgricultureHistory(GC.getGameINLINE().getGameTurn(), calculateTotalYield(YIELD_FOOD));
	updatePowerHistory(GC.getGameINLINE().getGameTurn(), getPower());
	updateCultureHistory(GC.getGameINLINE().getGameTurn(), countTotalCulture());
	expireMessages();  // turn log
	m_aszTradeMessages.clear();

	gDLL->getInterfaceIFace()->setDirty(CityInfo_DIRTY_BIT, true);

	AI_doTurnPost();

	gDLL->getEventReporterIFace()->endPlayerTurn( GC.getGameINLINE().getGameTurn(),  getID());

	FAssert(checkPower(false));
	FAssert(checkPopulation());
}


void CvPlayer::doTurnUnits()
{
	PROFILE_FUNC();

	CvSelectionGroup* pLoopSelectionGroup;
	int iLoop;

	AI_doTurnUnitsPre();

	for (pLoopSelectionGroup = firstSelectionGroup(&iLoop); pLoopSelectionGroup != NULL; pLoopSelectionGroup = nextSelectionGroup(&iLoop))
	{
		pLoopSelectionGroup->doDelayedDeath();
	}


	for (pLoopSelectionGroup = firstSelectionGroup(&iLoop); pLoopSelectionGroup != NULL; pLoopSelectionGroup = nextSelectionGroup(&iLoop))
	{
		pLoopSelectionGroup->doTurn();
	}
	

	if (getParent() != NO_PLAYER)
	{
		CvPlayer& kEurope = GET_PLAYER(getParent());
		if (kEurope.isAlive() && kEurope.isEurope() && !::atWar(getTeam(), kEurope.getTeam()))
		{
			int iLowestCost = MAX_INT;
			UnitTypes eCheapestShip = NO_UNIT;
			for (int iUnitClass = 0; iUnitClass < GC.getNumUnitClassInfos(); ++iUnitClass)
			{
				UnitTypes eLoopUnit = (UnitTypes) GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iUnitClass);
				if (eLoopUnit != NO_UNIT)
				{
					if (GC.getUnitInfo(eLoopUnit).getDomainType() == DOMAIN_SEA)
					{
						int iCost = getEuropeUnitBuyPrice(eLoopUnit);
						if (iCost < iLowestCost && iCost >= 0)
						{
							iLowestCost = iCost;
							eCheapestShip = eLoopUnit;
						}
					}
				}
			}

			if (eCheapestShip != NO_UNIT && getGold() < getEuropeUnitBuyPrice(eCheapestShip))
			{
				CvPlot* pBestPlot = getStartingPlot();
				CvPlot* pLoopPlot = NULL;
				int iBestPlotRand = 0;
				for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
				{
					pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);
					if (pLoopPlot->isRevealed(getTeam(), false) && pBestPlot->getEurope() == pLoopPlot->getEurope())
					{
						int iPlotRand = (1 + GC.getGameINLINE().getSorenRandNum(1000, "Starting Plot"));
						if (iPlotRand > iBestPlotRand)
						{
							iBestPlotRand = iPlotRand;
							pBestPlot = pLoopPlot;
						}
					}
				}
				setStartingPlot(pBestPlot, true);

				bool bHasShip = false;
				int iLoop;
				for (CvUnit* pLoopUnit = firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = nextUnit(&iLoop))
				{
					if (pLoopUnit->getDomainType() == DOMAIN_SEA)
					{
						bHasShip = true;
						break;
					}
				}

				if (!bHasShip)
				{
					for (uint i = 0; i < m_aEuropeUnits.size(); ++i)
					{
						CvUnit* pLoopUnit = m_aEuropeUnits[i];
						if (pLoopUnit->getDomainType() == DOMAIN_SEA)
						{
							bHasShip = true;
							break;
						}
					}
				}

				if (!bHasShip)
				{
					buyEuropeUnit(eCheapestShip, 0);

					//change taxrate
					int iOldTaxRate = getTaxRate();
					int iNewTaxRate = std::min(99, iOldTaxRate + 1 + GC.getGameINLINE().getSorenRandNum(GC.getDefineINT("TAX_RATE_MAX_INCREASE"), "Tax Rate Increase for ship"));
					int iChange = iNewTaxRate - iOldTaxRate;
					changeTaxRate(iChange);

					if (isHuman())
					{
						CvDiploParameters* pDiplo = new CvDiploParameters(kEurope.getID());
						pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_KING_GIFT_SHIP"));
						pDiplo->addDiploCommentVariable(iNewTaxRate);
						pDiplo->setAIContact(true);
						gDLL->beginDiplomacy(pDiplo, getID());
					}
				}
			}
		}
	}

	if (getID() == GC.getGameINLINE().getActivePlayer())
	{
		gDLL->getFAStarIFace()->ForceReset(&GC.getInterfacePathFinder());

		gDLL->getInterfaceIFace()->setDirty(Waypoints_DIRTY_BIT, true);
		gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
	}

	gDLL->getInterfaceIFace()->setDirty(UnitInfo_DIRTY_BIT, true);

	AI_doTurnUnitsPost();
}


void CvPlayer::verifyCivics()
{
	for (int iI = 0; iI < GC.getNumCivicOptionInfos(); iI++)
	{
		if (!canDoCivics(getCivic((CivicOptionTypes)iI)))
		{
			for (int iJ = 0; iJ < GC.getNumCivicInfos(); iJ++)
			{
				if (GC.getCivicInfo((CivicTypes)iJ).getCivicOptionType() == iI)
				{
					if (canDoCivics((CivicTypes)iJ))
					{
						setCivic(((CivicOptionTypes)iI), ((CivicTypes)iJ));
						break;
					}
				}
			}
		}
	}
}

void CvPlayer::updateYield()
{
	int iLoop;
	for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		pLoopCity->updateYield();
	}
}

void CvPlayer::updateCityPlotYield()
{
	int iLoop;
	for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		pLoopCity->plot()->updateYield(true);
	}
}


void CvPlayer::updateCitySight(bool bIncrement)
{
	int iLoop;
	for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		pLoopCity->plot()->updateSight(bIncrement);
	}
}

void CvPlayer::updateTimers()
{
	CvSelectionGroup* pLoopSelectionGroup;
	int iLoop;

	for(pLoopSelectionGroup = firstSelectionGroup(&iLoop); pLoopSelectionGroup; pLoopSelectionGroup = nextSelectionGroup(&iLoop))
	{
		pLoopSelectionGroup->updateTimers(); // could destroy the selection group...
	}

	// if a unit was busy, perhaps it was not quite deleted yet, give it one more try
	if (getNumSelectionGroups() > getNumUnits())
	{
		for(pLoopSelectionGroup = firstSelectionGroup(&iLoop); pLoopSelectionGroup; pLoopSelectionGroup = nextSelectionGroup(&iLoop))
		{
			pLoopSelectionGroup->doDelayedDeath(); // could destroy the selection group...
		}
	}

	FAssertMsg(getNumSelectionGroups() <= getNumUnits(), "The number of Units is expected not to exceed the number of Selection Groups");
}


bool CvPlayer::hasReadyUnit(bool bAny) const
{
	PROFILE_FUNC();

	CvSelectionGroup* pLoopSelectionGroup;
	int iLoop;

	for(pLoopSelectionGroup = firstSelectionGroup(&iLoop); pLoopSelectionGroup; pLoopSelectionGroup = nextSelectionGroup(&iLoop))
	{
		if (pLoopSelectionGroup->readyToMove(bAny))
		{
			return true;
		}
	}

	return false;
}


bool CvPlayer::hasAutoUnit() const
{
	PROFILE_FUNC();

	CvSelectionGroup* pLoopSelectionGroup;
	int iLoop;

	for(pLoopSelectionGroup = firstSelectionGroup(&iLoop); pLoopSelectionGroup; pLoopSelectionGroup = nextSelectionGroup(&iLoop))
	{
		if (pLoopSelectionGroup->readyToAuto())
		{
			return true;
		}
	}

	return false;
}


bool CvPlayer::hasBusyUnit() const
{
	PROFILE_FUNC();

	CvSelectionGroup* pLoopSelectionGroup;
	int iLoop;

	for(pLoopSelectionGroup = firstSelectionGroup(&iLoop); pLoopSelectionGroup; pLoopSelectionGroup = nextSelectionGroup(&iLoop))
	{
		if (pLoopSelectionGroup->isBusy())
		{
		    if (pLoopSelectionGroup->getNumUnits() == 0)
		    {
		        pLoopSelectionGroup->kill();
		        return false;
		    }

			return true;
		}
	}

	return false;
}

int CvPlayer::calculateScore(bool bFinal, bool bVictory) const
{
	PROFILE_FUNC();

	if (!isAlive())
	{
		return 0;
	}

	if (GET_TEAM(getTeam()).getNumMembers() == 0)
	{
		return 0;
	}

	if (isNative())
	{
		return 0;
	}

	long iScore = 0;

	gDLL->getPythonIFace()->pythonCalculateScore(getID(), &iScore, bFinal, bVictory);

	iScore *= getScoreTaxFactor();
	iScore /= 100;

	return ((int)iScore);
}

int CvPlayer::getScoreTaxFactor() const
{
	return std::max(0, 100 - GC.getDefineINT("SCORE_TAX_FACTOR") * getTaxRate() / 100);
}

int CvPlayer::findBestFoundValue() const
{
	CvArea* pLoopArea;
	int iValue;
	int iBestValue;
	int iLoop;

	iBestValue = 0;

	for(pLoopArea = GC.getMapINLINE().firstArea(&iLoop); pLoopArea != NULL; pLoopArea = GC.getMapINLINE().nextArea(&iLoop))
	{
		iValue = pLoopArea->getBestFoundValue(getID());

		if (iValue > iBestValue)
		{
			iBestValue = iValue;
		}
	}

	return iBestValue;
}


int CvPlayer::upgradeAllPrice(UnitTypes eUpgradeUnit, UnitTypes eFromUnit)
{
	CvUnit* pLoopUnit;
	int iPrice;
	int iLoop;

	iPrice = 0;

	// Loop through units and determine the total power of this player's military
	for (pLoopUnit = firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = nextUnit(&iLoop))
	{
		if (pLoopUnit->getUnitType() == eFromUnit)
		{
			if (pLoopUnit->canUpgrade(eUpgradeUnit, true))
			{
				iPrice += pLoopUnit->upgradePrice(eUpgradeUnit);
			}
		}
	}

	return iPrice;
}


int CvPlayer::countNumCoastalCities() const
{
	CvCity* pLoopCity;
	int iCount;
	int iLoop;

	iCount = 0;

	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		if (pLoopCity->isCoastal(GC.getMIN_WATER_SIZE_FOR_OCEAN()))
		{
			iCount++;
		}
	}

	return iCount;
}


int CvPlayer::countNumCoastalCitiesByArea(CvArea* pArea) const
{
	CvCity* pLoopCity;
	int iCount;
	int iLoop;

	iCount = 0;

	int iAreaID = pArea->getID();
	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		if (pLoopCity->isCoastal(GC.getMIN_WATER_SIZE_FOR_OCEAN()))
		{
			if ((pLoopCity->getArea() == iAreaID) || pLoopCity->plot()->isAdjacentToArea(iAreaID))
			{
				iCount++;
			}
		}
	}

	return iCount;
}


int CvPlayer::countTotalCulture() const
{
	int iLoop;
	int iCount = 0;

	for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		iCount += pLoopCity->getCulture(getID());
	}

	return (iCount);
}

int CvPlayer::countTotalYieldStored(YieldTypes eYield) const
{
	int iLoop;
	int iCount = 0;

	for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		iCount += pLoopCity->getYieldStored(eYield);
	}

	return (iCount);
}

int CvPlayer::countCityFeatures(FeatureTypes eFeature) const
{
	PROFILE_FUNC();

	CvCity* pLoopCity;
	CvPlot* pLoopPlot;
	int iCount;
	int iLoop;
	int iI;

	iCount = 0;

	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
		{
			pLoopPlot = plotCity(pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE(), iI);

			if (pLoopPlot != NULL)
			{
				if (pLoopPlot->getFeatureType() == eFeature)
				{
					iCount++;
				}
			}
		}
	}

	return iCount;
}


int CvPlayer::countNumBuildings(BuildingTypes eBuilding) const
{
	PROFILE_FUNC();

	CvCity* pLoopCity;
	int iCount;
	int iLoop;

	iCount = 0;

	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		if (pLoopCity->isHasConceptualBuilding(eBuilding))
		{
			++iCount;
		}
	}

	return iCount;
}


bool CvPlayer::canContact(PlayerTypes ePlayer) const
{
	if (ePlayer == getID())
	{
		return false;
	}

	if (!isAlive() || !(GET_PLAYER(ePlayer).isAlive()))
	{
		return false;
	}

	if (getTeam() != GET_PLAYER(ePlayer).getTeam())
	{
		if (!(GET_TEAM(getTeam()).isHasMet(GET_PLAYER(ePlayer).getTeam())))
		{
			return false;
		}

		if (atWar(getTeam(), GET_PLAYER(ePlayer).getTeam()))
		{
			if (!(GET_TEAM(getTeam()).canChangeWarPeace(GET_PLAYER(ePlayer).getTeam())))
			{
				return false;
			}

			if (isHuman() || GET_PLAYER(ePlayer).isHuman())
			{
				if (GC.getGameINLINE().isOption(GAMEOPTION_ALWAYS_WAR))
				{
					return false;
				}
			}

			if (GET_TEAM(getTeam()).isParentOf(GET_PLAYER(ePlayer).getTeam()) || GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isParentOf(getTeam()))
			{
				return false;
			}
		}
		
		if (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isParentOf(getTeam()))
		{
			if (getParent() != ePlayer)
			{
				return false;
			}
		}

		if (GET_TEAM(getTeam()).isParentOf(GET_PLAYER(ePlayer).getTeam()))
		{
			if (GET_PLAYER(ePlayer).getParent() != getID())
			{
				return false;
			}
		}

		if (isEurope() && GET_PLAYER(ePlayer).getParent() != getID())
		{
			return false;
		}

		if (GET_PLAYER(ePlayer).isEurope() && getParent() != ePlayer)
		{
			return false;
		}
	}

	return true;
}


void CvPlayer::contact(PlayerTypes ePlayer)
{
	CvDiploParameters* pDiplo;

	if (!canContact(ePlayer) || isTurnDone())
	{
		return;
	}

	if (GET_PLAYER(ePlayer).isHuman())
	{
		if (GC.getGameINLINE().isPbem() || GC.getGameINLINE().isHotSeat() || (GC.getGameINLINE().isPitboss() && !gDLL->isConnected(GET_PLAYER(ePlayer).getNetID())))
		{
			if (gDLL->isMPDiplomacy())
			{
				gDLL->beginMPDiplomacy(ePlayer, false, false);
			}
		}
		else
		{
			if (gDLL->getInterfaceIFace()->isFlashing(ePlayer))
			{
				if (!gDLL->getInterfaceIFace()->isDiplomacyLocked())
				{
					gDLL->getInterfaceIFace()->setDiplomacyLocked(true);
					gDLL->sendContactCiv(NETCONTACT_RESPONSE, ePlayer, gDLL->getInterfaceIFace()->getDiplomacyTransportId(ePlayer));
				}
			}
			else
			{
				gDLL->sendContactCiv(NETCONTACT_INITIAL, ePlayer, -1);
			}
		}
	}
	else
	{
		pDiplo = new CvDiploParameters(ePlayer);
		FAssert(pDiplo != NULL);
		if (gDLL->ctrlKey() && getParent() != ePlayer)
		{
			pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_TRADING"));
		}
		gDLL->beginDiplomacy(pDiplo, getID());
	}
}


void CvPlayer::handleDiploEvent(DiploEventTypes eDiploEvent, PlayerTypes ePlayer, int iData1, int iData2)
{
	CvCity* pCity;
	int iI;

	FAssertMsg(ePlayer != getID(), "shouldn't call this function on ourselves");

	switch (eDiploEvent)
	{
	case DIPLOEVENT_CONTACT:
		AI_setFirstContact(ePlayer, true);
		GET_PLAYER(ePlayer).AI_setFirstContact(getID(), true);
		break;

	case DIPLOEVENT_AI_CONTACT:
		break;

	case DIPLOEVENT_FAILED_CONTACT:
		AI_setFirstContact(ePlayer, true);
		GET_PLAYER(ePlayer).AI_setFirstContact(getID(), true);
		break;

	case DIPLOEVENT_GIVE_HELP:
		AI_changeMemoryCount(ePlayer, MEMORY_GIVE_HELP, 1);
		forcePeace(ePlayer);
		break;

	case DIPLOEVENT_REFUSED_HELP:
		AI_changeMemoryCount(ePlayer, MEMORY_REFUSED_HELP, 1);
		break;

	case DIPLOEVENT_ACCEPT_DEMAND:
		AI_changeMemoryCount(ePlayer, MEMORY_ACCEPT_DEMAND, 1);
		forcePeace(ePlayer);
		break;

	case DIPLOEVENT_REJECTED_DEMAND:
		FAssertMsg(GET_PLAYER(ePlayer).getTeam() != getTeam(), "shouldn't call this function on our own team");

		AI_changeMemoryCount(ePlayer, MEMORY_REJECTED_DEMAND, 1);

		if (AI_demandRebukedSneak(ePlayer))
		{
			GET_TEAM(getTeam()).AI_setWarPlan(GET_PLAYER(ePlayer).getTeam(), WARPLAN_PREPARING_LIMITED);
		}
		break;

	case DIPLOEVENT_DEMAND_WAR:
		FAssertMsg(GET_PLAYER(ePlayer).getTeam() != getTeam(), "shouldn't call this function on our own team");

		GET_TEAM(getTeam()).declareWar(GET_PLAYER(ePlayer).getTeam(), false, WARPLAN_LIMITED);
		break;

	case DIPLOEVENT_JOIN_WAR:
		AI_changeMemoryCount(ePlayer, MEMORY_ACCEPTED_JOIN_WAR, 1);
		GET_TEAM(GET_PLAYER(ePlayer).getTeam()).declareWar(((TeamTypes)iData1), false, WARPLAN_DOGPILE);

		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if (GET_PLAYER((PlayerTypes)iI).getTeam() == ((TeamTypes)iData1))
				{
					GET_PLAYER((PlayerTypes)iI).AI_changeMemoryCount(getID(), MEMORY_HIRED_WAR_ALLY, 1);
				}
			}
		}
		break;

	case DIPLOEVENT_NO_JOIN_WAR:
		AI_changeMemoryCount(ePlayer, MEMORY_DENIED_JOIN_WAR, 1);
		break;

	case DIPLOEVENT_STOP_TRADING:
		AI_changeMemoryCount(ePlayer, MEMORY_ACCEPTED_STOP_TRADING, 1);
		GET_PLAYER(ePlayer).stopTradingWithTeam((TeamTypes)iData1);

		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if (GET_PLAYER((PlayerTypes)iI).getTeam() == ((TeamTypes)iData1))
				{
					GET_PLAYER((PlayerTypes)iI).AI_changeMemoryCount(getID(), MEMORY_HIRED_TRADE_EMBARGO, 1);
				}
			}
		}
		break;

	case DIPLOEVENT_NO_STOP_TRADING:
		AI_changeMemoryCount(ePlayer, MEMORY_DENIED_STOP_TRADING, 1);
		break;

	case DIPLOEVENT_ASK_HELP:
		AI_changeMemoryCount(ePlayer, MEMORY_MADE_DEMAND_RECENT, 1);
		break;

	case DIPLOEVENT_MADE_DEMAND:
		if (AI_getMemoryCount(ePlayer, MEMORY_MADE_DEMAND) < 10)
		{
			AI_changeMemoryCount(ePlayer, MEMORY_MADE_DEMAND, 1);
		}
		AI_changeMemoryCount(ePlayer, MEMORY_MADE_DEMAND_RECENT, 1);
		break;

	case DIPLOEVENT_ACCEPT_TAX_RATE:
		GET_PLAYER(ePlayer).changeTaxRate(iData1);
		break;

	case DIPLOEVENT_REFUSE_TAX_RATE:
		{
			AI_changeMemoryCount(ePlayer, MEMORY_REFUSED_TAX, 1);
			CvPlayer& kPlayer = GET_PLAYER(ePlayer);
			YieldTypes eYield = (YieldTypes) iData1;
			kPlayer.setYieldEuropeTradable(eYield, false);
			kPlayer.setYieldTradedTotal(eYield, 0);
			CvCity* pCity = kPlayer.getCity(iData2);
			if (pCity != NULL)
			{
				pCity->setYieldStored(eYield, 0);
				CvWString szMessage = gDLL->getText("TXT_KEY_BOSTON_TEA_PARTY", kPlayer.getCivilizationAdjectiveKey(), pCity->getNameKey(), GC.getYieldInfo(eYield).getTextKeyWide());
				gDLL->getInterfaceIFace()->addMessage(ePlayer, true, GC.getEVENT_MESSAGE_TIME(), szMessage, "AS2D_CITY_REVOLT", MESSAGE_TYPE_MAJOR_EVENT, ARTFILEMGR.getInterfaceArtInfo("WORLDBUILDER_CITY_EDIT")->getPath(), (ColorTypes)GC.getInfoTypeForString("COLOR_WHITE"), pCity->getX_INLINE(), pCity->getY_INLINE(), true, true);
			}
		}
		break;

	case DIPLOEVENT_ACCEPT_KING_GOLD:
		GET_PLAYER(ePlayer).changeGold(-iData1);
		GET_PLAYER(ePlayer).doREFReduction(iData1);
		break;

	case DIPLOEVENT_REFUSE_KING_GOLD:
		AI_changeMemoryCount(ePlayer, MEMORY_REFUSED_TAX, 1);
		break;

	case DIPLOEVENT_LIVE_AMONG_NATIVES:
		{
			CvUnit* pUnit = GET_PLAYER(ePlayer).getUnit(iData1);
			if (pUnit != NULL)
			{
				pUnit->doLiveAmongNatives();
			}
		}
		break;

	case DIPLOEVENT_TRANSPORT_TREASURE:
		{
			CvUnit* pUnit = GET_PLAYER(ePlayer).getUnit(iData1);
			if (pUnit != NULL)
			{
				pUnit->doKingTransport();
			}
		}
		break;

	case DIPLOEVENT_FOUND_CITY:
		{
			CvUnit* pUnit = GET_PLAYER(ePlayer).getUnit(iData1);
			if (pUnit != NULL)
			{
				pUnit->doFound(iData2);
			}
		}
		break;

	case DIPLOEVENT_FOUND_CITY_CHECK_NATIVES:
		{
			CvUnit* pUnit = GET_PLAYER(ePlayer).getUnit(iData1);
			if (pUnit != NULL)
			{
				pUnit->doFoundCheckNatives();
			}
		}
		break;

	case DIPLOEVENT_TARGET_CITY:
		pCity = GET_PLAYER((PlayerTypes)iData1).getCity(iData2);
		if (pCity != NULL)
		{
			pCity->area()->setTargetCity(getID(), pCity);
		}
		break;

	default:
		FAssert(false);
		break;
	}
}


bool CvPlayer::canTradeWith(PlayerTypes eWhoTo) const
{
	if (getParent() != eWhoTo)
	{
		CvPlayer& kWhoTo = GET_PLAYER(eWhoTo);

		if (atWar(getTeam(), kWhoTo.getTeam()))
		{
			return true;
		}

		if (GET_TEAM(getTeam()).isGoldTrading() || GET_TEAM(kWhoTo.getTeam()).isGoldTrading())
		{
			return true;
		}

		if (GET_TEAM(getTeam()).isMapTrading() || GET_TEAM(kWhoTo.getTeam()).isMapTrading())
		{
			return true;
		}

		if (GET_TEAM(getTeam()).isOpenBordersTrading() || GET_TEAM(kWhoTo.getTeam()).isOpenBordersTrading())
		{
			return true;
		}

		if (GET_TEAM(getTeam()).isDefensivePactTrading() || GET_TEAM(kWhoTo.getTeam()).isDefensivePactTrading())
		{
			return true;
		}

		if (GET_TEAM(getTeam()).isPermanentAllianceTrading() || GET_TEAM(kWhoTo.getTeam()).isPermanentAllianceTrading())
		{
			return true;
		}
	}

	return false;
}

bool CvPlayer::canReceiveTradeCity(PlayerTypes eFromPlayer) const
{
	if (GC.getGameINLINE().isOption(GAMEOPTION_ONE_CITY_CHALLENGE) && isHuman())
	{
		return false;
	}

	if (isNative() && !GET_PLAYER(eFromPlayer).isNative())
	{
		return false;
	}

	return true;
}

bool CvPlayer::canTradeItem(PlayerTypes eWhoTo, TradeData item, bool bTestDenial) const
{
	CvPlayer& kWhoTo = GET_PLAYER(eWhoTo);

	if (isEurope() || kWhoTo.isEurope())
	{
		return false;
	}

	if (bTestDenial)
	{
		if (getTradeDenial(eWhoTo, item) != NO_DENIAL)
		{
			return false;
		}
	}

	switch (item.m_eItemType)
	{
	case TRADE_CITIES:
		{
			CvCity* pCityTraded = getCity(item.m_iData1);
			if (NULL != pCityTraded)
			{
				if (pCityTraded->canTradeAway(eWhoTo))
				{
					return true;
				}
			}
		}
		break;

	case TRADE_GOLD:
		if (GET_TEAM(getTeam()).isGoldTrading() || GET_TEAM(kWhoTo.getTeam()).isGoldTrading())
		{
			int iGold = getMaxGoldTrade(eWhoTo, item.m_kTransport);
			if (iGold > 0 && iGold >= item.m_iData1)
			{
				return true;
			}
		}
		break;

	case TRADE_YIELD:
		{
			YieldTypes eYield = (YieldTypes) item.m_iData1;
			CvUnit* pTransport = ::getUnit(item.m_kTransport);
			return (getTradeYieldAmount(eYield, pTransport) > 0);
		}
		break;

	case TRADE_MAPS:
		if (!isNative() && !kWhoTo.isNative())
		{
			if (getTeam() != kWhoTo.getTeam())
			{
				if (GET_TEAM(getTeam()).isMapTrading() || GET_TEAM(kWhoTo.getTeam()).isMapTrading())
				{
					return true;
				}
			}
		}
		break;

	case TRADE_PEACE:
		if (!(GET_TEAM(getTeam()).isHuman()))
		{
			if (GET_TEAM(getTeam()).isHasMet((TeamTypes)(item.m_iData1)) && GET_TEAM(kWhoTo.getTeam()).isHasMet((TeamTypes)(item.m_iData1)))
			{
				if (atWar(getTeam(), ((TeamTypes)(item.m_iData1))))
				{
					return true;
				}
			}
		}
		break;

	case TRADE_WAR:
		if (!(GET_TEAM(getTeam()).isHuman()))
		{
			if (GET_TEAM(getTeam()).isHasMet((TeamTypes)(item.m_iData1)) && GET_TEAM(kWhoTo.getTeam()).isHasMet((TeamTypes)(item.m_iData1)))
			{
				if (GET_TEAM(getTeam()).canDeclareWar((TeamTypes)(item.m_iData1)))
				{
					return true;
				}
			}
		}
		break;

	case TRADE_EMBARGO:
		if (!(GET_TEAM(getTeam()).isHuman()))
		{
			if (GET_TEAM(getTeam()).isHasMet((TeamTypes)(item.m_iData1)) && GET_TEAM(kWhoTo.getTeam()).isHasMet((TeamTypes)(item.m_iData1)))
			{
				if (canStopTradingWithTeam((TeamTypes)(item.m_iData1)))
				{
					return true;
				}
			}
		}
		break;

	case TRADE_OPEN_BORDERS:
		if (getTeam() != kWhoTo.getTeam())
		{
			if (!atWar(getTeam(), kWhoTo.getTeam()))
			{
				if (!(GET_TEAM(getTeam()).isOpenBorders(kWhoTo.getTeam())) && !isAlwaysOpenBorders() && !kWhoTo.isAlwaysOpenBorders())
				{
					if (GET_TEAM(getTeam()).isOpenBordersTrading() || GET_TEAM(kWhoTo.getTeam()).isOpenBordersTrading())
					{
						return true;
					}
				}
			}
		}
		break;

	case TRADE_DEFENSIVE_PACT:
		if (getParent() == NO_PLAYER || kWhoTo.getParent() == NO_PLAYER)
		{
			if (getTeam() != kWhoTo.getTeam())
			{
				if (!atWar(getTeam(), kWhoTo.getTeam()))
				{
					if (!(GET_TEAM(getTeam()).isDefensivePact(kWhoTo.getTeam())))
					{
						if (GET_TEAM(getTeam()).isDefensivePactTrading() || GET_TEAM(kWhoTo.getTeam()).isDefensivePactTrading())
						{
							if ((GET_TEAM(getTeam()).getAtWarCount() == 0) && (GET_TEAM(kWhoTo.getTeam()).getAtWarCount() == 0))
							{
								if (GET_TEAM(getTeam()).canSignDefensivePact(kWhoTo.getTeam()))
								{
									return true;
								}
							}
						}
					}
				}
			}
		}
		break;

	case TRADE_PERMANENT_ALLIANCE:
		if (getTeam() != kWhoTo.getTeam())
		{
			if (!atWar(getTeam(), kWhoTo.getTeam()))
			{
				if (GET_TEAM(getTeam()).isPermanentAllianceTrading() || GET_TEAM(kWhoTo.getTeam()).isPermanentAllianceTrading())
				{
					if ((GET_TEAM(getTeam()).getNumMembers() == 1) && (GET_TEAM(kWhoTo.getTeam()).getNumMembers() == 1))
					{
						return true;
					}
				}
			}
		}
		break;

	case TRADE_PEACE_TREATY:
		return true;
		break;
	}

	return false;
}


DenialTypes CvPlayer::getTradeDenial(PlayerTypes eWhoTo, TradeData item) const
{
	CvCity* pCity;
	CvPlayer& kWhoTo = GET_PLAYER(eWhoTo);

	switch (item.m_eItemType)
	{
	case TRADE_CITIES:
		pCity = getCity(item.m_iData1);
		if (pCity != NULL)
		{
			return AI_cityTrade(pCity, eWhoTo);
		}
		break;

	case TRADE_GOLD:
		break;

	case TRADE_YIELD:
		return AI_yieldTrade((YieldTypes) item.m_iData1, item.m_kTransport, eWhoTo);
		break;

	case TRADE_MAPS:
		return GET_TEAM(getTeam()).AI_mapTrade(kWhoTo.getTeam());
		break;

	case TRADE_PEACE:
		return GET_TEAM(getTeam()).AI_makePeaceTrade(((TeamTypes)(item.m_iData1)), kWhoTo.getTeam());
		break;

	case TRADE_WAR:
		return GET_TEAM(getTeam()).AI_declareWarTrade(((TeamTypes)(item.m_iData1)), kWhoTo.getTeam());
		break;

	case TRADE_EMBARGO:
		return AI_stopTradingTrade(((TeamTypes)(item.m_iData1)), eWhoTo);
		break;

	case TRADE_OPEN_BORDERS:
		return GET_TEAM(getTeam()).AI_openBordersTrade(kWhoTo.getTeam());
		break;

	case TRADE_DEFENSIVE_PACT:
		return GET_TEAM(getTeam()).AI_defensivePactTrade(kWhoTo.getTeam());
		break;

	case TRADE_PERMANENT_ALLIANCE:
		return GET_TEAM(getTeam()).AI_permanentAllianceTrade(kWhoTo.getTeam());
		break;

	case TRADE_PEACE_TREATY:
		break;
	}

	return NO_DENIAL;
}

bool CvPlayer::isTradingWithTeam(TeamTypes eTeam, bool bIncludeCancelable) const
{
	int iLoop;

	if (eTeam == getTeam())
	{
		return false;
	}

	for (CvDeal* pLoopDeal = GC.getGameINLINE().firstDeal(&iLoop); pLoopDeal != NULL; pLoopDeal = GC.getGameINLINE().nextDeal(&iLoop))
	{
		if (bIncludeCancelable || pLoopDeal->isCancelable(getID()))
		{
			if (!pLoopDeal->isPeaceDeal())
			{
				if ((pLoopDeal->getFirstPlayer() == getID()) && (GET_PLAYER(pLoopDeal->getSecondPlayer()).getTeam() == eTeam))
				{
					if (pLoopDeal->getLengthFirstTrades() > 0)
					{
						return true;
					}
				}

				if ((pLoopDeal->getSecondPlayer() == getID()) && (GET_PLAYER(pLoopDeal->getFirstPlayer()).getTeam() == eTeam))
				{
					if (pLoopDeal->getLengthSecondTrades() > 0)
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}


bool CvPlayer::canStopTradingWithTeam(TeamTypes eTeam, bool bContinueNotTrading) const
{
	if (eTeam == getTeam())
	{
		return false;
	}

	if (!isTradingWithTeam(eTeam, false))
	{
		if (bContinueNotTrading && !isTradingWithTeam(eTeam, true))
		{
			return true;
		}

		return false;
	}

	return true;
}


void CvPlayer::stopTradingWithTeam(TeamTypes eTeam)
{
	CvDeal* pLoopDeal;
	int iLoop;
	int iI;

	FAssert(eTeam != getTeam());

	for(pLoopDeal = GC.getGameINLINE().firstDeal(&iLoop); pLoopDeal != NULL; pLoopDeal = GC.getGameINLINE().nextDeal(&iLoop))
	{
		if (pLoopDeal->isCancelable(getID()) && !(pLoopDeal->isPeaceDeal()))
		{
			if (((pLoopDeal->getFirstPlayer() == getID()) && (GET_PLAYER(pLoopDeal->getSecondPlayer()).getTeam() == eTeam)) ||
				  ((pLoopDeal->getSecondPlayer() == getID()) && (GET_PLAYER(pLoopDeal->getFirstPlayer()).getTeam() == eTeam)))
			{
				pLoopDeal->kill(true, getTeam());
			}
		}
	}

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() == eTeam)
			{
				GET_PLAYER((PlayerTypes)iI).AI_changeMemoryCount(getID(), MEMORY_STOPPED_TRADING, 1);
				GET_PLAYER((PlayerTypes)iI).AI_changeMemoryCount(getID(), MEMORY_STOPPED_TRADING_RECENT, 1);
			}
		}
	}
}


void CvPlayer::killAllDeals()
{
	CvDeal* pLoopDeal;
	int iLoop;

	for(pLoopDeal = GC.getGameINLINE().firstDeal(&iLoop); pLoopDeal != NULL; pLoopDeal = GC.getGameINLINE().nextDeal(&iLoop))
	{
		if ((pLoopDeal->getFirstPlayer() == getID()) || (pLoopDeal->getSecondPlayer() == getID()))
		{
			pLoopDeal->kill(true, getTeam());
		}
	}
}


void CvPlayer::findNewCapital()
{
	CvCity* pOldCapital;
	CvCity* pLoopCity;
	CvCity* pBestCity;
	BuildingTypes eCapitalBuilding;
	int iValue;
	int iBestValue;
	int iLoop;

	BuildingClassTypes eCapitalBuildingClass = (BuildingClassTypes) GC.getDefineINT("CAPITAL_BUILDINGCLASS");
	if(eCapitalBuildingClass == NO_BUILDINGCLASS)
	{
		return;
	}

	eCapitalBuilding = ((BuildingTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(eCapitalBuildingClass)));
	if (eCapitalBuilding == NO_BUILDING)
	{
		return;
	}

	pOldCapital = getCapitalCity();

	iBestValue = 0;
	pBestCity = NULL;

	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		if (pLoopCity != pOldCapital)
		{
			if (!pLoopCity->isHasRealBuilding(eCapitalBuilding))
			{
				iValue = (pLoopCity->getPopulation() * 4);

				iValue += pLoopCity->getYieldRate(YIELD_FOOD);
				iValue += pLoopCity->getCultureLevel();

				iValue *= (pLoopCity->calculateCulturePercent(getID()) + 100);
				iValue /= 100;

				if (iValue > iBestValue)
				{
					iBestValue = iValue;
					pBestCity = pLoopCity;
				}
			}
		}
	}

	if (pBestCity != NULL)
	{
		if (pOldCapital != NULL)
		{
			pOldCapital->setHasRealBuilding(eCapitalBuilding, false);
		}
		FAssert(!(pBestCity->isHasRealBuilding(eCapitalBuilding)));
		pBestCity->setHasRealBuilding(eCapitalBuilding, true);
	}
}

bool CvPlayer::canRaze(CvCity* pCity) const
{
	if (!pCity->isAutoRaze())
	{
		if (GC.getGameINLINE().isOption(GAMEOPTION_NO_CITY_RAZING))
		{
			return false;
		}

		if (pCity->getOwnerINLINE() != getID())
		{
			return false;
		}

		if (pCity->calculateTeamCulturePercent(getTeam()) >= GC.getDefineINT("RAZING_CULTURAL_PERCENT_THRESHOLD"))
		{
			return false;
		}
	}

	CyCity* pyCity = new CyCity(pCity);
	CyArgsList argsList;
	argsList.add(getID());	// Player ID
	argsList.add(gDLL->getPythonIFace()->makePythonObject(pyCity));	// pass in city class
	long lResult=0;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "canRazeCity", argsList.makeFunctionArgs(), &lResult);
	delete pyCity;	// python fxn must not hold on to this pointer
	if (lResult == 0)
	{
		return (false);
	}

	return true;
}


void CvPlayer::raze(CvCity* pCity)
{
	if (!canRaze(pCity))
	{
		return;
	}

	FAssert(pCity->getOwnerINLINE() == getID());

	PlayerTypes eHighestCulturePlayer = pCity->findHighestCulture();

	if (eHighestCulturePlayer != NO_PLAYER)
	{
		if (GET_PLAYER(eHighestCulturePlayer).getTeam() != getTeam())
		{
			GET_PLAYER(eHighestCulturePlayer).AI_changeMemoryCount(getID(), MEMORY_RAZED_CITY, 1);
			AI_changeMemoryCount(eHighestCulturePlayer, MEMORY_REVENGE_TAKEN, 1);
			if (isNative())
			{
				GET_TEAM(getTeam()).AI_changeDamages(pCity->getTeam(), -AI_cityTradeVal(pCity, eHighestCulturePlayer));
			}
		}
	}

	CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_DESTROYED_CITY", pCity->getNameKey());
	gDLL->getInterfaceIFace()->addMessage(getID(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_CITYRAZE", MESSAGE_TYPE_MAJOR_EVENT, ARTFILEMGR.getInterfaceArtInfo("WORLDBUILDER_CITY_EDIT")->getPath(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pCity->getX_INLINE(), pCity->getY_INLINE(), true, true);

	for (int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (iI != getID())
			{
				if (pCity->isRevealed(GET_PLAYER((PlayerTypes)iI).getTeam(), false))
				{
					szBuffer = gDLL->getText("TXT_KEY_MISC_CITY_HAS_BEEN_RAZED_BY", pCity->getNameKey(), getCivilizationDescriptionKey());
					gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_CITYRAZED", MESSAGE_TYPE_MAJOR_EVENT, ARTFILEMGR.getInterfaceArtInfo("WORLDBUILDER_CITY_EDIT")->getPath(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pCity->getX_INLINE(), pCity->getY_INLINE(), true, true);
				}
			}
		}
	}

	szBuffer = gDLL->getText("TXT_KEY_MISC_CITY_RAZED_BY", pCity->getNameKey(), getCivilizationDescriptionKey());
	GC.getGameINLINE().addReplayMessage(REPLAY_MESSAGE_MAJOR_EVENT, getID(), szBuffer, pCity->getX_INLINE(), pCity->getY_INLINE(), (ColorTypes)GC.getInfoTypeForString("COLOR_WARNING_TEXT"));

	// Report this event
	gDLL->getEventReporterIFace()->cityRazed(pCity, getID());

	if (pCity->getPreviousOwner() != NO_PLAYER)
	{
		if (pCity->getHighestPopulation() > 1)
		{
			UnitClassTypes eUnitClass = (UnitClassTypes) GC.getCivilizationInfo(GET_PLAYER(pCity->getPreviousOwner()).getCivilizationType()).getCapturedCityUnitClass();

			if (eUnitClass != NO_UNITCLASS)
			{
				UnitTypes eUnit = (UnitTypes) GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(eUnitClass);

				if (eUnit != NO_UNIT)
				{
					for (int i = 0; i < pCity->getPopulation(); ++i)
					{
						initUnit(eUnit, (ProfessionTypes) GC.getUnitInfo(eUnit).getDefaultProfession(), pCity->getX_INLINE(), pCity->getY_INLINE());
					}
				}
			}
		}

		int iTreasure = GC.getDefineINT("SETTLEMENT_TREASURE_YIELD") * pCity->getHighestPopulation();
		iTreasure = iTreasure * GC.getCivilizationInfo(GET_PLAYER(pCity->getPreviousOwner()).getCivilizationType()).getTreasure() / 100;
		iTreasure = iTreasure * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getGrowthPercent() / 100;
		iTreasure = iTreasure / 2 + GC.getGameINLINE().getSorenRandNum(iTreasure / 2, "Treasure Gold 1") + GC.getGameINLINE().getSorenRandNum(iTreasure / 2, "Treasure Gold 2");

		int iTreasureMod = 100;
		for (int iTrait = 0; iTrait < GC.getNumTraitInfos(); ++iTrait)
		{
			CvTraitInfo& kTrait = GC.getTraitInfo((TraitTypes) iTrait);
			if (GET_PLAYER(pCity->getPreviousOwner()).hasTrait((TraitTypes) iTrait))
			{
				iTreasureMod += kTrait.getTreasureModifier();
			}
		}
		iTreasure *= iTreasureMod;
		iTreasure /= 100;

		if (iTreasure != 0)
		{
			UnitClassTypes eUnitClass = (UnitClassTypes) GC.getDefineINT("TREASURE_UNITCLASS");

			if (eUnitClass != NO_UNITCLASS)
			{
				UnitTypes eUnit = (UnitTypes) GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(eUnitClass);

				if (eUnit != NO_UNIT)
				{
					FAssert(GC.getUnitInfo(eUnit).isTreasure());
					CvUnit* pTreasure = initUnit(eUnit, (ProfessionTypes) GC.getUnitInfo(eUnit).getDefaultProfession(), pCity->getX_INLINE(), pCity->getY_INLINE(), NO_UNITAI, NO_DIRECTION, iTreasure);
				}
			}
		}
	}

	if (pCity->findHighestCulture() != getID())
	{
		for (int i = 0; i < GC.getNumFatherPointInfos(); ++i)
		{
			FatherPointTypes ePointType = (FatherPointTypes) i;
			changeFatherPoints(ePointType, GC.getFatherPointInfo(ePointType).getRazeCityPoints() - GC.getFatherPointInfo(ePointType).getConquerCityPoints());
		}
	}

	disband(pCity);
}


void CvPlayer::disband(CvCity* pCity)
{
	if (getNumCities() == 1)
	{
		setFoundedFirstCity(false);
	}

	GC.getGameINLINE().addDestroyedCityName(pCity->getNameKey());

	pCity->kill();
}


bool CvPlayer::canReceiveGoody(CvPlot* pPlot, GoodyTypes eGoody, const CvUnit* pUnit) const
{
	CvGoodyInfo& kGoody = GC.getGoodyInfo(eGoody);

	if (kGoody.isWar())
	{
		if (!pPlot->isOwned())
		{
			return false;
		}
		
		if (!GET_TEAM(pPlot->getTeam()).canDeclareWar(getTeam()))
		{
			return false;
		}
	}

	if (kGoody.getExperience() > 0)
	{
		if ((pUnit == NULL) || !pUnit->canAcquirePromotionAny())
		{
			return false;
		}
	}

	if (kGoody.getDamagePrereq() > 0)
	{
		if ((pUnit == NULL) || (pUnit->getDamage() < ((pUnit->maxHitPoints() * kGoody.getDamagePrereq()) / 100)))
		{
			return false;
		}
	}
	if (kGoody.isBad())
	{
		if ((pUnit == NULL) || pUnit->isNoBadGoodies())
		{
			return false;
		}
	}

	if (kGoody.getUnitClassType() != NO_UNITCLASS)
	{
		UnitTypes eUnit = ((UnitTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(kGoody.getUnitClassType())));

		if (eUnit == NO_UNIT)
		{
			return false;
		}

		if ((GC.getUnitInfo(eUnit).getCombat() > 0) && !(GC.getUnitInfo(eUnit).isOnlyDefensive()))
		{
			if (GC.getGameINLINE().isGameMultiPlayer())
			{
				return false;
			}
		}

		if (GC.getGameINLINE().isOption(GAMEOPTION_ONE_CITY_CHALLENGE) && isHuman())
		{
			if (GC.getUnitInfo(eUnit).isFound())
			{
				return false;
			}
		}
	}

	if (kGoody.getTeachUnitClassType() != NO_UNITCLASS)
	{
		if (pUnit == NULL)
		{
			return false;
		}

		UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(kGoody.getTeachUnitClassType());

		if (eUnit == NO_UNIT)
		{
			return false;
		}

		if (pUnit->getUnitInfo().getLearnTime() < 0)
		{
			return false;
		}

		if (pUnit->getProfession() != GC.getUnitInfo(eUnit).getDefaultProfession())
		{
			return false;
		}
	}

	for (int i = 0; i < GC.getNumGoodyInfos(); ++i)
	{
		if (kGoody.getGoodyWeight(i) > 0)
		{
			for (int j = 0; j < GC.getNumGoodyInfos(); ++j)
			{
				if (GC.getGoodyInfo((GoodyTypes) i).getGoodyWeight(j) > 0)
				{
					return false;
				}
			}					

			if (!canReceiveGoody(pPlot, (GoodyTypes) i, pUnit))
			{
				return false;
			}
		}
	}

	return true;
}

void CvPlayer::receiveRandomGoody(CvPlot* pPlot, GoodyTypes eGoody, CvUnit* pUnit)
{
	int iTotalWeight = 0;
	GoodyTypes eBestGoody = NO_GOODY;
	for (int i = 0; i < GC.getNumGoodyInfos(); ++i)
	{
		iTotalWeight += GC.getGoodyInfo(eGoody).getGoodyWeight(i);
	}

	int iPick = GC.getGameINLINE().getSorenRandNum(iTotalWeight, "goody result");

	iTotalWeight = 0;
	for (int i = 0; i < GC.getNumGoodyInfos(); ++i)
	{
		iTotalWeight += GC.getGoodyInfo(eGoody).getGoodyWeight(i);
		if (iTotalWeight > iPick)
		{
			receiveGoody(pPlot, (GoodyTypes) i, pUnit);
			break;
		}
	}
}

int CvPlayer::receiveGoody(CvPlot* pPlot, GoodyTypes eGoody, CvUnit* pUnit)
{
	int iReturnValue = -1;

	FAssertMsg(canReceiveGoody(pPlot, eGoody, pUnit), "Instance is expected to be able to recieve goody");

	CvGoodyInfo& kGoody = GC.getGoodyInfo(eGoody);

	CvWString szBuffer = gDLL->getObjectText(kGoody.getAnnounceTextKey(), 0);

	int iGold = kGoody.getGold() + GC.getGameINLINE().getSorenRandNum(kGoody.getGoldRand1(), "Goody Gold 1") + GC.getGameINLINE().getSorenRandNum(kGoody.getGoldRand2(), "Goody Gold 2");
	iGold = iGold * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getGrowthPercent() / 100;
	if ((pUnit != NULL) && pUnit->isNoBadGoodies())
	{
		iGold = iGold * GC.getDefineINT("NO_BAD_GOODIES_GOLD_PERCENT") / 100;
	}

	if (iGold != 0)
	{
		CvCity* pCity = pPlot->getPlotCity();
		if (pCity != NULL)
		{
			for (int iTrait = 0; iTrait < GC.getNumTraitInfos(); ++iTrait)
			{
				TraitTypes eTrait = (TraitTypes) iTrait;
				if (GET_PLAYER(pCity->getOwnerINLINE()).hasTrait(eTrait) || hasTrait(eTrait))
				{
					iGold *= 100 + GC.getTraitInfo(eTrait).getChiefGoldModifier();
					iGold /= 100;
				}
			}
		}

		iReturnValue = iGold;

		//store gold in treasure unit instead
		UnitClassTypes eUnitClass = (UnitClassTypes) kGoody.getUnitClassType();
		bool bTreasure = false;
		if (eUnitClass != NO_UNITCLASS)
		{
			UnitTypes eTreasure = (UnitTypes) GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(eUnitClass);
			if((eTreasure != NO_UNIT) && GC.getUnitInfo(eTreasure).isTreasure())
			{
				bTreasure = true;
			}
		}

		if (!bTreasure)
		{
			changeGold(iGold);
			szBuffer += gDLL->getText("TXT_KEY_MISC_RECEIVED_GOLD", iGold);
		}
	}

	if (!szBuffer.empty())
	{
		gDLL->getInterfaceIFace()->addMessage(getID(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, kGoody.getSound(), MESSAGE_TYPE_MINOR_EVENT, ARTFILEMGR.getImprovementArtInfo("ART_DEF_IMPROVEMENT_GOODY_HUT")->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_WHITE"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
	}

	int iRange = kGoody.getMapRange();
	if ((pUnit != NULL) && pUnit->isNoBadGoodies())
	{
		iRange = iRange * GC.getDefineINT("NO_BAD_GOODIES_GOLD_PERCENT") / 100;
	}

	if (iRange > 0)
	{
		int iOffset = kGoody.getMapOffset();

		CvPlot* pBestPlot = NULL;
		if (iOffset > 0)
		{
			int iBestValue = 0;

			for (int iDX = -(iOffset); iDX <= iOffset; iDX++)
			{
				for (int iDY = -(iOffset); iDY <= iOffset; iDY++)
				{
					CvPlot* pLoopPlot = plotXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iDX, iDY);

					if (pLoopPlot != NULL)
					{
						if (!(pLoopPlot->isRevealed(getTeam(), false)))
						{
							int iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "Goody Map"));

							iValue *= plotDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE());

							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								pBestPlot = pLoopPlot;
							}
						}
					}
				}
			}
		}

		if (pBestPlot == NULL)
		{
			pBestPlot = pPlot;
		}

		for (int iDX = -(iRange); iDX <= iRange; iDX++)
		{
			for (int iDY = -(iRange); iDY <= iRange; iDY++)
			{
				CvPlot* pLoopPlot = plotXY(pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), iDX, iDY);

				if (pLoopPlot != NULL)
				{
					if (plotDistance(pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE()) <= iRange)
					{
						if (GC.getGameINLINE().getSorenRandNum(100, "Goody Map") < kGoody.getMapProb())
						{
							pLoopPlot->setRevealed(getTeam(), true, false, NO_TEAM);
						}
					}
				}
			}
		}
	}

	if (pUnit != NULL)
	{
		int iExperience = kGoody.getExperience();
		if (pUnit->isNoBadGoodies())
		{
			iExperience = iExperience * GC.getDefineINT("NO_BAD_GOODIES_EXPERIENCE_PERCENT") / 100;
		}
		pUnit->changeExperience(iExperience);
	}

	if (pUnit != NULL)
	{
		pUnit->changeDamage(-(kGoody.getHealing()));
	}

	if (kGoody.getUnitClassType() != NO_UNITCLASS)
	{
		UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(kGoody.getUnitClassType());

		if (eUnit != NO_UNIT)
		{
			CvUnit* pGoodyUnit = initUnit(eUnit, (ProfessionTypes) GC.getUnitInfo(eUnit).getDefaultProfession(), pPlot->getX_INLINE(), pPlot->getY_INLINE());

			//treasure
			if((pGoodyUnit != NULL) && (iGold != 0) && pGoodyUnit->getUnitInfo().isTreasure())
			{
				pGoodyUnit->setYieldStored(iGold);
			}
		}
	}

	if (kGoody.getTeachUnitClassType() != NO_UNITCLASS)
	{
		UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(kGoody.getTeachUnitClassType());

		if (eUnit != NO_UNIT)
		{
			CvUnit* pLearnUnit = initUnit(eUnit, pUnit->getProfession(), pPlot->getX_INLINE(), pPlot->getY_INLINE(), pUnit->AI_getUnitAIType());
			FAssert(pLearnUnit != NULL);
			pLearnUnit->joinGroup(pUnit->getGroup());
			pLearnUnit->convert(pUnit, true);
		}
	}

	if (kGoody.isWar())
	{
		GET_TEAM(pPlot->getTeam()).declareWar(getTeam(), true, WARPLAN_EXTORTION);
	}

	for (int i = 0; i < GC.getNumGoodyInfos(); ++i)
	{
		if (kGoody.getGoodyWeight(i) > 0)
		{
			if (isHuman())
			{
				CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_CHOOSE_GOODY, eGoody, GC.getMapINLINE().plotNumINLINE(pPlot->getX_INLINE(), pPlot->getY_INLINE()), pUnit ? pUnit->getID() : -1);
				gDLL->getInterfaceIFace()->addPopup(pInfo, getID(), true);
			}
			else
			{
				if (AI_chooseGoody(eGoody))
				{
					receiveRandomGoody(pPlot, eGoody, pUnit);
				}
			}
			break;
		}
	}

	return iReturnValue;
}


void CvPlayer::doGoody(CvPlot* pPlot, CvUnit* pUnit)
{
	// Python override
	if (gDLL->getPythonIFace()->pythonDoGoody(getID(), pPlot, pUnit))
	{
		return;
	}
	else
	{
		FAssertMsg(pPlot->isGoody(), "pPlot->isGoody is expected to be true");

		if (!isNative())
		{
			pPlot->removeGoody();

			std::vector<int> aGoodyFactors(GC.getNumGoodyInfos(), 1);
			for (int iTrait = 0; iTrait < GC.getNumTraitInfos(); ++iTrait)
			{
				CvTraitInfo& kTraitInfo = GC.getTraitInfo((TraitTypes) iTrait);
				if (hasTrait((TraitTypes) iTrait))
				{
					for (int iGoody = 0; iGoody < GC.getNumGoodyInfos(); ++iGoody)
					{
						aGoodyFactors[iGoody] *= kTraitInfo.getGoodyFactor(iGoody);
					}
				}
			}

			int iBestValue = -1;
			GoodyTypes eBestGoody = NO_GOODY;
			for (int iI = 0; iI < GC.getHandicapInfo(getHandicapType()).getNumGoodies(); iI++)
			{
				GoodyTypes eGoody = (GoodyTypes) GC.getHandicapInfo(getHandicapType()).getGoodies(iI);

				FAssert(eGoody >= 0);
				FAssert(eGoody < GC.getNumGoodyInfos());

				if (canReceiveGoody(pPlot, eGoody, pUnit))
				{
					for (int iJ = 0; iJ < aGoodyFactors[eGoody]; ++iJ)
					{
						int iRandValue = GC.getGameINLINE().getSorenRandNum(1000, "Goodies");
						if(iRandValue > iBestValue)
						{
							iBestValue = iRandValue;
							eBestGoody = eGoody;
						}
					}
				}
			}

			FAssert(eBestGoody != NO_GOODY);
			if(eBestGoody != NO_GOODY)
			{
				receiveGoody(pPlot, eBestGoody, pUnit);

				// Python Event
				gDLL->getEventReporterIFace()->goodyReceived(getID(), pPlot, pUnit, eBestGoody);
			}
		}
	}
}


bool CvPlayer::canFound(int iX, int iY, bool bTestVisible) const
{
	CvPlot* pPlot;
	CvPlot* pLoopPlot;
	bool bValid;
	int iRange;
	int iDX, iDY;

	pPlot = GC.getMapINLINE().plotINLINE(iX, iY);

	if (GC.getGameINLINE().isFinalInitialized())
	{
		if (GC.getGameINLINE().isOption(GAMEOPTION_ONE_CITY_CHALLENGE) && isHuman())
		{
			if (getNumCities() > 0)
			{
				return false;
			}
		}
	}

	if (pPlot->isImpassable())
	{
		return false;
	}

	if (pPlot->isPeak())
	{
		return false;
	}

	if (pPlot->getFeatureType() != NO_FEATURE)
	{
		if (GC.getFeatureInfo(pPlot->getFeatureType()).isNoCity())
		{
			return false;
		}
	}

	if (pPlot->isCity())
	{
		return false;
	}

	if (pPlot->isOwned() && (pPlot->getOwnerINLINE() != getID()))
	{
		if (!GET_PLAYER(pPlot->getOwnerINLINE()).isNative())
		{
			return false;
		}
	}

	bValid = false;

	if (!bValid)
	{
		if (GC.getTerrainInfo(pPlot->getTerrainType()).isFound())
		{
			bValid = true;
		}
	}

	if (!bValid)
	{
		if (GC.getTerrainInfo(pPlot->getTerrainType()).isFoundCoast())
		{
			if (pPlot->isCoastalLand())
			{
				bValid = true;
			}
		}
	}

	long lResult = 0;
	if(GC.getUSE_CAN_FOUND_CITIES_ON_WATER_CALLBACK())
	{
		CyArgsList argsList2;
		argsList2.add(iX);
		argsList2.add(iY);
		lResult=0;
		gDLL->getPythonIFace()->callFunction(PYGameModule, "canFoundCitiesOnWater", argsList2.makeFunctionArgs(), &lResult);
	}

	if (lResult == 1)
	{
		bValid = true;
	}
	else
	{
		if (pPlot->isWater())
		{
			return false;
		}
	}

	if (!bValid)
	{
		return false;
	}

	if (!bTestVisible)
	{
		iRange = GC.getMIN_CITY_RANGE();

		for (iDX = -(iRange); iDX <= iRange; iDX++)
		{
			for (iDY = -(iRange); iDY <= iRange; iDY++)
			{
				pLoopPlot	= plotXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iDX, iDY);

				if (pLoopPlot != NULL)
				{
					if (pLoopPlot->isCity())
					{
						if (pLoopPlot->area() == pPlot->area())
						{
							if (!GET_PLAYER(pLoopPlot->getOwnerINLINE()).isNative())
							{
								return false;
							}
						}
					}
				}
			}
		}
	}

	return true;
}


void CvPlayer::found(int iX, int iY)
{
	if (!canFound(iX, iY))
	{
		return;
	}

	CvCity* pCity = initCity(iX, iY, true);
	FAssertMsg(pCity != NULL, "City is not assigned a valid value");

	for (int iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
	{
		BuildingTypes eLoopBuilding = ((BuildingTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iI)));

		if (eLoopBuilding != NO_BUILDING)
		{
			if (GC.getBuildingInfo(eLoopBuilding).getFreeStartEra() != NO_ERA)
			{
				if (GC.getGameINLINE().getStartEra() >= GC.getBuildingInfo(eLoopBuilding).getFreeStartEra())
				{
					if (pCity->canConstruct(eLoopBuilding))
					{
						pCity->setHasRealBuilding(eLoopBuilding, true);
					}
				}
			}
		}
	}

	if (getAdvancedStartPoints() < 0)
	{
		if (isHuman())
		{
			pCity->chooseProduction();
		}
		else
		{
			pCity->doFoundMessage();
		}
	}

	gDLL->getEventReporterIFace()->cityBuilt(pCity);
}


bool CvPlayer::canTrain(UnitTypes eUnit, bool bContinue, bool bTestVisible, bool bIgnoreCost) const
{
	PROFILE_FUNC();

	UnitClassTypes eUnitClass;
	eUnitClass = ((UnitClassTypes)(GC.getUnitInfo(eUnit).getUnitClassType()));

	FAssert(GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(eUnitClass) == eUnit);
	if (GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(eUnitClass) != eUnit)
	{
		return false;
	}

	if (!bIgnoreCost)
	{
		bool bCanBuild = false;
		for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
		{
			if (GC.getUnitInfo(eUnit).getYieldCost(iYield) > 0)
			{
				bCanBuild = true;
				break;
			}
		}

		if (!bCanBuild)
		{
			return false;
		}
	}

	if (GC.getGameINLINE().isOption(GAMEOPTION_ONE_CITY_CHALLENGE) && isHuman())
	{
		if (GC.getUnitInfo(eUnit).isFound())
		{
			return false;
		}
	}

	//if (!bTestVisible)
	{
		if (GC.getUnitInfo(eUnit).getSpecialUnitType() != NO_SPECIALUNIT)
		{
			if (!(GC.getGameINLINE().isSpecialUnitValid((SpecialUnitTypes)(GC.getUnitInfo(eUnit).getSpecialUnitType()))))
			{
				return false;
			}
		}
	}

	return true;
}


bool CvPlayer::canConstruct(BuildingTypes eBuilding, bool bContinue, bool bTestVisible, bool bIgnoreCost) const
{
	BuildingClassTypes eBuildingClass;
	CvTeamAI& currentTeam = GET_TEAM(getTeam());

	eBuildingClass = ((BuildingClassTypes)(GC.getBuildingInfo(eBuilding).getBuildingClassType()));

	FAssert(GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(eBuildingClass) == eBuilding);
	if (GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(eBuildingClass) != eBuilding)
	{
		return false;
	}

	if (!bIgnoreCost)
	{
		bool bCanBuild = false;
		for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
		{
			if (GC.getBuildingInfo(eBuilding).getYieldCost(iYield) > 0)
			{
				bCanBuild = true;
				break;
			}
		}

		if (!bCanBuild)
		{
			return false;
		}
	}
	if (GC.getGameINLINE().countCivTeamsEverAlive() < GC.getBuildingInfo(eBuilding).getNumTeamsPrereq())
	{
		return false;
	}

	if (GC.getBuildingInfo(eBuilding).getVictoryPrereq() != NO_VICTORY)
	{
		if (!(GC.getGameINLINE().isVictoryValid((VictoryTypes)(GC.getBuildingInfo(eBuilding).getVictoryPrereq()))))
		{
			return false;
		}
	}

	if (GC.getBuildingInfo(eBuilding).getMaxStartEra() != NO_ERA)
	{
		if (GC.getGameINLINE().getStartEra() > GC.getBuildingInfo(eBuilding).getMaxStartEra())
		{
			return false;
		}
	}

	//if (!bTestVisible)
	{
		if (GC.getBuildingInfo(eBuilding).getSpecialBuildingType() != NO_SPECIALBUILDING)
		{
			if (!(GC.getGameINLINE().isSpecialBuildingValid((SpecialBuildingTypes)(GC.getBuildingInfo(eBuilding).getSpecialBuildingType()))))
			{
				return false;
			}
		}

		if (getNumCities() < GC.getBuildingInfo(eBuilding).getNumCitiesPrereq())
		{
			return false;
		}

		if (getHighestUnitLevel() < GC.getBuildingInfo(eBuilding).getUnitLevelPrereq())
		{
			return false;
		}
		for (int iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
		{
			if (getBuildingClassCount((BuildingClassTypes)iI) < getBuildingClassPrereqBuilding(eBuilding, ((BuildingClassTypes)iI), ((bContinue) ? 0 : getBuildingClassMaking(eBuildingClass))))
			{
				return false;
			}
		}
	}

	return true;
}

int CvPlayer::getYieldProductionNeeded(UnitTypes eUnit, YieldTypes eYield) const
{
	UnitClassTypes eUnitClass = (UnitClassTypes)GC.getUnitInfo(eUnit).getUnitClassType();
	FAssert(NO_UNITCLASS != eUnitClass);

	int iProductionNeeded = GC.getUnitInfo(eUnit).getYieldCost(eYield);

	iProductionNeeded *= GC.getDefineINT("UNIT_PRODUCTION_PERCENT");
	iProductionNeeded /= 100;

	iProductionNeeded *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent();
	iProductionNeeded /= 100;

	iProductionNeeded *= GC.getEraInfo(GC.getGameINLINE().getStartEra()).getTrainPercent();
	iProductionNeeded /= 100;

	if (!isHuman() && !isNative())
	{
		iProductionNeeded *= GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAITrainPercent();
		iProductionNeeded /= 100;

		iProductionNeeded *= std::max(0, ((GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIPerEraModifier() * getCurrentEra()) + 100));
		iProductionNeeded /= 100;
	}

	// Python cost modifier
	if(GC.getUSE_GET_UNIT_COST_MOD_CALLBACK())
	{
		CyArgsList argsList;
		argsList.add(getID());	// Player ID
		argsList.add((int)eUnit);
		argsList.add((int)eYield);
		long lResult=0;
		gDLL->getPythonIFace()->callFunction(PYGameModule, "getUnitCostMod", argsList.makeFunctionArgs(), &lResult);

		if (lResult > 1)
		{
			iProductionNeeded *= lResult;
			iProductionNeeded /= 100;
		}
	}

	return iProductionNeeded;
}


int CvPlayer::getYieldProductionNeeded(BuildingTypes eBuilding, YieldTypes eYield) const
{
	int iProductionNeeded = GC.getBuildingInfo(eBuilding).getYieldCost(eYield);

	iProductionNeeded *= GC.getDefineINT("BUILDING_PRODUCTION_PERCENT");
	iProductionNeeded /= 100;

	iProductionNeeded *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getConstructPercent();
	iProductionNeeded /= 100;

	iProductionNeeded *= GC.getEraInfo(GC.getGameINLINE().getStartEra()).getConstructPercent();
	iProductionNeeded /= 100;

	iProductionNeeded *= 100 + getBuildingRequiredYieldModifier(eYield);
	iProductionNeeded /= 100;

	if (!isHuman())
	{
		iProductionNeeded *= GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIConstructPercent();
		iProductionNeeded /= 100;

		iProductionNeeded *= std::max(0, ((GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIPerEraModifier() * getCurrentEra()) + 100));
		iProductionNeeded /= 100;
	}

	return iProductionNeeded;
}


int CvPlayer::getProductionModifier(UnitTypes eUnit) const
{
	int iMultiplier = 0;

	if (GC.getUnitInfo(eUnit).isMilitaryProduction())
	{
		iMultiplier += getMilitaryProductionModifier();
	}

	for (int iI = 0; iI < GC.getNumTraitInfos(); iI++)
	{
		if (hasTrait((TraitTypes)iI))
		{
			iMultiplier += GC.getUnitInfo(eUnit).getProductionTraits(iI);

			if (GC.getUnitInfo(eUnit).getSpecialUnitType() != NO_SPECIALUNIT)
			{
				iMultiplier += GC.getSpecialUnitInfo((SpecialUnitTypes) GC.getUnitInfo(eUnit).getSpecialUnitType()).getProductionTraits(iI);
			}
		}
	}

	return iMultiplier;
}

int CvPlayer::getProductionModifier(BuildingTypes eBuilding) const
{
	int iMultiplier = 0;
	for (int iI = 0; iI < GC.getNumTraitInfos(); iI++)
	{
		if (hasTrait((TraitTypes)iI))
		{
			iMultiplier += GC.getBuildingInfo(eBuilding).getProductionTraits(iI);

			if (GC.getBuildingInfo(eBuilding).getSpecialBuildingType() != NO_SPECIALBUILDING)
			{
				iMultiplier += GC.getSpecialBuildingInfo((SpecialBuildingTypes) GC.getBuildingInfo(eBuilding).getSpecialBuildingType()).getProductionTraits(iI);
			}
		}
	}

	BuildingClassTypes eBuildingClass = (BuildingClassTypes) GC.getBuildingInfo(eBuilding).getBuildingClassType();
	for (int iI = 0; iI < GC.getNumTraitInfos(); iI++)
	{
		TraitTypes eTrait = (TraitTypes) iI;
		if (hasTrait(eTrait))
		{
			iMultiplier += GC.getTraitInfo(eTrait).getBuildingProductionModifier(eBuildingClass);
		}
	}

	return iMultiplier;
}

int CvPlayer::getBuildingClassPrereqBuilding(BuildingTypes eBuilding, BuildingClassTypes ePrereqBuildingClass, int iExtra) const
{
	CvBuildingInfo& kBuilding = GC.getBuildingInfo(eBuilding);

	int iPrereqs = kBuilding.getPrereqNumOfBuildingClass(ePrereqBuildingClass);
	
	// dont bother with the rest of the calcs if we have no prereqs
	if (iPrereqs < 1)
	{
		return 0;
	}

	BuildingClassTypes eBuildingClass = (BuildingClassTypes)kBuilding.getBuildingClassType();

	iPrereqs *= std::max(0, (GC.getWorldInfo(GC.getMapINLINE().getWorldSize()).getBuildingClassPrereqModifier() + 100));
	iPrereqs /= 100;

	iPrereqs *= (getBuildingClassCount((BuildingClassTypes)(GC.getBuildingInfo(eBuilding).getBuildingClassType())) + iExtra + 1);

	if (GC.getGameINLINE().isOption(GAMEOPTION_ONE_CITY_CHALLENGE) && isHuman())
	{
		iPrereqs = std::min(1, iPrereqs);
	}

	return iPrereqs;
}


void CvPlayer::removeBuildingClass(BuildingClassTypes eBuildingClass)
{
	CvCity* pLoopCity;
	BuildingTypes eBuilding;
	int iLoop;

	eBuilding = ((BuildingTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(eBuildingClass)));

	if (eBuilding != NO_BUILDING)
	{
		for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
		{
			if (pLoopCity->isHasRealBuilding(eBuilding))
			{
				pLoopCity->setHasRealBuilding(eBuilding, false);
				break;
			}
		}
	}
}

void CvPlayer::processTrait(TraitTypes eTrait, int iChange)
{
	CvTraitInfo& kTrait = GC.getTraitInfo(eTrait);

	changeTraitCount(eTrait, iChange);

	changeLevelExperienceModifier(kTrait.getLevelExperienceModifier() * iChange);
	changeGreatGeneralRateModifier(kTrait.getGreatGeneralRateModifier() * iChange);
	changeDomesticGreatGeneralRateModifier(kTrait.getDomesticGreatGeneralRateModifier() * iChange);

	changeNativeAngerModifier(kTrait.getNativeAngerModifier() * iChange);
	changeNativeCombatModifier(kTrait.getNativeCombatModifier() * iChange);
	changeMissionaryRateModifier(kTrait.getMissionaryModifier() * iChange);

	for (int iProfession = 0; iProfession < GC.getNumProfessionInfos(); ++iProfession)
	{
		ProfessionTypes eProfession = (ProfessionTypes) iProfession;
		int iNewValue = getProfessionEquipmentModifier(eProfession) + kTrait.getProfessionEquipmentModifier(eProfession) * iChange;
		setProfessionEquipmentModifier(eProfession, iNewValue);
	}

	int iLoop;
	std::vector<CvUnit*> apUnits;
	for (CvUnit* pLoopUnit = firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = nextUnit(&iLoop))
	{
		apUnits.push_back(pLoopUnit);
	}
	for (uint i = 0; i < m_aEuropeUnits.size(); ++i)
	{
		apUnits.push_back(m_aEuropeUnits[i]);
	}

	for (uint i = 0; i < apUnits.size(); ++i)
	{
		CvUnit* pLoopUnit = apUnits[i];
		for (int iJ = 0; iJ < GC.getNumPromotionInfos(); iJ++)
		{
			if (kTrait.isFreePromotion(iJ))
			{
				if ((pLoopUnit->getUnitCombatType() != NO_UNITCOMBAT) && kTrait.isFreePromotionUnitCombat(pLoopUnit->getUnitCombatType()))
				{
					pLoopUnit->changeFreePromotionCount(((PromotionTypes)iJ), iChange);
				}
			}
		}
	}

	bool bUpdatePlotYields = false;
	for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
	{
		YieldTypes eYield = (YieldTypes) iYield;

		changeYieldRateModifier(eYield, iChange * kTrait.getYieldModifier(iYield));
		changeBuildingRequiredYieldModifier(eYield, kTrait.getBuildingRequiredYieldModifier(eYield) * iChange);

		if (kTrait.isTaxYieldModifier(iYield))
		{
			changeTaxYieldModifierCount(eYield, iChange);
		}

		for (int iBuildingClass = 0; iBuildingClass < GC.getNumBuildingClassInfos(); ++iBuildingClass)
		{
			changeBuildingYieldChange((BuildingClassTypes) iBuildingClass, eYield, iChange * kTrait.getBuildingYieldChange(iBuildingClass, iYield));
		}

		updateExtraYieldThreshold(eYield);

		if (kTrait.getCityExtraYield(iYield) != 0 || kTrait.getExtraYieldThreshold(iYield) != 0)
		{
			bUpdatePlotYields = true;
		}
	}

	if (bUpdatePlotYields)
	{
		updateYield();
	}

	for (int iUnitClass = 0; iUnitClass < GC.getNumUnitClassInfos(); ++iUnitClass)
	{
		changeUnitMoveChange((UnitClassTypes) iUnitClass, iChange * kTrait.getUnitMoveChange(iUnitClass));
		changeUnitStrengthModifier((UnitClassTypes) iUnitClass, iChange * kTrait.getUnitStrengthModifier(iUnitClass));
	}

	for (int iProfession = 0; iProfession < GC.getNumProfessionInfos(); ++iProfession)
	{
		changeProfessionMoveChange((ProfessionTypes) iProfession, iChange * kTrait.getProfessionMoveChange(iProfession));
	}

	for (int iBuildingClass = 0; iBuildingClass < GC.getNumBuildingClassInfos(); ++iBuildingClass)
	{
		if (kTrait.isFreeBuildingClass(iBuildingClass))
		{
			BuildingTypes eFreeBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iBuildingClass);
			if (eFreeBuilding != NO_BUILDING)
			{
				changeFreeBuildingCount(eFreeBuilding, iChange);
			}
		}
	}

	if (kTrait.getNativeAttitudeChange() != 0)
	{
		for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
		{
			CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes) iPlayer);

			if (kLoopPlayer.isAlive() && kLoopPlayer.isNative())
			{
				kLoopPlayer.AI_changeAttitudeExtra(getID(), kTrait.getNativeAttitudeChange());

				if (iChange > 0 && kTrait.getNativeAttitudeChange() > 0)
				{
					GET_TEAM(getTeam()).makePeace(kLoopPlayer.getTeam());
				}
			}
		}
	}

	changeCityDefenseModifier(iChange * kTrait.getCityDefense());
}


void CvPlayer::processFather(FatherTypes eFather, int iChange)
{
	CvFatherInfo& kFatherInfo = GC.getFatherInfo(eFather);
	if (kFatherInfo.getTrait() != NO_TRAIT)
	{
		processTrait((TraitTypes) kFatherInfo.getTrait(), iChange);
	}
}

void CvPlayer::processFatherOnce(FatherTypes eFather)
{
	CvFatherInfo& kFatherInfo = GC.getFatherInfo(eFather);

	for (int iUnitClass = 0; iUnitClass < GC.getNumUnitClassInfos(); ++iUnitClass)
	{
		UnitTypes eUnit = (UnitTypes) GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iUnitClass);
		if (eUnit != NO_UNIT)
		{
			CvPlot* pPlot = NULL;
			int iLoop;
			for (CvCity* pCity = firstCity(&iLoop); pCity != NULL && pPlot == NULL; pCity = nextCity(&iLoop))
			{
				CvPlot* pCityPlot = pCity->plot();
				if (pCityPlot->isValidDomainForAction(eUnit))
				{
					pPlot = pCityPlot;
				}
			}

			for (CvUnit* pLoopUnit = firstUnit(&iLoop); pLoopUnit != NULL && pPlot == NULL; pLoopUnit = nextUnit(&iLoop))
			{
				CvPlot* pUnitPlot = pLoopUnit->plot();
				if (pUnitPlot != NULL && pUnitPlot->isValidDomainForAction(eUnit))
				{
					pPlot = pUnitPlot;
				}
			}

			for (int i = 0; i < kFatherInfo.getFreeUnits(iUnitClass); ++i)
			{
				if (pPlot != NULL)
				{
					initUnit(eUnit, (ProfessionTypes) GC.getUnitInfo(eUnit).getDefaultProfession(), pPlot->getX_INLINE(), pPlot->getY_INLINE());
				}
				else if (canTradeWithEurope())
				{
					CvPlot* pStartingPlot = getStartingPlot();
					if (GC.getUnitInfo(eUnit).getDomainType() == DOMAIN_SEA && pStartingPlot != NULL)
					{
						CvUnit* pUnit = initUnit(eUnit, (ProfessionTypes) GC.getUnitInfo(eUnit).getDefaultProfession(), INVALID_PLOT_COORD, INVALID_PLOT_COORD);
						if (pUnit != NULL)
						{
							pUnit->setUnitTravelState(UNIT_TRAVEL_STATE_IN_EUROPE, false);
							//add unit to map after setting Europe state so that it doesn't bump enemy units
							pUnit->addToMap(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE());
						}
					}
					else
					{
						initEuropeUnit(eUnit);
					}
				}
			}
		}
	}

	for (int iImprovement = 0; iImprovement < GC.getNumImprovementInfos(); ++iImprovement)
	{
		if (kFatherInfo.isRevealImprovement(iImprovement))
		{
			for (int i = 0; i < GC.getMapINLINE().numPlotsINLINE(); ++i)
			{
				CvPlot* pPlot = GC.getMapINLINE().plotByIndexINLINE(i);

				if (pPlot->getImprovementType() == iImprovement)
				{
					pPlot->setRevealed(getTeam(), true, false, NO_TEAM);
				}
			}
		}
	}

	changeFatherScore(1);
}

int CvPlayer::getFatherPointMultiplier() const
{
	return m_iFatherPointMultiplier;
}

void CvPlayer::setFatherPointMultiplier(int iValue)
{
	m_iFatherPointMultiplier = iValue;
}

void CvPlayer::changeFatherPoints(FatherPointTypes ePointType, int iChange)
{
	GET_TEAM(getTeam()).changeFatherPoints(ePointType, iChange * getFatherPointMultiplier() / 100);
}

bool CvPlayer::canBuild(const CvPlot* pPlot, BuildTypes eBuild, bool bTestEra, bool bTestVisible) const
{
	PROFILE_FUNC();

	if (!(pPlot->canBuild(eBuild, getID(), bTestVisible)))
	{
		return false;
	}
	if (!bTestVisible)
	{
		if (std::max(0, getGold()) < getBuildCost(pPlot, eBuild))
		{
			return false;
		}
	}

	return true;
}

// Returns the cost
int CvPlayer::getBuildCost(const CvPlot* pPlot, BuildTypes eBuild) const
{
	FAssert(eBuild >= 0 && eBuild < GC.getNumBuildInfos());

	if (pPlot->getBuildProgress(eBuild) > 0)
	{
		return 0;
	}

	return std::max(0, GC.getBuildInfo(eBuild).getCost());
}




RouteTypes CvPlayer::getBestRoute(CvPlot* pPlot) const
{
	PROFILE_FUNC();

	RouteTypes eRoute;
	RouteTypes eBestRoute;
	int iValue;
	int iBestValue;
	int iI;

	iBestValue = 0;
	eBestRoute = NO_ROUTE;

	for (iI = 0; iI < GC.getNumBuildInfos(); iI++)
	{
		eRoute = ((RouteTypes)(GC.getBuildInfo((BuildTypes)iI).getRoute()));

		if (eRoute != NO_ROUTE)
		{
			if(pPlot != NULL)
			{
				if ((pPlot->getRouteType() == eRoute) || canBuild(pPlot, ((BuildTypes)iI)))
				{
					iValue = GC.getRouteInfo(eRoute).getValue();
					if (iValue > iBestValue)
					{
						iBestValue = iValue;
						eBestRoute = eRoute;
					}
				}
			}
		}
	}

	return eBestRoute;
}


int CvPlayer::getImprovementUpgradeRate() const
{
	int iRate;

	iRate = 1; // XXX

	iRate *= std::max(0, (getImprovementUpgradeRateModifier() + 100));
	iRate /= 100;

	return iRate;
}


int CvPlayer::calculateTotalYield(YieldTypes eYield) const
{
	CvCity* pLoopCity;
	int iTotal = 0;
	int iLoop = 0;

	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		iTotal += pLoopCity->calculateNetYield(eYield);
	}

	return iTotal;
}

void CvPlayer::calculateTotalYields(int aiYields[]) const
{
	CvCity* pLoopCity;
	int iTotal = 0;
	int iLoop = 0;

	for(int i=0;i<NUM_YIELD_TYPES;i++)
	{
		aiYields[i] = 0;
	}

	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		int aiTempYields[NUM_YIELD_TYPES];
		pLoopCity->calculateNetYields(aiTempYields);
		for(int i=0;i<NUM_YIELD_TYPES;i++)
		{
			aiYields[i] += aiTempYields[i];
		}
	}
}

bool CvPlayer::isCivic(CivicTypes eCivic) const
{
	int iI;
	for (iI = 0; iI < GC.getNumCivicOptionInfos(); iI++)
	{
		if (getCivic((CivicOptionTypes)iI) == eCivic)
		{
			return true;
		}
	}
	return false;
}

bool CvPlayer::canDoCivics(CivicTypes eCivic) const
{
	PROFILE_FUNC();

	if (eCivic == NO_CIVIC)
	{
		return true;
	}

	if(GC.getUSE_CAN_DO_CIVIC_CALLBACK())
	{
		CyArgsList argsList;
		argsList.add(getID());
		argsList.add(eCivic);
		long lResult=0;
		gDLL->getPythonIFace()->callFunction(PYGameModule, "canDoCivic", argsList.makeFunctionArgs(), &lResult);
		if (lResult == 1)
		{
			return true;
		}
	}

	if(GC.getUSE_CANNOT_DO_CIVIC_CALLBACK())
	{
		CyArgsList argsList2; // XXX
		argsList2.add(getID());
		argsList2.add(eCivic);
		long lResult=0;
		gDLL->getPythonIFace()->callFunction(PYGameModule, "cannotDoCivic", argsList2.makeFunctionArgs(), &lResult);
		if (lResult == 1)
		{
			return false;
		}
	}

	return true;
}

int CvPlayer::greatGeneralThreshold() const
{
	int iThreshold = ((GC.getDefineINT("GREAT_GENERALS_THRESHOLD") * std::max(0, (getGreatGeneralsThresholdModifier() + 100))) / 100);

	iThreshold *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getGreatGeneralPercent();
	iThreshold /= 100;
	iThreshold *= GC.getEraInfo(GC.getGameINLINE().getStartEra()).getGreatGeneralPercent();
	iThreshold /= 100;

	return std::max(1, iThreshold);
}

int CvPlayer::immigrationThreshold() const
{
	int iThreshold = ((GC.getDefineINT("IMMIGRATION_THRESHOLD") * std::max(0, (getImmigrationThresholdMultiplier()))) / 100);

	for (int iTrait = 0; iTrait < GC.getNumTraitInfos(); ++iTrait)
	{
		TraitTypes eTrait = (TraitTypes) iTrait;
		if (hasTrait(eTrait))
		{
			iThreshold *= 100 + GC.getTraitInfo(eTrait).getImmigrationThresholdModifier();
			iThreshold /= 100;
		}
	}

	iThreshold *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getGrowthPercent();
	iThreshold /= 100;

	iThreshold *= GC.getEraInfo(GC.getGameINLINE().getStartEra()).getGrowthPercent();
	iThreshold /= 100;
	
	if (!isHuman())
	{
		iThreshold *= GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIGrowthPercent();
		iThreshold /= 100;
	}

	return std::max(1, iThreshold);
}

int CvPlayer::revolutionEuropeUnitThreshold() const
{
	int iThreshold = ((GC.getDefineINT("REVOLUTION_EUROPE_UNIT_THRESHOLD") * std::max(0, (getRevolutionEuropeUnitThresholdMultiplier()))) / 100);

	iThreshold *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent();
	iThreshold /= 100;

	iThreshold *= GC.getEraInfo(GC.getGameINLINE().getStartEra()).getTrainPercent();
	iThreshold /= 100;

	iThreshold *= GC.getHandicapInfo(getHandicapType()).getAITrainPercent();
	iThreshold /= 100;

	if (!isHuman())
	{
		iThreshold *= GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIKingUnitThresholdPercent();
		iThreshold /= 100;
	}

	return std::max(1, iThreshold);
}

CvPlot* CvPlayer::getStartingPlot() const
{
	return GC.getMapINLINE().plotSorenINLINE(m_iStartingX, m_iStartingY);
}


void CvPlayer::setStartingPlot(CvPlot* pNewValue, bool bUpdateStartDist)
{
	CvPlot* pOldStartingPlot;

	pOldStartingPlot = getStartingPlot();

	if (pOldStartingPlot != pNewValue)
	{
		if (pOldStartingPlot != NULL)
		{
			pOldStartingPlot->area()->changeNumStartingPlots(-1);

			if (bUpdateStartDist)
			{
				GC.getMapINLINE().updateMinOriginalStartDist(pOldStartingPlot->area());
			}
		}

		if (pNewValue == NULL)
		{
			m_iStartingX = INVALID_PLOT_COORD;
			m_iStartingY = INVALID_PLOT_COORD;
		}
		else
		{
			m_iStartingX = pNewValue->getX_INLINE();
			m_iStartingY = pNewValue->getY_INLINE();

			getStartingPlot()->area()->changeNumStartingPlots(1);

			if (bUpdateStartDist)
			{
				GC.getMapINLINE().updateMinOriginalStartDist(getStartingPlot()->area());
			}
		}
	}
}


int CvPlayer::getTotalPopulation() const
{
	return m_iTotalPopulation;
}


int CvPlayer::getAveragePopulation() const
{
	if (getNumCities() == 0)
	{
		return 0;
	}

	return ((getTotalPopulation() / getNumCities()) + 1);
}


void CvPlayer::changeTotalPopulation(int iChange)
{
	changePopScore(-getTotalPopulation());

	m_iTotalPopulation += iChange;
	FAssert(getTotalPopulation() >= 0);

	changePopScore(getTotalPopulation());
}


long CvPlayer::getRealPopulation() const
{
	__int64 iTotalPopulation = 0;
	int iLoop = 0;

	for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		iTotalPopulation += pLoopCity->getRealPopulation();
	}

	if (iTotalPopulation > MAX_INT)
	{
		iTotalPopulation = MAX_INT;
	}

	return ((long)(iTotalPopulation));
}


int CvPlayer::getTotalLand() const
{
	return m_iTotalLand;
}


void CvPlayer::changeTotalLand(int iChange)
{
	m_iTotalLand = (m_iTotalLand + iChange);
	FAssert(getTotalLand() >= 0);
}


int CvPlayer::getTotalLandScored() const
{
	return m_iTotalLandScored;
}


void CvPlayer::changeTotalLandScored(int iChange)
{
	if (iChange != 0)
	{
		changeLandScore(-getTotalLandScored());

		m_iTotalLandScored = (m_iTotalLandScored + iChange);
		FAssert(getTotalLandScored() >= 0);

		changeLandScore(getTotalLandScored());
	}
}


int CvPlayer::getGold() const
{
	return m_iGold;
}


void CvPlayer::setGold(int iNewValue)
{
	if (getGold() != iNewValue)
	{
		m_iGold = iNewValue;

		FAssert(getGold() >= 0);

		if (getID() == GC.getGameINLINE().getActivePlayer())
		{
			gDLL->getInterfaceIFace()->setDirty(MiscButtons_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(GameData_DIRTY_BIT, true);
		}
	}
}

void CvPlayer::changeGold(int iChange)
{
	setGold(getGold() + iChange);
	if (iChange > 0)
	{
		GET_PLAYER(getID()).AI_changeTotalIncome(iChange);
	}
}

int CvPlayer::getExtraTradeMultiplier(PlayerTypes eOtherPlayer) const
{
	int iMultiplier = 100;
	if (!isHuman() && !GET_PLAYER(eOtherPlayer).isHuman())
	{
		iMultiplier += GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIExtraTradePercent();
	}
	return iMultiplier;
}

int CvPlayer::getAdvancedStartPoints() const
{
	return m_iAdvancedStartPoints;
}


void CvPlayer::setAdvancedStartPoints(int iNewValue)
{
	if (getAdvancedStartPoints() != iNewValue)
	{
		m_iAdvancedStartPoints = iNewValue;

		if (getID() == GC.getGameINLINE().getActivePlayer())
		{
			gDLL->getInterfaceIFace()->setDirty(MiscButtons_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(GameData_DIRTY_BIT, true);
		}
	}
}

void CvPlayer::changeAdvancedStartPoints(int iChange)
{
	setAdvancedStartPoints(getAdvancedStartPoints() + iChange);
}

int CvPlayer::getGreatGeneralsCreated() const
{
	return m_iGreatGeneralsCreated;
}

void CvPlayer::incrementGreatGeneralsCreated()
{
	m_iGreatGeneralsCreated++;
}

int CvPlayer::getGreatGeneralsThresholdModifier() const
{
	return m_iGreatGeneralsThresholdModifier;
}

void CvPlayer::changeGreatGeneralsThresholdModifier(int iChange)
{
	m_iGreatGeneralsThresholdModifier += iChange;
}

int CvPlayer::getGreatGeneralRateModifier() const
{
	return m_iGreatGeneralRateModifier;
}

void CvPlayer::changeGreatGeneralRateModifier(int iChange)
{
	m_iGreatGeneralRateModifier += iChange;
}


int CvPlayer::getDomesticGreatGeneralRateModifier() const
{
	return (GC.getDefineINT("COMBAT_EXPERIENCE_IN_BORDERS_PERCENT") + m_iDomesticGreatGeneralRateModifier);
}


void CvPlayer::changeDomesticGreatGeneralRateModifier(int iChange)													
{
	m_iDomesticGreatGeneralRateModifier += iChange;
}


int CvPlayer::getImmigrationThresholdMultiplier() const
{
	return m_iImmigrationThresholdMultiplier;
}


void CvPlayer::setImmigrationThresholdMultiplier(int iValue)													
{
	m_iImmigrationThresholdMultiplier = iValue;
}

int CvPlayer::getRevolutionEuropeUnitThresholdMultiplier() const
{
	return m_iRevolutionEuropeUnitThresholdMultiplier;
}

void CvPlayer::setRevolutionEuropeUnitThresholdMultiplier(int iValue)													
{
	m_iRevolutionEuropeUnitThresholdMultiplier = iValue;
}

int CvPlayer::getKingNumUnitMultiplier() const
{
	return m_iKingNumUnitMultiplier;
}

void CvPlayer::setKingNumUnitMultiplier(int iValue)													
{
	m_iKingNumUnitMultiplier = iValue;
}

int CvPlayer::getNativeAngerModifier() const												
{
	return m_iNativeAngerModifier;
}

void CvPlayer::changeNativeAngerModifier(int iChange)
{
	m_iNativeAngerModifier += iChange;
}

int CvPlayer::getFreeExperience() const																
{
	return m_iFreeExperience;
}


void CvPlayer::changeFreeExperience(int iChange)
{
	m_iFreeExperience += iChange;
}

int CvPlayer::getWorkerSpeedModifier() const
{
	return m_iWorkerSpeedModifier;
}


void CvPlayer::changeWorkerSpeedModifier(int iChange)
{
	m_iWorkerSpeedModifier += iChange;
}


int CvPlayer::getImprovementUpgradeRateModifier() const
{
	return m_iImprovementUpgradeRateModifier;
}


void CvPlayer::changeImprovementUpgradeRateModifier(int iChange)
{
	m_iImprovementUpgradeRateModifier += iChange;
}


int CvPlayer::getMilitaryProductionModifier() const
{
	return m_iMilitaryProductionModifier;
}


void CvPlayer::changeMilitaryProductionModifier(int iChange)
{
	m_iMilitaryProductionModifier += iChange;
}

int CvPlayer::getCityDefenseModifier() const
{
	return m_iCityDefenseModifier;
}

void CvPlayer::changeCityDefenseModifier(int iChange)
{
	if (iChange != 0)
	{
		m_iCityDefenseModifier += iChange;
		setCityBillboardDirty(true);
	}
}

int CvPlayer::getHighestUnitLevel()	const																
{
	return m_iHighestUnitLevel;
}


void CvPlayer::setHighestUnitLevel(int iNewValue)
{
	m_iHighestUnitLevel = iNewValue;
	FAssert(getHighestUnitLevel() >= 0);
}

int CvPlayer::getFatherOverflowBells() const
{
	return m_iFatherOverflowBells;
}

void CvPlayer::setFatherOverflowBells(int iNewValue)
{
	m_iFatherOverflowBells = iNewValue;
	FAssert(getFatherOverflowBells() >= 0);
}

void CvPlayer::changeFatherOverflowBells(int iChange)
{
	setFatherOverflowBells(getFatherOverflowBells() + iChange);
}

int CvPlayer::getExpInBorderModifier() const
{
	return m_iExpInBorderModifier;
}

void CvPlayer::changeExpInBorderModifier(int iChange)
{
	if (iChange != 0)
	{
		m_iExpInBorderModifier += iChange;
		FAssert(getExpInBorderModifier() >= 0);
	}
}

int CvPlayer::getLevelExperienceModifier() const
{
	return m_iLevelExperienceModifier;
}

void CvPlayer::changeLevelExperienceModifier(int iChange)
{
	m_iLevelExperienceModifier += iChange;
}

CvCity* CvPlayer::getCapitalCity() const	 
{
	return getCity(m_iCapitalCityID);
}

CvCity* CvPlayer::getPrimaryCity() const	 
{
	CvCity* pCity = getCapitalCity();

	if (pCity == NULL)
	{
		int iIndex;
		pCity = firstCity(&iIndex);
	}

	return pCity;
}

void CvPlayer::setCapitalCity(CvCity* pNewCapitalCity)
{
	CvCity* pOldCapitalCity = getCapitalCity();

	if (pOldCapitalCity != pNewCapitalCity)
	{
		if (pNewCapitalCity != NULL)
		{
			m_iCapitalCityID = pNewCapitalCity->getID();
		}
		else
		{
			m_iCapitalCityID = FFreeList::INVALID_INDEX;
		}

		if (pOldCapitalCity != NULL)
		{
			pOldCapitalCity->setBillboardDirty(true);
		}
		if (pNewCapitalCity != NULL)
		{
			pNewCapitalCity->setBillboardDirty(true);
		}
	}
}

int CvPlayer::getCitiesLost() const
{
	return m_iCitiesLost;
}


void CvPlayer::changeCitiesLost(int iChange)
{
	m_iCitiesLost += iChange;
}

int CvPlayer::getAssets() const
{
	return m_iAssets;
}

void CvPlayer::changeAssets(int iChange)
{
	m_iAssets += iChange;
	FAssert(getAssets() >= 0);
}

int CvPlayer::getPower() const
{
	return m_iPower;
}

void CvPlayer::changePower(int iChange)
{
	m_iPower += iChange;
	FAssert(getPower() >= 0);
}


int CvPlayer::getPopScore() const		 
{
	return (m_iPopulationScore / std::max(1, GET_TEAM(getTeam()).getNumMembers()));
}

void CvPlayer::changePopScore(int iChange)
{
	if (iChange != 0)
	{
		m_iPopulationScore += iChange;
		FAssert(getPopScore() >= 0);

		GC.getGameINLINE().setScoreDirty(true);
	}
}


int CvPlayer::getLandScore() const		 
{
	return (m_iLandScore  / std::max(1, GET_TEAM(getTeam()).getNumMembers()));
}


void CvPlayer::changeLandScore(int iChange)
{
	if (iChange != 0)
	{
		m_iLandScore += iChange;
		FAssert(getLandScore() >= 0);

		GC.getGameINLINE().setScoreDirty(true);
	}
}

int CvPlayer::getFatherScore() const
{
	return m_iFatherScore;
}

void CvPlayer::changeFatherScore(int iChange)
{
	if (iChange != 0)
	{
		m_iFatherScore += iChange;
		FAssert(getFatherScore() >= 0);
		GC.getGameINLINE().setScoreDirty(true);
	}
}

int CvPlayer::getCombatExperience() const
{
	return m_iCombatExperience;
}

void CvPlayer::setCombatExperience(int iExperience)
{
	FAssert(iExperience >= 0);

	if (iExperience != getCombatExperience())
	{
		m_iCombatExperience = iExperience;

		int iExperienceThreshold = greatGeneralThreshold();
		if (getCombatExperience() >= iExperienceThreshold && iExperienceThreshold > 0)
		{
			// create great person
			CvCity* pBestCity = NULL;
			int iBestValue = MAX_INT;
			int iLoop;
			for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
			{
				int iValue = 4 * GC.getGameINLINE().getSorenRandNum(getNumCities(), "Warlord City Selection");

				for (int i = 0; i < NUM_YIELD_TYPES; i++)
				{
					iValue += pLoopCity->findYieldRateRank((YieldTypes)i);
				}
				iValue += pLoopCity->findPopulationRank();

				if (iValue < iBestValue)
				{
					pBestCity = pLoopCity;
					iBestValue = iValue;
				}
			}

			if (pBestCity)
			{
				int iRandOffset = GC.getGameINLINE().getSorenRandNum(GC.getNumUnitInfos(), "Warlord Unit Generation");
				for (int iI = 0; iI < GC.getNumUnitInfos(); iI++)
				{
					UnitTypes eLoopUnit = (UnitTypes)((iI + iRandOffset) % GC.getNumUnitInfos());
					if (GC.getUnitInfo(eLoopUnit).getLeaderExperience() > 0 || GC.getUnitInfo(eLoopUnit).getLeaderPromotion() != NO_PROMOTION)
					{
						pBestCity->createGreatGeneral(eLoopUnit, true);
						setCombatExperience(getCombatExperience() - iExperienceThreshold);
						break;
					}
				}
			}
		}
	}
}

void CvPlayer::changeCombatExperience(int iChange)
{
	setCombatExperience(getCombatExperience() + iChange);

	for (int i = 0; i <  GC.getNumFatherPointInfos(); ++i)
	{
		FatherPointTypes ePointType = (FatherPointTypes) i;
		changeFatherPoints(ePointType, iChange * GC.getFatherPointInfo(ePointType).getExperiencePoints());
	}
}


bool CvPlayer::isConnected() const
{
	return gDLL->isConnected( getNetID() );
}

int CvPlayer::getNetID() const
{
	return GC.getInitCore().getNetID(getID());
}


void CvPlayer::setNetID(int iNetID)
{
	GC.getInitCore().setNetID(getID(), iNetID);
}

void CvPlayer::sendReminder()
{
	CvWString szYearStr;

	// Only perform this step if we have a valid email address on record,
	// and we have provided information about how to send emails
	if ( !getPbemEmailAddress().empty() && 
		!gDLL->GetPitbossSmtpHost().empty() )
	{
		GAMETEXT.setTimeStr(szYearStr, GC.getGameINLINE().getGameTurn(), true);

		// Generate our arguments
		CyArgsList argsList;
		argsList.add(getPbemEmailAddress());
		argsList.add(gDLL->GetPitbossSmtpHost());
		argsList.add(gDLL->GetPitbossSmtpLogin());
		argsList.add(gDLL->GetPitbossSmtpPassword());
		argsList.add(GC.getGameINLINE().getName());
		argsList.add(GC.getGameINLINE().isMPOption(MPOPTION_TURN_TIMER));
		argsList.add(GC.getGameINLINE().getPitbossTurnTime());
		argsList.add(gDLL->GetPitbossEmail());
		argsList.add(szYearStr);

		// Now send our email via Python
		long iResult;
		bool bOK = gDLL->getPythonIFace()->callFunction(PYPitBossModule, "sendEmail", argsList.makeFunctionArgs(), &iResult);

		FAssertMsg( bOK, "Pitboss Python call to onSendEmail failed!" );
		FAssertMsg( iResult == 0, "Pitboss Python fn onSendEmail encountered an error" );
	}
}

uint CvPlayer::getStartTime() const
{
	return m_uiStartTime;
}


void CvPlayer::setStartTime(uint uiStartTime)
{
	m_uiStartTime = uiStartTime;
}


uint CvPlayer::getTotalTimePlayed() const
{
	return ((timeGetTime() - m_uiStartTime)/1000);
}


bool CvPlayer::isAlive() const
{
	return m_bAlive;
}


bool CvPlayer::isEverAlive() const
{
	return m_bEverAlive;
}


void CvPlayer::setAlive(bool bNewValue)
{
	CvWString szBuffer;
	int iI;

	if (isAlive() != bNewValue)
	{
		m_bAlive = bNewValue;

		GET_TEAM(getTeam()).changeAliveCount((isAlive()) ? 1 : -1);

		// Report event to Python
		gDLL->getEventReporterIFace()->setPlayerAlive(getID(), bNewValue);

		if (isAlive())
		{
			if (!isEverAlive())
			{
				m_bEverAlive = true;

				GET_TEAM(getTeam()).changeEverAliveCount(1);
			}

			if (getNumCities() == 0)
			{
				setFoundedFirstCity(false);
			}

			if (GC.getGameINLINE().isMPOption(MPOPTION_SIMULTANEOUS_TURNS) || (GC.getGameINLINE().getNumGameTurnActive() == 0) || (GC.getGameINLINE().isSimultaneousTeamTurns() && GET_TEAM(getTeam()).isTurnActive()))
			{
				setTurnActive(true);
			}

			gDLL->openSlot(getID());
		}
		else
		{
			killUnits();
			killCities();
			killAllDeals();

			setTurnActive(false);

			gDLL->endMPDiplomacy();
			gDLL->endDiplomacy();

			if (!isHuman())
			{
				gDLL->closeSlot(getID());
			}

			if (!GC.getGameINLINE().isGameMultiPlayer())
			{
				if (GC.getGameINLINE().getActivePlayer() == getID() && GC.getGameINLINE().getAIAutoPlay() <= 0 && !gDLL->GetAutorun())
				{
					GC.getGameINLINE().setGameState(GAMESTATE_OVER);
				}
			}

			if (GC.getGameINLINE().getElapsedGameTurns() > 0)
			{
				szBuffer = gDLL->getText("TXT_KEY_MISC_CIV_DESTROYED", getCivilizationAdjectiveKey());

				for (iI = 0; iI < MAX_PLAYERS; iI++)
				{
					if (GET_PLAYER((PlayerTypes)iI).isAlive())
					{
						gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_CIVDESTROYED", MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_WARNING_TEXT"));
					}
				}

				GC.getGameINLINE().addReplayMessage(REPLAY_MESSAGE_MAJOR_EVENT, getID(), szBuffer, -1, -1, (ColorTypes)GC.getInfoTypeForString("COLOR_WARNING_TEXT"));
			}
		}

		GC.getGameINLINE().setScoreDirty(true);
	}
}


void CvPlayer::verifyAlive()
{
	bool bKill;

	if (isAlive())
	{
		bKill = false;

		if (!bKill)
		{
			if (getNumCities() == 0 && !isEurope() && getAdvancedStartPoints() < 0)
			{
				if (isNative() || getNumUnits() == 0)
				{
					bKill = true;
				}
			}
		}

		if (bKill)
		{
			//don't kill colonist players
			if(getParent() != NO_PLAYER)
			{
				CvPlayer& kEurope = GET_PLAYER(getParent());
				if(kEurope.isAlive() && kEurope.isEurope() && !::atWar(getTeam(), kEurope.getTeam()) && (GC.getGameINLINE().getAIAutoPlay() == 0 || GC.getGameINLINE().getActivePlayer() != getID()))
				{
					initFreeUnits();
					if (isTurnActive())
					{
						setEndTurn(true);
					}

					//change taxrate
					int iOldTaxRate = getTaxRate();
					int iNewTaxRate = std::min(99, iOldTaxRate + 1 + GC.getGameINLINE().getSorenRandNum(GC.getDefineINT("TAX_RATE_MAX_INCREASE"), "Tax Rate Increase"));
					int iChange = iNewTaxRate - iOldTaxRate;
					changeTaxRate(iChange);

					if (isHuman())
					{
						CvDiploParameters* pDiplo = new CvDiploParameters(kEurope.getID());
						pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_KING_REVIVE"));
						pDiplo->addDiploCommentVariable(iNewTaxRate);
						pDiplo->setAIContact(true);
						gDLL->beginDiplomacy(pDiplo, getID());
					}

					bKill = false;
				}
			}
		}

		if (!bKill)
		{
			if (!isNative() && !isEurope())
			{
				if (GC.getGameINLINE().getMaxCityElimination() > 0)
				{
					if (getCitiesLost() >= GC.getGameINLINE().getMaxCityElimination())
					{
						bKill = true;
					}
				}
			}
		}

		if (bKill)
		{
			setAlive(false);
		}
	}
	else
	{
		if ((getNumCities() > 0) || (getNumUnits() > 0))
		{
			setAlive(true);
		}
	}
}

bool CvPlayer::isTurnActive() const
{
	return m_bTurnActive;
}

void CvPlayer::setTurnActiveForPbem(bool bActive)
{
	FAssertMsg(GC.getGameINLINE().isPbem(), "You are using setTurnActiveForPbem. Are you sure you know what you're doing?");

	// does nothing more than to set the member variable before saving the game
	// the rest of the turn will be performed upon loading the game
	// This allows the player to browse the game in paused mode after he has generated the save
	if (isTurnActive() != bActive)
	{
		m_bTurnActive = bActive;
		GC.getGameINLINE().changeNumGameTurnActive(isTurnActive() ? 1 : -1);
	}
}


void CvPlayer::setTurnActive(bool bNewValue, bool bDoTurn)
{
	if (isTurnActive() != bNewValue)
	{
		m_bTurnActive = bNewValue;

		if (isTurnActive())
		{
			if (GC.getLogging())
			{
				if (gDLL->getChtLvl() > 0)
				{
					TCHAR szOut[1024];
					sprintf(szOut, "Player %d Turn ON\n", getID());
					gDLL->messageControlLog(szOut);
				}
			}

			FAssertMsg(isAlive(), "isAlive is expected to be true");

			setEndTurn(false);
			GC.getGameINLINE().resetTurnTimer();

			// If we are the Pitboss, send this player an email
			if ( gDLL->IsPitbossHost() )
			{
				// If this guy is not currently connected, try sending him an email
				if ( isHuman() && !isConnected() )
				{
					sendReminder();
				}
			}

			if ((GC.getGameINLINE().isHotSeat() || GC.getGameINLINE().isPbem()) && isHuman() && bDoTurn)
			{
				gDLL->getInterfaceIFace()->clearEventMessages();
				gDLL->getEngineIFace()->setResourceLayer(false);

				GC.getGameINLINE().setActivePlayer(getID());
			}

			GC.getGameINLINE().changeNumGameTurnActive(1);

			if (bDoTurn)
			{
				if (isAlive() && !isHuman() && (getAdvancedStartPoints() >= 0))
				{
					AI_doAdvancedStart();
				}
						
				if (GC.getGameINLINE().getElapsedGameTurns() > 0)
				{
					if (isAlive())
					{
						if (GC.getGameINLINE().isMPOption(MPOPTION_SIMULTANEOUS_TURNS))
						{
							doTurn();
						}

						doTurnUnits();
					}
				}

				if ((getID() == GC.getGameINLINE().getActivePlayer()) && (GC.getGameINLINE().getElapsedGameTurns() > 0))
				{
					if (GC.getGameINLINE().isNetworkMultiPlayer())
					{
						gDLL->getInterfaceIFace()->addMessage(getID(), true, GC.getEVENT_MESSAGE_TIME(), gDLL->getText("TXT_KEY_MISC_TURN_BEGINS").GetCString(), "AS2D_NEWTURN", MESSAGE_TYPE_DISPLAY_ONLY);
					}
					else
					{
						gDLL->getInterfaceIFace()->playGeneralSound("AS2D_NEWTURN");
					}
				}

				doWarnings();
			}

			if (getID() == GC.getGameINLINE().getActivePlayer())
			{
				if (gDLL->getInterfaceIFace()->getLengthSelectionList() == 0)
				{
					gDLL->getInterfaceIFace()->setCycleSelectionCounter(1);
				}

				gDLL->getInterfaceIFace()->setDirty(SelectionCamera_DIRTY_BIT, true);
			}
		}
		else
		{
			if (GC.getLogging())
			{
				if (gDLL->getChtLvl() > 0)
				{
					TCHAR szOut[1024];
					sprintf(szOut, "Player %d Turn OFF\n", getID());
					gDLL->messageControlLog(szOut);
				}
			}

			if (getID() == GC.getGameINLINE().getActivePlayer())
			{
				gDLL->getInterfaceIFace()->setForcePopup(false);
				gDLL->getInterfaceIFace()->clearQueuedPopups();
				gDLL->getInterfaceIFace()->flushTalkingHeadMessages();
			}

			GC.getGameINLINE().changeNumGameTurnActive(-1);

			if (bDoTurn)
			{
				if (!GC.getGameINLINE().isMPOption(MPOPTION_SIMULTANEOUS_TURNS))
				{
					if (isAlive())
					{
						doTurn();
					}

					if ((GC.getGameINLINE().isPbem() || GC.getGameINLINE().isHotSeat()) && isHuman() && GC.getGameINLINE().countHumanPlayersAlive() > 1)
					{
						GC.getGameINLINE().setHotPbemBetweenTurns(true);
					}

					if (GC.getGameINLINE().isSimultaneousTeamTurns())
					{
						if (!GET_TEAM(getTeam()).isTurnActive())
						{
							for (int iI = (getTeam() + 1); iI < MAX_TEAMS; iI++)
							{
								if (GET_TEAM((TeamTypes)iI).isAlive())
								{
									GET_TEAM((TeamTypes)iI).setTurnActive(true);
									break;
								}
							}
						}
					}
					else
					{
						for (int iI = (getID() + 1); iI < MAX_PLAYERS; iI++)
						{
							if (GET_PLAYER((PlayerTypes)iI).isAlive())
							{
								if (GC.getGameINLINE().isPbem() && GET_PLAYER((PlayerTypes)iI).isHuman())
								{
									if (!GC.getGameINLINE().getPbemTurnSent())
									{
										gDLL->sendPbemTurn((PlayerTypes)iI);
									}
								}
								else
								{
									GET_PLAYER((PlayerTypes)iI).setTurnActive(true);
								}
								break;
							}
						}
					}
				}
			}
		}

		gDLL->getInterfaceIFace()->updateCursorType();

		gDLL->getInterfaceIFace()->setDirty(Score_DIRTY_BIT, true);
	}
}


bool CvPlayer::isAutoMoves() const																			
{
	return m_bAutoMoves;
}


void CvPlayer::setAutoMoves(bool bNewValue)
{
	if (isAutoMoves() != bNewValue)
	{
		m_bAutoMoves = bNewValue;

		if (!isAutoMoves())
		{
			if (isEndTurn() || !isHuman())
			{
				setTurnActive(false);
			}
			else
			{
				if (getID() == GC.getGameINLINE().getActivePlayer())
				{
					gDLL->getInterfaceIFace()->setCycleSelectionCounter(1);
				}
			}
		}
	}
}


bool CvPlayer::isEndTurn() const																			
{
	return m_bEndTurn;
}


void CvPlayer::setEndTurn(bool bNewValue)
{
	if (isEndTurn() != bNewValue)
	{
		FAssertMsg(isTurnActive(), "isTurnActive is expected to be true");

		m_bEndTurn = bNewValue;

		if (isEndTurn())
		{
			setAutoMoves(true);
		}
	}
}

bool CvPlayer::isTurnDone() const																			
{
	// if this returns true, popups and diplomacy will wait to appear until next turn
	if (!GC.getGameINLINE().isPbem() && !GC.getGameINLINE().isHotSeat())
	{
		return false;
	}
	if (!isHuman() )
	{
		return true;
	}
	if (!isEndTurn())
	{
		return false;
	}
	return (!isAutoMoves());
}

bool CvPlayer::isExtendedGame() const
{
	return m_bExtendedGame;
}


void CvPlayer::makeExtendedGame()
{
	m_bExtendedGame = true;
}


bool CvPlayer::isFoundedFirstCity() const
{
	return m_bFoundedFirstCity;
}


void CvPlayer::setFoundedFirstCity(bool bNewValue)
{
	if (isFoundedFirstCity() != bNewValue)
	{
		m_bFoundedFirstCity = bNewValue;
	}
}

PlayerTypes CvPlayer::getID() const
{
	return m_eID;
}


HandicapTypes CvPlayer::getHandicapType() const																
{
	return GC.getInitCore().getHandicap(getID());
}


CivilizationTypes CvPlayer::getCivilizationType() const														
{
	return GC.getInitCore().getCiv(getID());
}


LeaderHeadTypes CvPlayer::getLeaderType() const
{
	return GC.getInitCore().getLeader(getID());
}


LeaderHeadTypes CvPlayer::getPersonalityType() const
{
	return m_ePersonalityType;
}


void CvPlayer::setPersonalityType(LeaderHeadTypes eNewValue)
{
	m_ePersonalityType = eNewValue;
}


EraTypes CvPlayer::getCurrentEra() const
{
	return m_eCurrentEra;
}



void CvPlayer::setCurrentEra(EraTypes eNewValue)
{
	CvCity* pLoopCity;
	CvPlot* pLoopPlot;
	int iLoop;
	int iI;

	if (getCurrentEra() != eNewValue)
	{
		EraTypes eOldEra = m_eCurrentEra;
		m_eCurrentEra = eNewValue;

		if (GC.getGameINLINE().getActiveTeam() != NO_TEAM)
		{
			for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
			{
				pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);
				if (pLoopPlot->getRevealedImprovementType(GC.getGameINLINE().getActiveTeam(), true) != NO_IMPROVEMENT)
				{
					if ((pLoopPlot->getOwnerINLINE() == getID()) || (!(pLoopPlot->isOwned()) && (getID() == GC.getGameINLINE().getActivePlayer())))
					{
						pLoopPlot->setLayoutDirty(true);
					}
				}
			}
		}

		// dirty all of this player's cities...
		for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
		{
			if (pLoopCity->getOwnerINLINE() == getID())
			{
				pLoopCity->setLayoutDirty(true);
			}
		}

		if (getID() == GC.getGameINLINE().getActivePlayer())
		{
			gDLL->getInterfaceIFace()->setDirty(Soundtrack_DIRTY_BIT, true);
		}
	}
}


PlayerTypes CvPlayer::getParent() const
{
	return m_eParent;
}

void CvPlayer::setParent(PlayerTypes eParent)
{
	m_eParent = eParent;
}

TeamTypes CvPlayer::getTeam() const
{
	return m_eTeamType;
}

void CvPlayer::updateTeamType()
{
	if(getID() == NO_PLAYER)
	{
		m_eTeamType = NO_TEAM;
	}
	else
	{
		m_eTeamType = GC.getInitCore().getTeam(getID());
	}
}

void CvPlayer::setTeam(TeamTypes eTeam)
{
	FAssert(eTeam != NO_TEAM);
	FAssert(getTeam() != NO_TEAM);

	GET_TEAM(getTeam()).changeNumMembers(-1);
	if (isAlive())
	{
		GET_TEAM(getTeam()).changeAliveCount(-1);
	}
	if (isEverAlive())
	{
		GET_TEAM(getTeam()).changeEverAliveCount(-1);
	}
	GET_TEAM(getTeam()).changeNumCities(-(getNumCities()));
	GET_TEAM(getTeam()).changeTotalLand(-(getTotalLand()));

	GC.getInitCore().setTeam(getID(), eTeam);

	GET_TEAM(getTeam()).changeNumMembers(1);
	if (isAlive())
	{
		GET_TEAM(getTeam()).changeAliveCount(1);
	}
	if (isEverAlive())
	{
		GET_TEAM(getTeam()).changeEverAliveCount(1);
	}
	GET_TEAM(getTeam()).changeNumCities(getNumCities());
	GET_TEAM(getTeam()).changeTotalLand(getTotalLand());
}

YieldTypes CvPlayer::getImmigrationConversion() const
{
	return m_eImmigrationConversion;
}

void CvPlayer::setImmigrationConversion(YieldTypes eConversion)
{
	m_eImmigrationConversion = eConversion;
}

PlayerColorTypes CvPlayer::getPlayerColor() const
{
	return GC.getInitCore().getColor(getID());
}

int CvPlayer::getPlayerTextColorR() const
{
	FAssertMsg(getPlayerColor() != NO_PLAYERCOLOR, "getPlayerColor() is not expected to be equal with NO_PLAYERCOLOR");
	return ((int)(GC.getColorInfo((ColorTypes) GC.getPlayerColorInfo(getPlayerColor()).getTextColorType()).getColor().r * 255));
}

int CvPlayer::getPlayerTextColorG() const
{
	FAssertMsg(getPlayerColor() != NO_PLAYERCOLOR, "getPlayerColor() is not expected to be equal with NO_PLAYERCOLOR");
	return ((int)(GC.getColorInfo((ColorTypes) GC.getPlayerColorInfo(getPlayerColor()).getTextColorType()).getColor().g * 255));
}

int CvPlayer::getPlayerTextColorB() const
{
	FAssertMsg(getPlayerColor() != NO_PLAYERCOLOR, "getPlayerColor() is not expected to be equal with NO_PLAYERCOLOR");
	return ((int)(GC.getColorInfo((ColorTypes) GC.getPlayerColorInfo(getPlayerColor()).getTextColorType()).getColor().b * 255));
}

int CvPlayer::getPlayerTextColorA() const
{
	FAssertMsg(getPlayerColor() != NO_PLAYERCOLOR, "getPlayerColor() is not expected to be equal with NO_PLAYERCOLOR");
	return ((int)(GC.getColorInfo((ColorTypes) GC.getPlayerColorInfo(getPlayerColor()).getTextColorType()).getColor().a * 255));
}

int CvPlayer::getSeaPlotYield(YieldTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_YIELD_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiSeaPlotYield[eIndex];
}

void CvPlayer::changeSeaPlotYield(YieldTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_YIELD_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");

	if (iChange != 0)
	{
		m_aiSeaPlotYield[eIndex] = (m_aiSeaPlotYield[eIndex] + iChange);

		updateYield();
	}
}

int CvPlayer::getYieldRateModifier(YieldTypes eIndex) const		 
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_YIELD_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiYieldRateModifier[eIndex];
}

int CvPlayer::getTaxYieldRateModifier(YieldTypes eIndex) const		 
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_YIELD_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");

	if (getTaxYieldModifierCount(eIndex) > 0)
	{
		return getTaxRate();
	}

	return 0;
}

void CvPlayer::changeYieldRateModifier(YieldTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_YIELD_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");

	if (iChange != 0)
	{
		m_aiYieldRateModifier[eIndex] += iChange;

		invalidateYieldRankCache(eIndex);
		
		AI_makeAssignWorkDirty();

		if (getTeam() == GC.getGameINLINE().getActiveTeam())
		{
			gDLL->getInterfaceIFace()->setDirty(CityInfo_DIRTY_BIT, true);
		}
	}
}


int CvPlayer::getCapitalYieldRateModifier(YieldTypes eIndex) const		 
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_YIELD_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiCapitalYieldRateModifier[eIndex];
}


void CvPlayer::changeCapitalYieldRateModifier(YieldTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_YIELD_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");

	if (iChange != 0)
	{
		m_aiCapitalYieldRateModifier[eIndex] += iChange;

		invalidateYieldRankCache(eIndex);

		CvCity* pCapitalCity = getCapitalCity();
		if (pCapitalCity != NULL)
		{
			pCapitalCity->AI_setAssignWorkDirty(true);
			if (pCapitalCity->getTeam() == GC.getGameINLINE().getActiveTeam())
			{
				pCapitalCity->setBillboardDirty(true);
			}
		}
	}
}

int CvPlayer::getBuildingRequiredYieldModifier(YieldTypes eIndex) const		 
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_YIELD_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiBuildingRequiredYieldModifier[eIndex];
}


void CvPlayer::changeBuildingRequiredYieldModifier(YieldTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_YIELD_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");

	if (iChange != 0)
	{
		m_aiBuildingRequiredYieldModifier[eIndex] += iChange;
	}
}

int CvPlayer::getCityExtraYield(YieldTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_YIELD_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiCityExtraYield[eIndex];
}

void CvPlayer::updateCityExtraYield(YieldTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_YIELD_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");

	int iBestValue = 0;
	for (int iI = 0; iI < GC.getNumTraitInfos(); iI++)
	{
		if (hasTrait((TraitTypes)iI))
		{
			iBestValue += GC.getTraitInfo((TraitTypes) iI).getCityExtraYield(eIndex);
		}
	}

	if (getCityExtraYield(eIndex) != iBestValue)
	{
		m_aiCityExtraYield[eIndex] = iBestValue;
		FAssert(getCityExtraYield(eIndex) >= 0);

		updateYield();
	}
}


int CvPlayer::getExtraYieldThreshold(YieldTypes eIndex) const	
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_YIELD_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiExtraYieldThreshold[eIndex];
}


void CvPlayer::updateExtraYieldThreshold(YieldTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_YIELD_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");

	int iBestValue = 0;

	for (int iI = 0; iI < GC.getNumTraitInfos(); iI++)
	{
		if (hasTrait((TraitTypes)iI))
		{
			if (GC.getTraitInfo((TraitTypes) iI).getExtraYieldThreshold(eIndex) > 0)
			{
				if ((iBestValue == 0) || (GC.getTraitInfo((TraitTypes) iI).getExtraYieldThreshold(eIndex) < iBestValue))
				{
					iBestValue = GC.getTraitInfo((TraitTypes) iI).getExtraYieldThreshold(eIndex);
				}
			}
		}
	}

	if (getExtraYieldThreshold(eIndex) != iBestValue)
	{
		m_aiExtraYieldThreshold[eIndex] = iBestValue;
		FAssert(getExtraYieldThreshold(eIndex) >= 0);

		updateYield();
	}
}


int CvPlayer::getYieldRate(YieldTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_YIELD_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");

	if (getNumCities() == 0)
	{
		return 0;
	}

	int iTotalRate = GC.getCivilizationInfo(getCivilizationType()).getFreeYields(eIndex);
	int iLoop;
	for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		iTotalRate += pLoopCity->calculateNetYield(eIndex);
	}

	return iTotalRate;
}

bool CvPlayer::isYieldEuropeTradable(YieldTypes eYield) const
{
	FAssert(eYield >= 0 && eYield < NUM_YIELD_TYPES);

	if (getParent() == NO_PLAYER)
	{
		return false;
	}

	if (!GC.getYieldInfo(eYield).isCargo())
	{
		return false;
	}

	if (!canTradeWithEurope())
	{
		return false;
	}

	if (GET_PLAYER(getParent()).getYieldBuyPrice(eYield) <= 0)
	{
		return false;
	}

	return m_abYieldEuropeTradable[eYield];
}

void CvPlayer::setYieldEuropeTradable(YieldTypes eYield, bool bTradeable)
{
	FAssert(eYield >= 0 && eYield < NUM_YIELD_TYPES);

	bool bOldTradeable = isYieldEuropeTradable(eYield);

	m_abYieldEuropeTradable[eYield] = bTradeable;

	if (bOldTradeable != isYieldEuropeTradable(eYield))
	{
		validateTradeRoutes();
	}
}

void CvPlayer::setYieldEuropeTradableAll()
{
	for (int iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		setYieldEuropeTradable((YieldTypes)iI, true);
	}
}

bool CvPlayer::isFeatAccomplished(FeatTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_FEAT_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_abFeatAccomplished[eIndex];
}


void CvPlayer::setFeatAccomplished(FeatTypes eIndex, bool bNewValue)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_FEAT_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
	m_abFeatAccomplished[eIndex] = bNewValue;
}

bool CvPlayer::shouldDisplayFeatPopup(FeatTypes eIndex) const
{
	if (isFeatAccomplished(eIndex))
	{
		return false;
	}

	if (GC.getGameINLINE().isNetworkMultiPlayer())
	{
		return false;
	}

	if (!isOption(PLAYEROPTION_ADVISOR_POPUPS))
	{
		return false;
	}

	if (!isHuman())
	{
		return false;
	}

	if (GC.getGameINLINE().getElapsedGameTurns() == 0)
	{
		return false;
	}

	return true;
}

bool CvPlayer::isOption(PlayerOptionTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_PLAYEROPTION_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_abOptions[eIndex];
}


void CvPlayer::setOption(PlayerOptionTypes eIndex, bool bNewValue)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_PLAYEROPTION_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
	m_abOptions[eIndex] = bNewValue;
}

bool CvPlayer::isPlayable() const
{
	return GC.getInitCore().getPlayableCiv(getID());
}

void CvPlayer::setPlayable(bool bNewValue)
{
	GC.getInitCore().setPlayableCiv(getID(), bNewValue);
}

int CvPlayer::getImprovementCount(ImprovementTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumImprovementInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiImprovementCount[eIndex];
}


void CvPlayer::changeImprovementCount(ImprovementTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumImprovementInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_paiImprovementCount[eIndex] = (m_paiImprovementCount[eIndex] + iChange);
	FAssert(getImprovementCount(eIndex) >= 0);
}


int CvPlayer::getFreeBuildingCount(BuildingTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumBuildingInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiFreeBuildingCount[eIndex];
}


bool CvPlayer::isBuildingFree(BuildingTypes eIndex)	const															
{
	return (getFreeBuildingCount(eIndex) > 0);
}


void CvPlayer::changeFreeBuildingCount(BuildingTypes eIndex, int iChange)
{
	CvCity* pLoopCity;
	int iOldFreeBuildingCount;
	int iLoop;

	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumBuildingInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (iChange != 0)
	{
		iOldFreeBuildingCount = getFreeBuildingCount(eIndex);

		m_paiFreeBuildingCount[eIndex] = (m_paiFreeBuildingCount[eIndex] + iChange);
		FAssert(getFreeBuildingCount(eIndex) >= 0);

		if (iOldFreeBuildingCount == 0)
		{
			FAssertMsg(getFreeBuildingCount(eIndex) > 0, "getFreeBuildingCount(eIndex) is expected to be greater than 0");

			for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
			{
				pLoopCity->setHasFreeBuilding(eIndex, true);
			}
		}
		else if (getFreeBuildingCount(eIndex) == 0)
		{
			FAssertMsg(iOldFreeBuildingCount > 0, "iOldFreeBuildingCount is expected to be greater than 0");

			for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
			{
				pLoopCity->setHasFreeBuilding(eIndex, false);
			}
		}
	}
}

int CvPlayer::getUnitClassCount(UnitClassTypes eIndex) const 
{
	FAssert(eIndex >= 0 && eIndex < GC.getNumUnitClassInfos());
	return m_paiUnitClassCount[eIndex];
}

void CvPlayer::changeUnitClassCount(UnitClassTypes eIndex, int iChange)
{
	FAssert(eIndex >= 0 && eIndex < GC.getNumUnitClassInfos());
	m_paiUnitClassCount[eIndex] += iChange;
	FAssert(getUnitClassCount(eIndex) >= 0);
}

int CvPlayer::getUnitClassMaking(UnitClassTypes eIndex) const									 
{
	FAssert(eIndex >= 0 && eIndex < GC.getNumUnitClassInfos());
	return m_paiUnitClassMaking[eIndex];
}


void CvPlayer::changeUnitClassMaking(UnitClassTypes eIndex, int iChange)
{
	FAssert(eIndex >= 0 && eIndex < GC.getNumUnitClassInfos());

	if (iChange != 0)
	{
		m_paiUnitClassMaking[eIndex] += iChange;
		FAssert(getUnitClassMaking(eIndex) >= 0);

		if (getID() == GC.getGameINLINE().getActivePlayer())
		{
			gDLL->getInterfaceIFace()->setDirty(Help_DIRTY_BIT, true);
		}
	}
}

int CvPlayer::getUnitClassImmigrated(UnitClassTypes eIndex) const									 
{
	FAssert(eIndex >= 0 && eIndex < GC.getNumUnitClassInfos());
	return m_paiUnitClassImmigrated[eIndex];
}

void CvPlayer::changeUnitClassImmigrated(UnitClassTypes eIndex, int iChange)
{
	FAssert(eIndex >= 0 && eIndex < GC.getNumUnitClassInfos());

	if (iChange != 0)
	{
		m_paiUnitClassImmigrated[eIndex] += iChange;
		FAssert(getUnitClassImmigrated(eIndex) >= 0);
	}
}

int CvPlayer::getUnitClassCountPlusMaking(UnitClassTypes eIndex) const
{
	return (getUnitClassCount(eIndex) + getUnitClassMaking(eIndex));
}

int CvPlayer::getUnitMoveChange(UnitClassTypes eIndex) const
{
	FAssert(eIndex >= 0 && eIndex < GC.getNumUnitClassInfos());
	return m_paiUnitMoveChange[eIndex];
}

void CvPlayer::changeUnitMoveChange(UnitClassTypes eIndex, int iChange)
{
	FAssert(eIndex >= 0 && eIndex < GC.getNumUnitClassInfos());
	if(iChange != 0)
	{
		m_paiUnitMoveChange[eIndex] += iChange;
	}
}

int CvPlayer::getUnitStrengthModifier(UnitClassTypes eIndex) const
{
	FAssert(eIndex >= 0 && eIndex < GC.getNumUnitClassInfos());
	return m_paiUnitStrengthModifier[eIndex];
}

void CvPlayer::changeUnitStrengthModifier(UnitClassTypes eIndex, int iChange)
{
	FAssert(eIndex >= 0 && eIndex < GC.getNumUnitClassInfos());
	if(iChange != 0)
	{
		m_paiUnitStrengthModifier[eIndex] += iChange;
	}
}

int CvPlayer::getProfessionMoveChange(ProfessionTypes eIndex) const
{
	FAssert(eIndex >= 0 && eIndex < GC.getNumProfessionInfos());
	return m_paiProfessionMoveChange[eIndex];
}

void CvPlayer::changeProfessionMoveChange(ProfessionTypes eIndex, int iChange)
{
	FAssert(eIndex >= 0 && eIndex < GC.getNumProfessionInfos());
	if(iChange != 0)
	{
		m_paiProfessionMoveChange[eIndex] += iChange;
	}
}

int CvPlayer::getProfessionCombatChange(ProfessionTypes eIndex) const
{
	FAssert(eIndex >= 0 && eIndex < GC.getNumProfessionInfos());
	return m_paiProfessionCombatChange[eIndex];
}

void CvPlayer::changeProfessionCombatChange(ProfessionTypes eIndex, int iChange)
{
	FAssert(eIndex >= 0 && eIndex < GC.getNumProfessionInfos());
	if(iChange != 0)
	{
		int iLoop;

		for (CvCity* pCity = firstCity(&iLoop); pCity != NULL; pCity = nextCity(&iLoop))
		{
			pCity->AI_setWorkforceHack(true);
		}

		for (CvUnit* pUnit = firstUnit(&iLoop); pUnit != NULL; pUnit = nextUnit(&iLoop))
		{
			pUnit->processProfession(pUnit->getProfession(), -1, false);
		}

		for (uint i = 0; i < m_aEuropeUnits.size(); ++i)
		{
			m_aEuropeUnits[i]->processProfession(m_aEuropeUnits[i]->getProfession(), -1, false);
		}

		for (CvCity* pCity = firstCity(&iLoop); pCity != NULL; pCity = nextCity(&iLoop))
		{
			for (int i = 0; i < pCity->getPopulation(); ++i)
			{
				CvUnit* pUnit = pCity->getPopulationUnitByIndex(i);
				if (NULL != pUnit)
				{
					pUnit->processProfession(pUnit->getProfession(), -1, false);
				}
			}
		}

		m_paiProfessionCombatChange[eIndex] += iChange;

		for (CvUnit* pUnit = firstUnit(&iLoop); pUnit != NULL; pUnit = nextUnit(&iLoop))
		{
			pUnit->processProfession(pUnit->getProfession(), 1, false);
		}

		for (uint i = 0; i < m_aEuropeUnits.size(); ++i)
		{
			m_aEuropeUnits[i]->processProfession(m_aEuropeUnits[i]->getProfession(), 1, false);
		}

		for (CvCity* pCity = firstCity(&iLoop); pCity != NULL; pCity = nextCity(&iLoop))
		{
			for (int i = 0; i < pCity->getPopulation(); ++i)
			{
				CvUnit* pUnit = pCity->getPopulationUnitByIndex(i);
				if (NULL != pUnit)
				{
					pUnit->processProfession(pUnit->getProfession(), 1, false);
				}
			}
		}

		for (CvCity* pCity = firstCity(&iLoop); pCity != NULL; pCity = nextCity(&iLoop))
		{
			pCity->AI_setWorkforceHack(false);
		}
	}
}

int CvPlayer::getBuildingClassCount(BuildingClassTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumBuildingClassInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiBuildingClassCount[eIndex];
}

void CvPlayer::changeBuildingClassCount(BuildingClassTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumBuildingClassInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_paiBuildingClassCount[eIndex] = (m_paiBuildingClassCount[eIndex] + iChange);
	FAssert(getBuildingClassCount(eIndex) >= 0);
}


int CvPlayer::getBuildingClassMaking(BuildingClassTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumBuildingClassInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiBuildingClassMaking[eIndex];
}


void CvPlayer::changeBuildingClassMaking(BuildingClassTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumBuildingClassInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (iChange != 0)
	{
		m_paiBuildingClassMaking[eIndex] = (m_paiBuildingClassMaking[eIndex] + iChange);
		FAssert(getBuildingClassMaking(eIndex) >= 0);

		if (getID() == GC.getGameINLINE().getActivePlayer())
		{
			gDLL->getInterfaceIFace()->setDirty(Help_DIRTY_BIT, true);
		}
	}
}


int CvPlayer::getBuildingClassCountPlusMaking(BuildingClassTypes eIndex) const
{
	return (getBuildingClassCount(eIndex) + getBuildingClassMaking(eIndex));
}


int CvPlayer::getHurryCount(HurryTypes eIndex) const														
{
	FAssert(eIndex >= 0);
	FAssert(eIndex < GC.getNumHurryInfos());
	return m_paiHurryCount[eIndex];
}


bool CvPlayer::canPopRush()
{
	return (m_iPopRushHurryCount > 0);
}

void CvPlayer::changeHurryCount(HurryTypes eIndex, int iChange)										
{
	FAssert(eIndex >= 0);
	FAssert(eIndex < GC.getNumHurryInfos());

	int oldHurryCount = m_paiHurryCount[eIndex];
	m_paiHurryCount[eIndex] += iChange;
	FAssert(getHurryCount(eIndex) >= 0);
	
	// if we just went from 0 to 1 (or the reverse)
	if ((oldHurryCount > 0) != (m_paiHurryCount[eIndex] > 0))
	{
		// does this hurry reduce population?
		if (GC.getHurryInfo(eIndex).getProductionPerPopulation() > 0)
		{
			m_iPopRushHurryCount += iChange;
			FAssert(m_iPopRushHurryCount >= 0);
		}
	}
}
int CvPlayer::getSpecialBuildingNotRequiredCount(SpecialBuildingTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumSpecialBuildingInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiSpecialBuildingNotRequiredCount[eIndex];
}


bool CvPlayer::isSpecialBuildingNotRequired(SpecialBuildingTypes eIndex) const
{
	return (getSpecialBuildingNotRequiredCount(eIndex) > 0);
}

void CvPlayer::changeSpecialBuildingNotRequiredCount(SpecialBuildingTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumSpecialBuildingInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_paiSpecialBuildingNotRequiredCount[eIndex] = (m_paiSpecialBuildingNotRequiredCount[eIndex] + iChange);
	FAssert(getSpecialBuildingNotRequiredCount(eIndex) >= 0);
}


CivicTypes CvPlayer::getCivic(CivicOptionTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumCivicOptionInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paeCivics[eIndex];
}

void CvPlayer::setCivic(CivicOptionTypes eIndex, CivicTypes eNewValue)
{
	CivicTypes eOldCivic = getCivic(eIndex);

	if (eOldCivic != eNewValue)
	{
		m_paeCivics[eIndex] = eNewValue;

		if (eOldCivic != NO_CIVIC)
		{
			processCivics(eOldCivic, -1);
		}

		if (getCivic(eIndex) != NO_CIVIC)
		{
			processCivics(getCivic(eIndex), 1);
		}

		GC.getGameINLINE().AI_makeAssignWorkDirty();

		if (GC.getGameINLINE().isFinalInitialized())
		{
			if (gDLL->isDiplomacy() && (gDLL->getDiplomacyPlayer() == getID()))
			{
				gDLL->updateDiplomacyAttitude(true);
			}

			if (getCivic(eIndex) != NO_CIVIC)
			{
				if (getCivic(eIndex) != GC.getCivilizationInfo(getCivilizationType()).getCivilizationInitialCivics(eIndex))
				{
					for (int iI = 0; iI < MAX_PLAYERS; iI++)
					{
						if (GET_PLAYER((PlayerTypes)iI).isAlive())
						{
							if (GET_TEAM(getTeam()).isHasMet(GET_PLAYER((PlayerTypes)iI).getTeam()))
							{
								CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_PLAYER_ADOPTED_CIVIC", getNameKey(), GC.getCivicInfo(getCivic(eIndex)).getTextKeyWide());
								gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_CIVIC_ADOPT", MESSAGE_TYPE_MAJOR_EVENT);
							}
						}
					}

					CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_PLAYER_ADOPTED_CIVIC", getNameKey(), GC.getCivicInfo(getCivic(eIndex)).getTextKeyWide());
					GC.getGameINLINE().addReplayMessage(REPLAY_MESSAGE_MAJOR_EVENT, getID(), szBuffer);
				}
			}
		}
	}
}


int CvPlayer::getImprovementYieldChange(ImprovementTypes eIndex1, YieldTypes eIndex2) const
{
	FAssertMsg(eIndex1 >= 0, "eIndex1 is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex1 < GC.getNumImprovementInfos(), "eIndex1 is expected to be within maximum bounds (invalid Index)");
	FAssertMsg(eIndex2 >= 0, "eIndex2 is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex2 < NUM_YIELD_TYPES, "eIndex2 is expected to be within maximum bounds (invalid Index)");
	return m_ppiImprovementYieldChange[eIndex1][eIndex2];
}


void CvPlayer::changeImprovementYieldChange(ImprovementTypes eIndex1, YieldTypes eIndex2, int iChange)
{
	FAssertMsg(eIndex1 >= 0, "eIndex1 is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex1 < GC.getNumImprovementInfos(), "eIndex1 is expected to be within maximum bounds (invalid Index)");
	FAssertMsg(eIndex2 >= 0, "eIndex2 is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex2 < NUM_YIELD_TYPES, "eIndex2 is expected to be within maximum bounds (invalid Index)");

	if (iChange != 0)
	{
		m_ppiImprovementYieldChange[eIndex1][eIndex2] += iChange;
		FAssert(getImprovementYieldChange(eIndex1, eIndex2) >= 0);

		updateYield();
	}
}

int CvPlayer::getBuildingYieldChange(BuildingClassTypes eBuildingClass, YieldTypes eYield) const
{
	FAssert(eBuildingClass >= 0);
	FAssert(eBuildingClass < GC.getNumBuildingClassInfos());
	FAssert(eYield >= 0);
	FAssert(eYield < NUM_YIELD_TYPES);
	return m_ppiBuildingYieldChange[eBuildingClass][eYield];
}

void CvPlayer::changeBuildingYieldChange(BuildingClassTypes eBuildingClass, YieldTypes eYield, int iChange)
{
	FAssert(eBuildingClass >= 0);
	FAssert(eBuildingClass < GC.getNumBuildingClassInfos());
	FAssert(eYield >= 0);
	FAssert(eYield < NUM_YIELD_TYPES);

	if (iChange != 0)
	{
		m_ppiBuildingYieldChange[eBuildingClass][eYield] += iChange;
		FAssert(getBuildingYieldChange(eBuildingClass, eYield) >= 0);

		updateYield();
	}
}

int CvPlayer::getTaxYieldModifierCount(YieldTypes eYield) const
{
	FAssert(eYield > -1);
	FAssert(eYield < NUM_YIELD_TYPES);
	return m_aiTaxYieldModifierCount[eYield];
}

void CvPlayer::changeTaxYieldModifierCount(YieldTypes eYield, int iChange) const
{
	FAssert(eYield > -1);
	FAssert(eYield < NUM_YIELD_TYPES);
	m_aiTaxYieldModifierCount[eYield] += iChange;
}


// XXX should pUnit be a CvSelectionGroup???
void CvPlayer::updateGroupCycle(CvUnit* pUnit)
{
	CLLNode<IDInfo>* pUnitNode;
	CLLNode<int>* pSelectionGroupNode;
	CLLNode<int>* pBestSelectionGroupNode;
	CvSelectionGroup* pLoopSelectionGroup;
	CvUnit* pHeadUnit;
	CvUnit* pBeforeUnit;
	CvUnit* pAfterUnit;
	CvUnit* pLoopUnit;
	CvPlot* pPlot;
	int iValue;
	int iBestValue;

	if (!(pUnit->isOnMap()))
	{
		return;
	}

	FAssertMsg(pUnit->getGroup() != NULL, "Unit->getGroup() is not assigned a valid value");

	removeGroupCycle(pUnit->getGroupID());

	pPlot = pUnit->plot();

	pBeforeUnit = NULL;
	pAfterUnit = NULL;

	pUnitNode = pPlot->headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pPlot->nextUnitNode(pUnitNode);

		if (pLoopUnit->isGroupHead())
		{
			if (pLoopUnit != pUnit)
			{
				if (!isBeforeUnitCycle(pLoopUnit, pUnit))
				{
					pBeforeUnit = pLoopUnit;
					break;
				}
				else
				{
					pAfterUnit = pLoopUnit;
				}
			}
		}
	}

	pSelectionGroupNode = headGroupCycleNode();

	iBestValue = MAX_INT;
	pBestSelectionGroupNode = NULL;

	while (pSelectionGroupNode != NULL)
	{
		pLoopSelectionGroup = getSelectionGroup(pSelectionGroupNode->m_data);
		FAssertMsg(pLoopSelectionGroup != NULL, "LoopSelectionGroup is not assigned a valid value");

		pHeadUnit = pLoopSelectionGroup->getHeadUnit();

		if (pHeadUnit != NULL)
		{
			if (pBeforeUnit != NULL)
			{
				if (pBeforeUnit == pHeadUnit)
				{
					pBestSelectionGroupNode = pSelectionGroupNode;
					break;
				}
			}
			else if (pAfterUnit != NULL)
			{
				if (pAfterUnit == pHeadUnit)
				{
					pBestSelectionGroupNode = nextGroupCycleNode(pSelectionGroupNode);
					break;
				}
			}
			else
			{
				iValue = plotDistance(pUnit->getX_INLINE(), pUnit->getY_INLINE(), pHeadUnit->getX_INLINE(), pHeadUnit->getY_INLINE());

				if (iValue < iBestValue)
				{
					iBestValue = iValue;
					pBestSelectionGroupNode = pSelectionGroupNode;
				}
			}
		}

		pSelectionGroupNode = nextGroupCycleNode(pSelectionGroupNode);
	}

	if (pBestSelectionGroupNode != NULL)
	{
		m_groupCycle.insertBefore(pUnit->getGroupID(), pBestSelectionGroupNode);
	}
	else
	{
		m_groupCycle.insertAtEnd(pUnit->getGroupID());
	}
}


void CvPlayer::removeGroupCycle(int iID)
{
	CLLNode<int>* pSelectionGroupNode;

	pSelectionGroupNode = headGroupCycleNode();

	while (pSelectionGroupNode != NULL)
	{
		if (pSelectionGroupNode->m_data == iID)
		{
			pSelectionGroupNode = deleteGroupCycleNode(pSelectionGroupNode);
			break;
		}
		else
		{
			pSelectionGroupNode = nextGroupCycleNode(pSelectionGroupNode);
		}
	}
}

CLLNode<int>* CvPlayer::deleteGroupCycleNode(CLLNode<int>* pNode)
{
	return m_groupCycle.deleteNode(pNode);
}

CLLNode<int>* CvPlayer::nextGroupCycleNode(CLLNode<int>* pNode) const
{
	return m_groupCycle.next(pNode);
}

CLLNode<int>* CvPlayer::previousGroupCycleNode(CLLNode<int>* pNode) const
{
	return m_groupCycle.prev(pNode);
}

CLLNode<int>* CvPlayer::headGroupCycleNode() const
{
	return m_groupCycle.head();
}

CLLNode<int>* CvPlayer::tailGroupCycleNode() const
{
	return m_groupCycle.tail();
}
void CvPlayer::addCityName(const CvWString& szName)
{
	m_aszCityNames.push_back(szName);
}

int CvPlayer::getNumCityNames() const
{
	return m_aszCityNames.size();
}


const CvWString& CvPlayer::getCityName(int iIndex) const
{
	return m_aszCityNames[iIndex];
}

CvCity* CvPlayer::firstCity(int *pIterIdx, bool bRev) const
{
	return !bRev ? m_cities.beginIter(pIterIdx) : m_cities.endIter(pIterIdx);
}

CvCity* CvPlayer::nextCity(int *pIterIdx, bool bRev) const
{
	return !bRev ? m_cities.nextIter(pIterIdx) : m_cities.prevIter(pIterIdx);
}


int CvPlayer::getNumCities() const																			
{
	return m_cities.getCount();
}


CvCity* CvPlayer::getCity(int iID) const
{
	return(m_cities.getAt(iID));
}


CvCity* CvPlayer::addCity()
{
	return(m_cities.add());
}


void CvPlayer::deleteCity(int iID)
{
	m_cities.removeAt(iID);
}


CvUnit* CvPlayer::firstUnit(int *pIterIdx) const
{
	if (m_units.empty())
	{
		return NULL;
	}

	CvIdVector<CvUnitAI>::const_iterator it = m_units.begin();
	CvUnit* pUnit = it->second;

	++it;
	if (it == m_units.end())
	{
		*pIterIdx = -1;
	}
	else
	{
		*pIterIdx = it->first;
	}

	return pUnit;
}


CvUnit* CvPlayer::nextUnit(int *pIterIdx) const
{
	CvIdVector<CvUnitAI>::const_iterator it = m_units.find(*pIterIdx);

	if (it == m_units.end())
	{
		return NULL;
	}

	CvUnit* pUnit = it->second;

	++it;
	if (it == m_units.end())
	{
		*pIterIdx = -1;
	}
	else
	{
		*pIterIdx = it->first;
	}

	return pUnit;
}


int CvPlayer::getNumUnits() const
{
	return (int)(m_units.size());
}


CvUnit* CvPlayer::getUnit(int iID) const
{
    return (m_units.getById(iID));
}


CvUnit* CvPlayer::addUnit()																			
{
	return (m_units.addNew());
}

void CvPlayer::addExistingUnit(CvUnit *pUnit)
{	
	CvUnitAI* pUnitAI = dynamic_cast<CvUnitAI*>(pUnit);
	FAssert(pUnitAI != NULL);
	m_units[pUnitAI->getID()] = pUnitAI;
}

void CvPlayer::deleteUnit(int iID)																	
{
	m_units.removeById(iID);
}

CvUnit* CvPlayer::getAndRemoveUnit(int iId)
{
	CvUnit* pUnit = m_units.getById(iId);
	if(pUnit != NULL)
	{
		pUnit->removeFromMap();
		m_units.erase(pUnit->getID());
	}

	return pUnit;
}

int CvPlayer::getNumEuropeUnits() const
{
	return m_aEuropeUnits.size();
}

CvUnit* CvPlayer::getEuropeUnit(int iIndex) const
{
	FAssert(iIndex >= 0);
	FAssert(iIndex < (int)m_aEuropeUnits.size());
	return m_aEuropeUnits[iIndex];
}

CvUnit* CvPlayer::getEuropeUnitById(int iId) const
{
	for (int i = 0; i < getNumEuropeUnits(); ++i)
	{
		CvUnit* pUnit = getEuropeUnit(i);
		if (pUnit->getID() == iId)
		{
			return pUnit;
		}
	}

	return NULL;
}

void CvPlayer::loadUnitFromEurope(CvUnit* pUnit, CvUnit* pTransport)
{
	FAssert(NULL != pUnit && NULL != pTransport);
	if (NULL != pUnit && NULL != pTransport)
	{
		if (pUnit->canLoadUnit(pTransport, pTransport->plot(), false))
		{
			//transfer to player
			std::vector<CvUnit*>::iterator it = std::remove(m_aEuropeUnits.begin(), m_aEuropeUnits.end(), pUnit);
			if(it == m_aEuropeUnits.end())
			{
				FAssertMsg(false, "Could not find europe unit.");
			}
			else
			{
				m_aEuropeUnits.erase(it);
				addExistingUnit(pUnit);
				FAssert(pUnit->getUnitTravelState() == UNIT_TRAVEL_STATE_IN_EUROPE);
				pUnit->addToMap(pTransport->getX_INLINE(), pTransport->getY_INLINE());
				pUnit->setTransportUnit(pTransport);
				gDLL->getInterfaceIFace()->setDirty(EuropeScreen_DIRTY_BIT, true);
			}
		}
	}
}

void CvPlayer::unloadUnitToEurope(CvUnit* pUnit)
{
	FAssert(pUnit != NULL);
	if (NULL != pUnit)
	{
		FAssertMsg(pUnit->getYield() == NO_YIELD, "Yields are meant to be sold not unloaded");
		FAssertMsg(pUnit->getYieldStored() == 0, "Yields and Treasures are meant to be sold, not unloaded");
		UnitAITypes eUnitAI = pUnit->AI_getUnitAIType();
		CvUnit* pTransferUnit = getAndRemoveUnit(pUnit->getID());
		FAssert(pTransferUnit == pUnit);
		m_aEuropeUnits.push_back(pTransferUnit);
		pTransferUnit->AI_setUnitAIType(eUnitAI);
		pTransferUnit->setUnitTravelState(UNIT_TRAVEL_STATE_IN_EUROPE, false);
		gDLL->getInterfaceIFace()->setDirty(EuropeScreen_DIRTY_BIT, true);
	}
}

void CvPlayer::transferUnitInEurope(CvUnit* pUnit, CvUnit* pTransport)
{
	FAssert(NULL != pUnit && NULL != pTransport);
	if (NULL != pUnit && NULL != pTransport)
	{
		if (pUnit->canLoadUnit(pTransport, pTransport->plot(), false))
		{
			pUnit->setTransportUnit(pTransport);
			gDLL->getInterfaceIFace()->setDirty(EuropeScreen_DIRTY_BIT, true);	
		}
	}
}

int CvPlayer::countNumTravelUnits(UnitTravelStates eState, DomainTypes eDomain) const
{
	int iCount = 0;
	int iLoop;
	for (CvUnit* pLoopUnit = firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = nextUnit(&iLoop))
	{
		if (eDomain == NO_DOMAIN || pLoopUnit->getDomainType() == eDomain)
		{
			if (pLoopUnit->getUnitTravelState() == eState)
			{
				++iCount;
			}
		}
	}

	return iCount;
}

int CvPlayer::countNumDomainUnits(DomainTypes eDomain) const
{
	int iCount = 0;
	int iLoop;
	for (CvUnit* pLoopUnit = firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = nextUnit(&iLoop))
	{
		if (eDomain == NO_DOMAIN || pLoopUnit->getDomainType() == eDomain)
		{
			++iCount;
		}
	}
	for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		for (int i = 0; i < pLoopCity->getPopulation(); ++i)
		{
			if (eDomain == NO_DOMAIN || pLoopCity->getPopulationUnitByIndex(i)->getDomainType() == eDomain)
			{
				++iCount;
			}
		}
	}
	for (uint i = 0; i < m_aEuropeUnits.size(); ++i)
	{
		if (eDomain == NO_DOMAIN || m_aEuropeUnits[i]->getDomainType() == eDomain)
		{
			++iCount;
		}
	}

	return iCount;
}

CvSelectionGroup* CvPlayer::firstSelectionGroup(int *pIterIdx, bool bRev) const
{
	return !bRev ? m_selectionGroups.beginIter(pIterIdx) : m_selectionGroups.endIter(pIterIdx);
}


CvSelectionGroup* CvPlayer::nextSelectionGroup(int *pIterIdx, bool bRev) const
{
	return !bRev ? m_selectionGroups.nextIter(pIterIdx) : m_selectionGroups.prevIter(pIterIdx);
}


int CvPlayer::getNumSelectionGroups() const																
{
	return m_selectionGroups.getCount();
}


CvSelectionGroup* CvPlayer::getSelectionGroup(int iID) const												
{
	return ((CvSelectionGroup *)(m_selectionGroups.getAt(iID)));
}


CvSelectionGroup* CvPlayer::addSelectionGroup()
{
	return ((CvSelectionGroup *)(m_selectionGroups.add()));
}


void CvPlayer::deleteSelectionGroup(int iID)
{
	bool bRemoved = m_selectionGroups.removeAt(iID);
	
	FAssertMsg(bRemoved, "could not find group, delete failed");
}

EventTriggeredData* CvPlayer::firstEventTriggered(int *pIterIdx, bool bRev) const
{
	return !bRev ? m_eventsTriggered.beginIter(pIterIdx) : m_eventsTriggered.endIter(pIterIdx);
}

EventTriggeredData* CvPlayer::nextEventTriggered(int *pIterIdx, bool bRev) const
{
	return !bRev ? m_eventsTriggered.nextIter(pIterIdx) : m_eventsTriggered.prevIter(pIterIdx);
}

int CvPlayer::getNumEventsTriggered() const
{
	return m_eventsTriggered.getCount();
}

EventTriggeredData* CvPlayer::getEventTriggered(int iID) const
{
	return ((EventTriggeredData*)(m_eventsTriggered.getAt(iID)));
}

EventTriggeredData* CvPlayer::addEventTriggered()
{
	return ((EventTriggeredData*)(m_eventsTriggered.add()));
}

void CvPlayer::deleteEventTriggered(int iID)
{
	m_eventsTriggered.removeAt(iID);
}


void CvPlayer::addMessage(const CvTalkingHeadMessage& message)
{
	m_listGameMessages.push_back(message);
}


void CvPlayer::clearMessages()
{
	m_listGameMessages.clear();
}


const CvMessageQueue& CvPlayer::getGameMessages() const
{
	return (m_listGameMessages);
}


void CvPlayer::expireMessages()
{
	CvMessageQueue::iterator it = m_listGameMessages.begin();
	bool bFoundExpired = false;
	while(it != m_listGameMessages.end())
	{
		CvTalkingHeadMessage& message = *it;
		if (GC.getGameINLINE().getGameTurn() >= message.getExpireTurn())
		{
			it = m_listGameMessages.erase(it);
			bFoundExpired = true;
		}
		else
		{
			++it;
		}
	}
	if (bFoundExpired)
	{
		gDLL->getInterfaceIFace()->dirtyTurnLog(getID());
	}
}


void CvPlayer::addPopup(CvPopupInfo* pInfo, bool bFront)
{
	if (isHuman())
	{
		if (bFront)
		{
			m_listPopups.push_front(pInfo);
		}
		else
		{
			m_listPopups.push_back(pInfo);
		}
	}
	else
	{
		FAssertMsg(false, "Adding popup to AI");
		SAFE_DELETE(pInfo);
	}
}


void CvPlayer::clearPopups()
{
	CvPopupQueue::iterator it;
	for (it = m_listPopups.begin(); it != m_listPopups.end(); ++it)
	{
		CvPopupInfo* pInfo = *it;
		if (NULL != pInfo)
		{
			delete pInfo;
		}
	}
	m_listPopups.clear();
}


CvPopupInfo* CvPlayer::popFrontPopup()
{
	CvPopupInfo* pInfo = NULL;
	if (!m_listPopups.empty())
	{
		pInfo = m_listPopups.front();
		m_listPopups.pop_front();
	}
	return pInfo;
}


const CvPopupQueue& CvPlayer::getPopups() const
{
	return (m_listPopups);
}


void CvPlayer::addDiplomacy(CvDiploParameters* pDiplo)
{
	if (NULL != pDiplo)
	{
		m_listDiplomacy.push_back(pDiplo);
	}
}


void CvPlayer::clearDiplomacy()
{
	CvDiploQueue::iterator it;
	for (it = m_listDiplomacy.begin(); it != m_listDiplomacy.end(); ++it)
	{
		CvDiploParameters* pDiplo = *it;
		if (NULL != pDiplo)
		{
			delete pDiplo;
		}
	}
	m_listDiplomacy.clear();
}


const CvDiploQueue& CvPlayer::getDiplomacy() const
{
	return (m_listDiplomacy);
}


CvDiploParameters* CvPlayer::popFrontDiplomacy()
{
	CvDiploParameters* pDiplo = NULL;
	if (!m_listDiplomacy.empty())
	{
		pDiplo = m_listDiplomacy.front();
		m_listDiplomacy.pop_front();
	}
	return pDiplo;
}


int CvPlayer::getScoreHistory(int iTurn) const
{
	CvTurnScoreMap::const_iterator it = m_mapScoreHistory.find(iTurn);
	if (it != m_mapScoreHistory.end())
	{
		return (*it).second;
	}
	return 0;
}

void CvPlayer::updateScoreHistory(int iTurn, int iBestScore)
{
	m_mapScoreHistory[iTurn] = iBestScore;
}

int CvPlayer::getEconomyHistory(int iTurn) const
{
	CvTurnScoreMap::const_iterator it = m_mapEconomyHistory.find(iTurn);
	if (it != m_mapEconomyHistory.end())
	{
		return (*it).second;
	}
	return 0;
}

void CvPlayer::updateEconomyHistory(int iTurn, int iBestEconomy)
{
	m_mapEconomyHistory[iTurn] = iBestEconomy;
}

int CvPlayer::getIndustryHistory(int iTurn) const
{
	CvTurnScoreMap::const_iterator it = m_mapIndustryHistory.find(iTurn);
	if (it != m_mapIndustryHistory.end())
	{
		return (*it).second;
	}
	return 0;
}

void CvPlayer::updateIndustryHistory(int iTurn, int iBestIndustry)
{
	m_mapIndustryHistory[iTurn] = iBestIndustry;
}

int CvPlayer::getAgricultureHistory(int iTurn) const
{
	CvTurnScoreMap::const_iterator it = m_mapAgricultureHistory.find(iTurn);
	if (it != m_mapAgricultureHistory.end())
	{
		return (*it).second;
	}
	return 0;
}

void CvPlayer::updateAgricultureHistory(int iTurn, int iBestAgriculture)
{
	m_mapAgricultureHistory[iTurn] = iBestAgriculture;
}

int CvPlayer::getPowerHistory(int iTurn) const
{
	CvTurnScoreMap::const_iterator it = m_mapPowerHistory.find(iTurn);
	if (it != m_mapPowerHistory.end())
	{
		return (*it).second;
	}
	return 0;
}

void CvPlayer::updatePowerHistory(int iTurn, int iBestPower)
{
	m_mapPowerHistory[iTurn] = iBestPower;
}

int CvPlayer::getCultureHistory(int iTurn) const
{
	CvTurnScoreMap::const_iterator it = m_mapCultureHistory.find(iTurn);
	if (it != m_mapCultureHistory.end())
	{
		return (*it).second;
	}
	return 0;
}

int CvPlayer::addTradeRoute(const IDInfo& kSource, const IDInfo& kDestination, YieldTypes eYield)
{
	FAssert(eYield != NO_YIELD);

	CvCity* pSourceCity = ::getCity(kSource);
	FAssert(pSourceCity != NULL);
	if (pSourceCity == NULL)
	{
		return -1;
	}
	FAssert(pSourceCity->getTeam() == getTeam());

	CvCity* pDestinationCity = ::getCity(kDestination);
	FAssert(pDestinationCity != NULL || (kDestination.eOwner == getID() && kDestination.iID == CvTradeRoute::EUROPE_CITY_ID));
	FAssert(pDestinationCity == NULL || pDestinationCity->getTeam() == getTeam());

	if (kSource == kDestination)
	{
		return -1;
	}

	for (CvIdVector<CvTradeRoute>::iterator it = m_tradeRoutes.begin(); it != m_tradeRoutes.end(); ++it)
	{
		CvTradeRoute* pTradeRoute = it->second;
		if (pTradeRoute->getSourceCity() == kSource
			&& pTradeRoute->getDestinationCity() == kDestination
			&& pTradeRoute->getYield() == eYield)
		{
			return pTradeRoute->getID();
		}
	}

	CvTradeRoute* pTradeRoute = m_tradeRoutes.addNew();
	pTradeRoute->init(kSource, kDestination, eYield);
	pSourceCity->addExport(eYield, false);
	if (pDestinationCity != NULL)
	{
		pDestinationCity->addImport(eYield, false);
	}
	if (getID() == GC.getGameINLINE().getActivePlayer())
	{
		gDLL->getInterfaceIFace()->setDirty(Domestic_Advisor_DIRTY_BIT, true);
	}
	return pTradeRoute->getID();
}


bool CvPlayer::editTradeRoute(int iId, const IDInfo& kSource, const IDInfo& kDestination, YieldTypes eYield)
{
	CvTradeRoute* pTradeRoute = getTradeRoute(iId);
	if (pTradeRoute == NULL)
	{
		return false;
	}

	for (CvIdVector<CvTradeRoute>::iterator it = m_tradeRoutes.begin(); it != m_tradeRoutes.end(); ++it)
	{
		CvTradeRoute* pLoopTradeRoute = it->second;
		if (pLoopTradeRoute->getSourceCity() == kSource
			&& pLoopTradeRoute->getDestinationCity() == kDestination 
			&& pLoopTradeRoute->getYield() == eYield)
		{
			return false;
		}		
	}

	pTradeRoute->setSourceCity(kSource);
	pTradeRoute->setDestinationCity(kDestination);
	pTradeRoute->setYield(eYield);

	return true;
}


bool CvPlayer::removeTradeRoute(int iId)
{
	CvTradeRoute* pTradeRoute = getTradeRoute(iId);
	FAssert(pTradeRoute != NULL);
	if (pTradeRoute == NULL)
	{
		return false;
	}

	int iLoop;
	for(CvSelectionGroup* pLoopSelectionGroup = firstSelectionGroup(&iLoop); pLoopSelectionGroup; pLoopSelectionGroup = nextSelectionGroup(&iLoop))
	{
		pLoopSelectionGroup->assignTradeRoute(iId, false);
	}

	CvCity* pSourceCity = ::getCity(pTradeRoute->getSourceCity());
	CvCity* pDestinationCity = ::getCity(pTradeRoute->getDestinationCity());
	if(!m_tradeRoutes.removeById(iId))
	{
		return false;
	}

	if (pSourceCity != NULL)
	{
		pSourceCity->updateExports();
	}

	if (pDestinationCity != NULL)
	{
		pDestinationCity->updateImports();
	}

	if (getID() == GC.getGameINLINE().getActivePlayer())
	{
		gDLL->getInterfaceIFace()->setDirty(Domestic_Advisor_DIRTY_BIT, true);
	}

	return true;
}

CvTradeRoute* CvPlayer::getTradeRoute(int iId) const
{
	CvTradeRoute* pTradeRoute = m_tradeRoutes.getById(iId);
	FAssert(pTradeRoute == NULL || pTradeRoute->getDestinationCity().eOwner == getID() || pTradeRoute->getSourceCity().eOwner == getID());
	return pTradeRoute;
}

int CvPlayer::getNumTradeRoutes() const
{
	return m_tradeRoutes.size();
}

void CvPlayer::validateTradeRoutes()
{
	std::vector<CvTradeRoute*> aTradeRoutes;
	getTradeRoutes(aTradeRoutes);
	for (uint i = 0; i < aTradeRoutes.size(); ++i)
	{
		CvTradeRoute* pTradeRoute = aTradeRoutes[i];
		if (!pTradeRoute->checkValid(getID()))
		{
			removeTradeRoute(pTradeRoute->getID());
		}
	}

	//re-add missing europe destination routes
	for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
	{
		CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes) iPlayer);
		if (canLoadYield((PlayerTypes) iPlayer))
		{
			for (int iYield = 0; iYield < NUM_YIELD_TYPES; iYield++)
			{
				YieldTypes eYield = (YieldTypes) iYield;
				if (isYieldEuropeTradable(eYield))
				{
					int iLoop;
					for (CvCity* pLoopCity = kLoopPlayer.firstCity(&iLoop); NULL != pLoopCity; pLoopCity = kLoopPlayer.nextCity(&iLoop))
					{
						if (pLoopCity->isExport(eYield))
						{
							addTradeRoute(pLoopCity->getIDInfo(), IDInfo(getID(), CvTradeRoute::EUROPE_CITY_ID), eYield);
						}
					}
				}
			}
		}
	}
}

// slow
CvTradeRoute* CvPlayer::getTradeRouteByIndex(int iIndex) const
{
	FAssert(iIndex >= 0 && iIndex < getNumTradeRoutes());
	CvIdVector<CvTradeRoute>::const_iterator it;
	int i = 0;
	for (it = m_tradeRoutes.begin(); it != m_tradeRoutes.end(); ++it)
	{
		if (i++ == iIndex)
		{
			return it->second;
		}
	}

	return NULL;
}

void CvPlayer::getTradeRoutes(std::vector<CvTradeRoute*>& aTradeRoutes) const
{
	CvIdVector<CvTradeRoute>::const_iterator it;
	for (it = m_tradeRoutes.begin(); it != m_tradeRoutes.end(); ++it)
	{
		aTradeRoutes.push_back(it->second);
	}
}

bool CvPlayer::canLoadYield(PlayerTypes eCityPlayer) const
{
	if (eCityPlayer != getID())
	{
		return false;
	}

	return true;
}

bool CvPlayer::canUnloadYield(PlayerTypes eCityPlayer) const
{
	if (GET_PLAYER(eCityPlayer).getTeam() != getTeam())
	{
		return false;
	}

	return true;
}

void CvPlayer::updateCultureHistory(int iTurn, int iBestCulture)
{
	m_mapCultureHistory[iTurn] = iBestCulture;
}

std::string CvPlayer::getScriptData() const
{
	return m_szScriptData;
}

void CvPlayer::setScriptData(std::string szNewValue)
{
	m_szScriptData = szNewValue;
}

const CvString& CvPlayer::getPbemEmailAddress() const
{
	return GC.getInitCore().getEmail(getID());
}

void CvPlayer::setPbemEmailAddress(const char* szAddress)
{
	GC.getInitCore().setEmail(getID(), szAddress);
}

const CvString& CvPlayer::getSmtpHost() const
{
	return GC.getInitCore().getSmtpHost(getID());
}

void CvPlayer::setSmtpHost(const char* szHost)
{
	GC.getInitCore().setSmtpHost(getID(), szHost);
}

// Protected Functions...

void CvPlayer::doGold()
{
	CyArgsList argsList;
	argsList.add(getID());
	long lResult=0;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "doGold", argsList.makeFunctionArgs(), &lResult);
	if (lResult == 1)
	{
		return;
	}

	int iGoldChange = 0;

	FAssert(isHuman() || ((getGold() + iGoldChange) >= 0));

	changeGold(iGoldChange);
}

void CvPlayer::doBells()
{
	if (getParent() == NO_PLAYER)
	{
		return;
	}

	int iBellsRate = getYieldRate(YIELD_BELLS);
	if (iBellsRate == 0)
	{
		return;
	}
	//add bells to political points
	for (int i = 0; i < GC.getNumFatherPointInfos(); ++i)
	{
		FatherPointTypes ePointType = (FatherPointTypes) i;
		changeFatherPoints(ePointType, iBellsRate * GC.getFatherPointInfo(ePointType).getYieldPoints(YIELD_BELLS));
	}

	//update revolution unit bells
	if (!GC.getEraInfo(getCurrentEra()).isRevolution())
	{
		changeBellsStored(iBellsRate);
		if (getBellsStored() >= revolutionEuropeUnitThreshold() && iBellsRate > GC.getCivilizationInfo(getCivilizationType()).getFreeYields(YIELD_BELLS))
		{
			changeBellsStored(-revolutionEuropeUnitThreshold());
			setRevolutionEuropeUnitThresholdMultiplier((getRevolutionEuropeUnitThresholdMultiplier() * (100 + GC.getDefineINT("REVOLUTION_EUROPE_UNIT_THRESHOLD_INCREASE"))) / 100);
			setKingNumUnitMultiplier(getKingNumUnitMultiplier() * GC.getHandicapInfo(getHandicapType()).getKingNumUnitMultiplier() / 100);

			if (NO_PLAYER != getParent())
			{
				CvPlayer& kParent = GET_PLAYER(getParent());
				FAssert(kParent.isEurope());

				CvCivilizationInfo& kCivilizationInfo = GC.getCivilizationInfo(kParent.getCivilizationType());
				int iNumFreeUnits = kCivilizationInfo.getNumCivilizationFreeUnits();
				std::vector<int> aiUnitWeights(iNumFreeUnits, 100);
				for (int i = 0; i < iNumFreeUnits; ++i)
				{
					int iUnitClass = kCivilizationInfo.getCivilizationFreeUnitsClass(i);
					UnitTypes eUnit = (UnitTypes) kCivilizationInfo.getCivilizationUnits(iUnitClass);
					if (eUnit == NO_UNIT)
					{
						aiUnitWeights[i] = 0;
					}
					else
					{
						if (GC.getUnitInfo(eUnit).getDomainType() == DOMAIN_SEA)
						{
							aiUnitWeights[i] += std::max(-100, GC.getDefineINT("REVOLUTION_EUROPE_UNIT_SHIP_MODIFIER"));
						}
					}					
				}

				if (iNumFreeUnits > 0)
				{
					int iNumUnits = std::max(1, GC.getDefineINT("KING_INITIAL_UNIT_INCREASE") * getKingNumUnitMultiplier() / 100);
					for (int i = 0; i < iNumUnits; ++i)
					{
						int iIndex = GC.getGameINLINE().getSorenRand().pickValue(aiUnitWeights, "Pick Expeditionary force unit");
						int iUnitClass = kCivilizationInfo.getCivilizationFreeUnitsClass(iIndex);
						ProfessionTypes eUnitProfession = (ProfessionTypes) kCivilizationInfo.getCivilizationFreeUnitsProfession(iIndex);
						UnitTypes eUnit = (UnitTypes)kCivilizationInfo.getCivilizationUnits(iUnitClass);
						FAssert(eUnit != NO_UNIT);
						addRevolutionEuropeUnit(eUnit, eUnitProfession);
					}

					CvWString szBuffer = gDLL->getText("TXT_KEY_NEW_EUROPE_ARMY_NEW", kParent.getCivilizationShortDescriptionKey(), getCivilizationShortDescriptionKey(), iNumUnits, kParent.getCivilizationAdjectiveKey());
					gDLL->getInterfaceIFace()->addMessage(getID(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_UNIT_GREATPEOPLE", GC.getGameINLINE().isDebugMode() ? MESSAGE_TYPE_MAJOR_EVENT : MESSAGE_TYPE_INFO, GC.getLeaderHeadInfo(kParent.getLeaderType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_UNIT_TEXT"));
				}
			}
		}
	}
}

void CvPlayer::doCrosses()
{
	if (getParent() == NO_PLAYER)
	{
		return;
	}

	int iCrossRate = getYieldRate(YIELD_CROSSES);

	//add crosses to political points
	for (int i = 0; i < GC.getNumFatherPointInfos(); ++i)
	{
		FatherPointTypes ePointType = (FatherPointTypes) i;
		changeFatherPoints(ePointType, iCrossRate * GC.getFatherPointInfo(ePointType).getYieldPoints(YIELD_CROSSES));
	}

	if (getImmigrationConversion() == YIELD_CROSSES)
	{
		changeCrossesStored(iCrossRate);
		if (getCrossesStored() >= immigrationThreshold())
		{
			doImmigrant(GC.getGameINLINE().getSorenRandNum(GC.getDefineINT("DOCKS_NEXT_UNITS"), "pick immigrant"));
		}
	}
}

void CvPlayer::doAdvancedStartAction(AdvancedStartActionTypes eAction, int iX, int iY, int iData, bool bAdd)
{
	if (getAdvancedStartPoints() < 0)
	{
		return;
	}

	CvPlot* pPlot = GC.getMap().plot(iX, iY);

	if (0 == getNumCities())
	{
		switch (eAction)
		{
		case ADVANCEDSTARTACTION_EXIT:
			//Try to build this player's empire
			if (getID() == GC.getGameINLINE().getActivePlayer())
			{
				gDLL->getInterfaceIFace()->setBusy(true);
			}
			AI_doAdvancedStart(true);
			if (getID() == GC.getGameINLINE().getActivePlayer())
			{
				gDLL->getInterfaceIFace()->setBusy(false);
			}
			break;
		case ADVANCEDSTARTACTION_AUTOMATE:
		case ADVANCEDSTARTACTION_CITY:
			break;
		default:
			// The first action must be to place a city
			// so players can lose by spending everything
			return;
		}
	}

	switch (eAction)
	{
	case ADVANCEDSTARTACTION_EXIT:
		changeGold(getAdvancedStartPoints());
		setAdvancedStartPoints(-1);
		if (GC.getGameINLINE().getActivePlayer() == getID())
		{
			gDLL->getInterfaceIFace()->setInAdvancedStart(false);
		}

		if (isHuman())
		{
			int iLoop;
			for (CvCity* pCity = firstCity(&iLoop); NULL != pCity; pCity = nextCity(&iLoop))
			{
				pCity->chooseProduction();
			}
		}
		break;
	case ADVANCEDSTARTACTION_AUTOMATE:
		if (getID() == GC.getGameINLINE().getActivePlayer())
		{
			gDLL->getInterfaceIFace()->setBusy(true);
		}
		AI_doAdvancedStart(true);			
		if (getID() == GC.getGameINLINE().getActivePlayer())
		{
			gDLL->getInterfaceIFace()->setBusy(false);
		}
		break;
	case ADVANCEDSTARTACTION_UNIT:
		{
			if(pPlot == NULL)
				return;

			UnitTypes eUnit = (UnitTypes) iData;
			int iCost = getAdvancedStartUnitCost(eUnit, bAdd, pPlot);

			if (bAdd && iCost < 0)
			{
				return;
			}

			// Add unit to the map
			if (bAdd)
			{
				if (getAdvancedStartPoints() >= iCost)
				{
					CvUnit* pUnit = initUnit(eUnit, (ProfessionTypes) GC.getUnitInfo(eUnit).getDefaultProfession(), iX, iY);
					if (NULL != pUnit)
					{
						pUnit->finishMoves();
						changeAdvancedStartPoints(-iCost);
					}
				}
			}

			// Remove unit from the map
			else
			{
				// If cost is -1 we already know this unit isn't present
				if (iCost != -1)
				{
					CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();
					while (pUnitNode != NULL)
					{
						CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
						pUnitNode = pPlot->nextUnitNode(pUnitNode);

						if (pLoopUnit->getUnitType() == eUnit)
						{
							pLoopUnit->kill(false);
							changeAdvancedStartPoints(iCost);
							return;
						}
					}
				}

				// Proper unit not found above, delete first found
				CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();
				if (pUnitNode != NULL)
				{
					CvUnit* pUnit = ::getUnit(pUnitNode->m_data);

					iCost = getAdvancedStartUnitCost(pUnit->getUnitType(), false);
					FAssertMsg(iCost != -1, "If this is -1 then that means it's going to try to delete a unit which shouldn't exist");
					pUnit->kill(false);
					changeAdvancedStartPoints(iCost);
				}
			}

			if (getID() == GC.getGameINLINE().getActivePlayer())
			{
				gDLL->getInterfaceIFace()->setDirty(Advanced_Start_DIRTY_BIT, true);
			}
		}
		break;
	case ADVANCEDSTARTACTION_CITY:
		{
			if(pPlot == NULL)
				return;

			int iCost = getAdvancedStartCityCost(bAdd, pPlot);

			if (iCost < 0)
			{
				return;
			}

			// Add City to the map
			if (bAdd)
			{
				if (0 == getNumCities())
				{
					PlayerTypes eClosestPlayer = NO_PLAYER;
					int iMinDistance = MAX_INT;
					for (int iPlayer = 0; iPlayer < MAX_PLAYERS; iPlayer++)
					{
						CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)iPlayer);
						if (kPlayer.isAlive())
						{
							if (kPlayer.getTeam() == getTeam())
							{
								if (0 == kPlayer.getNumCities())
								{
									FAssert(kPlayer.getStartingPlot() != NULL);
									int iDistance = plotDistance(iX, iY, kPlayer.getStartingPlot()->getX_INLINE(), kPlayer.getStartingPlot()->getY_INLINE());
									if (iDistance < iMinDistance)
									{
										eClosestPlayer = kPlayer.getID();
										iMinDistance = iDistance;
									}
								}
							}
						}
					}
					FAssertMsg(eClosestPlayer != NO_PLAYER, "Self at a minimum should always be valid");
					if (eClosestPlayer != getID())
					{
						CvPlot* pTempPlot = GET_PLAYER(eClosestPlayer).getStartingPlot();
						GET_PLAYER(eClosestPlayer).setStartingPlot(getStartingPlot(), false);
						setStartingPlot(pTempPlot, false);					
					}
				}
				if (getAdvancedStartPoints() >= iCost || 0 == getNumCities())
				{
					found(iX, iY);
					changeAdvancedStartPoints(-iCost);
					GC.getGameINLINE().updateColoredPlots();
					CvCity* pCity = pPlot->getPlotCity();
					if (pCity != NULL)
					{
						if(pCity->getPopulation() == 0)
						{
							pCity->setPopulation(1);
						}

						if (pCity->getPopulation() > 1)
						{
							pCity->setFood(pCity->growthThreshold() / 2);
						}
					}
				}
			}

			// Remove City from the map
			else
			{
				pPlot->setRouteType(NO_ROUTE);
				pPlot->getPlotCity()->kill();
				pPlot->setImprovementType(NO_IMPROVEMENT);
				changeAdvancedStartPoints(iCost);
			}

			if (getID() == GC.getGameINLINE().getActivePlayer())
			{
				gDLL->getInterfaceIFace()->setDirty(Advanced_Start_DIRTY_BIT, true);
			}
		}
		break;
	case ADVANCEDSTARTACTION_POP:
		{
			if(pPlot == NULL)
				return;

			CvCity* pCity = pPlot->getPlotCity();

			if (pCity != NULL)
			{
				int iCost = getAdvancedStartPopCost(bAdd, pCity);
				
				if (iCost < 0)
				{
					return;
				}

				// Add Pop to the City
				if (bAdd)
				{
					if (getAdvancedStartPoints() >= iCost)
					{
						pCity->changePopulation(1);
						changeAdvancedStartPoints(-iCost);
						if (pCity->getPopulation() > 1)
						{
							pCity->setFood(pCity->growthThreshold() / 2);
							pCity->setFoodKept((pCity->getFood() * pCity->getMaxFoodKeptPercent()) / 100);
						}
					}
				}

				// Remove Pop from the city
				else
				{
					pCity->changePopulation(-1);
					changeAdvancedStartPoints(iCost);
					if (pCity->getPopulation() == 1)
					{
						pCity->setFood(0);
						pCity->setFoodKept(0);
					}
				}
			}
		}
		break;
	case ADVANCEDSTARTACTION_CULTURE:
		{
			if(pPlot == NULL)
				return;

			CvCity* pCity = pPlot->getPlotCity();

			if (pCity != NULL)
			{
				int iCost = getAdvancedStartCultureCost(bAdd, pCity);

				if (iCost < 0)
				{
					return;
				}

				// Add Culture to the City
				if (bAdd)
				{
					if (getAdvancedStartPoints() >= iCost)
					{
						pCity->setCulture(getID(), pCity->getCultureThreshold(), true);
						changeAdvancedStartPoints(-iCost);
					}
				}

				// Remove Culture from the city
				else
				{
					CultureLevelTypes eLevel = (CultureLevelTypes)std::max(0, pCity->getCultureLevel() - 1);
					pCity->setCulture(getID(), CvCity::getCultureThreshold(eLevel), true);
					changeAdvancedStartPoints(iCost);
				}
			}
		}
		break;
	case ADVANCEDSTARTACTION_BUILDING:
		{
			if(pPlot == NULL)
				return;

			CvCity* pCity = pPlot->getPlotCity();

			if (pCity != NULL)
			{
				BuildingTypes eBuilding = (BuildingTypes) iData;
				int iCost = getAdvancedStartBuildingCost(eBuilding, bAdd, pCity);

				if (iCost < 0)
				{
					return;
				}

				// Add Building to the City
				if (bAdd)
				{
					if (getAdvancedStartPoints() >= iCost)
					{
						pCity->setHasRealBuilding(eBuilding, true);
						changeAdvancedStartPoints(-iCost);
						if (GC.getBuildingInfo(eBuilding).getFoodKept() != 0)
						{
							pCity->setFoodKept((pCity->getFood() * pCity->getMaxFoodKeptPercent()) / 100);
						}
					}
				}

				// Remove Building from the map
				else
				{
					pCity->setHasRealBuilding(eBuilding, false);
					changeAdvancedStartPoints(iCost);
					if (GC.getBuildingInfo(eBuilding).getFoodKept() != 0)
					{
						pCity->setFoodKept((pCity->getFood() * pCity->getMaxFoodKeptPercent()) / 100);
					}
				}
			}

			if (getID() == GC.getGameINLINE().getActivePlayer())
			{
				gDLL->getInterfaceIFace()->setDirty(Advanced_Start_DIRTY_BIT, true);
			}
		}
		break;
	case ADVANCEDSTARTACTION_ROUTE:
		{
			if(pPlot == NULL)
				return;

			RouteTypes eRoute = (RouteTypes) iData;
			int iCost = getAdvancedStartRouteCost(eRoute, bAdd, pPlot);

			if (bAdd && iCost < 0)
			{
				return;
			}

			// Add Route to the plot
			if (bAdd)
			{
				if (getAdvancedStartPoints() >= iCost)
				{
					pPlot->setRouteType(eRoute);
					changeAdvancedStartPoints(-iCost);
				}
			}

			// Remove Route from the Plot
			else
			{
				if (pPlot->getRouteType() != eRoute)
				{
					eRoute = pPlot->getRouteType();
					iCost = getAdvancedStartRouteCost(eRoute, bAdd);
				}
				
				if (iCost < 0)
				{
					return;
				}

				pPlot->setRouteType(NO_ROUTE);
				changeAdvancedStartPoints(iCost);
			}

			if (getID() == GC.getGameINLINE().getActivePlayer())
			{
				gDLL->getInterfaceIFace()->setDirty(Advanced_Start_DIRTY_BIT, true);
			}
		}
		break;
	case ADVANCEDSTARTACTION_IMPROVEMENT:
		{
			if(pPlot == NULL)
				return;

			ImprovementTypes eImprovement = (ImprovementTypes) iData;
			int iCost = getAdvancedStartImprovementCost(eImprovement, bAdd, pPlot);

			if (bAdd && iCost < 0)
			{
				return;
			}

			// Add Improvement to the plot
			if (bAdd)
			{
				if (getAdvancedStartPoints() >= iCost)
				{
					if (pPlot->getFeatureType() != NO_FEATURE)
					{
						for (int iI = 0; iI < GC.getNumBuildInfos(); ++iI)
						{
							ImprovementTypes eLoopImprovement = ((ImprovementTypes)(GC.getBuildInfo((BuildTypes)iI).getImprovement()));

							if (eImprovement == eLoopImprovement)
							{
								if (GC.getBuildInfo((BuildTypes)iI).isFeatureRemove(pPlot->getFeatureType()) && canBuild(pPlot, (BuildTypes)iI))
								{
									pPlot->setFeatureType(NO_FEATURE);
									break;
								}
							}
						}
					}
					pPlot->setImprovementType(eImprovement);
					changeAdvancedStartPoints(-iCost);
				}
			}
			// Remove Improvement from the Plot
			else
			{
				if (pPlot->getImprovementType() != eImprovement)
				{
					eImprovement = pPlot->getImprovementType();
					iCost = getAdvancedStartImprovementCost(eImprovement, bAdd, pPlot);
				}
				if (iCost < 0)
				{
					return;
				}
				pPlot->setImprovementType(NO_IMPROVEMENT);
				changeAdvancedStartPoints(iCost);
			}

			if (getID() == GC.getGameINLINE().getActivePlayer())
			{
				gDLL->getInterfaceIFace()->setDirty(Advanced_Start_DIRTY_BIT, true);
			}
		}
		break;
	case ADVANCEDSTARTACTION_VISIBILITY:
		{
			if(pPlot == NULL)
				return;

			int iCost = getAdvancedStartVisibilityCost(bAdd, pPlot);

			if (iCost < 0)
			{
				return;
			}

			// Add Visibility to the plot
			if (bAdd)
			{
				if (getAdvancedStartPoints() >= iCost)
				{
					pPlot->setRevealed(getTeam(), true, true, NO_TEAM);
					changeAdvancedStartPoints(-iCost);
				}
			}

			// Remove Visibility from the Plot
			else
			{
				pPlot->setRevealed(getTeam(), false, true, NO_TEAM);
				changeAdvancedStartPoints(iCost);
			}
		}
		break;
	default:
		FAssert(false);
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Adding or removing a unit
/////////////////////////////////////////////////////////////////////////////////////////////

int CvPlayer::getAdvancedStartUnitCost(UnitTypes eUnit, bool bAdd, CvPlot* pPlot)
{
	int iLoop;
	int iNumUnitType = 0;

	if (0 == getNumCities())
	{
		return -1;
	}

	int iCost = 0;
	for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
	{
		iCost += getYieldProductionNeeded(eUnit, (YieldTypes) iYield);
	}

	iCost *= GC.getUnitInfo(eUnit).getAdvancedStartCost();
	if (iCost < 0)
	{
		return -1;
	}
	iCost /= 100;

	if (NULL == pPlot)
	{
		if (bAdd)
		{
			bool bValid = false;
			int iLoop;
			for (CvCity* pLoopCity = firstCity(&iLoop); NULL != pLoopCity; pLoopCity = nextCity(&iLoop))
			{
				if (pLoopCity->canTrain(eUnit))
				{
					bValid = true;
					break;
				}
			}

			if (!bValid)
			{
				return -1;
			}
		}
	}
	else
	{
		CvCity* pCity = NULL;

		if (0 == GC.getDefineINT("ADVANCED_START_ALLOW_UNITS_OUTSIDE_CITIES"))
		{
			pCity = pPlot->getPlotCity();

			if (NULL == pCity || pCity->getOwnerINLINE() != getID())
			{
				return -1;
			}

			iCost *= 100;
			iCost /= std::max(1, 100 + pCity->getProductionModifier(eUnit));
		}
		else
		{
			if (pPlot->getOwnerINLINE() != getID())
			{
				return -1;
			}

			iCost *= 100;
			iCost /= std::max(1, 100 + getProductionModifier(eUnit));
		}


		if (bAdd)
		{
			int iMaxUnitsPerCity = GC.getDefineINT("ADVANCED_START_MAX_UNITS_PER_CITY");
			if (iMaxUnitsPerCity >= 0)
			{
				if (getNumUnits() >= iMaxUnitsPerCity * getNumCities())
				{
					return -1;
				}
			}

			if (NULL != pCity)
			{
				if (!pCity->canTrain(eUnit))
				{
					return -1;
				}
			}
			else
			{
				if (!pPlot->canTrain(eUnit, false, false))
				{
					return -1;
				}

				if (pPlot->isImpassable() && !GC.getUnitInfo(eUnit).isCanMoveImpassable())
				{
					return -1;
				}

				if (pPlot->getFeatureType() != NO_FEATURE)
				{
					if (GC.getUnitInfo(eUnit).getFeatureImpassable(pPlot->getFeatureType()))
					{
						return -1;
					}
				}
				else
				{
					if (GC.getUnitInfo(eUnit).getTerrainImpassable(pPlot->getTerrainType()))
					{
						return -1;
					}
				}
			}
		}
		// Must be this unit at plot in order to remove
		else
		{
			bool bUnitFound = false;

			CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();
			while (pUnitNode != NULL)
			{
				CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
				pUnitNode = pPlot->nextUnitNode(pUnitNode);

				if (pLoopUnit->getUnitType() == eUnit)
				{
					bUnitFound = true;
				}
			}

			if (!bUnitFound)
			{
				return -1;
			}
		}
	}

	// Increase cost if the XML defines that additional units will cost more
	if (0 != GC.getUnitInfo(eUnit).getAdvancedStartCostIncrease())
	{
		for (CvUnit* pLoopUnit = firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = nextUnit(&iLoop))
		{
			if (pLoopUnit->getUnitType() == eUnit)
			{
				++iNumUnitType;
			}
		}

		if (!bAdd)
		{
			--iNumUnitType;
		}

		if (iNumUnitType > 0)
		{
			iCost *= 100 + GC.getUnitInfo(eUnit).getAdvancedStartCostIncrease() * iNumUnitType;
			iCost /= 100;
		}
	}
	
	return iCost;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Adding or removing a City
/////////////////////////////////////////////////////////////////////////////////////////////

int CvPlayer::getAdvancedStartCityCost(bool bAdd, CvPlot* pPlot)
{
	int iNumCities = getNumCities();

	int iCost = getNewCityProductionValue();

	if (iCost < 0)
	{
		return -1;
	}
	
	// Valid plot?
	if (pPlot != NULL)
	{
		// Need valid plot to found on if adding
		if (bAdd)
		{
			if (!canFound(pPlot->getX(), pPlot->getY(), false))
			{
				return -1;
			}
		}
		// Need your own city present to remove
		else
		{
			if (pPlot->isCity())
			{
				if (pPlot->getPlotCity()->getOwnerINLINE() != getID())
				{
					return -1;
				}
			}
			else
			{
				return -1;
			}
		}

		// Is there a distance limit on how far a city can be placed from a player's start/another city?
		if (GC.getDefineINT("ADVANCED_START_CITY_PLACEMENT_MAX_RANGE") > 0)
		{
			PlayerTypes eClosestPlayer = NO_PLAYER;
			int iClosestDistance = MAX_INT;

			for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
			{
				CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)iPlayer);

				if (kPlayer.isAlive())
				{
					CvPlot* pStartingPlot = kPlayer.getStartingPlot();

					if (NULL != pStartingPlot)
					{
						int iDistance = ::plotDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE());
						if (iDistance <= GC.getDefineINT("ADVANCED_START_CITY_PLACEMENT_MAX_RANGE"))
						{
							if (iDistance < iClosestDistance || (iDistance == iClosestDistance && getTeam() != kPlayer.getTeam()))
							{
								iClosestDistance = iDistance;
								eClosestPlayer = kPlayer.getID();
							}
						}				
					}
				}
			}

			if (NO_PLAYER == eClosestPlayer || GET_PLAYER(eClosestPlayer).getTeam() != getTeam())
			{
				return -1;
			}
			//Only allow founding a city at someone elses start point if 
			//We have no cities and they have no cities.
			if ((getID() != eClosestPlayer) && ((getNumCities() > 0) || (GET_PLAYER(eClosestPlayer).getNumCities() > 0)))
			{
				return -1;
			}
		}
	}

	// Increase cost if the XML defines that additional units will cost more
	if (0 != GC.getDefineINT("ADVANCED_START_CITY_COST_INCREASE"))
	{
		if (!bAdd)
		{
			--iNumCities;
		}

		if (iNumCities > 0)
		{
			iCost *= 100 + GC.getDefineINT("ADVANCED_START_CITY_COST_INCREASE") * iNumCities;
			iCost /= 100;
		}
	}
	
	return iCost;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Adding or removing Population
/////////////////////////////////////////////////////////////////////////////////////////////

int CvPlayer::getAdvancedStartPopCost(bool bAdd, CvCity* pCity)
{
	if (0 == getNumCities())
	{
		return -1;
	}

	int iCost = (getGrowthThreshold(1) * GC.getDefineINT("ADVANCED_START_POPULATION_COST")) / 100;

	if (NULL != pCity)
	{
		if (pCity->getOwnerINLINE() != getID())
		{
			return -1;
		}

		int iPopulation = pCity->getPopulation();

		// Need to have Population to remove it
		if (!bAdd)
		{
			--iPopulation;

			if (iPopulation < GC.getDefineINT("INITIAL_CITY_POPULATION") + GC.getEraInfo(GC.getGameINLINE().getStartEra()).getFreePopulation())
			{
				return -1;
			}
		}

		iCost = (getGrowthThreshold(iPopulation) * GC.getDefineINT("ADVANCED_START_POPULATION_COST")) / 100;

		// Increase cost if the XML defines that additional Pop will cost more
		if (0 != GC.getDefineINT("ADVANCED_START_POPULATION_COST_INCREASE"))
		{
			--iPopulation;

			if (iPopulation > 0)
			{
				iCost *= 100 + GC.getDefineINT("ADVANCED_START_POPULATION_COST_INCREASE") * iPopulation;
				iCost /= 100;
			}
		}
	}

	return iCost;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Adding or removing Culture
/////////////////////////////////////////////////////////////////////////////////////////////

int CvPlayer::getAdvancedStartCultureCost(bool bAdd, CvCity* pCity)
{
	if (0 == getNumCities())
	{
		return -1;
	}

	int iCost = GC.getDefineINT("ADVANCED_START_CULTURE_COST");
	if (iCost < 0)
	{
		return -1;
	}

	if (NULL != pCity)
	{
		if (pCity->getOwnerINLINE() != getID())
		{
			return -1;
		}

		// Need to have enough culture to remove it
		if (!bAdd)
		{
			if (pCity->getCultureLevel() <= 0)
			{
				return -1;
			}
		}

		int iCulture;
		if (bAdd)
		{
			iCulture = CvCity::getCultureThreshold((CultureLevelTypes)(pCity->getCultureLevel() + 1)) - pCity->getCulture(getID());
		}
		else
		{
			iCulture = pCity->getCulture(getID()) - CvCity::getCultureThreshold((CultureLevelTypes)(pCity->getCultureLevel() - 1));
		}

		iCost *= iCulture;
		iCost *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getGrowthPercent();
		iCost /= 10000;
	}

	return iCost;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Adding or removing a Building from a city
/////////////////////////////////////////////////////////////////////////////////////////////

int CvPlayer::getAdvancedStartBuildingCost(BuildingTypes eBuilding, bool bAdd, CvCity* pCity)
{
	CvBuildingInfo& kBuilding = GC.getBuildingInfo(eBuilding);
	if (0 == getNumCities())
	{
		return -1;
	}

	int iNumBuildingType = 0;

	int iCost = 0;
	for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
	{
		iCost += getYieldProductionNeeded(eBuilding, (YieldTypes) iYield);
	}

	iCost *= kBuilding.getAdvancedStartCost();
	if (iCost < 0)
	{
		return -1;
	}
	iCost /= 100;

	if (kBuilding.getFreeStartEra() != NO_ERA && GC.getGameINLINE().getStartEra() >=  kBuilding.getFreeStartEra())
	{
		// you get this building for free
		return -1;
	}

	if (NULL == pCity)
	{
		if (bAdd)
		{
			bool bValid = false;
			int iLoop;
			for (CvCity* pLoopCity = firstCity(&iLoop); NULL != pLoopCity; pLoopCity = nextCity(&iLoop))
			{
				if (pLoopCity->canConstruct(eBuilding))
				{
					bValid = true;
					break;
				}
			}

			if (!bValid)
			{
				return -1;
			}
		}
	}
	if (NULL != pCity)
	{
		if (pCity->getOwnerINLINE() != getID())
		{
			return -1;
		}

		iCost *= 100;
		iCost /= std::max(1, 100 + pCity->getProductionModifier(eBuilding));

		if (bAdd)
		{
			if (!pCity->canConstruct(eBuilding, true, false, false))
			{
				return -1;
			}
		}
		else
		{
			if (!pCity->isHasRealBuilding(eBuilding))
			{
				return -1;
			}

			// Check other buildings in this city and make sure none of them require this one
			for (int iBuildingLoop = 0; iBuildingLoop < GC.getNumBuildingInfos(); iBuildingLoop++)
			{
				BuildingTypes eBuildingLoop = (BuildingTypes) iBuildingLoop;
				if (pCity->isHasRealBuilding(eBuildingLoop))
				{
					if (GC.getBuildingInfo(eBuildingLoop).isBuildingClassNeededInCity(kBuilding.getBuildingClassType()))
					{
						return -1;
					}
				}
			}
		}
	}

	// Increase cost if the XML defines that additional Buildings will cost more
	if (0 != GC.getBuildingInfo(eBuilding).getAdvancedStartCostIncrease())
	{
		iNumBuildingType = countNumBuildings(eBuilding);
		
		if (!bAdd)
		{
			--iNumBuildingType;
		}

		if (iNumBuildingType > 0)
		{
			iCost *= 100 + GC.getBuildingInfo(eBuilding).getAdvancedStartCostIncrease() * std::max(0, iNumBuildingType - getNumCities());
			iCost /= 100;
		}
	}

	return iCost;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Adding or removing Route
/////////////////////////////////////////////////////////////////////////////////////////////

int CvPlayer::getAdvancedStartRouteCost(RouteTypes eRoute, bool bAdd, CvPlot* pPlot)
{
	if (0 == getNumCities())
	{
		return -1;
	}

	if (eRoute == NO_ROUTE)
	{
		return -1;
	}

	int iNumRoutes = 0;

	int iCost = GC.getRouteInfo(eRoute).getAdvancedStartCost();

	// This denotes cities may not be purchased through Advanced Start
	if (iCost < 0)
	{
		return -1;
	}

	iCost *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getGrowthPercent();
	iCost /= 100;

	// No invalid plots!
	if (pPlot != NULL)
	{
		if (pPlot->isCity())
		{
			return -1;
		}

		if (bAdd)
		{
			if (pPlot->isImpassable() || pPlot->isWater())
			{
				return -1;
			}
			// Can't place twice
			if (pPlot->getRouteType() == eRoute)
			{
				return -1;
			}
		}
		else
		{
			// Need Route to remove it
			if (pPlot->getRouteType() != eRoute)
			{
				return -1;
			}
		}

		// Must be owned by me
		if (pPlot->getOwner() != getID())
		{
			return -1;
		}
	}
	// Increase cost if the XML defines that additional units will cost more
	if (0 != GC.getRouteInfo(eRoute).getAdvancedStartCostIncrease())
	{
		int iPlotLoop = 0;
		CvPlot* pPlot;

		for (iPlotLoop = 0; iPlotLoop < GC.getMapINLINE().numPlots(); iPlotLoop++)
		{
			pPlot = GC.getMapINLINE().plotByIndex(iPlotLoop);

			if (pPlot->getRouteType() == eRoute)
			{
				++iNumRoutes;
			}
		}

		if (!bAdd)
		{
			--iNumRoutes;
		}

		if (iNumRoutes > 0)
		{
			iCost *= 100 + GC.getRouteInfo(eRoute).getAdvancedStartCostIncrease() * iNumRoutes;
			iCost /= 100;
		}
	}

	return iCost;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Adding or removing Improvement
/////////////////////////////////////////////////////////////////////////////////////////////

int CvPlayer::getAdvancedStartImprovementCost(ImprovementTypes eImprovement, bool bAdd, CvPlot* pPlot)
{
	if (eImprovement == NO_IMPROVEMENT)
	{
		return -1;
	}

	if (0 == getNumCities())
	{
		return -1;
	}

	int iNumImprovements = 0;
	int iCost = GC.getImprovementInfo(eImprovement).getAdvancedStartCost();

		// This denotes cities may not be purchased through Advanced Start
	if (iCost < 0)
	{
		return -1;
	}

	iCost *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getGrowthPercent();
	iCost /= 100;

	// Can this Improvement be on our plot?
	if (pPlot != NULL)
	{
		if (bAdd)
		{
			// Valid Plot
			if (!pPlot->canHaveImprovement(eImprovement, getTeam(), false))
			{
				return -1;
			}

			bool bValid = false;

			for (int iI = 0; iI < GC.getNumBuildInfos(); ++iI)
			{
				CvBuildInfo& kBuild = GC.getBuildInfo((BuildTypes)iI);
				ImprovementTypes eLoopImprovement = ((ImprovementTypes)(kBuild.getImprovement()));
				if (eImprovement == eLoopImprovement && canBuild(pPlot, (BuildTypes)iI))
				{
					bValid = true;
					FeatureTypes eFeature = pPlot->getFeatureType();
					if (NO_FEATURE != eFeature && kBuild.isFeatureRemove(eFeature))
					{
						iCost += GC.getFeatureInfo(eFeature).getAdvancedStartRemoveCost();
					}
					break;
				}
			}
			if (!bValid)
			{
				return -1;
			}
			// Can't place twice
			if (pPlot->getImprovementType() == eImprovement)
			{
				return -1;
			}
		}
		else
		{
			// Need this improvement in order to remove it
			if (pPlot->getImprovementType() != eImprovement)
			{
				return -1;
			}
		}
		// Must be owned by me
		if (pPlot->getOwner() != getID())
		{
			return -1;
		}
	}
	// Increase cost if the XML defines that additional units will cost more
	if (0 != GC.getImprovementInfo(eImprovement).getAdvancedStartCostIncrease())
	{
		int iPlotLoop = 0;
		CvPlot* pPlot;
		for (iPlotLoop = 0; iPlotLoop < GC.getMapINLINE().numPlots(); iPlotLoop++)
		{
			pPlot = GC.getMapINLINE().plotByIndex(iPlotLoop);
			if (pPlot->getImprovementType() == eImprovement)
			{
				++iNumImprovements;
			}
		}
		if (!bAdd)
		{
			--iNumImprovements;
		}
		if (iNumImprovements > 0)
		{
			iCost *= 100 + GC.getImprovementInfo(eImprovement).getAdvancedStartCostIncrease() * iNumImprovements;
			iCost /= 100;
		}
	}

	return iCost;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Adding or removing Visibility
/////////////////////////////////////////////////////////////////////////////////////////////

int CvPlayer::getAdvancedStartVisibilityCost(bool bAdd, CvPlot* pPlot)
{
	if (0 == getNumCities())
	{
		return -1;
	}

	int iNumVisiblePlots = 0;
	int iCost = GC.getDefineINT("ADVANCED_START_VISIBILITY_COST");

	// This denotes Visibility may not be purchased through Advanced Start
	if (iCost == -1)
	{
		return -1;
	}

	// Valid Plot?
	if (pPlot != NULL)
	{
		if (bAdd)
		{
			if (pPlot->isRevealed(getTeam(), false))
			{
				return -1;
			}
			if (!pPlot->isAdjacentRevealed(getTeam()))
			{
				return -1;
			}
		}
		else
		{
			if (!pPlot->isRevealed(getTeam(), false))
			{
				return -1;
			}
		}
	}

	// Increase cost if the XML defines that additional units will cost more
	if (0 != GC.getDefineINT("ADVANCED_START_VISIBILITY_COST_INCREASE"))
	{
		int iPlotLoop = 0;
		CvPlot* pPlot;

		for (iPlotLoop = 0; iPlotLoop < GC.getMapINLINE().numPlots(); iPlotLoop++)
		{
			pPlot = GC.getMapINLINE().plotByIndex(iPlotLoop);

			if (pPlot->isRevealed(getTeam(), false))
			{
				++iNumVisiblePlots;
			}
		}

		if (!bAdd)
		{
			--iNumVisiblePlots;
		}

		if (iNumVisiblePlots > 0)
		{
			iCost *= 100 + GC.getDefineINT("ADVANCED_START_VISIBILITY_COST_INCREASE") * iNumVisiblePlots;
			iCost /= 100;
		}
	}

	return iCost;
}

void CvPlayer::doWarnings()
{
	//update enemy unit in your territory glow
	int iLoop;
	for (CvUnit* pLoopUnit = firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = nextUnit(&iLoop))
	{
		//update glow
		gDLL->getEntityIFace()->updateEnemyGlow(pLoopUnit->getUnitEntity());
	}

	//update enemy units close to your territory
	int iMaxCount = range(((getNumCities() + 4) / 7), 2, 5);
	for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		if (iMaxCount == 0)
		{
			break;
		}

		CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (pLoopPlot->isAdjacentPlayer(getID()))
		{
			if (!(pLoopPlot->isCity()))
			{
				if (pLoopPlot->isVisible(getTeam(), false))
				{
					CvUnit *pUnit = pLoopPlot->getVisibleEnemyDefender(getID());
					if (pUnit != NULL)
					{
						CvCity* pNearestCity = GC.getMapINLINE().findCity(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), getID(), NO_TEAM, !(pLoopPlot->isWater()));
						if (pNearestCity != NULL)
						{	
							CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_ENEMY_TROOPS_SPOTTED", pNearestCity->getNameKey());
							gDLL->getInterfaceIFace()->addMessage(getID(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_ENEMY_TROOPS", MESSAGE_TYPE_INFO, pUnit->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), true, true);
							iMaxCount--;
						}
					}
				}
			}
		}
	}
}


void CvPlayer::processCivics(CivicTypes eCivic, int iChange)
{
	CvCivicInfo& kCivicInfo = GC.getCivicInfo(eCivic);

	changeGreatGeneralRateModifier(kCivicInfo.getGreatGeneralRateModifier() * iChange);
	changeDomesticGreatGeneralRateModifier(kCivicInfo.getDomesticGreatGeneralRateModifier() * iChange);
	changeFreeExperience(kCivicInfo.getFreeExperience() * iChange);
	changeWorkerSpeedModifier(kCivicInfo.getWorkerSpeedModifier() * iChange);
	changeImprovementUpgradeRateModifier(kCivicInfo.getImprovementUpgradeRateModifier() * iChange);
	changeMilitaryProductionModifier(kCivicInfo.getMilitaryProductionModifier() * iChange);
	changeExpInBorderModifier(kCivicInfo.getExpInBorderModifier() * iChange);
	changeNativeCombatModifier(kCivicInfo.getNativeCombatModifier() * iChange);
	changeDominateNativeBordersCount(kCivicInfo.isDominateNativeBorders() ? iChange : 0);
	changeRevolutionEuropeTradeCount(kCivicInfo.isRevolutionEuropeTrade() ? iChange : 0);
	setFatherPointMultiplier(getFatherPointMultiplier() + kCivicInfo.getFatherPointModifier() * iChange);

	if (kCivicInfo.getImmigrationConversion() != NO_YIELD)
	{
		if (iChange > 0)
		{
			setImmigrationConversion((YieldTypes) kCivicInfo.getImmigrationConversion());
		}
		else
		{
			setImmigrationConversion(YIELD_CROSSES);
		}
	}

	if (kCivicInfo.getNativeAttitudeChange() != 0)
	{
		for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
		{
			CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes) iPlayer);

			if (kLoopPlayer.isAlive() && kLoopPlayer.isNative())
			{
				kLoopPlayer.AI_changeAttitudeExtra(getID(), kCivicInfo.getNativeAttitudeChange() * iChange);

				if (iChange > 0 && kCivicInfo.getNativeAttitudeChange() > 0)
				{
					GET_TEAM(getTeam()).makePeace(kLoopPlayer.getTeam());
				}
			}
		}
	}

	for (int iProfession = 0; iProfession < GC.getNumProfessionInfos(); ++iProfession)
	{
		changeProfessionCombatChange((ProfessionTypes) iProfession, kCivicInfo.getProfessionCombatChange(iProfession) * iChange);
	}

	for (int iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		changeYieldRateModifier(((YieldTypes)iI), (kCivicInfo.getYieldModifier(iI) * iChange));
		changeCapitalYieldRateModifier(((YieldTypes)iI), (kCivicInfo.getCapitalYieldModifier(iI) * iChange));
	}

	for (int iI = 0; iI < GC.getNumHurryInfos(); iI++)
	{
		changeHurryCount(((HurryTypes)iI), ((kCivicInfo.isHurry(iI)) ? iChange : 0));
	}

	for (int iI = 0; iI < GC.getNumSpecialBuildingInfos(); iI++)
	{
		changeSpecialBuildingNotRequiredCount(((SpecialBuildingTypes)iI), ((kCivicInfo.isSpecialBuildingNotRequired(iI)) ? iChange : 0));
	}

	for (int iI = 0; iI < GC.getNumImprovementInfos(); iI++)
	{
		for (int iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
		{
			changeImprovementYieldChange(((ImprovementTypes)iI), ((YieldTypes)iJ), (kCivicInfo.getImprovementYieldChanges(iI, iJ) * iChange));
		}
	}

	for (int i = 0; i < kCivicInfo.getNumFreeUnitClasses(); ++i)
	{
		UnitTypes eUnit = (UnitTypes) GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(kCivicInfo.getFreeUnitClass(i));
		if (eUnit != NO_UNIT)
		{
			int iLoop;
			for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
			{
				if (iChange > 0)
				{
					int iTargetPercent = pLoopCity->getRebelPercent();
					for (int j = 0; j < iChange; ++j)
					{
						CvUnit* pUnit = initUnit(eUnit, (ProfessionTypes) GC.getUnitInfo(eUnit).getDefaultProfession(), pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE());
						if (pUnit != NULL)
						{
							pLoopCity->addPopulationUnit(pUnit, NO_PROFESSION);
						}				
					}
					if (pLoopCity->getRebelPercent() < iTargetPercent)
					{
						pLoopCity->setRebelSentiment(iTargetPercent * pLoopCity->getPopulation());
					}
				}
				else if (iChange < 0)
				{
					int iRemoved = 0;

					// remove proper unit from city
					for (int j = 0; j < -iChange; ++j)
					{
						CvUnit* pUnit = pLoopCity->removeUnitType(eUnit, (ProfessionTypes) GC.getCivilizationInfo(getCivilizationType()).getDefaultProfession());
						if (pUnit != NULL)
						{
							pUnit->kill(false);
							++iRemoved;
						}
					}

					// remove proper unit from field
					for (int j = iRemoved; j < -iChange; ++j)
					{
						int iUnitLoop;
						for (CvUnit* pUnit = firstUnit(&iUnitLoop); pUnit != NULL; pUnit = nextUnit(&iLoop))
						{
							if (pUnit->getUnitType() == eUnit)
							{
								pUnit->kill(false);
								++iRemoved;
								break;
							}
						}
					}

					// remove any city population
					int iRemaining = -iChange - iRemoved;
					if (iRemaining > 0 && pLoopCity->getPopulation() > iRemaining)
					{
						pLoopCity->changePopulation(-iRemaining);
						iRemoved += iRemaining;
					}

					// remove any unit from field
					for (int j = iRemoved; j < -iChange; ++j)
					{
						int iUnitLoop;
						for (CvUnit* pUnit = firstUnit(&iUnitLoop); pUnit != NULL; pUnit = nextUnit(&iLoop))
						{
							pUnit->kill(false);
							++iRemoved;
							break;
						}
					}
				}
			}
		}
	}
}

void CvPlayer::showMissedMessages()
{
	CvMessageQueue::iterator it = m_listGameMessages.begin();
	while (it != m_listGameMessages.end())
	{
		CvTalkingHeadMessage& msg = *it;
		if (!msg.getShown())
		{
			msg.setShown(true);
			gDLL->getInterfaceIFace()->showMessage(msg);
		}
		++it;
	}
}

bool CvPlayer::isPbemNewTurn() const
{
	return m_bPbemNewTurn;
}

void CvPlayer::setPbemNewTurn(bool bNew)
{
	m_bPbemNewTurn = bNew;
}



//
// read object from a stream
// used during load
//
void CvPlayer::read(FDataStreamBase* pStream)
{
	int iI;

	// Init data before load
	reset();

	uint uiFlag=0;
	pStream->Read(&uiFlag);	// flags for expansion

	pStream->Read(&m_iStartingX);
	pStream->Read(&m_iStartingY);
	pStream->Read(&m_iTotalPopulation);
	pStream->Read(&m_iTotalLand);
	pStream->Read(&m_iTotalLandScored);
	pStream->Read(&m_iGold);
	pStream->Read(&m_iAdvancedStartPoints);
	pStream->Read(&m_iGreatGeneralsCreated);
	pStream->Read(&m_iGreatGeneralsThresholdModifier);
	pStream->Read(&m_iGreatGeneralRateModifier);
	pStream->Read(&m_iDomesticGreatGeneralRateModifier);
	pStream->Read(&m_iImmigrationThresholdMultiplier);
	pStream->Read(&m_iRevolutionEuropeUnitThresholdMultiplier);
	if (uiFlag > 1)
	{
		pStream->Read(&m_iKingNumUnitMultiplier);
	}
	if (uiFlag <= 2) 
	{
		int iEducationThresholdMultiplier;
		pStream->Read(&iEducationThresholdMultiplier);
	}
	pStream->Read(&m_iNativeAngerModifier);
	pStream->Read(&m_iFreeExperience);
	pStream->Read(&m_iWorkerSpeedModifier);
	pStream->Read(&m_iImprovementUpgradeRateModifier);
	pStream->Read(&m_iMilitaryProductionModifier);
	pStream->Read(&m_iCityDefenseModifier);
	pStream->Read(&m_iHighestUnitLevel);
	pStream->Read(&m_iFatherOverflowBells);
	pStream->Read(&m_iExpInBorderModifier);
	pStream->Read(&m_iLevelExperienceModifier);
	pStream->Read(&m_iCapitalCityID);
	pStream->Read(&m_iCitiesLost);
	pStream->Read(&m_iAssets);
	pStream->Read(&m_iPower);
	pStream->Read(&m_iPopulationScore);
	pStream->Read(&m_iLandScore);
	pStream->Read(&m_iFatherScore);
	pStream->Read(&m_iCombatExperience);
	if (uiFlag > 1)
	{
		pStream->Read(&m_iMissionarySuccessPercent);
	}

	pStream->Read(&m_bAlive);
	pStream->Read(&m_bEverAlive);
	pStream->Read(&m_bTurnActive);
	pStream->Read(&m_bAutoMoves);
	pStream->Read(&m_bEndTurn);
	pStream->Read(&m_bPbemNewTurn);
	pStream->Read(&m_bExtendedGame);
	pStream->Read(&m_bFoundedFirstCity);
	pStream->Read(&m_bStrike);

	pStream->Read((int*)&m_eID);
	pStream->Read((int*)&m_ePersonalityType);
	pStream->Read((int*)&m_eCurrentEra);
	pStream->Read((int*)&m_eParent);
	updateTeamType(); //m_eTeamType not saved
	updateHuman();
	pStream->Read((int*)&m_eImmigrationConversion);

	pStream->Read(NUM_YIELD_TYPES, m_aiSeaPlotYield);
	pStream->Read(NUM_YIELD_TYPES, m_aiYieldRateModifier);
	pStream->Read(NUM_YIELD_TYPES, m_aiCapitalYieldRateModifier);
	pStream->Read(NUM_YIELD_TYPES, m_aiBuildingRequiredYieldModifier);
	pStream->Read(NUM_YIELD_TYPES, m_aiCityExtraYield);
	pStream->Read(NUM_YIELD_TYPES, m_aiExtraYieldThreshold);
	pStream->Read(NUM_YIELD_TYPES, m_aiYieldBuyPrice);
	pStream->Read(NUM_YIELD_TYPES, m_aiYieldTradedTotal);
	pStream->Read(NUM_YIELD_TYPES, m_aiYieldBoughtTotal);
	pStream->Read(NUM_YIELD_TYPES, m_aiTaxYieldModifierCount);
	if (uiFlag > 1)
	{
		pStream->Read(MAX_PLAYERS, m_aiMissionaryPoints);
		pStream->Read(MAX_PLAYERS, m_aiMissionaryThresholdMultiplier);
	}

	pStream->Read(NUM_YIELD_TYPES, m_abYieldEuropeTradable);
	pStream->Read(NUM_FEAT_TYPES, m_abFeatAccomplished);
	pStream->Read(NUM_PLAYEROPTION_TYPES, m_abOptions);

	pStream->ReadString(m_szScriptData);

	FAssertMsg((0 < GC.getNumBonusInfos()), "GC.getNumBonusInfos() is not greater than zero but it is expected to be in CvPlayer::read");
	pStream->Read(GC.getNumImprovementInfos(), m_paiImprovementCount);
	pStream->Read(GC.getNumBuildingInfos(), m_paiFreeBuildingCount);
	pStream->Read(GC.getNumUnitClassInfos(), m_paiUnitClassCount);
	pStream->Read(GC.getNumUnitClassInfos(), m_paiUnitClassMaking);
	pStream->Read(GC.getNumUnitClassInfos(), m_paiUnitClassImmigrated);
	pStream->Read(GC.getNumUnitClassInfos(), m_paiUnitMoveChange);
	pStream->Read(GC.getNumUnitClassInfos(), m_paiUnitStrengthModifier);
	pStream->Read(GC.getNumProfessionInfos(), m_paiProfessionCombatChange);
	pStream->Read(GC.getNumProfessionInfos(), m_paiProfessionMoveChange);
	pStream->Read(GC.getNumBuildingClassInfos(), m_paiBuildingClassCount);
	pStream->Read(GC.getNumBuildingClassInfos(), m_paiBuildingClassMaking);
	pStream->Read(GC.getNumHurryInfos(), m_paiHurryCount);
	pStream->Read(GC.getNumSpecialBuildingInfos(), m_paiSpecialBuildingNotRequiredCount);
	if (uiFlag <= 1)
	{
		std::vector<int> aiMissionaryPoints(GC.getNumCivilizationInfos());
		pStream->Read(GC.getNumCivilizationInfos(), &aiMissionaryPoints[0]);
		pStream->Read(GC.getNumCivilizationInfos(), &aiMissionaryPoints[0]);
	}
	pStream->Read(GC.getNumProfessionInfos(), m_aiProfessionEquipmentModifier);
	pStream->Read(GC.getNumTraitInfos(), m_aiTraitCount);

	for (iI=0;iI<GC.getNumCivicOptionInfos();iI++)
	{
		pStream->Read((int*)&m_paeCivics[iI]);
	}

	for (iI=0;iI<GC.getNumImprovementInfos();iI++)
	{
		pStream->Read(NUM_YIELD_TYPES, m_ppiImprovementYieldChange[iI]);
	}

	for (iI=0;iI<GC.getNumBuildingClassInfos();iI++)
	{
		pStream->Read(NUM_YIELD_TYPES, m_ppiBuildingYieldChange[iI]);
	}

	m_groupCycle.Read(pStream);
	{
		CvWString szBuffer;
		uint iSize;
		pStream->Read(&iSize);
		m_aszCityNames.resize(iSize);
		for (uint i = 0; i < iSize; i++)
		{
			pStream->ReadString(szBuffer);
			m_aszCityNames[i] = szBuffer;
		}
	}

	ReadStreamableFFreeListTrashArray(m_cities, pStream);
	m_tradeRoutes.Read(pStream);
	m_units.Read(pStream);

	freeEuropeUnits();
	int iNumEuropeUnits;
	pStream->Read(&iNumEuropeUnits);
	for(int i=0;i<iNumEuropeUnits;i++)
	{
		CvUnitAI *pUnit = new CvUnitAI();
		pUnit->read(pStream);
		m_aEuropeUnits.push_back(pUnit);
	}

	ReadStreamableFFreeListTrashArray(m_selectionGroups, pStream);
	ReadStreamableFFreeListTrashArray(m_eventsTriggered, pStream);

	{
		CvMessageQueue::_Alloc::size_type iSize;
		pStream->Read(&iSize);
		for (CvMessageQueue::_Alloc::size_type i = 0; i < iSize; i++)
		{
			CvTalkingHeadMessage message;
			message.read(*pStream);
			m_listGameMessages.push_back(message);
		}
	}

	{
		clearPopups();
		CvPopupQueue::_Alloc::size_type iSize;
		pStream->Read(&iSize);
		for (CvPopupQueue::_Alloc::size_type i = 0; i < iSize; i++)
		{
			CvPopupInfo* pInfo = new CvPopupInfo();
			if (NULL != pInfo)
			{
				pInfo->read(*pStream);
				m_listPopups.push_back(pInfo);
			}
		}
	}

	{
		clearDiplomacy();
		CvDiploQueue::_Alloc::size_type iSize;
		pStream->Read(&iSize);
		for (CvDiploQueue::_Alloc::size_type i = 0; i < iSize; i++)
		{
			CvDiploParameters* pDiplo = new CvDiploParameters(NO_PLAYER);
			if (NULL != pDiplo)
			{
				pDiplo->read(*pStream);
				m_listDiplomacy.push_back(pDiplo);
			}
		}
	}

	{
		uint iSize;
		pStream->Read(&iSize);
		for (uint i = 0; i < iSize; i++)
		{
			int iTurn;
			int iScore;
			pStream->Read(&iTurn);
			pStream->Read(&iScore);
			m_mapScoreHistory[iTurn] = iScore;
		}
	}

	{
		m_mapEconomyHistory.clear();
		uint iSize;
		pStream->Read(&iSize);
		for (uint i = 0; i < iSize; i++)
		{
			int iTurn;
			int iScore;
			pStream->Read(&iTurn);
			pStream->Read(&iScore);
			m_mapEconomyHistory[iTurn] = iScore;
		}
	}

	{
		m_mapIndustryHistory.clear();
		uint iSize;
		pStream->Read(&iSize);
		for (uint i = 0; i < iSize; i++)
		{
			int iTurn;
			int iScore;
			pStream->Read(&iTurn);
			pStream->Read(&iScore);
			m_mapIndustryHistory[iTurn] = iScore;
		}
	}

	{
		m_mapAgricultureHistory.clear();
		uint iSize;
		pStream->Read(&iSize);
		for (uint i = 0; i < iSize; i++)
		{
			int iTurn;
			int iScore;
			pStream->Read(&iTurn);
			pStream->Read(&iScore);
			m_mapAgricultureHistory[iTurn] = iScore;
		}
	}

	{
		m_mapPowerHistory.clear();
		uint iSize;
		pStream->Read(&iSize);
		for (uint i = 0; i < iSize; i++)
		{
			int iTurn;
			int iScore;
			pStream->Read(&iTurn);
			pStream->Read(&iScore);
			m_mapPowerHistory[iTurn] = iScore;
		}
	}

	{
		m_mapCultureHistory.clear();
		uint iSize;
		pStream->Read(&iSize);
		for (uint i = 0; i < iSize; i++)
		{
			int iTurn;
			int iScore;
			pStream->Read(&iTurn);
			pStream->Read(&iScore);
			m_mapCultureHistory[iTurn] = iScore;
		}
	}

	{
		m_mapEventsOccured.clear();
		uint iSize;
		pStream->Read(&iSize);
		for (uint i = 0; i < iSize; i++)
		{
			EventTriggeredData kData;
			EventTypes eEvent;
			pStream->Read((int*)&eEvent);
			kData.read(pStream);
			m_mapEventsOccured[eEvent] = kData;
		}
	}

	{
		m_mapEventCountdown.clear();
		uint iSize;
		pStream->Read(&iSize);
		for (uint i = 0; i < iSize; i++)
		{
			EventTriggeredData kData;
			EventTypes eEvent;
			pStream->Read((int*)&eEvent);
			kData.read(pStream);
			m_mapEventCountdown[eEvent] = kData;
		}
	}

	{
		m_aFreeUnitCombatPromotions.clear();
		uint iSize;
		pStream->Read(&iSize);
		for (uint i = 0; i < iSize; i++)
		{
			int iUnitCombat;
			int iPromotion;
			pStream->Read(&iUnitCombat);
			pStream->Read(&iPromotion);
			m_aFreeUnitCombatPromotions.push_back(std::make_pair((UnitCombatTypes)iUnitCombat, (PromotionTypes)iPromotion));
		}
	}

	{
		m_aFreeUnitClassPromotions.clear();
		uint iSize;
		pStream->Read(&iSize);
		for (uint i = 0; i < iSize; i++)
		{
			int iUnitClass;
			int iPromotion;
			pStream->Read(&iUnitClass);
			pStream->Read(&iPromotion);
			m_aFreeUnitClassPromotions.push_back(std::make_pair((UnitClassTypes)iUnitClass, (PromotionTypes)iPromotion));
		}
	}

	{
		m_aEuropeRevolutionUnits.clear();
		uint iSize;
		pStream->Read(&iSize);
		for (uint i = 0; i < iSize; i++)
		{
			ProfessionTypes eProfession;
			UnitTypes eUnit;
			pStream->Read((int*)&eUnit);
			pStream->Read((int*)&eProfession);
			m_aEuropeRevolutionUnits.push_back(std::make_pair(eUnit, eProfession));
		}
	}

	{
		m_aDocksNextUnits.clear();
		uint iSize;
		pStream->Read(&iSize);
		for (uint i = 0; i < iSize; i++)
		{
			UnitTypes eUnit;
			pStream->Read((int*)&eUnit);
			m_aDocksNextUnits.push_back(eUnit);
		}
	}

	m_triggersFired.clear();
	uint iSize;
	pStream->Read(&iSize);
	for (uint i = 0; i < iSize; i++)
	{
		int iTrigger;
		pStream->Read(&iTrigger);
		m_triggersFired.push_back((EventTriggerTypes)iTrigger);
	}

	// Get the NetID from the initialization structure
	setNetID(gDLL->getAssignedNetworkID(getID()));

	pStream->Read(&m_iPopRushHurryCount);
	pStream->Read(&m_iCrossesStored);
	pStream->Read(&m_iBellsStored);
	pStream->Read(&m_iTaxRate);
	pStream->Read(&m_iNativeCombatModifier);
	pStream->Read(&m_iDominateNativeBordersCount);
	pStream->Read(&m_iRevolutionEuropeTradeCount);
	pStream->Read(&m_iFatherPointMultiplier);
	pStream->Read(&m_iMissionaryRateModifier);
}

//
// save object to a stream
// used during save
//
void CvPlayer::write(FDataStreamBase* pStream)
{
	int iI;

	uint uiFlag = 3;
	pStream->Write(uiFlag);		// flag for expansion

	pStream->Write(m_iStartingX);
	pStream->Write(m_iStartingY);
	pStream->Write(m_iTotalPopulation);
	pStream->Write(m_iTotalLand);
	pStream->Write(m_iTotalLandScored);
	pStream->Write(m_iGold);
	pStream->Write(m_iAdvancedStartPoints);
	pStream->Write(m_iGreatGeneralsCreated);
	pStream->Write(m_iGreatGeneralsThresholdModifier);
	pStream->Write(m_iGreatGeneralRateModifier);
	pStream->Write(m_iDomesticGreatGeneralRateModifier);
	pStream->Write(m_iImmigrationThresholdMultiplier);
	pStream->Write(m_iRevolutionEuropeUnitThresholdMultiplier);
	pStream->Write(m_iKingNumUnitMultiplier);
	pStream->Write(m_iNativeAngerModifier);
	pStream->Write(m_iFreeExperience);
	pStream->Write(m_iWorkerSpeedModifier);
	pStream->Write(m_iImprovementUpgradeRateModifier);
	pStream->Write(m_iMilitaryProductionModifier);
	pStream->Write(m_iCityDefenseModifier);
	pStream->Write(m_iHighestUnitLevel);
	pStream->Write(m_iFatherOverflowBells);
	pStream->Write(m_iExpInBorderModifier);
	pStream->Write(m_iLevelExperienceModifier);
	pStream->Write(m_iCapitalCityID);
	pStream->Write(m_iCitiesLost);
	pStream->Write(m_iAssets);
	pStream->Write(m_iPower);
	pStream->Write(m_iPopulationScore);
	pStream->Write(m_iLandScore);
	pStream->Write(m_iFatherScore);
	pStream->Write(m_iCombatExperience);
	pStream->Write(m_iMissionarySuccessPercent);

	pStream->Write(m_bAlive);
	pStream->Write(m_bEverAlive);
	pStream->Write(m_bTurnActive);
	pStream->Write(m_bAutoMoves);
	pStream->Write(m_bEndTurn);
	pStream->Write(m_bPbemNewTurn && GC.getGameINLINE().isPbem());
	pStream->Write(m_bExtendedGame);
	pStream->Write(m_bFoundedFirstCity);
	pStream->Write(m_bStrike);

	pStream->Write(m_eID);
	pStream->Write(m_ePersonalityType);
	pStream->Write(m_eCurrentEra);
	pStream->Write(m_eParent);
	//m_eTeamType not saved
	pStream->Write(m_eImmigrationConversion);

	pStream->Write(NUM_YIELD_TYPES, m_aiSeaPlotYield);
	pStream->Write(NUM_YIELD_TYPES, m_aiYieldRateModifier);
	pStream->Write(NUM_YIELD_TYPES, m_aiCapitalYieldRateModifier);
	pStream->Write(NUM_YIELD_TYPES, m_aiBuildingRequiredYieldModifier);
	pStream->Write(NUM_YIELD_TYPES, m_aiCityExtraYield);
	pStream->Write(NUM_YIELD_TYPES, m_aiExtraYieldThreshold);
	pStream->Write(NUM_YIELD_TYPES, m_aiYieldBuyPrice);
	pStream->Write(NUM_YIELD_TYPES, m_aiYieldTradedTotal);
	pStream->Write(NUM_YIELD_TYPES, m_aiYieldBoughtTotal);
	pStream->Write(NUM_YIELD_TYPES, m_aiTaxYieldModifierCount);
	pStream->Write(MAX_PLAYERS, m_aiMissionaryPoints);
	pStream->Write(MAX_PLAYERS, m_aiMissionaryThresholdMultiplier);

	pStream->Write(NUM_YIELD_TYPES, m_abYieldEuropeTradable);
	pStream->Write(NUM_FEAT_TYPES, m_abFeatAccomplished);
	pStream->Write(NUM_PLAYEROPTION_TYPES, m_abOptions);

	pStream->WriteString(m_szScriptData);

	FAssertMsg((0 < GC.getNumBonusInfos()), "GC.getNumBonusInfos() is not greater than zero but an array is being allocated in CvPlayer::write");
	pStream->Write(GC.getNumImprovementInfos(), m_paiImprovementCount);
	pStream->Write(GC.getNumBuildingInfos(), m_paiFreeBuildingCount);
	pStream->Write(GC.getNumUnitClassInfos(), m_paiUnitClassCount);
	pStream->Write(GC.getNumUnitClassInfos(), m_paiUnitClassMaking);
	pStream->Write(GC.getNumUnitClassInfos(), m_paiUnitClassImmigrated);
	pStream->Write(GC.getNumUnitClassInfos(), m_paiUnitMoveChange);
	pStream->Write(GC.getNumUnitClassInfos(), m_paiUnitStrengthModifier);
	pStream->Write(GC.getNumProfessionInfos(), m_paiProfessionCombatChange);
	pStream->Write(GC.getNumProfessionInfos(), m_paiProfessionMoveChange);
	pStream->Write(GC.getNumBuildingClassInfos(), m_paiBuildingClassCount);
	pStream->Write(GC.getNumBuildingClassInfos(), m_paiBuildingClassMaking);
	pStream->Write(GC.getNumHurryInfos(), m_paiHurryCount);
	pStream->Write(GC.getNumSpecialBuildingInfos(), m_paiSpecialBuildingNotRequiredCount);
	pStream->Write(GC.getNumProfessionInfos(), m_aiProfessionEquipmentModifier);
	pStream->Write(GC.getNumTraitInfos(), m_aiTraitCount);

	for (iI=0;iI<GC.getNumCivicOptionInfos();iI++)
	{
		pStream->Write(m_paeCivics[iI]);
	}

	for (iI=0;iI<GC.getNumImprovementInfos();iI++)
	{
		pStream->Write(NUM_YIELD_TYPES, m_ppiImprovementYieldChange[iI]);
	}

	for (iI=0;iI<GC.getNumBuildingClassInfos();iI++)
	{
		pStream->Write(NUM_YIELD_TYPES, m_ppiBuildingYieldChange[iI]);
	}

	m_groupCycle.Write(pStream);
	{
		uint iSize = m_aszCityNames.size();
		pStream->Write(iSize);
		for (uint i = 0; i < iSize; ++i)
		{
			pStream->WriteString(m_aszCityNames[i]);
		}
	}

	WriteStreamableFFreeListTrashArray(m_cities, pStream);
	m_tradeRoutes.Write(pStream);
	m_units.Write(pStream);

	pStream->Write((int)m_aEuropeUnits.size());
	for(int i=0;i<(int)m_aEuropeUnits.size();i++)
	{
		m_aEuropeUnits[i]->write(pStream);
	}

	WriteStreamableFFreeListTrashArray(m_selectionGroups, pStream);
	WriteStreamableFFreeListTrashArray(m_eventsTriggered, pStream);

	{
		CvMessageQueue::_Alloc::size_type iSize = m_listGameMessages.size();
		pStream->Write(iSize);
		CvMessageQueue::iterator it;
		for (it = m_listGameMessages.begin(); it != m_listGameMessages.end(); ++it)
		{
			CvTalkingHeadMessage& message = *it;
			message.write(*pStream);
		}
	}

	{
		CvPopupQueue currentPopups;
		if (GC.getGameINLINE().isNetworkMultiPlayer())
		{
			// don't save open popups in MP to avoid having different state on different machines
			currentPopups.clear();
		}
		else
		{
			gDLL->getInterfaceIFace()->getDisplayedButtonPopups(currentPopups);
		}
		CvPopupQueue::_Alloc::size_type iSize = m_listPopups.size() + currentPopups.size();
		pStream->Write(iSize);
		CvPopupQueue::iterator it;
		for (it = currentPopups.begin(); it != currentPopups.end(); ++it)
		{
			CvPopupInfo* pInfo = *it;
			if (NULL != pInfo)
			{
				pInfo->write(*pStream);
			}
		}
		for (it = m_listPopups.begin(); it != m_listPopups.end(); ++it)
		{
			CvPopupInfo* pInfo = *it;
			if (NULL != pInfo)
			{
				pInfo->write(*pStream);
			}
		}
	}

	{
		CvDiploQueue::_Alloc::size_type iSize = m_listDiplomacy.size();
		pStream->Write(iSize);
		CvDiploQueue::iterator it;
		for (it = m_listDiplomacy.begin(); it != m_listDiplomacy.end(); ++it)
		{
			CvDiploParameters* pDiplo = *it;
			if (NULL != pDiplo)
			{
				pDiplo->write(*pStream);
			}
		}
	}

	{
		uint iSize = m_mapScoreHistory.size();
		pStream->Write(iSize);
		CvTurnScoreMap::iterator it;
		for (it = m_mapScoreHistory.begin(); it != m_mapScoreHistory.end(); ++it)
		{
			pStream->Write((*it).first);
			pStream->Write((*it).second);
		}
	}

	{
		uint iSize = m_mapEconomyHistory.size();
		pStream->Write(iSize);
		CvTurnScoreMap::iterator it;
		for (it = m_mapEconomyHistory.begin(); it != m_mapEconomyHistory.end(); ++it)
		{
			pStream->Write((*it).first);
			pStream->Write((*it).second);
		}
	}

	{
		uint iSize = m_mapIndustryHistory.size();
		pStream->Write(iSize);
		CvTurnScoreMap::iterator it;
		for (it = m_mapIndustryHistory.begin(); it != m_mapIndustryHistory.end(); ++it)
		{
			pStream->Write((*it).first);
			pStream->Write((*it).second);
		}
	}

	{
		uint iSize = m_mapAgricultureHistory.size();
		pStream->Write(iSize);
		CvTurnScoreMap::iterator it;
		for (it = m_mapAgricultureHistory.begin(); it != m_mapAgricultureHistory.end(); ++it)
		{
			pStream->Write((*it).first);
			pStream->Write((*it).second);
		}
	}

	{
		uint iSize = m_mapPowerHistory.size();
		pStream->Write(iSize);
		CvTurnScoreMap::iterator it;
		for (it = m_mapPowerHistory.begin(); it != m_mapPowerHistory.end(); ++it)
		{
			pStream->Write((*it).first);
			pStream->Write((*it).second);
		}
	}

	{
		uint iSize = m_mapCultureHistory.size();
		pStream->Write(iSize);
		CvTurnScoreMap::iterator it;
		for (it = m_mapCultureHistory.begin(); it != m_mapCultureHistory.end(); ++it)
		{
			pStream->Write((*it).first);
			pStream->Write((*it).second);
		}
	}

	{
		uint iSize = m_mapEventsOccured.size();
		pStream->Write(iSize);
		CvEventMap::iterator it;
		for (it = m_mapEventsOccured.begin(); it != m_mapEventsOccured.end(); ++it)
		{
			pStream->Write((*it).first);
			(*it).second.write(pStream);
		}
	}

	{
		uint iSize = m_mapEventCountdown.size();
		pStream->Write(iSize);
		CvEventMap::iterator it;
		for (it = m_mapEventCountdown.begin(); it != m_mapEventCountdown.end(); ++it)
		{
			pStream->Write((*it).first);
			(*it).second.write(pStream);
		}
	}

	{
		uint iSize = m_aFreeUnitCombatPromotions.size();
		pStream->Write(iSize);
		UnitCombatPromotionArray::iterator it;
		for (it = m_aFreeUnitCombatPromotions.begin(); it != m_aFreeUnitCombatPromotions.end(); ++it)
		{
			pStream->Write((*it).first);
			pStream->Write((*it).second);
		}
	}

	{
		uint iSize = m_aFreeUnitClassPromotions.size();
		pStream->Write(iSize);
		UnitClassPromotionArray::iterator it;
		for (it = m_aFreeUnitClassPromotions.begin(); it != m_aFreeUnitClassPromotions.end(); ++it)
		{
			pStream->Write((*it).first);
			pStream->Write((*it).second);
		}
	}

	{
		uint iSize = m_aEuropeRevolutionUnits.size();
		pStream->Write(iSize);
		std::vector< std::pair<UnitTypes, ProfessionTypes> >::iterator it;
		for (it = m_aEuropeRevolutionUnits.begin(); it != m_aEuropeRevolutionUnits.end(); ++it)
		{
			pStream->Write((*it).first);
			pStream->Write((*it).second);
		}
	}

	{
		uint iSize = m_aDocksNextUnits.size();
		pStream->Write(iSize);
		std::vector<UnitTypes>::iterator it;
		for (it = m_aDocksNextUnits.begin(); it != m_aDocksNextUnits.end(); ++it)
		{
			pStream->Write(*it);
		}
	}

	{
		uint iSize = m_triggersFired.size();
		pStream->Write(iSize);
		std::vector<EventTriggerTypes>::iterator it;
		for (it = m_triggersFired.begin(); it != m_triggersFired.end(); ++it)
		{
			pStream->Write((*it));
		}
	}

	pStream->Write(m_iPopRushHurryCount);
	pStream->Write(m_iCrossesStored);
	pStream->Write(m_iBellsStored);
	pStream->Write(m_iTaxRate);
	pStream->Write(m_iNativeCombatModifier);
	pStream->Write(m_iDominateNativeBordersCount);
	pStream->Write(m_iRevolutionEuropeTradeCount);
	pStream->Write(m_iFatherPointMultiplier);
	pStream->Write(m_iMissionaryRateModifier);
}

void CvPlayer::createGreatGeneral(UnitTypes eGreatGeneralUnit, bool bIncrementExperience, int iX, int iY)
{
	CvUnit* pGreatUnit = initUnit(eGreatGeneralUnit, (ProfessionTypes) GC.getUnitInfo(eGreatGeneralUnit).getDefaultProfession(), iX, iY);
	if (NULL == pGreatUnit)
	{
		FAssert(false);
		return;
	}

	if (bIncrementExperience)
	{
		incrementGreatGeneralsCreated();

		changeGreatGeneralsThresholdModifier(GC.getDefineINT("GREAT_GENERALS_THRESHOLD_INCREASE"));

		for (int iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() == getTeam())
			{
				GET_PLAYER((PlayerTypes)iI).changeGreatGeneralsThresholdModifier(GC.getDefineINT("GREAT_GENERALS_THRESHOLD_INCREASE_TEAM"));
			}
		}
	}


	CvPlot* pPlot = GC.getMapINLINE().plot(iX, iY);
	CvCity* pCity = pPlot->getPlotCity();
	CvWString szReplayMessage;

	if (pPlot)
	{
		if (pCity)
		{
			CvWString szCity;
			szCity.Format(L"%s (%s)", pCity->getName().GetCString(), GET_PLAYER(pCity->getOwnerINLINE()).getName());
			szReplayMessage = gDLL->getText("TXT_KEY_MISC_GP_BORN", pGreatUnit->getName().GetCString(), szCity.GetCString());
		}
		else
		{
			szReplayMessage = gDLL->getText("TXT_KEY_MISC_GP_BORN_FIELD", pGreatUnit->getName().GetCString());
		}
		GC.getGameINLINE().addReplayMessage(REPLAY_MESSAGE_MAJOR_EVENT, getID(), szReplayMessage, iX, iY, (ColorTypes)GC.getInfoTypeForString("COLOR_UNIT_TEXT"));
	}

	for (int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (pPlot->isRevealed(GET_PLAYER((PlayerTypes)iI).getTeam(), false))
			{
				gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), false, GC.getEVENT_MESSAGE_TIME(), szReplayMessage, "AS2D_UNIT_GREATPEOPLE", MESSAGE_TYPE_MAJOR_EVENT, pGreatUnit->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_UNIT_TEXT"), iX, iY, true, true);
			}
			else
			{
				CvWString szMessage = gDLL->getText("TXT_KEY_MISC_GP_BORN_SOMEWHERE", pGreatUnit->getName().GetCString());
				gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), false, GC.getEVENT_MESSAGE_TIME(), szMessage, "AS2D_UNIT_GREATPEOPLE", MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_UNIT_TEXT"));
			}
		}
	}
}


const EventTriggeredData* CvPlayer::getEventOccured(EventTypes eEvent) const
{
	FAssert(eEvent >= 0 && eEvent < GC.getNumEventInfos());

	CvEventMap::const_iterator it = m_mapEventsOccured.find(eEvent);

	if (it == m_mapEventsOccured.end())
	{
		return NULL;
	}

	return &((*it).second);
}

bool CvPlayer::isTriggerFired(EventTriggerTypes eEventTrigger) const
{
	return (std::find(m_triggersFired.begin(), m_triggersFired.end(), eEventTrigger) != m_triggersFired.end());
}

void CvPlayer::resetEventOccured(EventTypes eEvent, bool bAnnounce)
{
	FAssert(eEvent >= 0 && eEvent < GC.getNumEventInfos());

	CvEventMap::iterator it = m_mapEventsOccured.find(eEvent);

	if (it != m_mapEventsOccured.end())
	{
		expireEvent((*it).first, (*it).second, bAnnounce);
		m_mapEventsOccured.erase(it);
	}
}

void CvPlayer::setEventOccured(EventTypes eEvent, const EventTriggeredData& kEventTriggered, bool bOthers)
{
	FAssert(eEvent >= 0 && eEvent < GC.getNumEventInfos());

	m_mapEventsOccured[eEvent] = kEventTriggered;

	if (GC.getEventInfo(eEvent).isQuest())
	{
		CvWStringBuffer szMessageBuffer;
		szMessageBuffer.append(GC.getEventInfo(eEvent).getDescription());
		GAMETEXT.setEventHelp(szMessageBuffer, eEvent, kEventTriggered.getID(), getID());
		gDLL->getInterfaceIFace()->addQuestMessage(getID(), szMessageBuffer.getCString(), kEventTriggered.getID());
	}

	if (bOthers)
	{
		if (GC.getEventInfo(eEvent).isGlobal())
		{
			for (int i = 0; i < MAX_PLAYERS; i++)
			{
				if (i != getID())
				{
					GET_PLAYER((PlayerTypes)i).setEventOccured(eEvent, kEventTriggered, false);
				}
			}
		}
		else if (GC.getEventInfo(eEvent).isTeam())
		{
			for (int i = 0; i < MAX_PLAYERS; i++)
			{
				if (i != getID() && getTeam() == GET_PLAYER((PlayerTypes)i).getTeam())
				{
					GET_PLAYER((PlayerTypes)i).setEventOccured(eEvent, kEventTriggered, false);
				}
			}
		}
	}
}


const EventTriggeredData* CvPlayer::getEventCountdown(EventTypes eEvent) const
{
	FAssert(eEvent >= 0 && eEvent < GC.getNumEventInfos());

	CvEventMap::const_iterator it = m_mapEventCountdown.find(eEvent);

	if (it == m_mapEventCountdown.end())
	{
		return NULL;
	}

	return &((*it).second);
}

void CvPlayer::setEventCountdown(EventTypes eEvent, const EventTriggeredData& kEventTriggered)
{
	FAssert(eEvent >= 0 && eEvent < GC.getNumEventInfos());

	m_mapEventCountdown[eEvent] = kEventTriggered;
}

void CvPlayer::resetEventCountdown(EventTypes eEvent)
{
	FAssert(eEvent >= 0 && eEvent < GC.getNumEventInfos());

	CvEventMap::iterator it = m_mapEventCountdown.find(eEvent);

	if (it != m_mapEventCountdown.end())
	{
		m_mapEventCountdown.erase(it);
	}
}


void CvPlayer::resetTriggerFired(EventTriggerTypes eTrigger)
{
	std::vector<EventTriggerTypes>::iterator it = std::find(m_triggersFired.begin(), m_triggersFired.end(), eTrigger);

	if (it != m_triggersFired.end())
	{
		m_triggersFired.erase(it);
	}
}

void CvPlayer::setTriggerFired(const EventTriggeredData& kTriggeredData, bool bOthers, bool bAnnounce)
{
	FAssert(kTriggeredData.m_eTrigger >= 0 && kTriggeredData.m_eTrigger < GC.getNumEventTriggerInfos());

	CvEventTriggerInfo& kTrigger = GC.getEventTriggerInfo(kTriggeredData.m_eTrigger);

	if (!isTriggerFired(kTriggeredData.m_eTrigger))
	{
		m_triggersFired.push_back(kTriggeredData.m_eTrigger);

		if (bOthers)
		{
			if (kTrigger.isGlobal())
			{
				for (int i = 0; i < MAX_PLAYERS; i++)
				{
					if (i != getID())
					{
						GET_PLAYER((PlayerTypes)i).setTriggerFired(kTriggeredData, false, false);
					}
				}
			}
			else if (kTrigger.isTeam())
			{
				for (int i = 0; i < MAX_PLAYERS; i++)
				{
					if (i != getID() && getTeam() == GET_PLAYER((PlayerTypes)i).getTeam())
					{
						GET_PLAYER((PlayerTypes)i).setTriggerFired(kTriggeredData, false, false);
					}
				}
			}
		}
	}

	if (!isEmpty(kTrigger.getPythonCallback()))
	{
		long lResult;

		CyArgsList argsList;
		argsList.add(gDLL->getPythonIFace()->makePythonObject(&kTriggeredData));

		gDLL->getPythonIFace()->callFunction(PYRandomEventModule, kTrigger.getPythonCallback(), argsList.makeFunctionArgs(), &lResult);
	}

	if (bAnnounce && !kTrigger.isTutorial())
	{
		CvPlot* pPlot = GC.getMapINLINE().plot(kTriggeredData.m_iPlotX, kTriggeredData.m_iPlotY);

		if (!kTriggeredData.m_szGlobalText.empty())
		{
			for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
			{
				CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);

				if (kLoopPlayer.isAlive())
				{
					if (GET_TEAM(kLoopPlayer.getTeam()).isHasMet(getTeam()) && (NO_PLAYER == kTriggeredData.m_eOtherPlayer || GET_TEAM(GET_PLAYER(kTriggeredData.m_eOtherPlayer).getTeam()).isHasMet(getTeam())))
					{
						bool bShowPlot = kTrigger.isShowPlot();

						if (bShowPlot)
						{
							if (kLoopPlayer.getTeam() != getTeam())
							{
								if (NULL == pPlot || !pPlot->isRevealed(kLoopPlayer.getTeam(), false))
								{
									bShowPlot = false;
								}
							}
						}

						if (bShowPlot)
						{
							gDLL->getInterfaceIFace()->addMessage((PlayerTypes)iPlayer, false, GC.getEVENT_MESSAGE_TIME(), kTriggeredData.m_szGlobalText, "AS2D_CIVIC_ADOPT", MESSAGE_TYPE_MINOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_WHITE"), kTriggeredData.m_iPlotX, kTriggeredData.m_iPlotY, true, true);
						}
						else
						{
							gDLL->getInterfaceIFace()->addMessage((PlayerTypes)iPlayer, false, GC.getEVENT_MESSAGE_TIME(), kTriggeredData.m_szGlobalText, "AS2D_CIVIC_ADOPT", MESSAGE_TYPE_MINOR_EVENT);
						}
					}
				}
			}

			GC.getGameINLINE().addReplayMessage(REPLAY_MESSAGE_MAJOR_EVENT, getID(), kTriggeredData.m_szGlobalText, kTriggeredData.m_iPlotX, kTriggeredData.m_iPlotY, (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"));
		}
		else if (!kTriggeredData.m_szText.empty())
		{
			if (kTrigger.isShowPlot() && NULL != pPlot && pPlot->isRevealed(getTeam(), false))
			{
				gDLL->getInterfaceIFace()->addMessage(getID(), false, GC.getEVENT_MESSAGE_TIME(), kTriggeredData.m_szText, "AS2D_CIVIC_ADOPT", MESSAGE_TYPE_MINOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_WHITE"), kTriggeredData.m_iPlotX, kTriggeredData.m_iPlotY, true, true);
			}
			else
			{
				gDLL->getInterfaceIFace()->addMessage(getID(), false, GC.getEVENT_MESSAGE_TIME(), kTriggeredData.m_szText, "AS2D_CIVIC_ADOPT", MESSAGE_TYPE_MINOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_WHITE"));
			}
		}
	}

	if (kTrigger.isTutorial())
	{
		gDLL->getInterfaceIFace()->addTutorialMessage(getID(), kTriggeredData.m_szText);
	}
}

EventTriggeredData* CvPlayer::initTriggeredData(EventTriggerTypes eEventTrigger, bool bFire, int iCityId, int iPlotX, int iPlotY, PlayerTypes eOtherPlayer, int iOtherPlayerCityId, int iUnitId, BuildingTypes eBuilding)
{

	CvEventTriggerInfo& kTrigger = GC.getEventTriggerInfo(eEventTrigger);

	CvCity* pCity = getCity(iCityId);
	CvCity* pOtherPlayerCity = NULL;
	if (NO_PLAYER != eOtherPlayer)
	{
		pOtherPlayerCity = GET_PLAYER(eOtherPlayer).getCity(iOtherPlayerCityId);
	}
	CvPlot* pPlot = GC.getMapINLINE().plot(iPlotX, iPlotY);
	CvUnit* pUnit = getUnit(iUnitId);

	std::vector<CvPlot*> apPlots;
	bool bPickPlot = ::isPlotEventTrigger(eEventTrigger);
	if (kTrigger.isPickCity())
	{
		if (NULL == pCity)
		{
			pCity = pickTriggerCity(eEventTrigger);
		}

		if (NULL != pCity)
		{
			if (bPickPlot)
			{
				for (int iPlot = 0; iPlot < NUM_CITY_PLOTS; ++iPlot)
				{
					if (CITY_HOME_PLOT != iPlot)
					{
						CvPlot* pLoopPlot = pCity->getCityIndexPlot(iPlot);

						if (NULL != pLoopPlot)
						{
							if (pLoopPlot->canTrigger(eEventTrigger, getID()))
							{
								apPlots.push_back(pLoopPlot);
							}
						}
					}
				}
			}
		}
		else
		{
			return NULL;
		}
	}
	else 
	{
		if (kTrigger.getNumBuildings() > 0 && kTrigger.getNumBuildingsRequired() > 0)
		{
			int iFoundValid = 0;

			for (int i = 0; i < kTrigger.getNumBuildingsRequired(); ++i)
			{
				if (kTrigger.getBuildingRequired(i) != NO_BUILDINGCLASS)
				{
					iFoundValid += getBuildingClassCount((BuildingClassTypes)kTrigger.getBuildingRequired(i));
				}
			}

			if (iFoundValid < kTrigger.getNumBuildings())
			{
				return NULL;
			}
		}

		if (kTrigger.getMinPopulation() > 0)
		{
			if (getTotalPopulation() < kTrigger.getMinPopulation())
			{
				return NULL;
			}
		}

		if (kTrigger.getMaxPopulation() > 0)
		{
			if (getTotalPopulation() > kTrigger.getMaxPopulation())
			{
				return NULL;
			}
		}

		if (bPickPlot)
		{
			for (int iPlot = 0; iPlot < GC.getMapINLINE().numPlotsINLINE(); ++iPlot)
			{
				CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iPlot);

				if (pLoopPlot->canTrigger(eEventTrigger, getID()))
				{
					apPlots.push_back(pLoopPlot);
				}
			}
		}
	}

	if (NULL == pPlot)
	{
		if (apPlots.size() > 0)
		{
			int iChosen = GC.getGameINLINE().getSorenRandNum(apPlots.size(), "Event pick plot");
			pPlot = apPlots[iChosen];

			if (NULL == pCity)
			{
				pCity = GC.getMapINLINE().findCity(pPlot->getX_INLINE(), pPlot->getY_INLINE(), getID(), NO_TEAM, false);
			}
		}
		else 
		{
			if (bPickPlot)
			{
				return NULL;
			}

			if (NULL != pCity)
			{
				pPlot = pCity->plot();
			}
		}
	}

	if (kTrigger.getNumBuildings() > 0)
	{
		if (NULL != pCity && NO_BUILDING == eBuilding)
		{
			std::vector<BuildingTypes> aeBuildings;
			for (int i = 0; i < kTrigger.getNumBuildingsRequired(); ++i)
			{
				if (kTrigger.getBuildingRequired(i) != NO_BUILDINGCLASS)
				{
					BuildingTypes eTestBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(kTrigger.getBuildingRequired(i));
					if (NO_BUILDING != eTestBuilding && pCity->isHasRealBuilding(eTestBuilding))
					{
						aeBuildings.push_back(eTestBuilding);
					}
				}
			}

			if (aeBuildings.size() > 0)
			{
				int iChosen = GC.getGameINLINE().getSorenRandNum(aeBuildings.size(), "Event pick building");
				eBuilding = aeBuildings[iChosen];
			}
			else
			{
				return NULL;
			}
		}
	}	

	if (NULL == pUnit)
	{
		pUnit = pickTriggerUnit(eEventTrigger, pPlot, bPickPlot);
	}

	if (NULL == pUnit && kTrigger.getNumUnits() > 0)
	{
		return NULL;
	}

	if (NULL == pPlot && NULL != pUnit)
	{
		pPlot = pUnit->plot();
	}

	if (NULL == pPlot && bPickPlot)
	{
		return NULL;
	}

	if (kTrigger.getNumUnitsGlobal() > 0)
	{
		int iNumUnits = 0;
		for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
		{
			CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);

			if (kLoopPlayer.isAlive())
			{
				int iLoop;
				for (CvUnit* pLoopUnit = kLoopPlayer.firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = kLoopPlayer.nextUnit(&iLoop))
				{
					if (MIN_INT != pLoopUnit->getTriggerValue(eEventTrigger, pPlot, true))
					{
						++iNumUnits;
					}
				}
			}
		}

		if (iNumUnits < kTrigger.getNumUnitsGlobal())
		{
			return NULL;
		}
	}

	if (kTrigger.getNumBuildingsGlobal() > 0)
	{
		int iNumBuildings = 0;
		for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
		{
			CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);

			if (kLoopPlayer.isAlive())
			{
				for (int i = 0; i < kTrigger.getNumBuildingsRequired(); ++i)
				{
					if (kTrigger.getBuildingRequired(i) != NO_BUILDINGCLASS)
					{
						iNumBuildings += getBuildingClassCount((BuildingClassTypes)kTrigger.getBuildingRequired(i));
					}
				}
			}
		}

		if (iNumBuildings < kTrigger.getNumBuildingsGlobal())
		{
			return NULL;
		}
	}

	if (kTrigger.isPickPlayer())
	{
		std::vector<PlayerTypes> aePlayers;
		std::vector<CvCity*> apCities;

		if (NO_PLAYER == eOtherPlayer)
		{
			for (int i = 0; i < MAX_PLAYERS; i++)
			{
				if (GET_PLAYER((PlayerTypes)i).canTrigger(eEventTrigger, getID()))
				{
					if (kTrigger.isPickOtherPlayerCity())
					{
						CvCity* pBestCity = NULL;

						if (NULL != pCity)
						{
							pBestCity = GC.getMapINLINE().findCity(pCity->getX_INLINE(), pCity->getY_INLINE(), (PlayerTypes)i);
						}
						else
						{
							pBestCity = GET_PLAYER((PlayerTypes)i).pickTriggerCity(eEventTrigger);
						}

						if (NULL != pBestCity)
						{
							apCities.push_back(pBestCity);
							aePlayers.push_back((PlayerTypes)i);
						}
					}
					else
					{
						apCities.push_back(NULL);
						aePlayers.push_back((PlayerTypes)i);
					}
				}
			}

			if (aePlayers.size() > 0)
			{
				int iChosen = GC.getGameINLINE().getSorenRandNum(aePlayers.size(), "Event pick player");
				eOtherPlayer = aePlayers[iChosen];
				pOtherPlayerCity = apCities[iChosen];
			}
			else
			{
				return NULL;
			}
		}
	}	

	EventTriggeredData* pTriggerData = addEventTriggered();

	if (NULL != pTriggerData)
	{
		pTriggerData->m_eTrigger = eEventTrigger;
		pTriggerData->m_ePlayer = getID();
		pTriggerData->m_iTurn = GC.getGameINLINE().getGameTurn();
		pTriggerData->m_iCityId = (NULL != pCity) ? pCity->getID() : -1;
		pTriggerData->m_iPlotX = (NULL != pPlot) ? pPlot->getX_INLINE() : INVALID_PLOT_COORD;
		pTriggerData->m_iPlotY = (NULL != pPlot) ? pPlot->getY_INLINE() : INVALID_PLOT_COORD;
		pTriggerData->m_eOtherPlayer = eOtherPlayer;
		pTriggerData->m_iOtherPlayerCityId = (NULL != pOtherPlayerCity) ? pOtherPlayerCity->getID() : -1;
		pTriggerData->m_iUnitId = (NULL != pUnit) ? pUnit->getID() : -1;
		pTriggerData->m_eBuilding = eBuilding;
	}
	else
	{
		return NULL;
	}

	if (!isEmpty(kTrigger.getPythonCanDo()))
	{
		long lResult;

		CyArgsList argsList;
		argsList.add(gDLL->getPythonIFace()->makePythonObject(pTriggerData));

		gDLL->getPythonIFace()->callFunction(PYRandomEventModule, kTrigger.getPythonCanDo(), argsList.makeFunctionArgs(), &lResult);

		if (0 == lResult)
		{
			deleteEventTriggered(pTriggerData->getID());
			return NULL;
		}

		// python may change pTriggerData
		pCity = getCity(pTriggerData->m_iCityId);
		pPlot = GC.getMapINLINE().plot(pTriggerData->m_iPlotX, pTriggerData->m_iPlotY);
		pUnit = getUnit(pTriggerData->m_iUnitId);
		eOtherPlayer = pTriggerData->m_eOtherPlayer;
		if (NO_PLAYER != eOtherPlayer)
		{
			pOtherPlayerCity = GET_PLAYER(eOtherPlayer).getCity(pTriggerData->m_iOtherPlayerCityId);
		}
		eBuilding = pTriggerData->m_eBuilding;
	}

	std::vector<CvWString> aszTexts;
	for (int i = 0; i < kTrigger.getNumTexts(); ++i)
	{
		if (NO_ERA == kTrigger.getTextEra(i) || kTrigger.getTextEra(i) == getCurrentEra())
		{
			aszTexts.push_back(kTrigger.getText(i));
		}
	}

	if (aszTexts.size() > 0)
	{
		int iText = GC.getGameINLINE().getSorenRandNum(aszTexts.size(), "Event Text choice");
		pTriggerData->m_szText = gDLL->getText(aszTexts[iText].GetCString(),
			eOtherPlayer != NO_PLAYER ? GET_PLAYER(eOtherPlayer).getCivilizationAdjectiveKey() : L"",
			NULL != pCity ? pCity->getNameKey() : L"",
			NULL != pUnit ? pUnit->getNameOrProfessionKey() : L"",
			L"",
			NO_BUILDING != eBuilding ? GC.getBuildingInfo(eBuilding).getTextKeyWide() : L"",
			NULL != pOtherPlayerCity ? pOtherPlayerCity->getNameKey() : L"",
			NULL != pPlot && NO_TERRAIN != pPlot->getTerrainType() ? GC.getTerrainInfo(pPlot->getTerrainType()).getTextKeyWide() : L"",
			NULL != pPlot && NO_IMPROVEMENT != pPlot->getImprovementType() ? GC.getImprovementInfo(pPlot->getImprovementType()).getTextKeyWide() : L"",
			NULL != pPlot && NO_BONUS != pPlot->getBonusType() ? GC.getBonusInfo(pPlot->getBonusType()).getTextKeyWide() : L"",
			NULL != pPlot && NO_ROUTE != pPlot->getRouteType() ? GC.getRouteInfo(pPlot->getRouteType()).getTextKeyWide() : L""
			);

	}
	else
	{
		pTriggerData->m_szText = L"";
	}

	if (kTrigger.getNumWorldNews() > 0)
	{
		int iText = GC.getGameINLINE().getSorenRandNum(kTrigger.getNumWorldNews(), "Trigger World News choice");

		pTriggerData->m_szGlobalText = gDLL->getText(kTrigger.getWorldNews(iText).GetCString(),
			getCivilizationAdjectiveKey(),
			NULL != pCity ? pCity->getNameKey() : L"",
			L"",
			eOtherPlayer != NO_PLAYER ? GET_PLAYER(eOtherPlayer).getCivilizationAdjectiveKey() : L"",
			NULL != pOtherPlayerCity ? pOtherPlayerCity->getNameKey() : L""
			);
	}
	else
	{
		pTriggerData->m_szGlobalText.clear();
	}

	if (bFire)
	{
		trigger(*pTriggerData);
	}

	return pTriggerData;
}


bool CvPlayer::canDoEvent(EventTypes eEvent, const EventTriggeredData& kTriggeredData) const
{
	if (eEvent == NO_EVENT)
	{
		FAssert(false);
		return false;
	}

	CvEventInfo& kEvent = GC.getEventInfo(eEvent);

	int iGold = std::min(getEventCost(eEvent, kTriggeredData.m_eOtherPlayer, false), getEventCost(eEvent, kTriggeredData.m_eOtherPlayer, true));

	if (iGold != 0)
	{
		if (iGold > 0 && NO_PLAYER != kTriggeredData.m_eOtherPlayer && kEvent.isGoldToPlayer())
		{
			if (GET_PLAYER(kTriggeredData.m_eOtherPlayer).getGold() < iGold)
			{
				return false;
			}
		}
		else if (iGold < 0)
		{
			if (getGold() < -iGold)
			{
				return false;
			}
		}
	}

	if (kEvent.getUnitClass() != NO_UNITCLASS)
	{
		UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(kEvent.getUnitClass());
		if (eUnit == NO_UNIT)
		{
			return false;
		}
	}

	if (kEvent.isCityEffect())
	{
		CvCity* pCity =	getCity(kTriggeredData.m_iCityId);
		if (NULL == pCity || !pCity->canApplyEvent(eEvent, kTriggeredData))
		{
			return false;
		}
	}
	else if (kEvent.isOtherPlayerCityEffect())
	{
		if (NO_PLAYER == kTriggeredData.m_eOtherPlayer)
		{
			return false;
		}

		CvCity* pCity = GET_PLAYER(kTriggeredData.m_eOtherPlayer).getCity(kTriggeredData.m_iOtherPlayerCityId);
		if (NULL == pCity || !pCity->canApplyEvent(eEvent, kTriggeredData))
		{
			return false;
		}
	}

	if (::isPlotEventTrigger(kTriggeredData.m_eTrigger))
	{
		CvPlot* pPlot = GC.getMapINLINE().plotINLINE(kTriggeredData.m_iPlotX, kTriggeredData.m_iPlotY);
		if (NULL != pPlot)
		{
			if (!pPlot->canApplyEvent(eEvent))
			{
				return false;
			}
		}
	}

	CvUnit* pUnit = getUnit(kTriggeredData.m_iUnitId);
	if (NULL != pUnit)
	{
		if (!pUnit->canApplyEvent(eEvent))
		{
			return false;
		}
	}

	if (NO_BONUS != kEvent.getBonusRevealed())
	{
		if (GET_TEAM(getTeam()).isForceRevealedBonus((BonusTypes)kEvent.getBonusRevealed()))
		{
			return false;
		}
	}

	if (0 != kEvent.getAttitudeModifier())
	{
		if (NO_PLAYER == kTriggeredData.m_eOtherPlayer)
		{
			return false;
		}

		if (GET_PLAYER(kTriggeredData.m_eOtherPlayer).getTeam() == getTeam())
		{
			return false;
		}

		if (GET_PLAYER(kTriggeredData.m_eOtherPlayer).isHuman())
		{
			if (0 == kEvent.getOurAttitudeModifier())
			{
				return false;
			}
		}
	}

	if (0 != kEvent.getTheirEnemyAttitudeModifier())
	{
		if (NO_PLAYER == kTriggeredData.m_eOtherPlayer)
		{
			return false;
		}

		TeamTypes eWorstEnemy = GET_TEAM(GET_PLAYER(kTriggeredData.m_eOtherPlayer).getTeam()).AI_getWorstEnemy();
		if (NO_TEAM == eWorstEnemy || eWorstEnemy == getTeam())
		{
			return false;
		}

		if (!GET_TEAM(eWorstEnemy).isAlive())
		{
			return false;
		}

		if (eWorstEnemy == getTeam())
		{
			return false;
		}
	}

	if (kEvent.isDeclareWar())
	{
		if (NO_PLAYER == kTriggeredData.m_eOtherPlayer)
		{
			return false;
		}

		if (!GET_TEAM(GET_PLAYER(kTriggeredData.m_eOtherPlayer).getTeam()).canDeclareWar(getTeam()) || !GET_TEAM(getTeam()).canDeclareWar(GET_PLAYER(kTriggeredData.m_eOtherPlayer).getTeam()))
		{
			return false;
		}
	}

	if (kEvent.isQuest())
	{
		for (int iTrigger = 0; iTrigger < GC.getNumEventTriggerInfos(); ++iTrigger)
		{
			CvEventTriggerInfo& kTrigger = GC.getEventTriggerInfo((EventTriggerTypes)iTrigger);
			if (!kTrigger.isRecurring())
			{
				for (int i = 0; i < kTrigger.getNumPrereqEvents(); ++i)
				{
					if (kTrigger.getPrereqEvent(i) == eEvent)
					{
						if (isTriggerFired((EventTriggerTypes)iTrigger))
						{
							return false;
						}
					}
				}
			}
		}
	}

	if (!isEmpty(kEvent.getPythonCanDo()))
	{
		long lResult;

		CyArgsList argsList;
		argsList.add(eEvent);
		argsList.add(gDLL->getPythonIFace()->makePythonObject(&kTriggeredData));

		gDLL->getPythonIFace()->callFunction(PYRandomEventModule, kEvent.getPythonCanDo(), argsList.makeFunctionArgs(), &lResult);

		if (0 == lResult)
		{
			return false;
		}
	}

	return true;
}


void CvPlayer::applyEvent(EventTypes eEvent, int iEventTriggeredId, bool bUpdateTrigger)
{
	FAssert(eEvent != NO_EVENT);

	EventTriggeredData* pTriggeredData = getEventTriggered(iEventTriggeredId);

	if (NULL == pTriggeredData)
	{
		deleteEventTriggered(iEventTriggeredId);
		return;
	}

	if (bUpdateTrigger)
	{
		setTriggerFired(*pTriggeredData, true);
	}

	if (!canDoEvent(eEvent, *pTriggeredData))
	{
		if (bUpdateTrigger)
		{
			deleteEventTriggered(iEventTriggeredId);
		}
		return;
	}

	setEventOccured(eEvent, *pTriggeredData);

	CvEventInfo& kEvent = GC.getEventInfo(eEvent);
	CvCity* pCity =	getCity(pTriggeredData->m_iCityId);
	CvCity* pOtherPlayerCity = NULL;

	if (NO_PLAYER != pTriggeredData->m_eOtherPlayer)
	{
		pOtherPlayerCity = GET_PLAYER(pTriggeredData->m_eOtherPlayer).getCity(pTriggeredData->m_iOtherPlayerCityId);
	}

	int iGold = getEventCost(eEvent, pTriggeredData->m_eOtherPlayer, false);
	int iRandomGold = getEventCost(eEvent, pTriggeredData->m_eOtherPlayer, true);

	iGold += GC.getGameINLINE().getSorenRandNum(iRandomGold - iGold + 1, "Event random gold");

	if (iGold != 0)
	{
		changeGold(iGold);

		if (NO_PLAYER != pTriggeredData->m_eOtherPlayer && kEvent.isGoldToPlayer())
		{
			GET_PLAYER(pTriggeredData->m_eOtherPlayer).changeGold(-iGold);
		}
	}

	if (kEvent.isDeclareWar())
	{
		if (NO_PLAYER != pTriggeredData->m_eOtherPlayer)
		{
			GET_TEAM(GET_PLAYER(pTriggeredData->m_eOtherPlayer).getTeam()).declareWar(getTeam(), false, WARPLAN_LIMITED);
		}
	}

	bool bClear = false;
	for (int iEvent = 0; iEvent < GC.getNumEventInfos(); ++iEvent)
	{
		if (kEvent.getClearEventChance(iEvent) > 0)
		{
			bClear = GC.getGameINLINE().getSorenRandNum(100, "Event Clear") < kEvent.getClearEventChance(iEvent);
			if (bClear)
			{
				if (kEvent.isGlobal())
				{
					for (int j = 0; j < MAX_PLAYERS; j++)
					{
						GET_PLAYER((PlayerTypes)j).resetEventOccured((EventTypes)iEvent, j != getID());
					}
				}
				else if (kEvent.isTeam())
				{
					for (int j = 0; j < MAX_PLAYERS; j++)
					{
						if (getTeam() == GET_PLAYER((PlayerTypes)j).getTeam())
						{
							GET_PLAYER((PlayerTypes)j).resetEventOccured((EventTypes)iEvent, j != getID());
						}
					}
				}
				else
				{
					resetEventOccured((EventTypes)iEvent, false);
				}
			}
		}
	}

	if (NULL != pCity && kEvent.isCityEffect())
	{
		pCity->applyEvent(eEvent, *pTriggeredData, bClear);
	}
	else if (NULL != pOtherPlayerCity && kEvent.isOtherPlayerCityEffect())
	{
		pOtherPlayerCity->applyEvent(eEvent, *pTriggeredData, bClear);
	}

	if (!kEvent.isCityEffect() && !kEvent.isOtherPlayerCityEffect())
	{
		if (kEvent.getNumBuildingYieldChanges() > 0)
		{
			int iLoop;
			for (int iBuildingClass = 0; iBuildingClass < GC.getNumBuildingClassInfos(); ++iBuildingClass)
			{
				for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
				{
					for (CvCity* pLoopCity = firstCity(&iLoop); NULL != pLoopCity; pLoopCity = nextCity(&iLoop))
					{
						pLoopCity->changeBuildingYieldChange((BuildingClassTypes)iBuildingClass, (YieldTypes)iYield, kEvent.getBuildingYieldChange(iBuildingClass, iYield));
					}					
				}
			}
		}

		if (kEvent.getMaxPillage() > 0)
		{
			FAssert(kEvent.getMaxPillage() >= kEvent.getMinPillage());
			int iNumPillage = kEvent.getMinPillage() + GC.getGameINLINE().getSorenRandNum(kEvent.getMaxPillage() - kEvent.getMinPillage(), "Pick number of event pillaged plots");

			int iNumPillaged = 0;
			for (int i = 0; i < iNumPillage; ++i)
			{
				int iRandOffset = GC.getGameINLINE().getSorenRandNum(GC.getMapINLINE().numPlotsINLINE(), "Pick event pillage plot (any city)");
				for (int j = 0; j < GC.getMapINLINE().numPlotsINLINE(); ++j)
				{
					int iPlot = (j + iRandOffset) % GC.getMapINLINE().numPlotsINLINE();
					CvPlot* pPlot = GC.getMapINLINE().plotByIndexINLINE(iPlot);
					if (NULL != pPlot && pPlot->getOwnerINLINE() == getID() && pPlot->isCity())
					{
						if (NO_IMPROVEMENT != pPlot->getImprovementType() && !GC.getImprovementInfo(pPlot->getImprovementType()).isPermanent())
						{
							CvWString szBuffer = gDLL->getText("TXT_KEY_EVENT_CITY_IMPROVEMENT_DESTROYED", GC.getImprovementInfo(pPlot->getImprovementType()).getTextKeyWide());
							gDLL->getInterfaceIFace()->addMessage(getID(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_PILLAGED", MESSAGE_TYPE_INFO, GC.getImprovementInfo(pPlot->getImprovementType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
							pPlot->setImprovementType(NO_IMPROVEMENT);
							++iNumPillaged;
							break;
						}
					}
				}
			}

			if (NO_PLAYER != pTriggeredData->m_eOtherPlayer)
			{
				CvWString szBuffer = gDLL->getText("TXT_KEY_EVENT_NUM_CITY_IMPROVEMENTS_DESTROYED", iNumPillaged, getCivilizationAdjectiveKey());
				gDLL->getInterfaceIFace()->addMessage(pTriggeredData->m_eOtherPlayer, false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_PILLAGED", MESSAGE_TYPE_INFO);
			}
		}

		if (kEvent.getFood() != 0)
		{
			int iLoop;

			for (CvCity* pLoopCity = firstCity(&iLoop); NULL != pLoopCity; pLoopCity = nextCity(&iLoop))
			{
				pLoopCity->changeFood(kEvent.getFood());
			}
		}

		if (kEvent.getFoodPercent() != 0)
		{
			int iLoop;

			for (CvCity* pLoopCity = firstCity(&iLoop); NULL != pLoopCity; pLoopCity = nextCity(&iLoop))
			{
				pLoopCity->changeFood((pLoopCity->getFood() * kEvent.getFoodPercent()) / 100);
			}
		}

		if (kEvent.getPopulationChange() != 0)
		{
			int iLoop;

			for (CvCity* pLoopCity = firstCity(&iLoop); NULL != pLoopCity; pLoopCity = nextCity(&iLoop))
			{
				if (pLoopCity->getPopulation() + kEvent.getPopulationChange() > 0)
				{
					pLoopCity->changePopulation(kEvent.getPopulationChange());
				}
			}
		}

		if (kEvent.getCulture() != 0)
		{
			int iLoop;

			for (CvCity* pLoopCity = firstCity(&iLoop); NULL != pLoopCity; pLoopCity = nextCity(&iLoop))
			{
				if (pLoopCity->getCulture(pLoopCity->getOwnerINLINE()) + kEvent.getCulture() > 0)
				{
					pLoopCity->changeCulture(pLoopCity->getOwnerINLINE(), kEvent.getCulture(), true);
				}
			}
		}

		if (kEvent.getUnitClass() != NO_UNITCLASS)
		{
			UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(kEvent.getUnitClass());
			if (eUnit != NO_UNIT)
			{
				CvCity* pUnitCity = pCity;

				if (NULL == pUnitCity)
				{
					pUnitCity = getPrimaryCity();
				}

				if (NULL != pUnitCity)
				{
					for (int i = 0; i < kEvent.getNumUnits(); ++i)
					{
						initUnit(eUnit, (ProfessionTypes) GC.getUnitInfo(eUnit).getDefaultProfession(), pUnitCity->getX_INLINE(), pUnitCity->getY_INLINE());
					}
				}
			}
		}
	}

	CvPlot* pPlot = GC.getMapINLINE().plotINLINE(pTriggeredData->m_iPlotX, pTriggeredData->m_iPlotY);
	if (NULL != pPlot)
	{
		if (::isPlotEventTrigger(pTriggeredData->m_eTrigger))
		{
			FAssert(pPlot->canApplyEvent(eEvent));
			pPlot->applyEvent(eEvent);
		}
	}

	CvUnit* pUnit = getUnit(pTriggeredData->m_iUnitId);
	if (NULL != pUnit)
	{
		FAssert(pUnit->canApplyEvent(eEvent));
		pUnit->applyEvent(eEvent);   // might kill the unit
	}

	for (int i = 0; i < GC.getNumUnitCombatInfos(); ++i)
	{
		if (NO_PROMOTION != kEvent.getUnitCombatPromotion(i))
		{
			int iLoop;
			for (CvUnit* pLoopUnit = firstUnit(&iLoop); NULL != pLoopUnit; pLoopUnit = nextUnit(&iLoop))
			{
				if (pLoopUnit->getUnitCombatType() == i)
				{
					pLoopUnit->changeFreePromotionCount((PromotionTypes)kEvent.getUnitCombatPromotion(i), 1);
				}
			}

			setFreePromotion((UnitCombatTypes)i, (PromotionTypes)kEvent.getUnitCombatPromotion(i), true);
		}
	}

	for (int i = 0; i < GC.getNumUnitClassInfos(); ++i)
	{
		if (NO_PROMOTION != kEvent.getUnitClassPromotion(i))
		{
			int iLoop;
			for (CvUnit* pLoopUnit = firstUnit(&iLoop); NULL != pLoopUnit; pLoopUnit = nextUnit(&iLoop))
			{
				if (pLoopUnit->getUnitClassType() == i)
				{
					pLoopUnit->changeFreePromotionCount((PromotionTypes)kEvent.getUnitClassPromotion(i), 1);
				}
			}

			setFreePromotion((UnitClassTypes)i, (PromotionTypes)kEvent.getUnitClassPromotion(i), true);
		}
	}

	if (NO_BONUS != kEvent.getBonusRevealed())
	{
		GET_TEAM(getTeam()).setForceRevealedBonus((BonusTypes)kEvent.getBonusRevealed(), true);
	}

	if (0 != kEvent.getOurAttitudeModifier())
	{
		if (NO_PLAYER != pTriggeredData->m_eOtherPlayer)
		{
			if (kEvent.getOurAttitudeModifier() > 0)
			{
				AI_changeMemoryCount(pTriggeredData->m_eOtherPlayer, MEMORY_EVENT_GOOD_TO_US, kEvent.getOurAttitudeModifier());
			}
			else
			{
				AI_changeMemoryCount(pTriggeredData->m_eOtherPlayer, MEMORY_EVENT_BAD_TO_US, -kEvent.getOurAttitudeModifier());
			}
		}
	}

	if (0 != kEvent.getAttitudeModifier())
	{
		if (NO_PLAYER != pTriggeredData->m_eOtherPlayer)
		{
			if (kEvent.getAttitudeModifier() > 0)
			{
				GET_PLAYER(pTriggeredData->m_eOtherPlayer).AI_changeMemoryCount(getID(), MEMORY_EVENT_GOOD_TO_US, kEvent.getAttitudeModifier());
			}
			else
			{
				GET_PLAYER(pTriggeredData->m_eOtherPlayer).AI_changeMemoryCount(getID(), MEMORY_EVENT_BAD_TO_US, -kEvent.getAttitudeModifier());
			}
		}
	}

	if (0 != kEvent.getTheirEnemyAttitudeModifier())
	{
		if (NO_PLAYER != pTriggeredData->m_eOtherPlayer)
		{
			TeamTypes eWorstEnemy = GET_TEAM(GET_PLAYER(pTriggeredData->m_eOtherPlayer).getTeam()).AI_getWorstEnemy();
			if (NO_TEAM != eWorstEnemy)
			{
				for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
				{
					CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);
					if (kLoopPlayer.isAlive() && kLoopPlayer.getTeam() == eWorstEnemy)
					{
						if (kEvent.getTheirEnemyAttitudeModifier() > 0)
						{
							kLoopPlayer.AI_changeMemoryCount(getID(), MEMORY_EVENT_GOOD_TO_US, kEvent.getTheirEnemyAttitudeModifier());
							AI_changeMemoryCount((PlayerTypes)iPlayer, MEMORY_EVENT_GOOD_TO_US, kEvent.getTheirEnemyAttitudeModifier());
						}
						else
						{
							kLoopPlayer.AI_changeMemoryCount(getID(), MEMORY_EVENT_BAD_TO_US, -kEvent.getTheirEnemyAttitudeModifier());
							AI_changeMemoryCount((PlayerTypes)iPlayer, MEMORY_EVENT_BAD_TO_US, -kEvent.getTheirEnemyAttitudeModifier());
						}
					}
				}
			}
		}
	}

	if (!isEmpty(kEvent.getPythonCallback()))
	{
		long lResult;

		CyArgsList argsList;
		argsList.add(eEvent);
		argsList.add(gDLL->getPythonIFace()->makePythonObject(pTriggeredData));

		gDLL->getPythonIFace()->callFunction(PYRandomEventModule, kEvent.getPythonCallback(), argsList.makeFunctionArgs(), &lResult);
	}

	if (kEvent.getNumWorldNews() > 0)
	{
		int iText = GC.getGameINLINE().getSorenRandNum(kEvent.getNumWorldNews(), "Event World News choice");

		CvWString szGlobalText;

		TeamTypes eTheirWorstEnemy = NO_TEAM;
		if (NO_PLAYER != pTriggeredData->m_eOtherPlayer)
		{
			eTheirWorstEnemy = GET_TEAM(GET_PLAYER(pTriggeredData->m_eOtherPlayer).getTeam()).AI_getWorstEnemy();
		}

		szGlobalText = gDLL->getText(kEvent.getWorldNews(iText).GetCString(),
			getCivilizationAdjectiveKey(),
			NULL != pCity ? pCity->getNameKey() : L"",
			pTriggeredData->m_eOtherPlayer != NO_PLAYER ? GET_PLAYER(pTriggeredData->m_eOtherPlayer).getCivilizationAdjectiveKey() : L"",
			NULL != pOtherPlayerCity ? pOtherPlayerCity->getNameKey() : L"",
			L"",
			NO_TEAM != eTheirWorstEnemy ? GET_TEAM(eTheirWorstEnemy).getName().GetCString() : L""
			);

		for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
		{
			CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);

			if (kLoopPlayer.isAlive())
			{
				if (GET_TEAM(kLoopPlayer.getTeam()).isHasMet(getTeam()) && (NO_PLAYER == pTriggeredData->m_eOtherPlayer || GET_TEAM(GET_PLAYER(pTriggeredData->m_eOtherPlayer).getTeam()).isHasMet(getTeam())))
				{
					bool bShowPlot = GC.getEventTriggerInfo(pTriggeredData->m_eTrigger).isShowPlot();

					if (bShowPlot)
					{
						if (kLoopPlayer.getTeam() != getTeam())
						{
							if (NULL == pPlot || !pPlot->isRevealed(kLoopPlayer.getTeam(), false))
							{
								bShowPlot = false;
							}
						}
					}

					if (bShowPlot)
					{
						gDLL->getInterfaceIFace()->addMessage((PlayerTypes)iPlayer, false, GC.getEVENT_MESSAGE_TIME(), szGlobalText, "AS2D_CIVIC_ADOPT", MESSAGE_TYPE_MINOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_WHITE"), pTriggeredData->m_iPlotX, pTriggeredData->m_iPlotY, true, true);
					}
					else
					{
						gDLL->getInterfaceIFace()->addMessage((PlayerTypes)iPlayer, false, GC.getEVENT_MESSAGE_TIME(), szGlobalText, "AS2D_CIVIC_ADOPT", MESSAGE_TYPE_MINOR_EVENT);
					}
				}
			}
		}

		GC.getGameINLINE().addReplayMessage(REPLAY_MESSAGE_MAJOR_EVENT, getID(), szGlobalText, pTriggeredData->m_iPlotX, pTriggeredData->m_iPlotY, (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"));
	}

	if (!isEmpty(kEvent.getLocalInfoTextKey()))
	{
		CvWString szLocalText;

		TeamTypes eTheirWorstEnemy = NO_TEAM;
		if (NO_PLAYER != pTriggeredData->m_eOtherPlayer)
		{
			eTheirWorstEnemy = GET_TEAM(GET_PLAYER(pTriggeredData->m_eOtherPlayer).getTeam()).AI_getWorstEnemy();
		}

		szLocalText = gDLL->getText(kEvent.getLocalInfoTextKey(),
			getCivilizationAdjectiveKey(),
			NULL != pCity ? pCity->getNameKey() : L"",
			pTriggeredData->m_eOtherPlayer != NO_PLAYER ? GET_PLAYER(pTriggeredData->m_eOtherPlayer).getCivilizationAdjectiveKey() : L"",
			NULL != pOtherPlayerCity ? pOtherPlayerCity->getNameKey() : L"",
			L"",
			NO_TEAM != eTheirWorstEnemy ? GET_TEAM(eTheirWorstEnemy).getName().GetCString() : L""
			);

			if (GC.getEventTriggerInfo(pTriggeredData->m_eTrigger).isShowPlot())
			{
				gDLL->getInterfaceIFace()->addMessage(getID(), false, GC.getEVENT_MESSAGE_TIME(), szLocalText, "AS2D_CIVIC_ADOPT", MESSAGE_TYPE_MINOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_WHITE"), pTriggeredData->m_iPlotX, pTriggeredData->m_iPlotY, true, true);
			}
			else
			{
				gDLL->getInterfaceIFace()->addMessage(getID(), false, GC.getEVENT_MESSAGE_TIME(), szLocalText, "AS2D_CIVIC_ADOPT", MESSAGE_TYPE_MINOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_WHITE"));
			}

	}

	if (!isEmpty(kEvent.getOtherPlayerPopup()))
	{
		if (NO_PLAYER != pTriggeredData->m_eOtherPlayer)
		{
			CvWString szText = gDLL->getText(kEvent.getOtherPlayerPopup(),
				getCivilizationAdjectiveKey(),
				NULL != pCity ? pCity->getNameKey() : L"",
				pTriggeredData->m_eOtherPlayer != NO_PLAYER ? GET_PLAYER(pTriggeredData->m_eOtherPlayer).getCivilizationAdjectiveKey() : L"",
				NULL != pOtherPlayerCity ? pOtherPlayerCity->getNameKey() : L"",
				L""
				);

			CvPopupInfo* pInfo = new CvPopupInfo();

			if (NULL != pInfo)
			{
				pInfo->setText(szText);

				GET_PLAYER(pTriggeredData->m_eOtherPlayer).addPopup(pInfo);
			}
		}
	}

	bool bDeleteTrigger = bUpdateTrigger;

	for (int iEvent = 0; iEvent < GC.getNumEventInfos(); ++iEvent)
	{
		if (0 == kEvent.getAdditionalEventTime(iEvent))
		{
			if (kEvent.getAdditionalEventChance(iEvent) > 0)
			{
				if (canDoEvent((EventTypes)iEvent, *pTriggeredData))
				{
					if (GC.getGameINLINE().getSorenRandNum(100, "Additional Event") < kEvent.getAdditionalEventChance(iEvent))
					{
						applyEvent((EventTypes)iEvent, iEventTriggeredId, false);
					}
				}
			}
		}
		else
		{
			bool bSetTimer = true;
			if (kEvent.getAdditionalEventChance(iEvent) > 0)
			{
				if (GC.getGameINLINE().getSorenRandNum(100, "Additional Event 2") >= kEvent.getAdditionalEventChance(iEvent))
				{
					bSetTimer = false;
				}
			}

			if (bSetTimer)
			{
				EventTriggeredData kTriggered = *pTriggeredData;
				kTriggered.m_iTurn = (GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getGrowthPercent() * kEvent.getAdditionalEventTime((EventTypes)iEvent)) / 100 + GC.getGameINLINE().getGameTurn();

				const EventTriggeredData* pExistingTriggered = getEventCountdown((EventTypes)iEvent);

				if (NULL != pExistingTriggered)
				{
					kTriggered.m_iTurn = std::min(kTriggered.m_iTurn, pExistingTriggered->m_iTurn);
				}

				setEventCountdown((EventTypes)iEvent, kTriggered);
				bDeleteTrigger = false;
			}
		}
	}

	if (bDeleteTrigger)
	{
		deleteEventTriggered(iEventTriggeredId);
	}
}
void CvPlayer::freeEuropeUnits()
{
	for (uint i = 0; i < m_aEuropeUnits.size(); ++i)
	{
		SAFE_DELETE(m_aEuropeUnits[i]);
	}
	m_aEuropeUnits.clear();
}

int CvPlayer::getEventCost(EventTypes eEvent, PlayerTypes eOtherPlayer, bool bRandom) const
{
	CvEventInfo& kEvent = GC.getEventInfo(eEvent);

	int iGold = kEvent.getGold();
	if (bRandom)
	{
		iGold += kEvent.getRandomGold();
	}
	
	return iGold;
}


void CvPlayer::doEvents()
{
	if (GC.getGameINLINE().isOption(GAMEOPTION_NO_EVENTS))
	{
		return;
	}

	CvEventMap::iterator it = m_mapEventsOccured.begin();
	while (it != m_mapEventsOccured.end())
	{
		if (checkExpireEvent((*it).first, (*it).second))
		{
			expireEvent((*it).first, (*it).second, true);
			it = m_mapEventsOccured.erase(it);
		}
		else
		{
			++it;
		}
	}

	bool bNewEventEligible = true;
	if (GC.getGameINLINE().getElapsedGameTurns() < GC.getDefineINT("FIRST_EVENT_DELAY_TURNS"))
	{
		bNewEventEligible = false;
	}

	if (bNewEventEligible)
	{
		if (GC.getGameINLINE().getSorenRandNum(GC.getDefineINT("EVENT_PROBABILITY_ROLL_SIDES"), "Global event check") >= GC.getEraInfo(getCurrentEra()).getEventChancePerTurn())
		{
			bNewEventEligible = false;
		}
	}

	std::vector< std::pair<EventTriggeredData*, int> > aePossibleEventTriggerWeights;
	int iTotalWeight = 0;
	for (int i = 0; i < GC.getNumEventTriggerInfos(); ++i)
	{
		int iWeight = getEventTriggerWeight((EventTriggerTypes)i);
		if (iWeight < 0)
		{
			trigger((EventTriggerTypes)i);
		}
		else if (iWeight > 0 && bNewEventEligible)
		{
			EventTriggeredData* pTriggerData = initTriggeredData((EventTriggerTypes)i);
			if (NULL != pTriggerData)
			{
				iTotalWeight += iWeight;
				aePossibleEventTriggerWeights.push_back(std::make_pair(pTriggerData, iTotalWeight));
			}
		}
	}

	if (iTotalWeight > 0)
	{
		bool bFired = false;
		int iValue = GC.getGameINLINE().getSorenRandNum(iTotalWeight, "Event trigger");
		for (std::vector< std::pair<EventTriggeredData*, int> >::iterator it = aePossibleEventTriggerWeights.begin(); it != aePossibleEventTriggerWeights.end(); ++it)
		{
			EventTriggeredData* pTriggerData = (*it).first;
			if (NULL != pTriggerData)
			{
				if (iValue < (*it).second && !bFired)
				{
					trigger(*pTriggerData);
					bFired = true;
				}
				else
				{
					deleteEventTriggered(pTriggerData->getID());
				}
			}
		}
	}

	std::vector<int> aCleanup;
	for (int i = 0; i < GC.getNumEventInfos(); ++i)
	{
		const EventTriggeredData* pTriggeredData = getEventCountdown((EventTypes)i);
		if (NULL != pTriggeredData)
		{
			if (GC.getGameINLINE().getGameTurn() >= pTriggeredData->m_iTurn)
			{
				applyEvent((EventTypes)i, pTriggeredData->m_iId);
				resetEventCountdown((EventTypes)i);
				aCleanup.push_back(pTriggeredData->m_iId);
			}
		}
	}

	for (std::vector<int>::iterator it = aCleanup.begin(); it != aCleanup.end(); ++it)
	{
		bool bDelete = true;

		for (int i = 0; i < GC.getNumEventInfos(); ++i)
		{
			const EventTriggeredData* pTriggeredData = getEventCountdown((EventTypes)i);
			if (NULL != pTriggeredData)
			{
				if (pTriggeredData->m_iId == *it)
				{
					bDelete = false;
					break;
				}
			}
		}

		if (bDelete)
		{
			deleteEventTriggered(*it);
		}
	}
}


void CvPlayer::expireEvent(EventTypes eEvent, const EventTriggeredData& kTriggeredData, bool bFail)
{
	FAssert(getEventOccured(eEvent) == &kTriggeredData);
	FAssert(GC.getEventInfo(eEvent).isQuest() || GC.getGameINLINE().getGameTurn() - kTriggeredData.m_iTurn <= 4);

	if (GC.getEventInfo(eEvent).isQuest())
	{
		CvMessageQueue::iterator it;
		for (it = m_listGameMessages.begin(); it != m_listGameMessages.end(); ++it)
		{
			CvTalkingHeadMessage& message = *it;

			// the trigger ID is stored in the otherwise unused length field
			if (message.getLength() == kTriggeredData.getID())
			{
				m_listGameMessages.erase(it);
				gDLL->getInterfaceIFace()->dirtyTurnLog(getID());
				break;
			}
		}

		if (bFail)
		{
			gDLL->getInterfaceIFace()->addMessage(getID(), false, GC.getEVENT_MESSAGE_TIME(), gDLL->getText(GC.getEventInfo(eEvent).getQuestFailTextKey()), "AS2D_CIVIC_ADOPT", MESSAGE_TYPE_MINOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"));
		}
	}
}

bool CvPlayer::checkExpireEvent(EventTypes eEvent, const EventTriggeredData& kTriggeredData) const
{
	CvEventInfo& kEvent = GC.getEventInfo(eEvent);

	if (!isEmpty(kEvent.getPythonExpireCheck()))
	{
		long lResult;

		CyArgsList argsList;
		argsList.add(eEvent);
		argsList.add(gDLL->getPythonIFace()->makePythonObject(&kTriggeredData));

		gDLL->getPythonIFace()->callFunction(PYRandomEventModule, kEvent.getPythonExpireCheck(), argsList.makeFunctionArgs(), &lResult);

		if (0 != lResult)
		{
			return true;
		}
	}

	if (!kEvent.isQuest())
	{
		if (GC.getGameINLINE().getGameTurn() - kTriggeredData.m_iTurn > 2)
		{
			return true;
		}

		return false;
	}

	CvEventTriggerInfo& kTrigger = GC.getEventTriggerInfo(kTriggeredData.m_eTrigger);

	FAssert(kTriggeredData.m_ePlayer != NO_PLAYER);

	CvPlayer& kPlayer = GET_PLAYER(kTriggeredData.m_ePlayer);

	if (NO_CIVIC != kTrigger.getCivic())
	{
		if (!kPlayer.isCivic((CivicTypes)kTrigger.getCivic()))
		{
			return true;
		}
	}

	if (kTriggeredData.m_iCityId != -1)
	{
		if (NULL == kPlayer.getCity(kTriggeredData.m_iCityId))
		{
			return true;
		}
	}

	if (kTriggeredData.m_iUnitId != -1)
	{
		if (NULL == kPlayer.getUnit(kTriggeredData.m_iUnitId))
		{
			return true;
		}
	}

	if (NO_PLAYER != kTriggeredData.m_eOtherPlayer)
	{
		if (!GET_PLAYER(kTriggeredData.m_eOtherPlayer).isAlive())
		{
			return true;
		}

		if (kTriggeredData.m_iOtherPlayerCityId != -1)
		{
			if (NULL == GET_PLAYER(kTriggeredData.m_eOtherPlayer).getCity(kTriggeredData.m_iOtherPlayerCityId))
			{
				return true;
			}
		}
	}
	return false;
}


void CvPlayer::trigger(EventTriggerTypes eTrigger)
{
	initTriggeredData(eTrigger, true);
}

void CvPlayer::trigger(const EventTriggeredData& kData)
{
	if (isHuman())
	{
		CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_EVENT, kData.getID());
		addPopup(pInfo, GC.getEventTriggerInfo(kData.m_eTrigger).isFrontPopup());
	}
	else
	{
		EventTypes eEvent = AI_chooseEvent(kData.getID());
		if (NO_EVENT != eEvent)
		{
			applyEvent(eEvent, kData.getID());
		}
	}
}

bool CvPlayer::canTrigger(EventTriggerTypes eTrigger, PlayerTypes ePlayer) const
{
	if (!isAlive())
	{
		return false;
	}

	if (getID() == ePlayer)
	{
		return false;
	}

	CvPlayer& kPlayer = GET_PLAYER(ePlayer);
	CvEventTriggerInfo& kTrigger = GC.getEventTriggerInfo(eTrigger);

	if (getTeam() == kPlayer.getTeam())
	{
		return false;
	}

	if (!kTrigger.isPickPlayer())
	{
		return false;
	}

	if (!GET_TEAM(getTeam()).isHasMet(kPlayer.getTeam()))
	{
		return false;
	}

	if (isHuman() && kTrigger.isOtherPlayerAI())
	{
		return false;
	}

	if (!isNative() && kTrigger.isOtherPlayerNative())
	{
		return false;
	}

	if (!isPlayable() && kTrigger.isOtherPlayerPlayable())
	{
		return false;
	}

	if (GET_TEAM(getTeam()).isAtWar(kPlayer.getTeam()) != kTrigger.isOtherPlayerWar())
	{
		return false;
	}

	if (kTrigger.getOtherPlayerShareBorders() > 0)
	{
		int iCount = 0;

		for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); ++iI)
		{
			CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

			if (!pLoopPlot->isWater())
			{
				if ((pLoopPlot->getOwnerINLINE() == getID()) && pLoopPlot->isAdjacentPlayer(ePlayer, true))
				{
					++iCount;
				}
			}
		}

		if (iCount < kTrigger.getOtherPlayerShareBorders())
		{
			return false;
		}
	}

	return true;
}

CvCity* CvPlayer::pickTriggerCity(EventTriggerTypes eTrigger) const
{
	CvCity* pCity = NULL;
	std::vector<CvCity*> apCities;
	int iLoop;
	int iBestValue = MIN_INT;
	for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		int iValue = pLoopCity->getTriggerValue(eTrigger);
		if (iValue > iBestValue)
		{
			if (NULL != pCity)
			{
				apCities.push_back(pCity);
			}

			iBestValue = iValue;
			pCity = pLoopCity;
		}
		else if (MIN_INT != iValue)
		{
			apCities.push_back(pLoopCity);
		}
	}

	if (NULL == pCity && apCities.size() > 0)
	{
		int iChosen = GC.getGameINLINE().getSorenRandNum(apCities.size(), "Event pick city");
		pCity = apCities[iChosen];
	}

	return pCity;
}

CvUnit* CvPlayer::pickTriggerUnit(EventTriggerTypes eTrigger, CvPlot* pPlot, bool bPickPlot) const
{
	CvUnit* pUnit = NULL;
	std::vector<CvUnit*> apUnits;
	int iLoop;
	int iBestValue = MIN_INT;
	for (CvUnit* pLoopUnit = firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = nextUnit(&iLoop))
	{
		int iValue = pLoopUnit->getTriggerValue(eTrigger, pPlot, bPickPlot);

		if (iValue > iBestValue)
		{
			if (NULL != pUnit)
			{
				apUnits.push_back(pUnit);
			}

			iBestValue = iValue;
			pUnit = pLoopUnit;
		}
		else if (MIN_INT != iValue)
		{
			apUnits.push_back(pLoopUnit);
		}
	}

	if (NULL == pUnit && apUnits.size() > 0)
	{
		int iChosen = GC.getGameINLINE().getSorenRandNum(apUnits.size(), "Event pick unit");
		pUnit = apUnits[iChosen];
	}

	return pUnit;
}

int CvPlayer::getEventTriggerWeight(EventTriggerTypes eTrigger) const
{
	CvEventTriggerInfo& kTrigger = GC.getEventTriggerInfo(eTrigger);

	if (NO_HANDICAP != kTrigger.getMinDifficulty())
	{
		if (GC.getGameINLINE().getHandicapType() < kTrigger.getMinDifficulty())
		{
			return 0;
		}
	}

	if (kTrigger.isTutorial())
	{
		if (!isHuman() || !isOption(PLAYEROPTION_TUTORIAL))
		{
			return 0;
		}
	}

	if (kTrigger.isSinglePlayer() && GC.getGameINLINE().isGameMultiPlayer())
	{
		return 0;
	}

	if (!GC.getGameINLINE().isEventActive(eTrigger))
	{
		return 0;
	}
	if (!kTrigger.isRecurring())
	{
		if (isTriggerFired(eTrigger))
		{
			return 0;
		}
	}
	if (kTrigger.getNumPrereqEvents() > 0)
	{
		bool bFoundValid = true;
		for (int iI = 0; iI < kTrigger.getNumPrereqEvents(); iI++)
		{
			if (NULL == getEventOccured((EventTypes)kTrigger.getPrereqEvent(iI)))
			{
				bFoundValid = false;
				break;
			}
		}

		if (!bFoundValid)
		{
			return 0;
		}
	}

	if (NO_CIVIC != kTrigger.getCivic())
	{
		bool bFoundValid = false;

		for (int iI = 0; iI < GC.getNumCivicOptionInfos(); ++iI)
		{
			if (getCivic((CivicOptionTypes)iI) == kTrigger.getCivic())
			{
				bFoundValid = true;
				break;
			}
		}

		if (!bFoundValid)
		{
			return 0;
		}
	}

	if (kTrigger.getMinTreasury() > 0)
	{
		if (getGold() < kTrigger.getMinTreasury())
		{
			return 0;
		}
	}

	if (GC.getMapINLINE().getNumLandAreas() < kTrigger.getMinMapLandmass())
	{
		return 0;
	}

	if (kTrigger.getMinOurLandmass() > 0 || kTrigger.getMaxOurLandmass() != -1)
	{
		int iNumLandmass = 0;

		int iLoop;
		for (CvArea* pArea = GC.getMapINLINE().firstArea(&iLoop); NULL != pArea; pArea = GC.getMapINLINE().nextArea(&iLoop))
		{
			if (!pArea->isWater())
			{
				if (pArea->getCitiesPerPlayer(getID()) > 0)
				{
					++iNumLandmass;
				}
			}
		}

		if (iNumLandmass < kTrigger.getMinOurLandmass())
		{
			return 0;
		}

		if (kTrigger.getMaxOurLandmass() != -1 && iNumLandmass > kTrigger.getMaxOurLandmass())
		{
			return 0;
		}
	}

	if (kTrigger.getProbability() < 0)
	{
		return kTrigger.getProbability();
	}

	int iProbability = kTrigger.getProbability();

	if (kTrigger.isProbabilityUnitMultiply() && kTrigger.getNumUnits() > 0)
	{
		int iNumUnits = 0;
		int iLoop;
		for (CvUnit* pLoopUnit = firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = nextUnit(&iLoop))
		{
			if (MIN_INT != pLoopUnit->getTriggerValue(eTrigger, NULL, true))
			{
				++iNumUnits;
			}
		}

		iProbability *= iNumUnits;
	}

	if (kTrigger.isProbabilityBuildingMultiply() && kTrigger.getNumBuildings() > 0)
	{
		int iNumBuildings = 0;
		for (int i = 0; i < kTrigger.getNumBuildingsRequired(); ++i)
		{
			if (kTrigger.getBuildingRequired(i) != NO_BUILDINGCLASS)
			{
				iNumBuildings += getBuildingClassCount((BuildingClassTypes)kTrigger.getBuildingRequired(i));
			}
		}

		iProbability *= iNumBuildings;
	}

	return iProbability;
}

bool CvPlayer::isFreePromotion(UnitCombatTypes eUnitCombat, PromotionTypes ePromotion) const
{
	for (UnitCombatPromotionArray::const_iterator it = m_aFreeUnitCombatPromotions.begin(); it != m_aFreeUnitCombatPromotions.end(); ++it)
	{
		if ((*it).first == eUnitCombat && (*it).second == ePromotion)
		{
			return true;
		}
	}

	return false;
}

void CvPlayer::setFreePromotion(UnitCombatTypes eUnitCombat, PromotionTypes ePromotion, bool bFree)
{
	for (UnitCombatPromotionArray::iterator it = m_aFreeUnitCombatPromotions.begin(); it != m_aFreeUnitCombatPromotions.end(); ++it)
	{
		if ((*it).first == eUnitCombat && (*it).second == ePromotion)
		{
			if (!bFree)
			{
				m_aFreeUnitCombatPromotions.erase(it);
			}
			return;
		}
	}

	if (bFree)
	{
		m_aFreeUnitCombatPromotions.push_back(std::make_pair(eUnitCombat, ePromotion));
	}
}

bool CvPlayer::isFreePromotion(UnitClassTypes eUnitClass, PromotionTypes ePromotion) const
{
	for (UnitClassPromotionArray::const_iterator it = m_aFreeUnitClassPromotions.begin(); it != m_aFreeUnitClassPromotions.end(); ++it)
	{
		if ((*it).first == eUnitClass && (*it).second == ePromotion)
		{
			return true;
		}
	}

	return false;
}

void CvPlayer::setFreePromotion(UnitClassTypes eUnitClass, PromotionTypes ePromotion, bool bFree)
{
	for (UnitClassPromotionArray::iterator it = m_aFreeUnitClassPromotions.begin(); it != m_aFreeUnitClassPromotions.end(); ++it)
	{
		if ((*it).first == eUnitClass && (*it).second == ePromotion)
		{
			if (!bFree)
			{
				m_aFreeUnitClassPromotions.erase(it);
			}
			return;
		}
	}

	if (bFree)
	{
		m_aFreeUnitClassPromotions.push_back(std::make_pair(eUnitClass, ePromotion));
	}
}

// CACHE: cache frequently used values
///////////////////////////////////////
void CvPlayer::invalidatePopulationRankCache()
{
	int iLoop;
	CvCity* pLoopCity;
	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		pLoopCity->invalidatePopulationRankCache();
	}
}

void CvPlayer::invalidateYieldRankCache(YieldTypes eYield)
{
	int iLoop;
	CvCity* pLoopCity;
	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		pLoopCity->invalidateYieldRankCache();
	}
}

void CvPlayer::doUpdateCacheOnTurn()
{
	// add this back, after testing without it
	// invalidateYieldRankCache();
}

void CvPlayer::doEra()
{
	int iGameYear = GC.getGameINLINE().getGameTurnYear();
	EraTypes eBestEra = getCurrentEra();
	for (int i = eBestEra + 1; i < GC.getNumEraInfos(); i++)
	{
		EraTypes eLoopEra = (EraTypes) i;
		if(GC.getEraInfo(eLoopEra).isRevolution() == isInRevolution())
		{
			if(GC.getEraInfo(eLoopEra).getGameTurn() >= 0)
			{
				int iEraYear = GC.getGameINLINE().getTurnYear(GC.getEraInfo(eLoopEra).getGameTurn());

				if(iEraYear <= iGameYear)
				{
					eBestEra = eLoopEra;
				}
			}
			else
			{
				eBestEra = eLoopEra;
			}
		}
	}

	setCurrentEra(eBestEra);
}

PlayerTypes CvPlayer::pickConqueredCityOwner(const CvCity& kCity) const
{
	return getID();
}

void CvPlayer::forcePeace(PlayerTypes ePlayer)
{
	FAssert(GET_TEAM(getTeam()).canChangeWarPeace(GET_PLAYER(ePlayer).getTeam()));

	CLinkList<TradeData> playerList;
	CLinkList<TradeData> loopPlayerList;
	TradeData kTradeData;
	setTradeItem(&kTradeData, TRADE_PEACE_TREATY, 0, NULL);
	playerList.insertAtEnd(kTradeData);
	loopPlayerList.insertAtEnd(kTradeData);

	GC.getGameINLINE().implementDeal(getID(), ePlayer, &playerList, &loopPlayerList);
}

int CvPlayer::getNewCityProductionValue() const
{
	int iValue = 0;
	for (int iJ = 0; iJ < GC.getNumBuildingClassInfos(); iJ++)
	{
		BuildingTypes eBuilding = ((BuildingTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iJ)));

		if (NO_BUILDING != eBuilding)
		{
			if (GC.getBuildingInfo(eBuilding).getFreeStartEra() != NO_ERA)
			{
				if (GC.getGameINLINE().getStartEra() >= GC.getBuildingInfo(eBuilding).getFreeStartEra())
				{
					for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
					{
						int iModifier = 0;
						if (iYield == YIELD_HAMMERS)
						{
							iModifier = getProductionModifier(eBuilding);
						}
						iValue += (100 * getYieldProductionNeeded(eBuilding, (YieldTypes)iYield)) / std::max(1, 100 + iModifier);
					}
				}
			}
		}
	}

	iValue *= 100 + GC.getDefineINT("NEW_CITY_BUILDING_VALUE_MODIFIER");
	iValue /= 100;

	iValue += (GC.getDefineINT("ADVANCED_START_CITY_COST") * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getGrowthPercent()) / 100;

	int iPopulation = GC.getDefineINT("INITIAL_CITY_POPULATION") + GC.getEraInfo(GC.getGameINLINE().getStartEra()).getFreePopulation();
	for (int i = 1; i <= iPopulation; ++i)
	{
		iValue += (getGrowthThreshold(i) * GC.getDefineINT("ADVANCED_START_POPULATION_COST")) / 100;
	}

	return iValue;
}

int CvPlayer::getGrowthThreshold(int iPopulation) const
{
	int iThreshold;

	iThreshold = (GC.getDefineINT("BASE_CITY_GROWTH_THRESHOLD") + (iPopulation * GC.getDefineINT("CITY_GROWTH_MULTIPLIER")));

	iThreshold *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getGrowthPercent();
	iThreshold /= 100;

	iThreshold *= GC.getEraInfo(GC.getGameINLINE().getStartEra()).getGrowthPercent();
	iThreshold /= 100;

	if (isNative())
	{
		iThreshold *= GC.getDefineINT("NATIVE_GROWTH_THRESHOLD_MULTIPLIER");
		iThreshold /= 100;
	}
	else if (!isHuman())
	{
		iThreshold *= GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIGrowthPercent();
		iThreshold /= 100;

		iThreshold *= std::max(0, ((GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIPerEraModifier() * getCurrentEra()) + 100));
		iThreshold /= 100;
	}

	return std::max(1, iThreshold);
}

void CvPlayer::verifyUnitStacksValid()
{
	int iLoop;
	for(CvUnit* pLoopUnit = firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = nextUnit(&iLoop))
	{
		pLoopUnit->verifyStackValid();
	}
}

CvCity* CvPlayer::getPopulationUnitCity(int iUnitId) const
{
	int iLoop;

	for (CvCity* pCity = firstCity(&iLoop); pCity != NULL; pCity = nextCity(&iLoop))
	{
		if (NULL != pCity->getPopulationUnitById(iUnitId))
		{
			return pCity;
		}
	}

	return NULL;
}

int CvPlayer::getYieldSellPrice(YieldTypes eYield) const
{
	FAssert(eYield >= 0);
	FAssert(eYield < NUM_YIELD_TYPES);

	return std::max(1, getYieldBuyPrice(eYield) + GC.getYieldInfo(eYield).getSellPriceDifference());
}

int CvPlayer::getYieldBuyPrice(YieldTypes eYield) const
{	
	FAssert(eYield >= 0);
	FAssert(eYield < NUM_YIELD_TYPES);
	return m_aiYieldBuyPrice[eYield];
}

void CvPlayer::setYieldBuyPrice(YieldTypes eYield, int iPrice, bool bMessage)
{
	FAssert(eYield >= 0);
	FAssert(eYield < NUM_YIELD_TYPES);
	
	iPrice = std::max(iPrice, 1);
	if (iPrice != getYieldBuyPrice(eYield))
	{
		int iOldPrice = getYieldBuyPrice(eYield);
		m_aiYieldBuyPrice[eYield] = iPrice;

		gDLL->getInterfaceIFace()->setDirty(EuropeScreen_DIRTY_BIT, true);

		if (bMessage)
		{
			CvWString szMessage;
			if (iPrice > iOldPrice)
			{
				szMessage = gDLL->getText("TXT_KEY_PRICE_RISE", GC.getYieldInfo(eYield).getTextKeyWide(), GC.getCivilizationInfo(getCivilizationType()).getShortDescriptionKey(), getYieldBuyPrice(eYield));
			}
			else
			{
				szMessage = gDLL->getText("TXT_KEY_PRICE_FALL", GC.getYieldInfo(eYield).getTextKeyWide(), GC.getCivilizationInfo(getCivilizationType()).getShortDescriptionKey(), getYieldBuyPrice(eYield));
			}
		
			for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
			{
				CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes) iPlayer);
				if (kLoopPlayer.isAlive() && GET_TEAM(getTeam()).isHasMet(kLoopPlayer.getTeam()))
				{
					gDLL->getInterfaceIFace()->addMessage(kLoopPlayer.getID(), true, GC.getEVENT_MESSAGE_TIME(), szMessage, "AS2D_ADVISOR_SUGGEST", MESSAGE_TYPE_INFO, NULL);
				}
			}
		}
	}
}

void CvPlayer::sellYieldUnitToEurope(CvUnit* pUnit, int iAmount, int iCommission)
{
	FAssert(pUnit != NULL);
	if (pUnit != NULL)
	{
		FAssert(pUnit->isGoods());
		FAssert(getParent() != NO_PLAYER);
		CvPlayer& kPlayerEurope = GET_PLAYER(getParent());
		bool bDelayedDeath = (pUnit->getUnitTravelState() == NO_UNIT_TRAVEL_STATE);

		YieldTypes eYield = pUnit->getYield();
		if (NO_YIELD != eYield)
		{
			if (isYieldEuropeTradable(eYield))
			{
				iAmount = std::min(iAmount, pUnit->getYieldStored());
				int iProfit = getSellToEuropeProfit(eYield, iAmount * (100 - iCommission) / 100);
				changeGold(iProfit * getExtraTradeMultiplier(kPlayerEurope.getID()) / 100);

				changeYieldTradedTotal(eYield, iAmount);
				kPlayerEurope.changeYieldTradedTotal(eYield, iAmount);
				GC.getGameINLINE().changeYieldBoughtTotal(kPlayerEurope.getID(), eYield, -iAmount);
				
				pUnit->setYieldStored(pUnit->getYieldStored() - iAmount);
				if (pUnit->getYieldStored() <= 0)
				{
					pUnit->setTransportUnit(NULL);
					pUnit->kill(bDelayedDeath);
				}

				for (int i = 0; i < GC.getNumFatherPointInfos(); ++i)
				{
					FatherPointTypes ePointType = (FatherPointTypes) i;

					changeFatherPoints(ePointType, iProfit * GC.getFatherPointInfo(ePointType).getEuropeTradeGoldPointPercent() / 100);
				}

				CvWStringBuffer szMessage;
				GAMETEXT.setEuropeYieldSoldHelp(szMessage, *this, eYield, iAmount, iCommission);
				m_aszTradeMessages.push_back(szMessage.getCString());
				gDLL->getInterfaceIFace()->addMessage(getID(), true, GC.getEVENT_MESSAGE_TIME(), szMessage.getCString(), "AS2D_BUILD_BANK", MESSAGE_TYPE_LOG_ONLY);
				
				gDLL->getInterfaceIFace()->setDirty(EuropeScreen_DIRTY_BIT, true);

				gDLL->getEventReporterIFace()->yieldSoldToEurope(getID(), eYield, iAmount);
			}
		}
		else if (pUnit->getUnitInfo().isTreasure())
		{
			int iAmount = pUnit->getYieldStored();
			int iNetAmount = iAmount * (100 - iCommission) / 100;
			iNetAmount -= (iNetAmount * getTaxRate()) / 100;
			changeGold(iNetAmount * getExtraTradeMultiplier(kPlayerEurope.getID()) / 100);

			pUnit->setYieldStored(0);
			pUnit->setTransportUnit(NULL);
			pUnit->kill(bDelayedDeath);

			CvWStringBuffer szMessage;
			GAMETEXT.setEuropeYieldSoldHelp(szMessage, *this, eYield, iAmount, iCommission);
			m_aszTradeMessages.push_back(szMessage.getCString());
			gDLL->getInterfaceIFace()->addMessage(getID(), true, GC.getEVENT_MESSAGE_TIME(), szMessage.getCString(), "AS2D_BUILD_BANK", MESSAGE_TYPE_LOG_ONLY);

			gDLL->getInterfaceIFace()->setDirty(EuropeScreen_DIRTY_BIT, true);
		}
	}
}

CvUnit* CvPlayer::buyYieldUnitFromEurope(YieldTypes eYield, int iAmount, CvUnit* pTransport)
{
	if (!isYieldEuropeTradable(eYield))
	{
		return NULL;
	}

	FAssert(pTransport != NULL);
	if (NULL == pTransport)
	{
		return NULL;
	}

	int iAmountAvailable = pTransport->getLoadYieldAmount(eYield);
	iAmount = std::min(iAmount, iAmountAvailable);
	if(iAmount == 0)
	{
		FAssertMsg(false, "Can't load cargo yield.");
		return NULL;
	}

	FAssert(pTransport->getOwnerINLINE() == getID());
	FAssert(getParent() != NO_PLAYER);
	CvPlayer& kPlayerEurope = GET_PLAYER(getParent());
	int iPrice = iAmount * kPlayerEurope.getYieldSellPrice(eYield);
	FAssert(iPrice <= getGold());
	if (iPrice > getGold())
	{
		m_aszTradeMessages.push_back(gDLL->getText("EUROPE_SCREEN_BUY_UNIT_LACK_FUNDS", GC.getYieldInfo(eYield).getTextKeyWide(), iPrice));
		gDLL->getInterfaceIFace()->setDirty(EuropeScreen_DIRTY_BIT, true);
		return NULL;
	}

	CvYieldInfo& kYield = GC.getYieldInfo(eYield);
	UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(kYield.getUnitClass());
	FAssert(NO_UNIT != eUnit);
	if (NO_UNIT == eUnit)
	{
		return NULL;
	}

	CvUnit* pUnit = initUnit(eUnit, (ProfessionTypes) GC.getUnitInfo(eUnit).getDefaultProfession(), INVALID_PLOT_COORD, INVALID_PLOT_COORD, NO_UNITAI, NO_DIRECTION, iAmount);
	FAssert(NULL != pUnit);
	if (NULL != pUnit)
	{
		pUnit->setUnitTravelState(UNIT_TRAVEL_STATE_IN_EUROPE, false);
		UnitAITypes eUnitAI = pUnit->AI_getUnitAIType();
		pUnit->removeFromMap(); //needs to match addToMap
		pUnit->addToMap(pTransport->getX_INLINE(), pTransport->getY_INLINE());
		pUnit->AI_setUnitAIType(eUnitAI);
		
		//unit possibly killed after joining other cargo
		if (!pUnit->setTransportUnit(pTransport))
		{
			pUnit = NULL;
		}

		changeGold(-iPrice);
		changeYieldTradedTotal(eYield, iAmount);
		kPlayerEurope.changeYieldTradedTotal(eYield, iAmount);
		GC.getGameINLINE().changeYieldBoughtTotal(kPlayerEurope.getID(), eYield, iAmount);

		CvWStringBuffer szMessage;
		GAMETEXT.setEuropeYieldBoughtHelp(szMessage, *this, eYield, iAmount);
		m_aszTradeMessages.push_back(szMessage.getCString());
		gDLL->getInterfaceIFace()->addMessage(getID(), true, GC.getEVENT_MESSAGE_TIME(), szMessage.getCString(), "AS2D_BUILD_BANK", MESSAGE_TYPE_LOG_ONLY);

		gDLL->getInterfaceIFace()->setDirty(EuropeScreen_DIRTY_BIT, true);

		gDLL->getEventReporterIFace()->yieldBoughtFromEurope(getID(), eYield, iAmount);
	}

	return pUnit;
}

int CvPlayer::getEuropeUnitBuyPrice(UnitTypes eUnit) const
{
	CvUnitInfo& kUnit = GC.getUnitInfo(eUnit);

	int iCost = kUnit.getEuropeCost();
	
	bool bNegative = (iCost < 0);
	iCost = std::abs(iCost);

	iCost += GET_TEAM(getTeam()).getEuropeUnitsPurchased((UnitClassTypes) kUnit.getUnitClassType()) * kUnit.getEuropeCostIncrease();

	iCost *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent();
	iCost /= 100;

	iCost *= GC.getEraInfo(GC.getGameINLINE().getStartEra()).getTrainPercent();
	iCost /= 100;

	for (int iTrait = 0; iTrait < GC.getNumTraitInfos(); ++iTrait)
	{
		if (hasTrait((TraitTypes) iTrait))
		{
			iCost *= std::max(0, (100 - GC.getTraitInfo((TraitTypes) iTrait).getRecruitPriceDiscount()));
			iCost /= 100;
		}
	}

	if (!isHuman())
	{
		iCost *= GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAITrainPercent();
		iCost /= 100;

		iCost *= std::max(0, ((GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIPerEraModifier() * getCurrentEra()) + 100));
		iCost /= 100;
	}

	if(bNegative)
	{
		iCost = std::min(-iCost, -1);
	}

	return iCost;
}

CvUnit* CvPlayer::buyEuropeUnit(UnitTypes eUnit, int iPriceModifier)
{
	FAssert(canTradeWithEurope());
	if (!canTradeWithEurope())
	{
		return NULL;
	}

	if (getEuropeUnitBuyPrice(eUnit) < 0)
	{
		return NULL;
	}

	int iPrice = getEuropeUnitBuyPrice(eUnit) * iPriceModifier / 100;
	if (iPrice > getGold())
	{
		m_aszTradeMessages.push_back(gDLL->getText("EUROPE_SCREEN_BUY_UNIT_LACK_FUNDS", GC.getUnitInfo(eUnit).getTextKeyWide(), iPrice));
		gDLL->getInterfaceIFace()->setDirty(EuropeScreen_DIRTY_BIT, true);
		return NULL;
	}
	
	CvUnit* pUnit = NULL;
	CvPlot* pStartingPlot = getStartingPlot();
	if (GC.getUnitInfo(eUnit).getDomainType() == DOMAIN_SEA && pStartingPlot != NULL)
	{
		pUnit = initUnit(eUnit, (ProfessionTypes) GC.getUnitInfo(eUnit).getDefaultProfession(), INVALID_PLOT_COORD, INVALID_PLOT_COORD);
        if (pUnit != NULL)
		{
			pUnit->setUnitTravelState(UNIT_TRAVEL_STATE_IN_EUROPE, false);
			//add unit to map after setting Europe state so that it doesn't bump enemy units
			pUnit->addToMap(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE());
		}
	}
	else
	{
		pUnit = initEuropeUnit(eUnit);
	}

	FAssert(NULL != pUnit);
	if (NULL != pUnit)
	{
		changeGold(-iPrice);
		GET_TEAM(getTeam()).changeEuropeUnitsPurchased(pUnit->getUnitClassType(), 1);
		gDLL->getEventReporterIFace()->unitBoughtFromEurope(getID(), pUnit->getID());
	}

	return pUnit;
}

void CvPlayer::buyUnitsFromKing()
{
	PlayerTypes eParent = getParent();
	if (eParent == NO_PLAYER)
	{
		return;
	}

	CvPlayer& kParent = GET_PLAYER(eParent);

	int iNumUnits;
	UnitTypes eUnit;
	ProfessionTypes eProfession;
	int iPrice = kParent.AI_militaryHelp(getID(), iNumUnits, eUnit, eProfession);

	int iPriceModifier = GC.getDefineINT("KING_BUY_UNIT_PRICE_MODIFIER");
	for (int i = 0; i < iNumUnits; ++i)
	{
		CvUnit* pUnit = buyEuropeUnit(eUnit, iPriceModifier);
		if (pUnit != NULL)
		{
			pUnit->setProfession(eProfession);
		}
	}

	if (iNumUnits > 0)
	{
		changeBellsStored(iNumUnits * getEuropeUnitBuyPrice(eUnit) * iPriceModifier / 100);
	}
}


int CvPlayer::getYieldTradedTotal(YieldTypes eYield) const
{
	FAssert(eYield >= 0);
	FAssert(eYield < NUM_YIELD_TYPES);

	return m_aiYieldTradedTotal[eYield];
}

void CvPlayer::setYieldTradedTotal(YieldTypes eYield, int iValue)
{
	FAssert(eYield >= 0);
	FAssert(eYield < NUM_YIELD_TYPES);

	if(iValue != getYieldTradedTotal(eYield))
	{
		m_aiYieldTradedTotal[eYield] = iValue;
	}
}

void CvPlayer::changeYieldTradedTotal(YieldTypes eYield, int iChange)
{
	setYieldTradedTotal(eYield, getYieldTradedTotal(eYield) + iChange);
}

void CvPlayer::changeYieldBoughtTotal(YieldTypes eYield, int iChange)
{
	setYieldBoughtTotal(eYield, getYieldBoughtTotal(eYield) + iChange);
}

YieldTypes CvPlayer::getHighestTradedYield() const
{
	YieldTypes eBestYield = NO_YIELD;
	for (int i = 0; i < NUM_YIELD_TYPES; i++)
	{
		YieldTypes eYield = (YieldTypes) i;
		if (isYieldEuropeTradable(eYield))
		{
			if (getCity(getHighestStoredYieldCityId(eYield)) != NULL)
			{
				if ((eBestYield == NO_YIELD) || (getYieldTradedTotal(eYield) > getYieldTradedTotal(eBestYield)))
				{
					eBestYield = eYield;
				}
			}
		}
	}

	return eBestYield;
}

int CvPlayer::getHighestStoredYieldCityId(YieldTypes eYield) const
{
	int iLoop;
	int iBestCityId = -1;
	int iBestAmount = 0;
	for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		int iAmount = pLoopCity->getYieldStored(eYield);
		if(iAmount > iBestAmount)
		{
			iBestAmount = iAmount;
			iBestCityId = pLoopCity->getID();
		}
	}

	return iBestCityId;
}

int CvPlayer::getYieldBoughtTotal(YieldTypes eYield) const
{
	FAssert(eYield >= 0 && eYield < NUM_YIELD_TYPES);
	return m_aiYieldBoughtTotal[eYield];
}

void CvPlayer::setYieldBoughtTotal(YieldTypes eYield, int iValue)
{
	FAssert(eYield >= 0 && eYield < NUM_YIELD_TYPES);
	m_aiYieldBoughtTotal[eYield] = iValue;
}

int CvPlayer::getCrossesStored() const
{
	return m_iCrossesStored;
}

void CvPlayer::changeCrossesStored(int iChange)
{
	m_iCrossesStored += iChange;
	FAssert(getCrossesStored() >= 0);
}

int CvPlayer::getBellsStored() const
{
	return m_iBellsStored;
}

void CvPlayer::changeBellsStored(int iChange)
{
	m_iBellsStored += iChange;
	FAssert(getBellsStored() >= 0);
}

int CvPlayer::getTaxRate() const
{
	return m_iTaxRate;
}

void CvPlayer::setTaxRate(int iValue)
{
	m_iTaxRate = iValue;
	FAssert(getTaxRate() >= 0);
}

void CvPlayer::changeTaxRate(int iChange)
{
	if (iChange != 0)
	{
		int iOldRate = getTaxRate();
		setTaxRate(iOldRate + iChange);

		//reset yields traded
		for(int i=0;i<NUM_YIELD_TYPES;i++)
		{
			setYieldTradedTotal((YieldTypes) i, 0);
		}

		PlayerTypes eParent = getParent();
		if (eParent != NO_PLAYER)
		{
			CvString szTextKey = (iOldRate < getTaxRate() ? "TXT_KEY_TAX_RATE_CHANGED" : "TXT_KEY_TAX_RATE_LOWERED");
			CvWString szBuffer = gDLL->getText(szTextKey.GetCString(), GET_PLAYER(eParent).getNameKey(), iOldRate, getTaxRate());
			gDLL->getInterfaceIFace()->addMessage(getID(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_REVOLTSTART", MESSAGE_TYPE_MINOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"));
		}
	}
}

int CvPlayer::getNativeCombatModifier() const
{
	return m_iNativeCombatModifier;
}

void CvPlayer::setNativeCombatModifier(int iValue)
{
	m_iNativeCombatModifier = iValue;
}

void CvPlayer::changeNativeCombatModifier(int iChange)
{
	setNativeCombatModifier(getNativeCombatModifier() + iChange);
}

int CvPlayer::getDominateNativeBordersCount() const
{
	return m_iDominateNativeBordersCount;
}

void CvPlayer::changeDominateNativeBordersCount(int iChange)
{
	if (iChange != 0)
	{
		m_iDominateNativeBordersCount += iChange;
		FAssert(getDominateNativeBordersCount() >= 0);

		GC.getMapINLINE().updateCulture();
	}
}

int CvPlayer::getRevolutionEuropeTradeCount() const
{
	return m_iRevolutionEuropeTradeCount;
}

void CvPlayer::changeRevolutionEuropeTradeCount(int iChange)
{
	if (iChange != 0)
	{
		m_iRevolutionEuropeTradeCount += iChange;
		FAssert(getRevolutionEuropeTradeCount() >= 0);

		validateTradeRoutes();

		if (GC.getGameINLINE().getActiveTeam() == getID())
		{
			gDLL->getInterfaceIFace()->setDirty(ColoredPlots_DIRTY_BIT, true);
		}
	}
}

bool CvPlayer::canTradeWithEurope() const
{
	if (!isInRevolution())
	{
		return true;
	}

	if (getRevolutionEuropeTradeCount() > 0)
	{
		return true;
	}

	return false;
}


int CvPlayer::getSellToEuropeProfit(YieldTypes eYield, int iAmount) const
{
	if (getParent() == NO_PLAYER)
	{
		return 0;
	}

	if (!isYieldEuropeTradable(eYield))
	{
		return 0;
	}

	CvPlayer& kPlayerEurope = GET_PLAYER(getParent());

	int iPrice = iAmount * kPlayerEurope.getYieldBuyPrice(eYield);
	iPrice -= (iPrice * getTaxRate()) / 100;

	return iPrice;
}

void CvPlayer::doAction(PlayerActionTypes eAction, int iData1, int iData2, int iData3)
{
	switch (eAction)
	{
	case PLAYER_ACTION_BUY_EUROPE_UNIT:
		buyEuropeUnit((UnitTypes) iData1, 100);
		break;
	case PLAYER_ACTION_SELL_YIELD_UNIT:
		sellYieldUnitToEurope(getUnit(iData3), iData2, iData1);
		break;
	case PLAYER_ACTION_BUY_YIELD_UNIT:
		buyYieldUnitFromEurope((YieldTypes) iData1, iData2, getUnit(iData3));
		break;
	case PLAYER_ACTION_LOAD_UNIT_FROM_EUROPE:
		loadUnitFromEurope(getEuropeUnitById(iData1), getUnit(iData2));
		break;
	case PLAYER_ACTION_UNLOAD_UNIT_TO_EUROPE:
		unloadUnitToEurope(getUnit(iData1));
		break;
	case PLAYER_ACTION_TRANSFER_UNIT_IN_EUROPE:
		transferUnitInEurope(getUnit(iData1), getUnit(iData2));
		break;
	case PLAYER_ACTION_SET_OPTION:
		setOption((PlayerOptionTypes)iData1, (bool) iData2);
		break;
	case PLAYER_ACTION_AUTO_MOVES:
		setAutoMoves(true);
		break;
	case PLAYER_ACTION_TURN_COMPLETE:
		setEndTurn(true);
		break;
	case PLAYER_ACTION_APPLY_EVENT:
		applyEvent((EventTypes) iData1, iData2);
		break;
	case PLAYER_ACTION_CONVINCE_FATHER:
		GET_TEAM(getTeam()).convinceFather((FatherTypes) iData1, iData2 != 0);
		break;
	case PLAYER_ACTION_SET_CIVIC:
		setCivic((CivicOptionTypes) iData1, (CivicTypes) iData2);
		break;
	case PLAYER_ACTION_RECEIVE_GOODY:
		receiveRandomGoody(GC.getMapINLINE().plotByIndexINLINE(iData2), (GoodyTypes) iData1, getUnit(iData3));
		break;
	case PLAYER_ACTION_BUY_UNITS_FROM_KING:
		buyUnitsFromKing();
		break;
	case PLAYER_ACTION_HURRY:
		hurry((HurryTypes) iData1, iData2);
		break;	
	case PLAYER_ACTION_EUROPE_CHANGE_PROFESSION:
		changeProfessionEurope(iData1, (ProfessionTypes) iData2);
		break;	
	case PLAYER_ACTION_FEAT:
		setFeatAccomplished((FeatTypes) iData1, iData2 != 0);
		break;	
	default:
		FAssertMsg(false, "Unknown action");
		break;
	}
}

int CvPlayer::getTradeYieldAmount(YieldTypes eYield, CvUnit* pTransport) const
{
	FAssert(pTransport != NULL);
	if (pTransport == NULL)
	{
		return 0;
	}

	if (!GC.getYieldInfo(eYield).isCargo())
	{
		return 0;
	}

	CvPlot *pPlot = pTransport->plot();
	FAssert(pPlot != NULL);

	int iAmount = 0;
	if (getID() == pTransport->getOwnerINLINE()) //offer yields on the transport
	{
		for (int i=0;i<pPlot->getNumUnits();i++)
		{
			CvUnit* pLoopUnit = pPlot->getUnitByIndex(i);
			if (pLoopUnit != NULL)
			{
				if (pLoopUnit->getTransportUnit() == pTransport)
				{
					if (pLoopUnit->getYield() == eYield)
					{
						iAmount += pLoopUnit->getYieldStored();
					}
				}
			}
		}
	}
	else //offer yields from the city
	{
		CvCity *pCity = pPlot->getPlotCity();
		FAssert(pCity != NULL);
		if (pCity == NULL)
		{
			return 0;
		}

		int iCityAmount = pCity->getYieldStored(eYield);
		int iAmountAvailable = pTransport->getLoadYieldAmount(eYield);
		iAmount = std::min(iCityAmount, iAmountAvailable);
	}

	return iAmount;
}

void CvPlayer::setCityBillboardDirty(bool bNewValue)
{
	int iLoop;
	for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		pLoopCity->setBillboardDirty(bNewValue);
	}
}

bool CvPlayer::isEurope() const
{
	if (getCivilizationType() == NO_CIVILIZATION)
	{
		return false;
	}

	if (GC.getCivilizationInfo(getCivilizationType()).getDerivativeCiv() == NO_CIVILIZATION)
	{
		return false;
	}

	return true;
}

bool CvPlayer::isInRevolution() const
{
	if (getParent() == NO_PLAYER)
	{
		return false;
	}

	if (!::atWar(getTeam(), GET_PLAYER(getParent()).getTeam()))
	{
		return false;
	}

	return true; 
}

bool CvPlayer::checkIndependence() const
{
	if (getParent() == NO_PLAYER)
	{
		return false;
	}

	for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
	{
		CvPlayer& kParent = GET_PLAYER((PlayerTypes) iPlayer);

		if (kParent.isAlive() && GET_TEAM(kParent.getTeam()).isParentOf(getTeam()))
		{
			// land units on map
			int iNumUnits = kParent.countNumTravelUnits(NO_UNIT_TRAVEL_STATE, DOMAIN_LAND);
			if (iNumUnits > 0)
			{
				return false;
			}

			// need both ships and land units
			int iNumLandUnits = kParent.countNumDomainUnits(DOMAIN_LAND);
			int iShips = kParent.countNumDomainUnits(DOMAIN_SEA);

			for (int i = 0; i < getNumRevolutionEuropeUnits(); ++i)
			{
				switch (GC.getUnitInfo(getRevolutionEuropeUnit(i)).getDomainType())
				{
				case DOMAIN_SEA:
					++iShips;
					break;
				case DOMAIN_LAND:
					++iNumLandUnits;
					break;
				default:
					break;
				}
			}

			if (iShips > 0 && iNumLandUnits > 0)
			{
				return false;
			}
		}
	}

	return true;
}


void CvPlayer::applyMissionaryPoints(CvCity* pCity)
{
	FAssert(pCity->getOwnerINLINE() == getID());
	PlayerTypes ePlayer = pCity->getMissionaryPlayer();
	if (ePlayer != NO_PLAYER)
	{
		int iModifier = 100 + getMissionaryRateModifier() + GET_PLAYER(ePlayer).getMissionaryRateModifier();
		changeMissionaryPoints(ePlayer, pCity->getMissionaryRate() * iModifier / 100);
		int iThreshold = missionaryThreshold(ePlayer);
		if (getMissionaryPoints(ePlayer) >= iThreshold)
		{
			//spawn converted native
			bool bUnitCreated = false;
			UnitClassTypes eUnitClass = (UnitClassTypes) GC.getCivilizationInfo(getCivilizationType()).getCapturedCityUnitClass();
			if (eUnitClass != NO_UNITCLASS)
			{
				UnitTypes eUnit = (UnitTypes) GC.getCivilizationInfo(GET_PLAYER(ePlayer).getCivilizationType()).getCivilizationUnits(eUnitClass);
				if (eUnit != NO_UNIT)
				{
					CvUnit* pUnit = GET_PLAYER(ePlayer).initUnit(eUnit, (ProfessionTypes) GC.getUnitInfo(eUnit).getDefaultProfession(), pCity->getX_INLINE(), pCity->getY_INLINE());
					if(pUnit != NULL)
					{
						bUnitCreated = true;
						gDLL->getEventReporterIFace()->missionaryConvertedUnit(pUnit);

						CvWString szBuffer = gDLL->getText("TXT_KEY_NATIVES_CONVERTED", pCity->getNameKey());
						gDLL->getInterfaceIFace()->addMessage(ePlayer, true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_UNIT_GREATPEOPLE", MESSAGE_TYPE_INFO, GC.getUnitInfo(eUnit).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_UNIT_TEXT"), pCity->getX_INLINE(), pCity->getY_INLINE(), true, true);

						changeMissionaryPoints(ePlayer, -iThreshold);
						setMissionaryThresholdMultiplier(ePlayer, (getMissionaryThresholdMultiplier(ePlayer) * (100 + GC.getDefineINT("MISSIONARY_THRESHOLD_INCREASE"))) / 100);
					}
				}
			}
			FAssert(bUnitCreated);
		}
	}
}

int CvPlayer::getMissionaryPoints(PlayerTypes ePlayer) const
{
	FAssert(ePlayer >= 0 && ePlayer < MAX_PLAYERS);
	return m_aiMissionaryPoints[ePlayer];
}

void CvPlayer::changeMissionaryPoints(PlayerTypes ePlayer, int iChange)
{
	FAssert(ePlayer >= 0 && ePlayer < MAX_PLAYERS);
	if(iChange != 0)
	{
		m_aiMissionaryPoints[ePlayer] += iChange;
		FAssert(m_aiMissionaryPoints[ePlayer] >= 0);
	}
}

int CvPlayer::getMissionaryThresholdMultiplier(PlayerTypes ePlayer) const
{
	FAssert(ePlayer >= 0 && ePlayer < MAX_PLAYERS);
	return m_aiMissionaryThresholdMultiplier[ePlayer];
}

void CvPlayer::setMissionaryThresholdMultiplier(PlayerTypes ePlayer, int iValue)
{
	FAssert(ePlayer >= 0 && ePlayer < MAX_PLAYERS);
	m_aiMissionaryThresholdMultiplier[ePlayer] = iValue;
	FAssert(getMissionaryThresholdMultiplier(ePlayer) > 0);
}

int CvPlayer::missionaryThreshold(PlayerTypes ePlayer) const
{
	int iThreshold = ((GC.getDefineINT("MISSIONARY_THRESHOLD") * std::max(0, (getMissionaryThresholdMultiplier(ePlayer)))) / 100);

	iThreshold *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getGrowthPercent();
	iThreshold /= 100;

	iThreshold *= GC.getEraInfo(GC.getGameINLINE().getStartEra()).getGrowthPercent();
	iThreshold /= 100;

	return std::max(1, iThreshold);
}

void CvPlayer::burnMissions(PlayerTypes ePlayer)
{
	int iLoop;
	for (CvCity* pCity = firstCity(&iLoop); pCity != NULL; pCity = nextCity(&iLoop))
	{
		if (pCity->getMissionaryPlayer() == ePlayer)
		{
			pCity->setMissionaryPlayer(NO_PLAYER);
		}
	}
}

bool CvPlayer::canHaveMission(PlayerTypes ePlayer) const
{
	if (!isNative())
	{
		return false;
	}

	if (!GET_PLAYER(ePlayer).isAlive())
	{
		return false;
	}

	if (GET_PLAYER(ePlayer).isNative())
	{
		return false;
	}

	if (::atWar(GET_PLAYER(ePlayer).getTeam(), getTeam()))
	{
		return false;
	}

	return true;
}

void CvPlayer::validateMissions()
{
	for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
	{
		if (!canHaveMission((PlayerTypes) iPlayer))
		{
			burnMissions((PlayerTypes) iPlayer);
		}
	}
}

int CvPlayer::getMissionaryRateModifier() const
{
	return m_iMissionaryRateModifier;
}

void  CvPlayer::changeMissionaryRateModifier(int iChange)
{
	m_iMissionaryRateModifier += iChange;
}

int CvPlayer::getMissionarySuccessPercent() const
{
	return m_iMissionarySuccessPercent;
}

void CvPlayer::setMissionarySuccessPercent(int iValue)
{
	m_iMissionarySuccessPercent = iValue;
}

int CvPlayer::getRebelCombatPercent() const
{
	int iPercent = 100;

	for (int iTrait = 0; iTrait < GC.getNumTraitInfos(); ++iTrait)
	{
		TraitTypes eTrait = (TraitTypes) iTrait;
		CvTraitInfo& kTrait = GC.getTraitInfo(eTrait);
		if (kTrait.getRebelCombatModifier() != 0)
		{
			if (hasTrait(eTrait))
			{
				iPercent += kTrait.getRebelCombatModifier();
			}
		}
	}

	iPercent *= GC.getDefineINT("MAX_REBEL_COMBAT_MULTIPLER");
	iPercent /= 100;

	return iPercent;
}

int CvPlayer::getProfessionEquipmentModifier(ProfessionTypes eProfession) const
{
	FAssert(eProfession >= 0 && eProfession < GC.getNumProfessionInfos());
	return m_aiProfessionEquipmentModifier[eProfession];
}

void CvPlayer::setProfessionEquipmentModifier(ProfessionTypes eProfession, int iValue)
{
	FAssert(eProfession >= 0 && eProfession < GC.getNumProfessionInfos());
	int iChange = iValue - getProfessionEquipmentModifier(eProfession);
	if (iChange != 0)
	{
		std::vector<CvUnit*> aProfessionUnits;
		int iLoop;
		for (CvUnit* pUnit = firstUnit(&iLoop); pUnit != NULL; pUnit = nextUnit(&iLoop))
		{
			if (pUnit->getProfession() == eProfession)
			{
				aProfessionUnits.push_back(pUnit);
			}
		}
		for (uint i = 0; i < m_aEuropeUnits.size(); ++i)
		{
			CvUnit* pUnit = m_aEuropeUnits[i];
			if (pUnit->getProfession() == eProfession)
			{
				aProfessionUnits.push_back(pUnit);
			}
		}
		for (CvCity* pCity = firstCity(&iLoop); pCity != NULL; pCity = nextCity(&iLoop))
		{
			for (int i = 0; i < pCity->getPopulation(); ++i)
			{
				CvUnit* pUnit = pCity->getPopulationUnitByIndex(i);
				if (pUnit->getProfession() == eProfession)
				{
					aProfessionUnits.push_back(pUnit);
				}
			}
		}

		for (uint i = 0; i < aProfessionUnits.size(); ++i)
		{
			CvUnit* pUnit = aProfessionUnits[i];
			changeAssets(-pUnit->getAsset());
			int iPower = pUnit->getPower();
			changePower(-iPower);
			CvArea* pArea = pUnit->area();
			if (pArea != NULL)
			{
				pArea->changePower(getID(), -iPower);
			}
		}

		m_aiProfessionEquipmentModifier[eProfession] = iValue;

		for (uint i = 0; i < aProfessionUnits.size(); ++i)
		{
			CvUnit* pUnit = aProfessionUnits[i];
			changeAssets(pUnit->getAsset());
			int iPower = pUnit->getPower();
			changePower(iPower);
			CvArea* pArea = pUnit->area();
			if (pArea != NULL)
			{
				pArea->changePower(getID(), iPower);
			}
		}

		FAssert(getProfessionEquipmentModifier(eProfession) >= -100);
	}
}

int CvPlayer::getYieldEquipmentAmount(ProfessionTypes eProfession, YieldTypes eYield) const
{
	FAssert(eProfession >= 0 && eProfession < GC.getNumProfessionInfos());
	FAssert(eYield >= 0 && eYield < NUM_YIELD_TYPES);

	int iAmount = GC.getProfessionInfo(eProfession).getYieldEquipmentAmount(eYield);

	iAmount *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent();
	iAmount /= 100;
	
	if (!isHuman())
	{
		iAmount *= GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAITrainPercent();
		iAmount /= 100;
	}

	iAmount *= 100 + getProfessionEquipmentModifier(eProfession);
	iAmount /= 100;

	return std::max(0, iAmount);
}

bool CvPlayer::isProfessionValid(ProfessionTypes eProfession, UnitTypes eUnit) const
{
	if (eProfession != NO_PROFESSION)
	{
		if (!GC.getCivilizationInfo(getCivilizationType()).isValidProfession(eProfession))
		{
			return false;
		}

		if (eUnit != NO_UNIT)
		{
			if (GC.getUnitInfo(eUnit).isOnlyDefensive())
			{
				// Natives cannot be combat professions
				CvProfessionInfo& kProfession = GC.getProfessionInfo(eProfession);
				if ((!kProfession.isUnarmed() && kProfession.getCombatChange() > 0 && !kProfession.isScout()) || kProfession.getMissionaryRate() > 0)
				{
					return false;
				}
			}

			if (GC.getUnitInfo(eUnit).getDefaultProfession() == NO_PROFESSION)
			{
				return false;
			}
		}
	}

	return true;
}

void CvPlayer::doPrices()
{
	if (isEurope())
	{
		for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
		{
			YieldTypes eYield = (YieldTypes) iYield;
			CvYieldInfo& kYield = GC.getYieldInfo(eYield);

			if (kYield.isCargo())
			{
				int iBaseThreshold = kYield.getPriceChangeThreshold() * GC.getHandicapInfo(getHandicapType()).getEuropePriceThresholdMultiplier() * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getGrowthPercent() / 10000;
				int iNewPrice = kYield.getBuyPriceLow() + GC.getGameINLINE().getSorenRandNum(kYield.getBuyPriceHigh() - kYield.getBuyPriceLow() + 1, "Price selection");
				iNewPrice += getYieldBoughtTotal(eYield) / std::max(1, iBaseThreshold);

				if (GC.getGameINLINE().getSorenRandNum(100, "Price correction") < kYield.getPriceCorrectionPercent() * std::abs(iNewPrice - getYieldBuyPrice(eYield)))
				{
					iNewPrice = std::min(iNewPrice, getYieldBuyPrice(eYield) + 1);
					iNewPrice = std::max(iNewPrice, getYieldBuyPrice(eYield) - 1);
					setYieldBuyPrice(eYield, iNewPrice, true);
				}
			}
		}
	}

	//do tax rate change
	if (!GC.getEraInfo(getCurrentEra()).isRevolution())
	{
		PlayerTypes eParent = getParent();
		if (eParent != NO_PLAYER)
		{
			if (getHighestTradedYield() != NO_YIELD)
			{
				int iTotalTraded = 0;
				for (int i = 0; i < NUM_YIELD_TYPES; i++)
				{
					if (isYieldEuropeTradable((YieldTypes)i))
					{
						iTotalTraded += getYieldTradedTotal((YieldTypes) i);
					}
				}

				//modify the traded threshold
				int iMultiplier = 100;
				for (int iTrait = 0; iTrait < GC.getNumTraitInfos(); ++iTrait)
				{
					TraitTypes eTrait = (TraitTypes) iTrait;
					CvTraitInfo& kTrait = GC.getTraitInfo(eTrait);
					if (hasTrait(eTrait))
					{
						iMultiplier += kTrait.getTaxRateThresholdModifier();
					}
				}
				iMultiplier += getTaxRate() * GC.getDefineINT("TAX_TRADE_THRESHOLD_TAX_RATE_PERCENT") / 100;
				iMultiplier += GET_PLAYER(eParent).AI_getAttitudeVal(getID()) * GC.getDefineINT("TAX_TRADE_THRESHOLD_ATTITUDE_PERCENT");

				//compare total traded with trade threshold
				if (iTotalTraded * 10000 > GC.getDefineINT("TAX_TRADE_THRESHOLD") * std::max(100, iMultiplier) * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getGrowthPercent())
				{
					//random chance to raise tax rate
					if(GC.getGameINLINE().getSorenRandNum(100, "Tax rate increase") < GC.getDefineINT("TAX_INCREASE_CHANCE"))
					{
						int iOldTaxRate = getTaxRate();
						int iNewTaxRate = std::min(99, iOldTaxRate + 1 + GC.getGameINLINE().getSorenRandNum(GC.getDefineINT("TAX_RATE_MAX_INCREASE"), "Tax Rate Increase"));
						int iChange = iNewTaxRate - iOldTaxRate;

						if (isHuman())
						{
							CvDiploParameters* pDiplo = new CvDiploParameters(eParent);
							pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_KISS_PINKY"));
							pDiplo->addDiploCommentVariable(iOldTaxRate);
							pDiplo->addDiploCommentVariable(iNewTaxRate);
							pDiplo->setData(iChange);
							pDiplo->setAIContact(true);
							gDLL->beginDiplomacy(pDiplo, getID());
						}
						else
						{
							changeTaxRate(iChange);
						}
					}
				}
			}
		}
	}
}

int CvPlayer::getNumRevolutionEuropeUnits() const
{
	return m_aEuropeRevolutionUnits.size();
}

UnitTypes CvPlayer::getRevolutionEuropeUnit(int i) const
{
	FAssert(i >= 0 && i < getNumRevolutionEuropeUnits());

	return m_aEuropeRevolutionUnits[i].first;
}

ProfessionTypes CvPlayer::getRevolutionEuropeProfession(int i) const
{
	FAssert(i >= 0 && i < getNumRevolutionEuropeUnits());

	return m_aEuropeRevolutionUnits[i].second;
}

void CvPlayer::addRevolutionEuropeUnit(UnitTypes eUnit, ProfessionTypes eProfession)
{
	m_aEuropeRevolutionUnits.push_back(std::make_pair(eUnit, eProfession));
}

void CvPlayer::clearRevolutionEuropeUnits()
{
	m_aEuropeRevolutionUnits.clear();
}

UnitTypes CvPlayer::getDocksNextUnit(int i) const
{
	if (!canTradeWithEurope())
	{
		return NO_UNIT;
	}

	FAssert(i < (int)m_aDocksNextUnits.size());
	return m_aDocksNextUnits[i];
}

UnitTypes CvPlayer::pickBestImmigrant()
{
	std::vector<int> aiWeights(GC.getNumUnitInfos(), 0);
	for (int iUnitClass = 0; iUnitClass < GC.getNumUnitClassInfos(); ++iUnitClass)
	{
		UnitTypes eUnit = (UnitTypes) GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iUnitClass);
		if (NO_UNIT != eUnit)
		{
			int iWeight = GC.getUnitInfo(eUnit).getImmigrationWeight();
			for (int i = 0; i < getUnitClassImmigrated((UnitClassTypes) iUnitClass); ++i)
			{
				iWeight *= std::max(0, 100 - GC.getUnitInfo(eUnit).getImmigrationWeightDecay());
				iWeight /= 100;
			}

			aiWeights[eUnit] += iWeight;
		}
	}

	UnitTypes eBestUnit = (UnitTypes) GC.getGameINLINE().getSorenRand().pickValue(aiWeights, "pick immigrant");
	FAssert(NO_UNIT != eBestUnit);
	if (eBestUnit != NO_UNIT)
	{
		changeUnitClassImmigrated((UnitClassTypes) GC.getUnitInfo(eBestUnit).getUnitClassType(), 1);
	}

	return eBestUnit;
}

bool CvPlayer::canHurry(HurryTypes eHurry, int iIndex) const
{
	CvHurryInfo& kHurry = GC.getHurryInfo(eHurry);

	if (getHurryCount(eHurry) > 0)
	{
		if (kHurry.getGoldPerCross() > 0)
		{
			if (canTradeWithEurope())
			{
				if (immigrationThreshold() > getCrossesStored())
				{
					if (getHurryGold(eHurry, iIndex) <= getGold())
					{
						return true;
					}
				}
			}
		}
		else
		{
			return true;
		}
	}

	return false;
}

void CvPlayer::hurry(HurryTypes eHurry, int iIndex)
{
	if (!canHurry(eHurry, iIndex))
	{
		int iPrice = getHurryGold(eHurry, iIndex);
		if (iPrice >= getGold())
		{
			m_aszTradeMessages.push_back(gDLL->getText("EUROPE_SCREEN_RECRUIT_UNIT_LACK_FUNDS", GC.getUnitInfo(getDocksNextUnit(iIndex)).getTextKeyWide(), iPrice));
			gDLL->getInterfaceIFace()->setDirty(EuropeScreen_DIRTY_BIT, true);
		}
	}
	else
	{
		CvHurryInfo& kHurry = GC.getHurryInfo(eHurry);

		if (kHurry.getGoldPerCross() > 0)
		{
			changeGold(-getHurryGold(eHurry, iIndex));
			changeCrossesStored(immigrationThreshold() - getCrossesStored());
			doImmigrant(iIndex);
		}
	}
}

int CvPlayer::getHurryGold(HurryTypes eHurry, int iIndex) const
{
	int iThreshold = immigrationThreshold();
	int iCrossesLeft = iThreshold - getCrossesStored();
	int iGold = GC.getHurryInfo(eHurry).getGoldPerCross() * iCrossesLeft;
	iGold += GC.getHurryInfo(eHurry).getFlatGold() * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getGrowthPercent() / 100;
	if (iIndex != -1)
	{
		int iImmigrationPrice = std::abs(getEuropeUnitBuyPrice(getDocksNextUnit(iIndex))) * iCrossesLeft / std::max(1, iThreshold);
		iGold = std::min(iGold, iImmigrationPrice);
	}
	
	return iGold;
}

const wchar* CvPlayer::getHurryItemTextKey(HurryTypes eHurry, int iData) const
{
	CvHurryInfo& kHurry = GC.getHurryInfo(eHurry);

	if (kHurry.getGoldPerCross() > 0)
	{
		UnitTypes eUnit = getDocksNextUnit(iData);
		if (eUnit != NO_UNIT)
		{
			return GC.getUnitInfo(eUnit).getTextKeyWide();
		}
	}

	return L"";
}


void CvPlayer::doImmigrant(int iIndex)
{
	if (getParent() != NO_PLAYER && canTradeWithEurope())
	{	
		UnitTypes eBestUnit = getDocksNextUnit(iIndex);
		if (NO_UNIT != eBestUnit)
		{
			m_aDocksNextUnits[iIndex] = pickBestImmigrant();

			changeCrossesStored(-immigrationThreshold());
			setImmigrationThresholdMultiplier((getImmigrationThresholdMultiplier() * (100 + GC.getDefineINT("IMMIGRATION_THRESHOLD_INCREASE"))) / 100);

			CvUnit* pUnit = initEuropeUnit(eBestUnit);
			CvWString szBuffer = gDLL->getText("TXT_KEY_NEW_IMMIGRANT_AVAILABLE", GC.getCivilizationInfo(GET_PLAYER(getParent()).getCivilizationType()).getShortDescriptionKey(),  GC.getUnitInfo(eBestUnit).getTextKeyWide());
			gDLL->getInterfaceIFace()->addMessage(getID(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_UNIT_GREATPEOPLE", MESSAGE_TYPE_INFO, GC.getUnitInfo(eBestUnit).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_UNIT_TEXT"));

			gDLL->getInterfaceIFace()->setDirty(EuropeScreen_DIRTY_BIT, true);

			FAssert(pUnit != NULL);
			if(pUnit != NULL)
			{
				gDLL->getEventReporterIFace()->emmigrantAtDocks(getID(), pUnit->getID());
			}
		}
	}
}

void CvPlayer::buyLand(CvPlot* pPlot, bool bFree)
{
	int iCulture = pPlot->getBuyCultureAmount(getID());
	int iGoldCost = bFree ? 0 : pPlot->getBuyPrice(getID());

	if (getGold() < iGoldCost)
	{
		return;
	}

	PlayerTypes eOldOwner = pPlot->getOwnerINLINE();
	if (eOldOwner != NO_PLAYER)
	{
		pPlot->setCulture(eOldOwner, 0, false);
		if (!GET_TEAM(pPlot->getTeam()).isAtWar(getTeam()))
		{
			GET_PLAYER(eOldOwner).changeGold((iGoldCost * GC.getDefineINT("BUY_PLOT_SELLER_INCOME_PERCENT")) / 100);
			GET_PLAYER(getID()).AI_changeGoldTradedTo(eOldOwner, iGoldCost);
		}
	}

	pPlot->changeCulture(getID(), iCulture, true);
	changeGold(-iGoldCost);
}

int CvPlayer::getNumTradeMessages() const
{
	return m_aszTradeMessages.size();
}

const wchar* CvPlayer::getTradeMessage(int i) const
{
	return m_aszTradeMessages[i].GetCString();
}

void CvPlayer::buildTradeTable(PlayerTypes eOtherPlayer, CLinkList<TradeData>& ourList, const IDInfo& kTransport) const
{
	TradeData item;
	int iLoop;

	//	Gold
	setTradeItem(&item, TRADE_GOLD, 0, &kTransport);
	if (canTradeItem(eOtherPlayer, item))
	{
		ourList.insertAtEnd(item);
	}

	//	Maps
	setTradeItem(&item, TRADE_MAPS, 0, &kTransport);
	if (canTradeItem(eOtherPlayer, item))
	{
		ourList.insertAtEnd(item);
	}

	//	Open Borders
	setTradeItem(&item, TRADE_OPEN_BORDERS, 0, &kTransport);
	if (canTradeItem(eOtherPlayer, item))
	{
		ourList.insertAtEnd(item);
	}

	//	Defensive Pact
	setTradeItem(&item, TRADE_DEFENSIVE_PACT, 0, &kTransport);
	if (canTradeItem(eOtherPlayer, item))
	{
		ourList.insertAtEnd(item);
	}

	//	Permanent Alliance
	setTradeItem(&item, TRADE_PERMANENT_ALLIANCE, 0, &kTransport);
	if (canTradeItem(eOtherPlayer, item))
	{
		ourList.insertAtEnd(item);
	}

	if (::atWar(getTeam(), GET_PLAYER(eOtherPlayer).getTeam()))
	{
		//	We are at war, allow a peace treaty option
		setTradeItem(&item, TRADE_PEACE_TREATY, 0, &kTransport);
		ourList.insertAtEnd(item);
	}

	//	Initial build of the inventory lists and buttons.
	//	Go through all the possible headings
	for (int i = NUM_BASIC_ITEMS; i < NUM_TRADEABLE_HEADINGS; i++)
	{
		bool bFoundItemUs = false;

		//	Build what we need to build for this item
		switch (i)
		{
		case TRADE_YIELD:

			FAssert(0 < GC.getNUM_YIELD_TYPES());
			{
				CvUnit* pTransport = ::getUnit(kTransport);
				if (pTransport != NULL)
				{
					for (int j = 0; j < NUM_YIELD_TYPES; j++)
					{
						setTradeItem(&item, TRADE_YIELD, j, &kTransport);
						if (canTradeItem(eOtherPlayer, item))
						{
							bFoundItemUs = true;
							ourList.insertAtEnd(item);
						}
					}
				}
			}
			break;

		case TRADE_CITIES:
			for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
			{
				setTradeItem(&item, TRADE_CITIES, pLoopCity->getID(), &kTransport);
				if (canTradeItem(eOtherPlayer, item))
				{
					bFoundItemUs = true;
					ourList.insertAtEnd(item);
				}
			}
			break;

		case TRADE_PEACE:
			if (!isHuman())
			{
				for (int j = 0; j < MAX_TEAMS; j++)
				{
					if (GET_TEAM((TeamTypes)j).isAlive())
					{
						if (j != getTeam() && j != GET_PLAYER(eOtherPlayer).getTeam())
						{
							setTradeItem(&item, TRADE_PEACE, j, &kTransport);
							if (canTradeItem(eOtherPlayer, item))
							{
								ourList.insertAtEnd(item);
								bFoundItemUs = true;
							}
						}
					}
				}
			}
			break;

		case TRADE_WAR:
			if (!isHuman())
			{
				for (int j = 0; j < MAX_TEAMS; j++)
				{
					if (GET_TEAM((TeamTypes)j).isAlive())
					{
						if (j != getTeam() && j != GET_PLAYER(eOtherPlayer).getTeam())
						{
							setTradeItem(&item, TRADE_WAR, j, &kTransport);
							if (canTradeItem(eOtherPlayer, item))
							{
								ourList.insertAtEnd(item);
								bFoundItemUs = true;
							}
						}
					}
				}
			}
			break;

		case TRADE_EMBARGO:
			if (!isHuman())
			{
				for (int j = 0; j < MAX_TEAMS; j++)
				{
					if (GET_TEAM((TeamTypes)j).isAlive())
					{
						if (j != getTeam() && j != GET_PLAYER(eOtherPlayer).getTeam())
						{
							setTradeItem(&item, TRADE_EMBARGO, j, &kTransport);
							if (canTradeItem(eOtherPlayer, item))
							{
								ourList.insertAtEnd(item);
								bFoundItemUs = true;
							}
						}
					}
				}
			}
			break;
		}
	}
}

bool CvPlayer::getHeadingTradeString(PlayerTypes eOtherPlayer, TradeableItems eItem, CvWString& szString, CvString& szIcon) const
{
	szIcon.clear();

	switch ( eItem )
	{
	case TRADE_YIELD:
		szString = gDLL->getText("TXT_KEY_TRADE_YIELD_HEADING");
		break;

	case TRADE_CITIES:
		szString = gDLL->getText("TXT_KEY_TRADE_CITIES");
		break;

	case TRADE_PEACE:
		szString = gDLL->getText("TXT_KEY_TRADE_MAKE_PEACE_WITH");
		break;

	case TRADE_WAR:
		szString = gDLL->getText("TXT_KEY_TRADE_DECLARE_WAR_ON");
		break;

	case TRADE_EMBARGO:
		szString = gDLL->getText("TXT_KEY_TRADE_STOP_TRADING_WITH");
		break;

	default:
		szString.clear();
		return false;
		break;
	}

	return true;
}


bool CvPlayer::getItemTradeString(PlayerTypes eOtherPlayer, bool bOffer, bool bShowingCurrent, const TradeData& zTradeData, const IDInfo& kTransport, CvWString& szString, CvString& szIcon) const
{
	szIcon.clear();

	switch (zTradeData.m_eItemType)
	{
	case TRADE_GOLD:
		if (bOffer)
		{
			szString = gDLL->getText("TXT_KEY_TRADE_GOLD_NUM", zTradeData.m_iData1);
		}
		else
		{
			szString = gDLL->getText("TXT_KEY_TRADE_GOLD_NUM", getMaxGoldTrade(eOtherPlayer, kTransport));
		}
		break;
	case TRADE_MAPS:
		szString = gDLL->getText("TXT_KEY_TRADE_WORLD_MAP_STRING");
		break;
	case TRADE_OPEN_BORDERS:
		szString = gDLL->getText("TXT_KEY_TRADE_OPEN_BORDERS_STRING");
		break;
	case TRADE_DEFENSIVE_PACT:
		szString = gDLL->getText("TXT_KEY_TRADE_DEFENSIVE_PACT_STRING");
		break;
	case TRADE_PERMANENT_ALLIANCE:
		szString = gDLL->getText("TXT_KEY_TRADE_PERMANENT_ALLIANCE_STRING");
		break;
	case TRADE_PEACE_TREATY:
		szString = gDLL->getText("TXT_KEY_TRADE_PEACE_TREATY_STRING", GC.getDefineINT("PEACE_TREATY_LENGTH"));
		break;
	case TRADE_YIELD:
		{
			YieldTypes eYield = (YieldTypes) zTradeData.m_iData1;
			CvYieldInfo& kYield = GC.getYieldInfo(eYield);
			CvUnit* pTransport = ::getUnit(kTransport);
			FAssert(pTransport != NULL);
			if (pTransport != NULL)
			{
				int iAmount = bOffer ? GET_PLAYER(eOtherPlayer).getTradeYieldAmount(eYield, pTransport) : getTradeYieldAmount(eYield, pTransport);
				szString.Format( L"%s (%d%c)", kYield.getDescription(), iAmount, kYield.getChar());
			}
		}
		break;

	case TRADE_CITIES:
		{
			CvCity* pCity = bOffer ? GET_PLAYER(eOtherPlayer).getCity(zTradeData.m_iData1) : getCity(zTradeData.m_iData1);
			if (NULL != pCity)
			{
				if (pCity->getLiberationPlayer(false) == eOtherPlayer)
				{
					szString.Format(L"%s (%s)", pCity->getName().GetCString(), gDLL->getText("TXT_KEY_LIBERATE_CITY").GetCString());
				}
				else
				{
					szString = gDLL->getText("TXT_KEY_CITY_OF", pCity->getNameKey());
				}
			}
		}
		break;
	case TRADE_PEACE:
		if (bOffer)
		{
			szString = gDLL->getText("TXT_KEY_TRADE_PEACE_WITH");
			szString += GET_TEAM((TeamTypes)zTradeData.m_iData1).getName();
		}
		else
		{
			szString = GET_TEAM((TeamTypes)zTradeData.m_iData1).getName();
		}
		break;
	case TRADE_WAR:
		if (bOffer)
		{
			szString = gDLL->getText("TXT_KEY_TRADE_WAR_WITH");
			szString += GET_TEAM((TeamTypes)zTradeData.m_iData1).getName();
		}
		else
		{
			szString = GET_TEAM((TeamTypes)zTradeData.m_iData1).getName();
		}
		break;
	case TRADE_EMBARGO:
		if (bOffer)
		{
			szString = gDLL->getText("TXT_KEY_TRADE_STOP_TRADING_WITH");
			szString += L" " + GET_TEAM((TeamTypes)zTradeData.m_iData1).getName();
		}
		else
		{
			szString = GET_TEAM((TeamTypes)zTradeData.m_iData1).getName();
		}
		break;
	default:
		szString.clear();
		return false;
	}

	return true;
}

void CvPlayer::updateTradeList(PlayerTypes eOtherPlayer, CLinkList<TradeData>& ourInventory, const CLinkList<TradeData>& ourOffer, const CLinkList<TradeData>& theirOffer, const IDInfo& kTransport) const
{
	for (CLLNode<TradeData>* pNode = ourInventory.head(); pNode != NULL; pNode = ourInventory.next(pNode))
	{
		pNode->m_data.m_bHidden = false;

		// Don't show peace treaties when not at war
		if (!::atWar(getTeam(), GET_PLAYER(eOtherPlayer).getTeam()))
		{
			if (pNode->m_data.m_eItemType == TRADE_PEACE_TREATY)
			{
				pNode->m_data.m_bHidden = true;
			}
		}

		if (pNode->m_data.m_eItemType == TRADE_YIELD)
		{
			for (CLLNode<TradeData>* pOfferNode = theirOffer.head(); pOfferNode != NULL; pOfferNode = theirOffer.next(pOfferNode))
			{
				if (pOfferNode->m_data.m_eItemType == TRADE_YIELD)
				{
					pNode->m_data.m_bHidden = true;
					break;
				}
			}
		}
	}

	if (!isHuman() || !GET_PLAYER(eOtherPlayer).isHuman())  // everything allowed in human-human trades
	{
		CLLNode<TradeData>* pFirstOffer = ourOffer.head();
		if (pFirstOffer == NULL)
		{
			pFirstOffer = theirOffer.head();
		}

		if (pFirstOffer != NULL)
		{
			if (!CvDeal::isEndWar(pFirstOffer->m_data.m_eItemType) || !::atWar(getTeam(), GET_PLAYER(eOtherPlayer).getTeam()))
			{
				for (CLLNode<TradeData>* pNode = ourInventory.head(); pNode != NULL; pNode = ourInventory.next(pNode))
				{
					if (pFirstOffer->m_data.m_eItemType == TRADE_CITIES || pNode->m_data.m_eItemType == TRADE_CITIES)
					{
						pNode->m_data.m_bHidden = true;
					}
					else if (CvDeal::isAnnual(pFirstOffer->m_data.m_eItemType) != CvDeal::isAnnual(pNode->m_data.m_eItemType))
					{
						pNode->m_data.m_bHidden = true;
					}
				}
			}
		}
	}
}

int CvPlayer::getMaxGoldTrade(PlayerTypes eOtherPlayer, const IDInfo& kTransport) const
{
	CvCity* pTradeCity = NULL;
	CvUnit* pTransport = ::getUnit(kTransport);
	if (pTransport != NULL)
	{
		CvPlot* pPlot = pTransport->plot();
		if (pPlot != NULL)
		{
			pTradeCity = pPlot->getPlotCity();
		}
	}

	if (pTradeCity != NULL && pTradeCity->getOwnerINLINE() == getID())
	{
		return pTradeCity->AI_maxGoldTrade(eOtherPlayer);
	}
	else if (!isNative())
	{
		return AI_maxGoldTrade(eOtherPlayer);
	}

	return 0;
}

void CvPlayer::changeProfessionEurope(int iUnitId, ProfessionTypes eProfession)
{
	FAssert(getParent() != NO_PLAYER);
	CvUnit* pUnit = getEuropeUnitById(iUnitId);
	FAssert(pUnit != NULL);
	if (pUnit == NULL)
	{
		return;
	}

	FAssert(pUnit->canHaveProfession(eProfession, false));

	if (eProfession != pUnit->getProfession())
	{
		CvPlayer& kEurope = GET_PLAYER(getParent());
		int iCost = 0;
		for (int i = 0; i < NUM_YIELD_TYPES; ++i)
		{
			YieldTypes eYieldType = (YieldTypes) i;
			int iMissing = pUnit->getProfessionChangeYieldRequired(eProfession, eYieldType);
			if (iMissing > 0)
			{
				iCost += iMissing * kEurope.getYieldSellPrice(eYieldType);
				changeYieldTradedTotal(eYieldType, iMissing);
				kEurope.changeYieldTradedTotal(eYieldType, iMissing);
				GC.getGameINLINE().changeYieldBoughtTotal(kEurope.getID(), eYieldType, iMissing);
			}
			else if (iMissing < 0)
			{
				int iGold = getSellToEuropeProfit(eYieldType, -iMissing);
				iCost -= iGold;
				changeYieldTradedTotal(eYieldType, -iMissing);
				kEurope.changeYieldTradedTotal(eYieldType, -iMissing);
				GC.getGameINLINE().changeYieldBoughtTotal(kEurope.getID(), eYieldType, iMissing);

				for (int j = 0; j < GC.getNumFatherPointInfos(); ++j)
				{
					FatherPointTypes ePointType = (FatherPointTypes) j;

					changeFatherPoints(ePointType, iGold * GC.getFatherPointInfo(ePointType).getEuropeTradeGoldPointPercent() / 100);
				}

			}
		}
		pUnit->setProfession(eProfession);
		changeGold(-iCost);
		gDLL->getInterfaceIFace()->setDirty(EuropeScreen_DIRTY_BIT, true);
	}
}

bool CvPlayer::checkPopulation() const
{
	int iNumPopulation = 0;
	int iLoop;
	for (CvUnit* pLoopUnit = firstUnit(&iLoop); NULL != pLoopUnit; pLoopUnit = nextUnit(&iLoop))
	{
		if (pLoopUnit->getUnitInfo().isFound())
		{
			++iNumPopulation;
		}
	}
	for (CvCity* pLoopCity = firstCity(&iLoop); NULL != pLoopCity; pLoopCity = nextCity(&iLoop))
	{
		iNumPopulation += pLoopCity->getPopulation();
	}
	for (uint i = 0; i < m_aEuropeUnits.size(); ++i)
	{
		if (m_aEuropeUnits[i]->getUnitInfo().isFound())
		{
			++iNumPopulation;
		}
	}

	return (iNumPopulation == getTotalPopulation());
}

bool CvPlayer::checkPower(bool bReset)
{
	int iPower = 0;
	int iAsset = 0;
	std::map<int, int> mapAreaPower;
	int iLoop;
	for (CvUnit* pUnit = firstUnit(&iLoop); pUnit != NULL; pUnit = nextUnit(&iLoop))
	{
		int iUnitPower = pUnit->getPower();
		iPower += iUnitPower;
		CvArea* pArea = pUnit->area();
		if (pArea != NULL)
		{
			mapAreaPower[pArea->getID()] += iUnitPower;
		}
		iAsset += pUnit->getAsset();
	}
	for (uint i = 0; i < m_aEuropeUnits.size(); ++i)
	{
		iPower += m_aEuropeUnits[i]->getPower();
		iAsset += m_aEuropeUnits[i]->getAsset();
	}
	for (CvCity* pCity = firstCity(&iLoop); pCity != NULL; pCity = nextCity(&iLoop))
	{
		int iCityPower = 0;
		int iCityAsset = 0;
		for (int i = 0; i < pCity->getPopulation(); ++i)
		{
			iCityPower += pCity->getPopulationUnitByIndex(i)->getPower();
			iCityAsset += pCity->getPopulationUnitByIndex(i)->getAsset();
		}

		for (int i = 0; i < GC.getNumBuildingInfos(); ++i)
		{
			BuildingTypes eBuilding = (BuildingTypes) i;
			if (pCity->isHasBuilding(eBuilding))
			{
				iCityPower += GC.getBuildingInfo(eBuilding).getPowerValue();
				iCityAsset += GC.getBuildingInfo(eBuilding).getAssetValue();
			}
		}

		for (int i = 0; i < NUM_YIELD_TYPES; ++i)
		{
			YieldTypes eYield = (YieldTypes) i;
			iCityPower += pCity->getYieldStored(eYield) * GC.getYieldInfo(eYield).getPowerValue();
			iCityAsset += pCity->getYieldStored(eYield) * GC.getYieldInfo(eYield).getAssetValue();
		}

		iPower += iCityPower;
		iAsset += iCityAsset;
		mapAreaPower[pCity->area()->getID()] += iCityPower;
	}

	bool bCheck = true;
	if (iPower != getPower())
	{
		if (bReset)
		{
			changePower(iPower - getPower());
		}
		bCheck = false;
	}

	if (iAsset != getAssets())
	{
		if (bReset)
		{
			changeAssets(iAsset - getAssets());
		}
		bCheck = false;
	}

	for (CvArea* pArea = GC.getMapINLINE().firstArea(&iLoop); pArea != NULL; pArea = GC.getMapINLINE().nextArea(&iLoop))
	{
		if (mapAreaPower[pArea->getID()] != pArea->getPower(getID()))
		{
			if (bReset)
			{
				pArea->changePower(getID(), mapAreaPower[pArea->getID()] - pArea->getPower(getID()));
			}
			bCheck = false;
		}
	}

	return bCheck;
}

void CvPlayer::doREFReduction(int iGold)
{
	setRevolutionEuropeUnitThresholdMultiplier(getRevolutionEuropeUnitThresholdMultiplier() * (100 + iGold * GC.getHandicapInfo(getHandicapType()).getKingGoldThresholdPercent() / 100) / 100);
}

void CvPlayer::interceptEuropeUnits()
{
	if (!canTradeWithEurope())
	{
		std::vector<CvUnit*> apEuropeUnits;
		int iLoop;
		for (CvUnit* pUnit = firstUnit(&iLoop); pUnit != NULL; pUnit = nextUnit(&iLoop))
		{
			if (pUnit->getUnitTravelState() == UNIT_TRAVEL_STATE_TO_EUROPE || pUnit->getUnitTravelState() == UNIT_TRAVEL_STATE_IN_EUROPE)
			{
				if (!pUnit->isCargo())
				{
					apEuropeUnits.push_back(pUnit);
				}
			}
		}

		for (size_t i = 0; i < apEuropeUnits.size(); ++i)
		{
			CvUnit* pUnit = apEuropeUnits[i];
			CvPlot* pPlot = pUnit->plot();
			CvWString szMessage = gDLL->getText("TXT_KEY_YOU_UNITS_EUROPE_INTERCEPTED", pUnit->getNameOrProfessionKey());
			gDLL->getInterfaceIFace()->addMessage(getID(), true, GC.getEVENT_MESSAGE_TIME(), szMessage, "AS2D_COMBAT", MESSAGE_TYPE_DISPLAY_ONLY, pUnit->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true);
			pUnit->kill(false);
		}

		for (uint i = 0; i < m_aEuropeUnits.size(); ++i)
		{
			m_aEuropeUnits[i]->updateOwnerCache(-1);
		}
		freeEuropeUnits();
	}
}
