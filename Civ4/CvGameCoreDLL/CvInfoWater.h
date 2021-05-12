//  $Header:
//------------------------------------------------------------------------------------------------
//
//  FILE:    CvInfosWater.h
//
//  AUTHOR:	tomw
//					
//
//  PURPOSE: All Civ4 info classes and the base class for them
//
//------------------------------------------------------------------------------------------------
//  Copyright (c) 2003 Firaxis Games, Inc. All rights reserved.
//------------------------------------------------------------------------------------------------
#ifndef CV_WATERINFO_H
#define CV_WATERINFO_H

#include "CvInfos.h"

#pragma warning( disable: 4251 )		// needs to have dll-interface to be used by clients of class

class CvXMLLoadUtility;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  class : CvWaterPlaneInfo
//
//  DESC:   
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvWaterPlaneInfo :public CvInfoBase
{
public:

	DllExport CvWaterPlaneInfo();
	DllExport virtual ~CvWaterPlaneInfo();

	DllExport NiColor	getMaterialDiffuse() const;			// The water plane's material diffuse color
	DllExport NiColor	getMaterialSpecular() const;		// The water plane's material specular color
	DllExport NiColor	getMaterialEmmisive() const;		// The water plane's material emmisive color
	DllExport float getMaterialAlpha() const;				// The water plane's material alpha

	DllExport float getTextureScaling() const;				// The water plane's texture scale
	DllExport float getTextureScrollRateU() const;			// The water plane's texture scroll rate in U
	DllExport float getTextureScrollRateV() const;			// The water plane's texture scroll rate in V

	DllExport const TCHAR * getBaseTexture() const;
	DllExport void setBaseTexture(const TCHAR* szVal);		// The filename of the base texture

	DllExport const TCHAR *getTransitionTexture() const;
	DllExport void setTransitionTexture(const TCHAR* szVal); // The transition texture for fading ocean into land

	DllExport bool read(CvXMLLoadUtility*);

protected:

	NiColor	m_kMaterialDiffuse;		// The water plane's material diffuse color
	NiColor	m_kMaterialSpecular;	// The water plane's material specular color
	NiColor	m_kMaterialEmmisive;	// The water plane's material emmisive color
	float m_fMaterialAlpha;			// The water plane's material alpha

	CvString m_szBaseTexture;		// The filename of the base texture
	CvString m_szTransitionTexture;	// The filename of the transition texture

	float m_BaseTextureScale;		// Texture scaling
	float m_fURate;					// Texture scroll rate
	float m_fVRate;
};



#endif
