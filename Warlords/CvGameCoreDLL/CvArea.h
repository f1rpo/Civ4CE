// area.h

#ifndef CIV4_AREA_H
#define CIV4_AREA_H

//#include "CvStructs.h"

class CvCity;
class CvPlot;

class CvArea
{

public:

  CvArea();
  virtual ~CvArea();

  void init(int iID, bool bWater);
	void uninit();
	void reset(int iID = 0, bool bWater = false, bool bConstructorCall = false);

	int calculateTotalBestNatureYield();																// Exposed to Python

	int countCoastalLand();																							// Exposed to Python
	int countNumUniqueBonusTypes();																			// Exposed to Python
	int countHasReligion(ReligionTypes eReligion, PlayerTypes eOwner = NO_PLAYER);		// Exposed to Python

	DllExport int getID();																							// Exposed to Python
	void setID(int iID);

	int getNumTiles();																									// Exposed to Python
	bool isLake();																											// Exposed to Python
	void changeNumTiles(int iChange);

	int getNumOwnedTiles();																							// Exposed to Python
	int getNumUnownedTiles();																						// Exposed to Python
	void changeNumOwnedTiles(int iChange);

	int getNumRiverEdges();																							// Exposed to Python
	void changeNumRiverEdges(int iChange);

	int getNumCities();																									// Exposed to Python

	int getNumUnits();																									// Exposed to Python

	int getTotalPopulation();																						// Exposed to Python

	int getNumStartingPlots();																					// Exposed to Python
	void changeNumStartingPlots(int iChange);

	bool isWater();																											// Exposed to Python

	int getUnitsPerPlayer(PlayerTypes eIndex);													// Exposed to Python
	void changeUnitsPerPlayer(PlayerTypes eIndex, int iChange);

	int getAnimalsPerPlayer(PlayerTypes eIndex);												// Exposed to Python
	void changeAnimalsPerPlayer(PlayerTypes eIndex, int iChange);

	int getCitiesPerPlayer(PlayerTypes eIndex);													// Exposed to Python
	void changeCitiesPerPlayer(PlayerTypes eIndex, int iChange);

	int getPopulationPerPlayer(PlayerTypes eIndex);											// Exposed to Python
	void changePopulationPerPlayer(PlayerTypes eIndex, int iChange);

	int getBuildingGoodHealth(PlayerTypes eIndex);											// Exposed to Python
	void changeBuildingGoodHealth(PlayerTypes eIndex, int iChange);

	int getBuildingBadHealth(PlayerTypes eIndex);												// Exposed to Python
	void changeBuildingBadHealth(PlayerTypes eIndex, int iChange);

	int getBuildingHappiness(PlayerTypes eIndex);												// Exposed to Python
	void changeBuildingHappiness(PlayerTypes eIndex, int iChange);

	int getFreeSpecialist(PlayerTypes eIndex);													// Exposed to Python
	void changeFreeSpecialist(PlayerTypes eIndex, int iChange);

	int getPower(PlayerTypes eIndex);																		// Exposed to Python
	void changePower(PlayerTypes eIndex, int iChange);

	int getBestFoundValue(PlayerTypes eIndex);													// Exposed to Python
	void setBestFoundValue(PlayerTypes eIndex, int iNewValue);

	int getNumRevealedTiles(TeamTypes eIndex);													// Exposed to Python
	int getNumUnrevealedTiles(TeamTypes eIndex);												// Exposed to Python
	void changeNumRevealedTiles(TeamTypes eIndex, int iChange);

	int getCleanPowerCount(TeamTypes eIndex);
	bool isCleanPower(TeamTypes eIndex);																// Exposed to Python
	void changeCleanPowerCount(TeamTypes eIndex, int iChange);

	AreaAITypes getAreaAIType(TeamTypes eIndex);												// Exposed to Python
	void setAreaAIType(TeamTypes eIndex, AreaAITypes eNewValue);

	CvCity* getTargetCity(PlayerTypes eIndex);													// Exposed to Python
	void setTargetCity(PlayerTypes eIndex, CvCity* pNewValue);

	int getYieldRateModifier(PlayerTypes eIndex1, YieldTypes eIndex2);	// Exposed to Python
	void changeYieldRateModifier(PlayerTypes eIndex1, YieldTypes eIndex2, int iChange);

	int getNumTrainAIUnits(PlayerTypes eIndex1, UnitAITypes eIndex2);		// Exposed to Python
	void changeNumTrainAIUnits(PlayerTypes eIndex1, UnitAITypes eIndex2, int iChange);

	int getNumAIUnits(PlayerTypes eIndex1, UnitAITypes eIndex2);				// Exposed to Python
	void changeNumAIUnits(PlayerTypes eIndex1, UnitAITypes eIndex2, int iChange);

	int getNumBonuses(BonusTypes eBonus);																// Exposed to Python
	int getNumTotalBonuses();																						// Exposed to Python
	void changeNumBonuses(BonusTypes eBonus, int iChange);

	int getNumImprovements(ImprovementTypes eImprovement);							// Exposed to Python
	void changeNumImprovements(ImprovementTypes eImprovement, int iChange);

protected:

	int m_iID;
	int m_iNumTiles;
	int m_iNumOwnedTiles;
	int m_iNumRiverEdges;
	int m_iNumUnits;
	int m_iNumCities;
	int m_iTotalPopulation;
	int m_iNumStartingPlots;

	bool m_bWater;

	int m_aiUnitsPerPlayer[MAX_PLAYERS];
	int m_aiAnimalsPerPlayer[MAX_PLAYERS];
	int m_aiCitiesPerPlayer[MAX_PLAYERS];
	int m_aiPopulationPerPlayer[MAX_PLAYERS];
	int m_aiBuildingGoodHealth[MAX_PLAYERS];
	int m_aiBuildingBadHealth[MAX_PLAYERS];
	int m_aiBuildingHappiness[MAX_PLAYERS];
	int m_aiFreeSpecialist[MAX_PLAYERS];
	int m_aiPower[MAX_PLAYERS];
	int m_aiBestFoundValue[MAX_PLAYERS];
	int m_aiNumRevealedTiles[MAX_TEAMS];
	int m_aiCleanPowerCount[MAX_TEAMS];

	AreaAITypes m_aeAreaAIType[MAX_TEAMS];

	IDInfo m_aTargetCities[MAX_PLAYERS];

	int m_aaiYieldRateModifier[MAX_PLAYERS][NUM_YIELD_TYPES];
	int m_aaiNumTrainAIUnits[MAX_PLAYERS][NUM_UNITAI_TYPES];
	int m_aaiNumAIUnits[MAX_PLAYERS][NUM_UNITAI_TYPES];

	int* m_paiNumBonuses;
	int* m_paiNumImprovements;

public:

	// for serialization
	virtual void read(FDataStreamBase* pStream);
	virtual void write(FDataStreamBase* pStream);

};

#endif
