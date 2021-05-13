## Sid Meier's Civilization 4
## Copyright Firaxis Games 2008

from CvPythonExtensions import *
import CvUtil
import ScreenInput
import CvScreenEnums

# Modified by Solver for Col v1

# globals
gc = CyGlobalContext()
ArtFileMgr = CyArtFileMgr()
localText = CyTranslator()

# this class is shared by both the resource and technology foreign advisors
class CvCongressAdvisor:
	"Congress Advisor Screen"

	def __init__(self):
		self.SCREEN_NAME = "CongressAdvisor"
		self.BACKGROUND_ID = "CongressAdvisorBackground"

		self.XResolution = 0
		self.YResolution = 0

	def interfaceScreen (self):

		self.player = gc.getPlayer(gc.getGame().getActivePlayer())
		self.team = gc.getTeam(self.player.getTeam())

		screen = self.getScreen()
		if screen.isActive():
			return

		self.MyFathers = []

		self.XResolution = self.getScreen().getXResolution()
		self.YResolution = self.getScreen().getYResolution()

		screen.setRenderInterfaceOnly(True);
		screen.showScreen(PopupStates.POPUPSTATE_IMMEDIATE, False)

		# Set the background and exit button, and show the screen
		screen.setDimensions(0, 0, self.XResolution, self.YResolution)
		screen.addDDSGFC(self.BACKGROUND_ID, ArtFileMgr.getInterfaceArtInfo("INTERFACE_CONGRESS_BG").getPath(), 0, 0, self.XResolution, self.YResolution, WidgetTypes.WIDGET_GENERAL, -1, -1 )

		screen.addDDSGFC("TopPanel", ArtFileMgr.getInterfaceArtInfo("INTERFACE_SCREEN_TITLE").getPath(), 0, 0, self.XResolution, 55, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.addDDSGFC("BottomPanel", ArtFileMgr.getInterfaceArtInfo("INTERFACE_SCREEN_TAB_OFF").getPath(), 0, self.YResolution - 55, self.XResolution, 55, WidgetTypes.WIDGET_GENERAL, -1, -1 )

		screen.showWindowBackground(False)

		TitleText = localText.getText("TXT_KEY_CONGRESS_ADVISOR_TITLE", ()).upper()
		TitleText = localText.changeTextColor(TitleText, gc.getInfoTypeForString("COLOR_FONT_CREAM"))

		screen.setLabel( "Title", "Background", u"<font=4b>" + TitleText + u"</font>", CvUtil.FONT_CENTER_JUSTIFY, self.XResolution / 2, 4, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
		screen.setText( "CongressScreenExit", "Background", u"<font=4>" + CyTranslator().getText("TXT_KEY_PEDIA_SCREEN_EXIT", ()).upper() + "</font>", CvUtil.FONT_RIGHT_JUSTIFY, self.XResolution - 30, self.YResolution - 35, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_CLOSE_SCREEN, -1, -1 )
		screen.setActivation( "CongressScreenExit", ActivationTypes.ACTIVATE_MIMICPARENTFOCUS )

		ArrowButtonSize = 55
		screen.setButtonGFC("ScrollMinus", u"", "", self.XResolution * 3 / 4 - ArrowButtonSize / 2, self.YResolution - 50, ArrowButtonSize, ArrowButtonSize, WidgetTypes.WIDGET_GENERAL, 1, -1, ButtonStyles.BUTTON_STYLE_ARROW_LEFT )
		screen.setButtonGFC("ScrollPlus", u"", "", self.XResolution * 3 / 4, self.YResolution - 50, ArrowButtonSize, ArrowButtonSize, WidgetTypes.WIDGET_GENERAL, 2, -1, ButtonStyles.BUTTON_STYLE_ARROW_RIGHT )

		self.FatherColumn = 0

		self.drawContents()
		return 0

	def getScreen(self):
		return CyGInterfaceScreen(self.SCREEN_NAME, CvScreenEnums.CONGRESS_ADVISOR)
		
	def getActualColumnCount(self):
		return ((len(self.MyFathers) + 1) / 2)

	def drawContents(self):
		screen = self.getScreen()

		iRow = 0
		self.MaxColumnCount = 5
		iRecordHight = ((self.YResolution - 110) / 2) - 5
		iRecordWidth = ((self.XResolution - 15) / self.MaxColumnCount) * 19 / 20
		PortraitSize = int(iRecordWidth * 1.25)
		PortraitBorder = PortraitSize / 20

		self.MyFathers = []
		for iFather in range(gc.getNumFatherInfos()):
			Father = gc.getFatherInfo(iFather)
			if (gc.getGame().getFatherTeam(iFather) == self.team.getID()):
				self.MyFathers.append(iFather)

		if (self.FatherColumn == 0) or (self.getActualColumnCount() <= self.MaxColumnCount):
			screen.hide("ScrollMinus")
		else:
			screen.show("ScrollMinus")

		if (self.FatherColumn == self.getActualColumnCount() - self.MaxColumnCount) or (self.getActualColumnCount() <= self.MaxColumnCount):
			screen.hide("ScrollPlus")
		else:
			screen.show("ScrollPlus")

		iColumn = 0
		for iColumn in range(self.MaxColumnCount):
			screen.addDDSGFCAt("Portrait Box" + str((2 * iColumn) + 0), "Background", ArtFileMgr.getInterfaceArtInfo("INTERFACE_CONGRESS_BOX").getPath(), (iRecordWidth * iColumn) + (PortraitBorder * (iColumn + 1)), (iRecordHight * 0) + 55, iRecordWidth , iRecordHight, WidgetTypes.WIDGET_GENERAL, -1, -1, False)

		iColumn = 0
		for iColumn in range(self.MaxColumnCount):
			screen.addDDSGFCAt("Portrait Box" + str((2 * iColumn) + 1), "Background", ArtFileMgr.getInterfaceArtInfo("INTERFACE_CONGRESS_BOX").getPath(), (iRecordWidth * iColumn) + (PortraitBorder * (iColumn + 1)), (iRecordHight * 1) + 55, iRecordWidth , iRecordHight, WidgetTypes.WIDGET_GENERAL, -1, -1, False)


		FatherCountText = localText.getText("TXT_KEY_MEMBERS", (CyGameTextMgr().getInterfaceTimeStr(gc.getGame().getActivePlayer()), len(self.MyFathers)))
		FatherCountText = localText.changeTextColor(FatherCountText, gc.getInfoTypeForString("COLOR_FONT_GOLD"))
		screen.setLabel("FatherCount", "Background", "<font=4>" + FatherCountText + "</font>", CvUtil.FONT_CENTER_JUSTIFY, self.XResolution / 2, self.YResolution - 35, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)

		iColumn = 0
		for iIndex in range(self.MaxColumnCount * 2):
			iFatherIndex = iIndex + (self.FatherColumn * 2)
			if (iFatherIndex < len(self.MyFathers)):
				iFather = self.MyFathers[iIndex + (self.FatherColumn * 2)]
				iFatherTurn = CyGame().getFatherGameTurn(iFather)
				iFatherYear = CyGame().getTurnYear(iFatherTurn)

				screen.addDDSGFCAt("Portrait" + str(iIndex), "Background", gc.getFatherInfo(iFather).getPortrait(), (iRecordWidth * iColumn) + (PortraitBorder * (iColumn + 1)) + ((iRecordWidth / 2) - (PortraitSize / 2)), (iRecordHight * iRow) + 30, PortraitSize, PortraitSize, WidgetTypes.WIDGET_GENERAL, -1, -1, False)
				szFatherTitle = gc.getFatherInfo(iFather).getDescription() + u" (%d)" %iFatherYear
				
				fontSize = "<font=4>"
				
				if (len(szFatherTitle) > 30):
					fontSize = "<font=3>"
					
				szFatherTitle = localText.changeTextColor(szFatherTitle, gc.getInfoTypeForString("COLOR_FONT_CREAM"))
				
				screen.addMultilineText("FatherName" + str(iIndex), fontSize + szFatherTitle + "</font>", (iRecordWidth * iColumn) + (PortraitBorder * (iColumn + 1)), (iRecordHight * iRow) + 20 + (iRecordHight * 3 / 5), iRecordWidth, (iRecordHight * 18 / 100), WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_CENTER_JUSTIFY)

				szFatherData = u"<font=2>" + CyGameTextMgr().getFatherHelp(iFather, True)[1:] + u"</font>"
				screen.attachMultilineTextAt("Portrait Box" + str(iIndex), "FatherInfo" + str(iIndex), szFatherData, 0, (iRecordHight * 2 / 3) + 5, iRecordWidth, (iRecordHight / 3) - 14, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_CENTER_JUSTIFY)

				if ((iIndex + 1) % 2 == 0):
					iRow = 0
					iColumn += 1
				else:
					iRow += 1
			else:
				screen.hide("Portrait" + str(iIndex))
				screen.hide("FatherName" + str(iIndex))
				screen.hide("FatherInfo" + str(iIndex))

	def handleInput(self, inputClass):

		if (inputClass.getNotifyCode() == NotifyCode.NOTIFY_CLICKED):

			if (inputClass.getButtonType() == WidgetTypes.WIDGET_GENERAL):

				if(inputClass.getData1() == 1):
					self.FatherColumn = max(0, self.FatherColumn - 1)
					self.drawContents()

				elif(inputClass.getData1() == 2):
					self.FatherColumn = min(self.getActualColumnCount() - self.MaxColumnCount, self.FatherColumn + 1)
					self.drawContents()

		return 0

	def update(self, fDelta):
		return 0
		