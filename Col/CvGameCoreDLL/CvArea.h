#pragma once

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

	int calculateTotalBestNatureYield() const;

	int countCoastalLand() const;
	int countNumUniqueBonusTypes() const;
	int countHasReligion(ReligionTypes eReligion, PlayerTypes eOwner = NO_PLAYER) const;
	int countHasCorporation(CorporationTypes eCorporation, PlayerTypes eOwner = NO_PLAYER) const;

	DllExport int getID() const;
	void setID(int iID);

	int getNumTiles() const;
	bool isLake() const;
	void changeNumTiles(int iChange);

	int getNumOwnedTiles() const;
	int getNumUnownedTiles() const;
	void changeNumOwnedTiles(int iChange);

	int getNumRiverEdges() const;
	void changeNumRiverEdges(int iChange);

	int getNumCities() const;

	int getNumUnits() const;

	int getTotalPopulation() const;

	int getNumStartingPlots() const;
	void changeNumStartingPlots(int iChange);

	bool isWater() const;

	int getUnitsPerPlayer(PlayerTypes eIndex) const;
	void changeUnitsPerPlayer(PlayerTypes eIndex, int iChange);

	int getAnimalsPerPlayer(PlayerTypes eIndex) const;
	void changeAnimalsPerPlayer(PlayerTypes eIndex, int iChange);

	int getCitiesPerPlayer(PlayerTypes eIndex) const;
	void changeCitiesPerPlayer(PlayerTypes eIndex, int iChange);

	int getPopulationPerPlayer(PlayerTypes eIndex) const;
	void changePopulationPerPlayer(PlayerTypes eIndex, int iChange);

	int getBuildingGoodHealth(PlayerTypes eIndex) const;
	void changeBuildingGoodHealth(PlayerTypes eIndex, int iChange);

	int getBuildingBadHealth(PlayerTypes eIndex) const;
	void changeBuildingBadHealth(PlayerTypes eIndex, int iChange);

	int getBuildingHappiness(PlayerTypes eIndex) const;
	void changeBuildingHappiness(PlayerTypes eIndex, int iChange);

	int getFreeSpecialist(PlayerTypes eIndex) const;
	void changeFreeSpecialist(PlayerTypes eIndex, int iChange);

	int getPower(PlayerTypes eIndex) const;
	void changePower(PlayerTypes eIndex, int iChange);

	int getBestFoundValue(PlayerTypes eIndex) const;
	void setBestFoundValue(PlayerTypes eIndex, int iNewValue);

	int getNumRevealedTiles(TeamTypes eIndex) const;
	int getNumUnrevealedTiles(TeamTypes eIndex) const;
	void changeNumRevealedTiles(TeamTypes eIndex, int iChange);

	int getCleanPowerCount(TeamTypes eIndex) const;
	bool isCleanPower(TeamTypes eIndex) const;
	void changeCleanPowerCount(TeamTypes eIndex, int iChange);

	int getBorderObstacleCount(TeamTypes eIndex) const;
	DllExport bool isBorderObstacle(TeamTypes eIndex) const;
	void changeBorderObstacleCount(TeamTypes eIndex, int iChange);

	AreaAITypes getAreaAIType(TeamTypes eIndex) const;
	void setAreaAIType(TeamTypes eIndex, AreaAITypes eNewValue);

	CvCity* getTargetCity(PlayerTypes eIndex) const;
	void setTargetCity(PlayerTypes eIndex, CvCity* pNewValue);

	int getYieldRateModifier(PlayerTypes eIndex1, YieldTypes eIndex2) const;
	void changeYieldRateModifier(PlayerTypes eIndex1, YieldTypes eIndex2, int iChange);

	int getNumTrainAIUnits(PlayerTypes eIndex1, UnitAITypes eIndex2) const;
	void changeNumTrainAIUnits(PlayerTypes eIndex1, UnitAITypes eIndex2, int iChange);

	int getNumAIUnits(PlayerTypes eIndex1, UnitAITypes eIndex2) const;
	void changeNumAIUnits(PlayerTypes eIndex1, UnitAITypes eIndex2, int iChange);

	int getNumBonuses(BonusTypes eBonus) const;
	int getNumTotalBonuses() const;
	void changeNumBonuses(BonusTypes eBonus, int iChange);

	int getNumImprovements(ImprovementTypes eImprovement) const;
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

	int* m_aiUnitsPerPlayer;
	int* m_aiAnimalsPerPlayer;
	int* m_aiCitiesPerPlayer;
	int* m_aiPopulationPerPlayer;
	int* m_aiBuildingGoodHealth;
	int* m_aiBuildingBadHealth;
	int* m_aiBuildingHappiness;
	int* m_aiFreeSpecialist;
	int* m_aiPower;
	int* m_aiBestFoundValue;
	int* m_aiNumRevealedTiles;
	int* m_aiCleanPowerCount;
	int* m_aiBorderObstacleCount;

	AreaAITypes* m_aeAreaAIType;

	IDInfo* m_aTargetCities;

	int** m_aaiYieldRateModifier;
	int** m_aaiNumTrainAIUnits;
	int** m_aaiNumAIUnits;

	int* m_paiNumBonuses;
	int* m_paiNumImprovements;

public:

	// for serialization
	virtual void read(FDataStreamBase* pStream);
	virtual void write(FDataStreamBase* pStream);

};

#endif
