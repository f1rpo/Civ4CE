// extraSaveData.cpp

#include "CvGameCoreDLL.h"
#include "CvExtraSaveData.h"
#include "CvMap.h"
#include "CvPlot.h"
#include "CvTeamAI.h"
#include "CvPlayerAI.h"
#include "FFreeListTrashArray.h"

// Public Functions...


CvExtraCityDataArray::CvExtraCityDataArray()
{
	for (int iOwner = 0; iOwner < MAX_PLAYERS; iOwner++)
	{
		m_cityDataAllocation[iOwner] = 0;
		m_cityDataCount[iOwner] = 0;
		m_cityData[iOwner] = NULL;
	}
}

CvExtraCityDataArray::~CvExtraCityDataArray()
{
	for (int iOwner = 0; iOwner < MAX_PLAYERS; iOwner++)
	{
		SAFE_DELETE_ARRAY(m_cityData[iOwner]);
	}
}

bool CvExtraCityDataArray::initOwner(short iOwner, short iSize)
{
	// do we already have a big enough pool allocated?
	if (m_cityDataAllocation[iOwner] >= iSize)
	{
		return true;
	}
	
	// otherwise, increase our allocation
	return increaseAllocation(iOwner, iSize);
}


CvExtraSaveDataNode* CvExtraCityDataArray::getCityExtraDataNode(short iCityID, short iOwner, short iNodeIndex, bool bCreate)
{
	FAssertMsg(iOwner >= 0 && iOwner < MAX_PLAYERS, "city owner out of range");
	FAssertMsg(iNodeIndex >= 0 && iNodeIndex < EXTRASAVEDATA_MAX_CITY_VALUES, "city value index out of range");

	if (!(iOwner >= 0 && iOwner < MAX_PLAYERS) || !(iNodeIndex >= 0 && iNodeIndex < EXTRASAVEDATA_MAX_CITY_VALUES))
	{
		return NULL;
	}
	
	CvExtraSaveDataNode* saveDataNode = NULL;
		
	// does it already exist?
	CvExtraCityData* pExtraCityData = findCityExtraData(iCityID, iOwner);
	
	// do we need to add it?
	if (pExtraCityData == NULL && bCreate)
	{
		pExtraCityData = addCityExtraData(iCityID, iOwner);
	}

	// do we now have a valid item
	if (pExtraCityData != NULL)
	{
		// do we need to increment the node count?
		if (bCreate && iNodeIndex >= pExtraCityData->iNodeCount)
		{
			// zero fill the extras
			while (pExtraCityData->iNodeCount < iNodeIndex)
			{
				pExtraCityData->aNodes[pExtraCityData->iNodeCount++].iValue = 0;
			}
			
			pExtraCityData->iNodeCount++;
			FAssertMsg(pExtraCityData->iNodeCount == iNodeIndex + 1, "node count out of sync");
		}
		
		// is the index in range?
		FAssertMsg(iNodeIndex < pExtraCityData->iNodeCount, "city value index out of range");
		if (iNodeIndex < pExtraCityData->iNodeCount)
		{
			// get the node
			saveDataNode = &pExtraCityData->aNodes[iNodeIndex];
		}
	}

	return saveDataNode;
}

bool CvExtraCityDataArray::increaseAllocation(short iOwner, short iSize)
{
	FAssertMsg(iOwner >= 0 && iOwner < MAX_PLAYERS, "city owner out of range");

	bool bSuccess = false;
	
	int iOldAllocation = m_cityDataAllocation[iOwner];
	// double allocation (min allocation 4)
	int iNewAllocation = (iOldAllocation > 2) ? (iOldAllocation * 2) : 4;
	// if we asked for a size even larger than that, use it
	if (iSize > iNewAllocation)
	{
		iNewAllocation = iSize;
	}
	
	// allocate new
	CvExtraCityData* aExtraCityData = new CvExtraCityData[iNewAllocation];
	
	FAssertMsg(aExtraCityData != NULL, "failed to allocate city data array");
	if (aExtraCityData != NULL)
	{
		// did we already have an allocation?
		if (iOldAllocation > 0)
		{
			// copy old data
			for (int iI = 0; iI < m_cityDataCount[iOwner]; iI++)
			{
				aExtraCityData[iI] = m_cityData[iOwner][iI];
			}

			// dispose of old allocation
			SAFE_DELETE_ARRAY(m_cityData[iOwner]);
		}
		
		FAssertMsg(m_cityData[iOwner] == NULL, "leaking memory");

		// set to new size and pointer
		m_cityDataAllocation[iOwner] = iNewAllocation;
		m_cityData[iOwner] = aExtraCityData;

		bSuccess = true;
	}

	return bSuccess;
}

CvExtraCityData* CvExtraCityDataArray::findCityExtraData(short iCityID, short iOwner)
{
	FAssertMsg(iOwner >= 0 && iOwner < MAX_PLAYERS, "city owner out of range");
	
	if (!(iOwner >= 0 && iOwner < MAX_PLAYERS))
	{
		return NULL;
	}
	
	CvExtraCityData* pExtraCityData = NULL;
	
	// find the city ID already from our list
	for (int iCityIndex = 0; pExtraCityData == NULL && iCityIndex < m_cityDataCount[iOwner]; iCityIndex++)
	{
		FAssertMsg(m_cityData[iOwner][iCityIndex].iCityID != -1, "city ID invalid (-1)");
		if (m_cityData[iOwner][iCityIndex].iCityID == iCityID)
		{
			pExtraCityData = &m_cityData[iOwner][iCityIndex];
		}
	}
	
	return pExtraCityData;
}

CvExtraCityData* CvExtraCityDataArray::addCityExtraData(short iCityID, short iOwner)
{
	FAssertMsg(iOwner >= 0 && iOwner < MAX_PLAYERS, "city owner out of range");
	FAssertMsg(findCityExtraData(iCityID, iOwner) == NULL, "adding city which already exists");
	
	CvExtraCityData* pExtraCityData = NULL;
	
	// the new slot (at the end)
	int iCityIndex = m_cityDataCount[iOwner];

	// do we need to re-allocate?
	bool bSuccess = false;
	if (iCityIndex >= m_cityDataAllocation[iOwner])
	{
		bSuccess = increaseAllocation(iOwner, iCityIndex);
	}

	// do we have room for the new value? (true unless memory allocation failed)
	FAssertMsg(iCityIndex < m_cityDataAllocation[iOwner], "unable to add city data");
	if (iCityIndex < m_cityDataAllocation[iOwner])
	{
		// expand count
		FAssertMsg (iCityIndex == m_cityDataCount[iOwner], "city index out of order")
		m_cityDataCount[iOwner] = iCityIndex + 1;
		
		// remember pointer to next extraCityData
		pExtraCityData = &m_cityData[iOwner][iCityIndex];

		// initialize ID and count
		pExtraCityData->iCityID = iCityID;
		pExtraCityData->iNodeCount = 0;
	}

	return pExtraCityData;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CvExtraSaveData::CvExtraSaveData()
{
	m_bValidForRead = false;
	m_bValidForWrite = false;
	m_bValidForRestore = false;

	m_aiOldFoundValuesSize = 0;
	for (int iJ = 0; iJ < MAX_PLAYERS; iJ++)
	{
		m_aiOldFoundValues[iJ] = NULL;
	}
}

CvExtraSaveData::~CvExtraSaveData()
{
	for (int iJ = 0; iJ < MAX_PLAYERS; iJ++)
	{
		SAFE_DELETE_ARRAY(m_aiOldFoundValues[iJ]);
	}
}

// sets values from the plots, returns success
bool CvExtraSaveData::setupForRead(CvMap& kMap)
{
	// zero map data
	m_mapDataCount = 0;

	// zero team data for each team
	for (int iI = 0; iI < MAX_TEAMS; iI++)
	{
		m_teamDataCount[0] = 0;
	}

	// zero player data for each player
	for (int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		m_playerDataCount[0] = 0;
	}

	bool bValid;
	
	// read in the header
	int iIndex = 0;
	CvExtraSaveDataHeader header;
	bValid = popHeader(kMap, iIndex, header);
	
	// only used for later assert
	bool bHeaderExists = bValid;

	// confirm ValidCheck
	if (bValid)
	{
		if (header.iValidCheck1 != kExtraSaveData_validCheck1 || header.iValidCheck2 != kExtraSaveData_validCheck2 ||
			header.iValidCheck3 != kExtraSaveData_validCheck3)
		{
			bValid = false;
		}
	}
	
	// confirm size, larger than we expect is alright, smaller is not currently (all sizes are count of ints)
	// should update this if the save file format is increased, to deal with smaller sizes if we can handle them
	if (bValid)
	{
		if (header.iHeaderSize < (sizeof(CvExtraSaveDataHeader) / sizeof(short)))
		{
			bValid = false;
		}
		else if (header.iNodeHeaderSize < (sizeof(CvExtraSaveDataNodeHeader) / sizeof(short)))
		{
			bValid = false;
		}
		else if (header.iNodeSize < (sizeof(CvExtraSaveDataNode) / sizeof(short)))
		{
			bValid = false;
		}
	}
	
	FAssertMsg(bHeaderExists == bValid, "extra save info header exists but was invalid");

	// valid header, update iIndex
	if (bValid)
	{
		iIndex += header.iHeaderSize;
	}
	
	// if a valid header, read in the nodes
	bool bFinished = false;
	int iMapIndex = 0;
	int iTeamIndex = 0;
	int iPlayerIndex = 0;
	while (bValid && !bFinished)
	{
		CvExtraSaveDataNodeHeader nodeHeader;
		bValid = popNodeHeader(kMap, iIndex, nodeHeader);
		iIndex += header.iNodeHeaderSize;

		FAssertMsg(bValid, "extra save info exists but was invalid");
		
		if (bValid)
		{
			switch (nodeHeader.eType)
			{
			case EXTRASAVEDATA_MAP:
				FAssertMsg(iMapIndex == 0, "more than one map header in extra saved data");
				FAssertMsg(nodeHeader.iID == 0, "map header has non-zero owner in extra saved data");
				FAssertMsg(nodeHeader.iCount <= EXTRASAVEDATA_MAX_MAP_VALUES, "more map values than max in extra saved data");
				
				if (iMapIndex == 0 && nodeHeader.iID == 0)
				{
					// if there are more stored values than we can use, only use the number we can use
					// we assume here that extra values are for newer DLLs than us, and that order was not changed
					if (nodeHeader.iCount > EXTRASAVEDATA_MAX_MAP_VALUES)
					{
						nodeHeader.iCount = EXTRASAVEDATA_MAX_MAP_VALUES;
					}
					
					// remember count
					m_mapDataCount = nodeHeader.iCount;

					// loop over each node
					for (int iI = 0; bValid && iI < nodeHeader.iCount; iI++)
					{
						CvExtraSaveDataNode node;
						bValid = popNode(kMap, iIndex, node);
						iIndex += header.iNodeSize;
							
						// remember data
						if (bValid)
						{
							m_mapData[iI] = node;
						}
					}
				}

				iMapIndex++;
				break;

			case EXTRASAVEDATA_TEAM:
				FAssertMsg(iTeamIndex < MAX_TEAMS, "too many teams in extra saved data");
				FAssertMsg(nodeHeader.iID >= 0 && nodeHeader.iID < MAX_TEAMS, "team header has team out of range in extra saved data");
				FAssertMsg(nodeHeader.iCount <= EXTRASAVEDATA_MAX_TEAM_VALUES, "more team values than max in extra saved data");
				
				if (iTeamIndex < MAX_TEAMS && nodeHeader.iID >= 0 && nodeHeader.iID < MAX_TEAMS)
				{
					// if there are more stored values than we can use, only use the number we can use
					// we assume here that extra values are for newer DLLs than us, and that order was not changed
					if (nodeHeader.iCount > EXTRASAVEDATA_MAX_TEAM_VALUES)
					{
						nodeHeader.iCount = EXTRASAVEDATA_MAX_TEAM_VALUES;
					}
					
					// remember our count
					m_teamDataCount[nodeHeader.iID] = nodeHeader.iCount;

					// loop over each node
					for (int iI = 0; bValid && iI < nodeHeader.iCount; iI++)
					{
						CvExtraSaveDataNode node;
						bValid = popNode(kMap, iIndex, node);
						iIndex += header.iNodeSize;
							
						// remember data
						if (bValid)
						{
							m_teamData[nodeHeader.iID][iI] = node;
						}
					}
				}

				iTeamIndex++;
				break;

			case EXTRASAVEDATA_PLAYER:
				FAssertMsg(iPlayerIndex < MAX_PLAYERS, "too many players in extra saved data");
				FAssertMsg(nodeHeader.iID >= 0 && nodeHeader.iID < MAX_PLAYERS, "player header has player out of range in extra saved data");
				FAssertMsg(nodeHeader.iCount <= EXTRASAVEDATA_MAX_PLAYER_VALUES, "more player values than max in extra saved data");
				
				if (iPlayerIndex < MAX_PLAYERS && nodeHeader.iID >= 0 && nodeHeader.iID < MAX_PLAYERS)
				{
					// if there are more stored values than we can use, only use the number we can use
					// we assume here that extra values are for newer DLLs than us, and that order was not changed
					if (nodeHeader.iCount > EXTRASAVEDATA_MAX_PLAYER_VALUES)
					{
						nodeHeader.iCount = EXTRASAVEDATA_MAX_PLAYER_VALUES;
					}
					
					// remember our count
					m_playerDataCount[nodeHeader.iID] = nodeHeader.iCount;

					// loop over each node
					for (int iI = 0; bValid && iI < nodeHeader.iCount; iI++)
					{
						CvExtraSaveDataNode node;
						bValid = popNode(kMap, iIndex, node);
						iIndex += header.iNodeSize;
							
						// remember data
						if (bValid)
						{
							m_playerData[nodeHeader.iID][iI] = node;
						}
					}
				}

				iPlayerIndex++;
				break;

			case EXTRASAVEDATA_CITY:
				FAssertMsg(nodeHeader.iOwner >= 0 && nodeHeader.iOwner < MAX_PLAYERS, "city header has player out of range in extra saved data");
				FAssertMsg(nodeHeader.iCount <= EXTRASAVEDATA_MAX_CITY_VALUES, "more city values than max in extra saved data");

				// is player ID in range?
				if (nodeHeader.iOwner >= 0 && nodeHeader.iOwner < MAX_PLAYERS)
				{
					// if there are more stored values than we can use, only use the number we can use
					// we assume here that extra values are for newer DLLs than us, and that order was not changed
					if (nodeHeader.iCount > EXTRASAVEDATA_MAX_CITY_VALUES)
					{
						nodeHeader.iCount = EXTRASAVEDATA_MAX_CITY_VALUES;
					}
					
					// loop over each node
					for (int iNodeIndex = 0; bValid && iNodeIndex < nodeHeader.iCount; iNodeIndex++)
					{
						// get the node from our city array, creating the entry if necessary
						CvExtraSaveDataNode* pNode = m_cityDataArray.getCityExtraDataNode(nodeHeader.iID, nodeHeader.iOwner, iNodeIndex, /*bCreate*/ true);
						if (pNode == NULL)
						{
							bValid = false;
						}
						
						// pop the data into that node
						if (bValid)
						{
							bValid = popNode(kMap, iIndex, *pNode);
							iIndex += header.iNodeSize;
						}
					}
				}
				break;

			default:
				FAssertMsg(false, "unknown type in extra saved data");
				break;
			}
		}
		
		// if we have read in everything, we are finished
		if (iIndex == header.iSize)
		{
			bFinished = true;
		}
	}

	m_bValidForRead = bValid;

	return m_bValidForRead;
}

// cleans the plot values, all used values set to -1, returns whether any were cleaned
bool CvExtraSaveData::cleanPlotValues(CvMap& kMap)
{
	// clean values, anything less than -1 gets set to -1
	bool bCleaned = false;
	for (int iI = 0; iI < kMap.numPlotsINLINE(); iI++)
	{		
		CvPlot* pPlot = kMap.plotByIndexINLINE(iI);
		FAssertMsg(pPlot != NULL, "NULL plot by index!");

		if (pPlot != NULL)
		{
			for (int iJ = 0; iJ < MAX_PLAYERS; iJ++)
			{
				if (pPlot->m_aiFoundValue[iJ] < -1)
				{
					pPlot->m_aiFoundValue[iJ] = -1;
					bCleaned = true;
				}
			}
		}
	}
	
	return bCleaned;
}

// sets values from the classes, returns success
bool CvExtraSaveData::setupForWrite(CvMap& kMap)
{
	// is there room to store our info?
	int iMaxTotalSize = calculateMaxTotalSize();
	if (kMap.numPlotsINLINE() * MAX_PLAYERS >= iMaxTotalSize)
	{
		// as long as we have room, we will write out our data (must set before calling writeExtraSaveData)
		m_bValidForWrite = true;

		// zero map data
		m_mapDataCount = 0;
		// have map write out its data
		kMap.writeExtraSaveData(*this);

		// teams
		for (int iTeamIndex = 0; iTeamIndex < MAX_TEAMS; iTeamIndex++)
		{
			// zero team data for team
			m_teamDataCount[iTeamIndex] = 0;
			// have team write out its data
			GET_TEAM((TeamTypes)iTeamIndex).writeExtraSaveData(*this);
		}
		
		// players
		for (int iPlayerIndex = 0; iPlayerIndex < MAX_PLAYERS; iPlayerIndex++)
		{
			// zero player data for team
			m_playerDataCount[iPlayerIndex] = 0;
			// have player write out its data
			GET_PLAYER((PlayerTypes)iPlayerIndex).writeExtraSaveData(*this);
		}

		// cities
		for (int iPlayerIndex = 0; iPlayerIndex < MAX_PLAYERS; iPlayerIndex++)
		{
			CvPlayerAI& kPlayer = GET_PLAYER((PlayerTypes)iPlayerIndex);
			
			// if we successfully initialize our city data array for this player
			if (m_cityDataArray.initOwner(iPlayerIndex, kPlayer.getNumCities()))
			{
				// ask each city to save its extra data
				int iLoop;
				for (CvCity* pLoopCity = kPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kPlayer.nextCity(&iLoop))
				{
					CvCityAI* pLoopCityAI = static_cast<CvCityAI*>(pLoopCity);
					pLoopCityAI->writeExtraSaveData(*this);
				}
			}
		}
	}
	
	FAssertMsg(m_bValidForWrite, "unable to write extra data");

	return m_bValidForWrite;
}

// writes values to the plots, returns success
bool CvExtraSaveData::writeToPlots(CvMap& kMap)
{
	FAssertMsg(m_bValidForWrite, "invalid to write extra data");
	
	if (!m_bValidForWrite)
	{
		return false;
	}
	
	// start out valid
	bool bValid = true;

	// save the old plot found values
	savePlotValues(kMap);

	// set up the header	
	CvExtraSaveDataHeader header;
	header.iValidCheck1 = kExtraSaveData_validCheck1;
	header.iValidCheck2 = kExtraSaveData_validCheck2;
	header.iValidCheck3 = kExtraSaveData_validCheck3;
	header.iHeaderSize = sizeof(CvExtraSaveDataHeader) / sizeof(short);
	header.iNodeHeaderSize = sizeof(CvExtraSaveDataNodeHeader) / sizeof(short);
	header.iNodeSize = sizeof(CvExtraSaveDataNode) / sizeof(short);
	header.iSize = header.iHeaderSize; // note we will re-write out the header with the correct size at the end
	int iIndex = 0;
	bValid = pushHeader(kMap, iIndex, header);
	iIndex += header.iHeaderSize;
	
	// used for all
	CvExtraSaveDataNodeHeader nodeHeader;

	// write out the map data
	if (bValid && m_mapDataCount > 0)
	{
		nodeHeader.iID = 0;
		nodeHeader.iOwner = 0;
		nodeHeader.eType = EXTRASAVEDATA_MAP;
		nodeHeader.iCount = m_mapDataCount;
		bValid = pushNodeHeader(kMap, iIndex, nodeHeader);
		iIndex += header.iNodeHeaderSize;
		
		// write out the nodes
		for (int iI = 0; bValid && iI < nodeHeader.iCount; iI++)
		{
			bValid = pushNode(kMap, iIndex, m_mapData[iI]);
			iIndex += header.iNodeSize;
		}
	}

	// write out the teams data
	for (int iTeamIndex = 0; bValid && iTeamIndex < MAX_TEAMS; iTeamIndex++)
	{
		if (m_teamDataCount[iTeamIndex] > 0)
		{
			nodeHeader.iID = iTeamIndex;
			nodeHeader.iOwner = 0;
			nodeHeader.eType = EXTRASAVEDATA_TEAM;
			nodeHeader.iCount = m_teamDataCount[iTeamIndex];
			bValid = pushNodeHeader(kMap, iIndex, nodeHeader);
			iIndex += header.iNodeHeaderSize;
			
			// write out the nodes
			for (int iI = 0; bValid && iI < nodeHeader.iCount; iI++)
			{
				bValid = pushNode(kMap, iIndex, m_teamData[iTeamIndex][iI]);
				iIndex += header.iNodeSize;
			}
		}
	}

	// write out the players data
	for (int iPlayerIndex = 0; bValid && iPlayerIndex < MAX_PLAYERS; iPlayerIndex++)
	{
		if (m_playerDataCount[iPlayerIndex] > 0)
		{
			nodeHeader.iID = iPlayerIndex;
			nodeHeader.iOwner = 0;
			nodeHeader.eType = EXTRASAVEDATA_PLAYER;
			nodeHeader.iCount = m_playerDataCount[iPlayerIndex];
			bValid = pushNodeHeader(kMap, iIndex, nodeHeader);
			iIndex += header.iNodeHeaderSize;
			
			// write out the nodes
			for (int iI = 0; bValid && iI < nodeHeader.iCount; iI++)
			{
				bValid = pushNode(kMap, iIndex, m_playerData[iPlayerIndex][iI]);
				iIndex += header.iNodeSize;
			}
		}
	}

	// write out the cities data
	for (int iPlayerIndex = 0; bValid && iPlayerIndex < MAX_PLAYERS; iPlayerIndex++)
	{
		for (int iCityIndex = 0; iCityIndex < m_cityDataArray.m_cityDataCount[iPlayerIndex]; iCityIndex++)
		{
			CvExtraCityData& kCity = m_cityDataArray.m_cityData[iPlayerIndex][iCityIndex];
			nodeHeader.iID = kCity.iCityID;
			nodeHeader.iOwner = iPlayerIndex;
			nodeHeader.eType = EXTRASAVEDATA_CITY;
			nodeHeader.iCount = kCity.iNodeCount;
			bValid = pushNodeHeader(kMap, iIndex, nodeHeader);
			iIndex += header.iNodeHeaderSize;
			
			// write out the nodes
			for (int iI = 0; bValid && iI < nodeHeader.iCount; iI++)
			{
				bValid = pushNode(kMap, iIndex, kCity.aNodes[iI]);
				iIndex += header.iNodeSize;
			}
		}
	}

	// if everything was valid, re-write out the header with a correct size
	if (bValid)
	{
		header.iSize = iIndex;
		iIndex = 0;
		bValid = pushHeader(kMap, iIndex, header);
	}

	FAssertMsg(bValid, "unable to write extra data");
	
	// if we failed at some point, restore the found values
	if (!bValid)
	{
		restorePlotValues(kMap);
	}

	return bValid;
}

// save the plot values, returns success (called by setupForWrite)
bool CvExtraSaveData::savePlotValues(CvMap& kMap)
{
	bool bValid = true;
	
	int iNumPlots = kMap.numPlotsINLINE();
	int iOldSize = m_aiOldFoundValuesSize;
	int iNewSize = max(iOldSize, iNumPlots);
	
	for (int iPlayerIndex = 0; bValid && iPlayerIndex < MAX_PLAYERS; iPlayerIndex++)
	{
		// if we are too small, reallocate array
		if (iNewSize != iOldSize)
		{
			// dispose of old array if it exists
			SAFE_DELETE_ARRAY(m_aiOldFoundValues[iPlayerIndex]);

			// allocate new array
			m_aiOldFoundValues[iPlayerIndex] = new short[iNewSize];
		}

		// if we got it, then set the values
		if (m_aiOldFoundValues[iPlayerIndex] != NULL)
		{
			for (int iPlotIndex = 0; bValid && iPlotIndex < iNumPlots; iPlotIndex++)
			{		
				CvPlot* pPlot = kMap.plotByIndexINLINE(iPlotIndex);
				FAssertMsg(pPlot != NULL, "NULL plot by index!");

				if (pPlot != NULL)
				{
					m_aiOldFoundValues[iPlayerIndex][iPlotIndex] = pPlot->m_aiFoundValue[iPlayerIndex];
				}
				else 
				{
					m_aiOldFoundValues[iPlayerIndex][iPlotIndex] = -1;
					bValid = false;
				}
			}
		}
		else
		{
			bValid = false;
		}
	}
	
	// if success, reset size
	if (bValid)
	{
		m_aiOldFoundValuesSize = iNewSize;
	}
	
	m_bValidForRestore = bValid;

	return m_bValidForRestore;
}

// restores the plot values, returns success
bool CvExtraSaveData::restorePlotValues(CvMap& kMap)
{
	if (!m_bValidForRestore)
	{
		return false;
	}
	
	// restore values
	bool bError = false;
	for (int iPlayerIndex = 0; iPlayerIndex < MAX_PLAYERS; iPlayerIndex++)
	{
		// if array exists, restore the values
		int iNumPlots = kMap.numPlotsINLINE();

		FAssertMsg(m_aiOldFoundValues[iPlayerIndex] != NULL, "m_bValidForRestore true but we have NULL found values array");
		FAssertMsg(m_aiOldFoundValuesSize >= iNumPlots, "m_aiOldFoundValues array too small");

		if (m_aiOldFoundValues[iPlayerIndex] != NULL)
		{
			for (int iPlotIndex = 0; iPlotIndex < iNumPlots; iPlotIndex++)
			{		
				CvPlot* pPlot = kMap.plotByIndexINLINE(iPlotIndex);
				FAssertMsg(pPlot != NULL, "NULL plot by index!");

				if (pPlot != NULL)
				{
					pPlot->m_aiFoundValue[iPlayerIndex] = m_aiOldFoundValues[iPlayerIndex][iPlotIndex];
				}
				else 
				{
					bError = true;
				}
			}
		}
		else
		{
			bError = true;
		}
	}
	
	return !bError;
}

// called when read or write finished
bool CvExtraSaveData::invalidate(void)
{
	m_bValidForRead = false;
	m_bValidForWrite = false;
	m_bValidForRestore = false;

	return true;
}

// called by CvMap, CvTeamAI, CvPlayerAI, CvCityAI
bool CvExtraSaveData::getSaveData(ExtraSaveDataType eType, int iID, int iOwner, int iIndex, int& iValue)
{
	// if we not valid for read, we probably are using a save file without any info
	//FAssertMsg(m_bValidForRead, "save data not valid for reading");
	if (!m_bValidForRead)
	{
		return false;
	}
	
	bool bValid = false;
	iValue = -1;

	switch (eType)
	{
	case EXTRASAVEDATA_MAP:
		FAssertMsg(iOwner == 0, "map data get from non-zero owner");
		FAssertMsg(iIndex >= 0 && iIndex < EXTRASAVEDATA_MAX_MAP_VALUES, "map values index out of range");
		
		if (iIndex >= 0 && iIndex < m_mapDataCount)
		{
			// get the value
			iValue = m_mapData[iIndex].iValue;
			bValid = true;
		}
		break;

	case EXTRASAVEDATA_TEAM:
		FAssertMsg(iOwner >= 0 && iOwner < MAX_TEAMS, "team out of range");
		FAssertMsg(iIndex >= 0 && iIndex < EXTRASAVEDATA_MAX_TEAM_VALUES, "team value index out of range");
		
		if (iOwner >= 0 && iOwner < MAX_TEAMS &&
			iIndex >= 0 && iIndex < m_teamDataCount[iOwner])
		{
			// get the value
			iValue = m_teamData[iOwner][iIndex].iValue;
			bValid = true;
		}
		break;

	case EXTRASAVEDATA_PLAYER:
		FAssertMsg(iOwner >= 0 && iOwner < MAX_PLAYERS, "player out of range");
		FAssertMsg(iIndex >= 0 && iIndex < EXTRASAVEDATA_MAX_PLAYER_VALUES, "player value index out of range");
		
		if (iOwner >= 0 && iOwner < MAX_PLAYERS &&
			iIndex >= 0 && iIndex < m_playerDataCount[iOwner])
		{
			// get the value
			iValue = m_playerData[iOwner][iIndex].iValue;
			bValid = true;
		}
		break;
	
	case EXTRASAVEDATA_CITY:
		FAssertMsg(iOwner >= 0 && iOwner < MAX_PLAYERS, "city owner out of range");
		FAssertMsg(iIndex >= 0 && iIndex < EXTRASAVEDATA_MAX_CITY_VALUES, "city value index out of range");
		
		// cityIDs are huge numbers, but we know they are a FFreeListTrashArray ID, so just take the index, which is 12 bits
		iID = (iID & FLTA_INDEX_MASK);

		CvExtraSaveDataNode* pNode;
		pNode = m_cityDataArray.getCityExtraDataNode(iID, iOwner, iIndex);
		// do we now have a valid item
		if (pNode != NULL)
		{
			// get the value
			iValue = pNode->iValue;
			bValid = true;
		}
		break;
	
	default:
		FAssertMsg(false, "unknown type");
		break;
	}

	return bValid;
}

// called by CvMap, CvTeamAI, CvPlayerAI, CvCityAI
bool CvExtraSaveData::setSaveData(ExtraSaveDataType eType, int iID, int iOwner, int iIndex, int iValue)
{
	// we should only call this when we are valid for reading
	FAssertMsg(m_bValidForWrite, "save data not valid for writing");
	if (!m_bValidForWrite)
	{
		return false;
	}

	// range check
	FAssertMsg(!(iValue < 0 || iValue == MAX_SHORT), "trying to write value out of range");
	if (iValue < 0 || iValue == MAX_SHORT)
	{
		return false;
	}
	
	bool bValid = false;
	
	switch (eType)
	{
	case EXTRASAVEDATA_MAP:
		FAssertMsg(iID == 0, "map data set to non-zero iID");
		FAssertMsg(iOwner == 0, "map data set to non-zero owner");
		FAssertMsg(iIndex >= 0 && iIndex < EXTRASAVEDATA_MAX_MAP_VALUES, "map values index out of range");
		
		if (iIndex >= 0 && iIndex < EXTRASAVEDATA_MAX_MAP_VALUES)
		{
			// if this higher than the count, then set to the max count, zero filling
			if (m_mapDataCount < (iIndex + 1))
			{
				// zero fill the extras
				while (m_mapDataCount < iIndex)
				{
					m_mapData[m_mapDataCount++].iValue = 0;
				}
				
				m_mapDataCount = iIndex + 1;
			}
			
			// set the value
			m_mapData[iIndex].iValue = iValue;
			bValid = true;
		}
		break;

	case EXTRASAVEDATA_TEAM:
		FAssertMsg(iID >= 0 && iID < MAX_TEAMS, "team out of range");
		FAssertMsg(iOwner == 0, "team data set to non-zero owner");
		FAssertMsg(iIndex >= 0 && iIndex <= EXTRASAVEDATA_MAX_TEAM_VALUES, "team value index out of range");
		
		if (iID >= 0 && iID < MAX_TEAMS &&
			iIndex >= 0 && iIndex < EXTRASAVEDATA_MAX_TEAM_VALUES)
		{
			// if this higher than the count, then set to the max count, zero filling
			if (m_teamDataCount[iID] < (iIndex + 1))
			{
				// zero fill the extras
				while (m_teamDataCount[iID] < iIndex)
				{
					m_teamData[iID][m_teamDataCount[iID]++].iValue = 0;
				}
				
				m_teamDataCount[iID] = iIndex + 1;
			}
			
			// set the value
			m_teamData[iID][iIndex].iValue = iValue;
			bValid = true;
		}
		break;

	case EXTRASAVEDATA_PLAYER:
		FAssertMsg(iID >= 0 && iID < MAX_PLAYERS, "player out of range");
		FAssertMsg(iOwner == 0, "player data set to non-zero owner");
		FAssertMsg(iIndex >= 0 && iIndex <= EXTRASAVEDATA_MAX_PLAYER_VALUES, "player value index out of range");
		
		if (iID >= 0 && iID < MAX_PLAYERS &&
			iIndex >= 0 && iIndex < EXTRASAVEDATA_MAX_PLAYER_VALUES)
		{
			// if this higher than the count, then set to the max count, zero filling
			if (m_playerDataCount[iID] < (iIndex + 1))
			{
				// zero fill the extras
				while (m_playerDataCount[iID] < iIndex)
				{
					m_playerData[iID][m_playerDataCount[iID]++].iValue = 0;
				}
				
				m_playerDataCount[iID] = iIndex + 1;
			}
			
			// set the value
			m_playerData[iID][iIndex].iValue = iValue;
			bValid = true;
		}
		break;

	case EXTRASAVEDATA_CITY:
		FAssertMsg(iOwner >= 0 && iOwner < MAX_PLAYERS, "city owner out of range");
		FAssertMsg(iIndex >= 0 && iIndex < EXTRASAVEDATA_MAX_CITY_VALUES, "city value index out of range");
		
		// cityIDs are huge numbers, but we know they are a FFreeListTrashArray ID, so just take the index, which is 12 bits
		iID = (iID & FLTA_INDEX_MASK);

		// get the node, creating if necessary
		CvExtraSaveDataNode* pNode;
		pNode = m_cityDataArray.getCityExtraDataNode(iID, iOwner, iIndex, /*bCreate*/ true);
		// do we now have a valid item
		if (pNode != NULL)
		{
			// set the value
			pNode->iValue = iValue;
			bValid = true;
		}
		break;

	default:
		FAssertMsg(false, "unknown type");
		break;
	}

	return bValid;
}


/* protected functions */

// index should be 0 on input for header
bool CvExtraSaveData::pushHeader(CvMap& kMap, int iIndex, CvExtraSaveDataHeader& header)
{
	FAssertMsg(iIndex == 0, "header not being written to the start");

	bool bValid = true;
	
	if (bValid)
	{
		bValid = pushToFoundValues(kMap, iIndex++, header.iValidCheck1);
	}

	if (bValid)
	{
		bValid = pushToFoundValues(kMap, iIndex++, header.iValidCheck2);
	}
	
	if (bValid)
	{
		bValid = pushToFoundValues(kMap, iIndex++, header.iValidCheck3);
	}
	
	if (bValid)
	{
		bValid = pushToFoundValues(kMap, iIndex++, header.iSize);
	}
	
	if (bValid)
	{
		bValid = pushToFoundValues(kMap, iIndex++, header.iHeaderSize);
	}
	
	if (bValid)
	{
		bValid = pushToFoundValues(kMap, iIndex++, header.iNodeHeaderSize);
	}
	
	if (bValid)
	{
		bValid = pushToFoundValues(kMap, iIndex++, header.iNodeSize);
	}

	return bValid;
}

// index should be 0 on input for header
bool CvExtraSaveData::popHeader(CvMap& kMap, int iIndex, CvExtraSaveDataHeader& header)
{
	FAssertMsg(iIndex == 0, "header not being written to the start");

	bool bValid = true;
	
	if (bValid)
	{
		bValid = popFromFoundValues(kMap, iIndex++, header.iValidCheck1);
	}

	if (bValid)
	{
		bValid = popFromFoundValues(kMap, iIndex++, header.iValidCheck2);
	}
	
	if (bValid)
	{
		bValid = popFromFoundValues(kMap, iIndex++, header.iValidCheck3);
	}
	
	if (bValid)
	{
		bValid = popFromFoundValues(kMap, iIndex++, header.iSize);
	}
	
	if (bValid)
	{
		bValid = popFromFoundValues(kMap, iIndex++, header.iHeaderSize);
	}
	
	if (bValid)
	{
		bValid = popFromFoundValues(kMap, iIndex++, header.iNodeHeaderSize);
	}
	
	if (bValid)
	{
		bValid = popFromFoundValues(kMap, iIndex++, header.iNodeSize);
	}

	return bValid;
}

bool CvExtraSaveData::pushNodeHeader(CvMap& kMap, int iIndex, CvExtraSaveDataNodeHeader& nodeHeader)
{
	bool bValid = true;
	
	if (bValid)
	{
		bValid = pushToFoundValues(kMap, iIndex++, nodeHeader.eType);
	}
	
	if (bValid)
	{
		bValid = pushToFoundValues(kMap, iIndex++, nodeHeader.iID);
	}
	
	if (bValid)
	{
		bValid = pushToFoundValues(kMap, iIndex++, nodeHeader.iOwner);
	}
	
	if (bValid)
	{
		bValid = pushToFoundValues(kMap, iIndex++, nodeHeader.iCount);
	}
	
	return bValid;
}

bool CvExtraSaveData::popNodeHeader(CvMap& kMap, int iIndex, CvExtraSaveDataNodeHeader& nodeHeader)
{
	bool bValid = true;
	
	if (bValid)
	{
		bValid = popFromFoundValues(kMap, iIndex++, nodeHeader.eType);
	}
	
	if (bValid)
	{
		bValid = popFromFoundValues(kMap, iIndex++, nodeHeader.iID);
	}
	
	if (bValid)
	{
		bValid = popFromFoundValues(kMap, iIndex++, nodeHeader.iOwner);
	}
	
	if (bValid)
	{
		bValid = popFromFoundValues(kMap, iIndex++, nodeHeader.iCount);
	}
	
	return bValid;
}

bool CvExtraSaveData::pushNode(CvMap& kMap, int iIndex, CvExtraSaveDataNode& node)
{
	bool bValid = true;
	
	if (bValid)
	{
		bValid = pushToFoundValues(kMap, iIndex++, node.iValue);
	}
	
	return bValid;
}

bool CvExtraSaveData::popNode(CvMap& kMap, int iIndex, CvExtraSaveDataNode& node)
{
	bool bValid = true;
	
	if (bValid)
	{
		bValid = popFromFoundValues(kMap, iIndex++, node.iValue);
	}
	
	return bValid;
}


bool CvExtraSaveData::pushToFoundValues(CvMap& kMap, int iIndex, short iValue)
{
	FAssertMsg(validValueToEncode(iValue), "trying to push invalid value (can only use 15 bits), must be positive");

	bool bValid = false;
	
	if (validValueToEncode(iValue))
	{
		int iPlotIndex, iPlayerIndex;
		indexToPlotIndexPlayerIndex(iIndex, iPlotIndex, iPlayerIndex);
		FAssertMsg(iPlotIndex < kMap.numPlotsINLINE(), "invalid plot index!");
		FAssertMsg(iPlayerIndex < MAX_PLAYERS, "invalid player index!");
		
		CvPlot* pPlot = kMap.plotByIndexINLINE(iPlotIndex);
		FAssertMsg(pPlot != NULL, "NULL plot by index!");

		if (pPlot != NULL)
		{
			pPlot->m_aiFoundValue[iPlayerIndex] = encodeToFoundValue(iValue);
			bValid = true;
		}
	}

	return bValid;
}

bool CvExtraSaveData::popFromFoundValues(CvMap& kMap, int iIndex, short& iValue)
{
	bool bValid = false;
	iValue = -1;
	
	int iPlotIndex, iPlayerIndex;
	indexToPlotIndexPlayerIndex(iIndex, iPlotIndex, iPlayerIndex);
	FAssertMsg(iPlotIndex < kMap.numPlotsINLINE(), "invalid plot index!");
	FAssertMsg(iPlayerIndex < MAX_PLAYERS, "invalid player index!");
	
	CvPlot* pPlot = kMap.plotByIndexINLINE(iPlotIndex);
	FAssertMsg(pPlot != NULL, "NULL plot by index!");

	if (pPlot != NULL)
	{
		short iFoundValue = pPlot->m_aiFoundValue[iPlayerIndex];

		if (validFoundValueToDecode(iFoundValue))
		{
			iValue = decodeFromFoundValue(iFoundValue);

			bValid = true;
		}
		
	}

	return bValid;
}

// total number of shorts to hold all our data
// can only call this from write, it is not valid on read, we cannot know number of cities yet
int CvExtraSaveData::calculateMaxTotalSize()
{
	int iTotalSize = sizeof(CvExtraSaveDataHeader) / sizeof(short);
	
	int iMapDataSize = (sizeof(CvExtraSaveDataNodeHeader) + (sizeof(CvExtraSaveDataNode) * EXTRASAVEDATA_MAX_MAP_VALUES)) / sizeof(short);
	iTotalSize += iMapDataSize; 
	
	int iTeamDataSize = (sizeof(CvExtraSaveDataNodeHeader) + (sizeof(CvExtraSaveDataNode) * EXTRASAVEDATA_MAX_TEAM_VALUES)) / sizeof(short);
	iTotalSize += iTeamDataSize * MAX_TEAMS; 

	int iPlayerDataSize = (sizeof(CvExtraSaveDataNodeHeader) + (sizeof(CvExtraSaveDataNode) * EXTRASAVEDATA_MAX_PLAYER_VALUES)) / sizeof(short);
	iTotalSize += iPlayerDataSize * MAX_PLAYERS; 
	
	// count cities
	for (int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		int iCityCount = GET_PLAYER((PlayerTypes)iI).getNumCities();
		int iCityDataSize = (sizeof(CvExtraSaveDataNodeHeader) + (sizeof(CvExtraSaveDataNode) * EXTRASAVEDATA_MAX_CITY_VALUES)) / sizeof(short);
		iTotalSize += iCityDataSize * iCityCount; 
	}

	return iTotalSize;
}
