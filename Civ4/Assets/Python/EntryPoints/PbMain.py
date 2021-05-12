## Sid Meier's Civilization 4
## Copyright Firaxis Games 2005
#
# Sample PitBoss window/app framework
# Mustafa Thamer 2-15-05
#
import PbWizard
import PbAdmin

app = None
bAdmin = False

#
# entry point function
# 
def create():
	global app
	app = PbWizard.StartupIFace(0)
	
#
# entry point function
#
def run():
	global app
	app.startWizard()
	
#
# entry point function
#
def update():
	global bAdmin
	global app
	
	if (not bAdmin):
		app = PbAdmin.AdminIFace(0)
		bAdmin = True
		
	app.update()
	
#
# entry point functions
#
def patchAvailable(argsList):
	global app
	patchName = argsList[0]
	patchUrl = argsList[1]
	app.patchAvailable(patchName, patchUrl)
	
def patchProgress(argsList):
	global app
	bytesRecvd = argsList[0]
	bytesTotal = argsList[1]
	app.patchProgress(bytesRecvd, bytesTotal)
	
def patchDownloadComplete(argsList):
	global app
	bSuccess = argsList[0]
	app.patchDownloadComplete(bSuccess)
	
def appUpToDate():
	app.upToDate()
	
def refreshRow(argsList):
	global app
	rowNum = argsList[0]
	app.refreshRow(rowNum)
	
def refreshCustomMapOptions(argsList):
	global app
	mapName = argsList[0]
	app.refreshCustomMapOptions(mapName)
	
def getMessageOfTheDay():
	global app
	return app.getMotD()
	
def addChatMessage(argsList):
	global app
	szMessage = argsList[0]
	app.addChatMessage(szMessage)
	