//  $Header: //depot/main/Civilization4/CvGameCoreDLL/CvStructs.cpp#3 $
//------------------------------------------------------------------------------------------------
//
//  ***************** CIV4 GAME ENGINE   ********************
//
//! \file		CvStructs.cpp
//! \author		Multiple
//! \brief		Implementation of basic Civ4 structures
//
//------------------------------------------------------------------------------------------------
//  Copyright (c) 2005 Firaxis Games, Inc. All rights reserved.
//------------------------------------------------------------------------------------------------

#include "CvGameCoreDLL.h"
//#include "CvStructs.h"

CvBattleRound::CvBattleRound() :
	iWaveSize(0),
	bRangedRound( false) 
{
	iKilled[BDU_ATTACKER] = iKilled[BDU_DEFENDER] = 0;
	iAlive[BDU_ATTACKER] = iAlive[BDU_DEFENDER] = 0;
}

bool CvBattleRound::isValid() const
{
	bool bValid = true;

	// Valid if no more than the wave size was killed, and didn't kill more attackers than were defenders or vv.
	bValid &= (iKilled[BDU_ATTACKER] + iKilled[BDU_DEFENDER] <= iWaveSize);
	bValid &= (iKilled[BDU_ATTACKER] <= iAlive[BDU_DEFENDER]);
	bValid &= (iKilled[BDU_DEFENDER] <= iAlive[BDU_ATTACKER]);
	return bValid;
}

//------------------------------------------------------------------------------------------------
// FUNCTION:    CvMissionDefinition::CvMissionDefinition
//! \brief      Default constructor.
//------------------------------------------------------------------------------------------------
CvMissionDefinition::CvMissionDefinition() :
	fMissionTime(0.0f),
	eMissionType(NO_MISSION),
	pkPlot(NULL)
{
}

//------------------------------------------------------------------------------------------------
// FUNCTION:    CvBattleDefinition::CvBattleDefinition
//! \brief      Constructor.
//------------------------------------------------------------------------------------------------
CvBattleDefinition::CvBattleDefinition() : 
	bAdvanceSquare( false ), 
	CvMissionDefinition()
{
	fMissionTime = 0.0f;
	eMissionType = MISSION_BEGIN_COMBAT;
	for ( int i = 0; i < BDU_COUNT; i++ )
	{
		pkUnits[i] = NULL;
		for ( int j = 0; j < BDT_COUNT; j++ )
		{
			iDamage[i][j] = 0;
		}
		iFirstStrikes[i] = 0;
	}
}

//------------------------------------------------------------------------------------------------
// FUNCTION:    CvBattleDefinition::CvBattleDefinition
//! \brief      Copy constructor
//! \param      kCopy The object to copy
//------------------------------------------------------------------------------------------------
CvBattleDefinition::CvBattleDefinition( const CvBattleDefinition & kCopy ) :
	bAdvanceSquare( kCopy.bAdvanceSquare )
{
	fMissionTime = kCopy.fMissionTime;
	eMissionType = MISSION_BEGIN_COMBAT;

	for ( int i = 0; i < BDU_COUNT; i++ )
	{
		pkUnits[i] = kCopy.pkUnits[i];
		for ( int j = 0; j < BDT_COUNT; j++ )
		{
			iDamage[i][j] = kCopy.iDamage[i][j];
		}
		iFirstStrikes[i] = kCopy.iFirstStrikes[i];
	}

	vctBattlePlan.assign( kCopy.vctBattlePlan.begin(), kCopy.vctBattlePlan.end() );
}

//------------------------------------------------------------------------------------------------
// FUNCTION:    CvAirMissionDefinition::CvAirMissionDefinition
//! \brief      Constructor
//------------------------------------------------------------------------------------------------
CvAirMissionDefinition::CvAirMissionDefinition() :
	CvMissionDefinition()
{
	fMissionTime = 0.0f;
	eMissionType = MISSION_AIRPATROL;
}

//------------------------------------------------------------------------------------------------
// FUNCTION:    CvAirMissionDefinition::CvAirMissionDefinition
//! \brief      Copy constructor
//! \param      kCopy The object to copy
//------------------------------------------------------------------------------------------------
CvAirMissionDefinition::CvAirMissionDefinition( const CvAirMissionDefinition & kCopy )
{
	fMissionTime = kCopy.fMissionTime;
	eMissionType = kCopy.eMissionType;
	pkPlot = kCopy.pkPlot;

	int i;
	for ( i = 0; i < BDU_COUNT; i++ )
	{
		iDamage[i] = kCopy.iDamage[i];
		pkUnits[i] = kCopy.pkUnits[i];
	}
}
