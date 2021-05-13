#include "CvGameCoreDLL.h"
#include "CvGame.h"
#include "CvInitCore.h"
#include "CyPlot.h"
#include "CyArgsList.h"
#include "CvPopupInfo.h"

#include "CvDLLInterfaceIFaceBase.h"
#include "CvDLLEngineIFaceBase.h"

void CvGame::updateColoredPlots()
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pSelectedCityNode;
	CLLNode<IDInfo>* pSelectedUnitNode;
	CvCity* pHeadSelectedCity;
	CvCity* pSelectedCity;
	CvCity* pCity;
	CvUnit* pHeadSelectedUnit;
	CvUnit* pSelectedUnit;
	CvPlot* pRallyPlot;
	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	CvPlot* pNextBestPlot;
	long lResult;
	int iMaxAirRange;
	int iRange;
	int iDX, iDY;
	int iI;

	gDLL->getEngineIFace()->clearColoredPlots(PLOT_LANDSCAPE_LAYER_BASE);
	gDLL->getEngineIFace()->clearAreaBorderPlots(AREA_BORDER_LAYER_CITY_RADIUS);
	gDLL->getEngineIFace()->clearAreaBorderPlots(AREA_BORDER_LAYER_RANGED);
	gDLL->getEngineIFace()->clearAreaBorderPlots(AREA_BORDER_LAYER_BLOCKADING);

	if (!gDLL->GetWorldBuilderMode() || gDLL->getInterfaceIFace()->isInAdvancedStart())
	{
		gDLL->getEngineIFace()->clearColoredPlots(PLOT_LANDSCAPE_LAYER_RECOMMENDED_PLOTS);
	}

	lResult = 0;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "updateColoredPlots", NULL, &lResult);
	if (lResult == 1)
	{
		return;
	}

	// City circles when in Advanced Start
	if (gDLL->getInterfaceIFace()->isInAdvancedStart())
	{
		for (int iPlotLoop = 0; iPlotLoop < GC.getMap().numPlots(); iPlotLoop++)
		{
			CvPlot* pLoopPlot = GC.getMap().plotByIndex(iPlotLoop);

			if (pLoopPlot != NULL)
			{
				if (GET_PLAYER(getActivePlayer()).getAdvancedStartCityCost(true, pLoopPlot) > 0)
				{
					bool bStartingPlot = false;
					for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
					{
						CvPlayer& kPlayer = GET_PLAYER((PlayerTypes) iPlayer);
						if (kPlayer.isAlive() && getActiveTeam() == kPlayer.getTeam())
						{
							if (pLoopPlot == kPlayer.getStartingPlot())
							{
								bStartingPlot = true;
								break;
							}
						}
					}
					if (bStartingPlot)
					{
						gDLL->getEngineIFace()->addColoredPlot(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_WARNING_TEXT")).getColor(), PLOT_STYLE_CIRCLE, PLOT_LANDSCAPE_LAYER_RECOMMENDED_PLOTS);
					}
					else if (GET_PLAYER(getActivePlayer()).AI_isPlotCitySite(pLoopPlot))
					{
						gDLL->getEngineIFace()->addColoredPlot(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT")).getColor(), PLOT_STYLE_CIRCLE, PLOT_LANDSCAPE_LAYER_RECOMMENDED_PLOTS);
					}

					if (pLoopPlot->isRevealed(getActiveTeam(), false))
					{
						NiColorA color(GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_WHITE")).getColor());
						color.a = 0.4f;
						gDLL->getEngineIFace()->fillAreaBorderPlot(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), color, AREA_BORDER_LAYER_CITY_RADIUS);
					}
				}
			}
		}
	}

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();

	if (pHeadSelectedCity != NULL)
	{
		if (gDLL->getInterfaceIFace()->isCityScreenUp())
		{
			for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
			{
				if (pHeadSelectedCity->isWorkingPlot(iI))
				{
					pLoopPlot = plotCity(pHeadSelectedCity->getX_INLINE(), pHeadSelectedCity->getY_INLINE(), iI);

					if (pLoopPlot != NULL)
					{
						NiColorA color(GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_WHITE")).getColor());
						color.a = 0.7f;
						gDLL->getEngineIFace()->addColoredPlot(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), color, PLOT_STYLE_CIRCLE, PLOT_LANDSCAPE_LAYER_BASE);
					}
				}
			}
		}
		else
		{
			pSelectedCityNode = gDLL->getInterfaceIFace()->headSelectedCitiesNode();

			while (pSelectedCityNode != NULL)
			{
				pSelectedCity = ::getCity(pSelectedCityNode->m_data);
				pSelectedCityNode = gDLL->getInterfaceIFace()->nextSelectedCitiesNode(pSelectedCityNode);

				if (pSelectedCity != NULL)
				{
					pRallyPlot = pSelectedCity->getRallyPlot();

					if (pRallyPlot != NULL)
					{
						gDLL->getEngineIFace()->addColoredPlot(pRallyPlot->getX_INLINE(), pRallyPlot->getY_INLINE(), GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_YELLOW")).getColor(), PLOT_STYLE_CIRCLE, PLOT_LANDSCAPE_LAYER_BASE);
					}
				}
			}
		}
	}
	else if (pHeadSelectedUnit != NULL)
	{
		if (gDLL->getGraphicOption(GRAPHICOPTION_CITY_RADIUS))
		{
			if (gDLL->getInterfaceIFace()->canSelectionListFound())
			{
				for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
				{
					pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

					if (pLoopPlot->getOwnerINLINE() == pHeadSelectedUnit->getOwnerINLINE())
					{
						if (pLoopPlot->getWorkingCity() != NULL)
						{
							NiColorA color(GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT")/*(GC.getPlayerColorInfo(GET_PLAYER(pHeadSelectedUnit->getOwnerINLINE()).getPlayerColor()).getColorTypePrimary())*/).getColor());
							color.a = 1.0f;
							gDLL->getEngineIFace()->fillAreaBorderPlot(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), color, AREA_BORDER_LAYER_CITY_RADIUS);
						}
					}
				}
			}
		}

		if (pHeadSelectedUnit->getDomainType() == DOMAIN_AIR)
		{
			iMaxAirRange = 0;

			pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

			while (pSelectedUnitNode != NULL)
			{
				pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);
				pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);

				if (pSelectedUnit != NULL)
				{
					iMaxAirRange = std::max(iMaxAirRange, pSelectedUnit->airRange());
				}
			}

			if (iMaxAirRange > 0)
			{
				for (iDX = -(iMaxAirRange); iDX <= iMaxAirRange; iDX++)
				{
					for (iDY = -(iMaxAirRange); iDY <= iMaxAirRange; iDY++)
					{
						pLoopPlot = plotXY(pHeadSelectedUnit->getX_INLINE(), pHeadSelectedUnit->getY_INLINE(), iDX, iDY);

						if (pLoopPlot != NULL)
						{
							if (plotDistance(pHeadSelectedUnit->getX_INLINE(), pHeadSelectedUnit->getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE()) <= iMaxAirRange)
							{
								NiColorA color(GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_YELLOW")).getColor());
								color.a = 0.5f;
								gDLL->getEngineIFace()->fillAreaBorderPlot(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), color, AREA_BORDER_LAYER_RANGED);
							}
						}
					}
				}
			}
		}
		else if(pHeadSelectedUnit->airRange() > 0) //other ranged units
		{
			int iRange = pHeadSelectedUnit->airRange();
			for (iDX = -(iRange); iDX <= iRange; iDX++)
			{
				for (iDY = -(iRange); iDY <= iRange; iDY++)
				{
					CvPlot* pTargetPlot = plotXY(pHeadSelectedUnit->getX_INLINE(), pHeadSelectedUnit->getY_INLINE(), iDX, iDY);

					if (pTargetPlot != NULL && pTargetPlot->isVisible(pHeadSelectedUnit->getTeam(), false))
					{
						if (plotDistance(pHeadSelectedUnit->getX_INLINE(), pHeadSelectedUnit->getY_INLINE(), pTargetPlot->getX_INLINE(), pTargetPlot->getY_INLINE()) <= iRange)
						{
							if (pHeadSelectedUnit->plot()->canSeePlot(pTargetPlot, pHeadSelectedUnit->getTeam(), iRange, pHeadSelectedUnit->getFacingDirection(true)))
							{
								NiColorA color(GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_YELLOW")).getColor());
								color.a = 0.5f;
								gDLL->getEngineIFace()->fillAreaBorderPlot(pTargetPlot->getX_INLINE(), pTargetPlot->getY_INLINE(), color, AREA_BORDER_LAYER_RANGED);
							}
						}
					}
				}
			}
		}

		FAssert(getActivePlayer() != NO_PLAYER);

		if (!(GET_PLAYER(getActivePlayer()).isOption(PLAYEROPTION_NO_UNIT_RECOMMENDATIONS)))
		{
			if ((pHeadSelectedUnit->AI_getUnitAIType() == UNITAI_WORKER) || (pHeadSelectedUnit->AI_getUnitAIType() == UNITAI_WORKER_SEA))
			{
				if (pHeadSelectedUnit->plot()->getOwnerINLINE() == pHeadSelectedUnit->getOwnerINLINE())
				{
					pCity = pHeadSelectedUnit->plot()->getWorkingCity();

					if (pCity != NULL)
					{
						if (pHeadSelectedUnit->AI_bestCityBuild(pCity, &pBestPlot))
						{
							FAssert(pBestPlot != NULL);
							gDLL->getEngineIFace()->addColoredPlot(pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT")).getColor(), PLOT_STYLE_CIRCLE, PLOT_LANDSCAPE_LAYER_RECOMMENDED_PLOTS);

							if (pHeadSelectedUnit->AI_bestCityBuild(pCity, &pNextBestPlot, NULL, pBestPlot))
							{
								FAssert(pNextBestPlot != NULL);
								gDLL->getEngineIFace()->addColoredPlot(pNextBestPlot->getX_INLINE(), pNextBestPlot->getY_INLINE(), GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT")).getColor(), PLOT_STYLE_CIRCLE, PLOT_LANDSCAPE_LAYER_RECOMMENDED_PLOTS);
							}
						}
					}
				}
			}

			iRange = 4;

			for (iDX = -(iRange); iDX <= iRange; iDX++)
			{
				for (iDY = -(iRange); iDY <= iRange; iDY++)
				{
					pLoopPlot = plotXY(pHeadSelectedUnit->getX_INLINE(), pHeadSelectedUnit->getY_INLINE(), iDX, iDY);

					if (pLoopPlot != NULL)
					{
						if ((pLoopPlot->area() == pHeadSelectedUnit->area()) || pLoopPlot->isAdjacentToArea(pHeadSelectedUnit->area()))
						{
							if (pHeadSelectedUnit->canFound(pLoopPlot))
							{
								if (GET_PLAYER(pHeadSelectedUnit->getOwnerINLINE()).AI_isPlotCitySite(pLoopPlot))
								{
									gDLL->getEngineIFace()->addColoredPlot(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT")).getColor(), PLOT_STYLE_CIRCLE, PLOT_LANDSCAPE_LAYER_RECOMMENDED_PLOTS);
								}
							}
							if (plotDistance(pHeadSelectedUnit->getX_INLINE(), pHeadSelectedUnit->getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE()) <= iRange)
							{
								if (pLoopPlot->isVisible(pHeadSelectedUnit->getTeam(), false))
								{
									if (pHeadSelectedUnit->isNoBadGoodies())
									{
										if (pLoopPlot->isRevealedGoody(pHeadSelectedUnit->getTeam()))
										{
											gDLL->getEngineIFace()->addColoredPlot(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT")).getColor(), PLOT_STYLE_CIRCLE, PLOT_LANDSCAPE_LAYER_RECOMMENDED_PLOTS);
										}
									}
								}
							}
						}
					}
				}
			}
		}

		if (pHeadSelectedUnit->isBlockading())
		{
			int iBlockadeRange = GC.getDefineINT("SHIP_BLOCKADE_RANGE");

			for (int iPlayer = 0; iPlayer < MAX_CIV_PLAYERS; ++iPlayer)
			{
				CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)iPlayer);

				if (kPlayer.getTeam() == getActiveTeam())
				{
					int iLoop;
					for (CvUnit* pLoopUnit = kPlayer.firstUnit(&iLoop); NULL != pLoopUnit; pLoopUnit = kPlayer.nextUnit(&iLoop))
					{
						if (pLoopUnit->isBlockading())
						{
							for (int i = -iBlockadeRange; i <= iBlockadeRange; ++i)
							{
								for (int j = -iBlockadeRange; j <= iBlockadeRange; ++j)
								{
									CvPlot* pLoopPlot = ::plotXY(pLoopUnit->getX_INLINE(), pLoopUnit->getY_INLINE(), i, j);
									if (NULL != pLoopPlot && pLoopPlot->isRevealed(getActiveTeam(), false))
									{
										if (pLoopPlot->isWater() && pLoopPlot->area() == pLoopUnit->area())
										{
											NiColorA color(GC.getColorInfo((ColorTypes)GC.getPlayerColorInfo(GET_PLAYER(getActivePlayer()).getPlayerColor()).getColorTypePrimary()).getColor());
											color.a = 0.5f;
											gDLL->getEngineIFace()->fillAreaBorderPlot(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), color, AREA_BORDER_LAYER_BLOCKADING);
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

void CvGame::updateBlockadedPlots()
{
	PROFILE_FUNC();

	gDLL->getEngineIFace()->clearAreaBorderPlots(AREA_BORDER_LAYER_BLOCKADED);

	int iNumPlots = GC.getMapINLINE().numPlots();
	for (int i = 0; i < iNumPlots; ++i)
	{
		CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndex(i);

		FAssert(NULL != pLoopPlot);

		if (pLoopPlot->getBlockadedCount(getActiveTeam()) > 0 && pLoopPlot->isRevealed(getActiveTeam(), false))
		{
			NiColorA color(GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_BLACK")).getColor());
			color.a = 0.35f;
			gDLL->getEngineIFace()->fillAreaBorderPlot(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), color, AREA_BORDER_LAYER_BLOCKADED);
		}
	}
}


void CvGame::updateSelectionList()
{
	CvUnit* pHeadSelectedUnit;

	if (GET_PLAYER(getActivePlayer()).isOption(PLAYEROPTION_NO_UNIT_CYCLING))
	{
		return;
	}

	pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();

	if ((pHeadSelectedUnit == NULL) || !(pHeadSelectedUnit->getGroup()->readyToSelect(true)))
	{
		if ((gDLL->getInterfaceIFace()->getOriginalPlot() == NULL) || !(cyclePlotUnits(gDLL->getInterfaceIFace()->getOriginalPlot(), true, true, gDLL->getInterfaceIFace()->getOriginalPlotCount())))
		{
			if ((gDLL->getInterfaceIFace()->getSelectionPlot() == NULL) || !(cyclePlotUnits(gDLL->getInterfaceIFace()->getSelectionPlot(), true, true)))
			{
				cycleSelectionGroups(true);
			}
		}

		pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();

		if (pHeadSelectedUnit != NULL)
		{
			if (!(pHeadSelectedUnit->getGroup()->readyToSelect()))
			{
				gDLL->getInterfaceIFace()->clearSelectionList();
			}
		}
	}
}


void CvGame::updateTestEndTurn()
{
	bool bAny;

	bAny = ((gDLL->getInterfaceIFace()->getHeadSelectedUnit() != NULL) && !(GET_PLAYER(getActivePlayer()).isOption(PLAYEROPTION_NO_UNIT_CYCLING)));

	if (GET_PLAYER(getActivePlayer()).isTurnActive())
	{
		if (gDLL->getInterfaceIFace()->isEndTurnMessage())
		{
			if (GET_PLAYER(getActivePlayer()).hasReadyUnit(bAny))
			{
				gDLL->getInterfaceIFace()->setEndTurnMessage(false);
			}
		}
		else
		{
			if (!(GET_PLAYER(getActivePlayer()).hasBusyUnit()) && !(GET_PLAYER(getActivePlayer()).hasReadyUnit(bAny)))
			{
				if (!(gDLL->getInterfaceIFace()->isForcePopup()))
				{
					gDLL->getInterfaceIFace()->setForcePopup(true);
				}
				else
				{
					if (GET_PLAYER(getActivePlayer()).hasAutoUnit())
					{
						if (!(gDLL->shiftKey()))
						{
							gDLL->sendAutoMoves();
						}
					}
					else
					{
						if (GET_PLAYER(getActivePlayer()).isOption(PLAYEROPTION_WAIT_END_TURN) || !(gDLL->getInterfaceIFace()->isHasMovedUnit()) || isHotSeat() || isPbem())
						{
							gDLL->getInterfaceIFace()->setEndTurnMessage(true);
						}
						else
						{
							if (gDLL->getInterfaceIFace()->getEndTurnCounter() > 0)
							{
								gDLL->getInterfaceIFace()->changeEndTurnCounter(-1);
							}
							else
							{
								gDLL->sendTurnComplete();
								gDLL->getInterfaceIFace()->setEndTurnCounter(3); // XXX
							}
						}
					}
				}
			}
		}
	}
}

CvUnit* CvGame::getPlotUnit(const CvPlot* pPlot, int iIndex)
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode1;
	CLLNode<IDInfo>* pUnitNode2;
	CvUnit* pLoopUnit1;
	CvUnit* pLoopUnit2;
	int iCount;
	int iPass;
	PlayerTypes activePlayer = getActivePlayer();
	TeamTypes activeTeam = getActiveTeam();

	if (pPlot != NULL)
	{
		iCount = 0;

		for (iPass = 0; iPass < 2; iPass++)
		{
			pUnitNode1 = pPlot->headUnitNode();

			while (pUnitNode1 != NULL)
			{
				pLoopUnit1 = ::getUnit(pUnitNode1->m_data);
				pUnitNode1 = pPlot->nextUnitNode(pUnitNode1);

				if (!(pLoopUnit1->isInvisible(activeTeam, true)))
				{
					if (!(pLoopUnit1->isCargo()))
					{
						if ((pLoopUnit1->getOwnerINLINE() == activePlayer) == (iPass == 0))
						{
							if (iCount == iIndex)
							{
								return pLoopUnit1;
							}

							iCount++;

							//if ((pLoopUnit1->getTeam() == activeTeam) || isDebugMode())
							{
								if (pLoopUnit1->hasCargo())
								{
									pUnitNode2 = pPlot->headUnitNode();

									while (pUnitNode2 != NULL)
									{
										pLoopUnit2 = ::getUnit(pUnitNode2->m_data);
										pUnitNode2 = pPlot->nextUnitNode(pUnitNode2);

										if (!(pLoopUnit2->isInvisible(activeTeam, true)))
										{
											if (pLoopUnit2->getTransportUnit() == pLoopUnit1)
											{
												if (iCount == iIndex)
												{
													return pLoopUnit2;
												}

												iCount++;
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

	return NULL;
}

void CvGame::getPlotUnits(const CvPlot *pPlot, std::vector<CvUnit *> &plotUnits)
{
	PROFILE_FUNC();
	plotUnits.erase(plotUnits.begin(), plotUnits.end());

	CLLNode<IDInfo>* pUnitNode1;
	CLLNode<IDInfo>* pUnitNode2;
	CvUnit* pLoopUnit1;
	CvUnit* pLoopUnit2;
	int iPass;
	PlayerTypes activePlayer = getActivePlayer();
	TeamTypes activeTeam = getActiveTeam();

	if (pPlot != NULL)
	{
		for (iPass = 0; iPass < 2; iPass++)
		{
			pUnitNode1 = pPlot->headUnitNode();

			while (pUnitNode1 != NULL)
			{
				pLoopUnit1 = ::getUnit(pUnitNode1->m_data);
				pUnitNode1 = pPlot->nextUnitNode(pUnitNode1);

				if (!(pLoopUnit1->isInvisible(activeTeam, true)))
				{
					if (!(pLoopUnit1->isCargo()))
					{
						if ((pLoopUnit1->getOwnerINLINE() == activePlayer) == (iPass == 0))
						{
							plotUnits.push_back(pLoopUnit1);

							//if ((pLoopUnit1->getTeam() == activeTeam) || isDebugMode())
							{
								if (pLoopUnit1->hasCargo())
								{
									pUnitNode2 = pPlot->headUnitNode();

									while (pUnitNode2 != NULL)
									{
										pLoopUnit2 = ::getUnit(pUnitNode2->m_data);
										pUnitNode2 = pPlot->nextUnitNode(pUnitNode2);

										if (!(pLoopUnit2->isInvisible(activeTeam, true)))
										{
											if (pLoopUnit2->getTransportUnit() == pLoopUnit1)
											{
												plotUnits.push_back(pLoopUnit2);
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
}

void CvGame::cycleCities(bool bForward, bool bAdd)
{
	CvCity* pHeadSelectedCity;
	CvCity* pSelectCity;
	CvCity* pLoopCity;
	int iLoop;

	pSelectCity = NULL;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if ((pHeadSelectedCity != NULL) && ((pHeadSelectedCity->getTeam() == getActiveTeam()) || isDebugMode()))
	{
		iLoop = pHeadSelectedCity->getIndex();
		iLoop += (bForward ? 1 : -1);

		pLoopCity = GET_PLAYER(pHeadSelectedCity->getOwnerINLINE()).nextCity(&iLoop, !bForward);

		if (pLoopCity == NULL)
		{
			pLoopCity = GET_PLAYER(pHeadSelectedCity->getOwnerINLINE()).firstCity(&iLoop, !bForward);
		}

		if ((pLoopCity != NULL) && (pLoopCity != pHeadSelectedCity))
		{
			pSelectCity = pLoopCity;
		}
	}
	else
	{
		pSelectCity = GET_PLAYER(getActivePlayer()).firstCity(&iLoop, !bForward);
	}

	if (pSelectCity != NULL)
	{
		if (bAdd)
		{
			gDLL->getInterfaceIFace()->clearSelectedCities();
			gDLL->getInterfaceIFace()->addSelectedCity(pSelectCity);
		}
		else
		{
			gDLL->getInterfaceIFace()->selectCity(pSelectCity);
		}
	}
}


void CvGame::cycleSelectionGroups(bool bClear, bool bForward, bool bWorkers)
{
	CvSelectionGroup* pNextSelectionGroup;
	CvPlot* pPlot;
	CvUnit* pCycleUnit;
	bool bWrap;

	pCycleUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();

	if (pCycleUnit != NULL)
	{
		if (pCycleUnit->getOwnerINLINE() != getActivePlayer())
		{
			pCycleUnit = NULL;
		}

		pNextSelectionGroup = GET_PLAYER(getActivePlayer()).cycleSelectionGroups(pCycleUnit, bForward, bWorkers, &bWrap);

		if (bWrap)
		{
			if (GET_PLAYER(getActivePlayer()).hasAutoUnit())
			{
				gDLL->sendAutoMoves();
			}
		}
	}
	else
	{
		pPlot = gDLL->getInterfaceIFace()->getLookAtPlot();
		pNextSelectionGroup = GC.getMapINLINE().findSelectionGroup(((pPlot != NULL) ? pPlot->getX() : 0), ((pPlot != NULL) ? pPlot->getY() : 0), getActivePlayer(), true, bWorkers);
	}

	if (pNextSelectionGroup != NULL)
	{
		FAssert(pNextSelectionGroup->getOwnerINLINE() == getActivePlayer());
		gDLL->getInterfaceIFace()->selectUnit(pNextSelectionGroup->getHeadUnit(), bClear);
	}

	if ((pCycleUnit != gDLL->getInterfaceIFace()->getHeadSelectedUnit()) || ((pCycleUnit != NULL) && pCycleUnit->getGroup()->readyToSelect()))
	{
		gDLL->getInterfaceIFace()->lookAtSelectionPlot();
	}
}


// Returns true if unit was cycled...
bool CvGame::cyclePlotUnits(CvPlot* pPlot, bool bForward, bool bAuto, int iCount)
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pSelectedUnit;
	CvUnit* pLoopUnit = NULL;

	FAssertMsg(iCount >= -1, "iCount expected to be >= -1");

	if (iCount == -1)
	{
		pUnitNode = pPlot->headUnitNode();

		while (pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);

			if (pLoopUnit->IsSelected())
			{
				break;
			}

			pUnitNode = pPlot->nextUnitNode(pUnitNode);
		}
	}
	else
	{
		pUnitNode = pPlot->headUnitNode();

		while (pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);

			if ((iCount - 1) == 0)
			{
				break;
			}

			if (iCount > 0)
			{
				iCount--;
			}

			pUnitNode = pPlot->nextUnitNode(pUnitNode);
		}

		if (pUnitNode == NULL)
		{
			pUnitNode = pPlot->tailUnitNode();

			if (pUnitNode != NULL)
			{
				pLoopUnit = ::getUnit(pUnitNode->m_data);
			}
		}
	}

	if (pUnitNode != NULL)
	{
		pSelectedUnit = pLoopUnit;

		while (true)
		{
			if (bForward)
			{
				pUnitNode = pPlot->nextUnitNode(pUnitNode);
				if (pUnitNode == NULL)
				{
					pUnitNode = pPlot->headUnitNode();
				}
			}
			else
			{
				pUnitNode = pPlot->prevUnitNode(pUnitNode);
				if (pUnitNode == NULL)
				{
					pUnitNode = pPlot->tailUnitNode();
				}
			}

			pLoopUnit = ::getUnit(pUnitNode->m_data);

			if (iCount == -1)
			{
				if (pLoopUnit == pSelectedUnit)
				{
					break;
				}
			}

			if (pLoopUnit->getOwnerINLINE() == getActivePlayer())
			{
				if (bAuto)
				{
					if (pLoopUnit->getGroup()->readyToSelect())
					{
						gDLL->getInterfaceIFace()->selectUnit(pLoopUnit, true);
						return true;
					}
				}
				else
				{
					gDLL->getInterfaceIFace()->insertIntoSelectionList(pLoopUnit, true, false);
					return true;
				}
			}

			if (pLoopUnit == pSelectedUnit)
			{
				break;
			}
		}
	}

	return false;
}


void CvGame::selectionListMove(CvPlot* pPlot, bool bAlt, bool bShift, bool bCtrl)
{
	CLLNode<IDInfo>* pSelectedUnitNode;
	CvUnit* pHeadSelectedUnit;
	CvUnit* pSelectedUnit;
	TeamTypes eRivalTeam;

	if (pPlot == NULL)
	{
		return;
	}

	CyPlot* pyPlot = new CyPlot(pPlot);
	CyArgsList argsList;
	argsList.add(gDLL->getPythonIFace()->makePythonObject(pyPlot));	// pass in plot class
	argsList.add(bAlt);
	argsList.add(bShift);
	argsList.add(bCtrl);
	long lResult=0;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "cannotSelectionListMove", argsList.makeFunctionArgs(), &lResult);
	delete pyPlot;	// python fxn must not hold on to this pointer 
	if (lResult == 1)
	{
		return;
	}

	pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();

	if ((pHeadSelectedUnit == NULL) || (pHeadSelectedUnit->getOwnerINLINE() != getActivePlayer()))
	{
		return;
	}

	if (bAlt)
	{
		gDLL->getInterfaceIFace()->selectGroup(pHeadSelectedUnit, false, false, true);
	}
	else if (bCtrl)
	{
		gDLL->getInterfaceIFace()->selectGroup(pHeadSelectedUnit, false, true, false);
	}

	pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

	while (pSelectedUnitNode != NULL)
	{
		pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

		eRivalTeam = pSelectedUnit->getDeclareWarMove(pPlot);

		if (eRivalTeam != NO_TEAM)
		{
			CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_DECLAREWARMOVE);
			if (NULL != pInfo)
			{
				pInfo->setData1(eRivalTeam);
				pInfo->setData2(pPlot->getX());
				pInfo->setData3(pPlot->getY());
				pInfo->setOption1(bShift);
				pInfo->setOption2(pPlot->getTeam() != eRivalTeam);
				gDLL->getInterfaceIFace()->addPopup(pInfo);
			}
			return;
		}

		pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
	}

	selectionListGameNetMessage(GAMEMESSAGE_PUSH_MISSION, MISSION_MOVE_TO, pPlot->getX(), pPlot->getY(), 0, false, bShift);
}


void CvGame::selectionListGameNetMessage(int eMessage, int iData2, int iData3, int iData4, int iFlags, bool bAlt, bool bShift)
{
	CLLNode<IDInfo>* pSelectedUnitNode;
	CvUnit* pHeadSelectedUnit;
	CvUnit* pSelectedUnit;

	CyArgsList argsList;
	argsList.add(eMessage);	// pass in plot class
	argsList.add(iData2);
	argsList.add(iData3);
	argsList.add(iData4);
	argsList.add(iFlags);
	argsList.add(bAlt);
	argsList.add(bShift);
	long lResult=0;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "cannotSelectionListGameNetMessage", argsList.makeFunctionArgs(), &lResult);
	if (lResult == 1)
	{
		return;
	}

	pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();

	if (pHeadSelectedUnit != NULL)
	{
		if (pHeadSelectedUnit->getOwnerINLINE() == getActivePlayer())
		{
			if (eMessage == GAMEMESSAGE_JOIN_GROUP)
			{
				pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

				while (pSelectedUnitNode != NULL)
				{
					pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);
					pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);

					if (bShift)
					{
						gDLL->sendJoinGroup(pSelectedUnit->getID(), FFreeList::INVALID_INDEX);
					}
					else
					{
						if (pSelectedUnit == pHeadSelectedUnit)
						{
							gDLL->sendJoinGroup(pSelectedUnit->getID(), FFreeList::INVALID_INDEX);
						}

						gDLL->sendJoinGroup(pSelectedUnit->getID(), pHeadSelectedUnit->getID());
					}
				}

				if (bShift)
				{
					gDLL->getInterfaceIFace()->selectUnit(pHeadSelectedUnit, true);
				}
			}
			else if (eMessage == GAMEMESSAGE_DO_COMMAND)
			{
				pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

				while (pSelectedUnitNode != NULL)
				{
					pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);
					pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);

					gDLL->sendDoCommand(pSelectedUnit->getID(), ((CommandTypes)iData2), iData3, iData4, bAlt);
				}
			}
			else if ((eMessage == GAMEMESSAGE_PUSH_MISSION) || (eMessage == GAMEMESSAGE_AUTO_MISSION))
			{
				if (!(gDLL->getInterfaceIFace()->mirrorsSelectionGroup()))
				{
					selectionListGameNetMessage(GAMEMESSAGE_JOIN_GROUP);
				}

				if (eMessage == GAMEMESSAGE_PUSH_MISSION)
				{
					gDLL->sendPushMission(pHeadSelectedUnit->getID(), ((MissionTypes)iData2), iData3, iData4, iFlags, bShift);
				}
				else
				{
					gDLL->sendAutoMission(pHeadSelectedUnit->getID());
				}
			}
			else
			{
				FAssert(false);
			}
		}
	}
}


void CvGame::selectedCitiesGameNetMessage(int eMessage, int iData2, int iData3, int iData4, bool bOption, bool bAlt, bool bShift, bool bCtrl)
{
	CLLNode<IDInfo>* pSelectedCityNode;
	CvCity* pSelectedCity;

	pSelectedCityNode = gDLL->getInterfaceIFace()->headSelectedCitiesNode();

	while (pSelectedCityNode != NULL)
	{
		pSelectedCity = ::getCity(pSelectedCityNode->m_data);
		pSelectedCityNode = gDLL->getInterfaceIFace()->nextSelectedCitiesNode(pSelectedCityNode);

		if (pSelectedCity != NULL)
		{
			if (pSelectedCity->getOwnerINLINE() == getActivePlayer())
			{
				switch (eMessage)
				{
				case GAMEMESSAGE_PUSH_ORDER:
					cityPushOrder(pSelectedCity, ((OrderTypes)iData2), iData3, bAlt, bShift, bCtrl);
					break;

				case GAMEMESSAGE_POP_ORDER:
					if (pSelectedCity->getOrderQueueLength() > 1)
					{
						gDLL->sendPopOrder(pSelectedCity->getID(), iData2);
					}
					break;

				case GAMEMESSAGE_DO_TASK:
					gDLL->sendDoTask(pSelectedCity->getID(), ((TaskTypes)iData2), iData3, iData4, bOption, bAlt, bShift, bCtrl);
					break;

				default:
					FAssert(false);
					break;
				}
			}
		}
	}
}


bool CvGame::canHandleAction(int iAction, CvPlot* pPlot, bool bTestVisible, bool bUseCache)
{
	PROFILE_FUNC();

	CvSelectionGroup* pSelectedGroup;
	CvUnit* pHeadSelectedUnit;
	CvPlot* pMissionPlot;
	bool bShift = gDLL->shiftKey();

	if(GC.getUSE_CANNOT_HANDLE_ACTION_CALLBACK())
	{
		CyPlot* pyPlot = new CyPlot(pPlot);
		CyArgsList argsList;
		argsList.add(gDLL->getPythonIFace()->makePythonObject(pyPlot));	// pass in plot class
		argsList.add(iAction);
		argsList.add(bTestVisible);
		long lResult=0;
		gDLL->getPythonIFace()->callFunction(PYGameModule, "cannotHandleAction", argsList.makeFunctionArgs(), &lResult);
		delete pyPlot;	// python fxn must not hold on to this pointer 
		if (lResult == 1)
		{
			return false;
		}
	}

	if (GC.getActionInfo(iAction).getControlType() != NO_CONTROL)
	{
		if (canDoControl((ControlTypes)(GC.getActionInfo(iAction).getControlType())))
		{
			return true;
		}
	}

	if (gDLL->getInterfaceIFace()->isCitySelection())
	{
		return false; // XXX hack!
	}

	pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();

	if (pHeadSelectedUnit != NULL)
	{
		if (pHeadSelectedUnit->getOwnerINLINE() == getActivePlayer())
		{
			if (isMPOption(MPOPTION_SIMULTANEOUS_TURNS) || GET_PLAYER(pHeadSelectedUnit->getOwnerINLINE()).isTurnActive())
			{
				CvSelectionGroup* pSelectedInterfaceList = gDLL->getInterfaceIFace()->getSelectionList();

				if (GC.getActionInfo(iAction).getMissionType() != NO_MISSION)
				{
					if (gDLL->getInterfaceIFace()->mirrorsSelectionGroup())
					{
						pSelectedGroup = pHeadSelectedUnit->getGroup();

						if (pPlot != NULL)
						{
							pMissionPlot = pPlot;
						}
						else if (bShift)
						{
							pMissionPlot = pSelectedGroup->lastMissionPlot();
						}
						else
						{
							pMissionPlot = NULL;
						}

						if ((pMissionPlot == NULL) || !(pMissionPlot->isVisible(pHeadSelectedUnit->getTeam(), false)))
						{
							pMissionPlot = pSelectedGroup->plot();
						}

					}
					else
					{
						pMissionPlot = pSelectedInterfaceList->plot();
					}

					if (pSelectedInterfaceList->canStartMission(GC.getActionInfo(iAction).getMissionType(), GC.getActionInfo(iAction).getMissionData(), -1, pMissionPlot, bTestVisible, bUseCache))
					{
						return true;
					}
				}

				if (GC.getActionInfo(iAction).getCommandType() != NO_COMMAND)
				{
					if (pSelectedInterfaceList->canDoCommand(((CommandTypes)(GC.getActionInfo(iAction).getCommandType())), GC.getActionInfo(iAction).getCommandData(), -1, bTestVisible, bUseCache))
					{
						return true;
					}
				}

				if (gDLL->getInterfaceIFace()->canDoInterfaceMode(((InterfaceModeTypes)GC.getActionInfo(iAction).getInterfaceModeType()), pSelectedInterfaceList))
				{
					return true;
				}
			}
		}
	}

	return false;
}

void CvGame::setupActionCache()
{
	gDLL->getInterfaceIFace()->getSelectionList()->setupActionCache();
}

void CvGame::handleAction(int iAction)
{
	CvUnit* pHeadSelectedUnit;
	bool bAlt;
	bool bShift;
	bool bSkip;

	bAlt = gDLL->altKey();
	bShift = gDLL->shiftKey();

	if (!(gDLL->getInterfaceIFace()->canHandleAction(iAction)))
	{
		return;
	}

	if (GC.getActionInfo(iAction).getControlType() != NO_CONTROL)
	{
		doControl((ControlTypes)(GC.getActionInfo(iAction).getControlType()));
	}

	if (gDLL->getInterfaceIFace()->canDoInterfaceMode((InterfaceModeTypes)GC.getActionInfo(iAction).getInterfaceModeType(), gDLL->getInterfaceIFace()->getSelectionList()))
	{
		pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();

		if (pHeadSelectedUnit != NULL)
		{
			if (GC.getInterfaceModeInfo((InterfaceModeTypes)GC.getActionInfo(iAction).getInterfaceModeType()).getSelectAll())
			{
				gDLL->getInterfaceIFace()->selectGroup(pHeadSelectedUnit, false, false, true);
			}
			else if (GC.getInterfaceModeInfo((InterfaceModeTypes)GC.getActionInfo(iAction).getInterfaceModeType()).getSelectType())
			{
				gDLL->getInterfaceIFace()->selectGroup(pHeadSelectedUnit, false, true, false);
			}
		}

		gDLL->getInterfaceIFace()->setInterfaceMode((InterfaceModeTypes)GC.getActionInfo(iAction).getInterfaceModeType());
	}

	if (GC.getActionInfo(iAction).getMissionType() != NO_MISSION)
	{
		selectionListGameNetMessage(GAMEMESSAGE_PUSH_MISSION, GC.getActionInfo(iAction).getMissionType(), GC.getActionInfo(iAction).getMissionData(), -1, 0, false, bShift);
	}

	if (GC.getActionInfo(iAction).getCommandType() != NO_COMMAND)
	{
		bSkip = false;

		if (GC.getActionInfo(iAction).getCommandType() == COMMAND_LOAD)
		{
			CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_LOADUNIT);
			if (NULL != pInfo)
			{
				gDLL->getInterfaceIFace()->addPopup(pInfo);
				bSkip = true;
			}
		}

		if (!bSkip)
		{
			if (GC.getActionInfo(iAction).isConfirmCommand())
			{
				CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_CONFIRMCOMMAND);
				if (NULL != pInfo)
				{
					pInfo->setData1(iAction);
					pInfo->setOption1(bAlt);
					gDLL->getInterfaceIFace()->addPopup(pInfo);
				}
			}
			else
			{
				selectionListGameNetMessage(GAMEMESSAGE_DO_COMMAND, GC.getActionInfo(iAction).getCommandType(), GC.getActionInfo(iAction).getCommandData(), -1, 0, bAlt);
			}
		}
	}
}


bool CvGame::canDoControl(ControlTypes eControl)
{
	CyArgsList argsList;
	argsList.add(eControl);
	long lResult=0;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "cannotDoControl", argsList.makeFunctionArgs(), &lResult);
	if (lResult == 1)
	{
		return false;
	}

	switch (eControl)
	{
	case CONTROL_SELECTYUNITTYPE:
	case CONTROL_SELECTYUNITALL:
	case CONTROL_SELECT_HEALTHY:
	case CONTROL_SELECTCITY:
	case CONTROL_SELECTCAPITAL:
	case CONTROL_NEXTUNIT:
	case CONTROL_PREVUNIT:
	case CONTROL_CYCLEUNIT:
	case CONTROL_CYCLEUNIT_ALT:
	case CONTROL_CYCLEWORKER:
	case CONTROL_LASTUNIT:
	case CONTROL_AUTOMOVES:
	case CONTROL_SAVE_GROUP:
	case CONTROL_QUICK_SAVE:
	case CONTROL_QUICK_LOAD:
	case CONTROL_ORTHO_CAMERA:
	case CONTROL_CYCLE_CAMERA_FLYING_MODES:
	case CONTROL_ISOMETRIC_CAMERA_LEFT:
	case CONTROL_ISOMETRIC_CAMERA_RIGHT:
	case CONTROL_FLYING_CAMERA:
	case CONTROL_MOUSE_FLYING_CAMERA:
	case CONTROL_TOP_DOWN_CAMERA:
	case CONTROL_TURN_LOG:
	case CONTROL_CHAT_ALL:
	case CONTROL_CHAT_TEAM:
	case CONTROL_GLOBE_VIEW:
		if (!gDLL->getInterfaceIFace()->isFocused())
		{
			return true;
		}
		break;

	case CONTROL_FORCEENDTURN:
		if (!gDLL->getInterfaceIFace()->isFocused() && !gDLL->getInterfaceIFace()->isInAdvancedStart())
		{
			return true;
		}
		break;


	case CONTROL_PING:
	case CONTROL_SIGN:
	case CONTROL_GRID:
	case CONTROL_BARE_MAP:
	case CONTROL_YIELDS:
	case CONTROL_RESOURCE_ALL:
	case CONTROL_UNIT_ICONS:
	case CONTROL_GLOBELAYER:
	case CONTROL_SCORES:
	case CONTROL_FREE_COLONY:
		if (!gDLL->getInterfaceIFace()->isFocusedWidget())
		{
			return true;
		}
		break;

	case CONTROL_OPTIONS_SCREEN:
	case CONTROL_DOMESTIC_SCREEN:
	case CONTROL_VICTORY_SCREEN:
	case CONTROL_CIVILOPEDIA:
	case CONTROL_RELIGION_SCREEN:
	case CONTROL_CORPORATION_SCREEN:
	case CONTROL_CIVICS_SCREEN:
	case CONTROL_FOREIGN_SCREEN:
	case CONTROL_FINANCIAL_SCREEN:
	case CONTROL_MILITARY_SCREEN:
	case CONTROL_TECH_CHOOSER:
	case CONTROL_DIPLOMACY:
	case CONTROL_HALL_OF_FAME:
	case CONTROL_INFO:
	case CONTROL_DETAILS:
	case CONTROL_SAVE_NORMAL:
		return true;
		break;
	case CONTROL_ESPIONAGE_SCREEN:
		if (!isOption(GAMEOPTION_NO_ESPIONAGE))
		{
			return true;
		}
		break;

	case CONTROL_NEXTCITY:
	case CONTROL_PREVCITY:
		if (!gDLL->getInterfaceIFace()->isSpaceshipScreenUp())
		{
			return true;
		}
		break;

	case CONTROL_ADMIN_DETAILS:
		return true;
		break;

	case CONTROL_CENTERONSELECTION:
		if (gDLL->getInterfaceIFace()->getLookAtPlot() != gDLL->getInterfaceIFace()->getSelectionPlot())
		{
			return true;
		}
		break;

	case CONTROL_LOAD_GAME:
		if (!(isNetworkMultiPlayer()))
		{
			return true;
		}
		break;

	case CONTROL_RETIRE:
		if ((getGameState() == GAMESTATE_ON) || isGameMultiPlayer())
		{
			if (GET_PLAYER(getActivePlayer()).isAlive())
			{
				if (isPbem() || isHotSeat())
				{
					if (!GET_PLAYER(getActivePlayer()).isEndTurn())
					{
						return true;
					}
				}
				else
				{
					return true;
				}
			}
		}
		break;

	case CONTROL_WORLD_BUILDER:
		if (!(isGameMultiPlayer()) && GC.getInitCore().getAdminPassword().empty() && !gDLL->getInterfaceIFace()->isInAdvancedStart())
		{
			return true;
		}
		break;

	case CONTROL_ENDTURN:
	case CONTROL_ENDTURN_ALT:
		if (gDLL->getInterfaceIFace()->isEndTurnMessage() && !gDLL->getInterfaceIFace()->isFocused() && !gDLL->getInterfaceIFace()->isInAdvancedStart())
		{
			return true;
		}
		break;

	default:
		FAssertMsg(false, "eControl did not match any valid options");
		break;
	}

	return false;
}


void CvGame::doControl(ControlTypes eControl)
{
	CvPopupInfo* pInfo;
	CvCity* pCapitalCity;
	CvUnit* pHeadSelectedUnit;
	CvUnit* pUnit;
	CvPlot* pPlot;

	if (!canDoControl(eControl))
	{
		return;
	}

	switch (eControl)
	{
	case CONTROL_CENTERONSELECTION:
		gDLL->getInterfaceIFace()->lookAtSelectionPlot();
		break;

	case CONTROL_SELECTYUNITTYPE:
		pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();
		if (pHeadSelectedUnit != NULL)
		{
			gDLL->getInterfaceIFace()->selectGroup(pHeadSelectedUnit, false, true, false);
		}
		break;

	case CONTROL_SELECTYUNITALL:
		pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();
		if (pHeadSelectedUnit != NULL)
		{
			gDLL->getInterfaceIFace()->selectGroup(pHeadSelectedUnit, false, false, true);
		}
		break;

	case CONTROL_SELECT_HEALTHY:
		{
			CvUnit* pGroupHead = NULL;
			pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();
			gDLL->getInterfaceIFace()->clearSelectionList();
			if (pHeadSelectedUnit != NULL)
			{
				CvPlot* pPlot = pHeadSelectedUnit->plot();
				std::vector<CvUnit *> plotUnits;
				getPlotUnits(pPlot, plotUnits);
				gDLL->getInterfaceIFace()->selectionListPreChange();
				for (int iI = 0; iI < (int) plotUnits.size(); iI++)
				{
					pUnit = plotUnits[iI];

					if (pUnit->getOwnerINLINE() == getActivePlayer())
					{
						if (!isMPOption(MPOPTION_SIMULTANEOUS_TURNS) || getTurnSlice() - pUnit->getLastMoveTurn() > GC.getDefineINT("MIN_TIMER_UNIT_DOUBLE_MOVES"))
						{
							if (pUnit->isHurt())
							{
								if (pGroupHead != NULL)
								{
									gDLL->sendJoinGroup(pUnit->getID(), pGroupHead->getID());
								}
								else
								{
									pGroupHead = pUnit;
								}

								gDLL->getInterfaceIFace()->insertIntoSelectionList(pUnit, false, false, true, true, true);
							}
						}
					}
				}

				gDLL->getInterfaceIFace()->selectionListPostChange();
			}
		}
		break;

	case CONTROL_SELECTCITY:
		if (gDLL->getInterfaceIFace()->isCityScreenUp())
		{
			cycleCities();
		}
		else
		{
			gDLL->getInterfaceIFace()->selectLookAtCity();
		}
		break;

	case CONTROL_SELECTCAPITAL:
		pCapitalCity = GET_PLAYER(getActivePlayer()).getCapitalCity();
		if (pCapitalCity != NULL)
		{
			gDLL->getInterfaceIFace()->selectCity(pCapitalCity);
		}
		break;

	case CONTROL_NEXTCITY:
		if (gDLL->getInterfaceIFace()->isCitySelection())
		{
			cycleCities(true, !(gDLL->getInterfaceIFace()->isCityScreenUp()));
		}
		else
		{
			gDLL->getInterfaceIFace()->selectLookAtCity(true);
		}
		gDLL->getInterfaceIFace()->lookAtSelectionPlot();
		break;

	case CONTROL_PREVCITY:
		if (gDLL->getInterfaceIFace()->isCitySelection())
		{
			cycleCities(false, !(gDLL->getInterfaceIFace()->isCityScreenUp()));
		}
		else
		{
			gDLL->getInterfaceIFace()->selectLookAtCity(true);
		}
		gDLL->getInterfaceIFace()->lookAtSelectionPlot();
		break;

	case CONTROL_NEXTUNIT:
		pPlot = gDLL->getInterfaceIFace()->getSelectionPlot();
		if (pPlot != NULL)
		{
			cyclePlotUnits(pPlot);
		}
		break;

	case CONTROL_PREVUNIT:
		pPlot = gDLL->getInterfaceIFace()->getSelectionPlot();
		if (pPlot != NULL)
		{
			cyclePlotUnits(pPlot, false);
		}
		break;

	case CONTROL_CYCLEUNIT:
	case CONTROL_CYCLEUNIT_ALT:
		cycleSelectionGroups(true);
		break;

	case CONTROL_CYCLEWORKER:
		cycleSelectionGroups(true, true, true);
		break;

	case CONTROL_LASTUNIT:
		pUnit = gDLL->getInterfaceIFace()->getLastSelectedUnit();

		if (pUnit != NULL)
		{
			gDLL->getInterfaceIFace()->selectUnit(pUnit, true);
			gDLL->getInterfaceIFace()->lookAtSelectionPlot();
		}
		else
		{
			cycleSelectionGroups(true, false);
		}

		gDLL->getInterfaceIFace()->setLastSelectedUnit(NULL);
		break;

	case CONTROL_ENDTURN:
	case CONTROL_ENDTURN_ALT:
		if (gDLL->getInterfaceIFace()->isEndTurnMessage())
		{
			gDLL->sendTurnComplete();
		}
		break;

	case CONTROL_FORCEENDTURN:
		gDLL->sendTurnComplete();
		break;

	case CONTROL_AUTOMOVES:
		gDLL->sendAutoMoves();
		break;

	case CONTROL_PING:
		gDLL->getInterfaceIFace()->setInterfaceMode(INTERFACEMODE_PING);
		break;

	case CONTROL_SIGN:
		gDLL->getInterfaceIFace()->setInterfaceMode(INTERFACEMODE_SIGN);
		break;

	case CONTROL_GRID:
		gDLL->getEngineIFace()->SetGridMode(!(gDLL->getEngineIFace()->GetGridMode()));
		break;

	case CONTROL_BARE_MAP:
		gDLL->getInterfaceIFace()->toggleBareMapMode();
		break;

	case CONTROL_YIELDS:
		gDLL->getInterfaceIFace()->toggleYieldVisibleMode();
		break;

	case CONTROL_RESOURCE_ALL:
		gDLL->getEngineIFace()->toggleResourceLayer();
		break;

	case CONTROL_UNIT_ICONS:
		gDLL->getEngineIFace()->toggleUnitLayer();
		break;

	case CONTROL_GLOBELAYER:
		gDLL->getEngineIFace()->toggleGlobeview();
		break;

	case CONTROL_SCORES:
		gDLL->getInterfaceIFace()->toggleScoresVisible();
		break;

	case CONTROL_LOAD_GAME:
		gDLL->LoadGame();
		break;

	case CONTROL_OPTIONS_SCREEN:
		gDLL->getPythonIFace()->callFunction("CvScreensInterface", "showOptionsScreen");
		break;

	case CONTROL_RETIRE:
		if (!isGameMultiPlayer() || countHumanPlayersAlive() == 1)
		{
			if (gDLL->GetAutorun())
			{
				GC.getInitCore().setSlotStatus(getActivePlayer(), SS_COMPUTER);
			}
			else
			{
				setGameState(GAMESTATE_OVER);
				gDLL->getInterfaceIFace()->setDirty(Soundtrack_DIRTY_BIT, true);
			}
		}
		else
		{
			if (isNetworkMultiPlayer())
			{
				gDLL->sendMPRetire();
				gDLL->getInterfaceIFace()->exitingToMainMenu();
			}
			else
			{
				gDLL->handleRetirement(getActivePlayer());
			}
		}
		break;

	case CONTROL_SAVE_GROUP:
		gDLL->SaveGame(SAVEGAME_GROUP);
		break;

	case CONTROL_SAVE_NORMAL:
		gDLL->SaveGame(SAVEGAME_NORMAL);
		break;

	case CONTROL_QUICK_SAVE:
		if (!(isNetworkMultiPlayer()))	// SP only!
		{
			gDLL->QuickSave();
		}
		break;

	case CONTROL_QUICK_LOAD:
		if (!(isNetworkMultiPlayer()))	// SP only!
		{
			gDLL->QuickLoad();
		}
		break;

	case CONTROL_ORTHO_CAMERA:
		gDLL->getEngineIFace()->SetOrthoCamera(!(gDLL->getEngineIFace()->GetOrthoCamera()));
		break;

	case CONTROL_CYCLE_CAMERA_FLYING_MODES:
		gDLL->getEngineIFace()->CycleFlyingMode(1);
		break;

	case CONTROL_ISOMETRIC_CAMERA_LEFT:
		gDLL->getEngineIFace()->MoveBaseTurnLeft();
		break;

	case CONTROL_ISOMETRIC_CAMERA_RIGHT:
		gDLL->getEngineIFace()->MoveBaseTurnRight();
		break;

	case CONTROL_FLYING_CAMERA:
		gDLL->getEngineIFace()->SetFlying(!(gDLL->getEngineIFace()->GetFlying()));
		break;

	case CONTROL_MOUSE_FLYING_CAMERA:
		gDLL->getEngineIFace()->SetMouseFlying(!(gDLL->getEngineIFace()->GetMouseFlying()));
		break;

	case CONTROL_TOP_DOWN_CAMERA:
		gDLL->getEngineIFace()->SetSatelliteMode(!(gDLL->getEngineIFace()->GetSatelliteMode()));
		break;

	case CONTROL_CIVILOPEDIA:
		gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaShow");
		break;

	case CONTROL_RELIGION_SCREEN:
		gDLL->getPythonIFace()->callFunction(PYScreensModule, "showReligionScreen");
		break;

	case CONTROL_CORPORATION_SCREEN:
		gDLL->getPythonIFace()->callFunction(PYScreensModule, "showCorporationScreen");
		break;

	case CONTROL_CIVICS_SCREEN:
		gDLL->getPythonIFace()->callFunction(PYScreensModule, "showCivicsScreen");
		break;

	case CONTROL_FOREIGN_SCREEN:
		{
			CyArgsList argsList;
			argsList.add(-1);
			gDLL->getPythonIFace()->callFunction(PYScreensModule, "showForeignAdvisorScreen", argsList.makeFunctionArgs());
		}
		break;

	case CONTROL_FINANCIAL_SCREEN:
		gDLL->getPythonIFace()->callFunction(PYScreensModule, "showFinanceAdvisor");
		break;

	case CONTROL_MILITARY_SCREEN:
		gDLL->getPythonIFace()->callFunction(PYScreensModule, "showMilitaryAdvisor");
		break;

	case CONTROL_TECH_CHOOSER:
		gDLL->getPythonIFace()->callFunction(PYScreensModule, "showTechChooser");
		break;

	case CONTROL_TURN_LOG:
		if (!gDLL->GetWorldBuilderMode() || gDLL->getInterfaceIFace()->isInAdvancedStart())
		{
			gDLL->getInterfaceIFace()->toggleTurnLog();
		}
		break;

	case CONTROL_CHAT_ALL:
		if (!gDLL->GetWorldBuilderMode() || gDLL->getInterfaceIFace()->isInAdvancedStart())
		{
			gDLL->getInterfaceIFace()->showTurnLog(CHATTARGET_ALL);
		}
		break;

	case CONTROL_CHAT_TEAM:
		if (!gDLL->GetWorldBuilderMode() || gDLL->getInterfaceIFace()->isInAdvancedStart())
		{
			gDLL->getInterfaceIFace()->showTurnLog(CHATTARGET_TEAM);
		}
		break;

	case CONTROL_DOMESTIC_SCREEN:
		{
			CyArgsList argsList;
			argsList.add(-1);
			gDLL->getPythonIFace()->callFunction(PYScreensModule, "showDomesticAdvisor", argsList.makeFunctionArgs());
		}
		break;

	case CONTROL_VICTORY_SCREEN:
		gDLL->getPythonIFace()->callFunction(PYScreensModule, "showVictoryScreen");
		break;

	case CONTROL_INFO:
		{
			CyArgsList args;
			args.add(0);
			args.add(getGameState() == GAMESTATE_ON ? 0 : 1);
			gDLL->getPythonIFace()->callFunction(PYScreensModule, "showInfoScreen", args.makeFunctionArgs());
		}
		break;

	case CONTROL_GLOBE_VIEW:
		gDLL->getEngineIFace()->toggleGlobeview();
		break;

	case CONTROL_DETAILS:
		gDLL->getInterfaceIFace()->showDetails();
		break;

	case CONTROL_ADMIN_DETAILS:
		if (GC.getInitCore().getAdminPassword().empty())
		{
			gDLL->getInterfaceIFace()->showAdminDetails();
		}
		else
		{
			CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_ADMIN_PASSWORD);
			if (NULL != pInfo)
			{
				pInfo->setData1((int)CONTROL_ADMIN_DETAILS);
				gDLL->getInterfaceIFace()->addPopup(pInfo, NO_PLAYER, true);
			}
		}
		break;

	case CONTROL_HALL_OF_FAME:
		{
			CyArgsList args;
			args.add(true);
			gDLL->getPythonIFace()->callFunction(PYScreensModule, "showHallOfFame", args.makeFunctionArgs());
		}
		break;

	case CONTROL_WORLD_BUILDER:
		if (GC.getInitCore().getAdminPassword().empty())
		{
			gDLL->getInterfaceIFace()->setWorldBuilder(!(gDLL->GetWorldBuilderMode()));
		}
		else
		{
			CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_ADMIN_PASSWORD);
			if (NULL != pInfo)
			{
				pInfo->setData1((int)CONTROL_WORLD_BUILDER);
				gDLL->getInterfaceIFace()->addPopup(pInfo, NO_PLAYER, true);
			}
		}
		break;

	case CONTROL_ESPIONAGE_SCREEN:
		gDLL->getPythonIFace()->callFunction(PYScreensModule, "showEspionageAdvisor");
		break;

	case CONTROL_FREE_COLONY:
		{
			CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_FREE_COLONY);
			if (pInfo)
			{
				gDLL->getInterfaceIFace()->addPopup(pInfo);
			}
		}
		break;

	case CONTROL_DIPLOMACY:
		pInfo = new CvPopupInfo(BUTTONPOPUP_DIPLOMACY);
		if (NULL != pInfo)
		{
			gDLL->getInterfaceIFace()->addPopup(pInfo);
		}
		break;

	default:
		FAssertMsg(false, "eControl did not match any valid options");
		break;
	}
}


