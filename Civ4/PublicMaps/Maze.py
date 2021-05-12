#
#	FILE:	 Maze.py
#	AUTHOR:  Bob Thomas (Sirian)
#	PURPOSE: Regional map script - Creates a land/sea maze.
#-----------------------------------------------------------------------------
#	Copyright (c) 2005 Firaxis Games, Inc. All rights reserved.
#-----------------------------------------------------------------------------
#

from CvPythonExtensions import *
import CvUtil
import CvMapGeneratorUtil
from CvMapGeneratorUtil import FractalWorld
from CvMapGeneratorUtil import TerrainGenerator
from CvMapGeneratorUtil import FeatureGenerator

def getDescription():
	return "TXT_KEY_MAP_SCRIPT_MAZE_DESCR"

def getNumCustomMapOptions():
	return 1
	
def getCustomMapOptionName(argsList):
	translated_text = unicode(CyTranslator().getText("TXT_KEY_MAP_SCRIPT_MAZE_WIDTH", ()))
	return translated_text

def getNumCustomMapOptionValues(argsList):
	return 5
	
def getCustomMapOptionDescAt(argsList):
	iSelection = argsList[1]
	selection_names = ["TXT_KEY_MAP_SCRIPT_1_PLOT_WIDE",
	                   "TXT_KEY_MAP_SCRIPT_2_PLOTS_WIDE",
	                   "TXT_KEY_MAP_SCRIPT_3_PLOTS_WIDE",
	                   "TXT_KEY_MAP_SCRIPT_4_PLOTS_WIDE",
	                   "TXT_KEY_MAP_SCRIPT_5_PLOTS_WIDE"]
	translated_text = unicode(CyTranslator().getText(selection_names[iSelection], ()))
	return translated_text
	
def getCustomMapOptionDefault(argsList):
	return 2

def isAdvancedMap():
	"This map should not show up in simple mode"
	return 1

def isSeaLevelMap():
	return 0

def startHumansOnSameTile():
	return True

def getGridSize(argsList):
	# Reduce grid sizes. Note, nonstandard reductions!
	grid_sizes = {
		WorldSizeTypes.WORLDSIZE_DUEL:		(6,4),
		WorldSizeTypes.WORLDSIZE_TINY:		(9,4),
		WorldSizeTypes.WORLDSIZE_SMALL:		(10,6),
		WorldSizeTypes.WORLDSIZE_STANDARD:	(14,8),
		WorldSizeTypes.WORLDSIZE_LARGE:		(18,10),
		WorldSizeTypes.WORLDSIZE_HUGE:		(24,14)
	}

	if (argsList[0] == -1): # (-1,) is passed to function on loads
		return []
	[eWorldSize] = argsList
	return grid_sizes[eWorldSize]

def generatePlotTypes():
	NiTextOut("Setting Plot Types (Python Maze) ...")
	gc = CyGlobalContext()
	map = CyMap()
	dice = gc.getGame().getMapRand()
	iW = map.getGridWidth()
	iH = map.getGridHeight()

	# Get user input
	userInputMazeWidth = map.getCustomMapOption(0)
	multiplier = 1 + userInputMazeWidth

	# Set peak percentage by maze width:
	if userInputMazeWidth > 1:
		extraPeaks = 5 - userInputMazeWidth
	else:
		extraPeaks = 0 

	# Varying grains for reducing "clumping" of hills/peaks on larger maps.
	sizekey = map.getWorldSize()
	grainvalues = {
		WorldSizeTypes.WORLDSIZE_DUEL:		3,
		WorldSizeTypes.WORLDSIZE_TINY:		3,
		WorldSizeTypes.WORLDSIZE_SMALL:		4,
		WorldSizeTypes.WORLDSIZE_STANDARD:	4,
		WorldSizeTypes.WORLDSIZE_LARGE:		5,
		WorldSizeTypes.WORLDSIZE_HUGE:		6
		}
	grain_amount = grainvalues[sizekey]

	# Init fractal for distribution of Hills plots.
	hillsFrac = CyFractal()
	peaksFrac = CyFractal()
	hillsFrac.fracInit(iW, iH, grain_amount, dice, 0, -1, -1)
	peaksFrac.fracInit(iW, iH, grain_amount + 1, dice, 0, -1, -1)
	iHillsBottom1 = hillsFrac.getHeightFromPercent(max((25 - gc.getClimateInfo(map.getClimate()).getHillRange()), 0))
	iHillsTop1 = hillsFrac.getHeightFromPercent(min((25 + gc.getClimateInfo(map.getClimate()).getHillRange()), 100))
	iHillsBottom2 = hillsFrac.getHeightFromPercent(max((75 - gc.getClimateInfo(map.getClimate()).getHillRange()), 0))
	iHillsTop2 = hillsFrac.getHeightFromPercent(min((75 + gc.getClimateInfo(map.getClimate()).getHillRange()), 100))
	iPeakThreshold = peaksFrac.getHeightFromPercent((10 * extraPeaks) + gc.getClimateInfo(map.getClimate()).getPeakPercent())
	
	# Set maze dimensions
	mazeW = iW/(2 * multiplier)
	mazeH = iH/(2 * multiplier)
	
	# Init Maze
	plotTypes = [PlotTypes.PLOT_OCEAN] * (iW*iH)
	matrix = [False] * (mazeW*mazeH)
	path = []
	remainingSegments = mazeW*mazeH - 1
	iX = dice.get(mazeW, "Starting X - Maze PYTHON")
	iY = dice.get(mazeH, "Starting Y - Maze PYTHON")
	directions = 4
	if iX == 0 or iX == mazeW - 1:
		directions -= 1
	if iY == 0 or iY == mazeH - 1:
		directions -= 1
	
	# Add land at initial vertex.
	x = iX * 2 * multiplier
	y = iY * 2 * multiplier
	i = y*iW + x
	hillVal = hillsFrac.getHeight(x,y)
	if ((hillVal >= iHillsBottom1 and hillVal <= iHillsTop1) or (hillVal >= iHillsBottom2 and hillVal <= iHillsTop2)):
		plotTypes[i] = PlotTypes.PLOT_HILLS
	else:
		plotTypes[i] = PlotTypes.PLOT_LAND
	
	if multiplier == 1: pass
	else:
		for mazeX in range(x, x+multiplier):
			for mazeY in range(y, y+multiplier):
				i = mazeY*iW + mazeX
				hillVal = hillsFrac.getHeight(mazeX,mazeY)
				if ((hillVal >= iHillsBottom1 and hillVal <= iHillsTop1) or (hillVal >= iHillsBottom2 and hillVal <= iHillsTop2)):
					peakVal = peaksFrac.getHeight(mazeX,mazeY)
					if (peakVal <= iPeakThreshold):
						plotTypes[i] = PlotTypes.PLOT_PEAK
					else:
						plotTypes[i] = PlotTypes.PLOT_HILLS
				else:
					plotTypes[i] = PlotTypes.PLOT_LAND

	# Add Segments
	while remainingSegments:
		remainingSegments -= 1
		matrixIndex = mazeW*iY + iX
		matrix[matrixIndex] = True
		# Count number of valid possible paths from this vertex.
		# North
		if iY == mazeH - 1:
			north = 0
		elif matrix[matrixIndex + mazeW] == True:
			north = 0
		else:
			north = 1
		# South
		if iY == 0:
			south = 0
		elif matrix[matrixIndex - mazeW] == True:
			south = 0
		else:
			south = 1
		# East
		if iX == mazeW - 1:
			east = 0
		elif matrix[matrixIndex + 1] == True:
			east = 0
		else:
			east = 1
		# West
		if iX == 0:
			west = 0
		elif matrix[matrixIndex - 1] == True:
			west = 0
		else:
			west = 1
		
		# Possible Directions
		directions = north + south + east + west
		# Remember this vertex for possible return to it later
		if directions > 1:
			path.append([iX, iY])
			
		# If no Directions possible, must choose another vertex.
		while directions < 1:
			vertexRoll = dice.get(len(path), "Pathfinding - Maze PYTHON")
			[iX, iY] = path[vertexRoll]
			matrixIndex = mazeW*iY + iX
			# Count number of valid possible paths from replacement vertex.
			# North
			if iY == mazeH - 1:
				north = 0
			elif matrix[matrixIndex + mazeW] == True:
				north = 0
			else:
				north = 1
			# South
			if iY == 0:
				south = 0
			elif matrix[matrixIndex - mazeW] == True:
				south = 0
			else:
				south = 1
			# East
			if iX == mazeW - 1:
				east = 0
			elif matrix[matrixIndex + 1] == True:
				east = 0
			else:
				east = 1
			# West
			if iX == 0:
				west = 0
			elif matrix[matrixIndex - 1] == True:
				west = 0
			else:
				west = 1
			# Possible Directions
			directions = north + south + east + west
			# Remove this vertex if no longer valid
			if directions < 2:
				del path[vertexRoll]
		
		# Choose a direction at random.
		choose = []
		if north: choose.append([0, 1])
		if south: choose.append([0, -1])
		if east: choose.append([1, 0])
		if west: choose.append([-1, 0])
		dir = dice.get(len(choose), "Segment Direction - Maze PYTHON")
		[xPlus, yPlus] = choose[dir]
		
		# Add land in the chosen direction.
		for loop in range(1, 3):
			x = (iX * 2 * multiplier) + (multiplier * xPlus * loop)
			y = (iY * 2 * multiplier) + (multiplier * yPlus * loop)
			i = y*iW + x
			hillVal = hillsFrac.getHeight(x,y)
			if ((hillVal >= iHillsBottom1 and hillVal <= iHillsTop1) or (hillVal >= iHillsBottom2 and hillVal <= iHillsTop2)):
				plotTypes[i] = PlotTypes.PLOT_HILLS
			else:
				plotTypes[i] = PlotTypes.PLOT_LAND
	
			if multiplier == 1: pass
			else:
				for mazeX in range(x, x+multiplier):
					for mazeY in range(y, y+multiplier):
						i = mazeY*iW + mazeX
						hillVal = hillsFrac.getHeight(mazeX,mazeY)
						if ((hillVal >= iHillsBottom1 and hillVal <= iHillsTop1) or (hillVal >= iHillsBottom2 and hillVal <= iHillsTop2)):
							peakVal = peaksFrac.getHeight(mazeX,mazeY)
							if (peakVal <= iPeakThreshold):
								plotTypes[i] = PlotTypes.PLOT_PEAK
							else:
								plotTypes[i] = PlotTypes.PLOT_HILLS
						else:
							plotTypes[i] = PlotTypes.PLOT_LAND

		iX += xPlus
		iY += yPlus
		
	# Finished generating the maze!
	return plotTypes
	
def generateTerrainTypes():
	NiTextOut("Generating Terrain (Python Maze) ...")
	terraingen = TerrainGenerator()
	terrainTypes = terraingen.generateTerrain()
	return terrainTypes

def addFeatures():
	# Remove all peaks along the coasts, before adding Features, Bonuses, Goodies, etc.
	map = CyMap()
	iW = map.getGridWidth()
	iH = map.getGridHeight()
	for plotIndex in range(iW * iH):
		pPlot = map.plotByIndex(plotIndex)
		if pPlot.isPeak() and pPlot.isCoastalLand():
			# If a peak is along the coast, change to hills and recalc.
			pPlot.setPlotType(PlotTypes.PLOT_HILLS, true, true)
			
	# Now add the features.
	NiTextOut("Adding Features (Python Maze) ...")
	featuregen = FeatureGenerator()
	featuregen.addFeatures()
	return 0

def normalizeRemovePeaks():
	return None
