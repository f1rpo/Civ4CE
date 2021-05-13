//
// Python wrapper class for CvPlayer
//
#include "CvGameCoreDLL.h"
#include "CyPlayer.h"
#include "CyUnit.h"
#include "CyCity.h"
#include "CyArea.h"
#include "CyPlot.h"
#include "CvPlayerAI.h"
//#include "CvEnums.h"
#include "CvCity.h"
#include "CvMap.h"
#include "CvPlot.h"
#include "CySelectionGroup.h"
#include "CvDLLPythonIFaceBase.h"
#include "CvGlobals.h"
#include "CyTradeRoute.h"


CyPlayer::CyPlayer() : m_pPlayer(NULL)
{
}
CyPlayer::CyPlayer(CvPlayer* pPlayer) : m_pPlayer(pPlayer)
{
}
int CyPlayer::startingPlotRange()
{
	return m_pPlayer ? m_pPlayer->startingPlotRange() : -1;
}
CyPlot* CyPlayer::findStartingPlot(bool bRandomize)
{
	return m_pPlayer ? new CyPlot(m_pPlayer->findStartingPlot(bRandomize)) : NULL;
}
CyCity* CyPlayer::initCity(int x, int y)
{
	return m_pPlayer ? new CyCity(m_pPlayer->initCity(x, y, true)) : NULL;
}
void CyPlayer::acquireCity(CyCity* pCity, bool bConquest, bool bTrade)
{
	if (m_pPlayer)
		m_pPlayer->acquireCity(pCity->getCity(), bConquest, bTrade);
}
void CyPlayer::killCities()
{
	if (m_pPlayer)
		m_pPlayer->killCities();
}
std::wstring CyPlayer::getNewCityName()
{
	return m_pPlayer ? m_pPlayer->getNewCityName() : std::wstring();
}
CyUnit* CyPlayer::initUnit(int /*UnitTypes*/ iIndex, int iProfession, int iX, int iY, UnitAITypes eUnitAI, DirectionTypes eFacingDirection, int iYieldStored)
{
	return m_pPlayer ? new CyUnit(m_pPlayer->initUnit((UnitTypes) iIndex, (ProfessionTypes) iProfession, iX, iY, eUnitAI, eFacingDirection, iYieldStored)) : NULL;
}
CyUnit* CyPlayer::initEuropeUnit(int /*UnitTypes*/ eUnit, UnitAITypes eUnitAI, DirectionTypes eFacingDirection)
{
	return m_pPlayer ? new CyUnit(m_pPlayer->initEuropeUnit((UnitTypes) eUnit, eUnitAI, eFacingDirection)) : NULL;
}
void CyPlayer::killUnits()
{
	if (m_pPlayer)
		m_pPlayer->killUnits();
}
bool CyPlayer::hasTrait(int /*TraitTypes*/ iIndex)
{
	return m_pPlayer ? m_pPlayer->hasTrait((TraitTypes) iIndex) : false;
}
bool CyPlayer::isHuman()
{
	return m_pPlayer ? m_pPlayer->isHuman() : false;
}
bool CyPlayer::isNative()
{
	return m_pPlayer ? m_pPlayer->isNative() : false;
}
std::wstring CyPlayer::getName()
{
	return m_pPlayer ? m_pPlayer->getName() : std::wstring();
}
std::wstring CyPlayer::getNameForm(int iForm)
{
	return m_pPlayer ? m_pPlayer->getName((uint)iForm) : std::wstring();
}
std::wstring CyPlayer::getNameKey()
{
	return m_pPlayer ? m_pPlayer->getNameKey() : std::wstring();
}
std::wstring CyPlayer::getCivilizationDescription(int iForm)
{
	return m_pPlayer ? m_pPlayer->getCivilizationDescription((uint)iForm) : std::wstring();
}
std::wstring CyPlayer::getCivilizationDescriptionKey()
{
	return m_pPlayer ? m_pPlayer->getCivilizationDescriptionKey() : std::wstring();
}
std::wstring CyPlayer::getCivilizationShortDescription(int iForm)
{
	return m_pPlayer ? m_pPlayer->getCivilizationShortDescription((uint)iForm) : std::wstring();
}
std::wstring CyPlayer::getCivilizationShortDescriptionKey()
{
	return m_pPlayer ? m_pPlayer->getCivilizationShortDescriptionKey() : std::wstring();
}
std::wstring CyPlayer::getCivilizationAdjective(int iForm)
{
	return m_pPlayer ? m_pPlayer->getCivilizationAdjective((uint)iForm) : std::wstring();
}
std::wstring CyPlayer::getCivilizationAdjectiveKey( )
{
	return m_pPlayer ? m_pPlayer->getCivilizationAdjectiveKey() : std::wstring();
}
std::wstring CyPlayer::getWorstEnemyName()
{
	return m_pPlayer ? m_pPlayer->getWorstEnemyName() : std::wstring();
}
int /*ArtStyleTypes*/ CyPlayer::getArtStyleType()
{
	return m_pPlayer ? (int) m_pPlayer->getArtStyleType() : -1;
}
std::string CyPlayer::getUnitButton(int eUnit)
{
	return m_pPlayer ? m_pPlayer->getUnitButton((UnitTypes)eUnit) : "";
}
int CyPlayer::findBestFoundValue( )
{
	return m_pPlayer ? m_pPlayer->findBestFoundValue() : -1;
}
int CyPlayer::countNumCoastalCities()
{
	return m_pPlayer ? m_pPlayer->countNumCoastalCities() : -1;
}
int CyPlayer::countNumCoastalCitiesByArea(CyArea* pArea)
{
	return m_pPlayer ? m_pPlayer->countNumCoastalCitiesByArea(pArea->getArea()) : -1;
}
int CyPlayer::countTotalCulture()
{
	return m_pPlayer ? m_pPlayer->countTotalCulture() : -1;
}
int CyPlayer::countTotalYieldStored(int /*YieldTypes*/ eYield)
{
	return m_pPlayer ? m_pPlayer->countTotalYieldStored((YieldTypes) eYield) : -1;
}
int CyPlayer::countCityFeatures(int /*FeatureTypes*/ eFeature)
{
	return m_pPlayer ? m_pPlayer->countCityFeatures((FeatureTypes) eFeature) : -1;
}
int CyPlayer::countNumBuildings(int /*BuildingTypes*/ eBuilding)
{
	return m_pPlayer ? m_pPlayer->countNumBuildings((BuildingTypes) eBuilding) : -1;
}
bool CyPlayer::canContact(int /*PlayerTypes*/ ePlayer)
{
	return m_pPlayer ? m_pPlayer->canContact((PlayerTypes)ePlayer) : false;
}
void CyPlayer::contact(int /*PlayerTypes*/ ePlayer)
{
	if (m_pPlayer)
		m_pPlayer->contact((PlayerTypes)ePlayer);
}
bool CyPlayer::canTradeWith(int /*PlayerTypes*/ eWhoTo)
{
	return m_pPlayer ? m_pPlayer->canTradeWith((PlayerTypes)eWhoTo) : false;
}
bool CyPlayer::canTradeItem(int /*PlayerTypes*/ eWhoTo, TradeData item, bool bTestDenial)
{
	return m_pPlayer ? m_pPlayer->canTradeItem((PlayerTypes)eWhoTo, item, bTestDenial) : false;
}
DenialTypes CyPlayer::getTradeDenial(int /*PlayerTypes*/ eWhoTo, TradeData item)
{
	return m_pPlayer ? m_pPlayer->getTradeDenial((PlayerTypes)eWhoTo, item) : NO_DENIAL;
}
bool CyPlayer::canStopTradingWithTeam(int /*TeamTypes*/ eTeam)
{
	return m_pPlayer ? m_pPlayer->canStopTradingWithTeam((TeamTypes) eTeam) : false;
}
void CyPlayer::stopTradingWithTeam(int /*TeamTypes*/ eTeam)
{
	if (m_pPlayer)
		m_pPlayer->stopTradingWithTeam((TeamTypes) eTeam);
}
void CyPlayer::killAllDeals()
{
	if (m_pPlayer)
		m_pPlayer->killAllDeals();
}
bool CyPlayer::isTurnActive()
{
	return m_pPlayer ? m_pPlayer->isTurnActive() : false;
}
void CyPlayer::findNewCapital()
{
	if (m_pPlayer)
		m_pPlayer->findNewCapital();
}
bool CyPlayer::canRaze(CyCity* pCity)
{
	return m_pPlayer ? m_pPlayer->canRaze(pCity->getCity()) : false;
}
void CyPlayer::raze(CyCity* pCity)
{
	if (m_pPlayer)
		m_pPlayer->raze(pCity->getCity());
}
void CyPlayer::disband(CyCity* pCity)
{
	if (m_pPlayer)
		m_pPlayer->disband(pCity->getCity());
}
bool CyPlayer::canReceiveGoody(CyPlot* pPlot, int /*GoodyTypes*/ iIndex, CyUnit* pUnit)
{
	return m_pPlayer ? m_pPlayer->canReceiveGoody(pPlot->getPlot(), (GoodyTypes) iIndex, pUnit->getUnit()) : false;
}
void CyPlayer::receiveGoody(CyPlot* pPlot, int /*GoodyTypes*/ iIndex, CyUnit* pUnit)
{
	if (m_pPlayer)
		m_pPlayer->receiveGoody(pPlot->getPlot(), (GoodyTypes) iIndex, pUnit->getUnit());
}
void CyPlayer::doGoody(CyPlot* pPlot, CyUnit* pUnit)
{
	if (m_pPlayer)
		m_pPlayer->doGoody(pPlot->getPlot(), pUnit->getUnit());
}
bool CyPlayer::canFound(int iX, int iY)
{
	return m_pPlayer ? m_pPlayer->canFound(iX, iY) : false;
}
void CyPlayer::found(int x, int y)
{
	if (m_pPlayer)
		m_pPlayer->found(x,y);
}
bool CyPlayer::canTrain(int /*UnitTypes*/ eUnit, bool bContinue, bool bTestVisible)
{
	return m_pPlayer ? m_pPlayer->canTrain((UnitTypes)eUnit, bContinue, bTestVisible) : false;
}
bool CyPlayer::canConstruct(int /*BuildingTypes*/eBuilding, bool bContinue, bool bTestVisible, bool bIgnoreCost)
{
	return m_pPlayer ? m_pPlayer->canConstruct((BuildingTypes)eBuilding, bContinue, bTestVisible, bIgnoreCost) : false;
}
int CyPlayer::getUnitYieldProductionNeeded(int /*UnitTypes*/ eUnit, int /*YieldTypes*/ eYield) const
{
	return m_pPlayer ? m_pPlayer->getYieldProductionNeeded((UnitTypes) eUnit, (YieldTypes) eYield) : false;
}
int CyPlayer::getBuildingYieldProductionNeeded(int /*BuildingTypes*/ eBuilding, int /*YieldTypes*/ eYield) const
{
	return m_pPlayer ? m_pPlayer->getYieldProductionNeeded((BuildingTypes) eBuilding, (YieldTypes) eYield) : false;
}
int CyPlayer::getBuildingClassPrereqBuilding(int /*BuildingTypes*/ eBuilding, int /*BuildingClassTypes*/ ePrereqBuildingClass, int iExtra)
{
	return m_pPlayer ? m_pPlayer->getBuildingClassPrereqBuilding((BuildingTypes) eBuilding, (BuildingClassTypes) ePrereqBuildingClass, iExtra) : -1;
}
void CyPlayer::removeBuildingClass(int /*BuildingClassTypes*/ eBuildingClass)
{
	if (m_pPlayer)
		m_pPlayer->removeBuildingClass((BuildingClassTypes)eBuildingClass);
}
bool CyPlayer::canBuild(CyPlot* pPlot, int /*BuildTypes*/ eBuild, bool bTestEra, bool bTestVisible)
{
	return m_pPlayer ? m_pPlayer->canBuild(pPlot->getPlot(), (BuildTypes)eBuild, bTestEra, bTestVisible) : false;
}
int /*RouteTypes*/ CyPlayer::getBestRoute(CyPlot* pPlot) const
{
	return m_pPlayer ? (int) m_pPlayer->getBestRoute(NULL != pPlot ? pPlot->getPlot() : NULL) : -1;
}
int CyPlayer::getImprovementUpgradeRate() const
{
	return m_pPlayer ? m_pPlayer->getImprovementUpgradeRate() : -1;
}
int CyPlayer::calculateTotalYield(int /*YieldTypes*/ eYield)
{
	return m_pPlayer ? m_pPlayer->calculateTotalYield((YieldTypes)eYield) : -1;
}
bool CyPlayer::isCivic(int /*CivicTypes*/ eCivic)
{
	return m_pPlayer ? m_pPlayer->isCivic((CivicTypes)eCivic) : false;
}
bool CyPlayer::canDoCivics(int /*CivicTypes*/ eCivic)
{
	return m_pPlayer ? m_pPlayer->canDoCivics((CivicTypes)eCivic) : false;
}
int CyPlayer::greatGeneralThreshold()
{
	return m_pPlayer ? m_pPlayer->greatGeneralThreshold() : -1;
}
int CyPlayer::immigrationThreshold()
{
	return m_pPlayer ? m_pPlayer->immigrationThreshold() : -1;
}
int CyPlayer::educationThreshold()
{
	return m_pPlayer ? m_pPlayer->educationThreshold() : -1;
}
int CyPlayer::revolutionEuropeUnitThreshold()
{
	return m_pPlayer ? m_pPlayer->revolutionEuropeUnitThreshold() : -1;
}
CyPlot* CyPlayer::getStartingPlot()
{
	if (!m_pPlayer)
	{
		return NULL;
	}
	return new CyPlot(m_pPlayer->getStartingPlot());
}
void CyPlayer::setStartingPlot(CyPlot* pPlot, bool bUpdateStartDist)
{
	if (!m_pPlayer)
	{
		return;
	}
	m_pPlayer->setStartingPlot(NULL != pPlot ? pPlot->getPlot() : NULL, bUpdateStartDist);
}
int CyPlayer::getTotalPopulation()
{
	return m_pPlayer ? m_pPlayer->getTotalPopulation() : -1;
}
int CyPlayer::getAveragePopulation()
{
	return m_pPlayer ? m_pPlayer->getAveragePopulation() : -1;
}
long CyPlayer::getRealPopulation()
{
	return m_pPlayer ? m_pPlayer->getRealPopulation() : -1;
}
int CyPlayer::getTotalLand()
{
	return m_pPlayer ? m_pPlayer->getTotalLand() : -1;
}
int CyPlayer::getTotalLandScored()
{
	return m_pPlayer ? m_pPlayer->getTotalLandScored() : -1;
}
int CyPlayer::getGold()
{
	return m_pPlayer ? m_pPlayer->getGold() : -1;
}
void CyPlayer::setGold(int iNewValue)
{
	if (m_pPlayer)
		m_pPlayer->setGold(iNewValue);
}
void CyPlayer::changeGold(int iChange)
{
	if (m_pPlayer)
		m_pPlayer->changeGold(iChange);
}
int CyPlayer::getAdvancedStartPoints()
{
	return m_pPlayer ? m_pPlayer->getAdvancedStartPoints() : -1;
}
void CyPlayer::setAdvancedStartPoints(int iNewValue)
{
	if (m_pPlayer)
		m_pPlayer->setAdvancedStartPoints(iNewValue);
}
void CyPlayer::changeAdvancedStartPoints(int iChange)
{
	if (m_pPlayer)
		m_pPlayer->changeAdvancedStartPoints(iChange);
}
int CyPlayer::getAdvancedStartUnitCost(int /*UnitTypes*/ eUnit, bool bAdd, CyPlot* pPlot)
{
	return m_pPlayer ? m_pPlayer->getAdvancedStartUnitCost((UnitTypes) eUnit, bAdd, NULL != pPlot ? pPlot->getPlot() : NULL) : -1;
}
int CyPlayer::getAdvancedStartCityCost(bool bAdd, CyPlot* pPlot)
{
	return m_pPlayer ? m_pPlayer->getAdvancedStartCityCost(bAdd, NULL != pPlot ? pPlot->getPlot() : NULL) : -1;
}
int CyPlayer::getAdvancedStartPopCost(bool bAdd, CyCity* pCity)
{
	return m_pPlayer ? m_pPlayer->getAdvancedStartPopCost(bAdd, pCity->getCity()) : -1;
}
int CyPlayer::getAdvancedStartCultureCost(bool bAdd, CyCity* pCity)
{
	return m_pPlayer ? m_pPlayer->getAdvancedStartCultureCost(bAdd, pCity->getCity()) : -1;
}
int CyPlayer::getAdvancedStartBuildingCost(int /*BuildingTypes*/ eBuilding, bool bAdd, CyCity* pCity)
{
	return m_pPlayer ? m_pPlayer->getAdvancedStartBuildingCost((BuildingTypes) eBuilding, bAdd, pCity->getCity()) : -1;
}
int CyPlayer::getAdvancedStartImprovementCost(int /*ImprovementTypes*/ eImprovement, bool bAdd, CyPlot* pPlot)
{
	return m_pPlayer ? m_pPlayer->getAdvancedStartImprovementCost((ImprovementTypes) eImprovement, bAdd, NULL != pPlot ? pPlot->getPlot() : NULL) : -1;
}
int CyPlayer::getAdvancedStartRouteCost(int /*RouteTypes*/ eRoute, bool bAdd, CyPlot* pPlot)
{
	return m_pPlayer ? m_pPlayer->getAdvancedStartRouteCost((RouteTypes) eRoute, bAdd, NULL != pPlot ? pPlot->getPlot() : NULL) : -1;
}
int CyPlayer::getAdvancedStartVisibilityCost(bool bAdd, CyPlot* pPlot)
{
	return m_pPlayer ? m_pPlayer->getAdvancedStartVisibilityCost(bAdd, NULL != pPlot ? pPlot->getPlot() : NULL) : -1;
}
void CyPlayer::createGreatGeneral(int eGreatGeneralUnit, bool bIncrementExperience, int iX, int iY)
{
	if (m_pPlayer)
	{
		m_pPlayer->createGreatGeneral((UnitTypes)eGreatGeneralUnit, bIncrementExperience, iX, iY);
	}
}
int CyPlayer::getGreatGeneralsCreated()
{
	return m_pPlayer ? m_pPlayer->getGreatGeneralsCreated() : -1;
}
int CyPlayer::getGreatGeneralsThresholdModifier()
{
	return m_pPlayer ? m_pPlayer->getGreatGeneralsThresholdModifier() : -1;
}
int CyPlayer::getGreatGeneralRateModifier()
{
	return m_pPlayer ? m_pPlayer->getGreatGeneralRateModifier() : -1;
}
int CyPlayer::getDomesticGreatGeneralRateModifier()
{
	return m_pPlayer ? m_pPlayer->getDomesticGreatGeneralRateModifier() : -1;
}
int CyPlayer::getFreeExperience()
{
	return m_pPlayer ? m_pPlayer->getFreeExperience() : -1;
}
int CyPlayer::getWorkerSpeedModifier()
{
	return m_pPlayer ? m_pPlayer->getWorkerSpeedModifier() : -1;
}
int CyPlayer::getImprovementUpgradeRateModifier()
{
	return m_pPlayer ? m_pPlayer->getImprovementUpgradeRateModifier() : -1;
}
int CyPlayer::getMilitaryProductionModifier()
{
	return m_pPlayer ? m_pPlayer->getMilitaryProductionModifier() : -1;
}
int CyPlayer::getCityDefenseModifier()
{
	return m_pPlayer ? m_pPlayer->getCityDefenseModifier() : -1;
}
int CyPlayer::getHighestUnitLevel()
{
	return m_pPlayer ? m_pPlayer->getHighestUnitLevel() : -1;
}
bool CyPlayer::getExpInBorderModifier()
{
	return m_pPlayer ? m_pPlayer->getExpInBorderModifier() : false;
}
int CyPlayer::getLevelExperienceModifier() const
{
	return m_pPlayer ? m_pPlayer->getLevelExperienceModifier() : -1;
}
CyCity* CyPlayer::getCapitalCity()
{
	return m_pPlayer ? new CyCity(m_pPlayer->getCapitalCity()) : NULL;
}
int CyPlayer::getCitiesLost()
{
	return m_pPlayer ? m_pPlayer->getCitiesLost() : -1;
}
int CyPlayer::getAssets()
{
	return m_pPlayer ? m_pPlayer->getAssets() : -1;
}
void CyPlayer::changeAssets(int iChange)
{
	if (m_pPlayer)
		m_pPlayer->changeAssets(iChange);
}
int CyPlayer::getPower()
{
	return m_pPlayer ? m_pPlayer->getPower() : -1;
}
int CyPlayer::getPopScore()
{
	return m_pPlayer ? m_pPlayer->getPopScore() : -1;
}
int CyPlayer::getLandScore()
{
	return m_pPlayer ? m_pPlayer->getLandScore() : -1;
}
int CyPlayer::getFatherScore()
{
	return m_pPlayer ? m_pPlayer->getFatherScore() : -1;
}
int CyPlayer::getTotalTimePlayed()
{
	return m_pPlayer ? m_pPlayer->getTotalTimePlayed() : -1;
}
bool CyPlayer::isAlive()
{
	return m_pPlayer ? m_pPlayer->isAlive() : false;
}
bool CyPlayer::isEverAlive()
{
	return m_pPlayer ? m_pPlayer->isEverAlive() : false;
}
bool CyPlayer::isExtendedGame()
{
	return m_pPlayer ? m_pPlayer->isExtendedGame() : false;
}
bool CyPlayer::isFoundedFirstCity()
{
	return m_pPlayer ? m_pPlayer->isFoundedFirstCity() : false;
}
int CyPlayer::getID()
{
	return m_pPlayer ? m_pPlayer->getID() : -1;
}
int /* HandicapTypes */ CyPlayer::getHandicapType()
{
	return m_pPlayer ? (int) m_pPlayer->getHandicapType() : -1;
}
int /* CivilizationTypes */  CyPlayer::getCivilizationType()
{
	return m_pPlayer ? (int) m_pPlayer->getCivilizationType() : NO_CIVILIZATION;
}
int /*LeaderHeadTypes*/ CyPlayer::getLeaderType()
{
	return m_pPlayer ? (int) m_pPlayer->getLeaderType() : -1;
}
int /*LeaderHeadTypes*/ CyPlayer::getPersonalityType()
{
	return m_pPlayer ? (int) m_pPlayer->getPersonalityType() : -1;
}
void CyPlayer::setPersonalityType(int /*LeaderHeadTypes*/ eNewValue)
{
	if (m_pPlayer)
		m_pPlayer->setPersonalityType((LeaderHeadTypes) eNewValue);
}
int /*ErasTypes*/ CyPlayer::getCurrentEra()
{
	return m_pPlayer ? (int) m_pPlayer->getCurrentEra() : NO_ERA;
}
void CyPlayer::setCurrentEra(int /*EraTypes*/ iNewValue)
{
	if (m_pPlayer)
		m_pPlayer->setCurrentEra((EraTypes) iNewValue);
}
int /*PlayerTypes*/ CyPlayer::getParent()
{
	return m_pPlayer ? m_pPlayer->getParent() : -1;
}

int CyPlayer::getTeam()
{
	return m_pPlayer ? m_pPlayer->getTeam() : -1;
}
int /*PlayerColorTypes*/ CyPlayer::getPlayerColor()
{
	return m_pPlayer ? (int) m_pPlayer->getPlayerColor() : NO_COLOR;
}
int CyPlayer::getPlayerTextColorR()
{
	return m_pPlayer ? m_pPlayer->getPlayerTextColorR() : -1;
}
int CyPlayer::getPlayerTextColorG()
{
	return m_pPlayer ? m_pPlayer->getPlayerTextColorG() : -1;
}
int CyPlayer::getPlayerTextColorB()
{
	return m_pPlayer ? m_pPlayer->getPlayerTextColorB() : -1;
}
int CyPlayer::getPlayerTextColorA()
{
	return m_pPlayer ? m_pPlayer->getPlayerTextColorA() : -1;
}
int CyPlayer::getSeaPlotYield(YieldTypes eIndex)
{
	return m_pPlayer ? (int) m_pPlayer->getSeaPlotYield(eIndex) : 0;
}
int CyPlayer::getYieldRate(YieldTypes eIndex)
{
	return m_pPlayer ? m_pPlayer->getYieldRate(eIndex) : -1;
}
int CyPlayer::getYieldRateModifier(YieldTypes eIndex)
{
	return m_pPlayer ? m_pPlayer->getYieldRateModifier(eIndex) : 0;
}
int CyPlayer::getCapitalYieldRateModifier(YieldTypes eIndex)
{
	return m_pPlayer ? m_pPlayer->getCapitalYieldRateModifier(eIndex) : 0;
}
int CyPlayer::getExtraYieldThreshold(YieldTypes eIndex)
{
	return m_pPlayer ? m_pPlayer->getExtraYieldThreshold(eIndex) : 0;
}
bool CyPlayer::isYieldEuropeTradable(int /*YieldTypes*/ eIndex)
{
	return m_pPlayer ? m_pPlayer->isYieldEuropeTradable((YieldTypes)eIndex) : false;
}
void CyPlayer::setYieldEuropeTradable(int /*YieldTypes*/ eIndex, bool bTradeable)
{
	if (m_pPlayer)
		m_pPlayer->setYieldEuropeTradable((YieldTypes)eIndex, bTradeable);
}
bool CyPlayer::isFeatAccomplished(int /*FeatTypes*/ eIndex)
{
	return m_pPlayer ? m_pPlayer->isFeatAccomplished((FeatTypes)eIndex) : false;
}
void CyPlayer::setFeatAccomplished(int /*FeatTypes*/ eIndex, bool bNewValue)
{
	if (m_pPlayer)
		m_pPlayer->setFeatAccomplished((FeatTypes)eIndex, bNewValue);
}
bool CyPlayer::shouldDisplayFeatPopup(int /*FeatTypes*/ eIndex)
{
	return m_pPlayer ? m_pPlayer->shouldDisplayFeatPopup((FeatTypes)eIndex) : false;
}
bool CyPlayer::isOption(int /*PlayerOptionTypes*/ eIndex)
{
	return m_pPlayer ? m_pPlayer->isOption((PlayerOptionTypes)eIndex) : false;
}
void CyPlayer::setOption(int /*PlayerOptionTypes*/ eIndex, bool bNewValue)
{
	if (m_pPlayer)
		m_pPlayer->setOption((PlayerOptionTypes)eIndex, bNewValue);
}
bool CyPlayer::isPlayable()
{
	return m_pPlayer ? m_pPlayer->isPlayable() : false;
}
void CyPlayer::setPlayable(bool bNewValue)
{
	if (m_pPlayer)
		m_pPlayer->setPlayable(bNewValue);
}
int CyPlayer::getImprovementCount(int /*ImprovementTypes*/ iIndex)
{
	return m_pPlayer ? m_pPlayer->getImprovementCount((ImprovementTypes)iIndex) : -1;
}
bool CyPlayer::isBuildingFree(int /*BuildingTypes*/ iIndex)
{
	return m_pPlayer ? m_pPlayer->isBuildingFree((BuildingTypes)iIndex) : false;
}
int CyPlayer::getUnitClassCount(int /*UnitClassTypes*/ eIndex)
{
	return m_pPlayer ? m_pPlayer->getUnitClassCount((UnitClassTypes) eIndex) : NO_UNITCLASS;
}
int CyPlayer::getUnitClassMaking(int /*UnitClassTypes*/ eIndex)
{
	return m_pPlayer ? m_pPlayer->getUnitClassMaking((UnitClassTypes) eIndex) : -1;
}
int CyPlayer::getUnitClassCountPlusMaking(int /*UnitClassTypes*/ eIndex)
{
	return m_pPlayer ? m_pPlayer->getUnitClassCountPlusMaking((UnitClassTypes) eIndex) : -1;
}
int CyPlayer::getBuildingClassCount(int /*BuildingClassTypes*/ iIndex)
{
	return m_pPlayer ? m_pPlayer->getBuildingClassCount((BuildingClassTypes)iIndex) : -1;
}
int CyPlayer::getBuildingClassMaking(int /*BuildingClassTypes*/ iIndex)
{
	return m_pPlayer ? m_pPlayer->getBuildingClassMaking((BuildingClassTypes)iIndex) : -1;
}
int CyPlayer::getBuildingClassCountPlusMaking(int /*BuildingClassTypes*/ iIndex)
{
	return m_pPlayer ? m_pPlayer->getBuildingClassCountPlusMaking((BuildingClassTypes)iIndex) : -1;
}
int CyPlayer::getHurryCount(int /*HurryTypes*/ eIndex)
{
	return m_pPlayer ? m_pPlayer->getHurryCount((HurryTypes)eIndex) : (int) NO_HURRY;
}
bool CyPlayer::canHurry(int /*HurryTypes*/ eIndex)
{
	return m_pPlayer ? m_pPlayer->canHurry((HurryTypes)eIndex) : (int) NO_HURRY;
}
int CyPlayer::getSpecialBuildingNotRequiredCount(int /*SpecialBuildingTypes*/ eIndex)
{
	return m_pPlayer ? m_pPlayer->getSpecialBuildingNotRequiredCount((SpecialBuildingTypes)eIndex) : -1;
}
bool CyPlayer::isSpecialBuildingNotRequired(int /*SpecialBuildingTypes*/ eIndex)
{
	return m_pPlayer ? m_pPlayer->isSpecialBuildingNotRequired((SpecialBuildingTypes)eIndex) : -1;
}
int CyPlayer::getBuildingYieldChange(int /*BuildingClassTypes*/ eBuildingClass, int /*YieldTypes*/ eYield)
{
	return m_pPlayer ? m_pPlayer->getBuildingYieldChange((BuildingClassTypes) eBuildingClass, (YieldTypes) eYield) : -1;
}
int /* CivicTypes */ CyPlayer::getCivic(int /*CivicOptionTypes*/ iIndex)
{
	return m_pPlayer ? m_pPlayer->getCivic((CivicOptionTypes)iIndex) : -1;
}
void CyPlayer::setCivic(int /*CivicOptionTypes*/ eIndex, int /*CivicTypes*/ eNewValue)
{
	if (m_pPlayer)
		m_pPlayer->setCivic((CivicOptionTypes) eIndex, (CivicTypes) eNewValue);
}
int CyPlayer::getCombatExperience() const
{
	if (m_pPlayer)
	{
		return m_pPlayer->getCombatExperience();
	}
	return -1;
}
void CyPlayer::changeCombatExperience(int iChange)
{
	if (m_pPlayer)
	{
		m_pPlayer->changeCombatExperience(iChange);
	}
}
void CyPlayer::setCombatExperience(int iExperience)
{
	if (m_pPlayer)
	{
		m_pPlayer->setCombatExperience(iExperience);
	}
}
void CyPlayer::addCityName(std::wstring szName)
{
	if (m_pPlayer)
		m_pPlayer->addCityName(szName);
}
int CyPlayer::getNumCityNames()
{
	return m_pPlayer ? m_pPlayer->getNumCityNames() : -1;
}
std::wstring CyPlayer::getCityName(int iIndex)
{
	return m_pPlayer ? m_pPlayer->getCityName(iIndex) : std::wstring();
}
// returns tuple of (CyCity, iterOut)
python::tuple CyPlayer::firstCity(bool bRev)
{
	int iterIn = 0;
	CvCity* pvObj = m_pPlayer ? m_pPlayer->firstCity(&iterIn, bRev) : NULL;
	CyCity* pyObj = pvObj ? new CyCity(pvObj) : NULL;
	python::tuple tup=python::make_tuple(pyObj, iterIn);
	delete pyObj;
	return tup;
}
// returns tuple of (CyCity, iterOut)
python::tuple CyPlayer::nextCity(int iterIn, bool bRev)
{
	CvCity* pvObj = m_pPlayer ? m_pPlayer->nextCity(&iterIn, bRev) : NULL;
	CyCity* pyObj = pvObj ? new CyCity(pvObj) : NULL;
	python::tuple tup=python::make_tuple(pyObj, iterIn);
	delete pyObj;
	return tup;
}
int CyPlayer::getNumCities()
{
	return m_pPlayer ? m_pPlayer->getNumCities() : -1;
}
CyCity* CyPlayer::getCity(int iID)
{
	return m_pPlayer ? new CyCity(m_pPlayer->getCity(iID)) : NULL;
}
// returns tuple of (CyUnit, iterOut)
python::tuple CyPlayer::firstUnit()
{
	int iterIn = 0;
	CvUnit* pvUnit = m_pPlayer ? m_pPlayer->firstUnit(&iterIn) : NULL;
	CyUnit* pyUnit = pvUnit ? new CyUnit(pvUnit) : NULL;
	python::tuple tup=python::make_tuple(pyUnit, iterIn);
	delete pyUnit;
	return tup;
}
// returns tuple of (CyUnit, iterOut)
python::tuple CyPlayer::nextUnit(int iterIn)
{
	CvUnit* pvObj = m_pPlayer ? m_pPlayer->nextUnit(&iterIn) : NULL;
	CyUnit* pyObj = pvObj ? new CyUnit(pvObj) : NULL;
	python::tuple tup=python::make_tuple(pyObj, iterIn);
	delete pyObj;
	return tup;
}
int CyPlayer::getNumUnits()
{
	return m_pPlayer ? m_pPlayer->getNumUnits() : -1;
}
CyUnit* CyPlayer::getUnit(int iID)
{
	return m_pPlayer ? new CyUnit(m_pPlayer->getUnit(iID)) : NULL;
}
int CyPlayer::getNumEuropeUnits()
{
	return m_pPlayer ? m_pPlayer->getNumEuropeUnits() : -1;
}
CyUnit* CyPlayer::getEuropeUnit(int iIndex)
{
	return m_pPlayer ? new CyUnit(m_pPlayer->getEuropeUnit(iIndex)) : NULL;
}
CyUnit* CyPlayer::getEuropeUnitById(int iId)
{
	return m_pPlayer ? new CyUnit(m_pPlayer->getEuropeUnitById(iId)) : NULL;
}
void CyPlayer::loadUnitFromEurope(CyUnit* pUnit, CyUnit* pTransport)
{
	if (m_pPlayer)
	{
		m_pPlayer->loadUnitFromEurope(pUnit->getUnit(), pTransport->getUnit());
	}
}
void CyPlayer::unloadUnitToEurope(CyUnit* pUnit)
{
	if (m_pPlayer)
	{
		m_pPlayer->unloadUnitToEurope(pUnit->getUnit());
	}
}

// returns tuple of (CySelectionGroup, iterOut)
python::tuple CyPlayer::firstSelectionGroup(bool bRev)
{
	int iterIn = 0;
	CvSelectionGroup* pvObj = m_pPlayer ? m_pPlayer->firstSelectionGroup(&iterIn, bRev) : NULL;
	CySelectionGroup* pyObj = pvObj ? new CySelectionGroup(pvObj) : NULL;
	python::tuple tup=python::make_tuple(pyObj, iterIn);
	delete pyObj;
	return tup;
}
// returns tuple of (CySelectionGroup, iterOut)
python::tuple CyPlayer::nextSelectionGroup(int iterIn, bool bRev)
{
	CvSelectionGroup* pvObj = m_pPlayer ? m_pPlayer->nextSelectionGroup(&iterIn, bRev) : NULL;
	CySelectionGroup* pyObj = pvObj ? new CySelectionGroup(pvObj) : NULL;
	python::tuple tup=python::make_tuple(pyObj, iterIn);
	delete pyObj;
	return tup;
}
int CyPlayer::getNumSelectionGroups()
{
	return m_pPlayer ? m_pPlayer->getNumSelectionGroups() : -1;
}
CySelectionGroup* CyPlayer::getSelectionGroup(int iID)
{
	return m_pPlayer ? new CySelectionGroup(m_pPlayer->getSelectionGroup(iID)) : NULL;
}
int CyPlayer::countNumTravelUnits(int /*UnitTravelStates*/ eState, int /*DomainTypes*/ eDomain)
{
	return m_pPlayer ? m_pPlayer->countNumTravelUnits((UnitTravelStates) eState, (DomainTypes) eDomain) : -1;
}

void CyPlayer::trigger(/*EventTriggerTypes*/int eEventTrigger)
{
	if (m_pPlayer)
	{
		m_pPlayer->trigger((EventTriggerTypes)eEventTrigger);
	}
}
const EventTriggeredData* CyPlayer::getEventOccured(int /*EventTypes*/ eEvent) const
{
	return m_pPlayer ? m_pPlayer->getEventOccured((EventTypes)eEvent) : NULL;
}
void CyPlayer::resetEventOccured(int /*EventTypes*/ eEvent)
{
	if (m_pPlayer)
	{
		m_pPlayer->resetEventOccured((EventTypes)eEvent);
	}
}
EventTriggeredData* CyPlayer::getEventTriggered(int iID) const
{
	return m_pPlayer ? m_pPlayer->getEventTriggered(iID) : NULL;
}
EventTriggeredData* CyPlayer::initTriggeredData(int /*EventTriggerTypes*/ eEventTrigger, bool bFire, int iCityId, int iPlotX, int iPlotY, int /*PlayerTypes*/ eOtherPlayer, int iOtherPlayerCityId, int iUnitId, int /*BuildingTypes*/ eBuilding)
{
	return m_pPlayer ? m_pPlayer->initTriggeredData((EventTriggerTypes)eEventTrigger, bFire, iCityId, iPlotX, iPlotY, (PlayerTypes)eOtherPlayer, iOtherPlayerCityId, iUnitId, (BuildingTypes)eBuilding) : NULL;
}
int CyPlayer::getEventTriggerWeight(int /*EventTriggerTypes*/ eTrigger)
{
	return m_pPlayer ? m_pPlayer->getEventTriggerWeight((EventTriggerTypes)eTrigger) : NULL;
}
void CyPlayer::AI_updateFoundValues(bool bStartingLoc)
{
	if (m_pPlayer)
		m_pPlayer->AI_updateFoundValues(bStartingLoc);
}
int CyPlayer::AI_foundValue(int iX, int iY, int iMinUnitRange/* = -1*/, bool bStartingLoc/* = false*/)
{
	return m_pPlayer ? m_pPlayer->AI_foundValue(iX, iY, iMinUnitRange, bStartingLoc) : -1;
}
bool CyPlayer::AI_demandRebukedWar(int /*PlayerTypes*/ ePlayer)
{
	return m_pPlayer ? m_pPlayer->AI_demandRebukedWar((PlayerTypes)ePlayer) : false;
}
AttitudeTypes CyPlayer::AI_getAttitude(int /*PlayerTypes*/ ePlayer)
{
	return m_pPlayer ? m_pPlayer->AI_getAttitude((PlayerTypes)ePlayer) : NO_ATTITUDE;
}
int CyPlayer::AI_unitValue(int /*UnitTypes*/ eUnit, int /*UnitAITypes*/ eUnitAI, CyArea* pArea)
{
	return m_pPlayer ? m_pPlayer->AI_unitValue((UnitTypes)eUnit, (UnitAITypes)eUnitAI, pArea->getArea()) : -1;
}
int CyPlayer::AI_civicValue(int /*CivicTypes*/ eCivic)
{
	return m_pPlayer ? m_pPlayer->AI_civicValue((CivicTypes)eCivic) : -1;
}
int CyPlayer::AI_totalUnitAIs(int /*UnitAITypes*/ eUnitAI)
{
	return m_pPlayer ? m_pPlayer->AI_totalUnitAIs((UnitAITypes)eUnitAI) : -1;
}
int CyPlayer::AI_totalAreaUnitAIs(CyArea* pArea, int /*UnitAITypes*/ eUnitAI)
{
	return m_pPlayer ? m_pPlayer->AI_totalAreaUnitAIs(pArea->getArea(), (UnitAITypes)eUnitAI) : -1;
}
int CyPlayer::AI_totalWaterAreaUnitAIs(CyArea* pArea, int /*UnitAITypes*/ eUnitAI)
{
	return m_pPlayer ? m_pPlayer->AI_totalWaterAreaUnitAIs(pArea->getArea(), (UnitAITypes)eUnitAI) : -1;
}
int CyPlayer::AI_getNumAIUnits(int /*UnitAITypes*/ eIndex)
{
	return m_pPlayer ? m_pPlayer->AI_getNumAIUnits((UnitAITypes)eIndex) : NO_UNITAI;
}
int CyPlayer::AI_getAttitudeExtra(int /*PlayerTypes*/ eIndex)
{
	return m_pPlayer ? m_pPlayer->AI_getAttitudeExtra((PlayerTypes)eIndex) : -1;
}
void CyPlayer::AI_setAttitudeExtra(int /*PlayerTypes*/ eIndex, int iNewValue)
{
	if (m_pPlayer)
		m_pPlayer->AI_setAttitudeExtra((PlayerTypes)eIndex, iNewValue);
}
void CyPlayer::AI_changeAttitudeExtra(int /*PlayerTypes*/ eIndex, int iChange)
{
	if (m_pPlayer)
		m_pPlayer->AI_changeAttitudeExtra((PlayerTypes)eIndex, iChange);
}
int CyPlayer::AI_getMemoryCount(int /*PlayerTypes*/ eIndex1, int /*MemoryTypes*/ eIndex2)
{
	return m_pPlayer ? m_pPlayer->AI_getMemoryCount((PlayerTypes)eIndex1, (MemoryTypes)eIndex2) : -1;
}
void CyPlayer::AI_changeMemoryCount(int /*PlayerTypes*/ eIndex1, int /*MemoryTypes*/ eIndex2, int iChange)
{
	if (m_pPlayer)
		m_pPlayer->AI_changeMemoryCount((PlayerTypes)eIndex1, (MemoryTypes)eIndex2, iChange);
}
int CyPlayer::AI_getExtraGoldTarget() const
{
	return m_pPlayer ? m_pPlayer->AI_getExtraGoldTarget() : -1;
}
void CyPlayer::AI_setExtraGoldTarget(int iNewValue)
{
	if (m_pPlayer)
	{
		m_pPlayer->AI_setExtraGoldTarget(iNewValue);
	}
}

int CyPlayer::getScoreHistory(int iTurn) const
{
	return (NULL != m_pPlayer ? m_pPlayer->getScoreHistory(iTurn) : 0);
}
int CyPlayer::getEconomyHistory(int iTurn) const
{
	return (NULL != m_pPlayer ? m_pPlayer->getEconomyHistory(iTurn) : 0);
}
int CyPlayer::getIndustryHistory(int iTurn) const
{
	return (NULL != m_pPlayer ? m_pPlayer->getIndustryHistory(iTurn) : 0);
}
int CyPlayer::getAgricultureHistory(int iTurn) const
{
	return (NULL != m_pPlayer ? m_pPlayer->getAgricultureHistory(iTurn) : 0);
}
int CyPlayer::getPowerHistory(int iTurn) const
{
	return (NULL != m_pPlayer ? m_pPlayer->getPowerHistory(iTurn) : 0);
}
int CyPlayer::getCultureHistory(int iTurn) const
{
	return (NULL != m_pPlayer ? m_pPlayer->getCultureHistory(iTurn) : 0);
}

int CyPlayer::addTradeRoute(int iSourceCityOwner, int iSourceCityId, int iDestinationCityOwner, int iDestinationCityId, int /*YieldTypes*/ eYield)
{
	return m_pPlayer ? m_pPlayer->addTradeRoute(IDInfo((PlayerTypes)iSourceCityOwner, iSourceCityId), IDInfo((PlayerTypes)iDestinationCityOwner, iDestinationCityId), (YieldTypes) eYield) : -1;
}

bool CyPlayer::removeTradeRoute(int iId)
{
	return m_pPlayer ? m_pPlayer->removeTradeRoute(iId) : false;
}

CyTradeRoute* CyPlayer::getTradeRoute(int iId) const
{
	return new CyTradeRoute(m_pPlayer ? m_pPlayer->getTradeRoute(iId) : NULL);
}

int CyPlayer::getNumTradeRoutes() const
{
	return m_pPlayer ? m_pPlayer->getNumTradeRoutes() : -1;
}

CyTradeRoute* CyPlayer::getTradeRouteByIndex(int iIndex) const
{
	return new CyTradeRoute(m_pPlayer ? m_pPlayer->getTradeRouteByIndex(iIndex) : NULL);
}

bool CyPlayer::editTradeRoute(int iId, int iSourceCityOwner, int iSourceCityId, int iDestinationCityOwner, int iDestinationCityId, int /*YieldTypes*/ eYield)
{
	return m_pPlayer ? m_pPlayer->editTradeRoute(iId, IDInfo((PlayerTypes)iSourceCityOwner, iSourceCityId), IDInfo((PlayerTypes)iDestinationCityOwner, iDestinationCityId), (YieldTypes) eYield) : false;
}

bool CyPlayer::canLoadYield(int /*PlayerTypes*/ eCityPlayer) const
{
	return m_pPlayer ? m_pPlayer->canLoadYield((PlayerTypes) eCityPlayer) : false;
}

bool CyPlayer::canUnloadYield(int /*PlayerTypes*/ eCityPlayer) const
{
	return m_pPlayer ? m_pPlayer->canUnloadYield((PlayerTypes) eCityPlayer) : false;
}

int CyPlayer::getYieldEquipmentAmount(int /*ProfessionTypes*/ eProfession, int /*YieldTypes*/ eYield)
{
	return m_pPlayer ? m_pPlayer->getYieldEquipmentAmount((ProfessionTypes) eProfession, (YieldTypes) eYield) : -1;
}

std::string CyPlayer::getScriptData() const
{
	return m_pPlayer ? m_pPlayer->getScriptData() : "";
}
void CyPlayer::setScriptData(std::string szNewValue)
{
	if (m_pPlayer)
		m_pPlayer->setScriptData(szNewValue);
}
int CyPlayer::AI_maxGoldTrade(int iPlayer)
{
	CvPlayerAI* pPlayer = dynamic_cast<CvPlayerAI*>(m_pPlayer);
	if (pPlayer)
	{
		return (pPlayer->AI_maxGoldTrade((PlayerTypes)iPlayer));
	}
	return 0;
}
void CyPlayer::forcePeace(int iPlayer)
{
	if (m_pPlayer)
		m_pPlayer->forcePeace((PlayerTypes)iPlayer);
}
int CyPlayer::getHighestTradedYield()
{
	return m_pPlayer ? m_pPlayer->getHighestTradedYield() : -1;
}
int CyPlayer::getHighestStoredYieldCityId(int /*YieldTypes*/ eYield)
{
	return m_pPlayer ? m_pPlayer->getHighestStoredYieldCityId((YieldTypes) eYield) : -1;
}
int CyPlayer::getCrossesStored()
{
	return m_pPlayer ? m_pPlayer->getCrossesStored() : -1;
}
int CyPlayer::getBellsStored()
{
	return m_pPlayer ? m_pPlayer->getBellsStored() : -1;
}
int CyPlayer::getTaxRate()
{
	return m_pPlayer ? m_pPlayer->getTaxRate() : -1;
}
void CyPlayer::changeTaxRate(int iChange)
{
	if (m_pPlayer)
		m_pPlayer->changeTaxRate(iChange);
}
bool CyPlayer::canTradeWithEurope()
{
	return m_pPlayer ? m_pPlayer->canTradeWithEurope() : false;
}
int CyPlayer::getSellToEuropeProfit(int /*YieldTypes*/ eYield, int iAmount)
{
	return m_pPlayer ? m_pPlayer->getSellToEuropeProfit((YieldTypes) eYield, iAmount) : -1;
}
int CyPlayer::getYieldSellPrice(int /*YieldTypes*/ eYield)
{
	return m_pPlayer ? m_pPlayer->getYieldSellPrice((YieldTypes) eYield) : -1;
}
int CyPlayer::getYieldBuyPrice(int /*YieldTypes*/ eYield)
{
	return m_pPlayer ? m_pPlayer->getYieldBuyPrice((YieldTypes) eYield) : -1;
}
void CyPlayer::setYieldBuyPrice(int /*YieldTypes*/ eYield, int iPrice, bool bMessage)
{
	if (m_pPlayer)
		m_pPlayer->setYieldBuyPrice((YieldTypes)eYield, iPrice, bMessage);
}
void CyPlayer::sellYieldUnitToEurope(CyUnit* pUnit, int iAmount, int iCommission)
{
	if (m_pPlayer)
		m_pPlayer->sellYieldUnitToEurope(pUnit->getUnit(), iAmount, iCommission);
}
CyUnit* CyPlayer::buyYieldUnitFromEurope(int /*YieldTypes*/ eYield, int iAmount, CyUnit* pTransport)
{
	return m_pPlayer ? (new CyUnit(m_pPlayer->buyYieldUnitFromEurope((YieldTypes) eYield, iAmount, pTransport->getUnit()))) : NULL;
}
int CyPlayer::getEuropeUnitBuyPrice(int /*UnitTypes*/ eUnit)
{
	return m_pPlayer ? m_pPlayer->getEuropeUnitBuyPrice((UnitTypes) eUnit) : -1;
}
CyUnit* CyPlayer::buyEuropeUnit(int /*UnitTypes*/ eUnit)
{
	return m_pPlayer ? (new CyUnit(m_pPlayer->buyEuropeUnit((UnitTypes) eUnit, 100))) : NULL;
}
int CyPlayer::getYieldBoughtTotal(int /*YieldTypes*/ eYield) const
{
	return m_pPlayer ? m_pPlayer->getYieldBoughtTotal((YieldTypes) eYield) : 0;
}
int CyPlayer::getNumRevolutionEuropeUnits() const
{
	return m_pPlayer ? m_pPlayer->getNumRevolutionEuropeUnits() : -1;
}
int CyPlayer::getRevolutionEuropeUnit(int iIndex) const
{
	return (int) m_pPlayer ? m_pPlayer->getRevolutionEuropeUnit(iIndex) : -1;
}
int CyPlayer::getRevolutionEuropeProfession(int iIndex) const
{
	return (int) m_pPlayer ? m_pPlayer->getRevolutionEuropeProfession(iIndex) : -1;
}
bool CyPlayer::isEurope() const
{
	return m_pPlayer ? m_pPlayer->isEurope() : false;
}
bool CyPlayer::isInRevolution() const
{
	return m_pPlayer ? m_pPlayer->isInRevolution() : false;
}
int CyPlayer::getDocksNextUnit(int iIndex) const
{
	return (int) m_pPlayer ? m_pPlayer->getDocksNextUnit(iIndex) : -1;
}
void CyPlayer::addRevolutionEuropeUnit(int /*UnitTypes*/ eUnit, int /*ProfessionTypes*/ eProfession)
{
	if (m_pPlayer)
	{
		m_pPlayer->addRevolutionEuropeUnit((UnitTypes) eUnit, (ProfessionTypes) eProfession);
	}
}
int CyPlayer::getNumTradeMessages() const
{
	return m_pPlayer ? m_pPlayer->getNumTradeMessages() : -1;
}
std::wstring CyPlayer::getTradeMessage(int i) const
{
	return m_pPlayer ? m_pPlayer->getTradeMessage(i) : L"";
}
