## Sid Meier's Civilization 4
## Copyright Firaxis Games 2005
##
## Jesse Smith - 06-2005
## - Class to handle Quest Messaging

import CvUtil
import CvTutorialQuestScreen
import CvScreenEnums
from CvPythonExtensions import *

gc = CyGlobalContext()

class CvQuestManager:
	' Class for Managing Quests '
	def __init__(self):
		#CvUtil.pyPrint("CvQuestManager.__init__ %s" %self)
		self.__l_Quests = list()
		self.__l_CompletedQuests = list()
		self.__screenName = ""
		self.__iScreenID = -1
	
	def initQuestManager( self, strQuestType, argsList ):
		#CvUtil.pyPrint( "CvQuestManager.initQuestManager" )
		objective, update = argsList
		self.strQuestType = strQuestType
		self.strCurrentObjective = objective
		self.strQuestUpdate = update
	
	def setScreenValues( self, strScreenName, iScreenID ):
		self.__screenName = strScreenName
		self.__iScreenID = iScreenID

	def getQuestScreen( self ):
		if self.__iScreenID == CvScreenEnums.TUTORIAL_SCREEN:
			return CvAdvisorScreen.CvAdvisorScreen()
	
	def showQuestScreen(self):
		self.getQuestScreen().interfaceScreen()			
		
	def getCompletedQuestMessages( self ):
		' list - returns shown message list - called by CvAppInterface.onSave '
		CvUtil.pyPrint( "CvQuestManager.getCompletedQuestMessages" )
		return self.__l_CompletedQuests
	
	def setCompletedQuestMessages( self, data ):
		' called by CvAppInterface.OnLoad '
		CvUtil.pyPrint( "CvQuestManager.setCompletedQuestMessages" )
		self.__l_ShownMessages = data
	
	def loadQuestInfos( self ):
		' initializes data from dictionary_TutorialQuestInfo and adds them to l_Quests '
		CvUtil.pyPrint( "CvQuestManager.initQuestInfos (%s)" % self.strQuestType )
		[ self.__l_Quests.append(gc.getQuestInfo(i)) for i in range(gc.getNumQuestInfos()) if self.strQuestType in gc.getQuestInfo(i).getType() ]
	
	def processQuest( self, strQuestInfoKey ):
		' handles the processing of the quest, main loop '
		CvUtil.pyPrint( "CvQuestManager.processQuest (%s)" % strQuestInfoKey )
		
		CyInterface().stop2DSound()

		Quest = self.getQuest( strQuestInfoKey )
		
		# if strQuestInfoKey has not been completed
		if ( Quest and strQuestInfoKey not in self.__l_CompletedQuests):
			# if the linked quest is not yet complete then process this quest
			if ( not self.isQuestLinkComplete( Quest ) ):
				# update CvTutorialQuestScreen
				self.updateQuestObjective( Quest )
				self.updateQuestBodyText( Quest )
				self.displayImmediateMessage( Quest )
				self.doQuestComplete( Quest )
				return True
			
			# otherwise process quest link
			else:
				CvUtil.pyPrint( "CvQuestManager.processQuest: Processing Quest (%s) Link" % Quest.getType() )
				self.doQuestLink( Quest )
				return True
		return False

	def getQuest( self, strQuestInfoKey ):
		' returns a Quest object from the g_QuestInfos '
		if self.__l_Quests:
			for Quest in self.__l_Quests:
				if ( Quest.getType() == strQuestInfoKey ):
					return Quest
		return None
	
	def updateQuestObjective( self, Quest ):
		' updates the header text '
		CvUtil.pyPrint( "CvQuestManager.updateQuestObjective (%s)" % Quest.getType() )
		if ( Quest.getQuestObjective() != "NONE" ):
			self.playQuestSound(Quest)
			objectiveText = u"%s %s" %( self.strCurrentObjective, self.getQuestText(Quest.getQuestObjective()) )
			self.getQuestScreen().updateHeader( objectiveText )
	
	def updateQuestBodyText( self, Quest ):
		' updates the body text '
		CvUtil.pyPrint( "CvQuestManager.updateQuestBodyText (%s)" % Quest.getType() )
		if ( Quest.getQuestBodyText() != "NONE" ):
			bodyText = "%s" %(  self.getQuestText(Quest.getQuestBodyText()), )
			self.getQuestScreen().updateBodyText( bodyText )
	
	def displayImmediateMessage( self, Quest ):
		' displays an immediate message '
		CvUtil.pyPrint( "CvQuestManager.displayImmediateMessage (%s)" % Quest.getType() )
		if ( Quest.getNumQuestMessages() > 0 ):
			for i in range( Quest.getNumQuestMessages() ):
				messageText = "%s" %( self.getQuestText(Quest.getQuestMessages(i)),)
				CyInterface().addImmediateMessage( messageText, "" )
			
	def isQuestLinkComplete( self, Quest ):
		' test to see if the quests link has been completed or not '
		CvUtil.pyPrint( "CvQuestManager.isQuestLinkComplete (%s)" % Quest.getType() )
		if ( Quest.getNumQuestLinks() > 0 ):	
			return ( Quest.getType() in self.__l_CompletedQuests )
		return False
	
	def doQuestComplete( self, Quest ):
		' marks a quest Complete '
		CvUtil.pyPrint( "CvQuestManager.doQuestComplete (%s)" % Quest.getType() )
		self.__l_CompletedQuests.append( Quest.getType() )
	
	def getQuestText( self, strQuestInfoType ):
		' gets the Quest Text from the Translator '
		CvUtil.pyPrint( "CvQuestManager.getQuestText (%s)" % strQuestInfoType )
		return CyTranslator().getText(strQuestInfoType, tuple())
	
	def getLastQuestType( self ):
		' returns the questinfo type for the last quest that was processed '
		CvUtil.pyPrint( "CvQuestManager.getLastQuestType (%s)" % self.__l_CompletedQuests[len(self.__l_CompletedQuests)-1] )
		return self.__l_CompletedQuests[ len(self.__l_CompletedQuests) - 1 ]
	
	def isQuestCompleted( self, strQuestInfoType ):
		' returns whether a questinfo type has been completed '
		if ( strQuestInfoType in self.__l_CompletedQuests ):
			return True
		return False
	
	def playQuestSound( self, Quest ):
		' plays a 2D sound for the quest '
		CvUtil.pyPrint( "CvQuestManager.playQuestSound (%s)" % Quest.getType() )
		questSound = Quest.getQuestSounds(0)
		if questSound:
			CyInterface().playGeneralSound(questSound)
