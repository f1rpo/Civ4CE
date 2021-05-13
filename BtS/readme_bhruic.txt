Installation Instructions
_________________________

	This dll replaces the one located at "C:\Program Files\Firaxis Games\Sid Meier's Civilization 4\Beyond the Sword\Assets\CvGameCoreDLL.dll"
It is recommended to backup your current dll file (renaming it will work fine) before installing this version.  Simply copy the new
version into the same folder, making sure the game isn't currently running.  After that, simply play the game!

	The source code for the changes is also included in a separate folder (SDK Source).  If you are not a modder, you do not need those
files.  There is no need to extract them from the zip file.

Changelog
_________

- Fixed bug where culture/espionage would not display for building popups
- Fixed bug where spies would be ejected from a square when declaring war
- Fixed bug that caused the AI to trade for resources it already had (appears as grossly uneven trades)
- Refixed trade bug
- Refixed culture/espionage display bug
- Fixed bug with Warlords attaching to units when the Great General is part of a selection group
- Prevented new Colonies from overwriting previously existing Civs
- No longer displays new Colony message if player hasn't met Master Civ
- Workers on Transports no longer perform capture of cities (thereby displacing all other units) when city is captured
- Units considered "unsuitable" for city defense will no longer be prevented from healing in cities
- No longer able to base an unlimited number of air units in vassal's cities
- Can found Corporations under Mercantilism
- Vassal's spies will no longer be "caught" in your territory
- Amount of time worked taken into consideration by city plot selection (thanks SevenSpirits)
- The Vassal of a Capitulating Civ is freed before peace treaty
- Mouse-over for Worker actively working a plot no longer counts Worker twice
- Automated Workers will no longer idle when railroading is possible
- Mouse-over for Join City now displays extra bonuses for Great People
- Vassals freed by Capitulating Master no longer refuse to talk
- Privateers can only enter team members' or vassals' cities
- Automated Workers will no longer idle in cities within 2 squares of a hostile border
- Refixed Vassal/capitulation
- AIs less likely to build Executives if they can't afford to use them or already have sufficient
- Buildings that grant both heathiness and unhealthiness will display both instead of neither in building popups
- Vassals can no longer be their Master's worst enemy
- Espionage ratio for humans now displayed in score tooltip (thanks morbus)
- Improved worker threat assessment from units able to move multiple squares
- Spies no longer interrupt their mission when moving next to an enemy unit
- Fixed AI turn slowdown caused by change to worker threat assessment
- Relationship penalty for declaring war on a friend checks if Civs are at war
- Altered previous worker threat assessment check so it's less useful, but more efficient
- Fixed bug (introduced) causing endless loop at "waiting for players"
- Fixed bug (introduced) causing the Great Persons civilopedia entries to be blank
- Stolen techs can no longer be traded with No Tech Brokering enabled
- New Colonies mirror their Master's tech status with No Tech Brokering enabled
- New Colonies mirror their Master's exploration (map) status
- Healthy/Unhealthy bonuses will display properly under more circumstances (events, etc)
- Extended spy ejection protection to Great Spies (and any other unit considered "always invisible")
- AI Civs under Theocracy will no longer accept gifted Missionaries
- Units withdrawing from combat will be removed from selection group (thanks Moctezuma)
- When bumping naval units from cities, preference will be given to water tiles
- Tweaks/bugfixes to improve starting city locations (thanks SevenSpirits)
- Cities no longer require a Monument to allow Spies to sabotage buildings
- Rivers added for starting locations will generate flood plains if they pass through desert tiles
- Spy unit help popup will display the percentage espionage cost decrease from fortification