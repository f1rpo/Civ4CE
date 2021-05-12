//  $Header:
//------------------------------------------------------------------------------------------------
//
//  FILE:    CvInfoWater.cpp
//
//  AUTHOR:	
//					
//					
//
//  PURPOSE: The base class for all info classes to inherit from.  
//
//------------------------------------------------------------------------------------------------
//  Copyright (c) 2003 Firaxis Games, Inc. All rights reserved.
//------------------------------------------------------------------------------------------------
#include "CvGameCoreDLL.h"
#include "CvInfoWater.h"
#include "CvXMLLoadUtility.h"
#include "CvDLLXMLIFaceBase.h"
#include "CvGlobals.h"

//======================================================================================================
//					CvWaterPlaneInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvWaterPlaneInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvWaterPlaneInfo::CvWaterPlaneInfo()
{

}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvWaterPlaneInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvWaterPlaneInfo::~CvWaterPlaneInfo()
{
}
//------------------------------------------------------------------------------------------------------
float CvWaterPlaneInfo::getMaterialAlpha() const		// The water plane's material alpha
{
	return m_fMaterialAlpha;
}
//------------------------------------------------------------------------------------------------------
NiColor	CvWaterPlaneInfo::getMaterialDiffuse() const	// The water plane's material diffuse color
{
	return m_kMaterialDiffuse;
}
//------------------------------------------------------------------------------------------------------
NiColor	CvWaterPlaneInfo::getMaterialSpecular() const// The water plane's material specular color
{
	return m_kMaterialSpecular;
}
//------------------------------------------------------------------------------------------------------
NiColor	CvWaterPlaneInfo::getMaterialEmmisive() const// The water plane's material emmisive color
{
	return m_kMaterialEmmisive;
}
//------------------------------------------------------------------------------------------------------
const TCHAR * CvWaterPlaneInfo::getBaseTexture() const
{
	return m_szBaseTexture;
}
//------------------------------------------------------------------------------------------------------
void CvWaterPlaneInfo::setBaseTexture(const TCHAR* szVal)			// The filename of the base texture
{
	m_szBaseTexture=szVal;
}
//------------------------------------------------------------------------------------------------------
const TCHAR * CvWaterPlaneInfo::getTransitionTexture() const
{
	return m_szTransitionTexture;
}
//------------------------------------------------------------------------------------------------------
void CvWaterPlaneInfo::setTransitionTexture(const TCHAR* szVal)		// The filename of the detail texture
{
	m_szTransitionTexture=szVal;
}
//------------------------------------------------------------------------------------------------------
float CvWaterPlaneInfo::getTextureScaling() const
{
	return m_BaseTextureScale;
}
//------------------------------------------------------------------------------------------------------
float CvWaterPlaneInfo::getTextureScrollRateU() const
{
	return m_fURate;
}
//------------------------------------------------------------------------------------------------------
float CvWaterPlaneInfo::getTextureScrollRateV() const
{
	return m_fVRate;
}
//------------------------------------------------------------------------------------------------------
bool CvWaterPlaneInfo::read(CvXMLLoadUtility* pXML)
{
	CvString  szTextVal;
	if (!CvInfoBase::read(pXML))
		return false;

	//int j;
	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"WaterMaterial"))
	{
		if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"MaterialColors"))
		{
			if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"DiffuseMaterialColor"))
			{
				pXML->GetChildXmlValByName( &m_kMaterialDiffuse.r, "r");
				pXML->GetChildXmlValByName( &m_kMaterialDiffuse.g, "g");
				pXML->GetChildXmlValByName( &m_kMaterialDiffuse.b, "b");
				gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
			}
			if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"SpecularMaterialColor"))
			{
				pXML->GetChildXmlValByName( &m_kMaterialSpecular.r, "r");
				pXML->GetChildXmlValByName( &m_kMaterialSpecular.g, "g");
				pXML->GetChildXmlValByName( &m_kMaterialSpecular.b, "b");
				gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
			}
			if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"EmmisiveMaterialColor"))
			{
				pXML->GetChildXmlValByName( &m_kMaterialEmmisive.r, "r");
				pXML->GetChildXmlValByName( &m_kMaterialEmmisive.g, "g");
				pXML->GetChildXmlValByName( &m_kMaterialEmmisive.b, "b");
				gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
			}
			gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
		}

		pXML->GetChildXmlValByName( &m_fMaterialAlpha, "MaterialAlpha");

		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"WaterTextures"))
	{
		if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"WaterBaseTexture"))
		{
			pXML->GetChildXmlValByName( szTextVal, "TextureFile");
			setBaseTexture(szTextVal);

			pXML->GetChildXmlValByName( &m_BaseTextureScale, "TextureScaling");
			pXML->GetChildXmlValByName( &m_fURate, "URate");
			pXML->GetChildXmlValByName( &m_fVRate, "VRate");

			gDLL->getXMLIFace()->SetToParent(pXML->GetXML());

		}
		if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"WaterTransitionTexture"))
		{
			pXML->GetChildXmlValByName( szTextVal, "TextureFile");
			setTransitionTexture(szTextVal);
		}
		
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}

	gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	return true;
}
//------------------------------------------------------------------------------------------------------
