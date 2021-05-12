//
// XML Set functions
//

#include "CvGameCoreDLL.h"
#include "CvDLLXMLIFaceBase.h"
#include "CvXMLLoadUtility.h"
#include "CvGlobals.h"
#include "CvArtFileMgr.h"
#include "CvGameTextMgr.h"
#include <algorithm>
#include "CvInfoWater.h"
#include "FProfiler.h"
#include "FVariableSystem.h"
#include "CvGameCoreUtils.h"

// Macro for Setting Global Art Defines
#define INIT_ART_DEFINE_XML_LOAD(xmlFile, xmlInfoPath, infoArray, numInfos) SetGlobalArtDefineInfo(xmlFile, xmlInfoPath, infoArray, numInfos);
#define INIT_XML_GLOBAL_LOAD(xmlInfoPath, infoArray, numInfos)  SetGlobalClassInfo(infoArray, xmlInfoPath, numInfos);

bool CvXMLLoadUtility::ReadGlobalDefines(char* szXMLFileName, CvCacheObject* cache)
{
	bool bLoaded = false;	// used to make sure that the xml file was loaded correctly

	if (!gDLL->cacheRead(cache, szXMLFileName))			// src data file name
	{
		// load normally
		if (!CreateFXml())
		{
			return false;
		}

		// load the new FXml variable with the szXMLFileName file
		bLoaded = LoadCivXml(m_pFXml, szXMLFileName);
		if (!bLoaded)
		{
			char	szMessage[1024];
			sprintf( szMessage, "LoadXML call failed for %s \n Current XML file is: %s", szXMLFileName, GC.getCurrentXMLFile().GetCString());
			gDLL->MessageBox(szMessage, "XML Load Error");
		}

		// if the load succeeded we will continue
		if (bLoaded)
		{
			// if the xml is successfully validated
			if (Validate())
			{
				// locate the first define tag in the xml
				if (gDLL->getXMLIFace()->LocateNode(m_pFXml,"Civ4Defines/Define"))
				{
					int i;	// loop counter
					// get the number of other Define tags in the xml file
					int iNumDefines = gDLL->getXMLIFace()->GetNumSiblings(m_pFXml);
					// add one to the total in order to include the current Define tag
					iNumDefines++;

					// loop through all the Define tags
					for (i=0;i<iNumDefines;i++)
					{
						char szNodeType[256];	// holds the type of the current node
						char szName[256];

						// Skip any comments and stop at the next value we might want
						if (SkipToNextVal())
						{
							// call the function that sets the FXml pointer to the first non-comment child of 
							// the current tag and gets the value of that new node
							if (GetChildXmlVal(szName))
							{
								// set the FXml pointer to the next sibling of the current tag``
								if (gDLL->getXMLIFace()->NextSibling(GetXML()))
								{
									// Skip any comments and stop at the next value we might want
									if (SkipToNextVal())
									{
										// if we successfuly get the node type for the current tag
										if (gDLL->getXMLIFace()->GetLastLocatedNodeType(GetXML(),szNodeType))
										{
											// if the node type of the current tag isn't null
											if (strcmp(szNodeType,"")!=0)
											{
												// if the node type of the current tag is a float then 
												if (strcmp(szNodeType,"float")==0)
												{
													// get the float value for the define
													float fVal;
													GetXmlVal(&fVal);
													GC.getDefinesVarSystem()->SetValue(szName, fVal);
												}
												// else if the node type of the current tag is an int then
												else if (strcmp(szNodeType,"int")==0)
												{
													// get the int value for the define
													int iVal;
													GetXmlVal(&iVal);
													GC.getDefinesVarSystem()->SetValue(szName, iVal);
												}
												// else if the node type of the current tag is a boolean then
												else if (strcmp(szNodeType,"bool")==0)
												{
													// get the boolean value for the define
													bool bVal;
													GetXmlVal(&bVal);
													GC.getDefinesVarSystem()->SetValue(szName, bVal);
												}
												// otherwise we will assume it is a string/text value
												else
												{
													char szVal[256];
													// get the string/text value for the define
													GetXmlVal(szVal);
													GC.getDefinesVarSystem()->SetValue(szName, szVal);
												}
											}
											// otherwise we will default to getting the string/text value for the define
											else
											{
												char szVal[256];
												// get the string/text value for the define
												GetXmlVal(szVal);
												GC.getDefinesVarSystem()->SetValue(szName, szVal);
											}
										}
									}
								}

								// since we are looking at the children of a Define tag we will need to go up
								// one level so that we can go to the next Define tag.
								// Set the FXml pointer to the parent of the current tag
								gDLL->getXMLIFace()->SetToParent(GetXML());
							}
						}

						// now we set the FXml pointer to the sibling of the current tag, which should be the next
						// Define tag
						if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
						{
							break;
						}
					}

					// write global defines info to cache
					bool bOk = gDLL->cacheWrite(cache);
					if (!bOk)
					{
						char	szMessage[1024];
						sprintf( szMessage, "Failed writing to global defines cache. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
						gDLL->MessageBox(szMessage, "XML Caching Error");
					}
					else
					{
						logMsg("Wrote GlobalDefines to cache");
					}
				}
			}
		}

		// delete the pointer to the FXml variable
		gDLL->getXMLIFace()->DestroyFXml(m_pFXml);
	}
	else
	{
		logMsg("Read GobalDefines from cache");
	}

	return true;
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetGlobalDefines()
//
//  PURPOSE :   Initialize the variables located in globaldefines.cpp/h with the values in
//				GlobalDefines.xml
//
//------------------------------------------------------------------------------------------------------
bool CvXMLLoadUtility::SetGlobalDefines()
{
	UpdateProgressCB("GlobalDefines");

	/////////////////////////////////
	//
	// use disk cache if possible.
	// if no cache or cache is older than xml file, use xml file like normal, else read from cache
	//

	CvCacheObject* cache = gDLL->createGlobalDefinesCacheObject("GlobalDefines.dat");	// cache file name

	if (!ReadGlobalDefines("xml\\GlobalDefines.xml", cache))
	{
		return false;
	}

	if (!ReadGlobalDefines("xml\\GlobalDefinesAlt.xml", cache))
	{
		return false;
	}

	gDLL->destroyCache(cache);
	////////////////////////////////////////////////////////////////////////

	GC.cacheGlobals();

	return true;
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetPostGlobalsGlobalDefines()
//
//  PURPOSE :   This function assumes that the SetGlobalDefines function has already been called
//							it then loads the few global defines that needed to reference a global variable that
//							hadn't been loaded in prior to the SetGlobalDefines call
//
//------------------------------------------------------------------------------------------------------
bool CvXMLLoadUtility::SetPostGlobalsGlobalDefines()
{
	const char* szVal=NULL;		// holds the string value from the define queue
	int idx;

	if (GC.getDefinesVarSystem()->GetSize() > 0)
	{
		SetGlobalDefine("LAND_TERRAIN", szVal); 
		idx = FindInInfoClass(szVal, GC.getTerrainInfo(), sizeof(GC.getTerrainInfo((TerrainTypes) 0)), GC.getNumTerrainInfos());
		GC.getDefinesVarSystem()->SetValue("LAND_TERRAIN", idx);

		SetGlobalDefine("DEEP_WATER_TERRAIN", szVal); 
		idx = FindInInfoClass(szVal, GC.getTerrainInfo(), sizeof(GC.getTerrainInfo((TerrainTypes) 0)), GC.getNumTerrainInfos());
		GC.getDefinesVarSystem()->SetValue("DEEP_WATER_TERRAIN", idx);

		SetGlobalDefine("SHALLOW_WATER_TERRAIN", szVal);
		idx = FindInInfoClass(szVal, GC.getTerrainInfo(), sizeof(GC.getTerrainInfo((TerrainTypes) 0)), GC.getNumTerrainInfos());
		GC.getDefinesVarSystem()->SetValue("SHALLOW_WATER_TERRAIN", idx);

		SetGlobalDefine("LAND_IMPROVEMENT", szVal); 
		idx = FindInInfoClass(szVal, GC.getImprovementInfo(), sizeof(GC.getImprovementInfo((ImprovementTypes) 0)), GC.getNumImprovementInfos());
		GC.getDefinesVarSystem()->SetValue("LAND_IMPROVEMENT", idx);

		SetGlobalDefine("WATER_IMPROVEMENT", szVal); 
		idx = FindInInfoClass(szVal, GC.getImprovementInfo(), sizeof(GC.getImprovementInfo((ImprovementTypes) 0)), GC.getNumImprovementInfos());
		GC.getDefinesVarSystem()->SetValue("WATER_IMPROVEMENT", idx);

		SetGlobalDefine("RUINS_IMPROVEMENT", szVal); 
		idx = FindInInfoClass(szVal, GC.getImprovementInfo(), sizeof(GC.getImprovementInfo((ImprovementTypes) 0)), GC.getNumImprovementInfos());
		GC.getDefinesVarSystem()->SetValue("RUINS_IMPROVEMENT", idx);

		SetGlobalDefine("NUKE_FEATURE", szVal);
		idx = FindInInfoClass(szVal, GC.getFeatureInfo(), sizeof(GC.getFeatureInfo((FeatureTypes) 0)), GC.getNumFeatureInfos());
		GC.getDefinesVarSystem()->SetValue("NUKE_FEATURE", idx);

		SetGlobalDefine("GLOBAL_WARMING_TERRAIN", szVal);
		idx = FindInInfoClass(szVal, GC.getTerrainInfo(), sizeof(GC.getTerrainInfo((TerrainTypes) 0)), GC.getNumTerrainInfos());
		GC.getDefinesVarSystem()->SetValue("GLOBAL_WARMING_TERRAIN", idx);

		SetGlobalDefine("CAPITAL_BUILDINGCLASS", szVal); 
		idx = FindInInfoClass(szVal, GC.getBuildingClassInfo(), sizeof(GC.getBuildingClassInfo((BuildingClassTypes) 0)), GC.getNumBuildingClassInfos());
		GC.getDefinesVarSystem()->SetValue("CAPITAL_BUILDINGCLASS", idx);

		SetGlobalDefine("DEFAULT_SPECIALIST", szVal); 
		idx = FindInInfoClass(szVal, GC.getSpecialistInfo(), sizeof(GC.getSpecialistInfo((SpecialistTypes) 0)), GC.getNumSpecialistInfos());
		GC.getDefinesVarSystem()->SetValue("DEFAULT_SPECIALIST", idx);

		SetGlobalDefine("INITIAL_CITY_ROUTE_TYPE", szVal);
		idx = FindInInfoClass(szVal, GC.getRouteInfo(), sizeof(GC.getRouteInfo((RouteTypes) 0)), GC.getNumRouteInfos());
		GC.getDefinesVarSystem()->SetValue("INITIAL_CITY_ROUTE_TYPE", idx);

		SetGlobalDefine("STANDARD_HANDICAP", szVal);
		idx = FindInInfoClass(szVal, GC.getHandicapInfo(), sizeof(GC.getHandicapInfo((HandicapTypes) 0)), GC.getNumHandicapInfos());
		GC.getDefinesVarSystem()->SetValue("STANDARD_HANDICAP", idx);

		SetGlobalDefine("STANDARD_HANDICAP_QUICK", szVal);
		idx = FindInInfoClass(szVal, GC.getHandicapInfo(), sizeof(GC.getHandicapInfo((HandicapTypes) 0)), GC.getNumHandicapInfos());
		GC.getDefinesVarSystem()->SetValue("STANDARD_HANDICAP_QUICK", idx);

		SetGlobalDefine("STANDARD_GAMESPEED", szVal);
		idx = FindInInfoClass(szVal, GC.getGameSpeedInfo(), sizeof(GC.getGameSpeedInfo((GameSpeedTypes) 0)), GC.getNumGameSpeedInfos());
		GC.getDefinesVarSystem()->SetValue("STANDARD_GAMESPEED", idx);

		SetGlobalDefine("STANDARD_TURNTIMER", szVal);
		idx = FindInInfoClass(szVal, GC.getTurnTimerInfo(), sizeof(GC.getTurnTimerInfo((TurnTimerTypes) 0)), GC.getNumTurnTimerInfos());
		GC.getDefinesVarSystem()->SetValue("STANDARD_TURNTIMER", idx);

		SetGlobalDefine("STANDARD_CLIMATE", szVal);
		idx = FindInInfoClass(szVal, GC.getClimateInfo(), sizeof(GC.getClimateInfo((ClimateTypes) 0)), GC.getNumClimateInfos());
		GC.getDefinesVarSystem()->SetValue("STANDARD_CLIMATE", idx);

		SetGlobalDefine("STANDARD_SEALEVEL", szVal);
		idx = FindInInfoClass(szVal, GC.getSeaLevelInfo(), sizeof(GC.getSeaLevelInfo((SeaLevelTypes) 0)), GC.getNumSeaLevelInfos());
		GC.getDefinesVarSystem()->SetValue("STANDARD_SEALEVEL", idx);

		SetGlobalDefine("STANDARD_ERA", szVal);
		idx = FindInInfoClass(szVal, GC.getEraInfo(), sizeof(GC.getEraInfo((EraTypes) 0)), GC.getNumEraInfos());
		GC.getDefinesVarSystem()->SetValue("STANDARD_ERA", idx);

		SetGlobalDefine("STANDARD_CALENDAR", szVal);
		idx = FindInInfoClass(szVal, GC.getCalendarInfo(), sizeof(GC.getCalendarInfo((CalendarTypes) 0)), GC.getNumCalendarInfos());
		GC.getDefinesVarSystem()->SetValue("STANDARD_CALENDAR", idx);

		SetGlobalDefine("AI_HANDICAP", szVal);
		idx = FindInInfoClass(szVal, GC.getHandicapInfo(), sizeof(GC.getHandicapInfo((HandicapTypes) 0)), GC.getNumHandicapInfos());
		GC.getDefinesVarSystem()->SetValue("AI_HANDICAP", idx);

		SetGlobalDefine("BARBARIAN_HANDICAP", szVal);
		idx = FindInInfoClass(szVal, GC.getHandicapInfo(), sizeof(GC.getHandicapInfo((HandicapTypes) 0)), GC.getNumHandicapInfos());
		GC.getDefinesVarSystem()->SetValue("BARBARIAN_HANDICAP", idx);

		SetGlobalDefine("BARBARIAN_CIVILIZATION", szVal);
		idx = FindInInfoClass(szVal, GC.getCivilizationInfo(), sizeof(GC.getCivilizationInfo((CivilizationTypes) 0)), GC.getNumCivilizationInfos());
		GC.getDefinesVarSystem()->SetValue("BARBARIAN_CIVILIZATION", idx);

		SetGlobalDefine("BARBARIAN_LEADER", szVal);
		idx = FindInInfoClass(szVal, GC.getLeaderHeadInfo(), sizeof(GC.getLeaderHeadInfo((LeaderHeadTypes) 0)), GC.getNumLeaderHeadInfos());
		GC.getDefinesVarSystem()->SetValue("BARBARIAN_LEADER", idx);

		return true;
	}

	char	szMessage[1024];
	sprintf( szMessage, "Size of Global Defines is not greater than 0. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
	gDLL->MessageBox(szMessage, "XML Load Error");

	return false;
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetGlobalTypes()
//
//  PURPOSE :   Initialize the variables located in globaltypes.cpp/h with the values in
//				GlobalTypes.xml
//
//------------------------------------------------------------------------------------------------------
bool CvXMLLoadUtility::SetGlobalTypes()
{
	UpdateProgressCB("GlobalTypes");

	bool bLoaded = false;	// used to make sure that the xml file was loaded correctly
	if (!CreateFXml())
	{
		return false;
	}

	// load the new FXml variable with the GlobalTypes.xml file
	bLoaded = LoadCivXml(m_pFXml, "GlobalTypes.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for GlobalTypes.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}

	// if the load succeeded we will continue
	if (bLoaded)
	{
		// if the xml is successfully validated
		if (Validate())
		{
			SetGlobalStringArray(&GC.getAnimationOperatorTypes(), "Civ4Types/AnimationOperatorTypes/AnimationOperatorType", &GC.getNumAnimationOperatorTypes());
			int iEnumVal = NUM_FUNC_TYPES;
			SetGlobalStringArray(&GC.getFunctionTypes(), "Civ4Types/FunctionTypes/FunctionType", &iEnumVal, true);
			SetGlobalStringArray(&GC.getFlavorTypes(), "Civ4Types/FlavorTypes/FlavorType", &GC.getNumFlavorTypes());
			SetGlobalStringArray(&GC.getArtStyleTypes(), "Civ4Types/ArtStyleTypes/ArtStyleType", &GC.getNumArtStyleTypes());
			SetGlobalStringArray(&GC.getCitySizeTypes(), "Civ4Types/CitySizeTypes/CitySizeType", &GC.getNumCitySizeTypes());
			iEnumVal = NUM_CONTACT_TYPES;
			SetGlobalStringArray(&GC.getContactTypes(), "Civ4Types/ContactTypes/ContactType", &iEnumVal, true);
			iEnumVal = NUM_DIPLOMACYPOWER_TYPES;
			SetGlobalStringArray(&GC.getDiplomacyPowerTypes(), "Civ4Types/DiplomacyPowerTypes/DiplomacyPowerType", &iEnumVal, true);
			iEnumVal = NUM_AUTOMATE_TYPES;
			SetGlobalStringArray(&GC.getAutomateTypes(), "Civ4Types/AutomateTypes/AutomateType", &iEnumVal, true);
			iEnumVal = NUM_DIRECTION_TYPES;
			SetGlobalStringArray(&GC.getDirectionTypes(), "Civ4Types/DirectionTypes/DirectionType", &iEnumVal, true);
			SetGlobalStringArray(&GC.getFootstepAudioTypes(), "Civ4Types/FootstepAudioTypes/FootstepAudioType", &GC.getNumFootstepAudioTypes());

			gDLL->getXMLIFace()->SetToParent(m_pFXml);
			gDLL->getXMLIFace()->SetToParent(m_pFXml);
			SetVariableListTagPair(&GC.getFootstepAudioTags(), "FootstepAudioTags", GC.getFootstepAudioTypes(), GC.getNumFootstepAudioTypes(), "");
		}
	}

	// delete the pointer to the FXml variable
	DestroyFXml();

	return true;
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetDiplomacyCommentTypes()
//
//  PURPOSE :   Creates a full list of Diplomacy Comments
//				
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetDiplomacyCommentTypes(CvString** ppszString, int* iNumVals)
{
	FAssertMsg(false, "should never get here");
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetupGlobalLandscapeInfos()
//
//  PURPOSE :   Initialize the appropriate variables located in globals.cpp/h with the values in
//				Terrain\Civ4TerrainSettings.xml
//
//------------------------------------------------------------------------------------------------------
bool CvXMLLoadUtility::SetupGlobalLandscapeInfo()
{
	bool bLoaded = false;	// used to make sure that the xml file was loaded correctly
	if (!CreateFXml())
	{
		return false;
	}

	// load the new FXml variable with the CIV4TerrainSettings.xml file
	bLoaded = LoadCivXml(m_pFXml, "Terrain/CIV4TerrainSettings.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4TerrainSettings.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}

	if (bLoaded)
	{
		// if the xml is successfully validated
		if (Validate())
		{
			SetGlobalClassInfo(&GC.getLandscapeInfo(), "Civ4TerrainSettings/LandscapeInfos/LandscapeInfo", &GC.getNumLandscapeInfos());
		}
	}
	// delete the pointer to the FXml variable
	DestroyFXml();
	return true;
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetGlobalArtDefines()
//
//  PURPOSE :   Initialize the appropriate variables located in globals.cpp/h with the values in
//				Civ4ArtDefines.xml
//
//------------------------------------------------------------------------------------------------------
bool CvXMLLoadUtility::SetGlobalArtDefines()
{
		INIT_ART_DEFINE_XML_LOAD(&ARTFILEMGR.getInterfaceArtInfo(), "Art/CIV4ArtDefines_Interface.xml", "Civ4ArtDefines/InterfaceArtInfos/InterfaceArtInfo", ARTFILEMGR.getNumInterfaceArtInfos())	
		INIT_ART_DEFINE_XML_LOAD(&ARTFILEMGR.getMovieArtInfo(), "Art/CIV4ArtDefines_Movie.xml", "Civ4ArtDefines/MovieArtInfos/MovieArtInfo", ARTFILEMGR.getNumMovieArtInfos())	
		INIT_ART_DEFINE_XML_LOAD(&ARTFILEMGR.getMiscArtInfo(), "Art/CIV4ArtDefines_Misc.xml", "Civ4ArtDefines/MiscArtInfos/MiscArtInfo",  ARTFILEMGR.getNumMiscArtInfos())	
		INIT_ART_DEFINE_XML_LOAD(&ARTFILEMGR.getUnitArtInfo(), "Art/CIV4ArtDefines_Unit.xml", "Civ4ArtDefines/UnitArtInfos/UnitArtInfo",  ARTFILEMGR.getNumUnitArtInfos())	
		INIT_ART_DEFINE_XML_LOAD(&ARTFILEMGR.getBuildingArtInfo(), "Art/CIV4ArtDefines_Building.xml", "Civ4ArtDefines/BuildingArtInfos/BuildingArtInfo",  ARTFILEMGR.getNumBuildingArtInfos())	
		INIT_ART_DEFINE_XML_LOAD(&ARTFILEMGR.getCivilizationArtInfo(), "Art/CIV4ArtDefines_Civilization.xml", "Civ4ArtDefines/CivilizationArtInfos/CivilizationArtInfo",  ARTFILEMGR.getNumCivilizationArtInfos())	
		INIT_ART_DEFINE_XML_LOAD(&ARTFILEMGR.getLeaderheadArtInfo(), "Art/CIV4ArtDefines_Leaderhead.xml", "Civ4ArtDefines/LeaderheadArtInfos/LeaderheadArtInfo",  ARTFILEMGR.getNumLeaderheadArtInfos())	
		INIT_ART_DEFINE_XML_LOAD(&ARTFILEMGR.getBonusArtInfo(), "Art/CIV4ArtDefines_Bonus.xml", "Civ4ArtDefines/BonusArtInfos/BonusArtInfo", ARTFILEMGR.getNumBonusArtInfos())	
		INIT_ART_DEFINE_XML_LOAD(&ARTFILEMGR.getImprovementArtInfo(), "Art/CIV4ArtDefines_Improvement.xml", "Civ4ArtDefines/ImprovementArtInfos/ImprovementArtInfo", ARTFILEMGR.getNumImprovementArtInfos())	
		INIT_ART_DEFINE_XML_LOAD(&ARTFILEMGR.getTerrainArtInfo(), "Art/CIV4ArtDefines_Terrain.xml", "Civ4ArtDefines/TerrainArtInfos/TerrainArtInfo", ARTFILEMGR.getNumTerrainArtInfos())	
		INIT_ART_DEFINE_XML_LOAD(&ARTFILEMGR.getFeatureArtInfo(), "Art/CIV4ArtDefines_Feature.xml", "Civ4ArtDefines/FeatureArtInfos/FeatureArtInfo", ARTFILEMGR.getNumFeatureArtInfos())	
	return true;
}


template <class T>
bool CvXMLLoadUtility::SetGlobalArtDefineInfo(T **ppArtDefneInfos, char* szXMLFileName, char* szTagName, int& iNumVals)
{
	if (!CreateFXml())
		return false;

	bool bLoaded = LoadCivXml(m_pFXml, szXMLFileName); 
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for Art/CIV4ArtDefines.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}

	if (bLoaded && Validate()) 
	{ 
		INIT_XML_GLOBAL_LOAD(szTagName, &*ppArtDefneInfos, &iNumVals)	
	}
	DestroyFXml();
	return bLoaded;
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetGlobalText()
//
//  PURPOSE :   Handles all Global Text Infos 
//
//------------------------------------------------------------------------------------------------------
bool CvXMLLoadUtility::LoadGlobalText()
{
	CvCacheObject* cache = gDLL->createGlobalTextCacheObject("GlobalText.dat");	// cache file name
	if (!gDLL->cacheRead(cache))
	{
		bool bLoaded = false;

		if (!CreateFXml())
		{
			return false;
		}

		//
		// load all files in the xml text directory
		//
		std::vector<CvString> files;
		gDLL->enumerateFiles(files, "xml\\text\\*.xml");
		int i;
		for(i=0;i<(int)files.size();i++)
		{
			bLoaded = LoadCivXml(m_pFXml, files[i]);	// Load the XML
			if (!bLoaded)
			{
				char	szMessage[1024];
				sprintf( szMessage, "LoadXML call failed for %s. \n Current XML file is: %s", files[i].c_str(), GC.getCurrentXMLFile().GetCString());
				gDLL->MessageBox(szMessage, "XML Load Error");
			}
			if (bLoaded)
			{
				// if the xml is successfully validated
				if (Validate())
				{
					SetGameText("Civ4GameText", "Civ4GameText/TEXT");
				}
			}
		}

		DestroyFXml();

		// write global text info to cache
		bool bOk = gDLL->cacheWrite(cache);
		if (!bLoaded)
		{
			char	szMessage[1024];
			sprintf( szMessage, "Failed writing to Global Text cache. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
			gDLL->MessageBox(szMessage, "XML Caching Error");
		}
		if (bOk)
		{
			logMsg("Wrote GlobalText to cache");
		}
	}	// didn't read from cache
	else
	{
		logMsg("Read GlobalText from cache");
	}

	gDLL->destroyCache(cache);

	return true;
}

bool CvXMLLoadUtility::LoadBasicInfos()
{
	if (!CreateFXml())
	{
		return false;
	}

	bool bLoaded = LoadCivXml(m_pFXml, "BasicInfos/CIV4BasicInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for BasicInfos/CIV4BasicInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}

	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getConceptInfo(), "Civ4BasicInfos/ConceptInfos/ConceptInfo", &GC.getNumConceptInfos());
		SetGlobalClassInfo(&GC.getCityTabInfo(), "Civ4BasicInfos/CityTabInfos/CityTabInfo", &GC.getNumCityTabInfos());
		SetGlobalClassInfo(&GC.getCalendarInfo(), "Civ4BasicInfos/CalendarInfos/CalendarInfo", &GC.getNumCalendarInfos());
		SetGlobalClassInfo(&GC.getSeasonInfo(), "Civ4BasicInfos/SeasonInfos/SeasonInfo", &GC.getNumSeasonInfos());
		SetGlobalClassInfo(&GC.getMonthInfo(), "Civ4BasicInfos/MonthInfos/MonthInfo", &GC.getNumMonthInfos());
		SetGlobalClassInfo(&GC.getDenialInfo(), "Civ4BasicInfos/DenialInfos/DenialInfo", &GC.getNumDenialInfos());
		SetGlobalClassInfo(&GC.getInvisibleInfo(), "Civ4BasicInfos/InvisibleInfos/InvisibleInfo", &GC.getNumInvisibleInfos());
		SetGlobalClassInfo(&GC.getUnitCombatInfo(), "Civ4BasicInfos/UnitCombatInfos/UnitCombatInfo", &GC.getNumUnitCombatInfos());

		int iEnumVal = NUM_DOMAIN_TYPES;
		SetGlobalClassInfo(&GC.getDomainInfo(), "Civ4BasicInfos/DomainInfos/DomainInfo", &iEnumVal, true);

		iEnumVal = NUM_UNITAI_TYPES;
		SetGlobalClassInfo(&GC.getUnitAIInfo(), "Civ4BasicInfos/UnitAIInfos/UnitAIInfo", &iEnumVal, true);

		iEnumVal = NUM_ATTITUDE_TYPES;
		SetGlobalClassInfo(&GC.getAttitudeInfo(), "Civ4BasicInfos/AttitudeInfos/AttitudeInfo", &iEnumVal, true);

		iEnumVal = NUM_MEMORY_TYPES;
		SetGlobalClassInfo(&GC.getMemoryInfo(), "Civ4BasicInfos/MemoryInfos/MemoryInfo", &iEnumVal, true);
	}

	DestroyFXml();
	return bLoaded;
}

//
// Globals which must be loaded before the main menus.
// Don't put anything in here unless it has to be loaded before the main menus,
// instead try to load things in LoadPostMenuGlobals()
//
bool CvXMLLoadUtility::LoadPreMenuGlobals()
{
	if (!CreateFXml())
	{
		return false;
	}

	int i;
	CvString* pszDefaultBuildings=NULL;		// this will hold the default buildings for the building classes
	CvString *pszDefaultUnits = NULL;		// this will hold the default units for the unit classes
	CvCacheObject* cache;
	bool bLoaded;

	bLoaded = LoadCivXml(m_pFXml, "GameInfo/CIV4GameSpeedInfo.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4GameSpeedInfo.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getGameSpeedInfo(), "Civ4GameSpeedInfo/GameSpeedInfos/GameSpeedInfo", &GC.getNumGameSpeedInfos());
	}

	bLoaded = LoadCivXml(m_pFXml, "GameInfo/CIV4TurnTimerInfo.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4TurnTimerInfo.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getTurnTimerInfo(), "Civ4TurnTimerInfo/TurnTimerInfos/TurnTimerInfo", &GC.getNumTurnTimerInfos());
	}

	// load the new FXml variable with the CIV4WorldInfo.xml file
	bLoaded = LoadCivXml(m_pFXml, "GameInfo/CIV4WorldInfo.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4WorldInfo.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getWorldInfo(), "Civ4WorldInfo/WorldInfos/WorldInfo", &GC.getNumWorldInfos());
	}

	// load the new FXml variable with the CIV4ClimateInfo.xml file
	bLoaded = LoadCivXml(m_pFXml, "GameInfo/CIV4ClimateInfo.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4ClimateInfo.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getClimateInfo(), "Civ4ClimateInfo/ClimateInfos/ClimateInfo", &GC.getNumClimateInfos());
	}

	// load the new FXml variable with the CIV4SeaLevelInfo.xml file
	bLoaded = LoadCivXml(m_pFXml, "GameInfo/CIV4SeaLevelInfo.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4SeaLevelInfo.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getSeaLevelInfo(), "Civ4SeaLevelInfo/SeaLevelInfos/SeaLevelInfo", &GC.getNumSeaLevelInfos());
	}

	// load the new FXml variable with the CIV4AdvisorInfos.xml file
	bLoaded = LoadCivXml(m_pFXml, "Interface/CIV4AdvisorInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4AdvisorInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getAdvisorInfo(), "Civ4AdvisorInfos/AdvisorInfos/AdvisorInfo", &GC.getNumAdvisorInfos());
	}

	bLoaded = LoadCivXml(m_pFXml, "Terrain/CIV4TerrainInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4TerrainInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getTerrainInfo(), "Civ4TerrainInfos/TerrainInfos/TerrainInfo", &GC.getNumTerrainInfos());
	}

	bLoaded = LoadCivXml(m_pFXml, "GameInfo/CIV4EraInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4EraInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getEraInfo(), "Civ4EraInfos/EraInfos/EraInfo", &GC.getNumEraInfos());
	}

	bLoaded = LoadCivXml(m_pFXml, "Units/CIV4UnitClassInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4UnitClassInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalUnitClassInfo(&GC.getUnitClassInfo(), "Civ4UnitClassInfos/UnitClassInfos/UnitClassInfo", &GC.getNumUnitClassInfos(), &pszDefaultUnits);
	}

	bLoaded = LoadCivXml(m_pFXml, "GameInfo/CIV4SpecialistInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4SpecialistInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getSpecialistInfo(), "Civ4SpecialistInfos/SpecialistInfos/SpecialistInfo", &GC.getNumSpecialistInfos());
	}

	/////////////////////////////////
	// use disk cache if possible.
	// if no cache or cache is older than xml file, use xml file like normal, else read from cache
	//
	cache = gDLL->createTechInfoCacheObject("CIV4TechInfos.dat");	// cache file name
	if (!gDLL->cacheRead(cache, "xml\\Technologies\\CIV4TechInfos.xml"))			// src data file name
	{
		// load normally
		bLoaded = LoadCivXml(m_pFXml, "Technologies/CIV4TechInfos.xml");
		if (!bLoaded)
		{
			char	szMessage[1024];
			sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4TechInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
			gDLL->MessageBox(szMessage, "XML Load Error");
		}

		if (bLoaded)
		{
			// if the xml is successfully validated
			if (Validate())
			{
				SetGlobalTechInfo(&GC.getTechInfo(), "Civ4TechInfos/TechInfos/TechInfo", &GC.getNumTechInfos());

				// write tech info to cache
				bool bOk = gDLL->cacheWrite(cache);
				if (!bOk)
				{
					char	szMessage[1024];
					sprintf( szMessage, "Failed writing to TechInfos cache. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Caching Error");
				}
				if (bOk)
				{
					logMsg("Wrote Civ4TechInfos to cache");
				}
			}
		}
	}
	else
	{
		logMsg("Read Civ4TechInfos from cache");
	}
	gDLL->destroyCache(cache);

	bLoaded = LoadCivXml(m_pFXml, "Terrain/CIV4FeatureInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4FeatureInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getFeatureInfo(), "Civ4FeatureInfos/FeatureInfos/FeatureInfo", &GC.getNumFeatureInfos());
	}

	/////////////////////////////////

	cache = gDLL->createPromotionInfoCacheObject("CIV4PromotionInfos.dat");		// cache file name
	if (!gDLL->cacheRead(cache, "xml\\Units\\CIV4PromotionInfos.xml"))	// src data file name
	{
		// load the new FXml variable with the CIV4Promotions xml file
		bLoaded = LoadCivXml(m_pFXml, "Units/CIV4PromotionInfos.xml");
		if (!bLoaded)
		{
			char	szMessage[1024];
			sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4PromotionInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
			gDLL->MessageBox(szMessage, "XML Load Error");
		}
		if (bLoaded)
		{
			if (Validate())
			{
				SetGlobalPromotionInfo(&GC.getPromotionInfo(), "Civ4PromotionInfos/PromotionInfos/PromotionInfo", &GC.getNumPromotionInfos());

				// write civ info to cache
				bool bOk = gDLL->cacheWrite(cache);
				if (!bLoaded)
				{
					char	szMessage[1024];
					sprintf( szMessage, "Failed writing to PromotionInfos cache. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Caching Error");
				}
				if (bOk)
				{
					logMsg("Wrote Civ4PromotionInfos to cache");
				}
			}
		}
	}
	else
	{
		logMsg("Read CIV4PromotionInfos from cache");
		bLoaded	= true;
	}
	gDLL->destroyCache(cache);
//////////////

	// load the new FXml variable with the CIV4TraitInfos.xml file
	bLoaded = LoadCivXml(m_pFXml, "Civilizations/CIV4TraitInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4TraitInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getTraitInfo(), "Civ4TraitInfos/TraitInfos/TraitInfo", &GC.getNumTraitInfos());
	}

	bLoaded = LoadCivXml(m_pFXml, "GameInfo/CIV4GoodyInfo.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4GoodyInfo.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getGoodyInfo(), "Civ4GoodyInfo/GoodyInfos/GoodyInfo", &GC.getNumGoodyInfos());
	}

	/////////////////////////////////

	cache = gDLL->createHandicapInfoCacheObject("CIV4HandicapInfos.dat");		// cache file name
	if (!gDLL->cacheRead(cache, "xml\\GameInfo\\CIV4HandicapInfos.xml"))	// src data file name
	{
		// load the new FXml variable with the CIV4Handicaps xml file
		bLoaded = LoadCivXml(m_pFXml, "GameInfo/CIV4HandicapInfo.xml");
		if (!bLoaded)
		{
			char	szMessage[1024];
			sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4HandicapInfo.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
			gDLL->MessageBox(szMessage, "XML Load Error");
		}
		if (bLoaded)
		{
			if (Validate())
			{
				SetGlobalClassInfo(&GC.getHandicapInfo(), "Civ4HandicapInfo/HandicapInfos/HandicapInfo", &GC.getNumHandicapInfos());

				// write civ info to cache
				bool bOk = gDLL->cacheWrite(cache);
				if (!bLoaded)
				{
					char	szMessage[1024];
					sprintf( szMessage, "Failed writing to HandicapInfos cache. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Caching Error");
				}
				if (bOk)
				{
					logMsg("Wrote Civ4HandicapInfos to cache");
				}
			}
		}
	}
	else
	{
		logMsg("Read CIV4HandicapInfos from cache");
		bLoaded	= true;
	}
	gDLL->destroyCache(cache);

	//////////////

	// load the new FXml variable with the CIV4CursorInfo.xml file
	bLoaded = LoadCivXml(m_pFXml, "GameInfo/CIV4CursorInfo.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4CursorInfo.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded)
	{
		if (Validate())
		{
			SetGlobalClassInfo(&GC.getCursorInfo(), "Civ4CursorInfo/CursorInfos/CursorInfo", &GC.getNumCursorInfos());

			// add types to global var system
			int i, iVal;
			for(i=0;i<GC.getNumCursorInfos();i++)
			{
				CvString szType = GC.getCursorInfo((CursorTypes)i).getType();
				if (GC.getDefinesVarSystem()->GetValue(szType, iVal)==true)
				{
					char	szMessage[1024];
					sprintf( szMessage, "cursor type already set? \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Error");
				}
				GC.getDefinesVarSystem()->SetValue(szType, i);
			}
		}
	}
/*	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4Infos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}*/

	bLoaded = LoadCivXml(m_pFXml, "GameInfo/CIV4CivicOptionInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4CivicOptionInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getCivicOptionInfo(), "Civ4CivicOptionInfos/CivicOptionInfos/CivicOptionInfo", &GC.getNumCivicOptionInfos());
	}

	bLoaded = LoadCivXml(m_pFXml, "GameInfo/CIV4UpkeepInfo.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4UpkeepInfo.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getUpkeepInfo(), "Civ4UpkeepInfo/UpkeepInfos/UpkeepInfo", &GC.getNumUpkeepInfos());
	}

	bLoaded = LoadCivXml(m_pFXml, "GameInfo/CIV4HurryInfo.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4HurryInfo.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getHurryInfo(), "Civ4HurryInfo/HurryInfos/HurryInfo", &GC.getNumHurryInfos());
	}

	bLoaded = LoadCivXml(m_pFXml, "Buildings/CIV4SpecialBuildingInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for Buildings/CIV4SpecialBuildingInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getSpecialBuildingInfo(), "Civ4SpecialBuildingInfos/SpecialBuildingInfos/SpecialBuildingInfo", &GC.getNumSpecialBuildingInfos());
	}

	bLoaded = LoadCivXml(m_pFXml, "GameInfo/CIV4ReligionInfo.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4ReligionInfo.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getReligionInfo(), "Civ4ReligionInfo/ReligionInfos/ReligionInfo", &GC.getNumReligionInfos());
	}

	bLoaded = LoadCivXml(m_pFXml, "GameInfo/CIV4CultureLevelInfo.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4CultureLevelInfo.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getCultureLevelInfo(), "Civ4CultureLevelInfo/CultureLevelInfos/CultureLevelInfo", &GC.getNumCultureLevelInfos());
	}

	bLoaded = LoadCivXml(m_pFXml, "Terrain/CIV4BonusClassInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for Terrain/CIV4BonusClassInfos.xml.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getBonusClassInfo(), "Civ4BonusClassInfos/BonusClassInfos/BonusClassInfo", &GC.getNumBonusClassInfos());
	}

	bLoaded = LoadCivXml(m_pFXml, "GameInfo/CIV4VictoryInfo.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4VictoryInfo.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getVictoryInfo(), "Civ4VictoryInfo/VictoryInfos/VictoryInfo", &GC.getNumVictoryInfos());
	}

	/////////////////////////////////

	cache = gDLL->createBonusInfoCacheObject("CIV4BonusInfos.dat");		// cache file name
	if (!gDLL->cacheRead(cache, "xml\\Terrain\\CIV4BonusInfos.xml"))	// src data file name
	{
		// load the new FXml variable with the CIV4Bonuss xml file
		bLoaded = LoadCivXml(m_pFXml, "Terrain/CIV4BonusInfos.xml");
		if (!bLoaded)
		{
			char	szMessage[1024];
			sprintf( szMessage, "LoadXML call failed for Terrain/CIV4BonusInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
			gDLL->MessageBox(szMessage, "XML Load Error");
		}
		if (bLoaded)
		{
			if (Validate())
			{
				SetGlobalClassInfo(&GC.getBonusInfo(), "Civ4BonusInfos/BonusInfos/BonusInfo", &GC.getNumBonusInfos());

				// write civ info to cache
				bool bOk = gDLL->cacheWrite(cache);
				if (!bLoaded)
				{
					char	szMessage[1024];
					sprintf( szMessage, "Failed writing to BonusInfos cache. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Caching Error");
				}
				if (bOk)
				{
					logMsg("Wrote Civ4BonusInfos to cache");
				}
			}
		}
	}
	else
	{
		logMsg("Read CIV4BonusInfos from cache");
		bLoaded	= true;
	}
	gDLL->destroyCache(cache);

	//////////////

	bLoaded = LoadCivXml(m_pFXml, "Buildings/CIV4BuildingClassInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for Buildings/CIV4BuildingClassInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalBuildingClassInfo(&GC.getBuildingClassInfo(), "Civ4BuildingClassInfos/BuildingClassInfos/BuildingClassInfo", &GC.getNumBuildingClassInfos(), &pszDefaultBuildings);
	}

	//////////////////////////////////
	// use disk cache if possible
	// -if no cache or cache is older than xml file, use xml file like normal, else read from cache

	cache = gDLL->createBuildingInfoCacheObject("CIV4BuildingInfos.dat");				// cache file name
	if (!gDLL->cacheRead(cache, "xml\\Buildings\\CIV4BuildingInfos.xml"))		// source data file name	
	{
		// load normally
		bLoaded = LoadCivXml(m_pFXml, "Buildings/CIV4BuildingInfos.xml");
		if (!bLoaded)
		{
			char	szMessage[1024];
			sprintf( szMessage, "LoadXML call failed for Buildings/CIV4BuildingInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
			gDLL->MessageBox(szMessage, "XML Load Error");
		}

		if (bLoaded)
		{
			if (Validate())
			{
				SetGlobalBuildingInfo(&GC.getBuildingInfo(), "Civ4BuildingInfos/BuildingInfos/BuildingInfo", &GC.getNumBuildingInfos());	// SLOW
				// write building info to cache				
				bool bOk = gDLL->cacheWrite(cache);
				if (!bOk)
				{
					char	szMessage[1024];
					sprintf( szMessage, "Failed writing to building cache. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Caching Error");
				}
				if (bOk)
				{
					logMsg("Wrote CIV4BuildingInfos to cache");
				}
			}
			else
			{
				bLoaded = false;
			}
		}
	}
	else
	{
		bLoaded = true;
		logMsg("Read CIV4BuildingInfos from cache");
	}
	gDLL->destroyCache(cache);

	if (bLoaded)
	{
		for (i=0;i<GC.getNumBuildingClassInfos();i++)
		{
			GC.getBuildingClassInfo((BuildingClassTypes) i).setDefaultBuildingIndex(FindInInfoClass(pszDefaultBuildings[i], GC.getBuildingInfo(), sizeof(GC.getBuildingInfo((BuildingTypes) 0)), GC.getNumBuildingInfos()));
		}
	}

	bLoaded = LoadCivXml(m_pFXml, "Units/CIV4SpecialUnitInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for Units/CIV4SpecialUnitInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getSpecialUnitInfo(), "Civ4SpecialUnitInfos/SpecialUnitInfos/SpecialUnitInfo", &GC.getNumSpecialUnitInfos());
	}

	// load the new FXml variable with the CIV4ProjectInfo.xml file
	bLoaded = LoadCivXml(m_pFXml, "GameInfo/CIV4ProjectInfo.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for Units/CIV4ProjectInfo.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalProjectInfo(&GC.getProjectInfo(), "Civ4ProjectInfo/ProjectInfos/ProjectInfo", &GC.getNumProjectInfos());
	}


	bLoaded = LoadCivXml(m_pFXml, "Misc/CIV4RouteInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for Misc/CIV4RouteInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalRouteInfo(&GC.getRouteInfo(), "Civ4RouteInfos/RouteInfos/RouteInfo", &GC.getNumRouteInfos(), &GC.getRouteZBias());
	}

	/////////////////////////////////

	cache = gDLL->createImprovementInfoCacheObject("CIV4ImprovementInfos.dat");		// cache file name
	if (!gDLL->cacheRead(cache, "xml\\Terrain\\CIV4ImprovementInfos.xml"))	// src data file name
	{
		// load the new FXml variable with the CIV4Improvements xml file
		bLoaded = LoadCivXml(m_pFXml, "Terrain/CIV4ImprovementInfos.xml");
		if (!bLoaded)
		{
			char	szMessage[1024];
			sprintf( szMessage, "LoadXML call failed for Terrain/CIV4ImprovementInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
			gDLL->MessageBox(szMessage, "XML Load Error");
		}
		if (bLoaded)
		{
			if (Validate())
			{
				SetGlobalClassInfo(&GC.getImprovementInfo(), "Civ4ImprovementInfos/ImprovementInfos/ImprovementInfo", &GC.getNumImprovementInfos());

				// now that we are done reading in the improvement infos we need to check if any of them have pillages/upgrades
				// loop through all of the improvment infos
				int i,j;
				for (i=0;i<GC.getNumImprovementInfos();i++)
				{
					GC.getImprovementInfo((ImprovementTypes) i).setImprovementPillage(NO_IMPROVEMENT);
					GC.getImprovementInfo((ImprovementTypes) i).setImprovementUpgrade(NO_IMPROVEMENT);

					// loop through the improvement infos again comparing the i'th improvement infos' szImprovementPillage/Upgrade
					// variable with the j'th improvementinfos' szDescription, which is really the improvement's type
					for (j=0;j<GC.getNumImprovementInfos();j++)
					{
						// if the improvement pillage string matches the type string
						if (_tcscmp(GC.getImprovementInfo((ImprovementTypes) i).getImprovementPillageString(),GC.getImprovementInfo((ImprovementTypes) j).getType()) == 0)
						{
							// set the improvement pillage integer value to the inner loop variable j
							GC.getImprovementInfo((ImprovementTypes) i).setImprovementPillage(j);
						}
						// if the improvement upgrade string matches the type string
						if (_tcscmp(GC.getImprovementInfo((ImprovementTypes) i).getImprovementUpgradeString(),GC.getImprovementInfo((ImprovementTypes) j).getType()) == 0)
						{
							// set the improvement upgrade integer value to the inner loop variable j
							GC.getImprovementInfo((ImprovementTypes) i).setImprovementUpgrade(j);
						}
					}
				}

				// write civ info to cache
				bool bOk = gDLL->cacheWrite(cache);
				if (!bLoaded)
				{
					char	szMessage[1024];
					sprintf( szMessage, "Failed writing to ImprovementInfos cache. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Caching Error");
				}
				if (bOk)
				{
					logMsg("Wrote Civ4ImprovementInfos to cache");
				}
			}
		}
	}
	else
	{
		logMsg("Read CIV4ImprovementInfos from cache");
		bLoaded	= true;
	}
	gDLL->destroyCache(cache);

	/////////////////////////////////

	cache = gDLL->createCivicInfoCacheObject("CIV4CivicInfos.dat");		// cache file name
	if (!gDLL->cacheRead(cache, "xml\\GameInfo\\CIV4CivicInfos.xml"))	// src data file name
	{
		// load the new FXml variable with the CIV4Civics xml file
		bLoaded = LoadCivXml(m_pFXml, "GameInfo/CIV4CivicInfos.xml");
		if (!bLoaded)
		{
			char	szMessage[1024];
			sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4CivicInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
			gDLL->MessageBox(szMessage, "XML Load Error");
		}
		if (bLoaded)
		{
			if (Validate())
			{
				SetGlobalClassInfo(&GC.getCivicInfo(), "Civ4CivicInfos/CivicInfos/CivicInfo", &GC.getNumCivicInfos());

				// write civ info to cache
				bool bOk = gDLL->cacheWrite(cache);
				if (!bLoaded)
				{
					char	szMessage[1024];
					sprintf( szMessage, "Failed writing to CivicInfos cache. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Caching Error");
				}
				if (bOk)
				{
					logMsg("Wrote Civ4CivicInfos to cache");
				}
			}
		}
	}
	else
	{
		logMsg("Read CIV4CivicInfos from cache");
		bLoaded	= true;
	}
	gDLL->destroyCache(cache);

	//////////////

	//
	// use disk cache if possible
	//
	// -if no cache or cache is older than xml file, use xml file like normal, else read from cache
	//
	cache = gDLL->createLeaderHeadInfoCacheObject("CIV4LeaderHeadInfos.dat");		// cache file name
	if (!gDLL->cacheRead(cache, "xml\\Civilizations\\CIV4LeaderHeadInfos.xml"))	// src data file name
	{
		// load normally
		bLoaded = LoadCivXml(m_pFXml, "Civilizations/CIV4LeaderHeadInfos.xml");
		if (!bLoaded)
		{
			char	szMessage[1024];
			sprintf( szMessage, "LoadXML call failed for Civilizations/CIV4LeaderHeadInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
			gDLL->MessageBox(szMessage, "XML Load Error");
		}

		if (bLoaded)
		{
			SetGlobalClassInfo(&GC.getLeaderHeadInfo(), "Civ4LeaderHeadInfos/LeaderHeadInfos/LeaderHeadInfo", &GC.getNumLeaderHeadInfos());

			// write unit info to cache
			bool bOk = gDLL->cacheWrite(cache);
			if (!bOk)
			{
				char	szMessage[1024];
				sprintf( szMessage, "Failed writing to LeaderHead cache. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
				gDLL->MessageBox(szMessage, "XML Caching Error");
			}
			if (bOk)
			{
				logMsg("Wrote Civ4LeaderHeadInfos to cache");
			}
		}
	}
	else
	{
		logMsg("Read CIV4LeaderHeadInfos from cache");
		bLoaded	= true;
	}
	gDLL->destroyCache(cache);

	bLoaded = LoadCivXml(m_pFXml, "Interface/CIV4ColorVals.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for Interface/CIV4ColorVals.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getColorInfo(), "Civ4ColorVals/ColorVals/ColorVal", &GC.getNumColorInfos());
	}

	bLoaded = LoadCivXml(m_pFXml, "Interface/CIV4PlayerColorInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for Interface/CIV4PlayerColorInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getPlayerColorInfo(), "Civ4PlayerColorInfos/PlayerColorInfos/PlayerColorInfo", &GC.getNumPlayerColorInfos());
	}

	// Load the effect infos
	bLoaded = LoadCivXml(m_pFXml, "Misc/CIV4EffectInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for Misc/CIV4EffectInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getEffectInfo(), "Civ4EffectInfos/EffectInfos/EffectInfo", &GC.getNumEffectInfos());
	}

	bLoaded = LoadCivXml( m_pFXml, "Units/CIV4AnimationInfos.xml" );
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for Units/CIV4AnimationInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if ( bLoaded )
	{
		if (Validate())
		{
			SetGlobalClassInfo( &GC.getAnimationCategoryInfo(), "Civ4AnimationInfos/AnimationCategories/AnimationCategory", &GC.getNumAnimationCategoryInfos() );
			SetGlobalClassInfo( &GC.getAnimationPathInfo(), "Civ4AnimationInfos/AnimationPaths/AnimationPath", &GC.getNumAnimationPathInfos() );
			SetGlobalClassInfo( &GC.getEntityEventInfo(), "Civ4AnimationInfos/EntityEventInfos/EntityEventInfo", &GC.getNumEntityEventInfos() );
		}
	}

	bLoaded = LoadCivXml(m_pFXml, "Units/CIV4BuildInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for Units/CIV4BuildInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getBuildInfo(), "Civ4BuildInfos/BuildInfos/BuildInfo", &GC.getNumBuildInfos());
	}

	//
	// use disk cache if possible
	//
	// -if no cache or cache is older than xml file, use xml file like normal, else read from cache
	//
	cache = gDLL->createUnitInfoCacheObject("CIV4UnitInfos.dat");		// cache file name
	if (!gDLL->cacheRead(cache, "xml\\Units\\CIV4UnitInfos.xml"))	// src data file name
	{
		// load normally
		bLoaded = LoadCivXml(m_pFXml, "Units/CIV4UnitInfos.xml");
		if (!bLoaded)
		{
			char	szMessage[1024];
			sprintf( szMessage, "LoadXML call failed for Units/CIV4UnitInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
			gDLL->MessageBox(szMessage, "XML Load Error");
		}

		if (bLoaded)
		{
			SetGlobalClassInfo(&GC.getUnitInfo(), "Civ4UnitInfos/UnitInfos/UnitInfo", &GC.getNumUnitInfos());

			// write unit info to cache
			bool bOk = gDLL->cacheWrite(cache);
			if (!bOk)
			{
				char	szMessage[1024];
				sprintf( szMessage, "Failed writing to Unit cache. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
				gDLL->MessageBox(szMessage, "XML Caching Error");
			}
			if (bOk)
			{
				logMsg("Wrote Civ4UnitInfos to cache");
			}
		}
	}
	else
	{
		logMsg("Read CIV4UnitInfos from cache");
		bLoaded	= true;
	}
	gDLL->destroyCache(cache);

	if (bLoaded)
	{
		for (i=0;i<GC.getNumUnitClassInfos();i++)
		{
			GC.getUnitClassInfo((UnitClassTypes) i).setDefaultUnitIndex(FindInInfoClass(pszDefaultUnits[i], GC.getUnitInfo(), sizeof(GC.getUnitInfo((UnitTypes) 0)), GC.getNumUnitInfos()));
		}
	}

	//////////////////////////////////

	cache = gDLL->createCivilizationInfoCacheObject("CIV4CivilizationInfos.dat");		// cache file name
	if (!gDLL->cacheRead(cache, "xml\\Civilizations\\CIV4CivilizationInfos.xml"))	// src data file name
	{
		// load the new FXml variable with the CIV4Buildings.xml file
		bLoaded = LoadCivXml(m_pFXml, "Civilizations/CIV4CivilizationInfos.xml");
		if (!bLoaded)
		{
			char	szMessage[1024];
			sprintf( szMessage, "LoadXML call failed for Civilizations/CIV4CivilizationInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
			gDLL->MessageBox(szMessage, "XML Load Error");
		}
		if (bLoaded)
		{
			if (Validate())
			{
				SetGlobalClassInfo(&GC.getCivilizationInfo(), "Civ4CivilizationInfos/CivilizationInfos/CivilizationInfo", &GC.getNumCivilizationInfos());

				// write civ info to cache
				bool bOk = gDLL->cacheWrite(cache);
				if (!bOk)
				{
					char	szMessage[1024];
					sprintf( szMessage, "Failed writing to Civilization cache. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Caching Error");
				}
				if (bOk)
				{
					logMsg("Wrote Civ4CivilizationInfos to cache");
				}
			}
		}
	}
	else
	{
		logMsg("Read CIV4CivilizationInfos from cache");
		bLoaded	= true;
	}
	gDLL->destroyCache(cache);

	// Check Playables
	for (i=0;i<GC.getNumCivilizationInfos();i++)
	{
		// if the civilization is playable we will increment the playable var
		if (GC.getCivilizationInfo((CivilizationTypes) i).isPlayable())
		{
			GC.getNumPlayableCivilizationInfos() += 1;
		}

		// if the civilization is playable by AI increments num playable
		if (GC.getCivilizationInfo((CivilizationTypes) i).isAIPlayable())
		{
			GC.getNumAIPlayableCivilizationInfos() += 1;
		}
	}

	bLoaded = LoadCivXml(m_pFXml, "Terrain/CIV4YieldInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for Terrain/CIV4YieldInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		int iEnumVal = NUM_YIELD_TYPES;
		SetGlobalClassInfo(&GC.getYieldInfo(), "Civ4YieldInfos/YieldInfos/YieldInfo", &iEnumVal, true);
	}

	bLoaded = LoadCivXml(m_pFXml, "GameInfo/CIV4CommerceInfo.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4CommerceInfo.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		int iEnumVal = NUM_COMMERCE_TYPES;
		SetGlobalClassInfo(&GC.getCommerceInfo(), "Civ4CommerceInfo/CommerceInfos/CommerceInfo", &iEnumVal, true);
	}

	bLoaded = LoadCivXml(m_pFXml, "GameInfo/CIV4Hints.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4Hints.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getHints(), "Civ4Hints/HintInfos/HintInfo", &GC.getNumHints());
	}

	bLoaded = LoadCivXml(m_pFXml, "GameInfo/CIV4GameOptionInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4GameOptionInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		int iEnumVal = NUM_GAMEOPTION_TYPES;
		SetGlobalClassInfo(&GC.getGameOptionInfo(), "Civ4GameOptionInfos/GameOptionInfos/GameOptionInfo", &iEnumVal, true);
	}

	bLoaded = LoadCivXml(m_pFXml, "GameInfo/CIV4MPOptionInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4MPOptionInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		int iEnumVal = NUM_MPOPTION_TYPES;
		SetGlobalClassInfo(&GC.getMPOptionInfo(), "Civ4MPOptionInfos/MPOptionInfos/MPOptionInfo", &iEnumVal, true);
	}

	bLoaded = LoadCivXml(m_pFXml, "GameInfo/CIV4ForceControlInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4ForceControlInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		int iEnumVal = NUM_FORCECONTROL_TYPES;
		SetGlobalClassInfo(&GC.getForceControlInfo(), "Civ4ForceControlInfos/ForceControlInfos/ForceControlInfo", &iEnumVal, true);
	}

	bLoaded = LoadCivXml(m_pFXml, "Interface/CIV4SlideShowInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for Interface/CIV4SlideShowInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getSlideShowInfo(), "Civ4SlideShowInfos/SlideShowInfos/SlideShowInfo", &GC.getNumSlideShowInfos());
		SetGlobalClassInfo(&GC.getSlideShowRandomInfo(), "Civ4SlideShowInfos/SlideShowRandomInfos/SlideShowRandomInfo", &GC.getNumSlideShowRandomInfos());
	}

	bLoaded = LoadCivXml(m_pFXml, "Interface/CIV4WorldPickerInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for Interface/CIV4WorldPickerInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getWorldPickerInfo(), "Civ4WorldPickerInfos/WorldPickerInfos/WorldPickerInfo", &GC.getNumWorldPickerInfos());
	}

	UpdateProgressCB("GlobalOther");

	DestroyFXml();

	SAFE_DELETE_ARRAY(pszDefaultUnits);
	SAFE_DELETE_ARRAY(pszDefaultBuildings);

	return true;
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   LoadPostMenuGlobals()
//
//  PURPOSE :   loads global xml data which isn't needed for the main menus
//		this data is loaded as a secodn stage, when the game is launched
//
//------------------------------------------------------------------------------------------------------
bool CvXMLLoadUtility::LoadPostMenuGlobals()
{
	PROFILE_FUNC();
	if (!CreateFXml())
	{
		return false;
	}

	CvString *pszDefaultUnits = NULL;		// this will hold the default units for the unit classes
	bool bLoaded = false;	// used to make sure that the xml file was loaded correctly
	CvCacheObject* cache;

	//throne room disabled
	/*UpdateProgressCB("GlobalOther");

	// load the new FXml variable with the CIV4ThroneRoomInfos.xml file
	bLoaded = LoadCivXml(m_pFXml, "Interface/CIV4ThroneRoomInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for Interface/CIV4ThroneRoomInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getThroneRoomCamera(), "Civ4ThroneRoomInfos/ThroneRoomInfos/ThroneRoomCamera", &GC.getNumThroneRoomCameras());
		SetGlobalClassInfo(&GC.getThroneRoomInfo(), "Civ4ThroneRoomInfos/ThroneRoomInfos/ThroneRoomInfo", &GC.getNumThroneRoomInfos());
	}

	UpdateProgressCB("GlobalOther");

	// load the new FXml variable with the CIV4ThroneRoomStyleInfos.xml file
	bLoaded = LoadCivXml(m_pFXml, "Interface/CIV4ThroneRoomStyleInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for Interface/CIV4ThroneRoomStyleInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getThroneRoomStyleInfo(), "Civ4ThroneRoomStyleInfos/ThroneRoomStyleInfos/ThroneRoomStyleInfo", &GC.getNumThroneRoomStyleInfos());
	}
	*/

	UpdateProgressCB("GlobalOther");

	// load the new FXml variable with the CIV4RouteModelInfos.xml file
	bLoaded = LoadCivXml(m_pFXml, "Art/CIV4RouteModelInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for Art/CIV4RouteModelInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getRouteModelInfo(), "Civ4RouteModelInfos/RouteModelInfos/RouteModelInfo", &GC.getNumRouteModelInfos());
	}

	UpdateProgressCB("GlobalOther");

	// load the new FXml variable with the CIV4RiverInfos.xml file
	bLoaded = LoadCivXml(m_pFXml, "Misc/CIV4RiverInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for Misc/CIV4RiverInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalRiverInfo(&GC.getRiverInfo(), "Civ4RiverInfos/RiverInfos/RiverInfo", &GC.getNumRiverInfos(), &GC.getRiverZBias());
	}

	// load the new FXml variable with the CIV4Buildings.xml file
	bLoaded = LoadCivXml(m_pFXml, "Art/CIV4RiverModelInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for Art/CIV4RiverModelInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getRiverModelInfo(), "Civ4RiverModelInfos/RiverModelInfos/RiverModelInfo", &GC.getNumRiverModelInfos());
	}

	// load the new FXml variable with the CIV4Buildings.xml file
	bLoaded = LoadCivXml(m_pFXml, "Misc/CIV4WaterPlaneInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for Misc/CIV4WaterPlaneInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getWaterPlaneInfo(), "Civ4WaterPlaneInfos/WaterPlaneInfos/WaterPlaneInfo", &GC.getNumWaterPlaneInfos());
	}

	UpdateProgressCB("GlobalOther");
	// load the new FXml variable with the CIV4ProcessInfo.xml file
	bLoaded = LoadCivXml(m_pFXml, "GameInfo/CIV4ProcessInfo.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4ProcessInfo.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getProcessInfo(), "Civ4ProcessInfo/ProcessInfos/ProcessInfo", &GC.getNumProcessInfos());
	}

	UpdateProgressCB("GlobalOther");

	// load the new FXml variable with the CIV4Buildings.xml file
	bLoaded = LoadCivXml(m_pFXml, "GameInfo/CIV4EmphasizeInfo.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4EmphasizeInfo.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getEmphasizeInfo(), "Civ4EmphasizeInfo/EmphasizeInfos/EmphasizeInfo", &GC.getNumEmphasizeInfos());
	}

	UpdateProgressCB("GlobalOther");

	bLoaded = LoadCivXml(m_pFXml, "Units/CIV4MissionInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for Units/CIV4MissionInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		int iEnumVal = NUM_MISSION_TYPES;
		SetGlobalClassInfo(&GC.getMissionInfo(), "Civ4MissionInfos/MissionInfos/MissionInfo", &iEnumVal, true);
	}

	bLoaded = LoadCivXml(m_pFXml, "Units/CIV4ControlInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for Units/CIV4ControlInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		int iEnumVal = NUM_CONTROL_TYPES;
		SetGlobalClassInfo(&GC.getControlInfo(), "Civ4ControlInfos/ControlInfos/ControlInfo", &iEnumVal, true);
	}

	bLoaded = LoadCivXml(m_pFXml, "Units/CIV4CommandInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for Units/CIV4CommandInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded)
	{
		if (Validate())
		{
			int iEnumVal = NUM_COMMAND_TYPES;
			SetGlobalClassInfo(&GC.getCommandInfo(), "Civ4CommandInfos/CommandInfos/CommandInfo", &iEnumVal, true);
		}
	}

	bLoaded = LoadCivXml(m_pFXml, "Units/CIV4AutomateInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for Units/CIV4AutomateInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getAutomateInfo(), "Civ4AutomateInfos/AutomateInfos/AutomateInfo", &GC.getNumAutomateInfos());
	}

	UpdateProgressCB("GlobalOther");

	// load the new FXml variable with the CIV4VoteInfo.xml file
	bLoaded = LoadCivXml(m_pFXml, "GameInfo/CIV4VoteInfo.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for Units/CIV4VoteInfo.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getVoteInfo(), "Civ4VoteInfo/VoteInfos/VoteInfo", &GC.getNumVoteInfos());
	}

	UpdateProgressCB("GlobalOther");

	bLoaded = LoadCivXml(m_pFXml, "Interface/CIV4CameraInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for Interface/CIV4CameraInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getCameraInfo(), "Civ4CameraInfos/CameraInfos/CameraInfo", &GC.getNumCameraInfos());
	}

	bLoaded = LoadCivXml(m_pFXml, "Interface/CIV4InterfaceModeInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for Interface/CIV4InterfaceModeInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded)
	{
		if (Validate())
		{
			int iEnumVal = NUM_INTERFACEMODE_TYPES;
			SetGlobalClassInfo(&GC.getInterfaceModeInfo(), "Civ4InterfaceModeInfos/InterfaceModeInfos/InterfaceModeInfo", &iEnumVal, true);
		}
	}

	if (bLoaded && Validate())
	{
		SetGlobalActionInfo(&GC.getActionInfo(), &GC.getNumActionInfos());
	}


	// Load the formation info
	bLoaded = LoadCivXml(m_pFXml, "Units/CIV4FormationInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for Units/CIV4FormationInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded)
	{
		if (Validate())
		{
			SetGlobalClassInfo(&GC.getUnitFormationInfo(), "UnitFormations/UnitFormation", &GC.getNumUnitFormationInfos());
		}
	}

	// Load the attachable infos
	bLoaded = LoadCivXml(m_pFXml, "Misc/CIV4AttachableInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for Misc/CIV4AttachableInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getAttachableInfo(), "Civ4AttachableInfos/AttachableInfos/AttachableInfo", &GC.getNumAttachableInfos());
	}

	cache = gDLL->createDiplomacyInfoCacheObject("CIV4DiplomacyInfos.dat");		// cache file name
	if (!gDLL->cacheRead(cache, "xml\\GameInfo\\CIV4DiplomacyInfos.xml"))	// src data file name
	{
		// load the new FXml variable with the CIV4Buildings.xml file
		bLoaded = LoadCivXml(m_pFXml, "GameInfo/CIV4DiplomacyInfos.xml");
		if (!bLoaded)
		{
			char	szMessage[1024];
			sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4DiplomacyInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
			gDLL->MessageBox(szMessage, "XML Load Error");
		}
		if (bLoaded)
		{
			if (Validate())
			{
				SetGlobalClassInfo(&GC.getDiplomacyInfo(), "Civ4DiplomacyInfos/DiplomacyInfos/DiplomacyInfo", &GC.getNumDiplomacyInfos());

				// write civ info to cache
				bool bOk = gDLL->cacheWrite(cache);
				if (!bOk)
				{
					char	szMessage[1024];
					sprintf( szMessage, "Failed writing to Diplomacy cache. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Caching Error");
				}
				if (bOk)
				{
					logMsg("Wrote Civ4DiplomacyInfos to cache");
				}
			}
		}
	}
	else
	{
		logMsg("Read CIV4DiplomacyInfos from cache");
		bLoaded	= true;
	}
	gDLL->destroyCache(cache);

	// load the new FXml variable with the Civ4QuestInfos.xml file
	bLoaded = LoadCivXml(m_pFXml, "Misc/Civ4QuestInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for Misc/Civ4QuestInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getQuestInfo(), "Civ4QuestInfos/QuestInfo", &GC.getNumQuestInfos());
	}

	// load the new FXml variable with the Civ4TutorialInfos.xml file
	bLoaded = LoadCivXml(m_pFXml, "Misc/Civ4TutorialInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for Misc/Civ4TutorialInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}
	if (bLoaded && Validate())
	{
		SetGlobalClassInfo(&GC.getTutorialInfo(), "Civ4TutorialInfos/TutorialInfo", &GC.getNumTutorialInfos());
	}

	SAFE_DELETE_ARRAY(pszDefaultUnits);

	DestroyFXml();
	return true;
}


//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetGlobalStringArray(TCHAR (**ppszString)[256], char* szTagName, int* iNumVals)
//
//  PURPOSE :   takes the szTagName parameter and if it finds it in the m_pFXml member variable
//				then it loads the ppszString parameter with the string values under it and the
//				iNumVals with the total number of tags with the szTagName in the xml file
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetGlobalStringArray(CvString **ppszString, char* szTagName, int* iNumVals, bool bUseEnum)
{
	PROFILE_FUNC();
	logMsg("SetGlobalStringArray %s\n", szTagName);

	int i=0;					//loop counter
	CvString *pszString;	// hold the local pointer to the newly allocated string memory
	pszString = NULL;			// null out the local string pointer so that it can be checked at the
	// end of the function in an FAssert

	// if we locate the szTagName, the current node is set to the first instance of the tag name in the xml file
	if (gDLL->getXMLIFace()->LocateNode(m_pFXml,szTagName))
	{
		if (!bUseEnum)
		{
			// get the total number of times this tag appears in the xml
			*iNumVals = gDLL->getXMLIFace()->NumOfElementsByTagName(m_pFXml,szTagName);
		}
		// initialize the memory based on the total number of tags in the xml and the 256 character length we selected
		*ppszString = new CvString[*iNumVals];
		// set the local pointer to the memory just allocated
		pszString = *ppszString;

		// loop through each of the tags
		for (i=0;i<*iNumVals;i++)
		{
			// get the string value at the current node
			GetXmlVal(pszString[i]);
			GC.setTypesEnum(pszString[i], i);

			// if can't set the current node to a sibling node we will break out of the for loop
			// otherwise we will keep looping
			if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
			{
				break;
			}
		}
	}

	// if the local string pointer is null then we weren't able to find the szTagName in the xml
	// so we will FAssert to let whoever know it
	if (!pszString)
	{
		char	szMessage[1024];
		sprintf( szMessage, "Error locating tag node in SetGlobalStringArray function \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetGlobalRouteInfo(CvRouteInfo **ppRouteInfos, char* szTagName, int* iNumVals)
//
//  PURPOSE :   Takes the szTagName parameter and if it exists in the xml it loads the ppRouteInfos
//				with the value under it and sets the value of the iNumVals parameter to the total number
//				of occurances of the szTagName tag in the xml.   
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetGlobalRouteInfo(CvRouteInfo **ppRouteInfos, char* szTagName, int* iNumVals, float *fZbias)
{
	PROFILE_FUNC();
	logMsg("SetGlobalRouteInfo %s\n", szTagName);
	int i=0;				//loop counter
	CvRouteInfo* pRouteInfos;	// local pointer to the route info memory

	// null the local pointer so it can be checked at the end of the function
	pRouteInfos = NULL;

	// if we successfully locate the szTagName node
	if (gDLL->getXMLIFace()->LocateNode(m_pFXml,szTagName))
	{
		// get the number of times the szTagName tag appears in the xml file
		*iNumVals = gDLL->getXMLIFace()->NumOfElementsByTagName(m_pFXml,szTagName);
		// allocate memory for the route infos based on the number of tags in the xml
		*ppRouteInfos = new CvRouteInfo[*iNumVals];
		// set the local pointer to the memory we just allocated
		pRouteInfos = *ppRouteInfos;
		// set the current xml node to it's parent node since we are on the first child of the root tag group
		gDLL->getXMLIFace()->SetToParent(m_pFXml);
		GetChildXmlValByName(fZbias,  "fZBias");
		gDLL->getXMLIFace()->LocateNode(m_pFXml,szTagName);

		// loop through each tag
		for (i=0;i<*iNumVals;i++)
		{
			SkipToNextVal();	// skip to the next non-comment node

			if (!pRouteInfos[i].read(this))
				break;

			GC.setInfoTypeFromString(pRouteInfos[i].getType(), i);	// add type to global info type hash map
			if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
			{
				break;
			}
		}
	}

	// if we didn't find the tag name in the xml then we never set the local pointer to the 
	// newly allocated memory and there for we will FAssert to let people know this most
	// interesting fact
	if(!pRouteInfos)
	{
		char	szMessage[1024];
		sprintf( szMessage, "Error finding tag node in SetGlobalRouteInfo function \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
}


//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetGlobalRiverInfo(CvRiverInfo **ppRiverInfos, char* szTagName, int* iNumVals)
//
//  PURPOSE :   Takes the szTagName parameter and if it exists in the xml it loads the ppRiverInfos
//				with the value under it and sets the value of the iNumVals parameter to the total number
//				of occurances of the szTagName tag in the xml.   
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetGlobalRiverInfo(CvRiverInfo **ppRiverInfos, char* szTagName, int* iNumVals,float *fZBias)
{
	PROFILE_FUNC();
	logMsg("SetGlobalRiverInfo %s\n", szTagName);
	int i=0;							// loop counter
	CvRiverInfo* pRiverInfos;	// local pointer to the River info memory

	// null the local pointer so it can be checked at the end of the function
	pRiverInfos = NULL;

	// if we successfully locate the szTagName node
	if (gDLL->getXMLIFace()->LocateNode(m_pFXml,szTagName))
	{
		// get the number of times the szTagName tag appears in the xml file
		*iNumVals = gDLL->getXMLIFace()->NumOfElementsByTagName(m_pFXml,szTagName);
		// allocate memory for the River infos based on the number of tags in the xml
		*ppRiverInfos = new CvRiverInfo[*iNumVals];
		// set the local pointer to the memory we just allocated
		pRiverInfos = *ppRiverInfos;
		// set the current xml node to it's parent node since we are on the first child of the root tag group
		gDLL->getXMLIFace()->SetToParent(m_pFXml);
		GetChildXmlValByName(fZBias,  "fZBias");
		gDLL->getXMLIFace()->LocateNode(m_pFXml,szTagName);

		// loop through each tag
		for (i=0;i<*iNumVals;i++)
		{
			SkipToNextVal();	// skip to the next non-comment node

			if (!pRiverInfos[i].read(this))
				break;

			GC.setInfoTypeFromString(pRiverInfos[i].getType(), i);	// add type to global info type hash map
			if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
			{
				break;
			}
		}
	}

	// if we didn't find the tag name in the xml then we never set the local pointer to the 
	// newly allocated memory and there for we will FAssert to let people know this most
	// interesting fact
	if(!pRiverInfos)
	{
		char	szMessage[1024];
		sprintf( szMessage, "Error finding tag node in SetGlobalRiverInfo function \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetGlobalBuildingClassInfo(CvBuildingClassInfo** ppBuildingClassInfo, char* szTagName, int* iNumVals, TCHAR (**ppszDefualtBuildingList)[256])
//
//  PURPOSE :   Takes the szTagName parameter and if it exists in the xml it loads the ppBuildingInfo
//				with the value under it and sets the value of the iNumVals parameter to the total number
//				of occurances of the szTagName tag in the xml.   
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetGlobalBuildingClassInfo(CvBuildingClassInfo** ppBuildingClassInfo, char* szTagName, int* iNumVals, CvString** ppszDefualtBuildingList)
{
	PROFILE_FUNC();
	logMsg("SetGlobalBuildingClassInfo %s\n", szTagName);
	int i=0;								//loop counter
	CvBuildingClassInfo* pBuildingClassInfo;	// local pointer to the building info memory
	CvString *pszDefaultBuildingList;

	// null out the local pointer so that it can be checked at the end of this function
	pBuildingClassInfo = NULL;

	// if we successfully locate the szTagName node
	if (gDLL->getXMLIFace()->LocateNode(m_pFXml,szTagName))
	{
		// get the number of times the szTagName tag appears in the xml file
		*iNumVals = gDLL->getXMLIFace()->NumOfElementsByTagName(m_pFXml,szTagName);
		// allocate memory for the building info based on the number above
		*ppBuildingClassInfo = new CvBuildingClassInfo[*iNumVals];
		// set the local pointer to the memory we just allocated
		pBuildingClassInfo = *ppBuildingClassInfo;
		// allocate memory for the default building list
		*ppszDefualtBuildingList = new CvString[*iNumVals];
		// set the local pointer to the memory we just allocated
		pszDefaultBuildingList = *ppszDefualtBuildingList;

		gDLL->getXMLIFace()->SetToParent(m_pFXml);
		gDLL->getXMLIFace()->SetToChild(m_pFXml);

		// loop through each tag
		for (i=0;i<*iNumVals;i++)
		{			
			SkipToNextVal();	// skip to the next non-comment node

			GetChildXmlValByName(pszDefaultBuildingList[i], "DefaultBuilding");
			if (!pBuildingClassInfo[i].read(this))
				break;
			GC.setInfoTypeFromString(pBuildingClassInfo[i].getType(), i);	// add type to global info type hash map
			if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
			{
				break;
			}
		}
	}

	// if we didn't find the tag name in the xml then we never set the local pointer to the 
	// newly allocated memory and there for we will FAssert to let people know this most
	// interesting fact
	if(!pBuildingClassInfo)
	{
		char	szMessage[1024];
		sprintf( szMessage, "Error finding tag node in SetGlobalBuildingClassInfo function \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetGlobalBuildingInfo(CvBuildingInfo** ppBuildingInfo, char* szTagName, int* iNumVals)
//
//  PURPOSE :   Takes the szTagName parameter and if it exists in the xml it loads the ppBuildingInfo
//				with the value under it and sets the value of the iNumVals parameter to the total number
//				of occurances of the szTagName tag in the xml.   
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetGlobalBuildingInfo(CvBuildingInfo** ppBuildingInfo, char* szTagName, int* iNumVals)
{
	PROFILE_FUNC();
	logMsg("SetGlobalBuildingInfo %s\n", szTagName);
	int i=0;						//loop counter
	CvBuildingInfo* pBuildingInfo;	// local pointer to the building info memory

	// null out the local pointer so that it can be checked at the end of this function
	pBuildingInfo = NULL;

	// if we successfully locate the szTagName node
	if (gDLL->getXMLIFace()->LocateNode(m_pFXml,szTagName))
	{
		// get the number of times the szTagName tag appears in the xml file
		*iNumVals = gDLL->getXMLIFace()->NumOfElementsByTagName(m_pFXml,szTagName);
		// allocate memory for the building info based on the number above
		*ppBuildingInfo = new CvBuildingInfo[*iNumVals];
		// set the local pointer to the memory we just allocated
		pBuildingInfo = *ppBuildingInfo;

		gDLL->getXMLIFace()->SetToParent(m_pFXml);
		gDLL->getXMLIFace()->SetToChild(m_pFXml);

		// loop through each tag
		for (i=0;i<*iNumVals;i++)
		{
			SkipToNextVal();	// skip to the next non-comment node

			if (!pBuildingInfo[i].read(this))
				break;
			GC.setInfoTypeFromString(pBuildingInfo[i].getType(), i);	// add type to global info type hash map
			if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
			{
				break;
			}
		}
	}

	// if we didn't find the tag name in the xml then we never set the local pointer to the 
	// newly allocated memory and there for we will FAssert to let people know this most
	// interesting fact
	if(!pBuildingInfo)
	{
		char	szMessage[1024];
		sprintf( szMessage, "Error finding tag node in SetGlobalBuildingInfo function \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetGlobalProjectInfo(CvProjectInfo** ppProjectInfo, char* szTagName, int* iNumVals)
//
//  PURPOSE :   Takes the szTagName parameter and if it exists in the xml it loads the ppProjectInfo
//				with the value under it and sets the value of the iNumVals parameter to the total number
//				of occurances of the szTagName tag in the xml.   
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetGlobalProjectInfo(CvProjectInfo** ppProjectInfo, char* szTagName, int* iNumVals)
{
	PROFILE_FUNC();
	logMsg("SetGlobalProjectInfo %s\n", szTagName);
	int i=0;						//loop counter
	CvString *paszAnyonePrereqProject; // holds the anyone prereq project for checking after all the projects are in
	CvProjectInfo* pProjectInfo;	// local pointer to the project info memory

	// null out the local pointer so that it can be checked at the end of this function
	pProjectInfo = NULL;

	// if we successfully locate the szTagName node
	if (gDLL->getXMLIFace()->LocateNode(m_pFXml,szTagName))
	{
		// get the number of times the szTagName tag appears in the xml file
		*iNumVals = gDLL->getXMLIFace()->NumOfElementsByTagName(m_pFXml,szTagName);
		// allocate memory for the project info based on the number above
		*ppProjectInfo = new CvProjectInfo[*iNumVals];
		// set the local pointer to the memory we just allocated
		pProjectInfo = *ppProjectInfo;

		// we also need to allocate memory for the anyone prereq projects
		paszAnyonePrereqProject = new CvString[*iNumVals];

		gDLL->getXMLIFace()->SetToParent(m_pFXml);
		gDLL->getXMLIFace()->SetToChild(m_pFXml);

		// loop through each tag
		for (i=0;i<*iNumVals;i++)
		{
			SkipToNextVal();	// skip to the next non-comment node

			GetChildXmlValByName(paszAnyonePrereqProject[i], "AnyonePrereqProject");
			if (isEmpty(paszAnyonePrereqProject[i]))
			{
				paszAnyonePrereqProject[i] = "NONE";
			}
			if (!pProjectInfo[i].read(this))
				break;
			GC.setInfoTypeFromString(pProjectInfo[i].getType(), i);	// add type to global info type hash map
			if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
			{
				break;
			}
		}

		// call the find in list function to return either -1 if no value is found
		// or the index in the list the match is found at
		for (i=0;i<*iNumVals;i++)
		{
			pProjectInfo[i].setAnyoneProjectPrereq(FindInInfoClass(paszAnyonePrereqProject[i], pProjectInfo, sizeof(pProjectInfo[0]), *iNumVals));
		}

		SAFE_DELETE_ARRAY(paszAnyonePrereqProject);
	}
	else
	{
		*iNumVals = 0;
		*ppProjectInfo = new CvProjectInfo[*iNumVals];
	}

	// if we didn't find the tag name in the xml then we never set the local pointer to the 
	// newly allocated memory and there for we will FAssert to let people know this most
	// interesting fact
	if (NULL == *ppProjectInfo)
	{
		char	szMessage[1024];
		sprintf( szMessage, "Error finding tag node in SetGlobalProjectInfo function \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetGlobalUnitClassInfo(CvUnitClassInfo** ppUnitClassInfo, char* szTagName, int* iNumVals, TCHAR (**ppszDefualtUnitList)[256])
//
//  PURPOSE :   Takes the szTagName parameter and if it exists in the xml it loads the ppUnitClassInfo
//				with the value under it and sets the value of the iNumVals parameter to the total number
//				of occurances of the szTagName tag in the xml.   
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetGlobalUnitClassInfo(CvUnitClassInfo** ppUnitClassInfo, char* szTagName, int* iNumVals, 
											  CvString **ppszDefualtUnitList)
{
	PROFILE_FUNC();
	logMsg("SetGlobalUnitClassInfo %s\n", szTagName);
	int i=0;				//loop counter
	CvUnitClassInfo* pUnitClassInfo;	// local pointer to the unit class info memory
	CvString *pszDefaultUnitList;

	// null out the local pointer so it can be checked at the end of the function
	pUnitClassInfo = NULL;

	// if we successfully locate the szTagName node
	if (gDLL->getXMLIFace()->LocateNode(m_pFXml,szTagName))
	{
		// get the number of times the szTagName tag appears in the xml file
		*iNumVals = gDLL->getXMLIFace()->NumOfElementsByTagName(m_pFXml,szTagName);
		// allocate memory for the unit class info based on the number above
		*ppUnitClassInfo = new CvUnitClassInfo[*iNumVals];
		// set the local pointer to the memory we just allocated
		pUnitClassInfo = *ppUnitClassInfo;
		// allocate memory for the default unit list
		*ppszDefualtUnitList = new CvString[*iNumVals];
		// set the local pointer to the memory we just allocated
		pszDefaultUnitList = *ppszDefualtUnitList;

		gDLL->getXMLIFace()->SetToParent(m_pFXml);
		gDLL->getXMLIFace()->SetToChild(m_pFXml);

		// loop through each tag
		for (i=0;i<*iNumVals;i++)
		{
			SkipToNextVal();	// skip to the next non-comment node

			GetChildXmlValByName(pszDefaultUnitList[i], "DefaultUnit");
			if (!pUnitClassInfo[i].read(this))
				break;
			GC.setInfoTypeFromString(pUnitClassInfo[i].getType(), i);	// add type to global info type hash map
			if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
			{
				break;
			}
		}
	}

	// if we didn't find the tag name in the xml then we never set the local pointer to the 
	// newly allocated memory and there for we will FAssert to let people know this most
	// interesting fact
	if(!pUnitClassInfo)
	{
		char	szMessage[1024];
		sprintf( szMessage, "Error finding tag node in SetGlobalUnitClassInfo function \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetGlobalActionInfo(CvActionInfo** ppActionInfo, int* iNumVals)
//
//  PURPOSE :   Takes the szTagName parameter and if it exists in the xml it loads the ppActionInfo
//				with the value under it and sets the value of the iNumVals parameter to the total number
//				of occurances of the szTagName tag in the xml.   
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetGlobalActionInfo(CvActionInfo** ppActionInfo, int* iNumVals)
{
	PROFILE_FUNC();
	logMsg("SetGlobalActionInfo\n");
	int i=0;					//loop counter
	int j=0;					//loop counter
	CvActionInfo* pActionInfo;	// local pointer to the action info memory

	// null out the local pointer so that it can be checked at the end of this function
	pActionInfo = NULL;

	*iNumVals = 0;
	if(!(NUM_INTERFACEMODE_TYPES > 0))
	{
		char	szMessage[1024];
		sprintf( szMessage, "NUM_INTERFACE_TYPES is not greater than zero in CvXMLLoadUtility::SetGlobalActionInfo \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	if(!(GC.getNumBuildInfos() > 0))
	{
		char	szMessage[1024];
		sprintf( szMessage, "GC.getNumBuildInfos() is not greater than zero in CvXMLLoadUtility::SetGlobalActionInfo \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	if(!(GC.getNumPromotionInfos() > 0))
	{
		char	szMessage[1024];
		sprintf( szMessage, "GC.getNumPromotionInfos() is not greater than zero in CvXMLLoadUtility::SetGlobalActionInfo \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	if(!(GC.getNumReligionInfos() > 0) )
	{
		char	szMessage[1024];
		sprintf( szMessage, "GC.getNumReligionInfos() is not greater than zero in CvXMLLoadUtility::SetGlobalActionInfo \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	if(!(GC.getNumUnitClassInfos() > 0) )
	{
		char	szMessage[1024];
		sprintf( szMessage, "GC.getNumUnitClassInfos() is not greater than zero in CvXMLLoadUtility::SetGlobalActionInfo \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	if(!(GC.getNumSpecialistInfos() > 0) )
	{
		char	szMessage[1024];
		sprintf( szMessage, "GC.getNumSpecialistInfos() is not greater than zero in CvXMLLoadUtility::SetGlobalActionInfo \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	if(!(GC.getNumBuildingInfos() > 0) )
	{
		char	szMessage[1024];
		sprintf( szMessage, "GC.getNumBuildingInfos() is not greater than zero in CvXMLLoadUtility::SetGlobalActionInfo \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	if(!(NUM_CONTROL_TYPES > 0) )
	{
		char	szMessage[1024];
		sprintf( szMessage, "NUM_CONTROL_TYPES is not greater than zero in CvXMLLoadUtility::SetGlobalActionInfo \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	if(!(GC.getNumAutomateInfos() > 0) )
	{
		char	szMessage[1024];
		sprintf( szMessage, "GC.getNumAutomateInfos() is not greater than zero in CvXMLLoadUtility::SetGlobalActionInfo \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	if(!(NUM_COMMAND_TYPES > 0) )
	{
		char	szMessage[1024];
		sprintf( szMessage, "NUM_COMMAND_TYPES is not greater than zero in CvXMLLoadUtility::SetGlobalActionInfo \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	if(!(NUM_MISSION_TYPES > 0) )
	{
		char	szMessage[1024];
		sprintf( szMessage, "NUM_MISSION_TYPES is not greater than zero in CvXMLLoadUtility::SetGlobalActionInfo \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	// allocate memory for the action info based on the number above
	*ppActionInfo = new CvActionInfo[	(*iNumVals) + 
		NUM_INTERFACEMODE_TYPES + 
		GC.getNumBuildInfos() + 
		GC.getNumPromotionInfos() + 
		GC.getNumReligionInfos() + 
		GC.getNumUnitInfos() + 
		GC.getNumSpecialistInfos() + 
		GC.getNumBuildingInfos() + 
		NUM_CONTROL_TYPES +
		NUM_COMMAND_TYPES +
		GC.getNumAutomateInfos() +
		NUM_MISSION_TYPES];
	// set the local pointer to the memory we just allocated
	pActionInfo = *ppActionInfo;

	int* piOrderedIndex=NULL;

	int iNumActionInfos =(*iNumVals) + 
		NUM_INTERFACEMODE_TYPES + 
		GC.getNumBuildInfos() + 
		GC.getNumPromotionInfos() + 
		GC.getNumReligionInfos() + 
		GC.getNumUnitInfos() + 
		GC.getNumSpecialistInfos() + 
		GC.getNumBuildingInfos() + 
		NUM_CONTROL_TYPES +
		NUM_COMMAND_TYPES +
		GC.getNumAutomateInfos() +
		NUM_MISSION_TYPES;
	int* piIndexList = new int[iNumActionInfos];
	int* piPriorityList = new int[iNumActionInfos];
	int* piActionInfoTypeList = new int[iNumActionInfos];

	int iTotalActionInfoCount=0;
	// loop through control info
	for (i=0;i<NUM_COMMAND_TYPES;i++)
	{
		piIndexList[iTotalActionInfoCount] = i;
		piPriorityList[iTotalActionInfoCount] = GC.getCommandInfo((CommandTypes)i).getOrderPriority();
		piActionInfoTypeList[iTotalActionInfoCount] = ACTIONSUBTYPE_COMMAND;
		iTotalActionInfoCount++;
	}

	for (i=0;i<NUM_INTERFACEMODE_TYPES;i++)
	{
		piIndexList[iTotalActionInfoCount] = i;
		piPriorityList[iTotalActionInfoCount] = GC.getInterfaceModeInfo((InterfaceModeTypes)i).getOrderPriority();
		piActionInfoTypeList[iTotalActionInfoCount] = ACTIONSUBTYPE_INTERFACEMODE;
		iTotalActionInfoCount++;
	}

	for (i=0;i<GC.getNumBuildInfos();i++)
	{
		piIndexList[iTotalActionInfoCount] = i;
		piPriorityList[iTotalActionInfoCount] = GC.getBuildInfo((BuildTypes)i).getOrderPriority();
		piActionInfoTypeList[iTotalActionInfoCount] = ACTIONSUBTYPE_BUILD;
		iTotalActionInfoCount++;
	}

	for (i=0;i<GC.getNumPromotionInfos();i++)
	{
		piIndexList[iTotalActionInfoCount] = i;
		piPriorityList[iTotalActionInfoCount] = GC.getPromotionInfo((PromotionTypes)i).getOrderPriority();
		piActionInfoTypeList[iTotalActionInfoCount] = ACTIONSUBTYPE_PROMOTION;
		iTotalActionInfoCount++;
	}

	for (i=0;i<GC.getNumUnitInfos();i++)
	{
		piIndexList[iTotalActionInfoCount] = i;
		piPriorityList[iTotalActionInfoCount] = GC.getUnitInfo((UnitTypes)i).getOrderPriority();
		piActionInfoTypeList[iTotalActionInfoCount] = ACTIONSUBTYPE_UNIT;
		iTotalActionInfoCount++;
	}

	for (i=0;i<GC.getNumReligionInfos();i++)
	{
		piIndexList[iTotalActionInfoCount] = i;
		piPriorityList[iTotalActionInfoCount] = GC.getReligionInfo((ReligionTypes)i).getOrderPriority();
		piActionInfoTypeList[iTotalActionInfoCount] = ACTIONSUBTYPE_RELIGION;
		iTotalActionInfoCount++;
	}

	for (i=0;i<GC.getNumSpecialistInfos();i++)
	{
		piIndexList[iTotalActionInfoCount] = i;
		piPriorityList[iTotalActionInfoCount] = GC.getSpecialistInfo((SpecialistTypes)i).getOrderPriority();
		piActionInfoTypeList[iTotalActionInfoCount] = ACTIONSUBTYPE_SPECIALIST;
		iTotalActionInfoCount++;
	}

	for (i=0;i<GC.getNumBuildingInfos();i++)
	{
		piIndexList[iTotalActionInfoCount] = i;
		piPriorityList[iTotalActionInfoCount] = GC.getBuildingInfo((BuildingTypes)i).getOrderPriority();
		piActionInfoTypeList[iTotalActionInfoCount] = ACTIONSUBTYPE_BUILDING;
		iTotalActionInfoCount++;
	}

	for (i=0;i<NUM_CONTROL_TYPES;i++)
	{
		piIndexList[iTotalActionInfoCount] = i;
		piPriorityList[iTotalActionInfoCount] = GC.getControlInfo((ControlTypes)i).getOrderPriority();
		piActionInfoTypeList[iTotalActionInfoCount] = ACTIONSUBTYPE_CONTROL;
		iTotalActionInfoCount++;
	}

	for (i=0;i<GC.getNumAutomateInfos();i++)
	{
		piIndexList[iTotalActionInfoCount] = i;
		piPriorityList[iTotalActionInfoCount] = GC.getAutomateInfo(i).getOrderPriority();
		piActionInfoTypeList[iTotalActionInfoCount] = ACTIONSUBTYPE_AUTOMATE;
		iTotalActionInfoCount++;
	}

	for (i=0;i<NUM_MISSION_TYPES;i++)
	{
		piIndexList[iTotalActionInfoCount] = i;
		piPriorityList[iTotalActionInfoCount] = GC.getMissionInfo((MissionTypes)i).getOrderPriority();
		piActionInfoTypeList[iTotalActionInfoCount] = ACTIONSUBTYPE_MISSION;
		iTotalActionInfoCount++;
	}

	SAFE_DELETE_ARRAY(piOrderedIndex);
	piOrderedIndex = new int[iNumActionInfos];

	orderHotkeyInfo(&piOrderedIndex, piPriorityList, iNumActionInfos);
	for (i=0;i<iNumActionInfos;i++)
	{
		pActionInfo[i + *iNumVals].setOriginalIndex(piIndexList[piOrderedIndex[i]]);
		pActionInfo[i + *iNumVals].setSubType((ActionSubTypes)piActionInfoTypeList[piOrderedIndex[i]]);
		if ((ActionSubTypes)piActionInfoTypeList[piOrderedIndex[i]] == ACTIONSUBTYPE_COMMAND)
		{
			GC.getCommandInfo((CommandTypes)piIndexList[piOrderedIndex[i]]).setActionInfoIndex(i);
		}
		else if ((ActionSubTypes)piActionInfoTypeList[piOrderedIndex[i]] == ACTIONSUBTYPE_INTERFACEMODE)
		{
			GC.getInterfaceModeInfo((InterfaceModeTypes)piIndexList[piOrderedIndex[i]]).setActionInfoIndex(i);
		}
		else if ((ActionSubTypes)piActionInfoTypeList[piOrderedIndex[i]] == ACTIONSUBTYPE_BUILD)
		{
			GC.getBuildInfo((BuildTypes)piIndexList[piOrderedIndex[i]]).setMissionType(FindInInfoClass("MISSION_BUILD", GC.getMissionInfo(), sizeof(GC.getMissionInfo((MissionTypes) 0)), NUM_MISSION_TYPES));
			GC.getBuildInfo((BuildTypes)piIndexList[piOrderedIndex[i]]).setActionInfoIndex(i);
		}
		else if ((ActionSubTypes)piActionInfoTypeList[piOrderedIndex[i]] == ACTIONSUBTYPE_PROMOTION)
		{
			GC.getPromotionInfo((PromotionTypes)piIndexList[piOrderedIndex[i]]).setCommandType(FindInInfoClass("COMMAND_PROMOTION", GC.getCommandInfo(), sizeof(GC.getCommandInfo((CommandTypes) 0)), NUM_COMMAND_TYPES));
			GC.getPromotionInfo((PromotionTypes)piIndexList[piOrderedIndex[i]]).setActionInfoIndex(i);
			GC.getPromotionInfo((PromotionTypes)piIndexList[piOrderedIndex[i]]).setHotKeyDescription(GC.getPromotionInfo((PromotionTypes)piIndexList[piOrderedIndex[i]]).getTextKeyWide(), GC.getCommandInfo((CommandTypes)(GC.getPromotionInfo((PromotionTypes)piIndexList[piOrderedIndex[i]]).getCommandType())).getTextKeyWide(), CreateHotKeyFromDescription(GC.getPromotionInfo((PromotionTypes)piIndexList[piOrderedIndex[i]]).getHotKey(), GC.getPromotionInfo((PromotionTypes)piIndexList[piOrderedIndex[i]]).isShiftDown(), GC.getPromotionInfo((PromotionTypes)piIndexList[piOrderedIndex[i]]).isAltDown(), GC.getPromotionInfo((PromotionTypes)piIndexList[piOrderedIndex[i]]).isCtrlDown()));
		}
		else if ((ActionSubTypes)piActionInfoTypeList[piOrderedIndex[i]] == ACTIONSUBTYPE_UNIT)
		{
			GC.getUnitInfo((UnitTypes)piIndexList[piOrderedIndex[i]]).setCommandType(FindInInfoClass("COMMAND_UPGRADE", GC.getCommandInfo(), sizeof(GC.getCommandInfo((CommandTypes) 0)), NUM_COMMAND_TYPES));
			GC.getUnitInfo((UnitTypes)piIndexList[piOrderedIndex[i]]).setActionInfoIndex(i);
			GC.getUnitInfo((UnitTypes)piIndexList[piOrderedIndex[i]]).setHotKeyDescription(GC.getUnitInfo((UnitTypes)piIndexList[piOrderedIndex[i]]).getTextKeyWide(), GC.getCommandInfo((CommandTypes)(GC.getUnitInfo((UnitTypes)piIndexList[piOrderedIndex[i]]).getCommandType())).getTextKeyWide(), CreateHotKeyFromDescription(GC.getUnitInfo((UnitTypes)piIndexList[piOrderedIndex[i]]).getHotKey(), GC.getUnitInfo((UnitTypes)piIndexList[piOrderedIndex[i]]).isShiftDown(), GC.getUnitInfo((UnitTypes)piIndexList[piOrderedIndex[i]]).isAltDown(), GC.getUnitInfo((UnitTypes)piIndexList[piOrderedIndex[i]]).isCtrlDown()));
		}
		else if ((ActionSubTypes)piActionInfoTypeList[piOrderedIndex[i]] == ACTIONSUBTYPE_RELIGION)
		{
			GC.getReligionInfo((ReligionTypes)piIndexList[piOrderedIndex[i]]).setMissionType(FindInInfoClass("MISSION_SPREAD", GC.getMissionInfo(), sizeof(GC.getMissionInfo((MissionTypes) 0)), NUM_MISSION_TYPES));
			GC.getReligionInfo((ReligionTypes)piIndexList[piOrderedIndex[i]]).setActionInfoIndex(i);
			GC.getReligionInfo((ReligionTypes)piIndexList[piOrderedIndex[i]]).setHotKeyDescription(GC.getReligionInfo((ReligionTypes)piIndexList[piOrderedIndex[i]]).getTextKeyWide(), GC.getMissionInfo((MissionTypes)(GC.getReligionInfo((ReligionTypes)piIndexList[piOrderedIndex[i]]).getMissionType())).getTextKeyWide(), CreateHotKeyFromDescription(GC.getReligionInfo((ReligionTypes)piIndexList[piOrderedIndex[i]]).getHotKey(), GC.getReligionInfo((ReligionTypes)piIndexList[piOrderedIndex[i]]).isShiftDown(), GC.getReligionInfo((ReligionTypes)piIndexList[piOrderedIndex[i]]).isAltDown(), GC.getReligionInfo((ReligionTypes)piIndexList[piOrderedIndex[i]]).isCtrlDown()));
		}
		else if ((ActionSubTypes)piActionInfoTypeList[piOrderedIndex[i]] == ACTIONSUBTYPE_SPECIALIST)
		{
			GC.getSpecialistInfo((SpecialistTypes)piIndexList[piOrderedIndex[i]]).setMissionType(FindInInfoClass("MISSION_JOIN", GC.getMissionInfo(), sizeof(GC.getMissionInfo((MissionTypes) 0)), NUM_MISSION_TYPES));
			GC.getSpecialistInfo((SpecialistTypes)piIndexList[piOrderedIndex[i]]).setActionInfoIndex(i);
			GC.getSpecialistInfo((SpecialistTypes)piIndexList[piOrderedIndex[i]]).setHotKeyDescription(GC.getSpecialistInfo((SpecialistTypes)piIndexList[piOrderedIndex[i]]).getTextKeyWide(), GC.getMissionInfo((MissionTypes)(GC.getSpecialistInfo((SpecialistTypes)piIndexList[piOrderedIndex[i]]).getMissionType())).getTextKeyWide(), CreateHotKeyFromDescription(GC.getSpecialistInfo((SpecialistTypes)piIndexList[piOrderedIndex[i]]).getHotKey(), GC.getSpecialistInfo((SpecialistTypes)piIndexList[piOrderedIndex[i]]).isShiftDown(), GC.getSpecialistInfo((SpecialistTypes)piIndexList[piOrderedIndex[i]]).isAltDown(), GC.getSpecialistInfo((SpecialistTypes)piIndexList[piOrderedIndex[i]]).isCtrlDown()));
		}
		else if ((ActionSubTypes)piActionInfoTypeList[piOrderedIndex[i]] == ACTIONSUBTYPE_BUILDING)
		{
			GC.getBuildingInfo((BuildingTypes)piIndexList[piOrderedIndex[i]]).setMissionType(FindInInfoClass("MISSION_CONSTRUCT", GC.getMissionInfo(), sizeof(GC.getMissionInfo((MissionTypes) 0)), NUM_MISSION_TYPES));
			GC.getBuildingInfo((BuildingTypes)piIndexList[piOrderedIndex[i]]).setActionInfoIndex(i);
			GC.getBuildingInfo((BuildingTypes)piIndexList[piOrderedIndex[i]]).setHotKeyDescription(GC.getBuildingInfo((BuildingTypes)piIndexList[piOrderedIndex[i]]).getTextKeyWide(), GC.getMissionInfo((MissionTypes)(GC.getBuildingInfo((BuildingTypes)piIndexList[piOrderedIndex[i]]).getMissionType())).getTextKeyWide(), CreateHotKeyFromDescription(GC.getBuildingInfo((BuildingTypes)piIndexList[piOrderedIndex[i]]).getHotKey(), GC.getBuildingInfo((BuildingTypes)piIndexList[piOrderedIndex[i]]).isShiftDown(), GC.getBuildingInfo((BuildingTypes)piIndexList[piOrderedIndex[i]]).isAltDown(), GC.getBuildingInfo((BuildingTypes)piIndexList[piOrderedIndex[i]]).isCtrlDown()));
		}
		else if ((ActionSubTypes)piActionInfoTypeList[piOrderedIndex[i]] == ACTIONSUBTYPE_CONTROL)
		{
			GC.getControlInfo((ControlTypes)piIndexList[piOrderedIndex[i]]).setActionInfoIndex(i);
		}
		else if ((ActionSubTypes)piActionInfoTypeList[piOrderedIndex[i]] == ACTIONSUBTYPE_AUTOMATE)
		{
			GC.getAutomateInfo(piIndexList[piOrderedIndex[i]]).setActionInfoIndex(i);
		}
		else if ((ActionSubTypes)piActionInfoTypeList[piOrderedIndex[i]] == ACTIONSUBTYPE_MISSION)
		{
			GC.getMissionInfo((MissionTypes)piIndexList[piOrderedIndex[i]]).setActionInfoIndex(i + *iNumVals);
		}
	}

	*iNumVals = iNumActionInfos;

	SAFE_DELETE_ARRAY(piOrderedIndex);
	SAFE_DELETE_ARRAY(piIndexList);
	SAFE_DELETE_ARRAY(piPriorityList);
	SAFE_DELETE_ARRAY(piActionInfoTypeList);
}


//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetGlobalAnimationPathInfo(CvAnimationPathInfo** ppAnimationPathInfo, char* szTagName, int* iNumVals)
//
//  PURPOSE :   Takes the szTagName parameter and if it exists in the xml it loads the ppAnimationPathInfo
//				with the value under it and sets the value of the iNumVals parameter to the total number
//				of occurances of the szTagName tag in the xml.   
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetGlobalAnimationPathInfo(CvAnimationPathInfo** ppAnimationPathInfo, char* szTagName, int* iNumVals)
{
	PROFILE_FUNC();
	logMsg( "SetGlobalAnimationPathInfo %s\n", szTagName );

	int		i;						// Loop counters
	CvAnimationPathInfo * pAnimPathInfo;	// local pointer to the domain info memory

	if ( gDLL->getXMLIFace()->LocateNode(m_pFXml, szTagName ))
	{
		// get the number of times the szTagName tag appears in the xml file
		*iNumVals = gDLL->getXMLIFace()->NumOfElementsByTagName(m_pFXml,szTagName);

		// allocate memory for the domain info based on the number above
		*ppAnimationPathInfo = new CvAnimationPathInfo[*iNumVals];
		pAnimPathInfo = *ppAnimationPathInfo;

		gDLL->getXMLIFace()->SetToParent(m_pFXml);
		gDLL->getXMLIFace()->SetToChild(m_pFXml);
		gDLL->getXMLIFace()->SetToChild(m_pFXml);


		// Loop through each tag.
		for (i=0;i<*iNumVals;i++)
		{
			SkipToNextVal();	// skip to the next non-comment node

			if (!pAnimPathInfo[i].read(this))
				break;
			GC.setInfoTypeFromString(pAnimPathInfo[i].getType(), i);	// add type to global info type hash map
			if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
			{
				break;
			}
		}
	}

	// if we didn't find the tag name in the xml then we never set the local pointer to the 
	// newly allocated memory and there for we will FAssert to let people know this most
	// interesting fact
	if(!pAnimPathInfo )
	{
		char	szMessage[1024];
		sprintf( szMessage, "Error finding tag node in SetGlobalAnimationPathInfo function \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
}

//
// CvPromotionInfo
//
void CvXMLLoadUtility::SetGlobalPromotionInfo(CvPromotionInfo **ppPromotionInfos, char* szTagName, int* iNumVals)
{
	PROFILE_FUNC();
	logMsg("SetGlobalPromotionInfo");

	int i=0;					//loop counter
	CvPromotionInfo* pPromotionInfo;	// local pointer to the building info memory
	CvString* pszPrereq = NULL;
	CvString* pszPrereqOr1 = NULL;
	CvString* pszPrereqOr2 = NULL;

	// if we successfully locate the tag name in the xml file
	if (gDLL->getXMLIFace()->LocateNode(m_pFXml,szTagName))
	{
		// get the number of times the szTagName tag appears in the xml file
		*iNumVals = gDLL->getXMLIFace()->NumOfElementsByTagName(m_pFXml,szTagName);

		pszPrereq = new CvString[*iNumVals];
		pszPrereqOr1 = new CvString[*iNumVals];
		pszPrereqOr2 = new CvString[*iNumVals];

		// allocate memory for the building info based on the number above
		*ppPromotionInfos = new CvPromotionInfo[*iNumVals];
		// set the local pointer to the memory we just allocated
		pPromotionInfo = *ppPromotionInfos;

		gDLL->getXMLIFace()->SetToParent(m_pFXml);
		gDLL->getXMLIFace()->SetToChild(m_pFXml);

		// loop through each tag
		for (i=0;i<*iNumVals;i++)
		{
			SkipToNextVal();	// skip to the next non-comment node

			if (!pPromotionInfo[i].read(this))
				break;
			GC.setInfoTypeFromString(pPromotionInfo[i].getType(), i);	// add type to global info type hash map
			GetChildXmlValByName(pszPrereq[i], "PromotionPrereq");
			GetChildXmlValByName(pszPrereqOr1[i], "PromotionPrereqOr1");
			GetChildXmlValByName(pszPrereqOr2[i], "PromotionPrereqOr2");

			if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
			{
				break;
			}
		}
	}

	for (i=0;i<GC.getNumPromotionInfos();i++)
	{
		GC.getPromotionInfo((PromotionTypes) i).setPrereqPromotion(FindInInfoClass(pszPrereq[i], GC.getPromotionInfo(), sizeof(GC.getPromotionInfo((PromotionTypes) i)), GC.getNumPromotionInfos()));
		GC.getPromotionInfo((PromotionTypes) i).setPrereqOrPromotion1(FindInInfoClass(pszPrereqOr1[i], GC.getPromotionInfo(), sizeof(GC.getPromotionInfo((PromotionTypes) i)), GC.getNumPromotionInfos()));
		GC.getPromotionInfo((PromotionTypes) i).setPrereqOrPromotion2(FindInInfoClass(pszPrereqOr2[i], GC.getPromotionInfo(), sizeof(GC.getPromotionInfo((PromotionTypes) i)), GC.getNumPromotionInfos()));
	}

	SAFE_DELETE_ARRAY(pszPrereq);
	SAFE_DELETE_ARRAY(pszPrereqOr1);
	SAFE_DELETE_ARRAY(pszPrereqOr2);

	// if we didn't find the tag name in the xml then we never set the pszString to the 
	// newly allocated memory and there for we will FAssert to let people know this most
	// interesting fact
	if(!pPromotionInfo)
	{
		char	szMessage[1024];
		sprintf( szMessage, "Error finding tag node in SetGlobalPromotionInfo function \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetGlobalUnitScales(float* pfLargeScale, float* pfSmallScale, char* szTagName)
//
//  PURPOSE :   Takes the szTagName parameter and if it exists in the xml it loads the ppPromotionInfo
//				with the value under it and sets the value of the iNumVals parameter to the total number
//				of occurances of the szTagName tag in the xml.   
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetGlobalUnitScales(float* pfLargeScale, float* pfSmallScale, char* szTagName)
{
	PROFILE_FUNC();
	logMsg("SetGlobalUnitScales %s\n", szTagName);
	// if we successfully locate the szTagName node
	if (gDLL->getXMLIFace()->LocateNode(m_pFXml,szTagName))
	{
		// call the function that sets the FXml pointer to the first non-comment child of
		// the current tag and gets the value of that new node
		if (GetChildXmlVal(pfLargeScale))
		{
			// set the current xml node to it's next sibling and then
			// get the sibling's TCHAR value
			GetNextXmlVal(pfSmallScale);

			// set the current xml node to it's parent node
			gDLL->getXMLIFace()->SetToParent(m_pFXml);
		}
	}
	else
	{
		// if we didn't find the tag name in the xml then we never set the local pointer to the 
		// newly allocated memory and there for we will FAssert to let people know this most
		// interesting fact
		char	szMessage[1024];
		sprintf( szMessage, "Error finding tag node in SetGlobalUnitScales function \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetGlobalTechInfo(CvTechInfo** ppTechInfo, char* szTagName, int* iNumVals)
//
//  PURPOSE :   Takes the szTagName parameter and if it exists in the xml it loads the ppTechInfo
//				with the value under it and sets the value of the iNumVals parameter to the total number
//				of occurances of the szTagName tag in the xml.   
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetGlobalTechInfo(CvTechInfo** ppTechInfo, char* szTagName, int* iNumVals)
{
	PROFILE_FUNC();
	logMsg("SetGlobalTechInfo %s\n", szTagName);
	int i=0;				//loop counter
	int j=0;				//loop counter
	int k=0;				//loop counter
	int m=0;				//loop counter
	int iNumSibs=0;				// the number of siblings the current xml node has
	char szTextVal[256];	// Holds the value of a retrieved key that is to be interpreted

	CvTechInfo* pTechInfo;	// local pointer to the tech info memory

	// null out the local pointer so it can be checked at the end of this function
	pTechInfo = NULL;

	// if we successfully locate the szTagName node
	if (gDLL->getXMLIFace()->LocateNode(m_pFXml,szTagName))
	{
		// get the number of times the szTagName tag appears in the xml file
		*iNumVals = gDLL->getXMLIFace()->NumOfElementsByTagName(m_pFXml,szTagName);
		// allocate memory for the tech info based on the number above
		*ppTechInfo = new CvTechInfo[*iNumVals];
		// set the local pointer to the memory we just allocated
		pTechInfo = *ppTechInfo;

		gDLL->getXMLIFace()->SetToParent(m_pFXml);
		gDLL->getXMLIFace()->SetToChild(m_pFXml);

		// loop through each tag
		for (i=0;i<*iNumVals;i++)
		{
			SkipToNextVal();	// skip to the next non-comment node

			if (!pTechInfo[i].read(this))
			{
				break;
			}
			GC.setInfoTypeFromString(pTechInfo[i].getType(), i);	// add type to global info type hash map

			// if we cannot set the current xml node to it's next sibling then we will break out of the for loop
			// otherwise we will continue looping
			if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
			{
				break;
			}
		}
	}

	// if we successfully locate the szTagName node
	if (gDLL->getXMLIFace()->LocateNode(m_pFXml,szTagName))
	{
		gDLL->getXMLIFace()->SetToParent(m_pFXml);
		gDLL->getXMLIFace()->SetToChild(m_pFXml);

		// loop through each tag
		for (i=0;i<*iNumVals;i++)
		{
			if (gDLL->getXMLIFace()->SetToChildByTagName(m_pFXml,"OrPreReqs"))
			{
				if (SkipToNextVal())
				{
					iNumSibs = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);
					if(!(0 < GC.getDefineINT("NUM_OR_TECH_PREREQS")))
					{
						char	szMessage[1024];
						sprintf( szMessage, "Allocating zero or less memory in SetGlobalTechInfo \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
						gDLL->MessageBox(szMessage, "XML Error");
					}
					InitList(&pTechInfo[i].m_piPrereqOrTechs, GC.getDefineINT("NUM_OR_TECH_PREREQS"), -1);

					if (0 < iNumSibs)
					{
						if (GetChildXmlVal(szTextVal))
						{
							if(!(iNumSibs <= GC.getDefineINT("NUM_OR_TECH_PREREQS")))
							{
								char	szMessage[1024];
								sprintf( szMessage, "There are more siblings than memory allocated for them in SetGlobalTechInfo \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
								gDLL->MessageBox(szMessage, "XML Error");
							}
							for (j=0;j<iNumSibs;j++)
							{
								pTechInfo[i].m_piPrereqOrTechs[j] = FindInInfoClass(szTextVal, GC.getTechInfo(), sizeof(GC.getTechInfo((TechTypes) 0)), GC.getNumTechInfos());
								if (!GetNextXmlVal(szTextVal))
								{
									break;
								}
							}

							gDLL->getXMLIFace()->SetToParent(m_pFXml);
						}
					}
				}

				gDLL->getXMLIFace()->SetToParent(m_pFXml);
			}

			if (gDLL->getXMLIFace()->SetToChildByTagName(m_pFXml,"AndPreReqs"))
			{
				if (SkipToNextVal())
				{
					iNumSibs = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);
					if(!(0 < GC.getDefineINT("NUM_AND_TECH_PREREQS")))
					{
						char	szMessage[1024];
						sprintf( szMessage, "Allocating zero or less memory in SetGlobalTechInfo \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
						gDLL->MessageBox(szMessage, "XML Error");
					}
					InitList(&pTechInfo[i].m_piPrereqAndTechs, GC.getDefineINT("NUM_AND_TECH_PREREQS"), -1);

					if (0 < iNumSibs)
					{
						if (GetChildXmlVal(szTextVal))
						{
							if(!(iNumSibs <= GC.getDefineINT("NUM_AND_TECH_PREREQS")))
							{
								char	szMessage[1024];
								sprintf( szMessage, "There are more siblings than memory allocated for them in SetGlobalTechInfo \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
								gDLL->MessageBox(szMessage, "XML Error");
							}
							for (j=0;j<iNumSibs;j++)
							{
								pTechInfo[i].m_piPrereqAndTechs[j] = FindInInfoClass(szTextVal, GC.getTechInfo(), sizeof(GC.getTechInfo((TechTypes) 0)), GC.getNumTechInfos());
								if (!GetNextXmlVal(szTextVal))
								{
									break;
								}
							}

							gDLL->getXMLIFace()->SetToParent(m_pFXml);
						}
					}
				}

				gDLL->getXMLIFace()->SetToParent(m_pFXml);
			}

			// if we cannot set the current xml node to it's next sibling then we will break out of the for loop
			// otherwise we will continue looping
			if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
			{
				break;
			}
		}
	}

	// if we didn't find the tag name in the xml then we never set the local pointer to the 
	// newly allocated memory and there for we will FAssert to let people know this most
	// interesting fact
	if(!pTechInfo)
	{
		char	szMessage[1024];
		sprintf( szMessage, "Error finding tag node in SetGlobalTechInfo function \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetGameText()
//
//  PURPOSE :   Reads game text info from XML and adds it to the translation manager
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetGameText(const char* szTextGroup, const char* szTagName)
{
	PROFILE_FUNC();
	logMsg("SetGameText %s\n", szTagName);
	int i=0;		//loop counter - Index into pTextInfo

	if (gDLL->getXMLIFace()->LocateNode(m_pFXml, szTextGroup)) // Get the Text Group 1st 
	{
		int iNumVals = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);	// Get the number of Children that the Text Group has
		gDLL->getXMLIFace()->LocateNode(m_pFXml, szTagName); // Now switch to the TEXT Tag
		gDLL->getXMLIFace()->SetToParent(m_pFXml);
		gDLL->getXMLIFace()->SetToChild(m_pFXml);

		// loop through each tag
		for (i=0; i < iNumVals; i++)
		{
			CvGameText textInfo;
			textInfo.read(this);

			gDLL->addText(textInfo.getType() /*id*/, textInfo.getText(), textInfo.getGender(), textInfo.getPlural());
			if (!gDLL->getXMLIFace()->NextSibling(m_pFXml) && i!=iNumVals-1)
			{
				char	szMessage[1024];
				sprintf( szMessage, "failed to find sibling \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
				gDLL->MessageBox(szMessage, "XML Error");
				break;
			}		
		}
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetGlobalClassInfo - This is a template function that is USED FOR ALMOST ALL INFO CLASSES.
//		Each info class should have a read(CvXMLLoadUtility*) function that is responsible for initializing
//		the class from xml data.
//
//  PURPOSE :   takes the szTagName parameter and loads the ppszString with the text values
//				under the tags.  This will be the hints displayed during game initialization and load
//
//------------------------------------------------------------------------------------------------------
template <class T>
void CvXMLLoadUtility::SetGlobalClassInfo(T **ppClassInfos, char* szTagName, int* iNumVals, bool bUseEnum)
{
	char szLog[256];
	sprintf(szLog, "SetGlobalClassInfo (%s)", szTagName);
	PROFILE(szLog);
	logMsg(szLog);

	int i=0;					//loop counter
	T* pClassInfo = NULL;	// local pointer to the building info memory

	// if we successfully locate the tag name in the xml file
	if (gDLL->getXMLIFace()->LocateNode(m_pFXml,szTagName))
	{
		if (!bUseEnum)
		{
			// get the number of times the szTagName tag appears in the xml file
			*iNumVals = gDLL->getXMLIFace()->NumOfElementsByTagName(m_pFXml,szTagName);
		}

		// allocate memory for the building info based on the number above
		*ppClassInfos = new T[*iNumVals];
		//OutputDebugString(CvString::format("[Jason] %s\t0x%x\n", szTagName, *ppClassInfos));
		// set the local pointer to the memory we just allocated
		pClassInfo = *ppClassInfos;

		// loop through each tag
		for (i=0;i<*iNumVals;i++)
		{
			SkipToNextVal();	// skip to the next non-comment node

			if (!pClassInfo[i].read(this))
			{
				break;
			}
			GC.setInfoTypeFromString(pClassInfo[i].getType(), i);	// add type to global info type hash map
			if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
			{
				break;
			}
		}
	}

	// if we didn't find the tag name in the xml then we never set the pszString to the 
	// newly allocated memory and there for we will FAssert to let people know this most
	// interesting fact
	if(!pClassInfo)
	{
		char	szMessage[1024];
		sprintf( szMessage, "Error locating tag node in SetGlobalClassInfo function \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
}

//
// helper sort predicate
//

struct OrderIndex {int m_iPriority; int m_iIndex;};
bool sortHotkeyPriority(const OrderIndex orderIndex1, const OrderIndex orderIndex2)
{
	return (orderIndex1.m_iPriority > orderIndex2.m_iPriority);
}

template <class T>
void CvXMLLoadUtility::orderHotkeyInfo(int** ppiSortedIndex, T* pHotkeyInfos, int iLength)
{
	int iI;
	int* piSortedIndex;
	std::vector<OrderIndex> viOrderPriority;

	viOrderPriority.resize(iLength);
	piSortedIndex = *ppiSortedIndex;

	// set up vector
	for(iI=0;iI<iLength;iI++)
	{
		viOrderPriority[iI].m_iPriority = pHotkeyInfos[iI].getOrderPriority();
		viOrderPriority[iI].m_iIndex = iI;
	}

	// sort the array
	std::sort(viOrderPriority.begin(), viOrderPriority.end(), sortHotkeyPriority);

	// insert new order into the array to return
	for (iI=0;iI<iLength;iI++)
	{
		piSortedIndex[iI] = viOrderPriority[iI].m_iIndex;
	}
}

void CvXMLLoadUtility::orderHotkeyInfo(int** ppiSortedIndex, int* pHotkeyIndex, int iLength)
{
	int iI;
	int* piSortedIndex;
	std::vector<OrderIndex> viOrderPriority;

	viOrderPriority.resize(iLength);
	piSortedIndex = *ppiSortedIndex;

	// set up vector
	for(iI=0;iI<iLength;iI++)
	{
		viOrderPriority[iI].m_iPriority = pHotkeyIndex[iI];
		viOrderPriority[iI].m_iIndex = iI;
	}

	// sort the array
	std::sort(viOrderPriority.begin(), viOrderPriority.end(), sortHotkeyPriority);

	// insert new order into the array to return
	for (iI=0;iI<iLength;iI++)
	{
		piSortedIndex[iI] = viOrderPriority[iI].m_iIndex;
	}
}

//
// helper sort predicate
//
/*
bool sortHotkeyPriorityOld(const CvHotkeyInfo* hotkey1, const CvHotkeyInfo* hotkey2)
{
return (hotkey1->getOrderPriority() < hotkey2->getOrderPriority());
}
*/

//------------------------------------------------------------------------------------------------
// FUNCTION:    void CvXMLLoadUtility::orderHotkeyInfoOld(T **ppHotkeyInfos, int iLength)
//! \brief      order a hotkey info derived class
//! \param      ppHotkeyInfos is a hotkey info derived class
//!							iLength is the length of the hotkey info derived class array
//! \retval     
//------------------------------------------------------------------------------------------------
/*
template <class T>
void CvXMLLoadUtility::orderHotkeyInfoOld(T **ppHotkeyInfos, int iLength)
{
int iI;
std::vector<T*> vHotkeyInfo;
T* pHotkeyInfo;	// local pointer to the hotkey info memory

for (iI=0;iI<iLength;iI++)
{
pHotkeyInfo = new T;
*pHotkeyInfo = (*ppHotkeyInfos)[iI];
vHotkeyInfo.push_back(pHotkeyInfo);
}

std::sort(vHotkeyInfo.begin(), vHotkeyInfo.end(), sortHotkeyPriority);

for (iI=0;iI<iLength;iI++)
{
(*ppHotkeyInfos)[iI] = *vHotkeyInfo[iI];
}

for (iI=0;iI<(int)vHotkeyInfo.size();iI++)
{
vHotkeyInfo[iI]->reset();
SAFE_DELETE(vHotkeyInfo[iI]);
}
vHotkeyInfo.clear();
}
*/

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetYields(int** ppiYield)
//
//  PURPOSE :   Allocate memory for the yield parameter and set it to the values
//				in the xml file.  The current/last located node must be the first child of the
//				yield changes node
//
//------------------------------------------------------------------------------------------------------
int CvXMLLoadUtility::SetYields(int** ppiYield)
{
	int i=0;			//loop counter
	int iNumSibs=0;		// the number of siblings the current xml node has
	int *piYield;	// local pointer for the yield memory

	// Skip any comments and stop at the next value we might want
	if (SkipToNextVal())
	{
		// get the total number of children the current xml node has
		iNumSibs = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);

		InitList(ppiYield, NUM_YIELD_TYPES);

		// set the local pointer to the memory we just allocated
		piYield = *ppiYield;

		if (0 < iNumSibs)
		{
			// if the call to the function that sets the current xml node to it's first non-comment
			// child and sets the parameter with the new node's value succeeds
			if (GetChildXmlVal(&piYield[0]))
			{
				if(!(iNumSibs <= NUM_YIELD_TYPES))
				{
					char	szMessage[1024];
					sprintf( szMessage, "For loop iterator is greater than array size \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Error");
				}
				// loop through all the siblings, we start at 1 since we already have the first value
				for (i=1;i<iNumSibs;i++)
				{
					// if the call to the function that sets the current xml node to it's first non-comment
					// sibling and sets the parameter with the new node's value does not succeed
					// we will break out of this for loop
					if (!GetNextXmlVal(&piYield[i]))
					{
						break;
					}
				}
				// set the current xml node to it's parent node
				gDLL->getXMLIFace()->SetToParent(m_pFXml);
			}
		}
	}

	return iNumSibs;
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetFeatureStruct(int** ppiFeatureTech, int** ppiFeatureTime, int** ppiFeatureProduction, bool** ppbFeatureRemove)
//
//  PURPOSE :   allocate and set the feature struct variables for the CvBuildInfo class
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetFeatureStruct(int** ppiFeatureTech, int** ppiFeatureTime, int** ppiFeatureProduction, bool** ppbFeatureRemove)
{
	int i=0;				//loop counter
	int iNumSibs;					// the number of siblings the current xml node has
	int iFeatureIndex;
	TCHAR szTextVal[256];	// temporarily hold the text value of the current xml node
	int* paiFeatureTech = NULL;
	int* paiFeatureTime = NULL;
	int* paiFeatureProduction = NULL;
	bool* pabFeatureRemove = NULL;

	if(GC.getNumFeatureInfos() < 1)
	{
		char	szMessage[1024];
		sprintf( szMessage, "no feature infos set yet! \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	InitList(ppiFeatureTech, GC.getNumFeatureInfos(), -1);
	InitList(ppiFeatureTime, GC.getNumFeatureInfos());
	InitList(ppiFeatureProduction, GC.getNumFeatureInfos());
	InitList(ppbFeatureRemove, GC.getNumFeatureInfos());

	paiFeatureTech = *ppiFeatureTech;
	paiFeatureTime = *ppiFeatureTime;
	paiFeatureProduction = *ppiFeatureProduction;
	pabFeatureRemove = *ppbFeatureRemove;

	if (gDLL->getXMLIFace()->SetToChildByTagName(m_pFXml,"FeatureStructs"))
	{
		iNumSibs = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);

		if (0 < iNumSibs)
		{
			if (gDLL->getXMLIFace()->SetToChildByTagName(m_pFXml,"FeatureStruct"))
			{
				if(!(iNumSibs <= GC.getNumFeatureInfos()))
				{
					char	szMessage[1024];
					sprintf( szMessage, "iNumSibs is greater than GC.getNumFeatureInfos in SetFeatureStruct function \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Error");
				}
				for (i=0;i<iNumSibs;i++)
				{
					GetChildXmlValByName(szTextVal, "FeatureType");
					iFeatureIndex = FindInInfoClass(szTextVal, GC.getFeatureInfo(), sizeof(GC.getFeatureInfo((FeatureTypes) 0)), GC.getNumFeatureInfos());
					if(!(iFeatureIndex != -1))
					{
						char	szMessage[1024];
						sprintf( szMessage, "iFeatureIndex is -1 inside SetFeatureStruct function \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
						gDLL->MessageBox(szMessage, "XML Error");
					}
					GetChildXmlValByName(szTextVal, "PrereqTech");
					paiFeatureTech[iFeatureIndex] = FindInInfoClass(szTextVal, GC.getTechInfo(), sizeof(GC.getTechInfo((TechTypes) 0)), GC.getNumTechInfos());
					GetChildXmlValByName(&paiFeatureTime[iFeatureIndex], "iTime");
					GetChildXmlValByName(&paiFeatureProduction[iFeatureIndex], "iProduction");
					GetChildXmlValByName(&pabFeatureRemove[iFeatureIndex], "bRemove");

					if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
					{
						break;
					}
				}

				gDLL->getXMLIFace()->SetToParent(m_pFXml);
			}
		}

		gDLL->getXMLIFace()->SetToParent(m_pFXml);
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetImprovementBonuses(CvImprovementBonusInfo** ppImprovementBonus)
//
//  PURPOSE :   Allocate memory for the improvement bonus pointer and fill it based on the
//				values in the xml.
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetImprovementBonuses(CvImprovementBonusInfo** ppImprovementBonus)
{
	int i=0;				//loop counter
	int j=0;				//loop counter
	int iNumSibs;			// the number of siblings the current xml node has
	TCHAR szNodeVal[256];	// temporarily holds the string value of the current xml node
	CvImprovementBonusInfo* paImprovementBonus;	// local pointer to the bonus type struct in memory

	// Skip any comments and stop at the next value we might want
	if (SkipToNextVal())
	{
		// initialize the boolean list to the correct size and all the booleans to false
		InitImprovementBonusList(ppImprovementBonus, GC.getNumBonusInfos());
		// set the local pointer to the memory we just allocated
		paImprovementBonus = *ppImprovementBonus;

		// get the total number of children the current xml node has
		iNumSibs = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);
		// if we can set the current xml node to the child of the one it is at now
		if (gDLL->getXMLIFace()->SetToChild(m_pFXml))
		{
			if(!(iNumSibs <= GC.getNumBonusInfos()))
			{
				char	szMessage[1024];
				sprintf( szMessage, "For loop iterator is greater than array size \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
				gDLL->MessageBox(szMessage, "XML Error");
			}
			// loop through all the siblings
			for (i=0;i<iNumSibs;i++)
			{
				// skip to the next non-comment node
				if (SkipToNextVal())
				{
					// call the function that sets the FXml pointer to the first non-comment child of 
					// the current tag and gets the value of that new node
					if (GetChildXmlVal(szNodeVal))
					{
						int iBonusIndex;	// index of the match in the bonus types list
						// call the find in list function to return either -1 if no value is found
						// or the index in the list the match is found at
						iBonusIndex = FindInInfoClass(szNodeVal, GC.getBonusInfo(), sizeof(GC.getBonusInfo((BonusTypes) 0)), GC.getNumBonusInfos());
						// if we found a match we will get the next sibling's boolean value at that match's index
						if (iBonusIndex >= 0)
						{
							GetNextXmlVal(&paImprovementBonus[iBonusIndex].m_bBonusMakesValid);
							GetNextXmlVal(&paImprovementBonus[iBonusIndex].m_bBonusTrade);
							GetNextXmlVal(&paImprovementBonus[iBonusIndex].m_iDiscoverRand);
							gDLL->getXMLIFace()->SetToParent(m_pFXml);

							SAFE_DELETE_ARRAY(paImprovementBonus[iBonusIndex].m_piYieldChange);	// free memory - MT, since we are about to reallocate
							if (gDLL->getXMLIFace()->SetToChildByTagName(m_pFXml,"YieldChanges"))
							{
								SetYields(&paImprovementBonus[iBonusIndex].m_piYieldChange);
								gDLL->getXMLIFace()->SetToParent(m_pFXml);
							}
							else
							{
								InitList(&paImprovementBonus[iBonusIndex].m_piYieldChange, NUM_YIELD_TYPES);
							}
						}
						else
						{
							gDLL->getXMLIFace()->SetToParent(m_pFXml);
						}

						// set the current xml node to it's parent node
					}

					// if we cannot set the current xml node to it's next sibling then we will break out of the for loop
					// otherwise we will continue looping
					if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
					{
						break;
					}
				}
			}
			// set the current xml node to it's parent node
			gDLL->getXMLIFace()->SetToParent(m_pFXml);
		}
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetAndLoadVar(int** ppiVar, int iDefault)
//
//  PURPOSE :   set the variable to a default and load it from the xml if there are any children
//
//------------------------------------------------------------------------------------------------------
bool CvXMLLoadUtility::SetAndLoadVar(int** ppiVar, int iDefault)
{
	int iNumSibs;
	int* piVar;
	bool bReturn = false;
	int i; // loop counter

	// Skip any comments and stop at the next value we might want
	if (SkipToNextVal())
	{
		bReturn = true;

		// get the total number of children the current xml node has
		iNumSibs = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);

		// allocate memory 
		InitList(ppiVar, iNumSibs, iDefault);

		// set the a local pointer to the newly allocated memory
		piVar = *ppiVar;

		// if the call to the function that sets the current xml node to it's first non-comment
		// child and sets the parameter with the new node's value succeeds
		if (GetChildXmlVal(&piVar[0]))
		{
			// loop through all the siblings, we start at 1 since we already got the first sibling
			for (i=1;i<iNumSibs;i++)
			{
				// if the call to the function that sets the current xml node to it's next non-comment
				// sibling and sets the parameter with the new node's value does not succeed
				// we will break out of this for loop
				if (!GetNextXmlVal(&piVar[i]))
				{
					break;
				}
			}

			// set the current xml node to it's parent node
			gDLL->getXMLIFace()->SetToParent(m_pFXml);
		}
	}

	return bReturn;
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetVariableListTagPair(	int **ppiList, const TCHAR* szRootTagName, 
//										CvInfoBase* pCvInfoBase, 
//										int iInfoBaseSize, int iInfoBaseLength, int iDefaultListVal)
//
//  PURPOSE :   allocate and initialize a list from a tag pair in the xml
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetVariableListTagPair(int **ppiList, const TCHAR* szRootTagName,
											  CvInfoBase* pCvInfoBase,
											  int iInfoBaseSize, int iInfoBaseLength, int iDefaultListVal)
{
	int i;
	int iIndexVal;
	int iNumSibs;
	TCHAR szTextVal[256];
	int* piList;

	if(!(0 < iInfoBaseLength))
	{
		char	szMessage[1024];
		sprintf( szMessage, "Allocating zero or less memory in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	InitList(ppiList, iInfoBaseLength, iDefaultListVal);
	if (gDLL->getXMLIFace()->SetToChildByTagName(m_pFXml,szRootTagName))
	{
		if (SkipToNextVal())
		{
			iNumSibs = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);
			piList = *ppiList;
			if (0 < iNumSibs)
			{
				if(!(iNumSibs <= iInfoBaseLength))
				{
					char	szMessage[1024];
					sprintf( szMessage, "There are more siblings than memory allocated for them in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Error");
				}
				if (gDLL->getXMLIFace()->SetToChild(m_pFXml))
				{
					for (i=0;i<iNumSibs;i++)
					{
						if (GetChildXmlVal(szTextVal))
						{
							iIndexVal = FindInInfoClass(szTextVal, pCvInfoBase, iInfoBaseSize, iInfoBaseLength);
							GetNextXmlVal(&piList[iIndexVal]);

							gDLL->getXMLIFace()->SetToParent(m_pFXml);
						}

						if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
						{
							break;
						}
					}

					gDLL->getXMLIFace()->SetToParent(m_pFXml);
				}
			}
		}

		gDLL->getXMLIFace()->SetToParent(m_pFXml);
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetVariableListTagPair(	bool **ppbList, const TCHAR* szRootTagName, 
//										CvInfoBase* pCvInfoBase, 
//										int iInfoBaseSize, int iInfoBaseLength, bool bDefaultListVal)
//
//  PURPOSE :   allocate and initialize a list from a tag pair in the xml
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetVariableListTagPair(bool **ppbList, const TCHAR* szRootTagName,
											  CvInfoBase* pCvInfoBase,
											  int iInfoBaseSize, int iInfoBaseLength, bool bDefaultListVal)
{
	int i;
	int iIndexVal;
	int iNumSibs;
	TCHAR szTextVal[256];
	bool* pbList;

	if(!(0 < iInfoBaseLength))
	{
		char	szMessage[1024];
		sprintf( szMessage, "Allocating zero or less memory in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	InitList(ppbList, iInfoBaseLength, bDefaultListVal);
	if (gDLL->getXMLIFace()->SetToChildByTagName(m_pFXml,szRootTagName))
	{
		if (SkipToNextVal())
		{
			iNumSibs = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);
			pbList = *ppbList;
			if (0 < iNumSibs)
			{
				if(!(iNumSibs <= iInfoBaseLength))
				{
					char	szMessage[1024];
					sprintf( szMessage, "There are more siblings than memory allocated for them in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Error");
				}
				if (gDLL->getXMLIFace()->SetToChild(m_pFXml))
				{
					for (i=0;i<iNumSibs;i++)
					{
						if (GetChildXmlVal(szTextVal))
						{
							iIndexVal = FindInInfoClass(szTextVal, pCvInfoBase, iInfoBaseSize, iInfoBaseLength);
							GetNextXmlVal(&pbList[iIndexVal]);

							gDLL->getXMLIFace()->SetToParent(m_pFXml);
						}

						if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
						{
							break;
						}
					}

					gDLL->getXMLIFace()->SetToParent(m_pFXml);
				}
			}
		}

		gDLL->getXMLIFace()->SetToParent(m_pFXml);
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetVariableListTagPair(	float **ppfList, const TCHAR* szRootTagName, 
//										CvInfoBase* pCvInfoBase, 
//										int iInfoBaseSize, int iInfoBaseLength, float fDefaultListVal)
//
//  PURPOSE :   allocate and initialize a list from a tag pair in the xml
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetVariableListTagPair(float **ppfList, const TCHAR* szRootTagName,
											  CvInfoBase* pCvInfoBase,
											  int iInfoBaseSize, int iInfoBaseLength, float fDefaultListVal)
{
	int i;
	int iIndexVal;
	int iNumSibs;
	TCHAR szTextVal[256];
	float* pfList;

	if(!(0 < iInfoBaseLength))
	{
		char	szMessage[1024];
		sprintf( szMessage, "Allocating zero or less memory in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	InitList(ppfList, iInfoBaseLength, fDefaultListVal);
	if (gDLL->getXMLIFace()->SetToChildByTagName(m_pFXml,szRootTagName))
	{
		if (SkipToNextVal())
		{
			iNumSibs = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);
			pfList = *ppfList;
			if (0 < iNumSibs)
			{
				if(!(iNumSibs <= iInfoBaseLength))
				{
					char	szMessage[1024];
					sprintf( szMessage, "There are more siblings than memory allocated for them in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Error");
				}
				if (gDLL->getXMLIFace()->SetToChild(m_pFXml))
				{
					for (i=0;i<iNumSibs;i++)
					{
						if (GetChildXmlVal(szTextVal))
						{
							iIndexVal = FindInInfoClass(szTextVal, pCvInfoBase, iInfoBaseSize, iInfoBaseLength);
							GetNextXmlVal(&pfList[iIndexVal]);

							gDLL->getXMLIFace()->SetToParent(m_pFXml);
						}

						if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
						{
							break;
						}
					}

					gDLL->getXMLIFace()->SetToParent(m_pFXml);
				}
			}
		}

		gDLL->getXMLIFace()->SetToParent(m_pFXml);
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetVariableListTagPair(	CvString **ppfList, const TCHAR* szRootTagName, 
//										CvInfoBase* pCvInfoBase, 
//										int iInfoBaseSize, int iInfoBaseLength, CvString szDefaultListVal)
//
//  PURPOSE :   allocate and initialize a list from a tag pair in the xml
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetVariableListTagPair(CvString **ppszList, const TCHAR* szRootTagName,
											  CvInfoBase* pCvInfoBase,
											  int iInfoBaseSize, int iInfoBaseLength, CvString szDefaultListVal)
{
	int i;
	int iIndexVal;
	int iNumSibs;
	TCHAR szTextVal[256];
	CvString* pszList;

	if(!(0 < iInfoBaseLength))
	{
		char	szMessage[1024];
		sprintf( szMessage, "Allocating zero or less memory in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	InitStringList(ppszList, iInfoBaseLength, szDefaultListVal);
	if (gDLL->getXMLIFace()->SetToChildByTagName(m_pFXml,szRootTagName))
	{
		if (SkipToNextVal())
		{
			iNumSibs = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);
			pszList = *ppszList;
			if (0 < iNumSibs)
			{
				if(!(iNumSibs <= iInfoBaseLength))
				{
					char	szMessage[1024];
					sprintf( szMessage, "There are more siblings than memory allocated for them in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Error");
				}
				if (gDLL->getXMLIFace()->SetToChild(m_pFXml))
				{
					for (i=0;i<iNumSibs;i++)
					{
						if (GetChildXmlVal(szTextVal))
						{
							iIndexVal = FindInInfoClass(szTextVal, pCvInfoBase, iInfoBaseSize, iInfoBaseLength);
							GetNextXmlVal(pszList[iIndexVal]);

							gDLL->getXMLIFace()->SetToParent(m_pFXml);
						}

						if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
						{
							break;
						}
					}

					gDLL->getXMLIFace()->SetToParent(m_pFXml);
				}
			}
		}

		gDLL->getXMLIFace()->SetToParent(m_pFXml);
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetVariableListTagPair(int **ppiList, const TCHAR* szRootTagName, 
//										CvString* m_paszTagList, int iTagListLength, int iDefaultListVal)
//
//  PURPOSE :   allocate and initialize a list from a tag pair in the xml
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetVariableListTagPair(int **ppiList, const TCHAR* szRootTagName,
											  CvString* m_paszTagList, int iTagListLength, int iDefaultListVal)
{
	int i;
	int iIndexVal;
	int iNumSibs;
	TCHAR szTextVal[256];
	int* piList;

	if(!(0 < iTagListLength))
	{
		char	szMessage[1024];
		sprintf( szMessage, "Allocating zero or less memory in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	InitList(ppiList, iTagListLength, iDefaultListVal);
	if (gDLL->getXMLIFace()->SetToChildByTagName(m_pFXml,szRootTagName))
	{
		if (SkipToNextVal())
		{
			iNumSibs = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);
			piList = *ppiList;
			if (0 < iNumSibs)
			{
				if(!(iNumSibs <= iTagListLength))
				{
					char	szMessage[1024];
					sprintf( szMessage, "There are more siblings than memory allocated for them in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Error");
				}
				if (gDLL->getXMLIFace()->SetToChild(m_pFXml))
				{
					for (i=0;i<iNumSibs;i++)
					{
						if (GetChildXmlVal(szTextVal))
						{
							iIndexVal = GC.getTypesEnum(szTextVal);
							GetNextXmlVal(&piList[iIndexVal]);

							gDLL->getXMLIFace()->SetToParent(m_pFXml);
						}

						if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
						{
							break;
						}
					}

					gDLL->getXMLIFace()->SetToParent(m_pFXml);
				}
			}
		}

		gDLL->getXMLIFace()->SetToParent(m_pFXml);
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetVariableListTagPair(int **ppiList, const TCHAR* szRootTagName, 
//										CvString* m_paszTagList, int iTagListLength, int iDefaultListVal)
//
//  PURPOSE :   allocate and initialize a list from a tag pair in the xml for audio scripts
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetVariableListTagPairForAudioScripts(int **ppiList, const TCHAR* szRootTagName,
															 CvString* m_paszTagList, int iTagListLength, int iDefaultListVal)
{
	int i;
	int iIndexVal;
	int iNumSibs;
	TCHAR szTextVal[256];
	int* piList;
	CvString szTemp;

	if (gDLL->getXMLIFace()->SetToChildByTagName(m_pFXml,szRootTagName))
	{
		if (SkipToNextVal())
		{
			iNumSibs = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);
			if(!(0 < iTagListLength))
			{
				char	szMessage[1024];
				sprintf( szMessage, "Allocating zero or less memory in CvXMLLoadUtility::SetVariableListTagPairForAudio \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
				gDLL->MessageBox(szMessage, "XML Error");
			}
			InitList(ppiList, iTagListLength, iDefaultListVal);
			piList = *ppiList;
			if (0 < iNumSibs)
			{
				if(!(iNumSibs <= iTagListLength))
				{
					char	szMessage[1024];
					sprintf( szMessage, "There are more siblings than memory allocated for them in CvXMLLoadUtility::SetVariableListTagPairForAudio \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Error");
				}
				if (gDLL->getXMLIFace()->SetToChild(m_pFXml))
				{
					for (i=0;i<iNumSibs;i++)
					{
						if (GetChildXmlVal(szTextVal))
						{
							iIndexVal =	GC.getTypesEnum(szTextVal);
							GetNextXmlVal(szTemp);
							if ( szTemp.GetLength() > 0 )
								piList[iIndexVal] = gDLL->getAudioTagIndex(szTemp);
							else
								piList[iIndexVal] = -1;

							gDLL->getXMLIFace()->SetToParent(m_pFXml);
						}

						if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
						{
							break;
						}
					}

					gDLL->getXMLIFace()->SetToParent(m_pFXml);
				}
			}
		}

		gDLL->getXMLIFace()->SetToParent(m_pFXml);
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetVariableListTagPairForAudioScripts(int **ppiList, const TCHAR* szRootTagName,
//										CvInfoBase* pCvInfoBase, int iInfoBaseSize, int iInfoBaseLength, int iDefaultListVal)
//
//  PURPOSE :   allocate and initialize a list from a tag pair in the xml for audio scripts
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetVariableListTagPairForAudioScripts(int **ppiList, const TCHAR* szRootTagName,
															 CvInfoBase* pCvInfoBase, int iInfoBaseSize, int iInfoBaseLength, int iDefaultListVal)
{
	int i;
	int iIndexVal;
	int iNumSibs;
	TCHAR szTextVal[256];
	int* piList;
	CvString szTemp;

	if (gDLL->getXMLIFace()->SetToChildByTagName(m_pFXml,szRootTagName))
	{
		if (SkipToNextVal())
		{
			iNumSibs = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);
			if(!(0 < iInfoBaseLength))
			{
				char	szMessage[1024];
				sprintf( szMessage, "Allocating zero or less memory in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
				gDLL->MessageBox(szMessage, "XML Error");
			}
			InitList(ppiList, iInfoBaseLength, iDefaultListVal);
			piList = *ppiList;
			if (0 < iNumSibs)
			{
				if(!(iNumSibs <= iInfoBaseLength))
				{
					char	szMessage[1024];
					sprintf( szMessage, "There are more siblings than memory allocated for them in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Error");
				}
				if (gDLL->getXMLIFace()->SetToChild(m_pFXml))
				{
					for (i=0;i<iNumSibs;i++)
					{
						if (GetChildXmlVal(szTextVal))
						{
							iIndexVal = FindInInfoClass(szTextVal, pCvInfoBase, iInfoBaseSize, iInfoBaseLength);
							GetNextXmlVal(szTemp);
							if ( szTemp.GetLength() > 0 )
								piList[iIndexVal] = gDLL->getAudioTagIndex(szTemp);
							else
								piList[iIndexVal] = -1;

							gDLL->getXMLIFace()->SetToParent(m_pFXml);
						}

						if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
						{
							break;
						}
					}

					gDLL->getXMLIFace()->SetToParent(m_pFXml);
				}
			}
		}

		gDLL->getXMLIFace()->SetToParent(m_pFXml);
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetVariableListTagPair(bool **ppbList, const TCHAR* szRootTagName, 
//										CvString* m_paszTagList, int iTagListLength, int iDefaultListVal)
//
//  PURPOSE :   allocate and initialize a list from a tag pair in the xml
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetVariableListTagPair(bool **ppbList, const TCHAR* szRootTagName,
											  CvString* m_paszTagList, int iTagListLength, bool bDefaultListVal)
{
	int i;
	int iIndexVal;
	int iNumSibs;
	TCHAR szTextVal[256];
	bool* pbList;

	if(!(0 < iTagListLength))
	{
		char	szMessage[1024];
		sprintf( szMessage, "Allocating zero or less memory in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	InitList(ppbList, iTagListLength, bDefaultListVal);
	if (gDLL->getXMLIFace()->SetToChildByTagName(m_pFXml,szRootTagName))
	{
		if (SkipToNextVal())
		{
			iNumSibs = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);
			pbList = *ppbList;
			if (0 < iNumSibs)
			{
				if(!(iNumSibs <= iTagListLength))
				{
					char	szMessage[1024];
					sprintf( szMessage, "There are more siblings than memory allocated for them in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Error");
				}
				if (gDLL->getXMLIFace()->SetToChild(m_pFXml))
				{
					for (i=0;i<iNumSibs;i++)
					{
						if (GetChildXmlVal(szTextVal))
						{
							iIndexVal =	GC.getTypesEnum(szTextVal);	
							GetNextXmlVal(&pbList[iIndexVal]);

							gDLL->getXMLIFace()->SetToParent(m_pFXml);
						}

						if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
						{
							break;
						}
					}

					gDLL->getXMLIFace()->SetToParent(m_pFXml);
				}
			}
		}

		gDLL->getXMLIFace()->SetToParent(m_pFXml);
	}
}

//------------------------------------------------------------------------------------------------------
//
//	FUNCTION:	SetVariableListTagPair(CvString **ppszList, const TCHAR* szRootTagName,
//							CvString* m_paszTagList, int iTagListLength, CvString szDefaultListVal = "")
//
//  PURPOSE :   allocate and initialize a list from a tag pair in the xml
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetVariableListTagPair(CvString **ppszList, const TCHAR* szRootTagName,
											  CvString* m_paszTagList, int iTagListLength, CvString szDefaultListVal)
{
	int i;
	int iIndexVal;
	int iNumSibs;
	TCHAR szTextVal[256];
	CvString* pszList;

	if(!(0 < iTagListLength))
	{
		char	szMessage[1024];
		sprintf( szMessage, "Allocating zero or less memory in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	InitStringList(ppszList, iTagListLength, szDefaultListVal);
	if (gDLL->getXMLIFace()->SetToChildByTagName(m_pFXml,szRootTagName))
	{
		if (SkipToNextVal())
		{
			iNumSibs = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);
			pszList = *ppszList;
			if (0 < iNumSibs)
			{
				if(!(iNumSibs <= iTagListLength))
				{
					char	szMessage[1024];
					sprintf( szMessage, "There are more siblings than memory allocated for them in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Error");
				}
				if (gDLL->getXMLIFace()->SetToChild(m_pFXml))
				{
					for (i=0;i<iNumSibs;i++)
					{
						if (GetChildXmlVal(szTextVal))
						{
							iIndexVal =	GC.getTypesEnum(szTextVal);	
							GetNextXmlVal(pszList[iIndexVal]);

							gDLL->getXMLIFace()->SetToParent(m_pFXml);
						}

						if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
						{
							break;
						}
					}

					gDLL->getXMLIFace()->SetToParent(m_pFXml);
				}
			}
		}

		gDLL->getXMLIFace()->SetToParent(m_pFXml);
	}
}

DllExport bool CvXMLLoadUtility::LoadPlayerOptions()
{
	if (!CreateFXml())
		return false;

	bool bLoaded = LoadCivXml(m_pFXml, "GameInfo/CIV4PlayerOptionInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4PlayerOptionInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	if (bLoaded && Validate())
	{
		int iEnumVal = NUM_PLAYEROPTION_TYPES;
		SetGlobalClassInfo(&GC.getPlayerOptionInfo(), "Civ4PlayerOptionInfos/PlayerOptionInfos/PlayerOptionInfo", &iEnumVal, true);
	}
	DestroyFXml();
	return bLoaded;
}

DllExport bool CvXMLLoadUtility::LoadGraphicOptions()
{
	if (!CreateFXml())
		return false;

	bool bLoaded = LoadCivXml(m_pFXml, "GameInfo/CIV4GraphicOptionInfos.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for GameInfo/CIV4GraphicOptionInfos.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	if (bLoaded && Validate())
	{
		int iEnumVal = NUM_GRAPHICOPTION_TYPES;
		SetGlobalClassInfo(&GC.getGraphicOptionInfo(), "Civ4GraphicOptionInfos/GraphicOptionInfos/GraphicOptionInfo", &iEnumVal, true);
	}
	DestroyFXml();
	return bLoaded;
}
