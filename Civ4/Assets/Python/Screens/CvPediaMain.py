## Sid Meier's Civilization 4
## Copyright Firaxis Games 2005
from CvPythonExtensions import *
import string
import CvUtil
import ScreenInput
import CvScreenEnums
import CvPediaScreen		# base class
import CvPediaTech
import CvPediaUnit
import CvPediaBuilding
import CvPediaPromotion
import CvPediaUnitChart
import CvPediaBonus
import CvPediaTerrain
import CvPediaFeature
import CvPediaImprovement
import CvPediaCivic
import CvPediaCivilization
import CvPediaLeader
import CvPediaSpecialist
import CvPediaHistory
import CvPediaProject
import CvPediaReligion

# globals
gc = CyGlobalContext()
ArtFileMgr = CyArtFileMgr()
localText = CyTranslator()

class CvPediaMain( CvPediaScreen.CvPediaScreen ):
	"Civilopedia Main Screen"

	def __init__(self):
	
		self.PEDIA_MAIN_SCREEN_NAME = "PediaMainScreen"
		self.INTERFACE_ART_INFO = "SCREEN_BG_OPAQUE"

		self.WIDGET_ID = "PediaMainWidget"
		self.EXIT_ID = "PediaMainExitWidget"
		self.BACKGROUND_ID = "PediaMainBackground"
		self.TOP_PANEL_ID = "PediaMainTopPanel"
		self.BOTTOM_PANEL_ID = "PediaMainBottomPanel"
		self.BACK_ID = "PediaMainBack"
		self.NEXT_ID = "PediaMainForward"
		self.TOP_ID = "PediaMainTop"
		self.LIST_ID = "PediaMainList"

		self.X_SCREEN = 500
		self.Y_SCREEN = 396
		self.W_SCREEN = 1024
		self.H_SCREEN = 768
		self.Y_TITLE = 8
		self.DY_TEXT = 45
		
		self.X_EXIT = 925
		self.Y_EXIT = 730
						
		self.X_BACK = 50
		self.Y_BACK = 730

		self.X_FORWARD = 200
		self.Y_FORWARD = 730
		
		self.X_MENU = 450
		self.Y_MENU = 730

		self.BUTTON_SIZE = 64
		self.BUTTON_COLUMNS = 9

		self.X_ITEMS_PANE = 30
		self.Y_ITEMS_PANE = 80
		self.H_ITEMS_PANE = 610
		self.W_ITEMS_PANE = 740
		self.ITEMS_MARGIN = 18
		self.ITEMS_SEPARATION = 2
				
		self.X_LINKS = 797
		self.Y_LINKS = 51
		self.H_LINKS = 665
		self.W_LINKS = 225
		
		self.nWidgetCount = 0
		
		# screen instances
		self.pediaTechScreen = CvPediaTech.CvPediaTech(self)
		self.pediaUnitScreen = CvPediaUnit.CvPediaUnit(self)
		self.pediaBuildingScreen = CvPediaBuilding.CvPediaBuilding(self)
		self.pediaPromotionScreen = CvPediaPromotion.CvPediaPromotion(self)
		self.pediaUnitChart = CvPediaUnitChart.CvPediaUnitChart(self)
		self.pediaBonus = CvPediaBonus.CvPediaBonus(self)
		self.pediaTerrain = CvPediaTerrain.CvPediaTerrain(self)
		self.pediaFeature = CvPediaFeature.CvPediaFeature(self)
		self.pediaImprovement = CvPediaImprovement.CvPediaImprovement(self)
		self.pediaCivic = CvPediaCivic.CvPediaCivic(self)
		self.pediaCivilization = CvPediaCivilization.CvPediaCivilization(self)
		self.pediaLeader = CvPediaLeader.CvPediaLeader(self)
		self.pediaSpecialist = CvPediaSpecialist.CvPediaSpecialist(self)
		self.pediaProjectScreen = CvPediaProject.CvPediaProject(self)
		self.pediaReligion = CvPediaReligion.CvPediaReligion(self)
		self.pediaHistorical = CvPediaHistory.CvPediaHistory(self)
				
		# used for navigating "forward" and "back" in civilopedia
		self.pediaHistory = []
		self.pediaFuture = []
		
		self.listCategories = []

		
		self.iCategory = -1
		self.iLastScreen = -1
		self.iActivePlayer = -1
								
		self.mapCategories = { 
			CivilopediaPageTypes.CIVILOPEDIA_PAGE_TECH	: self.placeTechs, 
			CivilopediaPageTypes.CIVILOPEDIA_PAGE_UNIT	: self.placeUnits, 
			CivilopediaPageTypes.CIVILOPEDIA_PAGE_BUILDING	: self.placeBuildings, 
			}
		
	def getScreen(self):
		return CyGInterfaceScreen(self.PEDIA_MAIN_SCREEN_NAME, CvScreenEnums.PEDIA_MAIN)
		
	def setPediaCommonWidgets(self):
		self.EXIT_TEXT = u"<font=4>" + localText.getText("TXT_KEY_PEDIA_SCREEN_EXIT", ()).upper() + "</font>"
		self.BACK_TEXT = u"<font=4>" + localText.getText("TXT_KEY_PEDIA_SCREEN_BACK", ()).upper() + "</font>"
		self.FORWARD_TEXT = u"<font=4>" + localText.getText("TXT_KEY_PEDIA_SCREEN_FORWARD", ()).upper() + "</font>"
		self.MENU_TEXT = u"<font=4>" + localText.getText("TXT_KEY_PEDIA_SCREEN_TOP", ()).upper() + "</font>"
		
		self.szCategoryTech = localText.getText("TXT_KEY_PEDIA_CATEGORY_TECH", ())		
		self.szCategoryUnit = localText.getText("TXT_KEY_PEDIA_CATEGORY_UNIT", ())		
		self.szCategoryBuilding = localText.getText("TXT_KEY_PEDIA_CATEGORY_BUILDING", ())		
		
		self.listCategories = [ self.szCategoryTech, 
								self.szCategoryUnit, 
								self.szCategoryBuilding]
								
		# Create a new screen
		screen = self.getScreen()
		screen.setRenderInterfaceOnly(True);
		screen.showScreen(PopupStates.POPUPSTATE_IMMEDIATE, False)
		
		# Set background
		screen.addDDSGFC(self.BACKGROUND_ID, ArtFileMgr.getInterfaceArtInfo("SCREEN_BG_OPAQUE").getPath(), 0, 0, self.W_SCREEN, self.H_SCREEN, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.addPanel(self.TOP_PANEL_ID, u"", u"", True, False, 0, 0, self.W_SCREEN, 55, PanelStyles.PANEL_STYLE_TOPBAR )
		screen.addPanel(self.BOTTOM_PANEL_ID, u"", u"", True, False, 0, 713, self.W_SCREEN, 55, PanelStyles.PANEL_STYLE_BOTTOMBAR )
		screen.setDimensions(screen.centerX(0), screen.centerY(0), self.W_SCREEN, self.H_SCREEN)
		
		# Exit button
		screen.setText(self.EXIT_ID, "Background", self.EXIT_TEXT, CvUtil.FONT_LEFT_JUSTIFY, self.X_EXIT, self.Y_EXIT, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_CLOSE_SCREEN, -1, -1)

		# Back
		screen.setText(self.BACK_ID, "Background", self.BACK_TEXT, CvUtil.FONT_LEFT_JUSTIFY, self.X_BACK, self.Y_BACK, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_PEDIA_BACK, 1, -1)
			
		# Forward
		screen.setText(self.NEXT_ID, "Background", self.FORWARD_TEXT, CvUtil.FONT_LEFT_JUSTIFY, self.X_FORWARD, self.Y_FORWARD, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_PEDIA_FORWARD, 1, -1)

		# List of items on the right
		screen.addListBoxGFC(self.LIST_ID, "", self.X_LINKS, self.Y_LINKS, self.W_LINKS, self.H_LINKS, TableStyles.TABLE_STYLE_STANDARD)
		screen.enableSelect(self.LIST_ID, True)
		
	# Screen construction function
	def showScreen(self, iCategory):
		self.iCategory = iCategory

		self.deleteAllWidgets()						
							
		screen = self.getScreen()
		
		bNotActive = (not screen.isActive())
		if bNotActive:
			self.setPediaCommonWidgets()

		# Header...
		szHeader = u"<font=4b>" +localText.getText("TXT_KEY_WIDGET_HELP", ()).upper() + u"</font>"
		szHeaderId = self.getNextWidgetName()
		screen.setLabel(szHeaderId, "Background", szHeader, CvUtil.FONT_CENTER_JUSTIFY, self.X_SCREEN, self.Y_TITLE, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_PEDIA_DESCRIPTION, -1, -1)

		self.panelName = self.getNextWidgetName()
		screen.addPanel(self.panelName, "", "", false, false,
			self.X_ITEMS_PANE, self.Y_ITEMS_PANE, self.W_ITEMS_PANE, self.H_ITEMS_PANE, PanelStyles.PANEL_STYLE_BLUE50)
		
		if self.iLastScreen	!= CvScreenEnums.PEDIA_MAIN or bNotActive:		
			self.placeLinks()
			self.iLastScreen = CvScreenEnums.PEDIA_MAIN
		
		if (self.mapCategories.has_key(iCategory)):
			self.mapCategories.get(iCategory)()
			
	def placeTechs(self):
		screen = self.getScreen()
		
		# sort techs alphabetically
		techsList = self.getSortedList( gc.getNumTechInfos(), gc.getTechInfo )

		# display the techs
		if (gc.getNumTechInfos() > self.BUTTON_COLUMNS):
			rowListName = "Child" + self.panelName
			screen.attachMultiListControlGFC(self.panelName, rowListName, "", 1, self.BUTTON_SIZE,self.BUTTON_SIZE, TableStyles.TABLE_STYLE_STANDARD)
			for tech in techsList:
				screen.appendMultiListButton( rowListName, gc.getTechInfo(tech[1]).getButton(), 0, WidgetTypes.WIDGET_PEDIA_JUMP_TO_TECH, tech[1], 1, false )
		else:
			i = 0
			for tech in techsList:
				y = self.ITEMS_SEPARATION + i * (self.BUTTON_SIZE + self.ITEMS_SEPARATION)
				screen.setImageButtonAt("", self.panelName, gc.getTechInfo(tech[1]).getButton(), self.ITEMS_MARGIN, y, self.BUTTON_SIZE, self.BUTTON_SIZE, WidgetTypes.WIDGET_PEDIA_JUMP_TO_TECH, tech[1], 1)
				screen.setLabelAt("", self.panelName, u"<font=4>" + gc.getTechInfo(tech[1]).getDescription() + u"</font>", CvUtil.FONT_LEFT_JUSTIFY, 2*self.ITEMS_MARGIN + self.BUTTON_SIZE, y+10, 0, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
				i += 1


						
	def placeUnits(self):
		screen = self.getScreen()
		
		unitList = self.getSortedList( gc.getNumUnitInfos(), gc.getUnitInfo )
			
		# display the units
		if (gc.getNumUnitInfos() > self.BUTTON_COLUMNS):
			rowListName = "Child" + self.panelName
			screen.attachMultiListControlGFC(self.panelName, rowListName, "", 1, self.BUTTON_SIZE,self.BUTTON_SIZE, TableStyles.TABLE_STYLE_STANDARD)
			for unit in unitList:
				screen.appendMultiListButton( rowListName, gc.getUnitInfo(unit[1]).getButton(), 0, WidgetTypes.WIDGET_PEDIA_JUMP_TO_UNIT, unit[1], 1, false )		
		else:
			i = 0
			for unit in unitList:
				y = self.ITEMS_SEPARATION + i * (self.BUTTON_SIZE + self.ITEMS_SEPARATION)
				screen.setImageButtonAt("", self.panelName, gc.getUnitInfo(unit[1]).getButton(), self.ITEMS_MARGIN, y, self.BUTTON_SIZE, self.BUTTON_SIZE, WidgetTypes.WIDGET_PEDIA_JUMP_TO_UNIT, unit[1], 1)
				screen.setLabelAt("", self.panelName, u"<font=4>" + gc.getUnitInfo(unit[1]).getDescription() + u"</font>", CvUtil.FONT_LEFT_JUSTIFY, 2*self.ITEMS_MARGIN + self.BUTTON_SIZE, y+10, 0, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
				i += 1

	def placeBuildings(self):
		screen = self.getScreen()
				
		buildingList = self.pediaBuildingScreen.getBuildingSortedList()
		
		# display the buildings
		if (gc.getNumBuildingInfos() > self.BUTTON_COLUMNS):
			rowListName = "Child" + self.panelName
			screen.attachMultiListControlGFC(self.panelName, rowListName, "", 1, self.BUTTON_SIZE,self.BUTTON_SIZE, TableStyles.TABLE_STYLE_STANDARD)
			for building in buildingList:
				if (not gc.getBuildingInfo(building[1]).isGraphicalOnly()):
					screen.appendMultiListButton( rowListName, gc.getBuildingInfo(building[1]).getButton(), 0, WidgetTypes.WIDGET_PEDIA_JUMP_TO_BUILDING, building[1], 1, false )
		else:
			i = 0
			for building in buildingList:
				if (not gc.getBuildingInfo(building[1]).isGraphicalOnly()):
					y = self.ITEMS_SEPARATION + i * (self.BUTTON_SIZE + self.ITEMS_SEPARATION)
					screen.setImageButtonAt("", self.panelName, gc.getBuildingInfo(building[1]).getButton(), self.ITEMS_MARGIN, y, self.BUTTON_SIZE, self.BUTTON_SIZE, WidgetTypes.WIDGET_PEDIA_JUMP_TO_BUILDING, building[1], 1)
					screen.setLabelAt("", self.panelName, u"<font=4>" + gc.getBuildingInfo(building[1]).getDescription() + u"</font>", CvUtil.FONT_LEFT_JUSTIFY, 2*self.ITEMS_MARGIN + self.BUTTON_SIZE, y+10, 0, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
					i += 1

						
	def placeBoni(self):
		return
		screen = self.getScreen()
		
		bonusList = self.getSortedList( gc.getNumBonusInfos(), gc.getBonusInfo )
		
		# display the resources
		if (gc.getNumBonusInfos() > self.BUTTON_COLUMNS):
			rowListName = "Child" + self.panelName
			screen.attachMultiListControlGFC(self.panelName, rowListName, "", 1, self.BUTTON_SIZE,self.BUTTON_SIZE, TableStyles.TABLE_STYLE_STANDARD)
			for bonus in bonusList:
				screen.appendMultiListButton( rowListName, gc.getBonusInfo(bonus[1]).getButton(), 0, WidgetTypes.WIDGET_PEDIA_JUMP_TO_BONUS, bonus[1], 1, false )
		else:
			i = 0
			for bonus in bonusList:
				y = self.ITEMS_SEPARATION + i * (self.BUTTON_SIZE + self.ITEMS_SEPARATION)
				screen.setImageButtonAt("", self.panelName, gc.getBonusInfo(bonus[1]).getButton(), self.ITEMS_MARGIN, y, self.BUTTON_SIZE, self.BUTTON_SIZE, WidgetTypes.WIDGET_PEDIA_JUMP_TO_BONUS, bonus[1], 1)
				screen.setLabelAt("", self.panelName, u"<font=4>" + gc.getBonusInfo(bonus[1]).getDescription() + u"</font>", CvUtil.FONT_LEFT_JUSTIFY, 2*self.ITEMS_MARGIN + self.BUTTON_SIZE, y+10, 0, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
				i += 1
			
	def placeImprovements(self):
		return
		screen = self.getScreen()
		
		improvementList = self.getSortedList( gc.getNumImprovementInfos(), gc.getImprovementInfo )
		
		# display the improvements
		if (gc.getNumImprovementInfos() > self.BUTTON_COLUMNS):
			rowListName = "Child" + self.panelName
			screen.attachMultiListControlGFC(self.panelName, rowListName, "", 1, self.BUTTON_SIZE,self.BUTTON_SIZE, TableStyles.TABLE_STYLE_STANDARD)
			for improvement in improvementList:
				if (not gc.getImprovementInfo(improvement[1]).isGraphicalOnly()):
					screen.appendMultiListButton( rowListName, gc.getImprovementInfo(improvement[1]).getButton(), 0, WidgetTypes.WIDGET_PEDIA_JUMP_TO_IMPROVEMENT, improvement[1], 1, false )
		else:
			i = 0
			for improvement in improvementList:
				if (not gc.getImprovementInfo(improvement[1]).isGraphicalOnly()):
					y = self.ITEMS_SEPARATION + i * (self.BUTTON_SIZE + self.ITEMS_SEPARATION)
					screen.setImageButtonAt("", self.panelName, gc.getImprovementInfo(improvement[1]).getButton(), self.ITEMS_MARGIN, y, self.BUTTON_SIZE, self.BUTTON_SIZE, WidgetTypes.WIDGET_PEDIA_JUMP_TO_IMPROVEMENT, improvement[1], 1)
					screen.setLabelAt("", self.panelName, u"<font=4>" + gc.getImprovementInfo(improvement[1]).getDescription() + u"</font>", CvUtil.FONT_LEFT_JUSTIFY, 2*self.ITEMS_MARGIN + self.BUTTON_SIZE, y+10, 0, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
					i += 1
												
	def placePromotions(self):
		return
		screen = self.getScreen()
					
		promotionList = self.getSortedList( gc.getNumPromotionInfos(), gc.getPromotionInfo )
		
		# display the promotion
		if (gc.getNumPromotionInfos() > self.BUTTON_COLUMNS):
			rowListName = "Child" + self.panelName
			screen.attachMultiListControlGFC(self.panelName, rowListName, "", 1, self.BUTTON_SIZE,self.BUTTON_SIZE, TableStyles.TABLE_STYLE_STANDARD)
			for promotion in promotionList:
				screen.appendMultiListButton( rowListName, gc.getPromotionInfo(promotion[1]).getButton(), 0, WidgetTypes.WIDGET_PEDIA_JUMP_TO_PROMOTION, promotion[1], 1, false )
		else:
			i = 0
			for promotion in promotionList:
				y = self.ITEMS_SEPARATION + i * (self.BUTTON_SIZE + self.ITEMS_SEPARATION)
				screen.setImageButtonAt("", self.panelName, gc.getPromotionInfo(promotion[1]).getButton(), self.ITEMS_MARGIN, y, self.BUTTON_SIZE, self.BUTTON_SIZE, WidgetTypes.WIDGET_PEDIA_JUMP_TO_PROMOTION, promotion[1], 1)
				screen.setLabelAt("", self.panelName, u"<font=4>" + gc.getPromotionInfo(promotion[1]).getDescription() + u"</font>", CvUtil.FONT_LEFT_JUSTIFY, 2*self.ITEMS_MARGIN + self.BUTTON_SIZE, y+10, 0, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
				i += 1

	def placeUnitGroups(self):
		return
		screen = self.getScreen()
		
		groupList = self.getSortedList( gc.getNumUnitCombatInfos(), gc.getUnitCombatInfo )

		if (gc.getNumUnitCombatInfos() > self.BUTTON_COLUMNS):
			rowListName = "Child" + self.panelName
			screen.attachMultiListControlGFC(self.panelName, rowListName, "", 1, self.BUTTON_SIZE,self.BUTTON_SIZE, TableStyles.TABLE_STYLE_STANDARD)
			for group in groupList:
				screen.appendMultiListButton(rowListName, gc.getUnitCombatInfo(group[1]).getButton(), 0, WidgetTypes.WIDGET_PEDIA_JUMP_TO_UNIT_COMBAT, group[1], 1, false )
		else:
			i = 0
			for group in groupList:
				y = self.ITEMS_SEPARATION + i * (self.BUTTON_SIZE + self.ITEMS_SEPARATION)
				screen.setImageButtonAt("", self.panelName, gc.getUnitCombatInfo(group[1]).getButton(), self.ITEMS_MARGIN, y, self.BUTTON_SIZE, self.BUTTON_SIZE, WidgetTypes.WIDGET_PEDIA_JUMP_TO_UNIT_COMBAT, group[1], 1)
				screen.setLabelAt("", self.panelName, u"<font=4>" + gc.getUnitCombatInfo(group[1]).getDescription() + u"</font>", CvUtil.FONT_LEFT_JUSTIFY, 2*self.ITEMS_MARGIN + self.BUTTON_SIZE, y+10, 0, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
				i += 1

	def placeCivs(self):
		screen = self.getScreen()
				
		civList = self.getSortedList( gc.getNumCivilizationInfos(), gc.getCivilizationInfo )
			
		# display the civs

		if (gc.getNumCivilizationInfos() > self.BUTTON_COLUMNS):
			rowListName = "Child" + self.panelName
			screen.attachMultiListControlGFC(self.panelName, rowListName, "", 1, self.BUTTON_SIZE,self.BUTTON_SIZE, TableStyles.TABLE_STYLE_STANDARD)
			for civ in civList:
				if (gc.getCivilizationInfo(civ[1]).isPlayable()):
					screen.appendMultiListButton( rowListName, ArtFileMgr.getCivilizationArtInfo(gc.getCivilizationInfo(civ[1]).getArtDefineTag()).getButton(), 0, WidgetTypes.WIDGET_PEDIA_JUMP_TO_CIV, civ[1], 1, false )
		else:
			i = 0
			for civ in civList:
				if (gc.getCivilizationInfo(civ[1]).isPlayable()):
					y = self.ITEMS_SEPARATION + i * (self.BUTTON_SIZE + self.ITEMS_SEPARATION)
					screen.setImageButtonAt("", self.panelName, gc.getCivilizationInfo(civ[1]).getButton(), self.ITEMS_MARGIN, y, self.BUTTON_SIZE, self.BUTTON_SIZE, WidgetTypes.WIDGET_PEDIA_JUMP_TO_CIV, civ[1], 1)
					screen.setLabelAt("", self.panelName, u"<font=4>" + gc.getCivilizationInfo(civ[1]).getDescription() + u"</font>", CvUtil.FONT_LEFT_JUSTIFY, 2*self.ITEMS_MARGIN + self.BUTTON_SIZE, y+10, 0, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
					i += 1
						
	def placeLeaders(self):
		screen = self.getScreen()
		
		leaderList = self.getSortedList( gc.getNumLeaderHeadInfos(), gc.getLeaderHeadInfo )
		
		# display the leaders					
		if (gc.getNumLeaderHeadInfos() > self.BUTTON_COLUMNS):
			rowListName = "Child" + self.panelName
			screen.attachMultiListControlGFC(self.panelName, rowListName, "", 1, self.BUTTON_SIZE,self.BUTTON_SIZE, TableStyles.TABLE_STYLE_STANDARD)
			for leader in leaderList:
				if gc.getLeaderHeadInfo(leader[1]).getFavoriteCivic() != -1:
					screen.appendMultiListButton( rowListName, gc.getLeaderHeadInfo(leader[1]).getButton(), 0, WidgetTypes.WIDGET_PEDIA_JUMP_TO_LEADER, leader[1], 1, false )
		else:
			i = 0
			for leader in leaderList:
				if gc.getLeaderHeadInfo(leader[1]).getFavoriteCivic() != -1:
					y = self.ITEMS_SEPARATION + i * (self.BUTTON_SIZE + self.ITEMS_SEPARATION)
					screen.setImageButtonAt("", self.panelName, gc.getLeaderHeadInfo(leader[1]).getButton(), self.ITEMS_MARGIN, y, self.BUTTON_SIZE, self.BUTTON_SIZE, WidgetTypes.WIDGET_PEDIA_JUMP_TO_LEADER, leader[1], 1)
					screen.setLabelAt("", self.panelName, u"<font=4>" + gc.getLeaderHeadInfo(leader[1]).getDescription() + u"</font>", CvUtil.FONT_LEFT_JUSTIFY, 2*self.ITEMS_MARGIN + self.BUTTON_SIZE, y+10, 0, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
					i += 1

	def placeReligions(self):
		return
		screen = self.getScreen()
				
		religionList = self.getSortedList( gc.getNumReligionInfos(), gc.getReligionInfo )
			
		# display the religions
		if (gc.getNumReligionInfos() > self.BUTTON_COLUMNS):
			rowListName = "Child" + self.panelName
			screen.attachMultiListControlGFC(self.panelName, rowListName, "", 1, self.BUTTON_SIZE,self.BUTTON_SIZE, TableStyles.TABLE_STYLE_STANDARD)
			for religion in religionList:
				screen.appendMultiListButton(rowListName, gc.getReligionInfo(religion[1]).getButton(), 0, WidgetTypes.WIDGET_PEDIA_JUMP_TO_RELIGION, religion[1], 1, false )
		else:
			i = 0
			for religion in religionList:
				y = self.ITEMS_SEPARATION + i * (self.BUTTON_SIZE + self.ITEMS_SEPARATION)
				screen.setImageButtonAt("", self.panelName, gc.getReligionInfo(religion[1]).getButton(), self.ITEMS_MARGIN, y, self.BUTTON_SIZE, self.BUTTON_SIZE, WidgetTypes.WIDGET_PEDIA_JUMP_TO_RELIGION, religion[1], 1)
				screen.setLabelAt("", self.panelName, u"<font=4>" + gc.getReligionInfo(religion[1]).getDescription() + u"</font>", CvUtil.FONT_LEFT_JUSTIFY, 2*self.ITEMS_MARGIN + self.BUTTON_SIZE, y+10, 0, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
				i += 1
						
	def placeCivics(self):
		return
		screen = self.getScreen()
		
		civicList = self.getSortedList( gc.getNumCivicInfos(), gc.getCivicInfo )
		
		# display the civics
		if (gc.getNumCivicInfos() > self.BUTTON_COLUMNS):
			rowListName = "Child" + self.panelName
			screen.attachMultiListControlGFC(self.panelName, rowListName, "", 1, self.BUTTON_SIZE,self.BUTTON_SIZE, TableStyles.TABLE_STYLE_STANDARD)
			for civic in civicList:
				screen.appendMultiListButton( rowListName, gc.getCivicInfo(civic[1]).getButton(), 0, WidgetTypes.WIDGET_PEDIA_JUMP_TO_CIVIC, civic[1], 1, false )
		else:
			i = 0
			for civic in civicList:
				y = self.ITEMS_SEPARATION + i * (self.BUTTON_SIZE + self.ITEMS_SEPARATION)
				screen.setImageButtonAt("", self.panelName, gc.getCivicInfo(civic[1]).getButton(), self.ITEMS_MARGIN, y, self.BUTTON_SIZE, self.BUTTON_SIZE, WidgetTypes.WIDGET_PEDIA_JUMP_TO_CIVIC, civic[1], 1)
				screen.setLabelAt("", self.panelName, u"<font=4>" + gc.getCivicInfo(civic[1]).getDescription() + u"</font>", CvUtil.FONT_LEFT_JUSTIFY, 2*self.ITEMS_MARGIN + self.BUTTON_SIZE, y+10, 0, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
				i += 1

	def placeProjects(self):
		return
		screen = self.getScreen()
		
		projectList = self.pediaProjectScreen.getProjectSortedList()
		
		# display the projects						
		if (gc.getNumProjectInfos() > self.BUTTON_COLUMNS):
			rowListName = "Child" + self.panelName
			screen.attachMultiListControlGFC(self.panelName, rowListName, "", 1, self.BUTTON_SIZE,self.BUTTON_SIZE, TableStyles.TABLE_STYLE_STANDARD)
			for project in projectList:
				screen.appendMultiListButton( rowListName, gc.getProjectInfo(project[1]).getButton(), 0, WidgetTypes.WIDGET_PEDIA_JUMP_TO_PROJECT, project[1], 1, false )
		else:
			i = 0
			for project in projectList:
				y = self.ITEMS_SEPARATION + i * (self.BUTTON_SIZE + self.ITEMS_SEPARATION)
				screen.setImageButtonAt("", self.panelName, gc.getProjectInfo(project[1]).getButton(), self.ITEMS_MARGIN, y, self.BUTTON_SIZE, self.BUTTON_SIZE, WidgetTypes.WIDGET_PEDIA_JUMP_TO_PROJECT, project[1], 1)
				screen.setLabelAt("", self.panelName, u"<font=4>" + gc.getProjectInfo(project[1]).getDescription() + u"</font>", CvUtil.FONT_LEFT_JUSTIFY, 2*self.ITEMS_MARGIN + self.BUTTON_SIZE, y+10, 0, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
				i += 1

	def placeTerrains(self):
		return
		screen = self.getScreen()

		terrainList = self.getSortedList( gc.getNumTerrainInfos(), gc.getTerrainInfo )
		
		# display the terrains
		if (gc.getNumTerrainInfos() > self.BUTTON_COLUMNS):
			rowListName = "Child" + self.panelName
			screen.attachMultiListControlGFC(self.panelName, rowListName, "", 1, self.BUTTON_SIZE,self.BUTTON_SIZE, TableStyles.TABLE_STYLE_STANDARD)
			for terrain in terrainList:
				if (not gc.getTerrainInfo(terrain[1]).isGraphicalOnly()):
					screen.appendMultiListButton( rowListName, gc.getTerrainInfo(terrain[1]).getButton(), 0, WidgetTypes.WIDGET_PEDIA_JUMP_TO_TERRAIN, terrain[1], 1, false )
		else:
			i = 0
			for terrain in terrainList:
				if (not gc.getTerrainInfo(terrain[1]).isGraphicalOnly()):
					y = self.ITEMS_SEPARATION + i * (self.BUTTON_SIZE + self.ITEMS_SEPARATION)
					screen.setImageButtonAt("", self.panelName, gc.getTerrainInfo(terrain[1]).getButton(), self.ITEMS_MARGIN, y, self.BUTTON_SIZE, self.BUTTON_SIZE, WidgetTypes.WIDGET_PEDIA_JUMP_TO_TERRAIN, terrain[1], 1)
					screen.setLabelAt("", self.panelName, u"<font=4>" + gc.getTerrainInfo(terrain[1]).getDescription() + u"</font>", CvUtil.FONT_LEFT_JUSTIFY, 2*self.ITEMS_MARGIN + self.BUTTON_SIZE, y+10, 0, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
					i += 1
						
	def placeFeatures(self):
		return
		screen = self.getScreen()
		
		featureList = self.getSortedList( gc.getNumFeatureInfos(), gc.getFeatureInfo )
		
		# display the features
		if (gc.getNumFeatureInfos() > self.BUTTON_COLUMNS):
			rowListName = "Child" + self.panelName
			screen.attachMultiListControlGFC(self.panelName, rowListName, "", 1, self.BUTTON_SIZE,self.BUTTON_SIZE, TableStyles.TABLE_STYLE_STANDARD)
			for feature in featureList:
				if (not gc.getFeatureInfo(feature[1]).isGraphicalOnly()):
					screen.appendMultiListButton( rowListName, gc.getFeatureInfo(feature[1]).getButton(), 0, WidgetTypes.WIDGET_PEDIA_JUMP_TO_FEATURE, feature[1], 1, false )
		else:
			i = 0
			for feature in featureList:
				if (not gc.getFeatureInfo(feature[1]).isGraphicalOnly()):
					y = self.ITEMS_SEPARATION + i * (self.BUTTON_SIZE + self.ITEMS_SEPARATION)
					screen.setImageButtonAt("", self.panelName, gc.getFeatureInfo(feature[1]).getButton(), self.ITEMS_MARGIN, y, self.BUTTON_SIZE, self.BUTTON_SIZE, WidgetTypes.WIDGET_PEDIA_JUMP_TO_FEATURE, feature[1], 1)
					screen.setLabelAt("", self.panelName, u"<font=4>" + gc.getFeatureInfo(feature[1]).getDescription() + u"</font>", CvUtil.FONT_LEFT_JUSTIFY, 2*self.ITEMS_MARGIN + self.BUTTON_SIZE, y+10, 0, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
					i += 1
						
	def placeConcepts(self):
		return
		screen = self.getScreen()
		
		# Create and place a tech pane									
		conceptList = self.getSortedList( gc.getNumConceptInfos(), gc.getConceptInfo )

		nColumns = 2
		tableName = self.getNextWidgetName()
		screen.addTableControlGFC(tableName, nColumns, self.X_ITEMS_PANE, self.Y_ITEMS_PANE+5, self.W_ITEMS_PANE, self.H_ITEMS_PANE-5, False, False, 16, 16, TableStyles.TABLE_STYLE_STANDARD);
		screen.enableSelect(tableName, False)
		for i in range(nColumns):
			screen.setTableColumnHeader(tableName, i, "", self.W_ITEMS_PANE/nColumns)
		
		# display the game concepts
		iCounter = 0
		iNumRows = 0
		for concept in conceptList:
			iColumn = iCounter % nColumns
			iRow = iCounter // nColumns
			if iRow >= iNumRows:
				iNumRows += 1
				screen.appendTableRow(tableName)
			screen.setText("tableItem" + str(iRow*nColumns + iColumn), "", u"<font=4>" + concept[0] + u"</font>", CvUtil.FONT_LEFT_JUSTIFY, 0, 0, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_PEDIA_DESCRIPTION_NO_HELP, CivilopediaPageTypes.CIVILOPEDIA_PAGE_CONCEPT, concept[1])
			screen.attachControlToTableCell("tableItem" + str(iRow*nColumns + iColumn), tableName, iRow, iColumn)
			iCounter += 1
						
	def placeSpecialists(self):
		screen = self.getScreen()
		
		# display specialists
		if (gc.getNumSpecialistInfos() > self.BUTTON_COLUMNS):
			rowListName = "Child" + self.panelName
			screen.attachMultiListControlGFC(self.panelName, rowListName, "", 1, self.BUTTON_SIZE,self.BUTTON_SIZE, TableStyles.TABLE_STYLE_STANDARD)
			for i in range(gc.getNumSpecialistInfos()):
				screen.appendMultiListButton( rowListName, gc.getSpecialistInfo(i).getButton(), 0, WidgetTypes.WIDGET_PEDIA_JUMP_TO_SPECIALIST, i, 1, false )
		else:
			for i in range(gc.getNumSpecialistInfos()):
				y = self.ITEMS_SEPARATION + i * (self.BUTTON_SIZE + self.ITEMS_SEPARATION)
				screen.setImageButtonAt("", self.panelName, gc.getSpecialistInfo(i).getButton(), self.ITEMS_MARGIN, y, self.BUTTON_SIZE, self.BUTTON_SIZE, WidgetTypes.WIDGET_PEDIA_JUMP_TO_SPECIALIST, i, 1)
				screen.setLabelAt("", self.panelName, u"<font=4>" + gc.getSpecialistInfo(i).getDescription() + u"</font>", CvUtil.FONT_LEFT_JUSTIFY, 2*self.ITEMS_MARGIN + self.BUTTON_SIZE, y+10, 0, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
						
	def placeHints(self):
		screen = self.getScreen()
				                
		self.szAreaId = self.getNextWidgetName()
		screen.addListBoxGFC( self.szAreaId, "",
                                      self.X_ITEMS_PANE, self.Y_ITEMS_PANE, self.W_ITEMS_PANE, self.H_ITEMS_PANE, TableStyles.TABLE_STYLE_STANDARD )
		
		szHintsText = CyGameTextMgr().buildHintsList()
		hintText = string.split( szHintsText, "\n" )
		for hint in hintText:
			if len( hint ) != 0:
				screen.appendListBoxString( self.szAreaId, hint, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
										
	def placeLinks(self):
		
		screen = self.getScreen()
		
		screen.clearListBoxGFC(self.LIST_ID)

		i = 0
		for szCategory in self.listCategories:
			screen.appendListBoxString(self.LIST_ID, szCategory, WidgetTypes.WIDGET_PEDIA_MAIN, i, 0, CvUtil.FONT_LEFT_JUSTIFY )
			i += 1

		screen.setSelectedListBoxStringGFC(self.LIST_ID, self.iCategory)
					
	# returns a unique ID for a widget in this screen
	def getNextWidgetName(self):
		szName = self.WIDGET_ID + str(self.nWidgetCount)
		self.nWidgetCount += 1
		return szName
		
	def pediaJump(self, iScreen, iEntry, bRemoveFwdList):
	
		if (iEntry < 0):
			return
			
		self.iActivePlayer = gc.getGame().getActivePlayer()

		self.pediaHistory.append((iScreen, iEntry))
		if (bRemoveFwdList):
			self.pediaFuture = []

		if (iScreen == CvScreenEnums.PEDIA_MAIN):
			self.showScreen(iEntry)
		elif (iScreen == CvScreenEnums.PEDIA_TECH):
			self.pediaTechScreen.interfaceScreen(iEntry)
		elif (iScreen == CvScreenEnums.PEDIA_UNIT):
			self.pediaUnitScreen.interfaceScreen(iEntry)
		elif (iScreen == CvScreenEnums.PEDIA_BUILDING):
			self.pediaBuildingScreen.interfaceScreen(iEntry)	
		elif (iScreen == CvScreenEnums.PEDIA_CIVILIZATION):
			self.pediaCivilization.interfaceScreen(iEntry)	
		elif (iScreen == CvScreenEnums.PEDIA_LEADER):
			self.pediaLeader.interfaceScreen(iEntry)	

	def back(self):
		print "pedia back"
		if (len(self.pediaHistory) > 1):
			self.pediaFuture.append(self.pediaHistory.pop())
			current = self.pediaHistory.pop()
			self.pediaJump(current[0], current[1], False)
		return 1
		
	def forward(self):
		print "pedia fwd"
		if (self.pediaFuture):
			current = self.pediaFuture.pop()
			self.pediaJump(current[0], current[1], False)
		return 1
		
	def pediaShow(self):
		if (not self.pediaHistory):
			self.pediaHistory.append((CvScreenEnums.PEDIA_MAIN, 0))
			
		current = self.pediaHistory.pop()
		
		# erase history so it doesn't grow too large during the game
		self.pediaFuture = []
		self.pediaHistory = []
		
		# jump to the last screen that was up
		self.pediaJump(current[0], current[1], False)
		
	def link(self, szLink):
		for i in range(gc.getNumTechInfos()):
			if (gc.getTechInfo(i).getType() == szLink):
				return self.pediaJump(CvScreenEnums.PEDIA_TECH, i, True)
		for i in range(gc.getNumUnitInfos()):
			if (gc.getUnitInfo(i).getType() == szLink):
				return self.pediaJump(CvScreenEnums.PEDIA_UNIT, i, True)
		for i in range(gc.getNumBuildingInfos()):
			if (gc.getBuildingInfo(i).getType() == szLink):
				return self.pediaJump(CvScreenEnums.PEDIA_BUILDING, i, True)
		for i in range(gc.getNumCivilizationInfos()):
			if (gc.getCivilizationInfo(i).getType() == szLink):
				return self.pediaJump(CvScreenEnums.PEDIA_CIVILIZATION, i, True)
		for i in range(gc.getNumLeaderHeadInfos()):
			if (gc.getLeaderHeadInfo(i).getType() == szLink):
				return self.pediaJump(CvScreenEnums.PEDIA_LEADER, i, True)
																
	def deleteAllWidgets(self):
		screen = self.getScreen()
		iNumWidgets = self.nWidgetCount
		self.nWidgetCount = 0
		for i in range(iNumWidgets):
			screen.deleteWidget(self.getNextWidgetName())
		self.nWidgetCount = 0
		
		
			
	# Will handle the input for this screen...
	def handleInput (self, inputClass):
		# redirect to proper screen if necessary
		if (inputClass.getPythonFile() == CvScreenEnums.PEDIA_TECH):
			return self.pediaTechScreen.handleInput(inputClass)
		if (inputClass.getPythonFile() == CvScreenEnums.PEDIA_UNIT):
			return self.pediaUnitScreen.handleInput(inputClass)
		if (inputClass.getPythonFile() == CvScreenEnums.PEDIA_BUILDING):
			return self.pediaBuildingScreen.handleInput(inputClass)
		if (inputClass.getPythonFile() == CvScreenEnums.PEDIA_CIVILIZATION):
			return self.pediaCivilization.handleInput(inputClass)
		if (inputClass.getPythonFile() == CvScreenEnums.PEDIA_LEADER):
			return self.pediaLeader.handleInput(inputClass)						
		return 0

