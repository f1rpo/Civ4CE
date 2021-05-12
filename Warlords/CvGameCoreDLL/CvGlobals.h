// CvGlobals.h

#ifndef CIV4_GLOBALS_H
#define CIV4_GLOBALS_H

//#include "CvStructs.h"
//
// 'global' vars for Civ IV.  singleton class.
// All globals and global types should be contained in this class
// Author -	Mustafa Thamer
//					Jon Shafer - 03/2005
//

class FProfiler;
class CvDLLUtilityIFaceBase;
class CvRandom;
class CvGameAI;
class CMessageControl;
class CvDropMgr;
class CMessageQueue;
class CvSetupData;
class CvInitCore;
class CvMessageCodeTranslator;
class CvPortal;
class CvStatsReporter;
class CvDLLInterfaceIFaceBase;
class CvPlayerAI;
class CvDiplomacyScreen;
class CvCivicsScreen;
class CvWBUnitEditScreen;
class CvWBCityEditScreen;
class CMPDiplomacyScreen;
class CvLightDebugScreen;
class FMPIManager;
class FAStar;
class CvInterface;
class CMainMenu;
class CvEngine;
class CvArtFileMgr;
class FVariableSystem;
class CvWaterPlaneInfo;
class CvMap;
class CvPlayerAI;
class CvTeamAI;
class CvInterfaceModeInfo;
class CvWorldInfo;
class CvClimateInfo;
class CvSeaLevelInfo;
class CvColorInfo;
class CvPlayerColorInfo;
class CvAdvisorInfo;
class CvRouteModelInfo;
class CvRiverInfo;
class CvRiverModelInfo;
class CvWaterPlaneInfo;
class CvAnimationPathInfo;
class CvAnimationCategoryInfo;
class CvEntityEventInfo;
class CvEffectInfo;
class CvAttachableInfo;
class CvCameraInfo;
class CvUnitFormationInfo;
class CvGameText;
class CvLandscapeInfo;
class CvTerrainInfo;
class CvBonusClassInfo;
class CvBonusInfo;
class CvFeatureInfo;
class CvCivilizationInfo;
class CvLeaderHeadInfo;
class CvTraitInfo;
class CvCursorInfo;
class CvThroneRoomCamera;
class CvThroneRoomInfo;
class CvThroneRoomStyleInfo;
class CvSlideShowInfo;
class CvSlideShowRandomInfo;
class CvWorldPickerInfo;
class CvUnitInfo;
class CvSpecialUnitInfo;
class CvInfoBase;
class CvYieldInfo;
class CvCommerceInfo;
class CvRouteInfo;
class CvImprovementInfo;
class CvGoodyInfo;
class CvBuildInfo;
class CvHandicapInfo;
class CvGameSpeedInfo;
class CvTurnTimerInfo;
class CvProcessInfo;
class CvVoteInfo;
class CvProjectInfo;
class CvBuildingClassInfo;
class CvBuildingInfo;
class CvSpecialBuildingInfo;
class CvUnitClassInfo;
class CvActionInfo;
class CvMissionInfo;
class CvControlInfo;
class CvCommandInfo;
class CvAutomateInfo;
class CvPromotionInfo;
class CvTechInfo;
class CvReligionInfo;
class CvSpecialistInfo;
class CvCivicOptionInfo;
class CvCivicInfo;
class CvDiplomacyInfo;
class CvEraInfo;
class CvHurryInfo;
class CvEmphasizeInfo;
class CvUpkeepInfo;
class CvCultureLevelInfo;
class CvVictoryInfo;
class CvQuestInfo;
class CvGameOptionInfo;
class CvMPOptionInfo;
class CvForceControlInfo;
class CvPlayerOptionInfo;
class CvGraphicOptionInfo;
class CvTutorialInfo;

class CvGlobals
{
//	friend class CvDLLUtilityIFace;
	friend class CvXMLLoadUtility;
public:

	// singleton accessor
	DllExport inline static CvGlobals& getInstance();

	DllExport CvGlobals();
	DllExport virtual ~CvGlobals();

	DllExport void init();
	DllExport void uninit();
	DllExport void clearTypesMap();

	DllExport CvDiplomacyScreen* getDiplomacyScreen();
	DllExport CMPDiplomacyScreen* getMPDiplomacyScreen();
	DllExport CvLightDebugScreen* getLightDebugScreenPtr();

	DllExport FMPIManager*& getFMPMgrPtr();
	DllExport CvPortal& getPortal();
	DllExport CvSetupData& getSetupData();
	DllExport CvInitCore& getInitCore();
	DllExport CvInitCore& getLoadedInitCore();
	DllExport CvInitCore& getIniInitCore();
	DllExport CvMessageCodeTranslator& getMessageCodes();
	DllExport CvStatsReporter& getStatsReporter();
	DllExport CvStatsReporter* getStatsReporterPtr();
	DllExport CvInterface& getInterface();
	DllExport CvInterface* getInterfacePtr();
	DllExport int getMaxCivPlayers() const;
#ifdef _USRDLL
	CvMap& getMapINLINE() { return *m_map; }				// inlined for perf reasons, do not use outside of dll
	CvGameAI& getGameINLINE() { return *m_game; }			// inlined for perf reasons, do not use outside of dll
#endif
	DllExport CvMap& getMap();
	DllExport CvGameAI& getGame();
	DllExport CvRandom& getASyncRand();
	DllExport CMessageQueue& getMessageQueue();
	DllExport CMessageQueue& getHotMessageQueue();
	DllExport CMessageControl& getMessageControl();
	DllExport CvDropMgr& getDropMgr();
	DllExport FAStar& getPathFinder();
	DllExport FAStar& getInterfacePathFinder();
	DllExport FAStar& getStepFinder();
	DllExport FAStar& getRouteFinder();
	DllExport FAStar& getBorderFinder();
	DllExport FAStar& getAreaFinder();
	DllExport FAStar& getPlotGroupFinder();
	DllExport NiPoint3& getPt3Origin();

	DllExport CvInterfaceModeInfo*& getInterfaceModeInfo();
	DllExport CvInterfaceModeInfo& getInterfaceModeInfo(InterfaceModeTypes e);

	DllExport NiPoint3& getPt3CameraDir();

	DllExport bool& getLogging();
	DllExport bool& getRandLogging();
	DllExport bool& getSynchLogging();
	DllExport bool& overwriteLogs();

	DllExport int* getPlotDirectionX();
	DllExport int* getPlotDirectionY();
	DllExport int* getPlotCardinalDirectionX();
	DllExport int* getPlotCardinalDirectionY();
	DllExport int* getCityPlotX();
	DllExport int* getCityPlotY();
	DllExport int* getCityPlotPriority();
	DllExport int getXYCityPlot(int i, int j);
	DirectionTypes* getTurnLeftDirection();
	DirectionTypes getTurnLeftDirection(int i);
	DirectionTypes* getTurnRightDirection();
	DirectionTypes getTurnRightDirection(int i);
	DllExport DirectionTypes getXYDirection(int i, int j);

	//
	// Global Infos
	// All info type strings are upper case and are kept in this hash map for fast lookup
	//
	DllExport int getInfoTypeForString(const char* szType) const;			// returns the infos index, use this when searching for an info type string
	DllExport void setInfoTypeFromString(const char* szType, int idx);

	DllExport int& getNumWorldInfos();
	CvWorldInfo*& getWorldInfo();
	DllExport CvWorldInfo& getWorldInfo(WorldSizeTypes e);

	DllExport int& getNumClimateInfos();
	CvClimateInfo*& getClimateInfo();
	DllExport CvClimateInfo& getClimateInfo(ClimateTypes e);

	DllExport int& getNumSeaLevelInfos();
	CvSeaLevelInfo*& getSeaLevelInfo();
	DllExport CvSeaLevelInfo& getSeaLevelInfo(SeaLevelTypes e);

	DllExport int& getNumColorInfos();
	CvColorInfo*& getColorInfo();
	DllExport CvColorInfo& getColorInfo(ColorTypes e);

	DllExport int& getNumPlayerColorInfos();
	CvPlayerColorInfo*& getPlayerColorInfo();
	DllExport CvPlayerColorInfo& getPlayerColorInfo(PlayerColorTypes e);

	DllExport int& getNumAdvisorInfos();
	CvAdvisorInfo*& getAdvisorInfo();
	DllExport CvAdvisorInfo& getAdvisorInfo(AdvisorTypes e);

	DllExport int& getNumHints();
	CvInfoBase*& getHints();
	DllExport CvInfoBase& getHints(int i);

	DllExport int& getNumRouteModelInfos();
	CvRouteModelInfo*& getRouteModelInfo();
	DllExport CvRouteModelInfo& getRouteModelInfo(int i);
	DllExport float& getRouteZBias();

	DllExport int& getNumRiverInfos();
	CvRiverInfo*& getRiverInfo();
	DllExport CvRiverInfo& getRiverInfo(RiverTypes e);
	DllExport float& getRiverZBias();

	DllExport int& getNumRiverModelInfos();
	CvRiverModelInfo*& getRiverModelInfo();
	DllExport CvRiverModelInfo& getRiverModelInfo(int i);

	DllExport int& getNumWaterPlaneInfos();
	CvWaterPlaneInfo*& getWaterPlaneInfo();
	DllExport CvWaterPlaneInfo& getWaterPlaneInfo(int i);

	DllExport int& getNumAnimationPathInfos();
	CvAnimationPathInfo*& getAnimationPathInfo();
	DllExport CvAnimationPathInfo& getAnimationPathInfo(AnimationPathTypes e);

	DllExport int& getNumAnimationCategoryInfos();
	CvAnimationCategoryInfo*& getAnimationCategoryInfo();
	DllExport CvAnimationCategoryInfo& getAnimationCategoryInfo(AnimationCategoryTypes e);

	DllExport int& getNumEntityEventInfos();
	CvEntityEventInfo*& getEntityEventInfo();
	DllExport CvEntityEventInfo& getEntityEventInfo(EntityEventTypes e);

	DllExport int& getNumEffectInfos();
	CvEffectInfo*& getEffectInfo();
	DllExport CvEffectInfo& getEffectInfo(int i);

	DllExport int& getNumAttachableInfos();
	CvAttachableInfo*& getAttachableInfo();
	DllExport CvAttachableInfo& getAttachableInfo(int i);

	DllExport int& getNumCameraInfos();
	CvCameraInfo*& getCameraInfo();
	DllExport	CvCameraInfo& getCameraInfo(CameraAnimationTypes eCameraAnimationNum);

	DllExport int& getNumUnitFormationInfos();
	CvUnitFormationInfo*& getUnitFormationInfo();
	DllExport CvUnitFormationInfo& getUnitFormationInfo(int i);

	DllExport int& getNumGameTextXML();
	CvGameText*& getGameTextXML();

	DllExport int& getNumLandscapeInfos();
	CvLandscapeInfo*& getLandscapeInfo();
	DllExport CvLandscapeInfo& getLandscapeInfo(int iIndex);
	DllExport int getActiveLandscapeID();
	DllExport void setActiveLandscapeID(int iLandscapeID);

	DllExport int& getNumTerrainInfos();
	CvTerrainInfo*& getTerrainInfo();
	DllExport CvTerrainInfo& getTerrainInfo(TerrainTypes eTerrainNum);

	DllExport int& getNumBonusClassInfos();
	CvBonusClassInfo*& getBonusClassInfo();
	DllExport CvBonusClassInfo& getBonusClassInfo(BonusClassTypes eBonusNum);

	DllExport int& getNumBonusInfos();
	CvBonusInfo*& getBonusInfo();
	DllExport CvBonusInfo& getBonusInfo(BonusTypes eBonusNum);

	DllExport int& getNumFeatureInfos();
	CvFeatureInfo*& getFeatureInfo();
	DllExport	CvFeatureInfo& getFeatureInfo(FeatureTypes eFeatureNum);

	DllExport int& getNumPlayableCivilizationInfos();
	DllExport int& getNumAIPlayableCivilizationInfos();
	DllExport int& getNumCivilizationInfos();
	CvCivilizationInfo*& getCivilizationInfo();
	DllExport	CvCivilizationInfo& getCivilizationInfo(CivilizationTypes eCivilizationNum);

	DllExport int& getNumLeaderHeadInfos();
	CvLeaderHeadInfo*& getLeaderHeadInfo();
	DllExport	CvLeaderHeadInfo& getLeaderHeadInfo(LeaderHeadTypes eLeaderHeadNum);

	DllExport int& getNumTraitInfos();
	CvTraitInfo*& getTraitInfo();
	DllExport	CvTraitInfo& getTraitInfo(TraitTypes eTraitNum);

	DllExport int& getNumCursorInfos();
	CvCursorInfo*& getCursorInfo();
	DllExport	CvCursorInfo& getCursorInfo(CursorTypes eCursorNum);

	DllExport int& getNumThroneRoomCameras();
	CvThroneRoomCamera*& getThroneRoomCamera();
	DllExport	CvThroneRoomCamera& getThroneRoomCamera(int iIndex);

	DllExport int& getNumThroneRoomInfos();
	CvThroneRoomInfo*& getThroneRoomInfo();
	DllExport	CvThroneRoomInfo& getThroneRoomInfo(int iIndex);

	DllExport int& getNumThroneRoomStyleInfos();
	CvThroneRoomStyleInfo*& getThroneRoomStyleInfo();
	DllExport	CvThroneRoomStyleInfo& getThroneRoomStyleInfo(int iIndex);

	DllExport int& getNumSlideShowInfos();
	CvSlideShowInfo*& getSlideShowInfo();
	DllExport	CvSlideShowInfo& getSlideShowInfo(int iIndex);

	DllExport int& getNumSlideShowRandomInfos();
	CvSlideShowRandomInfo*& getSlideShowRandomInfo();
	DllExport	CvSlideShowRandomInfo& getSlideShowRandomInfo(int iIndex);

	DllExport int& getNumWorldPickerInfos();
	CvWorldPickerInfo*& getWorldPickerInfo();
	DllExport	CvWorldPickerInfo& getWorldPickerInfo(int iIndex);

	DllExport int& getNumUnitInfos();
	DllExport CvUnitInfo*& getUnitInfo();
	DllExport	CvUnitInfo& getUnitInfo(UnitTypes eUnitNum);

	DllExport int& getNumSpecialUnitInfos();
	CvSpecialUnitInfo*& getSpecialUnitInfo();
	DllExport	CvSpecialUnitInfo& getSpecialUnitInfo(SpecialUnitTypes eSpecialUnitNum);

	DllExport int& getNumConceptInfos();
	CvInfoBase*& getConceptInfo();
	DllExport CvInfoBase& getConceptInfo(ConceptTypes e);

	DllExport int& getNumCityTabInfos();
	CvInfoBase*& getCityTabInfo();
	DllExport CvInfoBase& getCityTabInfo(CityTabTypes e);

	DllExport int& getNumCalendarInfos();
	CvInfoBase*& getCalendarInfo();
	DllExport CvInfoBase& getCalendarInfo(CalendarTypes e);

	DllExport int& getNumSeasonInfos();
	CvInfoBase*& getSeasonInfo();
	DllExport CvInfoBase& getSeasonInfo(SeasonTypes e);

	DllExport int& getNumMonthInfos();
	CvInfoBase*& getMonthInfo();
	DllExport CvInfoBase& getMonthInfo(MonthTypes e);

	DllExport int& getNumDenialInfos();
	CvInfoBase*& getDenialInfo();
	DllExport CvInfoBase& getDenialInfo(DenialTypes e);

	DllExport int& getNumInvisibleInfos();
	CvInfoBase*& getInvisibleInfo();
	DllExport CvInfoBase& getInvisibleInfo(InvisibleTypes e);

	DllExport int& getNumUnitCombatInfos();
	CvInfoBase*& getUnitCombatInfo();
	DllExport CvInfoBase& getUnitCombatInfo(UnitCombatTypes e);

	CvInfoBase*& getDomainInfo();
	DllExport CvInfoBase& getDomainInfo(DomainTypes e);

	CvInfoBase*& getUnitAIInfo();
	DllExport CvInfoBase& getUnitAIInfo(UnitAITypes eUnitAINum);

	CvInfoBase*& getAttitudeInfo();
	DllExport	CvInfoBase& getAttitudeInfo(AttitudeTypes eAttitudeNum);

	CvInfoBase*& getMemoryInfo();
	DllExport	CvInfoBase& getMemoryInfo(MemoryTypes eMemoryNum);

	DllExport int& getNumGameOptionInfos();
	CvGameOptionInfo*& getGameOptionInfo();
	DllExport	CvGameOptionInfo& getGameOptionInfo(GameOptionTypes eGameOptionNum);

	DllExport int& getNumMPOptionInfos();
	CvMPOptionInfo*& getMPOptionInfo();
	DllExport	CvMPOptionInfo& getMPOptionInfo(MultiplayerOptionTypes eMPOptionNum);

	DllExport int& getNumForceControlInfos();
	CvForceControlInfo*& getForceControlInfo();
	DllExport	CvForceControlInfo& getForceControlInfo(ForceControlTypes eForceControlNum);

	CvPlayerOptionInfo*& getPlayerOptionInfo();
	DllExport	CvPlayerOptionInfo& getPlayerOptionInfo(PlayerOptionTypes ePlayerOptionNum);

	CvGraphicOptionInfo*& getGraphicOptionInfo();
	DllExport	CvGraphicOptionInfo& getGraphicOptionInfo(GraphicOptionTypes eGraphicOptionNum);

	CvYieldInfo*& getYieldInfo();
	DllExport	CvYieldInfo& getYieldInfo(YieldTypes eYieldNum);

	CvCommerceInfo*& getCommerceInfo();
	DllExport	CvCommerceInfo& getCommerceInfo(CommerceTypes eCommerceNum);

	DllExport int& getNumRouteInfos();
	CvRouteInfo*& getRouteInfo();
	DllExport	CvRouteInfo& getRouteInfo(RouteTypes eRouteNum);

	DllExport int& getNumImprovementInfos();
	CvImprovementInfo*& getImprovementInfo();
	DllExport CvImprovementInfo& getImprovementInfo(ImprovementTypes eImprovementNum);

	DllExport int& getNumGoodyInfos();
	CvGoodyInfo*& getGoodyInfo();
	DllExport CvGoodyInfo& getGoodyInfo(GoodyTypes eGoodyNum);

	DllExport int& getNumBuildInfos();
	CvBuildInfo*& getBuildInfo();
	DllExport CvBuildInfo& getBuildInfo(BuildTypes eBuildNum);

	DllExport int& getNumHandicapInfos();
	CvHandicapInfo*& getHandicapInfo();
	DllExport CvHandicapInfo& getHandicapInfo(HandicapTypes eHandicapNum);

	DllExport int& getNumGameSpeedInfos();
	CvGameSpeedInfo*& getGameSpeedInfo();
	DllExport CvGameSpeedInfo& getGameSpeedInfo(GameSpeedTypes eGameSpeedNum);

	DllExport int& getNumTurnTimerInfos();
	CvTurnTimerInfo*& getTurnTimerInfo();
	DllExport CvTurnTimerInfo& getTurnTimerInfo(TurnTimerTypes eTurnTimerNum);

	DllExport int& getNumProcessInfos();
	CvProcessInfo*& getProcessInfo();
	DllExport CvProcessInfo& getProcessInfo(ProcessTypes e);

	DllExport int& getNumVoteInfos();
	CvVoteInfo*& getVoteInfo();
	DllExport CvVoteInfo& getVoteInfo(VoteTypes e);

	DllExport int& getNumProjectInfos();
	CvProjectInfo*& getProjectInfo();
	DllExport CvProjectInfo& getProjectInfo(ProjectTypes e);

	DllExport int& getNumBuildingClassInfos();
	CvBuildingClassInfo*& getBuildingClassInfo();
	DllExport CvBuildingClassInfo& getBuildingClassInfo(BuildingClassTypes eBuildingClassNum);

	DllExport int& getNumBuildingInfos();
	CvBuildingInfo*& getBuildingInfo();
	DllExport CvBuildingInfo& getBuildingInfo(BuildingTypes eBuildingNum);

	DllExport int& getNumSpecialBuildingInfos();
	CvSpecialBuildingInfo*& getSpecialBuildingInfo();
	DllExport CvSpecialBuildingInfo& getSpecialBuildingInfo(SpecialBuildingTypes eSpecialBuildingNum);

	DllExport int& getNumUnitClassInfos();
	CvUnitClassInfo*& getUnitClassInfo();
	DllExport CvUnitClassInfo& getUnitClassInfo(UnitClassTypes eUnitClassNum);

	DllExport int& getNumActionInfos();
	CvActionInfo*& getActionInfo();
	DllExport CvActionInfo& getActionInfo(int i);

	CvMissionInfo*& getMissionInfo();
	DllExport CvMissionInfo& getMissionInfo(MissionTypes eMissionNum);

	CvControlInfo*& getControlInfo();
	DllExport CvControlInfo& getControlInfo(ControlTypes eControlNum);

	CvCommandInfo*& getCommandInfo();
	DllExport CvCommandInfo& getCommandInfo(CommandTypes eCommandNum);

	DllExport int& getNumAutomateInfos();
	CvAutomateInfo*& getAutomateInfo();
	DllExport CvAutomateInfo& getAutomateInfo(int iAutomateNum);

	DllExport int& getNumPromotionInfos();
	CvPromotionInfo*& getPromotionInfo();
	DllExport CvPromotionInfo& getPromotionInfo(PromotionTypes ePromotionNum);

	DllExport int& getNumTechInfos();
	CvTechInfo*& getTechInfo();
	DllExport CvTechInfo& getTechInfo(TechTypes eTechNum);

	DllExport int& getNumReligionInfos();
	CvReligionInfo*& getReligionInfo();
	DllExport CvReligionInfo& getReligionInfo(ReligionTypes eReligionNum);

	DllExport int& getNumSpecialistInfos();
	CvSpecialistInfo*& getSpecialistInfo();
	DllExport	CvSpecialistInfo& getSpecialistInfo(SpecialistTypes eSpecialistNum);

	DllExport int& getNumCivicOptionInfos();
	CvCivicOptionInfo*& getCivicOptionInfo();
	DllExport	CvCivicOptionInfo& getCivicOptionInfo(CivicOptionTypes eCivicOptionNum);

	DllExport int& getNumCivicInfos();
	CvCivicInfo*& getCivicInfo();
	DllExport	CvCivicInfo& getCivicInfo(CivicTypes eCivicNum);

	DllExport int& getNumDiplomacyInfos();
	CvDiplomacyInfo*& getDiplomacyInfo();
	DllExport	CvDiplomacyInfo& getDiplomacyInfo(int iDiplomacyNum);

	DllExport int& getNumEraInfos();
	CvEraInfo*& getEraInfo();
	DllExport	CvEraInfo& getEraInfo(EraTypes eEraNum);

	DllExport int& getNumHurryInfos();
	CvHurryInfo*& getHurryInfo();
	DllExport	CvHurryInfo& getHurryInfo(HurryTypes eHurryNum);

	DllExport int& getNumEmphasizeInfos();
	CvEmphasizeInfo*& getEmphasizeInfo();
	DllExport	CvEmphasizeInfo& getEmphasizeInfo(EmphasizeTypes eEmphasizeNum);

	DllExport int& getNumUpkeepInfos();
	CvUpkeepInfo*& getUpkeepInfo();
	DllExport	CvUpkeepInfo& getUpkeepInfo(UpkeepTypes eUpkeepNum);

	DllExport int& getNumCultureLevelInfos();
	CvCultureLevelInfo*& getCultureLevelInfo();
	DllExport	CvCultureLevelInfo& getCultureLevelInfo(CultureLevelTypes eCultureLevelNum);

	DllExport int& getNumVictoryInfos();
	CvVictoryInfo*& getVictoryInfo();	
	DllExport	CvVictoryInfo& getVictoryInfo(VictoryTypes eVictoryNum);

	DllExport int& getNumQuestInfos();
	CvQuestInfo*& getQuestInfo();
	DllExport CvQuestInfo& getQuestInfo(int iIndex);

	DllExport int& getNumTutorialInfos();
	CvTutorialInfo*& getTutorialInfo();
	DllExport CvTutorialInfo& getTutorialInfo(int i);

	//
	// Global Types
	// All type strings are upper case and are kept in this hash map for fast lookup
	// The other functions are kept for convenience when enumerating, but most are not used
	//
	DllExport int getTypesEnum(const char* szType) const;				// use this when searching for a type
	DllExport void setTypesEnum(const char* szType, int iEnum);

	DllExport int getNUM_ENGINE_DIRTY_BITS() const;
	DllExport int getNUM_INTERFACE_DIRTY_BITS() const;
	DllExport int getNUM_YIELD_TYPES() const;
	DllExport int getNUM_COMMERCE_TYPES() const;
	DllExport int getNUM_FORCECONTROL_TYPES() const;
	DllExport int getNUM_INFOBAR_TYPES() const;
	DllExport int getNUM_HEALTHBAR_TYPES() const;
	DllExport int getNUM_CONTROL_TYPES() const;
	DllExport int getNUM_LEADERANIM_TYPES() const;

	DllExport int& getNumEntityEventTypes();
	CvString*& getEntityEventTypes();
	DllExport CvString& getEntityEventTypes(EntityEventTypes e);

	DllExport int& getNumAnimationOperatorTypes();
	CvString*& getAnimationOperatorTypes();
	DllExport CvString& getAnimationOperatorTypes(AnimationOperatorTypes e);

	CvString*& getFunctionTypes();
	DllExport CvString& getFunctionTypes(FunctionTypes e);

	DllExport int& getNumFlavorTypes();
	CvString*& getFlavorTypes();
	DllExport CvString& getFlavorTypes(FlavorTypes e);

	DllExport int& getNumArtStyleTypes();
	CvString*& getArtStyleTypes();
	DllExport CvString& getArtStyleTypes(ArtStyleTypes e);

	DllExport int& getNumCitySizeTypes();
	CvString*& getCitySizeTypes();
	DllExport CvString& getCitySizeTypes(int i);

	CvString*& getContactTypes();
	DllExport CvString& getContactTypes(ContactTypes e);

	CvString*& getDiplomacyPowerTypes();
	DllExport CvString& getDiplomacyPowerTypes(DiplomacyPowerTypes e);

	CvString*& getAutomateTypes();
	DllExport CvString& getAutomateTypes(AutomateTypes e);

	CvString*& getDirectionTypes();
	DllExport CvString& getDirectionTypes(AutomateTypes e);

	DllExport int& getNumFootstepAudioTypes();
	CvString*& getFootstepAudioTypes();
	DllExport CvString& getFootstepAudioTypes(int i);
	DllExport int getFootstepAudioTypeByTag(CvString strTag);

	CvString*& getFootstepAudioTags();
	DllExport CvString& getFootstepAudioTags(int i);

	CvString& getCurrentXMLFile();
	void setCurrentXMLFile(const TCHAR* szFileName);

	//
	///////////////// BEGIN global defines
	// THESE ARE READ-ONLY
	//

	DllExport FVariableSystem* getDefinesVarSystem();
	DllExport void cacheGlobals();

	// ***** EXPOSED TO PYTHON *****
	DllExport int getDefineINT( const char * szName ) const;
	DllExport float getDefineFLOAT( const char * szName ) const;
	DllExport const char * getDefineSTRING( const char * szName ) const;
	DllExport void setDefineINT( const char * szName, int iValue );
	DllExport void setDefineFLOAT( const char * szName, float fValue );
	DllExport void setDefineSTRING( const char * szName, const char * szValue );

	DllExport int getMOVE_DENOMINATOR();
	DllExport int getNUM_UNIT_PREREQ_OR_BONUSES();
	DllExport int getNUM_BUILDING_PREREQ_OR_BONUSES();
	DllExport int getFOOD_CONSUMPTION_PER_POPULATION();
	DllExport int getMAX_HIT_POINTS();
	DllExport int getHILLS_EXTRA_DEFENSE();
	DllExport int getRIVER_ATTACK_MODIFIER();
	DllExport int getAMPHIB_ATTACK_MODIFIER();
	DllExport int getHILLS_EXTRA_MOVEMENT();
	DllExport int getFORTIFY_MODIFIER_PER_TURN();
	DllExport int getMAX_CITY_DEFENSE_DAMAGE();

	DllExport int getMAX_CIV_PLAYERS();
	DllExport int getMAX_PLAYERS();
	DllExport int getMAX_CIV_TEAMS();
	DllExport int getMAX_TEAMS();
	DllExport int getBARBARIAN_PLAYER();
	DllExport int getBARBARIAN_TEAM();
	DllExport int getINVALID_PLOT_COORD();
	DllExport int getNUM_CITY_PLOTS();
	DllExport int getCITY_HOME_PLOT();

	// ***** END EXPOSED TO PYTHON *****

	////////////// END DEFINES //////////////////

	DllExport void setDLLIFace(CvDLLUtilityIFaceBase* pDll);
#ifdef _USRDLL
	CvDLLUtilityIFaceBase* getDLLIFace() { return m_pDLL; }		// inlined for perf reasons, do not use outside of dll
#endif
	DllExport CvDLLUtilityIFaceBase* getDLLIFaceNonInl();
	DllExport void setDLLProfiler(FProfiler* prof);
	FProfiler* getDLLProfiler();
	DllExport void enableDLLProfiler(bool bEnable);
	bool isDLLProfilerEnabled() const;

	DllExport bool IsGraphicsInitialized() const;
	DllExport void SetGraphicsInitialized(bool bVal);

	// for caching
	DllExport bool readBuildingInfoArray(FDataStreamBase* pStream);
	DllExport void writeBuildingInfoArray(FDataStreamBase* pStream);

	DllExport bool readTechInfoArray(FDataStreamBase* pStream);
	DllExport void writeTechInfoArray(FDataStreamBase* pStream);

	DllExport bool readUnitInfoArray(FDataStreamBase* pStream);
	DllExport void writeUnitInfoArray(FDataStreamBase* pStream);

	DllExport bool readLeaderHeadInfoArray(FDataStreamBase* pStream);
	DllExport void writeLeaderHeadInfoArray(FDataStreamBase* pStream);

	DllExport bool readCivilizationInfoArray(FDataStreamBase* pStream);
	DllExport void writeCivilizationInfoArray(FDataStreamBase* pStream);

	DllExport bool readPromotionInfoArray(FDataStreamBase* pStream);
	DllExport void writePromotionInfoArray(FDataStreamBase* pStream);

	DllExport bool readDiplomacyInfoArray(FDataStreamBase* pStream);
	DllExport void writeDiplomacyInfoArray(FDataStreamBase* pStream);

	DllExport bool readDiploTextInfoArray(FDataStreamBase* pStream);
	DllExport void writeDiploTextInfoArray(FDataStreamBase* pStream);

	DllExport bool readCivicInfoArray(FDataStreamBase* pStream);
	DllExport void writeCivicInfoArray(FDataStreamBase* pStream);

	DllExport bool readHandicapInfoArray(FDataStreamBase* pStream);
	DllExport void writeHandicapInfoArray(FDataStreamBase* pStream);

	DllExport bool readBonusInfoArray(FDataStreamBase* pStream);
	DllExport void writeBonusInfoArray(FDataStreamBase* pStream);

	DllExport bool readImprovementInfoArray(FDataStreamBase* pStream);
	DllExport void writeImprovementInfoArray(FDataStreamBase* pStream);

	//
	// additional accessors for initting globals
	//

	DllExport void setInterface(CvInterface* pVal);
	DllExport void setDiplomacyScreen(CvDiplomacyScreen* pVal);
	DllExport void setMPDiplomacyScreen(CMPDiplomacyScreen* pVal);
	DllExport void setMessageQueue(CMessageQueue* pVal);
	DllExport void setHotJoinMessageQueue(CMessageQueue* pVal);
	DllExport void setMessageControl(CMessageControl* pVal);
	DllExport void setSetupData(CvSetupData* pVal);
	DllExport void setMessageCodeTranslator(CvMessageCodeTranslator* pVal);
	DllExport void setDropMgr(CvDropMgr* pVal);
	DllExport void setPortal(CvPortal* pVal);
	DllExport void setStatsReport(CvStatsReporter* pVal);
	DllExport void setLightDebugScreen(CvLightDebugScreen* pVal);
	DllExport void setPathFinder(FAStar* pVal);
	DllExport void setInterfacePathFinder(FAStar* pVal);
	DllExport void setStepFinder(FAStar* pVal);
	DllExport void setRouteFinder(FAStar* pVal);
	DllExport void setBorderFinder(FAStar* pVal);
	DllExport void setAreaFinder(FAStar* pVal);
	DllExport void setPlotGroupFinder(FAStar* pVal);

	// So that CvEnums are moddable in the DLL
	DllExport int getNumDirections() const;
	DllExport int getNumGameOptions() const;
	DllExport int getNumMPOptions() const;
	DllExport int getNumSpecialOptions() const;
	DllExport int getNumGraphicOptions() const;
	DllExport int getNumTradeableItems() const;
	DllExport int getNumBasicItems() const;
	DllExport int getNumTradeableHeadings() const;
	DllExport int getNumCommandInfos() const;
	DllExport int getNumControlInfos() const;
	DllExport int getNumMissionInfos() const;
	DllExport int getNumPlayerOptionInfos() const;
	DllExport int getMaxNumSymbols() const;
	DllExport int getNumGraphicLevels() const;

protected:

	bool m_bGraphicsInitialized;
	bool m_bDLLProfiler;
	bool m_bLogging;
	bool m_bRandLogging;
	bool m_bSynchLogging;
	bool m_bOverwriteLogs;
	NiPoint3  m_pt3CameraDir;
	int m_iNewPlayers;

	CMainMenu* m_pkMainMenu;

	bool m_bZoomOut;
	bool m_bZoomIn;
	bool m_bLoadGameFromFile;

	FMPIManager * m_pFMPMgr;

	CvRandom* m_asyncRand;

	CvGameAI* m_game;

	CMessageQueue* m_messageQueue;
	CMessageQueue* m_hotJoinMsgQueue;
	CMessageControl* m_messageControl;
	CvSetupData* m_setupData;
	CvInitCore* m_iniInitCore;
	CvInitCore* m_loadedInitCore;
	CvInitCore* m_initCore;
	CvMessageCodeTranslator * m_messageCodes;
	CvDropMgr* m_dropMgr;
	CvPortal* m_portal;
	CvStatsReporter * m_statsReporter;
	CvInterface* m_interface;

	CvArtFileMgr* m_pArtFileMgr;

	CvMap* m_map;

	CvDiplomacyScreen* m_diplomacyScreen;
	CMPDiplomacyScreen* m_mpDiplomacyScreen;
	CvLightDebugScreen* m_pLightDebugScreen;

	FAStar* m_pathFinder;
	FAStar* m_interfacePathFinder;
	FAStar* m_stepFinder;
	FAStar* m_routeFinder;
	FAStar* m_borderFinder;
	FAStar* m_areaFinder;
	FAStar* m_plotGroupFinder;

	NiPoint3 m_pt3Origin;

	int* m_aiPlotDirectionX;	// [NUM_DIRECTION_TYPES];
	int* m_aiPlotDirectionY;	// [NUM_DIRECTION_TYPES];
	int* m_aiPlotCardinalDirectionX;	// [NUM_CARDINALDIRECTION_TYPES];
	int* m_aiPlotCardinalDirectionY;	// [NUM_CARDINALDIRECTION_TYPES];
	int* m_aiCityPlotX;	// [NUM_CITY_PLOTS];
	int* m_aiCityPlotY;	// [NUM_CITY_PLOTS];
	int* m_aiCityPlotPriority;	// [NUM_CITY_PLOTS];
	int m_aaiXYCityPlot[CITY_PLOTS_DIAMETER][CITY_PLOTS_DIAMETER];

	DirectionTypes* m_aeTurnLeftDirection;	// [NUM_DIRECTION_TYPES];
	DirectionTypes* m_aeTurnRightDirection;	// [NUM_DIRECTION_TYPES];
	DirectionTypes m_aaeXYDirection[DIRECTION_DIAMETER][DIRECTION_DIAMETER];

	//InterfaceModeInfo m_aInterfaceModeInfo[NUM_INTERFACEMODE_TYPES] =
	CvInterfaceModeInfo* m_paInterfaceModeInfo;

	/***********************************************************************************************************************
	Globals loaded from XML
	************************************************************************************************************************/

	// all type strings are upper case and are kept in this hash map for fast lookup, Moose
	typedef stdext::hash_map<std::string /* type string */, int /* info index */> InfosMap;
	InfosMap m_infosMap;

	CvColorInfo* m_paColorInfo;
	int m_iNumColorInfos;

	CvPlayerColorInfo* m_paPlayerColorInfo;
	int m_iNumPlayerColorInfos;

	CvAdvisorInfo* m_paAdvisorInfo;
	int m_iNumAdvisorInfos;

	CvInfoBase* m_paHints;
	int m_iNumHints;

	//TerrainInfo 
	CvTerrainInfo* m_paTerrainInfo;
	int m_iNumTerrainInfos;

	CvLandscapeInfo* m_paLandscapeInfo;
	int m_iNumLandscapeInfos;
	int m_iActiveLandscapeID;

	CvWorldInfo* m_paWorldInfo;
	int m_iNumWorldInfos;

	CvClimateInfo* m_paClimateInfo;
	int m_iNumClimateInfos;

	CvSeaLevelInfo* m_paSeaLevelInfo;
	int m_iNumSeaLevelInfos;

	CvYieldInfo* m_paYieldInfo;

	CvCommerceInfo* m_paCommerceInfo;

	CvRouteInfo* m_paRouteInfo;
	int m_iNumRouteInfos;
	float m_fRouteZBias;

	CvFeatureInfo* m_paFeatureInfo;
	int m_iNumFeatureInfos;

	CvBonusClassInfo* m_paBonusClassInfo;
	int m_iNumBonusClassInfos;

	CvBonusInfo* m_paBonusInfo;
	int m_iNumBonusInfos;

	CvImprovementInfo* m_paImprovementInfo;
	int m_iNumImprovementInfos;

	CvGoodyInfo* m_paGoodyInfo;
	int m_iNumGoodyInfos;

	CvBuildInfo* m_paBuildInfo;
	int m_iNumBuildInfos;

	CvHandicapInfo* m_paHandicapInfo;
	int m_iNumHandicapInfos;

	CvGameSpeedInfo* m_paGameSpeedInfo;
	int m_iNumGameSpeedInfos;

	CvTurnTimerInfo* m_paTurnTimerInfo;
	int m_iNumTurnTimerInfos;

	CvCivilizationInfo* m_paCivilizationInfo;
	int m_iNumCivilizationInfos;
	int m_iNumPlayableCivilizationInfos;
	int m_iNumAIPlayableCivilizationInfos;

	CvLeaderHeadInfo* m_paLeaderHeadInfo;
	int m_iNumLeaderHeadInfos;

	CvTraitInfo* m_paTraitInfo;
	int m_iNumTraitInfos;

	CvCursorInfo* m_paCursorInfo;
	int m_iNumCursorInfos;

	CvThroneRoomCamera* m_paThroneRoomCamera;
	int m_iNumThroneRoomCameras;

	CvThroneRoomInfo* m_paThroneRoomInfo;
	int m_iNumThroneRoomInfos;

	CvThroneRoomStyleInfo* m_paThroneRoomStyleInfo;
	int m_iNumThroneRoomStyleInfos;

	CvSlideShowInfo* m_paSlideShowInfo;
	int m_iNumSlideShowInfos;

	CvSlideShowRandomInfo* m_paSlideShowRandomInfo;
	int m_iNumSlideShowRandomInfos;

	CvWorldPickerInfo* m_paWorldPickerInfo;
	int m_iNumWorldPickerInfos;

	CvProcessInfo* m_paProcessInfo;
	int m_iNumProcessInfos;

	CvVoteInfo* m_paVoteInfo;
	int m_iNumVoteInfos;

	CvProjectInfo* m_paProjectInfo;
	int m_iNumProjectInfos;

	//BuildingInfo m_aBuildingInfo[NUM_BUILDING_TYPES];
	CvBuildingClassInfo* m_paBuildingClassInfo;
	int m_iNumBuildingClassInfos;

	//BuildingInfo m_aBuildingInfo[NUM_BUILDING_TYPES];
	CvBuildingInfo* m_paBuildingInfo;
	int m_iNumBuildingInfos;

	//SpecialBuildingInfo* m_aSpecialBuildingInfo;	// [NUM_SPECIALBUILDING_TYPES];
	CvSpecialBuildingInfo* m_paSpecialBuildingInfo;
	int m_iNumSpecialBuildingInfos;

	CvUnitClassInfo* m_paUnitClassInfo;
	int m_iNumUnitClassInfos;

	CvUnitInfo* m_paUnitInfo;
	int m_iNumUnitInfos;

	CvSpecialUnitInfo* m_paSpecialUnitInfo;
	int m_iNumSpecialUnitInfos;

	CvInfoBase* m_paConceptInfo;
	int m_iNumConceptInfos;

	CvInfoBase* m_paCityTabInfo;
	int m_iNumCityTabInfos;

	CvInfoBase* m_paCalendarInfo;
	int m_iNumCalendarInfos;

	CvInfoBase* m_paSeasonInfo;
	int m_iNumSeasonInfos;

	CvInfoBase* m_paMonthInfo;
	int m_iNumMonthInfos;

	CvInfoBase* m_paDenialInfo;
	int m_iNumDenialInfos;

	CvInfoBase* m_paInvisibleInfo;
	int m_iNumInvisibleInfos;

	CvInfoBase* m_paUnitCombatInfo;
	int m_iNumUnitCombatInfos;

	CvInfoBase* m_paDomainInfo;

	CvInfoBase* m_paUnitAIInfos;

	CvInfoBase* m_paAttitudeInfos;

	CvInfoBase* m_paMemoryInfos;

	CvInfoBase* m_paFeatInfos;

	CvGameOptionInfo* m_paGameOptionInfos;
	int m_iNumGameOptionInfos;

	CvMPOptionInfo* m_paMPOptionInfos;
	int m_iNumMPOptionInfos;

	CvForceControlInfo* m_paForceControlInfos;
	int m_iNumForceControlInfos;

	CvPlayerOptionInfo* m_paPlayerOptionInfos;

	CvGraphicOptionInfo* m_paGraphicOptionInfos;

	CvSpecialistInfo* m_paSpecialistInfo;
	int m_iNumSpecialistInfos;

	CvEmphasizeInfo* m_paEmphasizeInfo;
	int m_iNumEmphasizeInfos;

	CvUpkeepInfo* m_paUpkeepInfo;
	int m_iNumUpkeepInfos;

	CvCultureLevelInfo* m_paCultureLevelInfo;
	int m_iNumCultureLevelInfos;

	CvReligionInfo* m_paReligionInfo;
	int m_iNumReligionInfos;

	CvActionInfo* m_paActionInfo;
	int m_iNumActionInfos;

	CvMissionInfo* m_paMissionInfo;

	CvControlInfo* m_paControlInfo;

	CvCommandInfo* m_paCommandInfo;

	CvAutomateInfo* m_paAutomateInfo;
	int m_iNumAutomateInfos;

	CvPromotionInfo* m_paPromotionInfo;
	int m_iNumPromotionInfos;

	CvTechInfo* m_paTechInfo;
	int m_iNumTechInfos;

	CvCivicOptionInfo* m_paCivicOptionInfo;
	int m_iNumCivicOptionInfos;

	CvCivicInfo* m_paCivicInfo;
	int m_iNumCivicInfos;

	CvDiplomacyInfo* m_paDiplomacyInfo;
	int m_iNumDiplomacyInfos;

	CvEraInfo* m_aEraInfo;	// [NUM_ERA_TYPES];
	int m_iNumEraInfos;

	CvHurryInfo* m_paHurryInfo;
	int m_iNumHurryInfos;

	CvVictoryInfo* m_paVictoryInfo;
	int m_iNumVictoryInfos;

	CvRouteModelInfo* m_paRouteModelInfo;
	int m_iNumRouteModelInfos;

	CvRiverInfo* m_paRiverInfo;
	int m_iNumRiverInfos;
	float m_fRiverZBias;

	CvRiverModelInfo* m_paRiverModelInfo;
	int m_iNumRiverModelInfos;

	CvWaterPlaneInfo* m_paWaterPlaneInfo;
	int m_iNumWaterPlaneInfos;

	CvAnimationPathInfo* m_paAnimationPathInfo;
	int m_iNumAnimationPathInfos;

	CvAnimationCategoryInfo* m_paAnimationCategoryInfo;
	int m_iNumAnimationCategoryInfos;

	CvEntityEventInfo * m_paEntityEventInfo;
	int m_iNumEntityEventInfos;

	CvUnitFormationInfo * m_paUnitFormationInfo;
	int m_iNumUnitFormationInfos;

	CvEffectInfo* m_paEffectInfo;
	int m_iNumEffectInfos;

	CvAttachableInfo* m_paAttachableInfo;
	int m_iNumAttachableInfos;

	CvCameraInfo* m_paCameraInfo;
	int m_iNumCameraInfos;

	CvQuestInfo* m_paQuestInfo;
	int m_iNumQuestInfos;

	CvTutorialInfo* m_paTutorialInfo;
	int m_iNumTutorialInfos;

	// Game Text
	CvGameText* m_paGameTextXML;
	int m_iNumGameTextXML;

	//////////////////////////////////////////////////////////////////////////
	// GLOBAL TYPES
	//////////////////////////////////////////////////////////////////////////

	// all type strings are upper case and are kept in this hash map for fast lookup, Moose
	typedef stdext::hash_map<std::string /* type string */, int /*enum value */> TypesMap;
	TypesMap m_typesMap;

	// XXX These are duplicates and are kept for enumeration convenience - most could be removed, Moose
	CvString *m_paszEntityEventTypes2;
	CvString *m_paszEntityEventTypes;
	int m_iNumEntityEventTypes;

	CvString *m_paszAnimationOperatorTypes;
	int m_iNumAnimationOperatorTypes;

	CvString* m_paszFunctionTypes;

	CvString* m_paszFlavorTypes;
	int m_iNumFlavorTypes;

	CvString *m_paszArtStyleTypes;
	int m_iNumArtStyleTypes;

	CvString *m_paszCitySizeTypes;
	int m_iNumCitySizeTypes;

	CvString *m_paszContactTypes;

	CvString *m_paszDiplomacyPowerTypes;

	CvString *m_paszAutomateTypes;

	CvString *m_paszDirectionTypes;

	CvString *m_paszFootstepAudioTypes;
	int m_iNumFootstepAudioTypes;

	CvString *m_paszFootstepAudioTags;
	int m_iNumFootstepAudioTags;

	CvString m_szCurrentXMLFile;
	//////////////////////////////////////////////////////////////////////////
	// Formerly Global Defines
	//////////////////////////////////////////////////////////////////////////

	FVariableSystem* m_VarSystem;

	int m_iMOVE_DENOMINATOR;
	int m_iNUM_UNIT_PREREQ_OR_BONUSES;
	int m_iNUM_BUILDING_PREREQ_OR_BONUSES;
	int m_iFOOD_CONSUMPTION_PER_POPULATION;
	int m_iMAX_HIT_POINTS;
	int m_iHILLS_EXTRA_DEFENSE;
	int m_iRIVER_ATTACK_MODIFIER;
	int m_iAMPHIB_ATTACK_MODIFIER;
	int m_iHILLS_EXTRA_MOVEMENT;
	int m_iFORTIFY_MODIFIER_PER_TURN;
	int m_iMAX_CITY_DEFENSE_DAMAGE;

	// DLL interface
	CvDLLUtilityIFaceBase* m_pDLL;

	FProfiler* m_Profiler;		// profiler
	CvString m_szDllProfileText;
};

extern CvGlobals gGlobals;	// for debugging

//
// inlines
//
inline CvGlobals& CvGlobals::getInstance()
{
	return gGlobals;
}


//
// helpers
//
#define GC CvGlobals::getInstance()
#ifndef _USRDLL
#define gDLL GC.getDLLIFaceNonInl()
#else
#define gDLL GC.getDLLIFace()
#endif

#ifndef _USRDLL
#define NUM_DIRECTION_TYPES (GC.getNumDirections())
#define NUM_GAMEOPTION_TYPES (GC.getNumGameOptions())
#define NUM_MPOPTION_TYPES (GC.getNumMPOptions())
#define NUM_SPECIALOPTION_TYPES (GC.getNumSpecialOptions())
#define NUM_GRAPHICOPTION_TYPES (GC.getNumGraphicOptions())
#define NUM_TRADEABLE_ITEMS (GC.getNumTradeableItems())
#define NUM_BASIC_ITEMS (GC.getNumBasicItems())
#define NUM_TRADEABLE_HEADINGS (GC.getNumTradeableHeadings())
#define NUM_COMMAND_TYPES (GC.getNumCommandInfos())
#define NUM_CONTROL_TYPES (GC.getNumControlInfos())
#define NUM_MISSION_TYPES (GC.getNumMissionInfos())
#define NUM_PLAYEROPTION_TYPES (GC.getNumPlayerOptionInfos())
#define MAX_NUM_SYMBOLS (GC.getMaxNumSymbols())
#define NUM_GRAPHICLEVELS (GC.getNumGraphicLevels())
#endif

#endif
