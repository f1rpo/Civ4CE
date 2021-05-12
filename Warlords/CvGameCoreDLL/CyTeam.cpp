//
// Python wrapper class for CvTeam 
// updated 6-5
//
#include "CvGameCoreDLL.h"
#include "CyTeam.h"
#include "CyArea.h"
#include "CvTeam.h"

CyTeam::CyTeam() : m_pTeam(NULL)
{
}

CyTeam::CyTeam(CvTeam* pTeam) : m_pTeam(pTeam)
{
}

void CyTeam::addTeam(int /*TeamTypes*/ eTeam)
{
	if (m_pTeam)
		m_pTeam->addTeam((TeamTypes)eTeam);
}

bool CyTeam::canChangeWarPeace(int /*TeamTypes*/ eTeam)
{
	return m_pTeam ? m_pTeam->canChangeWarPeace((TeamTypes)eTeam) : false;
}

bool CyTeam::canDeclareWar(int /*TeamTypes*/ eTeam)
{
	return m_pTeam ? m_pTeam->canDeclareWar((TeamTypes)eTeam) : false;
}

void CyTeam::declareWar(int /*TeamTypes*/ eTeam, bool bNewDiplo)
{
	if (m_pTeam)
		m_pTeam->declareWar((TeamTypes)eTeam, bNewDiplo);
}

void CyTeam::makePeace(int /*TeamTypes*/ eTeam)
{
	if (m_pTeam)
		m_pTeam->makePeace((TeamTypes)eTeam);
}

bool CyTeam::canContact(int /*TeamTypes*/ eTeam)
{
	return m_pTeam ? m_pTeam->canContact((TeamTypes)eTeam) : false;
}

void CyTeam::meet(int /*TeamTypes*/ eTeam, bool bNewDiplo)
{
	if (m_pTeam)
		m_pTeam->meet((TeamTypes)eTeam, bNewDiplo);
}

void CyTeam::signOpenBorders(int /*TeamTypes*/ eTeam)
{
	if (m_pTeam)
		m_pTeam->signOpenBorders((TeamTypes)eTeam);
}

void CyTeam::signDefensivePact(int /*TeamTypes*/ eTeam)
{
	if (m_pTeam)
		m_pTeam->signDefensivePact((TeamTypes)eTeam);
}

int CyTeam::getAssets()
{
	return m_pTeam ? m_pTeam->getAssets() : -1;
}

int CyTeam::getPower()
{
	return m_pTeam ? m_pTeam->getPower() : -1;
}

int CyTeam::getDefensivePower()
{
	return m_pTeam ? m_pTeam->getDefensivePower() : -1;
}

int CyTeam::getNumNukeUnits()
{
	return m_pTeam ? m_pTeam->getNumNukeUnits() : -1;
}

int CyTeam::getAtWarCount(bool bIgnoreMinors)
{
	return m_pTeam ? m_pTeam->getAtWarCount(bIgnoreMinors) : -1;
}

int CyTeam::getWarPlanCount(int /*WarPlanTypes*/ eWarPlan, bool bIgnoreMinors)
{
	return m_pTeam ? m_pTeam->getWarPlanCount((WarPlanTypes) eWarPlan, bIgnoreMinors) : -1;
}

int CyTeam::getAnyWarPlanCount(bool bIgnoreMinors)
{
	return m_pTeam ? m_pTeam->getAnyWarPlanCount(bIgnoreMinors) : -1;
}

int CyTeam::getChosenWarCount(bool bIgnoreMinors)
{
	return m_pTeam ? m_pTeam->getChosenWarCount(bIgnoreMinors) : -1;
}

int CyTeam::getHasMetCivCount(bool bIgnoreMinors)
{
	return m_pTeam ? m_pTeam->getHasMetCivCount(bIgnoreMinors) : -1;
}

bool CyTeam::hasMetHuman()
{
	return m_pTeam ? m_pTeam->hasMetHuman() : false;
}

int CyTeam::getDefensivePactCount()
{
	return m_pTeam ? m_pTeam->getDefensivePactCount() : -1;
}

int CyTeam::getUnitClassMaking(int /*UnitClassTypes*/ eUnitClass)
{
	return m_pTeam ? m_pTeam->getUnitClassMaking((UnitClassTypes)eUnitClass) : -1;
}

int CyTeam::getUnitClassCountPlusMaking(int /*UnitClassTypes*/ eUnitClass)
{
	return m_pTeam ? m_pTeam->getUnitClassCountPlusMaking((UnitClassTypes)eUnitClass) : -1;
}

int CyTeam::getBuildingClassMaking(int /*BuildingClassTypes*/ eBuildingClass)
{
	return m_pTeam ? m_pTeam->getBuildingClassMaking((BuildingClassTypes)eBuildingClass) : -1;
}

int CyTeam::getBuildingClassCountPlusMaking(int /*BuildingClassTypes*/ eBuildingClass)
{
	return m_pTeam ? m_pTeam->getBuildingClassCountPlusMaking((BuildingClassTypes)eBuildingClass) : -1;
}

int CyTeam::getHasReligionCount(int /*ReligionTypes*/ eReligion)
{
	return m_pTeam ? m_pTeam->getHasReligionCount((ReligionTypes)eReligion) : -1;
}
int CyTeam::countTotalCulture()
{
	return m_pTeam ? m_pTeam->countTotalCulture() : -1;
}

int CyTeam::countNumUnitsByArea(CyArea* pArea)
{
	return m_pTeam ? m_pTeam->countNumUnitsByArea(pArea->getArea()) : -1;
}

int CyTeam::countNumCitiesByArea(CyArea* pArea)
{
	return m_pTeam ? m_pTeam->countNumCitiesByArea(pArea->getArea()) : -1;
}

int CyTeam::countTotalPopulationByArea(CyArea* pArea)
{
	return m_pTeam ? m_pTeam->countTotalPopulationByArea(pArea->getArea()) : -1;
}

int CyTeam::countPowerByArea(CyArea* pArea)
{
	return m_pTeam ? m_pTeam->countPowerByArea(pArea->getArea()) : -1;
}

int CyTeam::countEnemyPowerByArea(CyArea* pArea)
{
	return m_pTeam ? m_pTeam->countEnemyPowerByArea(pArea->getArea()) : -1;
}

int CyTeam::countNumAIUnitsByArea(CyArea* pArea, int /*UnitAITypes*/ eUnitAI)
{
	return m_pTeam ? m_pTeam->countNumAIUnitsByArea(pArea->getArea(), (UnitAITypes) eUnitAI) : -1;
}

int CyTeam::countEnemyDangerByArea(CyArea* pArea)
{
	return m_pTeam ? m_pTeam->countEnemyDangerByArea(pArea->getArea()) : -1;
}

int CyTeam::getResearchCost(int /*TechTypes*/ eTech)
{
	return m_pTeam ? m_pTeam->getResearchCost((TechTypes)eTech) : -1;
}

int CyTeam::getResearchLeft(int /*TechTypes*/ eTech)
{
	return m_pTeam ? m_pTeam->getResearchLeft((TechTypes)eTech) : -1;
}
bool CyTeam::hasHolyCity(int /*ReligionTypes*/ eReligion)
{
	return m_pTeam ? m_pTeam->hasHolyCity((ReligionTypes)eReligion) : false;
}

bool CyTeam::isHuman()
{
	return m_pTeam ? m_pTeam->isHuman() : false;
}

bool CyTeam::isBarbarian()
{
	return m_pTeam ? m_pTeam->isBarbarian() : false;
}

bool CyTeam::isMinorCiv()
{
	return m_pTeam ? m_pTeam->isMinorCiv() : false;
}

int /*PlayerTypes*/ CyTeam::getLeaderID()
{
	return m_pTeam ? m_pTeam->getLeaderID() : -1;
}

int /*PlayerTypes*/ CyTeam::getSecretaryID()
{
	return m_pTeam ? m_pTeam->getSecretaryID() : -1;
}

int /*HandicapTypes*/ CyTeam::getHandicapType()
{
	return m_pTeam ? m_pTeam->getHandicapType() : -1;
}

std::wstring CyTeam::getName()
{
	return m_pTeam ? m_pTeam->getName() : L"";
}

int CyTeam::getNumMembers()
{
	return m_pTeam ? m_pTeam->getNumMembers() : -1;
}

bool CyTeam::isAlive()
{
	return m_pTeam ? m_pTeam->isAlive() : false;
}

bool CyTeam::isEverAlive()
{
	return m_pTeam ? m_pTeam->isEverAlive() : false;
}

int CyTeam::getNumCities()
{
	return m_pTeam ? m_pTeam->getNumCities() : -1;
}

int CyTeam::getTotalPopulation()
{
	return m_pTeam ? m_pTeam->getTotalPopulation() : -1;
}

int CyTeam::getTotalLand()
{
	return m_pTeam ? m_pTeam->getTotalLand() : -1;
}

int CyTeam::getNukeInterception()
{
	return m_pTeam ? m_pTeam->getNukeInterception() : -1;
}

void CyTeam::changeNukeInterception(int iChange)
{
	if (m_pTeam)
		m_pTeam->changeNukeInterception(iChange);
}

int CyTeam::getForceTeamVoteEligibilityCount()
{
	return m_pTeam ? m_pTeam->getForceTeamVoteEligibilityCount() : -1;
}

bool CyTeam::isForceTeamVoteEligible()
{
	return m_pTeam ? m_pTeam->isForceTeamVoteEligible() : false;
}

void CyTeam::changeForceTeamVoteEligibilityCount(int iChange)
{
	if (m_pTeam)
		m_pTeam->changeForceTeamVoteEligibilityCount(iChange);
}

int CyTeam::getExtraWaterSeeFromCount()
{
	return m_pTeam ? m_pTeam->getExtraWaterSeeFromCount() : -1;
}

bool CyTeam::isExtraWaterSeeFrom()	 
{
	return m_pTeam ? m_pTeam->isExtraWaterSeeFrom() : false;
}

void CyTeam::changeExtraWaterSeeFromCount(int iChange)
{
	if (m_pTeam)
		m_pTeam->changeExtraWaterSeeFromCount(iChange);
}

int CyTeam::getMapTradingCount()
{
	return m_pTeam ? m_pTeam->getMapTradingCount() : -1;
}

bool CyTeam::isMapTrading()
{
	return m_pTeam ? m_pTeam->isMapTrading() : false;
}

void CyTeam::changeMapTradingCount(int iChange)
{
	if (m_pTeam)
		m_pTeam->changeMapTradingCount(iChange);
}

int CyTeam::getTechTradingCount()
{
	return m_pTeam ? m_pTeam->getTechTradingCount() : -1;
}

bool CyTeam::isTechTrading()
{
	return m_pTeam ? m_pTeam->isTechTrading() : false;
}

void CyTeam::changeTechTradingCount(int iChange)
{
	if (m_pTeam)
		m_pTeam->changeTechTradingCount(iChange);
}

int CyTeam::getGoldTradingCount()
{
	return m_pTeam ? m_pTeam->getGoldTradingCount() : -1;
}

bool CyTeam::isGoldTrading()
{
	return m_pTeam ? m_pTeam->isGoldTrading() : false;
}

void CyTeam::changeGoldTradingCount(int iChange)
{
	if (m_pTeam)
		m_pTeam->changeGoldTradingCount(iChange);
}

int CyTeam::getOpenBordersTradingCount()
{
	return m_pTeam ? m_pTeam->getOpenBordersTradingCount() : -1;
}

bool CyTeam::isOpenBordersTrading()
{
	return m_pTeam ? m_pTeam->isOpenBordersTrading() : false;
}

void CyTeam::changeOpenBordersTradingCount(int iChange)
{
	if (m_pTeam)
		m_pTeam->changeOpenBordersTradingCount(iChange);
}

int CyTeam::getDefensivePactTradingCount()
{
	return m_pTeam ? m_pTeam->getDefensivePactTradingCount() : -1;
}

bool CyTeam::isDefensivePactTrading()
{
	return m_pTeam ? m_pTeam->isDefensivePactTrading() : false;
}

void CyTeam::changeDefensivePactTradingCount(int iChange)
{
	if (m_pTeam)
		m_pTeam->changeDefensivePactTradingCount(iChange);
}

int CyTeam::getPermanentAllianceTradingCount()
{
	return m_pTeam ? m_pTeam->getPermanentAllianceTradingCount() : -1;
}

bool CyTeam::isPermanentAllianceTrading()
{
	return m_pTeam ? m_pTeam->isPermanentAllianceTrading() : false;
}

void CyTeam::changePermanentAllianceTradingCount(int iChange)
{
	if (m_pTeam)
		m_pTeam->changePermanentAllianceTradingCount(iChange);
}

int CyTeam::getBridgeBuildingCount()
{
	return m_pTeam ? m_pTeam->getBridgeBuildingCount() : -1;
}

bool CyTeam::isBridgeBuilding()
{
	return m_pTeam ? m_pTeam->isBridgeBuilding() : false;
}

void CyTeam::changeBridgeBuildingCount(int iChange)
{
	if (m_pTeam)
		m_pTeam->changeBridgeBuildingCount(iChange);
}

int CyTeam::getIrrigationCount()
{
	return m_pTeam ? m_pTeam->getIrrigationCount() : -1;
}

bool CyTeam::isIrrigation()
{
	return m_pTeam ? m_pTeam->isIrrigation() : false;
}

void CyTeam::changeIrrigationCount(int iChange)
{
	if (m_pTeam)
		m_pTeam->changeIrrigationCount(iChange);
}

int CyTeam::getIgnoreIrrigationCount()
{
	return m_pTeam ? m_pTeam->getIgnoreIrrigationCount() : -1;
}

bool CyTeam::isIgnoreIrrigation()
{
	return m_pTeam ? m_pTeam->isIgnoreIrrigation() : false;
}

void CyTeam::changeIgnoreIrrigationCount(int iChange)
{
	if (m_pTeam)
		m_pTeam->changeIgnoreIrrigationCount(iChange);
}

int CyTeam::getWaterWorkCount()
{
	return m_pTeam ? m_pTeam->getWaterWorkCount() : -1;
}

bool CyTeam::isWaterWork()
{
	return m_pTeam ? m_pTeam->isWaterWork() : false;
}

void CyTeam::changeWaterWorkCount(int iChange)
{
	if (m_pTeam)
		m_pTeam->changeWaterWorkCount(iChange);
}

bool CyTeam::isMapCentering()
{
	return m_pTeam ? m_pTeam->isMapCentering() : false;
}

void CyTeam::setMapCentering(bool bNewValue)
{
	if (m_pTeam)
		m_pTeam->setMapCentering(bNewValue);
}

int CyTeam::getID()
{
	return m_pTeam ? m_pTeam->getID() : -1;
}

bool CyTeam::isStolenVisibility(int /*TeamTypes*/ eIndex)
{
	return m_pTeam ? m_pTeam->isStolenVisibility((TeamTypes)eIndex) : false;
}

int CyTeam::getWarWeariness(int /*TeamTypes*/ eIndex)
{
	return m_pTeam ? m_pTeam->getWarWeariness((TeamTypes)eIndex) : -1;
}

void CyTeam::setWarWeariness(int /*TeamTypes*/ eIndex, int iNewValue)
{
	if (m_pTeam)
		m_pTeam->setWarWeariness((TeamTypes)eIndex, iNewValue);
}

void CyTeam::changeWarWeariness(int /*TeamTypes*/ eIndex, int iChange)	 
{
	if (m_pTeam)
		m_pTeam->changeWarWeariness((TeamTypes)eIndex, iChange);
}

int CyTeam::getTechShareCount(int iIndex)
{
	return m_pTeam ? m_pTeam->getTechShareCount(iIndex) : -1;
}

bool CyTeam::isTechShare(int iIndex)
{
	return m_pTeam ? m_pTeam->isTechShare(iIndex) : false;
}

void CyTeam::changeTechShareCount(int iIndex, int iChange)
{
	if (m_pTeam)
		m_pTeam->changeTechShareCount(iIndex, iChange);
}

int CyTeam::getCommerceFlexibleCount(int /*CommerceTypes*/ eIndex)
{
	return m_pTeam ? m_pTeam->getCommerceFlexibleCount((CommerceTypes)eIndex) : -1;
}

bool CyTeam::isCommerceFlexible(int /*CommerceTypes*/ eIndex)
{
	return m_pTeam ? m_pTeam->isCommerceFlexible((CommerceTypes)eIndex) : false;
}

void CyTeam::changeCommerceFlexibleCount(int /*CommerceTypes*/ eIndex, int iChange)
{
	if (m_pTeam)
		m_pTeam->changeCommerceFlexibleCount((CommerceTypes)eIndex, eIndex);
}

int CyTeam::getExtraMoves(int /*DomainTypes*/ eIndex)
{
	return m_pTeam ? m_pTeam->getExtraMoves((DomainTypes)eIndex) : -1;
}

void CyTeam::changeExtraMoves(int /*DomainTypes*/ eIndex, int iChange)
{
	if (m_pTeam)
		m_pTeam->changeExtraMoves((DomainTypes)eIndex, iChange);
}

bool CyTeam::isHasMet(int /*TeamTypes*/ eIndex)
{
	return m_pTeam ? m_pTeam->isHasMet((TeamTypes)eIndex) : false;
}

bool CyTeam::isAtWar(int /*TeamTypes*/ iIndex)
{
	if (iIndex == NO_TEAM) return false;
	return m_pTeam ? m_pTeam->isAtWar((TeamTypes)iIndex) : false;
}

bool CyTeam::isPermanentWarPeace(int /*TeamTypes*/ eIndex)
{
	return m_pTeam ? m_pTeam->isPermanentWarPeace((TeamTypes)eIndex) : false;
}

void CyTeam::setPermanentWarPeace(int /*TeamTypes*/ eIndex, bool bNewValue)
{
	if (m_pTeam)
		m_pTeam->setPermanentWarPeace((TeamTypes)eIndex, bNewValue);
}

bool CyTeam::isFreeTrade(int /*TeamTypes*/ eIndex)
{
	return m_pTeam ? m_pTeam->isFreeTrade((TeamTypes)eIndex) : false;
}

bool CyTeam::isOpenBorders(int /*TeamTypes*/ eIndex)
{
	return m_pTeam ? m_pTeam->isOpenBorders((TeamTypes)eIndex) : false;
}

bool CyTeam::isForcePeace(int /*TeamTypes*/ eIndex)				 
{
	return m_pTeam ? m_pTeam->isForcePeace((TeamTypes)eIndex) : false;
}

bool CyTeam::isDefensivePact(int /*TeamTypes*/ eIndex)				 
{
	return m_pTeam ? m_pTeam->isDefensivePact((TeamTypes)eIndex) : false;
}

int CyTeam::getRouteChange(int /*RouteTypes*/ eIndex)
{
	return m_pTeam ? m_pTeam->getRouteChange((RouteTypes)eIndex) : -1;
}

void CyTeam::changeRouteChange(int /*RouteTypes*/ eIndex, int iChange)
{
	if (m_pTeam)
		m_pTeam->changeRouteChange((RouteTypes)eIndex, iChange);
}

int CyTeam::getProjectCount(int /*ProjectTypes*/ eIndex)
{
	return m_pTeam ? m_pTeam->getProjectCount((ProjectTypes)eIndex) : -1;
}

bool CyTeam::isProjectMaxedOut(int /*ProjectTypes*/ eIndex, int iExtra)
{
	return m_pTeam ? m_pTeam->isProjectMaxedOut((ProjectTypes)eIndex, iExtra) : false;
}

void CyTeam::changeProjectCount(int /*ProjectTypes*/ eIndex, int iChange)
{
	if (m_pTeam)
		m_pTeam->changeProjectCount((ProjectTypes)eIndex, iChange);
}

int CyTeam::getProjectMaking(int /*ProjectTypes*/ eIndex)
{
	return m_pTeam ? m_pTeam->getProjectMaking((ProjectTypes)eIndex) : -1;
}

int CyTeam::getUnitClassCount(int /*UnitClassTypes*/ eIndex)
{
	return m_pTeam ? m_pTeam->getUnitClassCount((UnitClassTypes)eIndex) : -1;
}

bool CyTeam::isUnitClassMaxedOut(int /*UnitClassTypes*/ eIndex, int iExtra)
{
	return m_pTeam ? m_pTeam->isUnitClassMaxedOut((UnitClassTypes)eIndex, iExtra) : false;
}

int CyTeam::getBuildingClassCount(int /*BuildingClassTypes*/ eIndex)
{
	return m_pTeam ? m_pTeam->getBuildingClassCount((BuildingClassTypes)eIndex) : -1;
}

bool CyTeam::isBuildingClassMaxedOut(int /*BuildingClassTypes*/ eIndex, int iExtra)
{
	return m_pTeam ? m_pTeam->isBuildingClassMaxedOut((BuildingClassTypes)eIndex, iExtra) : false;
}

int CyTeam::getObsoleteBuildingCount(int /*BuildingTypes*/ eIndex)
{
	return m_pTeam ? m_pTeam->getObsoleteBuildingCount((BuildingTypes)eIndex) : -1;
}

bool CyTeam::isObsoleteBuilding(int /*BuildingTypes*/ eIndex)
{
	return m_pTeam ? m_pTeam->isObsoleteBuilding((BuildingTypes)eIndex) : false;
}

int CyTeam::getResearchProgress(int /*TechTypes*/ eIndex)
{
	return m_pTeam ? m_pTeam->getResearchProgress((TechTypes)eIndex) : -1;
}

void CyTeam::setResearchProgress(int /*TechTypes*/ eIndex, int iNewValue, int /*PlayerTypes*/ ePlayer)
{
	if (m_pTeam)
		m_pTeam->setResearchProgress((TechTypes)eIndex, iNewValue, (PlayerTypes)ePlayer);
}

void CyTeam::changeResearchProgress(int /*TechTypes*/ eIndex, int iChange, int /*PlayerTypes*/ ePlayer)
{
	if (m_pTeam)
		m_pTeam->changeResearchProgress((TechTypes)eIndex, iChange, (PlayerTypes)ePlayer);
}

int CyTeam::getTechCount(int /*TechTypes*/ eIndex)
{
	return m_pTeam ? m_pTeam->getTechCount((TechTypes)eIndex) : -1;
}

bool CyTeam::isTerrainTrade(int /*TerrainTypes*/ eIndex)
{
	if (m_pTeam)
	{
		return m_pTeam->isTerrainTrade((TerrainTypes)eIndex);
	}
	return false;
}

bool CyTeam::isHasTech(int /* TechTypes */ iIndex)
{
	return m_pTeam ? m_pTeam->isHasTech((TechTypes)iIndex) : false;
}

void CyTeam::setHasTech(int /*TechTypes*/ eIndex, bool bNewValue, int /*PlayerTypes*/ ePlayer, bool bFirst, bool bAnnounce)
{
	if (m_pTeam)
		m_pTeam->setHasTech((TechTypes)eIndex, bNewValue, (PlayerTypes)ePlayer, bFirst, bAnnounce);
}

bool CyTeam::isNoTradeTech(int /* TechTypes */ iIndex)
{
	return m_pTeam ? m_pTeam->isNoTradeTech((TechTypes)iIndex) : false;
}

void CyTeam::setNoTradeTech(int /*TechTypes*/ eIndex, bool bNewValue)
{
	if (m_pTeam)
		m_pTeam->setNoTradeTech((TechTypes)eIndex, bNewValue);
}

int CyTeam::getImprovementYieldChange(int /*ImprovementTypes*/ eIndex1, int /*YieldTypes*/ eIndex2)
{
	return m_pTeam ? m_pTeam->getImprovementYieldChange((ImprovementTypes)eIndex1, (YieldTypes)eIndex2) : -1;
}

void CyTeam::changeImprovementYieldChange(int /*ImprovementTypes*/ eIndex1, int /*YieldTypes*/ eIndex2, int iChange)
{
	if (m_pTeam)
		m_pTeam->changeImprovementYieldChange((ImprovementTypes)eIndex1, (YieldTypes)eIndex2, iChange);
}

bool CyTeam::AI_shareWar(int /*TeamTypes*/ eTeam)
{
	return m_pTeam ? m_pTeam->AI_shareWar((TeamTypes)eTeam) : false;
}
