// city.cpp

#include "CvGameCoreDLL.h"
#include "CvGlobals.h"
#include "CvCity.h"
#include "CvArea.h"
#include "CvGameAI.h"
#include "CvMap.h"
#include "CvPlot.h"
#include "CvTeamAI.h"
#include "CvGameCoreUtils.h"
#include "CvPlayerAI.h"
#include "CvUnit.h"
#include "CvInfos.h"
#include "CvRandom.h"
#include "CvArtFileMgr.h"
#include "CvPopupInfo.h"
#include "CyCity.h"
#include "CyArgsList.h"
#include "FProfiler.h"
#include "CvGameTextMgr.h"
#include "CvTradeRoute.h"
#include <numeric>

// interfaces used
#include "CvDLLEngineIFaceBase.h"
#include "CvDLLPythonIFaceBase.h"
#include "CvDLLEntityIFaceBase.h"
#include "CvDLLInterfaceIFaceBase.h"
#include "CvDLLEventReporterIFaceBase.h"

// Public Functions...

CvCity::CvCity()
{
	m_aiSeaPlotYield = new int[NUM_YIELD_TYPES];
	m_aiRiverPlotYield = new int[NUM_YIELD_TYPES];
	m_aiYieldRateModifier = new int[NUM_YIELD_TYPES];
	m_aiYieldStored = new int[NUM_YIELD_TYPES];
	m_aiYieldRushed = new int[NUM_YIELD_TYPES];
	m_aiDomainFreeExperience = new int[NUM_DOMAIN_TYPES];
	m_aiDomainProductionModifier = new int[NUM_DOMAIN_TYPES];

	m_aiCulture = new int[MAX_PLAYERS];
	m_abEverOwned = new bool[MAX_PLAYERS];
	m_abRevealed = new bool[MAX_TEAMS];
	m_abScoutVisited = new bool[MAX_TEAMS];

	m_paiBuildingProduction = NULL;
	m_paiBuildingProductionTime = NULL;
	m_paiBuildingOriginalOwner = NULL;
	m_paiBuildingOriginalTime = NULL;
	m_paiUnitProduction = NULL;
	m_paiUnitProductionTime = NULL;
	m_aiSpecialistWeights = NULL;
	m_paiUnitCombatFreeExperience = NULL;
	m_paiFreePromotionCount = NULL;
	m_pabHasRealBuilding = NULL;
	m_pabHasFreeBuilding = NULL;

	m_paiWorkingPlot = NULL;

	m_paTradeCities = NULL;

	CvDLLEntity::createCityEntity(this);		// create and attach entity to city

	m_aiBaseYieldRank = new int[NUM_YIELD_TYPES];
	m_abBaseYieldRankValid = new bool[NUM_YIELD_TYPES];
	m_aiYieldRank = new int[NUM_YIELD_TYPES];
	m_abYieldRankValid = new bool[NUM_YIELD_TYPES];

	reset(0, NO_PLAYER, 0, 0, true);
}

CvCity::~CvCity()
{
	CvDLLEntity::removeEntity();			// remove entity from engine
	CvDLLEntity::destroyEntity();			// delete CvCityEntity and detach from us

	uninit();

	SAFE_DELETE_ARRAY(m_aiBaseYieldRank);
	SAFE_DELETE_ARRAY(m_abBaseYieldRankValid);
	SAFE_DELETE_ARRAY(m_aiYieldRank);
	SAFE_DELETE_ARRAY(m_abYieldRankValid);

	SAFE_DELETE_ARRAY(m_aiSeaPlotYield);
	SAFE_DELETE_ARRAY(m_aiRiverPlotYield);
	SAFE_DELETE_ARRAY(m_aiYieldRateModifier);
	SAFE_DELETE_ARRAY(m_aiYieldStored);
	SAFE_DELETE_ARRAY(m_aiYieldRushed);
	SAFE_DELETE_ARRAY(m_aiDomainFreeExperience);
	SAFE_DELETE_ARRAY(m_aiDomainProductionModifier);
	SAFE_DELETE_ARRAY(m_aiCulture);
	SAFE_DELETE_ARRAY(m_abEverOwned);
	SAFE_DELETE_ARRAY(m_abRevealed);
	SAFE_DELETE_ARRAY(m_abScoutVisited);
}


void CvCity::init(int iID, PlayerTypes eOwner, int iX, int iY, bool bBumpUnits)
{
	std::vector<int> aOldAttitude(MAX_PLAYERS, 0);
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		CvPlayerAI& kPlayer = GET_PLAYER((PlayerTypes) i );
		if (kPlayer.isAlive())
		{
			kPlayer.AI_invalidateCloseBordersAttitudeCache();
			aOldAttitude[i] = kPlayer.AI_getStolenPlotsAttitude(eOwner);
		}
	}

	CvPlot* pAdjacentPlot;
	CvPlot* pPlot;
	BuildingTypes eLoopBuilding;
	
	pPlot = GC.getMapINLINE().plotINLINE(iX, iY);

	//--------------------------------
	// Log this event
	if (GC.getLogging())
	{
		if (gDLL->getChtLvl() > 0)
		{
			TCHAR szOut[1024];
			sprintf(szOut, "Player %d City %d built at %d:%d\n", eOwner, iID, iX, iY);
			gDLL->messageControlLog(szOut);
		}
	}

	//--------------------------------
	// Init saved data
	reset(iID, eOwner, pPlot->getX_INLINE(), pPlot->getY_INLINE());

	//--------------------------------
	// Init non-saved data
	setupGraphical();

	//--------------------------------
	// Init other game data
	setName(GET_PLAYER(getOwnerINLINE()).addNewCityName());

	//pick teach unit
	int iBestValue = 0;
	setTeachUnitClass(bestTeachUnitClass());
	
	setEverOwned(getOwnerINLINE(), true);

	updateCultureLevel();

	if (pPlot->getCulture(getOwnerINLINE()) < GC.getDefineINT("FREE_CITY_CULTURE"))
	{
		pPlot->setCulture(getOwnerINLINE(), GC.getDefineINT("FREE_CITY_CULTURE"), bBumpUnits);
	}
	pPlot->setOwner(getOwnerINLINE(), bBumpUnits);
	pPlot->setPlotCity(this);

	for (int iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
	{
		pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

		if (pAdjacentPlot != NULL)
		{
			if (pAdjacentPlot->getCulture(getOwnerINLINE()) < GC.getDefineINT("FREE_CITY_ADJACENT_CULTURE"))
			{
				pAdjacentPlot->setCulture(getOwnerINLINE(), GC.getDefineINT("FREE_CITY_ADJACENT_CULTURE"), bBumpUnits);
			}
			pAdjacentPlot->updateCulture(bBumpUnits);
		}
	}

	CyArgsList argsList;
	argsList.add(iX);
	argsList.add(iY);
	long lResult=0;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "citiesDestroyFeatures", argsList.makeFunctionArgs(), &lResult);

	if (lResult == 1)
	{
		if (pPlot->getFeatureType() != NO_FEATURE)
		{
			pPlot->setFeatureType(NO_FEATURE);
		}
	}

	pPlot->setImprovementType(NO_IMPROVEMENT);
	pPlot->updateCityRoute();

	for (int iI = 0; iI < MAX_TEAMS; iI++)
	{
		if (GET_TEAM((TeamTypes)iI).isAlive())
		{
			if (pPlot->isVisible(((TeamTypes)iI), false))
			{
				setRevealed(((TeamTypes)iI), true);
			}
		}
	}

	for (int iI = 0; iI < GC.getNumBuildingInfos(); iI++)
	{
		if (GET_PLAYER(getOwnerINLINE()).isBuildingFree((BuildingTypes)iI))
		{
			setHasFreeBuilding(((BuildingTypes)iI), true);
		}
	}

	area()->changeCitiesPerPlayer(getOwnerINLINE(), 1);

	GET_TEAM(getTeam()).changeNumCities(1);

	GC.getGameINLINE().changeNumCities(1);

	setGameTurnFounded(GC.getGameINLINE().getGameTurn());
	setGameTurnAcquired(GC.getGameINLINE().getGameTurn());

	pPlot->updateYield(false);
	setYieldRateDirty();

	changePopulation(GC.getDefineINT("INITIAL_CITY_POPULATION") + GC.getEraInfo(GC.getGameINLINE().getStartEra()).getFreePopulation());

	GC.getMapINLINE().updateWorkingCity();

	GC.getGameINLINE().AI_makeAssignWorkDirty();

	GET_PLAYER(getOwnerINLINE()).setFoundedFirstCity(true);

	if (GC.getGameINLINE().isFinalInitialized() || isNative())
	{
		for (int iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
		{
			if (GC.getCivilizationInfo(getCivilizationType()).isCivilizationFreeBuildingClass(iI))
			{
				eLoopBuilding = ((BuildingTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iI)));

				if (eLoopBuilding != NO_BUILDING)
				{
					if (GET_PLAYER(getOwnerINLINE()).getNumCities() == 1 || !GC.getBuildingInfo(eLoopBuilding).isCapital())
					{
						setHasRealBuilding(eLoopBuilding, true);
					}
				}
			}
		}

		if (!isHuman())
		{
			changeOverflowProduction(GC.getDefineINT("INITIAL_AI_CITY_PRODUCTION"), 0);
		}
	}
	
	
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		CvPlayerAI& kPlayer = GET_PLAYER((PlayerTypes) i );
		
		if (kPlayer.isAlive())
		{
			kPlayer.AI_invalidateCloseBordersAttitudeCache();
			kPlayer.AI_diplomaticHissyFit(getOwnerINLINE(), kPlayer.AI_getStolenPlotsAttitude(eOwner) - aOldAttitude[i]);
		}
	}

	GET_PLAYER(getOwnerINLINE()).AI_invalidateDistanceMap();
	AI_init();
}


void CvCity::uninit()
{
	SAFE_DELETE_ARRAY(m_paiBuildingProduction);
	SAFE_DELETE_ARRAY(m_paiBuildingProductionTime);
	SAFE_DELETE_ARRAY(m_paiBuildingOriginalOwner);
	SAFE_DELETE_ARRAY(m_paiBuildingOriginalTime);
	SAFE_DELETE_ARRAY(m_paiUnitProduction);
	SAFE_DELETE_ARRAY(m_paiUnitProductionTime);
	SAFE_DELETE_ARRAY(m_aiSpecialistWeights);
	SAFE_DELETE_ARRAY(m_paiUnitCombatFreeExperience);
	SAFE_DELETE_ARRAY(m_paiFreePromotionCount);
	SAFE_DELETE_ARRAY(m_pabHasRealBuilding);
	SAFE_DELETE_ARRAY(m_pabHasFreeBuilding);

	SAFE_DELETE_ARRAY(m_paiWorkingPlot);

	SAFE_DELETE_ARRAY(m_paTradeCities);

	m_orderQueue.clear();

	//free unit population
	for (uint i = 0; i < m_aPopulationUnits.size(); ++i)
	{
		SAFE_DELETE(m_aPopulationUnits[i]);
	}
	m_aPopulationUnits.clear();

	m_setImports.clear();
	m_setExports.clear();
	m_mapMaintain.clear();
}

// FUNCTION: reset()
// Initializes data members that are serialized.
void CvCity::reset(int iID, PlayerTypes eOwner, int iX, int iY, bool bConstructorCall)
{
	int iI;

	//--------------------------------
	// Uninit class
	uninit();

	// PatchMod: Missionary player START
	m_eMissionaryPlayer = NO_PLAYER;
	// PatchMod: Missionary player END

	m_iID = iID;
	m_iX = iX;
	m_iY = iY;
	m_iRallyX = INVALID_PLOT_COORD;
	m_iRallyY = INVALID_PLOT_COORD;
	m_iGameTurnFounded = 0;
	m_iGameTurnAcquired = 0;
	m_iHighestPopulation = 0;
	m_iWorkingPopulation = 0;
	m_iNumBuildings = 0;
	m_iHealRate = 0;
	m_iFoodKept = 0;
	m_iMaxFoodKeptPercent = 0;
	m_iOverflowProduction = 0;
	m_iMilitaryProductionModifier = 0;
	m_iBuildingDefense = 0;
	m_iBuildingBombardDefense = 0;
	m_iFreeExperience = 0;
	m_iDefenseDamage = 0;
	m_iLastDefenseDamage = 0;
	m_iOccupationTimer = 0;
	m_iCultureUpdateTimer = 0;
	m_iCitySizeBoost = 0;
	m_iHammers = 0;
	m_iMissionaryRate = 0;
	m_iWorksWaterCount = 0;
	m_iRebelSentiment = 0;
	m_iTeachUnitMultiplier = 100;

	m_bNeverLost = true;
	m_bBombarded = false;
	m_bProductionAutomated = false;
	m_bWallOverride = false;
	m_bInfoDirty = true;
	m_bLayoutDirty = false;

	m_eOwner = eOwner;
	m_ePreviousOwner = NO_PLAYER;
	m_eOriginalOwner = eOwner;
	m_eCultureLevel = NO_CULTURELEVEL;
	m_eTeachUnitClass = NO_UNITCLASS;
	m_eMissionaryCivilization = NO_CIVILIZATION;

	for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		m_aiSeaPlotYield[iI] = 0;
		m_aiRiverPlotYield[iI] = 0;
		m_aiYieldRateModifier[iI] = 0;
		m_aiYieldStored[iI] = 0;
		m_aiYieldRushed[iI] = 0;
	}

	for (iI = 0; iI < NUM_DOMAIN_TYPES; iI++)
	{
		m_aiDomainFreeExperience[iI] = 0;
		m_aiDomainProductionModifier[iI] = 0;
	}

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		m_aiCulture[iI] = 0;
	}

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		m_abEverOwned[iI] = false;
	}

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		m_abRevealed[iI] = false;
		m_abScoutVisited[iI] = false;
	}

	clear(m_szName);
	m_szScriptData = "";

	m_bPopulationRankValid = false;
	m_iPopulationRank = -1;

	for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		m_abBaseYieldRankValid[iI] = false;
		m_abYieldRankValid[iI] = false;
		m_aiBaseYieldRank[iI] = -1;
		m_aiYieldRank[iI] = -1;
	}

	if (!bConstructorCall)
	{
		FAssertMsg((0 < GC.getNumBuildingInfos()),  "GC.getNumBuildingInfos() is not greater than zero but an array is being allocated in CvCity::reset");
		//m_ppBuildings = new CvBuilding *[GC.getNumBuildingInfos()];
		m_paiBuildingProduction = new int[GC.getNumBuildingInfos()];
		m_paiBuildingProductionTime = new int[GC.getNumBuildingInfos()];
		m_paiBuildingOriginalOwner = new int[GC.getNumBuildingInfos()];
		m_paiBuildingOriginalTime = new int[GC.getNumBuildingInfos()];
		m_pabHasRealBuilding = new bool[GC.getNumBuildingInfos()];
		m_pabHasFreeBuilding = new bool[GC.getNumBuildingInfos()];
		for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
		{
			//m_ppBuildings[iI] = NULL;
			m_paiBuildingProduction[iI] = 0;
			m_paiBuildingProductionTime[iI] = 0;
			m_paiBuildingOriginalOwner[iI] = -1;
			m_paiBuildingOriginalTime[iI] = MIN_INT;
			m_pabHasRealBuilding[iI] = false;
			m_pabHasFreeBuilding[iI] = false;
		}

		FAssertMsg((0 < GC.getNumUnitInfos()),  "GC.getNumUnitInfos() is not greater than zero but an array is being allocated in CvCity::reset");
		m_paiUnitProduction = new int[GC.getNumUnitInfos()];
		m_paiUnitProductionTime = new int[GC.getNumUnitInfos()];
		m_aiSpecialistWeights = new int[GC.getNumUnitInfos()];
		for (iI = 0;iI < GC.getNumUnitInfos();iI++)
		{
			m_paiUnitProduction[iI] = 0;
			m_paiUnitProductionTime[iI] = 0;
			m_aiSpecialistWeights[iI] = 0;
		}

		FAssertMsg((0 < GC.getNumUnitCombatInfos()),  "GC.getNumUnitCombatInfos() is not greater than zero but an array is being allocated in CvCity::reset");
		m_paiUnitCombatFreeExperience = new int[GC.getNumUnitCombatInfos()];
		for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
		{
			m_paiUnitCombatFreeExperience[iI] = 0;
		}

		FAssertMsg((0 < GC.getNumPromotionInfos()),  "GC.getNumPromotionInfos() is not greater than zero but an array is being allocated in CvCity::reset");
		m_paiFreePromotionCount = new int[GC.getNumPromotionInfos()];
		for (iI = 0; iI < GC.getNumPromotionInfos(); iI++)
		{
			m_paiFreePromotionCount[iI] = 0;
		}

		FAssertMsg((0 < NUM_CITY_PLOTS),  "NUM_CITY_PLOTS is not greater than zero but an array is being allocated in CvCity::reset");
		m_paiWorkingPlot = new int[NUM_CITY_PLOTS];
		for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
		{
			m_paiWorkingPlot[iI] = -1;
		}

		m_aEventsOccured.clear();
		m_aBuildingYieldChange.clear();
	}

	if (!bConstructorCall)
	{
		AI_reset();
	}
}


//////////////////////////////////////
// graphical only setup
//////////////////////////////////////
void CvCity::setupGraphical()
{
	if (!GC.IsGraphicsInitialized())
	{
		return;
	}

	CvDLLEntity::setup();

	setBillboardDirty(true);
	setLayoutDirty(true);
}

void CvCity::kill()
{
	CvPlot* pPlot = plot();

	removeTradeRoutes();

	if (isNative())
	{
		std::vector<CvUnit*> aUnits;
		CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();
		while (pUnitNode)
		{
			CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = plot()->nextUnitNode(pUnitNode);

			if (pLoopUnit->getUnitTravelState() == UNIT_TRAVEL_STATE_LIVE_AMONG_NATIVES)
			{
				aUnits.push_back(pLoopUnit);
			}
		}

		for (uint i = 0; i < aUnits.size(); ++i)
		{
			aUnits[i]->kill(false);
		}
	}

	if (isCitySelected())
	{
		gDLL->getInterfaceIFace()->clearSelectedCities();
	}

	for (int iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		CvPlot* pLoopPlot = getCityIndexPlot(iI);

		if (pLoopPlot != NULL)
		{
			if (pLoopPlot->getWorkingCityOverride() == this)
			{
				pLoopPlot->setWorkingCityOverride(NULL);
			}
		}
	}

	setCultureLevel(NO_CULTURELEVEL);

	for (int iI = 0; iI < GC.getNumBuildingInfos(); iI++)
	{
		setHasRealBuilding(((BuildingTypes)iI), false);
		setHasFreeBuilding(((BuildingTypes)iI), false);
	}

	for (int i = 0; i < NUM_YIELD_TYPES; ++i)
	{
		setYieldStored((YieldTypes) i, 0);
	}

	for (uint i = 0; i < m_aPopulationUnits.size(); ++i)
	{
		m_aPopulationUnits[i]->setProfession(NO_PROFESSION);
		m_aPopulationUnits[i]->updateOwnerCache(-1);
	}

	clearOrderQueue();

	//remove home plot yields
	setYieldRateDirty();

	pPlot->setPlotCity(NULL);

	area()->changeCitiesPerPlayer(getOwnerINLINE(), -1);

	GET_TEAM(getTeam()).changeNumCities(-1);

	GC.getGameINLINE().changeNumCities(-1);

	FAssert(!isProduction());

	PlayerTypes eOwner = getOwnerINLINE();

	pPlot->setImprovementType((ImprovementTypes)(GC.getDefineINT("RUINS_IMPROVEMENT")));

	gDLL->getEventReporterIFace()->cityLost(this);

	GET_PLAYER(getOwnerINLINE()).AI_invalidateDistanceMap();
	GET_PLAYER(getOwnerINLINE()).deleteCity(getID());

	pPlot->updateCulture(true);

	for (int iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
	{
		CvPlot* pAdjacentPlot = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), ((DirectionTypes)iI));

		if (pAdjacentPlot != NULL)
		{
			pAdjacentPlot->updateCulture(true);
		}
	}

	GC.getMapINLINE().updateWorkingCity();

	GC.getGameINLINE().AI_makeAssignWorkDirty();

	if (eOwner == GC.getGameINLINE().getActivePlayer())
	{
		gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
	}
}


void CvCity::doTurn()
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	int iI;

	if (!isBombarded())
	{
		changeDefenseDamage(-(GC.getDefineINT("CITY_DEFENSE_DAMAGE_HEAL_RATE")));
	}

	setLastDefenseDamage(getDefenseDamage());
	setBombarded(false);

	AI_doTurn();

	bool bAllowNoProduction = !doCheckProduction();

	doSpecialists();

	// PatchMod: Resource depletion bugfix START
	doProduction(bAllowNoProduction);
	// PatchMod: Resource depletion bugfix END

	doYields();

	doGrowth();

	doCulture();

	doPlotCulture(false, getOwnerINLINE(), getCultureRate());

	// PatchMod: Resource depletion bugfix START
//	doProduction(bAllowNoProduction);
	// PatchMod: Resource depletion bugfix END

	doDecay();

	doMissionaries();

	doRebelSentiment();

	if (!isDisorder())
	{
		for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
		{
			pLoopPlot = getCityIndexPlot(iI);

			if (pLoopPlot != NULL)
			{
				if (pLoopPlot->getWorkingCity() == this)
				{
					if (pLoopPlot->isBeingWorked())
					{
						pLoopPlot->doImprovement();
					}
				}
			}
		}
	}

	if (getCultureUpdateTimer() > 0)
	{
		changeCultureUpdateTimer(-1);
	}

	if (getOccupationTimer() > 0)
	{
		changeOccupationTimer(-1);
	}

	for (uint i = 0; i < m_aPopulationUnits.size(); ++i)
	{
		m_aPopulationUnits[i]->doTurn();
	}

	// ONEVENT - Do turn
	gDLL->getEventReporterIFace()->cityDoTurn(this, getOwnerINLINE());
}


bool CvCity::isCitySelected() const
{
	return gDLL->getInterfaceIFace()->isCitySelected(this);
}


bool CvCity::canBeSelected() const
{
	if ((getTeam() == GC.getGameINLINE().getActiveTeam()) || GC.getGameINLINE().isDebugMode())
	{
		return true;
	}

	return false;
}


void CvCity::updateSelectedCity()
{
	CvPlot* pLoopPlot;
	int iI;

	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		pLoopPlot = getCityIndexPlot(iI);

		if (pLoopPlot != NULL)
		{
			pLoopPlot->updateShowCitySymbols();
		}
	}
}


void CvCity::updateYield()
{
	for (int iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		CvPlot* pLoopPlot = getCityIndexPlot(iI);

		if (pLoopPlot != NULL)
		{
			pLoopPlot->updateYield(true);
		}
	}
}


// XXX kill this?
void CvCity::updateVisibility()
{
	PROFILE_FUNC();

	if (!GC.IsGraphicsInitialized())
	{
		return;
	}

	FAssert(GC.getGameINLINE().getActiveTeam() != NO_TEAM);

	CvDLLEntity::setVisible(isRevealed(GC.getGameINLINE().getActiveTeam(), true));
}


void CvCity::createGreatGeneral(UnitTypes eGreatGeneralUnit, bool bIncrementExperience)
{
	GET_PLAYER(getOwnerINLINE()).createGreatGeneral(eGreatGeneralUnit, bIncrementExperience, getX_INLINE(), getY_INLINE());
}


void CvCity::doTask(TaskTypes eTask, int iData1, int iData2, bool bOption, bool bAlt, bool bShift, bool bCtrl)
{
	switch (eTask)
	{
	case TASK_RAZE:
		GET_PLAYER(getOwnerINLINE()).raze(this);
		break;

	case TASK_GIFT:
		ejectTeachUnits();
		if (isNative() && !GET_PLAYER((PlayerTypes)iData1).isNative())
		{
			for (int i = 0; i < NUM_CITY_PLOTS; ++i)
			{
				CvPlot* pLoopPlot = ::plotCity(getX_INLINE(), getY_INLINE(), i);
				if (pLoopPlot != NULL)
				{
					pLoopPlot->setCulture(getOwnerINLINE(), 0, false);
				}
			}

			while (getPopulation() > 1)
			{
				if (!AI_removeWorstPopulationUnit(false))
				{
					FAssertMsg(false, "Could not eject unit");
					break;
				}
			}

			GET_PLAYER(getOwnerINLINE()).disband(this);
		}
		else
		{
			GET_PLAYER((PlayerTypes)iData1).acquireCity(this, false, true);
		}
		break;

	case TASK_LIBERATE:
		ejectTeachUnits();
		liberate(iData1 != 0);
		break;

	case TASK_SET_ALL_CITIZENS_AUTOMATED:
		setAllCitizensAutomated(iData1 != 0);
		break;

	case TASK_SET_CITIZEN_AUTOMATED:
		setCitizenAutomated(iData1);
		break;

	case TASK_SET_AUTOMATED_PRODUCTION:
		setProductionAutomated(bOption, bAlt && bShift && bCtrl);
		break;

	case TASK_SET_EMPHASIZE:
		AI_setEmphasize(((EmphasizeTypes)iData1), bOption);
		break;

	case TASK_CHANGE_WORKING_PLOT:
		alterUnitWorkingPlot(iData1, iData2, bOption);
		break;

	case TASK_CHANGE_PROFESSION:
		alterUnitProfession(iData1, (ProfessionTypes) iData2);
		break;

	case TASK_EJECT_TO_TRANSPORT:
		ejectToTransport(iData1, iData2);
		break;

	case TASK_REPLACE_CITIZEN:
		replaceCitizen(iData1, iData2, bOption);
		break;

	case TASK_CLEAR_WORKING_OVERRIDE:
		clearWorkingOverride(iData1);
		break;

	case TASK_HURRY:
		hurry((HurryTypes)iData1);
		break;

	case TASK_CLEAR_ORDERS:
		clearOrderQueue();
		break;

	case TASK_RALLY_PLOT:
		setRallyPlot(GC.getMapINLINE().plotINLINE(iData1, iData2));
		break;

	case TASK_CLEAR_RALLY_PLOT:
		setRallyPlot(NULL);
		break;

	case TASK_YIELD_IMPORT:
		if (iData2 != 0)
		{
			addImport((YieldTypes) iData1);
		}
		else
		{
			removeImport((YieldTypes) iData1);
		}
		break;

	case TASK_YIELD_EXPORT:
		if (iData2 != 0)
		{
			addExport((YieldTypes) iData1);
		}
		else
		{
			removeExport((YieldTypes) iData1);
		}
		break;

	case TASK_YIELD_LEVEL:
		setMaintainLevel((YieldTypes) iData1, iData2);
		break;

	case TASK_CLEAR_SPECIALTY:
		{
			CvUnit* pUnit = GET_PLAYER(getOwnerINLINE()).getUnit(iData1);
			if (pUnit == NULL)
			{
				pUnit = getPopulationUnitById(iData1);
			}

			if (pUnit != NULL)
			{
				pUnit->clearSpecialty();
			}
		}
		break;

	case TASK_EDUCATE:
		educateStudent(iData1, (UnitTypes) iData2);
		break;

	case TASK_PUSH_TRAIN_UNIT:
		if (canTrain((UnitTypes) iData1))
		{
			pushOrder(ORDER_TRAIN, iData1, iData2, false, false, false);
			if (checkRequiredYields(ORDER_TRAIN, iData1) && getProduction() >= getProductionNeeded(YIELD_HAMMERS))
			{
				popOrder(0, true, false);
			}
		}
		break;

	case TASK_PUSH_CONSTRUCT_BUILDING:
		if (canConstruct((BuildingTypes) iData1))
		{
			pushOrder(ORDER_CONSTRUCT, iData1, iData2, false, false, false);
			if (checkRequiredYields(ORDER_CONSTRUCT, iData1) && getProduction() >= getProductionNeeded(YIELD_HAMMERS))
			{
				popOrder(0, true, false);
			}
		}
		break;

	case TASK_CHEAT:
		doCheat(bAlt, bShift, bCtrl);
		break;

	default:
		FAssertMsg(false, "eTask failed to match a valid option");
		break;
	}
}


void CvCity::chooseProduction(UnitTypes eTrainUnit, BuildingTypes eConstructBuilding, bool bFinish, bool bFront)
{
	CvPopupInfo* pPopupInfo = new CvPopupInfo(BUTTONPOPUP_CHOOSEPRODUCTION);
	pPopupInfo->setData1(getID());
	pPopupInfo->setOption1(bFinish);

	if (eTrainUnit != NO_UNIT)
	{
		pPopupInfo->setData2(ORDER_TRAIN);
		pPopupInfo->setData3(eTrainUnit);
	}
	else if (eConstructBuilding != NO_BUILDING)
	{
		pPopupInfo->setData2(ORDER_CONSTRUCT);
		pPopupInfo->setData3(eConstructBuilding);
	}
	else
	{
		pPopupInfo->setData2(NO_ORDER);
		pPopupInfo->setData3(NO_UNIT);
	}

	gDLL->getInterfaceIFace()->addPopup(pPopupInfo, getOwnerINLINE(), false, bFront);
}


int CvCity::getCityPlotIndex(const CvPlot* pPlot) const
{
	return plotCityXY(this, pPlot);
}


CvPlot* CvCity::getCityIndexPlot(int iIndex) const
{
	return plotCity(getX_INLINE(), getY_INLINE(), iIndex);
}


bool CvCity::canWork(const CvPlot* pPlot) const
{
	if (pPlot == NULL)
	{
		return false;
	}

	if (pPlot->getWorkingCity() != this)
	{
		return false;
	}

	FAssertMsg(getCityPlotIndex(pPlot) != -1, "getCityPlotIndex(pPlot) is expected to be assigned (not -1)");

	if (pPlot->plotCheck(PUF_canSiege, getOwnerINLINE()) != NULL)
	{
		return false;
	}

	if (pPlot->isWater())
	{
		if (!isWorksWater() && !GC.getCivilizationInfo(getCivilizationType()).isWaterWorks())
		{
			return false;
		}
	}

	if (!(pPlot->hasYield()))
	{
		return false;
	}

	return true;
}


void CvCity::verifyWorkingPlot(int iPlotIndex)
{
	FAssert(iPlotIndex >= 0 && iPlotIndex < NUM_CITY_PLOTS);

	CvUnit* pUnit = getUnitWorkingPlot(iPlotIndex);
	if (pUnit != NULL)
	{
		CvPlot* pPlot = getCityIndexPlot(iPlotIndex);

		if (pPlot != NULL)
		{
			bool bCanWork = canWork(pPlot);

			if (!bCanWork)
			{
				pUnit->setColonistLocked(false);
			}

			//check if we have outdoor profession
			if (!bCanWork || NO_PROFESSION == pUnit->getProfession() || !GC.getProfessionInfo(pUnit->getProfession()).isWorkPlot())
			{
				clearUnitWorkingPlot(iPlotIndex);
				AI_setAssignWorkDirty(true);
			}
		}
	}
}


void CvCity::verifyWorkingPlots()
{
	for (int iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		verifyWorkingPlot(iI);
	}
}

int CvCity::getNumAvailableWorkPlots() const
{
	int iNumCanWorkPlots = 0;

	for (int iJ = 0; iJ < NUM_CITY_PLOTS; iJ++)
	{
		if (iJ != CITY_HOME_PLOT)
		{
			if (!isUnitWorkingPlot(iJ))
			{
				CvPlot* pLoopPlot = getCityIndexPlot(iJ);

				if (pLoopPlot != NULL)
				{
					if (canWork(pLoopPlot))
					{
						++iNumCanWorkPlots;
					}
				}
			}
		}
	}

	return iNumCanWorkPlots;
}

void CvCity::clearWorkingOverride(int iIndex)
{
	CvPlot* pPlot;

	pPlot = getCityIndexPlot(iIndex);

	if (pPlot != NULL)
	{
		pPlot->setWorkingCityOverride(NULL);
	}
}


int CvCity::countNumImprovedPlots(ImprovementTypes eImprovement, bool bPotential) const
{
	CvPlot* pLoopPlot;
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		pLoopPlot = getCityIndexPlot(iI);

		if (pLoopPlot != NULL)
		{
			if (pLoopPlot->getWorkingCity() == this)
			{
				if (eImprovement != NO_IMPROVEMENT)
				{
					if (pLoopPlot->getImprovementType() == eImprovement ||
						(bPotential && pLoopPlot->canHaveImprovement(eImprovement, getTeam())))
					{
						++iCount;
					}
				}
				else if (pLoopPlot->getImprovementType() != NO_IMPROVEMENT)
				{
					iCount++;
				}
			}
		}
	}

	return iCount;
}


int CvCity::countNumWaterPlots() const
{
	CvPlot* pLoopPlot;
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		pLoopPlot = getCityIndexPlot(iI);

		if (pLoopPlot != NULL)
		{
			if (pLoopPlot->getWorkingCity() == this)
			{
				if (pLoopPlot->isWater())
				{
					iCount++;
				}
			}
		}
	}

	return iCount;
}

int CvCity::countNumRiverPlots() const
{
	int iCount = 0;

	for (int iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		CvPlot* pLoopPlot = getCityIndexPlot(iI);

		if (pLoopPlot != NULL)
		{
			if (pLoopPlot->getWorkingCity() == this)
			{
				if (pLoopPlot->isRiver())
				{
					++iCount;
				}
			}
		}
	}

	return iCount;
}

int CvCity::getNumProfessionBuildingSlots(ProfessionTypes eProfession) const
{
	FAssert(!GC.getProfessionInfo(eProfession).isWorkPlot());
	CvProfessionInfo& kProfessionInfo = GC.getProfessionInfo(eProfession);
	SpecialBuildingTypes eSpecialBuilding = (SpecialBuildingTypes) kProfessionInfo.getSpecialBuilding();
	if (eSpecialBuilding == NO_SPECIALBUILDING)
	{
		return 0;
	}

	int iTotalSlots = 0;
	for (int i = 0; i < GC.getNumBuildingInfos(); ++i)
	{
		BuildingTypes eBuilding = (BuildingTypes) i;
		CvBuildingInfo& kBuilding = GC.getBuildingInfo(eBuilding);
		if (kBuilding.getSpecialBuildingType() == eSpecialBuilding)
		{
			if (isHasBuilding(eBuilding))
			{
				iTotalSlots += kBuilding.getMaxWorkers();
			}
		}
	}

	return iTotalSlots;
}

bool CvCity::isAvailableProfessionSlot(ProfessionTypes eProfession, const CvUnit* pUnit) const
{
	if (GC.getProfessionInfo(eProfession).isCitizen())
	{
		if (GC.getProfessionInfo(eProfession).isWorkPlot())
		{
			if (!isUnitWorkingAnyPlot(pUnit) && getNumAvailableWorkPlots() == 0)
			{
				return false;
			}
		}
		else
		{
			int iSlots = getNumProfessionBuildingSlots(eProfession);

			for (uint i = 0; i < m_aPopulationUnits.size() && iSlots > 0; ++i)
			{
				CvUnit* pLoopUnit = m_aPopulationUnits[i];
				if (pUnit != pLoopUnit)
				{
					if (pLoopUnit->getProfession() == eProfession)
					{
						--iSlots;
					}
				}
			}

			if (iSlots <= 0)
			{
				FAssert(iSlots == 0);
				return false;
			}
		}
	}

	return true;
}

int CvCity::professionCount(ProfessionTypes eProfession) const
{
	int iCount = 0;
	for (uint i = 0; i < m_aPopulationUnits.size(); ++i)
	{
		CvUnit* pLoopUnit = m_aPopulationUnits[i];
		if (pLoopUnit->getProfession() == eProfession)
		{
			++iCount;
		}
	}
	return iCount;
}

int CvCity::findPopulationRank() const
{
	if (!m_bPopulationRankValid)
	{
		int iRank = 1;

		int iLoop;
		CvCity* pLoopCity;
		for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
		{
			if ((pLoopCity->getPopulation() > getPopulation()) ||
				((pLoopCity->getPopulation() == getPopulation()) && (pLoopCity->getID() < getID())))
			{
				iRank++;
			}
		}

		// shenanigans are to get around the const check
		m_bPopulationRankValid = true;
		m_iPopulationRank = iRank;
	}

	return m_iPopulationRank;
}


int CvCity::findBaseYieldRateRank(YieldTypes eYield) const
{
	if (!m_abBaseYieldRankValid[eYield])
	{
		int iRate = getYieldRate(eYield);

		int iRank = 1;

		int iLoop;
		CvCity* pLoopCity;
		for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
		{
			int iYieldRate = pLoopCity->getYieldRate(eYield);
			if ((iYieldRate > iRate) || ((iYieldRate == iRate) && (pLoopCity->getID() < getID())))
			{
				iRank++;
			}
		}

		m_abBaseYieldRankValid[eYield] = true;
		m_aiBaseYieldRank[eYield] = iRank;
	}

	return m_aiBaseYieldRank[eYield];
}


int CvCity::findYieldRateRank(YieldTypes eYield) const
{
	if (!m_abYieldRankValid[eYield])
	{
		int iRate = getYieldRate(eYield);

		int iRank = 1;

		int iLoop;
		CvCity* pLoopCity;
		for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
		{
			if ((pLoopCity->getYieldRate(eYield) > iRate) ||
				((pLoopCity->getYieldRate(eYield) == iRate) && (pLoopCity->getID() < getID())))
			{
				iRank++;
			}
		}

		m_abYieldRankValid[eYield] = true;
		m_aiYieldRank[eYield] = iRank;
	}

	return m_aiYieldRank[eYield];
}


// Returns one of the upgrades...
UnitTypes CvCity::allUpgradesAvailable(UnitTypes eUnit, int iUpgradeCount) const
{
	UnitTypes eUpgradeUnit;
	UnitTypes eTempUnit;
	UnitTypes eLoopUnit;
	bool bUpgradeFound;
	bool bUpgradeAvailable;
	bool bUpgradeUnavailable;
	int iI;

	FAssertMsg(eUnit != NO_UNIT, "eUnit is expected to be assigned (not NO_UNIT)");

	if (iUpgradeCount > GC.getNumUnitClassInfos())
	{
		return NO_UNIT;
	}

	eUpgradeUnit = NO_UNIT;

	bUpgradeFound = false;
	bUpgradeAvailable = false;
	bUpgradeUnavailable = false;

	for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
	{
		if (GC.getUnitInfo(eUnit).getUpgradeUnitClass(iI))
		{
			eLoopUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI);

			if (eLoopUnit != NO_UNIT)
			{
				bUpgradeFound = true;

				eTempUnit = allUpgradesAvailable(eLoopUnit, (iUpgradeCount + 1));

				if (eTempUnit != NO_UNIT)
				{
					eUpgradeUnit = eTempUnit;
					bUpgradeAvailable = true;
				}
				else
				{
					bUpgradeUnavailable = true;
				}
			}
		}
	}

	if (iUpgradeCount > 0)
	{
		if (bUpgradeFound && bUpgradeAvailable)
		{
			FAssertMsg(eUpgradeUnit != NO_UNIT, "eUpgradeUnit is expected to be assigned (not NO_UNIT)");
			return eUpgradeUnit;
		}

		if (canTrain(eUnit, false, false, false, true))
		{
			return eUnit;
		}
	}
	else
	{
		if (bUpgradeFound && !bUpgradeUnavailable)
		{
			return eUpgradeUnit;
		}
	}

	return NO_UNIT;
}

bool CvCity::canTrain(UnitTypes eUnit, bool bContinue, bool bTestVisible, bool bIgnoreCost, bool bIgnoreUpgrades) const
{
	if (eUnit == NO_UNIT)
	{
		return false;
	}

	if(GC.getUSE_CAN_TRAIN_CALLBACK())
	{
		CyCity* pyCity = new CyCity((CvCity*)this);
		CyArgsList argsList;
		argsList.add(gDLL->getPythonIFace()->makePythonObject(pyCity));	// pass in city class
		argsList.add(eUnit);
		argsList.add(bContinue);
		argsList.add(bTestVisible);
		argsList.add(bIgnoreCost);
		argsList.add(bIgnoreUpgrades);
		long lResult=0;
		gDLL->getPythonIFace()->callFunction(PYGameModule, "canTrain", argsList.makeFunctionArgs(), &lResult);
		delete pyCity;	// python fxn must not hold on to this pointer
		if (lResult == 1)
		{
			return true;
		}
	}

	if (!(GET_PLAYER(getOwnerINLINE()).canTrain(eUnit, bContinue, bTestVisible, bIgnoreCost)))
	{
		return false;
	}

	if (!bIgnoreUpgrades)
	{
		if (allUpgradesAvailable(eUnit) != NO_UNIT)
		{
			return false;
		}
	}

	if (!plot()->canTrain(eUnit, bContinue, bTestVisible))
	{
		return false;
	}

	if(GC.getUSE_CANNOT_TRAIN_CALLBACK())
	{
		CyCity *pyCity = new CyCity((CvCity*)this);
		CyArgsList argsList2; // XXX
		argsList2.add(gDLL->getPythonIFace()->makePythonObject(pyCity));	// pass in city class
		argsList2.add(eUnit);
		argsList2.add(bContinue);
		argsList2.add(bTestVisible);
		argsList2.add(bIgnoreCost);
		argsList2.add(bIgnoreUpgrades);
		long lResult=0;
		gDLL->getPythonIFace()->callFunction(PYGameModule, "cannotTrain", argsList2.makeFunctionArgs(), &lResult);
		delete pyCity;	// python fxn must not hold on to this pointer
		if (lResult == 1)
		{
			return false;
		}
	}

	return true;
}

bool CvCity::canTrain(UnitCombatTypes eUnitCombat) const
{
	for (int i = 0; i < GC.getNumUnitClassInfos(); i++)
	{
		UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(GET_PLAYER(getOwnerINLINE()).getCivilizationType()).getCivilizationUnits(i);

		if (NO_UNIT != eUnit)
		{
			if (GC.getUnitInfo(eUnit).getUnitCombatType() == eUnitCombat)
			{
				if (canTrain(eUnit))
				{
					return true;
				}
			}
		}
	}

	return false;
}


bool CvCity::canConstruct(BuildingTypes eBuilding, bool bContinue, bool bTestVisible, bool bIgnoreCost) const
{
	if (eBuilding == NO_BUILDING)
	{
		return false;
	}

	CvBuildingInfo& kBuilding = GC.getBuildingInfo(eBuilding);

	if(GC.getUSE_CAN_CONSTRUCT_CALLBACK())
	{
		CyCity* pyCity = new CyCity((CvCity*)this);
		CyArgsList argsList;
		argsList.add(gDLL->getPythonIFace()->makePythonObject(pyCity));	// pass in city class
		argsList.add(eBuilding);
		argsList.add(bContinue);
		argsList.add(bTestVisible);
		argsList.add(bIgnoreCost);
		long lResult=0;
		gDLL->getPythonIFace()->callFunction(PYGameModule, "canConstruct", argsList.makeFunctionArgs(), &lResult);
		delete pyCity;	// python fxn must not hold on to this pointer
		if (lResult == 1)
		{
			return true;
		}
	}

	if (!(GET_PLAYER(getOwnerINLINE()).canConstruct(eBuilding, bContinue, bTestVisible, bIgnoreCost)))
	{
		return false;
	}

	if (kBuilding.isGraphicalOnly())
	{
		return false;
	}

	if (isHasConceptualBuilding(eBuilding))
	{
		return false;
	}

	if (!isDominantSpecialBuilding(eBuilding))
	{
		return false;
	}

	if (!isValidBuildingLocation(eBuilding))
	{
		return false;
	}

	if (plot()->getLatitude() > kBuilding.getMaxLatitude())
	{
		return false;
	}

	if (plot()->getLatitude() < kBuilding.getMinLatitude())
	{
		return false;
	}

	for (int iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
	{
		if (kBuilding.isBuildingClassNeededInCity(iI))
		{
			BuildingTypes ePrereqBuilding = ((BuildingTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iI)));

			if (ePrereqBuilding != NO_BUILDING)
			{
				if (!isHasConceptualBuilding(ePrereqBuilding))
				{
					return false;
				}
			}
		}
	}

	if (!bTestVisible)
	{
		if (!bContinue)
		{
			if (getFirstBuildingOrder(eBuilding) != -1)
			{
				return false;
			}
		}
	}

	if(GC.getUSE_CANNOT_CONSTRUCT_CALLBACK())
	{
		CyCity *pyCity = new CyCity((CvCity*)this);
		CyArgsList argsList2; // XXX
		argsList2.add(gDLL->getPythonIFace()->makePythonObject(pyCity));	// pass in city class
		argsList2.add(eBuilding);
		argsList2.add(bContinue);
		argsList2.add(bTestVisible);
		argsList2.add(bIgnoreCost);
		long lResult=0;
		gDLL->getPythonIFace()->callFunction(PYGameModule, "cannotConstruct", argsList2.makeFunctionArgs(), &lResult);
		delete pyCity;	// python fxn must not hold on to this pointer
		if (lResult == 1)
		{
			return false;
		}
	}

	return true;
}

bool CvCity::canConvince(FatherPointTypes eFatherPoint, bool bContinue, bool bTestVisible) const
{
	if (isNative())
	{
		return false;
	}

	if (GC.getFatherPointInfo(eFatherPoint).getProductionConversionPoints() <= 0)
	{
		return false;
	}

	std::vector<FatherTypes> aFathers;
	GC.getGameINLINE().getRemainingFathers(eFatherPoint, aFathers);
	if (aFathers.empty())
	{
		return false;
	}

	return true;
}


int CvCity::getFoodTurnsLeft() const
{
	int iFoodLeft;
	int iTurnsLeft;

	iFoodLeft = (growthThreshold() - getFood());

	if (foodDifference() <= 0)
	{
		return iFoodLeft;
	}

	iTurnsLeft = (iFoodLeft / foodDifference());

	if ((iTurnsLeft * foodDifference()) <  iFoodLeft)
	{
		iTurnsLeft++;
	}

	return std::max(1, iTurnsLeft);
}

bool CvCity::isProduction() const
{
	return (headOrderQueueNode() != NULL);
}

bool CvCity::isProductionUnit() const
{
	CLLNode<OrderData>* pOrderNode = headOrderQueueNode();

	if (pOrderNode != NULL)
	{
		return (pOrderNode->m_data.eOrderType == ORDER_TRAIN);
	}

	return false;
}


bool CvCity::isProductionBuilding() const
{
	CLLNode<OrderData>* pOrderNode = headOrderQueueNode();

	if (pOrderNode != NULL)
	{
		return (pOrderNode->m_data.eOrderType == ORDER_CONSTRUCT);
	}

	return false;
}

bool CvCity::isProductionConvince() const
{
	CLLNode<OrderData>* pOrderNode = headOrderQueueNode();

	if (pOrderNode != NULL)
	{
		return (pOrderNode->m_data.eOrderType == ORDER_CONVINCE);
	}

	return false;
}


bool CvCity::canContinueProduction(OrderData order) const
{
	switch (order.eOrderType)
	{
	case ORDER_TRAIN:
		return canTrain((UnitTypes)(order.iData1), true);
		break;

	case ORDER_CONSTRUCT:
		return canConstruct((BuildingTypes)(order.iData1), true);
		break;

	case ORDER_CONVINCE:
		return canConvince((FatherPointTypes) order.iData1, true);
		break;

	default:
		FAssertMsg(false, "order.eOrderType failed to match a valid option");
		break;
	}

	return false;
}


int CvCity::getProductionExperience(UnitTypes eUnit) const
{
	int iExperience;

	iExperience = getFreeExperience();
	iExperience += GET_PLAYER(getOwnerINLINE()).getFreeExperience();

	if (eUnit != NO_UNIT)
	{
		if (GC.getUnitInfo(eUnit).getUnitCombatType() != NO_UNITCOMBAT)
		{
			iExperience += getUnitCombatFreeExperience((UnitCombatTypes)(GC.getUnitInfo(eUnit).getUnitCombatType()));
		}
		iExperience += getDomainFreeExperience((DomainTypes)(GC.getUnitInfo(eUnit).getDomainType()));
	}

	return std::max(0, iExperience);
}


void CvCity::addProductionExperience(CvUnit* pUnit)
{
	if (pUnit->canAcquirePromotionAny())
	{
		pUnit->changeExperience(getProductionExperience(pUnit->getUnitType()));
	}

	for (int iI = 0; iI < GC.getNumPromotionInfos(); iI++)
	{
		if (isFreePromotion((PromotionTypes)iI))
		{
			if ((pUnit->getUnitCombatType() != NO_UNITCOMBAT) && GC.getPromotionInfo((PromotionTypes)iI).getUnitCombat(pUnit->getUnitCombatType()))
			{
				pUnit->setHasRealPromotion(((PromotionTypes)iI), true);
			}
		}
	}

	pUnit->testPromotionReady();
}


UnitTypes CvCity::getProductionUnit() const
{
	CLLNode<OrderData>* pOrderNode = headOrderQueueNode();

	if (pOrderNode != NULL)
	{
		switch (pOrderNode->m_data.eOrderType)
		{
		case ORDER_TRAIN:
			return ((UnitTypes)(pOrderNode->m_data.iData1));
			break;

		case ORDER_CONSTRUCT:
		case ORDER_CONVINCE:
			break;

		default:
			FAssertMsg(false, "pOrderNode->m_data.eOrderType failed to match a valid option");
			break;
		}
	}

	return NO_UNIT;
}


UnitAITypes CvCity::getProductionUnitAI() const
{
	CLLNode<OrderData>* pOrderNode = headOrderQueueNode();

	if (pOrderNode != NULL)
	{
		switch (pOrderNode->m_data.eOrderType)
		{
		case ORDER_TRAIN:
			return ((UnitAITypes)(pOrderNode->m_data.iData2));
			break;

		case ORDER_CONSTRUCT:
		case ORDER_CONVINCE:
			break;

		default:
			FAssertMsg(false, "pOrderNode->m_data.eOrderType failed to match a valid option");
			break;
		}
	}

	return NO_UNITAI;
}


BuildingTypes CvCity::getProductionBuilding() const
{
	CLLNode<OrderData>* pOrderNode = headOrderQueueNode();

	if (pOrderNode != NULL)
	{
		switch (pOrderNode->m_data.eOrderType)
		{
		case ORDER_TRAIN:
		case ORDER_CONVINCE:
			break;

		case ORDER_CONSTRUCT:
			return ((BuildingTypes)(pOrderNode->m_data.iData1));
			break;

		default:
			FAssertMsg(false, "pOrderNode->m_data.eOrderType failed to match a valid option");
			break;
		}
	}

	return NO_BUILDING;
}

FatherPointTypes CvCity::getProductionFatherPoint() const
{
	CLLNode<OrderData>* pOrderNode = headOrderQueueNode();

	if (pOrderNode != NULL)
	{
		switch (pOrderNode->m_data.eOrderType)
		{
		case ORDER_TRAIN:
		case ORDER_CONSTRUCT:
			break;

		case ORDER_CONVINCE:
			return ((FatherPointTypes)(pOrderNode->m_data.iData1));
			break;

		default:
			FAssertMsg(false, "pOrderNode->m_data.eOrderType failed to match a valid option");
			break;
		}
	}

	return NO_FATHER_POINT_TYPE;
}

const CvInfoBase* CvCity::getProductionInfo() const
{
	CLLNode<OrderData>* pOrderNode = headOrderQueueNode();

	if (pOrderNode != NULL)
	{
		switch (pOrderNode->m_data.eOrderType)
		{
		case ORDER_TRAIN:
			return &GC.getUnitInfo((UnitTypes) pOrderNode->m_data.iData1);
			break;

		case ORDER_CONSTRUCT:
			return &GC.getBuildingInfo((BuildingTypes) pOrderNode->m_data.iData1);
			break;

		case ORDER_CONVINCE:
			return &GC.getFatherPointInfo((FatherPointTypes) pOrderNode->m_data.iData1);
			break;

		default:
			FAssertMsg(false, "pOrderNode->m_data.eOrderType failed to match a valid option");
			break;
		}
	}

	return NULL;
}


const wchar* CvCity::getProductionName() const
{
	const CvInfoBase* pInfo = getProductionInfo();
	if (pInfo != NULL)
	{
		return pInfo->getDescription();
	}

	return L"";
}


int CvCity::getGeneralProductionTurnsLeft() const
{
	CLLNode<OrderData>* pOrderNode = headOrderQueueNode();

	if (pOrderNode != NULL)
	{
		switch (pOrderNode->m_data.eOrderType)
		{
		case ORDER_TRAIN:
			return getProductionTurnsLeft((UnitTypes)pOrderNode->m_data.iData1, 0);
			break;

		case ORDER_CONSTRUCT:
			return getProductionTurnsLeft((BuildingTypes)pOrderNode->m_data.iData1, 0);
			break;

		case ORDER_CONVINCE:
			return 0;
			break;

		default:
			FAssertMsg(false, "pOrderNode->m_data.eOrderType failed to match a valid option");
			break;
		}
	}

	return 0;
}

const wchar* CvCity::getProductionNameKey() const
{
	const CvInfoBase* pInfo = getProductionInfo();
	if (pInfo != NULL)
	{
		return pInfo->getTextKeyWide();
	}

	return L"";
}

int CvCity::getFirstUnitOrder(UnitTypes eUnit) const
{
	int iCount = 0;

	CLLNode<OrderData>* pOrderNode = headOrderQueueNode();

	while (pOrderNode != NULL)
	{
		if (pOrderNode->m_data.eOrderType == ORDER_TRAIN)
		{
			if (pOrderNode->m_data.iData1 == eUnit)
			{
				return iCount;
			}
		}

		iCount++;

		pOrderNode = nextOrderQueueNode(pOrderNode);
	}

	return -1;
}


int CvCity::getFirstBuildingOrder(BuildingTypes eBuilding) const
{
	int iCount = 0;

	CLLNode<OrderData>* pOrderNode = headOrderQueueNode();

	while (pOrderNode != NULL)
	{
		if (pOrderNode->m_data.eOrderType == ORDER_CONSTRUCT)
		{
			if (pOrderNode->m_data.iData1 == eBuilding)
			{
				return iCount;
			}
		}

		iCount++;

		pOrderNode = nextOrderQueueNode(pOrderNode);
	}

	return -1;
}

int CvCity::getNumTrainUnitAI(UnitAITypes eUnitAI) const
{
	int iCount = 0;

	CLLNode<OrderData>* pOrderNode = headOrderQueueNode();

	while (pOrderNode != NULL)
	{
		if (pOrderNode->m_data.eOrderType == ORDER_TRAIN)
		{
			if (pOrderNode->m_data.iData2 == eUnitAI)
			{
				iCount++;
			}
		}

		pOrderNode = nextOrderQueueNode(pOrderNode);
	}

	return iCount;
}


int CvCity::getProduction() const
{
	CLLNode<OrderData>* pOrderNode = headOrderQueueNode();

	if (pOrderNode != NULL)
	{
		switch (pOrderNode->m_data.eOrderType)
		{
		case ORDER_TRAIN:
			return getUnitProduction((UnitTypes)(pOrderNode->m_data.iData1));
			break;

		case ORDER_CONSTRUCT:
			return getBuildingProduction((BuildingTypes)(pOrderNode->m_data.iData1));
			break;

		case ORDER_CONVINCE:
			break;

		default:
			FAssertMsg(false, "pOrderNode->m_data.eOrderType failed to match a valid option");
			break;
		}
	}

	return 0;
}


int CvCity::getProductionNeeded(YieldTypes eYield) const
{
	CLLNode<OrderData>* pOrderNode = headOrderQueueNode();

	if (pOrderNode != NULL)
	{
		switch (pOrderNode->m_data.eOrderType)
		{
		case ORDER_TRAIN:
			return getYieldProductionNeeded((UnitTypes)pOrderNode->m_data.iData1, eYield);
			break;

		case ORDER_CONSTRUCT:
			return getYieldProductionNeeded((BuildingTypes)pOrderNode->m_data.iData1, eYield);
			break;

		case ORDER_CONVINCE:
			if (GC.getYieldInfo(eYield).isCargo())
			{
				return 0;
			}
			else
			{
				return MAX_INT;
			}
			break;

		default:
			FAssertMsg(false, "pOrderNode->m_data.eOrderType failed to match a valid option");
			break;
		}
	}

	return MAX_INT;
}

int CvCity::getYieldProductionNeeded(UnitTypes eUnit, YieldTypes eYield) const
{
	return GET_PLAYER(getOwnerINLINE()).getYieldProductionNeeded(eUnit, eYield);
}

int CvCity::getYieldProductionNeeded(BuildingTypes eBuilding, YieldTypes eYield) const
{
	int iProductionNeeded = GET_PLAYER(getOwnerINLINE()).getYieldProductionNeeded(eBuilding, eYield);

	// Python cost modifier
	if (GC.getUSE_GET_BUILDING_COST_MOD_CALLBACK())
	{
		CyArgsList argsList;
		argsList.add(getOwnerINLINE());	// Player ID
		argsList.add(getID());	// City ID
		argsList.add(eBuilding);	// Building ID
		argsList.add(eYield);
		long lResult=0;
		gDLL->getPythonIFace()->callFunction(PYGameModule, "getBuildingCostMod", argsList.makeFunctionArgs(), &lResult);

		if (lResult > 1)
		{
			iProductionNeeded *= lResult;
			iProductionNeeded /= 100;
		}
	}

	return iProductionNeeded;
}

int CvCity::getProductionTurnsLeft() const
{
	CLLNode<OrderData>* pOrderNode = headOrderQueueNode();

	if (pOrderNode != NULL)
	{
		switch (pOrderNode->m_data.eOrderType)
		{
		case ORDER_TRAIN:
			return getProductionTurnsLeft(((UnitTypes)(pOrderNode->m_data.iData1)), 0);
			break;

		case ORDER_CONSTRUCT:
			return getProductionTurnsLeft(((BuildingTypes)(pOrderNode->m_data.iData1)), 0);
			break;

		case ORDER_CONVINCE:
			break;

		default:
			FAssertMsg(false, "pOrderNode->m_data.eOrderType failed to match a valid option");
			break;
		}
	}

	return MAX_INT;
}


int CvCity::getProductionTurnsLeft(UnitTypes eUnit, int iNum) const
{
	int iProduction = 0;

	int iFirstUnitOrder = getFirstUnitOrder(eUnit);

	if ((iFirstUnitOrder == -1) || (iFirstUnitOrder == iNum))
	{
		iProduction += getUnitProduction(eUnit);
	}

	int iProductionNeeded = getYieldProductionNeeded(eUnit, YIELD_HAMMERS);
	int iProductionModifier = getProductionModifier(eUnit);

	return getProductionTurnsLeft(iProductionNeeded, iProduction, getProductionDifference(iProductionNeeded, iProduction, iProductionModifier, (iNum == 0)), getProductionDifference(iProductionNeeded, iProduction, iProductionModifier, false));
}


int CvCity::getProductionTurnsLeft(BuildingTypes eBuilding, int iNum) const
{
	int iProduction;
	int iFirstBuildingOrder;
	int iProductionNeeded;
	int iProductionModifier;

	iProduction = 0;

	iFirstBuildingOrder = getFirstBuildingOrder(eBuilding);

	if ((iFirstBuildingOrder == -1) || (iFirstBuildingOrder == iNum))
	{
		iProduction += getBuildingProduction(eBuilding);
	}

	iProductionNeeded = getYieldProductionNeeded(eBuilding, YIELD_HAMMERS);

	iProductionModifier = getProductionModifier(eBuilding);

	return getProductionTurnsLeft(iProductionNeeded, iProduction, getProductionDifference(iProductionNeeded, iProduction, iProductionModifier, (iNum == 0)), getProductionDifference(iProductionNeeded, iProduction, iProductionModifier, false));
}


int CvCity::getProductionTurnsLeft(int iProductionNeeded, int iProduction, int iFirstProductionDifference, int iProductionDifference) const
{
	int iProductionLeft;
	int iTurnsLeft;

	iProductionLeft = std::max(0, (iProductionNeeded - iProduction - iFirstProductionDifference));

	if (iProductionDifference == 0)
	{
		return iProductionLeft + 1;
	}

	iTurnsLeft = (iProductionLeft / iProductionDifference);

	if ((iTurnsLeft * iProductionDifference) < iProductionLeft)
	{
		iTurnsLeft++;
	}

	iTurnsLeft++;

	return std::max(1, iTurnsLeft);
}


void CvCity::setProduction(int iNewValue)
{
	if (isProductionUnit())
	{
		setUnitProduction(getProductionUnit(), iNewValue);
	}
	else if (isProductionBuilding())
	{
		setBuildingProduction(getProductionBuilding(), iNewValue);
	}
}


void CvCity::changeProduction(int iChange)
{
	if (isProductionUnit())
	{
		changeUnitProduction(getProductionUnit(), iChange);
	}
	else if (isProductionBuilding())
	{
		changeBuildingProduction(getProductionBuilding(), iChange);
	}
}


int CvCity::getProductionModifier() const
{
	CLLNode<OrderData>* pOrderNode = headOrderQueueNode();

	if (pOrderNode != NULL)
	{
		switch (pOrderNode->m_data.eOrderType)
		{
		case ORDER_TRAIN:
			return getProductionModifier((UnitTypes)(pOrderNode->m_data.iData1));
			break;

		case ORDER_CONSTRUCT:
			return getProductionModifier((BuildingTypes)(pOrderNode->m_data.iData1));
			break;

		case ORDER_CONVINCE:
			break;

		default:
			FAssertMsg(false, "pOrderNode->m_data.eOrderType failed to match a valid option");
			break;
		}
	}

	return 0;
}


int CvCity::getProductionModifier(UnitTypes eUnit) const
{
	int iMultiplier = GET_PLAYER(getOwnerINLINE()).getProductionModifier(eUnit);

	iMultiplier += getDomainProductionModifier((DomainTypes)(GC.getUnitInfo(eUnit).getDomainType()));

	if (GC.getUnitInfo(eUnit).isMilitaryProduction())
	{
		iMultiplier += getMilitaryProductionModifier();
	}

	return std::max(0, iMultiplier);
}


int CvCity::getProductionModifier(BuildingTypes eBuilding) const
{
	int iMultiplier = GET_PLAYER(getOwnerINLINE()).getProductionModifier(eBuilding);

	return std::max(0, iMultiplier);
}


int CvCity::getProductionDifference(int iProductionNeeded, int iProduction, int iProductionModifier, bool bOverflow) const
{
	if (isDisorder())
	{
		return 0;
	}

	int iNetYield = calculateNetYield(YIELD_HAMMERS);
	if (bOverflow)
	{
		iNetYield += getOverflowProduction();
	}

	return (iNetYield * (100 + iProductionModifier) / 100);
}


int CvCity::getCurrentProductionDifference(bool bOverflow) const
{
	return getProductionDifference(getProductionNeeded(YIELD_HAMMERS), getProduction(), getProductionModifier(), !isProductionConvince());
}

int CvCity::getExtraProductionDifference(int iExtra, int iModifier) const
{
	return ((iExtra * (getBaseYieldRateModifier(YIELD_HAMMERS) + iModifier)) / 100);
}

bool CvCity::canHurry(HurryTypes eHurry, bool bTestVisible) const
{
	if (!GC.getHurryInfo(eHurry).isCity())
	{
		return false;
	}

	if (!GET_PLAYER(getOwnerINLINE()).canHurry(eHurry))
	{
		return false;
	}

	if (isDisorder())
	{
		return false;
	}

	if (getProduction() >= getProductionNeeded(YIELD_HAMMERS))
	{
		bool bYieldBuyable = false;
		for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
		{
			YieldTypes eYield = (YieldTypes) iYield;
			if (GC.getYieldInfo(eYield).isCargo())
			{
				if (getHurryYieldDeficit(eHurry, eYield) > 0)
				{
					if (GET_PLAYER(getOwnerINLINE()).getParent() != NO_PLAYER && GET_PLAYER(getOwnerINLINE()).isYieldEuropeTradable(eYield))
					{
						bYieldBuyable = true;
						break;
					}
				}
			}
		}
		if (!bYieldBuyable)
		{
			return false;
		}
	}

	if (!bTestVisible)
	{
		if (!isProductionUnit() && !isProductionBuilding())
		{
			return false;
		}

		for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
		{
			YieldTypes eYield = (YieldTypes) iYield;
			if (GC.getYieldInfo(eYield).isCargo())
			{
				if (getHurryYieldDeficit(eHurry, eYield) > 0)
				{
					if (GET_PLAYER(getOwnerINLINE()).getParent() == NO_PLAYER || !GET_PLAYER(getOwnerINLINE()).isYieldEuropeTradable(eYield))
					{
						return false;
					}
				}
			}
		}

		if (GET_PLAYER(getOwnerINLINE()).getGold() < hurryGold(eHurry))
		{
			return false;
		}

		if (maxHurryPopulation() < hurryPopulation(eHurry))
		{
			return false;
		}
	}

	return true;
}

void CvCity::hurry(HurryTypes eHurry)
{
	if (!canHurry(eHurry))
	{
		return;
	}

	int iHurryGold = hurryGold(eHurry);
	int iHurryPopulation = hurryPopulation(eHurry);

	for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
	{
		YieldTypes eYield = (YieldTypes) iYield;
		changeYieldRushed(eYield, getHurryYieldDeficit(eHurry, eYield));

		if (GC.getHurryInfo(eHurry).getProductionYieldConsumed() == eYield)
		{
			int iYieldConsumed = hurryCost(false, true) * GC.getHurryInfo(eHurry).getProductionYieldPercent() / 100;
			int iRushed = std::min(iYieldConsumed, getYieldRushed(eYield));
			changeYieldRushed(eYield, -iRushed);
			changeYieldStored(eYield, iRushed - iYieldConsumed);
		}
	}

	changeProduction(hurryProduction(eHurry));

	GET_PLAYER(getOwnerINLINE()).changeGold(-iHurryGold);
	GET_PLAYER(getOwnerINLINE()).AI_changeHurrySpending(iHurryGold);
	changePopulation(-iHurryPopulation);

	if ((getOwnerINLINE() == GC.getGameINLINE().getActivePlayer()) && isCitySelected())
	{
		gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
		gDLL->getInterfaceIFace()->setDirty(ResourceTable_DIRTY_BIT, true);
	}

	// Python Event
	gDLL->getEventReporterIFace()->cityHurry(this, eHurry);
}


void CvCity::processBuilding(BuildingTypes eBuilding, int iChange)
{
	if (GC.getBuildingInfo(eBuilding).getFreePromotion() != NO_PROMOTION)
	{
		changeFreePromotionCount(((PromotionTypes)(GC.getBuildingInfo(eBuilding).getFreePromotion())), iChange);
	}
	changeFreeExperience(GC.getBuildingInfo(eBuilding).getFreeExperience() * iChange);
	changeMaxFoodKeptPercent(GC.getBuildingInfo(eBuilding).getFoodKept() * iChange);
	changeHealRate(GC.getBuildingInfo(eBuilding).getHealRateChange() * iChange);
	changeMilitaryProductionModifier(GC.getBuildingInfo(eBuilding).getMilitaryProductionModifier() * iChange);
	changeWorksWaterCount((GC.getBuildingInfo(eBuilding).isWorksWater()) ? iChange : 0);
	for (int iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		changeSeaPlotYield(((YieldTypes)iI), (GC.getBuildingInfo(eBuilding).getSeaPlotYieldChange(iI) * iChange));
		changeRiverPlotYield(((YieldTypes)iI), (GC.getBuildingInfo(eBuilding).getRiverPlotYieldChange(iI) * iChange));
		changeYieldRateModifier(((YieldTypes)iI), (GC.getBuildingInfo(eBuilding).getYieldModifier(iI) * iChange));
	}
	setYieldRateDirty();
	for (int iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
	{
		changeUnitCombatFreeExperience(((UnitCombatTypes)iI), GC.getBuildingInfo(eBuilding).getUnitCombatFreeExperience(iI) * iChange);
	}
	for (int iI = 0; iI < NUM_DOMAIN_TYPES; iI++)
	{
		changeDomainFreeExperience(((DomainTypes)iI), GC.getBuildingInfo(eBuilding).getDomainFreeExperience(iI) * iChange);
		changeDomainProductionModifier(((DomainTypes)iI), GC.getBuildingInfo(eBuilding).getDomainProductionModifier(iI) * iChange);
	}
	GET_PLAYER(getOwnerINLINE()).changeAssets(GC.getBuildingInfo(eBuilding).getAssetValue() * iChange);
	area()->changePower(getOwnerINLINE(), (GC.getBuildingInfo(eBuilding).getPowerValue() * iChange));
	GET_PLAYER(getOwnerINLINE()).changePower(GC.getBuildingInfo(eBuilding).getPowerValue() * iChange);
	changeBuildingDefense(GC.getBuildingInfo(eBuilding).getDefenseModifier() * iChange);
	changeBuildingBombardDefense(GC.getBuildingInfo(eBuilding).getBombardDefenseModifier() * iChange);
	GET_TEAM(getTeam()).changeBuildingClassCount((BuildingClassTypes)GC.getBuildingInfo(eBuilding).getBuildingClassType(), iChange);
	GET_PLAYER(getOwnerINLINE()).changeBuildingClassCount((BuildingClassTypes)GC.getBuildingInfo(eBuilding).getBuildingClassType(), iChange);
	setLayoutDirty(true);
}

HandicapTypes CvCity::getHandicapType() const
{
	return GET_PLAYER(getOwnerINLINE()).getHandicapType();
}

CivilizationTypes CvCity::getCivilizationType() const
{
	return GET_PLAYER(getOwnerINLINE()).getCivilizationType();
}

LeaderHeadTypes CvCity::getPersonalityType() const
{
	return GET_PLAYER(getOwnerINLINE()).getPersonalityType();
}

ArtStyleTypes CvCity::getArtStyleType() const
{
	return GET_PLAYER(getOwnerINLINE()).getArtStyleType();
}

CitySizeTypes CvCity::getCitySizeType() const
{
	return ((CitySizeTypes)(range((getPopulation() / 7), 0, (NUM_CITYSIZE_TYPES - 1))));
}

bool CvCity::hasTrait(TraitTypes eTrait) const
{
	return GET_PLAYER(getOwnerINLINE()).hasTrait(eTrait);
}

bool CvCity::isHuman() const
{
	return GET_PLAYER(getOwnerINLINE()).isHuman();
}

bool CvCity::isNative() const
{
	return GET_PLAYER(getOwnerINLINE()).isNative();
}

bool CvCity::isVisible(TeamTypes eTeam, bool bDebug) const
{
	return plot()->isVisible(eTeam, bDebug);
}

bool CvCity::isCapital() const
{
	return (GET_PLAYER(getOwnerINLINE()).getCapitalCity() == this);
}

bool CvCity::isCoastal(int iMinWaterSize) const
{
	return plot()->isCoastalLand(iMinWaterSize);
}

bool CvCity::isDisorder() const
{
	return (isOccupation());
}

int CvCity::extraPopulation() const
{
	int iNumExtra = 0;
	for (uint i = 0; i < m_aPopulationUnits.size(); ++i)
	{
		CvUnit* pUnit = m_aPopulationUnits[i];
		ProfessionTypes eUnitProfession = pUnit->getProfession();
		if (eUnitProfession == NO_PROFESSION) //idle, needs a profession
		{
			iNumExtra++;
		}
		else if(GC.getProfessionInfo(eUnitProfession).isWorkPlot() && !isUnitWorkingAnyPlot(pUnit)) //needs to be working, but not working yet
		{
			iNumExtra++;
		}
	}

	return iNumExtra;
}

int CvCity::foodConsumption(int iExtra) const
{
	return ((getPopulation() + iExtra) * GC.getFOOD_CONSUMPTION_PER_POPULATION());
}

int CvCity::foodDifference() const
{
	if (isDisorder())
	{
		return 0;
	}

	return calculateNetYield(YIELD_FOOD);
}

int CvCity::growthThreshold() const
{
	return (GET_PLAYER(getOwnerINLINE()).getGrowthThreshold(getPopulation()));
}

int CvCity::productionLeft() const
{
	return (getProductionNeeded(YIELD_HAMMERS) - getProduction());
}

int CvCity::getHurryCostModifier(bool bIgnoreNew) const
{
	int iModifier = 100;
	CLLNode<OrderData>* pOrderNode = headOrderQueueNode();

	if (pOrderNode != NULL)
	{
		switch (pOrderNode->m_data.eOrderType)
		{
		case ORDER_TRAIN:
			iModifier = getHurryCostModifier((UnitTypes) pOrderNode->m_data.iData1, bIgnoreNew);
			break;

		case ORDER_CONSTRUCT:
			iModifier = getHurryCostModifier((BuildingTypes) pOrderNode->m_data.iData1, bIgnoreNew);
			break;

		case ORDER_CONVINCE:
			break;

		default:
			FAssertMsg(false, "pOrderNode->m_data.eOrderType did not match a valid option");
			break;
		}
	}

	return iModifier;
}

int CvCity::getHurryCostModifier(UnitTypes eUnit, bool bIgnoreNew) const
{
	return getHurryCostModifier(GC.getUnitInfo(eUnit).getHurryCostModifier(), getUnitProduction(eUnit), bIgnoreNew);
}

int CvCity::getHurryCostModifier(BuildingTypes eBuilding, bool bIgnoreNew) const
{
	return getHurryCostModifier(GC.getBuildingInfo(eBuilding).getHurryCostModifier(), getBuildingProduction(eBuilding), bIgnoreNew);
}

int CvCity::getHurryCostModifier(int iBaseModifier, int iProduction, bool bIgnoreNew) const
{
	int iModifier = 100;
	iModifier *= std::max(0, iBaseModifier + 100);
	iModifier /= 100;

	if (iProduction == 0 && !bIgnoreNew)
	{
		iModifier *= std::max(0, (GC.getDefineINT("NEW_HURRY_MODIFIER") + 100));
		iModifier /= 100;
	}

	return iModifier;
}


int CvCity::hurryCost(bool bExtra, bool bIgnoreNew) const
{
	return (getHurryCost(bExtra, productionLeft(), getHurryCostModifier(bIgnoreNew), getProductionModifier()));
}

int CvCity::getHurryCost(bool bExtra, int iProductionLeft, int iHurryModifier, int iModifier) const
{
	int iProduction = (iProductionLeft * iHurryModifier + 99) / 100; // round up

	if (bExtra)
	{
		int iExtraProduction = getExtraProductionDifference(iProduction, iModifier);
		if (iExtraProduction > 0)
		{
			int iAdjustedProd = iProduction * iProduction;
			
			// round up
			iProduction = (iAdjustedProd + (iExtraProduction - 1)) / iExtraProduction;
		}
	}

	return std::max(0, iProduction);
}

int CvCity::hurryGold(HurryTypes eHurry) const
{
	int iGold = hurryCost(false, false) * GC.getHurryInfo(eHurry).getGoldPerProduction();
	iGold += GC.getHurryInfo(eHurry).getFlatGold() * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getGrowthPercent() / 100;

	PlayerTypes eParent = GET_PLAYER(getOwnerINLINE()).getParent();
	if (eParent != NO_PLAYER)
	{
		for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
		{
			YieldTypes eYield = (YieldTypes) iYield;
			iGold += getHurryYieldDeficit(eHurry, eYield) * GET_PLAYER(eParent).getYieldSellPrice(eYield) * GC.getHurryInfo(eHurry).getYieldCostEuropePercent() / 100;
		}
	}
	
	if (!isHuman())
	{
		iGold *= GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIHurryPercent();
		iGold /= 100;
	}

	return std::max(0, iGold);
}

int CvCity::getHurryYieldDeficit(HurryTypes eHurry, YieldTypes eYield) const
{
	if (!GC.getYieldInfo(eYield).isCargo())
	{
		return 0;
	}

	return std::max(0, getHurryYieldNeeded(eHurry, eYield) - getYieldStored(eYield) - getYieldRushed(eYield));
}

int CvCity::getHurryYieldNeeded(HurryTypes eHurry, YieldTypes eYield) const
{
	if (!GC.getYieldInfo(eYield).isCargo())
	{
		return 0;
	}

	int iAmountNeeded = getProductionNeeded(eYield);
	if (GC.getHurryInfo(eHurry).getProductionYieldConsumed() == eYield)
	{
		iAmountNeeded += hurryCost(false, true) * GC.getHurryInfo(eHurry).getProductionYieldPercent() / 100;
	}

	return iAmountNeeded;
}


int CvCity::hurryPopulation(HurryTypes eHurry) const
{
	return (getHurryPopulation(eHurry, hurryCost(true, false)));
}

int CvCity::getHurryPopulation(HurryTypes eHurry, int iHurryCost) const
{
	if (GC.getHurryInfo(eHurry).getProductionPerPopulation() == 0)
	{
		return 0;
	}

	int iPopulation = (iHurryCost - 1) / GC.getGameINLINE().getProductionPerPopulation(eHurry);

	return std::max(1, (iPopulation + 1));
}

int CvCity::hurryProduction(HurryTypes eHurry) const
{
	int iProduction;

	if (GC.getHurryInfo(eHurry).getProductionPerPopulation() > 0)
	{
		int iExtra = hurryPopulation(eHurry) * GC.getGameINLINE().getProductionPerPopulation(eHurry);
		iProduction = 100 * getExtraProductionDifference(iExtra, getProductionModifier()) / std::max(1, getHurryCostModifier());
		FAssert(iProduction >= productionLeft());
	}
	else
	{
		iProduction = productionLeft();
	}

	return iProduction;
}


int CvCity::maxHurryPopulation() const
{
	return (getPopulation() / 2);
}

int CvCity::hurryYield(HurryTypes eHurry, YieldTypes eYield) const
{
	return getHurryYieldNeeded(eHurry, eYield) - getHurryYieldDeficit(eHurry, eYield);
}

int CvCity::cultureDistance(int iDX, int iDY) const
{
	return std::max(1, plotDistance(0, 0, iDX, iDY));
}

bool CvCity::isHasBuilding(BuildingTypes eIndex) const
{
	PROFILE_FUNC();
	FAssertMsg(eIndex != NO_BUILDING, "BuildingType eIndex is expected to not be NO_BUILDING");
	return (isHasConceptualBuilding(eIndex) && isDominantSpecialBuilding(eIndex));
}


bool CvCity::isHasBuildingClass(BuildingClassTypes eIndex) const
{
	FAssert(eIndex != NO_BUILDINGCLASS);

	BuildingTypes eBuilding = (BuildingTypes) GC.getCivilizationInfo(GET_PLAYER(getOwnerINLINE()).getCivilizationType()).getCivilizationBuildings(eIndex);
	if(eBuilding == NO_BUILDING)
	{
		return false;
	}

	return isHasBuilding(eBuilding);
}

int CvCity::getProfessionOutput(ProfessionTypes eProfession, const CvUnit* pUnit, BuildingTypes* peBuilding) const
{
	if (peBuilding != NULL)
	{
		*peBuilding = NO_BUILDING;
	}

	FAssert(NO_PROFESSION != eProfession);
	if (NO_PROFESSION == eProfession)
	{
		return 0;
	}

	CvProfessionInfo& kProfessionInfo = GC.getProfessionInfo(eProfession);
	SpecialBuildingTypes eSpecialBuilding = (SpecialBuildingTypes) kProfessionInfo.getSpecialBuilding();
	if (eSpecialBuilding == NO_SPECIALBUILDING)
	{
		return 0;
	}

	YieldTypes eYieldProduced = (YieldTypes) kProfessionInfo.getYieldProduced();
	if (NO_YIELD == eYieldProduced)
	{
		return 0;
	}

	int iModifier = 100;
	if (pUnit != NULL)
	{
		iModifier += GC.getUnitInfo(pUnit->getUnitType()).getYieldModifier(eYieldProduced);
	}

	int iExtra = 0;
	if (pUnit != NULL)
	{
		iExtra += GC.getUnitInfo(pUnit->getUnitType()).getYieldChange(eYieldProduced);
	}

	int iProfessionOutput = 0;
	for (int i = 0; i < GC.getNumBuildingInfos(); i++)
	{
		BuildingTypes eBuilding = (BuildingTypes) i;
		if (GC.getBuildingInfo(eBuilding).getSpecialBuildingType() == eSpecialBuilding)
		{
			if (isHasBuilding(eBuilding))
			{
				int iBuildingOutput = (GC.getBuildingInfo(eBuilding).getProfessionOutput() + iExtra) * iModifier / 100;
				if (iBuildingOutput > iProfessionOutput)
				{
					iProfessionOutput = iBuildingOutput;
					if (peBuilding != NULL)
					{
						*peBuilding = eBuilding;
					}
				}
			}
		}
	}

	return iProfessionOutput;
}

int CvCity::getProfessionInput(ProfessionTypes eProfession, const CvUnit* pUnit) const
{
	FAssert(NO_PROFESSION != eProfession);
	if (NO_PROFESSION == eProfession)
	{
		return 0;
	}

	CvProfessionInfo& kProfessionInfo = GC.getProfessionInfo(eProfession);
	YieldTypes eYieldConsumed = (YieldTypes) kProfessionInfo.getYieldConsumed();
	if (eYieldConsumed == NO_YIELD)
	{
		return 0;
	}

	return getProfessionOutput(eProfession, pUnit);
}

BuildingTypes CvCity::getYieldBuilding(YieldTypes eYield) const
{
	for (int iProfession = 0; iProfession < GC.getNumProfessionInfos(); iProfession++)
	{
		ProfessionTypes eProfession = (ProfessionTypes) iProfession;
		if(GC.getProfessionInfo(eProfession).getYieldProduced() == eYield)
		{
			SpecialBuildingTypes eSpecialBuilding = (SpecialBuildingTypes) GC.getProfessionInfo(eProfession).getSpecialBuilding();
			if(eSpecialBuilding != NO_SPECIALBUILDING)
			{
				for (int iBuilding = 0; iBuilding < GC.getNumBuildingInfos(); ++iBuilding)
				{
					BuildingTypes eBuilding = (BuildingTypes) iBuilding;
					if(isHasBuilding(eBuilding))
					{
						if (GC.getBuildingInfo(eBuilding).getSpecialBuildingType() == eSpecialBuilding)
						{
							return eBuilding;
						}
					}
				}
			}
		}
	}

	return NO_BUILDING;
}

int CvCity::getID() const
{
	return m_iID;
}


int CvCity::getIndex() const
{
	return (getID() & FLTA_INDEX_MASK);
}


IDInfo CvCity::getIDInfo() const
{
	IDInfo city(getOwnerINLINE(), getID());
	return city;
}


void CvCity::setID(int iID)
{
	m_iID = iID;
}


int CvCity::getX() const
{
	return m_iX;
}


int CvCity::getY() const
{
	return m_iY;
}


bool CvCity::at(int iX,  int iY) const
{
	return ((getX_INLINE() == iX) && (getY_INLINE() == iY));
}


bool CvCity::at(CvPlot* pPlot) const
{
	return (plot() == pPlot);
}


CvPlot* CvCity::plot() const
{
	return GC.getMapINLINE().plotSorenINLINE(getX_INLINE(), getY_INLINE());
}

int CvCity::getArea() const
{
	return plot()->getArea();
}

CvArea* CvCity::area() const
{
	return plot()->area();
}


CvArea* CvCity::waterArea() const
{
	return plot()->waterArea();
}


CvPlot* CvCity::getRallyPlot() const
{
	return GC.getMapINLINE().plotSorenINLINE(m_iRallyX, m_iRallyY);
}


void CvCity::setRallyPlot(CvPlot* pPlot)
{
	if (getRallyPlot() != pPlot)
	{
		if (pPlot != NULL)
		{
			m_iRallyX = pPlot->getX_INLINE();
			m_iRallyY = pPlot->getY_INLINE();
		}
		else
		{
			m_iRallyX = INVALID_PLOT_COORD;
			m_iRallyY = INVALID_PLOT_COORD;
		}

		if (isCitySelected())
		{
			gDLL->getInterfaceIFace()->setDirty(ColoredPlots_DIRTY_BIT, true);
		}
	}
}


int CvCity::getGameTurnFounded() const
{
	return m_iGameTurnFounded;
}


void CvCity::setGameTurnFounded(int iNewValue)
{
	if (getGameTurnFounded() != iNewValue)
	{
		m_iGameTurnFounded = iNewValue;
		FAssert(getGameTurnFounded() >= 0);

		GC.getMapINLINE().updateWorkingCity();
	}
}


int CvCity::getGameTurnAcquired() const
{
	return m_iGameTurnAcquired;
}


void CvCity::setGameTurnAcquired(int iNewValue)
{
	m_iGameTurnAcquired = iNewValue;
	FAssert(getGameTurnAcquired() >= 0);
}


int CvCity::getPopulation() const
{
	return (int) m_aPopulationUnits.size();
}


void CvCity::setPopulation(int iNewValue)
{
	int iOldPopulation = getPopulation();

	if (iOldPopulation != iNewValue)
	{
		//adding more population
		for (int i = iOldPopulation; i < iNewValue; i++)
		{
			UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(GC.getDefineINT("DEFAULT_POPULATION_UNIT"));

			if (NO_UNIT != eUnit)
			{
				CvUnit* pUnit = GET_PLAYER(getOwnerINLINE()).initUnit(eUnit, NO_PROFESSION, getX_INLINE(), getY_INLINE());

				addPopulationUnit(pUnit, NO_PROFESSION);
			}
		}

		//removing population
		for (int i = iNewValue; i < iOldPopulation; i++)
		{
			AI_removeWorstPopulationUnit(true);
		}

		FAssert(getPopulation() >= 0);
	}
}

void CvCity::updatePopulation(int iOldPopulation)
{
	GET_PLAYER(getOwnerINLINE()).invalidatePopulationRankCache();

	if (getPopulation() > getHighestPopulation())
	{
		setHighestPopulation(getPopulation());
	}

	area()->changePopulationPerPlayer(getOwnerINLINE(), (getPopulation() - iOldPopulation));

	plot()->updateYield(true);

	GET_PLAYER(getOwnerINLINE()).AI_makeAssignWorkDirty();

	setBillboardDirty(true);
	setLayoutDirty(true);

	plot()->plotAction(PUF_makeInfoBarDirty);

	if ((getOwnerINLINE() == GC.getGameINLINE().getActivePlayer()) && isCitySelected())
	{
		gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
		gDLL->getInterfaceIFace()->setDirty(CityScreen_DIRTY_BIT, true);
	}

	//updateGenericBuildings();
}

void CvCity::changePopulation(int iChange)
{
	setPopulation(getPopulation() + iChange);
}


long CvCity::getRealPopulation() const
{
	return (getPopulation() * 100L);
}

int CvCity::getHighestPopulation() const
{
	return m_iHighestPopulation;
}


void CvCity::setHighestPopulation(int iNewValue)
{
 	m_iHighestPopulation = iNewValue;
	FAssert(getHighestPopulation() >= 0);
}


int CvCity::getWorksWaterCount() const
{
	return m_iWorksWaterCount;
}

bool CvCity::isWorksWater() const
{
	return (getWorksWaterCount() > 0);
}

void CvCity::changeWorksWaterCount(int iChange)
{
	if (iChange != 0)
	{
		m_iWorksWaterCount += iChange;
		FAssert(getWorksWaterCount() >= 0);

		AI_setAssignWorkDirty(true);
	}
}

int CvCity::getHealRate() const
{
	return m_iHealRate;
}


void CvCity::changeHealRate(int iChange)
{
	m_iHealRate = (m_iHealRate + iChange);
	FAssert(getHealRate() >= 0);
}

int CvCity::getFood() const
{
	return getYieldStored(YIELD_FOOD);
}


void CvCity::setFood(int iNewValue)
{
	setYieldStored(YIELD_FOOD, iNewValue);
}


void CvCity::changeFood(int iChange)
{
	setFood(getFood() + iChange);
}


int CvCity::getFoodKept() const
{
	return m_iFoodKept;
}


void CvCity::setFoodKept(int iNewValue)
{
	m_iFoodKept = iNewValue;
}


void CvCity::changeFoodKept(int iChange)
{
	setFoodKept(getFoodKept() + iChange);
}


int CvCity::getMaxFoodKeptPercent() const
{
	return m_iMaxFoodKeptPercent;
}


void CvCity::changeMaxFoodKeptPercent(int iChange)
{
	m_iMaxFoodKeptPercent = (m_iMaxFoodKeptPercent + iChange);
	FAssert(getMaxFoodKeptPercent() >= 0);
}


int CvCity::getOverflowProduction() const
{
	return m_iOverflowProduction;
}

void CvCity::setOverflowProduction(int iNewValue)
{
	m_iOverflowProduction = iNewValue;
	FAssert(getOverflowProduction() >= 0);
}

void CvCity::changeOverflowProduction(int iChange, int iProductionModifier)
{
	setOverflowProduction(getOverflowProduction() + iChange);
}

int CvCity::getMilitaryProductionModifier()	const
{
	return m_iMilitaryProductionModifier;
}

void CvCity::changeMilitaryProductionModifier(int iChange)
{
	m_iMilitaryProductionModifier = (m_iMilitaryProductionModifier + iChange);
}


int CvCity::getBuildingDefense() const
{
	return m_iBuildingDefense;
}


void CvCity::changeBuildingDefense(int iChange)
{
	if (iChange != 0)
	{
		m_iBuildingDefense = (m_iBuildingDefense + iChange);
		FAssert(getBuildingDefense() >= 0);

		setBillboardDirty(true);

		plot()->plotAction(PUF_makeInfoBarDirty);
	}
}


int CvCity::getBuildingBombardDefense() const
{
	return m_iBuildingBombardDefense;
}


void CvCity::changeBuildingBombardDefense(int iChange)
{
	if (iChange != 0)
	{
		m_iBuildingBombardDefense += iChange;
		FAssert(getBuildingBombardDefense() >= 0);
	}
}


int CvCity::getFreeExperience() const
{
	return m_iFreeExperience;
}


void CvCity::changeFreeExperience(int iChange)
{
	m_iFreeExperience = (m_iFreeExperience + iChange);
	FAssert(getFreeExperience() >= 0);
}


int CvCity::getDefenseDamage() const																
{
	return m_iDefenseDamage;
}

void CvCity::changeDefenseDamage(int iChange)
{
	if (iChange != 0)
	{
		m_iDefenseDamage = range((m_iDefenseDamage + iChange), 0, GC.getMAX_CITY_DEFENSE_DAMAGE());

		if (iChange > 0)
		{
			setBombarded(true);
		}

		setBillboardDirty(true);

		plot()->plotAction(PUF_makeInfoBarDirty);
	}
}

void CvCity::changeDefenseModifier(int iChange)
{
	if (iChange != 0)
	{
		int iTotalDefense = getTotalDefense();

		if (iTotalDefense > 0)
		{
			changeDefenseDamage(-(GC.getMAX_CITY_DEFENSE_DAMAGE() * iChange) / iTotalDefense);
		}
	}
}


int CvCity::getLastDefenseDamage() const
{
	return m_iLastDefenseDamage;
}


void CvCity::setLastDefenseDamage(int iNewValue)
{
	m_iLastDefenseDamage = iNewValue;
}


bool CvCity::isBombardable(const CvUnit* pUnit) const
{
	if (NULL != pUnit && !pUnit->isEnemy(getTeam()))
	{
		return false;
	}

	return (getDefenseModifier() > 0);
}


int CvCity::getTotalDefense() const
{
	return (getBuildingDefense() + GET_PLAYER(getOwnerINLINE()).getCityDefenseModifier());
}


int CvCity::getDefenseModifier() const
{
	if (isOccupation())
	{
		return 0;
	}

	return ((getTotalDefense() * (GC.getMAX_CITY_DEFENSE_DAMAGE() - getDefenseDamage())) / GC.getMAX_CITY_DEFENSE_DAMAGE());
}


int CvCity::getOccupationTimer() const
{
	return m_iOccupationTimer;
}


bool CvCity::isOccupation() const
{
	return (getOccupationTimer() > 0);
}


void CvCity::setOccupationTimer(int iNewValue)
{
	bool bOldOccupation;

	if (getOccupationTimer() != iNewValue)
	{
		bOldOccupation = isOccupation();

		m_iOccupationTimer = iNewValue;
		FAssert(getOccupationTimer() >= 0);

		if (bOldOccupation != isOccupation())
		{
			updateCultureLevel();

			AI_setAssignWorkDirty(true);
		}

		setBillboardDirty(true);
	}
}


void CvCity::changeOccupationTimer(int iChange)												
{
	setOccupationTimer(getOccupationTimer() + iChange);
}


int CvCity::getCultureUpdateTimer() const
{
	return m_iCultureUpdateTimer;
}


void CvCity::setCultureUpdateTimer(int iNewValue)
{
	m_iCultureUpdateTimer = iNewValue;
	FAssert(getOccupationTimer() >= 0);
}


void CvCity::changeCultureUpdateTimer(int iChange)												
{
	setCultureUpdateTimer(getCultureUpdateTimer() + iChange);
}


int CvCity::getCitySizeBoost() const
{
	return m_iCitySizeBoost;
}


void CvCity::setCitySizeBoost(int iBoost)
{
	if (getCitySizeBoost() != iBoost)
	{
		m_iCitySizeBoost = iBoost;

		setLayoutDirty(true);
	}
}


bool CvCity::isNeverLost() const
{
	return m_bNeverLost;
}


void CvCity::setNeverLost(bool bNewValue)
{
	m_bNeverLost = bNewValue;
}


bool CvCity::isBombarded() const
{
	return m_bBombarded;
}


void CvCity::setBombarded(bool bNewValue)
{
	m_bBombarded = bNewValue;
}

void CvCity::setAllCitizensAutomated(bool bAutomated)
{
	for (uint i = 0; i < m_aPopulationUnits.size(); ++i)
	{
		CvUnit* pUnit = m_aPopulationUnits[i];
		if (pUnit != NULL)
		{
			pUnit->setColonistLocked(!bAutomated);
		}
	}

	if (bAutomated)
	{
		AI_assignWorkingPlots();
	}

	if ((getOwnerINLINE() == GC.getGameINLINE().getActivePlayer()) && isCitySelected())
	{
		gDLL->getInterfaceIFace()->setDirty(CitizenButtons_DIRTY_BIT, true);
	}
}

void CvCity::setCitizenAutomated(int iUnitId)
{
	CvUnit* pUnit = getPopulationUnitById(iUnitId);
	if (pUnit != NULL)
	{
		pUnit->setColonistLocked(false);

		AI_assignWorkingPlots();
		if ((getOwnerINLINE() == GC.getGameINLINE().getActivePlayer()) && isCitySelected())
		{
			gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
		}
	}
}

bool CvCity::isProductionAutomated() const
{
	return m_bProductionAutomated;
}


void CvCity::setProductionAutomated(bool bNewValue, bool bClear)
{
	if (isProductionAutomated() != bNewValue)
	{
		m_bProductionAutomated = bNewValue;

		if ((getOwnerINLINE() == GC.getGameINLINE().getActivePlayer()) && isCitySelected())
		{
			gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);

			// if automated and not network game and all 3 modifiers down, clear the queue and choose again
			if (bNewValue && bClear)
			{
				clearOrderQueue();
			}
		}
		
		if (!isProduction())
		{
		    AI_chooseProduction();		    
		}
	}
}


bool CvCity::isWallOverride() const
{
	return m_bWallOverride;
}


void CvCity::setWallOverride(bool bOverride)
{
	if (isWallOverride() != bOverride)
	{
		m_bWallOverride = bOverride;

		setLayoutDirty(true);
	}
}


bool CvCity::isBillboardDirty() const
{
	return m_bInfoDirty;
}


void CvCity::setBillboardDirty(bool bNewValue)
{
	m_bInfoDirty = bNewValue;
}


bool CvCity::isLayoutDirty() const
{
	return m_bLayoutDirty;
}


void CvCity::setLayoutDirty(bool bNewValue)
{
	m_bLayoutDirty = bNewValue;
}


PlayerTypes CvCity::getOwner() const
{
	return getOwnerINLINE();
}


PlayerTypes CvCity::getPreviousOwner() const
{
	return m_ePreviousOwner;
}


void CvCity::setPreviousOwner(PlayerTypes eNewValue)
{
	m_ePreviousOwner = eNewValue;
}


PlayerTypes CvCity::getOriginalOwner() const
{
	return m_eOriginalOwner;
}


void CvCity::setOriginalOwner(PlayerTypes eNewValue)
{
	m_eOriginalOwner = eNewValue;
}


TeamTypes CvCity::getTeam() const
{
	return GET_PLAYER(getOwnerINLINE()).getTeam();
}


CultureLevelTypes CvCity::getCultureLevel() const
{
	return m_eCultureLevel;
}


int CvCity::getCultureThreshold() const
{
	return getCultureThreshold(getCultureLevel());
}

int CvCity::getCultureThreshold(CultureLevelTypes eLevel)
{
	if (eLevel == NO_CULTURELEVEL)
	{
		return 1;
	}

	CultureLevelTypes eCultureLevel = (CultureLevelTypes) std::min(eLevel + 1, GC.getNumCultureLevelInfos() - 1);
	return std::max(1, GC.getGameINLINE().getCultureLevelThreshold(eCultureLevel));
}


void CvCity::setCultureLevel(CultureLevelTypes eNewValue)
{
	CvPlot* pLoopPlot;
	CvWString szBuffer;
	CultureLevelTypes eOldValue;
	int iCultureRange;
	int iDX, iDY;
	int iI;

	eOldValue = getCultureLevel();

	if (eOldValue != eNewValue)
	{
		m_eCultureLevel = eNewValue;

		if (eOldValue != NO_CULTURELEVEL)
		{
			for (iDX = -eOldValue; iDX <= eOldValue; iDX++)
			{
				for (iDY = -eOldValue; iDY <= eOldValue; iDY++)
				{
					iCultureRange = cultureDistance(iDX, iDY);

					if (iCultureRange > getCultureLevel())
					{
						if (iCultureRange <= eOldValue)
						{
							FAssert(iCultureRange <= GC.getNumCultureLevelInfos());

							pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

							if (pLoopPlot != NULL)
							{
								pLoopPlot->changeCultureRangeCities(getOwnerINLINE(), iCultureRange, -1);
							}
						}
					}
				}
			}
		}

		if (getCultureLevel() != NO_CULTURELEVEL)
		{
			for (iDX = -getCultureLevel(); iDX <= getCultureLevel(); iDX++)
			{
				for (iDY = -getCultureLevel(); iDY <= getCultureLevel(); iDY++)
				{
					iCultureRange = cultureDistance(iDX, iDY);

					if (iCultureRange > eOldValue)
					{
						if (iCultureRange <= getCultureLevel())
						{
							FAssert(iCultureRange <= GC.getNumCultureLevelInfos());

							pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

							if (pLoopPlot != NULL)
							{
								pLoopPlot->changeCultureRangeCities(getOwnerINLINE(), iCultureRange, 1);
							}
						}
					}
				}
			}
		}

		//update all affected plots
		int iRange = std::max(getCultureLevel(), eOldValue) + GC.getDefineINT("BUY_PLOT_CULTURE_RANGE");
		for (iDX = -iRange; iDX <= iRange; iDX++)
		{
			for (iDY = -iRange; iDY <= iRange; iDY++)
			{
				pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

				if (pLoopPlot != NULL)
				{
					pLoopPlot->updateCulture(true);
				}
			}
		}

		if (GC.getGameINLINE().isFinalInitialized())
		{
			YieldTypes eCultureYield = (YieldTypes) GC.getDefineINT("CULTURE_YIELD");
			if ((getCultureLevel() > eOldValue) && (getCultureLevel() > 1) && eCultureYield != NO_YIELD)
			{
				szBuffer = gDLL->getText("TXT_KEY_MISC_BORDERS_EXPANDED", getNameKey());
				gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_CULTUREEXPANDS", MESSAGE_TYPE_MINOR_EVENT, GC.getYieldInfo(eCultureYield).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_WHITE"), getX_INLINE(), getY_INLINE(), true, true);

				if (getCultureLevel() == (GC.getNumCultureLevelInfos() - 1))
				{
					for (iI = 0; iI < MAX_PLAYERS; iI++)
					{
						if (GET_PLAYER((PlayerTypes)iI).isAlive())
						{
							if (isRevealed(GET_PLAYER((PlayerTypes)iI).getTeam(), false))
							{
								szBuffer = gDLL->getText("TXT_KEY_MISC_CULTURE_LEVEL", getNameKey(), GC.getCultureLevelInfo(getCultureLevel()).getTextKeyWide());
								gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_CULTURELEVEL", MESSAGE_TYPE_MAJOR_EVENT, GC.getYieldInfo(eCultureYield).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"), getX_INLINE(), getY_INLINE(), true, true);
							}
							else
							{
								szBuffer = gDLL->getText("TXT_KEY_MISC_CULTURE_LEVEL_UNKNOWN", GC.getCultureLevelInfo(getCultureLevel()).getTextKeyWide());
								gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_CULTURELEVEL", MESSAGE_TYPE_MAJOR_EVENT, GC.getYieldInfo(eCultureYield).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"));
							}
						}
					}
				}

				// ONEVENT - Culture growth
				gDLL->getEventReporterIFace()->cultureExpansion(this, getOwnerINLINE());
			}
		}
	}
}


void CvCity::updateCultureLevel()
{
	if (getCultureUpdateTimer() > 0)
	{
		return;
	}

	CultureLevelTypes eCultureLevel = ((CultureLevelTypes)0);

	if (!isOccupation())
	{
		for (int iI = (GC.getNumCultureLevelInfos() - 1); iI > 0; iI--)
		{
			if (getCulture(getOwnerINLINE()) >= GC.getGameINLINE().getCultureLevelThreshold((CultureLevelTypes) iI))
			{
				eCultureLevel = ((CultureLevelTypes)iI);
				break;
			}
		}
	}

	setCultureLevel(eCultureLevel);
}


int CvCity::getSeaPlotYield(YieldTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
	FAssertMsg(eIndex < NUM_YIELD_TYPES, "eIndex expected to be < NUM_YIELD_TYPES");
	return m_aiSeaPlotYield[eIndex];
}


void CvCity::changeSeaPlotYield(YieldTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
	FAssertMsg(eIndex < NUM_YIELD_TYPES, "eIndex expected to be < NUM_YIELD_TYPES");

	if (iChange != 0)
	{
		m_aiSeaPlotYield[eIndex] = (m_aiSeaPlotYield[eIndex] + iChange);
		FAssert(getSeaPlotYield(eIndex) >= 0);

		updateYield();
	}
}


int CvCity::getRiverPlotYield(YieldTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
	FAssertMsg(eIndex < NUM_YIELD_TYPES, "eIndex expected to be < NUM_YIELD_TYPES");
	return m_aiRiverPlotYield[eIndex];
}


void CvCity::changeRiverPlotYield(YieldTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
	FAssertMsg(eIndex < NUM_YIELD_TYPES, "eIndex expected to be < NUM_YIELD_TYPES");

	if (iChange != 0)
	{
		m_aiRiverPlotYield[eIndex] += iChange;
		FAssert(getRiverPlotYield(eIndex) >= 0);

		updateYield();
	}
}


int CvCity::getBaseRawYieldProduced(YieldTypes eYieldType) const													
{
	PROFILE_FUNC();

	FAssertMsg(NO_PLAYER != getOwnerINLINE(), "City must have an owner");

	if (NO_YIELD == eYieldType)
	{
		return 0;
	}
	CvYieldInfo& info = GC.getYieldInfo(eYieldType);

	if (isOccupation())
	{
		return 0;
	}

	if (NO_PLAYER == getOwnerINLINE())
	{
		return 0;
	}
	CvPlayer& owner = GET_PLAYER(getOwnerINLINE());

	//indoor professions
	int iCityYieldProduction = 0;
	for (int i = 0; i < getPopulation(); ++i)
	{
		CvUnit* pUnit = getPopulationUnitByIndex(i);
		if (NULL != pUnit)
		{
			ProfessionTypes eProfession = pUnit->getProfession();
			if (NO_PROFESSION != eProfession)
			{
				CvProfessionInfo& kProfessionInfo = GC.getProfessionInfo(eProfession);
				YieldTypes eYieldProduced = (YieldTypes) kProfessionInfo.getYieldProduced();
				if(eYieldProduced == eYieldType)
				{
					iCityYieldProduction += getProfessionOutput(eProfession, pUnit);
				}
			}
		}
	}

	//outdoor professions
	int iPlotYieldProduction = 0;
	for (int i = 0; i < NUM_CITY_PLOTS; ++i)
	{
		CvPlot* pPlot = getCityIndexPlot(i);
		if (pPlot != NULL)
		{
			if(isUnitWorkingPlot(i))
			{
				iPlotYieldProduction += pPlot->getYield(eYieldType);
			}
		}
	}

	//building extra
	int iBuildingYieldProduced = 0;
	CvCivilizationInfo& civilizationInfo = GC.getCivilizationInfo(getCivilizationType());
	for (int i = 0; i < GC.getNumBuildingClassInfos(); ++i)
	{
		BuildingTypes eBuilding = (BuildingTypes) civilizationInfo.getCivilizationBuildings(i);

		if (eBuilding != NO_BUILDING && isHasBuilding(eBuilding))
		{
			iBuildingYieldProduced += GC.getBuildingInfo(eBuilding).getYieldChange(eYieldType);
			iBuildingYieldProduced += getBuildingYieldChange((BuildingClassTypes) i, eYieldType);
			iBuildingYieldProduced += owner.getBuildingYieldChange((BuildingClassTypes) i, eYieldType);
		}
	}

	return (iCityYieldProduction + iPlotYieldProduction + iBuildingYieldProduced);
}

int CvCity::getRawYieldProduced(YieldTypes eYieldType) const													
{
	return (getBaseRawYieldProduced(eYieldType) * getBaseYieldRateModifier(eYieldType)) / 100;
}

int CvCity::getRawYieldConsumed(YieldTypes eYieldType) const													
{
	PROFILE_FUNC();

	if (isOccupation())
	{
		return 0;
	}

	int iYieldConsumed = 0;
	if (eYieldType == YIELD_FOOD)
	{
		iYieldConsumed = getPopulation() * GC.getFOOD_CONSUMPTION_PER_POPULATION();
	}

	for (int i = 0; i < getPopulation(); ++i)
	{
		CvUnit* pUnit = getPopulationUnitByIndex(i);
		if (NULL != pUnit)
		{
			ProfessionTypes eProfession = pUnit->getProfession();
			if (NO_PROFESSION != eProfession)
			{
				CvProfessionInfo& kProfessionInfo = GC.getProfessionInfo(eProfession);
				YieldTypes eYieldConsumed = (YieldTypes) kProfessionInfo.getYieldConsumed();
				if(eYieldConsumed == eYieldType)
				{
					int iProfessionYield = getProfessionInput(eProfession, pUnit);
					if (iProfessionYield != 0)
					{
						iYieldConsumed += iProfessionYield;
					}
				}
			}
		}
	}

	return iYieldConsumed;
}

int CvCity::getBaseYieldRateModifier(YieldTypes eIndex, int iExtra) const
{
	int iModifier;

	iModifier = getYieldRateModifier(eIndex);

	if (area() != NULL)
	{
		iModifier += area()->getYieldRateModifier(getOwnerINLINE(), eIndex);
	}

	iModifier += GET_PLAYER(getOwnerINLINE()).getYieldRateModifier(eIndex);

	iModifier += GET_PLAYER(getOwnerINLINE()).getTaxYieldRateModifier(eIndex);

	iModifier += getRebelPercent() * GC.getDefineINT("MAX_REBEL_YIELD_MODIFIER") / 100;

	if (isCapital())
	{
		iModifier += GET_PLAYER(getOwnerINLINE()).getCapitalYieldRateModifier(eIndex);
	}

	iModifier += iExtra;

	// note: player->invalidateYieldRankCache() must be called for anything that is checked here
	// so if any extra checked things are added here, the cache needs to be invalidated

	return std::max(0, (iModifier + 100));
}


int CvCity::getYieldRate(YieldTypes eIndex) const
{
	if (isOccupation())
	{
		return 0;
	}

	return (getRawYieldProduced(eIndex) - getRawYieldConsumed(eIndex));
}

int CvCity::getCultureRate() const
{
	YieldTypes eYield = (YieldTypes) GC.getDefineINT("CULTURE_YIELD");
	if (eYield == NO_YIELD)
	{
		return 0;
	}

	return calculateNetYield(eYield);
}

void CvCity::setYieldRateDirty()
{
	if (getTeam() == GC.getGameINLINE().getActiveTeam())
	{
		GET_PLAYER(getOwnerINLINE()).setCityBillboardDirty(true);
		GET_PLAYER(getOwnerINLINE()).invalidateYieldRankCache(NO_YIELD);

		if (isCitySelected())
		{
			gDLL->getInterfaceIFace()->setDirty(CityScreen_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(InfoPane_DIRTY_BIT, true );
		}
	}
}

int CvCity::getYieldRateModifier(YieldTypes eIndex)	const														
{
	FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
	FAssertMsg(eIndex < NUM_YIELD_TYPES, "eIndex expected to be < NUM_YIELD_TYPES");
	return m_aiYieldRateModifier[eIndex];
}


void CvCity::changeYieldRateModifier(YieldTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
	FAssertMsg(eIndex < NUM_YIELD_TYPES, "eIndex expected to be < NUM_YIELD_TYPES");

	if (iChange != 0)
	{
		m_aiYieldRateModifier[eIndex] += iChange;
		FAssert(getYieldRateModifier(eIndex) >= 0);

		GET_PLAYER(getOwnerINLINE()).invalidateYieldRankCache(eIndex);

		AI_setAssignWorkDirty(true);

		if (getTeam() == GC.getGameINLINE().getActiveTeam())
		{
			setBillboardDirty(true);
		}
	}
}

int CvCity::getDomainFreeExperience(DomainTypes eIndex) const													 
{
	FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
	FAssertMsg(eIndex < NUM_DOMAIN_TYPES, "eIndex expected to be < NUM_DOMAIN_TYPES");
	return m_aiDomainFreeExperience[eIndex];
}


void CvCity::changeDomainFreeExperience(DomainTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
	FAssertMsg(eIndex < NUM_DOMAIN_TYPES, "eIndex expected to be < NUM_DOMAIN_TYPES");
	m_aiDomainFreeExperience[eIndex] = (m_aiDomainFreeExperience[eIndex] + iChange);
	FAssert(getDomainFreeExperience(eIndex) >= 0);
}


int CvCity::getDomainProductionModifier(DomainTypes eIndex) const										 
{
	FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
	FAssertMsg(eIndex < NUM_DOMAIN_TYPES, "eIndex expected to be < NUM_DOMAIN_TYPES");
	return m_aiDomainProductionModifier[eIndex];
}


void CvCity::changeDomainProductionModifier(DomainTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
	FAssertMsg(eIndex < NUM_DOMAIN_TYPES, "eIndex expected to be < NUM_DOMAIN_TYPES");
	m_aiDomainProductionModifier[eIndex] = (m_aiDomainProductionModifier[eIndex] + iChange);
}


int CvCity::getCulture(PlayerTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex expected to be < MAX_PLAYERS");
	return m_aiCulture[eIndex];
}

int CvCity::countTotalCulture() const
{
	int iTotalCulture;
	int iI;

	iTotalCulture = 0;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			iTotalCulture += getCulture((PlayerTypes)iI);
		}
	}

	return iTotalCulture;
}


PlayerTypes CvCity::findHighestCulture() const
{
	PlayerTypes eBestPlayer;
	int iValue;
	int iBestValue;
	int iI;

	iBestValue = 0;
	eBestPlayer = NO_PLAYER;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			iValue = getCulture((PlayerTypes)iI);

			if (iValue > iBestValue)
			{
				iBestValue = iValue;
				eBestPlayer = ((PlayerTypes)iI);
			}
		}
	}

	return eBestPlayer;
}


int CvCity::calculateCulturePercent(PlayerTypes eIndex) const
{
	int iTotalCulture;

	iTotalCulture = countTotalCulture();

	if (iTotalCulture > 0)
	{
		return ((getCulture(eIndex) * 100) / iTotalCulture);
	}

	return 0;
}


int CvCity::calculateTeamCulturePercent(TeamTypes eIndex) const
{
	int iTeamCulturePercent;
	int iI;

	iTeamCulturePercent = 0;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() == eIndex)
			{
				iTeamCulturePercent += calculateCulturePercent((PlayerTypes)iI);
			}
		}
	}

	return iTeamCulturePercent;
}


void CvCity::setCulture(PlayerTypes eIndex, int iNewValue, bool bPlots)
{
	FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex expected to be < MAX_PLAYERS");

	if (getCulture(eIndex) != iNewValue)
	{
		m_aiCulture[eIndex] = iNewValue;
		FAssert(getCulture(eIndex) >= 0);

		updateCultureLevel();

		if (bPlots)
		{
			doPlotCulture(true, eIndex, 0);
		}
	}
}


void CvCity::changeCulture(PlayerTypes eIndex, int iChange, bool bPlots)
{
	setCulture(eIndex, (getCulture(eIndex) + iChange), bPlots);
}

int CvCity::getYieldStored(YieldTypes eYield) const
{
	FAssertMsg(eYield >= 0, "eYield expected to be >= 0");
	FAssertMsg(eYield < NUM_YIELD_TYPES	, "eYield expected to be < NUM_YIELD_TYPES");
	return m_aiYieldStored[eYield];
}

void CvCity::setYieldStored(YieldTypes eYield, int iValue)
{
	FAssertMsg(eYield >= 0, "eYield expected to be >= 0");
	FAssertMsg(eYield < NUM_YIELD_TYPES	, "eYield expected to be < NUM_YIELD_TYPES");
	FAssert(iValue >= 0 || eYield == YIELD_FOOD);

	int iChange = iValue - getYieldStored(eYield);
	if (iChange != 0)
	{
		m_aiYieldStored[eYield] = iValue;

		if (!AI_isWorkforceHack())
		{
			checkCompletedBuilds(eYield, iChange);
		}

		GET_PLAYER(getOwnerINLINE()).changePower(GC.getYieldInfo(eYield).getPowerValue() * iChange);
		GET_PLAYER(getOwnerINLINE()).changeAssets(GC.getYieldInfo(eYield).getAssetValue() * iChange);
		area()->changePower(getOwnerINLINE(), GC.getYieldInfo(eYield).getPowerValue() * iChange);

		if (getTeam() == GC.getGameINLINE().getActiveTeam())
		{
			setBillboardDirty(true);

			if (getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
			{
				gDLL->getInterfaceIFace()->setDirty(ResourceTable_DIRTY_BIT, true);
				gDLL->getInterfaceIFace()->setDirty(CitizenButtons_DIRTY_BIT, true);
				gDLL->getInterfaceIFace()->setDirty(CityScreen_DIRTY_BIT, true);
			}
		}
	}
}

void CvCity::changeYieldStored(YieldTypes eYield, int iChange)
{
	setYieldStored(eYield, getYieldStored(eYield) + iChange);
}

int CvCity::getYieldRushed(YieldTypes eYield) const
{
	FAssert(eYield >= 0 && eYield < NUM_YIELD_TYPES);
	return m_aiYieldRushed[eYield];
}

void CvCity::changeYieldRushed(YieldTypes eYield, int iChange)
{
	FAssert(eYield >= 0 && eYield < NUM_YIELD_TYPES);
	m_aiYieldRushed[eYield] += iChange;
	FAssert(getYieldRushed(eYield) >= 0);
}

void CvCity::calculateNetYields(int aiYields[NUM_YIELD_TYPES], int* aiProducedYields, int* aiConsumedYields) const
{
	PROFILE_FUNC();
	
	int aiConsumed[NUM_YIELD_TYPES];
	int aiProduced[NUM_YIELD_TYPES];
	if (aiProducedYields == NULL)
	{
		aiProducedYields = aiProduced;
	}
	if (aiConsumedYields == NULL)
	{
		aiConsumedYields = aiConsumed;
	}

	for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
	{
		YieldTypes eYield = (YieldTypes) iYield;
		aiConsumedYields[iYield] = getRawYieldConsumed(eYield);
		aiProducedYields[iYield] = getBaseRawYieldProduced(eYield);
		aiYields[iYield] = getYieldStored(eYield) - aiConsumedYields[iYield] + aiProducedYields[iYield] * getBaseYieldRateModifier(eYield) / 100;
	}

	if (!isOccupation())
	{
		std::vector<int> aiYieldsAvailable;
		for (uint i = 0; i < m_aPopulationUnits.size(); ++i)
		{
			CvUnit* pUnit = m_aPopulationUnits[i];
			if (pUnit->getProfession() != NO_PROFESSION)
			{
				aiYieldsAvailable.push_back(getProfessionInput(pUnit->getProfession(), pUnit));
			}
			else
			{
				aiYieldsAvailable.push_back(0);
			}
		}


		for (uint i = 0; i < m_aPopulationUnits.size(); ++i)
		{
			bool bFound = false;
			for (int iUnitIndex = 0; iUnitIndex < (int)m_aPopulationUnits.size(); ++iUnitIndex)
			{
				CvUnit* pUnit = m_aPopulationUnits[iUnitIndex];

				if (aiYieldsAvailable[iUnitIndex] > 0)
				{
					CvProfessionInfo& kProfession = GC.getProfessionInfo(pUnit->getProfession());

					YieldTypes eYieldConsumed = (YieldTypes) kProfession.getYieldConsumed();

					if (eYieldConsumed != NO_YIELD)
					{
						int iYieldStored = aiYields[eYieldConsumed];
						if (iYieldStored < 0)
						{
							bFound = true;
							YieldTypes eYieldProduced = (YieldTypes) kProfession.getYieldProduced();
							if (NO_YIELD != eYieldProduced)
							{
								int iDeficit = std::min(-iYieldStored, aiYieldsAvailable[iUnitIndex]);
								aiYieldsAvailable[iUnitIndex] -= iDeficit;

								aiConsumedYields[eYieldConsumed] -= iDeficit;
								aiProducedYields[eYieldProduced] -= iDeficit;

								aiYields[eYieldProduced] = getYieldStored(eYieldProduced) - aiConsumedYields[eYieldProduced] + aiProducedYields[eYieldProduced] * getBaseYieldRateModifier(eYieldProduced) / 100;
								aiYields[eYieldConsumed] = getYieldStored(eYieldConsumed) - aiConsumedYields[eYieldConsumed] + aiProducedYields[eYieldConsumed] * getBaseYieldRateModifier(eYieldConsumed) / 100;
							}
							else
							{
								FAssertMsg(false, "Could not find matching production for negative yield rate.");
							}
						}
					}
				}
			}

			if(!bFound)
			{
				break;
			}
		}
	}

	for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
	{
		FAssert((iYield == YIELD_FOOD) || (aiYields[iYield] >= 0));
		aiYields[iYield] -= getYieldStored((YieldTypes) iYield);
	}

	// Immigration
	int iImmigration = 0;
	YieldTypes eImmigrationYield = GET_PLAYER(getOwnerINLINE()).getImmigrationConversion();
	if (eImmigrationYield != YIELD_CROSSES)
	{
		aiYields[eImmigrationYield] += aiYields[YIELD_CROSSES];
	}
}

// PatchMod: Yields messages START
void CvCity::calculateNetYields(int aiYields[NUM_YIELD_TYPES],bool noval) const
{
	PROFILE_FUNC();

	int aiConsumedYields[NUM_YIELD_TYPES];
	int aiProducedYields[NUM_YIELD_TYPES];


	for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
	{
		YieldTypes eYield = (YieldTypes) iYield;
		aiConsumedYields[iYield] = getRawYieldConsumed(eYield);
		aiProducedYields[iYield] = getBaseRawYieldProduced(eYield);
		aiYields[iYield] = getYieldStored(eYield) - aiConsumedYields[iYield] + aiProducedYields[iYield] * getBaseYieldRateModifier(eYield) / 100;
	}

	if (!isOccupation())
	{


		std::vector<int> aiYieldsAvailable;
		for (uint i = 0; i < m_aPopulationUnits.size(); ++i)
		{
			CvUnit* pUnit = m_aPopulationUnits[i];
			if (pUnit->getProfession() != NO_PROFESSION)
			{
				aiYieldsAvailable.push_back(getProfessionInput(pUnit->getProfession(), pUnit));
			}
			else
			{
				aiYieldsAvailable.push_back(-1);
			}
		}


		for (uint i = 0; i < m_aPopulationUnits.size(); ++i)
		{
			bool bFound = false;
			for (int iUnitIndex = 0; iUnitIndex < (int)m_aPopulationUnits.size(); ++iUnitIndex)
			{
				CvUnit* pUnit = m_aPopulationUnits[iUnitIndex];

				if (aiYieldsAvailable[iUnitIndex] > 0)
				{
					CvProfessionInfo& kProfession = GC.getProfessionInfo(pUnit->getProfession());

					YieldTypes eYieldConsumed = (YieldTypes) kProfession.getYieldConsumed();

					if (eYieldConsumed != NO_YIELD)
					{
						int iYieldStored = aiYields[eYieldConsumed];
						if (iYieldStored < 0)
						{
							bFound = true;
							YieldTypes eYieldProduced = (YieldTypes) kProfession.getYieldProduced();
							if (NO_YIELD != eYieldProduced)
							{
								int iDeficit = std::min(-iYieldStored, aiYieldsAvailable[iUnitIndex]);
								aiYieldsAvailable[iUnitIndex] -= iDeficit;

								aiConsumedYields[eYieldConsumed] -= iDeficit;
								aiProducedYields[eYieldProduced] -= iDeficit;

								aiYields[eYieldProduced] = getYieldStored(eYieldProduced) - aiConsumedYields[eYieldProduced] + aiProducedYields[eYieldProduced] * getBaseYieldRateModifier(eYieldProduced) / 100;
								aiYields[eYieldConsumed] = getYieldStored(eYieldConsumed) - aiConsumedYields[eYieldConsumed] + aiProducedYields[eYieldConsumed] * getBaseYieldRateModifier(eYieldConsumed) / 100;
							}
							else
							{
								FAssertMsg(false, "Could not find matching production for negative yield rate.");
							}
						}
					}
				}
				else if (aiYieldsAvailable[iUnitIndex] == 0)
				{
				    CvProfessionInfo& kProfession = GC.getProfessionInfo(pUnit->getProfession());
				    YieldTypes eYieldConsumed = (YieldTypes) kProfession.getYieldConsumed();
				    YieldTypes eYieldProduced = (YieldTypes) kProfession.getYieldProduced();

				    if (eYieldConsumed != NO_YIELD)
				    {
                        CvWString szBuffer = gDLL->getText("TXT_KEY_NO_RAW", getNameKey(),GC.getYieldInfo(eYieldConsumed).getChar(),GC.getYieldInfo(eYieldProduced).getChar());
                        gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_DEAL_CANCELLED", MESSAGE_TYPE_MINOR_EVENT, GC.getYieldInfo(eYieldConsumed).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), getX_INLINE(), getY_INLINE(), true, true);
				    }
				}
			}

			if(!bFound)
			{
				break;
			}
		}
	}

	for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
	{
		FAssert((iYield == YIELD_FOOD) || (aiYields[iYield] >= 0));
		aiYields[iYield] -= getYieldStored((YieldTypes) iYield);
	}

	// Immigration
	int iImmigration = 0;
	YieldTypes eImmigrationYield = GET_PLAYER(getOwnerINLINE()).getImmigrationConversion();
	if (eImmigrationYield != YIELD_CROSSES)
	{
		aiYields[eImmigrationYield] += aiYields[YIELD_CROSSES];
	}
}
// PatchMod: Yields messages END

int CvCity::calculateNetYield(YieldTypes eYield) const
{
	FAssert(eYield < NUM_YIELD_TYPES && eYield >= 0);
	int aiYields[NUM_YIELD_TYPES];
	calculateNetYields(aiYields);
	return aiYields[eYield];
}

int CvCity::calculateActualYieldProduced(YieldTypes eYield) const
{
	FAssert(eYield < NUM_YIELD_TYPES && eYield >= 0);
	int aiYields[NUM_YIELD_TYPES];
	int aiYieldsProduced[NUM_YIELD_TYPES];
	calculateNetYields(aiYields, aiYieldsProduced);
	return aiYieldsProduced[eYield];
}

int CvCity::calculateActualYieldConsumed(YieldTypes eYield) const
{
	FAssert(eYield < NUM_YIELD_TYPES && eYield >= 0);
	int aiYields[NUM_YIELD_TYPES];
	int aiYieldsProduced[NUM_YIELD_TYPES];
	int aiYieldsConsumed[NUM_YIELD_TYPES];
	calculateNetYields(aiYields, aiYieldsProduced, aiYieldsConsumed);
	return aiYieldsConsumed[eYield];
}

int CvCity::getOverflowYieldSellPercent() const
{
	int iMaxPercent = 0;
	for (int i = 0; i < GC.getNumBuildingClassInfos(); ++i)
	{
		BuildingTypes eBuilding = (BuildingTypes) GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(i);
		if (eBuilding != NO_BUILDING && isHasBuilding(eBuilding))
		{
			iMaxPercent = std::max(GC.getBuildingInfo(eBuilding).getOverflowSellPercent(), iMaxPercent);
		}
	}

	return iMaxPercent;
}

bool CvCity::isEverOwned(PlayerTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex expected to be < MAX_PLAYERS");
	return m_abEverOwned[eIndex];
}


void CvCity::setEverOwned(PlayerTypes eIndex, bool bNewValue)
{
	FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex expected to be < MAX_PLAYERS");
	m_abEverOwned[eIndex] = bNewValue;
}


bool CvCity::isRevealed(TeamTypes eIndex, bool bDebug) const
{
	if (bDebug && GC.getGameINLINE().isDebugMode())
	{
		return true;
	}
	else
	{
		FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
		FAssertMsg(eIndex < MAX_TEAMS, "eIndex expected to be < MAX_TEAMS");

		return m_abRevealed[eIndex];
	}
}


void CvCity::setRevealed(TeamTypes eIndex, bool bNewValue)
{
	CvPlot* pLoopPlot;
	int iI;

	FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
	FAssertMsg(eIndex < MAX_TEAMS, "eIndex expected to be < MAX_TEAMS");

	if (isRevealed(eIndex, false) != bNewValue)
	{
		m_abRevealed[eIndex] = bNewValue;

		updateVisibility();

		if (eIndex == GC.getGameINLINE().getActiveTeam())
		{
			for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
			{
				pLoopPlot = getCityIndexPlot(iI);

				if (pLoopPlot != NULL)
				{
					pLoopPlot->updateSymbols();
				}
			}
		}
	}
}


const wchar* CvCity::getNameKey() const
{
	return m_szName;
}


const CvWString CvCity::getName(uint uiForm) const
{
	return gDLL->getObjectText(m_szName, uiForm, true);
}


void CvCity::setName(const wchar* szNewValue, bool bFound)
{
	CvWString szName(szNewValue);
	gDLL->stripSpecialCharacters(szName);

	if (!isEmpty(szName))
	{
		if (GET_PLAYER(getOwnerINLINE()).isCityNameValid(szName, false))
		{
			m_szName = szName;

			setBillboardDirty(true);

			if (isCitySelected())
			{
				gDLL->getInterfaceIFace()->setDirty(CityScreen_DIRTY_BIT, true);
			}
		}
		if (bFound)
		{
			doFoundMessage();
		}
	}
}


void CvCity::doFoundMessage()
{
	CvWString szBuffer;

	szBuffer = gDLL->getText("TXT_KEY_MISC_CITY_HAS_BEEN_FOUNDED", getNameKey());
	gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), false, -1, szBuffer, ARTFILEMGR.getInterfaceArtInfo("WORLDBUILDER_CITY_EDIT")->getPath(), MESSAGE_TYPE_MAJOR_EVENT, NULL, NO_COLOR, getX_INLINE(), getY_INLINE());

	szBuffer = gDLL->getText("TXT_KEY_MISC_CITY_IS_FOUNDED", getNameKey());
	GC.getGameINLINE().addReplayMessage(REPLAY_MESSAGE_CITY_FOUNDED, getOwnerINLINE(), szBuffer, getX_INLINE(), getY_INLINE(), (ColorTypes)GC.getInfoTypeForString("COLOR_ALT_HIGHLIGHT_TEXT"));
}


std::string CvCity::getScriptData() const
{
	return m_szScriptData;
}


void CvCity::setScriptData(std::string szNewValue)
{
	m_szScriptData = szNewValue;
}


int CvCity::getBuildingProduction(BuildingTypes eIndex)	const															 
{
	FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
	FAssertMsg(eIndex < GC.getNumBuildingInfos(), "eIndex expected to be < GC.getNumBuildingInfos()");
	return m_paiBuildingProduction[eIndex];
}


void CvCity::setBuildingProduction(BuildingTypes eIndex, int iNewValue)
{
	FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
	FAssertMsg(eIndex < GC.getNumBuildingInfos(), "eIndex expected to be < GC.getNumBuildingInfos()");

	if (getBuildingProduction(eIndex) != iNewValue)
	{
		m_paiBuildingProduction[eIndex] = iNewValue;
		FAssert(getBuildingProduction(eIndex) >= 0);

		if (getTeam() == GC.getGameINLINE().getActiveTeam())
		{
			setBillboardDirty(true);
		}

		if ((getOwnerINLINE() == GC.getGameINLINE().getActivePlayer()) && isCitySelected())
		{
			gDLL->getInterfaceIFace()->setDirty(CityScreen_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(Domestic_Advisor_DIRTY_BIT, true);
		}
	}
}


void CvCity::changeBuildingProduction(BuildingTypes eIndex, int iChange)							 
{
	setBuildingProduction(eIndex, (getBuildingProduction(eIndex) + iChange));
}


int CvCity::getBuildingProductionTime(BuildingTypes eIndex)	const															 
{
	FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
	FAssertMsg(eIndex < GC.getNumBuildingInfos(), "eIndex expected to be < GC.getNumBuildingInfos()");
	return m_paiBuildingProductionTime[eIndex];
}


void CvCity::setBuildingProductionTime(BuildingTypes eIndex, int iNewValue)
{
	FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
	FAssertMsg(eIndex < GC.getNumBuildingInfos(), "eIndex expected to be < GC.getNumBuildingInfos()");
	m_paiBuildingProductionTime[eIndex] = iNewValue;
	FAssert(getBuildingProductionTime(eIndex) >= 0);
}


void CvCity::changeBuildingProductionTime(BuildingTypes eIndex, int iChange)							 
{
	setBuildingProductionTime(eIndex, (getBuildingProductionTime(eIndex) + iChange));
}


int CvCity::getBuildingOriginalOwner(BuildingTypes eIndex) const															 
{
	FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
	FAssertMsg(eIndex < GC.getNumBuildingInfos(), "eIndex expected to be < GC.getNumBuildingInfos()");
	return m_paiBuildingOriginalOwner[eIndex];
}


int CvCity::getBuildingOriginalTime(BuildingTypes eIndex) const															  
{
	FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
	FAssertMsg(eIndex < GC.getNumBuildingInfos(), "eIndex expected to be < GC.getNumBuildingInfos()");
	return m_paiBuildingOriginalTime[eIndex];
}


int CvCity::getUnitProduction(UnitTypes eIndex)	const																			 
{
	FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
	FAssertMsg(eIndex < GC.getNumUnitInfos(), "eIndex expected to be < GC.getNumUnitInfos()");
	return m_paiUnitProduction[eIndex];
}


void CvCity::setUnitProduction(UnitTypes eIndex, int iNewValue)
{
	FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
	FAssertMsg(eIndex < GC.getNumUnitInfos(), "eIndex expected to be < GC.getNumUnitInfos()");

	if (getUnitProduction(eIndex) != iNewValue)
	{
		m_paiUnitProduction[eIndex] = iNewValue;
		FAssert(getUnitProduction(eIndex) >= 0);

		if (getTeam() == GC.getGameINLINE().getActiveTeam())
		{
			setBillboardDirty(true);
		}

		if ((getOwnerINLINE() == GC.getGameINLINE().getActivePlayer()) && isCitySelected())
		{
			gDLL->getInterfaceIFace()->setDirty(CityScreen_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(Domestic_Advisor_DIRTY_BIT, true);
		}
	}
}


void CvCity::changeUnitProduction(UnitTypes eIndex, int iChange)											 
{
	setUnitProduction(eIndex, (getUnitProduction(eIndex) + iChange));
}


int CvCity::getUnitProductionTime(UnitTypes eIndex)	const																			 
{
	FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
	FAssertMsg(eIndex < GC.getNumUnitInfos(), "eIndex expected to be < GC.getNumUnitInfos()");
	return m_paiUnitProductionTime[eIndex];
}


void CvCity::setUnitProductionTime(UnitTypes eIndex, int iNewValue)
{
	FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
	FAssertMsg(eIndex < GC.getNumUnitInfos(), "eIndex expected to be < GC.getNumUnitInfos()");
	m_paiUnitProductionTime[eIndex] = iNewValue;
	FAssert(getUnitProductionTime(eIndex) >= 0);
}


void CvCity::changeUnitProductionTime(UnitTypes eIndex, int iChange)											 
{
	setUnitProductionTime(eIndex, (getUnitProductionTime(eIndex) + iChange));
}


int CvCity::getUnitCombatFreeExperience(UnitCombatTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
	FAssertMsg(eIndex < GC.getNumUnitCombatInfos(), "eIndex expected to be < GC.getNumUnitCombatInfos()");
	return m_paiUnitCombatFreeExperience[eIndex];
}


void CvCity::changeUnitCombatFreeExperience(UnitCombatTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
	FAssertMsg(eIndex < GC.getNumUnitCombatInfos(), "eIndex expected to be < GC.getNumUnitCombatInfos()");
	m_paiUnitCombatFreeExperience[eIndex] = (m_paiUnitCombatFreeExperience[eIndex] + iChange);
	FAssert(getUnitCombatFreeExperience(eIndex) >= 0);
}


int CvCity::getFreePromotionCount(PromotionTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
	FAssertMsg(eIndex < GC.getNumPromotionInfos(), "eIndex expected to be < GC.getNumPromotionInfos()");
	return m_paiFreePromotionCount[eIndex];
}


bool CvCity::isFreePromotion(PromotionTypes eIndex) const
{
	return (getFreePromotionCount(eIndex) > 0);
}


void CvCity::changeFreePromotionCount(PromotionTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
	FAssertMsg(eIndex < GC.getNumPromotionInfos(), "eIndex expected to be < GC.getNumPromotionInfos()");
	m_paiFreePromotionCount[eIndex] = (m_paiFreePromotionCount[eIndex] + iChange);
	FAssert(getFreePromotionCount(eIndex) >= 0);
}


CvUnit* CvCity::getUnitWorkingPlot(int iPlotIndex) const
{
	FAssertMsg(iPlotIndex >= 0, "iPlotIndex expected to be >= 0");
	FAssertMsg(iPlotIndex < NUM_CITY_PLOTS, "iPlotIndex expected to be < NUM_CITY_PLOTS");

	int iUnitId = m_paiWorkingPlot[iPlotIndex];
	if (-1 != iUnitId)
	{
		return getPopulationUnitById(iUnitId);
	}

	return NULL;
}


bool CvCity::isUnitWorkingPlot(int iPlotIndex) const
{
	FAssertMsg(iPlotIndex >= 0, "iPlotIndex expected to be >= 0");
	FAssertMsg(iPlotIndex < NUM_CITY_PLOTS, "iPlotIndex expected to be < NUM_CITY_PLOTS");

	if (CITY_HOME_PLOT == iPlotIndex)
	{
		return true;
	}

	if (-1 != m_paiWorkingPlot[iPlotIndex])
	{
		return true;
	}

	return false;
}


bool CvCity::isUnitWorkingPlot(const CvPlot* pPlot) const
{
	int iIndex = getCityPlotIndex(pPlot);

	if (iIndex != -1)
	{
		return isUnitWorkingPlot(iIndex);
	}

	return false;
}

bool CvCity::isUnitWorkingAnyPlot(const CvUnit* pUnit) const
{
	for(int i=0;i<NUM_CITY_PLOTS;i++)
	{
		int iUnitId = m_paiWorkingPlot[i];
		if(iUnitId != -1)
		{
			if(getPopulationUnitById(iUnitId) == pUnit)
			{
				return true;
			}
		}
	}

	return false;
}

CvUnit* CvCity::getUnitWorkingPlot(const CvPlot* pPlot) const
{
	int iIndex = getCityPlotIndex(pPlot);

	if (iIndex != -1)
	{
		return getUnitWorkingPlot(iIndex);
	}

	return NULL;
}


void CvCity::setUnitWorkingPlot(int iPlotIndex, int iUnitId)
{
	FAssertMsg(iPlotIndex >= 0, "iPlotIndex expected to be >= 0");
	FAssertMsg(iPlotIndex < NUM_CITY_PLOTS, "iPlotIndex expected to be < NUM_CITY_PLOTS");
	FAssertMsg(iUnitId >= 0, "iUnitId expected to be >= 0");
	FAssert(iPlotIndex != CITY_HOME_PLOT);

	if (isUnitWorkingPlot(iPlotIndex))
	{
		FAssertMsg(false, "Trying to work a plot that's already being worked");
		return;
	}

	CvPlot* pPlot = getCityIndexPlot(iPlotIndex);

	if (pPlot != NULL)
	{
		//make sure unit is outdoor profession
		CvUnit* pUnit = getPopulationUnitById(iUnitId);
		FAssert(pUnit != NULL);
		if (pUnit == NULL)
		{
			return;
		}
		ProfessionTypes eUnitProfession = pUnit->getProfession();
		
		FAssert(pUnit->isColonistLocked() || !isUnitWorkingAnyPlot(pUnit));
		if (pUnit->isColonistLocked())
		{
			//assign profession that produces yields
			if((NO_PROFESSION == eUnitProfession) || !GC.getProfessionInfo(eUnitProfession).isWorkPlot() || pPlot->calculatePotentialProfessionYieldAmount(eUnitProfession, pUnit, false) == 0)
			{
				ProfessionTypes eBestProfession = NO_PROFESSION;
				int iBestYieldAmount = 0;
				for(int i=0;i<GC.getNumProfessionInfos();i++)
				{
					ProfessionTypes eLoopProfession = (ProfessionTypes) i;
					if (GET_PLAYER(getOwnerINLINE()).isProfessionValid(eLoopProfession, pUnit->getUnitType()))
					{
						if(GC.getProfessionInfo(eLoopProfession).isWorkPlot())
						{
							int iLoopYieldAmount = pPlot->calculatePotentialProfessionYieldAmount(eLoopProfession, pUnit, false);
							if(iLoopYieldAmount > iBestYieldAmount)
							{
								eBestProfession = eLoopProfession;
								iBestYieldAmount = iLoopYieldAmount;
							}
						}
					}
				}

				FAssert(eBestProfession != NO_PROFESSION);
				if(eBestProfession != NO_PROFESSION)
				{
					pUnit->setProfession(eBestProfession);
				}
			}

			//unassign current worked plot
			CvPlot* pCurrentWorkedPlot = getPlotWorkedByUnit(pUnit);
			if(pCurrentWorkedPlot != NULL)
			{
				clearUnitWorkingPlot(pCurrentWorkedPlot);
			}
		}
		
		m_paiWorkingPlot[iPlotIndex] = iUnitId;

		FAssertMsg(pPlot->getWorkingCity() == this, "WorkingCity is expected to be this");

		pPlot->updateYield(false);

		setYieldRateDirty();

		pPlot->updatePlotBuilder();

		if ((getTeam() == GC.getGameINLINE().getActiveTeam()) || GC.getGameINLINE().isDebugMode())
		{
			pPlot->updateSymbolDisplay();
		}	
		
		if (isCitySelected())
		{
			gDLL->getInterfaceIFace()->setDirty(InfoPane_DIRTY_BIT, true );
			gDLL->getInterfaceIFace()->setDirty(CityScreen_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(ColoredPlots_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(CitizenButtons_DIRTY_BIT, true);
		}
	}
}

void CvCity::clearUnitWorkingPlot(int iPlotIndex)
{
	FAssertMsg(iPlotIndex >= 0, "iPlotIndex expected to be >= 0");
	FAssertMsg(iPlotIndex < NUM_CITY_PLOTS, "iPlotIndex expected to be < NUM_CITY_PLOTS");
	FAssert(iPlotIndex != CITY_HOME_PLOT);

	if (!isUnitWorkingPlot(iPlotIndex))
	{
		return;
	}

	CvPlot* pPlot = getCityIndexPlot(iPlotIndex);
	if (pPlot != NULL)
	{
		m_paiWorkingPlot[iPlotIndex] = -1;
		
		FAssertMsg(pPlot->getWorkingCity() == this, "WorkingCity is expected to be this");

		setYieldRateDirty();

		pPlot->updateYield(false);

		pPlot->updatePlotBuilder();

		if ((getTeam() == GC.getGameINLINE().getActiveTeam()) || GC.getGameINLINE().isDebugMode())
		{
			pPlot->updateSymbolDisplay();
		}	

		if (isCitySelected())
		{
			gDLL->getInterfaceIFace()->setDirty(InfoPane_DIRTY_BIT, true );
			gDLL->getInterfaceIFace()->setDirty(CityScreen_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(ColoredPlots_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(CitizenButtons_DIRTY_BIT, true);
		}
	}
}

void CvCity::clearUnitWorkingPlot(CvPlot* pPlot)
{
	clearUnitWorkingPlot(getCityPlotIndex(pPlot));
}


void CvCity::setUnitWorkingPlot(const CvPlot* pPlot, int iUnitId)
{
	setUnitWorkingPlot(getCityPlotIndex(pPlot), iUnitId);
}


void CvCity::alterUnitWorkingPlot(int iPlotIndex, int iUnitId, bool bAskProfession)
{
	FAssertMsg(iPlotIndex >= 0, "iIndex expected to be >= 0");
	FAssertMsg(iPlotIndex < NUM_CITY_PLOTS, "iIndex expected to be < NUM_CITY_PLOTS");

	if (iPlotIndex == CITY_HOME_PLOT)
	{
		return;
	}

	CvPlot* pPlot = getCityIndexPlot(iPlotIndex);
	if (pPlot == NULL)
	{
		return;
	}

	CvUnit* pUnit = NULL;
	if (iUnitId != -1)
	{
		pUnit = getPopulationUnitById(iUnitId);
		if (pUnit == NULL)
		{
			pUnit = GET_PLAYER(getOwnerINLINE()).getUnit(iUnitId);
			if (pUnit != NULL)
			{
				if (!pUnit->canJoinCity(plot()))
				{
					return;
				}
				addPopulationUnit(pUnit, NO_PROFESSION);				
			}
		}
	}

	if (!canWork(pPlot) && pPlot->getOwnerINLINE() == getOwnerINLINE() && pPlot->getWorkingCityOverride() != this)
	{
		pPlot->setWorkingCityOverride(this);
	}

	if (canWork(pPlot))
	{
		if (pUnit != NULL)
		{
			pUnit->setColonistLocked(true);
		}

		CvUnit* pUnitWorkingPlot = getUnitWorkingPlot(iPlotIndex);
		if (NULL != pUnitWorkingPlot)
		{
			clearUnitWorkingPlot(iPlotIndex);
		}
		else //not worked
		{
			FAssert(iUnitId != -1);
			setUnitWorkingPlot(iPlotIndex, iUnitId);
		}

		if (isHuman() && bAskProfession)
		{
			if (pUnit != NULL)
			{
				CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_CHOOSE_PROFESSION, getID(), pUnit->getID(), 1);
				gDLL->getInterfaceIFace()->addPopup(pInfo, getOwnerINLINE(), true, true);
			}
		}
	}
}

void CvCity::alterUnitProfession(int iUnitId, ProfessionTypes eProfession)
{
	CvUnit* pUnit = getPopulationUnitById(iUnitId);
	if(pUnit != NULL)
	{
		if (pUnit->canHaveProfession(eProfession, false))
		{
			pUnit->setColonistLocked(true);
			pUnit->setProfession(eProfession);
			
			if (GC.getProfessionInfo(eProfession).isWorkPlot())
			{
				if (!isUnitWorkingAnyPlot(pUnit))
				{
					int iBestPlot = AI_bestProfessionPlot(eProfession, pUnit);
					if (iBestPlot != -1)
					{
						FAssert(!isUnitWorkingPlot(iBestPlot));
						setUnitWorkingPlot(iBestPlot, iUnitId);
					}
				}
			}
		}
	}
}

void CvCity::ejectToTransport(int iUnitId, int iTransportId)
{
	CvUnit* pUnit = getPopulationUnitById(iUnitId);
	CvUnit* pTransport = GET_PLAYER(getOwnerINLINE()).getUnit(iTransportId);

	if (pUnit != NULL && pTransport != NULL)
	{
		if (pUnit->canLoadUnit(pTransport, pUnit->plot(), true))
		{
			if (removePopulationUnit(pUnit, false, (ProfessionTypes) GC.getCivilizationInfo(GET_PLAYER(getOwnerINLINE()).getCivilizationType()).getDefaultProfession()))
			{
				pUnit->loadUnit(pTransport);
			}
		}
	}	
}

void CvCity::replaceCitizen(int iUnitId, int iReplacedUnitId, bool bAskProfession)
{
	CvUnit* pUnit = getPopulationUnitById(iUnitId);
	if (pUnit == NULL)
	{
		pUnit = GET_PLAYER(getOwnerINLINE()).getUnit(iUnitId);
		if (pUnit == NULL || !pUnit->canJoinCity(plot()))
		{
			return;
		}
		addPopulationUnit(pUnit, NO_PROFESSION);
	}

	CvUnit* pReplacedUnit = getPopulationUnitById(iReplacedUnitId);
	if (pReplacedUnit == NULL)
	{
		return;
	}

	CvPlot* pPlot = getPlotWorkedByUnit(pReplacedUnit);
	if (pPlot != NULL)
	{
		pUnit->setColonistLocked(true);
		clearUnitWorkingPlot(pPlot);
		setUnitWorkingPlot(pPlot, iUnitId);
		pReplacedUnit->setColonistLocked(false);
	}
	else
	{
		ProfessionTypes eProfession = pReplacedUnit->getProfession();
		if (pUnit->canHaveProfession(eProfession, true))
		{
			if (!isAvailableProfessionSlot(eProfession, pUnit))
			{
				pReplacedUnit->setProfession(NO_PROFESSION);
				pReplacedUnit->setColonistLocked(false);
			}
			pUnit->setProfession(eProfession);
			pUnit->setColonistLocked(true);
		}
	}

	AI_assignWorkingPlots();

	if (bAskProfession && isHuman())
	{
		CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_CHOOSE_PROFESSION, getID(), pUnit->getID(), 1);
		gDLL->getInterfaceIFace()->addPopup(pInfo, getOwnerINLINE(), true, true);
	}
}

int CvCity::getNextFreeUnitId() const
{
	for (int i = 0; i < (int)m_aPopulationUnits.size(); ++i)
	{
		CvUnit* pUnit = m_aPopulationUnits[i];
		ProfessionTypes eUnitProfession = pUnit->getProfession();
		if (eUnitProfession == NO_PROFESSION) //idle citizen
		{
			return pUnit->getID();
		}
		else if (GC.getProfessionInfo(eUnitProfession).isWorkPlot() && !isUnitWorkingAnyPlot(pUnit)) //outdoor profession, but not actively working land
		{
			return pUnit->getID();
		}
	}

	FAssertMsg(false, "Could not find free unit id.");
	return -1;
}


bool CvCity::isHasRealBuilding(BuildingTypes eIndex) const									 
{
	FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
	FAssertMsg(eIndex < GC.getNumBuildingInfos(), "eIndex expected to be < GC.getNumBuildingInfos()");
	return m_pabHasRealBuilding[eIndex];
}


void CvCity::setHasRealBuilding(BuildingTypes eIndex, bool bNewValue)
{
	setHasRealBuildingTimed(eIndex, bNewValue, true, getOwnerINLINE(), GC.getGameINLINE().getGameTurnYear());
}


void CvCity::setHasRealBuildingTimed(BuildingTypes eIndex, bool bNewValue, bool bFirst, PlayerTypes eOriginalOwner, int iOriginalTime)
{
	FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
	FAssertMsg(eIndex < GC.getNumBuildingInfos(), "eIndex expected to be < GC.getNumBuildingInfos()");

	if (bNewValue != isHasRealBuilding(eIndex))
	{
		std::deque<bool> abOldBuildings(GC.getNumBuildingInfos());
		for (int i = 0; i < GC.getNumBuildingInfos(); ++i)
		{
			abOldBuildings[i] = isHasBuilding((BuildingTypes) i);
		}

		m_pabHasRealBuilding[eIndex] = bNewValue;
		setYieldRateDirty();

		if (isHasRealBuilding(eIndex))
		{
			m_paiBuildingOriginalOwner[eIndex] = eOriginalOwner;
			m_paiBuildingOriginalTime[eIndex] = iOriginalTime;
		}
		else
		{
			m_paiBuildingOriginalOwner[eIndex] = NO_PLAYER;
			m_paiBuildingOriginalTime[eIndex] = MIN_INT;
		}

		for (int i = 0; i < GC.getNumBuildingInfos(); ++i)
		{
			BuildingTypes eBuilding = (BuildingTypes) i;
			if (abOldBuildings[eBuilding] != isHasBuilding(eBuilding))
			{
				processBuilding(eBuilding, abOldBuildings[eBuilding] ? -1 : 1);
			}
		}

		if (bNewValue)
		{
			if (bFirst)
			{
				if (GC.getBuildingInfo(eIndex).isCapital())
				{
					GET_PLAYER(getOwnerINLINE()).setCapitalCity(this);
				}

				GC.getGameINLINE().incrementBuildingClassCreatedCount((BuildingClassTypes)(GC.getBuildingInfo(eIndex).getBuildingClassType()));
			}
		}
	}
}


bool CvCity::isHasFreeBuilding(BuildingTypes eIndex) const													 
{
	FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
	FAssertMsg(eIndex < GC.getNumBuildingInfos(), "eIndex expected to be < GC.getNumBuildingInfos()");
	return m_pabHasFreeBuilding[eIndex];
}


void CvCity::setHasFreeBuilding(BuildingTypes eIndex, bool bNewValue)
{
	FAssertMsg(eIndex >= 0, "eIndex expected to be >= 0");
	FAssertMsg(eIndex < GC.getNumBuildingInfos(), "eIndex expected to be < GC.getNumBuildingInfos()");

	if (isHasFreeBuilding(eIndex) != bNewValue)
	{
		std::deque<bool> abOldBuildings(GC.getNumBuildingInfos());
		for (int i = 0; i < GC.getNumBuildingInfos(); ++i)
		{
			abOldBuildings[i] = isHasBuilding((BuildingTypes) i);
		}

		m_pabHasFreeBuilding[eIndex] = bNewValue;
		setYieldRateDirty();

		for (int i = 0; i < GC.getNumBuildingInfos(); ++i)
		{
			BuildingTypes eBuilding = (BuildingTypes) i;
			if (abOldBuildings[eBuilding] != isHasBuilding(eBuilding))
			{
				processBuilding(eBuilding, abOldBuildings[eBuilding] ? -1 : 1);
			}
		}
	}
}

bool CvCity::isHasConceptualBuilding(BuildingTypes eIndex) const
{
	return (isHasRealBuilding(eIndex) || isHasFreeBuilding(eIndex));
}

bool CvCity::isDominantSpecialBuilding(BuildingTypes eIndex) const
{
	FAssert((eIndex >= 0) && (eIndex < GC.getNumBuildingInfos()));
	CvBuildingInfo& kBuilding = GC.getBuildingInfo(eIndex);
	BuildingTypes eNextBuilding = (BuildingTypes) kBuilding.getNextSpecialBuilding();
	while (eNextBuilding != eIndex)
	{
		CvBuildingInfo& kNextBuilding = GC.getBuildingInfo(eNextBuilding);
		if (isHasConceptualBuilding(eNextBuilding))
		{
			if (kBuilding.getSpecialBuildingPriority() < kNextBuilding.getSpecialBuildingPriority())
			{
				return false;
			}
		}

		eNextBuilding = (BuildingTypes) kNextBuilding.getNextSpecialBuilding();
	}

	return true;
}

void CvCity::clearOrderQueue()
{
	while (headOrderQueueNode() != NULL)
	{
		popOrder(0);
	}

	if ((getTeam() == GC.getGameINLINE().getActiveTeam()) || GC.getGameINLINE().isDebugMode())
	{
		setBillboardDirty(true);
	}
}


void CvCity::pushOrder(OrderTypes eOrder, int iData1, int iData2, bool bSave, bool bPop, bool bAppend, bool bForce)
{
	if (bPop)
	{
		popOrder(0);
	}

	bool bValid = false;

	switch (eOrder)
	{
	case ORDER_TRAIN:
		if (canTrain((UnitTypes)iData1) || bForce)
		{
			if (iData2 == -1)
			{
				iData2 = GC.getUnitInfo((UnitTypes)iData1).getDefaultUnitAIType();
			}

			GET_PLAYER(getOwnerINLINE()).changeUnitClassMaking(((UnitClassTypes)(GC.getUnitInfo((UnitTypes) iData1).getUnitClassType())), 1);

			area()->changeNumTrainAIUnits(getOwnerINLINE(), ((UnitAITypes)iData2), 1);
			GET_PLAYER(getOwnerINLINE()).AI_changeNumTrainAIUnits(((UnitAITypes)iData2), 1);

			bValid = true;
			gDLL->getEventReporterIFace()->cityBuildingUnit(this, (UnitTypes)iData1);
		}
		break;

	case ORDER_CONSTRUCT:
		if (canConstruct((BuildingTypes)iData1) || bForce)
		{
			GET_PLAYER(getOwnerINLINE()).changeBuildingClassMaking(((BuildingClassTypes)(GC.getBuildingInfo((BuildingTypes) iData1).getBuildingClassType())), 1);

			bValid = true;
			gDLL->getEventReporterIFace()->cityBuildingBuilding(this, (BuildingTypes)iData1);
		}
		break;

	case ORDER_CONVINCE:
		if (canConvince((FatherPointTypes)iData1) || bForce)
		{
			bValid = true;
		}
		break;

	default:
		FAssertMsg(false, "iOrder did not match a valid option");
		break;
	}

	if (!bValid)
	{
		return;
	}

	OrderData order;
	order.eOrderType = eOrder;
	order.iData1 = iData1;
	order.iData2 = iData2;
	order.bSave = bSave;

	if (bAppend)
	{
		m_orderQueue.insertAtEnd(order);
	}
	else
	{
		stopHeadOrder();
		m_orderQueue.insertAtBeginning(order);
	}

	if (!bAppend || (getOrderQueueLength() == 1))
	{
		startHeadOrder();
	}

	if ((getTeam() == GC.getGameINLINE().getActiveTeam()) || GC.getGameINLINE().isDebugMode())
	{
		setBillboardDirty(true);

		if (isCitySelected())
		{
			gDLL->getInterfaceIFace()->setDirty(InfoPane_DIRTY_BIT, true );
			gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(CityScreen_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(PlotListButtons_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(Domestic_Advisor_DIRTY_BIT, true);
		}
	}
}


void CvCity::popOrder(int iNum, bool bFinish, bool bChoose)
{
	CvUnit* pUnit;
	CvPlot* pRallyPlot;
	CvWString szBuffer;
	CvString szSound;
	UnitAITypes eTrainAIUnit;
	bool bStart;
	bool bMessage;
	int iCount;
	int iProductionNeeded;
	int iOverflow;

	if (iNum == -1)
	{
		iNum = (getOrderQueueLength() - 1);
	}

	iCount = 0;

	CLLNode<OrderData>* pOrderNode = headOrderQueueNode();

	while (pOrderNode != NULL)
	{
		if (iCount == iNum)
		{
			break;
		}

		iCount++;

		pOrderNode = nextOrderQueueNode(pOrderNode);
	}

	if (pOrderNode == NULL)
	{
		return;
	}

	if (bFinish && !processRequiredYields(iNum))
	{
		bFinish = false;
	}

	if (bFinish && pOrderNode->m_data.bSave)
	{
		pushOrder(pOrderNode->m_data.eOrderType, pOrderNode->m_data.iData1, pOrderNode->m_data.iData2, true, false, true);
	}

	UnitTypes eTrainUnit = NO_UNIT;
	BuildingTypes eConstructBuilding = NO_BUILDING;

	switch (pOrderNode->m_data.eOrderType)
	{
	case ORDER_TRAIN:
		eTrainUnit = ((UnitTypes)(pOrderNode->m_data.iData1));
		eTrainAIUnit = ((UnitAITypes)(pOrderNode->m_data.iData2));
		FAssertMsg(eTrainUnit != NO_UNIT, "eTrainUnit is expected to be assigned a valid unit type");
		FAssertMsg(eTrainAIUnit != NO_UNITAI, "eTrainAIUnit is expected to be assigned a valid unit AI type");

		GET_PLAYER(getOwnerINLINE()).changeUnitClassMaking(((UnitClassTypes)(GC.getUnitInfo(eTrainUnit).getUnitClassType())), -1);

		area()->changeNumTrainAIUnits(getOwnerINLINE(), eTrainAIUnit, -1);
		GET_PLAYER(getOwnerINLINE()).AI_changeNumTrainAIUnits(eTrainAIUnit, -1);

		if (bFinish)
		{
			iProductionNeeded = getYieldProductionNeeded(eTrainUnit, YIELD_HAMMERS);

			// max overflow is the value of the item produced (to eliminate prebuild exploits)
			iOverflow = getUnitProduction(eTrainUnit) - iProductionNeeded;
			int iMaxOverflow = std::max(iProductionNeeded, getCurrentProductionDifference(false));
			int iLostProduction = std::max(0, iOverflow - iMaxOverflow);
			iOverflow = std::min(iMaxOverflow, iOverflow);
			if (iOverflow > 0)
			{
				changeOverflowProduction(iOverflow, getProductionModifier(eTrainUnit));
			}
			setUnitProduction(eTrainUnit, 0);

			int iProductionGold = ((iLostProduction * GC.getDefineINT("MAXED_UNIT_GOLD_PERCENT")) / 100);
			if (iProductionGold > 0)
			{
				GET_PLAYER(getOwnerINLINE()).changeGold(iProductionGold);
			}

			pUnit = GET_PLAYER(getOwnerINLINE()).initUnit(eTrainUnit, (ProfessionTypes) GC.getUnitInfo(eTrainUnit).getDefaultProfession(), getX_INLINE(), getY_INLINE(), eTrainAIUnit);
			FAssertMsg(pUnit != NULL, "pUnit is expected to be assigned a valid unit object");

			pUnit->finishMoves();

			addProductionExperience(pUnit);

			pRallyPlot = getRallyPlot();

			if (pRallyPlot != NULL)
			{
				pUnit->getGroup()->pushMission(MISSION_MOVE_TO, pRallyPlot->getX_INLINE(), pRallyPlot->getY_INLINE());
			}

			gDLL->getEventReporterIFace()->unitBuilt(this, pUnit);
		}
		break;

	case ORDER_CONSTRUCT:
		eConstructBuilding = ((BuildingTypes)(pOrderNode->m_data.iData1));

		GET_PLAYER(getOwnerINLINE()).changeBuildingClassMaking(((BuildingClassTypes)(GC.getBuildingInfo(eConstructBuilding).getBuildingClassType())), -1);

		if (bFinish)
		{
			iProductionNeeded = getYieldProductionNeeded(eConstructBuilding, YIELD_HAMMERS);
			// max overflow is the value of the item produced (to eliminate prebuild exploits)
			int iOverflow = getBuildingProduction(eConstructBuilding) - iProductionNeeded;
			int iMaxOverflow = std::max(iProductionNeeded, getCurrentProductionDifference(false));
			int iLostProduction = std::max(0, iOverflow - iMaxOverflow);
			iOverflow = std::min(iMaxOverflow, iOverflow);
			if (iOverflow > 0)
			{
				changeOverflowProduction(iOverflow, getProductionModifier(eConstructBuilding));
			}
			setBuildingProduction(eConstructBuilding, 0);

			int iProductionGold = ((iLostProduction * GC.getDefineINT("MAXED_BUILDING_GOLD_PERCENT")) / 100);
			if (iProductionGold > 0)
			{
				GET_PLAYER(getOwnerINLINE()).changeGold(iProductionGold);
			}

			setHasRealBuilding(eConstructBuilding, true);

			for (int i = 0; i < GC.getNumFatherPointInfos(); ++i)
			{
				FatherPointTypes ePointType = (FatherPointTypes) i;
				GET_PLAYER(getOwnerINLINE()).changeFatherPoints(ePointType, GC.getFatherPointInfo(ePointType).getBuildingPoints(GC.getBuildingInfo(eConstructBuilding).getBuildingClassType()));
			}

			gDLL->getEventReporterIFace()->buildingBuilt(this, eConstructBuilding);
		}
		break;

	case ORDER_CONVINCE:
		break;

	default:
		FAssertMsg(false, "pOrderNode->m_data.eOrderType is not a valid option");
		break;
	}

	if (pOrderNode == headOrderQueueNode())
	{
		bStart = true;
		stopHeadOrder();
	}
	else
	{
		bStart = false;
	}

	m_orderQueue.deleteNode(pOrderNode);
	pOrderNode = NULL;

	if (bStart)
	{
		startHeadOrder();
	}
	if ((getTeam() == GC.getGameINLINE().getActiveTeam()) || GC.getGameINLINE().isDebugMode())
	{
		setBillboardDirty(true);

		if (isCitySelected())
		{
			gDLL->getInterfaceIFace()->setDirty(InfoPane_DIRTY_BIT, true );
			gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(CityScreen_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(Domestic_Advisor_DIRTY_BIT, true);
		}
	}

	bMessage = false;

	if (bChoose)
	{
		if (getOrderQueueLength() == 0)
		{
			if (!isHuman() || isProductionAutomated())
			{
				AI_chooseProduction();
			}
			else
			{
				chooseProduction(eTrainUnit, eConstructBuilding, bFinish);

				bMessage = true;
			}
		}
	}

	LPCSTR szIcon = NULL;

	if (bFinish && !bMessage)
	{
		if (eTrainUnit != NO_UNIT)
		{
			szBuffer = gDLL->getText("TXT_KEY_MISC_TRAINED_UNIT_IN", GC.getUnitInfo(eTrainUnit).getTextKeyWide(), getNameKey());
			szSound = GC.getUnitInfo(eTrainUnit).getArtInfo(0, NO_PROFESSION)->getTrainSound();
			szIcon = GET_PLAYER(getOwnerINLINE()).getUnitButton(eTrainUnit);
		}
		else if (eConstructBuilding != NO_BUILDING)
		{
			szBuffer = gDLL->getText("TXT_KEY_MISC_CONSTRUCTED_BUILD_IN", GC.getBuildingInfo(eConstructBuilding).getTextKeyWide(), getNameKey());
			szSound = GC.getBuildingInfo(eConstructBuilding).getConstructSound();
			szIcon = GC.getBuildingInfo(eConstructBuilding).getButton();
		}

		if (isProductionUnit() || isProductionBuilding())
		{
			szBuffer += gDLL->getText("TXT_KEY_MISC_WORK_HAS_BEGUN", getProductionNameKey());
		}

		gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, szSound, MESSAGE_TYPE_MINOR_EVENT, szIcon, (ColorTypes)GC.getInfoTypeForString("COLOR_WHITE"), getX_INLINE(), getY_INLINE(), true, true);
	}

	if ((getTeam() == GC.getGameINLINE().getActiveTeam()) || GC.getGameINLINE().isDebugMode())
	{
		setBillboardDirty(true);

		if (isCitySelected())
		{
			gDLL->getInterfaceIFace()->setDirty(InfoPane_DIRTY_BIT, true );
			gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(CityScreen_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(PlotListButtons_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(Domestic_Advisor_DIRTY_BIT, true);
		}
	}
}

bool CvCity::checkRequiredYields(OrderTypes eOrder, int iData1) const
{
	for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
	{
		YieldTypes eYield = (YieldTypes) iYield;
		if (GC.getYieldInfo(eYield).isCargo())
		{	
			int iAmount = 0;
			switch (eOrder)
			{
			case ORDER_TRAIN:
				iAmount = GET_PLAYER(getOwnerINLINE()).getYieldProductionNeeded((UnitTypes) iData1, eYield);
				break;
			case ORDER_CONSTRUCT:
				iAmount = GET_PLAYER(getOwnerINLINE()).getYieldProductionNeeded((BuildingTypes) iData1, eYield);
				break;
			default:
				break;
			}

			if (iAmount > getYieldStored(eYield) + getYieldRushed(eYield))
			{
				return false;
			}
		}
	}

	return true;
}

void CvCity::checkCompletedBuilds(YieldTypes eYield, int iChange)
{
	if (iChange > 0)
	{
		if (GC.getYieldInfo(eYield).isCargo())
		{
			std::vector< std::pair<OrderTypes, int> > aOrders;
			getOrdersWaitingForYield(aOrders, eYield, true, iChange);

			if (!aOrders.empty())
			{
				if (isHuman() && !isProductionAutomated())
				{
					bool bFound = false;
					const CvPopupQueue& aPopups = GET_PLAYER(getOwnerINLINE()).getPopups();
					for (CvPopupQueue::const_iterator it = aPopups.begin(); it != aPopups.end() && !bFound; ++it)
					{
						CvPopupInfo* pPopup = *it;
						if (pPopup->getButtonPopupType() == BUTTONPOPUP_CHOOSE_YIELD_BUILD && pPopup->getData1() == getID() && pPopup->getData2() == eYield)
						{
							bFound = true;
						}
					}

					if (!bFound)
					{
						CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_CHOOSE_YIELD_BUILD, getID(), eYield);
						gDLL->getInterfaceIFace()->addPopup(pInfo, getOwnerINLINE());
					}
				}
				else
				{
					// just pick the first one
					pushOrder(aOrders[0].first, aOrders[0].second, NO_UNITAI, false, false, false);
					if (checkRequiredYields(aOrders[0].first, aOrders[0].second) && getProduction() >= getProductionNeeded(YIELD_HAMMERS))
					{
						popOrder(0, true, true);
					}
				}
			}
		}
	}
}

bool CvCity::processRequiredYields(int iNum)
{
	CLLNode<OrderData>* pOrderNode = headOrderQueueNode();
	
	for (int iCount = 0; pOrderNode != NULL; ++iCount)
	{
		if (iCount == iNum)
		{
			break;
		}
		pOrderNode = nextOrderQueueNode(pOrderNode);
	}

	if (pOrderNode == NULL)
	{
		return false;
	}

	if (!checkRequiredYields(pOrderNode->m_data.eOrderType, pOrderNode->m_data.iData1))
	{
		return false;
	}

	for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
	{
		YieldTypes eYield = (YieldTypes) iYield;
		if (GC.getYieldInfo(eYield).isCargo())
		{	
			int iNeeded = 0;
			switch (pOrderNode->m_data.eOrderType)
			{
			case ORDER_TRAIN:
				iNeeded = GET_PLAYER(getOwnerINLINE()).getYieldProductionNeeded((UnitTypes) pOrderNode->m_data.iData1, eYield);
				break;
			case ORDER_CONSTRUCT:
				iNeeded = GET_PLAYER(getOwnerINLINE()).getYieldProductionNeeded((BuildingTypes) pOrderNode->m_data.iData1, eYield);
				break;
			default:
				break;
			}

			int iRushedChange = std::min(iNeeded, getYieldRushed(eYield));
			changeYieldRushed(eYield, -iRushedChange);
			changeYieldStored(eYield, iRushedChange - iNeeded);
		}
	}

	return true;
}

void CvCity::getOrdersWaitingForYield(std::vector< std::pair<OrderTypes, int> >& aOrders, YieldTypes eYield, bool bYieldsComplete, int iChange) const
{
	int iStored = getYieldStored(eYield) + getYieldRushed(eYield);

	for (int iUnit = 0; iUnit < GC.getNumUnitInfos(); ++iUnit)
	{
		UnitTypes eUnit = (UnitTypes) iUnit;
		if (getUnitProduction(eUnit) > 0)
		{
			int iNeeded = getYieldProductionNeeded(eUnit, eYield);
			if ((iStored >= iNeeded && iStored - iChange < iNeeded && checkRequiredYields(ORDER_TRAIN, eUnit)) || !bYieldsComplete)
			{
				if (canTrain(eUnit) && GET_PLAYER(getOwnerINLINE()).getYieldProductionNeeded(eUnit, eYield) > 0)
				{
					bool bFound = false;
					for  (CLLNode<OrderData>* pOrderNode = headOrderQueueNode(); pOrderNode != NULL; pOrderNode = nextOrderQueueNode(pOrderNode))
					{
						if (pOrderNode->m_data.eOrderType == ORDER_TRAIN && pOrderNode->m_data.iData1 == eUnit)
						{
							bFound = true;
							break;
						}
					}

					if (!bFound)
					{
						aOrders.push_back( std::make_pair(ORDER_TRAIN, eUnit));
					}
				}
			}
		}
	}

	for (int iBuilding = 0; iBuilding < GC.getNumBuildingInfos(); ++iBuilding)
	{
		BuildingTypes eBuilding = (BuildingTypes) iBuilding;
		if (getBuildingProduction(eBuilding) > 0)
		{
			int iNeeded = getYieldProductionNeeded(eBuilding, eYield);
			if ((iStored >= iNeeded && iStored - iChange < iNeeded && checkRequiredYields(ORDER_CONSTRUCT, eBuilding)) || !bYieldsComplete)
			{
				if (canConstruct(eBuilding) && GET_PLAYER(getOwnerINLINE()).getYieldProductionNeeded(eBuilding, eYield) > 0)
				{
					bool bFound = false;
					for  (CLLNode<OrderData>* pOrderNode = headOrderQueueNode(); pOrderNode != NULL; pOrderNode = nextOrderQueueNode(pOrderNode))
					{
						if (pOrderNode->m_data.eOrderType == ORDER_CONSTRUCT && pOrderNode->m_data.iData1 == eBuilding)
						{
							bFound = true;
							break;
						}
					}

					if (!bFound)
					{
						aOrders.push_back( std::make_pair(ORDER_CONSTRUCT, eBuilding));
					}
				}
			}
		}
	}
}

void CvCity::startHeadOrder()
{
	CLLNode<OrderData>* pOrderNode = headOrderQueueNode();

	if (pOrderNode != NULL)
	{
		AI_setAssignWorkDirty(true);
	}
}


void CvCity::stopHeadOrder()
{
}


int CvCity::getOrderQueueLength() const
{
	return m_orderQueue.getLength();
}


OrderData* CvCity::getOrderFromQueue(int iIndex)
{
	CLLNode<OrderData>* pOrderNode;

	pOrderNode = m_orderQueue.nodeNum(iIndex);

	if (pOrderNode != NULL)
	{
		return &(pOrderNode->m_data);
	}
	else
	{
		return NULL;
	}
}


CLLNode<OrderData>* CvCity::nextOrderQueueNode(CLLNode<OrderData>* pNode) const
{
	return m_orderQueue.next(pNode);
}


CLLNode<OrderData>* CvCity::headOrderQueueNode() const
{
	return m_orderQueue.head();
}


CLLNode<OrderData>* CvCity::tailOrderQueueNode() const
{
	return m_orderQueue.tail();
}

void CvCity::setWallOverridePoints(const std::vector< std::pair<float, float> >& kPoints)
{
	m_kWallOverridePoints = kPoints;
	setLayoutDirty(true);
}


const std::vector< std::pair<float, float> >& CvCity::getWallOverridePoints() const
{
	return m_kWallOverridePoints;
}

// Protected Functions...

void CvCity::doGrowth()
{
	int iDiff;

	CyCity* pyCity = new CyCity(this);
	CyArgsList argsList;
	argsList.add(gDLL->getPythonIFace()->makePythonObject(pyCity));	// pass in city class
	long lResult=0;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "doGrowth", argsList.makeFunctionArgs(), &lResult);
	delete pyCity;	// python fxn must not hold on to this pointer
	if (lResult == 1)
	{
		return;
	}

	iDiff = foodDifference();

	changeFood(iDiff);
	changeFoodKept(iDiff);

	setFoodKept(range(getFoodKept(), 0, ((growthThreshold() * getMaxFoodKeptPercent()) / 100)));

	if (getFood() >= growthThreshold())
	{
		if (AI_isEmphasizeAvoidGrowth())
		{
			setFood(growthThreshold());
		}
		else
		{

			UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(GC.getDefineINT("DEFAULT_POPULATION_UNIT"));
			if (NO_UNIT != eUnit)
			{
				CvUnit* pUnit = GET_PLAYER(getOwnerINLINE()).initUnit(eUnit, (ProfessionTypes) GC.getCivilizationInfo(GET_PLAYER(getOwnerINLINE()).getCivilizationType()).getDefaultProfession(), getX_INLINE(), getY_INLINE());

				changeFood(-(std::max(0, (growthThreshold() - getFoodKept()))));
			}

			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), gDLL->getText("TXT_KEY_CITY_GROWTH", getNameKey()), "AS2D_POSITIVE_DINK", MESSAGE_TYPE_INFO, GC.getYieldInfo(YIELD_FOOD).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), getX_INLINE(), getY_INLINE(), true, true);


			// ONEVENT - City growth
			gDLL->getEventReporterIFace()->cityGrowth(this, getOwnerINLINE());
		}
	}
	else if (getFood() < 0)
	{
		changeFood(-(getFood()));
		if (getPopulation() > 1)
		{
			if (!AI_removeWorstPopulationUnit(false))
			{
				AI_removeWorstPopulationUnit(true);
			}

			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), gDLL->getText("TXT_KEY_CITY_STARVING", getNameKey()), "AS2D_DEAL_CANCELLED", MESSAGE_TYPE_INFO, GC.getYieldInfo(YIELD_FOOD).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), getX_INLINE(), getY_INLINE(), true, true);
		}
		else if (!isNative())
		{
			changeOccupationTimer(2);
		}
	}
}

void CvCity::doYields()
{
	for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
	{
		YieldTypes eYield = (YieldTypes) iYield;
		changeYieldStored(eYield, getYieldRushed(eYield));
		changeYieldRushed(eYield, -getYieldRushed(eYield));
	}

	int aiYields[NUM_YIELD_TYPES];
	// PatchMod: Yields messages START
	calculateNetYields(aiYields,false);
//	calculateNetYields(aiYields);
	// PatchMod: Yields messages END

	int iMaxCapacity = getMaxYieldCapacity();
	for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
	{
		YieldTypes eYield = (YieldTypes) iYield;
		switch (eYield)
		{
		case YIELD_FOOD:
			// handled in doGrowth
			break;
		case YIELD_HAMMERS:
			// handled in doProduction
			break;
		case YIELD_CROSSES:
			// handled in CvPlayer::doCrosses
			break;
		case YIELD_EDUCATION:
			{
				std::vector<CvUnit*> apStudents;
				for (uint i = 0; i < m_aPopulationUnits.size(); ++i)
				{
					CvUnit* pLoopUnit = m_aPopulationUnits[i];
					ProfessionTypes eProfession = pLoopUnit->getProfession();
					if (eProfession != NO_PROFESSION)
					{
						if (GC.getProfessionInfo(eProfession).getYieldProduced() == eYield)
						{
							FAssert(!pLoopUnit->getUnitInfo().isTreasure());
							int iStudentOutput = getProfessionOutput(eProfession, pLoopUnit, NULL) * getBaseYieldRateModifier(YIELD_EDUCATION) / 100;
							FAssert(iStudentOutput > 0);
							pLoopUnit->setYieldStored(pLoopUnit->getYieldStored() + std::max(iStudentOutput, 1));
						}
					}
				}
			}
			break;
		default:
			changeYieldStored(eYield, aiYields[eYield]);

			if (GC.getYieldInfo(eYield).isCargo())
			{
				int iExcess = getYieldStored(eYield) - iMaxCapacity;
				if (iExcess > 0)
				{
					int iLoss = std::max(GC.getDefineINT("CITY_YIELD_DECAY_PERCENT") * iExcess / 100, GC.getDefineINT("MIN_CITY_YIELD_DECAY"));
					iLoss = std::min(iLoss, iExcess);
					changeYieldStored(eYield, -iLoss);

					int iProfit = getOverflowYieldSellPercent() * GET_PLAYER(getOwnerINLINE()).getSellToEuropeProfit(eYield, iLoss) / 100;
					if (iProfit > 0)
					{
						GET_PLAYER(getOwnerINLINE()).changeGold(iProfit);

						// PatchMod: Record warehouse trades START
						CvPlayer& kPlayerEurope = GET_PLAYER(GET_PLAYER(getOwnerINLINE()).getParent());
						GET_PLAYER(getOwnerINLINE()).changeYieldTradedTotal(eYield, iLoss);
						kPlayerEurope.changeYieldTradedTotal(eYield, iLoss);
						GC.getGameINLINE().changeYieldBoughtTotal(kPlayerEurope.getID(), eYield, -iLoss);
						// PatchMod: Record warehouse trades END

						CvWString szBuffer = gDLL->getText("TXT_KEY_GOODS_LOST_SOLD", iLoss, GC.getYieldInfo(eYield).getChar(), getNameKey(), iProfit);
						gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_BUILD_BANK", MESSAGE_TYPE_MINOR_EVENT, GC.getYieldInfo(eYield).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_WHITE"), getX_INLINE(), getY_INLINE(), true, true);
					}
					else
					{
						CvWString szBuffer = gDLL->getText("TXT_KEY_GOODS_LOST", iLoss, GC.getYieldInfo(eYield).getChar(), getNameKey());
						gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_DEAL_CANCELLED", MESSAGE_TYPE_MINOR_EVENT, GC.getYieldInfo(eYield).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), getX_INLINE(), getY_INLINE(), true, true);
					}
				}
				// PatchMod: yield warnings START
				//else if((iExcess > (-1)*(.05*iMaxCapacity)) && (aiYields[eYield] > 0))
				else if((iExcess > (-1)*(aiYields[eYield])) && (aiYields[eYield] > 0))
				{
					CvWString szBuffer = gDLL->getText("TXT_KEY_RUNNING_OUT_OF_SPACE",GC.getYieldInfo(eYield).getChar(), getNameKey());
					gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_DEAL_CANCELLED", MESSAGE_TYPE_MINOR_EVENT, GC.getYieldInfo(eYield).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), getX_INLINE(), getY_INLINE(), true, true);
                }
                // PatchMod: yield warnings END
				if (aiYields[eYield] > 0)
				{
					for (int i = 0; i < GC.getNumFatherPointInfos(); ++i)
					{
						FatherPointTypes ePointType = (FatherPointTypes) i;
						GET_PLAYER(getOwnerINLINE()).changeFatherPoints(ePointType, aiYields[eYield] * GC.getFatherPointInfo(ePointType).getYieldPoints(eYield));
					}

					gDLL->getEventReporterIFace()->yieldProduced(getOwnerINLINE(), getID(), eYield);
				}
			}
		}
	}
}

void CvCity::doCulture()
{
	CyCity* pyCity = new CyCity(this);
	CyArgsList argsList;
	argsList.add(gDLL->getPythonIFace()->makePythonObject(pyCity));	// pass in city class
	long lResult=0;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "doCulture", argsList.makeFunctionArgs(), &lResult);
	delete pyCity;	// python fxn must not hold on to this pointer
	if (lResult == 1)
	{
		return;
	}

	changeCulture(getOwnerINLINE(), getCultureRate(), false);
}


void CvCity::doPlotCulture(bool bUpdate, PlayerTypes ePlayer, int iCultureRate)
{
	CvPlot* pLoopPlot;
	int iDX, iDY;
	int iCultureRange;
	CultureLevelTypes eCultureLevel = (CultureLevelTypes)0;

	CyCity* pyCity = new CyCity(this);
	CyArgsList argsList;
	argsList.add(gDLL->getPythonIFace()->makePythonObject(pyCity));	// pass in city class
	argsList.add(bUpdate);
	argsList.add(ePlayer);
	argsList.add(iCultureRate);
	long lResult=0;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "doPlotCulture", argsList.makeFunctionArgs(), &lResult);
	delete pyCity;	// python fxn must not hold on to this pointer
	if (lResult == 1)
	{
		return;
	}

	FAssert(NO_PLAYER != ePlayer);

	if (getOwnerINLINE() == ePlayer)
	{
		eCultureLevel = getCultureLevel();
	}
	else
	{
		for (int iI = (GC.getNumCultureLevelInfos() - 1); iI > 0; iI--)
		{
			if (getCulture(ePlayer) >= GC.getGameINLINE().getCultureLevelThreshold((CultureLevelTypes) iI))
			{
				eCultureLevel = (CultureLevelTypes)iI;
				break;
			}
		}
	}

	int iFreeCultureRate = GC.getDefineINT("CITY_FREE_CULTURE_GROWTH_FACTOR");
	if (getCulture(ePlayer) > 0)
	{
		if (eCultureLevel != NO_CULTURELEVEL)
		{
			for (iDX = -eCultureLevel; iDX <= eCultureLevel; iDX++)
			{
				for (iDY = -eCultureLevel; iDY <= eCultureLevel; iDY++)
				{
					iCultureRange = cultureDistance(iDX, iDY);

					if (iCultureRange <= eCultureLevel)
					{
						pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

						if (pLoopPlot != NULL)
						{
							if (pLoopPlot->isPotentialCityWorkForArea(area()))
							{
								pLoopPlot->changeCulture(ePlayer, (((eCultureLevel - iCultureRange) * iFreeCultureRate) + iCultureRate + 1), (bUpdate || !(pLoopPlot->isOwned())));
							}
						}
					}
				}
			}
		}
	}
}

void CvCity::doSpecialists()
{
	std::set<UnitTypes> setExisting;
	if (calculateNetYield(YIELD_EDUCATION) > 0)
	{
		for (uint i = 0; i < m_aPopulationUnits.size(); ++i)
		{
			UnitTypes eUnit =  m_aPopulationUnits[i]->getUnitType();
			if (setExisting.find(eUnit) == setExisting.end())
			{
				m_aiSpecialistWeights[eUnit] += GC.getUnitInfo(eUnit).getTeacherWeight();
				setExisting.insert(eUnit);
			}
		}
	}
}


bool CvCity::doCheckProduction()
{
	CLLNode<OrderData>* pOrderNode;
	OrderData* pOrder;
	UnitTypes eUpgradeUnit;
	int iUpgradeProduction;
	CvWString szBuffer;
	int iI;
	bool bOK = true;

	if (!isProduction() && !isDisorder() && isHuman() && !isProductionAutomated())
	{
		chooseProduction();
		return bOK;
	}

	for (iI = 0; iI < GC.getNumUnitInfos(); iI++)
	{
		if (getFirstUnitOrder((UnitTypes)iI) != -1)
		{
			eUpgradeUnit = allUpgradesAvailable((UnitTypes)iI);

			if (eUpgradeUnit != NO_UNIT)
			{
				FAssertMsg(eUpgradeUnit != iI, "eUpgradeUnit is expected to be different from iI");
				iUpgradeProduction = getUnitProduction((UnitTypes)iI);
				setUnitProduction(((UnitTypes)iI), 0);
				setUnitProduction(eUpgradeUnit, iUpgradeProduction);

				pOrderNode = headOrderQueueNode();

				while (pOrderNode != NULL)
				{
					if (pOrderNode->m_data.eOrderType == ORDER_TRAIN)
					{
						if (pOrderNode->m_data.iData1 == iI)
						{
							GET_PLAYER(getOwnerINLINE()).changeUnitClassMaking(((UnitClassTypes)(GC.getUnitInfo((UnitTypes)(pOrderNode->m_data.iData1)).getUnitClassType())), -1);
							pOrderNode->m_data.iData1 = eUpgradeUnit;
							if (GET_PLAYER(getOwnerINLINE()).AI_unitValue(eUpgradeUnit, ((UnitAITypes)(pOrderNode->m_data.iData2)), area()) == 0)
							{
								area()->changeNumTrainAIUnits(getOwnerINLINE(), ((UnitAITypes)(pOrderNode->m_data.iData2)), -1);
								GET_PLAYER(getOwnerINLINE()).AI_changeNumTrainAIUnits(((UnitAITypes)(pOrderNode->m_data.iData2)), -1);
								pOrderNode->m_data.iData2 = GC.getUnitInfo(eUpgradeUnit).getDefaultUnitAIType();
								area()->changeNumTrainAIUnits(getOwnerINLINE(), ((UnitAITypes)(pOrderNode->m_data.iData2)), 1);
								GET_PLAYER(getOwnerINLINE()).AI_changeNumTrainAIUnits(((UnitAITypes)(pOrderNode->m_data.iData2)), 1);
							}
							GET_PLAYER(getOwnerINLINE()).changeUnitClassMaking(((UnitClassTypes)(GC.getUnitInfo((UnitTypes)(pOrderNode->m_data.iData1)).getUnitClassType())), 1);
						}
					}

					pOrderNode = nextOrderQueueNode(pOrderNode);
				}
			}
		}
	}

	for (iI = (getOrderQueueLength() - 1); iI >= 0; iI--)
	{
		pOrder = getOrderFromQueue(iI);

		if (pOrder != NULL)
		{
			if (!canContinueProduction(*pOrder))
			{
				popOrder(iI, false, true);
				bOK = false;
			}
		}
	}

	return bOK;
}

void CvCity::doCheat(bool bAlt, bool bShift, bool bCtrl)
{
	if (bCtrl)
	{
		changeCulture(getOwnerINLINE(), getCultureThreshold(), true);
	}
	else if(bShift)
	{
		changePopulation(1);
	}
	else if(bAlt)
	{
		for (int i = 0; i < GC.getNUM_YIELD_TYPES(); ++i)
		{
			changeYieldStored((YieldTypes)i, GC.getGameINLINE().getCargoYieldCapacity() / 2);
		}
		setRebelSentiment(getRebelSentiment() + GC.getGameINLINE().getCargoYieldCapacity());
	}
	else
	{
		popOrder(0, true);
	}
}

void CvCity::doProduction(bool bAllowNoProduction)
{
	CyCity* pyCity = new CyCity(this);
	CyArgsList argsList;
	argsList.add(gDLL->getPythonIFace()->makePythonObject(pyCity));	// pass in city class
	long lResult=0;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "doProduction", argsList.makeFunctionArgs(), &lResult);
	delete pyCity;	// python fxn must not hold on to this pointer
	if (lResult == 1)
	{
		return;
	}

	if (!isHuman() || isProductionAutomated())
	{
		if (!isProduction() || isProductionConvince() || AI_isChooseProductionDirty())
		{
			AI_chooseProduction();
		}
	}

	if (!bAllowNoProduction && !isProduction())
	{
		return;
	}

	if (isDisorder())
	{
		return;
	}

	if (isProduction())
	{
		int iProduction = getCurrentProductionDifference(!isProductionConvince());

		FatherPointTypes eFatherPointType = getProductionFatherPoint();
		if (eFatherPointType != NO_FATHER_POINT_TYPE)
		{
			GET_PLAYER(getOwnerINLINE()).changeFatherPoints(eFatherPointType, iProduction * GC.getFatherPointInfo(eFatherPointType).getProductionConversionPoints());
		}
		else
		{
			if (!isProductionConvince())
			{
				setOverflowProduction(0);
			}
			changeProduction(iProduction);
			if (getProduction() >= getProductionNeeded(YIELD_HAMMERS))
			{
				popOrder(0, true, true);
			}
		}
	}
	else
	{
		changeOverflowProduction(getCurrentProductionDifference(false), getProductionModifier());
	}
}


void CvCity::doDecay()
{
	int iI;

	for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
	{
		if (getProductionBuilding() != ((BuildingTypes)iI))
		{
			if (getBuildingProduction((BuildingTypes)iI) > 0)
			{
				changeBuildingProductionTime(((BuildingTypes)iI), 1);

				if (isHuman())
				{
					if (getBuildingProductionTime((BuildingTypes)iI) > GC.getDefineINT("BUILDING_PRODUCTION_DECAY_TIME"))
					{
						setBuildingProduction(((BuildingTypes)iI), ((getBuildingProduction((BuildingTypes)iI) * GC.getDefineINT("BUILDING_PRODUCTION_DECAY_PERCENT")) / 100));
					}
				}
			}
			else
			{
				setBuildingProductionTime(((BuildingTypes)iI), 0);
			}
		}
	}

	for (iI = 0; iI < GC.getNumUnitInfos(); iI++)
	{
		if (getProductionUnit() != ((UnitTypes)iI))
		{
			if (getUnitProduction((UnitTypes)iI) > 0)
			{
				changeUnitProductionTime(((UnitTypes)iI), 1);

				if (isHuman())
				{
					if (getUnitProductionTime((UnitTypes)iI) > GC.getDefineINT("UNIT_PRODUCTION_DECAY_TIME"))
					{
						setUnitProduction(((UnitTypes)iI), ((getUnitProduction((UnitTypes)iI) * GC.getDefineINT("UNIT_PRODUCTION_DECAY_PERCENT")) / 100));
					}
				}
			}
			else
			{
				setUnitProductionTime(((UnitTypes)iI), 0);
			}
		}
	}
}

void CvCity::doMissionaries()
{
	GET_PLAYER(getOwnerINLINE()).applyMissionaryPoints(this);
}


// Private Functions...

void CvCity::read(FDataStreamBase* pStream)
{
	int iNumElts;

	// Init data before load
	reset();

	uint uiFlag=0;
	pStream->Read(&uiFlag);	// flags for expansion

	// PatchMod: Missionary player START
	pStream->Read((int*)&m_eMissionaryPlayer);
	// PatchMod: Missionary player END

	pStream->Read(&m_iID);
	pStream->Read(&m_iX);
	pStream->Read(&m_iY);
	pStream->Read(&m_iRallyX);
	pStream->Read(&m_iRallyY);
	pStream->Read(&m_iGameTurnFounded);
	pStream->Read(&m_iGameTurnAcquired);
	pStream->Read(&m_iHighestPopulation);
	pStream->Read(&m_iWorkingPopulation);
	pStream->Read(&m_iNumBuildings);
	pStream->Read(&m_iHealRate);
	pStream->Read(&m_iFoodKept);
	pStream->Read(&m_iMaxFoodKeptPercent);
	pStream->Read(&m_iOverflowProduction);
	pStream->Read(&m_iMilitaryProductionModifier);
	pStream->Read(&m_iBuildingDefense);
	pStream->Read(&m_iBuildingBombardDefense);
	pStream->Read(&m_iFreeExperience);
	pStream->Read(&m_iDefenseDamage);
	pStream->Read(&m_iLastDefenseDamage);
	pStream->Read(&m_iOccupationTimer);
	pStream->Read(&m_iCultureUpdateTimer);
	pStream->Read(&m_iCitySizeBoost);
	pStream->Read(&m_iHammers);
	pStream->Read(&m_iMissionaryRate);
	pStream->Read(&m_iWorksWaterCount);
	pStream->Read(&m_iRebelSentiment);
	pStream->Read(&m_iTeachUnitMultiplier);

	pStream->Read(&m_bNeverLost);
	pStream->Read(&m_bBombarded);
	pStream->Read(&m_bProductionAutomated);
	pStream->Read(&m_bWallOverride);
	// m_bInfoDirty not saved...
	// m_bLayoutDirty not saved...

	pStream->Read((int*)&m_eOwner);
	pStream->Read((int*)&m_ePreviousOwner);
	pStream->Read((int*)&m_eOriginalOwner);
	pStream->Read((int*)&m_eCultureLevel);
	pStream->Read((int*)&m_eTeachUnitClass);
	pStream->Read((int*)&m_eMissionaryCivilization);

	pStream->Read(NUM_YIELD_TYPES, m_aiSeaPlotYield);
	pStream->Read(NUM_YIELD_TYPES, m_aiRiverPlotYield);
	pStream->Read(NUM_YIELD_TYPES, m_aiYieldRateModifier);
	pStream->Read(NUM_YIELD_TYPES, m_aiYieldStored);
	pStream->Read(NUM_YIELD_TYPES, m_aiYieldRushed);
	pStream->Read(NUM_DOMAIN_TYPES, m_aiDomainFreeExperience);
	pStream->Read(NUM_DOMAIN_TYPES, m_aiDomainProductionModifier);
	pStream->Read(MAX_PLAYERS, m_aiCulture);

	pStream->Read(MAX_PLAYERS, m_abEverOwned);
	pStream->Read(MAX_TEAMS, m_abRevealed);
	pStream->Read(MAX_TEAMS, m_abScoutVisited);

	pStream->ReadString(m_szName);
	pStream->ReadString(m_szScriptData);

	pStream->Read(GC.getNumBuildingInfos(), m_paiBuildingProduction);
	pStream->Read(GC.getNumBuildingInfos(), m_paiBuildingProductionTime);
	pStream->Read(GC.getNumBuildingInfos(), m_paiBuildingOriginalOwner);
	pStream->Read(GC.getNumBuildingInfos(), m_paiBuildingOriginalTime);
	pStream->Read(GC.getNumUnitInfos(), m_paiUnitProduction);
	pStream->Read(GC.getNumUnitInfos(), m_paiUnitProductionTime);
	pStream->Read(GC.getNumUnitInfos(), m_aiSpecialistWeights);
	pStream->Read(GC.getNumUnitCombatInfos(), m_paiUnitCombatFreeExperience);
	pStream->Read(GC.getNumPromotionInfos(), m_paiFreePromotionCount);
	pStream->Read(GC.getNumBuildingInfos(), m_pabHasRealBuilding);
	pStream->Read(GC.getNumBuildingInfos(), m_pabHasFreeBuilding);

	pStream->Read(NUM_CITY_PLOTS, m_paiWorkingPlot);

	FAssert(m_aPopulationUnits.size() == 0);
	m_aPopulationUnits.clear();
	int iNumPopulation;
	pStream->Read(&iNumPopulation);
	for(int i=0;i<iNumPopulation;i++)
	{
		CvUnitAI *pUnit = new CvUnitAI();
		pUnit->read(pStream);
		m_aPopulationUnits.push_back(pUnit);
	}

	m_setImports.clear();
	int iNumYields;
	std::vector<YieldTypes> aYields;
	pStream->Read(&iNumYields);
	aYields.resize(iNumYields);
	if (iNumYields > 0)
	{
		pStream->Read(iNumYields, (int*)&aYields[0]);
		m_setImports.insert(aYields.begin(), aYields.end());
	}

	m_setExports.clear();
	pStream->Read(&iNumYields);
	aYields.resize(iNumYields);
	if (iNumYields > 0)
	{
		pStream->Read(iNumYields, (int*)&aYields[0]);
		m_setExports.insert(aYields.begin(), aYields.end());
	}

	m_mapMaintain.clear();
	pStream->Read(&iNumYields);
	for (int i = 0; i < iNumYields; ++i)
	{
		YieldTypes eYield;
		int iLevel;
		pStream->Read((int*)&eYield);
		pStream->Read(&iLevel);
		m_mapMaintain[eYield] = iLevel;
	}

	m_orderQueue.Read(pStream);

	pStream->Read(&m_iPopulationRank);
	pStream->Read(&m_bPopulationRankValid);
	pStream->Read(NUM_YIELD_TYPES, m_aiBaseYieldRank);
	pStream->Read(NUM_YIELD_TYPES, m_abBaseYieldRankValid);
	pStream->Read(NUM_YIELD_TYPES, m_aiYieldRank);
	pStream->Read(NUM_YIELD_TYPES, m_abYieldRankValid);

	pStream->Read(&iNumElts);
	m_aEventsOccured.clear();
	for (int i = 0; i < iNumElts; ++i)
	{
		EventTypes eEvent;
		pStream->Read((int*)&eEvent);
		m_aEventsOccured.push_back(eEvent);
	}

	pStream->Read(&iNumElts);
	m_aBuildingYieldChange.clear();
	for (int i = 0; i < iNumElts; ++i)
	{
		BuildingYieldChange kChange;
		kChange.read(pStream);
		m_aBuildingYieldChange.push_back(kChange);
	}
}

void CvCity::write(FDataStreamBase* pStream)
{
	uint uiFlag=0;
	pStream->Write(uiFlag);		// flag for expansion

	// PatchMod: Missionary player START
	pStream->Write(m_eMissionaryPlayer);
	// PatchMod: Missionary player END

	pStream->Write(m_iID);
	pStream->Write(m_iX);
	pStream->Write(m_iY);
	pStream->Write(m_iRallyX);
	pStream->Write(m_iRallyY);
	pStream->Write(m_iGameTurnFounded);
	pStream->Write(m_iGameTurnAcquired);
	pStream->Write(m_iHighestPopulation);
	pStream->Write(m_iWorkingPopulation);
	pStream->Write(m_iNumBuildings);
	pStream->Write(m_iHealRate);
	pStream->Write(m_iFoodKept);
	pStream->Write(m_iMaxFoodKeptPercent);
	pStream->Write(m_iOverflowProduction);
	pStream->Write(m_iMilitaryProductionModifier);
	pStream->Write(m_iBuildingDefense);
	pStream->Write(m_iBuildingBombardDefense);
	pStream->Write(m_iFreeExperience);
	pStream->Write(m_iDefenseDamage);
	pStream->Write(m_iLastDefenseDamage);
	pStream->Write(m_iOccupationTimer);
	pStream->Write(m_iCultureUpdateTimer);
	pStream->Write(m_iCitySizeBoost);
	pStream->Write(m_iHammers);
	pStream->Write(m_iMissionaryRate);
	pStream->Write(m_iWorksWaterCount);
	pStream->Write(m_iRebelSentiment);
	pStream->Write(m_iTeachUnitMultiplier);

	pStream->Write(m_bNeverLost);
	pStream->Write(m_bBombarded);
	pStream->Write(m_bProductionAutomated);
	pStream->Write(m_bWallOverride);
	// m_bInfoDirty not saved...
	// m_bLayoutDirty not saved...

	pStream->Write(m_eOwner);
	pStream->Write(m_ePreviousOwner);
	pStream->Write(m_eOriginalOwner);
	pStream->Write(m_eCultureLevel);
	pStream->Write(m_eTeachUnitClass);
	pStream->Write(m_eMissionaryCivilization);

	pStream->Write(NUM_YIELD_TYPES, m_aiSeaPlotYield);
	pStream->Write(NUM_YIELD_TYPES, m_aiRiverPlotYield);
	pStream->Write(NUM_YIELD_TYPES, m_aiYieldRateModifier);
	pStream->Write(NUM_YIELD_TYPES, m_aiYieldStored);
	pStream->Write(NUM_YIELD_TYPES, m_aiYieldRushed);
	pStream->Write(NUM_DOMAIN_TYPES, m_aiDomainFreeExperience);
	pStream->Write(NUM_DOMAIN_TYPES, m_aiDomainProductionModifier);
	pStream->Write(MAX_PLAYERS, m_aiCulture);

	pStream->Write(MAX_PLAYERS, m_abEverOwned);
	pStream->Write(MAX_TEAMS, m_abRevealed);
	pStream->Write(MAX_TEAMS, m_abScoutVisited);

	pStream->WriteString(m_szName);
	pStream->WriteString(m_szScriptData);

	pStream->Write(GC.getNumBuildingInfos(), m_paiBuildingProduction);
	pStream->Write(GC.getNumBuildingInfos(), m_paiBuildingProductionTime);
	pStream->Write(GC.getNumBuildingInfos(), m_paiBuildingOriginalOwner);
	pStream->Write(GC.getNumBuildingInfos(), m_paiBuildingOriginalTime);
	pStream->Write(GC.getNumUnitInfos(), m_paiUnitProduction);
	pStream->Write(GC.getNumUnitInfos(), m_paiUnitProductionTime);
	pStream->Write(GC.getNumUnitInfos(), m_aiSpecialistWeights);
	pStream->Write(GC.getNumUnitCombatInfos(), m_paiUnitCombatFreeExperience);
	pStream->Write(GC.getNumPromotionInfos(), m_paiFreePromotionCount);
	pStream->Write(GC.getNumBuildingInfos(), m_pabHasRealBuilding);
	pStream->Write(GC.getNumBuildingInfos(), m_pabHasFreeBuilding);

	pStream->Write(NUM_CITY_PLOTS, m_paiWorkingPlot);

	pStream->Write((int)m_aPopulationUnits.size());
	for(int i=0;i<(int)m_aPopulationUnits.size();i++)
	{
		m_aPopulationUnits[i]->write(pStream);
	}

	pStream->Write((int)m_setImports.size());
	for (std::set<YieldTypes>::iterator it = m_setImports.begin(); it != m_setImports.end(); ++it)
	{
		pStream->Write(*it);
	}
	pStream->Write((int)m_setExports.size());
	for (std::set<YieldTypes>::iterator it = m_setExports.begin(); it != m_setExports.end(); ++it)
	{
		pStream->Write(*it);
	}
	pStream->Write((int)m_mapMaintain.size());
	for (std::map<YieldTypes, int>::iterator it = m_mapMaintain.begin(); it != m_mapMaintain.end(); ++it)
	{
		pStream->Write(it->first);
		pStream->Write(it->second);
	}

	m_orderQueue.Write(pStream);

	pStream->Write(m_iPopulationRank);
	pStream->Write(m_bPopulationRankValid);
	pStream->Write(NUM_YIELD_TYPES, m_aiBaseYieldRank);
	pStream->Write(NUM_YIELD_TYPES, m_abBaseYieldRankValid);
	pStream->Write(NUM_YIELD_TYPES, m_aiYieldRank);
	pStream->Write(NUM_YIELD_TYPES, m_abYieldRankValid);

	pStream->Write(m_aEventsOccured.size());
	for (std::vector<EventTypes>::iterator it = m_aEventsOccured.begin(); it != m_aEventsOccured.end(); ++it)
	{
		pStream->Write(*it);
	}

	pStream->Write(m_aBuildingYieldChange.size());
	for (std::vector<BuildingYieldChange>::iterator it = m_aBuildingYieldChange.begin(); it != m_aBuildingYieldChange.end(); ++it)
	{
		(*it).write(pStream);
	}
}


//------------------------------------------------------------------------------------------------
class VisibleBuildingComparator
{
public:
	bool operator() (BuildingTypes e1, BuildingTypes e2)
	{
		if(GC.getBuildingInfo(e1).getVisibilityPriority() > GC.getBuildingInfo(e2).getVisibilityPriority())
			return true;
		else if(GC.getBuildingInfo(e1).getVisibilityPriority() == GC.getBuildingInfo(e2).getVisibilityPriority())
		{
			//break ties by building type higher building type
			if(e1 > e2)
				return true;
		}

		return false;
	}
};

void CvCity::getVisibleBuildings(std::list<BuildingTypes>& kChosenVisible, int& iChosenNumGenerics) const
{
	int iNumBuildings;
	BuildingTypes eCurType;
	std::vector<BuildingTypes> kVisible;

	iNumBuildings = GC.getNumBuildingInfos();
	for(int i = 0; i < iNumBuildings; i++)
	{
		eCurType = (BuildingTypes) i;
		if(isHasBuilding(eCurType))
		{
			kVisible.push_back(eCurType);
		}
	}

	BonusTypes eBonus = plot()->getBonusType();
	if (eBonus != NO_BONUS)
	{
		BuildingTypes eBonusBuilding = (BuildingTypes) GC.getBonusInfo(eBonus).getBuilding();
		if(eBonusBuilding != NO_BUILDING)
		{
			kVisible.push_back(eBonusBuilding);
		}
	}

	// sort the visible ones by decreasing priority
	VisibleBuildingComparator kComp;
	std::sort(kVisible.begin(), kVisible.end(), kComp);

	// how big is this city, in terms of buildings?
	// general rule: no more than fPercentUnique percent of a city can be uniques
	int iTotalVisibleBuildings;
	if(stricmp(GC.getDefineSTRING("GAME_CITY_SIZE_METHOD"), "METHOD_EXPONENTIAL") == 0)
	{
		int iCityScaleMod =  ((int)(pow((float)getPopulation(), GC.getDefineFLOAT("GAME_CITY_SIZE_EXP_MODIFIER")))) * 2;
		iTotalVisibleBuildings = (10 + iCityScaleMod);
	} 
	else 
	{
		float fLo = GC.getDefineFLOAT("GAME_CITY_SIZE_LINMAP_AT_0");
		float fHi = GC.getDefineFLOAT("GAME_CITY_SIZE_LINMAP_AT_50");
		float fCurSize = (float)getPopulation();
		iTotalVisibleBuildings = int(((fHi - fLo) / 50.0f) * fCurSize + fLo);
	}
	float fMaxUniquePercent = GC.getDefineFLOAT("GAME_CITY_SIZE_MAX_PERCENT_UNIQUE");
	int iMaxNumUniques = (int)(fMaxUniquePercent * iTotalVisibleBuildings);

	// compute how many buildings are generics vs. unique Civ buildings?
	int iNumGenerics;
	int iNumUniques;
	if((int)kVisible.size() > iMaxNumUniques)
	{
		iNumUniques = iMaxNumUniques;
	}
	else 
	{
		iNumUniques = kVisible.size();
	}
	iNumGenerics = iTotalVisibleBuildings - iNumUniques + getCitySizeBoost();
	
	// return
	iChosenNumGenerics = iNumGenerics;
	for(int i = 0; i < iNumUniques; i++)
	{
		kChosenVisible.push_back(kVisible[i]);
	}
}

static int natGetDeterministicRandom(int iMin, int iMax, int iSeedX, int iSeedY)
{
	srand(7297 * iSeedX + 2909  * iSeedY);
	return (rand() % (iMax - iMin)) + iMin;
}

void CvCity::getVisibleEffects(ZoomLevelTypes eCurZoom, std::vector<const TCHAR*>& kEffectNames) const
{
	if (isOccupation() && isVisible(getTeam(), false))
	{
		if (eCurZoom  == ZOOM_DETAIL)
		{
			kEffectNames.push_back("EFFECT_CITY_BIG_BURNING_SMOKE");
			kEffectNames.push_back("EFFECT_CITY_FIRE");
		}
		else
		{
			kEffectNames.push_back("EFFECT_CITY_BIG_BURNING_SMOKE");
		}
		return;
	} 
}

void CvCity::getCityBillboardSizeIconColors(NiColorA& kDotColor, NiColorA& kTextColor) const
{
	NiColorA kPlayerColor = GC.getColorInfo((ColorTypes) GC.getPlayerColorInfo(GET_PLAYER(getOwnerINLINE()).getPlayerColor()).getColorTypePrimary()).getColor();
	NiColorA kGrowing;
	kGrowing = NiColorA(0.73f,1,0.73f,1);
	NiColorA kShrinking(1,0.73f,0.73f,1);
	NiColorA kStagnant(0.83f,0.83f,0.83f,1);
	NiColorA kUnknown(.5f,.5f,.5f,1);
	NiColorA kWhite(1,1,1,1);
	NiColorA kBlack(0,0,0,1);

	if ((getTeam() == GC.getGameINLINE().getActiveTeam()))
	{
		if (foodDifference() < 0)
		{
			if ((foodDifference() == -1) && (getFood() >= ((75 * growthThreshold()) / 100)))
			{
				kDotColor = kStagnant;
				kTextColor = kBlack;	
			}
			else
			{
				kDotColor = kShrinking;
				kTextColor = kBlack;
			}
		}
		else if (foodDifference() > 0)
		{
			kDotColor = kGrowing;
			kTextColor = kBlack;
		}
		else if (foodDifference() == 0)
		{
			kDotColor = kStagnant;
			kTextColor = kBlack;
		}
	}
	else
	{
		kDotColor = kPlayerColor;
		NiColorA kPlayerSecondaryColor = GC.getColorInfo((ColorTypes) GC.getPlayerColorInfo(GET_PLAYER(getOwnerINLINE()).getPlayerColor()).getColorTypeSecondary()).getColor();
		kTextColor = kPlayerSecondaryColor;
	}
}

const TCHAR* CvCity::getCityBillboardProductionIcon() const
{
	if (isNative() && getOwnerINLINE() != GC.getGameINLINE().getActivePlayer() && AI_getDesiredYield() != NO_YIELD)
	{
		return GC.getYieldInfo(AI_getDesiredYield()).getButton();
	}

	if (canBeSelected() && isProduction())
	{
		CLLNode<OrderData>* pOrderNode;
		pOrderNode = headOrderQueueNode();
		FAssert(pOrderNode != NULL);

		const CvInfoBase* pInfo = getProductionInfo();
		if (pInfo != NULL)
		{
			return pInfo->getButton();
		}
	}

	return ARTFILEMGR.getInterfaceArtInfo("INTERFACE_BUTTONS_NOPRODUCTION")->getPath();
}

bool CvCity::getCityBillboardTopBarValues(float& fStored, float& fRate, float& fRateExtra) const
{
	if (!canBeSelected())
	{
		return false;
	}

	// food bar
	float fThreshold = std::max(1.0f, (float) growthThreshold());
	if (foodDifference() < 0)
	{
		fStored = std::max(0.0f, (float) (getFood() + foodDifference())) / fThreshold;
		fRate = 0;
		fRateExtra = (float) std::min(-foodDifference(), getFood()) / fThreshold;
	}
	else
	{
		fStored = (float) getFood() / fThreshold;
		fRate = (float) foodDifference() / fThreshold;
		fRateExtra = 0;
	}

	return true;
}

bool CvCity::getCityBillboardBottomBarValues(float& fStored, float& fRate, float& fRateExtra) const
{
	// configure the production bars
	if (!canBeSelected() && !isScoutVisited(GC.getGameINLINE().getActiveTeam()))
	{
		return false;
	}

	if (!isNative() || getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
	{
		float fProductionDiff = (float) getCurrentProductionDifference(true);
		float fProductionNeeded = std::max(1.0f, (float) getProductionNeeded(YIELD_HAMMERS));

		fStored = (float) getProduction() / fProductionNeeded;
		fRate = fProductionDiff / fProductionNeeded;
		fRateExtra = 0.0f;
	}
	else
	{
		fStored = 0;
		fRate = 0;
		fRateExtra = 0;

		// show training progress
		int iBestTimer = MAX_INT;
		CvUnit* pBestUnit = NULL;
		CvPlot* pPlot = plot();
		CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();
		while (pUnitNode != NULL)
		{
			CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = plot()->nextUnitNode(pUnitNode);

			if (pLoopUnit->getUnitTravelState() == UNIT_TRAVEL_STATE_LIVE_AMONG_NATIVES)
			{
				if (pLoopUnit->getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
				{
					if (pLoopUnit->getUnitTravelTimer() < iBestTimer)
					{
						iBestTimer = pLoopUnit->getUnitTravelTimer();
						pBestUnit = pLoopUnit;
					}
				}
			}
		}

		if (pBestUnit != NULL)
		{
			int iLearnTime = pBestUnit->getLearnTime();
			float fLearnTime = std::max(1.0f, (float) iLearnTime);
			fStored = (iLearnTime - iBestTimer) / fLearnTime;
			fRate = 1.0f / fLearnTime;
		}
	}

	return true;
}

bool CvCity::isValidBuildingLocation(BuildingTypes eBuilding) const
{
	// if both the river and water flags are set, we require one of the two conditions, not both
	if (GC.getBuildingInfo(eBuilding).isWater())
	{
		if (!GC.getBuildingInfo(eBuilding).isRiver() || !plot()->isRiver())
		{
			if (!isCoastal(GC.getBuildingInfo(eBuilding).getMinAreaSize()))
			{
				return false;
			}
		}
	}
	else
	{
		if (area()->getNumTiles() < GC.getBuildingInfo(eBuilding).getMinAreaSize())
		{
			return false;
		}

		if (GC.getBuildingInfo(eBuilding).isRiver())
		{
			if (!(plot()->isRiver()))
			{
				return false;
			}
		}
	}

	return true;
}

int CvCity::getTriggerValue(EventTriggerTypes eTrigger) const
{
	FAssert(eTrigger >= 0);
	FAssert(eTrigger < GC.getNumEventTriggerInfos());

	CvEventTriggerInfo& kTrigger = GC.getEventTriggerInfo(eTrigger);


	if (!isEmpty(kTrigger.getPythonCanDoCity()))
	{
		long lResult;

		CyArgsList argsList;
		argsList.add(eTrigger);
		argsList.add(getOwnerINLINE());
		argsList.add(getID());

		gDLL->getPythonIFace()->callFunction(PYRandomEventModule, kTrigger.getPythonCanDoCity(), argsList.makeFunctionArgs(), &lResult);

		if (0 == lResult)
		{
			return MIN_INT;
		}
	}

	if (kTrigger.getNumBuildings() > 0 && kTrigger.getNumBuildingsRequired() > 0)
	{
		bool bFoundValid = false;

		for (int i = 0; i < kTrigger.getNumBuildingsRequired(); ++i)
		{
			if (kTrigger.getBuildingRequired(i) != NO_BUILDINGCLASS)
			{
				BuildingTypes eBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(kTrigger.getBuildingRequired(i));
				if (NO_BUILDING != eBuilding)
				{
					if (isHasRealBuilding(eBuilding))
					{
						bFoundValid = true;
					}
				}
			}
		}

		if (!bFoundValid)
		{
			return MIN_INT;
		}
	}

	if (kTrigger.getMinPopulation() > 0)
	{
		if (getPopulation() < kTrigger.getMinPopulation())
		{
			return MIN_INT;
		}
	}

	if (kTrigger.getMaxPopulation() > 0)
	{
		if (getPopulation() > kTrigger.getMaxPopulation())
		{
			return MIN_INT;
		}
	}

	if (kTrigger.isPrereqEventCity() && kTrigger.getNumPrereqEvents() > 0)
	{
		bool bFoundValid = true;

		for (int iI = 0; iI < kTrigger.getNumPrereqEvents(); ++iI)
		{
			if (!isEventOccured((EventTypes)kTrigger.getPrereqEvent(iI)))
			{
				bFoundValid = false;
				break;
			}
		}

		if (!bFoundValid)
		{
			return MIN_INT;
		}
	}


	int iValue = 0;

	if (0 == getFood() && kTrigger.getCityFoodWeight() > 0)
	{
		return MIN_INT;
	}

	iValue += getFood() * kTrigger.getCityFoodWeight();

	return iValue;
}

bool CvCity::canApplyEvent(EventTypes eEvent, const EventTriggeredData& kTriggeredData) const
{
	CvEventInfo& kEvent = GC.getEventInfo(eEvent);

	if (!kEvent.isCityEffect() && !kEvent.isOtherPlayerCityEffect())
	{
		return true;
	}

	if (-1 == kTriggeredData.m_iCityId && kEvent.isCityEffect())
	{
		return false;
	}

	if (-1 == kTriggeredData.m_iOtherPlayerCityId && kEvent.isOtherPlayerCityEffect())
	{
		return false;
	}

	if (kEvent.getFood() + ((100 + kEvent.getFoodPercent()) * getFood()) / 100 < 0)
	{
		return false;
	}

	if (kEvent.getPopulationChange() + getPopulation() <= 0)
	{
		return false;
	}

	if (kEvent.getCulture() + getCulture(getOwnerINLINE()) < 0)
	{
		return false;
	}

	if (kEvent.getBuildingClass() != NO_BUILDINGCLASS)
	{
		BuildingTypes eBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(kEvent.getBuildingClass());
		if (eBuilding == NO_BUILDING)
		{
			return false;
		}

		if (kEvent.getBuildingChange() > 0)
		{
			if (isHasRealBuilding(eBuilding))
			{
				return false;
			}
		}
		else if (kEvent.getBuildingChange() < 0)
		{
			if (!isHasRealBuilding(eBuilding) && kEvent.getBuildingChange() < 0)
			{
				return false;
			}
		}
	}

	if (kEvent.getMinPillage() > 0)
	{
		int iNumImprovements = 0;
		for (int i = 0; i < NUM_CITY_PLOTS; ++i)
		{
			if (CITY_HOME_PLOT != i)
			{
				CvPlot* pPlot = getCityIndexPlot(i);
				if (NULL != pPlot && pPlot->getOwnerINLINE() == getOwnerINLINE())
				{
					if (NO_IMPROVEMENT != pPlot->getImprovementType() && !GC.getImprovementInfo(pPlot->getImprovementType()).isPermanent())
					{
						++iNumImprovements;
					}
				}
			}
		}

		if (iNumImprovements < kEvent.getMinPillage())
		{
			return false;
		}
	}

	return true;
}

void CvCity::applyEvent(EventTypes eEvent, const EventTriggeredData& kTriggeredData, bool bClear)
{
	if (!canApplyEvent(eEvent, kTriggeredData))
	{
		return;
	}

	setEventOccured(eEvent, true);

	CvEventInfo& kEvent = GC.getEventInfo(eEvent);

	if (kEvent.isCityEffect() || kEvent.isOtherPlayerCityEffect())
	{
		if (kEvent.getFood() != 0 || kEvent.getFoodPercent() != 0)
		{
			changeFood(kEvent.getFood() + (kEvent.getFoodPercent() * getFood()) / 100);
		}

		if (kEvent.getPopulationChange() != 0)
		{
			changePopulation(kEvent.getPopulationChange());
		}

		if (kEvent.getRevoltTurns() > 0)
		{
			changeCultureUpdateTimer(kEvent.getRevoltTurns());
			changeOccupationTimer(kEvent.getRevoltTurns());
		}

		if (kEvent.getMaxPillage() > 0)
		{
			FAssert(kEvent.getMaxPillage() >= kEvent.getMinPillage());
			int iNumPillage = kEvent.getMinPillage() + GC.getGameINLINE().getSorenRandNum(kEvent.getMaxPillage() - kEvent.getMinPillage(), "Pick number of event pillaged plots");

			int iNumPillaged = 0;
			for (int i = 0; i < iNumPillage; ++i)
			{
				int iRandOffset = GC.getGameINLINE().getSorenRandNum(NUM_CITY_PLOTS, "Pick event pillage plot");
				for (int j = 0; j < NUM_CITY_PLOTS; ++j)
				{
					int iPlot = (j + iRandOffset) % NUM_CITY_PLOTS;
					if (CITY_HOME_PLOT != iPlot)
					{
						CvPlot* pPlot = getCityIndexPlot(iPlot);
						if (NULL != pPlot && pPlot->getOwnerINLINE() == getOwnerINLINE())
						{
							if (NO_IMPROVEMENT != pPlot->getImprovementType() && !GC.getImprovementInfo(pPlot->getImprovementType()).isPermanent())
							{
								CvWString szBuffer = gDLL->getText("TXT_KEY_EVENT_CITY_IMPROVEMENT_DESTROYED", GC.getImprovementInfo(pPlot->getImprovementType()).getTextKeyWide());
								gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_PILLAGED", MESSAGE_TYPE_INFO, GC.getImprovementInfo(pPlot->getImprovementType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
								pPlot->setImprovementType(NO_IMPROVEMENT);
								++iNumPillaged;
								break;
							}
						}
					}
				}
			}

			PlayerTypes eOtherPlayer = kTriggeredData.m_eOtherPlayer;
			if (!kEvent.isCityEffect() && kEvent.isOtherPlayerCityEffect())
			{
				eOtherPlayer = kTriggeredData.m_ePlayer;
			}

			if (NO_PLAYER != eOtherPlayer)
			{
				CvWString szBuffer = gDLL->getText("TXT_KEY_EVENT_NUM_CITY_IMPROVEMENTS_DESTROYED", iNumPillaged, GET_PLAYER(getOwnerINLINE()).getCivilizationAdjectiveKey());
				gDLL->getInterfaceIFace()->addMessage(eOtherPlayer, false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_PILLAGED", MESSAGE_TYPE_INFO);
			}
		}

		if (kEvent.getCulture() != 0)
		{
			changeCulture(getOwnerINLINE(), kEvent.getCulture(), true);
		}
	}


	if (kEvent.getUnitClass() != NO_UNITCLASS)
	{
		UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(kEvent.getUnitClass());
		if (eUnit != NO_UNIT)
		{
			for (int i = 0; i < kEvent.getNumUnits(); ++i)
			{
				GET_PLAYER(getOwnerINLINE()).initUnit(eUnit, (ProfessionTypes) GC.getUnitInfo(eUnit).getDefaultProfession(), getX_INLINE(), getY_INLINE());
			}
		}
	}

	if (kEvent.getBuildingClass() != NO_BUILDINGCLASS)
	{
		BuildingTypes eBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(kEvent.getBuildingClass());
		if (eBuilding != NO_BUILDING)
		{
			if (0 != kEvent.getBuildingChange())
			{
				setHasRealBuilding(eBuilding, kEvent.getBuildingChange() > 0);
			}
		}
	}

	if (kEvent.getNumBuildingYieldChanges() > 0)
	{
		for (int iBuildingClass = 0; iBuildingClass < GC.getNumBuildingClassInfos(); ++iBuildingClass)
		{
			for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
			{
				setBuildingYieldChange((BuildingClassTypes)iBuildingClass, (YieldTypes)iYield, getBuildingYieldChange((BuildingClassTypes)iBuildingClass, (YieldTypes)iYield) + kEvent.getBuildingYieldChange(iBuildingClass, iYield));
			}
		}
	}

	if (bClear)
	{
		for (int i = 0; i < GC.getNumEventInfos(); ++i)
		{
			setEventOccured((EventTypes)i, false);
		}
	}
}

bool CvCity::isEventOccured(EventTypes eEvent) const
{
	for (std::vector<EventTypes>::const_iterator it = m_aEventsOccured.begin(); it != m_aEventsOccured.end(); ++it)
	{
		if (*it == eEvent)
		{
			return true;
		}
	}

	return false;
}

void CvCity::setEventOccured(EventTypes eEvent, bool bOccured)
{
	for (std::vector<EventTypes>::iterator it = m_aEventsOccured.begin(); it != m_aEventsOccured.end(); ++it)
	{
		if (*it == eEvent)
		{
			if (!bOccured)
			{
				m_aEventsOccured.erase(it);
			}
			return;
		}
	}

	if (bOccured)
	{
		m_aEventsOccured.push_back(eEvent);
	}
}

// CACHE: cache frequently used values
///////////////////////////////////////
void CvCity::invalidatePopulationRankCache()
{
	m_bPopulationRankValid = false;
}

void CvCity::invalidateYieldRankCache(YieldTypes eYield)
{
	FAssertMsg(eYield >= NO_YIELD && eYield < NUM_YIELD_TYPES, "invalidateYieldRankCache passed bogus yield index");

	if (eYield == NO_YIELD)
	{
		for (int iI = 0; iI < NUM_YIELD_TYPES; iI++)
		{
			m_abBaseYieldRankValid[iI] = false;
			m_abYieldRankValid[iI] = false;
		}
	}
	else
	{
		m_abBaseYieldRankValid[eYield] = false;
		m_abYieldRankValid[eYield] = false;
	}
}

int CvCity::getBuildingYieldChange(BuildingClassTypes eBuildingClass, YieldTypes eYield) const
{
	for (std::vector<BuildingYieldChange>::const_iterator it = m_aBuildingYieldChange.begin(); it != m_aBuildingYieldChange.end(); ++it)
	{
		if ((*it).eBuildingClass == eBuildingClass && (*it).eYield == eYield)
		{
			return (*it).iChange;
		}
	}

	return 0;
}

void CvCity::setBuildingYieldChange(BuildingClassTypes eBuildingClass, YieldTypes eYield, int iChange)
{
	for (std::vector<BuildingYieldChange>::iterator it = m_aBuildingYieldChange.begin(); it != m_aBuildingYieldChange.end(); ++it)
	{
		if ((*it).eBuildingClass == eBuildingClass && (*it).eYield == eYield)
		{
			int iOldChange = (*it).iChange;
			if (iOldChange != iChange)
			{

				if (iChange == 0)
				{
					m_aBuildingYieldChange.erase(it);
				}
				else
				{
					(*it).iChange = iChange;
				}

				BuildingTypes eBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(eBuildingClass);
				if (NO_BUILDING != eBuilding)
				{
					if (isHasBuilding(eBuilding))
					{
						setYieldRateDirty();
					}
				}
			}

			return;
		}
	}

	if (0 != iChange)
	{
		BuildingYieldChange kChange;
		kChange.eBuildingClass = eBuildingClass;
		kChange.eYield = eYield;
		kChange.iChange = iChange;
		m_aBuildingYieldChange.push_back(kChange);

		BuildingTypes eBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(eBuildingClass);
		if (NO_BUILDING != eBuilding)
		{
			if (isHasBuilding(eBuilding))
			{
				setYieldRateDirty();
			}
		}
	}
}

void CvCity::changeBuildingYieldChange(BuildingClassTypes eBuildingClass, YieldTypes eYield, int iChange)
{
	setBuildingYieldChange(eBuildingClass, eYield, getBuildingYieldChange(eBuildingClass, eYield) + iChange);
}

void CvCity::liberate(bool bConquest)
{
	CvPlot* pPlot = plot();
	PlayerTypes ePlayer = getLiberationPlayer(bConquest);
	PlayerTypes eOwner = getOwnerINLINE();

	if (NO_PLAYER != ePlayer)
	{
		int iOldOwnerCulture = getCulture(eOwner);

		CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_CITY_LIBERATED", getNameKey(), GET_PLAYER(eOwner).getNameKey(), GET_PLAYER(ePlayer).getCivilizationAdjectiveKey());
		for (int iI = 0; iI < MAX_PLAYERS; ++iI)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if (isRevealed(GET_PLAYER((PlayerTypes)iI).getTeam(), false))
				{
					gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_REVOLTEND", MESSAGE_TYPE_MAJOR_EVENT, ARTFILEMGR.getInterfaceArtInfo("WORLDBUILDER_CITY_EDIT")->getPath(), (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"), getX_INLINE(), getY_INLINE(), true, true);
				}
			}
		}
		GC.getGameINLINE().addReplayMessage(REPLAY_MESSAGE_MAJOR_EVENT, eOwner, szBuffer, getX_INLINE(), getY_INLINE(), (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"));

		GET_PLAYER(ePlayer).acquireCity(this, false, true);
		GET_PLAYER(ePlayer).AI_changeMemoryCount(eOwner, MEMORY_LIBERATED_CITIES, 1);

		if (NULL != pPlot)
		{
			CvCity* pCity = pPlot->getPlotCity();
			if (NULL != pCity)
			{
				pCity->setCulture(ePlayer, pCity->getCulture(ePlayer) + iOldOwnerCulture / 2, true);
			}
		}
	}
}

PlayerTypes CvCity::getLiberationPlayer(bool bConquest) const
{
	if (isCapital())
	{
		return NO_PLAYER;
	}

	if (isNative())
	{
		return NO_PLAYER;
	}

	for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
	{
		CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);
		if (kLoopPlayer.isAlive() && kLoopPlayer.getParent() == getOwnerINLINE())
		{
			CvCity* pLoopCapital = kLoopPlayer.getPrimaryCity();
			if (NULL != pLoopCapital)
			{
				if (pLoopCapital->area() == area())
				{
					return (PlayerTypes)iPlayer;
				}
			}
		}
	}

	CvPlayer& kOwner = GET_PLAYER(getOwnerINLINE());

	PlayerTypes eBestPlayer = NO_PLAYER;
	int iBestValue = 0;

	int iTotalCulture = countTotalCulture();

	for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
	{
		CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);

		if (kLoopPlayer.isAlive())
		{
			if (kLoopPlayer.canReceiveTradeCity(getOwnerINLINE()))
			{
				CvCity* pCapital = kLoopPlayer.getPrimaryCity();
				if (NULL != pCapital)
				{
					int iCapitalDistance = ::plotDistance(getX_INLINE(), getY_INLINE(), pCapital->getX_INLINE(), pCapital->getY_INLINE());
					if (area() != pCapital->area())
					{
						iCapitalDistance *= 2;
					}

					int iCulture = getCulture((PlayerTypes)iPlayer);

					if (bConquest)
					{
						if (iPlayer == getOriginalOwner())
						{
							iCulture *= 3;
							iCulture /= 2;
						}
					}

					if (GET_PLAYER((PlayerTypes)iPlayer).getTeam() == getTeam())
					{
						iCulture *= 2;
						iCulture = (iCulture + iTotalCulture) / 2;
					}

					int iValue = std::max(1, iCulture) / std::max(1, iCapitalDistance);

					if (iValue > iBestValue)
					{
						iBestValue = iValue;
						eBestPlayer = (PlayerTypes)iPlayer;
					}
				}
			}
		}
	}

	if (NO_PLAYER != eBestPlayer)
	{
		if (getOwnerINLINE() == eBestPlayer)
		{
			return NO_PLAYER;
		}

		for (int iPlot = 0; iPlot < NUM_CITY_PLOTS; ++iPlot)
		{
			CvPlot* pLoopPlot = ::plotCity(getX_INLINE(), getY_INLINE(), iPlot);

			if (NULL != pLoopPlot)
			{
				if (pLoopPlot->isVisibleEnemyUnit(eBestPlayer))
				{
					return NO_PLAYER;
				}
			}
		}
	}

	return eBestPlayer;
}

int CvCity::getBestYieldAmountAvailable(ProfessionTypes eProfession, const CvUnit* pUnit) const
{
	if (eProfession == NO_PROFESSION)
	{
		return 0;
	}

	FAssert(GC.getProfessionInfo(eProfession).getYieldProduced() != NO_YIELD);

	int iBestYieldAvailable = 0;

	if(pUnit != NULL)
	{
		CvPlot* pWorkingPlot = getPlotWorkedByUnit(pUnit);
		if(pWorkingPlot != NULL)
		{
			iBestYieldAvailable = pWorkingPlot->calculatePotentialProfessionYieldAmount(eProfession, pUnit, false);
		}
	}

	for (int iJ = 0; iJ < NUM_CITY_PLOTS; ++iJ)
	{
		if (iJ != CITY_HOME_PLOT)
		{
			if (!isUnitWorkingPlot(iJ))
			{
				CvPlot* pPlot = getCityIndexPlot(iJ);

				if (NULL != pPlot && canWork(pPlot))
				{
					int iYieldAmount = pPlot->calculatePotentialProfessionYieldAmount(eProfession, pUnit, false);
					if (iYieldAmount > iBestYieldAvailable)
					{
						iBestYieldAvailable = iYieldAmount;
					}
				}
			}
		}
	}

	if (pUnit->canHaveProfession(eProfession, false))
	{
		iBestYieldAvailable = std::max(iBestYieldAvailable, getProfessionOutput(eProfession, pUnit));
	}

	return iBestYieldAvailable;
}

void CvCity::addPopulationUnit(CvUnit* pUnit, ProfessionTypes eProfession)
{
	if (NULL == pUnit)
	{
		FAssert(false);
		return;
	}
	
	if ((getPopulation() == 0) && (GC.getDefineINT("CONSUME_EQUIPMENT_ON_FOUND") != 0))
	{
		// Pioneers consume tools when founding
		// must do this before joining the city
		pUnit->setProfession(eProfession); 
	}

	CvUnit* pTransferUnit = GET_PLAYER(pUnit->getOwnerINLINE()).getAndRemoveUnit(pUnit->getID());
	FAssert(pTransferUnit == pUnit);

	int iOldPopulation = getPopulation();
	m_aPopulationUnits.push_back(pTransferUnit);
	area()->changePower(getOwnerINLINE(), pTransferUnit->getPower());
	setYieldRateDirty();
	pTransferUnit->setProfession(eProfession);
	
	pTransferUnit->setColonistLocked(false);

	updatePopulation(iOldPopulation);

	gDLL->getEventReporterIFace()->populationJoined(getOwnerINLINE(), getID(), pTransferUnit->getID());
}

bool CvCity::removePopulationUnit(CvUnit* pUnit, bool bDelete, ProfessionTypes eProfession)
{
	int iUnitIndex = getPopulationUnitIndex(pUnit);
	if(iUnitIndex < 0)
	{
        FAssertMsg(false, "Could not find unit in city");
		return false;
	}

	if (!pUnit->canHaveProfession(eProfession, false))
	{
		FAssertMsg(false, "Illegal Profession");
		pUnit->setProfession(NO_PROFESSION);
		return false;
	}

	pUnit->setColonistLocked(false);

	//remove unit from worked plots
	CvPlot* pWorkedPlot = getPlotWorkedByUnit(pUnit);
	if (pWorkedPlot != NULL)
	{
		clearUnitWorkingPlot(pWorkedPlot);
	}

	int iOldPopulation = getPopulation();

	FAssert(pUnit->getOwnerINLINE() == getOwnerINLINE());
	m_aPopulationUnits.erase(std::remove(m_aPopulationUnits.begin(), m_aPopulationUnits.end(), pUnit));
	area()->changePower(getOwnerINLINE(), -pUnit->getPower());
	setYieldRateDirty();

	updatePopulation(iOldPopulation);

	if (bDelete)
	{
		pUnit->updateOwnerCache(-1);
		SAFE_DELETE(pUnit);
	}
	else
	{
		//transfer back to player
		GET_PLAYER(getOwnerINLINE()).addExistingUnit(pUnit);
		pUnit->addToMap(getX_INLINE(), getY_INLINE());
		pUnit->setProfession(eProfession);

		if (pUnit->getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
		{
			gDLL->getInterfaceIFace()->insertIntoSelectionList(pUnit, true, false);
		}

		gDLL->getEventReporterIFace()->populationUnjoined(getOwnerINLINE(), getID(), pUnit->getID());
	}

	return true;
}

CvUnit* CvCity::removeUnitType(UnitTypes eUnit, ProfessionTypes eProfession)
{
	for (uint i = 0; i < m_aPopulationUnits.size(); ++i)
	{
		CvUnit* pUnit = m_aPopulationUnits[i];

		if (pUnit->getUnitType() == eUnit)
		{
			if (removePopulationUnit(pUnit, false, eProfession))
			{
				return pUnit;
			}
		}
	}

	return NULL;
}


void CvCity::removeNonCityPopulationUnits()
{
	bool bDone = false;
	while (!bDone)
	{
		bDone = true;
		for (uint i=0; i < m_aPopulationUnits.size(); i++)
		{
			CvUnit* pUnit = m_aPopulationUnits[i];
			ProfessionTypes eUnitProfession = pUnit->getProfession();
			if (NO_PROFESSION != eUnitProfession && !GC.getProfessionInfo(eUnitProfession).isCitizen())
			{
				//unit list changes, so break and repeat
				removePopulationUnit(pUnit, false, eUnitProfession);
				bDone = false;
				break;
			}
		}
	}
}

CvUnit* CvCity::getPopulationUnitById(int iUnitId) const
{
	for (uint i = 0; i < m_aPopulationUnits.size(); ++i)
	{
		CvUnit* pUnit = m_aPopulationUnits[i];
		if (pUnit->getID() == iUnitId)
		{
			return pUnit;
		}
	}

	return NULL;
}

CvUnit* CvCity::getPopulationUnitByIndex(int iUnitIndex) const
{
	if((iUnitIndex >= 0) && (iUnitIndex < (int)m_aPopulationUnits.size()))
	{
		return m_aPopulationUnits[iUnitIndex];
	}
	
	FAssert(false);
	return NULL;
}

int CvCity::getPopulationUnitIndex(CvUnit *pUnit) const
{
	for(int i=0;i<(int)m_aPopulationUnits.size();i++)
	{
		if(m_aPopulationUnits[i] == pUnit)
		{
			return i;
		}
	}

	return -1;
}

CvPlot* CvCity::getPlotWorkedByUnit(const CvUnit* pUnit) const
{
	for (int i = 0; i < NUM_CITY_PLOTS; ++i)
	{
		int iUnitId = m_paiWorkingPlot[i];
		if (iUnitId == pUnit->getID())
		{
			return getCityIndexPlot(i);
		}
	}

	return NULL;
}

CvUnit* CvCity::createYieldUnit(YieldTypes eYield, PlayerTypes ePlayer, int iYieldAmount)
{
	FAssert(getYieldStored(eYield) > 0);
	FAssert(iYieldAmount > 0);
	FAssert(ePlayer != NO_PLAYER);

	CvUnit* pUnit = NULL;
	CvPlayer& kOwner = GET_PLAYER(ePlayer);
	UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(kOwner.getCivilizationType()).getCivilizationUnits(GC.getYieldInfo(eYield).getUnitClass());
	if (NO_UNIT != eUnit)
	{
		pUnit = kOwner.initUnit(eUnit, (ProfessionTypes) GC.getUnitInfo(eUnit).getDefaultProfession(), getX_INLINE(), getY_INLINE(), NO_UNITAI, NO_DIRECTION, iYieldAmount);
		FAssertMsg(pUnit != NULL, "pUnit expected to be assigned (not NULL)");

		FAssert(iYieldAmount <= getYieldStored(eYield));
		changeYieldStored(eYield, -iYieldAmount);
		AI_changeTradeBalance(eYield, -iYieldAmount);
		if (NULL != pUnit)
		{
			pUnit->setHomeCity(this);
		}
	}

	return pUnit;
}

UnitClassTypes CvCity::getTeachUnitClass() const
{
	return m_eTeachUnitClass;
}

void CvCity::setTeachUnitClass(UnitClassTypes eUnitClass)
{
	m_eTeachUnitClass = eUnitClass;
}

void CvCity::ejectTeachUnits()
{
	CvPlot *pPlot = plot();
	CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();
	while (pUnitNode)
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pPlot->nextUnitNode(pUnitNode);

		if (pLoopUnit->getUnitTravelState() == UNIT_TRAVEL_STATE_LIVE_AMONG_NATIVES)
		{
			pLoopUnit->setUnitTravelState(NO_UNIT_TRAVEL_STATE, false);
			pLoopUnit->setUnitTravelTimer(0);
		}
	}
}

bool CvCity::canProduceYield(YieldTypes eYield)
{
	CvPlayer& kPlayer = GET_PLAYER(getOwnerINLINE());
	for (int i = 0; i < GC.getNumProfessionInfos(); i++)
	{
		ProfessionTypes eLoopProfession = (ProfessionTypes)i;
		CvProfessionInfo& kLoopProfession = GC.getProfessionInfo(eLoopProfession);
		if (GC.getCivilizationInfo(kPlayer.getCivilizationType()).isValidProfession(eLoopProfession))
		{
			if (kLoopProfession.getYieldProduced() == eYield)
			{
				if (kLoopProfession.isWorkPlot())
				{
					for (int j = 0; j < NUM_CITY_PLOTS; ++j)
					{
						CvPlot* pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), j);
						if (pLoopPlot != NULL)
						{
							if (pLoopPlot->calculateYield(eYield, false) > 0)
							{
								return true;
							}
						}
					}
				}
				else
				{
					if (getNumProfessionBuildingSlots(eLoopProfession) > 0)
					{
						return true;
					}
				}
			}
		}
	}
	
	return false;
}

int CvCity::getMaxYieldCapacity() const
{
	int iCapacity = GC.getGameINLINE().getCargoYieldCapacity();

	for (int iBuildingClass = 0; iBuildingClass < GC.getNumBuildingClassInfos(); ++iBuildingClass)
	{
		BuildingTypes eBuilding = (BuildingTypes) GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iBuildingClass);
		if (eBuilding != NO_BUILDING)
		{
			if (isHasBuilding(eBuilding))
			{
				iCapacity += GC.getBuildingInfo(eBuilding).getYieldStorage() * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getStoragePercent() / 100;
			}
		}
	}

	return iCapacity;
}

bool CvCity::isAutoRaze() const
{
	if (getPopulation() == 0)
	{
		return true;
	}

	if (!GC.getGameINLINE().isOption(GAMEOPTION_NO_CITY_RAZING))
	{
		if (getHighestPopulation() == 1)
		{
			return true;
		}

		if (GC.getGameINLINE().getMaxCityElimination() > 0)
		{
			return true;
		}
	}

	if (GC.getGameINLINE().isOption(GAMEOPTION_ONE_CITY_CHALLENGE) && isHuman())
	{
		return true;
	}

	if (getPreviousOwner() != NO_PLAYER)
	{
		if (GET_PLAYER(getPreviousOwner()).isNative() != isNative())
		{
			return true;
		}
	}

	return false;
}

void CvCity::setScoutVisited(TeamTypes eTeam, bool bVisited)
{
	FAssert(eTeam >= 0);
	FAssert(eTeam < MAX_TEAMS);

	if(bVisited != isScoutVisited(eTeam))
	{
		m_abScoutVisited[eTeam] = bVisited;
		setBillboardDirty(true);
	}
}


bool CvCity::isScoutVisited(TeamTypes eTeam) const
{
	FAssert(eTeam >= 0 && eTeam < MAX_TEAMS);
	if (eTeam == getTeam())
	{
		return true;
	}
	return m_abScoutVisited[eTeam];
}


GoodyTypes CvCity::getGoodyType(const CvUnit* pUnit) const
{
	if (!isNative())
	{
		return NO_GOODY;
	}

	for (int iTeam = 0; iTeam < MAX_TEAMS; ++iTeam)
	{
		if (iTeam != getTeam())
		{
			if (isScoutVisited((TeamTypes) iTeam))
			{
				return NO_GOODY;
			}
		}
	}

	int iBestValue = 0;
	GoodyTypes eBestGoody = NO_GOODY;
	for (int i = 0; i < GC.getHandicapInfo(getHandicapType()).getNumGoodies(); i++)
	{
		GoodyTypes eGoody = (GoodyTypes) GC.getHandicapInfo(getHandicapType()).getGoodies(i);

		FAssert(eGoody >= 0);
		FAssert(eGoody < GC.getNumGoodyInfos());

		if (pUnit == NULL || GET_PLAYER(pUnit->getOwnerINLINE()).canReceiveGoody(plot(), eGoody, pUnit))
		{
			int iRandValue = GC.getGameINLINE().getSorenRandNum(GC.getGoodyInfo(eGoody).getCityGoodyWeight(), "City Goodies");
			if(iRandValue > iBestValue)
			{
				iBestValue = iRandValue;
				eBestGoody = eGoody;
			}
		}
	}

	return eBestGoody;
}

int CvCity::doGoody(CvUnit* pUnit, GoodyTypes eGoody)
{
	int iValue = -1;
	FAssert(pUnit != NULL);

	CvPlayer& kPlayer = GET_PLAYER(pUnit->getOwnerINLINE());
	if (eGoody != NO_GOODY)
	{
		iValue = kPlayer.receiveGoody(plot(), eGoody, pUnit);
	}

	setScoutVisited(kPlayer.getTeam(), true);

	for (int i = 0; i < GC.getNumFatherPointInfos(); ++i)
	{
		FatherPointTypes ePointType = (FatherPointTypes) i;
		GET_PLAYER(pUnit->getOwnerINLINE()).changeFatherPoints(ePointType, GC.getFatherPointInfo(ePointType).getScoutVillagePoints());
	}

	return iValue;
}

CivilizationTypes CvCity::getMissionaryCivilization() const
{
	return m_eMissionaryCivilization;
}

void CvCity::setMissionaryCivilization(CivilizationTypes eCivilization)
{
	if (eCivilization != getMissionaryCivilization())
	{
		CivilizationTypes eOldCivilization = getMissionaryCivilization();

		m_eMissionaryCivilization = eCivilization;

		if (eOldCivilization != NO_CIVILIZATION)
		{
			CvWString szBuffer = gDLL->getText("TXT_KEY_MISSION_REMOVED", getNameKey(), GC.getCivilizationInfo(eOldCivilization).getAdjectiveKey());

			for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
			{
				CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes) iPlayer);
				if (kLoopPlayer.isAlive())
				{
					if (isRevealed(kLoopPlayer.getTeam(), false))
					{
						gDLL->getInterfaceIFace()->addMessage(kLoopPlayer.getID(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_DEAL_CANCELLED", MESSAGE_TYPE_MINOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"), getX_INLINE(), getY_INLINE(), false, false);
					}
				}
			}
		}

		if (getMissionaryCivilization() != NO_CIVILIZATION)
		{
			CvWString szBuffer = gDLL->getText("TXT_KEY_MISSION_ESTABLISHED", getNameKey(), GC.getCivilizationInfo(getMissionaryCivilization()).getAdjectiveKey());

			for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
			{
				CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes) iPlayer);
				if (kLoopPlayer.isAlive())
				{
					if (isRevealed(kLoopPlayer.getTeam(), false))
					{
						gDLL->getInterfaceIFace()->addMessage(kLoopPlayer.getID(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_POSITIVE_DINK", MESSAGE_TYPE_MINOR_EVENT, GC.getCommandInfo(COMMAND_ESTABLISH_MISSION).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"), getX_INLINE(), getY_INLINE(), true, true);
					}
				}
			}
		}

		setBillboardDirty(true);
	}
}

int CvCity::getMissionaryRate() const
{
	return m_iMissionaryRate;
}

void CvCity::setMissionaryRate(int iRate)
{
	m_iMissionaryRate = iRate;
}

//This function returns the (random) player for which a converted native should be spawned.
//If eTeam is passed in, it instead calculates the expected odds that the unit will
//spawn for that team.
PlayerTypes CvCity::calculateConvertRecipient(TeamTypes eTeam, int* piOdds)
{
	if (eTeam != NO_TEAM)
	{
		FAssert(piOdds != NULL);
		if (piOdds != NULL)
		{
			*piOdds = 0;
		}
	}
	
	int iMaxRange = MAX_INT;//std::max(10, GC.getMap().maxPlotDistance() / 4);
	
	
	std::vector<int> values(MAX_PLAYERS, 0);
	
	int iMaxValue = 0;
	
	CvPlayer& kOwner = GET_PLAYER(getOwnerINLINE());
	for (int iPlayer = 0; iPlayer < MAX_PLAYERS; iPlayer++)
	{
		PlayerTypes eLoopPlayer = (PlayerTypes)iPlayer;
		CvPlayerAI& kLoopPlayer = GET_PLAYER(eLoopPlayer);
		if (kLoopPlayer.isAlive())
		{
			if (kOwner.canHaveMission(kLoopPlayer.getCivilizationType()))
			{
				int iValue = 0;
				
				int iLoop;
				CvCity* pLoopCity;
				for (pLoopCity = kLoopPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kLoopPlayer.nextCity(&iLoop))
				{
					int iDistance = plotDistance(getX_INLINE(), getY_INLINE(), pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE());
					FAssert(iDistance > 0);
					
					if (iDistance < iMaxRange)
					{
						int iCityValue = pLoopCity->getPopulation() * 100;
						if (pLoopCity->getArea() == getArea())
						{
							//Penalize cities on other areas 100-fold.
							iCityValue *= 100;
						}
						
						iCityValue /= std::max(1, iDistance);
						iCityValue++;
					}
				}
 
				if (iValue > 0)
				{
					values[iPlayer] = iValue;
					int iMaxValue = std::max(iMaxValue, iValue);
				}
			}
		}
	}
	
	//Eliminate any players which have less than 20% of the highest.
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (values[i] < (iMaxValue / 5))
		{
			values[i] = 0;
		}
	}

	//For if we want the player.
	int iBestValue = 0;
	PlayerTypes eBestPlayer = NO_PLAYER;
	
	//For if we want the odds.
	int iOdds = 0;
	int iTotal = 0;
	
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		if (values[i] > 0)
		{
			if (eTeam == NO_TEAM)
			{
				int iValue = 1 + GC.getGameINLINE().getSorenRandNum(values[i], "AI Convert Recipient");
				if (iValue > iBestValue)
				{
					iBestValue = iValue;
					eBestPlayer = (PlayerTypes)i;
				}
			}
			else
			{
				iTotal += values[i];
				if (GET_PLAYER((PlayerTypes)i).getTeam() == eTeam)
				{
					iOdds += values[i];                    
				}
			}            
		}
	}
	
	if (eTeam != NO_TEAM)
	{
		*piOdds = (iOdds * 100) / std::max(1, iTotal);
	}
	return eBestPlayer;
}

void CvCity::doRebelSentiment()
{
	int iTurnFactor = std::max(1, GC.getDefineINT("REBEL_SENTIMENT_TURN_WEIGHT") * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getGrowthPercent() / 100);

	int iPast = (iTurnFactor - 1) * getRebelSentiment();
	int iNew = 0;

	if (!GET_PLAYER(getOwnerINLINE()).isEurope())
	{
		iNew = calculateNetYield(YIELD_BELLS) * GC.getDefineINT("REBEL_SENTIMENT_BELLS_FACTOR");
	}

	if (!isHuman())
	{
		iNew *= 100 + GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIRebelModifier();
		iNew /= 100;
	}

	setRebelSentiment((iNew + iPast) / iTurnFactor);
}

int CvCity::getRebelPercent() const
{
	return std::max(0, std::min(100, getRebelSentiment() / std::max(1, getPopulation())));
}

int CvCity::getRebelSentiment() const
{
	return m_iRebelSentiment;
}

void CvCity::setTeachUnitMultiplier(int iModifier)
{
	m_iTeachUnitMultiplier = iModifier;
}

int CvCity::getTeachUnitMultiplier() const
{
	return m_iTeachUnitMultiplier;
}

void CvCity::setRebelSentiment(int iValue)
{
	m_iRebelSentiment = iValue;
	FAssert(getRebelSentiment() >= 0);	
}

UnitClassTypes CvCity::bestTeachUnitClass()
{
	PROFILE_FUNC();
	int iBestValue = 0;
	UnitClassTypes eBestUnitClass = NO_UNITCLASS;
	
	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
	
	std::vector<int> values(GC.getNumUnitClassInfos(), 0);
	for (int i = 0; i < GC.getNumUnitClassInfos(); ++i)
	{
		if (GC.getCivilizationInfo(kOwner.getCivilizationType()).getTeachUnitClassWeight(i) > 0)
		{
			UnitTypes eLoopUnit = (UnitTypes)GC.getUnitClassInfo((UnitClassTypes) i).getDefaultUnitIndex();
			if (eLoopUnit != NO_UNIT)
			{
				
				int iValue = 0;
								
				CvUnitInfo& kUnitType = GC.getUnitInfo(eLoopUnit);
				ProfessionTypes eIdealProfession = kOwner.AI_idealProfessionForUnit(eLoopUnit);
		
				if (eIdealProfession == NO_PROFESSION)
				{
					iValue += 100;
				}
				else
				{
					CvProfessionInfo& kIdealProfession = GC.getProfessionInfo(eIdealProfession);
					
					if (!kIdealProfession.isCitizen())
					{
						iValue += 100;
					}
					else
					{
						YieldTypes eWantedYield = (YieldTypes)kIdealProfession.getYieldProduced();
						if (!kIdealProfession.isWorkPlot())
						{
							eWantedYield = (YieldTypes)kIdealProfession.getYieldConsumed();
						}
						
						if (eWantedYield == NO_YIELD)
						{
							iValue += 100;
						}
						else
						{
						
							int iPlotValue = 0;
							for (int j = 0; j < NUM_CITY_PLOTS; ++j)
							{
								CvPlot* pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), j);
								if (pLoopPlot != NULL)
								{
									if (pLoopPlot->isValidYieldChanges(eLoopUnit) && !(eWantedYield == YIELD_FOOD && pLoopPlot->isHills()))
									{
										int iBaseYield = pLoopPlot->calculateNatureYield(eWantedYield, getTeam(), true);
										if (iBaseYield > 0)
										{
											iPlotValue += 2;
										}
										
										if (pLoopPlot->getFeatureType() != NO_FEATURE)
										{
											int iChange = GC.getFeatureInfo(pLoopPlot->getFeatureType()).getYieldChange(eWantedYield);
											if (iChange > 0 && iBaseYield == 0)
											{
												iValue ++;
											}
											if (iChange < 0 && iBaseYield > 0)
											{
												iValue --;
											}
										}

										if (pLoopPlot->getBonusType() != NO_BONUS)
										{
											if (GC.getBonusInfo(pLoopPlot->getBonusType()).getYieldChange(eWantedYield) > 0)
											{
												iPlotValue += 4;
											}
										}
									}
								}
							}
							
							iValue = 25 + 125 * iPlotValue / NUM_CITY_PLOTS;
						}
					}
				}
				
				iValue *= GC.getCivilizationInfo(kOwner.getCivilizationType()).getTeachUnitClassWeight(i);
				values[i] = iValue;
			}
		}
	}
	
	int iTotal = 0;
	int iCount = 0;
	for (int i = 0; i < GC.getNumUnitClassInfos(); ++i)
	{
		if (values[i] != 0)
		{
			iTotal += values[i];
			iCount ++;
		}
	}

	if (iCount == 0)
	{
		return NO_UNITCLASS;
	}
	
	int iLoop = 0;
	CvCity* pLoopCity;
	for (pLoopCity = kOwner.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kOwner.nextCity(&iLoop))
	{
		if (pLoopCity->getTeachUnitClass() != NO_UNITCLASS)
		{
			values[pLoopCity->getTeachUnitClass()] ++;
			values[pLoopCity->getTeachUnitClass()] /= 2;
		}
	}
	
	//This dampens the favortism towards the most abundant yields.
	int iAverage = iTotal / iCount;
	for (int i = 0; i < GC.getNumUnitClassInfos(); ++i)
	{
		int iValue = values[i];
		if (iValue > 0)
		{
			if (iValue > iAverage)
			{
				iValue = ((iValue - iAverage) / 4) + iAverage;
			}
			iValue = 1 + GC.getGameINLINE().getSorenRandNum(iValue, "Pick City Training");
			
			if (iValue > iBestValue)
			{
				iBestValue = iValue;
				eBestUnitClass = (UnitClassTypes)i;
			}
		}
	}
	
	return eBestUnitClass;
}

CvUnit* CvCity::ejectBestDefender(CvUnit* pCurrentBest, CvUnit* pAttacker)
{
	ProfessionTypes eProfession;
	CvUnit* pDefender = getBestDefender(&eProfession, pCurrentBest, pAttacker);

	if (pDefender != NULL)
	{
		if (pDefender != pCurrentBest)
		{
			if (!removePopulationUnit(pDefender, false, eProfession))
			{
				return pCurrentBest;
			}
		}
	}

	return pDefender;
}

CvUnit* CvCity::getBestDefender(ProfessionTypes* peProfession, CvUnit* pCurrentBest, const CvUnit* pAttacker) const
{
	//initialize profession
	if (peProfession != NULL)
	{
		*peProfession = NO_PROFESSION;
		if (pCurrentBest != NULL)
		{
			*peProfession = pCurrentBest->getProfession();
		}
	}

	if (pAttacker != NULL && !pAttacker->isEnemy(getTeam(), plot()))
	{
		return pCurrentBest;
	}

	if (pCurrentBest != NULL && !pCurrentBest->isUnarmed() && pCurrentBest->isCityDefender())
	{
		return pCurrentBest;
	}

	CvUnit* pBestUnit = pCurrentBest;
	ProfessionTypes eBestProfession = NO_PROFESSION;
	if(pBestUnit != NULL)
	{
		eBestProfession = pBestUnit->getProfession();
	}

	for (uint i = 0; i < m_aPopulationUnits.size(); ++i)
	{
		CvUnit* pLoopUnit = m_aPopulationUnits[i];
		ProfessionTypes eBestCitizenProfession = NO_PROFESSION;
		int iBestValue = 0;
		for (int iProfession = 0; iProfession < GC.getNumProfessionInfos(); ++iProfession)
		{
			ProfessionTypes eProfession = (ProfessionTypes) iProfession;
			if (GC.getProfessionInfo(eProfession).isCityDefender())
			{
				int iValue = GET_PLAYER(getOwnerINLINE()).AI_professionValue(eProfession, UNITAI_DEFENSIVE);
				if (iValue > iBestValue)
				{
					if (pLoopUnit->canHaveProfession(eProfession, false))
					{
						iBestValue = iValue;
						eBestCitizenProfession = eProfession;
					}
				}
			}
		}

		if (eBestCitizenProfession != NO_PROFESSION)
		{
			CvUnitTemporaryStrengthModifier modifier1(pBestUnit, eBestProfession);
			CvUnitTemporaryStrengthModifier modifier2(pLoopUnit, eBestCitizenProfession);
			if (pLoopUnit->isBetterDefenderThan(pBestUnit, pAttacker, false))
			{
				pBestUnit = pLoopUnit;
				eBestProfession = eBestCitizenProfession;
			}					
		}
	}

	if (peProfession != NULL)
	{
		*peProfession = eBestProfession;
	}

	return pBestUnit;
}

bool CvCity::canTradeAway(PlayerTypes eToPlayer) const
{
	if (getLiberationPlayer(false) == eToPlayer)
	{
		return true;
	}

	if (!GET_PLAYER(eToPlayer).canReceiveTradeCity(getOwnerINLINE()))
	{
		return false;
	}

	if (0 != GC.getGameINLINE().getMaxCityElimination())
	{
		return false;
	}

	if (isCapital())
	{
		return false;
	}

	if (isNative() && !GET_PLAYER(eToPlayer).isNative())
	{
		if (plot()->getCulture(eToPlayer) < plot()->getCulture(getOwnerINLINE()))
		{
			return false;
		}
	}

	return true;
}

bool CvCity::educateStudent(int iUnitId, UnitTypes eUnit)
{
	CvUnit* pUnit = getPopulationUnitById(iUnitId);
	if (pUnit == NULL)
	{
		return false;
	}

	CvPlayer& kPlayer = GET_PLAYER(getOwnerINLINE());
	int iPrice = getSpecialistTuition(eUnit);
	if (iPrice < 0)
	{
		return false;
	}

	pUnit->setYieldStored(0);
	// perform the conversion
	if (!removePopulationUnit(pUnit, false, (ProfessionTypes) GC.getCivilizationInfo(GET_PLAYER(getOwnerINLINE()).getCivilizationType()).getDefaultProfession()))
	{
		return false;
	}

	CvUnit* pLearnUnit = GET_PLAYER(getOwnerINLINE()).initUnit(eUnit, NO_PROFESSION, pUnit->getX_INLINE(), pUnit->getY_INLINE(), pUnit->AI_getUnitAIType());
	FAssert(pLearnUnit != NULL);
	pLearnUnit->convert(pUnit, true);

	// PatchMod: Further education START
	// clear the accumulated teacher points
//	for (int i = 0; i < GC.getNumUnitInfos(); ++i)
//	{
//		m_aiSpecialistWeights[i] = 0;
//	}
	// PatchMod: Further education END

	kPlayer.setEducationThresholdMultiplier((kPlayer.getEducationThresholdMultiplier() * (100 + GC.getDefineINT("EDUCATION_THRESHOLD_INCREASE"))) / 100);
	kPlayer.changeGold(-iPrice);

	BuildingTypes eSchoolBuilding = getYieldBuilding(YIELD_EDUCATION);
	FAssert(eSchoolBuilding != NO_BUILDING);
	if(eSchoolBuilding != NO_BUILDING)
	{
		CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_STUDENT_GRADUATED", GC.getUnitInfo(eUnit).getTextKeyWide(), getNameKey(), GC.getBuildingInfo(eSchoolBuilding).getTextKeyWide());
		gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_CULTUREEXPANDS", MESSAGE_TYPE_MINOR_EVENT, GC.getYieldInfo(YIELD_EDUCATION).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_WHITE"), getX_INLINE(), getY_INLINE(), true, true);
	}

	return true;
}

int CvCity::getSpecialistTuition(UnitTypes eUnit) const
{
	if (m_aiSpecialistWeights[eUnit] <= 0)
	{
		return -1;
	}

	int* pMaxElement = std::max_element(m_aiSpecialistWeights, m_aiSpecialistWeights + GC.getNumUnitInfos());
	int iBestWeight = *pMaxElement;
	if (iBestWeight <= 0)
	{
		return -1;
	}

	int iPrice = GC.getDefineINT("EDUCATION_BASE_TUITION");
	iPrice *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getGrowthPercent();
	iPrice /= 100;
	
	if (!isHuman())
	{
		iPrice *= GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAITrainPercent();
		iPrice /= 100;
	}

	iPrice *= iBestWeight - m_aiSpecialistWeights[eUnit];
	iPrice /= iBestWeight;

	return iPrice;
}

bool CvCity::isExport(YieldTypes eYield) const
{
	return (m_setExports.find(eYield) != m_setExports.end());
}

void CvCity::addExport(YieldTypes eYield, bool bUpdateRoutes)
{
	std::pair<std::set<YieldTypes>::iterator, bool> zPair;

	zPair = m_setExports.insert(eYield);
	if (!zPair.second)
	{
		return;
	}

	if (bUpdateRoutes)
	{
		for (int iRoutePlayer = 0; iRoutePlayer < MAX_PLAYERS; ++iRoutePlayer)
		{
			CvPlayer& kRoutePlayer = GET_PLAYER((PlayerTypes) iRoutePlayer);
			if (kRoutePlayer.isAlive() && kRoutePlayer.canLoadYield(getOwnerINLINE()))
			{
				for (int iCityPlayer = 0; iCityPlayer < MAX_PLAYERS; ++iCityPlayer)
				{
					CvPlayer& kCityPlayer = GET_PLAYER((PlayerTypes) iCityPlayer);
					if (kCityPlayer.isAlive() && kRoutePlayer.canUnloadYield((PlayerTypes) iCityPlayer))
					{
						int iLoop;
						for (CvCity* pLoopCity = kCityPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kCityPlayer.nextCity(&iLoop))
						{
							if (pLoopCity->isImport(eYield))
							{
								kRoutePlayer.addTradeRoute(getIDInfo(), pLoopCity->getIDInfo(), eYield);
							}
						}
					}
				}

				if (kRoutePlayer.isYieldEuropeTradable(eYield))
				{
					kRoutePlayer.addTradeRoute(getIDInfo(), IDInfo((PlayerTypes) iRoutePlayer, CvTradeRoute::EUROPE_CITY_ID), eYield);
				}
			}
		}
	}

	if (getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
	{
		gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
		gDLL->getInterfaceIFace()->setDirty(Domestic_Advisor_DIRTY_BIT, true);
	}
}

void CvCity::removeExport(YieldTypes eYield, bool bUpdateRoutes)
{
	int iNumErased = m_setExports.erase(eYield);
	if (iNumErased == 0)
	{
		return;
	}

	if (bUpdateRoutes)
	{
		for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
		{
			CvPlayer& kPlayer = GET_PLAYER((PlayerTypes) iPlayer);
			if (kPlayer.isAlive())
			{
				std::vector<int> aiRouteIds;
				std::vector<CvTradeRoute*> aiRoutes;
				kPlayer.getTradeRoutes(aiRoutes);
				for (uint i = 0; i < aiRoutes.size(); ++i)
				{
					CvTradeRoute* pRoute = aiRoutes[i];
					if (pRoute != NULL && pRoute->getSourceCity() == getIDInfo() && pRoute->getYield() == eYield)
					{
						aiRouteIds.push_back(pRoute->getID());
					}
				}

				for (uint i = 0; i < aiRouteIds.size(); ++i)
				{
					kPlayer.removeTradeRoute(aiRouteIds[i]);
				}
			}
		}
	}

	if (getTeam() == GC.getGameINLINE().getActiveTeam())
	{
		gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
		gDLL->getInterfaceIFace()->setDirty(Domestic_Advisor_DIRTY_BIT, true);
	}
}

void CvCity::updateExport(YieldTypes eYield)
{
	int iExportCount = 0;
	for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
	{
		CvPlayer& kPlayer = GET_PLAYER((PlayerTypes) iPlayer);
		if (kPlayer.isAlive())
		{
			std::vector<CvTradeRoute*> aiRoutes;
			kPlayer.getTradeRoutes(aiRoutes);
			for (uint i = 0; i < aiRoutes.size(); ++i)
			{
				CvTradeRoute* pRoute = aiRoutes[i];
				if (pRoute != NULL && pRoute->getSourceCity() == getIDInfo() && pRoute->getYield() == eYield)
				{
					++iExportCount;
				}
			}
		}
	}

	if (iExportCount > 0)
	{
		addExport(eYield, false);
	}
	else
	{
		removeExport(eYield, false);
	}
}

void CvCity::updateExports()
{
	for (int i = 0; i < NUM_YIELD_TYPES; ++i)
	{
		updateExport((YieldTypes) i);
	}
}

bool CvCity::isImport(YieldTypes eYield) const
{
	return (m_setImports.find(eYield) != m_setImports.end());
}

void CvCity::addImport(YieldTypes eYield, bool bUpdateRoutes)
{
	std::pair<std::set<YieldTypes>::iterator, bool> zPair;
	
	zPair = m_setImports.insert(eYield);
	if (!zPair.second)
	{
		return;
	}

	if (bUpdateRoutes)
	{
		for (int iRoutePlayer = 0; iRoutePlayer < MAX_PLAYERS; ++iRoutePlayer)
		{
			CvPlayer& kRoutePlayer = GET_PLAYER((PlayerTypes) iRoutePlayer);
			if (kRoutePlayer.isAlive() && kRoutePlayer.canUnloadYield(getOwnerINLINE()))
			{
				for (int iCityPlayer = 0; iCityPlayer < MAX_PLAYERS; ++iCityPlayer)
				{
					CvPlayer& kCityPlayer = GET_PLAYER((PlayerTypes) iCityPlayer);
					if (kCityPlayer.isAlive() && kRoutePlayer.canLoadYield((PlayerTypes) iCityPlayer))
					{
						int iLoop;
						for (CvCity* pLoopCity = kCityPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kCityPlayer.nextCity(&iLoop))
						{
							if (pLoopCity->isExport(eYield))
							{
								kRoutePlayer.addTradeRoute(pLoopCity->getIDInfo(), getIDInfo(), eYield);
							}
						}
					}
				}
			}
		}
	}

	if (getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
	{
		gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
		gDLL->getInterfaceIFace()->setDirty(Domestic_Advisor_DIRTY_BIT, true);
	}
}

void CvCity::removeImport(YieldTypes eYield, bool bUpdateRoutes)
{
	int iNumErased = m_setImports.erase(eYield);
	if (iNumErased == 0)
	{
		return;
	}

	if (bUpdateRoutes)
	{
		for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
		{
			CvPlayer& kPlayer = GET_PLAYER((PlayerTypes) iPlayer);
			if (kPlayer.isAlive())
			{
				std::vector<int> aiRouteIds;
				std::vector<CvTradeRoute*> aiRoutes;
				kPlayer.getTradeRoutes(aiRoutes);
				for (uint i = 0; i < aiRoutes.size(); ++i)
				{
					CvTradeRoute* pRoute = aiRoutes[i];
					if (pRoute != NULL && pRoute->getDestinationCity() == getIDInfo() && pRoute->getYield() == eYield)
					{
						aiRouteIds.push_back(pRoute->getID());
					}
				}

				for (uint i = 0; i < aiRouteIds.size(); ++i)
				{
					kPlayer.removeTradeRoute(aiRouteIds[i]);
				}
			}
		}
	}

	if (getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
	{
		gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
		gDLL->getInterfaceIFace()->setDirty(Domestic_Advisor_DIRTY_BIT, true);
	}
}

void CvCity::updateImport(YieldTypes eYield)
{
	int iImportCount = 0;
	for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
	{
		CvPlayer& kPlayer = GET_PLAYER((PlayerTypes) iPlayer);
		if (kPlayer.isAlive())
		{
			std::vector<CvTradeRoute*> aiRoutes;
			kPlayer.getTradeRoutes(aiRoutes);
			for (uint i = 0; i < aiRoutes.size(); ++i)
			{
				CvTradeRoute* pRoute = aiRoutes[i];
				if (pRoute != NULL && pRoute->getDestinationCity() == getIDInfo() && pRoute->getYield() == eYield)
				{
					++iImportCount;
				}
			}
		}
	}

	if (iImportCount > 0)
	{
		addImport(eYield, false);
	}
	else
	{
		removeImport(eYield, false);
	}
}

void CvCity::updateImports()
{
	for (int i = 0; i < NUM_YIELD_TYPES; ++i)
	{
		updateImport((YieldTypes) i);
	}
}

void CvCity::removeTradeRoutes()
{
	for (int i = 0; i < NUM_YIELD_TYPES; ++i)
	{
		YieldTypes eYield = (YieldTypes) i;

		if (isExport(eYield))
		{
			removeExport(eYield);
		}

		if (isImport(eYield))
		{
			removeImport(eYield);
		}
	}
}

void CvCity::setMaintainLevel(YieldTypes eYield, int iMaintainLevel)
{
	if (getMaintainLevel(eYield) != iMaintainLevel)
	{
		m_mapMaintain[eYield] = iMaintainLevel;

		if (getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
		{
			gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
		}
	}
}

int CvCity::getMaintainLevel(YieldTypes eYield) const
{
	std::map<YieldTypes, int>::const_iterator it = m_mapMaintain.find(eYield);
	if (it != m_mapMaintain.end())
	{
		return it->second;
	}

	return 0;
}

// PatchMod: Missionary player START
PlayerTypes CvCity::getMissionaryPlayer() const
{
	return m_eMissionaryPlayer;
}

void CvCity::setMissionaryPlayer(PlayerTypes ePlayer)
{
	m_eMissionaryPlayer = ePlayer;
}
// PatchMod: Missionary player END
