## Sid Meier's Civilization 4
## Copyright Firaxis Games 2005
from CvPythonExtensions import *
import CvUtil

DebugLogging = False

gc = CyGlobalContext()

class CvDiplomacy:
	"Code used by Civ Diplomacy interface"

	def __init__(self):
		"constructor - set up class vars, AI and User strings"
		if DebugLogging:
			print "Launching Diplomacy"

		self.iLastResponseID = -1

		self.diploScreen = CyDiplomacy()

	def setDebugLogging(self, bDebugLogging):
		global DebugLogging
		DebugLogging = bDebugLogging

	def determineResponses (self, eComment):
		"Will determine the user responses given an AI comment"
		if DebugLogging:
			print "CvDiplomacy.determineResponses: %s" %(eComment,)

		# Eliminate previous comments
		self.diploScreen.clearUserComments()

		# If the AI is declaring war
		if (self.isComment(eComment, "AI_DIPLOCOMMENT_DECLARE_WAR") ):

			# We respond to their declaration
			self.addUserComment("USER_DIPLOCOMMENT_WAR_RESPONSE", -1, -1)
			self.diploScreen.endTrade()

		# If this is the first time we are being contacted by the AI
		elif (self.isComment(eComment, "AI_DIPLOCOMMENT_FIRST_CONTACT")):

			# We say hi and begin our peace
			self.addUserComment("USER_DIPLOCOMMENT_PEACE", -1, -1)

			# if you are on different teams and NOT at war, give the user the option to declare war
			#if (gc.getTeam(gc.getGame().getActiveTeam()).canDeclareWar(gc.getPlayer(self.diploScreen.getWhoTradingWith()).getTeam())):
			#	self.addUserComment("USER_DIPLOCOMMENT_WAR", -1, -1)

			self.diploScreen.endTrade()

		# The AI refuses to talk
		elif (self.isComment(eComment, "AI_DIPLOCOMMENT_REFUSE_TO_TALK") ):

			# Give the option to exit
			self.addUserComment("USER_DIPLOCOMMENT_EXIT", -1, -1)
			self.diploScreen.endTrade();

		# If the AI is offering a city oo
		# If the AI is giving help
		elif (self.isComment(eComment, "AI_DIPLOCOMMENT_OFFER_CITY") or
					self.isComment(eComment, "AI_DIPLOCOMMENT_GIVE_HELP")):

			# We can accept their offer
			self.addUserComment("USER_DIPLOCOMMENT_ACCEPT_OFFER", -1, -1)
			# Or reject it...
			self.addUserComment("USER_DIPLOCOMMENT_REJECT_OFFER", -1, -1)

		# If the AI is offering a deal
		elif (self.isComment(eComment, "AI_DIPLOCOMMENT_OFFER_PEACE") or
					self.isComment(eComment, "AI_DIPLOCOMMENT_OFFER_DEAL")):

			# We can accept their offer
			self.addUserComment("USER_DIPLOCOMMENT_ACCEPT_OFFER", -1, -1)
			# Or we can try to negotiate
			self.addUserComment("USER_DIPLOCOMMENT_RENEGOTIATE", -1, -1)
			# Or reject it...
			self.addUserComment("USER_DIPLOCOMMENT_REJECT_OFFER", -1, -1)

		# If the AI is cancelling a deal
		elif (self.isComment(eComment, "AI_DIPLOCOMMENT_CANCEL_DEAL")):

			# We can try to renegotiate
			self.addUserComment("USER_DIPLOCOMMENT_RENEGOTIATE", -1, -1)
			# Or just exit...
			self.addUserComment("USER_DIPLOCOMMENT_NO_RENEGOTIATE", -1, -1)

		# If the AI is demanding tribute
		elif (self.isComment(eComment, "AI_DIPLOCOMMENT_ASK_FOR_HELP")):

			# We can give them help
			self.addUserComment("USER_DIPLOCOMMENT_GIVE_HELP", -1, -1)
			# Or refuse...
			self.addUserComment("USER_DIPLOCOMMENT_REFUSE_HELP", -1, -1)

		# If the AI is demanding tribute
		elif (self.isComment(eComment, "AI_DIPLOCOMMENT_DEMAND_TRIBUTE")):

			# We can accept their demands
			self.addUserComment("USER_DIPLOCOMMENT_ACCEPT_DEMAND", -1, -1)
			# Or reject them...
			self.addUserComment("USER_DIPLOCOMMENT_REJECT_DEMAND", -1, -1)

		elif (self.isComment(eComment, "AI_DIPLOCOMMENT_BUY_UNITS")):

			self.addUserComment("USER_DIPLOCOMMENT_ACCEPT_BUY_UNITS", -1, -1)
			self.addUserComment("USER_DIPLOCOMMENT_REJECT_OFFER", -1, -1)

		# If the AI is raising taxes
		elif (self.isComment(eComment, "AI_DIPLOCOMMENT_KISS_PINKY")):
		
			# We can accept their demands
			self.addUserComment("USER_DIPLOCOMMENT_KISS_PINKY", -1, -1)

			# Or reject them...
			player = gc.getPlayer(gc.getGame().getActivePlayer())
			eYield = player.getHighestTradedYield()
# PatchMod: Tax party city START
			iCityId = player.getHighestStoredYieldPartyCityId(eYield)
#			iCityId = player.getHighestStoredYieldCityId(eYield)
# PatchMod: Tax party city END
			city = player.getCity(iCityId)
			if city.isNone():
				szCityName = u""
			else:
				szCityName = city.getNameKey()

			self.addUserComment("USER_DIPLOCOMMENT_TAX_PARTY", eYield, -1, szCityName, gc.getYieldInfo(eYield).getTextKey())

		# accept pinky ring
		elif (self.isComment(eComment, "AI_DIPLOCOMMENT_ACCEPT_PINKY")):

			# We can accept their demands
			self.addUserComment("USER_DIPLOCOMMENT_ACCEPT_PINKY", -1, -1)

		# reject pinky ring
		elif (self.isComment(eComment, "AI_DIPLOCOMMENT_REJECT_PINKY")):

			player = gc.getPlayer(gc.getGame().getActivePlayer())
			eYield = player.getHighestTradedYield()
			iCityId = player.getHighestStoredYieldCityId(eYield)
			self.addUserComment("USER_DIPLOCOMMENT_REJECT_PINKY", eYield, iCityId)

		# If the king is asking for gold
		elif (self.isComment(eComment, "AI_DIPLOCOMMENT_KING_ASK_FOR_GOLD")):

			# We can accept their demands
			self.addUserComment("USER_DIPLOCOMMENT_KING_GIVE_GOLD", -1, -1)
			# Or reject them...
			self.addUserComment("USER_DIPLOCOMMENT_KING_REFUSE_GOLD", -1, -1)

		elif (self.isComment(eComment, "AI_DIPLOCOMMENT_KING_ACCEPT_GOLD")):

			# We can accept their demands
			self.addUserComment("USER_DIPLOCOMMENT_KING_ACCEPTED_GOLD", -1, -1)

		elif (self.isComment(eComment, "AI_DIPLOCOMMENT_KING_REFUSE_GOLD")):

			self.addUserComment("USER_DIPLOCOMMENT_KING_REFUSED_GOLD", -1, -1)

		elif (self.isComment(eComment, "AI_DIPLOCOMMENT_KING_REVIVE")):

			self.addUserComment("USER_DIPLOCOMMENT_KING_REVIVE", -1, -1)

		elif (self.isComment(eComment, "AI_DIPLOCOMMENT_KING_GIFT_SHIP")):

			self.addUserComment("USER_DIPLOCOMMENT_KING_GIFT_SHIP", -1, -1)
			
		# If the AI is giving goodies to our scout
		elif (self.isComment(eComment, "AI_DIPLOCOMMENT_CHIEF_GOODY")):

			# accept
			self.addUserComment("USER_DIPLOCOMMENT_CHIEF_GOODY", -1, -1)

		# If the AI is teaching our unit
		elif (self.isComment(eComment, "AI_DIPLOCOMMENT_LIVE_AMONG_NATIVES")):

			# accept
			self.addUserComment("USER_DIPLOCOMMENT_LIVE_AMONG_NATIVES_ACCEPT", -1, -1)

			# reject
			self.addUserComment("USER_DIPLOCOMMENT_LIVE_AMONG_NATIVES_REJECT", -1, -1)

		# If the AI is offering to transport treasure
		elif (self.isComment(eComment, "AI_DIPLOCOMMENT_TREASURE_TRANSPORT")):

			# accept
			self.addUserComment("USER_DIPLOCOMMENT_TREASURE_TRANSPORT_ACCEPT", -1, -1)

			# reject
			self.addUserComment("USER_DIPLOCOMMENT_TREASURE_TRANSPORT_REJECT", -1, -1)

		elif (self.isComment(eComment, "AI_DIPLOCOMMENT_FOUND_CITY_NO_FOOD")):

			self.addUserComment("USER_DIPLOCOMMENT_FOUND_CITY_NO_FOOD_YES", -1, -1)
			self.addUserComment("USER_DIPLOCOMMENT_FOUND_CITY_NO_FOOD_NO", -1, -1)

		elif (self.isComment(eComment, "AI_DIPLOCOMMENT_FOUND_CITY_INLAND")):

			self.addUserComment("USER_DIPLOCOMMENT_FOUND_CITY_INLAND_YES", -1, -1)
			self.addUserComment("USER_DIPLOCOMMENT_FOUND_CITY_INLAND_NO", -1, -1)

		elif (self.isComment(eComment, "AI_DIPLOCOMMENT_FOUND_FIRST_CITY")):

			self.addUserComment("USER_DIPLOCOMMENT_FOUND_FIRST_CITY", -1, -1)

		elif (self.isComment(eComment, "AI_DIPLOCOMMENT_FOUND_CITY")):

			self.addUserComment("USER_DIPLOCOMMENT_FOUND_CITY_BUY", -1, -1)
			self.addUserComment("USER_DIPLOCOMMENT_FOUND_CITY_TAKE", -1, -1)
			self.addUserComment("USER_DIPLOCOMMENT_FOUND_CITY_CANCEL", -1, -1)

		elif (self.isComment(eComment, "AI_DIPLOCOMMENT_FOUND_CITY_CANT_AFFORD")):

			self.addUserComment("USER_DIPLOCOMMENT_FOUND_CITY_TAKE", -1, -1)
			self.addUserComment("USER_DIPLOCOMMENT_FOUND_CITY_CANCEL", -1, -1)

		# If the AI is gifting some goods (yields)
		elif (self.isComment(eComment, "AI_DIPLOCOMMENT_NATIVES_YIELD_GIFT")):

			# accept
			self.addUserComment("USER_DIPLOCOMMENT_NATIVES_YIELD_GIFT_ACCEPT", -1, -1)

		# If the AI is pressuring us to join their war
		elif (self.isComment(eComment, "AI_DIPLOCOMMENT_JOIN_WAR")):

			# We can accept their demands
			self.addUserComment("USER_DIPLOCOMMENT_JOIN_WAR", -1, -1)
			# Or reject them...
			self.addUserComment("USER_DIPLOCOMMENT_NO_JOIN_WAR", -1, -1)

		# If the AI is pressuring us to stop trading with their enemy
		elif (self.isComment(eComment, "AI_DIPLOCOMMENT_STOP_TRADING")):

			# We can accept their demands
			self.addUserComment("USER_DIPLOCOMMENT_STOP_TRADING", -1, -1)
			# Or reject them...
			self.addUserComment("USER_DIPLOCOMMENT_NO_STOP_TRADING", -1, -1)

		# If we are viewing our current deals or
		elif (self.isComment(eComment, "AI_DIPLOCOMMENT_CURRENT_DEALS")):

			# Exit option
			self.addUserComment("USER_DIPLOCOMMENT_NEVERMIND", -1, -1)
			self.addUserComment("USER_DIPLOCOMMENT_EXIT", -1, -1)
			self.diploScreen.startTrade( eComment, true)

		# If we are trading or
		# If we are trying another proposal or
		# If they reject our offer or
		# If they reject our demand
		elif (self.isComment(eComment, "AI_DIPLOCOMMENT_TRADING") or
					self.isComment(eComment, "AI_DIPLOCOMMENT_REJECT") or
					self.isComment(eComment, "AI_DIPLOCOMMENT_SORRY") or
					self.isComment(eComment, "AI_DIPLOCOMMENT_TRY_THIS_DEAL") or
					self.isComment(eComment, "AI_DIPLOCOMMENT_NO_DEAL") or
					self.isComment(eComment, "AI_DIPLOCOMMENT_REJECT_ASK") or
					self.isComment(eComment, "AI_DIPLOCOMMENT_REJECT_DEMAND")):

			# If no one is currently offering anything
			if (self.diploScreen.ourOfferEmpty() == 1 and self.diploScreen.theirOfferEmpty() == 1):

				# If we are at war, allow us to suggest a peace treaty
				if (self.diploScreen.atWar()):
					self.addUserComment("USER_DIPLOCOMMENT_PROPOSE", -1, -1)
					self.addUserComment("USER_DIPLOCOMMENT_OFFER_PEACE", -1, -1)

			# If one of us has something on the table
			if (self.diploScreen.ourOfferEmpty() == 0 or self.diploScreen.theirOfferEmpty() == 0):

				# If the offer is from the AI
				if (self.diploScreen.isAIOffer()):

					# We can accept or reject the offer
					self.addUserComment("USER_DIPLOCOMMENT_ACCEPT", -1, -1)
					self.addUserComment("USER_DIPLOCOMMENT_REJECT", -1, -1)

				# Otherwise this is a player offer to the AI
				else:

					# This is a two way deal
					if (self.diploScreen.ourOfferEmpty() == 0 and self.diploScreen.theirOfferEmpty() == 0):

						# Insert the propose trade button
						self.addUserComment("USER_DIPLOCOMMENT_PROPOSE", -1, -1)

						# During peace, see what we can get for these items
						if (not self.diploScreen.atWar()):
							if (gc.getGame().getActiveTeam() != gc.getPlayer(self.diploScreen.getWhoTradingWith()).getTeam()):
								self.addUserComment("USER_DIPLOCOMMENT_COMPLETE_DEAL", -1, -1)

					# Otherwise they have something on the table and we dont
					elif (self.diploScreen.theirOfferEmpty() == 0):

						# If we are at war, demand the items for peace or ask what they want
						if (self.diploScreen.atWar()):
							self.addUserComment("USER_DIPLOCOMMENT_PROPOSE", -1, -1)

						# Otherwise (during peacetime) ask what they want for our item or demand they give it to us
						else:
							if (gc.getGame().getActiveTeam() == gc.getPlayer(self.diploScreen.getWhoTradingWith()).getTeam()):
								self.addUserComment("USER_DIPLOCOMMENT_DEMAND_TEAM", -1, -1)

							else:
								self.addUserComment("USER_DIPLOCOMMENT_OFFER", -1, -1)

								if (gc.getPlayer(self.diploScreen.getWhoTradingWith()).AI_getAttitude(gc.getGame().getActivePlayer()) >= AttitudeTypes.ATTITUDE_PLEASED):
									self.addUserComment("USER_DIPLOCOMMENT_ASK", -1, -1)
								elif (gc.getTeam(gc.getGame().getActiveTeam()).canDeclareWar(gc.getPlayer(self.diploScreen.getWhoTradingWith()).getTeam())):
									self.addUserComment("USER_DIPLOCOMMENT_DEMAND", -1, -1)

					# Otherwise we have something on the table and they dont
					else:

						# If we are at war, use this item to fish for peace or propose peace with the items
						if (self.diploScreen.atWar()):
							self.addUserComment("USER_DIPLOCOMMENT_PROPOSE", -1, -1)

						# During peace, see what we can get for these items or simply gift them to the AI
						else:
							if (gc.getGame().getActiveTeam() != gc.getPlayer(self.diploScreen.getWhoTradingWith()).getTeam()):
								self.addUserComment("USER_DIPLOCOMMENT_FISH_FOR_DEAL", -1, -1)

							self.addUserComment("USER_DIPLOCOMMENT_GIFT", -1, -1)

			# Exit option
			self.addUserComment("USER_DIPLOCOMMENT_NEVERMIND", -1, -1)
			self.addUserComment("USER_DIPLOCOMMENT_EXIT", -1, -1)
			self.diploScreen.startTrade( eComment, false )

		elif (self.isComment(eComment, "AI_DIPLOCOMMENT_SOMETHING_ELSE")):
			if (gc.getTeam(gc.getGame().getActiveTeam()).canDeclareWar(gc.getPlayer(self.diploScreen.getWhoTradingWith()).getTeam())):
				if gc.getTeam(gc.getPlayer(self.diploScreen.getWhoTradingWith()).getTeam()).isParentOf(gc.getGame().getActiveTeam()):
					self.addUserComment("USER_DIPLOCOMMENT_REVOLUTION", -1, -1)
				else:
					self.addUserComment("USER_DIPLOCOMMENT_WAR", -1, -1)

			self.addUserComment("USER_DIPLOCOMMENT_ATTITUDE", -1, -1)
			if (gc.getTeam(gc.getGame().getActiveTeam()).AI_shareWar(gc.getPlayer(self.diploScreen.getWhoTradingWith()).getTeam())):
				self.addUserComment("USER_DIPLOCOMMENT_TARGET", -1, -1)

			self.addUserComment("USER_DIPLOCOMMENT_NEVERMIND", -1, -1)
			self.addUserComment("USER_DIPLOCOMMENT_EXIT", -1, -1)

		elif (self.isComment(eComment, "AI_DIPLOCOMMENT_ATTITUDE") or
					self.isComment(eComment, "AI_DIPLOCOMMENT_ATTITUDE_PLAYER_FURIOUS") or
					self.isComment(eComment, "AI_DIPLOCOMMENT_ATTITUDE_PLAYER_ANNOYED") or
					self.isComment(eComment, "AI_DIPLOCOMMENT_ATTITUDE_PLAYER_CAUTIOUS") or
					self.isComment(eComment, "AI_DIPLOCOMMENT_ATTITUDE_PLAYER_PLEASED") or
					self.isComment(eComment, "AI_DIPLOCOMMENT_ATTITUDE_PLAYER_FRIENDLY")):
			for i in range(gc.getMAX_CIV_PLAYERS()):
				if (gc.getPlayer(i).isAlive()):
					if ((i != gc.getGame().getActivePlayer()) and (i != self.diploScreen.getWhoTradingWith())):
						if (gc.getTeam(gc.getGame().getActiveTeam()).isHasMet(gc.getPlayer(i).getTeam()) and gc.getTeam(gc.getPlayer(self.diploScreen.getWhoTradingWith()).getTeam()).isHasMet(gc.getPlayer(i).getTeam())):
							self.addUserComment("USER_DIPLOCOMMENT_ATTITUDE_PLAYER", i, -1, gc.getPlayer(i).getNameKey())

			self.addUserComment("USER_DIPLOCOMMENT_SOMETHING_ELSE", -1, -1)
			self.addUserComment("USER_DIPLOCOMMENT_EXIT", -1, -1)

		elif (self.isComment(eComment, "AI_DIPLOCOMMENT_TARGET")):
			for i in range(gc.getMAX_CIV_PLAYERS()):
				if (gc.getPlayer(i).isAlive()):
					if (gc.getTeam(gc.getGame().getActiveTeam()).isAtWar(gc.getPlayer(i).getTeam())):
						player = gc.getPlayer(i)
						(loopCity, iter) = player.firstCity(false)
						while(loopCity):
							if (loopCity.isRevealed(gc.getGame().getActiveTeam(), false)):
								self.addUserComment("USER_DIPLOCOMMENT_TARGET_CITY", i, loopCity.getID(), loopCity.getNameKey())
							(loopCity, iter) = player.nextCity(iter, false)

			self.addUserComment("USER_DIPLOCOMMENT_SOMETHING_ELSE", -1, -1)
			self.addUserComment("USER_DIPLOCOMMENT_EXIT", -1, -1)

		# The default...
		else:

			if (gc.getPlayer(gc.getGame().getActivePlayer()).canTradeWith(self.diploScreen.getWhoTradingWith())):
				# Allow us to begin another proposal
				self.addUserComment("USER_DIPLOCOMMENT_PROPOSAL", -1, -1)

			if (self.isComment(eComment, "AI_DIPLOCOMMENT_GREETINGS") or
					self.isComment(eComment, "AI_DIPLOCOMMENT_WORST_ENEMY") or
					self.isComment(eComment, "AI_DIPLOCOMMENT_WORST_ENEMY_TRADING")):
				# If we are at war, allow to suggest peace
				if (self.diploScreen.atWar()):
					self.addUserComment("USER_DIPLOCOMMENT_SUGGEST_PEACE", -1, -1)

			# If we have a current deal, allow us to see the deals
			if (self.diploScreen.hasAnnualDeal()):
				self.addUserComment("USER_DIPLOCOMMENT_CURRENT_DEALS", -1, -1)

			if (gc.getTeam(gc.getPlayer(self.diploScreen.getWhoTradingWith()).getTeam())).isParentOf(gc.getGame().getActiveTeam()):
				if (gc.getTeam(gc.getGame().getActiveTeam()).canDeclareWar(gc.getPlayer(self.diploScreen.getWhoTradingWith()).getTeam())):
					self.addUserComment("USER_DIPLOCOMMENT_REVOLUTION", -1, -1)

				if (not gc.getTeam(gc.getGame().getActiveTeam()).isAtWar(gc.getPlayer(self.diploScreen.getWhoTradingWith()).getTeam())):
					self.addUserComment("USER_DIPLOCOMMENT_BUY_UNITS", -1, -1)
			else:
				self.addUserComment("USER_DIPLOCOMMENT_SOMETHING_ELSE", -1, -1)

			# Exit potential
			self.addUserComment("USER_DIPLOCOMMENT_EXIT", -1, -1)
			self.diploScreen.endTrade();

	def addUserComment(self, eComment, iData1, iData2, *args):
		" Helper for adding User Comments "
		iComment = self.getCommentID( eComment )
		self.diploScreen.addUserComment( iComment, iData1, iData2, self.getDiplomacyComment(iComment), args)

	def setAIComment (self, eComment, *args):
		" Handles the determining the AI comments"
		AIString = self.getDiplomacyComment(eComment)

		if DebugLogging:
			print "CvDiplomacy.setAIComment: %s" %(eComment,)
			if (len(args)):
				print "args", args
			AIString = "(%d) - %s" %(self.getLastResponseID(), AIString)

		self.diploScreen.setAIString(AIString, args)
		self.diploScreen.setAIComment(eComment)
		self.determineResponses(eComment)
		self.performHeadAction(eComment)

	def performHeadAction( self, eComment ):

		if ( eComment == self.getCommentID("AI_DIPLOCOMMENT_NO_PEACE") or
		     eComment == self.getCommentID("AI_DIPLOCOMMENT_REJECT") or
		     eComment == self.getCommentID("AI_DIPLOCOMMENT_NO_DEAL") or
		     eComment == self.getCommentID("AI_DIPLOCOMMENT_CANCEL_DEAL") or
		     eComment == self.getCommentID("AI_DIPLOCOMMENT_REJECT_ASK") or
		     eComment == self.getCommentID("AI_DIPLOCOMMENT_REJECT_DEMAND") ):
			self.diploScreen.performHeadAction( LeaderheadAction.LEADERANIM_DISAGREE )
		elif ( eComment == self.getCommentID("AI_DIPLOCOMMENT_ACCEPT") or
		       eComment == self.getCommentID("AI_DIPLOCOMMENT_TRY_THIS_DEAL") ):
			self.diploScreen.performHeadAction( LeaderheadAction.LEADERANIM_AGREE )
		elif ( eComment == self.getCommentID("AI_DIPLOCOMMENT_FIRST_CONTACT") or
		       eComment == self.getCommentID("AI_DIPLOCOMMENT_GREETINGS") or
		       eComment == self.getCommentID("AI_DIPLOCOMMENT_WORST_ENEMY") or
		       eComment == self.getCommentID("AI_DIPLOCOMMENT_OFFER_PEACE") or
		       eComment == self.getCommentID("AI_DIPLOCOMMENT_OFFER_CITY") or
		       eComment == self.getCommentID("AI_DIPLOCOMMENT_OFFER_DEAL") or
		       eComment == self.getCommentID("AI_DIPLOCOMMENT_GIVE_HELP") or
		       eComment == self.getCommentID("AI_DIPLOCOMMENT_JOIN_WAR") or
		       eComment == self.getCommentID("AI_DIPLOCOMMENT_STOP_TRADING") or
		       eComment == self.getCommentID("AI_DIPLOCOMMENT_ASK_FOR_HELP") or
		       eComment == self.getCommentID("AI_DIPLOCOMMENT_DEMAND_TRIBUTE") ):
			self.diploScreen.performHeadAction( LeaderheadAction.LEADERANIM_GREETING )
		elif ( eComment == self.getCommentID("AI_DIPLOCOMMENT_ATTITUDE_PLAYER_FURIOUS") or
		       eComment == self.getCommentID("AI_DIPLOCOMMENT_WORST_ENEMY_TRADING") or
		       eComment == self.getCommentID("AI_DIPLOCOMMENT_HELP_REFUSED") or
		       eComment == self.getCommentID("AI_DIPLOCOMMENT_DEMAND_REJECTED") or
		       eComment == self.getCommentID("AI_DIPLOCOMMENT_JOIN_DENIED") or
		       eComment == self.getCommentID("AI_DIPLOCOMMENT_STOP_DENIED") ):
			self.diploScreen.performHeadAction( LeaderheadAction.LEADERANIM_FURIOUS )
		elif ( eComment == self.getCommentID("AI_DIPLOCOMMENT_ATTITUDE_PLAYER_ANNOYED") ):
			self.diploScreen.performHeadAction( LeaderheadAction.LEADERANIM_ANNOYED )
		elif ( eComment == self.getCommentID("AI_DIPLOCOMMENT_ATTITUDE_PLAYER_CAUTIOUS") ):
			self.diploScreen.performHeadAction( LeaderheadAction.LEADERANIM_CAUTIOUS )
		elif ( eComment == self.getCommentID("AI_DIPLOCOMMENT_ATTITUDE_PLAYER_PLEASED") or
		       eComment == self.getCommentID("AI_DIPLOCOMMENT_SORRY") ):
			self.diploScreen.performHeadAction( LeaderheadAction.LEADERANIM_PLEASED )
		elif ( eComment == self.getCommentID("AI_DIPLOCOMMENT_ATTITUDE_PLAYER_FRIENDLY") or
		       eComment == self.getCommentID("AI_DIPLOCOMMENT_GLAD") or
		       eComment == self.getCommentID("AI_DIPLOCOMMENT_THANKS") ):
			self.diploScreen.performHeadAction( LeaderheadAction.LEADERANIM_FRIENDLY )
		elif ( eComment == self.getCommentID("AI_DIPLOCOMMENT_KISS_PINKY") or
			   eComment == self.getCommentID("AI_DIPLOCOMMENT_KING_ASK_FOR_GOLD") or
		       eComment == self.getCommentID("AI_DIPLOCOMMENT_BUY_UNITS") or
		       eComment == self.getCommentID("AI_DIPLOCOMMENT_KING_GIFT_SHIP") or
		       eComment == self.getCommentID("AI_DIPLOCOMMENT_KING_REVIVE")):
			self.diploScreen.performHeadAction( LeaderheadAction.LEADERANIM_OFFER_PINKY )
		elif ( eComment == self.getCommentID("AI_DIPLOCOMMENT_ACCEPT_PINKY") or 
		       eComment == self.getCommentID("AI_DIPLOCOMMENT_KING_ACCEPT_GOLD")):
			self.diploScreen.performHeadAction( LeaderheadAction.LEADERANIM_PINKY_ACCEPT )
			CyInterface().playGeneralSound("AS2D_KISS_MY_RING")
		elif ( eComment == self.getCommentID("AI_DIPLOCOMMENT_REJECT_PINKY") or
		       eComment == self.getCommentID("AI_DIPLOCOMMENT_KING_REFUSE_GOLD")):
			self.diploScreen.performHeadAction( LeaderheadAction.LEADERANIM_PINKY_REJECT )

		return

	def getDiplomacyComment (self, eComment):
		"Function to get the user String"
		debugString = "CvDiplomacy.getDiplomacyComment: %s" %(eComment,)
		eComment = int(eComment)
		if DebugLogging:
			print debugString, eComment

		szString = ""
		szFailString = "Error***: No string found for eComment: %s"

		if ( gc.getDiplomacyInfo(eComment) ):
			DiplomacyTextInfo = gc.getDiplomacyInfo(eComment)
			if ( not DiplomacyTextInfo ):
				print "%s IS AN INVALID DIPLOCOMMENT" %(eComment,)
				CvUtil.pyAssert(True, "CvDiplomacy.getDiplomacyComment: %s does not have a DiplomacyTextInfo" %(eComment,))
				return szFailString %(eComment,)

			szString = self.filterUserResponse(DiplomacyTextInfo)

		else:
			szString = szFailString %(eComment,)

		return szString

	def setLastResponseID(self, iResponse):
		self.iLastResponseID = iResponse

	def getLastResponseID(self):
		return self.iLastResponseID

	def isUsed(self, var, i, num):
		"returns true if any element in the var list is true"
		for j in range(num):
			if (var(i, j)):
				return true
		return false

	def filterUserResponse(self, diploInfo):
		"pick the user's response from a CvDiplomacyTextInfo, based on response conditions"
		if (self.diploScreen.getWhoTradingWith() == -1):
			return ""

		theirPlayer = gc.getPlayer(self.diploScreen.getWhoTradingWith())
		ourPlayer = gc.getActivePlayer()
		responses = []

		for i in range(diploInfo.getNumResponses()):

			# check attitude of other player towards me
			if (self.isUsed(diploInfo.getAttitudeTypes, i, AttitudeTypes.NUM_ATTITUDE_TYPES)):
				att = theirPlayer.AI_getAttitude(CyGame().getActivePlayer())
				if (not diploInfo.getAttitudeTypes(i, att)):
					continue

			# check civ type
			if (self.isUsed(diploInfo.getCivilizationTypes, i, gc.getNumCivilizationInfos()) and
				not diploInfo.getCivilizationTypes(i, theirPlayer.getCivilizationType())):
				continue

			# check leader type
			if (self.isUsed(diploInfo.getLeaderHeadTypes, i, gc.getNumLeaderHeadInfos()) and
				not diploInfo.getLeaderHeadTypes(i, theirPlayer.getLeaderType())):
				continue

			# check power type
			if (self.isUsed(diploInfo.getDiplomacyPowerTypes, i, DiplomacyPowerTypes.NUM_DIPLOMACYPOWER_TYPES)):
				theirPower = theirPlayer.getPower()
				ourPower = ourPlayer.getPower()

				if (ourPower < (theirPower / 2)):
					if not diploInfo.getDiplomacyPowerTypes(i, DiplomacyPowerTypes.DIPLOMACYPOWER_STRONGER):
						continue

				elif (ourPower > (theirPower * 2)):
					if not diploInfo.getDiplomacyPowerTypes(i, DiplomacyPowerTypes.DIPLOMACYPOWER_WEAKER):
						continue

				else:
					if not diploInfo.getDiplomacyPowerTypes(i, DiplomacyPowerTypes.DIPLOMACYPOWER_EQUAL):
						continue

			# passed all tests, so add to response list
			for j in range(diploInfo.getNumDiplomacyText(i)):
				responses.append(diploInfo.getDiplomacyText(i, j))

		# pick a random response
		numResponses = len(responses)
		if (numResponses>0):
			iResponse = gc.getASyncRand().get(numResponses, "Python Diplomacy ASYNC")
			self.setLastResponseID(iResponse)
			return responses[iResponse]

		return ""	# no responses matched

	def handleUserResponse(self, eComment, iData1, iData2):
		if DebugLogging:
			print "CvDiplomacy.handleUserResponse: %s" %(eComment,)

		diploScreen = CyDiplomacy()

		# If we accept peace
		if (self.isComment(eComment, "USER_DIPLOCOMMENT_PEACE")):
			self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_PEACE"))

		# If we choose war
		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_WAR") or self.isComment(eComment, "USER_DIPLOCOMMENT_REVOLUTION")):
			diploScreen.declareWar()

		# If we are asking for units
		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_BUY_UNITS")):
			args = diploScreen.militaryHelp()
			iNumUnits = args[0]
			eUnit = args[1]
			eProfession = args[2]
			iPrice = args[3]
			if (iNumUnits > 0 and iPrice <= gc.getPlayer(gc.getGame().getActivePlayer()).getGold()):
				if eProfession == -1:
					szName = gc.getUnitInfo(eUnit).getTextKey()
				else:
					szName = gc.getProfessionInfo(eProfession).getTextKey()
				self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_BUY_UNITS"), iPrice, szName)
			else:
				self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_REJECT_BUY_UNIS"))

		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_ACCEPT_BUY_UNITS")):
			CyInterface().playGeneralSound("AS2D_KISS_MY_RING")
			diploScreen.buyUnits()
			self.closeDiploScreen()

		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_KING_GIFT_SHIP")):
			CyInterface().playGeneralSound("AS2D_KISS_MY_RING")
			self.closeDiploScreen()

		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_KING_REVIVE")):
			CyInterface().playGeneralSound("AS2D_KISS_MY_RING")
			self.closeDiploScreen()

		# If we wish to make a trade proposal or try to renegotiate
		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_PROPOSAL") or
					self.isComment(eComment, "USER_DIPLOCOMMENT_RENEGOTIATE")):
			self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_TRADING"))
			diploScreen.showAllTrade(True)

		# If we want to propose a trade
		elif(self.isComment(eComment, "USER_DIPLOCOMMENT_PROPOSE")):
			if (diploScreen.offerDeal() == 1):
				self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_ACCEPT"))
			else:
				self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_REJECT"))

		# If we ask for peace
		elif(self.isComment(eComment, "USER_DIPLOCOMMENT_SUGGEST_PEACE")):
			if (diploScreen.offerDeal() == 1):
				self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_PEACE"))
			else:
				self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_NO_PEACE"))

		# If we accept a trade
		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_ACCEPT")):
			diploScreen.implementDeal()
			diploScreen.setAIOffer(0)
			self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_GLAD"))

		# If we reject a trade
		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_REJECT")):
			diploScreen.setAIOffer(0)
			self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_SORRY"))

		# If we offer a deal, or is we are fishing for a deal, or if we are offering peace or fishing for peace
		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_OFFER") or
					self.isComment(eComment, "USER_DIPLOCOMMENT_COMPLETE_DEAL") or
					self.isComment(eComment, "USER_DIPLOCOMMENT_FISH_FOR_DEAL") or
					self.isComment(eComment, "USER_DIPLOCOMMENT_OFFER_PEACE")):
			if (diploScreen.counterPropose() == 1):
				self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_TRY_THIS_DEAL"))
			else:
				self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_NO_DEAL"))

		# if we are asking for something
		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_ASK")):
			diploScreen.diploEvent(DiploEventTypes.DIPLOEVENT_ASK_HELP, -1, -1)
			if (diploScreen.offerDeal()):
				self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_ACCEPT_ASK"))
			else:
				self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_REJECT_ASK"))

		# if we are demanding something
		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_DEMAND")):
			diploScreen.diploEvent(DiploEventTypes.DIPLOEVENT_MADE_DEMAND, -1, -1)
			if (diploScreen.offerDeal()):
				self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_ACCEPT_DEMAND"))
			else:
				self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_REJECT_DEMAND"))

		# if we are demanding something from our teammate
		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_DEMAND_TEAM")):
			diploScreen.offerDeal()
			self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_ACCEPT_DEMAND_TEAM"))

		# If we are giving a gift
		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_GIFT")):
			diploScreen.offerDeal()
			self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_THANKS"))

		# If we decide to view current deals
		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_CURRENT_DEALS")):
			self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_CURRENT_DEALS"))
			diploScreen.showAllTrade(False)

		# If we give help
		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_GIVE_HELP")):
			diploScreen.diploEvent(DiploEventTypes.DIPLOEVENT_GIVE_HELP, -1, -1)
			diploScreen.implementDeal()
			diploScreen.setAIOffer(0)
			self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_THANKS"))

		# If we accept their demand
		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_ACCEPT_DEMAND")):
			diploScreen.diploEvent(DiploEventTypes.DIPLOEVENT_ACCEPT_DEMAND, -1, -1)
			diploScreen.implementDeal()
			diploScreen.setAIOffer(0)
			self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_THANKS"))

		# If we accept the offer
		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_ACCEPT_OFFER")):
			diploScreen.implementDeal()
			diploScreen.setAIOffer(0)
			self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_THANKS"))

		# If we refuse to help
		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_REFUSE_HELP")):
			diploScreen.diploEvent(DiploEventTypes.DIPLOEVENT_REFUSED_HELP, -1, -1)
			self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_HELP_REFUSED"))

		# If we reject their demand
		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_REJECT_DEMAND")):
			diploScreen.diploEvent(DiploEventTypes.DIPLOEVENT_REJECTED_DEMAND, -1, -1)

			if (gc.getPlayer(self.diploScreen.getWhoTradingWith()).AI_demandRebukedWar(gc.getGame().getActivePlayer())):
				self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_DECLARE_WAR"))
				diploScreen.diploEvent(DiploEventTypes.DIPLOEVENT_DEMAND_WAR, -1, -1)
			else:
				self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_DEMAND_REJECTED"))

		# If we join their war
		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_JOIN_WAR")):
			diploScreen.diploEvent(DiploEventTypes.DIPLOEVENT_JOIN_WAR, diploScreen.getData(), -1)
			self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_THANKS"))

		# If we refuse to join their war
		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_NO_JOIN_WAR")):
			diploScreen.diploEvent(DiploEventTypes.DIPLOEVENT_NO_JOIN_WAR, -1, -1)
			self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_JOIN_DENIED"))

		# If we stop the trading
		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_STOP_TRADING")):
			diploScreen.diploEvent(DiploEventTypes.DIPLOEVENT_STOP_TRADING, diploScreen.getData(), -1)
			self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_THANKS"))

		# If we refuse to stop the trading
		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_NO_STOP_TRADING")):
			diploScreen.diploEvent(DiploEventTypes.DIPLOEVENT_NO_STOP_TRADING, -1, -1)
			self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_STOP_DENIED"))

		# If we want to go back to first screen
		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_NEVERMIND")):
			self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_WELL"))

		# If we want to discuss something else
		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_SOMETHING_ELSE")):
			self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_SOMETHING_ELSE"))

		# If we want to ask them to what their attitude is
		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_ATTITUDE")):
			self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_ATTITUDE"))

		# If we want to ask them to what their attitude is on a specific player
		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_ATTITUDE_PLAYER")):
			eAttitude = gc.getPlayer(self.diploScreen.getWhoTradingWith()).AI_getAttitude(iData1)

			if (eAttitude == AttitudeTypes.ATTITUDE_FURIOUS):
				self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_ATTITUDE_PLAYER_FURIOUS"), gc.getPlayer(iData1).getNameKey())
			elif (eAttitude == AttitudeTypes.ATTITUDE_ANNOYED):
				self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_ATTITUDE_PLAYER_ANNOYED"), gc.getPlayer(iData1).getNameKey())
			elif (eAttitude == AttitudeTypes.ATTITUDE_CAUTIOUS):
				self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_ATTITUDE_PLAYER_CAUTIOUS"), gc.getPlayer(iData1).getNameKey())
			elif (eAttitude == AttitudeTypes.ATTITUDE_PLEASED):
				self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_ATTITUDE_PLAYER_PLEASED"), gc.getPlayer(iData1).getNameKey())
			else:
				self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_ATTITUDE_PLAYER_FRIENDLY"), gc.getPlayer(iData1).getNameKey())

		# If we want to ask them to change their target
		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_TARGET")):
			self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_TARGET"))

		# If we want to ask them to change their target to a specific city
		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_TARGET_CITY")):
			diploScreen.diploEvent(DiploEventTypes.DIPLOEVENT_TARGET_CITY, iData1, iData2)
			self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_TARGET_CITY"))

		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_KISS_PINKY")):
			self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_ACCEPT_PINKY"), gc.getPlayer(gc.getGame().getActivePlayer()).getTaxRate() + diploScreen.getData())

		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_ACCEPT_PINKY")):
			diploScreen.diploEvent(DiploEventTypes.DIPLOEVENT_ACCEPT_TAX_RATE, diploScreen.getData(), -1)
			self.closeDiploScreen()

		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_TAX_PARTY")):
			self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_REJECT_PINKY"), gc.getYieldInfo(iData1).getTextKey())

		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_REJECT_PINKY")):
			diploScreen.diploEvent(DiploEventTypes.DIPLOEVENT_REFUSE_TAX_RATE, iData1, iData2)
			self.closeDiploScreen()

		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_KING_GIVE_GOLD")):
			self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_KING_ACCEPT_GOLD"))

		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_KING_ACCEPTED_GOLD")):
			diploScreen.diploEvent(DiploEventTypes.DIPLOEVENT_ACCEPT_KING_GOLD, diploScreen.getData(), -1)
			self.closeDiploScreen()

		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_KING_REFUSE_GOLD")):
			self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_KING_REFUSE_GOLD"))

		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_KING_REFUSED_GOLD")):
			diploScreen.diploEvent(DiploEventTypes.DIPLOEVENT_REFUSE_KING_GOLD, diploScreen.getData(), -1)
			self.closeDiploScreen()

		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_LIVE_AMONG_NATIVES_ACCEPT")):
			diploScreen.diploEvent(DiploEventTypes.DIPLOEVENT_LIVE_AMONG_NATIVES, diploScreen.getData(), -1)
			self.closeDiploScreen()

		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_TREASURE_TRANSPORT_ACCEPT")):
			diploScreen.diploEvent(DiploEventTypes.DIPLOEVENT_TRANSPORT_TREASURE, diploScreen.getData(), -1)
			self.closeDiploScreen()

		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_FOUND_CITY_NO_FOOD_YES")):
			diploScreen.diploEvent(DiploEventTypes.DIPLOEVENT_FOUND_CITY_CHECK_NATIVES, diploScreen.getData(), -1)
			self.closeDiploScreen()

		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_FOUND_CITY_INLAND_YES")):
			diploScreen.diploEvent(DiploEventTypes.DIPLOEVENT_FOUND_CITY_CHECK_NATIVES, diploScreen.getData(), -1)
			self.closeDiploScreen()

		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_FOUND_FIRST_CITY")):
			diploScreen.diploEvent(DiploEventTypes.DIPLOEVENT_FOUND_CITY, diploScreen.getData(), false)
			self.closeDiploScreen()

		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_FOUND_CITY_BUY")):
			diploScreen.diploEvent(DiploEventTypes.DIPLOEVENT_FOUND_CITY, diploScreen.getData(), true)
			self.closeDiploScreen()

		elif (self.isComment(eComment, "USER_DIPLOCOMMENT_FOUND_CITY_TAKE")):
			diploScreen.diploEvent(DiploEventTypes.DIPLOEVENT_FOUND_CITY, diploScreen.getData(), false)
			self.closeDiploScreen()

		else:
			self.closeDiploScreen()

	def dealCanceled( self ):

		self.setAIComment(self.getCommentID("AI_DIPLOCOMMENT_TRADING"))

		return

	def isComment(self, eComment, strComment):
		'bool - comment matching'
		if ( gc.getDiplomacyInfo(eComment).getType() == strComment ):
			return True
		return False

	def getCommentID(self, strComment):
		'int - ID for DiploCommentType'
		for i in range(gc.getNumDiplomacyInfos()):
			if ( gc.getDiplomacyInfo(i).getType() == strComment ):
				return i

		print "Jason " + strComment
		return -1

# PatchMod: Stop F1 pressing during diplomacy START
	def closeDiploScreen(self):
		diploScreen = CyDiplomacy()
		CyGame().inDiplomacy(false)
		diploScreen.closeScreen()
# PatchMod: Stop F1 pressing during diplomacy END
