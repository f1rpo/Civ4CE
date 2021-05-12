#include "CvGameCoreDLL.h"
#include "CvInfos.h"

//
// Python interface for info classes (formerly structs)
// These are simple enough to be exposed directly - no wrappers
//

void CyInfoPythonInterface3()
{
	OutputDebugString("Python Extension Module - CyInfoPythonInterface3\n");
	
	python::class_<CvYieldInfo, python::bases<CvInfoBase> >("CvYieldInfo")
		.def("getChar", &CvYieldInfo::getChar, "int ()")
		.def("getHillsChange", &CvYieldInfo::getHillsChange, "int ()")
		.def("getPeakChange", &CvYieldInfo::getPeakChange, "int ()")
		.def("getLakeChange", &CvYieldInfo::getLakeChange, "int ()")
		.def("getCityChange", &CvYieldInfo::getCityChange, "int ()")
		.def("getPopulationChangeOffset", &CvYieldInfo::getPopulationChangeOffset, "int ()")
		.def("getPopulationChangeDivisor", &CvYieldInfo::getPopulationChangeDivisor, "int ()")
		.def("getMinCity", &CvYieldInfo::getMinCity, "int ()")
		.def("getTradeModifier", &CvYieldInfo::getTradeModifier, "int ()")
		.def("getGoldenAgeYield", &CvYieldInfo::getGoldenAgeYield, "int ()")
		.def("getGoldenAgeYieldThreshold", &CvYieldInfo::getGoldenAgeYieldThreshold, "int ()")
		.def("getAIWeightPercent", &CvYieldInfo::getAIWeightPercent, "int ()")
		.def("getColorType", &CvYieldInfo::getColorType, "int ()")
		;

	python::class_<CvTerrainInfo, python::bases<CvInfoBase> >("CvTerrainInfo")

		.def("getMovementCost", &CvTerrainInfo::getMovementCost, "int ()")
		.def("getSeeFromLevel", &CvTerrainInfo::getSeeFromLevel, "int ()")
		.def("getSeeThroughLevel", &CvTerrainInfo::getSeeThroughLevel, "int ()")
		.def("getBuildModifier", &CvTerrainInfo::getBuildModifier, "int ()")
		.def("getDefenseModifier", &CvTerrainInfo::getDefenseModifier, "int ()")

		.def("isWater", &CvTerrainInfo::isWater, "bool ()")
		.def("isImpassable", &CvTerrainInfo::isImpassable, "bool ()")
		.def("isFound", &CvTerrainInfo::isFound, "bool ()")
		.def("isFoundCoast", &CvTerrainInfo::isFoundCoast, "bool ()")
		.def("isFoundFreshWater", &CvTerrainInfo::isFoundFreshWater, "bool ()")

		// Arrays

		.def("getYield", &CvTerrainInfo::getYield, "int (int i)")
		.def("getRiverYieldChange", &CvTerrainInfo::getRiverYieldChange, "int (int i)")
		.def("getHillsYieldChange", &CvTerrainInfo::getHillsYieldChange, "int (int i)")
		;

	// CvInterfaceModeInfo

	python::class_<CvLeaderHeadInfo, python::bases<CvInfoBase> >("CvLeaderHeadInfo")
		.def("getWonderConstructRand", &CvLeaderHeadInfo::getWonderConstructRand, "int ()")
		.def("getBaseAttitude", &CvLeaderHeadInfo::getBaseAttitude, "int ()")
		.def("getBasePeaceWeight", &CvLeaderHeadInfo::getBasePeaceWeight, "int ()")
		.def("getPeaceWeightRand", &CvLeaderHeadInfo::getPeaceWeightRand, "int ()")
		.def("getWarmongerRespect", &CvLeaderHeadInfo::getWarmongerRespect, "int ()")
		.def("getRefuseToTalkWarThreshold", &CvLeaderHeadInfo::getRefuseToTalkWarThreshold, "int ()")
		.def("getNoTechTradeThreshold", &CvLeaderHeadInfo::getNoTechTradeThreshold, "int ()")
		.def("getTechTradeKnownPercent", &CvLeaderHeadInfo::getTechTradeKnownPercent, "int ()")
		.def("getMaxGoldTradePercent", &CvLeaderHeadInfo::getMaxGoldTradePercent, "int ()")
		.def("getMaxGoldPerTurnTradePercent", &CvLeaderHeadInfo::getMaxGoldPerTurnTradePercent, "int ()")
		.def("getMaxWarRand", &CvLeaderHeadInfo::getMaxWarRand, "int ()")
		.def("getMaxWarNearbyPowerRatio", &CvLeaderHeadInfo::getMaxWarNearbyPowerRatio, "int ()")
		.def("getMaxWarDistantPowerRatio", &CvLeaderHeadInfo::getMaxWarDistantPowerRatio, "int ()")
		.def("getMaxWarMinAdjacentLandPercent", &CvLeaderHeadInfo::getMaxWarMinAdjacentLandPercent, "int ()")
		.def("getLimitedWarRand", &CvLeaderHeadInfo::getLimitedWarRand, "int ()")
		.def("getLimitedWarPowerRatio", &CvLeaderHeadInfo::getLimitedWarPowerRatio, "int ()")
		.def("getDogpileWarRand", &CvLeaderHeadInfo::getDogpileWarRand, "int ()")
		.def("getMakePeaceRand", &CvLeaderHeadInfo::getMakePeaceRand, "int ()")
		.def("getDeclareWarTradeRand", &CvLeaderHeadInfo::getDeclareWarTradeRand, "int ()")
		.def("getDemandRebukedSneakProb", &CvLeaderHeadInfo::getDemandRebukedSneakProb, "int ()")
		.def("getDemandRebukedWarProb", &CvLeaderHeadInfo::getDemandRebukedWarProb, "int ()")
		.def("getRazeCityProb", &CvLeaderHeadInfo::getRazeCityProb, "int ()")
		.def("getBuildUnitProb", &CvLeaderHeadInfo::getBuildUnitProb, "int ()")
		.def("getBaseAttackOddsChange", &CvLeaderHeadInfo::getBaseAttackOddsChange, "int ()")
		.def("getAttackOddsChangeRand", &CvLeaderHeadInfo::getAttackOddsChangeRand, "int ()")
		.def("getWorseRankDifferenceAttitudeChange", &CvLeaderHeadInfo::getWorseRankDifferenceAttitudeChange, "int ()")
		.def("getBetterRankDifferenceAttitudeChange", &CvLeaderHeadInfo::getBetterRankDifferenceAttitudeChange, "int ()")
		.def("getCloseBordersAttitudeChange", &CvLeaderHeadInfo::getCloseBordersAttitudeChange, "int ()")
		.def("getLostWarAttitudeChange", &CvLeaderHeadInfo::getLostWarAttitudeChange, "int ()")
		.def("getAtWarAttitudeDivisor", &CvLeaderHeadInfo::getAtWarAttitudeDivisor, "int ()")
		.def("getAtWarAttitudeChangeLimit", &CvLeaderHeadInfo::getAtWarAttitudeChangeLimit, "int ()")
		.def("getAtPeaceAttitudeDivisor", &CvLeaderHeadInfo::getAtPeaceAttitudeDivisor, "int ()")
		.def("getAtPeaceAttitudeChangeLimit", &CvLeaderHeadInfo::getAtPeaceAttitudeChangeLimit, "int ()")
		.def("getSameReligionAttitudeChange", &CvLeaderHeadInfo::getSameReligionAttitudeChange, "int ()")
		.def("getSameReligionAttitudeDivisor", &CvLeaderHeadInfo::getSameReligionAttitudeDivisor, "int ()")
		.def("getSameReligionAttitudeChangeLimit", &CvLeaderHeadInfo::getSameReligionAttitudeChangeLimit, "int ()")
		.def("getDifferentReligionAttitudeChange", &CvLeaderHeadInfo::getDifferentReligionAttitudeChange, "int ()")
		.def("getDifferentReligionAttitudeDivisor", &CvLeaderHeadInfo::getDifferentReligionAttitudeDivisor, "int ()")
		.def("getDifferentReligionAttitudeChangeLimit", &CvLeaderHeadInfo::getDifferentReligionAttitudeChangeLimit, "int ()")
		.def("getBonusTradeAttitudeDivisor", &CvLeaderHeadInfo::getBonusTradeAttitudeDivisor, "int ()")
		.def("getBonusTradeAttitudeChangeLimit", &CvLeaderHeadInfo::getBonusTradeAttitudeChangeLimit, "int ()")
		.def("getOpenBordersAttitudeDivisor", &CvLeaderHeadInfo::getOpenBordersAttitudeDivisor, "int ()")
		.def("getOpenBordersAttitudeChangeLimit", &CvLeaderHeadInfo::getOpenBordersAttitudeChangeLimit, "int ()")
		.def("getDefensivePactAttitudeDivisor", &CvLeaderHeadInfo::getDefensivePactAttitudeDivisor, "int ()")
		.def("getDefensivePactAttitudeChangeLimit", &CvLeaderHeadInfo::getDefensivePactAttitudeChangeLimit, "int ()")
		.def("getShareWarAttitudeChange", &CvLeaderHeadInfo::getShareWarAttitudeChange, "int ()")
		.def("getShareWarAttitudeDivisor", &CvLeaderHeadInfo::getShareWarAttitudeDivisor, "int ()")
		.def("getShareWarAttitudeChangeLimit", &CvLeaderHeadInfo::getShareWarAttitudeChangeLimit, "int ()")
		.def("getFavoriteCivicAttitudeChange", &CvLeaderHeadInfo::getFavoriteCivicAttitudeChange, "int ()")
		.def("getFavoriteCivicAttitudeDivisor", &CvLeaderHeadInfo::getFavoriteCivicAttitudeDivisor, "int ()")
		.def("getFavoriteCivicAttitudeChangeLimit", &CvLeaderHeadInfo::getFavoriteCivicAttitudeChangeLimit, "int ()")
		.def("getDemandTributeAttitudeThreshold", &CvLeaderHeadInfo::getDemandTributeAttitudeThreshold, "int ()")
		.def("getNoGiveHelpAttitudeThreshold", &CvLeaderHeadInfo::getNoGiveHelpAttitudeThreshold, "int ()")
		.def("getTechRefuseAttitudeThreshold", &CvLeaderHeadInfo::getTechRefuseAttitudeThreshold, "int ()")
		.def("getStrategicBonusRefuseAttitudeThreshold", &CvLeaderHeadInfo::getStrategicBonusRefuseAttitudeThreshold, "int ()")
		.def("getHappinessBonusRefuseAttitudeThreshold", &CvLeaderHeadInfo::getHappinessBonusRefuseAttitudeThreshold, "int ()")
		.def("getHealthBonusRefuseAttitudeThreshold", &CvLeaderHeadInfo::getHealthBonusRefuseAttitudeThreshold, "int ()")
		.def("getMapRefuseAttitudeThreshold", &CvLeaderHeadInfo::getMapRefuseAttitudeThreshold, "int ()")
		.def("getDeclareWarRefuseAttitudeThreshold", &CvLeaderHeadInfo::getDeclareWarRefuseAttitudeThreshold, "int ()")
		.def("getDeclareWarThemRefuseAttitudeThreshold", &CvLeaderHeadInfo::getDeclareWarThemRefuseAttitudeThreshold, "int ()")
		.def("getStopTradingRefuseAttitudeThreshold", &CvLeaderHeadInfo::getStopTradingRefuseAttitudeThreshold, "int ()")
		.def("getStopTradingThemRefuseAttitudeThreshold", &CvLeaderHeadInfo::getStopTradingThemRefuseAttitudeThreshold, "int ()")
		.def("getAdoptCivicRefuseAttitudeThreshold", &CvLeaderHeadInfo::getAdoptCivicRefuseAttitudeThreshold, "int ()")
		.def("getConvertReligionRefuseAttitudeThreshold", &CvLeaderHeadInfo::getConvertReligionRefuseAttitudeThreshold, "int ()")
		.def("getOpenBordersRefuseAttitudeThreshold", &CvLeaderHeadInfo::getOpenBordersRefuseAttitudeThreshold, "int ()")
		.def("getDefensivePactRefuseAttitudeThreshold", &CvLeaderHeadInfo::getDefensivePactRefuseAttitudeThreshold, "int ()")
		.def("getPermanentAllianceRefuseAttitudeThreshold", &CvLeaderHeadInfo::getPermanentAllianceRefuseAttitudeThreshold, "int ()")
		.def("getFavoriteCivic", &CvLeaderHeadInfo::getFavoriteCivic, "int ()")

		.def("getArtDefineTag", &CvLeaderHeadInfo::getArtDefineTag, "string ()")

		// Arrays

		.def("hasTrait", &CvLeaderHeadInfo::hasTrait, "bool (int i)")

		.def("getFlavorValue", &CvLeaderHeadInfo::getFlavorValue, "int (int i)")
		.def("getContactRand", &CvLeaderHeadInfo::getContactRand, "int (int i)")
		.def("getContactDelay", &CvLeaderHeadInfo::getContactDelay, "int (int i)")
		.def("getMemoryDecayRand", &CvLeaderHeadInfo::getMemoryDecayRand, "int (int i)")
		.def("getMemoryAttitudePercent", &CvLeaderHeadInfo::getMemoryAttitudePercent, "int (int i)")
		.def("getNoWarAttitudeProb", &CvLeaderHeadInfo::getNoWarAttitudeProb, "int (int i)")
		.def("getUnitAIWeightModifier", &CvLeaderHeadInfo::getUnitAIWeightModifier, "int (int i)")
		.def("getImprovementWeightModifier", &CvLeaderHeadInfo::getImprovementWeightModifier, "int (int i)")
		.def("getDiploPeaceIntroMusicScriptIds", &CvLeaderHeadInfo::getDiploPeaceIntroMusicScriptIds, "int (int i)")
		.def("getDiploPeaceMusicScriptIds", &CvLeaderHeadInfo::getDiploPeaceMusicScriptIds, "int (int i)")
		.def("getDiploWarIntroMusicScriptIds", &CvLeaderHeadInfo::getDiploWarIntroMusicScriptIds, "int (int i)")
		.def("getDiploWarMusicScriptIds", &CvLeaderHeadInfo::getDiploWarMusicScriptIds, "int (int i)")

		// Other

		.def("getLeaderHead", &CvLeaderHeadInfo::getLeaderHead, "string ()")
		.def("getButton", &CvLeaderHeadInfo::getButton, "string ()")
		;

	// CvProcessInfos
	python::class_<CvProcessInfo, python::bases<CvInfoBase> >("CvProcessInfo")
		.def("getTechPrereq", &CvProcessInfo::getTechPrereq, "int ()")

		// Arrays

		.def("getProductionToCommerceModifier", &CvProcessInfo::getProductionToCommerceModifier, "int (int i)")
		;

	python::class_<CvVoteInfo, python::bases<CvInfoBase> >("CvVoteInfo")
		.def("getPopulationThreshold", &CvVoteInfo::getPopulationThreshold, "int ()")
		.def("getTradeRoutes", &CvVoteInfo::getTradeRoutes, "int ()")

		.def("isVictory", &CvVoteInfo::isVictory, "bool ()")
		.def("isFreeTrade", &CvVoteInfo::isFreeTrade, "bool ()")
		.def("isNoNukes", &CvVoteInfo::isNoNukes, "bool ()")

		// Arrays

		.def("isForceCivic", &CvVoteInfo::isForceCivic, "bool (int i)")
		;

	python::class_<CvProjectInfo, python::bases<CvInfoBase> >("CvProjectInfo")
		.def("getVictoryPrereq", &CvProjectInfo::getVictoryPrereq, "int ()")
		.def("getTechPrereq", &CvProjectInfo::getTechPrereq, "int ()")
		.def("getAnyoneProjectPrereq", &CvProjectInfo::getAnyoneProjectPrereq, "int ()")
		.def("getMaxGlobalInstances", &CvProjectInfo::getMaxGlobalInstances, "int ()")
		.def("getMaxTeamInstances", &CvProjectInfo::getMaxTeamInstances, "int ()")
		.def("getProductionCost", &CvProjectInfo::getProductionCost, "int ()")
		.def("getNukeInterception", &CvProjectInfo::getNukeInterception, "int ()")
		.def("getTechShare", &CvProjectInfo::getTechShare, "int ()")
		.def("getEveryoneSpecialUnit", &CvProjectInfo::getEveryoneSpecialUnit, "int ()")
		.def("getEveryoneSpecialBuilding", &CvProjectInfo::getEveryoneSpecialBuilding, "int ()")

		.def("isSpaceship", &CvProjectInfo::isSpaceship, "bool ()")

		.def("getMovieArtDef", &CvProjectInfo::getMovieArtDef, "string ()")
		.def("getCreateSound", &CvProjectInfo::getCreateSound, "string ()")

		// Arrays

		.def("getBonusProductionModifier", &CvProjectInfo::getBonusProductionModifier, "int (int i)")
		.def("getVictoryThreshold", &CvProjectInfo::getVictoryThreshold, "int (int i)")
		.def("getProjectsNeeded", &CvProjectInfo::getProjectsNeeded, "int (int i)")
		;

	python::class_<CvReligionInfo, python::bases<CvInfoBase> >("CvReligionInfo")
		.def("getChar", &CvReligionInfo::getChar, "int ()")
		.def("getHolyCityChar", &CvReligionInfo::getHolyCityChar, "int ()")
		.def("getTechPrereq", &CvReligionInfo::getTechPrereq, "int ()")
		.def("getFreeUnitClass", &CvReligionInfo::getFreeUnitClass, "int ()")
		.def("getSpreadFactor", &CvReligionInfo::getSpreadFactor, "int ()")
		.def("getMissionType", &CvReligionInfo::getMissionType, "int ()")

		.def("getTechButton", &CvReligionInfo::getTechButton, "string ()")
		.def("getMovieFile", &CvReligionInfo::getMovieFile, "string ()")
		.def("getMovieSound", &CvReligionInfo::getMovieSound, "string ()")
		.def("getSound", &CvReligionInfo::getSound, "string ()")
		.def("getButtonDisabled", &CvReligionInfo::getButtonDisabled, "string ()")

		// Arrays

		.def("getGlobalReligionCommerce", &CvReligionInfo::getGlobalReligionCommerce, "int (int i)")
		.def("getHolyCityCommerce", &CvReligionInfo::getHolyCityCommerce, "int (int i)")
		.def("getStateReligionCommerce", &CvReligionInfo::getStateReligionCommerce, "int (int i)")
		;

	python::class_<CvTraitInfo, python::bases<CvInfoBase> >("CvTraitInfo")
		.def("getHealth", &CvTraitInfo::getHealth, "int ()")
		.def("getMaxAnarchy", &CvTraitInfo::getMaxAnarchy, "int ()")
		.def("getUpkeepModifier", &CvTraitInfo::getUpkeepModifier, "int ()")
		.def("getGreatPeopleRateModifier", &CvTraitInfo::getGreatPeopleRateModifier, "int ()")
		.def("getMaxGlobalBuildingProductionModifier", &CvTraitInfo::getMaxGlobalBuildingProductionModifier, "int ()")
		.def("getMaxTeamBuildingProductionModifier", &CvTraitInfo::getMaxTeamBuildingProductionModifier, "int ()")
		.def("getMaxPlayerBuildingProductionModifier", &CvTraitInfo::getMaxPlayerBuildingProductionModifier, "int ()")

		.def("getShortDescription", &CvTraitInfo::getShortDescription, "int (int i)")
		.def("getExtraYieldThreshold", &CvTraitInfo::getExtraYieldThreshold, "int (int i)")
		.def("getTradeYieldModifier", &CvTraitInfo::getTradeYieldModifier, "int (int i)")
		.def("getCommerceChange", &CvTraitInfo::getCommerceChange, "int (int i)")
		.def("getCommerceModifier", &CvTraitInfo::getCommerceModifier, "int (int i)")

		.def("isFreePromotion", &CvTraitInfo::isFreePromotion, "int (int i)")
		;

	// CvWorldInfo
	python::class_<CvWorldInfo, python::bases<CvInfoBase> >("CvWorldInfo")
		.def("getDefaultPlayers", &CvWorldInfo::getDefaultPlayers, "int ()")
		.def("getUnitNameModifier", &CvWorldInfo::getUnitNameModifier, "int ()")
		.def("getTargetNumCities", &CvWorldInfo::getTargetNumCities, "int ()")
		.def("getNumFreeBuildingBonuses", &CvWorldInfo::getNumFreeBuildingBonuses, "int ()")
		.def("getBuildingClassPrereqModifier", &CvWorldInfo::getBuildingClassPrereqModifier, "int ()")
		.def("getMaxConscriptModifier", &CvWorldInfo::getMaxConscriptModifier, "int ()")
		.def("getWarWearinessModifier", &CvWorldInfo::getWarWearinessModifier, "int ()")
		.def("getGridWidth", &CvWorldInfo::getGridWidth, "int ()")
		.def("getGridHeight", &CvWorldInfo::getGridHeight, "int ()")
		.def("getTerrainGrainChange", &CvWorldInfo::getTerrainGrainChange, "int ()")
		.def("getFeatureGrainChange", &CvWorldInfo::getFeatureGrainChange, "int ()")
		.def("getResearchPercent", &CvWorldInfo::getResearchPercent, "int ()")
		.def("getTradeProfitPercent", &CvWorldInfo::getTradeProfitPercent, "int ()")
		.def("getDistanceMaintenancePercent", &CvWorldInfo::getDistanceMaintenancePercent, "int ()")
		.def("getNumCitiesMaintenancePercent", &CvWorldInfo::getNumCitiesMaintenancePercent, "int ()")
		.def("getNumCitiesAnarchyPercent", &CvWorldInfo::getNumCitiesAnarchyPercent, "int ()")
		;

	python::class_<CvClimateInfo, python::bases<CvInfoBase> >("CvClimateInfo")
		.def("getDesertPercentChange", &CvClimateInfo::getDesertPercentChange, "int ()")
		.def("getJungleLatitude", &CvClimateInfo::getJungleLatitude, "int ()")
		.def("getHillRange", &CvClimateInfo::getHillRange, "int ()")
		.def("getPeakPercent", &CvClimateInfo::getPeakPercent, "int ()")

		.def("getSnowLatitudeChange", &CvClimateInfo::getSnowLatitudeChange, "float ()")
		.def("getTundraLatitudeChange", &CvClimateInfo::getTundraLatitudeChange, "float ()")
		.def("getGrassLatitudeChange", &CvClimateInfo::getGrassLatitudeChange, "float ()")
		.def("getDesertBottomLatitudeChange", &CvClimateInfo::getDesertBottomLatitudeChange, "float ()")
		.def("getDesertTopLatitudeChange", &CvClimateInfo::getDesertTopLatitudeChange, "float ()")
		.def("getIceLatitude", &CvClimateInfo::getIceLatitude, "float ()")
		.def("getRandIceLatitude", &CvClimateInfo::getRandIceLatitude, "float ()")
		;

	python::class_<CvSeaLevelInfo, python::bases<CvInfoBase> >("CvSeaLevelInfo")
		.def("getSeaLevelChange", &CvSeaLevelInfo::getSeaLevelChange, "int ()")
		;

	python::class_<CvAssetInfoBase>("CvAssetInfoBase")
		.def("setTag", &CvAssetInfoBase::setTag, "void (string)")
		.def("getTag", &CvAssetInfoBase::getTag, "string ()")
		.def("setPath", &CvAssetInfoBase::setPath, "void (string)")
		.def("getPath", &CvAssetInfoBase::getPath, "string ()")
		;

	python::class_<CvArtInfoAsset, python::bases<CvAssetInfoBase> >("CvArtInfoAsset")
		.def("setButton", &CvArtInfoAsset::setButton, "void (string)")
		.def("getButton", &CvArtInfoAsset::getButton, "string ()")
		.def("setNIF", &CvArtInfoAsset::setNIF, "void (string)")
		.def("getNIF", &CvArtInfoAsset::getNIF, "string ()")
		.def("setKFM", &CvArtInfoAsset::setKFM, "void (string)")
		.def("getKFM", &CvArtInfoAsset::getKFM, "string ()")
		;

	python::class_<CvArtInfoScalableAsset, python::bases<CvArtInfoAsset, CvScalableInfo> >("CvArtInfoScalableAsset")
		;

	python::class_<CvArtInfoInterface, python::bases<CvArtInfoAsset> >("CvArtInfoInterface")
		;

	python::class_<CvArtInfoMovie, python::bases<CvArtInfoAsset> >("CvArtInfoMovie")
		;

	python::class_<CvArtInfoMisc, python::bases<CvArtInfoAsset> >("CvArtInfoMisc")
		;

	python::class_<CvArtInfoUnit, python::bases<CvArtInfoScalableAsset> >("CvArtInfoUnit")
		.def("getInterfaceScale", &CvArtInfoUnit::getInterfaceScale, "float ()")
		.def("getKFM", &CvArtInfoUnit::getKFM, "string ()")
		;

	python::class_<CvArtInfoBuilding, python::bases<CvArtInfoScalableAsset> >("CvArtInfoBuilding")
		.def("isAnimated", &CvArtInfoBuilding::isAnimated, "bool ()")
		;

	python::class_<CvArtInfoCivilization, python::bases<CvArtInfoAsset> >("CvArtInfoCivilization")
		.def("isWhiteFlag", &CvArtInfoCivilization::isWhiteFlag, "bool ()")
		;

	python::class_<CvArtInfoLeaderhead, python::bases<CvArtInfoAsset> >("CvArtInfoLeaderhead")
		;

	python::class_<CvArtInfoBonus, python::bases<CvArtInfoScalableAsset> >("CvArtInfoBonus")
		;

	python::class_<CvArtInfoImprovement, python::bases<CvArtInfoScalableAsset> >("CvArtInfoImprovement")
		.def("isExtraAnimations", &CvArtInfoImprovement::isExtraAnimations, "bool ()")
		;

	python::class_<CvArtInfoTerrain, python::bases<CvArtInfoAsset> >("CvArtInfoTerrain")
		;

	python::class_<CvArtInfoFeature, python::bases<CvArtInfoScalableAsset> >("CvArtInfoFeature")
		.def("isAnimated", &CvArtInfoFeature::isAnimated, "bool ()")
		.def("isRiverArt", &CvArtInfoFeature::isRiverArt, "bool ()")
		;

	python::class_<CvEmphasizeInfo, python::bases<CvInfoBase> >("CvEmphasizeInfo")
		.def("isAvoidGrowth", &CvEmphasizeInfo::isAvoidGrowth, "bool ()")
		.def("isGreatPeople", &CvEmphasizeInfo::isGreatPeople, "bool ()")

		// Arrays

		.def("getYieldChange", &CvEmphasizeInfo::getYieldChange, "int (int i)")
		.def("getCommerceChange", &CvEmphasizeInfo::getCommerceChange, "int (int i)")
		;

	python::class_<CvUpkeepInfo, python::bases<CvInfoBase> >("CvUpkeepInfo")
		.def("getPopulationPercent", &CvUpkeepInfo::getPopulationPercent, "int ()")
		.def("getCityPercent", &CvUpkeepInfo::getCityPercent, "int ()")
		;

	python::class_<CvCultureLevelInfo, python::bases<CvInfoBase> >("CvCultureLevelInfo")
		.def("getCityDefenseModifier", &CvCultureLevelInfo::getCityDefenseModifier, "int ()")

		.def("getSpeedThreshold", &CvCultureLevelInfo::getSpeedThreshold, "int ()")
		;

	python::class_<CvEraInfo, python::bases<CvInfoBase> >("CvEraInfo")
		.def("getStartingUnitMultiplier", &CvEraInfo::getStartingUnitMultiplier, "int () -")
		.def("getStartingDefenseUnits", &CvEraInfo::getStartingDefenseUnits, "int () -")
		.def("getStartingWorkerUnits", &CvEraInfo::getStartingWorkerUnits, "int () -")
		.def("getStartingExploreUnits", &CvEraInfo::getStartingExploreUnits, "int () -")
		.def("getStartingGold", &CvEraInfo::getStartingGold, "int () -")
		.def("getFreePopulation", &CvEraInfo::getFreePopulation, "int () -")
		.def("getStartPercent", &CvEraInfo::getStartPercent, "int () -")
		.def("getGrowthPercent", &CvEraInfo::getGrowthPercent, "int () -")
		.def("getTrainPercent", &CvEraInfo::getTrainPercent, "int () -")
		.def("getConstructPercent", &CvEraInfo::getConstructPercent, "int () -")
		.def("getCreatePercent", &CvEraInfo::getCreatePercent, "int () -")
		.def("getResearchPercent", &CvEraInfo::getResearchPercent, "int () -")
		.def("getBuildPercent", &CvEraInfo::getBuildPercent, "int () -")
		.def("getImprovementPercent", &CvEraInfo::getImprovementPercent, "int () -")
		.def("getGreatPeoplePercent", &CvEraInfo::getGreatPeoplePercent, "int () -")
		.def("getAnarchyPercent", &CvEraInfo::getAnarchyPercent, "int () -")
		.def("getSoundtrackSpace", &CvEraInfo::getSoundtrackSpace, "int () -")
		.def("getNumSoundtracks", &CvEraInfo::getNumSoundtracks, "int () -")
		.def("getAudioUnitVictoryScript", &CvEraInfo::getAudioUnitVictoryScript, "string () -")
		.def("getAudioUnitDefeatScript", &CvEraInfo::getAudioUnitDefeatScript, "string () -")

		.def("isNoGoodies", &CvEraInfo::isNoGoodies, "bool () -")
		.def("isNoAnimals", &CvEraInfo::isNoAnimals, "bool () -")
		.def("isNoBarbUnits", &CvEraInfo::isNoBarbUnits, "bool () -")
		.def("isNoBarbCities", &CvEraInfo::isNoBarbCities, "bool () -")

		// Arrays

		.def("getSoundtracks", &CvEraInfo::getSoundtracks, "int (int i) -")
		.def("getCitySoundscapeSciptId", &CvEraInfo::getCitySoundscapeSciptId, "int (int i) -")
		;

	python::class_<CvColorInfo, python::bases<CvInfoBase> >("CvColorInfo")
		.def("getColor", &CvColorInfo::getColor,  python::return_value_policy<python::reference_existing_object>())
		;

	python::class_<CvPlayerColorInfo, python::bases<CvInfoBase> >("CvPlayerColorInfo")
		.def("getColorTypePrimary", &CvPlayerColorInfo::getColorTypePrimary, "int ()")
		.def("getColorTypeSecondary", &CvPlayerColorInfo::getColorTypeSecondary, "int ()")
		.def("getTextColorType", &CvPlayerColorInfo::getTextColorType, "int ()")
		;

	python::class_<CvGameText, python::bases<CvInfoBase> >("CvGameText")
		.def("getText", &CvGameText::pyGetText, "wstring ()")
		.def("setText", &CvGameText::setText, "void (wstring)")
		.def("getNumLanguages", &CvGameText::getNumLanguages, "int ()")
		;

	python::class_<CvDiplomacyTextInfo, python::bases<CvInfoBase> >("CvDiplomacyTextInfo")
		.def("getResponse", &CvDiplomacyTextInfo::getResponse,  python::return_value_policy<python::reference_existing_object>(), "Response (int iNum)")
		.def("getNumResponses", &CvDiplomacyTextInfo::getNumResponses, "int ()")

		.def("getCivilizationTypes", &CvDiplomacyTextInfo::getCivilizationTypes, "bool (int i, int j)")
		.def("getLeaderHeadTypes", &CvDiplomacyTextInfo::getLeaderHeadTypes, "bool (int i, int j)")
		.def("getAttitudeTypes", &CvDiplomacyTextInfo::getAttitudeTypes, "bool (int i, int j)")
		.def("getDiplomacyPowerTypes", &CvDiplomacyTextInfo::getDiplomacyPowerTypes, "bool (int i, int j)")

		.def("getNumDiplomacyText", &CvDiplomacyTextInfo::getNumDiplomacyText, "int (int i)")

		.def("getDiplomacyText", &CvDiplomacyTextInfo::getDiplomacyText, "string (int i, int j)")
		;

	python::class_<CvDiplomacyInfo, python::bases<CvInfoBase> >("CvDiplomacyInfo")
		.def("getResponse", &CvDiplomacyInfo::getResponse,  python::return_value_policy<python::reference_existing_object>(), "CvDiplomacyResponse (int iNum)")
		.def("getNumResponses", &CvDiplomacyInfo::getNumResponses, "int ()")

		.def("getCivilizationTypes", &CvDiplomacyInfo::getCivilizationTypes, "bool (int i, int j)")
		.def("getLeaderHeadTypes", &CvDiplomacyInfo::getLeaderHeadTypes, "bool (int i, int j)")
		.def("getAttitudeTypes", &CvDiplomacyInfo::getAttitudeTypes, "bool (int i, int j)")
		.def("getDiplomacyPowerTypes", &CvDiplomacyInfo::getDiplomacyPowerTypes, "bool (int i, int j)")

		.def("getNumDiplomacyText", &CvDiplomacyInfo::getNumDiplomacyText, "int (int i)")

		.def("getDiplomacyText", &CvDiplomacyInfo::getDiplomacyText, "string (int i, int j)")
		;

	python::class_<CvEffectInfo, python::bases<CvInfoBase, CvScalableInfo> >("CvEffectInfo")
		.def("getPath", &CvEffectInfo::getPath, "string ()")
		.def("setPath", &CvEffectInfo::setPath, "void (string)")
		;

	python::class_<CvControlInfo, python::bases<CvInfoBase> >("CvControlInfo")
		.def("getActionInfoIndex", &CvControlInfo::getActionInfoIndex, "int ()")
		;

	python::class_<CvQuestInfo, python::bases<CvInfoBase> >("CvQuestInfo")
		.def("getQuestMessages", &CvQuestInfo::getQuestMessages, "int ()")
		.def("getNumQuestLinks", &CvQuestInfo::getNumQuestLinks, "int ()")
		.def("getNumQuestSounds", &CvQuestInfo::getNumQuestSounds, "int ()")

		.def("getQuestObjective", &CvQuestInfo::getQuestObjective, "string ()")
		.def("getQuestBodyText", &CvQuestInfo::getQuestBodyText, "string ()")
		.def("getNumQuestMessages", &CvQuestInfo::getNumQuestMessages, "string ()")
		.def("getQuestLinkType", &CvQuestInfo::getQuestLinkType, "string ()")
		.def("getQuestLinkName", &CvQuestInfo::getQuestLinkName, "string ()")
		.def("getQuestSounds", &CvQuestInfo::getQuestSounds, "string ()")

		.def("setNumQuestMessages", &CvQuestInfo::setNumQuestMessages, "void (int)")

		.def("setQuestObjective", &CvQuestInfo::setQuestObjective, "void (string)")
		.def("setQuestBodyText", &CvQuestInfo::setQuestBodyText, "void (string)")
		.def("setQuestMessages", &CvQuestInfo::setQuestMessages, "void (int iIndex, string)")
		;

	python::class_<CvTutorialMessage>("CvTutorialMessage")
		.def("getText", &CvTutorialMessage::getText, "string ()")
		.def("getImage", &CvTutorialMessage::getImage, "string ()")
		.def("getSound", &CvTutorialMessage::getSound, "string ()")

		.def("getNumTutorialScripts", &CvTutorialMessage::getNumTutorialScripts, "int ()")
		.def("getTutorialScriptByIndex", &CvTutorialMessage::getTutorialScriptByIndex, "int (int i)")
		;

	python::class_<CvTutorialInfo, python::bases<CvInfoBase> >("CvTutorialInfo")
		.def("getNextTutorialInfoType", &CvTutorialInfo::getNextTutorialInfoType, "string ()")

		.def("getNumTutorialMessages", &CvTutorialInfo::getNumTutorialMessages, "int ()")
		.def("getTutorialMessage", &CvTutorialInfo::getTutorialMessage,  python::return_value_policy<python::reference_existing_object>(), "CvTutorialMessage* (int iIndex)")
		;

	python::class_<CvAutomateInfo, python::bases<CvInfoBase> >("CvAutomateInfo")
		;

	python::class_<CvCommandInfo, python::bases<CvInfoBase> >("CvCommandInfo")
		;

	python::class_<CvGameOptionInfo, python::bases<CvInfoBase> >("CvGameOptionInfo")
		.def("getDefault", &CvGameOptionInfo::getDefault, "bool ()")
		;

	python::class_<CvMPOptionInfo, python::bases<CvInfoBase> >("CvMPOptionInfo")
		.def("getDefault", &CvMPOptionInfo::getDefault, "bool ()")
		;

	python::class_<CvForceControlInfo, python::bases<CvInfoBase> >("CvForceControlInfo")
		.def("getDefault", &CvForceControlInfo::getDefault, "bool ()")
		;

	python::class_<CvPlayerOptionInfo, python::bases<CvInfoBase> >("CvPlayerOptionInfo")
		.def("getDefault", &CvPlayerOptionInfo::getDefault, "bool ()")
		;

	python::class_<CvGraphicOptionInfo, python::bases<CvInfoBase> >("CvGraphicOptionInfo")
		.def("getDefault", &CvGraphicOptionInfo::getDefault, "bool ()")
		;

}
