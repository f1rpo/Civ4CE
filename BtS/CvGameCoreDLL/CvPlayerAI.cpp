// playerAI.cpp

#include "CvGameCoreDLL.h"
#include "CvPlayerAI.h"
#include "CvRandom.h"
#include "CvGlobals.h"
#include "CvGameCoreUtils.h"
#include "CvMap.h"
#include "CvArea.h"
#include "CvPlot.h"
#include "CvGameAI.h"
#include "CvTeamAI.h"
#include "CvGameCoreUtils.h"
#include "CvDiploParameters.h"
#include "CvInitCore.h"
#include "CyArgsList.h"
#include "CvDLLInterfaceIFaceBase.h"
#include "CvDLLEntityIFaceBase.h"
#include "CvDLLPythonIFaceBase.h"
#include "CvInfos.h"
#include "CvPopupInfo.h"
#include "CvExtraSaveData.h"
#include "FProfiler.h"

#define DANGER_RANGE						(4)
#define GREATER_FOUND_RANGE			(5)
#define CIVIC_CHANGE_DELAY			(25)
#define RELIGION_CHANGE_DELAY		(15)

// for testing only
#ifndef FINAL_RELEASE
#define DEBUG_TECH_CHOICES
//#define DEBUG_TECH_CHOICES_NO_RANDOM
#endif

// our own version number on our extra data
// if we want to reorder what we save, change this number
// adding to the list should be fine
#define PLAYEREXTRADATA_CURRENTVERSION		1
enum PlayerExtraDataIndicies
{
	PLAYEREXTRADATA_VERSION = 0,
	//PLAYEREXTRADATA_UNUSED1,
	//PLAYEREXTRADATA_UNUSED2,
};

// statics

CvPlayerAI* CvPlayerAI::m_aPlayers = NULL;

void CvPlayerAI::initStatics()
{
	m_aPlayers = new CvPlayerAI[MAX_PLAYERS];
	for (int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		m_aPlayers[iI].m_eID = ((PlayerTypes)iI);
	}
}

void CvPlayerAI::freeStatics()
{
	SAFE_DELETE_ARRAY(m_aPlayers);
}

DllExport CvPlayerAI& CvPlayerAI::getPlayerNonInl(PlayerTypes ePlayer)
{
	return getPlayer(ePlayer);
}

// Public Functions...

CvPlayerAI::CvPlayerAI()
{
	m_aiNumTrainAIUnits = new int[NUM_UNITAI_TYPES];
	m_aiNumAIUnits = new int[NUM_UNITAI_TYPES];
	m_aiSameReligionCounter = new int[MAX_PLAYERS];
	m_aiDifferentReligionCounter = new int[MAX_PLAYERS];
	m_aiFavoriteCivicCounter = new int[MAX_PLAYERS];
	m_aiBonusTradeCounter = new int[MAX_PLAYERS];
	m_aiPeacetimeTradeValue = new int[MAX_PLAYERS];
	m_aiPeacetimeGrantValue = new int[MAX_PLAYERS];
	m_aiGoldTradedTo = new int[MAX_PLAYERS];
	m_aiAttitudeExtra = new int[MAX_PLAYERS];

	m_abFirstContact = new bool[MAX_PLAYERS];

	m_aaiContactTimer = new int*[MAX_PLAYERS];
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		m_aaiContactTimer[i] = new int[NUM_CONTACT_TYPES];
	}

	m_aaiMemoryCount = new int*[MAX_PLAYERS];
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		m_aaiMemoryCount[i] = new int[NUM_MEMORY_TYPES];
	}
	
	m_aiAverageYieldMultiplier = new int[NUM_YIELD_TYPES];
	m_aiAverageCommerceMultiplier = new int[NUM_COMMERCE_TYPES];
	m_aiAverageCommerceExchange = new int[NUM_COMMERCE_TYPES];

	AI_reset();
}


CvPlayerAI::~CvPlayerAI()
{
	AI_uninit();

	SAFE_DELETE_ARRAY(m_aiNumTrainAIUnits);
	SAFE_DELETE_ARRAY(m_aiNumAIUnits);
	SAFE_DELETE_ARRAY(m_aiSameReligionCounter);
	SAFE_DELETE_ARRAY(m_aiDifferentReligionCounter);
	SAFE_DELETE_ARRAY(m_aiFavoriteCivicCounter);
	SAFE_DELETE_ARRAY(m_aiBonusTradeCounter);
	SAFE_DELETE_ARRAY(m_aiPeacetimeTradeValue);
	SAFE_DELETE_ARRAY(m_aiPeacetimeGrantValue);
	SAFE_DELETE_ARRAY(m_aiGoldTradedTo);
	SAFE_DELETE_ARRAY(m_aiAttitudeExtra);
	SAFE_DELETE_ARRAY(m_abFirstContact);
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		SAFE_DELETE_ARRAY(m_aaiContactTimer[i]);
	}
	SAFE_DELETE_ARRAY(m_aaiContactTimer);

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		SAFE_DELETE_ARRAY(m_aaiMemoryCount[i]);
	}
	SAFE_DELETE_ARRAY(m_aaiMemoryCount);
	
	SAFE_DELETE_ARRAY(m_aiAverageYieldMultiplier);
	SAFE_DELETE_ARRAY(m_aiAverageCommerceMultiplier);
	SAFE_DELETE_ARRAY(m_aiAverageCommerceExchange);
}


void CvPlayerAI::AI_init()
{
	AI_reset();

	//--------------------------------
	// Init other game data
	if ((GC.getInitCore().getSlotStatus(getID()) == SS_TAKEN) || (GC.getInitCore().getSlotStatus(getID()) == SS_COMPUTER))
	{
		FAssert(getPersonalityType() != NO_LEADER);
		AI_setPeaceWeight(GC.getLeaderHeadInfo(getPersonalityType()).getBasePeaceWeight() + GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getPeaceWeightRand(), "AI Peace Weight"));
		AI_setCivicTimer(((getMaxAnarchyTurns() == 0) ? (GC.getDefineINT("MIN_REVOLUTION_TURNS") * 2) : CIVIC_CHANGE_DELAY) / 2);
	}
}


void CvPlayerAI::AI_uninit()
{
}


void CvPlayerAI::AI_reset()
{
	int iI, iJ;

	AI_uninit();

	m_iPeaceWeight = 0;
	m_iAttackOddsChange = 0;
	m_iCivicTimer = 0;
	m_iReligionTimer = 0;

	for (iI = 0; iI < NUM_UNITAI_TYPES; iI++)
	{
		m_aiNumTrainAIUnits[iI] = 0;
		m_aiNumAIUnits[iI] = 0;
	}

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		m_aiSameReligionCounter[iI] = 0;
		m_aiDifferentReligionCounter[iI] = 0;
		m_aiFavoriteCivicCounter[iI] = 0;
		m_aiBonusTradeCounter[iI] = 0;
		m_aiPeacetimeTradeValue[iI] = 0;
		m_aiPeacetimeGrantValue[iI] = 0;
		m_aiGoldTradedTo[iI] = 0;
		m_aiAttitudeExtra[iI] = 0;
	}

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		m_abFirstContact[iI] = false;
	}

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		for (iJ = 0; iJ < NUM_CONTACT_TYPES; iJ++)
		{
			m_aaiContactTimer[iI][iJ] = 0;
		}
	}

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		for (iJ = 0; iJ < NUM_MEMORY_TYPES; iJ++)
		{
			m_aaiMemoryCount[iI][iJ] = 0;
		}
	}
	
	for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		m_aiAverageYieldMultiplier[iI] = 0;
	}
	for (iI = 0; iI< NUM_COMMERCE_TYPES; iI++)
	{
		m_aiAverageCommerceMultiplier[iI] = 0;
		m_aiAverageCommerceExchange[iI] = 0;
	}
	m_iAverageGreatPeopleMultiplier = 0;
	m_iAveragesCacheTurn = -1;
	
	m_iStrategyHash = 0;
	
	m_bWasFinancialTrouble = false;
	m_iTurnLastProductionDirty = -1;

	m_iUpgradeUnitsCacheTurn = -1;
	m_iUpgradeUnitsCachedExpThreshold = 0;
	m_iUpgradeUnitsCachedGold = 0;
}


int CvPlayerAI::AI_getFlavorValue(FlavorTypes eFlavor)
{
	FAssertMsg((getPersonalityType() >= 0), "getPersonalityType() is less than zero");
	FAssertMsg((eFlavor >= 0), "eFlavor is less than zero");
	return GC.getLeaderHeadInfo(getPersonalityType()).getFlavorValue(eFlavor);
}


void CvPlayerAI::AI_doTurnPre()
{
	PROFILE_FUNC();

	FAssertMsg(getPersonalityType() != NO_LEADER, "getPersonalityType() is not expected to be equal with NO_LEADER");
	FAssertMsg(getLeaderType() != NO_LEADER, "getLeaderType() is not expected to be equal with NO_LEADER");
	FAssertMsg(getCivilizationType() != NO_CIVILIZATION, "getCivilizationType() is not expected to be equal with NO_CIVILIZATION");

	AI_doCounter();

	if (isHuman())
	{
		return;
	}

	AI_doResearch();
	
	AI_doCommerce();

	AI_doMilitary();

	AI_doCivics();

	AI_doReligion();

	AI_doCheckFinancialTrouble();

#ifdef NO_WAY_THIS_DEFINED
	// debugging
	if (false)
	{
		// finish all builds
		if (false)
		{
			int iLoop = 0;
			for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
			{
				pLoopCity->changeProduction(pLoopCity->productionLeft());
			}
		}

		if (false)
		{
			AI_makeProductionDirty();
		}
	}
#endif

	if (isBarbarian())
	{
		return;
	}

	if (isMinorCiv())
	{
		return;
	}
}


void CvPlayerAI::AI_doTurnPost()
{
	PROFILE_FUNC();

	if (isHuman())
	{
		return;
	}

	if (isBarbarian())
	{
		return;
	}

	if (isMinorCiv())
	{
		return;
	}

	AI_doDiplo();
}


void CvPlayerAI::AI_doTurnUnitsPre()
{
	PROFILE_FUNC();

	AI_updateFoundValues();

	if (GC.getGameINLINE().getSorenRandNum(8, "AI Update Area Targets") == 0) // XXX personality???
	{
		AI_updateAreaTargets();
	}

	if (isHuman())
	{
		return;
	}

	if (isBarbarian())
	{
		return;
	}
	
	if (AI_isDoStrategy(AI_STRATEGY_CRUSH))
	{
		AI_convertUnitAITypesForCrush();		
	}
}


void CvPlayerAI::AI_doTurnUnitsPost()
{
	PROFILE_FUNC();

	CvUnit* pLoopUnit;
	CvPlot* pUnitPlot;
	bool bValid;
	int iPass;
	int iLoop;

	if (!isHuman() || isOption(PLAYEROPTION_AUTO_PROMOTION))
	{
		for(pLoopUnit = firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = nextUnit(&iLoop))
		{
			pLoopUnit->AI_promote();
		}
	}

	if (isHuman())
	{
		return;
	}

	CvPlot* pLastUpgradePlot = NULL;
	for (iPass = 0; iPass < 4; iPass++)
	{
		for(pLoopUnit = firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = nextUnit(&iLoop))
		{
			bValid = false;

			switch (iPass)
			{
			case 0:
				if (AI_unitImpassable(pLoopUnit->getUnitType()))
				{
					bValid = true;
				}
				break;
			case 1:
				pUnitPlot = pLoopUnit->plot();
				if (pUnitPlot->isCity())
				{
					if (pUnitPlot->getBestDefender(getID()) == pLoopUnit)
					{
						bValid = true;
						pLastUpgradePlot = pUnitPlot;
					}

					// try to upgrade units which are in danger... but don't get obsessed
					if (!bValid && (pLastUpgradePlot != pUnitPlot) && ((AI_getPlotDanger(pUnitPlot, 1, false)) > 0))
					{
						bValid = true;
						pLastUpgradePlot = pUnitPlot;
					}
				}
				break;
			case 2:
				if (pLoopUnit->cargoSpace() > 0)
				{
					bValid = true;
				}
				break;
			case 3:
				bValid = true;
				break;
			default:
				FAssert(false);
				break;
			}

			if (bValid)
			{
				pLoopUnit->AI_upgrade(); // CAN DELETE UNIT!!!
			}
		}
	}

	if (isBarbarian())
	{
		return;
	}
}


void CvPlayerAI::AI_doPeace()
{
	PROFILE_FUNC();

	CvDiploParameters* pDiplo;
	CvCity* pBestReceiveCity;
	CvCity* pBestGiveCity;
	CvCity* pLoopCity;
	CLinkList<TradeData> ourList;
	CLinkList<TradeData> theirList;
	bool abContacted[MAX_TEAMS];
	TradeData item;
	TechTypes eBestReceiveTech;
	TechTypes eBestGiveTech;
	int iReceiveGold;
	int iGiveGold;
	int iGold;
	int iValue;
	int iBestValue;
	int iOurValue;
	int iTheirValue;
	int iLoop;
	int iI, iJ;

	FAssert(!isHuman());
	FAssert(!isMinorCiv());
	FAssert(!isBarbarian());

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		abContacted[iI] = false;
	}

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (iI != getID())
			{
				if (canContact((PlayerTypes)iI) && AI_isWillingToTalk((PlayerTypes)iI))
				{
					if (!(GET_TEAM(getTeam()).isHuman()) && (GET_PLAYER((PlayerTypes)iI).isHuman() || !(GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).isHuman())))
					{
						if (GET_TEAM(getTeam()).isAtWar(GET_PLAYER((PlayerTypes)iI).getTeam()))
						{
							if (!(GET_PLAYER((PlayerTypes)iI).isHuman()) || (GET_TEAM(getTeam()).getLeaderID() == getID()))
							{
								FAssertMsg(!(GET_PLAYER((PlayerTypes)iI).isBarbarian()), "(GET_PLAYER((PlayerTypes)iI).isBarbarian()) did not return false as expected");
								FAssertMsg(iI != getID(), "iI is not expected to be equal with getID()");
								FAssert(GET_PLAYER((PlayerTypes)iI).getTeam() != getTeam());

								if (GET_TEAM(getTeam()).AI_getAtWarCounter(GET_PLAYER((PlayerTypes)iI).getTeam()) > 10)
								{
									if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_PEACE_TREATY) == 0)
									{
										bool bOffered = false;

										setTradeItem(&item, TRADE_SURRENDER);

										if (canTradeItem((PlayerTypes)iI, item, true))
										{
											ourList.clear();
											theirList.clear();

											ourList.insertAtEnd(item);

											bOffered = true;

											if (GET_PLAYER((PlayerTypes)iI).isHuman())
											{
												if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
												{
													AI_changeContactTimer(((PlayerTypes)iI), CONTACT_PEACE_TREATY, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_PEACE_TREATY));
													pDiplo = new CvDiploParameters(getID());
													FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
													pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_OFFER_PEACE"));
													pDiplo->setAIContact(true);
													pDiplo->setOurOfferList(theirList);
													pDiplo->setTheirOfferList(ourList);
													gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
													abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
												}
											}
											else
											{
												GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
											}
										}

										if (!bOffered)
										{
											if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_PEACE_TREATY), "AI Diplo Peace Treaty") == 0)
											{
												setTradeItem(&item, TRADE_PEACE_TREATY);

												if (canTradeItem(((PlayerTypes)iI), item, true) && GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
												{
													iOurValue = GET_TEAM(getTeam()).AI_endWarVal(GET_PLAYER((PlayerTypes)iI).getTeam());
													iTheirValue = GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).AI_endWarVal(getTeam());

													eBestReceiveTech = NO_TECH;
													eBestGiveTech = NO_TECH;

													iReceiveGold = 0;
													iGiveGold = 0;

													pBestReceiveCity = NULL;
													pBestGiveCity = NULL;

													if (iTheirValue > iOurValue)
													{
														if (iTheirValue > iOurValue)
														{
															iBestValue = 0;

															for (iJ = 0; iJ < GC.getNumTechInfos(); iJ++)
															{
																setTradeItem(&item, TRADE_TECHNOLOGIES, iJ);

																if (GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
																{
																	iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Peace Trading (Tech #1)"));

																	if (iValue > iBestValue)
																	{
																		iBestValue = iValue;
																		eBestReceiveTech = ((TechTypes)iJ);
																	}
																}
															}

															if (eBestReceiveTech != NO_TECH)
															{
																iOurValue += GET_TEAM(getTeam()).AI_techTradeVal(eBestReceiveTech, GET_PLAYER((PlayerTypes)iI).getTeam());
															}
														}

														iGold = min((iTheirValue - iOurValue), GET_PLAYER((PlayerTypes)iI).AI_maxGoldTrade(getID()));

														if (iGold > 0)
														{
															setTradeItem(&item, TRADE_GOLD, iGold);

															if (GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
															{
																iReceiveGold = iGold;
																iOurValue += iGold;
															}
														}

														if (iTheirValue > iOurValue)
														{
															iBestValue = 0;

															for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
															{
																setTradeItem(&item, TRADE_CITIES, pLoopCity->getID());

																if (GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
																{
																	iValue = pLoopCity->plot()->calculateCulturePercent(getID());

																	if (iValue > iBestValue)
																	{
																		iBestValue = iValue;
																		pBestReceiveCity = pLoopCity;
																	}
																}
															}

															if (pBestReceiveCity != NULL)
															{
																iOurValue += AI_cityTradeVal(pBestReceiveCity);
															}
														}
													}
													else if (iOurValue > iTheirValue)
													{
														iBestValue = 0;

														for (iJ = 0; iJ < GC.getNumTechInfos(); iJ++)
														{
															setTradeItem(&item, TRADE_TECHNOLOGIES, iJ);

															if (canTradeItem(((PlayerTypes)iI), item, true))
															{
																if (GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).AI_techTradeVal((TechTypes)iJ, getTeam()) <= (iOurValue - iTheirValue))
																{
																	iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Peace Trading (Tech #2)"));

																	if (iValue > iBestValue)
																	{
																		iBestValue = iValue;
																		eBestGiveTech = ((TechTypes)iJ);
																	}
																}
															}
														}

														if (eBestGiveTech != NO_TECH)
														{
															iTheirValue += GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).AI_techTradeVal(eBestGiveTech, getTeam());
														}

														iGold = min((iOurValue - iTheirValue), AI_maxGoldTrade((PlayerTypes)iI));

														if (iGold > 0)
														{
															setTradeItem(&item, TRADE_GOLD, iGold);

															if (canTradeItem(((PlayerTypes)iI), item, true))
															{
																iGiveGold = iGold;
																iTheirValue += iGold;
															}
														}

														iBestValue = 0;

														for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
														{
															setTradeItem(&item, TRADE_CITIES, pLoopCity->getID());

															if (canTradeItem(((PlayerTypes)iI), item, true))
															{
																if (GET_PLAYER((PlayerTypes)iI).AI_cityTradeVal(pLoopCity) <= (iOurValue - iTheirValue))
																{
																	iValue = pLoopCity->plot()->calculateCulturePercent((PlayerTypes)iI);

																	if (iValue > iBestValue)
																	{
																		iBestValue = iValue;
																		pBestGiveCity = pLoopCity;
																	}
																}
															}
														}

														if (pBestGiveCity != NULL)
														{
															iTheirValue += GET_PLAYER((PlayerTypes)iI).AI_cityTradeVal(pBestGiveCity);
														}
													}

													if ((GET_PLAYER((PlayerTypes)iI).isHuman()) ? (iOurValue >= iTheirValue) : ((iOurValue > ((iTheirValue * 3) / 5)) && (iTheirValue > ((iOurValue * 3) / 5))))
													{
														ourList.clear();
														theirList.clear();

														setTradeItem(&item, TRADE_PEACE_TREATY);

														ourList.insertAtEnd(item);
														theirList.insertAtEnd(item);

														if (eBestGiveTech != NO_TECH)
														{
															setTradeItem(&item, TRADE_TECHNOLOGIES, eBestGiveTech);
															ourList.insertAtEnd(item);
														}

														if (eBestReceiveTech != NO_TECH)
														{
															setTradeItem(&item, TRADE_TECHNOLOGIES, eBestReceiveTech);
															theirList.insertAtEnd(item);
														}

														if (iGiveGold != 0)
														{
															setTradeItem(&item, TRADE_GOLD, iGiveGold);
															ourList.insertAtEnd(item);
														}

														if (iReceiveGold != 0)
														{
															setTradeItem(&item, TRADE_GOLD, iReceiveGold);
															theirList.insertAtEnd(item);
														}

														if (pBestGiveCity != NULL)
														{
															setTradeItem(&item, TRADE_CITIES, pBestGiveCity->getID());
															ourList.insertAtEnd(item);
														}

														if (pBestReceiveCity != NULL)
														{
															setTradeItem(&item, TRADE_CITIES, pBestReceiveCity->getID());
															theirList.insertAtEnd(item);
														}

														if (GET_PLAYER((PlayerTypes)iI).isHuman())
														{
															if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
															{
																AI_changeContactTimer(((PlayerTypes)iI), CONTACT_PEACE_TREATY, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_PEACE_TREATY));
																pDiplo = new CvDiploParameters(getID());
																FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
																pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_OFFER_PEACE"));
																pDiplo->setAIContact(true);
																pDiplo->setOurOfferList(theirList);
																pDiplo->setTheirOfferList(ourList);
																gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
																abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
															}
														}
														else
														{
															GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
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
		}
	}
}


void CvPlayerAI::AI_updateFoundValues(bool bStartingLoc)
{
	PROFILE_FUNC();

	CvArea* pLoopArea;
	CvPlot* pLoopPlot;
	int iValue;
	int iLoop;
	int iI;

	for(pLoopArea = GC.getMapINLINE().firstArea(&iLoop); pLoopArea != NULL; pLoopArea = GC.getMapINLINE().nextArea(&iLoop))
	{
		pLoopArea->setBestFoundValue(getID(), 0);
	}

	if (bStartingLoc)
	{
		for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
		{
			GC.getMapINLINE().plotByIndexINLINE(iI)->setFoundValue(getID(), -1);
		}
	}
	else
	{
		for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
		{
			pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

			if (pLoopPlot->isRevealed(getTeam(), false) || AI_isPrimaryArea(pLoopPlot->area()))
			{
				iValue = AI_foundValue(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE());

				pLoopPlot->setFoundValue(getID(), iValue);

				if (iValue > pLoopPlot->area()->getBestFoundValue(getID()))
				{
					pLoopPlot->area()->setBestFoundValue(getID(), iValue);
				}
			}
		}
	}
}


void CvPlayerAI::AI_updateAreaTargets()
{
	CvArea* pLoopArea;
	int iLoop;

	for(pLoopArea = GC.getMapINLINE().firstArea(&iLoop); pLoopArea != NULL; pLoopArea = GC.getMapINLINE().nextArea(&iLoop))
	{
		if (!(pLoopArea->isWater()))
		{
			if (GC.getGameINLINE().getSorenRandNum(3, "AI Target City") == 0)
			{
				pLoopArea->setTargetCity(getID(), NULL);
			}
			else
			{
				pLoopArea->setTargetCity(getID(), AI_findTargetCity(pLoopArea));
			}
		}
	}
}


// Returns priority for unit movement (lower values move first...)
int CvPlayerAI::AI_movementPriority(CvSelectionGroup* pGroup)
{
	CvUnit* pHeadUnit;
	int iCurrCombat;
	int iBestCombat;

	pHeadUnit = pGroup->getHeadUnit();

	if (pHeadUnit != NULL)
	{
		if (pHeadUnit->hasCargo())
		{
			if (pHeadUnit->specialCargo() == NO_SPECIALUNIT)
			{
				return 0;
			}
			else
			{
				return 1;
			}
		}

		if (pHeadUnit->getDomainType() == DOMAIN_AIR)
		{
			return 2;
		}

		if ((pHeadUnit->AI_getUnitAIType() == UNITAI_WORKER) || (pHeadUnit->AI_getUnitAIType() == UNITAI_WORKER_SEA))
		{
			return 3;
		}

		if ((pHeadUnit->AI_getUnitAIType() == UNITAI_EXPLORE) || (pHeadUnit->AI_getUnitAIType() == UNITAI_EXPLORE_SEA))
		{
			return 4;
		}

		if (pHeadUnit->bombardRate() > 0)
		{
			return 5;
		}

		if (pHeadUnit->collateralDamage() > 0)
		{
			return 6;
		}

		if (pHeadUnit->canFight())
		{
			if (pHeadUnit->withdrawalProbability() > 20)
			{
				return 7;
			}

			if (pHeadUnit->withdrawalProbability() > 0)
			{
				return 8;
			}

			iCurrCombat = pHeadUnit->currCombatStr(NULL, NULL);
			iBestCombat = (GC.getGameINLINE().getBestLandUnitCombat() * 100);

			if (pHeadUnit->noDefensiveBonus())
			{
				iCurrCombat *= 3;
				iCurrCombat /= 2;
			}

			if (pHeadUnit->AI_isCityAIType())
			{
				iCurrCombat /= 2;
			}

			if (iCurrCombat > iBestCombat)
			{
				return 9;
			}
			else if (iCurrCombat > ((iBestCombat * 4) / 5))
			{
				return 10;
			}
			else if (iCurrCombat > ((iBestCombat * 3) / 5))
			{
				return 11;
			}
			else if (iCurrCombat > ((iBestCombat * 2) / 5))
			{
				return 12;
			}
			else if (iCurrCombat > ((iBestCombat * 1) / 5))
			{
				return 13;
			}
			else
			{
				return 14;
			}
		}

		return 15;
	}

	return 16;
}


void CvPlayerAI::AI_unitUpdate()
{
	PROFILE_FUNC();

	CLLNode<int>* pCurrUnitNode;
	CvSelectionGroup* pLoopSelectionGroup;
	CLinkList<int> tempGroupCycle;
	CLinkList<int> finalGroupCycle;
	int iValue;


	if (!hasBusyUnit())
	{
		pCurrUnitNode = headGroupCycleNode();

		while (pCurrUnitNode != NULL)
		{
			pLoopSelectionGroup = getSelectionGroup(pCurrUnitNode->m_data);
			pCurrUnitNode = nextGroupCycleNode(pCurrUnitNode);

			if (pLoopSelectionGroup->AI_isForceSeparate())
			{
				// do not split groups that are in the midst of attacking
				if (pLoopSelectionGroup->isForceUpdate() || !pLoopSelectionGroup->AI_isGroupAttack())
				{
					pLoopSelectionGroup->AI_separate();	// pointers could become invalid...
				}
			}
		}

		if (isHuman())
		{
			pCurrUnitNode = headGroupCycleNode();

			while (pCurrUnitNode != NULL)
			{
				pLoopSelectionGroup = getSelectionGroup(pCurrUnitNode->m_data);
				pCurrUnitNode = nextGroupCycleNode(pCurrUnitNode);

				if (pLoopSelectionGroup->AI_update())
				{
					break; // pointers could become invalid...
				}
			}
		}
		else
		{
			tempGroupCycle.clear();
			finalGroupCycle.clear();

			pCurrUnitNode = headGroupCycleNode();

			while (pCurrUnitNode != NULL)
			{
				tempGroupCycle.insertAtEnd(pCurrUnitNode->m_data);
				pCurrUnitNode = nextGroupCycleNode(pCurrUnitNode);
			}

#ifdef DEBUG_AI_UPDATE_GROUPS
			// debugging, for DEBUGLOG
			int iTempGroupLength = tempGroupCycle.getLength();
#endif
			iValue = 0;

			while (tempGroupCycle.getLength() > 0)
			{
				pCurrUnitNode = tempGroupCycle.head();

				while (pCurrUnitNode != NULL)
				{
					pLoopSelectionGroup = getSelectionGroup(pCurrUnitNode->m_data);
					FAssertMsg(pLoopSelectionGroup != NULL, "selection group node with NULL selection group");
					
					if (AI_movementPriority(pLoopSelectionGroup) <= iValue)
					{
						finalGroupCycle.insertAtEnd(pCurrUnitNode->m_data);
						pCurrUnitNode = tempGroupCycle.deleteNode(pCurrUnitNode);
					}
					else
					{
						pCurrUnitNode = tempGroupCycle.next(pCurrUnitNode);
					}
				}

				iValue++;
			}

#ifdef DEBUG_AI_UPDATE_GROUPS
			// debugging
			if (true)
			{
				CvWString szPlayerName = getName();
				DEBUGLOG("AI_unitUpdate:%S (%d = %d) groups\n", szPlayerName.GetCString(), iTempGroupLength, finalGroupCycle.getLength());
			}
#endif

			pCurrUnitNode = finalGroupCycle.head();
			while (pCurrUnitNode != NULL)
			{
				if (getSelectionGroup(pCurrUnitNode->m_data)->AI_update())
				{
					break; // pointers could become invalid...
				}

				pCurrUnitNode = finalGroupCycle.next(pCurrUnitNode);
			}
		}
	}
}


void CvPlayerAI::AI_makeAssignWorkDirty()
{
	CvCity* pLoopCity;
	int iLoop;

	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		pLoopCity->AI_setAssignWorkDirty(true);
	}
}


void CvPlayerAI::AI_assignWorkingPlots()
{
	CvCity* pLoopCity;
	int iLoop;

	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		pLoopCity->AI_assignWorkingPlots();
	}
}


void CvPlayerAI::AI_updateAssignWork()
{
	CvCity* pLoopCity;
	int iLoop;

	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		pLoopCity->AI_updateAssignWork();
	}
}


void CvPlayerAI::AI_makeProductionDirty()
{
	CvCity* pLoopCity;
	int iLoop;

	FAssertMsg(!isHuman(), "isHuman did not return false as expected");

	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		pLoopCity->AI_setChooseProductionDirty(true);
	}
}


void CvPlayerAI::AI_conquerCity(CvCity* pCity)
{
	CvCity* pNearestCity;
	bool bRaze;
	int iRazeValue;
	int iI;

	if (canRaze(pCity))
	{
	    iRazeValue = 0;
		if (GC.getGameINLINE().getElapsedGameTurns() > 20)
		{
			if (getNumCities() > 4)
			{
				if (!(pCity->isHolyCity()) && !(pCity->hasActiveWorldWonder()))
				{
                    if (pCity->getPreviousOwner() != BARBARIAN_PLAYER)
                    {
                        pNearestCity = GC.getMapINLINE().findCity(pCity->getX_INLINE(), pCity->getY_INLINE(), NO_PLAYER, getTeam(), true, false, NO_TEAM, NO_DIRECTION, pCity);

                        if (pNearestCity == NULL)
                        {
                            if (pCity->getPreviousOwner() != NO_PLAYER)
                            {
                                if (GET_TEAM(GET_PLAYER(pCity->getPreviousOwner()).getTeam()).countNumCitiesByArea(pCity->area()) > 3)
                                {
                                    bRaze = true;
                                    iRazeValue += 30;
                                }
                            }
                        }
                        else
                        {
                            int iDistance = plotDistance(pCity->getX_INLINE(), pCity->getY_INLINE(), pNearestCity->getX_INLINE(), pNearestCity->getY_INLINE());
                            if ( iDistance > 12)
                            {
                                iRazeValue += iDistance * 2;
                            }
                        }
                    }


                    if (pCity->area()->getCitiesPerPlayer(getID()) > 0)
                    {
                        if (AI_isFinancialTrouble())
                        {
                            iRazeValue += (70 - 15 * pCity->getPopulation());
                        }
                    }
                    
                    if (getStateReligion() != NO_RELIGION)
                    {
                        if (pCity->isHasReligion(getStateReligion()))
                        {
                            iRazeValue -= 50;                            
                        }
                    }
                    
                    int iCloseness = pCity->AI_playerCloseness(getID());
                    if (iCloseness > 0)
                    {
                    	iRazeValue -= 25;
                    	iRazeValue -= iCloseness * 2;
                    }
                    else
                    {
                    	iRazeValue += 60;
                    }
                    
                    if (pCity->area()->getCitiesPerPlayer(getID()) > 0)
                    {
                        if (pCity->getPreviousOwner() != BARBARIAN_PLAYER)
                        {
                            iRazeValue += GC.getLeaderHeadInfo(getPersonalityType()).getRazeCityProb();
                        }
                    }
					
					if (iRazeValue > 0)
					{
					    for (iI = 0; iI < GC.getNumTraitInfos(); iI++)
					    {
                            if (hasTrait((TraitTypes)iI))
                            {
                                iRazeValue *= (100 - (GC.getTraitInfo((TraitTypes)iI).getUpkeepModifier()));
                                iRazeValue /= 100;
                            }
					    }

                        if (GC.getGameINLINE().getSorenRandNum(100, "AI Raze City") < iRazeValue)
                        {
                            pCity->doTask(TASK_RAZE);
                        }
					}
				}
			}
		}
	}
}


bool CvPlayerAI::AI_acceptUnit(CvUnit* pUnit)
{
	if (isHuman())
	{
		return true;
	}

	if (AI_isFinancialTrouble())
	{
		return false;
	}

	return true;
}


bool CvPlayerAI::AI_captureUnit(UnitTypes eUnit, CvPlot* pPlot)
{
	CvCity* pNearestCity;

	FAssert(!isHuman());

	if (pPlot->getTeam() == getTeam())
	{
		return true;
	}

	pNearestCity = GC.getMapINLINE().findCity(pPlot->getX_INLINE(), pPlot->getY_INLINE(), NO_PLAYER, getTeam());

	if (pNearestCity != NULL)
	{
		if (plotDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pNearestCity->getX_INLINE(), pNearestCity->getY_INLINE()) <= 4)
		{
			return true;
		}
	}

	return false;
}


DomainTypes CvPlayerAI::AI_unitAIDomainType(UnitAITypes eUnitAI)
{
	switch (eUnitAI)
	{
	case UNITAI_UNKNOWN:
		return NO_DOMAIN;
		break;

	case UNITAI_ANIMAL:
	case UNITAI_SETTLE:
	case UNITAI_WORKER:
	case UNITAI_ATTACK:
	case UNITAI_ATTACK_CITY:
	case UNITAI_COLLATERAL:
	case UNITAI_PILLAGE:
	case UNITAI_RESERVE:
	case UNITAI_COUNTER:
	case UNITAI_CITY_DEFENSE:
	case UNITAI_CITY_COUNTER:
	case UNITAI_CITY_SPECIAL:
	case UNITAI_EXPLORE:
	case UNITAI_MISSIONARY:
	case UNITAI_PROPHET:
	case UNITAI_ARTIST:
	case UNITAI_SCIENTIST:
	case UNITAI_GENERAL:
	case UNITAI_MERCHANT:
	case UNITAI_ENGINEER:
	case UNITAI_SPY:
		return DOMAIN_LAND;
		break;

	case UNITAI_ICBM:
		return DOMAIN_IMMOBILE;
		break;

	case UNITAI_WORKER_SEA:
	case UNITAI_ATTACK_SEA:
	case UNITAI_RESERVE_SEA:
	case UNITAI_ESCORT_SEA:
	case UNITAI_EXPLORE_SEA:
	case UNITAI_ASSAULT_SEA:
	case UNITAI_SETTLER_SEA:
	case UNITAI_MISSIONARY_SEA:
	case UNITAI_SPY_SEA:
	case UNITAI_CARRIER_SEA:
		return DOMAIN_SEA;
		break;

	case UNITAI_ATTACK_AIR:
	case UNITAI_DEFENSE_AIR:
	case UNITAI_CARRIER_AIR:
		return DOMAIN_AIR;
		break;

	default:
		FAssert(false);
		break;
	}

	return NO_DOMAIN;
}


int CvPlayerAI::AI_yieldWeight(YieldTypes eYield)
{
	return GC.getYieldInfo(eYield).getAIWeightPercent();
}


int CvPlayerAI::AI_commerceWeight(CommerceTypes eCommerce, CvCity* pCity)
{
	int iWeight;

	iWeight = GC.getCommerceInfo(eCommerce).getAIWeightPercent();
	
	//sorry but the merchant descrimination must stop.
	iWeight = min(110, iWeight);

	if (eCommerce == COMMERCE_RESEARCH)
	{
		if (AI_avoidScience())
		{
			if (isNoResearchAvailable())
			{
				iWeight = 0;
			}
			else
			{
				iWeight /= 4;
			}
		}
	}
	else if (eCommerce == COMMERCE_GOLD)
	{
		if (getCommercePercent(COMMERCE_GOLD) == 100)
		{
			//avoid strikes
			if (getGoldPerTurn() < 0)
			{
				iWeight += 15;
			}
		}
		else if (getCommercePercent(COMMERCE_GOLD) == 0)
		{
			//put more money towards other commerce types
			if (getGoldPerTurn() > 0)
			{
				iWeight -= 15;
			}
		}
	}
	else if (eCommerce == COMMERCE_CULTURE)
	{
		// COMMERCE_CULTURE AIWeightPercent is 25% in default xml
	
		if (pCity != NULL)
		{
			if (AI_isDoStrategy(AI_STRATEGY_CULTURE3))
			{
				int iCultureRateRank = pCity->findCommerceRateRank(COMMERCE_CULTURE);
				int iCulturalVictoryNumCultureCities = GC.getGameINLINE().culturalVictoryNumCultureCities();
				
				// if one of the currently best cities, then focus hard, *4
				if (iCultureRateRank <= iCulturalVictoryNumCultureCities)
				{
					iWeight *= 4;
				}
				// if one of the 3 close to the top, then still emphasize culture some, *2
				else if (iCultureRateRank <= iCulturalVictoryNumCultureCities + 3)
				{
					iWeight *= 2;
				}
			}
			else if (AI_isDoStrategy(AI_STRATEGY_CULTURE2))
			{
				iWeight *= 3;
			}
			else if (AI_isDoStrategy(AI_STRATEGY_CULTURE1))
			{
				iWeight *= 2;
			}
			
			iWeight += (100 - pCity->plot()->calculateCulturePercent(getID()));
			
			if (pCity->getCultureLevel() <= (CultureLevelTypes) 1)
			{
				iWeight = max(iWeight, 400);				
			}
		}
		// pCity == NULL
		else
		{
			if (AI_isDoStrategy(AI_STRATEGY_CULTURE3))
			{
				iWeight *= 3;
				iWeight /= 4;
			}
			else if (AI_isDoStrategy(AI_STRATEGY_CULTURE2))
			{
				iWeight *= 2;
				iWeight /= 3;
			}
			else if (AI_isDoStrategy(AI_STRATEGY_CULTURE1))
			{
				iWeight /= 2;
			}
			else 
			{
				iWeight /= 3;
			}
		}
	}

	return iWeight;
}

// Improved as per Blake - thanks!
int CvPlayerAI::AI_foundValue(int iX, int iY, int iMinRivalRange, bool bStartingLoc)
{
	PROFILE("CvPlayerAI::AI_foundValue()");

	CvCity* pNearestCity;
	CvArea* pArea;
	CvPlot* pPlot;
	CvPlot* pLoopPlot;
	FeatureTypes eFeature;
	BonusTypes eBonus;
	ImprovementTypes eBonusImprovement;
	bool bHasGoodBonus;
	int iOwnedTiles;
	int iBadTile;
	int iTakenTiles;
	int iTeammateTakenTiles;
	int iDifferentAreaTile;
	int iTeamAreaCities;
	int iHealth;
	int iValue;
	int iTempValue;
	int iRange;
	int iDX, iDY;
	int iI;
	bool bIsCoastal;
	int iResourceValue = 0;
	int iSpecialFood = 0;
	int iSpecialFoodPlus = 0;
	int iSpecialFoodMinus = 0;
	int iSpecialProduction = 0;
	int iSpecialCommerce = 0;

	int iGreed;
	int iNumAreaCities;

	int * paiBonusCount;

	if (!canFound(iX, iY))
	{
		return 0;
	}

	pPlot = GC.getMapINLINE().plotINLINE(iX, iY);
	pArea = pPlot->area();
	iNumAreaCities = pArea->getCitiesPerPlayer(getID());
	bIsCoastal = pPlot->isCoastalLand(GC.getDefineINT("MIN_WATER_SIZE_FOR_OCEAN"));

    paiBonusCount = new int[GC.getNumBonusInfos()];

    for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
    {
        paiBonusCount[iI] = 0;
    }

	if (iMinRivalRange != -1)
	{
		for (iDX = -(iMinRivalRange); iDX <= iMinRivalRange; iDX++)
		{
			for (iDY = -(iMinRivalRange); iDY <= iMinRivalRange; iDY++)
			{
				pLoopPlot	= plotXY(iX, iY, iDX, iDY);

				if (pLoopPlot != NULL)
				{
					if (pLoopPlot->plotCheck(PUF_isOtherTeam, getID()) != NULL)
					{
						return 0;
					}
				}
			}
		}
	}

	if (bStartingLoc)
	{
		if (pPlot->isGoody())
		{
			return 0;
		}

		for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
		{
			pLoopPlot = plotCity(iX, iY, iI);

			if (pLoopPlot == NULL)
			{
				return 0;
			}
		}
	}

	iOwnedTiles = 0;

	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		pLoopPlot = plotCity(iX, iY, iI);

		if (pLoopPlot == NULL)
		{
			iOwnedTiles++;
		}
		else if (pLoopPlot->isOwned())
        {
            if (pLoopPlot->getTeam() != getTeam())
            {
                iOwnedTiles++;
            }
        }
	}

	if (iOwnedTiles > (NUM_CITY_PLOTS / 3))
	{
		return 0;
	}

	iBadTile = 0;

	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		PROFILE("CvPlayerAI::AI_foundValue() 1");

		pLoopPlot = plotCity(iX, iY, iI);

		if (iI != CITY_HOME_PLOT)
		{
			if ((pLoopPlot == NULL) || pLoopPlot->isImpassable())
			{
				iBadTile += 2;
			}
			else if (!(pLoopPlot->isFreshWater()) && !(pLoopPlot->isHills()))
			{
				if ((pLoopPlot->calculateBestNatureYield(YIELD_FOOD, getTeam()) == 0) || (pLoopPlot->calculateTotalBestNatureYield(getTeam()) <= 1))
				{
					iBadTile += 2;
				}
				else if (pLoopPlot->isWater() && !bIsCoastal && (pLoopPlot->calculateBestNatureYield(YIELD_FOOD, getTeam()) <= 1))
				{
					iBadTile++;
				}
			}
            else if (pLoopPlot->isOwned())
            {
                if (pLoopPlot->getTeam() == getTeam())
                {
                    if (pLoopPlot->isCityRadius())
                    {
                        iBadTile++;
                    }
                }
            }
		}
	}

	iBadTile /= 2;

	if (!bStartingLoc)
	{
		if (iBadTile > (NUM_CITY_PLOTS / 2))
		{
			bHasGoodBonus = false;

			for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
			{
				pLoopPlot = plotCity(iX, iY, iI);

				if (pLoopPlot != NULL)
				{
					if (!(pLoopPlot->isOwned()))
					{
						if (pLoopPlot->isWater() || (pLoopPlot->area() == pArea))
						{
							eBonus = pLoopPlot->getBonusType(getTeam());

							if (eBonus != NO_BONUS)
							{
								if ((getNumTradeableBonuses(eBonus) == 0) || (AI_bonusVal(eBonus) > 10))
								{
									bHasGoodBonus = true;
									break;
								}
							}
						}
					}
				}
			}

			if (!bHasGoodBonus)
			{
				return 0;
			}
		}
	}

	iTakenTiles = 0;
	iTeammateTakenTiles = 0;
	iHealth = 0;
	iValue = 1000;

	iGreed = 100;


    if (!bStartingLoc)
    {
        for (iI = 0; iI < GC.getNumTraitInfos(); iI++)
        {
            if (hasTrait((TraitTypes)iI))
            {
                //Greedy founding means getting the best possible sites - fitting maximum
                //resources into the fat cross.
                iGreed += (GC.getTraitInfo((TraitTypes)iI).getUpkeepModifier() / 2);
                iGreed += 20 * (GC.getTraitInfo((TraitTypes)iI).getCommerceChange(COMMERCE_CULTURE));
            }
        }
    }
    //iClaimThreshold is the culture required to pop the 2nd borders.
    int iClaimThreshold = GC.getCultureLevelInfo((CultureLevelTypes)(min(2, (GC.getNumCultureLevelInfos() - 1)))).getSpeedThreshold(GC.getGameINLINE().getGameSpeedType());
    iClaimThreshold = max(1, iClaimThreshold);
    iClaimThreshold *= (max(100, iGreed));

	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		PROFILE("CvPlayerAI::AI_foundValue() 2");

		pLoopPlot = plotCity(iX, iY, iI);

		if (pLoopPlot == NULL)
		{
			iTakenTiles++;
		}
		else if (pLoopPlot->isCityRadius())
		{
			iTakenTiles++;

			if (pLoopPlot->isWithinTeamCityRadius(getTeam(), getID()))
			{
				iTeammateTakenTiles++;
			}
		}
		else
		{
			PROFILE("CvPlayerAI::AI_foundValue() 3");

			iTempValue = 0;

			eFeature = pLoopPlot->getFeatureType();
			eBonus = pLoopPlot->getBonusType((bStartingLoc) ? NO_TEAM : getTeam());
			eBonusImprovement = NO_IMPROVEMENT;
			
			int iCultureMultiplier;
            if (!pLoopPlot->isOwned() || (pLoopPlot->getOwnerINLINE() == getID()))
            {	
                iCultureMultiplier = 100;    
            }
            else
            {
                int iOurCulture = pLoopPlot->getCulture(getID());
                int iOtherCulture = max(1, pLoopPlot->getCulture(pLoopPlot->getOwnerINLINE()));
                iCultureMultiplier = 100 * (iOurCulture + iClaimThreshold);
                iCultureMultiplier /= (iOtherCulture + iClaimThreshold);
                iCultureMultiplier = min(100, iCultureMultiplier);
                //The multiplier is basically normalized...
                //100% means we own (or rightfully own) the tile.
                //50% means the hostile culture is fairly firmly entrenched.
            }
            
            if (iCultureMultiplier < ((iNumAreaCities > 0) ? 25 : 50))
            {
                //discourage hopeless cases, especially on other continents.
                iTakenTiles += (iNumAreaCities > 0) ? 1 : 2;
            }

			if (eBonus != NO_BONUS)
			{
				for (int iImprovement = 0; iImprovement < GC.getNumImprovementInfos(); ++iImprovement)
				{
					CvImprovementInfo& kImprovement = GC.getImprovementInfo((ImprovementTypes)iImprovement);

					if (kImprovement.isImprovementBonusMakesValid(eBonus))
					{
						eBonusImprovement = (ImprovementTypes)iImprovement;
						break;
					}
				}
			}

			int aiYield[NUM_YIELD_TYPES];

			for (int iYieldType = 0; iYieldType < NUM_YIELD_TYPES; ++iYieldType)
			{
				YieldTypes eYield = (YieldTypes)iYieldType;
				aiYield[eYield] = pLoopPlot->getYield(eYield);

				if (iI == CITY_HOME_PLOT)
				{
					aiYield[eYield] += GC.getYieldInfo(eYield).getCityChange();

					if (eFeature != NO_FEATURE)
					{
						aiYield[eYield] -= GC.getFeatureInfo(eFeature).getYieldChange(eYield);
					}

					aiYield[eYield] = max(aiYield[eYield], GC.getYieldInfo(eYield).getMinCity());

					if (eBonusImprovement != NO_IMPROVEMENT)
					{
						aiYield[eYield] -= GC.getImprovementInfo(eBonusImprovement).getImprovementBonusYield(eBonus, eYield);
					}
				}
			}

			if (iI == CITY_HOME_PLOT)
			{
				iTempValue += aiYield[YIELD_FOOD] * 60;
				iTempValue += aiYield[YIELD_PRODUCTION] * 60;
				iTempValue += aiYield[YIELD_COMMERCE] * 40;
			}
			else if (aiYield[YIELD_FOOD] >= GC.getFOOD_CONSUMPTION_PER_POPULATION())
			{
				iTempValue += aiYield[YIELD_FOOD] * 40;
				iTempValue += aiYield[YIELD_PRODUCTION] * 40;
				iTempValue += aiYield[YIELD_COMMERCE] * 30;

				if (bStartingLoc)
				{
					iTempValue *= 2;
				}
			}
			else if (aiYield[YIELD_FOOD] == GC.getFOOD_CONSUMPTION_PER_POPULATION() - 1)
			{
				iTempValue += aiYield[YIELD_FOOD] * 25;
				iTempValue += aiYield[YIELD_PRODUCTION] * 25;
				iTempValue += aiYield[YIELD_COMMERCE] * 20;
			}
			else
			{
				iTempValue += aiYield[YIELD_FOOD] * 15;
				iTempValue += aiYield[YIELD_PRODUCTION] * 15;
				iTempValue += aiYield[YIELD_COMMERCE] * 10;
			}
			
            if (pLoopPlot->isWater())
			{
                if (aiYield[YIELD_COMMERCE] > 1)
                {
                    iTempValue += bIsCoastal ? 30 : -20;
					if (bIsCoastal && (aiYield[YIELD_FOOD] >= GC.getFOOD_CONSUMPTION_PER_POPULATION()))
                    {
                    	iSpecialFoodPlus += 1;                    	
                    }
                    if (bStartingLoc)
                    {
                        // I'm pretty much forbidding starting 1 tile inland non-coastal.
                        // with more than a few coast tiles.
                        iTempValue += bIsCoastal ? 0 : -400;
					}
                }
            }
            
			if (pLoopPlot->isRiver())
			{
				iTempValue += 10;
			}

			if (iI == CITY_HOME_PLOT)
			{
				iTempValue *= 2;
			}
			else if ((pLoopPlot->getOwnerINLINE() == getID()) || (stepDistance(iX, iY, pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE()) == 1))
			{
				iTempValue *= 3;
				iTempValue /= 2;
			}
			else
			{
				iTempValue *= iGreed;
				iTempValue /= 100;
			}
			
			iTempValue *= iCultureMultiplier;
			iTempValue /= 100;
			
			iValue += iTempValue;
			
			if (iCultureMultiplier > 33) //ignore hopelessly entrenched tiles.
			{
                if (eFeature != NO_FEATURE)
                {
                    if (iI != CITY_HOME_PLOT)
                    {
                        iHealth += GC.getFeatureInfo(eFeature).getHealthPercent();

                        iSpecialFoodPlus += max(0, aiYield[YIELD_FOOD] - GC.getFOOD_CONSUMPTION_PER_POPULATION());
                    }
                }

                if (eBonus != NO_BONUS)
                {
                    PROFILE("CvPlayerAI::AI_foundValue() 4");

                    paiBonusCount[eBonus]++;
                    FAssert(paiBonusCount[eBonus] > 0);

                    iTempValue = (AI_bonusVal(eBonus) * ((!bStartingLoc && (getNumTradeableBonuses(eBonus) == 0) && (paiBonusCount[eBonus] == 1)) ? 80 : 20));
                    iTempValue *= ((bStartingLoc) ? 100 : iGreed);
                    iTempValue /= 100;

                    if (iI != CITY_HOME_PLOT && !bStartingLoc)
                    {
                        if ((pLoopPlot->getOwnerINLINE() != getID()) && stepDistance(pPlot->getX_INLINE(),pPlot->getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE()) > 1)
                        {
                            iTempValue *= 2;
                            iTempValue /= 3;

                            iTempValue *= min(150, iGreed);
                            iTempValue /= 100;
                        }
                    }

                    iValue += (iTempValue + 10);

                    if (iI != CITY_HOME_PLOT)
                    {
                        if (eBonusImprovement != NO_IMPROVEMENT)
                        {
                            int iSpecialFoodTemp;
                            iSpecialFoodTemp = pLoopPlot->calculateBestNatureYield(YIELD_FOOD, getTeam()) + GC.getImprovementInfo(eBonusImprovement).getImprovementBonusYield(eBonus, YIELD_FOOD);

                            iSpecialFood += iSpecialFoodTemp;

                            iSpecialFoodTemp -= GC.getFOOD_CONSUMPTION_PER_POPULATION();

                            iSpecialFoodPlus += max(0,iSpecialFoodTemp);
                            iSpecialFoodMinus -= min(0,iSpecialFoodTemp);
                            iSpecialProduction += pLoopPlot->calculateBestNatureYield(YIELD_PRODUCTION, getTeam()) + GC.getImprovementInfo(eBonusImprovement).getImprovementBonusYield(eBonus, YIELD_PRODUCTION);
                            iSpecialCommerce += pLoopPlot->calculateBestNatureYield(YIELD_COMMERCE, getTeam()) + GC.getImprovementInfo(eBonusImprovement).getImprovementBonusYield(eBonus, YIELD_COMMERCE);
                        }

                        if (eFeature != NO_FEATURE)
                        {
                            if (GC.getFeatureInfo(eFeature).getYieldChange(YIELD_FOOD) < 0)
                            {
                                iResourceValue -= 30;
                            }
                        }

                        if (pLoopPlot->isWater())
                        {
                            iValue += (bIsCoastal ? 100 : -800);
                        }
                    }                    
				}
			}
		}
	}
	
    iResourceValue += iSpecialFood * 50;
    iResourceValue += iSpecialProduction * 50;
    iResourceValue += iSpecialCommerce * 50;
    if (bStartingLoc)
    {
        iResourceValue /= 2;
    }

	iValue += max(0, iResourceValue);

	if (iTakenTiles > (NUM_CITY_PLOTS / 3) && iResourceValue < 250)
	{
		return 0;
	}

	if (iTeammateTakenTiles > 1)
	{
		return 0;
	}

	iValue += (iHealth / 5);

	if (bIsCoastal)
	{
		iValue += 400;

		if (!bStartingLoc)
		{
			if (pArea->getCitiesPerPlayer(getID()) == 0)
			{
				iValue += 400;
			}
		}
		else
		{
		    //let other penalties bring this down.
		    iValue += 600;
		    if (!pPlot->isStartingPlot())
		    {
                if (pArea->getNumStartingPlots() == 0)
                {
                    iValue += 1000;                    
                }
			}
		}
	}

	if (pPlot->isHills())
	{
		iValue += 200;
	}

	if (pPlot->isRiver())
	{
		iValue += 40;
	}

	if (pPlot->isFreshWater())
	{
		iValue += 40;
		iValue += (GC.getDefineINT("FRESH_WATER_HEALTH_CHANGE") * 30);
	}

	if (bStartingLoc)
	{
		PROFILE("CvPlayerAI::AI_foundValue() 5");

		iRange = GREATER_FOUND_RANGE;
		int iGreaterBadTile = 0;

		for (iDX = -(iRange); iDX <= iRange; iDX++)
		{
			for (iDY = -(iRange); iDY <= iRange; iDY++)
			{
				pLoopPlot = plotXY(iX, iY, iDX, iDY);

				if (pLoopPlot != NULL)
				{
					if (pLoopPlot->isWater() || (pLoopPlot->area() == pArea))
					{
						if (plotDistance(iX, iY, pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE()) <= iRange)
						{
						    iTempValue = 0;
							iTempValue += (pLoopPlot->getYield(YIELD_FOOD) * 15);
							iTempValue += (pLoopPlot->getYield(YIELD_PRODUCTION) * 11);
							iTempValue += (pLoopPlot->getYield(YIELD_COMMERCE) * 5);
							iValue += iTempValue;
							if (iTempValue < 21)
							{

								iGreaterBadTile += 2;
								if (pLoopPlot->getFeatureType() != NO_FEATURE)
								{
							    	if (pLoopPlot->calculateBestNatureYield(YIELD_FOOD,getTeam()) > 1)
							    	{
										iGreaterBadTile--;
							    	}
								}
							}
						}
					}
				}
			}
		}

		if (!pPlot->isStartingPlot())
		{
			iGreaterBadTile /= 2;
			if (iGreaterBadTile > 12)
			{
				iValue *= 11;
				iValue /= iGreaterBadTile;
			}
		}
		
		int iWaterCount = 0;
		
		for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
		{
		    pLoopPlot = plotCity(iX, iY, iI);
            
            if (pLoopPlot != NULL)
		    {
		        if (pLoopPlot->isWater())
		        {
		            iWaterCount++;
		            if (pLoopPlot->getYield(YIELD_FOOD) <= 1)
		            {
		                iWaterCount++;
		            }
		        }
		    }
		}
		iWaterCount /= 2;
		
		int iLandCount = (NUM_CITY_PLOTS - iWaterCount);
		
		if (iLandCount < (NUM_CITY_PLOTS / 2))
		{
		    //discourage very water-heavy starts.
		    iValue *= 1 + iLandCount;
		    iValue /= (1 + (NUM_CITY_PLOTS / 2));
		}
	}

	if (bStartingLoc)
	{
		if (pPlot->getMinOriginalStartDist() == -1)
		{
			iValue += (GC.getMapINLINE().maxStepDistance() * 100);
		}
		else
		{
			iValue *= (1 + 3 * pPlot->getMinOriginalStartDist());
			iValue /= (1 + 2 * GC.getMapINLINE().maxStepDistance());
		}
		
		//nice hacky way to avoid this messing with normalizer, use elsewhere?
		if (!pPlot->isStartingPlot())
		{
			int iMinDistanceFactor = MAX_INT;
			int iMinRange = startingPlotRange();
			
			for (int iJ = 0; iJ < MAX_CIV_PLAYERS; iJ++)
			{
				if (GET_PLAYER((PlayerTypes)iJ).isAlive())
				{
					if (iJ != getID())
					{
						int iClosenessFactor = GET_PLAYER((PlayerTypes)iJ).startingPlotDistanceFactor(pPlot, getID(), iMinRange);
						iMinDistanceFactor = min(iClosenessFactor, iMinDistanceFactor);
						
//						if (iClosenessFactor < 100)
//						{
//							iValue *= iClosenessFactor;
//							iValue /= 100;
//						}
					}
				}
			}
			
			if (iMinDistanceFactor > 100)
			{
				//give a maximum boost of 25% for somewhat distant locations, don't go overboard.
				iMinDistanceFactor = min(150, iMinDistanceFactor);
				iValue *= (100 + iMinDistanceFactor);				
				iValue /= 200;				
			}
			else if (iMinDistanceFactor < 100)
			{
				//this is too close so penalize quite heavily
				iValue *= iMinDistanceFactor;
				iValue *= iMinDistanceFactor;
				iValue /= 10000;
			}
			
			
            if (pPlot->getBonusType() != NO_BONUS)
            {
                iValue /= 2;
            }
		}
	}

	pNearestCity = GC.getMapINLINE().findCity(iX, iY, ((isBarbarian()) ? NO_PLAYER : getID()));

	if (pNearestCity != NULL)
	{
		if (isBarbarian())
		{
			iValue -= (max(0, (8 - plotDistance(iX, iY, pNearestCity->getX_INLINE(), pNearestCity->getY_INLINE()))) * 200);
		}
		else
		{
		    int iDistance = plotDistance(iX, iY, pNearestCity->getX_INLINE(), pNearestCity->getY_INLINE());
		    int iNumCities = getNumCities();
			iValue -= (min(abs(iDistance - 4), abs(iDistance - 5)) * 500);
			iValue *= (8 + iNumCities * 4);
			iValue /= (2 + (iNumCities * 4) + iDistance);
		}
	}

	if (pArea->getNumCities() == 0)
	{
		iValue *= 2;
	}
	else
	{
		iTeamAreaCities = GET_TEAM(getTeam()).countNumCitiesByArea(pArea);

		if (pArea->getNumCities() == iTeamAreaCities)
		{
			iValue *= 3;
			iValue /= 2;
		}
		else if (pArea->getNumCities() == (iTeamAreaCities + GET_TEAM(BARBARIAN_TEAM).countNumCitiesByArea(pArea)))
		{
			iValue *= 4;
			iValue /= 3;
		}
		else if (iTeamAreaCities > 0)
		{
			iValue *= 5;
			iValue /= 4;
		}
	}
	
	if (!bStartingLoc)
	{
		int iFoodSurplus = max(0, iSpecialFoodPlus - iSpecialFoodMinus);
		int iFoodDeficit = max(0, iSpecialFoodMinus - iSpecialFoodPlus);

		iValue *= 100 + 20 * max(0, min(iFoodSurplus, 2 * GC.getFOOD_CONSUMPTION_PER_POPULATION()));
		iValue /= 100 + 20 * max(0, iFoodDeficit);
	}
	
	if (!bStartingLoc)
	{
	    int iBonusCount = 0;
	    int iUniqueBonusCount = 0;
	    for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
	    {
	        iBonusCount += paiBonusCount[iI];
	        iUniqueBonusCount += (paiBonusCount[iI] > 0) ? 1 : 0;
	    }
	    if (iBonusCount > 4)
	    {
	        iValue *= 5;
	        iValue /= (1 + iBonusCount);
	    }
	    else if (iUniqueBonusCount > 2)
	    {
	        iValue *= 5;
	        iValue /= (3 + iUniqueBonusCount);	        
	    }
	}
	
	iValue /= (1 + AI_countDeadlockedBonuses(pPlot));
	
	iValue /= (max(0, (iBadTile - (NUM_CITY_PLOTS / 4))) + 3);
	
	if (bStartingLoc)
	{
		iDifferentAreaTile = 0;

		for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
		{
			pLoopPlot = plotCity(iX, iY, iI);

			if ((pLoopPlot == NULL) || !(pLoopPlot->isWater() || pLoopPlot->area() == pArea))
			{
				iDifferentAreaTile++;
			}
		}

		iValue /= (max(0, (iDifferentAreaTile - ((NUM_CITY_PLOTS * 2) / 3))) + 2);
	}

	SAFE_DELETE(paiBonusCount);

	return max(0, iValue);
}


bool CvPlayerAI::AI_isAreaAlone(CvArea* pArea)
{
	return ((pArea->getNumCities() - GET_TEAM(BARBARIAN_TEAM).countNumCitiesByArea(pArea)) == GET_TEAM(getTeam()).countNumCitiesByArea(pArea));
}


bool CvPlayerAI::AI_isCapitalAreaAlone()
{
	CvCity* pCapitalCity;

	pCapitalCity = getCapitalCity();

	if (pCapitalCity != NULL)
	{
		return AI_isAreaAlone(pCapitalCity->area());
	}

	return false;
}


bool CvPlayerAI::AI_isPrimaryArea(CvArea* pArea)
{
	CvCity* pCapitalCity;

	if (pArea->isWater())
	{
		return false;
	}

	if (pArea->getCitiesPerPlayer(getID()) > 2)
	{
		return true;
	}

	pCapitalCity = getCapitalCity();

	if (pCapitalCity != NULL)
	{
		if (pCapitalCity->area() == pArea)
		{
			return true;
		}
	}

	return false;
}


int CvPlayerAI::AI_militaryWeight(CvArea* pArea)
{
	return (pArea->getPopulationPerPlayer(getID()) + pArea->getCitiesPerPlayer(getID()) + 1);
}


int CvPlayerAI::AI_targetCityValue(CvCity* pCity, bool bRandomize)
{
	PROFILE_FUNC();

	CvCity* pNearestCity;
	CvPlot* pLoopPlot;
	int iValue;
	int iI;

	FAssertMsg(pCity != NULL, "City is not assigned a valid value");

	iValue = 1;

	iValue += ((pCity->getPopulation() * (50 + pCity->calculateCulturePercent(getID()))) / 100);

	if (pCity->getDefenseDamage() > 0)
	{
		iValue += ((pCity->getDefenseDamage() / 30) + 1);
	}

	if (pCity->isCoastal(GC.getDefineINT("MIN_WATER_SIZE_FOR_OCEAN")))
	{
		iValue++;
	}

	if (pCity->hasActiveWorldWonder())
	{
		iValue += 1 + pCity->getNumWorldWonders();
	}

	if (pCity->isHolyCity())
	{
		iValue += 2;
	}

	if (pCity->isEverOwned(getID()))
	{
		iValue += 3;
	}

	iValue += AI_adjacentPotentialAttackers(pCity->plot());
	

	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		pLoopPlot = plotCity(pCity->getX_INLINE(), pCity->getY_INLINE(), iI);

		if (pLoopPlot != NULL)
		{
			if (pLoopPlot->getBonusType(getTeam()) != NO_BONUS)
			{
				iValue++;
			}

			if (pLoopPlot->getOwnerINLINE() == getID())
			{
				iValue++;
			}

			if (pLoopPlot->isAdjacentPlayer(getID(), true))
			{
				iValue++;
			}
		}
	}

	pNearestCity = GC.getMapINLINE().findCity(pCity->getX_INLINE(), pCity->getY_INLINE(), getID());

	if (pNearestCity != NULL)
	{
		iValue += max(1, ((GC.getMapINLINE().maxStepDistance() * 2) - GC.getMapINLINE().calculatePathDistance(pNearestCity->plot(), pCity->plot())));
	}

	if (bRandomize)
	{
		iValue += GC.getGameINLINE().getSorenRandNum(((pCity->getPopulation() / 2) + 1), "AI Target City Value");
	}

	return iValue;
}


CvCity* CvPlayerAI::AI_findTargetCity(CvArea* pArea)
{
	CvCity* pLoopCity;
	CvCity* pBestCity;
	int iValue;
	int iBestValue;
	int iLoop;
	int iI;

	iBestValue = 0;
	pBestCity = NULL;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (isPotentialEnemy(getTeam(), GET_PLAYER((PlayerTypes)iI).getTeam()))
			{
				for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
				{
					if (pLoopCity->area() == pArea)
					{
						iValue = AI_targetCityValue(pLoopCity, true);

						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							pBestCity = pLoopCity;
						}
					}
				}
			}
		}
	}

	return pBestCity;
}


bool CvPlayerAI::AI_isCommercePlot(CvPlot* pPlot)
{
	return (pPlot->getYield(YIELD_FOOD) >= GC.getFOOD_CONSUMPTION_PER_POPULATION());
}


int CvPlayerAI::AI_getPlotDanger(CvPlot* pPlot, int iRange, bool bTestMoves)
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pLoopPlot;
	int iCount;
	int iDistance;
	int iBorderDanger;
	int iDX, iDY;


	iCount = 0;
	iBorderDanger = 0;

	if (iRange == -1)
	{
		iRange = DANGER_RANGE;
	}


	for (iDX = -(iRange); iDX <= iRange; iDX++)
	{
		for (iDY = -(iRange); iDY <= iRange; iDY++)
		{
			pLoopPlot = plotXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (pLoopPlot->area() == pPlot->area())
				{
				    iDistance = stepDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE());
				    if (atWar(pLoopPlot->getTeam(), getTeam()))
				    {
				        if (iDistance == 1)
				        {
				            iBorderDanger++;
				        }
				        else if ((iDistance == 2) && (pLoopPlot->isRoute()))
				        {
				            iBorderDanger++;
				        }
				    }


					pUnitNode = pLoopPlot->headUnitNode();

					while (pUnitNode != NULL)
					{
						pLoopUnit = ::getUnit(pUnitNode->m_data);
						pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);

						if (atWar(pLoopUnit->getTeam(), getTeam()))
						{
							if (pLoopUnit->canAttack())
							{
								if (!(pLoopUnit->isInvisible(getTeam(), false)))
								{
								    if (pLoopUnit->canMoveOrAttackInto(pPlot))
								    {
                                        if (!bTestMoves)
                                        {
                                            iCount++;
                                        }
                                        else
                                        {
                                            int iDangerRange = pLoopUnit->baseMoves();
                                            iDangerRange += ((pLoopPlot->isValidRoute(pLoopUnit)) ? 1 : 0);
                                            if (iDangerRange >= iDistance)
                                            {
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

	if (iBorderDanger > 0)
	{
	    if (!isHuman())
	    {
            iCount += iBorderDanger;
	    }
	}

	return iCount;
}

int CvPlayerAI::AI_getUnitDanger(CvUnit* pUnit, int iRange, bool bTestMoves, bool bAnyDanger)
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pLoopPlot;
	int iCount;
	int iDistance;
	int iBorderDanger;
	int iDX, iDY;

    CvPlot* pPlot = pUnit->plot();
	iCount = 0;
	iBorderDanger = 0;

	if (iRange == -1)
	{
		iRange = DANGER_RANGE;
	}
	
	for (iDX = -(iRange); iDX <= iRange; iDX++)
	{
		for (iDY = -(iRange); iDY <= iRange; iDY++)
		{
			pLoopPlot = plotXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (pLoopPlot->area() == pPlot->area())
				{
				    iDistance = stepDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE());
				    if (atWar(pLoopPlot->getTeam(), getTeam()))
				    {
				        if (iDistance == 1)
				        {
				            iBorderDanger++;
				        }
				        else if ((iDistance == 2) && (pLoopPlot->isRoute()))
				        {
				            iBorderDanger++;
				        }
				    }


					pUnitNode = pLoopPlot->headUnitNode();

					while (pUnitNode != NULL)
					{
						pLoopUnit = ::getUnit(pUnitNode->m_data);
						pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);

						if (atWar(pLoopUnit->getTeam(), getTeam()))
						{
							if (pLoopUnit->canAttack())
							{
								if (!(pLoopUnit->isInvisible(getTeam(), false)))
								{
								    if (pLoopUnit->canMoveOrAttackInto(pPlot))
								    {
                                        if (!bTestMoves)
                                        {
                                            iCount++;
                                        }
                                        else
                                        {
                                            int iDangerRange = pLoopUnit->baseMoves();
                                            iDangerRange += ((pLoopPlot->isValidRoute(pLoopUnit)) ? 1 : 0);
                                            if (iDangerRange >= iDistance)
                                            {
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

	if (iBorderDanger > 0)
	{
	    if (!isHuman() || pUnit->isAutomated())
	    {
            iCount += iBorderDanger;
	    }
	}

	return iCount;
}

bool CvPlayerAI::AI_avoidScience()
{
    if (AI_isDoStrategy(AI_STRATEGY_CULTURE4))
    {
        return true;
    }
	if (isCurrentResearchRepeat())
	{
		return true;
	}

	if (isNoResearchAvailable())
	{
		return true;
	}

	return false;
}


// XXX
bool CvPlayerAI::AI_isFinancialTrouble()
{
	if (getCommercePercent(COMMERCE_GOLD) > (((GET_TEAM(getTeam()).getAnyWarPlanCount(true) > 0) ? 60 : 40)))
	{
		if (calculateBaseNetGold() < 0)
		{
			return true;
		}

		if (!AI_avoidScience())
		{
			return true;
		}

		if (isCurrentResearchRepeat())
		{
			if (getCommercePercent(COMMERCE_RESEARCH) == 0)
			{
				return true;
			}
		}
	}

	return false;
}


int CvPlayerAI::AI_goldTarget()
{
	// if we have negative trade deals, we need at least enough to keep us solvent during anarchy
	int iGold = (getMaxAnarchyTurns() > 0) ? max(0, -getGoldPerTurn() * 5) : 0;

	if (GC.getGameINLINE().getElapsedGameTurns() < 40)
	{
		return iGold;
	}

	iGold += ((getNumCities() * 3) + (getTotalPopulation() / 3));

	iGold += (GC.getGameINLINE().getElapsedGameTurns() / 2);

	if (GET_TEAM(getTeam()).getAnyWarPlanCount(true) > 0)
	{
		iGold *= 3;
		iGold /= 2;
	}

	if (AI_avoidScience())
	{
		iGold *= 10;
	}

	iGold += (AI_goldToUpgradeAllUnits() / 2);

	return iGold;
}


TechTypes CvPlayerAI::AI_bestTech(int iMaxPathLength, bool bIgnoreCost, bool bAsync, TechTypes eIgnoreTech, AdvisorTypes eIgnoreAdvisor)
{
	PROFILE("CvPlayerAI::AI_bestTech");

	CvCity* pCapitalCity;
	ImprovementTypes eImprovement;
	BuildingTypes eLoopBuilding;
	RouteTypes eRoute;
	UnitTypes eLoopUnit;
	int iPathLength;
	int iNumBonuses;
	int iValue;
	int iTempValue;
	int iBuildValue;
	int iBonusValue;
	int iMilitaryValue;
	int iI, iJ, iK, iL;

	pCapitalCity = getCapitalCity();

	CvTeam& kTeam = GET_TEAM(getTeam());

	bool bWarPlan = (kTeam.getAnyWarPlanCount(true) > 0);
	bool bCapitalAlone = (GC.getGameINLINE().getElapsedGameTurns() > 0) ? AI_isCapitalAreaAlone() : false;
	bool bFinancialTrouble = AI_isFinancialTrouble();

	int iHasMetCount = kTeam.getHasMetCivCount(true);
	int iCoastalCityCount = countNumCoastalCities();
	int iConnectedForeignCities = countPotentialForeignTradeCitiesConnected();

	int iCityCount = getNumCities();
	int iTeamCityCount = kTeam.getNumCities();

	int iBestValue = 0;
	TechTypes eBestTech = NO_TECH;
	
	int* paiBonusClassRevealed;
	int* paiBonusClassUnrevealed;
	int* paiBonusClassHave;
	
	paiBonusClassRevealed = new int[GC.getNumBonusClassInfos()];
	paiBonusClassUnrevealed = new int[GC.getNumBonusClassInfos()];
	paiBonusClassHave = new int[GC.getNumBonusClassInfos()];
	
	for (iI = 0; iI < GC.getNumBonusClassInfos(); iI++)
	{
        paiBonusClassRevealed[iI] = 0;
        paiBonusClassUnrevealed[iI] = 0;
        paiBonusClassHave[iI] = 0;	    
	}
	
	for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
	{
	    TechTypes eRevealTech = (TechTypes)GC.getBonusInfo((BonusTypes)iI).getTechReveal();
	    BonusClassTypes eBonusClass = (BonusClassTypes)GC.getBonusInfo((BonusTypes)iI).getBonusClassType();
	    if (eRevealTech != NO_TECH)
	    {
	        if ((kTeam.isHasTech(eRevealTech)))
	        {
	            paiBonusClassRevealed[eBonusClass]++;
	        }
	        else
	        {
	            paiBonusClassUnrevealed[eBonusClass]++;
	        }

            if (getNumAvailableBonuses((BonusTypes)iI) > 0)
            {
                paiBonusClassHave[eBonusClass]++;                
            }
            else if (countOwnedBonuses((BonusTypes)iI) > 0)
            {
                paiBonusClassHave[eBonusClass]++;
            }
	    }
	}

#ifdef DEBUG_TECH_CHOICES
	CvWString szPlayerName = getName();
	DEBUGLOG("AI_bestTech:%S\n", szPlayerName.GetCString());
#endif

	for (iI = 0; iI < GC.getNumTechInfos(); iI++)
	{
		if ((eIgnoreTech == NO_TECH) || (iI != eIgnoreTech))
		{
			if ((eIgnoreAdvisor == NO_ADVISOR) || (GC.getTechInfo((TechTypes)iI).getAdvisorType() != eIgnoreAdvisor))
			{
				if (canEverResearch((TechTypes)iI))
				{
					if (!(kTeam.isHasTech((TechTypes)iI)))
					{
						if (GC.getTechInfo((TechTypes)iI).getEra() <= (getCurrentEra() + 1))
						{
							iPathLength = findPathLength(((TechTypes)iI), false);

							if (iPathLength <= iMaxPathLength)
							{
								iValue = 1;

								int iRandomFactor = ((bAsync) ? GC.getASyncRand().get(2000, "AI Research ASYNC") : GC.getGameINLINE().getSorenRandNum(2000, "AI Research"));
								int iRandomMax = 2000;
								iValue += iRandomFactor;

								iValue += kTeam.getResearchProgress((TechTypes)iI);

								if (GC.getTechInfo((TechTypes)iI).isExtraWaterSeeFrom())
								{
									if (iCoastalCityCount > 0)
									{
										iValue += 100;

										if (bCapitalAlone)
										{
											iValue += 400;
										}
									}
								}

								if (GC.getTechInfo((TechTypes)iI).isMapCentering())
								{
									iValue += 100;
								}

								if (GC.getTechInfo((TechTypes)iI).isMapVisible())
								{
									iValue += 100;

									if (bCapitalAlone)
									{
										iValue += 400;
									}
								}

								if (GC.getTechInfo((TechTypes)iI).isMapTrading())
								{
									iValue += 100;

									if (bCapitalAlone)
									{
										iValue += 400;
									}
								}

								if (GC.getTechInfo((TechTypes)iI).isTechTrading() && !GC.getGameINLINE().isOption(GAMEOPTION_NO_TECH_TRADING))
								{
									iValue += 500;

									if (iHasMetCount > 0)
									{
										iValue += 1000;
									}
								}

								if (GC.getTechInfo((TechTypes)iI).isGoldTrading())
								{
									iValue += 200;

									if (iHasMetCount > 0)
									{
										iValue += 400;
									}
								}

								if (GC.getTechInfo((TechTypes)iI).isOpenBordersTrading())
								{
									if (iHasMetCount > 0)
									{
										iValue += 500;

										if (iCoastalCityCount > 0)
										{
											iValue += 400;
										}
									}
								}

								if (GC.getTechInfo((TechTypes)iI).isDefensivePactTrading())
								{
									iValue += 400;
								}

								if (GC.getTechInfo((TechTypes)iI).isPermanentAllianceTrading())
								{
									iValue += 200;
								}

								if (GC.getTechInfo((TechTypes)iI).isVassalStateTrading())
								{
									iValue += 200;
								}

								if (GC.getTechInfo((TechTypes)iI).isBridgeBuilding())
								{
									iValue += 200;
								}

								if (GC.getTechInfo((TechTypes)iI).isIrrigation())
								{
									iValue += 400;
								}

								if (GC.getTechInfo((TechTypes)iI).isIgnoreIrrigation())
								{
									iValue += 500;
								}

								if (GC.getTechInfo((TechTypes)iI).isWaterWork())
								{
									iValue += (600 * iCoastalCityCount);
								}

								iValue += (GC.getTechInfo((TechTypes)iI).getFeatureProductionModifier() * 2);
								iValue += (GC.getTechInfo((TechTypes)iI).getWorkerSpeedModifier() * 4);
								iValue += (GC.getTechInfo((TechTypes)iI).getTradeRoutes() * (max((getNumCities() + 2), iConnectedForeignCities) + 1) * ((bFinancialTrouble) ? 200 : 100));
								iValue += (GC.getTechInfo((TechTypes)iI).getHealth() * 200);

								for (iJ = 0; iJ < GC.getNumRouteInfos(); iJ++)
								{
									iValue += -(GC.getRouteInfo((RouteTypes)iJ).getTechMovementChange(iI) * 100);
								}

								for (iJ = 0; iJ < NUM_DOMAIN_TYPES; iJ++)
								{
									iValue += (GC.getTechInfo((TechTypes)iI).getDomainExtraMoves(iJ) * 200);
								}

								for (iJ = 0; iJ < NUM_COMMERCE_TYPES; iJ++)
								{
									if (GC.getTechInfo((TechTypes)iI).isCommerceFlexible(iJ))
									{
										iValue += 100;
										if ((iJ == COMMERCE_CULTURE) && (AI_isDoStrategy(AI_STRATEGY_CULTURE2)))
										{
										    iValue += 1000;
										}
									}
								}

								for (iJ = 0; iJ < GC.getNumTerrainInfos(); iJ++)
								{
									if (GC.getTechInfo((TechTypes)iI).isTerrainTrade(iJ))
									{
										if (GC.getTerrainInfo((TerrainTypes)iJ).isWater())
										{
											if (pCapitalCity != NULL)
											{
												iValue += (countPotentialForeignTradeCities(pCapitalCity->area()) * 100);
											}

											if (iCoastalCityCount > 0)
											{
												iValue += ((bCapitalAlone) ? 950 : 350);
											}

											iValue += 50;
										}
										else
										{
											iValue += 1000;
										}
									}
								}

								for (iJ = 0; iJ < GC.getNumImprovementInfos(); iJ++)
								{
									for (iK = 0; iK < NUM_YIELD_TYPES; iK++)
									{
										iTempValue = 0;

										iTempValue += (GC.getImprovementInfo((ImprovementTypes)iJ).getTechYieldChanges(iI, iK) * getImprovementCount((ImprovementTypes)iJ) * 50);

										iTempValue *= AI_yieldWeight((YieldTypes)iK);
										iTempValue /= 100;

										iValue += iTempValue;
									}
								}

								iBuildValue = 0;
								for (iJ = 0; iJ < GC.getNumBuildInfos(); iJ++)
								{
									if (GC.getBuildInfo((BuildTypes)iJ).getTechPrereq() == iI)
									{
										eImprovement = (ImprovementTypes)(GC.getBuildInfo((BuildTypes)iJ).getImprovement());
										if (eImprovement != NO_IMPROVEMENT)
										{
											eImprovement = finalImprovementUpgrade(eImprovement);
										}
										else
										{
											// only increment build value if it is not an improvement, otherwise handle it there
											iBuildValue += 200;
										}

										if (eImprovement != NO_IMPROVEMENT)
										{
											CvImprovementInfo& kImprovement = GC.getImprovementInfo(eImprovement);
											
											int iImprovementValue = 300;

											iImprovementValue += ((kImprovement.isActsAsCity()) ? 100 : 0);
											iImprovementValue += ((kImprovement.isHillsMakesValid()) ? 100 : 0);
											iImprovementValue += ((kImprovement.isFreshWaterMakesValid()) ? 200 : 0);
											iImprovementValue += ((kImprovement.isRiverSideMakesValid()) ? 100 : 0);
											iImprovementValue += ((kImprovement.isCarriesIrrigation()) ? 300 : 0);

											for (iK = 0; iK < GC.getNumTerrainInfos(); iK++)
											{
												iImprovementValue += (kImprovement.getTerrainMakesValid(iK) ? 50 : 0);
											}

											for (iK = 0; iK < GC.getNumFeatureInfos(); iK++)
											{
												iImprovementValue += (kImprovement.getFeatureMakesValid(iK) ? 50 : 0);
											}

											for (iK = 0; iK < NUM_YIELD_TYPES; iK++)
											{
												iTempValue = 0;

												iTempValue += (kImprovement.getYieldChange(iK) * 200);
												iTempValue += (kImprovement.getRiverSideYieldChange(iK) * 100);
												iTempValue += (kImprovement.getHillsYieldChange(iK) * 100);
												iTempValue += (kImprovement.getIrrigatedYieldChange(iK) * 150);

												// land food yield is more valueble
												if (iK == YIELD_FOOD && !kImprovement.isWater())
												{
													iTempValue *= 3;
													iTempValue /= 2;
												}

												iTempValue *= AI_yieldWeight((YieldTypes)iK);
												iTempValue /= 100;

												iImprovementValue += iTempValue;
											}
											
											for (iK = 0; iK < GC.getNumBonusInfos(); iK++)
											{
												iBonusValue = 0;

												iBonusValue += ((kImprovement.isImprovementBonusMakesValid(iK)) ? 450 : 0);
												iBonusValue += ((kImprovement.isImprovementBonusTrade(iK)) ? (45 * AI_bonusVal((BonusTypes) iK)) : 0);

												if (iBonusValue > 0)
												{
													for (iL = 0; iL < NUM_YIELD_TYPES; iL++)
													{
														iTempValue = 0;

														iTempValue += (kImprovement.getImprovementBonusYield(iK, iL) * 300);
														iTempValue += (kImprovement.getIrrigatedYieldChange(iL) * 200);

														// food bonuses are more valueble
														if (iL == YIELD_FOOD)
														{
															iTempValue *= 4;
															iTempValue /= 3;
														}
														// otherwise, devalue the bonus slightly
														else
														{
															iTempValue *= 3;
															iTempValue /= 4;
														}

														iTempValue *= AI_yieldWeight((YieldTypes)iL);
														iTempValue /= 100;

														iBonusValue += iTempValue;
													}

													iNumBonuses = countOwnedBonuses((BonusTypes) iK);

													if (iNumBonuses > 0)
													{
														iBonusValue *= (iNumBonuses + 3);
														iBonusValue /= kImprovement.isWater() ? 4 : 3;	// water resources are worth less

														iImprovementValue += iBonusValue;
													}
												}
											}
											
											// if water improvement, weight by coastal cities (weight the whole build)
											if (kImprovement.isWater())
											{
												iImprovementValue *= iCoastalCityCount;
												iImprovementValue /= max(1, iCityCount/2);
											}
											
											iBuildValue += iImprovementValue;
										}

										eRoute = ((RouteTypes)(GC.getBuildInfo((BuildTypes)iJ).getRoute()));

										if (eRoute != NO_ROUTE)
										{
											iBuildValue += ((getBestRoute() == NO_ROUTE) ? 700 : 200) * getNumCities();

											for (iK = 0; iK < NUM_YIELD_TYPES; iK++)
											{
												iTempValue = 0;

												iTempValue += (GC.getRouteInfo(eRoute).getYieldChange(iK) * 100);

												for (iL = 0; iL < GC.getNumImprovementInfos(); iL++)
												{
													iTempValue += (GC.getImprovementInfo((ImprovementTypes)iL).getRouteYieldChanges(eRoute, iK) * 50);
												}

												iTempValue *= AI_yieldWeight((YieldTypes)iK);
												iTempValue /= 100;

												iBuildValue += iTempValue;
											}
										}
									}
								}

                                //the way feature-remove is done in XML is pretty weird
                                //I believe this code needs to be outside the general BuildTypes loop
                                //to ensure the feature-remove is only counted once rather than once per build
								//which could be a lot since nearly every build clears jungle...

								for (iJ = 0; iJ < GC.getNumFeatureInfos(); iJ++)
                                {
                                    bool bIsFeatureRemove = false;
                                    for (iK = 0; iK < GC.getNumBuildInfos(); iK++)
                                    {
                                        if (GC.getBuildInfo((BuildTypes)iK).getFeatureTech(iJ) == iI)
                                        {
                                            bIsFeatureRemove = true;
                                            break;
                                        }
                                    }

                                    if (bIsFeatureRemove)
                                    {
                                        iBuildValue += 100;

                                        if ((GC.getFeatureInfo(FeatureTypes(iJ)).getHealthPercent() < 0) ||
                                            ((GC.getFeatureInfo(FeatureTypes(iJ)).getYieldChange(YIELD_FOOD) + GC.getFeatureInfo(FeatureTypes(iJ)).getYieldChange(YIELD_PRODUCTION) + GC.getFeatureInfo(FeatureTypes(iJ)).getYieldChange(YIELD_COMMERCE)) < 0))
                                        {
                                            iBuildValue += 25 * countCityFeatures((FeatureTypes)iJ);
                                        }
                                    }
                                }

								/*if (AI_totalUnitAIs(UNITAI_WORKER) == 0)
								{
									iBuildValue /= 2;
								}*/
								
								iValue += iBuildValue;
								
								// does tech reveal bonus resources
								for (iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
								{
									if (GC.getBonusInfo((BonusTypes)iJ).getTechReveal() == iI)
									{
										int iRevealValue = 150;
										iRevealValue += (AI_bonusVal((BonusTypes)iJ) * 50);
										
										BonusClassTypes eBonusClass = (BonusClassTypes)GC.getBonusInfo((BonusTypes)iJ).getBonusClassType();
										int iBonusClassTotal = (paiBonusClassRevealed[eBonusClass] + paiBonusClassUnrevealed[eBonusClass]);
										
										//iMultiplier is basically a desperation value
										//it gets larger as the AI runs out of options
										//Copper after failing to get horses is +66%
										//Iron after failing to get horse or copper is +200%
										//but with either copper or horse, Iron is only +25%
										int iMultiplier = 0;
										if (iBonusClassTotal > 0)
										{
                                            iMultiplier = (paiBonusClassRevealed[eBonusClass] - paiBonusClassHave[eBonusClass]);
                                            iMultiplier *= 100;
                                            iMultiplier /= iBonusClassTotal;
                                            
                                            iMultiplier *= (paiBonusClassRevealed[eBonusClass] + 1);
                                            iMultiplier /= ((paiBonusClassHave[eBonusClass] * iBonusClassTotal) + 1);
										}
										
										iMultiplier *= min(3, getNumCities());
										iMultiplier /= 3;
										
										iRevealValue *= 100 + iMultiplier;
										iRevealValue /= 100;
                                    

										iValue += iRevealValue;
										
									}
								}

								bool bEnablesUnitWonder = false;
								for (iJ = 0; iJ < GC.getNumUnitClassInfos(); iJ++)
								{
									eLoopUnit = ((UnitTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iJ)));

									if (eLoopUnit != NO_UNIT)
									{
										if (isTechRequiredForUnit(((TechTypes)iI), eLoopUnit))
										{
                                            const int iBaseUnitValue = 200;
											int iUnitValue = 0;
											
											if ((GC.getUnitClassInfo((UnitClassTypes)iJ).getDefaultUnitIndex()) != (GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iJ)))
                                            {
                                                //UU
                                                iUnitValue += 600;
                                            }

											if (GC.getUnitInfo(eLoopUnit).getPrereqAndTech() == iI)
											{
												iMilitaryValue = 0;

												switch (GC.getUnitInfo(eLoopUnit).getDefaultUnitAIType())
												{
												case UNITAI_UNKNOWN:
												case UNITAI_ANIMAL:
													iUnitValue += iBaseUnitValue;
													break;

												case UNITAI_SETTLE:
													iUnitValue += 1200 + iBaseUnitValue;
													break;

												case UNITAI_WORKER:
													iUnitValue += 800 + iBaseUnitValue;
													break;

												case UNITAI_ATTACK:
													iMilitaryValue += ((bWarPlan) ? 600 : 300);
													iMilitaryValue += (AI_isDoStrategy(AI_STRATEGY_DAGGER ) ? 800 : 0);
													iUnitValue += 100 + iBaseUnitValue;
													break;

												case UNITAI_ATTACK_CITY:
													iMilitaryValue += ((bWarPlan) ? 800 : 400);
													iMilitaryValue += (AI_isDoStrategy(AI_STRATEGY_DAGGER ) ? 800 : 0);
													iUnitValue += 100 + iBaseUnitValue;
													break;

												case UNITAI_COLLATERAL:
													iMilitaryValue += ((bWarPlan) ? 600 : 300);
													iMilitaryValue += (AI_isDoStrategy(AI_STRATEGY_DAGGER ) ? 500 : 0);
													iUnitValue += iBaseUnitValue;
													break;

												case UNITAI_PILLAGE:
													iMilitaryValue += ((bWarPlan) ? 200 : 100);
													iUnitValue += iBaseUnitValue;
													break;

												case UNITAI_RESERVE:
													iMilitaryValue += ((bWarPlan) ? 200 : 100);
													iUnitValue += iBaseUnitValue;
													break;

												case UNITAI_COUNTER:
													iMilitaryValue += ((bWarPlan) ? 600 : 300);
													iMilitaryValue += (AI_isDoStrategy(AI_STRATEGY_DAGGER ) ? 600 : 0);
													iUnitValue += iBaseUnitValue;
													break;

												case UNITAI_CITY_DEFENSE:
													iMilitaryValue += ((bWarPlan) ? 800 : 400);
													iMilitaryValue += ((!bCapitalAlone) ? 400 : 200);
													iUnitValue += ((iHasMetCount > 0) ? 800 : 200) + iBaseUnitValue;
													break;

												case UNITAI_CITY_COUNTER:
													iMilitaryValue += ((bWarPlan) ? 800 : 400);
													iUnitValue += iBaseUnitValue;
													break;

												case UNITAI_CITY_SPECIAL:
													iMilitaryValue += ((bWarPlan) ? 800 : 400);
													iUnitValue += iBaseUnitValue;
													break;

												case UNITAI_EXPLORE:
													iUnitValue += ((bCapitalAlone) ? 100 : 200) + iBaseUnitValue;
													break;

												case UNITAI_MISSIONARY:
													iUnitValue += ((getStateReligion() != NO_RELIGION) ? 600 : 300) + iBaseUnitValue;
													break;

												case UNITAI_PROPHET:
												case UNITAI_ARTIST:
												case UNITAI_SCIENTIST:
												case UNITAI_GENERAL:
												case UNITAI_MERCHANT:
												case UNITAI_ENGINEER:
													iUnitValue += iBaseUnitValue;
													break;

												case UNITAI_SPY:
													iMilitaryValue += ((bWarPlan) ? 100 : 50);
													iUnitValue += iBaseUnitValue;
													break;

												case UNITAI_ICBM:
													iMilitaryValue += ((bWarPlan) ? 100 : 50);
													iUnitValue += iBaseUnitValue;
													break;

												case UNITAI_WORKER_SEA:
													if (iCoastalCityCount > 0)
													{
														// note, workboat improvements are already counted in the improvement section above
														iUnitValue += iBaseUnitValue;
													}
													break;

												case UNITAI_ATTACK_SEA:
													if (iCoastalCityCount > 0)
													{
														iMilitaryValue += ((bWarPlan) ? 200 : 100);
														iMilitaryValue += 100;
														iUnitValue += iBaseUnitValue;
													}
													break;

												case UNITAI_RESERVE_SEA:
													if (iCoastalCityCount > 0)
													{
														iMilitaryValue += ((bWarPlan) ? 100 : 50);
														iUnitValue += iBaseUnitValue;
													}
													break;

												case UNITAI_ESCORT_SEA:
													if (iCoastalCityCount > 0)
													{
														iMilitaryValue += ((bWarPlan) ? 100 : 50);
														iUnitValue += iBaseUnitValue;
													}
													break;

												case UNITAI_EXPLORE_SEA:
													if (iCoastalCityCount > 0)
													{
														iUnitValue += ((bCapitalAlone) ? 1800 : 600) + iBaseUnitValue;
													}
													break;

												case UNITAI_ASSAULT_SEA:
													if (iCoastalCityCount > 0)
													{
														iMilitaryValue += ((bWarPlan || bCapitalAlone) ? 400 : 200);
														iUnitValue += iBaseUnitValue;
													}
													break;

												case UNITAI_SETTLER_SEA:
													if (iCoastalCityCount > 0)
													{
														iUnitValue += ((bWarPlan || bCapitalAlone) ? 200 : 100) + iBaseUnitValue;
													}
													break;

												case UNITAI_MISSIONARY_SEA:
													if (iCoastalCityCount > 0)
													{
														iUnitValue += 100 + iBaseUnitValue;
													}
													break;

												case UNITAI_SPY_SEA:
													if (iCoastalCityCount > 0)
													{
														iMilitaryValue += 100;
														iUnitValue += iBaseUnitValue;
													}
													break;

												case UNITAI_CARRIER_SEA:
													if (iCoastalCityCount > 0)
													{
														iMilitaryValue += ((bWarPlan) ? 100 : 50);
														iUnitValue += iBaseUnitValue;
													}
													break;

												case UNITAI_ATTACK_AIR:
													iMilitaryValue += ((bWarPlan) ? 200 : 100);
													iUnitValue += iBaseUnitValue;
													break;

												case UNITAI_DEFENSE_AIR:
													iMilitaryValue += ((bWarPlan) ? 600 : 400);
													iUnitValue += iBaseUnitValue;
													break;

												case UNITAI_CARRIER_AIR:
													if (iCoastalCityCount > 0)
													{
														iMilitaryValue += ((bWarPlan) ? 100 : 50);
														iUnitValue += iBaseUnitValue;
													}
													break;

												default:
													FAssert(false);
													break;
												}

												if (iHasMetCount == 0)
												{
													iMilitaryValue /= 2;
												}

												if (bCapitalAlone)
												{
													iMilitaryValue *= 2;
													iMilitaryValue /= 3;
												}

												iUnitValue += iMilitaryValue;

												if (AI_totalUnitAIs((UnitAITypes)(GC.getUnitInfo(eLoopUnit).getDefaultUnitAIType())) == 0)
												{
													// do not give bonus to seagoing units if they are worthless
													if (iUnitValue > 0)
													{
														iUnitValue += 200;
													}

													if (GC.getUnitInfo(eLoopUnit).getDefaultUnitAIType() == UNITAI_EXPLORE)
													{
														if (pCapitalCity != NULL)
														{
															iUnitValue += (AI_neededExplorers(pCapitalCity->area()) * 400);
														}
													}

													if (GC.getUnitInfo(eLoopUnit).getDefaultUnitAIType() == UNITAI_EXPLORE_SEA)
													{
														iUnitValue += 400;
														iUnitValue += ((GC.getGameINLINE().countCivTeamsAlive() - iHasMetCount) * 200);
													}
												}

												if (iPathLength <= 1)
												{
													if (getTotalPopulation() > 5)
													{
														if (isWorldUnitClass((UnitClassTypes)iJ))
														{
															if (!(GC.getGameINLINE().isUnitClassMaxedOut((UnitClassTypes)iJ)))
															{
																bEnablesUnitWonder = true;
															}
														}
													}
												}

												iValue += iUnitValue;
											}
										}
									}
								}
								if (bEnablesUnitWonder)
								{
									int iWonderRandom = ((bAsync) ? GC.getASyncRand().get(400, "AI Research Wonder Unit ASYNC") : GC.getGameINLINE().getSorenRandNum(400, "AI Research Wonder Unit"));
									iValue += iWonderRandom + (bCapitalAlone ? 200 : 0);

									iRandomMax += 400;
									iRandomFactor += iWonderRandom;
								}

								int iBestLandBuildingValue = 0;
								bool bEnablesWonder = false;
								for (iJ = 0; iJ < GC.getNumBuildingClassInfos(); iJ++)
								{
									eLoopBuilding = ((BuildingTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iJ)));

									if (eLoopBuilding != NO_BUILDING)
									{
										if (isTechRequiredForBuilding(((TechTypes)iI), eLoopBuilding))
										{
											CvBuildingInfo& kLoopBuilding = GC.getBuildingInfo(eLoopBuilding);
											int iBuildingValue = 0;
											
											if (kLoopBuilding.getSpecialBuildingType() != NO_BUILDING)
											{
												iBuildingValue += ((bCapitalAlone) ? 100 : 25);
											}
											else
											{
												iBuildingValue += ((bCapitalAlone) ? 200 : 50);
											}

											//the granary effect is SO powerful it deserves special code
											if (kLoopBuilding.getFoodKept() > 0)
											{
												iBuildingValue += (15 * kLoopBuilding.getFoodKept());
											}

											iBuildingValue += 100;

                                            if ((GC.getBuildingClassInfo((BuildingClassTypes)iJ).getDefaultBuildingIndex()) != (GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iJ)))
                                            {
                                                //UB
                                                iBuildingValue += 600;
                                            }
                                            
                                            if (AI_isDoStrategy(AI_STRATEGY_CULTURE2))
                                            {
                                                int iMultiplier = (isLimitedWonderClass((BuildingClassTypes)iJ) ? 1 : 3);
                                                iBuildingValue += (150 * kLoopBuilding.getCommerceChange(COMMERCE_CULTURE)) * iMultiplier;
                                                iBuildingValue += kLoopBuilding.getCommerceModifier(COMMERCE_CULTURE) * 4 * iMultiplier ;
                                            }
											
											if (bFinancialTrouble)
											{
												iBuildingValue += kLoopBuilding.getMaintenanceModifier() * 15;
												iBuildingValue += kLoopBuilding.getYieldModifier(YIELD_COMMERCE) * 8;
												iBuildingValue += kLoopBuilding.getCommerceModifier(COMMERCE_GOLD) * 15;
											}

											// if this is a religious building, its not as useful
											ReligionTypes eReligion = (ReligionTypes) kLoopBuilding.getReligionType();
											if (eReligion != NO_RELIGION)
											{
											    
												// reduce by a factor based on how many cities we have with that relgion
												if (iTeamCityCount > 0)
												{
													int iCitiesWithReligion = kTeam.getHasReligionCount(eReligion);
													
													iBuildingValue *= (4 + iCitiesWithReligion);
													iBuildingValue /= (4 + iTeamCityCount);
												}

												// if this building requires a religion, then only count it as 1/7th as much
												// or in other words, only count things like temples once, not 7 times
												// doing it this way in case some mods give buildings to only one religion
                                                iBuildingValue /= max (1, GC.getNumReligionInfos());
											}

											if (kLoopBuilding.getPrereqAndTech() == iI)
											{
												if (iPathLength <= 1)
												{
													if (getTotalPopulation() > 5)
													{
														if (isWorldWonderClass((BuildingClassTypes)iJ))
														{
															if (!(GC.getGameINLINE().isBuildingClassMaxedOut((BuildingClassTypes)iJ)))
															{
																bEnablesWonder = true;

																if (AI_isDoStrategy(AI_STRATEGY_CULTURE1))
																{
																    iValue += 400;
																}

																if (bCapitalAlone)
																{
																	iBuildingValue += 400;
																}
															}
														}
													}
												}
											}


											if (iBuildingValue > iBestLandBuildingValue)
											{
												iBestLandBuildingValue = iBuildingValue;
											}

											// if water building, weight by coastal cities
											if (kLoopBuilding.isWater())
											{
												iBuildingValue *= iCoastalCityCount;
												iBuildingValue /= max(1, iCityCount/2);
											}
											// if land building, is it the best?
											else if (iBuildingValue > iBestLandBuildingValue)
											{
												iBestLandBuildingValue = iBuildingValue;
											}

											iValue += iBuildingValue;
										}
									}
								}
								// if tech gives at least one building (so we can count each individual building less)
								if (iBestLandBuildingValue > 0)
								{
									iValue += iBestLandBuildingValue;
								}

								// if it gives at least one wonder
								if (bEnablesWonder)
								{
									int iWonderRandom = ((bAsync) ? GC.getASyncRand().get(800, "AI Research Wonder Building ASYNC") : GC.getGameINLINE().getSorenRandNum(800, "AI Research Wonder Building"));
									iValue += 500 + iWonderRandom;

									iRandomMax += 800;
									iRandomFactor += iWonderRandom;
								}

								bool bEnablesProjectWonder = false;
								for (iJ = 0; iJ < GC.getNumProjectInfos(); iJ++)
								{
									if (GC.getProjectInfo((ProjectTypes)iJ).getTechPrereq() == iI)
									{
										iValue += 1000;

										if (iPathLength <= 1)
										{
											if (getTotalPopulation() > 5)
											{
												if (isWorldProject((ProjectTypes)iJ))
												{
													if (!(GC.getGameINLINE().isProjectMaxedOut((ProjectTypes)iJ)))
													{
														bEnablesProjectWonder = true;

														if (bCapitalAlone)
														{
															iValue += 100;
														}
													}
												}
											}
										}
									}
								}
								if (bEnablesProjectWonder)
								{
									int iWonderRandom = ((bAsync) ? GC.getASyncRand().get(200, "AI Research Wonder Project ASYNC") : GC.getGameINLINE().getSorenRandNum(200, "AI Research Wonder Project"));
									iValue += iWonderRandom;

									iRandomMax += 200;
									iRandomFactor += iWonderRandom;
								}

								for (iJ = 0; iJ < GC.getNumProcessInfos(); iJ++)
								{
									if (GC.getProcessInfo((ProcessTypes)iJ).getTechPrereq() == iI)
									{
										iValue += 100;

										for (iK = 0; iK < NUM_COMMERCE_TYPES; iK++)
										{
											iTempValue = (GC.getProcessInfo((ProcessTypes)iJ).getProductionToCommerceModifier(iK) * 4);

											iTempValue *= AI_commerceWeight((CommerceTypes)iK);
											iTempValue /= 100;

											if ((iK == COMMERCE_CULTURE) && AI_isDoStrategy(AI_STRATEGY_CULTURE1))
											{
												iTempValue *= 3;
											}

											iValue += iTempValue;
										}
									}
								}

								for (iJ = 0; iJ < GC.getNumCivicInfos(); iJ++)
								{
									if (GC.getCivicInfo((CivicTypes)iJ).getTechPrereq() == iI)
									{
										iValue += 200;

										int iCurrentCivicValue = AI_civicValue(getCivics((CivicOptionTypes)(GC.getCivicInfo((CivicTypes)iJ).getCivicOptionType())));
										int iNewCivicValue = AI_civicValue((CivicTypes)iJ);

										if (iNewCivicValue > iCurrentCivicValue)
										{
											iValue += min(2400, (2400 * (iNewCivicValue - iCurrentCivicValue)) / max(1, iCurrentCivicValue));
										}
									}
								}

								if (iPathLength <= 2)
								{
									if (GC.getGameINLINE().countKnownTechNumTeams((TechTypes)iI) == 0)
									{
										for (iJ = 0; iJ < GC.getNumReligionInfos(); iJ++)
										{
											if (GC.getReligionInfo((ReligionTypes)iJ).getTechPrereq() == iI)
											{
												if (!(GC.getGameINLINE().isReligionFounded((ReligionTypes)iJ)))
												{
													int iReligionValue = ((bAsync) ? GC.getASyncRand().get(2400, "AI Research Religion ASYNC") : GC.getGameINLINE().getSorenRandNum(2400, "AI Research Religion"));
													iRandomMax += 2400;
													iRandomFactor += iReligionValue;

													if (AI_isDoStrategy(AI_STRATEGY_CULTURE1))
                                                    {
                                                        iReligionValue += 500;

														if (countHolyCities() < 1)
														{
															iReligionValue += 1000;
														}
                                                    }
                                                    else
                                                    {
                                                        iReligionValue /= (1 + countHolyCities());
                                                    }

													if (countTotalHasReligion() == 0)
													{
														iReligionValue *= 2;
														iReligionValue += 500;
													}
													
													if (AI_isDoStrategy(AI_STRATEGY_DAGGER))
													{
													    iReligionValue /= 2;
													}

													if (iPathLength < 2)
													{
														iReligionValue *= 3;
														iReligionValue /= 2;
													}

													iValue += iReligionValue;
												}
											}
										}

										if (GC.getTechInfo((TechTypes)iI).getFirstFreeUnitClass() != NO_UNITCLASS)
										{
											if (((UnitTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(GC.getTechInfo((TechTypes) iI).getFirstFreeUnitClass()))) != NO_UNIT)
											{
												int iGreatPeopleRandom = ((bAsync) ? GC.getASyncRand().get(3200, "AI Research Great People ASYNC") : GC.getGameINLINE().getSorenRandNum(3200, "AI Research Great People"));
												iValue += iGreatPeopleRandom;
												
												iRandomMax += 3200;
												iRandomFactor += iGreatPeopleRandom;

												if (bCapitalAlone)
												{
													iValue += 400;
												}

												iValue += 200;
											}
										}

										iValue += (GC.getTechInfo((TechTypes)iI).getFirstFreeTechs() * (200 + ((bCapitalAlone) ? 400 : 0) + ((bAsync) ? GC.getASyncRand().get(3200, "AI Research Free Tech ASYNC") : GC.getGameINLINE().getSorenRandNum(3200, "AI Research Free Tech"))));
									}
								}

								iValue += GC.getTechInfo((TechTypes)iI).getAIWeight();

								if (!isHuman())
								{
									for (iJ = 0; iJ < GC.getNumFlavorTypes(); iJ++)
									{
										iValue += (AI_getFlavorValue((FlavorTypes)iJ) * GC.getTechInfo((TechTypes)iI).getFlavorValue(iJ) * 20);
									}
								}

								if (GC.getTechInfo((TechTypes)iI).isRepeat())
								{
									iValue /= 10;
								}

#ifdef DEBUG_TECH_CHOICES_NO_RANDOM
								// remove random factors, use average
								iValue -= iRandomFactor;
								iValue += iRandomMax/2;
#endif
								int iPreNormalizedValue = iValue;

								if (bIgnoreCost)
								{
									iValue *= (1 + (getResearchTurnsLeft(((TechTypes)iI), false)));
									iValue /= 10;
								}
								else
								{
									if (iValue > 0)
									{
									    //this stops quick speed messing up.... might want to adjust by other things too...
									    int iAdjustment = GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getResearchPercent();
								
									    int iTurnsLeft = getResearchTurnsLeftTimes100(((TechTypes)iI), false);
									    bool bCheapBooster = ((iTurnsLeft < (2 * iAdjustment)) && (0 == ((bAsync) ? GC.getASyncRand().get(5, "AI Choose Cheap Tech") : GC.getGameINLINE().getSorenRandNum(5, "AI Choose Cheap Tech"))));
									    
									    
										iValue *= 100000;
										
                                        iValue /= (iTurnsLeft + (bCheapBooster ? 1 : (5 * iAdjustment)));
									}
								}
								
								if (AI_isDoStrategy(AI_STRATEGY_CULTURE3))
								{
									int iCVValue = AI_cultureVictoryTechValue((TechTypes)iI);
									iValue *= (iCVValue + 10);
									iValue /= ((iCVValue < 100) ? 400 : 100);
								}

								iValue = max(1, iValue);

#ifdef DEBUG_TECH_CHOICES
								CvWString szTechName = GC.getTechInfo((TechTypes)iI).getDescription();
								DEBUGLOG("    %S (%d [%d/%d]->%.2f)\n", szTechName.GetCString(), 
									iPreNormalizedValue, iRandomFactor, iRandomMax, ((float) iValue) / 1000);
#endif
								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									eBestTech = ((TechTypes)iI);
								}
							}
						}
					}
				}
			}
		}
	}

#ifdef DEBUG_TECH_CHOICES
	CvWString szBestTechName = (eBestTech != NO_TECH) ? GC.getTechInfo(eBestTech).getDescription() : L"No Tech";
	DEBUGLOG("  best:%S (%.2f)\n", szBestTechName.GetCString(), ((float) iBestValue) / 1000);
#endif

    SAFE_DELETE_ARRAY(paiBonusClassRevealed);
    SAFE_DELETE_ARRAY(paiBonusClassUnrevealed);
    SAFE_DELETE_ARRAY(paiBonusClassHave);	

	return eBestTech;
}


void CvPlayerAI::AI_chooseFreeTech()
{
	TechTypes eBestTech;

	clearResearchQueue();

	CyArgsList argsList;
	long lResult;
	argsList.add(getID());
	argsList.add(true);
	lResult = -1;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "AI_chooseTech", argsList.makeFunctionArgs(), &lResult);
	eBestTech = ((TechTypes)lResult);

	if (eBestTech == NO_TECH)
	{
		eBestTech = AI_bestTech(1, true);
	}

	if (eBestTech != NO_TECH)
	{
		GET_TEAM(getTeam()).setHasTech(eBestTech, true, getID(), true, true);
	}
}


void CvPlayerAI::AI_chooseResearch()
{
	TechTypes eBestTech;
	int iI;

	clearResearchQueue();

	if (getCurrentResearch() == NO_TECH)
	{
		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if ((iI != getID()) && (GET_PLAYER((PlayerTypes)iI).getTeam() == getTeam()))
				{
					if (GET_PLAYER((PlayerTypes)iI).getCurrentResearch() != NO_TECH)
					{
						if (canResearch(GET_PLAYER((PlayerTypes)iI).getCurrentResearch()))
						{
							pushResearch(GET_PLAYER((PlayerTypes)iI).getCurrentResearch());
						}
					}
				}
			}
		}
	}

	if (getCurrentResearch() == NO_TECH)
	{
		CyArgsList argsList;
		long lResult;
		argsList.add(getID());
		argsList.add(false);
		lResult = -1;
		gDLL->getPythonIFace()->callFunction(PYGameModule, "AI_chooseTech", argsList.makeFunctionArgs(), &lResult);
		eBestTech = ((TechTypes)lResult);

		if (eBestTech == NO_TECH)
		{
			int iAIResearchDepth;
			iAIResearchDepth = AI_isDoStrategy(AI_STRATEGY_CULTURE3) ? 1 : 3;
			
			eBestTech = AI_bestTech((isHuman()) ? 1 : iAIResearchDepth);
		}

		if (eBestTech != NO_TECH)
		{
			pushResearch(eBestTech);
		}
	}
}


DiploCommentTypes CvPlayerAI::AI_getGreeting(PlayerTypes ePlayer)
{
	TeamTypes eWorstEnemy;

	if (GET_PLAYER(ePlayer).getTeam() != getTeam())
	{
		eWorstEnemy = GET_TEAM(getTeam()).AI_getWorstEnemy();

		if ((eWorstEnemy != NO_TEAM) && (eWorstEnemy != GET_PLAYER(ePlayer).getTeam()) && GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isHasMet(eWorstEnemy) && (GC.getASyncRand().get(4) == 0))
		{
			if (GET_PLAYER(ePlayer).AI_hasTradedWithTeam(eWorstEnemy) && !atWar(GET_PLAYER(ePlayer).getTeam(), eWorstEnemy))
			{
				return (DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_WORST_ENEMY_TRADING");
			}
			else
			{
				return (DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_WORST_ENEMY");
			}
		}
		else if ((GET_PLAYER(ePlayer).getPower() < getPower()) && (GC.getASyncRand().get(4) == 0))
		{
			return (DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_UNIT_BRAG");
		}
		else if ((getNumNukeUnits() > 0) && (GC.getASyncRand().get(10) == 0))
		{
			return (DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_NUKES");
		}
	}

	return (DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_GREETINGS");
}


bool CvPlayerAI::AI_isWillingToTalk(PlayerTypes ePlayer)
{
	FAssertMsg(getPersonalityType() != NO_LEADER, "getPersonalityType() is not expected to be equal with NO_LEADER");
	FAssertMsg(ePlayer != getID(), "shouldn't call this function on ourselves");

	if (GET_PLAYER(ePlayer).getTeam() == getTeam()
		|| GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isVassal(getTeam())
		|| GET_TEAM(getTeam()).isVassal(GET_PLAYER(ePlayer).getTeam()))
	{
		return true;
	}

	if (GET_TEAM(getTeam()).isHuman())
	{
		return false;
	}

	if (atWar(getTeam(), GET_PLAYER(ePlayer).getTeam()))
	{
		if (GET_TEAM(getTeam()).AI_getAtWarCounter(GET_PLAYER(ePlayer).getTeam()) < (GC.getLeaderHeadInfo(getPersonalityType()).getRefuseToTalkWarThreshold() * ((GET_TEAM(getTeam()).AI_isChosenWar(GET_PLAYER(ePlayer).getTeam())) ? 2 : 1)))
		{
			return false;
		}

		if (GET_TEAM(getTeam()).isAVassal())
		{
			return false;
		}
	}
	else
	{
		if (AI_getMemoryCount(ePlayer, MEMORY_STOPPED_TRADING_RECENT) > 0)
		{
			return false;
		}
	}

	return true;
}


// XXX what if already at war???
// Returns true if the AI wants to sneak attack...
bool CvPlayerAI::AI_demandRebukedSneak(PlayerTypes ePlayer)
{
	FAssertMsg(!isHuman(), "isHuman did not return false as expected");
	FAssertMsg(ePlayer != getID(), "shouldn't call this function on ourselves");

	FAssert(!(GET_TEAM(getTeam()).isAVassal()));
	FAssert(!(GET_TEAM(getTeam()).isHuman()));

	if (GC.getGameINLINE().getSorenRandNum(100, "AI Demand Rebuked") < GC.getLeaderHeadInfo(getPersonalityType()).getDemandRebukedSneakProb())
	{
		if (GET_TEAM(getTeam()).getPower(true) > GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getDefensivePower())
		{
			return true;
		}
	}

	return false;
}


// XXX what if already at war???
// Returns true if the AI wants to declare war...
bool CvPlayerAI::AI_demandRebukedWar(PlayerTypes ePlayer)
{
	FAssertMsg(!isHuman(), "isHuman did not return false as expected");
	FAssertMsg(ePlayer != getID(), "shouldn't call this function on ourselves");

	FAssert(!(GET_TEAM(getTeam()).isAVassal()));
	FAssert(!(GET_TEAM(getTeam()).isHuman()));

	// needs to be async because it only happens on the computer of the player who is in diplomacy...
	if (GC.getASyncRand().get(100, "AI Demand Rebuked ASYNC") < GC.getLeaderHeadInfo(getPersonalityType()).getDemandRebukedWarProb())
	{
		if (GET_TEAM(getTeam()).getPower(true) > GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getDefensivePower())
		{
			if (GET_TEAM(getTeam()).AI_isAllyLandTarget(GET_PLAYER(ePlayer).getTeam()))
			{
				return true;
			}
		}
	}

	return false;
}


// XXX maybe make this a little looser (by time...)
bool CvPlayerAI::AI_hasTradedWithTeam(TeamTypes eTeam)
{
	int iI;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() == eTeam)
			{
				if ((AI_getPeacetimeGrantValue((PlayerTypes)iI) + AI_getPeacetimeTradeValue((PlayerTypes)iI)) > 0)
				{
					return true;
				}
			}
		}
	}

	return false;
}

// static
AttitudeTypes CvPlayerAI::AI_getAttitude(int iAttitudeVal)
{
	if (iAttitudeVal >= 10)
	{
		return ATTITUDE_FRIENDLY;
	}
	else if (iAttitudeVal >= 3)
	{
		return ATTITUDE_PLEASED;
	}
	else if (iAttitudeVal <= -10)
	{
		return ATTITUDE_FURIOUS;
	}
	else if (iAttitudeVal <= -3)
	{
		return ATTITUDE_ANNOYED;
	}
	else
	{
		return ATTITUDE_CAUTIOUS;
	}
}

AttitudeTypes CvPlayerAI::AI_getAttitude(PlayerTypes ePlayer, bool bForced)
{
	PROFILE_FUNC();

	FAssertMsg(ePlayer != getID(), "shouldn't call this function on ourselves");

	return (AI_getAttitude(AI_getAttitudeVal(ePlayer, bForced)));
}


int CvPlayerAI::AI_getAttitudeVal(PlayerTypes ePlayer, bool bForced)
{
	PROFILE_FUNC();

	int iRankDifference;
	int iAttitude;
	int iI;

	FAssertMsg(ePlayer != getID(), "shouldn't call this function on ourselves");

	if (bForced)
	{
		if (getTeam() == GET_PLAYER(ePlayer).getTeam() || (GET_TEAM(getTeam()).isVassal(GET_PLAYER(ePlayer).getTeam()) && !GET_TEAM(getTeam()).isCapitulated()))
		{
			return 100;
		}

		if (isBarbarian() || GET_PLAYER(ePlayer).isBarbarian())
		{
			return -100;
		}
	}

	iAttitude = GC.getLeaderHeadInfo(getPersonalityType()).getBaseAttitude();

	iAttitude += GC.getHandicapInfo(GET_PLAYER(ePlayer).getHandicapType()).getAttitudeChange();

//	if (GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI))
//	{
//		if (GET_PLAYER(ePlayer).isHuman())
//		{
//			iAttitude -= 2;
//		}
//	}

	if (!(GET_PLAYER(ePlayer).isHuman()))
	{
		iAttitude += (4 - abs(AI_getPeaceWeight() - GET_PLAYER(ePlayer).AI_getPeaceWeight()));
		iAttitude += min(GC.getLeaderHeadInfo(getPersonalityType()).getWarmongerRespect(), GC.getLeaderHeadInfo(GET_PLAYER(ePlayer).getPersonalityType()).getWarmongerRespect());
	}

	iAttitude -= max(0, (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getNumMembers() - GET_TEAM(getTeam()).getNumMembers()));

	iRankDifference = (GC.getGameINLINE().getPlayerRank(getID()) - GC.getGameINLINE().getPlayerRank(ePlayer));

	if (iRankDifference > 0)
	{
		iAttitude += ((GC.getLeaderHeadInfo(getPersonalityType()).getWorseRankDifferenceAttitudeChange() * iRankDifference) / (GC.getGameINLINE().countCivPlayersEverAlive() + 1));
	}
	else
	{
		iAttitude += ((GC.getLeaderHeadInfo(getPersonalityType()).getBetterRankDifferenceAttitudeChange() * -(iRankDifference)) / (GC.getGameINLINE().countCivPlayersEverAlive() + 1));
	}

	if ((GC.getGameINLINE().getPlayerRank(getID()) >= (GC.getGameINLINE().countCivPlayersEverAlive() / 2)) &&
		  (GC.getGameINLINE().getPlayerRank(ePlayer) >= (GC.getGameINLINE().countCivPlayersEverAlive() / 2)))
	{
		iAttitude++;
	}

	if (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).AI_getWarSuccess(getTeam()) > GET_TEAM(getTeam()).AI_getWarSuccess(GET_PLAYER(ePlayer).getTeam()))
	{
		iAttitude += GC.getLeaderHeadInfo(getPersonalityType()).getLostWarAttitudeChange();
	}

	iAttitude += AI_getCloseBordersAttitude(ePlayer);
	iAttitude += AI_getWarAttitude(ePlayer);
	iAttitude += AI_getPeaceAttitude(ePlayer);
	iAttitude += AI_getSameReligionAttitude(ePlayer);
	iAttitude += AI_getDifferentReligionAttitude(ePlayer);
	iAttitude += AI_getBonusTradeAttitude(ePlayer);
	iAttitude += AI_getOpenBordersAttitude(ePlayer);
	iAttitude += AI_getDefensivePactAttitude(ePlayer);
	iAttitude += AI_getRivalDefensivePactAttitude(ePlayer);
	iAttitude += AI_getRivalVassalAttitude(ePlayer);
	iAttitude += AI_getShareWarAttitude(ePlayer);
	iAttitude += AI_getFavoriteCivicAttitude(ePlayer);
	iAttitude += AI_getTradeAttitude(ePlayer);
	iAttitude += AI_getRivalTradeAttitude(ePlayer);

	for (iI = 0; iI < NUM_MEMORY_TYPES; iI++)
	{
		iAttitude += AI_getMemoryAttitude(ePlayer, ((MemoryTypes)iI));
	}

	iAttitude += AI_getAttitudeExtra(ePlayer);

	return range(iAttitude, -100, 100);
}


int CvPlayerAI::AI_calculateStolenCityRadiusPlots(PlayerTypes ePlayer) const
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	int iCount;
	int iI;

	FAssert(ePlayer != getID());

	iCount = 0;

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (pLoopPlot->getOwnerINLINE() == ePlayer)
		{
			if (pLoopPlot->isPlayerCityRadius(getID()))
			{
				iCount++;
			}
		}
	}

	return iCount;
}


int CvPlayerAI::AI_getCloseBordersAttitude(PlayerTypes ePlayer)
{
	int iPercent;

	if (getTeam() == GET_PLAYER(ePlayer).getTeam() || GET_TEAM(getTeam()).isVassal(GET_PLAYER(ePlayer).getTeam()) || GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isVassal(getTeam()))
	{
		return 0;
	}

	iPercent = min(60, (AI_calculateStolenCityRadiusPlots(ePlayer) * 3));

	if (GET_TEAM(getTeam()).AI_isLandTarget(GET_PLAYER(ePlayer).getTeam()))
	{
		iPercent += 40;
	}

	return ((GC.getLeaderHeadInfo(getPersonalityType()).getCloseBordersAttitudeChange() * iPercent) / 100);
}


int CvPlayerAI::AI_getWarAttitude(PlayerTypes ePlayer)
{
	int iAttitudeChange;
	int iAttitude;

	iAttitude = 0;

	if (atWar(getTeam(), GET_PLAYER(ePlayer).getTeam()))
	{
		iAttitude -= 3;
	}

	if (GC.getLeaderHeadInfo(getPersonalityType()).getAtWarAttitudeDivisor() != 0)
	{
		iAttitudeChange = (GET_TEAM(getTeam()).AI_getAtWarCounter(GET_PLAYER(ePlayer).getTeam()) / GC.getLeaderHeadInfo(getPersonalityType()).getAtWarAttitudeDivisor());
		iAttitude += range(iAttitudeChange, -(abs(GC.getLeaderHeadInfo(getPersonalityType()).getAtWarAttitudeChangeLimit())), abs(GC.getLeaderHeadInfo(getPersonalityType()).getAtWarAttitudeChangeLimit()));
	}

	return iAttitude;
}


int CvPlayerAI::AI_getPeaceAttitude(PlayerTypes ePlayer)
{
	int iAttitudeChange;

	if (GC.getLeaderHeadInfo(getPersonalityType()).getAtPeaceAttitudeDivisor() != 0)
	{
		iAttitudeChange = (GET_TEAM(getTeam()).AI_getAtPeaceCounter(GET_PLAYER(ePlayer).getTeam()) / GC.getLeaderHeadInfo(getPersonalityType()).getAtPeaceAttitudeDivisor());
		return range(iAttitudeChange, -(abs(GC.getLeaderHeadInfo(getPersonalityType()).getAtPeaceAttitudeChangeLimit())), abs(GC.getLeaderHeadInfo(getPersonalityType()).getAtPeaceAttitudeChangeLimit()));
	}

	return 0;
}


int CvPlayerAI::AI_getSameReligionAttitude(PlayerTypes ePlayer)
{
	int iAttitudeChange;
	int iAttitude;

	iAttitude = 0;

	if ((getStateReligion() != NO_RELIGION) && (getStateReligion() == GET_PLAYER(ePlayer).getStateReligion()))
	{
		iAttitude += GC.getLeaderHeadInfo(getPersonalityType()).getSameReligionAttitudeChange();

		if (hasHolyCity(getStateReligion()))
		{
			iAttitude++;
		}

		if (GC.getLeaderHeadInfo(getPersonalityType()).getSameReligionAttitudeDivisor() != 0)
		{
			iAttitudeChange = (AI_getSameReligionCounter(ePlayer) / GC.getLeaderHeadInfo(getPersonalityType()).getSameReligionAttitudeDivisor());
			iAttitude += range(iAttitudeChange, -(abs(GC.getLeaderHeadInfo(getPersonalityType()).getSameReligionAttitudeChangeLimit())), abs(GC.getLeaderHeadInfo(getPersonalityType()).getSameReligionAttitudeChangeLimit()));
		}
	}

	return iAttitude;
}


int CvPlayerAI::AI_getDifferentReligionAttitude(PlayerTypes ePlayer)
{
	int iAttitudeChange;
	int iAttitude;

	iAttitude = 0;

	if ((getStateReligion() != NO_RELIGION) && (GET_PLAYER(ePlayer).getStateReligion() != NO_RELIGION) && (getStateReligion() != GET_PLAYER(ePlayer).getStateReligion()))
	{
		iAttitude += GC.getLeaderHeadInfo(getPersonalityType()).getDifferentReligionAttitudeChange();

		if (hasHolyCity(getStateReligion()))
		{
			iAttitude--;
		}

		if (GC.getLeaderHeadInfo(getPersonalityType()).getDifferentReligionAttitudeDivisor() != 0)
		{
			iAttitudeChange = (AI_getDifferentReligionCounter(ePlayer) / GC.getLeaderHeadInfo(getPersonalityType()).getDifferentReligionAttitudeDivisor());
			iAttitude += range(iAttitudeChange, -(abs(GC.getLeaderHeadInfo(getPersonalityType()).getDifferentReligionAttitudeChangeLimit())), abs(GC.getLeaderHeadInfo(getPersonalityType()).getDifferentReligionAttitudeChangeLimit()));
		}
	}

	return iAttitude;
}


int CvPlayerAI::AI_getBonusTradeAttitude(PlayerTypes ePlayer)
{
	int iAttitudeChange;

	if (!atWar(getTeam(), GET_PLAYER(ePlayer).getTeam()))
	{
		if (GC.getLeaderHeadInfo(getPersonalityType()).getBonusTradeAttitudeDivisor() != 0)
		{
			iAttitudeChange = (AI_getBonusTradeCounter(ePlayer) / GC.getLeaderHeadInfo(getPersonalityType()).getBonusTradeAttitudeDivisor());
			return range(iAttitudeChange, -(abs(GC.getLeaderHeadInfo(getPersonalityType()).getBonusTradeAttitudeChangeLimit())), abs(GC.getLeaderHeadInfo(getPersonalityType()).getBonusTradeAttitudeChangeLimit()));
		}
	}

	return 0;
}


int CvPlayerAI::AI_getOpenBordersAttitude(PlayerTypes ePlayer)
{
	int iAttitudeChange;

	if (!atWar(getTeam(), GET_PLAYER(ePlayer).getTeam()))
	{
		if (GC.getLeaderHeadInfo(getPersonalityType()).getOpenBordersAttitudeDivisor() != 0)
		{
			iAttitudeChange = (GET_TEAM(getTeam()).AI_getOpenBordersCounter(GET_PLAYER(ePlayer).getTeam()) / GC.getLeaderHeadInfo(getPersonalityType()).getOpenBordersAttitudeDivisor());
			return range(iAttitudeChange, -(abs(GC.getLeaderHeadInfo(getPersonalityType()).getOpenBordersAttitudeChangeLimit())), abs(GC.getLeaderHeadInfo(getPersonalityType()).getOpenBordersAttitudeChangeLimit()));
		}
	}

	return 0;
}


int CvPlayerAI::AI_getDefensivePactAttitude(PlayerTypes ePlayer)
{
	int iAttitudeChange;

	if (getTeam() != GET_PLAYER(ePlayer).getTeam() && (GET_TEAM(getTeam()).isVassal(GET_PLAYER(ePlayer).getTeam()) || GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isVassal(getTeam())))
	{
		return GC.getLeaderHeadInfo(getPersonalityType()).getDefensivePactAttitudeChangeLimit();
	}

	if (!atWar(getTeam(), GET_PLAYER(ePlayer).getTeam()))
	{
		if (GC.getLeaderHeadInfo(getPersonalityType()).getDefensivePactAttitudeDivisor() != 0)
		{
			iAttitudeChange = (GET_TEAM(getTeam()).AI_getDefensivePactCounter(GET_PLAYER(ePlayer).getTeam()) / GC.getLeaderHeadInfo(getPersonalityType()).getDefensivePactAttitudeDivisor());
			return range(iAttitudeChange, -(abs(GC.getLeaderHeadInfo(getPersonalityType()).getDefensivePactAttitudeChangeLimit())), abs(GC.getLeaderHeadInfo(getPersonalityType()).getDefensivePactAttitudeChangeLimit()));
		}
	}

	return 0;
}


int CvPlayerAI::AI_getRivalDefensivePactAttitude(PlayerTypes ePlayer)
{
	int iAttitude = 0;

	if (getTeam() == GET_PLAYER(ePlayer).getTeam() || GET_TEAM(getTeam()).isVassal(GET_PLAYER(ePlayer).getTeam()) || GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isVassal(getTeam()))
	{
		return iAttitude;
	}

	if (!(GET_TEAM(getTeam()).isDefensivePact(GET_PLAYER(ePlayer).getTeam())))
	{
		iAttitude -= ((4 * GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getDefensivePactCount(GET_PLAYER(ePlayer).getTeam())) / max(1, (GC.getGameINLINE().countCivTeamsAlive() - 2)));
	}

	return iAttitude;
}


int CvPlayerAI::AI_getRivalVassalAttitude(PlayerTypes ePlayer)
{
	int iAttitude = 0;

	if (getTeam() == GET_PLAYER(ePlayer).getTeam() || GET_TEAM(getTeam()).isVassal(GET_PLAYER(ePlayer).getTeam()) || GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isVassal(getTeam()))
	{
		return iAttitude;
	}

	if (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getVassalCount(getTeam()) > 0)
	{
		iAttitude -= (6 * GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getPower(true)) / max(1, GC.getGameINLINE().countTotalCivPower());
	}

	return iAttitude;
}


int CvPlayerAI::AI_getShareWarAttitude(PlayerTypes ePlayer)
{
	int iAttitudeChange;
	int iAttitude;

	iAttitude = 0;

	if (!atWar(getTeam(), GET_PLAYER(ePlayer).getTeam()))
	{
		if (GET_TEAM(getTeam()).AI_shareWar(GET_PLAYER(ePlayer).getTeam()))
		{
			iAttitude += GC.getLeaderHeadInfo(getPersonalityType()).getShareWarAttitudeChange();
		}

		if (GC.getLeaderHeadInfo(getPersonalityType()).getShareWarAttitudeDivisor() != 0)
		{
			iAttitudeChange = (GET_TEAM(getTeam()).AI_getShareWarCounter(GET_PLAYER(ePlayer).getTeam()) / GC.getLeaderHeadInfo(getPersonalityType()).getShareWarAttitudeDivisor());
			iAttitude += range(iAttitudeChange, -(abs(GC.getLeaderHeadInfo(getPersonalityType()).getShareWarAttitudeChangeLimit())), abs(GC.getLeaderHeadInfo(getPersonalityType()).getShareWarAttitudeChangeLimit()));
		}
	}

	return iAttitude;
}


int CvPlayerAI::AI_getFavoriteCivicAttitude(PlayerTypes ePlayer)
{
	int iAttitudeChange;

	int iAttitude;

	iAttitude = 0;

	if (GC.getLeaderHeadInfo(getPersonalityType()).getFavoriteCivic() != NO_CIVIC)
	{
		if (isCivic((CivicTypes)(GC.getLeaderHeadInfo(getPersonalityType()).getFavoriteCivic())) && GET_PLAYER(ePlayer).isCivic((CivicTypes)(GC.getLeaderHeadInfo(getPersonalityType()).getFavoriteCivic())))
		{
			iAttitude += GC.getLeaderHeadInfo(getPersonalityType()).getFavoriteCivicAttitudeChange();

			if (GC.getLeaderHeadInfo(getPersonalityType()).getFavoriteCivicAttitudeDivisor() != 0)
			{
				iAttitudeChange = (AI_getFavoriteCivicCounter(ePlayer) / GC.getLeaderHeadInfo(getPersonalityType()).getFavoriteCivicAttitudeDivisor());
				iAttitude += range(iAttitudeChange, -(abs(GC.getLeaderHeadInfo(getPersonalityType()).getFavoriteCivicAttitudeChangeLimit())), abs(GC.getLeaderHeadInfo(getPersonalityType()).getFavoriteCivicAttitudeChangeLimit()));
			}
		}
	}

	return iAttitude;
}


int CvPlayerAI::AI_getTradeAttitude(PlayerTypes ePlayer)
{
	// XXX human only?
	return range(((AI_getPeacetimeGrantValue(ePlayer) + max(0, (AI_getPeacetimeTradeValue(ePlayer) - GET_PLAYER(ePlayer).AI_getPeacetimeTradeValue(getID())))) / ((GET_TEAM(getTeam()).AI_getHasMetCounter(GET_PLAYER(ePlayer).getTeam()) + 1) * 5)), 0, 4);
}


int CvPlayerAI::AI_getRivalTradeAttitude(PlayerTypes ePlayer)
{
	// XXX human only?
	return -(range(((GET_TEAM(getTeam()).AI_getEnemyPeacetimeGrantValue(GET_PLAYER(ePlayer).getTeam()) + (GET_TEAM(getTeam()).AI_getEnemyPeacetimeTradeValue(GET_PLAYER(ePlayer).getTeam()) / 3)) / ((GET_TEAM(getTeam()).AI_getHasMetCounter(GET_PLAYER(ePlayer).getTeam()) + 1) * 10)), 0, 4));
}


int CvPlayerAI::AI_getMemoryAttitude(PlayerTypes ePlayer, MemoryTypes eMemory)
{
	return ((AI_getMemoryCount(ePlayer, eMemory) * GC.getLeaderHeadInfo(getPersonalityType()).getMemoryAttitudePercent(eMemory)) / 100);
}


int CvPlayerAI::AI_diploVote(VoteTypes eVote)
{
	PROFILE_FUNC();

	CivicTypes eBestCivic;
	TeamTypes eBestTeam;
	bool bValid;
	int iOpenCount;
	int iClosedCount;
	int iValue;
	int iBestValue;
	int iI;

	if (GC.getGameINLINE().isTeamVote(eVote))
	{
		if (GC.getGameINLINE().isTeamVoteEligible(getTeam()))
		{
			return getTeam();
		}

		if (GC.getVoteInfo(eVote).isVictory())
		{
			iBestValue = 7;
		}
		else
		{
			iBestValue = 0;
		}
		eBestTeam = NO_TEAM;

		for (iI = 0; iI < MAX_TEAMS; iI++)
		{
			if (GET_TEAM((TeamTypes)iI).isAlive())
			{
				if (GC.getGameINLINE().isTeamVoteEligible((TeamTypes)iI))
				{
					if (GET_TEAM(getTeam()).isVassal((TeamTypes)iI))
					{
						return ((TeamTypes)iI);
					}

					iValue = GET_TEAM(getTeam()).AI_getAttitudeVal((TeamTypes)iI);

					if (iValue > iBestValue)
					{
						iBestValue = iValue;
						eBestTeam = ((TeamTypes)iI);
					}
				}
			}
		}

		return eBestTeam;
	}
	else
	{
		if (GC.getGameINLINE().getSecretaryGeneral() != NO_TEAM)
		{
			if ((GC.getGameINLINE().getSecretaryGeneral() == getTeam()) ||(GET_TEAM(getTeam()).AI_getAttitude(GC.getGameINLINE().getSecretaryGeneral()) == ATTITUDE_FRIENDLY))
			{
				return true;
			}
		}

		bValid = true;

		if (bValid)
		{
			for (iI = 0; iI < GC.getNumCivicInfos(); iI++)
			{
				if (GC.getVoteInfo(eVote).isForceCivic(iI))
				{
					if (!isCivic((CivicTypes)iI))
					{
						eBestCivic = AI_bestCivic((CivicOptionTypes)(GC.getCivicInfo((CivicTypes)iI).getCivicOptionType()));

						if (eBestCivic != NO_CIVIC)
						{
							if (eBestCivic != ((CivicTypes)iI))
							{
								if (AI_civicValue(eBestCivic) > ((AI_civicValue((CivicTypes)iI) * 6) / 5))
								{
									bValid = false;
									break;
								}
							}
						}
					}
				}
			}
		}

		if (bValid)
		{
			if (GC.getVoteInfo(eVote).getTradeRoutes() > 0)
			{
				if (getNumCities() > ((GC.getGameINLINE().getNumCities() * 2) / (GC.getGameINLINE().countCivPlayersAlive() + 1)))
				{
					bValid = false;
				}
			}
		}

		if (bValid)
		{
			if (GC.getVoteInfo(eVote).isNoNukes())
			{
				if ((GET_TEAM(getTeam()).getNumNukeUnits() / max(1, GET_TEAM(getTeam()).getNumMembers())) < (GC.getGameINLINE().countTotalNukeUnits() / max(1, GC.getGameINLINE().countCivPlayersAlive())))
				{
					bValid = false;
				}
			}
		}

		if (bValid)
		{
			if (GC.getVoteInfo(eVote).isFreeTrade())
			{
				iOpenCount = 0;
				iClosedCount = 0;

				for (iI = 0; iI < MAX_TEAMS; iI++)
				{
					if (GET_TEAM((TeamTypes)iI).isAlive())
					{
						if (iI != getTeam())
						{
							if (GET_TEAM(getTeam()).isOpenBorders((TeamTypes)iI))
							{
								iOpenCount += GET_TEAM((TeamTypes)iI).getNumCities();
							}
							else
							{
								iClosedCount += GET_TEAM((TeamTypes)iI).getNumCities();
							}
						}
					}
				}

				if (iOpenCount >= (getNumCities() * getTradeRoutes()))
				{
					bValid = false;
				}

				if (iClosedCount == 0)
				{
					bValid = false;
				}
			}
		}

		return bValid;
	}
}


int CvPlayerAI::AI_dealVal(PlayerTypes ePlayer, const CLinkList<TradeData>* pList, bool bIgnoreAnnual)
{
	CLLNode<TradeData>* pNode;
	CvCity* pCity;
	int iValue;

	FAssertMsg(ePlayer != getID(), "shouldn't call this function on ourselves");

	iValue = 0;

	if (atWar(getTeam(), GET_PLAYER(ePlayer).getTeam()))
	{
		iValue += GET_TEAM(getTeam()).AI_endWarVal(GET_PLAYER(ePlayer).getTeam());
	}

	for (pNode = pList->head(); pNode; pNode = pList->next(pNode))
	{
		FAssertMsg(!(pNode->m_data.m_bHidden), "(pNode->m_data.m_bHidden) did not return false as expected");

		switch (pNode->m_data.m_eItemType)
		{
		case TRADE_TECHNOLOGIES:
			iValue += GET_TEAM(getTeam()).AI_techTradeVal((TechTypes)(pNode->m_data.m_iData), GET_PLAYER(ePlayer).getTeam());
			break;
		case TRADE_RESOURCES:
			if (!bIgnoreAnnual)
			{
				iValue += AI_bonusTradeVal(((BonusTypes)(pNode->m_data.m_iData)), ePlayer);
			}
			break;
		case TRADE_CITIES:
			pCity = GET_PLAYER(ePlayer).getCity(pNode->m_data.m_iData);
			if (pCity != NULL)
			{
				iValue += AI_cityTradeVal(pCity);
			}
			break;
		case TRADE_GOLD:
			iValue += (pNode->m_data.m_iData * AI_goldTradeValuePercent()) / 100;
			break;
		case TRADE_GOLD_PER_TURN:
			if (!bIgnoreAnnual)
			{
				iValue += AI_goldPerTurnTradeVal(pNode->m_data.m_iData);
			}
			break;
		case TRADE_MAPS:
			iValue += GET_TEAM(getTeam()).AI_mapTradeVal(GET_PLAYER(ePlayer).getTeam());
			break;
		case TRADE_SURRENDER:
			if (!bIgnoreAnnual)
			{
				iValue += GET_TEAM(getTeam()).AI_surrenderTradeVal(GET_PLAYER(ePlayer).getTeam());
			}
			break;
		case TRADE_VASSAL:
			if (!bIgnoreAnnual)
			{
				iValue += GET_TEAM(getTeam()).AI_vassalTradeVal(GET_PLAYER(ePlayer).getTeam());
			}
			break;
		case TRADE_OPEN_BORDERS:
			iValue += GET_TEAM(getTeam()).AI_openBordersTradeVal(GET_PLAYER(ePlayer).getTeam());
			break;
		case TRADE_DEFENSIVE_PACT:
			iValue += GET_TEAM(getTeam()).AI_defensivePactTradeVal(GET_PLAYER(ePlayer).getTeam());
			break;
		case TRADE_PEACE:
			iValue += GET_TEAM(getTeam()).AI_makePeaceTradeVal(((TeamTypes)(pNode->m_data.m_iData)), GET_PLAYER(ePlayer).getTeam());
			break;
		case TRADE_WAR:
			iValue += GET_TEAM(getTeam()).AI_declareWarTradeVal(((TeamTypes)(pNode->m_data.m_iData)), GET_PLAYER(ePlayer).getTeam());
			break;
		case TRADE_EMBARGO:
			iValue += AI_stopTradingTradeVal(((TeamTypes)(pNode->m_data.m_iData)), ePlayer);
			break;
		case TRADE_CIVIC:
			iValue += AI_civicTradeVal(((CivicTypes)(pNode->m_data.m_iData)), ePlayer);
			break;
		case TRADE_RELIGION:
			iValue += AI_religionTradeVal(((ReligionTypes)(pNode->m_data.m_iData)), ePlayer);
			break;
		}
	}

	return iValue;
}


bool CvPlayerAI::AI_goldDeal(const CLinkList<TradeData>* pList)
{
	CLLNode<TradeData>* pNode;

	for (pNode = pList->head(); pNode; pNode = pList->next(pNode))
	{
		FAssert(!(pNode->m_data.m_bHidden));

		switch (pNode->m_data.m_eItemType)
		{
		case TRADE_GOLD:
		case TRADE_GOLD_PER_TURN:
			return true;
			break;
		}
	}

	return false;
}


bool CvPlayerAI::AI_considerOffer(PlayerTypes ePlayer, const CLinkList<TradeData>* pTheirList, const CLinkList<TradeData>* pOurList)
{
	CLLNode<TradeData>* pNode;
	int iThreshold;

	FAssertMsg(ePlayer != getID(), "shouldn't call this function on ourselves");

	if (AI_goldDeal(pTheirList) && AI_goldDeal(pOurList))
	{
		return false;
	}

	for (pNode = pOurList->head(); pNode; pNode = pOurList->next(pNode))
	{
		if (getTradeDenial(ePlayer, pNode->m_data) != NO_DENIAL)
		{
			return false;
		}
	}

	if (GET_PLAYER(ePlayer).getTeam() == getTeam())
	{
		return true;
	}

	if ((pOurList->getLength() == 0) && (pTheirList->getLength() > 0))
	{
		return true;
	}

	int iOurValue = GET_PLAYER(ePlayer).AI_dealVal(getID(), pOurList);
	int iTheirValue = AI_dealVal(ePlayer, pTheirList);

	if (iOurValue > 0 && 0 == pTheirList->getLength() && 0 == iTheirValue)
	{
		if (GET_TEAM(getTeam()).isVassal(GET_PLAYER(ePlayer).getTeam()) && CvDeal::isVassalTributeDeal(pOurList) && !isTributePaid(ePlayer))
		{
			setTributePaid(ePlayer, true);
			if (AI_getAttitude(ePlayer, false) <= GC.getLeaderHeadInfo(getPersonalityType()).getVassalRefuseAttitudeThreshold()
				&& GET_TEAM(getTeam()).getAtWarCount(true) == 0
				&& GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getDefensivePactCount() == 0)
			{
				iOurValue *= (GET_TEAM(getTeam()).getPower(false) + 10);
				iOurValue /= (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getPower(false) + 10);
			}
			else
			{
				return true;
			}
		}
		else
		{
			if (AI_getAttitude(ePlayer) < ATTITUDE_PLEASED)
			{
				if (GET_TEAM(getTeam()).getPower(false) > ((GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getPower(false) * 4) / 3))
				{
					return false;
				}
			}

			if (AI_getMemoryCount(ePlayer, MEMORY_MADE_DEMAND_RECENT) > 0)
			{
				return false;
			}
		}

		iThreshold = (GET_TEAM(getTeam()).AI_getHasMetCounter(GET_PLAYER(ePlayer).getTeam()) + 50);

		iThreshold *= 2;

		if (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).AI_isLandTarget(getTeam()))
		{
			iThreshold *= 3;
		}

		iThreshold *= (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getPower(false) + 100);
		iThreshold /= (GET_TEAM(getTeam()).getPower(false) + 100);

		iThreshold -= GET_PLAYER(ePlayer).AI_getPeacetimeGrantValue(getID());

		return (iOurValue < iThreshold);
	}

	return (iTheirValue >= iOurValue);
}


bool CvPlayerAI::AI_counterPropose(PlayerTypes ePlayer, const CLinkList<TradeData>* pTheirList, const CLinkList<TradeData>* pOurList, CLinkList<TradeData>* pTheirInventory, CLinkList<TradeData>* pOurInventory, CLinkList<TradeData>* pTheirCounter, CLinkList<TradeData>* pOurCounter)
{
	CLLNode<TradeData>* pNode;
	CLLNode<TradeData>* pBestNode;
	CLLNode<TradeData>* pGoldPerTurnNode;
	CLLNode<TradeData>* pGoldNode;
	bool* pabBonusDeal;
	CvCity* pCity;
	bool bTheirGoldDeal;
	bool bOurGoldDeal;
	int iHumanDealWeight;
	int iAIDealWeight;
	int iGoldData;
	int iGoldWeight;
	int iWeight;
	int iBestWeight;
	int iValue;
	int iBestValue;
	int iI;

	bTheirGoldDeal = AI_goldDeal(pTheirList);
	bOurGoldDeal = AI_goldDeal(pOurList);

	if (bOurGoldDeal && bTheirGoldDeal)
	{
		return false;
	}

	pabBonusDeal = new bool[GC.getNumBonusInfos()];

	for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
	{
		pabBonusDeal[iI] = false;
	}

	pGoldPerTurnNode = NULL;
	pGoldNode = NULL;

	iHumanDealWeight = AI_dealVal(ePlayer, pTheirList);
	iAIDealWeight = GET_PLAYER(ePlayer).AI_dealVal(getID(), pOurList);
	
	int iGoldValuePercent = AI_goldTradeValuePercent();

	pTheirCounter->clear();
	pOurCounter->clear();

	if (iAIDealWeight > iHumanDealWeight)
	{
		if (atWar(getTeam(), GET_PLAYER(ePlayer).getTeam()))
		{
			iBestValue = 0;
			iBestWeight = 0;
			pBestNode = NULL;

			for (pNode = pTheirInventory->head(); pNode && iAIDealWeight > iHumanDealWeight; pNode = pTheirInventory->next(pNode))
			{
				if (!pNode->m_data.m_bOffering && !pNode->m_data.m_bHidden)
				{
					if (pNode->m_data.m_eItemType == TRADE_CITIES)
					{
						FAssert(GET_PLAYER(ePlayer).canTradeItem(getID(), pNode->m_data));

						if (GET_PLAYER(ePlayer).getTradeDenial(getID(), pNode->m_data) == NO_DENIAL)
						{
							pCity = GET_PLAYER(ePlayer).getCity(pNode->m_data.m_iData);

							if (pCity != NULL)
							{
								iWeight = AI_cityTradeVal(pCity);

								if (iWeight > 0)
								{
									iValue = AI_targetCityValue(pCity, false);

									if (iValue > iBestValue)
									{
										iBestValue = iValue;
										iBestWeight = iWeight;
										pBestNode = pNode;
									}
								}
							}
						}
					}
				}
			}

			if (pBestNode != NULL)
			{
				iHumanDealWeight += iBestWeight;
				pTheirCounter->insertAtEnd(pBestNode->m_data);
			}
		}

		for (pNode = pTheirInventory->head(); pNode && iAIDealWeight > iHumanDealWeight; pNode = pTheirInventory->next(pNode))
		{
			if (!pNode->m_data.m_bOffering && !pNode->m_data.m_bHidden)
			{
				FAssert(GET_PLAYER(ePlayer).canTradeItem(getID(), pNode->m_data));

				if (GET_PLAYER(ePlayer).getTradeDenial(getID(), pNode->m_data) == NO_DENIAL)
				{
					switch (pNode->m_data.m_eItemType)
					{
					case TRADE_GOLD:
						if (!bOurGoldDeal)
						{
							pGoldNode = pNode;
						}
						break;
					case TRADE_GOLD_PER_TURN:
						if (!bOurGoldDeal)
						{
							pGoldPerTurnNode = pNode;
						}
						break;
					}
				}
			}
		}
		
		int iGoldWeight = iAIDealWeight - iHumanDealWeight;

		if (iGoldWeight > 0)
		{
			if (pGoldNode)
			{
				iGoldData = iGoldWeight * 100;
				iGoldData /= iGoldValuePercent;
				if ((iGoldData * iGoldValuePercent) < iGoldWeight)
				{
					iGoldData++;
				}
				if (GET_PLAYER(ePlayer).AI_maxGoldTrade(getID()) >= iGoldData)
				{
					pGoldNode->m_data.m_iData = iGoldData;
					iHumanDealWeight += (iGoldData * iGoldValuePercent) / 100;
					pTheirCounter->insertAtEnd(pGoldNode->m_data);
					pGoldNode = NULL;
				}
			}
		}

		for (pNode = pOurList->head(); pNode; pNode = pOurList->next(pNode))
		{
			FAssert(!(pNode->m_data.m_bHidden));

			switch (pNode->m_data.m_eItemType)
			{
			case TRADE_RESOURCES:
				pabBonusDeal[pNode->m_data.m_iData] = true;
				break;
			}
		}

		for (pNode = pTheirInventory->head(); pNode && iAIDealWeight > iHumanDealWeight; pNode = pTheirInventory->next(pNode))
		{
			if (!pNode->m_data.m_bOffering && !pNode->m_data.m_bHidden)
			{
				FAssert(GET_PLAYER(ePlayer).canTradeItem(getID(), pNode->m_data));

				if (GET_PLAYER(ePlayer).getTradeDenial(getID(), pNode->m_data) == NO_DENIAL)
				{
					iWeight = 0;

					switch (pNode->m_data.m_eItemType)
					{
					case TRADE_TECHNOLOGIES:
						iWeight += GET_TEAM(getTeam()).AI_techTradeVal((TechTypes)(pNode->m_data.m_iData), GET_PLAYER(ePlayer).getTeam());
						break;
					case TRADE_RESOURCES:
						if (!pabBonusDeal[pNode->m_data.m_iData])
						{
							if (GET_PLAYER(ePlayer).getNumTradeableBonuses((BonusTypes)(pNode->m_data.m_iData)) > 1)
							{
								iWeight += AI_bonusTradeVal(((BonusTypes)(pNode->m_data.m_iData)), ePlayer);
								pabBonusDeal[pNode->m_data.m_iData] = true;
							}
						}
						break;
					}

					if (iWeight > 0)
					{
						iHumanDealWeight += iWeight;
						pTheirCounter->insertAtEnd(pNode->m_data);
					}
				}
			}
		}

		for (pNode = pTheirInventory->head(); pNode && iAIDealWeight > iHumanDealWeight; pNode = pTheirInventory->next(pNode))
		{
			if (!pNode->m_data.m_bOffering && !pNode->m_data.m_bHidden)
			{
				if (pNode->m_data.m_eItemType == TRADE_MAPS)
				{
					FAssert(GET_PLAYER(ePlayer).canTradeItem(getID(), pNode->m_data));

					if (GET_PLAYER(ePlayer).getTradeDenial(getID(), pNode->m_data) == NO_DENIAL)
					{
						iWeight = GET_TEAM(getTeam()).AI_mapTradeVal(GET_PLAYER(ePlayer).getTeam());

						if (iWeight > 0)
						{
							iHumanDealWeight += iWeight;
							pTheirCounter->insertAtEnd(pNode->m_data);
						}
					}
				}
			}
		}
		
		iGoldWeight = iAIDealWeight - iHumanDealWeight;

		if (iGoldWeight > 0)
		{
			if (pGoldNode)
			{
				iGoldData = iGoldWeight * 100;
				iGoldData /= iGoldValuePercent;
				
				if ((iGoldWeight * 100) > (iGoldData * iGoldValuePercent))
				{
					iGoldData++;
				}

				iGoldData = min(iGoldData, GET_PLAYER(ePlayer).AI_maxGoldTrade(getID()));

				if (iGoldData > 0)
				{
					pGoldNode->m_data.m_iData = iGoldData;
					iHumanDealWeight += (iGoldData * iGoldValuePercent) / 100;
					pTheirCounter->insertAtEnd(pGoldNode->m_data);
					pGoldNode = NULL;
				}
			}
		}

		if (iAIDealWeight > iHumanDealWeight)
		{
			if (pGoldPerTurnNode)
			{
				iGoldData = 0;

				while (AI_goldPerTurnTradeVal(iGoldData) < (iAIDealWeight - iHumanDealWeight))
				{
					iGoldData++;
				}

				iGoldData = min(iGoldData, GET_PLAYER(ePlayer).AI_maxGoldPerTurnTrade(getID()));

				if (iGoldData > 0)
				{
					pGoldPerTurnNode->m_data.m_iData = iGoldData;
					iHumanDealWeight += AI_goldPerTurnTradeVal(pGoldPerTurnNode->m_data.m_iData);
					pTheirCounter->insertAtEnd(pGoldPerTurnNode->m_data);
					pGoldPerTurnNode = NULL;
				}
			}
		}

		for (pNode = pTheirInventory->head(); pNode && iAIDealWeight > iHumanDealWeight; pNode = pTheirInventory->next(pNode))
		{
			if (!pNode->m_data.m_bOffering && !pNode->m_data.m_bHidden)
			{
				if (pNode->m_data.m_eItemType == TRADE_RESOURCES)
				{
					FAssert(GET_PLAYER(ePlayer).canTradeItem(getID(), pNode->m_data));

					if (GET_PLAYER(ePlayer).getTradeDenial(getID(), pNode->m_data) == NO_DENIAL)
					{
						iWeight = 0;

						if (!pabBonusDeal[pNode->m_data.m_iData])
						{
							if (GET_PLAYER(ePlayer).getNumTradeableBonuses((BonusTypes)(pNode->m_data.m_iData)) > 0)
							{
								iWeight += AI_bonusTradeVal(((BonusTypes)(pNode->m_data.m_iData)), ePlayer);
								pabBonusDeal[pNode->m_data.m_iData] = true;
							}
						}

						if (iWeight > 0)
						{
							iHumanDealWeight += iWeight;
							pTheirCounter->insertAtEnd(pNode->m_data);
						}
					}
				}
			}
		}
	}
	else if (iHumanDealWeight > iAIDealWeight)
	{
		if (atWar(getTeam(), GET_PLAYER(ePlayer).getTeam()))
		{
			bool bSurrender = false;
			for (pNode = pOurInventory->head(); pNode; pNode = pOurInventory->next(pNode))
			{
				if (!pNode->m_data.m_bOffering && !pNode->m_data.m_bHidden)
				{
					if (pNode->m_data.m_eItemType == TRADE_SURRENDER)
					{
						if (getTradeDenial(ePlayer, pNode->m_data) == NO_DENIAL)
						{
							iAIDealWeight += GET_TEAM(GET_PLAYER(ePlayer).getTeam()).AI_surrenderTradeVal(getTeam());
							pOurCounter->insertAtEnd(pNode->m_data);
							bSurrender = true;
						}
						break;
					}
				}
			}

			if (!bSurrender)
			{
				for (pNode = pOurInventory->head(); pNode; pNode = pOurInventory->next(pNode))
				{
					if (!pNode->m_data.m_bOffering && !pNode->m_data.m_bHidden)
					{
						if (pNode->m_data.m_eItemType == TRADE_PEACE_TREATY)
						{
							pOurCounter->insertAtEnd(pNode->m_data);
							break;
						}
					}
				}
			}

			iBestValue = 0;
			iBestWeight = 0;
			pBestNode = NULL;

			for (pNode = pOurInventory->head(); pNode && iHumanDealWeight > iAIDealWeight; pNode = pOurInventory->next(pNode))
			{
				if (!pNode->m_data.m_bOffering && !pNode->m_data.m_bHidden)
				{
					if (pNode->m_data.m_eItemType == TRADE_CITIES)
					{
						FAssert(canTradeItem(ePlayer, pNode->m_data));

						if (getTradeDenial(ePlayer, pNode->m_data) == NO_DENIAL)
						{
							pCity = getCity(pNode->m_data.m_iData);

							if (pCity != NULL)
							{
								iWeight = GET_PLAYER(ePlayer).AI_cityTradeVal(pCity);

								if (iWeight > 0)
								{
									iValue = GET_PLAYER(ePlayer).AI_targetCityValue(pCity, false);

									if (iValue > iBestValue)
									{
										if (iHumanDealWeight >= (iAIDealWeight + iWeight))
										{
											iBestValue = iValue;
											iBestWeight = iWeight;
											pBestNode = pNode;
										}
									}
								}
							}
						}
					}
				}
			}

			if (pBestNode != NULL)
			{
				iAIDealWeight += iBestWeight;
				pOurCounter->insertAtEnd(pBestNode->m_data);
			}
		}

		for (pNode = pOurInventory->head(); pNode && iHumanDealWeight > iAIDealWeight; pNode = pOurInventory->next(pNode))
		{
			if (!pNode->m_data.m_bOffering && !pNode->m_data.m_bHidden)
			{
				FAssert(canTradeItem(ePlayer, pNode->m_data));

				if (getTradeDenial(ePlayer, pNode->m_data) == NO_DENIAL)
				{
					switch (pNode->m_data.m_eItemType)
					{
					case TRADE_GOLD:
						if (!bTheirGoldDeal)
						{
							pGoldNode = pNode;
						}
						break;
					case TRADE_GOLD_PER_TURN:
						if (!bTheirGoldDeal)
						{
							pGoldPerTurnNode = pNode;
						}
						break;
					}
				}
			}
		}
		
		iGoldWeight = iHumanDealWeight - iAIDealWeight;

		if (iGoldWeight > 0)
		{
			if (pGoldNode)
			{
				int iGoldData = iGoldWeight * 100;
				iGoldData /= iGoldValuePercent;

				if (AI_maxGoldTrade(ePlayer) >= iGoldData)
				{
					pGoldNode->m_data.m_iData = iGoldData;
					iAIDealWeight += ((iGoldData * iGoldValuePercent) / 100);
					pOurCounter->insertAtEnd(pGoldNode->m_data);
					pGoldNode = NULL;
				}
			}
		}

		for (pNode = pTheirList->head(); pNode; pNode = pTheirList->next(pNode))
		{
			FAssert(!(pNode->m_data.m_bHidden));

			switch (pNode->m_data.m_eItemType)
			{
			case TRADE_RESOURCES:
				pabBonusDeal[pNode->m_data.m_iData] = true;
				break;
			}
		}

		for (pNode = pOurInventory->head(); pNode && iHumanDealWeight > iAIDealWeight; pNode = pOurInventory->next(pNode))
		{
			if (!pNode->m_data.m_bOffering && !pNode->m_data.m_bHidden)
			{
				FAssert(canTradeItem(ePlayer, pNode->m_data));

				if (getTradeDenial(ePlayer, pNode->m_data) == NO_DENIAL)
				{
					iWeight = 0;

					switch (pNode->m_data.m_eItemType)
					{
					case TRADE_TECHNOLOGIES:
						iWeight += GET_TEAM(GET_PLAYER(ePlayer).getTeam()).AI_techTradeVal((TechTypes)(pNode->m_data.m_iData), getTeam());
						break;
					case TRADE_RESOURCES:
						if (!pabBonusDeal[pNode->m_data.m_iData])
						{
							if (getNumTradeableBonuses((BonusTypes)(pNode->m_data.m_iData)) > 1)
							{
								iWeight += GET_PLAYER(ePlayer).AI_bonusTradeVal(((BonusTypes)(pNode->m_data.m_iData)), getID());
								pabBonusDeal[pNode->m_data.m_iData] = true;
							}
						}
						break;
					}

					if (iWeight > 0)
					{
						if (iHumanDealWeight >= (iAIDealWeight + iWeight))
						{
							iAIDealWeight += iWeight;
							pOurCounter->insertAtEnd(pNode->m_data);
						}
					}
				}
			}
		}

		for (pNode = pOurInventory->head(); pNode && iHumanDealWeight > iAIDealWeight; pNode = pOurInventory->next(pNode))
		{
			if (!pNode->m_data.m_bOffering && !pNode->m_data.m_bHidden)
			{
				if (pNode->m_data.m_eItemType == TRADE_MAPS)
				{
					FAssert(canTradeItem(ePlayer, pNode->m_data));

					if (getTradeDenial(ePlayer, pNode->m_data) == NO_DENIAL)
					{
						iWeight = GET_TEAM(GET_PLAYER(ePlayer).getTeam()).AI_mapTradeVal(getTeam());

						if (iWeight > 0)
						{
							if (iHumanDealWeight >= (iAIDealWeight + iWeight))
							{
								iAIDealWeight += iWeight;
								pOurCounter->insertAtEnd(pNode->m_data);
							}
						}
					}
				}
			}
		}

		iGoldWeight = iHumanDealWeight - iAIDealWeight;
		if (iGoldWeight > 0)
		{
			if (pGoldNode)
			{
				iGoldData = iGoldWeight * 100;
				iGoldData /= AI_goldTradeValuePercent();

				iGoldData = min(iGoldData, AI_maxGoldTrade(ePlayer));

				if (iGoldData > 0)
				{
					pGoldNode->m_data.m_iData = iGoldData;
					iAIDealWeight += (iGoldData * AI_goldTradeValuePercent()) / 100;
					pOurCounter->insertAtEnd(pGoldNode->m_data);
					pGoldNode = NULL;
				}
			}
		}

		if (iHumanDealWeight > iAIDealWeight)
		{
			if (pGoldPerTurnNode)
			{
				iGoldData = 0;

				while (GET_PLAYER(ePlayer).AI_goldPerTurnTradeVal(iGoldData + 1) <= (iHumanDealWeight - iAIDealWeight))
				{
					iGoldData++;
				}

				iGoldData = min(iGoldData, AI_maxGoldPerTurnTrade(ePlayer));

				if (iGoldData > 0)
				{
					pGoldPerTurnNode->m_data.m_iData = iGoldData;
					iAIDealWeight += GET_PLAYER(ePlayer).AI_goldPerTurnTradeVal(pGoldPerTurnNode->m_data.m_iData);
					pOurCounter->insertAtEnd(pGoldPerTurnNode->m_data);
					pGoldPerTurnNode = NULL;
				}
			}
		}
	}

	SAFE_DELETE_ARRAY(pabBonusDeal);

	return ((iAIDealWeight <= iHumanDealWeight) && ((pOurList->getLength() > 0) || (pOurCounter->getLength() > 0) || (pTheirCounter->getLength() > 0)));
}


int CvPlayerAI::AI_maxGoldTrade(PlayerTypes ePlayer)
{
	int iMaxGold;
	int iResearchBuffer;

	FAssert(ePlayer != getID());

	if (isHuman() || (GET_PLAYER(ePlayer).getTeam() == getTeam()))
	{
		iMaxGold = getGold();
	}
	else
	{

		iMaxGold = getTotalPopulation();

		iMaxGold *= (GET_TEAM(getTeam()).AI_getHasMetCounter(GET_PLAYER(ePlayer).getTeam()) + 10);

		iMaxGold *= GC.getLeaderHeadInfo(getPersonalityType()).getMaxGoldTradePercent();
		iMaxGold /= 100;

		iMaxGold -= AI_getGoldTradedTo(ePlayer);

        iResearchBuffer = -calculateGoldRate() * 12;
        iResearchBuffer *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getResearchPercent();
        iResearchBuffer /= 100;

		iMaxGold = min(iMaxGold, getGold() - iResearchBuffer);

		iMaxGold = min(iMaxGold, getGold());

		iMaxGold -= (iMaxGold % GC.getDefineINT("DIPLOMACY_VALUE_REMAINDER"));


	}


	return max(0, iMaxGold);
}


int CvPlayerAI::AI_maxGoldPerTurnTrade(PlayerTypes ePlayer)
{
	int iMaxGoldPerTurn;

	FAssert(ePlayer != getID());

	if (isHuman() || (GET_PLAYER(ePlayer).getTeam() == getTeam()))
	{
		iMaxGoldPerTurn = (calculateGoldRate() + (getGold() / GC.getDefineINT("PEACE_TREATY_LENGTH")));
	}
	else
	{
		iMaxGoldPerTurn = getTotalPopulation();

		iMaxGoldPerTurn *= GC.getLeaderHeadInfo(getPersonalityType()).getMaxGoldPerTurnTradePercent();
		iMaxGoldPerTurn /= 100;

		iMaxGoldPerTurn += min(0, getGoldPerTurnByPlayer(ePlayer));
	}

	return max(0, min(iMaxGoldPerTurn, calculateGoldRate()));
}


int CvPlayerAI::AI_goldPerTurnTradeVal(int iGoldPerTurn)
{
	int iValue = iGoldPerTurn * GC.getDefineINT("PEACE_TREATY_LENGTH");
	iValue *= AI_goldTradeValuePercent();
	iValue /= 100;
	
	return iValue;
}


int CvPlayerAI::AI_bonusVal(BonusTypes eBonus)
{
	PROFILE_FUNC();

	UnitTypes eLoopUnit;
	BuildingTypes eLoopBuilding;
	int iDiff;
	int iValue;
	int iTempValue;
	int iI, iJ;

	if ((GC.getBonusInfo(eBonus).getTechObsolete() != NO_TECH) && GET_TEAM(getTeam()).isHasTech((TechTypes)(GC.getBonusInfo(eBonus).getTechObsolete())))
	{
		return 0;
	}

	iValue = 0;

	iValue += (GC.getBonusInfo(eBonus).getHappiness() * 100);
	iValue += (GC.getBonusInfo(eBonus).getHealth() * 100);
	
	CvTeam& kTeam = GET_TEAM(getTeam());

	CvCity* pCapital = getCapitalCity();
	int iCityCount = getNumCities();
	int iCoastalCityCount = countNumCoastalCities();
	
	// find the first coastal city
	CvCity* pCoastalCity = NULL;
	CvCity* pUnconnectedCoastalCity = NULL;
	if (iCoastalCityCount > 0)
	{
		int iLoop;
		for (CvCity* pLoopCity = firstCity(&iLoop); pCoastalCity == NULL && pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
		{
			if (pLoopCity->isCoastal(GC.getDefineINT("MIN_WATER_SIZE_FOR_OCEAN")))
			{
				if (pLoopCity->isConnectedToCapital(getID()))
				{
					pCoastalCity = pLoopCity;
				}
				else if (pUnconnectedCoastalCity == NULL)
				{
					pUnconnectedCoastalCity = pLoopCity;
				}
			}
		}
	}
	if (pCoastalCity == NULL && pUnconnectedCoastalCity != NULL)
	{
		pCoastalCity = pUnconnectedCoastalCity;
	}


	for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
	{
		eLoopUnit = ((UnitTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI)));

		if (eLoopUnit != NO_UNIT)
		{
			CvUnitInfo& kLoopUnit = GC.getUnitInfo(eLoopUnit);

			iTempValue = 0;

			if (kLoopUnit.getPrereqAndBonus() == eBonus)
			{
				iTempValue += 50;
			}

			for (iJ = 0; iJ < GC.getNUM_UNIT_PREREQ_OR_BONUSES(); iJ++)
			{
				if (kLoopUnit.getPrereqOrBonuses(iJ) == eBonus)
				{
					iTempValue += 40;
				}
			}

			iTempValue += kLoopUnit.getBonusProductionModifier(eBonus) / 10;

			if (iTempValue > 0)
			{
				bool bIsWater = (kLoopUnit.getDomainType() == DOMAIN_SEA);
				
				// if non-limited water unit, weight by coastal cities
				if (bIsWater && !isLimitedUnitClass((UnitClassTypes)(kLoopUnit.getUnitClassType())))
				{
					iTempValue *= min(iCoastalCityCount * 2, iCityCount);	// double coastal cities, cap at total cities
					iTempValue /= max(1, iCityCount);
				}

				if (canTrain(eLoopUnit))
				{
					// is it a water unit and no coastal cities or our coastal city cannot build because its obsolete
					if ((bIsWater && (pCoastalCity == NULL || (pCoastalCity->allUpgradesAvailable(eLoopUnit) != NO_UNIT))) ||
						// or our capital cannot build because its obsolete (we can already build all its upgrades)
						(pCapital != NULL && pCapital->allUpgradesAvailable(eLoopUnit) != NO_UNIT))
					{
						// its worthless
						iTempValue = 2;
					}
					// otherwise, value units we could build if we had this bonus double
					else
					{
						iTempValue *= 2;
					}
				}

				if (kLoopUnit.getPrereqAndTech() != NO_TECH)
				{
					iDiff = abs(GC.getTechInfo((TechTypes)(kLoopUnit.getPrereqAndTech())).getEra() - getCurrentEra());

					if (iDiff == 0)
					{
						iTempValue *= 3;
						iTempValue /= 2;
					}
					else
					{
						iTempValue /= iDiff;
					}
				}

				iValue += iTempValue;
			}
		}
	}

	for (iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
	{
		eLoopBuilding = ((BuildingTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iI)));

		if (eLoopBuilding != NO_BUILDING)
		{
			CvBuildingInfo& kLoopBuilding = GC.getBuildingInfo(eLoopBuilding);
			
			iTempValue = 0;

			if (kLoopBuilding.getPrereqAndBonus() == eBonus)
			{
				iTempValue += 30;
			}

			for (iJ = 0; iJ < GC.getNUM_BUILDING_PREREQ_OR_BONUSES(); iJ++)
			{
				if (kLoopBuilding.getPrereqOrBonuses(iJ) == eBonus)
				{
					iTempValue += 20;
				}
			}
			
			iTempValue += kLoopBuilding.getBonusProductionModifier(eBonus) / 10;
			
			if (iTempValue > 0)
			{
				// determine whether we have the tech for this building
				bool bHasTechForBuilding = true;
				if (!(kTeam.isHasTech((TechTypes)(kLoopBuilding.getPrereqAndTech()))))
				{
					bHasTechForBuilding = false;
				}
				for (int iPrereqIndex = 0; bHasTechForBuilding && iPrereqIndex < GC.getDefineINT("NUM_BUILDING_AND_TECH_PREREQS"); iPrereqIndex++)
				{
					if (kLoopBuilding.getPrereqAndTechs(iPrereqIndex) != NO_TECH)
					{
						if (!(kTeam.isHasTech((TechTypes)(kLoopBuilding.getPrereqAndTechs(iPrereqIndex)))))
						{
							bHasTechForBuilding = false;
						}
					}
				}
				
				bool bIsStateReligion = (((ReligionTypes) kLoopBuilding.getStateReligion()) != NO_RELIGION);
				bool bCanConstruct = canConstruct(eLoopBuilding, false, /*bTestVisible*/ true, /*bIgnoreCost*/ true);
				
				// bCanNeverBuild when true is accurate, it may be false in some cases where we will never be able to build 
				bool bCanNeverBuild = (bHasTechForBuilding && !bCanConstruct && !bIsStateReligion);

				// if we can never build this, it is worthless
				if (bCanNeverBuild)
				{
					iTempValue = 1;
				}
				// double value if we can build it right now
				else if (bCanConstruct)
				{
					iTempValue *= 2;
				}
				
				// if non-limited water building, weight by coastal cities
				if (kLoopBuilding.isWater() && !isLimitedWonderClass((BuildingClassTypes)(kLoopBuilding.getBuildingClassType())))
				{
					iTempValue *= iCoastalCityCount;
					iTempValue /= max(1, iCityCount/2);
				}

				if (kLoopBuilding.getPrereqAndTech() != NO_TECH)
				{
					iDiff = abs(GC.getTechInfo((TechTypes)(kLoopBuilding.getPrereqAndTech())).getEra() - getCurrentEra());

					if (iDiff == 0)
					{
						iTempValue *= 3;
						iTempValue /= 2;
					}
					else
					{
						iTempValue /= iDiff;
					}
				}

				iValue += iTempValue;
			}
		}
	}

	for (iI = 0; iI < GC.getNumProjectInfos(); iI++)
	{
		ProjectTypes eProject = (ProjectTypes) iI;
		CvProjectInfo& kLoopProject = GC.getProjectInfo(eProject);
		iTempValue = 0;

		iTempValue += kLoopProject.getBonusProductionModifier(eBonus) / 10;
		
		if (iTempValue > 0)
		{
			bool bMaxedOut = (GC.getGameINLINE().isProjectMaxedOut(eProject) || kTeam.isProjectMaxedOut(eProject));

			if (bMaxedOut)
			{
				// project worthless
				iTempValue = 1;
			}
			else if (canCreate(eProject))
			{
				iTempValue *= 2;
			}

			if (kLoopProject.getTechPrereq() != NO_TECH)
			{
				iDiff = abs(GC.getTechInfo((TechTypes)(kLoopProject.getTechPrereq())).getEra() - getCurrentEra());

				if (iDiff == 0)
				{
					iTempValue *= 3;
					iTempValue /= 2;
				}
				else
				{
					iTempValue /= iDiff;
				}
			}

			iValue += iTempValue;
		}
	}

    RouteTypes eBestRoute = getBestRoute();
	for (iI = 0; iI < GC.getNumBuildInfos(); iI++)
	{
        RouteTypes eRoute = (RouteTypes)(GC.getBuildInfo((BuildTypes)iI).getRoute());

		if ((eRoute != NO_ROUTE) && (GC.getRouteInfo(eRoute).getPrereqBonus() == eBonus))
        {
			if ((eBestRoute != NO_ROUTE) && (GC.getRouteInfo(getBestRoute()).getValue() <= GC.getRouteInfo(eRoute).getValue()))
			{
				iTempValue = 160;
			}
			else
			{
				iTempValue = 40;
			}

			iValue += iTempValue;
		}
    }

	iValue /= 10;

	return iValue;
}


int CvPlayerAI::AI_bonusTradeVal(BonusTypes eBonus, PlayerTypes ePlayer)
{
	int iValue;

	FAssertMsg(ePlayer != getID(), "shouldn't call this function on ourselves");

	iValue = AI_bonusVal(eBonus);

	iValue *= ((min(getNumCities(), GET_PLAYER(ePlayer).getNumCities()) + 3) * 30);
	iValue /= 100;

	iValue *= max(0, (GC.getBonusInfo(eBonus).getAITradeModifier() + 100));
	iValue /= 100;

	if (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isVassal(getTeam()) && !GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isCapitulated())
	{
		iValue /= 2;
	}

	return (iValue * GC.getDefineINT("PEACE_TREATY_LENGTH"));
}


DenialTypes CvPlayerAI::AI_bonusTrade(BonusTypes eBonus, PlayerTypes ePlayer)
{
	PROFILE_FUNC();

	AttitudeTypes eAttitude;
	bool bStrategic;
	int iI, iJ;

	FAssertMsg(ePlayer != getID(), "shouldn't call this function on ourselves");

	if (isHuman())
	{
		return NO_DENIAL;
	}

	if (GET_TEAM(getTeam()).isVassal(GET_PLAYER(ePlayer).getTeam()))
	{
		return NO_DENIAL;
	}

	if (atWar(getTeam(), GET_PLAYER(ePlayer).getTeam()))
	{
		return NO_DENIAL;
	}

	if (GET_PLAYER(ePlayer).getTeam() == getTeam())
	{
		return NO_DENIAL;
	}

	if (GET_TEAM(getTeam()).AI_getWorstEnemy() == GET_PLAYER(ePlayer).getTeam())
	{
		return DENIAL_WORST_ENEMY;
	}

	bStrategic = false;

	for (iI = 0; iI < GC.getNumUnitInfos(); iI++)
	{
		if (GC.getUnitInfo((UnitTypes) iI).getPrereqAndBonus() == eBonus)
		{
			bStrategic = true;
		}

		for (iJ = 0; iJ < GC.getNUM_UNIT_PREREQ_OR_BONUSES(); iJ++)
		{
			if (GC.getUnitInfo((UnitTypes) iI).getPrereqOrBonuses(iJ) == eBonus)
			{
				bStrategic = true;
			}
		}
	}

	for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
	{
		if (GC.getBuildingInfo((BuildingTypes) iI).getPrereqAndBonus() == eBonus)
		{
			bStrategic = true;
		}

		for (iJ = 0; iJ < GC.getNUM_BUILDING_PREREQ_OR_BONUSES(); iJ++)
		{
			if (GC.getBuildingInfo((BuildingTypes) iI).getPrereqOrBonuses(iJ) == eBonus)
			{
				bStrategic = true;
			}
		}
	}

	// XXX marble and stone???

	eAttitude = AI_getAttitude(ePlayer);

	if (bStrategic)
	{
		if (eAttitude <= GC.getLeaderHeadInfo(getPersonalityType()).getStrategicBonusRefuseAttitudeThreshold())
		{
			return DENIAL_ATTITUDE;
		}
	}

	if (GC.getBonusInfo(eBonus).getHappiness() > 0)
	{
		if (eAttitude <= GC.getLeaderHeadInfo(getPersonalityType()).getHappinessBonusRefuseAttitudeThreshold())
		{
			return DENIAL_ATTITUDE;
		}
	}

	if (GC.getBonusInfo(eBonus).getHealth() > 0)
	{
		if (eAttitude <= GC.getLeaderHeadInfo(getPersonalityType()).getHealthBonusRefuseAttitudeThreshold())
		{
			return DENIAL_ATTITUDE;
		}
	}

	return NO_DENIAL;
}


int CvPlayerAI::AI_cityTradeVal(CvCity* pCity)
{
	CvPlot* pLoopPlot;
	int iValue;
	int iI;

	FAssert(pCity->getOwnerINLINE() != getID());

	iValue = 300;

	iValue += (pCity->getPopulation() * 50);

	iValue += (pCity->getCultureLevel() * 200);

	iValue += (((((pCity->getPopulation() * 50) + GC.getGameINLINE().getElapsedGameTurns() + 100) * 4) * pCity->plot()->calculateCulturePercent(pCity->getOwnerINLINE())) / 100);

	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		pLoopPlot = plotCity(pCity->getX_INLINE(), pCity->getY_INLINE(), iI);

		if (pLoopPlot != NULL)
		{
			if (pLoopPlot->getBonusType(getTeam()) != NO_BONUS)
			{
				iValue += (AI_bonusVal(pLoopPlot->getBonusType(getTeam())) * 10);
			}
		}
	}

	if (!(pCity->isEverOwned(getID())))
	{
		iValue *= 3;
		iValue /= 2;
	}

	iValue -= (iValue % GC.getDefineINT("DIPLOMACY_VALUE_REMAINDER"));

	if (isHuman())
	{
		return max(iValue, GC.getDefineINT("DIPLOMACY_VALUE_REMAINDER"));
	}
	else
	{
		return iValue;
	}
}


DenialTypes CvPlayerAI::AI_cityTrade(CvCity* pCity, PlayerTypes ePlayer)
{
	CvCity* pNearestCity;

	FAssert(pCity->getOwnerINLINE() == getID());

	if (!(GET_PLAYER(ePlayer).isHuman()))
	{
		if (GET_PLAYER(ePlayer).getTeam() != getTeam())
		{
			if ((pCity->plot()->calculateCulturePercent(ePlayer) == 0) && !(pCity->isEverOwned(ePlayer)) && (GET_PLAYER(ePlayer).getNumCities() > 3))
			{
				if (GET_PLAYER(ePlayer).AI_isFinancialTrouble())
				{
					return DENIAL_UNKNOWN;
				}

				pNearestCity = GC.getMapINLINE().findCity(pCity->getX_INLINE(), pCity->getY_INLINE(), ePlayer, NO_TEAM, true, false, NO_TEAM, NO_DIRECTION, pCity);

				if ((pNearestCity == NULL) || (plotDistance(pCity->getX_INLINE(), pCity->getY_INLINE(), pNearestCity->getX_INLINE(), pNearestCity->getY_INLINE()) > 9))
				{
					return DENIAL_UNKNOWN;
				}
			}
		}
	}

	if (isHuman())
	{
		return NO_DENIAL;
	}

	if (atWar(getTeam(), GET_PLAYER(ePlayer).getTeam()))
	{
		return NO_DENIAL;
	}

	if (GET_PLAYER(ePlayer).getTeam() != getTeam())
	{
		return DENIAL_NEVER;
	}

	if (pCity->calculateCulturePercent(getID()) > 50)
	{
		return DENIAL_TOO_MUCH;
	}

	return NO_DENIAL;
}


int CvPlayerAI::AI_stopTradingTradeVal(TeamTypes eTradeTeam, PlayerTypes ePlayer)
{
	CvDeal* pLoopDeal;
	int iModifier;
	int iValue;
	int iLoop;

	FAssertMsg(ePlayer != getID(), "shouldn't call this function on ourselves");
	FAssertMsg(GET_PLAYER(ePlayer).getTeam() != getTeam(), "shouldn't call this function on ourselves");
	FAssertMsg(eTradeTeam != getTeam(), "shouldn't call this function on ourselves");
	FAssertMsg(GET_TEAM(eTradeTeam).isAlive(), "GET_TEAM(eWarTeam).isAlive is expected to be true");
	FAssertMsg(!atWar(eTradeTeam, GET_PLAYER(ePlayer).getTeam()), "eTeam should be at peace with eWarTeam");

	iValue = (50 + (GC.getGameINLINE().getGameTurn() / 2));
	iValue += (GET_TEAM(eTradeTeam).getNumCities() * 5);

	iModifier = 0;

	switch (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).AI_getAttitude(eTradeTeam))
	{
	case ATTITUDE_FURIOUS:
		break;

	case ATTITUDE_ANNOYED:
		iModifier += 25;
		break;

	case ATTITUDE_CAUTIOUS:
		iModifier += 50;
		break;

	case ATTITUDE_PLEASED:
		iModifier += 100;
		break;

	case ATTITUDE_FRIENDLY:
		iModifier += 200;
		break;

	default:
		FAssert(false);
		break;
	}

	iValue *= max(0, (iModifier + 100));
	iValue /= 100;

	if (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isOpenBorders(eTradeTeam))
	{
		iValue *= 2;
	}

	if (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isDefensivePact(eTradeTeam))
	{
		iValue *= 3;
	}

	for(pLoopDeal = GC.getGameINLINE().firstDeal(&iLoop); pLoopDeal != NULL; pLoopDeal = GC.getGameINLINE().nextDeal(&iLoop))
	{
		if (pLoopDeal->isCancelable(getID()) && !(pLoopDeal->isPeaceDeal()))
		{
			if (GET_PLAYER(pLoopDeal->getFirstPlayer()).getTeam() == GET_PLAYER(ePlayer).getTeam())
			{
				if (pLoopDeal->getLengthSecondTrades() > 0)
				{
					iValue += (GET_PLAYER(pLoopDeal->getFirstPlayer()).AI_dealVal(pLoopDeal->getSecondPlayer(), pLoopDeal->getSecondTrades()) * ((pLoopDeal->getLengthFirstTrades() == 0) ? 2 : 1));
				}
			}

			if (GET_PLAYER(pLoopDeal->getSecondPlayer()).getTeam() == GET_PLAYER(ePlayer).getTeam())
			{
				if (pLoopDeal->getLengthFirstTrades() > 0)
				{
					iValue += (GET_PLAYER(pLoopDeal->getSecondPlayer()).AI_dealVal(pLoopDeal->getFirstPlayer(), pLoopDeal->getFirstTrades()) * ((pLoopDeal->getLengthSecondTrades() == 0) ? 2 : 1));
				}
			}
		}
	}

	if (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isVassal(getTeam()))
	{
		iValue /= 2;
	}

	iValue -= (iValue % GC.getDefineINT("DIPLOMACY_VALUE_REMAINDER"));

	if (isHuman())
	{
		return max(iValue, GC.getDefineINT("DIPLOMACY_VALUE_REMAINDER"));
	}
	else
	{
		return iValue;
	}
}


DenialTypes CvPlayerAI::AI_stopTradingTrade(TeamTypes eTradeTeam, PlayerTypes ePlayer)
{
	AttitudeTypes eAttitude;
	AttitudeTypes eAttitudeThem;
	int iI;

	FAssertMsg(ePlayer != getID(), "shouldn't call this function on ourselves");
	FAssertMsg(GET_PLAYER(ePlayer).getTeam() != getTeam(), "shouldn't call this function on ourselves");
	FAssertMsg(eTradeTeam != getTeam(), "shouldn't call this function on ourselves");
	FAssertMsg(GET_TEAM(eTradeTeam).isAlive(), "GET_TEAM(eTradeTeam).isAlive is expected to be true");
	FAssertMsg(!atWar(getTeam(), eTradeTeam), "should be at peace with eTradeTeam");

	if (isHuman())
	{
		return NO_DENIAL;
	}

	if (GET_TEAM(getTeam()).isVassal(GET_PLAYER(ePlayer).getTeam()))
	{
		return NO_DENIAL;
	}

	if (GET_TEAM(getTeam()).isVassal(eTradeTeam))
	{
		return DENIAL_POWER_THEM;
	}

	eAttitude = GET_TEAM(getTeam()).AI_getAttitude(GET_PLAYER(ePlayer).getTeam());

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() == getTeam())
			{
				if (eAttitude <= GC.getLeaderHeadInfo(GET_PLAYER((PlayerTypes)iI).getPersonalityType()).getStopTradingRefuseAttitudeThreshold())
				{
					return DENIAL_ATTITUDE;
				}
			}
		}
	}

	eAttitudeThem = GET_TEAM(getTeam()).AI_getAttitude(eTradeTeam);

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() == getTeam())
			{
				if (eAttitudeThem > GC.getLeaderHeadInfo(GET_PLAYER((PlayerTypes)iI).getPersonalityType()).getStopTradingThemRefuseAttitudeThreshold())
				{
					return DENIAL_ATTITUDE_THEM;
				}
			}
		}
	}

	return NO_DENIAL;
}


int CvPlayerAI::AI_civicTradeVal(CivicTypes eCivic, PlayerTypes ePlayer)
{
	CivicTypes eBestCivic;
	int iValue;

	iValue = (2 * (getTotalPopulation() + GET_PLAYER(ePlayer).getTotalPopulation())); // XXX

	eBestCivic = GET_PLAYER(ePlayer).AI_bestCivic((CivicOptionTypes)(GC.getCivicInfo(eCivic).getCivicOptionType()));

	if (eBestCivic != NO_CIVIC)
	{
		if (eBestCivic != eCivic)
		{
			iValue += max(0, ((GET_PLAYER(ePlayer).AI_civicValue(eBestCivic) - GET_PLAYER(ePlayer).AI_civicValue(eCivic)) * 2));
		}
	}

	if (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isVassal(getTeam()))
	{
		iValue /= 2;
	}

	iValue -= (iValue % GC.getDefineINT("DIPLOMACY_VALUE_REMAINDER"));

	if (isHuman())
	{
		return max(iValue, GC.getDefineINT("DIPLOMACY_VALUE_REMAINDER"));
	}
	else
	{
		return iValue;
	}
}


DenialTypes CvPlayerAI::AI_civicTrade(CivicTypes eCivic, PlayerTypes ePlayer)
{
	if (isHuman())
	{
		return NO_DENIAL;
	}

	if (GET_TEAM(getTeam()).isVassal(GET_PLAYER(ePlayer).getTeam()))
	{
		return NO_DENIAL;
	}

	if (atWar(getTeam(), GET_PLAYER(ePlayer).getTeam()))
	{
		return NO_DENIAL;
	}

	if (GET_PLAYER(ePlayer).getTeam() == getTeam())
	{
		return NO_DENIAL;
	}

	if (getCivicPercentAnger(eCivic) > 0)
	{
		return DENIAL_ANGER_CIVIC;
	}

	if (GC.getLeaderHeadInfo(getPersonalityType()).getFavoriteCivic() != NO_CIVIC)
	{
		if (isCivic((CivicTypes)(GC.getLeaderHeadInfo(getPersonalityType()).getFavoriteCivic())))
		{
			return DENIAL_FAVORITE_CIVIC;
		}
	}

	if (GC.getCivilizationInfo(getCivilizationType()).getCivilizationInitialCivics(GC.getCivicInfo(eCivic).getCivicOptionType()) == eCivic)
	{
		return DENIAL_JOKING;
	}

	if (AI_getAttitude(ePlayer) <= GC.getLeaderHeadInfo(getPersonalityType()).getAdoptCivicRefuseAttitudeThreshold())
	{
		return DENIAL_ATTITUDE;
	}

	return NO_DENIAL;
}


int CvPlayerAI::AI_religionTradeVal(ReligionTypes eReligion, PlayerTypes ePlayer)
{
	ReligionTypes eBestReligion;
	int iValue;

	iValue = (3 * (getTotalPopulation() + GET_PLAYER(ePlayer).getTotalPopulation())); // XXX

	eBestReligion = GET_PLAYER(ePlayer).AI_bestReligion();

	if (eBestReligion != NO_RELIGION)
	{
		if (eBestReligion != eReligion)
		{
			iValue += max(0, (GET_PLAYER(ePlayer).AI_religionValue(eBestReligion) - GET_PLAYER(ePlayer).AI_religionValue(eReligion)));
		}
	}

	if (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isVassal(getTeam()))
	{
		iValue /= 2;
	}

	iValue -= (iValue % GC.getDefineINT("DIPLOMACY_VALUE_REMAINDER"));

	if (isHuman())
	{
		return max(iValue, GC.getDefineINT("DIPLOMACY_VALUE_REMAINDER"));
	}
	else
	{
		return iValue;
	}
}


DenialTypes CvPlayerAI::AI_religionTrade(ReligionTypes eReligion, PlayerTypes ePlayer)
{
	if (isHuman())
	{
		return NO_DENIAL;
	}

	if (GET_TEAM(getTeam()).isVassal(GET_PLAYER(ePlayer).getTeam()))
	{
		return NO_DENIAL;
	}

	if (atWar(getTeam(), GET_PLAYER(ePlayer).getTeam()))
	{
		return NO_DENIAL;
	}

	if (GET_PLAYER(ePlayer).getTeam() == getTeam())
	{
		return NO_DENIAL;
	}

	if (getStateReligion() != NO_RELIGION)
	{
		if (getHasReligionCount(eReligion) < min((getHasReligionCount(getStateReligion()) - 1), (getNumCities() / 2)))
		{
			return DENIAL_MINORITY_RELIGION;
		}
	}

	if (AI_getAttitude(ePlayer) <= GC.getLeaderHeadInfo(getPersonalityType()).getConvertReligionRefuseAttitudeThreshold())
	{
		return DENIAL_ATTITUDE;
	}

	return NO_DENIAL;
}


bool CvPlayerAI::AI_unitImpassable(UnitTypes eUnit)
{
	int iI;

	for (iI = 0; iI < GC.getNumTerrainInfos(); iI++)
	{
		if (GC.getUnitInfo(eUnit).getTerrainImpassable(iI))
		{
			return true;
		}
	}

	for (iI = 0; iI < GC.getNumFeatureInfos(); iI++)
	{
		if (GC.getUnitInfo(eUnit).getFeatureImpassable(iI))
		{
			return true;
		}
	}

	return false;
}


int CvPlayerAI::AI_unitValue(UnitTypes eUnit, UnitAITypes eUnitAI, CvArea* pArea)
{
	bool bValid;
	int iNeededMissionaries;
	int iCombatValue;
	int iValue;
	int iTempValue;
	int iI;

	FAssertMsg(eUnit != NO_UNIT, "Unit is not assigned a valid value");
	FAssertMsg(eUnitAI != NO_UNITAI, "UnitAI is not assigned a valid value");

	if (GC.getUnitInfo(eUnit).getDomainType() != AI_unitAIDomainType(eUnitAI))
	{
		return 0;
	}

	if (GC.getUnitInfo(eUnit).getNotUnitAIType(eUnitAI))
	{
		return 0;
	}

	bValid = GC.getUnitInfo(eUnit).getUnitAIType(eUnitAI);

	if (!bValid)
	{
		switch (eUnitAI)
		{
		case UNITAI_UNKNOWN:
			break;

		case UNITAI_ANIMAL:
			if (GC.getUnitInfo(eUnit).isAnimal())
			{
				bValid = true;
			}
			break;

		case UNITAI_SETTLE:
			if (GC.getUnitInfo(eUnit).isFound())
			{
				bValid = true;
			}
			break;

		case UNITAI_WORKER:
			for (iI = 0; iI < GC.getNumBuildInfos(); iI++)
			{
				if (GC.getUnitInfo(eUnit).getBuilds(iI))
				{
					bValid = true;
					break;
				}
			}
			break;

		case UNITAI_ATTACK:
			if (GC.getUnitInfo(eUnit).getCombat() > 0)
			{
				if (!(GC.getUnitInfo(eUnit).isOnlyDefensive()))
				{
					bValid = true;
				}
			}
			break;

		case UNITAI_ATTACK_CITY:
			if (GC.getUnitInfo(eUnit).getCombat() > 0)
			{
				if (!(GC.getUnitInfo(eUnit).isOnlyDefensive()))
				{
					if (!(GC.getUnitInfo(eUnit).isNoCapture()))
					{
						bValid = true;
					}
				}
			}
			break;

		case UNITAI_COLLATERAL:
			if (GC.getUnitInfo(eUnit).getCombat() > 0)
			{
				if (!(GC.getUnitInfo(eUnit).isOnlyDefensive()))
				{
					if (GC.getUnitInfo(eUnit).getCollateralDamage() > 0)
					{
						bValid = true;
					}
				}
			}
			break;

		case UNITAI_PILLAGE:
			if (GC.getUnitInfo(eUnit).getCombat() > 0)
			{
				if (!(GC.getUnitInfo(eUnit).isOnlyDefensive()))
				{
					bValid = true;
				}
			}
			break;

		case UNITAI_RESERVE:
			if (GC.getUnitInfo(eUnit).getCombat() > 0)
			{
				if (!(GC.getUnitInfo(eUnit).isOnlyDefensive()))
				{
					bValid = true;
				}
			}
			break;

		case UNITAI_COUNTER:
			if (GC.getUnitInfo(eUnit).getCombat() > 0)
			{
				if (!(GC.getUnitInfo(eUnit).isOnlyDefensive()))
				{
					if (GC.getUnitInfo(eUnit).getInterceptionProbability() > 0)
					{
						bValid = true;
					}

					if (!bValid)
					{
						for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
						{
							if (GC.getUnitInfo(eUnit).getUnitClassAttackModifier(iI) > 0)
							{
								bValid = true;
								break;
							}

							if (GC.getUnitInfo(eUnit).getTargetUnitClass(iI))
							{
								bValid = true;
								break;
							}
						}
					}

					if (!bValid)
					{
						for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
						{
							if (GC.getUnitInfo(eUnit).getUnitCombatModifier(iI) > 0)
							{
								bValid = true;
								break;
							}

							if (GC.getUnitInfo(eUnit).getTargetUnitCombat(iI))
							{
								bValid = true;
								break;
							}
						}
					}

					if (!bValid)
					{

						for (iI = 0; iI < GC.getNumUnitInfos(); iI++)
						{
							int iUnitClass = GC.getUnitInfo(eUnit).getUnitClassType();
							if (NO_UNITCLASS != iUnitClass && GC.getUnitInfo((UnitTypes)iI).getDefenderUnitClass(iUnitClass))
							{
								bValid = true;
								break;
							}

							int iUnitCombat = GC.getUnitInfo(eUnit).getUnitCombatType();
							if (NO_UNITCOMBAT !=  iUnitCombat && GC.getUnitInfo((UnitTypes)iI).getDefenderUnitCombat(iUnitCombat))
							{
								bValid = true;
								break;
							}
						}
					}
				}
			}
			break;

		case UNITAI_CITY_DEFENSE:
			if (GC.getUnitInfo(eUnit).getCombat() > 0)
			{
				if (!(GC.getUnitInfo(eUnit).isNoDefensiveBonus()))
				{
					if (GC.getUnitInfo(eUnit).getCityDefenseModifier() > 0)
					{
						bValid = true;
					}
				}
			}
			break;

		case UNITAI_CITY_COUNTER:
			if (GC.getUnitInfo(eUnit).getCombat() > 0)
			{
				if (!(GC.getUnitInfo(eUnit).isNoDefensiveBonus()))
				{
					if (GC.getUnitInfo(eUnit).getInterceptionProbability() > 0)
					{
						bValid = true;
					}

					if (!bValid)
					{
						for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
						{
							if (GC.getUnitInfo(eUnit).getUnitClassDefenseModifier(iI) > 0)
							{
								bValid = true;
								break;
							}
						}
					}

					if (!bValid)
					{
						for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
						{
							if (GC.getUnitInfo(eUnit).getUnitCombatModifier(iI) > 0)
							{
								bValid = true;
								break;
							}
						}
					}
				}
			}
			break;

		case UNITAI_CITY_SPECIAL:
			break;

		case UNITAI_EXPLORE:
			if (GC.getUnitInfo(eUnit).getCombat() > 0)
			{
				if (!AI_unitImpassable(eUnit))
				{
					bValid = true;
				}
			}
			break;

		case UNITAI_MISSIONARY:
			if (pArea != NULL)
			{
				for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
				{
					if (GC.getUnitInfo(eUnit).getReligionSpreads((ReligionTypes)iI))
					{
						iNeededMissionaries = AI_neededMissionaries(pArea, ((ReligionTypes)iI));

						if (iNeededMissionaries > 0)
						{
							if (iNeededMissionaries > countReligionSpreadUnits(pArea, ((ReligionTypes)iI)))
							{
								bValid = true;
								break;
							}
						}
					}
				}
			}
			break;

		case UNITAI_PROPHET:
		case UNITAI_ARTIST:
		case UNITAI_SCIENTIST:
		case UNITAI_GENERAL:
		case UNITAI_MERCHANT:
		case UNITAI_ENGINEER:
		case UNITAI_SPY:
			break;

		case UNITAI_ICBM:
			if (GC.getUnitInfo(eUnit).getNukeRange() != -1)
			{
				bValid = true;
			}
			break;

		case UNITAI_WORKER_SEA:
			for (iI = 0; iI < GC.getNumBuildInfos(); iI++)
			{
				if (GC.getUnitInfo(eUnit).getBuilds(iI))
				{
					bValid = true;
					break;
				}
			}
			break;

		case UNITAI_ATTACK_SEA:
			if (GC.getUnitInfo(eUnit).getCombat() > 0)
			{
				if (GC.getUnitInfo(eUnit).getCargoSpace() == 0)
				{
					if (!(GC.getUnitInfo(eUnit).isInvisible()) && (GC.getUnitInfo(eUnit).getInvisibleType() == NO_INVISIBLE))
					{
						bValid = true;
					}
				}
			}
			break;

		case UNITAI_RESERVE_SEA:
			if (GC.getUnitInfo(eUnit).getCombat() > 0)
			{
				if (GC.getUnitInfo(eUnit).getCargoSpace() == 0)
				{
					bValid = true;
				}
			}
			break;

		case UNITAI_ESCORT_SEA:
			if (GC.getUnitInfo(eUnit).getCombat() > 0)
			{
				if (GC.getUnitInfo(eUnit).getCargoSpace() == 0)
				{
					if (!AI_unitImpassable(eUnit))
					{
						bValid = true;
					}
				}
			}
			break;

		case UNITAI_EXPLORE_SEA:
			if (GC.getUnitInfo(eUnit).getCombat() > 0)
			{
				if (GC.getUnitInfo(eUnit).getCargoSpace() <= 1)
				{
					if (!AI_unitImpassable(eUnit))
					{
						bValid = true;
					}
				}
			}
			break;

		case UNITAI_ASSAULT_SEA:
		case UNITAI_SETTLER_SEA:
			if (GC.getUnitInfo(eUnit).getCargoSpace() > 0)
			{
				if (GC.getUnitInfo(eUnit).getSpecialCargo() == NO_SPECIALUNIT)
				{
					bValid = true;
				}
			}
			break;

		case UNITAI_MISSIONARY_SEA:
		case UNITAI_SPY_SEA:
			break;

		case UNITAI_CARRIER_SEA:
			if (GC.getUnitInfo(eUnit).getCargoSpace() > 0)
			{
				if (GC.getUnitInfo(eUnit).getDomainCargo() == DOMAIN_AIR)
				{
					bValid = true;
				}
			}
			break;

		case UNITAI_ATTACK_AIR:
			if (GC.getUnitInfo(eUnit).getAirCombat() > 0)
			{
				bValid = true;
			}
			break;

		case UNITAI_DEFENSE_AIR:
			if (GC.getUnitInfo(eUnit).getInterceptionProbability() > 0)
			{
				bValid = true;
			}
			break;

		case UNITAI_CARRIER_AIR:
			if (GC.getUnitInfo(eUnit).getAirCombat() > 0)
			{
				if (GC.getUnitInfo(eUnit).getInterceptionProbability() > 0)
				{
					bValid = true;
				}
			}
			break;

		default:
			FAssert(false);
			break;
		}
	}

	if (!bValid)
	{
		return 0;
	}

	iCombatValue = GC.getGameINLINE().AI_combatValue(eUnit);

	iValue = 1;

	iValue += GC.getUnitInfo(eUnit).getAIWeight();

	switch (eUnitAI)
	{
	case UNITAI_UNKNOWN:
	case UNITAI_ANIMAL:
		break;

	case UNITAI_SETTLE:
		iValue += (GC.getUnitInfo(eUnit).getMoves() * 100);
		break;

	case UNITAI_WORKER:
		for (iI = 0; iI < GC.getNumBuildInfos(); iI++)
		{
			if (GC.getUnitInfo(eUnit).getBuilds(iI))
			{
				iValue += 50;
			}
		}
		iValue += (GC.getUnitInfo(eUnit).getMoves() * 100);
		break;

	case UNITAI_ATTACK:
		iValue += iCombatValue;
		iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getMoves()) / 2);
		iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getWithdrawalProbability()) / 100);
		break;

	case UNITAI_ATTACK_CITY:
		iTempValue = ((iCombatValue * iCombatValue) / 100) + (iCombatValue / 2);
		iValue += iTempValue;
		if (GC.getUnitInfo(eUnit).isNoDefensiveBonus())
		{
			iValue -= iTempValue / 2;
		}
		if (GC.getUnitInfo(eUnit).isFirstStrikeImmune())
		{
			iValue += (iTempValue * 8) / 100;
		}		
		iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getCityAttackModifier()) / 100);
		iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getCollateralDamage()) / 400);
		iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getMoves()) / 4);
		iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getWithdrawalProbability()) / 100);
		iValue += (iCombatValue * GC.getUnitInfo(eUnit).getBombardRate() * 4);
		break;

	case UNITAI_COLLATERAL:
		iValue += iCombatValue;
		iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getCollateralDamage()) / 50);
		iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getMoves()) / 4);
		iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getWithdrawalProbability()) / 25);
		iValue -= ((iCombatValue * GC.getUnitInfo(eUnit).getCityAttackModifier()) / 100);
		break;

	case UNITAI_PILLAGE:
		iValue += iCombatValue;
		iValue += (iCombatValue * GC.getUnitInfo(eUnit).getMoves());
		break;

	case UNITAI_RESERVE:
		iValue += iCombatValue;
		iValue -= ((iCombatValue * GC.getUnitInfo(eUnit).getCollateralDamage()) / 200);
		for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
		{
			int iCombatModifier = GC.getUnitInfo(eUnit).getUnitCombatModifier(iI);
			iCombatModifier = (iCombatModifier < 40) ? iCombatModifier : (40 + (iCombatModifier - 40) / 2);
			iValue += ((iCombatValue * iCombatModifier) / 100);
		}
		iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getMoves()) / 2);
		break;

	case UNITAI_COUNTER:
		iValue += (iCombatValue / 2);
		for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
		{
			iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getUnitClassAttackModifier(iI)) / 100);
			iValue += ((iCombatValue * (GC.getUnitInfo(eUnit).getTargetUnitClass(iI) ? 50 : 0)) / 100);
		}
		for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
		{
			int iCombatModifier = GC.getUnitInfo(eUnit).getUnitCombatModifier(iI);
			iCombatModifier = (iCombatModifier < 40) ? iCombatModifier : (40 + (iCombatModifier - 40) / 2);
			iValue += ((iCombatValue * iCombatModifier) / 100);
			iValue += ((iCombatValue * (GC.getUnitInfo(eUnit).getTargetUnitCombat(iI) ? 50 : 0)) / 100);
		}
		for (iI = 0; iI < GC.getNumUnitInfos(); iI++)
		{
			int eUnitClass = GC.getUnitInfo(eUnit).getUnitClassType();
			if (NO_UNITCLASS != eUnitClass && GC.getUnitInfo((UnitTypes)iI).getDefenderUnitClass(eUnitClass))
			{
				iValue += (50 * iCombatValue) / 100;
			}

			int eUnitCombat = GC.getUnitInfo(eUnit).getUnitCombatType();
			if (NO_UNITCOMBAT != eUnitCombat && GC.getUnitInfo((UnitTypes)iI).getDefenderUnitCombat(eUnitCombat))
			{
				iValue += (50 * iCombatValue) / 100;
			}
		}

		iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getMoves()) / 2);
		iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getWithdrawalProbability()) / 100);
		iValue += (GC.getUnitInfo(eUnit).getInterceptionProbability() * 2);
		break;

	case UNITAI_CITY_DEFENSE:
		iValue += ((iCombatValue * 2) / 3);
		iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getCityDefenseModifier()) / 100);
        for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
		{
			iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getUnitCombatModifier(iI)) / 200);
		}
		break;

	case UNITAI_CITY_COUNTER:
	case UNITAI_CITY_SPECIAL:
		iValue += (iCombatValue / 2);
		iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getCityDefenseModifier()) / 100);
		for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
		{
			iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getUnitClassDefenseModifier(iI)) / 100);
			iValue += ((iCombatValue * (GC.getUnitInfo(eUnit).getDefenderUnitClass(iI) ? 50 : 0)) / 100);
		}
		for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
		{
			iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getUnitCombatModifier(iI)) / 100);
			iValue += ((iCombatValue * (GC.getUnitInfo(eUnit).getDefenderUnitCombat(iI) ? 50 : 0)) / 100);
		}
		iValue += (GC.getUnitInfo(eUnit).getInterceptionProbability() * 3);
		break;

	case UNITAI_EXPLORE:
		iValue += (iCombatValue / 2);
		iValue += (GC.getUnitInfo(eUnit).getMoves() * 200);
		if (GC.getUnitInfo(eUnit).isNoBadGoodies())
		{
			iValue += 100;
		}
		break;

	case UNITAI_MISSIONARY:
		iValue += (GC.getUnitInfo(eUnit).getMoves() * 100);
		if (getStateReligion() != NO_RELIGION)
		{
			if (GC.getUnitInfo(eUnit).getReligionSpreads(getStateReligion()))
			{
				iValue += 75;
			}
		}
		for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
		{
			if (GC.getUnitInfo(eUnit).getReligionSpreads((ReligionTypes)iI) && hasHolyCity((ReligionTypes)iI))
			{
				iValue += 80;
				break;
			}
		}
		if (AI_isDoStrategy(AI_STRATEGY_CULTURE2))
		{
		    int iTempValue = 0;
		    for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
		    {
                if (GC.getUnitInfo(eUnit).getReligionSpreads((ReligionTypes)iI))
                {
                    iTempValue += (50 * getNumCities()) / (1 + getHasReligionCount((ReligionTypes)iI));
                }
		    }
		    iValue += iTempValue;		    
		}
		break;

	case UNITAI_PROPHET:
	case UNITAI_ARTIST:
	case UNITAI_SCIENTIST:
	case UNITAI_GENERAL:
	case UNITAI_MERCHANT:
	case UNITAI_ENGINEER:
		break;

	case UNITAI_SPY:
		iValue += (GC.getUnitInfo(eUnit).getMoves() * 100);
		if (GC.getUnitInfo(eUnit).isSabotage())
		{
			iValue += 50;
		}
		if (GC.getUnitInfo(eUnit).isDestroy())
		{
			iValue += 50;
		}
		if (GC.getUnitInfo(eUnit).isCounterSpy())
		{
			iValue += 100;
		}
		break;

	case UNITAI_ICBM:
		if (GC.getUnitInfo(eUnit).getNukeRange() != -1)
		{
			iValue += (GC.getUnitInfo(eUnit).getNukeRange() * 400);
		}
		break;

	case UNITAI_WORKER_SEA:
		for (iI = 0; iI < GC.getNumBuildInfos(); iI++)
		{
			if (GC.getUnitInfo(eUnit).getBuilds(iI))
			{
				iValue += 50;
			}
		}
		iValue += (GC.getUnitInfo(eUnit).getMoves() * 100);
		break;

	case UNITAI_ATTACK_SEA:
		iValue += iCombatValue;
		iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getMoves()) / 2);
		iValue += (GC.getUnitInfo(eUnit).getBombardRate() * 4);
		break;

	case UNITAI_RESERVE_SEA:
		iValue += iCombatValue;
		iValue += (iCombatValue * GC.getUnitInfo(eUnit).getMoves());
		break;

	case UNITAI_ESCORT_SEA:
		iValue += iCombatValue;
		iValue += (iCombatValue * GC.getUnitInfo(eUnit).getMoves());
		iValue += (GC.getUnitInfo(eUnit).getInterceptionProbability() * 3);
		if (GC.getUnitInfo(eUnit).getSeeInvisibleType() != NO_INVISIBLE)
		{
			iValue += 200;
		}
		break;

	case UNITAI_EXPLORE_SEA:
		iValue += (iCombatValue / 2);
		iValue += (GC.getUnitInfo(eUnit).getMoves() * 300);
		break;

	case UNITAI_ASSAULT_SEA:
	case UNITAI_SETTLER_SEA:
	case UNITAI_MISSIONARY_SEA:
	case UNITAI_SPY_SEA:
		iValue += (iCombatValue / 2);
		iValue += (GC.getUnitInfo(eUnit).getMoves() * 200);
		iValue += (GC.getUnitInfo(eUnit).getCargoSpace() * 300);
		break;

	case UNITAI_CARRIER_SEA:
		iValue += iCombatValue;
		iValue += (GC.getUnitInfo(eUnit).getMoves() * 50);
		iValue += (GC.getUnitInfo(eUnit).getCargoSpace() * 400);
		break;

	case UNITAI_ATTACK_AIR:
		iValue += iCombatValue;
		iValue += (GC.getUnitInfo(eUnit).getAirRange() * 200);
		break;

	case UNITAI_DEFENSE_AIR:
		iValue += (iCombatValue / 3);
		iValue += (GC.getUnitInfo(eUnit).getInterceptionProbability() * 3);
		iValue += (GC.getUnitInfo(eUnit).getAirRange() * 200);
		break;

	case UNITAI_CARRIER_AIR:
		iValue += (iCombatValue / 2);
		iValue += (GC.getUnitInfo(eUnit).getInterceptionProbability() * 2);
		iValue += (GC.getUnitInfo(eUnit).getAirRange() * 200);
		break;

	default:
		FAssert(false);
		break;
	}



	return max(0, iValue);
}


int CvPlayerAI::AI_totalUnitAIs(UnitAITypes eUnitAI)
{
	return (AI_getNumTrainAIUnits(eUnitAI) + AI_getNumAIUnits(eUnitAI));
}


int CvPlayerAI::AI_totalAreaUnitAIs(CvArea* pArea, UnitAITypes eUnitAI)
{
	return (pArea->getNumTrainAIUnits(getID(), eUnitAI) + pArea->getNumAIUnits(getID(), eUnitAI));
}


int CvPlayerAI::AI_totalWaterAreaUnitAIs(CvArea* pArea, UnitAITypes eUnitAI)
{
	CvCity* pLoopCity;
	int iCount;
	int iLoop;
	int iI;

	iCount = AI_totalAreaUnitAIs(pArea, eUnitAI);

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
			{
				if (pLoopCity->waterArea() == pArea)
				{
					iCount += pLoopCity->plot()->plotCount(PUF_isUnitAIType, eUnitAI, -1, getID());

					if (pLoopCity->getOwnerINLINE() == getID())
					{
						iCount += pLoopCity->getNumTrainUnitAI(eUnitAI);
					}
				}
			}
		}
	}


	return iCount;
}


int CvPlayerAI::AI_countCargoSpace(UnitAITypes eUnitAI)
{
	CvUnit* pLoopUnit;
	int iCount;
	int iLoop;

	iCount = 0;

	for(pLoopUnit = firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = nextUnit(&iLoop))
	{
		if (pLoopUnit->AI_getUnitAIType() == eUnitAI)
		{
			iCount += pLoopUnit->cargoSpace();
		}
	}

	return iCount;
}


int CvPlayerAI::AI_neededExplorers(CvArea* pArea)
{
	FAssert(pArea != NULL);

	if (pArea->isWater())
	{
		return min((pArea->getNumUnrevealedTiles(getTeam()) / 800), min(2, ((getNumCities() / 2) + 1)));
	}
	else
	{
		return min((pArea->getNumUnrevealedTiles(getTeam()) / 200), min(3, ((getNumCities() / 3) + 2)));
	}
}


int CvPlayerAI::AI_neededWorkers(CvArea* pArea)
{
	CvCity* pLoopCity;
	int iCount;
	int iLoop;

	iCount = countUnimprovedBonuses(pArea) * 2;

	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		iCount += pLoopCity->AI_countBestBuilds(pArea);
	}

	if (getBestRoute() != NO_ROUTE)
	{
		iCount += pArea->getCitiesPerPlayer(getID());
	}

	if (iCount > 0)
	{
	    iCount += 1;
		iCount /= 3;
		iCount = min(iCount, 3 * pArea->getCitiesPerPlayer(getID()));
		iCount = min(iCount, (1 + getTotalPopulation()) / 2);
	}

	return max(1, iCount);
}


int CvPlayerAI::AI_neededMissionaries(CvArea* pArea, ReligionTypes eReligion)
{
    PROFILE_FUNC();
	int iCount;
	bool bHoly, bState, bHolyState;
	bool bCultureVictory = AI_isDoStrategy(AI_STRATEGY_CULTURE2);

	bHoly = hasHolyCity(eReligion);
	bState = (getStateReligion() == eReligion);
	bHolyState = ((getStateReligion() != NO_RELIGION) && hasHolyCity(getStateReligion()));

	iCount = 0;

    //internal spread.
    if (bCultureVictory || bState || bHoly)
    {
        iCount = max(iCount, (pArea->getCitiesPerPlayer(getID()) - pArea->countHasReligion(eReligion, getID())));
        if (iCount > 0)
        {
            if (!bCultureVictory)
            {
                iCount = max(1, iCount / (bHoly ? 2 : 4));
            }
            return iCount;
        }
    }

    //external spread.
    if ((bHoly && bState) || (bHoly && !bHolyState && (getStateReligion() != NO_RELIGION)))
    {
        iCount += ((pArea->getNumCities() * 2) - (pArea->countHasReligion(eReligion) * 3));
        iCount /= 8;
        
        iCount = max(0, iCount);
        
        if (AI_isPrimaryArea(pArea))
        {
            iCount++;
        }
    }


	return iCount;
}


int CvPlayerAI::AI_adjacentPotentialAttackers(CvPlot* pPlot, bool bTestCanMove)
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pLoopPlot;
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
	{
		pLoopPlot = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), ((DirectionTypes)iI));

		if (pLoopPlot != NULL)
		{
			if (pLoopPlot->area() == pPlot->area())
			{
				pUnitNode = pLoopPlot->headUnitNode();

				while (pUnitNode != NULL)
				{
					pLoopUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);

					if (pLoopUnit->getOwnerINLINE() == getID())
					{
						if (pLoopUnit->getDomainType() == ((pPlot->isWater()) ? DOMAIN_SEA : DOMAIN_LAND))
						{
							if (pLoopUnit->canAttack())
							{
								if (!bTestCanMove || pLoopUnit->canMove())
								{
									if (!(pLoopUnit->AI_isCityAIType()))
									{
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

	return iCount;
}


int CvPlayerAI::AI_totalMissionAIs(MissionAITypes eMissionAI, CvSelectionGroup* pSkipSelectionGroup)
{
	PROFILE_FUNC();

	CvSelectionGroup* pLoopSelectionGroup;
	int iCount;
	int iLoop;

	iCount = 0;

	for(pLoopSelectionGroup = firstSelectionGroup(&iLoop); pLoopSelectionGroup; pLoopSelectionGroup = nextSelectionGroup(&iLoop))
	{
		if (pLoopSelectionGroup != pSkipSelectionGroup)
		{
			if (pLoopSelectionGroup->AI_getMissionAIType() == eMissionAI)
			{
				iCount += pLoopSelectionGroup->getNumUnits();
			}
		}
	}

	return iCount;
}


int CvPlayerAI::AI_areaMissionAIs(CvArea* pArea, MissionAITypes eMissionAI, CvSelectionGroup* pSkipSelectionGroup)
{
	PROFILE_FUNC();

	CvSelectionGroup* pLoopSelectionGroup;
	CvPlot* pMissionPlot;
	int iCount;
	int iLoop;

	iCount = 0;

	for(pLoopSelectionGroup = firstSelectionGroup(&iLoop); pLoopSelectionGroup; pLoopSelectionGroup = nextSelectionGroup(&iLoop))
	{
		if (pLoopSelectionGroup != pSkipSelectionGroup)
		{
			if (pLoopSelectionGroup->AI_getMissionAIType() == eMissionAI)
			{
				pMissionPlot = pLoopSelectionGroup->AI_getMissionAIPlot();

				if (pMissionPlot != NULL)
				{
					if (pMissionPlot->area() == pArea)
					{
						iCount += pLoopSelectionGroup->getNumUnits();
					}
				}
			}
		}
	}

	return iCount;
}

int CvPlayerAI::AI_plotTargetMissionAIs(CvPlot* pPlot, MissionAITypes eMissionAI, CvSelectionGroup* pSkipSelectionGroup, int iRange)
{
	int iClosestTargetRange;
	return AI_plotTargetMissionAIs(pPlot, &eMissionAI, 1, iClosestTargetRange, pSkipSelectionGroup, iRange);
}

int CvPlayerAI::AI_plotTargetMissionAIs(CvPlot* pPlot, MissionAITypes eMissionAI, int& iClosestTargetRange, CvSelectionGroup* pSkipSelectionGroup, int iRange)
{
	return AI_plotTargetMissionAIs(pPlot, &eMissionAI, 1, iClosestTargetRange, pSkipSelectionGroup, iRange);
}

int CvPlayerAI::AI_plotTargetMissionAIs(CvPlot* pPlot, MissionAITypes* aeMissionAI, int iMissionAICount, int& iClosestTargetRange, CvSelectionGroup* pSkipSelectionGroup, int iRange)
{
	PROFILE_FUNC();

	int iCount = 0;
	iClosestTargetRange = MAX_INT;

	// only valid to pass NO_MISSIONAI as the first and only MissionAI
	bool bIsAllMissionAIs = (iMissionAICount == 1 && aeMissionAI[0] == NO_MISSIONAI);

	int iLoop;
	for(CvSelectionGroup* pLoopSelectionGroup = firstSelectionGroup(&iLoop); pLoopSelectionGroup; pLoopSelectionGroup = nextSelectionGroup(&iLoop))
	{
		if (pLoopSelectionGroup != pSkipSelectionGroup)
		{
			MissionAITypes eGroupMissionAI = pLoopSelectionGroup->AI_getMissionAIType();
			for (int iMissionAIIndex = 0; iMissionAIIndex < iMissionAICount; iMissionAIIndex++)
			{
				if (bIsAllMissionAIs || eGroupMissionAI == aeMissionAI[iMissionAIIndex])
				{
					CvPlot* pMissionPlot = pLoopSelectionGroup->AI_getMissionAIPlot();

					if (pMissionPlot != NULL)
					{
						int iDistance = stepDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pMissionPlot->getX_INLINE(), pMissionPlot->getY_INLINE());
						if (iDistance <= iRange)
						{
							iCount += pLoopSelectionGroup->getNumUnits();

						}

						if (iDistance < iClosestTargetRange)
						{
							iClosestTargetRange = iDistance;
						}
					}
				}
			}
		}
	}

	return iCount;
}


int CvPlayerAI::AI_unitTargetMissionAIs(CvUnit* pUnit, MissionAITypes eMissionAI, CvSelectionGroup* pSkipSelectionGroup)
{
	return AI_unitTargetMissionAIs(pUnit, &eMissionAI, 1, pSkipSelectionGroup);
}

int CvPlayerAI::AI_unitTargetMissionAIs(CvUnit* pUnit, MissionAITypes* aeMissionAI, int iMissionAICount, CvSelectionGroup* pSkipSelectionGroup)
{
	PROFILE_FUNC();

	CvSelectionGroup* pLoopSelectionGroup;
	int iCount;
	int iLoop;

	iCount = 0;

	for(pLoopSelectionGroup = firstSelectionGroup(&iLoop); pLoopSelectionGroup; pLoopSelectionGroup = nextSelectionGroup(&iLoop))
	{
		if (pLoopSelectionGroup != pSkipSelectionGroup)
		{
			if (pLoopSelectionGroup->AI_getMissionAIUnit() == pUnit)
			{
				// only valid to pass NO_MISSIONAI as the first MissionAI
				if (aeMissionAI[0] == NO_MISSIONAI)
				{
					iCount += pLoopSelectionGroup->getNumUnits();
				}
				else
				{
					MissionAITypes eGroupMissionAI = pLoopSelectionGroup->AI_getMissionAIType();
					for (int iMissionAIIndex = 0; iMissionAIIndex < iMissionAICount; iMissionAIIndex++)
					{
						if (eGroupMissionAI == aeMissionAI[iMissionAIIndex])
						{
							iCount += pLoopSelectionGroup->getNumUnits();
						}
					}
				}
			}
		}
	}

	return iCount;
}

int CvPlayerAI::AI_wakePlotTargetMissionAIs(CvPlot* pPlot, MissionAITypes eMissionAI, CvSelectionGroup* pSkipSelectionGroup)
{
	PROFILE_FUNC();

	FAssert(pPlot != NULL);
	
	int iCount = 0;

	int iLoop;
	for(CvSelectionGroup* pLoopSelectionGroup = firstSelectionGroup(&iLoop); pLoopSelectionGroup; pLoopSelectionGroup = nextSelectionGroup(&iLoop))
	{
		if (pLoopSelectionGroup != pSkipSelectionGroup)
		{
			MissionAITypes eGroupMissionAI = pLoopSelectionGroup->AI_getMissionAIType();
			if (eMissionAI == NO_MISSIONAI || eMissionAI == eGroupMissionAI)
			{
				CvPlot* pMissionPlot = pLoopSelectionGroup->AI_getMissionAIPlot();
				if (pMissionPlot != NULL && pMissionPlot == pPlot)
				{
					iCount += pLoopSelectionGroup->getNumUnits();
					pLoopSelectionGroup->setActivityType(ACTIVITY_AWAKE);
				}
			}
		}
	}

	return iCount;
}

CivicTypes CvPlayerAI::AI_bestCivic(CivicOptionTypes eCivicOption)
{
	CivicTypes eBestCivic;
	int iValue;
	int iBestValue;
	int iI;

	iBestValue = MIN_INT;
	eBestCivic = NO_CIVIC;

	for (iI = 0; iI < GC.getNumCivicInfos(); iI++)
	{
		if (GC.getCivicInfo((CivicTypes)iI).getCivicOptionType() == eCivicOption)
		{
			if (canDoCivics((CivicTypes)iI))
			{
				iValue = AI_civicValue((CivicTypes)iI);

				if (isCivic((CivicTypes)iI))
				{
					if (getMaxAnarchyTurns() > 0)
					{
						iValue *= 6;
						iValue /= 5;
					}
					else
					{
						iValue *= 16;
						iValue /= 15;
					}
				}

				if (iValue > iBestValue)
				{
					iBestValue = iValue;
					eBestCivic = ((CivicTypes)iI);
				}
			}
		}
	}

	return eBestCivic;
}


int CvPlayerAI::AI_civicValue(CivicTypes eCivic)
{
	PROFILE_FUNC();

	bool bWarPlan;
	int iConnectedForeignCities;
	int iTotalReligonCount;
	int iHighestReligionCount;
	int iWarmongerPercent;
	int iHappiness;
	int iValue;
	int iTempValue;
	int iI, iJ;
	
	bool bCultureVictory3 = AI_isDoStrategy(AI_STRATEGY_CULTURE3);
	bool bCultureVictory2 = AI_isDoStrategy(AI_STRATEGY_CULTURE2);

	FAssertMsg(eCivic < GC.getNumCivicInfos(), "eCivic is expected to be within maximum bounds (invalid Index)");
	FAssertMsg(eCivic >= 0, "eCivic is expected to be non-negative (invalid Index)");

	bWarPlan = (GET_TEAM(getTeam()).getAnyWarPlanCount(true) > 0);

	iConnectedForeignCities = countPotentialForeignTradeCitiesConnected();
	iTotalReligonCount = countTotalHasReligion();
	iHighestReligionCount = findHighestHasReligionCount();
	iWarmongerPercent = 25000 / max(100, (100 + GC.getLeaderHeadInfo(getPersonalityType()).getMaxWarRand()));

	iValue = (getNumCities() * 6);

	iValue += (GC.getCivicInfo(eCivic).getAIWeight() * getNumCities());

	iValue += (getCivicPercentAnger(eCivic) / 10);

	iValue += -(GC.getCivicInfo(eCivic).getAnarchyLength() * getNumCities());

	iValue += -(getSingleCivicUpkeep(eCivic, true));

	iValue += ((GC.getCivicInfo(eCivic).getGreatPeopleRateModifier() * getNumCities()) / 10);
	iValue += ((GC.getCivicInfo(eCivic).getGreatGeneralRateModifier() * getNumMilitaryUnits()) / 50);
	iValue += ((GC.getCivicInfo(eCivic).getDomesticGreatGeneralRateModifier() * getNumMilitaryUnits()) / 100);
	iValue += -((GC.getCivicInfo(eCivic).getDistanceMaintenanceModifier() * max(0, (getNumCities() - 3))) / 8);
	iValue += -((GC.getCivicInfo(eCivic).getNumCitiesMaintenanceModifier() * max(0, (getNumCities() - 3))) / 8);
	iValue += ((GC.getCivicInfo(eCivic).getExtraHealth() * getTotalPopulation()) / 8);
	iValue += (GC.getCivicInfo(eCivic).getFreeExperience() * getNumCities() * (bWarPlan ? 8 : 5) * iWarmongerPercent) / 100;
	iValue += ((GC.getCivicInfo(eCivic).getWorkerSpeedModifier() * AI_getNumAIUnits(UNITAI_WORKER)) / 18);
	iValue += ((GC.getCivicInfo(eCivic).getImprovementUpgradeRateModifier() * getNumCities()) / 50);
	iValue += (GC.getCivicInfo(eCivic).getMilitaryProductionModifier() * getNumCities() * iWarmongerPercent) / (bWarPlan ? 300 : 500 );
	iValue += (GC.getCivicInfo(eCivic).getBaseFreeUnits() / 2);
	iValue += (GC.getCivicInfo(eCivic).getBaseFreeMilitaryUnits() / 3);
	iValue += ((GC.getCivicInfo(eCivic).getFreeUnitsPopulationPercent() * getTotalPopulation()) / 200);
	iValue += ((GC.getCivicInfo(eCivic).getFreeMilitaryUnitsPopulationPercent() * getTotalPopulation()) / 300);
	iValue += -(GC.getCivicInfo(eCivic).getGoldPerUnit() * getNumUnits());
	iValue += -(GC.getCivicInfo(eCivic).getGoldPerMilitaryUnit() * getNumMilitaryUnits() * iWarmongerPercent) / 200;

	iValue += ((GC.getCivicInfo(eCivic).getHappyPerMilitaryUnit() * getNumCities()) * 8);
	//iValue += ((GC.getCivicInfo(eCivic).isMilitaryFoodProduction()) ? 0 : 0);
	iValue += (getWorldSizeMaxConscript(eCivic) * ((bWarPlan) ? (20 + getNumCities()) : ((8 + getNumCities()) / 2)));
	iValue += ((GC.getCivicInfo(eCivic).isNoUnhealthyPopulation()) ? (getTotalPopulation() / 3) : 0);
	if (bWarPlan)
	{
		iValue += ((GC.getCivicInfo(eCivic).getExpInBorderModifier() * getNumMilitaryUnits()) / 200);
	}
	iValue += ((GC.getCivicInfo(eCivic).isBuildingOnlyHealthy()) ? (getNumCities() * 3) : 0);
	iValue += (GC.getCivicInfo(eCivic).getLargestCityHappiness() * min(getNumCities(), GC.getWorldInfo(GC.getMapINLINE().getWorldSize()).getTargetNumCities()) * 4);
	iValue += -((GC.getCivicInfo(eCivic).getWarWearinessModifier() * getNumCities()) / ((bWarPlan) ? 10 : 50));
	iValue += (GC.getCivicInfo(eCivic).getFreeSpecialist() * getNumCities() * 12);
	iValue += ((GC.getCivicInfo(eCivic).getTradeRoutes() * max(0, iConnectedForeignCities - getNumCities() * 3) * 6) + (getNumCities() * 2));
	iValue += -((GC.getCivicInfo(eCivic).isNoForeignTrade()) ? (iConnectedForeignCities * 3) : 0);
	iValue += ((min(20, GC.getCivicInfo(eCivic).getCivicPercentAnger()) * (GC.getGameINLINE().getNumCities() - getNumCities())) / 10) / (bCultureVictory3 ? 5 : 1);
	iValue += (GC.getCivicInfo(eCivic).getNonStateReligionHappiness() * (iTotalReligonCount - iHighestReligionCount) * 5);

	if (GC.getCivicInfo(eCivic).isStateReligion())
	{
		if (iHighestReligionCount > 0)
		{
			iValue += iHighestReligionCount;

			iValue += ((GC.getCivicInfo(eCivic).isNoNonStateReligionSpread()) ? ((getNumCities() - iHighestReligionCount) * 2) : 0);
			iValue += (GC.getCivicInfo(eCivic).getStateReligionHappiness() * iHighestReligionCount * 4);
			iValue += ((GC.getCivicInfo(eCivic).getStateReligionGreatPeopleRateModifier() * iHighestReligionCount) / 20);
			iValue += (GC.getCivicInfo(eCivic).getStateReligionGreatPeopleRateModifier() / 4);
			iValue += ((GC.getCivicInfo(eCivic).getStateReligionUnitProductionModifier() * iHighestReligionCount) / 4);
			iValue += ((GC.getCivicInfo(eCivic).getStateReligionBuildingProductionModifier() * iHighestReligionCount) / 3);
			iValue += (GC.getCivicInfo(eCivic).getStateReligionFreeExperience() * iHighestReligionCount * ((bWarPlan) ? 6 : 2));
		}
	}

	for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		iTempValue = 0;

		iTempValue += ((GC.getCivicInfo(eCivic).getYieldModifier(iI) * getNumCities()) / 2);
		//iTempValue += ((GC.getCivicInfo(eCivic).getCapitalYieldModifier(iI)) / 2);
		CvCity* pCapital = getCapitalCity();
		if (pCapital)
		{
			iTempValue += ((GC.getCivicInfo(eCivic).getCapitalYieldModifier(iI) * pCapital->getBaseYieldRate((YieldTypes)iI)) / 80);
		}
		iTempValue += ((GC.getCivicInfo(eCivic).getTradeYieldModifier(iI) * getNumCities()) / 6);

		for (iJ = 0; iJ < GC.getNumImprovementInfos(); iJ++)
		{
			iTempValue += (GC.getCivicInfo(eCivic).getImprovementYieldChanges(iJ, iI) * getImprovementCount((ImprovementTypes)iJ) * 1);
		}

		if (iI == YIELD_FOOD)
		{
			iTempValue *= 3;
		}
		else if (iI == YIELD_PRODUCTION)
		{
			iTempValue *= ((AI_avoidScience()) ? 6 : 2);
		}
		else if (iI == YIELD_COMMERCE)
		{
			iTempValue *= 2;
		}

		iValue += iTempValue;
	}

	for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		iTempValue = 0;

		iTempValue += ((GC.getCivicInfo(eCivic).getCommerceModifier(iI) * getNumCities()) / 3);
		iTempValue += (GC.getCivicInfo(eCivic).getCapitalCommerceModifier(iI) / 2);
		iTempValue += ((GC.getCivicInfo(eCivic).getSpecialistExtraCommerce(iI) * getTotalPopulation()) / 15);

		iTempValue *= AI_commerceWeight((CommerceTypes)iI);
		if ((iI == COMMERCE_CULTURE) && bCultureVictory2)
		{
		    iTempValue *= 2;
		    if (bCultureVictory3)
		    {
		        iTempValue *= 2;		        
		    }
		}
		iTempValue /= 100;

		iValue += iTempValue;
	}

	for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
	{
		if (GC.getCivicInfo(eCivic).getBuildingHappinessChanges(iI) != 0)
		{
			iValue += (GC.getCivicInfo(eCivic).getBuildingHappinessChanges(iI) * countNumBuildings((BuildingTypes)iI) * 3);
		}
	}

	for (iI = 0; iI < GC.getNumFeatureInfos(); iI++)
	{
		iHappiness = GC.getCivicInfo(eCivic).getFeatureHappinessChanges(iI);

		if (iHappiness != 0)
		{
			iValue += (iHappiness * countCityFeatures((FeatureTypes)iI) * 5);
		}
	}

	for (iI = 0; iI < GC.getNumHurryInfos(); iI++)
	{
		if (GC.getCivicInfo(eCivic).isHurry(iI))
		{
			iTempValue = 0;

			if (GC.getHurryInfo((HurryTypes)iI).getGoldPerProduction() > 0)
			{
				iTempValue += ((((AI_avoidScience()) ? 50 : 25) * getNumCities()) / GC.getHurryInfo((HurryTypes)iI).getGoldPerProduction());
			}
			iTempValue += (GC.getHurryInfo((HurryTypes)iI).getProductionPerPopulation() * getNumCities() * (bWarPlan ? 2 : 1)) / 5;
			iValue += iTempValue;
		}
	}

	for (iI = 0; iI < GC.getNumSpecialBuildingInfos(); iI++)
	{
		if (GC.getCivicInfo(eCivic).isSpecialBuildingNotRequired(iI))
		{
			iValue += ((getNumCities() / 2) + 1); // XXX
		}
	}

	for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
	{
		iTempValue = 0;
		if (GC.getCivicInfo(eCivic).isSpecialistValid(iI))
		{
			iTempValue += ((getNumCities() *  (bCultureVictory3 ? 10 : 1)) + 6); // XXX
		}
		iValue += (iTempValue / 2);
	}

	if (GC.getLeaderHeadInfo(getPersonalityType()).getFavoriteCivic() == eCivic)
	{
		if (!GC.getCivicInfo(eCivic).isStateReligion() || iHighestReligionCount > 0)
		{
			iValue *= 5;
			iValue /= 4;
			iValue += 6 * getNumCities();
			iValue += 20;
		}
	}
	
	if (AI_isDoStrategy(AI_STRATEGY_CULTURE2) && (GC.getCivicInfo(eCivic).isNoNonStateReligionSpread()))
	{
	    iValue /= 10;	    
	}

	return iValue;
}


ReligionTypes CvPlayerAI::AI_bestReligion()
{
	ReligionTypes eBestReligion;
	int iValue;
	int iBestValue;
	int iI;

	iBestValue = 0;
	eBestReligion = NO_RELIGION;

	for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
	{
		if (canDoReligion((ReligionTypes)iI))
		{
			iValue = AI_religionValue((ReligionTypes)iI);

			if (getStateReligion() == ((ReligionTypes)iI))
			{
				iValue *= 4;
				iValue /= 3;
			}

			if (iValue > iBestValue)
			{
				iBestValue = iValue;
				eBestReligion = ((ReligionTypes)iI);
			}
		}
	}

	return eBestReligion;
}


int CvPlayerAI::AI_religionValue(ReligionTypes eReligion)
{
	if (getHasReligionCount(eReligion) == 0)
	{
		return 0;
	}

	int iValue = GC.getGameINLINE().countReligionLevels(eReligion);

	int iLoop;
	CvCity* pLoopCity;
	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		if (pLoopCity->isHasReligion(eReligion))
		{
			iValue += pLoopCity->getPopulation();
		}
	}

	CvCity* pHolyCity = GC.getGameINLINE().getHolyCity(eReligion);
	if (pHolyCity != NULL)
	{
		bool bOurHolyCity = pHolyCity->getOwnerINLINE() == getID();
		bool bOurTeamHolyCity = pHolyCity->getTeam() == getTeam();
		
		if (bOurHolyCity || bOurTeamHolyCity)
		{
			int iCommerceCount = 0;

			for (int iI = 0; iI < GC.getNumBuildingInfos(); iI++)
			{
				if (pHolyCity->hasActiveBuilding((BuildingTypes)iI))
				{
					for (int iJ = 0; iJ < NUM_COMMERCE_TYPES; iJ++)
					{
						if (GC.getBuildingInfo((BuildingTypes)iI).getGlobalReligionCommerce() == eReligion)
						{
							iCommerceCount += GC.getReligionInfo(eReligion).getGlobalReligionCommerce((CommerceTypes)iJ);
						}
					}
				}
			}

			if (bOurHolyCity)
			{
				iValue *= (3 + iCommerceCount);
				iValue /= 2;
			}
			else if (bOurTeamHolyCity)
			{
				iValue *= (4 + iCommerceCount);
				iValue /= 3;
			}
		}
	}

	return iValue;
}


int CvPlayerAI::AI_getPeaceWeight()
{
	return m_iPeaceWeight;
}


void CvPlayerAI::AI_setPeaceWeight(int iNewValue)
{
	m_iPeaceWeight = iNewValue;
}


int CvPlayerAI::AI_getAttackOddsChange()
{
	return m_iAttackOddsChange;
}


void CvPlayerAI::AI_setAttackOddsChange(int iNewValue)
{
	m_iAttackOddsChange = iNewValue;
}


int CvPlayerAI::AI_getCivicTimer()
{
	return m_iCivicTimer;
}


void CvPlayerAI::AI_setCivicTimer(int iNewValue)
{
	m_iCivicTimer = iNewValue;
	FAssert(AI_getCivicTimer() >= 0);
}


void CvPlayerAI::AI_changeCivicTimer(int iChange)
{
	AI_setCivicTimer(AI_getCivicTimer() + iChange);
}


int CvPlayerAI::AI_getReligionTimer()
{
	return m_iReligionTimer;
}


void CvPlayerAI::AI_setReligionTimer(int iNewValue)
{
	m_iReligionTimer = iNewValue;
	FAssert(AI_getReligionTimer() >= 0);
}


void CvPlayerAI::AI_changeReligionTimer(int iChange)
{
	AI_setReligionTimer(AI_getReligionTimer() + iChange);
}


int CvPlayerAI::AI_getNumTrainAIUnits(UnitAITypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_UNITAI_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiNumTrainAIUnits[eIndex];
}


void CvPlayerAI::AI_changeNumTrainAIUnits(UnitAITypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_UNITAI_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
	m_aiNumTrainAIUnits[eIndex] = (m_aiNumTrainAIUnits[eIndex] + iChange);
	FAssert(AI_getNumTrainAIUnits(eIndex) >= 0);
}


int CvPlayerAI::AI_getNumAIUnits(UnitAITypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_UNITAI_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiNumAIUnits[eIndex];
}


void CvPlayerAI::AI_changeNumAIUnits(UnitAITypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_UNITAI_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
	m_aiNumAIUnits[eIndex] = (m_aiNumAIUnits[eIndex] + iChange);
	FAssert(AI_getNumAIUnits(eIndex) >= 0);
}


int CvPlayerAI::AI_getSameReligionCounter(PlayerTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiSameReligionCounter[eIndex];
}


void CvPlayerAI::AI_changeSameReligionCounter(PlayerTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	m_aiSameReligionCounter[eIndex] = (m_aiSameReligionCounter[eIndex] + iChange);
	FAssert(AI_getSameReligionCounter(eIndex) >= 0);
}


int CvPlayerAI::AI_getDifferentReligionCounter(PlayerTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiDifferentReligionCounter[eIndex];
}


void CvPlayerAI::AI_changeDifferentReligionCounter(PlayerTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	m_aiDifferentReligionCounter[eIndex] = (m_aiDifferentReligionCounter[eIndex] + iChange);
	FAssert(AI_getDifferentReligionCounter(eIndex) >= 0);
}


int CvPlayerAI::AI_getFavoriteCivicCounter(PlayerTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiFavoriteCivicCounter[eIndex];
}


void CvPlayerAI::AI_changeFavoriteCivicCounter(PlayerTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	m_aiFavoriteCivicCounter[eIndex] = (m_aiFavoriteCivicCounter[eIndex] + iChange);
	FAssert(AI_getFavoriteCivicCounter(eIndex) >= 0);
}


int CvPlayerAI::AI_getBonusTradeCounter(PlayerTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiBonusTradeCounter[eIndex];
}


void CvPlayerAI::AI_changeBonusTradeCounter(PlayerTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	m_aiBonusTradeCounter[eIndex] = (m_aiBonusTradeCounter[eIndex] + iChange);
	FAssert(AI_getBonusTradeCounter(eIndex) >= 0);
}


int CvPlayerAI::AI_getPeacetimeTradeValue(PlayerTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiPeacetimeTradeValue[eIndex];
}


void CvPlayerAI::AI_changePeacetimeTradeValue(PlayerTypes eIndex, int iChange)
{
	PROFILE_FUNC();

	int iI;

	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");

	if (iChange != 0)
	{
		m_aiPeacetimeTradeValue[eIndex] = (m_aiPeacetimeTradeValue[eIndex] + iChange);
		FAssert(AI_getPeacetimeTradeValue(eIndex) >= 0);

		FAssert(iChange > 0);

		if (iChange > 0)
		{
			if (GET_PLAYER(eIndex).getTeam() != getTeam())
			{
				for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
				{
					if (GET_TEAM((TeamTypes)iI).isAlive())
					{
						if (GET_TEAM((TeamTypes)iI).AI_getWorstEnemy() == getTeam())
						{
							GET_TEAM((TeamTypes)iI).AI_changeEnemyPeacetimeTradeValue(GET_PLAYER(eIndex).getTeam(), iChange);
						}
					}
				}
			}
		}
	}
}


int CvPlayerAI::AI_getPeacetimeGrantValue(PlayerTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiPeacetimeGrantValue[eIndex];
}


void CvPlayerAI::AI_changePeacetimeGrantValue(PlayerTypes eIndex, int iChange)
{
	PROFILE_FUNC();

	int iI;

	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");

	if (iChange != 0)
	{
		m_aiPeacetimeGrantValue[eIndex] = (m_aiPeacetimeGrantValue[eIndex] + iChange);
		FAssert(AI_getPeacetimeGrantValue(eIndex) >= 0);

		FAssert(iChange > 0);

		if (iChange > 0)
		{
			if (GET_PLAYER(eIndex).getTeam() != getTeam())
			{
				for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
				{
					if (GET_TEAM((TeamTypes)iI).isAlive())
					{
						if (GET_TEAM((TeamTypes)iI).AI_getWorstEnemy() == getTeam())
						{
							GET_TEAM((TeamTypes)iI).AI_changeEnemyPeacetimeGrantValue(GET_PLAYER(eIndex).getTeam(), iChange);
						}
					}
				}
			}
		}
	}
}


int CvPlayerAI::AI_getGoldTradedTo(PlayerTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiGoldTradedTo[eIndex];
}


void CvPlayerAI::AI_changeGoldTradedTo(PlayerTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	m_aiGoldTradedTo[eIndex] = (m_aiGoldTradedTo[eIndex] + iChange);
	FAssert(AI_getGoldTradedTo(eIndex) >= 0);
}


int CvPlayerAI::AI_getAttitudeExtra(PlayerTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiAttitudeExtra[eIndex];
}


void CvPlayerAI::AI_setAttitudeExtra(PlayerTypes eIndex, int iNewValue)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	m_aiAttitudeExtra[eIndex] = iNewValue;
}


void CvPlayerAI::AI_changeAttitudeExtra(PlayerTypes eIndex, int iChange)
{
	AI_setAttitudeExtra(eIndex, (AI_getAttitudeExtra(eIndex) + iChange));
}


bool CvPlayerAI::AI_isFirstContact(PlayerTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_abFirstContact[eIndex];
}


void CvPlayerAI::AI_setFirstContact(PlayerTypes eIndex, bool bNewValue)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	m_abFirstContact[eIndex] = bNewValue;
}


int CvPlayerAI::AI_getContactTimer(PlayerTypes eIndex1, ContactTypes eIndex2)
{
	FAssertMsg(eIndex1 >= 0, "eIndex1 is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex1 < MAX_PLAYERS, "eIndex1 is expected to be within maximum bounds (invalid Index)");
	FAssertMsg(eIndex2 >= 0, "eIndex2 is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex2 < NUM_CONTACT_TYPES, "eIndex2 is expected to be within maximum bounds (invalid Index)");
	return m_aaiContactTimer[eIndex1][eIndex2];
}


void CvPlayerAI::AI_changeContactTimer(PlayerTypes eIndex1, ContactTypes eIndex2, int iChange)
{
	FAssertMsg(eIndex1 >= 0, "eIndex1 is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex1 < MAX_PLAYERS, "eIndex1 is expected to be within maximum bounds (invalid Index)");
	FAssertMsg(eIndex2 >= 0, "eIndex2 is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex2 < NUM_CONTACT_TYPES, "eIndex2 is expected to be within maximum bounds (invalid Index)");
	m_aaiContactTimer[eIndex1][eIndex2] = (AI_getContactTimer(eIndex1, eIndex2) + iChange);
	FAssert(AI_getContactTimer(eIndex1, eIndex2) >= 0);
}


int CvPlayerAI::AI_getMemoryCount(PlayerTypes eIndex1, MemoryTypes eIndex2)
{
	FAssertMsg(eIndex1 >= 0, "eIndex1 is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex1 < MAX_PLAYERS, "eIndex1 is expected to be within maximum bounds (invalid Index)");
	FAssertMsg(eIndex2 >= 0, "eIndex2 is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex2 < NUM_MEMORY_TYPES, "eIndex2 is expected to be within maximum bounds (invalid Index)");
	return m_aaiMemoryCount[eIndex1][eIndex2];
}


void CvPlayerAI::AI_changeMemoryCount(PlayerTypes eIndex1, MemoryTypes eIndex2, int iChange)
{
	FAssertMsg(eIndex1 >= 0, "eIndex1 is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex1 < MAX_PLAYERS, "eIndex1 is expected to be within maximum bounds (invalid Index)");
	FAssertMsg(eIndex2 >= 0, "eIndex2 is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex2 < NUM_MEMORY_TYPES, "eIndex2 is expected to be within maximum bounds (invalid Index)");
	m_aaiMemoryCount[eIndex1][eIndex2] = (AI_getMemoryCount(eIndex1, eIndex2) + iChange);
	FAssert(AI_getMemoryCount(eIndex1, eIndex2) >= 0);
}

int CvPlayerAI::AI_calculateGoldenAgeValue()
{
    int iValue;
    int iTempValue;
    int iI;

    iValue = 0;
    for (iI = 0; iI <  NUM_YIELD_TYPES; ++iI)
    {
        iTempValue = (GC.getYieldInfo((YieldTypes)iI).getGoldenAgeYield() * AI_yieldWeight((YieldTypes)iI));
        iTempValue /= max(1, (1 + GC.getYieldInfo((YieldTypes)iI).getGoldenAgeYieldThreshold()));
        iValue += iTempValue;
    }

    iValue *= getTotalPopulation();
    iValue *= GC.getGameINLINE().goldenAgeLength();
    iValue /= 100;

    return iValue;
}

// Protected Functions...

void CvPlayerAI::AI_doCounter()
{
	int iBonusImports;
	int iI, iJ;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() != getTeam())
			{
				if (GET_TEAM(getTeam()).isHasMet(GET_PLAYER((PlayerTypes)iI).getTeam()))
				{
					if ((getStateReligion() != NO_RELIGION) &&
						  (getStateReligion() == GET_PLAYER((PlayerTypes)iI).getStateReligion()))
					{
						AI_changeSameReligionCounter(((PlayerTypes)iI), 1);
					}
					else
					{
						if (AI_getSameReligionCounter((PlayerTypes)iI) > 0)
						{
							AI_changeSameReligionCounter(((PlayerTypes)iI), -1);
						}
					}

					if ((getStateReligion() != NO_RELIGION) &&
						  (GET_PLAYER((PlayerTypes)iI).getStateReligion() != NO_RELIGION) &&
						  (getStateReligion() != GET_PLAYER((PlayerTypes)iI).getStateReligion()))
					{
						AI_changeDifferentReligionCounter(((PlayerTypes)iI), 1);
					}
					else
					{
						if (AI_getDifferentReligionCounter((PlayerTypes)iI) > 0)
						{
							AI_changeDifferentReligionCounter(((PlayerTypes)iI), -1);
						}
					}

					if (GC.getLeaderHeadInfo(getPersonalityType()).getFavoriteCivic() != NO_CIVIC)
					{
						if (isCivic((CivicTypes)(GC.getLeaderHeadInfo(getPersonalityType()).getFavoriteCivic())) &&
							  GET_PLAYER((PlayerTypes)iI).isCivic((CivicTypes)(GC.getLeaderHeadInfo(getPersonalityType()).getFavoriteCivic())))
						{
							AI_changeFavoriteCivicCounter(((PlayerTypes)iI), 1);
						}
						else
						{
							if (AI_getFavoriteCivicCounter((PlayerTypes)iI) > 0)
							{
								AI_changeFavoriteCivicCounter(((PlayerTypes)iI), -1);
							}
						}
					}

					iBonusImports = getNumTradeBonusImports((PlayerTypes)iI);

					if (iBonusImports > 0)
					{
						AI_changeBonusTradeCounter(((PlayerTypes)iI), iBonusImports);
					}
					else
					{
						AI_changeBonusTradeCounter(((PlayerTypes)iI), -(min(AI_getBonusTradeCounter((PlayerTypes)iI), ((GET_PLAYER((PlayerTypes)iI).getNumCities() / 4) + 1))));
					}
				}
			}
		}
	}

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			for (iJ = 0; iJ < NUM_CONTACT_TYPES; iJ++)
			{
				if (AI_getContactTimer(((PlayerTypes)iI), ((ContactTypes)iJ)) > 0)
				{
					AI_changeContactTimer(((PlayerTypes)iI), ((ContactTypes)iJ), -1);
				}
			}
		}
	}

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			for (iJ = 0; iJ < NUM_MEMORY_TYPES; iJ++)
			{
				if (AI_getMemoryCount(((PlayerTypes)iI), ((MemoryTypes)iJ)) > 0)
				{
					if (GC.getLeaderHeadInfo(getPersonalityType()).getMemoryDecayRand(iJ) > 0)
					{
						if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getMemoryDecayRand(iJ), "Memory Decay") == 0)
						{
							AI_changeMemoryCount(((PlayerTypes)iI), ((MemoryTypes)iJ), -1);
						}
					}
				}
			}
		}
	}
}


void CvPlayerAI::AI_doMilitary()
{
	if (GET_TEAM(getTeam()).getAnyWarPlanCount(true) == 0)
	{
		if (AI_isFinancialTrouble() && (calculateUnitCost() > 0))
		{
			disbandUnit(false);
		}
	}

	AI_setAttackOddsChange(GC.getLeaderHeadInfo(getPersonalityType()).getBaseAttackOddsChange() +
		GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getAttackOddsChangeRand(), "AI Attack Odds Change #1") +
		GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getAttackOddsChangeRand(), "AI Attack Odds Change #2"));
}


void CvPlayerAI::AI_doResearch()
{
	FAssertMsg(!isHuman(), "isHuman did not return false as expected");

	if (getCurrentResearch() == NO_TECH)
	{
		AI_chooseResearch();
		AI_forceUpdateStrategies(); //to account for current research.
	}
}


void CvPlayerAI::AI_doCommerce()
{
	CvCity* pLoopCity;
	int iIdealPercent;
	int iGoldTarget;
	int iLoop;

	FAssertMsg(!isHuman(), "isHuman did not return false as expected");

	iGoldTarget = AI_goldTarget();

    if (isCommerceFlexible(COMMERCE_RESEARCH) && !AI_avoidScience())
	{
        // set research rate to 90%
		setCommercePercent(COMMERCE_RESEARCH, 100 - (GC.getDefineINT("COMMERCE_PERCENT_CHANGE_INCREMENTS")));
		
		// if the gold rate is under 0 at 90% research
		int iGoldRate = calculateGoldRate();
		if (iGoldRate < 0)
		{
			TechTypes eCurrentResearch = getCurrentResearch();
			if (eCurrentResearch != NO_TECH)
			{
				int iResearchTurnsLeft = getResearchTurnsLeft(eCurrentResearch, true);
				
				// if we can finish the current research without running out of gold, let us spend 2/3rds of our gold 
				if (getGold() >= iResearchTurnsLeft * iGoldRate)
				{
					iGoldTarget /= 3;
				}
			}
		}
	}

	if (isCommerceFlexible(COMMERCE_CULTURE))
	{
		if (getCommercePercent(COMMERCE_CULTURE) > 0)
		{
			setCommercePercent(COMMERCE_CULTURE, 0);

			AI_assignWorkingPlots();
		}

		if (getNumCities() > 0)
		{
			iIdealPercent = 0;

			for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
			{
				if (pLoopCity->getCommerceHappinessPer(COMMERCE_CULTURE) > 0)
				{
					iIdealPercent += ((pLoopCity->angryPopulation() * 100) / pLoopCity->getCommerceHappinessPer(COMMERCE_CULTURE));
				}
			}

			iIdealPercent /= getNumCities();

			iIdealPercent -= (iIdealPercent % GC.getDefineINT("COMMERCE_PERCENT_CHANGE_INCREMENTS"));

			iIdealPercent = min(iIdealPercent, 20);
			
			if (AI_isDoStrategy(AI_STRATEGY_CULTURE4))
			{
			    iIdealPercent = 100;
			}

			setCommercePercent(COMMERCE_CULTURE, iIdealPercent);
		}
	}

	if (isCommerceFlexible(COMMERCE_RESEARCH))
	{
		if (isNoResearchAvailable() || AI_isDoStrategy(AI_STRATEGY_CULTURE4))
		{
			setCommercePercent(COMMERCE_RESEARCH, 0);
		}
		else
		{
			while (calculateGoldRate() > 0)
			{
				changeCommercePercent(COMMERCE_RESEARCH, GC.getDefineINT("COMMERCE_PERCENT_CHANGE_INCREMENTS"));

				if (getCommercePercent(COMMERCE_RESEARCH) == 100)
				{
					break;
				}
			}

			if (getGold() + 8 * calculateGoldRate() < iGoldTarget)
			{
				while (getGold() + 8 * calculateGoldRate() <= iGoldTarget)
				{
					changeCommercePercent(COMMERCE_RESEARCH, -(GC.getDefineINT("COMMERCE_PERCENT_CHANGE_INCREMENTS")));

					if ((getCommercePercent(COMMERCE_RESEARCH) == 0))
					{
						break;
					}
				}
			}
			else
			{
				if (AI_avoidScience())
				{
					changeCommercePercent(COMMERCE_RESEARCH, -(GC.getDefineINT("COMMERCE_PERCENT_CHANGE_INCREMENTS")));
				}
			}

			if ((GET_TEAM(getTeam()).getChosenWarCount(true) > 0) || (GET_TEAM(getTeam()).getWarPlanCount(WARPLAN_ATTACKED_RECENT, true) > 0))
			{
				changeCommercePercent(COMMERCE_RESEARCH, -(GC.getDefineINT("COMMERCE_PERCENT_CHANGE_INCREMENTS")));
			}

			if ((getCommercePercent(COMMERCE_RESEARCH) == 0) && (calculateGoldRate() > 0))
			{
				setCommercePercent(COMMERCE_RESEARCH, GC.getDefineINT("COMMERCE_PERCENT_CHANGE_INCREMENTS"));
			}
		}
	}
	
	// this is called on doTurn, so make sure our gold is high enough keep us above zero gold.
	verifyGoldCommercePercent();
}


void CvPlayerAI::AI_doCivics()
{
	CivicTypes* paeBestCivic;
	int iI;

	FAssertMsg(!isHuman(), "isHuman did not return false as expected");

	if (AI_getCivicTimer() > 0)
	{
		AI_changeCivicTimer(-1);
		return;
	}

	if (getMaxAnarchyTurns() > 0)
	{
		if (isGoldenAge())
		{
			return;
		}

		// if we will become insolvent during anarchy, wait (getGoldPerTurn is just our trade deals)
		if (getGold() + getGoldPerTurn() < 0)
		{
			return;
		}
	}

	if (!canRevolution(NULL))
	{
		return;
	}

	FAssertMsg(AI_getCivicTimer() == 0, "AI Civic timer is expected to be 0");

	paeBestCivic = new CivicTypes[GC.getNumCivicOptionInfos()];

	for (iI = 0; iI < GC.getNumCivicOptionInfos(); iI++)
	{
		paeBestCivic[iI] = AI_bestCivic((CivicOptionTypes)iI);

		if (paeBestCivic[iI] == NO_CIVIC)
		{
			paeBestCivic[iI] = getCivics((CivicOptionTypes)iI);
		}
	}

	// XXX AI skips revolution???
	// check again for real anarchy length, to make sure we will not become insolvent (getGoldPerTurn is just our trade deals)
	if (canRevolution(paeBestCivic) && (getGold() + (getCivicAnarchyLength(paeBestCivic)* getGoldPerTurn()) > 0))
	{
		revolution(paeBestCivic);
		AI_setCivicTimer((getMaxAnarchyTurns() == 0) ? (GC.getDefineINT("MIN_REVOLUTION_TURNS") * 2) : CIVIC_CHANGE_DELAY);
	}

	SAFE_DELETE_ARRAY(paeBestCivic);
}


void CvPlayerAI::AI_doReligion()
{
	ReligionTypes eBestReligion;

	FAssertMsg(!isHuman(), "isHuman did not return false as expected");

	if (AI_getReligionTimer() > 0)
	{
		AI_changeReligionTimer(-1);
		return;
	}

	if (getMaxAnarchyTurns() > 0)
	{
		if (isGoldenAge())
		{
			return;
		}
		
		// if we will become insolvent during anarchy, wait (getGoldPerTurn is just our trade deals)
		if (getGold() + (getReligionAnarchyLength() * getGoldPerTurn()) < 0)
		{
			return;
		}
	}

	if (!canChangeReligion())
	{
		return;
	}

	FAssertMsg(AI_getReligionTimer() == 0, "AI Religion timer is expected to be 0");

	eBestReligion = AI_bestReligion();

	if (eBestReligion == NO_RELIGION)
	{
		eBestReligion = getStateReligion();
	}

	if (canConvert(eBestReligion))
	{
		convert(eBestReligion);
		AI_setReligionTimer((getMaxAnarchyTurns() == 0) ? (GC.getDefineINT("MIN_CONVERSION_TURNS") * 2) : RELIGION_CHANGE_DELAY);
	}
}


void CvPlayerAI::AI_doDiplo()
{
	PROFILE_FUNC();

	CLLNode<TradeData>* pNode;
	CvDiploParameters* pDiplo;
	CvDeal* pLoopDeal;
	CvCity* pLoopCity;
	CvPlot* pLoopPlot;
	CLinkList<TradeData> ourList;
	CLinkList<TradeData> theirList;
	bool abContacted[MAX_TEAMS];
	TradeData item;
	CivicTypes eFavoriteCivic;
	BonusTypes eBestReceiveBonus;
	BonusTypes eBestGiveBonus;
	TechTypes eBestReceiveTech;
	TechTypes eBestGiveTech;
	TeamTypes eBestTeam;
	bool bCancelDeal;
	int iReceiveGold;
	int iGiveGold;
	int iGold;
	int iCount;
	int iPossibleCount;
	int iValue;
	int iBestValue;
	int iOurValue;
	int iTheirValue;
	int iPass;
	int iLoop;
	int iI, iJ;

	FAssert(!isHuman());
	FAssert(!isMinorCiv());
	FAssert(!isBarbarian());

	// allow python to handle it
	CyArgsList argsList;
	argsList.add(getID());
	long lResult=0;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "AI_doDiplo", argsList.makeFunctionArgs(), &lResult);
	if (lResult == 1)
	{
		return;
	}

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		abContacted[iI] = false;
	}

	for (iPass = 0; iPass < 2; iPass++)
	{
		for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if (GET_PLAYER((PlayerTypes)iI).isHuman() == (iPass == 1))
				{
					if (iI != getID())
					{
						if (GET_PLAYER((PlayerTypes)iI).getTeam() != getTeam())
						{
							for(pLoopDeal = GC.getGameINLINE().firstDeal(&iLoop); pLoopDeal != NULL; pLoopDeal = GC.getGameINLINE().nextDeal(&iLoop))
							{
								if (pLoopDeal->isCancelable(getID()))
								{
									if ((GC.getGameINLINE().getGameTurn() - pLoopDeal->getInitialGameTurn()) >= (GC.getDefineINT("PEACE_TREATY_LENGTH") * 2))
									{
										bCancelDeal = false;

										if ((pLoopDeal->getFirstPlayer() == getID()) && (pLoopDeal->getSecondPlayer() == ((PlayerTypes)iI)))
										{
											if (GET_PLAYER((PlayerTypes)iI).isHuman())
											{
												if (!AI_considerOffer(((PlayerTypes)iI), pLoopDeal->getSecondTrades(), pLoopDeal->getFirstTrades()))
												{
													bCancelDeal = true;
												}
											}
											else
											{
												for (pNode = pLoopDeal->getFirstTrades()->head(); pNode; pNode = pLoopDeal->getFirstTrades()->next(pNode))
												{
													if (getTradeDenial(((PlayerTypes)iI), pNode->m_data) != NO_DENIAL)
													{
														bCancelDeal = true;
														break;
													}
												}
											}
										}
										else if ((pLoopDeal->getFirstPlayer() == ((PlayerTypes)iI)) && (pLoopDeal->getSecondPlayer() == getID()))
										{
											if (GET_PLAYER((PlayerTypes)iI).isHuman())
											{
												if (!AI_considerOffer(((PlayerTypes)iI), pLoopDeal->getFirstTrades(), pLoopDeal->getSecondTrades()))
												{
													bCancelDeal = true;
												}
											}
											else
											{
												for (pNode = pLoopDeal->getSecondTrades()->head(); pNode; pNode = pLoopDeal->getSecondTrades()->next(pNode))
												{
													if (getTradeDenial(((PlayerTypes)iI), pNode->m_data) != NO_DENIAL)
													{
														bCancelDeal = true;
														break;
													}
												}
											}
										}

										if (bCancelDeal)
										{
											if (canContact((PlayerTypes)iI) && AI_isWillingToTalk((PlayerTypes)iI))
											{
												if (GET_PLAYER((PlayerTypes)iI).isHuman())
												{
													ourList.clear();
													theirList.clear();

													for (pNode = pLoopDeal->headFirstTradesNode(); (pNode != NULL); pNode = pLoopDeal->nextFirstTradesNode(pNode))
													{
														if (pLoopDeal->getFirstPlayer() == getID())
														{
															ourList.insertAtEnd(pNode->m_data);
														}
														else
														{
															theirList.insertAtEnd(pNode->m_data);
														}
													}

													for (pNode = pLoopDeal->headSecondTradesNode(); (pNode != NULL); pNode = pLoopDeal->nextSecondTradesNode(pNode))
													{
														if (pLoopDeal->getSecondPlayer() == getID())
														{
															ourList.insertAtEnd(pNode->m_data);
														}
														else
														{
															theirList.insertAtEnd(pNode->m_data);
														}
													}

													pDiplo = new CvDiploParameters(getID());
													FAssertMsg(pDiplo != NULL, "pDiplo must be valid");

													if (pLoopDeal->isVassalDeal())
													{
														pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_NO_VASSAL"));
														pDiplo->setAIContact(true);
														gDLL->beginDiplomacy(pDiplo, ((PlayerTypes)iI));

													}
													else
													{
														pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_CANCEL_DEAL"));
														pDiplo->setAIContact(true);
														pDiplo->setOurOfferList(theirList);
														pDiplo->setTheirOfferList(ourList);
														gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
													}
													abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
												}
											}

											pLoopDeal->kill(); // XXX test this for AI...
										}
									}
								}
							}
						}

						if (canContact((PlayerTypes)iI) && AI_isWillingToTalk((PlayerTypes)iI))
						{
							if (GET_PLAYER((PlayerTypes)iI).getTeam() == getTeam() || GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).isVassal(getTeam()))
							{
								// XXX will it cancel this deal if it loses it's first resource???

								iBestValue = 0;
								eBestGiveBonus = NO_BONUS;

								for (iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
								{
									if (getNumTradeableBonuses((BonusTypes)iJ) > 1)
									{
										if (GET_PLAYER((PlayerTypes)iI).AI_bonusTradeVal(((BonusTypes)iJ), getID()) > 0)
										{
											setTradeItem(&item, TRADE_RESOURCES, iJ);

											if (canTradeItem(((PlayerTypes)iI), item, true))
											{
												iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Bonus Trading #1"));

												if (iValue > iBestValue)
												{
													iBestValue = iValue;
													eBestGiveBonus = ((BonusTypes)iJ);
												}
											}
										}
									}
								}

								if (eBestGiveBonus != NO_BONUS)
								{
									ourList.clear();
									theirList.clear();

									setTradeItem(&item, TRADE_RESOURCES, eBestGiveBonus);
									ourList.insertAtEnd(item);

									if (GET_PLAYER((PlayerTypes)iI).isHuman())
									{
										if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
										{
											pDiplo = new CvDiploParameters(getID());
											FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
											pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_GIVE_HELP"));
											pDiplo->setAIContact(true);
											pDiplo->setTheirOfferList(ourList);
											gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
											abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
										}
									}
									else
									{
										GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
									}
								}
							}

							if (GET_PLAYER((PlayerTypes)iI).getTeam() != getTeam() && GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).isVassal(getTeam()))
							{
								iBestValue = 0;
								eBestGiveTech = NO_TECH;

								for (iJ = 0; iJ < GC.getNumTechInfos(); iJ++)
								{
									if (GET_TEAM(getTeam()).AI_techTrade((TechTypes)iJ, GET_PLAYER((PlayerTypes)iI).getTeam()) == NO_DENIAL)
									{
										setTradeItem(&item, TRADE_TECHNOLOGIES, iJ);

										if (canTradeItem(((PlayerTypes)iI), item, true))
										{
											iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Vassal Tech gift"));

											if (iValue > iBestValue)
											{
												iBestValue = iValue;
												eBestGiveTech = ((TechTypes)iJ);
											}
										}
									}
								}

								if (eBestGiveTech != NO_TECH)
								{
									ourList.clear();
									theirList.clear();

									setTradeItem(&item, TRADE_TECHNOLOGIES, eBestGiveTech);
									ourList.insertAtEnd(item);

									if (GET_PLAYER((PlayerTypes)iI).isHuman())
									{
										if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
										{
											pDiplo = new CvDiploParameters(getID());
											FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
											pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_GIVE_HELP"));
											pDiplo->setAIContact(true);
											pDiplo->setTheirOfferList(ourList);
											gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
											abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
										}
									}
									else
									{
										GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
									}
								}
							}

							if (GET_PLAYER((PlayerTypes)iI).getTeam() != getTeam() && !(GET_TEAM(getTeam()).isHuman()) && (GET_PLAYER((PlayerTypes)iI).isHuman() || !(GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).isHuman())))
							{
								FAssertMsg(!(GET_PLAYER((PlayerTypes)iI).isBarbarian()), "(GET_PLAYER((PlayerTypes)iI).isBarbarian()) did not return false as expected");
								FAssertMsg(iI != getID(), "iI is not expected to be equal with getID()");

								if (GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).isVassal(getTeam()) && !GET_PLAYER((PlayerTypes)iI).isTributePaid(getID()))
								{
									iBestValue = 0;
									eBestGiveBonus = NO_BONUS;

									for (iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
									{
										if (GET_PLAYER((PlayerTypes)iI).getNumTradeableBonuses((BonusTypes)iJ) > 0 && getNumAvailableBonuses((BonusTypes)iJ) == 0)
										{
											iValue = AI_bonusTradeVal((BonusTypes)iJ, (PlayerTypes)iI);

											if (iValue > iBestValue)
											{
												iBestValue = iValue;
												eBestGiveBonus = ((BonusTypes)iJ);
											}
										}
									}

									if (eBestGiveBonus != NO_BONUS)
									{
										theirList.clear();
										ourList.clear();

										setTradeItem(&item, TRADE_RESOURCES, eBestGiveBonus);
										theirList.insertAtEnd(item);

										if (GET_PLAYER((PlayerTypes)iI).isHuman())
										{
											if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
											{
												CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_VASSAL_GRANT_TRIBUTE, getID(), eBestGiveBonus);
												if (pInfo)
												{
													gDLL->getInterfaceIFace()->addPopup(pInfo, (PlayerTypes)iI);
													abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
												}
											}
										}
										else
										{
											GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
										}
									}
								}

								if (!(GET_TEAM(getTeam()).isAtWar(GET_PLAYER((PlayerTypes)iI).getTeam())))
								{
									if (AI_getAttitude((PlayerTypes)iI) >= ATTITUDE_CAUTIOUS)
									{
										for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
										{
											if (pLoopCity->getPreviousOwner() != ((PlayerTypes)iI))
											{
												if (((pLoopCity->getGameTurnAcquired() + 4) % 20) == (GC.getGameINLINE().getGameTurn() % 20))
												{
													iCount = 0;
													iPossibleCount = 0;

													for (iJ = 0; iJ < NUM_CITY_PLOTS; iJ++)
													{
														pLoopPlot = plotCity(pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE(), iJ);

														if (pLoopPlot != NULL)
														{
															if (pLoopPlot->getOwnerINLINE() == iI)
															{
																iCount++;
															}

															iPossibleCount++;
														}
													}

													if (iCount >= (iPossibleCount / 2))
													{
														setTradeItem(&item, TRADE_CITIES, pLoopCity->getID());

														if (canTradeItem(((PlayerTypes)iI), item, true))
														{
															ourList.clear();

															ourList.insertAtEnd(item);

															if (GET_PLAYER((PlayerTypes)iI).isHuman())
															{
																//if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
																{
																	pDiplo = new CvDiploParameters(getID());
																	FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
																	pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_OFFER_CITY"));
																	pDiplo->setAIContact(true);
																	pDiplo->setTheirOfferList(ourList);
																	gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
																	abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
																}
															}
															else
															{
																GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, NULL);
															}
														}
													}
												}
											}
										}
									}

									if (GET_TEAM(getTeam()).getLeaderID() == getID())
									{
										if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_PERMANENT_ALLIANCE) == 0)
										{
											bool bOffered = false;
											if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_PERMANENT_ALLIANCE), "AI Diplo Alliance") == 0)
											{
												setTradeItem(&item, TRADE_PERMANENT_ALLIANCE);

												if (canTradeItem(((PlayerTypes)iI), item, true) && GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
												{
													ourList.clear();
													theirList.clear();

													ourList.insertAtEnd(item);
													theirList.insertAtEnd(item);

													bOffered = true;

													if (GET_PLAYER((PlayerTypes)iI).isHuman())
													{
														if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
														{
															AI_changeContactTimer(((PlayerTypes)iI), CONTACT_PERMANENT_ALLIANCE, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_PERMANENT_ALLIANCE));
															pDiplo = new CvDiploParameters(getID());
															FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
															pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_OFFER_DEAL"));
															pDiplo->setAIContact(true);
															pDiplo->setOurOfferList(theirList);
															pDiplo->setTheirOfferList(ourList);
															gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
															abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
														}
													}
													else
													{
														GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
													}
												}
											}

											if (!bOffered)
											{
												setTradeItem(&item, TRADE_VASSAL);

												if (canTradeItem((PlayerTypes)iI, item, true))
												{
													ourList.clear();
													theirList.clear();

													ourList.insertAtEnd(item);

													if (GET_PLAYER((PlayerTypes)iI).isHuman())
													{
														if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
														{
															AI_changeContactTimer(((PlayerTypes)iI), CONTACT_PERMANENT_ALLIANCE, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_PERMANENT_ALLIANCE));
															pDiplo = new CvDiploParameters(getID());
															FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
															pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_OFFER_VASSAL"));
															pDiplo->setAIContact(true);
															pDiplo->setOurOfferList(theirList);
															pDiplo->setTheirOfferList(ourList);
															gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
															abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
														}
													}
													else
													{
														bool bAccepted = true;
														TeamTypes eMasterTeam = GET_PLAYER((PlayerTypes)iI).getTeam();
														for (int iTeam = 0; iTeam < MAX_CIV_TEAMS; iTeam++)
														{
															if (GET_TEAM((TeamTypes)iTeam).isAlive())
															{
																if (iTeam != getTeam() && iTeam != eMasterTeam && atWar(getTeam(), (TeamTypes)iTeam) && !atWar(eMasterTeam, (TeamTypes)iTeam))
																{
																	if (GET_TEAM(eMasterTeam).AI_declareWarTrade((TeamTypes)iTeam, getTeam(), false) != NO_DENIAL)
																	{
																		bAccepted = false;
																		break;
																	}
																}
															}
														}

														if (bAccepted)
														{
															GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
														}
													}
												}
											}
										}
									}

									if (GET_PLAYER((PlayerTypes)iI).isHuman() && (GET_TEAM(getTeam()).getLeaderID() == getID()) && !GET_TEAM(getTeam()).isVassal(GET_PLAYER((PlayerTypes)iI).getTeam()))
									{
										if (getStateReligion() != NO_RELIGION)
										{
											if (GET_PLAYER((PlayerTypes)iI).canConvert(getStateReligion()))
											{
												if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_RELIGION_PRESSURE) == 0)
												{
													if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_RELIGION_PRESSURE), "AI Diplo Religion Pressure") == 0)
													{
														if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
														{
															AI_changeContactTimer(((PlayerTypes)iI), CONTACT_RELIGION_PRESSURE, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_RELIGION_PRESSURE));
															pDiplo = new CvDiploParameters(getID());
															FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
															pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_RELIGION_PRESSURE"));
															pDiplo->setAIContact(true);
															gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
															abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
														}
													}
												}
											}
										}
									}

									if (GET_PLAYER((PlayerTypes)iI).isHuman() && (GET_TEAM(getTeam()).getLeaderID() == getID()) && !GET_TEAM(getTeam()).isVassal(GET_PLAYER((PlayerTypes)iI).getTeam()))
									{
										eFavoriteCivic = ((CivicTypes)(GC.getLeaderHeadInfo(getPersonalityType()).getFavoriteCivic()));

										if (eFavoriteCivic != NO_CIVIC)
										{
											if (isCivic(eFavoriteCivic))
											{
												if (GET_PLAYER((PlayerTypes)iI).canDoCivics(eFavoriteCivic) && !(GET_PLAYER((PlayerTypes)iI).isCivic(eFavoriteCivic)))
												{
													if (GET_PLAYER((PlayerTypes)iI).canRevolution(NULL))
													{
														if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_CIVIC_PRESSURE) == 0)
														{
															if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_CIVIC_PRESSURE), "AI Diplo Civic Pressure") == 0)
															{
																if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
																{
																	AI_changeContactTimer(((PlayerTypes)iI), CONTACT_CIVIC_PRESSURE, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_CIVIC_PRESSURE));
																	pDiplo = new CvDiploParameters(getID());
																	FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
																	pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_CIVIC_PRESSURE"), GC.getCivicInfo(eFavoriteCivic).getTextKey());
																	pDiplo->setAIContact(true);
																	gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
																	abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
																}
															}
														}
													}
												}
											}
										}
									}

									if (GET_PLAYER((PlayerTypes)iI).isHuman() && (GET_TEAM(getTeam()).getLeaderID() == getID()))
									{
										if ((AI_getMemoryCount(((PlayerTypes)iI), MEMORY_DECLARED_WAR) == 0) && (AI_getMemoryCount(((PlayerTypes)iI), MEMORY_HIRED_WAR_ALLY) == 0))
										{
											if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_JOIN_WAR) == 0)
											{
												if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_JOIN_WAR), "AI Diplo Join War") == 0)
												{
													iBestValue = 0;
													eBestTeam = NO_TEAM;

													for (iJ = 0; iJ < MAX_CIV_TEAMS; iJ++)
													{
														if (GET_TEAM((TeamTypes)iJ).isAlive())
														{
															if (atWar(((TeamTypes)iJ), getTeam()) && !atWar(((TeamTypes)iJ), GET_PLAYER((PlayerTypes)iI).getTeam()))
															{
																if (GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).isHasMet((TeamTypes)iJ))
																{
																	if (GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).canDeclareWar((TeamTypes)iJ))
																	{
																		iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Joining War"));

																		if (iValue > iBestValue)
																		{
																			iBestValue = iValue;
																			eBestTeam = ((TeamTypes)iJ);
																		}
																	}
																}
															}
														}
													}

													if (eBestTeam != NO_TEAM)
													{
														if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
														{
															AI_changeContactTimer(((PlayerTypes)iI), CONTACT_JOIN_WAR, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_JOIN_WAR));
															pDiplo = new CvDiploParameters(getID());
															FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
															pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_JOIN_WAR"), GET_PLAYER(GET_TEAM(eBestTeam).getLeaderID()).getCivilizationAdjectiveKey());
															pDiplo->setAIContact(true);
															pDiplo->setData(eBestTeam);
															gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
															abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
														}
													}
												}
											}
										}
									}

									if (GET_PLAYER((PlayerTypes)iI).isHuman() && (GET_TEAM(getTeam()).getLeaderID() == getID()) && !GET_TEAM(getTeam()).isVassal(GET_PLAYER((PlayerTypes)iI).getTeam()))
									{
										if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_STOP_TRADING) == 0)
										{
											if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_STOP_TRADING), "AI Diplo Stop Trading") == 0)
											{
												eBestTeam = GET_TEAM(getTeam()).AI_getWorstEnemy();

												if ((eBestTeam != NO_TEAM) && GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).isHasMet(eBestTeam) && !GET_TEAM(eBestTeam).isVassal(GET_PLAYER((PlayerTypes)iI).getTeam()))
												{
													if (GET_PLAYER((PlayerTypes)iI).canStopTradingWithTeam(eBestTeam))
													{
														FAssert(!atWar(GET_PLAYER((PlayerTypes)iI).getTeam(), eBestTeam));
														FAssert(GET_PLAYER((PlayerTypes)iI).getTeam() != eBestTeam);

														if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
														{
															AI_changeContactTimer(((PlayerTypes)iI), CONTACT_STOP_TRADING, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_STOP_TRADING));
															pDiplo = new CvDiploParameters(getID());
															FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
															pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_STOP_TRADING"), GET_PLAYER(GET_TEAM(eBestTeam).getLeaderID()).getCivilizationAdjectiveKey());
															pDiplo->setAIContact(true);
															pDiplo->setData(eBestTeam);
															gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
															abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
														}
													}
												}
											}
										}
									}

									if (GET_PLAYER((PlayerTypes)iI).isHuman() && (GET_TEAM(getTeam()).getLeaderID() == getID()))
									{
										if (GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).getAssets() < (GET_TEAM(getTeam()).getAssets() / 2))
										{
											if (AI_getAttitude((PlayerTypes)iI) > GC.getLeaderHeadInfo(GET_PLAYER((PlayerTypes)iI).getPersonalityType()).getNoGiveHelpAttitudeThreshold())
											{
												if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_GIVE_HELP) == 0)
												{
													if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_GIVE_HELP), "AI Diplo Give Help") == 0)
													{
														// XXX maybe do gold instead???

														iBestValue = 0;
														eBestGiveTech = NO_TECH;

														for (iJ = 0; iJ < GC.getNumTechInfos(); iJ++)
														{
															setTradeItem(&item, TRADE_TECHNOLOGIES, iJ);

															if (canTradeItem(((PlayerTypes)iI), item, true))
															{
																iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Giving Help"));

																if (iValue > iBestValue)
																{
																	iBestValue = iValue;
																	eBestGiveTech = ((TechTypes)iJ);
																}
															}
														}

														if (eBestGiveTech != NO_TECH)
														{
															ourList.clear();

															setTradeItem(&item, TRADE_TECHNOLOGIES, eBestGiveTech);
															ourList.insertAtEnd(item);

															if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
															{
																AI_changeContactTimer(((PlayerTypes)iI), CONTACT_GIVE_HELP, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_GIVE_HELP));
																pDiplo = new CvDiploParameters(getID());
																FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
																pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_GIVE_HELP"));
																pDiplo->setAIContact(true);
																pDiplo->setTheirOfferList(ourList);
																gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
																abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
															}
														}
													}
												}
											}
										}
									}

									if (GET_PLAYER((PlayerTypes)iI).isHuman() && (GET_TEAM(getTeam()).getLeaderID() == getID()))
									{
										if (GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).getAssets() > (GET_TEAM(getTeam()).getAssets() / 2))
										{
											if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_ASK_FOR_HELP) == 0)
											{
												if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_ASK_FOR_HELP), "AI Diplo Ask For Help") == 0)
												{
													iBestValue = 0;
													eBestReceiveTech = NO_TECH;

													for (iJ = 0; iJ < GC.getNumTechInfos(); iJ++)
													{
														setTradeItem(&item, TRADE_TECHNOLOGIES, iJ);

														if (GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
														{
															iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Asking For Help"));

															if (iValue > iBestValue)
															{
																iBestValue = iValue;
																eBestReceiveTech = ((TechTypes)iJ);
															}
														}
													}

													if (eBestReceiveTech != NO_TECH)
													{
														theirList.clear();

														setTradeItem(&item, TRADE_TECHNOLOGIES, eBestReceiveTech);
														theirList.insertAtEnd(item);

														if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
														{
															AI_changeContactTimer(((PlayerTypes)iI), CONTACT_ASK_FOR_HELP, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_ASK_FOR_HELP));
															pDiplo = new CvDiploParameters(getID());
															FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
															pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_ASK_FOR_HELP"));
															pDiplo->setAIContact(true);
															pDiplo->setOurOfferList(theirList);
															gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
															abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
														}
													}
												}
											}
										}
									}

									if (GET_PLAYER((PlayerTypes)iI).isHuman() && (GET_TEAM(getTeam()).getLeaderID() == getID()))
									{
										if (GET_TEAM(getTeam()).canDeclareWar(GET_PLAYER((PlayerTypes)iI).getTeam()))
										{
											if (GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).getDefensivePower() < GET_TEAM(getTeam()).getPower(true))
											{
												if (AI_getAttitude((PlayerTypes)iI) <= GC.getLeaderHeadInfo(GET_PLAYER((PlayerTypes)iI).getPersonalityType()).getDemandTributeAttitudeThreshold())
												{
													if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_DEMAND_TRIBUTE) == 0)
													{
														if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_DEMAND_TRIBUTE), "AI Diplo Demand Tribute") == 0)
														{
															iReceiveGold = min(max(0, (GET_PLAYER((PlayerTypes)iI).getGold() - 50)), GET_PLAYER((PlayerTypes)iI).AI_goldTarget());

															iReceiveGold -= (iReceiveGold % GC.getDefineINT("DIPLOMACY_VALUE_REMAINDER"));

															if (iReceiveGold > 50)
															{
																theirList.clear();

																setTradeItem(&item, TRADE_GOLD, iReceiveGold);
																theirList.insertAtEnd(item);

																if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
																{
																	AI_changeContactTimer(((PlayerTypes)iI), CONTACT_DEMAND_TRIBUTE, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_DEMAND_TRIBUTE));
																	pDiplo = new CvDiploParameters(getID());
																	FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
																	pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_DEMAND_TRIBUTE"));
																	pDiplo->setAIContact(true);
																	pDiplo->setOurOfferList(theirList);
																	gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
																	abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
																}
															}
														}
													}
												}
											}
										}
									}

									if (GET_PLAYER((PlayerTypes)iI).isHuman() && (GET_TEAM(getTeam()).getLeaderID() == getID()))
									{
										if (GET_TEAM(getTeam()).canDeclareWar(GET_PLAYER((PlayerTypes)iI).getTeam()))
										{
											if (GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).getDefensivePower() < GET_TEAM(getTeam()).getPower(true))
											{
												if (AI_getAttitude((PlayerTypes)iI) <= GC.getLeaderHeadInfo(GET_PLAYER((PlayerTypes)iI).getPersonalityType()).getDemandTributeAttitudeThreshold())
												{
													if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_DEMAND_TRIBUTE) == 0)
													{
														if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_DEMAND_TRIBUTE), "AI Diplo Demand Tribute") == 0)
														{
															if (GET_TEAM(getTeam()).AI_mapTradeVal(GET_PLAYER((PlayerTypes)iI).getTeam()) > 100)
															{
																theirList.clear();

																setTradeItem(&item, TRADE_MAPS);
																theirList.insertAtEnd(item);

																if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
																{
																	AI_changeContactTimer(((PlayerTypes)iI), CONTACT_DEMAND_TRIBUTE, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_DEMAND_TRIBUTE));
																	pDiplo = new CvDiploParameters(getID());
																	FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
																	pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_DEMAND_TRIBUTE"));
																	pDiplo->setAIContact(true);
																	pDiplo->setOurOfferList(theirList);
																	gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
																	abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
																}
															}
														}
													}
												}
											}
										}
									}

									if (GET_PLAYER((PlayerTypes)iI).isHuman() && (GET_TEAM(getTeam()).getLeaderID() == getID()))
									{
										if (GET_TEAM(getTeam()).canDeclareWar(GET_PLAYER((PlayerTypes)iI).getTeam()))
										{
											if (GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).getDefensivePower() < GET_TEAM(getTeam()).getPower(true))
											{
												if (AI_getAttitude((PlayerTypes)iI) <= GC.getLeaderHeadInfo(GET_PLAYER((PlayerTypes)iI).getPersonalityType()).getDemandTributeAttitudeThreshold())
												{
													if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_DEMAND_TRIBUTE) == 0)
													{
														if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_DEMAND_TRIBUTE), "AI Diplo Demand Tribute") == 0)
														{
															iBestValue = 0;
															eBestReceiveTech = NO_TECH;

															for (iJ = 0; iJ < GC.getNumTechInfos(); iJ++)
															{
																setTradeItem(&item, TRADE_TECHNOLOGIES, iJ);

																if (GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
																{
																	if (GC.getGameINLINE().countKnownTechNumTeams((TechTypes)iJ) > 1)
																	{
																		iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Demanding Tribute (Tech)"));

																		if (iValue > iBestValue)
																		{
																			iBestValue = iValue;
																			eBestReceiveTech = ((TechTypes)iJ);
																		}
																	}
																}
															}

															if (eBestReceiveTech != NO_TECH)
															{
																theirList.clear();

																setTradeItem(&item, TRADE_TECHNOLOGIES, eBestReceiveTech);
																theirList.insertAtEnd(item);

																if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
																{
																	AI_changeContactTimer(((PlayerTypes)iI), CONTACT_DEMAND_TRIBUTE, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_DEMAND_TRIBUTE));
																	pDiplo = new CvDiploParameters(getID());
																	FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
																	pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_DEMAND_TRIBUTE"));
																	pDiplo->setAIContact(true);
																	pDiplo->setOurOfferList(theirList);
																	gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
																	abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
																}
															}
														}
													}
												}
											}
										}
									}

									if (GET_PLAYER((PlayerTypes)iI).isHuman() && (GET_TEAM(getTeam()).getLeaderID() == getID()))
									{
										if (GET_TEAM(getTeam()).canDeclareWar(GET_PLAYER((PlayerTypes)iI).getTeam()))
										{
											if (GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).getDefensivePower() < GET_TEAM(getTeam()).getPower(true))
											{
												if (AI_getAttitude((PlayerTypes)iI) <= GC.getLeaderHeadInfo(GET_PLAYER((PlayerTypes)iI).getPersonalityType()).getDemandTributeAttitudeThreshold())
												{
													if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_DEMAND_TRIBUTE) == 0)
													{
														if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_DEMAND_TRIBUTE), "AI Diplo Demand Tribute") == 0)
														{
															iBestValue = 0;
															eBestReceiveBonus = NO_BONUS;

															for (iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
															{
																if (GET_PLAYER((PlayerTypes)iI).getNumTradeableBonuses((BonusTypes)iJ) > 1)
																{
																	if (AI_bonusTradeVal(((BonusTypes)iJ), ((PlayerTypes)iI)) > 0)
																	{
																		setTradeItem(&item, TRADE_RESOURCES, iJ);

																		if (GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
																		{
																			iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Demanding Tribute (Bonus)"));

																			if (iValue > iBestValue)
																			{
																				iBestValue = iValue;
																				eBestReceiveBonus = ((BonusTypes)iJ);
																			}
																		}
																	}
																}
															}

															if (eBestReceiveBonus != NO_BONUS)
															{
																theirList.clear();

																setTradeItem(&item, TRADE_RESOURCES, eBestReceiveBonus);
																theirList.insertAtEnd(item);

																if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
																{
																	AI_changeContactTimer(((PlayerTypes)iI), CONTACT_DEMAND_TRIBUTE, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_DEMAND_TRIBUTE));
																	pDiplo = new CvDiploParameters(getID());
																	FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
																	pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_DEMAND_TRIBUTE"));
																	pDiplo->setAIContact(true);
																	pDiplo->setOurOfferList(theirList);
																	gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
																	abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
																}
															}
														}
													}
												}
											}
										}
									}

									if (GET_TEAM(getTeam()).getLeaderID() == getID())
									{
										if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_OPEN_BORDERS) == 0)
										{
											if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_OPEN_BORDERS), "AI Diplo Open Borders") == 0)
											{
												setTradeItem(&item, TRADE_OPEN_BORDERS);

												if (canTradeItem(((PlayerTypes)iI), item, true) && GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
												{
													ourList.clear();
													theirList.clear();

													ourList.insertAtEnd(item);
													theirList.insertAtEnd(item);

													if (GET_PLAYER((PlayerTypes)iI).isHuman())
													{
														if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
														{
															AI_changeContactTimer(((PlayerTypes)iI), CONTACT_OPEN_BORDERS, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_OPEN_BORDERS));
															pDiplo = new CvDiploParameters(getID());
															FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
															pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_OFFER_DEAL"));
															pDiplo->setAIContact(true);
															pDiplo->setOurOfferList(theirList);
															pDiplo->setTheirOfferList(ourList);
															gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
															abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
														}
													}
													else
													{
														GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
													}
												}
											}
										}
									}

									if (GET_TEAM(getTeam()).getLeaderID() == getID())
									{
										if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_DEFENSIVE_PACT) == 0)
										{
											if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_DEFENSIVE_PACT), "AI Diplo Defensive Pact") == 0)
											{
												setTradeItem(&item, TRADE_DEFENSIVE_PACT);

												if (canTradeItem(((PlayerTypes)iI), item, true) && GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
												{
													ourList.clear();
													theirList.clear();

													ourList.insertAtEnd(item);
													theirList.insertAtEnd(item);

													if (GET_PLAYER((PlayerTypes)iI).isHuman())
													{
														if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
														{
															AI_changeContactTimer(((PlayerTypes)iI), CONTACT_DEFENSIVE_PACT, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_DEFENSIVE_PACT));
															pDiplo = new CvDiploParameters(getID());
															FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
															pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_OFFER_DEAL"));
															pDiplo->setAIContact(true);
															pDiplo->setOurOfferList(theirList);
															pDiplo->setTheirOfferList(ourList);
															gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
															abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
														}
													}
													else
													{
														GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
													}
												}
											}
										}
									}

									if (!(GET_PLAYER((PlayerTypes)iI).isHuman()) || (GET_TEAM(getTeam()).getLeaderID() == getID()))
									{
										if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_TRADE_TECH) == 0)
										{
											if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_TRADE_TECH), "AI Diplo Trade Tech") == 0)
											{
												iBestValue = 0;
												eBestReceiveTech = NO_TECH;

												for (iJ = 0; iJ < GC.getNumTechInfos(); iJ++)
												{
													setTradeItem(&item, TRADE_TECHNOLOGIES, iJ);

													if (GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
													{
														iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Tech Trading #1"));

														if (iValue > iBestValue)
														{
															iBestValue = iValue;
															eBestReceiveTech = ((TechTypes)iJ);
														}
													}
												}

												if (eBestReceiveTech != NO_TECH)
												{
													iBestValue = 0;
													eBestGiveTech = NO_TECH;

													for (iJ = 0; iJ < GC.getNumTechInfos(); iJ++)
													{
														setTradeItem(&item, TRADE_TECHNOLOGIES, iJ);

														if (canTradeItem(((PlayerTypes)iI), item, true))
														{
															iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Tech Trading #2"));

															if (iValue > iBestValue)
															{
																iBestValue = iValue;
																eBestGiveTech = ((TechTypes)iJ);
															}
														}
													}

													iOurValue = GET_TEAM(getTeam()).AI_techTradeVal(eBestReceiveTech, GET_PLAYER((PlayerTypes)iI).getTeam());
													if (eBestGiveTech != NO_TECH)
													{
														iTheirValue = GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).AI_techTradeVal(eBestGiveTech, getTeam());
													}
													else
													{
														iTheirValue = 0;
													}

													iReceiveGold = 0;
													iGiveGold = 0;

													if (iTheirValue > iOurValue)
													{
														iGold = min((iTheirValue - iOurValue), GET_PLAYER((PlayerTypes)iI).AI_maxGoldTrade(getID()));

														if (iGold > 0)
														{
															setTradeItem(&item, TRADE_GOLD, iGold);

															if (GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
															{
																iReceiveGold = iGold;
																iOurValue += iGold;
															}
														}
													}
													else if (iOurValue > iTheirValue)
													{
														iGold = min((iOurValue - iTheirValue), AI_maxGoldTrade((PlayerTypes)iI));

														if (iGold > 0)
														{
															setTradeItem(&item, TRADE_GOLD, iGold);

															if (canTradeItem(((PlayerTypes)iI), item, true))
															{
																iGiveGold = iGold;
																iTheirValue += iGold;
															}
														}
													}

													if (!(GET_PLAYER((PlayerTypes)iI).isHuman()) || (iOurValue >= iTheirValue))
													{
														if ((iOurValue > ((iTheirValue * 2) / 3)) && (iTheirValue > ((iOurValue * 2) / 3)))
														{
															ourList.clear();
															theirList.clear();

															if (eBestGiveTech != NO_TECH)
															{
																setTradeItem(&item, TRADE_TECHNOLOGIES, eBestGiveTech);
																ourList.insertAtEnd(item);
															}

															setTradeItem(&item, TRADE_TECHNOLOGIES, eBestReceiveTech);
															theirList.insertAtEnd(item);

															if (iGiveGold != 0)
															{
																setTradeItem(&item, TRADE_GOLD, iGiveGold);
																ourList.insertAtEnd(item);
															}

															if (iReceiveGold != 0)
															{
																setTradeItem(&item, TRADE_GOLD, iReceiveGold);
																theirList.insertAtEnd(item);
															}

															if (GET_PLAYER((PlayerTypes)iI).isHuman())
															{
																if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
																{
																	AI_changeContactTimer(((PlayerTypes)iI), CONTACT_TRADE_TECH, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_TRADE_TECH));
																	pDiplo = new CvDiploParameters(getID());
																	FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
																	pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_OFFER_DEAL"));
																	pDiplo->setAIContact(true);
																	pDiplo->setOurOfferList(theirList);
																	pDiplo->setTheirOfferList(ourList);
																	gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
																	abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
																}
															}
															else
															{
																GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
															}
														}
													}
												}
											}
										}
									}

									if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_TRADE_BONUS) == 0)
									{
										if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_TRADE_BONUS), "AI Diplo Trade Bonus") == 0)
										{
											iBestValue = 0;
											eBestReceiveBonus = NO_BONUS;

											for (iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
											{
												if (GET_PLAYER((PlayerTypes)iI).getNumTradeableBonuses((BonusTypes)iJ) > 1)
												{
													if (AI_bonusTradeVal(((BonusTypes)iJ), ((PlayerTypes)iI)) > 0)
													{
														setTradeItem(&item, TRADE_RESOURCES, iJ);

														if (GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
														{
															iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Bonus Trading #1"));

															if (iValue > iBestValue)
															{
																iBestValue = iValue;
																eBestReceiveBonus = ((BonusTypes)iJ);
															}
														}
													}
												}
											}

											if (eBestReceiveBonus != NO_BONUS)
											{
												iBestValue = 0;
												eBestGiveBonus = NO_BONUS;

												for (iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
												{
													if (iJ != eBestReceiveBonus)
													{
														if (getNumTradeableBonuses((BonusTypes)iJ) > 1)
														{
															if (GET_PLAYER((PlayerTypes)iI).AI_bonusTradeVal(((BonusTypes)iJ), getID()) > 0)
															{
																setTradeItem(&item, TRADE_RESOURCES, iJ);

																if (canTradeItem(((PlayerTypes)iI), item, true))
																{
																	iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Bonus Trading #2"));

																	if (iValue > iBestValue)
																	{
																		iBestValue = iValue;
																		eBestGiveBonus = ((BonusTypes)iJ);
																	}
																}
															}
														}
													}
												}

												if (eBestGiveBonus != NO_BONUS)
												{
													if (!(GET_PLAYER((PlayerTypes)iI).isHuman()) || (AI_bonusTradeVal(eBestReceiveBonus, ((PlayerTypes)iI)) >= GET_PLAYER((PlayerTypes)iI).AI_bonusTradeVal(eBestGiveBonus, getID())))
													{
														ourList.clear();
														theirList.clear();

														setTradeItem(&item, TRADE_RESOURCES, eBestGiveBonus);
														ourList.insertAtEnd(item);

														setTradeItem(&item, TRADE_RESOURCES, eBestReceiveBonus);
														theirList.insertAtEnd(item);

														if (GET_PLAYER((PlayerTypes)iI).isHuman())
														{
															if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
															{
																AI_changeContactTimer(((PlayerTypes)iI), CONTACT_TRADE_BONUS, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_TRADE_BONUS));
																pDiplo = new CvDiploParameters(getID());
																FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
																pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_OFFER_DEAL"));
																pDiplo->setAIContact(true);
																pDiplo->setOurOfferList(theirList);
																pDiplo->setTheirOfferList(ourList);
																gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
																abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
															}
														}
														else
														{
															GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
														}
													}
												}
											}
										}
									}

									if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_TRADE_MAP) == 0)
									{
										if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_TRADE_MAP), "AI Diplo Trade Map") == 0)
										{
											setTradeItem(&item, TRADE_MAPS);

											if (GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true) && canTradeItem(((PlayerTypes)iI), item, true))
											{
												if (!(GET_PLAYER((PlayerTypes)iI).isHuman()) || (GET_TEAM(getTeam()).AI_mapTradeVal(GET_PLAYER((PlayerTypes)iI).getTeam()) >= GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).AI_mapTradeVal(getTeam())))
												{
													ourList.clear();
													theirList.clear();

													setTradeItem(&item, TRADE_MAPS);
													ourList.insertAtEnd(item);

													setTradeItem(&item, TRADE_MAPS);
													theirList.insertAtEnd(item);

													if (GET_PLAYER((PlayerTypes)iI).isHuman())
													{
														if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
														{
															AI_changeContactTimer(((PlayerTypes)iI), CONTACT_TRADE_MAP, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_TRADE_MAP));
															pDiplo = new CvDiploParameters(getID());
															FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
															pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_OFFER_DEAL"));
															pDiplo->setAIContact(true);
															pDiplo->setOurOfferList(theirList);
															pDiplo->setTheirOfferList(ourList);
															gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
															abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
														}
													}
													else
													{
														GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
													}
												}
											}
										}
									}

									if (!(GET_PLAYER((PlayerTypes)iI).isHuman()))
									{
										if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getDeclareWarTradeRand(), "AI Diplo Declare War Trade") == 0)
										{
											iBestValue = 0;
											eBestTeam = NO_TEAM;

											for (iJ = 0; iJ < MAX_CIV_TEAMS; iJ++)
											{
												if (GET_TEAM((TeamTypes)iJ).isAlive())
												{
													if (atWar(((TeamTypes)iJ), getTeam()) && !atWar(((TeamTypes)iJ), GET_PLAYER((PlayerTypes)iI).getTeam()))
													{
														if (GET_TEAM((TeamTypes)iJ).getAtWarCount(true) < max(2, (GC.getGameINLINE().countCivTeamsAlive() / 2)))
														{
															setTradeItem(&item, TRADE_WAR, iJ);

															if (GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
															{
																iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Declare War Trading"));

																if (iValue > iBestValue)
																{
																	iBestValue = iValue;
																	eBestTeam = ((TeamTypes)iJ);
																}
															}
														}
													}
												}
											}

											if (eBestTeam != NO_TEAM)
											{
												iBestValue = 0;
												eBestGiveTech = NO_TECH;

												for (iJ = 0; iJ < GC.getNumTechInfos(); iJ++)
												{
													setTradeItem(&item, TRADE_TECHNOLOGIES, iJ);

													if (canTradeItem(((PlayerTypes)iI), item, true))
													{
														iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Tech Trading #2"));

														if (iValue > iBestValue)
														{
															iBestValue = iValue;
															eBestGiveTech = ((TechTypes)iJ);
														}
													}
												}

												iOurValue = GET_TEAM(getTeam()).AI_declareWarTradeVal(eBestTeam, GET_PLAYER((PlayerTypes)iI).getTeam());
												if (eBestGiveTech != NO_TECH)
												{
													iTheirValue = GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).AI_techTradeVal(eBestGiveTech, getTeam());
												}
												else
												{
													iTheirValue = 0;
												}

												iReceiveGold = 0;
												iGiveGold = 0;

												if (iTheirValue > iOurValue)
												{
													iGold = min((iTheirValue - iOurValue), GET_PLAYER((PlayerTypes)iI).AI_maxGoldTrade(getID()));

													if (iGold > 0)
													{
														setTradeItem(&item, TRADE_GOLD, iGold);

														if (GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
														{
															iReceiveGold = iGold;
															iOurValue += iGold;
														}
													}
												}
												else if (iOurValue > iTheirValue)
												{
													iGold = min((iOurValue - iTheirValue), AI_maxGoldTrade((PlayerTypes)iI));

													if (iGold > 0)
													{
														setTradeItem(&item, TRADE_GOLD, iGold);

														if (canTradeItem(((PlayerTypes)iI), item, true))
														{
															iGiveGold = iGold;
															iTheirValue += iGold;
														}
													}
												}

												if ((iOurValue > ((iTheirValue * 3) / 4)) && (iTheirValue > ((iOurValue * 3) / 4)))
												{
													ourList.clear();
													theirList.clear();

													if (eBestGiveTech != NO_TECH)
													{
														setTradeItem(&item, TRADE_TECHNOLOGIES, eBestGiveTech);
														ourList.insertAtEnd(item);
													}

													setTradeItem(&item, TRADE_WAR, eBestTeam);
													theirList.insertAtEnd(item);

													if (iGiveGold != 0)
													{
														setTradeItem(&item, TRADE_GOLD, iGiveGold);
														ourList.insertAtEnd(item);
													}

													if (iReceiveGold != 0)
													{
														setTradeItem(&item, TRADE_GOLD, iReceiveGold);
														theirList.insertAtEnd(item);
													}

													GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
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
}


void CvPlayerAI::AI_doCheckFinancialTrouble()
{
	// if we just ran into financial trouble this turn
	bool bFinancialTrouble = AI_isFinancialTrouble();
	if (bFinancialTrouble != m_bWasFinancialTrouble)
	{
		if (bFinancialTrouble)
		{
			int iGameTurn = GC.getGameINLINE().getGameTurn();
			
			// only reset at most every 10 turns
			if (iGameTurn > m_iTurnLastProductionDirty + 10)
			{
				// redeterimine the best things to build in each city
				AI_makeProductionDirty();
			
				m_iTurnLastProductionDirty = iGameTurn;
			}
		}
		
		m_bWasFinancialTrouble = bFinancialTrouble;
	}
}

int CvPlayerAI::AI_cultureVictoryTechValue(TechTypes eTech)
{
	int iI;
	
	if (eTech == NO_TECH)
	{
		return 0;
	}
	
	int iValue = 0;
	
	if (GC.getTechInfo(eTech).isDefensivePactTrading())
	{
		iValue += 50;
	}
	
	if (GC.getTechInfo(eTech).isCommerceFlexible(COMMERCE_CULTURE))
	{
		iValue += 100;
	}
	
	//units
	bool bAnyWarplan = (GET_TEAM(getTeam()).getAnyWarPlanCount(true) > 0);
	int iBestUnitValue = 0;
	for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
	{
		UnitTypes eLoopUnit = ((UnitTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI)));

		if (eLoopUnit != NO_UNIT)
		{
			if (isTechRequiredForUnit((eTech), eLoopUnit))
			{
				int iTempValue = (GC.getUnitInfo(eLoopUnit).getCombat() * 100) / max(1, (GC.getGame().getBestLandUnitCombat()));
				iTempValue *= bAnyWarplan ? 2 : 1;
				
				iValue += iTempValue / 3;
				iBestUnitValue = max(iBestUnitValue, iTempValue);
			}
		}
	}
	iValue += max(0, iBestUnitValue - 15);
	
	//cultural things
	for (iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
	{
		BuildingTypes eLoopBuilding = ((BuildingTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iI)));

		if (eLoopBuilding != NO_BUILDING)
		{
			if (isTechRequiredForBuilding((eTech), eLoopBuilding))
			{
				CvBuildingInfo& kLoopBuilding = GC.getBuildingInfo(eLoopBuilding);

				if ((GC.getBuildingClassInfo((BuildingClassTypes)iI).getDefaultBuildingIndex()) != (GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iI)))
				{
					//UB
					iValue += 100;
				}
				
				iValue += (150 * kLoopBuilding.getCommerceChange(COMMERCE_CULTURE)) * 20;
				iValue += kLoopBuilding.getCommerceModifier(COMMERCE_CULTURE) * 2;
			}
		}
	}
	
	//important civics
	for (iI = 0; iI < GC.getNumCivicInfos(); iI++)
	{
		if (GC.getCivicInfo((CivicTypes)iI).getTechPrereq() == eTech)
		{
			iValue += GC.getCivicInfo((CivicTypes)iI).getCommerceModifier(COMMERCE_CULTURE) * 2;
		}
	}
	
	return iValue;
}

int CvPlayerAI::AI_getCultureVictoryStage()
{
    int iValue;
	
    if (!GC.getGameINLINE().culturalVictoryValid())
    {
        return 0;
    }

    if (getCapitalCity() == NULL)
    {
        return 0;
    }

    if (getCapitalCity()->getGameTurnFounded() > (10 + GC.getGameINLINE().getStartTurn()))
    {
        //the loss of the capital indicates it might be a good idea to abort any culture victory
        return 0;
    }

    iValue = 0;

    for (int iI = 0; iI < GC.getNumTraitInfos(); iI++)
    {
        if (hasTrait((TraitTypes)iI))
        {
            // spiritual +4
			iValue += ((GC.getTraitInfo((TraitTypes)iI).getMaxAnarchy() == 0) ? 4 : 0);

			// creative +4
            iValue += (GC.getTraitInfo((TraitTypes)iI).getCommerceChange(COMMERCE_CULTURE) * 2);

			// (none in default xml), set so 25% boost = 4
            iValue += (GC.getTraitInfo((TraitTypes)iI).getCommerceModifier(COMMERCE_CULTURE) / 6);

			// philisophical +4
            iValue += (GC.getTraitInfo((TraitTypes)iI).getGreatPeopleRateModifier() / 25);

			// financial +4
            iValue += (GC.getTraitInfo((TraitTypes)iI).getExtraYieldThreshold(YIELD_COMMERCE)) * 2 * GC.getDefineINT("EXTRA_YIELD");
            
			// (none in default xml), set so 100% boost = 4
			iValue += (GC.getTraitInfo((TraitTypes)iI).getTradeYieldModifier(YIELD_COMMERCE)) / 25;
            
			// industrious +4
			iValue += ((GC.getTraitInfo((TraitTypes)iI).getMaxGlobalBuildingProductionModifier() / 25));
        }
    }
    
    iValue += (GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI) ? -3 : 0);
	
	CivicTypes eCivic = (CivicTypes)GC.getLeaderHeadInfo(getPersonalityType()).getFavoriteCivic();
	if ((eCivic != NO_CIVIC) && (GC.getCivicInfo(eCivic).isNoNonStateReligionSpread()))
	{
		iValue -= 3;		
	}
	
	iValue += countHolyCities();
	if ((GET_TEAM(getTeam()).isAVassal()) && (getNumCities() > 5))
	{
		int iReligionCount = countTotalHasReligion();
		if (((iReligionCount * 100) / getNumCities()) > 250)
		{
			iValue += 1;
			iValue += ((2 * iReligionCount) + 1) / getNumCities();
		}
	}

    //atleast I'm honest
    int iNonsense = 0;
    iNonsense += getCapitalCity()->getX();
    iNonsense += getCapitalCity()->getY();

    iValue += (iNonsense % 7);

    if (iValue < 10)
    {
        return 0;
    }
    
    if (getCurrentEra() >= (GC.getNumEraInfos() - (2 + iNonsense % 2)))
    {
        if (countTotalHasReligion() >= getNumCities() * 3)
        {
            return 3;
        }

		int iHighCultureCount = 0;
		
		int iLoop;
		CvCity* pLoopCity;
		for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
		{
			// is culture rate over 100?
			if (pLoopCity->getBaseCommerceRate(COMMERCE_CULTURE) > 100)
			{
				iHighCultureCount++;
			}
			// or is the culture level 1 less than required to win?
			else if (pLoopCity->getCultureLevel() >= (GC.getGameINLINE().culturalVictoryCultureLevel() - 1))
			{
				iHighCultureCount++;
			}
		}
		
		// if we have enough high culture cities, go to stage 3
		if (iHighCultureCount >= GC.getGameINLINE().culturalVictoryNumCultureCities())
		{
			return 3;
		}
    }
	
	if (getCurrentEra() >= ((GC.getNumEraInfos() / 3) + iNonsense % 2))
	{
	    return 2;
	}
	else
	{
        return 1;
	}
	FAssert(false);
}


bool CvPlayerAI::AI_isDoStrategy(int iStrategy)
{
    if (isHuman())
    {
        return false;
    }
    return (iStrategy & AI_getStrategyHash());
}

void CvPlayerAI::AI_forceUpdateStrategies()
{
	//this forces a recache.
	m_iStrategyHashCacheTurn = -1;
}

int CvPlayerAI::AI_getStrategyHash()
{
    if ((m_iStrategyHash != 0) && (m_iStrategyHashCacheTurn == GC.getGameINLINE().getGameTurn()))
    {
        return m_iStrategyHash;        
    }
    
    const FlavorTypes AI_FLAVOR_MILITARY = (FlavorTypes)0;
	const FlavorTypes AI_FLAVOR_RELIGION = (FlavorTypes)1;
    const FlavorTypes AI_FLAVOR_PRODUCTION = (FlavorTypes)2;
    const FlavorTypes AI_FLAVOR_GOLD = (FlavorTypes)3;
    const FlavorTypes AI_FLAVOR_SCIENCE = (FlavorTypes)4;
    const FlavorTypes AI_FLAVOR_CULTURE = (FlavorTypes)5;
    const FlavorTypes AI_FLAVOR_GROWTH = (FlavorTypes)6;
    
    int iI, iJ, iK;
    UnitTypes eLoopUnit;
    
    m_iStrategyHash = AI_DEFAULT_STRATEGY;
    m_iStrategyHashCacheTurn = GC.getGameINLINE().getGameTurn();
    
	if (AI_getFlavorValue(AI_FLAVOR_PRODUCTION) >= 2) // 0, 2, 5 or 10 in default xml [augustus 5, frederick 10, huayna 2, jc 2, chinese leader 2, qin 5, ramsess 2, roosevelt 5, stalin 2]
	{
		m_iStrategyHash |= AI_STRATEGY_PRODUCTION;
	}
	
	if (getCapitalCity() == NULL)
    {
        return m_iStrategyHash;
    }
    
    int iNonsense = 0;
    iNonsense += getCapitalCity()->getX();
    iNonsense += getCapitalCity()->getY();
    
    int iCurrentEra = getCurrentEra();
    int iMetCount = GET_TEAM(getTeam()).getHasMetCivCount(true);
    int iParanoia = 0;
    
    //peace
    if (iMetCount == 0)
    {
    	iParanoia = 0;
    }
    else   
    {
    	for (iI = 0; iI < MAX_PLAYERS; iI++)
    	{
    		if ((iI != getID()) && GET_PLAYER((PlayerTypes)iI).isAlive())
    		{
    			if (!GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).isAVassal())
    			{
					int iCloseness = AI_playerCloseness((PlayerTypes)iI, 5 + iCurrentEra);
					if (iCloseness > 0)
					{
						int iWarMemory = GET_PLAYER((PlayerTypes)iI).AI_getMemoryAttitude(getID(), MEMORY_DECLARED_WAR);
						if (iWarMemory > 0)
						{
							//they are a snake
							iParanoia += 50 + 20 * iWarMemory;
							break;							
						}
						
						if (AI_getAttitude((PlayerTypes)iI) < ATTITUDE_PLEASED)
						{
							iParanoia += 30;
						}
					
						//perform some personality profiling
						for (iJ = 0; iJ < MAX_PLAYERS; iJ++)
						{
							//XXX the alive check means it doesn't count murder victims, should it?
							if ((iJ != getID()) && (iI != iJ) && GET_PLAYER((PlayerTypes)iJ).isAlive() 								
							&& GET_TEAM(getTeam()).isHasMet((TeamTypes)iJ) && !GET_TEAM(GET_PLAYER((PlayerTypes)iJ).getTeam()).isAVassal())
							{
								if (GET_PLAYER((PlayerTypes)iI).AI_getMemoryAttitude((PlayerTypes)iJ, MEMORY_DECLARED_WAR) > 0)
								{
									//Do we not hate the victim?
									if (AI_getAttitude((PlayerTypes)iJ) >= ATTITUDE_ANNOYED)
									{
										iParanoia += (iCloseness < 15) ? 60 : 100;
									}
								}
							}
						}
					}
				}
    		}
    	}
    }
    
    iParanoia -= iCurrentEra * 10;
    
    if (iParanoia < ((iMetCount > 1) ? 100 : 50))
    {
    	m_iStrategyHash |= AI_STRATEGY_PEACE;
    }
    
    //Unit Analysis
    int iAttackUnitCount = 0;
	for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
	{
		eLoopUnit = ((UnitTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI)));

		if (eLoopUnit != NO_UNIT && 
			(GC.getUnitInfo(eLoopUnit).getUnitAIType(UNITAI_RESERVE) || GC.getUnitInfo(eLoopUnit).getUnitAIType(UNITAI_ATTACK_CITY)
			|| GC.getUnitInfo(eLoopUnit).getUnitAIType(UNITAI_COUNTER) || GC.getUnitInfo(eLoopUnit).getUnitAIType(UNITAI_PILLAGE)))
		{
			if (getCapitalCity() != NULL)
			{
				if (getCapitalCity()->canTrain(eLoopUnit))
				{
					iAttackUnitCount++;
					
					//UU love
					if ((GC.getUnitClassInfo((UnitClassTypes)iI).getDefaultUnitIndex()) != (GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI)))
					{
						if (GC.getUnitInfo(eLoopUnit).getUnitAIType(UNITAI_ATTACK_CITY)
							|| (GC.getUnitInfo(eLoopUnit).getUnitAIType(UNITAI_ATTACK) && !GC.getUnitInfo(eLoopUnit).getUnitAIType(UNITAI_CITY_DEFENSE)))
						{
							iAttackUnitCount++;					
						}
					}
				}
			}
		}
	}
	
	if (iAttackUnitCount <= 1)
	{
		m_iStrategyHash |= AI_STRATEGY_GET_BETTER_UNITS;
	}
    
    
	//missionary
	{
	    if (getStateReligion() != NO_RELIGION)
	    {
            int iHolyCityCount = countHolyCities();
            if ((iHolyCityCount > 0) && hasHolyCity(getStateReligion()))
            {
                int iMissionary = 0;
                //Missionary
                iMissionary += AI_getFlavorValue(AI_FLAVOR_GROWTH) * 2; // up to 10
                iMissionary += AI_getFlavorValue(AI_FLAVOR_CULTURE) * 4; // up to 40
                iMissionary += AI_getFlavorValue(AI_FLAVOR_RELIGION) * 6; // up to 60
                
                CivicTypes eCivic = (CivicTypes)GC.getLeaderHeadInfo(getPersonalityType()).getFavoriteCivic();
                if ((eCivic != NO_CIVIC) && (GC.getCivicInfo(eCivic).isNoNonStateReligionSpread()))
                {
                	iMissionary += 20;
                }
                
                iMissionary += (iHolyCityCount - 1) * 5;
                
                iMissionary += iMetCount * 7;
                
                for (iI = 0; iI < MAX_PLAYERS; iI++)
                {
                    if (GET_PLAYER((PlayerTypes)iI).isAlive())
                    {
                        if (iI != getID())
                        {
                            if (GET_TEAM(getTeam()).isOpenBorders(GET_PLAYER((PlayerTypes)iI).getTeam()))
                            {
                                iMissionary += (GET_PLAYER((PlayerTypes)iI).countHolyCities() == 0) ? 12 : 4;
                            }
                            if ((GET_PLAYER((PlayerTypes)iI).getStateReligion() == getStateReligion()))
                            {
                                iMissionary += 10;
                            }
                        }
                    }
                }
                
                iMissionary += (iNonsense % 7) * 3;
                
                if (iMissionary > 100)
                {
                    m_iStrategyHash |= AI_STRATEGY_MISSIONARY;
                }
            }
	    }
	}
	
    //dagger
    if (!(m_iStrategyHash & AI_STRATEGY_MISSIONARY)
     && ((iCurrentEra <= (2+(iNonsense%2)) && (iMetCount > 0)) && (iParanoia > 0)))
    {	    
	    int iDagger = 0;
	    iDagger += 12000 / max(100, (50 + GC.getLeaderHeadInfo(getPersonalityType()).getMaxWarRand()));
	    iDagger *= (iNonsense % 11);
	    iDagger /= 10;
	    iDagger += 5 * min(8, AI_getFlavorValue(AI_FLAVOR_MILITARY));
	    
        for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
        {
            eLoopUnit = ((UnitTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI)));

            if ((eLoopUnit != NO_UNIT) && (GC.getUnitInfo(eLoopUnit).getCombat() > 0))
            {
                if ((GC.getUnitClassInfo((UnitClassTypes)iI).getDefaultUnitIndex()) != (GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI)))
                {
                	bool bIsDefensive = (GC.getUnitInfo(eLoopUnit).getUnitAIType(UNITAI_CITY_DEFENSE) &&
                		!GC.getUnitInfo(eLoopUnit).getUnitAIType(UNITAI_RESERVE));
					
					iDagger += bIsDefensive ? -10 : 0;
                       
                    if (getCapitalCity()->canTrain(eLoopUnit))
                    {
                        iDagger += bIsDefensive ? 10 : 40;
                        
                        int iUUStr = GC.getUnitInfo(eLoopUnit).getCombat();
                        int iNormalStr = GC.getUnitInfo((UnitTypes)(GC.getUnitClassInfo((UnitClassTypes)iI).getDefaultUnitIndex())).getCombat();
                        iDagger += 20 * range((iUUStr - iNormalStr), 0, 2);
						if (GC.getUnitInfo(eLoopUnit).getPrereqAndTech() == NO_TECH)
                        {
                            iDagger += 20;
                        }
                    }
                    else
                    {
                        if (GC.getUnitInfo(eLoopUnit).getPrereqAndTech() != NO_TECH)
                        {
                            if (GC.getTechInfo((TechTypes)(GC.getUnitInfo(eLoopUnit).getPrereqAndTech())).getEra() <= (iCurrentEra + 1))
                            {
                                if (GET_TEAM(getTeam()).isHasTech((TechTypes)GC.getUnitInfo(eLoopUnit).getPrereqAndTech()))
                                {
                                	//we have the tech but can't train the unit, dejection.
                                    iDagger += 10;
                                }
                                else
                                {
                                	//we don't have the tech, it's understandable we can't train.
                                    iDagger += 30;
                                }
                            }
                        }
                                
                        bool bNeedsAndBonus = false;
                        int iOrBonusCount = 0;
                        int iOrBonusHave = 0;
                        
                        for (iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
                        {
                            BonusTypes eBonus = (BonusTypes)iJ;
                            if (eBonus != NO_BONUS)
                            {
                                if (GC.getUnitInfo(eLoopUnit).getPrereqAndBonus() == eBonus)
                                {
                                    if (getNumTradeableBonuses(eBonus) == 0)
                                    {
                                        bNeedsAndBonus = true;
                                    }
                                }

                                for (iK = 0; iK < GC.getNUM_UNIT_PREREQ_OR_BONUSES(); iK++)
                                {
                                    if (GC.getUnitInfo(eLoopUnit).getPrereqOrBonuses(iK) == eBonus)
                                    {
                                        iOrBonusCount++;
                                        if (getNumTradeableBonuses(eBonus) > 0)
                                        {
                                            iOrBonusHave++;
                                        }
                                    }
                                }
                            }
                        }
                        
                        
                        iDagger += 20;
                        if (bNeedsAndBonus)
                        {
                            iDagger -= 20;
                        }
                        if ((iOrBonusCount > 0) && (iOrBonusHave == 0))
                        {
                            iDagger -= 20;
                        }
                    }
                }
            }
        }
        
        //iDagger += (GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI) ? 20 : 0);
        iDagger -= ((iAttackUnitCount == 0) ? 40 : 0);
        
        if ((getCapitalCity()->area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE) || (getCapitalCity()->area()->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE))
        {
            iDagger += (iAttackUnitCount > 0) ? 40 : 20;
        }
        
        if (iDagger >= AI_DAGGER_THRESHOLD)
        {
            m_iStrategyHash |= AI_STRATEGY_DAGGER;            
        }
	}
	
	int iCultureStage = AI_getCultureVictoryStage();
	if (iCultureStage > 0)
	{
	    m_iStrategyHash |= AI_STRATEGY_CULTURE1;
	    if (iCultureStage > 1)
	    {
	        m_iStrategyHash |= AI_STRATEGY_CULTURE2;	        
            if (iCultureStage > 2)
            {
                m_iStrategyHash |= AI_STRATEGY_CULTURE3;                
                if (AI_cultureVictoryTechValue(getCurrentResearch()) < 100)
                {
                	m_iStrategyHash |= AI_STRATEGY_CULTURE4; 
                }
            }
	    }
	}

	// turn off dagger if culture gets to 2
	if (m_iStrategyHash & AI_STRATEGY_CULTURE2)
	{
		m_iStrategyHash &= ~AI_STRATEGY_DAGGER;            
	}
	
	
	{//Crush
		int iWarCount = 0;
		int iCrushValue = 0;
		
		iCrushValue += (iNonsense % 4);
		
		if (m_iStrategyHash & AI_STRATEGY_DAGGER)
		{
			iCrushValue += 3;			
		}
		if (GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI))
		{
			iCrushValue += 3;
		}

		for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
		{
			if ((GET_TEAM((TeamTypes)iI).isAlive()) && (iI != getID()))
			{
				if (GET_TEAM(getTeam()).AI_getWarPlan((TeamTypes)iI) != NO_WARPLAN)
				{
					if (!GET_TEAM((TeamTypes)iI).isAVassal())
					{
						if (GET_TEAM(getTeam()).AI_teamCloseness((TeamTypes)iI) > 0)
						{
							iWarCount++;
						}
					}
					
					if (GET_TEAM(getTeam()).AI_getWarPlan((TeamTypes)iI) == WARPLAN_PREPARING_TOTAL)
					{
						iCrushValue += 6;					
					}
					else if ((GET_TEAM(getTeam()).AI_getWarPlan((TeamTypes)iI) == WARPLAN_TOTAL) && (GET_TEAM(getTeam()).AI_getWarPlanStateCounter((TeamTypes)iI) < 20))
					{
						iCrushValue += 6;						
					}
					
					if ((GET_TEAM(getTeam()).AI_getWarPlan((TeamTypes)iI) == WARPLAN_DOGPILE) && (GET_TEAM(getTeam()).AI_getWarPlanStateCounter((TeamTypes)iI) < 20))
					{
						for (iJ = 0; iJ < MAX_TEAMS; iJ++)
						{
							if ((iJ != iI) && iJ != getID())
							{
								if ((atWar((TeamTypes)iI, (TeamTypes)iJ)) && !GET_TEAM((TeamTypes)iI).isAVassal())
								{
									iCrushValue += 4;
								}
							}
						}
					}
				}
			}
		}
		if ((iWarCount <= 1) && (iCrushValue >= 10))
		{
			m_iStrategyHash |= AI_STRATEGY_CRUSH;
		}
	}
	
	return m_iStrategyHash;   
}


void CvPlayerAI::AI_nowHasTech(TechTypes eTech)
{
	// while its _possible_ to do checks, for financial trouble, and this tech adds financial buildings
	// if in war and this tech adds important war units
	// etc
	// it makes more sense to just redetermine what to produce
	// that is already done every time a civ meets a new civ, it makes sense to do it when a new tech is learned
	// if this is changed, then at a minimum, AI_isFinancialTrouble should be checked
	if (!isHuman())
	{
		int iGameTurn = GC.getGameINLINE().getGameTurn();
		
		// only reset at most every 10 turns
		if (iGameTurn > m_iTurnLastProductionDirty + 10)
		{
			// redeterimine the best things to build in each city
			AI_makeProductionDirty();
		
			m_iTurnLastProductionDirty = iGameTurn;
		}
	}

}


int CvPlayerAI::AI_countDeadlockedBonuses(CvPlot* pPlot)
{
    CvPlot* pLoopPlot;
    CvPlot* pLoopPlot2;
    int iDX, iDY;
    int iI;
    
    int iMinRange = GC.getDefineINT("MIN_CITY_RANGE");
    int iRange = iMinRange * 2;
    int iCount = 0;

    for (iDX = -(iRange); iDX <= iRange; iDX++)
    {
        for (iDY = -(iRange); iDY <= iRange; iDY++)
        {
            if (plotDistance(iDX, iDY, 0, 0) > CITY_PLOTS_RADIUS)
            {
                pLoopPlot = plotXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iDX, iDY);

                if (pLoopPlot != NULL)
                {
                    if (pLoopPlot->getBonusType(getTeam()) != NO_BONUS)
                    {
                        if (!pLoopPlot->isCityRadius() && ((pLoopPlot->area() == pPlot->area()) || pLoopPlot->isWater()))
                        {
                            bool bCanFound = false;
                            bool bNeverFound = true;
                            //potentially blockable resource
                            //look for a city site within a city radius
                            for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
                            {
                                pLoopPlot2 = plotCity(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), iI);
                                if (pLoopPlot2 != NULL)
                                {
                                    //canFound usually returns very quickly
                                    if (canFound(pLoopPlot2->getX_INLINE(), pLoopPlot2->getY_INLINE(), false))
                                    {
                                        bNeverFound = false;
                                        if (stepDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pLoopPlot2->getX_INLINE(), pLoopPlot2->getY_INLINE()) > iMinRange)
                                        {
                                            bCanFound = true;
                                            break;
                                        }
                                    }
                                }
                            }
                            if (!bNeverFound && !bCanFound)
                            {
                                iCount++;
                            }
                        }
                    }
                }
            }
        }
    }
    
    return iCount;
}

int CvPlayerAI::AI_getOurPlotStrength(CvPlot* pPlot, int iRange, bool bDefensiveBonuses, bool bTestMoves)
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pLoopPlot;
	int iValue;
	int iDistance;
	int iDX, iDY;

	iValue = 0;

	for (iDX = -(iRange); iDX <= iRange; iDX++)
	{
		for (iDY = -(iRange); iDY <= iRange; iDY++)
		{
			pLoopPlot = plotXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (pLoopPlot->area() == pPlot->area())
				{
				    iDistance = stepDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE());
					pUnitNode = pLoopPlot->headUnitNode();

					while (pUnitNode != NULL)
					{
						pLoopUnit = ::getUnit(pUnitNode->m_data);
						pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);

						if (pLoopUnit->getOwnerINLINE() == getID())
						{
							if ((bDefensiveBonuses && pLoopUnit->canDefend()) || pLoopUnit->canAttack())
							{
								if (!(pLoopUnit->isInvisible(getTeam(), false)))
								{
								    if (pLoopUnit->atPlot(pPlot) || pLoopUnit->canMoveInto(pPlot) || pLoopUnit->canMoveInto(pPlot, /*bAttack*/ true))
								    {
                                        if (!bTestMoves)
                                        {
                                        	iValue += pLoopUnit->currEffectiveStr((bDefensiveBonuses ? pPlot : NULL), NULL);
                                        }
                                        else
                                        {
											if (pLoopUnit->baseMoves() >= iDistance)
                                            {
                                                iValue += pLoopUnit->currEffectiveStr((bDefensiveBonuses ? pPlot : NULL), NULL);
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


	return iValue;
}

int CvPlayerAI::AI_getEnemyPlotStrength(CvPlot* pPlot, int iRange, bool bDefensiveBonuses, bool bTestMoves)
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pLoopPlot;
	int iValue;
	int iDistance;
	int iDX, iDY;

	iValue = 0;

	for (iDX = -(iRange); iDX <= iRange; iDX++)
	{
		for (iDY = -(iRange); iDY <= iRange; iDY++)
		{
			pLoopPlot = plotXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (pLoopPlot->area() == pPlot->area())
				{
				    iDistance = stepDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE());
					pUnitNode = pLoopPlot->headUnitNode();

					while (pUnitNode != NULL)
					{
						pLoopUnit = ::getUnit(pUnitNode->m_data);
						pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);

						if (atWar(pLoopUnit->getTeam(), getTeam()))
						{
							if ((bDefensiveBonuses && pLoopUnit->canDefend()) || pLoopUnit->canAttack())
							{
								if (!(pLoopUnit->isInvisible(getTeam(), false)))
								{
								    if (pPlot->isValidDomainForAction(pLoopUnit->getDomainType()))
								    {
                                        if (!bTestMoves)
                                        {
                                            iValue += pLoopUnit->currEffectiveStr((bDefensiveBonuses ? pPlot : NULL), NULL);
                                        }
                                        else
                                        {
                                            int iDangerRange = pLoopUnit->baseMoves();
                                            iDangerRange += ((pLoopPlot->isValidRoute(pLoopUnit)) ? 1 : 0);
                                            if (iDangerRange >= iDistance)
                                            {
                                                iValue += pLoopUnit->currEffectiveStr((bDefensiveBonuses ? pPlot : NULL), NULL);
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


	return iValue;
	
}

int CvPlayerAI::AI_goldToUpgradeAllUnits(int iExpThreshold)
{
	if (m_iUpgradeUnitsCacheTurn == GC.getGameINLINE().getGameTurn() && m_iUpgradeUnitsCachedExpThreshold == iExpThreshold)
	{
		return m_iUpgradeUnitsCachedGold;
	}

	int iTotalGold = 0;
	
	CvCivilizationInfo& kCivilizationInfo = GC.getCivilizationInfo(getCivilizationType());
	
	// cache the value for each unit type
	std::vector<int> aiUnitUpgradePrice(GC.getNumUnitInfos(), 0);	// initializes to zeros

	int iLoop;
	for (CvUnit* pLoopUnit = firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = nextUnit(&iLoop))
	{
		// if experience is below threshold, skip this unit
		if (pLoopUnit->getExperience() < iExpThreshold)
		{
			continue;
		}
		
		UnitTypes eUnitType = pLoopUnit->getUnitType();

		// check cached value for this unit type
		int iCachedUnitGold = aiUnitUpgradePrice[eUnitType];
		if (iCachedUnitGold != 0)
		{
			// if positive, add it to the sum
			if (iCachedUnitGold > 0)
			{
				iTotalGold += iCachedUnitGold;
			}
			
			// either way, done with this unit
			continue;
		}
		
		int iUnitGold = 0;
		int iUnitUpgradePossibilities = 0;
		
		UnitAITypes eUnitAIType = pLoopUnit->AI_getUnitAIType();
		CvArea* pUnitArea = pLoopUnit->area();
		int iUnitValue = AI_unitValue(eUnitType, eUnitAIType, pUnitArea);

		for (int iI = 0; iI < GC.getNumUnitInfos(); iI++)
		{
			UnitTypes eUpgradeUnitType = (UnitTypes) iI;
			UnitClassTypes eUpgradeUnitClassType = (UnitClassTypes) GC.getUnitInfo(eUpgradeUnitType).getUnitClassType();
			
			// is this unit the correct one of this class for this civilization?
			if (kCivilizationInfo.getCivilizationUnits(eUpgradeUnitClassType) == eUpgradeUnitType)
			{
				// is it better?
				int iUpgradeValue = AI_unitValue(eUpgradeUnitType, eUnitAIType, pUnitArea);
				if (iUpgradeValue > iUnitValue)
				{
					// is this a valid upgrade?
					if (pLoopUnit->upgradeAvailable(eUnitType, eUpgradeUnitClassType))
					{
						// can we actually make this upgrade?
						bool bCanUpgrade = false;
						CvCity* pCapitalCity = getCapitalCity();
						if (pCapitalCity != NULL && pCapitalCity->canTrain(eUpgradeUnitType))
						{
							bCanUpgrade = true;
						}
						else
						{
							CvCity* pCloseCity = GC.getMapINLINE().findCity(pLoopUnit->getX_INLINE(), pLoopUnit->getY_INLINE(), NO_PLAYER, getTeam(), true, (pLoopUnit->getDomainType() == DOMAIN_SEA));
							if (pCloseCity != NULL && pCloseCity->canTrain(eUpgradeUnitType))
							{
								bCanUpgrade = true;
							}
						}

						if (bCanUpgrade)
						{
							iUnitGold += pLoopUnit->upgradePrice(eUpgradeUnitType);
							iUnitUpgradePossibilities++; 
						}
					}
				}
			}
		}
		
		// if we found any, find average and add to total
		if (iUnitUpgradePossibilities > 0)
		{
			iUnitGold /= iUnitUpgradePossibilities;

			// add to cache
			aiUnitUpgradePrice[eUnitType] = iUnitGold;

			// add to sum
			iTotalGold += iUnitGold;
		}
		else
		{
			// add to cache, dont upgrade to this type
			aiUnitUpgradePrice[eUnitType] = -1;
		}
	}

	m_iUpgradeUnitsCacheTurn = GC.getGameINLINE().getGameTurn();
	m_iUpgradeUnitsCachedExpThreshold = iExpThreshold;
	m_iUpgradeUnitsCachedGold = iTotalGold;

	return iTotalGold;
}

int CvPlayerAI::AI_goldTradeValuePercent()
{
	int iValue = 2;
	if (AI_isFinancialTrouble())
	{
		iValue += 1;
	}
	return 100 * iValue;
	
}

int CvPlayerAI::AI_averageYieldMultiplier(YieldTypes eYield)
{
	FAssert(eYield > -1);
	FAssert(eYield < NUM_YIELD_TYPES);
	
	if (m_iAveragesCacheTurn != GC.getGameINLINE().getGameTurn())
	{
		AI_calculateAverages();
	}
	
	FAssert(m_aiAverageYieldMultiplier[eYield] > 0);
	return m_aiAverageYieldMultiplier[eYield];
}

int CvPlayerAI::AI_averageCommerceMultiplier(CommerceTypes eCommerce)
{
	FAssert(eCommerce > -1);
	FAssert(eCommerce < NUM_COMMERCE_TYPES);
	
	if (m_iAveragesCacheTurn != GC.getGameINLINE().getGameTurn())
	{
		AI_calculateAverages();
	}
	
	return m_aiAverageCommerceMultiplier[eCommerce];	
}

int CvPlayerAI::AI_averageGreatPeopleMultiplier()
{
	if (m_iAveragesCacheTurn != GC.getGameINLINE().getGameTurn())
	{
		AI_calculateAverages();
	}
	return m_iAverageGreatPeopleMultiplier;	
}

//"100 eCommerce is worth (return) raw YIELD_COMMERCE
int CvPlayerAI::AI_averageCommerceExchange(CommerceTypes eCommerce)
{
	FAssert(eCommerce > -1);
	FAssert(eCommerce < NUM_COMMERCE_TYPES);
	
	if (m_iAveragesCacheTurn != GC.getGameINLINE().getGameTurn())
	{
		AI_calculateAverages();
	}
	
	return m_aiAverageCommerceExchange[eCommerce];
}

void CvPlayerAI::AI_calculateAverages()
{
	CvCity* pLoopCity;
	int iLoop;
	int iI;
	
	int iPopulation;
	int iTotalPopulation;

	for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		m_aiAverageYieldMultiplier[iI] = 0;		
	}
	for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		m_aiAverageCommerceMultiplier[iI] = 0;	
	}
	m_iAverageGreatPeopleMultiplier = 0;
	
	iTotalPopulation = 0;
	
	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		iPopulation = max(pLoopCity->getPopulation(), NUM_CITY_PLOTS);
		iTotalPopulation += iPopulation;
			
		for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
		{
			m_aiAverageYieldMultiplier[iI] += iPopulation * pLoopCity->AI_yieldMultiplier((YieldTypes)iI);
		}
		for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
		{
			m_aiAverageCommerceMultiplier[iI] += iPopulation * pLoopCity->getTotalCommerceRateModifier((CommerceTypes)iI);
		}
		m_iAverageGreatPeopleMultiplier += iPopulation * pLoopCity->getTotalGreatPeopleRateModifier();
	}
	
	
	if (iTotalPopulation > 0)
	{
		for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
		{
			m_aiAverageYieldMultiplier[iI] /= iTotalPopulation;
			FAssert(m_aiAverageYieldMultiplier[iI] > 0);
		}
		for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
		{
			m_aiAverageCommerceMultiplier[iI] /= iTotalPopulation;	
			FAssert(m_aiAverageCommerceMultiplier[iI] > 0);	
		}
		m_iAverageGreatPeopleMultiplier /= iTotalPopulation;
		FAssert(m_iAverageGreatPeopleMultiplier > 0);
	}
	else
	{
		for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
		{
			m_aiAverageYieldMultiplier[iI] = 100;
		}
		for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
		{
			m_aiAverageCommerceMultiplier[iI] = 100;
		}
		m_iAverageGreatPeopleMultiplier = 100;
	}
	
	
	//Calculate Exchange Rate
	
	for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		m_aiAverageCommerceExchange[iI] = 0;		
	}
	
	int iCommerce = 0;
	int iTotalCommerce = 0;
	
	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		iCommerce = pLoopCity->getYieldRate(YIELD_COMMERCE);
		iTotalCommerce += iCommerce;
		
		int iExtraCommerce = 0;
		for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
		{
			iExtraCommerce +=((pLoopCity->getSpecialistPopulation() + pLoopCity->getNumGreatPeople()) * getSpecialistExtraCommerce((CommerceTypes)iI));
			iExtraCommerce += (pLoopCity->getBuildingCommerce((CommerceTypes)iI) + pLoopCity->getSpecialistCommerce((CommerceTypes)iI) + pLoopCity->getReligionCommerce((CommerceTypes)iI) + getFreeCityCommerce((CommerceTypes)iI));
		}
		iTotalCommerce += iExtraCommerce;
		
		for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
		{
			m_aiAverageCommerceExchange[iI] += ((iCommerce + iExtraCommerce) * pLoopCity->getTotalCommerceRateModifier((CommerceTypes)iI)) / 100;		
		}
	}

	for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		if (m_aiAverageCommerceExchange[iI] > 0)
		{
			m_aiAverageCommerceExchange[iI] = (100 * iTotalCommerce) / m_aiAverageCommerceExchange[iI];
		}
		else
		{
			m_aiAverageCommerceExchange[iI] = 100;
		}
	}

	m_iAveragesCacheTurn = GC.getGameINLINE().getGameTurn();
}

void CvPlayerAI::AI_convertUnitAITypesForCrush()
{
	CvUnit* pLoopUnit;
	
	int iLoop;
	
	for(pLoopUnit = firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = nextUnit(&iLoop))
	{
		bool bValid = false;
		if ((pLoopUnit->AI_getUnitAIType() == UNITAI_RESERVE)
			|| (pLoopUnit->AI_isCityAIType() && (pLoopUnit->getExtraCityDefensePercent() <= 0)))
		{
			bValid = true;
		}
		if ((pLoopUnit->area()->getAreaAIType(getTeam()) == AREAAI_ASSAULT)
			|| (pLoopUnit->area()->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE))
		{
			bValid = false;			
		}
	
		if (bValid)
		{
			if (pLoopUnit->plot()->isCity())
			{
				if (pLoopUnit->plot()->getPlotCity()->getOwner() == getID())
				{
					if (pLoopUnit->plot()->getBestDefender(getID()) == pLoopUnit)
					{
						bValid = false;
					}
				}
			}
		}
		
		if (bValid)
		{
			pLoopUnit->AI_setUnitAIType(UNITAI_ATTACK_CITY);			
		}
	}
}

int CvPlayerAI::AI_playerCloseness(PlayerTypes eIndex, int iMaxDistance)
{
	PROFILE_FUNC();
	CvCity* pLoopCity;
	int iLoop;
	int iValue;
	
	FAssert(GET_PLAYER(eIndex).isAlive());
	FAssert(eIndex != getID());
	
	iValue = 0;
	for (pLoopCity = GET_PLAYER(eIndex).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(eIndex).nextCity(&iLoop))
	{
		iValue += pLoopCity->AI_playerCloseness(eIndex, iMaxDistance);
	}
	
	return iValue;
}


int CvPlayerAI::AI_getTotalAreaCityThreat(CvArea* pArea)
{
	PROFILE_FUNC();
	CvCity* pLoopCity;
	int iLoop;
	int iValue;
	
	iValue = 0;
	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		if (pLoopCity->getArea() == pArea->getID())
		{
			iValue += pLoopCity->AI_cityThreat();
		}
	}
	return iValue;
}

int CvPlayerAI::AI_countNumAreaHostileUnits(CvArea* pArea, bool bLand, bool bWater)
{
	PROFILE_FUNC();
	CvPlot* pLoopPlot;
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);
		if (pLoopPlot->area() == pArea)
		{
			if ((pLoopPlot->getOwnerINLINE() == getID()) || pLoopPlot->isAdjacentPlayer(getID()))
			{
				iCount += pLoopPlot->getNumVisibleEnemyDefenders(getID());
			}
		}
	}
	return iCount;
}

//this doesn't include the minimal one or two garrison units in each city.
int CvPlayerAI::AI_getTotalFloatingDefendersNeeded(CvArea* pArea)
{
	PROFILE_FUNC();
	int iDefenders;
	int iCurrentEra = getCurrentEra();
	
	iDefenders = 1 + ((iCurrentEra + 2) * pArea->getCitiesPerPlayer(getID()));
	iDefenders /= 3;
	iDefenders += pArea->getPopulationPerPlayer(getID()) / 5;
	iDefenders += countHolyCities();
	
	iDefenders *= 100;
	iDefenders /= GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAITrainPercent();
	
	if (pArea->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE)
	{
		iDefenders *= 2;
	}
	else if (pArea->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE)
	{
		iDefenders *= 2;
		iDefenders /= 3;
	}
	
	if (AI_getTotalAreaCityThreat(pArea) == 0)
	{
		iDefenders /= 2;
	}
	
	if (!GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI))
	{
		iDefenders *= 2;
		iDefenders /= 3;
	}
	
	if (AI_isDoStrategy(AI_STRATEGY_CULTURE3))
	{
		iDefenders += 5 + getNumCities();
		if (pArea->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE)
		{
			iDefenders *= 2; //go crazy
		}
	}	
	
	if ((iCurrentEra < 3) && (GC.getGameINLINE().isOption(GAMEOPTION_RAGING_BARBARIANS)))
	{
		iDefenders += 2;
	}
		
	iDefenders = max(iDefenders, AI_countNumAreaHostileUnits(pArea));
	
	return iDefenders;
}

int CvPlayerAI::AI_getTotalFloatingDefenders(CvArea* pArea)
{
	PROFILE_FUNC();
	int iCount = 0;

	iCount += AI_totalAreaUnitAIs(pArea, UNITAI_COLLATERAL);
	iCount += AI_totalAreaUnitAIs(pArea, UNITAI_RESERVE);
	iCount += max(0, (AI_totalAreaUnitAIs(pArea, UNITAI_CITY_DEFENSE) - (pArea->getCitiesPerPlayer(getID()) * 2)));
	iCount += AI_totalAreaUnitAIs(pArea, UNITAI_CITY_COUNTER);
	iCount += AI_totalAreaUnitAIs(pArea, UNITAI_CITY_SPECIAL);
	return iCount;
}
	
//
// read object from a stream
// used during load
//
void CvPlayerAI::read(FDataStreamBase* pStream)
{
	CvPlayer::read(pStream);	// read base class data first

	uint uiFlag=0;
	pStream->Read(&uiFlag);	// flags for expansion

	pStream->Read(&m_iPeaceWeight);
	pStream->Read(&m_iAttackOddsChange);
	pStream->Read(&m_iCivicTimer);
	pStream->Read(&m_iReligionTimer);

	pStream->Read(NUM_UNITAI_TYPES, m_aiNumTrainAIUnits);
	pStream->Read(NUM_UNITAI_TYPES, m_aiNumAIUnits);
	pStream->Read(MAX_PLAYERS, m_aiSameReligionCounter);
	pStream->Read(MAX_PLAYERS, m_aiDifferentReligionCounter);
	pStream->Read(MAX_PLAYERS, m_aiFavoriteCivicCounter);
	pStream->Read(MAX_PLAYERS, m_aiBonusTradeCounter);
	pStream->Read(MAX_PLAYERS, m_aiPeacetimeTradeValue);
	pStream->Read(MAX_PLAYERS, m_aiPeacetimeGrantValue);
	pStream->Read(MAX_PLAYERS, m_aiGoldTradedTo);
	pStream->Read(MAX_PLAYERS, m_aiAttitudeExtra);

	pStream->Read(MAX_PLAYERS, m_abFirstContact);

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		pStream->Read(NUM_CONTACT_TYPES, m_aaiContactTimer[i]);
	}
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		pStream->Read(NUM_MEMORY_TYPES, m_aaiMemoryCount[i]);
	}

	CvExtraSaveData& extraSaveData = GC.getMapINLINE().getExtraSaveDataClass();
	int iVersion;
	if (extraSaveData.getSaveData(EXTRASAVEDATA_PLAYER, m_eID, 0, PLAYEREXTRADATA_VERSION, iVersion))
	{
		if (iVersion == PLAYEREXTRADATA_CURRENTVERSION)
		{
			//int iValue;
			//if (extraSaveData.getSaveData(EXTRASAVEDATA_PLAYER, m_eID, 0, PLAYEREXTRADATA_UNUSED1, iValue))
			//{
			//	m_unused1 = iValue;
			//}
			//if (extraSaveData.getSaveData(EXTRASAVEDATA_PLAYER, m_eID, 0, PLAYEREXTRADATA_UNUSED2, iValue))
			//{
			//	m_unused2 = iValue;
			//}
		}
	}
}


//
// save object to a stream
// used during save
//
void CvPlayerAI::write(FDataStreamBase* pStream)
{
	CvPlayer::write(pStream);	// write base class data first

	uint uiFlag=0;
	pStream->Write(uiFlag);		// flag for expansion

	pStream->Write(m_iPeaceWeight);
	pStream->Write(m_iAttackOddsChange);
	pStream->Write(m_iCivicTimer);
	pStream->Write(m_iReligionTimer);

	pStream->Write(NUM_UNITAI_TYPES, m_aiNumTrainAIUnits);
	pStream->Write(NUM_UNITAI_TYPES, m_aiNumAIUnits);
	pStream->Write(MAX_PLAYERS, m_aiSameReligionCounter);
	pStream->Write(MAX_PLAYERS, m_aiDifferentReligionCounter);
	pStream->Write(MAX_PLAYERS, m_aiFavoriteCivicCounter);
	pStream->Write(MAX_PLAYERS, m_aiBonusTradeCounter);
	pStream->Write(MAX_PLAYERS, m_aiPeacetimeTradeValue);
	pStream->Write(MAX_PLAYERS, m_aiPeacetimeGrantValue);
	pStream->Write(MAX_PLAYERS, m_aiGoldTradedTo);
	pStream->Write(MAX_PLAYERS, m_aiAttitudeExtra);

	pStream->Write(MAX_PLAYERS, m_abFirstContact);

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		pStream->Write(NUM_CONTACT_TYPES, m_aaiContactTimer[i]);
	}
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		pStream->Write(NUM_MEMORY_TYPES, m_aaiMemoryCount[i]);
	}
}

void CvPlayerAI::writeExtraSaveData(CvExtraSaveData& extraSaveData)
{
	int iVersion = PLAYEREXTRADATA_CURRENTVERSION;
	extraSaveData.setSaveData(EXTRASAVEDATA_PLAYER, m_eID, 0, PLAYEREXTRADATA_VERSION, iVersion);

	//int iUnused = 0;
	//extraSaveData.setSaveData(EXTRASAVEDATA_PLAYER, m_eID, 0, PLAYEREXTRADATA_UNUSED1, iUnused);
	
	//int iUnused = 0;
	//extraSaveData.setSaveData(EXTRASAVEDATA_PLAYER, m_eID, 0, PLAYEREXTRADATA_UNUSED2, iUnused);
}

