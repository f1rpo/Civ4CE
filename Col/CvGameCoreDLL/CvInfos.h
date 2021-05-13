#pragma once

//  $Header:
//------------------------------------------------------------------------------------------------
//
//  FILE:    CvInfos.h
//
//  AUTHOR:	Eric MacDonald  --  8/2003
//					Mustafa Thamer 11/2004
//					Jon Shafer - 03/2005
//
//  PURPOSE: All Civ4 info classes and the base class for them
//
//------------------------------------------------------------------------------------------------
//  Copyright (c) 2003 Firaxis Games, Inc. All rights reserved.
//------------------------------------------------------------------------------------------------
#ifndef CV_INFO_H
#define CV_INFO_H

//#include "CvStructs.h"

#pragma warning( disable: 4251 )		// needs to have dll-interface to be used by clients of class
#pragma warning( disable: 4127 )

class CvXMLLoadUtility;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvInfoBase
//
//  DESC:   The base class for all info classes to inherit from.  This gives us
//			the base description and type strings
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvInfoBase
{
//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	//constructor
	DllExport CvInfoBase();
	//destructor
	DllExport virtual ~CvInfoBase();

	DllExport CvInfoBase(const CvInfoBase& kCopy);
	DllExport CvInfoBase& operator=(const CvInfoBase& kCopy);

	DllExport virtual void reset();

	DllExport bool isGraphicalOnly() const;

	DllExport const TCHAR* getType() const;
	DllExport virtual const TCHAR* getButton() const;
	DllExport const TCHAR* getTextKey() const;
	DllExport const char* getXmlVal() const;  // the name of this XML node

	// for python wide string handling
	std::wstring pyGetTextKey() { return getTextKeyWide(); }
	std::wstring pyGetDescription() { return getDescription(0); }
	std::wstring pyGetDescriptionForm(uint uiForm) { return getDescription(uiForm); }
	std::wstring pyGetText() { return getText(); }
	std::wstring pyGetCivilopedia() { return getCivilopedia(); }
	std::wstring pyGetHelp() { return getHelp(); }
	std::wstring pyGetStrategy() { return getStrategy(); }

	DllExport const wchar* getTextKeyWide() const;
	DllExport const wchar* getDescription(uint uiForm = 0) const;
	DllExport const wchar* getText() const;
	DllExport const wchar* getCivilopedia() const;
	DllExport const wchar* getHelp() const;
	DllExport const wchar* getStrategy() const;

	DllExport void setType(const TCHAR* szVal);
	DllExport void setButton(const TCHAR* szVal);
	DllExport void setTextKey(const TCHAR* szVal);
	DllExport void setCivilopediaKey(const TCHAR* szVal);
	DllExport void setHelpKey(const TCHAR* szVal);
	DllExport void setStrategyKey(const TCHAR* szVal);
	DllExport void setXmlVal(const char* szVal);

	DllExport bool isMatchForLink(std::wstring szLink, bool bKeysOnly) const;

	DllExport virtual void read(FDataStreamBase* pStream);
	DllExport virtual void write(FDataStreamBase* pStream);

	DllExport virtual bool read(CvXMLLoadUtility* pXML);
	DllExport virtual bool readPass2(CvXMLLoadUtility* pXML) { pXML; FAssertMsg(false, "Override this"); return false; }
	DllExport virtual bool readPass3() { FAssertMsg(false, "Override this"); return false; }

protected:

	bool doneReadingXML(CvXMLLoadUtility* pXML);
	void setTempText(const wchar* szValue) const { m_szTempText = szValue;	}
	void copy(const CvInfoBase& kCopy);

	bool m_bGraphicalOnly;

	char* m_szType;
	char* m_szButton;				// Used for Infos that don't require an ArtAssetInfo
	char* m_szTextKey;
	mutable wchar* m_szCachedTextKey;
	mutable wchar* m_szCachedText;
	char* m_szCivilopediaKey;
	mutable wchar* m_szCachedCivilopedia;
	char* m_szHelpKey;
	mutable wchar* m_szCachedHelp;
	char* m_szStrategyKey;
	mutable wchar* m_szCachedStrategy;
	char* m_szXmlVal;

	// translated text
	mutable CvWString m_szTempText;
	std::vector<CvString> m_aszExtraXMLforPass3;
	mutable std::vector<CvWString> m_aCachedDescriptions;
};

//
// holds the scale for scalable objects
//
class CvScalableInfo
{
public:

	DllExport CvScalableInfo() : m_fScale(1.0f), m_fInterfaceScale(1.0f) { }

	DllExport float getScale() const;
	DllExport void setScale(float fScale);

	DllExport float getInterfaceScale() const;
	DllExport void setInterfaceScale(float fInterfaceScale);

	DllExport bool read(CvXMLLoadUtility* pXML);

protected:

	float m_fScale;
	float m_fInterfaceScale;	//!< the scale of the unit appearing in the interface screens
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CLASS:      CvHotkeyInfo
//!  \brief			holds the hotkey info for an info class
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvHotkeyInfo : public CvInfoBase
{
public:
	//constructor
	DllExport CvHotkeyInfo();
	//destructor
	DllExport virtual ~CvHotkeyInfo();

	DllExport bool read(CvXMLLoadUtility* pXML);

	DllExport virtual void read(FDataStreamBase* pStream);
	DllExport virtual void write(FDataStreamBase* pStream);

	DllExport int getActionInfoIndex() const;
	DllExport void setActionInfoIndex(int i);

	DllExport int getHotKeyVal() const;
	DllExport void setHotKeyVal(int i);
	DllExport int getHotKeyPriority() const;
	DllExport void setHotKeyPriority(int i);
	DllExport int getHotKeyValAlt() const;
	DllExport void setHotKeyValAlt(int i);
	DllExport int getHotKeyPriorityAlt() const;
	DllExport void setHotKeyPriorityAlt(int i);
	DllExport int getOrderPriority() const;
	DllExport void setOrderPriority(int i);

	DllExport bool isAltDown() const;
	DllExport void setAltDown(bool b);
	DllExport bool isShiftDown() const;
	DllExport void setShiftDown(bool b);
	DllExport bool isCtrlDown() const;
	DllExport void setCtrlDown(bool b);
	DllExport bool isAltDownAlt() const;
	DllExport void setAltDownAlt(bool b);
	DllExport bool isShiftDownAlt() const;
	DllExport void setShiftDownAlt(bool b);
	DllExport bool isCtrlDownAlt() const;
	DllExport void setCtrlDownAlt(bool b);

	DllExport const TCHAR* getHotKey() const;
	DllExport void setHotKey(const TCHAR* szVal);

	DllExport std::wstring getHotKeyDescription() const;
	DllExport void setHotKeyDescription(const wchar* szHotKeyDescKey, const wchar* szHotKeyAltDescKey, const wchar* szHotKeyString);

protected:

	int m_iActionInfoIndex;

	int m_iHotKeyVal;
	int m_iHotKeyPriority;
	int m_iHotKeyValAlt;
	int m_iHotKeyPriorityAlt;
	int m_iOrderPriority;

	bool m_bAltDown;
	bool m_bShiftDown;
	bool m_bCtrlDown;
	bool m_bAltDownAlt;
	bool m_bShiftDownAlt;
	bool m_bCtrlDownAlt;

	CvString m_szHotKey;
	CvWString m_szHotKeyDescriptionKey;
	CvWString m_szHotKeyAltDescriptionKey;
	CvWString m_szHotKeyString;

};

// JON - XXX TODO, expose to python or remove (used by CvDiplomacyInfo.getResponse() )
class CvDiplomacyResponse
{
//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvDiplomacyResponse();
	DllExport virtual ~CvDiplomacyResponse();

	DllExport int getNumDiplomacyText();
	DllExport void setNumDiplomacyText(int i);

	DllExport bool getCivilizationTypes(int i);
	DllExport bool* getCivilizationTypes() const;
	DllExport void setCivilizationTypes(int i, bool bVal);

	DllExport bool getLeaderHeadTypes(int i);
	DllExport bool* getLeaderHeadTypes() const;
	DllExport void setLeaderHeadTypes(int i, bool bVal);

	DllExport bool getAttitudeTypes(int i) const;
	DllExport bool* getAttitudeTypes() const;
	DllExport void setAttitudeTypes(int i, bool bVal);

	DllExport bool getDiplomacyPowerTypes(int i);
	DllExport bool* getDiplomacyPowerTypes() const;
	DllExport void setDiplomacyPowerTypes(int i, bool bVal);

	DllExport const TCHAR* getDiplomacyText(int i) const;
	DllExport const CvString* getDiplomacyText() const;
	DllExport void setDiplomacyText(int i, CvString szText);

	DllExport void read(FDataStreamBase* stream);
	DllExport void write(FDataStreamBase* stream);
	DllExport bool read(CvXMLLoadUtility* pXML);

protected:
	int m_iNumDiplomacyText;
	bool* m_pbCivilizationTypes;
	bool* m_pbLeaderHeadTypes;
	bool* m_pbAttitudeTypes;
	bool* m_pbDiplomacyPowerTypes;
	CvString* m_paszDiplomacyText;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvSpecialistInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvSpecialistInfo : public CvHotkeyInfo
{
//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvSpecialistInfo();
	DllExport virtual ~CvSpecialistInfo();

	DllExport int getGreatPeopleUnitClass() const;
	DllExport int getGreatPeopleRateChange() const;
	DllExport int getMissionType() const;
	DllExport void setMissionType(int iNewType);
	DllExport int getExperience() const;

	DllExport bool isVisible() const;

	// Arrays

	DllExport int getYieldChange(int i) const;
	DllExport const int* getYieldChangeArray() const;		 - For Moose - CvWidgetData
	DllExport int getCommerceChange(int i) const;
	DllExport int getFlavorValue(int i) const;

	DllExport const TCHAR* getTexture() const;
	DllExport void setTexture(const TCHAR* szVal);

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------

protected:

	int m_iGreatPeopleUnitClass;
	int m_iGreatPeopleRateChange;
	int m_iMissionType;
	int m_iExperience;

	bool m_bVisible;

	CvString m_szTexture;

	// Arrays

	int* m_piYieldChange;
	int* m_piCommerceChange;
	int* m_piFlavorValue;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvTechInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvTechInfo : public CvInfoBase
{

friend class CvXMLLoadUtility;

//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvTechInfo();
	DllExport virtual ~CvTechInfo();

	DllExport int getAdvisorType() const;
	DllExport int getAIWeight() const;
	DllExport int getAITradeModifier() const;
	DllExport int getResearchCost() const;
	DllExport int getAdvancedStartCost() const;
	DllExport int getAdvancedStartCostIncrease() const;
	DllExport int getEra() const;
	DllExport int getTradeRoutes() const;
	DllExport int getFeatureProductionModifier() const;
	DllExport int getWorkerSpeedModifier() const;
	DllExport int getFirstFreeUnitClass() const;
	DllExport int getHealth() const;
	DllExport int getHappiness() const;
	DllExport int getFirstFreeTechs() const;
	DllExport int getAssetValue() const;
	DllExport int getPowerValue() const;

	DllExport int getGridX() const;
	DllExport int getGridY() const;

	DllExport bool isRepeat() const;
	DllExport bool isTrade() const;
	DllExport bool isDisable() const;
	DllExport bool isGoodyTech() const;
	DllExport bool isExtraWaterSeeFrom() const;
	DllExport bool isMapCentering() const;
	DllExport bool isMapVisible() const;
	DllExport bool isMapTrading() const;
	DllExport bool isTechTrading() const;
	DllExport bool isGoldTrading() const;
	DllExport bool isOpenBordersTrading() const;
	DllExport bool isDefensivePactTrading() const;
	DllExport bool isPermanentAllianceTrading() const;
	DllExport bool isVassalStateTrading() const;
	DllExport bool isBridgeBuilding() const;
	DllExport bool isIrrigation() const;
	DllExport bool isIgnoreIrrigation() const;
	DllExport bool isWaterWork() const;
	DllExport bool isRiverTrade() const;

	std::wstring pyGetQuote() { return getQuote(); }
	DllExport const wchar* getQuote();
	DllExport void setQuoteKey(const TCHAR* szVal);
	DllExport const TCHAR* getSound() const;
	DllExport void setSound(const TCHAR* szVal);
	DllExport const TCHAR* getSoundMP() const;
	DllExport void setSoundMP(const TCHAR* szVal);

	// Arrays

	DllExport int getDomainExtraMoves(int i) const;
	DllExport int getFlavorValue(int i) const;
	DllExport int getPrereqOrTechs(int i) const;
	DllExport int getPrereqAndTechs(int i) const;

	DllExport bool isCommerceFlexible(int i) const;
	DllExport bool isTerrainTrade(int i) const;

	DllExport void read(FDataStreamBase* );
	DllExport void write(FDataStreamBase* );

	DllExport bool read(CvXMLLoadUtility* pXML);
	DllExport bool readPass2(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------

protected:

	int m_iAdvisorType;
	int m_iAIWeight;
	int m_iAITradeModifier;
	int m_iResearchCost;
	int m_iAdvancedStartCost;
	int m_iAdvancedStartCostIncrease;
	int m_iEra;
	int m_iTradeRoutes;
	int m_iFeatureProductionModifier;
	int m_iWorkerSpeedModifier;
	int m_iFirstFreeUnitClass;
	int m_iHealth;
	int m_iHappiness;
	int m_iFirstFreeTechs;
	int m_iAssetValue;
	int m_iPowerValue;

	int m_iGridX;
	int m_iGridY;

	bool m_bRepeat;
	bool m_bTrade;
	bool m_bDisable;
	bool m_bGoodyTech;
	bool m_bExtraWaterSeeFrom;
	bool m_bMapCentering;
	bool m_bMapVisible;
	bool m_bMapTrading;
	bool m_bTechTrading;
	bool m_bGoldTrading;
	bool m_bOpenBordersTrading;
	bool m_bDefensivePactTrading;
	bool m_bPermanentAllianceTrading;
	bool m_bVassalStateTrading;
	bool m_bBridgeBuilding;
	bool m_bIrrigation;
	bool m_bIgnoreIrrigation;
	bool m_bWaterWork;
	bool m_bRiverTrade;

	CvString m_szQuoteKey;
	CvString m_szSound;
	CvString m_szSoundMP;

	// Arrays

	int* m_piDomainExtraMoves;
	int* m_piFlavorValue;

	int* m_piPrereqOrTechs;
	int* m_piPrereqAndTechs;

	bool* m_pbCommerceFlexible;
	bool* m_pbTerrainTrade;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvPromotionInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvPromotionInfo :	public CvHotkeyInfo
{
//---------------------------------------PUBLIC INTERFACE---------------------------------
public:
	DllExport CvPromotionInfo();
	DllExport virtual ~CvPromotionInfo();

	DllExport int getLayerAnimationPath() const;
	DllExport int getPrereqPromotion() const;
	DllExport void setPrereqPromotion(int i);
	DllExport int getPrereqOrPromotion1() const;
	DllExport void setPrereqOrPromotion1(int i);
	DllExport int getPrereqOrPromotion2() const;
	DllExport void setPrereqOrPromotion2(int i);

	DllExport int getTechPrereq() const;
	DllExport int getStateReligionPrereq() const;
	DllExport int getVisibilityChange() const;
	DllExport int getMovesChange() const;
	DllExport int getMoveDiscountChange() const;
	DllExport int getAirRangeChange() const;
	DllExport int getInterceptChange() const;
	DllExport int getEvasionChange() const;
	DllExport int getWithdrawalChange() const;
	DllExport int getCargoChange() const;
	DllExport int getCollateralDamageChange() const;
	DllExport int getBombardRateChange() const;
	DllExport int getFirstStrikesChange() const;
	DllExport int getChanceFirstStrikesChange() const;
	DllExport int getEnemyHealChange() const;
	DllExport int getNeutralHealChange() const;
	DllExport int getFriendlyHealChange() const;
	DllExport int getSameTileHealChange() const;
	DllExport int getAdjacentTileHealChange() const;
	DllExport int getCombatPercent() const;
	DllExport int getCityAttackPercent() const;
	DllExport int getCityDefensePercent() const;
	DllExport int getHillsAttackPercent() const;
	DllExport int getHillsDefensePercent() const;
	DllExport int getCommandType() const;
	DllExport void setCommandType(int iNewType);

	DllExport int getRevoltProtection() const;
	DllExport int getCollateralDamageProtection() const;
	DllExport int getPillageChange() const;
	DllExport int getUpgradeDiscount() const;
	DllExport int getExperiencePercent() const;
	DllExport int getKamikazePercent() const;

	DllExport bool isLeader() const;
	DllExport bool isBlitz() const;
	DllExport bool isAmphib() const;
	DllExport bool isRiver() const;
	DllExport bool isEnemyRoute() const;
	DllExport bool isAlwaysHeal() const;
	DllExport bool isHillsDoubleMove() const;
	DllExport bool isImmuneToFirstStrikes() const;

	DllExport const TCHAR* getSound() const;
	DllExport void setSound(const TCHAR* szVal);

	// Arrays

	DllExport int getTerrainAttackPercent(int i) const;
	DllExport int getTerrainDefensePercent(int i) const;
	DllExport int getFeatureAttackPercent(int i) const;
	DllExport int getFeatureDefensePercent(int i) const;
	DllExport int getUnitCombatModifierPercent(int i) const;
	DllExport int getDomainModifierPercent(int i) const;

	DllExport bool getTerrainDoubleMove(int i) const;
	DllExport bool getFeatureDoubleMove(int i) const;
	DllExport bool getUnitCombat(int i) const;

	DllExport void read(FDataStreamBase* stream);
	DllExport void write(FDataStreamBase* stream);

	DllExport bool read(CvXMLLoadUtility* pXML);
	DllExport bool readPass2(CvXMLLoadUtility* pXML);

//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------

protected:

	int m_iLayerAnimationPath;
	int m_iPrereqPromotion;
	int m_iPrereqOrPromotion1;
	int m_iPrereqOrPromotion2;

	int m_iTechPrereq;
	int m_iStateReligionPrereq;
	int m_iVisibilityChange;
	int m_iMovesChange;
	int m_iMoveDiscountChange;
	int m_iAirRangeChange;
	int m_iInterceptChange;
	int m_iEvasionChange;
	int m_iWithdrawalChange;
	int m_iCargoChange;
	int m_iCollateralDamageChange;
	int m_iBombardRateChange;
	int m_iFirstStrikesChange;
	int m_iChanceFirstStrikesChange;
	int m_iEnemyHealChange;
	int m_iNeutralHealChange;
	int m_iFriendlyHealChange;
	int m_iSameTileHealChange;
	int m_iAdjacentTileHealChange;
	int m_iCombatPercent;
	int m_iCityAttackPercent;
	int m_iCityDefensePercent;
	int m_iHillsAttackPercent;
	int m_iHillsDefensePercent;
	int m_iCommandType;
	int m_iRevoltProtection;
	int m_iCollateralDamageProtection;
	int m_iPillageChange;
	int m_iUpgradeDiscount;
	int m_iExperiencePercent;
	int m_iKamikazePercent;

	bool m_bLeader;
	bool m_bBlitz;
	bool m_bAmphib;
	bool m_bRiver;
	bool m_bEnemyRoute;
	bool m_bAlwaysHeal;
	bool m_bHillsDoubleMove;
	bool m_bImmuneToFirstStrikes;

	CvString m_szSound;

	// Arrays

	int* m_piTerrainAttackPercent;
	int* m_piTerrainDefensePercent;
	int* m_piFeatureAttackPercent;
	int* m_piFeatureDefensePercent;
	int* m_piUnitCombatModifierPercent;
	int* m_piDomainModifierPercent;

	bool* m_pbTerrainDoubleMove;
	bool* m_pbFeatureDoubleMove;
	bool* m_pbUnitCombat;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvMissionInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvMissionInfo : public CvHotkeyInfo
{
//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvMissionInfo();
	DllExport virtual ~CvMissionInfo();

	DllExport int getTime() const;

	DllExport bool isSound() const;
	DllExport bool isTarget() const;
	DllExport bool isBuild() const;
	DllExport bool getVisible() const;
	DllExport EntityEventTypes getEntityEvent() const;

	DllExport const TCHAR* getWaypoint() const;		// effect type, Exposed to Python

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------

protected:

	int m_iTime;

	bool m_bSound;
	bool m_bTarget;
	bool m_bBuild;
	bool m_bVisible;
	EntityEventTypes m_eEntityEvent;

	CvString m_szWaypoint;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvControlInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvControlInfo : public CvHotkeyInfo
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvControlInfo();
	DllExport virtual ~CvControlInfo();

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------

protected:
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvCommandInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvCommandInfo : public CvHotkeyInfo
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvCommandInfo();
	DllExport virtual ~CvCommandInfo();

	DllExport int getAutomate() const;
	DllExport void setAutomate(int i);

	DllExport bool getConfirmCommand() const;
	DllExport bool getVisible() const;
	DllExport bool getAll() const;

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------

protected:
	int m_iAutomate;

	bool m_bConfirmCommand;
	bool m_bVisible;
	bool m_bAll;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvAutomateInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvAutomateInfo : public CvHotkeyInfo
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvAutomateInfo();
	DllExport virtual ~CvAutomateInfo();

	DllExport int getCommand() const;
	DllExport void setCommand(int i);
	DllExport int getAutomate() const;
	DllExport void setAutomate(int i);

	DllExport bool getConfirmCommand() const;
	DllExport void setConfirmCommand(bool bVal);
	DllExport bool getVisible() const;
	DllExport void setVisible(bool bVal);

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------

protected:
	int m_iCommand;
	int m_iAutomate;

	bool m_bConfirmCommand;
	bool m_bVisible;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvActionInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvActionInfo
{
//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvActionInfo();
	DllExport virtual ~CvActionInfo();

	DllExport int getMissionData() const;
	DllExport int getCommandData() const;

	DllExport int getAutomateType() const;
	DllExport int getInterfaceModeType() const;
	DllExport int getMissionType() const;
	DllExport int getCommandType() const;
	DllExport int getControlType() const;
	DllExport int getOriginalIndex() const;
	DllExport void setOriginalIndex(int i);

	DllExport bool isConfirmCommand() const;
	DllExport bool isVisible() const;
	DllExport ActionSubTypes getSubType() const;
	DllExport void setSubType(ActionSubTypes eSubType);

	// functions to replace the CvInfoBase calls
	DllExport const TCHAR* getType() const;
	DllExport const wchar* getDescription() const;
	DllExport const wchar* getCivilopedia() const;
	DllExport const wchar* getHelp() const;
	DllExport const wchar* getStrategy() const;
	DllExport virtual const TCHAR* getButton() const;
	DllExport const TCHAR* getTextKey() const;
	DllExport const wchar* getTextKeyWide() const;

	// functions to replace the CvHotkey calls
	DllExport int getActionInfoIndex() const;
	DllExport int getHotKeyVal() const;
	DllExport int getHotKeyPriority() const;
	DllExport int getHotKeyValAlt() const;
	DllExport int getHotKeyPriorityAlt() const;
	DllExport int getOrderPriority() const;

	DllExport bool isAltDown() const;
	DllExport bool isShiftDown() const;
	DllExport bool isCtrlDown() const;
	DllExport bool isAltDownAlt() const;
	DllExport bool isShiftDownAlt() const;
	DllExport bool isCtrlDownAlt() const;

	DllExport const TCHAR* getHotKey() const;

	DllExport std::wstring getHotKeyDescription() const;

//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------

protected:

	int m_iOriginalIndex;
	ActionSubTypes m_eSubType;

private:
	CvHotkeyInfo* getHotkeyInfo() const;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvUnitInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvArtInfoUnit;
class CvUnitInfo : public CvHotkeyInfo
{
//---------------------------------------PUBLIC INTERFACE---------------------------------
public:
	DllExport CvUnitInfo();
	DllExport virtual ~CvUnitInfo();

	DllExport int getAIWeight() const;
	DllExport int getProductionCost() const;
	DllExport int getHurryCostModifier() const;
	DllExport int getAdvancedStartCost() const;
	DllExport int getAdvancedStartCostIncrease() const;
	DllExport int getMinAreaSize() const;
	DllExport int getMoves() const;
	DllExport int getAirRange() const;
	DllExport int getAirUnitCap() const;
	DllExport int getDropRange() const;
	DllExport int getNukeRange() const;
	DllExport int getWorkRate() const;
	DllExport int getBaseDiscover() const;
	DllExport int getDiscoverMultiplier() const;
	DllExport int getBaseHurry() const;
	DllExport int getHurryMultiplier() const;
	DllExport int getBaseTrade() const;
	DllExport int getTradeMultiplier() const;
	DllExport int getGreatWorkCulture() const;
	DllExport int getEspionagePoints() const;
	DllExport int getCombat() const;
	DllExport void setCombat(int iNum);
	DllExport int getCombatLimit() const;
	DllExport int getAirCombat() const;
	DllExport int getAirCombatLimit() const;
	DllExport int getXPValueAttack() const;
	DllExport int getXPValueDefense() const;
	DllExport int getFirstStrikes() const;
	DllExport int getChanceFirstStrikes() const;
	DllExport int getInterceptionProbability() const;
	DllExport int getEvasionProbability() const;
	DllExport int getWithdrawalProbability() const;
	DllExport int getCollateralDamage() const;
	DllExport int getCollateralDamageLimit() const;
	DllExport int getCollateralDamageMaxUnits() const;
	DllExport int getCityAttackModifier() const;
	DllExport int getCityDefenseModifier() const;
	DllExport int getAnimalCombatModifier() const;
	DllExport int getHillsAttackModifier() const;
	DllExport int getHillsDefenseModifier() const;
	DllExport int getBombRate() const;
	DllExport int getBombardRate() const;
	DllExport int getSpecialCargo() const;
	DllExport int getDomainCargo() const;

	DllExport int getCargoSpace() const;
	DllExport int getConscriptionValue() const;
	DllExport int getCultureGarrisonValue() const;
	DllExport int getExtraCost() const;
	DllExport int getAssetValue() const;
	DllExport int getPowerValue() const;
	DllExport int getUnitClassType() const;
	DllExport int getSpecialUnitType() const;
	DllExport int getUnitCaptureClassType() const;
	DllExport int getUnitCombatType() const;
	DllExport int getDomainType() const;
	DllExport int getDefaultUnitAIType() const;
	DllExport int getInvisibleType() const;
	DllExport int getSeeInvisibleType(int i) const;
	DllExport int getNumSeeInvisibleTypes() const;
	DllExport int getAdvisorType() const;
	DllExport int getHolyCity() const;
	DllExport int getReligionType() const;
	DllExport int getStateReligion() const;
	DllExport int getPrereqReligion() const;
	DllExport int getPrereqCorporation() const;
	DllExport int getPrereqBuilding() const;
	DllExport int getPrereqAndTech() const;
	DllExport int getPrereqAndBonus() const;
	DllExport int getGroupSize() const;									 - the initial number of individuals in the unit group
	DllExport int getGroupDefinitions() const;					 - the number of UnitMeshGroups for this unit
	DllExport int getMeleeWaveSize() const;
	DllExport int getRangedWaveSize() const;
	DllExport int getNumUnitNames() const;
	DllExport int getCommandType() const;
	DllExport void setCommandType(int iNewType);

	DllExport bool isAnimal() const;
	DllExport bool isFoodProduction() const;
	DllExport bool isNoBadGoodies() const;
	DllExport bool isOnlyDefensive() const;
	DllExport bool isNoCapture() const;
	DllExport bool isQuickCombat() const;
	DllExport bool isRivalTerritory() const;
	DllExport bool isMilitaryHappiness() const;
	DllExport bool isMilitarySupport() const;
	DllExport bool isMilitaryProduction() const;
	DllExport bool isPillage() const;
	DllExport bool isSpy() const;
	DllExport bool isSabotage() const;
	DllExport bool isDestroy() const;
	DllExport bool isStealPlans() const;
	DllExport bool isInvestigate() const;
	DllExport bool isCounterSpy() const;
	DllExport bool isFound() const;
	DllExport bool isGoldenAge() const;
	DllExport bool isInvisible() const;
	DllExport void setInvisible(bool bEnable) ;
	DllExport bool isFirstStrikeImmune() const;
	DllExport bool isNoDefensiveBonus() const;
	DllExport bool isIgnoreBuildingDefense() const;
	DllExport bool isCanMoveImpassable() const;
	DllExport bool isCanMoveAllTerrain() const;
	DllExport bool isFlatMovementCost() const;
	DllExport bool isIgnoreTerrainCost() const;
	DllExport bool isNukeImmune() const;
	DllExport bool isPrereqBonuses() const;
	DllExport bool isPrereqReligion() const;
	DllExport bool isMechUnit() const;
	DllExport bool isRenderBelowWater() const;
	DllExport bool isRenderAlways() const;
	DllExport bool isSuicide() const;
	DllExport bool isLineOfSight() const;
	DllExport bool isHiddenNationality() const;
	DllExport bool isAlwaysHostile() const;
	DllExport bool isNoRevealMap() const;

	DllExport float getUnitMaxSpeed() const;
	DllExport float getUnitPadTime() const;

	// Arrays

	DllExport int getPrereqAndTechs(int i) const;
	DllExport int getPrereqOrBonuses(int i) const;
	DllExport int getProductionTraits(int i) const;
	DllExport int getFlavorValue(int i) const;
	DllExport int getTerrainAttackModifier(int i) const;
	DllExport int getTerrainDefenseModifier(int i) const;
	DllExport int getFeatureAttackModifier(int i) const;
	DllExport int getFeatureDefenseModifier(int i) const;
	DllExport int getUnitClassAttackModifier(int i) const;
	DllExport int getUnitClassDefenseModifier(int i) const;
	DllExport int getUnitCombatModifier(int i) const;
	DllExport int getUnitCombatCollateralImmune(int i) const;
	DllExport int getDomainModifier(int i) const;
	DllExport int getBonusProductionModifier(int i) const;
	DllExport int getUnitGroupRequired(int i) const;
	DllExport int getReligionSpreads(int i) const;
	DllExport int getCorporationSpreads(int i) const;
	DllExport int getTerrainPassableTech(int i) const;
	DllExport int getFeaturePassableTech(int i) const;
	DllExport int getFlankingStrikeUnitClass(int i) const;

	DllExport bool getUpgradeUnitClass(int i) const;
	DllExport bool getTargetUnitClass(int i) const;
	DllExport bool getTargetUnitCombat(int i) const;
	DllExport bool getDefenderUnitClass(int i) const;
	DllExport bool getDefenderUnitCombat(int i) const;
	DllExport bool getUnitAIType(int i) const;
	DllExport bool getNotUnitAIType(int i) const;
	DllExport bool getBuilds(int i) const;
	DllExport bool getGreatPeoples(int i) const;
	DllExport bool getBuildings(int i) const;
	DllExport bool getForceBuildings(int i) const;
	DllExport bool getTerrainImpassable(int i) const;
	DllExport bool getFeatureImpassable(int i) const;
	DllExport bool getTerrainNative(int i) const;
	DllExport bool getFeatureNative(int i) const;
	DllExport bool getFreePromotions(int i) const;
	DllExport int getLeaderPromotion() const;
	DllExport int getLeaderExperience() const;

	DllExport const TCHAR* getEarlyArtDefineTag(int i, UnitArtStyleTypes eStyle) const;
	DllExport void setEarlyArtDefineTag(int i, const TCHAR* szVal);
	DllExport const TCHAR* getLateArtDefineTag(int i, UnitArtStyleTypes eStyle) const;
	DllExport void setLateArtDefineTag(int i, const TCHAR* szVal);
	DllExport const TCHAR* getMiddleArtDefineTag(int i, UnitArtStyleTypes eStyle) const;
	DllExport void setMiddleArtDefineTag(int i, const TCHAR* szVal);
	DllExport const TCHAR* getUnitNames(int i) const;
	DllExport const TCHAR* getFormationType() const;

	DllExport const TCHAR* getButton() const;
	DllExport void updateArtDefineButton();

	DllExport const CvArtInfoUnit* getArtInfo(int i, EraTypes eEra, UnitArtStyleTypes eStyle) const;

	DllExport void read(FDataStreamBase* );
	DllExport void write(FDataStreamBase* );

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------

protected:

	int m_iAIWeight;
	int m_iProductionCost;
	int m_iHurryCostModifier;
	int m_iAdvancedStartCost;
	int m_iAdvancedStartCostIncrease;
	int m_iMinAreaSize;
	int m_iMoves;
	int m_iAirRange;
	int m_iAirUnitCap;
	int m_iDropRange;
	int m_iNukeRange;
	int m_iWorkRate;
	int m_iBaseDiscover;
	int m_iDiscoverMultiplier;
	int m_iBaseHurry;
	int m_iHurryMultiplier;
	int m_iBaseTrade;
	int m_iTradeMultiplier;
	int m_iGreatWorkCulture;
	int m_iEspionagePoints;
	int m_iCombat;
	int m_iCombatLimit;
	int m_iAirCombat;
	int m_iAirCombatLimit;
	int m_iXPValueAttack;
	int m_iXPValueDefense;
	int m_iFirstStrikes;
	int m_iChanceFirstStrikes;
	int m_iInterceptionProbability;
	int m_iEvasionProbability;
	int m_iWithdrawalProbability;
	int m_iCollateralDamage;
	int m_iCollateralDamageLimit;
	int m_iCollateralDamageMaxUnits;
	int m_iCityAttackModifier;
	int m_iCityDefenseModifier;
	int m_iAnimalCombatModifier;
	int m_iHillsAttackModifier;
	int m_iHillsDefenseModifier;
	int m_iBombRate;
	int m_iBombardRate;
	int m_iSpecialCargo;

	int m_iDomainCargo;
	int m_iCargoSpace;
	int m_iConscriptionValue;
	int m_iCultureGarrisonValue;
	int m_iExtraCost;
	int m_iAssetValue;
	int m_iPowerValue;
	int m_iUnitClassType;
	int m_iSpecialUnitType;
	int m_iUnitCaptureClassType;
	int m_iUnitCombatType;
	int m_iDomainType;
	int m_iDefaultUnitAIType;
	int m_iInvisibleType;
	int m_iAdvisorType;
	int m_iHolyCity;
	int m_iReligionType;
	int m_iStateReligion;
	int m_iPrereqReligion;
	int m_iPrereqCorporation;
	int m_iPrereqBuilding;
	int m_iPrereqAndTech;
	int m_iPrereqAndBonus;
	int m_iGroupSize;
	int m_iGroupDefinitions;
	int m_iUnitMeleeWaveSize;
	int m_iUnitRangedWaveSize;
	int m_iNumUnitNames;
	int m_iCommandType;
	int m_iLeaderExperience;

	bool m_bAnimal;
	bool m_bFoodProduction;
	bool m_bNoBadGoodies;
	bool m_bOnlyDefensive;
	bool m_bNoCapture;
	bool m_bQuickCombat;
	bool m_bRivalTerritory;
	bool m_bMilitaryHappiness;
	bool m_bMilitarySupport;
	bool m_bMilitaryProduction;
	bool m_bPillage;
	bool m_bSpy;
	bool m_bSabotage;
	bool m_bDestroy;
	bool m_bStealPlans;
	bool m_bInvestigate;
	bool m_bCounterSpy;
	bool m_bFound;
	bool m_bGoldenAge;
	bool m_bInvisible;
	bool m_bFirstStrikeImmune;
	bool m_bNoDefensiveBonus;
	bool m_bIgnoreBuildingDefense;
	bool m_bCanMoveImpassable;
	bool m_bCanMoveAllTerrain;
	bool m_bFlatMovementCost;
	bool m_bIgnoreTerrainCost;
	bool m_bNukeImmune;
	bool m_bPrereqBonuses;
	bool m_bPrereqReligion;
	bool m_bMechanized;
	bool m_bRenderBelowWater;
	bool m_bRenderAlways;
	bool m_bSuicide;
	bool m_bLineOfSight;
	bool m_bHiddenNationality;
	bool m_bAlwaysHostile;
	bool m_bNoRevealMap;
	int m_iLeaderPromotion;

	float m_fUnitMaxSpeed;
	float m_fUnitPadTime;

	// Arrays

	int* m_piPrereqAndTechs;
	int* m_piPrereqOrBonuses;
	int* m_piProductionTraits;
	int* m_piFlavorValue;
	int* m_piTerrainAttackModifier;
	int* m_piTerrainDefenseModifier;
	int* m_piFeatureAttackModifier;
	int* m_piFeatureDefenseModifier;
	int* m_piUnitClassAttackModifier;
	int* m_piUnitClassDefenseModifier;
	int* m_piUnitCombatModifier;
	int* m_piUnitCombatCollateralImmune;
	int* m_piDomainModifier;
	int* m_piBonusProductionModifier;
	int* m_piUnitGroupRequired;
	int* m_piReligionSpreads;
	int* m_piCorporationSpreads;
	int* m_piTerrainPassableTech;
	int* m_piFeaturePassableTech;
	int* m_piFlankingStrikeUnitClass;

	bool* m_pbUpgradeUnitClass;
	bool* m_pbTargetUnitClass;
	bool* m_pbTargetUnitCombat;
	bool* m_pbDefenderUnitClass;
	bool* m_pbDefenderUnitCombat;
	bool* m_pbUnitAIType;
	bool* m_pbNotUnitAIType;
	bool* m_pbBuilds;
	bool* m_pbGreatPeoples;
	bool* m_pbBuildings;
	bool* m_pbForceBuildings;
	bool* m_pbTerrainNative;
	bool* m_pbFeatureNative;
	bool* m_pbTerrainImpassable;
	bool* m_pbFeatureImpassable;
	bool* m_pbFreePromotions;

	CvString* m_paszEarlyArtDefineTags;
	CvString* m_paszLateArtDefineTags;
	CvString* m_paszMiddleArtDefineTags;
	CvString* m_paszUnitNames;
	CvString m_szFormationType;
	CvString m_szArtDefineButton;

	std::vector<int> m_aiSeeInvisibleTypes;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// class	: CvUnitFormationInfo
//
// \brief	: Holds information relating to the formation of sub-units within a unit
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class CvUnitEntry
{
public:
	CvUnitEntry()
	{
	}

	CvUnitEntry(const NiPoint2 &position, float radius, float facingDirection, float facingVariance) :
	m_position(position),
	m_fRadius(radius),
	m_fFacingDirection(facingDirection),
	m_fFacingVariance(facingVariance)
	{
	}

	NiPoint2 m_position;
	float m_fRadius;
	float m_fFacingDirection;
	float m_fFacingVariance;
};

class CvUnitFormationInfo : public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvUnitFormationInfo();
	DllExport virtual ~CvUnitFormationInfo();

	DllExport const TCHAR* getFormationType() const;
	DllExport const std::vector<EntityEventTypes> & getEventTypes() const;

	DllExport int getNumUnitEntries() const;
	DllExport const CvUnitEntry &getUnitEntry(int index) const;
	DllExport void addUnitEntry(const CvUnitEntry &unitEntry);
	DllExport int getNumGreatUnitEntries() const;
	DllExport const CvUnitEntry &getGreatUnitEntry(int index) const;
	DllExport int getNumSiegeUnitEntries() const;
	DllExport const CvUnitEntry &getSiegeUnitEntry(int index) const;

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PUBLIC MEMBER VARIABLES---------------------------------

protected:

	CvString m_szFormationType;
	std::vector<EntityEventTypes>	m_vctEventTypes;		//!< The list of EntityEventTypes that this formation is intended for

	std::vector<CvUnitEntry> m_vctUnitEntries;
	std::vector<CvUnitEntry> m_vctGreatUnitEntries;
	std::vector<CvUnitEntry> m_vctSiegeUnitEntries;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvSpecialUnitInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvSpecialUnitInfo :
	public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvSpecialUnitInfo();
	DllExport virtual ~CvSpecialUnitInfo();

	DllExport bool isValid() const;
	DllExport bool isCityLoad() const;

	// Arrays

	DllExport bool isCarrierUnitAIType(int i) const;
	DllExport int getProductionTraits(int i) const;

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------

protected:

	bool m_bValid;
	bool m_bCityLoad;

	// Arrays
	bool* m_pbCarrierUnitAITypes;
	int* m_piProductionTraits;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvCivicOptionInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvCivicOptionInfo :
	public CvInfoBase
{
//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvCivicOptionInfo();
	DllExport virtual ~CvCivicOptionInfo();

	DllExport bool getTraitNoUpkeep(int i) const;

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	bool* m_pabTraitNoUpkeep;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvCivicInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvCivicInfo :
	public CvInfoBase
{
//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvCivicInfo();
	DllExport virtual ~CvCivicInfo();

	DllExport int getCivicOptionType() const;
	DllExport int getAnarchyLength() const;
	DllExport int getUpkeep() const;
	DllExport int getAIWeight() const;
	DllExport int getGreatPeopleRateModifier() const;
	DllExport int getGreatGeneralRateModifier() const;
	DllExport int getDomesticGreatGeneralRateModifier() const;
	DllExport int getStateReligionGreatPeopleRateModifier() const;
	DllExport int getDistanceMaintenanceModifier() const;
	DllExport int getNumCitiesMaintenanceModifier() const;
	DllExport int getCorporationMaintenanceModifier() const;
	DllExport int getExtraHealth() const;
	DllExport int getFreeExperience() const;
	DllExport int getWorkerSpeedModifier() const;
	DllExport int getImprovementUpgradeRateModifier() const;
	DllExport int getMilitaryProductionModifier() const;
	DllExport int getBaseFreeUnits() const;
	DllExport int getBaseFreeMilitaryUnits() const;
	DllExport int getFreeUnitsPopulationPercent() const;
	DllExport int getFreeMilitaryUnitsPopulationPercent() const;
	DllExport int getGoldPerUnit() const;
	DllExport int getGoldPerMilitaryUnit() const;
	DllExport int getHappyPerMilitaryUnit() const;
	DllExport int getLargestCityHappiness() const;
	DllExport int getWarWearinessModifier() const;
	DllExport int getFreeSpecialist() const;
	DllExport int getTradeRoutes() const;
	DllExport int getTechPrereq() const;
	DllExport int getCivicPercentAnger() const;
	DllExport int getMaxConscript() const;
	DllExport int getStateReligionHappiness() const;
	DllExport int getNonStateReligionHappiness() const;
	DllExport int getStateReligionUnitProductionModifier() const;
	DllExport int getStateReligionBuildingProductionModifier() const;
	DllExport int getStateReligionFreeExperience() const;
	DllExport int getExpInBorderModifier() const;

	DllExport bool isMilitaryFoodProduction() const;
	DllExport bool isNoUnhealthyPopulation() const;
	DllExport bool isBuildingOnlyHealthy() const;
	DllExport bool isNoForeignTrade() const;
	DllExport bool isNoCorporations() const;
	DllExport bool isNoForeignCorporations() const;
	DllExport bool isStateReligion() const;
	DllExport bool isNoNonStateReligionSpread() const;

	std::wstring pyGetWeLoveTheKing() { return getWeLoveTheKing(); }
	DllExport const wchar* getWeLoveTheKing();
	DllExport void setWeLoveTheKingKey(const TCHAR* szVal);

	// Arrays

	DllExport int getYieldModifier(int i) const;
	int* getYieldModifierArray() const;
	DllExport int getCapitalYieldModifier(int i) const;
	int* getCapitalYieldModifierArray() const;
	DllExport int getTradeYieldModifier(int i) const;
	int* getTradeYieldModifierArray() const;
	DllExport int getCommerceModifier(int i) const;
	int* getCommerceModifierArray() const;
	DllExport int getCapitalCommerceModifier(int i) const;
	int* getCapitalCommerceModifierArray() const;
	DllExport int getSpecialistExtraCommerce(int i) const;
	int* getSpecialistExtraCommerceArray() const;
	DllExport int getBuildingHappinessChanges(int i) const;
	DllExport int getBuildingHealthChanges(int i) const;
	DllExport int getFeatureHappinessChanges(int i) const;

	DllExport bool isHurry(int i) const;
	DllExport bool isSpecialBuildingNotRequired(int i) const;
	DllExport bool isSpecialistValid(int i) const;

	DllExport int getImprovementYieldChanges(int i, int j) const;

	DllExport void read(FDataStreamBase* stream);
	DllExport void write(FDataStreamBase* stream);

	DllExport bool read(CvXMLLoadUtility* pXML);

//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	int m_iCivicOptionType;
	int m_iAnarchyLength;
	int m_iUpkeep;
	int m_iAIWeight;
	int m_iGreatPeopleRateModifier;
	int m_iGreatGeneralRateModifier;
	int m_iDomesticGreatGeneralRateModifier;
	int m_iStateReligionGreatPeopleRateModifier;
	int m_iDistanceMaintenanceModifier;
	int m_iNumCitiesMaintenanceModifier;
	int m_iCorporationMaintenanceModifier;
	int m_iExtraHealth;
	int m_iFreeExperience;
	int m_iWorkerSpeedModifier;
	int m_iImprovementUpgradeRateModifier;
	int m_iMilitaryProductionModifier;
	int m_iBaseFreeUnits;
	int m_iBaseFreeMilitaryUnits;
	int m_iFreeUnitsPopulationPercent;
	int m_iFreeMilitaryUnitsPopulationPercent;
	int m_iGoldPerUnit;
	int m_iGoldPerMilitaryUnit;
	int m_iHappyPerMilitaryUnit;
	int m_iLargestCityHappiness;
	int m_iWarWearinessModifier;
	int m_iFreeSpecialist;
	int m_iTradeRoutes;
	int m_iTechPrereq;
	int m_iCivicPercentAnger;
	int m_iMaxConscript;
	int m_iStateReligionHappiness;
	int m_iNonStateReligionHappiness;
	int m_iStateReligionUnitProductionModifier;
	int m_iStateReligionBuildingProductionModifier;
	int m_iStateReligionFreeExperience;
	int m_iExpInBorderModifier;

	bool m_bMilitaryFoodProduction;
	bool m_bNoUnhealthyPopulation;
	bool m_bBuildingOnlyHealthy;
	bool m_bNoForeignTrade;
	bool m_bNoCorporations;
	bool m_bNoForeignCorporations;
	bool m_bStateReligion;
	bool m_bNoNonStateReligionSpread;

	CvString m_szWeLoveTheKingKey;

	// Arrays

	int* m_piYieldModifier;
	int* m_piCapitalYieldModifier;
	int* m_piTradeYieldModifier;
	int* m_piCommerceModifier;
	int* m_piCapitalCommerceModifier;
	int* m_piSpecialistExtraCommerce;
	int* m_paiBuildingHappinessChanges;
	int* m_paiBuildingHealthChanges;
	int* m_paiFeatureHappinessChanges;

	bool* m_pabHurry;
	bool* m_pabSpecialBuildingNotRequired;
	bool* m_pabSpecialistValid;

	int** m_ppiImprovementYieldChanges;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvDiplomacyInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvDiplomacyInfo :
	public CvInfoBase
{

	friend class CvXMLLoadUtility;		// so it can access private vars to initialize the class
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvDiplomacyInfo();
	DllExport virtual ~CvDiplomacyInfo();

	DllExport void uninit();

	DllExport const CvDiplomacyResponse& getResponse(int iNum) const;
	DllExport int getNumResponses() const;

	DllExport bool getCivilizationTypes(int i, int j) const;
	DllExport bool getLeaderHeadTypes(int i, int j) const;
	DllExport bool getAttitudeTypes(int i, int j) const;
	DllExport bool getDiplomacyPowerTypes(int i, int j) const;

	DllExport int getNumDiplomacyText(int i) const;

	DllExport const TCHAR* getDiplomacyText(int i, int j) const;

	DllExport void read(FDataStreamBase* stream);
	DllExport void write(FDataStreamBase* stream);
	DllExport bool read(CvXMLLoadUtility* pXML);

private:
	std::vector<CvDiplomacyResponse*> m_pResponses;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvUnitClassInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvUnitClassInfo :
	public CvInfoBase
{
//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvUnitClassInfo();
	DllExport virtual ~CvUnitClassInfo();

	DllExport int getMaxGlobalInstances() const;
	DllExport int getMaxTeamInstances() const;
	DllExport int getMaxPlayerInstances() const;
	DllExport int getInstanceCostModifier() const;
	DllExport int getDefaultUnitIndex() const;
	DllExport void setDefaultUnitIndex(int i);

	DllExport bool read(CvXMLLoadUtility* pXML);
	DllExport bool readPass3();

//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	int m_iMaxGlobalInstances;
	int m_iMaxTeamInstances;
	int m_iMaxPlayerInstances;
	int m_iInstanceCostModifier;
	int m_iDefaultUnitIndex;


};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvBuildingInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvArtInfoBuilding;
class CvArtInfoMovie;
class CvBuildingInfo : public CvHotkeyInfo
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvBuildingInfo();
	DllExport virtual ~CvBuildingInfo();

	DllExport int getBuildingClassType() const;
	DllExport int getVictoryPrereq() const;
	DllExport int getFreeStartEra() const;
	DllExport int getMaxStartEra() const;
	DllExport int getObsoleteTech() const;
	DllExport int getPrereqAndTech() const;
	DllExport int getNoBonus() const;
	DllExport int getPowerBonus() const;
	DllExport int getFreeBonus() const;
	DllExport int getNumFreeBonuses() const;
	DllExport int getFreeBuildingClass() const;
	DllExport void setNumFreeBuildingClass(int i);
	DllExport int getFreePromotion() const;
	DllExport int getCivicOption() const;
	DllExport int getAIWeight() const;
	DllExport int getProductionCost() const;
	DllExport int getHurryCostModifier() const;
	DllExport int getHurryAngerModifier() const;
	DllExport int getAdvancedStartCost() const;
	DllExport int getAdvancedStartCostIncrease() const;
	DllExport int getMinAreaSize() const;
	DllExport int getNumCitiesPrereq() const;
	DllExport int getNumTeamsPrereq() const;
	DllExport int getUnitLevelPrereq() const;
	DllExport int getMinLatitude() const;
	DllExport int getMaxLatitude() const;
	DllExport int getGreatPeopleRateModifier() const;
	DllExport int getGreatGeneralRateModifier() const;
	DllExport int getDomesticGreatGeneralRateModifier() const;
	DllExport int getGlobalGreatPeopleRateModifier() const;
	DllExport int getAnarchyModifier() const;
	DllExport int getGoldenAgeModifier() const;
	DllExport int getGlobalHurryModifier() const;
	DllExport int getFreeExperience() const;
	DllExport int getGlobalFreeExperience() const;
	DllExport int getFoodKept() const;
	DllExport int getAirlift() const;
	DllExport int getAirModifier() const;
	DllExport int getAirUnitCapacity() const;
	DllExport int getNukeModifier() const;
	DllExport int getNukeExplosionRand() const;
	DllExport int getFreeSpecialist() const;
	DllExport int getAreaFreeSpecialist() const;
	DllExport int getGlobalFreeSpecialist() const;
	DllExport int getHappiness() const;
	DllExport int getAreaHappiness() const;
	DllExport int getGlobalHappiness() const;
	DllExport int getStateReligionHappiness() const;
	DllExport int getWorkerSpeedModifier() const;
	DllExport int getMilitaryProductionModifier() const;
	DllExport int getSpaceProductionModifier() const;
	DllExport int getGlobalSpaceProductionModifier() const;
	DllExport int getTradeRoutes() const;
	DllExport int getCoastalTradeRoutes() const;
	DllExport int getGlobalTradeRoutes() const;
	DllExport int getTradeRouteModifier() const;
	DllExport int getForeignTradeRouteModifier() const;
	DllExport int getAssetValue() const;
	DllExport int getPowerValue() const;
	DllExport int getSpecialBuildingType() const;
	DllExport int getAdvisorType() const;
	DllExport int getHolyCity() const;
	DllExport int getReligionType() const;
	DllExport int getStateReligion() const;
	DllExport int getPrereqReligion() const;
	DllExport int getPrereqCorporation() const;
	DllExport int getFoundsCorporation() const;
	DllExport int getGlobalReligionCommerce() const;
	DllExport int getGlobalCorporationCommerce() const;
	DllExport int getPrereqAndBonus() const;
	DllExport int getGreatPeopleUnitClass() const;
	DllExport int getGreatPeopleRateChange() const;
	DllExport int getConquestProbability() const;
	DllExport int getMaintenanceModifier() const;
	DllExport int getWarWearinessModifier() const;
	DllExport int getGlobalWarWearinessModifier() const;
	DllExport int getEnemyWarWearinessModifier() const;
	DllExport int getHealRateChange() const;
	DllExport int getHealth() const;
	DllExport int getAreaHealth() const;
	DllExport int getGlobalHealth() const;
	DllExport int getGlobalPopulationChange() const;
	DllExport int getFreeTechs() const;
	DllExport int getDefenseModifier() const;
	DllExport int getBombardDefenseModifier() const;
	DllExport int getAllCityDefenseModifier() const;
	DllExport int getEspionageDefenseModifier() const;
	DllExport int getMissionType() const;
	DllExport void setMissionType(int iNewType);
	DllExport int getVoteSourceType() const;

	DllExport float getVisibilityPriority() const;

	DllExport bool isTeamShare() const;
	DllExport bool isWater() const;
	DllExport bool isRiver() const;
	DllExport bool isPower() const;
	DllExport bool isDirtyPower() const;
	DllExport bool isAreaCleanPower() const;
	DllExport bool isAreaBorderObstacle() const;
	DllExport bool isForceTeamVoteEligible() const;
	DllExport bool isCapital() const;
	DllExport bool isGovernmentCenter() const;
	DllExport bool isGoldenAge() const;
	DllExport bool isMapCentering() const;
	DllExport bool isNoUnhappiness() const;
	DllExport bool isNoUnhealthyPopulation() const;
	DllExport bool isBuildingOnlyHealthy() const;
	DllExport bool isNeverCapture() const;
	DllExport bool isNukeImmune() const;
	DllExport bool isPrereqReligion() const;
	DllExport bool isCenterInCity() const;
	DllExport bool isStateReligion() const;
	DllExport bool isAllowsNukes() const;

	DllExport const TCHAR* getConstructSound() const;
	DllExport void setConstructSound(const TCHAR* szVal);
	DllExport const TCHAR* getArtDefineTag() const;
	DllExport void setArtDefineTag(const TCHAR* szVal);
	DllExport const TCHAR* getMovieDefineTag() const;
	DllExport void setMovieDefineTag(const TCHAR* szVal);

	// Arrays

	DllExport int getYieldChange(int i) const;
	int* getYieldChangeArray() const;
	DllExport int getYieldModifier(int i) const;;
	int* getYieldModifierArray() const;
	DllExport int getPowerYieldModifier(int i) const;
	int* getPowerYieldModifierArray() const;
	DllExport int getAreaYieldModifier(int i) const;
	int* getAreaYieldModifierArray() const;
	DllExport int getGlobalYieldModifier(int i) const;
	int* getGlobalYieldModifierArray() const;
	DllExport int getSeaPlotYieldChange(int i) const;
	int* getSeaPlotYieldChangeArray() const;
	DllExport int getRiverPlotYieldChange(int i) const;
	int* getRiverPlotYieldChangeArray() const;
	DllExport int getGlobalSeaPlotYieldChange(int i) const;
	int* getGlobalSeaPlotYieldChangeArray() const;

	DllExport int getCommerceChange(int i) const;
	int* getCommerceChangeArray() const;
	DllExport int getObsoleteSafeCommerceChange(int i) const;
	int* getObsoleteSafeCommerceChangeArray() const;
	DllExport int getCommerceChangeDoubleTime(int i) const;
	DllExport int getCommerceModifier(int i) const;
	int* getCommerceModifierArray() const;
	DllExport int getGlobalCommerceModifier(int i) const;
	int* getGlobalCommerceModifierArray() const;
	DllExport int getSpecialistExtraCommerce(int i) const;
	int* getSpecialistExtraCommerceArray() const;
	DllExport int getStateReligionCommerce(int i) const;
	int* getStateReligionCommerceArray() const;
	DllExport int getCommerceHappiness(int i) const;
	DllExport int getReligionChange(int i) const;
	DllExport int getSpecialistCount(int i) const;
	DllExport int getFreeSpecialistCount(int i) const;
	DllExport int getBonusHealthChanges(int i) const;
	DllExport int getBonusHappinessChanges(int i) const;
	DllExport int getBonusProductionModifier(int i) const;
	DllExport int getUnitCombatFreeExperience(int i) const;
	DllExport int getDomainFreeExperience(int i) const;
	DllExport int getDomainProductionModifier(int i) const;
	DllExport int getPrereqAndTechs(int i) const;
	DllExport int getPrereqOrBonuses(int i) const;
	DllExport int getProductionTraits(int i) const;
	DllExport int getHappinessTraits(int i) const;
	DllExport int getBuildingHappinessChanges(int i) const;
	DllExport int getPrereqNumOfBuildingClass(int i) const;
	DllExport int getFlavorValue(int i) const;
	DllExport int getImprovementFreeSpecialist(int i) const;

	DllExport bool isCommerceFlexible(int i) const;
	DllExport bool isCommerceChangeOriginalOwner(int i) const;
	DllExport bool isBuildingClassNeededInCity(int i) const;

	DllExport int getSpecialistYieldChange(int i, int j) const;
	int* getSpecialistYieldChangeArray(int i) const;

	DllExport int getBonusYieldModifier(int i, int j) const;
	int* getBonusYieldModifierArray(int i) const;

	// Other

	DllExport const CvArtInfoBuilding* getArtInfo() const;
	DllExport const CvArtInfoMovie* getMovieInfo() const;
	DllExport const TCHAR* getButton() const;
	DllExport const TCHAR* getMovie() const;

	// serialization
	DllExport void read(FDataStreamBase*);
	DllExport void write(FDataStreamBase*);
	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PUBLIC MEMBER VARIABLES---------------------------------
protected:

	int m_iBuildingClassType;
	int m_iVictoryPrereq;
	int m_iFreeStartEra;
	int m_iMaxStartEra;
	int m_iObsoleteTech;
	int m_iPrereqAndTech;
	int m_iNoBonus;
	int m_iPowerBonus;
	int m_iFreeBonus;
	int m_iNumFreeBonuses;
	int m_iFreeBuildingClass;
	int m_iFreePromotion;
	int m_iCivicOption;
	int m_iAIWeight;
	int m_iProductionCost;
	int m_iHurryCostModifier;
	int m_iHurryAngerModifier;
	int m_iAdvancedStartCost;
	int m_iAdvancedStartCostIncrease;
	int m_iMinAreaSize;
	int m_iNumCitiesPrereq;
	int m_iNumTeamsPrereq;
	int m_iUnitLevelPrereq;
	int m_iMinLatitude;
	int m_iMaxLatitude;
	int m_iGreatPeopleRateModifier;
	int m_iGreatGeneralRateModifier;
	int m_iDomesticGreatGeneralRateModifier;
	int m_iGlobalGreatPeopleRateModifier;
	int m_iAnarchyModifier;
	int m_iGoldenAgeModifier;
	int m_iGlobalHurryModifier;
	int m_iFreeExperience;
	int m_iGlobalFreeExperience;
	int m_iFoodKept;
	int m_iAirlift;
	int m_iAirModifier;
	int m_iAirUnitCapacity;
	int m_iNukeModifier;
	int m_iNukeExplosionRand;
	int m_iFreeSpecialist;
	int m_iAreaFreeSpecialist;
	int m_iGlobalFreeSpecialist;
	int m_iHappiness;
	int m_iAreaHappiness;
	int m_iGlobalHappiness;
	int m_iStateReligionHappiness;
	int m_iWorkerSpeedModifier;
	int m_iMilitaryProductionModifier;
	int m_iSpaceProductionModifier;
	int m_iGlobalSpaceProductionModifier;
	int m_iTradeRoutes;
	int m_iCoastalTradeRoutes;
	int m_iGlobalTradeRoutes;
	int m_iTradeRouteModifier;
	int m_iForeignTradeRouteModifier;
	int m_iAssetValue;
	int m_iPowerValue;
	int m_iSpecialBuildingType;
	int m_iAdvisorType;
	int m_iHolyCity;
	int m_iReligionType;
	int m_iStateReligion;
	int m_iPrereqReligion;
	int m_iPrereqCorporation;
	int m_iFoundsCorporation;
	int m_iGlobalReligionCommerce;
	int m_iGlobalCorporationCommerce;
	int m_iPrereqAndBonus;
	int m_iGreatPeopleUnitClass;
	int m_iGreatPeopleRateChange;
	int m_iConquestProbability;
	int m_iMaintenanceModifier;
	int m_iWarWearinessModifier;
	int m_iGlobalWarWearinessModifier;
	int m_iEnemyWarWearinessModifier;
	int m_iHealRateChange;
	int m_iHealth;
	int m_iAreaHealth;
	int m_iGlobalHealth;
	int m_iGlobalPopulationChange;
	int m_iFreeTechs;
	int m_iDefenseModifier;
	int m_iBombardDefenseModifier;
	int m_iAllCityDefenseModifier;
	int m_iEspionageDefenseModifier;
	int m_iMissionType;
	int m_iVoteSourceType;

	float m_fVisibilityPriority;

	bool m_bTeamShare;
	bool m_bWater;
	bool m_bRiver;
	bool m_bPower;
	bool m_bDirtyPower;
	bool m_bAreaCleanPower;
	bool m_bAreaBorderObstacle;
	bool m_bForceTeamVoteEligible;
	bool m_bCapital;
	bool m_bGovernmentCenter;
	bool m_bGoldenAge;
	bool m_bMapCentering;
	bool m_bNoUnhappiness;
	bool m_bNoUnhealthyPopulation;
	bool m_bBuildingOnlyHealthy;
	bool m_bNeverCapture;
	bool m_bNukeImmune;
	bool m_bPrereqReligion;
	bool m_bCenterInCity;
	bool m_bStateReligion;
	bool m_bAllowsNukes;

	CvString m_szConstructSound;
	CvString m_szArtDefineTag;
	CvString m_szMovieDefineTag;

	// Arrays

	int* m_piPrereqAndTechs;
	int* m_piPrereqOrBonuses;
	int* m_piProductionTraits;
	int* m_piHappinessTraits;
	int* m_piSeaPlotYieldChange;
	int* m_piRiverPlotYieldChange;
	int* m_piGlobalSeaPlotYieldChange;
	int* m_piYieldChange;
	int* m_piYieldModifier;
	int* m_piPowerYieldModifier;
	int* m_piAreaYieldModifier;
	int* m_piGlobalYieldModifier;
	int* m_piCommerceChange;
	int* m_piObsoleteSafeCommerceChange;
	int* m_piCommerceChangeDoubleTime;
	int* m_piCommerceModifier;
	int* m_piGlobalCommerceModifier;
	int* m_piSpecialistExtraCommerce;
	int* m_piStateReligionCommerce;
	int* m_piCommerceHappiness;
	int* m_piReligionChange;
	int* m_piSpecialistCount;
	int* m_piFreeSpecialistCount;
	int* m_piBonusHealthChanges;
	int* m_piBonusHappinessChanges;
	int* m_piBonusProductionModifier;
	int* m_piUnitCombatFreeExperience;
	int* m_piDomainFreeExperience;
	int* m_piDomainProductionModifier;
	int* m_piBuildingHappinessChanges;
	int* m_piPrereqNumOfBuildingClass;
	int* m_piFlavorValue;
	int* m_piImprovementFreeSpecialist;

	bool* m_pbCommerceFlexible;
	bool* m_pbCommerceChangeOriginalOwner;
	bool* m_pbBuildingClassNeededInCity;

	int** m_ppaiSpecialistYieldChange;
	int** m_ppaiBonusYieldModifier;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvSpecialBuildingInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvSpecialBuildingInfo :
	public CvInfoBase
{

	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvSpecialBuildingInfo();
	DllExport virtual ~CvSpecialBuildingInfo();

	DllExport int getObsoleteTech( void ) const;
	DllExport int getTechPrereq( void ) const;
	DllExport int getTechPrereqAnyone( void ) const;

	DllExport bool isValid( void ) const;

	// Arrays

	DllExport int getProductionTraits(int i) const;

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	int m_iObsoleteTech;
	int m_iTechPrereq;
	int m_iTechPrereqAnyone;

	bool m_bValid;

	// Arrays

	int* m_piProductionTraits;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvBuildingClassInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvBuildingClassInfo :
	public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvBuildingClassInfo();
	DllExport virtual ~CvBuildingClassInfo();

	DllExport int getMaxGlobalInstances() const;
	DllExport int getMaxTeamInstances() const;
	DllExport int getMaxPlayerInstances() const;
	DllExport int getExtraPlayerInstances() const;
	DllExport int getDefaultBuildingIndex() const;
	DllExport void setDefaultBuildingIndex(int i);

	DllExport bool isNoLimit() const;
	DllExport bool isMonument() const;

	// Arrays

	DllExport int getVictoryThreshold(int i) const;

	DllExport bool read(CvXMLLoadUtility* pXML);
	DllExport bool readPass3();

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	int m_iMaxGlobalInstances;
	int m_iMaxTeamInstances;
	int m_iMaxPlayerInstances;
	int m_iExtraPlayerInstances;
	int m_iDefaultBuildingIndex;

	bool m_bNoLimit;
	bool m_bMonument;

	// Arrays

	int* m_piVictoryThreshold;

};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvRiverInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvRiverInfo :
	public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvRiverInfo();
	DllExport virtual ~CvRiverInfo();

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvRiverModelInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvRiverModelInfo :
	public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvRiverModelInfo();
	DllExport virtual ~CvRiverModelInfo();

	DllExport const TCHAR* getModelFile() const;
	DllExport void setModelFile(const TCHAR* szVal);				// The model filename
	DllExport const TCHAR* getBorderFile() const;
	DllExport void setBorderFile(const TCHAR* szVal);				// The model filename

	DllExport int getTextureIndex() const;
	DllExport const TCHAR* getDeltaString() const;				//Exposed to Python
	DllExport const TCHAR* getConnectString() const;
	DllExport const TCHAR* getRotateString() const;

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------

protected:

	CvString m_szModelFile;					// The model filename
	CvString m_szBorderFile;				// The border filename

	TCHAR		m_szDeltaString[32];		// The delta type
	TCHAR		m_szConnectString[32];		// The connections this cell makes ( N S E W NE NW SE SW )
	TCHAR		m_szRotateString[32];		// The possible rotations for this cell ( 0 90 180 270 )
	int			m_iTextureIndex;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvRouteModelInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvRouteModelInfo :
	public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvRouteModelInfo();
	DllExport virtual ~CvRouteModelInfo();

	DllExport RouteTypes getRouteType() const;				// The route type

	DllExport const TCHAR* getModelFile() const;
	DllExport void setModelFile(const TCHAR* szVal);				// The model filename
	DllExport const TCHAR* getLateModelFile() const;
	DllExport void setLateModelFile(const TCHAR* szVal);				// The model filename
	DllExport const TCHAR* getModelFileKey() const;
	DllExport void setModelFileKey(const TCHAR* szVal);				// The model filename Key

	DllExport bool isAnimated() const;

	DllExport const TCHAR* getConnectString() const;
	DllExport const TCHAR* getModelConnectString() const;
	DllExport const TCHAR* getRotateString() const;

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------

protected:

	RouteTypes	m_eRouteType;			// The route type

	CvString m_szModelFile;				// The model filename
	CvString m_szLateModelFile;				// The model filename
	CvString m_szModelFileKey;			// The model file key reference
	bool m_bAnimated;

	TCHAR		m_szConnectString[32];	// The connections this cell makes ( N S E W NE NW SE SW )
	TCHAR		m_szModelConnectString[32];	// The connections this model makes ( N S E W NE NW SE SW )
	TCHAR		m_szRotateString[32];	// The possible rotations for this cell ( 0 90 180 270 )
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvCivilizationInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvArtInfoCivilization;
class CvCivilizationInfo :
	public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvCivilizationInfo();
	DllExport virtual ~CvCivilizationInfo();

	DllExport int getDefaultPlayerColor() const;				// Expose to Python
	DllExport int getArtStyleType() const;				// Expose to Python
	DllExport int getUnitArtStyleType() const;         // Expose to Python
	DllExport int getNumCityNames() const;				// Expose to Python
	DllExport int getNumLeaders() const;				 - the number of leaders the Civ has, this is needed so that random leaders can be generated easily
	DllExport int getSelectionSoundScriptId() const;				// Expose to Python
	DllExport int getActionSoundScriptId() const;				// Expose to Python

	DllExport bool isAIPlayable() const;				// Expose to Python
	DllExport bool isPlayable() const;				// Expose to Python

	std::wstring pyGetShortDescription(uint uiForm) { return getShortDescription(uiForm); }
	DllExport const wchar* getShortDescription(uint uiForm = 0);
	DllExport void setShortDescriptionKey(const TCHAR* szVal);
	DllExport const wchar* getShortDescriptionKey() const;
	std::wstring pyGetShortDescriptionKey() { return getShortDescriptionKey(); }

	std::wstring pyGetAdjective(uint uiForm) { return getAdjective(uiForm);  }
	DllExport const wchar* getAdjective(uint uiForm = 0);
	DllExport void setAdjectiveKey(const TCHAR* szVal);
	DllExport const wchar* getAdjectiveKey() const;
	std::wstring pyGetAdjectiveKey() { return getAdjectiveKey(); }

	DllExport const TCHAR* getFlagTexture() const;
	DllExport const TCHAR* getArtDefineTag() const;
	DllExport void setArtDefineTag(const TCHAR* szVal);
	// Arrays

	DllExport int getCivilizationBuildings(int i) const;
	DllExport int getCivilizationUnits(int i) const;
	DllExport int getCivilizationFreeUnitsClass(int i) const;
	DllExport int getCivilizationInitialCivics(int i) const;

	DllExport bool isLeaders(int i) const;
	DllExport bool isCivilizationFreeBuildingClass(int i) const;
	DllExport bool isCivilizationFreeTechs(int i) const;
	DllExport bool isCivilizationDisableTechs(int i) const;

	DllExport std::string getCityNames(int i) const;

	DllExport const CvArtInfoCivilization* getArtInfo() const;
	DllExport const TCHAR* getButton() const;

	DllExport int getDerivativeCiv() const;
	void setDerivativeCiv(int iCiv);

	DllExport bool read(CvXMLLoadUtility* pXML);
	DllExport bool readPass2(CvXMLLoadUtility* pXML);
	DllExport void read(FDataStreamBase* stream);
	DllExport void write(FDataStreamBase* stream);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------

protected:

	int m_iDefaultPlayerColor;
	int m_iArtStyleType;
	int m_iUnitArtStyleType;  // FlavorUnits by Impaler[WrG]
	int m_iNumCityNames;
	int m_iNumLeaders;				 // the number of leaders the Civ has, this is needed so that random leaders can be generated easily
	int m_iSelectionSoundScriptId;
	int m_iActionSoundScriptId;
	int m_iDerivativeCiv;

	bool m_bAIPlayable;
	bool m_bPlayable;

	CvString m_szArtDefineTag;
	CvString m_szShortDescriptionKey;
	CvString m_szAdjectiveKey;
	// Arrays

	int* m_piCivilizationBuildings;
	int* m_piCivilizationUnits;
	int* m_piCivilizationFreeUnitsClass;
	int* m_piCivilizationInitialCivics;

	bool* m_pbLeaders;
	bool* m_pbCivilizationFreeBuildingClass;
	bool* m_pbCivilizationFreeTechs;
	bool* m_pbCivilizationDisableTechs;

	CvString* m_paszCityNames;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvVictoryInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvVictoryInfo :
	public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvVictoryInfo();
	DllExport virtual ~CvVictoryInfo();

	DllExport int getPopulationPercentLead() const;
	DllExport int getLandPercent() const;
	DllExport int getMinLandPercent() const;
	DllExport int getReligionPercent() const;
	DllExport int getCityCulture() const;
	DllExport int getNumCultureCities() const;
	DllExport int getTotalCultureRatio() const;
	DllExport int getVictoryDelayTurns() const;

	DllExport bool isTargetScore() const;
	DllExport bool isEndScore() const;
	DllExport bool isConquest() const;
	DllExport bool isDiploVote() const;
	DllExport bool isPermanent() const;

	DllExport const char* getMovie() const;

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------

protected:

	int m_iPopulationPercentLead;
	int m_iLandPercent;
	int m_iMinLandPercent;
	int m_iReligionPercent;
	int m_iCityCulture;
	int m_iNumCultureCities;
	int m_iTotalCultureRatio;
	int m_iVictoryDelayTurns;

	bool m_bTargetScore;
	bool m_bEndScore;
	bool m_bConquest;
	bool m_bDiploVote;
	bool m_bPermanent;

	CvString m_szMovie;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvHurryInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvHurryInfo :
	public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
	public:

		DllExport CvHurryInfo();
		DllExport virtual ~CvHurryInfo();

		DllExport int getGoldPerProduction() const;
		DllExport int getProductionPerPopulation() const;

		DllExport bool isAnger() const;

		DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PUBLIC MEMBER VARIABLES---------------------------------
	protected:

		int m_iGoldPerProduction;
		int m_iProductionPerPopulation;

		bool m_bAnger;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvHandicapInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvHandicapInfo :
	public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvHandicapInfo();
	DllExport virtual ~CvHandicapInfo();

	DllExport int getFreeWinsVsBarbs() const;
	DllExport int getAnimalAttackProb() const;
	DllExport int getStartingLocationPercent() const;
	DllExport int getAdvancedStartPointsMod() const;
	DllExport int getStartingGold() const;
	DllExport int getFreeUnits() const;
	DllExport int getUnitCostPercent() const;
	DllExport int getResearchPercent() const;
	DllExport int getDistanceMaintenancePercent() const;
	DllExport int getNumCitiesMaintenancePercent() const;
	DllExport int getMaxNumCitiesMaintenance() const;
	DllExport int getColonyMaintenancePercent() const;
	DllExport int getMaxColonyMaintenance() const;
	DllExport int getCorporationMaintenancePercent() const;
	DllExport int getCivicUpkeepPercent() const;
	DllExport int getInflationPercent() const;
	DllExport int getHealthBonus() const;
	DllExport int getHappyBonus() const;
	DllExport int getAttitudeChange() const;
	DllExport int getNoTechTradeModifier() const;
	DllExport int getTechTradeKnownModifier() const;
	DllExport int getUnownedTilesPerGameAnimal() const;
	DllExport int getUnownedTilesPerBarbarianUnit() const;
	DllExport int getUnownedWaterTilesPerBarbarianUnit() const;
	DllExport int getUnownedTilesPerBarbarianCity() const;
	DllExport int getBarbarianCreationTurnsElapsed() const;
	DllExport int getBarbarianCityCreationTurnsElapsed() const;
	DllExport int getBarbarianCityCreationProb() const;
	DllExport int getAnimalCombatModifier() const;
	DllExport int getBarbarianCombatModifier() const;
	DllExport int getAIAnimalCombatModifier() const;
	DllExport int getAIBarbarianCombatModifier() const;

	DllExport int getStartingDefenseUnits() const;
	DllExport int getStartingWorkerUnits() const;
	DllExport int getStartingExploreUnits() const;
	DllExport int getAIStartingUnitMultiplier() const;
	DllExport int getAIStartingDefenseUnits() const;
	DllExport int getAIStartingWorkerUnits() const;
	DllExport int getAIStartingExploreUnits() const;
	DllExport int getBarbarianInitialDefenders() const;
	DllExport int getAIDeclareWarProb() const;
	DllExport int getAIWorkRateModifier() const;
	DllExport int getAIGrowthPercent() const;
	DllExport int getAITrainPercent() const;
	DllExport int getAIWorldTrainPercent() const;
	DllExport int getAIConstructPercent() const;
	DllExport int getAIWorldConstructPercent() const;
	DllExport int getAICreatePercent() const;
	DllExport int getAIWorldCreatePercent() const;
	DllExport int getAICivicUpkeepPercent() const;
	DllExport int getAIUnitCostPercent() const;
	DllExport int getAIUnitSupplyPercent() const;
	DllExport int getAIUnitUpgradePercent() const;
	DllExport int getAIInflationPercent() const;
	DllExport int getAIWarWearinessPercent() const;
	DllExport int getAIPerEraModifier() const;
	DllExport int getAIAdvancedStartPercent() const;
	DllExport int getNumGoodies() const;

	// Arrays

	DllExport int getGoodies(int i) const;
	DllExport int isFreeTechs(int i) const;
	DllExport int isAIFreeTechs(int i) const;

	DllExport void read(FDataStreamBase* stream);
	DllExport void write(FDataStreamBase* stream);

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	int m_iFreeWinsVsBarbs;
	int m_iAnimalAttackProb;
	int m_iStartingLocationPercent;
	int m_iAdvancedStartPointsMod;
	int m_iStartingGold;
	int m_iFreeUnits;
	int m_iUnitCostPercent;
	int m_iResearchPercent;
	int m_iDistanceMaintenancePercent;
	int m_iNumCitiesMaintenancePercent;
	int m_iMaxNumCitiesMaintenance;
	int m_iColonyMaintenancePercent;
	int m_iMaxColonyMaintenance;
	int m_iCorporationMaintenancePercent;
	int m_iCivicUpkeepPercent;
	int m_iInflationPercent;
	int m_iHealthBonus;
	int m_iHappyBonus;
	int m_iAttitudeChange;
	int m_iNoTechTradeModifier;
	int m_iTechTradeKnownModifier;
	int m_iUnownedTilesPerGameAnimal;
	int m_iUnownedTilesPerBarbarianUnit;
	int m_iUnownedWaterTilesPerBarbarianUnit;
	int m_iUnownedTilesPerBarbarianCity;
	int m_iBarbarianCreationTurnsElapsed;
	int m_iBarbarianCityCreationTurnsElapsed;
	int m_iBarbarianCityCreationProb;
	int m_iAnimalCombatModifier;
	int m_iBarbarianCombatModifier;
	int m_iAIAnimalCombatModifier;
	int m_iAIBarbarianCombatModifier;

	int m_iStartingDefenseUnits;
	int m_iStartingWorkerUnits;
	int m_iStartingExploreUnits;
	int m_iAIStartingUnitMultiplier;
	int m_iAIStartingDefenseUnits;
	int m_iAIStartingWorkerUnits;
	int m_iAIStartingExploreUnits;
	int m_iBarbarianInitialDefenders;
	int m_iAIDeclareWarProb;
	int m_iAIWorkRateModifier;
	int m_iAIGrowthPercent;
	int m_iAITrainPercent;
	int m_iAIWorldTrainPercent;
	int m_iAIConstructPercent;
	int m_iAIWorldConstructPercent;
	int m_iAICreatePercent;
	int m_iAIWorldCreatePercent;
	int m_iAICivicUpkeepPercent;
	int m_iAIUnitCostPercent;
	int m_iAIUnitSupplyPercent;
	int m_iAIUnitUpgradePercent;
	int m_iAIInflationPercent;
	int m_iAIWarWearinessPercent;
	int m_iAIPerEraModifier;
	int m_iAIAdvancedStartPercent;
	int m_iNumGoodies;

	CvString m_szHandicapName;

	// Arrays

	int* m_piGoodies;

	bool* m_pbFreeTechs;
	bool* m_pbAIFreeTechs;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvGameSpeedInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvGameSpeedInfo :
	public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvGameSpeedInfo();
	DllExport virtual ~CvGameSpeedInfo();

	DllExport int getGrowthPercent() const;
	DllExport int getTrainPercent() const;
	DllExport int getConstructPercent() const;
	DllExport int getCreatePercent() const;
	DllExport int getResearchPercent() const;
	DllExport int getBuildPercent() const;
	DllExport int getImprovementPercent() const;
	DllExport int getGreatPeoplePercent() const;
	DllExport int getAnarchyPercent() const;
	DllExport int getBarbPercent() const;
	DllExport int getFeatureProductionPercent() const;
	DllExport int getUnitDiscoverPercent() const;
	DllExport int getUnitHurryPercent() const;
	DllExport int getUnitTradePercent() const;
	DllExport int getUnitGreatWorkPercent() const;
	DllExport int getGoldenAgePercent() const;
	DllExport int getHurryPercent() const;
	DllExport int getHurryConscriptAngerPercent() const;
	DllExport int getInflationOffset() const;
	DllExport int getInflationPercent() const;
	DllExport int getVictoryDelayPercent() const;
	DllExport int getNumTurnIncrements() const;

	DllExport GameTurnInfo& getGameTurnInfo(int iIndex) const;
	DllExport void allocateGameTurnInfos(const int iSize);

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	int m_iGrowthPercent;
	int m_iTrainPercent;
	int m_iConstructPercent;
	int m_iCreatePercent;
	int m_iResearchPercent;
	int m_iBuildPercent;
	int m_iImprovementPercent;
	int m_iGreatPeoplePercent;
	int m_iAnarchyPercent;
	int m_iBarbPercent;
	int m_iFeatureProductionPercent;
	int m_iUnitDiscoverPercent;
	int m_iUnitHurryPercent;
	int m_iUnitTradePercent;
	int m_iUnitGreatWorkPercent;
	int m_iGoldenAgePercent;
	int m_iHurryPercent;
	int m_iHurryConscriptAngerPercent;
	int m_iInflationOffset;
	int m_iInflationPercent;
	int m_iVictoryDelayPercent;
	int m_iNumTurnIncrements;

	CvString m_szGameSpeedName;
	GameTurnInfo* m_pGameTurnInfo;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvTurnTimerInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvTurnTimerInfo :
	public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvTurnTimerInfo();
	DllExport virtual ~CvTurnTimerInfo();

	DllExport int getBaseTime() const;
	DllExport int getCityBonus() const;
	DllExport int getUnitBonus() const;
	DllExport int getFirstTurnMultiplier() const;

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	int m_iBaseTime;
	int m_iCityBonus;
	int m_iUnitBonus;
	int m_iFirstTurnMultiplier;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvBuildInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvBuildInfo : public CvHotkeyInfo
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvBuildInfo();
	DllExport virtual ~CvBuildInfo();

	DllExport int getTime() const;
	DllExport int getCost() const;
	DllExport int getTechPrereq() const;
	DllExport int getImprovement() const;
	DllExport int getRoute() const;
	DllExport int getEntityEvent() const;
	DllExport int getMissionType() const;
	DllExport void setMissionType(int iNewType);

	DllExport bool isKill() const;

	// Arrays

	DllExport int getFeatureTech(int i) const;
	DllExport int getFeatureTime(int i) const;
	DllExport int getFeatureProduction(int i) const;

	DllExport bool isFeatureRemove(int i) const;

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------

protected:

	int m_iTime;
	int m_iCost;
	int m_iTechPrereq;
	int m_iImprovement;
	int m_iRoute;
	int m_iEntityEvent;
	int m_iMissionType;

	bool m_bKill;

	// Arrays

	int* m_paiFeatureTech;
	int* m_paiFeatureTime;
	int* m_paiFeatureProduction;

	bool* m_pabFeatureRemove;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvGoodyInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvGoodyInfo :
	public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvGoodyInfo();
	DllExport virtual ~CvGoodyInfo();

	DllExport int getGold() const;
	DllExport int getGoldRand1() const;
	DllExport int getGoldRand2() const;
	DllExport int getMapOffset() const;
	DllExport int getMapRange() const;
	DllExport int getMapProb() const;
	DllExport int getExperience() const;
	DllExport int getHealing() const;
	DllExport int getDamagePrereq() const;
	DllExport int getBarbarianUnitProb() const;
	DllExport int getMinBarbarians() const;
	DllExport int getUnitClassType() const;
	DllExport int getBarbarianUnitClass() const;

	DllExport bool isTech() const;
	DllExport bool isBad() const;

	DllExport const TCHAR* getSound() const;
	DllExport void setSound(const TCHAR* szVal);

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------

protected:

	int m_iGold;
	int m_iGoldRand1;
	int m_iGoldRand2;
	int m_iMapOffset;
	int m_iMapRange;
	int m_iMapProb;
	int m_iExperience;
	int m_iHealing;
	int m_iDamagePrereq;
	int m_iBarbarianUnitProb;
	int m_iMinBarbarians;
	int m_iUnitClassType;
	int m_iBarbarianUnitClass;

	bool m_bTech;
	bool m_bBad;

	CvString m_szSound;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvRouteInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvRouteInfo :
	public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvRouteInfo();
	DllExport virtual ~CvRouteInfo();

	DllExport int getAdvancedStartCost() const;
	DllExport int getAdvancedStartCostIncrease() const;

	DllExport int getValue() const;
	DllExport int getMovementCost() const;
	DllExport int getFlatMovementCost() const;
	DllExport int getPrereqBonus() const;

	// Arrays

	DllExport int getYieldChange(int i) const;
	DllExport int getTechMovementChange(int i) const;
	DllExport int getPrereqOrBonus(int i) const;

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	int m_iAdvancedStartCost;
	int m_iAdvancedStartCostIncrease;

	int m_iValue;
	int m_iMovementCost;
	int m_iFlatMovementCost;
	int m_iPrereqBonus;

	// Arrays

	int* m_piYieldChange;
	int* m_piTechMovementChange;
	int* m_piPrereqOrBonuses;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvImprovementBonusInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvImprovementBonusInfo :
	public CvInfoBase
{

friend class CvImprovementInfo;
friend class CvXMLLoadUtility;

	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvImprovementBonusInfo();
	DllExport virtual ~CvImprovementBonusInfo();

	DllExport int getDiscoverRand() const;

	DllExport bool isBonusMakesValid() const;
	DllExport bool isBonusTrade() const;

	DllExport int getYieldChange(int i) const;

	// Serialize

	DllExport void read(FDataStreamBase* stream);
	DllExport void write(FDataStreamBase* stream);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	int m_iDiscoverRand;

	bool m_bBonusMakesValid;
	bool m_bBonusTrade;

	// Arrays

	int* m_piYieldChange;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvImprovementInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvArtInfoImprovement;
class CvImprovementInfo :
	public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvImprovementInfo();
	DllExport virtual ~CvImprovementInfo();

	DllExport int getAdvancedStartCost() const;
	DllExport int getAdvancedStartCostIncrease() const;

	DllExport int getTilesPerGoody() const;
	DllExport int getGoodyUniqueRange() const;
	DllExport int getFeatureGrowthProbability() const;
	DllExport int getUpgradeTime() const;
	DllExport int getAirBombDefense() const;
	DllExport int getDefenseModifier() const;
	DllExport int getHappiness() const;
	DllExport int getPillageGold() const;
	DllExport int getImprovementPillage() const;
	DllExport void setImprovementPillage(int i);
	DllExport int getImprovementUpgrade() const;
	DllExport void setImprovementUpgrade(int i);

	DllExport bool isActsAsCity() const;
	DllExport bool isHillsMakesValid() const;
	DllExport bool isFreshWaterMakesValid() const;
	DllExport bool isRiverSideMakesValid() const;
	DllExport bool isNoFreshWater() const;
	DllExport bool isRequiresFlatlands() const;
	DllExport bool isRequiresRiverSide() const;
	DllExport bool isRequiresIrrigation() const;
	DllExport bool isCarriesIrrigation() const;
	DllExport bool isRequiresFeature() const;
	DllExport bool isWater() const;
	DllExport bool isGoody() const;
	DllExport bool isPermanent() const;
	DllExport bool useLSystem() const;
	DllExport void setUseLSystem(bool bUse);
	DllExport bool isOutsideBorders() const;

	DllExport const TCHAR* getArtDefineTag() const;
	DllExport void setArtDefineTag(const TCHAR* szVal);

	DllExport int getWorldSoundscapeScriptId() const;

	// Arrays

	DllExport int getPrereqNatureYield(int i) const;
	int* getPrereqNatureYieldArray();
	DllExport int getYieldChange(int i) const;
	int* getYieldChangeArray();
	DllExport int getRiverSideYieldChange(int i) const;
	int* getRiverSideYieldChangeArray();
	DllExport int getHillsYieldChange(int i) const;
	int* getHillsYieldChangeArray();
	DllExport int getIrrigatedYieldChange(int i) const;
	DllExport int* getIrrigatedYieldChangeArray();				// For Moose - CvWidgetData XXX

	DllExport bool getTerrainMakesValid(int i) const;
	DllExport bool getFeatureMakesValid(int i) const;

	DllExport int getTechYieldChanges(int i, int j) const;
	int* getTechYieldChangesArray(int i);
	DllExport int getRouteYieldChanges(int i, int j) const;
	DllExport int* getRouteYieldChangesArray(int i);				// For Moose - CvWidgetData XXX

	DllExport int getImprovementBonusYield(int i, int j) const;
	DllExport bool isImprovementBonusMakesValid(int i) const;
	DllExport bool isImprovementBonusTrade(int i) const;
	DllExport int getImprovementBonusDiscoverRand(int i) const;

	// Other

	DllExport const TCHAR* getButton() const;
	DllExport const CvArtInfoImprovement* getArtInfo() const;

	DllExport void read(FDataStreamBase* stream);
	DllExport void write(FDataStreamBase* stream);

	DllExport bool read(CvXMLLoadUtility* pXML);
	DllExport bool readPass2(CvXMLLoadUtility* pXML);




	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------

protected:

	int m_iAdvancedStartCost;
	int m_iAdvancedStartCostIncrease;

	int m_iTilesPerGoody;
	int m_iGoodyUniqueRange;
	int m_iFeatureGrowthProbability;
	int m_iUpgradeTime;
	int m_iAirBombDefense;
	int m_iDefenseModifier;
	int m_iHappiness;
	int m_iPillageGold;
	int m_iImprovementPillage;
	int m_iImprovementUpgrade;

	bool m_bActsAsCity;
	bool m_bHillsMakesValid;
	bool m_bFreshWaterMakesValid;
	bool m_bRiverSideMakesValid;
	bool m_bNoFreshWater;
	bool m_bRequiresFlatlands;
	bool m_bRequiresRiverSide;
	bool m_bRequiresIrrigation;
	bool m_bCarriesIrrigation;
	bool m_bRequiresFeature;
	bool m_bWater;
	bool m_bGoody;
	bool m_bPermanent;
	bool m_bUseLSystem;
	bool m_bOutsideBorders;

	CvString m_szArtDefineTag;


	int m_iWorldSoundscapeScriptId;

	// Arrays

	int* m_piPrereqNatureYield;
	int* m_piYieldChange;
	int* m_piRiverSideYieldChange;
	int* m_piHillsYieldChange;
	int* m_piIrrigatedChange;

	bool* m_pbTerrainMakesValid;
	bool* m_pbFeatureMakesValid;

	int** m_ppiTechYieldChanges;
	int** m_ppiRouteYieldChanges;

	CvImprovementBonusInfo* m_paImprovementBonus;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvBonusClassInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvBonusClassInfo :
	public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvBonusClassInfo();
	DllExport virtual ~CvBonusClassInfo();

	DllExport int getUniqueRange() const;

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------

protected:

	int m_iUniqueRange;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvBonusInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvArtInfoBonus;
class CvBonusInfo :
	public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvBonusInfo();
	DllExport virtual ~CvBonusInfo();

	DllExport int getBonusClassType() const;
	DllExport int getChar() const;
	DllExport void setChar(int i);
	DllExport int getTechReveal() const;
	DllExport int getTechCityTrade() const;
	DllExport int getTechObsolete() const;
	DllExport int getAITradeModifier() const;
	DllExport int getAIObjective() const;
	DllExport int getHealth() const;
	DllExport int getHappiness() const;
	DllExport int getMinAreaSize() const;
	DllExport int getMinLatitude() const;
	DllExport int getMaxLatitude() const;
	DllExport int getPlacementOrder() const;
	DllExport int getConstAppearance() const;
	DllExport int getRandAppearance1() const;
	DllExport int getRandAppearance2() const;
	DllExport int getRandAppearance3() const;
	DllExport int getRandAppearance4() const;
	DllExport int getPercentPerPlayer() const;
	DllExport int getTilesPer() const;
	DllExport int getMinLandPercent() const;
	DllExport int getUniqueRange() const;
	DllExport int getGroupRange() const;
	DllExport int getGroupRand() const;

	DllExport bool isOneArea() const;
	DllExport bool isHills() const;
	DllExport bool isFlatlands() const;
	DllExport bool isNoRiverSide() const;
	DllExport bool isNormalize() const;
	DllExport bool useLSystem() const;
	DllExport void setUseLSystem(bool bUse);

	DllExport const TCHAR* getArtDefineTag() const;
	DllExport void setArtDefineTag(const TCHAR* szVal);

	// Arrays

	DllExport int getYieldChange(int i) const;
	int* getYieldChangeArray();
	DllExport int getImprovementChange(int i) const;

	DllExport bool isTerrain(int i) const;
	DllExport bool isFeature(int i) const;
	DllExport bool isFeatureTerrain(int i) const;

	// Other

	DllExport const TCHAR* getButton() const;
	DllExport const CvArtInfoBonus* getArtInfo() const;

	DllExport void read(FDataStreamBase* stream);
	DllExport void write(FDataStreamBase* stream);

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PUBLIC MEMBER VARIABLES---------------------------------

protected:

	int m_iBonusClassType;
	int m_iChar;
	int m_iTechReveal;
	int m_iTechCityTrade;
	int m_iTechObsolete;
	int m_iAITradeModifier;
	int m_iAIObjective;
	int m_iHealth;
	int m_iHappiness;
	int m_iMinAreaSize;
	int m_iMinLatitude;
	int m_iMaxLatitude;
	int m_iPlacementOrder;
	int m_iConstAppearance;
	int m_iRandAppearance1;
	int m_iRandAppearance2;
	int m_iRandAppearance3;
	int m_iRandAppearance4;
	int m_iPercentPerPlayer;
	int m_iTilesPer;
	int m_iMinLandPercent;
	int m_iUniqueRange;
	int m_iGroupRange;
	int m_iGroupRand;

	bool m_bOneArea;
	bool m_bHills;
	bool m_bFlatlands;
	bool m_bNoRiverSide;
	bool m_bNormalize;
	bool m_bUseLSystem;

	CvString m_szArtDefineTag;

	// Arrays

	int* m_piYieldChange;
	int* m_piImprovementChange;

	bool* m_pbTerrain;
	bool* m_pbFeature;
	bool* m_pbFeatureTerrain;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvFeatureInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvArtInfoFeature;
class CvFeatureInfo :
	public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvFeatureInfo();
	DllExport virtual ~CvFeatureInfo();

	DllExport int getMovementCost() const;
	DllExport int getSeeThroughChange() const;
	DllExport int getHealthPercent() const;
	DllExport int getAppearanceProbability() const;
	DllExport int getDisappearanceProbability() const;
	DllExport int getGrowthProbability() const;
	DllExport int getDefenseModifier() const;
	DllExport int getAdvancedStartRemoveCost() const;
	DllExport int getTurnDamage() const;

	DllExport bool isNoCoast() const;
	DllExport bool isNoRiver() const;
	DllExport bool isNoAdjacent() const;
	DllExport bool isRequiresFlatlands() const;
	DllExport bool isRequiresRiver() const;
	DllExport bool isAddsFreshWater() const;
	DllExport bool isImpassable() const;
	DllExport bool isNoCity() const;
	DllExport bool isNoImprovement() const;
	DllExport bool isVisibleAlways() const;
	DllExport bool isNukeImmune() const;
	DllExport const TCHAR* getOnUnitChangeTo() const;

	DllExport const TCHAR* getArtDefineTag() const;
	DllExport void setArtDefineTag(const TCHAR* szTag);

	DllExport int getWorldSoundscapeScriptId() const;

	DllExport const TCHAR* getEffectType() const;
	DllExport int getEffectProbability() const;

	// Arrays

	DllExport int getYieldChange(int i) const;
	DllExport int getRiverYieldChange(int i) const;
	DllExport int getHillsYieldChange(int i) const;
	DllExport int get3DAudioScriptFootstepIndex(int i) const;

	DllExport bool isTerrain(int i) const;
	DllExport int getNumVarieties() const;

	// Other

	DllExport const CvArtInfoFeature* getArtInfo() const;
	DllExport const TCHAR* getButton() const;

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	int m_iMovementCost;
	int m_iSeeThroughChange;
	int m_iHealthPercent;
	int m_iAppearanceProbability;
	int m_iDisappearanceProbability;
	int m_iGrowthProbability;
	int m_iDefenseModifier;
	int m_iAdvancedStartRemoveCost;
	int m_iTurnDamage;

	bool m_bNoCoast;
	bool m_bNoRiver;
	bool m_bNoAdjacent;
	bool m_bRequiresFlatlands;
	bool m_bRequiresRiver;
	bool m_bAddsFreshWater;
	bool m_bImpassable;
	bool m_bNoCity;
	bool m_bNoImprovement;
	bool m_bVisibleAlways;
	bool m_bNukeImmune;
	CvString m_szOnUnitChangeTo;

	int m_iWorldSoundscapeScriptId;

	CvString m_szEffectType;
	int m_iEffectProbability;

	// Arrays

	int* m_piYieldChange;
	int* m_piRiverYieldChange;
	int* m_piHillsYieldChange;
	int* m_pi3DAudioScriptFootstepIndex;

	bool* m_pbTerrain;

private:

	CvString m_szArtDefineTag;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvCommerceInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvCommerceInfo :
	public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvCommerceInfo();
	DllExport virtual ~CvCommerceInfo();

	DllExport int getChar() const;
	DllExport void setChar(int i);
	DllExport int getInitialPercent() const;
	DllExport int getInitialHappiness() const;
	DllExport int getAIWeightPercent() const;

	DllExport bool isFlexiblePercent() const;

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	int m_iChar;
	int m_iInitialPercent;
	int m_iInitialHappiness;
	int m_iAIWeightPercent;

	bool m_bFlexiblePercent;

};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvYieldInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvYieldInfo :
	public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvYieldInfo();
	DllExport virtual ~CvYieldInfo();

	DllExport int getChar() const;
	DllExport void setChar(int i);
	DllExport int getHillsChange() const;
	DllExport int getPeakChange() const;
	DllExport int getLakeChange() const;
	DllExport int getCityChange() const;
	DllExport int getPopulationChangeOffset() const;
	DllExport int getPopulationChangeDivisor() const;
	DllExport int getMinCity() const;
	DllExport int getTradeModifier() const;
	DllExport int getGoldenAgeYield() const;
	DllExport int getGoldenAgeYieldThreshold() const;
	DllExport int getAIWeightPercent() const;
	DllExport int getColorType() const;

	// Arrays

	DllExport const TCHAR* getSymbolPath(int i) const;

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	int m_iChar;
	int m_iHillsChange;
	int m_iPeakChange;
	int m_iLakeChange;
	int m_iCityChange;
	int m_iPopulationChangeOffset;
	int m_iPopulationChangeDivisor;
	int m_iMinCity;
	int m_iTradeModifier;
	int m_iGoldenAgeYield;
	int m_iGoldenAgeYieldThreshold;
	int m_iAIWeightPercent;
	int m_iColorType;

	CvString* m_paszSymbolPath;

};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvTerrainInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvArtInfoTerrain;
class CvTerrainInfo :
	public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvTerrainInfo();
	DllExport virtual ~CvTerrainInfo();

	DllExport int getMovementCost() const;
	DllExport int getSeeFromLevel() const;
	DllExport int getSeeThroughLevel() const;
	DllExport int getBuildModifier() const;
	DllExport int getDefenseModifier() const;

	DllExport bool isWater() const;
	DllExport bool isImpassable() const;
	DllExport bool isFound() const;
	DllExport bool isFoundCoast() const;
	DllExport bool isFoundFreshWater() const;

	DllExport const TCHAR* getArtDefineTag() const;
	DllExport void setArtDefineTag(const TCHAR* szTag);

	DllExport int getWorldSoundscapeScriptId() const;

	// Arrays

	DllExport int getYield(int i) const;
	DllExport int getRiverYieldChange(int i) const;
	DllExport int getHillsYieldChange(int i) const;
	DllExport int get3DAudioScriptFootstepIndex(int i) const;

	// Other

	DllExport const CvArtInfoTerrain* getArtInfo() const;
	DllExport const TCHAR* getButton() const;

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	int m_iMovementCost;
	int m_iSeeFromLevel;
	int m_iSeeThroughLevel;
	int m_iBuildModifier;
	int m_iDefenseModifier;

	bool m_bWater;
	bool m_bImpassable;
	bool m_bFound;
	bool m_bFoundCoast;
	bool m_bFoundFreshWater;

	int m_iWorldSoundscapeScriptId;

	// Arrays

	int* m_piYields;
	int* m_piRiverYieldChange;
	int* m_piHillsYieldChange;
	int* m_pi3DAudioScriptFootstepIndex;

private:

	CvString m_szArtDefineTag;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvInterfaceModeInfo (ADD to Python)
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvInterfaceModeInfo :
	public CvHotkeyInfo
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvInterfaceModeInfo();
	DllExport virtual ~CvInterfaceModeInfo();

	DllExport int getCursorIndex() const;
	DllExport int getMissionType() const;

	DllExport bool getVisible() const;
	DllExport bool getGotoPlot() const;
	DllExport bool getHighlightPlot() const;
	DllExport bool getSelectType() const;
	DllExport bool getSelectAll() const;

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	int m_iCursorIndex;
	int m_iMissionType;

	bool m_bVisible;
	bool m_bGotoPlot;
	bool m_bHighlightPlot;
	bool m_bSelectType;
	bool m_bSelectAll;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvAdvisorInfo (ADD to Python)
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvAdvisorInfo :
	public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvAdvisorInfo();
	DllExport virtual ~CvAdvisorInfo();

	DllExport const TCHAR* getTexture() const;
	DllExport void setTexture(const TCHAR* szVal);
	DllExport int getNumCodes() const;
	DllExport int getEnableCode(uint uiCode) const;
	DllExport int getDisableCode(uint uiCode) const;

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:
	CvString m_szTexture;
	std::vector< std::pair< int, int > > m_vctEnableDisableCodes;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvLeaderHeadInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvArtInfoLeaderhead;
class CvLeaderHeadInfo :
	public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvLeaderHeadInfo();
	DllExport virtual ~CvLeaderHeadInfo();

	DllExport int getWonderConstructRand() const;
	DllExport int getBaseAttitude() const;
	DllExport int getBasePeaceWeight() const;
	DllExport int getPeaceWeightRand() const;
	DllExport int getWarmongerRespect() const;
	DllExport int getEspionageWeight() const;
	DllExport int getRefuseToTalkWarThreshold() const;
	DllExport int getNoTechTradeThreshold() const;
	DllExport int getTechTradeKnownPercent() const;
	DllExport int getMaxGoldTradePercent() const;
	DllExport int getMaxGoldPerTurnTradePercent() const;
	DllExport int getMaxWarRand() const;
	DllExport int getMaxWarNearbyPowerRatio() const;
	DllExport int getMaxWarDistantPowerRatio() const;
	DllExport int getMaxWarMinAdjacentLandPercent() const;
	DllExport int getLimitedWarRand() const;
	DllExport int getLimitedWarPowerRatio() const;
	DllExport int getDogpileWarRand() const;
	DllExport int getMakePeaceRand() const;
	DllExport int getDeclareWarTradeRand() const;
	DllExport int getDemandRebukedSneakProb() const;
	DllExport int getDemandRebukedWarProb() const;
	DllExport int getRazeCityProb() const;
	DllExport int getBuildUnitProb() const;
	DllExport int getBaseAttackOddsChange() const;
	DllExport int getAttackOddsChangeRand() const;
	DllExport int getWorseRankDifferenceAttitudeChange() const;
	DllExport int getBetterRankDifferenceAttitudeChange() const;
	DllExport int getCloseBordersAttitudeChange() const;
	DllExport int getLostWarAttitudeChange() const;
	DllExport int getAtWarAttitudeDivisor() const;
	DllExport int getAtWarAttitudeChangeLimit() const;
	DllExport int getAtPeaceAttitudeDivisor() const;
	DllExport int getAtPeaceAttitudeChangeLimit() const;
	DllExport int getSameReligionAttitudeChange() const;
	DllExport int getSameReligionAttitudeDivisor() const;
	DllExport int getSameReligionAttitudeChangeLimit() const;
	DllExport int getDifferentReligionAttitudeChange() const;
	DllExport int getDifferentReligionAttitudeDivisor() const;
	DllExport int getDifferentReligionAttitudeChangeLimit() const;
	DllExport int getBonusTradeAttitudeDivisor() const;
	DllExport int getBonusTradeAttitudeChangeLimit() const;
	DllExport int getOpenBordersAttitudeDivisor() const;
	DllExport int getOpenBordersAttitudeChangeLimit() const;
	DllExport int getDefensivePactAttitudeDivisor() const;
	DllExport int getDefensivePactAttitudeChangeLimit() const;
	DllExport int getShareWarAttitudeChange() const;
	DllExport int getShareWarAttitudeDivisor() const;
	DllExport int getShareWarAttitudeChangeLimit() const;
	DllExport int getFavoriteCivicAttitudeChange() const;
	DllExport int getFavoriteCivicAttitudeDivisor() const;
	DllExport int getFavoriteCivicAttitudeChangeLimit() const;
	DllExport int getDemandTributeAttitudeThreshold() const;
	DllExport int getNoGiveHelpAttitudeThreshold() const;
	DllExport int getTechRefuseAttitudeThreshold() const;
	DllExport int getStrategicBonusRefuseAttitudeThreshold() const;
	DllExport int getHappinessBonusRefuseAttitudeThreshold() const;
	DllExport int getHealthBonusRefuseAttitudeThreshold() const;
	DllExport int getMapRefuseAttitudeThreshold() const;
	DllExport int getDeclareWarRefuseAttitudeThreshold() const;
	DllExport int getDeclareWarThemRefuseAttitudeThreshold() const;
	DllExport int getStopTradingRefuseAttitudeThreshold() const;
	DllExport int getStopTradingThemRefuseAttitudeThreshold() const;
	DllExport int getAdoptCivicRefuseAttitudeThreshold() const;
	DllExport int getConvertReligionRefuseAttitudeThreshold() const;
	DllExport int getOpenBordersRefuseAttitudeThreshold() const;
	DllExport int getDefensivePactRefuseAttitudeThreshold() const;
	DllExport int getPermanentAllianceRefuseAttitudeThreshold() const;
	DllExport int getVassalRefuseAttitudeThreshold() const;
	DllExport int getVassalPowerModifier() const;
	DllExport int getFavoriteCivic() const;
	DllExport int getFavoriteReligion() const;
	DllExport int getFreedomAppreciation() const;

	DllExport const TCHAR* getArtDefineTag() const;
	DllExport void setArtDefineTag(const TCHAR* szVal);

	// Arrays

	DllExport bool hasTrait(int i) const;

	DllExport int getFlavorValue(int i) const;
	DllExport int getContactRand(int i) const;
	DllExport int getContactDelay(int i) const;
	DllExport int getMemoryDecayRand(int i) const;
	DllExport int getMemoryAttitudePercent(int i) const;
	DllExport int getNoWarAttitudeProb(int i) const;
	DllExport int getUnitAIWeightModifier(int i) const;
	DllExport int getImprovementWeightModifier(int i) const;
	DllExport int getDiploPeaceIntroMusicScriptIds(int i) const;
	DllExport int getDiploPeaceMusicScriptIds(int i) const;
	DllExport int getDiploWarIntroMusicScriptIds(int i) const;
	DllExport int getDiploWarMusicScriptIds(int i) const;

	// Other

	DllExport const CvArtInfoLeaderhead* getArtInfo() const;
	DllExport const TCHAR* getLeaderHead() const;
	DllExport const TCHAR* getButton() const;

	DllExport void write(FDataStreamBase* stream);
	DllExport void read(FDataStreamBase* stream);
	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	int m_iWonderConstructRand;
	int m_iBaseAttitude;
	int m_iBasePeaceWeight;
	int m_iPeaceWeightRand;
	int m_iWarmongerRespect;
	int m_iEspionageWeight;
	int m_iRefuseToTalkWarThreshold;
	int m_iNoTechTradeThreshold;
	int m_iTechTradeKnownPercent;
	int m_iMaxGoldTradePercent;
	int m_iMaxGoldPerTurnTradePercent;
	int m_iMaxWarRand;
	int m_iMaxWarNearbyPowerRatio;
	int m_iMaxWarDistantPowerRatio;
	int m_iMaxWarMinAdjacentLandPercent;
	int m_iLimitedWarRand;
	int m_iLimitedWarPowerRatio;
	int m_iDogpileWarRand;
	int m_iMakePeaceRand;
	int m_iDeclareWarTradeRand;
	int m_iDemandRebukedSneakProb;
	int m_iDemandRebukedWarProb;
	int m_iRazeCityProb;
	int m_iBuildUnitProb;
	int m_iBaseAttackOddsChange;
	int m_iAttackOddsChangeRand;
	int m_iWorseRankDifferenceAttitudeChange;
	int m_iBetterRankDifferenceAttitudeChange;
	int m_iCloseBordersAttitudeChange;
	int m_iLostWarAttitudeChange;
	int m_iAtWarAttitudeDivisor;
	int m_iAtWarAttitudeChangeLimit;
	int m_iAtPeaceAttitudeDivisor;
	int m_iAtPeaceAttitudeChangeLimit;
	int m_iSameReligionAttitudeChange;
	int m_iSameReligionAttitudeDivisor;
	int m_iSameReligionAttitudeChangeLimit;
	int m_iDifferentReligionAttitudeChange;
	int m_iDifferentReligionAttitudeDivisor;
	int m_iDifferentReligionAttitudeChangeLimit;
	int m_iBonusTradeAttitudeDivisor;
	int m_iBonusTradeAttitudeChangeLimit;
	int m_iOpenBordersAttitudeDivisor;
	int m_iOpenBordersAttitudeChangeLimit;
	int m_iDefensivePactAttitudeDivisor;
	int m_iDefensivePactAttitudeChangeLimit;
	int m_iShareWarAttitudeChange;
	int m_iShareWarAttitudeDivisor;
	int m_iShareWarAttitudeChangeLimit;
	int m_iFavoriteCivicAttitudeChange;
	int m_iFavoriteCivicAttitudeDivisor;
	int m_iFavoriteCivicAttitudeChangeLimit;
	int m_iDemandTributeAttitudeThreshold;
	int m_iNoGiveHelpAttitudeThreshold;
	int m_iTechRefuseAttitudeThreshold;
	int m_iStrategicBonusRefuseAttitudeThreshold;
	int m_iHappinessBonusRefuseAttitudeThreshold;
	int m_iHealthBonusRefuseAttitudeThreshold;
	int m_iMapRefuseAttitudeThreshold;
	int m_iDeclareWarRefuseAttitudeThreshold;
	int m_iDeclareWarThemRefuseAttitudeThreshold;
	int m_iStopTradingRefuseAttitudeThreshold;
	int m_iStopTradingThemRefuseAttitudeThreshold;
	int m_iAdoptCivicRefuseAttitudeThreshold;
	int m_iConvertReligionRefuseAttitudeThreshold;
	int m_iOpenBordersRefuseAttitudeThreshold;
	int m_iDefensivePactRefuseAttitudeThreshold;
	int m_iPermanentAllianceRefuseAttitudeThreshold;
	int m_iVassalRefuseAttitudeThreshold;
	int m_iVassalPowerModifier;
	int m_iFreedomAppreciation;
	int m_iFavoriteCivic;
	int m_iFavoriteReligion;

	CvString m_szArtDefineTag;

	// Arrays

	bool* m_pbTraits;

	int* m_piFlavorValue;
	int* m_piContactRand;
	int* m_piContactDelay;
	int* m_piMemoryDecayRand;
	int* m_piMemoryAttitudePercent;
	int* m_piNoWarAttitudeProb;
	int* m_piUnitAIWeightModifier;
	int* m_piImprovementWeightModifier;
	int* m_piDiploPeaceIntroMusicScriptIds;
	int* m_piDiploPeaceMusicScriptIds;
	int* m_piDiploWarIntroMusicScriptIds;
	int* m_piDiploWarMusicScriptIds;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvWorldInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvWorldInfo :
	public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvWorldInfo();
	DllExport virtual ~CvWorldInfo();

	DllExport int getDefaultPlayers() const;
	DllExport int getUnitNameModifier() const;
	DllExport int getTargetNumCities() const;
	DllExport int getNumFreeBuildingBonuses() const;
	DllExport int getBuildingClassPrereqModifier() const;
	DllExport int getMaxConscriptModifier() const;
	DllExport int getWarWearinessModifier() const;
	DllExport int getGridWidth() const;
	DllExport int getGridHeight() const;
	DllExport int getTerrainGrainChange() const;
	DllExport int getFeatureGrainChange() const;
	DllExport int getResearchPercent() const;
	DllExport int getTradeProfitPercent() const;
	DllExport int getDistanceMaintenancePercent() const;
	DllExport int getNumCitiesMaintenancePercent() const;
	DllExport int getColonyMaintenancePercent() const;
	DllExport int getCorporationMaintenancePercent() const;
	DllExport int getNumCitiesAnarchyPercent() const;
	DllExport int getAdvancedStartPointsMod() const;

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	int m_iDefaultPlayers;
	int m_iUnitNameModifier;
	int m_iTargetNumCities;
	int m_iNumFreeBuildingBonuses;
	int m_iBuildingClassPrereqModifier;
	int m_iMaxConscriptModifier;
	int m_iWarWearinessModifier;
	int m_iGridWidth;
	int m_iGridHeight;
	int m_iTerrainGrainChange;
	int m_iFeatureGrainChange;
	int m_iResearchPercent;
	int m_iTradeProfitPercent;
	int m_iDistanceMaintenancePercent;
	int m_iNumCitiesMaintenancePercent;
	int m_iColonyMaintenancePercent;
	int m_iCorporationMaintenancePercent;
	int m_iNumCitiesAnarchyPercent;
	int m_iAdvancedStartPointsMod;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  class : CvClimateInfo
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvClimateInfo :	public CvInfoBase
{
public:

	DllExport CvClimateInfo();
	DllExport virtual ~CvClimateInfo();

	DllExport int getDesertPercentChange() const;
	DllExport int getJungleLatitude() const;
	DllExport int getHillRange() const;
	DllExport int getPeakPercent() const;

	DllExport float getSnowLatitudeChange() const;
	DllExport float getTundraLatitudeChange() const;
	DllExport float getGrassLatitudeChange() const;
	DllExport float getDesertBottomLatitudeChange() const;
	DllExport float getDesertTopLatitudeChange() const;
	DllExport float getIceLatitude() const;
	DllExport float getRandIceLatitude() const;

	DllExport bool read(CvXMLLoadUtility* pXML);

protected:

	int m_iDesertPercentChange;
	int m_iJungleLatitude;
	int m_iHillRange;
	int m_iPeakPercent;

	float m_fSnowLatitudeChange;
	float m_fTundraLatitudeChange;
	float m_fGrassLatitudeChange;
	float m_fDesertBottomLatitudeChange;
	float m_fDesertTopLatitudeChange;
	float m_fIceLatitude;
	float m_fRandIceLatitude;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  class : CvSeaLevelInfo
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvSeaLevelInfo :	public CvInfoBase
{
public:

	DllExport CvSeaLevelInfo();
	DllExport virtual ~CvSeaLevelInfo();

	DllExport int getSeaLevelChange() const;

	DllExport bool read(CvXMLLoadUtility* pXML);

protected:

	int m_iSeaLevelChange;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvProcessInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvProcessInfo :
	public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvProcessInfo();
	DllExport virtual ~CvProcessInfo();

	DllExport int getTechPrereq() const;

	// Arrays

	DllExport int getProductionToCommerceModifier(int i) const;

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	int m_iTechPrereq;

	// Arrays

	int* m_paiProductionToCommerceModifier;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvVoteInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvVoteInfo :	public CvInfoBase
{
public:
	DllExport CvVoteInfo();
	DllExport virtual ~CvVoteInfo();

	DllExport int getPopulationThreshold() const;
	DllExport int getStateReligionVotePercent() const;
	DllExport int getTradeRoutes() const;
	DllExport int getMinVoters() const;

	DllExport bool isSecretaryGeneral() const;
	DllExport bool isVictory() const;
	DllExport bool isFreeTrade() const;
	DllExport bool isNoNukes() const;
	DllExport bool isCityVoting() const;
	DllExport bool isCivVoting() const;
	DllExport bool isDefensivePact() const;
	DllExport bool isOpenBorders() const;
	DllExport bool isForcePeace() const;
	DllExport bool isForceNoTrade() const;
	DllExport bool isForceWar() const;
	DllExport bool isAssignCity() const;

	// Arrays

	DllExport bool isForceCivic(int i) const;
	DllExport bool isVoteSourceType(int i) const;

	DllExport bool read(CvXMLLoadUtility* pXML);

protected:

	int m_iPopulationThreshold;
	int m_iStateReligionVotePercent;
	int m_iTradeRoutes;
	int m_iMinVoters;

	bool m_bSecretaryGeneral;
	bool m_bVictory;
	bool m_bFreeTrade;
	bool m_bNoNukes;
	bool m_bCityVoting;
	bool m_bCivVoting;
	bool m_bDefensivePact;
	bool m_bOpenBorders;
	bool m_bForcePeace;
	bool m_bForceNoTrade;
	bool m_bForceWar;
	bool m_bAssignCity;

	// Arrays

	bool* m_pbForceCivic;
	bool* m_abVoteSourceTypes;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvProjectInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvProjectInfo :
	public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvProjectInfo();
	DllExport virtual ~CvProjectInfo();

	DllExport int getVictoryPrereq() const;
	DllExport int getTechPrereq() const;
	DllExport int getAnyoneProjectPrereq() const;
	DllExport void setAnyoneProjectPrereq(int i);
	DllExport int getMaxGlobalInstances() const;
	DllExport int getMaxTeamInstances() const;
	DllExport int getProductionCost() const;
	DllExport int getNukeInterception() const;
	DllExport int getTechShare() const;
	DllExport int getEveryoneSpecialUnit() const;
	DllExport int getEveryoneSpecialBuilding() const;
	DllExport int getVictoryDelayPercent() const;
	DllExport int getSuccessRate() const;

	DllExport bool isSpaceship() const;
	DllExport bool isAllowsNukes() const;
	DllExport const char* getMovieArtDef() const;

	DllExport const TCHAR* getCreateSound() const;
	DllExport void setCreateSound(const TCHAR* szVal);

	// Arrays

	DllExport int getBonusProductionModifier(int i) const;
	DllExport int getVictoryThreshold(int i) const;
	DllExport int getVictoryMinThreshold(int i) const;
	DllExport int getProjectsNeeded(int i) const;

	DllExport bool read(CvXMLLoadUtility* pXML);
	DllExport bool readPass2(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	int m_iVictoryPrereq;
	int m_iTechPrereq;
	int m_iAnyoneProjectPrereq;
	int m_iMaxGlobalInstances;
	int m_iMaxTeamInstances;
	int m_iProductionCost;
	int m_iNukeInterception;
	int m_iTechShare;
	int m_iEveryoneSpecialUnit;
	int m_iEveryoneSpecialBuilding;
	int m_iVictoryDelayPercent;
	int m_iSuccessRate;

	bool m_bSpaceship;
	bool m_bAllowsNukes;

	CvString m_szCreateSound;
	CvString m_szMovieArtDef;

	// Arrays

	int* m_piBonusProductionModifier;
	int* m_piVictoryThreshold;
	int* m_piVictoryMinThreshold;
	int* m_piProjectsNeeded;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvReligionInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvReligionInfo : public CvHotkeyInfo
{
	//---------------------------------------PUBLIC INTERFACE----------------------------------------
public:

	DllExport CvReligionInfo();
	DllExport virtual ~CvReligionInfo();

	DllExport int getChar() const;
	DllExport void setChar(int i);
	DllExport int getHolyCityChar() const;
	DllExport void setHolyCityChar(int i);
	DllExport int getTechPrereq() const;
	DllExport int getFreeUnitClass() const;
	DllExport int getNumFreeUnits() const;
	DllExport int getSpreadFactor() const;
	DllExport int getMissionType() const;
	DllExport void setMissionType(int iNewType);

	DllExport const TCHAR* getTechButton() const;
	DllExport void setTechButton(const TCHAR* szVal);
	DllExport const TCHAR* getGenericTechButton() const;
	DllExport void setGenericTechButton(const TCHAR* szVal);
	DllExport const TCHAR* getMovieFile() const;
	DllExport void setMovieFile(const TCHAR* szVal);
	DllExport const TCHAR* getMovieSound() const;
	DllExport void setMovieSound(const TCHAR* szVal);
	DllExport const TCHAR* getSound() const;
	DllExport void setSound(const TCHAR* szVal);

	DllExport const TCHAR* getButtonDisabled() const;		//	Exposed to Python

	std::wstring pyGetAdjective(uint uiForm) { return getAdjective(uiForm);  }
	DllExport const wchar* getAdjective(uint uiForm = 0);
	DllExport void setAdjectiveKey(const TCHAR* szVal);
	DllExport const wchar* getAdjectiveKey() const;
	std::wstring pyGetAdjectiveKey() { return getAdjectiveKey(); }

	// Arrays

	DllExport int getGlobalReligionCommerce(int i) const;
	int* getGlobalReligionCommerceArray() const;
	DllExport int getHolyCityCommerce(int i) const;
	int* getHolyCityCommerceArray() const;
	DllExport int getStateReligionCommerce(int i) const;
	int* getStateReligionCommerceArray() const;

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	int m_iChar;
	int m_iHolyCityChar;
	int m_iTechPrereq;
	int m_iFreeUnitClass;
	int m_iNumFreeUnits;
	int m_iSpreadFactor;
	int m_iMissionType;

	CvString m_szTechButton;
	CvString m_szGenericTechButton;
	CvString m_szMovieFile;
	CvString m_szMovieSound;
	CvString m_szSound;
	CvString m_szAdjectiveKey;

	// Arrays

	int* m_paiGlobalReligionCommerce;
	int* m_paiHolyCityCommerce;
	int* m_paiStateReligionCommerce;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvCorporationInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvCorporationInfo : public CvHotkeyInfo
{
	//---------------------------------------PUBLIC INTERFACE----------------------------------------
public:

	DllExport CvCorporationInfo();
	DllExport virtual ~CvCorporationInfo();

	DllExport int getChar() const;
	DllExport void setChar(int i);
	DllExport int getHeadquarterChar() const;
	DllExport void setHeadquarterChar(int i);
	DllExport int getTechPrereq() const;
	DllExport int getFreeUnitClass() const;
	DllExport int getSpreadFactor() const;
	DllExport int getSpreadCost() const;
	DllExport int getMaintenance() const;
	DllExport int getMissionType() const;
	DllExport void setMissionType(int iNewType);

	DllExport int getBonusProduced() const;

	DllExport const TCHAR* getMovieFile() const;
	DllExport void setMovieFile(const TCHAR* szVal);
	DllExport const TCHAR* getMovieSound() const;
	DllExport void setMovieSound(const TCHAR* szVal);
	DllExport const TCHAR* getSound() const;
	DllExport void setSound(const TCHAR* szVal);

	// Arrays

	DllExport int getPrereqBonus(int i) const;
	DllExport int getHeadquarterCommerce(int i) const;
	int* getHeadquarterCommerceArray() const;
	DllExport int getCommerceProduced(int i) const;
	int* getCommerceProducedArray() const;
	DllExport int getYieldProduced(int i) const;
	int* getYieldProducedArray() const;

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	int m_iChar;
	int m_iHeadquarterChar;
	int m_iTechPrereq;
	int m_iFreeUnitClass;
	int m_iSpreadFactor;
	int m_iSpreadCost;
	int m_iMaintenance;
	int m_iMissionType;
	int m_iBonusProduced;

	CvString m_szMovieFile;
	CvString m_szMovieSound;
	CvString m_szSound;

	// Arrays

	int* m_paiPrereqBonuses;
	int* m_paiHeadquarterCommerce;
	int* m_paiCommerceProduced;
	int* m_paiYieldProduced;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvTraitInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvTraitInfo :
	public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvTraitInfo();
	DllExport virtual ~CvTraitInfo();

	DllExport int getHealth() const;
	DllExport int getHappiness() const;
	DllExport int getMaxAnarchy() const;
	DllExport int getUpkeepModifier() const;
	DllExport int getLevelExperienceModifier() const;
	DllExport int getGreatPeopleRateModifier() const;
	DllExport int getGreatGeneralRateModifier() const;
	DllExport int getDomesticGreatGeneralRateModifier() const;
	DllExport int getMaxGlobalBuildingProductionModifier() const;
	DllExport int getMaxTeamBuildingProductionModifier() const;
	DllExport int getMaxPlayerBuildingProductionModifier() const;

	DllExport const TCHAR* getShortDescription() const;
	void setShortDescription(const TCHAR* szVal);

	// Arrays

	DllExport int getExtraYieldThreshold(int i) const;
	DllExport int getTradeYieldModifier(int i) const;
	DllExport int getCommerceChange(int i) const;
	DllExport int getCommerceModifier(int i) const;

	DllExport int isFreePromotion(int i) const;
	DllExport int isFreePromotionUnitCombat(int i) const;

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	int m_iHealth;
	int m_iHappiness;
	int m_iMaxAnarchy;
	int m_iUpkeepModifier;
	int m_iLevelExperienceModifier;
	int m_iGreatPeopleRateModifier;
	int m_iGreatGeneralRateModifier;
	int m_iDomesticGreatGeneralRateModifier;
	int m_iMaxGlobalBuildingProductionModifier;
	int m_iMaxTeamBuildingProductionModifier;
	int m_iMaxPlayerBuildingProductionModifier;

	CvString m_szShortDescription;

	// Arrays

	int* m_paiExtraYieldThreshold;
	int* m_paiTradeYieldModifier;
	int* m_paiCommerceChange;
	int* m_paiCommerceModifier;

	bool* m_pabFreePromotion;
	bool* m_pabFreePromotionUnitCombat;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvCursorInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvCursorInfo : public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvCursorInfo();
	DllExport virtual ~CvCursorInfo();

	DllExport const TCHAR* getPath();
	DllExport void setPath(const TCHAR* szVal);

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	CvString m_szPath;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvThroneRoomCamera
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvThroneRoomCamera : public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvThroneRoomCamera();
	DllExport virtual ~CvThroneRoomCamera();

	DllExport const TCHAR* getFileName();
	DllExport void setFileName(const TCHAR* szVal);

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	CvString m_szFileName;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvThroneRoomInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvThroneRoomInfo : public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvThroneRoomInfo();
	DllExport virtual ~CvThroneRoomInfo();

	DllExport const TCHAR* getEvent();
	DllExport void setEvent(const TCHAR* szVal);
	DllExport const TCHAR* getNodeName();
	DllExport void setNodeName(const TCHAR* szVal);
	DllExport int getFromState();
	DllExport void setFromState(int iVal);
	DllExport int getToState();
	DllExport void setToState(int iVal);
	DllExport int getAnimation();
	DllExport void setAnimation(int iVal);

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	int m_iFromState;
	int m_iToState;
	int m_iAnimation;
	CvString m_szEvent;
	CvString m_szNodeName;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvThroneRoomStyleInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvThroneRoomStyleInfo : public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvThroneRoomStyleInfo();
	DllExport virtual ~CvThroneRoomStyleInfo();

	DllExport const TCHAR* getArtStyleType();
	DllExport void setArtStyleType(const TCHAR* szVal);
	DllExport const TCHAR* getEraType();
	DllExport void setEraType(const TCHAR* szVal);
	DllExport const TCHAR* getFileName();
	DllExport void setFileName(const TCHAR* szVal);

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	CvString m_szArtStyleType;
	CvString m_szEraType;
	CvString m_szFileName;
	std::vector<CvString> m_aNodeNames;
	std::vector<CvString> m_aTextureNames;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvSlideShowInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvSlideShowInfo : public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvSlideShowInfo();
	DllExport virtual ~CvSlideShowInfo();

	DllExport const TCHAR* getPath();
	DllExport void setPath(const TCHAR* szVal);
	DllExport const TCHAR* getTransitionType();
	DllExport void setTransitionType(const TCHAR* szVal);
	DllExport float getStartTime();
	DllExport void setStartTime(float fVal);

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	float m_fStartTime;
	CvString m_szPath;
	CvString m_szTransitionType;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvSlideShowRandomInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvSlideShowRandomInfo : public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvSlideShowRandomInfo();
	DllExport virtual ~CvSlideShowRandomInfo();

	DllExport const TCHAR* getPath();
	DllExport void setPath(const TCHAR* szVal);

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	CvString m_szPath;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvWorldPickerInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvWorldPickerInfo : public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvWorldPickerInfo();
	DllExport virtual ~CvWorldPickerInfo();

	DllExport const TCHAR* getMapName();
	DllExport void setMapName(const TCHAR* szVal);
	DllExport const TCHAR* getModelFile();
	DllExport void setModelFile(const TCHAR* szVal);
	DllExport int getNumSizes();
	DllExport float getSize(int index);
	DllExport int getNumClimates();
	DllExport const TCHAR* getClimatePath(int index);
	DllExport int getNumWaterLevelDecals();
	DllExport const TCHAR* getWaterLevelDecalPath(int index);
	DllExport int getNumWaterLevelGloss();
	DllExport const TCHAR* getWaterLevelGlossPath(int index);

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	CvString m_szMapName;
	CvString m_szModelFile;
	std::vector<float> m_aSizes;
	std::vector<CvString> m_aClimates;
	std::vector<CvString> m_aWaterLevelDecals;
	std::vector<CvString> m_aWaterLevelGloss;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvSpaceShipInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvSpaceShipInfo : public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvSpaceShipInfo();
	DllExport virtual ~CvSpaceShipInfo();

	DllExport const TCHAR* getNodeName();
	DllExport void setNodeName(const TCHAR* szVal);
	DllExport const TCHAR* getProjectName();
	DllExport void setProjectName(const TCHAR* szVal);
	DllExport ProjectTypes getProjectType();
	DllExport AxisTypes getCameraUpAxis();
	DllExport SpaceShipInfoTypes getSpaceShipInfoType();
	DllExport int getPartNumber();
	DllExport int getArtType();
	DllExport int getEventCode();

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	CvString m_szNodeName;
	CvString m_szProjectName;
	ProjectTypes m_eProjectType;
	AxisTypes m_eCameraUpAxis;
	int m_iPartNumber;
	int m_iArtType;
	int m_iEventCode;
	SpaceShipInfoTypes m_eSpaceShipInfoType;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvAnimationInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef std::vector<std::pair<int,float> > CvAnimationPathDefinition;
typedef std::pair<int,int >			CvAnimationCategoryDefinition;

class CvAnimationPathInfo : public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
	public:

		DllExport CvAnimationPathInfo();
		DllExport virtual ~CvAnimationPathInfo();

		DllExport int getPathCategory( int i );
		DllExport float getPathParameter( int i );
		DllExport int getNumPathDefinitions();
		DllExport CvAnimationPathDefinition * getPath( );
		DllExport bool isMissionPath() const;

		DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PRIVATE MEMBER VARIABLES---------------------------------
	private:

		CvAnimationPathDefinition 	m_vctPathDefinition;	//!< Animation path definitions, pair(category,param).
		bool						m_bMissionPath;			//!< True if this animation is used in missions
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvAnimationInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvAnimationCategoryInfo : public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
	public:

		DllExport CvAnimationCategoryInfo();
		DllExport virtual ~CvAnimationCategoryInfo();

		DllExport int getCategoryBaseID( );
		DllExport int getCategoryDefaultTo( );

		DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PRIVATE MEMBER VARIABLES---------------------------------
	private:

		CvAnimationCategoryDefinition	m_kCategory;		//!< The pair(base IDs, default categories) defining the animation categories
		CvString						m_szDefaultTo;		//!< Holds the default to parameter, until all categories are read
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvEntityEventInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvEntityEventInfo : public CvInfoBase
{
		//---------------------------------------PUBLIC INTERFACE---------------------------------
	public:

		DllExport CvEntityEventInfo();
		DllExport virtual ~CvEntityEventInfo();

		DllExport bool read(CvXMLLoadUtility* pXML);

		DllExport AnimationPathTypes getAnimationPathType(int iIndex = 0) const;
		DllExport EffectTypes getEffectType(int iIndex = 0) const;
		DllExport int getAnimationPathCount() const;
		DllExport int getEffectTypeCount() const;

		DllExport bool getUpdateFormation() const;

		//---------------------------------------PRIVATE MEMBER VARIABLES---------------------------------
	private:

		std::vector<AnimationPathTypes>	m_vctAnimationPathType;
		std::vector<EffectTypes>		m_vctEffectTypes;
		bool							m_bUpdateFormation;
};

// The below classes are for the ArtFile Management
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  classes : CvArtInfos
//
// This is also an abstract BASE class
//
//  DESC:  Used to store data from Art\Civ4ArtDefines.xml
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvAssetInfoBase : public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvAssetInfoBase()  {}
	DllExport virtual ~CvAssetInfoBase() {}

	DllExport const TCHAR* getTag() const;
	DllExport void setTag(const TCHAR* szDesc);

	DllExport const TCHAR* getPath() const;
	DllExport void setPath(const TCHAR* szDesc);

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	CvString m_szPath;

};

class CvArtInfoAsset : 	public CvAssetInfoBase
{
public:

	DllExport CvArtInfoAsset() {}
	DllExport virtual ~CvArtInfoAsset() {}

	DllExport const TCHAR* getNIF() const;
	DllExport const TCHAR* getKFM() const;

	DllExport void setNIF(const TCHAR* szDesc);
	DllExport void setKFM(const TCHAR* szDesc);

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	CvString m_szKFM;
	CvString m_szNIF;
};

//
//////////////////////////////////////////////////////////////////////////
// Another base class
//////////////////////////////////////////////////////////////////////////

class CvArtInfoScalableAsset :
	public CvArtInfoAsset,
	public CvScalableInfo
{
public:

	DllExport bool read(CvXMLLoadUtility* pXML);

};

// todoJS: Remove empty classes if additional items are not added

class CvArtInfoInterface : 	public CvArtInfoAsset
{
public:

	DllExport CvArtInfoInterface() {}
	DllExport virtual ~CvArtInfoInterface() {}

};

class CvArtInfoMisc : 	public CvArtInfoScalableAsset
{
public:

	DllExport CvArtInfoMisc() {}
	DllExport virtual ~CvArtInfoMisc() {}

};

class CvArtInfoMovie : 	public CvArtInfoAsset
{
public:

	DllExport CvArtInfoMovie() {}
	DllExport virtual ~CvArtInfoMovie() {}

};

class CvArtInfoUnit : public CvArtInfoScalableAsset
{
public:

	DllExport CvArtInfoUnit();
	DllExport virtual ~CvArtInfoUnit();

	DllExport bool getActAsRanged() const;
	DllExport bool getActAsLand() const;
	DllExport bool getActAsAir() const;

	DllExport const TCHAR* getShaderNIF() const;
	DllExport void setShaderNIF(const TCHAR* szDesc);

	DllExport const TCHAR* getShadowNIF() const;
	DllExport float getShadowScale() const;
	DllExport const TCHAR* getShadowAttachNode() const;
	DllExport int getDamageStates() const;

	DllExport const TCHAR* getTrailTexture() const;
	DllExport float getTrailWidth() const;
	DllExport float getTrailLength() const;
	DllExport float getTrailTaper() const;
	DllExport float getTrailFadeStarTime() const;
	DllExport float getTrailFadeFalloff() const;

	DllExport float getBattleDistance() const;
	DllExport float getRangedDeathTime() const;
	DllExport float getExchangeAngle() const;
	DllExport bool getCombatExempt() const;
	DllExport bool getSmoothMove() const;
	DllExport float getAngleInterpRate() const;
	DllExport float getBankRate() const;

	DllExport bool read(CvXMLLoadUtility* pXML);

	DllExport const TCHAR* getTrainSound() const;
	DllExport void setTrainSound(const TCHAR* szVal);
	DllExport int getRunLoopSoundTag() const;
	DllExport int getRunEndSoundTag() const;
	DllExport int getPatrolSoundTag() const;
	DllExport int getSelectionSoundScriptId() const;
	DllExport int getActionSoundScriptId() const;

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:
	CvString m_szShaderNIF;		//!< The NIF used if the graphics card supports shaders
	CvString m_szShadowNIF;		//!< The shadow blob NIF to use for the unit
	CvString m_szShadowAttach;	//!< The name of the node to which the shadow takes its x,y position

	float m_fShadowScale;		//!< the scale of the unit's shadow.

	int m_iDamageStates;		//!< The maximum number of damage states this unit type supports
	bool m_bActAsRanged;		//!< true if the unit acts as a ranged unit in combat (but may or may not be actually a ranged unit)
	bool m_bActAsLand;
	bool m_bActAsAir;
	bool m_bCombatExempt;		//!< true if the unit is 'exempt' from combat - ie. it just flees instead of dying
	bool m_bSmoothMove;			//!< true if the unit should do non-linear interpolation for moves

	CvString m_szTrailTexture;	//!< The trail texture of the unit
	float m_fTrailWidth;		//!< The width of the trail
	float m_fTrailLength;		//!< The length of the trail
	float m_fTrailTaper;		//!< Tapering of the trail
	float m_fTrailFadeStartTime;//!< Time after which the trail starts to fade
	float m_fTrailFadeFalloff;	//!< Speed at which the fade happens

	float m_fBattleDistance;	//!< The preferred attack distance of this unit (1.0 == plot size)
	float m_fRangedDeathTime;	//!< The offset from firing in which an opponent should die
	float m_fExchangeAngle;		//!< The angle at which the unit does combat.
	float m_fAngleInterRate;	//!< The rate at which the units' angle interpolates
	float m_fBankRate;

	CvString m_szTrainSound;
	int m_iRunLoopSoundTag;
	int m_iRunEndSoundTag;
	int m_iPatrolSoundTag;
	int m_iSelectionSoundScriptId;
	int m_iActionSoundScriptId;
};

class CvArtInfoBuilding : public CvArtInfoScalableAsset
{
public:

	DllExport CvArtInfoBuilding();
	DllExport virtual ~CvArtInfoBuilding();

	DllExport bool isAnimated() const;
	DllExport const TCHAR* getLSystemName() const;

	DllExport bool read(CvXMLLoadUtility* pXML);

protected:

	bool m_bAnimated;
	CvString m_szLSystemName;

};

class CvArtInfoCivilization : public CvArtInfoAsset
{
public:

	DllExport CvArtInfoCivilization();
	DllExport virtual ~CvArtInfoCivilization();

	DllExport bool isWhiteFlag() const;

	DllExport bool read(CvXMLLoadUtility* pXML);

protected:

	bool m_bWhiteFlag;

};

class CvArtInfoLeaderhead : public CvArtInfoAsset
{
public:

	DllExport CvArtInfoLeaderhead() {}
	DllExport virtual ~CvArtInfoLeaderhead() {}

	DllExport const TCHAR* getNoShaderNIF() const;
	DllExport void setNoShaderNIF(const TCHAR* szNIF);
	DllExport const TCHAR* getBackgroundKFM() const;
	DllExport void setBackgroundKFM( const TCHAR* szKFM);

	DllExport bool read(CvXMLLoadUtility* pXML);

protected:

	CvString m_szNoShaderNIF;
	CvString m_szBackgroundKFM;
};

class CvArtInfoBonus : public CvArtInfoScalableAsset
{
public:
	DllExport CvArtInfoBonus();
	DllExport virtual ~CvArtInfoBonus() {}

	DllExport int getFontButtonIndex() const;

	DllExport const TCHAR* getShaderNIF() const;
	DllExport void setShaderNIF(const TCHAR* szDesc);

	DllExport bool read(CvXMLLoadUtility* pXML);

protected:
	CvString m_szShaderNIF;		//!< The NIF used if the graphics card supports shaders
	int m_iFontButtonIndex;
};

class CvArtInfoImprovement : public CvArtInfoScalableAsset
{
public:

	DllExport CvArtInfoImprovement();
	DllExport virtual ~CvArtInfoImprovement();

	DllExport const TCHAR* getShaderNIF() const;
	DllExport void setShaderNIF(const TCHAR* szDesc);

	DllExport bool isExtraAnimations() const;

	DllExport bool read(CvXMLLoadUtility* pXML);

protected:
	CvString m_szShaderNIF;		//!< The NIF used if the graphics card supports shaders

	bool m_bExtraAnimations;

};

typedef std::vector<std::pair<int, int> > CvTextureBlendSlotList;

class CvArtInfoTerrain : public CvArtInfoAsset
{
public:

	DllExport CvArtInfoTerrain();
	DllExport virtual ~CvArtInfoTerrain();

	DllExport const TCHAR* getBaseTexture();
	DllExport void setBaseTexture(const TCHAR* szTmp );
	DllExport const TCHAR* getGridTexture();
	DllExport void setGridTexture(const TCHAR* szTmp );
	DllExport const TCHAR* getDetailTexture();
	DllExport void setDetailTexture(const TCHAR* szTmp);
	DllExport int getLayerOrder();
	DllExport bool useAlphaShader();
	DllExport CvTextureBlendSlotList &getBlendList(int blendMask);

	DllExport bool read(CvXMLLoadUtility* pXML);

protected:

	CvString m_szDetailTexture;				//!< Detail texture associated with the Terrain base texture
	CvString m_szGridTexture;

	int m_iLayerOrder;									//!< Layering order of texture
	bool m_bAlphaShader;
	int m_numTextureBlends;						//!< number to blend textures.
	CvTextureBlendSlotList  **m_pTextureSlots;	//!< Array of Textureslots per blend tile
};

class CvArtInfoFeature : public CvArtInfoScalableAsset
{
public:

	DllExport CvArtInfoFeature();
	DllExport virtual ~CvArtInfoFeature();

	DllExport bool isAnimated() const;
	DllExport bool isRiverArt() const;
	DllExport TileArtTypes getTileArtType() const;
	DllExport LightTypes getLightType() const;

	DllExport bool read(CvXMLLoadUtility* pXML);

	class FeatureArtModel
	{
	public:
		FeatureArtModel(const CvString &modelFile, RotationTypes rotation)
		{
			m_szModelFile = modelFile;
			m_eRotation = rotation;
		}

		const CvString &getModelFile() const
		{
			return m_szModelFile;
		}

		RotationTypes getRotation() const
		{
			return m_eRotation;
		}

	private:
		CvString m_szModelFile;
		RotationTypes m_eRotation;
	};

	class FeatureArtPiece
	{
	public:
		FeatureArtPiece(int connectionMask)
		{
			m_iConnectionMask = connectionMask;
		}

		int getConnectionMask() const
		{
			return m_iConnectionMask;
		}

		int getNumArtModels() const
		{
			return m_aArtModels.size();
		}

		const FeatureArtModel &getArtModel(int index) const
		{
			FAssertMsg((index >= 0) && (index < (int) m_aArtModels.size()), "[Jason] Invalid feature model file index.");
			return m_aArtModels[index];
		}

	private:
		std::vector<FeatureArtModel> m_aArtModels;
		int m_iConnectionMask;

		friend CvArtInfoFeature;
	};

	class FeatureDummyNode
	{
	public:
		FeatureDummyNode(const CvString &tagName, const CvString &nodeName)
		{
			m_szTag = tagName;
			m_szName = nodeName;
		}

		const CvString getTagName() const
		{
			return m_szTag;
		}

		const CvString getNodeName() const
		{
			return m_szName;
		}

	private:
		CvString m_szTag;
		CvString m_szName;
	};

	class FeatureVariety
	{
	public:
		FeatureVariety()
		{
		}

		const CvString &getVarietyButton() const
		{
			return m_szVarietyButton;
		}

		const FeatureArtPiece &getFeatureArtPiece(int index) const
		{
			FAssertMsg((index >= 0) && (index < (int) m_aFeatureArtPieces.size()), "[Jason] Invalid feature art index.");
			return m_aFeatureArtPieces[index];
		}

		const FeatureArtPiece &getFeatureArtPieceFromConnectionMask(int connectionMask) const
		{
			for(int i=0;i<(int)m_aFeatureArtPieces.size();i++)
				if(m_aFeatureArtPieces[i].getConnectionMask() == connectionMask)
					return m_aFeatureArtPieces[i];

			FAssertMsg(false, "[Jason] Failed to find feature art piece with valid connection mask.");
			return m_aFeatureArtPieces[0];
		}

		const CvString getFeatureDummyNodeName(const CvString &tagName) const
		{
			for(int i=0;i<(int)m_aFeatureDummyNodes.size();i++)
			{
				if(m_aFeatureDummyNodes[i].getTagName().CompareNoCase(tagName) == 0)
					return m_aFeatureDummyNodes[i].getNodeName();
			}

			FAssertMsg(false, "[Jason] Failed to find dummy tag name.");
			return "";
		}

		const CvString getFeatureDummyTag(const CvString &nodeName) const
		{
			for(int i=0;i<(int)m_aFeatureDummyNodes.size();i++)
			{
				if(m_aFeatureDummyNodes[i].getNodeName().CompareNoCase(nodeName) == 0)
					return m_aFeatureDummyNodes[i].getTagName();
			}

			return "";
		}

		FeatureArtPiece &createFeatureArtPieceFromConnectionMask(int connectionMask)
		{
			for(int i=0;i<(int)m_aFeatureArtPieces.size();i++)
				if(m_aFeatureArtPieces[i].getConnectionMask() == connectionMask)
					return m_aFeatureArtPieces[i];

			m_aFeatureArtPieces.push_back(FeatureArtPiece(connectionMask));
			return m_aFeatureArtPieces.back();
		}

		void createFeatureDummyNode(const CvString &tagName, const CvString &nodeName)
		{
			m_aFeatureDummyNodes.push_back(FeatureDummyNode(tagName, nodeName));
		}

	private:
		std::vector<FeatureArtPiece> m_aFeatureArtPieces;
		std::vector<FeatureDummyNode> m_aFeatureDummyNodes;
		CvString m_szVarietyButton;

		friend CvArtInfoFeature;
	};

	DllExport const FeatureVariety &getVariety(int index) const;
	DllExport int getNumVarieties() const;
	DllExport std::string getFeatureDummyNodeName(int variety, std::string tagName);

protected:

	int getConnectionMaskFromString(const CvString &connectionString);
	int getRotatedConnectionMask(int connectionMask, RotationTypes rotation);

	bool m_bAnimated;
	bool m_bRiverArt;
	TileArtTypes m_eTileArtType;
	LightTypes m_eLightType;
	std::vector<FeatureVariety> m_aFeatureVarieties;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvEmphasizeInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvEmphasizeInfo :
	public CvInfoBase
{
//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvEmphasizeInfo();
	DllExport virtual ~CvEmphasizeInfo();

	DllExport bool isAvoidGrowth() const;
	DllExport bool isGreatPeople() const;

	// Arrays

	DllExport int getYieldChange(int i) const;
	DllExport int getCommerceChange(int i) const;

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	bool m_bAvoidGrowth;
	bool m_bGreatPeople;

	// Arrays

	int* m_piYieldModifiers;
	int* m_piCommerceModifiers;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvUpkeepInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvUpkeepInfo :
	public CvInfoBase
{
//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvUpkeepInfo();
	DllExport virtual ~CvUpkeepInfo();

	DllExport int getPopulationPercent() const;			//	Exposed to Python
	DllExport int getCityPercent() const;						//	Exposed to Python

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	int m_iPopulationPercent;
	int m_iCityPercent;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvCultureLevelInfo
//
//  DESC:
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvCultureLevelInfo :
	public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvCultureLevelInfo();
	DllExport virtual ~CvCultureLevelInfo();

	DllExport int getCityDefenseModifier() const;		//	Exposed to Python

	DllExport int getSpeedThreshold(int i) const;		//	Exposed to Python

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	int m_iCityDefenseModifier;

	int* m_paiSpeedThreshold;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvEraInfo
//
//  DESC:   Used to manage different types of Art Styles
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvEraInfo :
	public CvInfoBase
{
public:

	DllExport CvEraInfo();
	DllExport virtual ~CvEraInfo();

	DllExport int getStartingUnitMultiplier() const;		//	Exposed to Python
	DllExport int getStartingDefenseUnits() const;			//	Exposed to Python
	DllExport int getStartingWorkerUnits() const;				//	Exposed to Python
	DllExport int getStartingExploreUnits() const;			//	Exposed to Python
	DllExport int getAdvancedStartPoints() const;					//	Exposed to Python
	DllExport int getStartingGold() const;					//	Exposed to Python
	DllExport int getFreePopulation() const;				//	Exposed to Python
	DllExport int getStartPercent() const;					//	Exposed to Python
	DllExport int getGrowthPercent() const;					//	Exposed to Python
	DllExport int getTrainPercent() const;					//	Exposed to Python
	DllExport int getConstructPercent() const;			//	Exposed to Python
	DllExport int getCreatePercent() const;					//	Exposed to Python
	DllExport int getResearchPercent() const;				//	Exposed to Python
	DllExport int getBuildPercent() const;					//	Exposed to Python
	DllExport int getImprovementPercent() const;		//	Exposed to Python
	DllExport int getGreatPeoplePercent() const;		//	Exposed to Python
	DllExport int getAnarchyPercent() const;				//	Exposed to Python
	DllExport int getEventChancePerTurn() const;				//	Exposed to Python
	DllExport int getSoundtrackSpace() const;				//	Exposed to Python
	DllExport int getNumSoundtracks() const;				//	Exposed to Python
	DllExport const TCHAR* getAudioUnitVictoryScript() const;				//	Exposed to Python
	DllExport const TCHAR* getAudioUnitDefeatScript() const;				//	Exposed to Python

	DllExport bool isNoGoodies() const;					//	Exposed to Python
	DllExport bool isNoAnimals() const;					//	Exposed to Python
	DllExport bool isNoBarbUnits() const;				//	Exposed to Python
	DllExport bool isNoBarbCities() const;			//	Exposed to Python
	DllExport bool isFirstSoundtrackFirst() const;			//	Exposed to Python

	// Arrays

	DllExport int getSoundtracks(int i) const;
	DllExport int getCitySoundscapeSciptId(int i) const;

	DllExport bool read(CvXMLLoadUtility* pXML);

protected:

	int m_iStartingUnitMultiplier;
	int m_iStartingDefenseUnits;
	int m_iStartingWorkerUnits;
	int m_iStartingExploreUnits;
	int m_iAdvancedStartPoints;
	int m_iStartingGold;
	int m_iFreePopulation;
	int m_iStartPercent;
	int m_iGrowthPercent;
	int m_iTrainPercent;
	int m_iConstructPercent;
	int m_iCreatePercent;
	int m_iResearchPercent;
	int m_iBuildPercent;
	int m_iImprovementPercent;
	int m_iGreatPeoplePercent;
	int m_iAnarchyPercent;
	int m_iEventChancePerTurn;
	int m_iSoundtrackSpace;
	int m_iNumSoundtracks;
	CvString m_szAudioUnitVictoryScript;
	CvString m_szAudioUnitDefeatScript;

	bool m_bNoGoodies;
	bool m_bNoAnimals;
	bool m_bNoBarbUnits;
	bool m_bNoBarbCities;
	bool m_bFirstSoundtrackFirst;

	// Arrays

	int* m_paiSoundtracks;
	int* m_paiCitySoundscapeSciptIds;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvColorInfo
//
//  DESC:   Used to manage different types of Art Styles
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvColorInfo :
	public CvInfoBase
{
public:

	DllExport CvColorInfo();
	DllExport virtual ~CvColorInfo();

	DllExport const NiColorA& getColor() const;

	DllExport bool read(CvXMLLoadUtility* pXML);

protected:

	NiColorA m_Color;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvPlayerColorInfo (ADD to Python)
//
//  DESC:   Used to manage different types of Art Styles
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvPlayerColorInfo :
	public CvInfoBase
{
public:

	DllExport CvPlayerColorInfo();
	DllExport virtual ~CvPlayerColorInfo();

	DllExport int getColorTypePrimary() const;
	DllExport int getColorTypeSecondary() const;
	DllExport int getTextColorType() const;

	DllExport bool read(CvXMLLoadUtility* pXML);

protected:

	int m_iColorTypePrimary;
	int m_iColorTypeSecondary;
	int m_iTextColorType;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvLandscapeInfo
//
//  Purpose:	This info acts as the Civ4Terrain.ini and is initialize in CvXmlLoadUtility with the infos in
//					XML/Terrain/TerrainSettings.xml
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvLandscapeInfo :
	public CvInfoBase
{
	public:

		DllExport CvLandscapeInfo();
		DllExport virtual ~CvLandscapeInfo() {}

		DllExport int getFogR() const;
		DllExport int getFogG() const;
		DllExport int getFogB() const;
		DllExport int getHorizontalGameCell() const;
		DllExport int getVerticalGameCell() const;
		DllExport int getPlotsPerCellX() const;
		DllExport int getPlotsPerCellY() const;
		DllExport int getHorizontalVertCnt() const;
		DllExport int getVerticalVertCnt() const;
		DllExport int getWaterHeight() const;

		DllExport float getTextureScaleX() const;
		DllExport float getTextureScaleY() const;
		DllExport float getZScale() const;

		DllExport bool isUseTerrainShader() const;
		DllExport bool isUseLightmap() const;
		DllExport bool isRandomMap() const;
		DllExport float getPeakScale() const;
		DllExport float getHillScale() const;

		DllExport const TCHAR* getSkyArt();
		DllExport void setSkyArt(const TCHAR* szPath);
		DllExport const TCHAR* getHeightMap();
		DllExport void setHeightMap(const TCHAR* szPath);
		DllExport const TCHAR* getTerrainMap();
		DllExport void setTerrainMap(const TCHAR* szPath);
		DllExport const TCHAR* getNormalMap();
		DllExport void setNormalMap(const TCHAR* szPath);
		DllExport const TCHAR* getBlendMap();
		DllExport void setBlendMap(const TCHAR* szPath);

		DllExport bool read(CvXMLLoadUtility* pXML);

	protected:

		int m_iFogR;
		int m_iFogG;
		int m_iFogB;
		int m_iHorizontalGameCell;
		int m_iVerticalGameCell;
		int m_iPlotsPerCellX;
		int m_iPlotsPerCellY;
		int m_iHorizontalVertCnt;
		int m_iVerticalVertCnt;
		int m_iWaterHeight;

		float m_fTextureScaleX;
		float m_fTextureScaleY;
		float m_fZScale;

		float m_fPeakScale;
		float m_fHillScale;

		bool m_bUseTerrainShader;
		bool m_bUseLightmap;
		bool m_bRandomMap;

		CvString m_szSkyArt;
		CvString m_szHeightMap;
		CvString m_szTerrainMap;
		CvString m_szNormalMap;
		CvString m_szBlendMap;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvGameText
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvGameText : public CvInfoBase
{
public:
	DllExport CvGameText();

	const wchar* getText() const;
	void setText(const wchar* szText);

	// for Python
	std::wstring pyGetText() const { return getText(); }

	void setGender(const wchar* szGender) { m_szGender = szGender;	}
	const wchar* getGender() const { return m_szGender; }

	void setPlural(const wchar* szPlural) { m_szPlural = szPlural; }
	const wchar* getPlural() const { return m_szPlural; }

	DllExport int getNumLanguages() const; // not static for Python access
	DllExport void setNumLanguages(int iNum); // not static for Python access

	bool read(CvXMLLoadUtility* pXML);

protected:

	CvWString m_szText;
	CvWString m_szGender;
	CvWString m_szPlural;

	static int NUM_LANGUAGES;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvDiplomacyTextInfo
//
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvDiplomacyTextInfo :	public CvInfoBase
{
	friend class CvXMLLoadUtility;		// so it can access private vars to initialize the class
public:
	struct Response
	{
		DllExport Response() :
			m_iNumDiplomacyText(0),
			m_pbCivilizationTypes(NULL),
			m_pbLeaderHeadTypes(NULL),
			m_pbAttitudeTypes(NULL),
			m_pbDiplomacyPowerTypes(NULL),
			m_paszDiplomacyText(NULL)
		{
		}

		DllExport virtual ~Response ()
		{
			SAFE_DELETE_ARRAY(m_pbCivilizationTypes);
			SAFE_DELETE_ARRAY(m_pbLeaderHeadTypes);
			SAFE_DELETE_ARRAY(m_pbAttitudeTypes);
			SAFE_DELETE_ARRAY(m_pbDiplomacyPowerTypes);
			SAFE_DELETE_ARRAY(m_paszDiplomacyText);
		}

		void read(FDataStreamBase* stream);
		void write(FDataStreamBase* stream);

		int m_iNumDiplomacyText;
		bool* m_pbCivilizationTypes;
		bool* m_pbLeaderHeadTypes;
		bool* m_pbAttitudeTypes;
		bool* m_pbDiplomacyPowerTypes;
		CvString* m_paszDiplomacyText;	// needs to be public for xml load assignment
	};

	DllExport CvDiplomacyTextInfo();
	DllExport virtual ~CvDiplomacyTextInfo() { uninit(); }	// free memory - MT

	// note - Response member vars allocated by CvXmlLoadUtility
	DllExport void init(int iNum);
	DllExport void uninit();

	DllExport const Response& getResponse(int iNum) const { return m_pResponses[iNum]; }
	DllExport int getNumResponses() const;

	DllExport bool getCivilizationTypes(int i, int j) const;
	DllExport bool getLeaderHeadTypes(int i, int j) const;
	DllExport bool getAttitudeTypes(int i, int j) const;
	DllExport bool getDiplomacyPowerTypes(int i, int j) const;

	DllExport int getNumDiplomacyText(int i) const;

	DllExport const TCHAR* getDiplomacyText(int i, int j) const;

	void read(FDataStreamBase* stream);
	void write(FDataStreamBase* stream);
	DllExport bool read(CvXMLLoadUtility* pXML);

private:

	int m_iNumResponses;			// set by init
	Response* m_pResponses;

};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvEffectInfo
//
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvEffectInfo :
	public CvInfoBase,
	public CvScalableInfo
{
public:

	DllExport CvEffectInfo();
	DllExport virtual ~CvEffectInfo();

	DllExport const TCHAR* getPath() const { return m_szPath; }
	DllExport void setPath(const TCHAR* szVal) { m_szPath = szVal; }
	DllExport float getUpdateRate( ) const { return m_fUpdateRate; };
	DllExport void setUpdateRate( float fUpdateRate ) { m_fUpdateRate = fUpdateRate; }
	DllExport bool isProjectile() const { return m_bProjectile; };
	DllExport float getProjectileSpeed() const { return m_fProjectileSpeed; };
	DllExport float getProjectileArc() const { return m_fProjectileArc; };
	DllExport bool isSticky() const { return m_bSticky; };
	DllExport bool read(CvXMLLoadUtility* pXML);

private:
	CvString m_szPath;
	float m_fUpdateRate;
	bool m_bProjectile;
	bool m_bSticky;
	float m_fProjectileSpeed;
	float m_fProjectileArc;
};



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvAttachableInfo
//
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvAttachableInfo :
	public CvInfoBase,
	public CvScalableInfo
{
public:

	DllExport CvAttachableInfo();
	DllExport virtual ~CvAttachableInfo();

	DllExport const TCHAR* getPath() const { return m_szPath; }
	DllExport void setPath(const TCHAR* szVal) { m_szPath = szVal; }

	DllExport bool read(CvXMLLoadUtility* pXML);

private:
	CvString m_szPath;
	float m_fUpdateRate;
};



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvCameraInfo
//
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvCameraInfo :
	public CvInfoBase
{
public:

	DllExport CvCameraInfo() {}
	DllExport virtual ~CvCameraInfo() {}

	DllExport const TCHAR* getPath() const { return m_szPath; }
	DllExport void setPath(const TCHAR* szVal) { m_szPath = szVal; }

	DllExport bool read(CvXMLLoadUtility* pXML);

private:
	CvString m_szPath;
};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvQuestInfo
//
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvQuestInfo :
	public CvInfoBase
{
public:
	struct QuestLink
	{
		// Stores the QuestLinks Type and Name
		DllExport QuestLink() :
		m_szQuestLinkType("No Type"),
		m_szQuestLinkName("No Name")
		{
		}

	CvString m_szQuestLinkType;
	CvString m_szQuestLinkName;
	};

	DllExport CvQuestInfo();
	DllExport virtual ~CvQuestInfo();

	DllExport void reset();
	DllExport bool initQuestLinks(int iNum);

	DllExport int getNumQuestMessages() const;
	DllExport int getNumQuestLinks() const;
	DllExport int getNumQuestSounds() const;
	DllExport const TCHAR* getQuestObjective() const;
	DllExport const TCHAR* getQuestBodyText() const;
	DllExport const TCHAR* getQuestMessages(int iIndex) const;
	DllExport const TCHAR* getQuestLinkType(int iIndex) const;
	DllExport const TCHAR* getQuestLinkName(int iIndex) const;
	DllExport const TCHAR* getQuestSounds(int iIndex) const;
	DllExport const TCHAR* getQuestScript() const;

	DllExport void setNumQuestMessages(int iNum);
	DllExport void setNumQuestSounds(int iNum);
	DllExport void setQuestObjective(const TCHAR* szText);
	DllExport void setQuestBodyText(const TCHAR* szText);
	DllExport void setQuestMessages(int iIndex, const TCHAR* szText);
	DllExport void setQuestSounds(int iIndex, const TCHAR* szText);
	DllExport void setQuestScript(const TCHAR* szText);

	DllExport bool read(CvXMLLoadUtility* pXML);

private:
	int m_iNumQuestMessages;
	int m_iNumQuestLinks;
	int m_iNumQuestSounds;

	CvString m_szQuestObjective;
	CvString m_szQuestBodyText;
	CvString m_szQuestScript;

	CvString* m_paszQuestMessages;
	QuestLink* m_pQuestLinks;
	CvString* m_paszQuestSounds;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvTutorialInfo
//
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvTutorialMessage
{
public:
	DllExport CvTutorialMessage();
	DllExport virtual ~CvTutorialMessage();

	DllExport const TCHAR* getText() const;
	DllExport const TCHAR* getImage() const;
	DllExport const TCHAR* getSound() const;

	DllExport void setText(const TCHAR* szText);
	DllExport void setImage(const TCHAR* szText);
	DllExport void setSound(const TCHAR* szText);
	DllExport void setScript(int iIndex, const TCHAR* szText);

	DllExport int getNumTutorialScripts() const;
	DllExport const TCHAR* getTutorialScriptByIndex(int i) const;
	DllExport bool read(CvXMLLoadUtility* pXML);

private:
	int m_iNumTutorialScripts;
	CvString m_szTutorialMessageText;
	CvString m_szTutorialMessageImage;
	CvString m_szTutorialMessageSound;
	CvString* m_paszTutorialScripts;
};

class CvTutorialMessage;
class CvTutorialInfo :
	public CvInfoBase
{
public:
	DllExport CvTutorialInfo();
	DllExport virtual ~CvTutorialInfo();

	DllExport const TCHAR* getNextTutorialInfoType();
	DllExport void setNextTutorialInfoType(const TCHAR* szVal);

	DllExport bool initTutorialMessages(int iNum);
	DllExport void resetMessages();

	DllExport int getNumTutorialMessages() const;
	DllExport const CvTutorialMessage* getTutorialMessage(int iIndex) const;

	DllExport bool read(CvXMLLoadUtility* pXML);

private:
	CvString m_szNextTutorialInfoType;
	int m_iNumTutorialMessages;
	CvTutorialMessage* m_paTutorialMessages;
};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvGameOptionInfo
//
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvGameOptionInfo :
	public CvInfoBase
{
public:
	DllExport CvGameOptionInfo();
	DllExport virtual ~CvGameOptionInfo();

	DllExport bool getDefault() const;
	DllExport bool getVisible() const;

	DllExport bool read(CvXMLLoadUtility* pXML);

private:
	bool m_bDefault;
	bool m_bVisible;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvMPOptionInfo
//
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvMPOptionInfo :
	public CvInfoBase
{
public:
	DllExport CvMPOptionInfo();
	DllExport virtual ~CvMPOptionInfo();

	DllExport bool getDefault() const;

	DllExport bool read(CvXMLLoadUtility* pXML);

private:
	bool m_bDefault;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvForceControlInfo
//
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvForceControlInfo :
	public CvInfoBase
{
public:
	DllExport CvForceControlInfo();
	DllExport virtual ~CvForceControlInfo();

	DllExport bool getDefault() const;

	DllExport bool read(CvXMLLoadUtility* pXML);

private:
	bool m_bDefault;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvPlayerOptionInfo
//
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvPlayerOptionInfo :
	public CvInfoBase
{
public:
	DllExport CvPlayerOptionInfo();
	DllExport virtual ~CvPlayerOptionInfo();

	DllExport bool getDefault() const;

	DllExport bool read(CvXMLLoadUtility* pXML);

private:
	bool m_bDefault;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvGraphicOptionInfo
//
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvGraphicOptionInfo :
	public CvInfoBase
{
public:
	DllExport CvGraphicOptionInfo();
	DllExport virtual ~CvGraphicOptionInfo();

	DllExport bool getDefault() const;

	DllExport bool read(CvXMLLoadUtility* pXML);

private:
	bool m_bDefault;

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvEventTriggerInfo
//
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvEventTriggerInfo : public CvInfoBase
{
	friend class CvXMLLoadUtility;

public:
	DllExport CvEventTriggerInfo();
	DllExport virtual ~CvEventTriggerInfo();

	DllExport int getPercentGamesActive() const;
	DllExport int getProbability() const;
	DllExport int getNumUnits() const;
	DllExport int getNumBuildings() const;
	DllExport int getNumUnitsGlobal() const;
	DllExport int getNumBuildingsGlobal() const;
	DllExport int getNumPlotsRequired() const;
	DllExport int getPlotType() const;
	DllExport int getNumReligions() const;
	DllExport int getNumCorporations() const;
	DllExport int getOtherPlayerShareBorders() const;
	DllExport int getOtherPlayerHasTech() const;
	DllExport int getCityFoodWeight() const;
	DllExport int getCivic() const;
	DllExport int getMinPopulation() const;
	DllExport int getMaxPopulation() const;
	DllExport int getMinMapLandmass() const;
	DllExport int getMinOurLandmass() const;
	DllExport int getMaxOurLandmass() const;
	DllExport int getMinDifficulty() const;
	DllExport int getAngry() const;
	DllExport int getUnhealthy() const;
	DllExport int getUnitDamagedWeight() const;
	DllExport int getUnitDistanceWeight() const;
	DllExport int getUnitExperienceWeight() const;
	DllExport int getMinTreasury() const;

	DllExport int getBuildingRequired(int i) const;
	DllExport int getNumBuildingsRequired() const;
	DllExport int getUnitRequired(int i) const;
	DllExport int getNumUnitsRequired() const;
	DllExport int getPrereqOrTechs(int i) const;
	DllExport int getNumPrereqOrTechs() const;
	DllExport int getPrereqAndTechs(int i) const;
	DllExport int getNumPrereqAndTechs() const;
	DllExport int getObsoleteTech(int i) const;
	DllExport int getNumObsoleteTechs() const;
	DllExport int getEvent(int i) const;
	DllExport int getNumEvents() const;
	DllExport int getPrereqEvent(int i) const;
	DllExport int getNumPrereqEvents() const;
	DllExport int getFeatureRequired(int i) const;
	DllExport int getNumFeaturesRequired() const;
	DllExport int getTerrainRequired(int i) const;
	DllExport int getNumTerrainsRequired() const;
	DllExport int getImprovementRequired(int i) const;
	DllExport int getNumImprovementsRequired() const;
	DllExport int getBonusRequired(int i) const;
	DllExport int getNumBonusesRequired() const;
	DllExport int getRouteRequired(int i) const;
	DllExport int getNumRoutesRequired() const;
	DllExport int getReligionRequired(int i) const;
	DllExport int getNumReligionsRequired() const;
	DllExport int getCorporationRequired(int i) const;
	DllExport int getNumCorporationsRequired() const;

	DllExport const CvWString& getText(int i) const;
	DllExport int getTextEra(int i) const;
	DllExport int getNumTexts() const;
	DllExport const CvWString& getWorldNews(int i) const;
	DllExport int getNumWorldNews() const;

	DllExport bool isSinglePlayer() const;
	DllExport bool isTeam() const;
	DllExport bool isRecurring() const;
	DllExport bool isGlobal() const;
	DllExport bool isPickPlayer() const;
	DllExport bool isOtherPlayerWar() const;
	DllExport bool isOtherPlayerHasReligion() const;
	DllExport bool isOtherPlayerHasOtherReligion() const;
	DllExport bool isOtherPlayerAI() const;
	DllExport bool isPickCity() const;
	DllExport bool isPickOtherPlayerCity() const;
	DllExport bool isShowPlot() const;
	DllExport bool isUnitsOnPlot() const;
	DllExport bool isOwnPlot() const;
	DllExport bool isPickReligion() const;
	DllExport bool isStateReligion() const;
	DllExport bool isHolyCity() const;
	DllExport bool isPickCorporation() const;
	DllExport bool isHeadquarters() const;
	DllExport bool isProbabilityUnitMultiply() const;
	DllExport bool isProbabilityBuildingMultiply() const;
	DllExport bool isPrereqEventCity() const;

	DllExport const char* getPythonCallback() const;
	DllExport const char* getPythonCanDo() const;
	DllExport const char* getPythonCanDoCity() const;
	DllExport const char* getPythonCanDoUnit() const;

	DllExport void read(FDataStreamBase* );
	DllExport void write(FDataStreamBase* );

	DllExport bool read(CvXMLLoadUtility* pXML);

private:
	int m_iPercentGamesActive;
	int m_iProbability;
	int m_iNumUnits;
	int m_iNumBuildings;
	int m_iNumUnitsGlobal;
	int m_iNumBuildingsGlobal;
	int m_iNumPlotsRequired;
	int m_iPlotType;
	int m_iNumReligions;
	int m_iNumCorporations;
	int m_iOtherPlayerShareBorders;
	int m_iOtherPlayerHasTech;
	int m_iCityFoodWeight;
	int m_iCivic;
	int m_iMinPopulation;
	int m_iMaxPopulation;
	int m_iMinMapLandmass;
	int m_iMinOurLandmass;
	int m_iMaxOurLandmass;
	int m_iMinDifficulty;
	int m_iAngry;
	int m_iUnhealthy;
	int m_iUnitDamagedWeight;
	int m_iUnitDistanceWeight;
	int m_iUnitExperienceWeight;
	int m_iMinTreasury;

	std::vector<int> m_aiUnitsRequired;
	std::vector<int> m_aiBuildingsRequired;
	std::vector<int> m_aiPrereqOrTechs;
	std::vector<int> m_aiPrereqAndTechs;
	std::vector<int> m_aiObsoleteTechs;
	std::vector<int> m_aiEvents;
	std::vector<int> m_aiPrereqEvents;
	std::vector<int> m_aiFeaturesRequired;
	std::vector<int> m_aiTerrainsRequired;
	std::vector<int> m_aiImprovementsRequired;
	std::vector<int> m_aiBonusesRequired;
	std::vector<int> m_aiRoutesRequired;
	std::vector<int> m_aiReligionsRequired;
	std::vector<int> m_aiCorporationsRequired;

	std::vector<int> m_aiTextEra;
	std::vector<CvWString> m_aszText;
	std::vector<CvWString> m_aszWorldNews;

	bool m_bSinglePlayer;
	bool m_bTeam;
	bool m_bRecurring;
	bool m_bGlobal;
	bool m_bPickPlayer;
	bool m_bOtherPlayerWar;
	bool m_bOtherPlayerHasReligion;
	bool m_bOtherPlayerHasOtherReligion;
	bool m_bOtherPlayerAI;
	bool m_bPickCity;
	bool m_bPickOtherPlayerCity;
	bool m_bShowPlot;
	bool m_bUnitsOnPlot;
	bool m_bOwnPlot;
	bool m_bPickReligion;
	bool m_bStateReligion;
	bool m_bHolyCity;
	bool m_bPickCorporation;
	bool m_bHeadquarters;
	bool m_bProbabilityUnitMultiply;
	bool m_bProbabilityBuildingMultiply;
	bool m_bPrereqEventCity;

	CvString m_szPythonCallback;
	CvString m_szPythonCanDo;
	CvString m_szPythonCanDoCity;
	CvString m_szPythonCanDoUnit;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvEventInfo
//
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvEventInfo : public CvInfoBase
{
	friend class CvXMLLoadUtility;

public:
	DllExport CvEventInfo();
	DllExport virtual ~CvEventInfo();

	DllExport bool isQuest() const;
	DllExport bool isGlobal() const;
	DllExport bool isTeam() const;
	DllExport bool isCityEffect() const;
	DllExport bool isOtherPlayerCityEffect() const;
	DllExport bool isGoldToPlayer() const;
	DllExport bool isGoldenAge() const;
	DllExport bool isDeclareWar() const;
	DllExport bool isDisbandUnit() const;

	DllExport int getGold() const;
	DllExport int getRandomGold() const;
	DllExport int getEspionagePoints() const;
	DllExport int getCulture() const;
	DllExport int getTech() const;
	DllExport int getTechPercent() const;
	DllExport int getTechCostPercent() const;
	DllExport int getTechMinTurnsLeft() const;
	DllExport int getPrereqTech() const;
	DllExport int getUnitClass() const;
	DllExport int getNumUnits() const;
	DllExport int getBuildingClass() const;
	DllExport int getBuildingChange() const;
	DllExport int getHappy() const;
	DllExport int getHealth() const;
	DllExport int getHurryAnger() const;
	DllExport int getHappyTurns() const;
	DllExport int getFood() const;
	DllExport int getFoodPercent() const;
	DllExport int getFeature() const;
	DllExport int getFeatureChange() const;
	DllExport int getImprovement() const;
	DllExport int getImprovementChange() const;
	DllExport int getBonus() const;
	DllExport int getBonusChange() const;
	DllExport int getRoute() const;
	DllExport int getRouteChange() const;
	DllExport int getBonusRevealed() const;
	DllExport int getBonusGift() const;
	DllExport int getUnitExperience() const;
	DllExport int getUnitImmobileTurns() const;
	DllExport int getConvertOwnCities() const;
	DllExport int getConvertOtherCities() const;
	DllExport int getMaxNumReligions() const;
	DllExport int getOurAttitudeModifier() const;
	DllExport int getAttitudeModifier() const;
	DllExport int getTheirEnemyAttitudeModifier() const;
	DllExport int getPopulationChange() const;
	DllExport int getRevoltTurns() const;
	DllExport int getMinPillage() const;
	DllExport int getMaxPillage() const;
	DllExport int getUnitPromotion() const;
	DllExport int getFreeUnitSupport() const;
	DllExport int getInflationModifier() const;
	DllExport int getSpaceProductionModifier() const;
	DllExport int getAIValue() const;

	DllExport int getAdditionalEventChance(int i) const;
	DllExport int getAdditionalEventTime(int i) const;
	DllExport int getClearEventChance(int i) const;
	DllExport int getTechFlavorValue(int i) const;
	DllExport int getPlotExtraYield(int i) const;
	DllExport int getFreeSpecialistCount(int i) const;
	DllExport int getUnitCombatPromotion(int i) const;
	DllExport int getUnitClassPromotion(int i) const;
	DllExport const CvWString& getWorldNews(int i) const;
	DllExport int getNumWorldNews() const;

	DllExport int getBuildingYieldChange(int iBuildingClass, int iYield) const;
	DllExport int getNumBuildingYieldChanges() const;
	DllExport int getBuildingCommerceChange(int iBuildingClass, int iCommerce) const;
	DllExport int getNumBuildingCommerceChanges() const;
	DllExport int getBuildingHappyChange(int iBuildingClass) const;
	DllExport int getNumBuildingHappyChanges() const;
	DllExport int getBuildingHealthChange(int iBuildingClass) const;
	DllExport int getNumBuildingHealthChanges() const;

	DllExport const char* getPythonCallback() const;
	DllExport const char* getPythonExpireCheck() const;
	DllExport const char* getPythonCanDo() const;
	DllExport const char* getPythonHelp() const;
	DllExport const wchar* getUnitNameKey() const;
	DllExport const wchar* getQuestFailTextKey() const;
	DllExport const wchar* getOtherPlayerPopup() const;
	DllExport const wchar* getLocalInfoTextKey() const;

	DllExport void read(FDataStreamBase* );
	DllExport void write(FDataStreamBase* );

	DllExport bool read(CvXMLLoadUtility* pXML);
	DllExport bool readPass2(CvXMLLoadUtility* pXML);

private:
	bool m_bQuest;
	bool m_bGlobal;
	bool m_bTeam;
	bool m_bCityEffect;
	bool m_bOtherPlayerCityEffect;
	bool m_bGoldToPlayer;
	bool m_bGoldenAge;
	bool m_bDeclareWar;
	bool m_bDisbandUnit;

	int m_iGold;
	int m_iRandomGold;
	int m_iCulture;
	int m_iEspionagePoints;
	int m_iTech;
	int m_iTechPercent;
	int m_iTechCostPercent;
	int m_iTechMinTurnsLeft;
	int m_iPrereqTech;
	int m_iUnitClass;
	int m_iNumUnits;
	int m_iBuildingClass;
	int m_iBuildingChange;
	int m_iHappy;
	int m_iHealth;
	int m_iHurryAnger;
	int m_iHappyTurns;
	int m_iFood;
	int m_iFoodPercent;
	int m_iFeature;
	int m_iFeatureChange;
	int m_iImprovement;
	int m_iImprovementChange;
	int m_iBonus;
	int m_iBonusChange;
	int m_iRoute;
	int m_iRouteChange;
	int m_iBonusRevealed;
	int m_iBonusGift;
	int m_iUnitExperience;
	int m_iUnitImmobileTurns;
	int m_iConvertOwnCities;
	int m_iConvertOtherCities;
	int m_iMaxNumReligions;
	int m_iOurAttitudeModifier;
	int m_iAttitudeModifier;
	int m_iTheirEnemyAttitudeModifier;
	int m_iPopulationChange;
	int m_iRevoltTurns;
	int m_iMinPillage;
	int m_iMaxPillage;
	int m_iUnitPromotion;
	int m_iFreeUnitSupport;
	int m_iInflationModifier;
	int m_iSpaceProductionModifier;
	int m_iAIValue;

	int* m_piTechFlavorValue;
	int* m_piPlotExtraYields;
	int* m_piFreeSpecialistCount;
	int* m_piAdditionalEventChance;
	int* m_piAdditionalEventTime;
	int* m_piClearEventChance;
	int* m_piUnitCombatPromotions;
	int* m_piUnitClassPromotions;

	std::vector<BuildingYieldChange> m_aBuildingYieldChanges;
	std::vector<BuildingCommerceChange> m_aBuildingCommerceChanges;
	BuildingChangeArray m_aBuildingHappyChanges;
	BuildingChangeArray m_aBuildingHealthChanges;

	CvString m_szPythonCallback;
	CvString m_szPythonExpireCheck;
	CvString m_szPythonCanDo;
	CvString m_szPythonHelp;
	CvWString m_szUnitName;
	CvWString m_szOtherPlayerPopup;
	CvWString m_szQuestFailText;
	CvWString m_szLocalInfoText;
	std::vector<CvWString> m_aszWorldNews;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvEspionageMissionInfo
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvEspionageMissionInfo : public CvInfoBase
{
	//---------------------------------------PUBLIC INTERFACE---------------------------------
public:

	DllExport CvEspionageMissionInfo();
	DllExport virtual ~CvEspionageMissionInfo();

	DllExport int getCost() const;
	DllExport bool isPassive() const;
	DllExport bool isTwoPhases() const;
	DllExport bool isTargetsCity() const;
	DllExport bool isSelectPlot() const;

	DllExport int getTechPrereq() const;
	DllExport int getVisibilityLevel() const;
	DllExport bool isInvestigateCity() const;
	DllExport bool isSeeDemographics() const;
	DllExport bool isNoActiveMissions() const;
	DllExport bool isSeeResearch() const;

	DllExport bool isDestroyImprovement() const;
	DllExport int getDestroyBuildingCostFactor() const;
	DllExport int getDestroyUnitCostFactor() const;
	DllExport int getDestroyProjectCostFactor() const;
	DllExport int getDestroyProductionCostFactor() const;
	DllExport int getBuyUnitCostFactor() const;
	DllExport int getBuyCityCostFactor() const;
	DllExport int getStealTreasuryTypes() const;
	DllExport int getCityInsertCultureAmountFactor() const;
	DllExport int getCityInsertCultureCostFactor() const;
	DllExport int getCityPoisonWaterCounter() const;
	DllExport int getCityUnhappinessCounter() const;
	DllExport int getCityRevoltCounter() const;
	DllExport int getBuyTechCostFactor() const;
	DllExport int getSwitchCivicCostFactor() const;
	DllExport int getSwitchReligionCostFactor() const;
	DllExport int getPlayerAnarchyCounter() const;
	DllExport int getCounterespionageNumTurns() const;
	DllExport int getCounterespionageMod() const;
	DllExport int getDifficultyMod() const;

	DllExport bool read(CvXMLLoadUtility* pXML);

	//---------------------------------------PROTECTED MEMBER VARIABLES---------------------------------
protected:

	int m_iCost;
	bool m_bIsPassive;
	bool m_bIsTwoPhases;
	bool m_bTargetsCity;
	bool m_bSelectPlot;

	int m_iTechPrereq;
	int m_iVisibilityLevel;
	bool m_bInvestigateCity;
	bool m_bSeeDemographics;
	bool m_bNoActiveMissions;
	bool m_bSeeResearch;

	bool m_bDestroyImprovement;
	int m_iDestroyBuildingCostFactor;
	int m_iDestroyUnitCostFactor;
	int m_iDestroyProjectCostFactor;
	int m_iDestroyProductionCostFactor;
	int m_iBuyUnitCostFactor;
	int m_iBuyCityCostFactor;
	int m_iStealTreasuryTypes;
	int m_iCityInsertCultureAmountFactor;
	int m_iCityInsertCultureCostFactor;
	int m_iCityPoisonWaterCounter;
	int m_iCityUnhappinessCounter;
	int m_iCityRevoltCounter;
	int m_iBuyTechCostFactor;
	int m_iSwitchCivicCostFactor;
	int m_iSwitchReligionCostFactor;
	int m_iPlayerAnarchyCounter;
	int m_iCounterespionageNumTurns;
	int m_iCounterespionageMod;
	int m_iDifficultyMod;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvUnitArtStyleTypeInfo
//
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvUnitArtStyleTypeInfo : public CvInfoBase
{
public:

	DllExport CvUnitArtStyleTypeInfo();
	DllExport virtual ~CvUnitArtStyleTypeInfo();

    DllExport const TCHAR* getEarlyArtDefineTag(int /*Mesh Index*/ i, int /*UnitType*/ j) const;
	DllExport void setEarlyArtDefineTag(int /*Mesh Index*/ i, int /*UnitType*/ j, const TCHAR* szVal);
	DllExport const TCHAR* getLateArtDefineTag(int /*Mesh Index*/ i, int /*UnitType*/ j) const;
	DllExport void setLateArtDefineTag(int /*Mesh Index*/ i, int /*UnitType*/ j, const TCHAR* szVal);
	DllExport const TCHAR* getMiddleArtDefineTag(int /*Mesh Index*/ i, int /*UnitType*/ j) const;
	DllExport void setMiddleArtDefineTag(int /*Mesh Index*/ i, int /*UnitType*/ j, const TCHAR* szVal);

	DllExport bool read(CvXMLLoadUtility* pXML);

protected:

	struct ArtDefneTag
	{
		int iMeshIndex;
		int iUnitType;
		CvString szTag;
	};
	typedef std::vector<ArtDefneTag> ArtDefineArray;
    ArtDefineArray m_azEarlyArtDefineTags;
	ArtDefineArray m_azLateArtDefineTags;
	ArtDefineArray m_azMiddleArtDefineTags;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvVoteSourceInfo
//
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvVoteSourceInfo : public CvInfoBase
{
public:

	DllExport CvVoteSourceInfo();
	DllExport virtual ~CvVoteSourceInfo();

	DllExport int getVoteInterval() const;
	DllExport int getFreeSpecialist() const;
	DllExport int getCivic() const;
	DllExport const CvWString& getPopupText() const;
	DllExport const CvWString& getSecretaryGeneralText() const;

	std::wstring pyGetSecretaryGeneralText() { return getSecretaryGeneralText(); }

	DllExport int getReligionYield(int i) const;
	DllExport int getReligionCommerce(int i) const;

	DllExport bool read(CvXMLLoadUtility* pXML);
	DllExport bool readPass3();

protected:
	int m_iVoteInterval;
	int m_iFreeSpecialist;
	int m_iCivic;

	int* m_aiReligionYields;
	int* m_aiReligionCommerces;

	CvString m_szPopupText;
	CvString m_szSecretaryGeneralText;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvMainMenuInfo
//
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvMainMenuInfo : public CvInfoBase
{
public:

	DllExport CvMainMenuInfo();
	DllExport virtual ~CvMainMenuInfo();

	DllExport std::string getScene() const;
	DllExport std::string getSceneNoShader() const;
	DllExport std::string getSoundtrack() const;
	DllExport std::string getLoading() const;
	DllExport std::string getLoadingSlideshow() const;

	DllExport bool read(CvXMLLoadUtility* pXML);

protected:
	std::string m_szScene;
	std::string m_szSceneNoShader;
	std::string m_szSoundtrack;
	std::string m_szLoading;
	std::string m_szLoadingSlideshow;
};
#endif
