## Sid Meier's Civilization 4
## Copyright Firaxis Games 2005
import CvUtil
import CvScreenEnums
from CvPythonExtensions import *

# globals
gc = CyGlobalContext()
ArtFileMgr = CyArtFileMgr()

# globals
g_bShowQuestBody = 0
g_bChooseOpen = 0
g_iStartTime = 0

class CvTutorialQuestScreen:
	
	def interfaceScreen(self):
		global g_bShowQuestBody
		global g_iStartTime
		
		screen = CyInterfaceScreen("CvTutorialQuestScreen", CvScreenEnums.ADVISOR_SCREEN)
		screen.setBackgroundBorderedPane(ArtFileMgr.getInterfaceArtInfo("POPUPS_BACKGROUND_TRANSPARENT").getPath(), 520, 35, 0.1, 420, 50, 4, False)
		screen.showScreen(PopupStates.POPUPSTATE_IMMEDIATE, True)
		screen.addBorderedPane( "QuestBody", "Background", ArtFileMgr.getInterfaceArtInfo("POPUPS_BACKGROUND_TRANSPARENT").getPath(), 0, 50, 0.1, 500, 140, 4, False, WidgetTypes.WIDGET_GENERAL, -1, -1)
		screen.hide( "QuestBody" )
		screen.setButton( "QuestToggle", "Background", 396, 26, -0.1, ArtFileMgr.getInterfaceArtInfo("INTERFACE_GENERAL_UPARROW").getPath(), WidgetTypes.WIDGET_GENERAL, -1, -1, False, GenericButtonSizes.BUTTON_SIZE_16, "" )
		g_bShowQuestBody = 0
		g_iStartTime = screen.getCurrentTime()

	def updateHeader( self, objectiveText ):
		screen = CyInterfaceScreen("CvTutorialQuestScreen", CvScreenEnums.ADVISOR_SCREEN)
		screen.setText( "QuestHeaderText", "Background", objectiveText, CvUtil.FONT_CENTER_JUSTIFY, 210, 8, -0.1, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		return
		
	def updateBodyText( self, bodyText ):
		global g_bShowQuestBody
		global g_iStartTime
		global g_bChooseOpen
		
		screen = CyInterfaceScreen("CvTutorialQuestScreen", CvScreenEnums.ADVISOR_SCREEN)
		fNewY = screen.setFittedText( "QuestBodyText", "QuestBody", bodyText, 480, True, True, CvUtil.FONT_LEFT_JUSTIFY, 10, 8, -0.1, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.resizeBorderedPane( "QuestBody", 0, 500, 50, 70 + fNewY )
		screen.show( "QuestBody" )
		screen.setButton( "QuestToggle", "Background", 396, 26, -0.1, ArtFileMgr.getInterfaceArtInfo("INTERFACE_GENERAL_UPARROW").getPath(), WidgetTypes.WIDGET_GENERAL, -1, -1, False, GenericButtonSizes.BUTTON_SIZE_16, "" )
		g_bShowQuestBody = 1
		g_iStartTime = screen.getCurrentTime()
		g_bChooseOpen = 0
		return
		
	def toggleQuest( self ):
		global g_bShowQuestBody
		global g_bChooseOpen
	
		g_bChooseOpen = 1
	
		screen = CyInterfaceScreen("CvTutorialQuestScreen", CvScreenEnums.ADVISOR_SCREEN)
		
		if ( g_bShowQuestBody == 1 ):
			g_bShowQuestBody = 0
			screen.setButton( "QuestToggle", "Background", 396, 26, -0.1, ArtFileMgr.getInterfaceArtInfo("INTERFACE_GENERAL_DOWNARROW").getPath(), WidgetTypes.WIDGET_GENERAL, -1, -1, False, GenericButtonSizes.BUTTON_SIZE_16, "" )
			screen.hide( "QuestBody" )
		else:
			g_bShowQuestBody = 1
			screen.setButton( "QuestToggle", "Background", 396, 26, -0.1, ArtFileMgr.getInterfaceArtInfo("INTERFACE_GENERAL_UPARROW").getPath(), WidgetTypes.WIDGET_GENERAL, -1, -1, False, GenericButtonSizes.BUTTON_SIZE_16, "" )
			screen.show( "QuestBody" )
		
	def update(self, fDelta):
		global g_iStartTime
		global g_bChooseOpen
		global g_bShowQuestBody

		screen = CyInterfaceScreen("CvTutorialQuestScreen", CvScreenEnums.ADVISOR_SCREEN)
		
		if ( screen.getCurrentTime() - g_iStartTime > 20000 and g_bChooseOpen == 0):
			g_bShowQuestBody = 0
			screen.setButton( "QuestToggle", "Background", 396, 26, -0.1, ArtFileMgr.getInterfaceArtInfo("INTERFACE_GENERAL_DOWNARROW").getPath(), WidgetTypes.WIDGET_GENERAL, -1, -1, False, GenericButtonSizes.BUTTON_SIZE_16, "" )
			screen.hide( "QuestBody" )

		return
		
	# Will handle the input for this screen...
	def handleInput (self, inputClass):
		if ( inputClass.getFunctionName() == "QuestToggle" and inputClass.getNotifyCode() == NotifyCode.NOTIFY_CLICKED ):
			self.toggleQuest()
		return 0
