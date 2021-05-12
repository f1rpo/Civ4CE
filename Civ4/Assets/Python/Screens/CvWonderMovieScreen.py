## Sid Meier's Civilization 4
## Copyright Firaxis Games 2005
from CvPythonExtensions import *
import PyHelpers
import CvUtil
import ScreenInput
import CvScreenEnums

PyPlayer = PyHelpers.PyPlayer
PyInfo = PyHelpers.PyInfo

# globals
gc = CyGlobalContext()
ArtFileMgr = CyArtFileMgr()
localText = CyTranslator()

MOVIE_SCREEN_WONDER = 0
MOVIE_SCREEN_RELIGION = 1
MOVIE_SCREEN_PROJECT = 2

class CvWonderMovieScreen:
	"Wonder Movie Screen"

	def __init__(self):
		self.fDelay = -1.0
		self.fTime = 0.0
		self.bDone = false

	def interfaceScreen (self, iMovieItem, iCityId, iMovieType):
		# iMovieItem is either the WonderID, the ReligionID, or the ProjectID, depending on iMovieType
		
		self.Z_CONTROLS = -2.2

		self.X_SCREEN = 250
		self.Y_SCREEN = 40
		self.W_SCREEN = 760
		self.H_SCREEN = 590
		self.Y_TITLE = self.Y_SCREEN + 20
		self.iWonderId = iMovieItem
		
		self.X_EXIT = self.X_SCREEN + self.W_SCREEN/2 - 50
		self.Y_EXIT = self.Y_SCREEN + self.H_SCREEN - 50
		self.W_EXIT = 120
		self.H_EXIT = 30
		
		self.X_MOVIE = 20
		self.Y_MOVIE = 50
		self.W_MOVIE = 720
		self.H_MOVIE = 480
		
		self.iMovieType = iMovieType
		self.fTime = 0.0
		self.fDelay = 1.5
		self.bDone = false
		
		# not all projects have movies
		if self.iMovieType == MOVIE_SCREEN_PROJECT:
			szArtDef = gc.getProjectInfo(iMovieItem).getMovieArtDef()
			if len(szArtDef) == 0:
				return
		
		player = PyPlayer(CyGame().getActivePlayer())
		
		# move the camera and mark the interface camera as dirty so that it gets reset - JW
		if self.iMovieType == MOVIE_SCREEN_WONDER:
			CyInterface().lookAtCityBuilding(iCityId, iMovieItem)
		else:
			CyInterface().lookAtCityBuilding(iCityId, -1)
		CyInterface().setDirty(InterfaceDirtyBits.SelectionCamera_DIRTY_BIT, True)
		
		screen = CyGInterfaceScreen( "WonderMovieScreen" + str(iMovieItem), CvScreenEnums.WONDER_MOVIE_SCREEN )
		screen.addPanel("WonderMoviePanel", "", "", true, true,
			self.X_SCREEN, self.Y_SCREEN, self.W_SCREEN, self.H_SCREEN, PanelStyles.PANEL_STYLE_MAIN)
		
		screen.showWindowBackground( True )
		screen.setRenderInterfaceOnly(False)
		screen.showScreen(PopupStates.POPUPSTATE_IMMEDIATE, False)
		screen.enableWorldSounds( false )
                		
		# Header...
		szHeaderId = "WonderTitleHeader" + str(iMovieItem)
		if self.iMovieType == MOVIE_SCREEN_RELIGION:
			szHeader = localText.getText("TXT_KEY_MISC_REL_FOUNDED_MOVIE", (gc.getReligionInfo(iMovieItem).getTextKey(), ))
		elif self.iMovieType == MOVIE_SCREEN_WONDER:
			szHeader = gc.getBuildingInfo(iMovieItem).getDescription()
		elif self.iMovieType == MOVIE_SCREEN_PROJECT:
			szHeader = gc.getProjectInfo(iMovieItem).getDescription()

		screen.setText(szHeaderId, "Background", szHeader, CvUtil.FONT_CENTER_JUSTIFY,
				self.X_SCREEN + self.W_SCREEN / 2, self.Y_TITLE, self.Z_CONTROLS, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
				
		screen.hide("Background")

		screen.playMovie("", 0, 0, 0, 0, 0) # dummy call to hide screen if no movies are supposed to be shown

	def playMovie(self):
			
		screen = CyGInterfaceScreen( "WonderMovieScreen" + str(self.iWonderId), CvScreenEnums.WONDER_MOVIE_SCREEN )
		screen.setRenderInterfaceOnly(True)
		screen.show("Background")

		# Play the movie
		if self.iMovieType == MOVIE_SCREEN_RELIGION:
			screen.addReligionMovieWidgetGFC( "ReligionMovie", gc.getReligionInfo(self.iWonderId).getMovieFile(), self.X_SCREEN + self.X_MOVIE, self.Y_SCREEN + self.Y_MOVIE, self.W_MOVIE, self.H_MOVIE, WidgetTypes.WIDGET_GENERAL, -1, -1)
			CyInterface().playGeneralSound(gc.getReligionInfo(self.iWonderId).getMovieSound())		
		elif self.iMovieType == MOVIE_SCREEN_WONDER:		
			screen.playMovie(gc.getBuildingInfo(self.iWonderId).getMovie(), self.X_SCREEN + self.X_MOVIE, self.Y_SCREEN + self.Y_MOVIE, self.W_MOVIE, self.H_MOVIE, -2.3 )
		elif self.iMovieType == MOVIE_SCREEN_PROJECT:
			szArtDef = gc.getProjectInfo(self.iWonderId).getMovieArtDef()
			screen.playMovie(CyArtFileMgr().getMovieArtInfo(szArtDef).getPath(), self.X_SCREEN + self.X_MOVIE, self.Y_SCREEN + self.Y_MOVIE, self.W_MOVIE, self.H_MOVIE, -2.3 )
			
		screen.setButtonGFC("WonderExit" + str(self.iWonderId), localText.getText("TXT_KEY_MAIN_MENU_OK", ()), "", self.X_EXIT, self.Y_EXIT, self.W_EXIT, self.H_EXIT, WidgetTypes.WIDGET_CLOSE_SCREEN, -1, -1, ButtonStyles.BUTTON_STYLE_STANDARD )

	# Will handle the input for this screen...
	def handleInput (self, inputClass):
		if (inputClass.getNotifyCode() == NotifyCode.NOTIFY_MOVIE_DONE):
			if (not self.bDone):
				screen = CyGInterfaceScreen( "WonderMovieScreen" + str(self.iWonderId), CvScreenEnums.WONDER_MOVIE_SCREEN )
				if self.iMovieType == MOVIE_SCREEN_WONDER:
					szHelp = CyGameTextMgr().getBuildingHelp(self.iWonderId, False, False, False, None)
				elif self.iMovieType == MOVIE_SCREEN_PROJECT:
					szHelp = CyGameTextMgr().getProjectHelp(self.iWonderId, False, None)
				else:
					szHelp = ""
				
				if len(szHelp) > 0:
					screen.addMultilineText("MonkeyText", szHelp, self.X_SCREEN + self.X_MOVIE + self.W_MOVIE / 8, self.Y_SCREEN + self.Y_MOVIE + 100, 3 * self.W_MOVIE / 4, self.H_MOVIE - 100, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)	
				self.bDone = true

		return 0

	def update(self, fDelta):
	
		if self.fDelay > 0:
			self.fTime += fDelta
			if self.fTime > self.fDelay:
				self.playMovie()
				self.fDelay = -1
		return

