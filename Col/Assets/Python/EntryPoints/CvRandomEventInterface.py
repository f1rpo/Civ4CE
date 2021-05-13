# Sid Meier's Civilization 4
# Copyright Firaxis Games 2005
#
# CvRandomEventInterface.py
#
# These functions are App Entry Points from C++
# WARNING: These function names should not be changed
# WARNING: These functions can not be placed into a class
#
# No other modules should import this
#
import CvUtil
import CvScreensInterface
from CvPythonExtensions import *

gc = CyGlobalContext()
localText = CyTranslator()

def doEventEndTutorial(argsList):
	eEvent = argsList[0]
	pTriggeredData = argsList[1]
	player = gc.getPlayer(pTriggeredData.ePlayer)
	CyMessageControl().sendPlayerOption(PlayerOptionTypes.PLAYEROPTION_TUTORIAL, false)

def isExpiredFoundColony(argsList):
	eEvent = argsList[0]
	pTriggeredData = argsList[1]
	player = gc.getPlayer(pTriggeredData.ePlayer)
	if player.getNumCities() > 0:
		return true
	return false

def doEventCivilopediaSettlement(argsList):
	eEvent = argsList[0]
	pTriggeredData = argsList[1]
	CvScreensInterface.pediaShowHistorical((CivilopediaPageTypes.CIVILOPEDIA_PAGE_CONCEPT, gc.getInfoTypeForString("CONCEPT_SETTLEMENTS")))

def canDoTriggerImmigrant(argsList):
	pTriggeredData = argsList[0]
	player = gc.getPlayer(pTriggeredData.ePlayer)
	if player.getNumEuropeUnits() == 0:
		return false
	return true

def canDoTriggerImmigrantDone(argsList):
	pTriggeredData = argsList[0]
	player = gc.getPlayer(pTriggeredData.ePlayer)
	if player.getNumEuropeUnits() > 0:
		return false
	return true

def doEventCivilopediaEurope(argsList):
	eEvent = argsList[0]
	pTriggeredData = argsList[1]
	CvScreensInterface.pediaShowHistorical((CivilopediaPageTypes.CIVILOPEDIA_PAGE_CONCEPT, gc.getInfoTypeForString("CONCEPT_EUROPE")))

def doEventCivilopediaImmigration(argsList):
	eEvent = argsList[0]
	pTriggeredData = argsList[1]
	CvScreensInterface.pediaShowHistorical((CivilopediaPageTypes.CIVILOPEDIA_PAGE_CONCEPT, gc.getInfoTypeForString("CONCEPT_IMMIGRATION")))


def canDoTriggerMotherland(argsList):
	pTriggeredData = argsList[0]
	player = gc.getPlayer(pTriggeredData.ePlayer)
	(unit, iter) = player.firstUnit()
	while (unit):
		if unit.getDomainType() == DomainTypes.DOMAIN_SEA and unit.getUnitTravelState() == UnitTravelStates.UNIT_TRAVEL_STATE_TO_EUROPE and unit.getUnitTravelTimer() == 1:
			return true
		(unit, iter) = player.nextUnit(iter)
	return false

def doEventCivilopediaProfessions(argsList):
	eEvent = argsList[0]
	pTriggeredData = argsList[1]
	CvScreensInterface.pediaShowHistorical((CivilopediaPageTypes.CIVILOPEDIA_PAGE_CONCEPT, gc.getInfoTypeForString("CONCEPT_PROFESSIONS")))

def canDoTriggerPioneer(argsList):
	pTriggeredData = argsList[0]
	player = gc.getPlayer(pTriggeredData.ePlayer)

	improvementList = [gc.getInfoTypeForString("IMPROVEMENT_FARM"), gc.getInfoTypeForString("IMPROVEMENT_MINE"), gc.getInfoTypeForString("IMPROVEMENT_LODGE")]
	for iImprovement in improvementList:
		if player.getImprovementCount(iImprovement) > 0:
			return false

	ePioneer = gc.getInfoTypeForString("PROFESSION_PIONEER")
	(unit, iter) = player.firstUnit()
	while (unit):
		if unit.getProfession() == ePioneer:
			return false
		(unit, iter) = player.nextUnit(iter)

	return true

def canDoTriggerImproveLand(argsList):
	pTriggeredData = argsList[0]
	player = gc.getPlayer(pTriggeredData.ePlayer)

	improvementList = [gc.getInfoTypeForString("IMPROVEMENT_FARM"), gc.getInfoTypeForString("IMPROVEMENT_MINE"), gc.getInfoTypeForString("IMPROVEMENT_LODGE")]
	for iImprovement in improvementList:
		if player.getImprovementCount(iImprovement) > 0:
			return false

	ePioneer = gc.getInfoTypeForString("PROFESSION_PIONEER")
	(unit, iter) = player.firstUnit()
	while (unit):
		if unit.getProfession() == ePioneer:
			return true
		(unit, iter) = player.nextUnit(iter)

	return false

def doEventCivilopediaImproveLand(argsList):
	eEvent = argsList[0]
	pTriggeredData = argsList[1]
	CvScreensInterface.pediaShowHistorical((CivilopediaPageTypes.CIVILOPEDIA_PAGE_CONCEPT, gc.getInfoTypeForString("CONCEPT_IMPROVEMENTS")))

def canDoTriggerFoundingFather(argsList):
	pTriggeredData = argsList[0]
	player = gc.getPlayer(pTriggeredData.ePlayer)
	team = gc.getTeam(player.getTeam())

	for iFather in range(gc.getNumFatherInfos()):
		if (team.canConvinceFather(iFather)):
			return true
	return false

def doEventCivilopediaFoundingFather(argsList):
	eEvent = argsList[0]
	pTriggeredData = argsList[1]
	CvScreensInterface.pediaShowHistorical((CivilopediaPageTypes.CIVILOPEDIA_PAGE_CONCEPT, gc.getInfoTypeForString("CONCEPT_FATHERS")))

def doEventCivilopediaNativeVillages(argsList):
	eEvent = argsList[0]
	pTriggeredData = argsList[1]
	CvScreensInterface.pediaShowHistorical((CivilopediaPageTypes.CIVILOPEDIA_PAGE_CONCEPT, gc.getInfoTypeForString("CONCEPT_NATIVES")))

def canDoTriggerRevolution( argsList ):
	pTriggeredData = argsList[ 0 ]
	player = gc.getPlayer( pTriggeredData.ePlayer )

	if gc.getTeam(player.getTeam()).canDoRevolution():
		return true

	return false

def doEventCivilopediaRevolution(argsList):
	eEvent = argsList[0]
	pTriggeredData = argsList[1]
	CvScreensInterface.pediaShowHistorical((CivilopediaPageTypes.CIVILOPEDIA_PAGE_CONCEPT, gc.getInfoTypeForString("CONCEPT_LIBERTY")))

def canDoCityTriggerTools(argsList):
	eTrigger = argsList[0]
	ePlayer = argsList[1]
	iCityId = argsList[2]
	player = gc.getPlayer(ePlayer)
	city = player.getCity(iCityId)

	if (not city.isNone() and city.getYieldRate(gc.getInfoTypeForString("YIELD_TOOLS")) > 0):
		return true

	return false

def doEventCivilopediaTools(argsList):
	eEvent = argsList[0]
	pTriggeredData = argsList[1]
	CvScreensInterface.pediaShowHistorical((CivilopediaPageTypes.CIVILOPEDIA_PAGE_CONCEPT, gc.getInfoTypeForString("CONCEPT_PROFESSIONS")))

def canDoCityTriggerBuildingRequiresTools(argsList):
	eTrigger = argsList[0]
	ePlayer = argsList[1]
	iCityId = argsList[2]
	player = gc.getPlayer(ePlayer)
	city = player.getCity(iCityId)
	building = city.getProductionBuilding()

	if building != BuildingTypes.NO_BUILDING:
		if (gc.getBuildingInfo(building).getYieldCost(gc.getInfoTypeForString("YIELD_TOOLS")) > 0):
			return true

	return false

def doEventCivilopediaAutomatedTools(argsList):
	eEvent = argsList[0]
	pTriggeredData = argsList[1]
	CvScreensInterface.pediaShowHistorical((CivilopediaPageTypes.CIVILOPEDIA_PAGE_CONCEPT, gc.getInfoTypeForString("CONCEPT_TRADE")))

def canDoSpeakToChief(argsList):
	pTriggeredData = argsList[0]
	player = gc.getPlayer(pTriggeredData.ePlayer)	

	bFoundNative = false
	for iPlayer in range(gc.getMAX_PLAYERS()):
		loopPlayer = gc.getPlayer(iPlayer)
		if loopPlayer.isAlive() and loopPlayer.isNative():
			bFoundNative = true
			(city, iter) = loopPlayer.firstCity(true)
			while(city):
				if city.isScoutVisited(player.getTeam()):
					return false
				(city, iter) = loopPlayer.nextCity(iter, true)
	
	return bFoundNative
	
def canDoSpeakToChiefCompleted(argsList):
	pTriggeredData = argsList[0]
	player = gc.getPlayer(pTriggeredData.ePlayer)	

	for iPlayer in range(gc.getMAX_PLAYERS()):
		loopPlayer = gc.getPlayer(iPlayer)
		if loopPlayer.isAlive() and loopPlayer.isNative():
			(city, iter) = loopPlayer.firstCity(true)
			while(city):
				if city.isScoutVisited(player.getTeam()):
					return true
				(city, iter) = loopPlayer.nextCity(iter, true)
	
	return false
	
def doEventCivilopediaWar(argsList):
	eEvent = argsList[0]
	pTriggeredData = argsList[1]
	CvScreensInterface.pediaShowHistorical((CivilopediaPageTypes.CIVILOPEDIA_PAGE_CONCEPT, gc.getInfoTypeForString("CONCEPT_WAR")))

def canCityTriggerDoOverstock(argsList):
	eTrigger = argsList[0]
	ePlayer = argsList[1]
	iCityId = argsList[2]
	player = gc.getPlayer(ePlayer)
	city = player.getCity(iCityId)

	for i in range(YieldTypes.NUM_YIELD_TYPES):
		if (not city.isNone() and city.getYieldStored(i) > city.getMaxYieldCapacity() and i != gc.getInfoTypeForString("YIELD_FOOD")):
			return true

	return false
	
def canDoTaxes(argsList):
	pTriggeredData = argsList[0]
	player = gc.getPlayer(pTriggeredData.ePlayer)	

	if player.getTaxRate() > 0:
		return true
		
	return false