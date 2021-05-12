// extraSaveData.h

/* 
The purpose of this class is to allow us to save additional data without
breaking save file compatibility.

On read: we transform data that is stored in the found values of the map 
into data we use.

On write: we transform in the opposite direction

Found values are always positive or -1. 
So, we will always use negative numbers, and never use -1.

Found values are shorts, this will give us 15 bits to use, we will always set bit 15, bits 0-14 all set (0x4FFF) is not valid.

The order that classes are read is the following:
game
map (which reads plots)
teams
players (which read cities)

changes with this class:

on read:
map will setup this class for read: (setupForRead)
 - filling the data from the found values
 - setting all found values to -1 that we used (cleanPlotValues)
map will read from this class (getSaveData)
teams will read from this class (getSaveData)
players will read from this class (getSaveData)
cities will read from this class (getSaveData)

on CvMap::doTurn:
this class marked invalid, we should be done reading by then (invalidate)

on write:
map will setup this class for write: (setupForWrite)
 - this class will call CvMap::writeExtraSaveData which calls back to CvExtraSaveData::setSaveData
 - this class will call CvTeamAI::writeExtraSaveData which calls back to CvExtraSaveData::setSaveData
 - this class will call CvPlayerAI::writeExtraSaveData which calls back to CvExtraSaveData::setSaveData
 - this class will call CvCityAI::writeExtraSaveData which calls back to CvExtraSaveData::setSaveData
map will tell this class to push its values to the found values (writeToPlots)
map will write out the plots (CvPlot::write)
map will tell this class to restore the found values (restorePlotValues)
map will mark invalid (invalidate)
*/

#ifndef CIV4_EXTRA_SAVE_DATA_H
#define CIV4_EXTRA_SAVE_DATA_H

#include "CvDefines.h"
#include "FAssert.h"

/*
	Save file format:
		CvExtraSaveDataHeader
	
		repeat
		{
			CvExtraSaveDataNodeHeader
				CvExtraSaveDataNode * CvExtraSaveDataNodeHeader->iCount
		}
		until CvExtraSaveDataHeader->iSize exhausted

	if we find invalid value before finished, then everything is discarded.
*/

// you can change these defines, to add more data to the save
#define EXTRASAVEDATA_MAX_MAP_VALUES			3
#define EXTRASAVEDATA_MAX_TEAM_VALUES			3
#define EXTRASAVEDATA_MAX_PLAYER_VALUES			3
#define EXTRASAVEDATA_MAX_CITY_VALUES			3

// do not change these enums, you can add, but do not change, will break saves
enum ExtraSaveDataType
{
	EXTRASAVEDATA_NONE = 0,
	EXTRASAVEDATA_GAME = 1,		// currently not used, no game data saved
	EXTRASAVEDATA_MAP = 2,
	EXTRASAVEDATA_TEAM = 3,
	EXTRASAVEDATA_PLAYER = 4,
	EXTRASAVEDATA_CITY = 5,
};

// this is hardcoded, so if MAX_PLAYERS is changed, it does not break our save compatibility
#define EXTRASAVEDATA_MAX_PLAYERS			19

// note all sizes are count of ints

struct CvExtraSaveDataHeader
{
	short			iValidCheck1;
	short			iValidCheck2;
	short			iValidCheck3;
	short			iSize;
	short			iHeaderSize;
	short			iNodeHeaderSize;
	short			iNodeSize;
};

struct CvExtraSaveDataNodeHeader
{
	short			eType;
	short			iID;
	short			iOwner;
	short			iCount;
};

struct CvExtraSaveDataNode
{
	short			iValue;
};

// constants
const short kExtraSaveData_validCheck1 = ' B';
const short kExtraSaveData_validCheck2 = ' A';
const short kExtraSaveData_validCheck3 = ' I';

const unsigned short kBit15 = (1 << 15);

class CvMap;

struct CvExtraCityData
{
	short					iCityID;
	short					iNodeCount;
	CvExtraSaveDataNode		aNodes[EXTRASAVEDATA_MAX_CITY_VALUES];
};

class CvExtraSaveData;

class CvExtraCityDataArray
{
public:
	CvExtraCityDataArray();
	virtual ~CvExtraCityDataArray();
	
	bool initOwner(short iOwner, short iSize = -1);

	CvExtraSaveDataNode* getCityExtraDataNode(short iCityID, short iOwner, short iNodeIndex, bool bCreate = false);

protected:

	bool				increaseAllocation(short iOwner, short iSize = -1);
	CvExtraCityData*	findCityExtraData(short iCityID, short iOwner);
	CvExtraCityData*	addCityExtraData(short iCityID, short iOwner);


	short							m_cityDataAllocation[MAX_PLAYERS];
	short							m_cityDataCount[MAX_PLAYERS];
	CvExtraCityData*				m_cityData[MAX_PLAYERS];

	friend CvExtraSaveData;
};

class CvExtraSaveData
{

public:
	
	// called by CvMap:
	CvExtraSaveData();
	virtual ~CvExtraSaveData();
	
	bool setupForRead(CvMap& kMap);			// sets values from the plots, returns success
	bool cleanPlotValues(CvMap& kMap);		// cleans the plot values, all used values set to -1, returns whether any were cleaned

	bool setupForWrite(CvMap& kMap);		// sets values from the classes, returns success
	bool writeToPlots(CvMap& kMap);			// writes values to the plots, returns success

	bool savePlotValues(CvMap& kMap);		// save the plot values, returns success (called by setupForWrite)
	bool restorePlotValues(CvMap& kMap);	// restores the plot values, returns success

	bool invalidate(void);					// called when read or write finished

	// called by CvMap, CvTeamAI, CvPlayerAI
	bool getSaveData(ExtraSaveDataType eType, int iID, int iOwner, int iIndex, int& iValue);
	bool setSaveData(ExtraSaveDataType eType, int iID, int iOwner, int iIndex, int iValue);


protected:
	// convert index to plot and player indicies
	// we use a constant defined here, so we do not break save game compatibility if MAX_PLAYERS changes
	inline void indexToPlotIndexPlayerIndex(int iIndex, int& iPlotIndex, int& iPlayerIndex) const
	{
		FAssert(EXTRASAVEDATA_MAX_PLAYERS >= MAX_PLAYERS);
		
		iPlotIndex = iIndex / EXTRASAVEDATA_MAX_PLAYERS;
		iPlayerIndex = iIndex % EXTRASAVEDATA_MAX_PLAYERS;
	}
	
	// we encode values by setting the high bit, but we need to ensure we never get -1, so 0x7FFF (MAX_SHORT) is invalid
	inline bool validValueToEncode(short iValue) const
	{
		// valid if bit 15 clear and not MAX_SHORT
		return (!(iValue & kBit15) && iValue != MAX_SHORT);
	}
	inline bool validFoundValueToDecode(short iValue) const
	{
		// valid if bit 15 set and not -1
		return ((iValue & kBit15) && iValue != -1);
	}
	inline short encodeToFoundValue(short iValue) const
	{
		FAssert(validValueToEncode(iValue));
		
		// set bit 15
		return (iValue | kBit15);
	}
	inline short decodeFromFoundValue(short iValue) const
	{
		FAssert(validFoundValueToDecode(iValue));
		
		// clear bit 15
		return (iValue & ~kBit15);
	}
	
	// high level push/pop functions
	bool pushHeader(CvMap& kMap, int iIndex, CvExtraSaveDataHeader& header);	// iIndex should be 0 on input
	bool popHeader(CvMap& kMap, int iIndex, CvExtraSaveDataHeader& header);	// iIndex should be 0 on input

	bool pushNodeHeader(CvMap& kMap, int iIndex, CvExtraSaveDataNodeHeader& nodeHeader);
	bool popNodeHeader(CvMap& kMap, int iIndex, CvExtraSaveDataNodeHeader& nodeHeader);

	bool pushNode(CvMap& kMap, int iIndex, CvExtraSaveDataNode& node);
	bool popNode(CvMap& kMap, int iIndex, CvExtraSaveDataNode& node);

	// these are actually used to set/get the found values
	bool pushToFoundValues(CvMap& kMap, int iIndex, short iValue);
	bool popFromFoundValues(CvMap& kMap, int iIndex, short& iValue);
	
	// total number of shorts to hold all our data
	// can only call this from write, it is not valid on read, we cannot know number of cities yet
	int calculateMaxTotalSize();

	// we hardcode knowledge about the types actually used
	// we could change this implementation, but this is going to be a lot faster than dynamically allocating every save
	// changing this implementation will not affect save file compatibility

	// only 1 map
	short							m_mapDataCount;
	CvExtraSaveDataNode				m_mapData[EXTRASAVEDATA_MAX_MAP_VALUES];
	
	// MAX_TEAMS number of teams
	short							m_teamDataCount[MAX_TEAMS];
	CvExtraSaveDataNode				m_teamData[MAX_TEAMS][EXTRASAVEDATA_MAX_TEAM_VALUES];
	
	// MAX_PLAYERS number of players
	short							m_playerDataCount[MAX_PLAYERS];
	CvExtraSaveDataNode				m_playerData[MAX_PLAYERS][EXTRASAVEDATA_MAX_PLAYER_VALUES];
	
	
	// city data array (arrays of cities for each player)
	CvExtraCityDataArray			m_cityDataArray;

	// all the old values
	int		m_aiOldFoundValuesSize;
	short*	m_aiOldFoundValues[MAX_PLAYERS];

	// is the data valid, both of these will not be true at the same time
	// m_bValidForRead is set on CvMap::read by calling setupForRead and cleared on CvMap::doTurn by calling invalidate
	// m_bValidForWrite is set and cleared inside CvMap::write by calling setupForWrite and invalidate
	bool m_bValidForRead;
	bool m_bValidForWrite;

	// m_bValidForRestore is set from writeToPlots and cleared from invalidate
	bool m_bValidForRestore;
};

#endif
