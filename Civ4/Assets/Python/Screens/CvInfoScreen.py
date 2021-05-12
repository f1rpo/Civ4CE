## Sid Meier's Civilization 4
## Copyright Firaxis Games 2005
from CvPythonExtensions import *
import CvScreenEnums
import CvUtil
import ScreenInput

import string
import time

from PyHelpers import PyPlayer

# globals
gc = CyGlobalContext()
ArtFileMgr = CyArtFileMgr()
localText = CyTranslator()

class CvInfoScreen:
	"Info Screen! Contains the Demographics, Wonders / Top Cities and Statistics Screens"
	
	def __init__(self, screenId):
		return
		
		self.screenId = screenId
		self.DEMO_SCREEN_NAME = "DemographicsScreen"
		self.TOP_CITIES_SCREEN_NAME = "TopCitiesScreen"
		
		self.INTERFACE_ART_INFO = "TECH_BG"

		self.WIDGET_ID = "DemoScreenWidget"

		self.Z_BACKGROUND = -6.1
		self.Z_CONTROLS = self.Z_BACKGROUND - 0.2
		self.DZ = -0.2
		self.Z_HELP_AREA = self.Z_CONTROLS - 1

		self.X_SCREEN = 0
		self.Y_SCREEN = 0
		self.W_SCREEN = 1024
		self.H_SCREEN = 768
		self.X_TITLE = 512
		self.Y_TITLE = 8
		self.BORDER_WIDTH = 4
		self.W_HELP_AREA = 200
		
		self.X_EXIT = 994
		self.Y_EXIT = 730
		
		self.X_DEMOGRAPHICS_TAB = 30
		self.X_TOP_CITIES_TAB = 250
		self.X_STATS_TAB = 550
		self.Y_TABS = 730
		self.W_BUTTON = 200
		self.H_BUTTON = 30
		
		self.nWidgetCount = 0
		# This is used to allow the wonders screen to refresh without redrawing everything
		self.iNumWondersPermanentWidgets = 0
		
		self.iDemographicsID	=	0
		self.iTopCitiesID	=	1
		self.iStatsID		=	2
				
		self.iActiveTab = self.iDemographicsID
		
############################################### DEMOGRAPHICS ###############################################
		
		self.X_CHART = 460
		self.Y_CHART = 80
		self.W_CHART = 530
		self.H_CHART = 600
		
		self.X_GRAPH = 45
		self.Y_GRAPH = 110#80
		self.W_GRAPH = 400
		self.H_GRAPH = 570
		
		self.X_DEMO_DROPDOWN = 45
		self.Y_DEMO_DROPDOWN = 78#650
		self.W_DEMO_DROPDOWN = 197
		
		self.X_ALL_PLAYERS_CHECKBOX = self.X_DEMO_DROPDOWN + self.W_DEMO_DROPDOWN + 10
		self.Y_ALL_PLAYERS_CHECKBOX = self.Y_DEMO_DROPDOWN + 7
		self.BUTTON_SIZE = 20
		
		self.W_TEXT = 140
		self.H_TEXT = 15
		self.X_TEXT_BUFFER = 0
		self.Y_TEXT_BUFFER = 43
		
		self.X_COL_1 = 535
		self.X_COL_2 = self.X_COL_1 + self.W_TEXT + self.X_TEXT_BUFFER
		self.X_COL_3 = self.X_COL_2 + self.W_TEXT + self.X_TEXT_BUFFER
		self.X_COL_4 = self.X_COL_3 + self.W_TEXT + self.X_TEXT_BUFFER
		
		self.Y_ROW_1 = 100
		self.Y_ROW_2 = self.Y_ROW_1 + self.H_TEXT + self.Y_TEXT_BUFFER
		self.Y_ROW_3 = self.Y_ROW_2 + self.H_TEXT + self.Y_TEXT_BUFFER
		self.Y_ROW_4 = self.Y_ROW_3 + self.H_TEXT + self.Y_TEXT_BUFFER
		self.Y_ROW_5 = self.Y_ROW_4 + self.H_TEXT + self.Y_TEXT_BUFFER
		self.Y_ROW_6 = self.Y_ROW_5 + self.H_TEXT + self.Y_TEXT_BUFFER
		self.Y_ROW_7 = self.Y_ROW_6 + self.H_TEXT + self.Y_TEXT_BUFFER
		self.Y_ROW_8 = self.Y_ROW_7 + self.H_TEXT + self.Y_TEXT_BUFFER
		self.Y_ROW_9 = self.Y_ROW_8 + self.H_TEXT + self.Y_TEXT_BUFFER
		self.Y_ROW_10 = self.Y_ROW_9 + self.H_TEXT + self.Y_TEXT_BUFFER
		
		self.bAbleToShowAllPlayers = false
		self.iShowingPlayer = -1
		self.aiDropdownPlayerIDs = []
		
		self.iScoreTabID	= 0
		self.iEconomyTabID	= 1
		self.iIndustryTabID	= 2
		self.iAgricultureTabID	= 3
		self.iPowerTabID	= 4
		self.iCultureTabID	= 5
		
############################################### TOP CITIES ###############################################
		
		self.X_LEFT_PANE = 45
		self.Y_LEFT_PANE = 70
		self.W_LEFT_PANE = 470
		self.H_LEFT_PANE = 620
		
		# Text
		
		self.W_TC_TEXT = 280
		self.H_TC_TEXT = 15
		self.X_TC_TEXT_BUFFER = 0
		self.Y_TC_TEXT_BUFFER = 43
		
		# Animated City thingies
				
		self.X_CITY_ANIMATION = self.X_LEFT_PANE + 20
		self.Z_CITY_ANIMATION = self.Z_BACKGROUND - 0.5
		self.W_CITY_ANIMATION = 150
		self.H_CITY_ANIMATION = 110
		self.Y_CITY_ANIMATION_BUFFER = self.H_CITY_ANIMATION / 2
		
		self.X_ROTATION_CITY_ANIMATION = -20
		self.Z_ROTATION_CITY_ANIMATION = 30
		self.SCALE_ANIMATION = 0.5
		
		# Placement of Cities
		
		self.X_COL_1_CITIES = self.X_LEFT_PANE + 20
		self.Y_CITIES_BUFFER = 118
				
		self.Y_ROWS_CITIES = []
		self.Y_ROWS_CITIES.append(self.Y_LEFT_PANE + 20)
		for i in range(1,5):
			self.Y_ROWS_CITIES.append(self.Y_ROWS_CITIES[i-1] + self.Y_CITIES_BUFFER)
		
		self.X_COL_1_CITIES_DESC = self.X_LEFT_PANE + self.W_CITY_ANIMATION + 30
		self.Y_CITIES_DESC_BUFFER = -4
		self.W_CITIES_DESC = 275
		self.H_CITIES_DESC = 60
		
		self.Y_CITIES_WONDER_BUFFER = 57
		self.W_CITIES_WONDER = 275
		self.H_CITIES_WONDER = 51
		
############################################### WONDERS ###############################################
		
		self.X_RIGHT_PANE = 520
		self.Y_RIGHT_PANE = 70
		self.W_RIGHT_PANE = 460
		self.H_RIGHT_PANE = 620
		
		# Info about this wonder, e.g. name, cost so on
		
		self.X_STATS_PANE = self.X_RIGHT_PANE + 20
		self.Y_STATS_PANE = self.Y_RIGHT_PANE + 20
		self.W_STATS_PANE = 210
		self.H_STATS_PANE = 220
		
		# Wonder mode dropdown Box
		
		self.X_DROPDOWN = self.X_RIGHT_PANE + 240 + 3 # the 3 is the 'fudge factor' due to the widgets not lining up perfectly
		self.Y_DROPDOWN = self.Y_RIGHT_PANE + 20
		self.W_DROPDOWN = 200
		
		# List Box that displays all wonders built
		
		self.X_WONDER_LIST = self.X_RIGHT_PANE + 240 + 6 # the 6 is the 'fudge factor' due to the widgets not lining up perfectly
		self.Y_WONDER_LIST = self.Y_RIGHT_PANE + 60
		self.W_WONDER_LIST = 200 - 6 # the 6 is the 'fudge factor' due to the widgets not lining up perfectly
		self.H_WONDER_LIST = 180
		
		# Animated Wonder thingies
		
		self.X_WONDER_GRAPHIC = 540
		self.Y_WONDER_GRAPHIC = self.Y_RIGHT_PANE + 20 + 200 + 35
		self.W_WONDER_GRAPHIC = 420
		self.H_WONDER_GRAPHIC = 190
		
		self.X_ROTATION_WONDER_ANIMATION = -20
		self.Z_ROTATION_WONDER_ANIMATION = 30
		
		# Icons used for Projects instead because no on-map art exists
		self.X_PROJECT_ICON = self.X_WONDER_GRAPHIC + self.W_WONDER_GRAPHIC / 2
		self.Y_PROJECT_ICON = self.Y_WONDER_GRAPHIC + self.H_WONDER_GRAPHIC / 2
		self.W_PROJECT_ICON = 128
		
		# Special Stats about this wonder
		
		self.X_SPECIAL_TITLE = 540
		self.Y_SPECIAL_TITLE = 310 + 200 + 7
		
		self.X_SPECIAL_PANE = 540
		self.Y_SPECIAL_PANE = 310 + 200 + 20 + 15
		self.W_SPECIAL_PANE = 420
		self.H_SPECIAL_PANE = 140 - 15
		
		self.szWonderDisplayMode = "World Wonders"
		
		self.iWonderID = -1			# BuildingType ID of the active wonder, e.g. Palace is 0, Globe Theater is 66
		self.iActiveWonderCounter = 0		# Screen ID for this wonder (0, 1, 2, etc.) - different from the above variable
		
############################################### STATISTICS ###############################################
		
		# STATISTICS TAB
		
		# Top Panel
		
		self.X_STATS_TOP_PANEL = 45
		self.Y_STATS_TOP_PANEL = 75
		self.W_STATS_TOP_PANEL = 935
		self.H_STATS_TOP_PANEL = 180
		
		# Leader
		
		self.X_LEADER_ICON = 250
		self.Y_LEADER_ICON = 95
		self.H_LEADER_ICON = 140
		self.W_LEADER_ICON = 110
		
		# Top Chart
		
		self.X_STATS_TOP_CHART = 400
		self.Y_STATS_TOP_CHART = 130
		self.W_STATS_TOP_CHART = 380
		self.H_STATS_TOP_CHART = 102
		
		self.STATS_TOP_CHART_W_COL_0 = 304
		self.STATS_TOP_CHART_W_COL_1 = 76
		
		self.iNumTopChartCols = 2
		self.iNumTopChartRows = 4
		
		self.X_LEADER_NAME = self.X_STATS_TOP_CHART
		self.Y_LEADER_NAME = self.Y_STATS_TOP_CHART - 40
		
		# Bottom Chart
		
		self.X_STATS_BOTTOM_CHART = 45
		self.Y_STATS_BOTTOM_CHART = 280
		self.W_STATS_BOTTOM_CHART = 935
		self.H_STATS_BOTTOM_CHART = 410
		
		self.reset()
		
	def initText(self):
		
		###### TEXT ######
		self.SCREEN_TITLE = u"<font=4b>" + localText.getText("TXT_KEY_INFO_SCREEN", ()).upper() + u"</font>"
		self.SCREEN_DEMOGRAPHICS_TITLE = u"<font=4b>" + localText.getText("TXT_KEY_DEMO_SCREEN_TITLE", ()).upper() + u"</font>"
		self.SCREEN_TOP_CITIES_TITLE = u"<font=4b>" + localText.getText("TXT_KEY_CONCEPT_WONDERS", ()).upper() + " / " + localText.getText("TXT_KEY_WONDERS_SCREEN_TOP_CITIES_TEXT", ()).upper() + u"</font>"
		self.SCREEN_STATS_TITLE = u"<font=4b>" + localText.getText("TXT_KEY_INFO_SCREEN_STATISTICS_TITLE", ()).upper() + u"</font>"
		
		self.EXIT_TEXT = u"<font=4>" + localText.getText("TXT_KEY_PEDIA_SCREEN_EXIT", ()).upper() + u"</font>"
		
		self.TEXT_DEMOGRAPHICS = u"<font=4>" + localText.getText("TXT_KEY_DEMO_SCREEN_TITLE", ()).upper() + u"</font>"
		self.TEXT_DEMOGRAPHICS_SMALL = localText.getText("TXT_KEY_DEMO_SCREEN_TITLE", ())
		self.TEXT_TOP_CITIES = u"<font=4>" + localText.getText("TXT_KEY_CONCEPT_WONDERS", ()).upper() + " / " + localText.getText("TXT_KEY_WONDERS_SCREEN_TOP_CITIES_TEXT", ()).upper() + u"</font>"
		self.TEXT_STATS = u"<font=4>" + localText.getText("TXT_KEY_INFO_SCREEN_STATISTICS_TITLE", ()).upper() + u"</font>"
		self.TEXT_DEMOGRAPHICS_YELLOW = u"<font=4>" + localText.getColorText("TXT_KEY_DEMO_SCREEN_TITLE", (), gc.getInfoTypeForString("COLOR_YELLOW")).upper() + u"</font>"
		self.TEXT_TOP_CITIES_YELLOW = u"<font=4>" + localText.getColorText("TXT_KEY_CONCEPT_WONDERS", (), gc.getInfoTypeForString("COLOR_YELLOW")).upper() + " / " + localText.getColorText("TXT_KEY_WONDERS_SCREEN_TOP_CITIES_TEXT", (), gc.getInfoTypeForString("COLOR_YELLOW")).upper() + u"</font>"
		self.TEXT_STATS_YELLOW = u"<font=4>" + localText.getColorText("TXT_KEY_INFO_SCREEN_STATISTICS_TITLE", (), gc.getInfoTypeForString("COLOR_YELLOW")).upper() + u"</font>"
		
		self.TEXT_SHOW_ALL_PLAYERS = "Show All Players"
		self.TEXT_SHOW_ALL_PLAYERS_GRAY = localText.getColorText("TXT_KEY_DEMO_SCREEN_TITLE", (), gc.getInfoTypeForString("COLOR_PLAYER_GRAY")).upper()
		
		self.TEXT_SCORE = localText.getText("TXT_KEY_GAME_SCORE", ())
		self.TEXT_POWER = localText.getText("TXT_KEY_POWER", ())
		self.TEXT_CULTURE = localText.getText("TXT_KEY_COMMERCE_CULTURE", ())
		
		self.TEXT_VALUE = localText.getText("TXT_KEY_DEMO_SCREEN_VALUE_TEXT", ())
		self.TEXT_RANK = localText.getText("TXT_KEY_DEMO_SCREEN_RANK_TEXT", ())
		self.TEXT_AVERAGE = localText.getText("TXT_KEY_DEMOGRAPHICS_SCREEN_RIVAL_AVERAGE", ())
		
		self.TEXT_ECONOMY = localText.getText("TXT_KEY_DEMO_SCREEN_ECONOMY_TEXT", ())
		self.TEXT_INDUSTRY = localText.getText("TXT_KEY_DEMO_SCREEN_INDUSTRY_TEXT", ())
		self.TEXT_AGRICULTURE = localText.getText("TXT_KEY_DEMO_SCREEN_AGRICULTURE_TEXT", ())
		self.TEXT_MILITARY = localText.getText("TXT_KEY_DEMO_SCREEN_MILITARY_TEXT", ())
		self.TEXT_LAND_AREA = localText.getText("TXT_KEY_DEMO_SCREEN_LAND_AREA_TEXT", ())
		self.TEXT_POPULATION = localText.getText("TXT_KEY_DEMO_SCREEN_POPULATION_TEXT", ())
		self.TEXT_HAPPINESS = localText.getText("TXT_KEY_DEMO_SCREEN_HAPPINESS_TEXT", ())
		self.TEXT_HEALTH = localText.getText("TXT_KEY_DEMO_SCREEN_HEALTH_TEXT", ())
		self.TEXT_IMP_EXP = localText.getText("TXT_KEY_DEMO_SCREEN_IMPORTS_TEXT", ()) + "/" + localText.getText("TXT_KEY_DEMO_SCREEN_EXPORTS_TEXT", ())
		
		self.TEXT_ECONOMY_MEASURE = (u"  %c" % CyGame().getSymbolID(FontSymbols.BULLET_CHAR)) + localText.getText("TXT_KEY_DEMO_SCREEN_ECONOMY_MEASURE", ())
		self.TEXT_INDUSTRY_MEASURE = (u"  %c" % CyGame().getSymbolID(FontSymbols.BULLET_CHAR)) + localText.getText("TXT_KEY_DEMO_SCREEN_INDUSTRY_MEASURE", ())
		self.TEXT_AGRICULTURE_MEASURE = (u"  %c" % CyGame().getSymbolID(FontSymbols.BULLET_CHAR)) + localText.getText("TXT_KEY_DEMO_SCREEN_AGRICULTURE_MEASURE", ())
		self.TEXT_MILITARY_MEASURE = ""
		self.TEXT_LAND_AREA_MEASURE = (u"  %c" % CyGame().getSymbolID(FontSymbols.BULLET_CHAR)) + localText.getText("TXT_KEY_DEMO_SCREEN_LAND_AREA_MEASURE", ())
		self.TEXT_POPULATION_MEASURE = ""
		self.TEXT_HAPPINESS_MEASURE = "%"
		self.TEXT_HEALTH_MEASURE = (u"  %c" % CyGame().getSymbolID(FontSymbols.BULLET_CHAR)) + localText.getText("TXT_KEY_DEMO_SCREEN_POPULATION_MEASURE", ())
		self.TEXT_IMP_EXP_MEASURE = (u"  %c" % CyGame().getSymbolID(FontSymbols.BULLET_CHAR)) + localText.getText("TXT_KEY_DEMO_SCREEN_ECONOMY_MEASURE", ())
		
		self.TEXT_TIME_PLAYED = localText.getText("TXT_KEY_INFO_SCREEN_TIME_PLAYED", ())
		self.TEXT_CITIES_BUILT = localText.getText("TXT_KEY_INFO_SCREEN_CITIES_BUILT", ())
		self.TEXT_CITIES_RAZED = localText.getText("TXT_KEY_INFO_SCREEN_CITIES_RAZED", ())
		self.TEXT_NUM_GOLDEN_AGES = localText.getText("TXT_KEY_INFO_SCREEN_NUM_GOLDEN_AGES", ())
		self.TEXT_NUM_RELIGIONS_FOUNDED = localText.getText("TXT_KEY_INFO_SCREEN_RELIGIONS_FOUNDED", ())
		
		self.TEXT_CURRENT = localText.getText("TXT_KEY_CURRENT", ())
		self.TEXT_UNITS = localText.getText("TXT_KEY_CONCEPT_UNITS", ())
		self.TEXT_BUILDINGS = localText.getText("TXT_KEY_CONCEPT_BUILDINGS", ())
		self.TEXT_KILLED = localText.getText("TXT_KEY_INFO_SCREEN_KILLED", ())
		self.TEXT_LOST = localText.getText("TXT_KEY_INFO_SCREEN_LOST", ())
		self.TEXT_BUILT = localText.getText("TXT_KEY_INFO_SCREEN_BUILT", ())
		
	def reset(self):
		
		# City Members
		
		self.szCityNames = [    "",
					"",
					"",
					"",
					""	]
		
		self.iCitySizes = [	-1,
					-1,
					-1,
					-1,
					-1	]
		
		self.szCityDescs = [    "",
					"",
					"",
					"",
					""	]
		
		self.aaCitiesXY = [	[-1, -1],
					[-1, -1],
					[-1, -1],
					[-1, -1],
					[-1, -1]	]
		
		self.iCityValues =   [  0,
					0,
					0,
					0,
					0	]
		
		self.pCityPointers = [  0,
					0,
					0,
					0,
					0	]
		
#		self.bShowAllPlayers = false
		self.iShowingPlayer = -1
		
	def resetWonders(self):
		
		self.szWonderDisplayMode = "World Wonders"
		
		self.iWonderID = -1			# BuildingType ID of the active wonder, e.g. Palace is 0, Globe Theater is 66
		self.iActiveWonderCounter = 0		# Screen ID for this wonder (0, 1, 2, etc.) - different from the above variable
		
		self.aiWonderListBoxIDs = []
		self.aiTurnYearBuilt = []
		self.aiWonderBuiltBy = []
		
	def getScreen(self):
		return CyGInterfaceScreen(self.DEMO_SCREEN_NAME, self.screenId)

	def hideScreen(self):
		screen = self.getScreen()
		screen.hideScreen()
	
	def getLastTurn(self):
		return (gc.getGame().getReplayMessageTurn(gc.getGame().getNumReplayMessages()-1))
		
	# Screen construction function
	def showScreen(self, iTurn, iTabID):
		
		self.initText();
		
		self.iStartTurn = 0
		for iI in range(gc.getGameSpeedInfo(gc.getGame().getGameSpeedType()).getNumTurnIncrements()):
			self.iStartTurn += gc.getGameSpeedInfo(gc.getGame().getGameSpeedType()).getGameTurnInfo(iI).iNumGameTurnsPerIncrement
		self.iStartTurn *= gc.getEraInfo(gc.getGame().getStartEra()).getStartPercent()
		self.iStartTurn /= 100
	
		self.iTurn = 0

		if (iTurn > self.getLastTurn()):
			return
					
		# Create a new screen
		screen = self.getScreen()
		if screen.isActive():
			return
		screen.setRenderInterfaceOnly(True)
		screen.showScreen(PopupStates.POPUPSTATE_IMMEDIATE, False)
		
		self.reset()
		
		self.deleteAllWidgets()

		# Set the background widget and exit button
		screen.addDDSGFC("DemographicsScreenBackground", ArtFileMgr.getInterfaceArtInfo("MAINMENU_SLIDESHOW_LOAD").getPath(), 0, 0, self.W_SCREEN, self.H_SCREEN, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.addPanel( "TechTopPanel", u"", u"", True, False, 0, 0, self.W_SCREEN, 55, PanelStyles.PANEL_STYLE_TOPBAR )
		screen.addPanel( "TechBottomPanel", u"", u"", True, False, 0, 713, self.W_SCREEN, 55, PanelStyles.PANEL_STYLE_BOTTOMBAR )
		screen.showWindowBackground( False )
		screen.setDimensions(screen.centerX(self.X_SCREEN), screen.centerY(self.Y_SCREEN), self.W_SCREEN, self.H_SCREEN)
		self.szExitButtonName = self.getNextWidgetName()
		screen.setText(self.szExitButtonName, "Background", self.EXIT_TEXT, CvUtil.FONT_RIGHT_JUSTIFY, self.X_EXIT, self.Y_EXIT, self.Z_CONTROLS, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
	
		# Header...
		self.szHeaderWidget = self.getNextWidgetName()
		screen.setText(self.szHeaderWidget, "Background", self.SCREEN_TITLE, CvUtil.FONT_CENTER_JUSTIFY, self.X_TITLE, self.Y_TITLE, self.Z_CONTROLS, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
		screen.setTextHighlightColor(self.szHeaderWidget, gc.getInfoTypeForString("COLOR_BROWN_TEXT"))

		# Help area for tooltips
		screen.setHelpTextArea(self.W_HELP_AREA, FontTypes.SMALL_FONT, self.X_SCREEN, self.Y_SCREEN, self.Z_HELP_AREA, 1, ArtFileMgr.getInterfaceArtInfo("POPUPS_BACKGROUND_TRANSPARENT").getPath(), True, True, CvUtil.FONT_LEFT_JUSTIFY, 0 )
		
		self.DEBUG_DROPDOWN_ID = ""
		
		if (CyGame().isDebugMode()):
			self.DEBUG_DROPDOWN_ID = "InfoScreenDropdownWidget"
			self.szDropdownName = self.DEBUG_DROPDOWN_ID
			screen.addDropDownBoxGFC(self.szDropdownName, 22, 12, 300, WidgetTypes.WIDGET_GENERAL, -1, -1, FontTypes.GAME_FONT)
			for j in range(gc.getMAX_PLAYERS()):
				if (gc.getPlayer(j).isAlive()):
					screen.addPullDownString(self.szDropdownName, gc.getPlayer(j).getName(), j, j, False )
		
		self.iActivePlayer = CyGame().getActivePlayer()
		self.pActivePlayer = gc.getPlayer(self.iActivePlayer)
		self.iActiveTeam = self.pActivePlayer.getTeam()
		self.pActiveTeam = gc.getTeam(self.iActiveTeam)
		
		# Determine who this active player knows
		self.aiPlayersMet = []
		self.iNumPlayersMet = 0
		for iLoopPlayer in range(gc.getMAX_CIV_PLAYERS()):
			pLoopPlayer = gc.getPlayer(iLoopPlayer)
			iLoopPlayerTeam = pLoopPlayer.getTeam()
			if (gc.getTeam(iLoopPlayerTeam).isEverAlive()):
				if (self.pActiveTeam.isHasMet(iLoopPlayerTeam) or CyGame().isDebugMode()):
					self.aiPlayersMet.append(iLoopPlayer)
					self.iNumPlayersMet += 1
		
		# "Save" current widgets so they won't be deleted later when changing tabs
		self.iNumPermanentWidgets = self.nWidgetCount
		
		self.iActiveTab = iTabID
		if (self.iNumPlayersMet > 1):
			self.iShowingPlayer = 666#self.iActivePlayer
		else:
			self.iShowingPlayer = self.iActivePlayer
		
		self.redrawContents()
		
		return
		
	def redrawContents(self):
		
		screen = self.getScreen()
		self.deleteAllWidgets(self.iNumPermanentWidgets)
		self.iNumWondersPermanentWidgets = 0
		
		self.szDemographicsTabWidget = self.getNextWidgetName()
		self.szTopCitiesTabWidget = self.getNextWidgetName()
		self.szStatsTabWidget = self.getNextWidgetName()
		
		# Draw Tab buttons and tabs
		if (self.iActiveTab == self.iDemographicsID):
			screen.setText(self.szDemographicsTabWidget, "", self.TEXT_DEMOGRAPHICS_YELLOW, CvUtil.FONT_LEFT_JUSTIFY, self.X_DEMOGRAPHICS_TAB, self.Y_TABS, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
			screen.setText(self.szTopCitiesTabWidget, "", self.TEXT_TOP_CITIES, CvUtil.FONT_LEFT_JUSTIFY, self.X_TOP_CITIES_TAB, self.Y_TABS, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
			screen.setText(self.szStatsTabWidget, "", self.TEXT_STATS, CvUtil.FONT_LEFT_JUSTIFY, self.X_STATS_TAB, self.Y_TABS, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
			
			self.drawDemographicsTab()
			
		elif(self.iActiveTab == self.iTopCitiesID):
			screen.setText(self.szDemographicsTabWidget, "", self.TEXT_DEMOGRAPHICS, CvUtil.FONT_LEFT_JUSTIFY, self.X_DEMOGRAPHICS_TAB, self.Y_TABS, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
			screen.setText(self.szTopCitiesTabWidget, "", self.TEXT_TOP_CITIES_YELLOW, CvUtil.FONT_LEFT_JUSTIFY, self.X_TOP_CITIES_TAB, self.Y_TABS, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
			screen.setText(self.szStatsTabWidget, "", self.TEXT_STATS, CvUtil.FONT_LEFT_JUSTIFY, self.X_STATS_TAB, self.Y_TABS, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
			self.drawTopCitiesTab()
			
		elif(self.iActiveTab == self.iStatsID):
			screen.setText(self.szDemographicsTabWidget, "", self.TEXT_DEMOGRAPHICS, CvUtil.FONT_LEFT_JUSTIFY, self.X_DEMOGRAPHICS_TAB, self.Y_TABS, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
			screen.setText(self.szTopCitiesTabWidget, "", self.TEXT_TOP_CITIES, CvUtil.FONT_LEFT_JUSTIFY, self.X_TOP_CITIES_TAB, self.Y_TABS, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
			screen.setText(self.szStatsTabWidget, "", self.TEXT_STATS_YELLOW, CvUtil.FONT_LEFT_JUSTIFY, self.X_STATS_TAB, self.Y_TABS, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
			self.drawStatsTab()
			
#############################################################################################################
################################################# DEMOGRAPHICS ##############################################
#############################################################################################################
	
	def drawDemographicsTab(self):
		
		self.iDemoGraphTabID = self.iScoreTabID
		self.fMaxDemoStatValue = 0.0
		
		self.drawTextChart()
		self.drawPermanentGraphWidgets()
		self.drawGraph()
		self.updateGraph()
		
	def drawTextChart(self):
		
		######## DATA ########

		iNumActivePlayers = 0

		pPlayer = gc.getPlayer(self.iActivePlayer)

		iEconomy = pPlayer.calculateTotalYield(YieldTypes.YIELD_COMMERCE) - pPlayer.calculateInflatedCosts()
		iIndustry = pPlayer.calculateTotalYield(YieldTypes.YIELD_PRODUCTION)
		iAgriculture = pPlayer.calculateTotalYield(YieldTypes.YIELD_FOOD)
		fMilitary = pPlayer.getPower() * 1000
		iLandArea = pPlayer.getTotalLand() * 1000
		iPopulation = pPlayer.getRealPopulation()
		if (pPlayer.calculateTotalCityHappiness() > 0):
			iHappiness = int((1.0 * pPlayer.calculateTotalCityHappiness()) / (pPlayer.calculateTotalCityHappiness() + \
					pPlayer.calculateTotalCityUnhappiness()) * 100)
		else:
			iHappiness = 50

		if (pPlayer.calculateTotalCityHealthiness() > 0):
			iHealth = int((1.0 * pPlayer.calculateTotalCityHealthiness()) / (pPlayer.calculateTotalCityHealthiness() + \
					pPlayer.calculateTotalCityUnhealthiness()) * 100)
		else:
			iHealth = 30
		iImports = pPlayer.calculateTotalImports(YieldTypes.YIELD_COMMERCE)
		iExports = pPlayer.calculateTotalExports(YieldTypes.YIELD_COMMERCE)

		if (iExports > 0):
			if (iImports == 0):
				fImpExpRatio = 1 / (1.0 * iExports)
			else:
				fImpExpRatio = iImports / (1.0 * iExports)
		else:
			# Make ratio 1 when both imports and exports are 0
			if (iImports == 0):
				fImpExpRatio = 1.0
			else:
				fImpExpRatio = 1.0 * iImports
		
		iEconomyRank = 0
		iIndustryRank = 0
		iAgricultureRank = 0
		iMilitaryRank = 0
		iLandAreaRank = 0
		iPopulationRank = 0
		iHappinessRank = 0
		iHealthRank = 0
		iImpExpRatioRank = 0
		
		fEconomyGameAverage = 0
		fIndustryGameAverage = 0
		fAgricultureGameAverage = 0
		fMilitaryGameAverage = 0
		fLandAreaGameAverage = 0
		fPopulationGameAverage = 0
		fHappinessGameAverage = 0
		fHealthGameAverage = 0
		fImportsGameAverage = 0
		fExportsGameAverage = 0
		fImpExpRatioGameAverage = 0
		
		# Lists of Player values - will be used to determine rank, strength and average per city
		
		aiGroupEconomy = []
		aiGroupIndustry = []
		aiGroupAgriculture = []
		aiGroupMilitary = []
		aiGroupLandArea = []
		aiGroupPopulation = []
		aiGroupHappiness = []
		aiGroupHealth = []
		aiGroupImports = []
		aiGroupExports = []
		afGroupImpExpRatio = []
		
		# Loop through all players to determine Rank and relative Strength
		for iPlayerLoop in range(gc.getMAX_PLAYERS()):
			
			if (gc.getPlayer(iPlayerLoop).isAlive() and not gc.getPlayer(iPlayerLoop).isBarbarian()):
				
				iNumActivePlayers += 1
				
				pCurrPlayer = gc.getPlayer(iPlayerLoop)
				aiGroupEconomy.append(pCurrPlayer.calculateTotalYield(YieldTypes.YIELD_COMMERCE) - pCurrPlayer.calculateInflatedCosts())
				aiGroupIndustry.append(pCurrPlayer.calculateTotalYield(YieldTypes.YIELD_PRODUCTION))
				aiGroupAgriculture.append(pCurrPlayer.calculateTotalYield(YieldTypes.YIELD_FOOD))
				aiGroupMilitary.append(pCurrPlayer.getPower() * 1000)
				aiGroupLandArea.append(pCurrPlayer.getTotalLand() * 1000)
				aiGroupPopulation.append(pCurrPlayer.getRealPopulation())
				if (pCurrPlayer.calculateTotalCityHappiness() > 0):
					aiGroupHappiness.append(int((1.0 * pCurrPlayer.calculateTotalCityHappiness()) / (pCurrPlayer.calculateTotalCityHappiness() \
						+ pCurrPlayer.calculateTotalCityUnhappiness()) * 100))
				else:
					aiGroupHappiness.append(50)
				
				if (pCurrPlayer.calculateTotalCityHealthiness() > 0):
					aiGroupHealth.append(int((1.0 * pCurrPlayer.calculateTotalCityHealthiness()) / (pCurrPlayer.calculateTotalCityHealthiness() \
						+ pCurrPlayer.calculateTotalCityUnhealthiness()) * 100))
				else:
					aiGroupHealth.append(30)
				iTempImports = pCurrPlayer.calculateTotalImports(YieldTypes.YIELD_COMMERCE)
				aiGroupImports.append(iTempImports)
				iTempExports = pCurrPlayer.calculateTotalExports(YieldTypes.YIELD_COMMERCE)
				aiGroupExports.append(iTempExports)
				
				if (iTempExports > 0):
					if (iTempImports == 0):
						fGroupImpExpRatio = 1 / (1.0 * iTempExports)
						afGroupImpExpRatio.append(fGroupImpExpRatio)
					else:
						fGroupImpExpRatio = iTempImports / (1.0 * iTempExports)
						afGroupImpExpRatio.append(fGroupImpExpRatio)
				else:
					# Make ratio 1 when both imports and exports are 0
					if (iTempImports == 0):
						fGroupImpExpRatio = 1.0
						afGroupImpExpRatio.append(fGroupImpExpRatio)
					else:
						fGroupImpExpRatio = 1.0 * iTempImports
						afGroupImpExpRatio.append(fGroupImpExpRatio)

		aiGroupEconomy.sort()
		aiGroupIndustry.sort()
		aiGroupAgriculture.sort()
		aiGroupMilitary.sort()
		aiGroupLandArea.sort()
		aiGroupPopulation.sort()
		aiGroupHappiness.sort()
		aiGroupHealth.sort()
		aiGroupImports.sort()
		aiGroupExports.sort()
		afGroupImpExpRatio.sort()

		aiGroupEconomy.reverse()
		aiGroupIndustry.reverse()
		aiGroupAgriculture.reverse()
		aiGroupMilitary.reverse()
		aiGroupLandArea.reverse()
		aiGroupPopulation.reverse()
		aiGroupHappiness.reverse()
		aiGroupHealth.reverse()
		aiGroupImports.reverse()
		aiGroupExports.reverse()
		afGroupImpExpRatio.reverse()

		# Lists of player values are ordered from highest first to lowest, so determine Rank, Strength and World Average

		bEconomyFound = false
		bIndustryFound = false
		bAgricultureFound = false
		bMilitaryFound = false
		bLandAreaFound = false
		bPopulationFound = false
		bHappinessFound = false
		bHealthFound = false
		bImpExpRatioFound = false

		for i in range(len(aiGroupEconomy)):
			
			if (iEconomy == aiGroupEconomy[i] and bEconomyFound == false):
				iEconomyRank = i + 1
				bEconomyFound = true
			else:
				fEconomyGameAverage += aiGroupEconomy[i]
				
			if (iIndustry == aiGroupIndustry[i] and bIndustryFound == false):
				iIndustryRank = i + 1
				bIndustryFound = true
			else:
				fIndustryGameAverage += aiGroupIndustry[i]
				
			if (iAgriculture == aiGroupAgriculture[i] and bAgricultureFound == false):
				iAgricultureRank = i + 1
				bAgricultureFound = true
			else:
				fAgricultureGameAverage += aiGroupAgriculture[i]
				
			if (fMilitary == aiGroupMilitary[i] and bMilitaryFound == false):
				iMilitaryRank = i + 1
				bMilitaryFound = true
			else:
				fMilitaryGameAverage += aiGroupMilitary[i]
				
			if (iLandArea == aiGroupLandArea[i] and bLandAreaFound == false):
				iLandAreaRank = i + 1
				bLandAreaFound = true
			else:
				fLandAreaGameAverage += aiGroupLandArea[i]
				
			if (iPopulation == aiGroupPopulation[i] and bPopulationFound == false):
				iPopulationRank = i + 1
				bPopulationFound = true
			else:
				fPopulationGameAverage += aiGroupPopulation[i]
				
			if (iHappiness == aiGroupHappiness[i] and bHappinessFound == false):
				iHappinessRank = i + 1
				bHappinessFound = true
			else:
				fHappinessGameAverage += aiGroupHappiness[i]
				
			if (iHealth == aiGroupHealth[i] and bHealthFound == false):
				iHealthRank = i + 1
				bHealthFound = true
			else:
				fHealthGameAverage += aiGroupHealth[i]
				
			if (fImpExpRatio == afGroupImpExpRatio[i] and bImpExpRatioFound == false):
				iImpExpRatioRank = i + 1
				bImpExpRatioFound = true
			else:
				fImportsGameAverage += aiGroupImports[i]
				fExportsGameAverage += aiGroupImports[i]
			
		if (iNumActivePlayers > 1):
			fEconomyGameAverage = (1.0 * fEconomyGameAverage) / (iNumActivePlayers - 1)
			fIndustryGameAverage = (1.0 * fIndustryGameAverage) / (iNumActivePlayers - 1)
			fAgricultureGameAverage = (1.0 * fAgricultureGameAverage) / (iNumActivePlayers - 1)
			fMilitaryGameAverage = int((1.0 * fMilitaryGameAverage) / (iNumActivePlayers - 1))
			fLandAreaGameAverage = (1.0 * fLandAreaGameAverage) / (iNumActivePlayers - 1)
			fPopulationGameAverage = int((1.0 * fPopulationGameAverage) / (iNumActivePlayers - 1))
			fHappinessGameAverage = (1.0 * fHappinessGameAverage) / (iNumActivePlayers - 1)
			fHealthGameAverage = (1.0 * fHealthGameAverage) / (iNumActivePlayers - 1)
			fImportsGameAverage = (1.0 * fImportsGameAverage) / (iNumActivePlayers - 1)
			fExportsGameAverage = (1.0 * fExportsGameAverage) / (iNumActivePlayers - 1)

		######## TEXT ########

		screen = self.getScreen()

		# Create Table
		szTable = self.getNextWidgetName()
		screen.addTableControlGFC(szTable, 4, self.X_CHART, self.Y_CHART, self.W_CHART, self.H_CHART, true, true, 32,32, TableStyles.TABLE_STYLE_STANDARD)
		screen.setTableColumnHeader(szTable, 0, self.TEXT_DEMOGRAPHICS_SMALL, 190) # Total graph width is 530
		screen.setTableColumnHeader(szTable, 1, self.TEXT_VALUE, 135)
		screen.setTableColumnHeader(szTable, 2, self.TEXT_AVERAGE, 135)
		screen.setTableColumnHeader(szTable, 3, self.TEXT_RANK, 70)
		
		for i in range(18 + 5): # 18 normal items + 5 lines for spacing
			screen.appendTableRow(szTable)
		iNumRows = screen.getTableNumRows(szTable)
		iRow = iNumRows - 1
		iCol = 0
		screen.setTableText(szTable, iCol, 0, self.TEXT_ECONOMY, "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 1, self.TEXT_ECONOMY_MEASURE, "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 3, self.TEXT_INDUSTRY, "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 4, self.TEXT_INDUSTRY_MEASURE, "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 6, self.TEXT_AGRICULTURE, "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 7, self.TEXT_AGRICULTURE_MEASURE, "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 9, self.TEXT_MILITARY, "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 11, self.TEXT_LAND_AREA, "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 12, self.TEXT_LAND_AREA_MEASURE, "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 14, self.TEXT_POPULATION, "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 16, self.TEXT_HAPPINESS, "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 18, self.TEXT_HEALTH, "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 19, self.TEXT_HEALTH_MEASURE, "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 21, self.TEXT_IMP_EXP, "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 22, self.TEXT_IMP_EXP_MEASURE, "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)

		iCol = 1
		screen.setTableText(szTable, iCol, 0, str(iEconomy), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 3, str(iIndustry), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 6, str(iAgriculture), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 9, str(int(fMilitary)), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 11, str(iLandArea), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 14, str(iPopulation), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 16, str(iHappiness) + self.TEXT_HAPPINESS_MEASURE, "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 18, str(iHealth), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 21, str(iImports) + "/" + str(iExports), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		
		iCol = 2
		screen.setTableText(szTable, iCol, 0, str(int(fEconomyGameAverage)), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 3, str(int(fIndustryGameAverage)), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 6, str(int(fAgricultureGameAverage)), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 9, str(int(fMilitaryGameAverage)), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 11, str(int(fLandAreaGameAverage)), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 14, str(int(fPopulationGameAverage)), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 16, str(int(fHappinessGameAverage)) + self.TEXT_HAPPINESS_MEASURE, "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 18, str(int(fHealthGameAverage)), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 21, str(int(fImportsGameAverage)) + "/" + str(int(fExportsGameAverage)), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		
		iCol = 3
		screen.setTableText(szTable, iCol, 0, str(iEconomyRank), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 3, str(iIndustryRank), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 6, str(iAgricultureRank), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 9, str(iMilitaryRank), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 11, str(iLandAreaRank), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 14, str(iPopulationRank), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 16, str(iHappinessRank), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 18, str(iHealthRank), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.setTableText(szTable, iCol, 21, str(iImpExpRatioRank), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)

		return

	def drawGraph(self):
		
		screen = self.getScreen()
		
		print("Drawing Graph")
		
		szTextPanel = self.getNextWidgetName()
		screen.addPanel( szTextPanel, "", "", true, true,
			self.X_GRAPH, self.Y_GRAPH, self.W_GRAPH, self.H_GRAPH, PanelStyles.PANEL_STYLE_DAWNBOTTOM )
		
		######## GRAPH ########
		
		self.iNumPreDemoChartWidgets = self.nWidgetCount
		
		# Create Graph itself
		self.szGraphWidget = self.getNextWidgetName()
		screen.addGraphWidget(self.szGraphWidget, "Background", ArtFileMgr.getInterfaceArtInfo("POPUPS_BACKGROUND_TRANSPARENT").getPath(),
				      self.X_GRAPH, self.Y_GRAPH, self.Z_CONTROLS, self.W_GRAPH, self.H_GRAPH, WidgetTypes.WIDGET_GENERAL, -1, -1)

		screen.setGraphLabelX(self.szGraphWidget, localText.getText("TXT_KEY_REPLAY_SCREEN_TURNS", ()))
		screen.setGraphLabelY(self.szGraphWidget, self.TEXT_SCORE)
		
		return
		
	def drawPermanentGraphWidgets(self):
		
		screen = self.getScreen()
		
		# Dropdown Box
		self.szGraphDropdownWidget = self.getNextWidgetName()
		screen.addDropDownBoxGFC(self.szGraphDropdownWidget, self.X_DEMO_DROPDOWN, self.Y_DEMO_DROPDOWN, self.W_DEMO_DROPDOWN, WidgetTypes.WIDGET_GENERAL, -1, -1, FontTypes.GAME_FONT)
		screen.addPullDownString(self.szGraphDropdownWidget, self.TEXT_SCORE, 0, 0, False )
		screen.addPullDownString(self.szGraphDropdownWidget, self.TEXT_ECONOMY, 1, 1, False )
		screen.addPullDownString(self.szGraphDropdownWidget, self.TEXT_INDUSTRY, 2, 2, False )
		screen.addPullDownString(self.szGraphDropdownWidget, self.TEXT_AGRICULTURE, 3, 3, False )
		screen.addPullDownString(self.szGraphDropdownWidget, self.TEXT_POWER, 4, 4, False )
		screen.addPullDownString(self.szGraphDropdownWidget, self.TEXT_CULTURE, 5, 5, False )
		
		iCounter = 0
		
		self.aiDropdownPlayerIDs = []
		
		# Show Players Dropdown
		self.szCivsDropdownWidget = self.getNextWidgetName()
		screen.addDropDownBoxGFC(self.szCivsDropdownWidget, self.X_DEMO_DROPDOWN + self.W_DEMO_DROPDOWN + 8, self.Y_DEMO_DROPDOWN, self.W_DEMO_DROPDOWN, WidgetTypes.WIDGET_GENERAL, -1, -1, FontTypes.GAME_FONT)
		if (self.iNumPlayersMet > 1):
			screen.addPullDownString(self.szCivsDropdownWidget, localText.getText("TXT_KEY_SHOW_ALL_PLAYERS", ()), iCounter, iCounter, False )
			self.iShowingPlayer = 666
			self.aiDropdownPlayerIDs.append(666)
			
		else:
			self.iShowingPlayer = self.iActivePlayer
			
		# Add active player
		szCivName = gc.getPlayer(self.iActivePlayer).getCivilizationShortDescription(0)
		screen.addPullDownString(self.szCivsDropdownWidget, szCivName, iCounter, iCounter, False )
		self.aiDropdownPlayerIDs.append(self.iActivePlayer)
		
		for iPlayerIDLoop in self.aiPlayersMet:
			if (iPlayerIDLoop != self.iActivePlayer):
				self.aiDropdownPlayerIDs.append(iPlayerIDLoop)
				szCivName = gc.getPlayer(iPlayerIDLoop).getCivilizationShortDescription(0)
				screen.addPullDownString(self.szCivsDropdownWidget, szCivName, iCounter, iCounter, False )
				iCounter += 1
				
#		print("Player Dropdown List:")
		print(self.aiDropdownPlayerIDs)
				
	def updateGraph(self):

		screen = self.getScreen()
		
		print("Updating Graph")

		self.iTurn = CyGame().getGameTurn()
		iTurn = CyGame().getGameTurn()
		
		iLoopTurn = CyGame().getStartTurn()
#		print("StartTurn: %d" %(CyGame().getStartTurn()))
		
		# If player has met someone else then allow him the different graph modes
		if (self.iNumPlayersMet > 1):
			# Allow "Show All Buttons" CheckBox to be used
			self.bAbleToShowAllPlayers = true
		
#		print("iDemoGraphTabID: %s" %(str(self.iDemoGraphTabID)))
				
		# Graph should only show active player's progression if he has yet to meet any other civs
		# Or if player has elected not to show all players
		if (self.iNumPlayersMet == 1 or self.iShowingPlayer != 666):
			# Add layers and set colors
			if (self.iDemoGraphTabID == self.iScoreTabID):
				screen.setGraphLabelY(self.szGraphWidget, self.TEXT_SCORE)
				if (self.iNumPlayersMet == 1 or self.iShowingPlayer != 666):
					screen.addGraphLayer(self.szGraphWidget, 0, gc.getInfoTypeForString("COLOR_WHITE"))
			elif (self.iDemoGraphTabID == self.iEconomyTabID):
				screen.setGraphLabelY(self.szGraphWidget, self.TEXT_ECONOMY)
				if (self.iNumPlayersMet == 1 or self.iShowingPlayer != 666):
					screen.addGraphLayer(self.szGraphWidget, 0, gc.getInfoTypeForString("COLOR_YELLOW"))
			elif (self.iDemoGraphTabID == self.iIndustryTabID):
				screen.setGraphLabelY(self.szGraphWidget, self.TEXT_INDUSTRY)
				if (self.iNumPlayersMet == 1 or self.iShowingPlayer != 666):
					screen.addGraphLayer(self.szGraphWidget, 0, gc.getInfoTypeForString("COLOR_BLUE"))
			elif (self.iDemoGraphTabID == self.iAgricultureTabID):
				screen.setGraphLabelY(self.szGraphWidget, self.TEXT_AGRICULTURE)
				if (self.iNumPlayersMet == 1 or self.iShowingPlayer != 666):
					screen.addGraphLayer(self.szGraphWidget, 0, gc.getInfoTypeForString("COLOR_GREEN"))
			elif (self.iDemoGraphTabID == self.iPowerTabID):
				screen.setGraphLabelY(self.szGraphWidget, self.TEXT_POWER)
				if (self.iNumPlayersMet == 1 or self.iShowingPlayer != 666):
					screen.addGraphLayer(self.szGraphWidget, 0, gc.getInfoTypeForString("COLOR_RED"))
			elif (self.iDemoGraphTabID == self.iCultureTabID):
				screen.setGraphLabelY(self.szGraphWidget, self.TEXT_CULTURE)
				if (self.iNumPlayersMet == 1 or self.iShowingPlayer != 666):
					screen.addGraphLayer(self.szGraphWidget, 0, gc.getInfoTypeForString("COLOR_PLAYER_PURPLE"))
#			print("Drawing graph for player %s" %(str(self.iShowingPlayer)))
			screen.addGraphData(self.szGraphWidget, 0, 0, 0)
			while (iLoopTurn <= self.iTurn - 1):
				if (self.iDemoGraphTabID == self.iScoreTabID):
					fValue = (1.0 * gc.getPlayer(self.iShowingPlayer).getScoreHistory(iLoopTurn))
				elif (self.iDemoGraphTabID == self.iEconomyTabID):
					fValue = (1.0 * gc.getPlayer(self.iShowingPlayer).getEconomyHistory(iLoopTurn))
				elif (self.iDemoGraphTabID == self.iIndustryTabID):
					fValue = (1.0 * gc.getPlayer(self.iShowingPlayer).getIndustryHistory(iLoopTurn))
				elif (self.iDemoGraphTabID == self.iAgricultureTabID):
					fValue = (1.0 * gc.getPlayer(self.iShowingPlayer).getAgricultureHistory(iLoopTurn))
				elif (self.iDemoGraphTabID == self.iPowerTabID):
					fValue = (1.0 * gc.getPlayer(self.iShowingPlayer).getPowerHistory(iLoopTurn))
				elif (self.iDemoGraphTabID == self.iCultureTabID):
					fValue = (1.0 * gc.getPlayer(self.iShowingPlayer).getCultureHistory(iLoopTurn))
					
				screen.addGraphData(self.szGraphWidget, iLoopTurn + 1 - CyGame().getStartTurn(), fValue, 0)
				iLoopTurn += 1
				
			# Determine max y value for graph
			for iLoopPlayer in self.aiPlayersMet:
				iLoopTurn = CyGame().getStartTurn()
				while (iLoopTurn <= self.iTurn - 1):
					if (self.iDemoGraphTabID == self.iScoreTabID):
						fValue = (1.0 * gc.getPlayer(iLoopPlayer).getScoreHistory(iLoopTurn))
					elif (self.iDemoGraphTabID == self.iEconomyTabID):
						fValue = (1.0 * gc.getPlayer(iLoopPlayer).getEconomyHistory(iLoopTurn))
					elif (self.iDemoGraphTabID == self.iIndustryTabID):
						fValue = (1.0 * gc.getPlayer(iLoopPlayer).getIndustryHistory(iLoopTurn))
					elif (self.iDemoGraphTabID == self.iAgricultureTabID):
						fValue = (1.0 * gc.getPlayer(iLoopPlayer).getAgricultureHistory(iLoopTurn))
					elif (self.iDemoGraphTabID == self.iPowerTabID):
						fValue = (1.0 * gc.getPlayer(iLoopPlayer).getPowerHistory(iLoopTurn))
					elif (self.iDemoGraphTabID == self.iCultureTabID):
						fValue = (1.0 * gc.getPlayer(iLoopPlayer).getCultureHistory(iLoopTurn))
						
					if (fValue > self.fMaxDemoStatValue):
						self.fMaxDemoStatValue = fValue
					iLoopTurn += 1
				
			screen.setGraphYDataRange(self.szGraphWidget, 0.0, self.fMaxDemoStatValue)
				
		# We have other players to display
		else:
			iCounter = 0
			# Add graph layers and set them to the player color
			for iPlayerLoop in self.aiPlayersMet:
				pLoopPlayer = gc.getPlayer(iPlayerLoop)
				iPlayerColor = gc.getPlayerColorInfo(pLoopPlayer.getPlayerColor()).getColorTypePrimary()
				screen.addGraphLayer(self.szGraphWidget, iCounter, iPlayerColor)
				iCounter += 1
			
			screen.setGraphYDataRange(self.szGraphWidget, 0.0, 1.0)
			
			# Add first turn (which isn't stored)
			# Determine fraction for each player
			iTotalValue = 0
			iBaseline = 0
			iCounter = 0
			iBaseline = 0
			# Determine total
			for iLoopPlayer in self.aiPlayersMet:
				iTotalValue += 1.0
			for iLoopPlayer in self.aiPlayersMet:
				iValue = 1.0
				iBaseline += iValue
				if (iTotalValue > 0):
					fGraphHeight = (iValue + iTotalValue - iBaseline) / (1.0 * iTotalValue)
				else:
					fGraphHeight = 1 / (1.0 * self.iNumPlayersMet)
#				print("Layer %d - X: %d, Y: %f, iValue: %d, iBaseline: %d, iTotal: %d" %(iCounter, iLoopTurn, fGraphHeight, iValue, iBaseline, iTotalValue))
				screen.addGraphData(self.szGraphWidget, 0, fGraphHeight, iCounter)	# Counter is the (player num) Layer
				iCounter += 1
			
			while (iLoopTurn <= self.iTurn - 1):
				iTotalValue = 0
				iBaseline = 0
				iCounter = 0
#				print("iLoopTurn: %d" %(iLoopTurn))
				# Score
				if (self.iDemoGraphTabID == self.iScoreTabID):
					# Determine total
					for iLoopPlayer in self.aiPlayersMet:
						iTotalValue += gc.getPlayer(iLoopPlayer).getScoreHistory(iLoopTurn)
						
					# Determine fraction for each player
					iBaseline = 0
					for iLoopPlayer in self.aiPlayersMet:
						iValue = (1.0 * gc.getPlayer(iLoopPlayer).getScoreHistory(iLoopTurn))
						iBaseline += iValue
						if (iTotalValue > 0):
							fGraphHeight = (iValue + iTotalValue - iBaseline) / (1.0 * iTotalValue)
						else:
							fGraphHeight = 1 / (1.0 * self.iNumPlayersMet)
#						print("Layer %d - X: %d, Y: %f, iValue: %d, iBaseline: %d, iTotal: %d" %(iCounter, iLoopTurn + 1 - CyGame().getStartTurn(), fGraphHeight, iValue, iBaseline, iTotalValue))
						screen.addGraphData(self.szGraphWidget, iLoopTurn + 1 - CyGame().getStartTurn(), fGraphHeight, iCounter)	# Counter is the (player num) Layer
						iCounter += 1
				
				# Economy
				elif (self.iDemoGraphTabID == self.iEconomyTabID):
					# Determine total
					for iLoopPlayer in self.aiPlayersMet:
						iTotalValue += gc.getPlayer(iLoopPlayer).getEconomyHistory(iLoopTurn)
						
					# Determine fraction for each player
					iBaseline = 0
					for iLoopPlayer in self.aiPlayersMet:
						iValue = (1.0 * gc.getPlayer(iLoopPlayer).getEconomyHistory(iLoopTurn))
						iBaseline += iValue
						if (iTotalValue > 0):
							fGraphHeight = (iValue + iTotalValue - iBaseline) / (1.0 * iTotalValue)
						else:
							fGraphHeight = 1 / (1.0 * self.iNumPlayersMet)
						screen.addGraphData(self.szGraphWidget, iLoopTurn + 1 - CyGame().getStartTurn(), fGraphHeight, iCounter)	# Counter is the (player num) Layer
						iCounter += 1
						
				# Industry
				elif (self.iDemoGraphTabID == self.iIndustryTabID):
					# Determine total
					for iLoopPlayer in self.aiPlayersMet:
						iTotalValue += gc.getPlayer(iLoopPlayer).getIndustryHistory(iLoopTurn)
						
					# Determine fraction for each player
					iBaseline = 0
					for iLoopPlayer in self.aiPlayersMet:
						iValue = (1.0 * gc.getPlayer(iLoopPlayer).getIndustryHistory(iLoopTurn))
						iBaseline += iValue
						if (iTotalValue > 0):
							fGraphHeight = (iValue + iTotalValue - iBaseline) / (1.0 * iTotalValue)
						else:
							fGraphHeight = 1 / (1.0 * self.iNumPlayersMet)
						screen.addGraphData(self.szGraphWidget, iLoopTurn + 1 - CyGame().getStartTurn(), fGraphHeight, iCounter)	# Counter is the (player num) Layer
						iCounter += 1
						
				# Agriculture
				elif (self.iDemoGraphTabID == self.iAgricultureTabID):
					# Determine total
					for iLoopPlayer in self.aiPlayersMet:
						iTotalValue += gc.getPlayer(iLoopPlayer).getAgricultureHistory(iLoopTurn)
						
					# Determine fraction for each player
					iBaseline = 0
					for iLoopPlayer in self.aiPlayersMet:
						iValue = (1.0 * gc.getPlayer(iLoopPlayer).getAgricultureHistory(iLoopTurn))
						iBaseline += iValue
						if (iTotalValue > 0):
							fGraphHeight = (iValue + iTotalValue - iBaseline) / (1.0 * iTotalValue)
						else:
							fGraphHeight = 1 / (1.0 * self.iNumPlayersMet)
						screen.addGraphData(self.szGraphWidget, iLoopTurn + 1 - CyGame().getStartTurn(), fGraphHeight, iCounter)	# Counter is the (player num) Layer
						iCounter += 1
						
				# Power
				elif (self.iDemoGraphTabID == self.iPowerTabID):
					# Determine total
					for iLoopPlayer in self.aiPlayersMet:
						iTotalValue += gc.getPlayer(iLoopPlayer).getPowerHistory(iLoopTurn)
						
					# Determine fraction for each player
					iBaseline = 0
					for iLoopPlayer in self.aiPlayersMet:
						iValue = (1.0 * gc.getPlayer(iLoopPlayer).getPowerHistory(iLoopTurn))
						iBaseline += iValue
						if (iTotalValue > 0):
							fGraphHeight = (iValue + iTotalValue - iBaseline) / (1.0 * iTotalValue)
						else:
							fGraphHeight = 1 / (1.0 * self.iNumPlayersMet)
						screen.addGraphData(self.szGraphWidget, iLoopTurn + 1 - CyGame().getStartTurn(), fGraphHeight, iCounter)	# Counter is the (player num) Layer
						iCounter += 1
						
				# Culture
				elif (self.iDemoGraphTabID == self.iCultureTabID):
					# Determine total
					for iLoopPlayer in self.aiPlayersMet:
						iTotalValue += gc.getPlayer(iLoopPlayer).getCultureHistory(iLoopTurn)
						
					# Determine fraction for each player
					iBaseline = 0
					for iLoopPlayer in self.aiPlayersMet:
						iValue = (1.0 * gc.getPlayer(iLoopPlayer).getCultureHistory(iLoopTurn))
						iBaseline += iValue
						if (iTotalValue > 0):
							fGraphHeight = (iValue + iTotalValue - iBaseline) / (1.0 * iTotalValue)
						else:
							fGraphHeight = 1 / (1.0 * self.iNumPlayersMet)
						screen.addGraphData(self.szGraphWidget, iLoopTurn + 1 - CyGame().getStartTurn(), fGraphHeight, iCounter)	# Counter is the (player num) Layer
						iCounter += 1
						
				iLoopTurn += 1
		
#############################################################################################################
################################################## TOP CITIES ###############################################
#############################################################################################################
		
	def drawTopCitiesTab(self):
		
		screen = self.getScreen()
		
		# Background Panes
		self.szLeftPaneWidget = self.getNextWidgetName()
		screen.addPanel( self.szLeftPaneWidget, "", "", true, true,
			self.X_LEFT_PANE, self.Y_LEFT_PANE, self.W_LEFT_PANE, self.H_LEFT_PANE, PanelStyles.PANEL_STYLE_MAIN )#PanelStyles.PANEL_STYLE_DAWNTOP )
		
		self.drawTopCities()
		self.drawWondersTab()
	
	def drawTopCities(self):
		
		self.calculateTopCities()
		self.determineCityData()
		
		screen = self.getScreen()
		
		self.szCityNameWidgets = []
		self.szCityDescWidgets = []
		self.szCityAnimWidgets = []
		
		for iWidgetLoop in range(self.iNumCities):
			
			szTextPanel = self.getNextWidgetName()
			screen.addPanel( szTextPanel, "", "", false, true,
				self.X_COL_1_CITIES_DESC, self.Y_ROWS_CITIES[iWidgetLoop] + self.Y_CITIES_DESC_BUFFER, self.W_CITIES_DESC, self.H_CITIES_DESC, PanelStyles.PANEL_STYLE_DAWNTOP )
			self.szCityNameWidgets.append(self.getNextWidgetName())
#			szProjectDesc = u"<font=3b>" + pProjectInfo.getDescription().upper() + u"</font>"
			szCityDesc = u"<font=4b>" + str(self.iCitySizes[iWidgetLoop]) + u"</font>" + " - " + u"<font=3b>" + self.szCityNames[iWidgetLoop] + u"</font>" + "\n"
			szCityDesc += self.szCityDescs[iWidgetLoop]
			screen.addMultilineText(self.szCityNameWidgets[iWidgetLoop], szCityDesc,
				self.X_COL_1_CITIES_DESC + 6, self.Y_ROWS_CITIES[iWidgetLoop] + self.Y_CITIES_DESC_BUFFER + 3, self.W_CITIES_DESC - 6, self.H_CITIES_DESC - 6, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
#			screen.attachMultilineText( szTextPanel, self.szCityNameWidgets[iWidgetLoop], str(self.iCitySizes[iWidgetLoop]) + " - " + self.szCityNames[iWidgetLoop] + "\n" + self.szCityDescs[iWidgetLoop], WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
			
			iCityX = self.aaCitiesXY[iWidgetLoop][0]
			iCityY = self.aaCitiesXY[iWidgetLoop][1]
			pPlot = CyMap().plot(iCityX, iCityY)
			pCity = pPlot.getPlotCity()
			
			iDistance = 200 + (pCity.getPopulation() * 5)
			if (iDistance > 350):
				iDistance = 350
			
			self.szCityAnimWidgets.append(self.getNextWidgetName())
			screen.addPlotGraphicGFC(self.szCityAnimWidgets[iWidgetLoop], self.X_CITY_ANIMATION, self.Y_ROWS_CITIES[iWidgetLoop] + self.Y_CITY_ANIMATION_BUFFER - self.H_CITY_ANIMATION / 2, self.W_CITY_ANIMATION, self.H_CITY_ANIMATION, pPlot, iDistance, false, WidgetTypes.WIDGET_GENERAL, -1, -1)
			
		# Draw Wonder icons
		self.drawCityWonderIcons();
		
		return
		
	def drawCityWonderIcons(self):
		
		screen = self.getScreen()
		
		aaiTopCitiesWonders = []
		aiTopCitiesNumWonders = []
		for i in range(self.iNumCities):
			aaiTopCitiesWonders.append(0)
			aiTopCitiesNumWonders.append(0)
		
		# Loop through top cities and determine if they have any wonders to display
		for iCityLoop in range(self.iNumCities):
			
			if (self.pCityPointers[iCityLoop]):
				
				pCity = self.pCityPointers[iCityLoop]
				
				aiTempWondersList = []
				
				# Loop through buildings
				
				for iBuildingLoop in range(gc.getNumBuildingInfos()):
					
					pBuilding = gc.getBuildingInfo(iBuildingLoop)
					
					# If this building is a wonder...
					if (isWorldWonderClass(gc.getBuildingInfo(iBuildingLoop).getBuildingClassType())):
						
						if (pCity.hasBuilding(iBuildingLoop)):
							
							aiTempWondersList.append(iBuildingLoop)
							aiTopCitiesNumWonders[iCityLoop] += 1
							
				aaiTopCitiesWonders[iCityLoop] = aiTempWondersList
			
		# Create Scrollable areas under each city
		self.szCityWonderScrollArea = []
		for iCityLoop in range (self.iNumCities):
			
			self.szCityWonderScrollArea.append(self.getNextWidgetName())
			
			#iScollAreaY = (self.Y_CITIES_BUFFER * iCityLoop) + 90 + self.Y_CITIES_WONDER_BUFFER
			
			szIconPanel = self.szCityWonderScrollArea[iCityLoop]
			screen.addPanel( szIconPanel, "", "", false, true,
				self.X_COL_1_CITIES_DESC, self.Y_ROWS_CITIES[iCityLoop] + self.Y_CITIES_WONDER_BUFFER + self.Y_CITIES_DESC_BUFFER, self.W_CITIES_DESC, self.H_CITIES_DESC, PanelStyles.PANEL_STYLE_DAWNTOP )
			
			# Now place the wonder buttons
			for iWonderLoop in range(aiTopCitiesNumWonders[iCityLoop]):
				
				iBuildingID = aaiTopCitiesWonders[iCityLoop][iWonderLoop]
				screen.attachImageButton( szIconPanel, "", gc.getBuildingInfo(iBuildingID).getButton(),
				    GenericButtonSizes.BUTTON_SIZE_46, WidgetTypes.WIDGET_PEDIA_JUMP_TO_BUILDING, iBuildingID, -1, False )
			
	def calculateTopCities(self):
		
		# Calculate the top 5 cities

		for iPlayerLoop in range(gc.getMAX_PLAYERS()):
			
			apCityList = PyPlayer(iPlayerLoop).getCityList()
			
			for pCity in apCityList:
				
				iTotalCityValue = ((pCity.getCulture() / 5) + (pCity.getFoodRate() + pCity.getProductionRate() \
					+ pCity.calculateGoldRate())) * pCity.getPopulation()
				
				for iRankLoop in range(5):
					
					if (iTotalCityValue > self.iCityValues[iRankLoop] and not pCity.isBarbarian()):
						
						self.addCityToList(iRankLoop, pCity, iTotalCityValue)
						
						break
			
	# Recursive
	def addCityToList(self, iRank, pCity, iTotalCityValue):
		
		if (iRank > 4):
			
			return
			
		else:
			pTempCity = self.pCityPointers[iRank]
			
			# Verify a city actually exists at this rank
			if (pTempCity):
				
				iTempCityValue = self.iCityValues[iRank]
				
				self.addCityToList(iRank+1, pTempCity, iTempCityValue)
				
				self.pCityPointers[iRank] = pCity
				self.iCityValues[iRank] = iTotalCityValue
				
			else:
				self.pCityPointers[iRank] = pCity
				self.iCityValues[iRank] = iTotalCityValue
				
				return
			
	def determineCityData(self):
		
		self.iNumCities = 0
		
		for iRankLoop in range(5):
			
			pCity = self.pCityPointers[iRankLoop]
			
			# If this city exists and has data we can use
			if (pCity):
				
				pPlayer = gc.getPlayer(pCity.getOwner())
				
				iTurnYear = CyGame().getTurnYear(pCity.getGameTurnFounded())
				
				if (iTurnYear < 0):
					szTurnFounded = localText.getText("TXT_KEY_TIME_BC", (-iTurnYear,))#"%d %s" %(-iTurnYear, self.TEXT_BC)
				else:
					szTurnFounded = localText.getText("TXT_KEY_TIME_AD", (iTurnYear,))#"%d %s" %(iTurnYear, self.TEXT_AD)
				
				self.szCityNames[iRankLoop] = pCity.getName().upper()
				self.iCitySizes[iRankLoop] = pCity.getPopulation()
				self.szCityDescs[iRankLoop] = ("%s, %s" %(pPlayer.getCivilizationAdjective(0), localText.getText("TXT_KEY_MISC_FOUNDED_IN", (szTurnFounded,))))
				self.aaCitiesXY[iRankLoop] = [pCity.getX(), pCity.getY()]
				
				self.iNumCities += 1
			else:
				
				self.szCityNames[iRankLoop] = ""
				self.iCitySizes[iRankLoop] = -1
				self.szCityDescs[iRankLoop] = ""
				self.aaCitiesXY[iRankLoop] = [-1, -1]
				
		return
		
#############################################################################################################
################################################### WONDERS #################################################
#############################################################################################################
		
	def drawWondersTab(self):
		
		screen = self.getScreen()
		
		self.szRightPaneWidget = self.getNextWidgetName()
		screen.addPanel( self.szRightPaneWidget, "", "", true, true,
			self.X_RIGHT_PANE, self.Y_RIGHT_PANE, self.W_RIGHT_PANE, self.H_RIGHT_PANE, PanelStyles.PANEL_STYLE_MAIN )#PanelStyles.PANEL_STYLE_DAWNTOP )
		
		self.drawWondersDropdownBox()
		self.calculateWondersList()
		self.drawWondersList()
		
	def drawWondersDropdownBox(self):
		"Draws the Wonders Dropdown Box"
		
		screen = self.getScreen()
		
		######################### Dropdown Box Widget #########################################
		
		self.szWondersDropdownWidget = self.getNextWidgetName()
		
		screen.addDropDownBoxGFC(self.szWondersDropdownWidget,
		    self.X_DROPDOWN, self.Y_DROPDOWN, self.W_DROPDOWN, WidgetTypes.WIDGET_GENERAL, -1, -1, FontTypes.GAME_FONT)
		
		if (self.szWonderDisplayMode == "World Wonders"):
			bDefault = true
		else:
			bDefault = false
		screen.addPullDownString(self.szWondersDropdownWidget, localText.getText("TXT_KEY_TOP_CITIES_SCREEN_WORLD_WONDERS", ()), 0, 0, bDefault )

		if (self.szWonderDisplayMode == "National Wonders"):
			bDefault = true
		else:
			bDefault = false
		screen.addPullDownString(self.szWondersDropdownWidget, localText.getText("TXT_KEY_TOP_CITIES_SCREEN_NATIONAL_WONDERS", ()), 1, 1, bDefault )

		if (self.szWonderDisplayMode == "Projects"):
			bDefault = true
		else:
			bDefault = false
		screen.addPullDownString(self.szWondersDropdownWidget, localText.getText("TXT_KEY_PEDIA_CATEGORY_PROJECT", ()), 2, 2, bDefault )
		
		return
		
	def determineListBoxContents(self):
		
		screen = self.getScreen()
		
		# Fill wonders listbox
	
		iNumWondersBeingBuilt = len(self.aaWondersBeingBuilt)
		
		szWonderName = ""
		self.aiWonderListBoxIDs = []
		self.aiTurnYearBuilt = []
		self.aiWonderBuiltBy = []
		
		if (self.szWonderDisplayMode == "Projects"):
			
	############### Create ListBox for Projects ###############
			
			for iWonderLoop in range(iNumWondersBeingBuilt):
				
				iProjectType = self.aaWondersBeingBuilt[iWonderLoop][0]
				pProjectInfo = gc.getProjectInfo(iProjectType)
				szProjectName = pProjectInfo.getDescription()
				
				self.aiWonderListBoxIDs.append(iProjectType)
				self.aiTurnYearBuilt.append(-6666)
				szWonderBuiltBy = self.aaWondersBeingBuilt[iWonderLoop][1]
				self.aiWonderBuiltBy.append(szWonderBuiltBy)
				
				screen.appendListBoxString( self.szWondersListBox, szProjectName + " (" + szWonderBuiltBy + ")", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
		
			for iWonderLoop in range(self.iNumWonders):
				
				iProjectType = self.aaWondersBuilt[iWonderLoop][1]
				pProjectInfo = gc.getProjectInfo(iProjectType)
				szProjectName = pProjectInfo.getDescription()
				
				self.aiWonderListBoxIDs.append(iProjectType)
				self.aiTurnYearBuilt.append(-9999)
				szWonderBuiltBy = self.aaWondersBuilt[iWonderLoop][2]
				self.aiWonderBuiltBy.append(szWonderBuiltBy)
				
				screen.appendListBoxString( self.szWondersListBox, szProjectName + " (" + szWonderBuiltBy + ")", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
		
		else:
			
	############### Create ListBox for Wonders ###############
			
			for iWonderLoop in range(iNumWondersBeingBuilt):
				
				iWonderType = self.aaWondersBeingBuilt[iWonderLoop][0]
				pWonderInfo = gc.getBuildingInfo(iWonderType)
				szWonderName = pWonderInfo.getDescription()
				
				self.aiWonderListBoxIDs.append(iWonderType)
				self.aiTurnYearBuilt.append(-9999)
				szWonderBuiltBy = self.aaWondersBeingBuilt[iWonderLoop][1]
				self.aiWonderBuiltBy.append(szWonderBuiltBy)
				
				screen.appendListBoxString( self.szWondersListBox, szWonderName + " (" + szWonderBuiltBy + ")", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
		
			for iWonderLoop in range(self.iNumWonders):
				
				iWonderType = self.aaWondersBuilt[iWonderLoop][1]
				pWonderInfo = gc.getBuildingInfo(iWonderType)
				szWonderName = pWonderInfo.getDescription()
				
				self.aiWonderListBoxIDs.append(iWonderType)
				self.aiTurnYearBuilt.append(self.aaWondersBuilt[iWonderLoop][0])
				szWonderBuiltBy = self.aaWondersBuilt[iWonderLoop][2]
				self.aiWonderBuiltBy.append(szWonderBuiltBy)
				
				screen.appendListBoxString( self.szWondersListBox, szWonderName + " (" + szWonderBuiltBy + ")", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
		
	def drawWondersList(self):
		
		screen = self.getScreen()
		
		if (self.iNumWondersPermanentWidgets == 0):
			
			# Wonders List ListBox
			self.szWondersListBox = self.getNextWidgetName()
			screen.addListBoxGFC(self.szWondersListBox, "",
			    self.X_WONDER_LIST, self.Y_WONDER_LIST, self.W_WONDER_LIST, self.H_WONDER_LIST, TableStyles.TABLE_STYLE_STANDARD )
			
			self.determineListBoxContents()
			
			self.iNumWondersPermanentWidgets = self.nWidgetCount
			
		# Stats Panel
		panelName = self.getNextWidgetName()
		screen.addPanel( panelName, "", "", true, true,
				 self.X_STATS_PANE, self.Y_STATS_PANE, self.W_STATS_PANE, self.H_STATS_PANE, PanelStyles.PANEL_STYLE_IN )
		
############################################### DISPLAY SINGLE WONDER ###############################################
		
		# Set default wonder if any exist in this list
		if (len(self.aiWonderListBoxIDs) > 0 and self.iWonderID == -1):
			self.iWonderID = self.aiWonderListBoxIDs[0]
		
		# Only display/do the following if a wonder is actively being displayed
		if (self.iWonderID > -1):
			
############################################### DISPLAY PROJECT MODE ###############################################
			
			if (self.szWonderDisplayMode == "Projects"):
				
				pProjectInfo = gc.getProjectInfo(self.iWonderID)
				
				# Stats panel (cont'd) - Name
				szProjectDesc = u"<font=3b>" + pProjectInfo.getDescription().upper() + u"</font>"
				szStatsText = szProjectDesc + "\n\n"
				
				# Say whether this project is built yet or not
				
				iTurnYear = self.aiTurnYearBuilt[self.iActiveWonderCounter]
				if (iTurnYear == -6666):	# -6666 used for wonders in progress
					szTempText = localText.getText("TXT_KEY_BEING_BUILT", ())
					
				else:
					szTempText = localText.getText("TXT_KEY_INFO_SCREEN_BUILT", ())
					
				szWonderDesc = "%s, %s" %(self.aiWonderBuiltBy[self.iActiveWonderCounter], szTempText)
				szStatsText += szWonderDesc + "\n\n"
				
				if (pProjectInfo.getProductionCost() > 0):
					szCost = localText.getText("TXT_KEY_PEDIA_COST", (gc.getActivePlayer().getProjectProductionNeeded(self.iWonderID),))
					szStatsText += szCost.upper() + (u"%c" % gc.getYieldInfo(YieldTypes.YIELD_PRODUCTION).getChar()) + "\n"
				
				if (isWorldProject(self.iWonderID)):
					iMaxInstances = gc.getProjectInfo(self.iWonderID).getMaxGlobalInstances()
					szProjectType = localText.getText("TXT_KEY_PEDIA_WORLD_PROJECT", ())
					if (iMaxInstances > 1):
						szProjectType += " " + localText.getText("TXT_KEY_PEDIA_WONDER_INSTANCES", (iMaxInstances,))
					szStatsText += szProjectType.upper() + "\n"

				if (isTeamProject(self.iWonderID)):
					iMaxInstances = gc.getProjectInfo(self.iWonderID).getMaxTeamInstances()
					szProjectType = localText.getText("TXT_KEY_PEDIA_TEAM_PROJECT", ())
					if (iMaxInstances > 1):
						szProjectType += " " + localText.getText("TXT_KEY_PEDIA_WONDER_INSTANCES", (iMaxInstances,))
					szStatsText += szProjectType.upper()
				
				screen.addMultilineText(self.getNextWidgetName(), szStatsText, self.X_STATS_PANE + 5, self.Y_STATS_PANE + 15, self.W_STATS_PANE - 10, self.H_STATS_PANE - 20, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_CENTER_JUSTIFY)
				
				# Add Graphic
				iIconX = self.X_PROJECT_ICON - self.W_PROJECT_ICON / 2
				iIconY = self.Y_PROJECT_ICON - self.W_PROJECT_ICON / 2
				
				screen.addDDSGFC(self.getNextWidgetName(), gc.getProjectInfo(self.iWonderID).getButton(),
						 iIconX, iIconY, self.W_PROJECT_ICON, self.W_PROJECT_ICON, WidgetTypes.WIDGET_GENERAL, -1, -1 )
				
				# Special Abilities ListBox
				
				szSpecialTitle = u"<font=3b>" + localText.getText("TXT_KEY_PEDIA_SPECIAL_ABILITIES", ()) + u"</font>"
				self.szSpecialTitleWidget = self.getNextWidgetName()
				screen.setText(self.szSpecialTitleWidget, "", szSpecialTitle, CvUtil.FONT_LEFT_JUSTIFY, self.X_SPECIAL_TITLE, self.Y_SPECIAL_TITLE, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
				
				panelName = self.getNextWidgetName()
				screen.addPanel( panelName, "", "", true, true,
						 self.X_SPECIAL_PANE, self.Y_SPECIAL_PANE, self.W_SPECIAL_PANE, self.H_SPECIAL_PANE, PanelStyles.PANEL_STYLE_IN)
				
				listName = self.getNextWidgetName()
				screen.attachListBoxGFC( panelName, listName, "", TableStyles.TABLE_STYLE_EMPTY )
				screen.enableSelect(listName, False)
				
				szSpecialText = CyGameTextMgr().getProjectHelp(self.iWonderID, True, None)
				splitText = string.split( szSpecialText, "\n" )
				for special in splitText:
					if len( special ) != 0:
						screen.appendListBoxString( listName, special, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
						
			else:
					
	############################################### DISPLAY WONDER MODE ###############################################
				
				pWonderInfo = gc.getBuildingInfo(self.iWonderID)
				
				# Stats panel (cont'd) - Name
				szWonderDesc = u"<font=3b>" + gc.getBuildingInfo(self.iWonderID).getDescription().upper() + u"</font>"
				szStatsText = szWonderDesc + "\n\n"
				
				# Wonder built-in year
				iTurnYear = self.aiTurnYearBuilt[self.iActiveWonderCounter]#self.aaWondersBuilt[self.iActiveWonderCounter][0]#.append([0,iProjectLoop,""]
				
				szDateBuilt = ""
				
				if (iTurnYear != -9999):	# -9999 used for wonders in progress
					if (iTurnYear < 0):
						szTurnFounded = localText.getText("TXT_KEY_TIME_BC", (-iTurnYear,))
					else:
						szTurnFounded = localText.getText("TXT_KEY_TIME_AD", (iTurnYear,))
				
					szDateBuilt = (", %s" %(szTurnFounded))
					
				else:
					szDateBuilt = (", %s" %(localText.getText("TXT_KEY_BEING_BUILT", ())))
					
				szWonderDesc = "%s%s" %(self.aiWonderBuiltBy[self.iActiveWonderCounter], szDateBuilt)
				szStatsText += szWonderDesc + "\n\n"
				
				# Building attributes
				
				if (pWonderInfo.getProductionCost() > 0):
					szCost = localText.getText("TXT_KEY_PEDIA_COST", (gc.getActivePlayer().getBuildingProductionNeeded(self.iWonderID),))
					szStatsText += szCost.upper() + (u"%c" % gc.getYieldInfo(YieldTypes.YIELD_PRODUCTION).getChar()) + "\n"
				
				for k in range(CommerceTypes.NUM_COMMERCE_TYPES):
					if (pWonderInfo.getObsoleteSafeCommerceChange(k) != 0):
						if (pWonderInfo.getObsoleteSafeCommerceChange(k) > 0):
							szSign = "+"
						else:
							szSign = ""
							
						szCommerce = gc.getCommerceInfo(k).getDescription() + ": "
						
						szText1 = szCommerce.upper() + szSign + str(pWonderInfo.getObsoleteSafeCommerceChange(k))
						szText2 = szText1 + (u"%c" % (gc.getCommerceInfo(k).getChar()))
						szStatsText += szText2 + "\n"

				if (pWonderInfo.getHappiness() > 0):
					szText = localText.getText("TXT_KEY_PEDIA_HAPPY", (pWonderInfo.getHappiness(),))
					szStatsText += szText + (u"%c" % CyGame().getSymbolID(FontSymbols.HAPPY_CHAR)) + "\n"
					
				elif (pWonderInfo.getHappiness() < 0):
					szText = localText.getText("TXT_KEY_PEDIA_UNHAPPY", (-pWonderInfo.getHappiness(),))
					szStatsText += szText + (u"%c" % CyGame().getSymbolID(FontSymbols.UNHAPPY_CHAR)) + "\n"

				if (pWonderInfo.getHealth() > 0):
					szText = localText.getText("TXT_KEY_PEDIA_HEALTHY", (pWonderInfo.getHealth(),))
					szStatsText += szText + (u"%c" % CyGame().getSymbolID(FontSymbols.HEALTHY_CHAR)) + "\n"
					
				elif (pWonderInfo.getHealth() < 0):
					szText = localText.getText("TXT_KEY_PEDIA_UNHEALTHY", (-pWonderInfo.getHealth(),))
					szStatsText += szText + (u"%c" % CyGame().getSymbolID(FontSymbols.UNHEALTHY_CHAR)) + "\n"
				
				if (pWonderInfo.getGreatPeopleRateChange() != 0):
					szText = localText.getText("TXT_KEY_PEDIA_GREAT_PEOPLE", (pWonderInfo.getGreatPeopleRateChange(),))
					szStatsText += szText + (u"%c" % CyGame().getSymbolID(FontSymbols.GREAT_PEOPLE_CHAR)) + "\n"
				
				screen.addMultilineText(self.getNextWidgetName(), szStatsText, self.X_STATS_PANE + 5, self.Y_STATS_PANE + 15, self.W_STATS_PANE - 10, self.H_STATS_PANE - 20, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_CENTER_JUSTIFY)
				
				# Add Graphic
				screen.addBuildingGraphicGFC(self.getNextWidgetName(), self.iWonderID,
				    self.X_WONDER_GRAPHIC, self.Y_WONDER_GRAPHIC, self.W_WONDER_GRAPHIC, self.H_WONDER_GRAPHIC,
				    WidgetTypes.WIDGET_GENERAL, -1, -1, self.X_ROTATION_WONDER_ANIMATION, self.Z_ROTATION_WONDER_ANIMATION, self.SCALE_ANIMATION, True)
				
				# Special Abilities ListBox
				
				szSpecialTitle = u"<font=3b>" + localText.getText("TXT_KEY_PEDIA_SPECIAL_ABILITIES", ()) + u"</font>"
				self.szSpecialTitleWidget = self.getNextWidgetName()
				screen.setText(self.szSpecialTitleWidget, "", szSpecialTitle, CvUtil.FONT_LEFT_JUSTIFY, self.X_SPECIAL_TITLE, self.Y_SPECIAL_TITLE, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
				
				panelName = self.getNextWidgetName()
				screen.addPanel( panelName, "", "", true, true,#localText.getText("TXT_KEY_PEDIA_SPECIAL_ABILITIES", ())
						 self.X_SPECIAL_PANE, self.Y_SPECIAL_PANE, self.W_SPECIAL_PANE, self.H_SPECIAL_PANE, PanelStyles.PANEL_STYLE_IN)
				
				listName = self.getNextWidgetName()
				screen.attachListBoxGFC( panelName, listName, "", TableStyles.TABLE_STYLE_EMPTY )
				screen.enableSelect(listName, False)
				
				szSpecialText = CyGameTextMgr().getBuildingHelp(self.iWonderID, True, False, False, None)
				splitText = string.split( szSpecialText, "\n" )
				for special in splitText:
					if len( special ) != 0:
						screen.appendListBoxString( listName, special, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
						
	def calculateWondersList(self):
		
		self.aaWondersBeingBuilt = []
		self.aaWondersBuilt = []
		self.iNumWonders = 0
		
		self.pActivePlayer = gc.getPlayer(CyGame().getActivePlayer())
		
		# Loop through players to determine Wonders
		for iPlayerLoop in range(gc.getMAX_PLAYERS()):
			
			pPlayer = gc.getPlayer(iPlayerLoop)
			iPlayerTeam = pPlayer.getTeam()
			
			# No barbs and only display national wonders for the active player's team
 			if (pPlayer and not pPlayer.isBarbarian() and ((self.szWonderDisplayMode != "National Wonders") or (iPlayerTeam == gc.getTeam(gc.getPlayer(self.iActivePlayer).getTeam()).getID()))):

				# Loop through this player's cities and determine if they have any wonders to display
				apCityList = PyPlayer(iPlayerLoop).getCityList()
				for pCity in apCityList:
					
					# Loop through projects to find any under construction
					if (self.szWonderDisplayMode == "Projects"):
						for iProjectLoop in range(gc.getNumProjectInfos()):
							
							iProjectProd = pCity.getProductionProject()
							pProject = gc.getProjectInfo(iProjectLoop)
							
							# Project is being constructed
							if (iProjectProd == iProjectLoop):
								
								# Project Mode
								if (iPlayerTeam == gc.getTeam(gc.getPlayer(self.iActivePlayer).getTeam()).getID()):
									
									self.aaWondersBeingBuilt.append([iProjectProd, pPlayer.getCivilizationShortDescription(0)])
						
					# Loop through buildings
					else:
						
						for iBuildingLoop in range(gc.getNumBuildingInfos()):
							
							iBuildingProd = pCity.getProductionBuilding()
							
							pBuilding = gc.getBuildingInfo(iBuildingLoop)
							
							# World Wonder Mode
							if (self.szWonderDisplayMode == "World Wonders" and isWorldWonderClass(gc.getBuildingInfo(iBuildingLoop).getBuildingClassType())):
								
								# Is this city building a wonder?
								if (iBuildingProd == iBuildingLoop):
									
									# Only show our wonders under construction
									if (iPlayerTeam == gc.getTeam(gc.getPlayer(self.iActivePlayer).getTeam()).getID()):
										
										self.aaWondersBeingBuilt.append([iBuildingProd, pPlayer.getCivilizationShortDescription(0)])
									
								if (pCity.hasBuilding(iBuildingLoop)):
									
									self.aaWondersBuilt.append([pCity.getBuildingOriginalTime(iBuildingLoop),iBuildingLoop,pPlayer.getCivilizationShortDescription(0)])
	#								print("Adding World wonder to list: %s, %d, %s" %(pCity.getBuildingOriginalTime(iBuildingLoop),iBuildingLoop,pPlayer.getCivilizationAdjective(0)))
									self.iNumWonders += 1
							
							# National/Team Wonder Mode
							elif (self.szWonderDisplayMode == "National Wonders" and (isNationalWonderClass(gc.getBuildingInfo(iBuildingLoop).getBuildingClassType()) or isTeamWonderClass(gc.getBuildingInfo(iBuildingLoop).getBuildingClassType()))):
								
								# Is this city building a wonder?
								if (iBuildingProd == iBuildingLoop):
									
									# Only show our wonders under construction
									if (iPlayerTeam == gc.getTeam(gc.getPlayer(self.iActivePlayer).getTeam()).getID()):
										
										self.aaWondersBeingBuilt.append([iBuildingProd, pPlayer.getCivilizationShortDescription(0)])
									
								if (pCity.hasBuilding(iBuildingLoop)):
									
	#								print("Adding National wonder to list: %s, %d, %s" %(pCity.getBuildingOriginalTime(iBuildingLoop),iBuildingLoop,pPlayer.getCivilizationAdjective(0)))
									self.aaWondersBuilt.append([pCity.getBuildingOriginalTime(iBuildingLoop),iBuildingLoop,pPlayer.getCivilizationShortDescription(0)])
									self.iNumWonders += 1
		
		# This array used to store which players have already used up a team's slot so team projects don't get added to list more than once
		aiTeamsUsed = []
		
		# Project Mode
		if (self.szWonderDisplayMode == "Projects"):
			
			# Loop through players to determine Projects
			for iPlayerLoop in range(gc.getMAX_PLAYERS()):
				
				pPlayer = gc.getPlayer(iPlayerLoop)
				iTeamLoop = pPlayer.getTeam()
				
				# Block duplicates
				if (iTeamLoop not in aiTeamsUsed):
					
					aiTeamsUsed.append(iTeamLoop)
					pTeam = gc.getTeam(iTeamLoop)
					
					# No barbs and only display national wonders for the active player's team
					if (pTeam.isAlive() and not pTeam.isBarbarian()):
						
						# Loop through projects
						for iProjectLoop in range(gc.getNumProjectInfos()):
							
							for iI in range(pTeam.getProjectCount(iProjectLoop)):
								
								self.aaWondersBuilt.append([-9999,iProjectLoop,gc.getPlayer(iPlayerLoop).getCivilizationShortDescription(0)])
								self.iNumWonders += 1
		
		# Sort wonders in order of date built
		self.aaWondersBuilt.sort()
		self.aaWondersBuilt.reverse()
		
#		print("List of wonders/projects Built:")
#		print(self.aaWondersBuilt)
										   
#############################################################################################################
################################################## STATISTICS ###############################################
#############################################################################################################
		
	def drawStatsTab(self):
		
		screen = self.getScreen()
		
		iNumUnits = gc.getNumUnitInfos()
		iNumBuildings = gc.getNumBuildingInfos()
		
		self.iNumStatsChartCols = 7
		self.iNumStatsChartRows = max(iNumUnits, iNumBuildings)
		
################################################### CALCULATE STATS ###################################################
		
		iMinutesPlayed = CyGame().getMinutesPlayed()
		iHoursPlayed = iMinutesPlayed / 60
		iMinutesPlayed = iMinutesPlayed - (iHoursPlayed * 60)
		
		szMinutesString = str(iMinutesPlayed)
		if (iMinutesPlayed < 10):
			szMinutesString = "0" + szMinutesString
		szHoursString = str(iHoursPlayed)
		if (iHoursPlayed < 10):
			szHoursString = "0" + szHoursString
		
		szTimeString = szHoursString + ":" + szMinutesString
		
		iNumCitiesBuilt = CyStatistics().getPlayerNumCitiesBuilt(self.iActivePlayer)
		
		iNumCitiesRazed = CyStatistics().getPlayerNumCitiesRazed(self.iActivePlayer)
		
		iNumReligionsFounded = 0
		for iReligionLoop in range(gc.getNumReligionInfos()):
			if (CyStatistics().getPlayerReligionFounded(self.iActivePlayer, iReligionLoop)):
				iNumReligionsFounded += 1
		
		aiUnitsBuilt = []
		for iUnitLoop in range(iNumUnits):
			aiUnitsBuilt.append(CyStatistics().getPlayerNumUnitsBuilt(self.iActivePlayer, iUnitLoop))
		
		aiUnitsKilled = []
		for iUnitLoop in range(iNumUnits):
			aiUnitsKilled.append(CyStatistics().getPlayerNumUnitsKilled(self.iActivePlayer, iUnitLoop))
		
		aiUnitsLost = []
		for iUnitLoop in range(iNumUnits):
			aiUnitsLost.append(CyStatistics().getPlayerNumUnitsLost(self.iActivePlayer, iUnitLoop))
		
		aiBuildingsBuilt = []
		for iBuildingLoop in range(iNumBuildings):
			aiBuildingsBuilt.append(CyStatistics().getPlayerNumBuildingsBuilt(self.iActivePlayer, iBuildingLoop))
		
		aiUnitsCurrent = []
		for iUnitLoop in range(iNumUnits):
			aiUnitsCurrent.append(0)
		
		apUnitList = PyPlayer(self.iActivePlayer).getUnitList()
		for pUnit in apUnitList:
			iType = pUnit.getUnitType()
			aiUnitsCurrent[iType] += 1
			
################################################### TOP PANEL ###################################################
		
		# Add Panel
		szTopPanelWidget = self.getNextWidgetName()
		screen.addPanel( szTopPanelWidget, u"", u"", True, False, self.X_STATS_TOP_PANEL, self.Y_STATS_TOP_PANEL, self.W_STATS_TOP_PANEL, self.H_STATS_TOP_PANEL,
				 PanelStyles.PANEL_STYLE_DAWNTOP )
		
		# Leaderhead graphic
		player = gc.getPlayer(gc.getGame().getActivePlayer())
		szLeaderWidget = self.getNextWidgetName()
		screen.addLeaderheadGFC(szLeaderWidget, player.getLeaderType(), AttitudeTypes.ATTITUDE_PLEASED,
			self.X_LEADER_ICON, self.Y_LEADER_ICON, self.W_LEADER_ICON, self.H_LEADER_ICON, WidgetTypes.WIDGET_GENERAL, -1, -1)
		
		# Leader Name
		self.szLeaderNameWidget = self.getNextWidgetName()
		szText = u"<font=4b>" + gc.getPlayer(self.iActivePlayer).getName() + u"</font>"
		screen.setText(self.szLeaderNameWidget, "", szText, CvUtil.FONT_LEFT_JUSTIFY,
			       self.X_LEADER_NAME, self.Y_LEADER_NAME, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
		
		# Create Table
		szTopChart = self.getNextWidgetName()
		screen.addTableControlGFC(szTopChart, self.iNumTopChartCols, self.X_STATS_TOP_CHART, self.Y_STATS_TOP_CHART, self.W_STATS_TOP_CHART, self.H_STATS_TOP_CHART,
					  False, True, 32,32, TableStyles.TABLE_STYLE_STANDARD)
		
		# Add Columns
		screen.setTableColumnHeader(szTopChart, 0, "", self.STATS_TOP_CHART_W_COL_0)
		screen.setTableColumnHeader(szTopChart, 1, "", self.STATS_TOP_CHART_W_COL_1)
		
		# Add Rows
		for i in range(self.iNumTopChartRows - 1):
			screen.appendTableRow(szTopChart)
		iNumRows = screen.getTableNumRows(szTopChart)
		
		# Graph itself
		iRow = 0
		iCol = 0
		screen.setTableText(szTopChart, iCol, iRow, self.TEXT_TIME_PLAYED, "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		iCol = 1
		screen.setTableText(szTopChart, iCol, iRow, szTimeString, "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		
		iRow = 1
		iCol = 0
		screen.setTableText(szTopChart, iCol, iRow, self.TEXT_CITIES_BUILT, "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		iCol = 1
		screen.setTableText(szTopChart, iCol, iRow, str(iNumCitiesBuilt), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)

		iRow = 2
		iCol = 0
		screen.setTableText(szTopChart, iCol, iRow, self.TEXT_CITIES_RAZED, "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		iCol = 1
		screen.setTableText(szTopChart, iCol, iRow, str(iNumCitiesRazed), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)

		iRow = 3
		iCol = 0
		screen.setTableText(szTopChart, iCol, iRow, self.TEXT_NUM_RELIGIONS_FOUNDED, "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		iCol = 1
		screen.setTableText(szTopChart, iCol, iRow, str(iNumReligionsFounded), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		
################################################### BOTTOM PANEL ###################################################
		
		# Create Table
		szTable = self.getNextWidgetName()
		screen.addTableControlGFC(szTable, self.iNumStatsChartCols, self.X_STATS_BOTTOM_CHART, self.Y_STATS_BOTTOM_CHART, self.W_STATS_BOTTOM_CHART, self.H_STATS_BOTTOM_CHART,
					  True, True, 32,32, TableStyles.TABLE_STYLE_STANDARD)
		screen.enableSort(szTable)
		
		# Reducing the width a bit to leave room for the vertical scrollbar, preventing a horizontal scrollbar from also being created
		iChartWidth = self.W_STATS_BOTTOM_CHART - 12
		
		# Add Columns
		iColWidth = int((iChartWidth / 12 * 3))
		screen.setTableColumnHeader(szTable, 0, self.TEXT_UNITS, iColWidth)
		iColWidth = int((iChartWidth / 12 * 1))
		screen.setTableColumnHeader(szTable, 1, self.TEXT_CURRENT, iColWidth)
		iColWidth = int((iChartWidth / 12 * 1))
		screen.setTableColumnHeader(szTable, 2, self.TEXT_BUILT, iColWidth)
		iColWidth = int((iChartWidth / 12 * 1))
		screen.setTableColumnHeader(szTable, 3, self.TEXT_KILLED, iColWidth)
		iColWidth = int((iChartWidth / 12 * 1))
		screen.setTableColumnHeader(szTable, 4, self.TEXT_LOST, iColWidth)
		iColWidth = int((iChartWidth / 12 * 4))
		screen.setTableColumnHeader(szTable, 5, self.TEXT_BUILDINGS, iColWidth)
		iColWidth = int((iChartWidth / 12 * 1))
		screen.setTableColumnHeader(szTable, 6, self.TEXT_BUILT, iColWidth)
		
		# Add Rows
		for i in range(self.iNumStatsChartRows - 1):
			screen.appendTableRow(szTable)
		iNumRows = screen.getTableNumRows(szTable)
		
		# Add Units to table
		for iUnitLoop in range(iNumUnits):
			iRow = iUnitLoop
			
			iCol = 0
			szUnitName = gc.getUnitInfo(iUnitLoop).getDescription()
			screen.setTableText(szTable, iCol, iRow, szUnitName, "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
			
			iCol = 1
			iNumUnitsCurrent = aiUnitsCurrent[iUnitLoop]
			screen.setTableInt(szTable, iCol, iRow, str(iNumUnitsCurrent), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
			
			iCol = 2
			iNumUnitsBuilt = aiUnitsBuilt[iUnitLoop]
			screen.setTableInt(szTable, iCol, iRow, str(iNumUnitsBuilt), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
			
			iCol = 3
			iNumUnitsKilled = aiUnitsKilled[iUnitLoop]
			screen.setTableInt(szTable, iCol, iRow, str(iNumUnitsKilled), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
			
			iCol = 4
			iNumUnitsLost = aiUnitsLost[iUnitLoop]
			screen.setTableInt(szTable, iCol, iRow, str(iNumUnitsLost), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		
		# Add Buildings to table
		for iBuildingLoop in range(iNumBuildings):
			iRow = iBuildingLoop
			
			iCol = 5
			szBuildingName = gc.getBuildingInfo(iBuildingLoop).getDescription()
			screen.setTableText(szTable, iCol, iRow, szBuildingName, "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
			iCol = 6
			iNumBuildingsBuilt = aiBuildingsBuilt[iBuildingLoop]
			screen.setTableInt(szTable, iCol, iRow, str(iNumBuildingsBuilt), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		
#############################################################################################################
##################################################### OTHER #################################################
#############################################################################################################

	# returns a unique ID for a widget in this screen
	def getNextWidgetName(self):
		szName = self.WIDGET_ID + str(self.nWidgetCount)
		self.nWidgetCount += 1
		return szName
		
	def deleteAllWidgets(self, iNumPermanentWidgets = 0):
		screen = self.getScreen()
		i = self.nWidgetCount - 1
		while (i >= iNumPermanentWidgets):
			self.nWidgetCount = i
			screen.deleteWidget(self.getNextWidgetName())
			i -= 1
		
		self.nWidgetCount = iNumPermanentWidgets
		self.yMessage = 5
	
	# handle the input for this screen...
	def handleInput (self, inputClass):
		
		screen = self.getScreen()
			
		szWidgetName = inputClass.getFunctionName() + str(inputClass.getID())
		
		# Exit
		if ( szWidgetName == self.szExitButtonName and inputClass.getNotifyCode() == NotifyCode.NOTIFY_CLICKED \
				or inputClass.getData() == int(InputTypes.KB_RETURN) ):
			# Reset Wonders so nothing lingers next time the screen is opened
			self.resetWonders()
			screen.hideScreen()
		
#		print("szWidgetName: %s" %(szWidgetName))
#		print("Notify Code: %s" %(inputClass.getNotifyCode()))
		
		# Dropdown Box/ ListBox
		if (inputClass.getNotifyCode() == NotifyCode.NOTIFY_LISTBOX_ITEM_SELECTED):
			
			# Debug dropdown
			if (inputClass.getFunctionName() == self.DEBUG_DROPDOWN_ID):
				iIndex = screen.getSelectedPullDownID(self.DEBUG_DROPDOWN_ID)
				self.iActivePlayer = screen.getPullDownData(self.DEBUG_DROPDOWN_ID, iIndex)
				
				self.pActivePlayer = gc.getPlayer(self.iActivePlayer)
				self.iActiveTeam = self.pActivePlayer.getTeam()
				self.pActiveTeam = gc.getTeam(self.iActiveTeam)
				
				# Determine who this active player knows
				self.aiPlayersMet = []
				self.iNumPlayersMet = 0
				for iLoopPlayer in range(gc.getMAX_CIV_PLAYERS()):
					pLoopPlayer = gc.getPlayer(iLoopPlayer)
					iLoopPlayerTeam = pLoopPlayer.getTeam()
					if (self.pActiveTeam.isHasMet(iLoopPlayerTeam)):
						self.aiPlayersMet.append(iLoopPlayer)
						self.iNumPlayersMet += 1
				self.redrawContents()
			
			iSelected = inputClass.getData()
#			print("iSelected : %d" %(iSelected))
			
############################### WONDERS / TOP CITIES TAB ###############################
			
			if (self.iActiveTab == self.iTopCitiesID):
				
				# Wonder type dropdown box
				if (szWidgetName == self.szWondersDropdownWidget):
					
					# Reset wonders stuff so that when the type shown changes the old contents don't mess with things
					
					self.iNumWonders = 0
					self.iActiveWonderCounter = 0
					self.iWonderID = -1
					self.aaWondersBuilt = []
					
					self.aaWondersBeingBuilt = []
					
					if (iSelected == 0):
						self.szWonderDisplayMode = "World Wonders"
					
					elif (iSelected == 1):
						self.szWonderDisplayMode = "National Wonders"
					
					elif (iSelected == 2):
						self.szWonderDisplayMode = "Projects"
						
					self.reset()
					
					self.calculateWondersList()
					self.determineListBoxContents()
					
					# Change selected wonder to the one at the top of the new list
					if (self.iNumWonders > 0):
						self.iWonderID = self.aiWonderListBoxIDs[0]
						
					self.redrawContents()
					
				# Wonders ListBox
				elif (szWidgetName == self.szWondersListBox):
					
					self.reset()
					self.iWonderID = self.aiWonderListBoxIDs[iSelected]
					self.iActiveWonderCounter = iSelected
					self.deleteAllWidgets(self.iNumWondersPermanentWidgets)
					self.drawWondersList()
#					self.redrawContents()
					
############################### DEMOGRAPHICS TAB ###############################
			
			elif (self.iActiveTab == self.iDemographicsID):
				
				# Graph dropdown to select what values are being graphed
				if (szWidgetName == self.szGraphDropdownWidget):
					
					if (iSelected == 0):
						self.iDemoGraphTabID = self.iScoreTabID
					
					elif (iSelected == 1):
						self.iDemoGraphTabID = self.iEconomyTabID
					
					elif (iSelected == 2):
						self.iDemoGraphTabID = self.iIndustryTabID
					
					elif (iSelected == 3):
						self.iDemoGraphTabID = self.iAgricultureTabID
					
					elif (iSelected == 4):
						self.iDemoGraphTabID = self.iPowerTabID
					
					elif (iSelected == 5):
						self.iDemoGraphTabID = self.iCultureTabID
					
					self.fMaxDemoStatValue = 0.0
					self.updateGraph()
						
				# Dropdown to select players to show
				elif (szWidgetName == self.szCivsDropdownWidget):
					
					screen.deleteWidget(self.szGraphWidget)
					self.drawGraph()
					self.iShowingPlayer = self.aiDropdownPlayerIDs[iSelected]
					self.updateGraph()
					
		# Something Clicked
		elif (inputClass.getNotifyCode() == NotifyCode.NOTIFY_CLICKED):
			
			######## Screen 'Tabs' for Navigation ########
			
			if (szWidgetName == self.szDemographicsTabWidget):
				self.iActiveTab = self.iDemographicsID
				self.reset()
				self.redrawContents()
				
			elif (szWidgetName == self.szTopCitiesTabWidget):
				self.iActiveTab = self.iTopCitiesID
				self.reset()
				self.redrawContents()
				
			elif (szWidgetName == self.szStatsTabWidget):
				self.iActiveTab = self.iStatsID
				self.reset()
				self.redrawContents()
				
		return 0

	def update(self, fDelta):
		
		return
