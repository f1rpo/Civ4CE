//  $Header:
//------------------------------------------------------------------------------------------------
//
//  FILE:    CvInfos.cpp
//
//  AUTHOR:	Eric MacDonald  --  8/2003
//					Mustafa Thamer 11/2004
//					Jon Shafer - 03/2005		
//
//  PURPOSE: The base class for all info classes to inherit from.  This gives us the base description
//				and type strings
//
//------------------------------------------------------------------------------------------------
//  Copyright (c) 2003 Firaxis Games, Inc. All rights reserved.
//------------------------------------------------------------------------------------------------
#include "CvGameCoreDLL.h"
#include "CvInfos.h"
//#include "CvEnums.h"
#include "CvGlobals.h"
#include "CvArtFileMgr.h"
#include "CvXMLLoadUtility.h"
#include "CvDLLXMLIFaceBase.h"
#include "CvGameTextMgr.h"
#include "CvGameCoreUtils.h"

// replacement for strdup
char* copyString(const char* szString)
{
	if (!szString)
		return NULL;
	char* pBuf = new char[strlen(szString)+1];
	strcpy(pBuf, szString);
	return pBuf;
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CInfoBase()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvInfoBase::CvInfoBase() :
m_bGraphicalOnly(false),
m_szType(NULL),
m_szButton(NULL),	
m_szTextKey(NULL),
m_szCivilopediaKey(NULL),
m_szHelpKey(NULL),
m_szStrategyKey(NULL),
m_szXmlVal(NULL)
{
	m_szType=copyString("ERROR");
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CInfoBase()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvInfoBase::~CvInfoBase()
{
	SAFE_DELETE_ARRAY(m_szType);
	SAFE_DELETE_ARRAY(m_szButton);		
	SAFE_DELETE_ARRAY(m_szTextKey);
	SAFE_DELETE_ARRAY(m_szCivilopediaKey);
	SAFE_DELETE_ARRAY(m_szHelpKey);
	SAFE_DELETE_ARRAY(m_szStrategyKey);
	SAFE_DELETE_ARRAY(m_szXmlVal);
}

CvInfoBase::CvInfoBase(const CvInfoBase& kCopy)
{
	copy(kCopy);
}

CvInfoBase& CvInfoBase::operator=(const CvInfoBase& kCopy)
{
	copy(kCopy);
	return *this;
}

void CvInfoBase::copy(const CvInfoBase& kCopy)
{
	setType(kCopy.m_szType);
	setButton(kCopy.m_szButton);
	setTextKey(kCopy.m_szTextKey);
	setCivilopediaKey(kCopy.m_szCivilopediaKey);
	setHelpKey(kCopy.m_szHelpKey);
	setStrategyKey(kCopy.m_szStrategyKey);
	setXmlVal(kCopy.m_szXmlVal);
	m_bGraphicalOnly = kCopy.m_bGraphicalOnly;
}

void CvInfoBase::read(FDataStreamBase* pStream)
{
	SAFE_DELETE_ARRAY(m_szType);
	SAFE_DELETE_ARRAY(m_szButton);		
	SAFE_DELETE_ARRAY(m_szTextKey);
	SAFE_DELETE_ARRAY(m_szCivilopediaKey);
	SAFE_DELETE_ARRAY(m_szHelpKey);
	SAFE_DELETE_ARRAY(m_szStrategyKey);
	SAFE_DELETE_ARRAY(m_szXmlVal);

	pStream->Read(&m_bGraphicalOnly);
	m_szType = pStream->ReadString();
	m_szCivilopediaKey = pStream->ReadString();
	m_szHelpKey = pStream->ReadString();
	m_szStrategyKey = pStream->ReadString();
	m_szButton = pStream->ReadString();
	m_szTextKey = pStream->ReadString();
	m_szXmlVal = pStream->ReadString();
}

void CvInfoBase::write(FDataStreamBase* pStream)
{
	pStream->Write(m_bGraphicalOnly);
	pStream->WriteString(m_szType);
	pStream->WriteString(m_szCivilopediaKey);
	pStream->WriteString(m_szHelpKey);
	pStream->WriteString(m_szStrategyKey);
	pStream->WriteString(m_szButton);
	pStream->WriteString(m_szTextKey);
	pStream->WriteString(m_szXmlVal);
}

bool CvInfoBase::isGraphicalOnly() const
{
	return m_bGraphicalOnly;
}

const TCHAR* CvInfoBase::getType() const
{
	return m_szType;
}

const char* CvInfoBase::getXmlVal() const
{
	return m_szXmlVal;
}

const TCHAR* CvInfoBase::getButton() const
{
	return m_szButton;
}

const TCHAR* CvInfoBase::getTextKey() const
{
	return m_szTextKey;
}

const wchar* CvInfoBase::getTextKeyWide() const
{
	setTempText(CvWString(m_szTextKey));
	return m_szTempText;
}

const wchar* CvInfoBase::getDescription(uint uiForm) const
{
	m_szTempText = gDLL->getObjectText(m_szTextKey, uiForm);
	return m_szTempText;
}

const wchar* CvInfoBase::getText() const
{
	// used instead of getDescription for Info entries that are not objects
	// so they do not have gender/plurality/forms defined in the Translator system
	setTempText(gDLL->getText(m_szTextKey));
	return m_szTempText;
}

const wchar* CvInfoBase::getCivilopedia() const
{
	setTempText(gDLL->getText(m_szCivilopediaKey));
	return m_szTempText;
}

const wchar*  CvInfoBase::getHelp() const
{
	setTempText(gDLL->getText(m_szHelpKey));
	return m_szTempText;
}

const wchar* CvInfoBase::getStrategy() const
{
	setTempText(gDLL->getText(m_szStrategyKey));
	return m_szTempText;
}

void CvInfoBase::setType(const TCHAR* szVal)
{
	SAFE_DELETE_ARRAY(m_szType);
	m_szType = copyString(szVal);
}

void CvInfoBase::setXmlVal(const char* szVal)
{
	SAFE_DELETE_ARRAY(m_szXmlVal);
	m_szXmlVal = copyString(szVal);
}

void CvInfoBase::setButton(const TCHAR* szVal)
{
	SAFE_DELETE_ARRAY(m_szButton);
	m_szButton = copyString(szVal);
}

void CvInfoBase::setTextKey(const TCHAR* szVal)
{
	SAFE_DELETE_ARRAY(m_szTextKey);
	m_szTextKey = copyString(szVal);
}

void CvInfoBase::setCivilopediaKey(const TCHAR* szVal)
{
	SAFE_DELETE_ARRAY(m_szCivilopediaKey);
	m_szCivilopediaKey = copyString(szVal);
}

void CvInfoBase::setHelpKey(const TCHAR* szVal)
{
	SAFE_DELETE_ARRAY(m_szHelpKey);
	m_szHelpKey = copyString(szVal);
}

void CvInfoBase::setStrategyKey(const TCHAR* szVal)
{
	SAFE_DELETE_ARRAY(m_szStrategyKey);
	m_szStrategyKey = copyString(szVal);
}

bool CvInfoBase::isMatchForLink(std::wstring szLink, bool bKeysOnly) const
{
	if (szLink == CvWString(getType()).GetCString())
	{
		return true;
	}

	if (!bKeysOnly)
	{
		uint iNumForms = gDLL->getNumForms(getTextKeyWide());
		for (uint i = 0; i < iNumForms; i++)
		{
			if (szLink == getDescription(i))
			{
				return true;
			}
		}
	}

	return false;
}

//
// read from XML
// TYPE, DESC, BUTTON
//
bool CvInfoBase::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;

	// Skip any comments and stop at the next value we might want
	if (!pXML->SkipToNextVal())
	{
		return false;
	}

	if (pXML->GetXmlVal(szTextVal))
	{
		// GetXmlVal returns the XML tags for all children as well, separated by spaces
		// Keep only the first value
		CvString::size_type i = szTextVal.find(' ', 0);
		if (CvString::npos != i)
		{
			szTextVal.resize(i);
		}
		setXmlVal(szTextVal);
	}

	pXML->MapChildren();	// try to hash children for fast lookup by name

	// GRAPHICAL ONLY
	pXML->GetChildXmlValByName(&m_bGraphicalOnly, "bGraphicalOnly");

	// TYPE
	if (pXML->GetChildXmlValByName(szTextVal, "Type"))
	{
		setType(szTextVal);
	}

	// DESCRIPTION
	if (pXML->GetChildXmlValByName(szTextVal, "Description"))
	{
		setTextKey(szTextVal);
	}

	// CIVILOPEDIA
	if (pXML->GetChildXmlValByName(szTextVal, "Civilopedia"))
	{
		setCivilopediaKey(szTextVal);
	}

	// HELP
	if (pXML->GetChildXmlValByName(szTextVal, "Help"))
	{
		setHelpKey(szTextVal);
	}

	// STRATEGY
	if (pXML->GetChildXmlValByName(szTextVal, "Strategy"))
	{
		setStrategyKey(szTextVal);
	}

	// BUTTON
	if (pXML->GetChildXmlValByName(szTextVal, "Button"))
	{
		setButton(szTextVal);
	}

	return true;
}

//======================================================================================================
//					CvScalableInfo
//======================================================================================================
bool CvScalableInfo::read(CvXMLLoadUtility* pXML)
{
	float fScale;
	pXML->GetChildXmlValByName(&fScale, "fScale");
	setScale(fScale);
	pXML->GetChildXmlValByName(&fScale, "fInterfaceScale", 1.0f);
	setInterfaceScale(fScale);
	return true;
}

float CvScalableInfo::getScale() const
{
	return m_fScale;
}

void CvScalableInfo::setScale(float fScale)
{
	m_fScale = fScale;
}

float CvScalableInfo::getInterfaceScale() const
{
	return m_fInterfaceScale;
}

void CvScalableInfo::setInterfaceScale(float fInterfaceScale)
{
	m_fInterfaceScale = fInterfaceScale;
}


//======================================================================================================
//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvHotkeyInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvHotkeyInfo::CvHotkeyInfo() :
m_iActionInfoIndex(-1),
m_iHotKeyVal(-1),
m_iHotKeyPriority(-1),
m_iHotKeyValAlt(-1),
m_iHotKeyPriorityAlt(-1),
m_iOrderPriority(0),
m_bAltDown(false),
m_bShiftDown(false),
m_bCtrlDown(false),
m_bAltDownAlt(false),
m_bShiftDownAlt(false),
m_bCtrlDownAlt(false)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvHotkeyInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvHotkeyInfo::~CvHotkeyInfo()
{
}

bool CvHotkeyInfo::read(CvXMLLoadUtility* pXML)
{
	int iVal;
	bool bVal;
	CvString szTextVal;

	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	if (pXML->GetChildXmlValByName(szTextVal, "HotKey"))
	{
		setHotKey(szTextVal);
	}
	else
	{
		setHotKey("");
	}
	iVal = pXML->GetHotKeyInt(szTextVal);
	setHotKeyVal(iVal);
  if (pXML->GetChildXmlValByName(&iVal, "iHotKeyPriority"))
	{
		setHotKeyPriority(iVal);
	}
	else
	{
		setHotKeyPriority(-1);
	}

	if (pXML->GetChildXmlValByName(szTextVal, "HotKeyAlt"))
	{
		iVal = pXML->GetHotKeyInt(szTextVal);
	}
	else
	{
		iVal = pXML->GetHotKeyInt("");
	}
	setHotKeyValAlt(iVal);
	if (pXML->GetChildXmlValByName(&iVal, "iHotKeyPriorityAlt"))
	{
		setHotKeyPriorityAlt(iVal);
	}
	else
	{
		setHotKeyPriorityAlt(-1);
	}

	if (pXML->GetChildXmlValByName(&bVal, "bAltDown"))
	{
		setAltDown(bVal);
	}
	else
	{
		setAltDown(false);
	}
	if (pXML->GetChildXmlValByName(&bVal, "bShiftDown"))
	{
		setShiftDown(bVal);
	}
	else
	{
		setShiftDown(false);
	}
	if (pXML->GetChildXmlValByName(&bVal, "bCtrlDown"))
	{
		setCtrlDown(bVal);
	}
	else
	{
		setCtrlDown(false);
	}

	if (pXML->GetChildXmlValByName(&bVal, "bAltDownAlt"))
	{
		setAltDownAlt(bVal);
	}
	else
	{
		setAltDownAlt(false);
	}
	if (pXML->GetChildXmlValByName(&bVal, "bShiftDownAlt"))
	{
		setShiftDownAlt(bVal);
	}
	else
	{
		setShiftDownAlt(false);
	}
	if (pXML->GetChildXmlValByName(&bVal, "bCtrlDownAlt"))
	{
		setCtrlDownAlt(bVal);
	}
	else
	{
		setCtrlDownAlt(false);
	}
	if (pXML->GetChildXmlValByName(&iVal, "iOrderPriority"))
	{
		setOrderPriority(iVal);
	}
	else
	{
		setOrderPriority(5);
	}

	setHotKeyDescription(getTextKeyWide(), NULL, pXML->CreateHotKeyFromDescription(getHotKey(), m_bShiftDown, m_bAltDown, m_bCtrlDown));

	return true;
}

void CvHotkeyInfo::read(FDataStreamBase* pStream)
{
	CvInfoBase::read(pStream);

	uint uiFlag=0;
	pStream->Read(&uiFlag);	// flags for expansion

	pStream->Read(&m_iHotKeyVal);
	pStream->Read(&m_iHotKeyPriority);
	pStream->Read(&m_iHotKeyValAlt);
	pStream->Read(&m_iHotKeyPriorityAlt);
	pStream->Read(&m_iOrderPriority);
	pStream->Read(&m_bAltDown);
	pStream->Read(&m_bShiftDown);
	pStream->Read(&m_bCtrlDown);
	pStream->Read(&m_bAltDownAlt);
	pStream->Read(&m_bShiftDownAlt);
	pStream->Read(&m_bCtrlDownAlt);
	pStream->ReadString(m_szHotKey);
	pStream->ReadString(m_szHotKeyDescriptionKey);
	pStream->ReadString(m_szHotKeyAltDescriptionKey);
	pStream->ReadString(m_szHotKeyString);
}

void CvHotkeyInfo::write(FDataStreamBase* pStream)
{
	CvInfoBase::write(pStream);

	uint uiFlag = 0;
	pStream->Write(uiFlag);		// flag for expansion

	pStream->Write(m_iHotKeyVal);
	pStream->Write(m_iHotKeyPriority);
	pStream->Write(m_iHotKeyValAlt);
	pStream->Write(m_iHotKeyPriorityAlt);
	pStream->Write(m_iOrderPriority);
	pStream->Write(m_bAltDown);
	pStream->Write(m_bShiftDown);
	pStream->Write(m_bCtrlDown);
	pStream->Write(m_bAltDownAlt);
	pStream->Write(m_bShiftDownAlt);
	pStream->Write(m_bCtrlDownAlt);
	pStream->WriteString(m_szHotKey);
	pStream->WriteString(m_szHotKeyDescriptionKey);
	pStream->WriteString(m_szHotKeyAltDescriptionKey);
	pStream->WriteString(m_szHotKeyString);
}

int CvHotkeyInfo::getActionInfoIndex() const
{
	return m_iActionInfoIndex;
}

void CvHotkeyInfo::setActionInfoIndex(int i)
{
	m_iActionInfoIndex = i;
}

int CvHotkeyInfo::getHotKeyVal() const
{
	return m_iHotKeyVal;
}

void CvHotkeyInfo::setHotKeyVal(int i)
{
	m_iHotKeyVal = i;
}

int CvHotkeyInfo::getHotKeyPriority() const
{
	return m_iHotKeyPriority;
}

void CvHotkeyInfo::setHotKeyPriority(int i)
{
	m_iHotKeyPriority = i;
}

int CvHotkeyInfo::getHotKeyValAlt() const
{
	return m_iHotKeyValAlt;
}

void CvHotkeyInfo::setHotKeyValAlt(int i)
{
	m_iHotKeyValAlt = i;
}

int CvHotkeyInfo::getHotKeyPriorityAlt() const
{
	return m_iHotKeyPriorityAlt;
}

void CvHotkeyInfo::setHotKeyPriorityAlt(int i)
{
	m_iHotKeyPriorityAlt = i;
}

int CvHotkeyInfo::getOrderPriority() const
{
	return m_iOrderPriority;
}

void CvHotkeyInfo::setOrderPriority(int i)
{
	m_iOrderPriority = i;
}

bool CvHotkeyInfo::isAltDown() const
{
	return m_bAltDown;
}

void CvHotkeyInfo::setAltDown(bool b)
{
	m_bAltDown = b;
}

bool CvHotkeyInfo::isShiftDown() const
{
	return m_bShiftDown;
}

void CvHotkeyInfo::setShiftDown(bool b)
{
	m_bShiftDown = b;
}

bool CvHotkeyInfo::isCtrlDown() const
{
	return m_bCtrlDown;
}

void CvHotkeyInfo::setCtrlDown(bool b)
{
	m_bCtrlDown = b;
}

bool CvHotkeyInfo::isAltDownAlt() const
{
	return m_bAltDownAlt;
}

void CvHotkeyInfo::setAltDownAlt(bool b)
{
	m_bAltDownAlt = b;
}

bool CvHotkeyInfo::isShiftDownAlt() const
{
	return m_bShiftDownAlt;
}

void CvHotkeyInfo::setShiftDownAlt(bool b)
{
	m_bShiftDownAlt = b;
}

bool CvHotkeyInfo::isCtrlDownAlt() const
{
	return m_bCtrlDownAlt;
}

void CvHotkeyInfo::setCtrlDownAlt(bool b)
{
	m_bCtrlDownAlt = b;
}

const TCHAR* CvHotkeyInfo::getHotKey() const
{
	return m_szHotKey;
}

void CvHotkeyInfo::setHotKey(const TCHAR* szVal)
{
	m_szHotKey = szVal;
}

std::wstring CvHotkeyInfo::getHotKeyDescription() const
{
	if (!m_szHotKeyAltDescriptionKey.empty())
	{
		m_szTempText = CvWString::format(L"%s (%s)", gDLL->getObjectText(m_szHotKeyAltDescriptionKey, 0).GetCString(), gDLL->getObjectText(m_szHotKeyDescriptionKey, 0).GetCString());
	}
	else
	{
		m_szTempText = gDLL->getObjectText(m_szHotKeyDescriptionKey, 0);
	}

	if (!m_szHotKeyString.empty())
	{
		m_szTempText += m_szHotKeyString;
	}

	return m_szTempText.GetCString();
}

void CvHotkeyInfo::setHotKeyDescription(const wchar* szHotKeyDescKey, const wchar* szHotKeyAltDescKey, const wchar* szHotKeyString)
{
	m_szHotKeyDescriptionKey = szHotKeyDescKey;
	m_szHotKeyAltDescriptionKey = szHotKeyAltDescKey;
	m_szHotKeyString = szHotKeyString;
}

//======================================================================================================
//					CvDiplomacyResponse
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvDiplomacyResponse()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvDiplomacyResponse::CvDiplomacyResponse() :
m_iNumDiplomacyText(0), 
m_pbCivilizationTypes(NULL), 
m_pbLeaderHeadTypes(NULL), 
m_pbAttitudeTypes(NULL), 
m_pbDiplomacyPowerTypes(NULL), 
m_paszDiplomacyText(NULL)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvDiplomacyResponse()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvDiplomacyResponse::~CvDiplomacyResponse()
{
	SAFE_DELETE_ARRAY(m_pbCivilizationTypes);
	SAFE_DELETE_ARRAY(m_pbLeaderHeadTypes);
	SAFE_DELETE_ARRAY(m_pbAttitudeTypes);
	SAFE_DELETE_ARRAY(m_pbDiplomacyPowerTypes);
	SAFE_DELETE_ARRAY(m_paszDiplomacyText);
}

int CvDiplomacyResponse::getNumDiplomacyText()
{
	return m_iNumDiplomacyText;
}

void CvDiplomacyResponse::setNumDiplomacyText(int i)
{
	m_iNumDiplomacyText = i;
}

bool CvDiplomacyResponse::getCivilizationTypes(int i)
{
	FAssertMsg(i < GC.getNumCivilizationInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbCivilizationTypes[i];
}

bool* CvDiplomacyResponse::getCivilizationTypes() const
{
	return m_pbCivilizationTypes;
}

void CvDiplomacyResponse::setCivilizationTypes(int i, bool bVal)
{
	FAssertMsg(i < GC.getNumCivilizationInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	m_pbCivilizationTypes[i] = bVal;
}

bool CvDiplomacyResponse::getLeaderHeadTypes(int i)
{
	FAssertMsg(i < GC.getNumLeaderHeadInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbLeaderHeadTypes[i];
}

bool* CvDiplomacyResponse::getLeaderHeadTypes() const
{
	return m_pbLeaderHeadTypes;
}

void CvDiplomacyResponse::setLeaderHeadTypes(int i, bool bVal)
{
	FAssertMsg(i < GC.getNumLeaderHeadInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	m_pbLeaderHeadTypes[i] = bVal; 
}

bool CvDiplomacyResponse::getAttitudeTypes(int i) const
{
	FAssertMsg(i < NUM_ATTITUDE_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbAttitudeTypes[i];
}

bool* CvDiplomacyResponse::getAttitudeTypes() const
{
	return m_pbAttitudeTypes;
}

void CvDiplomacyResponse::setAttitudeTypes(int i, bool bVal)
{
	FAssertMsg(i < NUM_ATTITUDE_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	m_pbAttitudeTypes[i] = bVal;
}

bool CvDiplomacyResponse::getDiplomacyPowerTypes(int i)
{
	FAssertMsg(i < NUM_DIPLOMACYPOWER_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbDiplomacyPowerTypes[i];
}

bool* CvDiplomacyResponse::getDiplomacyPowerTypes() const
{
	return m_pbDiplomacyPowerTypes;
}

void CvDiplomacyResponse::setDiplomacyPowerTypes(int i, bool bVal)
{
	FAssertMsg(i < NUM_DIPLOMACYPOWER_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	m_pbDiplomacyPowerTypes[i] = bVal;
}

const TCHAR* CvDiplomacyResponse::getDiplomacyText(int i) const
{
	return m_paszDiplomacyText[i];
}

const CvString* CvDiplomacyResponse::getDiplomacyText() const
{
	return m_paszDiplomacyText;
}

void CvDiplomacyResponse::setDiplomacyText(int i, CvString szText)
{
	FAssertMsg(i < getNumDiplomacyText(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	m_paszDiplomacyText[i] = szText;
}

void CvDiplomacyResponse::read(FDataStreamBase* stream)
{
	uint uiFlag=0;
	stream->Read(&uiFlag);		// flag for expansion
	
	stream->Read(&m_iNumDiplomacyText);

	SAFE_DELETE_ARRAY(m_pbCivilizationTypes);
	m_pbCivilizationTypes = new bool[GC.getNumCivilizationInfos()];
	stream->Read(GC.getNumCivilizationInfos(), m_pbCivilizationTypes);

	SAFE_DELETE_ARRAY(m_pbLeaderHeadTypes);
	m_pbLeaderHeadTypes = new bool[GC.getNumLeaderHeadInfos()];
	stream->Read(GC.getNumLeaderHeadInfos(), m_pbLeaderHeadTypes);

	SAFE_DELETE_ARRAY(m_pbAttitudeTypes);
	m_pbAttitudeTypes = new bool[NUM_ATTITUDE_TYPES];
	stream->Read(NUM_ATTITUDE_TYPES, m_pbAttitudeTypes);

	SAFE_DELETE_ARRAY(m_pbDiplomacyPowerTypes);
	m_pbDiplomacyPowerTypes = new bool[NUM_DIPLOMACYPOWER_TYPES];
	stream->Read(NUM_DIPLOMACYPOWER_TYPES, m_pbDiplomacyPowerTypes);

	SAFE_DELETE_ARRAY(m_paszDiplomacyText);
	m_paszDiplomacyText = new CvString[m_iNumDiplomacyText];
	stream->ReadString(m_iNumDiplomacyText, m_paszDiplomacyText);
}

void CvDiplomacyResponse::write(FDataStreamBase* stream)
{
	uint uiFlag=0;
	stream->Write(uiFlag);		// flag for expansion

	stream->Write(m_iNumDiplomacyText);

	stream->Write(GC.getNumCivilizationInfos(), m_pbCivilizationTypes);
	stream->Write(GC.getNumLeaderHeadInfos(), m_pbLeaderHeadTypes);
	stream->Write(NUM_ATTITUDE_TYPES, m_pbAttitudeTypes);
	stream->Write(NUM_DIPLOMACYPOWER_TYPES, m_pbDiplomacyPowerTypes);
	stream->WriteString(m_iNumDiplomacyText, m_paszDiplomacyText);
}

bool CvDiplomacyResponse::read(CvXMLLoadUtility* pXML)
{
	pXML->SetVariableListTagPair(&m_pbCivilizationTypes, "Civilizations",GC.getCivilizationInfo(), sizeof(GC.getCivilizationInfo((CivilizationTypes)0)), GC.getNumCivilizationInfos());
	// Leaders
	pXML->SetVariableListTagPair(&m_pbLeaderHeadTypes, "Leaders", GC.getLeaderHeadInfo(), sizeof(GC.getLeaderHeadInfo((LeaderHeadTypes)0)), GC.getNumLeaderHeadInfos());
	// AttitudeTypes
	pXML->SetVariableListTagPair(&m_pbAttitudeTypes, "Attitudes", GC.getAttitudeInfo(), sizeof(GC.getAttitudeInfo((AttitudeTypes)0)), NUM_ATTITUDE_TYPES);
	// PowerTypes
	pXML->SetVariableListTagPair(&m_pbDiplomacyPowerTypes, "DiplomacyPowers", GC.getDiplomacyPowerTypes(), NUM_DIPLOMACYPOWER_TYPES);
	// DiplomacyText
	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"DiplomacyText"))
	{
		pXML->SetStringList(&m_paszDiplomacyText, &m_iNumDiplomacyText);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	return true;
}

//======================================================================================================
//					CvSpecialistInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvSpecialistInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvSpecialistInfo::CvSpecialistInfo() :
m_iGreatPeopleUnitClass(NO_UNITCLASS),
m_iGreatPeopleRateChange(0),
m_iMissionType(NO_MISSION),
m_bVisible(false), 
m_piYieldChange(NULL), 
m_piCommerceChange(NULL), 
m_piFlavorValue(NULL),
m_iExperience(0)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvSpecialistInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvSpecialistInfo::~CvSpecialistInfo()
{
	SAFE_DELETE_ARRAY(m_piYieldChange);
	SAFE_DELETE_ARRAY(m_piCommerceChange);
	SAFE_DELETE_ARRAY(m_piFlavorValue);
}

int CvSpecialistInfo::getGreatPeopleUnitClass() const
{
	return m_iGreatPeopleUnitClass;
}

int CvSpecialistInfo::getGreatPeopleRateChange() const
{
	return m_iGreatPeopleRateChange;
}

int CvSpecialistInfo::getMissionType() const
{
	return m_iMissionType;
}

void CvSpecialistInfo::setMissionType(int iNewType)
{
	m_iMissionType = iNewType;
}

bool CvSpecialistInfo::isVisible() const		
{
	return m_bVisible;
}

int CvSpecialistInfo::getExperience() const
{
	return m_iExperience;
}

// Arrays

int CvSpecialistInfo::getYieldChange(int i) const		
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piYieldChange ? m_piYieldChange[i] : -1;
}

const int* CvSpecialistInfo::getYieldChangeArray() const		
{
	return m_piYieldChange;
}

int CvSpecialistInfo::getCommerceChange(int i) const
{
	FAssertMsg(i < NUM_COMMERCE_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piCommerceChange ? m_piCommerceChange[i] : -1;
}

int CvSpecialistInfo::getFlavorValue(int i) const		
{
	FAssertMsg(i < GC.getNumFlavorTypes(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piFlavorValue ? m_piFlavorValue[i] : -1;
}

const TCHAR* CvSpecialistInfo::getTexture() const		
{
	return m_szTexture;
}

void CvSpecialistInfo::setTexture(const TCHAR* szVal)
{
	m_szTexture = szVal;
}

//
// read from xml
//
bool CvSpecialistInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvHotkeyInfo::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(szTextVal, "Texture");
	setTexture(szTextVal);

	pXML->GetChildXmlValByName(&m_bVisible, "bVisible");

	pXML->GetChildXmlValByName(szTextVal, "GreatPeopleUnitClass");
	m_iGreatPeopleUnitClass = pXML->FindInInfoClass(szTextVal, GC.getUnitClassInfo(), sizeof(GC.getUnitClassInfo((UnitClassTypes)0)), GC.getNumUnitClassInfos());

	pXML->GetChildXmlValByName(&m_iGreatPeopleRateChange, "iGreatPeopleRateChange");

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"Yields"))
	{
		pXML->SetYields(&m_piYieldChange);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piYieldChange, NUM_YIELD_TYPES);
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"Commerces"))
	{
		pXML->SetCommerce(&m_piCommerceChange);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piCommerceChange, NUM_COMMERCE_TYPES);
	}

	pXML->GetChildXmlValByName(&m_iExperience, "iExperience");

	pXML->SetVariableListTagPair(&m_piFlavorValue, "Flavors", GC.getFlavorTypes(), GC.getNumFlavorTypes());

	return true;
}

//======================================================================================================
//					CvTechInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvTechInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvTechInfo::CvTechInfo() :
m_iAdvisorType(NO_ADVISOR),
m_iAIWeight(0),
m_iAITradeModifier(0),
m_iResearchCost(0),
m_iEra(NO_ERA),
m_iTradeRoutes(0),
m_iFeatureProductionModifier(0),
m_iWorkerSpeedModifier(0),
m_iFirstFreeUnitClass(NO_UNITCLASS),
m_iHealth(0),
m_iHappiness(0),
m_iFirstFreeTechs(0),
m_iAssetValue(0),
m_iPowerValue(0),
m_iGridX(0),
m_iGridY(0),
m_bRepeat(false),
m_bTrade(false),
m_bDisable(false),
m_bGoodyTech(false),
m_bExtraWaterSeeFrom(false),
m_bMapCentering(false),
m_bMapVisible(false),
m_bMapTrading(false),
m_bTechTrading(false),
m_bGoldTrading(false),
m_bOpenBordersTrading(false),
m_bDefensivePactTrading(false),
m_bPermanentAllianceTrading(false),
m_bVassalStateTrading(false),
m_bBridgeBuilding(false),
m_bIrrigation(false),
m_bIgnoreIrrigation(false),
m_bWaterWork(false),
m_piDomainExtraMoves(NULL), 
m_piFlavorValue(NULL), 
m_piPrereqOrTechs(NULL),
m_piPrereqAndTechs(NULL), 
m_pbCommerceFlexible(NULL), 
m_pbTerrainTrade(NULL)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvTechInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvTechInfo::~CvTechInfo()
{
	SAFE_DELETE_ARRAY(m_piDomainExtraMoves);
	SAFE_DELETE_ARRAY(m_piFlavorValue);
	SAFE_DELETE_ARRAY(m_piPrereqOrTechs);
	SAFE_DELETE_ARRAY(m_piPrereqAndTechs);
	SAFE_DELETE_ARRAY(m_pbCommerceFlexible);
	SAFE_DELETE_ARRAY(m_pbTerrainTrade);
}

int CvTechInfo::getAdvisorType() const
{
	return m_iAdvisorType;
}

int CvTechInfo::getAIWeight() const
{
	return m_iAIWeight;
}

int CvTechInfo::getAITradeModifier() const
{
	return m_iAITradeModifier;
}

int CvTechInfo::getResearchCost() const			
{
	return m_iResearchCost;
}

int CvTechInfo::getEra() const			
{
	return m_iEra;
}

int CvTechInfo::getTradeRoutes() const
{
	return m_iTradeRoutes;
}

int CvTechInfo::getFeatureProductionModifier() const	
{
	return m_iFeatureProductionModifier;
}

int CvTechInfo::getWorkerSpeedModifier() const	
{
	return m_iWorkerSpeedModifier;
}

int CvTechInfo::getFirstFreeUnitClass() const	
{
	return m_iFirstFreeUnitClass;
}

int CvTechInfo::getHealth() const	
{
	return m_iHealth;
}

int CvTechInfo::getHappiness() const
{
	return m_iHappiness;
}

int CvTechInfo::getFirstFreeTechs() const			
{
	return m_iFirstFreeTechs;
}

int CvTechInfo::getAssetValue() const
{
	return m_iAssetValue;
}

int CvTechInfo::getPowerValue() const
{
	return m_iPowerValue;
}

int CvTechInfo::getGridX() const			
{
	return m_iGridX;
}

int CvTechInfo::getGridY() const			
{
	return m_iGridY;
}

bool CvTechInfo::isRepeat() const	
{
	return m_bRepeat;
}

bool CvTechInfo::isTrade() const	
{
	return m_bTrade;
}

bool CvTechInfo::isDisable() const
{
	return m_bDisable;
}

bool CvTechInfo::isGoodyTech() const
{
	return m_bGoodyTech;
}

bool CvTechInfo::isExtraWaterSeeFrom() const	
{
	return m_bExtraWaterSeeFrom;
}

bool CvTechInfo::isMapCentering() const
{
	return m_bMapCentering;
}

bool CvTechInfo::isMapVisible() const
{
	return m_bMapVisible;
}

bool CvTechInfo::isMapTrading() const
{
	return m_bMapTrading;
}

bool CvTechInfo::isTechTrading() const
{
	return m_bTechTrading;
}

bool CvTechInfo::isGoldTrading() const
{
	return m_bGoldTrading;
}

bool CvTechInfo::isOpenBordersTrading() const
{
	return m_bOpenBordersTrading;
}

bool CvTechInfo::isDefensivePactTrading() const
{
	return m_bDefensivePactTrading;
}

bool CvTechInfo::isPermanentAllianceTrading() const
{
	return m_bPermanentAllianceTrading;
}

bool CvTechInfo::isVassalStateTrading() const
{
	return m_bVassalStateTrading;
}

bool CvTechInfo::isBridgeBuilding() const
{
	return m_bBridgeBuilding;
}

bool CvTechInfo::isIrrigation() const
{
	return m_bIrrigation;
}

bool CvTechInfo::isIgnoreIrrigation() const
{
	return m_bIgnoreIrrigation;
}

bool CvTechInfo::isWaterWork() const
{
	return m_bWaterWork;
}

const wchar* CvTechInfo::getQuote()		
{
	m_szTempText = gDLL->getText(m_szQuoteKey);
	return m_szTempText;
}

void CvTechInfo::setQuoteKey(const TCHAR* szVal)
{
	m_szQuoteKey = szVal;
}

const TCHAR* CvTechInfo::getSound() const			
{
	return m_szSound;
}

void CvTechInfo::setSound(const TCHAR* szVal)
{
	m_szSound = szVal;
}

const TCHAR* CvTechInfo::getSoundMP() const			
{
	return m_szSoundMP;
}

void CvTechInfo::setSoundMP(const TCHAR* szVal)
{
	m_szSoundMP = szVal;
}

// Arrays

int CvTechInfo::getDomainExtraMoves(int i) const			
{
	return m_piDomainExtraMoves ? m_piDomainExtraMoves[i] : -1;
}

int CvTechInfo::getFlavorValue(int i) const			
{
	FAssertMsg(i < GC.getNumFlavorTypes(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piFlavorValue ? m_piFlavorValue[i] : -1;
}

int CvTechInfo::getPrereqOrTechs(int i) const	
{
	return m_piPrereqOrTechs ? m_piPrereqOrTechs[i] : -1;
}

int CvTechInfo::getPrereqAndTechs(int i) const
{
	return m_piPrereqAndTechs ? m_piPrereqAndTechs[i] : -1;
}

bool CvTechInfo::isCommerceFlexible(int i) const
{
	FAssertMsg(i < NUM_COMMERCE_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbCommerceFlexible ? m_pbCommerceFlexible[i] : false;
}

bool CvTechInfo::isTerrainTrade(int i) const
{
	return m_pbTerrainTrade ? m_pbTerrainTrade[i] : false;
}

void CvTechInfo::read(FDataStreamBase* stream)
{
	CvInfoBase::read(stream);

	uint uiFlag=0;
	stream->Read(&uiFlag);	// flags for expansion

	stream->Read(&m_iAdvisorType);
	stream->Read(&m_iAIWeight);
	stream->Read(&m_iAITradeModifier);
	stream->Read(&m_iResearchCost);
	stream->Read(&m_iEra);
	stream->Read(&m_iFirstFreeUnitClass);
	stream->Read(&m_iFeatureProductionModifier);
	stream->Read(&m_iWorkerSpeedModifier);
	stream->Read(&m_iTradeRoutes);
	stream->Read(&m_iHealth);
	stream->Read(&m_iHappiness);
	stream->Read(&m_iFirstFreeTechs);
	stream->Read(&m_iAssetValue);
	stream->Read(&m_iPowerValue);
	stream->Read(&m_bRepeat);
	stream->Read(&m_bTrade);
	stream->Read(&m_bDisable);
	stream->Read(&m_bGoodyTech);
	stream->Read(&m_bExtraWaterSeeFrom);
	stream->Read(&m_bMapCentering);
	stream->Read(&m_bMapVisible);
	stream->Read(&m_bMapTrading);
	stream->Read(&m_bTechTrading);
	stream->Read(&m_bGoldTrading);
	stream->Read(&m_bOpenBordersTrading);
	stream->Read(&m_bDefensivePactTrading);
	stream->Read(&m_bPermanentAllianceTrading);
	stream->Read(&m_bVassalStateTrading);
	stream->Read(&m_bBridgeBuilding);
	stream->Read(&m_bIrrigation);
	stream->Read(&m_bIgnoreIrrigation);
	stream->Read(&m_bWaterWork);
	stream->Read(&m_iGridX);
	stream->Read(&m_iGridY);

	SAFE_DELETE_ARRAY(m_piDomainExtraMoves);
	m_piDomainExtraMoves = new int[NUM_DOMAIN_TYPES];
	stream->Read(NUM_DOMAIN_TYPES, m_piDomainExtraMoves);

	SAFE_DELETE_ARRAY(m_piFlavorValue);
	m_piFlavorValue = new int[GC.getNumFlavorTypes()];
	stream->Read(GC.getNumFlavorTypes(), m_piFlavorValue);

	SAFE_DELETE_ARRAY(m_piPrereqOrTechs);
	m_piPrereqOrTechs = new int[GC.getDefineINT("NUM_OR_TECH_PREREQS")];
	stream->Read(GC.getDefineINT("NUM_OR_TECH_PREREQS"), m_piPrereqOrTechs);

	SAFE_DELETE_ARRAY(m_piPrereqAndTechs);
	m_piPrereqAndTechs = new int[GC.getDefineINT("NUM_AND_TECH_PREREQS")];
	stream->Read(GC.getDefineINT("NUM_AND_TECH_PREREQS"), m_piPrereqAndTechs);

	SAFE_DELETE_ARRAY(m_pbCommerceFlexible);
	m_pbCommerceFlexible = new bool[NUM_COMMERCE_TYPES];
	stream->Read(NUM_COMMERCE_TYPES, m_pbCommerceFlexible);

	SAFE_DELETE_ARRAY(m_pbTerrainTrade);
	m_pbTerrainTrade = new bool[GC.getNumTerrainInfos()];
	stream->Read(GC.getNumTerrainInfos(), m_pbTerrainTrade);

	stream->ReadString(m_szQuoteKey);
	stream->ReadString(m_szSound);
	stream->ReadString(m_szSoundMP);
}

void CvTechInfo::write(FDataStreamBase* stream)
{
	CvInfoBase::write(stream);

	uint uiFlag=0;
	stream->Write(uiFlag);		// flag for expansion

	stream->Write(m_iAdvisorType);
	stream->Write(m_iAIWeight);
	stream->Write(m_iAITradeModifier);
	stream->Write(m_iResearchCost);
	stream->Write(m_iEra);
	stream->Write(m_iFirstFreeUnitClass);
	stream->Write(m_iFeatureProductionModifier);
	stream->Write(m_iWorkerSpeedModifier);
	stream->Write(m_iTradeRoutes);
	stream->Write(m_iHealth);
	stream->Write(m_iHappiness);
	stream->Write(m_iFirstFreeTechs);
	stream->Write(m_iAssetValue);
	stream->Write(m_iPowerValue);
	stream->Write(m_bRepeat);
	stream->Write(m_bTrade);
	stream->Write(m_bDisable);
	stream->Write(m_bGoodyTech);
	stream->Write(m_bExtraWaterSeeFrom);
	stream->Write(m_bMapCentering);
	stream->Write(m_bMapVisible);
	stream->Write(m_bMapTrading);
	stream->Write(m_bTechTrading);
	stream->Write(m_bGoldTrading);
	stream->Write(m_bOpenBordersTrading);
	stream->Write(m_bDefensivePactTrading);
	stream->Write(m_bPermanentAllianceTrading);
	stream->Write(m_bVassalStateTrading);
	stream->Write(m_bBridgeBuilding);
	stream->Write(m_bIrrigation);
	stream->Write(m_bIgnoreIrrigation);
	stream->Write(m_bWaterWork);
	stream->Write(m_iGridX);
	stream->Write(m_iGridY);
	
	stream->Write(NUM_DOMAIN_TYPES, m_piDomainExtraMoves);
	stream->Write(GC.getNumFlavorTypes(), m_piFlavorValue);
	stream->Write(GC.getDefineINT("NUM_OR_TECH_PREREQS"), m_piPrereqOrTechs);
	stream->Write(GC.getDefineINT("NUM_AND_TECH_PREREQS"), m_piPrereqAndTechs);
	stream->Write(NUM_COMMERCE_TYPES, m_pbCommerceFlexible);
	stream->Write(GC.getNumTerrainInfos(), m_pbTerrainTrade);

	stream->WriteString(m_szQuoteKey);
	stream->WriteString(m_szSound);
	stream->WriteString(m_szSoundMP);
}

bool CvTechInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(szTextVal, "Advisor");
	m_iAdvisorType = pXML->FindInInfoClass(szTextVal, GC.getAdvisorInfo(), sizeof(GC.getAdvisorInfo((AdvisorTypes)0)), GC.getNumAdvisorInfos());

	pXML->GetChildXmlValByName(&m_iAIWeight, "iAIWeight");
	pXML->GetChildXmlValByName(&m_iAITradeModifier, "iAITradeModifier");
	pXML->GetChildXmlValByName(&m_iResearchCost, "iCost");

	pXML->GetChildXmlValByName(szTextVal, "Era");
	m_iEra = pXML->FindInInfoClass(szTextVal, GC.getEraInfo(), sizeof(GC.getEraInfo((EraTypes)0)), GC.getNumEraInfos());

	pXML->GetChildXmlValByName(szTextVal, "FirstFreeUnitClass");
	m_iFirstFreeUnitClass = pXML->FindInInfoClass(szTextVal, GC.getUnitClassInfo(), sizeof(GC.getUnitClassInfo((UnitClassTypes)0)), GC.getNumUnitClassInfos());

	pXML->GetChildXmlValByName(&m_iFeatureProductionModifier, "iFeatureProductionModifier");
	pXML->GetChildXmlValByName(&m_iWorkerSpeedModifier, "iWorkerSpeedModifier");
	pXML->GetChildXmlValByName(&m_iTradeRoutes, "iTradeRoutes");
	pXML->GetChildXmlValByName(&m_iHealth, "iHealth");
	pXML->GetChildXmlValByName(&m_iHappiness, "iHappiness");
	pXML->GetChildXmlValByName(&m_iFirstFreeTechs, "iFirstFreeTechs");
	pXML->GetChildXmlValByName(&m_iAssetValue, "iAsset");
	pXML->GetChildXmlValByName(&m_iPowerValue, "iPower");
	pXML->GetChildXmlValByName(&m_bRepeat, "bRepeat");
	pXML->GetChildXmlValByName(&m_bTrade, "bTrade");
	pXML->GetChildXmlValByName(&m_bDisable, "bDisable");
	pXML->GetChildXmlValByName(&m_bGoodyTech, "bGoodyTech");
	pXML->GetChildXmlValByName(&m_bExtraWaterSeeFrom, "bExtraWaterSeeFrom");
	pXML->GetChildXmlValByName(&m_bMapCentering, "bMapCentering");
	pXML->GetChildXmlValByName(&m_bMapVisible, "bMapVisible");
	pXML->GetChildXmlValByName(&m_bMapTrading, "bMapTrading");
	pXML->GetChildXmlValByName(&m_bTechTrading, "bTechTrading");
	pXML->GetChildXmlValByName(&m_bGoldTrading, "bGoldTrading");
	pXML->GetChildXmlValByName(&m_bOpenBordersTrading, "bOpenBordersTrading");
	pXML->GetChildXmlValByName(&m_bDefensivePactTrading, "bDefensivePactTrading");
	pXML->GetChildXmlValByName(&m_bPermanentAllianceTrading, "bPermanentAllianceTrading");
	pXML->GetChildXmlValByName(&m_bVassalStateTrading, "bVassalTrading");
	pXML->GetChildXmlValByName(&m_bBridgeBuilding, "bBridgeBuilding");
	pXML->GetChildXmlValByName(&m_bIrrigation, "bIrrigation");
	pXML->GetChildXmlValByName(&m_bIgnoreIrrigation, "bIgnoreIrrigation");
	pXML->GetChildXmlValByName(&m_bWaterWork, "bWaterWork");
	pXML->GetChildXmlValByName(&m_iGridX, "iGridX");
	pXML->GetChildXmlValByName(&m_iGridY, "iGridY");

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"CommerceFlexible"))
	{
		pXML->SetCommerce(&m_pbCommerceFlexible);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_pbCommerceFlexible, NUM_COMMERCE_TYPES);
	}

	pXML->SetVariableListTagPair(&m_piDomainExtraMoves, "DomainExtraMoves", GC.getDomainInfo(), sizeof(GC.getDomainInfo((DomainTypes)0)), NUM_DOMAIN_TYPES);
	pXML->SetVariableListTagPair(&m_pbTerrainTrade, "TerrainTrades", GC.getTerrainInfo(), sizeof(GC.getTerrainInfo((TerrainTypes)0)), GC.getNumTerrainInfos(), false);
	pXML->SetVariableListTagPair(&m_piFlavorValue, "Flavors", GC.getFlavorTypes(), GC.getNumFlavorTypes());

	pXML->GetChildXmlValByName(szTextVal, "Quote");
	setQuoteKey(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "Sound");
	setSound(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "SoundMP");
	setSoundMP(szTextVal);

	return true;
}

//======================================================================================================
//					CvPromotionInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvPromotionInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvPromotionInfo::CvPromotionInfo() :
m_iPrereqPromotion(NO_PROMOTION),
m_iPrereqOrPromotion1(NO_PROMOTION),
m_iPrereqOrPromotion2(NO_PROMOTION),
m_iTechPrereq(NO_TECH),
m_iVisibilityChange(0),
m_iMovesChange(0),
m_iMoveDiscountChange(0),
m_iWithdrawalChange(0),
m_iCollateralDamageChange(0),
m_iBombardRateChange(0),	
m_iFirstStrikesChange(0),
m_iChanceFirstStrikesChange(0),
m_iEnemyHealChange(0),
m_iNeutralHealChange(0),
m_iFriendlyHealChange(0),
m_iSameTileHealChange(0),
m_iAdjacentTileHealChange(0),
m_iCombatPercent(0),
m_iCityAttackPercent(0),
m_iCityDefensePercent(0),
m_iHillsAttackPercent(0),
m_iHillsDefensePercent(0),
m_iCommandType(NO_COMMAND),
m_iRevoltProtection(0),
m_iCollateralDamageProtection(0),
m_iPillageChange(0),
m_iUpgradeDiscount(0),
m_iExperiencePercent(0),
m_iKamikazePercent(0),
m_bLeader(false),  // Warlords
m_bBlitz(false),
m_bAmphib(false),
m_bRiver(false),
m_bEnemyRoute(false),
m_bAlwaysHeal(false),
m_bHillsDoubleMove(false),
m_bImmuneToFirstStrikes(false),
m_piTerrainDefensePercent(NULL),
m_piFeatureDefensePercent(NULL),
m_piUnitCombatModifierPercent(NULL),
m_piDomainModifierPercent(NULL),
m_pbTerrainDoubleMove(NULL),
m_pbFeatureDoubleMove(NULL),
m_pbUnitCombat(NULL)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvPromotionInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvPromotionInfo::~CvPromotionInfo()
{
	SAFE_DELETE_ARRAY(m_piTerrainDefensePercent);
	SAFE_DELETE_ARRAY(m_piFeatureDefensePercent);
	SAFE_DELETE_ARRAY(m_piUnitCombatModifierPercent);
	SAFE_DELETE_ARRAY(m_piDomainModifierPercent);
	SAFE_DELETE_ARRAY(m_pbTerrainDoubleMove);
	SAFE_DELETE_ARRAY(m_pbFeatureDoubleMove);
	SAFE_DELETE_ARRAY(m_pbUnitCombat);
}

int CvPromotionInfo::getPrereqPromotion() const			
{
	return m_iPrereqPromotion;
}

void CvPromotionInfo::setPrereqPromotion(int i)				
{
	m_iPrereqPromotion = i;
}

int CvPromotionInfo::getPrereqOrPromotion1() const			
{
	return m_iPrereqOrPromotion1;
}

void CvPromotionInfo::setPrereqOrPromotion1(int i)				
{
	m_iPrereqOrPromotion1 = i;
}

int CvPromotionInfo::getPrereqOrPromotion2() const			
{
	return m_iPrereqOrPromotion2;
}

void CvPromotionInfo::setPrereqOrPromotion2(int i)				
{
	m_iPrereqOrPromotion2 = i;
}

int CvPromotionInfo::getTechPrereq() const	
{
	return m_iTechPrereq;
}

int CvPromotionInfo::getVisibilityChange() const
{
	return m_iVisibilityChange;
}

int CvPromotionInfo::getMovesChange() const	
{
	return m_iMovesChange;
}

int CvPromotionInfo::getMoveDiscountChange() const			
{
	return m_iMoveDiscountChange;
}

int CvPromotionInfo::getWithdrawalChange() const
{
	return m_iWithdrawalChange;
}

int CvPromotionInfo::getCollateralDamageChange() const	
{
	return m_iCollateralDamageChange;
}

int CvPromotionInfo::getBombardRateChange() const
{
	return m_iBombardRateChange;
}

int CvPromotionInfo::getFirstStrikesChange() const			
{
	return m_iFirstStrikesChange;
}

int CvPromotionInfo::getChanceFirstStrikesChange() const
{
	return m_iChanceFirstStrikesChange;
}

int CvPromotionInfo::getEnemyHealChange() const
{
	return m_iEnemyHealChange;
}

int CvPromotionInfo::getNeutralHealChange() const
{
	return m_iNeutralHealChange;
}

int CvPromotionInfo::getFriendlyHealChange() const			
{
	return m_iFriendlyHealChange;
}

int CvPromotionInfo::getSameTileHealChange() const			
{
	return m_iSameTileHealChange;
}

int CvPromotionInfo::getAdjacentTileHealChange() const	
{
	return m_iAdjacentTileHealChange;
}

int CvPromotionInfo::getCombatPercent() const
{
	return m_iCombatPercent;
}

int CvPromotionInfo::getCityAttackPercent() const
{
	return m_iCityAttackPercent;
}

int CvPromotionInfo::getCityDefensePercent() const
{
	return m_iCityDefensePercent;
}

int CvPromotionInfo::getHillsAttackPercent() const
{
	return m_iHillsAttackPercent;
}

int CvPromotionInfo::getHillsDefensePercent() const
{
	return m_iHillsDefensePercent;
}

int CvPromotionInfo::getCommandType() const
{
	return m_iCommandType;
}

void CvPromotionInfo::setCommandType(int iNewType)
{
	m_iCommandType = iNewType;
}

int CvPromotionInfo::getRevoltProtection() const
{
	return m_iRevoltProtection;
}

int CvPromotionInfo::getCollateralDamageProtection() const
{
	return m_iCollateralDamageProtection;
}

int CvPromotionInfo::getPillageChange() const
{
	return m_iPillageChange;
}

int CvPromotionInfo::getUpgradeDiscount() const
{
	return m_iUpgradeDiscount;
}

int CvPromotionInfo::getExperiencePercent() const
{
	return m_iExperiencePercent;
}

int CvPromotionInfo::getKamikazePercent() const			
{
	return m_iKamikazePercent;
}

bool CvPromotionInfo::isLeader() const			
{
	return m_bLeader;
}

bool CvPromotionInfo::isBlitz() const			
{
	return m_bBlitz;
}

bool CvPromotionInfo::isAmphib() const			
{
	return m_bAmphib;
}

bool CvPromotionInfo::isRiver() const			
{
	return m_bRiver;
}

bool CvPromotionInfo::isEnemyRoute() const	
{
	return m_bEnemyRoute;
}

bool CvPromotionInfo::isAlwaysHeal() const	
{
	return m_bAlwaysHeal;
}

bool CvPromotionInfo::isHillsDoubleMove() const
{
	return m_bHillsDoubleMove;
}

bool CvPromotionInfo::isImmuneToFirstStrikes() const
{
	return m_bImmuneToFirstStrikes;
}

const TCHAR* CvPromotionInfo::getSound() const										
{
	return m_szSound;
}

void CvPromotionInfo::setSound(const TCHAR* szVal)
{
	m_szSound = szVal;
}

// Arrays

int CvPromotionInfo::getTerrainDefensePercent(int i) const
{
	FAssertMsg(i < GC.getNumTerrainInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piTerrainDefensePercent ? m_piTerrainDefensePercent[i] : -1;
}

int CvPromotionInfo::getFeatureDefensePercent(int i) const
{
	FAssertMsg(i < GC.getNumFeatureInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piFeatureDefensePercent ? m_piFeatureDefensePercent[i] : -1;
}

int CvPromotionInfo::getUnitCombatModifierPercent(int i) const
{
	FAssertMsg(i < GC.getNumUnitCombatInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piUnitCombatModifierPercent ? m_piUnitCombatModifierPercent[i] : -1;
}

int CvPromotionInfo::getDomainModifierPercent(int i) const
{
	FAssertMsg(i < NUM_DOMAIN_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piDomainModifierPercent ? m_piDomainModifierPercent[i] : -1;
}

bool CvPromotionInfo::getTerrainDoubleMove(int i) const
{
	FAssertMsg(i < GC.getNumTerrainInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbTerrainDoubleMove ? m_pbTerrainDoubleMove[i] : false;
}

bool CvPromotionInfo::getFeatureDoubleMove(int i) const
{
	FAssertMsg(i < GC.getNumFeatureInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbFeatureDoubleMove ? m_pbFeatureDoubleMove[i] : false;
}

bool CvPromotionInfo::getUnitCombat(int i) const	
{
	FAssertMsg(i < GC.getNumUnitCombatInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbUnitCombat ? m_pbUnitCombat[i] : false;
}

void CvPromotionInfo::read(FDataStreamBase* stream)
{
	CvHotkeyInfo::read(stream);

	uint uiFlag=0;
	stream->Read(&uiFlag);		// flag for expansion
	
	stream->Read(&m_iPrereqPromotion);			
	stream->Read(&m_iPrereqOrPromotion1);			
	stream->Read(&m_iPrereqOrPromotion2);			

	stream->Read(&m_iTechPrereq);							
	stream->Read(&m_iVisibilityChange);					
	stream->Read(&m_iMovesChange);						
	stream->Read(&m_iMoveDiscountChange);			
	stream->Read(&m_iWithdrawalChange);				
	stream->Read(&m_iCollateralDamageChange);	
	stream->Read(&m_iBombardRateChange);			
	stream->Read(&m_iFirstStrikesChange);				
	stream->Read(&m_iChanceFirstStrikesChange);	
	stream->Read(&m_iEnemyHealChange);				
	stream->Read(&m_iNeutralHealChange);				
	stream->Read(&m_iFriendlyHealChange);				
	stream->Read(&m_iSameTileHealChange);			
	stream->Read(&m_iAdjacentTileHealChange);		
	stream->Read(&m_iCombatPercent);
	stream->Read(&m_iCityAttackPercent);
	stream->Read(&m_iCityDefensePercent);
	stream->Read(&m_iHillsAttackPercent);
	stream->Read(&m_iHillsDefensePercent);
	stream->Read(&m_iCommandType);
	stream->Read(&m_iRevoltProtection);
	stream->Read(&m_iCollateralDamageProtection);
	stream->Read(&m_iPillageChange);
	stream->Read(&m_iUpgradeDiscount);
	stream->Read(&m_iExperiencePercent);
	stream->Read(&m_iKamikazePercent);

	stream->Read(&m_bLeader);
	stream->Read(&m_bBlitz);
	stream->Read(&m_bAmphib);
	stream->Read(&m_bRiver);
	stream->Read(&m_bEnemyRoute);
	stream->Read(&m_bAlwaysHeal);
	stream->Read(&m_bHillsDoubleMove);
	stream->Read(&m_bImmuneToFirstStrikes);				

	stream->ReadString(m_szSound);

	// Arrays

	SAFE_DELETE_ARRAY(m_piTerrainDefensePercent);
	m_piTerrainDefensePercent = new int[GC.getNumTerrainInfos()];
	stream->Read(GC.getNumTerrainInfos(), m_piTerrainDefensePercent);

	SAFE_DELETE_ARRAY(m_piFeatureDefensePercent);
	m_piFeatureDefensePercent = new int[GC.getNumFeatureInfos()];
	stream->Read(GC.getNumFeatureInfos(), m_piFeatureDefensePercent);

	SAFE_DELETE_ARRAY(m_piUnitCombatModifierPercent);
	m_piUnitCombatModifierPercent = new int[GC.getNumUnitCombatInfos()];
	stream->Read(GC.getNumUnitCombatInfos(), m_piUnitCombatModifierPercent);

	SAFE_DELETE_ARRAY(m_piDomainModifierPercent);
	m_piDomainModifierPercent = new int[NUM_DOMAIN_TYPES];
	stream->Read(NUM_DOMAIN_TYPES, m_piDomainModifierPercent);

	SAFE_DELETE_ARRAY(m_pbTerrainDoubleMove);
	m_pbTerrainDoubleMove = new bool[GC.getNumTerrainInfos()];
	stream->Read(GC.getNumTerrainInfos(), m_pbTerrainDoubleMove);

	SAFE_DELETE_ARRAY(m_pbFeatureDoubleMove);
	m_pbFeatureDoubleMove = new bool[GC.getNumFeatureInfos()];
	stream->Read(GC.getNumFeatureInfos(), m_pbFeatureDoubleMove);

	SAFE_DELETE_ARRAY(m_pbUnitCombat);
	m_pbUnitCombat = new bool[GC.getNumUnitCombatInfos()];
	stream->Read(GC.getNumUnitCombatInfos(), m_pbUnitCombat);
}

void CvPromotionInfo::write(FDataStreamBase* stream)
{
	CvHotkeyInfo::write(stream);

	uint uiFlag = 0;
	stream->Write(uiFlag);		// flag for expansion

	stream->Write(m_iPrereqPromotion);			
	stream->Write(m_iPrereqOrPromotion1);			
	stream->Write(m_iPrereqOrPromotion2);			

	stream->Write(m_iTechPrereq);							
	stream->Write(m_iVisibilityChange);					
	stream->Write(m_iMovesChange);						
	stream->Write(m_iMoveDiscountChange);			
	stream->Write(m_iWithdrawalChange);				
	stream->Write(m_iCollateralDamageChange);	
	stream->Write(m_iBombardRateChange);			
	stream->Write(m_iFirstStrikesChange);				
	stream->Write(m_iChanceFirstStrikesChange);	
	stream->Write(m_iEnemyHealChange);				
	stream->Write(m_iNeutralHealChange);				
	stream->Write(m_iFriendlyHealChange);				
	stream->Write(m_iSameTileHealChange);			
	stream->Write(m_iAdjacentTileHealChange);		
	stream->Write(m_iCombatPercent);
	stream->Write(m_iCityAttackPercent);
	stream->Write(m_iCityDefensePercent);
	stream->Write(m_iHillsAttackPercent);
	stream->Write(m_iHillsDefensePercent);
	stream->Write(m_iCommandType);
	stream->Write(m_iRevoltProtection);
	stream->Write(m_iCollateralDamageProtection);
	stream->Write(m_iPillageChange);
	stream->Write(m_iUpgradeDiscount);
	stream->Write(m_iExperiencePercent);
	stream->Write(m_iKamikazePercent);

	stream->Write(m_bLeader);
	stream->Write(m_bBlitz);
	stream->Write(m_bAmphib);
	stream->Write(m_bRiver);
	stream->Write(m_bEnemyRoute);
	stream->Write(m_bAlwaysHeal);
	stream->Write(m_bHillsDoubleMove);
	stream->Write(m_bImmuneToFirstStrikes);

	stream->WriteString(m_szSound);

	// Arrays

	stream->Write(GC.getNumTerrainInfos(), m_piTerrainDefensePercent);
	stream->Write(GC.getNumFeatureInfos(), m_piFeatureDefensePercent);
	stream->Write(GC.getNumUnitCombatInfos(), m_piUnitCombatModifierPercent);
	stream->Write(NUM_DOMAIN_TYPES, m_piDomainModifierPercent);
	stream->Write(GC.getNumTerrainInfos(), m_pbTerrainDoubleMove);
	stream->Write(GC.getNumFeatureInfos(), m_pbFeatureDoubleMove);
	stream->Write(GC.getNumUnitCombatInfos(), m_pbUnitCombat);
}

bool CvPromotionInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvHotkeyInfo::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(szTextVal, "Sound");
	setSound(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "TechPrereq");
	m_iTechPrereq = pXML->FindInInfoClass(szTextVal, GC.getTechInfo(), sizeof(GC.getTechInfo((TechTypes)0)), GC.getNumTechInfos());

	pXML->GetChildXmlValByName(&m_bLeader, "bLeader");
	if (m_bLeader)
	{
		m_bGraphicalOnly = true;  // don't show in Civilopedia list of promotions
	}
	pXML->GetChildXmlValByName(&m_bBlitz, "bBlitz");
	pXML->GetChildXmlValByName(&m_bAmphib, "bAmphib");
	pXML->GetChildXmlValByName(&m_bRiver, "bRiver");
	pXML->GetChildXmlValByName(&m_bEnemyRoute, "bEnemyRoute");
	pXML->GetChildXmlValByName(&m_bAlwaysHeal, "bAlwaysHeal");
	pXML->GetChildXmlValByName(&m_bHillsDoubleMove, "bHillsDoubleMove");
	pXML->GetChildXmlValByName(&m_bImmuneToFirstStrikes, "bImmuneToFirstStrikes");
	pXML->GetChildXmlValByName(&m_iVisibilityChange, "iVisibilityChange");
	pXML->GetChildXmlValByName(&m_iMovesChange, "iMovesChange");
	pXML->GetChildXmlValByName(&m_iMoveDiscountChange, "iMoveDiscountChange");
	pXML->GetChildXmlValByName(&m_iWithdrawalChange, "iWithdrawalChange");
	pXML->GetChildXmlValByName(&m_iCollateralDamageChange, "iCollateralDamageChange");
	pXML->GetChildXmlValByName(&m_iBombardRateChange, "iBombardRateChange");
	pXML->GetChildXmlValByName(&m_iFirstStrikesChange, "iFirstStrikesChange");
	pXML->GetChildXmlValByName(&m_iChanceFirstStrikesChange, "iChanceFirstStrikesChange");
	pXML->GetChildXmlValByName(&m_iEnemyHealChange, "iEnemyHealChange");
	pXML->GetChildXmlValByName(&m_iNeutralHealChange, "iNeutralHealChange");
	pXML->GetChildXmlValByName(&m_iFriendlyHealChange, "iFriendlyHealChange");
	pXML->GetChildXmlValByName(&m_iSameTileHealChange, "iSameTileHealChange");
	pXML->GetChildXmlValByName(&m_iAdjacentTileHealChange, "iAdjacentTileHealChange");
	pXML->GetChildXmlValByName(&m_iCombatPercent, "iCombatPercent");
	pXML->GetChildXmlValByName(&m_iCityAttackPercent, "iCityAttack");
	pXML->GetChildXmlValByName(&m_iCityDefensePercent, "iCityDefense");
	pXML->GetChildXmlValByName(&m_iHillsAttackPercent, "iHillsAttack");
	pXML->GetChildXmlValByName(&m_iHillsDefensePercent, "iHillsDefense");
	pXML->GetChildXmlValByName(&m_iRevoltProtection, "iRevoltProtection");
	pXML->GetChildXmlValByName(&m_iCollateralDamageProtection, "iCollateralDamageProtection");
	pXML->GetChildXmlValByName(&m_iPillageChange, "iPillageChange");
	pXML->GetChildXmlValByName(&m_iUpgradeDiscount, "iUpgradeDiscount");
	pXML->GetChildXmlValByName(&m_iExperiencePercent, "iExperiencePercent");
	pXML->GetChildXmlValByName(&m_iKamikazePercent, "iKamikazePercent");

	pXML->SetVariableListTagPair(&m_piTerrainDefensePercent, "TerrainDefenses", GC.getTerrainInfo(), sizeof(GC.getTerrainInfo((TerrainTypes)0)), GC.getNumTerrainInfos());
	pXML->SetVariableListTagPair(&m_piFeatureDefensePercent, "FeatureDefenses", GC.getFeatureInfo(), sizeof(GC.getFeatureInfo((FeatureTypes)0)), GC.getNumFeatureInfos());
	pXML->SetVariableListTagPair(&m_piUnitCombatModifierPercent, "UnitCombatMods", GC.getUnitCombatInfo(), sizeof(GC.getUnitCombatInfo((UnitCombatTypes)0)), GC.getNumUnitCombatInfos());
	pXML->SetVariableListTagPair(&m_piDomainModifierPercent, "DomainMods", GC.getDomainInfo(), sizeof(GC.getDomainInfo((DomainTypes)0)), NUM_DOMAIN_TYPES);

	pXML->SetVariableListTagPair(&m_pbTerrainDoubleMove, "TerrainDoubleMoves", GC.getTerrainInfo(), sizeof(GC.getTerrainInfo((TerrainTypes)0)), GC.getNumTerrainInfos());
	pXML->SetVariableListTagPair(&m_pbFeatureDoubleMove, "FeatureDoubleMoves", GC.getFeatureInfo(), sizeof(GC.getFeatureInfo((FeatureTypes)0)), GC.getNumFeatureInfos());
	pXML->SetVariableListTagPair(&m_pbUnitCombat, "UnitCombats", GC.getUnitCombatInfo(), sizeof(GC.getUnitCombatInfo((UnitCombatTypes)0)), GC.getNumUnitCombatInfos());

	return true;
}
//======================================================================================================
//					CvMissionInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvMissionInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvMissionInfo::CvMissionInfo() :
m_iTime(0),
m_bSound(false),
m_bTarget(false),
m_bBuild(false),
m_bVisible(false),
m_eEntityEvent(ENTITY_EVENT_NONE)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvMissionInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvMissionInfo::~CvMissionInfo()
{
}

int CvMissionInfo::getTime() const		
{
	return m_iTime;
}

bool CvMissionInfo::isSound() const
{
	return m_bSound;
}

bool CvMissionInfo::isTarget() const
{
	return m_bTarget;
}

bool CvMissionInfo::isBuild() const	
{
	return m_bBuild;
}

bool CvMissionInfo::getVisible() const
{
	return m_bVisible;
}

const TCHAR* CvMissionInfo::getWaypoint() const
{
	return m_szWaypoint;
}

EntityEventTypes CvMissionInfo::getEntityEvent() const
{
	return m_eEntityEvent;
}

bool CvMissionInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTmp;
	if (!CvHotkeyInfo::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(m_szWaypoint, "Waypoint");
	pXML->GetChildXmlValByName(&m_iTime, "iTime");
	pXML->GetChildXmlValByName(&m_bSound, "bSound");
	pXML->GetChildXmlValByName(&m_bTarget, "bTarget");
	pXML->GetChildXmlValByName(&m_bBuild, "bBuild");
	pXML->GetChildXmlValByName(&m_bVisible, "bVisible");

	if ( pXML->GetChildXmlValByName(szTmp, "EntityEventType") )
	{
		m_eEntityEvent = (EntityEventTypes)pXML->FindInInfoClass(szTmp, GC.getEntityEventInfo(), sizeof(
			GC.getEntityEventInfo((EntityEventTypes)0)), GC.getNumEntityEventInfos() );
	}
	else
	{
		m_eEntityEvent = ENTITY_EVENT_NONE;
	}

	return true;
}

//======================================================================================================
//					CvControlInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvControlInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvControlInfo::CvControlInfo()
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvControlInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvControlInfo::~CvControlInfo()
{
}

bool CvControlInfo::read(CvXMLLoadUtility* pXML)
{
	if (!CvHotkeyInfo::read(pXML))
	{
		return false;
	}

	return true;
}

//======================================================================================================
//					CvCommandInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvCommandInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvCommandInfo::CvCommandInfo() :
m_iAutomate(NO_AUTOMATE),
m_bConfirmCommand(false),
m_bVisible(false),
m_bAll(false)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvCommandInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvCommandInfo::~CvCommandInfo()
{
}

int CvCommandInfo::getAutomate() const
{
	return m_iAutomate;
}

void CvCommandInfo::setAutomate(int i)
{
	m_iAutomate = i;
}

bool CvCommandInfo::getConfirmCommand() const
{
	return m_bConfirmCommand;
}

bool CvCommandInfo::getVisible() const
{
	return m_bVisible;
}

bool CvCommandInfo::getAll() const
{
	return m_bAll;
}

bool CvCommandInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvHotkeyInfo::read(pXML))
	{
		return false;
	}

	if (pXML->GetChildXmlValByName(szTextVal, "Automate"))
	{		
		setAutomate(GC.getTypesEnum(szTextVal));
	}

	pXML->GetChildXmlValByName(&m_bConfirmCommand, "bConfirmCommand");
	pXML->GetChildXmlValByName(&m_bVisible, "bVisible");
	pXML->GetChildXmlValByName(&m_bAll, "bAll");

	return true;
}

//======================================================================================================
//					CvAutomateInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvAutomateInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvAutomateInfo::CvAutomateInfo() :
m_iCommand(NO_COMMAND),
m_iAutomate(NO_AUTOMATE),
m_bConfirmCommand(false),
m_bVisible(false)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvAutomateInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvAutomateInfo::~CvAutomateInfo()
{
}

int CvAutomateInfo::getCommand() const
{
	return m_iCommand;
}

void CvAutomateInfo::setCommand(int i)
{
	m_iCommand = i;
}

int CvAutomateInfo::getAutomate() const
{
	return m_iAutomate;
}

void CvAutomateInfo::setAutomate(int i)
{
	m_iAutomate = i;
}

bool CvAutomateInfo::getConfirmCommand() const
{
	return m_bConfirmCommand;
}

bool CvAutomateInfo::getVisible() const
{
	return m_bVisible;
}

bool CvAutomateInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvHotkeyInfo::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(szTextVal, "Command");
	setCommand(pXML->FindInInfoClass(szTextVal, GC.getCommandInfo(), sizeof(GC.getCommandInfo((CommandTypes)0)), NUM_COMMAND_TYPES));

	pXML->GetChildXmlValByName(szTextVal, "Automate");
	setAutomate(GC.getTypesEnum(szTextVal));

	pXML->GetChildXmlValByName(&m_bConfirmCommand, "bConfirmCommand");
	pXML->GetChildXmlValByName(&m_bVisible, "bVisible");

	return true;
}

//======================================================================================================
//					CvActionInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvActionInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvActionInfo::CvActionInfo() :
m_iOriginalIndex(-1),
m_eSubType(NO_ACTIONSUBTYPE)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvActionInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvActionInfo::~CvActionInfo()
{
}

int CvActionInfo::getMissionData() const		
{

	if	(
				(ACTIONSUBTYPE_BUILD == m_eSubType)				||
				(ACTIONSUBTYPE_RELIGION == m_eSubType)		||
				(ACTIONSUBTYPE_SPECIALIST == m_eSubType)	||
				(ACTIONSUBTYPE_BUILDING == m_eSubType)
			)
	{
		return m_iOriginalIndex;
	}

	return -1;
}

int CvActionInfo::getCommandData() const		
{

	if	(
				(ACTIONSUBTYPE_PROMOTION == m_eSubType)	||
				(ACTIONSUBTYPE_UNIT == m_eSubType)
			)
	{
		return m_iOriginalIndex;
	}
	else if (ACTIONSUBTYPE_COMMAND == m_eSubType)
	{
		return GC.getCommandInfo((CommandTypes)m_iOriginalIndex).getAutomate();
	}
	else if (ACTIONSUBTYPE_AUTOMATE == m_eSubType)
	{
		return GC.getAutomateInfo(m_iOriginalIndex).getAutomate();
	}

	return -1;
}

int CvActionInfo::getAutomateType() const
{

	if (ACTIONSUBTYPE_COMMAND == m_eSubType)
	{
		return GC.getCommandInfo((CommandTypes)m_iOriginalIndex).getAutomate();
	}
	else if (ACTIONSUBTYPE_AUTOMATE == m_eSubType)
	{
		return GC.getAutomateInfo(m_iOriginalIndex).getAutomate();
	}

	return NO_AUTOMATE;
}

int CvActionInfo::getInterfaceModeType() const
{
	if (ACTIONSUBTYPE_INTERFACEMODE == m_eSubType)
	{
		return m_iOriginalIndex;
	}
	return NO_INTERFACEMODE;
}

int CvActionInfo::getMissionType() const		
{
	if (ACTIONSUBTYPE_BUILD == m_eSubType)
	{
		return GC.getBuildInfo((BuildTypes)m_iOriginalIndex).getMissionType();
	}
	else if (ACTIONSUBTYPE_RELIGION == m_eSubType)
	{
		return GC.getReligionInfo((ReligionTypes)m_iOriginalIndex).getMissionType();
	}
	else if (ACTIONSUBTYPE_SPECIALIST == m_eSubType)
	{
		return GC.getSpecialistInfo((SpecialistTypes)m_iOriginalIndex).getMissionType();
	}
	else if (ACTIONSUBTYPE_BUILDING == m_eSubType)
	{
		return GC.getBuildingInfo((BuildingTypes)m_iOriginalIndex).getMissionType();
	}
	else if (ACTIONSUBTYPE_MISSION == m_eSubType)
	{
		return m_iOriginalIndex;
	}

	return NO_MISSION;
}

int CvActionInfo::getCommandType() const		
{
	if (ACTIONSUBTYPE_COMMAND == m_eSubType)
	{
		return m_iOriginalIndex;
	}
	else if (ACTIONSUBTYPE_PROMOTION == m_eSubType)
	{
		return GC.getPromotionInfo((PromotionTypes)m_iOriginalIndex).getCommandType();
	}
	else if (ACTIONSUBTYPE_UNIT == m_eSubType)
	{
		return GC.getUnitInfo((UnitTypes)m_iOriginalIndex).getCommandType();
	}
	else if (ACTIONSUBTYPE_AUTOMATE == m_eSubType)
	{
		return GC.getAutomateInfo(m_iOriginalIndex).getCommand();
	}

	return NO_COMMAND;
}

int CvActionInfo::getControlType() const		
{
	if (ACTIONSUBTYPE_CONTROL == m_eSubType)
	{
		return m_iOriginalIndex;
	}
	return -1;
}

int CvActionInfo::getOriginalIndex() const		
{
	return m_iOriginalIndex;
}

void CvActionInfo::setOriginalIndex(int i)
{
	m_iOriginalIndex = i;
}

bool CvActionInfo::isConfirmCommand() const
{
	if	(ACTIONSUBTYPE_COMMAND == m_eSubType)
	{
		return GC.getCommandInfo((CommandTypes)m_iOriginalIndex).getConfirmCommand();
	}
	else if (ACTIONSUBTYPE_AUTOMATE == m_eSubType)
	{
		return GC.getAutomateInfo(m_iOriginalIndex).getConfirmCommand();
	}

	return false;
}

bool CvActionInfo::isVisible() const			
{

	if (ACTIONSUBTYPE_CONTROL == m_eSubType)
	{
		return false;
	}
	else if	(ACTIONSUBTYPE_COMMAND == m_eSubType)
	{
		return GC.getCommandInfo((CommandTypes)m_iOriginalIndex).getVisible();
	}
	else if (ACTIONSUBTYPE_AUTOMATE == m_eSubType)
	{
		return GC.getAutomateInfo(m_iOriginalIndex).getVisible();
	}
	else if (ACTIONSUBTYPE_MISSION == m_eSubType)
	{
		return GC.getMissionInfo((MissionTypes)m_iOriginalIndex).getVisible();
	}
	else if (ACTIONSUBTYPE_INTERFACEMODE== m_eSubType)
	{
		return GC.getInterfaceModeInfo((InterfaceModeTypes)m_iOriginalIndex).getVisible();
	}

	return true;
}

ActionSubTypes CvActionInfo::getSubType() const			
{
	return m_eSubType;
}

void CvActionInfo::setSubType(ActionSubTypes eSubType)
{
	m_eSubType = eSubType;
}

CvHotkeyInfo* CvActionInfo::getHotkeyInfo() const
{
	switch (getSubType())
	{
		case ACTIONSUBTYPE_INTERFACEMODE:
			return &GC.getInterfaceModeInfo((InterfaceModeTypes)getOriginalIndex());
			break;
		case ACTIONSUBTYPE_COMMAND:
			return &GC.getCommandInfo((CommandTypes)getOriginalIndex());
			break;
		case ACTIONSUBTYPE_BUILD:
			return &GC.getBuildInfo((BuildTypes)getOriginalIndex());
			break;
		case ACTIONSUBTYPE_PROMOTION:
			return &GC.getPromotionInfo((PromotionTypes)getOriginalIndex());
			break;
		case ACTIONSUBTYPE_UNIT:
			return &GC.getUnitInfo((UnitTypes)getOriginalIndex());
			break;
		case ACTIONSUBTYPE_RELIGION:
			return &GC.getReligionInfo((ReligionTypes)getOriginalIndex());
			break;
		case ACTIONSUBTYPE_SPECIALIST:
			return &GC.getSpecialistInfo((SpecialistTypes)getOriginalIndex());
			break;
		case ACTIONSUBTYPE_BUILDING:
			return &GC.getBuildingInfo((BuildingTypes)getOriginalIndex());
			break;
		case ACTIONSUBTYPE_CONTROL:
			return &GC.getControlInfo((ControlTypes)getOriginalIndex());
			break;
		case ACTIONSUBTYPE_AUTOMATE:
			return &GC.getAutomateInfo(getOriginalIndex());
			break;
		case ACTIONSUBTYPE_MISSION:
			return &GC.getMissionInfo((MissionTypes)getOriginalIndex());
			break;
	}

	FAssertMsg((0) ,"Unknown Action Subtype in CvActionInfo::getHotkeyInfo");
	return NULL;
}

const TCHAR* CvActionInfo::getType() const
{
	if (getHotkeyInfo())
	{
		return getHotkeyInfo()->getType();
	}

	return NULL;
}

const wchar* CvActionInfo::getDescription() const
{
	if (getHotkeyInfo())
	{
		return getHotkeyInfo()->getDescription();
	}

	return L"";
}

const wchar* CvActionInfo::getCivilopedia() const
{
	if (getHotkeyInfo())
	{
		return getHotkeyInfo()->getCivilopedia();
	}

	return L"";
}

const wchar* CvActionInfo::getHelp() const
{
	if (getHotkeyInfo())
	{
		return getHotkeyInfo()->getHelp();
	}

	return L"";
}

const wchar* CvActionInfo::getStrategy() const
{
	if (getHotkeyInfo())
	{
		return getHotkeyInfo()->getStrategy();
	}

	return L"";
}

const TCHAR* CvActionInfo::getButton() const
{
	if (getHotkeyInfo())
	{
		return getHotkeyInfo()->getButton();
	}

	return NULL;
}

const TCHAR* CvActionInfo::getTextKey() const
{
	if (getHotkeyInfo())
	{
		return getHotkeyInfo()->getTextKey();
	}

	return NULL;
}

const wchar* CvActionInfo::getTextKeyWide() const
{
	if (getHotkeyInfo())
	{
		return getHotkeyInfo()->getTextKeyWide();
	}

	return NULL;
}

int CvActionInfo::getActionInfoIndex() const
{
	if (getHotkeyInfo())
	{
		return getHotkeyInfo()->getActionInfoIndex();
	}

	return -1;
}

int CvActionInfo::getHotKeyVal() const
{
	if (getHotkeyInfo())
	{
		return getHotkeyInfo()->getHotKeyVal();
	}

	return -1;
}

int CvActionInfo::getHotKeyPriority() const
{
	if (getHotkeyInfo())
	{
		return getHotkeyInfo()->getHotKeyPriority();
	}

	return -1;
}

int CvActionInfo::getHotKeyValAlt() const
{
	if (getHotkeyInfo())
	{
		return getHotkeyInfo()->getHotKeyValAlt();
	}

	return -1;
}

int CvActionInfo::getHotKeyPriorityAlt() const
{
	if (getHotkeyInfo())
	{
		return getHotkeyInfo()->getHotKeyPriorityAlt();
	}

	return -1;
}

int CvActionInfo::getOrderPriority() const
{
	if (getHotkeyInfo())
	{
		return getHotkeyInfo()->getOrderPriority();
	}

	return -1;
}

bool CvActionInfo::isAltDown() const
{
	if (getHotkeyInfo())
	{
		return getHotkeyInfo()->isAltDown();
	}

	return false;
}

bool CvActionInfo::isShiftDown() const
{
	if (getHotkeyInfo())
	{
		return getHotkeyInfo()->isShiftDown();
	}

	return false;
}

bool CvActionInfo::isCtrlDown() const
{
	if (getHotkeyInfo())
	{
		return getHotkeyInfo()->isCtrlDown();
	}

	return false;
}

bool CvActionInfo::isAltDownAlt() const
{
	if (getHotkeyInfo())
	{
		return getHotkeyInfo()->isAltDownAlt();
	}

	return false;
}

bool CvActionInfo::isShiftDownAlt() const
{
	if (getHotkeyInfo())
	{
		return getHotkeyInfo()->isShiftDownAlt();
	}

	return false;
}

bool CvActionInfo::isCtrlDownAlt() const
{
	if (getHotkeyInfo())
	{
		return getHotkeyInfo()->isCtrlDownAlt();
	}

	return false;
}

const TCHAR* CvActionInfo::getHotKey() const
{
	if (getHotkeyInfo())
	{
		return getHotkeyInfo()->getHotKey();
	}

	return NULL;
}

std::wstring CvActionInfo::getHotKeyDescription() const
{
	if (getHotkeyInfo())
	{
		return getHotkeyInfo()->getHotKeyDescription();
	}

	return L"";
}

//======================================================================================================
//					CvUnitInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvUnitInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvUnitInfo::CvUnitInfo() :
m_iAIWeight(0),
m_iProductionCost(0),
m_iHurryCostModifier(0),
m_iMinAreaSize(0),
m_iMoves(0),
m_iAirRange(0),
m_iNukeRange(0),
m_iWorkRate(0),
m_iBaseDiscover(0),
m_iDiscoverMultiplier(0),
m_iBaseHurry(0),
m_iHurryMultiplier(0),
m_iBaseTrade(0),
m_iTradeMultiplier(0),
m_iGreatWorkCulture(0),
m_iCombat(0),
m_iAirCombat(0),
m_iAirCombatLimit(0),
m_iXPValueAttack(0),
m_iXPValueDefense(0),
m_iFirstStrikes(0),
m_iChanceFirstStrikes(0),
m_iInterceptionProbability(0),
m_iEvasionProbability(0),
m_iWithdrawalProbability(0),
m_iCollateralDamage(0),
m_iCollateralDamageLimit(0),
m_iCollateralDamageMaxUnits(0),
m_iCityAttackModifier(0),
m_iCityDefenseModifier(0),
m_iAnimalCombatModifier(0),
m_iHillsAttackModifier(0),
m_iHillsDefenseModifier(0),
m_iBombRate(0),
m_iBombardRate(0),
m_iSpecialCargo(0),
m_iDomainCargo(0),					
m_iCargoSpace(0),						
m_iConscriptionValue(0),
m_iCultureGarrisonValue(0),
m_iExtraCost(0),							
m_iAssetValue(0),						
m_iPowerValue(0),						
m_iUnitClassType(NO_UNITCLASS),
m_iSpecialUnitType(NO_SPECIALUNIT),
m_iUnitCaptureClassType(NO_UNITCLASS),
m_iUnitCombatType(NO_UNITCOMBAT),
m_iDomainType(NO_DOMAIN),
m_iDefaultUnitAIType(NO_UNITAI),
m_iInvisibleType(NO_INVISIBLE),
m_iSeeInvisibleType(NO_INVISIBLE),
m_iAdvisorType(NO_ADVISOR),
m_iHolyCity(NO_RELIGION),
m_iReligionType(NO_RELIGION),
m_iStateReligion(NO_RELIGION),					
m_iPrereqReligion(NO_RELIGION),					
m_iPrereqBuilding(NO_BUILDING),
m_iPrereqAndTech(NO_TECH),
m_iPrereqAndBonus(NO_BONUS),
m_iGroupSize(0),
m_iGroupDefinitions(0),
m_iUnitMeleeWaveSize(0),
m_iUnitRangedWaveSize(0),
m_iNumUnitNames(0),
m_iCommandType(NO_COMMAND),
m_bAnimal(false),
m_bFoodProduction(false),
m_bNoBadGoodies(false),
m_bOnlyDefensive(false),
m_bNoCapture(false),
m_bRivalTerritory(false),
m_bMilitaryHappiness(false),
m_bMilitarySupport(false),
m_bMilitaryProduction(false),
m_bPillage(false),
m_bSabotage(false),
m_bDestroy(false),
m_bStealPlans(false),
m_bInvestigate(false),
m_bCounterSpy(false),
m_bFound(false),
m_bGoldenAge(false),
m_bInvisible(false),
m_bFirstStrikeImmune(false),
m_bNoDefensiveBonus(false),
m_bIgnoreBuildingDefense(false),
m_bCanMoveImpassable(false),
m_bFlatMovementCost(false),
m_bIgnoreTerrainCost(false),
m_bNukeImmune(false),
m_bPrereqBonuses(false),
m_bPrereqReligion(false),
m_bMechanized(false),
m_bRenderBelowWater(false),
m_fUnitMaxSpeed(0.0f),
m_pbUpgradeUnitClass(NULL),
m_pbTargetUnitClass(NULL),
m_pbTargetUnitCombat(NULL),
m_pbDefenderUnitClass(NULL),
m_pbDefenderUnitCombat(NULL),
m_pbUnitAIType(NULL),
m_pbNotUnitAIType(NULL),
m_pbBuilds(NULL),
m_pbReligionSpreads(NULL),
m_pbGreatPeoples(NULL),
m_pbBuildings(NULL),
m_pbForceBuildings(NULL),
m_pbTerrainImpassable(NULL),
m_pbFeatureImpassable(NULL),
m_piPrereqAndTechs(NULL),
m_piPrereqOrBonuses(NULL),
m_piProductionTraits(NULL),
m_piFlavorValue(NULL),
m_piTerrainDefenseModifier(NULL),
m_piFeatureDefenseModifier(NULL),
m_piUnitClassAttackModifier(NULL),
m_piUnitClassDefenseModifier(NULL),
m_piUnitCombatModifier(NULL),
m_piUnitCombatCollateralImmune(NULL),
m_piDomainModifier(NULL),
m_piBonusProductionModifier(NULL),
m_piUnitGroupRequired(NULL),
m_pbTerrainNative(NULL),
m_pbFeatureNative(NULL),
m_pbFreePromotions(NULL),
m_paszEarlyArtDefineTags(NULL),
m_paszLateArtDefineTags(NULL),
m_paszMiddleArtDefineTags(NULL),
m_paszUnitNames(NULL)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvUnitInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvUnitInfo::~CvUnitInfo()
{
	SAFE_DELETE_ARRAY(m_pbUpgradeUnitClass);
	SAFE_DELETE_ARRAY(m_pbTargetUnitClass);
	SAFE_DELETE_ARRAY(m_pbTargetUnitCombat);
	SAFE_DELETE_ARRAY(m_pbDefenderUnitClass);
	SAFE_DELETE_ARRAY(m_pbDefenderUnitCombat);
	SAFE_DELETE_ARRAY(m_pbUnitAIType);
	SAFE_DELETE_ARRAY(m_pbNotUnitAIType);
	SAFE_DELETE_ARRAY(m_pbBuilds);
	SAFE_DELETE_ARRAY(m_pbReligionSpreads);
	SAFE_DELETE_ARRAY(m_pbGreatPeoples);
	SAFE_DELETE_ARRAY(m_pbBuildings);
	SAFE_DELETE_ARRAY(m_pbForceBuildings);
	SAFE_DELETE_ARRAY(m_pbTerrainImpassable);
	SAFE_DELETE_ARRAY(m_pbFeatureImpassable);
	SAFE_DELETE_ARRAY(m_piPrereqAndTechs);
	SAFE_DELETE_ARRAY(m_piPrereqOrBonuses);
	SAFE_DELETE_ARRAY(m_piProductionTraits);
	SAFE_DELETE_ARRAY(m_piFlavorValue);
	SAFE_DELETE_ARRAY(m_piTerrainDefenseModifier);
	SAFE_DELETE_ARRAY(m_piFeatureDefenseModifier);
	SAFE_DELETE_ARRAY(m_piUnitClassAttackModifier);
	SAFE_DELETE_ARRAY(m_piUnitClassDefenseModifier);
	SAFE_DELETE_ARRAY(m_piUnitCombatModifier);
	SAFE_DELETE_ARRAY(m_piUnitCombatCollateralImmune);
	SAFE_DELETE_ARRAY(m_piDomainModifier);
	SAFE_DELETE_ARRAY(m_piBonusProductionModifier);
	SAFE_DELETE_ARRAY(m_piUnitGroupRequired);
	SAFE_DELETE_ARRAY(m_pbTerrainNative);
	SAFE_DELETE_ARRAY(m_pbFeatureNative);
	SAFE_DELETE_ARRAY(m_pbFreePromotions);
	SAFE_DELETE_ARRAY(m_paszEarlyArtDefineTags);
	SAFE_DELETE_ARRAY(m_paszLateArtDefineTags);
	SAFE_DELETE_ARRAY(m_paszMiddleArtDefineTags);
	SAFE_DELETE_ARRAY(m_paszUnitNames);
}

int CvUnitInfo::getAIWeight() const			
{
	return m_iAIWeight;
}

int CvUnitInfo::getProductionCost() const			
{
	return m_iProductionCost;
}

int CvUnitInfo::getHurryCostModifier() const		
{
	return m_iHurryCostModifier;
}

int CvUnitInfo::getMinAreaSize() const
{
	return m_iMinAreaSize;
}

int CvUnitInfo::getMoves() const		
{
	return m_iMoves;
}

int CvUnitInfo::getAirRange() const
{
	return m_iAirRange;
}

int CvUnitInfo::getNukeRange() const
{
	return m_iNukeRange;
}

int CvUnitInfo::getWorkRate() const
{
	return m_iWorkRate;
}

int CvUnitInfo::getBaseDiscover() const
{
	return m_iBaseDiscover;
}

int CvUnitInfo::getDiscoverMultiplier() const
{
	return m_iDiscoverMultiplier;
}

int CvUnitInfo::getBaseHurry() const
{
	return m_iBaseHurry;
}

int CvUnitInfo::getHurryMultiplier() const
{
	return m_iHurryMultiplier;
}

int CvUnitInfo::getBaseTrade() const
{
	return m_iBaseTrade;
}

int CvUnitInfo::getTradeMultiplier() const			
{
	return m_iTradeMultiplier;
}

int CvUnitInfo::getGreatWorkCulture() const		
{
	return m_iGreatWorkCulture;
}

int CvUnitInfo::getCombat() const	
{
	return m_iCombat;
}

void CvUnitInfo::setCombat(int iNum)
{
	m_iCombat = iNum;
}

int CvUnitInfo::getAirCombat() const
{
	return m_iAirCombat;
}

int CvUnitInfo::getAirCombatLimit() const
{
	return m_iAirCombatLimit;
}

int CvUnitInfo::getXPValueAttack() const
{
	return m_iXPValueAttack;
}

int CvUnitInfo::getXPValueDefense() const			
{
	return m_iXPValueDefense;
}

int CvUnitInfo::getFirstStrikes() const
{
	return m_iFirstStrikes;
}

int CvUnitInfo::getChanceFirstStrikes() const	
{
	return m_iChanceFirstStrikes;
}

int CvUnitInfo::getInterceptionProbability() const
{
	return m_iInterceptionProbability;
}

int CvUnitInfo::getEvasionProbability() const	
{
	return m_iEvasionProbability;
}

int CvUnitInfo::getWithdrawalProbability() const
{
	return m_iWithdrawalProbability;
}

int CvUnitInfo::getCollateralDamage() const		
{
	return m_iCollateralDamage;
}

int CvUnitInfo::getCollateralDamageLimit() const
{
	return m_iCollateralDamageLimit;
}

int CvUnitInfo::getCollateralDamageMaxUnits() const
{
	return m_iCollateralDamageMaxUnits;
}

int CvUnitInfo::getCityAttackModifier() const
{
	return m_iCityAttackModifier;
}

int CvUnitInfo::getCityDefenseModifier() const
{
	return m_iCityDefenseModifier;
}

int CvUnitInfo::getAnimalCombatModifier() const
{
	return m_iAnimalCombatModifier;
}

int CvUnitInfo::getHillsAttackModifier() const
{
	return m_iHillsAttackModifier;
}

int CvUnitInfo::getHillsDefenseModifier() const
{
	return m_iHillsDefenseModifier;
}

int CvUnitInfo::getBombRate() const
{
	return m_iBombRate;
}

int CvUnitInfo::getBombardRate() const
{
	return m_iBombardRate;
}

int CvUnitInfo::getSpecialCargo() const
{
	return m_iSpecialCargo;
}

int CvUnitInfo::getDomainCargo() const
{
	return m_iDomainCargo;
}

int CvUnitInfo::getCargoSpace() const
{
	return m_iCargoSpace;
}

int CvUnitInfo::getConscriptionValue() const
{
	return m_iConscriptionValue;
}

int CvUnitInfo::getCultureGarrisonValue() const
{
	return m_iCultureGarrisonValue;
}

int CvUnitInfo::getExtraCost() const
{
	return m_iExtraCost;
}

int CvUnitInfo::getAssetValue() const
{
	return m_iAssetValue;
}

int CvUnitInfo::getPowerValue() const
{
	return m_iPowerValue;
}

int CvUnitInfo::getUnitClassType() const
{
	return m_iUnitClassType;
}

int CvUnitInfo::getSpecialUnitType() const			
{
	return m_iSpecialUnitType;
}

int CvUnitInfo::getUnitCaptureClassType() const
{
	return m_iUnitCaptureClassType;
}

int CvUnitInfo::getUnitCombatType() const			
{
	return m_iUnitCombatType;
}

int CvUnitInfo::getDomainType() const
{
	return m_iDomainType;
}

int CvUnitInfo::getDefaultUnitAIType() const
{
	return m_iDefaultUnitAIType;
}

int CvUnitInfo::getInvisibleType() const
{
	return m_iInvisibleType;
}

int CvUnitInfo::getSeeInvisibleType() const
{
	return m_iSeeInvisibleType;
}

int CvUnitInfo::getAdvisorType() const
{
	return m_iAdvisorType;
}

int CvUnitInfo::getHolyCity() const
{
	return m_iHolyCity;
}

int CvUnitInfo::getReligionType() const
{
	return m_iReligionType;
}

int CvUnitInfo::getStateReligion() const
{
	return m_iStateReligion;
}

int CvUnitInfo::getPrereqReligion() const			
{
	return m_iPrereqReligion;
}

int CvUnitInfo::getPrereqBuilding() const			
{
	return m_iPrereqBuilding;
}

int CvUnitInfo::getPrereqAndTech() const
{
	return m_iPrereqAndTech;
}

int CvUnitInfo::getPrereqAndBonus() const			
{
	return m_iPrereqAndBonus;
}

int CvUnitInfo::getGroupSize() const// the initial number of individuals in the unit group
{
	return m_iGroupSize;
}

int CvUnitInfo::getGroupDefinitions() const// the number of UnitMeshGroups for this unit
{
	return m_iGroupDefinitions;
}

int CvUnitInfo::getMeleeWaveSize() const
{
	return m_iUnitMeleeWaveSize;
}

int CvUnitInfo::getRangedWaveSize() const
{
	return m_iUnitRangedWaveSize;
}

int CvUnitInfo::getNumUnitNames() const
{
	return m_iNumUnitNames;
}

bool CvUnitInfo::isAnimal() const				
{
	return m_bAnimal;
}

bool CvUnitInfo::isFoodProduction() const
{
	return m_bFoodProduction;
}

bool CvUnitInfo::isNoBadGoodies() const	
{
	return m_bNoBadGoodies;
}

bool CvUnitInfo::isOnlyDefensive() const
{
	return m_bOnlyDefensive;
}

bool CvUnitInfo::isNoCapture() const	
{
	return m_bNoCapture;
}

bool CvUnitInfo::isRivalTerritory() const
{
	return m_bRivalTerritory;
}

bool CvUnitInfo::isMilitaryHappiness() const
{
	return m_bMilitaryHappiness;
}

bool CvUnitInfo::isMilitarySupport() const
{
	return m_bMilitarySupport;
}

bool CvUnitInfo::isMilitaryProduction() const
{
	return m_bMilitaryProduction;
}

bool CvUnitInfo::isPillage() const			
{
	return m_bPillage;
}

bool CvUnitInfo::isSabotage() const			
{
	return m_bSabotage;
}

bool CvUnitInfo::isDestroy() const
{
	return m_bDestroy;
}

bool CvUnitInfo::isStealPlans() const
{
	return m_bStealPlans;
}

bool CvUnitInfo::isInvestigate() const	
{
	return m_bInvestigate;
}

bool CvUnitInfo::isCounterSpy() const		
{
	return m_bCounterSpy;
}

bool CvUnitInfo::isFound() const				
{
	return m_bFound;
}

bool CvUnitInfo::isGoldenAge() const		
{
	return m_bGoldenAge;
}

bool CvUnitInfo::isInvisible() const		
{
	return m_bInvisible;
}

void CvUnitInfo::setInvisible(bool bEnable)
{
	m_bInvisible = bEnable;
}

bool CvUnitInfo::isFirstStrikeImmune() const
{
	return m_bFirstStrikeImmune;
}

bool CvUnitInfo::isNoDefensiveBonus() const			
{
	return m_bNoDefensiveBonus;
}

bool CvUnitInfo::isIgnoreBuildingDefense() const		
{
	return m_bIgnoreBuildingDefense;
}

bool CvUnitInfo::isCanMoveImpassable() const
{
	return m_bCanMoveImpassable;
}

bool CvUnitInfo::isFlatMovementCost() const
{
	return m_bFlatMovementCost;
}

bool CvUnitInfo::isIgnoreTerrainCost() const
{
	return m_bIgnoreTerrainCost;
}

bool CvUnitInfo::isNukeImmune() const		
{
	return m_bNukeImmune;
}

bool CvUnitInfo::isPrereqBonuses() const
{
	return m_bPrereqBonuses;
}

bool CvUnitInfo::isPrereqReligion() const
{
	return m_bPrereqReligion;
}

bool CvUnitInfo::isMechUnit() const
{
	return m_bMechanized;
}
bool CvUnitInfo::isRenderBelowWater() const
{
	return m_bRenderBelowWater;
}

float CvUnitInfo::getUnitMaxSpeed() const			
{
	return m_fUnitMaxSpeed;
}

int CvUnitInfo::getCommandType() const
{
	return m_iCommandType;
}

void CvUnitInfo::setCommandType(int iNewType)
{
	m_iCommandType = iNewType;
}


// Arrays

int CvUnitInfo::getPrereqAndTechs(int i) const	
{
	FAssertMsg(i < GC.getDefineINT("NUM_UNIT_AND_TECH_PREREQS"), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piPrereqAndTechs ? m_piPrereqAndTechs[i] : -1;
}

int CvUnitInfo::getPrereqOrBonuses(int i) const
{
	FAssertMsg(i < GC.getNUM_UNIT_PREREQ_OR_BONUSES(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piPrereqOrBonuses ? m_piPrereqOrBonuses[i] : -1;
}

int CvUnitInfo::getProductionTraits(int i) const			
{
	FAssertMsg(i < GC.getNumTraitInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piProductionTraits ? m_piProductionTraits[i] : -1;
}

int CvUnitInfo::getFlavorValue(int i) const				
{
	FAssertMsg(i < GC.getNumFlavorTypes(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piFlavorValue ? m_piFlavorValue[i] : -1;
}

int CvUnitInfo::getTerrainDefenseModifier(int i) const			
{
	FAssertMsg(i < GC.getNumTerrainInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piTerrainDefenseModifier ? m_piTerrainDefenseModifier[i] : -1;
}

int CvUnitInfo::getFeatureDefenseModifier(int i) const			
{
	FAssertMsg(i < GC.getNumFeatureInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piFeatureDefenseModifier ? m_piFeatureDefenseModifier[i] : -1;
}

int CvUnitInfo::getUnitClassAttackModifier(int i) const		
{
	FAssertMsg(i < GC.getNumUnitClassInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piUnitClassAttackModifier ? m_piUnitClassAttackModifier[i] : -1;
}

int CvUnitInfo::getUnitClassDefenseModifier(int i) const
{
	FAssertMsg(i < GC.getNumUnitClassInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piUnitClassDefenseModifier ? m_piUnitClassDefenseModifier[i] : -1;
}

int CvUnitInfo::getUnitCombatModifier(int i) const
{
	FAssertMsg(i < GC.getNumUnitCombatInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piUnitCombatModifier ? m_piUnitCombatModifier[i] : -1;
}

int CvUnitInfo::getUnitCombatCollateralImmune(int i) const
{
	FAssertMsg(i < GC.getNumUnitCombatInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piUnitCombatCollateralImmune ? m_piUnitCombatCollateralImmune[i] : -1;
}

int CvUnitInfo::getDomainModifier(int i) const
{
	FAssertMsg(i < NUM_DOMAIN_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piDomainModifier ? m_piDomainModifier[i] : -1;
}

int CvUnitInfo::getBonusProductionModifier(int i) const
{
	FAssertMsg(i < GC.getNumBonusInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piBonusProductionModifier ? m_piBonusProductionModifier[i] : -1;
}

int CvUnitInfo::getUnitGroupRequired(int i) const		
{
	FAssertMsg(i < getGroupDefinitions(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piUnitGroupRequired ? m_piUnitGroupRequired[i] : NULL;
}

bool CvUnitInfo::getUpgradeUnitClass(int i) const
{
	FAssertMsg(i < GC.getNumUnitClassInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbUpgradeUnitClass ? m_pbUpgradeUnitClass[i] : false;
}

bool CvUnitInfo::getTargetUnitClass(int i) const
{
	FAssertMsg(i < GC.getNumUnitClassInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbTargetUnitClass ? m_pbTargetUnitClass[i] : false;
}

bool CvUnitInfo::getTargetUnitCombat(int i) const
{
	FAssertMsg(i < GC.getNumUnitCombatInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbTargetUnitCombat ? m_pbTargetUnitCombat[i] : false;
}

bool CvUnitInfo::getDefenderUnitClass(int i) const
{
	FAssertMsg(i < GC.getNumUnitClassInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbDefenderUnitClass ? m_pbDefenderUnitClass[i] : false;
}

bool CvUnitInfo::getDefenderUnitCombat(int i) const
{
	FAssertMsg(i < GC.getNumUnitCombatInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbDefenderUnitCombat ? m_pbDefenderUnitCombat[i] : false;
}

bool CvUnitInfo::getUnitAIType(int i) const			
{
	FAssertMsg(i < NUM_UNITAI_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbUnitAIType ? m_pbUnitAIType[i] : false;
}

bool CvUnitInfo::getNotUnitAIType(int i) const			
{
	FAssertMsg(i < NUM_UNITAI_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbNotUnitAIType ? m_pbNotUnitAIType[i] : false;
}

bool CvUnitInfo::getBuilds(int i) const									
{
	FAssertMsg(i < GC.getNumBuildInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbBuilds ? m_pbBuilds[i] : false;
}

bool CvUnitInfo::getReligionSpreads(int i) const
{
	FAssertMsg(i < GC.getNumReligionInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbReligionSpreads ? m_pbReligionSpreads[i] : false;
}

bool CvUnitInfo::getGreatPeoples(int i) const
{
	FAssertMsg(i < GC.getNumSpecialistInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbGreatPeoples ? m_pbGreatPeoples[i] : false;
}

bool CvUnitInfo::getBuildings(int i) const
{
	FAssertMsg(i < GC.getNumBuildingInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbBuildings ? m_pbBuildings[i] : false;
}

bool CvUnitInfo::getForceBuildings(int i) const
{
	FAssertMsg(i < GC.getNumBuildingInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbForceBuildings ? m_pbForceBuildings[i] : false;
}

bool CvUnitInfo::getTerrainImpassable(int i) const		
{
	FAssertMsg(i < GC.getNumTerrainInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbTerrainImpassable ? m_pbTerrainImpassable[i] : false;
}

bool CvUnitInfo::getFeatureImpassable(int i) const		
{
	FAssertMsg(i < GC.getNumFeatureInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbFeatureImpassable ? m_pbFeatureImpassable[i] : false;
}

bool CvUnitInfo::getTerrainNative(int i) const
{
	FAssertMsg(i < GC.getNumTerrainInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbTerrainNative ? m_pbTerrainNative[i] : false;
}

bool CvUnitInfo::getFeatureNative(int i) const
{
	FAssertMsg(i < GC.getNumFeatureInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbFeatureNative ? m_pbFeatureNative[i] : false;
}

bool CvUnitInfo::getFreePromotions(int i) const
{
	FAssertMsg(i < GC.getNumPromotionInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbFreePromotions ? m_pbFreePromotions[i] : false;
}

int CvUnitInfo::getLeaderPromotion() const
{
	return m_iLeaderPromotion;
}

int CvUnitInfo::getLeaderExperience() const
{
	return m_iLeaderExperience;
}

const TCHAR* CvUnitInfo::getEarlyArtDefineTag(int i) const
{
	FAssertMsg(i < getGroupDefinitions(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return (m_paszEarlyArtDefineTags) ? m_paszEarlyArtDefineTags[i] : NULL;
}

void CvUnitInfo::setEarlyArtDefineTag(int i, const TCHAR* szVal)
{
	FAssertMsg(i < getGroupDefinitions(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	m_paszEarlyArtDefineTags[i] = szVal;
}

const TCHAR* CvUnitInfo::getLateArtDefineTag(int i) const
{
	FAssertMsg(i < getGroupDefinitions(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return (m_paszLateArtDefineTags) ? m_paszLateArtDefineTags[i] : NULL;
}

void CvUnitInfo::setLateArtDefineTag(int i, const TCHAR* szVal)
{
	FAssertMsg(i < getGroupDefinitions(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	m_paszLateArtDefineTags[i] = szVal;
}

const TCHAR* CvUnitInfo::getMiddleArtDefineTag(int i) const
{
	FAssertMsg(i < getGroupDefinitions(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return (m_paszMiddleArtDefineTags) ? m_paszMiddleArtDefineTags[i] : NULL;
}

void CvUnitInfo::setMiddleArtDefineTag(int i, const TCHAR* szVal)
{
	FAssertMsg(i < getGroupDefinitions(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	m_paszMiddleArtDefineTags[i] = szVal;
}

const TCHAR* CvUnitInfo::getUnitNames(int i) const
{
	FAssertMsg(i < getNumUnitNames(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return (m_paszUnitNames) ? m_paszUnitNames[i] : NULL;
}


const CvArtInfoUnit* CvUnitInfo::getArtInfo(int i, EraTypes eEra) const
{
	if (!isEmpty(getLateArtDefineTag(i)) && eEra > GC.getNumEraInfos() / 2)
	{
		return ARTFILEMGR.getUnitArtInfo(getLateArtDefineTag(i));
	}
	else if (!isEmpty(getMiddleArtDefineTag(i)) && eEra > GC.getNumEraInfos() / 4)
	{
		return ARTFILEMGR.getUnitArtInfo(getMiddleArtDefineTag(i));
	}
	else
	{
		return ARTFILEMGR.getUnitArtInfo(getEarlyArtDefineTag(i));
	}
}

void CvUnitInfo::read(FDataStreamBase* stream)
{
	CvHotkeyInfo::read(stream);

	uint uiFlag=0;
	stream->Read(&uiFlag);	// flags for expansion

	stream->Read(&m_iAIWeight);
	stream->Read(&m_iProductionCost);
	stream->Read(&m_iHurryCostModifier);
	stream->Read(&m_iMinAreaSize);
	stream->Read(&m_iMoves);
	stream->Read(&m_iAirRange);
	stream->Read(&m_iNukeRange);
	stream->Read(&m_iWorkRate);
	stream->Read(&m_iBaseDiscover);
	stream->Read(&m_iDiscoverMultiplier);
	stream->Read(&m_iBaseHurry);
	stream->Read(&m_iHurryMultiplier);
	stream->Read(&m_iBaseTrade);
	stream->Read(&m_iTradeMultiplier);
	stream->Read(&m_iGreatWorkCulture);
	stream->Read(&m_iCombat);
	stream->Read(&m_iAirCombat);
	stream->Read(&m_iAirCombatLimit);
	stream->Read(&m_iXPValueAttack);
	stream->Read(&m_iXPValueDefense);
	stream->Read(&m_iFirstStrikes);
	stream->Read(&m_iChanceFirstStrikes);
	stream->Read(&m_iInterceptionProbability);
	stream->Read(&m_iEvasionProbability);
	stream->Read(&m_iWithdrawalProbability);
	stream->Read(&m_iCollateralDamage);
	stream->Read(&m_iCollateralDamageLimit);
	stream->Read(&m_iCollateralDamageMaxUnits);
	stream->Read(&m_iCityAttackModifier);
	stream->Read(&m_iCityDefenseModifier);
	stream->Read(&m_iAnimalCombatModifier);
	stream->Read(&m_iHillsAttackModifier);
	stream->Read(&m_iHillsDefenseModifier);
	stream->Read(&m_iBombRate);
	stream->Read(&m_iBombardRate);
	stream->Read(&m_iSpecialCargo);
	stream->Read(&m_iDomainCargo);
	stream->Read(&m_iCargoSpace);
	stream->Read(&m_iConscriptionValue);
	stream->Read(&m_iCultureGarrisonValue);
	stream->Read(&m_iExtraCost);
	stream->Read(&m_iAssetValue);
	stream->Read(&m_iPowerValue);
	stream->Read(&m_iUnitClassType);
	stream->Read(&m_iSpecialUnitType);
	stream->Read(&m_iUnitCaptureClassType);
	stream->Read(&m_iUnitCombatType);
	stream->Read(&m_iDomainType);
	stream->Read(&m_iDefaultUnitAIType);
	stream->Read(&m_iInvisibleType);
	stream->Read(&m_iSeeInvisibleType);
	stream->Read(&m_iAdvisorType);
	stream->Read(&m_iHolyCity);
	stream->Read(&m_iReligionType);
	stream->Read(&m_iStateReligion);
	stream->Read(&m_iPrereqReligion);
	stream->Read(&m_iPrereqBuilding);
	stream->Read(&m_iPrereqAndTech);
	stream->Read(&m_iPrereqAndBonus);
	stream->Read(&m_iGroupSize);
	stream->Read(&m_iGroupDefinitions);
	stream->Read(&m_iUnitMeleeWaveSize);
	stream->Read(&m_iUnitRangedWaveSize);
	stream->Read(&m_iNumUnitNames);
	stream->Read(&m_iCommandType);

	stream->Read(&m_bAnimal);
	stream->Read(&m_bFoodProduction);
	stream->Read(&m_bNoBadGoodies);
	stream->Read(&m_bOnlyDefensive);
	stream->Read(&m_bNoCapture);
	stream->Read(&m_bRivalTerritory);
	stream->Read(&m_bMilitaryHappiness);
	stream->Read(&m_bMilitarySupport);
	stream->Read(&m_bMilitaryProduction);
	stream->Read(&m_bPillage);
	stream->Read(&m_bSabotage);
	stream->Read(&m_bDestroy);
	stream->Read(&m_bStealPlans);
	stream->Read(&m_bInvestigate);
	stream->Read(&m_bCounterSpy);
	stream->Read(&m_bFound);
	stream->Read(&m_bGoldenAge);
	stream->Read(&m_bInvisible);
	stream->Read(&m_bFirstStrikeImmune);
	stream->Read(&m_bNoDefensiveBonus);
	stream->Read(&m_bIgnoreBuildingDefense);
	stream->Read(&m_bCanMoveImpassable);
	stream->Read(&m_bFlatMovementCost);
	stream->Read(&m_bIgnoreTerrainCost);
	stream->Read(&m_bNukeImmune);
	stream->Read(&m_bPrereqBonuses);
	stream->Read(&m_bPrereqReligion);
	stream->Read(&m_bMechanized);
	stream->Read(&m_bRenderBelowWater);

	stream->Read(&m_fUnitMaxSpeed);

	SAFE_DELETE_ARRAY(m_piPrereqAndTechs);
	m_piPrereqAndTechs = new int[GC.getDefineINT("NUM_UNIT_AND_TECH_PREREQS")];
	stream->Read(GC.getDefineINT("NUM_UNIT_AND_TECH_PREREQS"), m_piPrereqAndTechs);

	SAFE_DELETE_ARRAY(m_piPrereqOrBonuses);
	m_piPrereqOrBonuses = new int[GC.getNUM_UNIT_PREREQ_OR_BONUSES()];
	stream->Read(GC.getNUM_UNIT_PREREQ_OR_BONUSES(), m_piPrereqOrBonuses);

	SAFE_DELETE_ARRAY(m_piProductionTraits);
	m_piProductionTraits = new int[GC.getNumTraitInfos()];
	stream->Read(GC.getNumTraitInfos(), m_piProductionTraits);

	SAFE_DELETE_ARRAY(m_piFlavorValue);
	m_piFlavorValue = new int[GC.getNumFlavorTypes()];
	stream->Read(GC.getNumFlavorTypes(), m_piFlavorValue);

	SAFE_DELETE_ARRAY(m_piTerrainDefenseModifier);
	m_piTerrainDefenseModifier = new int[GC.getNumTerrainInfos()];
	stream->Read(GC.getNumTerrainInfos(), m_piTerrainDefenseModifier);

	SAFE_DELETE_ARRAY(m_piFeatureDefenseModifier);
	m_piFeatureDefenseModifier = new int[GC.getNumFeatureInfos()];
	stream->Read(GC.getNumFeatureInfos(), m_piFeatureDefenseModifier);

	SAFE_DELETE_ARRAY(m_piUnitClassAttackModifier);
	m_piUnitClassAttackModifier = new int[GC.getNumUnitClassInfos()];
	stream->Read(GC.getNumUnitClassInfos(), m_piUnitClassAttackModifier);

	SAFE_DELETE_ARRAY(m_piUnitClassDefenseModifier);
	m_piUnitClassDefenseModifier = new int[GC.getNumUnitClassInfos()];
	stream->Read(GC.getNumUnitClassInfos(), m_piUnitClassDefenseModifier);

	SAFE_DELETE_ARRAY(m_piUnitCombatModifier);
	m_piUnitCombatModifier = new int[GC.getNumUnitCombatInfos()];
	stream->Read(GC.getNumUnitCombatInfos(), m_piUnitCombatModifier);

	SAFE_DELETE_ARRAY(m_piUnitCombatCollateralImmune);
	m_piUnitCombatCollateralImmune = new int[GC.getNumUnitCombatInfos()];
	stream->Read(GC.getNumUnitCombatInfos(), m_piUnitCombatCollateralImmune);

	SAFE_DELETE_ARRAY(m_piDomainModifier);
	m_piDomainModifier = new int[NUM_DOMAIN_TYPES];
	stream->Read(NUM_DOMAIN_TYPES, m_piDomainModifier);

	SAFE_DELETE_ARRAY(m_piBonusProductionModifier);
	m_piBonusProductionModifier = new int[GC.getNumBonusInfos()];
	stream->Read(GC.getNumBonusInfos(), m_piBonusProductionModifier);

	SAFE_DELETE_ARRAY(m_piUnitGroupRequired);
	m_piUnitGroupRequired = new int[m_iGroupDefinitions];
	stream->Read(m_iGroupDefinitions, m_piUnitGroupRequired);

	SAFE_DELETE_ARRAY(m_pbUpgradeUnitClass);
	m_pbUpgradeUnitClass = new bool[GC.getNumUnitClassInfos()];
	stream->Read(GC.getNumUnitClassInfos(), m_pbUpgradeUnitClass);

	SAFE_DELETE_ARRAY(m_pbTargetUnitClass);
	m_pbTargetUnitClass = new bool[GC.getNumUnitClassInfos()];
	stream->Read(GC.getNumUnitClassInfos(), m_pbTargetUnitClass);

	SAFE_DELETE_ARRAY(m_pbTargetUnitCombat);
	m_pbTargetUnitCombat = new bool[GC.getNumUnitCombatInfos()];
	stream->Read(GC.getNumUnitCombatInfos(), m_pbTargetUnitCombat);

	SAFE_DELETE_ARRAY(m_pbDefenderUnitClass);
	m_pbDefenderUnitClass = new bool[GC.getNumUnitClassInfos()];
	stream->Read(GC.getNumUnitClassInfos(), m_pbDefenderUnitClass);

	SAFE_DELETE_ARRAY(m_pbDefenderUnitCombat);
	m_pbDefenderUnitCombat = new bool[GC.getNumUnitCombatInfos()];
	stream->Read(GC.getNumUnitCombatInfos(), m_pbDefenderUnitCombat);

	SAFE_DELETE_ARRAY(m_pbUnitAIType);
	m_pbUnitAIType = new bool[NUM_UNITAI_TYPES];
	stream->Read(NUM_UNITAI_TYPES, m_pbUnitAIType);

	SAFE_DELETE_ARRAY(m_pbNotUnitAIType);
	m_pbNotUnitAIType = new bool[NUM_UNITAI_TYPES];
	stream->Read(NUM_UNITAI_TYPES, m_pbNotUnitAIType);

	SAFE_DELETE_ARRAY(m_pbBuilds);
	m_pbBuilds = new bool[GC.getNumBuildInfos()];
	stream->Read(GC.getNumBuildInfos(), m_pbBuilds);

	SAFE_DELETE_ARRAY(m_pbReligionSpreads);
	m_pbReligionSpreads = new bool[GC.getNumReligionInfos()];
	stream->Read(GC.getNumReligionInfos(), m_pbReligionSpreads);

	SAFE_DELETE_ARRAY(m_pbGreatPeoples);
	m_pbGreatPeoples = new bool[GC.getNumSpecialistInfos()];
	stream->Read(GC.getNumSpecialistInfos(), m_pbGreatPeoples);

	SAFE_DELETE_ARRAY(m_pbBuildings);
	m_pbBuildings = new bool[GC.getNumBuildingInfos()];
	stream->Read(GC.getNumBuildingInfos(), m_pbBuildings);

	SAFE_DELETE_ARRAY(m_pbForceBuildings);
	m_pbForceBuildings = new bool[GC.getNumBuildingInfos()];
	stream->Read(GC.getNumBuildingInfos(), m_pbForceBuildings);

	SAFE_DELETE_ARRAY(m_pbTerrainNative);
	m_pbTerrainNative = new bool[GC.getNumTerrainInfos()];
	stream->Read(GC.getNumTerrainInfos(), m_pbTerrainNative);

	SAFE_DELETE_ARRAY(m_pbFeatureNative);
	m_pbFeatureNative = new bool[GC.getNumFeatureInfos()];
	stream->Read(GC.getNumFeatureInfos(), m_pbFeatureNative);

	SAFE_DELETE_ARRAY(m_pbTerrainImpassable);
	m_pbTerrainImpassable = new bool[GC.getNumTerrainInfos()];
	stream->Read(GC.getNumTerrainInfos(), m_pbTerrainImpassable);

	SAFE_DELETE_ARRAY(m_pbFeatureImpassable);
	m_pbFeatureImpassable = new bool[GC.getNumFeatureInfos()];
	stream->Read(GC.getNumFeatureInfos(), m_pbFeatureImpassable);

	SAFE_DELETE_ARRAY(m_pbFreePromotions);
	m_pbFreePromotions = new bool[GC.getNumPromotionInfos()];
	stream->Read(GC.getNumPromotionInfos(), m_pbFreePromotions);

	stream->Read(&m_iLeaderPromotion);
	stream->Read(&m_iLeaderExperience);

	SAFE_DELETE_ARRAY(m_paszEarlyArtDefineTags);
	m_paszEarlyArtDefineTags = new CvString [m_iGroupDefinitions];
	stream->ReadString(m_iGroupDefinitions, m_paszEarlyArtDefineTags);

	SAFE_DELETE_ARRAY(m_paszLateArtDefineTags);
	m_paszLateArtDefineTags = new CvString [m_iGroupDefinitions];
	stream->ReadString(m_iGroupDefinitions, m_paszLateArtDefineTags);

	SAFE_DELETE_ARRAY(m_paszMiddleArtDefineTags);
	m_paszMiddleArtDefineTags = new CvString [m_iGroupDefinitions];
	stream->ReadString(m_iGroupDefinitions, m_paszMiddleArtDefineTags);

	SAFE_DELETE_ARRAY(m_paszUnitNames);
	m_paszUnitNames = new CvString[m_iNumUnitNames];
	stream->ReadString(m_iNumUnitNames, m_paszUnitNames);
}

void CvUnitInfo::write(FDataStreamBase* stream)
{
	CvHotkeyInfo::write(stream);

	uint uiFlag=0;
	stream->Write(uiFlag);		// flag for expansion

	stream->Write(m_iAIWeight);
	stream->Write(m_iProductionCost);
	stream->Write(m_iHurryCostModifier);
	stream->Write(m_iMinAreaSize);
	stream->Write(m_iMoves);
	stream->Write(m_iAirRange);
	stream->Write(m_iNukeRange);
	stream->Write(m_iWorkRate);
	stream->Write(m_iBaseDiscover);
	stream->Write(m_iDiscoverMultiplier);
	stream->Write(m_iBaseHurry);
	stream->Write(m_iHurryMultiplier);
	stream->Write(m_iBaseTrade);
	stream->Write(m_iTradeMultiplier);
	stream->Write(m_iGreatWorkCulture);
	stream->Write(m_iCombat);
	stream->Write(m_iAirCombat);
	stream->Write(m_iAirCombatLimit);
	stream->Write(m_iXPValueAttack);
	stream->Write(m_iXPValueDefense);
	stream->Write(m_iFirstStrikes);
	stream->Write(m_iChanceFirstStrikes);
	stream->Write(m_iInterceptionProbability);
	stream->Write(m_iEvasionProbability);
	stream->Write(m_iWithdrawalProbability);
	stream->Write(m_iCollateralDamage);
	stream->Write(m_iCollateralDamageLimit);
	stream->Write(m_iCollateralDamageMaxUnits);
	stream->Write(m_iCityAttackModifier);
	stream->Write(m_iCityDefenseModifier);
	stream->Write(m_iAnimalCombatModifier);
	stream->Write(m_iHillsAttackModifier);
	stream->Write(m_iHillsDefenseModifier);
	stream->Write(m_iBombRate);
	stream->Write(m_iBombardRate);
	stream->Write(m_iSpecialCargo);
	stream->Write(m_iDomainCargo);
	stream->Write(m_iCargoSpace);
	stream->Write(m_iConscriptionValue);
	stream->Write(m_iCultureGarrisonValue);
	stream->Write(m_iExtraCost);
	stream->Write(m_iAssetValue);
	stream->Write(m_iPowerValue);
	stream->Write(m_iUnitClassType);
	stream->Write(m_iSpecialUnitType);
	stream->Write(m_iUnitCaptureClassType);
	stream->Write(m_iUnitCombatType);
	stream->Write(m_iDomainType);
	stream->Write(m_iDefaultUnitAIType);
	stream->Write(m_iInvisibleType);
	stream->Write(m_iSeeInvisibleType);
	stream->Write(m_iAdvisorType);
	stream->Write(m_iHolyCity);
	stream->Write(m_iReligionType);
	stream->Write(m_iStateReligion);
	stream->Write(m_iPrereqReligion);
	stream->Write(m_iPrereqBuilding);
	stream->Write(m_iPrereqAndTech);
	stream->Write(m_iPrereqAndBonus);
	stream->Write(m_iGroupSize);
	stream->Write(m_iGroupDefinitions);
	stream->Write(m_iUnitMeleeWaveSize);
	stream->Write(m_iUnitRangedWaveSize);
	stream->Write(m_iNumUnitNames);
	stream->Write(m_iCommandType);

	stream->Write(m_bAnimal);
	stream->Write(m_bFoodProduction);
	stream->Write(m_bNoBadGoodies);
	stream->Write(m_bOnlyDefensive);
	stream->Write(m_bNoCapture);
	stream->Write(m_bRivalTerritory);
	stream->Write(m_bMilitaryHappiness);
	stream->Write(m_bMilitarySupport);
	stream->Write(m_bMilitaryProduction);
	stream->Write(m_bPillage);
	stream->Write(m_bSabotage);
	stream->Write(m_bDestroy);
	stream->Write(m_bStealPlans);
	stream->Write(m_bInvestigate);
	stream->Write(m_bCounterSpy);
	stream->Write(m_bFound);
	stream->Write(m_bGoldenAge);
	stream->Write(m_bInvisible);
	stream->Write(m_bFirstStrikeImmune);
	stream->Write(m_bNoDefensiveBonus);
	stream->Write(m_bIgnoreBuildingDefense);
	stream->Write(m_bCanMoveImpassable);
	stream->Write(m_bFlatMovementCost);
	stream->Write(m_bIgnoreTerrainCost);
	stream->Write(m_bNukeImmune);
	stream->Write(m_bPrereqBonuses);
	stream->Write(m_bPrereqReligion);
	stream->Write(m_bMechanized);
	stream->Write(m_bRenderBelowWater);

	stream->Write(m_fUnitMaxSpeed);

	stream->Write(GC.getDefineINT("NUM_UNIT_AND_TECH_PREREQS"), m_piPrereqAndTechs);
	stream->Write(GC.getNUM_UNIT_PREREQ_OR_BONUSES(), m_piPrereqOrBonuses);
	stream->Write(GC.getNumTraitInfos(), m_piProductionTraits);
	stream->Write(GC.getNumFlavorTypes(), m_piFlavorValue);
	stream->Write(GC.getNumTerrainInfos(), m_piTerrainDefenseModifier);
	stream->Write(GC.getNumFeatureInfos(), m_piFeatureDefenseModifier);
	stream->Write(GC.getNumUnitClassInfos(), m_piUnitClassAttackModifier);
	stream->Write(GC.getNumUnitClassInfos(), m_piUnitClassDefenseModifier);
	stream->Write(GC.getNumUnitCombatInfos(), m_piUnitCombatModifier);
	stream->Write(GC.getNumUnitCombatInfos(), m_piUnitCombatCollateralImmune);
	stream->Write(NUM_DOMAIN_TYPES, m_piDomainModifier);
	stream->Write(GC.getNumBonusInfos(), m_piBonusProductionModifier);
	stream->Write(m_iGroupDefinitions, m_piUnitGroupRequired);

	stream->Write(GC.getNumUnitClassInfos(), m_pbUpgradeUnitClass);
	stream->Write(GC.getNumUnitClassInfos(), m_pbTargetUnitClass);
	stream->Write(GC.getNumUnitCombatInfos(), m_pbTargetUnitCombat);
	stream->Write(GC.getNumUnitClassInfos(), m_pbDefenderUnitClass);
	stream->Write(GC.getNumUnitCombatInfos(), m_pbDefenderUnitCombat);
	stream->Write(NUM_UNITAI_TYPES, m_pbUnitAIType);
	stream->Write(NUM_UNITAI_TYPES, m_pbNotUnitAIType);
	stream->Write(GC.getNumBuildInfos(), m_pbBuilds);
	stream->Write(GC.getNumReligionInfos(), m_pbReligionSpreads);
	stream->Write(GC.getNumSpecialistInfos(), m_pbGreatPeoples);
	stream->Write(GC.getNumBuildingInfos(), m_pbBuildings);
	stream->Write(GC.getNumBuildingInfos(), m_pbForceBuildings);
	stream->Write(GC.getNumTerrainInfos(), m_pbTerrainNative);
	stream->Write(GC.getNumFeatureInfos(), m_pbFeatureNative);
	stream->Write(GC.getNumTerrainInfos(), m_pbTerrainImpassable);
	stream->Write(GC.getNumFeatureInfos(), m_pbFeatureImpassable);
	stream->Write(GC.getNumPromotionInfos(), m_pbFreePromotions);
	stream->Write(m_iLeaderPromotion);
	stream->Write(m_iLeaderExperience);

	stream->WriteString(m_iGroupDefinitions, m_paszEarlyArtDefineTags);
	stream->WriteString(m_iGroupDefinitions, m_paszLateArtDefineTags);
	stream->WriteString(m_iGroupDefinitions, m_paszMiddleArtDefineTags);
	stream->WriteString(m_iNumUnitNames, m_paszUnitNames);
}

//
// read from xml
//
bool CvUnitInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvHotkeyInfo::read(pXML))
	{
		return false;
	}

	int j=0;				//loop counter
	int k=0;				//loop counter
	int iNumSibs=0;				// the number of siblings the current xml node has
	int iIndexVal;

	pXML->GetChildXmlValByName(szTextVal, "Class");
	m_iUnitClassType = pXML->FindInInfoClass(szTextVal, GC.getUnitClassInfo(), sizeof(GC.getUnitClassInfo((UnitClassTypes)0)), GC.getNumUnitClassInfos());

	pXML->GetChildXmlValByName(szTextVal, "Special");
	m_iSpecialUnitType = pXML->FindInInfoClass(szTextVal, GC.getSpecialUnitInfo(), sizeof(GC.getSpecialUnitInfo((SpecialUnitTypes)0)), GC.getNumSpecialUnitInfos());

	pXML->GetChildXmlValByName(szTextVal, "Capture");
	m_iUnitCaptureClassType = pXML->FindInInfoClass(szTextVal, GC.getUnitClassInfo(), sizeof(GC.getUnitClassInfo((UnitClassTypes)0)), GC.getNumUnitClassInfos());

	pXML->GetChildXmlValByName(szTextVal, "Combat");
	m_iUnitCombatType = pXML->FindInInfoClass(szTextVal, GC.getUnitCombatInfo(), sizeof(GC.getUnitCombatInfo((UnitCombatTypes)0)), GC.getNumUnitCombatInfos());

	pXML->GetChildXmlValByName(szTextVal, "Domain");
	m_iDomainType = pXML->FindInInfoClass(szTextVal, GC.getDomainInfo(), sizeof(GC.getDomainInfo((DomainTypes)0)), NUM_DOMAIN_TYPES);

	pXML->GetChildXmlValByName(szTextVal, "DefaultUnitAI");
	m_iDefaultUnitAIType = pXML->FindInInfoClass(szTextVal, GC.getUnitAIInfo(), sizeof(GC.getUnitAIInfo((UnitAITypes)0)), NUM_UNITAI_TYPES);

	pXML->GetChildXmlValByName(szTextVal, "Invisible");
	m_iInvisibleType = pXML->FindInInfoClass(szTextVal, GC.getInvisibleInfo(), sizeof(GC.getInvisibleInfo((InvisibleTypes)0)), GC.getNumInvisibleInfos());

	pXML->GetChildXmlValByName(szTextVal, "SeeInvisible");
	m_iSeeInvisibleType = pXML->FindInInfoClass(szTextVal, GC.getInvisibleInfo(), sizeof(GC.getInvisibleInfo((InvisibleTypes)0)), GC.getNumInvisibleInfos());

	pXML->GetChildXmlValByName(szTextVal, "Advisor");
	m_iAdvisorType = pXML->FindInInfoClass(szTextVal, GC.getAdvisorInfo(), sizeof(GC.getAdvisorInfo((AdvisorTypes)0)), GC.getNumAdvisorInfos());

	pXML->GetChildXmlValByName(&m_bAnimal, "bAnimal");
	pXML->GetChildXmlValByName(&m_bFoodProduction, "bFood");
	pXML->GetChildXmlValByName(&m_bNoBadGoodies, "bNoBadGoodies");
	pXML->GetChildXmlValByName(&m_bOnlyDefensive, "bOnlyDefensive");
	pXML->GetChildXmlValByName(&m_bNoCapture, "bNoCapture");
	pXML->GetChildXmlValByName(&m_bRivalTerritory, "bRivalTerritory");
	pXML->GetChildXmlValByName(&m_bMilitaryHappiness, "bMilitaryHappiness");
	pXML->GetChildXmlValByName(&m_bMilitarySupport, "bMilitarySupport");
	pXML->GetChildXmlValByName(&m_bMilitaryProduction, "bMilitaryProduction");
	pXML->GetChildXmlValByName(&m_bPillage, "bPillage");
	pXML->GetChildXmlValByName(&m_bSabotage, "bSabotage");
	pXML->GetChildXmlValByName(&m_bDestroy, "bDestroy");
	pXML->GetChildXmlValByName(&m_bStealPlans, "bStealPlans");
	pXML->GetChildXmlValByName(&m_bInvestigate, "bInvestigate");
	pXML->GetChildXmlValByName(&m_bCounterSpy, "bCounterSpy");
	pXML->GetChildXmlValByName(&m_bFound, "bFound");
	pXML->GetChildXmlValByName(&m_bGoldenAge, "bGoldenAge");
	pXML->GetChildXmlValByName(&m_bInvisible, "bInvisible");
	pXML->GetChildXmlValByName(&m_bFirstStrikeImmune, "bFirstStrikeImmune");
	pXML->GetChildXmlValByName(&m_bNoDefensiveBonus, "bNoDefensiveBonus");
	pXML->GetChildXmlValByName(&m_bIgnoreBuildingDefense, "bIgnoreBuildingDefense");
	pXML->GetChildXmlValByName(&m_bCanMoveImpassable, "bCanMoveImpassable");
	pXML->GetChildXmlValByName(&m_bFlatMovementCost, "bFlatMovementCost");
	pXML->GetChildXmlValByName(&m_bIgnoreTerrainCost, "bIgnoreTerrainCost");
	pXML->GetChildXmlValByName(&m_bNukeImmune, "bNukeImmune");
	pXML->GetChildXmlValByName(&m_bPrereqBonuses, "bPrereqBonuses");
	pXML->GetChildXmlValByName(&m_bPrereqReligion, "bPrereqReligion");
	pXML->GetChildXmlValByName(&m_bMechanized,"bMechanized",false);
	pXML->GetChildXmlValByName(&m_bRenderBelowWater,"bRenderBelowWater",false);

	pXML->SetVariableListTagPair(&m_pbUpgradeUnitClass, "UnitClassUpgrades", GC.getUnitClassInfo(), sizeof(GC.getUnitClassInfo((UnitClassTypes)0)), GC.getNumUnitClassInfos());
	pXML->SetVariableListTagPair(&m_pbTargetUnitClass, "UnitClassTargets", GC.getUnitClassInfo(), sizeof(GC.getUnitClassInfo((UnitClassTypes)0)), GC.getNumUnitClassInfos());
	pXML->SetVariableListTagPair(&m_pbTargetUnitCombat, "UnitCombatTargets", GC.getUnitCombatInfo(), sizeof(GC.getUnitCombatInfo((UnitCombatTypes)0)), GC.getNumUnitCombatInfos());
	pXML->SetVariableListTagPair(&m_pbDefenderUnitClass, "UnitClassDefenders", GC.getUnitClassInfo(), sizeof(GC.getUnitClassInfo((UnitClassTypes)0)), GC.getNumUnitClassInfos());
	pXML->SetVariableListTagPair(&m_pbDefenderUnitCombat, "UnitCombatDefenders", GC.getUnitCombatInfo(), sizeof(GC.getUnitCombatInfo((UnitCombatTypes)0)), GC.getNumUnitCombatInfos());
	pXML->SetVariableListTagPair(&m_pbUnitAIType, "UnitAIs", GC.getUnitAIInfo(), sizeof(GC.getUnitAIInfo((UnitAITypes)0)), NUM_UNITAI_TYPES);
	pXML->SetVariableListTagPair(&m_pbNotUnitAIType, "NotUnitAIs", GC.getUnitAIInfo(), sizeof(GC.getUnitAIInfo((UnitAITypes)0)), NUM_UNITAI_TYPES);

	pXML->SetVariableListTagPair(&m_pbBuilds, "Builds", GC.getBuildInfo(), sizeof(GC.getBuildInfo((BuildTypes)0)), GC.getNumBuildInfos());

	pXML->SetVariableListTagPair(&m_pbReligionSpreads, "ReligionSpreads", GC.getReligionInfo(), sizeof(GC.getReligionInfo((ReligionTypes)0)), GC.getNumReligionInfos());

	pXML->SetVariableListTagPair(&m_pbGreatPeoples, "GreatPeoples", GC.getSpecialistInfo(), sizeof(GC.getSpecialistInfo((SpecialistTypes)0)), GC.getNumSpecialistInfos());

	pXML->SetVariableListTagPair(&m_pbBuildings, "Buildings", GC.getBuildingInfo(), sizeof(GC.getBuildingInfo((BuildingTypes)0)), GC.getNumBuildingInfos());
	pXML->SetVariableListTagPair(&m_pbForceBuildings, "ForceBuildings", GC.getBuildingInfo(), sizeof(GC.getBuildingInfo((BuildingTypes)0)), GC.getNumBuildingInfos());

	pXML->GetChildXmlValByName(szTextVal, "HolyCity");
	m_iHolyCity = pXML->FindInInfoClass(szTextVal, GC.getReligionInfo(), sizeof(GC.getReligionInfo((ReligionTypes)0)), GC.getNumReligionInfos());

	pXML->GetChildXmlValByName(szTextVal, "ReligionType");
	m_iReligionType = pXML->FindInInfoClass(szTextVal, GC.getReligionInfo(), sizeof(GC.getReligionInfo((ReligionTypes)0)), GC.getNumReligionInfos());

	pXML->GetChildXmlValByName(szTextVal, "StateReligion");
	m_iStateReligion = pXML->FindInInfoClass(szTextVal, GC.getReligionInfo(), sizeof(GC.getReligionInfo((ReligionTypes)0)), GC.getNumReligionInfos());

	pXML->GetChildXmlValByName(szTextVal, "PrereqReligion");
	m_iPrereqReligion = pXML->FindInInfoClass(szTextVal, GC.getReligionInfo(), sizeof(GC.getReligionInfo((ReligionTypes)0)), GC.getNumReligionInfos());

	pXML->GetChildXmlValByName(szTextVal, "PrereqBuilding");
	m_iPrereqBuilding = pXML->FindInInfoClass(szTextVal, GC.getBuildingInfo(), sizeof(GC.getBuildingInfo((BuildingTypes)0)), GC.getNumBuildingInfos());

	pXML->GetChildXmlValByName(szTextVal, "PrereqTech");
	m_iPrereqAndTech = pXML->FindInInfoClass(szTextVal, GC.getTechInfo(), sizeof(GC.getTechInfo((TechTypes)0)), GC.getNumTechInfos());

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"TechTypes"))
	{
		if (pXML->SkipToNextVal())
		{
			iNumSibs = gDLL->getXMLIFace()->GetNumChildren(pXML->GetXML());
			FAssertMsg((0 < GC.getDefineINT("NUM_UNIT_AND_TECH_PREREQS")) ,"Allocating zero or less memory in SetGlobalUnitInfo");
			pXML->InitList(&m_piPrereqAndTechs, GC.getDefineINT("NUM_UNIT_AND_TECH_PREREQS"), -1);

			if (0 < iNumSibs)
			{
				if (pXML->GetChildXmlVal(szTextVal))
				{
					FAssertMsg((iNumSibs <= GC.getDefineINT("NUM_UNIT_AND_TECH_PREREQS")) ,"There are more siblings than memory allocated for them in SetGlobalUnitInfo");
					for (j=0;j<iNumSibs;j++)
					{
						m_piPrereqAndTechs[j] = pXML->FindInInfoClass(szTextVal, GC.getTechInfo(), sizeof(GC.getTechInfo((TechTypes)0)), GC.getNumTechInfos());
						if (!pXML->GetNextXmlVal(szTextVal))
						{
							break;
						}
					}

					gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
				}
			}
		}

		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	pXML->GetChildXmlValByName(szTextVal, "BonusType");
	m_iPrereqAndBonus = pXML->FindInInfoClass(szTextVal, GC.getBonusInfo(), sizeof(GC.getBonusInfo((BonusTypes)0)), GC.getNumBonusInfos());

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"PrereqBonuses"))
	{
		if (pXML->SkipToNextVal())
		{
			iNumSibs = gDLL->getXMLIFace()->GetNumChildren(pXML->GetXML());
			FAssertMsg((0 < GC.getNUM_UNIT_PREREQ_OR_BONUSES()),"Allocating zero or less memory in SetGlobalUnitInfo");
			pXML->InitList(&m_piPrereqOrBonuses, GC.getNUM_UNIT_PREREQ_OR_BONUSES(), -1);

			if (0 < iNumSibs)
			{
				if (pXML->GetChildXmlVal(szTextVal))
				{
					FAssertMsg((iNumSibs <= GC.getNUM_UNIT_PREREQ_OR_BONUSES()) , "There are more siblings than memory allocated for them in SetGlobalUnitInfo");
					for (j=0;j<iNumSibs;j++)
					{
						m_piPrereqOrBonuses[j] = pXML->FindInInfoClass(szTextVal, GC.getBonusInfo(), sizeof(GC.getBonusInfo((BonusTypes)0)), GC.getNumBonusInfos());
						if (!pXML->GetNextXmlVal(szTextVal))
						{
							break;
						}
					}

					gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
				}
			}
		}

		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	pXML->SetVariableListTagPair(&m_piProductionTraits, "ProductionTraits", GC.getTraitInfo(), sizeof(GC.getTraitInfo((TraitTypes)0)), GC.getNumTraitInfos());

	pXML->SetVariableListTagPair(&m_piFlavorValue, "Flavors", GC.getFlavorTypes(), GC.getNumFlavorTypes());

	pXML->GetChildXmlValByName(&m_iAIWeight, "iAIWeight");
	pXML->GetChildXmlValByName(&m_iProductionCost, "iCost");
	pXML->GetChildXmlValByName(&m_iHurryCostModifier, "iHurryCostModifier");
	pXML->GetChildXmlValByName(&m_iMinAreaSize, "iMinAreaSize");
	pXML->GetChildXmlValByName(&m_iMoves, "iMoves");
	pXML->GetChildXmlValByName(&m_iAirRange, "iAirRange");
	pXML->GetChildXmlValByName(&m_iNukeRange, "iNukeRange");
	pXML->GetChildXmlValByName(&m_iWorkRate, "iWorkRate");
	pXML->GetChildXmlValByName(&m_iBaseDiscover, "iBaseDiscover");
	pXML->GetChildXmlValByName(&m_iDiscoverMultiplier, "iDiscoverMultiplier");
	pXML->GetChildXmlValByName(&m_iBaseHurry, "iBaseHurry");
	pXML->GetChildXmlValByName(&m_iHurryMultiplier, "iHurryMultiplier");
	pXML->GetChildXmlValByName(&m_iBaseTrade, "iBaseTrade");
	pXML->GetChildXmlValByName(&m_iTradeMultiplier, "iTradeMultiplier");
	pXML->GetChildXmlValByName(&m_iGreatWorkCulture, "iGreatWorkCulture");

	pXML->SetVariableListTagPair(&m_pbTerrainImpassable, "TerrainImpassables", GC.getTerrainInfo(), sizeof(GC.getTerrainInfo((TerrainTypes)0)), GC.getNumTerrainInfos(), false);
	pXML->SetVariableListTagPair(&m_pbFeatureImpassable, "FeatureImpassables", GC.getFeatureInfo(), sizeof(GC.getFeatureInfo((FeatureTypes)0)), GC.getNumFeatureInfos(), false);

	pXML->GetChildXmlValByName(&m_iCombat, "iCombat");
	pXML->GetChildXmlValByName(&m_iAirCombat, "iAirCombat");
	pXML->GetChildXmlValByName(&m_iAirCombatLimit, "iAirCombatLimit");
	pXML->GetChildXmlValByName(&m_iXPValueAttack, "iXPValueAttack");
	pXML->GetChildXmlValByName(&m_iXPValueDefense, "iXPValueDefense");
	pXML->GetChildXmlValByName(&m_iFirstStrikes, "iFirstStrikes");
	pXML->GetChildXmlValByName(&m_iChanceFirstStrikes, "iChanceFirstStrikes");
	pXML->GetChildXmlValByName(&m_iInterceptionProbability, "iInterceptionProbability");
	pXML->GetChildXmlValByName(&m_iEvasionProbability, "iEvasionProbability");
	pXML->GetChildXmlValByName(&m_iWithdrawalProbability, "iWithdrawalProb");
	pXML->GetChildXmlValByName(&m_iCollateralDamage, "iCollateralDamage");
	pXML->GetChildXmlValByName(&m_iCollateralDamageLimit, "iCollateralDamageLimit");
	pXML->GetChildXmlValByName(&m_iCollateralDamageMaxUnits, "iCollateralDamageMaxUnits");
	pXML->GetChildXmlValByName(&m_iCityAttackModifier, "iCityAttack");
	pXML->GetChildXmlValByName(&m_iCityDefenseModifier, "iCityDefense");
	pXML->GetChildXmlValByName(&m_iAnimalCombatModifier, "iAnimalCombat");
	pXML->GetChildXmlValByName(&m_iHillsAttackModifier, "iHillsAttack");
	pXML->GetChildXmlValByName(&m_iHillsDefenseModifier, "iHillsDefense");

	pXML->SetVariableListTagPair(&m_pbTerrainNative, "TerrainNatives", GC.getTerrainInfo(), sizeof(GC.getTerrainInfo((TerrainTypes)0)), GC.getNumTerrainInfos());
	pXML->SetVariableListTagPair(&m_pbFeatureNative, "FeatureNatives", GC.getFeatureInfo(), sizeof(GC.getFeatureInfo((FeatureTypes)0)), GC.getNumFeatureInfos());

	pXML->SetVariableListTagPair(&m_piTerrainDefenseModifier, "TerrainDefenses", GC.getTerrainInfo(), sizeof(GC.getTerrainInfo((TerrainTypes)0)), GC.getNumTerrainInfos());
	pXML->SetVariableListTagPair(&m_piFeatureDefenseModifier, "FeatureDefenses", GC.getFeatureInfo(), sizeof(GC.getFeatureInfo((FeatureTypes)0)), GC.getNumFeatureInfos());

	pXML->SetVariableListTagPair(&m_piUnitClassAttackModifier, "UnitClassAttackMods", GC.getUnitClassInfo(), sizeof(GC.getUnitClassInfo((UnitClassTypes)0)), GC.getNumUnitClassInfos());
	pXML->SetVariableListTagPair(&m_piUnitClassDefenseModifier, "UnitClassDefenseMods", GC.getUnitClassInfo(), sizeof(GC.getUnitClassInfo((UnitClassTypes)0)), GC.getNumUnitClassInfos());

	pXML->SetVariableListTagPair(&m_piUnitCombatModifier, "UnitCombatMods", GC.getUnitCombatInfo(), sizeof(GC.getUnitCombatInfo((UnitCombatTypes)0)), GC.getNumUnitCombatInfos());
	pXML->SetVariableListTagPair(&m_piUnitCombatCollateralImmune, "UnitCombatCollateralImmunes", GC.getUnitCombatInfo(), sizeof(GC.getUnitCombatInfo((UnitCombatTypes)0)), GC.getNumUnitCombatInfos());
	pXML->SetVariableListTagPair(&m_piDomainModifier, "DomainMods", GC.getDomainInfo(), sizeof(GC.getDomainInfo((DomainTypes)0)), NUM_DOMAIN_TYPES);

	pXML->SetVariableListTagPair(&m_piBonusProductionModifier, "BonusProductionModifiers", GC.getBonusInfo(), sizeof(GC.getBonusInfo((BonusTypes)0)), GC.getNumBonusInfos());

	pXML->GetChildXmlValByName(&m_iBombRate, "iBombRate");
	pXML->GetChildXmlValByName(&m_iBombardRate, "iBombardRate");

	pXML->GetChildXmlValByName(szTextVal, "SpecialCargo");
	m_iSpecialCargo = pXML->FindInInfoClass(szTextVal, GC.getSpecialUnitInfo(), sizeof(GC.getSpecialUnitInfo((SpecialUnitTypes)0)), GC.getNumSpecialUnitInfos());

	pXML->GetChildXmlValByName(szTextVal, "DomainCargo");
	m_iDomainCargo = pXML->FindInInfoClass(szTextVal, GC.getDomainInfo(), sizeof(GC.getDomainInfo((DomainTypes)0)), NUM_DOMAIN_TYPES);

	pXML->GetChildXmlValByName(&m_iCargoSpace, "iCargo");
	pXML->GetChildXmlValByName(&m_iConscriptionValue, "iConscription");
	pXML->GetChildXmlValByName(&m_iCultureGarrisonValue, "iCultureGarrison");
	pXML->GetChildXmlValByName(&m_iExtraCost, "iExtraCost");
	pXML->GetChildXmlValByName(&m_iAssetValue, "iAsset");
	pXML->GetChildXmlValByName(&m_iPowerValue, "iPower");

	// Read the mesh groups elements
	if ( gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"UnitMeshGroups") )
	{
		pXML->GetChildXmlValByName( &m_iGroupSize, "iGroupSize");
		m_iGroupDefinitions = iIndexVal = gDLL->getXMLIFace()->NumOfChildrenByTagName(pXML->GetXML(), "UnitMeshGroup");
		m_piUnitGroupRequired = new int[ iIndexVal ];
		pXML->GetChildXmlValByName( &m_iUnitMeleeWaveSize, "iMeleeWaveSize" );
		pXML->GetChildXmlValByName( &m_iUnitRangedWaveSize, "iRangedWaveSize" );
		pXML->GetChildXmlValByName( &m_fUnitMaxSpeed, "fMaxSpeed");
		m_paszEarlyArtDefineTags = new CvString[ iIndexVal ];
		m_paszLateArtDefineTags = new CvString[ iIndexVal ];
		m_paszMiddleArtDefineTags = new CvString[ iIndexVal ];

		gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(), "UnitMeshGroup");
		for ( k = 0; k < iIndexVal; k++ )
		{
			pXML->GetChildXmlValByName( &m_piUnitGroupRequired[k], "iRequired");
			pXML->GetChildXmlValByName(szTextVal, "EarlyArtDefineTag");
			setEarlyArtDefineTag(k, szTextVal);
			pXML->GetChildXmlValByName(szTextVal, "LateArtDefineTag");
			setLateArtDefineTag(k, szTextVal);
			pXML->GetChildXmlValByName(szTextVal, "MiddleArtDefineTag");
			setMiddleArtDefineTag(k, szTextVal);
			gDLL->getXMLIFace()->NextSibling(pXML->GetXML());
		}
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	gDLL->getXMLIFace()->SetToParent(pXML->GetXML());

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"UniqueNames"))
	{
		pXML->SetStringList(&m_paszUnitNames, &m_iNumUnitNames);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	pXML->SetVariableListTagPair(&m_pbFreePromotions, "FreePromotions", GC.getPromotionInfo(), sizeof(GC.getPromotionInfo((PromotionTypes)0)), GC.getNumPromotionInfos());

	pXML->GetChildXmlValByName(szTextVal, "LeaderPromotion");
	m_iLeaderPromotion = pXML->FindInInfoClass(szTextVal, GC.getPromotionInfo(), sizeof(GC.getPromotionInfo((PromotionTypes)0)), GC.getNumPromotionInfos());

	pXML->GetChildXmlValByName(&m_iLeaderExperience, "iLeaderExperience");

	return true;
}

//======================================================================================================
//					CvUnitFormationInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvUnitFormationInfo()
//
//  \brief		Default Constructor
//
//------------------------------------------------------------------------------------------------------
CvUnitFormationInfo::CvUnitFormationInfo() :
m_uiDirectionMask(0),
m_eDirectionType(NO_DIRECTION),
m_bUseFacingDirection(false)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvUnitFormationInfo()
//
//  \brief		Destructor
//
//------------------------------------------------------------------------------------------------------
CvUnitFormationInfo::~CvUnitFormationInfo()
{
}

const std::vector<EntityEventTypes> & CvUnitFormationInfo::getEventTypes() const
{
	return m_vctEventTypes;
}

const std::vector<UnitTypes> & CvUnitFormationInfo::getUnitTypes() const
{
	return m_vctUnitTypes;
}

const std::vector<PromotionTypes> & CvUnitFormationInfo::getPromotionTypes() const
{
	return m_vctPromotionTypes;
}

uint CvUnitFormationInfo::getDirectionTypeMask() const
{
	return m_uiDirectionMask;
}

DirectionTypes CvUnitFormationInfo::getDirectionType() const
{
	return m_eDirectionType;
}

int CvUnitFormationInfo::getNumUnits() const	// The number of units in the formation
{
	return m_vctUnitEntries.size();
}

void CvUnitFormationInfo::setNumUnits(int i)		// The number of units in the formation
{
	while((int) m_vctUnitEntries.size() < i)
		m_vctUnitEntries.push_back(UnitEntry());
}

bool CvUnitFormationInfo::getUseFacing() const
{
	return m_bUseFacingDirection;
}

// Arrays

float CvUnitFormationInfo::getUnitPosX(int i) const		// The normalized x positions of the units
{
	FAssertMsg((i >= 0) && (i < getNumUnits()), "Index out of bounds");
	return m_vctUnitEntries[i].m_position.x;
}

void CvUnitFormationInfo::setUnitPosX(int i, float f)
{
	FAssertMsg((i >= 0) && (i < getNumUnits()), "Index out of bounds");
	m_vctUnitEntries[i].m_position.x = f;
}

float CvUnitFormationInfo::getUnitPosY(int i) const		// The normalized y positions of the units
{
	FAssertMsg((i >= 0) && (i < getNumUnits()), "Index out of bounds");
	return m_vctUnitEntries[i].m_position.y;
}

void CvUnitFormationInfo::setUnitPosY(int i, float f)
{
	FAssertMsg((i >= 0) && (i < getNumUnits()), "Index out of bounds");
	m_vctUnitEntries[i].m_position.y = f;
}

float CvUnitFormationInfo::getUnitPosRadius(int i) const	// The (normalized) variance radius of the units position
{
	FAssertMsg((i >= 0) && (i < getNumUnits()), "Index out of bounds");
	return m_vctUnitEntries[i].m_fRadius;
}

void CvUnitFormationInfo::setUnitPosRadius(int i, float f)
{
	FAssertMsg((i >= 0) && (i < getNumUnits()), "Index out of bounds");
	m_vctUnitEntries[i].m_fRadius = f;
}

float CvUnitFormationInfo::getUnitFacingDir(int i) const	// The facing direction of the units
{
	FAssertMsg((i >= 0) && (i < getNumUnits()), "Index out of bounds");
	return m_vctUnitEntries[i].m_fFacingDirection;
}

void CvUnitFormationInfo::setUnitFacingDir(int i, float f)
{
	FAssertMsg((i >= 0) && (i < getNumUnits()), "Index out of bounds");
	m_vctUnitEntries[i].m_fFacingDirection = f;
}

float CvUnitFormationInfo::getUnitFacingVar(int i) const	// The variance of the facing direction of the units
{
	FAssertMsg((i >= 0) && (i < getNumUnits()), "Index out of bounds");
	return m_vctUnitEntries[i].m_fFacingVariance;
}

void CvUnitFormationInfo::setUnitFacingVar(int i, float f)
{
	FAssertMsg((i >= 0) && (i < getNumUnits()), "Index out of bounds");
	m_vctUnitEntries[i].m_fFacingVariance = f;
}

//great unit entries

int CvUnitFormationInfo::getNumGreatUnits() const	// The number of units in the formation
{
	return m_vctGreatUnitEntries.size();
}

void CvUnitFormationInfo::setNumGreatUnits(int i)		// The number of units in the formation
{
	while((int) m_vctGreatUnitEntries.size() < i)
		m_vctGreatUnitEntries.push_back(UnitEntry());
}

// Arrays

float CvUnitFormationInfo::getGreatUnitPosX(int i) const		// The normalized x positions of the units
{
	FAssertMsg((i >= 0) && (i < getNumGreatUnits()), "Index out of bounds");
	return m_vctGreatUnitEntries[i].m_position.x;
}

void CvUnitFormationInfo::setGreatUnitPosX(int i, float f)
{
	FAssertMsg((i >= 0) && (i < getNumGreatUnits()), "Index out of bounds");
	m_vctGreatUnitEntries[i].m_position.x = f;
}

float CvUnitFormationInfo::getGreatUnitPosY(int i) const		// The normalized y positions of the units
{
	FAssertMsg((i >= 0) && (i < getNumGreatUnits()), "Index out of bounds");
	return m_vctGreatUnitEntries[i].m_position.y;
}

void CvUnitFormationInfo::setGreatUnitPosY(int i, float f)
{
	FAssertMsg((i >= 0) && (i < getNumGreatUnits()), "Index out of bounds");
	m_vctGreatUnitEntries[i].m_position.y = f;
}

float CvUnitFormationInfo::getGreatUnitPosRadius(int i) const	// The (normalized) variance radius of the units position
{
	FAssertMsg((i >= 0) && (i < getNumGreatUnits()), "Index out of bounds");
	return m_vctGreatUnitEntries[i].m_fRadius;
}

void CvUnitFormationInfo::setGreatUnitPosRadius(int i, float f)
{
	FAssertMsg((i >= 0) && (i < getNumGreatUnits()), "Index out of bounds");
	m_vctGreatUnitEntries[i].m_fRadius = f;
}

float CvUnitFormationInfo::getGreatUnitFacingDir(int i) const	// The facing direction of the units
{
	FAssertMsg((i >= 0) && (i < getNumGreatUnits()), "Index out of bounds");
	return m_vctGreatUnitEntries[i].m_fFacingDirection;
}

void CvUnitFormationInfo::setGreatUnitFacingDir(int i, float f)
{
	FAssertMsg((i >= 0) && (i < getNumGreatUnits()), "Index out of bounds");
	m_vctGreatUnitEntries[i].m_fFacingDirection = f;
}

float CvUnitFormationInfo::getGreatUnitFacingVar(int i) const	// The variance of the facing direction of the units
{
	FAssertMsg((i >= 0) && (i < getNumGreatUnits()), "Index out of bounds");
	return m_vctGreatUnitEntries[i].m_fFacingVariance;
}

void CvUnitFormationInfo::setGreatUnitFacingVar(int i, float f)
{
	FAssertMsg((i >= 0) && (i < getNumGreatUnits()), "Index out of bounds");
	m_vctGreatUnitEntries[i].m_fFacingVariance = f;
}

//siege unit entries

int CvUnitFormationInfo::getNumSiegeUnits() const	// The number of units in the formation
{
	return m_vctSiegeUnitEntries.size();
}

void CvUnitFormationInfo::setNumSiegeUnits(int i)		// The number of units in the formation
{
	while((int) m_vctSiegeUnitEntries.size() < i)
		m_vctSiegeUnitEntries.push_back(UnitEntry());
}

// Arrays

float CvUnitFormationInfo::getSiegeUnitPosX(int i) const		// The normalized x positions of the units
{
	FAssertMsg((i >= 0) && (i < getNumSiegeUnits()), "Index out of bounds");
	return m_vctSiegeUnitEntries[i].m_position.x;
}

void CvUnitFormationInfo::setSiegeUnitPosX(int i, float f)
{
	FAssertMsg((i >= 0) && (i < getNumSiegeUnits()), "Index out of bounds");
	m_vctSiegeUnitEntries[i].m_position.x = f;
}

float CvUnitFormationInfo::getSiegeUnitPosY(int i) const		// The normalized y positions of the units
{
	FAssertMsg((i >= 0) && (i < getNumSiegeUnits()), "Index out of bounds");
	return m_vctSiegeUnitEntries[i].m_position.y;
}

void CvUnitFormationInfo::setSiegeUnitPosY(int i, float f)
{
	FAssertMsg((i >= 0) && (i < getNumSiegeUnits()), "Index out of bounds");
	m_vctSiegeUnitEntries[i].m_position.y = f;
}

float CvUnitFormationInfo::getSiegeUnitPosRadius(int i) const	// The (normalized) variance radius of the units position
{
	FAssertMsg((i >= 0) && (i < getNumSiegeUnits()), "Index out of bounds");
	return m_vctSiegeUnitEntries[i].m_fRadius;
}

void CvUnitFormationInfo::setSiegeUnitPosRadius(int i, float f)
{
	FAssertMsg((i >= 0) && (i < getNumSiegeUnits()), "Index out of bounds");
	m_vctSiegeUnitEntries[i].m_fRadius = f;
}

float CvUnitFormationInfo::getSiegeUnitFacingDir(int i) const	// The facing direction of the units
{
	FAssertMsg((i >= 0) && (i < getNumSiegeUnits()), "Index out of bounds");
	return m_vctSiegeUnitEntries[i].m_fFacingDirection;
}

void CvUnitFormationInfo::setSiegeUnitFacingDir(int i, float f)
{
	FAssertMsg((i >= 0) && (i < getNumSiegeUnits()), "Index out of bounds");
	m_vctSiegeUnitEntries[i].m_fFacingDirection = f;
}

float CvUnitFormationInfo::getSiegeUnitFacingVar(int i) const	// The variance of the facing direction of the units
{
	FAssertMsg((i >= 0) && (i < getNumSiegeUnits()), "Index out of bounds");
	return m_vctSiegeUnitEntries[i].m_fFacingVariance;
}

void CvUnitFormationInfo::setSiegeUnitFacingVar(int i, float f)
{
	FAssertMsg((i >= 0) && (i < getNumSiegeUnits()), "Index out of bounds");
	m_vctSiegeUnitEntries[i].m_fFacingVariance = f;
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvUnitFormationInfo()
//
//  \brief		Reads from XML
//
//------------------------------------------------------------------------------------------------------
bool CvUnitFormationInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	int iIndex;
	bool bNextSibling;
	
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	int iRotationMask;
	int iTemporary;

	// Create the rotation mask
	pXML->GetChildXmlValByName( szTextVal, "DirectionType");
	m_eDirectionType = (DirectionTypes)GC.getTypesEnum(szTextVal);
	pXML->GetChildXmlValByName( &iRotationMask,  "RotationMask" );
	for ( int i = 0; i < NUM_DIRECTION_TYPES; i++ )
	{
		if ( iRotationMask & 1 << ((( i - m_eDirectionType ) + NUM_DIRECTION_TYPES) % NUM_DIRECTION_TYPES) )
		{
			m_uiDirectionMask |= (1 << i);
		}
	}
	
	pXML->GetChildXmlValByName( &iTemporary, "UseFacing" );
	m_bUseFacingDirection = iTemporary == 0;

	if ( gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(), "EventMaskList" ))
	{
		if ( gDLL->getXMLIFace()->SetToChild( pXML->GetXML() ) )
		{
			pXML->GetXmlVal( szTextVal );
			do 
			{
				iIndex = pXML->FindInInfoClass(szTextVal, GC.getEntityEventInfo(), 
					sizeof( GC.getEntityEventInfo((EntityEventTypes)0)), GC.getNumEntityEventInfos());
				if ( iIndex != -1 )
					m_vctEventTypes.push_back( (EntityEventTypes)iIndex );
				bNextSibling = pXML->GetNextXmlVal( szTextVal );
			} 
			while( bNextSibling );
			gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
		}
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	if ( gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(), "UnitMaskList" ))
	{
		if ( gDLL->getXMLIFace()->SetToChild( pXML->GetXML() ) )
		{
			pXML->GetXmlVal( szTextVal );
			do 
			{
				iIndex = pXML->FindInInfoClass( szTextVal, GC.getUnitInfo(), sizeof(CvUnitInfo), GC.getNumUnitInfos() );
				if ( iIndex != -1 )
					m_vctUnitTypes.push_back( (UnitTypes)iIndex );
				bNextSibling = pXML->GetNextXmlVal( szTextVal );
			} 
			while( bNextSibling);
			gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
		}
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	if ( gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(), "PromotionMaskList" ))
	{
		if ( gDLL->getXMLIFace()->SetToChild( pXML->GetXML() ) )
		{
			pXML->GetXmlVal( szTextVal );
			do 
			{
				iIndex = pXML->FindInInfoClass( szTextVal, GC.getPromotionInfo(), sizeof(CvPromotionInfo), GC.getNumPromotionInfos() );
				if ( iIndex != -1 )
					m_vctPromotionTypes.push_back( (PromotionTypes)iIndex );
				bNextSibling = pXML->GetNextXmlVal( szTextVal );
			} 
			while( bNextSibling );
			gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
		}
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	// Read the entries
	if ( gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(), "UnitEntry" ) )
	{
		do 
		{
			UnitEntry unitEntry;
			if ( gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(), "Position" ) )
			{
				pXML->GetChildXmlValByName( &unitEntry.m_position.x, "x");
				pXML->GetChildXmlValByName( &unitEntry.m_position.y, "y");
				gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
			}
			pXML->GetChildXmlValByName( &unitEntry.m_fRadius, "PositionRadius");
			pXML->GetChildXmlValByName( &unitEntry.m_fFacingDirection, "Direction");
			pXML->GetChildXmlValByName( &unitEntry.m_fFacingVariance, "DirVariation");
			
			m_vctUnitEntries.push_back(unitEntry);
		}
		while ( gDLL->getXMLIFace()->LocateNextSiblingNodeByTagName(pXML->GetXML(), "UnitEntry"));
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	// Read the entries
	if ( gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(), "GreatUnitEntry" ) )
	{
		do 
		{
			UnitEntry unitEntry;
			if ( gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(), "Position" ) )
			{
				pXML->GetChildXmlValByName( &unitEntry.m_position.x, "x");
				pXML->GetChildXmlValByName( &unitEntry.m_position.y, "y");
				gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
			}
			pXML->GetChildXmlValByName( &unitEntry.m_fRadius, "PositionRadius");
			pXML->GetChildXmlValByName( &unitEntry.m_fFacingDirection, "Direction");
			pXML->GetChildXmlValByName( &unitEntry.m_fFacingVariance, "DirVariation");

			m_vctGreatUnitEntries.push_back(unitEntry);
		}
		while ( gDLL->getXMLIFace()->LocateNextSiblingNodeByTagName(pXML->GetXML(), "GreatUnitEntry"));
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	// Read the entries
	if ( gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(), "SiegeUnitEntry" ) )
	{
		do 
		{
			UnitEntry unitEntry;
			if ( gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(), "Position" ) )
			{
				pXML->GetChildXmlValByName( &unitEntry.m_position.x, "x");
				pXML->GetChildXmlValByName( &unitEntry.m_position.y, "y");
				gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
			}
			pXML->GetChildXmlValByName( &unitEntry.m_fRadius, "PositionRadius");
			pXML->GetChildXmlValByName( &unitEntry.m_fFacingDirection, "Direction");
			pXML->GetChildXmlValByName( &unitEntry.m_fFacingVariance, "DirVariation");

			m_vctSiegeUnitEntries.push_back(unitEntry);
		}
		while ( gDLL->getXMLIFace()->LocateNextSiblingNodeByTagName(pXML->GetXML(), "SiegeUnitEntry"));
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	return true;
}


//======================================================================================================
//					CvSpecialUnitInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvSpecialUnitInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvSpecialUnitInfo::CvSpecialUnitInfo() :
m_bValid(false),
m_piProductionTraits(NULL)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvSpecialUnitInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvSpecialUnitInfo::~CvSpecialUnitInfo()
{
	SAFE_DELETE_ARRAY(m_piProductionTraits);
}

bool CvSpecialUnitInfo::isValid() const
{
	return m_bValid;
}

// Arrays

int CvSpecialUnitInfo::getProductionTraits(int i) const		
{
	FAssertMsg(i < GC.getNumTraitInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piProductionTraits ? m_piProductionTraits[i] : -1;
}

bool CvSpecialUnitInfo::read(CvXMLLoadUtility* pXML)
{
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(&m_bValid, "bValid");

	pXML->SetVariableListTagPair(&m_piProductionTraits, "ProductionTraits", GC.getTraitInfo(), sizeof(GC.getTraitInfo((TraitTypes)0)), GC.getNumTraitInfos());

	return true;
}

//======================================================================================================
//					CvCivicOptionInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvCivicOptionInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvCivicOptionInfo::CvCivicOptionInfo() :
m_pabTraitNoUpkeep(NULL)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvCivicOptionInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvCivicOptionInfo::~CvCivicOptionInfo()
{
	SAFE_DELETE_ARRAY(m_pabTraitNoUpkeep);
}

bool CvCivicOptionInfo::getTraitNoUpkeep(int i) const
{
	FAssertMsg(i < GC.getNumTraitInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pabTraitNoUpkeep ? m_pabTraitNoUpkeep[i] : false;
}

bool CvCivicOptionInfo::read(CvXMLLoadUtility* pXML)
{
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->SetVariableListTagPair(&m_pabTraitNoUpkeep, "TraitNoUpkeeps", GC.getTraitInfo(), sizeof(GC.getTraitInfo((TraitTypes)0)), GC.getNumTraitInfos());

	return true;
}

//======================================================================================================
//					CvCivicInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvCivicInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvCivicInfo::CvCivicInfo() :
m_iCivicOptionType(NO_CIVICOPTION),
m_iAnarchyLength(0),
m_iUpkeep(0),
m_iAIWeight(0),
m_iGreatPeopleRateModifier(0),
m_iGreatGeneralRateModifier(0),
m_iDomesticGreatGeneralRateModifier(0),
m_iStateReligionGreatPeopleRateModifier(0),
m_iDistanceMaintenanceModifier(0),
m_iNumCitiesMaintenanceModifier(0),
m_iExtraHealth(0),
m_iFreeExperience(0),
m_iWorkerSpeedModifier(0),
m_iImprovementUpgradeRateModifier(0),
m_iMilitaryProductionModifier(0),
m_iBaseFreeUnits(0),
m_iBaseFreeMilitaryUnits(0),
m_iFreeUnitsPopulationPercent(0),
m_iFreeMilitaryUnitsPopulationPercent(0),
m_iGoldPerUnit(0),												
m_iGoldPerMilitaryUnit(0),									
m_iHappyPerMilitaryUnit(0),
m_iLargestCityHappiness(0),
m_iWarWearinessModifier(0),
m_iFreeSpecialist(0),
m_iTradeRoutes(0),
m_iTechPrereq(NO_TECH),
m_iCivicPercentAnger(0),									
m_iMaxConscript(0),											
m_iStateReligionHappiness(0),							
m_iNonStateReligionHappiness(0),						
m_iStateReligionUnitProductionModifier(0),
m_iStateReligionBuildingProductionModifier(0),
m_iStateReligionFreeExperience(0),
m_iExpInBorderModifier(0),
m_bMilitaryFoodProduction(false),
m_bNoUnhealthyPopulation(false),
m_bBuildingOnlyHealthy(false),
m_bNoForeignTrade(false),
m_bStateReligion(false),
m_bNoNonStateReligionSpread(false),
m_piYieldModifier(NULL),
m_piCapitalYieldModifier(NULL),
m_piTradeYieldModifier(NULL),
m_piCommerceModifier(NULL),
m_piCapitalCommerceModifier(NULL),
m_piSpecialistExtraCommerce(NULL),
m_paiBuildingHappinessChanges(NULL),
m_paiFeatureHappinessChanges(NULL),
m_pabHurry(NULL),
m_pabSpecialBuildingNotRequired(NULL),
m_pabSpecialistValid(NULL),
m_ppiImprovementYieldChanges(NULL)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvCivicInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvCivicInfo::~CvCivicInfo()
{
	int iI;

	SAFE_DELETE_ARRAY(m_piYieldModifier);
	SAFE_DELETE_ARRAY(m_piCapitalYieldModifier);
	SAFE_DELETE_ARRAY(m_piTradeYieldModifier);
	SAFE_DELETE_ARRAY(m_piCommerceModifier);
	SAFE_DELETE_ARRAY(m_piCapitalCommerceModifier);
	SAFE_DELETE_ARRAY(m_piSpecialistExtraCommerce);
	SAFE_DELETE_ARRAY(m_paiBuildingHappinessChanges);
	SAFE_DELETE_ARRAY(m_paiFeatureHappinessChanges);
	SAFE_DELETE_ARRAY(m_pabHurry);
	SAFE_DELETE_ARRAY(m_pabSpecialBuildingNotRequired);
	SAFE_DELETE_ARRAY(m_pabSpecialistValid);
	if (m_ppiImprovementYieldChanges != NULL)
	{
		for (iI=0;iI<GC.getNumImprovementInfos();iI++)
		{
			SAFE_DELETE_ARRAY(m_ppiImprovementYieldChanges[iI]);
		}
		SAFE_DELETE_ARRAY(m_ppiImprovementYieldChanges);
	}
}

int CvCivicInfo::getCivicOptionType() const										
{
	return m_iCivicOptionType;
}

int CvCivicInfo::getAnarchyLength() const						
{
	return m_iAnarchyLength;
}

int CvCivicInfo::getUpkeep() const
{
	return m_iUpkeep;
}

int CvCivicInfo::getAIWeight() const
{
	return m_iAIWeight;
}

int CvCivicInfo::getGreatPeopleRateModifier() const	
{
	return m_iGreatPeopleRateModifier;
}

int CvCivicInfo::getGreatGeneralRateModifier() const	
{
	return m_iGreatGeneralRateModifier;
}

int CvCivicInfo::getDomesticGreatGeneralRateModifier() const	
{
	return m_iDomesticGreatGeneralRateModifier;
}

int CvCivicInfo::getStateReligionGreatPeopleRateModifier() const
{
	return m_iStateReligionGreatPeopleRateModifier;
}

int CvCivicInfo::getDistanceMaintenanceModifier() const
{
	return m_iDistanceMaintenanceModifier;
}

int CvCivicInfo::getNumCitiesMaintenanceModifier() const
{
	return m_iNumCitiesMaintenanceModifier;
}

int CvCivicInfo::getExtraHealth() const
{
	return m_iExtraHealth;
}

int CvCivicInfo::getFreeExperience() const
{
	return m_iFreeExperience;
}

int CvCivicInfo::getWorkerSpeedModifier() const
{
	return m_iWorkerSpeedModifier;
}

int CvCivicInfo::getImprovementUpgradeRateModifier() const
{
	return m_iImprovementUpgradeRateModifier;
}

int CvCivicInfo::getMilitaryProductionModifier() const
{
	return m_iMilitaryProductionModifier;
}

int CvCivicInfo::getBaseFreeUnits() const
{
	return m_iBaseFreeUnits;
}

int CvCivicInfo::getBaseFreeMilitaryUnits() const		
{
	return m_iBaseFreeMilitaryUnits;
}

int CvCivicInfo::getFreeUnitsPopulationPercent() const
{
	return m_iFreeUnitsPopulationPercent;
}

int CvCivicInfo::getFreeMilitaryUnitsPopulationPercent() const	
{
	return m_iFreeMilitaryUnitsPopulationPercent;
}

int CvCivicInfo::getGoldPerUnit() const							
{
	return m_iGoldPerUnit;
}

int CvCivicInfo::getGoldPerMilitaryUnit() const			
{
	return m_iGoldPerMilitaryUnit;
}

int CvCivicInfo::getHappyPerMilitaryUnit() const
{
	return m_iHappyPerMilitaryUnit;
}

int CvCivicInfo::getLargestCityHappiness() const
{
	return m_iLargestCityHappiness;
}

int CvCivicInfo::getWarWearinessModifier() const
{
	return m_iWarWearinessModifier;
}

int CvCivicInfo::getFreeSpecialist() const					
{
	return m_iFreeSpecialist;
}

int CvCivicInfo::getTradeRoutes() const							
{
	return m_iTradeRoutes;
}

int CvCivicInfo::getTechPrereq() const							
{
	return m_iTechPrereq;
}

int CvCivicInfo::getCivicPercentAnger() const				
{
	return m_iCivicPercentAnger;
}

int CvCivicInfo::getMaxConscript() const						
{
	return m_iMaxConscript;
}

int CvCivicInfo::getStateReligionHappiness() const	
{
	return m_iStateReligionHappiness;
}

int CvCivicInfo::getNonStateReligionHappiness() const
{
	return m_iNonStateReligionHappiness;
}

int CvCivicInfo::getStateReligionUnitProductionModifier() const
{
	return m_iStateReligionUnitProductionModifier;
}

int CvCivicInfo::getStateReligionBuildingProductionModifier() const
{
	return m_iStateReligionBuildingProductionModifier;
}

int CvCivicInfo::getStateReligionFreeExperience() const
{
	return m_iStateReligionFreeExperience;
}

int CvCivicInfo::getExpInBorderModifier() const
{
	return m_iExpInBorderModifier;
}

bool CvCivicInfo::isMilitaryFoodProduction() const
{
	return m_bMilitaryFoodProduction;
}

bool CvCivicInfo::isNoUnhealthyPopulation() const
{
	return m_bNoUnhealthyPopulation;
}

bool CvCivicInfo::isBuildingOnlyHealthy() const
{
	return m_bBuildingOnlyHealthy;
}

bool CvCivicInfo::isNoForeignTrade() const
{
	return m_bNoForeignTrade;
}

bool CvCivicInfo::isStateReligion() const
{
	return m_bStateReligion;
}

bool CvCivicInfo::isNoNonStateReligionSpread() const
{
	return m_bNoNonStateReligionSpread;
}

const wchar* CvCivicInfo::getWeLoveTheKing()		
{
	m_szTempText = gDLL->getText(m_szWeLoveTheKingKey);
	return m_szTempText;
}

void CvCivicInfo::setWeLoveTheKingKey(const TCHAR* szVal)
{
	m_szWeLoveTheKingKey = szVal;
}

// Arrays

int CvCivicInfo::getYieldModifier(int i) const
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piYieldModifier ? m_piYieldModifier[i] : -1;
}

int* CvCivicInfo::getYieldModifierArray() const
{
	return m_piYieldModifier;
}

int CvCivicInfo::getCapitalYieldModifier(int i) const
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piCapitalYieldModifier ? m_piCapitalYieldModifier[i] : -1;
}

int* CvCivicInfo::getCapitalYieldModifierArray() const
{
	return m_piCapitalYieldModifier;
}

int CvCivicInfo::getTradeYieldModifier(int i) const
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piTradeYieldModifier ? m_piTradeYieldModifier[i] : -1;
}

int* CvCivicInfo::getTradeYieldModifierArray() const
{
	return m_piTradeYieldModifier;
}

int CvCivicInfo::getCommerceModifier(int i) const
{
	FAssertMsg(i < NUM_COMMERCE_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piCommerceModifier ? m_piCommerceModifier[i] : -1;
}

int* CvCivicInfo::getCommerceModifierArray() const
{
	return m_piCommerceModifier;
}

int CvCivicInfo::getCapitalCommerceModifier(int i) const
{
	FAssertMsg(i < NUM_COMMERCE_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piCapitalCommerceModifier ? m_piCapitalCommerceModifier[i] : -1;
}

int* CvCivicInfo::getCapitalCommerceModifierArray() const
{
	return m_piCapitalCommerceModifier;
}

int CvCivicInfo::getSpecialistExtraCommerce(int i) const
{
	FAssertMsg(i < NUM_COMMERCE_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piSpecialistExtraCommerce ? m_piSpecialistExtraCommerce[i] : -1;
}

int* CvCivicInfo::getSpecialistExtraCommerceArray() const
{
	return m_piSpecialistExtraCommerce;
}

int CvCivicInfo::getBuildingHappinessChanges(int i) const
{
	FAssertMsg(i < GC.getNumBuildingInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_paiBuildingHappinessChanges ? m_paiBuildingHappinessChanges[i] : -1;
}

int CvCivicInfo::getFeatureHappinessChanges(int i) const
{
	FAssertMsg(i < GC.getNumFeatureInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_paiFeatureHappinessChanges ? m_paiFeatureHappinessChanges[i] : -1;
}

bool CvCivicInfo::isHurry(int i) const							
{
	FAssertMsg(i < GC.getNumHurryInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pabHurry ? m_pabHurry[i] : false;
}

bool CvCivicInfo::isSpecialBuildingNotRequired(int i) const							
{
	FAssertMsg(i < GC.getNumSpecialBuildingInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pabSpecialBuildingNotRequired ? m_pabSpecialBuildingNotRequired[i] : false;
}

bool CvCivicInfo::isSpecialistValid(int i) const							
{
	FAssertMsg(i < GC.getNumSpecialistInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pabSpecialistValid ? m_pabSpecialistValid[i] : false;
}

int CvCivicInfo::getImprovementYieldChanges(int i, int j) const
{
	FAssertMsg(i < GC.getNumImprovementInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	FAssertMsg(j < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(j > -1, "Index out of bounds");
	return m_ppiImprovementYieldChanges[i][j];
}

void CvCivicInfo::read(FDataStreamBase* stream)
{
	CvInfoBase::read(stream);

	uint uiFlag=0;
	stream->Read(&uiFlag);		// flag for expansion
	
	stream->Read(&m_iCivicOptionType);
	stream->Read(&m_iAnarchyLength);
	stream->Read(&m_iUpkeep);
	stream->Read(&m_iAIWeight);
	stream->Read(&m_iGreatPeopleRateModifier);					
	stream->Read(&m_iGreatGeneralRateModifier);					
	stream->Read(&m_iDomesticGreatGeneralRateModifier);					
	stream->Read(&m_iStateReligionGreatPeopleRateModifier);					
	stream->Read(&m_iDistanceMaintenanceModifier);					
	stream->Read(&m_iNumCitiesMaintenanceModifier);					
	stream->Read(&m_iExtraHealth);
	stream->Read(&m_iFreeExperience);
	stream->Read(&m_iWorkerSpeedModifier);
	stream->Read(&m_iImprovementUpgradeRateModifier);
	stream->Read(&m_iMilitaryProductionModifier);
	stream->Read(&m_iBaseFreeUnits);											
	stream->Read(&m_iBaseFreeMilitaryUnits);								
	stream->Read(&m_iFreeUnitsPopulationPercent);						
	stream->Read(&m_iFreeMilitaryUnitsPopulationPercent);			
	stream->Read(&m_iGoldPerUnit);												
	stream->Read(&m_iGoldPerMilitaryUnit);									
	stream->Read(&m_iHappyPerMilitaryUnit);
	stream->Read(&m_iLargestCityHappiness);
	stream->Read(&m_iWarWearinessModifier);
	stream->Read(&m_iFreeSpecialist);
	stream->Read(&m_iTradeRoutes);												
	stream->Read(&m_iTechPrereq);												
	stream->Read(&m_iCivicPercentAnger);									
	stream->Read(&m_iMaxConscript);											
	stream->Read(&m_iStateReligionHappiness);							
	stream->Read(&m_iNonStateReligionHappiness);						
	stream->Read(&m_iStateReligionUnitProductionModifier);			
	stream->Read(&m_iStateReligionBuildingProductionModifier);	
	stream->Read(&m_iStateReligionFreeExperience);	
	stream->Read(&m_iExpInBorderModifier);

	stream->Read(&m_bMilitaryFoodProduction);
	stream->Read(&m_bNoUnhealthyPopulation);
	stream->Read(&m_bBuildingOnlyHealthy);								
	stream->Read(&m_bNoForeignTrade);
	stream->Read(&m_bStateReligion);
	stream->Read(&m_bNoNonStateReligionSpread);

	// Arrays

	SAFE_DELETE_ARRAY(m_piYieldModifier);
	m_piYieldModifier = new int[NUM_YIELD_TYPES];
	stream->Read(NUM_YIELD_TYPES, m_piYieldModifier);

	SAFE_DELETE_ARRAY(m_piCapitalYieldModifier);
	m_piCapitalYieldModifier = new int[NUM_YIELD_TYPES];
	stream->Read(NUM_YIELD_TYPES, m_piCapitalYieldModifier);

	SAFE_DELETE_ARRAY(m_piTradeYieldModifier);
	m_piTradeYieldModifier = new int[NUM_YIELD_TYPES];
	stream->Read(NUM_YIELD_TYPES, m_piTradeYieldModifier);

	SAFE_DELETE_ARRAY(m_piCommerceModifier);
	m_piCommerceModifier = new int[NUM_COMMERCE_TYPES];
	stream->Read(NUM_COMMERCE_TYPES, m_piCommerceModifier);

	SAFE_DELETE_ARRAY(m_piCapitalCommerceModifier);
	m_piCapitalCommerceModifier = new int[NUM_COMMERCE_TYPES];
	stream->Read(NUM_COMMERCE_TYPES, m_piCapitalCommerceModifier);

	SAFE_DELETE_ARRAY(m_piSpecialistExtraCommerce);
	m_piSpecialistExtraCommerce = new int[NUM_COMMERCE_TYPES];
	stream->Read(NUM_COMMERCE_TYPES, m_piSpecialistExtraCommerce);

	SAFE_DELETE_ARRAY(m_paiBuildingHappinessChanges);
	m_paiBuildingHappinessChanges = new int[GC.getNumBuildingInfos()];
	stream->Read(GC.getNumBuildingInfos(), m_paiBuildingHappinessChanges);

	SAFE_DELETE_ARRAY(m_paiFeatureHappinessChanges);
	m_paiFeatureHappinessChanges = new int[GC.getNumFeatureInfos()];
	stream->Read(GC.getNumFeatureInfos(), m_paiFeatureHappinessChanges);

	SAFE_DELETE_ARRAY(m_pabHurry);
	m_pabHurry = new bool[GC.getNumHurryInfos()];
	stream->Read(GC.getNumHurryInfos(), m_pabHurry);

	SAFE_DELETE_ARRAY(m_pabSpecialBuildingNotRequired);
	m_pabSpecialBuildingNotRequired = new bool[GC.getNumSpecialBuildingInfos()];
	stream->Read(GC.getNumSpecialBuildingInfos(), m_pabSpecialBuildingNotRequired);

	SAFE_DELETE_ARRAY(m_pabSpecialistValid);
	m_pabSpecialistValid = new bool[GC.getNumSpecialistInfos()];
	stream->Read(GC.getNumSpecialistInfos(), m_pabSpecialistValid);
	
	int i;
	if (m_ppiImprovementYieldChanges != NULL)
	{
		for(i=0;i<GC.getNumImprovementInfos();i++)
		{
			SAFE_DELETE_ARRAY(m_ppiImprovementYieldChanges[i]);
		}
		SAFE_DELETE_ARRAY(m_ppiImprovementYieldChanges);
	}
	m_ppiImprovementYieldChanges = new int*[GC.getNumImprovementInfos()];
	for(i=0;i<GC.getNumImprovementInfos();i++)
	{
		m_ppiImprovementYieldChanges[i]  = new int[NUM_YIELD_TYPES];
		stream->Read(NUM_YIELD_TYPES, m_ppiImprovementYieldChanges[i]);
	}

	stream->ReadString(m_szWeLoveTheKingKey);
}

void CvCivicInfo::write(FDataStreamBase* stream)
{
	CvInfoBase::write(stream);

	uint uiFlag=0;
	stream->Write(uiFlag);		// flag for expansion

	stream->Write(m_iCivicOptionType);
	stream->Write(m_iAnarchyLength);
	stream->Write(m_iUpkeep);
	stream->Write(m_iAIWeight);
	stream->Write(m_iGreatPeopleRateModifier);					
	stream->Write(m_iGreatGeneralRateModifier);					
	stream->Write(m_iDomesticGreatGeneralRateModifier);					
	stream->Write(m_iStateReligionGreatPeopleRateModifier);					
	stream->Write(m_iDistanceMaintenanceModifier);					
	stream->Write(m_iNumCitiesMaintenanceModifier);					
	stream->Write(m_iExtraHealth);
	stream->Write(m_iFreeExperience);
	stream->Write(m_iWorkerSpeedModifier);
	stream->Write(m_iImprovementUpgradeRateModifier);
	stream->Write(m_iMilitaryProductionModifier);
	stream->Write(m_iBaseFreeUnits);											
	stream->Write(m_iBaseFreeMilitaryUnits);								
	stream->Write(m_iFreeUnitsPopulationPercent);						
	stream->Write(m_iFreeMilitaryUnitsPopulationPercent);			
	stream->Write(m_iGoldPerUnit);												
	stream->Write(m_iGoldPerMilitaryUnit);									
	stream->Write(m_iHappyPerMilitaryUnit);
	stream->Write(m_iLargestCityHappiness);
	stream->Write(m_iWarWearinessModifier);
	stream->Write(m_iFreeSpecialist);
	stream->Write(m_iTradeRoutes);												
	stream->Write(m_iTechPrereq);												
	stream->Write(m_iCivicPercentAnger);									
	stream->Write(m_iMaxConscript);											
	stream->Write(m_iStateReligionHappiness);							
	stream->Write(m_iNonStateReligionHappiness);						
	stream->Write(m_iStateReligionUnitProductionModifier);			
	stream->Write(m_iStateReligionBuildingProductionModifier);	
	stream->Write(m_iStateReligionFreeExperience);	
	stream->Write(m_iExpInBorderModifier);

	stream->Write(m_bMilitaryFoodProduction);
	stream->Write(m_bNoUnhealthyPopulation);
	stream->Write(m_bBuildingOnlyHealthy);								
	stream->Write(m_bNoForeignTrade);
	stream->Write(m_bStateReligion);
	stream->Write(m_bNoNonStateReligionSpread);

	// Arrays

	stream->Write(NUM_YIELD_TYPES, m_piYieldModifier);
	stream->Write(NUM_YIELD_TYPES, m_piCapitalYieldModifier);
	stream->Write(NUM_YIELD_TYPES, m_piTradeYieldModifier);
	stream->Write(NUM_COMMERCE_TYPES, m_piCommerceModifier);
	stream->Write(NUM_COMMERCE_TYPES, m_piCapitalCommerceModifier);
	stream->Write(NUM_COMMERCE_TYPES, m_piSpecialistExtraCommerce);
	stream->Write(GC.getNumBuildingInfos(), m_paiBuildingHappinessChanges);
	stream->Write(GC.getNumFeatureInfos(), m_paiFeatureHappinessChanges);
	stream->Write(GC.getNumHurryInfos(), m_pabHurry);
	stream->Write(GC.getNumSpecialBuildingInfos(), m_pabSpecialBuildingNotRequired);
	stream->Write(GC.getNumSpecialistInfos(), m_pabSpecialistValid);

	int i;
	for(i=0;i<GC.getNumImprovementInfos();i++)
	{
		stream->Write(NUM_YIELD_TYPES, m_ppiImprovementYieldChanges[i]);
	}

	stream->WriteString(m_szWeLoveTheKingKey);
}

bool CvCivicInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	int j;
	int iNumSibs=0;				// the number of siblings the current xml node has
	int iIndex;

	pXML->GetChildXmlValByName(szTextVal, "CivicOptionType");
	m_iCivicOptionType = pXML->FindInInfoClass(szTextVal, GC.getCivicOptionInfo(), sizeof(GC.getCivicOptionInfo((CivicOptionTypes)0)), GC.getNumCivicOptionInfos());

	pXML->GetChildXmlValByName(szTextVal, "TechPrereq");
	m_iTechPrereq = pXML->FindInInfoClass(szTextVal, GC.getTechInfo(), sizeof(GC.getTechInfo((TechTypes)0)), GC.getNumTechInfos());

	pXML->GetChildXmlValByName(&m_iAnarchyLength, "iAnarchyLength");

	pXML->GetChildXmlValByName(szTextVal, "Upkeep");
	m_iUpkeep = pXML->FindInInfoClass(szTextVal, GC.getUpkeepInfo(), sizeof(GC.getUpkeepInfo((UpkeepTypes)0)), GC.getNumUpkeepInfos());

	pXML->GetChildXmlValByName(&m_iAIWeight, "iAIWeight");
	pXML->GetChildXmlValByName(&m_iGreatPeopleRateModifier, "iGreatPeopleRateModifier");
	pXML->GetChildXmlValByName(&m_iGreatGeneralRateModifier, "iGreatGeneralRateModifier");
	pXML->GetChildXmlValByName(&m_iDomesticGreatGeneralRateModifier, "iDomesticGreatGeneralRateModifier");
	pXML->GetChildXmlValByName(&m_iStateReligionGreatPeopleRateModifier, "iStateReligionGreatPeopleRateModifier");
	pXML->GetChildXmlValByName(&m_iDistanceMaintenanceModifier, "iDistanceMaintenanceModifier");
	pXML->GetChildXmlValByName(&m_iNumCitiesMaintenanceModifier, "iNumCitiesMaintenanceModifier");
	pXML->GetChildXmlValByName(&m_iExtraHealth, "iExtraHealth");
	pXML->GetChildXmlValByName(&m_iFreeExperience, "iFreeExperience");
	pXML->GetChildXmlValByName(&m_iWorkerSpeedModifier, "iWorkerSpeedModifier");
	pXML->GetChildXmlValByName(&m_iImprovementUpgradeRateModifier, "iImprovementUpgradeRateModifier");
	pXML->GetChildXmlValByName(&m_iMilitaryProductionModifier, "iMilitaryProductionModifier");
	pXML->GetChildXmlValByName(&m_iBaseFreeUnits, "iBaseFreeUnits");
	pXML->GetChildXmlValByName(&m_iBaseFreeMilitaryUnits, "iBaseFreeMilitaryUnits");
	pXML->GetChildXmlValByName(&m_iFreeUnitsPopulationPercent, "iFreeUnitsPopulationPercent");
	pXML->GetChildXmlValByName(&m_iFreeMilitaryUnitsPopulationPercent, "iFreeMilitaryUnitsPopulationPercent");
	pXML->GetChildXmlValByName(&m_iGoldPerUnit, "iGoldPerUnit");
	pXML->GetChildXmlValByName(&m_iGoldPerMilitaryUnit, "iGoldPerMilitaryUnit");
	pXML->GetChildXmlValByName(&m_iHappyPerMilitaryUnit, "iHappyPerMilitaryUnit");
	pXML->GetChildXmlValByName(&m_bMilitaryFoodProduction, "bMilitaryFoodProduction");
	pXML->GetChildXmlValByName(&m_iMaxConscript, "iMaxConscript");
	pXML->GetChildXmlValByName(&m_bNoUnhealthyPopulation, "bNoUnhealthyPopulation");
	pXML->GetChildXmlValByName(&m_bBuildingOnlyHealthy, "bBuildingOnlyHealthy");
	pXML->GetChildXmlValByName(&m_iLargestCityHappiness, "iLargestCityHappiness");
	pXML->GetChildXmlValByName(&m_iWarWearinessModifier, "iWarWearinessModifier");
	pXML->GetChildXmlValByName(&m_iFreeSpecialist, "iFreeSpecialist");
	pXML->GetChildXmlValByName(&m_iTradeRoutes, "iTradeRoutes");
	pXML->GetChildXmlValByName(&m_bNoForeignTrade, "bNoForeignTrade");
	pXML->GetChildXmlValByName(&m_iCivicPercentAnger, "iCivicPercentAnger");
	pXML->GetChildXmlValByName(&m_bStateReligion, "bStateReligion");
	pXML->GetChildXmlValByName(&m_bNoNonStateReligionSpread, "bNoNonStateReligionSpread");
	pXML->GetChildXmlValByName(&m_iStateReligionHappiness, "iStateReligionHappiness");
	pXML->GetChildXmlValByName(&m_iNonStateReligionHappiness, "iNonStateReligionHappiness");
	pXML->GetChildXmlValByName(&m_iStateReligionUnitProductionModifier, "iStateReligionUnitProductionModifier");
	pXML->GetChildXmlValByName(&m_iStateReligionBuildingProductionModifier, "iStateReligionBuildingProductionModifier");
	pXML->GetChildXmlValByName(&m_iStateReligionFreeExperience, "iStateReligionFreeExperience");
	pXML->GetChildXmlValByName(&m_iExpInBorderModifier, "iExpInBorderModifier");

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"YieldModifiers"))
	{
		pXML->SetYields(&m_piYieldModifier);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piYieldModifier, NUM_YIELD_TYPES);
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"CapitalYieldModifiers"))
	{
		pXML->SetYields(&m_piCapitalYieldModifier);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piCapitalYieldModifier, NUM_YIELD_TYPES);
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"TradeYieldModifiers"))
	{
		pXML->SetYields(&m_piTradeYieldModifier);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piTradeYieldModifier, NUM_YIELD_TYPES);
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"CommerceModifiers"))
	{
		pXML->SetCommerce(&m_piCommerceModifier);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piCommerceModifier, NUM_COMMERCE_TYPES);
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"CapitalCommerceModifiers"))
	{
		pXML->SetCommerce(&m_piCapitalCommerceModifier);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piCapitalCommerceModifier, NUM_COMMERCE_TYPES);
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"SpecialistExtraCommerces"))
	{
		pXML->SetCommerce(&m_piSpecialistExtraCommerce);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piSpecialistExtraCommerce, NUM_COMMERCE_TYPES);
	}

	pXML->SetVariableListTagPair(&m_pabHurry, "Hurrys", GC.getHurryInfo(), sizeof(GC.getHurryInfo((HurryTypes)0)), GC.getNumHurryInfos());
	pXML->SetVariableListTagPair(&m_pabSpecialBuildingNotRequired, "SpecialBuildingNotRequireds", GC.getSpecialBuildingInfo(), sizeof(GC.getSpecialBuildingInfo((SpecialBuildingTypes)0)), GC.getNumSpecialBuildingInfos());
	pXML->SetVariableListTagPair(&m_pabSpecialistValid, "SpecialistValids", GC.getSpecialistInfo(), sizeof(GC.getSpecialistInfo((SpecialistTypes)0)), GC.getNumSpecialistInfos());

	pXML->SetVariableListTagPair(&m_paiBuildingHappinessChanges, "BuildingHappinessChanges", GC.getBuildingInfo(), sizeof(GC.getBuildingInfo((BuildingTypes)0)), GC.getNumBuildingInfos());
	pXML->SetVariableListTagPair(&m_paiFeatureHappinessChanges, "FeatureHappinessChanges", GC.getFeatureInfo(), sizeof(GC.getFeatureInfo((FeatureTypes)0)), GC.getNumFeatureInfos());

	// initialize the boolean list to the correct size and all the booleans to false
	FAssertMsg((GC.getNumImprovementInfos() > 0) && (NUM_YIELD_TYPES) > 0,"either the number of improvement infos is zero or less or the number of yield types is zero or less");
	pXML->Init2DIntList(&m_ppiImprovementYieldChanges, GC.getNumImprovementInfos(), NUM_YIELD_TYPES);
	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"ImprovementYieldChanges"))
	{
		if (pXML->SkipToNextVal())
		{
			iNumSibs = gDLL->getXMLIFace()->GetNumChildren(pXML->GetXML());
			if (gDLL->getXMLIFace()->SetToChild(pXML->GetXML()))
			{
				if (0 < iNumSibs)
				{
					for (j=0;j<iNumSibs;j++)
					{
						pXML->GetChildXmlValByName(szTextVal, "ImprovementType");
						iIndex = pXML->FindInInfoClass(szTextVal, GC.getImprovementInfo(), sizeof(GC.getImprovementInfo((ImprovementTypes)0)), GC.getNumImprovementInfos());

						if (iIndex > -1)
						{
							// delete the array since it will be reallocated
							SAFE_DELETE_ARRAY(m_ppiImprovementYieldChanges[iIndex]);
							// if we can set the current xml node to it's next sibling
							if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"ImprovementYields"))
							{
								// call the function that sets the yield change variable
								pXML->SetYields(&m_ppiImprovementYieldChanges[iIndex]);
								gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
							}
							else
							{
								pXML->InitList(&m_ppiImprovementYieldChanges[iIndex], NUM_YIELD_TYPES);
							}
						}

						if (!gDLL->getXMLIFace()->NextSibling(pXML->GetXML()))
						{
							break;
						}
					}
				}

				gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
			}
		}

		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	pXML->GetChildXmlValByName(szTextVal, "WeLoveTheKing");
	setWeLoveTheKingKey(szTextVal);

	return true;
}

//======================================================================================================
//					CvDiplomacyInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvDiplomacyInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvDiplomacyInfo::CvDiplomacyInfo() :
m_iNumResponses(-1),
m_pResponses(NULL)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvDiplomacyInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvDiplomacyInfo::~CvDiplomacyInfo()
{
	uninit();
}

// note - Response member vars allocated by CvXmlLoadUtility  
void CvDiplomacyInfo::init(int iNum)
{
	uninit();
	m_pResponses = new CvDiplomacyResponse[iNum];
	m_iNumResponses=iNum; 
}

void CvDiplomacyInfo::uninit()
{
	SAFE_DELETE_ARRAY(m_pResponses); 
}

int CvDiplomacyInfo::getNumResponses() const
{
	return m_iNumResponses; 
}

bool CvDiplomacyInfo::getCivilizationTypes(int i, int j) const
{
	FAssertMsg(i < getNumResponses(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	FAssertMsg(j < GC.getNumCivilizationInfos(), "Index out of bounds");
	FAssertMsg(j > -1, "Index out of bounds");
	return m_pResponses[i].getCivilizationTypes(j); 
}

bool CvDiplomacyInfo::getLeaderHeadTypes(int i, int j) const
{
	FAssertMsg(i < getNumResponses(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	FAssertMsg(j < GC.getNumLeaderHeadInfos(), "Index out of bounds");
	FAssertMsg(j > -1, "Index out of bounds");
	return m_pResponses[i].getLeaderHeadTypes(j); 
}

bool CvDiplomacyInfo::getAttitudeTypes(int i, int j) const
{
	FAssertMsg(i < getNumResponses(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	FAssertMsg(j < NUM_ATTITUDE_TYPES, "Index out of bounds");
	FAssertMsg(j > -1, "Index out of bounds");
	return m_pResponses[i].getAttitudeTypes(j); 
}

bool CvDiplomacyInfo::getDiplomacyPowerTypes(int i, int j) const
{
	FAssertMsg(i < getNumResponses(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	FAssertMsg(j < NUM_DIPLOMACYPOWER_TYPES, "Index out of bounds");
	FAssertMsg(j > -1, "Index out of bounds");
	return m_pResponses[i].getDiplomacyPowerTypes(j); 
}

int CvDiplomacyInfo::getNumDiplomacyText(int i) const
{
	FAssertMsg(i < getNumResponses(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pResponses[i].getNumDiplomacyText(); 
}

const TCHAR* CvDiplomacyInfo::getDiplomacyText(int i, int j) const
{
	FAssertMsg(i < getNumResponses(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	FAssertMsg(j < getNumDiplomacyText(i), "Index out of bounds");
	FAssertMsg(j > -1, "Index out of bounds");
	return m_pResponses[i].getDiplomacyText(j); 
}

void CvDiplomacyInfo::read(FDataStreamBase* stream)
{
	CvInfoBase::read(stream);

	uint uiFlag=0;
	stream->Read(&uiFlag);		// flag for expansion
	
	stream->Read(&m_iNumResponses);

	// Arrays

	init(m_iNumResponses);

	for (uint uiIndex = 0; (int) uiIndex < m_iNumResponses; uiIndex++)
	{
		m_pResponses[uiIndex].read(stream);
	}
}

void CvDiplomacyInfo::write(FDataStreamBase* stream)
{
	CvInfoBase::write(stream);

	uint uiFlag=0;
	stream->Write(uiFlag);		// flag for expansion

	stream->Write(m_iNumResponses);

	// Arrays
	
	for (uint uiIndex = 0; (int) uiIndex < m_iNumResponses; uiIndex++)
	{
		m_pResponses[uiIndex].write(stream);
	}
}

bool CvDiplomacyInfo::read(CvXMLLoadUtility* pXML)
{
	int i;
	int iIndexVal;

	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	if ( gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"Responses") )
	{
		iIndexVal = gDLL->getXMLIFace()->NumOfChildrenByTagName(pXML->GetXML(), "Response");
		init(iIndexVal);

		for (i = 0; i < iIndexVal; i++)
		{
			if (i == 0)
			{
				gDLL->getXMLIFace()->SetToChild(pXML->GetXML());
			}
			m_pResponses[i].read(pXML);
			// Civilizations

			if (!gDLL->getXMLIFace()->NextSibling(pXML->GetXML()))
			{
				break;
			}
		}

		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	gDLL->getXMLIFace()->SetToParent(pXML->GetXML());

	return true;
}

//======================================================================================================
//					CvUnitClassInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvUnitClassInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvUnitClassInfo::CvUnitClassInfo() :
m_iMaxGlobalInstances(0),
m_iMaxTeamInstances(0),
m_iMaxPlayerInstances(0),
m_iDefaultUnitIndex(NO_UNIT)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvUnitClassInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvUnitClassInfo::~CvUnitClassInfo()
{
}

int CvUnitClassInfo::getMaxGlobalInstances() const
{
	return m_iMaxGlobalInstances;
}

int CvUnitClassInfo::getMaxTeamInstances() const
{
	return m_iMaxTeamInstances;
}

int CvUnitClassInfo::getMaxPlayerInstances() const
{
	return m_iMaxPlayerInstances;
}

int CvUnitClassInfo::getDefaultUnitIndex() const
{
	return m_iDefaultUnitIndex;
}

void CvUnitClassInfo::setDefaultUnitIndex(int i)
{
	m_iDefaultUnitIndex = i;
}

bool CvUnitClassInfo::read(CvXMLLoadUtility* pXML)
{
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(&m_iMaxGlobalInstances, "iMaxGlobalInstances");
	pXML->GetChildXmlValByName(&m_iMaxTeamInstances, "iMaxTeamInstances");
	pXML->GetChildXmlValByName(&m_iMaxPlayerInstances, "iMaxPlayerInstances");

	return true;
}

//======================================================================================================
//					CvBuildingInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvBuildingInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvBuildingInfo::CvBuildingInfo() :
m_iBuildingClassType(NO_BUILDINGCLASS),
m_iVictoryPrereq(NO_VICTORY),
m_iFreeStartEra(NO_ERA),
m_iMaxStartEra(NO_ERA),
m_iObsoleteTech(NO_TECH),
m_iPrereqAndTech(NO_TECH),
m_iPowerBonus(NO_BONUS),
m_iFreeBonus(NO_BONUS),
m_iNumFreeBonuses(0),
m_iFreeBuildingClass(NO_BUILDINGCLASS),
m_iFreePromotion(NO_PROMOTION),
m_iCivicOption(NO_CIVICOPTION),
m_iAIWeight(0),
m_iProductionCost(0),
m_iHurryCostModifier(0),							
m_iHurryAngerModifier(0),
m_iMinAreaSize(0),									
m_iNumCitiesPrereq(0),							
m_iNumTeamsPrereq(0),							
m_iUnitLevelPrereq(0),							
m_iMinLatitude(0),									
m_iMaxLatitude(0),									
m_iGreatPeopleRateModifier(0),				
m_iGreatGeneralRateModifier(0),				
m_iDomesticGreatGeneralRateModifier(0),				
m_iGlobalGreatPeopleRateModifier(0),	
m_iAnarchyModifier(0),							
m_iGlobalHurryModifier(0),						
m_iFreeExperience(0),
m_iGlobalFreeExperience(0),						
m_iFoodKept(0),
m_iAirlift(0),
m_iAirModifier(0),									
m_iNukeModifier(0),
m_iNukeExplosionRand(0),
m_iFreeSpecialist(0),								
m_iAreaFreeSpecialist(0),						
m_iGlobalFreeSpecialist(0),						
m_iHappiness(0),									
m_iAreaHappiness(0),								
m_iGlobalHappiness(0),							
m_iStateReligionHappiness(0),				
m_iWorkerSpeedModifier(0),					
m_iMilitaryProductionModifier(0),				
m_iSpaceProductionModifier(0),				
m_iGlobalSpaceProductionModifier(0),	
m_iTradeRoutes(0),									
m_iCoastalTradeRoutes(0),						
m_iGlobalTradeRoutes(0),						
m_iTradeRouteModifier(0),						
m_iAssetValue(0),									
m_iPowerValue(0),									
m_iSpecialBuildingType(NO_SPECIALBUILDING),						
m_iAdvisorType(NO_ADVISOR),
m_iHolyCity(NO_RELIGION),										
m_iReligionType(NO_RELIGION),								
m_iStateReligion(NO_RELIGION),								
m_iPrereqReligion(NO_RELIGION),								
m_iGlobalReligionCommerce(0),
m_iPrereqAndBonus(NO_BONUS),							
m_iGreatPeopleUnitClass(NO_UNITCLASS),					
m_iGreatPeopleRateChange(0),				
m_iConquestProbability(0),
m_iMaintenanceModifier(0),
m_iWarWearinessModifier(0),
m_iGlobalWarWearinessModifier(0),
m_iHealRateChange(0),
m_iHealth(0),
m_iAreaHealth(0),
m_iGlobalHealth(0),
m_iGlobalPopulationChange(0),
m_iFreeTechs(0),
m_iDefenseModifier(0),
m_iBombardDefenseModifier(0),
m_iAllCityDefenseModifier(0),
m_iMissionType(NO_MISSION),
m_fVisibilityPriority(0.0f),
m_bTeamShare(false),
m_bWater(false),								
m_bRiver(false),									
m_bPower(false),								
m_bDirtyPower(false),
m_bAreaCleanPower(false),
m_bAreaBorderObstacle(false),
m_bDiploVote(false),
m_bForceTeamVoteEligible(false),
m_bCapital(false),
m_bGovernmentCenter(false),
m_bGoldenAge(false),
m_bMapCentering(false),
m_bNoUnhappiness(false),
m_bNoUnhealthyPopulation(false),
m_bBuildingOnlyHealthy(false),			
m_bNeverCapture(false),					
m_bNukeImmune(false),					
m_bPrereqReligion(false),					
m_bCenterInCity(false),
m_piPrereqAndTechs(NULL),
m_piPrereqOrBonuses(NULL),
m_piProductionTraits(NULL),
m_piHappinessTraits(NULL),
m_piSeaPlotYieldChange(NULL),
m_piGlobalSeaPlotYieldChange(NULL),
m_piYieldChange(NULL),
m_piYieldModifier(NULL),
m_piPowerYieldModifier(NULL),
m_piAreaYieldModifier(NULL),
m_piGlobalYieldModifier(NULL),
m_piCommerceChange(NULL),
m_piObsoleteSafeCommerceChange(NULL),
m_piCommerceChangeDoubleTime(NULL),
m_piCommerceModifier(NULL),
m_piGlobalCommerceModifier(NULL),
m_piSpecialistExtraCommerce(NULL),
m_piStateReligionCommerce(NULL),
m_piCommerceHappiness(NULL),
m_piReligionChange(NULL),
m_piSpecialistCount(NULL),
m_piFreeSpecialistCount(NULL),
m_piBonusHealthChanges(NULL),
m_piBonusHappinessChanges(NULL),
m_piBonusProductionModifier(NULL),
m_piUnitCombatFreeExperience(NULL),
m_piDomainFreeExperience(NULL),
m_piDomainProductionModifier(NULL),
m_piBuildingHappinessChanges(NULL),
m_piPrereqNumOfBuildingClass(NULL),
m_piFlavorValue(NULL),
m_pbCommerceFlexible(NULL),
m_pbCommerceChangeOriginalOwner(NULL),
m_pbBuildingClassNeededInCity(NULL),
m_ppaiSpecialistYieldChange(NULL),
m_ppaiBonusYieldModifier(NULL)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvBuildingInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvBuildingInfo::~CvBuildingInfo()
{
	SAFE_DELETE_ARRAY(m_piPrereqAndTechs);
	SAFE_DELETE_ARRAY(m_piPrereqOrBonuses);
	SAFE_DELETE_ARRAY(m_piProductionTraits);
	SAFE_DELETE_ARRAY(m_piHappinessTraits);
	SAFE_DELETE_ARRAY(m_piSeaPlotYieldChange);
	SAFE_DELETE_ARRAY(m_piGlobalSeaPlotYieldChange);
	SAFE_DELETE_ARRAY(m_piYieldChange);
	SAFE_DELETE_ARRAY(m_piYieldModifier);
	SAFE_DELETE_ARRAY(m_piPowerYieldModifier);
	SAFE_DELETE_ARRAY(m_piAreaYieldModifier);
	SAFE_DELETE_ARRAY(m_piGlobalYieldModifier);
	SAFE_DELETE_ARRAY(m_piCommerceChange);
	SAFE_DELETE_ARRAY(m_piObsoleteSafeCommerceChange);
	SAFE_DELETE_ARRAY(m_piCommerceChangeDoubleTime);
	SAFE_DELETE_ARRAY(m_piCommerceModifier);
	SAFE_DELETE_ARRAY(m_piGlobalCommerceModifier);
	SAFE_DELETE_ARRAY(m_piSpecialistExtraCommerce);
	SAFE_DELETE_ARRAY(m_piStateReligionCommerce);
	SAFE_DELETE_ARRAY(m_piCommerceHappiness);
	SAFE_DELETE_ARRAY(m_piReligionChange);
	SAFE_DELETE_ARRAY(m_piSpecialistCount);
	SAFE_DELETE_ARRAY(m_piFreeSpecialistCount);
	SAFE_DELETE_ARRAY(m_piBonusHealthChanges);
	SAFE_DELETE_ARRAY(m_piBonusHappinessChanges);
	SAFE_DELETE_ARRAY(m_piBonusProductionModifier);
	SAFE_DELETE_ARRAY(m_piUnitCombatFreeExperience);
	SAFE_DELETE_ARRAY(m_piDomainFreeExperience);
	SAFE_DELETE_ARRAY(m_piDomainProductionModifier);
	SAFE_DELETE_ARRAY(m_piBuildingHappinessChanges);
	SAFE_DELETE_ARRAY(m_piPrereqNumOfBuildingClass);
	SAFE_DELETE_ARRAY(m_piFlavorValue);
	SAFE_DELETE_ARRAY(m_pbCommerceFlexible);
	SAFE_DELETE_ARRAY(m_pbCommerceChangeOriginalOwner);
	SAFE_DELETE_ARRAY(m_pbBuildingClassNeededInCity);

	if (m_ppaiSpecialistYieldChange != NULL)
	{
		for(int i=0;i<GC.getNumSpecialistInfos();i++)
		{
			SAFE_DELETE_ARRAY(m_ppaiSpecialistYieldChange[i]);
		}
		SAFE_DELETE_ARRAY(m_ppaiSpecialistYieldChange);
	}

	if (m_ppaiBonusYieldModifier != NULL)
	{
		for(int i=0;i<GC.getNumBonusInfos();i++)
		{
			SAFE_DELETE_ARRAY(m_ppaiBonusYieldModifier[i]);
		}
		SAFE_DELETE_ARRAY(m_ppaiBonusYieldModifier);
	}
}

int CvBuildingInfo::getBuildingClassType() const	
{
	return m_iBuildingClassType;
}

int CvBuildingInfo::getVictoryPrereq() const
{
	return m_iVictoryPrereq;
}

int CvBuildingInfo::getFreeStartEra() const				
{
	return m_iFreeStartEra;
}

int CvBuildingInfo::getMaxStartEra() const				
{
	return m_iMaxStartEra;
}

int CvBuildingInfo::getObsoleteTech() const			
{
	return m_iObsoleteTech;
}

int CvBuildingInfo::getPrereqAndTech() const			
{
	return m_iPrereqAndTech;
}

int CvBuildingInfo::getPowerBonus() const				
{
	return m_iPowerBonus;
}

int CvBuildingInfo::getFreeBonus() const					
{
	return m_iFreeBonus;
}

int CvBuildingInfo::getNumFreeBonuses() const		
{
	return m_iNumFreeBonuses;
}

int CvBuildingInfo::getFreeBuildingClass() const			
{
	return m_iFreeBuildingClass;
}

void CvBuildingInfo::setFreeBuildingClass(int i)
{
	m_iFreeBuildingClass = i;
}

int CvBuildingInfo::getFreePromotion() const			
{
	return m_iFreePromotion;
}

int CvBuildingInfo::getCivicOption() const
{
	return m_iCivicOption;
}

int CvBuildingInfo::getAIWeight() const
{
	return m_iAIWeight;
}

int CvBuildingInfo::getProductionCost() const		
{
	return m_iProductionCost;
}

int CvBuildingInfo::getHurryCostModifier() const	
{
	return m_iHurryCostModifier;
}

int CvBuildingInfo::getHurryAngerModifier() const	
{
	return m_iHurryAngerModifier;
}

int CvBuildingInfo::getMinAreaSize() const				
{
	return m_iMinAreaSize;
}

int CvBuildingInfo::getNumCitiesPrereq() const		
{
	return m_iNumCitiesPrereq;
}

int CvBuildingInfo::getNumTeamsPrereq() const		
{
	return m_iNumTeamsPrereq;
}

int CvBuildingInfo::getUnitLevelPrereq() const		
{
	return m_iUnitLevelPrereq;
}

int CvBuildingInfo::getMinLatitude() const				
{
	return m_iMinLatitude;
}

int CvBuildingInfo::getMaxLatitude() const				
{
	return m_iMaxLatitude;
}

int CvBuildingInfo::getGreatPeopleRateModifier() const
{
	return m_iGreatPeopleRateModifier;
}

int CvBuildingInfo::getGreatGeneralRateModifier() const
{
	return m_iGreatGeneralRateModifier;
}

int CvBuildingInfo::getDomesticGreatGeneralRateModifier() const
{
	return m_iDomesticGreatGeneralRateModifier;
}

int CvBuildingInfo::getGlobalGreatPeopleRateModifier() const	
{
	return m_iGlobalGreatPeopleRateModifier;
}

int CvBuildingInfo::getAnarchyModifier() const		
{
	return m_iAnarchyModifier;
}

int CvBuildingInfo::getGlobalHurryModifier() const
{
	return m_iGlobalHurryModifier;
}

int CvBuildingInfo::getFreeExperience() const		
{
	return m_iFreeExperience;
}

int CvBuildingInfo::getGlobalFreeExperience() const
{
	return m_iGlobalFreeExperience;
}

int CvBuildingInfo::getFoodKept() const
{
	return m_iFoodKept;
}

int CvBuildingInfo::getAirlift() const
{
	return m_iAirlift;
}

int CvBuildingInfo::getAirModifier() const				
{
	return m_iAirModifier;
}

int CvBuildingInfo::getNukeModifier() const
{
	return m_iNukeModifier;
}

int CvBuildingInfo::getNukeExplosionRand() const
{
	return m_iNukeExplosionRand;
}

int CvBuildingInfo::getFreeSpecialist() const		
{
	return m_iFreeSpecialist;
}

int CvBuildingInfo::getAreaFreeSpecialist() const
{
	return m_iAreaFreeSpecialist;
}

int CvBuildingInfo::getGlobalFreeSpecialist() const
{
	return m_iGlobalFreeSpecialist;
}

int CvBuildingInfo::getHappiness() const					
{
	return m_iHappiness;
}

int CvBuildingInfo::getAreaHappiness() const			
{
	return m_iAreaHappiness;
}

int CvBuildingInfo::getGlobalHappiness() const		
{
	return m_iGlobalHappiness;
}

int CvBuildingInfo::getStateReligionHappiness() const
{
	return m_iStateReligionHappiness;
}

int CvBuildingInfo::getWorkerSpeedModifier() const
{
	return m_iWorkerSpeedModifier;
}

int CvBuildingInfo::getMilitaryProductionModifier() const		
{
	return m_iMilitaryProductionModifier;
}

int CvBuildingInfo::getSpaceProductionModifier() const
{
	return m_iSpaceProductionModifier;
}

int CvBuildingInfo::getGlobalSpaceProductionModifier() const	
{
	return m_iGlobalSpaceProductionModifier;
}

int CvBuildingInfo::getTradeRoutes() const				
{
	return m_iTradeRoutes;
}

int CvBuildingInfo::getCoastalTradeRoutes() const
{
	return m_iCoastalTradeRoutes;
}

int CvBuildingInfo::getGlobalTradeRoutes() const	
{
	return m_iGlobalTradeRoutes;
}

int CvBuildingInfo::getTradeRouteModifier() const
{
	return m_iTradeRouteModifier;
}

int CvBuildingInfo::getAssetValue() const				
{
	return m_iAssetValue;
}

int CvBuildingInfo::getPowerValue() const				
{
	return m_iPowerValue;
}

int CvBuildingInfo::getSpecialBuildingType() const
{
	return m_iSpecialBuildingType;
}

int CvBuildingInfo::getAdvisorType() const
{
	return m_iAdvisorType;
}

int CvBuildingInfo::getHolyCity() const					
{
	return m_iHolyCity;
}

int CvBuildingInfo::getReligionType() const			
{
	return m_iReligionType;
}

int CvBuildingInfo::getStateReligion() const			
{
	return m_iStateReligion;
}

int CvBuildingInfo::getPrereqReligion() const		
{
	return m_iPrereqReligion;
}

int CvBuildingInfo::getGlobalReligionCommerce() const
{
	return m_iGlobalReligionCommerce;
}

int CvBuildingInfo::getPrereqAndBonus() const		
{
	return m_iPrereqAndBonus;
}

int CvBuildingInfo::getGreatPeopleUnitClass() const
{
	return m_iGreatPeopleUnitClass;
}

int CvBuildingInfo::getGreatPeopleRateChange() const
{
	return m_iGreatPeopleRateChange;
}

int CvBuildingInfo::getConquestProbability() const
{
	return m_iConquestProbability;
}

int CvBuildingInfo::getMaintenanceModifier() const
{
	return m_iMaintenanceModifier;
}

int CvBuildingInfo::getWarWearinessModifier() const
{
	return m_iWarWearinessModifier;
}

int CvBuildingInfo::getGlobalWarWearinessModifier() const		
{
	return m_iGlobalWarWearinessModifier;
}

int CvBuildingInfo::getHealRateChange() const		
{
	return m_iHealRateChange;
}

int CvBuildingInfo::getHealth() const						
{
	return m_iHealth;
}

int CvBuildingInfo::getAreaHealth() const				
{
	return m_iAreaHealth;
}

int CvBuildingInfo::getGlobalHealth() const			
{
	return m_iGlobalHealth;
}

int CvBuildingInfo::getGlobalPopulationChange() const
{
	return m_iGlobalPopulationChange;
}

int CvBuildingInfo::getFreeTechs() const					
{
	return m_iFreeTechs;
}

int CvBuildingInfo::getDefenseModifier() const	
{
	return m_iDefenseModifier;
}

int CvBuildingInfo::getBombardDefenseModifier() const	
{
	return m_iBombardDefenseModifier;
}

int CvBuildingInfo::getAllCityDefenseModifier() const			
{
	return m_iAllCityDefenseModifier;
}

int CvBuildingInfo::getMissionType() const
{
	return m_iMissionType;
}

void CvBuildingInfo::setMissionType(int iNewType)
{
	m_iMissionType = iNewType;
}

float CvBuildingInfo::getVisibilityPriority() const
{
	return m_fVisibilityPriority;
}

bool CvBuildingInfo::isTeamShare() const
{
	return m_bTeamShare;
}

bool CvBuildingInfo::isWater() const
{
	return m_bWater;
}

bool CvBuildingInfo::isRiver() const
{
	return m_bRiver;
}

bool CvBuildingInfo::isPower() const
{
	return m_bPower;
}

bool CvBuildingInfo::isDirtyPower() const
{
	return m_bDirtyPower;
}

bool CvBuildingInfo::isAreaCleanPower() const
{
	return m_bAreaCleanPower;
}

bool CvBuildingInfo::isAreaBorderObstacle() const
{
	return m_bAreaBorderObstacle;
}

bool CvBuildingInfo::isDiploVote() const
{
	return m_bDiploVote;
}

bool CvBuildingInfo::isForceTeamVoteEligible() const
{
	return m_bForceTeamVoteEligible;
}

bool CvBuildingInfo::isCapital() const					
{
	return m_bCapital;
}

bool CvBuildingInfo::isGovernmentCenter() const	
{
	return m_bGovernmentCenter;
}

bool CvBuildingInfo::isGoldenAge() const				
{
	return m_bGoldenAge;
}

bool CvBuildingInfo::isMapCentering() const			
{
	return m_bMapCentering;
}

bool CvBuildingInfo::isNoUnhappiness() const		
{
	return m_bNoUnhappiness;
}

bool CvBuildingInfo::isNoUnhealthyPopulation() const
{
	return m_bNoUnhealthyPopulation;
}

bool CvBuildingInfo::isBuildingOnlyHealthy() const
{
	return m_bBuildingOnlyHealthy;
}

bool CvBuildingInfo::isNeverCapture() const
{
	return m_bNeverCapture;
}

bool CvBuildingInfo::isNukeImmune() const
{
	return m_bNukeImmune;
}

bool CvBuildingInfo::isPrereqReligion() const
{
	return m_bPrereqReligion;
}

bool CvBuildingInfo::isCenterInCity() const
{
	return m_bCenterInCity;
}

const TCHAR* CvBuildingInfo::getConstructSound() const
{
	return m_szConstructSound;
}

void CvBuildingInfo::setConstructSound(const TCHAR* szVal)
{
	m_szConstructSound = szVal;
}

const TCHAR* CvBuildingInfo::getArtDefineTag() const
{
	return m_szArtDefineTag;
}

void CvBuildingInfo::setArtDefineTag(const TCHAR* szVal)
{
	m_szArtDefineTag = szVal;
}

const TCHAR* CvBuildingInfo::getMovieDefineTag() const
{
	return m_szMovieDefineTag;
}

void CvBuildingInfo::setMovieDefineTag(const TCHAR* szVal)
{
	m_szMovieDefineTag = szVal;
}

// Arrays

int CvBuildingInfo::getYieldChange(int i) const
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piYieldChange ? m_piYieldChange[i] : -1;
}

int* CvBuildingInfo::getYieldChangeArray() const
{
	return m_piYieldChange;
}

int CvBuildingInfo::getYieldModifier(int i) const
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piYieldModifier ? m_piYieldModifier[i] : -1;
}

int* CvBuildingInfo::getYieldModifierArray() const
{
	return m_piYieldModifier;
}

int CvBuildingInfo::getPowerYieldModifier(int i) const	
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piPowerYieldModifier ? m_piPowerYieldModifier[i] : -1;
}

int* CvBuildingInfo::getPowerYieldModifierArray() const
{
	return m_piPowerYieldModifier;
}

int CvBuildingInfo::getAreaYieldModifier(int i) const	
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piAreaYieldModifier ? m_piAreaYieldModifier[i] : -1;
}

int* CvBuildingInfo::getAreaYieldModifierArray() const
{
	return m_piAreaYieldModifier;
}

int CvBuildingInfo::getGlobalYieldModifier(int i) const
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piGlobalYieldModifier ? m_piGlobalYieldModifier[i] : -1;
}

int* CvBuildingInfo::getGlobalYieldModifierArray() const
{
	return m_piGlobalYieldModifier;
}

int CvBuildingInfo::getSeaPlotYieldChange(int i) const
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piSeaPlotYieldChange ? m_piSeaPlotYieldChange[i] : -1;
}

int* CvBuildingInfo::getSeaPlotYieldChangeArray() const
{
	return m_piSeaPlotYieldChange;
}

int CvBuildingInfo::getGlobalSeaPlotYieldChange(int i) const
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piGlobalSeaPlotYieldChange ? m_piGlobalSeaPlotYieldChange[i] : -1;
}

int* CvBuildingInfo::getGlobalSeaPlotYieldChangeArray() const
{
	return m_piGlobalSeaPlotYieldChange;
}

int CvBuildingInfo::getCommerceChange(int i) const			
{
	FAssertMsg(i < NUM_COMMERCE_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piCommerceChange ? m_piCommerceChange[i] : -1;
}

int* CvBuildingInfo::getCommerceChangeArray() const
{
	return m_piCommerceChange;
}

int CvBuildingInfo::getObsoleteSafeCommerceChange(int i) const
{
	FAssertMsg(i < NUM_COMMERCE_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piObsoleteSafeCommerceChange ? m_piObsoleteSafeCommerceChange[i] : -1;
}

int* CvBuildingInfo::getObsoleteSafeCommerceChangeArray() const
{
	return m_piObsoleteSafeCommerceChange;
}

int CvBuildingInfo::getCommerceChangeDoubleTime(int i) const
{
	FAssertMsg(i < NUM_COMMERCE_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piCommerceChangeDoubleTime ? m_piCommerceChangeDoubleTime[i] : -1;
}

int CvBuildingInfo::getCommerceModifier(int i) const	
{
	FAssertMsg(i < NUM_COMMERCE_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piCommerceModifier ? m_piCommerceModifier[i] : -1;
}

int* CvBuildingInfo::getCommerceModifierArray() const
{
	return m_piCommerceModifier;
}

int CvBuildingInfo::getGlobalCommerceModifier(int i) const
{
	FAssertMsg(i < NUM_COMMERCE_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piGlobalCommerceModifier ? m_piGlobalCommerceModifier[i] : -1;
}

int* CvBuildingInfo::getGlobalCommerceModifierArray() const
{
	return m_piGlobalCommerceModifier;
}

int CvBuildingInfo::getSpecialistExtraCommerce(int i) const
{
	FAssertMsg(i < NUM_COMMERCE_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piSpecialistExtraCommerce ? m_piSpecialistExtraCommerce[i] : -1;
}

int* CvBuildingInfo::getSpecialistExtraCommerceArray() const
{
	return m_piSpecialistExtraCommerce;
}

int CvBuildingInfo::getStateReligionCommerce(int i) const
{
	FAssertMsg(i < NUM_COMMERCE_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piStateReligionCommerce ? m_piStateReligionCommerce[i] : -1;
}

int* CvBuildingInfo::getStateReligionCommerceArray() const
{
	return m_piStateReligionCommerce;
}

int CvBuildingInfo::getCommerceHappiness(int i) const	
{
	FAssertMsg(i < NUM_COMMERCE_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piCommerceHappiness ? m_piCommerceHappiness[i] : -1;
}

int CvBuildingInfo::getReligionChange(int i) const		
{
	FAssertMsg(i < GC.getNumReligionInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piReligionChange ? m_piReligionChange[i] : -1;
}

int CvBuildingInfo::getSpecialistCount(int i) const		
{
	FAssertMsg(i < GC.getNumSpecialistInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piSpecialistCount ? m_piSpecialistCount[i] : -1;
}

int CvBuildingInfo::getFreeSpecialistCount(int i) const	
{
	FAssertMsg(i < GC.getNumSpecialistInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piFreeSpecialistCount ? m_piFreeSpecialistCount[i] : -1;
}

int CvBuildingInfo::getBonusHealthChanges(int i) const
{
	FAssertMsg(i < GC.getNumBonusInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piBonusHealthChanges ? m_piBonusHealthChanges[i] : -1;
}

int CvBuildingInfo::getBonusHappinessChanges(int i) const
{
	FAssertMsg(i < GC.getNumBonusInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piBonusHappinessChanges ? m_piBonusHappinessChanges[i] : -1;
}

int CvBuildingInfo::getBonusProductionModifier(int i) const
{
	FAssertMsg(i < GC.getNumBonusInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piBonusProductionModifier ? m_piBonusProductionModifier[i] : -1;
}

int CvBuildingInfo::getUnitCombatFreeExperience(int i) const
{
	FAssertMsg(i < GC.getNumUnitCombatInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piUnitCombatFreeExperience ? m_piUnitCombatFreeExperience[i] : -1;
}

int CvBuildingInfo::getDomainFreeExperience(int i) const
{
	FAssertMsg(i < NUM_DOMAIN_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piDomainFreeExperience ? m_piDomainFreeExperience[i] : -1;
}

int CvBuildingInfo::getDomainProductionModifier(int i) const
{
	FAssertMsg(i < NUM_DOMAIN_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piDomainProductionModifier ? m_piDomainProductionModifier[i] : -1;
}

int CvBuildingInfo::getPrereqAndTechs(int i) const
{
	FAssertMsg(i < GC.getDefineINT("NUM_BUILDING_AND_TECH_PREREQS"), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piPrereqAndTechs ? m_piPrereqAndTechs[i] : -1;
}

int CvBuildingInfo::getPrereqOrBonuses(int i) const		
{
	FAssertMsg(i < GC.getNUM_BUILDING_PREREQ_OR_BONUSES(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piPrereqOrBonuses ? m_piPrereqOrBonuses[i] : -1;
}

int CvBuildingInfo::getProductionTraits(int i) const		
{
	FAssertMsg(i < GC.getNumTraitInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piProductionTraits ? m_piProductionTraits[i] : -1;
}

int CvBuildingInfo::getHappinessTraits(int i) const		
{
	FAssertMsg(i < GC.getNumTraitInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piHappinessTraits ? m_piHappinessTraits[i] : -1;
}

int CvBuildingInfo::getBuildingHappinessChanges(int i) const
{
	FAssertMsg(i < GC.getNumBuildingInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piBuildingHappinessChanges ? m_piBuildingHappinessChanges[i] : -1;
}

int CvBuildingInfo::getPrereqNumOfBuildingClass(int i) const
{
	FAssertMsg(i < GC.getNumBuildingClassInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piPrereqNumOfBuildingClass ? m_piPrereqNumOfBuildingClass[i] : -1;
}

int CvBuildingInfo::getFlavorValue(int i) const
{
	FAssertMsg(i < GC.getNumFlavorTypes(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piFlavorValue ? m_piFlavorValue[i] : -1;
}

bool CvBuildingInfo::isCommerceFlexible(int i) const
{
	FAssertMsg(i < NUM_COMMERCE_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbCommerceFlexible ? m_pbCommerceFlexible[i] : false;
}

bool CvBuildingInfo::isCommerceChangeOriginalOwner(int i) const
{
	FAssertMsg(i < NUM_COMMERCE_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbCommerceChangeOriginalOwner ? m_pbCommerceChangeOriginalOwner[i] : false;
}

bool CvBuildingInfo::isBuildingClassNeededInCity(int i) const
{
	FAssertMsg(i < GC.getNumBuildingClassInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbBuildingClassNeededInCity ? m_pbBuildingClassNeededInCity[i] : false;
}

int CvBuildingInfo::getSpecialistYieldChange(int i, int j) const
{
	FAssertMsg(i < GC.getNumSpecialistInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	FAssertMsg(j < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(j > -1, "Index out of bounds");
	return m_ppaiSpecialistYieldChange ? m_ppaiSpecialistYieldChange[i][j] : -1;
}

int* CvBuildingInfo::getSpecialistYieldChangeArray(int i) const
{
	FAssertMsg(i < GC.getNumSpecialistInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_ppaiSpecialistYieldChange[i];
}

int CvBuildingInfo::getBonusYieldModifier(int i, int j) const
{
	FAssertMsg(i < GC.getNumBonusInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	FAssertMsg(j < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(j > -1, "Index out of bounds");
	return m_ppaiBonusYieldModifier ? m_ppaiBonusYieldModifier[i][j] : -1;
}

int* CvBuildingInfo::getBonusYieldModifierArray(int i) const
{
	FAssertMsg(i < GC.getNumBonusInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_ppaiBonusYieldModifier[i];
}

const TCHAR* CvBuildingInfo::getButton() const
{
	const CvArtInfoBuilding * pBuildingArtInfo;
	pBuildingArtInfo = getArtInfo();
	if (pBuildingArtInfo != NULL)
	{
		return pBuildingArtInfo->getButton();
	}
	else
	{
		return NULL;
	}
}

const CvArtInfoBuilding* CvBuildingInfo::getArtInfo() const
{
	return ARTFILEMGR.getBuildingArtInfo(getArtDefineTag());
}

const CvArtInfoMovie* CvBuildingInfo::getMovieInfo() const
{
	return ARTFILEMGR.getMovieArtInfo(getMovieDefineTag());
}

const TCHAR* CvBuildingInfo::getMovie() const
{
	const CvArtInfoMovie* pArt;
	pArt = getMovieInfo();
	if (pArt != NULL)
	{
		return pArt->getPath();
	}
	else
	{
		return NULL;
	}
}


//
// serialization
//
void CvBuildingInfo::read(FDataStreamBase* stream)
{
	CvHotkeyInfo::read(stream);

	uint uiFlag=0;
	stream->Read(&uiFlag);	// flags for expansion

	stream->Read(&m_iBuildingClassType);
	stream->Read(&m_iVictoryPrereq);
	stream->Read(&m_iFreeStartEra);
	stream->Read(&m_iMaxStartEra);
	stream->Read(&m_iObsoleteTech);
	stream->Read(&m_iPrereqAndTech);
	stream->Read(&m_iPowerBonus);
	stream->Read(&m_iFreeBonus);
	stream->Read(&m_iNumFreeBonuses);
	stream->Read(&m_iFreeBuildingClass);
	stream->Read(&m_iFreePromotion);
	stream->Read(&m_iCivicOption);
	stream->Read(&m_iAIWeight);
	stream->Read(&m_iProductionCost);
	stream->Read(&m_iHurryCostModifier);
	stream->Read(&m_iHurryAngerModifier);
	stream->Read(&m_iMinAreaSize);
	stream->Read(&m_iNumCitiesPrereq);
	stream->Read(&m_iNumTeamsPrereq);
	stream->Read(&m_iUnitLevelPrereq);
	stream->Read(&m_iMinLatitude);
	stream->Read(&m_iMaxLatitude);
	stream->Read(&m_iGreatPeopleRateModifier);
	stream->Read(&m_iGreatGeneralRateModifier);
	stream->Read(&m_iDomesticGreatGeneralRateModifier);
	stream->Read(&m_iGlobalGreatPeopleRateModifier);
	stream->Read(&m_iAnarchyModifier);
	stream->Read(&m_iGlobalHurryModifier);
	stream->Read(&m_iFreeExperience);
	stream->Read(&m_iGlobalFreeExperience);
	stream->Read(&m_iFoodKept);
	stream->Read(&m_iAirlift);
	stream->Read(&m_iAirModifier);
	stream->Read(&m_iNukeModifier);
	stream->Read(&m_iNukeExplosionRand);
	stream->Read(&m_iFreeSpecialist);
	stream->Read(&m_iAreaFreeSpecialist);
	stream->Read(&m_iGlobalFreeSpecialist);
	stream->Read(&m_iHappiness);
	stream->Read(&m_iAreaHappiness);
	stream->Read(&m_iGlobalHappiness);
	stream->Read(&m_iStateReligionHappiness);
	stream->Read(&m_iWorkerSpeedModifier);
	stream->Read(&m_iMilitaryProductionModifier);
	stream->Read(&m_iSpaceProductionModifier);
	stream->Read(&m_iGlobalSpaceProductionModifier);
	stream->Read(&m_iTradeRoutes);
	stream->Read(&m_iCoastalTradeRoutes);
	stream->Read(&m_iGlobalTradeRoutes);
	stream->Read(&m_iTradeRouteModifier);
	stream->Read(&m_iAssetValue);
	stream->Read(&m_iPowerValue);
	stream->Read(&m_iSpecialBuildingType);
	stream->Read(&m_iAdvisorType);
	stream->Read(&m_iHolyCity);
	stream->Read(&m_iReligionType);
	stream->Read(&m_iStateReligion);
	stream->Read(&m_iPrereqReligion);
	stream->Read(&m_iGlobalReligionCommerce);
	stream->Read(&m_iPrereqAndBonus);
	stream->Read(&m_iGreatPeopleUnitClass);
	stream->Read(&m_iGreatPeopleRateChange);
	stream->Read(&m_iConquestProbability);
	stream->Read(&m_iMaintenanceModifier);
	stream->Read(&m_iWarWearinessModifier);
	stream->Read(&m_iGlobalWarWearinessModifier);
	stream->Read(&m_iHealRateChange);
	stream->Read(&m_iHealth);
	stream->Read(&m_iAreaHealth);
	stream->Read(&m_iGlobalHealth);
	stream->Read(&m_iGlobalPopulationChange);
	stream->Read(&m_iFreeTechs);
	stream->Read(&m_iDefenseModifier);
	stream->Read(&m_iBombardDefenseModifier);
	stream->Read(&m_iAllCityDefenseModifier);
	stream->Read(&m_iMissionType);

	stream->Read(&m_fVisibilityPriority);

	stream->Read(&m_bTeamShare);
	stream->Read(&m_bWater);
	stream->Read(&m_bRiver);
	stream->Read(&m_bPower);
	stream->Read(&m_bDirtyPower);
	stream->Read(&m_bAreaCleanPower);
	stream->Read(&m_bAreaBorderObstacle);
	stream->Read(&m_bDiploVote);
	stream->Read(&m_bForceTeamVoteEligible);
	stream->Read(&m_bCapital);
	stream->Read(&m_bGovernmentCenter);
	stream->Read(&m_bGoldenAge);
	stream->Read(&m_bMapCentering);
	stream->Read(&m_bNoUnhappiness);
	stream->Read(&m_bNoUnhealthyPopulation);
	stream->Read(&m_bBuildingOnlyHealthy);
	stream->Read(&m_bNeverCapture);
	stream->Read(&m_bNukeImmune);
	stream->Read(&m_bPrereqReligion);
	stream->Read(&m_bCenterInCity);

	stream->ReadString(m_szConstructSound);
	stream->ReadString(m_szArtDefineTag);
	stream->ReadString(m_szMovieDefineTag);

	SAFE_DELETE_ARRAY(m_piPrereqAndTechs);
	m_piPrereqAndTechs = new int[GC.getDefineINT("NUM_BUILDING_AND_TECH_PREREQS")];
	stream->Read(GC.getDefineINT("NUM_BUILDING_AND_TECH_PREREQS"), m_piPrereqAndTechs);

	SAFE_DELETE_ARRAY(m_piPrereqOrBonuses);
	m_piPrereqOrBonuses = new int[GC.getNUM_BUILDING_PREREQ_OR_BONUSES()];
	stream->Read(GC.getNUM_BUILDING_PREREQ_OR_BONUSES(), m_piPrereqOrBonuses);

	SAFE_DELETE_ARRAY(m_piProductionTraits);
	m_piProductionTraits = new int[GC.getNumTraitInfos()];
	stream->Read(GC.getNumTraitInfos(), m_piProductionTraits);

	SAFE_DELETE_ARRAY(m_piHappinessTraits);
	m_piHappinessTraits = new int[GC.getNumTraitInfos()];
	stream->Read(GC.getNumTraitInfos(), m_piHappinessTraits);

	SAFE_DELETE_ARRAY(m_piSeaPlotYieldChange);
	m_piSeaPlotYieldChange = new int[NUM_YIELD_TYPES];
	stream->Read(NUM_YIELD_TYPES, m_piSeaPlotYieldChange);

	SAFE_DELETE_ARRAY(m_piGlobalSeaPlotYieldChange);
	m_piGlobalSeaPlotYieldChange = new int[NUM_YIELD_TYPES];
	stream->Read(NUM_YIELD_TYPES, m_piGlobalSeaPlotYieldChange);

	SAFE_DELETE_ARRAY(m_piYieldChange);
	m_piYieldChange = new int[NUM_YIELD_TYPES];
	stream->Read(NUM_YIELD_TYPES, m_piYieldChange);

	SAFE_DELETE_ARRAY(m_piYieldModifier);
	m_piYieldModifier = new int[NUM_YIELD_TYPES];
	stream->Read(NUM_YIELD_TYPES, m_piYieldModifier);

	SAFE_DELETE_ARRAY(m_piPowerYieldModifier);
	m_piPowerYieldModifier = new int[NUM_YIELD_TYPES];
	stream->Read(NUM_YIELD_TYPES, m_piPowerYieldModifier);

	SAFE_DELETE_ARRAY(m_piAreaYieldModifier);
	m_piAreaYieldModifier = new int[NUM_YIELD_TYPES];
	stream->Read(NUM_YIELD_TYPES, m_piAreaYieldModifier);

	SAFE_DELETE_ARRAY(m_piGlobalYieldModifier);
	m_piGlobalYieldModifier = new int[NUM_YIELD_TYPES];
	stream->Read(NUM_YIELD_TYPES, m_piGlobalYieldModifier);

	SAFE_DELETE_ARRAY(m_piCommerceChange);
	m_piCommerceChange = new int[NUM_COMMERCE_TYPES];
	stream->Read(NUM_COMMERCE_TYPES, m_piCommerceChange);

	SAFE_DELETE_ARRAY(m_piObsoleteSafeCommerceChange);
	m_piObsoleteSafeCommerceChange = new int[NUM_COMMERCE_TYPES];
	stream->Read(NUM_COMMERCE_TYPES, m_piObsoleteSafeCommerceChange);

	SAFE_DELETE_ARRAY(m_piCommerceChangeDoubleTime);
	m_piCommerceChangeDoubleTime = new int[NUM_COMMERCE_TYPES];
	stream->Read(NUM_COMMERCE_TYPES, m_piCommerceChangeDoubleTime);

	SAFE_DELETE_ARRAY(m_piCommerceModifier);
	m_piCommerceModifier = new int[NUM_COMMERCE_TYPES];
	stream->Read(NUM_COMMERCE_TYPES, m_piCommerceModifier);

	SAFE_DELETE_ARRAY(m_piGlobalCommerceModifier);
	m_piGlobalCommerceModifier = new int[NUM_COMMERCE_TYPES];
	stream->Read(NUM_COMMERCE_TYPES, m_piGlobalCommerceModifier);

	SAFE_DELETE_ARRAY(m_piSpecialistExtraCommerce);
	m_piSpecialistExtraCommerce = new int[NUM_COMMERCE_TYPES];
	stream->Read(NUM_COMMERCE_TYPES, m_piSpecialistExtraCommerce);

	SAFE_DELETE_ARRAY(m_piStateReligionCommerce);
	m_piStateReligionCommerce = new int[NUM_COMMERCE_TYPES];
	stream->Read(NUM_COMMERCE_TYPES, m_piStateReligionCommerce);

	SAFE_DELETE_ARRAY(m_piCommerceHappiness);
	m_piCommerceHappiness = new int[NUM_COMMERCE_TYPES];
	stream->Read(NUM_COMMERCE_TYPES, m_piCommerceHappiness);

	SAFE_DELETE_ARRAY(m_piReligionChange);
	m_piReligionChange = new int[GC.getNumReligionInfos()];
	stream->Read(GC.getNumReligionInfos(), m_piReligionChange);

	SAFE_DELETE_ARRAY(m_piSpecialistCount);
	m_piSpecialistCount = new int[GC.getNumSpecialistInfos()];
	stream->Read(GC.getNumSpecialistInfos(), m_piSpecialistCount);

	SAFE_DELETE_ARRAY(m_piFreeSpecialistCount);
	m_piFreeSpecialistCount = new int[GC.getNumSpecialistInfos()];
	stream->Read(GC.getNumSpecialistInfos(), m_piFreeSpecialistCount);

	SAFE_DELETE_ARRAY(m_piBonusHealthChanges);
	m_piBonusHealthChanges = new int[GC.getNumBonusInfos()];
	stream->Read(GC.getNumBonusInfos(), m_piBonusHealthChanges);

	SAFE_DELETE_ARRAY(m_piBonusHappinessChanges);
	m_piBonusHappinessChanges = new int[GC.getNumBonusInfos()];
	stream->Read(GC.getNumBonusInfos(), m_piBonusHappinessChanges);

	SAFE_DELETE_ARRAY(m_piBonusProductionModifier);
	m_piBonusProductionModifier = new int[GC.getNumBonusInfos()];
	stream->Read(GC.getNumBonusInfos(), m_piBonusProductionModifier);

	SAFE_DELETE_ARRAY(m_piUnitCombatFreeExperience);
	m_piUnitCombatFreeExperience = new int[GC.getNumUnitCombatInfos()];
	stream->Read(GC.getNumUnitCombatInfos(), m_piUnitCombatFreeExperience);

	SAFE_DELETE_ARRAY(m_piDomainFreeExperience);
	m_piDomainFreeExperience = new int[NUM_DOMAIN_TYPES];
	stream->Read(NUM_DOMAIN_TYPES, m_piDomainFreeExperience);

	SAFE_DELETE_ARRAY(m_piDomainProductionModifier);
	m_piDomainProductionModifier = new int[NUM_DOMAIN_TYPES];
	stream->Read(NUM_DOMAIN_TYPES, m_piDomainProductionModifier);

	SAFE_DELETE_ARRAY(m_piBuildingHappinessChanges);
	m_piBuildingHappinessChanges = new int[GC.getNumBuildingInfos()];
	stream->Read(GC.getNumBuildingInfos(), m_piBuildingHappinessChanges);

	SAFE_DELETE_ARRAY(m_piPrereqNumOfBuildingClass);
	m_piPrereqNumOfBuildingClass = new int[GC.getNumBuildingClassInfos()];
	stream->Read(GC.getNumBuildingClassInfos(), m_piPrereqNumOfBuildingClass);

	SAFE_DELETE_ARRAY(m_piFlavorValue);
	m_piFlavorValue = new int[GC.getNumFlavorTypes()];
	stream->Read(GC.getNumFlavorTypes(), m_piFlavorValue);

	SAFE_DELETE_ARRAY(m_pbCommerceFlexible);
	m_pbCommerceFlexible = new bool[NUM_COMMERCE_TYPES];
	stream->Read(NUM_COMMERCE_TYPES, m_pbCommerceFlexible);

	SAFE_DELETE_ARRAY(m_pbCommerceChangeOriginalOwner);
	m_pbCommerceChangeOriginalOwner = new bool[NUM_COMMERCE_TYPES];
	stream->Read(NUM_COMMERCE_TYPES, m_pbCommerceChangeOriginalOwner);

	SAFE_DELETE_ARRAY(m_pbBuildingClassNeededInCity);
	m_pbBuildingClassNeededInCity = new bool[GC.getNumBuildingClassInfos()];
	stream->Read(GC.getNumBuildingClassInfos(), m_pbBuildingClassNeededInCity);

	int i;
	if (m_ppaiSpecialistYieldChange != NULL)
	{
		for(i=0;i<GC.getNumSpecialistInfos();i++)
		{
			SAFE_DELETE_ARRAY(m_ppaiSpecialistYieldChange[i]);
		}
		SAFE_DELETE_ARRAY(m_ppaiSpecialistYieldChange);
	}

	m_ppaiSpecialistYieldChange = new int*[GC.getNumSpecialistInfos()];
	for(i=0;i<GC.getNumSpecialistInfos();i++)
	{
		m_ppaiSpecialistYieldChange[i]  = new int[NUM_YIELD_TYPES];
		stream->Read(NUM_YIELD_TYPES, m_ppaiSpecialistYieldChange[i]);
	}

	if (m_ppaiBonusYieldModifier != NULL)
	{
		for(i=0;i<GC.getNumBonusInfos();i++)
		{
			SAFE_DELETE_ARRAY(m_ppaiBonusYieldModifier[i]);
		}
		SAFE_DELETE_ARRAY(m_ppaiBonusYieldModifier);
	}

	m_ppaiBonusYieldModifier = new int*[GC.getNumBonusInfos()];
	for(i=0;i<GC.getNumBonusInfos();i++)
	{
		m_ppaiBonusYieldModifier[i]  = new int[NUM_YIELD_TYPES];
		stream->Read(NUM_YIELD_TYPES, m_ppaiBonusYieldModifier[i]);
	}
}

//
// serialization
//
void CvBuildingInfo::write(FDataStreamBase* stream)
{
	CvHotkeyInfo::write(stream);

	uint uiFlag=0;
	stream->Write(uiFlag);		// flag for expansion

	stream->Write(m_iBuildingClassType);
	stream->Write(m_iVictoryPrereq);
	stream->Write(m_iFreeStartEra);
	stream->Write(m_iMaxStartEra);
	stream->Write(m_iObsoleteTech);
	stream->Write(m_iPrereqAndTech);
	stream->Write(m_iPowerBonus);
	stream->Write(m_iFreeBonus);
	stream->Write(m_iNumFreeBonuses);
	stream->Write(m_iFreeBuildingClass);
	stream->Write(m_iFreePromotion);
	stream->Write(m_iCivicOption);
	stream->Write(m_iAIWeight);
	stream->Write(m_iProductionCost);
	stream->Write(m_iHurryCostModifier);
	stream->Write(m_iHurryAngerModifier);
	stream->Write(m_iMinAreaSize);
	stream->Write(m_iNumCitiesPrereq);
	stream->Write(m_iNumTeamsPrereq);
	stream->Write(m_iUnitLevelPrereq);
	stream->Write(m_iMinLatitude);
	stream->Write(m_iMaxLatitude);
	stream->Write(m_iGreatPeopleRateModifier);
	stream->Write(m_iGreatGeneralRateModifier);
	stream->Write(m_iDomesticGreatGeneralRateModifier);
	stream->Write(m_iGlobalGreatPeopleRateModifier);
	stream->Write(m_iAnarchyModifier);
	stream->Write(m_iGlobalHurryModifier);
	stream->Write(m_iFreeExperience);
	stream->Write(m_iGlobalFreeExperience);
	stream->Write(m_iFoodKept);
	stream->Write(m_iAirlift);
	stream->Write(m_iAirModifier);
	stream->Write(m_iNukeModifier);
	stream->Write(m_iNukeExplosionRand);
	stream->Write(m_iFreeSpecialist);
	stream->Write(m_iAreaFreeSpecialist);
	stream->Write(m_iGlobalFreeSpecialist);
	stream->Write(m_iHappiness);
	stream->Write(m_iAreaHappiness);
	stream->Write(m_iGlobalHappiness);
	stream->Write(m_iStateReligionHappiness);
	stream->Write(m_iWorkerSpeedModifier);
	stream->Write(m_iMilitaryProductionModifier);
	stream->Write(m_iSpaceProductionModifier);
	stream->Write(m_iGlobalSpaceProductionModifier);
	stream->Write(m_iTradeRoutes);
	stream->Write(m_iCoastalTradeRoutes);
	stream->Write(m_iGlobalTradeRoutes);
	stream->Write(m_iTradeRouteModifier);
	stream->Write(m_iAssetValue);
	stream->Write(m_iPowerValue);
	stream->Write(m_iSpecialBuildingType);
	stream->Write(m_iAdvisorType);
	stream->Write(m_iHolyCity);
	stream->Write(m_iReligionType);
	stream->Write(m_iStateReligion);
	stream->Write(m_iPrereqReligion);
	stream->Write(m_iGlobalReligionCommerce);
	stream->Write(m_iPrereqAndBonus);
	stream->Write(m_iGreatPeopleUnitClass);
	stream->Write(m_iGreatPeopleRateChange);
	stream->Write(m_iConquestProbability);
	stream->Write(m_iMaintenanceModifier);
	stream->Write(m_iWarWearinessModifier);
	stream->Write(m_iGlobalWarWearinessModifier);
	stream->Write(m_iHealRateChange);
	stream->Write(m_iHealth);
	stream->Write(m_iAreaHealth);
	stream->Write(m_iGlobalHealth);
	stream->Write(m_iGlobalPopulationChange);
	stream->Write(m_iFreeTechs);
	stream->Write(m_iDefenseModifier);
	stream->Write(m_iBombardDefenseModifier);
	stream->Write(m_iAllCityDefenseModifier);
	stream->Write(m_iMissionType);

	stream->Write(m_fVisibilityPriority);

	stream->Write(m_bTeamShare);
	stream->Write(m_bWater);
	stream->Write(m_bRiver);
	stream->Write(m_bPower);
	stream->Write(m_bDirtyPower);
	stream->Write(m_bAreaCleanPower);
	stream->Write(m_bAreaBorderObstacle);
	stream->Write(m_bDiploVote);
	stream->Write(m_bForceTeamVoteEligible);
	stream->Write(m_bCapital);
	stream->Write(m_bGovernmentCenter);
	stream->Write(m_bGoldenAge);
	stream->Write(m_bMapCentering);
	stream->Write(m_bNoUnhappiness);
	stream->Write(m_bNoUnhealthyPopulation);
	stream->Write(m_bBuildingOnlyHealthy);
	stream->Write(m_bNeverCapture);
	stream->Write(m_bNukeImmune);
	stream->Write(m_bPrereqReligion);
	stream->Write(m_bCenterInCity);

	stream->WriteString(m_szConstructSound);
	stream->WriteString(m_szArtDefineTag);
	stream->WriteString(m_szMovieDefineTag);

	stream->Write(GC.getDefineINT("NUM_BUILDING_AND_TECH_PREREQS"), m_piPrereqAndTechs);
	stream->Write(GC.getNUM_BUILDING_PREREQ_OR_BONUSES(), m_piPrereqOrBonuses);
	stream->Write(GC.getNumTraitInfos(), m_piProductionTraits);
	stream->Write(GC.getNumTraitInfos(), m_piHappinessTraits);
	stream->Write(NUM_YIELD_TYPES, m_piSeaPlotYieldChange);
	stream->Write(NUM_YIELD_TYPES, m_piGlobalSeaPlotYieldChange);
	stream->Write(NUM_YIELD_TYPES, m_piYieldChange);
	stream->Write(NUM_YIELD_TYPES, m_piYieldModifier);
	stream->Write(NUM_YIELD_TYPES, m_piPowerYieldModifier);
	stream->Write(NUM_YIELD_TYPES, m_piAreaYieldModifier);
	stream->Write(NUM_YIELD_TYPES, m_piGlobalYieldModifier);
	stream->Write(NUM_COMMERCE_TYPES, m_piCommerceChange);
	stream->Write(NUM_COMMERCE_TYPES, m_piObsoleteSafeCommerceChange);
	stream->Write(NUM_COMMERCE_TYPES, m_piCommerceChangeDoubleTime);
	stream->Write(NUM_COMMERCE_TYPES, m_piCommerceModifier);
	stream->Write(NUM_COMMERCE_TYPES, m_piGlobalCommerceModifier);
	stream->Write(NUM_COMMERCE_TYPES, m_piSpecialistExtraCommerce);
	stream->Write(NUM_COMMERCE_TYPES, m_piStateReligionCommerce);
	stream->Write(NUM_COMMERCE_TYPES, m_piCommerceHappiness);
	stream->Write(GC.getNumReligionInfos(), m_piReligionChange);
	stream->Write(GC.getNumSpecialistInfos(), m_piSpecialistCount);
	stream->Write(GC.getNumSpecialistInfos(), m_piFreeSpecialistCount);
	stream->Write(GC.getNumBonusInfos(), m_piBonusHealthChanges);
	stream->Write(GC.getNumBonusInfos(), m_piBonusHappinessChanges);
	stream->Write(GC.getNumBonusInfos(), m_piBonusProductionModifier);
	stream->Write(GC.getNumUnitCombatInfos(), m_piUnitCombatFreeExperience);
	stream->Write(NUM_DOMAIN_TYPES, m_piDomainFreeExperience);
	stream->Write(NUM_DOMAIN_TYPES, m_piDomainProductionModifier);
	stream->Write(GC.getNumBuildingInfos(), m_piBuildingHappinessChanges);
	stream->Write(GC.getNumBuildingClassInfos(), m_piPrereqNumOfBuildingClass);
	stream->Write(GC.getNumFlavorTypes(), m_piFlavorValue);

	stream->Write(NUM_COMMERCE_TYPES, m_pbCommerceFlexible);
	stream->Write(NUM_COMMERCE_TYPES, m_pbCommerceChangeOriginalOwner);
	stream->Write(GC.getNumBuildingClassInfos(), m_pbBuildingClassNeededInCity);

	int i;
	for(i=0;i<GC.getNumSpecialistInfos();i++)
	{
		stream->Write(NUM_YIELD_TYPES, m_ppaiSpecialistYieldChange[i]);
	}

	for(i=0;i<GC.getNumBonusInfos();i++)
	{
		stream->Write(NUM_YIELD_TYPES, m_ppaiBonusYieldModifier[i]);
	}
}

//
// read from XML
//
bool CvBuildingInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvHotkeyInfo::read(pXML))
	{
		return false;
	}

	int i=0;						//loop counter
	int j=0;						//loop counter
	int k=0;						//loop counter
	int iNumSibs=0;				// the number of siblings the current xml node has
	int iNumChildren;				// the number of children the current node has

	pXML->GetChildXmlValByName(szTextVal, "BuildingClass");
	m_iBuildingClassType = pXML->FindInInfoClass(szTextVal, GC.getBuildingClassInfo(), sizeof(GC.getBuildingClassInfo((BuildingClassTypes)0)), GC.getNumBuildingClassInfos());

	pXML->GetChildXmlValByName(szTextVal, "SpecialBuildingType");
	m_iSpecialBuildingType = pXML->FindInInfoClass(szTextVal, GC.getSpecialBuildingInfo(), sizeof(GC.getSpecialBuildingInfo((SpecialBuildingTypes)0)), GC.getNumSpecialBuildingInfos());

	pXML->GetChildXmlValByName(szTextVal, "Advisor");
	m_iAdvisorType = pXML->FindInInfoClass(szTextVal, GC.getAdvisorInfo(), sizeof(GC.getAdvisorInfo((AdvisorTypes)0)), GC.getNumAdvisorInfos());

	pXML->GetChildXmlValByName(szTextVal, "ArtDefineTag");
	setArtDefineTag(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "MovieDefineTag");
	setMovieDefineTag(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "HolyCity");
	m_iHolyCity = pXML->FindInInfoClass(szTextVal, GC.getReligionInfo(), sizeof(GC.getReligionInfo((ReligionTypes)0)), GC.getNumReligionInfos());

	pXML->GetChildXmlValByName(szTextVal, "ReligionType");
	m_iReligionType = pXML->FindInInfoClass(szTextVal, GC.getReligionInfo(), sizeof(GC.getReligionInfo((ReligionTypes)0)), GC.getNumReligionInfos());

	pXML->GetChildXmlValByName(szTextVal, "StateReligion");
	m_iStateReligion = pXML->FindInInfoClass(szTextVal, GC.getReligionInfo(), sizeof(GC.getReligionInfo((ReligionTypes)0)), GC.getNumReligionInfos());

	pXML->GetChildXmlValByName(szTextVal, "PrereqReligion");
	m_iPrereqReligion = pXML->FindInInfoClass(szTextVal, GC.getReligionInfo(), sizeof(GC.getReligionInfo((ReligionTypes)0)), GC.getNumReligionInfos());

	pXML->GetChildXmlValByName(szTextVal, "GlobalReligionCommerce");
	m_iGlobalReligionCommerce = pXML->FindInInfoClass(szTextVal, GC.getReligionInfo(), sizeof(GC.getReligionInfo((ReligionTypes)0)), GC.getNumReligionInfos());

	pXML->GetChildXmlValByName(szTextVal, "VictoryPrereq");
	m_iVictoryPrereq = pXML->FindInInfoClass(szTextVal, GC.getVictoryInfo(), sizeof(GC.getVictoryInfo((VictoryTypes)0)), GC.getNumVictoryInfos());

	pXML->GetChildXmlValByName(szTextVal, "FreeStartEra");
	m_iFreeStartEra = pXML->FindInInfoClass(szTextVal, GC.getEraInfo(), sizeof(GC.getEraInfo((EraTypes)0)), GC.getNumEraInfos());

	pXML->GetChildXmlValByName(szTextVal, "MaxStartEra");
	m_iMaxStartEra = pXML->FindInInfoClass(szTextVal, GC.getEraInfo(), sizeof(GC.getEraInfo((EraTypes)0)), GC.getNumEraInfos());

	pXML->GetChildXmlValByName(szTextVal, "ObsoleteTech");
	m_iObsoleteTech = pXML->FindInInfoClass(szTextVal, GC.getTechInfo(), sizeof(GC.getTechInfo((TechTypes)0)), GC.getNumTechInfos());

	pXML->GetChildXmlValByName(szTextVal, "PrereqTech");
	m_iPrereqAndTech = pXML->FindInInfoClass(szTextVal, GC.getTechInfo(), sizeof(GC.getTechInfo((TechTypes)0)), GC.getNumTechInfos());

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"TechTypes"))
	{
		if (pXML->SkipToNextVal())
		{
			iNumSibs = gDLL->getXMLIFace()->GetNumChildren(pXML->GetXML());
			pXML->InitList(&m_piPrereqAndTechs, GC.getDefineINT("NUM_BUILDING_AND_TECH_PREREQS"), -1);

			if (0 < iNumSibs)
			{
				if (pXML->GetChildXmlVal(szTextVal))
				{
					FAssertMsg((iNumSibs <= GC.getDefineINT("NUM_BUILDING_AND_TECH_PREREQS")),"For loop iterator is greater than array size");
					for (j=0;j<iNumSibs;j++)
					{
						m_piPrereqAndTechs[j] = pXML->FindInInfoClass(szTextVal, GC.getTechInfo(), sizeof(GC.getTechInfo((TechTypes)0)), GC.getNumTechInfos());
						if (!pXML->GetNextXmlVal(szTextVal))
						{
							break;
						}
					}

					gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
				}
			}
		}

		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	pXML->GetChildXmlValByName(szTextVal, "Bonus");
	m_iPrereqAndBonus = pXML->FindInInfoClass(szTextVal, GC.getBonusInfo(), sizeof(GC.getBonusInfo((BonusTypes)0)), GC.getNumBonusInfos());

	// if we can set the current xml node to it's next sibling
	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"PrereqBonuses"))
	{
		// the next xml val in the buildinginfo tag set is the bonuses tag set
		// Skip any comments and stop at the next value we might want
		if (pXML->SkipToNextVal())
		{
			// get the total number of children the current xml node has
			iNumChildren = gDLL->getXMLIFace()->GetNumChildren(pXML->GetXML());
			pXML->InitList(&m_piPrereqOrBonuses, GC.getNUM_BUILDING_PREREQ_OR_BONUSES(), -1);

			if (0 < iNumChildren)
			{
				// if the call to the function that sets the current xml node to it's first non-comment
				// child and sets the parameter with the new node's value succeeds
				if (pXML->GetChildXmlVal(szTextVal))
				{
					FAssertMsg((iNumChildren <= GC.getNUM_BUILDING_PREREQ_OR_BONUSES()),"For loop iterator is greater than array size");
					// loop through all the siblings
					for (j=0;j<iNumChildren;j++)
					{
						// call the find in list function to return either -1 if no value is found
						// or the index in the list the match is found at
						m_piPrereqOrBonuses[j] = pXML->FindInInfoClass(szTextVal, GC.getBonusInfo(), sizeof(GC.getBonusInfo((BonusTypes)0)), GC.getNumBonusInfos());

						// if the call to the function that sets the current xml node to it's first non-comment
						// sibling and sets the parameter with the new node's value does not succeed
						// we will break out of this for loop
						if (!pXML->GetNextXmlVal(szTextVal))
						{
							break;
						}
					}

					// set the current xml node to it's parent node
					gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
				}
			}
		}

		// set the current xml node to it's parent node
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	pXML->SetVariableListTagPair(&m_piProductionTraits, "ProductionTraits", GC.getTraitInfo(), sizeof(GC.getTraitInfo((TraitTypes)0)), GC.getNumTraitInfos());

	pXML->SetVariableListTagPair(&m_piHappinessTraits, "HappinessTraits", GC.getTraitInfo(), sizeof(GC.getTraitInfo((TraitTypes)0)), GC.getNumTraitInfos());

	pXML->GetChildXmlValByName(szTextVal, "PowerBonus");
	m_iPowerBonus = pXML->FindInInfoClass(szTextVal, GC.getBonusInfo(), sizeof(GC.getBonusInfo((BonusTypes)0)), GC.getNumBonusInfos());

	pXML->GetChildXmlValByName(szTextVal, "FreeBonus");
	m_iFreeBonus = pXML->FindInInfoClass(szTextVal, GC.getBonusInfo(), sizeof(GC.getBonusInfo((BonusTypes)0)), GC.getNumBonusInfos());
	pXML->GetChildXmlValByName(&m_iNumFreeBonuses, "iNumFreeBonuses");

	pXML->GetChildXmlValByName(szTextVal, "FreeBuilding");
	m_iFreeBuildingClass = pXML->FindInInfoClass(szTextVal, GC.getBuildingClassInfo(), sizeof(GC.getBuildingClassInfo((BuildingClassTypes)0)), GC.getNumBuildingClassInfos());

	pXML->GetChildXmlValByName(szTextVal, "FreePromotion");
	m_iFreePromotion = pXML->FindInInfoClass(szTextVal, GC.getPromotionInfo(), sizeof(GC.getPromotionInfo((PromotionTypes)0)), GC.getNumPromotionInfos());

	pXML->GetChildXmlValByName(szTextVal, "CivicOption");
	m_iCivicOption = pXML->FindInInfoClass(szTextVal, GC.getCivicOptionInfo(), sizeof(GC.getCivicOptionInfo((CivicOptionTypes)0)), GC.getNumCivicOptionInfos());

	pXML->GetChildXmlValByName(szTextVal, "GreatPeopleUnitClass");
	m_iGreatPeopleUnitClass = pXML->FindInInfoClass(szTextVal, GC.getUnitClassInfo(), sizeof(GC.getUnitClassInfo((UnitClassTypes)0)), GC.getNumUnitClassInfos());

	pXML->GetChildXmlValByName(&m_iGreatPeopleRateChange, "iGreatPeopleRateChange");
	pXML->GetChildXmlValByName(&m_bTeamShare, "bTeamShare");
	pXML->GetChildXmlValByName(&m_bWater, "bWater");
	pXML->GetChildXmlValByName(&m_bRiver, "bRiver");
	pXML->GetChildXmlValByName(&m_bPower, "bPower");
	pXML->GetChildXmlValByName(&m_bDirtyPower, "bDirtyPower");
	pXML->GetChildXmlValByName(&m_bAreaCleanPower, "bAreaCleanPower");
	pXML->GetChildXmlValByName(&m_bAreaBorderObstacle, "bBorderObstacle");
	pXML->GetChildXmlValByName(&m_bDiploVote, "bDiploVote");
	pXML->GetChildXmlValByName(&m_bForceTeamVoteEligible, "bForceTeamVoteEligible");
	pXML->GetChildXmlValByName(&m_bCapital, "bCapital");
	pXML->GetChildXmlValByName(&m_bGovernmentCenter, "bGovernmentCenter");
	pXML->GetChildXmlValByName(&m_bGoldenAge, "bGoldenAge");
	pXML->GetChildXmlValByName(&m_bMapCentering, "bMapCentering");
	pXML->GetChildXmlValByName(&m_bNoUnhappiness, "bNoUnhappiness");
	pXML->GetChildXmlValByName(&m_bNoUnhealthyPopulation, "bNoUnhealthyPopulation");
	pXML->GetChildXmlValByName(&m_bBuildingOnlyHealthy, "bBuildingOnlyHealthy");
	pXML->GetChildXmlValByName(&m_bNeverCapture, "bNeverCapture");
	pXML->GetChildXmlValByName(&m_bNukeImmune, "bNukeImmune");
	pXML->GetChildXmlValByName(&m_bPrereqReligion, "bPrereqReligion");
	pXML->GetChildXmlValByName(&m_bCenterInCity, "bCenterInCity");
	pXML->GetChildXmlValByName(&m_iAIWeight, "iAIWeight");
	pXML->GetChildXmlValByName(&m_iProductionCost, "iCost");
	pXML->GetChildXmlValByName(&m_iHurryCostModifier, "iHurryCostModifier");
	pXML->GetChildXmlValByName(&m_iHurryAngerModifier, "iHurryAngerModifier");
	pXML->GetChildXmlValByName(&m_iMinAreaSize, "iMinAreaSize");
	pXML->GetChildXmlValByName(&m_iConquestProbability, "iConquestProb");
	pXML->GetChildXmlValByName(&m_iNumCitiesPrereq, "iCitiesPrereq");
	pXML->GetChildXmlValByName(&m_iNumTeamsPrereq, "iTeamsPrereq");
	pXML->GetChildXmlValByName(&m_iUnitLevelPrereq, "iLevelPrereq");
	pXML->GetChildXmlValByName(&m_iMinLatitude, "iMinLatitude");
	pXML->GetChildXmlValByName(&m_iMaxLatitude, "iMaxLatitude");
	pXML->GetChildXmlValByName(&m_iGreatPeopleRateModifier, "iGreatPeopleRateModifier");
	pXML->GetChildXmlValByName(&m_iGreatGeneralRateModifier, "iGreatGeneralRateModifier");
	pXML->GetChildXmlValByName(&m_iDomesticGreatGeneralRateModifier, "iDomesticGreatGeneralRateModifier");
	pXML->GetChildXmlValByName(&m_iGlobalGreatPeopleRateModifier, "iGlobalGreatPeopleRateModifier");
	pXML->GetChildXmlValByName(&m_iAnarchyModifier, "iAnarchyModifier");
	pXML->GetChildXmlValByName(&m_iGlobalHurryModifier, "iGlobalHurryModifier");
	pXML->GetChildXmlValByName(&m_iFreeExperience, "iExperience");
	pXML->GetChildXmlValByName(&m_iGlobalFreeExperience, "iGlobalExperience");
	pXML->GetChildXmlValByName(&m_iFoodKept, "iFoodKept");
	pXML->GetChildXmlValByName(&m_iAirlift, "iAirlift");
	pXML->GetChildXmlValByName(&m_iAirModifier, "iAirModifier");
	pXML->GetChildXmlValByName(&m_iNukeModifier, "iNukeModifier");
	pXML->GetChildXmlValByName(&m_iNukeExplosionRand, "iNukeExplosionRand");
	pXML->GetChildXmlValByName(&m_iFreeSpecialist, "iFreeSpecialist");
	pXML->GetChildXmlValByName(&m_iAreaFreeSpecialist, "iAreaFreeSpecialist");
	pXML->GetChildXmlValByName(&m_iGlobalFreeSpecialist, "iGlobalFreeSpecialist");
	pXML->GetChildXmlValByName(&m_iMaintenanceModifier, "iMaintenanceModifier");
	pXML->GetChildXmlValByName(&m_iWarWearinessModifier, "iWarWearinessModifier");
	pXML->GetChildXmlValByName(&m_iGlobalWarWearinessModifier, "iGlobalWarWearinessModifier");
	pXML->GetChildXmlValByName(&m_iHealRateChange, "iHealRateChange");
	pXML->GetChildXmlValByName(&m_iHealth, "iHealth");
	pXML->GetChildXmlValByName(&m_iAreaHealth, "iAreaHealth");
	pXML->GetChildXmlValByName(&m_iGlobalHealth, "iGlobalHealth");
	pXML->GetChildXmlValByName(&m_iHappiness, "iHappiness");
	pXML->GetChildXmlValByName(&m_iAreaHappiness, "iAreaHappiness");
	pXML->GetChildXmlValByName(&m_iGlobalHappiness, "iGlobalHappiness");
	pXML->GetChildXmlValByName(&m_iStateReligionHappiness, "iStateReligionHappiness");
	pXML->GetChildXmlValByName(&m_iWorkerSpeedModifier, "iWorkerSpeedModifier");
	pXML->GetChildXmlValByName(&m_iMilitaryProductionModifier, "iMilitaryProductionModifier");
	pXML->GetChildXmlValByName(&m_iSpaceProductionModifier, "iSpaceProductionModifier");
	pXML->GetChildXmlValByName(&m_iGlobalSpaceProductionModifier, "iGlobalSpaceProductionModifier");
	pXML->GetChildXmlValByName(&m_iTradeRoutes, "iTradeRoutes");
	pXML->GetChildXmlValByName(&m_iCoastalTradeRoutes, "iCoastalTradeRoutes");
	pXML->GetChildXmlValByName(&m_iGlobalTradeRoutes, "iGlobalTradeRoutes");
	pXML->GetChildXmlValByName(&m_iTradeRouteModifier, "iTradeRouteModifier");
	pXML->GetChildXmlValByName(&m_iGlobalPopulationChange, "iGlobalPopulationChange");
	pXML->GetChildXmlValByName(&m_iFreeTechs, "iFreeTechs");
	pXML->GetChildXmlValByName(&m_iDefenseModifier, "iDefense");
	pXML->GetChildXmlValByName(&m_iBombardDefenseModifier, "iBombardDefense");	
	pXML->GetChildXmlValByName(&m_iAllCityDefenseModifier, "iAllCityDefense");
	pXML->GetChildXmlValByName(&m_iAssetValue, "iAsset");
	pXML->GetChildXmlValByName(&m_iPowerValue, "iPower");
	pXML->GetChildXmlValByName(&m_fVisibilityPriority, "fVisibilityPriority");

	// if we can set the current xml node to it's next sibling
	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"SeaPlotYieldChanges"))
	{
		// call the function that sets the yield change variable
		pXML->SetYields(&m_piSeaPlotYieldChange);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piSeaPlotYieldChange, NUM_YIELD_TYPES);
	}

	// if we can set the current xml node to it's next sibling
	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"GlobalSeaPlotYieldChanges"))
	{
		// call the function that sets the yield change variable
		pXML->SetYields(&m_piGlobalSeaPlotYieldChange);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piGlobalSeaPlotYieldChange, NUM_YIELD_TYPES);
	}

	// if we can set the current xml node to it's next sibling
	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"YieldChanges"))
	{
		// call the function that sets the yield change variable
		pXML->SetYields(&m_piYieldChange);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piYieldChange, NUM_YIELD_TYPES);
	}

	// if we can set the current xml node to it's next sibling
	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"YieldModifiers"))
	{
		// call the function that sets the yield change variable
		pXML->SetYields(&m_piYieldModifier);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piYieldModifier, NUM_YIELD_TYPES);
	}

	// if we can set the current xml node to it's next sibling
	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"PowerYieldModifiers"))
	{
		// call the function that sets the yield change variable
		pXML->SetYields(&m_piPowerYieldModifier);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piPowerYieldModifier, NUM_YIELD_TYPES);
	}

	// if we can set the current xml node to it's next sibling
	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"AreaYieldModifiers"))
	{
		// call the function that sets the yield change variable
		pXML->SetYields(&m_piAreaYieldModifier);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piAreaYieldModifier, NUM_YIELD_TYPES);
	}

	// if we can set the current xml node to it's next sibling
	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"GlobalYieldModifiers"))
	{
		// call the function that sets the yield change variable
		pXML->SetYields(&m_piGlobalYieldModifier);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piGlobalYieldModifier, NUM_YIELD_TYPES);
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"CommerceChanges"))
	{
		pXML->SetCommerce(&m_piCommerceChange);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piCommerceChange, NUM_COMMERCE_TYPES);
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"ObsoleteSafeCommerceChanges"))
	{
		pXML->SetCommerce(&m_piObsoleteSafeCommerceChange);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piObsoleteSafeCommerceChange, NUM_COMMERCE_TYPES);
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"CommerceChangeDoubleTimes"))
	{
		pXML->SetCommerce(&m_piCommerceChangeDoubleTime);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piCommerceChangeDoubleTime, NUM_COMMERCE_TYPES);
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"CommerceModifiers"))
	{
		pXML->SetCommerce(&m_piCommerceModifier);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piCommerceModifier, NUM_COMMERCE_TYPES);
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"GlobalCommerceModifiers"))
	{
		pXML->SetCommerce(&m_piGlobalCommerceModifier);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piGlobalCommerceModifier, NUM_COMMERCE_TYPES);
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"SpecialistExtraCommerces"))
	{
		pXML->SetCommerce(&m_piSpecialistExtraCommerce);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piSpecialistExtraCommerce, NUM_COMMERCE_TYPES);
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"StateReligionCommerces"))
	{
		pXML->SetCommerce(&m_piStateReligionCommerce);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piStateReligionCommerce, NUM_COMMERCE_TYPES);
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"CommerceHappinesses"))
	{
		pXML->SetCommerce(&m_piCommerceHappiness);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piCommerceHappiness, NUM_COMMERCE_TYPES);
	}

	pXML->SetVariableListTagPair(&m_piReligionChange, "ReligionChanges", GC.getReligionInfo(), sizeof(GC.getReligionInfo((ReligionTypes)0)), GC.getNumReligionInfos());

	pXML->SetVariableListTagPair(&m_piSpecialistCount, "SpecialistCounts", GC.getSpecialistInfo(), sizeof(GC.getSpecialistInfo((SpecialistTypes)0)), GC.getNumSpecialistInfos());
	pXML->SetVariableListTagPair(&m_piFreeSpecialistCount, "FreeSpecialistCounts", GC.getSpecialistInfo(), sizeof(GC.getSpecialistInfo((SpecialistTypes)0)), GC.getNumSpecialistInfos());

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"CommerceFlexibles"))
	{
		pXML->SetCommerce(&m_pbCommerceFlexible);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_pbCommerceFlexible, NUM_COMMERCE_TYPES);
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"CommerceChangeOriginalOwners"))
	{
		pXML->SetCommerce(&m_pbCommerceChangeOriginalOwner);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_pbCommerceChangeOriginalOwner, NUM_COMMERCE_TYPES);
	}

	pXML->GetChildXmlValByName(szTextVal, "ConstructSound");
	setConstructSound(szTextVal);

	pXML->SetVariableListTagPair(&m_piBonusHealthChanges, "BonusHealthChanges", GC.getBonusInfo(), sizeof(GC.getBonusInfo((BonusTypes)0)), GC.getNumBonusInfos());
	pXML->SetVariableListTagPair(&m_piBonusHappinessChanges, "BonusHappinessChanges", GC.getBonusInfo(), sizeof(GC.getBonusInfo((BonusTypes)0)), GC.getNumBonusInfos());
	pXML->SetVariableListTagPair(&m_piBonusProductionModifier, "BonusProductionModifiers", GC.getBonusInfo(), sizeof(GC.getBonusInfo((BonusTypes)0)), GC.getNumBonusInfos());

	pXML->SetVariableListTagPair(&m_piUnitCombatFreeExperience, "UnitCombatFreeExperiences", GC.getUnitCombatInfo(), sizeof(GC.getUnitCombatInfo((UnitCombatTypes)0)), GC.getNumUnitCombatInfos());

	pXML->SetVariableListTagPair(&m_piDomainFreeExperience, "DomainFreeExperiences", GC.getDomainInfo(), sizeof(GC.getDomainInfo((DomainTypes)0)), NUM_DOMAIN_TYPES);
	pXML->SetVariableListTagPair(&m_piDomainProductionModifier, "DomainProductionModifiers", GC.getDomainInfo(), sizeof(GC.getDomainInfo((DomainTypes)0)), NUM_DOMAIN_TYPES);

	pXML->SetVariableListTagPair(&m_piBuildingHappinessChanges, "BuildingHappinessChanges", GC.getBuildingInfo(), sizeof(GC.getBuildingInfo((BuildingTypes)0)), GC.getNumBuildingInfos());

	pXML->SetVariableListTagPair(&m_piPrereqNumOfBuildingClass, "PrereqBuildingClasses", GC.getBuildingClassInfo(), sizeof(GC.getBuildingClassInfo((BuildingClassTypes)0)), GC.getNumBuildingClassInfos());
	pXML->SetVariableListTagPair(&m_pbBuildingClassNeededInCity, "BuildingClassNeededs", GC.getBuildingClassInfo(), sizeof(GC.getBuildingClassInfo((BuildingClassTypes)0)), GC.getNumBuildingClassInfos());

	pXML->Init2DIntList(&m_ppaiSpecialistYieldChange, GC.getNumSpecialistInfos(), NUM_YIELD_TYPES);
	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"SpecialistYieldChanges"))
	{
		iNumChildren = gDLL->getXMLIFace()->GetNumChildren(pXML->GetXML());

		if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"SpecialistYieldChange"))
		{
			for(j=0;j<iNumChildren;j++)
			{
				pXML->GetChildXmlValByName(szTextVal, "SpecialistType");
				k = pXML->FindInInfoClass(szTextVal, GC.getSpecialistInfo(), sizeof(GC.getSpecialistInfo((SpecialistTypes)0)), GC.getNumSpecialistInfos());
				// delete the array since it will be reallocated
				SAFE_DELETE_ARRAY(m_ppaiSpecialistYieldChange[k]);
				if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"YieldChanges") && (k > -1))
				{
					// call the function that sets the yield change variable
					pXML->SetYields(&m_ppaiSpecialistYieldChange[k]);
					gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
				}
				else
				{
					pXML->InitList(&m_ppaiSpecialistYieldChange[k], NUM_YIELD_TYPES);
				}

				if (!gDLL->getXMLIFace()->NextSibling(pXML->GetXML()))
				{
					break;
				}
			}

			// set the current xml node to it's parent node
			gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
		}

		// set the current xml node to it's parent node
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	pXML->Init2DIntList(&m_ppaiBonusYieldModifier, GC.getNumBonusInfos(), NUM_YIELD_TYPES);
	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"BonusYieldModifiers"))
	{
		iNumChildren = gDLL->getXMLIFace()->GetNumChildren(pXML->GetXML());

		if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"BonusYieldModifier"))
		{
			for(j=0;j<iNumChildren;j++)
			{
				pXML->GetChildXmlValByName(szTextVal, "BonusType");
				k = pXML->FindInInfoClass(szTextVal, GC.getBonusInfo(), sizeof(GC.getBonusInfo((BonusTypes)0)), GC.getNumBonusInfos());
				// delete the array since it will be reallocated
				SAFE_DELETE_ARRAY(m_ppaiBonusYieldModifier[k]);
				if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"YieldModifiers") && (k > -1))
				{
					// call the function that sets the yield change variable
					pXML->SetYields(&m_ppaiBonusYieldModifier[k]);
					gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
				}
				else
				{
					pXML->InitList(&m_ppaiBonusYieldModifier[k], NUM_YIELD_TYPES);
				}

				if (!gDLL->getXMLIFace()->NextSibling(pXML->GetXML()))
				{
					break;
				}
			}

			// set the current xml node to it's parent node
			gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
		}

		// set the current xml node to it's parent node
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	pXML->SetVariableListTagPair(&m_piFlavorValue, "Flavors", GC.getFlavorTypes(), GC.getNumFlavorTypes());

	return true;
}

//======================================================================================================
//					CvSpecialBuildingInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvSpecialBuildingInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvSpecialBuildingInfo::CvSpecialBuildingInfo() :
m_iObsoleteTech(NO_TECH),
m_iTechPrereq(NO_TECH),
m_bValid(false),
m_piProductionTraits(NULL)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvSpecialBuildingInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvSpecialBuildingInfo::~CvSpecialBuildingInfo()
{
	SAFE_DELETE_ARRAY(m_piProductionTraits);
}

int CvSpecialBuildingInfo::getObsoleteTech( void ) const
{
	return m_iObsoleteTech;
}

int CvSpecialBuildingInfo::getTechPrereq( void ) const
{
	return m_iTechPrereq;
}

bool CvSpecialBuildingInfo::isValid( void ) const
{
	return m_bValid;
}

// Arrays

int CvSpecialBuildingInfo::getProductionTraits(int i) const		
{
	FAssertMsg(i < GC.getNumTraitInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piProductionTraits ? m_piProductionTraits[i] : -1;
}

bool CvSpecialBuildingInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(szTextVal, "ObsoleteTech");
	m_iObsoleteTech = pXML->FindInInfoClass(szTextVal, GC.getTechInfo(), sizeof(GC.getTechInfo((TechTypes)0)), GC.getNumTechInfos());
	pXML->GetChildXmlValByName(szTextVal, "TechPrereq");
	m_iTechPrereq = pXML->FindInInfoClass(szTextVal, GC.getTechInfo(), sizeof(GC.getTechInfo((TechTypes)0)), GC.getNumTechInfos());

	pXML->GetChildXmlValByName(&m_bValid, "bValid");

	pXML->SetVariableListTagPair(&m_piProductionTraits, "ProductionTraits", GC.getTraitInfo(), sizeof(GC.getTraitInfo((TraitTypes)0)), GC.getNumTraitInfos());

	return true;
}

//======================================================================================================
//					CvBuildingClassInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvBuildingClassInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvBuildingClassInfo::CvBuildingClassInfo() :
m_iMaxGlobalInstances(0),
m_iMaxTeamInstances(0),
m_iMaxPlayerInstances(0),
m_iExtraPlayerInstances(0),
m_iDefaultBuildingIndex(NO_BUILDING),
m_bNoLimit(false),
m_bMonument(false),
m_piVictoryThreshold(NULL)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvBuildingClassInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvBuildingClassInfo::~CvBuildingClassInfo()
{
	SAFE_DELETE_ARRAY(m_piVictoryThreshold);
}

int CvBuildingClassInfo::getMaxGlobalInstances() const	
{
	return m_iMaxGlobalInstances;
}

int CvBuildingClassInfo::getMaxTeamInstances() const		
{
	return m_iMaxTeamInstances;
}

int CvBuildingClassInfo::getMaxPlayerInstances() const	
{
	return m_iMaxPlayerInstances;
}

int CvBuildingClassInfo::getExtraPlayerInstances() const
{
	return m_iExtraPlayerInstances;
}

int CvBuildingClassInfo::getDefaultBuildingIndex() const
{
	return m_iDefaultBuildingIndex;
}

void CvBuildingClassInfo::setDefaultBuildingIndex(int i)	
{
	m_iDefaultBuildingIndex = i;
}

bool CvBuildingClassInfo::isNoLimit() const				
{
	return m_bNoLimit;
}

bool CvBuildingClassInfo::isMonument() const				
{
	return m_bMonument;
}

// Arrays

int CvBuildingClassInfo::getVictoryThreshold(int i) const
{
	FAssertMsg(i < GC.getNumVictoryInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piVictoryThreshold ? m_piVictoryThreshold[i] : -1;
}

bool CvBuildingClassInfo::read(CvXMLLoadUtility* pXML)
{
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(&m_iMaxGlobalInstances, "iMaxGlobalInstances");
	pXML->GetChildXmlValByName(&m_iMaxTeamInstances, "iMaxTeamInstances");
	pXML->GetChildXmlValByName(&m_iMaxPlayerInstances, "iMaxPlayerInstances");
	pXML->GetChildXmlValByName(&m_iExtraPlayerInstances, "iExtraPlayerInstances");

	pXML->GetChildXmlValByName(&m_bNoLimit, "bNoLimit");
	pXML->GetChildXmlValByName(&m_bMonument, "bMonument");

	pXML->SetVariableListTagPair(&m_piVictoryThreshold, "VictoryThresholds", GC.getVictoryInfo(), sizeof(GC.getVictoryInfo((VictoryTypes)0)), GC.getNumVictoryInfos());

	return true;
}


//======================================================================================================
//					CvRiverInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvRiverInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvRiverInfo::CvRiverInfo()
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvRiverInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvRiverInfo::~CvRiverInfo()
{
}

//======================================================================================================
//					CvRiverModelInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvRiverModelInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvRiverModelInfo::CvRiverModelInfo() :
m_iTextureIndex(0)
{
	m_szDeltaString[0] = '\0';
	m_szConnectString[0] = '\0';
	m_szRotateString[0] = '\0';
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvRiverModelInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvRiverModelInfo::~CvRiverModelInfo()
{
}

const TCHAR* CvRiverModelInfo::getModelFile() const	
{
	return m_szModelFile;
}

void CvRiverModelInfo::setModelFile(const TCHAR* szVal)					// The model filename
{
	m_szModelFile=szVal;
}

const TCHAR* CvRiverModelInfo::getBorderFile() const	
{
	return m_szBorderFile;
}

void CvRiverModelInfo::setBorderFile(const TCHAR* szVal)					// The model filename
{
	m_szBorderFile=szVal;
}

int CvRiverModelInfo::getTextureIndex() const
{
	return m_iTextureIndex;
}

const TCHAR* CvRiverModelInfo::getDeltaString() const
{
	return m_szDeltaString;
}

const TCHAR* CvRiverModelInfo::getConnectString() const
{
	return m_szConnectString;
}

const TCHAR* CvRiverModelInfo::getRotateString() const
{
	return m_szRotateString;
}

bool CvRiverModelInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	if(pXML->GetChildXmlValByName(szTextVal, "ModelFile"))
	{
		setModelFile(szTextVal);
	}
	if(pXML->GetChildXmlValByName(szTextVal, "BorderFile"))
	{
		setBorderFile(szTextVal);
	}

	pXML->GetChildXmlValByName(&m_iTextureIndex, "TextureIndex");
	pXML->GetChildXmlValByName(m_szDeltaString, "DeltaType");
	pXML->GetChildXmlValByName(m_szConnectString, "Connections");
	pXML->GetChildXmlValByName(m_szRotateString, "Rotations");

	return true;
}

//======================================================================================================
//					CvRouteModelInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvRouteModelInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvRouteModelInfo::CvRouteModelInfo() :
m_eRouteType(NO_ROUTE)
{
	m_szConnectString[0] = '\0';
	m_szModelConnectString[0] = '\0';
	m_szRotateString[0] = '\0';
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvRouteModelInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvRouteModelInfo::~CvRouteModelInfo()
{
}

RouteTypes CvRouteModelInfo::getRouteType() const		// The route type
{
	return m_eRouteType;
}

const TCHAR* CvRouteModelInfo::getModelFile() const	
{
	return m_szModelFile;
}

void CvRouteModelInfo::setModelFile(const TCHAR* szVal)				// The model filename
{
	m_szModelFile=szVal;
}

const TCHAR* CvRouteModelInfo::getModelFileKey() const	
{
	return m_szModelFileKey;
}

void CvRouteModelInfo::setModelFileKey(const TCHAR* szVal)				// The model filename Key
{
	m_szModelFileKey=szVal;
}

const TCHAR* CvRouteModelInfo::getConnectString() const
{
	return m_szConnectString;
}

const TCHAR* CvRouteModelInfo::getModelConnectString() const
{
	return m_szModelConnectString;
}

const TCHAR* CvRouteModelInfo::getRotateString() const
{
	return m_szRotateString;
}

bool CvRouteModelInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(szTextVal, "ModelFile");
	setModelFile(szTextVal);
	pXML->GetChildXmlValByName(szTextVal, "ModelFileKey");
	setModelFileKey(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "RouteType");
	m_eRouteType = (RouteTypes)(pXML->FindInInfoClass( szTextVal, GC.getRouteInfo(), sizeof(GC.getRouteInfo((RouteTypes)0)), GC.getNumRouteInfos()));
	pXML->GetChildXmlValByName(m_szConnectString, "Connections");
	pXML->GetChildXmlValByName(m_szModelConnectString, "ModelConnections");
	pXML->GetChildXmlValByName(m_szRotateString, "Rotations");

	return true;
}

//======================================================================================================
//					CvCivilizationInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvCivilizationInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvCivilizationInfo::CvCivilizationInfo():
m_iDefaultPlayerColor(NO_PLAYERCOLOR),
m_iArtStyleType(NO_ARTSTYLE),
m_iNumCityNames(0),
m_iNumLeaders(0),
m_iSelectionSoundScriptId(0),
m_iActionSoundScriptId(0),
m_bPlayable(false),
m_bAIPlayable(false),
m_piCivilizationBuildings(NULL),
m_piCivilizationUnits(NULL),
m_piCivilizationFreeUnitsClass(NULL),
m_piCivilizationInitialCivics(NULL),
m_pbLeaders(NULL),
m_pbCivilizationFreeBuildingClass(NULL),
m_pbCivilizationFreeTechs(NULL),
m_pbCivilizationDisableTechs(NULL),
m_paszCityNames(NULL)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvCivilizationInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvCivilizationInfo::~CvCivilizationInfo()
{
	SAFE_DELETE_ARRAY(m_piCivilizationBuildings);
	SAFE_DELETE_ARRAY(m_piCivilizationUnits);
	SAFE_DELETE_ARRAY(m_piCivilizationFreeUnitsClass);
	SAFE_DELETE_ARRAY(m_piCivilizationInitialCivics);
	SAFE_DELETE_ARRAY(m_pbLeaders);
	SAFE_DELETE_ARRAY(m_pbCivilizationFreeBuildingClass);
	SAFE_DELETE_ARRAY(m_pbCivilizationFreeTechs);
	SAFE_DELETE_ARRAY(m_pbCivilizationDisableTechs);
	SAFE_DELETE_ARRAY(m_paszCityNames);
}

int CvCivilizationInfo::getDefaultPlayerColor() const
{
	return m_iDefaultPlayerColor;
}

int CvCivilizationInfo::getArtStyleType() const
{
	return m_iArtStyleType;
}

int CvCivilizationInfo::getNumCityNames() const
{
	return m_iNumCityNames;
}

int CvCivilizationInfo::getNumLeaders() const// the number of leaders the Civ has, this is needed so that random leaders can be generated easily
{
	return m_iNumLeaders;
}

int CvCivilizationInfo::getSelectionSoundScriptId() const
{
	return m_iSelectionSoundScriptId;
}

int CvCivilizationInfo::getActionSoundScriptId() const
{
	return m_iActionSoundScriptId;
}

bool CvCivilizationInfo::isAIPlayable() const
{
	return m_bAIPlayable;
}

bool CvCivilizationInfo::isPlayable() const	
{
	return m_bPlayable;
}

const wchar* CvCivilizationInfo::getShortDescription(uint uiForm)
{
	m_szTempText = gDLL->getObjectText(m_szShortDescriptionKey, uiForm);
	return m_szTempText;
}

void CvCivilizationInfo::setShortDescriptionKey(const TCHAR* szVal)
{
	m_szShortDescriptionKey = szVal;
}

const wchar* CvCivilizationInfo::getShortDescriptionKey() const
{
	setTempText(CvWString(m_szShortDescriptionKey));
	return m_szTempText;
}

const wchar* CvCivilizationInfo::getAdjective(uint uiForm)		
{
	m_szTempText = gDLL->getObjectText(m_szAdjectiveKey, uiForm);
	return m_szTempText;
}

void CvCivilizationInfo::setAdjectiveKey(const TCHAR* szVal)
{
	m_szAdjectiveKey = szVal;
}

const wchar* CvCivilizationInfo::getAdjectiveKey() const
{
	setTempText(CvWString(m_szAdjectiveKey));
	return m_szTempText;
}

const TCHAR* CvCivilizationInfo::getFlagTexture() const	
{
	return ARTFILEMGR.getCivilizationArtInfo( getArtDefineTag() )->getPath();
}

const TCHAR* CvCivilizationInfo::getArtDefineTag() const
{
	return m_szArtDefineTag; 
}

void CvCivilizationInfo::setArtDefineTag(const TCHAR* szVal)
{
	m_szArtDefineTag = szVal; 
}

// Arrays

int CvCivilizationInfo::getCivilizationBuildings(int i) const		
{
	FAssertMsg(i < GC.getNumBuildingClassInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piCivilizationBuildings ? m_piCivilizationBuildings[i] : -1;
}

int CvCivilizationInfo::getCivilizationUnits(int i) const
{
	FAssertMsg(i < GC.getNumUnitClassInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piCivilizationUnits ? m_piCivilizationUnits[i] : -1;
}

int CvCivilizationInfo::getCivilizationFreeUnitsClass(int i) const
{
	FAssertMsg(i < GC.getNumUnitClassInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piCivilizationFreeUnitsClass ? m_piCivilizationFreeUnitsClass[i] : -1;
}

int CvCivilizationInfo::getCivilizationInitialCivics(int i) const
{
	FAssertMsg(i < GC.getNumCivicOptionInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piCivilizationInitialCivics ? m_piCivilizationInitialCivics[i] : -1;
}

bool CvCivilizationInfo::isLeaders(int i) const
{
	FAssertMsg(i < GC.getNumLeaderHeadInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbLeaders ? m_pbLeaders[i] : false;
}

bool CvCivilizationInfo::isCivilizationFreeBuildingClass(int i) const
{
	FAssertMsg(i < GC.getNumBuildingClassInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbCivilizationFreeBuildingClass ? m_pbCivilizationFreeBuildingClass[i] : false;
}

bool CvCivilizationInfo::isCivilizationFreeTechs(int i) const
{
	FAssertMsg(i < GC.getNumTechInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbCivilizationFreeTechs ? m_pbCivilizationFreeTechs[i] : false;
}

bool CvCivilizationInfo::isCivilizationDisableTechs(int i) const
{
	FAssertMsg(i < GC.getNumTechInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbCivilizationDisableTechs ? m_pbCivilizationDisableTechs[i] : false;
}

const CvArtInfoCivilization* CvCivilizationInfo::getArtInfo() const
{
	return ARTFILEMGR.getCivilizationArtInfo( getArtDefineTag() );
}

const TCHAR* CvCivilizationInfo::getButton() const
{
	return getArtInfo()->getButton();
}

std::string CvCivilizationInfo::getCityNames(int i) const
{
	FAssertMsg(i < getNumCityNames(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_paszCityNames[i];
}

void CvCivilizationInfo::read(FDataStreamBase* stream)
{
	CvInfoBase::read(stream);

	uint uiFlag=0;
	stream->Read(&uiFlag);		// flag for expansion

	stream->Read(&m_iDefaultPlayerColor);
	stream->Read(&m_iArtStyleType);
	stream->Read(&m_iNumCityNames);
	stream->Read(&m_iNumLeaders);
	stream->Read(&m_iSelectionSoundScriptId);
	stream->Read(&m_iActionSoundScriptId);

	stream->Read(&m_bAIPlayable);
	stream->Read(&m_bPlayable);

	stream->ReadString(m_szArtDefineTag);
	stream->ReadString(m_szShortDescriptionKey);
	stream->ReadString(m_szAdjectiveKey);

	// Arrays

	SAFE_DELETE_ARRAY(m_piCivilizationBuildings);
	m_piCivilizationBuildings = new int[GC.getNumBuildingClassInfos()];
	stream->Read(GC.getNumBuildingClassInfos(), m_piCivilizationBuildings);

	SAFE_DELETE_ARRAY(m_piCivilizationUnits);
	m_piCivilizationUnits = new int[GC.getNumUnitClassInfos()];
	stream->Read(GC.getNumUnitClassInfos(), m_piCivilizationUnits);

	SAFE_DELETE_ARRAY(m_piCivilizationFreeUnitsClass);
	m_piCivilizationFreeUnitsClass = new int[GC.getNumUnitClassInfos()];
	stream->Read(GC.getNumUnitClassInfos(), m_piCivilizationFreeUnitsClass);

	SAFE_DELETE_ARRAY(m_piCivilizationInitialCivics);
	m_piCivilizationInitialCivics = new int[GC.getNumCivicOptionInfos()];
	stream->Read(GC.getNumCivicOptionInfos(), m_piCivilizationInitialCivics);

	SAFE_DELETE_ARRAY(m_pbLeaders);
	m_pbLeaders = new bool[GC.getNumLeaderHeadInfos()];
	stream->Read(GC.getNumLeaderHeadInfos(), m_pbLeaders);

	SAFE_DELETE_ARRAY(m_pbCivilizationFreeBuildingClass);
	m_pbCivilizationFreeBuildingClass = new bool[GC.getNumBuildingClassInfos()];
	stream->Read(GC.getNumBuildingClassInfos(), m_pbCivilizationFreeBuildingClass);

	SAFE_DELETE_ARRAY(m_pbCivilizationFreeTechs);
	m_pbCivilizationFreeTechs = new bool[GC.getNumTechInfos()];
	stream->Read(GC.getNumTechInfos(), m_pbCivilizationFreeTechs);

	SAFE_DELETE_ARRAY(m_pbCivilizationDisableTechs);
	m_pbCivilizationDisableTechs = new bool[GC.getNumTechInfos()];
	stream->Read(GC.getNumTechInfos(), m_pbCivilizationDisableTechs);

	SAFE_DELETE_ARRAY(m_paszCityNames);
	m_paszCityNames = new CvString[m_iNumCityNames];
	stream->ReadString(m_iNumCityNames, m_paszCityNames);
}

void CvCivilizationInfo::write(FDataStreamBase* stream)
{
	CvInfoBase::write(stream);

	uint uiFlag=0;
	stream->Write(uiFlag);		// flag for expansion

	stream->Write(m_iDefaultPlayerColor);
	stream->Write(m_iArtStyleType);
	stream->Write(m_iNumCityNames);
	stream->Write(m_iNumLeaders);
	stream->Write(m_iSelectionSoundScriptId);
	stream->Write(m_iActionSoundScriptId);

	stream->Write(m_bAIPlayable);
	stream->Write(m_bPlayable);

	stream->WriteString(m_szArtDefineTag);
	stream->WriteString(m_szShortDescriptionKey);
	stream->WriteString(m_szAdjectiveKey);

	// Arrays

	stream->Write(GC.getNumBuildingClassInfos(), m_piCivilizationBuildings);
	stream->Write(GC.getNumUnitClassInfos(), m_piCivilizationUnits);
	stream->Write(GC.getNumUnitClassInfos(), m_piCivilizationFreeUnitsClass);
	stream->Write(GC.getNumCivicOptionInfos(), m_piCivilizationInitialCivics);
	stream->Write(GC.getNumLeaderHeadInfos(), m_pbLeaders);
	stream->Write(GC.getNumBuildingClassInfos(), m_pbCivilizationFreeBuildingClass);
	stream->Write(GC.getNumTechInfos(), m_pbCivilizationFreeTechs);
	stream->Write(GC.getNumTechInfos(), m_pbCivilizationDisableTechs);
	stream->WriteString(m_iNumCityNames, m_paszCityNames);
}

bool CvCivilizationInfo::read(CvXMLLoadUtility* pXML)
{
	char szClassVal[256];					// holds the text value of the relevant classinfo

	CvString szTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	int j, iNumSibs;

	pXML->GetChildXmlValByName(szTextVal, "ShortDescription");
	// Get the Text from Text/Civ4GameTextXML.xml
	setShortDescriptionKey(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "Adjective");
	// Get the Text from Text/Civ4GameTextXML.xml
	setAdjectiveKey(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "DefaultPlayerColor");
	m_iDefaultPlayerColor = pXML->FindInInfoClass(szTextVal, GC.getPlayerColorInfo(), sizeof(GC.getPlayerColorInfo((PlayerColorTypes)0)), GC.getNumPlayerColorInfos());

	pXML->GetChildXmlValByName(szTextVal, "ArtDefineTag");
	setArtDefineTag(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "ArtStyleType");
	m_iArtStyleType = GC.getTypesEnum(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "CivilizationSelectionSound");
	m_iSelectionSoundScriptId = (szTextVal.GetLength() > 0) ? gDLL->getAudioTagIndex( szTextVal.GetCString(), AUDIOTAG_3DSCRIPT ) : -1;
	pXML->GetChildXmlValByName(szTextVal, "CivilizationActionSound");
	m_iActionSoundScriptId = (szTextVal.GetLength() > 0) ? gDLL->getAudioTagIndex( szTextVal.GetCString(), AUDIOTAG_3DSCRIPT ) : -1;

	// set the current xml node to it's next sibling and then
	pXML->GetChildXmlValByName(&m_bPlayable, "bPlayable");
	pXML->GetChildXmlValByName(&m_bAIPlayable, "bAIPlayable");

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"Cities"))
	{
		pXML->SetStringList(&m_paszCityNames, &m_iNumCityNames);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	// if we can set the current xml node to it's next sibling
	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"Buildings"))
	{
		// pXML->Skip any comments and stop at the next value we might want
		if (pXML->SkipToNextVal())
		{
			// call the function that sets the default civilization buildings
			pXML->InitBuildingDefaults(&m_piCivilizationBuildings);
			// get the total number of children the current xml node has
			iNumSibs = gDLL->getXMLIFace()->GetNumChildren(pXML->GetXML());
			// if the call to the function that sets the current xml node to it's first non-comment
			// child and sets the parameter with the new node's value succeeds
			if ( (0 < iNumSibs) && (gDLL->getXMLIFace()->SetToChild(pXML->GetXML())) )
			{
				int iBuildingClassIndex;

				FAssertMsg((iNumSibs <= GC.getNumBuildingClassInfos()) ,"In SetGlobalCivilizationInfo iNumSibs is greater than GC.getNumBuildingClassInfos()");

				// loop through all the siblings
				for (j=0;j<iNumSibs;j++)
				{
					if (pXML->GetChildXmlVal(szClassVal))
					{
						// get the index into the array based on the building class type
						iBuildingClassIndex = pXML->FindInInfoClass(szClassVal, GC.getBuildingClassInfo(), sizeof(GC.getBuildingClassInfo((BuildingClassTypes)0)), GC.getNumBuildingClassInfos());
						if (-1 < iBuildingClassIndex)
						{
							// get the next value which should be the building type to set this civilization's version of this building class too
							pXML->GetNextXmlVal( szTextVal);
							// call the find in list function to return either -1 if no value is found
							// or the index in the list the match is found at
							m_piCivilizationBuildings[iBuildingClassIndex] = pXML->FindInInfoClass(szTextVal, GC.getBuildingInfo(), sizeof(GC.getBuildingInfo((BuildingTypes)0)), GC.getNumBuildingInfos());
						}
						else
						{
							FAssertMsg(0,"BuildingClass index is -1 in SetGlobalCivilizationInfo function");
						}

						// set the current xml node to it's parent node
						gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
					}

					// if the call to the function that sets the current xml node to it's first non-comment
					// sibling and sets the parameter with the new node's value does not succeed
					// we will break out of this for loop
					if (!gDLL->getXMLIFace()->NextSibling(pXML->GetXML()))
					{
						break;
					}
				}

				// set the current xml node to it's parent node
				gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
			}
		}

		// set the current xml node to it's parent node
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	// if we can set the current xml node to it's next sibling
	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"Units"))
	{
		// pXML->Skip any comments and stop at the next value we might want
		if (pXML->SkipToNextVal())
		{
			// call the function that sets the default civilization buildings
			pXML->InitUnitDefaults(&m_piCivilizationUnits);
			// get the total number of children the current xml node has
			iNumSibs = gDLL->getXMLIFace()->GetNumChildren(pXML->GetXML());
			// if the call to the function that sets the current xml node to it's first non-comment
			// child and sets the parameter with the new node's value succeeds
			if ( (0 < iNumSibs) && (gDLL->getXMLIFace()->SetToChild(pXML->GetXML())) )
			{
				int iUnitClassIndex;

				FAssertMsg((iNumSibs <= GC.getNumUnitClassInfos()),"In SetGlobalCivilizationInfo iNumSibs is greater than GC.getNumUnitClassInfos()");

				// loop through all the siblings
				for (j=0;j<iNumSibs;j++)
				{
					if (pXML->GetChildXmlVal(szClassVal))
					{
						// set the unit class index
						iUnitClassIndex = pXML->FindInInfoClass(szClassVal, GC.getUnitClassInfo(), sizeof(GC.getUnitClassInfo((UnitClassTypes)0)), GC.getNumUnitClassInfos());
						if (-1 < iUnitClassIndex)
						{
							// get the next value which should be the building type to set this civilization's version of this building class too
							pXML->GetNextXmlVal( szTextVal);
							// call the find in list function to return either -1 if no value is found
							// or the index in the list the match is found at
							m_piCivilizationUnits[iUnitClassIndex] = pXML->FindInInfoClass(szTextVal, GC.getUnitInfo(), sizeof(GC.getUnitInfo((UnitTypes)0)), GC.getNumUnitInfos());
						}
						else
						{
							FAssertMsg(0, "UnitClass index is -1 in SetGlobalCivilizationInfo function");
						}

						// set the current xml node to it's parent node
						gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
					}

					// if the call to the function that sets the current xml node to it's first non-comment
					// sibling and sets the parameter with the new node's value does not succeed
					// we will break out of this for loop
					if (!gDLL->getXMLIFace()->NextSibling(pXML->GetXML()))
					{
						break;
					}
				}

				// set the current xml node to it's parent node
				gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
			}
		}

		// set the current xml node to it's parent node
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	pXML->SetVariableListTagPair(&m_piCivilizationFreeUnitsClass, "FreeUnitClasses", GC.getUnitClassInfo(), sizeof(GC.getUnitClassInfo((UnitClassTypes)0)), GC.getNumUnitClassInfos());

	pXML->SetVariableListTagPair(&m_pbCivilizationFreeBuildingClass, "FreeBuildingClasses", GC.getBuildingClassInfo(), sizeof(GC.getBuildingClassInfo((BuildingClassTypes)0)), GC.getNumBuildingClassInfos());

	pXML->SetVariableListTagPair(&m_pbCivilizationFreeTechs, "FreeTechs", GC.getTechInfo(), sizeof(GC.getTechInfo((TechTypes)0)), GC.getNumTechInfos());
	pXML->SetVariableListTagPair(&m_pbCivilizationDisableTechs, "DisableTechs", GC.getTechInfo(), sizeof(GC.getTechInfo((TechTypes)0)), GC.getNumTechInfos());

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"InitialCivics"))
	{
		if (pXML->SkipToNextVal())
		{
			iNumSibs = gDLL->getXMLIFace()->GetNumChildren(pXML->GetXML());
			pXML->InitList(&m_piCivilizationInitialCivics, GC.getNumCivicOptionInfos());

			if (0 < iNumSibs)
			{
				if (pXML->GetChildXmlVal(szTextVal))
				{
					FAssertMsg((iNumSibs <= GC.getNumCivicOptionInfos()),"For loop iterator is greater than array size");
					for (j=0;j<iNumSibs;j++)
					{
						m_piCivilizationInitialCivics[j] = pXML->FindInInfoClass(szTextVal, GC.getCivicInfo(), sizeof(GC.getCivicInfo((CivicTypes)0)), GC.getNumCivicInfos());
						if (!pXML->GetNextXmlVal(szTextVal))
						{
							break;
						}
					}

					gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
				}
			}
		}

		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	pXML->SetVariableListTagPair(&m_pbLeaders, "Leaders", GC.getLeaderHeadInfo(), sizeof(GC.getLeaderHeadInfo((LeaderHeadTypes)0)), GC.getNumLeaderHeadInfos());

	pXML->GetChildXmlValByName(szTextVal, "CivilizationSelectionSound");

	return true;
}

//======================================================================================================
//					CvVictoryInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvVictoryInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvVictoryInfo::CvVictoryInfo() :
m_iPopulationPercentLead(0),
m_iLandPercent(0),
m_iMinLandPercent(0),
m_iReligionPercent(0),
m_iCityCulture(0),
m_iNumCultureCities(0),
m_iTotalCultureRatio(0),
m_bTargetScore(false),
m_bEndScore(false),
m_bConquest(false),
m_bDiploVote(false),
m_bPermanent(false)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvVictoryInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvVictoryInfo::~CvVictoryInfo()
{
}

int CvVictoryInfo::getPopulationPercentLead() const		
{
	return m_iPopulationPercentLead;
}

int CvVictoryInfo::getLandPercent() const
{
	return m_iLandPercent;
}

int CvVictoryInfo::getMinLandPercent() const
{
	return m_iMinLandPercent;
}

int CvVictoryInfo::getReligionPercent() const			
{
	return m_iReligionPercent;
}

int CvVictoryInfo::getCityCulture() const
{
	return m_iCityCulture;
}

int CvVictoryInfo::getNumCultureCities() const		
{
	return m_iNumCultureCities;
}

int CvVictoryInfo::getTotalCultureRatio() const		
{
	return m_iTotalCultureRatio;
}

bool CvVictoryInfo::isTargetScore() const
{
	return m_bTargetScore;
}

bool CvVictoryInfo::isEndScore() const
{
	return m_bEndScore;
}

bool CvVictoryInfo::isConquest() const
{
	return m_bConquest;
}

bool CvVictoryInfo::isDiploVote() const
{
	return m_bDiploVote;
}

bool CvVictoryInfo::isPermanent() const
{
	return m_bPermanent;
}

const char* CvVictoryInfo::getMovie() const
{
	return m_szMovie;
}


//
// read from xml
//
bool CvVictoryInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(&m_bTargetScore, "bTargetScore");
	pXML->GetChildXmlValByName(&m_bEndScore, "bEndScore");
	pXML->GetChildXmlValByName(&m_bConquest, "bConquest");
	pXML->GetChildXmlValByName(&m_bDiploVote, "bDiploVote");
	pXML->GetChildXmlValByName(&m_bPermanent, "bPermanent");
	pXML->GetChildXmlValByName(&m_iPopulationPercentLead, "iPopulationPercentLead");
	pXML->GetChildXmlValByName(&m_iLandPercent, "iLandPercent");
	pXML->GetChildXmlValByName(&m_iMinLandPercent, "iMinLandPercent");
	pXML->GetChildXmlValByName(&m_iReligionPercent, "iReligionPercent");

	pXML->GetChildXmlValByName(szTextVal, "CityCulture");
	m_iCityCulture = pXML->FindInInfoClass(szTextVal, GC.getCultureLevelInfo(), sizeof(GC.getCultureLevelInfo((CultureLevelTypes)0)), GC.getNumCultureLevelInfos());

	pXML->GetChildXmlValByName(&m_iNumCultureCities, "iNumCultureCities");
	pXML->GetChildXmlValByName(&m_iTotalCultureRatio, "iTotalCultureRatio");
	pXML->GetChildXmlValByName(m_szMovie, "VictoryMovie");
	
	return true;
}

//======================================================================================================
//					CvHurryInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvHurryInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvHurryInfo::CvHurryInfo() :
m_iGoldPerProduction(0),
m_iProductionPerPopulation(0),
m_bAnger(false)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvHurryInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvHurryInfo::~CvHurryInfo()
{
}

int CvHurryInfo::getGoldPerProduction() const			
{
	return m_iGoldPerProduction;
}

int CvHurryInfo::getProductionPerPopulation() const	
{
	return m_iProductionPerPopulation;
}

bool CvHurryInfo::isAnger() const		
{
	return m_bAnger;
}

bool CvHurryInfo::read(CvXMLLoadUtility* pXML)
{
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(&m_iGoldPerProduction, "iGoldPerProduction");
	pXML->GetChildXmlValByName(&m_iProductionPerPopulation, "iProductionPerPopulation");

	pXML->GetChildXmlValByName(&m_bAnger, "bAnger");

	return true;
}
//======================================================================================================
//					CvHandicapInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvHandicapInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvHandicapInfo::CvHandicapInfo() :
m_iFreeWinsVsBarbs(0),
m_iAnimalAttackProb(0),
m_iStartingLocationPercent(0),
m_iStartingGold(0),
m_iFreeUnits(0),
m_iUnitCostPercent(0),
m_iResearchPercent(0),
m_iDistanceMaintenancePercent(0),				
m_iNumCitiesMaintenancePercent(0),				
m_iMaxNumCitiesMaintenance(0),					
m_iCivicUpkeepPercent(0),								
m_iInflationPercent(0),									
m_iHealthBonus(0),									
m_iHappyBonus(0),
m_iAttitudeChange(0),
m_iNoTechTradeModifier(0),
m_iTechTradeKnownModifier(0),
m_iUnownedTilesPerGameAnimal(0),				
m_iUnownedTilesPerBarbarianUnit(0),			
m_iUnownedWaterTilesPerBarbarianUnit(0),	
m_iUnownedTilesPerBarbarianCity(0),			
m_iBarbarianCreationTurnsElapsed(0),
m_iBarbarianCityCreationTurnsElapsed(0),
m_iBarbarianCityCreationProb(0),					
m_iAnimalCombatModifier(0),
m_iBarbarianCombatModifier(0),
m_iAIAnimalCombatModifier(0),
m_iAIBarbarianCombatModifier(0),
m_iStartingDefenseUnits(0),
m_iStartingWorkerUnits(0),
m_iStartingExploreUnits(0),
m_iAIStartingUnitMultiplier(0),					
m_iAIStartingDefenseUnits(0),				
m_iAIStartingWorkerUnits(0),					
m_iAIStartingExploreUnits(0),					
m_iBarbarianInitialDefenders(0),			
m_iAIDeclareWarProb(0),
m_iAIWorkRateModifier(0),
m_iAIGrowthPercent(0),
m_iAITrainPercent(0),
m_iAIWorldTrainPercent(0),
m_iAIConstructPercent(0),
m_iAIWorldConstructPercent(0),
m_iAICreatePercent(0),
m_iAIWorldCreatePercent(0),
m_iAICivicUpkeepPercent(0),
m_iAIUnitCostPercent(0),
m_iAIUnitSupplyPercent(0),
m_iAIUnitUpgradePercent(0),
m_iAIInflationPercent(0),
m_iAIWarWearinessPercent(0),
m_iAIPerEraModifier(0),
m_iNumGoodies(0),
m_piGoodies(NULL),
m_pbFreeTechs(NULL),
m_pbAIFreeTechs(NULL)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvHandicapInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvHandicapInfo::~CvHandicapInfo()
{
	SAFE_DELETE_ARRAY(m_piGoodies);
	SAFE_DELETE_ARRAY(m_pbFreeTechs);
	SAFE_DELETE_ARRAY(m_pbAIFreeTechs);
}

int CvHandicapInfo::getFreeWinsVsBarbs() const		
{
	return m_iFreeWinsVsBarbs;
}

int CvHandicapInfo::getAnimalAttackProb() const		
{
	return m_iAnimalAttackProb;
}

int CvHandicapInfo::getStartingLocationPercent() const
{
	return m_iStartingLocationPercent;
}

int CvHandicapInfo::getStartingGold() const				
{
	return m_iStartingGold;
}

int CvHandicapInfo::getFreeUnits() const					
{
	return m_iFreeUnits;
}

int CvHandicapInfo::getUnitCostPercent() const		
{
	return m_iUnitCostPercent;
}

int CvHandicapInfo::getResearchPercent() const		
{
	return m_iResearchPercent;
}

int CvHandicapInfo::getDistanceMaintenancePercent() const			
{
	return m_iDistanceMaintenancePercent;
}

int CvHandicapInfo::getNumCitiesMaintenancePercent() const		
{
	return m_iNumCitiesMaintenancePercent;
}

int CvHandicapInfo::getMaxNumCitiesMaintenance() const
{
	return m_iMaxNumCitiesMaintenance;
}

int CvHandicapInfo::getCivicUpkeepPercent() const	
{
	return m_iCivicUpkeepPercent;
}

int CvHandicapInfo::getInflationPercent() const		
{
	return m_iInflationPercent;
}

int CvHandicapInfo::getHealthBonus() const			
{
	return m_iHealthBonus;
}

int CvHandicapInfo::getHappyBonus() const				
{
	return m_iHappyBonus;
}

int CvHandicapInfo::getAttitudeChange() const
{
	return m_iAttitudeChange;
}

int CvHandicapInfo::getNoTechTradeModifier() const
{
	return m_iNoTechTradeModifier;
}

int CvHandicapInfo::getTechTradeKnownModifier() const
{
	return m_iTechTradeKnownModifier;
}

int CvHandicapInfo::getUnownedTilesPerGameAnimal() const			
{
	return m_iUnownedTilesPerGameAnimal;
}

int CvHandicapInfo::getUnownedTilesPerBarbarianUnit() const		
{
	return m_iUnownedTilesPerBarbarianUnit;
}

int CvHandicapInfo::getUnownedWaterTilesPerBarbarianUnit() const
{
	return m_iUnownedWaterTilesPerBarbarianUnit;
}

int CvHandicapInfo::getUnownedTilesPerBarbarianCity() const		
{
	return m_iUnownedTilesPerBarbarianCity;
}

int CvHandicapInfo::getBarbarianCreationTurnsElapsed() const	
{
	return m_iBarbarianCreationTurnsElapsed;
}

int CvHandicapInfo::getBarbarianCityCreationTurnsElapsed() const
{
	return m_iBarbarianCityCreationTurnsElapsed;
}

int CvHandicapInfo::getBarbarianCityCreationProb() const			
{
	return m_iBarbarianCityCreationProb;
}

int CvHandicapInfo::getAnimalCombatModifier() const
{
	return m_iAnimalCombatModifier;
}

int CvHandicapInfo::getBarbarianCombatModifier() const			
{
	return m_iBarbarianCombatModifier;
}

int CvHandicapInfo::getAIAnimalCombatModifier() const
{
	return m_iAIAnimalCombatModifier;
}

int CvHandicapInfo::getAIBarbarianCombatModifier() const		
{
	return m_iAIBarbarianCombatModifier;
}

int CvHandicapInfo::getStartingDefenseUnits() const
{
	return m_iStartingDefenseUnits; 
}

int CvHandicapInfo::getStartingWorkerUnits() const
{
	return m_iStartingWorkerUnits; 
}

int CvHandicapInfo::getStartingExploreUnits() const
{
	return m_iStartingExploreUnits; 
}

int CvHandicapInfo::getAIStartingUnitMultiplier() const
{
	return m_iAIStartingUnitMultiplier;
}

int CvHandicapInfo::getAIStartingDefenseUnits() const
{
	return m_iAIStartingDefenseUnits;
}

int CvHandicapInfo::getAIStartingWorkerUnits() const
{
	return m_iAIStartingWorkerUnits;
}

int CvHandicapInfo::getAIStartingExploreUnits() const
{
	return m_iAIStartingExploreUnits;
}

int CvHandicapInfo::getBarbarianInitialDefenders() const
{
	return m_iBarbarianInitialDefenders;
}

int CvHandicapInfo::getAIDeclareWarProb() const
{
	return m_iAIDeclareWarProb;
}

int CvHandicapInfo::getAIWorkRateModifier() const
{
	return m_iAIWorkRateModifier;
}

int CvHandicapInfo::getAIGrowthPercent() const		
{
	return m_iAIGrowthPercent;
}

int CvHandicapInfo::getAITrainPercent() const
{
	return m_iAITrainPercent;
}

int CvHandicapInfo::getAIWorldTrainPercent() const
{
	return m_iAIWorldTrainPercent;
}

int CvHandicapInfo::getAIConstructPercent() const	
{
	return m_iAIConstructPercent;
}

int CvHandicapInfo::getAIWorldConstructPercent() const	
{
	return m_iAIWorldConstructPercent;
}

int CvHandicapInfo::getAICreatePercent() const		
{
	return m_iAICreatePercent;
}

int CvHandicapInfo::getAIWorldCreatePercent() const		
{
	return m_iAIWorldCreatePercent;
}

int CvHandicapInfo::getAICivicUpkeepPercent() const
{
	return m_iAICivicUpkeepPercent;
}

int CvHandicapInfo::getAIUnitCostPercent() const	
{
	return m_iAIUnitCostPercent;
}

int CvHandicapInfo::getAIUnitSupplyPercent() const
{
	return m_iAIUnitSupplyPercent;
}

int CvHandicapInfo::getAIUnitUpgradePercent() const
{
	return m_iAIUnitUpgradePercent;
}

int CvHandicapInfo::getAIInflationPercent() const
{
	return m_iAIInflationPercent;
}

int CvHandicapInfo::getAIWarWearinessPercent() const
{
	return m_iAIWarWearinessPercent;
}

int CvHandicapInfo::getAIPerEraModifier() const		
{
	return m_iAIPerEraModifier;
}

int CvHandicapInfo::getNumGoodies() const					
{
	return m_iNumGoodies;
}

// Arrays

int CvHandicapInfo::getGoodies(int i) const				
{
	FAssertMsg(i < getNumGoodies(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piGoodies[i];
}

int CvHandicapInfo::isFreeTechs(int i) const
{
	FAssertMsg(i < GC.getNumTechInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbFreeTechs[i];
}

int CvHandicapInfo::isAIFreeTechs(int i) const
{
	FAssertMsg(i < GC.getNumTechInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbAIFreeTechs[i];
}

void CvHandicapInfo::read(FDataStreamBase* stream)
{
	CvInfoBase::read(stream);

	uint uiFlag=0;
	stream->Read(&uiFlag);		// Flag for Expansion

	stream->Read(&m_iFreeWinsVsBarbs);
	stream->Read(&m_iAnimalAttackProb);
	stream->Read(&m_iStartingLocationPercent);
	stream->Read(&m_iStartingGold);
	stream->Read(&m_iFreeUnits);
	stream->Read(&m_iUnitCostPercent);
	stream->Read(&m_iResearchPercent);
	stream->Read(&m_iDistanceMaintenancePercent);
	stream->Read(&m_iNumCitiesMaintenancePercent);
	stream->Read(&m_iMaxNumCitiesMaintenance);
	stream->Read(&m_iCivicUpkeepPercent);
	stream->Read(&m_iInflationPercent);
	stream->Read(&m_iHealthBonus);
	stream->Read(&m_iHappyBonus);
	stream->Read(&m_iAttitudeChange);
	stream->Read(&m_iNoTechTradeModifier);
	stream->Read(&m_iTechTradeKnownModifier);
	stream->Read(&m_iUnownedTilesPerGameAnimal);
	stream->Read(&m_iUnownedTilesPerBarbarianUnit);
	stream->Read(&m_iUnownedWaterTilesPerBarbarianUnit);
	stream->Read(&m_iUnownedTilesPerBarbarianCity);
	stream->Read(&m_iBarbarianCreationTurnsElapsed);
	stream->Read(&m_iBarbarianCityCreationTurnsElapsed);
	stream->Read(&m_iBarbarianCityCreationProb);
	stream->Read(&m_iAnimalCombatModifier);
	stream->Read(&m_iBarbarianCombatModifier);
	stream->Read(&m_iAIAnimalCombatModifier);
	stream->Read(&m_iAIBarbarianCombatModifier);

	stream->Read(&m_iStartingDefenseUnits);
	stream->Read(&m_iStartingWorkerUnits);
	stream->Read(&m_iStartingExploreUnits);
	stream->Read(&m_iAIStartingUnitMultiplier);
	stream->Read(&m_iAIStartingDefenseUnits);
	stream->Read(&m_iAIStartingWorkerUnits);
	stream->Read(&m_iAIStartingExploreUnits);
	stream->Read(&m_iBarbarianInitialDefenders);
	stream->Read(&m_iAIDeclareWarProb);
	stream->Read(&m_iAIWorkRateModifier);
	stream->Read(&m_iAIGrowthPercent);
	stream->Read(&m_iAITrainPercent);
	stream->Read(&m_iAIWorldTrainPercent);
	stream->Read(&m_iAIConstructPercent);
	stream->Read(&m_iAIWorldConstructPercent);
	stream->Read(&m_iAICreatePercent);
	stream->Read(&m_iAIWorldCreatePercent);
	stream->Read(&m_iAICivicUpkeepPercent);
	stream->Read(&m_iAIUnitCostPercent);
	stream->Read(&m_iAIUnitSupplyPercent);
	stream->Read(&m_iAIUnitUpgradePercent);
	stream->Read(&m_iAIInflationPercent);
	stream->Read(&m_iAIWarWearinessPercent);
	stream->Read(&m_iAIPerEraModifier);
	stream->Read(&m_iNumGoodies);

	stream->ReadString(m_szHandicapName);

	// Arrays

	SAFE_DELETE_ARRAY(m_piGoodies);
	m_piGoodies = new int[getNumGoodies()];
	stream->Read(getNumGoodies(), m_piGoodies);

	SAFE_DELETE_ARRAY(m_pbFreeTechs);
	m_pbFreeTechs = new bool[GC.getNumTechInfos()];
	stream->Read(GC.getNumTechInfos(), m_pbFreeTechs);

	SAFE_DELETE_ARRAY(m_pbAIFreeTechs);
	m_pbAIFreeTechs = new bool[GC.getNumTechInfos()];
	stream->Read(GC.getNumTechInfos(), m_pbAIFreeTechs);
}

void CvHandicapInfo::write(FDataStreamBase* stream)
{
	CvInfoBase::write(stream);

	uint uiFlag=0;
	stream->Write(uiFlag);		// Flag for Expansion

	stream->Write(m_iFreeWinsVsBarbs);
	stream->Write(m_iAnimalAttackProb);
	stream->Write(m_iStartingLocationPercent);
	stream->Write(m_iStartingGold);
	stream->Write(m_iFreeUnits);
	stream->Write(m_iUnitCostPercent);
	stream->Write(m_iResearchPercent);
	stream->Write(m_iDistanceMaintenancePercent);
	stream->Write(m_iNumCitiesMaintenancePercent);
	stream->Write(m_iMaxNumCitiesMaintenance);
	stream->Write(m_iCivicUpkeepPercent);
	stream->Write(m_iInflationPercent);
	stream->Write(m_iHealthBonus);
	stream->Write(m_iHappyBonus);
	stream->Write(m_iAttitudeChange);
	stream->Write(m_iNoTechTradeModifier);
	stream->Write(m_iTechTradeKnownModifier);
	stream->Write(m_iUnownedTilesPerGameAnimal);
	stream->Write(m_iUnownedTilesPerBarbarianUnit);
	stream->Write(m_iUnownedWaterTilesPerBarbarianUnit);
	stream->Write(m_iUnownedTilesPerBarbarianCity);
	stream->Write(m_iBarbarianCreationTurnsElapsed);
	stream->Write(m_iBarbarianCityCreationTurnsElapsed);
	stream->Write(m_iBarbarianCityCreationProb);
	stream->Write(m_iAnimalCombatModifier);
	stream->Write(m_iBarbarianCombatModifier);
	stream->Write(m_iAIAnimalCombatModifier);
	stream->Write(m_iAIBarbarianCombatModifier);

	stream->Write(m_iStartingDefenseUnits);
	stream->Write(m_iStartingWorkerUnits);
	stream->Write(m_iStartingExploreUnits);
	stream->Write(m_iAIStartingUnitMultiplier);
	stream->Write(m_iAIStartingDefenseUnits);
	stream->Write(m_iAIStartingWorkerUnits);
	stream->Write(m_iAIStartingExploreUnits);
	stream->Write(m_iBarbarianInitialDefenders);
	stream->Write(m_iAIDeclareWarProb);
	stream->Write(m_iAIWorkRateModifier);
	stream->Write(m_iAIGrowthPercent);
	stream->Write(m_iAITrainPercent);
	stream->Write(m_iAIWorldTrainPercent);
	stream->Write(m_iAIConstructPercent);
	stream->Write(m_iAIWorldConstructPercent);
	stream->Write(m_iAICreatePercent);
	stream->Write(m_iAIWorldCreatePercent);
	stream->Write(m_iAICivicUpkeepPercent);
	stream->Write(m_iAIUnitCostPercent);
	stream->Write(m_iAIUnitSupplyPercent);
	stream->Write(m_iAIUnitUpgradePercent);
	stream->Write(m_iAIInflationPercent);
	stream->Write(m_iAIWarWearinessPercent);
	stream->Write(m_iAIPerEraModifier);
	stream->Write(m_iNumGoodies);

	stream->WriteString(m_szHandicapName);

	// Arrays

	stream->Write(getNumGoodies(), m_piGoodies);
	stream->Write(GC.getNumTechInfos(), m_pbFreeTechs);
	stream->Write(GC.getNumTechInfos(), m_pbAIFreeTechs);
}

bool CvHandicapInfo::read(CvXMLLoadUtility* pXML)
{
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	int j;

	pXML->GetChildXmlValByName(&m_iFreeWinsVsBarbs, "iFreeWinsVsBarbs");
	pXML->GetChildXmlValByName(&m_iAnimalAttackProb, "iAnimalAttackProb");
	pXML->GetChildXmlValByName(&m_iStartingLocationPercent, "iStartingLocPercent");
	pXML->GetChildXmlValByName(&m_iStartingGold, "iGold");
	pXML->GetChildXmlValByName(&m_iFreeUnits, "iFreeUnits");
	pXML->GetChildXmlValByName(&m_iUnitCostPercent, "iUnitCostPercent");
	pXML->GetChildXmlValByName(&m_iResearchPercent, "iResearchPercent");
	pXML->GetChildXmlValByName(&m_iDistanceMaintenancePercent, "iDistanceMaintenancePercent");
	pXML->GetChildXmlValByName(&m_iNumCitiesMaintenancePercent, "iNumCitiesMaintenancePercent");
	pXML->GetChildXmlValByName(&m_iMaxNumCitiesMaintenance, "iMaxNumCitiesMaintenance");
	pXML->GetChildXmlValByName(&m_iCivicUpkeepPercent, "iCivicUpkeepPercent");
	pXML->GetChildXmlValByName(&m_iInflationPercent, "iInflationPercent");
	pXML->GetChildXmlValByName(&m_iHealthBonus, "iHealthBonus");
	pXML->GetChildXmlValByName(&m_iHappyBonus, "iHappyBonus");
	pXML->GetChildXmlValByName(&m_iAttitudeChange, "iAttitudeChange");
	pXML->GetChildXmlValByName(&m_iNoTechTradeModifier, "iNoTechTradeModifier");
	pXML->GetChildXmlValByName(&m_iTechTradeKnownModifier, "iTechTradeKnownModifier");
	pXML->GetChildXmlValByName(&m_iUnownedTilesPerGameAnimal, "iUnownedTilesPerGameAnimal");
	pXML->GetChildXmlValByName(&m_iUnownedTilesPerBarbarianUnit, "iUnownedTilesPerBarbarianUnit");
	pXML->GetChildXmlValByName(&m_iUnownedWaterTilesPerBarbarianUnit, "iUnownedWaterTilesPerBarbarianUnit");
	pXML->GetChildXmlValByName(&m_iUnownedTilesPerBarbarianCity, "iUnownedTilesPerBarbarianCity");
	pXML->GetChildXmlValByName(&m_iBarbarianCreationTurnsElapsed, "iBarbarianCreationTurnsElapsed");
	pXML->GetChildXmlValByName(&m_iBarbarianCityCreationTurnsElapsed, "iBarbarianCityCreationTurnsElapsed");
	pXML->GetChildXmlValByName(&m_iBarbarianCityCreationProb, "iBarbarianCityCreationProb");
	pXML->GetChildXmlValByName(&m_iAnimalCombatModifier, "iAnimalBonus");
	pXML->GetChildXmlValByName(&m_iBarbarianCombatModifier, "iBarbarianBonus");
	pXML->GetChildXmlValByName(&m_iAIAnimalCombatModifier, "iAIAnimalBonus");
	pXML->GetChildXmlValByName(&m_iAIBarbarianCombatModifier, "iAIBarbarianBonus");
	pXML->GetChildXmlValByName(&m_iStartingDefenseUnits, "iStartingDefenseUnits");
	pXML->GetChildXmlValByName(&m_iStartingWorkerUnits, "iStartingWorkerUnits");
	pXML->GetChildXmlValByName(&m_iStartingExploreUnits, "iStartingExploreUnits");
	pXML->GetChildXmlValByName(&m_iAIStartingUnitMultiplier, "iAIStartingUnitMultiplier");
	pXML->GetChildXmlValByName(&m_iAIStartingDefenseUnits, "iAIStartingDefenseUnits");
	pXML->GetChildXmlValByName(&m_iAIStartingWorkerUnits, "iAIStartingWorkerUnits");
	pXML->GetChildXmlValByName(&m_iAIStartingExploreUnits, "iAIStartingExploreUnits");
	pXML->GetChildXmlValByName(&m_iBarbarianInitialDefenders, "iBarbarianDefenders");
	pXML->GetChildXmlValByName(&m_iAIDeclareWarProb, "iAIDeclareWarProb");
	pXML->GetChildXmlValByName(&m_iAIWorkRateModifier, "iAIWorkRateModifier");
	pXML->GetChildXmlValByName(&m_iAIGrowthPercent, "iAIGrowthPercent");
	pXML->GetChildXmlValByName(&m_iAITrainPercent, "iAITrainPercent");
	pXML->GetChildXmlValByName(&m_iAIWorldTrainPercent, "iAIWorldTrainPercent");
	pXML->GetChildXmlValByName(&m_iAIConstructPercent, "iAIConstructPercent");
	pXML->GetChildXmlValByName(&m_iAIWorldConstructPercent, "iAIWorldConstructPercent");
	pXML->GetChildXmlValByName(&m_iAICreatePercent, "iAICreatePercent");
	pXML->GetChildXmlValByName(&m_iAIWorldCreatePercent, "iAIWorldCreatePercent");
	pXML->GetChildXmlValByName(&m_iAICivicUpkeepPercent, "iAICivicUpkeepPercent");
	pXML->GetChildXmlValByName(&m_iAIUnitCostPercent, "iAIUnitCostPercent");
	pXML->GetChildXmlValByName(&m_iAIUnitSupplyPercent, "iAIUnitSupplyPercent");
	pXML->GetChildXmlValByName(&m_iAIUnitUpgradePercent, "iAIUnitUpgradePercent");
	pXML->GetChildXmlValByName(&m_iAIInflationPercent, "iAIInflationPercent");
	pXML->GetChildXmlValByName(&m_iAIWarWearinessPercent, "iAIWarWearinessPercent");
	pXML->GetChildXmlValByName(&m_iAIPerEraModifier, "iAIPerEraModifier");

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(), "Goodies"))
	{
		CvString* pszGoodyNames = NULL;
		pXML->SetStringList(&pszGoodyNames, &m_iNumGoodies);

		if (m_iNumGoodies > 0)
		{
			m_piGoodies = new int[m_iNumGoodies];

			for (j=0;j<m_iNumGoodies;j++)
			{
				m_piGoodies[j] = pXML->FindInInfoClass(pszGoodyNames[j], GC.getGoodyInfo(), sizeof(GC.getGoodyInfo((GoodyTypes)0)), GC.getNumGoodyInfos());
			}
		}
		else
		{
			m_piGoodies = NULL;
		}

		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());

		SAFE_DELETE_ARRAY(pszGoodyNames);
	}

	pXML->SetVariableListTagPair(&m_pbFreeTechs, "FreeTechs", GC.getTechInfo(), sizeof(GC.getTechInfo((TechTypes)0)), GC.getNumTechInfos());
	pXML->SetVariableListTagPair(&m_pbAIFreeTechs, "AIFreeTechs", GC.getTechInfo(), sizeof(GC.getTechInfo((TechTypes)0)), GC.getNumTechInfos());

	return true;
}

//======================================================================================================
//					CvGameSpeedInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvGameSpeedInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvGameSpeedInfo::CvGameSpeedInfo() :
m_iGrowthPercent(0),
m_iTrainPercent(0),
m_iConstructPercent(0),
m_iCreatePercent(0),
m_iResearchPercent(0),
m_iBuildPercent(0),
m_iImprovementPercent(0),
m_iGreatPeoplePercent(0),
m_iAnarchyPercent(0),
m_iBarbPercent(0),
m_iFeatureProductionPercent(0),
m_iUnitDiscoverPercent(0),
m_iUnitHurryPercent(0),
m_iUnitTradePercent(0),
m_iUnitGreatWorkPercent(0),
m_iGoldenAgePercent(0),
m_iHurryPercent(0),
m_iHurryConscriptAngerPercent(0),
m_iInflationOffset(0),
m_iInflationPercent(0),
m_iNumTurnIncrements(0),
m_pGameTurnInfo(NULL)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvGameSpeedInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvGameSpeedInfo::~CvGameSpeedInfo()
{
	SAFE_DELETE_ARRAY(m_pGameTurnInfo);
}

int CvGameSpeedInfo::getGrowthPercent() const			
{
	return m_iGrowthPercent;
}

int CvGameSpeedInfo::getTrainPercent() const			
{
	return m_iTrainPercent;
}

int CvGameSpeedInfo::getConstructPercent() const	
{
	return m_iConstructPercent;
}

int CvGameSpeedInfo::getCreatePercent() const			
{
	return m_iCreatePercent;
}

int CvGameSpeedInfo::getResearchPercent() const		
{
	return m_iResearchPercent;
}

int CvGameSpeedInfo::getBuildPercent() const			
{
	return m_iBuildPercent;
}

int CvGameSpeedInfo::getImprovementPercent() const
{
	return m_iImprovementPercent;
}

int CvGameSpeedInfo::getGreatPeoplePercent() const
{
	return m_iGreatPeoplePercent;
}

int CvGameSpeedInfo::getAnarchyPercent() const		
{
	return m_iAnarchyPercent;
}

int CvGameSpeedInfo::getBarbPercent() const
{
	return m_iBarbPercent;
}

int CvGameSpeedInfo::getFeatureProductionPercent() const	
{
	return m_iFeatureProductionPercent;
}

int CvGameSpeedInfo::getUnitDiscoverPercent() const	
{
	return m_iUnitDiscoverPercent;
}

int CvGameSpeedInfo::getUnitHurryPercent() const
{
	return m_iUnitHurryPercent;
}

int CvGameSpeedInfo::getUnitTradePercent() const
{
	return m_iUnitTradePercent;
}

int CvGameSpeedInfo::getUnitGreatWorkPercent() const
{
	return m_iUnitGreatWorkPercent;
}

int CvGameSpeedInfo::getGoldenAgePercent() const
{
	return m_iGoldenAgePercent;
}

int CvGameSpeedInfo::getHurryPercent() const
{
	return m_iHurryPercent;
}

int CvGameSpeedInfo::getHurryConscriptAngerPercent() const
{
	return m_iHurryConscriptAngerPercent;
}

int CvGameSpeedInfo::getInflationOffset() const
{
	return m_iInflationOffset;
}

int CvGameSpeedInfo::getInflationPercent() const
{
	return m_iInflationPercent;
}

int CvGameSpeedInfo::getNumTurnIncrements() const
{
	return m_iNumTurnIncrements;
}

GameTurnInfo& CvGameSpeedInfo::getGameTurnInfo(int iIndex) const
{
	return m_pGameTurnInfo[iIndex];
}

void CvGameSpeedInfo::allocateGameTurnInfos(const int iSize)
{
	m_pGameTurnInfo = new GameTurnInfo[iSize];
}

bool CvGameSpeedInfo::read(CvXMLLoadUtility* pXML)
{
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	int j, iTempVal;

	pXML->GetChildXmlValByName(&m_iGrowthPercent, "iGrowthPercent");
	pXML->GetChildXmlValByName(&m_iTrainPercent, "iTrainPercent");
	pXML->GetChildXmlValByName(&m_iConstructPercent, "iConstructPercent");
	pXML->GetChildXmlValByName(&m_iCreatePercent, "iCreatePercent");
	pXML->GetChildXmlValByName(&m_iResearchPercent, "iResearchPercent");
	pXML->GetChildXmlValByName(&m_iBuildPercent, "iBuildPercent");
	pXML->GetChildXmlValByName(&m_iImprovementPercent, "iImprovementPercent");
	pXML->GetChildXmlValByName(&m_iGreatPeoplePercent, "iGreatPeoplePercent");
	pXML->GetChildXmlValByName(&m_iAnarchyPercent, "iAnarchyPercent");
	pXML->GetChildXmlValByName(&m_iBarbPercent, "iBarbPercent");
	pXML->GetChildXmlValByName(&m_iFeatureProductionPercent, "iFeatureProductionPercent");
	pXML->GetChildXmlValByName(&m_iUnitDiscoverPercent, "iUnitDiscoverPercent");
	pXML->GetChildXmlValByName(&m_iUnitHurryPercent, "iUnitHurryPercent");
	pXML->GetChildXmlValByName(&m_iUnitTradePercent, "iUnitTradePercent");
	pXML->GetChildXmlValByName(&m_iUnitGreatWorkPercent, "iUnitGreatWorkPercent");
	pXML->GetChildXmlValByName(&m_iGoldenAgePercent, "iGoldenAgePercent");
	pXML->GetChildXmlValByName(&m_iHurryPercent, "iHurryPercent");
	pXML->GetChildXmlValByName(&m_iHurryConscriptAngerPercent, "iHurryConscriptAngerPercent");
	pXML->GetChildXmlValByName(&m_iInflationOffset, "iInflationOffset");
	pXML->GetChildXmlValByName(&m_iInflationPercent, "iInflationPercent");

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"GameTurnInfos"))
	{
		m_iNumTurnIncrements = gDLL->getXMLIFace()->GetNumChildren(pXML->GetXML());
		if (gDLL->getXMLIFace()->SetToChild(pXML->GetXML()))
		{
			allocateGameTurnInfos(getNumTurnIncrements());

			// loop through each tag
			for (j=0;j<getNumTurnIncrements();j++)
			{
				pXML->GetChildXmlValByName(&iTempVal, "iYearIncrement");
				getGameTurnInfo(j).iYearIncrement = iTempVal;
				pXML->GetChildXmlValByName(&iTempVal, "iTurnsPerIncrement");
				getGameTurnInfo(j).iNumGameTurnsPerIncrement = iTempVal;

				// if we cannot set the current xml node to it's next sibling then we will break out of the for loop
				// otherwise we will continue looping
				if (!gDLL->getXMLIFace()->NextSibling(pXML->GetXML()))
				{
					break;
				}
			}

			gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
		}

		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	return true;
}


//======================================================================================================
//					CvTurnTimerInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvTurnTimerInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvTurnTimerInfo::CvTurnTimerInfo() :
m_iBaseTime(0),
m_iCityBonus(0),
m_iUnitBonus(0),
m_iFirstTurnMultiplier(0)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvTurnTimerInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvTurnTimerInfo::~CvTurnTimerInfo()
{
}

int CvTurnTimerInfo::getBaseTime() const			
{
	return m_iBaseTime;
}

int CvTurnTimerInfo::getCityBonus() const			
{
	return m_iCityBonus;
}

int CvTurnTimerInfo::getUnitBonus() const	
{
	return m_iUnitBonus;
}

int CvTurnTimerInfo::getFirstTurnMultiplier() const			
{
	return m_iFirstTurnMultiplier;
}

bool CvTurnTimerInfo::read(CvXMLLoadUtility* pXML)
{
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(&m_iBaseTime, "iBaseTime");
	pXML->GetChildXmlValByName(&m_iCityBonus, "iCityBonus");
	pXML->GetChildXmlValByName(&m_iUnitBonus, "iUnitBonus");
	pXML->GetChildXmlValByName(&m_iFirstTurnMultiplier, "iFirstTurnMultiplier");

	return true;
}

//======================================================================================================
//					CvBuildInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvBuildInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvBuildInfo::CvBuildInfo() :
m_iTime(0),
m_iTechPrereq(NO_TECH),
m_iImprovement(NO_IMPROVEMENT),
m_iRoute(NO_ROUTE),
m_iEntityEvent(ENTITY_EVENT_NONE),
m_iMissionType(NO_MISSION),
m_bKill(false),
m_paiFeatureTech(NULL),
m_paiFeatureTime(NULL),
m_paiFeatureProduction(NULL),
m_pabFeatureRemove(NULL)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvBuildInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvBuildInfo::~CvBuildInfo()
{
	SAFE_DELETE_ARRAY(m_paiFeatureTech);
	SAFE_DELETE_ARRAY(m_paiFeatureTime);
	SAFE_DELETE_ARRAY(m_paiFeatureProduction);
	SAFE_DELETE_ARRAY(m_pabFeatureRemove);
}

int CvBuildInfo::getTime() const
{
	return m_iTime;
}

int CvBuildInfo::getTechPrereq() const	
{
	return m_iTechPrereq;
}

int CvBuildInfo::getImprovement() const	
{
	return m_iImprovement;
}

int CvBuildInfo::getRoute() const
{
	return m_iRoute;
}

int CvBuildInfo::getEntityEvent() const	
{
	return m_iEntityEvent;
}

int CvBuildInfo::getMissionType() const
{
	return m_iMissionType;
}

void CvBuildInfo::setMissionType(int iNewType)
{
	m_iMissionType = iNewType;
}

bool CvBuildInfo::isKill() const	
{
	return m_bKill;
}

// Arrays

int CvBuildInfo::getFeatureTech(int i) const
{
	FAssertMsg(i < GC.getNumFeatureInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_paiFeatureTech ? m_paiFeatureTech[i] : -1;
}

int CvBuildInfo::getFeatureTime(int i) const
{
	FAssertMsg(i < GC.getNumFeatureInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_paiFeatureTime ? m_paiFeatureTime[i] : -1;
}

int CvBuildInfo::getFeatureProduction(int i) const
{
	FAssertMsg(i < GC.getNumFeatureInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_paiFeatureProduction ? m_paiFeatureProduction[i] : -1;
}

bool CvBuildInfo::isFeatureRemove(int i) const
{
	FAssertMsg(i < GC.getNumFeatureInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pabFeatureRemove ? m_pabFeatureRemove[i] : false;
}

bool CvBuildInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvHotkeyInfo::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(szTextVal, "PrereqTech");
	m_iTechPrereq = pXML->FindInInfoClass(szTextVal, GC.getTechInfo(), sizeof(GC.getTechInfo((TechTypes)0)), GC.getNumTechInfos());

	pXML->GetChildXmlValByName(&m_iTime, "iTime");
	pXML->GetChildXmlValByName(&m_bKill, "bKill");

	pXML->GetChildXmlValByName(szTextVal, "ImprovementType");
	m_iImprovement = pXML->FindInInfoClass(szTextVal, GC.getImprovementInfo(), sizeof(GC.getImprovementInfo((ImprovementTypes)0)), GC.getNumImprovementInfos());
	pXML->GetChildXmlValByName(szTextVal, "RouteType");
	m_iRoute = pXML->FindInInfoClass(szTextVal, GC.getRouteInfo(), sizeof(GC.getRouteInfo((RouteTypes)0)), GC.getNumRouteInfos());
	pXML->GetChildXmlValByName(szTextVal, "EntityEvent");
	m_iEntityEvent = pXML->FindInInfoClass(szTextVal, GC.getEntityEventInfo(), sizeof(GC.getEntityEventInfo((EntityEventTypes)0)), 
		GC.getNumEntityEventInfos());

	pXML->SetFeatureStruct(&m_paiFeatureTech, &m_paiFeatureTime, &m_paiFeatureProduction, &m_pabFeatureRemove);

	return true;
}

//======================================================================================================
//					CvGoodyInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvGoodyInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvGoodyInfo::CvGoodyInfo() :
m_iGold(0),							
m_iGoldRand1(0),				
m_iGoldRand2(0),				
m_iMapOffset(0),					
m_iMapRange(0),				
m_iMapProb(0),					
m_iExperience(0),				
m_iHealing(0),						
m_iDamagePrereq(0),			
m_iBarbarianUnitProb(0),	
m_iMinBarbarians(0),			
m_iUnitClassType(NO_UNITCLASS),
m_iBarbarianUnitClass(NO_UNITCLASS),
m_bTech(false),
m_bBad(false)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvGoodyInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvGoodyInfo::~CvGoodyInfo()
{
}

int CvGoodyInfo::getGold() const
{
	return m_iGold;
}

int CvGoodyInfo::getGoldRand1() const
{
	return m_iGoldRand1;
}

int CvGoodyInfo::getGoldRand2() const
{
	return m_iGoldRand2;
}

int CvGoodyInfo::getMapOffset() const
{
	return m_iMapOffset;
}

int CvGoodyInfo::getMapRange() const
{
	return m_iMapRange;
}

int CvGoodyInfo::getMapProb() const
{
	return m_iMapProb;
}

int CvGoodyInfo::getExperience() const			
{
	return m_iExperience;
}

int CvGoodyInfo::getHealing() const
{
	return m_iHealing;
}

int CvGoodyInfo::getDamagePrereq() const		
{
	return m_iDamagePrereq;
}

int CvGoodyInfo::getBarbarianUnitProb() const
{
	return m_iBarbarianUnitProb;
}

int CvGoodyInfo::getMinBarbarians() const		
{
	return m_iMinBarbarians;
}

int CvGoodyInfo::getUnitClassType() const		
{
	return m_iUnitClassType;
}

int CvGoodyInfo::getBarbarianUnitClass() const
{
	return m_iBarbarianUnitClass;
}

bool CvGoodyInfo::isTech() const
{
	return m_bTech;
}

bool CvGoodyInfo::isBad() const	
{
	return m_bBad;
}

const TCHAR* CvGoodyInfo::getSound() const	
{
	return m_szSound;
}

void CvGoodyInfo::setSound(const TCHAR* szVal)
{
	m_szSound=szVal;
}

bool CvGoodyInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(szTextVal, "Sound");
	setSound(szTextVal);

	pXML->GetChildXmlValByName(&m_iGold, "iGold");
	pXML->GetChildXmlValByName(&m_iGoldRand1, "iGoldRand1");
	pXML->GetChildXmlValByName(&m_iGoldRand2, "iGoldRand2");
	pXML->GetChildXmlValByName(&m_iMapOffset, "iMapOffset");
	pXML->GetChildXmlValByName(&m_iMapRange, "iMapRange");
	pXML->GetChildXmlValByName(&m_iMapProb, "iMapProb");
	pXML->GetChildXmlValByName(&m_iExperience, "iExperience");
	pXML->GetChildXmlValByName(&m_iHealing, "iHealing");
	pXML->GetChildXmlValByName(&m_iDamagePrereq, "iDamagePrereq");
	pXML->GetChildXmlValByName(&m_bTech, "bTech");
	pXML->GetChildXmlValByName(&m_bBad, "bBad");

	pXML->GetChildXmlValByName(szTextVal, "UnitClass");
	m_iUnitClassType = pXML->FindInInfoClass(szTextVal, GC.getUnitClassInfo(), sizeof(GC.getUnitClassInfo((UnitClassTypes)0)), GC.getNumUnitClassInfos());

	pXML->GetChildXmlValByName(szTextVal, "BarbarianClass");
	m_iBarbarianUnitClass = pXML->FindInInfoClass(szTextVal, GC.getUnitClassInfo(), sizeof(GC.getUnitClassInfo((UnitClassTypes)0)), GC.getNumUnitClassInfos());

	pXML->GetChildXmlValByName(&m_iBarbarianUnitProb, "iBarbarianUnitProb");
	pXML->GetChildXmlValByName(&m_iMinBarbarians, "iMinBarbarians");

	return true;
}

//======================================================================================================
//					CvRouteInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvRouteInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvRouteInfo::CvRouteInfo() :
m_iValue(0),
m_iMovementCost(0),
m_iFlatMovementCost(0),
m_iPrereqBonus(NO_BONUS),
m_piYieldChange(NULL),
m_piTechMovementChange(NULL)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvRouteInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvRouteInfo::~CvRouteInfo()
{
	SAFE_DELETE_ARRAY(m_piYieldChange);
	SAFE_DELETE_ARRAY(m_piTechMovementChange);	// free memory - MT
}

int CvRouteInfo::getValue() const		
{
	return m_iValue;
}

int CvRouteInfo::getMovementCost() const		
{
	return m_iMovementCost;
}

int CvRouteInfo::getFlatMovementCost() const
{
	return m_iFlatMovementCost;
}

int CvRouteInfo::getPrereqBonus() const			
{
	return m_iPrereqBonus;
}

// Arrays

int CvRouteInfo::getYieldChange(int i) const
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piYieldChange ? m_piYieldChange[i] : -1;
}

int CvRouteInfo::getTechMovementChange(int i) const
{
	FAssertMsg(i < GC.getNumTechInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piTechMovementChange ? m_piTechMovementChange[i] : -1;
}

bool CvRouteInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(&m_iValue, "iValue");
	pXML->GetChildXmlValByName(&m_iMovementCost, "iMovement");
	pXML->GetChildXmlValByName(&m_iFlatMovementCost, "iFlatMovement");

	pXML->GetChildXmlValByName(szTextVal, "BonusType");
	m_iPrereqBonus = pXML->FindInInfoClass(szTextVal, GC.getBonusInfo(), sizeof(GC.getBonusInfo((BonusTypes)0)), GC.getNumBonusInfos());

	// if we can set the current xml node to it's next sibling
	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"Yields"))
	{
		// call the function that sets the yield change variable
		pXML->SetYields(&m_piYieldChange);
		// set the current xml node to it's parent node
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piYieldChange, NUM_YIELD_TYPES);
	}

	pXML->SetVariableListTagPair(&m_piTechMovementChange, "TechMovementChanges", GC.getTechInfo(), sizeof(GC.getTechInfo((TechTypes)0)), GC.getNumTechInfos());

	return true;
}

//======================================================================================================
//					CvImprovementBonusInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvImprovementBonusInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvImprovementBonusInfo::CvImprovementBonusInfo() :
m_iDiscoverRand(0),
m_bBonusMakesValid(false),
m_bBonusTrade(false),
m_piYieldChange(NULL)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvImprovementBonusInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvImprovementBonusInfo::~CvImprovementBonusInfo()
{
	SAFE_DELETE_ARRAY(m_piYieldChange);
}

int CvImprovementBonusInfo::getDiscoverRand() const
{
	return m_iDiscoverRand;
}

bool CvImprovementBonusInfo::isBonusMakesValid() const
{
	return m_bBonusMakesValid;
}

bool CvImprovementBonusInfo::isBonusTrade() const
{
	return m_bBonusTrade;
}

int CvImprovementBonusInfo::getYieldChange(int i) const
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piYieldChange ? m_piYieldChange[i] : -1;
}

void CvImprovementBonusInfo::read(FDataStreamBase* stream)
{
	CvInfoBase::read(stream);

	uint uiFlag=0;
	stream->Read(&uiFlag);		// flag for expansion

	stream->Read(&m_iDiscoverRand);

	stream->Read(&m_bBonusMakesValid);
	stream->Read(&m_bBonusTrade);

	// Arrays

	SAFE_DELETE_ARRAY(m_piYieldChange);
	m_piYieldChange = new int[NUM_YIELD_TYPES];
	stream->Read(NUM_YIELD_TYPES, m_piYieldChange);
}

void CvImprovementBonusInfo::write(FDataStreamBase* stream)
{
	CvInfoBase::write(stream);

	uint uiFlag=0;
	stream->Write(uiFlag);		// flag for expansion

	stream->Write(m_iDiscoverRand);

	stream->Write(m_bBonusMakesValid);
	stream->Write(m_bBonusTrade);

	// Arrays
	
	stream->Write(NUM_YIELD_TYPES, m_piYieldChange);
}

//======================================================================================================
//					CvImprovementInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvImprovementInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvImprovementInfo::CvImprovementInfo() :
m_iTilesPerGoody(0),
m_iGoodyUniqueRange(0),
m_iUpgradeTime(0),
m_iAirBombDefense(0),
m_iDefenseModifier(0),
m_iPillageGold(0),
m_iImprovementPillage(NO_IMPROVEMENT),
m_iImprovementUpgrade(NO_IMPROVEMENT),
m_bActsAsCity(true),				
m_bHillsMakesValid(false),				
m_bFreshWaterMakesValid(false),	
m_bRiverSideMakesValid(false),	
m_bNoFreshWater(false),				
m_bRequiresFlatlands(false),							
m_bRequiresRiverSide(false),		
m_bRequiresIrrigation(false),		
m_bCarriesIrrigation(false),			
m_bRequiresFeature(false),
m_bWater(false),							
m_bGoody(false),							
m_bPermanent(false),							
m_bUseLSystem(false),
m_iWorldSoundscapeScriptId(0),
m_piPrereqNatureYield(NULL),
m_piYieldChange(NULL),
m_piRiverSideYieldChange(NULL),
m_piHillsYieldChange(NULL),
m_piIrrigatedChange(NULL),
m_pbTerrainMakesValid(NULL),
m_pbFeatureMakesValid(NULL),
m_ppiTechYieldChanges(NULL),
m_ppiRouteYieldChanges(NULL),
m_paImprovementBonus(NULL)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvImprovementInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvImprovementInfo::~CvImprovementInfo()
{
	int iI;

	SAFE_DELETE_ARRAY(m_piPrereqNatureYield);
	SAFE_DELETE_ARRAY(m_piYieldChange);
	SAFE_DELETE_ARRAY(m_piRiverSideYieldChange);
	SAFE_DELETE_ARRAY(m_piHillsYieldChange);
	SAFE_DELETE_ARRAY(m_piIrrigatedChange);
	SAFE_DELETE_ARRAY(m_pbTerrainMakesValid);
	SAFE_DELETE_ARRAY(m_pbFeatureMakesValid);

	if (m_paImprovementBonus != NULL)
	{
		SAFE_DELETE_ARRAY(m_paImprovementBonus); // XXX make sure this isn't leaking memory...
	}

	if (m_ppiTechYieldChanges != NULL)
	{
		for (iI=0;iI<GC.getNumTechInfos();iI++)
		{
			SAFE_DELETE_ARRAY(m_ppiTechYieldChanges[iI]);
		}
		SAFE_DELETE_ARRAY(m_ppiTechYieldChanges);
	}

	if (m_ppiRouteYieldChanges != NULL)
	{
		for (iI=0;iI<GC.getNumRouteInfos();iI++)
		{
			SAFE_DELETE_ARRAY(m_ppiRouteYieldChanges[iI]);
		}
		SAFE_DELETE_ARRAY(m_ppiRouteYieldChanges);
	}
}

int CvImprovementInfo::getTilesPerGoody() const
{
	return m_iTilesPerGoody;
}

int CvImprovementInfo::getGoodyUniqueRange() const
{
	return m_iGoodyUniqueRange; 
}

int CvImprovementInfo::getUpgradeTime() const
{
	return m_iUpgradeTime; 
}

int CvImprovementInfo::getAirBombDefense() const
{
	return m_iAirBombDefense; 
}

int CvImprovementInfo::getDefenseModifier() const
{
	return m_iDefenseModifier; 
}

int CvImprovementInfo::getPillageGold() const
{
	return m_iPillageGold; 
}

int CvImprovementInfo::getImprovementPillage() const			
{
	return m_iImprovementPillage; 
}

void CvImprovementInfo::setImprovementPillage(int i)
{
	m_iImprovementPillage = i; 
}

int CvImprovementInfo::getImprovementUpgrade() const			
{
	return m_iImprovementUpgrade; 
}

void CvImprovementInfo::setImprovementUpgrade(int i)
{
	m_iImprovementUpgrade = i; 
}

bool CvImprovementInfo::isActsAsCity() const
{
	return m_bActsAsCity; 
}

bool CvImprovementInfo::isHillsMakesValid() const
{
	return m_bHillsMakesValid; 
}

bool CvImprovementInfo::isFreshWaterMakesValid() const		
{
	return m_bFreshWaterMakesValid; 
}

bool CvImprovementInfo::isRiverSideMakesValid() const		
{
	return m_bRiverSideMakesValid; 
}

bool CvImprovementInfo::isNoFreshWater() const
{
	return m_bNoFreshWater; 
}

bool CvImprovementInfo::isRequiresFlatlands() const		
{
	return m_bRequiresFlatlands; 
}

bool CvImprovementInfo::isRequiresRiverSide() const			
{
	return m_bRequiresRiverSide; 
}

bool CvImprovementInfo::isRequiresIrrigation() const			
{
	return m_bRequiresIrrigation; 
}

bool CvImprovementInfo::isCarriesIrrigation() const			
{
	return m_bCarriesIrrigation; 
}

bool CvImprovementInfo::isRequiresFeature() const			
{
	return m_bRequiresFeature; 
}

bool CvImprovementInfo::isWater() const			
{
	return m_bWater; 
}

bool CvImprovementInfo::isGoody() const			
{
	return m_bGoody; 
}

bool CvImprovementInfo::isPermanent() const			
{
	return m_bPermanent; 
}

bool CvImprovementInfo::useLSystem() const
{
	return m_bUseLSystem;
}

void CvImprovementInfo::setUseLSystem(bool bUse)
{
	m_bUseLSystem = bUse;
}

const TCHAR* CvImprovementInfo::getImprovementPillageString() const
{
	return m_szImprovementPillage; 
}

void CvImprovementInfo::setImprovementPillageString(const TCHAR* szVal)
{
	m_szImprovementPillage=szVal; 
}

const TCHAR* CvImprovementInfo::getImprovementUpgradeString() const
{
	return m_szImprovementUpgrade; 
}

void CvImprovementInfo::setImprovementUpgradeString(const TCHAR* szVal)
{
	m_szImprovementUpgrade=szVal; 
}

const TCHAR* CvImprovementInfo::getArtDefineTag() const
{
	return m_szArtDefineTag; 
}

void CvImprovementInfo::setArtDefineTag(const TCHAR* szVal)
{
	m_szArtDefineTag = szVal; 
}

int CvImprovementInfo::getWorldSoundscapeScriptId() const
{
	return m_iWorldSoundscapeScriptId;
}

// Arrays

int CvImprovementInfo::getPrereqNatureYield(int i) const		
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piPrereqNatureYield ? m_piPrereqNatureYield[i] : -1; 
}

int* CvImprovementInfo::getPrereqNatureYieldArray()
{
	return m_piPrereqNatureYield;
}

int CvImprovementInfo::getYieldChange(int i) const
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piYieldChange ? m_piYieldChange[i] : -1; 
}

int* CvImprovementInfo::getYieldChangeArray() 
{
	return m_piYieldChange;
}

int CvImprovementInfo::getRiverSideYieldChange(int i) const	
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piRiverSideYieldChange ? m_piRiverSideYieldChange[i] : -1; 
}

int* CvImprovementInfo::getRiverSideYieldChangeArray()
{
	return m_piRiverSideYieldChange;
}

int CvImprovementInfo::getHillsYieldChange(int i) const
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piHillsYieldChange ? m_piHillsYieldChange[i] : -1; 
}

int* CvImprovementInfo::getHillsYieldChangeArray()
{
	return m_piHillsYieldChange;
}

int CvImprovementInfo::getIrrigatedYieldChange(int i) const	
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piIrrigatedChange ? m_piIrrigatedChange[i] : -1; 
}

int* CvImprovementInfo::getIrrigatedYieldChangeArray()		
{
	return m_piIrrigatedChange;
}

bool CvImprovementInfo::getTerrainMakesValid(int i) const
{
	FAssertMsg(i < GC.getNumTerrainInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbTerrainMakesValid ? m_pbTerrainMakesValid[i] : false; 
}

bool CvImprovementInfo::getFeatureMakesValid(int i) const
{
	FAssertMsg(i < GC.getNumFeatureInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbFeatureMakesValid ? m_pbFeatureMakesValid[i] : false; 
}

int CvImprovementInfo::getTechYieldChanges(int i, int j) const
{
	FAssertMsg(i < GC.getNumTechInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	FAssertMsg(j < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(j > -1, "Index out of bounds");
	return m_ppiTechYieldChanges[i][j]; 
}

int* CvImprovementInfo::getTechYieldChangesArray(int i)
{
	return m_ppiTechYieldChanges[i];
}

int CvImprovementInfo::getRouteYieldChanges(int i, int j) const
{
	FAssertMsg(i < GC.getNumRouteInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	FAssertMsg(j < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(j > -1, "Index out of bounds");
	return m_ppiRouteYieldChanges[i][j]; 
}

int* CvImprovementInfo::getRouteYieldChangesArray(int i)
{
	return m_ppiRouteYieldChanges[i];
}

int CvImprovementInfo::getImprovementBonusYield(int i, int j) const
{
	FAssertMsg(i < GC.getNumBonusInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	FAssertMsg(j < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(j > -1, "Index out of bounds");
	return m_paImprovementBonus[i].m_piYieldChange ? m_paImprovementBonus[i].getYieldChange(j) : -1; 
}

bool CvImprovementInfo::isImprovementBonusMakesValid(int i) const
{
	FAssertMsg(i < GC.getNumBonusInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_paImprovementBonus[i].m_bBonusMakesValid; 
}

bool CvImprovementInfo::isImprovementBonusTrade(int i) const
{
	FAssertMsg(i < GC.getNumBonusInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_paImprovementBonus[i].m_bBonusTrade; 
}

int CvImprovementInfo::getImprovementBonusDiscoverRand(int i) const
{
	FAssertMsg(i < GC.getNumBonusInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_paImprovementBonus[i].m_iDiscoverRand; 
}

const TCHAR* CvImprovementInfo::getButton() const
{
	const CvArtInfoImprovement * pImprovementArtInfo;
	pImprovementArtInfo = getArtInfo();
	if (pImprovementArtInfo != NULL)   
	{
		return pImprovementArtInfo->getButton();
	}
	return NULL;
}

const CvArtInfoImprovement* CvImprovementInfo::getArtInfo() const
{
	return ARTFILEMGR.getImprovementArtInfo(getArtDefineTag());
}

const TCHAR* CvArtInfoImprovement::getShaderNIF() const
{
	return m_szShaderNIF; 
}
void CvArtInfoImprovement::setShaderNIF(const TCHAR* szDesc)		
{
	m_szShaderNIF = szDesc; 
}

void CvImprovementInfo::read(FDataStreamBase* stream)
{
	CvInfoBase::read(stream);

	uint uiFlag=0;
	stream->Read(&uiFlag);		// flag for expansion

	stream->Read(&m_iTilesPerGoody);
	stream->Read(&m_iGoodyUniqueRange);
	stream->Read(&m_iUpgradeTime);
	stream->Read(&m_iAirBombDefense);
	stream->Read(&m_iDefenseModifier);
	stream->Read(&m_iPillageGold);
	stream->Read(&m_iImprovementPillage);
	stream->Read(&m_iImprovementUpgrade);

	stream->Read(&m_bActsAsCity);				
	stream->Read(&m_bHillsMakesValid);				
	stream->Read(&m_bFreshWaterMakesValid);	
	stream->Read(&m_bRiverSideMakesValid);	
	stream->Read(&m_bNoFreshWater);				
	stream->Read(&m_bRequiresFlatlands);							
	stream->Read(&m_bRequiresRiverSide);		
	stream->Read(&m_bRequiresIrrigation);		
	stream->Read(&m_bCarriesIrrigation);			
	stream->Read(&m_bRequiresFeature);
	stream->Read(&m_bWater);							
	stream->Read(&m_bGoody);							
	stream->Read(&m_bPermanent);							
	stream->Read(&m_bUseLSystem);

	stream->ReadString(m_szImprovementPillage);
	stream->ReadString(m_szImprovementUpgrade);
	stream->ReadString(m_szArtDefineTag);

	stream->Read(&m_iWorldSoundscapeScriptId);

	// Arrays

	SAFE_DELETE_ARRAY(m_piPrereqNatureYield);
	m_piPrereqNatureYield = new int[NUM_YIELD_TYPES];
	stream->Read(NUM_YIELD_TYPES, m_piPrereqNatureYield);

	SAFE_DELETE_ARRAY(m_piYieldChange);
	m_piYieldChange = new int[NUM_YIELD_TYPES];
	stream->Read(NUM_YIELD_TYPES, m_piYieldChange);

	SAFE_DELETE_ARRAY(m_piRiverSideYieldChange);
	m_piRiverSideYieldChange = new int[NUM_YIELD_TYPES];
	stream->Read(NUM_YIELD_TYPES, m_piRiverSideYieldChange);

	SAFE_DELETE_ARRAY(m_piHillsYieldChange);
	m_piHillsYieldChange = new int[NUM_YIELD_TYPES];
	stream->Read(NUM_YIELD_TYPES, m_piHillsYieldChange);

	SAFE_DELETE_ARRAY(m_piIrrigatedChange);
	m_piIrrigatedChange = new int[NUM_YIELD_TYPES];
	stream->Read(NUM_YIELD_TYPES, m_piIrrigatedChange);

	SAFE_DELETE_ARRAY(m_pbTerrainMakesValid);
	m_pbTerrainMakesValid = new bool[GC.getNumTerrainInfos()];
	stream->Read(GC.getNumTerrainInfos(), m_pbTerrainMakesValid);

	SAFE_DELETE_ARRAY(m_pbFeatureMakesValid);
	m_pbFeatureMakesValid = new bool[GC.getNumFeatureInfos()];
	stream->Read(GC.getNumFeatureInfos(), m_pbFeatureMakesValid);

	SAFE_DELETE_ARRAY(m_paImprovementBonus);
	m_paImprovementBonus = new CvImprovementBonusInfo[GC.getNumBonusInfos()];
	int i;
	for (i = 0; i < GC.getNumBonusInfos(); i++)
	{
		m_paImprovementBonus[i].read(stream);
	}

	if (m_ppiTechYieldChanges != NULL)
	{
		for(i=0;i<GC.getNumTechInfos();i++)
		{
			SAFE_DELETE_ARRAY(m_ppiTechYieldChanges[i]);
		}
		SAFE_DELETE_ARRAY(m_ppiTechYieldChanges);
	}

	m_ppiTechYieldChanges = new int*[GC.getNumTechInfos()];
	for(i=0;i<GC.getNumTechInfos();i++)
	{
		m_ppiTechYieldChanges[i]  = new int[NUM_YIELD_TYPES];
		stream->Read(NUM_YIELD_TYPES, m_ppiTechYieldChanges[i]);
	}

	if (m_ppiRouteYieldChanges != NULL)
	{
		for(i=0;i<GC.getNumRouteInfos();i++)
		{
			SAFE_DELETE_ARRAY(m_ppiRouteYieldChanges[i]);
		}
		SAFE_DELETE_ARRAY(m_ppiRouteYieldChanges);
	}

	m_ppiRouteYieldChanges = new int*[GC.getNumRouteInfos()];
	for(i=0;i<GC.getNumRouteInfos();i++)
	{
		m_ppiRouteYieldChanges[i]  = new int[NUM_YIELD_TYPES];
		stream->Read(NUM_YIELD_TYPES, m_ppiRouteYieldChanges[i]);
	}
}

void CvImprovementInfo::write(FDataStreamBase* stream)
{
	CvInfoBase::write(stream);

	uint uiFlag=0;
	stream->Write(uiFlag);		// flag for expansion

	stream->Write(m_iTilesPerGoody);
	stream->Write(m_iGoodyUniqueRange);
	stream->Write(m_iUpgradeTime);
	stream->Write(m_iAirBombDefense);
	stream->Write(m_iDefenseModifier);
	stream->Write(m_iPillageGold);
	stream->Write(m_iImprovementPillage);
	stream->Write(m_iImprovementUpgrade);

	stream->Write(m_bActsAsCity);				
	stream->Write(m_bHillsMakesValid);				
	stream->Write(m_bFreshWaterMakesValid);	
	stream->Write(m_bRiverSideMakesValid);	
	stream->Write(m_bNoFreshWater);				
	stream->Write(m_bRequiresFlatlands);							
	stream->Write(m_bRequiresRiverSide);		
	stream->Write(m_bRequiresIrrigation);		
	stream->Write(m_bCarriesIrrigation);			
	stream->Write(m_bRequiresFeature);
	stream->Write(m_bWater);							
	stream->Write(m_bGoody);							
	stream->Write(m_bPermanent);							
	stream->Write(m_bUseLSystem);

	stream->WriteString(m_szImprovementPillage);
	stream->WriteString(m_szImprovementUpgrade);
	stream->WriteString(m_szArtDefineTag);

	stream->Write(m_iWorldSoundscapeScriptId);

	// Arrays

	stream->Write(NUM_YIELD_TYPES, m_piPrereqNatureYield);
	stream->Write(NUM_YIELD_TYPES, m_piYieldChange);
	stream->Write(NUM_YIELD_TYPES, m_piRiverSideYieldChange);
	stream->Write(NUM_YIELD_TYPES, m_piHillsYieldChange);
	stream->Write(NUM_YIELD_TYPES, m_piIrrigatedChange);
	stream->Write(GC.getNumTerrainInfos(), m_pbTerrainMakesValid);
	stream->Write(GC.getNumFeatureInfos(), m_pbFeatureMakesValid);

	int i;
	for (i = 0; i < GC.getNumBonusInfos(); i++)
	{
		m_paImprovementBonus[i].write(stream);
	}

	for(i=0;i<GC.getNumTechInfos();i++)
	{
		stream->Write(NUM_YIELD_TYPES, m_ppiTechYieldChanges[i]);
	}

	for(i=0;i<GC.getNumRouteInfos();i++)
	{
		stream->Write(NUM_YIELD_TYPES, m_ppiRouteYieldChanges[i]);
	}
}
bool CvImprovementInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	int iIndex, j, iNumSibs;

	pXML->GetChildXmlValByName(szTextVal, "ArtDefineTag");
	setArtDefineTag(szTextVal);

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"PrereqNatureYields"))
	{
		// call the function that sets the yield change variable
		pXML->SetYields(&m_piPrereqNatureYield);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piPrereqNatureYield, NUM_YIELD_TYPES);
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"YieldChanges"))
	{
		// call the function that sets the yield change variable
		pXML->SetYields(&m_piYieldChange);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piYieldChange, NUM_YIELD_TYPES);
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"RiverSideYieldChange"))
	{
		// call the function that sets the yield change variable
		pXML->SetYields(&m_piRiverSideYieldChange);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piRiverSideYieldChange, NUM_YIELD_TYPES);
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"HillsYieldChange"))
	{
		// call the function that sets the yield change variable
		pXML->SetYields(&m_piHillsYieldChange);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piHillsYieldChange, NUM_YIELD_TYPES);
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"IrrigatedYieldChange"))
	{
		// call the function that sets the yield change variable
		pXML->SetYields(&m_piIrrigatedChange);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piIrrigatedChange, NUM_YIELD_TYPES);
	}

	pXML->GetChildXmlValByName(&m_bActsAsCity, "bActsAsCity");
	pXML->GetChildXmlValByName(&m_bHillsMakesValid, "bHillsMakesValid");
	pXML->GetChildXmlValByName(&m_bFreshWaterMakesValid, "bFreshWaterMakesValid");
	pXML->GetChildXmlValByName(&m_bRiverSideMakesValid, "bRiverSideMakesValid");
	pXML->GetChildXmlValByName(&m_bNoFreshWater, "bNoFreshWater");
	pXML->GetChildXmlValByName(&m_bRequiresFlatlands, "bRequiresFlatlands");
	pXML->GetChildXmlValByName(&m_bRequiresRiverSide, "bRequiresRiverSide");
	pXML->GetChildXmlValByName(&m_bRequiresIrrigation, "bRequiresIrrigation");
	pXML->GetChildXmlValByName(&m_bCarriesIrrigation, "bCarriesIrrigation");
	pXML->GetChildXmlValByName(&m_bRequiresFeature, "bRequiresFeature");
	pXML->GetChildXmlValByName(&m_bWater, "bWater");
	pXML->GetChildXmlValByName(&m_bGoody, "bGoody");
	pXML->GetChildXmlValByName(&m_bPermanent, "bPermanent");
	pXML->GetChildXmlValByName(&m_bUseLSystem, "bUseLSystem");
	pXML->GetChildXmlValByName(&m_iTilesPerGoody, "iTilesPerGoody");
	pXML->GetChildXmlValByName(&m_iGoodyUniqueRange, "iGoodyRange");
	pXML->GetChildXmlValByName(&m_iUpgradeTime, "iUpgradeTime");
	pXML->GetChildXmlValByName(&m_iAirBombDefense, "iAirBombDefense");
	pXML->GetChildXmlValByName(&m_iDefenseModifier, "iDefenseModifier");
	pXML->GetChildXmlValByName(&m_iPillageGold, "iPillageGold");

	pXML->SetVariableListTagPair(&m_pbTerrainMakesValid, "TerrainMakesValids", GC.getTerrainInfo(), sizeof(GC.getTerrainInfo((TerrainTypes)0)), GC.getNumTerrainInfos());
	pXML->SetVariableListTagPair(&m_pbFeatureMakesValid, "FeatureMakesValids", GC.getFeatureInfo(), sizeof(GC.getFeatureInfo((FeatureTypes)0)), GC.getNumFeatureInfos());

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"BonusTypeStructs"))
	{
		// call the function that sets the bonus booleans
		pXML->SetImprovementBonuses(&m_paImprovementBonus);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		// initialize the boolean list to the correct size and all the booleans to false
		pXML->InitImprovementBonusList(&m_paImprovementBonus, GC.getNumBonusInfos());
	}

	pXML->GetChildXmlValByName(szTextVal, "ImprovementPillage");
	setImprovementPillageString(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "ImprovementUpgrade");
	setImprovementUpgradeString(szTextVal);

	// initialize the boolean list to the correct size and all the booleans to false
	FAssertMsg((GC.getNumTechInfos() > 0) && (NUM_YIELD_TYPES) > 0,"either the number of tech infos is zero or less or the number of yield types is zero or less");
	pXML->Init2DIntList(&m_ppiTechYieldChanges, GC.getNumTechInfos(), NUM_YIELD_TYPES);
	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"TechYieldChanges"))
	{
		if (pXML->SkipToNextVal())
		{
			iNumSibs = gDLL->getXMLIFace()->GetNumChildren(pXML->GetXML());
			if (gDLL->getXMLIFace()->SetToChild(pXML->GetXML()))
			{
				if (0 < iNumSibs)
				{
					for (j=0;j<iNumSibs;j++)
					{
						pXML->GetChildXmlValByName(szTextVal, "PrereqTech");
						iIndex = pXML->FindInInfoClass(szTextVal, GC.getTechInfo(), sizeof(GC.getTechInfo((TechTypes)0)), GC.getNumTechInfos());

						if (iIndex > -1)
						{
							// delete the array since it will be reallocated
							SAFE_DELETE_ARRAY(m_ppiTechYieldChanges[iIndex]);
							// if we can set the current xml node to it's next sibling
							if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"TechYields"))
							{
								// call the function that sets the yield change variable
								pXML->SetYields(&m_ppiTechYieldChanges[iIndex]);
								gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
							}
							else
							{
								pXML->InitList(&m_ppiTechYieldChanges[iIndex], NUM_YIELD_TYPES);
							}
						}

						if (!gDLL->getXMLIFace()->NextSibling(pXML->GetXML()))
						{
							break;
						}
					}
				}

				gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
			}
		}

		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	// initialize the boolean list to the correct size and all the booleans to false
	FAssertMsg((GC.getNumRouteInfos() > 0) && (NUM_YIELD_TYPES) > 0,"either the number of route infos is zero or less or the number of yield types is zero or less");
	pXML->Init2DIntList(&m_ppiRouteYieldChanges, GC.getNumRouteInfos(), NUM_YIELD_TYPES);
	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"RouteYieldChanges"))
	{
		if (pXML->SkipToNextVal())
		{
			iNumSibs = gDLL->getXMLIFace()->GetNumChildren(pXML->GetXML());
			if (gDLL->getXMLIFace()->SetToChild(pXML->GetXML()))
			{

				if (0 < iNumSibs)
				{
					for (j=0;j<iNumSibs;j++)
					{
						pXML->GetChildXmlValByName(szTextVal, "RouteType");
						iIndex = pXML->FindInInfoClass(szTextVal, GC.getRouteInfo(), sizeof(GC.getRouteInfo((RouteTypes)0)), GC.getNumRouteInfos());

						if (iIndex > -1)
						{
							// delete the array since it will be reallocated
							SAFE_DELETE_ARRAY(m_ppiRouteYieldChanges[iIndex]);
							// if we can set the current xml node to it's next sibling
							if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"RouteYields"))
							{
								// call the function that sets the yield change variable
								pXML->SetYields(&m_ppiRouteYieldChanges[iIndex]);
								gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
							}
							else
							{
								pXML->InitList(&m_ppiRouteYieldChanges[iIndex], NUM_YIELD_TYPES);
							}
						}

						if (!gDLL->getXMLIFace()->NextSibling(pXML->GetXML()))
						{
							break;
						}
					}
				}

				gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
			}
		}

		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	pXML->GetChildXmlValByName(szTextVal, "WorldSoundscapeAudioScript");
	if ( szTextVal.GetLength() > 0 )
		m_iWorldSoundscapeScriptId = gDLL->getAudioTagIndex( szTextVal.GetCString(), AUDIOTAG_SOUNDSCAPE );
	else
		m_iWorldSoundscapeScriptId = -1;

	return true;
}

//======================================================================================================
//					CvBonusClassInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvBonusClassInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvBonusClassInfo::CvBonusClassInfo() :
m_iUniqueRange(0)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvBonusClassInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvBonusClassInfo::~CvBonusClassInfo()
{
}

int CvBonusClassInfo::getUniqueRange() const
{
	return m_iUniqueRange; 
}

bool CvBonusClassInfo::read(CvXMLLoadUtility* pXML)
{
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(&m_iUniqueRange, "iUnique");

	return true;
}

//======================================================================================================
//					CvBonusInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvBonusInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvBonusInfo::CvBonusInfo() :
m_iBonusClassType(NO_BONUSCLASS),
m_iChar(0),
m_iTechReveal(0),
m_iTechCityTrade(0),
m_iTechObsolete(0),
m_iAITradeModifier(0),
m_iAIObjective(0),
m_iHealth(0),
m_iHappiness(0),
m_iMinAreaSize(0),
m_iMinLatitude(0),
m_iMaxLatitude(0),
m_iPlacementOrder(0),
m_iConstAppearance(0),
m_iRandAppearance1(0),
m_iRandAppearance2(0),
m_iRandAppearance3(0),
m_iRandAppearance4(0),
m_iPercentPerPlayer(0),
m_iTilesPer(0),
m_iMinLandPercent(0),
m_iUniqueRange(0),
m_iGroupRange(0),
m_iGroupRand(0),
m_bOneArea(false),
m_bHills(false),
m_bFlatlands(false),
m_bNoRiverSide(false),
m_bNormalize(false),
m_bUseLSystem(false),
m_piYieldChange(NULL),
m_piImprovementChange(NULL),
m_pbTerrain(NULL),
m_pbFeature(NULL),
m_pbFeatureTerrain(NULL)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvBonusInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvBonusInfo::~CvBonusInfo()
{
	SAFE_DELETE_ARRAY(m_piYieldChange);
	SAFE_DELETE_ARRAY(m_piImprovementChange);
	SAFE_DELETE_ARRAY(m_pbTerrain);
	SAFE_DELETE_ARRAY(m_pbFeature);
	SAFE_DELETE_ARRAY(m_pbFeatureTerrain);	// free memory - MT
}

int CvBonusInfo::getBonusClassType() const
{
	return m_iBonusClassType; 
}

int CvBonusInfo::getChar() const
{
	return m_iChar; 
}

void CvBonusInfo::setChar(int i)
{
	m_iChar = i; 
}

int CvBonusInfo::getTechReveal() const
{
	return m_iTechReveal; 
}

int CvBonusInfo::getTechCityTrade() const
{
	return m_iTechCityTrade; 
}

int CvBonusInfo::getTechObsolete() const
{
	return m_iTechObsolete; 
}

int CvBonusInfo::getAITradeModifier() const
{
	return m_iAITradeModifier; 
}

int CvBonusInfo::getAIObjective() const
{
	return m_iAIObjective; 
}

int CvBonusInfo::getHealth() const
{
	return m_iHealth; 
}

int CvBonusInfo::getHappiness() const
{
	return m_iHappiness; 
}

int CvBonusInfo::getMinAreaSize() const
{
	return m_iMinAreaSize; 
}

int CvBonusInfo::getMinLatitude() const
{
	return m_iMinLatitude; 
}

int CvBonusInfo::getMaxLatitude() const
{
	return m_iMaxLatitude; 
}

int CvBonusInfo::getPlacementOrder() const
{
	return m_iPlacementOrder; 
}

int CvBonusInfo::getConstAppearance() const
{
	return m_iConstAppearance; 
}

int CvBonusInfo::getRandAppearance1() const
{
	return m_iRandAppearance1; 
}

int CvBonusInfo::getRandAppearance2() const
{
	return m_iRandAppearance2; 
}

int CvBonusInfo::getRandAppearance3() const
{
	return m_iRandAppearance3; 
}

int CvBonusInfo::getRandAppearance4() const
{
	return m_iRandAppearance4; 
}

int CvBonusInfo::getPercentPerPlayer() const
{
	return m_iPercentPerPlayer; 
}

int CvBonusInfo::getTilesPer() const
{
	return m_iTilesPer; 
}

int CvBonusInfo::getMinLandPercent() const		
{
	return m_iMinLandPercent; 
}

int CvBonusInfo::getUniqueRange() const
{
	return m_iUniqueRange; 
}

int CvBonusInfo::getGroupRange() const
{
	return m_iGroupRange; 
}

int CvBonusInfo::getGroupRand() const
{
	return m_iGroupRand; 
}

bool CvBonusInfo::isOneArea() const
{
	return m_bOneArea; 
}

bool CvBonusInfo::isHills() const
{
	return m_bHills; 
}

bool CvBonusInfo::isFlatlands() const
{
	return m_bFlatlands; 
}

bool CvBonusInfo::isNoRiverSide() const			
{
	return m_bNoRiverSide; 
}

bool CvBonusInfo::isNormalize() const			
{
	return m_bNormalize; 
}

bool CvBonusInfo::useLSystem() const
{
	return m_bUseLSystem;
}

void CvBonusInfo::setUseLSystem(bool bUse)
{
	m_bUseLSystem = bUse;
}

const TCHAR* CvBonusInfo::getArtDefineTag() const
{
	return m_szArtDefineTag; 
}

void CvBonusInfo::setArtDefineTag(const TCHAR* szVal)
{
	m_szArtDefineTag = szVal; 
}

// Arrays

int CvBonusInfo::getYieldChange(int i) const
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piYieldChange ? m_piYieldChange[i] : -1; 
}

int* CvBonusInfo::getYieldChangeArray()
{
	return m_piYieldChange;
}

int CvBonusInfo::getImprovementChange(int i) const
{
	FAssertMsg(i < GC.getNumImprovementInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piImprovementChange ? m_piImprovementChange[i] : -1; 
}

bool CvBonusInfo::isTerrain(int i) const	
{
	FAssertMsg(i < GC.getNumTerrainInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbTerrain ?	m_pbTerrain[i] : false; 
}

bool CvBonusInfo::isFeature(int i) const	
{
	FAssertMsg(i < GC.getNumFeatureInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbFeature ? m_pbFeature[i] : false; 
}

bool CvBonusInfo::isFeatureTerrain(int i) const	
{
	FAssertMsg(i < GC.getNumFeatureInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbFeatureTerrain ?	m_pbFeatureTerrain[i] : false; 
}

const TCHAR* CvBonusInfo::getButton() const
{
	const CvArtInfoBonus * pBonusArtInfo;
	pBonusArtInfo = getArtInfo();
	if (pBonusArtInfo != NULL)
	{
		return pBonusArtInfo->getButton();
	}
	else
	{
		return NULL;
	}
}

void CvBonusInfo::read(FDataStreamBase* stream)
{
	CvInfoBase::read(stream);

	uint uiFlag=0;
	stream->Read(&uiFlag);		// flag for expansion

	stream->Read(&m_iBonusClassType);
	stream->Read(&m_iChar);
	stream->Read(&m_iTechReveal);
	stream->Read(&m_iTechCityTrade);
	stream->Read(&m_iTechObsolete);
	stream->Read(&m_iAITradeModifier);
	stream->Read(&m_iAIObjective);
	stream->Read(&m_iHealth);
	stream->Read(&m_iHappiness);
	stream->Read(&m_iMinAreaSize);
	stream->Read(&m_iMinLatitude);
	stream->Read(&m_iMaxLatitude);
	stream->Read(&m_iPlacementOrder);
	stream->Read(&m_iConstAppearance);
	stream->Read(&m_iRandAppearance1);
	stream->Read(&m_iRandAppearance2);
	stream->Read(&m_iRandAppearance3);
	stream->Read(&m_iRandAppearance4);
	stream->Read(&m_iPercentPerPlayer);
	stream->Read(&m_iTilesPer);
	stream->Read(&m_iMinLandPercent);
	stream->Read(&m_iUniqueRange);
	stream->Read(&m_iGroupRange);
	stream->Read(&m_iGroupRand);

	stream->Read(&m_bOneArea);
	stream->Read(&m_bHills);
	stream->Read(&m_bFlatlands);
	stream->Read(&m_bNoRiverSide);
	stream->Read(&m_bNormalize);
	stream->Read(&m_bUseLSystem);

	stream->ReadString(m_szArtDefineTag);

	// Arrays

	SAFE_DELETE_ARRAY(m_piYieldChange);
	m_piYieldChange = new int[NUM_YIELD_TYPES];
	stream->Read(NUM_YIELD_TYPES, m_piYieldChange);

	SAFE_DELETE_ARRAY(m_piImprovementChange);
	m_piImprovementChange = new int[GC.getNumImprovementInfos()];
	stream->Read(GC.getNumImprovementInfos(), m_piImprovementChange);

	SAFE_DELETE_ARRAY(m_pbTerrain);
	m_pbTerrain = new bool[GC.getNumTerrainInfos()];
	stream->Read(GC.getNumTerrainInfos(), m_pbTerrain);

	SAFE_DELETE_ARRAY(m_pbFeature);
	m_pbFeature = new bool[GC.getNumFeatureInfos()];
	stream->Read(GC.getNumFeatureInfos(), m_pbFeature);

	SAFE_DELETE_ARRAY(m_pbFeatureTerrain);
	m_pbFeatureTerrain = new bool[GC.getNumTerrainInfos()];
	stream->Read(GC.getNumTerrainInfos(), m_pbFeatureTerrain);
}

void CvBonusInfo::write(FDataStreamBase* stream)
{
	CvInfoBase::write(stream);

	uint uiFlag=0;
	stream->Write(uiFlag);		// flag for expansion

	stream->Write(m_iBonusClassType);
	stream->Write(m_iChar);
	stream->Write(m_iTechReveal);
	stream->Write(m_iTechCityTrade);
	stream->Write(m_iTechObsolete);
	stream->Write(m_iAITradeModifier);
	stream->Write(m_iAIObjective);
	stream->Write(m_iHealth);
	stream->Write(m_iHappiness);
	stream->Write(m_iMinAreaSize);
	stream->Write(m_iMinLatitude);
	stream->Write(m_iMaxLatitude);
	stream->Write(m_iPlacementOrder);
	stream->Write(m_iConstAppearance);
	stream->Write(m_iRandAppearance1);
	stream->Write(m_iRandAppearance2);
	stream->Write(m_iRandAppearance3);
	stream->Write(m_iRandAppearance4);
	stream->Write(m_iPercentPerPlayer);
	stream->Write(m_iTilesPer);
	stream->Write(m_iMinLandPercent);
	stream->Write(m_iUniqueRange);
	stream->Write(m_iGroupRange);
	stream->Write(m_iGroupRand);

	stream->Write(m_bOneArea);
	stream->Write(m_bHills);
	stream->Write(m_bFlatlands);
	stream->Write(m_bNoRiverSide);
	stream->Write(m_bNormalize);
	stream->Write(m_bUseLSystem);

	stream->WriteString(m_szArtDefineTag);

	// Arrays

	stream->Write(NUM_YIELD_TYPES, m_piYieldChange);
	stream->Write(GC.getNumImprovementInfos(), m_piImprovementChange);
	stream->Write(GC.getNumTerrainInfos(), m_pbTerrain);
	stream->Write(GC.getNumFeatureInfos(), m_pbFeature);
	stream->Write(GC.getNumTerrainInfos(), m_pbFeatureTerrain);
}

bool CvBonusInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName( szTextVal, "BonusClassType");
	m_iBonusClassType = pXML->FindInInfoClass(szTextVal, GC.getBonusClassInfo(), sizeof(GC.getBonusClassInfo((BonusClassTypes)0)), GC.getNumBonusClassInfos());

	pXML->GetChildXmlValByName( szTextVal, "ArtDefineTag");
	setArtDefineTag(szTextVal);

	pXML->GetChildXmlValByName( szTextVal, "TechReveal");
	m_iTechReveal = pXML->FindInInfoClass(szTextVal, GC.getTechInfo(), sizeof(GC.getTechInfo((TechTypes)0)), GC.getNumTechInfos());

	pXML->GetChildXmlValByName( szTextVal, "TechCityTrade");
	m_iTechCityTrade = pXML->FindInInfoClass(szTextVal, GC.getTechInfo(), sizeof(GC.getTechInfo((TechTypes)0)), GC.getNumTechInfos());

	pXML->GetChildXmlValByName( szTextVal, "TechObsolete");
	m_iTechObsolete = pXML->FindInInfoClass(szTextVal, GC.getTechInfo(), sizeof(GC.getTechInfo((TechTypes)0)), GC.getNumTechInfos());

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"YieldChanges"))
	{
		pXML->SetYields(&m_piYieldChange);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piYieldChange, NUM_YIELD_TYPES);
	}

	pXML->GetChildXmlValByName(&m_iAITradeModifier, "iAITradeModifier");
	pXML->GetChildXmlValByName(&m_iAIObjective, "iAIObjective");
	pXML->GetChildXmlValByName(&m_iHealth, "iHealth");
	pXML->GetChildXmlValByName(&m_iHappiness, "iHappiness");
	pXML->GetChildXmlValByName(&m_iMinAreaSize, "iMinAreaSize");
	pXML->GetChildXmlValByName(&m_iMinLatitude, "iMinLatitude");
	pXML->GetChildXmlValByName(&m_iMaxLatitude, "iMaxLatitude");
	pXML->GetChildXmlValByName(&m_iPlacementOrder, "iPlacementOrder");
	pXML->GetChildXmlValByName(&m_iConstAppearance, "iConstAppearance");

	// if we can set the current xml node to it's next sibling
	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"Rands"))
	{
		pXML->GetChildXmlValByName(&m_iRandAppearance1, "iRandApp1");
		pXML->GetChildXmlValByName(&m_iRandAppearance2, "iRandApp2");
		pXML->GetChildXmlValByName(&m_iRandAppearance3, "iRandApp3");
		pXML->GetChildXmlValByName(&m_iRandAppearance4, "iRandApp4");

		// set the current xml node to it's parent node
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	pXML->GetChildXmlValByName(&m_iPercentPerPlayer, "iPlayer");
	pXML->GetChildXmlValByName(&m_iTilesPer, "iTilesPer");
	pXML->GetChildXmlValByName(&m_iMinLandPercent, "iMinLandPercent");
	pXML->GetChildXmlValByName(&m_iUniqueRange, "iUnique");
	pXML->GetChildXmlValByName(&m_iGroupRange, "iGroupRange");
	pXML->GetChildXmlValByName(&m_iGroupRand, "iGroupRand");
	pXML->GetChildXmlValByName(&m_bOneArea, "bArea");
	pXML->GetChildXmlValByName(&m_bHills, "bHills");
	pXML->GetChildXmlValByName(&m_bFlatlands, "bFlatlands");
	pXML->GetChildXmlValByName(&m_bNoRiverSide, "bNoRiverSide");
	pXML->GetChildXmlValByName(&m_bNormalize, "bNormalize");
	pXML->GetChildXmlValByName(&m_bUseLSystem, "bUseLSystem");

	pXML->SetVariableListTagPair(&m_pbTerrain, "TerrainBooleans", GC.getTerrainInfo(), sizeof(GC.getTerrainInfo((TerrainTypes)0)), GC.getNumTerrainInfos());
	pXML->SetVariableListTagPair(&m_pbFeature, "FeatureBooleans", GC.getFeatureInfo(), sizeof(GC.getFeatureInfo((FeatureTypes)0)), GC.getNumFeatureInfos());
	pXML->SetVariableListTagPair(&m_pbFeatureTerrain, "FeatureTerrainBooleans", GC.getTerrainInfo(), sizeof(GC.getTerrainInfo((TerrainTypes)0)), GC.getNumTerrainInfos());

	return true;
}

//======================================================================================================
//					CvFeatureInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvFeatureInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvFeatureInfo::CvFeatureInfo() :
m_iMovementCost(0),
m_iSeeThroughChange(0),
m_iHealthPercent(0),
m_iAppearanceProbability(0),
m_iDisappearanceProbability(0),
m_iGrowthProbability(0),
m_iDefenseModifier(0),
m_iNumVarieties(0),
m_bNoCoast(false),				
m_bNoRiver(false),					
m_bNoAdjacent(false),			
m_bRequiresFlatlands(false),
m_bRequiresRiver(false),
m_bAddsFreshWater(false),	
m_bImpassable(false),			
m_bNoCity(false),					
m_bNoImprovement(false),	
m_iWorldSoundscapeScriptId(0),
m_iEffectProbability(0),
m_piYieldChange(NULL),
m_piRiverYieldChange(NULL),
m_piHillsYieldChange(NULL),
m_pi3DAudioScriptFootstepIndex(NULL),
m_pbTerrain(NULL),
m_paszSymbolPath(NULL),
m_pasVarietyButton(NULL)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvFeatureInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvFeatureInfo::~CvFeatureInfo()
{
	SAFE_DELETE_ARRAY(m_piYieldChange);
	SAFE_DELETE_ARRAY(m_piRiverYieldChange);
	SAFE_DELETE_ARRAY(m_piHillsYieldChange);
	SAFE_DELETE_ARRAY(m_pi3DAudioScriptFootstepIndex);
	SAFE_DELETE_ARRAY(m_pbTerrain);
	SAFE_DELETE_ARRAY(m_paszSymbolPath);
	SAFE_DELETE_ARRAY(m_pasVarietyButton);
}

int CvFeatureInfo::getMovementCost() const
{
	return m_iMovementCost; 
}

int CvFeatureInfo::getSeeThroughChange() const			
{
	return m_iSeeThroughChange; 
}

int CvFeatureInfo::getHealthPercent() const
{
	return m_iHealthPercent; 
}

int CvFeatureInfo::getAppearanceProbability() const	
{
	return m_iAppearanceProbability; 
}

int CvFeatureInfo::getDisappearanceProbability() const	
{
	return m_iDisappearanceProbability; 
}

int CvFeatureInfo::getGrowthProbability() const			
{
	return m_iGrowthProbability; 
}

int CvFeatureInfo::getDefenseModifier() const			
{
	return m_iDefenseModifier; 
}

int CvFeatureInfo::getNumVarieties() const
{
	return m_iNumVarieties;
}

bool CvFeatureInfo::isNoCoast() const	
{
	return m_bNoCoast; 
}

bool CvFeatureInfo::isNoRiver() const	
{
	return m_bNoRiver; 
}

bool CvFeatureInfo::isNoAdjacent() const
{
	return m_bNoAdjacent; 
}

bool CvFeatureInfo::isRequiresFlatlands() const	
{
	return m_bRequiresFlatlands; 
}

bool CvFeatureInfo::isRequiresRiver() const			
{
	return m_bRequiresRiver; 
}

bool CvFeatureInfo::isAddsFreshWater() const
{
	return m_bAddsFreshWater; 
}

bool CvFeatureInfo::isImpassable() const
{
	return m_bImpassable; 
}

bool CvFeatureInfo::isNoCity() const		
{
	return m_bNoCity; 
}

bool CvFeatureInfo::isNoImprovement() const
{
	return m_bNoImprovement; 
}

const TCHAR* CvFeatureInfo::getArtDefineTag() const
{
	return m_szArtDefineTag; 
}

void CvFeatureInfo::setArtDefineTag(const TCHAR* szTag)
{
	m_szArtDefineTag = szTag; 
}

int CvFeatureInfo::getWorldSoundscapeScriptId() const
{
	return m_iWorldSoundscapeScriptId;
}

const TCHAR* CvFeatureInfo::getEffectType() const
{
	return m_szEffectType;
}

int CvFeatureInfo::getEffectProbability() const
{
	return m_iEffectProbability;
}

// Arrays

int CvFeatureInfo::getYieldChange(int i) const
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piYieldChange ? m_piYieldChange[i] : -1; 
}

int CvFeatureInfo::getRiverYieldChange(int i) const
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piRiverYieldChange ? m_piRiverYieldChange[i] : -1; 
}

int CvFeatureInfo::getHillsYieldChange(int i) const
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piHillsYieldChange ? m_piHillsYieldChange[i] : -1; 
}

int CvFeatureInfo::get3DAudioScriptFootstepIndex(int i) const
{
	//	FAssertMsg(i < ?, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pi3DAudioScriptFootstepIndex ? m_pi3DAudioScriptFootstepIndex[i] : -1;
}

bool CvFeatureInfo::isTerrain(int i) const
{
	FAssertMsg(i < GC.getNumTerrainInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbTerrain ? m_pbTerrain[i] : false; 
}

const TCHAR* CvFeatureInfo::getSymbolPath(int i) const
{
	FAssertMsg(i < getNumVarieties(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_paszSymbolPath[i];
}

const TCHAR* CvFeatureInfo::getVarietyButton(int i) const
{
	FAssertMsg(i < getNumVarieties(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pasVarietyButton[i];
}

const TCHAR* CvFeatureInfo::getButton() const
{
	const CvArtInfoFeature * pFeatureArtInfo;
	pFeatureArtInfo = getArtInfo();
	if (pFeatureArtInfo != NULL)
	{
		return pFeatureArtInfo->getButton();
	}
	else
	{
		return NULL;
	}
}

const CvArtInfoFeature* CvFeatureInfo::getArtInfo() const
{
	return ARTFILEMGR.getFeatureArtInfo( getArtDefineTag());
}

bool CvFeatureInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	int j, iNumSibs;

	pXML->GetChildXmlValByName( szTextVal, "ArtDefineTag");
	setArtDefineTag(szTextVal);

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"YieldChanges"))
	{
		pXML->SetYields(&m_piYieldChange);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piYieldChange, NUM_YIELD_TYPES);
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"RiverYieldChange"))
	{
		pXML->SetYields(&m_piRiverYieldChange);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piRiverYieldChange, NUM_YIELD_TYPES);
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"HillsYieldChange"))
	{
		pXML->SetYields(&m_piHillsYieldChange);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piHillsYieldChange, NUM_YIELD_TYPES);
	}

	pXML->GetChildXmlValByName(&m_iMovementCost, "iMovement");
	pXML->GetChildXmlValByName(&m_iSeeThroughChange, "iSeeThrough");
	pXML->GetChildXmlValByName(&m_iHealthPercent, "iHealthPercent");
	pXML->GetChildXmlValByName(&m_iDefenseModifier, "iDefense");
	pXML->GetChildXmlValByName(&m_iAppearanceProbability, "iAppearance");
	pXML->GetChildXmlValByName(&m_iDisappearanceProbability, "iDisappearance");
	pXML->GetChildXmlValByName(&m_iGrowthProbability, "iGrowth");
	pXML->GetChildXmlValByName(&m_bNoCoast, "bNoCoast");
	pXML->GetChildXmlValByName(&m_bNoRiver, "bNoRiver");
	pXML->GetChildXmlValByName(&m_bNoAdjacent, "bNoAdjacent");
	pXML->GetChildXmlValByName(&m_bRequiresFlatlands, "bRequiresFlatlands");
	pXML->GetChildXmlValByName(&m_bRequiresRiver, "bRequiresRiver");
	pXML->GetChildXmlValByName(&m_bAddsFreshWater, "bAddsFreshWater");
	pXML->GetChildXmlValByName(&m_bImpassable, "bImpassable");
	pXML->GetChildXmlValByName(&m_bNoCity, "bNoCity");
	pXML->GetChildXmlValByName(&m_bNoImprovement, "bNoImprovement");

	pXML->SetVariableListTagPairForAudioScripts(&m_pi3DAudioScriptFootstepIndex, "FootstepSounds", GC.getFootstepAudioTypes(), GC.getNumFootstepAudioTypes());

	pXML->GetChildXmlValByName(szTextVal, "WorldSoundscapeAudioScript");
	if ( szTextVal.GetLength() > 0 )
	{
		m_iWorldSoundscapeScriptId = gDLL->getAudioTagIndex( szTextVal.GetCString(), AUDIOTAG_SOUNDSCAPE );
	}
	else
	{
		m_iWorldSoundscapeScriptId = -1;
	}

	pXML->GetChildXmlValByName(m_szEffectType, "EffectType");
	pXML->GetChildXmlValByName(&m_iEffectProbability, "iEffectProbability");

	pXML->SetVariableListTagPair(&m_pbTerrain, "TerrainBooleans", GC.getTerrainInfo(), sizeof(GC.getTerrainInfo((TerrainTypes)0)), GC.getNumTerrainInfos());

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(), "SymbolPaths"))
	{
		if (pXML->SkipToNextVal())
		{
			m_iNumVarieties = gDLL->getXMLIFace()->GetNumChildren(pXML->GetXML());
			m_paszSymbolPath = new CvString[m_iNumVarieties];

			if (0 < m_iNumVarieties)
			{
				if (pXML->GetChildXmlVal(szTextVal))
				{
					for (j=0;j<m_iNumVarieties;j++)
					{
						m_paszSymbolPath[j] = szTextVal;
						if (!pXML->GetNextXmlVal(szTextVal))
						{
							break;
						}
					}

					gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
				}
			}
		}

		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(), "VarietyButtons"))
	{
		if (pXML->SkipToNextVal())
		{
			iNumSibs = gDLL->getXMLIFace()->GetNumChildren(pXML->GetXML());
			FAssert(iNumSibs == getNumVarieties());
			m_pasVarietyButton = new CvString[iNumSibs];

			if (0 < iNumSibs)
			{
				if (pXML->GetChildXmlVal(szTextVal))
				{
					for (j=0;j<iNumSibs;j++)
					{
						m_pasVarietyButton[j] = szTextVal;
						if (!pXML->GetNextXmlVal(szTextVal))
						{
							break;
						}
					}

					gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
				}
			}
		}

		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	return true;
}

//======================================================================================================
//					CvCommerceInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvCommerceInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvCommerceInfo::CvCommerceInfo() :
m_iChar(0),
m_iInitialPercent(0),
m_iInitialHappiness(0),
m_iAIWeightPercent(0),
m_bFlexiblePercent(false)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvCommerceInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvCommerceInfo::~CvCommerceInfo()
{
}

int CvCommerceInfo::getChar() const					
{
	return m_iChar; 
}

void CvCommerceInfo::setChar(int i)
{
	m_iChar = i; 
}

int CvCommerceInfo::getInitialPercent() const
{
	return m_iInitialPercent; 
}

int CvCommerceInfo::getInitialHappiness() const
{
	return m_iInitialHappiness; 
}

int CvCommerceInfo::getAIWeightPercent() const			
{
	return m_iAIWeightPercent; 
}

bool CvCommerceInfo::isFlexiblePercent() const
{
	return m_bFlexiblePercent; 
}

bool CvCommerceInfo::read(CvXMLLoadUtility* pXML)
{
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(&m_iInitialPercent, "iInitialPercent");
	pXML->GetChildXmlValByName(&m_iInitialHappiness, "iInitialHappiness");
	pXML->GetChildXmlValByName(&m_iAIWeightPercent, "iAIWeightPercent");
	pXML->GetChildXmlValByName(&m_bFlexiblePercent, "bFlexiblePercent");

	return true;
}

//======================================================================================================
//					CvYieldInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvYieldInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvYieldInfo::CvYieldInfo() :
m_iChar(0),
m_iHillsChange(0),
m_iPeakChange(0),
m_iLakeChange(0),
m_iCityChange(0),
m_iPopulationChangeOffset(0),
m_iPopulationChangeDivisor(0),
m_iMinCity(0),
m_iTradeModifier(0),
m_iGoldenAgeYield(0),
m_iGoldenAgeYieldThreshold(0),
m_iAIWeightPercent(0),
m_iColorType(NO_COLOR),
m_paszSymbolPath(NULL)
{
}


//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvYieldInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvYieldInfo::~CvYieldInfo()
{
	SAFE_DELETE_ARRAY(m_paszSymbolPath);
}

int CvYieldInfo::getChar() const			
{
	return m_iChar; 
}

void CvYieldInfo::setChar(int i)
{
	m_iChar = i; 
}

int CvYieldInfo::getHillsChange() const
{
	return m_iHillsChange; 
}

int CvYieldInfo::getPeakChange() const
{
	return m_iPeakChange; 
}

int CvYieldInfo::getLakeChange() const
{
	return m_iLakeChange; 
}

int CvYieldInfo::getCityChange() const
{
	return m_iCityChange; 
}

int CvYieldInfo::getPopulationChangeOffset() const
{
	return m_iPopulationChangeOffset; 
}

int CvYieldInfo::getPopulationChangeDivisor() const
{
	return m_iPopulationChangeDivisor; 
}

int CvYieldInfo::getMinCity() const		
{
	return m_iMinCity; 
}

int CvYieldInfo::getTradeModifier() const
{
	return m_iTradeModifier; 
}

int CvYieldInfo::getGoldenAgeYield() const
{
	return m_iGoldenAgeYield; 
}

int CvYieldInfo::getGoldenAgeYieldThreshold() const
{
	return m_iGoldenAgeYieldThreshold; 
}

int CvYieldInfo::getAIWeightPercent() const	
{
	return m_iAIWeightPercent; 
}

int CvYieldInfo::getColorType() const
{
	return m_iColorType;
}

// Arrays

const TCHAR* CvYieldInfo::getSymbolPath(int i) const
{
	FAssertMsg(i < GC.getDefineINT("MAX_YIELD_STACK"), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_paszSymbolPath ? m_paszSymbolPath[i] : -1; 
}

bool CvYieldInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	int iNumSibs, j;

	pXML->GetChildXmlValByName(&m_iHillsChange, "iHillsChange");
	pXML->GetChildXmlValByName(&m_iPeakChange, "iPeakChange");
	pXML->GetChildXmlValByName(&m_iLakeChange, "iLakeChange");
	pXML->GetChildXmlValByName(&m_iCityChange, "iCityChange");
	pXML->GetChildXmlValByName(&m_iPopulationChangeOffset, "iPopulationChangeOffset");
	pXML->GetChildXmlValByName(&m_iPopulationChangeDivisor, "iPopulationChangeDivisor");
	pXML->GetChildXmlValByName(&m_iMinCity, "iMinCity");
	pXML->GetChildXmlValByName(&m_iTradeModifier, "iTradeModifier");
	pXML->GetChildXmlValByName(&m_iGoldenAgeYield, "iGoldenAgeYield");
	pXML->GetChildXmlValByName(&m_iGoldenAgeYieldThreshold, "iGoldenAgeYieldThreshold");
	pXML->GetChildXmlValByName(&m_iAIWeightPercent, "iAIWeightPercent");

	pXML->GetChildXmlValByName(szTextVal, "ColorType");
	m_iColorType = pXML->FindInInfoClass(szTextVal, GC.getColorInfo(), sizeof(GC.getColorInfo((ColorTypes)0)), GC.getNumColorInfos());

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(), "SymbolPaths"))
	{
		if (pXML->SkipToNextVal())
		{
			iNumSibs = gDLL->getXMLIFace()->GetNumChildren(pXML->GetXML());
			FAssertMsg((0 < GC.getDefineINT("MAX_YIELD_STACK")) ,"Allocating zero or less memory in SetGlobalYieldInfo");
			m_paszSymbolPath = new CvString[GC.getDefineINT("MAX_YIELD_STACK")];

			if (0 < iNumSibs)
			{
				if (pXML->GetChildXmlVal(szTextVal))
				{
					FAssertMsg((iNumSibs <= GC.getDefineINT("MAX_YIELD_STACK")) ,"There are more siblings than memory allocated for them in SetGlobalYieldInfo");
					for (j=0;j<iNumSibs;j++)
					{
						m_paszSymbolPath[j] = szTextVal;
						if (!pXML->GetNextXmlVal(szTextVal))
						{
							break;
						}
					}

					gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
				}
			}
		}

		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	return true;
}

//======================================================================================================
//					CvTerrainInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvTerrainInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvTerrainInfo::CvTerrainInfo() :
m_iMovementCost(0),
m_iSeeFromLevel(0),	
m_iSeeThroughLevel(0),
m_iBuildModifier(0),
m_iDefenseModifier(0),
m_bWater(false),
m_bImpassable(false),
m_bFound(false),
m_bFoundCoast(false),
m_bFoundFreshWater(false),
m_iWorldSoundscapeScriptId(0),
m_piYields(NULL),
m_piRiverYieldChange(NULL),
m_piHillsYieldChange(NULL),
m_pi3DAudioScriptFootstepIndex(NULL)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvTerrainInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvTerrainInfo::~CvTerrainInfo()
{
	SAFE_DELETE_ARRAY(m_piYields);
	SAFE_DELETE_ARRAY(m_piRiverYieldChange);
	SAFE_DELETE_ARRAY(m_piHillsYieldChange);
	SAFE_DELETE_ARRAY(m_pi3DAudioScriptFootstepIndex);
}

int CvTerrainInfo::getMovementCost() const	
{
	return m_iMovementCost; 
}

int CvTerrainInfo::getSeeFromLevel() const	
{
	return m_iSeeFromLevel; 
}

int CvTerrainInfo::getSeeThroughLevel() const
{
	return m_iSeeThroughLevel; 
}

int CvTerrainInfo::getBuildModifier() const	
{
	return m_iBuildModifier; 
}

int CvTerrainInfo::getDefenseModifier() const
{
	return m_iDefenseModifier; 
}

bool CvTerrainInfo::isWater() const				
{
	return m_bWater; 
}

bool CvTerrainInfo::isImpassable() const		
{
	return m_bImpassable; 
}

bool CvTerrainInfo::isFound() const		
{
	return m_bFound; 
}

bool CvTerrainInfo::isFoundCoast() const		
{
	return m_bFoundCoast; 
}

bool CvTerrainInfo::isFoundFreshWater() const		
{
	return m_bFoundFreshWater; 
}

const TCHAR* CvTerrainInfo::getArtDefineTag() const
{
	return m_szArtDefineTag; 
}

void CvTerrainInfo::setArtDefineTag(const TCHAR* szTag)
{
	m_szArtDefineTag = szTag; 
}

int CvTerrainInfo::getWorldSoundscapeScriptId() const
{
	return m_iWorldSoundscapeScriptId;
}

// Arrays

int CvTerrainInfo::getYield(int i) const	
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piYields ? m_piYields[i] : -1; 
}

int CvTerrainInfo::getRiverYieldChange(int i) const
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piRiverYieldChange ? m_piRiverYieldChange[i] : -1; 
}

int CvTerrainInfo::getHillsYieldChange(int i) const
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piHillsYieldChange ? m_piHillsYieldChange[i] : -1; 
}

int CvTerrainInfo::get3DAudioScriptFootstepIndex(int i) const
{
//	FAssertMsg(i < ?, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pi3DAudioScriptFootstepIndex ? m_pi3DAudioScriptFootstepIndex[i] : -1;
}

bool CvTerrainInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName( szTextVal, "ArtDefineTag");
	setArtDefineTag(szTextVal);

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"Yields"))
	{
		pXML->SetYields(&m_piYields);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piYields, NUM_YIELD_TYPES);
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"RiverYieldChange"))
	{
		pXML->SetYields(&m_piRiverYieldChange);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piRiverYieldChange, NUM_YIELD_TYPES);
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"HillsYieldChange"))
	{
		pXML->SetYields(&m_piHillsYieldChange);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piHillsYieldChange, NUM_YIELD_TYPES);
	}

	pXML->GetChildXmlValByName(&m_bWater, "bWater");
	pXML->GetChildXmlValByName(&m_bImpassable, "bImpassable");
	pXML->GetChildXmlValByName(&m_bFound, "bFound");
	pXML->GetChildXmlValByName(&m_bFoundCoast, "bFoundCoast");
	pXML->GetChildXmlValByName(&m_bFoundFreshWater, "bFoundFreshWater");

	pXML->GetChildXmlValByName(&m_iMovementCost, "iMovement");
	pXML->GetChildXmlValByName(&m_iSeeFromLevel, "iSeeFrom");
	pXML->GetChildXmlValByName(&m_iSeeThroughLevel, "iSeeThrough");
	pXML->GetChildXmlValByName(&m_iBuildModifier, "iBuildModifier");
	pXML->GetChildXmlValByName(&m_iDefenseModifier, "iDefense");

	pXML->SetVariableListTagPairForAudioScripts(&m_pi3DAudioScriptFootstepIndex, "FootstepSounds", GC.getFootstepAudioTypes(), GC.getNumFootstepAudioTypes());

	pXML->GetChildXmlValByName(szTextVal, "WorldSoundscapeAudioScript");
	if ( szTextVal.GetLength() > 0 )
		m_iWorldSoundscapeScriptId = gDLL->getAudioTagIndex( szTextVal.GetCString(), AUDIOTAG_SOUNDSCAPE );
	else
		m_iWorldSoundscapeScriptId = -1;

	return true;
}

const TCHAR* CvTerrainInfo::getButton() const
{
	const CvArtInfoTerrain * pTerrainArtInfo;
	pTerrainArtInfo = getArtInfo();
	if (pTerrainArtInfo != NULL)
	{
		return pTerrainArtInfo->getButton();
	}
	else
	{
		return NULL;
	}
}

const CvArtInfoTerrain* CvTerrainInfo::getArtInfo() const
{
	return ARTFILEMGR.getTerrainArtInfo( getArtDefineTag());
}

//======================================================================================================
//					CvInterfaceModeInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvInterfaceModeInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvInterfaceModeInfo::CvInterfaceModeInfo() :
m_iCursorIndex(NO_CURSOR),
m_iMissionType(NO_MISSION),
m_bVisible(false),
m_bGotoPlot(false),
m_bHighlightPlot(false),
m_bSelectType(false),
m_bSelectAll(false)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvInterfaceModeInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvInterfaceModeInfo::~CvInterfaceModeInfo()
{
}

int CvInterfaceModeInfo::getCursorIndex() const
{
	return m_iCursorIndex; 
}

int CvInterfaceModeInfo::getMissionType() const
{
	return m_iMissionType; 
}

bool CvInterfaceModeInfo::getVisible() const
{
	return m_bVisible;
}

bool CvInterfaceModeInfo::getGotoPlot() const
{
	return m_bGotoPlot;
}

bool CvInterfaceModeInfo::getHighlightPlot() const
{
	return m_bHighlightPlot;
}

bool CvInterfaceModeInfo::getSelectType() const
{
	return m_bSelectType;
}

bool CvInterfaceModeInfo::getSelectAll() const
{
	return m_bSelectAll;
}

bool CvInterfaceModeInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvHotkeyInfo::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(szTextVal, "CursorType");
	m_iCursorIndex = pXML->FindInInfoClass( szTextVal, GC.getCursorInfo(), sizeof(GC.getCursorInfo((CursorTypes)0)), GC.getNumCursorInfos());

	pXML->GetChildXmlValByName(szTextVal, "Mission");
	m_iMissionType = pXML->FindInInfoClass(szTextVal, GC.getMissionInfo(), sizeof(GC.getMissionInfo((MissionTypes)0)), NUM_MISSION_TYPES);

	pXML->GetChildXmlValByName(&m_bVisible, "bVisible");
	pXML->GetChildXmlValByName(&m_bGotoPlot, "bGotoPlot");
	pXML->GetChildXmlValByName(&m_bHighlightPlot, "bHighlightPlot");
	pXML->GetChildXmlValByName(&m_bSelectType, "bSelectType");
	pXML->GetChildXmlValByName(&m_bSelectAll, "bSelectAll");

	return true;
}

//======================================================================================================
//					CvAdvisorInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvAdvisorInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvAdvisorInfo::CvAdvisorInfo()
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvAdvisorInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvAdvisorInfo::~CvAdvisorInfo()
{
}

const TCHAR* CvAdvisorInfo::getTexture() const	
{
	return m_szTexture; 
}

void CvAdvisorInfo::setTexture(const TCHAR* szVal)
{
	m_szTexture = szVal; 
}

int CvAdvisorInfo::getNumCodes() const
{
	return m_vctEnableDisableCodes.size();
}

int CvAdvisorInfo::getEnableCode(uint uiCode) const
{
	FAssert( uiCode < m_vctEnableDisableCodes.size() );
	return m_vctEnableDisableCodes[uiCode].first;
}

int CvAdvisorInfo::getDisableCode(uint uiCode) const
{
	FAssert( uiCode < m_vctEnableDisableCodes.size() );
	return m_vctEnableDisableCodes[uiCode].second;
}

bool CvAdvisorInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(szTextVal, "Texture");
	setTexture( szTextVal );

	gDLL->getXMLIFace()->SetToChild( pXML->GetXML() );
	while(gDLL->getXMLIFace()->LocateNextSiblingNodeByTagName(pXML->GetXML(), "EventCodes"))
	{
		int iEnableCode, iDisableCode;
		pXML->GetChildXmlValByName(&iEnableCode, "iEnableCode");
		pXML->GetChildXmlValByName(&iDisableCode, "iDisableCode");
		m_vctEnableDisableCodes.push_back( std::make_pair( iEnableCode, iDisableCode ));
	}
	gDLL->getXMLIFace()->SetToParent( pXML->GetXML() );

	return true;
}

//======================================================================================================
//					CvLeaderHeadInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvLeaderHeadInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvLeaderHeadInfo::CvLeaderHeadInfo() :
m_iWonderConstructRand(0),
m_iBaseAttitude(0),
m_iBasePeaceWeight(0),
m_iPeaceWeightRand(0),
m_iWarmongerRespect(0),
m_iRefuseToTalkWarThreshold(0),
m_iNoTechTradeThreshold(0),
m_iTechTradeKnownPercent(0),
m_iMaxGoldTradePercent(0),
m_iMaxGoldPerTurnTradePercent(0),
m_iMaxWarRand(0),
m_iMaxWarNearbyPowerRatio(0),
m_iMaxWarDistantPowerRatio(0),
m_iMaxWarMinAdjacentLandPercent(0),
m_iLimitedWarRand(0),
m_iLimitedWarPowerRatio(0),
m_iDogpileWarRand(0),
m_iMakePeaceRand(0),
m_iDeclareWarTradeRand(0),
m_iDemandRebukedSneakProb(0),
m_iDemandRebukedWarProb(0),
m_iRazeCityProb(0),
m_iBuildUnitProb(0),
m_iBaseAttackOddsChange(0),
m_iAttackOddsChangeRand(0),
m_iWorseRankDifferenceAttitudeChange(0),
m_iBetterRankDifferenceAttitudeChange(0),
m_iCloseBordersAttitudeChange(0),
m_iLostWarAttitudeChange(0),
m_iAtWarAttitudeDivisor(0),
m_iAtWarAttitudeChangeLimit(0),
m_iAtPeaceAttitudeDivisor(0),
m_iAtPeaceAttitudeChangeLimit(0),
m_iSameReligionAttitudeChange(0),
m_iSameReligionAttitudeDivisor(0),
m_iSameReligionAttitudeChangeLimit(0),
m_iDifferentReligionAttitudeChange(0),
m_iDifferentReligionAttitudeDivisor(0),
m_iDifferentReligionAttitudeChangeLimit(0),
m_iBonusTradeAttitudeDivisor(0),
m_iBonusTradeAttitudeChangeLimit(0),
m_iOpenBordersAttitudeDivisor(0),
m_iOpenBordersAttitudeChangeLimit(0),
m_iDefensivePactAttitudeDivisor(0),
m_iDefensivePactAttitudeChangeLimit(0),
m_iShareWarAttitudeChange(0),
m_iShareWarAttitudeDivisor(0),
m_iShareWarAttitudeChangeLimit(0),
m_iFavoriteCivicAttitudeChange(0),
m_iFavoriteCivicAttitudeDivisor(0),
m_iFavoriteCivicAttitudeChangeLimit(0),
m_iDemandTributeAttitudeThreshold(NO_ATTITUDE),
m_iNoGiveHelpAttitudeThreshold(NO_ATTITUDE),
m_iTechRefuseAttitudeThreshold(NO_ATTITUDE),
m_iStrategicBonusRefuseAttitudeThreshold(NO_ATTITUDE),
m_iHappinessBonusRefuseAttitudeThreshold(NO_ATTITUDE),
m_iHealthBonusRefuseAttitudeThreshold(NO_ATTITUDE),
m_iMapRefuseAttitudeThreshold(NO_ATTITUDE),
m_iDeclareWarRefuseAttitudeThreshold(NO_ATTITUDE),
m_iDeclareWarThemRefuseAttitudeThreshold(NO_ATTITUDE),
m_iStopTradingRefuseAttitudeThreshold(NO_ATTITUDE),
m_iStopTradingThemRefuseAttitudeThreshold(NO_ATTITUDE),
m_iAdoptCivicRefuseAttitudeThreshold(NO_ATTITUDE),
m_iConvertReligionRefuseAttitudeThreshold(NO_ATTITUDE),
m_iOpenBordersRefuseAttitudeThreshold(NO_ATTITUDE),
m_iDefensivePactRefuseAttitudeThreshold(NO_ATTITUDE),
m_iPermanentAllianceRefuseAttitudeThreshold(NO_ATTITUDE),
m_iVassalRefuseAttitudeThreshold(NO_ATTITUDE),
m_iVassalPowerModifier(0),
m_iFavoriteCivic(NO_CIVIC),
m_pbTraits(NULL),
m_piFlavorValue(NULL),
m_piContactRand(NULL),
m_piContactDelay(NULL),
m_piMemoryDecayRand(NULL),
m_piMemoryAttitudePercent(NULL),
m_piNoWarAttitudeProb(NULL),
m_piUnitAIWeightModifier(NULL),
m_piImprovementWeightModifier(NULL),
m_piDiploPeaceIntroMusicScriptIds(NULL),
m_piDiploPeaceMusicScriptIds(NULL),
m_piDiploWarIntroMusicScriptIds(NULL),
m_piDiploWarMusicScriptIds(NULL)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvLeaderHeadInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvLeaderHeadInfo::~CvLeaderHeadInfo()
{
	SAFE_DELETE_ARRAY(m_pbTraits);
	SAFE_DELETE_ARRAY(m_piFlavorValue);
	SAFE_DELETE_ARRAY(m_piContactRand);
	SAFE_DELETE_ARRAY(m_piContactDelay);
	SAFE_DELETE_ARRAY(m_piMemoryDecayRand);
	SAFE_DELETE_ARRAY(m_piMemoryAttitudePercent);
	SAFE_DELETE_ARRAY(m_piNoWarAttitudeProb);
	SAFE_DELETE_ARRAY(m_piUnitAIWeightModifier);
	SAFE_DELETE_ARRAY(m_piImprovementWeightModifier);
	SAFE_DELETE_ARRAY(m_piDiploPeaceIntroMusicScriptIds);
	SAFE_DELETE_ARRAY(m_piDiploPeaceMusicScriptIds);
	SAFE_DELETE_ARRAY(m_piDiploWarIntroMusicScriptIds);
	SAFE_DELETE_ARRAY(m_piDiploWarMusicScriptIds);
}

const TCHAR* CvLeaderHeadInfo::getButton() const
{
	const CvArtInfoLeaderhead * pLeaderheadArtInfo;
	pLeaderheadArtInfo = getArtInfo();
	if (pLeaderheadArtInfo != NULL)
	{
		return pLeaderheadArtInfo->getButton();
	}
	else
	{
		return NULL;
	}
}

int CvLeaderHeadInfo::getWonderConstructRand() const
{
	return m_iWonderConstructRand; 
}

int CvLeaderHeadInfo::getBaseAttitude() const
{
	return m_iBaseAttitude; 
}

int CvLeaderHeadInfo::getBasePeaceWeight() const
{
	return m_iBasePeaceWeight; 
}

int CvLeaderHeadInfo::getPeaceWeightRand() const
{
	return m_iPeaceWeightRand;
}

int CvLeaderHeadInfo::getWarmongerRespect() const
{
	return m_iWarmongerRespect; 
}

int CvLeaderHeadInfo::getRefuseToTalkWarThreshold() const
{
	return m_iRefuseToTalkWarThreshold; 
}

int CvLeaderHeadInfo::getNoTechTradeThreshold() const
{
	return m_iNoTechTradeThreshold; 
}

int CvLeaderHeadInfo::getTechTradeKnownPercent() const
{
	return m_iTechTradeKnownPercent; 
}

int CvLeaderHeadInfo::getMaxGoldTradePercent() const
{
	return m_iMaxGoldTradePercent; 
}

int CvLeaderHeadInfo::getMaxGoldPerTurnTradePercent() const
{
	return m_iMaxGoldPerTurnTradePercent; 
}

int CvLeaderHeadInfo::getMaxWarRand() const
{
	return m_iMaxWarRand; 
}

int CvLeaderHeadInfo::getMaxWarNearbyPowerRatio() const
{
	return m_iMaxWarNearbyPowerRatio; 
}

int CvLeaderHeadInfo::getMaxWarDistantPowerRatio() const
{
	return m_iMaxWarDistantPowerRatio; 
}

int CvLeaderHeadInfo::getMaxWarMinAdjacentLandPercent() const
{
	return m_iMaxWarMinAdjacentLandPercent; 
}

int CvLeaderHeadInfo::getLimitedWarRand() const
{
	return m_iLimitedWarRand; 
}

int CvLeaderHeadInfo::getLimitedWarPowerRatio() const
{
	return m_iLimitedWarPowerRatio; 
}

int CvLeaderHeadInfo::getDogpileWarRand() const
{
	return m_iDogpileWarRand; 
}

int CvLeaderHeadInfo::getMakePeaceRand() const
{
	return m_iMakePeaceRand; 
}

int CvLeaderHeadInfo::getDeclareWarTradeRand() const
{
	return m_iDeclareWarTradeRand; 
}

int CvLeaderHeadInfo::getDemandRebukedSneakProb() const
{
	return m_iDemandRebukedSneakProb; 
}

int CvLeaderHeadInfo::getDemandRebukedWarProb() const
{
	return m_iDemandRebukedWarProb; 
}

int CvLeaderHeadInfo::getRazeCityProb() const
{
	return m_iRazeCityProb; 
}

int CvLeaderHeadInfo::getBuildUnitProb() const
{
	return m_iBuildUnitProb; 
}

int CvLeaderHeadInfo::getBaseAttackOddsChange() const
{
	return m_iBaseAttackOddsChange; 
}

int CvLeaderHeadInfo::getAttackOddsChangeRand() const
{
	return m_iAttackOddsChangeRand; 
}

int CvLeaderHeadInfo::getWorseRankDifferenceAttitudeChange() const
{
	return m_iWorseRankDifferenceAttitudeChange; 
}

int CvLeaderHeadInfo::getBetterRankDifferenceAttitudeChange() const
{
	return m_iBetterRankDifferenceAttitudeChange; 
}

int CvLeaderHeadInfo::getCloseBordersAttitudeChange() const
{
	return m_iCloseBordersAttitudeChange; 
}

int CvLeaderHeadInfo::getLostWarAttitudeChange() const
{
	return m_iLostWarAttitudeChange; 
}

int CvLeaderHeadInfo::getAtWarAttitudeDivisor() const
{
	return m_iAtWarAttitudeDivisor; 
}

int CvLeaderHeadInfo::getAtWarAttitudeChangeLimit() const
{
	return m_iAtWarAttitudeChangeLimit; 
}

int CvLeaderHeadInfo::getAtPeaceAttitudeDivisor() const
{
	return m_iAtPeaceAttitudeDivisor; 
}

int CvLeaderHeadInfo::getAtPeaceAttitudeChangeLimit() const
{
	return m_iAtPeaceAttitudeChangeLimit; 
}

int CvLeaderHeadInfo::getSameReligionAttitudeChange() const
{
	return m_iSameReligionAttitudeChange; 
}

int CvLeaderHeadInfo::getSameReligionAttitudeDivisor() const
{
	return m_iSameReligionAttitudeDivisor; 
}

int CvLeaderHeadInfo::getSameReligionAttitudeChangeLimit() const
{
	return m_iSameReligionAttitudeChangeLimit; 
}

int CvLeaderHeadInfo::getDifferentReligionAttitudeChange() const
{
	return m_iDifferentReligionAttitudeChange; 
}

int CvLeaderHeadInfo::getDifferentReligionAttitudeDivisor() const
{
	return m_iDifferentReligionAttitudeDivisor; 
}

int CvLeaderHeadInfo::getDifferentReligionAttitudeChangeLimit() const
{
	return m_iDifferentReligionAttitudeChangeLimit; 
}

int CvLeaderHeadInfo::getBonusTradeAttitudeDivisor() const
{
	return m_iBonusTradeAttitudeDivisor; 
}

int CvLeaderHeadInfo::getBonusTradeAttitudeChangeLimit() const
{
	return m_iBonusTradeAttitudeChangeLimit; 
}

int CvLeaderHeadInfo::getOpenBordersAttitudeDivisor() const
{
	return m_iOpenBordersAttitudeDivisor; 
}

int CvLeaderHeadInfo::getOpenBordersAttitudeChangeLimit() const
{
	return m_iOpenBordersAttitudeChangeLimit; 
}

int CvLeaderHeadInfo::getDefensivePactAttitudeDivisor() const
{
	return m_iDefensivePactAttitudeDivisor; 
}

int CvLeaderHeadInfo::getDefensivePactAttitudeChangeLimit() const
{
	return m_iDefensivePactAttitudeChangeLimit; 
}

int CvLeaderHeadInfo::getShareWarAttitudeChange() const
{
	return m_iShareWarAttitudeChange; 
}

int CvLeaderHeadInfo::getShareWarAttitudeDivisor() const
{
	return m_iShareWarAttitudeDivisor; 
}

int CvLeaderHeadInfo::getShareWarAttitudeChangeLimit() const
{
	return m_iShareWarAttitudeChangeLimit; 
}

int CvLeaderHeadInfo::getFavoriteCivicAttitudeChange() const
{
	return m_iFavoriteCivicAttitudeChange; 
}

int CvLeaderHeadInfo::getFavoriteCivicAttitudeDivisor() const
{
	return m_iFavoriteCivicAttitudeDivisor; 
}

int CvLeaderHeadInfo::getFavoriteCivicAttitudeChangeLimit() const
{
	return m_iFavoriteCivicAttitudeChangeLimit; 
}

int CvLeaderHeadInfo::getDemandTributeAttitudeThreshold() const
{
	return m_iDemandTributeAttitudeThreshold; 
}

int CvLeaderHeadInfo::getNoGiveHelpAttitudeThreshold() const
{
	return m_iNoGiveHelpAttitudeThreshold; 
}

int CvLeaderHeadInfo::getTechRefuseAttitudeThreshold() const
{
	return m_iTechRefuseAttitudeThreshold; 
}

int CvLeaderHeadInfo::getStrategicBonusRefuseAttitudeThreshold() const
{
	return m_iStrategicBonusRefuseAttitudeThreshold; 
}

int CvLeaderHeadInfo::getHappinessBonusRefuseAttitudeThreshold() const
{
	return m_iHappinessBonusRefuseAttitudeThreshold; 
}

int CvLeaderHeadInfo::getHealthBonusRefuseAttitudeThreshold() const
{
	return m_iHealthBonusRefuseAttitudeThreshold; 
}

int CvLeaderHeadInfo::getMapRefuseAttitudeThreshold() const
{
	return m_iMapRefuseAttitudeThreshold; 
}

int CvLeaderHeadInfo::getDeclareWarRefuseAttitudeThreshold() const
{
	return m_iDeclareWarRefuseAttitudeThreshold; 
}

int CvLeaderHeadInfo::getDeclareWarThemRefuseAttitudeThreshold() const
{
	return m_iDeclareWarThemRefuseAttitudeThreshold; 
}

int CvLeaderHeadInfo::getStopTradingRefuseAttitudeThreshold() const
{
	return m_iStopTradingRefuseAttitudeThreshold; 
}

int CvLeaderHeadInfo::getStopTradingThemRefuseAttitudeThreshold() const
{
	return m_iStopTradingThemRefuseAttitudeThreshold; 
}

int CvLeaderHeadInfo::getAdoptCivicRefuseAttitudeThreshold() const
{
	return m_iAdoptCivicRefuseAttitudeThreshold; 
}

int CvLeaderHeadInfo::getConvertReligionRefuseAttitudeThreshold() const
{
	return m_iConvertReligionRefuseAttitudeThreshold; 
}

int CvLeaderHeadInfo::getOpenBordersRefuseAttitudeThreshold() const
{
	return m_iOpenBordersRefuseAttitudeThreshold; 
}

int CvLeaderHeadInfo::getDefensivePactRefuseAttitudeThreshold() const
{
	return m_iDefensivePactRefuseAttitudeThreshold; 
}

int CvLeaderHeadInfo::getPermanentAllianceRefuseAttitudeThreshold() const
{
	return m_iPermanentAllianceRefuseAttitudeThreshold; 
}

int CvLeaderHeadInfo::getVassalRefuseAttitudeThreshold() const
{
	return m_iVassalRefuseAttitudeThreshold; 
}

int CvLeaderHeadInfo::getVassalPowerModifier() const
{
	return m_iVassalPowerModifier; 
}

int CvLeaderHeadInfo::getFavoriteCivic() const
{
	return m_iFavoriteCivic; 
}

const TCHAR* CvLeaderHeadInfo::getArtDefineTag() const
{
	return m_szArtDefineTag; 
}

void CvLeaderHeadInfo::setArtDefineTag(const TCHAR* szVal)
{
	m_szArtDefineTag = szVal; 
}

// Arrays

bool CvLeaderHeadInfo::hasTrait(int i) const
{
	FAssertMsg(i < GC.getNumTraitInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbTraits ? m_pbTraits[i] : false; 
}

int CvLeaderHeadInfo::getFlavorValue(int i) const
{
	FAssertMsg(i < GC.getNumFlavorTypes(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piFlavorValue ? m_piFlavorValue[i] : -1;	
}

int CvLeaderHeadInfo::getContactRand(int i) const
{
	FAssertMsg(i < NUM_CONTACT_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piContactRand ? m_piContactRand[i] : -1;	
}

int CvLeaderHeadInfo::getContactDelay(int i) const
{
	FAssertMsg(i < NUM_CONTACT_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piContactDelay ? m_piContactDelay[i] : -1;	
}

int CvLeaderHeadInfo::getMemoryDecayRand(int i) const
{
	FAssertMsg(i < NUM_MEMORY_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piMemoryDecayRand ? m_piMemoryDecayRand[i] : -1;	
}

int CvLeaderHeadInfo::getMemoryAttitudePercent(int i) const
{
	FAssertMsg(i < NUM_MEMORY_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piMemoryAttitudePercent ? m_piMemoryAttitudePercent[i] : -1;	
}

int CvLeaderHeadInfo::getNoWarAttitudeProb(int i) const
{
	FAssertMsg(i < NUM_ATTITUDE_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piNoWarAttitudeProb ? m_piNoWarAttitudeProb[i] : -1;	
}

int CvLeaderHeadInfo::getUnitAIWeightModifier(int i) const
{
	FAssertMsg(i < NUM_UNITAI_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piUnitAIWeightModifier ? m_piUnitAIWeightModifier[i] : -1;	
}

int CvLeaderHeadInfo::getImprovementWeightModifier(int i) const
{
	FAssertMsg(i < GC.getNumImprovementInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piImprovementWeightModifier ? m_piImprovementWeightModifier[i] : -1;
}

int CvLeaderHeadInfo::getDiploPeaceIntroMusicScriptIds(int i) const
{
	FAssertMsg(i < GC.getNumEraInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piDiploPeaceIntroMusicScriptIds ? m_piDiploPeaceIntroMusicScriptIds[i] : -1;
}

int CvLeaderHeadInfo::getDiploPeaceMusicScriptIds(int i) const
{
	FAssertMsg(i < GC.getNumEraInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piDiploPeaceMusicScriptIds ? m_piDiploPeaceMusicScriptIds[i] : -1;
}

int CvLeaderHeadInfo::getDiploWarIntroMusicScriptIds(int i) const
{
	FAssertMsg(i < GC.getNumEraInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piDiploWarIntroMusicScriptIds ? m_piDiploWarIntroMusicScriptIds[i] : -1;
}

int CvLeaderHeadInfo::getDiploWarMusicScriptIds(int i) const
{
	FAssertMsg(i < GC.getNumEraInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piDiploWarMusicScriptIds ? m_piDiploWarMusicScriptIds[i] : -1;
}

const TCHAR* CvLeaderHeadInfo::getLeaderHead() const
{
	const CvArtInfoLeaderhead * pLeaderheadArtInfo;
	pLeaderheadArtInfo = getArtInfo();
	if (pLeaderheadArtInfo != NULL)
	{
		return pLeaderheadArtInfo->getNIF();
	}
	else
	{
		return NULL;
	}
}

void CvLeaderHeadInfo::read(FDataStreamBase* stream)
{
	CvInfoBase::read(stream);

	uint uiFlag=0;
	stream->Read(&uiFlag);		// flag for expansion

	stream->Read(&m_iWonderConstructRand);
	stream->Read(&m_iBaseAttitude);
	stream->Read(&m_iBasePeaceWeight);
	stream->Read(&m_iPeaceWeightRand);
	stream->Read(&m_iWarmongerRespect);
	stream->Read(&m_iRefuseToTalkWarThreshold);
	stream->Read(&m_iNoTechTradeThreshold);
	stream->Read(&m_iTechTradeKnownPercent);
	stream->Read(&m_iMaxGoldTradePercent);
	stream->Read(&m_iMaxGoldPerTurnTradePercent);
	stream->Read(&m_iMaxWarRand);
	stream->Read(&m_iMaxWarNearbyPowerRatio);
	stream->Read(&m_iMaxWarDistantPowerRatio);
	stream->Read(&m_iMaxWarMinAdjacentLandPercent);
	stream->Read(&m_iLimitedWarRand);
	stream->Read(&m_iLimitedWarPowerRatio);
	stream->Read(&m_iDogpileWarRand);
	stream->Read(&m_iMakePeaceRand);
	stream->Read(&m_iDeclareWarTradeRand);
	stream->Read(&m_iDemandRebukedSneakProb);
	stream->Read(&m_iDemandRebukedWarProb);
	stream->Read(&m_iRazeCityProb);
	stream->Read(&m_iBuildUnitProb);
	stream->Read(&m_iBaseAttackOddsChange);
	stream->Read(&m_iAttackOddsChangeRand);
	stream->Read(&m_iWorseRankDifferenceAttitudeChange);
	stream->Read(&m_iBetterRankDifferenceAttitudeChange);
	stream->Read(&m_iCloseBordersAttitudeChange);
	stream->Read(&m_iLostWarAttitudeChange);
	stream->Read(&m_iAtWarAttitudeDivisor);
	stream->Read(&m_iAtWarAttitudeChangeLimit);
	stream->Read(&m_iAtPeaceAttitudeDivisor);
	stream->Read(&m_iAtPeaceAttitudeChangeLimit);
	stream->Read(&m_iSameReligionAttitudeChange);
	stream->Read(&m_iSameReligionAttitudeDivisor);
	stream->Read(&m_iSameReligionAttitudeChangeLimit);
	stream->Read(&m_iDifferentReligionAttitudeChange);
	stream->Read(&m_iDifferentReligionAttitudeDivisor);
	stream->Read(&m_iDifferentReligionAttitudeChangeLimit);
	stream->Read(&m_iBonusTradeAttitudeDivisor);
	stream->Read(&m_iBonusTradeAttitudeChangeLimit);
	stream->Read(&m_iOpenBordersAttitudeDivisor);
	stream->Read(&m_iOpenBordersAttitudeChangeLimit);
	stream->Read(&m_iDefensivePactAttitudeDivisor);
	stream->Read(&m_iDefensivePactAttitudeChangeLimit);
	stream->Read(&m_iShareWarAttitudeChange);
	stream->Read(&m_iShareWarAttitudeDivisor);
	stream->Read(&m_iShareWarAttitudeChangeLimit);
	stream->Read(&m_iFavoriteCivicAttitudeChange);
	stream->Read(&m_iFavoriteCivicAttitudeDivisor);
	stream->Read(&m_iFavoriteCivicAttitudeChangeLimit);
	stream->Read(&m_iDemandTributeAttitudeThreshold);
	stream->Read(&m_iNoGiveHelpAttitudeThreshold);
	stream->Read(&m_iTechRefuseAttitudeThreshold);
	stream->Read(&m_iStrategicBonusRefuseAttitudeThreshold);
	stream->Read(&m_iHappinessBonusRefuseAttitudeThreshold);
	stream->Read(&m_iHealthBonusRefuseAttitudeThreshold);
	stream->Read(&m_iMapRefuseAttitudeThreshold);
	stream->Read(&m_iDeclareWarRefuseAttitudeThreshold);
	stream->Read(&m_iDeclareWarThemRefuseAttitudeThreshold);
	stream->Read(&m_iStopTradingRefuseAttitudeThreshold);
	stream->Read(&m_iStopTradingThemRefuseAttitudeThreshold);
	stream->Read(&m_iAdoptCivicRefuseAttitudeThreshold);
	stream->Read(&m_iConvertReligionRefuseAttitudeThreshold);
	stream->Read(&m_iOpenBordersRefuseAttitudeThreshold);
	stream->Read(&m_iDefensivePactRefuseAttitudeThreshold);
	stream->Read(&m_iPermanentAllianceRefuseAttitudeThreshold);
	stream->Read(&m_iVassalRefuseAttitudeThreshold);
	stream->Read(&m_iVassalPowerModifier);
	stream->Read(&m_iFavoriteCivic);

	stream->ReadString(m_szArtDefineTag);

	// Arrays

	SAFE_DELETE_ARRAY(m_pbTraits);
	m_pbTraits = new bool[GC.getNumTraitInfos()];
	stream->Read(GC.getNumTraitInfos(), m_pbTraits);

	SAFE_DELETE_ARRAY(m_piFlavorValue);
	m_piFlavorValue = new int[GC.getNumFlavorTypes()];
	stream->Read(GC.getNumFlavorTypes(), m_piFlavorValue);

	SAFE_DELETE_ARRAY(m_piContactRand);
	m_piContactRand = new int[NUM_CONTACT_TYPES];
	stream->Read(NUM_CONTACT_TYPES, m_piContactRand);

	SAFE_DELETE_ARRAY(m_piContactDelay);
	m_piContactDelay = new int[NUM_CONTACT_TYPES];
	stream->Read(NUM_CONTACT_TYPES, m_piContactDelay);

	SAFE_DELETE_ARRAY(m_piMemoryDecayRand);
	m_piMemoryDecayRand = new int[NUM_MEMORY_TYPES];
	stream->Read(NUM_MEMORY_TYPES, m_piMemoryDecayRand);

	SAFE_DELETE_ARRAY(m_piMemoryAttitudePercent);
	m_piMemoryAttitudePercent = new int[NUM_MEMORY_TYPES];
	stream->Read(NUM_MEMORY_TYPES, m_piMemoryAttitudePercent);

	SAFE_DELETE_ARRAY(m_piNoWarAttitudeProb);
	m_piNoWarAttitudeProb = new int[NUM_ATTITUDE_TYPES];
	stream->Read(NUM_ATTITUDE_TYPES, m_piNoWarAttitudeProb);

	SAFE_DELETE_ARRAY(m_piUnitAIWeightModifier);
	m_piUnitAIWeightModifier = new int[NUM_UNITAI_TYPES];
	stream->Read(NUM_UNITAI_TYPES, m_piUnitAIWeightModifier);

	SAFE_DELETE_ARRAY(m_piImprovementWeightModifier);
	m_piImprovementWeightModifier = new int[GC.getNumImprovementInfos()];
	stream->Read(GC.getNumImprovementInfos(), m_piImprovementWeightModifier);

	SAFE_DELETE_ARRAY(m_piDiploPeaceIntroMusicScriptIds);
	m_piDiploPeaceIntroMusicScriptIds = new int[GC.getNumEraInfos()];
	stream->Read(GC.getNumEraInfos(), m_piDiploPeaceIntroMusicScriptIds);

	SAFE_DELETE_ARRAY(m_piDiploPeaceMusicScriptIds);
	m_piDiploPeaceMusicScriptIds = new int[GC.getNumEraInfos()];
	stream->Read(GC.getNumEraInfos(), m_piDiploPeaceMusicScriptIds);

	SAFE_DELETE_ARRAY(m_piDiploWarIntroMusicScriptIds);
	m_piDiploWarIntroMusicScriptIds = new int[GC.getNumEraInfos()];
	stream->Read(GC.getNumEraInfos(), m_piDiploWarIntroMusicScriptIds);

	SAFE_DELETE_ARRAY(m_piDiploWarMusicScriptIds);
	m_piDiploWarMusicScriptIds = new int[GC.getNumEraInfos()];
	stream->Read(GC.getNumEraInfos(), m_piDiploWarMusicScriptIds);
}

void CvLeaderHeadInfo::write(FDataStreamBase* stream)
{
	CvInfoBase::write(stream);

	uint uiFlag=0;
	stream->Write(uiFlag);		// flag for expansion

	stream->Write(m_iWonderConstructRand);
	stream->Write(m_iBaseAttitude);
	stream->Write(m_iBasePeaceWeight);
	stream->Write(m_iPeaceWeightRand);
	stream->Write(m_iWarmongerRespect);
	stream->Write(m_iRefuseToTalkWarThreshold);
	stream->Write(m_iNoTechTradeThreshold);
	stream->Write(m_iTechTradeKnownPercent);
	stream->Write(m_iMaxGoldTradePercent);
	stream->Write(m_iMaxGoldPerTurnTradePercent);
	stream->Write(m_iMaxWarRand);
	stream->Write(m_iMaxWarNearbyPowerRatio);
	stream->Write(m_iMaxWarDistantPowerRatio);
	stream->Write(m_iMaxWarMinAdjacentLandPercent);
	stream->Write(m_iLimitedWarRand);
	stream->Write(m_iLimitedWarPowerRatio);
	stream->Write(m_iDogpileWarRand);
	stream->Write(m_iMakePeaceRand);
	stream->Write(m_iDeclareWarTradeRand);
	stream->Write(m_iDemandRebukedSneakProb);
	stream->Write(m_iDemandRebukedWarProb);
	stream->Write(m_iRazeCityProb);
	stream->Write(m_iBuildUnitProb);
	stream->Write(m_iBaseAttackOddsChange);
	stream->Write(m_iAttackOddsChangeRand);
	stream->Write(m_iWorseRankDifferenceAttitudeChange);
	stream->Write(m_iBetterRankDifferenceAttitudeChange);
	stream->Write(m_iCloseBordersAttitudeChange);
	stream->Write(m_iLostWarAttitudeChange);
	stream->Write(m_iAtWarAttitudeDivisor);
	stream->Write(m_iAtWarAttitudeChangeLimit);
	stream->Write(m_iAtPeaceAttitudeDivisor);
	stream->Write(m_iAtPeaceAttitudeChangeLimit);
	stream->Write(m_iSameReligionAttitudeChange);
	stream->Write(m_iSameReligionAttitudeDivisor);
	stream->Write(m_iSameReligionAttitudeChangeLimit);
	stream->Write(m_iDifferentReligionAttitudeChange);
	stream->Write(m_iDifferentReligionAttitudeDivisor);
	stream->Write(m_iDifferentReligionAttitudeChangeLimit);
	stream->Write(m_iBonusTradeAttitudeDivisor);
	stream->Write(m_iBonusTradeAttitudeChangeLimit);
	stream->Write(m_iOpenBordersAttitudeDivisor);
	stream->Write(m_iOpenBordersAttitudeChangeLimit);
	stream->Write(m_iDefensivePactAttitudeDivisor);
	stream->Write(m_iDefensivePactAttitudeChangeLimit);
	stream->Write(m_iShareWarAttitudeChange);
	stream->Write(m_iShareWarAttitudeDivisor);
	stream->Write(m_iShareWarAttitudeChangeLimit);
	stream->Write(m_iFavoriteCivicAttitudeChange);
	stream->Write(m_iFavoriteCivicAttitudeDivisor);
	stream->Write(m_iFavoriteCivicAttitudeChangeLimit);
	stream->Write(m_iDemandTributeAttitudeThreshold);
	stream->Write(m_iNoGiveHelpAttitudeThreshold);
	stream->Write(m_iTechRefuseAttitudeThreshold);
	stream->Write(m_iStrategicBonusRefuseAttitudeThreshold);
	stream->Write(m_iHappinessBonusRefuseAttitudeThreshold);
	stream->Write(m_iHealthBonusRefuseAttitudeThreshold);
	stream->Write(m_iMapRefuseAttitudeThreshold);
	stream->Write(m_iDeclareWarRefuseAttitudeThreshold);
	stream->Write(m_iDeclareWarThemRefuseAttitudeThreshold);
	stream->Write(m_iStopTradingRefuseAttitudeThreshold);
	stream->Write(m_iStopTradingThemRefuseAttitudeThreshold);
	stream->Write(m_iAdoptCivicRefuseAttitudeThreshold);
	stream->Write(m_iConvertReligionRefuseAttitudeThreshold);
	stream->Write(m_iOpenBordersRefuseAttitudeThreshold);
	stream->Write(m_iDefensivePactRefuseAttitudeThreshold);
	stream->Write(m_iPermanentAllianceRefuseAttitudeThreshold);
	stream->Write(m_iVassalRefuseAttitudeThreshold);
	stream->Write(m_iVassalPowerModifier);
	stream->Write(m_iFavoriteCivic);

	stream->WriteString(m_szArtDefineTag);

	// Arrays

	stream->Write(GC.getNumTraitInfos(), m_pbTraits);

	stream->Write(GC.getNumFlavorTypes(), m_piFlavorValue);
	stream->Write(NUM_CONTACT_TYPES, m_piContactRand);
	stream->Write(NUM_CONTACT_TYPES, m_piContactDelay);
	stream->Write(NUM_MEMORY_TYPES, m_piMemoryDecayRand);
	stream->Write(NUM_MEMORY_TYPES, m_piMemoryAttitudePercent);
	stream->Write(NUM_ATTITUDE_TYPES, m_piNoWarAttitudeProb);
	stream->Write(NUM_UNITAI_TYPES, m_piUnitAIWeightModifier);
	stream->Write(GC.getNumImprovementInfos(), m_piImprovementWeightModifier);
	stream->Write(GC.getNumEraInfos(), m_piDiploPeaceIntroMusicScriptIds);
	stream->Write(GC.getNumEraInfos(), m_piDiploPeaceMusicScriptIds);
	stream->Write(GC.getNumEraInfos(), m_piDiploWarIntroMusicScriptIds);
	stream->Write(GC.getNumEraInfos(), m_piDiploWarMusicScriptIds);

}

const CvArtInfoLeaderhead* CvLeaderHeadInfo::getArtInfo() const
{
	return ARTFILEMGR.getLeaderheadArtInfo( getArtDefineTag());
}

bool CvLeaderHeadInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(szTextVal, "ArtDefineTag");
	setArtDefineTag(szTextVal);

	pXML->GetChildXmlValByName(&m_iWonderConstructRand, "iWonderConstructRand");
	pXML->GetChildXmlValByName(&m_iBaseAttitude, "iBaseAttitude");
	pXML->GetChildXmlValByName(&m_iBasePeaceWeight, "iBasePeaceWeight");
	pXML->GetChildXmlValByName(&m_iPeaceWeightRand, "iPeaceWeightRand");
	pXML->GetChildXmlValByName(&m_iWarmongerRespect, "iWarmongerRespect");
	pXML->GetChildXmlValByName(&m_iRefuseToTalkWarThreshold, "iRefuseToTalkWarThreshold");
	pXML->GetChildXmlValByName(&m_iNoTechTradeThreshold, "iNoTechTradeThreshold");
	pXML->GetChildXmlValByName(&m_iTechTradeKnownPercent, "iTechTradeKnownPercent");
	pXML->GetChildXmlValByName(&m_iMaxGoldTradePercent, "iMaxGoldTradePercent");
	pXML->GetChildXmlValByName(&m_iMaxGoldPerTurnTradePercent, "iMaxGoldPerTurnTradePercent");
	pXML->GetChildXmlValByName(&m_iMaxWarRand, "iMaxWarRand");
	pXML->GetChildXmlValByName(&m_iMaxWarNearbyPowerRatio, "iMaxWarNearbyPowerRatio");
	pXML->GetChildXmlValByName(&m_iMaxWarDistantPowerRatio, "iMaxWarDistantPowerRatio");
	pXML->GetChildXmlValByName(&m_iMaxWarMinAdjacentLandPercent, "iMaxWarMinAdjacentLandPercent");
	pXML->GetChildXmlValByName(&m_iLimitedWarRand, "iLimitedWarRand");
	pXML->GetChildXmlValByName(&m_iLimitedWarPowerRatio, "iLimitedWarPowerRatio");
	pXML->GetChildXmlValByName(&m_iDogpileWarRand, "iDogpileWarRand");
	pXML->GetChildXmlValByName(&m_iMakePeaceRand, "iMakePeaceRand");
	pXML->GetChildXmlValByName(&m_iDeclareWarTradeRand, "iDeclareWarTradeRand");
	pXML->GetChildXmlValByName(&m_iDemandRebukedSneakProb, "iDemandRebukedSneakProb");
	pXML->GetChildXmlValByName(&m_iDemandRebukedWarProb, "iDemandRebukedWarProb");
	pXML->GetChildXmlValByName(&m_iRazeCityProb, "iRazeCityProb");
	pXML->GetChildXmlValByName(&m_iBuildUnitProb, "iBuildUnitProb");
	pXML->GetChildXmlValByName(&m_iBaseAttackOddsChange, "iBaseAttackOddsChange");
	pXML->GetChildXmlValByName(&m_iAttackOddsChangeRand, "iAttackOddsChangeRand");
	pXML->GetChildXmlValByName(&m_iWorseRankDifferenceAttitudeChange, "iWorseRankDifferenceAttitudeChange");
	pXML->GetChildXmlValByName(&m_iBetterRankDifferenceAttitudeChange, "iBetterRankDifferenceAttitudeChange");
	pXML->GetChildXmlValByName(&m_iCloseBordersAttitudeChange, "iCloseBordersAttitudeChange");
	pXML->GetChildXmlValByName(&m_iLostWarAttitudeChange, "iLostWarAttitudeChange");
	pXML->GetChildXmlValByName(&m_iAtWarAttitudeDivisor, "iAtWarAttitudeDivisor");
	pXML->GetChildXmlValByName(&m_iAtWarAttitudeChangeLimit, "iAtWarAttitudeChangeLimit");
	pXML->GetChildXmlValByName(&m_iAtPeaceAttitudeDivisor, "iAtPeaceAttitudeDivisor");
	pXML->GetChildXmlValByName(&m_iAtPeaceAttitudeChangeLimit, "iAtPeaceAttitudeChangeLimit");
	pXML->GetChildXmlValByName(&m_iSameReligionAttitudeChange, "iSameReligionAttitudeChange");
	pXML->GetChildXmlValByName(&m_iSameReligionAttitudeDivisor, "iSameReligionAttitudeDivisor");
	pXML->GetChildXmlValByName(&m_iSameReligionAttitudeChangeLimit, "iSameReligionAttitudeChangeLimit");
	pXML->GetChildXmlValByName(&m_iDifferentReligionAttitudeChange, "iDifferentReligionAttitudeChange");
	pXML->GetChildXmlValByName(&m_iDifferentReligionAttitudeDivisor, "iDifferentReligionAttitudeDivisor");
	pXML->GetChildXmlValByName(&m_iDifferentReligionAttitudeChangeLimit, "iDifferentReligionAttitudeChangeLimit");
	pXML->GetChildXmlValByName(&m_iBonusTradeAttitudeDivisor, "iBonusTradeAttitudeDivisor");
	pXML->GetChildXmlValByName(&m_iBonusTradeAttitudeChangeLimit, "iBonusTradeAttitudeChangeLimit");
	pXML->GetChildXmlValByName(&m_iOpenBordersAttitudeDivisor, "iOpenBordersAttitudeDivisor");
	pXML->GetChildXmlValByName(&m_iOpenBordersAttitudeChangeLimit, "iOpenBordersAttitudeChangeLimit");
	pXML->GetChildXmlValByName(&m_iDefensivePactAttitudeDivisor, "iDefensivePactAttitudeDivisor");
	pXML->GetChildXmlValByName(&m_iDefensivePactAttitudeChangeLimit, "iDefensivePactAttitudeChangeLimit");
	pXML->GetChildXmlValByName(&m_iShareWarAttitudeChange, "iShareWarAttitudeChange");
	pXML->GetChildXmlValByName(&m_iShareWarAttitudeDivisor, "iShareWarAttitudeDivisor");
	pXML->GetChildXmlValByName(&m_iShareWarAttitudeChangeLimit, "iShareWarAttitudeChangeLimit");
	pXML->GetChildXmlValByName(&m_iFavoriteCivicAttitudeChange, "iFavoriteCivicAttitudeChange");
	pXML->GetChildXmlValByName(&m_iFavoriteCivicAttitudeDivisor, "iFavoriteCivicAttitudeDivisor");
	pXML->GetChildXmlValByName(&m_iFavoriteCivicAttitudeChangeLimit, "iFavoriteCivicAttitudeChangeLimit");
	pXML->GetChildXmlValByName(&m_iVassalPowerModifier, "iVassalPowerModifier");

	pXML->GetChildXmlValByName(szTextVal, "DemandTributeAttitudeThreshold");
	m_iDemandTributeAttitudeThreshold = pXML->FindInInfoClass( szTextVal, GC.getAttitudeInfo(), sizeof(GC.getAttitudeInfo((AttitudeTypes)0)), NUM_ATTITUDE_TYPES);

	pXML->GetChildXmlValByName(szTextVal, "NoGiveHelpAttitudeThreshold");
	m_iNoGiveHelpAttitudeThreshold = pXML->FindInInfoClass( szTextVal, GC.getAttitudeInfo(), sizeof(GC.getAttitudeInfo((AttitudeTypes)0)), NUM_ATTITUDE_TYPES);

	pXML->GetChildXmlValByName(szTextVal, "TechRefuseAttitudeThreshold");
	m_iTechRefuseAttitudeThreshold = pXML->FindInInfoClass( szTextVal, GC.getAttitudeInfo(), sizeof(GC.getAttitudeInfo((AttitudeTypes)0)), NUM_ATTITUDE_TYPES);

	pXML->GetChildXmlValByName(szTextVal, "StrategicBonusRefuseAttitudeThreshold");
	m_iStrategicBonusRefuseAttitudeThreshold = pXML->FindInInfoClass( szTextVal, GC.getAttitudeInfo(), sizeof(GC.getAttitudeInfo((AttitudeTypes)0)), NUM_ATTITUDE_TYPES);

	pXML->GetChildXmlValByName(szTextVal, "HappinessBonusRefuseAttitudeThreshold");
	m_iHappinessBonusRefuseAttitudeThreshold = pXML->FindInInfoClass( szTextVal, GC.getAttitudeInfo(), sizeof(GC.getAttitudeInfo((AttitudeTypes)0)), NUM_ATTITUDE_TYPES);

	pXML->GetChildXmlValByName(szTextVal, "HealthBonusRefuseAttitudeThreshold");
	m_iHealthBonusRefuseAttitudeThreshold = pXML->FindInInfoClass( szTextVal, GC.getAttitudeInfo(), sizeof(GC.getAttitudeInfo((AttitudeTypes)0)), NUM_ATTITUDE_TYPES);

	pXML->GetChildXmlValByName(szTextVal, "MapRefuseAttitudeThreshold");
	m_iMapRefuseAttitudeThreshold = pXML->FindInInfoClass( szTextVal, GC.getAttitudeInfo(), sizeof(GC.getAttitudeInfo((AttitudeTypes)0)), NUM_ATTITUDE_TYPES);

	pXML->GetChildXmlValByName(szTextVal, "DeclareWarRefuseAttitudeThreshold");
	m_iDeclareWarRefuseAttitudeThreshold = pXML->FindInInfoClass( szTextVal, GC.getAttitudeInfo(), sizeof(GC.getAttitudeInfo((AttitudeTypes)0)), NUM_ATTITUDE_TYPES);

	pXML->GetChildXmlValByName(szTextVal, "DeclareWarThemRefuseAttitudeThreshold");
	m_iDeclareWarThemRefuseAttitudeThreshold = pXML->FindInInfoClass( szTextVal, GC.getAttitudeInfo(), sizeof(GC.getAttitudeInfo((AttitudeTypes)0)), NUM_ATTITUDE_TYPES);

	pXML->GetChildXmlValByName(szTextVal, "StopTradingRefuseAttitudeThreshold");
	m_iStopTradingRefuseAttitudeThreshold = pXML->FindInInfoClass( szTextVal, GC.getAttitudeInfo(), sizeof(GC.getAttitudeInfo((AttitudeTypes)0)), NUM_ATTITUDE_TYPES);

	pXML->GetChildXmlValByName(szTextVal, "StopTradingThemRefuseAttitudeThreshold");
	m_iStopTradingThemRefuseAttitudeThreshold = pXML->FindInInfoClass( szTextVal, GC.getAttitudeInfo(), sizeof(GC.getAttitudeInfo((AttitudeTypes)0)), NUM_ATTITUDE_TYPES);

	pXML->GetChildXmlValByName(szTextVal, "AdoptCivicRefuseAttitudeThreshold");
	m_iAdoptCivicRefuseAttitudeThreshold = pXML->FindInInfoClass( szTextVal, GC.getAttitudeInfo(), sizeof(GC.getAttitudeInfo((AttitudeTypes)0)), NUM_ATTITUDE_TYPES);

	pXML->GetChildXmlValByName(szTextVal, "ConvertReligionRefuseAttitudeThreshold");
	m_iConvertReligionRefuseAttitudeThreshold = pXML->FindInInfoClass( szTextVal, GC.getAttitudeInfo(), sizeof(GC.getAttitudeInfo((AttitudeTypes)0)), NUM_ATTITUDE_TYPES);

	pXML->GetChildXmlValByName(szTextVal, "OpenBordersRefuseAttitudeThreshold");
	m_iOpenBordersRefuseAttitudeThreshold = pXML->FindInInfoClass( szTextVal, GC.getAttitudeInfo(), sizeof(GC.getAttitudeInfo((AttitudeTypes)0)), NUM_ATTITUDE_TYPES);

	pXML->GetChildXmlValByName(szTextVal, "DefensivePactRefuseAttitudeThreshold");
	m_iDefensivePactRefuseAttitudeThreshold = pXML->FindInInfoClass( szTextVal, GC.getAttitudeInfo(), sizeof(GC.getAttitudeInfo((AttitudeTypes)0)), NUM_ATTITUDE_TYPES);

	pXML->GetChildXmlValByName(szTextVal, "PermanentAllianceRefuseAttitudeThreshold");
	m_iPermanentAllianceRefuseAttitudeThreshold = pXML->FindInInfoClass( szTextVal, GC.getAttitudeInfo(), sizeof(GC.getAttitudeInfo((AttitudeTypes)0)), NUM_ATTITUDE_TYPES);

	pXML->GetChildXmlValByName(szTextVal, "VassalRefuseAttitudeThreshold");
	m_iVassalRefuseAttitudeThreshold = pXML->FindInInfoClass( szTextVal, GC.getAttitudeInfo(), sizeof(GC.getAttitudeInfo((AttitudeTypes)0)), NUM_ATTITUDE_TYPES);

	pXML->GetChildXmlValByName(szTextVal, "FavoriteCivic");
	m_iFavoriteCivic = pXML->FindInInfoClass(szTextVal, GC.getCivicInfo(), sizeof(GC.getCivicInfo((CivicTypes)0)), GC.getNumCivicInfos());

	pXML->SetVariableListTagPair(&m_pbTraits, "Traits", GC.getTraitInfo(), sizeof(GC.getTraitInfo((TraitTypes)0)), GC.getNumTraitInfos());

	pXML->SetVariableListTagPair(&m_piFlavorValue, "Flavors", GC.getFlavorTypes(), GC.getNumFlavorTypes());
	pXML->SetVariableListTagPair(&m_piContactRand, "ContactRands", GC.getContactTypes(), NUM_CONTACT_TYPES);
	pXML->SetVariableListTagPair(&m_piContactDelay, "ContactDelays", GC.getContactTypes(), NUM_CONTACT_TYPES);
	pXML->SetVariableListTagPair(&m_piMemoryDecayRand, "MemoryDecays", GC.getMemoryInfo(), sizeof(GC.getMemoryInfo((MemoryTypes)0)), NUM_MEMORY_TYPES);
	pXML->SetVariableListTagPair(&m_piMemoryAttitudePercent, "MemoryAttitudePercents", GC.getMemoryInfo(), sizeof(GC.getMemoryInfo((MemoryTypes)0)), NUM_MEMORY_TYPES);
	pXML->SetVariableListTagPair(&m_piNoWarAttitudeProb, "NoWarAttitudeProbs", GC.getAttitudeInfo(), sizeof(GC.getAttitudeInfo((AttitudeTypes)0)), NUM_ATTITUDE_TYPES);
	pXML->SetVariableListTagPair(&m_piUnitAIWeightModifier, "UnitAIWeightModifiers", GC.getUnitAIInfo(), sizeof(GC.getUnitAIInfo((UnitAITypes)0)), NUM_UNITAI_TYPES);
	pXML->SetVariableListTagPair(&m_piImprovementWeightModifier, "ImprovementWeightModifiers", GC.getImprovementInfo(), sizeof(GC.getImprovementInfo((ImprovementTypes)0)), GC.getNumImprovementInfos());
	pXML->SetVariableListTagPairForAudioScripts(&m_piDiploPeaceIntroMusicScriptIds, "DiplomacyIntroMusicPeace", GC.getEraInfo(), sizeof(GC.getEraInfo((EraTypes)0)), GC.getNumEraInfos());
	pXML->SetVariableListTagPairForAudioScripts(&m_piDiploPeaceMusicScriptIds, "DiplomacyMusicPeace", GC.getEraInfo(), sizeof(GC.getEraInfo((EraTypes)0)), GC.getNumEraInfos());
	pXML->SetVariableListTagPairForAudioScripts(&m_piDiploWarIntroMusicScriptIds, "DiplomacyIntroMusicWar", GC.getEraInfo(), sizeof(GC.getEraInfo((EraTypes)0)), GC.getNumEraInfos());
	pXML->SetVariableListTagPairForAudioScripts(&m_piDiploWarMusicScriptIds, "DiplomacyMusicWar", GC.getEraInfo(), sizeof(GC.getEraInfo((EraTypes)0)), GC.getNumEraInfos());

	return true;
}

//======================================================================================================
//					CvWorldInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvWorldInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvWorldInfo::CvWorldInfo() :
m_iDefaultPlayers(0),
m_iUnitNameModifier(0),
m_iTargetNumCities(0),
m_iNumFreeBuildingBonuses(0),
m_iBuildingClassPrereqModifier(0),
m_iMaxConscriptModifier(0),
m_iWarWearinessModifier(0),
m_iGridWidth(0),
m_iGridHeight(0),
m_iTerrainGrainChange(0),
m_iFeatureGrainChange(0),
m_iResearchPercent(0),
m_iTradeProfitPercent(0),
m_iDistanceMaintenancePercent(0),
m_iNumCitiesMaintenancePercent(0),
m_iNumCitiesAnarchyPercent(0)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvWorldInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvWorldInfo::~CvWorldInfo()
{
}

int CvWorldInfo::getDefaultPlayers() const
{
	return m_iDefaultPlayers; 
}

int CvWorldInfo::getUnitNameModifier() const
{
	return m_iUnitNameModifier; 
}

int CvWorldInfo::getTargetNumCities() const
{
	return m_iTargetNumCities; 
}

int CvWorldInfo::getNumFreeBuildingBonuses() const
{
	return m_iNumFreeBuildingBonuses; 
}

int CvWorldInfo::getBuildingClassPrereqModifier() const
{
	return m_iBuildingClassPrereqModifier; 
}

int CvWorldInfo::getMaxConscriptModifier() const
{
	return m_iMaxConscriptModifier; 
}

int CvWorldInfo::getWarWearinessModifier() const
{
	return m_iWarWearinessModifier; 
}

int CvWorldInfo::getGridWidth() const
{
	return m_iGridWidth; 
}

int CvWorldInfo::getGridHeight() const
{
	return m_iGridHeight; 
}

int CvWorldInfo::getTerrainGrainChange() const
{
	return m_iTerrainGrainChange; 
}

int CvWorldInfo::getFeatureGrainChange() const
{
	return m_iFeatureGrainChange; 
}

int CvWorldInfo::getResearchPercent() const
{
	return m_iResearchPercent; 
}

int CvWorldInfo::getTradeProfitPercent() const
{
	return m_iTradeProfitPercent; 
}

int CvWorldInfo::getDistanceMaintenancePercent() const
{
	return m_iDistanceMaintenancePercent; 
}

int CvWorldInfo::getNumCitiesMaintenancePercent() const
{
	return m_iNumCitiesMaintenancePercent; 
}

int CvWorldInfo::getNumCitiesAnarchyPercent() const
{
	return m_iNumCitiesAnarchyPercent; 
}

bool CvWorldInfo::read(CvXMLLoadUtility* pXML)
{
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(&m_iDefaultPlayers, "iDefaultPlayers");
	pXML->GetChildXmlValByName(&m_iUnitNameModifier, "iUnitNameModifier");
	pXML->GetChildXmlValByName(&m_iTargetNumCities, "iTargetNumCities");
	pXML->GetChildXmlValByName(&m_iNumFreeBuildingBonuses, "iNumFreeBuildingBonuses");
	pXML->GetChildXmlValByName(&m_iBuildingClassPrereqModifier, "iBuildingClassPrereqModifier");
	pXML->GetChildXmlValByName(&m_iMaxConscriptModifier, "iMaxConscriptModifier");
	pXML->GetChildXmlValByName(&m_iWarWearinessModifier, "iWarWearinessModifier");
	pXML->GetChildXmlValByName(&m_iGridWidth, "iGridWidth");
	pXML->GetChildXmlValByName(&m_iGridHeight, "iGridHeight");
	pXML->GetChildXmlValByName(&m_iTerrainGrainChange, "iTerrainGrainChange");
	pXML->GetChildXmlValByName(&m_iFeatureGrainChange, "iFeatureGrainChange");
	pXML->GetChildXmlValByName(&m_iResearchPercent, "iResearchPercent");
	pXML->GetChildXmlValByName(&m_iTradeProfitPercent, "iTradeProfitPercent");
	pXML->GetChildXmlValByName(&m_iDistanceMaintenancePercent, "iDistanceMaintenancePercent");
	pXML->GetChildXmlValByName(&m_iNumCitiesMaintenancePercent, "iNumCitiesMaintenancePercent");
	pXML->GetChildXmlValByName(&m_iNumCitiesAnarchyPercent, "iNumCitiesAnarchyPercent");

	return true;
}

//======================================================================================================
//					CvClimateInfo
//======================================================================================================
CvClimateInfo::CvClimateInfo() :
m_iDesertPercentChange(0),
m_iJungleLatitude(0),
m_iHillRange(0),
m_iPeakPercent(0),
m_fSnowLatitudeChange(0.0f),
m_fTundraLatitudeChange(0.0f),
m_fGrassLatitudeChange(0.0f),
m_fDesertBottomLatitudeChange(0.0f),
m_fDesertTopLatitudeChange(0.0f),
m_fIceLatitude(0.0f),
m_fRandIceLatitude(0.0f)
{
}

CvClimateInfo::~CvClimateInfo()
{
}

int CvClimateInfo::getDesertPercentChange() const
{
	return m_iDesertPercentChange; 
}

int CvClimateInfo::getJungleLatitude() const
{
	return m_iJungleLatitude; 
}

int CvClimateInfo::getHillRange() const
{
	return m_iHillRange; 
}

int CvClimateInfo::getPeakPercent() const
{
	return m_iPeakPercent; 
}

float CvClimateInfo::getSnowLatitudeChange() const
{
	return m_fSnowLatitudeChange; 
}

float CvClimateInfo::getTundraLatitudeChange() const
{
	return m_fTundraLatitudeChange; 
}

float CvClimateInfo::getGrassLatitudeChange() const
{
	return m_fGrassLatitudeChange; 
}

float CvClimateInfo::getDesertBottomLatitudeChange() const
{
	return m_fDesertBottomLatitudeChange; 
}

float CvClimateInfo::getDesertTopLatitudeChange() const
{
	return m_fDesertTopLatitudeChange; 
}

float CvClimateInfo::getIceLatitude() const
{
	return m_fIceLatitude; 
}

float CvClimateInfo::getRandIceLatitude() const
{
	return m_fRandIceLatitude; 
}

bool CvClimateInfo::read(CvXMLLoadUtility* pXML)
{
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(&m_iDesertPercentChange, "iDesertPercentChange");
	pXML->GetChildXmlValByName(&m_iJungleLatitude, "iJungleLatitude");
	pXML->GetChildXmlValByName(&m_iHillRange, "iHillRange");
	pXML->GetChildXmlValByName(&m_iPeakPercent, "iPeakPercent");

	pXML->GetChildXmlValByName(&m_fSnowLatitudeChange, "fSnowLatitudeChange");
	pXML->GetChildXmlValByName(&m_fTundraLatitudeChange, "fTundraLatitudeChange");
	pXML->GetChildXmlValByName(&m_fGrassLatitudeChange, "fGrassLatitudeChange");
	pXML->GetChildXmlValByName(&m_fDesertBottomLatitudeChange, "fDesertBottomLatitudeChange");
	pXML->GetChildXmlValByName(&m_fDesertTopLatitudeChange, "fDesertTopLatitudeChange");
	pXML->GetChildXmlValByName(&m_fIceLatitude, "fIceLatitude");
	pXML->GetChildXmlValByName(&m_fRandIceLatitude, "fRandIceLatitude");

	return true;
}

//======================================================================================================
//					CvSeaLevelInfo
//======================================================================================================
CvSeaLevelInfo::CvSeaLevelInfo() :
m_iSeaLevelChange(0)
{
}

CvSeaLevelInfo::~CvSeaLevelInfo()
{
}

int CvSeaLevelInfo::getSeaLevelChange() const
{
	return m_iSeaLevelChange; 
}

bool CvSeaLevelInfo::read(CvXMLLoadUtility* pXML)
{
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(&m_iSeaLevelChange, "iSeaLevelChange");

	return true;
}

//======================================================================================================
//					CvProcessInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvProcessInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvProcessInfo::CvProcessInfo() :
m_iTechPrereq(NO_TECH),
m_paiProductionToCommerceModifier(NULL)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvProcessInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvProcessInfo::~CvProcessInfo()
{
	SAFE_DELETE_ARRAY(m_paiProductionToCommerceModifier);
}

int CvProcessInfo::getTechPrereq() const	
{
	return m_iTechPrereq; 
}

// Arrays

int CvProcessInfo::getProductionToCommerceModifier(int i) const	
{
	FAssertMsg(i < NUM_COMMERCE_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_paiProductionToCommerceModifier ? m_paiProductionToCommerceModifier[i] : -1; 
}

bool CvProcessInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(szTextVal, "TechPrereq");
	m_iTechPrereq = pXML->FindInInfoClass(szTextVal, GC.getTechInfo(), sizeof(GC.getTechInfo((TechTypes)0)), GC.getNumTechInfos());

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"ProductionToCommerceModifiers"))
	{
		pXML->SetCommerce(&m_paiProductionToCommerceModifier);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_paiProductionToCommerceModifier, NUM_COMMERCE_TYPES);
	}

	return true;
}

//======================================================================================================
//					CvVoteInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvVoteInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvVoteInfo::CvVoteInfo() :
m_iPopulationThreshold(0),
m_iTradeRoutes(0),
m_bSecretaryGeneral(0),
m_bVictory(0),
m_bFreeTrade(0),
m_bNoNukes(0),
m_pbForceCivic(NULL)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvVoteInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvVoteInfo::~CvVoteInfo()
{
	SAFE_DELETE_ARRAY(m_pbForceCivic);
}

int CvVoteInfo::getPopulationThreshold() const
{
	return m_iPopulationThreshold; 
}

int CvVoteInfo::getTradeRoutes() const
{
	return m_iTradeRoutes; 
}

bool CvVoteInfo::isSecretaryGeneral() const
{
	return m_bSecretaryGeneral; 
}

bool CvVoteInfo::isVictory() const
{
	return m_bVictory; 
}

bool CvVoteInfo::isFreeTrade() const
{
	return m_bFreeTrade; 
}

bool CvVoteInfo::isNoNukes() const	
{
	return m_bNoNukes; 
}

bool CvVoteInfo::isForceCivic(int i) const
{
	FAssertMsg(i < GC.getNumCivicInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbForceCivic ? m_pbForceCivic[i] : false; 
}

bool CvVoteInfo::read(CvXMLLoadUtility* pXML)
{
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(&m_iPopulationThreshold, "iPopulationThreshold");
	pXML->GetChildXmlValByName(&m_iTradeRoutes, "iTradeRoutes");

	pXML->GetChildXmlValByName(&m_bSecretaryGeneral, "bSecretaryGeneral");
	pXML->GetChildXmlValByName(&m_bVictory, "bVictory");
	pXML->GetChildXmlValByName(&m_bFreeTrade, "bFreeTrade");
	pXML->GetChildXmlValByName(&m_bNoNukes, "bNoNukes");

	pXML->SetVariableListTagPair(&m_pbForceCivic, "ForceCivics", GC.getCivicInfo(), sizeof(GC.getCivicInfo((CivicTypes)0)), GC.getNumCivicInfos());

	return true;
}

//======================================================================================================
//					CvProjectInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvProjectInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvProjectInfo::CvProjectInfo() :
m_iVictoryPrereq(NO_VICTORY),
m_iTechPrereq(NO_TECH),
m_iAnyoneProjectPrereq(NO_PROJECT),
m_iMaxGlobalInstances(0),
m_iMaxTeamInstances(0),
m_iProductionCost(0),
m_iNukeInterception(0),
m_iTechShare(0),
m_iEveryoneSpecialUnit(NO_SPECIALUNIT),
m_iEveryoneSpecialBuilding(NO_SPECIALBUILDING),
m_bSpaceship(false),
m_piBonusProductionModifier(NULL),
m_piVictoryThreshold(NULL),
m_piProjectsNeeded(NULL)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvProjectInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvProjectInfo::~CvProjectInfo()
{
	SAFE_DELETE_ARRAY(m_piBonusProductionModifier);
	SAFE_DELETE_ARRAY(m_piVictoryThreshold);
	SAFE_DELETE_ARRAY(m_piProjectsNeeded);
}

int CvProjectInfo::getVictoryPrereq() const		
{
	return m_iVictoryPrereq; 
}

int CvProjectInfo::getTechPrereq() const
{
	return m_iTechPrereq; 
}

int CvProjectInfo::getAnyoneProjectPrereq() const
{
	return m_iAnyoneProjectPrereq; 
}

void CvProjectInfo::setAnyoneProjectPrereq(int i)
{
	m_iAnyoneProjectPrereq = i;
}

int CvProjectInfo::getMaxGlobalInstances() const
{
	return m_iMaxGlobalInstances; 
}

int CvProjectInfo::getMaxTeamInstances() const
{
	return m_iMaxTeamInstances; 
}

int CvProjectInfo::getProductionCost() const	
{
	return m_iProductionCost; 
}

int CvProjectInfo::getNukeInterception() const
{
	return m_iNukeInterception; 
}

int CvProjectInfo::getTechShare() const
{
	return m_iTechShare; 
}

int CvProjectInfo::getEveryoneSpecialUnit() const
{
	return m_iEveryoneSpecialUnit;
}

int CvProjectInfo::getEveryoneSpecialBuilding() const
{
	return m_iEveryoneSpecialBuilding;
}

bool CvProjectInfo::isSpaceship() const			
{
	return m_bSpaceship; 
}

const char* CvProjectInfo::getMovieArtDef() const	
{
	return m_szMovieArtDef; 
}

const TCHAR* CvProjectInfo::getCreateSound() const	
{
	return m_szCreateSound; 
}

void CvProjectInfo::setCreateSound(const TCHAR* szVal)
{
	m_szCreateSound = szVal; 
}

// Arrays

int CvProjectInfo::getBonusProductionModifier(int i) const										
{
	FAssertMsg(i < GC.getNumBonusInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piBonusProductionModifier ? m_piBonusProductionModifier[i] : -1; 
}

int CvProjectInfo::getVictoryThreshold(int i) const																					
{
	FAssertMsg(i < GC.getNumVictoryInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piVictoryThreshold ? m_piVictoryThreshold[i] : -1; 
}

int CvProjectInfo::getProjectsNeeded(int i) const
{
	FAssertMsg(i < GC.getNumProjectInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piProjectsNeeded ? m_piProjectsNeeded[i] : false; 
}

bool CvProjectInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(szTextVal, "VictoryPrereq");
	m_iVictoryPrereq = pXML->FindInInfoClass(szTextVal, GC.getVictoryInfo(), sizeof(GC.getVictoryInfo((VictoryTypes)0)), GC.getNumVictoryInfos());

	pXML->GetChildXmlValByName(szTextVal, "TechPrereq");
	m_iTechPrereq = pXML->FindInInfoClass(szTextVal, GC.getTechInfo(), sizeof(GC.getTechInfo((TechTypes)0)), GC.getNumTechInfos());

	pXML->GetChildXmlValByName(&m_iMaxGlobalInstances, "iMaxGlobalInstances");
	pXML->GetChildXmlValByName(&m_iMaxTeamInstances, "iMaxTeamInstances");
	pXML->GetChildXmlValByName(&m_iProductionCost, "iCost");
	pXML->GetChildXmlValByName(&m_iNukeInterception, "iNukeInterception");
	pXML->GetChildXmlValByName(&m_iTechShare, "iTechShare");

	pXML->GetChildXmlValByName(szTextVal, "EveryoneSpecialUnit");
	m_iEveryoneSpecialUnit = pXML->FindInInfoClass(szTextVal, GC.getSpecialUnitInfo(), sizeof(GC.getSpecialUnitInfo((SpecialUnitTypes)0)), GC.getNumSpecialUnitInfos());

	pXML->GetChildXmlValByName(szTextVal, "EveryoneSpecialBuilding");
	m_iEveryoneSpecialBuilding = pXML->FindInInfoClass(szTextVal, GC.getSpecialBuildingInfo(), sizeof(GC.getSpecialBuildingInfo((SpecialBuildingTypes)0)), GC.getNumSpecialBuildingInfos());

	pXML->GetChildXmlValByName(&m_bSpaceship, "bSpaceship");
	pXML->GetChildXmlValByName(m_szMovieArtDef, "MovieDefineTag");

	pXML->SetVariableListTagPair(&m_piBonusProductionModifier, "BonusProductionModifiers", GC.getBonusInfo(), sizeof(GC.getBonusInfo((BonusTypes)0)), GC.getNumBonusInfos());
	pXML->SetVariableListTagPair(&m_piVictoryThreshold, "VictoryThresholds", GC.getVictoryInfo(), sizeof(GC.getVictoryInfo((VictoryTypes)0)), GC.getNumVictoryInfos());
	pXML->SetVariableListTagPair(&m_piProjectsNeeded, "PrereqProjects", GC.getProjectInfo(), sizeof(GC.getProjectInfo((ProjectTypes)0)), GC.getNumProjectInfos());

	pXML->GetChildXmlValByName(szTextVal, "CreateSound");
	setCreateSound(szTextVal);

	return true;
}
//======================================================================================================
//					CvReligionInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvReligionInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvReligionInfo::CvReligionInfo() :
m_iChar(0),
m_iHolyCityChar(0),
m_iTechPrereq(NO_TECH),
m_iFreeUnitClass(NO_UNITCLASS),
m_iSpreadFactor(0),
m_iMissionType(NO_MISSION),
m_paiGlobalReligionCommerce(NULL),
m_paiHolyCityCommerce(NULL),
m_paiStateReligionCommerce(NULL)
{
	reset();
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvReligionInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvReligionInfo::~CvReligionInfo()
{
	SAFE_DELETE_ARRAY(m_paiGlobalReligionCommerce);
	SAFE_DELETE_ARRAY(m_paiHolyCityCommerce);
	SAFE_DELETE_ARRAY(m_paiStateReligionCommerce);
}

int CvReligionInfo::getChar() const
{
	return m_iChar; 
}

void CvReligionInfo::setChar(int i)
{
	m_iChar = i; 
}

int CvReligionInfo::getHolyCityChar() const		
{
	return m_iHolyCityChar; 
}

void CvReligionInfo::setHolyCityChar(int i)
{
	m_iHolyCityChar = i; 
}

int CvReligionInfo::getTechPrereq() const
{
	return m_iTechPrereq; 
}

int CvReligionInfo::getFreeUnitClass() const
{
	return m_iFreeUnitClass; 
}

int CvReligionInfo::getSpreadFactor() const
{
	return m_iSpreadFactor;
}

int CvReligionInfo::getMissionType() const
{
	return m_iMissionType;
}

void CvReligionInfo::setMissionType(int iNewType)
{
	m_iMissionType = iNewType;
}

const TCHAR* CvReligionInfo::getTechButton() const
{
	return m_szTechButton; 
}

void CvReligionInfo::setTechButton(const TCHAR* szVal)
{
	m_szTechButton=szVal; 
}

const TCHAR* CvReligionInfo::getMovieFile() const
{
	return m_szMovieFile;
}

void CvReligionInfo::setMovieFile(const TCHAR* szVal)
{
	m_szMovieFile = szVal;
}

const TCHAR* CvReligionInfo::getMovieSound() const
{
	return m_szMovieSound;
}

void CvReligionInfo::setMovieSound(const TCHAR* szVal)
{
	m_szMovieSound = szVal;
}

const TCHAR* CvReligionInfo::getButtonDisabled( void ) const
{
	static TCHAR szDisabled[512];

	szDisabled[0] = '\0';

	if ( getButton() && strlen(getButton()) > 4 )
	{
		strncpy( szDisabled, getButton(), strlen(getButton()) - 4 );
		szDisabled[strlen(getButton()) - 4] = '\0';
		strcat( szDisabled, "_D.dds" );
	}

	return szDisabled;
}

const TCHAR* CvReligionInfo::getSound() const
{
	return m_szSound; 
}

void CvReligionInfo::setSound(const TCHAR* szVal)
{
	m_szSound=szVal; 
}

// Arrays

int CvReligionInfo::getGlobalReligionCommerce(int i) const
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_paiGlobalReligionCommerce ? m_paiGlobalReligionCommerce[i] : -1; 
}

int* CvReligionInfo::getGlobalReligionCommerceArray() const
{
	return m_paiGlobalReligionCommerce;
}

int CvReligionInfo::getHolyCityCommerce(int i) const	
{
	FAssertMsg(i < NUM_COMMERCE_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_paiHolyCityCommerce ? m_paiHolyCityCommerce[i] : -1; 
}

int* CvReligionInfo::getHolyCityCommerceArray() const
{
	return m_paiHolyCityCommerce;
}

int CvReligionInfo::getStateReligionCommerce(int i) const	
{
	FAssertMsg(i < NUM_COMMERCE_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_paiStateReligionCommerce ? m_paiStateReligionCommerce[i] : -1; 
}

int* CvReligionInfo::getStateReligionCommerceArray() const
{
	return m_paiStateReligionCommerce;
}

//
// read from xml
//
bool CvReligionInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvHotkeyInfo::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(szTextVal, "TechPrereq");
	m_iTechPrereq = pXML->FindInInfoClass(szTextVal, GC.getTechInfo(), sizeof(GC.getTechInfo((TechTypes)0)), GC.getNumTechInfos());

	pXML->GetChildXmlValByName(szTextVal, "FreeUnitClass");
	m_iFreeUnitClass = pXML->FindInInfoClass(szTextVal, GC.getUnitClassInfo(), sizeof(GC.getUnitClassInfo((UnitClassTypes)0)), GC.getNumUnitClassInfos());

	pXML->GetChildXmlValByName(&m_iSpreadFactor, "iSpreadFactor");

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"GlobalReligionCommerces"))
	{
		pXML->SetYields(&m_paiGlobalReligionCommerce);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_paiGlobalReligionCommerce, NUM_YIELD_TYPES);
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"HolyCityCommerces"))
	{
		pXML->SetCommerce(&m_paiHolyCityCommerce);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_paiHolyCityCommerce, NUM_COMMERCE_TYPES);
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"StateReligionCommerces"))
	{
		pXML->SetCommerce(&m_paiStateReligionCommerce);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_paiStateReligionCommerce, NUM_COMMERCE_TYPES);
	}

	pXML->GetChildXmlValByName(szTextVal, "TechButton");
	setTechButton(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "MovieFile");
	setMovieFile(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "MovieSound");
	setMovieSound(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "Sound");
	setSound(szTextVal);

	return true;
}

//======================================================================================================
//					CvTraitInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvTraitInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvTraitInfo::CvTraitInfo() :
m_iHealth(0),													
m_iHappiness(0),													
m_iMaxAnarchy(0),											
m_iUpkeepModifier(0),									
m_iLevelExperienceModifier(0),									
m_iGreatPeopleRateModifier(0),						
m_iGreatGeneralRateModifier(0),						
m_iDomesticGreatGeneralRateModifier(0),						
m_iMaxGlobalBuildingProductionModifier(0),	
m_iMaxTeamBuildingProductionModifier(0),		
m_iMaxPlayerBuildingProductionModifier(0),
m_paiExtraYieldThreshold(NULL),
m_paiTradeYieldModifier(NULL),
m_paiCommerceChange(NULL),
m_paiCommerceModifier(NULL),
m_pabFreePromotionUnitCombat(NULL),
m_pabFreePromotion(NULL)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvTraitInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvTraitInfo::~CvTraitInfo()
{
	SAFE_DELETE_ARRAY(m_paiExtraYieldThreshold);
	SAFE_DELETE_ARRAY(m_paiTradeYieldModifier);
	SAFE_DELETE_ARRAY(m_paiCommerceChange);
	SAFE_DELETE_ARRAY(m_paiCommerceModifier);
	SAFE_DELETE_ARRAY(m_pabFreePromotionUnitCombat);
	SAFE_DELETE_ARRAY(m_pabFreePromotion);
}

int CvTraitInfo::getHealth() const									
{
	return m_iHealth; 
}

int CvTraitInfo::getHappiness() const									
{
	return m_iHappiness; 
}

int CvTraitInfo::getMaxAnarchy() const							
{
	return m_iMaxAnarchy; 
}

int CvTraitInfo::getUpkeepModifier() const					
{
	return m_iUpkeepModifier; 
}

int CvTraitInfo::getLevelExperienceModifier() const					
{
	return m_iLevelExperienceModifier; 
}

int CvTraitInfo::getGreatPeopleRateModifier() const	
{
	return m_iGreatPeopleRateModifier; 
}

int CvTraitInfo::getGreatGeneralRateModifier() const	
{
	return m_iGreatGeneralRateModifier; 
}

int CvTraitInfo::getDomesticGreatGeneralRateModifier() const	
{
	return m_iDomesticGreatGeneralRateModifier; 
}

int CvTraitInfo::getMaxGlobalBuildingProductionModifier() const	
{
	return m_iMaxGlobalBuildingProductionModifier; 
}

int CvTraitInfo::getMaxTeamBuildingProductionModifier() const		
{
	return m_iMaxTeamBuildingProductionModifier; 
}

int CvTraitInfo::getMaxPlayerBuildingProductionModifier() const	
{
	return m_iMaxPlayerBuildingProductionModifier; 
}

const TCHAR* CvTraitInfo::getShortDescription() const
{
	return m_szShortDescription; 
}

void CvTraitInfo::setShortDescription(const TCHAR* szVal)
{
	m_szShortDescription = szVal; 
}

// Arrays

int CvTraitInfo::getExtraYieldThreshold(int i) const
{
	return m_paiExtraYieldThreshold ? m_paiExtraYieldThreshold[i] : -1; 
}

int CvTraitInfo::getTradeYieldModifier(int i) const
{
	return m_paiTradeYieldModifier ? m_paiTradeYieldModifier[i] : -1; 
}

int CvTraitInfo::getCommerceChange(int i) const			
{
	return m_paiCommerceChange ? m_paiCommerceChange[i] : -1; 
}

int CvTraitInfo::getCommerceModifier(int i) const		
{
	return m_paiCommerceModifier ? m_paiCommerceModifier[i] : -1; 
}

int CvTraitInfo::isFreePromotion(int i) const
{
	return m_pabFreePromotion ? m_pabFreePromotion[i] : -1; 
}

int CvTraitInfo::isFreePromotionUnitCombat(int i) const
{
	return m_pabFreePromotionUnitCombat ? m_pabFreePromotionUnitCombat[i] : -1; 
}

bool CvTraitInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(szTextVal, "ShortDescription");
	setShortDescription(szTextVal);

	pXML->GetChildXmlValByName(&m_iHealth, "iHealth");
	pXML->GetChildXmlValByName(&m_iHappiness, "iHappiness");
	pXML->GetChildXmlValByName(&m_iMaxAnarchy, "iMaxAnarchy");
	pXML->GetChildXmlValByName(&m_iUpkeepModifier, "iUpkeepModifier");
	pXML->GetChildXmlValByName(&m_iLevelExperienceModifier, "iLevelExperienceModifier");
	pXML->GetChildXmlValByName(&m_iGreatPeopleRateModifier, "iGreatPeopleRateModifier");
	pXML->GetChildXmlValByName(&m_iGreatGeneralRateModifier, "iGreatGeneralRateModifier");
	pXML->GetChildXmlValByName(&m_iDomesticGreatGeneralRateModifier, "iDomesticGreatGeneralRateModifier");
	pXML->GetChildXmlValByName(&m_iMaxGlobalBuildingProductionModifier, "iMaxGlobalBuildingProductionModifier");
	pXML->GetChildXmlValByName(&m_iMaxTeamBuildingProductionModifier, "iMaxTeamBuildingProductionModifier");
	pXML->GetChildXmlValByName(&m_iMaxPlayerBuildingProductionModifier, "iMaxPlayerBuildingProductionModifier");

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(), "ExtraYieldThresholds"))
	{
		pXML->SetYields(&m_paiExtraYieldThreshold);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_paiExtraYieldThreshold, NUM_YIELD_TYPES);
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(), "TradeYieldModifiers"))
	{
		pXML->SetYields(&m_paiTradeYieldModifier);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_paiTradeYieldModifier, NUM_YIELD_TYPES);
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(), "CommerceChanges"))
	{
		pXML->SetCommerce(&m_paiCommerceChange);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_paiCommerceChange, NUM_COMMERCE_TYPES);
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(), "CommerceModifiers"))
	{
		pXML->SetCommerce(&m_paiCommerceModifier);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_paiCommerceModifier, NUM_COMMERCE_TYPES);
	}

	pXML->SetVariableListTagPair(&m_pabFreePromotion, "FreePromotions", GC.getPromotionInfo(), sizeof(GC.getPromotionInfo((PromotionTypes)0)), GC.getNumPromotionInfos());

	pXML->SetVariableListTagPair(&m_pabFreePromotionUnitCombat, "FreePromotionUnitCombats", GC.getUnitCombatInfo(), sizeof(GC.getUnitCombatInfo((UnitCombatTypes)0)), GC.getNumUnitCombatInfos());

	return true;
}

//======================================================================================================
//					CvCursorInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvCursorInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvCursorInfo::CvCursorInfo()
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvCursorInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvCursorInfo::~CvCursorInfo()
{
}

const TCHAR* CvCursorInfo::getPath()
{
	return m_szPath;
}

void CvCursorInfo::setPath(const TCHAR* szVal)
{
	m_szPath = szVal; 
}

bool CvCursorInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(szTextVal, "CursorPath");
	setPath(szTextVal);

	return true;
}

//======================================================================================================
//					CvThroneRoomCamera
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvThroneRoomCamera()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvThroneRoomCamera::CvThroneRoomCamera()
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvThroneRoomCamera()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvThroneRoomCamera::~CvThroneRoomCamera()
{
}

const TCHAR* CvThroneRoomCamera::getFileName()
{
	return m_szFileName;
}

void CvThroneRoomCamera::setFileName(const TCHAR* szVal)
{
	m_szFileName = szVal; 
}

bool CvThroneRoomCamera::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(szTextVal, "FileName");
	setFileName(szTextVal);

	return true;
}

//======================================================================================================
//					CvThroneRoomInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvThroneRoomInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvThroneRoomInfo::CvThroneRoomInfo() :
m_iFromState(0),
m_iToState(0),
m_iAnimation(0)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvThroneRoomInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvThroneRoomInfo::~CvThroneRoomInfo()
{
}

const TCHAR* CvThroneRoomInfo::getEvent()
{
	return m_szEvent;
}

void CvThroneRoomInfo::setEvent(const TCHAR* szVal)
{
	m_szEvent = szVal; 
}

const TCHAR* CvThroneRoomInfo::getNodeName()
{
	return m_szNodeName;
}

void CvThroneRoomInfo::setNodeName(const TCHAR* szVal)
{
	m_szNodeName = szVal; 
}

int CvThroneRoomInfo::getFromState()
{
	return m_iFromState;
}

void CvThroneRoomInfo::setFromState(int iVal)
{
	m_iFromState = iVal; 
}

int CvThroneRoomInfo::getToState()
{
	return m_iToState;
}

void CvThroneRoomInfo::setToState(int iVal)
{
	m_iToState = iVal; 
}

int CvThroneRoomInfo::getAnimation()
{
	return m_iAnimation;
}

void CvThroneRoomInfo::setAnimation(int iVal)
{
	m_iAnimation= iVal; 
}

bool CvThroneRoomInfo::read(CvXMLLoadUtility* pXML)
{
	int iVal;
	CvString szTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(szTextVal, "Event");
	setEvent(szTextVal);
	pXML->GetChildXmlValByName(&iVal, "iFromState" );
	setFromState(iVal);
	pXML->GetChildXmlValByName(&iVal, "iToState" );
	setToState(iVal);
	pXML->GetChildXmlValByName(szTextVal, "NodeName");
	setNodeName(szTextVal);
	pXML->GetChildXmlValByName(&iVal, "iAnimation" );
	setAnimation(iVal);

	return true;
}

//======================================================================================================
//					CvThroneRoomStyleInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvThroneRoomStyleInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvThroneRoomStyleInfo::CvThroneRoomStyleInfo()
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvThroneRoomStyleInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvThroneRoomStyleInfo::~CvThroneRoomStyleInfo()
{
}

const TCHAR* CvThroneRoomStyleInfo::getArtStyleType()
{
	return m_szArtStyleType;
}

void CvThroneRoomStyleInfo::setArtStyleType(const TCHAR* szVal)
{
	m_szArtStyleType = szVal; 
}

const TCHAR* CvThroneRoomStyleInfo::getEraType()
{
	return m_szEraType;
}

void CvThroneRoomStyleInfo::setEraType(const TCHAR* szVal)
{
	m_szEraType = szVal; 
}

const TCHAR* CvThroneRoomStyleInfo::getFileName()
{
	return m_szFileName;
}

void CvThroneRoomStyleInfo::setFileName(const TCHAR* szVal)
{
	m_szFileName = szVal; 
}

bool CvThroneRoomStyleInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(szTextVal, "ArtStyleType");
	setArtStyleType(szTextVal);
	pXML->GetChildXmlValByName(szTextVal, "EraType");
	setEraType(szTextVal);
	pXML->GetChildXmlValByName(szTextVal, "FileName");
	setFileName(szTextVal);

	//node names
	if(gDLL->getXMLIFace()->SetToChild(pXML->GetXML()))
	{
		while(gDLL->getXMLIFace()->LocateNextSiblingNodeByTagName(pXML->GetXML(), "NodeName"))
		{
			pXML->GetXmlVal(szTextVal);
			m_aNodeNames.push_back(szTextVal);
		}
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	//texture names
	if(gDLL->getXMLIFace()->SetToChild(pXML->GetXML()))
	{
		while(gDLL->getXMLIFace()->LocateNextSiblingNodeByTagName(pXML->GetXML(), "TextureName"))
		{
			pXML->GetXmlVal(szTextVal);
			m_aTextureNames.push_back(szTextVal);
		}
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	return true;
}

//======================================================================================================
//					CvSlideShowInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvSlideShowInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvSlideShowInfo::CvSlideShowInfo() :
m_fStartTime(0.0f)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvSlideShowInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvSlideShowInfo::~CvSlideShowInfo()
{
}

const TCHAR* CvSlideShowInfo::getPath()
{
	return m_szPath;
}

void CvSlideShowInfo::setPath(const TCHAR* szVal)
{
	m_szPath = szVal; 
}

const TCHAR* CvSlideShowInfo::getTransitionType()
{
	return m_szTransitionType;
}

void CvSlideShowInfo::setTransitionType(const TCHAR* szVal)
{
	m_szTransitionType = szVal; 
}

float CvSlideShowInfo::getStartTime()
{
	return m_fStartTime;
}

void CvSlideShowInfo::setStartTime(float fVal)
{
	m_fStartTime = fVal; 
}

bool CvSlideShowInfo::read(CvXMLLoadUtility* pXML)
{
	float fVal;
	CvString szTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(szTextVal, "Path");
	setPath(szTextVal);
	pXML->GetChildXmlValByName(szTextVal, "TransitionType");
	setTransitionType(szTextVal);
	pXML->GetChildXmlValByName(&fVal, "fStartTime");
	setStartTime(fVal);

	return true;
}

//======================================================================================================
//					CvSlideShowRandomInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvSlideShowRandomInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvSlideShowRandomInfo::CvSlideShowRandomInfo()
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvSlideShowRandomInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvSlideShowRandomInfo::~CvSlideShowRandomInfo()
{
}

const TCHAR* CvSlideShowRandomInfo::getPath()
{
	return m_szPath;
}

void CvSlideShowRandomInfo::setPath(const TCHAR* szVal)
{
	m_szPath = szVal; 
}

bool CvSlideShowRandomInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(szTextVal, "Path");
	setPath(szTextVal);

	return true;
}

//======================================================================================================
//					CvWorldPickerInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvWorldPickerInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvWorldPickerInfo::CvWorldPickerInfo()
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvWorldPickerInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvWorldPickerInfo::~CvWorldPickerInfo()
{
}

const TCHAR* CvWorldPickerInfo::getMapName()
{
	return m_szMapName;
}

void CvWorldPickerInfo::setMapName(const TCHAR* szVal)
{
	m_szMapName = szVal;
}

const TCHAR* CvWorldPickerInfo::getModelFile()
{
	return m_szModelFile;
}

void CvWorldPickerInfo::setModelFile(const TCHAR* szVal)
{
	m_szModelFile = szVal;
}

int CvWorldPickerInfo::getNumSizes()
{
	return m_aSizes.size();
}

float CvWorldPickerInfo::getSize(int index)
{
	return m_aSizes[index];
}

int CvWorldPickerInfo::getNumClimates()
{
	return m_aClimates.size();
}

const TCHAR* CvWorldPickerInfo::getClimatePath(int index)
{
	return m_aClimates[index];
}

int CvWorldPickerInfo::getNumWaterLevelDecals()
{
	return m_aWaterLevelDecals.size();
}

const TCHAR* CvWorldPickerInfo::getWaterLevelDecalPath(int index)
{
	return m_aWaterLevelDecals[index];
}

int CvWorldPickerInfo::getNumWaterLevelGloss()
{
	return m_aWaterLevelGloss.size();
}

const TCHAR* CvWorldPickerInfo::getWaterLevelGlossPath(int index)
{
	return m_aWaterLevelGloss[index];
}

bool CvWorldPickerInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	float fVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(szTextVal, "MapName");
	setMapName(szTextVal);
	pXML->GetChildXmlValByName(szTextVal, "ModelFile");
	setModelFile(szTextVal);

	//sizes
	if(gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(), "Sizes"))
	{
		if(gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(), "Size"))
		{
			do
			{
				pXML->GetXmlVal(&fVal);
				m_aSizes.push_back(fVal);
			} while(gDLL->getXMLIFace()->LocateNextSiblingNodeByTagName(pXML->GetXML(), "Size"));
			
			gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
		}
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	//climates
	if(gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(), "Climates"))
	{
		if(gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(), "ClimatePath"))
		{
			do
			{
				pXML->GetXmlVal(szTextVal);
				m_aClimates.push_back(szTextVal);
			} while(gDLL->getXMLIFace()->LocateNextSiblingNodeByTagName(pXML->GetXML(), "ClimatePath"));

			gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
		}
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	//water level decals
	if(gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(), "WaterLevelDecals"))
	{
		if(gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(), "WaterLevelDecalPath"))
		{
			do
			{
				pXML->GetXmlVal(szTextVal);
				m_aWaterLevelDecals.push_back(szTextVal);
			} while(gDLL->getXMLIFace()->LocateNextSiblingNodeByTagName(pXML->GetXML(), "WaterLevelDecalPath"));

			gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
		}
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	//water level gloss
	if(gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(), "WaterLevelGloss"))
	{
		if(gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(), "WaterLevelGlossPath"))
		{
			do
			{
				pXML->GetXmlVal(szTextVal);
				m_aWaterLevelGloss.push_back(szTextVal);
			} while(gDLL->getXMLIFace()->LocateNextSiblingNodeByTagName(pXML->GetXML(), "WaterLevelGlossPath"));

			gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
		}
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	return true;
}

//======================================================================================================
//					CvAnimationPathInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvAnimationPathInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvAnimationPathInfo::CvAnimationPathInfo() :
	m_bMissionPath(false)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvAnimationPathInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvAnimationPathInfo::~CvAnimationPathInfo()
{
}

int CvAnimationPathInfo::getPathCategory( int i )
{
	return (int)m_vctPathDefinition.size() > i ? m_vctPathDefinition[i].first : -1;
}

float CvAnimationPathInfo::getPathParameter( int i )
{
	return (int)m_vctPathDefinition.size() > i ? m_vctPathDefinition[i].second : -1;
}

CvAnimationPathDefinition * CvAnimationPathInfo::getPath( )
{
	return &m_vctPathDefinition;
}

bool CvAnimationPathInfo::isMissionPath() const
{
	return m_bMissionPath;
}

//------------------------------------------------------------------------------------------------
// FUNCTION:    CvAnimationPathInfo::read
//! \brief      Reads in a CvAnimationPathInfo definition from XML
//! \param      pXML Pointer to the XML loading object
//! \retval     true if the definition was read successfully, false otherwise
//------------------------------------------------------------------------------------------------
bool CvAnimationPathInfo::read(CvXMLLoadUtility* pXML)
{
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	TCHAR	szTempString[1024];				// Extracting text
	int		iCurrentPath = 0;					// The current path information we are building
	int		iCurrentCategory;				// The current category information we are building
	float	fParameter;						// Temporary

	pXML->GetChildXmlValByName( &m_bMissionPath, "bMissionPath" );
	gDLL->getXMLIFace()->SetToChild(pXML->GetXML() );
	gDLL->getXMLIFace()->GetLastNodeText(pXML->GetXML(), szTempString);
	gDLL->getXMLIFace()->NextSibling(pXML->GetXML());
	gDLL->getXMLIFace()->NextSibling(pXML->GetXML());
	do
	{
		if ( pXML->GetChildXmlValByName( szTempString, _T("Category") ))
		{
			iCurrentCategory = pXML->FindInInfoClass( szTempString, GC.getAnimationCategoryInfo(), sizeof(GC.getAnimationCategoryInfo((AnimationCategoryTypes)0)),
				GC.getNumAnimationCategoryInfos());
			fParameter = 0.0f;
		}
		else 
		{			
			pXML->GetChildXmlValByName( szTempString, _T("Operator"));
			iCurrentCategory = GC.getTypesEnum(szTempString);
			iCurrentCategory = ((int)ANIMOP_FIRST) + iCurrentCategory;
			if ( !pXML->GetChildXmlValByName( &fParameter, "Parameter" ) )
			{
				fParameter = 0.0f;
			}
		}
			m_vctPathDefinition.push_back( std::make_pair(iCurrentCategory, fParameter ));
	}
	while ( gDLL->getXMLIFace()->NextSibling(pXML->GetXML()));
	gDLL->getXMLIFace()->SetToParent(pXML->GetXML());

	return true;
}

//======================================================================================================
//					CvAnimationCategoryInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvAnimationCategoryInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvAnimationCategoryInfo::CvAnimationCategoryInfo()
{
	m_kCategory.second = -7540; // invalid.
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvAnimationCategoryInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvAnimationCategoryInfo::~CvAnimationCategoryInfo()
{
}

int CvAnimationCategoryInfo::getCategoryBaseID( )	
{
	return m_kCategory.first;
}

int CvAnimationCategoryInfo::getCategoryDefaultTo( )	
{
	if ( m_kCategory.second < -1 )
	{
		// CvXMLLoadUtility *pXML = new CvXMLLoadUtility();
		m_kCategory.second = CvXMLLoadUtility::FindInInfoClass( m_szDefaultTo, GC.getAnimationCategoryInfo(),
			sizeof( GC.getAnimationCategoryInfo((AnimationCategoryTypes)0)), GC.getNumAnimationCategoryInfos() );
	}
	return (int)m_kCategory.second;
}

bool CvAnimationCategoryInfo::read(CvXMLLoadUtility* pXML)
{
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	int		iBaseID;						// Temporary
	pXML->GetChildXmlValByName( m_szDefaultTo, "DefaultTo");
	pXML->GetChildXmlValByName( &iBaseID, "BaseID");
	m_kCategory.first = iBaseID;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// CvEntityEventInfo
/////////////////////////////////////////////////////////////////////////////////////////////

CvEntityEventInfo::CvEntityEventInfo() :
m_bUpdateFormation(true)
{
}

CvEntityEventInfo::~CvEntityEventInfo()
{
}

bool CvEntityEventInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTmp, szTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	int iNumSibs, i;

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"AnimationPathTypes"))
	{
		// Skip any comments and stop at the next value we might want
		if (pXML->SkipToNextVal())
		{
			// get the total number of children the current xml node has
			iNumSibs = gDLL->getXMLIFace()->GetNumChildren(pXML->GetXML());
			if (iNumSibs > 0)
			{
				// if the call to the function that sets the current xml node to it's first non-comment
				// child and sets the parameter with the new node's value succeeds
				if (pXML->GetChildXmlVal(szTmp))
				{
					AnimationPathTypes eAnimationPath = (AnimationPathTypes)CvXMLLoadUtility::FindInInfoClass( szTmp, 
						GC.getAnimationPathInfo(), sizeof(GC.getAnimationPathInfo((AnimationPathTypes)0)), 
						GC.getNumAnimationPathInfos());
					if ( eAnimationPath > ANIMATIONPATH_NONE )
						m_vctAnimationPathType.push_back( eAnimationPath );

					// loop through all the siblings, we start at 1 since we already have the first value
					for (i=1;i<iNumSibs;i++)
					{
						if (!pXML->GetNextXmlVal(szTmp))
						{
							break;
						}
						AnimationPathTypes eAnimationPath = (AnimationPathTypes)CvXMLLoadUtility::FindInInfoClass( szTmp, 
							GC.getAnimationPathInfo(), sizeof(GC.getAnimationPathInfo((AnimationPathTypes)0)), 
							GC.getNumAnimationPathInfos());
						if ( eAnimationPath > ANIMATIONPATH_NONE )
							m_vctAnimationPathType.push_back( eAnimationPath );

					}
					gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
				}
			}
		}
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"EffectTypes"))
	{
		// Skip any comments and stop at the next value we might want
		if (pXML->SkipToNextVal())
		{
			// get the total number of children the current xml node has
			iNumSibs = gDLL->getXMLIFace()->GetNumChildren(pXML->GetXML());
			if (iNumSibs > 0)
			{
				// if the call to the function that sets the current xml node to it's first non-comment
				// child and sets the parameter with the new node's value succeeds
				if (pXML->GetChildXmlVal(szTmp))
				{
					EffectTypes eEffectType = (EffectTypes)CvXMLLoadUtility::FindInInfoClass( szTmp, 
						GC.getEffectInfo(), sizeof(GC.getEffectInfo((EffectTypes)0)), 
						GC.getNumEffectInfos());
					if ( eEffectType > NO_EFFECT )
						m_vctEffectTypes.push_back( eEffectType );

					// loop through all the siblings, we start at 1 since we already have the first value
					for (i=1;i<iNumSibs;i++)
					{
						if (!pXML->GetNextXmlVal(szTmp))
						{
							break;
						}
						EffectTypes eEffectType = (EffectTypes)CvXMLLoadUtility::FindInInfoClass( szTmp, 
							GC.getEffectInfo(), sizeof(GC.getEffectInfo((EffectTypes)0)), 
							GC.getNumEffectInfos());
						if ( eEffectType > NO_EFFECT )
							m_vctEffectTypes.push_back( eEffectType );
					}
					gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
				}
			}
		}
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	pXML->GetChildXmlValByName( &m_bUpdateFormation, "bUpdateFormation" );

	return true;
}

AnimationPathTypes CvEntityEventInfo::getAnimationPathType(int iIndex) const
{
	return iIndex >= (int)m_vctAnimationPathType.size() ? ANIMATIONPATH_NONE : m_vctAnimationPathType[iIndex];
}

EffectTypes CvEntityEventInfo::getEffectType(int iIndex) const
{
	return iIndex >= (int)m_vctEffectTypes.size() ? NO_EFFECT : m_vctEffectTypes[iIndex];
}

int CvEntityEventInfo::getAnimationPathCount() const
{
	return m_vctAnimationPathType.size();
}

int CvEntityEventInfo::getEffectTypeCount() const
{
	return m_vctEffectTypes.size();
}

bool CvEntityEventInfo::getUpdateFormation() const
{
	return m_bUpdateFormation;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// CvAssetInfoBase
/////////////////////////////////////////////////////////////////////////////////////////////

const TCHAR* CvAssetInfoBase::getTag() const			
{
	return getType(); 
}

void CvAssetInfoBase::setTag(const TCHAR* szDesc)		
{
	setType(szDesc); 
}

const TCHAR* CvAssetInfoBase::getPath() const			
{
	return m_szPath; 
}

void CvAssetInfoBase::setPath(const TCHAR* szDesc)
{
	m_szPath = szDesc; 
}

bool CvAssetInfoBase::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvInfoBase::read(pXML))		// 'tag' is the same as 'type'
	{
		return false;
	}

	pXML->GetChildXmlValByName(szTextVal, "Path");
	setPath(szTextVal);

	return true;	
}

/////////////////////////////////////////////////////////////////////////////////////////////
// CvArtInfoAsset
/////////////////////////////////////////////////////////////////////////////////////////////

const TCHAR* CvArtInfoAsset::getNIF() const
{
	return m_szNIF; 
}

const TCHAR* CvArtInfoAsset::getKFM() const
{
	return m_szKFM; 
}

void CvArtInfoAsset::setNIF(const TCHAR* szDesc)		
{
	m_szNIF = szDesc; 
}

void CvArtInfoAsset::setKFM(const TCHAR* szDesc)		
{
	m_szKFM = szDesc; 
}

bool CvArtInfoAsset::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvAssetInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(szTextVal, "NIF");
	setNIF(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "KFM");
	setKFM(szTextVal);


	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// CvArtInfoBonus
/////////////////////////////////////////////////////////////////////////////////////////////

bool CvArtInfoBonus::read(CvXMLLoadUtility* pXML)
{
	if (!CvArtInfoScalableAsset::read(pXML))
	{
		return false;
	}

	CvString szTextVal;
	pXML->GetChildXmlValByName(szTextVal, "SHADERNIF");
	setShaderNIF(szTextVal);

	return true;
}

const TCHAR* CvArtInfoBonus::getShaderNIF() const
{
	return m_szShaderNIF; 
}
void CvArtInfoBonus::setShaderNIF(const TCHAR* szDesc)		
{
	m_szShaderNIF = szDesc; 
}

const CvArtInfoBonus* CvBonusInfo::getArtInfo() const
{
	return ARTFILEMGR.getBonusArtInfo( getArtDefineTag());
}


/////////////////////////////////////////////////////////////////////////////////////////////
// CvArtInfoUnit
/////////////////////////////////////////////////////////////////////////////////////////////

CvArtInfoUnit::CvArtInfoUnit() :
m_fShadowScale(0.0f),	
m_iDamageStates(0),
m_bActAsRanged(false),
m_bCombatExempt(false),
m_fTrailWidth(0.0f),
m_fTrailLength(0.0f),
m_fTrailTaper(0.0f),
m_fTrailFadeStartTime(0.0f),
m_fTrailFadeFalloff(0.0f),
m_fRangedDeathTime(0.0f),
m_fExchangeAngle(0.0f),
m_bSmoothMove(false),
m_fAngleInterRate(FLT_MAX),
m_iRunLoopSoundTag(0),
m_iRunEndSoundTag(0),
m_iSelectionSoundScriptId(0),
m_iActionSoundScriptId(0),
m_iPatrolSoundTag(0)
{
}

CvArtInfoUnit::~CvArtInfoUnit()
{
}

bool CvArtInfoUnit::getActAsRanged() const
{
	return m_bActAsRanged;
}

const TCHAR* CvArtInfoUnit::getShaderNIF() const
{
	return m_szShaderNIF; 
}

void CvArtInfoUnit::setShaderNIF(const TCHAR* szDesc)		
{
	m_szShaderNIF = szDesc; 
}

const TCHAR* CvArtInfoUnit::getShadowNIF() const
{
	return m_szShadowNIF;
}

float CvArtInfoUnit::getShadowScale() const
{
	return m_fShadowScale;
}

const TCHAR* CvArtInfoUnit::getShadowAttachNode() const
{
	return m_szShadowAttach;
}

int CvArtInfoUnit::getDamageStates() const
{
	return m_iDamageStates;
}


const TCHAR* CvArtInfoUnit::getTrailTexture() const
{
	return m_szTrailTexture;
}

float CvArtInfoUnit::getTrailWidth() const
{
	return m_fTrailWidth;
}

float CvArtInfoUnit::getTrailLength() const
{
	return m_fTrailLength;
}

float CvArtInfoUnit::getTrailTaper() const
{
	return m_fTrailTaper;
}

float CvArtInfoUnit::getTrailFadeStarTime() const
{
	return m_fTrailFadeStartTime;
}

float CvArtInfoUnit::getTrailFadeFalloff() const
{
	return m_fTrailFadeFalloff;
}

float CvArtInfoUnit::getBattleDistance() const
{
	return m_fBattleDistance;
}

float CvArtInfoUnit::getRangedDeathTime() const
{
	return m_fRangedDeathTime;
}

float CvArtInfoUnit::getExchangeAngle() const
{
	return m_fExchangeAngle;
}

bool CvArtInfoUnit::getCombatExempt() const
{
	return m_bCombatExempt;
}

bool CvArtInfoUnit::getSmoothMove() const
{
	return m_bSmoothMove;
}

float CvArtInfoUnit::getAngleInterpRate() const
{
	return m_fAngleInterRate;
}

bool CvArtInfoUnit::read(CvXMLLoadUtility* pXML)
{
	if (!CvArtInfoScalableAsset::read(pXML))
	{
		return false;
	}

	CvString szTextVal;

	pXML->GetChildXmlValByName(szTextVal, "ActionSound");
	m_iActionSoundScriptId = (szTextVal.GetLength() > 0) ? gDLL->getAudioTagIndex( szTextVal.GetCString(), AUDIOTAG_3DSCRIPT ) : -1;
	pXML->GetChildXmlValByName(szTextVal, "SelectionSound");
	m_iSelectionSoundScriptId = (szTextVal.GetLength() > 0) ? gDLL->getAudioTagIndex( szTextVal.GetCString(), AUDIOTAG_3DSCRIPT ) : -1;
	pXML->GetChildXmlValByName(szTextVal, "PatrolSound");
	m_iPatrolSoundTag = (szTextVal.GetLength() > 0) ? gDLL->getAudioTagIndex( szTextVal.GetCString(), AUDIOTAG_3DSCRIPT ) : -1;

	pXML->GetChildXmlValByName(szTextVal, "TrainSound");
	setTrainSound(szTextVal);

	pXML->GetChildXmlValByName(&m_bActAsRanged, "bActAsRanged" );
	pXML->GetChildXmlValByName(&m_bCombatExempt, "bCombatExempt", false );
	pXML->GetChildXmlValByName(&m_fExchangeAngle, "fExchangeAngle", 0.0f );
	pXML->GetChildXmlValByName(&m_bSmoothMove, "bSmoothMove", false );
	pXML->GetChildXmlValByName(&m_fAngleInterRate, "fAngleInterpRate", FLT_MAX );

	pXML->GetChildXmlValByName(szTextVal, "SHADERNIF");
	setShaderNIF(szTextVal);

	if ( gDLL->getXMLIFace()->SetToChildByTagName( pXML->GetXML(), "ShadowDef" ))
	{
		pXML->GetChildXmlValByName( m_szShadowAttach, "ShadowAttachNode" );
		pXML->GetChildXmlValByName(m_szShadowNIF, "ShadowNIF");
		pXML->GetChildXmlValByName(&m_fShadowScale, "fShadowScale");
		gDLL->getXMLIFace()->SetToParent( pXML->GetXML());
	}

	pXML->GetChildXmlValByName(&m_iDamageStates, "iDamageStates", 0);
	pXML->GetChildXmlValByName(&m_fBattleDistance, "fBattleDistance", 0.0f);
	pXML->GetChildXmlValByName(&m_fRangedDeathTime, "fRangedDeathTime", 0.0f );

	m_fTrailWidth = -1.0f; // invalid.
	if ( gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(), "TrailDefinition"))
	{
		pXML->GetChildXmlValByName(m_szTrailTexture, "Texture" );
		pXML->GetChildXmlValByName(&m_fTrailWidth, "fWidth" );
		pXML->GetChildXmlValByName(&m_fTrailLength, "fLength" );
		pXML->GetChildXmlValByName(&m_fTrailTaper, "fTaper" );
		pXML->GetChildXmlValByName(&m_fTrailFadeStartTime, "fFadeStartTime" );
		pXML->GetChildXmlValByName(&m_fTrailFadeFalloff, "fFadeFalloff" );
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML() );
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"AudioRunSounds"))
	{
		pXML->GetChildXmlValByName(szTextVal, "AudioRunTypeLoop");
		m_iRunLoopSoundTag = GC.getFootstepAudioTypeByTag(szTextVal);
		pXML->GetChildXmlValByName(szTextVal, "AudioRunTypeEnd");
		m_iRunEndSoundTag = GC.getFootstepAudioTypeByTag(szTextVal);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	return true;
}

const TCHAR* CvArtInfoUnit::getTrainSound() const						
{
	return m_szTrainSound;
}

void CvArtInfoUnit::setTrainSound(const TCHAR* szVal)
{
	m_szTrainSound = szVal;
}

int CvArtInfoUnit::getRunLoopSoundTag() const						
{
	return m_iRunLoopSoundTag;
}

int CvArtInfoUnit::getRunEndSoundTag() const						
{
	return m_iRunEndSoundTag;
}

int CvArtInfoUnit::getPatrolSoundTag() const
{
	return m_iPatrolSoundTag;
}

int CvArtInfoUnit::getSelectionSoundScriptId() const
{
	return m_iSelectionSoundScriptId;
}

int CvArtInfoUnit::getActionSoundScriptId() const
{
	return m_iActionSoundScriptId;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// CvArtInfoBuilding
/////////////////////////////////////////////////////////////////////////////////////////////

CvArtInfoBuilding::CvArtInfoBuilding() :
m_bAnimated(false)
{
}

CvArtInfoBuilding::~CvArtInfoBuilding()
{
}

bool CvArtInfoBuilding::isAnimated() const
{
	return m_bAnimated; 
}

bool CvArtInfoBuilding::read(CvXMLLoadUtility* pXML)
{
	if (!CvArtInfoScalableAsset::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(&m_bAnimated, "bAnimated");
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// CvArtInfoCivilization
/////////////////////////////////////////////////////////////////////////////////////////////

CvArtInfoCivilization::CvArtInfoCivilization() :
m_bWhiteFlag(false)
{
}

CvArtInfoCivilization::~CvArtInfoCivilization()
{
}

bool CvArtInfoCivilization::isWhiteFlag() const
{
	return m_bWhiteFlag; 
}

bool CvArtInfoCivilization::read(CvXMLLoadUtility* pXML)
{
	if (!CvArtInfoAsset::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(&m_bWhiteFlag, "bWhiteFlag");
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// CvArtInfoLeaderhead
/////////////////////////////////////////////////////////////////////////////////////////////

const TCHAR* CvArtInfoLeaderhead::getNoShaderNIF() const
{
	return m_szNoShaderNIF; 
}

void CvArtInfoLeaderhead::setNoShaderNIF(const TCHAR* szNIF)
{
	m_szNoShaderNIF = szNIF; 
}

const TCHAR* CvArtInfoLeaderhead::getBackgroundKFM() const
{
	return m_szBackgroundKFM;
}

void CvArtInfoLeaderhead::setBackgroundKFM( const TCHAR* szKFM)
{
	m_szBackgroundKFM = szKFM;
}

bool CvArtInfoLeaderhead::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvArtInfoAsset::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(szTextVal, "NoShaderNIF");
	setNoShaderNIF(szTextVal);

	if (pXML->GetChildXmlValByName(szTextVal, "BackgroundKFM"))
		setBackgroundKFM(szTextVal);
	else
		setBackgroundKFM("");

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// CvArtInfoScalableAsset
/////////////////////////////////////////////////////////////////////////////////////////////

bool CvArtInfoScalableAsset::read(CvXMLLoadUtility* pXML)
{
	if (!CvArtInfoAsset::read(pXML))
	{
		return false;
	}

	return CvScalableInfo::read(pXML);
}

//////////////////////////////////////////////////////////////////////////
// CvArtInfoImprovement
//////////////////////////////////////////////////////////////////////////

CvArtInfoImprovement::CvArtInfoImprovement() :
m_bExtraAnimations(false)
{
}

CvArtInfoImprovement::~CvArtInfoImprovement()
{
}

bool CvArtInfoImprovement::isExtraAnimations() const
{
	return m_bExtraAnimations;
}

bool CvArtInfoImprovement::read(CvXMLLoadUtility* pXML)
{
	if (!CvArtInfoScalableAsset::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(&m_bExtraAnimations, "bExtraAnimations");

	CvString szTextVal;
	pXML->GetChildXmlValByName(szTextVal, "SHADERNIF");
	setShaderNIF(szTextVal);

	return true;
}

//////////////////////////////////////////////////////////////////////////
// CvArtInfoTerrain
//////////////////////////////////////////////////////////////////////////

CvArtInfoTerrain::CvArtInfoTerrain() :
m_iLayerOrder(0),
m_numTextureBlends(16),
m_pTextureSlots(NULL)
{
	m_pTextureSlots = new CvTextureBlendSlotList * [m_numTextureBlends];
	for ( int i = 0; i < m_numTextureBlends; i++ )
	{
		m_pTextureSlots[i] = new CvTextureBlendSlotList;
	}

}

CvArtInfoTerrain::~CvArtInfoTerrain() 
{
	for ( int i = 0; i < m_numTextureBlends; i++ )
	{
		SAFE_DELETE(m_pTextureSlots[i]);
	}
	SAFE_DELETE_ARRAY( m_pTextureSlots);
}

const TCHAR* CvArtInfoTerrain::getBaseTexture()
{
	return getPath(); 
}

void CvArtInfoTerrain::setBaseTexture(const TCHAR* szTmp )
{
	setPath(szTmp); 
}

const TCHAR* CvArtInfoTerrain::getDetailTexture()
{
	return m_szDetailTexture; 
}

int CvArtInfoTerrain::getLayerOrder()
{
	return m_iLayerOrder;
}
CvTextureBlendSlotList &CvArtInfoTerrain::getBlendList(int blendMask)
{
	FAssert(blendMask>0 && blendMask<16);
	return *m_pTextureSlots[blendMask];
}

void CvArtInfoTerrain::setDetailTexture(const TCHAR* szTmp)
{
	m_szDetailTexture = szTmp; 
};

void BuildSlotList( CvTextureBlendSlotList &list, CvString &numlist)
{
	//convert string to 
	char seps[]   = " ,\t\n";
	char *token;
	const char *numstring = numlist;
	token = strtok( const_cast<char *>(numstring), seps);
	while( token != NULL )
	{
		int slot = atoi(token);
		token = strtok( NULL, seps);
		int rotation = atoi(token);
		list.push_back(std::make_pair( slot, rotation));
		token = strtok( NULL, seps);
	}
}

bool CvArtInfoTerrain::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvArtInfoAsset::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(szTextVal, "Detail");
	setDetailTexture(szTextVal);
	pXML->GetChildXmlValByName(&m_iLayerOrder, "LayerOrder");

	// Parse texture slots for blend tile lists
	char xmlName[] = "TextureBlend00";
	for(int i =1; i<m_numTextureBlends;i++ )
	{
		sprintf(xmlName+(strlen(xmlName)-2),"%02d",i);
		pXML->GetChildXmlValByName(szTextVal, xmlName);
		BuildSlotList(*m_pTextureSlots[i], szTextVal);
	}
    
	return CvArtInfoAsset::read(pXML);
}

//////////////////////////////////////////////////////////////////////////
// CvArtInfoFeature
//////////////////////////////////////////////////////////////////////////

CvArtInfoFeature::CvArtInfoFeature() :
m_bAnimated(false),
m_bRiverArt(false),
m_bTileArt(false)
{
}

CvArtInfoFeature::~CvArtInfoFeature()
{
}

bool CvArtInfoFeature::isAnimated() const
{
	return m_bAnimated;
}

bool CvArtInfoFeature::isRiverArt() const
{
	return m_bRiverArt;
}

bool CvArtInfoFeature::isTileArt() const
{
	return m_bTileArt;
}

bool CvArtInfoFeature::read(CvXMLLoadUtility* pXML)
{
	if (!CvArtInfoScalableAsset::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(&m_bAnimated, "bAnimated");
	pXML->GetChildXmlValByName(&m_bRiverArt, "bRiverArt");
	pXML->GetChildXmlValByName(&m_bTileArt, "bTileArt");

	return true;
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvEmphasizeInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvEmphasizeInfo::CvEmphasizeInfo() :
m_bAvoidGrowth(false),
m_bGreatPeople(false),
m_piYieldModifiers(NULL),
m_piCommerceModifiers(NULL)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvEmphasizeInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvEmphasizeInfo::~CvEmphasizeInfo()
{
	SAFE_DELETE_ARRAY(m_piYieldModifiers);
	SAFE_DELETE_ARRAY(m_piCommerceModifiers);
}

bool CvEmphasizeInfo::isAvoidGrowth() const
{
	return m_bAvoidGrowth; 
}

bool CvEmphasizeInfo::isGreatPeople() const
{
	return m_bGreatPeople; 
}

// Arrays

int CvEmphasizeInfo::getYieldChange(int i) const		
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piYieldModifiers ? m_piYieldModifiers[i] : -1;	
}

int CvEmphasizeInfo::getCommerceChange(int i) const
{
	FAssertMsg(i < NUM_COMMERCE_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piCommerceModifiers ? m_piCommerceModifiers[i] : -1;	
}

//
// read from XML
//
bool CvEmphasizeInfo::read(CvXMLLoadUtility* pXML)
{
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(&m_bAvoidGrowth, "bAvoidGrowth");
	pXML->GetChildXmlValByName(&m_bGreatPeople, "bGreatPeople");

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"YieldModifiers"))
	{
		pXML->SetYields(&m_piYieldModifiers);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piYieldModifiers, NUM_YIELD_TYPES);
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"CommerceModifiers"))
	{
		pXML->SetCommerce(&m_piCommerceModifiers);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	else
	{
		pXML->InitList(&m_piCommerceModifiers, NUM_COMMERCE_TYPES);
	}

	return true;
}

//------------------------------------------------------------------------------------------------------
//
//  CvUpkeepInfo
//

CvUpkeepInfo::CvUpkeepInfo() :
m_iPopulationPercent(0),
m_iCityPercent(0)
{
}

CvUpkeepInfo::~CvUpkeepInfo()
{
}

int CvUpkeepInfo::getPopulationPercent() const
{
	return m_iPopulationPercent; 
}

int CvUpkeepInfo::getCityPercent() const
{
	return m_iCityPercent; 
}

bool CvUpkeepInfo::read(CvXMLLoadUtility* pXml)
{
	if (!CvInfoBase::read(pXml))
	{
		return false;
	}

	pXml->GetChildXmlValByName(&m_iPopulationPercent, "iPopulationPercent");
	pXml->GetChildXmlValByName(&m_iCityPercent, "iCityPercent");

	return true;
}

//------------------------------------------------------------------------------------------------------
//
//  CvCultureLevelInfo
//

CvCultureLevelInfo::CvCultureLevelInfo() :
m_iCityDefenseModifier(0),
m_paiSpeedThreshold(NULL)
{
}

CvCultureLevelInfo::~CvCultureLevelInfo()
{
	SAFE_DELETE_ARRAY(m_paiSpeedThreshold);
}

int CvCultureLevelInfo::getCityDefenseModifier() const
{
	return m_iCityDefenseModifier; 
}

int CvCultureLevelInfo::getSpeedThreshold(int i) const
{
	FAssert(i < GC.getNumGameSpeedInfos());
	FAssert(i > -1);
	return m_paiSpeedThreshold[i];
}

bool CvCultureLevelInfo::read(CvXMLLoadUtility* pXml)
{
	if (!CvInfoBase::read(pXml))
	{
		return false;
	}

	pXml->GetChildXmlValByName(&m_iCityDefenseModifier, "iCityDefenseModifier");

	pXml->SetVariableListTagPair(&m_paiSpeedThreshold, "SpeedThresholds", GC.getGameSpeedInfo(), sizeof(GC.getGameSpeedInfo((GameSpeedTypes)0)), GC.getNumGameSpeedInfos());

	return true;
}

//------------------------------------------------------------------------------------------------------
//
//  CvEraInfo
//

CvEraInfo::CvEraInfo() :
m_iStartingUnitMultiplier(0),
m_iStartingDefenseUnits(0),
m_iStartingWorkerUnits(0),
m_iStartingExploreUnits(0),
m_iStartingGold(0),
m_iFreePopulation(0),
m_iStartPercent(0),
m_iGrowthPercent(0),
m_iTrainPercent(0),
m_iConstructPercent(0),
m_iCreatePercent(0),
m_iResearchPercent(0),
m_iBuildPercent(0),
m_iImprovementPercent(0),
m_iGreatPeoplePercent(0),
m_iAnarchyPercent(0),
m_iSoundtrackSpace(0),
m_iNumSoundtracks(0),
m_bNoGoodies(false),
m_bNoAnimals(false),
m_bNoBarbUnits(false),
m_bNoBarbCities(false),
m_bFirstSoundtrackFirst(false),
m_paiCitySoundscapeSciptIds(NULL),
m_paiSoundtracks(NULL)
{
}

CvEraInfo::~CvEraInfo()
{
	SAFE_DELETE_ARRAY(m_paiCitySoundscapeSciptIds);
	SAFE_DELETE_ARRAY(m_paiSoundtracks);
}

int CvEraInfo::getStartingUnitMultiplier() const
{
	return m_iStartingUnitMultiplier; 
}

int CvEraInfo::getStartingDefenseUnits() const
{
	return m_iStartingDefenseUnits; 
}

int CvEraInfo::getStartingWorkerUnits() const
{
	return m_iStartingWorkerUnits; 
}

int CvEraInfo::getStartingExploreUnits() const
{
	return m_iStartingExploreUnits; 
}

int CvEraInfo::getStartingGold() const
{
	return m_iStartingGold; 
}

int CvEraInfo::getFreePopulation() const
{
	return m_iFreePopulation; 
}

int CvEraInfo::getStartPercent() const
{
	return m_iStartPercent; 
}

int CvEraInfo::getGrowthPercent() const
{
	return m_iGrowthPercent; 
}

int CvEraInfo::getTrainPercent() const
{
	return m_iTrainPercent; 
}

int CvEraInfo::getConstructPercent() const
{
	return m_iConstructPercent; 
}

int CvEraInfo::getCreatePercent() const
{
	return m_iCreatePercent; 
}

int CvEraInfo::getResearchPercent() const
{
	return m_iResearchPercent; 
}

int CvEraInfo::getBuildPercent() const
{
	return m_iBuildPercent; 
}

int CvEraInfo::getImprovementPercent() const
{
	return m_iImprovementPercent; 
}

int CvEraInfo::getGreatPeoplePercent() const
{
	return m_iGreatPeoplePercent; 
}

int CvEraInfo::getAnarchyPercent() const
{
	return m_iAnarchyPercent; 
}

int CvEraInfo::getSoundtrackSpace() const
{
	return m_iSoundtrackSpace; 
}

bool CvEraInfo::isFirstSoundtrackFirst() const
{
	return m_bFirstSoundtrackFirst; 
}

int CvEraInfo::getNumSoundtracks() const
{
	return m_iNumSoundtracks; 
}

const TCHAR* CvEraInfo::getAudioUnitVictoryScript() const
{
	return m_szAudioUnitVictoryScript;
}

const TCHAR* CvEraInfo::getAudioUnitDefeatScript() const
{
	return m_szAudioUnitDefeatScript;
}

bool CvEraInfo::isNoGoodies() const
{
	return m_bNoGoodies; 
}

bool CvEraInfo::isNoAnimals() const
{
	return m_bNoAnimals; 
}

bool CvEraInfo::isNoBarbUnits() const
{
	return m_bNoBarbUnits; 
}

bool CvEraInfo::isNoBarbCities() const
{
	return m_bNoBarbCities; 
}

// Arrays

int CvEraInfo::getSoundtracks(int i) const
{
	FAssertMsg(i < getNumSoundtracks(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_paiSoundtracks ? m_paiSoundtracks[i] : -1; 
}

int CvEraInfo::getCitySoundscapeSciptId(int i) const
{
//	FAssertMsg(i < ?, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_paiCitySoundscapeSciptIds ? m_paiCitySoundscapeSciptIds[i] : -1; 
}

bool CvEraInfo::read(CvXMLLoadUtility* pXML)
{
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(&m_bNoGoodies, "bNoGoodies");
	pXML->GetChildXmlValByName(&m_bNoAnimals, "bNoAnimals");
	pXML->GetChildXmlValByName(&m_bNoBarbUnits, "bNoBarbUnits");
	pXML->GetChildXmlValByName(&m_bNoBarbCities, "bNoBarbCities");
	pXML->GetChildXmlValByName(&m_iStartingUnitMultiplier, "iStartingUnitMultiplier");
	pXML->GetChildXmlValByName(&m_iStartingDefenseUnits, "iStartingDefenseUnits");
	pXML->GetChildXmlValByName(&m_iStartingWorkerUnits, "iStartingWorkerUnits");
	pXML->GetChildXmlValByName(&m_iStartingExploreUnits, "iStartingExploreUnits");
	pXML->GetChildXmlValByName(&m_iStartingGold, "iStartingGold");
	pXML->GetChildXmlValByName(&m_iFreePopulation, "iFreePopulation");
	pXML->GetChildXmlValByName(&m_iStartPercent, "iStartPercent");
	pXML->GetChildXmlValByName(&m_iGrowthPercent, "iGrowthPercent");
	pXML->GetChildXmlValByName(&m_iTrainPercent, "iTrainPercent");
	pXML->GetChildXmlValByName(&m_iConstructPercent, "iConstructPercent");
	pXML->GetChildXmlValByName(&m_iCreatePercent, "iCreatePercent");
	pXML->GetChildXmlValByName(&m_iResearchPercent, "iResearchPercent");
	pXML->GetChildXmlValByName(&m_iBuildPercent, "iBuildPercent");
	pXML->GetChildXmlValByName(&m_iImprovementPercent, "iImprovementPercent");
	pXML->GetChildXmlValByName(&m_iGreatPeoplePercent, "iGreatPeoplePercent");
	pXML->GetChildXmlValByName(&m_iAnarchyPercent, "iAnarchyPercent");
	pXML->GetChildXmlValByName(&m_iSoundtrackSpace, "iSoundtrackSpace");
	pXML->GetChildXmlValByName(&m_bFirstSoundtrackFirst, "bFirstSoundtrackFirst");
	pXML->GetChildXmlValByName(m_szAudioUnitVictoryScript, "AudioUnitVictoryScript");
	pXML->GetChildXmlValByName(m_szAudioUnitDefeatScript, "AudioUnitDefeatScript");

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(), "EraInfoSoundtracks"))
	{
		CvString* pszSoundTrackNames = NULL;
		pXML->SetStringList(&pszSoundTrackNames, &m_iNumSoundtracks);

		if (m_iNumSoundtracks > 0)
		{
			m_paiSoundtracks = new int[m_iNumSoundtracks];

			int j;
			for (j=0;j<m_iNumSoundtracks;j++)
			{
				m_paiSoundtracks[j] = ((!gDLL->getAudioDisabled()) ? gDLL->getAudioTagIndex(pszSoundTrackNames[j], AUDIOTAG_2DSCRIPT) : -1);
			}
		}
		else
		{
			m_paiSoundtracks = NULL;
		}

		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());

		SAFE_DELETE_ARRAY(pszSoundTrackNames);
	}

	pXML->SetVariableListTagPairForAudioScripts(&m_paiCitySoundscapeSciptIds, "CitySoundscapes", GC.getCitySizeTypes(), sizeof(GC.getCitySizeTypes((CitySizeTypes)0)));

	return true;
}
//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvColorInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvColorInfo::CvColorInfo()
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvColorInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvColorInfo::~CvColorInfo()
{
}

const NiColorA& CvColorInfo::getColor() const
{
	return m_Color; 
}

bool CvColorInfo::read(CvXMLLoadUtility* pXML)
{
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	float afColorVals[4];	// array to hold the 4 color values, red, green, blue, and alpha	

	pXML->GetChildXmlValByName(&afColorVals[0], "fRed");
	pXML->GetChildXmlValByName(&afColorVals[1], "fGreen");
	pXML->GetChildXmlValByName(&afColorVals[2], "fBlue");
	pXML->GetChildXmlValByName(&afColorVals[3], "fAlpha");

	m_Color = NiColorA(afColorVals[0], afColorVals[1], afColorVals[2], afColorVals[3]);

	return true;
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvPlayerColorInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvPlayerColorInfo::CvPlayerColorInfo() :
m_iColorTypePrimary(NO_COLOR),
m_iColorTypeSecondary(NO_COLOR),
m_iTextColorType(NO_COLOR)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvPlayerColorInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvPlayerColorInfo::~CvPlayerColorInfo()
{
}

int CvPlayerColorInfo::getColorTypePrimary() const
{
	return m_iColorTypePrimary; 
}

int CvPlayerColorInfo::getColorTypeSecondary() const
{
	return m_iColorTypeSecondary; 
}

int CvPlayerColorInfo::getTextColorType() const
{
	return m_iTextColorType; 
}

bool CvPlayerColorInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(szTextVal, "ColorTypePrimary");
	m_iColorTypePrimary = pXML->FindInInfoClass( szTextVal, GC.getColorInfo(), sizeof(GC.getColorInfo((ColorTypes)0)), GC.getNumColorInfos());

	pXML->GetChildXmlValByName(szTextVal, "ColorTypeSecondary");
	m_iColorTypeSecondary = pXML->FindInInfoClass( szTextVal, GC.getColorInfo(), sizeof(GC.getColorInfo((ColorTypes)0)), GC.getNumColorInfos());

	pXML->GetChildXmlValByName(szTextVal, "TextColorType");
	m_iTextColorType = pXML->FindInInfoClass( szTextVal, GC.getColorInfo(), sizeof(GC.getColorInfo((ColorTypes)0)), GC.getNumColorInfos());

	return true;
}

//------------------------------------------------------------------------------------------------------
//
//	CvLandscapeInfo
//
//
CvLandscapeInfo::CvLandscapeInfo() :
m_iFogR(0),
m_iFogG(0),
m_iFogB(0),
m_iHorizontalGameCell(0),
m_iVerticalGameCell(0),
m_iPlotsPerCellX(0),
m_iPlotsPerCellY(0),
m_iHorizontalVertCnt(0),
m_iVerticalVertCnt(0),
m_iWaterHeight(0),
m_fTextureScaleX(0.0f),
m_fTextureScaleY(0.0f),
m_fZScale(0.0f),
m_bUseTerrainShader(false),
m_bUseLightmap(false),
m_bRandomMap(false)
{
}

int CvLandscapeInfo::getFogR() const
{
	return m_iFogR; 
}

int CvLandscapeInfo::getFogG() const
{
	return m_iFogG; 
}

int CvLandscapeInfo::getFogB() const
{
	return m_iFogB; 
}

int CvLandscapeInfo::getHorizontalGameCell() const
{
	return m_iHorizontalGameCell; 
}

int CvLandscapeInfo::getVerticalGameCell() const
{
	return m_iVerticalGameCell; 
}

int CvLandscapeInfo::getPlotsPerCellX() const
{
	return m_iPlotsPerCellX; 
}

int CvLandscapeInfo::getPlotsPerCellY() const
{
	return m_iPlotsPerCellY; 
}

int CvLandscapeInfo::getHorizontalVertCnt() const
{
	return m_iHorizontalVertCnt; 
}

int CvLandscapeInfo::getVerticalVertCnt() const
{
	return m_iVerticalVertCnt; 
}

int CvLandscapeInfo::getWaterHeight() const
{
	return m_iWaterHeight; 
}

float CvLandscapeInfo::getTextureScaleX() const
{
	return m_fTextureScaleX; 
}

float CvLandscapeInfo::getTextureScaleY() const
{
	return m_fTextureScaleY; 
}

float CvLandscapeInfo::getZScale() const
{
	return m_fZScale; 
}

bool CvLandscapeInfo::isUseTerrainShader() const
{
	return m_bUseTerrainShader; 
}

bool CvLandscapeInfo::isUseLightmap() const
{
	return m_bUseLightmap; 
}
float CvLandscapeInfo::getPeakScale() const
{
	return 	m_fPeakScale;
}

float CvLandscapeInfo::getHillScale() const
{
	return 	m_fHillScale;
}

bool CvLandscapeInfo::isRandomMap() const
{
	return m_bRandomMap; 
}

const TCHAR* CvLandscapeInfo::getSkyArt()
{
	return m_szSkyArt; 
}

void CvLandscapeInfo::setSkyArt(const TCHAR* szPath)
{
	m_szSkyArt = szPath; 
}

const TCHAR* CvLandscapeInfo::getHeightMap()
{
	return m_szHeightMap; 
}

void CvLandscapeInfo::setHeightMap(const TCHAR* szPath)
{
	m_szHeightMap = szPath; 
}

const TCHAR* CvLandscapeInfo::getTerrainMap()
{
	return m_szTerrainMap; 
}

void CvLandscapeInfo::setTerrainMap(const TCHAR* szPath)
{
	m_szTerrainMap = szPath; 
}

const TCHAR* CvLandscapeInfo::getNormalMap()
{
	return m_szNormalMap; 
}

void CvLandscapeInfo::setNormalMap(const TCHAR* szPath)
{
	m_szNormalMap = szPath; 
}

const TCHAR* CvLandscapeInfo::getBlendMap()
{
	return m_szBlendMap; 
}

void CvLandscapeInfo::setBlendMap(const TCHAR* szPath)
{
	m_szBlendMap = szPath; 
}

//
// read from xml
//
bool CvLandscapeInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(&m_iWaterHeight, "iWaterHeight");
	pXML->GetChildXmlValByName(&m_bRandomMap, "bRandomMap");

	pXML->GetChildXmlValByName(szTextVal, "HeightMap");
	setHeightMap(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "TerrainMap");
	setTerrainMap(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "NormalMap");
	setNormalMap(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "BlendMap");
	setBlendMap(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "SkyArt");
	setSkyArt(szTextVal);

	pXML->GetChildXmlValByName(&m_iFogR, "iFogR");
	pXML->GetChildXmlValByName(&m_iFogG, "iFogG");
	pXML->GetChildXmlValByName(&m_iFogB, "iFogB");

	pXML->GetChildXmlValByName(&m_fTextureScaleX, "fTextureScaleX");
	pXML->GetChildXmlValByName(&m_fTextureScaleY, "fTextureScaleY");

	pXML->GetChildXmlValByName(&m_iHorizontalGameCell, "iGameCellSizeX");
	pXML->GetChildXmlValByName(&m_iVerticalGameCell, "iGameCellSizeY");

	pXML->GetChildXmlValByName(&m_iPlotsPerCellX, "iPlotsPerCellX");
	pXML->GetChildXmlValByName(&m_iPlotsPerCellY, "iPlotsPerCellY");

	m_iHorizontalVertCnt = m_iPlotsPerCellX * m_iHorizontalGameCell - (m_iPlotsPerCellX - 1);
	m_iVerticalVertCnt   = m_iPlotsPerCellY * m_iVerticalGameCell - (m_iPlotsPerCellY - 1);

	pXML->GetChildXmlValByName(&m_fZScale, "fZScale");
	pXML->GetChildXmlValByName(&m_bUseTerrainShader, "bTerrainShader");
	pXML->GetChildXmlValByName(&m_bUseLightmap, "bUseLightmap");
	pXML->GetChildXmlValByName(&m_fPeakScale, "fPeakScale");
	pXML->GetChildXmlValByName(&m_fHillScale, "fHillScale");


	return true;
}

//////////////////////////////////////////////////////////////////////////
// CvGameText
//////////////////////////////////////////////////////////////////////////

// static
int CvGameText::NUM_LANGUAGES = 0;

int CvGameText::getNumLanguages() const
{
	return NUM_LANGUAGES;
}
void CvGameText::setNumLanguages(int iNum)
{
	NUM_LANGUAGES = iNum;
}

CvGameText::CvGameText() : 
	m_szGender("N"),
	m_szPlural("false")
{
}

const wchar* CvGameText::getText() const					
{
	return m_szText; 
}

void CvGameText::setText(const wchar* szText)				
{
	m_szText = szText; 
}

bool CvGameText::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	CvWString wszTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	gDLL->getXMLIFace()->SetToChild(pXML->GetXML()); // Move down to Child level
	pXML->GetXmlVal(szTextVal);		// TAG
	setType(szTextVal);

//	OutputDebugString(szTextVal.c_str());
//	OutputDebugString("\n");

	static const int iMaxNumLanguages = GC.getDefineINT("MAX_NUM_LANGUAGES");
	int iNumLanguages = NUM_LANGUAGES ? NUM_LANGUAGES : iMaxNumLanguages + 1;

	int j=0;
	for (j = 0; j < iNumLanguages; j++)
	{
		pXML->SkipToNextVal();	// skip comments

		if (!gDLL->getXMLIFace()->NextSibling(pXML->GetXML()) || j == iMaxNumLanguages)
		{
			NUM_LANGUAGES = j;
			break;
		}
		if (j == GAMETEXT.getCurrentLanguage()) // Only add appropriate language Text 
		{
			// TEXT
			if (pXML->GetChildXmlValByName(wszTextVal, "Text"))
			{
				setText(wszTextVal);
			}
			else
			{
				pXML->GetXmlVal(wszTextVal);
				setText(wszTextVal);
				if (NUM_LANGUAGES > 0)
				{
					break;
				}
			}

			// GENDER
			if (pXML->GetChildXmlValByName(szTextVal, "Gender"))
			{
				setGender(szTextVal);
			}

			// PLURAL
			if (pXML->GetChildXmlValByName(szTextVal, "Plural"))
			{
				setPlural(szTextVal);
			}
			if (NUM_LANGUAGES > 0)
			{
				break;
			}
		}
	}

	gDLL->getXMLIFace()->SetToParent(pXML->GetXML()); // Move back up to Parent

	return true;
}

//////////////////////////////////////////////////////////////////////////
//
//	CvDiplomacyTextInfo
//
//

CvDiplomacyTextInfo::CvDiplomacyTextInfo() :
m_iNumResponses(0),
m_pResponses(NULL)
{
}

// note - Response member vars allocated by CvXmlLoadUtility  
void CvDiplomacyTextInfo::init(int iNum)
{
	uninit();
	m_pResponses = new Response[iNum];
	m_iNumResponses=iNum; 
}

void CvDiplomacyTextInfo::uninit()
{
	SAFE_DELETE_ARRAY(m_pResponses); 
}

int CvDiplomacyTextInfo::getNumResponses() const
{
	return m_iNumResponses; 
}

bool CvDiplomacyTextInfo::getCivilizationTypes(int i, int j) const
{
	FAssertMsg(i < getNumResponses(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	FAssertMsg(j < GC.getNumCivilizationInfos(), "Index out of bounds");
	FAssertMsg(j > -1, "Index out of bounds");
	return m_pResponses[i].m_pbCivilizationTypes[j]; 
}

bool CvDiplomacyTextInfo::getLeaderHeadTypes(int i, int j) const
{
	FAssertMsg(i < getNumResponses(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	FAssertMsg(j < GC.getNumLeaderHeadInfos(), "Index out of bounds");
	FAssertMsg(j > -1, "Index out of bounds");
	return m_pResponses[i].m_pbLeaderHeadTypes[j]; 
}

bool CvDiplomacyTextInfo::getAttitudeTypes(int i, int j) const
{
	FAssertMsg(i < getNumResponses(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	FAssertMsg(j < NUM_ATTITUDE_TYPES, "Index out of bounds");
	FAssertMsg(j > -1, "Index out of bounds");
	return m_pResponses[i].m_pbAttitudeTypes[j]; 
}

bool CvDiplomacyTextInfo::getDiplomacyPowerTypes(int i, int j) const
{
	FAssertMsg(i < getNumResponses(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	FAssertMsg(j < NUM_DIPLOMACYPOWER_TYPES, "Index out of bounds");
	FAssertMsg(j > -1, "Index out of bounds");
	return m_pResponses[i].m_pbDiplomacyPowerTypes[j]; 
}

int CvDiplomacyTextInfo::getNumDiplomacyText(int i) const
{
	FAssertMsg(i < getNumResponses(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pResponses[i].m_iNumDiplomacyText; 
}

const TCHAR* CvDiplomacyTextInfo::getDiplomacyText(int i, int j) const
{
	FAssertMsg(i < getNumResponses(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	FAssertMsg(j < getNumDiplomacyText(i), "Index out of bounds");
	FAssertMsg(j > -1, "Index out of bounds");
	return m_pResponses[i].m_paszDiplomacyText[j]; 
}

void CvDiplomacyTextInfo::Response::read(FDataStreamBase* stream)
{
	stream->Read(&m_iNumDiplomacyText);

	SAFE_DELETE_ARRAY(m_pbCivilizationTypes);
	m_pbCivilizationTypes = new bool[GC.getNumCivilizationInfos()];
	stream->Read(GC.getNumCivilizationInfos(), m_pbCivilizationTypes);

	SAFE_DELETE_ARRAY(m_pbLeaderHeadTypes);
	m_pbLeaderHeadTypes = new bool[GC.getNumLeaderHeadInfos()];
	stream->Read(GC.getNumLeaderHeadInfos(), m_pbLeaderHeadTypes);

	SAFE_DELETE_ARRAY(m_pbAttitudeTypes);
	m_pbAttitudeTypes = new bool[NUM_ATTITUDE_TYPES];
	stream->Read(NUM_ATTITUDE_TYPES, m_pbAttitudeTypes);

	SAFE_DELETE_ARRAY(m_pbDiplomacyPowerTypes);
	m_pbDiplomacyPowerTypes = new bool[NUM_DIPLOMACYPOWER_TYPES];
	stream->Read(NUM_DIPLOMACYPOWER_TYPES, m_pbDiplomacyPowerTypes);

	SAFE_DELETE_ARRAY(m_paszDiplomacyText);
	m_paszDiplomacyText = new CvString[m_iNumDiplomacyText];
	stream->ReadString(m_iNumDiplomacyText, m_paszDiplomacyText);
}

void CvDiplomacyTextInfo::Response::write(FDataStreamBase* stream)
{
	stream->Write(m_iNumDiplomacyText);

	stream->Write(GC.getNumCivilizationInfos(), m_pbCivilizationTypes);
	stream->Write(GC.getNumLeaderHeadInfos(), m_pbLeaderHeadTypes);
	stream->Write(NUM_ATTITUDE_TYPES, m_pbAttitudeTypes);
	stream->Write(NUM_DIPLOMACYPOWER_TYPES, m_pbDiplomacyPowerTypes);
	stream->WriteString(m_iNumDiplomacyText, m_paszDiplomacyText);
}

void CvDiplomacyTextInfo::read(FDataStreamBase* stream)
{
	CvInfoBase::read(stream);

	uint uiFlag=0;
	stream->Read(&uiFlag);		// flag for expansion

	stream->Read(&m_iNumResponses);
	
	// Arrays

	init(m_iNumResponses);

	for (uint uiIndex = 0; (int) uiIndex < m_iNumResponses; uiIndex++)
	{
		m_pResponses[uiIndex].read(stream);
	}
}

void CvDiplomacyTextInfo::write(FDataStreamBase* stream)
{
	CvInfoBase::write(stream);

	uint uiFlag=0;
	stream->Write(uiFlag);		// flag for expansion

	stream->Write(m_iNumResponses);

	// Arrays

	for (uint uiIndex = 0; (int) uiIndex < m_iNumResponses; uiIndex++)
	{
		m_pResponses[uiIndex].write(stream);
	}
}

bool CvDiplomacyTextInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	int j;
	pXML->GetChildXmlValByName(szTextVal, "Type");

	if ( gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"Responses") )
	{
		int iIndexVal = gDLL->getXMLIFace()->NumOfChildrenByTagName(pXML->GetXML(), "Response");
		init(iIndexVal);

		for (j = 0; j < iIndexVal; j++)
		{
			if (j == 0)
			{
				gDLL->getXMLIFace()->SetToChild(pXML->GetXML());
			}
			// Civilizations
			pXML->SetVariableListTagPair(&m_pResponses[j].m_pbCivilizationTypes, "Civilizations",GC.getCivilizationInfo(), sizeof(GC.getCivilizationInfo((CivilizationTypes)0)), GC.getNumCivilizationInfos());
			// Leaders
			pXML->SetVariableListTagPair(&m_pResponses[j].m_pbLeaderHeadTypes, "Leaders", GC.getLeaderHeadInfo(), sizeof(GC.getLeaderHeadInfo((LeaderHeadTypes)0)), GC.getNumLeaderHeadInfos());
			// AttitudeTypes
			pXML->SetVariableListTagPair(&m_pResponses[j].m_pbAttitudeTypes, "Attitudes", GC.getAttitudeInfo(), sizeof(GC.getAttitudeInfo((AttitudeTypes)0)), NUM_ATTITUDE_TYPES);
			// PowerTypes
			pXML->SetVariableListTagPair(&m_pResponses[j].m_pbDiplomacyPowerTypes, "DiplomacyPowers", GC.getDiplomacyPowerTypes(), NUM_DIPLOMACYPOWER_TYPES);
			// DiplomacyText
			if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"DiplomacyText"))
			{
				pXML->SetStringList(&m_pResponses[j].m_paszDiplomacyText, &m_pResponses[j].m_iNumDiplomacyText);
				gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
			}

			if (!gDLL->getXMLIFace()->NextSibling(pXML->GetXML()))
			{
				break;
			}
		}

		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	gDLL->getXMLIFace()->SetToParent(pXML->GetXML());

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//	CvEffectInfo			Misc\CIV4EffectInfos.xml
//
//

CvEffectInfo::CvEffectInfo() :
m_fUpdateRate(0.0f),
m_bProjectile(false),
m_bSticky(false),
m_fProjectileSpeed(0.0f),
m_fProjectileArc(0.0f)
{
}

CvEffectInfo::~CvEffectInfo()
{
}

bool CvEffectInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	CvScalableInfo::read(pXML);

	pXML->GetChildXmlValByName(szTextVal, "Path");
	setPath(szTextVal);

	pXML->GetChildXmlValByName(&m_fUpdateRate, "fUpdateRate" );
	
	int iTemporary;
	pXML->GetChildXmlValByName(&iTemporary, "bIsProjectile" );
	m_bProjectile = iTemporary != 0;

	pXML->GetChildXmlValByName(&m_fProjectileSpeed, "fSpeed" );
	pXML->GetChildXmlValByName(&m_fProjectileArc, "fArcValue" );
	pXML->GetChildXmlValByName(&m_bSticky, "bSticky", false );
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//	CvAttachableInfo			Misc\CIV4AttachableInfos.xml
//
//
CvAttachableInfo::CvAttachableInfo() :
m_fUpdateRate(0.0f)
{
}

CvAttachableInfo::~CvAttachableInfo()
{
}

bool CvAttachableInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	CvScalableInfo::read(pXML);

	pXML->GetChildXmlValByName(szTextVal, "Path");
	setPath(szTextVal);

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//	CvCameraInfo			Interface\CIV4CameraInfos.xml
//
//
bool CvCameraInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(szTextVal, "Path");
	setPath(szTextVal);

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//	CvQuestInfo			Misc\CIV4QuestInfos.xml
//
//
CvQuestInfo::CvQuestInfo() :
m_iNumQuestMessages(0),
m_iNumQuestLinks(0),
m_iNumQuestSounds(0),
m_paszQuestMessages(NULL),
m_pQuestLinks(NULL),
m_paszQuestSounds(NULL)
{
	m_szQuestScript = "NONE";
}

CvQuestInfo::~CvQuestInfo()
{
	reset();
}


void CvQuestInfo::reset()
{
	SAFE_DELETE_ARRAY(m_paszQuestMessages); 
	SAFE_DELETE_ARRAY(m_pQuestLinks); 
	SAFE_DELETE_ARRAY(m_paszQuestSounds); 
}

bool CvQuestInfo::initQuestLinks(int iNum)
{
	reset();
	if ( iNum > 0 )
	{
		m_pQuestLinks = new QuestLink[iNum]; 
		m_iNumQuestLinks = iNum;
		return true;
	}
	return false;
}

const TCHAR* CvQuestInfo::getQuestObjective() const 
{
	return m_szQuestObjective; 
}

const TCHAR* CvQuestInfo::getQuestBodyText() const 
{ 
	return m_szQuestBodyText; 
}

int CvQuestInfo::getNumQuestMessages() const
{
	return m_iNumQuestMessages; 
}

const TCHAR* CvQuestInfo::getQuestMessages(int iIndex) const
{ 
	return m_paszQuestMessages ? m_paszQuestMessages[iIndex] : "";
}

int CvQuestInfo::getNumQuestLinks() const 
{
	return m_iNumQuestLinks; 
}

const TCHAR* CvQuestInfo::getQuestLinkType(int iIndex)  const
{
	return m_pQuestLinks[iIndex].m_szQuestLinkType; 
}

const TCHAR* CvQuestInfo::getQuestLinkName(int iIndex)  const
{ 
	return m_pQuestLinks[iIndex].m_szQuestLinkName; 
}

int CvQuestInfo::getNumQuestSounds() const 
{
	return m_iNumQuestSounds; 
}

const TCHAR* CvQuestInfo::getQuestSounds(int iIndex) const
{
	return m_paszQuestSounds ? m_paszQuestSounds[iIndex] : "";
}

const TCHAR* CvQuestInfo::getQuestScript() const
{
	return m_szQuestScript;
}

void CvQuestInfo::setQuestObjective(const TCHAR* szText) 
{
	m_szQuestObjective = szText; 
}

void CvQuestInfo::setQuestBodyText(const TCHAR* szText) 
{
	m_szQuestBodyText = szText; 
}

void CvQuestInfo::setNumQuestMessages(int iNum) 
{ 
	m_iNumQuestMessages = iNum; 
}

void CvQuestInfo::setQuestMessages(int iIndex, const TCHAR* szText) 
{
	m_paszQuestMessages[iIndex] = szText; 
}

void CvQuestInfo::setNumQuestSounds(int iNum) 
{ 
	m_iNumQuestSounds = iNum; 
}

void CvQuestInfo::setQuestSounds(int iIndex, const TCHAR* szText) 
{
	m_paszQuestSounds[iIndex] = szText; 
}

void CvQuestInfo::setQuestScript(const TCHAR* szText)
{
	m_szQuestScript = szText;
}

bool CvQuestInfo::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(szTextVal, "QuestObjective");
	setQuestObjective(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "QuestBodyText");
	setQuestBodyText(szTextVal);

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(), "QuestMessages"))
	{
		pXML->SetStringList(&m_paszQuestMessages, &m_iNumQuestMessages);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"QuestLinks"))
	{
		int iNum;
		iNum = gDLL->getXMLIFace()->NumOfChildrenByTagName(pXML->GetXML(), "QuestLink");

		if (initQuestLinks(iNum))
		{						
			int i;
			for (i=0; i<m_iNumQuestLinks; i++)
			{
				pXML->GetChildXmlValByName(szTextVal, "QuestLinkType");
				m_pQuestLinks[i].m_szQuestLinkType = szTextVal;
	
				pXML->GetChildXmlValByName(szTextVal, "QuestLinkName");
				m_pQuestLinks[i].m_szQuestLinkName = szTextVal;

				if (!gDLL->getXMLIFace()->NextSibling(pXML->GetXML()))
				{
					break;
				}
			}
		}
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(), "QuestSounds"))
	{
		pXML->SetStringList(&m_paszQuestSounds, &m_iNumQuestSounds);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	pXML->GetChildXmlValByName(szTextVal, "QuestScript");
	setQuestScript(szTextVal);

	return true;
}

//////////////////////////////////////////////////////////////////////////
//
//	CvTutorialMessage			
//
CvTutorialMessage::CvTutorialMessage() :
m_iNumTutorialScripts(0),
m_paszTutorialScripts(NULL)
{
	m_szTutorialMessageText = "No Text";
	m_szTutorialMessageImage = "No Text";
	m_szTutorialMessageSound = "No Text";
}

CvTutorialMessage::~CvTutorialMessage()
{
	SAFE_DELETE_ARRAY(m_paszTutorialScripts);
}

const TCHAR* CvTutorialMessage::getText() const
{
	return m_szTutorialMessageText;
}

const TCHAR* CvTutorialMessage::getImage() const
{
	return m_szTutorialMessageImage;
}

const TCHAR* CvTutorialMessage::getSound() const
{
	return m_szTutorialMessageSound;
}

void CvTutorialMessage::setText(const TCHAR* szText)
{
	m_szTutorialMessageText = szText;
}

void CvTutorialMessage::setImage(const TCHAR* szText)
{
	m_szTutorialMessageImage = szText;
}

void CvTutorialMessage::setSound(const TCHAR* szText)
{
	m_szTutorialMessageSound = szText;
}

int CvTutorialMessage::getNumTutorialScripts() const
{
	return m_iNumTutorialScripts;
}

const TCHAR* CvTutorialMessage::getTutorialScriptByIndex(int i) const
{
	return m_paszTutorialScripts[i];
}

bool CvTutorialMessage::read(CvXMLLoadUtility* pXML)
{
	// Skip any comments and stop at the next value we might want
	if (!pXML->SkipToNextVal())
	{
		return false;
	}
	CvString szTextVal;
	
	pXML->MapChildren();	// try to hash children for fast lookup by name
	if (pXML->GetChildXmlValByName(szTextVal, "TutorialMessageText"))
	{
		setText(szTextVal);
	}
	if (pXML->GetChildXmlValByName(szTextVal, "TutorialMessageImage"))
	{
		setImage(szTextVal);
	}
	if (pXML->GetChildXmlValByName(szTextVal, "TutorialMessageSound"))
	{
		setSound(szTextVal);
	}
	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(), "TutorialScripts"))
	{
		pXML->SetStringList(&m_paszTutorialScripts, &m_iNumTutorialScripts);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	return true;
}


CvTutorialInfo::CvTutorialInfo() :
m_iNumTutorialMessages(0),
m_paTutorialMessages(NULL)
{
	m_szNextTutorialInfoType = "NONE";
}

CvTutorialInfo::~CvTutorialInfo()
{
	resetMessages();
}

const TCHAR* CvTutorialInfo::getNextTutorialInfoType()
{
	return m_szNextTutorialInfoType;
}

void CvTutorialInfo::setNextTutorialInfoType(const TCHAR* szVal)
{
	m_szNextTutorialInfoType = szVal;
}

bool CvTutorialInfo::initTutorialMessages(int iNum)
{
	resetMessages();
	m_paTutorialMessages = new CvTutorialMessage[iNum];
	m_iNumTutorialMessages = iNum;
	return true;
}

void CvTutorialInfo::resetMessages()
{
	SAFE_DELETE_ARRAY(m_paTutorialMessages);
	m_iNumTutorialMessages = 0;
}

int CvTutorialInfo::getNumTutorialMessages() const
{
	return m_iNumTutorialMessages;
}

const CvTutorialMessage* CvTutorialInfo::getTutorialMessage(int iIndex) const
{
	return &m_paTutorialMessages[iIndex];
}

bool CvTutorialInfo::read(CvXMLLoadUtility* pXML)
{
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}
	CvString szTextVal;

	pXML->MapChildren();	// try to hash children for fast lookup by name
	if (pXML->GetChildXmlValByName(szTextVal, "NextTutorialInfoType"))
	{
		setNextTutorialInfoType(szTextVal);
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"TutorialMessages"))
	{
		int iNum;
		iNum = gDLL->getXMLIFace()->NumOfChildrenByTagName(pXML->GetXML(), "TutorialMessage");
		if ( iNum > 0 )
		{
			gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"TutorialMessage");
			initTutorialMessages(iNum);
			for (int i = 0; i<m_iNumTutorialMessages; i++)
			{
				if (!m_paTutorialMessages[i].read(pXML))
				{
					return false;
				}
				
				if (!gDLL->getXMLIFace()->NextSibling(pXML->GetXML()))
				{
					break;
				}
			}
			gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
		}
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//	CvGameOptionInfo			
//	Game options and their default values
//
//
CvGameOptionInfo::CvGameOptionInfo() :
m_bDefault(false)
{
}

CvGameOptionInfo::~CvGameOptionInfo()
{
}

bool CvGameOptionInfo::getDefault() const 
{ 
	return m_bDefault;
}

bool CvGameOptionInfo::read(CvXMLLoadUtility* pXML)
{
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(&m_bDefault, "bDefault");
	
	return true;
}

//////////////////////////////////////////////////////////////////////////
//
//	CvMPOptionInfo			
//	Multiplayer options and their default values
//
//
CvMPOptionInfo::CvMPOptionInfo() :
m_bDefault(false)
{
}

CvMPOptionInfo::~CvMPOptionInfo()
{
}

bool CvMPOptionInfo::getDefault() const 
{ 
	return m_bDefault;
}

bool CvMPOptionInfo::read(CvXMLLoadUtility* pXML)
{
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(&m_bDefault, "bDefault");

	return true;
}

//////////////////////////////////////////////////////////////////////////
//
//	CvForceControlInfo			
//	Forced Controls and their default values
//
//
CvForceControlInfo::CvForceControlInfo() :
m_bDefault(false)
{
}

CvForceControlInfo::~CvForceControlInfo()
{
}

bool CvForceControlInfo::getDefault() const 
{ 
	return m_bDefault;
}

bool CvForceControlInfo::read(CvXMLLoadUtility* pXML)
{
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(&m_bDefault, "bDefault");

	return true;
}

//////////////////////////////////////////////////////////////////////////
//
//	CvPlayerOptionInfo			
//	Player options and their default values
//
//
CvPlayerOptionInfo::CvPlayerOptionInfo() :
m_bDefault(false)
{
}

CvPlayerOptionInfo::~CvPlayerOptionInfo()
{
}

bool CvPlayerOptionInfo::getDefault() const 
{ 
	return m_bDefault;
}

bool CvPlayerOptionInfo::read(CvXMLLoadUtility* pXML)
{
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(&m_bDefault, "bDefault");

	return true;
}

//////////////////////////////////////////////////////////////////////////
//
//	CvGraphicOptionInfo			
//	Graphic options and their default values
//
//
CvGraphicOptionInfo::CvGraphicOptionInfo() :
m_bDefault(false)
{
}

CvGraphicOptionInfo::~CvGraphicOptionInfo()
{
}

bool CvGraphicOptionInfo::getDefault() const 
{ 
	return m_bDefault;
}

bool CvGraphicOptionInfo::read(CvXMLLoadUtility* pXML)
{
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	pXML->GetChildXmlValByName(&m_bDefault, "bDefault");

	return true;
}
