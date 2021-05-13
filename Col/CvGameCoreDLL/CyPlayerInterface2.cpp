#include "CvGameCoreDLL.h"
#include "CyPlayer.h"
#include "CyUnit.h"
#include "CyCity.h"
#include "CyPlot.h"
#include "CySelectionGroup.h"
#include "CyArea.h"
#include "CyTradeRoute.h"
//# include <boost/python/manage_new_object.hpp>
//# include <boost/python/return_value_policy.hpp>
//# include <boost/python/scope.hpp>
//
// published python interface for CyPlayer
//
void CyPlayerPythonInterface2(python::class_<CyPlayer>& x)
{
	OutputDebugString("Python Extension Module - CyPlayerPythonInterface2\n");
	// set the docstring of the current module scope
	python::scope().attr("__doc__") = "Civilization IV Player Class";
	x
		// PatchMod: Tax party city START
		.def("getHighestStoredYieldPartyCityId", &CyPlayer::getHighestStoredYieldPartyCityId)
		// PatchMod: Tax party city END

		.def("AI_updateFoundValues", &CyPlayer::AI_updateFoundValues, "void (bool bStartingLoc)")
		.def("AI_foundValue", &CyPlayer::AI_foundValue, "int (int, int, int, bool)")
		.def("AI_demandRebukedWar", &CyPlayer::AI_demandRebukedWar, "bool (int /*PlayerTypes*/)")
		.def("AI_getAttitude", &CyPlayer::AI_getAttitude, "AttitudeTypes (int /*PlayerTypes*/) - Gets the attitude of the player towards the player passed in")
		.def("AI_unitValue", &CyPlayer::AI_unitValue, "int (int /*UnitTypes*/ eUnit, int /*UnitAITypes*/ eUnitAI, CyArea* pArea)")
		.def("AI_civicValue", &CyPlayer::AI_civicValue, "int (int /*CivicTypes*/ eCivic)")
		.def("AI_totalUnitAIs", &CyPlayer::AI_totalUnitAIs, "int (int /*UnitAITypes*/ eUnitAI)")
		.def("AI_totalAreaUnitAIs", &CyPlayer::AI_totalAreaUnitAIs, "int (CyArea* pArea, int /*UnitAITypes*/ eUnitAI)")
		.def("AI_totalWaterAreaUnitAIs", &CyPlayer::AI_totalWaterAreaUnitAIs, "int (CyArea* pArea, int /*UnitAITypes*/ eUnitAI)")
		.def("AI_getNumAIUnits", &CyPlayer::AI_getNumAIUnits, "int (UnitAIType) - Returns # of UnitAITypes the player current has of UnitAIType")
		.def("AI_getAttitudeExtra", &CyPlayer::AI_getAttitudeExtra, "int (int /*PlayerTypes*/ eIndex) - Returns the extra attitude for this player - usually scenario specific")
		.def("AI_setAttitudeExtra", &CyPlayer::AI_setAttitudeExtra, "void (int /*PlayerTypes*/ eIndex, int iNewValue) - Sets the extra attitude for this player - usually scenario specific")
		.def("AI_changeAttitudeExtra", &CyPlayer::AI_changeAttitudeExtra, "void (int /*PlayerTypes*/ eIndex, int iChange) - Changes the extra attitude for this player - usually scenario specific")
		.def("AI_getMemoryCount", &CyPlayer::AI_getMemoryCount, "int (/*PlayerTypes*/ eIndex1, /*MemoryTypes*/ eIndex2)")
		.def("AI_changeMemoryCount", &CyPlayer::AI_changeMemoryCount, "void (/*PlayerTypes*/ eIndex1, /*MemoryTypes*/ eIndex2, int iChange)")
		.def("AI_getExtraGoldTarget", &CyPlayer::AI_getExtraGoldTarget, "int ()")
		.def("AI_setExtraGoldTarget", &CyPlayer::AI_setExtraGoldTarget, "void (int)")
		.def("getScoreHistory", &CyPlayer::getScoreHistory, "int (int iTurn)")
		.def("getEconomyHistory", &CyPlayer::getEconomyHistory, "int (int iTurn)")
		.def("getIndustryHistory", &CyPlayer::getIndustryHistory, "int (int iTurn)")
		.def("getAgricultureHistory", &CyPlayer::getAgricultureHistory, "int (int iTurn)")
		.def("getPowerHistory", &CyPlayer::getPowerHistory, "int (int iTurn)")
		.def("getCultureHistory", &CyPlayer::getCultureHistory, "int (int iTurn)")
		.def("getScriptData", &CyPlayer::getScriptData, "str () - Get stored custom data (via pickle)")
		.def("setScriptData", &CyPlayer::setScriptData, "void (str) - Set stored custom data (via pickle)")
		.def("AI_maxGoldTrade", &CyPlayer::AI_maxGoldTrade, "int (int)")
		.def("forcePeace", &CyPlayer::forcePeace, "void (int)")
		.def("getHighestTradedYield", &CyPlayer::getHighestTradedYield, "YieldTypes ()")
		.def("getHighestStoredYieldCityId", &CyPlayer::getHighestStoredYieldCityId, "int (int /*YieldTypes*/)")
		.def("getCrossesStored", &CyPlayer::getCrossesStored, "int ()")
		.def("getBellsStored", &CyPlayer::getBellsStored, "int ()")
		.def("getTaxRate", &CyPlayer::getTaxRate, "int ()")
		.def("changeTaxRate", &CyPlayer::changeTaxRate, "void (int)")
		.def("canTradeWithEurope", &CyPlayer::canTradeWithEurope, "bool ()")
		.def("getSellToEuropeProfit", &CyPlayer::getSellToEuropeProfit, "int ()")
		.def("getYieldSellPrice", &CyPlayer::getYieldSellPrice, "int (eYield)")
		.def("getYieldBuyPrice", &CyPlayer::getYieldBuyPrice, "int (eYield)")
		.def("setYieldBuyPrice", &CyPlayer::setYieldBuyPrice, "void (eYield, int iValue, bool bMessage)")
		.def("sellYieldUnitToEurope", &CyPlayer::sellYieldUnitToEurope, "void (CyUnit* pUnit, int iAmount, int iCommission)")
		.def("buyYieldUnitFromEurope", &CyPlayer::buyYieldUnitFromEurope, python::return_value_policy<python::manage_new_object>(), "CyUnit (int /*YieldTypes*/ eYield, int iAmount, CyUnit* pTransport)")
		.def("getEuropeUnitBuyPrice", &CyPlayer::getEuropeUnitBuyPrice, "int (int (UnitTypes))")
		.def("buyEuropeUnit", &CyPlayer::buyEuropeUnit, python::return_value_policy<python::manage_new_object>(), "CyUnit (int /*UnitTypes*/ eUnit)")
		.def("getYieldBoughtTotal", &CyPlayer::getYieldBoughtTotal, "int (int /*YieldTypes*/ eYield)")
		.def("getNumRevolutionEuropeUnits", &CyPlayer::getNumRevolutionEuropeUnits, "int ()")
		.def("getRevolutionEuropeUnit", &CyPlayer::getRevolutionEuropeUnit, "int (int iIndex)")
		.def("getRevolutionEuropeProfession", &CyPlayer::getRevolutionEuropeProfession, "int (int iIndex)")
		.def("getDocksNextUnit", &CyPlayer::getDocksNextUnit, "int (int iIndex)")
		.def("isEurope", &CyPlayer::isEurope, "bool ()")
		.def("isInRevolution", &CyPlayer::isInRevolution, "bool ()")
		.def("addTradeRoute", &CyPlayer::addTradeRoute, "int (int iSourceCityOwner, int iSourceCityId, int iDestinationCityOwner, int iDestinationCityId, int /*YieldTypes*/ eYield)")
		.def("removeTradeRoute", &CyPlayer::removeTradeRoute, "bool (int iId)")
		.def("getTradeRoute", &CyPlayer::getTradeRoute, python::return_value_policy<python::manage_new_object>(), "CyTradeRoute (int iId)")
		.def("getNumTradeRoutes", &CyPlayer::getNumTradeRoutes, "int ()")
		.def("getTradeRouteByIndex", &CyPlayer::getTradeRouteByIndex, python::return_value_policy<python::manage_new_object>(), "CyTradeRoute (int iIndex)")
		.def("editTradeRoute", &CyPlayer::editTradeRoute, "void (int iId, int iSourceCityOwner, int iSourceCityId, int iDestinationCityOwner, int iDestinationCityId, int /*YieldTypes*/ eYield)")
		.def("canLoadYield", &CyPlayer::canLoadYield, "	bool canLoadYield(int /*PlayerTypes*/ eCityPlayer)")
		.def("canUnloadYield", &CyPlayer::canUnloadYield, "bool canUnloadYield(int /*PlayerTypes*/ eCityPlayer) ")
		.def("getYieldEquipmentAmount", &CyPlayer::getYieldEquipmentAmount, "int (int iProfession, int iYield)")
		.def("addRevolutionEuropeUnit", &CyPlayer::addRevolutionEuropeUnit, "void addRevolutionEuropeUnit(int /*UnitTypes*/ eUnit, int /*ProfessionTypes*/ eProfession)")
		.def("getNumTradeMessages", &CyPlayer::getNumTradeMessages, "int ()")
		.def("getTradeMessage", &CyPlayer::getTradeMessage, "string (int)")
		;
}
