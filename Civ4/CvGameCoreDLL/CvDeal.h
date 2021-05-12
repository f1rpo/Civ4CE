// CvDeal.h

#ifndef CIV4_DEAL_H
#define CIV4_DEAL_H

//#include "CvStructs.h"
#include "LinkedList.h"

class CvDeal
{

public:

	CvDeal();
	virtual ~CvDeal();

	void init(int iID, PlayerTypes eFirstPlayer, PlayerTypes eSecondPlayer);
	void uninit();
	void reset(int iID = 0, PlayerTypes eFirstPlayer = NO_PLAYER, PlayerTypes eSecondPlayer = NO_PLAYER);

	DllExport void kill();

	void addTrades(CLinkList<TradeData>* pFirstList, CLinkList<TradeData>* pSecondList);

	void doTurn();

	void verify();

	bool isPeaceDeal();
	bool isPeaceDealBetweenOthers(CLinkList<TradeData>* pFirstList, CLinkList<TradeData>* pSecondList);

	DllExport int getID() const;
	void setID(int iID);

	int getInitialGameTurn() const;
	void setInitialGameTurn(int iNewValue);

	DllExport PlayerTypes getFirstPlayer() const;
	DllExport PlayerTypes getSecondPlayer() const;

	void clearFirstTrades();
	void insertAtEndFirstTrades(TradeData trade);
	DllExport CLLNode<TradeData>* nextFirstTradesNode(CLLNode<TradeData>* pNode);
	int getLengthFirstTrades() const;
	DllExport CLLNode<TradeData>* headFirstTradesNode();
	CLinkList<TradeData>* getFirstTrades();

	void clearSecondTrades();
	void insertAtEndSecondTrades(TradeData trade);
	DllExport CLLNode<TradeData>* nextSecondTradesNode(CLLNode<TradeData>* pNode);
	int getLengthSecondTrades() const;
	DllExport CLLNode<TradeData>* headSecondTradesNode();
	CLinkList<TradeData>* getSecondTrades();

	DllExport bool isCancelable(void);
	DllExport int turnsToCancel(void);

	void read(FDataStreamBase* pStream);
	void write(FDataStreamBase* pStream);

protected:

	bool startTrade(TradeData trade, PlayerTypes eFromPlayer, PlayerTypes eToPlayer);
	void endTrade(TradeData trade, PlayerTypes eFromPlayer, PlayerTypes eToPlayer);

	int m_iID;
	int m_iInitialGameTurn;

	PlayerTypes m_eFirstPlayer;
	PlayerTypes m_eSecondPlayer;

	CLinkList<TradeData> m_firstTrades;
	CLinkList<TradeData> m_secondTrades;

};

#endif
