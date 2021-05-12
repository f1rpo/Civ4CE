## Sid Meier's Civilization 4
## Copyright Firaxis Games 2005
#
# Sample PitBoss window/app framework
# Mustafa Thamer 2-15-05
#
from CvPythonExtensions import *
import wx
import wx.wizard
import wx.lib.scrolledpanel
import time
import string

bPublic = True
bSaved = False
bPatchConfirmed = False
bPatchOK = False
szPatchName = None
PB = CyPitboss()
gc = CyGlobalContext()
localText = CyTranslator()
curPage = None

#
# Net Select Page (first page of wizard)
#
class NetSelectPage(wx.wizard.PyWizardPage):
	def __init__(self, parent):
		wx.wizard.PyWizardPage.__init__(self, parent)
		self.next = self.prev = None
		self.myParent = parent
		
		# Place the radio buttons
		selections = ['LAN', 'Internet']
		sizer = wx.BoxSizer(wx.VERTICAL)
		self.rb = wx.RadioBox(
					self, -1, (localText.getText("TXT_KEY_PITBOSS_SELECT_NETWORK", ())), wx.DefaultPosition, wx.DefaultSize,
					selections, 1, wx.RA_SPECIFY_COLS
					)
				
		self.rb.SetToolTip(wx.ToolTip((localText.getText("TXT_KEY_PITBOSS_SELECT_NETWORK_HELP", ()))))
		sizer.Add(self.rb, 0, wx.ALL, 5)
		
		self.SetSizer(sizer)
		
		self.Bind(wx.wizard.EVT_WIZARD_PAGE_CHANGED, self.OnPageChanged)
		
	def enableButtons(self):
		self.myParent.FindWindowById(wx.ID_FORWARD).Enable(True)
		self.myParent.FindWindowById(wx.ID_BACKWARD).Enable(True)
		
	def OnPageChanged(self, event):
		global curPage
		global bPatchConfirmed
		global bPatchOK
		
		bPatchConfirmed = False
		bPatchOK = False

		# Determine what buttons should be enabled
		self.enableButtons()
		
		# We are the current page
		curPage = self
		
	def SetNext(self, next):
		self.next = next
		
	def SetPrev(self, prev):
		self.prev = prev
		
	def GetNext(self):
		"Select which next page to show based on network selected"
		global bPublic 
		
		next = self.next
		
		if (self.rb.GetSelection() == 0):
			bPublic = False
			next = next.GetNext()
		else:
			bPublic = True
			
		return next
		
	def GetPrev(self):
		return self.prev
		
#
# Login page (optional 2nd page)
#
class LoginPage(wx.wizard.WizardPageSimple):
	def __init__(self, parent):
		wx.wizard.WizardPageSimple.__init__(self, parent)
		
		self.myParent = parent
		header = wx.StaticText(self, -1, (localText.getText("TXT_KEY_PITBOSS_LOGIN", ())))
		
		usernameLbl = wx.StaticText(self, -1, (localText.getText("TXT_KEY_PITBOSS_USERNAME", ())))
		self.username = wx.TextCtrl(self, -1, "", size=(125,-1))
		self.username.SetHelpText((localText.getText("TXT_KEY_PITBOSS_USERNAME_HELP", ())))
		self.username.SetInsertionPoint(0)
		self.Bind(wx.EVT_TEXT, self.OnTextEntered, self.username)
		
		passwordLbl = wx.StaticText(self, -1, (localText.getText("TXT_KEY_PITBOSS_PASSWORD", ())))
		self.password = wx.TextCtrl(self, -1, "", size=(125,-1), style=wx.TE_PASSWORD)
		self.password.SetHelpText((localText.getText("TXT_KEY_PITBOSS_PASSWORD_HELP", ())))
		self.Bind(wx.EVT_TEXT, self.OnTextEntered, self.password)
		
		self.Bind(wx.wizard.EVT_WIZARD_PAGE_CHANGING, self.OnPageChanging)
		self.Bind(wx.wizard.EVT_WIZARD_PAGE_CHANGED, self.OnPageChanged)
		
		sizer = wx.FlexGridSizer(cols=2, hgap=4, vgap=4)
		sizer.AddMany([ usernameLbl, self.username,
						passwordLbl, self.password,
						])
		border = wx.BoxSizer(wx.VERTICAL)
		border.Add(sizer, 0, wx.ALL, 25)
		self.SetSizer(border)
		self.SetAutoLayout(True)
		
	def enableButtons(self):
		global bPatchConfirmed
		global bPatchOK
		
		if (not bPatchConfirmed):
			# Not confirmed, disable buttons
			self.myParent.FindWindowById(wx.ID_FORWARD).Disable()
			self.myParent.FindWindowById(wx.ID_BACKWARD).Disable()
		# Check to see if there is text in both boxes
		elif ( (self.username.GetValue() == "") or (self.password.GetValue() == "") or (not bPatchOK) ):
			# There isn't, disable the forward button
			self.myParent.FindWindowById(wx.ID_FORWARD).Disable()
			self.myParent.FindWindowById(wx.ID_BACKWARD).Enable(True)
		else:
			# Text entered, enable the forward button
			self.myParent.FindWindowById(wx.ID_FORWARD).Enable(True)
			self.myParent.FindWindowById(wx.ID_BACKWARD).Enable(True)
			
	def patchAvailable(self, patchName, patchUrl):
		global bPatchConfirmed
		global szPatchName
		
		# Put up a dialog
		dlg = wx.MessageDialog(
			self, "You must install the latest updates to continue",
			"Patch required!", wx.OK|wx.CANCEL|wx.ICON_EXCLAMATION)
			
		# Show the modal dialog and get the response
		if dlg.ShowModal() == wx.ID_OK:
			# They want to download the patch - tell the app
			if (not PB.downloadPatch(patchName, patchUrl)):
				# Patching failed - tell the user
				msg = wx.MessageBox("Error while downloadint the patch", "Download Error", wx.ICON_ERROR)
				bPatchConfirmed = true
				szPatchName = patchName
				self.enableButtons()
		else:
			bPatchConfirmed = true
			self.enableButtons()
			
	def patchComplete(self):
		global bPatchConfirmed
		global bPatchOK
		global szPatchName
		
		# Put up a dialog
		dlg = wx.MessageDialog(
			self, "Press ok to shut down and install the patch",
			"Download complete!", wx.OK|wx.ICON_EXCLAMATION)
			
		# Show the dialog and get the response
		if dlg.ShowModal() == wx.ID_OK:
			# They want to restart - tell the app
			PB.installPatch(szPatchName)
		else:
			# Not sure if this can actually happen, but handle it anyway
			bPatchConfirmed = true
			bPatchOK = false
		
	def OnTextEntered(self, event):
		# Determine what buttons should be enabled
		self.enableButtons()
			
	def OnPageChanging(self, event):
		# Check direction
		if event.GetDirection():
			# We are trying to move forward - check password
			if ( not PB.login(self.username.GetValue(), self.password.GetValue()) ):
				# Login failed - let the user know
				msg = wx.MessageBox((localText.getText("TXT_KEY_PITBOSS_LOGIN_FAILED", ())), 
								(localText.getText("TXT_KEY_PITBOSS_LOGIN_ERROR", ())),	wx.ICON_ERROR)
				# Veto the event to prevent moving forward
				event.Veto()
			
	def OnPageChanged(self, event):
		global bPatchConfirmed
		global curPage
		
		# Check for a patch here
		if (not bPatchConfirmed):
			if (not PB.checkPatch()):
				# Error in checking for a patch
				msg = wx.MessageBox("Error while looking for updates", "Update error", wx.ICON_ERROR)
				bPatchConfirmed = true
		
		# Determine what buttons should be enabled
		self.enableButtons()
		
		# We are the current page
		curPage = self
			
#
# Load Select Page
#
class LoadSelectPage(wx.wizard.WizardPageSimple):
	def __init__(self, parent):
		wx.wizard.WizardPageSimple.__init__(self, parent)
		
		self.myParent = parent
		
		# Place the radio buttons
		selections = ['New Game', 'Load Game']
		sizer = wx.BoxSizer(wx.VERTICAL)
		self.rb = wx.RadioBox(
					self, -1, (localText.getText("TXT_KEY_PITBOSS_SELECT_INIT", ())), wx.DefaultPosition, wx.DefaultSize,
					selections, 1, wx.RA_SPECIFY_COLS
					)
				
		self.rb.SetToolTip(wx.ToolTip((localText.getText("TXT_KEY_PITBOSS_SELECT_INIT_HELP", ()))))
		sizer.Add(self.rb, 0, wx.ALL, 5)
		
		self.SetSizer(sizer)
		
		self.Bind(wx.wizard.EVT_WIZARD_PAGE_CHANGED, self.OnPageChanged)
		self.Bind(wx.wizard.EVT_WIZARD_PAGE_CHANGING, self.OnPageChanging)
		
	def enableButtons(self):
		# If the patch state is ok, enable appropriate buttons
		global bPatchConfirmed
		
		if (not bPatchConfirmed):
			# Not confirmed, disable buttons
			self.myParent.FindWindowById(wx.ID_FORWARD).Disable()
			self.myParent.FindWindowById(wx.ID_BACKWARD).Disable()
		else:
			self.myParent.FindWindowById(wx.ID_FORWARD).Enable(True)
			self.myParent.FindWindowById(wx.ID_BACKWARD).Enable(True)
			
	def patchAvailable(self, patchName, patchUrl):
		global bPatchConfirmed
		
		# Put up a dialog
		dlg = wx.MessageDialog(
			self, "Would you like to install the latest updates?",
			"Patch available!", wx.YES_NO|wx.ICON_QUESTION)
			
		# Show the modal dialog and get the response
		if dlg.ShowModal() == wx.ID_YES:
			# They want to download the patch - tell the app
			if (not PB.downloadPatch(patchName, patchUrl)):
				# Patching failed - tell the user
				msg = wx.MessageBox("Error while downloadint the patch", "Download Error", wx.ICON_ERROR)
				bPatchConfirmed = true
				self.enableButtons()
		else:
			# They didn't want to download it, which is ok for LAN games
			bPatchConfirmed = true
			self.enableButtons()
			
	def patchComplete(self):
		global bPatchConfirmed
		global bPatchOK
		global szPatchName
		
		# Put up a dialog
		dlg = wx.MessageDialog(
			self, "Press ok to shut down and install the patch",
			"Download complete!", wx.OK|wx.CANCEL|wx.ICON_EXCLAMATION)
			
		# Show the dialog and get the response
		if dlg.ShowModal() == wx.ID_OK:
			# They want to restart - tell the app
			PB.installPatch(szPatchName)
		else:
			# This is ok for LAN games
			bPatchConfirmed = true
			bPatchOK = false
		
	def OnPageChanged(self, event):
		global curPage
		
		# If we haven't already, check for a patch
		global bPatchConfirmed
		if (not bPatchConfirmed):
			if (not PB.checkPatch()):
				# Error in checking for a patch
				msg = wx.MessageBox("Error while looking for updates", "Update error", wx.ICON_ERROR)
				bPatchConfirmed = true
				
		# Determine what buttons should be enabled
		self.enableButtons()
		
		curPage = self
			
	def OnPageChanging(self, event):
		
		global bSaved
		
		# Check direction
		if event.GetDirection():
			# We are trying to move forward - are we trying to start a new game?
			if (self.rb.GetSelection() == 0):
				# Hosting a new game - pop the gamename dialog
				dlg = wx.TextEntryDialog(
					self, 'Please enter a name for your game',
					'Name your game!')
					
				# Show the modal dialog and get the response
				if dlg.ShowModal() == wx.ID_OK:
					# Check the game name
					gamename = dlg.GetValue()
					if (gamename != ""):
						# We got a gamename, save it here
						PB.setGamename(gamename)
						
						# Now host!
						if ( not PB.host() ):
							# Hosting failed for some reason.  Clean up and exit
							msg = wx.MessageBox((localText.getText("TXT_KEY_PITBOSS_ERROR_HOSTING", ())), (localText.getText("TXT_KEY_PITBOSS_HOST_ERROR", ())), wx.ICON_ERROR)
							PB.reset()
							event.Veto()
						else:
							bSaved = False
							
					else:
						# Malicious user didn't enter a gamename...
						event.Veto()
						
				else:
					# User hit cancel
					event.Veto()
					
				dlg.Destroy()
					
			else:
				# Loading a game - popup the file browser
				dlg = wx.FileDialog(
					self, message=(localText.getText("TXT_KEY_PITBOSS_CHOOSE_SAVE", ())), defaultDir=".\saves\multi",
					defaultFile="", wildcard=localText.getText("TXT_KEY_PITBOSS_SAVE_FILES", ("(*.sav)|*.sav", )), style=wx.OPEN
					)
				
				# Show the modal dialog and get the response
				if dlg.ShowModal() == wx.ID_OK:
					# Get the file name
					path = dlg.GetPath()
					if (path != ""):
						# We got a save file - try to load the setup info
						if ( not PB.load(path) ):
							# Loading setup info failed.  Clean up and exit
							msg = wx.MessageBox((localText.getText("TXT_KEY_PITBOSS_ERROR_LOADING", ())), (localText.getText("TXT_KEY_PITBOSS_LOAD_ERROR", wx.ICON_ERROR)))
							PB.reset()
							event.Veto()
						else:
							# Successfully loaded, try hosting
							PB.setLoadFileName(path)
							if ( not PB.host() ):
								msg = wx.MessageBox((localText.getText("TXT_KEY_PITBOSS_ERROR_HOSTING", ())), (localText.getText("TXT_KEY_PITBOSS_HOST_ERROR", wx.ICON_ERROR)))
								PB.reset()
								event.Veto()
							else:
								bSaved = True
							
					else:
						# Didn't get a save file - veto the page change
						event.Veto()
						
				else:
					#User hit cancel - veto the page change
					event.Veto()
					
				# Destroy the dialog
				dlg.Destroy()
		
		else:
			# We are moving backward - reset the network layer
			PB.reset()
			PB.logout()
			
#
# Staging room (last page before launch)
#
class StagingPage(wx.wizard.WizardPageSimple):
	def __init__(self, parent):
		wx.wizard.WizardPageSimple.__init__(self, parent)
		self.myParent = parent
		
		# Get the game info struct
		gameData = PB.getGameSetupData()
		
		# Create our array of controls
		self.optionArray = []
		self.mpOptionArray = []
		self.whoArray = []
		self.civArray = []
		self.leaderArray = []
		self.teamArray = []
		self.diffArray = []
		self.statusArray = []
		
		# Build the initial selections
		# Map
		mapNameList = []
		rowNum = 0
		for rowNum in range(PB.getNumMapScripts()):
			mapNameList.append((PB.getMapNameAt(rowNum)))
			
		# World size
		sizeList = []
		rowNum = 0
		for rowNum in range(PB.getNumSizes()):
			sizeList.append((PB.getSizeAt(rowNum)))
			
		# Climate
		climateList = []
		rowNum = 0
		for rowNum in range(PB.getNumClimates()):
			climateList.append((PB.getClimateAt(rowNum)))
			
		# Sealevel
		seaLevelList = []
		rowNum = 0
		for rowNum in range(PB.getNumSeaLevels()):
			seaLevelList.append((PB.getSeaLevelAt(rowNum)))
			
		# Era
		eraList = []
		rowNum = 0
		for rowNum in range(PB.getNumEras()):
			eraList.append((PB.getEraAt(rowNum)))
			
		# Game speed
		speedList = []
		rowNum = 0
		for rowNum in range(PB.getNumSpeeds()):
			speedList.append((PB.getSpeedAt(rowNum)))
			
		# Options
		optionList = []
		rowNum = 0
		for rowNum in range(PB.getNumOptions()):
			optionList.append((PB.getOptionDescAt(rowNum)))
			
		# Create the master page sizer
		self.pageSizer = wx.BoxSizer(wx.VERTICAL)
		
		# Create the game options area
		border = wx.StaticBox(self, -1, ((localText.getText("TXT_KEY_PITBOSS_GAME_OPTIONS", ()))))
		self.optionsSizer = wx.StaticBoxSizer(border, wx.HORIZONTAL)
		
		# Create the drop down side
		self.dropDownSizer = wx.BoxSizer(wx.VERTICAL)
		
		# Create label/control pairs for map
		itemSizer = wx.BoxSizer(wx.VERTICAL)
		txt = wx.StaticText(self, -1, (localText.getText("TXT_KEY_PITBOSS_MAP", ())))
		self.mapChoice = wx.Choice(self, -1, (-1,-1), choices=mapNameList)
		self.mapChoice.SetStringSelection(gameData.getMapName())
		itemSizer.Add(txt)
		itemSizer.Add(self.mapChoice)
		self.dropDownSizer.Add(itemSizer, 0, wx.TOP, 3)
		self.Bind(wx.EVT_CHOICE, self.OnGameChoice, self.mapChoice)
		
		
		# Create label/control pairs for size
		itemSizer = wx.BoxSizer(wx.VERTICAL)
		txt = wx.StaticText(self, -1, (localText.getText("TXT_KEY_PITBOSS_SIZE", ())))
		self.sizeChoice = wx.Choice(self, -1, (-1,-1), choices=sizeList)
		self.sizeChoice.SetSelection(gameData.iSize)
		itemSizer.Add(txt)
		itemSizer.Add(self.sizeChoice)
		self.dropDownSizer.Add(itemSizer, 0, wx.TOP, 3)
		self.Bind(wx.EVT_CHOICE, self.OnGameChoice, self.sizeChoice)
		
		# Create label/control pairs for climate
		itemSizer = wx.BoxSizer(wx.VERTICAL)
		txt = wx.StaticText(self, -1, (localText.getText("TXT_KEY_PITBOSS_CLIMATE", ())))
		self.climateChoice = wx.Choice(self, -1, (-1,-1), choices=climateList)
		self.climateChoice.SetSelection(gameData.iClimate)
		itemSizer.Add(txt)
		itemSizer.Add(self.climateChoice)
		self.dropDownSizer.Add(itemSizer, 0, wx.TOP, 3)
		self.Bind(wx.EVT_CHOICE, self.OnGameChoice, self.climateChoice)
		
		# Create label/control pairs for sealevel
		itemSizer = wx.BoxSizer(wx.VERTICAL)
		txt = wx.StaticText(self, -1, (localText.getText("TXT_KEY_PITBOSS_SEALEVEL", ())))
		self.seaLevelChoice = wx.Choice(self, -1, (-1,-1), choices=seaLevelList)
		self.seaLevelChoice.SetSelection(gameData.iSeaLevel)
		itemSizer.Add(txt)
		itemSizer.Add(self.seaLevelChoice)
		self.dropDownSizer.Add(itemSizer, 0, wx.TOP, 3)
		self.Bind(wx.EVT_CHOICE, self.OnGameChoice, self.seaLevelChoice)
		
		# Create label/control pairs for era
		itemSizer = wx.BoxSizer(wx.VERTICAL)
		txt = wx.StaticText(self, -1, (localText.getText("TXT_KEY_PITBOSS_ERA", ())))
		self.eraChoice = wx.Choice(self, -1, (-1,-1), choices=eraList)
		self.eraChoice.SetSelection(gameData.iEra)
		itemSizer.Add(txt)
		itemSizer.Add(self.eraChoice)
		self.dropDownSizer.Add(itemSizer, 0, wx.TOP, 3)
		self.Bind(wx.EVT_CHOICE, self.OnGameChoice, self.eraChoice)
		
		# Create label/control pairs for speed
		itemSizer = wx.BoxSizer(wx.VERTICAL)
		txt = wx.StaticText(self, -1, (localText.getText("TXT_KEY_PITBOSS_SPEED", ())))
		self.speedChoice = wx.Choice(self, -1, (-1,-1), choices=speedList)
		self.speedChoice.SetSelection(gameData.iSpeed)
		itemSizer.Add(txt)
		itemSizer.Add(self.speedChoice)
		self.dropDownSizer.Add(itemSizer, 0, wx.TOP, 3)
		self.Bind(wx.EVT_CHOICE, self.OnGameChoice, self.speedChoice)
		
		# Create label/control pairs for custom map options
		self.buildCustomMapOptions( gameData.getMapName() )
		
		self.optionsSizer.Add(self.dropDownSizer, 0, wx.RIGHT, 10)
		
		# Create the multiplayer option column
		mpOptionsSizer = wx.BoxSizer(wx.VERTICAL)
		
		# Entry box to set turn timer time
		timerOutputSizer = wx.BoxSizer(wx.HORIZONTAL)
		timerPreText = wx.StaticText(self, -1, "Allow ")
		self.turnTimerEdit = wx.TextCtrl(self, -1, str(gameData.iTurnTime), size=(30,-1))
		timerPostText = wx.StaticText(self, -1, " hours to complete a turn")
		timerOutputSizer.Add(timerPreText, 0, wx.TOP, 5)
		timerOutputSizer.Add(self.turnTimerEdit, 0, wx.TOP, 5)
		timerOutputSizer.Add(timerPostText, 0, wx.TOP, 5)
		self.Bind(wx.EVT_TEXT, self.OnTurnTimeEntered, self.turnTimerEdit)
		
		mpOptionsSizer.Add(timerOutputSizer, 0, wx.ALL, 5)
		
		# Create and add Multiplayer option checkboxes
		rowNum = 0
		for rowNum in range(PB.getNumMPOptions()):
			mpCheckBox = wx.CheckBox(self, (rowNum+1000), (PB.getMPOptionDescAt(rowNum)))
			mpCheckBox.SetValue(gameData.getMPOptionAt(rowNum))
			mpOptionsSizer.Add(mpCheckBox, 0, wx.TOP, 5)
			self.mpOptionArray.append(mpCheckBox)
			self.Bind(wx.EVT_CHECKBOX, self.OnOptionChoice, mpCheckBox)
			
		self.optionsSizer.Add(mpOptionsSizer, 0, wx.ALL, 10)
			
		# Create the CheckBox side
		checkBoxSizer = wx.BoxSizer(wx.VERTICAL)
		
		# Create and add the Options checkboxes
		rowNum = 0
		for rowNum in range(PB.getNumOptions()):
			checkBox = wx.CheckBox(self, rowNum, (PB.getOptionDescAt(rowNum)))
			checkBox.SetValue(gameData.getOptionAt(rowNum))
			checkBoxSizer.Add(checkBox, 0, wx.TOP, 5)
			self.optionArray.append(checkBox)
			self.Bind(wx.EVT_CHECKBOX, self.OnOptionChoice, checkBox)
			
		self.optionsSizer.Add(checkBoxSizer, 0, wx.ALIGN_CENTER_VERTICAL|wx.LEFT, 10)
		
		# Add our options box to the page
		self.pageSizer.Add(self.optionsSizer, 0, wx.ALIGN_CENTER_HORIZONTAL|wx.ALL, 5)
		
		# Slot status - choices are static
		slotStatusList = ['Human', 'Computer', 'Closed']
		
		# Civilizations - get from app
		civList = []
		civList.append('Random')
		rowNum = 0
		for rowNum in range(PB.getNumCivs()):
			civList.append((PB.getCivAt(rowNum)))
			
		leaderList = ['Random']
		
		teamList = []
		rowNum = 0
		for rowNum in range(gc.getMAX_CIV_PLAYERS()):
			teamList.append(str(rowNum+1))
			
		playerPanel = wx.lib.scrolledpanel.ScrolledPanel(self, -1, size=(425, 300), style = wx.SUNKEN_BORDER)
		panelSizer = wx.BoxSizer(wx.VERTICAL)
		
		# Create a row - enough for the max players in a Pitboss game
		rowNum = 0
		for rowNum in range(gc.getMAX_CIV_PLAYERS()):
			# Create the border box
			border = wx.StaticBox(playerPanel, -1, (localText.getText("TXT_KEY_PITBOSS_PLAYER", (rowNum, ))), (0,(rowNum*30)))
			# Create the layout mgr
			rowSizer = wx.StaticBoxSizer(border, wx.HORIZONTAL)
			
			# Get the info struct
			playerData = PB.getPlayerSetupData(rowNum)
			
			# Slot status dropdown
			itemSizer = wx.BoxSizer(wx.VERTICAL)
			txt = wx.StaticText(playerPanel, -1, (localText.getText("TXT_KEY_PITBOSS_WHO", ())))
			dropDown = wx.Choice(playerPanel, rowNum, (-1,-1), choices=slotStatusList)
			dropDown.SetSelection(playerData.iWho)
			itemSizer.Add(txt)
			itemSizer.Add(dropDown)
			rowSizer.Add(itemSizer, 0, wx.TOP, 3)
			self.whoArray.append(dropDown)
			self.Bind(wx.EVT_CHOICE, self.OnPlayerChoice, dropDown)
			
			# Civ dropdown
			itemSizer = wx.BoxSizer(wx.VERTICAL)
			txt = wx.StaticText(playerPanel, -1, (localText.getText("TXT_KEY_PITBOSS_CIV", ())))
			dropDown = wx.Choice(playerPanel, rowNum, (-1,-1), choices=civList)
			dropDown.SetSelection(playerData.iCiv+1)
			itemSizer.Add(txt)
			itemSizer.Add(dropDown)
			rowSizer.Add(itemSizer, 0, wx.TOP, 3)
			self.civArray.append(dropDown)
			self.Bind(wx.EVT_CHOICE, self.OnPlayerChoice, dropDown)
			
			# Leader dropdown
			itemSizer = wx.BoxSizer(wx.VERTICAL)
			txt = wx.StaticText(playerPanel, -1, (localText.getText("TXT_KEY_PITBOSS_LEADER", ())))
			dropDown = wx.Choice(playerPanel, rowNum, (-1,-1), choices=leaderList)
			dropDown.SetSelection(playerData.iLeader+1)
			itemSizer.Add(txt)
			itemSizer.Add(dropDown)
			rowSizer.Add(itemSizer, 0, wx.TOP, 3)
			self.leaderArray.append(dropDown)
			self.Bind(wx.EVT_CHOICE, self.OnPlayerChoice, dropDown)
			
			# Team dropdown
			itemSizer = wx.BoxSizer(wx.VERTICAL)
			txt = wx.StaticText(playerPanel, -1, (localText.getText("TXT_KEY_PITBOSS_TEAM", ())))
			dropDown = wx.Choice(playerPanel, rowNum, (-1,-1), choices=teamList)
			dropDown.SetSelection(playerData.iTeam)
			itemSizer.Add(txt)
			itemSizer.Add(dropDown)
			rowSizer.Add(itemSizer, 0, wx.TOP, 3)
			self.teamArray.append(dropDown)
			self.Bind(wx.EVT_CHOICE, self.OnPlayerChoice, dropDown)
			
			# Difficulty dropdown
			itemSizer = wx.BoxSizer(wx.VERTICAL)
			txt = wx.StaticText(playerPanel, -1, (localText.getText("TXT_KEY_PITBOSS_DIFFICULTY", ())))
			statusTxt = wx.StaticText(playerPanel, rowNum, (PB.getHandicapAt(playerData.iDifficulty)))
			itemSizer.Add(txt)
			itemSizer.Add(statusTxt)
			rowSizer.Add(itemSizer, 0, wx.ALL, 5)
			self.diffArray.append(statusTxt)
			
			# Ready status
			itemSizer = wx.BoxSizer(wx.VERTICAL)
			txt = wx.StaticText(playerPanel, -1, (localText.getText("TXT_KEY_PITBOSS_STATUS", ())))
			statusTxt = wx.StaticText(playerPanel, rowNum, playerData.getStatusText())
			itemSizer.Add(txt)
			itemSizer.Add(statusTxt)
			rowSizer.Add(itemSizer, 0, wx.ALL, 5)
			self.statusArray.append(statusTxt)
			
			# Add row to page Sizer
			panelSizer.Add(rowSizer, 0, wx.ALL, 5)
			
		playerPanel.SetSizer(panelSizer)
		playerPanel.SetAutoLayout(1)
		playerPanel.SetupScrolling()
		
		self.pageSizer.Add(playerPanel, 0, wx.ALIGN_CENTER_HORIZONTAL|wx.ALL, 5)
		
		self.leaderRefresh = False
		
		self.Bind(wx.wizard.EVT_WIZARD_PAGE_CHANGED, self.OnPageChanged)
		self.Bind(wx.wizard.EVT_WIZARD_PAGE_CHANGING, self.OnPageChanging)
		
		self.SetSizer(self.pageSizer)
		
	def enableButtons(self):
		self.myParent.FindWindowById(wx.ID_FORWARD).Enable(True)
		self.myParent.FindWindowById(wx.ID_BACKWARD).Enable(True)
		
	def OnGameChoice(self, event):
		self.ChangeGameParam()
		
	def ChangeGameParam(self):
		strValue = self.turnTimerEdit.GetValue()
		if (len(strValue) > 0):
			intValue = (int)(self.turnTimerEdit.GetValue())
		else:
			intValue = 0
			
		PB.gameParamChanged( self.mapChoice.GetStringSelection(), self.sizeChoice.GetSelection(),
			self.climateChoice.GetSelection(), self.seaLevelChoice.GetSelection(),
			self.eraChoice.GetSelection(), self.speedChoice.GetSelection(), intValue )
			
	def OnCustomMapOptionChoice(self, event):
		# Get the option ID
		optionID = event.GetId()
		PB.customMapOptionChanged( optionID, self.customMapOptionArray[optionID].GetSelection() )
		
	def IsNumericString(self, myStr):
		for myChar in myStr:
			if myChar not in string.digits:
				return False
		return True
		
	def OnTurnTimeEntered(self, event):
		# Check to see if there is a time string
		if ( (self.turnTimerEdit.GetValue() != "")  ):
			# There is, make sure it's a number
			if ( not self.IsNumericString(self.turnTimerEdit.GetValue()) ):
				# It's not - lay the smack down				
				dlg = wx.MessageDialog(
					self, "Turn Time Value must be an integer",
					"Oops", wx.OK|wx.ICON_EXCLAMATION)
					
				# Show the modal dialog and get the response
				if dlg.ShowModal() == wx.ID_OK:
					# Clear out the TurnTimer Edit box
					self.turnTimerEdit.SetValue("")
			else:
				# It's a number
				self.ChangeGameParam()
		else:
			# It's been cleared
			self.ChangeGameParam()
			
	def OnOptionChoice(self, event):
		# Get the option ID
		optionID = event.GetId()
		
		# Values >= 1000 are MP options
		if  (optionID >= 1000):
			PB.mpOptionChanged( (optionID-1000), self.mpOptionArray[(optionID-1000)].GetValue() )
		else:
			PB.gameOptionChanged( optionID, self.optionArray[optionID].GetValue() )
		
	def OnPlayerChoice(self, event):
		# Get the row for the player modified
		rowNum = event.GetId()
		
		# See if we need to update the leader box
		if (not self.leaderRefresh):
			self.leaderRefresh = ( PB.getCiv(rowNum) != (self.civArray[rowNum].GetSelection()-1) );
				
		PB.playerParamChanged( rowNum, self.whoArray[rowNum].GetSelection(), self.civArray[rowNum].GetSelection()-1, self.teamArray[rowNum].GetSelection(), 
			PB.getGlobalLeaderIndex(self.civArray[rowNum].GetSelection()-1, self.leaderArray[rowNum].GetSelection()-1) )
		
	def OnPageChanging(self, event):
		# Check direction
		if (not event.GetDirection()):
			# We are trying to move backward - reset the network resources
			PB.reset()
			PB.logout()
			
	def OnPageChanged(self, event):
		global curPage
		
		# Determine what buttons should be enabled
		self.enableButtons()
		self.setDefaults()
		
		# We are the current page
		curPage = self
			
	def setDefaults(self):
		# Display the current initialization information
		global bSaved
		
		# Get game data first
		gameData = PB.getGameSetupData()
		
		# Set the selections currently in our init structure
		self.mapChoice.SetStringSelection(gameData.getMapName())
		if (bSaved):
			self.mapChoice.Disable()
		else:
			self.mapChoice.Enable(True)
		self.sizeChoice.SetSelection(gameData.iSize)
		if (bSaved):
			self.sizeChoice.Disable()
		else:
			self.sizeChoice.Enable(True)
		self.climateChoice.SetSelection(gameData.iClimate)
		if (bSaved):
			self.climateChoice.Disable()
		else:
			self.climateChoice.Enable(True)
		self.seaLevelChoice.SetSelection(gameData.iSeaLevel)
		if (bSaved):
			self.seaLevelChoice.Disable()
		else:
			self.seaLevelChoice.Enable(True)
		self.eraChoice.SetSelection(gameData.iEra)
		if (bSaved):
			self.eraChoice.Disable()
		else:
			self.eraChoice.Enable(True)
		self.speedChoice.SetSelection(gameData.iSpeed)
		if (bSaved):
			self.speedChoice.Disable()
		else:
			self.speedChoice.Enable(True)
		self.turnTimerEdit.SetValue(str(gameData.iTurnTime))
		if (bSaved):
			self.turnTimerEdit.Disable()
		else:
			self.turnTimerEdit.Enable(True)
			
		# Set selections of map options
		optionNum = 0
		for optionNum in range(PB.getNumCustomMapOptions(gameData.getMapName())):
			self.customMapOptionArray[optionNum].SetSelection(gameData.getCustomMapOption(optionNum))
			if (bSaved):
				self.customMapOptionArray[optionNum].Disable()
			else:
				self.customMapOptionArray[optionNum].Enable(True)	
				
		# set the mp options selection
		rowNum = 0
		for rowNum in range(PB.getNumMPOptions()):
			self.mpOptionArray[rowNum].SetValue(gameData.getMPOptionAt(rowNum))
			if (bSaved):
				self.mpOptionArray[rowNum].Disable()
			else:
				self.mpOptionArray[rowNum].Enable(True)
		
		# Set the options selected
		rowNum = 0
		for rowNum in range(PB.getNumOptions()):
			self.optionArray[rowNum].SetValue(gameData.getOptionAt(rowNum))
			if (bSaved):
				self.optionArray[rowNum].Disable()
			else:
				self.optionArray[rowNum].Enable(True)
				
		# Have the app suggest number of players based on map size
		PB.suggestPlayerSetup()
		
		rowNum = 0
		for rowNum in range(gc.getMAX_CIV_PLAYERS()):
			# Get the player data
			playerData = PB.getPlayerSetupData(rowNum)
			
			# We may need to add/remove items from who box
			self.refreshWhoBox(rowNum, playerData.iWho)
			self.whoArray[rowNum].SetSelection(playerData.iWho)
			if (bSaved):
				self.whoArray[rowNum].Disable()
			else:
				self.whoArray[rowNum].Enable(True)
			
			# Civ choices are static inside the instance
			civChoice = playerData.iCiv+1
			self.civArray[rowNum].SetSelection(civChoice)
			if (bSaved):
				self.civArray[rowNum].Disable()
			else:
				self.civArray[rowNum].Enable(True)
			
			# We may need to add/remove items from the leader box
			self.refreshLeaderBox(rowNum, playerData.iCiv)
			self.leaderRefresh = False
			self.leaderArray[rowNum].SetSelection(PB.getCivLeaderIndex(civChoice-1, playerData.iLeader)+1)
			if (bSaved):
				self.leaderArray[rowNum].Disable()
			else:
				self.leaderArray[rowNum].Enable(True)
			
			# Team choices are static
			self.teamArray[rowNum].SetSelection(playerData.iTeam)
			if (bSaved):
				self.teamArray[rowNum].Disable()
			else:
				self.teamArray[rowNum].Enable(True)
		
			# Difficulty choices are static
			self.diffArray[rowNum].SetLabel((PB.getHandicapAt(playerData.iDifficulty)))
			
			# Status is static
			self.statusArray[rowNum].SetLabel(playerData.getStatusText())
		
			
	def refreshRow(self, iRow):
		
		global bSaved
		
		# Disable finish button if all players not ready to start
		bAllReady = True
		
		# Don't wait for ready's if we're loading
		if (not bSaved):
			index = 0
			for index in range(gc.getMAX_CIV_PLAYERS()):
				if (PB.getWho(index) == 3): # If a row is taken by a human
					if (PB.getReady(index) == False): # If this human is not ready for the event to begin
						# Don't allow a launch
						bAllReady = False
						break
					
		if (not bAllReady):
			self.myParent.FindWindowById(wx.ID_FORWARD).Disable()
		else:
			self.myParent.FindWindowById(wx.ID_FORWARD).Enable(True)
					
		# Get information from the app for this row
		playerData = PB.getPlayerSetupData(iRow)
		
		# Refresh the choices in this slot
		self.refreshWhoBox(iRow, playerData.iWho)
		self.whoArray[iRow].SetSelection(playerData.iWho)
		
		# Get the Civ and see if we should refresh the list of leaders
		dropDown = self.civArray[iRow]
		civChoice = playerData.iCiv+1
		if (not self.leaderRefresh):
			self.leaderRefresh = (civChoice != dropDown.GetSelection())
		dropDown.SetSelection(civChoice)
		
		if (self.leaderRefresh):
			self.refreshLeaderBox(iRow, playerData.iCiv)
			self.leaderRefresh = False
								
		# Get the Leader
		dropDown = self.leaderArray[iRow]
		dropDown.SetSelection(PB.getCivLeaderIndex(civChoice-1, playerData.iLeader)+1)
		
		# Get the Team
		dropDown = self.teamArray[iRow]
		dropDown.SetSelection(playerData.iTeam)
		
		# Get the Difficulty
		self.diffArray[iRow].SetLabel((PB.getHandicapAt(playerData.iDifficulty)))
		
		
		# Modify Status 
		self.statusArray[iRow].SetLabel(playerData.getStatusText())
	
		
	def refreshWhoBox(self, iRow, iWho):
		# Add or remove choices depending on the state and the change
		dropDown = self.whoArray[iRow]
		
		if (iWho < 3):  # Status changing to non-taken state
			# Remove the player name from the drop down if it is there
			if (dropDown.GetCount() > 3):
				dropDown.Delete(3)
		else:			# Slot taken!  
			if (dropDown.GetCount() == 3):
				# Add and display the player name
				dropDown.Append((PB.getName(iRow)))
			else:
				# Set the current player name with the new one
				dropDown.SetString(3, (PB.getName(iRow)))
		
		
	def refreshLeaderBox(self, iRow, iCiv):
		# Need to reset the leader choices - first clear the list
		dropDown = self.leaderArray[iRow]
		dropDown.Clear()
		
		# Give the Random choice
		dropDown.Append((localText.getText("TXT_KEY_PITBOSS_RANDOM", ())))
			
		civChoice = iCiv+1
		if (civChoice != 0):
			# If there are leaders to list, list them
			i = 0
			iNumLeaders = PB.getNumLeaders(civChoice-1)
			for i in range(iNumLeaders):
				dropDown.Append((PB.getCivLeaderAt(civChoice-1, i)))
		
		dropDown.SetSelection(0)
		
	def refreshCustomMapOptions(self, szMapName):
		# Clear the widgets from the custom option area
		i = 0
		for i in range( len(self.customItemSizerArray) ):
			currentSizer = self.customItemSizerArray[i]
			currentSizer.Remove(1)	#dropDown
			currentSizer.Remove(0)	#txt
			self.dropDownSizer.Remove(currentSizer)
		
		self.buildCustomMapOptions(szMapName)
		
		# Now rebuild the sizers
		self.dropDownSizer.Layout()
		self.optionsSizer.Layout()
		self.pageSizer.Layout()
		
	def buildCustomMapOptions(self, szMapName):
		gameData = PB.getGameSetupData()
		
		# Declare storage arrays
		self.customItemSizerArray = []
		self.customMapOptionArray = []
		
		# Get the map options
		customMapOptionsList = []
		optionNum = 0
		for optionNum in range(PB.getNumCustomMapOptions( gameData.getMapName() )):
			customMapOptionValuesList = []
			rowNum = 0
			for rowNum in range(PB.getNumCustomMapOptionValues( optionNum, gameData.getMapName() )):
				customMapOptionValuesList.append(PB.getCustomMapOptionDescAt( optionNum, rowNum, gameData.getMapName() ))
			customMapOptionsList.append(customMapOptionValuesList[:])
			
		# Create label/control pairs for custom map option
		optionNum = 0
		for optionNum in range(PB.getNumCustomMapOptions( szMapName )):
			itemSizer = wx.BoxSizer(wx.VERTICAL)
			self.customItemSizerArray.append(itemSizer)
			txt = wx.StaticText( self, -1, PB.getCustomMapOptionName(optionNum, szMapName) )
			optionDropDown = wx.Choice(self, optionNum, (-1,-1), choices=customMapOptionsList[optionNum])
			optionDropDown.SetSelection(gameData.getCustomMapOption(optionNum))
			itemSizer.Add(txt)
			itemSizer.Add(optionDropDown)
			self.customMapOptionArray.append(optionDropDown)
			self.dropDownSizer.Add(itemSizer, 0, wx.TOP, 3)
			self.Bind(wx.EVT_CHOICE, self.OnCustomMapOptionChoice, optionDropDown)

#
# Progress bar dialog
#
class ProgressDialog(wx.Dialog):
	def __init__(self, parent):
		global curPage
		wx.Dialog.__init__(self, curPage, -1, "Patch Progress", wx.DefaultPosition, wx.DefaultSize, wx.STAY_ON_TOP)
		
		self.myParent = parent
		
		self.iValue = 0
		self.iTotal = 100	# Arbitrary Value until it's actually set
		
		# Place the progress bar
		selections = ['LAN', 'Internet']
		self.sizer = wx.BoxSizer(wx.VERTICAL)
		
		self.progress = None
		progressSizer = wx.BoxSizer(wx.VERTICAL)
		self.sizer.Add(progressSizer, 0, wx.ALL, 5)
		
		# Add a cancel button
		cancelButton = wx.Button(self, -1, localText.getText("TXT_KEY_SCREEN_CANCEL", ()))
		cancelButton.SetHelpText(localText.getText("TXT_KEY_CANCEL_PATCH_DOWNLOAD", ()))
		self.Bind(wx.EVT_BUTTON, self.OnCancelDownload, cancelButton)
		self.sizer.Add(cancelButton, 0, wx.ALL, 5)
		
		self.SetSizer(self.sizer)
		
	def setValue(self, iValue):
		if (iValue > 0):
			self.iValue = iValue
			if (self.progress != None):
				self.progress.SetValue(self.iValue)
			
	def setTotal(self, iTotal):
		if (iTotal != self.iTotal):
			if (iTotal > 0):
				self.iTotal = iTotal
				if (self.progress == None):
					self.progress = wx.Gauge(self, self.iValue, self.iTotal)
					self.sizer.Add(self.progress, 0, wx.ALL, 5)
				
	
	def OnCancelDownload(self, event):
		"Cancel download handler"
		# Tell the app
		self.myParent.cancelDownload()
		
		# Return to our caller
		if (self.IsModal()):
			self.EndModal(wx.ID_CANCEL)
		else:
			self.Show(false)
			return wx.ID_CANCEL
		

#
# main app class
#
class StartupIFace(wx.App):
	def OnInit(self):
		global curPage
		
		"Create the Pitboss Setup Wizard"
		self.wizard = wx.wizard.Wizard(None, -1, (localText.getText("TXT_KEY_PITBOSS_TITLE", ())))
		
		# Create each wizard page
		self.netSelect = NetSelectPage(self.wizard)
		self.login = LoginPage(self.wizard)
		self.loadSelect = LoadSelectPage(self.wizard)
		self.staging = StagingPage(self.wizard)
		
		self.netSelect.SetNext(self.login)
		self.login.SetPrev(self.netSelect)
		self.login.SetNext(self.loadSelect)
		self.loadSelect.SetPrev(self.netSelect)
		self.loadSelect.SetNext(self.staging)
		self.staging.SetPrev(self.netSelect)
		
		self.progressDlg = None
		
		curPage = self.netSelect
		
		self.wizard.FitToPage(curPage)
		
		# Create a timer callback that will handle our updates
		timerID = wx.NewId()
		self.updateTimer = wx.Timer(self, timerID)
		self.Bind(wx.EVT_TIMER, self.OnTimedUpdate, id=timerID)
		self.updateTimer.Start(250)
		
		return True
		
	def startWizard(self):
		global curPage
		
		# Try starting the wizard
		if (self.wizard.RunWizard(curPage)):
			# launch game here
			self.updateTimer.Stop()
			PB.launch()
			return True
		else:
			# user cancelled...
			self.updateTimer.Stop()
			PB.quit()
			return False
	
	def OnTimedUpdate(self, event):
		# Handle received net messages
		PB.handleMessages()
		
	def patchAvailable(self, patchName, patchUrl):
		global curPage
		
		# Save info and display a popup to the user
		if ( (curPage == self.login) or (curPage == self.loadSelect) ):
			# Show the popup
			curPage.patchAvailable(patchName, patchUrl)
		
	def patchProgress(self, bytesRecvd, bytesTotal):
		global bPatchConfirmed
		if (not bPatchConfirmed):
			# Display our progress
			if (self.progressDlg == None):
				# Need to create the dialog
				self.progressDlg = ProgressDialog(self)
				self.progressDlg.Show(true)
				
			self.progressDlg.setTotal(bytesTotal)
			self.progressDlg.setValue(bytesRecvd)
			
	def cancelDownload(self):
		global bPatchConfirmed
		bPatchConfirmed = true
		
		# get rid of the dialog
		if (self.progressDlg != None):
			self.progressDlg.Show(false)
			self.progressDlg = None
			
		# Tell the application
		PB.cancelPatchDownload()
		
	def patchDownloadComplete(self, bSuccess):
		# Download complete - check if it was successful
		global curPage
		global bPatchConfirmed
		global bPatchOK
		
		# get rid of the dialog
		if (self.progressDlg != None):
			self.progressDlg.Show(false)
			self.progressDlg = None
		
		if (bSuccess):
			curPage.patchComplete()
		else:
			bPatchOK = false
			msg = wx.MessageBox("Patch was not successfully retrieved", "Download failure", wx.ICON_ERROR)
		
		bPatchConfirmed = true
		
		curPage.enableButtons()
			
	def upToDate(self):
		global curPage
		
		global bPatchConfirmed
		global bPatchOK
		bPatchConfirmed = true
		bPatchOK = true
		
		if ( (curPage == self.login) or (curPage == self.loadSelect) ):
			curPage.enableButtons()
		
		
	def refreshRow(self, iRow):
		global curPage
		
		# Get the latest data from the app and display in the view
		if (curPage == self.staging):
			# In the staging room, update the row
			curPage.refreshRow(iRow)
			
	def refreshCustomMapOptions(self, szMapName):
		global curPage
		
		# Refresh the page if we in the staging window
		if (curPage == self.staging):
			# Update the custom map options in the staging room
			curPage.refreshCustomMapOptions(szMapName)
	