## Sid Meier's Civilization 4
## Copyright Firaxis Games 2005
from CvPythonExtensions import *
import CvUtil
import ScreenInput
import PyHelpers
import time

PyPlayer = PyHelpers.PyPlayer

# globals
gc = CyGlobalContext()
ArtFileMgr = CyArtFileMgr()
localText = CyTranslator()

VICTORY_CONDITION_SCREEN = 0
GAME_SETTINGS_SCREEN = 1
SPACESHIP_SCREEN = 2
UN_RESOLUTION_SCREEN = 3

class CvVictoryScreen:
	"Keeps track of victory conditions"

	def __init__(self, screenId):
		self.screenId = screenId
		self.SCREEN_NAME = "VictoryScreen"
		self.DEBUG_DROPDOWN_ID =  "VictoryScreenDropdownWidget"
		self.INTERFACE_ART_INFO = "TECH_BG"
		self.EXIT_AREA = "EXIT"
		self.EXIT_ID = "VictoryScreenExit"
		self.BACKGROUND_ID = "VictoryScreenBackground"
		self.HEADER_ID = "VictoryScreenHeader"
		self.WIDGET_ID = "VictoryScreenWidget"
		self.VC_TAB_ID = "VictoryTabWidget"
		self.SETTINGS_TAB_ID = "SettingsTabWidget"
		self.SPACESHIP_TAB_ID = "SpaceshipTabWidget"
		self.UN_RESOLUTION_TAB_ID = "VotingTabWidget"
		self.SPACESHIP_WIDGET_ID = "SpaceshipWidget"

		self.Z_BACKGROUND = -6.1
		self.Z_CONTROLS = self.Z_BACKGROUND - 0.2
		self.DZ = -0.2

		self.X_SCREEN = 500
		self.Y_SCREEN = 396
		self.W_SCREEN = 1024
		self.H_SCREEN = 768
		self.Y_TITLE = 12
		
		self.X_EXIT = 994
		self.Y_EXIT = 726
		
		self.X_AREA = 10
		self.Y_AREA = 60
		self.W_AREA = 1010
		self.H_AREA = 650
		
		self.TABLE_WIDTH_0 = 430
		self.TABLE_WIDTH_1 = 180
		self.TABLE_WIDTH_2 = 100
		self.TABLE_WIDTH_3 = 180
		self.TABLE_WIDTH_4 = 100

		self.TABLE2_WIDTH_0 = 740
		self.TABLE2_WIDTH_1 = 265

		self.X_LINK = 100
		self.DX_LINK = 220
		self.Y_LINK = 726
		self.MARGIN = 20
		
		self.SETTINGS_PANEL_X1 = 50
		self.SETTINGS_PANEL_X2 = 355
		self.SETTINGS_PANEL_X3 = 660
		self.SETTINGS_PANEL_Y = 150
		self.SETTINGS_PANEL_WIDTH = 300
		self.SETTINGS_PANEL_HEIGHT = 500
								
		self.nWidgetCount = 0
		self.iActivePlayer = -1
		self.bSpaceshipTab = False
		self.bVoteTab = False

		# spaceship art
		self.X_BG = 150
		self.Y_BG = 150
		self.W_BG = 720
		self.H_BG = 480
		self.X_DETAIL = 452
		self.Y_DETAIL = 52
		self.W_DETAIL = 150
		self.H_DETAIL = 150
		self.locations = [[171, 88, 442, 350, "Art/Interface/Screens/SpaceShip/SS_Casing.dds", "Art/Interface/Screens/SpaceShip/SS_Casing_Callout.dds", "TXT_KEY_PROJECT_SS_CASING"], 
						  [28, 15, 320, 280, "Art/Interface/Screens/SpaceShip/SS_Thrusters.dds", "Art/Interface/Screens/SpaceShip/SS_Thrusters_Callout.dds", "TXT_KEY_PROJECT_SS_THRUSTERS"], 
						  [144, 106, 96, 124, "Art/Interface/Screens/SpaceShip/SS_Engine.dds", "Art/Interface/Screens/SpaceShip/SS_Engine_Callout.dds", "TXT_KEY_PROJECT_SS_ENGINE"],
						  [309, 232, 99, 107, "Art/Interface/Screens/SpaceShip/SS_DockingBay.dds", "Art/Interface/Screens/SpaceShip/SS_DockingBay_Callout.dds", "TXT_KEY_PROJECT_SS_DOCKING_BAY"], 
						  [453, 352, 122, 107, "Art/Interface/Screens/SpaceShip/SS_Cockpit.dds", "Art/Interface/Screens/SpaceShip/SS_Cockpit_Callout.dds", "TXT_KEY_PROJECT_SS_COCKPIT"],
						  [373, 284, 112, 114, "Art/Interface/Screens/SpaceShip/SS_LifeSupport.dds", "Art/Interface/Screens/SpaceShip/SS_LifeSupport_Callout.dds", "TXT_KEY_PROJECT_SS_LIFE_SUPPORT"],
						  [221, 167, 110, 137, "Art/Interface/Screens/SpaceShip/SS_StasisChamber.dds", "Art/Interface/Screens/SpaceShip/SS_Stasis_Callout.dds", "TXT_KEY_PROJECT_SS_STASIS_CHAMBER"] 
						  ]
						
		self.iScreen = VICTORY_CONDITION_SCREEN
						
	def getScreen(self):
		return CyGInterfaceScreen(self.SCREEN_NAME, self.screenId)

	def hideScreen(self):
		screen = self.getScreen()
		screen.hideScreen()
										
	def interfaceScreen(self):

		# Create a new screen
		screen = self.getScreen()
		if screen.isActive():
			return
		screen.setRenderInterfaceOnly(True);
		screen.showScreen(PopupStates.POPUPSTATE_IMMEDIATE, False)

		self.iActivePlayer = CyGame().getActivePlayer()
		if self.iScreen == -1:
			self.iScreen = VICTORY_CONDITION_SCREEN

		# Set the background widget and exit button
		screen.addDDSGFC(self.BACKGROUND_ID, ArtFileMgr.getInterfaceArtInfo("MAINMENU_SLIDESHOW_LOAD").getPath(), 0, 0, self.W_SCREEN, self.H_SCREEN, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.addPanel( "TechTopPanel", u"", u"", True, False, 0, 0, self.W_SCREEN, 55, PanelStyles.PANEL_STYLE_TOPBAR )
		screen.addPanel( "TechBottomPanel", u"", u"", True, False, 0, 713, self.W_SCREEN, 55, PanelStyles.PANEL_STYLE_BOTTOMBAR )
		screen.showWindowBackground( False )
		screen.setDimensions(screen.centerX(0), screen.centerY(0), self.W_SCREEN, self.H_SCREEN)
		screen.setText(self.EXIT_ID, "Background", u"<font=4>" + localText.getText("TXT_KEY_PEDIA_SCREEN_EXIT", ()).upper() + "</font>", CvUtil.FONT_RIGHT_JUSTIFY, self.X_EXIT, self.Y_EXIT, self.Z_CONTROLS, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_CLOSE_SCREEN, -1, -1 )

		# Header...
		screen.setLabel(self.HEADER_ID, "Background", u"<font=4b>" + localText.getText("TXT_KEY_VICTORY_SCREEN_TITLE", ()).upper() + u"</font>", CvUtil.FONT_CENTER_JUSTIFY, self.X_SCREEN, self.Y_TITLE, self.Z_CONTROLS, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
		
		if self.iScreen == VICTORY_CONDITION_SCREEN:
			self.showVictoryConditionScreen()
		elif self.iScreen == GAME_SETTINGS_SCREEN:
			self.showGameSettingsScreen()
		elif self.iScreen == SPACESHIP_SCREEN:
			self.showSpaceshipScreen()
		elif self.iScreen == UN_RESOLUTION_SCREEN:
			self.showVotingScreen()

	def drawTabs(self):
	
		screen = self.getScreen()

		xLink = self.X_LINK
		if (self.iScreen != VICTORY_CONDITION_SCREEN):
			screen.setText(self.VC_TAB_ID, "", u"<font=4>" + localText.getText("TXT_KEY_MAIN_MENU_VICTORIES", ()).upper() + "</font>", CvUtil.FONT_CENTER_JUSTIFY, xLink, self.Y_LINK, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
		else:
			screen.setText(self.VC_TAB_ID, "", u"<font=4>" + localText.getColorText("TXT_KEY_MAIN_MENU_VICTORIES", (), gc.getInfoTypeForString("COLOR_YELLOW")).upper() + "</font>", CvUtil.FONT_CENTER_JUSTIFY, xLink, self.Y_LINK, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
		xLink += self.DX_LINK
			
		if (self.iScreen != GAME_SETTINGS_SCREEN):
			screen.setText(self.SETTINGS_TAB_ID, "", u"<font=4>" + localText.getText("TXT_KEY_MAIN_MENU_SETTINGS", ()).upper() + "</font>", CvUtil.FONT_CENTER_JUSTIFY, xLink, self.Y_LINK, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
		else:
			screen.setText(self.SETTINGS_TAB_ID, "", u"<font=4>" + localText.getColorText("TXT_KEY_MAIN_MENU_SETTINGS", (), gc.getInfoTypeForString("COLOR_YELLOW")).upper() + "</font>", CvUtil.FONT_CENTER_JUSTIFY, xLink, self.Y_LINK, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
		xLink += self.DX_LINK
			
		if self.bVoteTab:			
			if (self.iScreen != UN_RESOLUTION_SCREEN):
				screen.setText(self.UN_RESOLUTION_TAB_ID, "", u"<font=4>" + localText.getText("TXT_KEY_VOTING_TITLE", ()).upper() + "</font>", CvUtil.FONT_CENTER_JUSTIFY, xLink, self.Y_LINK, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
			else:
				screen.setText(self.UN_RESOLUTION_TAB_ID, "", u"<font=4>" + localText.getColorText("TXT_KEY_VOTING_TITLE", (), gc.getInfoTypeForString("COLOR_YELLOW")).upper() + "</font>", CvUtil.FONT_CENTER_JUSTIFY, xLink, self.Y_LINK, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
			xLink += self.DX_LINK

		if self.bSpaceshipTab:			
			if (self.iScreen != SPACESHIP_SCREEN):
				screen.setText(self.SPACESHIP_TAB_ID, "", u"<font=4>" + localText.getText("TXT_KEY_SPACESHIP_SCREEN_TITLE", ()).upper() + "</font>", CvUtil.FONT_CENTER_JUSTIFY, xLink, self.Y_LINK, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
			else:
				screen.setText(self.SPACESHIP_TAB_ID, "", u"<font=4>" + localText.getColorText("TXT_KEY_SPACESHIP_SCREEN_TITLE", (), gc.getInfoTypeForString("COLOR_YELLOW")).upper() + "</font>", CvUtil.FONT_CENTER_JUSTIFY, xLink, self.Y_LINK, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
			xLink += self.DX_LINK
	
	def showVotingScreen(self):
	
		self.deleteAllWidgets()
	
		activePlayer = gc.getPlayer(self.iActivePlayer)
		iActiveTeam = activePlayer.getTeam()
		
		nVoteBuildings = 0
		iVoteBuildingClass = -1
		for i in range(gc.getNumBuildingInfos()):
			if (gc.getBuildingInfo(i).isDiploVote()):
				iVoteBuildingClass = gc.getBuildingInfo(i).getBuildingClassType()
				nVoteBuildings += 1
				
		if (iVoteBuildingClass == -1):
			return

		iUNTeam = -1
		if (nVoteBuildings == 1 and isWorldWonderClass(iVoteBuildingClass)):
			for iLoopTeam in range(gc.getMAX_CIV_TEAMS()):
				if (gc.getTeam(iLoopTeam).isAlive() and not gc.getTeam(iLoopTeam).isMinorCiv() and not gc.getTeam(iLoopTeam).isBarbarian()):
					if (iLoopTeam == iActiveTeam or gc.getTeam(activePlayer.getTeam()).isHasMet(iLoopTeam) or gc.getGame().isDebugMode()):
						if (gc.getTeam(iLoopTeam).getBuildingClassCount(iVoteBuildingClass) > 0):
							iUNTeam = iLoopTeam

		screen = self.getScreen()

		screen.addPanel(self.getNextWidgetName(), "", "", False, False, self.X_AREA-10, self.Y_AREA-15, self.W_AREA+20, self.H_AREA+30, PanelStyles.PANEL_STYLE_BLUE50)
		szTable = self.getNextWidgetName()
		screen.addTableControlGFC(szTable, 2, self.X_AREA, self.Y_AREA, self.W_AREA, self.H_AREA, False, False, 32,32, TableStyles.TABLE_STYLE_STANDARD)
		screen.enableSelect(szTable, False)		
		screen.setTableColumnHeader(szTable, 0, "", self.TABLE2_WIDTH_0)
		screen.setTableColumnHeader(szTable, 1, "", self.TABLE2_WIDTH_1)

		iRow = screen.appendTableRow(szTable)
		screen.setTableText(szTable, 0, iRow, localText.getText("TXT_KEY_VICTORY_SCREEN_ELECTION", (gc.getBuildingClassInfo(iVoteBuildingClass).getTextKey(), )), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		if (iUNTeam != -1):
			screen.setTableText(szTable, 1, iRow, localText.getText("TXT_KEY_VICTORY_SCREEN_BUILT", (gc.getTeam(iUNTeam).getName(), )), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		else:
			screen.setTableText(szTable, 1, iRow, localText.getText("TXT_KEY_VICTORY_SCREEN_NOT_BUILT", ()), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)

		if (gc.getGame().getSecretaryGeneral() != -1):
			iRow = screen.appendTableRow(szTable)
			screen.setTableText(szTable, 0, iRow, localText.getText("TXT_KEY_SECRETARY_GENERAL", ()), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
			screen.setTableText(szTable, 1, iRow, gc.getTeam(gc.getGame().getSecretaryGeneral()).getName(), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
	
		if gc.getGame().countPossibleVote() > 0:		
			for iLoop in range(gc.getNumVoteInfos()):
				info = gc.getVoteInfo(iLoop)
				if gc.getGame().isChooseElection(iLoop):			
					iRow = screen.appendTableRow(szTable)
					screen.setTableText(szTable, 0, iRow, info.getDescription(), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
					if gc.getGame().isVotePassed(iLoop):
						screen.setTableText(szTable, 1, iRow, localText.getText("TXT_KEY_POPUP_PASSED", ()), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
					else:
						screen.setTableText(szTable, 1, iRow, localText.getText("TXT_KEY_POPUP_ELECTION_OPTION", (u"", gc.getGame().getVoteRequired(iLoop), gc.getGame().countPossibleVote())), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
								
		self.drawTabs()


	def showSpaceshipScreen(self):

		self.deleteAllWidgets()
	
		activePlayer = gc.getPlayer(self.iActivePlayer)
		iActiveTeam = activePlayer.getTeam()
		
		screen = self.getScreen()

		screen.addDDSGFC(self.getNextWidgetName(), ArtFileMgr.getInterfaceArtInfo("SPACESHIP_PROGRESS_BG").getPath(), self.X_BG, self.Y_BG, self.W_BG, self.H_BG, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		
		for i in range(len(self.locations)):
			screen.addDDSGFC(self.getMouseOverWidget(i, False), self.locations[i][5], self.X_BG + self.X_DETAIL, self.Y_BG + self.Y_DETAIL, self.W_DETAIL, self.H_DETAIL, WidgetTypes.WIDGET_GENERAL, -1, -1)
			szName = localText.getObjectText(self.locations[i][6], 0)
			szName = localText.changeTextColor(szName, gc.getInfoTypeForString("COLOR_BROWN_TEXT"))
			screen.setLabel(self.getMouseOverWidget(i, True), "Background", u"<font=3>" + szName + u"</font>", CvUtil.FONT_CENTER_JUSTIFY, self.X_BG + self.X_DETAIL + self.W_DETAIL/2, self.Y_BG + self.Y_DETAIL - 25, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
			screen.hide(self.getMouseOverWidget(i, False))
			screen.hide(self.getMouseOverWidget(i, True))

		for iLoopVC in range(gc.getNumVictoryInfos()):
			victory = gc.getVictoryInfo(iLoopVC)
			if gc.getGame().isVictoryValid(iLoopVC):			
				iSSComponent = 0
				for i in range(gc.getNumProjectInfos()):
					component = gc.getProjectInfo(i)
					if (component.isSpaceship()):
						if (True):
#						if (component.getVictoryThreshold(iLoopVC) > 0 and component.getVictoryThreshold(iLoopVC) == gc.getTeam(iActiveTeam).getProjectCount(i)):
							if iSSComponent < len(self.locations):	
								screen.setImageButton(self.SPACESHIP_WIDGET_ID + str(iSSComponent), self.locations[iSSComponent][4], self.X_BG + self.locations[iSSComponent][0], self.Y_BG + self.locations[iSSComponent][1], self.locations[iSSComponent][2], self.locations[iSSComponent][3], WidgetTypes.WIDGET_GENERAL, -1, -1)	
						iSSComponent += 1
		self.drawTabs()


	def showGameSettingsScreen(self):
	
		self.deleteAllWidgets()	
		screen = self.getScreen()
				

		activePlayer = gc.getPlayer(self.iActivePlayer)		

		szSettingsPanel = self.getNextWidgetName()
		screen.addPanel(szSettingsPanel, localText.getText("TXT_KEY_MAIN_MENU_SETTINGS", ()).upper(), "", True, True, self.SETTINGS_PANEL_X1, self.SETTINGS_PANEL_Y - 10, self.SETTINGS_PANEL_WIDTH, self.SETTINGS_PANEL_HEIGHT, PanelStyles.PANEL_STYLE_MAIN)
		szSettingsTable = self.getNextWidgetName()
		screen.addListBoxGFC(szSettingsTable, "", self.SETTINGS_PANEL_X1 + self.MARGIN, self.SETTINGS_PANEL_Y + self.MARGIN, self.SETTINGS_PANEL_WIDTH - 2*self.MARGIN, self.SETTINGS_PANEL_HEIGHT - 2*self.MARGIN, TableStyles.TABLE_STYLE_EMPTY)
		screen.enableSelect(szSettingsTable, False)
		
		screen.appendListBoxString(szSettingsTable, localText.getText("TXT_KEY_LEADER_CIV_DESCRIPTION", (activePlayer.getNameKey(), activePlayer.getCivilizationShortDescriptionKey())), WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
		screen.appendListBoxString(szSettingsTable, u"     (" + CyGameTextMgr().parseLeaderTraits(activePlayer.getLeaderType(), activePlayer.getCivilizationType(), True, False) + ")", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
		screen.appendListBoxString(szSettingsTable, " ", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
		screen.appendListBoxString(szSettingsTable, localText.getText("TXT_KEY_SETTINGS_DIFFICULTY", (gc.getHandicapInfo(activePlayer.getHandicapType()).getTextKey(), )), WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
		screen.appendListBoxString(szSettingsTable, " ", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
		screen.appendListBoxString(szSettingsTable, gc.getMap().getMapScriptName(), WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
		screen.appendListBoxString(szSettingsTable, localText.getText("TXT_KEY_SETTINGS_MAP_SIZE", (gc.getWorldInfo(gc.getMap().getWorldSize()).getTextKey(), )), WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
		screen.appendListBoxString(szSettingsTable, localText.getText("TXT_KEY_SETTINGS_CLIMATE", (gc.getClimateInfo(gc.getMap().getClimate()).getTextKey(), )), WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
		screen.appendListBoxString(szSettingsTable, localText.getText("TXT_KEY_SETTINGS_SEA_LEVEL", (gc.getSeaLevelInfo(gc.getMap().getSeaLevel()).getTextKey(), )), WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
		screen.appendListBoxString(szSettingsTable, " ", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
		screen.appendListBoxString(szSettingsTable, localText.getText("TXT_KEY_SETTINGS_STARTING_ERA", (gc.getEraInfo(gc.getGame().getStartEra()).getTextKey(), )), WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
		screen.appendListBoxString(szSettingsTable, localText.getText("TXT_KEY_SETTINGS_GAME_SPEED", (gc.getGameSpeedInfo(gc.getGame().getGameSpeedType()).getTextKey(), )), WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
		
		szOptionsPanel = self.getNextWidgetName()
		screen.addPanel(szOptionsPanel, localText.getText("TXT_KEY_MAIN_MENU_CUSTOM_SETUP_OPTIONS", ()).upper(), "", True, True, self.SETTINGS_PANEL_X2, self.SETTINGS_PANEL_Y - 10, self.SETTINGS_PANEL_WIDTH, self.SETTINGS_PANEL_HEIGHT, PanelStyles.PANEL_STYLE_MAIN)
		szOptionsTable = self.getNextWidgetName()
		screen.addListBoxGFC(szOptionsTable, "", self.SETTINGS_PANEL_X2 + self.MARGIN, self.SETTINGS_PANEL_Y + self.MARGIN, self.SETTINGS_PANEL_WIDTH - 2*self.MARGIN, self.SETTINGS_PANEL_HEIGHT - 2*self.MARGIN, TableStyles.TABLE_STYLE_EMPTY)
		screen.enableSelect(szOptionsTable, False)

		for i in range(GameOptionTypes.NUM_GAMEOPTION_TYPES):
			if gc.getGame().isOption(i):
				screen.appendListBoxString(szOptionsTable, gc.getGameOptionInfo(i).getDescription(), WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)		

		if (gc.getGame().isGameMultiPlayer()):
			for i in range(gc.getNumMPOptionInfos()):
				if (gc.getGame().isMPOption(i)):
					screen.appendListBoxString(szOptionsTable, gc.getMPOptionInfo(i).getDescription(), WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		
			if (gc.getGame().getMaxTurns() > 0):
				szMaxTurns = u"%s %d" % (localText.getText("TXT_KEY_TURN_LIMIT_TAG", ()), gc.getGame().getMaxTurns())
				screen.appendListBoxString(szOptionsTable, szMaxTurns, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)		
				
			if (gc.getGame().getMaxCityElimination() > 0):
				szMaxCityElimination = u"%s %d" % (localText.getText("TXT_KEY_CITY_ELIM_TAG", ()), gc.getGame().getMaxCityElimination())
				screen.appendListBoxString(szOptionsTable, szMaxCityElimination, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)		

		if (gc.getGame().hasSkippedSaveChecksum()):
			screen.appendListBoxString(szOptionsTable, "Skipped Checksum", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)		

		szCivsPanel = self.getNextWidgetName()
		screen.addPanel(szCivsPanel, localText.getText("TXT_KEY_RIVALS_MET", ()).upper(), "", True, True, self.SETTINGS_PANEL_X3, self.SETTINGS_PANEL_Y - 10, self.SETTINGS_PANEL_WIDTH, self.SETTINGS_PANEL_HEIGHT, PanelStyles.PANEL_STYLE_MAIN)

		szCivsTable = self.getNextWidgetName()
		screen.addListBoxGFC(szCivsTable, "", self.SETTINGS_PANEL_X3 + self.MARGIN, self.SETTINGS_PANEL_Y + self.MARGIN, self.SETTINGS_PANEL_WIDTH - 2*self.MARGIN, self.SETTINGS_PANEL_HEIGHT - 2*self.MARGIN, TableStyles.TABLE_STYLE_EMPTY)
		screen.enableSelect(szCivsTable, False)

		for iLoopPlayer in range(gc.getMAX_CIV_PLAYERS()):
			player = gc.getPlayer(iLoopPlayer)
			if (player.isEverAlive() and iLoopPlayer != self.iActivePlayer and (gc.getTeam(player.getTeam()).isHasMet(activePlayer.getTeam()) or gc.getGame().isDebugMode()) and not player.isBarbarian() and not player.isMinorCiv()):
				screen.appendListBoxString(szCivsTable, localText.getText("TXT_KEY_LEADER_CIV_DESCRIPTION", (player.getNameKey(), player.getCivilizationShortDescriptionKey())), WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
				screen.appendListBoxString(szCivsTable, u"     (" + CyGameTextMgr().parseLeaderTraits(player.getLeaderType(), player.getCivilizationType(), True, False) + ")", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
				screen.appendListBoxString(szCivsTable, " ", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )

		self.drawTabs()
		

	def showVictoryConditionScreen(self):
				
		activePlayer = PyHelpers.PyPlayer(self.iActivePlayer)
		iActiveTeam = gc.getPlayer(self.iActivePlayer).getTeam()
		
		# Conquest
		nRivals = -1 
		for i in range(gc.getMAX_CIV_TEAMS()):
			if (gc.getTeam(i).isAlive() and not gc.getTeam(i).isMinorCiv() and not gc.getTeam(i).isBarbarian()):
				nRivals += 1

		# Population
		totalPop = gc.getGame().getTotalPopulation()
		ourPop = activePlayer.getTeam().getTotalPopulation()
		if (totalPop > 0):
			popPercent = (ourPop * 100.0) / totalPop
		else:
			popPercent = 0.0

		iBestPopTeam = -1
		bestPop = 0
		for iLoopTeam in range(gc.getMAX_CIV_TEAMS()):
			if (gc.getTeam(iLoopTeam).isAlive() and not gc.getTeam(iLoopTeam).isMinorCiv() and not gc.getTeam(iLoopTeam).isBarbarian()):
				if (iLoopTeam != iActiveTeam and (activePlayer.getTeam().isHasMet(iLoopTeam) or gc.getGame().isDebugMode())):
					teamPop = gc.getTeam(iLoopTeam).getTotalPopulation()
					if (teamPop > bestPop):
						bestPop = teamPop
						iBestPopTeam = iLoopTeam

		# Score
		ourScore = gc.getGame().getTeamScore(iActiveTeam)
			
		iBestScoreTeam = -1
		bestScore = 0
		for iLoopTeam in range(gc.getMAX_CIV_TEAMS()):
			if (gc.getTeam(iLoopTeam).isAlive() and not gc.getTeam(iLoopTeam).isMinorCiv() and not gc.getTeam(iLoopTeam).isBarbarian()):
				if (iLoopTeam != iActiveTeam and (activePlayer.getTeam().isHasMet(iLoopTeam) or gc.getGame().isDebugMode())):
					teamScore = gc.getGame().getTeamScore(iLoopTeam)
					if (teamScore > bestScore):
						bestScore = teamScore
						iBestScoreTeam = iLoopTeam

		# Land Area
		totalLand = gc.getMap().getLandPlots()
		ourLand = activePlayer.getTeam().getTotalLand()
		if (totalLand > 0):
			landPercent = (ourLand * 100.0) / totalLand
		else:
			landPercent = 0.0
			
		iBestLandTeam = -1
		bestLand = 0
		for iLoopTeam in range(gc.getMAX_CIV_TEAMS()):
			if (gc.getTeam(iLoopTeam).isAlive() and not gc.getTeam(iLoopTeam).isMinorCiv() and not gc.getTeam(iLoopTeam).isBarbarian()):
				if (iLoopTeam != iActiveTeam and (activePlayer.getTeam().isHasMet(iLoopTeam) or gc.getGame().isDebugMode())):
					teamLand = gc.getTeam(iLoopTeam).getTotalLand()
					if (teamLand > bestLand):
						bestLand = teamLand
						iBestLandTeam = iLoopTeam

		# Religion
		iOurReligion = -1
		ourReligionPercent = 0
		for iLoopReligion in range(gc.getNumReligionInfos()):
			if (activePlayer.getTeam().hasHolyCity(iLoopReligion)):
				religionPercent = gc.getGame().calculateReligionPercent(iLoopReligion)
				if (religionPercent > ourReligionPercent):
					ourReligionPercent = religionPercent
					iOurReligion = iLoopReligion

		iBestReligion = -1
		bestReligionPercent = 0
		for iLoopReligion in range(gc.getNumReligionInfos()):
			if (iLoopReligion != iOurReligion):
				religionPercent = gc.getGame().calculateReligionPercent(iLoopReligion)
				if (religionPercent > bestReligionPercent):
					bestReligionPercent = religionPercent
					iBestReligion = iLoopReligion

		# Total Culture
		ourCulture = activePlayer.getTeam().countTotalCulture()

		iBestCultureTeam = -1
		bestCulture = 0
		for iLoopTeam in range(gc.getMAX_CIV_TEAMS()):
			if (gc.getTeam(iLoopTeam).isAlive() and not gc.getTeam(iLoopTeam).isMinorCiv() and not gc.getTeam(iLoopTeam).isBarbarian()):
				if (iLoopTeam != iActiveTeam and (activePlayer.getTeam().isHasMet(iLoopTeam) or gc.getGame().isDebugMode())):
					teamCulture = gc.getTeam(iLoopTeam).countTotalCulture()
					if (teamCulture > bestCulture):
						bestCulture = teamCulture
						iBestCultureTeam = iLoopTeam

		# Vote
		nVoteBuildings = 0
		iVoteBuildingClass = -1
		for i in range(gc.getNumBuildingInfos()):
			if (gc.getBuildingInfo(i).isDiploVote()):
				iVoteBuildingClass = gc.getBuildingInfo(i).getBuildingClassType()
				nVoteBuildings += 1
		self.bVoteTab = (nVoteBuildings > 0)

		iUNTeam = -1
		if (nVoteBuildings == 1 and isWorldWonderClass(iVoteBuildingClass)):
			for iLoopTeam in range(gc.getMAX_CIV_TEAMS()):
				if (gc.getTeam(iLoopTeam).isAlive() and not gc.getTeam(iLoopTeam).isMinorCiv() and not gc.getTeam(iLoopTeam).isBarbarian()):
					if (iLoopTeam == iActiveTeam or activePlayer.getTeam().isHasMet(iLoopTeam) or gc.getGame().isDebugMode()):
						if (gc.getTeam(iLoopTeam).getBuildingClassCount(iVoteBuildingClass) > 0):
							iUNTeam = iLoopTeam
		
		self.deleteAllWidgets()	
		screen = self.getScreen()
														
		# Start filling in the table below
		screen.addPanel(self.getNextWidgetName(), "", "", False, False, self.X_AREA-10, self.Y_AREA-15, self.W_AREA+20, self.H_AREA+30, PanelStyles.PANEL_STYLE_BLUE50)
		szTable = self.getNextWidgetName()
		screen.addTableControlGFC(szTable, 5, self.X_AREA, self.Y_AREA, self.W_AREA, self.H_AREA, False, False, 32,32, TableStyles.TABLE_STYLE_STANDARD)
		screen.setTableColumnHeader(szTable, 0, "", self.TABLE_WIDTH_0)
		screen.setTableColumnHeader(szTable, 1, "", self.TABLE_WIDTH_1)
		screen.setTableColumnHeader(szTable, 2, "", self.TABLE_WIDTH_2)
		screen.setTableColumnHeader(szTable, 3, "", self.TABLE_WIDTH_3)
		screen.setTableColumnHeader(szTable, 4, "", self.TABLE_WIDTH_4)
		screen.appendTableRow(szTable)
		
		for iLoopVC in range(gc.getNumVictoryInfos()):
			victory = gc.getVictoryInfo(iLoopVC)
			if gc.getGame().isVictoryValid(iLoopVC):
				
				iNumRows = screen.getTableNumRows(szTable)
				szVictoryType = u"<font=4b>" + victory.getDescription().upper() + u"</font>"
				if (victory.isEndScore() and (gc.getGame().getMaxTurns() > gc.getGame().getElapsedGameTurns())):
					szVictoryType += "    (" + localText.getText("TXT_KEY_MISC_TURNS_LEFT", (gc.getGame().getMaxTurns() - gc.getGame().getElapsedGameTurns(), )) + ")"

				screen.setTableText(szTable, 0, iNumRows - 1, szVictoryType, "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
					
				bEntriesFound = False
				
				if (victory.isTargetScore() and gc.getGame().getTargetScore() != 0):
										
					iRow = screen.appendTableRow(szTable)
					screen.setTableText(szTable, 0, iRow, localText.getText("TXT_KEY_VICTORY_SCREEN_TARGET_SCORE", (gc.getGame().getTargetScore(), )), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
					screen.setTableText(szTable, 1, iRow, activePlayer.getTeam().getName() + ":", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
					screen.setTableText(szTable, 2, iRow, (u"%d" % ourScore), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
					
					if (iBestScoreTeam != -1):
						screen.setTableText(szTable, 3, iRow, gc.getTeam(iBestScoreTeam).getName() + ":", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
						screen.setTableText(szTable, 4, iRow, (u"%d" % bestScore), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
						
					bEntriesFound = True
				
				if (victory.isEndScore()):

					szText1 = localText.getText("TXT_KEY_VICTORY_SCREEN_HIGHEST_SCORE", (CyGameTextMgr().getTimeStr(gc.getGame().getStartTurn() + gc.getGame().getMaxTurns(), false), ))

					iRow = screen.appendTableRow(szTable)
					screen.setTableText(szTable, 0, iRow, szText1, "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
					screen.setTableText(szTable, 1, iRow, activePlayer.getTeam().getName() + ":", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
					screen.setTableText(szTable, 2, iRow, (u"%d" % ourScore), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
					
					if (iBestScoreTeam != -1):
						screen.setTableText(szTable, 3, iRow, gc.getTeam(iBestScoreTeam).getName() + ":", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
						screen.setTableText(szTable, 4, iRow, (u"%d" % bestScore), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
						
					bEntriesFound = True
					
				if (victory.isConquest()):
					iRow = screen.appendTableRow(szTable)
					screen.setTableText(szTable, 0, iRow, localText.getText("TXT_KEY_VICTORY_SCREEN_ELIMINATE_ALL", ()), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
					screen.setTableText(szTable, 1, iRow, localText.getText("TXT_KEY_VICTORY_SCREEN_RIVALS_LEFT", ()), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
					screen.setTableText(szTable, 2, iRow, unicode(nRivals), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
					bEntriesFound = True
					
				
				if (gc.getGame().getAdjustedPopulationPercent(iLoopVC) > 0):			
					iRow = screen.appendTableRow(szTable)
					screen.setTableText(szTable, 0, iRow, localText.getText("TXT_KEY_VICTORY_SCREEN_PERCENT_POP", (gc.getGame().getAdjustedPopulationPercent(iLoopVC), )), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
					screen.setTableText(szTable, 1, iRow, activePlayer.getTeam().getName() + ":", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
					screen.setTableText(szTable, 2, iRow, (u"%.2f%%" % popPercent), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
					if (iBestPopTeam != -1):
						screen.setTableText(szTable, 3, iRow, gc.getTeam(iBestPopTeam).getName() + ":", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
						screen.setTableText(szTable, 4, iRow, (u"%.2f%%" % (bestPop * 100 / totalPop)), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
					bEntriesFound = True


				if (gc.getGame().getAdjustedLandPercent(iLoopVC) > 0):
					iRow = screen.appendTableRow(szTable)
					screen.setTableText(szTable, 0, iRow, localText.getText("TXT_KEY_VICTORY_SCREEN_PERCENT_LAND", (gc.getGame().getAdjustedLandPercent(iLoopVC), )), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
					screen.setTableText(szTable, 1, iRow, activePlayer.getTeam().getName() + ":", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
					screen.setTableText(szTable, 2, iRow, (u"%.2f%%" % landPercent), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
					if (iBestLandTeam != -1):
						screen.setTableText(szTable, 3, iRow, gc.getTeam(iBestLandTeam).getName() + ":", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
						screen.setTableText(szTable, 4, iRow, (u"%.2f%%" % (bestLand * 100 / totalLand)), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
					bEntriesFound = True

				if (victory.getReligionPercent() > 0):			
					iRow = screen.appendTableRow(szTable)
					screen.setTableText(szTable, 0, iRow, localText.getText("TXT_KEY_VICTORY_SCREEN_PERCENT_RELIGION", (victory.getReligionPercent(), )), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
					if (iOurReligion != -1):
						screen.setTableText(szTable, 1, iRow, gc.getReligionInfo(iOurReligion).getDescription() + ":", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
						screen.setTableText(szTable, 2, iRow, (u"%d%%" % ourReligionPercent), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
					else:
						screen.setTableText(szTable, 1, iRow, activePlayer.getTeam().getName() + ":", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
						screen.setTableText(szTable, 2, iRow, u"No Holy City", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
					if (iBestReligion != -1):
						screen.setTableText(szTable, 3, iRow, gc.getReligionInfo(iBestReligion).getDescription() + ":", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
						screen.setTableText(szTable, 4, iRow, (u"%d%%" % religionPercent), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
					bEntriesFound = True
				
				if (victory.getTotalCultureRatio() > 0):			
					iRow = screen.appendTableRow(szTable)
					screen.setTableText(szTable, 0, iRow, localText.getText("TXT_KEY_VICTORY_SCREEN_PERCENT_CULTURE", (int((100.0 * bestCulture) / victory.getTotalCultureRatio()), )), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
					screen.setTableText(szTable, 1, iRow, activePlayer.getTeam().getName() + ":", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
					screen.setTableText(szTable, 2, iRow, unicode(ourCulture), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
					if (iBestLandTeam != -1):
						screen.setTableText(szTable, 3, iRow, gc.getTeam(iBestCultureTeam).getName() + ":", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
						screen.setTableText(szTable, 4, iRow, unicode(bestCulture), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
					bEntriesFound = True

				iBestBuildingTeam = -1
				bestBuilding = 0
				for iLoopTeam in range(gc.getMAX_CIV_TEAMS()):
					if (gc.getTeam(iLoopTeam).isAlive() and not gc.getTeam(iLoopTeam).isMinorCiv() and not gc.getTeam(iLoopTeam).isBarbarian()):
						if (iLoopTeam != iActiveTeam and (activePlayer.getTeam().isHasMet(iLoopTeam) or gc.getGame().isDebugMode())):
							teamBuilding = 0
							for i in range(gc.getNumProjectInfos()):
								if (gc.getBuildingClassInfo(i).getVictoryThreshold(iLoopVC) > 0):					
									teamBuilding += gc.getTeam(iLoopTeam).getBuildingClassCount(i)
							if (teamBuilding > bestBuilding):
								bestBuilding = teamBuilding
								iBestBuildingTeam = iLoopTeam	
											
				for i in range(gc.getNumBuildingClassInfos()):
					if (gc.getBuildingClassInfo(i).getVictoryThreshold(iLoopVC) > 0):
						iRow = screen.appendTableRow(szTable)
						screen.setTableText(szTable, 0, iRow, localText.getText("TXT_KEY_VICTORY_SCREEN_BUILDING", (gc.getBuildingClassInfo(i).getVictoryThreshold(iLoopVC), gc.getBuildingClassInfo(i).getTextKey())), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
						screen.setTableText(szTable, 1, iRow, activePlayer.getTeam().getName() + ":", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
						screen.setTableText(szTable, 2, iRow, activePlayer.getTeam().getBuildingClassCount(i), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
						if (iBestBuildingTeam != -1):
							screen.setTableText(szTable, 3, iRow, gc.getTeam(iBestBuildingTeam).getName() + ":", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
							screen.setTableText(szTable, 4, iRow, gc.getTeam(iBestBuildingTeam).getBuildingClassCount(i), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
						bEntriesFound = True
						
				iBestProjectTeam = -1
				bestProject = 0
				for iLoopTeam in range(gc.getMAX_CIV_TEAMS()):
					if (gc.getTeam(iLoopTeam).isAlive() and not gc.getTeam(iLoopTeam).isMinorCiv() and not gc.getTeam(iLoopTeam).isBarbarian()):
						if (iLoopTeam != iActiveTeam and (activePlayer.getTeam().isHasMet(iLoopTeam) or gc.getGame().isDebugMode())):
							teamProject = 0
							for i in range(gc.getNumProjectInfos()):
								if (gc.getProjectInfo(i).getVictoryThreshold(iLoopVC) > 0):					
									teamProject += gc.getTeam(iLoopTeam).getProjectCount(i)
							if (teamProject > bestProject):
								bestProject = teamProject
								iBestProjectTeam = iLoopTeam					
					
				for i in range(gc.getNumProjectInfos()):
					if (gc.getProjectInfo(i).getVictoryThreshold(iLoopVC) > 0):
						if (gc.getProjectInfo(i).isSpaceship()):
							self.bSpaceshipTab = True
						iRow = screen.appendTableRow(szTable)
						
						screen.setTableText(szTable, 0, iRow, localText.getText("TXT_KEY_VICTORY_SCREEN_BUILDING", (gc.getProjectInfo(i).getVictoryThreshold(iLoopVC), gc.getProjectInfo(i).getTextKey())), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
						screen.setTableText(szTable, 1, iRow, activePlayer.getTeam().getName() + ":", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
						screen.setTableText(szTable, 2, iRow, str(activePlayer.getTeam().getProjectCount(i)), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
						if (iBestProjectTeam != -1):
							screen.setTableText(szTable, 3, iRow, gc.getTeam(iBestProjectTeam).getName() + ":", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
							screen.setTableText(szTable, 4, iRow, unicode(gc.getTeam(iBestProjectTeam).getProjectCount(i)), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
						bEntriesFound = True
						
				if (victory.isDiploVote() and iVoteBuildingClass != -1):						
					iRow = screen.appendTableRow(szTable)
					screen.setTableText(szTable, 0, iRow, localText.getText("TXT_KEY_VICTORY_SCREEN_ELECTION", (gc.getBuildingClassInfo(iVoteBuildingClass).getTextKey(), )), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
					if (iUNTeam != -1):
						screen.setTableText(szTable, 1, iRow, localText.getText("TXT_KEY_VICTORY_SCREEN_BUILT", (gc.getTeam(iUNTeam).getName(), )), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
					else:
						screen.setTableText(szTable, 1, iRow, localText.getText("TXT_KEY_VICTORY_SCREEN_NOT_BUILT", ()), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
					bEntriesFound = True
					
				if (victory.getCityCulture() != CultureLevelTypes.NO_CULTURELEVEL and victory.getNumCultureCities() > 0):
					ourBestCities = self.getListCultureCities(self.iActivePlayer)[0:victory.getNumCultureCities()]
					
					iBestCulturePlayer = -1
					bestCityCulture = 0
					for iLoopPlayer in range(gc.getMAX_PLAYERS()):
						if (gc.getPlayer(iLoopPlayer).isAlive() and not gc.getPlayer(iLoopPlayer).isMinorCiv() and not gc.getPlayer(iLoopPlayer).isBarbarian()):
							if (iLoopPlayer != self.iActivePlayer and (activePlayer.getTeam().isHasMet(gc.getPlayer(iLoopPlayer).getTeam()) or gc.getGame().isDebugMode())):
								theirBestCities = self.getListCultureCities(iLoopPlayer)[0:victory.getNumCultureCities()]
								
								if (len(theirBestCities) >= victory.getNumCultureCities() and theirBestCities[victory.getNumCultureCities()-1][0] > bestCityCulture):
									bestCityCulture = theirBestCities[victory.getNumCultureCities()-1][0]
									iBestCulturePlayer = iLoopPlayer

					iRow = screen.appendTableRow(szTable)
					screen.setTableText(szTable, 0, iRow, localText.getText("TXT_KEY_VICTORY_SCREEN_CITY_CULTURE", (victory.getNumCultureCities(), gc.getCultureLevelInfo(victory.getCityCulture()).getTextKey())), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
					for i in range(len(ourBestCities)):
						if i > 0:
							iRow = screen.appendTableRow(szTable)
						screen.setTableText(szTable, 1, iRow, ourBestCities[i][1].getName() + ":", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
						screen.setTableText(szTable, 2, iRow, str(ourBestCities[i][0]), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
					if (iBestCulturePlayer != -1):
						theirBestCities = self.getListCultureCities(iBestCulturePlayer)[0:(victory.getNumCultureCities())]
						iRow = iRow - len(ourBestCities) + 1
						for i in range(len(theirBestCities)):
							screen.setTableText(szTable, 3, iRow, theirBestCities[i][1].getName() + ":", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
							screen.setTableText(szTable, 4, iRow, unicode(theirBestCities[i][0]), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
							iRow += 1
					bEntriesFound = True
					
				if (bEntriesFound):
					screen.appendTableRow(szTable)
					screen.appendTableRow(szTable)

		# civ picker dropdown
		if (CyGame().isDebugMode()):
			self.szDropdownName = self.DEBUG_DROPDOWN_ID
			screen.addDropDownBoxGFC(self.szDropdownName, 22, 12, 300, WidgetTypes.WIDGET_GENERAL, -1, -1, FontTypes.GAME_FONT)
			for j in range(gc.getMAX_PLAYERS()):
				if (gc.getPlayer(j).isAlive()):
					screen.addPullDownString(self.szDropdownName, gc.getPlayer(j).getName(), j, j, False )
		
		self.drawTabs()

	def getListCultureCities(self, iPlayer):
		if iPlayer >= 0:
			player = PyPlayer(iPlayer)
			if player.isAlive():
				cityList = player.getCityList()
				listCultureCities = len(cityList) * [(0, 0)]
				i = 0
				for city in cityList:
					listCultureCities[i] = (city.getCulture(), city)
					i += 1
				listCultureCities.sort()
				listCultureCities.reverse()
				return listCultureCities
		return []					
				
										
	# returns a unique ID for a widget in this screen
	def getNextWidgetName(self):
		szName = self.WIDGET_ID + str(self.nWidgetCount)
		self.nWidgetCount += 1
		return szName
	
	def getMouseOverWidget(self, iComponent, bText):
		if bText:
			iWidget = len(self.locations) + iComponent
		else:
			iWidget = 2 * len(self.locations) + iComponent
		return self.WIDGET_ID + str(iWidget)
				
	def deleteAllWidgets(self):
		screen = self.getScreen()
		i = self.nWidgetCount - 1
		while (i >= 0):
			self.nWidgetCount = i
			screen.deleteWidget(self.getNextWidgetName())
			i -= 1

		self.nWidgetCount = 0		

		for i in range(len(self.locations)):
			screen.deleteWidget(self.SPACESHIP_WIDGET_ID + str(i))
			screen.deleteWidget(self.getMouseOverWidget(i, True))
			screen.deleteWidget(self.getMouseOverWidget(i, False))
																				

																				
	# handle the input for this screen...
	def handleInput (self, inputClass):
		if (inputClass.getNotifyCode() == NotifyCode.NOTIFY_LISTBOX_ITEM_SELECTED):
			if (inputClass.getFunctionName() == self.DEBUG_DROPDOWN_ID):
				szName = self.DEBUG_DROPDOWN_ID
				iIndex = self.getScreen().getSelectedPullDownID(szName)
				self.iActivePlayer = self.getScreen().getPullDownData(szName, iIndex)
				self.iScreen = VICTORY_CONDITION_SCREEN
				self.showVictoryConditionScreen()				
		elif (inputClass.getNotifyCode() == NotifyCode.NOTIFY_CLICKED):
			if (inputClass.getFunctionName() == self.VC_TAB_ID):
				self.iScreen = VICTORY_CONDITION_SCREEN
				self.showVictoryConditionScreen()				
			elif (inputClass.getFunctionName() == self.SETTINGS_TAB_ID):
				self.iScreen = GAME_SETTINGS_SCREEN
				self.showGameSettingsScreen()
			elif (inputClass.getFunctionName() == self.SPACESHIP_TAB_ID):
				self.iScreen = SPACESHIP_SCREEN
				self.showSpaceshipScreen()
			elif (inputClass.getFunctionName() == self.UN_RESOLUTION_TAB_ID):
				self.iScreen = UN_RESOLUTION_SCREEN
				self.showVotingScreen()
		elif (inputClass.getNotifyCode() == NotifyCode.NOTIFY_CURSOR_MOVE_ON):
			if (inputClass.getFunctionName() == self.SPACESHIP_WIDGET_ID and inputClass.getID() < len(self.locations)):
				screen = self.getScreen()
				screen.show(self.getMouseOverWidget(inputClass.getID(), False))
				screen.show(self.getMouseOverWidget(inputClass.getID(), True))
		elif (inputClass.getNotifyCode() == NotifyCode.NOTIFY_CURSOR_MOVE_OFF):
			if (inputClass.getFunctionName() == self.SPACESHIP_WIDGET_ID and inputClass.getID() < len(self.locations)):
				screen = self.getScreen()
				screen.hide(self.getMouseOverWidget(inputClass.getID(), False))
				screen.hide(self.getMouseOverWidget(inputClass.getID(), True))

	def update(self, fDelta):
		return
