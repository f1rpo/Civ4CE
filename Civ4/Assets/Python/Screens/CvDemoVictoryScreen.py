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

class CvDemoVictoryScreen:
	def __init__(self):
		self.DEMO_VICTORY_SCREEN_NAME = "DemoVictoryScreen"
		
		self.X_SCREEN = 0
		self.Y_SCREEN = 0
		self.W_SCREEN = 1024
		self.H_SCREEN = 768
		
	def getScreen(self):
		return CyGInterfaceScreen(self.DEMO_VICTORY_SCREEN_NAME, CvScreenEnums.DEMO_VICTORY_SCREEN)

	def interfaceScreen(self):
		screen = self.getScreen()
		screen.setRenderInterfaceOnly(True);
		screen.showScreen(PopupStates.POPUPSTATE_IMMEDIATE, False)

		screen.setImageButton( "DemoVictoryScreenBackground", "Art/Interface/Screens/Purchase/BuyIt.dds", -2, -2, 1028, 776, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		#screen.addDDSGFC("DemoVictoryScreenBackground", "Art/Interface/Screens/Purchase/BuyIt.dds", 0, 0, self.W_SCREEN, self.H_SCREEN, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.setDimensions(screen.centerX(0), screen.centerY(0), self.W_SCREEN, self.H_SCREEN)			

	# Will handle the input for this screen...
	def handleInput (self, inputClass):
		if ( inputClass.getNotifyCode() == NotifyCode.NOTIFY_CLICKED or inputClass.getNotifyCode() == NotifyCode.NOTIFY_CHARACTER ):
			# this should close the app and open the browser
			self.getScreen().hideScreen()
			CyEngine().QuitGame()
			#import Popup
			#popup=Popup.PyPopup(-1)
			#popup.setBodyString("The app should exit")
			#popup.launch()
			return
	
	def update(self, fDelta):
		return 0
