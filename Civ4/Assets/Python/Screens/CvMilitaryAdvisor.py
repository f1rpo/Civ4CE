## Sid Meier's Civilization 4
## Copyright Firaxis Games 2005
from CvPythonExtensions import *
import CvUtil
import ScreenInput
import time
import PyHelpers

PyPlayer = PyHelpers.PyPlayer
PyInfo = PyHelpers.PyInfo

# globals
gc = CyGlobalContext()
ArtFileMgr = CyArtFileMgr()
localText = CyTranslator()

class CvMilitaryAdvisor:
	"Military Advisor"

	def __init__(self, screenId):
		self.screenId = screenId
		self.MILITARY_SCREEN_NAME = "MilitaryAdvisor"
		self.BACKGROUND_ID = "MilitaryAdvisorBackground"
		self.EXIT_ID = "MilitaryAdvisorExitWidget"

		self.WIDGET_ID = "MilitaryAdvisorWidget"
		self.REFRESH_WIDGET_ID = "MilitaryAdvisorRefreshWidget"
		self.ATTACH_WIDGET_ID = "MilitaryAdvisorAttachWidget"
		self.SELECTION_WIDGET_ID = "MilitaryAdvisorSelectionWidget"
		self.ATTACHED_WIDGET_ID = "MilitaryAdvisorAttachedWidget" # no need to explicitly delete these
		self.LEADER_BUTTON_ID = "MilitaryAdvisorLeaderButton"
		self.UNIT_PANEL_ID = "MilitaryAdvisorUnitPanel"
		self.LEADER_PANEL_ID = "MilitaryAdvisorLeaderPanel"
		self.UNIT_LIST_ID = "MilitaryAdvisorUnitList"

		self.Z_BACKGROUND = -2.1
		self.Z_CONTROLS = self.Z_BACKGROUND - 0.2
		self.DZ = -0.2

		self.X_SCREEN = 500
		self.Y_SCREEN = 396
		self.W_SCREEN = 1024
		self.H_SCREEN = 768
		self.Y_TITLE = 8
		self.BORDER_WIDTH = 4
		self.W_HELP_AREA = 200
		
		self.X_EXIT = 994
		self.Y_EXIT = 726
								
		self.nWidgetCount = 0
		self.nRefreshWidgetCount = 0
		self.nAttachedWidgetCount = 0
		self.iActivePlayer = -1
		self.selectedPlayerList = []
		self.selectedUnitList = []
		
		self.X_MAP = 20
		self.Y_MAP = 190
		self.W_MAP = 580
		self.H_MAP_MAX = 500
		self.MAP_MARGIN = 20
		
		self.X_TEXT = 625
		self.Y_TEXT = 190
		self.W_TEXT = 380
		self.H_TEXT = 500
						
		self.X_LEADERS = 20
		self.Y_LEADERS = 80
		self.W_LEADERS = 985
		self.H_LEADERS = 90
		self.LEADER_BUTTON_SIZE = 64
		self.LEADER_MARGIN = 12
		
		self.LEADER_COLUMNS = int(self.W_LEADERS / (self.LEADER_BUTTON_SIZE + self.LEADER_MARGIN))
		
						
	def getScreen(self):
		return CyGInterfaceScreen(self.MILITARY_SCREEN_NAME, self.screenId)

	def hideScreen(self):
		screen = self.getScreen()
		screen.hideScreen()
										
	# Screen construction function
	def interfaceScreen(self):
							
		# Create a new screen
		screen = self.getScreen()
		if screen.isActive():
			return
		screen.setRenderInterfaceOnly(True);
		screen.showScreen(PopupStates.POPUPSTATE_IMMEDIATE, False)

		self.EXIT_TEXT = u"<font=4>" + localText.getText("TXT_KEY_PEDIA_SCREEN_EXIT", ()).upper() + "</font>"
		self.TITLE = u"<font=4b>" + localText.getText("TXT_KEY_MILITARY_ADVISOR_TITLE", ()).upper() + "</font>"

		self.nWidgetCount = 0
	
		# Set the background and exit button, and show the screen
		screen.setDimensions(screen.centerX(0), screen.centerY(0), self.W_SCREEN, self.H_SCREEN)
		screen.addDDSGFC(self.BACKGROUND_ID, ArtFileMgr.getInterfaceArtInfo("MAINMENU_SLIDESHOW_LOAD").getPath(), 0, 0, self.W_SCREEN, self.H_SCREEN, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.addPanel( "TechTopPanel", u"", u"", True, False, 0, 0, self.W_SCREEN, 55, PanelStyles.PANEL_STYLE_TOPBAR )
		screen.addPanel( "TechBottomPanel", u"", u"", True, False, 0, 713, self.W_SCREEN, 55, PanelStyles.PANEL_STYLE_BOTTOMBAR )
		screen.showWindowBackground(False)
		screen.setText(self.EXIT_ID, "Background", self.EXIT_TEXT, CvUtil.FONT_RIGHT_JUSTIFY, self.X_EXIT, self.Y_EXIT, self.Z_CONTROLS, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_CLOSE_SCREEN, -1, -1 )
												
		# Header...
		self.szHeader = self.getNextWidgetName()
		screen.setText(self.szHeader, "Background", self.TITLE, CvUtil.FONT_CENTER_JUSTIFY, self.X_SCREEN, self.Y_TITLE, self.Z_CONTROLS, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)

		# Minimap initialization
		self.H_MAP = (self.W_MAP * CyMap().getGridHeight()) / CyMap().getGridWidth()
		if (self.H_MAP > self.H_MAP_MAX):
			self.W_MAP = (self.H_MAP_MAX * CyMap().getGridWidth()) / CyMap().getGridHeight()
			self.H_MAP = self.H_MAP_MAX
		screen.addPanel("", u"", "", False, False, self.X_MAP, self.Y_MAP, self.W_MAP, self.H_MAP, PanelStyles.PANEL_STYLE_MAIN)
		screen.initMinimap(self.X_MAP + self.MAP_MARGIN, self.X_MAP + self.W_MAP - self.MAP_MARGIN, self.Y_MAP + self.MAP_MARGIN, self.Y_MAP + self.H_MAP - self.MAP_MARGIN, self.Z_CONTROLS)
		screen.updateMinimapSection(False)

		screen.updateMinimapColorFromMap(MinimapModeTypes.MINIMAPMODE_TERRITORY, 0.3)

		screen.setMinimapMode(MinimapModeTypes.MINIMAPMODE_MILITARY)
		
		iOldMode = CyInterface().getShowInterface()
		CyInterface().setShowInterface(InterfaceVisibility.INTERFACE_MINIMAP_ONLY)
		screen.updateMinimapVisibility()
		CyInterface().setShowInterface(iOldMode)
					
		self.iActivePlayer = gc.getGame().getActivePlayer()
		
		self.refresh()
		
																									
	# returns a unique ID for a widget in this screen
	def getNextWidgetName(self):
		szName = self.WIDGET_ID + str(self.nWidgetCount)
		self.nWidgetCount += 1
		return szName
																	
	def resetMinimapColor(self):
		screen = self.getScreen()
		for iX in range(gc.getMap().getGridWidth()):
			for iY in range(gc.getMap().getGridHeight()):
				screen.setMinimapColor(MinimapModeTypes.MINIMAPMODE_MILITARY, iX, iY, -1, 0.6)
																				
	# handle the input for this screen...
	def handleInput (self, inputClass):
		return 0

	def update(self, fDelta):
		screen = self.getScreen()
		screen.updateMinimap(fDelta)

	def minimapClicked(self):
		self.hideScreen()
						
	def isSelectedGroup(self, iGroup):
		if -1 in self.selectedUnitList:
			return True
		return ((iGroup + gc.getNumUnitInfos()) in self.selectedUnitList)
				
	def isSelectedUnit(self, iUnit):
		if -1 in self.selectedUnitList:
			return True
		if self.isSelectedGroup(gc.getUnitInfo(iUnit).getUnitCombatType()):
			return True
		return (iUnit in self.selectedUnitList)
		
	def refreshSelectedLeader(self, iPlayer):
		if (iPlayer in self.selectedPlayerList):
			self.selectedPlayerList.remove(iPlayer)
		else:
			self.selectedPlayerList.append(iPlayer)
		self.refresh()
				
	def getLeaderButton(self, iPlayer):
		szName = self.LEADER_BUTTON_ID + str(iPlayer)
		return szName

	def refreshSelected(self, iSelected):
		if (iSelected in self.selectedUnitList):
			self.selectedUnitList.remove(iSelected)
		else:
			self.selectedUnitList.append(iSelected)
		self.refreshUnitSelection()
			
	
	def refreshUnitSelection(self):
		screen = self.getScreen()
		
		screen.minimapClearAllFlashingTiles()

		unitsList = [(0, 0, 0, 0, 0)] * gc.getNumUnitInfos()
		for iUnit in range(gc.getNumUnitInfos()):
			unitsList[iUnit] = (gc.getUnitInfo(iUnit).getUnitCombatType(), 0, iUnit, -1, 0)

		for iPlayer in range(gc.getMAX_PLAYERS()):			
			player = PyPlayer(iPlayer)
			if (player.isAlive()):
				unitList = player.getUnitList()
				for loopUnit in unitList:
					unitType = loopUnit.getUnitType()
					
					bVisible = False
					plot = loopUnit.plot()
					if (not plot.isNone()):
						bVisible = plot.isVisible(gc.getPlayer(self.iActivePlayer).getTeam(), False)

					if unitType > 0 and unitType < gc.getNumUnitInfos() and bVisible:
						iAllUnits = unitsList[unitType][4] + 1
						if (iPlayer == self.iActivePlayer):
							iOwnUnits = unitsList[unitType][1] + 1
						else:
							iOwnUnits = unitsList[unitType][1]
						
						unitsList[unitType] = (unitsList[unitType][0], iOwnUnits, unitsList[unitType][2], iPlayer, iAllUnits)
						iColor = gc.getPlayerColorInfo(gc.getPlayer(iPlayer).getPlayerColor()).getColorTypePrimary()
						screen.setMinimapColor(MinimapModeTypes.MINIMAPMODE_MILITARY, loopUnit.getX(), loopUnit.getY(), iColor, 0.6)
						if (self.isSelectedUnit(unitType) and (iPlayer not in self.selectedPlayerList)):
							
							if (player.getTeam().isAtWar(gc.getPlayer(self.iActivePlayer).getTeam())):
								iColor = gc.getInfoTypeForString("COLOR_RED")
							elif (gc.getPlayer(iPlayer).getTeam() != gc.getPlayer(self.iActivePlayer).getTeam()):
								iColor = gc.getInfoTypeForString("COLOR_YELLOW")
							else:
								iColor = gc.getInfoTypeForString("COLOR_WHITE")
							screen.minimapFlashPlot(loopUnit.getX(), loopUnit.getY(), iColor, -1)

		# sort by unit combat type
		unitsList.sort()
		
		screen.addListBoxGFC(self.UNIT_LIST_ID, "", self.X_TEXT+self.MAP_MARGIN, self.Y_TEXT+self.MAP_MARGIN, self.W_TEXT-2*self.MAP_MARGIN, self.H_TEXT-2*self.MAP_MARGIN, TableStyles.TABLE_STYLE_STANDARD)
		screen.enableSelect(self.UNIT_LIST_ID, False)
		szText = localText.getText("TXT_KEY_PEDIA_ALL_UNITS", ()).upper()
		if (-1 in self.selectedUnitList):
			szText = localText.changeTextColor(szText, gc.getInfoTypeForString("COLOR_YELLOW"))
		screen.appendListBoxString(self.UNIT_LIST_ID, szText, WidgetTypes.WIDGET_MINIMAP_HIGHLIGHT, 0, -1, CvUtil.FONT_LEFT_JUSTIFY)

		iPrevUnitCombat = -2
		for iUnit in range(gc.getNumUnitInfos()):
			if (unitsList[iUnit][4] > 0):
				if (iPrevUnitCombat != unitsList[iUnit][0] and unitsList[iUnit][0] != -1):
					iPrevUnitCombat = unitsList[iUnit][0]
					szDescription = u"   " + gc.getUnitCombatInfo(unitsList[iUnit][0]).getDescription().upper()
					if (self.isSelectedGroup(unitsList[iUnit][0])):
						szDescription = localText.changeTextColor(szDescription, gc.getInfoTypeForString("COLOR_YELLOW"))
					screen.appendListBoxString(self.UNIT_LIST_ID, szDescription, WidgetTypes.WIDGET_MINIMAP_HIGHLIGHT, 0, unitsList[iUnit][0] + gc.getNumUnitInfos(), CvUtil.FONT_LEFT_JUSTIFY)
				
				szDescription = u"      " + gc.getUnitInfo(unitsList[iUnit][2]).getDescription() + u" (" + unicode(unitsList[iUnit][1]) + u")"
				if (self.isSelectedUnit(unitsList[iUnit][2])):
					szDescription = localText.changeTextColor(szDescription, gc.getInfoTypeForString("COLOR_YELLOW"))
				screen.appendListBoxString(self.UNIT_LIST_ID, szDescription, WidgetTypes.WIDGET_MINIMAP_HIGHLIGHT, 0, unitsList[iUnit][2], CvUtil.FONT_LEFT_JUSTIFY)
	
	def refresh(self):
	
		if (self.iActivePlayer < 0):
			return
						
		screen = self.getScreen()
				
		# Set scrollable area for unit buttons
		screen.addPanel(self.UNIT_PANEL_ID, "", "", True, True, self.X_TEXT, self.Y_TEXT, self.W_TEXT, self.H_TEXT, PanelStyles.PANEL_STYLE_MAIN)
		
		# Set scrollable area for leaders
		screen.addPanel(self.LEADER_PANEL_ID, "", "", False, True, self.X_LEADERS, self.Y_LEADERS, self.W_LEADERS, self.H_LEADERS, PanelStyles.PANEL_STYLE_MAIN)

		listLeaders = []
		for iLoopPlayer in range(gc.getMAX_PLAYERS()):
			player = gc.getPlayer(iLoopPlayer)
			if (player.isAlive() and (gc.getTeam(player.getTeam()).isHasMet(gc.getPlayer(self.iActivePlayer).getTeam()) or gc.getGame().isDebugMode()) and not player.isBarbarian() and not player.isMinorCiv()):
				listLeaders.append(iLoopPlayer)
				
		iNumLeaders = len(listLeaders)
		if iNumLeaders >= self.LEADER_COLUMNS:
			iButtonSize = self.LEADER_BUTTON_SIZE / 2
		else:
			iButtonSize = self.LEADER_BUTTON_SIZE

		iColumns = int(self.W_LEADERS / (iButtonSize + self.LEADER_MARGIN))

		# loop through all players and display leaderheads
		for iIndex in range(iNumLeaders):
			iLoopPlayer = listLeaders[iIndex]
			player = gc.getPlayer(iLoopPlayer)
			
			x = self.X_LEADERS + self.LEADER_MARGIN + (iIndex % iColumns) * (iButtonSize + self.LEADER_MARGIN)
			y = self.Y_LEADERS + self.LEADER_MARGIN + (iIndex // iColumns) * (iButtonSize + self.LEADER_MARGIN)

			screen.addCheckBoxGFC(self.getLeaderButton(iLoopPlayer), gc.getLeaderHeadInfo(gc.getPlayer(iLoopPlayer).getLeaderType()).getButton(), ArtFileMgr.getInterfaceArtInfo("BUTTON_HILITE_SQUARE").getPath(), x, y, iButtonSize, iButtonSize, WidgetTypes.WIDGET_MINIMAP_HIGHLIGHT, 1, iLoopPlayer, ButtonStyles.BUTTON_STYLE_LABEL)
			screen.setState(self.getLeaderButton(iLoopPlayer), (iLoopPlayer not in self.selectedPlayerList))				
		
		self.refreshUnitSelection()
		
		
		
		