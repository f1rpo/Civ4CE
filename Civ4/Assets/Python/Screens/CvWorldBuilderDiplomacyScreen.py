## Sid Meier's Civilization 4
## Copyright Firaxis Games 2005
from CvPythonExtensions import *
import CvUtil
import ScreenInput
import CvScreenEnums
import time

gc = CyGlobalContext()
ArtFileMgr = CyArtFileMgr()

class CvWorldBuilderDiplomacyScreen:
	"World Builder Screen"

	def __init__ (self) :
		self.m_tabCtrl = 0
		self.m_iActivePlayer = 0
		self.m_bShown = False
		return

	def interfaceScreen (self):
		# This is the main interface screen, create it as such
		screen = CyGInterfaceScreen( "WorldBuilderDiplomacyScreen", CvScreenEnums.WORLDBUILDER_DIPLOMACY_SCREEN )	

		# This should show the screen immidiately and pass input to the game
		screen.showScreen(PopupStates.POPUPSTATE_IMMEDIATE, True)
		self.m_bShown = True

		iMaxScreenWidth = 800
		iScreenWidth = 150*self.getNumPlayers()
		if (iScreenWidth > iMaxScreenWidth):
			iScreenWidth = iMaxScreenWidth
		
		iScreenHeight = 175

		screen.addPanel( "WorldBuilderDiplomacyMainPanel", "", "", False, True, 200, 325, iScreenWidth, iScreenHeight, PanelStyles.PANEL_STYLE_EXTERNAL )		

		self.refreshLeftSide()
		self.refreshRightSide()

		# This should be a forced redraw screen
		screen.setForcedRedraw( True )
		return 0

	def killScreen(self):
		if (self.m_bShown):
			screen = CyGInterfaceScreen( "WorldBuilderDiplomacyScreen", CvScreenEnums.WORLDBUILDER_DIPLOMACY_SCREEN )	
			screen.hideScreen()
			self.m_bShown = False
		return

	def handleInput (self, inputClass):
		if (inputClass.getNotifyCode() == NotifyCode.NOTIFY_LISTBOX_ITEM_SELECTED):
			if (inputClass.getFunctionName() == "WorldBuilderDiplomacyPlayerChoice"):
				self.handlePlayerPullDownCB(inputClass.getData())
			elif (inputClass.getFunctionName() == "WorldBuilderDiplomacyWPChoice"):
				self.setAtWar(inputClass.getID(), inputClass.getData())
		elif (inputClass.getNotifyCode() == NotifyCode.NOTIFY_SLIDER_NEWSTOP):
			print("inside: NOTIFY_SLIDER_NEWSTOP")
			self.handleAIAttitudeChange(inputClass.getFunctionName(), inputClass.getID(), inputClass.getData())
		return 1

	def mouseOverPlot (self, argsList):
		return

	def leftMouseDown (self, argsList):
		return 1

	def rightMouseDown (self, argsList):
		return 1

	def update(self, fDelta):
		return

	# Will update the screen (every 250 MS)
	def updateScreen(self):
		return 0

	def redraw( self ):
		return 0

	def refreshLeftSide(self):
		if ( self.m_bShown ):
			screen = CyGInterfaceScreen( "WorldBuilderDiplomacyScreen", CvScreenEnums.WORLDBUILDER_DIPLOMACY_SCREEN )	
			screen.attachPanel( "WorldBuilderDiplomacyMainPanel", "WorldBuilderDiplomacyLeftPanel", "", "", True, True, PanelStyles.PANEL_STYLE_IN )

			self.removeLeftSideWidget()
			szDropDownName = "WorldBuilderDiplomacyPlayerChoice"
			screen.attachDropDownBoxGFC("WorldBuilderDiplomacyLeftPanel", szDropDownName, False)
			for i in range( gc.getMAX_CIV_PLAYERS() ):
				if ( gc.getPlayer(i).isEverAlive() ):
					if (i == self.m_iActivePlayer):
						screen.addPullDownString(szDropDownName, gc.getPlayer(i).getName(), i, i, True )
					else:
						screen.addPullDownString(szDropDownName, gc.getPlayer(i).getName(), i, i, False )

			screen.attachTextGFC( "WorldBuilderDiplomacyLeftPanel", "WorldBuilderDiplomacyLeftPanelAW", "At War?", FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
			screen.attachTextGFC( "WorldBuilderDiplomacyLeftPanel", "WorldBuilderDiplomacyLeftPanelAIAttitude", "AI Attitude Weight", FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
			#screen.attachTextGFC( "WorldBuilderDiplomacyLeftPanel", "WorldBuilderDiplomacyLeftPanelRoP", "Right of Passage?", FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
			#screen.attachTextGFC( "WorldBuilderDiplomacyLeftPanel", "WorldBuilderDiplomacyLeftPanelMP", "Mutual Protection?", FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
			#screen.attachTextGFC( "WorldBuilderDiplomacyLeftPanel", "WorldBuilderDiplomacyLeftPanelTG", "Trading", FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )

		return

	def refreshRightSide(self):
		if ( self.m_bShown ):
			screen = CyGInterfaceScreen( "WorldBuilderDiplomacyScreen", CvScreenEnums.WORLDBUILDER_DIPLOMACY_SCREEN )	
			
			iCount = -1
			iPanelCount = -1
			for i in range( gc.getMAX_CIV_PLAYERS() ):
				if ( gc.getPlayer(i).isEverAlive() ):
					iCount = iCount + 1
					if (iCount != self.m_iActivePlayer):
						iPanelCount = iPanelCount+1
						panelName = self.getRightSidePanelName(iPanelCount)
						self.removeRightSideWidget(iPanelCount)
						screen.deleteWidget(panelName)
						screen.attachPanel( "WorldBuilderDiplomacyMainPanel", panelName, "", "", True, True, PanelStyles.PANEL_STYLE_IN )
						self.addTextToRightSidePanel(panelName, gc.getPlayer(i).getName())
			self.addRightSideWarPeaceText()
			self.addRightSideAIAttitude()
			#self.addRightSideOBText()
			#self.addRightSideMutualProtectionText()
			#self.addRightSideTradingText()
		return

	def addTextToRightSidePanel(self, szPanel, szText):
		if ( self.m_bShown ):
			screen = CyGInterfaceScreen( "WorldBuilderDiplomacyScreen", CvScreenEnums.WORLDBUILDER_DIPLOMACY_SCREEN )	
			screen.attachTextGFC( szPanel, "", szText, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		return

	def addRightSideOBText(self):
		if ( self.m_bShown ):
			screen = CyGInterfaceScreen( "WorldBuilderDiplomacyScreen", CvScreenEnums.WORLDBUILDER_DIPLOMACY_SCREEN )	
			iCount = -1
			iPanelCount = -1
			for i in range( gc.getMAX_CIV_PLAYERS() ):
				if ( gc.getPlayer(i).isEverAlive() ):
					iCount = iCount + 1
					if (iCount != self.m_iActivePlayer):
						iPanelCount = iPanelCount+1
						panelName = self.getRightSidePanelName(iPanelCount)
						if (gc.getPlayer(i).getTeam() == gc.getPlayer(self.m_iActivePlayer).getTeam()):
							self.addTextToRightSidePanel(panelName, "Same Team")
						else:
							szDropDownName = "WorldBuilderDiplomacyROPChoice" + str(iPanelCount)
							screen.attachDropDownBoxGFC(panelName, szDropDownName, False)
							if (gc.getTeam(gc.getPlayer(self.m_iActivePlayer).getTeam()).isOpenBorders(gc.getPlayer(i).getTeam())):
								screen.addPullDownString(szDropDownName, "Yes", 0, 0, True )
								screen.addPullDownString(szDropDownName, "No", 0, 0, False )
							else:
								screen.addPullDownString(szDropDownName, "Yes", 0, 0, False )
								screen.addPullDownString(szDropDownName, "No", 0, 0, True )
							
		return

	def addRightSideMutualProtectionText(self):
		if ( self.m_bShown ):
			screen = CyGInterfaceScreen( "WorldBuilderDiplomacyScreen", CvScreenEnums.WORLDBUILDER_DIPLOMACY_SCREEN )	
			iCount = -1
			iPanelCount = -1
			for i in range( gc.getMAX_CIV_PLAYERS() ):
				if ( gc.getPlayer(i).isEverAlive() ):
					iCount = iCount + 1
					if (iCount != self.m_iActivePlayer):
						iPanelCount = iPanelCount+1
						panelName = self.getRightSidePanelName(iPanelCount)
						if (gc.getPlayer(i).getTeam() == gc.getPlayer(self.m_iActivePlayer).getTeam()):
							self.addTextToRightSidePanel(panelName, "Same Team")
						else:
							szDropDownName = "WorldBuilderDiplomacyMPChoice" + str(iPanelCount)
							screen.attachDropDownBoxGFC(panelName, szDropDownName, False)
							if (gc.getTeam(gc.getPlayer(self.m_iActivePlayer).getTeam()).isOpenBorders(gc.getPlayer(i).getTeam())):
								screen.addPullDownString(szDropDownName, "Yes", 0, 0, True )
								screen.addPullDownString(szDropDownName, "No", 0, 0, False )
							else:
								screen.addPullDownString(szDropDownName, "Yes", 0, 0, False )
								screen.addPullDownString(szDropDownName, "No", 0, 0, True )
							
		return

	def addRightSideWarPeaceText(self):
		if ( self.m_bShown ):
			screen = CyGInterfaceScreen( "WorldBuilderDiplomacyScreen", CvScreenEnums.WORLDBUILDER_DIPLOMACY_SCREEN )	
			iCount = -1
			iPanelCount = -1
			for i in range( gc.getMAX_CIV_PLAYERS() ):
				if ( gc.getPlayer(i).isEverAlive() ):
					iCount = iCount + 1
					if (iCount != self.m_iActivePlayer):
						iPanelCount = iPanelCount+1
						panelName = self.getRightSidePanelName(iPanelCount)
						if (gc.getPlayer(i).getTeam() == gc.getPlayer(self.m_iActivePlayer).getTeam()):
							self.addTextToRightSidePanel(panelName, "Same Team")
						else:
							szDropDownName = "WorldBuilderDiplomacyWPChoice" + str(iPanelCount)
							screen.attachDropDownBoxGFC(panelName, szDropDownName, False)
							if (atWar(gc.getPlayer(self.m_iActivePlayer).getTeam(), gc.getPlayer(i).getTeam())):
								screen.addPullDownString(szDropDownName, "At War", 0, 0, True )
								screen.addPullDownString(szDropDownName, "At Peace", 0, 0, False )
							else:
								screen.addPullDownString(szDropDownName, "At War", 0, 0, False )
								screen.addPullDownString(szDropDownName, "At Peace", 0, 0, True )
							
		return

	def addRightSideTradingText(self):
		if ( self.m_bShown ):
			screen = CyGInterfaceScreen( "WorldBuilderDiplomacyScreen", CvScreenEnums.WORLDBUILDER_DIPLOMACY_SCREEN )	
			iCount = -1
			iPanelCount = -1
			for i in range( gc.getMAX_CIV_PLAYERS() ):
				if ( gc.getPlayer(i).isEverAlive() ):
					iCount = iCount + 1
					if (iCount != self.m_iActivePlayer):
						iPanelCount = iPanelCount+1
						panelName = self.getRightSidePanelName(iPanelCount)
						self.addTextToRightSidePanel(panelName, "Nothing")
							
		return

	def addRightSideAIAttitude(self):
		if ( self.m_bShown ):
			screen = CyGInterfaceScreen( "WorldBuilderDiplomacyScreen", CvScreenEnums.WORLDBUILDER_DIPLOMACY_SCREEN )	
			iCount = -1
			iPanelCount = -1
			for i in range( gc.getMAX_CIV_PLAYERS() ):
				if ( gc.getPlayer(i).isEverAlive() ):
					iCount = iCount + 1
					if (iCount != self.m_iActivePlayer):
						iPanelCount = iPanelCount+1
						panelName = self.getRightSidePanelName(iPanelCount)
						if (gc.getPlayer(i).isHuman()):
							self.addTextToRightSidePanel(panelName, "Human")
						else:
							szSliderName = "WorldBuilderDiplomacyAIAttitudeChange" + str(iPanelCount)
							screen.attachSlider(panelName, szSliderName, 0, 0, 100, 50, gc.getPlayer(i).AI_getAttitudeExtra(self.m_iActivePlayer), -100, 100, WidgetTypes.WIDGET_GENERAL, -1, -1, False)
							szSliderText = self.getSliderText(-100, 100, gc.getPlayer(i).AI_getAttitudeExtra(self.m_iActivePlayer))
							szTextName = "WorldBuilderDiplomacySliderText" + str(iPanelCount)
							screen.attachTextGFC( panelName, szTextName, szSliderText, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		return

	def getRightSidePanelName(self, iPanel):
		return "WorldBuilderDiplomacyRightPanel" + str(iPanel)

	def handlePlayerPullDownCB ( self, iIndex ) :
		iCount = -1
		for i in range( gc.getMAX_CIV_PLAYERS() ):
			if ( gc.getPlayer(i).isEverAlive() ):
				iCount = iCount + 1
				if (iCount == iIndex):
					self.m_iActivePlayer = i
					self.refreshRightSide()
					return
		return

	def removeLeftSideWidget(self):
		if ( self.m_bShown ):
			screen = CyGInterfaceScreen( "WorldBuilderDiplomacyScreen", CvScreenEnums.WORLDBUILDER_DIPLOMACY_SCREEN )	
			szWidget = "WorldBuilderDiplomacyLeftPanelAW"
			screen.deleteWidget(szWidget)
			szWidget = "WorldBuilderDiplomacyLeftPanelAIAttitude"
			screen.deleteWidget(szWidget)
			szWidget = "WorldBuilderDiplomacyPlayerChoice"
			screen.deleteWidget(szWidget)
			szWidget = "WorldBuilderDiplomacyLeftPanelRoP"
			screen.deleteWidget(szWidget)
			szWidget = "WorldBuilderDiplomacyLeftPanelMP"
			screen.deleteWidget(szWidget)
			szWidget = "WorldBuilderDiplomacyLeftPanelTG"
			screen.deleteWidget(szWidget)
		return

	def removeRightSideWidget(self, iPanel):
		if ( self.m_bShown ):
			screen = CyGInterfaceScreen( "WorldBuilderDiplomacyScreen", CvScreenEnums.WORLDBUILDER_DIPLOMACY_SCREEN )	
			szWidget = "WorldBuilderDiplomacyROPChoice" + str(iPanel)
			screen.deleteWidget(szWidget)
			szWidget = "WorldBuilderDiplomacyMPChoice" + str(iPanel)
			screen.deleteWidget(szWidget)
			szWidget = "WorldBuilderDiplomacyWPChoice" + str(iPanel)
			screen.deleteWidget(szWidget)
			szWidget = "WorldBuilderDiplomacyAIAttitudeChange" + str(iPanel)
			screen.deleteWidget(szWidget)
			szWidget = "WorldBuilderDiplomacySliderText" + str(iPanel)
			screen.deleteWidget(szWidget)
		return

	def getNumPlayers(self):
		iCount = 0
		for i in range( gc.getMAX_CIV_PLAYERS() ):
			if ( gc.getPlayer(i).isEverAlive() ):
				iCount = iCount + 1
		return iCount

	def getActualPlayer(self, iPlayer):
		iActualPlayer = -1
		iCount = -1
		iPanelCount = -1
		for i in range( gc.getMAX_CIV_PLAYERS() ):
			if ( gc.getPlayer(i).isEverAlive() ):
				iCount = iCount + 1
				if (iCount != self.m_iActivePlayer):
					iPanelCount = iPanelCount+1
					if (iPanelCount == iPlayer):
						iActualPlayer = i
		return iActualPlayer
		
	def setAtWar(self, iPlayer, iIndex):
		iActualPlayer = self.getActualPlayer(iPlayer)

		if (iActualPlayer != -1):
			if (iIndex == 0):
				gc.getTeam(gc.getPlayer(self.m_iActivePlayer).getTeam()).declareWar(gc.getPlayer(iActualPlayer).getTeam(), False)
			else:
				gc.getTeam(gc.getPlayer(self.m_iActivePlayer).getTeam()).makePeace(gc.getPlayer(iActualPlayer).getTeam())

			self.refreshRightSide()
		return
		
	def handleAIAttitudeChange(self, szTextName, iPanelIndex, iIndex):
		szOutput = szTextName
		szOutput = szOutput + " Player: "
		szOutput = szOutput + str(iPanelIndex)
		szOutput = szOutput + " Index: "
		szOutput = szOutput + str(iIndex)
		print(szOutput)
		
		panelName = self.getRightSidePanelName(iPanelIndex)
		szTextName = "WorldBuilderDiplomacySliderText" + str(iPanelIndex)
		iPlayer = self.getActualPlayer(iPanelIndex)
		gc.getPlayer(iPlayer).AI_setAttitudeExtra(self.m_iActivePlayer, iIndex)
		szSliderText = self.getSliderText(-100, 100, gc.getPlayer(iPlayer).AI_getAttitudeExtra(self.m_iActivePlayer))
		screen = CyGInterfaceScreen( "WorldBuilderDiplomacyScreen", CvScreenEnums.WORLDBUILDER_DIPLOMACY_SCREEN )	
		screen.deleteWidget(szTextName)
		screen.attachTextGFC( panelName, szTextName, szSliderText, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		return

	def getSliderText(self, iMin, iMax, iCurrent):
		szOutput = "iMin: "
		szOutput = szOutput + str(iMin)
		szOutput = szOutput + " iMax: "
		szOutput = szOutput + str(iMax)
		szOutput = szOutput + " iCurrent: "
		szOutput = szOutput + str(iCurrent)
		print(szOutput)

		szSliderText = str(iMin)
		szSliderText = szSliderText + "  ***  "
		szSliderText = szSliderText + str(iCurrent)
		szSliderText = szSliderText + "  ***  "
		szSliderText = szSliderText + str(iMax)
		return szSliderText

