#ifndef CVSTRUCTS_H
#define CVSTRUCTS_H

// structs.h

//#include "CvEnums.h"
#include "CvString.h"

// XXX these should not be in the DLL per se (if the user changes them, we are screwed...)

struct DllExport XYCoords
{
	XYCoords(int x=0, int y=0) : iX(x), iY(y) {}
	int iX;
	int iY;
};

struct DllExport IDInfo
{

	IDInfo(PlayerTypes eOwner=NO_PLAYER, int iID=FFreeList::INVALID_INDEX) : eOwner(eOwner), iID(iID) {}
	PlayerTypes eOwner;
	int iID;

	bool operator== (const IDInfo& info) const
	{
		return (eOwner == info.eOwner && iID == info.iID);
	}

	void reset()
	{
		eOwner = NO_PLAYER;
		iID = FFreeList::INVALID_INDEX;
	}
};

struct DllExport GameTurnInfo				// Exposed to Python
{
	int iYearIncrement;
	int iNumGameTurnsPerIncrement;
};

struct DllExport OrderData					// Exposed to Python
{
	OrderTypes eOrderType;
	int iData1;
	int iData2;
	bool bSave;
};

struct DllExport MissionData				// Exposed to Python
{
	MissionTypes eMissionType;
	int iData1;
	int iData2;
	int iFlags;
	int iPushTurn;
};

struct DllExport TradeData					// Exposed to Python
{
	TradeableItems m_eItemType;				//	What type of item is this
	int m_iData;											//	Any additional data?
	bool m_bOffering;									//	Is this item up for grabs?
	bool m_bHidden;										//	Are we hidden?
};

struct DllExport EventMessage
{
	CvWString szDescription;
	int iExpirationTurn;
	int iX;
	int iY;

	// python friendly accessors
	std::wstring getDescription() const { return szDescription;	}
};

struct DllExport FOWVis
{
	uint uiCount;
	POINT* pOffsets;  // array of "Offset" points

	// python friendly accessors
	POINT getOffsets(int i) const { return pOffsets[i]; }
};

struct DllExport FOWInfo
{
	unsigned char ucFogValue;							// Value drawn to texture
	bool bDirty;								// Set to true of the FOW texture does not reflect this information.
};

struct DllExport PBGameSetupData
{
	int iSize;
	int iClimate;
	int iSeaLevel;
	int iSpeed;
	int iEra;

	int iMaxTurns;
	int iCityElimination;
	int iTurnTime;

	int iNumCustomMapOptions;
	int * aiCustomMapOptions;
	int getCustomMapOption(int iOption) {return aiCustomMapOptions[iOption];}

	int iNumVictories;
	bool * abVictories;
	bool getVictory(int iVictory) {return abVictories[iVictory];}

	std::wstring szMapName;
	std::wstring getMapName() {return szMapName;}

	bool abOptions[NUM_GAMEOPTION_TYPES];
	bool getOptionAt(int iOption) {return abOptions[iOption];}

	bool abMPOptions[NUM_MPOPTION_TYPES];
	bool getMPOptionAt(int iOption) {return abMPOptions[iOption];}
};

struct DllExport PBPlayerSetupData
{
	int iWho;
	int iCiv;
	int iLeader;
	int iTeam;
	int iDifficulty;

	std::wstring szStatusText;
	std::wstring getStatusText() {return szStatusText;}
};

struct DllExport PBPlayerAdminData
{
	std::wstring szName;
	std::wstring getName() {return szName;}
	std::wstring szPing;
	std::wstring getPing() {return szPing;}
	std::wstring szScore;
	std::wstring getScore() {return szScore;}
	bool bHuman;
	bool bClaimed;
	bool bTurnActive;
};

class CvUnit;
class CvPlot;

//! An enumeration for indexing units within a CvBattleDefinition
enum eBattleUnitIndex
{
	BDU_ATTACKER,	//!< Index referencing the attacking unit
	BDU_DEFENDER,	//!< Index referencing the defending unit
	BDU_COUNT		//!< The number of unit index references
};


//!< An enumeration for indexing times within the CvBattleDefinition
enum eBattleTimeIndex
{
	BDT_BEGIN,
	BDT_RANGED,
	BDT_END,
	BDT_COUNT
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  STRUCT:      CvBattleRound
//!  \brief		Represents a single round within a battle.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
struct DllExport CvBattleRound
{
public:

	CvBattleRound();
	bool isValid() const;


	int		iKilled[BDU_COUNT];		//!< The number of units killed during this round for both sides
	int		iAlive[BDU_COUNT];		//!< The number of units alive at the end of this round for both sides
	int		iWaveSize;				//!< The number of units that can perform exchanges
	bool	bRangedRound;				//!< true if this round is ranged, false otherwise
};

//------------------------------------------------------------------------------------------------

typedef std::vector< CvBattleRound > CvBattlePlanVector;		//!< Type declaration for a collection of battle round definitions

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CLASS:      CvMissionDefinition
//!  \brief		Base mission definition struct
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
struct DllExport CvMissionDefinition
{
	MissionTypes		eMissionType;			//!< The type of event
	CvUnit *			pkUnits[BDU_COUNT];		//!< The units involved
	float				fMissionTime;			//!< The amount of time that the event will take
	const CvPlot *		pkPlot;					//!< The plot associated with the event

	CvMissionDefinition();
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  STRUCT:     CvBattleDefinition
//!  \brief		A definition passed to CvBattleManager to start a battle between units
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
struct DllExport CvBattleDefinition : public CvMissionDefinition
{
	int					iDamage[BDU_COUNT][BDT_COUNT];	//!< The beginning damage of the units
	int					iFirstStrikes[BDU_COUNT];		//!< The number of ranged first strikes the units made
	int					iRangedRoundCount;				//!< The number of ranged rounds
	int					iMeleeRoundCount;				//!< The number of melee rounds
	bool				bAdvanceSquare;					//!< true if the attacking unit should move into the new square
	CvBattlePlanVector	vctBattlePlan;					//!< The rounds that define the battle plan

	CvBattleDefinition();
	CvBattleDefinition( const CvBattleDefinition & kCopy );
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CLASS:      CvAirMissionDefinition
//!  \brief		A definition passed to CvAirMissionManager to start an air mission
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
struct DllExport CvAirMissionDefinition : public CvMissionDefinition
{
	int					iDamage[BDU_COUNT];		//!< The ending damage of the units

	CvAirMissionDefinition();
	CvAirMissionDefinition( const CvAirMissionDefinition & kCopy );
};

struct DllExport CvWidgetDataStruct
{
	int m_iData1;										//	The first bit of data
	int m_iData2;										//	The second piece of data

	bool m_bOption;									//	A boolean piece of data

	WidgetTypes m_eWidgetType;			//	What the 'type' of this widget is (for parsing help and executing actions)
};

#endif	// CVSTRUCTS_H
