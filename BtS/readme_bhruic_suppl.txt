Installation Instructions
_________________________

	This supplementary patch addresses bugs/changes that don't need dll modification.  Generally that means XML or PY files.  I do 
not recommend overwriting your original files, although you can choose to do that (backup first!).  Rather, the best way to handle them
is to place them in your CustomAssets folder.

My Documents\My Games\Beyond the Sword\CustomAssets\python\screens\CvExoticForeignAdvisor.py
My Documents\My Games\Beyond the Sword\CustomAssets\python\screens\CvWorldBuilderDiplomacyScreen.py
My Documents\My Games\Beyond the Sword\CustomAssets\python\screens\CvPediaPromotion.py
My Documents\My Games\Beyond the Sword\CustomAssets\python\CvEventManager.py

ChangeLog
_________

- Reactivated the Glance diplomatic screen
- Fixed a bug preventing war declarations via Worldbuilder
- Civilopedia promotions no longer display a unit class if only a subsection of that class can get the promotion
- Partisans only appear when Civ is running Emancipation