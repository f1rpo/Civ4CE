## Sid Meier's Civilization 4
## Copyright Firaxis Games 2005
from CvPythonExtensions import *
import CvWBPopups
import CvUtil
import CvWBDesc

# globals
WBDesc = CvWBDesc.CvWBDesc()
lastFileRead = None
gc = CyGlobalContext()

def writeDesc(argsList):
	"Save out a high-level desc of the world, for WorldBuilder"
	fileName = argsList[0]
	lastFileRead=None
	return WBDesc.write(fileName)
	
def readAndApplyDesc(argsList):
	"Read in and apply a high-level desc of the world.  In-game load only"
	fileName = argsList[0]
	if WBDesc.read(fileName) < 0:
		return -1
	if (WBDesc.applyMap() < 0):
		return -1
	return WBDesc.applyInitialItems()

def readDesc(argsList):
	"Read in a high-level desc of the world, for WorldBuilder.  Must call applyMap and applyInitialItems to finish the process"
	global lastFileRead
	fileName = argsList[0]
	if (fileName!=lastFileRead):
		ret=WBDesc.read(fileName)
		if (ret==0):
			lastFileRead=fileName
	else:
		ret=0		
	return ret
		
def applyMapDesc():	
	"After reading, applies the map loaded data"
	return WBDesc.applyMap()

def applyInitialItems():	
	"After reading, applies player units, cities, and techs"
	return WBDesc.applyInitialItems()

def getAssignedStartingPlots():
	"Reads in starting plots for random players"
	return WBDesc.getAssignedStartingPlots()

def initWBEditor(argsList):
	"Called from the Worldbuilder app - sends to CvWBPopups for handling"
	return CvWBPopups.CvWBPopups().initWB(argsList)
	
def getGameData():
	"after reading a save file, return game/player data as a tuple"
	t=()
	gameTurn = WBDesc.gameDesc.gameTurn
	maxTurns = WBDesc.gameDesc.maxTurns
	maxCityElimination = WBDesc.gameDesc.maxCityElimination
	targetScore = WBDesc.gameDesc.targetScore
	worldSizeType = CvUtil.findInfoTypeNum(gc.getWorldInfo, gc.getNumWorldInfos(), WBDesc.mapDesc.worldSize)
	climateType = CvUtil.findInfoTypeNum(gc.getClimateInfo, gc.getNumClimateInfos(), WBDesc.mapDesc.climate)
	seaLevelType = CvUtil.findInfoTypeNum(gc.getSeaLevelInfo, gc.getNumSeaLevelInfos(), WBDesc.mapDesc.seaLevel)
	eraType = CvUtil.findInfoTypeNum(gc.getEraInfo, gc.getNumEraInfos(), WBDesc.gameDesc.eraType)
	gameSpeedType = CvUtil.findInfoTypeNum(gc.getGameSpeedInfo, gc.getNumGameSpeedInfos(), WBDesc.gameDesc.speedType)
	calendarType = CvUtil.findInfoTypeNum(gc.getCalendarInfo, gc.getNumCalendarInfos(), WBDesc.gameDesc.calendarType)
	
	t=t+(worldSizeType,)
	t=t+(climateType,)
	t=t+(seaLevelType,)
	t=t+(eraType,)
	t=t+(gameSpeedType,)
	t=t+(calendarType,)
	
	t=t+(len(WBDesc.gameDesc.options),)
	for i in range(len(WBDesc.gameDesc.options)):
		option = CvUtil.findInfoTypeNum(gc.getGameOptionInfo, gc.getNumGameOptionInfos(), WBDesc.gameDesc.options[i])
		t=t+(option,)
	
	t=t+(len(WBDesc.gameDesc.mpOptions),)
	for i in range(len(WBDesc.gameDesc.mpOptions)):
		mpOption = CvUtil.findInfoTypeNum(gc.getMPOptionInfo, gc.getNumMPOptionInfos(), WBDesc.gameDesc.mpOptions[i])
		t=t+(mpOption,)
	
	t=t+(len(WBDesc.gameDesc.forceControls),)
	for i in range(len(WBDesc.gameDesc.forceControls)):
		forceControl = CvUtil.findInfoTypeNum(gc.getForceControlInfo, gc.getNumForceControlInfos(), WBDesc.gameDesc.forceControls[i])
		t=t+(forceControl,)
	
	t=t+(len(WBDesc.gameDesc.victories),)
	for i in range(len(WBDesc.gameDesc.victories)):
		victory = CvUtil.findInfoTypeNum(gc.getVictoryInfo, gc.getNumVictoryInfos(), WBDesc.gameDesc.victories[i])
		t=t+(victory,)
	
	t=t+(gameTurn,)
	t=t+(maxTurns,)
	t=t+(maxCityElimination,)
	t=t+(targetScore,)
	
	return t
	
def getModPath():
	"Returns the path for the Mod that this scenario should load (if applicable)"
	return (WBDesc.gameDesc.szModPath)
	
def getMapDescriptionKey():
	"Returns the TXT_KEY Description of the map to be displayed in the map/mod selection screen"
	return (WBDesc.gameDesc.szDescription)
	
def getPlayerData():
	"after reading a save file, return player data as a tuple, terminated by -1"
	t=()
	for i in range(gc.getMAX_CIV_PLAYERS()):
#		print "leaderType"
		leaderType = CvUtil.findInfoTypeNum(gc.getLeaderHeadInfo, gc.getNumLeaderHeadInfos(), WBDesc.playersDesc[i].leaderType)
#		print "civType"
		civType = CvUtil.findInfoTypeNum(gc.getCivilizationInfo, gc.getNumCivilizationInfos(), WBDesc.playersDesc[i].civType)
#		print "handicapType"
		handicapType = CvUtil.findInfoTypeNum(gc.getHandicapInfo, gc.getNumHandicapInfos(), WBDesc.playersDesc[i].handicap)
#		print "color"
		color = CvUtil.findInfoTypeNum(gc.getPlayerColorInfo, gc.getNumPlayerColorInfos(), WBDesc.playersDesc[i].color)
#		print "artStyle"
		artStyle = gc.getTypesEnum(WBDesc.playersDesc[i].artStyle)
#		print "done"
		
		t=t+(civType,)
		t=t+(WBDesc.playersDesc[i].isPlayableCiv,)
		t=t+(leaderType,)
		t=t+(handicapType,)
		t=t+(WBDesc.playersDesc[i].team,)
		t=t+(color,)
		t=t+(artStyle,)
		t=t+(WBDesc.playersDesc[i].isMinorNationCiv,)
		t=t+(WBDesc.playersDesc[i].isWhiteFlag,)
				
	return t
		
def getPlayerDesc():
	"after reading a save file, return player description data (wide strings) as a tuple"
	t=()
	for i in range(gc.getMAX_CIV_PLAYERS()):
		t=t+(WBDesc.playersDesc[i].szCivDesc,)
		t=t+(WBDesc.playersDesc[i].szCivShortDesc,)
		t=t+(WBDesc.playersDesc[i].szLeaderName,)
		t=t+(WBDesc.playersDesc[i].szCivAdjective,)
		t=t+(WBDesc.playersDesc[i].szFlagDecal,)
		
	return t
