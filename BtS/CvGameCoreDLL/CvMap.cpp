//
//	FILE:	 CvMap.cpp
//	AUTHOR:  Soren Johnson
//	PURPOSE: Game map class
//-----------------------------------------------------------------------------
//	Copyright (c) 2004 Firaxis Games, Inc. All rights reserved.
//-----------------------------------------------------------------------------
//


#include "CvGameCoreDLL.h"
#include "CvMap.h"
#include "CvCity.h"
#include "CvPlotGroup.h"
#include "CvGlobals.h"
#include "CvGameAI.h"
#include "CvPlayerAI.h"
#include "CvRandom.h"
#include "CvGameCoreUtils.h"
#include "CvFractal.h"
#include "CvPlot.h"
#include "CvGameCoreUtils.h"
#include "CvMap.h"
#include "CvMapGenerator.h"
#include "FAStarNode.h"
#include "CvInitCore.h"
#include "CvInfos.h"
#include "CvExtraSaveData.h"
#include "FProfiler.h"

#include "CvDLLEngineIFaceBase.h"
#include "CvDLLIniParserIFaceBase.h"
#include "CvDLLFAStarIFaceBase.h"
#include "CvDLLFAStarIFaceBase.h"
#include "CvDLLPythonIFaceBase.h"

// our own version number on our extra data
// if we want to reorder what we save, change this number
// adding to the list should be fine
#define MAPEXTRADATA_CURRENTVERSION		1
enum MapExtraDataIndicies
{
	MAPEXTRADATA_VERSION = 0,
	MAPEXTRADATA_MAPVISIONTYPE,
};

enum MapVisionTypes
{
	MAPVISION_STANDARD = 1,
	MAPVISION_FASTER_VISION = 2,
};

#define MAPEXTRADATA_MAPVISIONTYPE_CURRENT		MAPVISION_STANDARD

#ifdef FASTER_VISIBILITY_NON_SAVE_COMPATABLE
#undef MAPEXTRADATA_MAPVISIONTYPE_CURRENT
#define MAPEXTRADATA_MAPVISIONTYPE_CURRENT		MAPVISION_FASTER_VISION
#endif

// Public Functions...

CvMap::CvMap()
{
	CvMapInitData defaultMapData;

	m_paiNumBonus = NULL;
	m_paiNumBonusOnLand = NULL;

	m_pMapPlots = NULL;
	
	//m_extraSaveData = new CvExtraSaveData;

	constructCache();
	
	m_bConstructorCall = true;	// hack, because reset does not have a constructer call parameter, and we cannot add it
	reset(&defaultMapData);
	m_bConstructorCall = false;	// hack
}


CvMap::~CvMap()
{
	uninit();

	destructCache();
}

// FUNCTION: init()
// Initializes the map.
// Parameters:
//	pInitInfo					- Optional init structure (used for WB load)
// Returns:
//	nothing.
void CvMap::init(CvMapInitData* pInitInfo/*=NULL*/)
{
	int iX, iY;

	PROFILE("CvMap::init");
	gDLL->logMemState( CvString::format("CvMap::init begin - world size=%s, climate=%s, sealevel=%s, num custom options=%6", 
		GC.getWorldInfo(GC.getInitCore().getWorldSize()).getDescription(), 
		GC.getClimateInfo(GC.getInitCore().getClimate()).getDescription(), 
		GC.getSeaLevelInfo(GC.getInitCore().getSeaLevel()).getDescription(),
		GC.getInitCore().getNumCustomMapOptions()).c_str() );

	gDLL->getPythonIFace()->callFunction(gDLL->getPythonIFace()->getMapScriptModule(), "beforeInit");

	//--------------------------------
	// Init saved data
	reset(pInitInfo);

	//--------------------------------
	// Init containers
	m_areas.init();

	//--------------------------------
	// Init non-saved data
	setup();

	//--------------------------------
	// Init other game data
	gDLL->logMemState("CvMap before init plots");
	m_pMapPlots = new CvPlot[numPlotsINLINE()];
	for (iX = 0; iX < getGridWidthINLINE(); iX++)
	{
		gDLL->callUpdater();
		for (iY = 0; iY < getGridHeightINLINE(); iY++)
		{
			plotSorenINLINE(iX, iY)->init(iX, iY);
		}
	}
	calculateAreas();
	gDLL->logMemState("CvMap after init plots");

	initCache();
}


void CvMap::uninit()
{
	uninitCache();

	SAFE_DELETE_ARRAY(m_paiNumBonus);
	SAFE_DELETE_ARRAY(m_paiNumBonusOnLand);

	SAFE_DELETE_ARRAY(m_pMapPlots);

	m_areas.uninit();
}

// FUNCTION: reset()
// Initializes data members that are serialized.
void CvMap::reset(CvMapInitData* pInitInfo)
{
	int iI;

	//--------------------------------
	// Uninit class
	uninit();

	//
	// set grid size
	// initially set in terrain cell units
	//
	m_iGridWidth = (GC.getInitCore().getWorldSize() != NO_WORLDSIZE) ? GC.getWorldInfo(GC.getInitCore().getWorldSize()).getGridWidth (): 0;	//todotw:tcells wide
	m_iGridHeight = (GC.getInitCore().getWorldSize() != NO_WORLDSIZE) ? GC.getWorldInfo(GC.getInitCore().getWorldSize()).getGridHeight (): 0;

	// allow grid size override
	if (pInitInfo)
	{
		m_iGridWidth	= pInitInfo->m_iGridW;
		m_iGridHeight	= pInitInfo->m_iGridH;
	}
	else
	{
		// check map script for grid size override
		gDLL->getPythonIFace()->pythonGetGridSize(GC.getInitCore().getWorldSize(), &m_iGridWidth, &m_iGridHeight);

		// convert to plot dimensions
		if (GC.getLandscapeInfo())
		{
			m_iGridWidth *= GC.getLandscapeInfo(GC.getActiveLandscapeID()).getPlotsPerCellX();
			m_iGridHeight *= GC.getLandscapeInfo(GC.getActiveLandscapeID()).getPlotsPerCellY();
		}
	}

	m_iLandPlots = 0;
	m_iOwnedPlots = 0;

	if (pInitInfo)
	{
		m_iTopLatitude = pInitInfo->m_iTopLatitude;
		m_iBottomLatitude = pInitInfo->m_iBottomLatitude;
	}
	else
	{
		// Check map script for latitude override (map script beats ini file)
		gDLL->getPythonIFace()->pythonGetLatitudes(&m_iTopLatitude, &m_iBottomLatitude);
	}

	m_iNextRiverID = 0;

	//
	// set wrapping
	//
	m_bWrapX = true;
	m_bWrapY = false;
	if (pInitInfo)
	{
		m_bWrapX = pInitInfo->m_bWrapX;
		m_bWrapY = pInitInfo->m_bWrapY;
	}
	else
	{
		// Check map script for wrap override (map script beats ini file)
		gDLL->getPythonIFace()->pythonGetWrapXY(&m_bWrapX, &m_bWrapY);		
	}

	if (GC.getNumBonusInfos())
	{
		FAssertMsg((0 < GC.getNumBonusInfos()), "GC.getNumBonusInfos() is not greater than zero but an array is being allocated in CvMap::reset");
		FAssertMsg(m_paiNumBonus==NULL, "mem leak m_paiNumBonus");
		m_paiNumBonus = new int[GC.getNumBonusInfos()];
		FAssertMsg(m_paiNumBonusOnLand==NULL, "mem leak m_paiNumBonusOnLand");
		m_paiNumBonusOnLand = new int[GC.getNumBonusInfos()];
		for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
		{
			m_paiNumBonus[iI] = 0;
			m_paiNumBonusOnLand[iI] = 0;
		}
	}

	m_areas.removeAll();

	resetCache(m_bConstructorCall);
}


// FUNCTION: setup()
// Initializes all data that is not serialized but needs to be initialized after loading.
void CvMap::setup()
{
	PROFILE("CvMap::setup");

	gDLL->getFAStarIFace()->Initialize(&GC.getPathFinder(), getGridWidthINLINE(), getGridHeightINLINE(), isWrapXINLINE(), isWrapYINLINE(), pathDestValid, pathHeuristic, pathCost, pathValid, pathAdd, NULL, NULL);
	gDLL->getFAStarIFace()->Initialize(&GC.getInterfacePathFinder(), getGridWidthINLINE(), getGridHeightINLINE(), isWrapXINLINE(), isWrapYINLINE(), pathDestValid, pathHeuristic, pathCost, pathValid, pathAdd, NULL, NULL);
	gDLL->getFAStarIFace()->Initialize(&GC.getStepFinder(), getGridWidthINLINE(), getGridHeightINLINE(), isWrapXINLINE(), isWrapYINLINE(), stepDestValid, stepHeuristic, stepCost, stepValid, stepAdd, NULL, NULL);
	gDLL->getFAStarIFace()->Initialize(&GC.getRouteFinder(), getGridWidthINLINE(), getGridHeightINLINE(), isWrapXINLINE(), isWrapYINLINE(), NULL, NULL, NULL, routeValid, NULL, NULL, NULL);
	gDLL->getFAStarIFace()->Initialize(&GC.getBorderFinder(), getGridWidthINLINE(), getGridHeightINLINE(), isWrapXINLINE(), isWrapYINLINE(), NULL, NULL, NULL, borderValid, NULL, NULL, NULL);
	gDLL->getFAStarIFace()->Initialize(&GC.getAreaFinder(), getGridWidthINLINE(), getGridHeightINLINE(), isWrapXINLINE(), isWrapYINLINE(), NULL, NULL, NULL, areaValid, NULL, joinArea, NULL);
	gDLL->getFAStarIFace()->Initialize(&GC.getPlotGroupFinder(), getGridWidthINLINE(), getGridHeightINLINE(), isWrapXINLINE(), isWrapYINLINE(), NULL, NULL, NULL, plotGroupValid, NULL, countPlotGroup, NULL);
	
	resetCache(false);
}


//////////////////////////////////////
// graphical only setup
//////////////////////////////////////
void CvMap::setupGraphical()
{
	if (!GC.IsGraphicsInitialized())
		return;

	if (m_pMapPlots != NULL)
	{
		int iI;
		for (iI = 0; iI < numPlotsINLINE(); iI++)
		{
			gDLL->callUpdater();	// allow windows msgs to update
			plotByIndexINLINE(iI)->setupGraphical();
		}
	}
}


void CvMap::erasePlots()
{
	int iI;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		plotByIndexINLINE(iI)->erase();
	}
}


void CvMap::setRevealedPlots(TeamTypes eTeam, bool bNewValue, bool bTerrainOnly)
{
	int iI;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		plotByIndexINLINE(iI)->setRevealed(eTeam, bNewValue, bTerrainOnly);
	}
}


void CvMap::setAllPlotTypes(PlotTypes ePlotType)
{
	//float startTime = (float) timeGetTime();

	for(int i=0;i<numPlotsINLINE();i++)
	{
		plotByIndexINLINE(i)->setPlotType(ePlotType, false, false);
	}

	recalculateAreas();

	//rebuild landscape
	gDLL->getEngineIFace()->RebuildAllPlots();

	//mark minimap as dirty
	gDLL->getEngineIFace()->SetDirty(MinimapTexture_DIRTY_BIT, true);
	gDLL->getEngineIFace()->SetDirty(GlobeTexture_DIRTY_BIT, true);
	
	//float endTime = (float) timeGetTime();
	//OutputDebugString(CvString::format("[Jason] setAllPlotTypes: %f\n", endTime - startTime).c_str());
}


// XXX generalize these funcs? (macro?)
void CvMap::doTurn()
{
	PROFILE("CvMap::doTurn()")

	int iI;

	// the save data is no longer valid, we are done reading or writing
	getExtraSaveDataClass().invalidate();

	doUpdateCacheOnTurn();

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		plotByIndexINLINE(iI)->doTurn();
	}
}


void CvMap::updateFlagSymbols()
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	int iI;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		pLoopPlot = plotByIndexINLINE(iI);

		if (pLoopPlot->isFlagDirty())
		{
			pLoopPlot->updateFlagSymbol();
			pLoopPlot->setFlagDirty(false);
		}
	}
}


void CvMap::updateFog()
{
	int iI;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		plotByIndexINLINE(iI)->updateFog();
	}
}


void CvMap::updateVisibility()
{
	int iI;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		plotByIndexINLINE(iI)->updateVisibility();
	}
}


void CvMap::updateSymbolVisibility()
{
	int iI;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		plotByIndexINLINE(iI)->updateSymbolVisibility();
	}
}


void CvMap::updateSymbols()
{
	int iI;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		plotByIndexINLINE(iI)->updateSymbols();
	}
}


void CvMap::updateMinimapColor()
{
	int iI;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		plotByIndexINLINE(iI)->updateMinimapColor();
	}
}


void CvMap::updateSight(bool bIncrement)
{
	int iI;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		if (bIncrement)
		{
			plotByIndexINLINE(iI)->updateSight(true);
		}
		else
		{
			plotByIndexINLINE(iI)->clearAllVisibility();
		}
	}
}


void CvMap::updateIrrigated()
{
	int iI;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		plotByIndexINLINE(iI)->updateIrrigated();
	}
}


void CvMap::updateCenterUnit()
{
	int iI;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		plotByIndexINLINE(iI)->updateCenterUnit();
	}
}


void CvMap::updateWorkingCity()
{
	int iI;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		plotByIndexINLINE(iI)->updateWorkingCity();
	}
}


void CvMap::updateMinOriginalStartDist(CvArea* pArea)
{
	PROFILE_FUNC();

	CvPlot* pStartingPlot;
	CvPlot* pLoopPlot;
	int iDist;
	int iI, iJ;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		pLoopPlot = plotByIndexINLINE(iI);

		if (pLoopPlot->area() == pArea)
		{
			pLoopPlot->setMinOriginalStartDist(-1);
		}
	}

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		pStartingPlot = GET_PLAYER((PlayerTypes)iI).getStartingPlot();

		if (pStartingPlot != NULL)
		{
			if (pStartingPlot->area() == pArea)
			{
				for (iJ = 0; iJ < numPlotsINLINE(); iJ++)
				{
					pLoopPlot = plotByIndexINLINE(iJ);

					if (pLoopPlot->area() == pArea)
					{
						iDist = GC.getMapINLINE().calculatePathDistance(pStartingPlot, pLoopPlot);

						if (iDist != -1)
						{
						    int iCrowDistance = plotDistance(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE());
						    iDist = min(iDist,  iCrowDistance * 2);
							if ((pLoopPlot->getMinOriginalStartDist() == -1) || (iDist < pLoopPlot->getMinOriginalStartDist()))
							{
								pLoopPlot->setMinOriginalStartDist(iDist);
							}
						}
					}
				}
			}
		}
	}
}


void CvMap::updateYield()
{
	int iI;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		plotByIndexINLINE(iI)->updateYield();
	}
}


void CvMap::verifyUnitValidPlot()
{
	int iI;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		plotByIndexINLINE(iI)->verifyUnitValidPlot();
	}
}


void CvMap::combinePlotGroups(PlayerTypes ePlayer, CvPlotGroup* pPlotGroup1, CvPlotGroup* pPlotGroup2)
{
	CLLNode<XYCoords>* pPlotNode;
	CvPlotGroup* pNewPlotGroup;
	CvPlotGroup* pOldPlotGroup;
	CvPlot* pPlot;

	FAssertMsg(pPlotGroup1 != NULL, "pPlotGroup is not assigned to a valid value");
	FAssertMsg(pPlotGroup2 != NULL, "pPlotGroup is not assigned to a valid value");

	FAssertMsg(pPlotGroup1 != NULL, "pPlotGroup is not assigned to a valid value");
	FAssertMsg(pPlotGroup2 != NULL, "pPlotGroup is not assigned to a valid value");

	if (pPlotGroup1 == pPlotGroup2)
	{
		return;
	}

	if (pPlotGroup1->getLengthPlots() > pPlotGroup2->getLengthPlots())
	{
		pNewPlotGroup = pPlotGroup1;
		pOldPlotGroup = pPlotGroup2;
	}
	else
	{
		pNewPlotGroup = pPlotGroup2;
		pOldPlotGroup = pPlotGroup1;
	}

	pPlotNode = pOldPlotGroup->headPlotsNode();

	while (pPlotNode != NULL)
	{
		pPlot = plotSorenINLINE(pPlotNode->m_data.iX, pPlotNode->m_data.iY);

		pNewPlotGroup->addPlot(pPlot);

		pPlotNode = pOldPlotGroup->deletePlotsNode(pPlotNode);
	}
}


CvPlot* CvMap::syncRandPlot(int iFlags, int iArea, int iMinUnitDistance, int iTimeout)
{
	CvPlot* pPlot;
	CvPlot* pTestPlot;
	CvPlot* pLoopPlot;
	bool bValid;
	int iCount;
	int iDX, iDY;

	pPlot = NULL;

	iCount = 0;

	while (iCount < iTimeout)
	{
		pTestPlot = plotSorenINLINE(GC.getGameINLINE().getSorenRandNum(getGridWidthINLINE(), "Rand Plot Width"), GC.getGameINLINE().getSorenRandNum(getGridHeightINLINE(), "Rand Plot Height"));

		FAssertMsg(pTestPlot != NULL, "TestPlot is not assigned a valid value");

		if ((iArea == -1) || (pTestPlot->getArea() == iArea))
		{
			bValid = true;

			if (bValid)
			{
				if (iMinUnitDistance != -1)
				{
					for (iDX = -(iMinUnitDistance); iDX <= iMinUnitDistance; iDX++)
					{
						for (iDY = -(iMinUnitDistance); iDY <= iMinUnitDistance; iDY++)
						{
							pLoopPlot	= plotXY(pTestPlot->getX_INLINE(), pTestPlot->getY_INLINE(), iDX, iDY);

							if (pLoopPlot != NULL)
							{
								if (pLoopPlot->isUnit())
								{
									bValid = false;
								}
							}
						}
					}
				}
			}

			if (bValid)
			{
				if (iFlags & RANDPLOT_LAND)
				{
					if (pTestPlot->isWater())
					{
						bValid = false;
					}
				}
			}

			if (bValid)
			{
				if (iFlags & RANDPLOT_UNOWNED)
				{
					if (pTestPlot->isOwned())
					{
						bValid = false;
					}
				}
			}

			if (bValid)
			{
				if (iFlags & RANDPLOT_ADJACENT_UNOWNED)
				{
					if (pTestPlot->isAdjacentOwned())
					{
						bValid = false;
					}
				}
			}

			if (bValid)
			{
				if (iFlags & RANDPLOT_ADJACENT_LAND)
				{
					if (!(pTestPlot->isAdjacentToLand()))
					{
						bValid = false;
					}
				}
			}

			if (bValid)
			{
				if (iFlags & RANDPLOT_PASSIBLE)
				{
					if (pTestPlot->isImpassable())
					{
						bValid = false;
					}
				}
			}

			if (bValid)
			{
				if (iFlags & RANDPLOT_NOT_VISIBLE_TO_CIV)
				{
					if (pTestPlot->isVisibleToCivTeam())
					{
						bValid = false;
					}
				}
			}

			if (bValid)
			{
				if (iFlags & RANDPLOT_NOT_CITY)
				{
					if (pTestPlot->isCity())
					{
						bValid = false;
					}
				}
			}

			if (bValid)
			{
				pPlot = pTestPlot;
				break;
			}
		}

		iCount++;
	}

	return pPlot;
}


CvCity* CvMap::findCity(int iX, int iY, PlayerTypes eOwner, TeamTypes eTeam, bool bSameArea, bool bCoastalOnly, TeamTypes eTeamAtWarWith, DirectionTypes eDirection, CvCity* pSkipCity)
{
	PROFILE_FUNC();

	CvCity* pLoopCity;
	CvCity* pBestCity;
	int iValue;
	int iBestValue;
	int iLoop;
	int iI;

	// XXX look for barbarian cities???

	iBestValue = MAX_INT;
	pBestCity = NULL;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if ((eOwner == NO_PLAYER) || (iI == eOwner))
			{
				if ((eTeam == NO_TEAM) || (GET_PLAYER((PlayerTypes)iI).getTeam() == eTeam))
				{
					for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
					{
						if (!bSameArea || (pLoopCity->area() == plotINLINE(iX, iY)->area()) || (bCoastalOnly && (pLoopCity->waterArea() == plotINLINE(iX, iY)->area())))
						{
							if (!bCoastalOnly || pLoopCity->isCoastal(GC.getDefineINT("MIN_WATER_SIZE_FOR_OCEAN")))
							{
								if ((eTeamAtWarWith == NO_TEAM) || atWar(GET_PLAYER((PlayerTypes)iI).getTeam(), eTeamAtWarWith))
								{
									if ((eDirection == NO_DIRECTION) || (estimateDirection(dxWrap(pLoopCity->getX_INLINE() - iX), dyWrap(pLoopCity->getY_INLINE() - iY)) == eDirection))
									{
										if ((pSkipCity == NULL) || (pLoopCity != pSkipCity))
										{
											iValue = plotDistance(iX, iY, pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE());

											if (iValue < iBestValue)
											{
												iBestValue = iValue;
												pBestCity = pLoopCity;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return pBestCity;
}


CvSelectionGroup* CvMap::findSelectionGroup(int iX, int iY, PlayerTypes eOwner, bool bReadyToSelect, bool bWorkers)
{
	CvSelectionGroup* pLoopSelectionGroup;
	CvSelectionGroup* pBestSelectionGroup;
	int iValue;
	int iBestValue;
	int iLoop;
	int iI;

	// XXX look for barbarian cities???

	iBestValue = MAX_INT;
	pBestSelectionGroup = NULL;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if ((eOwner == NO_PLAYER) || (iI == eOwner))
			{
				for(pLoopSelectionGroup = GET_PLAYER((PlayerTypes)iI).firstSelectionGroup(&iLoop); pLoopSelectionGroup != NULL; pLoopSelectionGroup = GET_PLAYER((PlayerTypes)iI).nextSelectionGroup(&iLoop))
				{
					if (!bReadyToSelect || pLoopSelectionGroup->readyToSelect())
					{
						if (!bWorkers || pLoopSelectionGroup->hasWorker())
						{
							iValue = plotDistance(iX, iY, pLoopSelectionGroup->getX(), pLoopSelectionGroup->getY());

							if (iValue < iBestValue)
							{
								iBestValue = iValue;
								pBestSelectionGroup = pLoopSelectionGroup;
							}
						}
					}
				}
			}
		}
	}

	return pBestSelectionGroup;
}


CvArea* CvMap::findBiggestArea(bool bWater)
{
	CvArea* pLoopArea;
	CvArea* pBestArea;
	int iValue;
	int iBestValue;
	int iLoop;

	iBestValue = 0;
	pBestArea = NULL;

	for(pLoopArea = firstArea(&iLoop); pLoopArea != NULL; pLoopArea = nextArea(&iLoop))
	{
		if (pLoopArea->isWater() == bWater)
		{
			iValue = pLoopArea->getNumTiles();

			if (iValue > iBestValue)
			{
				iBestValue = iValue;
				pBestArea = pLoopArea;
			}
		}
	}

	return pBestArea;
}


int CvMap::getMapFractalFlags()
{
	int wrapX = 0;
	if (isWrapXINLINE())
	{
		wrapX = (int)CvFractal::FRAC_WRAP_X;
	}

	int wrapY = 0;
	if (isWrapYINLINE())
	{
		wrapY = (int)CvFractal::FRAC_WRAP_Y;
	}

	return (wrapX | wrapY);
}


//"Check plots for wetlands or seaWater.  Returns true if found"
bool CvMap::findWater(CvPlot* pPlot, int iRange, bool bFreshWater)
{
	PROFILE("CvMap::findWater()");

	CvPlot* pLoopPlot;
	int iDX, iDY;

	for (iDX = -(iRange); iDX <= iRange; iDX++)
	{
		for (iDY = -(iRange); iDY <= iRange; iDY++)
		{
			pLoopPlot	= plotXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (bFreshWater)
				{
					if (pLoopPlot->isFreshWater())
					{
						return true;
					}
				}
				else
				{
					if (pLoopPlot->isWater())
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}


bool CvMap::isPlot(int iX, int iY) const
{
	return isPlotINLINE(iX, iY);
}


int CvMap::numPlots() const																											 
{
	return numPlotsINLINE();
}


int CvMap::plotNum(int iX, int iY) const
{
	return plotNumINLINE(iX, iY);
}


int CvMap::plotX(int iIndex) const
{
	return (iIndex % getGridWidthINLINE());
}


int CvMap::plotY(int iIndex) const
{
	return (iIndex / getGridWidthINLINE());
}


int CvMap::pointXToPlotX(float fX)
{
	float fWidth, fHeight;
	gDLL->getEngineIFace()->GetLandscapeGameDimensions(fWidth, fHeight);
	return (int)(((fX + (fWidth/2.0f)) / fWidth) * getGridWidthINLINE());
}


float CvMap::plotXToPointX(int iX)
{
	float fWidth, fHeight;
	gDLL->getEngineIFace()->GetLandscapeGameDimensions(fWidth, fHeight);
	return ((iX * fWidth) / ((float)getGridWidthINLINE())) - (fWidth / 2.0f) + (GC.getDefineFLOAT( "PLOT_SIZE" ) / 2.0f);
}


int CvMap::pointYToPlotY(float fY)
{
	float fWidth, fHeight;
	gDLL->getEngineIFace()->GetLandscapeGameDimensions(fWidth, fHeight);
	return (int)(((fY + (fHeight/2.0f)) / fHeight) * getGridHeightINLINE());
}


float CvMap::plotYToPointY(int iY)
{
	float fWidth, fHeight;
	gDLL->getEngineIFace()->GetLandscapeGameDimensions(fWidth, fHeight);
	return ((iY * fHeight) / ((float)getGridHeightINLINE())) - (fHeight / 2.0f) + (GC.getDefineFLOAT( "PLOT_SIZE" ) / 2.0f);
}


float CvMap::getWidthCoords()																	
{
	return (GC.getDefineFLOAT( "PLOT_SIZE" ) * ((float)getGridWidthINLINE()));
}


float CvMap::getHeightCoords()																	
{
	return (GC.getDefineFLOAT( "PLOT_SIZE" ) * ((float)getGridHeightINLINE()));
}


int CvMap::maxPlotDistance()
{
	return max(1, plotDistance(0, 0, ((isWrapXINLINE()) ? (getGridWidthINLINE() / 2) : (getGridWidthINLINE() - 1)), ((isWrapYINLINE()) ? (getGridHeightINLINE() / 2) : (getGridHeightINLINE() - 1))));
}


int CvMap::maxStepDistance()
{
	return max(1, stepDistance(0, 0, ((isWrapXINLINE()) ? (getGridWidthINLINE() / 2) : (getGridWidthINLINE() - 1)), ((isWrapYINLINE()) ? (getGridHeightINLINE() / 2) : (getGridHeightINLINE() - 1))));
}


int CvMap::getGridWidth() const
{
	return getGridWidthINLINE();
}


int CvMap::getGridHeight() const
{
	return getGridHeightINLINE();
}


int CvMap::getLandPlots()
{
	return m_iLandPlots;
}


void CvMap::changeLandPlots(int iChange)
{
	m_iLandPlots = (m_iLandPlots + iChange);
	FAssert(getLandPlots() >= 0);
}


int CvMap::getOwnedPlots()
{
	return m_iOwnedPlots;
}


void CvMap::changeOwnedPlots(int iChange)
{
	m_iOwnedPlots = (m_iOwnedPlots + iChange);
	FAssert(getOwnedPlots() >= 0);
}


int CvMap::getTopLatitude()
{
	return m_iTopLatitude;
}


int CvMap::getBottomLatitude()
{
	return m_iBottomLatitude;
}


int CvMap::getNextRiverID()
{
	return m_iNextRiverID;
}


void CvMap::incrementNextRiverID()
{
	m_iNextRiverID++;
}


bool CvMap::isWrapX()
{
	return isWrapXINLINE();
}


bool CvMap::isWrapY()
{
	return isWrapYINLINE();
}

bool CvMap::isWrap()
{
	return isWrapINLINE();
}

WorldSizeTypes CvMap::getWorldSize()
{
	return GC.getInitCore().getWorldSize();
}


ClimateTypes CvMap::getClimate()
{
	return GC.getInitCore().getClimate();
}


SeaLevelTypes CvMap::getSeaLevel()
{
	return GC.getInitCore().getSeaLevel();
}



int CvMap::getNumCustomMapOptions()
{
	return GC.getInitCore().getNumCustomMapOptions();
}


CustomMapOptionTypes CvMap::getCustomMapOption(int iOption)
{
	return GC.getInitCore().getCustomMapOption(iOption);
}


int CvMap::getNumBonuses(BonusTypes eIndex)													
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumBonusInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiNumBonus[eIndex];
}


void CvMap::changeNumBonuses(BonusTypes eIndex, int iChange)									
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumBonusInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_paiNumBonus[eIndex] = (m_paiNumBonus[eIndex] + iChange);
	FAssert(getNumBonuses(eIndex) >= 0);
}


int CvMap::getNumBonusesOnLand(BonusTypes eIndex)													
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumBonusInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiNumBonusOnLand[eIndex];
}


void CvMap::changeNumBonusesOnLand(BonusTypes eIndex, int iChange)									
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumBonusInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_paiNumBonusOnLand[eIndex] = (m_paiNumBonusOnLand[eIndex] + iChange);
	FAssert(getNumBonusesOnLand(eIndex) >= 0);
}


CvPlot* CvMap::plotByIndex(int iIndex) const
{
	return plotByIndexINLINE(iIndex);
}


CvPlot* CvMap::plot(int iX, int iY) const
{
	return plotINLINE(iX, iY);
}


CvPlot* CvMap::pointToPlot(float fX, float fY)													
{
	return plotINLINE(pointXToPlotX(fX), pointYToPlotY(fY));
}


int CvMap::getIndexAfterLastArea()																
{
	return m_areas.getIndexAfterLast();
}


int CvMap::getNumAreas()																		
{
	return m_areas.getCount();
}


int CvMap::getNumLandAreas()
{
	CvArea* pLoopArea;
	int iNumLandAreas;
	int iLoop;

	iNumLandAreas = 0;

	for(pLoopArea = GC.getMap().firstArea(&iLoop); pLoopArea != NULL; pLoopArea = GC.getMap().nextArea(&iLoop))
	{
		if (!(pLoopArea->isWater()))
		{
			iNumLandAreas++;
		}
	}

	return iNumLandAreas;
}


CvArea* CvMap::getArea(int iID)																
{
	return m_areas.getAt(iID);
}


CvArea* CvMap::addArea()
{
	return m_areas.add();
}


void CvMap::deleteArea(int iID)
{
	m_areas.removeAt(iID);
}


CvArea* CvMap::firstArea(int *pIterIdx, bool bRev)
{
	return !bRev ? m_areas.beginIter(pIterIdx) : m_areas.endIter(pIterIdx);
}


CvArea* CvMap::nextArea(int *pIterIdx, bool bRev)
{
	return !bRev ? m_areas.nextIter(pIterIdx) : m_areas.prevIter(pIterIdx);
}


void CvMap::recalculateAreas()
{
	PROFILE("CvMap::recalculateAreas");

	int iI;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		plotByIndexINLINE(iI)->setArea(FFreeList::INVALID_INDEX);
	}

	m_areas.removeAll();

	calculateAreas();
}


void CvMap::resetPathDistance()
{
	gDLL->getFAStarIFace()->ForceReset(&GC.getStepFinder());
}


int CvMap::calculatePathDistance(CvPlot *pSource, CvPlot *pDest)
{
	FAStarNode* pNode;

	if (pSource == NULL || pDest == NULL)
	{
		return -1;
	}

	if (gDLL->getFAStarIFace()->GeneratePath(&GC.getStepFinder(), pSource->getX_INLINE(), pSource->getY_INLINE(), pDest->getX_INLINE(), pDest->getY_INLINE(), false, 0, true))
	{
		pNode = gDLL->getFAStarIFace()->GetLastNode(&GC.getStepFinder());

		if (pNode != NULL)
		{
			return pNode->m_iData1;
		}
	}

	return -1; // no passable path exists
}

//
// read object from a stream
// used during load
//
void CvMap::read(FDataStreamBase* pStream)
{
	CvMapInitData defaultMapData;

	// Init data before load
	reset(&defaultMapData);

	uint uiFlag=0;
	pStream->Read(&uiFlag);	// flags for expansion

	pStream->Read(&m_iGridWidth);
	pStream->Read(&m_iGridHeight);
	pStream->Read(&m_iLandPlots);
	pStream->Read(&m_iOwnedPlots);
	pStream->Read(&m_iTopLatitude);
	pStream->Read(&m_iBottomLatitude);
	pStream->Read(&m_iNextRiverID);

	pStream->Read(&m_bWrapX);
	pStream->Read(&m_bWrapY);

	FAssertMsg((0 < GC.getNumBonusInfos()), "GC.getNumBonusInfos() is not greater than zero but an array is being allocated");
	pStream->Read(GC.getNumBonusInfos(), m_paiNumBonus);
	pStream->Read(GC.getNumBonusInfos(), m_paiNumBonusOnLand);

	if (numPlotsINLINE() > 0)
	{
		m_pMapPlots = new CvPlot[numPlotsINLINE()];
		int iI;
		for (iI = 0; iI < numPlotsINLINE(); iI++)
		{
			m_pMapPlots[iI].read(pStream);
		}
	}
	

	// read in the save info from the found values, and set those values to -1 (cleaning them)
	CvExtraSaveData& extraSaveData = getExtraSaveDataClass();
	extraSaveData.setupForRead(*this);
	extraSaveData.cleanPlotValues(*this);

	// call the read of the free list CvArea class allocations
	ReadStreamableFFreeListTrashArray(m_areas, pStream);

	setup();

	// get the map saved data for ourselves
	int iVersion;
	int iMapVisionType = -1;
	if (extraSaveData.getSaveData(EXTRASAVEDATA_MAP, 0, 0, MAPEXTRADATA_VERSION, iVersion))
	{
		if (iVersion == MAPEXTRADATA_CURRENTVERSION)
		{
			if (extraSaveData.getSaveData(EXTRASAVEDATA_MAP, 0, 0, MAPEXTRADATA_MAPVISIONTYPE, iMapVisionType))
			{
				// dont save vision type, we always save as current vision type
				// m_iMapVisionType = iMapVisionType;
			}
		}
	}

#ifdef FASTER_VISIBILITY_NON_SAVE_COMPATABLE
	if (iMapVisionType != MAPEXTRADATA_MAPVISIONTYPE_CURRENT)
#else
	if (iMapVisionType > MAPEXTRADATA_MAPVISIONTYPE_CURRENT)
#endif
	if (false)
	{
		//m_bVisibiltyDirty = false;

		// force refresh of all visibility on a load ,incompatable vision type
		updateSight(false);
		updateSight(true);
	}

}

// save object to a stream
// used during save
//
void CvMap::write(FDataStreamBase* pStream)
{
	uint uiFlag=0;
	pStream->Write(uiFlag);		// flag for expansion

	pStream->Write(m_iGridWidth);
	pStream->Write(m_iGridHeight);
	pStream->Write(m_iLandPlots);
	pStream->Write(m_iOwnedPlots);
	pStream->Write(m_iTopLatitude);
	pStream->Write(m_iBottomLatitude);
	pStream->Write(m_iNextRiverID);

	pStream->Write(m_bWrapX);
	pStream->Write(m_bWrapY);

	FAssertMsg((0 < GC.getNumBonusInfos()), "GC.getNumBonusInfos() is not greater than zero but an array is being allocated");
	pStream->Write(GC.getNumBonusInfos(), m_paiNumBonus);
	pStream->Write(GC.getNumBonusInfos(), m_paiNumBonusOnLand);

	// push the extra save info to the plot found values
	CvExtraSaveData& extraSaveData = getExtraSaveDataClass();
	extraSaveData.setupForWrite(*this);
	extraSaveData.writeToPlots(*this);

	int iI;	
	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		m_pMapPlots[iI].write(pStream);
	}

	// restore the plot found values
	extraSaveData.restorePlotValues(*this);
	extraSaveData.invalidate();

	// call the read of the free list CvArea class allocations
	WriteStreamableFFreeListTrashArray(m_areas, pStream);
}


//
// used for loading WB maps
//
void CvMap::rebuild(int iGridW, int iGridH, int iTopLatitude, int iBottomLatitude, bool bWrapX, bool bWrapY, WorldSizeTypes eWorldSize, ClimateTypes eClimate, SeaLevelTypes eSeaLevel, int iNumCustomMapOptions, CustomMapOptionTypes * aeCustomMapOptions)
{
	CvMapInitData initData(iGridW, iGridH, iTopLatitude, iBottomLatitude, bWrapX, bWrapY);

	// Set init core data
	GC.getInitCore().setWorldSize(eWorldSize);
	GC.getInitCore().setClimate(eClimate);
	GC.getInitCore().setSeaLevel(eSeaLevel);
	GC.getInitCore().setCustomMapOptions(iNumCustomMapOptions, aeCustomMapOptions);

	// Init map
	init(&initData);
}


//////////////////////////////////////////////////////////////////////////
// Protected Functions...
//////////////////////////////////////////////////////////////////////////

void CvMap::calculateAreas()
{
	PROFILE("CvMap::calculateAreas");
	CvPlot* pLoopPlot;
	CvArea* pArea;
	int iArea;
	int iI;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		pLoopPlot = plotByIndexINLINE(iI);
		gDLL->callUpdater();
		FAssertMsg(pLoopPlot != NULL, "LoopPlot is not assigned a valid value");

		if (pLoopPlot->getArea() == FFreeList::INVALID_INDEX)
		{
			pArea = addArea();
			pArea->init(pArea->getID(), pLoopPlot->isWater());

			iArea = pArea->getID();

			pLoopPlot->setArea(iArea);

			gDLL->getFAStarIFace()->GeneratePath(&GC.getAreaFinder(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), -1, -1, pLoopPlot->isWater(), iArea);
		}
	}
}

void CvMap::writeExtraSaveData(CvExtraSaveData& extraSaveData)
{
	int iVersion = MAPEXTRADATA_CURRENTVERSION;
	extraSaveData.setSaveData(EXTRASAVEDATA_MAP, 0, 0, MAPEXTRADATA_VERSION, iVersion);

#ifdef FASTER_VISIBILITY_NON_SAVE_COMPATABLE
	// for now only save for faster version (works fine if you remove the #ifdef though)
	int iMapVisionType = MAPEXTRADATA_MAPVISIONTYPE_CURRENT;
	extraSaveData.setSaveData(EXTRASAVEDATA_MAP, 0, 0, MAPEXTRADATA_MAPVISIONTYPE, iMapVisionType);
#endif
}


// CACHE: cache frequently used values
///////////////////////////////////////

// public functions here
int CvMap::getCachedExploreValue(const CvPlot* pPlot, const CvUnit* pUnit)
{	
	// if cache valid, get the value, otherwise, clear the whole cache
	int iTurnSlice = GC.getGameINLINE().getTurnSlice();
	if (m_iLastExploreTurnSlice == iTurnSlice && m_iLastExplorePlayerID == pUnit->getOwnerINLINE() && m_iLastExploreUnitID == pUnit->getID())
	{
		int iPlotNum = plotNumINLINE(pPlot->getX_INLINE(), pPlot->getY_INLINE());
		return m_paiCachedExploreValues[iPlotNum];
	}
	else
	{
		// remember for next time
		m_iLastExploreTurnSlice = iTurnSlice;
		m_iLastExplorePlayerID = pUnit->getOwnerINLINE();
		m_iLastExploreUnitID = pUnit->getID();

		// initialize all values to -1
		for (int iI = 0; iI < m_iCachedExploreValuesSize; iI++)
		{
			m_paiCachedExploreValues[iI] = -1;
		}
	}
	
	return -1;
}

void CvMap::setCachedExploreValue(const CvPlot* pPlot, const CvUnit* pUnit, int iValue)
{
	FAssertMsg(m_iCachedExploreValuesSize == numPlotsINLINE(), "explore value cache size incorrect");
	
	// if cache invalid, clear the whole cache first
	int iTurnSlice = GC.getGameINLINE().getTurnSlice();
	if (m_iLastExploreTurnSlice != iTurnSlice || m_iLastExplorePlayerID != pUnit->getOwnerINLINE() || m_iLastExploreUnitID != pUnit->getID())
	{
		// remember for next time
		m_iLastExploreTurnSlice = iTurnSlice;
		m_iLastExplorePlayerID = pUnit->getOwnerINLINE();
		m_iLastExploreUnitID = pUnit->getID();

		// initialize all values to -1
		for (int iI = 0; iI < m_iCachedExploreValuesSize; iI++)
		{
			m_paiCachedExploreValues[iI] = -1;
		}
	}
	
	// store the value
	int iPlotNum = plotNumINLINE(pPlot->getX_INLINE(), pPlot->getY_INLINE());
	m_paiCachedExploreValues[iPlotNum] = iValue;
}


void CvMap::constructCache()
{
	// allocate known constants

	// zero everything out, just to be safe
	m_iCachedExploreValuesSize = 0;
	m_paiCachedExploreValues = NULL;
	
	// note resetCache(/*bConstructorCall*/ true) will be called by CvMap::CvMap
}

void CvMap::destructCache()
{
	uninitCache(); // note, will be called by CvMap::~CvMap as well

	// deallocate anything allocated in constructor
}

void CvMap::initCache()
{
	// init non-saved data

	// note resetCache(/*bConstructorCall*/ false) will be called by CvMap::init
}

void CvMap::uninitCache()
{
	m_iLastExplorePlayerID = FFreeList::INVALID_INDEX;
	m_iLastExploreUnitID = FFreeList::INVALID_INDEX;
	m_iLastExploreTurnSlice = -1;

	m_iCachedExploreValuesSize = 0;
	SAFE_DELETE_ARRAY(m_paiCachedExploreValues);
}

void CvMap::resetCache(bool bConstructorCall)
{
	int	iI;
	
	uninitCache(); // note, will be called by CvMap::reset as well
	
	if (!bConstructorCall)
	{
		FAssertMsg(m_paiCachedExploreValues==NULL, "about to leak memory, CvGame::resetCache");
		m_iCachedExploreValuesSize = numPlotsINLINE();
		m_paiCachedExploreValues = new int[m_iCachedExploreValuesSize];
		for (iI = 0; iI < m_iCachedExploreValuesSize; iI++)
		{
			m_paiCachedExploreValues[iI] = -1;
		}
	}
}


void CvMap::doUpdateCacheOnTurn()
{
	// not necessary, but cannot hurt
	m_iLastExplorePlayerID = FFreeList::INVALID_INDEX;
	m_iLastExploreUnitID = FFreeList::INVALID_INDEX;
	m_iLastExploreTurnSlice = -1;
}
