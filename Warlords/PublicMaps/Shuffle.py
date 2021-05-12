#
#	FILE:	 Shuffle.py
#	AUTHOR:  Bob Thomas (Sirian)
#	PURPOSE: Global map script - Return of Civ3's "random" map option.
#-----------------------------------------------------------------------------
#	Copyright (c) 2005, 2006 Firaxis Games, Inc. All rights reserved.
#-----------------------------------------------------------------------------
#

from CvPythonExtensions import *
import CvUtil
import CvMapGeneratorUtil
from CvMapGeneratorUtil import FractalWorld
from CvMapGeneratorUtil import TerrainGenerator
from CvMapGeneratorUtil import FeatureGenerator

'''
This map script type added by popular demand.
The function is very similar to what "random" map type would provide in Civ3.

- Bob Thomas	October 30, 2005 // January 10, 2006
'''

def getDescription():
	return "TXT_KEY_MAP_SCRIPT_SHUFFLE_DESCR"
	
def isAdvancedMap():
	"This map should show up in simple mode"
	return 0

def generatePlotTypes():
	NiTextOut("Setting Plot Types (Python Shuffle) ...")
	gc = CyGlobalContext()
	dice = gc.getGame().getMapRand()
	fractal_world = FractalWorld()
	grainRoll = 1 + dice.get(8, "Fractal Grain - Shuffle PYTHON")
	if grainRoll > 5: grainRoll -= 5
	if grainRoll == 2:
		fractal_world.initFractal(polar = True)
		return fractal_world.generatePlotTypes(water_percent=75)
	else:
		if grainRoll == 5: grainRoll -= 3
		fractal_world.initFractal(continent_grain = grainRoll, rift_grain = -1, has_center_rift = False, polar = True)
		return fractal_world.generatePlotTypes()

def generateTerrainTypes():
	NiTextOut("Generating Terrain (Python Shuffle) ...")
	terraingen = TerrainGenerator()
	terrainTypes = terraingen.generateTerrain()
	return terrainTypes

def addFeatures():
	NiTextOut("Adding Features (Python Shuffle) ...")
	featuregen = FeatureGenerator()
	featuregen.addFeatures()
	return 0
