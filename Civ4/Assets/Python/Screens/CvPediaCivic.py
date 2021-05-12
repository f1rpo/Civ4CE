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

class CvPediaCivic:
	"Civilopedia Screen for Civics"

	def __init__(self, main):
		self.iCivic = -1
		self.top = main
		
		self.X_MAIN_PANE = 45
		self.Y_MAIN_PANE = 85
		self.W_MAIN_PANE = 433
		self.H_MAIN_PANE = 210

		self.X_ICON = 78
		self.Y_ICON = 120
		self.W_ICON = 150
		self.H_ICON = 150
		self.ICON_SIZE = 64

		self.X_STATS_PANE = self.X_ICON + self.W_ICON + 20
		self.Y_STATS_PANE = 165
		self.W_STATS_PANE = 250
		self.H_STATS_PANE = 200

		self.X_REQUIRES = self.X_MAIN_PANE + self.W_MAIN_PANE + 20
		self.Y_REQUIRES = 185
		self.W_REQUIRES = 250
		self.H_REQUIRES = 110

		self.X_SPECIAL = self.X_MAIN_PANE
		self.Y_SPECIAL = self.Y_MAIN_PANE + self.H_MAIN_PANE + 10
		self.W_SPECIAL = 705
		self.H_SPECIAL = 130

		self.X_TEXT = self.X_MAIN_PANE
		self.Y_TEXT = self.Y_SPECIAL + self.H_SPECIAL + 20
		self.W_TEXT = 705
		self.H_TEXT = 220
		
	# Screen construction function
	def interfaceScreen(self, iCivic):	
			
		self.iCivic = iCivic
	
		self.top.deleteAllWidgets()						
							
		screen = self.top.getScreen()
		
		bNotActive = (not screen.isActive())
		if bNotActive:
			self.top.setPediaCommonWidgets()

		# Header...
		szHeader = u"<font=4b>" + gc.getCivicInfo(self.iCivic).getDescription().upper() + u"</font>"
		szHeaderId = self.top.getNextWidgetName()
		screen.setLabel(szHeaderId, "Background", szHeader, CvUtil.FONT_CENTER_JUSTIFY, self.top.X_SCREEN, self.top.Y_TITLE, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
		
		# Top
		screen.setText(self.top.getNextWidgetName(), "Background", self.top.MENU_TEXT, CvUtil.FONT_LEFT_JUSTIFY, self.top.X_MENU, self.top.Y_MENU, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_PEDIA_MAIN, CivilopediaPageTypes.CIVILOPEDIA_PAGE_CIVIC, -1)

		if self.top.iLastScreen	!= CvScreenEnums.PEDIA_CIVIC or bNotActive:		
			self.placeLinks(true)
			self.top.iLastScreen = CvScreenEnums.PEDIA_CIVIC
		else:
			self.placeLinks(false)

			
		# Icon
		screen.addPanel( self.top.getNextWidgetName(), "", "", False, False,
		    self.X_MAIN_PANE, self.Y_MAIN_PANE, self.W_MAIN_PANE, self.H_MAIN_PANE, PanelStyles.PANEL_STYLE_BLUE50)
		screen.addPanel(self.top.getNextWidgetName(), "", "", false, false,
		    self.X_ICON, self.Y_ICON, self.W_ICON, self.H_ICON, PanelStyles.PANEL_STYLE_MAIN)
		screen.addDDSGFC(self.top.getNextWidgetName(), gc.getCivicInfo(self.iCivic).getButton(),
		    self.X_ICON + self.W_ICON/2 - self.ICON_SIZE/2, self.Y_ICON + self.H_ICON/2 - self.ICON_SIZE/2, self.ICON_SIZE, self.ICON_SIZE, WidgetTypes.WIDGET_GENERAL, -1, -1 )
#		screen.addDDSGFC(self.top.getNextWidgetName(), gc.getCivicInfo(self.iCivic).getButton(), self.X_ICON, self.Y_ICON, self.W_ICON, self.H_ICON, WidgetTypes.WIDGET_GENERAL, self.iCivic, -1 )
		
		self.placeStats()
		self.placeSpecial()
		self.placeRequires()
		self.placeText()
		
	def placeStats(self):
							
		screen = self.top.getScreen()

		panelName = self.top.getNextWidgetName()
		screen.addListBoxGFC(panelName, "",
		    self.X_STATS_PANE, self.Y_STATS_PANE, self.W_STATS_PANE, self.H_STATS_PANE, TableStyles.TABLE_STYLE_EMPTY)
		screen.enableSelect(panelName, False)

		# Civic Category
		iCivicOptionType = gc.getCivicInfo(self.iCivic).getCivicOptionType()
		if (iCivicOptionType != -1):
			screen.appendListBoxString(panelName, u"<font=4>" + gc.getCivicOptionInfo(iCivicOptionType).getDescription().upper() + u"</font>", WidgetTypes.WIDGET_GENERAL, 0, 0, CvUtil.FONT_LEFT_JUSTIFY)
#			screen.setText(self.top.getNextWidgetName(), "Background", gc.getCivicOptionInfo(iCivicOptionType).getDescription().upper(), CvUtil.FONT_LEFT_JUSTIFY, self.X_STATS_PANE, self.Y_STATS_PANE-35, 0, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)

		# Upkeep
		pUpkeepInfo = gc.getUpkeepInfo(gc.getCivicInfo(self.iCivic).getUpkeep())
		if (pUpkeepInfo):
			screen.appendListBoxString(panelName, u"<font=4>" + pUpkeepInfo.getDescription().upper() + u"</font>", WidgetTypes.WIDGET_GENERAL, 0, 0, CvUtil.FONT_LEFT_JUSTIFY)
#			screen.setText(self.top.getNextWidgetName(), "Background", pUpkeepInfo.getDescription().upper(), CvUtil.FONT_LEFT_JUSTIFY, self.X_STATS_PANE, self.Y_STATS_PANE, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		
	def placeRequires(self):
		
		screen = self.top.getScreen()
		
		panelName = self.top.getNextWidgetName()
		screen.addPanel( panelName, localText.getText("TXT_KEY_PEDIA_REQUIRES", ()), "", false, true,
				 self.X_REQUIRES, self.Y_REQUIRES, self.W_REQUIRES, self.H_REQUIRES, PanelStyles.PANEL_STYLE_BLUE50 )
		screen.enableSelect(panelName, False)
		screen.attachLabel(panelName, "", "  ")
		
		iTech = gc.getCivicInfo(self.iCivic).getTechPrereq()
		if (iTech > -1):
			screen.attachImageButton( panelName, "", gc.getTechInfo(iTech).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_TECH, iTech, 1, False )
			
	def placeSpecial(self):
		
		screen = self.top.getScreen()
		
		panelName = self.top.getNextWidgetName()
		screen.addPanel( panelName, localText.getText("TXT_KEY_PEDIA_EFFECTS", ()), "", true, false,
				 self.X_SPECIAL, self.Y_SPECIAL, self.W_SPECIAL, self.H_SPECIAL, PanelStyles.PANEL_STYLE_BLUE50 )
				
		listName = self.top.getNextWidgetName()
		screen.attachListBoxGFC( panelName, listName, "", TableStyles.TABLE_STYLE_EMPTY )
		screen.enableSelect(listName, False)
		
		szSpecialText = CyGameTextMgr().parseCivicInfo(self.iCivic, True, False, True)
		screen.addMultilineText(listName, szSpecialText[1:], self.X_SPECIAL+5, self.Y_SPECIAL+25, self.W_SPECIAL-10, self.H_SPECIAL-30, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)	
		
	def placeText(self):
		
		screen = self.top.getScreen()
		
		panelName = self.top.getNextWidgetName()
		screen.addPanel( panelName, "", "", true, true,
				 self.X_TEXT, self.Y_TEXT, self.W_TEXT, self.H_TEXT, PanelStyles.PANEL_STYLE_BLUE50 )
 
		szText = gc.getCivicInfo(self.iCivic).getCivilopedia()
		screen.attachMultilineText( panelName, "Text", szText, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		
	def placeLinks(self, bRedraw):

		screen = self.top.getScreen()
		
		if bRedraw:                
			screen.clearListBoxGFC(self.top.LIST_ID)
		
		# sort Improvements alphabetically
		listSorted=[(0,0)]*gc.getNumCivicInfos()
		for j in range(gc.getNumCivicInfos()):
			listSorted[j] = (gc.getCivicInfo(j).getDescription(), j)
		listSorted.sort()	

		iSelected = 0			
		i = 0
		for iI in range(gc.getNumCivicInfos()):
			if (not gc.getCivicInfo(listSorted[iI][1]).isGraphicalOnly()):
				if bRedraw:
					screen.appendListBoxString(self.top.LIST_ID, listSorted[iI][0], WidgetTypes.WIDGET_PEDIA_JUMP_TO_CIVIC, listSorted[iI][1], 0, CvUtil.FONT_LEFT_JUSTIFY )
				if listSorted[iI][1] == self.iCivic:
					iSelected = i
				i += 1			

		screen.setSelectedListBoxStringGFC(self.top.LIST_ID, iSelected)
			
	# Will handle the input for this screen...
	def handleInput (self, inputClass):
		return 0


