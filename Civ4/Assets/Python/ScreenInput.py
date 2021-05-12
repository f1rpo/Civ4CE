## Sid Meier's Civilization 4
## Copyright Firaxis Games 2005
#	enum NotifyCode
#	{
#		NOTIFY_CLICKED,
#		NOTIFY_DBL_CLICKED,
#		NOTIFY_FOCUS,
#		NOTIFY_UNFOCUS,
#		NOTIFY_CURSOR_MOVE_ON,
#		NOTIFY_CURSOR_MOVE_OFF,
#		NOTIFY_CHARACTER,
#		NOTIFY_SCROLL_UP,
#		NOTIFY_SCROLL_DOWN,
#		NOTIFY_NEW_HORIZONTAL_STOP,
#		NOTIFY_NEW_VERTICAL_STOP,
#		NOTIFY_LISTBOX_ITEM_SELECTED,
#		NOTIFY_FLYOUT_ITEM_SELECTED,
#		NOTIFY_MOUSEMOVE,
#		NOTIFY_MOUSEWHEELUP,
#		NOTIFY_MOUSEWHEELDOWN,
#		NOTIFY_MOUSELREPEAT,
#		NOTIFY_TEXTCHANGE,
#		NOTIFY_LINKEXECUTE,
#		NOTIFY_MOVIE_DONE,
#		NOTIFY_SLIDER_NEWSTOP,
#		NOTIFY_TABLE_HEADER_SELECTED,
#	};
#


import CvUtil


# Class to decipher and make screen input easy to read...
class ScreenInput:

	# Init call...
	def __init__ (self, argsList):
		self.eNotifyCode = argsList[0]
		self.iData = argsList[1]
		self.uiFlags = argsList[2]
		self.iItemID = argsList[3]
		self.ePythonFileEnum = argsList[4]
		self.szFunctionName = argsList[5]
		self.bShift = argsList[6]
		self.bCtrl = argsList[7]
		self.bAlt = argsList[8]
		self.iMouseX = argsList[9]
		self.iMouseY = argsList[10]
		self.iButtonType = argsList[11]
		self.iData1 = argsList[12]
		self.iData2 = argsList[13]
		self.bOption = argsList[14]

	# NotifyCode
	def getNotifyCode (self):
		return self.eNotifyCode

	# Data
	def getData (self):
		return self.iData

	# Flags
	def getFlags (self):
		return self.uiFlags

	# Item ID
	def getID (self):
		return self.iItemID

	# Python File
	def getPythonFile (self):
		return self.ePythonFileEnum

	# Function Name...
	def getFunctionName (self):
		return self.szFunctionName

	# Shift Key Down
	def isShiftKeyDown (self):
		return self.bShift

	# Ctrl Key Down
	def isCtrlKeyDown (self):
		return self.bCtrl

	# Alt Key Down
	def isAltKeyDown (self):
		return self.bAlt

	# X location of the mouse cursor
	def getMouseX (self):
		return self.iMouseX

	# Y location of the mouse cursor
	def getMouseY (self):
		return self.iMouseY

	# WidgetType
	def getButtonType (self):
		return self.iButtonType

	# Widget Data 1
	def getData1 (self):
		return self.iData1

	# Widget Data 2
	def getData2 (self):
		return self.iData2

	# Widget Option
	def getOption (self):
		return self.bOption

