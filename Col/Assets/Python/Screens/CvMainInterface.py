## Sid Meier's Civilization 4
## Copyright Firaxis Games 2005
from CvPythonExtensions import *
import CvUtil
import ScreenInput
import CvScreenEnums
import CvEventInterface
import time
import math

# GLOBAL SYSTEM SHORTCUTS
gc = CyGlobalContext()
ArtFileMgr = CyArtFileMgr()
localText = CyTranslator()

# GLOBAL INFOCLASS COUNTS
g_NumEmphasizeInfos = 0
g_NumHurryInfos = 0
g_NumActionInfos = 0
g_eEndTurnButtonState = -1

# GLOBAL INITIALIZATION BOOLEAN
INITIALIZED = false

# GLOBAL RESOLUTION VALUES
xResolution = -1
yResolution = -1

# GLOBAL INTERFACE FUNDAMENTAL CONSTRUCTS
FLAG_WIDTH = -1
FLAG_PERCENT_WIDTH = 8

MINI_MAP_WIDTH = -1
MINI_MAP_PERCENT_WIDTH = 30

LOWER_RIGHT_CORNER_BACKGROUND_WIDTH = MINI_MAP_WIDTH

SADDLE_HEIGHT = -1
SADDLE_HEIGHT_PERCENT = 31

LOWER_LEFT_CORNER_BACKGROUND_WIDTH = -1
LOWER_LEFT_CORNER_BACKGROUND_PERCENT_WIDTH = 22

LOWER_LEFT_CORNER_BACKGROUND_HEIGHT = -1
LOWER_LEFT_CORNER_BACKGROUND_PERCENT_HEIGHT = 25

LOWER_RIGHT_CORNER_BACKGROUND_WIDTH = -1
LOWER_RIGHT_CORNER_BACKGROUND_WIDTH_PERCENT = 19

LOWER_RIGHT_CORNER_BACKGROUND_HEIGHT = -1
LOWER_RIGHT_CORNER_BACKGROUND_HEIGHT_PERCENT = 10

BOTTOM_CENTER_HUD_HEIGHT = -1
BOTTOM_CENTER_HUD_PERCENT_HEIGHT = 15

TOP_CORNER_HUD_WIDTHS = -1
TOP_CORNER_HUD_PERCENT_WIDTH = 20

TOP_CORNER_HUD_HEIGHT = -1
TOP_CORNER_HUD_PERCENT_HEIGHT = 8

TOP_CENTER_HUD_HEIGHT = -1
TOP_CENTER_HUD_PERCENT_HEIGHT = 8

TOP_CENTER_HUD_WIDTH = -1
TOP_CENTER_HUD_PERCENT_WIDTH = 45

CITIZEN_BAR_HEIGHT = -1
CITIZEN_BAR_PERCENT_HEIGHT = 10

CITY_TITLE_BAR_HEIGHT = -1
CITY_TITLE_BAR_PERCENT_HEIGHT = 5

TRANSPORT_AREA_HEIGHT = -1

CITIZEN_BAR_WIDTH = -1
CITIZEN_BAR_WIDTH_PERCENT = 80

STACK_BAR_HEIGHT = -1
STACK_BAR_PERCENT_HEIGHT = 2.7

EJECT_AREA_WIDTH = -1
EJECT_AREA_PERCENT_WIDTH = 6

MINI_MAP_RADIUS = -1
MINI_MAP_PERCENT_RADIUS = 31

LEFT_PLOT_LIST_EDGE = -1
LEFT_PLOT_LIST_PERCENT_EDGE = 40

RIGHT_PLOT_LIST_EDGE = -1
RIGHT_PLOT_LIST_PERCENT_EDGE = 25

MAIN_LARGE_PORTRAIT_HEIGHT = -1
MAIN_LARGE_PORTRAIT_PERCENT_HEIGHT = 22

MAIN_LARGE_PORTRAIT_WIDTH = -1
MAIL_LARGE_PORTRAIT_PERCENT_WIDTH = 10

UNIT_PORTRAIT_HEIGHT = -1
UNIT_PORTRAIT_PERCENT_HEIGHT = 15

UNIT_HEALTH_BAR_WIDTH = -1
UNIT_HEALHT_BAR_PERCENT_WIDTH = 17

CITY_VIEW_BOX_HEIGHT_AND_WIDTH = -1
CITY_VIEW_BOX_PERCENT_HEIGHT = 38
CITY_VIEW_BOX_MODIFIED_PERCENT_HEIGHT_AND_WIDTH = -1

CITY_MULTI_TAB_AREA_HEIGHT = -1
CITY_MULTI_TAB_SIZE = -1

MAP_EDGE_MARGIN_WIDTH = -1
MAP_EDGE_MARGIN_PERCENT_WIDTH = 0.7

BUILD_AREA_WIDTH = -1
BUILD_AREA_HEIGHT = -1

SMALL_BUTTON_SIZE = -1
MEDIUM_BUTTON_SIZE = -1
LARGE_BUTTON_SIZE = -1
STANDARD_FONT_SIZE = -1

ASPECT_RATIO = -1
ASPECT_ADJUSTMENT = -1

RESOURCE_TABLE_COLUMN_WIDTH = -1

AVOID_GROWTH = -1

HELPTEXT_AREA_Y_MARGIN = 8
HELTTEXT_AREA_X_MARGIN = 7
HELPTEXT_AREA_MAX_WIDTH = 400
HELPTEXT_AREA_MIN_WIDTH = 150

# CITY BUILDINGS GRID MAP
BUILDING_DATA = ["", "", "", "", "", "", "", "", "", "", "", "", "", "", ""]

BUILDING_DATA[0] = [40, 33, 33, 18] #Bells
BUILDING_DATA[1] = [60, 42, 33, 18] #Hammers
BUILDING_DATA[2] = [40, 66, 33, 18] #Coats
BUILDING_DATA[3] = [0, 33, 33, 18] #Cloth
BUILDING_DATA[4] = [80, 0, 33, 18] #Rum
BUILDING_DATA[5] = [20, 42, 33, 18] #Cigars
BUILDING_DATA[6] = [60, 8, 33, 18] #Tools
BUILDING_DATA[7] = [40, 0, 33, 18] #Crosses
BUILDING_DATA[8] = [0, 66, 33, 18] #Muskets
BUILDING_DATA[9] = [0, 0, 33, 18] #Horses
BUILDING_DATA[10] = [80, 33, 33, 18] #Dock
BUILDING_DATA[11] = [60, 74, 33, 18] #WareHouse
BUILDING_DATA[12] = [20, 8, 33, 18] #Education
BUILDING_DATA[13] = [80, 74, 33, 18] #Wall
BUILDING_DATA[14] = [20, 74, 33, 18] #Print


BUILDING_GRID = ["", "", "", "", "", "", "", "", "", "", "", "", "", "", ""]

BUILDING_AREA_WIDTH = -1
BUILDING_AREA_HEIGHT = -1

# PANEL TOGGLE CONTROLS
BUILDING_MANAGMENT_TOGGLE = 1000
AUTOMATION_MANAGMENT_TOGGLE = 1001
MAP_MANAGMENT_TOGGLE = 1002
GO_TO_CITY = 1003

BUILDING_MANAGMENT_PANEL_UP = True
BUILDING_CHANGE_MANAGMENT_PANEL_UP = False
SHIP_MANAGMENT_PANEL_UP = False
AUTOMATION_MANAGMENT_PANEL_UP = False
MAP_MANAGMENT_PANEL_UP = False
GARRISON_MANAGMENT_PANEL_UP = False

# HIDE TYPE
HIDE_TYPE_CITY = 0
HIDE_TYPE_MAP = 1
HIDE_TYPE_GLOBAL = 2
HIDE_TYPE_ADVANCED_START = 3
HIDE_TYPE_MINI_MAP = 4
NUM_HIDE_TYPES = 5

# HIDE LEVEL
HIDE_LEVEL_OFF = 0
HIDE_LEVEL_ALL = 1
HIDE_LEVEL_HIDE = 2
HIDE_LEVEL_NORMAL = 3
HIDE_LEVEL_UBER = 4
NUM_HIDE_LEVELS = 5

# BONUS HIDE GROUPS
RESOURCE_TABLE_HIDE = (NUM_HIDE_TYPES * NUM_HIDE_LEVELS) + 1
CITIZEN_HIDE = (NUM_HIDE_TYPES * NUM_HIDE_LEVELS) + 2
ACTION_BUTTON_HIDE = (NUM_HIDE_TYPES * NUM_HIDE_LEVELS) + 3

# EMPHASIZE DATA
EMPHASIZEYIELDS = []

# INIT MASTER HIDE LIST
MasterHideList = []
for i in range(NUM_HIDE_TYPES):
	SubList = []
	for j in range(NUM_HIDE_LEVELS):
		SubSubList = []
		SubList.append(SubSubList)
	MasterHideList.append(SubList)
	
# PLOT LIST BUTTONS
NUM_PLOT_LIST_BUTTONS = 1

# GLOBE LAYER OPTION POSITIONING
iGlobeLayerOptionsY_Minimal = 38 # distance from bottom edge
iGlobeLayerOptionHeight = 24

g_szTimeText = ""
g_iTimeTextCounter = 0
g_pSelectedUnit = 0

class CvMainInterface:
	"Main Interface Screen"

	def numPlotListButtons( self ):
		return NUM_PLOT_LIST_BUTTONS

	# Adds the Widget to a list for hiding and unhiding as the screen updates
	def appendtoHideState( self, screen, Name, Type, Level ):
		global MasterHideList
		screen.hide(Name)
		MasterHideList[Type][Level].append(Name)

	# Shows Widgets of the specified Type at and above a particular Hiding level
	def SetHideState( self, screen, Type, Level ):

		for T in range(NUM_HIDE_TYPES):
			if T != Type:
				for L in range(NUM_HIDE_LEVELS):
					List = (T * NUM_HIDE_LEVELS) +  L
					screen.hideList(List)

		for T in range(NUM_HIDE_TYPES):
			if T == Type:
				for L in range(NUM_HIDE_LEVELS):
					if (L > Level):
						List = (T * NUM_HIDE_LEVELS) +  L
						screen.hideList(List)
					else:
						List = (T * NUM_HIDE_LEVELS) +  L
						screen.showList(List)

	def SetHideLists( self, screen ):
		for T in range(NUM_HIDE_TYPES):
			for L in range(NUM_HIDE_TYPES):
				HideIndex = (T * NUM_HIDE_LEVELS) + L
				screen.registerHideList(MasterHideList[T][L], len(MasterHideList[T][L]), HideIndex)
				
		screen.registerHideList([], 0, NUM_HIDE_TYPES * NUM_HIDE_LEVELS)

	# Sets Global screen construct values based on our Resolution
	def SetGlobals ( self, screen ):

	# GET RESOLUTION
		global xResolution
		global yResolution

		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()

	# ADJUST FOR RESOLUTION
		global FLAG_WIDTH
		FLAG_WIDTH = int((FLAG_PERCENT_WIDTH * xResolution) / 100)

		global MINI_MAP_WIDTH
		MINI_MAP_WIDTH = int((MINI_MAP_PERCENT_WIDTH * xResolution) / 100)

		global SADDLE_HEIGHT
		SADDLE_HEIGHT = int((SADDLE_HEIGHT_PERCENT * yResolution) / 100)

		global MINIMAP_HEIGHT
		MINIMAP_HEIGHT = SADDLE_HEIGHT - 5

		global LOWER_RIGHT_CORNER_BACKGROUND_WIDTH
		LOWER_RIGHT_CORNER_BACKGROUND_WIDTH = int((LOWER_RIGHT_CORNER_BACKGROUND_WIDTH_PERCENT * xResolution) / 100)

		global LOWER_LEFT_CORNER_BACKGROUND_WIDTH
		LOWER_LEFT_CORNER_BACKGROUND_WIDTH = int((LOWER_LEFT_CORNER_BACKGROUND_PERCENT_WIDTH * xResolution) / 100)

		global LOWER_LEFT_CORNER_BACKGROUND_HEIGHT
		LOWER_LEFT_CORNER_BACKGROUND_HEIGHT = int((LOWER_LEFT_CORNER_BACKGROUND_PERCENT_HEIGHT * yResolution) / 100)

		global LOWER_RIGHT_CORNER_BACKGROUND_HEIGHT
		LOWER_RIGHT_CORNER_BACKGROUND_HEIGHT = int((LOWER_RIGHT_CORNER_BACKGROUND_HEIGHT_PERCENT * yResolution) / 100)

		global BOTTOM_CENTER_HUD_HEIGHT
		BOTTOM_CENTER_HUD_HEIGHT = int((BOTTOM_CENTER_HUD_PERCENT_HEIGHT * yResolution) / 100)

		global TOP_CORNER_HUD_WIDTHS
		TOP_CORNER_HUD_WIDTHS = int((TOP_CORNER_HUD_PERCENT_WIDTH * xResolution) / 100)

		global TOP_CORNER_HUD_HEIGHT
		TOP_CORNER_HUD_HEIGHT = int((TOP_CORNER_HUD_PERCENT_HEIGHT * yResolution) / 100)

		global TOP_CENTER_HUD_HEIGHT
		TOP_CENTER_HUD_HEIGHT = int((TOP_CENTER_HUD_PERCENT_HEIGHT * yResolution) / 100)

		global TOP_CENTER_HUD_WIDTH
		TOP_CENTER_HUD_WIDTH = max(400, int((TOP_CENTER_HUD_PERCENT_WIDTH * xResolution) / 100))

		global CITY_TITLE_BAR_HEIGHT
		CITY_TITLE_BAR_HEIGHT = int((CITY_TITLE_BAR_PERCENT_HEIGHT * yResolution) / 100)

		global CITIZEN_BAR_HEIGHT
		CITIZEN_BAR_HEIGHT = int((CITIZEN_BAR_PERCENT_HEIGHT * yResolution) / 100)

		global STACK_BAR_HEIGHT
		STACK_BAR_HEIGHT = int((STACK_BAR_PERCENT_HEIGHT * yResolution) / 100)

		global EJECT_AREA_WIDTH
		EJECT_AREA_WIDTH = int((EJECT_AREA_PERCENT_WIDTH * xResolution) / 100)

		global MINI_MAP_RADIUS
		MINI_MAP_RADIUS = int((MINI_MAP_PERCENT_RADIUS * xResolution) / 100)

		global LEFT_PLOT_LIST_EDGE
		LEFT_PLOT_LIST_EDGE = int((LEFT_PLOT_LIST_PERCENT_EDGE * xResolution) / 100)

		global RIGHT_PLOT_LIST_EDGE
		RIGHT_PLOT_LIST_EDGE = int((RIGHT_PLOT_LIST_PERCENT_EDGE * xResolution) / 100)

		global MAIN_LARGE_PORTRAIT_HEIGHT
		MAIN_LARGE_PORTRAIT_HEIGHT = int((MAIN_LARGE_PORTRAIT_PERCENT_HEIGHT * yResolution) / 100)

		global MAIN_LARGE_PORTRAIT_WIDTH
		MAIN_LARGE_PORTRAIT_WIDTH = int((MAIL_LARGE_PORTRAIT_PERCENT_WIDTH * xResolution) / 100)

		global UNIT_PORTRAIT_HEIGHT
		UNIT_PORTRAIT_HEIGHT = int((UNIT_PORTRAIT_PERCENT_HEIGHT * yResolution) / 100)

		global UNIT_HEALTH_BAR_WIDTH
		UNIT_HEALTH_BAR_WIDTH = int((UNIT_HEALHT_BAR_PERCENT_WIDTH * xResolution) / 100)

		global SMALL_BUTTON_SIZE
		global MEDIUM_BUTTON_SIZE
		global LARGE_BUTTON_SIZE
		global STANDARD_FONT_SIZE

		if xResolution > 1600 and yResolution > 1024:
			SMALL_BUTTON_SIZE = 36
			MEDIUM_BUTTON_SIZE = 42
			LARGE_BUTTON_SIZE =	48
			STANDARD_FONT_SIZE = 4
		elif xResolution > 1280 and yResolution > 800:
			SMALL_BUTTON_SIZE = 32
			MEDIUM_BUTTON_SIZE = 36
			LARGE_BUTTON_SIZE = 40
			STANDARD_FONT_SIZE = 3
		else:
			SMALL_BUTTON_SIZE = 24
			MEDIUM_BUTTON_SIZE = 32
			LARGE_BUTTON_SIZE = 36
			STANDARD_FONT_SIZE = 3

		global ASPECT_RATIO
		ASPECT_RATIO = xResolution / yResolution

		global ASPECT_ADJUSTMENT
		ASPECT_ADJUSTMENT = ((float(xResolution) / float(yResolution)) * 3.0) / 4.0

		AspectAdjustment = 1 - ((1 - ASPECT_ADJUSTMENT) / 3) 
		global CITY_VIEW_BOX_MODIFIED_PERCENT_HEIGHT_AND_WIDTH
		CITY_VIEW_BOX_MODIFIED_PERCENT_HEIGHT_AND_WIDTH = int(CITY_VIEW_BOX_PERCENT_HEIGHT * AspectAdjustment)
	
		global MAP_EDGE_MARGIN_WIDTH
		MAP_EDGE_MARGIN_WIDTH = int((MAP_EDGE_MARGIN_PERCENT_WIDTH * xResolution) / 100)

		global CITY_VIEW_BOX_HEIGHT_AND_WIDTH
		CITY_VIEW_BOX_HEIGHT_AND_WIDTH = int((CITY_VIEW_BOX_MODIFIED_PERCENT_HEIGHT_AND_WIDTH * yResolution) / 100)

		global CITIZEN_BAR_WIDTH
		CITIZEN_BAR_WIDTH = int((CITIZEN_BAR_WIDTH_PERCENT * (xResolution - CITY_VIEW_BOX_HEIGHT_AND_WIDTH)) / 100)

		global BUILD_AREA_WIDTH
		BUILD_AREA_WIDTH = int(((100 - CITIZEN_BAR_WIDTH_PERCENT) * (xResolution - CITY_VIEW_BOX_HEIGHT_AND_WIDTH )) / 100)
	
		global BUILD_AREA_HEIGHT
		BUILD_AREA_HEIGHT = CITY_VIEW_BOX_HEIGHT_AND_WIDTH - STACK_BAR_HEIGHT
		
		global BUILDING_AREA_WIDTH
		BUILDING_AREA_WIDTH = int(CITIZEN_BAR_WIDTH - (STACK_BAR_HEIGHT * 2))

		global BUILDING_AREA_HEIGHT 
		#BUILDING_AREA_HEIGHT = yResolution - BOTTOM_CENTER_HUD_HEIGHT - CITIZEN_BAR_HEIGHT - STACK_BAR_HEIGHT - CITY_TITLE_BAR_HEIGHT
		BUILDING_AREA_HEIGHT = yResolution - BOTTOM_CENTER_HUD_HEIGHT - STACK_BAR_HEIGHT - CITY_TITLE_BAR_HEIGHT

		global TRANSPORT_AREA_HEIGHT
		TRANSPORT_AREA_HEIGHT = (yResolution - BOTTOM_CENTER_HUD_HEIGHT - CITY_TITLE_BAR_HEIGHT - CITY_VIEW_BOX_HEIGHT_AND_WIDTH) / 2

		global TRANSPORT_AREA_WIDTH
		TRANSPORT_AREA_WIDTH = (xResolution - CITIZEN_BAR_WIDTH) * 7 / 10

		global CITY_MULTI_TAB_AREA_HEIGHT
		CITY_MULTI_TAB_AREA_HEIGHT = ((yResolution - BOTTOM_CENTER_HUD_HEIGHT - CITY_TITLE_BAR_HEIGHT - CITY_VIEW_BOX_HEIGHT_AND_WIDTH) / 2) - (STACK_BAR_HEIGHT * 3 / 2)

		global CITY_MULTI_TAB_SIZE
		CITY_MULTI_TAB_SIZE = (TRANSPORT_AREA_HEIGHT - STACK_BAR_HEIGHT - MAP_EDGE_MARGIN_WIDTH) / 3

		global BUILDING_GRID
		BUILDING_GRID = []
		for iBUILDING_TYPE in range(len(BUILDING_DATA)):
			TypeArray = []
			for iData in range(len(BUILDING_DATA[iBUILDING_TYPE])):
				if  iData == 0 :
					Dimention = BUILDING_AREA_WIDTH
				elif iData == 1:
					Dimention = BUILDING_AREA_HEIGHT
				elif iData == 2:
					Dimention = BUILDING_AREA_HEIGHT
				else:
					Dimention = BUILDING_AREA_WIDTH

				TypeArray.append(int((BUILDING_DATA[iBUILDING_TYPE][iData] * Dimention) / 100))

			BUILDING_GRID.append(TypeArray)

		global AVOID_GROWTH
		for iEmphasize in range(gc.getNumEmphasizeInfos()):
			if gc.getEmphasizeInfo(iEmphasize).isAvoidGrowth():
				AVOID_GROWTH = iEmphasize
				break

	# EMPHASIZE YIELDS
		global EMPHASIZEYIELDS
		EMPHASIZEYIELDS = []
		for iYield in range(YieldTypes.NUM_YIELD_TYPES):
			for iEmp in range(gc.getNumEmphasizeInfos()):
				if gc.getEmphasizeInfo(iEmp).getYieldChange(iYield) != 0:
					EMPHASIZEYIELDS.append(iYield)
					break

	# SET INITIALIZATION
		global INITIALIZED
		INITIALIZED = True

	# Wraps <Font> tags around text
	def setFontSize( self, Text, relativeSize ):
		size = STANDARD_FONT_SIZE + relativeSize
		if size > 4:
			size = 4
		return "<font=" + str(size) + ">" + Text + "</font>"

	# Will Initialize the majority of Background panels and Widgets
	def interfaceScreen ( self ):
		if (CyGame().isPitbossHost()):
			return

	# GLOBAL NUM VARIABLES SET
		global g_NumEmphasizeInfos
		global g_NumHurryInfos
		global g_NumActionInfos

		g_NumEmphasizeInfos = gc.getNumEmphasizeInfos()
		g_NumHurryInfos = gc.getNumHurryInfos()
		g_NumActionInfos = gc.getNumActionInfos()

	# INIT MAIN INTERFACE SCREEN
		screen = CyGInterfaceScreen("MainInterface", CvScreenEnums.MAIN_INTERFACE )
		self.SetGlobals (screen)
		screen.setForcedRedraw(True)
		screen.setMainInterface(True)

	# CITY BURLAP BACKGROUND PANELS
		screen.addPanel("CityTopBackground", u"", u"", True, False, 0, 0, xResolution, CITY_TITLE_BAR_HEIGHT, PanelStyles.PANEL_STYLE_STANDARD, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.addDrawControl("CityTopBackground", ArtFileMgr.getInterfaceArtInfo("INTERFACE_CITY_BG_TOP").getPath(), 0, 0, xResolution, CITY_TITLE_BAR_HEIGHT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		self.appendtoHideState(screen, "CityTopBackground", HIDE_TYPE_CITY, HIDE_LEVEL_ALL)

		screen.addPanel("CityLeftBackground", u"", u"", True, False, 0, CITY_TITLE_BAR_HEIGHT, CITIZEN_BAR_WIDTH, yResolution, PanelStyles.PANEL_STYLE_STANDARD, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.addDrawControl("CityLeftBackground", ArtFileMgr.getInterfaceArtInfo("INTERFACE_CITY_BG_LEFT").getPath(), 0, CITY_TITLE_BAR_HEIGHT, CITIZEN_BAR_WIDTH, yResolution, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		self.appendtoHideState(screen, "CityLeftBackground", HIDE_TYPE_CITY, HIDE_LEVEL_ALL)

		screen.addPanel("CityLowBackground", u"", u"", True, False, CITIZEN_BAR_WIDTH, CITY_TITLE_BAR_HEIGHT + CITY_VIEW_BOX_HEIGHT_AND_WIDTH - (MAP_EDGE_MARGIN_WIDTH * 2), xResolution - CITIZEN_BAR_WIDTH, yResolution - CITY_VIEW_BOX_HEIGHT_AND_WIDTH - CITY_TITLE_BAR_HEIGHT + MAP_EDGE_MARGIN_WIDTH, PanelStyles.PANEL_STYLE_STANDARD, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.addDrawControl("CityLowBackground", ArtFileMgr.getInterfaceArtInfo("INTERFACE_CITY_BG_LOW").getPath(), CITIZEN_BAR_WIDTH, CITY_TITLE_BAR_HEIGHT + CITY_VIEW_BOX_HEIGHT_AND_WIDTH - (MAP_EDGE_MARGIN_WIDTH * 2), xResolution - CITIZEN_BAR_WIDTH, yResolution - CITY_VIEW_BOX_HEIGHT_AND_WIDTH - CITY_TITLE_BAR_HEIGHT + (MAP_EDGE_MARGIN_WIDTH * 2), WidgetTypes.WIDGET_GENERAL, -1, -1 )
		self.appendtoHideState(screen, "CityLowBackground", HIDE_TYPE_CITY, HIDE_LEVEL_ALL)

		screen.addPanel("CityRightBackground", u"", u"", True, False, CITIZEN_BAR_WIDTH + CITY_VIEW_BOX_HEIGHT_AND_WIDTH - (MAP_EDGE_MARGIN_WIDTH * 2), CITY_TITLE_BAR_HEIGHT, BUILD_AREA_WIDTH + (MAP_EDGE_MARGIN_WIDTH * 2), CITY_VIEW_BOX_HEIGHT_AND_WIDTH, PanelStyles.PANEL_STYLE_STANDARD, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.addDrawControl("CityRightBackground", ArtFileMgr.getInterfaceArtInfo("INTERFACE_CITY_BG_RIGHT").getPath(), CITIZEN_BAR_WIDTH + CITY_VIEW_BOX_HEIGHT_AND_WIDTH - (MAP_EDGE_MARGIN_WIDTH * 2), CITY_TITLE_BAR_HEIGHT, BUILD_AREA_WIDTH + (MAP_EDGE_MARGIN_WIDTH * 2) + 10, CITY_VIEW_BOX_HEIGHT_AND_WIDTH, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		self.appendtoHideState(screen, "CityRightBackground", HIDE_TYPE_CITY, HIDE_LEVEL_ALL)

	# CITY FRAMING PANELS
		screen.addPanel("CityTitlePanel", u"", u"", True, False, 0, 0, xResolution, CITY_TITLE_BAR_HEIGHT, PanelStyles.PANEL_STYLE_STANDARD, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.addDrawControl("CityTitlePanel", ArtFileMgr.getInterfaceArtInfo("INTERFACE_SCREEN_TITLE").getPath(), 0, 0, xResolution, int(CITY_TITLE_BAR_HEIGHT * 1.3), WidgetTypes.WIDGET_GENERAL, -1, -1 )
		self.appendtoHideState(screen, "CityTitlePanel", HIDE_TYPE_CITY, HIDE_LEVEL_ALL)	

	# RESOURCE TABLE BACKGROUND
		screen.addScrollPanel( "CityTransportBackGroundPanel", u"", xResolution - TRANSPORT_AREA_WIDTH , yResolution - BOTTOM_CENTER_HUD_HEIGHT - TRANSPORT_AREA_HEIGHT, TRANSPORT_AREA_WIDTH, TRANSPORT_AREA_HEIGHT + (STACK_BAR_HEIGHT / 2), PanelStyles.PANEL_STYLE_MAIN, false, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.addDrawControl("CityTransportBackGroundPanel", ArtFileMgr.getInterfaceArtInfo("INTERFACE_EUROPE_SHADOW_BOX").getPath(),  xResolution - TRANSPORT_AREA_WIDTH , yResolution - BOTTOM_CENTER_HUD_HEIGHT - TRANSPORT_AREA_HEIGHT, TRANSPORT_AREA_WIDTH, TRANSPORT_AREA_HEIGHT + (STACK_BAR_HEIGHT / 2), WidgetTypes.WIDGET_GENERAL, -1, -1 )
		self.appendtoHideState(screen, "CityTransportBackGroundPanel", HIDE_TYPE_CITY, HIDE_LEVEL_ALL)	

		screen.addPanel("CityBuildFramePanel", u"", u"", True, False, CITIZEN_BAR_WIDTH + CITY_VIEW_BOX_HEIGHT_AND_WIDTH - MAP_EDGE_MARGIN_WIDTH, CITY_TITLE_BAR_HEIGHT, BUILD_AREA_WIDTH, CITY_VIEW_BOX_HEIGHT_AND_WIDTH - (MAP_EDGE_MARGIN_WIDTH * 2), PanelStyles.PANEL_STYLE_STANDARD, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.addDrawControl("CityBuildFramePanel", ArtFileMgr.getInterfaceArtInfo("INTERFACE_BUILD_BOX").getPath(), CITIZEN_BAR_WIDTH + CITY_VIEW_BOX_HEIGHT_AND_WIDTH - MAP_EDGE_MARGIN_WIDTH, CITY_TITLE_BAR_HEIGHT, BUILD_AREA_WIDTH, CITY_VIEW_BOX_HEIGHT_AND_WIDTH - (MAP_EDGE_MARGIN_WIDTH * 2), WidgetTypes.WIDGET_GENERAL, -1, -1 )
		self.appendtoHideState(screen, "CityBuildFramePanel", HIDE_TYPE_CITY, HIDE_LEVEL_ALL)	

		screen.addPanel("CityMapFramePanel", u"", u"", True, False, CITIZEN_BAR_WIDTH, CITY_TITLE_BAR_HEIGHT, CITY_VIEW_BOX_HEIGHT_AND_WIDTH - (MAP_EDGE_MARGIN_WIDTH * 2), CITY_VIEW_BOX_HEIGHT_AND_WIDTH - (MAP_EDGE_MARGIN_WIDTH * 2) , PanelStyles.PANEL_STYLE_STANDARD, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.addDrawControl("CityMapFramePanel", ArtFileMgr.getInterfaceArtInfo("INTERFACE_MAP_FRAME").getPath(), CITIZEN_BAR_WIDTH, CITY_TITLE_BAR_HEIGHT, CITY_VIEW_BOX_HEIGHT_AND_WIDTH - (MAP_EDGE_MARGIN_WIDTH * 2), CITY_VIEW_BOX_HEIGHT_AND_WIDTH - (MAP_EDGE_MARGIN_WIDTH * 2), WidgetTypes.WIDGET_GENERAL, -1, -1 )
		self.appendtoHideState(screen, "CityMapFramePanel", HIDE_TYPE_CITY, HIDE_LEVEL_ALL)	

		screen.addPanel("CityMultiTabFramePanel", u"", u"", True, False, CITIZEN_BAR_WIDTH, CITY_TITLE_BAR_HEIGHT + CITY_VIEW_BOX_HEIGHT_AND_WIDTH, xResolution - CITIZEN_BAR_WIDTH - MAP_EDGE_MARGIN_WIDTH, TRANSPORT_AREA_HEIGHT - STACK_BAR_HEIGHT, PanelStyles.PANEL_STYLE_STANDARD, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.addDrawControl("CityMultiTabFramePanel", ArtFileMgr.getInterfaceArtInfo("INTERFACE_WIDE_BOX").getPath(), CITIZEN_BAR_WIDTH, CITY_TITLE_BAR_HEIGHT + CITY_VIEW_BOX_HEIGHT_AND_WIDTH - MAP_EDGE_MARGIN_WIDTH , xResolution - CITIZEN_BAR_WIDTH - (MAP_EDGE_MARGIN_WIDTH * 1), TRANSPORT_AREA_HEIGHT - STACK_BAR_HEIGHT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		self.appendtoHideState(screen, "CityMultiTabFramePanel", HIDE_TYPE_CITY, HIDE_LEVEL_ALL)	

		screen.addScrollPanel("CityGarrisonBackGroundPanel", u"", CITIZEN_BAR_WIDTH - 30, yResolution - BOTTOM_CENTER_HUD_HEIGHT - TRANSPORT_AREA_HEIGHT, xResolution - CITIZEN_BAR_WIDTH - TRANSPORT_AREA_WIDTH + STACK_BAR_HEIGHT + 30, TRANSPORT_AREA_HEIGHT + (STACK_BAR_HEIGHT / 2), PanelStyles.PANEL_STYLE_STANDARD, false, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.addDrawControl("CityGarrisonBackGroundPanel", ArtFileMgr.getInterfaceArtInfo("INTERFACE_EUROPE_SHADOW_BOX").getPath(), CITIZEN_BAR_WIDTH - 30, yResolution - BOTTOM_CENTER_HUD_HEIGHT - TRANSPORT_AREA_HEIGHT, xResolution - CITIZEN_BAR_WIDTH - TRANSPORT_AREA_WIDTH + STACK_BAR_HEIGHT + 30, TRANSPORT_AREA_HEIGHT + (STACK_BAR_HEIGHT / 2), WidgetTypes.WIDGET_GENERAL, -1, -1 )
		self.appendtoHideState(screen, "CityGarrisonBackGroundPanel", HIDE_TYPE_CITY, HIDE_LEVEL_ALL)	

		screen.addPanel("CityManagerBackground", u"", u"", True, False, 0, CITY_TITLE_BAR_HEIGHT, int(CITIZEN_BAR_WIDTH * 1.07), int((yResolution - (BOTTOM_CENTER_HUD_HEIGHT + CITY_TITLE_BAR_HEIGHT)) * 1.07), PanelStyles.PANEL_STYLE_STANDARD, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.addDrawControl("CityManagerBackground", ArtFileMgr.getInterfaceArtInfo("INTERFACE_SCROLL_BG").getPath(), 0, CITY_TITLE_BAR_HEIGHT, int(CITIZEN_BAR_WIDTH * 1.07), int((yResolution - (BOTTOM_CENTER_HUD_HEIGHT + CITY_TITLE_BAR_HEIGHT)) * 1.04), WidgetTypes.WIDGET_GENERAL, -1, -1 )
		self.appendtoHideState(screen, "CityManagerBackground", HIDE_TYPE_CITY, HIDE_LEVEL_ALL)

	# DATE/TIME WINDOW
		screen.addPanel("DateWindowPanel", u"", u"", True, False, (xResolution - TOP_CENTER_HUD_WIDTH) / 2, 0, TOP_CENTER_HUD_WIDTH, TOP_CENTER_HUD_HEIGHT, PanelStyles.PANEL_STYLE_MAIN, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.addDrawControl("DateWindowPanel", ArtFileMgr.getInterfaceArtInfo("SCREEN_DATE_BOX").getPath(), (xResolution - TOP_CENTER_HUD_WIDTH) / 2, 0, TOP_CENTER_HUD_WIDTH, TOP_CENTER_HUD_HEIGHT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		self.appendtoHideState(screen, "DateWindowPanel", HIDE_TYPE_MAP, HIDE_LEVEL_HIDE)

	# MAP BACKGROUND SADDLE
		screen.addPanel("InterfaceMapViewSaddleFarRight", u"", u"", True, False, xResolution - (xResolution / 8), yResolution - SADDLE_HEIGHT, xResolution / 8, SADDLE_HEIGHT, PanelStyles.PANEL_STYLE_STANDARD, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.addDrawControl("InterfaceMapViewSaddleFarRight", ArtFileMgr.getInterfaceArtInfo("SCREEN_BG_MAP_SADDLE_FAR_RIGHT").getPath(), xResolution - (xResolution / 8), yResolution - SADDLE_HEIGHT, xResolution / 8, SADDLE_HEIGHT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		self.appendtoHideState(screen, "InterfaceMapViewSaddleFarRight", HIDE_TYPE_MAP, HIDE_LEVEL_NORMAL)

		screen.addPanel("InterfaceMapViewSaddleRight", u"", u"", True, False, xResolution - (xResolution / 4), yResolution - (SADDLE_HEIGHT * 5 / 7), (xResolution / 4) - (xResolution / 8), SADDLE_HEIGHT * 5 / 7, PanelStyles.PANEL_STYLE_STANDARD, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.addDrawControl("InterfaceMapViewSaddleRight", ArtFileMgr.getInterfaceArtInfo("SCREEN_BG_MAP_SADDLE_RIGHT").getPath(), xResolution - (xResolution / 4), yResolution - (SADDLE_HEIGHT * 5 / 7), (xResolution / 4) - (xResolution / 8), SADDLE_HEIGHT * 5 / 7, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		self.appendtoHideState(screen, "InterfaceMapViewSaddleRight", HIDE_TYPE_MAP, HIDE_LEVEL_NORMAL)

		screen.addPanel("InterfaceMapViewSaddleMiddle", u"", u"", True, False, xResolution / 4, yResolution - (SADDLE_HEIGHT  * 4 / 7), xResolution / 2, SADDLE_HEIGHT  * 4 / 7, PanelStyles.PANEL_STYLE_STANDARD, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.addDrawControl("InterfaceMapViewSaddleMiddle", ArtFileMgr.getInterfaceArtInfo("SCREEN_BG_MAP_SADDLE_MIDDLE").getPath(), xResolution / 4, yResolution - (SADDLE_HEIGHT  * 4 / 7), xResolution / 2, SADDLE_HEIGHT  * 4 / 7, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		self.appendtoHideState(screen, "InterfaceMapViewSaddleMiddle", HIDE_TYPE_MAP, HIDE_LEVEL_NORMAL)

		screen.addPanel("InterfaceMapViewSaddleLeft", u"", u"", True, False, xResolution / 8, yResolution - (SADDLE_HEIGHT * 5 / 7), (xResolution / 4) - (xResolution / 8), SADDLE_HEIGHT * 5 / 7, PanelStyles.PANEL_STYLE_STANDARD, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.addDrawControl("InterfaceMapViewSaddleLeft", ArtFileMgr.getInterfaceArtInfo("SCREEN_BG_MAP_SADDLE_LEFT").getPath(), xResolution / 8, yResolution - (SADDLE_HEIGHT * 5 / 7), (xResolution / 4) - (xResolution / 8), SADDLE_HEIGHT * 5 / 7, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		self.appendtoHideState(screen, "InterfaceMapViewSaddleLeft", HIDE_TYPE_MAP, HIDE_LEVEL_NORMAL)

		screen.addPanel("InterfaceMapViewSaddleFarLeft", u"", u"", True, False, 0, yResolution - SADDLE_HEIGHT, xResolution / 8, SADDLE_HEIGHT, PanelStyles.PANEL_STYLE_STANDARD, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.addDrawControl("InterfaceMapViewSaddleFarLeft", ArtFileMgr.getInterfaceArtInfo("SCREEN_BG_MAP_SADDLE_FAR_LEFT").getPath(), 0, yResolution - SADDLE_HEIGHT, xResolution / 8, SADDLE_HEIGHT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		self.appendtoHideState(screen, "InterfaceMapViewSaddleFarLeft", HIDE_TYPE_MAP, HIDE_LEVEL_NORMAL)

	# MULITPANEL TABS
		iCumulativeY = (STACK_BAR_HEIGHT * 3 / 2) + (MAP_EDGE_MARGIN_WIDTH / 2)
		RelativeButtonSize = 130

		screen.setImageButton("MapHighlightButton", ArtFileMgr.getInterfaceArtInfo("INTERFACE_HIGHLIGHTED_BUTTON").getPath(), xResolution - (MAP_EDGE_MARGIN_WIDTH * 2) - CITY_MULTI_TAB_SIZE - ((CITY_MULTI_TAB_SIZE * RelativeButtonSize / 100) / 2) + (CITY_MULTI_TAB_SIZE / 2), CITY_TITLE_BAR_HEIGHT + CITY_VIEW_BOX_HEIGHT_AND_WIDTH - (MAP_EDGE_MARGIN_WIDTH * 2) + iCumulativeY - (CITY_MULTI_TAB_SIZE / 2) - ((CITY_MULTI_TAB_SIZE * RelativeButtonSize / 100) / 2) + (CITY_MULTI_TAB_SIZE / 2), CITY_MULTI_TAB_SIZE * RelativeButtonSize / 100, CITY_MULTI_TAB_SIZE * RelativeButtonSize / 100, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.setImageButton("MapToggle", ArtFileMgr.getInterfaceArtInfo("INTERFACE_CITY_MAP_BUTTON").getPath(), xResolution - (MAP_EDGE_MARGIN_WIDTH * 2) - CITY_MULTI_TAB_SIZE, CITY_TITLE_BAR_HEIGHT + CITY_VIEW_BOX_HEIGHT_AND_WIDTH - (MAP_EDGE_MARGIN_WIDTH * 2) + iCumulativeY - (CITY_MULTI_TAB_SIZE / 2), CITY_MULTI_TAB_SIZE, CITY_MULTI_TAB_SIZE, WidgetTypes.WIDGET_GENERAL, MAP_MANAGMENT_TOGGLE, -1)
		self.appendtoHideState(screen, "MapToggle", HIDE_TYPE_CITY, HIDE_LEVEL_ALL)
		self.appendtoHideState(screen, "MapHighlightButton", HIDE_TYPE_CITY, HIDE_LEVEL_ALL)
		iCumulativeY += CITY_MULTI_TAB_SIZE
		
		screen.setImageButton("BuildHighlightButton", ArtFileMgr.getInterfaceArtInfo("INTERFACE_HIGHLIGHTED_BUTTON").getPath(), xResolution - (MAP_EDGE_MARGIN_WIDTH * 2) - CITY_MULTI_TAB_SIZE - ((CITY_MULTI_TAB_SIZE * RelativeButtonSize / 100) / 2) + (CITY_MULTI_TAB_SIZE / 2), CITY_TITLE_BAR_HEIGHT + CITY_VIEW_BOX_HEIGHT_AND_WIDTH - (MAP_EDGE_MARGIN_WIDTH * 2) + iCumulativeY - (CITY_MULTI_TAB_SIZE / 2) - ((CITY_MULTI_TAB_SIZE * RelativeButtonSize / 100) / 2) + (CITY_MULTI_TAB_SIZE / 2), CITY_MULTI_TAB_SIZE * RelativeButtonSize / 100, CITY_MULTI_TAB_SIZE * RelativeButtonSize / 100, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.setImageButton("BuildToggle", ArtFileMgr.getInterfaceArtInfo("INTERFACE_CITY_BUILD_BUTTON").getPath(), xResolution - (MAP_EDGE_MARGIN_WIDTH * 2) - CITY_MULTI_TAB_SIZE, CITY_TITLE_BAR_HEIGHT + CITY_VIEW_BOX_HEIGHT_AND_WIDTH - (MAP_EDGE_MARGIN_WIDTH * 2) + iCumulativeY - (CITY_MULTI_TAB_SIZE / 2), CITY_MULTI_TAB_SIZE, CITY_MULTI_TAB_SIZE, WidgetTypes.WIDGET_GENERAL, BUILDING_MANAGMENT_TOGGLE, -1)
		self.appendtoHideState(screen, "BuildToggle", HIDE_TYPE_CITY, HIDE_LEVEL_ALL)
		self.appendtoHideState(screen, "BuildHighlightButton", HIDE_TYPE_CITY, HIDE_LEVEL_ALL)
		iCumulativeY += CITY_MULTI_TAB_SIZE
		
		screen.setImageButton("GovernorHighlightButton", ArtFileMgr.getInterfaceArtInfo("INTERFACE_HIGHLIGHTED_BUTTON").getPath(), xResolution - (MAP_EDGE_MARGIN_WIDTH * 2) - CITY_MULTI_TAB_SIZE - ((CITY_MULTI_TAB_SIZE * RelativeButtonSize / 100) / 2) + (CITY_MULTI_TAB_SIZE / 2), CITY_TITLE_BAR_HEIGHT + CITY_VIEW_BOX_HEIGHT_AND_WIDTH - (MAP_EDGE_MARGIN_WIDTH * 2) + iCumulativeY - (CITY_MULTI_TAB_SIZE / 2) - ((CITY_MULTI_TAB_SIZE * RelativeButtonSize / 100) / 2) + (CITY_MULTI_TAB_SIZE / 2), CITY_MULTI_TAB_SIZE * RelativeButtonSize / 100, CITY_MULTI_TAB_SIZE * RelativeButtonSize / 100, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.setImageButton("GovernorToggle", ArtFileMgr.getInterfaceArtInfo("INTERFACE_CITY_GOVENOR_BUTTON").getPath(), xResolution - (MAP_EDGE_MARGIN_WIDTH * 2) - CITY_MULTI_TAB_SIZE, CITY_TITLE_BAR_HEIGHT + CITY_VIEW_BOX_HEIGHT_AND_WIDTH - (MAP_EDGE_MARGIN_WIDTH * 2) + iCumulativeY - (CITY_MULTI_TAB_SIZE / 2), CITY_MULTI_TAB_SIZE, CITY_MULTI_TAB_SIZE, WidgetTypes.WIDGET_GENERAL, AUTOMATION_MANAGMENT_TOGGLE, -1)
		self.appendtoHideState(screen, "GovernorToggle", HIDE_TYPE_CITY, HIDE_LEVEL_ALL)
		self.appendtoHideState(screen, "GovernorHighlightButton", HIDE_TYPE_CITY, HIDE_LEVEL_ALL)
		iCumulativeY += CITY_MULTI_TAB_SIZE

	# CITY BUILDING MULTILIST
		self.BOTTOM_BUTTON_SIZE = (BUILD_AREA_WIDTH - (STACK_BAR_HEIGHT * 3 / 2)) / 4
		self.BOTTOM_BUTTON_ROWS = 3

		screen.addMultiListControlGFC("CityBuildingSelectionMultiList", u"", CITIZEN_BAR_WIDTH + CITY_VIEW_BOX_HEIGHT_AND_WIDTH + (STACK_BAR_HEIGHT / 2) - MAP_EDGE_MARGIN_WIDTH, CITY_TITLE_BAR_HEIGHT + (STACK_BAR_HEIGHT / 2), BUILD_AREA_WIDTH, BUILD_AREA_HEIGHT - STACK_BAR_HEIGHT, self.BOTTOM_BUTTON_ROWS, self.BOTTOM_BUTTON_SIZE, self.BOTTOM_BUTTON_SIZE, TableStyles.TABLE_STYLE_STANDARD )		
		self.appendtoHideState(screen, "CityBuildingSelectionMultiList", HIDE_TYPE_CITY, HIDE_LEVEL_HIDE)

		screen.addMultiListControlGFC("MapBuildingSelectionMultiList", u"", xResolution * 7 / 20, yResolution - SADDLE_HEIGHT + (self.BOTTOM_BUTTON_SIZE / 2), xResolution * 9 / 20, self.BOTTOM_BUTTON_SIZE * 3, self.BOTTOM_BUTTON_ROWS, self.BOTTOM_BUTTON_SIZE, self.BOTTOM_BUTTON_SIZE, TableStyles.TABLE_STYLE_STANDARD )		
		screen.hide("MapBuildingSelectionMultiList")

	# CITY BUILDING MAP CONSTRUCTION BAR
		screen.addStackedBarGFC("MapCityProductionBar", xResolution - UNIT_HEALTH_BAR_WIDTH - 10, yResolution - LOWER_RIGHT_CORNER_BACKGROUND_HEIGHT - STACK_BAR_HEIGHT, UNIT_HEALTH_BAR_WIDTH, STACK_BAR_HEIGHT, InfoBarTypes.NUM_INFOBAR_TYPES, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.setStackedBarColors("MapCityProductionBar", InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_GREAT_PEOPLE_STORED") )
		screen.setStackedBarColors("MapCityProductionBar", InfoBarTypes.INFOBAR_RATE, gc.getInfoTypeForString("COLOR_GREAT_PEOPLE_RATE") )
		screen.setStackedBarColors("MapCityProductionBar", InfoBarTypes.INFOBAR_RATE_EXTRA, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.setStackedBarColors("MapCityProductionBar", InfoBarTypes.INFOBAR_EMPTY, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.hide("MapCityProductionBar")
	
	# EMPHASIZE TABLE
		screen.addTableControlGFC("EmphasizeTable", 1, CITIZEN_BAR_WIDTH + CITY_VIEW_BOX_HEIGHT_AND_WIDTH + (STACK_BAR_HEIGHT / 2) - MAP_EDGE_MARGIN_WIDTH, CITY_TITLE_BAR_HEIGHT + (STACK_BAR_HEIGHT / 2), BUILD_AREA_WIDTH, BUILD_AREA_HEIGHT - STACK_BAR_HEIGHT, False, False, MEDIUM_BUTTON_SIZE, MEDIUM_BUTTON_SIZE, TableStyles.TABLE_STYLE_STANDARD)
		self.appendtoHideState(screen, "EmphasizeTable", HIDE_TYPE_CITY, HIDE_LEVEL_HIDE)
		screen.setStyle("EmphasizeTable", "Table_EmptyScroll_Style")
		screen.appendTableRow("EmphasizeTable")
		for iYield in EMPHASIZEYIELDS:
			screen.appendTableRow("EmphasizeTable")

	# HELP TEXT AREA
		screen.setHelpTextArea( HELPTEXT_AREA_MAX_WIDTH, FontTypes.SMALL_FONT, HELTTEXT_AREA_X_MARGIN, yResolution - SADDLE_HEIGHT, -0.1, False, "", True, False, CvUtil.FONT_LEFT_JUSTIFY, HELPTEXT_AREA_MIN_WIDTH)

	# ADVISOR BUTTONS
		SpaceAvailable = (xResolution / 2) - (TOP_CENTER_HUD_WIDTH / 2)
		self.ADVISOR_BUTTON_SPACING = SpaceAvailable / 9
		
		self.ADVISOR_BUTTON_SIZE = SMALL_BUTTON_SIZE * 2
		iBtnX = (xResolution / 2) + (TOP_CENTER_HUD_WIDTH / 2) - (self.ADVISOR_BUTTON_SIZE / 3)

		screen.setImageButton("DomesticAdvisorButton", ArtFileMgr.getInterfaceArtInfo("INTERFACE_DOMESTIC_ADVISOR").getPath(), iBtnX, (TOP_CENTER_HUD_HEIGHT - self.ADVISOR_BUTTON_SIZE) / 2, self.ADVISOR_BUTTON_SIZE, self.ADVISOR_BUTTON_SIZE, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_DOMESTIC_SCREEN).getActionInfoIndex(), -1 )
		screen.setImageShape("DomesticAdvisorButton", ImageShapes.IMAGE_SHAPE_ELLIPSE, -1)
		screen.setHitMargins("DomesticAdvisorButton", self.ADVISOR_BUTTON_SIZE / 6, self.ADVISOR_BUTTON_SIZE / 6)
		self.appendtoHideState(screen, "DomesticAdvisorButton", HIDE_TYPE_MAP, HIDE_LEVEL_HIDE)
		iBtnX += self.ADVISOR_BUTTON_SPACING

		screen.setImageButton("EuropeScreenButton", ArtFileMgr.getInterfaceArtInfo("INTERFACE_EUROPE").getPath(), iBtnX, (TOP_CENTER_HUD_HEIGHT - self.ADVISOR_BUTTON_SIZE) / 2, self.ADVISOR_BUTTON_SIZE, self.ADVISOR_BUTTON_SIZE, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_EUROPE_SCREEN).getActionInfoIndex(), -1 )
		screen.setImageShape("EuropeScreenButton", ImageShapes.IMAGE_SHAPE_ELLIPSE, -1)
		screen.setHitMargins("EuropeScreenButton", self.ADVISOR_BUTTON_SIZE / 6, self.ADVISOR_BUTTON_SIZE / 6)
		self.appendtoHideState(screen, "EuropeScreenButton", HIDE_TYPE_MAP, HIDE_LEVEL_HIDE)
		iBtnX += self.ADVISOR_BUTTON_SPACING

		screen.setImageButton("RevolutionAdvisorButton", ArtFileMgr.getInterfaceArtInfo("INTERFACE_REVOLUTION_ADVISOR").getPath(), iBtnX, (TOP_CENTER_HUD_HEIGHT - self.ADVISOR_BUTTON_SIZE) / 2, self.ADVISOR_BUTTON_SIZE, self.ADVISOR_BUTTON_SIZE, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_REVOLUTION_SCREEN).getActionInfoIndex(), -1 )
		screen.setImageShape("RevolutionAdvisorButton", ImageShapes.IMAGE_SHAPE_ELLIPSE, -1)
		screen.setHitMargins("RevolutionAdvisorButton", self.ADVISOR_BUTTON_SIZE / 6, self.ADVISOR_BUTTON_SIZE / 6)
		self.appendtoHideState(screen, "RevolutionAdvisorButton", HIDE_TYPE_MAP, HIDE_LEVEL_HIDE)
		iBtnX += self.ADVISOR_BUTTON_SPACING

		screen.setImageButton("ForeignAdvisorButton", ArtFileMgr.getInterfaceArtInfo("INTERFACE_FOREIGN_ADVISOR").getPath(), iBtnX, (TOP_CENTER_HUD_HEIGHT - self.ADVISOR_BUTTON_SIZE) / 2, self.ADVISOR_BUTTON_SIZE, self.ADVISOR_BUTTON_SIZE, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_FOREIGN_SCREEN).getActionInfoIndex(), -1 )
		screen.setImageShape("ForeignAdvisorButton", ImageShapes.IMAGE_SHAPE_ELLIPSE, -1)
		screen.setHitMargins("ForeignAdvisorButton", self.ADVISOR_BUTTON_SIZE / 6, self.ADVISOR_BUTTON_SIZE / 6)
		self.appendtoHideState(screen, "ForeignAdvisorButton", HIDE_TYPE_MAP, HIDE_LEVEL_HIDE)
		iBtnX += self.ADVISOR_BUTTON_SPACING

		screen.setImageButton("MilitaryAdvisorButton", ArtFileMgr.getInterfaceArtInfo("INTERFACE_MILITARY_ADVISOR").getPath(), iBtnX, (TOP_CENTER_HUD_HEIGHT - self.ADVISOR_BUTTON_SIZE) / 2, self.ADVISOR_BUTTON_SIZE, self.ADVISOR_BUTTON_SIZE, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_MILITARY_SCREEN).getActionInfoIndex(), -1 )
		screen.setImageShape("MilitaryAdvisorButton", ImageShapes.IMAGE_SHAPE_ELLIPSE, -1)
		screen.setHitMargins("MilitaryAdvisorButton", self.ADVISOR_BUTTON_SIZE / 6, self.ADVISOR_BUTTON_SIZE / 6)
		self.appendtoHideState(screen, "MilitaryAdvisorButton", HIDE_TYPE_MAP, HIDE_LEVEL_HIDE)
		iBtnX += self.ADVISOR_BUTTON_SPACING

		screen.setImageButton( "FatherAdvisorButton", ArtFileMgr.getInterfaceArtInfo("INTERFACE_FOUNDING_FATHER").getPath(), iBtnX, (TOP_CENTER_HUD_HEIGHT - self.ADVISOR_BUTTON_SIZE) / 2, self.ADVISOR_BUTTON_SIZE, self.ADVISOR_BUTTON_SIZE, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_FATHER_SCREEN).getActionInfoIndex(), -1 )
		screen.setImageShape("FatherAdvisorButton", ImageShapes.IMAGE_SHAPE_ELLIPSE, -1)
		screen.setHitMargins("FatherAdvisorButton", self.ADVISOR_BUTTON_SIZE / 6, self.ADVISOR_BUTTON_SIZE / 6)
		self.appendtoHideState(screen, "FatherAdvisorButton", HIDE_TYPE_MAP, HIDE_LEVEL_HIDE)
		iBtnX += self.ADVISOR_BUTTON_SPACING

		screen.setImageButton("CongressAdvisorButton", ArtFileMgr.getInterfaceArtInfo("INTERFACE_CONTINENTAL_CONGRESS").getPath(), iBtnX, (TOP_CENTER_HUD_HEIGHT - self.ADVISOR_BUTTON_SIZE) / 2, self.ADVISOR_BUTTON_SIZE, self.ADVISOR_BUTTON_SIZE, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_CONGRESS_SCREEN).getActionInfoIndex(), -1 )
		screen.setImageShape("CongressAdvisorButton", ImageShapes.IMAGE_SHAPE_ELLIPSE, -1)
		screen.setHitMargins("CongressAdvisorButton", self.ADVISOR_BUTTON_SIZE / 6, self.ADVISOR_BUTTON_SIZE / 6)
		self.appendtoHideState(screen, "CongressAdvisorButton", HIDE_TYPE_MAP, HIDE_LEVEL_HIDE)
		iBtnX += self.ADVISOR_BUTTON_SPACING

		screen.setImageButton( "SettingsAdvisorButton", ArtFileMgr.getInterfaceArtInfo("INTERFACE_VICTORY_ADVISOR").getPath(), iBtnX, (TOP_CENTER_HUD_HEIGHT - self.ADVISOR_BUTTON_SIZE) / 2, self.ADVISOR_BUTTON_SIZE, self.ADVISOR_BUTTON_SIZE, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_VICTORY_SCREEN).getActionInfoIndex(), -1 )
		screen.setImageShape("SettingsAdvisorButton", ImageShapes.IMAGE_SHAPE_ELLIPSE, -1)
		screen.setHitMargins("SettingsAdvisorButton", self.ADVISOR_BUTTON_SIZE / 6, self.ADVISOR_BUTTON_SIZE / 6)
		self.appendtoHideState(screen, "SettingsAdvisorButton", HIDE_TYPE_MAP, HIDE_LEVEL_HIDE)
		iBtnX += self.ADVISOR_BUTTON_SPACING

		screen.setImageButton("InfoAdvisorButton", ArtFileMgr.getInterfaceArtInfo("INTERFACE_INFO").getPath(), iBtnX, (TOP_CENTER_HUD_HEIGHT - self.ADVISOR_BUTTON_SIZE) / 2, self.ADVISOR_BUTTON_SIZE, self.ADVISOR_BUTTON_SIZE, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_INFO).getActionInfoIndex(), -1 )
		screen.setImageShape("InfoAdvisorButton", ImageShapes.IMAGE_SHAPE_ELLIPSE, -1)
		screen.setHitMargins("InfoAdvisorButton", self.ADVISOR_BUTTON_SIZE / 6, self.ADVISOR_BUTTON_SIZE / 6)
		self.appendtoHideState(screen, "InfoAdvisorButton", HIDE_TYPE_MAP, HIDE_LEVEL_HIDE)
		#iBtnX += self.ADVISOR_BUTTON_SPACING

	# MINIMAP RING
		screen.addPanel("MiniMapRing", u"", u"", True, False, 0, yResolution - SADDLE_HEIGHT, SADDLE_HEIGHT, SADDLE_HEIGHT, PanelStyles.PANEL_STYLE_STANDARD, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.addDrawControl("MiniMapRing", ArtFileMgr.getInterfaceArtInfo("SCREEN_MINI_MAP_RING").getPath(), 0, yResolution - SADDLE_HEIGHT, SADDLE_HEIGHT, SADDLE_HEIGHT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		self.appendtoHideState(screen, "MiniMapRing", HIDE_TYPE_MAP, HIDE_LEVEL_HIDE)
		self.appendtoHideState(screen, "MiniMapRing", HIDE_TYPE_GLOBAL, HIDE_LEVEL_HIDE)

	# MINIMAP INITIALIZATION
		self.MINIMAP_SIDE_MARGIN = MINIMAP_HEIGHT * 13 / 100
		screen.initMinimap(self.MINIMAP_SIDE_MARGIN, MINIMAP_HEIGHT - self.MINIMAP_SIDE_MARGIN, yResolution - MINIMAP_HEIGHT + self.MINIMAP_SIDE_MARGIN, yResolution - self.MINIMAP_SIDE_MARGIN, -0.1, true )
		self.appendtoHideState(screen, "_FXS_Screen_Bogus_Minimap_Name", HIDE_TYPE_MAP, HIDE_LEVEL_ALL)
		self.appendtoHideState(screen, "_FXS_Screen_Bogus_Minimap_Name", HIDE_TYPE_CITY, HIDE_LEVEL_ALL)
		self.appendtoHideState(screen, "_FXS_Screen_Bogus_Minimap_Name", HIDE_TYPE_GLOBAL, HIDE_LEVEL_ALL)

	# UNIT HEALTH BAR
		screen.addStackedBarGFC("UnitHealthBar", xResolution - UNIT_HEALTH_BAR_WIDTH - 10, yResolution - LOWER_RIGHT_CORNER_BACKGROUND_HEIGHT - STACK_BAR_HEIGHT, UNIT_HEALTH_BAR_WIDTH, STACK_BAR_HEIGHT, InfoBarTypes.NUM_INFOBAR_TYPES, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.setStackedBarColors("UnitHealthBar", InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_CULTURE_STORED") )
		screen.setStackedBarColors("UnitHealthBar", InfoBarTypes.INFOBAR_RATE, gc.getInfoTypeForString("COLOR_CULTURE_RATE") )
		screen.setStackedBarColors("UnitHealthBar", InfoBarTypes.INFOBAR_RATE_EXTRA, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.setStackedBarColors("UnitHealthBar", InfoBarTypes.INFOBAR_EMPTY, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.hide("UnitHealthBar")

	# MINIMAP BUTTONS
		self.MINIMAP_BUTTON_SPACING = MEDIUM_BUTTON_SIZE
		self.MINIMAP_BUTTON_LEFT_OFFSET = 49

		self.ANGLE_SEPERATION = 90 / 4
		iAngle = 0
		iRadius = SADDLE_HEIGHT * 0.43
		self.MINI_MAP_BUTTON_SIZE = SMALL_BUTTON_SIZE * 2

		self.ButtonList = ["INTERFACE_MAP_UNIT", "INTERFACE_MAP_GRID_TOGGLE", "INTERFACE_MAP_SHOW_WORLD", "INTERFACE_MAP_INFO", "INTERFACE_MAP_ZOOM", "INTERFACE_MAP_YIELD", "INTERFACE_MAP_LIST"]
		self.ControlList = [ControlTypes.CONTROL_UNIT_ICONS, ControlTypes.CONTROL_GRID, ControlTypes.CONTROL_BARE_MAP, ControlTypes.CONTROL_RESOURCE_ALL, ControlTypes.CONTROL_GLOBELAYER, ControlTypes.CONTROL_YIELDS, ControlTypes.CONTROL_SCORES] 
		self.ButtonNames = ["UnitIcons", "Grid", "BareMap", "ResourceIcons", "GlobeToggle", "Yields", "ScoresVisible"]

		for iButtonIndex in range(len(self.ButtonList)):
			Xcord = math.sin(math.radians(self.ANGLE_SEPERATION * (iButtonIndex + 1))) * int(iRadius) + int(SADDLE_HEIGHT / 2)
			Ycord = math.cos(math.radians(self.ANGLE_SEPERATION * (iButtonIndex + 1))) * int(iRadius) - int(SADDLE_HEIGHT / 2) + yResolution + 3

			szName = self.ButtonNames[iButtonIndex]
			screen.setImageButton(szName, ArtFileMgr.getInterfaceArtInfo(self.ButtonList[iButtonIndex]).getPath(), int(Xcord) - (self.MINI_MAP_BUTTON_SIZE / 2), int(Ycord) - (self.MINI_MAP_BUTTON_SIZE / 2), self.MINI_MAP_BUTTON_SIZE, self.MINI_MAP_BUTTON_SIZE, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(self.ControlList[iButtonIndex]).getActionInfoIndex(), -1)
			screen.setImageShape(szName, ImageShapes.IMAGE_SHAPE_ELLIPSE, -1)
			screen.setHitMargins(szName, 5, 5)
			self.appendtoHideState(screen, szName, HIDE_TYPE_GLOBAL, HIDE_LEVEL_ALL)
			self.appendtoHideState(screen, szName, HIDE_TYPE_MAP, HIDE_LEVEL_HIDE)

	# GLOBAL VIEW BUTTONS
		for i in range (CyGlobeLayerManager().getNumLayers()):
			szButtonID = "GlobeLayer" + str(i)
			szStyle = ArtFileMgr.getInterfaceArtInfo(CyGlobeLayerManager().getLayer(i).getButtonStyle()).getPath()
			
			Xcord = math.sin(math.radians(self.ANGLE_SEPERATION * (i + 1))) * int(iRadius) + int(SADDLE_HEIGHT / 2)
			Ycord = math.cos(math.radians(self.ANGLE_SEPERATION * (i + 1))) * int(iRadius) - int(SADDLE_HEIGHT / 2) + yResolution + 3

			screen.setImageButton( szButtonID, szStyle, int(Xcord) - (self.MINI_MAP_BUTTON_SIZE / 2), int(Ycord) - (self.MINI_MAP_BUTTON_SIZE / 2), self.MINI_MAP_BUTTON_SIZE, self.MINI_MAP_BUTTON_SIZE, WidgetTypes.WIDGET_GLOBELAYER, i, -1)
			screen.setImageShape(szButtonID, ImageShapes.IMAGE_SHAPE_ELLIPSE, -1)
			screen.setHitMargins(szButtonID, 5, 5)
			screen.hide( szButtonID )

	# MAIN MENU AND PEDIA
		SpaceAvailable = (xResolution / 2) - (TOP_CENTER_HUD_WIDTH / 2)
		self.MENU_BUTTON_SPACING = SpaceAvailable / 9

		self.MENU_BUTTON_SIZE = SMALL_BUTTON_SIZE * 2

		screen.setImageButton("InterfaceHelpButton", ArtFileMgr.getInterfaceArtInfo("INTERFACE_GENERAL_CIVILOPEDIA_ICON").getPath(), (xResolution / 2) - (TOP_CENTER_HUD_WIDTH / 2) - (1 * self.MENU_BUTTON_SPACING ), (TOP_CENTER_HUD_HEIGHT - self.MENU_BUTTON_SIZE) / 2, self.MENU_BUTTON_SIZE, self.MENU_BUTTON_SIZE, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_CIVILOPEDIA).getActionInfoIndex(), -1 )
		screen.setImageShape("InterfaceHelpButton", ImageShapes.IMAGE_SHAPE_ELLIPSE, -1)
		screen.setHitMargins("InterfaceHelpButton", self.MENU_BUTTON_SIZE / 6, self.MENU_BUTTON_SIZE / 6)
		self.appendtoHideState(screen, "InterfaceHelpButton", HIDE_TYPE_MAP, HIDE_LEVEL_HIDE)

		screen.setImageButton("MainMenuButton", ArtFileMgr.getInterfaceArtInfo("INTERFACE_GENERAL_MENU_ICON").getPath(), (xResolution / 2) - (TOP_CENTER_HUD_WIDTH / 2) - (2 * self.MENU_BUTTON_SPACING ), (TOP_CENTER_HUD_HEIGHT - self.MENU_BUTTON_SIZE) / 2, self.MENU_BUTTON_SIZE, self.MENU_BUTTON_SIZE, WidgetTypes.WIDGET_MENU_ICON, -1, -1 )
		screen.setImageShape("MainMenuButton", ImageShapes.IMAGE_SHAPE_ELLIPSE, -1)
		screen.setHitMargins("MainMenuButton", self.MENU_BUTTON_SIZE / 6, self.MENU_BUTTON_SIZE / 6)
		self.appendtoHideState(screen, "MainMenuButton", HIDE_TYPE_MAP, HIDE_LEVEL_HIDE)

		screen.setImageButton("TurnLogButton", ArtFileMgr.getInterfaceArtInfo("INTERFACE_GENERAL_TURNLOG_ICON").getPath(), (xResolution / 2) - (TOP_CENTER_HUD_WIDTH / 2) - (3 * self.MENU_BUTTON_SPACING ), (TOP_CENTER_HUD_HEIGHT - self.MENU_BUTTON_SIZE) / 2, self.MENU_BUTTON_SIZE, self.MENU_BUTTON_SIZE, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_TURN_LOG).getActionInfoIndex(), -1 )
		screen.setImageShape("TurnLogButton", ImageShapes.IMAGE_SHAPE_ELLIPSE, -1)
		screen.setHitMargins("TurnLogButton", self.MENU_BUTTON_SIZE / 6, self.MENU_BUTTON_SIZE / 6)
		self.appendtoHideState(screen, "TurnLogButton", HIDE_TYPE_MAP, HIDE_LEVEL_HIDE)

		screen.setImageButton("DiplomacyButton", ArtFileMgr.getInterfaceArtInfo("INTERFACE_GENERAL_DIPLOMACY").getPath(), (xResolution / 2) - (TOP_CENTER_HUD_WIDTH / 2) - (4 * self.MENU_BUTTON_SPACING ), (TOP_CENTER_HUD_HEIGHT - self.MENU_BUTTON_SIZE) / 2, self.MENU_BUTTON_SIZE, self.MENU_BUTTON_SIZE, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_DIPLOMACY).getActionInfoIndex(), -1 )
		screen.setImageShape("DiplomacyButton", ImageShapes.IMAGE_SHAPE_ELLIPSE, -1)
		screen.setHitMargins("DiplomacyButton", self.MENU_BUTTON_SIZE / 6, self.MENU_BUTTON_SIZE / 6)
		self.appendtoHideState(screen, "DiplomacyButton", HIDE_TYPE_MAP, HIDE_LEVEL_HIDE)

	# GOLD PILE
		screen.addDDSGFC("GoldPile", ArtFileMgr.getInterfaceArtInfo("SCREEN_GOLD_PILE").getPath(), 0, yResolution - 90, 80, 80, WidgetTypes.WIDGET_GENERAL, -1, -1)
		screen.setImageShape("GoldPile", ImageShapes.IMAGE_SHAPE_ELLIPSE, -1)
		screen.setHitMargins("GoldPile", 30, 30)
		self.appendtoHideState(screen, "GoldPile", HIDE_TYPE_MAP, HIDE_LEVEL_HIDE)

	# AUTOMATE PRODUCTION & CITIZEN BUTTON
		screen.addCheckBoxGFC("AutomateProduction", ArtFileMgr.getInterfaceArtInfo("INTERFACE_CITY_AUTOMATE_PRODUCTION").getPath(), ArtFileMgr.getInterfaceArtInfo("INTERFACE_HIGHLIGHTED_BUTTON").getPath(), CITIZEN_BAR_WIDTH + ((xResolution - CITIZEN_BAR_WIDTH) * 3 / 7) , yResolution - BOTTOM_CENTER_HUD_HEIGHT - TRANSPORT_AREA_HEIGHT - (STACK_BAR_HEIGHT * 2) - (SMALL_BUTTON_SIZE * 3 / 2), SMALL_BUTTON_SIZE * 2, SMALL_BUTTON_SIZE * 2, WidgetTypes.WIDGET_AUTOMATE_PRODUCTION, -1, -1, ButtonStyles.BUTTON_STYLE_LABEL )
		self.appendtoHideState(screen, "AutomateProduction", HIDE_TYPE_CITY, HIDE_LEVEL_HIDE)

		screen.setImageButton("AutomateCitizens", ArtFileMgr.getInterfaceArtInfo("INTERFACE_CITY_AUTOMATE_CITIZENS").getPath(), CITIZEN_BAR_WIDTH - (SMALL_BUTTON_SIZE * 3), yResolution - BOTTOM_CENTER_HUD_HEIGHT - (STACK_BAR_HEIGHT * 2) - SMALL_BUTTON_SIZE, SMALL_BUTTON_SIZE * 2, SMALL_BUTTON_SIZE * 2, WidgetTypes.WIDGET_AUTOMATE_CITIZENS, 1, -1)
		self.appendtoHideState(screen, "AutomateCitizens", HIDE_TYPE_CITY, HIDE_LEVEL_HIDE)

		screen.setImageButton("LockCitizens", ArtFileMgr.getInterfaceArtInfo("INTERFACE_CITY_CITIZEN_LOCK").getPath(), CITIZEN_BAR_WIDTH - (SMALL_BUTTON_SIZE * 5), yResolution - BOTTOM_CENTER_HUD_HEIGHT - (STACK_BAR_HEIGHT * 2) - SMALL_BUTTON_SIZE, SMALL_BUTTON_SIZE * 2, SMALL_BUTTON_SIZE * 2, WidgetTypes.WIDGET_AUTOMATE_CITIZENS, 0, -1)
		self.appendtoHideState(screen, "LockCitizens", HIDE_TYPE_CITY, HIDE_LEVEL_HIDE)

	# RESOURCE TABLE
		screen.addScrollPanel("ResourceTable", u"", 0, yResolution - BOTTOM_CENTER_HUD_HEIGHT, xResolution, BOTTOM_CENTER_HUD_HEIGHT, PanelStyles.PANEL_STYLE_STANDARD, false, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.hide("ResourceTable")
		
		self.RESOURCE_LABELS = ["", ""]
		self.RESOURCE_LABELS[0] = localText.getText("TXT_KEY_SCREEN_STORED", ())
		self.RESOURCE_LABELS[1] = localText.getText("TXT_KEY_SCREEN_NET", ())
		
		TableYields = []
		for iYield in range(YieldTypes.NUM_YIELD_TYPES):
			if gc.getYieldInfo(iYield).isCargo():
				TableYields.append(iYield)

		global RESOURCE_TABLE_COLUMN_WIDTH
		RESOURCE_TABLE_COLUMN_WIDTH = int((xResolution - (STACK_BAR_HEIGHT * 2)) / len(TableYields))
		ArtPath = ""

		for iYield in TableYields:
			szName = "BonusPane" + str(iYield)
			screen.attachPanelAt("ResourceTable", szName, u"", u"", True, True, PanelStyles.PANEL_STYLE_STANDARD, STACK_BAR_HEIGHT + (iYield * RESOURCE_TABLE_COLUMN_WIDTH), yResolution - BOTTOM_CENTER_HUD_HEIGHT, RESOURCE_TABLE_COLUMN_WIDTH, BOTTOM_CENTER_HUD_HEIGHT - STACK_BAR_HEIGHT, WidgetTypes.WIDGET_RECEIVE_MOVE_CARGO_TO_CITY, -1, -1 )
			if (iYield == 0):
				ArtPath = ArtFileMgr.getInterfaceArtInfo("INTERFACE_BOX_START").getPath()
			elif (iYield == (len(TableYields) - 1)):
				ArtPath = ArtFileMgr.getInterfaceArtInfo("INTERFACE_BOX_END").getPath()
			else:
				ArtPath = ArtFileMgr.getInterfaceArtInfo("INTERFACE_BOX_MID").getPath()
			screen.addDrawControl(szName, ArtPath, STACK_BAR_HEIGHT + (iYield * RESOURCE_TABLE_COLUMN_WIDTH), yResolution - BOTTOM_CENTER_HUD_HEIGHT + STACK_BAR_HEIGHT, RESOURCE_TABLE_COLUMN_WIDTH, BOTTOM_CENTER_HUD_HEIGHT - (STACK_BAR_HEIGHT * 3 / 2), WidgetTypes.WIDGET_RECEIVE_MOVE_CARGO_TO_CITY, -1, -1)
			self.appendtoHideState(screen, szName, HIDE_TYPE_CITY, HIDE_LEVEL_NORMAL)

			szName = "BonusPanePos" + str(iYield)
			screen.attachPanelAt("ResourceTable", szName, u"", u"", True, True, PanelStyles.PANEL_STYLE_STANDARD, STACK_BAR_HEIGHT + (iYield * RESOURCE_TABLE_COLUMN_WIDTH), yResolution - BOTTOM_CENTER_HUD_HEIGHT, RESOURCE_TABLE_COLUMN_WIDTH, BOTTOM_CENTER_HUD_HEIGHT - STACK_BAR_HEIGHT, WidgetTypes.WIDGET_RECEIVE_MOVE_CARGO_TO_CITY, -1, -1 )
			if (iYield == 0):
				ArtPath = ArtFileMgr.getInterfaceArtInfo("INTERFACE_BOX_POS_START").getPath()
			elif (iYield == (len(TableYields) - 1)):
				ArtPath = ArtFileMgr.getInterfaceArtInfo("INTERFACE_BOX_POS_END").getPath()
			else:
				ArtPath = ArtFileMgr.getInterfaceArtInfo("INTERFACE_BOX_POS_MID").getPath()
			screen.addDrawControl(szName, ArtPath, STACK_BAR_HEIGHT + (iYield * RESOURCE_TABLE_COLUMN_WIDTH), yResolution - BOTTOM_CENTER_HUD_HEIGHT + STACK_BAR_HEIGHT, RESOURCE_TABLE_COLUMN_WIDTH, BOTTOM_CENTER_HUD_HEIGHT - (STACK_BAR_HEIGHT * 3 / 2), WidgetTypes.WIDGET_RECEIVE_MOVE_CARGO_TO_CITY, -1, -1 )
			self.appendtoHideState(screen, szName, HIDE_TYPE_CITY, HIDE_LEVEL_NORMAL)
			
			szName = "BonusPaneNeg" + str(iYield)
			screen.attachPanelAt("ResourceTable", szName, u"", u"", True, True, PanelStyles.PANEL_STYLE_STANDARD, STACK_BAR_HEIGHT + (iYield * RESOURCE_TABLE_COLUMN_WIDTH), yResolution - BOTTOM_CENTER_HUD_HEIGHT, RESOURCE_TABLE_COLUMN_WIDTH, BOTTOM_CENTER_HUD_HEIGHT - STACK_BAR_HEIGHT, WidgetTypes.WIDGET_RECEIVE_MOVE_CARGO_TO_CITY, -1, -1 )
			if (iYield == 0):
				ArtPath = ArtFileMgr.getInterfaceArtInfo("INTERFACE_BOX_NEG_START").getPath()
			elif (iYield == (len(TableYields) - 1)):
				ArtPath = ArtFileMgr.getInterfaceArtInfo("INTERFACE_BOX_NEG_END").getPath()
			else:
				ArtPath = ArtFileMgr.getInterfaceArtInfo("INTERFACE_BOX_NEG_MID").getPath()
			screen.addDrawControl(szName, ArtPath, STACK_BAR_HEIGHT + (iYield * RESOURCE_TABLE_COLUMN_WIDTH), yResolution - BOTTOM_CENTER_HUD_HEIGHT + STACK_BAR_HEIGHT, RESOURCE_TABLE_COLUMN_WIDTH, BOTTOM_CENTER_HUD_HEIGHT - (STACK_BAR_HEIGHT * 3 / 2), WidgetTypes.WIDGET_RECEIVE_MOVE_CARGO_TO_CITY, -1, -1 )
			self.appendtoHideState(screen, szName, HIDE_TYPE_CITY, HIDE_LEVEL_NORMAL)
			
			screen.setLabel("YieldStoredlabel" + str(iYield), "ResourceTable", "", CvUtil.FONT_CENTER_JUSTIFY, STACK_BAR_HEIGHT + (iYield * RESOURCE_TABLE_COLUMN_WIDTH) + (RESOURCE_TABLE_COLUMN_WIDTH / 2), yResolution - (STACK_BAR_HEIGHT * 3), -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_RECEIVE_MOVE_CARGO_TO_CITY, -1, -1 )			
			screen.setLabel("YieldNetlabel" + str(iYield), "ResourceTable", "", CvUtil.FONT_CENTER_JUSTIFY, STACK_BAR_HEIGHT + (iYield * RESOURCE_TABLE_COLUMN_WIDTH) + (RESOURCE_TABLE_COLUMN_WIDTH / 2), yResolution - (STACK_BAR_HEIGHT * 2), -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_RECEIVE_MOVE_CARGO_TO_CITY, -1, -1 )			

			screen.addDragableButton("YieldIcon" + str(iYield), gc.getYieldInfo(iYield).getIcon(), "", STACK_BAR_HEIGHT + (iYield * RESOURCE_TABLE_COLUMN_WIDTH), yResolution - BOTTOM_CENTER_HUD_HEIGHT, RESOURCE_TABLE_COLUMN_WIDTH, RESOURCE_TABLE_COLUMN_WIDTH, WidgetTypes.WIDGET_MOVE_CARGO_TO_TRANSPORT, iYield, -1, ButtonStyles.BUTTON_STYLE_IMAGE )
			self.appendtoHideState(screen, "YieldIcon" + str(iYield), HIDE_TYPE_CITY, HIDE_LEVEL_NORMAL)

		screen.addScrollPanel("CityReciveCargo", u"", 0, yResolution - BOTTOM_CENTER_HUD_HEIGHT, xResolution, BOTTOM_CENTER_HUD_HEIGHT, PanelStyles.PANEL_STYLE_MAIN, false, WidgetTypes.WIDGET_RECEIVE_MOVE_CARGO_TO_CITY, -1, -1 )
		self.appendtoHideState(screen, "CityReciveCargo", HIDE_TYPE_CITY, HIDE_LEVEL_NORMAL)

		for iYield in TableYields:
			screen.moveToFront("YieldIcon" + str(iYield))

	# MAP YIELD EMPHASIZE
		iYieldCount = 0
		ButtonSize = LARGE_BUTTON_SIZE
		
		xStart = LOWER_LEFT_CORNER_BACKGROUND_WIDTH
		IconSpacing = int((xResolution - LOWER_RIGHT_CORNER_BACKGROUND_WIDTH - xStart) / max(1, len(EMPHASIZEYIELDS)))
		
		for iYield in EMPHASIZEYIELDS:
			screen.addDDSGFC("MapYieldEmphasize" + str(iYield), ArtFileMgr.getInterfaceArtInfo("INTERFACE_EMPHASIZE_LOZENGE").getPath(), xStart + (iYieldCount * IconSpacing), yResolution - (ButtonSize * 3 / 2), ButtonSize, ButtonSize, WidgetTypes.WIDGET_GENERAL, -1, -1)
			screen.addDDSGFC("MapYieldDe-Emphasize" + str(iYield), ArtFileMgr.getInterfaceArtInfo("INTERFACE_DEEMPHASIZE_LOZENGE").getPath(), xStart + (iYieldCount * IconSpacing), yResolution - (ButtonSize * 3 / 2), ButtonSize, ButtonSize, WidgetTypes.WIDGET_GENERAL, -1, -1)
			screen.setImageButton("MapYieldIcon" + str(iYield), gc.getYieldInfo(iYield).getIcon(), xStart + (iYieldCount * IconSpacing), yResolution - (ButtonSize * 3 / 2), ButtonSize, ButtonSize, WidgetTypes.WIDGET_EMPHASIZE, iYield, -1)
			iYieldCount += 1

			screen.hide("MapYieldIcon" + str(iYield))
			screen.hide("MapYieldEmphasize" + str(iYield))
			screen.hide("MapYieldDe-Emphasize" + str(iYield))
			
	# YIELD LOZENGE
		screen.addDDSGFC("BonusPaneNeg" + str(int(YieldTypes.YIELD_BELLS)), ArtFileMgr.getInterfaceArtInfo("INTERFACE_DEEMPHASIZE_LOZENGE").getPath(), (xResolution * 30 / 100) - STACK_BAR_HEIGHT, STACK_BAR_HEIGHT / 3, STACK_BAR_HEIGHT * 2, STACK_BAR_HEIGHT , WidgetTypes.WIDGET_GENERAL, -1, -1)
		self.appendtoHideState(screen, "BonusPaneNeg" + str(int(YieldTypes.YIELD_BELLS)), HIDE_TYPE_CITY, HIDE_LEVEL_NORMAL)
		screen.addDDSGFC("BonusPaneNeg" + str(int(YieldTypes.YIELD_CROSSES)), ArtFileMgr.getInterfaceArtInfo("INTERFACE_DEEMPHASIZE_LOZENGE").getPath(), (xResolution * 70 / 100) - STACK_BAR_HEIGHT, STACK_BAR_HEIGHT / 3, STACK_BAR_HEIGHT * 2, STACK_BAR_HEIGHT , WidgetTypes.WIDGET_GENERAL, -1, -1)
		self.appendtoHideState(screen, "BonusPaneNeg" + str(int(YieldTypes.YIELD_CROSSES)), HIDE_TYPE_CITY, HIDE_LEVEL_NORMAL)
		screen.addDDSGFC("BonusPaneNeg" + str(int(YieldTypes.YIELD_HAMMERS)), ArtFileMgr.getInterfaceArtInfo("INTERFACE_DEEMPHASIZE_LOZENGE").getPath(), (xResolution * 25 / 100) - STACK_BAR_HEIGHT, STACK_BAR_HEIGHT / 3, STACK_BAR_HEIGHT * 2, STACK_BAR_HEIGHT , WidgetTypes.WIDGET_GENERAL, -1, -1)
		self.appendtoHideState(screen, "BonusPaneNeg" + str(int(YieldTypes.YIELD_HAMMERS)), HIDE_TYPE_CITY, HIDE_LEVEL_NORMAL)
		screen.addDDSGFC("BonusPaneNeg" + str(int(YieldTypes.YIELD_EDUCATION)), ArtFileMgr.getInterfaceArtInfo("INTERFACE_DEEMPHASIZE_LOZENGE").getPath(), (xResolution * 75 / 100) - STACK_BAR_HEIGHT, STACK_BAR_HEIGHT / 3, STACK_BAR_HEIGHT * 2, STACK_BAR_HEIGHT , WidgetTypes.WIDGET_GENERAL, -1, -1)
		self.appendtoHideState(screen, "BonusPaneNeg" + str(int(YieldTypes.YIELD_EDUCATION)), HIDE_TYPE_CITY, HIDE_LEVEL_NORMAL)

		screen.addDDSGFC("BonusPanePos" + str(int(YieldTypes.YIELD_BELLS)), ArtFileMgr.getInterfaceArtInfo("INTERFACE_EMPHASIZE_LOZENGE").getPath(), (xResolution * 30 / 100) - STACK_BAR_HEIGHT, STACK_BAR_HEIGHT / 3, STACK_BAR_HEIGHT * 2, STACK_BAR_HEIGHT , WidgetTypes.WIDGET_GENERAL, -1, -1)
		self.appendtoHideState(screen, "BonusPanePos" + str(int(YieldTypes.YIELD_BELLS)), HIDE_TYPE_CITY, HIDE_LEVEL_NORMAL)
		screen.addDDSGFC("BonusPanePos" + str(int(YieldTypes.YIELD_CROSSES)), ArtFileMgr.getInterfaceArtInfo("INTERFACE_EMPHASIZE_LOZENGE").getPath(), (xResolution * 70 / 100) - STACK_BAR_HEIGHT, STACK_BAR_HEIGHT / 3, STACK_BAR_HEIGHT * 2, STACK_BAR_HEIGHT , WidgetTypes.WIDGET_GENERAL, -1, -1)
		self.appendtoHideState(screen, "BonusPanePos" + str(int(YieldTypes.YIELD_CROSSES)), HIDE_TYPE_CITY, HIDE_LEVEL_NORMAL)
		screen.addDDSGFC("BonusPanePos" + str(int(YieldTypes.YIELD_HAMMERS)), ArtFileMgr.getInterfaceArtInfo("INTERFACE_EMPHASIZE_LOZENGE").getPath(), (xResolution * 25 / 100) - STACK_BAR_HEIGHT, STACK_BAR_HEIGHT / 3, STACK_BAR_HEIGHT * 2, STACK_BAR_HEIGHT , WidgetTypes.WIDGET_GENERAL, -1, -1)
		self.appendtoHideState(screen, "BonusPanePos" + str(int(YieldTypes.YIELD_HAMMERS)), HIDE_TYPE_CITY, HIDE_LEVEL_NORMAL)
		screen.addDDSGFC("BonusPanePos" + str(int(YieldTypes.YIELD_EDUCATION)), ArtFileMgr.getInterfaceArtInfo("INTERFACE_EMPHASIZE_LOZENGE").getPath(), (xResolution * 75 / 100) - STACK_BAR_HEIGHT, STACK_BAR_HEIGHT / 3, STACK_BAR_HEIGHT * 2, STACK_BAR_HEIGHT , WidgetTypes.WIDGET_GENERAL, -1, -1)
		self.appendtoHideState(screen, "BonusPanePos" + str(int(YieldTypes.YIELD_EDUCATION)), HIDE_TYPE_CITY, HIDE_LEVEL_NORMAL)
		
	# MAP PLOT LIST BUTTONS
		self.END_TURN_TEXT_BOTTOM_MARGIN = 55
		self.PLOT_LIST_BUTTON_SPACER = 7
		self.PLOT_LIST_HEALTH_BAR_SHORTENING_CORRECTION = 6
		self.PLOT_LIST_HEALTH_BAR_HEIGHT = 11
		self.PLOT_LIST_ICON_SIZE = 12
		
		self.PLOT_LIST_BUTTON_AREA_MARGIN = 10
		Width = xResolution - (LOWER_LEFT_CORNER_BACKGROUND_WIDTH + LOWER_RIGHT_CORNER_BACKGROUND_WIDTH )
		Height = LARGE_BUTTON_SIZE + self.PLOT_LIST_BUTTON_SPACER
		global NUM_PLOT_LIST_BUTTONS
		NUM_PLOT_LIST_BUTTONS = Width / (LARGE_BUTTON_SIZE + self.PLOT_LIST_BUTTON_SPACER)

		for j in range(gc.getMAX_PLOT_LIST_ROWS()):
			yRow = (j - gc.getMAX_PLOT_LIST_ROWS()) * (LARGE_BUTTON_SIZE + self.PLOT_LIST_BUTTON_SPACER) - BOTTOM_CENTER_HUD_HEIGHT - self.END_TURN_TEXT_BOTTOM_MARGIN
			yPixel = yResolution + yRow
			xPixel = LOWER_LEFT_CORNER_BACKGROUND_WIDTH

			szStringPanel = "PlotListPanelMulti" + str(j)
			screen.addPanel(szStringPanel, u"", u"", True, False, xPixel, yPixel, Width, Height, PanelStyles.PANEL_STYLE_EMPTY, WidgetTypes.WIDGET_GENERAL, -1, -1)
			self.appendtoHideState(screen, "PlotListPanelMulti", HIDE_TYPE_MAP, HIDE_LEVEL_ALL)

			for i in range(self.numPlotListButtons()):
				k = j * self.numPlotListButtons() + i
				xOffset = i * (LARGE_BUTTON_SIZE + self.PLOT_LIST_BUTTON_SPACER)

				szString = "PlotListButtonMulti" + str(k)
				screen.addCheckBoxGFCAt(szStringPanel, szString, ArtFileMgr.getInterfaceArtInfo("INTERFACE_BUTTONS_GOVERNOR").getPath(), ArtFileMgr.getInterfaceArtInfo("BUTTON_HILITE_SQUARE").getPath(), xOffset, 0, LARGE_BUTTON_SIZE, LARGE_BUTTON_SIZE, WidgetTypes.WIDGET_PLOT_LIST, k, -1, ButtonStyles.BUTTON_STYLE_LABEL )
				screen.hide( szString )

				szStringHealth = szString + "Health"
				screen.addStackedBarGFCAt( szStringHealth, szStringPanel, xOffset, LARGE_BUTTON_SIZE - self.PLOT_LIST_HEALTH_BAR_SHORTENING_CORRECTION, LARGE_BUTTON_SIZE, self.PLOT_LIST_HEALTH_BAR_HEIGHT, InfoBarTypes.NUM_INFOBAR_TYPES, WidgetTypes.WIDGET_GENERAL, k, -1 )
				screen.hide( szStringHealth )

				szStringIcon = szString + "Icon"
				szFileName = ArtFileMgr.getInterfaceArtInfo("OVERLAY_MOVE").getPath()
				screen.addDDSGFCAt( szStringIcon, szStringPanel, szFileName, xOffset, 0, self.PLOT_LIST_ICON_SIZE, self.PLOT_LIST_ICON_SIZE, WidgetTypes.WIDGET_PLOT_LIST, k, -1, False )
				screen.hide( szStringIcon )

		yPixel = yResolution - (LARGE_BUTTON_SIZE + self.PLOT_LIST_BUTTON_SPACER)
		xPixel = LOWER_LEFT_CORNER_BACKGROUND_WIDTH

		szStringPanel = "PlotListPanel"
		screen.addPanel(szStringPanel, u"", u"", True, False, xPixel, yPixel, Width, Height, PanelStyles.PANEL_STYLE_EMPTY, WidgetTypes.WIDGET_GENERAL, -1, -1)
		self.appendtoHideState(screen, "PlotListPanel", HIDE_TYPE_MAP, HIDE_LEVEL_ALL)

		for i in range(self.numPlotListButtons()):
			xOffset = i * (LARGE_BUTTON_SIZE + self.PLOT_LIST_BUTTON_SPACER)

			szString = "PlotListButton" + str(i)
			screen.addCheckBoxGFCAt(szStringPanel, szString, ArtFileMgr.getInterfaceArtInfo("INTERFACE_BUTTONS_GOVERNOR").getPath(), ArtFileMgr.getInterfaceArtInfo("BUTTON_HILITE_SQUARE").getPath(), xOffset, 0, LARGE_BUTTON_SIZE, LARGE_BUTTON_SIZE, WidgetTypes.WIDGET_PLOT_LIST, i, -1, ButtonStyles.BUTTON_STYLE_LABEL )
			screen.hide( szString )

			szStringHealth = szString + "Health"
			screen.addStackedBarGFCAt( szStringHealth, szStringPanel, xOffset, LARGE_BUTTON_SIZE - self.PLOT_LIST_HEALTH_BAR_SHORTENING_CORRECTION, LARGE_BUTTON_SIZE, self.PLOT_LIST_HEALTH_BAR_HEIGHT, InfoBarTypes.NUM_INFOBAR_TYPES, WidgetTypes.WIDGET_GENERAL, i, -1 )
			screen.hide( szStringHealth )

			szStringIcon = szString + "Icon"
			szFileName = ArtFileMgr.getInterfaceArtInfo("OVERLAY_MOVE").getPath()
			screen.addDDSGFCAt( szStringIcon, szStringPanel, szFileName, xOffset, 0, self.PLOT_LIST_ICON_SIZE, self.PLOT_LIST_ICON_SIZE, WidgetTypes.WIDGET_PLOT_LIST, i, -1, False )
			screen.hide( szStringIcon )

	# ENDTURN TEXT & BUTTON
		screen.setLabel("EndTurnText", "Background", u"", CvUtil.FONT_CENTER_JUSTIFY, 0, yResolution - (BOTTOM_CENTER_HUD_HEIGHT + self.END_TURN_TEXT_BOTTOM_MARGIN), -0.1, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.setHitTest("EndTurnText", HitTestTypes.HITTEST_NOHIT )

		self.CENTER_POINT_HEIGHT = 1.45
		self.CENTER_HUD_HEIGHT = 0.095

		Xcord = (math.sin(math.radians(11.5)) * int(yResolution * self.CENTER_POINT_HEIGHT)) * -1 * ASPECT_ADJUSTMENT + ((xResolution) / 2)
		Ycord = (math.cos(math.radians(11.5)) * int(yResolution * self.CENTER_POINT_HEIGHT)) - (yResolution * self.CENTER_POINT_HEIGHT) + yResolution - (yResolution * self.CENTER_HUD_HEIGHT)

		screen.addPanel("EndTurnRing", u"", u"", True, False, int(Xcord) - LARGE_BUTTON_SIZE, int(Ycord) - LARGE_BUTTON_SIZE, LARGE_BUTTON_SIZE * 2, LARGE_BUTTON_SIZE * 2, PanelStyles.PANEL_STYLE_STANDARD, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.addDrawControl("EndTurnRing", ArtFileMgr.getInterfaceArtInfo("SCREEN_END_TURN_RING").getPath(), int(Xcord) - LARGE_BUTTON_SIZE, int(Ycord) - LARGE_BUTTON_SIZE, LARGE_BUTTON_SIZE * 2, LARGE_BUTTON_SIZE * 2, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		self.appendtoHideState(screen, "EndTurnRing", HIDE_TYPE_MAP, HIDE_LEVEL_NORMAL)
		self.appendtoHideState(screen, "EndTurnRing", HIDE_TYPE_GLOBAL, HIDE_LEVEL_NORMAL)

		screen.setImageButton("EndTurnButton", "", int(Xcord) - LARGE_BUTTON_SIZE, int(Ycord) - LARGE_BUTTON_SIZE, LARGE_BUTTON_SIZE * 2, LARGE_BUTTON_SIZE * 2, WidgetTypes.WIDGET_END_TURN, -1, -1 )
		screen.setStyle("EndTurnButton", "Button_HUDEndTurn_Style")
		screen.setEndTurnState("EndTurnButton", "Red")
		screen.hide("EndTurnButton")

	# BUILDING CONSTRUCTION BAR
		screen.addStackedBarGFC("CityProductionBar", CITIZEN_BAR_WIDTH + (SMALL_BUTTON_SIZE), yResolution - BOTTOM_CENTER_HUD_HEIGHT - TRANSPORT_AREA_HEIGHT - (STACK_BAR_HEIGHT ), xResolution - CITIZEN_BAR_WIDTH - (MAP_EDGE_MARGIN_WIDTH * 3) - (SMALL_BUTTON_SIZE / 2), STACK_BAR_HEIGHT, InfoBarTypes.NUM_INFOBAR_TYPES, WidgetTypes.WIDGET_HELP_SELECTED, 0, -1 )
		screen.setStackedBarColors("CityProductionBar", InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_GREAT_PEOPLE_STORED") )
		screen.setStackedBarColors("CityProductionBar", InfoBarTypes.INFOBAR_RATE, gc.getInfoTypeForString("COLOR_GREAT_PEOPLE_RATE") )
		screen.setStackedBarColors("CityProductionBar", InfoBarTypes.INFOBAR_RATE_EXTRA, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.setStackedBarColors("CityProductionBar", InfoBarTypes.INFOBAR_EMPTY, gc.getInfoTypeForString("COLOR_EMPTY") )
		self.appendtoHideState(screen, "CityProductionBar", HIDE_TYPE_CITY, HIDE_LEVEL_HIDE)

		screen.addPanel("CityProductionBarFrame", u"", u"", True, False, CITIZEN_BAR_WIDTH + (SMALL_BUTTON_SIZE), yResolution - BOTTOM_CENTER_HUD_HEIGHT - TRANSPORT_AREA_HEIGHT - STACK_BAR_HEIGHT, xResolution - CITIZEN_BAR_WIDTH - (MAP_EDGE_MARGIN_WIDTH * 3) - (SMALL_BUTTON_SIZE / 2), STACK_BAR_HEIGHT, PanelStyles.PANEL_STYLE_STANDARD, WidgetTypes.WIDGET_HELP_SELECTED, 0, -1 )
		screen.addDrawControl("CityProductionBarFrame", ArtFileMgr.getInterfaceArtInfo("INTERFACE_BUILD_PROGRESS_BAR").getPath(), CITIZEN_BAR_WIDTH + (SMALL_BUTTON_SIZE), yResolution - BOTTOM_CENTER_HUD_HEIGHT - TRANSPORT_AREA_HEIGHT - STACK_BAR_HEIGHT, xResolution - CITIZEN_BAR_WIDTH - (MAP_EDGE_MARGIN_WIDTH * 3) - (SMALL_BUTTON_SIZE / 2), STACK_BAR_HEIGHT, WidgetTypes.WIDGET_HELP_SELECTED, 0, -1 )
		self.appendtoHideState(screen, "CityProductionBarFrame", HIDE_TYPE_CITY, HIDE_LEVEL_HIDE)	

		screen.setLabelAt("ProductionText", "CityProductionBar", "", CvUtil.FONT_CENTER_JUSTIFY, (xResolution - CITIZEN_BAR_WIDTH - BUILD_AREA_WIDTH - (MAP_EDGE_MARGIN_WIDTH * 2)) / 2, 0, -1.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_HELP_SELECTED, 0, -1 )

	# CITY REBEL BAR
		screen.addStackedBarGFC("RebelBar", STACK_BAR_HEIGHT, yResolution - BOTTOM_CENTER_HUD_HEIGHT - STACK_BAR_HEIGHT, CITIZEN_BAR_WIDTH - (STACK_BAR_HEIGHT * 2), STACK_BAR_HEIGHT, InfoBarTypes.NUM_INFOBAR_TYPES, WidgetTypes.WIDGET_HELP_REBEL, 1, -1 )
		screen.setStackedBarColors("RebelBar", InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_BLUE") )
		screen.setStackedBarColors("RebelBar", InfoBarTypes.INFOBAR_RATE, gc.getInfoTypeForString("COLOR_BLUE") )
		screen.setStackedBarColors("RebelBar", InfoBarTypes.INFOBAR_RATE_EXTRA, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.setStackedBarColors("RebelBar", InfoBarTypes.INFOBAR_EMPTY, gc.getInfoTypeForString("COLOR_EMPTY") )
		self.appendtoHideState(screen, "RebelBar", HIDE_TYPE_CITY, HIDE_LEVEL_HIDE)

		screen.addPanel("RebelBarFrame", u"", u"", True, False, STACK_BAR_HEIGHT, yResolution - BOTTOM_CENTER_HUD_HEIGHT - STACK_BAR_HEIGHT, CITIZEN_BAR_WIDTH - (STACK_BAR_HEIGHT * 2), STACK_BAR_HEIGHT, PanelStyles.PANEL_STYLE_STANDARD, WidgetTypes.WIDGET_HELP_REBEL, 1, -1 )
		screen.addDrawControl("RebelBarFrame", ArtFileMgr.getInterfaceArtInfo("INTERFACE_REBEL_PROGRESS_BAR").getPath(), STACK_BAR_HEIGHT, yResolution - BOTTOM_CENTER_HUD_HEIGHT - STACK_BAR_HEIGHT, CITIZEN_BAR_WIDTH - (STACK_BAR_HEIGHT * 2), STACK_BAR_HEIGHT, WidgetTypes.WIDGET_HELP_REBEL, 1, -1 )
		self.appendtoHideState(screen, "RebelBarFrame", HIDE_TYPE_CITY, HIDE_LEVEL_HIDE)	

		screen.setLabelAt("RebelText", "RebelBar", "", CvUtil.FONT_CENTER_JUSTIFY, (CITIZEN_BAR_WIDTH - (STACK_BAR_HEIGHT * 3 / 2)) / 2, 0, -1.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
	
	# CITY BUILDING GRID	
		for iSpecial in range(gc.getNumSpecialBuildingInfos()):
			iTargetProfession = -1
			for iProfession in range(gc.getNumProfessionInfos()):
				if gc.getCivilizationInfo(gc.getActivePlayer().getCivilizationType()).isValidProfession(iProfession):
					if(gc.getProfessionInfo(iProfession).getSpecialBuilding() == iSpecial):
						iTargetProfession = iProfession
						break

			#screen.addDDSGFC("BuildingBox" + str(iSpecial), ArtFileMgr.getInterfaceArtInfo("INTERFACE_WORKER_BOX").getPath(), BUILDING_GRID[iSpecial][0] + STACK_BAR_HEIGHT, BUILDING_GRID[iSpecial][1] + CITY_TITLE_BAR_HEIGHT, BUILDING_GRID[iSpecial][3], BUILDING_GRID[iSpecial][3], WidgetTypes.WIDGET_GENERAL, -1, -1 )
			screen.setImageButton("CityBuildingGraphic" + str(iSpecial), "", BUILDING_GRID[iSpecial][0] + STACK_BAR_HEIGHT, BUILDING_GRID[iSpecial][1] + CITY_TITLE_BAR_HEIGHT, BUILDING_GRID[iSpecial][3], BUILDING_GRID[iSpecial][3], WidgetTypes.WIDGET_CITY_UNIT_ASSIGN_PROFESSION, iSpecial, iTargetProfession)
			screen.addDDSGFC("ProductionBox" + str(iSpecial), ArtFileMgr.getInterfaceArtInfo("INTERFACE_PRODUCTION_BOX").getPath(), BUILDING_GRID[iSpecial][0] + STACK_BAR_HEIGHT, BUILDING_GRID[iSpecial][1] + BUILDING_GRID[iSpecial][2], BUILDING_GRID[iSpecial][3] + STACK_BAR_HEIGHT, STACK_BAR_HEIGHT, WidgetTypes.WIDGET_GENERAL, -1, -1)
			screen.hide("ProductionBox" + str(iSpecial))

	# BUTTONS
		screen.setImageButton("HurryGold", ArtFileMgr.getInterfaceArtInfo("INTERFACE_EUROPE_PURCHASE_UNIT").getPath(), CITIZEN_BAR_WIDTH - (SMALL_BUTTON_SIZE), yResolution - BOTTOM_CENTER_HUD_HEIGHT - TRANSPORT_AREA_HEIGHT - (STACK_BAR_HEIGHT / 2) - SMALL_BUTTON_SIZE, SMALL_BUTTON_SIZE * 2, SMALL_BUTTON_SIZE * 2, WidgetTypes.WIDGET_HURRY, gc.getInfoTypeForString("HURRY_GOLD"), -1)
		self.appendtoHideState(screen, "HurryGold", HIDE_TYPE_CITY, HIDE_LEVEL_HIDE)

	# CITY AND PLOT SCROLL BUTTONS
		ScrollButtonSize = MEDIUM_BUTTON_SIZE
		screen.setImageButton("CityScrollMinus",ArtFileMgr.getInterfaceArtInfo("INTERFACE_CITY_LEFT_ARROW").getPath(), (xResolution * 35 / 100) - (ScrollButtonSize / 2), (STACK_BAR_HEIGHT / 2) - (ScrollButtonSize / 3), ScrollButtonSize, ScrollButtonSize, WidgetTypes.WIDGET_CITY_SCROLL, -1, -1)
		self.appendtoHideState(screen, "CityScrollMinus", HIDE_TYPE_CITY, HIDE_LEVEL_HIDE)
		screen.setImageButton("CityScrollPlus",ArtFileMgr.getInterfaceArtInfo("INTERFACE_CITY_RIGHT_ARROW").getPath(), (xResolution * 65 / 100) - (ScrollButtonSize / 2), (STACK_BAR_HEIGHT / 2) - (ScrollButtonSize / 3), ScrollButtonSize, ScrollButtonSize, WidgetTypes.WIDGET_CITY_SCROLL, 1, -1)
		self.appendtoHideState(screen, "CityScrollPlus", HIDE_TYPE_CITY, HIDE_LEVEL_HIDE)

		screen.setButtonGFC("PlotListMinus", u"", "", xResolution - RIGHT_PLOT_LIST_EDGE + int(MEDIUM_BUTTON_SIZE * -0.75), yResolution - 2*LARGE_BUTTON_SIZE, MEDIUM_BUTTON_SIZE, MEDIUM_BUTTON_SIZE, WidgetTypes.WIDGET_PLOT_LIST_SHIFT, -1, -1, ButtonStyles.BUTTON_STYLE_ARROW_LEFT )
		screen.hide("PlotListMinus")
		screen.setButtonGFC("PlotListPlus", u"", "", xResolution - RIGHT_PLOT_LIST_EDGE + int(MEDIUM_BUTTON_SIZE * 0.75), yResolution - 2*LARGE_BUTTON_SIZE, MEDIUM_BUTTON_SIZE, MEDIUM_BUTTON_SIZE, WidgetTypes.WIDGET_PLOT_LIST_SHIFT, 1, -1, ButtonStyles.BUTTON_STYLE_ARROW_RIGHT )
		screen.hide("PlotListPlus")
		screen.setButtonGFC("PlotListUp", u"", "", xResolution - RIGHT_PLOT_LIST_EDGE + 2, yResolution - 2*LARGE_BUTTON_SIZE - 10, MEDIUM_BUTTON_SIZE, MEDIUM_BUTTON_SIZE, WidgetTypes.WIDGET_PLOT_LIST_SHIFT, 0, -1, ButtonStyles.BUTTON_STYLE_ARROW_UP )
		screen.hide("PlotListUp")
		screen.setButtonGFC("PlotListDown", u"", "", xResolution - RIGHT_PLOT_LIST_EDGE + 2, yResolution - 2*LARGE_BUTTON_SIZE - 10, MEDIUM_BUTTON_SIZE, MEDIUM_BUTTON_SIZE, WidgetTypes.WIDGET_PLOT_LIST_SHIFT, 0, -1, ButtonStyles.BUTTON_STYLE_ARROW_DOWN )
		screen.hide("PlotListDown")

	# EXIT BUTTON
		screen.setText("CityExitText", "", u"<font=4>" + localText.getText("TXT_KEY_PEDIA_SCREEN_EXIT", ()).upper() + u"</font>", CvUtil.FONT_RIGHT_JUSTIFY, xResolution - 10, CITY_TITLE_BAR_HEIGHT / 12, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_CLOSE_SCREEN, -1, -1 )
		self.appendtoHideState(screen, "CityExitText", HIDE_TYPE_CITY, HIDE_LEVEL_HIDE)

	# GROWTH EMPHASIZE/DEMPHASIZE
		screen.setImageButton("AvoidGrowth", ArtFileMgr.getInterfaceArtInfo("INTERFACE_BUTTONS_CANCEL").getPath(), xResolution * 1 / 100, CITY_TITLE_BAR_HEIGHT / 10, ScrollButtonSize, ScrollButtonSize, WidgetTypes.WIDGET_EMPHASIZE, -1, AVOID_GROWTH)
		self.appendtoHideState(screen, "AvoidGrowth", HIDE_TYPE_CITY, HIDE_LEVEL_HIDE)

	# SCORES TABLE
		screen.addPanel("ScoreBackground", u"", u"", True, False, 0, 0, 0, 0, PanelStyles.PANEL_STYLE_HUD_HELP, WidgetTypes.WIDGET_GENERAL, -1, -1 )

		for i in range(gc.getMAX_PLAYERS()):
			szName = "ScoreText" + str(i)
			screen.setText( szName, "Background", u"", CvUtil.FONT_RIGHT_JUSTIFY, 996, 622, -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_CONTACT_CIV, i, -1 )
			screen.hide( szName )

		# This should be a forced redraw screen
		screen.setForcedRedraw(True)
		self.SetHideLists(screen)

		# This should show the screen immidiately and pass input to the game
		screen.showScreen(PopupStates.POPUPSTATE_IMMEDIATE, True)

	# Will update the screen (every 250 MS)
	def updateScreen( self ):

		global g_szTimeText
		global g_iTimeTextCounter

		screen = CyGInterfaceScreen("MainInterface", CvScreenEnums.MAIN_INTERFACE )

		# Find out our resolution
		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()

		messageControl = CyMessageControl()

		# Hide all interface widgets
		bShow = False
		if ( CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_HIDE_ALL and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_MINIMAP_ONLY ):
			if (gc.getGame().isPaused()):
				# Pause overrides other messages
				acOutput = localText.getText("SYSTEM_GAME_PAUSED", (gc.getPlayer(gc.getGame().getPausePlayer()).getNameKey(), ))
				screen.setEndTurnState("EndTurnText", acOutput )
				bShow = True
			elif (messageControl.GetFirstBadConnection() != -1):
				# Waiting on a bad connection to resolve
				if (messageControl.GetConnState(messageControl.GetFirstBadConnection()) == 1):
					if (gc.getGame().isMPOption(MultiplayerOptionTypes.MPOPTION_ANONYMOUS)):
						acOutput = localText.getText("SYSTEM_WAITING_FOR_PLAYER", (gc.getPlayer(messageControl.GetFirstBadConnection()).getNameKey(), 0))
					else:
						acOutput = localText.getText("SYSTEM_WAITING_FOR_PLAYER", (gc.getPlayer(messageControl.GetFirstBadConnection()).getNameKey(), (messageControl.GetFirstBadConnection() + 1)))
					screen.setEndTurnState("EndTurnText", acOutput )
					bShow = True
				elif (messageControl.GetConnState(messageControl.GetFirstBadConnection()) == 2):
					if (gc.getGame().isMPOption(MultiplayerOptionTypes.MPOPTION_ANONYMOUS)):
						acOutput = localText.getText("SYSTEM_PLAYER_JOINING", (gc.getPlayer(messageControl.GetFirstBadConnection()).getNameKey(), 0))
					else:
						acOutput = localText.getText("SYSTEM_PLAYER_JOINING", (gc.getPlayer(messageControl.GetFirstBadConnection()).getNameKey(), (messageControl.GetFirstBadConnection() + 1)))
					screen.setEndTurnState("EndTurnText", acOutput )
					bShow = True
			else:
				# Flash select messages if no popups are present
				if ( CyInterface().shouldDisplayWaitingOthers() ):
					acOutput = localText.getText("SYSTEM_WAITING", ())
					screen.setEndTurnState("EndTurnText", acOutput )
					bShow = True
				elif ( CyInterface().shouldDisplayEndTurn() ):
					acOutput = localText.getText("SYSTEM_END_TURN", ())
					screen.setEndTurnState("EndTurnText", acOutput )
					bShow = True
				elif ( CyInterface().shouldDisplayWaitingYou() ):
					acOutput = localText.getText("SYSTEM_WAITING_FOR_YOU", ())
					screen.setEndTurnState("EndTurnText", acOutput )
					bShow = True

		if ( bShow ):
			screen.showEndTurn("EndTurnText")
			if ( CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_SHOW or CyInterface().isCityScreenUp() ):
				screen.moveItem("EndTurnText", 0, yResolution - 194, -0.1 )
			else:
				screen.moveItem("EndTurnText", 0, yResolution - 86, -0.1 )
		else:
			screen.hideEndTurn("EndTurnText")

		self.updateEndTurnButton()

		if (CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_HIDE_ALL and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_ADVANCED_START):
			self.updateTimeText()
			if (CyInterface().isCityScreenUp()):
				screen.hide("TimeText")
			else:
				screen.setLabel("TimeText", "Background", self.setFontSize(g_szTimeText, 0), CvUtil.FONT_CENTER_JUSTIFY, xResolution / 2, TOP_CENTER_HUD_HEIGHT / 3, -0.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
				screen.show("TimeText")
		else:
			screen.hide("TimeText")

		return 0

	# Will redraw the interface
	def redraw( self ):

		screen = CyGInterfaceScreen("MainInterface", CvScreenEnums.MAIN_INTERFACE )

		if not INITIALIZED:
			self.SetGlobals(screen)

		# Check Dirty Bits, see what we need to redraw...
		if ( CyInterface().isDirty(InterfaceDirtyBits.MiscButtons_DIRTY_BIT) ):
			# Miscellaneous buttons (civics screen, etc)
			self.updateMiscButtons()
			CyInterface().setDirty(InterfaceDirtyBits.MiscButtons_DIRTY_BIT, False)
		if ( CyInterface().isDirty(InterfaceDirtyBits.InfoPane_DIRTY_BIT) ):
			# Info Pane Dirty Bit
			# This must come before updatePlotListButtons so that the entity widget appears in front of the stats
			self.updateInfoPaneStrings()
			CyInterface().setDirty(InterfaceDirtyBits.InfoPane_DIRTY_BIT, False)
		if ( CyInterface().isDirty(InterfaceDirtyBits.PlotListButtons_DIRTY_BIT) ):
			# Plot List Buttons Dirty
			self.updatePlotListButtons()
			CyInterface().setDirty(InterfaceDirtyBits.PlotListButtons_DIRTY_BIT, False)
		if ( CyInterface().isDirty(InterfaceDirtyBits.SelectionButtons_DIRTY_BIT) ):
			# Selection Buttons Dirty
			self.updateSelectionButtons()
			CyInterface().setDirty(InterfaceDirtyBits.SelectionButtons_DIRTY_BIT, False)
		if ( CyInterface().isDirty(InterfaceDirtyBits.GameData_DIRTY_BIT) ):
			# Game Data Strings Dirty
			self.updateGameDataStrings()
			CyInterface().setDirty(InterfaceDirtyBits.GameData_DIRTY_BIT, False)
		if ( CyInterface().isDirty(InterfaceDirtyBits.Help_DIRTY_BIT) ):
			# Help Dirty bit
			self.updateHelpStrings()
			CyInterface().setDirty(InterfaceDirtyBits.Help_DIRTY_BIT, False)
		if ( CyInterface().isDirty(InterfaceDirtyBits.CityScreen_DIRTY_BIT) ):
			# Selection Data Dirty Bit
			self.updateCityScreen()
			#CyInterface().setDirty(InterfaceDirtyBits.Domestic_Advisor_DIRTY_BIT, True)
			CyInterface().setDirty(InterfaceDirtyBits.CityScreen_DIRTY_BIT, False)
		if ( CyInterface().isDirty(InterfaceDirtyBits.ResourceTable_DIRTY_BIT) ):
			# Resource Table
			self.updateResourceTable()
			CyInterface().setDirty(InterfaceDirtyBits.ResourceTable_DIRTY_BIT, False)
		if ( CyInterface().isDirty(InterfaceDirtyBits.CitizenButtons_DIRTY_BIT) ):
			# Citizen Buttons Dirty
			self.updateCitizenButtons()
			CyInterface().setDirty(InterfaceDirtyBits.CitizenButtons_DIRTY_BIT, False)
		if ( CyInterface().isDirty(InterfaceDirtyBits.Score_DIRTY_BIT) or CyInterface().checkFlashUpdate() ):
			# Scores!
			self.updateScoreStrings()
			CyInterface().setDirty(InterfaceDirtyBits.Score_DIRTY_BIT, False)
		if ( CyInterface().isDirty(InterfaceDirtyBits.GlobeInfo_DIRTY_BIT) ):
			# Globeview and Globelayer buttons
			CyInterface().setDirty(InterfaceDirtyBits.GlobeInfo_DIRTY_BIT, False)
			self.updateGlobeviewButtons()

		return 0

	# Will update the end Turn Button
	def updateEndTurnButton( self ):

		global g_eEndTurnButtonState
		screen = CyGInterfaceScreen("MainInterface", CvScreenEnums.MAIN_INTERFACE )

		if (CyInterface().shouldDisplayEndTurnButton() and CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_SHOW ):
			eState = CyInterface().getEndTurnState()
			bShow = False

			if (eState == EndTurnButtonStates.END_TURN_OVER_HIGHLIGHT):
				screen.setEndTurnState("EndTurnButton", u"Red")
				bShow = True
			elif (eState == EndTurnButtonStates.END_TURN_OVER_DARK):
				screen.setEndTurnState("EndTurnButton", u"Red")
				bShow = True
			elif (eState == EndTurnButtonStates.END_TURN_GO ):
				screen.setEndTurnState("EndTurnButton", u"Green")
				bShow = True

			if (bShow):
				screen.showEndTurn("EndTurnButton")
			else:
				screen.hideEndTurn("EndTurnButton")

			if (g_eEndTurnButtonState == eState):
				return

			g_eEndTurnButtonState = eState

		else:
			screen.hideEndTurn("EndTurnButton")

		return 0

	# Update the miscellaneous buttons
	def updateMiscButtons( self ):
		screen = CyGInterfaceScreen("MainInterface", CvScreenEnums.MAIN_INTERFACE )

		if (CyInterface().shouldDisplayFlag() and CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_SHOW):
			screen.show("CivilizationFlag")
		else:
			screen.hide("CivilizationFlag")

		if (CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_SHOW):
			iLevel = 3
		if (CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_HIDE):
			iLevel = 2
		if (CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_HIDE_ALL):
			iLevel = 1
		elif (CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_ADVANCED_START):
			iLevel = 0
		elif (CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_MINIMAP_ONLY):
			iLevel = 1

		if (CyInterface().isCityScreenUp()):
			self.SetHideState(screen, HIDE_TYPE_CITY, iLevel)
		elif (CyEngine().isGlobeviewUp()):
			self.SetHideState(screen, HIDE_TYPE_GLOBAL, iLevel)
		else:
			self.SetHideState(screen, HIDE_TYPE_MAP, iLevel)

		screen.updateMinimapVisibility()

		return 0

	# Update plot List Buttons
	def updatePlotListButtons( self ):

		screen = CyGInterfaceScreen("MainInterface", CvScreenEnums.MAIN_INTERFACE )
		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()
		pHeadSelectedCity = CyInterface().getHeadSelectedCity()

		bHandled = False
		if (CyInterface().shouldDisplayUnitModel() and not CyEngine().isGlobeviewUp() and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_HIDE_ALL and not pHeadSelectedCity):
			if (CyInterface().isCityScreenUp()):
				if ( not bHandled ):
					screen.hide("InterfaceUnitModel")
					bHandled = True

				screen.moveToFront("CityBuildQueue")
				screen.moveToFront("SelectedUnitText")

			elif (CyInterface().getHeadSelectedUnit()):
				if (CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_HIDE):
					screen.addUnitGraphicGFC("InterfaceUnitModel", CyInterface().getHeadSelectedUnit().getUnitType(), CyInterface().getHeadSelectedUnit().getProfession(), xResolution - int(MAIN_LARGE_PORTRAIT_WIDTH * 3 / 2), yResolution - MAIN_LARGE_PORTRAIT_HEIGHT - LOWER_RIGHT_CORNER_BACKGROUND_HEIGHT - STACK_BAR_HEIGHT , MAIN_LARGE_PORTRAIT_WIDTH * 3 / 2, MAIN_LARGE_PORTRAIT_HEIGHT, WidgetTypes.WIDGET_UNIT_MODEL, CyInterface().getHeadSelectedUnit().getUnitType(), -1, -100, 0, 1, False )
				else:
					screen.addUnitGraphicGFC("InterfaceUnitModel", CyInterface().getHeadSelectedUnit().getUnitType(), CyInterface().getHeadSelectedUnit().getProfession(), xResolution - int(MAIN_LARGE_PORTRAIT_WIDTH * 3 / 2), yResolution - MAIN_LARGE_PORTRAIT_HEIGHT, MAIN_LARGE_PORTRAIT_WIDTH * 3 / 2, MAIN_LARGE_PORTRAIT_HEIGHT, WidgetTypes.WIDGET_UNIT_MODEL, CyInterface().getHeadSelectedUnit().getUnitType(), -1, -100, 0, 1, False )

				screen.moveToFront("SelectedUnitText")
				if (not pHeadSelectedCity):
					for iYield in EMPHASIZEYIELDS:
						screen.hide("MapYieldIcon" + str(iYield))
						screen.hide("MapYieldEmphasize" + str(iYield))
						screen.hide("MapYieldDe-Emphasize" + str(iYield))
						
					screen.hide("MapBuildingSelectionMultiList")
					screen.hide("MapCityProductionBar")
					screen.hide("MapCityBuildQueue")

			else:
				screen.hide("InterfaceUnitModel")
				if (not pHeadSelectedCity):
					for iYield in EMPHASIZEYIELDS:
						screen.hide("MapYieldIcon" + str(iYield))
						screen.hide("MapYieldEmphasize" + str(iYield))
						screen.hide("MapYieldDe-Emphasize" + str(iYield))
						
					screen.hide("MapBuildingSelectionMultiList")
					screen.hide("MapCityProductionBar")
					screen.hide("MapCityBuildQueue")
		else:
			screen.hide("InterfaceUnitModel")
			if (not pHeadSelectedCity):
				for iYield in EMPHASIZEYIELDS:
					screen.hide("MapYieldIcon" + str(iYield))
					screen.hide("MapYieldEmphasize" + str(iYield))
					screen.hide("MapYieldDe-Emphasize" + str(iYield))
					
				screen.hide("MapBuildingSelectionMultiList")
				screen.hide("MapCityProductionBar")
				screen.hide("MapCityBuildQueue")

		pPlot = CyInterface().getSelectionPlot()

		screen.hide("PlotListMinus")
		screen.hide("PlotListPlus")
		screen.hide("PlotListUp")
		screen.hide("PlotListDown")
		screen.hide("MapBuildingSelectionMultiList")
		screen.hide("MapCityProductionBar")
		screen.hide("MapCityBuildQueue")

		for j in range(gc.getMAX_PLOT_LIST_ROWS()):
			for i in range(self.numPlotListButtons()):
				szString = "PlotListButtonMulti" + str(j * self.numPlotListButtons() + i)
				screen.hide( szString )
				screen.hide( szString + "Health")
				screen.hide( szString + "Icon")
				
		for i in range(self.numPlotListButtons()):
			szString = "PlotListButton" + str(i)
			screen.hide( szString )
			screen.hide( szString + "Health")
			screen.hide( szString + "Icon")

		if ( pPlot and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_HIDE_ALL and not CyEngine().isGlobeviewUp() and not CyInterface().isCityScreenUp() and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_MINIMAP_ONLY and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_ADVANCED_START):

			iVisibleUnits = CyInterface().getNumVisibleUnits()
			iCount = -(CyInterface().getPlotListColumn())
			bLeftArrow = False
			bRightArrow = False

			if CyInterface().isPlotListMultiRow():
				iMaxRows = gc.getMAX_PLOT_LIST_ROWS()
				szPlotListButtonName = "PlotListButtonMulti"
			else:
				iMaxRows = 1
				szPlotListButtonName = "PlotListButton"
				
			iCount += CyInterface().getPlotListOffset()
			iSkipped = 0

			CyInterface().cacheInterfacePlotUnits(pPlot)
			for i in range(CyInterface().getNumCachedInterfacePlotUnits()):
				pLoopUnit = CyInterface().getCachedInterfacePlotUnit(i)
				if (pLoopUnit):
					if ((iCount == 0) and (CyInterface().getPlotListColumn() > 0)):
						bLeftArrow = True
					elif ((iCount == (iMaxRows * self.numPlotListButtons() - 1)) and ((iVisibleUnits - iCount - CyInterface().getPlotListColumn() + iSkipped) > 1)):
						bRightArrow = True

					if ((iCount >= 0) and (iCount <  self.numPlotListButtons() * iMaxRows)):
						if ((pLoopUnit.getTeam() != gc.getGame().getActiveTeam()) or pLoopUnit.isWaiting()):
							szFileName = ArtFileMgr.getInterfaceArtInfo("OVERLAY_FORTIFY").getPath()

						elif (pLoopUnit.canMove()):
							if (pLoopUnit.hasMoved()):
								szFileName = ArtFileMgr.getInterfaceArtInfo("OVERLAY_HASMOVED").getPath()
							else:
								szFileName = ArtFileMgr.getInterfaceArtInfo("OVERLAY_MOVE").getPath()
						else:
							szFileName = ArtFileMgr.getInterfaceArtInfo("OVERLAY_NOMOVE").getPath()

						szString = szPlotListButtonName + str(iCount)
						screen.changeImageButton( szString, pLoopUnit.getButton() )
						if ( pLoopUnit.getOwner() == gc.getGame().getActivePlayer() ):
							screen.enable(szString, True)
						else:
							screen.enable(szString, False)

						screen.setState(szString, pLoopUnit.IsSelected())
						screen.show( szString )

						# place the health bar
						if (pLoopUnit.isFighting()):
							bShowHealth = False
						else:
							bShowHealth = pLoopUnit.canFight()

						if bShowHealth:
							szStringHealth = szString + "Health"
							screen.setBarPercentage( szStringHealth, InfoBarTypes.INFOBAR_STORED, float( pLoopUnit.currHitPoints() ) / float( pLoopUnit.maxHitPoints() ) )
							if (pLoopUnit.getDamage() >= ((pLoopUnit.maxHitPoints() * 2) / 3)):
								screen.setStackedBarColors(szStringHealth, InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_RED"))
							elif (pLoopUnit.getDamage() >= (pLoopUnit.maxHitPoints() / 3)):
								screen.setStackedBarColors(szStringHealth, InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_YELLOW"))
							else:
								screen.setStackedBarColors(szStringHealth, InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_GREEN"))
							screen.show( szStringHealth )

						# Adds the overlay first
						szStringIcon = szString + "Icon"
						screen.changeDDSGFC( szStringIcon, szFileName )
						screen.show( szStringIcon )

					iCount += 1

			if iVisibleUnits > 0:				
				if CyInterface().isPlotListMultiRow():
					screen.show("PlotListDown")
					screen.enable("PlotListDown", true)
				else:
					screen.show("PlotListUp")
					screen.enable("PlotListUp", true)

			if (iVisibleUnits > self.numPlotListButtons() * iMaxRows):
				screen.enable("PlotListMinus", bLeftArrow)
				screen.show("PlotListMinus")
				screen.enable("PlotListPlus", bRightArrow)
				screen.show("PlotListPlus")
				
		if (not CyInterface().isCityScreenUp() and not CyEngine().isGlobeviewUp()):
			if (pHeadSelectedCity):
			
				for i in range(self.numPlotListButtons()):
					szString = "PlotListButton" + str(i)
					screen.hide( szString )
					screen.hide( szString + "Health")
					screen.hide( szString + "Icon")

				for i in EMPHASIZEYIELDS:
					if pHeadSelectedCity.AI_getEmphasizeYieldCount(i) > 0:
						screen.show("MapYieldIcon" + str(i))
						screen.show("MapYieldEmphasize" + str(i))
						screen.hide("MapYieldDe-Emphasize" + str(i))
					elif pHeadSelectedCity.AI_getEmphasizeYieldCount(i) < 0:
						screen.show("MapYieldIcon" + str(i))
						screen.hide("MapYieldEmphasize" + str(i))
						screen.show("MapYieldDe-Emphasize" + str(i))
					else:
						screen.show("MapYieldIcon" + str(i))
						screen.hide("MapYieldEmphasize" + str(i))
						screen.hide("MapYieldDe-Emphasize" + str(i))

				#if (CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_HIDE or InterfaceVisibility.INTERFACE_HIDE_ALL):
				#	screen.addMultiListControlGFC("MapBuildingSelectionMultiList", u"", xResolution * 7 / 20, yResolution - (self.BOTTOM_BUTTON_SIZE * 5), xResolution * 9 / 20, self.BOTTOM_BUTTON_SIZE * 3, self.BOTTOM_BUTTON_ROWS, self.BOTTOM_BUTTON_SIZE, self.BOTTOM_BUTTON_SIZE, TableStyles.TABLE_STYLE_STANDARD )		
				#else:
				#	screen.addMultiListControlGFC("MapBuildingSelectionMultiList", u"", xResolution * 7 / 20, yResolution - SADDLE_HEIGHT + (self.BOTTOM_BUTTON_SIZE / 2), xResolution * 9 / 20, self.BOTTOM_BUTTON_SIZE * 3, self.BOTTOM_BUTTON_ROWS, self.BOTTOM_BUTTON_SIZE, self.BOTTOM_BUTTON_SIZE, TableStyles.TABLE_STYLE_STANDARD )		

				screen.clearMultiList("MapBuildingSelectionMultiList")
					
				iCount = 0
				iRow = 0
				bFound = False
				for i in range (gc.getNumUnitClassInfos()):
					eLoopUnit = gc.getCivilizationInfo(pHeadSelectedCity.getCivilizationType()).getCivilizationUnits(i)
					if (eLoopUnit != UnitTypes.NO_UNIT):
						if (pHeadSelectedCity.canTrain(eLoopUnit, False, False)):
							szButton = gc.getPlayer(pHeadSelectedCity.getOwner()).getUnitButton(eLoopUnit)
							screen.appendMultiListButton("MapBuildingSelectionMultiList", szButton, iRow, WidgetTypes.WIDGET_TRAIN, i, -1, False)
							iCount += 1
							bFound = True

				bFound = False
				for i in range (gc.getNumBuildingClassInfos()):
					eLoopBuilding = gc.getCivilizationInfo(pHeadSelectedCity.getCivilizationType()).getCivilizationBuildings(i)
					if (eLoopBuilding != BuildingTypes.NO_BUILDING):
						if (pHeadSelectedCity.canConstruct(eLoopBuilding, False, False, False)):
							screen.appendMultiListButton("MapBuildingSelectionMultiList", gc.getBuildingInfo(eLoopBuilding).getButton(), iRow, WidgetTypes.WIDGET_CONSTRUCT, i, -1, False )
							iCount += 1
							bFound = True

				bFound = False
				for i in range (gc.getNumFatherPointInfos()):
					if (pHeadSelectedCity.canConvince(i, False, True)):
						screen.appendMultiListButton("MapBuildingSelectionMultiList", gc.getFatherPointInfo(i).getButton(), iRow, WidgetTypes.WIDGET_CONVINCE, i, -1, False )
						if (not pHeadSelectedCity.canConvince(i, False, False)):
							screen.disableMultiListButton("MapBuildingSelectionMultiList", iRow, iCount, gc.getFatherPointInfo(i).getButton())
						iCount += 1
						bFound = True

				screen.show("MapBuildingSelectionMultiList")
				
			# MAP BUILD QUEUE
				QueueWidth = LOWER_RIGHT_CORNER_BACKGROUND_WIDTH								
				screen.addTableControlGFC("MapCityBuildQueue", 2, xResolution - LOWER_RIGHT_CORNER_BACKGROUND_WIDTH, yResolution - LOWER_RIGHT_CORNER_BACKGROUND_HEIGHT, LOWER_RIGHT_CORNER_BACKGROUND_WIDTH, LOWER_RIGHT_CORNER_BACKGROUND_HEIGHT - (LOWER_RIGHT_CORNER_BACKGROUND_HEIGHT / 10), False, False, self.SELECTION_PANEL_ROW_HEIGHT, self.SELECTION_PANEL_ROW_HEIGHT, TableStyles.TABLE_STYLE_STANDARD)
				screen.setStyle("MapCityBuildQueue", "Table_EmptyScroll_Style")
				screen.hide("MapCityBuildQueue")
				screen.setTableColumnHeader("MapCityBuildQueue", 0, u"", QueueWidth - (QueueWidth * 3 / 10))
				screen.setTableColumnHeader("MapCityBuildQueue", 1, u"", QueueWidth * 3 / 10)
				
				iOrders = CyInterface().getNumOrdersQueued()
				iRow = 0
				
				for i in range(iOrders):					
					szLeftBuffer = u""
					szRightBuffer = u""

					if ( CyInterface().getOrderNodeType(i) == OrderTypes.ORDER_TRAIN ):
						szLeftBuffer = gc.getUnitInfo(CyInterface().getOrderNodeData1(i)).getDescription()
						szRightBuffer = str(pHeadSelectedCity.getUnitProductionTurnsLeft(CyInterface().getOrderNodeData1(i), i))

						if (CyInterface().getOrderNodeSave(i)):
							szLeftBuffer = u"*" + szLeftBuffer

					elif ( CyInterface().getOrderNodeType(i) == OrderTypes.ORDER_CONSTRUCT ):
						szLeftBuffer = gc.getBuildingInfo(CyInterface().getOrderNodeData1(i)).getDescription()
						szRightBuffer = str(pHeadSelectedCity.getBuildingProductionTurnsLeft(CyInterface().getOrderNodeData1(i), i))

					elif ( CyInterface().getOrderNodeType(i) == OrderTypes.ORDER_CONVINCE ):
						szLeftBuffer = gc.getFatherPointInfo(CyInterface().getOrderNodeData1(i)).getDescription()

					screen.appendTableRow("MapCityBuildQueue")
					screen.setTableText("MapCityBuildQueue", 0, iRow, szLeftBuffer, "", WidgetTypes.WIDGET_HELP_SELECTED, i, -1, CvUtil.FONT_LEFT_JUSTIFY )
					screen.setTableText("MapCityBuildQueue", 1, iRow, szRightBuffer + "  ", "", WidgetTypes.WIDGET_HELP_SELECTED, i, -1, CvUtil.FONT_RIGHT_JUSTIFY )

					iRow += 1
				screen.show("MapCityBuildQueue")	
				
				
			# CURRENT PRODUCTION BAR FILL
				fProductionNeeded = float(pHeadSelectedCity.getProductionNeeded(YieldTypes.YIELD_HAMMERS))
				iProductionDiffNoFood = pHeadSelectedCity.getCurrentProductionDifference(True)
				iProductionDiffJustFood = 0
				if (fProductionNeeded > 0):
					iFirst = ((float(pHeadSelectedCity.getProduction())) / fProductionNeeded)
					screen.setBarPercentage("MapCityProductionBar", InfoBarTypes.INFOBAR_STORED, iFirst )
					if ( iFirst == 1 ):
						iSecond = (((float(iProductionDiffNoFood)) / fProductionNeeded) )
					else:
						iSecond = (((float(iProductionDiffNoFood)) / fProductionNeeded) ) / ( 1 - iFirst )
					screen.setBarPercentage("MapCityProductionBar", InfoBarTypes.INFOBAR_RATE, iSecond )
					if ( iFirst + iSecond == 1 ):
						screen.setBarPercentage("MapCityProductionBar", InfoBarTypes.INFOBAR_RATE_EXTRA, (((float(iProductionDiffJustFood)) / fProductionNeeded) ) )
					else:
						screen.setBarPercentage("MapCityProductionBar", InfoBarTypes.INFOBAR_RATE_EXTRA, (( ((float(iProductionDiffJustFood)) / fProductionNeeded) ) ) / ( 1 - ( iFirst + iSecond ) ) )

			# CURRENT PRODUCTION BAR TEXT
				if (pHeadSelectedCity.isProductionBuilding() or pHeadSelectedCity.isProductionUnit()):
					szBuffer = localText.getText("INTERFACE_CITY_PRODUCTION", (pHeadSelectedCity.getProductionNameKey(), pHeadSelectedCity.getProductionTurnsLeft()))
				elif pHeadSelectedCity.isProduction():
					szBuffer = pHeadSelectedCity.getProductionName()
				else:
					szBuffer = u""
					
				RedText = false
				for iYield in range(YieldTypes.NUM_YIELD_TYPES):
					if iYield != YieldTypes.YIELD_HAMMERS:
						if (CyInterface().getOrderNodeType(0) == OrderTypes.ORDER_TRAIN ):
							YieldRequired = gc.getUnitInfo(CyInterface().getOrderNodeData1(0)).getYieldCost(iYield)					
						elif (CyInterface().getOrderNodeType(0) == OrderTypes.ORDER_CONSTRUCT):
							YieldRequired = gc.getBuildingInfo(CyInterface().getOrderNodeData1(0)).getYieldCost(iYield)
						else:
							YieldRequired = 0
							
						if (YieldRequired > 0):
							if (YieldRequired >  pHeadSelectedCity.getYieldStored(iYield) + pHeadSelectedCity.getYieldRushed(iYield)):
								RedText = true
								break
				if (RedText):
					screen.setStackedBarColors("MapCityProductionBar", InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_RED") )
					screen.setStackedBarColors("MapCityProductionBar", InfoBarTypes.INFOBAR_RATE, gc.getInfoTypeForString("COLOR_WARNING_RATE") )
				else:
					screen.setStackedBarColors("MapCityProductionBar", InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_GREAT_PEOPLE_STORED") )
					screen.setStackedBarColors("MapCityProductionBar", InfoBarTypes.INFOBAR_RATE, gc.getInfoTypeForString("COLOR_GREAT_PEOPLE_RATE") )
					
				screen.setLabelAt("ProductionText", "MapCityProductionBar", szBuffer, CvUtil.FONT_CENTER_JUSTIFY, UNIT_HEALTH_BAR_WIDTH / 2, STACK_BAR_HEIGHT / 2, -1.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
				screen.show("MapCityProductionBar")
				screen.hide("PlotListUp")

			# BUILDING PORTRAIT
			eOrderNodeType = CyInterface().getOrderNodeType(0)
			if (eOrderNodeType  == OrderTypes.ORDER_TRAIN):
				screen.addUnitGraphicGFC("InterfaceUnitModel", CyInterface().getOrderNodeData1(0), -1, xResolution - int(MAIN_LARGE_PORTRAIT_WIDTH * 2), yResolution - MAIN_LARGE_PORTRAIT_HEIGHT - LOWER_RIGHT_CORNER_BACKGROUND_HEIGHT - (STACK_BAR_HEIGHT * 3 / 2), MAIN_LARGE_PORTRAIT_WIDTH * 2, MAIN_LARGE_PORTRAIT_HEIGHT, WidgetTypes.WIDGET_GENERAL, -1, -1, -100, 0, 1, False )
			elif (eOrderNodeType == OrderTypes.ORDER_CONSTRUCT):
				screen.addBuildingGraphicGFC("InterfaceUnitModel", CyInterface().getOrderNodeData1(0), xResolution - int(MAIN_LARGE_PORTRAIT_WIDTH * 2), yResolution - MAIN_LARGE_PORTRAIT_HEIGHT - LOWER_RIGHT_CORNER_BACKGROUND_HEIGHT - (STACK_BAR_HEIGHT * 3 / 2), MAIN_LARGE_PORTRAIT_WIDTH * 2, MAIN_LARGE_PORTRAIT_HEIGHT, WidgetTypes.WIDGET_GENERAL, -1, -1, -20, 30, 0.9, False )
			
			self.updateScoreStrings()

			
		return 0

	# Will hide and show the selection buttons and their associated buttons
	def updateSelectionButtons( self ):

		global g_pSelectedUnit
		screen = CyGInterfaceScreen("MainInterface", CvScreenEnums.MAIN_INTERFACE )

		# Find out our resolution
		pHeadSelectedCity = CyInterface().getHeadSelectedCity()
		pHeadSelectedUnit = CyInterface().getHeadSelectedUnit()
		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()

		# All of the hides...
		screen.clearMultiList("CityBuildingSelectionMultiList")
		screen.hide("AutomateProduction")
		screen.hide("CityTransportPanel")
		screen.hide("CityGarrisonPanel")
		screen.hide("TradeRouteText")
		screen.hide("ImportTradeRouteText")
		screen.hide("ExportTradeRouteText")
		screen.hide("ImportButton")
		screen.hide("ExportButton")
		screen.hide("ImportLabel")
		screen.hide("ExportLabel")
		screen.hide("CityBuildQueue")
		screen.hide("EmphasizeTable")

		self.setMinimapButtonVisibility(False)
		screen.hideList(ACTION_BUTTON_HIDE)

		for iYield in range(YieldTypes.NUM_YIELD_TYPES):
			screen.hide("YieldCost" + str(iYield))

		if (not CyInterface().isCityScreenUp() and not CyEngine().isGlobeviewUp() and pHeadSelectedCity):
			for i in EMPHASIZEYIELDS:
				if pHeadSelectedCity.AI_getEmphasizeYieldCount(i) > 0:
					screen.show("MapYieldIcon" + str(i))
					screen.show("MapYieldEmphasize" + str(i))
					screen.hide("MapYieldDe-Emphasize" + str(i))
				elif pHeadSelectedCity.AI_getEmphasizeYieldCount(i) < 0:
					screen.show("MapYieldIcon" + str(i))
					screen.hide("MapYieldEmphasize" + str(i))
					screen.show("MapYieldDe-Emphasize" + str(i))
				else:
					screen.show("MapYieldIcon" + str(i))
					screen.hide("MapYieldEmphasize" + str(i))
					screen.hide("MapYieldDe-Emphasize" + str(i))


		if (not CyEngine().isGlobeviewUp() and CyInterface().isCityScreenUp() and pHeadSelectedCity):
			self.setMinimapButtonVisibility(True)

			self.updateGarrisonAndTransports()
			g_pSelectedUnit = 0
			screen.enable("HurryGold", pHeadSelectedCity.canHurry(0, False))

		# BUILDING SELECTION BUTTONS
			iCount = 0
			iRow = 0
			bFound = False
			for i in range (gc.getNumUnitClassInfos()):
				eLoopUnit = gc.getCivilizationInfo(pHeadSelectedCity.getCivilizationType()).getCivilizationUnits(i)
				if (eLoopUnit != UnitTypes.NO_UNIT):
					if (pHeadSelectedCity.canTrain(eLoopUnit, False, True)):
						szButton = gc.getPlayer(pHeadSelectedCity.getOwner()).getUnitButton(eLoopUnit)
						screen.appendMultiListButton("CityBuildingSelectionMultiList", szButton, iRow, WidgetTypes.WIDGET_TRAIN, i, -1, False)
						if ( not pHeadSelectedCity.canTrain(eLoopUnit, False, False) ):
							screen.disableMultiListButton("CityBuildingSelectionMultiList", iRow, iCount, szButton)
						iCount += 1
						bFound = True

			iCount = 0
			iRow += 1
			bFound = False
			for i in range (gc.getNumBuildingClassInfos()):
				eLoopBuilding = gc.getCivilizationInfo(pHeadSelectedCity.getCivilizationType()).getCivilizationBuildings(i)
				if (eLoopBuilding != BuildingTypes.NO_BUILDING):
					if (pHeadSelectedCity.canConstruct(eLoopBuilding, False, True, False)):
						screen.appendMultiListButton("CityBuildingSelectionMultiList", gc.getBuildingInfo(eLoopBuilding).getButton(), iRow, WidgetTypes.WIDGET_CONSTRUCT, i, -1, False )
						if (not pHeadSelectedCity.canConstruct(eLoopBuilding, False, False, False)):
							screen.disableMultiListButton("CityBuildingSelectionMultiList", iRow, iCount, gc.getBuildingInfo(eLoopBuilding).getButton())
						iCount += 1
						bFound = True

			iCount = 0
			iRow += 1
			bFound = False
			for i in range (gc.getNumFatherPointInfos()):
				if (pHeadSelectedCity.canConvince(i, False, True)):
					screen.appendMultiListButton("CityBuildingSelectionMultiList", gc.getFatherPointInfo(i).getButton(), iRow, WidgetTypes.WIDGET_CONVINCE, i, -1, False )
					if (not pHeadSelectedCity.canConvince(i, False, False)):
						screen.disableMultiListButton("CityBuildingSelectionMultiList", iRow, iCount, gc.getFatherPointInfo(i).getButton())
					iCount += 1
					bFound = True

		# GOVENOR MANAGMENT PANEL
			if AUTOMATION_MANAGMENT_PANEL_UP:
				screen.hide("InterfaceUnitModel")
				screen.hide("_FXS_Screen_Bogus_Minimap_Name")
				screen.hide("CityList")
				screen.hide("CityBuildingSelectionMultiList")
				
				screen.hide("MapHighlightButton")
				screen.hide("BuildHighlightButton")
				screen.show("GovernorHighlightButton")

			# EMPHASIZE TABLE
				iRow = 0
				screen.setTableText("EmphasizeTable", 0, iRow, self.setFontSize(localText.getText("TXT_KEY_EMPHASIZE", ()), 1), "", WidgetTypes.WIDGET_EMPHASIZE, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
				for iYield in EMPHASIZEYIELDS:
					iEmphasize = pHeadSelectedCity.AI_getEmphasizeYieldCount(iYield)
					if iEmphasize == 0:
						szText = localText.getText("TXT_KEY_EMPHASIS_NEUTRAL", (gc.getYieldInfo(iYield).getChar(), ))
					elif iEmphasize > 0:
						szText = localText.getText("TXT_KEY_EMPHASIZED", (gc.getYieldInfo(iYield).getChar(), ))
					else:
						szText = localText.getText("TXT_KEY_DEEMPHASIZED", (gc.getYieldInfo(iYield).getChar(), ))
					iRow += 1
					screen.setTableText("EmphasizeTable", 0, iRow, u"<font=3>%s</font>" % szText, "", WidgetTypes.WIDGET_EMPHASIZE, iYield, -1, CvUtil.FONT_LEFT_JUSTIFY )
				screen.show("EmphasizeTable")
				
				screen.setButtonGFC("ImportButton", self.setFontSize(localText.getText("TXT_KEY_DEMO_SCREEN_IMPORTS_TEXT", ()), 1), "", CITIZEN_BAR_WIDTH + (MAP_EDGE_MARGIN_WIDTH * 2), CITY_TITLE_BAR_HEIGHT + CITY_VIEW_BOX_HEIGHT_AND_WIDTH, BUILD_AREA_WIDTH - MEDIUM_BUTTON_SIZE, 3 * STACK_BAR_HEIGHT / 2, WidgetTypes.WIDGET_YIELD_IMPORT_EXPORT, false, -1, ButtonStyles.BUTTON_STYLE_STANDARD)
				screen.addTableControlGFC("ImportTradeRouteText", 1, CITIZEN_BAR_WIDTH + (MAP_EDGE_MARGIN_WIDTH * 2), CITY_TITLE_BAR_HEIGHT + CITY_VIEW_BOX_HEIGHT_AND_WIDTH + 3 * STACK_BAR_HEIGHT / 2, BUILD_AREA_WIDTH, TRANSPORT_AREA_HEIGHT - (STACK_BAR_HEIGHT * 3), False, False, MEDIUM_BUTTON_SIZE, MEDIUM_BUTTON_SIZE, TableStyles.TABLE_STYLE_STANDARD)
				screen.setStyle("ImportTradeRouteText", "Table_EmptyScroll_Style")

				screen.setButtonGFC("ExportButton", self.setFontSize(localText.getText("TXT_KEY_DEMO_SCREEN_EXPORTS_TEXT", ()), 1), "", CITIZEN_BAR_WIDTH + (MAP_EDGE_MARGIN_WIDTH * 2) + BUILD_AREA_WIDTH, CITY_TITLE_BAR_HEIGHT + CITY_VIEW_BOX_HEIGHT_AND_WIDTH, BUILD_AREA_WIDTH - MEDIUM_BUTTON_SIZE, 3 * STACK_BAR_HEIGHT / 2, WidgetTypes.WIDGET_YIELD_IMPORT_EXPORT, false, -1, ButtonStyles.BUTTON_STYLE_STANDARD)
				screen.addTableControlGFC("ExportTradeRouteText", 1, CITIZEN_BAR_WIDTH + (MAP_EDGE_MARGIN_WIDTH * 2) + BUILD_AREA_WIDTH, CITY_TITLE_BAR_HEIGHT + CITY_VIEW_BOX_HEIGHT_AND_WIDTH + 3 * STACK_BAR_HEIGHT / 2, BUILD_AREA_WIDTH, TRANSPORT_AREA_HEIGHT - (STACK_BAR_HEIGHT * 3), False, False, MEDIUM_BUTTON_SIZE, MEDIUM_BUTTON_SIZE, TableStyles.TABLE_STYLE_STANDARD)
				screen.setStyle("ExportTradeRouteText", "Table_EmptyScroll_Style")				

				for iYield in range(YieldTypes.NUM_YIELD_TYPES):
					if (pHeadSelectedCity.isExport(iYield)):
						iExportRow = screen.appendTableRow("ExportTradeRouteText")
						szExportText = u"<font=3>%c %s</font>" % (gc.getYieldInfo(iYield).getChar(), gc.getYieldInfo(iYield).getDescription())
						if pHeadSelectedCity.getMaintainLevel(iYield) > 0:
							szExportText += " (%s %d)" % (localText.getText("TXT_KEY_TRADE_ROUTE_MAINTAIN", ()), pHeadSelectedCity.getMaintainLevel(iYield))
						screen.setTableText("ExportTradeRouteText", 0, iExportRow, u"<font=3>%s</font>" % szExportText, "", WidgetTypes.WIDGET_YIELD_IMPORT_EXPORT, false, -1, CvUtil.FONT_LEFT_JUSTIFY )

					if (pHeadSelectedCity.isImport(iYield)):
						iImportRow = screen.appendTableRow("ImportTradeRouteText")
						szImportText = u"<font=3>%c %s</font>" % (gc.getYieldInfo(iYield).getChar(), gc.getYieldInfo(iYield).getDescription())
						screen.setTableText("ImportTradeRouteText", 0, iImportRow, szImportText, "", WidgetTypes.WIDGET_YIELD_IMPORT_EXPORT, true, -1, CvUtil.FONT_LEFT_JUSTIFY )

		# BUILDING MANGMENT PANEL
			elif BUILDING_MANAGMENT_PANEL_UP:
				screen.hide("_FXS_Screen_Bogus_Minimap_Name")
				screen.hide("CityList")
				screen.setState("AutomateProduction", pHeadSelectedCity.isProductionAutomated())
				
				screen.hide("GovernorHighlightButton")
				screen.hide("MapHighlightButton")
				screen.show("BuildHighlightButton")

				screen.show("AutomateProduction")
				screen.show("CityBuildingSelectionMultiList")
				screen.hide("EmphasizeTable")

				if (CyInterface().shouldDisplayUnitModel() and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_HIDE_ALL ):
					if (CyInterface().isCitySelection()):
						iOrders = CyInterface().getNumOrdersQueued()
						if (iOrders > 0):
							eOrderNodeType = CyInterface().getOrderNodeType(0)
							if (eOrderNodeType  == OrderTypes.ORDER_TRAIN):
								screen.addUnitGraphicGFC("InterfaceUnitModel", CyInterface().getOrderNodeData1(0), -1, CITIZEN_BAR_WIDTH + ((xResolution - CITIZEN_BAR_WIDTH) * 3 / 7) + (SMALL_BUTTON_SIZE * 2), CITY_TITLE_BAR_HEIGHT + CITY_VIEW_BOX_HEIGHT_AND_WIDTH - (MAP_EDGE_MARGIN_WIDTH * 2), BUILD_AREA_WIDTH, CITY_MULTI_TAB_AREA_HEIGHT + (MAP_EDGE_MARGIN_WIDTH * 2), WidgetTypes.WIDGET_HELP_SELECTED, -1, -1, -100, 0, 1, False )
							elif (eOrderNodeType == OrderTypes.ORDER_CONSTRUCT):
								screen.addBuildingGraphicGFC("InterfaceUnitModel", CyInterface().getOrderNodeData1(0), CITIZEN_BAR_WIDTH + ((xResolution - CITIZEN_BAR_WIDTH) * 3 / 7) + (SMALL_BUTTON_SIZE * 2), CITY_TITLE_BAR_HEIGHT + CITY_VIEW_BOX_HEIGHT_AND_WIDTH - (MAP_EDGE_MARGIN_WIDTH * 2), BUILD_AREA_WIDTH, CITY_MULTI_TAB_AREA_HEIGHT + (MAP_EDGE_MARGIN_WIDTH * 2), WidgetTypes.WIDGET_HELP_SELECTED, -1, -1, -20, 30, 0.9, False )
							screen.moveToFront("AutomateProduction")
						else:
							screen.hide("ProductionText")
							screen.hide("AutomateProduction")
						screen.show("CityBuildQueue")

		# MAP MANAGMENT PANEL
			elif MAP_MANAGMENT_PANEL_UP:
				screen.hide("InterfaceUnitModel")
				screen.hide("CityBuildingSelectionMultiList")
				screen.hide("EmphasizeTable")

				screen.addTableControlGFC("CityList", 2, CITIZEN_BAR_WIDTH + CITY_VIEW_BOX_HEIGHT_AND_WIDTH - MAP_EDGE_MARGIN_WIDTH, CITY_TITLE_BAR_HEIGHT + (STACK_BAR_HEIGHT / 2), BUILD_AREA_WIDTH, BUILD_AREA_HEIGHT - STACK_BAR_HEIGHT, False, False, self.SELECTION_PANEL_ROW_HEIGHT, self.SELECTION_PANEL_ROW_HEIGHT, TableStyles.TABLE_STYLE_STANDARD)
				screen.setStyle("CityList", "Table_EmptyScroll_Style")

				screen.setTableColumnHeader("CityList", 0, u"", BUILD_AREA_WIDTH - (BUILD_AREA_WIDTH * 3 / 10))
				screen.setTableColumnHeader("CityList", 1, u"", BUILD_AREA_WIDTH * 3 / 10)
				screen.appendTableRow("CityList")

				pPlayer = gc.getPlayer(gc.getGame().getActivePlayer())
				iRow = 0
				for iCity in range(gc.getActivePlayer().getNumCities()):
					pCity = pPlayer.getCity(iCity)
					if (pCity.getID() != pHeadSelectedCity.getID()):
						screen.appendTableRow("CityList")
						screen.setTableText("CityList", 0, iRow, pCity.getName(), "", WidgetTypes.WIDGET_GOTO_CITY, iCity, -1, CvUtil.FONT_LEFT_JUSTIFY )
						screen.setTableText("CityList", 1, iRow, u" (%d)" %(pCity.getPopulation()), "", WidgetTypes.WIDGET_GOTO_CITY, iCity, -1, CvUtil.FONT_RIGHT_JUSTIFY )
						iRow += 1

				screen.show("CityList")
				screen.show("_FXS_Screen_Bogus_Minimap_Name")
				
				screen.hide("GovernorHighlightButton")
				screen.hide("BuildHighlightButton")
				screen.show("MapHighlightButton")

		elif (not CyEngine().isGlobeviewUp() and pHeadSelectedUnit and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_HIDE_ALL and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_MINIMAP_ONLY):
			self.setMinimapButtonVisibility(True)
			if (CyInterface().getInterfaceMode() == InterfaceModeTypes.INTERFACEMODE_SELECTION):
				if (pHeadSelectedUnit.getOwner() == gc.getGame().getActivePlayer() and g_pSelectedUnit != pHeadSelectedUnit):
					g_pSelectedUnit = pHeadSelectedUnit
					iCount = 0

					actions = CyInterface().getActionsToShow()

					self.RIGHT_MAX_ANGLE = 17
					self.LEFT_MAX_ANGLE = 10
					self.ARC_SEPERATION = 2.5
					self.NADER_SIGN = 1
					self.CENTER_POINT_HEIGHT = 1.45
					self.CENTER_HUD_HEIGHT = 0.095
					iAngle = self.RIGHT_MAX_ANGLE
					self.SADDLE_BUTTON_SIZE = LARGE_BUTTON_SIZE * 2

					iTotalActions = 0
					if CyInterface().canCreateGroup():
						iTotalActions += 1
					if CyInterface().canDeleteGroup():
						iTotalActions += 1
					iTotalActions += len(actions)

					if ((self.RIGHT_MAX_ANGLE + self.LEFT_MAX_ANGLE) / self.ARC_SEPERATION) < iTotalActions:
						self.ARC_SEPERATION = float(self.RIGHT_MAX_ANGLE + self.LEFT_MAX_ANGLE) / iTotalActions

					ActionButtonList = []
					for i in actions:
						if CyInterface().canHandleAction(i, True):
							Xcord = (math.sin(math.radians(iAngle * self.NADER_SIGN)) * int(yResolution * self.CENTER_POINT_HEIGHT)) * self.NADER_SIGN * ASPECT_ADJUSTMENT + ((xResolution) / 2)
							Ycord = (math.cos(math.radians(iAngle * self.NADER_SIGN)) * int(yResolution * self.CENTER_POINT_HEIGHT)) - (yResolution * self.CENTER_POINT_HEIGHT) + yResolution - (yResolution * self.CENTER_HUD_HEIGHT)

							szName = "ActionButton" + str(iCount)
							screen.setImageButton(szName, gc.getActionInfo(i).getButton(), int(Xcord) - LARGE_BUTTON_SIZE, int(Ycord) - LARGE_BUTTON_SIZE, LARGE_BUTTON_SIZE * 2, LARGE_BUTTON_SIZE * 2, WidgetTypes.WIDGET_ACTION, i, -1)
							screen.setImageShape(szName, ImageShapes.IMAGE_SHAPE_ELLIPSE, -1)
							screen.setHitMargins(szName, 18, 18)
							if not CyInterface().canHandleAction(i, False):
								screen.overlayButtonGFC(szName, ArtFileMgr.getInterfaceArtInfo("INTERFACE_DISABLED_BUTTON").getPath())
							elif pHeadSelectedUnit.isActionRecommended(i):
								screen.overlayButtonGFC(szName, ArtFileMgr.getInterfaceArtInfo("INTERFACE_HIGHLIGHTED_BUTTON").getPath())
							ActionButtonList.append(szName)

							iAngle -= self.ARC_SEPERATION
							if iAngle < 0:
								self.NADER_SIGN = -1
							iCount += 1

					if (CyInterface().canCreateGroup()):
						Xcord = (math.sin(math.radians(iAngle * self.NADER_SIGN)) * int(yResolution * self.CENTER_POINT_HEIGHT)) * self.NADER_SIGN * ASPECT_ADJUSTMENT + ((xResolution) / 2)
						Ycord = (math.cos(math.radians(iAngle * self.NADER_SIGN)) * int(yResolution * self.CENTER_POINT_HEIGHT)) - (yResolution * self.CENTER_POINT_HEIGHT) + yResolution - (yResolution * self.CENTER_HUD_HEIGHT)
						screen.setImageButton("CreateGroupButton", ArtFileMgr.getInterfaceArtInfo("INTERFACE_BUTTONS_CREATEGROUP").getPath(), int(Xcord) - LARGE_BUTTON_SIZE, int(Ycord) - LARGE_BUTTON_SIZE, LARGE_BUTTON_SIZE * 2, LARGE_BUTTON_SIZE * 2, WidgetTypes.WIDGET_CREATE_GROUP, -1, -1)
						screen.setImageShape("CreateGroupButton", ImageShapes.IMAGE_SHAPE_ELLIPSE, -1)
						screen.setHitMargins("CreateGroupButton", 18, 18)
						ActionButtonList.append("CreateGroupButton")

						iAngle -= self.ARC_SEPERATION
						if iAngle < 0:
							self.NADER_SIGN = -1
						iCount += 1

					if (CyInterface().canDeleteGroup()):
						Xcord = (math.sin(math.radians(iAngle * self.NADER_SIGN)) * int(yResolution * self.CENTER_POINT_HEIGHT)) * self.NADER_SIGN * ASPECT_ADJUSTMENT + ((xResolution) / 2)
						Ycord = (math.cos(math.radians(iAngle * self.NADER_SIGN)) * int(yResolution * self.CENTER_POINT_HEIGHT)) - (yResolution * self.CENTER_POINT_HEIGHT) + yResolution - (yResolution * self.CENTER_HUD_HEIGHT)
						screen.setImageButton("DeleteGroupButton", ArtFileMgr.getInterfaceArtInfo("INTERFACE_BUTTONS_SPLITGROUP").getPath(), int(Xcord) - LARGE_BUTTON_SIZE, int(Ycord) - LARGE_BUTTON_SIZE, LARGE_BUTTON_SIZE * 2, LARGE_BUTTON_SIZE * 2, WidgetTypes.WIDGET_DELETE_GROUP, -1, -1)
						screen.setImageShape("DeleteGroupButton", ImageShapes.IMAGE_SHAPE_ELLIPSE, -1)
						screen.setHitMargins("DeleteGroupButton", 18, 18)
						ActionButtonList.append("DeleteGroupButton")

						iAngle -= self.ARC_SEPERATION
						if iAngle < 0:
							self.NADER_SIGN = -1
						iCount += 1

					screen.registerHideList(ActionButtonList, len(ActionButtonList), ACTION_BUTTON_HIDE)

		elif (CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_HIDE_ALL and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_MINIMAP_ONLY):
			screen.hide("CityBuildingSelectionMultiList")
			screen.hide("EmphasizeTable")
			self.setMinimapButtonVisibility(True)

		return 0
	# Will update the citizen buttons
	def updateCitizenButtons( self ):

		screen = CyGInterfaceScreen("MainInterface", CvScreenEnums.MAIN_INTERFACE )
		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()

		screen.hideList(CITIZEN_HIDE)

		if (CyInterface().isCityScreenUp()):
			CitizenHideList = []

		# ON PLOT WORKERS
			pHeadSelectedCity = CyInterface().getHeadSelectedCity()
			ButtonSize = (CITY_VIEW_BOX_HEIGHT_AND_WIDTH - (MAP_EDGE_MARGIN_WIDTH * 4))  / 3
			if (pHeadSelectedCity and CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_SHOW):
				for iPlotIndex in range(gc.getNUM_CITY_PLOTS()):
					if iPlotIndex != gc.getCITY_HOME_PLOT():
						pUnit = pHeadSelectedCity.getUnitWorkingPlot(iPlotIndex)
						pPlot = pHeadSelectedCity.getCityIndexPlot(iPlotIndex)
						if not pPlot.isNone():
							worldPosition = pPlot.getPoint()
							worldPosition.x -= gc.getPLOT_SIZE() / 2
							screenPosition = CyEngine().worldPointToScreenPoint(worldPosition)

							# PLOT DRAG ON PANELS
							screen.addDDSGFC("PlotDragOn" + str(iPlotIndex), "", int(screenPosition.x), int(screenPosition.y) - ButtonSize / 2, CITY_VIEW_BOX_HEIGHT_AND_WIDTH / 3, CITY_VIEW_BOX_HEIGHT_AND_WIDTH / 3, WidgetTypes.WIDGET_ASSIGN_CITIZEN_TO_PLOT, iPlotIndex, -1)
							CitizenHideList.append("PlotDragOn" + str(iPlotIndex))

							if (not pUnit.isNone()):
								if (pUnit.isColonistLocked()):
									szName = "CitizenButtonLock" + str(pUnit.getID())
									screen.addDDSGFC(szName, ArtFileMgr.getInterfaceArtInfo("INTERFACE_CITY_CITIZEN_LOCK").getPath(), int(screenPosition.x) - 5, int(screenPosition.y) - ButtonSize / 2 - 5, STACK_BAR_HEIGHT * 3 / 2, STACK_BAR_HEIGHT * 3 / 2, WidgetTypes.WIDGET_GENERAL, -1, -1)
									CitizenHideList.append(szName)

								szName = "WorkerButton" + str(iPlotIndex)
								screen.addDragableButton(szName, pUnit.getFullLengthIcon(), "", int(screenPosition.x), int(screenPosition.y) - ButtonSize / 2, ButtonSize / 2, ButtonSize, WidgetTypes.WIDGET_CITIZEN, pUnit.getID(), -1, ButtonStyles.BUTTON_STYLE_IMAGE )
								CitizenHideList.append(szName)
							elif (pPlot.getOwner() == pHeadSelectedCity.getOwner()):
								szName = "WorkerSlot" + str(iPlotIndex)
								screen.addDDSGFC(szName, ArtFileMgr.getInterfaceArtInfo("INTERFACE_CITIZEN_SLOT").getPath(), int(screenPosition.x), int(screenPosition.y) - ButtonSize / 2, ButtonSize / 2, ButtonSize, WidgetTypes.WIDGET_ASSIGN_CITIZEN_TO_PLOT, iPlotIndex, -1)
								CitizenHideList.append(szName)

		# IN CITY WORKERS
			pHeadSelectedCity = CyInterface().getHeadSelectedCity()
			pHeadSelectedUnit = CyInterface().getHeadSelectedUnit()
			
			if (pHeadSelectedCity and CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_SHOW):
				CitizenProfessionIndexArray = []
				CitizenBarCount = 0
				for iProfession in range(gc.getNumProfessionInfos()):
					CitizenProfessionIndexArray.append([])
						
				for j in range(pHeadSelectedCity.getPopulation()):
					pCitizen = pHeadSelectedCity.getPopulationUnitByIndex(j)
					CitizenProfessionIndexArray[pCitizen.getProfession()].append(pCitizen)
					CitizenBarCount += 1

				for iProfession in range(gc.getNumProfessionInfos()):
					if (not gc.getProfessionInfo(iProfession).isWorkPlot() and gc.getProfessionInfo(iProfession).isCitizen()):
						if gc.getCivilizationInfo(pHeadSelectedCity.getCivilizationType()).isValidProfession(iProfession):
							iSpecialBuildingType = gc.getProfessionInfo(iProfession).getSpecialBuilding()
							iYield = gc.getProfessionInfo(iProfession).getYieldProduced()
							CityBuilding = -1
							for iBuilding in range(gc.getNumBuildingInfos()):
								if (gc.getBuildingInfo(iBuilding).getSpecialBuildingType() == iSpecialBuildingType):
									if (pHeadSelectedCity.isHasBuilding(iBuilding)):
										CityBuilding = iBuilding
										break
										
							if CityBuilding != -1:
								ButtonSize = LARGE_BUTTON_SIZE * 5 / 2
								ProducedYield = pHeadSelectedCity.getBaseRawYieldProduced(iYield)
								UnproducedYield = ProducedYield - pHeadSelectedCity.calculateActualYieldProduced(iYield)
								bHasYield = (pHeadSelectedCity.getBaseRawYieldProduced(iYield) != 0 or pHeadSelectedCity.getRawYieldConsumed(iYield) != 0)
	
								if (gc.getBuildingInfo(CityBuilding).getMaxWorkers() > 0):
									CitizenSpacing = BUILDING_GRID[iSpecialBuildingType][3] / gc.getBuildingInfo(iBuilding).getMaxWorkers()
								else:
									CitizenSpacing = ButtonSize / 2

								if (CityBuilding != -1):
									screen.show("ProductionBox" + str(iSpecialBuildingType))
									szName = "YieldOutPutIcon" + str(iYield)
									screen.addDDSGFC(szName, gc.getYieldInfo(iYield).getIcon(), BUILDING_GRID[iSpecialBuildingType][0] + (STACK_BAR_HEIGHT / 2), BUILDING_GRID[iSpecialBuildingType][1] + BUILDING_GRID[iSpecialBuildingType][2] - (BUILDING_GRID[iSpecialBuildingType][2] / 6) + STACK_BAR_HEIGHT, STACK_BAR_HEIGHT * 5 / 2, STACK_BAR_HEIGHT * 5 / 2, WidgetTypes.WIDGET_HELP_YIELD, iYield, -1)
									CitizenHideList.append(szName)
								else:
									screen.hide("ProductionBox" + str(iSpecialBuildingType))
									
								ProfessionCount = len(CitizenProfessionIndexArray[iProfession])
								for GroupIndex in range(ProfessionCount):
									pCitizen = CitizenProfessionIndexArray[iProfession][GroupIndex]
									
									if (pCitizen.isColonistLocked()):
										szName = "CitizenButtonLock" + str(pCitizen.getID())
										screen.addDDSGFC(szName, ArtFileMgr.getInterfaceArtInfo("INTERFACE_CITY_CITIZEN_LOCK").getPath(), BUILDING_GRID[iSpecialBuildingType][0] + (CitizenSpacing * GroupIndex) + (CitizenSpacing / 2) - (STACK_BAR_HEIGHT / 3), BUILDING_GRID[iSpecialBuildingType][1] + BUILDING_GRID[iSpecialBuildingType][2] - (ButtonSize) - (STACK_BAR_HEIGHT / 3), STACK_BAR_HEIGHT * 3 / 2, STACK_BAR_HEIGHT * 3 / 2, WidgetTypes.WIDGET_GENERAL, -1, -1)
										CitizenHideList.append(szName)
									
									szName = "CitizenButton" + str(iProfession) + "-" + str(GroupIndex)
									screen.addDragableButton(szName, pCitizen.getFullLengthIcon(), "", BUILDING_GRID[iSpecialBuildingType][0] + (CitizenSpacing * GroupIndex) + (CitizenSpacing/ 2), BUILDING_GRID[iSpecialBuildingType][1] + BUILDING_GRID[iSpecialBuildingType][2] - (ButtonSize), ButtonSize / 2, ButtonSize, WidgetTypes.WIDGET_CITIZEN, pCitizen.getID(), -1, ButtonStyles.BUTTON_STYLE_IMAGE)							
									CitizenHideList.append(szName)
								
								for iSlot in range (gc.getBuildingInfo(CityBuilding).getMaxWorkers() - ProfessionCount):
									szName = "CitizenSlot" + str(iProfession) + "-" + str(iSlot)
									screen.addDDSGFC(szName, ArtFileMgr.getInterfaceArtInfo("INTERFACE_CITIZEN_SLOT").getPath(), BUILDING_GRID[iSpecialBuildingType][0] + (CitizenSpacing * (iSlot + ProfessionCount)) + (CitizenSpacing/ 2), BUILDING_GRID[iSpecialBuildingType][1] + BUILDING_GRID[iSpecialBuildingType][2] - ButtonSize, ButtonSize / 2, ButtonSize, WidgetTypes.WIDGET_CITY_UNIT_ASSIGN_PROFESSION, -1, iProfession)
									CitizenHideList.append(szName)
	
								SzText = ""
								if (ProducedYield > 0):
									SzText += u"<color=0,255,0> +" + str(ProducedYield) + "</color>"
								if (UnproducedYield > 0):
									SzText += u"<color=255,0,0> -" + str(UnproducedYield) + "</color>"
								
								szName = "WorkerOutputText" + str(iYield)
								screen.setLabelAt(szName, "ProductionBox" + str(iSpecialBuildingType), self.setFontSize(SzText , 1), CvUtil.FONT_RIGHT_JUSTIFY, BUILDING_GRID[iSpecialBuildingType][3] + (STACK_BAR_HEIGHT / 2), STACK_BAR_HEIGHT / 2, -0.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_HELP_YIELD, iYield, -1)
								CitizenHideList.append(szName)
								
							else:
								screen.hide("ProductionBox" + str(iSpecialBuildingType))

				iSpace = CITIZEN_BAR_WIDTH - (LARGE_BUTTON_SIZE * 3)
				iSeperation = iSpace / CitizenBarCount
				if (iSeperation > (LARGE_BUTTON_SIZE * 2)):
					iSeperation = (LARGE_BUTTON_SIZE * 2)
				iCount = 0
	
				for iProfession in range(gc.getNumProfessionInfos()):
					for GroupIndex in range(len(CitizenProfessionIndexArray[iProfession])):
						pCitizen = CitizenProfessionIndexArray[iProfession][GroupIndex]
						szName = "PopulationButton" + str(iCount)
						ButtonSize = LARGE_BUTTON_SIZE * 3
						screen.show(szName)
						screen.setState(szName, pCitizen.isColonistLocked())
						CitizenHideList.append(szName)
						iCount += 1

			screen.registerHideList(CitizenHideList, len(CitizenHideList), CITIZEN_HIDE)
			self.updateGarrisonAndTransports()
			self.updateResourceTable()

		return 0

	# Will update the Garrison and Transport Panels
	def updateGarrisonAndTransports( self ):
	
		screen = CyGInterfaceScreen("MainInterface", CvScreenEnums.MAIN_INTERFACE )
		pHeadSelectedCity = CyInterface().getHeadSelectedCity()

	# TRANSPORT MANAGMENT
		TransportButtonSize = LARGE_BUTTON_SIZE * 4 / 3
		CargoButtonSize = MEDIUM_BUTTON_SIZE
		yLocation = STACK_BAR_HEIGHT	* 3 / 2
		PanelHeight = TransportButtonSize * 6 / 5
		CyInterface().cacheInterfacePlotUnits(pHeadSelectedCity.plot())		
		screen.addScrollPanel("CityTransportPanel", u"", xResolution - TRANSPORT_AREA_WIDTH + MAP_EDGE_MARGIN_WIDTH, yResolution - BOTTOM_CENTER_HUD_HEIGHT - TRANSPORT_AREA_HEIGHT, TRANSPORT_AREA_WIDTH, TRANSPORT_AREA_HEIGHT - STACK_BAR_HEIGHT, PanelStyles.PANEL_STYLE_MAIN, false, WidgetTypes.WIDGET_RECEIVE_MOVE_CARGO_TO_CITY, -1, -1 )
		screen.setLabelAt("TransportPanelLabel", "CityTransportPanel", "    " + self.setFontSize((localText.getColorText("TXT_KEY_TRANSPORT_PANEL", (), gc.getInfoTypeForString("COLOR_FONT_CREAM"))).upper(), 0), CvUtil.FONT_LEFT_JUSTIFY, 0, STACK_BAR_HEIGHT / 2, -1.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_RECEIVE_MOVE_CARGO_TO_CITY, -1, -1 )
		for i in range(CyInterface().getNumCachedInterfacePlotUnits()):
			pLoopUnit = CyInterface().getCachedInterfacePlotUnit(i)
			if (pLoopUnit and pLoopUnit.getOwner() == pHeadSelectedCity.getOwner()):
				if (pLoopUnit.cargoSpace() > 0 and not pLoopUnit.isCargo()):
					szName = "VisitingShip" + str(i)
					xPosition = 10
					screen.addDDSGFCAt("VisitingShipPanel" + str(i), "CityTransportPanel", ArtFileMgr.getInterfaceArtInfo("INTERFACE_EUROPE_IN_PORT_BOX").getPath(), xPosition, yLocation, TransportButtonSize + pLoopUnit.cargoSpace() * (CargoButtonSize * 11 / 10), PanelHeight, WidgetTypes.WIDGET_RECEIVE_MOVE_CARGO_TO_TRANSPORT, pLoopUnit.getID(), -1, False)
					screen.addCheckBoxGFCAt("CityTransportPanel", "VisitingShipIcon" + str(i), pLoopUnit.getButton(), "", xPosition, yLocation + (PanelHeight / 2) - (TransportButtonSize / 2), TransportButtonSize, TransportButtonSize, WidgetTypes.WIDGET_RECEIVE_MOVE_CARGO_TO_TRANSPORT, pLoopUnit.getID(), -1, ButtonStyles.BUTTON_STYLE_LABEL)
					
					screen.addCheckBoxGFCAt("CityTransportPanel", "VisitingShipIcon" + str(i), pLoopUnit.getButton(), "", xPosition, yLocation + (PanelHeight / 2) - (TransportButtonSize / 2), TransportButtonSize, TransportButtonSize, WidgetTypes.WIDGET_RECEIVE_MOVE_CARGO_TO_TRANSPORT, pLoopUnit.getID(), -1, ButtonStyles.BUTTON_STYLE_LABEL )
					if pLoopUnit.isAutomated():
						screen.addDDSGFCAt("TransportAutomated" + str(i), "CityTransportPanel", ArtFileMgr.getInterfaceArtInfo("INTERFACE_BUTTONS_AUTOMATE").getPath(), xPosition + (CargoButtonSize * 3 / 4), yLocation + (PanelHeight / 2) - (TransportButtonSize / 2) + (CargoButtonSize * 3 / 4), TransportButtonSize * 3 / 4, TransportButtonSize * 3 / 4, WidgetTypes.WIDGET_RECEIVE_MOVE_CARGO_TO_TRANSPORT, pLoopUnit.getID(), -1, False)
					yLocation += (PanelHeight / 2) - (CargoButtonSize / 2)
					xPosition += TransportButtonSize 
					
					for j in range(pLoopUnit.cargoSpace()):
						screen.addDDSGFCAt("CargoCell" + str(i) + "-" + str(j), "CityTransportPanel", ArtFileMgr.getInterfaceArtInfo("INTERFACE_EUROPE_BOX_CARGO").getPath(), xPosition + (j * (CargoButtonSize * 11 / 10)), yLocation - (CargoButtonSize / 4), CargoButtonSize, CargoButtonSize, WidgetTypes.WIDGET_RECEIVE_MOVE_CARGO_TO_TRANSPORT, pLoopUnit.getID(), -1, False)

					for j in range(CyInterface().getNumCachedInterfacePlotUnits()):
						CargoUnit = CyInterface().getCachedInterfacePlotUnit(j)
						transportUnit = CargoUnit.getTransportUnit()
						if (not transportUnit.isNone() and transportUnit.getID() == pLoopUnit.getID()):
							iYield = CargoUnit.getYield()
							screen.addDragableButtonAt("CityTransportPanel", "CargoUnit" + str(j), CargoUnit.getButton(), "", xPosition, yLocation - (CargoButtonSize / 4), CargoButtonSize, CargoButtonSize, WidgetTypes.WIDGET_MOVE_CARGO_TO_CITY, pLoopUnit.getID(), CargoUnit.getID(), ButtonStyles.BUTTON_STYLE_LABEL)
							if CargoUnit.isGoods():
								szText = u"<font=3>%s</font>" % CargoUnit.getYieldStored()
								screen.setLabelAt("CargoCount" + str(j), "CityTransportPanel", szText, CvUtil.FONT_CENTER_JUSTIFY, xPosition + (CargoButtonSize / 2), yLocation + (TransportButtonSize * 4 / 6), -0.1, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
							xPosition += (CargoButtonSize * 11 / 10)
					yLocation += (PanelHeight / 2) + (CargoButtonSize / 2) + (CargoButtonSize / 10)

	# GARISSON MANAGMENT PANEL
		GarrisonButtonSize = LARGE_BUTTON_SIZE
		self.PLOT_LIST_ICON_SIZE = 12
		self.PLOT_LIST_HEALTH_BAR_HEIGHT = 11
		self.PLOT_LIST_HEALTH_BAR_SHORTENING_CORRECTION = 6
		CyInterface().cacheInterfacePlotUnits(pHeadSelectedCity.plot())		
		screen.addScrollPanel("CityGarrisonPanel", u"", CITIZEN_BAR_WIDTH, yResolution - BOTTOM_CENTER_HUD_HEIGHT - TRANSPORT_AREA_HEIGHT, xResolution - CITIZEN_BAR_WIDTH - TRANSPORT_AREA_WIDTH + STACK_BAR_HEIGHT - 4, TRANSPORT_AREA_HEIGHT - STACK_BAR_HEIGHT, PanelStyles.PANEL_STYLE_MAIN, false, WidgetTypes.WIDGET_EJECT_CITIZEN, -1, -1 )	
		screen.setLabelAt("CityGarrisonLabel", "CityGarrisonPanel", self.setFontSize((localText.getColorText("TXT_KEY_GARRISON_PANEL", (), gc.getInfoTypeForString("COLOR_FONT_CREAM"))).upper(), 0), CvUtil.FONT_LEFT_JUSTIFY, 0, STACK_BAR_HEIGHT / 2, -1.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_EJECT_CITIZEN, -1, -1 )
		xPosition = 0
		yPosition = 0
		for i in range(CyInterface().getNumCachedInterfacePlotUnits()):
			pLoopUnit = CyInterface().getCachedInterfacePlotUnit(i)
			if (pLoopUnit):
				if (pLoopUnit.getOwner() == pHeadSelectedCity.getOwner() and pLoopUnit.cargoSpace() == 0):
					if (pLoopUnit.getYield() == YieldTypes.NO_YIELD ):
						if (pLoopUnit.getTransportUnit().isNone()):
							cityGarrisonX = xPosition * GarrisonButtonSize
							cityGarrisonY = STACK_BAR_HEIGHT * 3 / 2 + (yPosition * (GarrisonButtonSize + self.PLOT_LIST_HEALTH_BAR_SHORTENING_CORRECTION))
							screen.addDragableButtonAt("CityGarrisonPanel", "GarrisonUnit" + str(i), pLoopUnit.getButton(), "", cityGarrisonX, cityGarrisonY, GarrisonButtonSize, GarrisonButtonSize, WidgetTypes.WIDGET_EJECT_CITIZEN, pLoopUnit.getID(), -1, ButtonStyles.BUTTON_STYLE_LABEL)
		
						# HEALTH BAR
							HealthBar = "GarrisonHealth" + str(i)
							screen.addStackedBarGFCAt( HealthBar, "CityGarrisonPanel", cityGarrisonX, cityGarrisonY + GarrisonButtonSize - (self.PLOT_LIST_HEALTH_BAR_HEIGHT / 2), GarrisonButtonSize, self.PLOT_LIST_HEALTH_BAR_HEIGHT, InfoBarTypes.NUM_INFOBAR_TYPES, WidgetTypes.WIDGET_EJECT_CITIZEN, pLoopUnit.getID(), -1 )
							screen.setBarPercentage( HealthBar, InfoBarTypes.INFOBAR_STORED, float( pLoopUnit.currHitPoints() ) / float( pLoopUnit.maxHitPoints() ) )
							
							if (pLoopUnit.getDamage() >= ((pLoopUnit.maxHitPoints() * 2) / 3)):
								screen.setStackedBarColors(HealthBar, InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_RED"))
							elif (pLoopUnit.getDamage() >= (pLoopUnit.maxHitPoints() / 3)):
								screen.setStackedBarColors(HealthBar, InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_YELLOW"))
							else:
								screen.setStackedBarColors(HealthBar, InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_GREEN"))

						# MOVE ICON
							if pLoopUnit.isWaiting():
								szFileName = ArtFileMgr.getInterfaceArtInfo("OVERLAY_FORTIFY").getPath()
							elif (pLoopUnit.canMove()):
								if (pLoopUnit.hasMoved()):
									szFileName = ArtFileMgr.getInterfaceArtInfo("OVERLAY_HASMOVED").getPath()
								else:
									szFileName = ArtFileMgr.getInterfaceArtInfo("OVERLAY_MOVE").getPath()
							else:
								szFileName = ArtFileMgr.getInterfaceArtInfo("OVERLAY_NOMOVE").getPath()
							screen.addDDSGFCAt( "GarrisonMoveIcon" + str(i), "CityGarrisonPanel", szFileName, xPosition * GarrisonButtonSize, (STACK_BAR_HEIGHT * 3 / 2) + (yPosition * (GarrisonButtonSize + self.PLOT_LIST_HEALTH_BAR_SHORTENING_CORRECTION)), self.PLOT_LIST_ICON_SIZE, self.PLOT_LIST_ICON_SIZE, WidgetTypes.WIDGET_EJECT_CITIZEN, -1, -1, False )										
							
							xPosition += 1
							if (xPosition == 3):
								xPosition = 0
								yPosition += 1

		return 0
		
	# Will update the game data strings
	def updateGameDataStrings( self ):

		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )
		#screen.hide( "GoldText" )
		screen.hide( "TimeText" )
		bShift = CyInterface().shiftKey()

		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()
		pHeadSelectedCity = CyInterface().getHeadSelectedCity()

		if (pHeadSelectedCity):
			ePlayer = pHeadSelectedCity.getOwner()
		else:
			ePlayer = gc.getGame().getActivePlayer()

		if ( ePlayer < 0 or ePlayer >= gc.getMAX_PLAYERS() ):
			return 0

		if ( CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_HIDE_ALL and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_MINIMAP_ONLY  and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_ADVANCED_START):
			self.updateTimeText()
			screen.setLabel("TimeText", "Background", self.setFontSize(g_szTimeText, 0), CvUtil.FONT_CENTER_JUSTIFY, xResolution / 2,  TOP_CENTER_HUD_HEIGHT / 3, -0.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
			screen.show("TimeText")

			if (gc.getPlayer(ePlayer).isAlive()):
				szText = CyGameTextMgr().getGoldStr(ePlayer)
				screen.setLabel("GoldText", "Background", self.setFontSize(szText, 1), CvUtil.FONT_CENTER_JUSTIFY, 40, yResolution - 30, -0.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )

		return 0

	def updateTimeText( self ):
		global g_szTimeText

		g_szTimeText = localText.getText("TXT_KEY_TIME_TURN", (CyGame().getGameTurn()+1, )) + u" - " + unicode(CyGameTextMgr().getInterfaceTimeStr(gc.getGame().getActivePlayer()))
		if (CyUserProfile().isClockOn()):
			g_szTimeText = getClockText() + u" - " + g_szTimeText

	# Will update the selection Data Strings
	def updateCityScreen( self ):
		screen = CyGInterfaceScreen("MainInterface", CvScreenEnums.MAIN_INTERFACE )
		pHeadSelectedCity = CyInterface().getHeadSelectedCity()

		# Find out our resolution
		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()
		bShift = CyInterface().shiftKey()
		
	# INITILIZE CITY VIEW CAMERA
		x = ((xResolution * 1.0) - ((CITY_VIEW_BOX_HEIGHT_AND_WIDTH / 2) + BUILD_AREA_WIDTH + (MAP_EDGE_MARGIN_WIDTH))) / xResolution 
		y = 1.00 - ((CITY_TITLE_BAR_PERCENT_HEIGHT + (CITY_VIEW_BOX_PERCENT_HEIGHT - (MAP_EDGE_MARGIN_PERCENT_WIDTH * 2)) / 2) / 100.0)
		CyCamera().SetCityViewPortCenter(x, y)
		
		screen.hide("TimeText")
		screen.hide("DefenseText")
		screen.hide("LibertyText")
		screen.hide("HammerText")
		screen.hide("CrossesText")
		screen.hide("EducationText")
		screen.hide("CityNameText")
		screen.hide("PopulationText")
		screen.hide("ProductionInputText")

		for iProfession in range(gc.getNumProfessionInfos()):
			if( not gc.getProfessionInfo(iProfession).isCitizen()):
				screen.hide("EjectProfession" + str(iProfession))

	# CITY SCREEN UP
		if (CyInterface().isCityScreenUp()):
			for iYield in EMPHASIZEYIELDS:
				screen.hide("MapYieldIcon" + str(iYield))
				screen.hide("MapYieldEmphasize" + str(iYield))
				screen.hide("MapYieldDe-Emphasize" + str(iYield))

			minimapWidth = min(xResolution - MAP_EDGE_MARGIN_WIDTH - CITY_MULTI_TAB_SIZE - CITIZEN_BAR_WIDTH, yResolution - BOTTOM_CENTER_HUD_HEIGHT - TRANSPORT_AREA_HEIGHT - (STACK_BAR_HEIGHT *2) - CITY_TITLE_BAR_HEIGHT - CITY_VIEW_BOX_HEIGHT_AND_WIDTH)
			xCenter = (CITIZEN_BAR_WIDTH + MAP_EDGE_MARGIN_WIDTH + xResolution - CITY_MULTI_TAB_SIZE) / 2
			yCenter = (CITY_TITLE_BAR_HEIGHT + CITY_VIEW_BOX_HEIGHT_AND_WIDTH + yResolution - BOTTOM_CENTER_HUD_HEIGHT - TRANSPORT_AREA_HEIGHT - (STACK_BAR_HEIGHT *2)) / 2
			screen.placeMinimap(xCenter - minimapWidth / 2, xCenter + minimapWidth / 2, yCenter - minimapWidth / 2, yCenter + minimapWidth / 2)
			if (pHeadSelectedCity):

				self.updateSelectionButtons()

				for j in range(gc.getMAX_PLOT_LIST_ROWS()):
					for i in range(self.numPlotListButtons()):
						szString = "PlotListButtonMulti" + str(j * self.numPlotListButtons() + i)
						screen.hide( szString )
						screen.hide( szString + "Health")
						screen.hide( szString + "Icon")
				for i in range(self.numPlotListButtons()):
					szString = "PlotListButton" + str(i)
					screen.hide( szString )
					screen.hide( szString + "Health")
					screen.hide( szString + "Icon")

				screen.show("ResourceTable")
				for iYield in range(YieldTypes.NUM_YIELD_TYPES):
					screen.show("YieldIcon" + str(iYield))

			# CITY SCROLL BUTTONS
				if (pHeadSelectedCity.getTeam() == gc.getGame().getActiveTeam()):
					if (gc.getActivePlayer().getNumCities() < 2):
						screen.hide("CityScrollMinus")
						screen.hide("CityScrollPlus")

			# CITY NAME HEADER
				szBuffer = u"<font=4>"
				if (pHeadSelectedCity.isCapital()):
					szBuffer += u"%c" %(CyGame().getSymbolID(FontSymbols.STAR_CHAR))
				szBuffer += u"%s: %d" %(pHeadSelectedCity.getName(), pHeadSelectedCity.getPopulation())
				if (pHeadSelectedCity.isOccupation()):
					szBuffer += u" (%c:%d)" %(CyGame().getSymbolID(FontSymbols.OCCUPATION_CHAR), pHeadSelectedCity.getOccupationTimer())
				szBuffer += u"</font>"
				
				localText.changeTextColor(szBuffer, gc.getInfoTypeForString("COLOR_FONT_CREAM"))
				screen.setText("CityNameText", "Background", szBuffer, CvUtil.FONT_CENTER_JUSTIFY, xResolution / 2 , CITY_TITLE_BAR_HEIGHT / 12, -0.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_CITY_NAME, -1, -1 )

			# POPULATION GROWTH/STARVATION TEXT
				iFoodDifference = pHeadSelectedCity.foodDifference()
				iProductionDiffNoFood = pHeadSelectedCity.getCurrentProductionDifference(True)
				iProductionDiffJustFood = 0
				szBuffer = u"<font=4>"
				if (iFoodDifference > 0):
					szBuffer = localText.getText("INTERFACE_CITY_GROWING", (pHeadSelectedCity.getFoodTurnsLeft(), ))
				elif (iFoodDifference < 0):
					szBuffer = localText.getText("INTERFACE_CITY_STARVING", ())
				else:
					szBuffer = localText.getText("INTERFACE_CITY_STAGNANT", ())
				szBuffer += u"</font>"
				screen.setText("PopulationText", "Background", szBuffer, CvUtil.FONT_LEFT_JUSTIFY, xResolution * 5 / 100, CITY_TITLE_BAR_HEIGHT / 8, -0.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_HELP_POPULATION, -1, -1 )
				screen.setStyle("PopulationText", "Button_Stone_Style")					
					
			# CURRENT PRODUCTION BAR FILL
				fProductionNeeded = float(pHeadSelectedCity.getProductionNeeded(YieldTypes.YIELD_HAMMERS))
				if (fProductionNeeded > 0):
					iFirst = ((float(pHeadSelectedCity.getProduction())) / fProductionNeeded)
					screen.setBarPercentage("CityProductionBar", InfoBarTypes.INFOBAR_STORED, iFirst )
					if ( iFirst == 1 ):
						iSecond = (((float(iProductionDiffNoFood)) / fProductionNeeded) )
					else:
						iSecond = (((float(iProductionDiffNoFood)) / fProductionNeeded) ) / ( 1 - iFirst )
					screen.setBarPercentage("CityProductionBar", InfoBarTypes.INFOBAR_RATE, iSecond )
					if ( iFirst + iSecond == 1 ):
						screen.setBarPercentage("CityProductionBar", InfoBarTypes.INFOBAR_RATE_EXTRA, (((float(iProductionDiffJustFood)) / fProductionNeeded) ) )
					else:
						screen.setBarPercentage("CityProductionBar", InfoBarTypes.INFOBAR_RATE_EXTRA, (( ((float(iProductionDiffJustFood)) / fProductionNeeded) ) ) / ( 1 - ( iFirst + iSecond ) ) )

			# CURRENT PRODUCTION BAR TEXT
				if (pHeadSelectedCity.isProductionBuilding() or pHeadSelectedCity.isProductionUnit()):
					szBuffer = localText.getText("INTERFACE_CITY_PRODUCTION", (pHeadSelectedCity.getProductionNameKey(), pHeadSelectedCity.getProductionTurnsLeft()))
				elif pHeadSelectedCity.isProduction():
					szBuffer = pHeadSelectedCity.getProductionName()
				else:
					szBuffer = u""
				
			# YIELD REQUIREMENT INFO
				szYieldSymbols = u""
				for iYield in range(YieldTypes.NUM_YIELD_TYPES):
					if iYield != YieldTypes.YIELD_HAMMERS:
						if (CyInterface().getOrderNodeType(0) == OrderTypes.ORDER_TRAIN ):
							iYieldRequired = gc.getPlayer(pHeadSelectedCity.getOwner()).getUnitYieldProductionNeeded(CyInterface().getOrderNodeData1(0), iYield)
						elif (CyInterface().getOrderNodeType(0) == OrderTypes.ORDER_CONSTRUCT):
							iYieldRequired = gc.getPlayer(pHeadSelectedCity.getOwner()).getBuildingYieldProductionNeeded(CyInterface().getOrderNodeData1(0), iYield)
						else:
							iYieldRequired = 0
							
						iYieldStored = pHeadSelectedCity.getYieldStored(iYield) + pHeadSelectedCity.getYieldRushed(iYield)
						if (iYieldRequired > iYieldStored):
							if (len(szYieldSymbols) > 0):
								szYieldSymbols += u","
							szYieldSymbols += u"% i%c" % (iYieldRequired - iYieldStored, gc.getYieldInfo(iYield).getChar())
							
				if len(szYieldSymbols) > 0:
					screen.setStackedBarColors("CityProductionBar", InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_RED") )
					screen.setStackedBarColors("CityProductionBar", InfoBarTypes.INFOBAR_RATE, gc.getInfoTypeForString("COLOR_WARNING_RATE") )
					szBuffer += u" " + localText.getText("TXT_KEY_PEDIA_REQUIRES", ()) + szYieldSymbols
				else:
					screen.setStackedBarColors("CityProductionBar", InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_GREAT_PEOPLE_STORED") )
					screen.setStackedBarColors("CityProductionBar", InfoBarTypes.INFOBAR_RATE, gc.getInfoTypeForString("COLOR_GREAT_PEOPLE_RATE") )
					
				screen.setLabelAt("ProductionText", "CityProductionBar", szBuffer, CvUtil.FONT_CENTER_JUSTIFY, (xResolution - CITIZEN_BAR_WIDTH - (MAP_EDGE_MARGIN_WIDTH * 2)) / 2, STACK_BAR_HEIGHT / 2, -1.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_HELP_SELECTED, 0, -1 )

			# 3 D BUILDINGS
				for iSpecial in range(gc.getNumSpecialBuildingInfos()):
					BuildingPresent = False
					for iBuilding in range(gc.getNumBuildingInfos()):
						if (pHeadSelectedCity.isHasBuilding(iBuilding)):
							if(gc.getBuildingInfo(iBuilding).getSpecialBuildingType() == iSpecial):
								BuildingPresent = True
								break

					if (BuildingPresent):
						Texture = gc.getBuildingInfo(iBuilding).getArtInfo().getCityTexture()
						screen.changeImageButton("CityBuildingGraphic" + str(iSpecial), gc.getBuildingInfo(iBuilding).getArtInfo().getCityTexture())
						screen.show("CityBuildingGraphic" + str(iSpecial))
					else:
						screen.hide("CityBuildingGraphic" + str(iSpecial))
				
			# CITIY DEFENSE MODIFIER
				iDefenseModifier = pHeadSelectedCity.getDefenseModifier()
				if (iDefenseModifier != 0):
					szBuffer = u"+%s%% %c" % (str(iDefenseModifier), CyGame().getSymbolID(FontSymbols.DEFENSE_CHAR))  #localText.getText("TXT_KEY_MAIN_CITY_DEFENSE", (CyGame().getSymbolID(FontSymbols.DEFENSE_CHAR), iDefenseModifier))
					if (pHeadSelectedCity.getDefenseDamage() > 0):
						szTempBuffer = u" (%d%%)" %(( ( gc.getMAX_CITY_DEFENSE_DAMAGE() - pHeadSelectedCity.getDefenseDamage() ) * 100 ) / gc.getMAX_CITY_DEFENSE_DAMAGE() )
						szBuffer = szBuffer + szTempBuffer
					szBuffer = "<font=3>" + szBuffer + "</font>"
					screen.setLabel("DefenseText", "Background", szBuffer, CvUtil.FONT_RIGHT_JUSTIFY, xResolution * 88 / 100, CITY_TITLE_BAR_HEIGHT / 8, -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_HELP_DEFENSE, -1, -1 )
					screen.show("DefenseText")

			# CITY HAMMER PRODUCTION
				iHammers = pHeadSelectedCity.getCurrentProductionDifference(True)
				szBuffer = u"<font=4>" + u"%i%c" % (iHammers, gc.getYieldInfo(YieldTypes.YIELD_HAMMERS).getChar()) + u"</font>"
				screen.setLabel("HammerText", "Background", szBuffer, CvUtil.FONT_CENTER_JUSTIFY, xResolution * 25 / 100, CITY_TITLE_BAR_HEIGHT / 12, -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_PRODUCTION_MOD_HELP, -1, -1 )

			# CITY LIBERTYBELL PRODUCTION
				iLiberty = pHeadSelectedCity.calculateNetYield(YieldTypes.YIELD_BELLS)
				szBuffer = u"<font=4>" + u"%i%c" % (iLiberty, gc.getYieldInfo(YieldTypes.YIELD_BELLS).getChar()) + u"</font>"
				screen.setLabel("LibertyText", "Background", szBuffer, CvUtil.FONT_CENTER_JUSTIFY, xResolution * 30 / 100, CITY_TITLE_BAR_HEIGHT / 12, -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_HELP_YIELD, YieldTypes.YIELD_BELLS, -1 )

			# CITY CROSS PRODUCTION
				iCrosses = pHeadSelectedCity.calculateNetYield(YieldTypes.YIELD_CROSSES)
				szBuffer = u"<font=4>" + u"%i%c" % (iCrosses, gc.getYieldInfo(YieldTypes.YIELD_CROSSES).getChar()) + u"</font>"
				screen.setLabel("CrossesText", "Background", szBuffer, CvUtil.FONT_CENTER_JUSTIFY, xResolution * 70 / 100, CITY_TITLE_BAR_HEIGHT / 12, -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_HELP_YIELD, YieldTypes.YIELD_CROSSES, -1 )

			# CITY EDUCATION PRODUCTION
				iBooks = pHeadSelectedCity.calculateNetYield(YieldTypes.YIELD_EDUCATION)
				szBuffer = u"<font=4>" + u"%i%c" % (iBooks, gc.getYieldInfo(YieldTypes.YIELD_EDUCATION).getChar()) + u"</font>"
				screen.setLabel("EducationText", "Background", szBuffer, CvUtil.FONT_CENTER_JUSTIFY, xResolution * 75 / 100, CITY_TITLE_BAR_HEIGHT / 12, -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_HELP_YIELD, YieldTypes.YIELD_EDUCATION, -1 )

			# REBEL BAR FILL PERCENTAGE
				fPercentage = float(pHeadSelectedCity.getRebelPercent() / 100.0)
				screen.setBarPercentage("RebelBar", InfoBarTypes.INFOBAR_STORED, fPercentage)

			# REBEL BAR TEXT
				iRebel = pHeadSelectedCity.getRebelPercent()
				szBuffer = localText.getText("TXT_KEY_MISC_REBEL", (iRebel, ))
				screen.setLabelAt("RebelText", "RebelBar", szBuffer, CvUtil.FONT_CENTER_JUSTIFY, (CITIZEN_BAR_WIDTH - (STACK_BAR_HEIGHT * 3 / 2)) / 2, STACK_BAR_HEIGHT / 2, -1.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
				screen.setHitTest("RebelText", HitTestTypes.HITTEST_NOHIT)
				screen.show("RebelText")

			screen.hide("GoldText")
			screen.hide("GoldPile")

		else: # CITY SCREEN IS DOWN
			self.MINIMAP_SIDE_MARGIN = MINIMAP_HEIGHT * 13 / 100
			screen.placeMinimap(self.MINIMAP_SIDE_MARGIN, MINIMAP_HEIGHT - self.MINIMAP_SIDE_MARGIN, yResolution - MINIMAP_HEIGHT + self.MINIMAP_SIDE_MARGIN, yResolution - self.MINIMAP_SIDE_MARGIN)
			screen.setHelpTextArea( HELPTEXT_AREA_MAX_WIDTH, FontTypes.SMALL_FONT, HELTTEXT_AREA_X_MARGIN, yResolution - SADDLE_HEIGHT, -0.1, False, "", True, False, CvUtil.FONT_LEFT_JUSTIFY, HELPTEXT_AREA_MIN_WIDTH )

			if ( CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_SHOW ):
				self.setMinimapButtonVisibility(True)
				pPlayer = gc.getPlayer(gc.getGame().getActivePlayer())

				screen.hide("ResourceTable")
				for iYield in range(YieldTypes.NUM_YIELD_TYPES):
					screen.hide("YieldIcon" + str(iYield))
					screen.hide("YieldStoredlabel" + str(iYield))
					screen.hide("YieldNetlabel" + str(iYield))

				screen.hide("LibertyText")
				screen.hide("CrossesText")
				screen.hide("HammerText")
				screen.hide("EducationText")

			for iSpecial in range(gc.getNumSpecialBuildingInfos()):
				screen.hide("CityBuildingGraphic" + str(iSpecial))
				screen.hide("ProductionBox" + str(iSpecial))

			screen.hideList(RESOURCE_TABLE_HIDE)
			screen.hide("CityList")
			
		return 0

	# Will set the table to display the out put of a city or the players whole empire
	def updateResourceTable( self ):

		screen = CyGInterfaceScreen("MainInterface", CvScreenEnums.MAIN_INTERFACE )
		ResourceHideList = []

		if not CyInterface().isCityScreenUp():
			screen.hideList(RESOURCE_TABLE_HIDE)
			return
			
		TableYields = []
		for iYield in range(YieldTypes.NUM_YIELD_TYPES):
			if gc.getYieldInfo(iYield).isCargo():
				TableYields.append(iYield)

		pCity = CyInterface().getHeadSelectedCity()
		if pCity != None:
			for index in range(len(TableYields)):
				i = TableYields[index]
				iStored = pCity.getYieldStored(i)
				iRate = pCity.calculateNetYield(i)

				if (iStored > pCity.getMaxYieldCapacity() and i != int(YieldTypes.YIELD_FOOD)):
					szStored = u"<color=255,0,0>%d</color>" %(iStored)
				else:
					szStored = u"<color=0,255,255>%d</color>" %(iStored)
				szRate = u"%d" %(iRate)
				if(iRate > 0):
					szRate = u"<color=0,255,0>+" + szRate + u"</color>"
				if(iRate == 0):
					szRate = u""
				if(iRate < 0):
					szRate = u"<color=255,255,0>" + szRate + u"</color>"

				szStorageLabel = "YieldStoredlabel" + str(i)
				screen.setLabel(szStorageLabel, "", self.setFontSize(szStored, 1), CvUtil.FONT_CENTER_JUSTIFY, STACK_BAR_HEIGHT + (i * RESOURCE_TABLE_COLUMN_WIDTH) + (RESOURCE_TABLE_COLUMN_WIDTH / 2), yResolution - (STACK_BAR_HEIGHT * 3), -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_RECEIVE_MOVE_CARGO_TO_CITY, -1, -1 )
				ResourceHideList.append(szStorageLabel)
				szRateLabel = "YieldNetlabel" + str(i)
				screen.setLabel(szRateLabel, "", self.setFontSize(szRate, 1), CvUtil.FONT_CENTER_JUSTIFY, STACK_BAR_HEIGHT + (i * RESOURCE_TABLE_COLUMN_WIDTH) + (RESOURCE_TABLE_COLUMN_WIDTH / 2), yResolution - (STACK_BAR_HEIGHT * 2), -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_RECEIVE_MOVE_CARGO_TO_CITY, -1, -1 )
				ResourceHideList.append(szRateLabel)

				screen.registerHideList(ResourceHideList, len(ResourceHideList), RESOURCE_TABLE_HIDE)
			
			# REBEL BAR FILL PERCENTAGE
				fPercentage = float(pCity.getRebelPercent() / 100.0)
				screen.setBarPercentage("RebelBar", InfoBarTypes.INFOBAR_STORED, fPercentage)

			# REBEL BAR TEXT
				iRebel = pCity.getRebelPercent()
				szBuffer = localText.getText("TXT_KEY_MISC_REBEL", (iRebel, ))
				screen.setLabelAt("RebelText", "RebelBar", szBuffer, CvUtil.FONT_CENTER_JUSTIFY, (CITIZEN_BAR_WIDTH - (STACK_BAR_HEIGHT * 3 / 2)) / 2, STACK_BAR_HEIGHT / 2, -1.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
				screen.setHitTest("RebelText", HitTestTypes.HITTEST_NOHIT)
				screen.show("RebelText")
				
			# GOLD
				screen.hide("GoldText")
				screen.hide("GoldPile")

		# EMPHISIZE DISPLAY
			for i in range(YieldTypes.NUM_YIELD_TYPES):
				if pCity.AI_getEmphasizeYieldCount(i) > 0:
					screen.hide("BonusPane" + str(i))
					screen.show("BonusPanePos" + str(i))
					screen.hide("BonusPaneNeg" + str(i))
				elif pCity.AI_getEmphasizeYieldCount(i) < 0:
					screen.hide("BonusPane" + str(i))
					screen.hide("BonusPanePos" + str(i))
					screen.show("BonusPaneNeg" + str(i))
				else:
					screen.show("BonusPane" + str(i))
					screen.hide("BonusPanePos" + str(i))
					screen.hide("BonusPaneNeg" + str(i))

			if pCity.AI_isEmphasize(AVOID_GROWTH):
				screen.overlayButtonGFC("AvoidGrowth", ArtFileMgr.getInterfaceArtInfo("INTERFACE_HIGHLIGHTED_BUTTON").getPath())
			else:
				screen.overlayButtonGFC("AvoidGrowth", None)

	# Will update the info pane strings
	def updateInfoPaneStrings( self ):

		iRow = 0
		screen = CyGInterfaceScreen("MainInterface", CvScreenEnums.MAIN_INTERFACE )

		pHeadSelectedCity = CyInterface().getHeadSelectedCity()
		pHeadSelectedUnit = CyInterface().getHeadSelectedUnit()

		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()

		bShift = CyInterface().shiftKey()

		self.SELECTION_PALEL_HEIGHT = LOWER_RIGHT_CORNER_BACKGROUND_HEIGHT - (LOWER_RIGHT_CORNER_BACKGROUND_HEIGHT / 10)
		self.SELECTION_PANEL_ROW_HEIGHT = self.SELECTION_PALEL_HEIGHT / 3
		self.SELECTION_PALEL_LEFT_COLUMN_WIDTH = 60

		screen.addPanel("SelectedUnitPanel", u"", u"", True, False, xResolution - LOWER_RIGHT_CORNER_BACKGROUND_WIDTH, yResolution - LOWER_RIGHT_CORNER_BACKGROUND_HEIGHT, LOWER_RIGHT_CORNER_BACKGROUND_WIDTH, LOWER_RIGHT_CORNER_BACKGROUND_HEIGHT - (LOWER_RIGHT_CORNER_BACKGROUND_HEIGHT / 10), PanelStyles.PANEL_STYLE_STANDARD, WidgetTypes.WIDGET_GENERAL, -1, -1)
		screen.setStyle("SelectedUnitPanel", "Panel_Game_HudStat_Style")
		screen.hide("SelectedUnitPanel")

		screen.addTableControlGFC("SelectedUnitText", 3, xResolution - LOWER_RIGHT_CORNER_BACKGROUND_WIDTH, yResolution - LOWER_RIGHT_CORNER_BACKGROUND_HEIGHT, LOWER_RIGHT_CORNER_BACKGROUND_WIDTH, LOWER_RIGHT_CORNER_BACKGROUND_HEIGHT - (LOWER_RIGHT_CORNER_BACKGROUND_HEIGHT / 10), False, False, self.SELECTION_PANEL_ROW_HEIGHT, self.SELECTION_PANEL_ROW_HEIGHT, TableStyles.TABLE_STYLE_STANDARD)
		screen.setStyle("SelectedUnitText", "Table_EmptyScroll_Style")
		screen.hide("SelectedUnitText")
		screen.hide("SelectedUnitLabel")

		QueueWidth = (xResolution - CITIZEN_BAR_WIDTH) * 3 / 7
		screen.addTableControlGFC("CityBuildQueue", 2, CITIZEN_BAR_WIDTH + 5, CITY_TITLE_BAR_HEIGHT + CITY_VIEW_BOX_HEIGHT_AND_WIDTH, QueueWidth, TRANSPORT_AREA_HEIGHT - (STACK_BAR_HEIGHT * 2), False, False, self.SELECTION_PANEL_ROW_HEIGHT, self.SELECTION_PANEL_ROW_HEIGHT, TableStyles.TABLE_STYLE_STANDARD)
		screen.setStyle("CityBuildQueue", "Table_EmptyScroll_Style")
		screen.hide("CityBuildQueue")

	# UNIT HEALTH BAR
		if (pHeadSelectedUnit and pHeadSelectedUnit.getUnitTravelState() == UnitTravelStates.NO_UNIT_TRAVEL_STATE and not pHeadSelectedCity and CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_SHOW and not CyEngine().isGlobeviewUp()):
			fPercentage = 1 - float(pHeadSelectedUnit.getDamage() / 100.0)
			screen.setBarPercentage("UnitHealthBar", InfoBarTypes.INFOBAR_STORED, fPercentage)
			if (fPercentage >= .666):
				screen.setStackedBarColors("UnitHealthBar", InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_GREEN"))
			elif (fPercentage >= .333):
				screen.setStackedBarColors("UnitHealthBar", InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_YELLOW"))
			else:
				screen.setStackedBarColors("UnitHealthBar", InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_RED"))
			screen.show("UnitHealthBar")
		else:
			screen.hide("UnitHealthBar")

		if (CyEngine().isGlobeviewUp() or CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_HIDE_ALL):
			screen.hide("GoldText")
			for iYield in EMPHASIZEYIELDS:
				screen.hide("MapYieldIcon" + str(iYield))
				screen.hide("MapYieldEmphasize" + str(iYield))
				screen.hide("MapYieldDe-Emphasize" + str(iYield))
				
			screen.hide("MapBuildingSelectionMultiList")
			screen.hide("MapCityProductionBar")
			screen.hide("MapCityBuildQueue")
			
			return
		else:
			screen.show("GoldText")
			screen.show("GoldPile")
			screen.setTableColumnHeader("CityBuildQueue", 0, u"", QueueWidth - (QueueWidth * 3 / 10))
			screen.setTableColumnHeader("CityBuildQueue", 1, u"", QueueWidth * 3 / 10)

		if (pHeadSelectedCity):
			iOrders = CyInterface().getNumOrdersQueued()

			for i in range(iOrders):
				szLeftBuffer = u""
				szRightBuffer = u""

				if ( CyInterface().getOrderNodeType(i) == OrderTypes.ORDER_TRAIN ):
					szLeftBuffer = gc.getUnitInfo(CyInterface().getOrderNodeData1(i)).getDescription()
					szRightBuffer = str(pHeadSelectedCity.getUnitProductionTurnsLeft(CyInterface().getOrderNodeData1(i), i))

					if (CyInterface().getOrderNodeSave(i)):
						szLeftBuffer = u"*" + szLeftBuffer

				elif ( CyInterface().getOrderNodeType(i) == OrderTypes.ORDER_CONSTRUCT ):
					szLeftBuffer = gc.getBuildingInfo(CyInterface().getOrderNodeData1(i)).getDescription()
					szRightBuffer = str(pHeadSelectedCity.getBuildingProductionTurnsLeft(CyInterface().getOrderNodeData1(i), i))

				elif ( CyInterface().getOrderNodeType(i) == OrderTypes.ORDER_CONVINCE ):
					szLeftBuffer = gc.getFatherPointInfo(CyInterface().getOrderNodeData1(i)).getDescription()

				screen.appendTableRow("CityBuildQueue")
				screen.setTableText("CityBuildQueue", 0, iRow, szLeftBuffer, "", WidgetTypes.WIDGET_HELP_SELECTED, i, -1, CvUtil.FONT_LEFT_JUSTIFY )
				screen.setTableText("CityBuildQueue", 1, iRow, szRightBuffer + "  ", "", WidgetTypes.WIDGET_HELP_SELECTED, i, -1, CvUtil.FONT_RIGHT_JUSTIFY )

				iRow += 1

		elif (pHeadSelectedUnit and CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_SHOW and not pHeadSelectedCity):
			screen.setTableColumnHeader("SelectedUnitText", 0, u"", LOWER_RIGHT_CORNER_BACKGROUND_WIDTH - self.SELECTION_PALEL_LEFT_COLUMN_WIDTH)
			screen.setTableColumnHeader("SelectedUnitText", 1, u"", self.SELECTION_PALEL_LEFT_COLUMN_WIDTH)

			if (CyInterface().mirrorsSelectionGroup()):
				pSelectedGroup = pHeadSelectedUnit.getGroup()
			else:
				pSelectedGroup = 0

			if (CyInterface().getLengthSelectionList() > 1):
				screen.setText("SelectedUnitLabel", "Background", localText.getText("TXT_KEY_UNIT_STACK", (CyInterface().getLengthSelectionList(), )), CvUtil.FONT_LEFT_JUSTIFY, xResolution - LOWER_RIGHT_CORNER_BACKGROUND_WIDTH, yResolution - LOWER_RIGHT_CORNER_BACKGROUND_HEIGHT , -0.1, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_UNIT_NAME, -1, -1 )
				screen.hide("UnitHealthBar")

				iRow += 1

				if ((pSelectedGroup == 0) or (pSelectedGroup.getLengthMissionQueue() <= 1)):
					if (pHeadSelectedUnit):
						for i in range(gc.getNumUnitInfos()):
							iCount = CyInterface().countEntities(i)

							if (iCount > 0):
								szRightBuffer = u""
								szLeftBuffer = gc.getUnitInfo(i).getDescription()

								if (iCount > 1):
									szRightBuffer = u"(" + str(iCount) + u")"

								szBuffer = szLeftBuffer + u"  " + szRightBuffer
								iRow = screen.appendTableRow("SelectedUnitText")
								#screen.setTableText("SelectedUnitText", 0, iRow, szLeftBuffer, "", WidgetTypes.WIDGET_HELP_SELECTED, i, -1, CvUtil.FONT_LEFT_JUSTIFY )
								#screen.setTableText("SelectedUnitText", 1, iRow, szRightBuffer, "", WidgetTypes.WIDGET_HELP_SELECTED, i, -1, CvUtil.FONT_RIGHT_JUSTIFY )
								screen.show("SelectedUnitText")
								screen.show("SelectedUnitPanel")
				#else:
					#screen.setText("SelectedUnitLabel", "Background", localText.getText("TXT_KEY_UNIT_STACK", (CyInterface().getLengthSelectionList(), )), CvUtil.FONT_LEFT_JUSTIFY, xResolution - LOWER_RIGHT_CORNER_BACKGROUND_WIDTH, yResolution - LOWER_RIGHT_CORNER_BACKGROUND_HEIGHT , -0.1, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_UNIT_NAME, -1, -1 )
			else:
				if (pHeadSelectedUnit.getHotKeyNumber() == -1):
					szBuffer = localText.getText("INTERFACE_PANE_UNIT_NAME", (pHeadSelectedUnit.getName(), ))
				else:
					szBuffer = localText.getText("INTERFACE_PANE_UNIT_NAME_HOT_KEY", (pHeadSelectedUnit.getHotKeyNumber(), pHeadSelectedUnit.getName()))
				szBuffer = "<font=2b>" + szBuffer + "</font>"
				#screen.setText("SelectedUnitLabel", "Background", szBuffer, CvUtil.FONT_RIGHT_JUSTIFY, xResolution - LOWER_RIGHT_CORNER_BACKGROUND_WIDTH, yResolution - LOWER_RIGHT_CORNER_BACKGROUND_HEIGHT, -0.1, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_UNIT_NAME, -1, -1 )

				iRow = screen.appendTableRow("SelectedUnitText")
				screen.setTableText("SelectedUnitText", 0, iRow, szBuffer, "", WidgetTypes.WIDGET_UNIT_NAME, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
				screen.show("SelectedUnitText")
				screen.show("SelectedUnitText")

				if (pHeadSelectedUnit.getProfession() != -1):
					szBuffer = localText.getText("INTERFACE_PANE_UNIT_NAME", (gc.getProfessionInfo(pHeadSelectedUnit.getProfession()).getDescription(), ))
					szBuffer = "<font=2b>" + szBuffer + "</font>"

					iRow = screen.appendTableRow("SelectedUnitText")
					screen.setTableText("SelectedUnitText", 0, iRow, szBuffer, "", WidgetTypes.WIDGET_UNIT_NAME, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
					screen.show("SelectedUnitText")
					screen.show("SelectedUnitText")

				if ((pSelectedGroup == 0) or (pSelectedGroup.getLengthMissionQueue() <= 1)):
					screen.show("SelectedUnitText")
					screen.show("SelectedUnitPanel")

					szBuffer, szLeftBuffer, szRightBuffer = u"", u"", u""

					if (pHeadSelectedUnit.canFight()):
						szLeftBuffer = localText.getText("INTERFACE_PANE_STRENGTH", ())
						if (pHeadSelectedUnit.isFighting()):
							szRightBuffer = u"?/%d%c" %(pHeadSelectedUnit.baseCombatStr(), CyGame().getSymbolID(FontSymbols.STRENGTH_CHAR))
						elif (pHeadSelectedUnit.isHurt()):
							szRightBuffer = u"%.1f/%d%c" %(((float(pHeadSelectedUnit.baseCombatStr() * pHeadSelectedUnit.currHitPoints())) / (float(pHeadSelectedUnit.maxHitPoints()))), pHeadSelectedUnit.baseCombatStr(), CyGame().getSymbolID(FontSymbols.STRENGTH_CHAR))
						else:
							szRightBuffer = u"%d%c" %(pHeadSelectedUnit.baseCombatStr(), CyGame().getSymbolID(FontSymbols.STRENGTH_CHAR))

					if pHeadSelectedUnit.baseCombatStr() > 0:
						szBuffer = szLeftBuffer + szRightBuffer
						screen.setLabelAt("StrengthText", "UnitHealthBar", szBuffer, CvUtil.FONT_CENTER_JUSTIFY, UNIT_HEALTH_BAR_WIDTH / 2, STACK_BAR_HEIGHT / 2, -1.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
					else:
						screen.hide("UnitHealthBar")

					szLeftBuffer = u""
					szRightBuffer = u""

					if ((pHeadSelectedUnit.movesLeft() % gc.getMOVE_DENOMINATOR()) > 0 ):
						iDenom = 1
					else:
						iDenom = 0
					iCurrMoves = ((pHeadSelectedUnit.movesLeft() / gc.getMOVE_DENOMINATOR()) + iDenom )
					szLeftBuffer = localText.getText("INTERFACE_PANE_MOVEMENT", ())
					if (pHeadSelectedUnit.baseMoves() == iCurrMoves):
						szRightBuffer = u"%d%c" %(pHeadSelectedUnit.baseMoves(), CyGame().getSymbolID(FontSymbols.MOVES_CHAR) )
					else:
						szRightBuffer = u"%d/%d%c" %(iCurrMoves, pHeadSelectedUnit.baseMoves(), CyGame().getSymbolID(FontSymbols.MOVES_CHAR) )

					iRow = screen.appendTableRow("SelectedUnitText")
					screen.setTableText("SelectedUnitText", 0, iRow, szLeftBuffer, "", WidgetTypes.WIDGET_HELP_SELECTED, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
					screen.setTableText("SelectedUnitText", 1, iRow, szRightBuffer, "", WidgetTypes.WIDGET_HELP_SELECTED, -1, -1, CvUtil.FONT_RIGHT_JUSTIFY )
					screen.show("SelectedUnitText")
					screen.show("SelectedUnitPanel")

					szLeftBuffer = ""
					iMaxWidthPromotions = LOWER_RIGHT_CORNER_BACKGROUND_WIDTH - self.SELECTION_PALEL_LEFT_COLUMN_WIDTH
					iCurrentWidthPromotions = 0
					for i in range(gc.getNumPromotionInfos()):
						if (pHeadSelectedUnit.isHasPromotion(i) and not gc.getPromotionInfo(i).isGraphicalOnly()):
							if iCurrentWidthPromotions + 23 > iMaxWidthPromotions:
								iRow = screen.appendTableRow("SelectedUnitText")
								screen.setTableText("SelectedUnitText", 0, iRow, szLeftBuffer, "", WidgetTypes.WIDGET_HELP_UNIT_PROMOTION, pHeadSelectedUnit.getOwner(), pHeadSelectedUnit.getID(), CvUtil.FONT_LEFT_JUSTIFY )
								iCurrentWidthPromotions = 0
								szLeftBuffer = ""
							iCurrentWidthPromotions += 23
							szLeftBuffer += "<img=%s size=16></img>" % (gc.getPromotionInfo(i).getButton(), )

					if len(szBuffer) > 0:
						iRow = screen.appendTableRow("SelectedUnitText")
						screen.setTableText("SelectedUnitText", 0, iRow, szLeftBuffer, "", WidgetTypes.WIDGET_HELP_UNIT_PROMOTION, pHeadSelectedUnit.getOwner(), pHeadSelectedUnit.getID(), CvUtil.FONT_LEFT_JUSTIFY )

			if (pSelectedGroup):
				iNodeCount = pSelectedGroup.getLengthMissionQueue()
				if (iNodeCount > 1):
					for i in range( iNodeCount ):
						szLeftBuffer = u""
						szRightBuffer = u""

						if (gc.getMissionInfo(pSelectedGroup.getMissionType(i)).isBuild()):
							if (i == 0):
								szLeftBuffer = gc.getBuildInfo(pSelectedGroup.getMissionData1(i)).getDescription()
								szRightBuffer = localText.getText("INTERFACE_CITY_TURNS", (pSelectedGroup.plot().getBuildTurnsLeft(pSelectedGroup.getMissionData1(i), 0, 0), ))
							else:
								szLeftBuffer = u"%s..." %(gc.getBuildInfo(pSelectedGroup.getMissionData1(i)).getDescription())
						else:
							szLeftBuffer = u"%s..." %(gc.getMissionInfo(pSelectedGroup.getMissionType(i)).getDescription())

						szBuffer = szLeftBuffer + "  " + szRightBuffer
						screen.appendTableRow("SelectedUnitText")
						screen.setTableText("SelectedUnitText", 0, iRow, szLeftBuffer, "", WidgetTypes.WIDGET_HELP_SELECTED, i, -1, CvUtil.FONT_LEFT_JUSTIFY )
						screen.setTableText("SelectedUnitText", 1, iRow, szRightBuffer, "", WidgetTypes.WIDGET_HELP_SELECTED, i, -1, CvUtil.FONT_RIGHT_JUSTIFY )
						screen.show("SelectedUnitText")
						screen.show("SelectedUnitPanel")
						iRow += 1

	# Will update the scores
	def updateScoreStrings( self ):
		screen = CyGInterfaceScreen("MainInterface", CvScreenEnums.MAIN_INTERFACE )

		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()

		screen.hide("ScoreBackground")

		for i in range( gc.getMAX_PLAYERS() ):
			szName = "ScoreText" + str(i)
			screen.hide( szName )

		self.SCORE_BACKGROUND_SIDE_MARGIN = 21
		self.SCORE_BACKGROUND_BOTTOM_MARGIN_LARGE = 18
		self.SCORE_BACKGROUND_BOTTOM_MARGIN_SMALL = -100
		self.SCORE_TEXT_BOTTOM_MARGIN_LARGE = 38
		self.SCORE_TEXT_BOTTOM_MARGIN_SMALL = -80

		iWidth = 0
		iCount = 0
		iBtnHeight = 22
		pHeadSelectedCity = CyInterface().getHeadSelectedCity()

		if ((CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_HIDE_ALL and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_MINIMAP_ONLY)):
			if (CyInterface().isScoresVisible() and not CyInterface().isCityScreenUp() and not CyEngine().isGlobeviewUp() ):
				i = gc.getMAX_CIV_TEAMS() - 1
				while (i > -1):
					eTeam = gc.getGame().getRankTeam(i)
					if (gc.getTeam(gc.getGame().getActiveTeam()).isHasMet(eTeam) or gc.getTeam(eTeam).isHuman() or gc.getGame().isDebugMode()):
						j = gc.getMAX_CIV_PLAYERS() - 1
						while (j > -1):
							ePlayer = gc.getGame().getRankPlayer(j)
							if (gc.getPlayer(ePlayer).isAlive()):
								if (gc.getPlayer(ePlayer).getTeam() == eTeam):
									szBuffer = u"<font=2>"

									if (gc.getGame().isGameMultiPlayer()):
										if (not (gc.getPlayer(ePlayer).isTurnActive())):
											szBuffer = szBuffer + "*"

									if gc.getGame().getPlayerScore(ePlayer) > 0:
										szBuffer += u"%d: " % gc.getGame().getPlayerScore(ePlayer)

									if (not CyInterface().isFlashingPlayer(ePlayer) or CyInterface().shouldFlash(ePlayer)):
										if (ePlayer == gc.getGame().getActivePlayer()):
											szTempBuffer = u"[<color=%d,%d,%d,%d>%s</color>]" %(gc.getPlayer(ePlayer).getPlayerTextColorR(), gc.getPlayer(ePlayer).getPlayerTextColorG(), gc.getPlayer(ePlayer).getPlayerTextColorB(), gc.getPlayer(ePlayer).getPlayerTextColorA(), gc.getPlayer(ePlayer).getName())
										else:
											szTempBuffer = u"<color=%d,%d,%d,%d>%s</color>" %(gc.getPlayer(ePlayer).getPlayerTextColorR(), gc.getPlayer(ePlayer).getPlayerTextColorG(), gc.getPlayer(ePlayer).getPlayerTextColorB(), gc.getPlayer(ePlayer).getPlayerTextColorA(), gc.getPlayer(ePlayer).getName())
									else:
										szTempBuffer = u"%s" %(gc.getPlayer(ePlayer).getName())
									szBuffer = szBuffer + szTempBuffer

									if (gc.getTeam(eTeam).isAlive()):
										if ( not (gc.getTeam(gc.getGame().getActiveTeam()).isHasMet(eTeam)) ):
											szBuffer = szBuffer + (" ?")
										if (gc.getTeam(eTeam).isAtWar(gc.getGame().getActiveTeam())):
											szBuffer = szBuffer + "("  + localText.getColorText("TXT_KEY_WAR", (), gc.getInfoTypeForString("COLOR_RED")).upper() + ")"
										if (gc.getTeam(eTeam).isOpenBorders(gc.getGame().getActiveTeam())):
											szTempBuffer = u"%c" %(CyGame().getSymbolID(FontSymbols.OPEN_BORDERS_CHAR))
											szBuffer = szBuffer + szTempBuffer
										if (gc.getTeam(eTeam).isDefensivePact(gc.getGame().getActiveTeam())):
											szTempBuffer = u"%c" %(CyGame().getSymbolID(FontSymbols.DEFENSIVE_PACT_CHAR))
											szBuffer = szBuffer + szTempBuffer

									if (CyGame().isNetworkMultiPlayer()):
										szBuffer = szBuffer + CyGameTextMgr().getNetStats(ePlayer)

									if (gc.getPlayer(ePlayer).isHuman() and CyInterface().isOOSVisible()):
										szTempBuffer = u" <color=255,0,0>* %s *</color>" %(CyGameTextMgr().getOOSSeeds(ePlayer))
										szBuffer = szBuffer + szTempBuffer

									szBuffer = szBuffer + "</font>"

									if ( CyInterface().determineWidth( szBuffer ) > iWidth ):
										iWidth = CyInterface().determineWidth( szBuffer )

									szName = "ScoreText" + str(ePlayer)
									if ( CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_SHOW or CyInterface().isInAdvancedStart() or pHeadSelectedCity != None):
										yCoord = yResolution - SADDLE_HEIGHT - self.SCORE_TEXT_BOTTOM_MARGIN_LARGE
									else:
										yCoord = yResolution - SADDLE_HEIGHT - self.SCORE_TEXT_BOTTOM_MARGIN_SMALL
									screen.setText( szName, "Background", szBuffer, CvUtil.FONT_RIGHT_JUSTIFY, xResolution - 12, yCoord - (iCount * iBtnHeight) - 31, -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_CONTACT_CIV, ePlayer, -1 )
									screen.show( szName )
									CyInterface().checkFlashReset(ePlayer)

									iCount += 1
							j = j - 1
					i = i - 1
				
				if ( CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_SHOW or CyInterface().isInAdvancedStart() or pHeadSelectedCity != None):
					yCoord = yResolution - SADDLE_HEIGHT - self.SCORE_BACKGROUND_BOTTOM_MARGIN_LARGE
				else:
					yCoord = yResolution - SADDLE_HEIGHT - self.SCORE_BACKGROUND_BOTTOM_MARGIN_SMALL

				screen.setPanelSize("ScoreBackground", xResolution - self.SCORE_BACKGROUND_SIDE_MARGIN - iWidth, yCoord - (iBtnHeight * iCount) - 35, iWidth + 12, (iBtnHeight * iCount) + 8 )
				screen.show("ScoreBackground")

	# Will update the help Strings
	def updateHelpStrings( self ):
		screen = CyGInterfaceScreen("MainInterface", CvScreenEnums.MAIN_INTERFACE )

		if ( CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_HIDE_ALL ):
			screen.setHelpTextString("")
		else:
			screen.setHelpTextString( CyInterface().getHelpString() )

		return 0

	# Will set the promotion button position
	def setPromotionButtonPosition( self, szName, iPromotionCount ):
		screen = CyGInterfaceScreen("MainInterface", CvScreenEnums.MAIN_INTERFACE )
		yResolution = screen.getYResolution()

		if ( CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_SHOW ):
			screen.moveItem( szName, 266 - (24 * (iPromotionCount / 6)), yResolution - 144 + (24 * (iPromotionCount % 6)), -0.3 )

	# Will set the selection button position
	def setScoreTextPosition( self, szButtonID, iWhichLine ):

		screen = CyGInterfaceScreen("MainInterface", CvScreenEnums.MAIN_INTERFACE )
		yResolution = screen.getYResolution()
		if ( CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_SHOW ):
			yCoord = yResolution - 180
		else:
			yCoord = yResolution - 88
		screen.moveItem( szButtonID, 996, yCoord - (iWhichLine * 18), -0.3 )

	# Will build the globeview UI
	def updateGlobeviewButtons( self ):
		kInterface = CyInterface()
		screen = CyGInterfaceScreen("MainInterface", CvScreenEnums.MAIN_INTERFACE )
		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()

		kEngine = CyEngine()
		kGLM = CyGlobeLayerManager()
		iCurrentLayerID = kGLM.getCurrentLayerID()

		# Positioning things based on the visibility of the globe
		#screen.setHelpTextArea( HELPTEXT_AREA_MAX_WIDTH, FontTypes.SMALL_FONT, HELTTEXT_AREA_X_MARGIN, yResolution - (MINI_MAP_RADIUS - 80), -0.1, False, "", True, False, CvUtil.FONT_LEFT_JUSTIFY, HELPTEXT_AREA_MIN_WIDTH)
		# Set base Y position for the LayerOptions, if we find them
		if CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_HIDE:
			iY = yResolution - iGlobeLayerOptionsY_Minimal
		else:
			iY = yResolution - LOWER_RIGHT_CORNER_BACKGROUND_HEIGHT

		# Hide the layer options ... all of them
		for i in range (20):
			szName = "GlobeLayerOption" + str(i)
			screen.hide(szName)

		# Setup the GlobeLayer panel
		if kEngine.isGlobeviewUp() and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_HIDE_ALL:
			# set up panel
			if iCurrentLayerID != -1 and kGLM.getLayer(iCurrentLayerID).getNumOptions() != 0:
				bHasOptions = True
			else:
				bHasOptions = False
				screen.hide("ScoreBackground")

			# set up toggle button
			screen.overlayButtonGFC("GlobeToggle", ArtFileMgr.getInterfaceArtInfo("INTERFACE_HIGHLIGHTED_BUTTON").getPath())

			# Set GlobeLayer indicators correctly
			for i in range(kGLM.getNumLayers()):
				szButtonID = "GlobeLayer" + str(i)
				if (iCurrentLayerID == i):
					screen.overlayButtonGFC(szButtonID, ArtFileMgr.getInterfaceArtInfo("INTERFACE_HIGHLIGHTED_BUTTON").getPath())
				else:
					screen.overlayButtonGFC(szButtonID, None)

			# Set up options pane
			if bHasOptions:
				kLayer = kGLM.getLayer(iCurrentLayerID)

				iCurY = iY
				iNumOptions = kLayer.getNumOptions()
				iCurOption = kLayer.getCurrentOption()
				iMaxTextWidth = -1
				for iTmp in range(iNumOptions):
					iOption = iTmp
					szName = "GlobeLayerOption" + str(iOption)
					szCaption = kLayer.getOptionName(iOption)
					if(iOption == iCurOption):
						szBuffer = "  <color=0,255,0>%s</color>  " % (szCaption)
					else:
						szBuffer = "  %s  " % (szCaption)
					iTextWidth = CyInterface().determineWidth( szBuffer )

					screen.setText( szName, "Background", szBuffer, CvUtil.FONT_LEFT_JUSTIFY, xResolution - 9 - iTextWidth, iCurY-iGlobeLayerOptionHeight - 10, -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_GLOBELAYER_OPTION, iOption, -1 )
					screen.show( szName )

					iCurY -= iGlobeLayerOptionHeight

					if iTextWidth > iMaxTextWidth:
						iMaxTextWidth = iTextWidth

				#make extra space
				iCurY -= iGlobeLayerOptionHeight;
				iPanelWidth = iMaxTextWidth + 32
				iPanelHeight = iY - iCurY
				iPanelX = xResolution - 14 - iPanelWidth
				iPanelY = iCurY
				screen.setPanelSize("ScoreBackground", iPanelX, iPanelY, iPanelWidth, iPanelHeight )
				screen.show("ScoreBackground")

		else:
			if iCurrentLayerID != -1:
				kLayer = kGLM.getLayer(iCurrentLayerID)
				if kLayer.getName() == "RESOURCES":
					screen.overlayButtonGFC("ResourceIcons", ArtFileMgr.getInterfaceArtInfo("INTERFACE_HIGHLIGHTED_BUTTON").getPath())
				else:
					screen.overlayButtonGFC("ResourceIcons", None)

				if kLayer.getName() == "UNITS":
					screen.overlayButtonGFC("UnitIcons", ArtFileMgr.getInterfaceArtInfo("INTERFACE_HIGHLIGHTED_BUTTON").getPath())
				else:
					screen.overlayButtonGFC("UnitIcons", None)
			else:
				screen.overlayButtonGFC("ResourceIcons", None)
				screen.overlayButtonGFC("UnitIcons", None)

			if CyUserProfile().getGrid():
				screen.overlayButtonGFC("Grid", ArtFileMgr.getInterfaceArtInfo("INTERFACE_HIGHLIGHTED_BUTTON").getPath())
			else:
				screen.overlayButtonGFC("Grid", None)
			if CyUserProfile().getMap():
				screen.overlayButtonGFC("BareMap", ArtFileMgr.getInterfaceArtInfo("INTERFACE_HIGHLIGHTED_BUTTON").getPath())
			else:
				screen.overlayButtonGFC("BareMap", None)
			if CyUserProfile().getYields():
				screen.overlayButtonGFC("Yields", ArtFileMgr.getInterfaceArtInfo("INTERFACE_HIGHLIGHTED_BUTTON").getPath())
			else:
				screen.overlayButtonGFC("Yields", None)
			if CyUserProfile().getScores():
				screen.overlayButtonGFC("ScoresVisible", ArtFileMgr.getInterfaceArtInfo("INTERFACE_HIGHLIGHTED_BUTTON").getPath())
			else:
				screen.overlayButtonGFC("ScoresVisible", None)

			screen.hide("InterfaceGlobeLayerPanel")
			screen.overlayButtonGFC("GlobeToggle", None)

	# Update minimap buttons
	def setMinimapButtonVisibility( self, bVisible ):
		screen = CyGInterfaceScreen("MainInterface", CvScreenEnums.MAIN_INTERFACE )
		kInterface = CyInterface()
		kGLM = CyGlobeLayerManager()
		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()

		if (CyInterface().isCityScreenUp()):
			bVisible = False

		kMainButtons = ["UnitIcons", "Grid", "BareMap", "Yields", "ScoresVisible", "ResourceIcons"]
		kGlobeButtons = []
		for i in range(kGLM.getNumLayers()):
			szButtonID = "GlobeLayer" + str(i)
			kGlobeButtons.append(szButtonID)

		if bVisible:
			if CyEngine().isGlobeviewUp():
				kHide = kMainButtons
				kShow = kGlobeButtons
			else:
				kHide = kGlobeButtons
				kShow = kMainButtons
			screen.show("GlobeToggle")

		else:
			kHide = kMainButtons + kGlobeButtons
			kShow = []
			screen.hide("GlobeToggle")

		for szButton in kHide:
			screen.hide(szButton)

		if len(kShow) > 0:
			for szButton in kShow:
				screen.show(szButton)

	# Will handle the input for this screen...
	def handleInput ( self, inputClass ):

		global BUILDING_MANAGMENT_PANEL_UP
		global AUTOMATION_MANAGMENT_PANEL_UP
		global MAP_MANAGMENT_PANEL_UP

		if (inputClass.getNotifyCode() == NotifyCode.NOTIFY_CLICKED):

			if (inputClass.getButtonType() == WidgetTypes.WIDGET_GENERAL and inputClass.getData1() == BUILDING_MANAGMENT_TOGGLE):
				BUILDING_MANAGMENT_PANEL_UP = True
				AUTOMATION_MANAGMENT_PANEL_UP, MAP_MANAGMENT_PANEL_UP = False, False
				self.updateSelectionButtons()

			elif (inputClass.getButtonType() == WidgetTypes.WIDGET_GENERAL and inputClass.getData1() == AUTOMATION_MANAGMENT_TOGGLE):
				AUTOMATION_MANAGMENT_PANEL_UP = True
				BUILDING_MANAGMENT_PANEL_UP, MAP_MANAGMENT_PANEL_UP = False, False
				self.updateSelectionButtons()

			elif (inputClass.getButtonType() == WidgetTypes.WIDGET_GENERAL and inputClass.getData1() == MAP_MANAGMENT_TOGGLE):
				MAP_MANAGMENT_PANEL_UP = True
				AUTOMATION_MANAGMENT_PANEL_UP, BUILDING_MANAGMENT_PANEL_UP = False, False
				self.updateSelectionButtons()

			elif (inputClass.getButtonType() == WidgetTypes.WIDGET_CLOSE_SCREEN):
				CyInterface().clearSelectedCities()		
			
		return 0
	
	# Updates the Screen
	def update( self, fDelta ):
		return

	# Adds Mouse Over Help to General Widgets
	def getWidgetHelp( self, argsList ):
		iScreen, eWidgetType, iData1, iData2, bOption = argsList

		if eWidgetType == WidgetTypes.WIDGET_GENERAL:
			if iData1 == BUILDING_MANAGMENT_TOGGLE:
				return localText.getText("TXT_KEY_INTERFACE_BUILD_TOGGLE", ());
			elif iData1 == MAP_MANAGMENT_TOGGLE:
				return localText.getText("TXT_KEY_INTERFACE_MAP_TOGGLE", ());
			elif iData1 == AUTOMATION_MANAGMENT_TOGGLE:
				return localText.getText("TXT_KEY_INTERFACE_GOVERNOR_TOGGLE", ());

		return u""
