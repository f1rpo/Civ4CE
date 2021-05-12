## Sid Meier's Civilization 4
## Copyright Firaxis Games 2005
from CvPythonExtensions import *
import PyHelpers
import CvUtil
import ScreenInput
import CvScreenEnums

PyPlayer = PyHelpers.PyPlayer
PyCity = PyHelpers.PyCity

#	IMPORTANT INFORMATION
#	
#	All widget names MUST be unique when creating screens.  If you create
#	a widget named 'Hello', and then try to create another named 'Hello', it
#	will modify the first hello.
#
#	Also, when attaching widgets, 'Background' is a reserve word meant for
#	the background widget.  Do NOT use 'Background' to name any widget, but
#	when attaching to the background, please use the 'Background' keyword.

#  Thanks to Lee Reeves, AKA Taelis on civfanatics.com


# globals
gc = CyGlobalContext()
ArtFileMgr = CyArtFileMgr()
localText = CyTranslator()

#	Set up the base Coordinates here...
STANDARD_Z = -2.2
TOP_Y = 96
Y_SPACING = 32
DATE_COLUMN = 70
NAME_COLUMN = DATE_COLUMN + 90
CULTURE_COLUMN = NAME_COLUMN + 130
HAPPY_COLUMN = CULTURE_COLUMN + 90
HEALTH_COLUMN = HAPPY_COLUMN + 100
FOOD_COLUMN = HEALTH_COLUMN + 40
PRODUCTION_COLUMN = FOOD_COLUMN + 40
GOLD_COLUMN = PRODUCTION_COLUMN + 40
GREATPEOPLE_COLUMN = GOLD_COLUMN + 40
PRODUCING_COLUMN = GREATPEOPLE_COLUMN + 170


# Sorting information
DATE = 0
NAME = 1
CULTURE = 2
HAPPY = 3
HEALTH = 4
FOOD = 5
PRODUCTION = 6
GOLD = 7
GREATPEOPLE = 8
PRODUCING = 9
POPULATION = 10
UNHAPPY = 11
ANGRY = 12
BAD_HEALTH = 13
CONSUMPTION = 14
SCIENCE = 15
CULTURE_RATE = 16
GREATPEOPLE_RATE = 17

# What are we sorting by?
iSortingBy = DATE
iSortingDown = True

# Listbox identifiers...
UNIT_CHOSEN = 0
BUILDING_CHOSEN = 1
PROJECT_CHOSEN = 2
PROCESS_CHOSEN = 3

class CvDomesticAdvisor:
	"Domestic Advisor Screen"
	def __init__(self):
		self.listSelectedCities = []
		
		self.nFirstSpecialistX = 30
		self.nSpecialistY = 540
		self.nSpecialistWidth = 32
		self.nSpecialistLength = 32
		self.nSpecialistDistance = 100

		# Offset from Specialist Image/Size for the Specialist Plus/Minus buttons
		self.nPlusOffsetX = -4
		self.nMinusOffsetX = 16
		self.nPlusOffsetY = self.nMinusOffsetY = 30
		self.nPlusWidth = self.nPlusHeight = self.nMinusWidth = self.nMinusHeight = 20

		# Offset from Specialist Image for the Specialist Text
		self.nSpecTextOffsetX = 40
		self.nSpecTextOffsetY = 10

	# Screen construction function
	def interfaceScreen(self):
	
		# This will get the actual Player class into iPlayer
		iPlayer = PyPlayer(CyGame().getActivePlayer())
		
		# Create a new screen, called DomesticAdvisur, using the file CvDomesticAdvisor.py for input
		screen = CyGInterfaceScreen( "DomesticAdvisor", CvScreenEnums.DOMESTIC_ADVISOR )
		screen.setDimensions(screen.centerX(0), screen.centerY(0), 1024, 768)
		screen.showScreen(PopupStates.POPUPSTATE_IMMEDIATE, False)
	
		# Here we set the background widget and exit button, and we show the screen
		screen.addPanel( "DomesticAdvisorBG", u"", u"", True, False, 0, 48, 1024, 548, PanelStyles.PANEL_STYLE_MAIN )
		screen.setText("DomesticExit", "Background", localText.getText("TXT_KEY_PEDIA_SCREEN_EXIT", ()).upper(), CvUtil.FONT_RIGHT_JUSTIFY, 1012, 564, -0.1, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_CLOSE_SCREEN, -1, -1 )
	
		# Header...
		#szText = "<font=4>" + localText.getText("TXT_KEY_DOMESTIC_ADVISOR_TITLE", ()).upper() + "</font>"
		#screen.setLabel( "DomesticTitleHeader", "Background", szText, CvUtil.FONT_CENTER_JUSTIFY, 472, 40, STANDARD_Z, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )

		# Erase the flag?
		CyInterface().setDirty(InterfaceDirtyBits.MiscButtons_DIRTY_BIT, True)

		# Draw the city list...
		self.drawContents( )
		
	# headers...
	def drawHeaders( self ):

		# Get the screen and the player
		screen = CyGInterfaceScreen( "DomesticAdvisor", CvScreenEnums.DOMESTIC_ADVISOR )
		
		# Zoom to City
		screen.setTableColumnHeader( "CityListBackground", 0, "", 30 )
		
		# Date Founded Column
		screen.setTableColumnHeader( "CityListBackground", 1, "<font=2>" + localText.getText("TXT_KEY_DOMESTIC_ADVISOR_FOUNDED", ()) + "</font>", 85 )
		
		# Name Column
		screen.setTableColumnHeader( "CityListBackground", 2, "<font=2>" + localText.getText("TXT_KEY_DOMESTIC_ADVISOR_NAME", ()) + "</font>", 101 )
		
		# Population Column
		screen.setTableColumnHeader( "CityListBackground", 3, "<font=2>" + localText.getText("TXT_KEY_POPULATION", ()) + "</font>", 40 )
		
		# Angry Column
		screen.setTableColumnHeader( "CityListBackground", 4, "<font=2>" + (u"%c" % CyGame().getSymbolID(FontSymbols.ANGRY_POP_CHAR)) + "</font>", 40 )
		
		# Happiness Column
		screen.setTableColumnHeader( "CityListBackground", 5, "<font=2>" + (u"%c" % CyGame().getSymbolID(FontSymbols.HAPPY_CHAR)) + "</font>", 40 )
		
		# Unhappiness Column
		screen.setTableColumnHeader( "CityListBackground", 6, "<font=2>" + (u"%c" % CyGame().getSymbolID(FontSymbols.UNHAPPY_CHAR)) + "</font>", 40 )
		
		# Health Column
		screen.setTableColumnHeader( "CityListBackground", 7, "<font=2>" + (u"%c" % CyGame().getSymbolID(FontSymbols.HEALTHY_CHAR)) + "</font>", 40 )
		
		# Bad Health Column
		screen.setTableColumnHeader( "CityListBackground", 8, "<font=2>" + (u"%c" % CyGame().getSymbolID(FontSymbols.UNHEALTHY_CHAR)) + "</font>", 40 )
		
		# Food Column
		screen.setTableColumnHeader( "CityListBackground", 9, "<font=2>" + (u"%c" % gc.getYieldInfo(YieldTypes.YIELD_FOOD).getChar()) + "</font>", 40 )
		
		# Food Consumed Column
		screen.setTableColumnHeader( "CityListBackground", 10, "<font=2>" + (u"%c" % CyGame().getSymbolID(FontSymbols.EATEN_FOOD_CHAR)) + "</font>", 40 )
		
		# Production Column
		screen.setTableColumnHeader( "CityListBackground", 11, "<font=2>" + (u"%c" % gc.getYieldInfo(YieldTypes.YIELD_PRODUCTION).getChar()) + "</font>", 40 )
		
		# Gold Column
		screen.setTableColumnHeader( "CityListBackground", 12, "<font=2>" + (u"%c" % gc.getYieldInfo(YieldTypes.YIELD_COMMERCE).getChar()) + "</font>", 40 )
		
		# Research Column
		szText = u"%c" %(gc.getCommerceInfo(CommerceTypes.COMMERCE_RESEARCH).getChar())
		screen.setTableColumnHeader( "CityListBackground", 13, "<font=2>" + szText, 40 )
		
		# Culture Column
		screen.setTableColumnHeader( "CityListBackground", 14, "<font=2>" + (u"%c" % gc.getCommerceInfo(CommerceTypes.COMMERCE_CULTURE).getChar()) + "</font>", 40 )
		
		# Culture Column
		screen.setTableColumnHeader( "CityListBackground", 15, "<font=2>" + localText.getText("TXT_KEY_TOTAL", ()) + "</font>", 55 )
		
		# Great Person Column
		screen.setTableColumnHeader( "CityListBackground", 16, "<font=2>" + (u"%c" % CyGame().getSymbolID(FontSymbols.GREAT_PEOPLE_CHAR)) + "</font>", 40 )
		
		# Great Person Column
		screen.setTableColumnHeader( "CityListBackground", 17, "<font=2>" + localText.getText("TXT_KEY_TOTAL", ()) + "</font>", 55 )
		
		# Production Column
		screen.setTableColumnHeader( "CityListBackground", 18, "<font=2>" + localText.getText("TXT_KEY_DOMESTIC_ADVISOR_PRODUCING", ()) + "</font>", 132 )
	
	# Function to draw the contents of the cityList passed in
	def drawContents (self):
	
		# Get the screen and the player
		screen = CyGInterfaceScreen( "DomesticAdvisor", CvScreenEnums.DOMESTIC_ADVISOR )
		iPlayer = PyPlayer(CyGame().getActivePlayer())
		
		screen.moveToFront( "Background" )
		
		# Build the table	
		screen.addTableControlGFC( "CityListBackground", 19, 22, 61, 980, 476, True, False, 24, 24, TableStyles.TABLE_STYLE_STANDARD )
		screen.enableSelect( "CityListBackground", True )
		screen.enableSort( "CityListBackground" )
		screen.setStyle("CityListBackground", "Table_StandardCiv_Style")

		# Loop through the cities
		cityList = iPlayer.getCityList()
		i = 0
		for pLoopCity in cityList:

			screen.appendTableRow( "CityListBackground" )
	
			if (pLoopCity.getName() in self.listSelectedCities):
				screen.selectRow( "CityListBackground", i, True )
						
			self.updateTable(pLoopCity, i)

			i += 1
		
		self.drawHeaders()
		
		self.drawSpecialists()
		
		screen.moveToBack( "DomesticAdvisorBG" )
		
		self.updateAppropriateCitySelection(len(cityList))
		
		CyInterface().setDirty(InterfaceDirtyBits.Domestic_Advisor_DIRTY_BIT, true)

	def updateTable(self, pLoopCity, i):

		screen = CyGInterfaceScreen( "DomesticAdvisor", CvScreenEnums.DOMESTIC_ADVISOR )

		screen.setTableText( "CityListBackground", 0, i, "", ArtFileMgr.getInterfaceArtInfo("INTERFACE_BUTTONS_CITYSELECTION").getPath(), WidgetTypes.WIDGET_ZOOM_CITY, pLoopCity.getOwner(), pLoopCity.getID(), CvUtil.FONT_LEFT_JUSTIFY);

		# Founded date first...
		szIDText = "Date" + str(pLoopCity.getID())
		
		iTurnTime = pLoopCity.getGameTurnFounded()
		szFounded = ""
		szFounded = szFounded + unicode(CyGameTextMgr().getTimeStr(iTurnTime, false))
		if pLoopCity.isCapital():
			szFounded = szFounded + (u"%c" % CyGame().getSymbolID(FontSymbols.STAR_CHAR))
		
		screen.setTableDate( "CityListBackground", 1, i, szFounded, "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
		
		# City name...
		screen.setTableText( "CityListBackground", 2, i, pLoopCity.getName(), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
		
		# Population
		screen.setTableInt( "CityListBackground", 3, i, unicode(pLoopCity.getPopulation()), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )

		# Angry...	
		screen.setTableInt( "CityListBackground", 4, i, unicode(pLoopCity.getAngryPopulation()), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )

		# Happiness...
		screen.setTableInt( "CityListBackground", 5, i, unicode(pLoopCity.getHappyPopulation()), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )

		# Unhappiness...
		screen.setTableInt( "CityListBackground", 6, i, unicode(pLoopCity.getUnhappyPopulation()), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )

		# Health...
		screen.setTableInt( "CityListBackground", 7, i, unicode(pLoopCity.getGoodHealth()), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )

		# Unhealthy...			
		screen.setTableInt( "CityListBackground", 8, i, unicode(pLoopCity.getBadHealth()), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )

		# Food status...
		screen.setTableInt( "CityListBackground", 9, i, unicode(pLoopCity.getFoodRate()), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
		
		# Food consumed
		screen.setTableInt( "CityListBackground", 10, i, unicode(pLoopCity.foodConsumption( False, 0 )), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )

		# Production status...
		screen.setTableInt( "CityListBackground", 11, i, unicode(pLoopCity.getProductionRate()), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )

		# Gold status...
		screen.setTableInt( "CityListBackground", 12, i, unicode(pLoopCity.calculateGoldRate()), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )

		# Science rate...
		screen.setTableInt( "CityListBackground", 13, i, unicode(pLoopCity.getResearchRate()), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )

		# Culture status...
		screen.setTableInt( "CityListBackground", 14, i, unicode(pLoopCity.getCultureCommerce()), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )

		# Total Culture			
		screen.setTableInt( "CityListBackground", 15, i, unicode(pLoopCity.getCulture()), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )

		# Great Person
		screen.setTableInt( "CityListBackground", 16, i, unicode(pLoopCity.getGreatPeopleRate()), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )

		# Great Person
		screen.setTableInt( "CityListBackground", 17, i, unicode(pLoopCity.getGreatPeopleProgress()), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )

		# Producing	
		screen.setTableText( "CityListBackground", 18, i, pLoopCity.getProductionName() + " (" + str(pLoopCity.getGeneralProductionTurnsLeft()) + ")", "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
		
		
	# Draw the specialist and their increase and decrease buttons
	def drawSpecialists(self):
		screen = CyGInterfaceScreen( "DomesticAdvisor", CvScreenEnums.DOMESTIC_ADVISOR )

		for i in range( gc.getNumSpecialistInfos() ):
			if (gc.getSpecialistInfo(i).isVisible()):			
				szName = "SpecialistImage" + str(i)
				screen.setImageButton( szName, gc.getSpecialistInfo(i).getTexture(), self.nFirstSpecialistX + (self.nSpecialistDistance * i), self.nSpecialistY, self.nSpecialistWidth, self.nSpecialistLength, WidgetTypes.WIDGET_CITIZEN, i, -1 )
				screen.hide(szName)

				szName = "SpecialistPlus" + str(i)
				screen.setButtonGFC( szName, u"", "", self.nFirstSpecialistX + (self.nSpecialistDistance * i) + self.nPlusOffsetX, self.nSpecialistY + self.nPlusOffsetY, self.nPlusWidth, self.nPlusHeight, WidgetTypes.WIDGET_CHANGE_SPECIALIST, i, 1, ButtonStyles.BUTTON_STYLE_CITY_PLUS )
				screen.hide(szName)

				szName = "SpecialistMinus" + str(i)
				screen.setButtonGFC( szName, u"", "", self.nFirstSpecialistX + (self.nSpecialistDistance * i) + self.nMinusOffsetX, self.nSpecialistY + self.nMinusOffsetY, self.nMinusWidth, self.nMinusHeight, WidgetTypes.WIDGET_CHANGE_SPECIALIST, i, -1, ButtonStyles.BUTTON_STYLE_CITY_MINUS )
				screen.hide(szName)

				szName = "SpecialistText" + str(i)
				screen.setLabel(szName, "Background", "", CvUtil.FONT_LEFT_JUSTIFY, self.nFirstSpecialistX + (self.nSpecialistDistance * i) + self.nSpecTextOffsetX, self.nSpecialistY + self.nSpecTextOffsetY, 0, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
				screen.hide(szName)

	def hideSpecialists(self):
		screen = CyGInterfaceScreen( "DomesticAdvisor", CvScreenEnums.DOMESTIC_ADVISOR )
		for i in range( gc.getNumSpecialistInfos() ):
			if (gc.getSpecialistInfo(i).isVisible()):			
				screen.hide("SpecialistImage" + str(i))
				screen.hide("SpecialistPlus" + str(i))
				screen.hide("SpecialistMinus" + str(i))
				screen.hide("SpecialistText" + str(i))

	def updateSpecialists(self):
		""" Function which shows the specialists."""
		screen = CyGInterfaceScreen( "DomesticAdvisor", CvScreenEnums.DOMESTIC_ADVISOR )

		if (CyInterface().isOneCitySelected()):
		
			city = CyInterface().getHeadSelectedCity()
			nPopulation = city.getPopulation()
			nFreeSpecial = city.totalFreeSpecialists()

			for i in range( gc.getNumSpecialistInfos() ):
				if (gc.getSpecialistInfo(i).isVisible()):	
					szName = "SpecialistImage" + str(i)
					screen.show(szName)
					
					szName = "SpecialistText" + str(i)
					screen.setLabel(szName, "Background", str (city.getSpecialistCount(i)) + "/" + str(city.getMaxSpecialistCount(i)), CvUtil.FONT_LEFT_JUSTIFY, self.nFirstSpecialistX + (self.nSpecialistDistance * i) + self.nSpecTextOffsetX, self.nSpecialistY + self.nSpecTextOffsetY, 0, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
					screen.show(szName)

					# If the specialist is valid and we can increase it
					szName = "SpecialistPlus" + str(i)
					if (city.isSpecialistValid(i, 1) and (city.getForceSpecialistCount(i) < (nPopulation + nFreeSpecial))):
						screen.show(szName)
					else:
						screen.hide(szName)

					# if we HAVE specialists already and they're not forced.
					szName = "SpecialistMinus" + str(i)
					if (city.getSpecialistCount(i) > 0 or city.getForceSpecialistCount(i) > 0):
						screen.show(szName)
					else:
						screen.hide(szName)
		else:
			self.hideSpecialists()
				
	# Will handle the input for this screen...
	def handleInput (self, inputClass):
		' Calls function mapped in DomesticAdvisorInputMap'
		# only get from the map if it has the key
		
		if ( inputClass.getNotifyCode() == NotifyCode.NOTIFY_LISTBOX_ITEM_SELECTED ):
			if (inputClass.getMouseX() == 0):
				screen = CyGInterfaceScreen( "DomesticAdvisor", CvScreenEnums.DOMESTIC_ADVISOR )
				screen.hideScreen()
				
				CyInterface().selectCity(gc.getPlayer(inputClass.getData1()).getCity(inputClass.getData2()), true);
				
				popupInfo = CyPopupInfo()
				popupInfo.setButtonPopupType(ButtonPopupTypes.BUTTONPOPUP_PYTHON_SCREEN)
				popupInfo.setText(u"showDomesticAdvisor")
				popupInfo.addPopup(inputClass.getData1())		
			else:
				self.updateAppropriateCitySelection(len(PyPlayer(CyGame().getActivePlayer()).getCityList()))
				self.updateSpecialists()
			
		return 0
	
	def updateAppropriateCitySelection(self, nCities):
		screen = CyGInterfaceScreen( "DomesticAdvisor", CvScreenEnums.DOMESTIC_ADVISOR )
		screen.updateAppropriateCitySelection( "CityListBackground", nCities, 2 )
		self.listSelectedCities = []
		for i in range(nCities):
			if screen.isRowSelected("CityListBackground", i):
				self.listSelectedCities.append(screen.getTableText("CityListBackground", 2, i))
								
	def update(self, fDelta):
		if (CyInterface().isDirty(InterfaceDirtyBits.Domestic_Advisor_DIRTY_BIT) == True):
			CyInterface().setDirty(InterfaceDirtyBits.Domestic_Advisor_DIRTY_BIT, False)
			
			screen = CyGInterfaceScreen( "DomesticAdvisor", CvScreenEnums.DOMESTIC_ADVISOR )
			iPlayer = PyPlayer(CyGame().getActivePlayer())

			cityList = iPlayer.getCityList()
			for i in range(len(cityList)):
			
				pLoopCity = PyCity( CyGame().getActivePlayer(), cityList[i].getID() )

				#screen.setTableTextKey( "CityListBackground", 18, pLoopCity.getName(), 2, pLoopCity.getProductionName() + " (" + str(pLoopCity.getGeneralProductionTurnsLeft()) + ")", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY, len(cityList) )	
				self.updateTable(pLoopCity, i)
				
			self.updateSpecialists()
		
		return