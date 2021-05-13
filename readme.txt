Change history of Sid Meier's Civilization 4, its expansions and official and unofficial patches

Created by extracting archives from installers and committing the files in chronological order. In a private repository, I've done this with all extracted files, including artwork and official mods, at a total size of about 2.5 GB. This public repository is a trimmed down version of that, tracking only text files. I'm making an exception for the GameCore DLL binary because the source code of that DLL is unavailable until patch 1.61 (and also unavailable for Solver's unofficial patch). Putting the DLL under source control at least shows whether or not it has been changed. Text files that I'm omitting:
• License files (terms of use) that are shown by the installers;
• Assets\Python\System folders – because they're expansive, don't change much and the copyright is murky;
• source code for dependencies, namely Boost 1.32 and Python 2.4;
• mods bundled with the original game. (I don't think those change histories would be of much interest to anyone.)

Colonization and the Civ 4 demo are included but not part of the main branch.
