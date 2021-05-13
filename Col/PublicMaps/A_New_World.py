from CvPythonExtensions import *
import CvUtil
import CvMapGeneratorUtil
from CvMapGeneratorUtil import FractalWorld
from CvMapGeneratorUtil import TerrainGenerator
from CvMapGeneratorUtil import FeatureGenerator

def getDescription():
	return "TXT_KEY_MAP_SCRIPT_ANEWWORLD_DESCR"
	
def isAdvancedMap():
	"This map should show up in simple mode"
	return 0

def getWrapX():
	return false
	
def getWrapY():
	return false

def generatePlotTypes():
	NiTextOut("Setting Plot Types (Python Fractal) ...")
	fractal_world = FractalWorld( 4, 4 )
	fractal_world.initFractal(continent_grain = 3, rift_grain = 4, has_center_rift = False, polar = True)
	retVal = fractal_world.generatePlotTypes(70)
	sinkEasternShore(fractal_world, 4)
	return retVal

def generateTerrainTypes():
	NiTextOut("Generating Terrain (Python Fractal) ...")
	terraingen = TerrainGenerator(grain_amount=6)
	terrainTypes = terraingen.generateTerrain()
	return terrainTypes

def addFeatures():
	NiTextOut("Adding Features (Python Fractal) ...")
	featuregen = FeatureGenerator()
	featuregen.addFeatures()
	return 0
	
def sinkEasternShore(fractal_world, iWidth):
	for x in range(iWidth):
		for y in range(fractal_world.iNumPlotsY):
			i = y * fractal_world.iNumPlotsX + fractal_world.iNumPlotsX - 1 - x 
			fractal_world.plotTypes[i] = PlotTypes.PLOT_OCEAN
				
	return 0
