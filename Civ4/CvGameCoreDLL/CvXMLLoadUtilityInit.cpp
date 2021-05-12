//
// init/cleanup XML functions
//
#include "CvGameCoreDLL.h"
#include "CvXMLLoadUtility.h"
#include "CvGlobals.h"
#include "CvArtFileMgr.h"
#include "CvGameTextMgr.h"
#include "CvInfoWater.h"
#include "FProfiler.h"

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   InitStringList(CvString **ppszList, int iListLen, CvString szString = "")
//
//  PURPOSE :   allocate and initialize a CvString pointer list of iListLen size to "", or
//				to szString if it is supplied
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::InitStringList(CvString **ppszList, int iListLen, CvString szString)
{
	PROFILE_FUNC();
	int i;	// loop counter
	CvString* pszList;

	FAssertMsg(*ppszList == NULL,"memory leak?");
	FAssertMsg((0 < iListLen),"list size to allocate is less than 1");
	// allocate memory for the boolean pointer based on the list length parameter
	*ppszList = new CvString[iListLen];
	// set the local pointer to the memory we just allocated
	pszList = *ppszList;

	// loop through all the booleans
	for (i=0;i<iListLen;i++)
	{
		// set the current boolean to false
		pszList[i] = szString;
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   void Init2DFloatList(float*** pppfList, int iSizeX, int iSizeY)
//
//  PURPOSE :   allocate and initialize a 2d array of float data
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::Init2DFloatList(float*** pppfList, int iSizeX, int iSizeY)
{
	// SPEEDUP
	PROFILE_FUNC();
	int i,j;	// loop counters
	float** ppfList;

	FAssertMsg(*pppfList == NULL,"memory leak?");
	FAssertMsg(((0 < iSizeX) && (0 < iSizeY)), "list size to allocate is less than 1");
	// allocate the memory for the array of pointers to arrays of floats
	*pppfList = new float *[iSizeX];
	// set the local pointer to the newly allocated memory
	ppfList = *pppfList;

	// loop through each of the pointers
	for (i=0;i<iSizeX;i++)
	{
		// allocate a list of floats for the current pointer
		ppfList[i] = new float[iSizeY];
		// loop through all of the current pointer's floats
		for (j=0;j<iSizeY;j++)
		{
			// set the current float to zero
			ppfList[i][j] = 0.0f;
		}
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   void Init2DIntList(int*** pppiList, int iSizeX, int iSizeY)
//
//  PURPOSE :   allocate and initialize a 2d array of int data
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::Init2DIntList(int*** pppiList, int iSizeX, int iSizeY)
{
	// SPEEDUP
	PROFILE_FUNC();
	int i,j;	// loop counters
	int** ppiList;

	FAssertMsg(*pppiList == NULL,"memory leak?");
	FAssertMsg(((0 < iSizeX) && (0 < iSizeY)), "list size to allocate is less than 1");
	// allocate the memory for the array of pointers to arrays of ints
	*pppiList = new int *[iSizeX];
	// set the local pointer to the newly allocated memory
	ppiList = *pppiList;

	// loop through each of the pointers
	for (i=0;i<iSizeX;i++)
	{
		// allocate a list of ints for the current pointer
		ppiList[i] = new int[iSizeY];
		// loop through all of the current pointer's ints
		for (j=0;j<iSizeY;j++)
		{
			// set the current int to zero
			ppiList[i][j] = 0;
		}
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   void InitPointerFloatList(float*** pppfList, int iSizeX)
//
//  PURPOSE :   allocate and initialize a 2d array of float data
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::InitPointerFloatList(float*** pppfList, int iSizeX)
{
	// SPEEDUP
	PROFILE_FUNC();
	int i;	// loop counters
	float** ppfList;

	FAssertMsg(*pppfList == NULL,"memory leak?");
	FAssertMsg((0 < iSizeX), "list size to allocate is less than 1");
	// allocate the memory for the array of pointers to arrays of floats
	*pppfList = new float *[iSizeX];
	// set the local pointer to the newly allocated memory
	ppfList = *pppfList;

	// loop through each of the pointers
	for (i=0;i<iSizeX;i++)
	{
		// Null each pointer
		ppfList[i] = NULL;
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   void InitPointerIntList(int*** pppiList, int iSizeX)
//
//  PURPOSE :   allocate and initialize a 2d array of int data
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::InitPointerIntList(int*** pppiList, int iSizeX)
{
	// SPEEDUP
	PROFILE_FUNC();
	int i;	// loop counters
	int** ppiList;

	FAssertMsg(*pppiList == NULL,"memory leak?");
	FAssertMsg((0 < iSizeX), "list size to allocate is less than 1");
	// allocate the memory for the array of pointers to arrays of ints
	*pppiList = new int *[iSizeX];
	// set the local pointer to the newly allocated memory
	ppiList = *pppiList;

	// loop through each of the pointers
	for (i=0;i<iSizeX;i++)
	{
		// Null each pointer
		ppiList[i] = NULL;
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   void Init2DDirectionTypesList(DirectionTypes*** pppiList, int iSizeX, int iSizeY)
//
//  PURPOSE :   allocate and initialize a 2d array of DirectionTypes data
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::Init2DDirectionTypesList(DirectionTypes*** pppiList, int iSizeX, int iSizeY)
{
	// SPEEDUP
	PROFILE_FUNC();
	int i,j;	// loop counters
	DirectionTypes** ppiList;

	FAssertMsg(*pppiList == NULL,"memory leak?");
	FAssertMsg(((0 < iSizeX) && (0 < iSizeY)), "list size to allocate is less than 1");
	// allocate the memory for the array of pointers to arrays of DirectionTypes
	*pppiList = new DirectionTypes *[iSizeX];
	// set the local pointer to the newly allocated memory
	ppiList = *pppiList;

	// loop through each of the pointers
	for (i=0;i<iSizeX;i++)
	{
		// allocate a list of DirectionTypes for the current pointer
		ppiList[i] = new DirectionTypes[iSizeY];
		// loop through all of the current pointer's DirectionTypes
		for (j=0;j<iSizeY;j++)
		{
			// set the current DirectionTypes to NO_DIRECTION
			ppiList[i][j] = NO_DIRECTION;
		}
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   InitImprovementBonusList(CvImprovementBonusInfo** ppImprovementBonus, int iListLen)
//
//  PURPOSE :   allocate a improvement bonus struct list of iListLen size and initialize it's members
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::InitImprovementBonusList(CvImprovementBonusInfo** ppImprovementBonus, int iListLen)
{
	// SPEEDUP
	PROFILE_FUNC();
	int i;	// loop counter
	CvImprovementBonusInfo* paImprovementBonus;

	FAssertMsg(*ppImprovementBonus == NULL,"memory leak?");
	FAssertMsg((0 < iListLen),"list size to allocate is less than 1");
	// allocate memory for the bonus type pointer based on the list length parameter
	*ppImprovementBonus = new CvImprovementBonusInfo[iListLen];
	// set the local pointer to the memory we just allocated
	paImprovementBonus = *ppImprovementBonus;

	// loop through all the bonus structs
	for (i=0;i<iListLen;i++)
	{
		paImprovementBonus[i].m_bBonusMakesValid = false;
		paImprovementBonus[i].m_bBonusTrade = false;

#if 1
		FAssertMsg(paImprovementBonus[i].m_piYieldChange==NULL, "mem leak");
		paImprovementBonus[i].m_piYieldChange = new int[NUM_YIELD_TYPES];
		for (int j = 0; j < NUM_YIELD_TYPES; j++)
		{
			paImprovementBonus[i].m_piYieldChange[j] = 0;
		}
#endif

		paImprovementBonus[i].m_iDiscoverRand = 0;
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   InitBuildingDefaults(int **ppiDefaults)
//
//  PURPOSE :   allocate and initialize the civilization's default buildings
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::InitBuildingDefaults(int **ppiDefaults)
{
	// SPEEDUP
	PROFILE_FUNC();

	int i;
	int* piDefaults;

	FAssertMsg(*ppiDefaults == NULL,"memory leak?");
	// allocate memory based on the number of building classes
	*ppiDefaults = new int[GC.getNumBuildingClassInfos()];
	// set the local pointer to the new memory
	piDefaults = *ppiDefaults;

	// loop through all the pointers and set their default values
	for (i=0;i<GC.getNumBuildingClassInfos();i++)
	{
		piDefaults[i] = GC.getBuildingClassInfo((BuildingClassTypes) i).getDefaultBuildingIndex();
	}

}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   InitUnitDefaults(int **ppiDefaults)
//
//  PURPOSE :   allocate and initialize the civilization's default Units
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::InitUnitDefaults(int **ppiDefaults)
{
	// SPEEDUP
	PROFILE_FUNC();

	int i;
	int* piDefaults;

	FAssertMsg(*ppiDefaults == NULL,"memory leak?");
	// allocate memory based on the number of uniting classes
	*ppiDefaults = new int[GC.getNumUnitClassInfos()];
	// set the local pointer to the new memory
	piDefaults = *ppiDefaults;

	// loop through all the pointers and set their default values
	for (i=0;i<GC.getNumUnitClassInfos();i++)
	{
		piDefaults[i] = GC.getUnitClassInfo((UnitClassTypes) i).getDefaultUnitIndex();
	}
}


//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CleanUpGlobals()
//
//  PURPOSE :   free the variables that are in globals.cpp/h
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::CleanUpGlobalVariables()
{
	SAFE_DELETE_ARRAY(GC.getBuildingClassInfo());
	SAFE_DELETE_ARRAY(GC.getBuildingInfo());
	SAFE_DELETE_ARRAY(GC.getSpecialBuildingInfo());

	SAFE_DELETE_ARRAY(GC.getLeaderHeadInfo());
	SAFE_DELETE_ARRAY(GC.getTraitInfo());
	SAFE_DELETE_ARRAY(GC.getCivilizationInfo());

	SAFE_DELETE_ARRAY(GC.getHints());
	SAFE_DELETE_ARRAY(GC.getGoodyInfo());
	SAFE_DELETE_ARRAY(GC.getHandicapInfo());
	SAFE_DELETE_ARRAY(GC.getGameSpeedInfo());
	SAFE_DELETE_ARRAY(GC.getTurnTimerInfo());
	SAFE_DELETE_ARRAY(GC.getVictoryInfo());
	SAFE_DELETE_ARRAY(GC.getHurryInfo());
	SAFE_DELETE_ARRAY(GC.getWorldInfo());
	SAFE_DELETE_ARRAY(GC.getSeaLevelInfo());
	SAFE_DELETE_ARRAY(GC.getClimateInfo());
	SAFE_DELETE_ARRAY(GC.getProcessInfo());
	SAFE_DELETE_ARRAY(GC.getVoteInfo());
	SAFE_DELETE_ARRAY(GC.getProjectInfo());
	SAFE_DELETE_ARRAY(GC.getReligionInfo());
	SAFE_DELETE_ARRAY(GC.getCommerceInfo());
	SAFE_DELETE_ARRAY(GC.getEmphasizeInfo());
	SAFE_DELETE_ARRAY(GC.getUpkeepInfo());
	SAFE_DELETE_ARRAY(GC.getCultureLevelInfo());

	SAFE_DELETE_ARRAY(GC.getColorInfo());
	SAFE_DELETE_ARRAY(GC.getPlayerColorInfo());
	SAFE_DELETE_ARRAY(GC.getInterfaceModeInfo());
	SAFE_DELETE_ARRAY(GC.getCameraInfo());
	SAFE_DELETE_ARRAY(GC.getAdvisorInfo());
	//SAFE_DELETE_ARRAY(GC.getThroneRoomCamera());
	//SAFE_DELETE_ARRAY(GC.getThroneRoomInfo());
	//SAFE_DELETE_ARRAY(GC.getThroneRoomStyleInfo());
	SAFE_DELETE_ARRAY(GC.getSlideShowInfo());
	SAFE_DELETE_ARRAY(GC.getSlideShowRandomInfo());
	SAFE_DELETE_ARRAY(GC.getWorldPickerInfo());

	SAFE_DELETE_ARRAY(GC.getRouteInfo());
	SAFE_DELETE_ARRAY(GC.getRouteModelInfo());
	SAFE_DELETE_ARRAY(GC.getRiverInfo());
	SAFE_DELETE_ARRAY(GC.getRiverModelInfo());

	SAFE_DELETE_ARRAY(GC.getWaterPlaneInfo());

	SAFE_DELETE_ARRAY(GC.getCivicInfo());

	SAFE_DELETE_ARRAY(GC.getEraInfo());
	SAFE_DELETE_ARRAY(GC.getEffectInfo());
	SAFE_DELETE_ARRAY(GC.getAttachableInfo());

	SAFE_DELETE_ARRAY(GC.getTechInfo());
	SAFE_DELETE_ARRAY(GC.getDiplomacyInfo());

	SAFE_DELETE_ARRAY(GC.getImprovementInfo());
	SAFE_DELETE_ARRAY(GC.getBuildInfo());
	SAFE_DELETE_ARRAY(GC.getUnitClassInfo());
	SAFE_DELETE_ARRAY(GC.getUnitInfo());
	SAFE_DELETE_ARRAY(GC.getSpecialUnitInfo());
	SAFE_DELETE_ARRAY(GC.getSpecialistInfo());
	SAFE_DELETE_ARRAY(GC.getActionInfo());
	SAFE_DELETE_ARRAY(GC.getMissionInfo());
	SAFE_DELETE_ARRAY(GC.getControlInfo());
	SAFE_DELETE_ARRAY(GC.getCommandInfo());
	SAFE_DELETE_ARRAY(GC.getAutomateInfo());
	SAFE_DELETE_ARRAY(GC.getPromotionInfo());

	SAFE_DELETE_ARRAY(GC.getConceptInfo());
	SAFE_DELETE_ARRAY(GC.getCityTabInfo());
	SAFE_DELETE_ARRAY(GC.getCalendarInfo());
	SAFE_DELETE_ARRAY(GC.getSeasonInfo());
	SAFE_DELETE_ARRAY(GC.getMonthInfo());
	SAFE_DELETE_ARRAY(GC.getDenialInfo());
	SAFE_DELETE_ARRAY(GC.getInvisibleInfo());
	SAFE_DELETE_ARRAY(GC.getUnitCombatInfo());
	SAFE_DELETE_ARRAY(GC.getDomainInfo());
	SAFE_DELETE_ARRAY(GC.getUnitAIInfo());
	SAFE_DELETE_ARRAY(GC.getAttitudeInfo());
	SAFE_DELETE_ARRAY(GC.getMemoryInfo());
	SAFE_DELETE_ARRAY(GC.getGameOptionInfo());
	SAFE_DELETE_ARRAY(GC.getMPOptionInfo());
	SAFE_DELETE_ARRAY(GC.getForceControlInfo());
	SAFE_DELETE_ARRAY(GC.getPlayerOptionInfo());
	SAFE_DELETE_ARRAY(GC.getGraphicOptionInfo());

	SAFE_DELETE_ARRAY(GC.getYieldInfo());
	SAFE_DELETE_ARRAY(GC.getTerrainInfo());
	SAFE_DELETE_ARRAY(GC.getFeatureInfo());
	SAFE_DELETE_ARRAY(GC.getBonusClassInfo());
	SAFE_DELETE_ARRAY(GC.getBonusInfo());
	SAFE_DELETE_ARRAY(GC.getLandscapeInfo());

	SAFE_DELETE_ARRAY(GC.getUnitFormationInfo());
	SAFE_DELETE_ARRAY(GC.getCivicOptionInfo());
	SAFE_DELETE_ARRAY(GC.getCursorInfo());

	SAFE_DELETE_ARRAY(GC.getEntityEventTypes());
	SAFE_DELETE_ARRAY(GC.getAnimationOperatorTypes());
	SAFE_DELETE_ARRAY(GC.getFunctionTypes());
	SAFE_DELETE_ARRAY(GC.getFlavorTypes());
	SAFE_DELETE_ARRAY(GC.getArtStyleTypes());
	SAFE_DELETE_ARRAY(GC.getCitySizeTypes());
	SAFE_DELETE_ARRAY(GC.getContactTypes());
	SAFE_DELETE_ARRAY(GC.getDiplomacyPowerTypes());
	SAFE_DELETE_ARRAY(GC.getAutomateTypes());
	SAFE_DELETE_ARRAY(GC.getDirectionTypes());
	SAFE_DELETE_ARRAY(GC.getFootstepAudioTypes());
	SAFE_DELETE_ARRAY(GC.getFootstepAudioTags());
	SAFE_DELETE_ARRAY(GC.getQuestInfo());
	SAFE_DELETE_ARRAY(GC.getHints());
	SAFE_DELETE_ARRAY(GC.getTutorialInfo());

	SAFE_DELETE_ARRAY(GC.getEntityEventInfo());
	SAFE_DELETE_ARRAY(GC.getAnimationCategoryInfo());
	SAFE_DELETE_ARRAY(GC.getAnimationPathInfo());

	GC.clearTypesMap();
}
