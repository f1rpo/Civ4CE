## Sid Meier's Civilization 4
## Copyright Firaxis Games 2005
from CvPythonExtensions import *
import CvUtil
import ScreenInput
import CvScreenEnums

# globals
gc = CyGlobalContext()
ArtFileMgr = CyArtFileMgr()
localText = CyTranslator()

class CvPediaUnit:
	"Civilopedia Screen for Units"

	def __init__(self, main):
		self.iUnit = -1
		self.top = main

	# Screen construction function
	def interfaceScreen(self, iUnit, x, y, h, w):

		self.iUnit = iUnit

		self.X_UNIT_PANE = x
		self.Y_UNIT_PANE = y
		self.W_UNIT_PANE = (w * 30 / 100)
		self.H_UNIT_PANE = self.W_UNIT_PANE

		self.X_UNIT_ANIMATION = x + (w * 60 / 100)
		self.Y_UNIT_ANIMATION = y
		self.W_UNIT_ANIMATION = (w * 40 / 100)
		self.H_UNIT_ANIMATION = self.H_UNIT_PANE
		
		self.X_ROTATION_UNIT_ANIMATION = -20
		self.Z_ROTATION_UNIT_ANIMATION = 30
		self.SCALE_ANIMATION = 1.0

		self.W_ICON = self.W_UNIT_PANE * 2 / 3
		self.H_ICON = self.H_UNIT_PANE * 2 / 3
		self.X_ICON = x + (self.W_UNIT_PANE / 2) - (self.W_ICON / 2)
		self.Y_ICON = y + (self.H_UNIT_PANE / 2) - (self.H_ICON / 2)
	
		self.ICON_SIZE = self.W_ICON / 2
		self.W_UNIT_PANE = (w * 55 / 100)

		self.X_STATS_PANE = self.X_ICON + self.W_ICON + (w * 2 / 100)
		self.Y_STATS_PANE = self.Y_ICON
		self.W_STATS_PANE = (w * 35 / 100)
		self.H_STATS_PANE = (h * 30 / 100)

		self.X_SPECIAL_PANE = x
		self.Y_SPECIAL_PANE = y + self.H_UNIT_PANE + (h * 5 / 100)
		self.W_SPECIAL_PANE = (w * 50 / 100)
		self.H_SPECIAL_PANE = h - self.H_UNIT_PANE - (h * 5 / 100)

		self.X_HISTORY_PANE = x + (w * 55 / 100)
		self.Y_HISTORY_PANE = y + self.H_UNIT_ANIMATION + (h * 5 / 100)
		self.W_HISTORY_PANE = (w * 45 / 100)
		self.H_HISTORY_PANE = h - self.H_UNIT_ANIMATION - (h * 5 / 100)

		self.top.deleteAllWidgets()
		screen = self.top.getScreen()

		bNotActive = (not screen.isActive())
		if bNotActive:
			self.top.setPediaCommonWidgets()

		# Header...
		szHeader = u"<font=4b>" + gc.getUnitInfo(self.iUnit).getDescription().upper() + u"</font>"
		screen.setLabel(self.top.getNextWidgetName(), "Background", szHeader, CvUtil.FONT_CENTER_JUSTIFY, self.top.X_SCREEN, self.top.Y_TITLE, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, CivilopediaPageTypes.CIVILOPEDIA_PAGE_UNIT, iUnit)

		# Top
		screen.setText(self.top.getNextWidgetName(), "Background", self.top.MENU_TEXT, CvUtil.FONT_LEFT_JUSTIFY, self.top.X_MENU, self.top.Y_MENU, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_PEDIA_MAIN, CivilopediaPageTypes.CIVILOPEDIA_PAGE_UNIT, -1)
		if self.top.iLastScreen	!= CvScreenEnums.PEDIA_UNIT or bNotActive:
			self.placeLinks(true)
			self.top.iLastScreen = CvScreenEnums.PEDIA_UNIT
		else:
			self.placeLinks(true)

		# Icon
		screen.addPanel( self.top.getNextWidgetName(), "", "", False, False, self.X_UNIT_PANE, self.Y_UNIT_PANE, self.W_UNIT_PANE, self.H_UNIT_PANE, PanelStyles.PANEL_STYLE_BLUE50, WidgetTypes.WIDGET_GENERAL, -1, -1)
		screen.addPanel( self.top.getNextWidgetName(), "", "", false, false, self.X_ICON, self.Y_ICON, self.W_ICON, self.H_ICON, PanelStyles.PANEL_STYLE_MAIN, WidgetTypes.WIDGET_GENERAL, -1, -1)
		szButton = gc.getUnitInfo(self.iUnit).getButton()
		if self.top.iActivePlayer != -1:
			szButton = gc.getPlayer(self.top.iActivePlayer).getUnitButton(self.iUnit)
		screen.addDDSGFC( self.top.getNextWidgetName(), szButton, self.X_ICON + self.W_ICON/2 - self.ICON_SIZE/2, self.Y_ICON + self.H_ICON/2 - self.ICON_SIZE/2, self.ICON_SIZE, self.ICON_SIZE, WidgetTypes.WIDGET_GENERAL, -1, -1 )

		# Unit animation
		screen.addUnitGraphicGFC( self.top.getNextWidgetName(), self.iUnit, -1, self.X_UNIT_ANIMATION, self.Y_UNIT_ANIMATION, self.W_UNIT_ANIMATION, self.H_UNIT_ANIMATION, WidgetTypes.WIDGET_GENERAL, -1, -1, self.X_ROTATION_UNIT_ANIMATION, self.Z_ROTATION_UNIT_ANIMATION, self.SCALE_ANIMATION, True)

		self.placeStats()
		self.placeSpecial()
		self.placeHistory()

	# Place strength/movement
	def placeStats(self):

		screen = self.top.getScreen()

		panelName = self.top.getNextWidgetName()

		# Unit combat group
		iCombatType = gc.getUnitInfo(self.iUnit).getUnitCombatType()
		if (iCombatType != -1):
			#screen.setImageButton(self.top.getNextWidgetName(), gc.getUnitCombatInfo(iCombatType).getButton(), self.X_STATS_PANE, self.Y_STATS_PANE - 40, 32, 32, WidgetTypes.WIDGET_GENERAL, -1, -1)
			#screen.setText(self.top.getNextWidgetName(), "", u"<font=3>" + gc.getUnitCombatInfo(iCombatType).getDescription() + u"</font>", CvUtil.FONT_LEFT_JUSTIFY, self.X_STATS_PANE + 37, self.Y_STATS_PANE - 35, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, 0)
			pass
		
		screen.addListBoxGFC(panelName, "", self.X_STATS_PANE, self.Y_STATS_PANE, self.W_STATS_PANE, self.H_STATS_PANE, TableStyles.TABLE_STYLE_EMPTY)
		screen.enableSelect(panelName, False)

		iStrength = gc.getUnitInfo(self.iUnit).getCombat()

		szName = self.top.getNextWidgetName()
		szStrength = localText.getText("TXT_KEY_PEDIA_STRENGTH", ( iStrength, ) )
		screen.appendListBoxStringNoUpdate(panelName, u"<font=3>" + szStrength.upper() + u"%c" % CyGame().getSymbolID(FontSymbols.STRENGTH_CHAR) + u"</font>", WidgetTypes.WIDGET_GENERAL, 0, 0, CvUtil.FONT_LEFT_JUSTIFY)

		szName = self.top.getNextWidgetName()
		szMovement = localText.getText("TXT_KEY_PEDIA_MOVEMENT", ( gc.getUnitInfo(self.iUnit).getMoves(), ) )
		screen.appendListBoxStringNoUpdate(panelName, u"<font=3>" + szMovement.upper() + u"%c" % CyGame().getSymbolID(FontSymbols.MOVES_CHAR) + u"</font>", WidgetTypes.WIDGET_GENERAL, 0, 0, CvUtil.FONT_LEFT_JUSTIFY)

		for iYield in range(YieldTypes.NUM_YIELD_TYPES):
			if (gc.getUnitInfo(self.iUnit).getYieldCost(iYield) > 0):
				szName = self.top.getNextWidgetName()
				if self.top.iActivePlayer == -1:
					szCost = localText.getText("TXT_KEY_PEDIA_COST", ((gc.getUnitInfo(self.iUnit).getYieldCost(iYield) * gc.getDefineINT("UNIT_PRODUCTION_PERCENT"))/100,))
				else:
					szCost = localText.getText("TXT_KEY_PEDIA_COST", ( gc.getActivePlayer().getUnitYieldProductionNeeded(self.iUnit, iYield), ) )
				screen.appendListBoxStringNoUpdate(panelName, u"<font=2>" + szCost.upper() + u"%c" % gc.getYieldInfo(iYield).getChar() + u"</font>", WidgetTypes.WIDGET_GENERAL, 0, 0, CvUtil.FONT_LEFT_JUSTIFY)

		screen.updateListBox(panelName)

	# Place Special abilities
	def placeSpecial(self):

		screen = self.top.getScreen()

		panelName = self.top.getNextWidgetName()
		screen.addPanel( panelName, localText.getText("TXT_KEY_PEDIA_SPECIAL_ABILITIES", ()), "", true, false, self.X_SPECIAL_PANE, self.Y_SPECIAL_PANE, self.W_SPECIAL_PANE, self.H_SPECIAL_PANE, PanelStyles.PANEL_STYLE_BLUE50, WidgetTypes.WIDGET_GENERAL, -1, -1 )

		listName = self.top.getNextWidgetName()
		szSpecialText = CyGameTextMgr().getUnitHelp( self.iUnit, True, False, None )[1:]
		screen.addMultilineText(listName, szSpecialText, self.X_SPECIAL_PANE + 15, self.Y_SPECIAL_PANE + 40, self.W_SPECIAL_PANE - 30, self.H_SPECIAL_PANE - 60, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)

	def placeHistory(self):

		screen = self.top.getScreen()

		panelName = self.top.getNextWidgetName()
		screen.addPanel( panelName, localText.getText("TXT_KEY_CIVILOPEDIA_HISTORY", ()), "", True, True, self.X_HISTORY_PANE, self.Y_HISTORY_PANE, self.W_HISTORY_PANE, self.H_HISTORY_PANE, PanelStyles.PANEL_STYLE_BLUE50, WidgetTypes.WIDGET_GENERAL, -1, -1 )

		textName = self.top.getNextWidgetName()
		szText = u""
		if len(gc.getUnitInfo(self.iUnit).getStrategy()) > 0:
			szText += localText.getText("TXT_KEY_CIVILOPEDIA_STRATEGY", ())
			szText += gc.getUnitInfo(self.iUnit).getStrategy()
			szText += u"\n\n"
		szText += localText.getText("TXT_KEY_CIVILOPEDIA_BACKGROUND", ())
		szText += gc.getUnitInfo(self.iUnit).getCivilopedia()
		screen.addMultilineText( textName, szText, self.X_HISTORY_PANE + 15, self.Y_HISTORY_PANE + 40,
		    self.W_HISTORY_PANE - (15 * 2), self.H_HISTORY_PANE - (15 * 2) - 25, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)

	def placeLinks(self, bRedraw):

		screen = self.top.getScreen()

		if bRedraw:
			screen.clearListBoxGFC(self.top.LIST_ID)

		# sort Units alphabetically
		unitsList=[(0,0)]*gc.getNumUnitInfos()
		for j in range(gc.getNumUnitInfos()):
			unitsList[j] = (gc.getUnitInfo(j).getDescription(), j)
		unitsList.sort()

		i = 0
		iSelected = 0
		for iI in range(gc.getNumUnitInfos()):
			if (not gc.getUnitInfo(unitsList[iI][1]).isGraphicalOnly()):
				if (not gc.getDefineINT("CIVILOPEDIA_SHOW_ACTIVE_CIVS_ONLY") or not gc.getGame().isFinalInitialized() or gc.getGame().isUnitEverActive(unitsList[iI][1])):
					if bRedraw:
						screen.appendListBoxStringNoUpdate( self.top.LIST_ID, unitsList[iI][0], WidgetTypes.WIDGET_PEDIA_JUMP_TO_UNIT, unitsList[iI][1], 0, CvUtil.FONT_LEFT_JUSTIFY )
					if unitsList[iI][1] == self.iUnit:
						iSelected = i
					i += 1

		if bRedraw:
			screen.updateListBox(self.top.LIST_ID)

		screen.setSelectedListBoxStringGFC(self.top.LIST_ID, iSelected)

	# Will handle the input for this screen...
	def handleInput (self, inputClass):
		return 0


