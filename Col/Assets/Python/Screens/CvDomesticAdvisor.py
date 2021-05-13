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

class CvDomesticAdvisor:
	"Domestic Advisor Screen"
	def __init__(self):
		self.listSelectedCities = []
		self.selectedSelectionGroupHeadUnitID = -1
		
	# Screen construction function
	def interfaceScreen(self):
		player = gc.getPlayer(gc.getGame().getActivePlayer())

		# Create a new screen, called DomesticAdvisur, using the file CvDomesticAdvisor.py for input
		screen = CyGInterfaceScreen( "DomesticAdvisor", CvScreenEnums.DOMESTIC_ADVISOR )

		self.nScreenWidth = screen.getXResolution()
		self.nScreenHeight = (screen.getYResolution() - (screen.getYResolution() * 31 / 100))
		
		self.nTableWidth = self.nScreenWidth * 19 / 20
		self.nTableHeight = self.nScreenHeight - 150
		self.nNormalizedTableWidth = self.nTableWidth
		
		self.iButtonSpacing = 80
		self.iButtonSize = 60
		self.iCityButtonSize = 48
		self.Y_LOWER_ROW = self.nScreenHeight - 70
		self.CITY_NAME_COLUMN_WIDTH = 250
		self.PRODUCTION_COLUMN_SIZE = (self.nTableWidth - self.CITY_NAME_COLUMN_WIDTH) / len(range(YieldTypes.YIELD_FOOD, YieldTypes.YIELD_TRADE_GOODS + 1))
		self.WAREHOUSE_COLUMN_SIZE = (self.nTableWidth - self.CITY_NAME_COLUMN_WIDTH - 60) / len(range(YieldTypes.YIELD_FOOD, YieldTypes.YIELD_TRADE_GOODS + 1))
		self.BUILDING_COLUMN_SIZE = (self.nTableWidth - self.CITY_NAME_COLUMN_WIDTH) / (gc.getNumSpecialBuildingInfos() -1)
		self.ROW_HIGHT = 48

		self.Y_EXIT = self.nScreenHeight - 36
		self.X_EXIT = self.nScreenWidth - 30

		screen.setRenderInterfaceOnly(True)
		screen.setRenderFrozenWorld(True)
		screen.setDimensions((screen.getXResolution() - self.nScreenWidth) / 2, 0, self.nScreenWidth, self.nScreenHeight)
		screen.showScreen(PopupStates.POPUPSTATE_IMMEDIATE, False)

		# Here we set the background widget and exit button, and we show the screen
		screen.addPanel( "DomesticAdvisorBG", u"", u"", True, False, 0, 0, self.nScreenWidth, self.nScreenHeight, PanelStyles.PANEL_STYLE_MAIN, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.addDrawControl("DomesticAdvisorBG", ArtFileMgr.getInterfaceArtInfo("MAINMENU_SLIDESHOW_LOAD").getPath(), 0, 0, self.nScreenWidth, self.nScreenHeight, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.addDDSGFC("TopPanel", ArtFileMgr.getInterfaceArtInfo("INTERFACE_SCREEN_TITLE").getPath(), 0, 0, self.nScreenWidth, 55, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.addDDSGFC("BottomPanel", ArtFileMgr.getInterfaceArtInfo("INTERFACE_SCREEN_TAB_OFF").getPath(), 0, self.nScreenHeight - 55, self.nScreenWidth, 55, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.setText("DomesticExit", "Background", u"<font=4>" + localText.getText("TXT_KEY_PEDIA_SCREEN_EXIT", ()).upper() + "</font>", CvUtil.FONT_RIGHT_JUSTIFY, self.X_EXIT, self.Y_EXIT, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_CLOSE_SCREEN, -1, -1 )

		screen.setLabel("DomesticScreenWidgetHeader", "Background", u"<font=4b>" + localText.getText("TXT_KEY_DOMESTIC_ADVISOR_TITLE", ()).upper() + u"</font>", CvUtil.FONT_CENTER_JUSTIFY, self.nScreenWidth / 2, 4, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)

		self.StateTypes = []
		self.StateButtons = []

		#StateTypes Enums
		self.GENERAL_STATE = 0
		self.StateTypes.append("GeneralState")
		self.StateButtons.append("INTERFACE_CITY_MAP_BUTTON")
		self.PRODUCTION_STATE = 1
		self.StateTypes.append("ProductionState")
		self.StateButtons.append("INTERFACE_NET_YIELD_BUTTON")
		self.WAREHOUSE_STATE = 2
		self.StateTypes.append("WareHouseState")
		self.StateButtons.append("INTERFACE_STORES_BUTTON")
		self.BUILDING_STATE = 3
 		self.StateTypes.append("BuildingState")
		self.StateButtons.append("INTERFACE_CITY_BUILD_BUTTON")
		self.CITIZEN_STATE = 4
		self.StateTypes.append("CitizenState")
		self.StateButtons.append("INTERFACE_CITY_CITIZEN_BUTTON")
		self.IMPORTEXPORT_STATE = 5
		self.StateTypes.append("ImportExportState")
		self.StateButtons.append("INTERFACE_CITY_GOVENOR_BUTTON")
		self.TRADEROUTE_STATE = 6
		self.StateTypes.append("TradeRouteState")
		self.StateButtons.append("INTERFACE_IMPORT_EXPORT_BUTTON")
		
		self.RebuildArrays()

		#Initialize the Lists
		for iState in range(len(self.StateTypes)):
			if iState != self.TRADEROUTE_STATE:
				szStateName = self.StateTypes[iState] + "ListBackground"
				screen.addTableControlGFC(szStateName, 19, (self.nScreenWidth - self.nTableWidth) / 2, 60, self.nTableWidth, self.nTableHeight, True, False, self.iCityButtonSize, self.iCityButtonSize, TableStyles.TABLE_STYLE_STANDARD )
				screen.enableSelect(szStateName, True)
				screen.enableSort(szStateName)
				screen.setStyle(szStateName, "Table_StandardCiv_Style")
				screen.hide(szStateName)
				screen.setTableColumnHeader(szStateName, 0, "", 56 )
				screen.setTableColumnHeader(szStateName, 1, "<font=3>" + localText.getText("TXT_KEY_DOMESTIC_ADVISOR_NAME", ()).upper() + "</font>", self.CITY_NAME_COLUMN_WIDTH - 56 )

				for iCity in range(len(self.Cities)):
					screen.appendTableRow(szStateName)
					screen.setTableRowHeight(szStateName, iCity, self.ROW_HIGHT)
		
		self.RebuildRouteTable()
		self.RebuildTransportTable()

		#GeneralState Headers
		szListName = "GeneralStateListBackground"
		# Population Column
		screen.setTableColumnHeader( szListName, 2, "<font=3>" + localText.getText("TXT_KEY_POPULATION", ()) + "</font>", (self.nTableWidth - self.CITY_NAME_COLUMN_WIDTH) / 12 )
		# Liberty Column
		screen.setTableColumnHeader( szListName, 3, "<font=3>" + (u" %c" %(CyGame().getSymbolID(FontSymbols.POWER_CHAR))) + "</font>", (self.nTableWidth - self.CITY_NAME_COLUMN_WIDTH) / 12 )		
		# Food Column
		screen.setTableColumnHeader( szListName, 5, "<font=3>" + (u" %c" % gc.getYieldInfo(YieldTypes.YIELD_FOOD).getChar()) + "</font>", (self.nTableWidth - self.CITY_NAME_COLUMN_WIDTH) / 12 )
		# Hammers Column
		screen.setTableColumnHeader( szListName, 6, "<font=3>" + (u" %c" % gc.getYieldInfo(YieldTypes.YIELD_HAMMERS).getChar()) + "</font>", (self.nTableWidth - self.CITY_NAME_COLUMN_WIDTH) / 12 )
		# Bells Column
		screen.setTableColumnHeader( szListName, 8, "<font=3>" + (u" %c" % gc.getYieldInfo(YieldTypes.YIELD_BELLS).getChar()) + "</font>", (self.nTableWidth - self.CITY_NAME_COLUMN_WIDTH) / 12 )
		# Crosses Column
		screen.setTableColumnHeader( szListName, 10, "<font=3>" + (u" %c" % gc.getYieldInfo(YieldTypes.YIELD_CROSSES).getChar()) + "</font>", (self.nTableWidth - self.CITY_NAME_COLUMN_WIDTH) / 12 )
		# Education Column
		screen.setTableColumnHeader( szListName, 11, "<font=3>" + (u" %c" % gc.getYieldInfo(YieldTypes.YIELD_EDUCATION).getChar()) + "</font>", (self.nTableWidth - self.CITY_NAME_COLUMN_WIDTH) / 12 )
		# Garrison Column
		screen.setTableColumnHeader( szListName, 13, "<font=3>" + (u" %c" % CyGame().getSymbolID(FontSymbols.STRENGTH_CHAR)) + "</font>", (self.nTableWidth - self.CITY_NAME_COLUMN_WIDTH) / 12 )
		# Defense Column
		screen.setTableColumnHeader( szListName, 14, "<font=3>" + (u" %c" % CyGame().getSymbolID(FontSymbols.DEFENSE_CHAR)) + "</font>", (self.nTableWidth - self.CITY_NAME_COLUMN_WIDTH) / 12 )
		# Production Column
		screen.setTableColumnHeader( szListName, 15, "<font=3>" + localText.getText("TXT_KEY_DOMESTIC_ADVISOR_PRODUCING", ()).upper() + "</font>", (self.nTableWidth - self.CITY_NAME_COLUMN_WIDTH) / 4 )

		#WareHouseState Headers
		screen.setTableColumnHeader( "WareHouseStateListBackground", 2, "<font=3>" + "MAX" + "</font>", 60 )
		for iYield in range(YieldTypes.YIELD_FOOD, YieldTypes.YIELD_TRADE_GOODS + 1):
			screen.setTableColumnHeader( "WareHouseStateListBackground", iYield + 3, "<font=3> " + (u" %c" % gc.getYieldInfo(iYield).getChar()) + "</font>", (self.WAREHOUSE_COLUMN_SIZE * self.nTableWidth) / self.nNormalizedTableWidth )

		#ProductionState Headers
		for iYield in range(YieldTypes.YIELD_FOOD, YieldTypes.YIELD_TRADE_GOODS + 1):
			screen.setTableColumnHeader( "ProductionStateListBackground", iYield + 2, "<font=3> " + (u" %c" % gc.getYieldInfo(iYield).getChar()) + "</font>", (self.PRODUCTION_COLUMN_SIZE * self.nTableWidth) / self.nNormalizedTableWidth )

		#ImportExportState Headers
		for iYield in range(YieldTypes.YIELD_FOOD, YieldTypes.YIELD_TRADE_GOODS + 1):
			screen.setTableColumnHeader( "ImportExportStateListBackground", iYield + 2, "<font=3> " + (u" %c" % gc.getYieldInfo(iYield).getChar()) + "</font>", (self.PRODUCTION_COLUMN_SIZE * self.nTableWidth) / self.nNormalizedTableWidth )

		# Building Headers
		for iSpecial in range(gc.getNumSpecialBuildingInfos()):
			if (iSpecial != gc.getInfoTypeForString("SPECIALBUILDING_BELLS")):
				screen.setTableColumnHeader( "BuildingStateListBackground", iSpecial + 1, "<font=3> " + (u" %c" %  gc.getSpecialBuildingInfo(iSpecial).getChar()) + "</font>", (self.BUILDING_COLUMN_SIZE * self.nTableWidth) / self.nNormalizedTableWidth )

		# Citizen Headers
		screen.setTableColumnHeader( "CitizenStateListBackground", 2, "<font=3>" +  localText.getText("TXT_KEY_DOMESTIC_ADVISOR_STATE_CITIZEN", ()).upper() + "</font>", self.nTableWidth * 3 / 4 )
			
		#Default State on Screen opening
		self.CurrentState = self.GENERAL_STATE

		# Draw the city list...
		self.drawContents()
		
	def drawButtons(self):
		screen = CyGInterfaceScreen( "DomesticAdvisor", CvScreenEnums.DOMESTIC_ADVISOR )

		for iState in range(len(self.StateTypes)):
			szStateName = self.StateTypes[iState] + "ListBackground"
			screen.setImageButton(szStateName + "Button", ArtFileMgr.getInterfaceArtInfo(self.StateButtons[iState]).getPath(), (self.iButtonSpacing * iState) + (self.iButtonSpacing / 2), self.Y_LOWER_ROW, self.iButtonSize, self.iButtonSize, WidgetTypes.WIDGET_GENERAL, iState, -1 )
			if (int(self.CurrentState) == iState):
				RelativeButtonSize = 130
				screen.setImageButton("HighlightButton", ArtFileMgr.getInterfaceArtInfo("INTERFACE_HIGHLIGHTED_BUTTON").getPath(), (self.iButtonSpacing * iState) + (self.iButtonSpacing / 2) - ((self.iButtonSize * RelativeButtonSize / 100) / 2) + (self.iButtonSize / 2), self.Y_LOWER_ROW - ((self.iButtonSize * RelativeButtonSize / 100) / 2) + (self.iButtonSize / 2), self.iButtonSize * RelativeButtonSize / 100, self.iButtonSize * RelativeButtonSize / 100, WidgetTypes.WIDGET_GENERAL, iState, -1 )

	# Function to draw the contents of the cityList passed in
	def drawContents (self):

		# Get the screen and the player
		screen = CyGInterfaceScreen( "DomesticAdvisor", CvScreenEnums.DOMESTIC_ADVISOR )
		player = gc.getPlayer(CyGame().getActivePlayer())
		screen.moveToFront( "Background" )

		#Loop through the cities and update the table
		if (self.CurrentState != self.TRADEROUTE_STATE):
			for iCity in range(len(self.Cities)):
				if (self.Cities[iCity].getName() in self.listSelectedCities):
					screen.selectRow( self.StateTypes[self.CurrentState] + "ListBackground", iCity, True )
				self.updateCityTable(self.Cities[iCity], iCity)

			if (self.CurrentState == self.CITIZEN_STATE):
				for iCity in range(len(self.Cities)):
					self.updateCitizenTable(self.Cities[iCity], iCity)	
		else:
			self.RebuildRouteTable()
			BonusRow = 0
			for iRoute in range(player.getNumTradeRoutes()):
				self.updateRouteTable(self.Routes[iRoute], player.getTradeRouteByIndex(iRoute).getID(), iRoute)
				BonusRow += 1
			self.BuildAllTransportsRow(BonusRow)

		self.drawButtons()
		screen.show(self.StateTypes[self.CurrentState] + "ListBackground")
		self.updateAppropriateCitySelection()

	def updateCityTable(self, pLoopCity, i):
		screen = CyGInterfaceScreen( "DomesticAdvisor", CvScreenEnums.DOMESTIC_ADVISOR )

		szState = self.StateTypes[self.CurrentState]
		screen.setTableText(szState + "ListBackground", 0, i, "", ArtFileMgr.getInterfaceArtInfo("INTERFACE_BUTTONS_CITYSELECTION").getPath(), WidgetTypes.WIDGET_ZOOM_CITY, pLoopCity.getOwner(), pLoopCity.getID(), CvUtil.FONT_LEFT_JUSTIFY);
		screen.setTableText(szState + "ListBackground", 1, i, "<font=4>" + pLoopCity.getName() + "</font>", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )

		if(self.CurrentState == self.GENERAL_STATE):

			# Population
			screen.setTableInt(szState + "ListBackground", 2, i, "<font=3>" + unicode(pLoopCity.getPopulation()) + "</font>", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
			# Liberty
			screen.setTableInt(szState + "ListBackground", 3, i, "<font=3>" + unicode(pLoopCity.getRebelPercent()) + "%" + "</font>", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
			# Food rate
			iNetFood = pLoopCity.foodDifference()
			szText = unicode(iNetFood)
			if iNetFood > 0:
				szText = localText.getText("TXT_KEY_COLOR_POSITIVE", ()) + szText + localText.getText("TXT_KEY_COLOR_REVERT", ())
			elif iNetFood < 0:
				szText = localText.getText("TXT_KEY_COLOR_NEGATIVE", ()) + szText + localText.getText("TXT_KEY_COLOR_REVERT", ())			
			screen.setTableInt(szState + "ListBackground", 5, i, "<font=3>" + szText + "</font>", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
			# Hammers rate
			screen.setTableInt(szState + "ListBackground", 6, i, "<font=3>" + unicode(pLoopCity.calculateNetYield(YieldTypes.YIELD_HAMMERS)) + "</font>", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
			#Bell rate
			screen.setTableInt(szState + "ListBackground", 8, i, "<font=3>" + unicode(pLoopCity.calculateNetYield(YieldTypes.YIELD_BELLS)) + "</font>", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
			# Crosses rate
			screen.setTableInt(szState + "ListBackground", 10, i, "<font=3>" + unicode(pLoopCity.calculateNetYield(YieldTypes.YIELD_CROSSES)) + "</font>", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
			# Education rate
			screen.setTableInt(szState + "ListBackground", 11, i, "<font=3>" + unicode(pLoopCity.calculateNetYield(YieldTypes.YIELD_EDUCATION)) + "</font>", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
			# Garrison
			screen.setTableInt(szState + "ListBackground", 13, i, "<font=3>" + unicode(pLoopCity.plot().getNumDefenders(pLoopCity.getOwner())) + "</font>", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
			# Defense
			szBuffer = u"<font=3>%s%%</font>" % (str(pLoopCity.getDefenseModifier()))
			screen.setTableInt(szState + "ListBackground", 14, i, "<font=3>" + szBuffer + "</font>", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
			# Producing
			screen.setTableText(szState + "ListBackground", 15, i, "<font=3>" + pLoopCity.getProductionName() + " (" + str(pLoopCity.getGeneralProductionTurnsLeft()) + ")" + "</font>", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )

		elif(self.CurrentState == self.PRODUCTION_STATE):

			for iYield in range(YieldTypes.YIELD_FOOD, YieldTypes.YIELD_TRADE_GOODS + 1):
				iNetYield = pLoopCity.calculateNetYield(iYield)
				szText = unicode(iNetYield)
				if iNetYield > 0:
					szText = localText.getText("TXT_KEY_COLOR_POSITIVE", ()) + u"+" + szText + localText.getText("TXT_KEY_COLOR_REVERT", ())
				elif iNetYield < 0:
					szText = localText.getText("TXT_KEY_COLOR_NEGATIVE", ()) + szText + localText.getText("TXT_KEY_COLOR_REVERT", ())
				elif iNetYield == 0:
					szText = ""
				screen.setTableInt("ProductionStateListBackground", iYield + 2, i, "<font=3>" + szText + "<font/>", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )

		elif(self.CurrentState == self.WAREHOUSE_STATE):

			screen.setTableInt("WareHouseStateListBackground", 2, i, u"<font=3><color=255,255,255>" + str(pLoopCity.getMaxYieldCapacity()) + u"</color></font>", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )

			for iYield in range(YieldTypes.YIELD_FOOD, YieldTypes.YIELD_TRADE_GOODS + 1):
				iNetYield = pLoopCity.getYieldStored(iYield)
				szText = unicode(iNetYield)
				if iNetYield == 0:
					szText = ""
				if (pLoopCity.calculateNetYield(iYield) * 5 + pLoopCity.getYieldStored(iYield) <= pLoopCity.getMaxYieldCapacity() or iYield == YieldTypes.YIELD_FOOD):
					screen.setTableInt("WareHouseStateListBackground", iYield + 3, i, u"<font=3><color=0,255,255>" + szText + u"</color></font>", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
				elif (pLoopCity.getYieldStored(iYield) <= pLoopCity.getMaxYieldCapacity()):			
					screen.setTableInt("WareHouseStateListBackground", iYield + 3, i, u"<font=3><color=255,255,0>" + szText + u"</color></font>", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
				else:
					screen.setTableInt("WareHouseStateListBackground", iYield + 3, i, u"<font=3><color=255,0,0>" + szText + u"</color></font>", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
				
		elif(self.CurrentState == self.BUILDING_STATE):
			
			for iSpecial in range(gc.getNumSpecialBuildingInfos()):
				if (iSpecial != gc.getInfoTypeForString("SPECIALBUILDING_BELLS")):
					iIconBuilding = -1
					for iBuilding in range(gc.getNumBuildingInfos()):
						if gc.getBuildingInfo(iBuilding).getSpecialBuildingType() == iSpecial:
							if pLoopCity.isHasBuilding(iBuilding):
								iIconBuilding = iBuilding
								break
					if iIconBuilding != -1:
						screen.setTableInt("BuildingStateListBackground", iSpecial + 1, i, "", gc.getBuildingInfo(iBuilding).getButton(), WidgetTypes.WIDGET_PEDIA_JUMP_TO_BUILDING, iBuilding, -1, CvUtil.FONT_LEFT_JUSTIFY )

		elif(self.CurrentState == self.IMPORTEXPORT_STATE):

			for iYield in range(YieldTypes.YIELD_FOOD, YieldTypes.YIELD_TRADE_GOODS + 1):
				bExportYield = pLoopCity.isExport(iYield)
				bImportYield = pLoopCity.isImport(iYield)
				if (bExportYield and bImportYield):
					screen.setTableInt("ImportExportStateListBackground", iYield + 2, i, u"<font=3><color=255,255,0>" + localText.getText("TXT_KEY_IN_AND_OUT", ()) + u"</color></font>", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
				elif (not bExportYield and bImportYield):
					screen.setTableInt("ImportExportStateListBackground", iYield + 2, i, u"<font=3><color=0,255,0>" + localText.getText("TXT_KEY_IN", ()) + u"</color></font>", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
				elif (bExportYield and not bImportYield):
					screen.setTableInt("ImportExportStateListBackground", iYield + 2, i, u"<font=3><color=255,0,0>" + localText.getText("TXT_KEY_OUT", ()) + u"</color></font>", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
			
			screen.setTableText(szState + "ListBackground", 1, i, "<font=4>" + pLoopCity.getName() + "</font>", "", WidgetTypes.WIDGET_YIELD_IMPORT_EXPORT, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
		
	def updateCitizenTable(self, pCity, iRow):
		screen = CyGInterfaceScreen("DomesticAdvisor", CvScreenEnums.DOMESTIC_ADVISOR)

		if(self.CurrentState == self.CITIZEN_STATE):
			szState = self.StateTypes[self.CurrentState]
			screen.addPanel("CitizenPanel" + str(iRow), u"", u"", True, False, 0, 0, self.nTableWidth - self.CITY_NAME_COLUMN_WIDTH, 30, PanelStyles.PANEL_STYLE_EMPTY, WidgetTypes.WIDGET_GENERAL, -1, -1)
			screen.attachControlToTableCell("CitizenPanel" + str(iRow), szState + "ListBackground", iRow, 2 )

			iSpace = (self.nTableWidth - self.CITY_NAME_COLUMN_WIDTH - (self.iCityButtonSize / 4)) / pCity.getPopulation()
			if iSpace > self.iCityButtonSize:
				iSpace = self.iCityButtonSize
			ButtonSizePercentage = 40
			for iCitizen in range(pCity.getPopulation() - 1, -1, -1):
				pCitizen = pCity.getPopulationUnitByIndex(iCitizen)
				iType = pCitizen.getUnitType()
				szButtonName = "CitizenGroupButton" + str(iRow) + "-" + str(iCitizen)
				screen.setImageButtonAt(szButtonName, "CitizenPanel" + str(iRow), gc.getUnitInfo(iType).getButton(), iCitizen * iSpace, self.iCityButtonSize / 5, self.iCityButtonSize * 4 / 5, self.iCityButtonSize * 4 / 5, WidgetTypes.WIDGET_CITIZEN, pCitizen.getID(), pCity.getID())
				screen.setImageButtonAt("CitizenProfession" + str(iRow) + "-" + str(iCitizen), "CitizenPanel" + str(iRow), gc.getProfessionInfo(pCitizen.getProfession()).getButton(), (iCitizen * iSpace) + (self.iCityButtonSize * 2 / 4), 0, self.iCityButtonSize * ButtonSizePercentage / 100 , self.iCityButtonSize * ButtonSizePercentage / 100, WidgetTypes.WIDGET_CITIZEN, pCitizen.getID(), pCity.getID())
			
	def updateRouteTable(self, pRoute, iRoute, iRow):
		screen = CyGInterfaceScreen("DomesticAdvisor", CvScreenEnums.DOMESTIC_ADVISOR)

		if(self.CurrentState == self.TRADEROUTE_STATE):
			szState = self.StateTypes[self.CurrentState]
			player = gc.getPlayer(gc.getGame().getActivePlayer())

			screen.setTableText(szState + "ListBackground", 0, iRow, "<font=4>" + pRoute.getSourceCityName() + "</font>", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
			screen.setTableText(szState + "ListBackground", 1, iRow, "<font=4>" + pRoute.getDestinationCityName() + "</font>", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
			screen.setTableText(szState + "ListBackground", 2, iRow, u"<font=3>%c</font>" % gc.getYieldInfo(pRoute.getYield()).getChar(), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )

			screen.addPanel("RoutePanel" + str(iRow), u"", u"", True, False, 0, 0, (390 * self.nTableWidth) / self.nNormalizedTableWidth, 30, PanelStyles.PANEL_STYLE_EMPTY, WidgetTypes.WIDGET_GENERAL, iRoute, -1)
			screen.attachControlToTableCell("RoutePanel" + str(iRow), szState + "ListBackground", iRow, 4 )

			SelectionState = False
			iPosition = 10
			SelectedTransportID = -1
			ToggleButtonSet = False

			iSpace = (self.nTableWidth - 390) / len(self.Transports)
			if iSpace > self.iCityButtonSize:
				iSpace = self.iCityButtonSize
						
			for iSelectionGroupIndex in range(len(self.Transports) -1, -1, -1):
				SelectionGroup = self.Transports[iSelectionGroupIndex]

				if (not SelectionGroup.isNone()):
					GroupHeadID = SelectionGroup.getHeadUnit().getID()
					szButtonName = "UnitGroupButton" + str(SelectionGroup.getID()) + "Route" + str(iRow)
					if(SelectionGroup.isAssignedTradeRoute(pRoute.getID())):
						screen.addCheckBoxGFCAt("RoutePanel" + str(iRow), szButtonName, SelectionGroup.getHeadUnit().getButton(), ArtFileMgr.getInterfaceArtInfo("BUTTON_HILITE_SQUARE").getPath(), iSelectionGroupIndex * iSpace, 0, self.iCityButtonSize, self.iCityButtonSize, WidgetTypes.WIDGET_GENERAL, 10001, SelectionGroup.getHeadUnit().getID(), ButtonStyles.BUTTON_STYLE_LABEL )
					elif (not self.RouteValidity[iSelectionGroupIndex][iRow]):
						screen.addDDSGFCAt( szButtonName, "RoutePanel" + str(iRow), ArtFileMgr.getInterfaceArtInfo("INTERFACE_BUTTONS_CANCEL").getPath(), iSelectionGroupIndex * iSpace, 0, self.iCityButtonSize, self.iCityButtonSize, WidgetTypes.WIDGET_GENERAL, -1, -1, False )

					iPosition += 20
					SelectedTransportID = self.selectedSelectionGroupHeadUnitID
					if (SelectedTransportID != -1):
						if (SelectedTransportID == SelectionGroup.getHeadUnit().getID()):
						
							if (self.RouteValidity[iSelectionGroupIndex][iRow]):
								if (SelectionGroup.isAssignedTradeRoute(pRoute.getID())):
									screen.setState(szButtonName, True)
									screen.setButtonGFC("RouteToggle" + str(iRow), "-", "", 0, 0, 60, 30, WidgetTypes.WIDGET_ASSIGN_TRADE_ROUTE, self.selectedSelectionGroupHeadUnitID, iRoute, ButtonStyles.BUTTON_STYLE_STANDARD )
								else:
									screen.setButtonGFC("RouteToggle" + str(iRow), "+", "", 0, 0, 60, 30, WidgetTypes.WIDGET_ASSIGN_TRADE_ROUTE, self.selectedSelectionGroupHeadUnitID, iRoute, ButtonStyles.BUTTON_STYLE_STANDARD )
								screen.attachControlToTableCell("RouteToggle" + str(iRow), szState + "ListBackground", iRow, 3 )
	
	def BuildAllTransportsRow(self, iRow):
		screen = CyGInterfaceScreen("DomesticAdvisor", CvScreenEnums.DOMESTIC_ADVISOR)

		if(self.CurrentState == self.TRADEROUTE_STATE):
			szState = self.StateTypes[self.CurrentState]
			player = gc.getPlayer(gc.getGame().getActivePlayer())

			screen.addPanel("AllTransportPanel", u"", u"", True, False, 0, 0, (390 * self.nTableWidth) / self.nNormalizedTableWidth, 20, PanelStyles.PANEL_STYLE_EMPTY, WidgetTypes.WIDGET_GENERAL, iRow, -1)
			screen.attachControlToTableCell("AllTransportPanel", szState + "ListBackground", iRow, 4 )

			iSpace = self.iCityButtonSize
			if (len(self.Transports)):
				iSpace = (self.nTableWidth - 390) / len(self.Transports)
				if iSpace > self.iCityButtonSize:
					iSpace = self.iCityButtonSize

			SelectionState = False
			iPosition = 10
			SelectedTransportID = -1
			for iSelectionGroupIndex in range(len(self.Transports) -1, -1, -1):
				SelectionGroup = self.Transports[iSelectionGroupIndex]

				if (not SelectionGroup.isNone()):
					GroupHeadID = SelectionGroup.getHeadUnit().getID()
					szButtonName = "AllTransport" + str(SelectionGroup.getID())
					screen.addCheckBoxGFCAt("AllTransportPanel", szButtonName, SelectionGroup.getHeadUnit().getButton(), ArtFileMgr.getInterfaceArtInfo("BUTTON_HILITE_SQUARE").getPath(), iSelectionGroupIndex * iSpace, 0, self.iCityButtonSize, self.iCityButtonSize, WidgetTypes.WIDGET_GENERAL, 10001, SelectionGroup.getHeadUnit().getID(), ButtonStyles.BUTTON_STYLE_LABEL )
					screen.setLabelAt("SelectionGroupCount" + str(SelectionGroup.getID()), szButtonName, "<font=4>" + str(SelectionGroup.getNumUnits()) + "</font>", CvUtil.FONT_LEFT_JUSTIFY, 35, 35, -1.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )

					iPosition += 20
					SelectedTransportID = self.selectedSelectionGroupHeadUnitID
					if (SelectedTransportID != -1):
						if (SelectedTransportID == SelectionGroup.getHeadUnit().getID()):
							screen.setState(szButtonName, True)
							SelectionState = True
							
	def RebuildArrays (self):
		#Get a list of the Players Cities
		player = gc.getPlayer(gc.getGame().getActivePlayer())
		self.Cities = []
		(pLoopCity, iter) = player.firstCity(false)
		while(pLoopCity):
			self.Cities.append(pLoopCity)
			(pLoopCity, iter) = player.nextCity(iter, false)

		self.Routes = []
		for iRoute in range(player.getNumTradeRoutes()):
			self.Routes.append(player.getTradeRouteByIndex(iRoute))

		self.Transports = []
		SelectionGroup, Iterator = player.firstSelectionGroup(false)
		while (SelectionGroup != None):
			if (SelectionGroup.canAssignTradeRoute(-1, false)):
				self.Transports.append(SelectionGroup)		
			SelectionGroup, Iterator = player.nextSelectionGroup(Iterator, false)

		self.RouteValidity = []
		for iTransport in range(len(self.Transports)):
			Transport = self.Transports[iTransport]
			RouteValidArray = []
			bReusePath = false
			for Route in self.Routes:
				RouteValidArray.append(Transport.canAssignTradeRoute(Route.getID(), bReusePath))
				bReusePath = true
			self.RouteValidity.append(RouteValidArray)

	def RebuildTransportTable (self):
		player = gc.getPlayer(gc.getGame().getActivePlayer())
		screen = CyGInterfaceScreen( "DomesticAdvisor", CvScreenEnums.DOMESTIC_ADVISOR )

		szStateName = self.StateTypes[self.TRADEROUTE_STATE]
		screen.setImageButton( szStateName + "Button", ArtFileMgr.getInterfaceArtInfo(self.StateButtons[self.TRADEROUTE_STATE]).getPath(), (self.iButtonSpacing * 5) + (self.iButtonSpacing / 2), self.Y_LOWER_ROW, self.iButtonSize, self.iButtonSize, WidgetTypes.WIDGET_GENERAL, 5, -1 )
		
		screen.addTableControlGFC( szStateName + "ListBackground", 19, (self.nScreenWidth - self.nTableWidth) / 2, 60, self.nTableWidth, self.nTableHeight, True, False, self.iCityButtonSize, self.iCityButtonSize, TableStyles.TABLE_STYLE_STANDARD )
		screen.enableSelect( szStateName + "ListBackground", True )
		screen.enableSort( szStateName + "ListBackground" )
		screen.setStyle( szStateName + "ListBackground", "Table_StandardCiv_Style")
		screen.hide( szStateName + "ListBackground")
		screen.setTableColumnHeader( szStateName + "ListBackground", 0, "", (30 * self.nTableWidth) / self.nNormalizedTableWidth )
		screen.setTableColumnHeader( szStateName + "ListBackground", 1, "<font=2>" + localText.getText("TXT_KEY_DOMESTIC_ADVISOR_NAME", ()) + "</font>", (221 * self.nTableWidth) / self.nNormalizedTableWidth )
		
	def RebuildRouteTable (self):
		player = gc.getPlayer(gc.getGame().getActivePlayer())
		screen = CyGInterfaceScreen( "DomesticAdvisor", CvScreenEnums.DOMESTIC_ADVISOR )

		szTableName = self.StateTypes[self.TRADEROUTE_STATE] + "ListBackground"
		screen.setImageButton( szTableName + "Button", ArtFileMgr.getInterfaceArtInfo(self.StateButtons[self.TRADEROUTE_STATE]).getPath(), (self.iButtonSpacing * 5) + (self.iButtonSpacing / 2), self.Y_LOWER_ROW, self.iButtonSize, self.iButtonSize, WidgetTypes.WIDGET_GENERAL, 5, -1 )

		screen.addTableControlGFC( szTableName, 19, (self.nScreenWidth - self.nTableWidth) / 2, 60, self.nTableWidth, self.nTableHeight, True, False, self.iCityButtonSize, self.iCityButtonSize, TableStyles.TABLE_STYLE_STANDARD )
		screen.enableSelect( szTableName, True )
		screen.enableSort( szTableName )
		screen.setStyle( szTableName, "Table_StandardCiv_Style" )
		screen.hide( szTableName )

		screen.setTableColumnHeader( szTableName, 0, localText.getText("TXT_KEY_SOURCE", ()), (150 * self.nTableWidth) / self.nNormalizedTableWidth)
		screen.setTableColumnHeader( szTableName, 1, localText.getText("TXT_KEY_DESTINATION", ()), (150 * self.nTableWidth) / self.nNormalizedTableWidth)
		screen.setTableColumnHeader( szTableName, 2, localText.getText("TXT_KEY_YIELD", ()), (65 * self.nTableWidth) / self.nNormalizedTableWidth)
		screen.setTableColumnHeader( szTableName, 3, " +/-", (45 * self.nTableWidth) / self.nNormalizedTableWidth)
		screen.setTableColumnHeader( szTableName, 4, localText.getText("TXT_KEY_TRANSPORT_PANEL", ()), self.nTableWidth - 390)

		for iRoute in range(len(self.Routes)):
			screen.appendTableRow( szTableName )
			screen.setTableRowHeight(szTableName, iRoute, self.ROW_HIGHT)

		screen.appendTableRow( szTableName )
		screen.setTableRowHeight(szTableName, len(self.Routes), self.ROW_HIGHT)

	# Will handle the input for this screen...
	def handleInput (self, inputClass):
		' Calls function mapped in DomesticAdvisorInputMap'
		screen = CyGInterfaceScreen( "DomesticAdvisor", CvScreenEnums.DOMESTIC_ADVISOR )
		if ( inputClass.getNotifyCode() == NotifyCode.NOTIFY_LISTBOX_ITEM_SELECTED ):
			if self.CurrentState != self.TRADEROUTE_STATE:
				if (inputClass.getMouseX() == 0):
					screen = CyGInterfaceScreen( "DomesticAdvisor", CvScreenEnums.DOMESTIC_ADVISOR )
					screen.hideScreen()

					CyInterface().selectCity(gc.getPlayer(inputClass.getData1()).getCity(inputClass.getData2()), true);

					popupInfo = CyPopupInfo()
					popupInfo.setButtonPopupType(ButtonPopupTypes.BUTTONPOPUP_PYTHON_SCREEN)
					popupInfo.setText(u"showDomesticAdvisor")
					popupInfo.addPopup(inputClass.getData1())
				else:
					self.updateAppropriateCitySelection()

		if (inputClass.getNotifyCode() == NotifyCode.NOTIFY_CLICKED):
			if (inputClass.getButtonType() == WidgetTypes.WIDGET_GENERAL):
				iData = inputClass.getData1()
				if (iData >= 0 and iData < len(self.StateTypes)):
					if(self.CurrentState != iData):
						screen.hide(self.StateTypes[self.CurrentState] + "ListBackground")
						self.CurrentState = iData
						self.drawContents()
				if (iData == 10001):
					if (self.selectedSelectionGroupHeadUnitID == inputClass.getData2()):
						self.selectedSelectionGroupHeadUnitID = -1
						self.drawContents()
					else:
						self.selectedSelectionGroupHeadUnitID = inputClass.getData2()
						self.drawContents()
		return 0

	def updateAppropriateCitySelection(self):
		nCities = gc.getPlayer(gc.getGame().getActivePlayer()).getNumCities()
		screen = CyGInterfaceScreen( "DomesticAdvisor", CvScreenEnums.DOMESTIC_ADVISOR )
		screen.updateAppropriateCitySelection( self.StateTypes[self.CurrentState] + "ListBackground", nCities, 1 )
		self.listSelectedCities = []
		for iCity in range(nCities):
			if screen.isRowSelected(self.StateTypes[self.CurrentState] + "ListBackground", iCity):
				self.listSelectedCities.append(screen.getTableText(self.StateTypes[self.CurrentState] + "ListBackground", 2, iCity))

	def update(self, fDelta):
		if (CyInterface().isDirty(InterfaceDirtyBits.Domestic_Advisor_DIRTY_BIT)):
			CyInterface().setDirty(InterfaceDirtyBits.Domestic_Advisor_DIRTY_BIT, False)
			player = gc.getPlayer(gc.getGame().getActivePlayer())

			self.RebuildArrays()
			
			for iCity in range(len(self.Cities)):
				self.updateCityTable(self.Cities[iCity], iCity)

			self.RebuildRouteTable()
			self.RebuildTransportTable()

			self.drawContents()
			
	def getWidgetHelp(self, argsList):
		iScreen, eWidgetType, iData1, iData2, bOption = argsList

		if eWidgetType == WidgetTypes.WIDGET_GENERAL:
			if iData1 == self.GENERAL_STATE:
				return localText.getText("TXT_KEY_DOMESTIC_ADVISOR_STATE_GENERAL", ())
			elif iData1 == self.PRODUCTION_STATE:
				return localText.getText("TXT_KEY_CONCEPT_PRODUCTION", ())
			elif iData1 == self.BUILDING_STATE:
				return localText.getText("TXT_KEY_BUILDINGS", ())
			elif iData1 == self.WAREHOUSE_STATE:
				return localText.getText("TXT_KEY_BUILDING_WAREHOUSE", ())
			elif iData1 == self.CITIZEN_STATE:
				return localText.getText("TXT_KEY_DOMESTIC_ADVISOR_STATE_CITIZEN", ())
			elif iData1 == self.IMPORTEXPORT_STATE:
				return localText.getText("TXT_KEY_CONCEPT_TRADE_ROUTE", ())
			elif iData1 == self.TRADEROUTE_STATE:
				return localText.getText("TXT_KEY_DOMESTIC_ADVISOR_STATE_TRADEROUTE", ())
			elif iData1 == 10001:
				unit = gc.getActivePlayer().getUnit(iData2)
				if not unit.isNone():
					return CyGameTextMgr().getSpecificUnitHelp(unit, true, false)
