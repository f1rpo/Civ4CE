## Sid Meier's Civilization 4
## Copyright Firaxis Games 2005
from CvPythonExtensions import *
import CvUtil
import ScreenInput
import CvScreenEnums
import string

# globals
gc = CyGlobalContext()
ArtFileMgr = CyArtFileMgr()
localText = CyTranslator()

class CvPediaBuilding:
	"Civilopedia Screen for Buildings"
	
	def __init__(self, main):
		self.iBuilding = -1
		self.top = main

		self.BUTTON_SIZE = 46
		
		self.X_BUILDING_PANE = 20
		self.Y_BUILDING_PANE = 70
		self.W_BUILDING_PANE = 433
		self.H_BUILDING_PANE = 210
		
		self.X_BUILDING_ANIMATION = 475
		self.Y_BUILDING_ANIMATION = 78
		self.W_BUILDING_ANIMATION = 303
		self.H_BUILDING_ANIMATION = 200
		self.X_ROTATION_BUILDING_ANIMATION = -20
		self.Z_ROTATION_BUILDING_ANIMATION = 30
		self.SCALE_ANIMATION = 1.0
                
		self.X_STATS_PANE = 210
		self.Y_STATS_PANE = 120
		self.W_STATS_PANE = 250
		self.H_STATS_PANE = 200
                
		self.X_ICON = 48
		self.Y_ICON = 105
		self.W_ICON = 150
		self.H_ICON = 150
		self.ICON_SIZE = 64

		self.X_PREREQ_PANE = 20
		self.Y_PREREQ_PANE = 292
		self.W_PREREQ_PANE = 433
		self.H_PREREQ_PANE = 124

		self.X_SPECIAL_PANE = 20
		self.Y_SPECIAL_PANE = 420
		self.W_SPECIAL_PANE = 433
		self.H_SPECIAL_PANE = 278
		
		self.X_HISTORY_PANE = 473
		self.Y_HISTORY_PANE = 292
		self.W_HISTORY_PANE = 304
		self.H_HISTORY_PANE = 407
		
	# Screen construction function
	def interfaceScreen(self, iBuilding):	
			
		self.iBuilding = iBuilding
	
		self.top.deleteAllWidgets()						
							
		screen = self.top.getScreen()
		
		bNotActive = (not screen.isActive())
		if bNotActive:
			self.top.setPediaCommonWidgets()
		
		# Header...
		szHeader = u"<font=4b>" + gc.getBuildingInfo(self.iBuilding).getDescription().upper() + u"</font>"
		szHeaderId = self.top.getNextWidgetName()
		screen.setLabel(szHeaderId, "Background", szHeader, CvUtil.FONT_CENTER_JUSTIFY, self.top.X_SCREEN, self.top.Y_TITLE, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, CivilopediaPageTypes.CIVILOPEDIA_PAGE_BUILDING, iBuilding)
		
		# Top
		screen.setText(self.top.getNextWidgetName(), "Background", self.top.MENU_TEXT, CvUtil.FONT_LEFT_JUSTIFY, self.top.X_MENU, self.top.Y_MENU, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_PEDIA_MAIN, CivilopediaPageTypes.CIVILOPEDIA_PAGE_BUILDING, -1)

		if self.top.iLastScreen	!= CvScreenEnums.PEDIA_BUILDING or bNotActive:		
			self.placeLinks()
			self.top.iLastScreen = CvScreenEnums.PEDIA_BUILDING
					
		screen.addPanel( self.top.getNextWidgetName(), "", "", False, False, self.X_BUILDING_PANE, self.Y_BUILDING_PANE, self.W_BUILDING_PANE, self.H_BUILDING_PANE, PanelStyles.PANEL_STYLE_BLUE50)
		
		# Icon
		screen.addPanel(self.top.getNextWidgetName(), "", "", false, false, self.X_ICON, self.Y_ICON, self.W_ICON, self.H_ICON, PanelStyles.PANEL_STYLE_MAIN)
		screen.addDDSGFC(self.top.getNextWidgetName(), gc.getBuildingInfo(self.iBuilding).getButton(), self.X_ICON + self.W_ICON/2 - self.ICON_SIZE/2, self.Y_ICON + self.H_ICON/2 - self.ICON_SIZE/2, self.ICON_SIZE, self.ICON_SIZE, WidgetTypes.WIDGET_GENERAL, -1, -1 )

		# Unit animation
		screen.addBuildingGraphicGFC(self.top.getNextWidgetName(), self.iBuilding, self.X_BUILDING_ANIMATION, self.Y_BUILDING_ANIMATION, self.W_BUILDING_ANIMATION, self.H_BUILDING_ANIMATION, WidgetTypes.WIDGET_GENERAL, -1, -1, self.X_ROTATION_BUILDING_ANIMATION, self.Z_ROTATION_BUILDING_ANIMATION, self.SCALE_ANIMATION, True)

		self.placeStats()
		
		self.placeRequires()

		self.placeSpecial()
		
		self.placeHistory()

			
	# Place happiness/health/commerce/great people modifiers
	def placeStats(self):

		screen = self.top.getScreen()
				
		buildingInfo = gc.getBuildingInfo(self.iBuilding)
		
		panelName = self.top.getNextWidgetName()

		screen.addListBoxGFC(panelName, "", self.X_STATS_PANE, self.Y_STATS_PANE, self.W_STATS_PANE, self.H_STATS_PANE, TableStyles.TABLE_STYLE_EMPTY)
		screen.enableSelect(panelName, False)
                
		if (isWorldWonderClass(gc.getBuildingInfo(self.iBuilding).getBuildingClassType())):
			iMaxInstances = gc.getBuildingClassInfo(gc.getBuildingInfo(self.iBuilding).getBuildingClassType()).getMaxGlobalInstances()
			szBuildingType = localText.getText("TXT_KEY_PEDIA_WORLD_WONDER", ())
			if (iMaxInstances > 1):
				szBuildingType += " " + localText.getText("TXT_KEY_PEDIA_WONDER_INSTANCES", (iMaxInstances,))
				screen.appendListBoxString(panelName, u"<font=4>" + szBuildingType.upper() + u"</font>", WidgetTypes.WIDGET_GENERAL, 0, 0, CvUtil.FONT_LEFT_JUSTIFY)

		if (isTeamWonderClass(gc.getBuildingInfo(self.iBuilding).getBuildingClassType())):
			iMaxInstances = gc.getBuildingClassInfo(gc.getBuildingInfo(self.iBuilding).getBuildingClassType()).getMaxTeamInstances()
			szBuildingType = localText.getText("TXT_KEY_PEDIA_TEAM_WONDER", ())
			if (iMaxInstances > 1):
				szBuildingType += " " + localText.getText("TXT_KEY_PEDIA_WONDER_INSTANCES", (iMaxInstances,))
				screen.appendListBoxString(panelName, u"<font=4>" + szBuildingType.upper() + u"</font>", WidgetTypes.WIDGET_GENERAL, 0, 0, CvUtil.FONT_LEFT_JUSTIFY)

		if (isNationalWonderClass(gc.getBuildingInfo(self.iBuilding).getBuildingClassType())):
			iMaxInstances = gc.getBuildingClassInfo(gc.getBuildingInfo(self.iBuilding).getBuildingClassType()).getMaxPlayerInstances()
			szBuildingType = localText.getText("TXT_KEY_PEDIA_NATIONAL_WONDER", ())
			if (iMaxInstances > 1):
				szBuildingType += " " + localText.getText("TXT_KEY_PEDIA_WONDER_INSTANCES", (iMaxInstances,))
				screen.appendListBoxString(panelName, u"<font=4>" + szBuildingType.upper() + u"</font>", WidgetTypes.WIDGET_GENERAL, 0, 0, CvUtil.FONT_LEFT_JUSTIFY)

		if (buildingInfo.getProductionCost() > 0):
			if self.top.iActivePlayer == -1:
				szCost = localText.getText("TXT_KEY_PEDIA_COST", ((buildingInfo.getProductionCost() * gc.getDefineINT("PROJECT_PRODUCTION_PERCENT"))/100,))
			else:
				szCost = localText.getText("TXT_KEY_PEDIA_COST", (gc.getPlayer(self.top.iActivePlayer).getBuildingProductionNeeded(self.iBuilding),))
			screen.appendListBoxString(panelName, u"<font=4>" + szCost.upper() + u"%c" % gc.getYieldInfo(YieldTypes.YIELD_PRODUCTION).getChar() + u"</font>", WidgetTypes.WIDGET_GENERAL, 0, 0, CvUtil.FONT_LEFT_JUSTIFY)

		for k in range(CommerceTypes.NUM_COMMERCE_TYPES):
			if (buildingInfo.getObsoleteSafeCommerceChange(k) != 0):
				if (buildingInfo.getObsoleteSafeCommerceChange(k) > 0):
					szSign = "+"
				else:
					szSign = ""
					
				szCommerce = gc.getCommerceInfo(k).getDescription() + ": "
				
				szText1 = szCommerce.upper() + szSign + str(buildingInfo.getObsoleteSafeCommerceChange(k))
				szText2 = szText1 + (u"%c" % (gc.getCommerceInfo(k).getChar()))
				screen.appendListBoxString(panelName, u"<font=4>" + szText2 + u"</font>", WidgetTypes.WIDGET_GENERAL, 0, 0, CvUtil.FONT_LEFT_JUSTIFY)

		if (buildingInfo.getHappiness() > 0):
			szText = localText.getText("TXT_KEY_PEDIA_HAPPY", (buildingInfo.getHappiness(),)).upper()
			screen.appendListBoxString(panelName, u"<font=4>" + szText + u"%c" % CyGame().getSymbolID(FontSymbols.HAPPY_CHAR) + u"</font>", WidgetTypes.WIDGET_GENERAL, 0, 0, CvUtil.FONT_LEFT_JUSTIFY)
			
		elif (buildingInfo.getHappiness() < 0):
			szText = localText.getText("TXT_KEY_PEDIA_UNHAPPY", (-buildingInfo.getHappiness(),)).upper()
			screen.appendListBoxString(panelName, u"<font=4>" + szText + u"%c" % CyGame().getSymbolID(FontSymbols.UNHAPPY_CHAR) + u"</font>", WidgetTypes.WIDGET_GENERAL, 0, 0, CvUtil.FONT_LEFT_JUSTIFY)

		if (buildingInfo.getHealth() > 0):
			szText = localText.getText("TXT_KEY_PEDIA_HEALTHY", (buildingInfo.getHealth(),)).upper()
			screen.appendListBoxString(panelName, u"<font=4>" + szText + u"%c" % CyGame().getSymbolID(FontSymbols.HEALTHY_CHAR) + u"</font>", WidgetTypes.WIDGET_GENERAL, 0, 0, CvUtil.FONT_LEFT_JUSTIFY)
			
		elif (buildingInfo.getHealth() < 0):
			szText = localText.getText("TXT_KEY_PEDIA_UNHEALTHY", (-buildingInfo.getHealth(),)).upper()
			screen.appendListBoxString(panelName, u"<font=4>" + szText + u"%c" % CyGame().getSymbolID(FontSymbols.UNHEALTHY_CHAR) + u"</font>", WidgetTypes.WIDGET_GENERAL, 0, 0, CvUtil.FONT_LEFT_JUSTIFY)
		
		if (buildingInfo.getGreatPeopleRateChange() != 0):
			szText = localText.getText("TXT_KEY_PEDIA_GREAT_PEOPLE", (buildingInfo.getGreatPeopleRateChange(),)).upper()
			screen.appendListBoxString(panelName, u"<font=4>" + szText + u"%c" % CyGame().getSymbolID(FontSymbols.GREAT_PEOPLE_CHAR) + u"</font>", WidgetTypes.WIDGET_GENERAL, 0, 0, CvUtil.FONT_LEFT_JUSTIFY)
			
	# Place prereqs (techs, resources)
	def placeRequires(self):

		screen = self.top.getScreen()
		
		panelName = self.top.getNextWidgetName()
		screen.addPanel( panelName, localText.getText("TXT_KEY_PEDIA_REQUIRES", ()), "", false, true,
                                 self.X_PREREQ_PANE, self.Y_PREREQ_PANE, self.W_PREREQ_PANE, self.H_PREREQ_PANE, PanelStyles.PANEL_STYLE_BLUE50 )
						
		screen.attachLabel(panelName, "", "  ")

		# add tech buttons
		for iPrereq in range(gc.getNumTechInfos()):
			if isTechRequiredForBuilding(iPrereq, self.iBuilding):
				screen.attachImageButton( panelName, "", gc.getTechInfo(iPrereq).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_TECH, iPrereq, 1, False )

		# add resource buttons
		iPrereq = gc.getBuildingInfo(self.iBuilding).getPrereqAndBonus()
		if (iPrereq >= 0):
			screen.attachImageButton( panelName, "", gc.getBonusInfo(iPrereq).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_BONUS, iPrereq, -1, False )
			
		for k in range(gc.getNUM_BUILDING_PREREQ_OR_BONUSES()):
			iPrereq = gc.getBuildingInfo(self.iBuilding).getPrereqOrBonuses(k)
			if (iPrereq >= 0):
				screen.attachImageButton( panelName, "", gc.getBonusInfo(iPrereq).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_BONUS, iPrereq, -1, False )
				
		# add religion button
		iPrereq = gc.getBuildingInfo(self.iBuilding).getPrereqReligion()
		if (iPrereq >= 0):
			screen.attachImageButton( panelName, "", gc.getReligionInfo(iPrereq).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_RELIGION, iPrereq, -1, False )
		
	# Place Special abilities
	def placeSpecial(self):

		screen = self.top.getScreen()
		
		panelName = self.top.getNextWidgetName()
		screen.addPanel( panelName, localText.getText("TXT_KEY_PEDIA_SPECIAL_ABILITIES", ()), "", true, false,
                                 self.X_SPECIAL_PANE, self.Y_SPECIAL_PANE, self.W_SPECIAL_PANE, self.H_SPECIAL_PANE, PanelStyles.PANEL_STYLE_BLUE50 )
		
		listName = self.top.getNextWidgetName()
		screen.attachListBoxGFC( panelName, listName, "", TableStyles.TABLE_STYLE_EMPTY )
		screen.enableSelect(listName, False)
		
		szSpecialText = CyGameTextMgr().getBuildingHelp(self.iBuilding, True, False, False, None)
		splitText = string.split( szSpecialText, "\n" )
		for special in splitText:
			if len( special ) != 0:
				screen.appendListBoxString( listName, special, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )

	def placeHistory(self):
		
		screen = self.top.getScreen()
		
		panelName = self.top.getNextWidgetName()
		screen.addPanel( panelName, "History", "", True, True,
						self.X_HISTORY_PANE, self.Y_HISTORY_PANE,
						self.W_HISTORY_PANE, self.H_HISTORY_PANE,
						PanelStyles.PANEL_STYLE_BLUE50 )
		
		textName = self.top.getNextWidgetName()
#		screen.attachMultilineText( panelName, textName, gc.getBuildingInfo(self.iBuilding).getCivilopedia(),WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.addMultilineText( textName, gc.getBuildingInfo(self.iBuilding).getCivilopedia(), self.X_HISTORY_PANE + 15, self.Y_HISTORY_PANE + 40,
		    self.W_HISTORY_PANE - (15 * 2), self.H_HISTORY_PANE - (15 * 2) - 25, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		
	def placeLinks(self):

		screen = self.top.getScreen()
                		
		screen.clearListBoxGFC(self.top.LIST_ID)

		listSorted = self.getBuildingSortedList()
				
		iSelected = 0
		i = 0
		for iI in range(len(listSorted)):
			if (not gc.getBuildingInfo(listSorted[iI][1]).isGraphicalOnly()):
				screen.appendListBoxString(self.top.LIST_ID, listSorted[iI][0], WidgetTypes.WIDGET_PEDIA_JUMP_TO_BUILDING, listSorted[iI][1], 0, CvUtil.FONT_LEFT_JUSTIFY)
				if listSorted[iI][1] == self.iBuilding:
					iSelected = i
				i += 1			

		screen.setSelectedListBoxStringGFC(self.top.LIST_ID, iSelected)
			
	def getBuildingType(self, iBuilding):
		if (isWorldWonderClass(gc.getBuildingInfo(iBuilding).getBuildingClassType())):
			return (3)			

		if (isTeamWonderClass(gc.getBuildingInfo(iBuilding).getBuildingClassType())):
			return (2)

		if (isNationalWonderClass(gc.getBuildingInfo(iBuilding).getBuildingClassType())):
			return (1)

		# Regular building
		return (0)
		
	def getBuildingSortedList(self):
		listOfAllTypes = []
		for iBuildingType in range(4):		
			listBuildings = []
			iCount = 0
			for iBuilding in range(gc.getNumBuildingInfos()):
				if (self.getBuildingType(iBuilding) == iBuildingType):
					listBuildings.append(iBuilding)
					iCount += 1
			
			listSorted = [(0,0)] * iCount
			iI = 0
			for iBuilding in listBuildings:
				listSorted[iI] = (gc.getBuildingInfo(iBuilding).getDescription(), iBuilding)
				iI += 1
			listSorted.sort()
			for i in range(len(listSorted)):
				listOfAllTypes.append(listSorted[i])
		return listOfAllTypes
		
													
	# Will handle the input for this screen...
	def handleInput (self, inputClass):
		return 0

