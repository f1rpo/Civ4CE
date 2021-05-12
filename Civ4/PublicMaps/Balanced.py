#
#	FILE:	 Balanced.py
#	AUTHOR:  Andy Szybalski
#	PURPOSE: Global map script - Solid pangaea, balanced strategic resources.
#-----------------------------------------------------------------------------
#	Copyright (c) 2004, 2005 Firaxis Games, Inc. All rights reserved.
#-----------------------------------------------------------------------------
#

from CvPythonExtensions import *
import CvUtil
import random
import CvMapGeneratorUtil
import sys
from CvMapGeneratorUtil import HintedWorld
from CvMapGeneratorUtil import TerrainGenerator
from CvMapGeneratorUtil import FeatureGenerator

def getDescription():
	return "TXT_KEY_MAP_SCRIPT_BALANCED_DESCR"

resourcesToBalance = ('BONUS_ALUMINUM', 'BONUS_COAL', 'BONUS_COPPER', 'BONUS_HORSE', 'BONUS_IRON', 'BONUS_OIL', 'BONUS_URANIUM')
resourcesToEliminate = ('BONUS_MARBLE', 'BONUS_OIL')
	
def generatePlotTypes():
	NiTextOut("Setting Plot Types (Python Balanced) ...")
	global hinted_world
	hinted_world = HintedWorld(16,8)

	mapRand = CyGlobalContext().getGame().getMapRand()

	numBlocks = hinted_world.w * hinted_world.h
	numBlocksLand = int(numBlocks*0.25)
	cont = hinted_world.addContinent(numBlocksLand,mapRand.get(5, "Generate Plot Types PYTHON")+4,mapRand.get(3, "Generate Plot Types PYTHON")+2)
	if not cont:
		print "Couldn't create continent! Reverting to C implementation."
		CyPythonMgr().allowDefaultImpl()
	else:		
		for x in range(hinted_world.w):
			for y in (0, hinted_world.h - 1):
				hinted_world.setValue(x,y, 1) # force ocean at poles
		hinted_world.buildAllContinents()
		return hinted_world.generatePlotTypes(shift_plot_types=True)

def generateTerrainTypes():
	NiTextOut("Generating Terrain (Python Balanced) ...")
	terraingen = TerrainGenerator()
	terrainTypes = terraingen.generateTerrain()
	return terrainTypes

def addFeatures():
	NiTextOut("Adding Features (Python Balanced) ...")
	featuregen = FeatureGenerator()
	featuregen.addFeatures()
	return 0

def normalizeAddExtras():
	gc = CyGlobalContext()
	map = CyMap()
	for i in range(gc.getMAX_CIV_PLAYERS()):
		if (gc.getPlayer(i).isAlive()):
			start_plot = gc.getPlayer(i).getStartingPlot() # returns a CyPlot
			startx, starty = start_plot.getX(), start_plot.getY()
			
			plots = [] # build a list of the plots near the starting plot
			for dx in range(-5,6):
				for dy in range(-5,6):
					x,y = startx+dx, starty+dy
					pLoopPlot = map.plot(x,y)
					if not pLoopPlot.isNone():
						plots.append(pLoopPlot)
			
			resources_placed = []
			for pass_num in range(4):
				bIgnoreUniqueRange  = pass_num >= 1
				bIgnoreOneArea 		= pass_num >= 2
				bIgnoreAdjacent 	= pass_num >= 3
				
				for bonus in range(gc.getNumBonusInfos()):
					type_string = gc.getBonusInfo(bonus).getType()
					if (type_string not in resources_placed) and (type_string in resourcesToBalance):
						for (pLoopPlot) in plots:
							if (pLoopPlot.canHaveBonus(bonus, True)):
								if isBonusValid(bonus, pLoopPlot, bIgnoreUniqueRange, bIgnoreOneArea, bIgnoreAdjacent):
									pLoopPlot.setBonusType(bonus)
									resources_placed.append(type_string)
									#print "placed", type_string, "on pass", pass_num
									break # go to the next bonus

	CyPythonMgr().allowDefaultImpl()	# do the rest of the usual normalizeStartingPlots stuff, don't overrride
	
def addBonusType(argsList):
	[iBonusType] = argsList
	gc = CyGlobalContext()
	type_string = gc.getBonusInfo(iBonusType).getType()

	if (type_string in resourcesToBalance) or (type_string in resourcesToEliminate):
		return None # don't place any of this bonus randomly
	else:
		CyPythonMgr().allowDefaultImpl() # pretend we didn't implement this method, and let C handle this bonus in the default way
		

def isBonusValid(eBonus, pPlot, bIgnoreUniqueRange, bIgnoreOneArea, bIgnoreAdjacent):
	"Returns true if we can place a bonus here"
	map = CyMap()
	gc = CyGlobalContext()

	iX, iY = pPlot.getX(), pPlot.getY()

	if (not bIgnoreOneArea) and gc.getBonusInfo(eBonus).isOneArea():
		if map.getNumBonuses(eBonus) > 0:
			if map.getArea(pPlot.getArea()).getNumBonuses(eBonus) == 0:
				return False
				
	if not bIgnoreAdjacent:
		for iI in range(DirectionTypes.NUM_DIRECTION_TYPES):
			pLoopPlot = plotDirection(iX, iY, DirectionTypes(iI))
			if not pLoopPlot.isNone():
				if (pLoopPlot.getBonusType(-1) != -1) and (pLoopPlot.getBonusType(-1) != eBonus):
					return False

	if not bIgnoreUniqueRange:
		uniqueRange = gc.getBonusInfo(eBonus).getUniqueRange()
		for iDX in range(-uniqueRange, uniqueRange+1):
			for iDY in range(-uniqueRange, uniqueRange+1):
				pLoopPlot = plotXY(iX, iY, iDX, iDY)
				if not pLoopPlot.isNone() and pLoopPlot.getBonusType(-1) == eBonus:
					return False
	
	return True
