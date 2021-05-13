### Variables used in this Makefile
TOOLKIT=C:\Program Files\Microsoft Visual C++ Toolkit 2003
PSDK=C:\Program Files\Microsoft Platform SDK for Windows Server 2003 R2
# For most, these will be in the current directory
BOOSTDIR=../Common/Boost-1.32.0
PYTHONDIR=../Common/Python24
RM=del
Debug_CC="$(TOOLKIT)/bin/cl.exe"
Debug_CPP="$(TOOLKIT)/bin/cl.exe"
Debug_LD="$(TOOLKIT)/bin/link.exe"
Debug_LIB="$(TOOLKIT)/bin/link.exe"
Debug_RESCOMP=rc.exe

Final_Release_CC="$(TOOLKIT)/bin/cl.exe"
Final_Release_CPP="$(TOOLKIT)/bin/cl.exe"
Final_Release_LD="$(TOOLKIT)/bin/link.exe"
Final_Release_LIB="$(TOOLKIT)/bin/link.exe"
Final_Release_RESCOMP=rc.exe

### Compiler/linker options
Debug_GLOBAL_CFLAGS=
Debug_PROJECT_CFLAGS=
Debug_GLOBAL_LDFLAGS=
Debug_PROJECT_LDFLAGS=
Debug_GLOBAL_INCS= /I"$(TOOLKIT)/include" /I"$(PSDK)/Include"
Debug_PROJECT_INCS=
Debug_GLOBAL_LIBDIRS= /LIBPATH:"$(TOOLKIT)/lib" /LIBPATH:"$(PSDK)/Lib"
Debug_PROJECT_LIBDIRS=
Debug_GLOBAL_LIBS=
Debug_PROJECT_LIBS=

Final_Release_GLOBAL_CFLAGS=
Final_Release_PROJECT_CFLAGS=
Final_Release_GLOBAL_LDFLAGS=
Final_Release_PROJECT_LDFLAGS=
Final_Release_GLOBAL_INCS= /I"$(TOOLKIT)/include" /I"$(PSDK)/Include"
Final_Release_PROJECT_INCS=
Final_Release_GLOBAL_LIBDIRS= /LIBPATH:"$(TOOLKIT)/lib" /LIBPATH:"$(PSDK)/Lib"
Final_Release_PROJECT_LIBDIRS=
Final_Release_GLOBAL_LIBS=
Final_Release_PROJECT_LIBS=

### Targets compiler flags
Debug_CFLAGS= $(Debug_PROJECT_CFLAGS) /MD /GR /Od /W3 /EHsc /DWIN32 /D_DEBUG /D_WINDOWS /D_USRDLL /DCVGAMECOREDLL_EXPORTS /nologo /Zi /Gd /TP $(Debug_GLOBAL_CFLAGS)
Debug_SAFE_CFLAGS= $(Debug_PROJECT_CFLAGS) /MD /GR /Od /W3 /EHsc /DWIN32 /D_DEBUG /D_WINDOWS /D_USRDLL /DCVGAMECOREDLL_EXPORTS /nologo /Gd /TP $(Debug_GLOBAL_CFLAGS)
Final_Release_CFLAGS= $(Final_Release_PROJECT_CFLAGS) /MD /Gd /G7 /GR /O2 /W3 /EHsc /DWIN32 /DNDEBUG /D_WINDOWS /D_USRDLL /DCVGAMECOREDLL_EXPORTS /DFINAL_RELEASE $(Final_Release_GLOBAL_CFLAGS)

### Targets linker flags
Debug_LDFLAGS= $(Debug_PROJECT_LDFLAGS) /debug /NODEFAULTLIB:msvcprtd.lib /pdb:Debug\CvGameCoreDLL.pdb /INCREMENTAL /SUBSYSTEM:WINDOWS $(Debug_GLOBAL_LDFLAGS)
Final_Release_LDFLAGS= $(Final_Release_PROJECT_LDFLAGS) /debug /INCREMENTAL:NO /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF $(Final_Release_GLOBAL_LDFLAGS)

### Targets include directories
Debug_INCS= $(Debug_PROJECT_INCS) /I"$(BOOSTDIR)/include" /I"$(PYTHONDIR)/include" $(Debug_GLOBAL_INCS)
Final_Release_INCS= $(Final_Release_PROJECT_INCS) /I"$(BOOSTDIR)/include" /I"$(PYTHONDIR)/include" $(Final_Release_GLOBAL_INCS)

### Targets library directories
Debug_LIBDIRS= $(Debug_PROJECT_LIBDIRS) /LIBPATH:"$(PYTHONDIR)/libs" /LIBPATH:"$(BOOSTDIR)/libs/" $(Debug_GLOBAL_LIBDIRS)
Final_Release_LIBDIRS= $(Final_Release_PROJECT_LIBDIRS) /LIBPATH:"$(PYTHONDIR)/libs" /LIBPATH:"$(BOOSTDIR)/libs/" $(Final_Release_GLOBAL_LIBDIRS)

### Targets libraries
Debug_LIBS= $(Debug_PROJECT_LIBS) msvcprt.lib boost_python-vc71-mt-1_32.lib winmm.lib user32.lib $(Debug_GLOBAL_LIBS)
Final_Release_LIBS= $(Final_Release_PROJECT_LIBS) boost_python-vc71-mt-1_32.lib winmm.lib user32.lib $(Final_Release_GLOBAL_LIBS)

###############################################################################
#         You shouldn't need to modify anything beyond this point             #
###############################################################################

### Resources used in this Makefile
Debug_RESOURCE=
Final_Release_RESOURCE=

### Objects used in this Makefile
Debug_OBJS=Debug\CvArea.obj Debug\CvArtFileMgr.obj Debug\CvCity.obj Debug\CvCityAI.obj Debug\CvDLLButtonPopup.obj Debug\CvDLLEntity.obj Debug\CvDLLPython.obj Debug\CvDLLWidgetData.obj Debug\CvDeal.obj Debug\CvDiploParameters.obj Debug\CvFractal.obj Debug\CvGame.obj Debug\CvGameAI.obj Debug\CvGameCoreDLL.obj Debug\CvGameCoreUtils.obj Debug\CvGameInterface.obj Debug\CvGameTextMgr.obj Debug\CvGlobals.obj Debug\CvHallOfFameInfo.obj Debug\CvInfoWater.obj Debug\CvInfos.obj Debug\CvInitCore.obj Debug\CvMap.obj Debug\CvMapGenerator.obj Debug\CvPlayer.obj Debug\CvPlayerAI.obj Debug\CvPlot.obj Debug\CvPlotGroup.obj Debug\CvPopupInfo.obj Debug\CvPopupReturn.obj Debug\CvRandom.obj Debug\CvReplayInfo.obj Debug\CvReplayMessage.obj Debug\CvSelectionGroup.obj Debug\CvSelectionGroupAI.obj Debug\CvStructs.obj Debug\CvTalkingHeadMessage.obj Debug\CvTeam.obj Debug\CvTeamAI.obj Debug\CvUnit.obj Debug\CvUnitAI.obj Debug\CvXMLLoadUtility.obj Debug\CvXMLLoadUtilityGet.obj Debug\CvXMLLoadUtilityInit.obj Debug\CvXMLLoadUtilitySet.obj Debug\CyArea.obj Debug\CyAreaInterface.obj Debug\CyArgsList.obj Debug\CyArtFileMgr.obj Debug\CyArtFileMgrInterface.obj Debug\CyCity.obj Debug\CyCityInterface1.obj Debug\CyDeal.obj Debug\CyEnumsInterface.obj Debug\CyGame.obj Debug\CyGameCoreUtils.obj Debug\CyGameCoreUtilsInterface.obj Debug\CyGameInterface.obj Debug\CyGameTextMgr.obj Debug\CyGameTextMgrInterface.obj Debug\CyGlobalContext.obj Debug\CyGlobalContextInterface1.obj Debug\CyGlobalContextInterface2.obj Debug\CyGlobalContextInterface3.obj Debug\CyGlobalContextInterface4.obj Debug\CyHallOfFameInfo.obj Debug\CyHallOfFameInterface.obj Debug\CyInfoInterface1.obj Debug\CyInfoInterface2.obj Debug\CyInfoInterface3.obj Debug\CyMap.obj Debug\CyMapGenerator.obj Debug\CyMapGeneratorInterface.obj Debug\CyMapInterface.obj Debug\CyPlayer.obj Debug\CyPlayerInterface1.obj Debug\CyPlayerInterface2.obj Debug\CyPlot.obj Debug\CyPlotInterface1.obj Debug\CyRandomInterface.obj Debug\CyReplayInfo.obj Debug\CySelectionGroup.obj Debug\CySelectionGroupInterface.obj Debug\CyStructsInterface1.obj Debug\CyTeam.obj Debug\CyTeamInterface.obj Debug\CyUnit.obj Debug\CyUnitInterface1.obj Debug\FAssert.obj Debug\FDialogTemplate.obj Debug\_precompile.obj
Debug_LINKOBJS=$(Debug_OBJS)

Final_Release_OBJS=Final_Release\CvArea.obj Final_Release\CvArtFileMgr.obj Final_Release\CvCity.obj Final_Release\CvCityAI.obj Final_Release\CvDLLButtonPopup.obj Final_Release\CvDLLEntity.obj Final_Release\CvDLLPython.obj Final_Release\CvDLLWidgetData.obj Final_Release\CvDeal.obj Final_Release\CvDiploParameters.obj Final_Release\CvFractal.obj Final_Release\CvGame.obj Final_Release\CvGameAI.obj Final_Release\CvGameCoreDLL.obj Final_Release\CvGameCoreUtils.obj Final_Release\CvGameInterface.obj Final_Release\CvGameTextMgr.obj Final_Release\CvGlobals.obj Final_Release\CvHallOfFameInfo.obj Final_Release\CvInfoWater.obj Final_Release\CvInfos.obj Final_Release\CvInitCore.obj Final_Release\CvMap.obj Final_Release\CvMapGenerator.obj Final_Release\CvPlayer.obj Final_Release\CvPlayerAI.obj Final_Release\CvPlot.obj Final_Release\CvPlotGroup.obj Final_Release\CvPopupInfo.obj Final_Release\CvPopupReturn.obj Final_Release\CvRandom.obj Final_Release\CvReplayInfo.obj Final_Release\CvReplayMessage.obj Final_Release\CvSelectionGroup.obj Final_Release\CvSelectionGroupAI.obj Final_Release\CvStructs.obj Final_Release\CvTalkingHeadMessage.obj Final_Release\CvTeam.obj Final_Release\CvTeamAI.obj Final_Release\CvUnit.obj Final_Release\CvUnitAI.obj Final_Release\CvXMLLoadUtility.obj Final_Release\CvXMLLoadUtilityGet.obj Final_Release\CvXMLLoadUtilityInit.obj Final_Release\CvXMLLoadUtilitySet.obj Final_Release\CyArea.obj Final_Release\CyAreaInterface.obj Final_Release\CyArgsList.obj Final_Release\CyArtFileMgr.obj Final_Release\CyArtFileMgrInterface.obj Final_Release\CyCity.obj Final_Release\CyCityInterface1.obj Final_Release\CyDeal.obj Final_Release\CyEnumsInterface.obj Final_Release\CyGame.obj Final_Release\CyGameCoreUtils.obj Final_Release\CyGameCoreUtilsInterface.obj Final_Release\CyGameInterface.obj Final_Release\CyGameTextMgr.obj Final_Release\CyGameTextMgrInterface.obj Final_Release\CyGlobalContext.obj Final_Release\CyGlobalContextInterface1.obj Final_Release\CyGlobalContextInterface2.obj Final_Release\CyGlobalContextInterface3.obj Final_Release\CyGlobalContextInterface4.obj Final_Release\CyHallOfFameInfo.obj Final_Release\CyHallOfFameInterface.obj Final_Release\CyInfoInterface1.obj Final_Release\CyInfoInterface2.obj Final_Release\CyInfoInterface3.obj Final_Release\CyMap.obj Final_Release\CyMapGenerator.obj Final_Release\CyMapGeneratorInterface.obj Final_Release\CyMapInterface.obj Final_Release\CyPlayer.obj Final_Release\CyPlayerInterface1.obj Final_Release\CyPlayerInterface2.obj Final_Release\CyPlot.obj Final_Release\CyPlotInterface1.obj Final_Release\CyRandomInterface.obj Final_Release\CyReplayInfo.obj Final_Release\CySelectionGroup.obj Final_Release\CySelectionGroupInterface.obj Final_Release\CyStructsInterface1.obj Final_Release\CyTeam.obj Final_Release\CyTeamInterface.obj Final_Release\CyUnit.obj Final_Release\CyUnitInterface1.obj Final_Release\FAssert.obj Final_Release\FDialogTemplate.obj Final_Release\_precompile.obj 
Final_Release_LINKOBJS=$(Final_Release_OBJS)

### The targets of this project
Debug_BIN=Debug\CvGameCoreDLL.dll
Debug_STATIC_LIB=Debug\CvGameCoreDLL.lib
Debug_LIB_DEF=Debug\CvGameCoreDLL.def
Debug_OTHER=Debug\CvGameCoreDLL.exp Debug\CvGameCoreDLL.pdb Debug\CvGameCoreDLL.ilk
Final_Release_BIN=Final_Release\CvGameCoreDLL.dll
Final_Release_STATIC_LIB=Final_Release\CvGameCoreDLL.lib
Final_Release_LIB_DEF=Final_Release\CvGameCoreDLL.def
Final_Release_OTHER=Final_Release\CvGameCoreDLL.exp Final_Release\CvGameCoreDLL.pdb

.PHONY: all all-before all-custom all-after clean clean-custom distclean distclean-custom depend_Debug Debug-before Debug-after depend_Release Release-before Release-after depend_Release_FxsMemory Release_FxsMemory-before Release_FxsMemory-after depend_Debug_FxsMemory Debug_FxsMemory-before Debug_FxsMemory-after depend_Final_Release Final_Release-before Final_Release-after 

all-before:
all-custom:
all-after:
clean-custom:
distclean-custom:
depend_Debug:
Debug-before:
Debug-after:
depend_Release:
Release-before:
Release-after:
depend_Release_FxsMemory:
Release_FxsMemory-before:
Release_FxsMemory-after:
depend_Debug_FxsMemory:
Debug_FxsMemory-before:
Debug_FxsMemory-after:
depend_Final_Release:
Final_Release-before:
Final_Release-after:

all: all-before Debug Final_Release all-after


dist:
	@zip CvGameCoreDLL.cbp.zip CvGameCoreDLL.cbp Makefile CvArea.cpp CvArea.h CvArtFileMgr.cpp CvArtFileMgr.h CvCity.cpp CvCity.h CvCityAI.cpp CvCityAI.h CvDLLButtonPopup.cpp CvDLLButtonPopup.h CvDLLDataStreamIFaceBase.h CvDLLEngineIFaceBase.h CvDLLEntity.cpp CvDLLEntity.h CvDLLEntityIFaceBase.h CvDLLEventReporterIFaceBase.h CvDLLFAStarIFaceBase.h CvDLLFlagEntityIFaceBase.h CvDLLIniParserIFaceBase.h CvDLLInterfaceIFaceBase.h CvDLLPlotBuilderIFaceBase.h CvDLLPython.cpp CvDLLPythonIFaceBase.h CvDLLSymbolIFaceBase.h CvDLLUtilityIFaceBase.h CvDLLWidgetData.cpp CvDLLWidgetData.h CvDLLXMLIFaceBase.h CvDeal.cpp CvDeal.h CvDefines.h CvDiploParameters.cpp CvDiploParameters.h CvEnums.h CvFractal.cpp CvFractal.h CvGame.cpp CvGame.h CvGameAI.cpp CvGameAI.h CvGameCoreDLL.cpp CvGameCoreDLL.h CvGameCoreDLLDefNew.h CvGameCoreDLLUndefNew.h CvGameCoreUtils.cpp CvGameCoreUtils.h CvGameInterface.cpp CvGameTextMgr.cpp CvGameTextMgr.h CvGlobals.cpp CvGlobals.h CvHallOfFameInfo.cpp CvHallOfFameInfo.h CvInfoWater.cpp CvInfoWater.h CvInfos.cpp CvInfos.h CvInitCore.cpp CvInitCore.h CvMap.cpp CvMap.h CvMapGenerator.cpp CvMapGenerator.h CvPlayer.cpp CvPlayer.h CvPlayerAI.cpp CvPlayerAI.h CvPlot.cpp CvPlot.h CvPlotGroup.cpp CvPlotGroup.h CvPopupInfo.cpp CvPopupInfo.h CvPopupReturn.cpp CvPopupReturn.h CvRandom.cpp CvRandom.h CvReplayInfo.cpp CvReplayInfo.h CvReplayMessage.cpp CvReplayMessage.h CvSelectionGroup.cpp CvSelectionGroup.h CvSelectionGroupAI.cpp CvSelectionGroupAI.h CvString.h CvStructs.cpp CvStructs.h CvTalkingHeadMessage.cpp CvTalkingHeadMessage.h CvTeam.cpp CvTeam.h CvTeamAI.cpp CvTeamAI.h CvUnit.cpp CvUnit.h CvUnitAI.cpp CvUnitAI.h CvXMLLoadUtility.cpp CvXMLLoadUtility.h CvXMLLoadUtilityGet.cpp CvXMLLoadUtilityInit.cpp CvXMLLoadUtilitySet.cpp CyArea.cpp CyArea.h CyAreaInterface.cpp CyArgsList.cpp CyArgsList.h CyArtFileMgr.cpp CyArtFileMgr.h CyArtFileMgrInterface.cpp CyCity.cpp CyCity.h CyCityInterface1.cpp CyDeal.cpp CyDeal.h CyEnumsInterface.cpp CyGame.cpp CyGame.h CyGameCoreUtils.cpp CyGameCoreUtils.h CyGameCoreUtilsInterface.cpp CyGameInterface.cpp CyGameTextMgr.cpp CyGameTextMgr.h CyGameTextMgrInterface.cpp CyGlobalContext.cpp CyGlobalContext.h CyGlobalContextInterface1.cpp CyGlobalContextInterface2.cpp CyGlobalContextInterface3.cpp CyGlobalContextInterface4.cpp CyHallOfFameInfo.cpp CyHallOfFameInfo.h CyHallOfFameInterface.cpp CyInfoInterface1.cpp CyInfoInterface2.cpp CyInfoInterface3.cpp CyMap.cpp CyMap.h CyMapGenerator.cpp CyMapGenerator.h CyMapGeneratorInterface.cpp CyMapInterface.cpp CyPlayer.cpp CyPlayer.h CyPlayerInterface1.cpp CyPlayerInterface2.cpp CyPlot.cpp CyPlot.h CyPlotInterface1.cpp CyRandomInterface.cpp CyReplayInfo.cpp CyReplayInfo.h CySelectionGroup.cpp CySelectionGroup.h CySelectionGroupInterface.cpp CyStructsInterface1.cpp CyTeam.cpp CyTeam.h CyTeamInterface.cpp CyUnit.cpp CyUnit.h CyUnitInterface1.cpp FAssert.cpp FAssert.h FDialogTemplate.cpp FDialogTemplate.h FProfiler.h LinkedList.h _precompile.cpp 

clean_Debug:
	$(RM) $(Debug_BIN) $(Debug_OBJS) $(Debug_RESOURCE) $(Debug_STATIC_LIB) $(Debug_LIB_DEF) $(Debug_OTHER)

distclean_Debug:
	$(RM) $(Debug_BIN) $(Debug_OBJS) $(Debug_DEPS) $(Debug_RESOURCE) $(Debug_STATIC_LIB) $(Debug_LIB_DEF) $(Debug_OTHER) 

clean_Final_Release:
	$(RM) $(Final_Release_BIN) $(Final_Release_OBJS) $(Final_Release_RESOURCE) $(Final_Release_STATIC_LIB) $(Final_Release_LIB_DEF) $(Final_Release_OTHER)

distclean_Final_Release:
	$(RM) $(Final_Release_BIN) $(Final_Release_OBJS) $(Final_Release_DEPS) $(Final_Release_RESOURCE) $(Final_Release_STATIC_LIB) $(Final_Release_LIB_DEF) $(Final_Release_OTHER) 

clean: clean_Debug clean_Final_Release 

distclean: distclean_Debug distclean_Final_Release 

depend:

Debug_DIRS:
	-@if not exist "Debug\." mkdir "Debug"
	-@if not exist "Debug\." mkdir "Debug"

Debug: depend_Debug Debug_DIRS Debug-before $(Debug_BIN) Debug-after

$(Debug_BIN): $(Debug_LINKOBJS) $(Debug_RESOURCE)
	$(Debug_LD) /dll /nologo $(Debug_LIBDIRS) /out:$(Debug_BIN) $(Debug_LIBS) $(Debug_LINKOBJS) $(Debug_RESOURCE) $(Debug_LDFLAGS)

Final_Release_DIRS:
	-@if not exist "Final_Release\." mkdir "Final_Release"
	-@if not exist "Final_Release\." mkdir "Final_Release"

Final_Release: depend_Final_Release Final_Release_DIRS Final_Release-before $(Final_Release_BIN) Final_Release-after

$(Final_Release_BIN): $(Final_Release_LINKOBJS) $(Final_Release_RESOURCE)
	$(Final_Release_LD) /dll /nologo $(Final_Release_LIBDIRS) /out:$(Final_Release_BIN) $(Final_Release_LIBS) $(Final_Release_LINKOBJS) $(Final_Release_RESOURCE) $(Final_Release_LDFLAGS)


Debug/CvArea.obj: CvArea.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvArea.cpp /FoDebug/CvArea.obj

Debug/CvArtFileMgr.obj: CvArtFileMgr.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvArtFileMgr.cpp /FoDebug/CvArtFileMgr.obj

Debug/CvCity.obj: CvCity.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvCity.cpp /FoDebug/CvCity.obj

Debug/CvCityAI.obj: CvCityAI.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvCityAI.cpp /FoDebug/CvCityAI.obj

Debug/CvDLLButtonPopup.obj: CvDLLButtonPopup.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvDLLButtonPopup.cpp /FoDebug/CvDLLButtonPopup.obj

Debug/CvDLLEntity.obj: CvDLLEntity.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvDLLEntity.cpp /FoDebug/CvDLLEntity.obj

Debug/CvDLLPython.obj: CvDLLPython.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvDLLPython.cpp /FoDebug/CvDLLPython.obj

Debug/CvDLLWidgetData.obj: CvDLLWidgetData.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvDLLWidgetData.cpp /FoDebug/CvDLLWidgetData.obj

Debug/CvDeal.obj: CvDeal.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvDeal.cpp /FoDebug/CvDeal.obj

Debug/CvDiploParameters.obj: CvDiploParameters.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvDiploParameters.cpp /FoDebug/CvDiploParameters.obj

Debug/CvFractal.obj: CvFractal.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvFractal.cpp /FoDebug/CvFractal.obj

Debug/CvGame.obj: CvGame.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvGame.cpp /FoDebug/CvGame.obj

Debug/CvGameAI.obj: CvGameAI.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvGameAI.cpp /FoDebug/CvGameAI.obj

Debug/CvGameCoreDLL.obj: CvGameCoreDLL.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvGameCoreDLL.cpp /FoDebug/CvGameCoreDLL.obj

Debug/CvGameCoreUtils.obj: CvGameCoreUtils.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvGameCoreUtils.cpp /FoDebug/CvGameCoreUtils.obj

Debug/CvGameInterface.obj: CvGameInterface.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvGameInterface.cpp /FoDebug/CvGameInterface.obj

Debug/CvGameTextMgr.obj: CvGameTextMgr.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvGameTextMgr.cpp /FoDebug/CvGameTextMgr.obj

Debug/CvGlobals.obj: CvGlobals.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvGlobals.cpp /FoDebug/CvGlobals.obj

Debug/CvHallOfFameInfo.obj: CvHallOfFameInfo.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvHallOfFameInfo.cpp /FoDebug/CvHallOfFameInfo.obj

Debug/CvInfoWater.obj: CvInfoWater.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvInfoWater.cpp /FoDebug/CvInfoWater.obj

Debug/CvInfos.obj: CvInfos.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvInfos.cpp /FoDebug/CvInfos.obj

Debug/CvInitCore.obj: CvInitCore.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvInitCore.cpp /FoDebug/CvInitCore.obj

Debug/CvMap.obj: CvMap.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvMap.cpp /FoDebug/CvMap.obj

Debug/CvMapGenerator.obj: CvMapGenerator.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvMapGenerator.cpp /FoDebug/CvMapGenerator.obj

Debug/CvPlayer.obj: CvPlayer.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvPlayer.cpp /FoDebug/CvPlayer.obj

Debug/CvPlayerAI.obj: CvPlayerAI.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvPlayerAI.cpp /FoDebug/CvPlayerAI.obj

Debug/CvPlot.obj: CvPlot.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvPlot.cpp /FoDebug/CvPlot.obj

Debug/CvPlotGroup.obj: CvPlotGroup.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvPlotGroup.cpp /FoDebug/CvPlotGroup.obj

Debug/CvPopupInfo.obj: CvPopupInfo.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvPopupInfo.cpp /FoDebug/CvPopupInfo.obj

Debug/CvPopupReturn.obj: CvPopupReturn.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvPopupReturn.cpp /FoDebug/CvPopupReturn.obj

Debug/CvRandom.obj: CvRandom.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvRandom.cpp /FoDebug/CvRandom.obj

Debug/CvReplayInfo.obj: CvReplayInfo.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvReplayInfo.cpp /FoDebug/CvReplayInfo.obj

Debug/CvReplayMessage.obj: CvReplayMessage.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvReplayMessage.cpp /FoDebug/CvReplayMessage.obj

Debug/CvSelectionGroup.obj: CvSelectionGroup.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvSelectionGroup.cpp /FoDebug/CvSelectionGroup.obj

Debug/CvSelectionGroupAI.obj: CvSelectionGroupAI.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvSelectionGroupAI.cpp /FoDebug/CvSelectionGroupAI.obj

Debug/CvStructs.obj: CvStructs.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvStructs.cpp /FoDebug/CvStructs.obj

Debug/CvTalkingHeadMessage.obj: CvTalkingHeadMessage.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvTalkingHeadMessage.cpp /FoDebug/CvTalkingHeadMessage.obj

Debug/CvTeam.obj: CvTeam.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvTeam.cpp /FoDebug/CvTeam.obj

Debug/CvTeamAI.obj: CvTeamAI.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvTeamAI.cpp /FoDebug/CvTeamAI.obj

Debug/CvUnit.obj: CvUnit.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvUnit.cpp /FoDebug/CvUnit.obj

Debug/CvUnitAI.obj: CvUnitAI.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvUnitAI.cpp /FoDebug/CvUnitAI.obj

Debug/CvXMLLoadUtility.obj: CvXMLLoadUtility.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvXMLLoadUtility.cpp /FoDebug/CvXMLLoadUtility.obj

Debug/CvXMLLoadUtilityGet.obj: CvXMLLoadUtilityGet.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvXMLLoadUtilityGet.cpp /FoDebug/CvXMLLoadUtilityGet.obj

Debug/CvXMLLoadUtilityInit.obj: CvXMLLoadUtilityInit.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvXMLLoadUtilityInit.cpp /FoDebug/CvXMLLoadUtilityInit.obj

Debug/CvXMLLoadUtilitySet.obj: CvXMLLoadUtilitySet.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CvXMLLoadUtilitySet.cpp /FoDebug/CvXMLLoadUtilitySet.obj

Debug/CyArea.obj: CyArea.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CyArea.cpp /FoDebug/CyArea.obj

Debug/CyAreaInterface.obj: CyAreaInterface.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CyAreaInterface.cpp /FoDebug/CyAreaInterface.obj

Debug/CyArgsList.obj: CyArgsList.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CyArgsList.cpp /FoDebug/CyArgsList.obj

Debug/CyArtFileMgr.obj: CyArtFileMgr.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CyArtFileMgr.cpp /FoDebug/CyArtFileMgr.obj

Debug/CyArtFileMgrInterface.obj: CyArtFileMgrInterface.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CyArtFileMgrInterface.cpp /FoDebug/CyArtFileMgrInterface.obj

Debug/CyCity.obj: CyCity.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CyCity.cpp /FoDebug/CyCity.obj

Debug/CyCityInterface1.obj: CyCityInterface1.cpp
	$(Debug_CPP) /nologo $(Debug_SAFE_CFLAGS) $(Debug_INCS) /c CyCityInterface1.cpp /FoDebug/CyCityInterface1.obj

Debug/CyDeal.obj: CyDeal.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CyDeal.cpp /FoDebug/CyDeal.obj

Debug/CyEnumsInterface.obj: CyEnumsInterface.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CyEnumsInterface.cpp /FoDebug/CyEnumsInterface.obj

Debug/CyGame.obj: CyGame.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CyGame.cpp /FoDebug/CyGame.obj

Debug/CyGameCoreUtils.obj: CyGameCoreUtils.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CyGameCoreUtils.cpp /FoDebug/CyGameCoreUtils.obj

Debug/CyGameCoreUtilsInterface.obj: CyGameCoreUtilsInterface.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CyGameCoreUtilsInterface.cpp /FoDebug/CyGameCoreUtilsInterface.obj

Debug/CyGameInterface.obj: CyGameInterface.cpp
	$(Debug_CPP) /nologo $(Debug_SAFE_CFLAGS) $(Debug_INCS) /c CyGameInterface.cpp /FoDebug/CyGameInterface.obj

Debug/CyGameTextMgr.obj: CyGameTextMgr.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CyGameTextMgr.cpp /FoDebug/CyGameTextMgr.obj

Debug/CyGameTextMgrInterface.obj: CyGameTextMgrInterface.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CyGameTextMgrInterface.cpp /FoDebug/CyGameTextMgrInterface.obj

Debug/CyGlobalContext.obj: CyGlobalContext.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CyGlobalContext.cpp /FoDebug/CyGlobalContext.obj

Debug/CyGlobalContextInterface1.obj: CyGlobalContextInterface1.cpp
	$(Debug_CPP) /nologo $(Debug_SAFE_CFLAGS) $(Debug_INCS) /c CyGlobalContextInterface1.cpp /FoDebug/CyGlobalContextInterface1.obj

Debug/CyGlobalContextInterface2.obj: CyGlobalContextInterface2.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CyGlobalContextInterface2.cpp /FoDebug/CyGlobalContextInterface2.obj

Debug/CyGlobalContextInterface3.obj: CyGlobalContextInterface3.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CyGlobalContextInterface3.cpp /FoDebug/CyGlobalContextInterface3.obj

Debug/CyGlobalContextInterface4.obj: CyGlobalContextInterface4.cpp
	$(Debug_CPP) /nologo $(Debug_SAFE_CFLAGS) $(Debug_INCS) /c CyGlobalContextInterface4.cpp /FoDebug/CyGlobalContextInterface4.obj

Debug/CyHallOfFameInfo.obj: CyHallOfFameInfo.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CyHallOfFameInfo.cpp /FoDebug/CyHallOfFameInfo.obj

Debug/CyHallOfFameInterface.obj: CyHallOfFameInterface.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CyHallOfFameInterface.cpp /FoDebug/CyHallOfFameInterface.obj

Debug/CyInfoInterface1.obj: CyInfoInterface1.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CyInfoInterface1.cpp /FoDebug/CyInfoInterface1.obj

Debug/CyInfoInterface2.obj: CyInfoInterface2.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CyInfoInterface2.cpp /FoDebug/CyInfoInterface2.obj

Debug/CyInfoInterface3.obj: CyInfoInterface3.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CyInfoInterface3.cpp /FoDebug/CyInfoInterface3.obj

Debug/CyMap.obj: CyMap.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CyMap.cpp /FoDebug/CyMap.obj

Debug/CyMapGenerator.obj: CyMapGenerator.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CyMapGenerator.cpp /FoDebug/CyMapGenerator.obj

Debug/CyMapGeneratorInterface.obj: CyMapGeneratorInterface.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CyMapGeneratorInterface.cpp /FoDebug/CyMapGeneratorInterface.obj

Debug/CyMapInterface.obj: CyMapInterface.cpp
	$(Debug_CPP) /nologo $(Debug_SAFE_CFLAGS) $(Debug_INCS) /c CyMapInterface.cpp /FoDebug/CyMapInterface.obj

Debug/CyPlayer.obj: CyPlayer.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CyPlayer.cpp /FoDebug/CyPlayer.obj

Debug/CyPlayerInterface1.obj: CyPlayerInterface1.cpp
	$(Debug_CPP) /nologo $(Debug_SAFE_CFLAGS) $(Debug_INCS) /c CyPlayerInterface1.cpp /FoDebug/CyPlayerInterface1.obj

Debug/CyPlayerInterface2.obj: CyPlayerInterface2.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CyPlayerInterface2.cpp /FoDebug/CyPlayerInterface2.obj

Debug/CyPlot.obj: CyPlot.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CyPlot.cpp /FoDebug/CyPlot.obj

Debug/CyPlotInterface1.obj: CyPlotInterface1.cpp
	$(Debug_CPP) /nologo $(Debug_SAFE_CFLAGS) $(Debug_INCS) /c CyPlotInterface1.cpp /FoDebug/CyPlotInterface1.obj

Debug/CyRandomInterface.obj: CyRandomInterface.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CyRandomInterface.cpp /FoDebug/CyRandomInterface.obj

Debug/CyReplayInfo.obj: CyReplayInfo.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CyReplayInfo.cpp /FoDebug/CyReplayInfo.obj

Debug/CySelectionGroup.obj: CySelectionGroup.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CySelectionGroup.cpp /FoDebug/CySelectionGroup.obj

Debug/CySelectionGroupInterface.obj: CySelectionGroupInterface.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CySelectionGroupInterface.cpp /FoDebug/CySelectionGroupInterface.obj

Debug/CyStructsInterface1.obj: CyStructsInterface1.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CyStructsInterface1.cpp /FoDebug/CyStructsInterface1.obj

Debug/CyTeam.obj: CyTeam.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CyTeam.cpp /FoDebug/CyTeam.obj

Debug/CyTeamInterface.obj: CyTeamInterface.cpp
	$(Debug_CPP) /nologo $(Debug_SAFE_CFLAGS) $(Debug_INCS) /c CyTeamInterface.cpp /FoDebug/CyTeamInterface.obj

Debug/CyUnit.obj: CyUnit.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c CyUnit.cpp /FoDebug/CyUnit.obj

Debug/CyUnitInterface1.obj: CyUnitInterface1.cpp
	$(Debug_CPP) /nologo $(Debug_SAFE_CFLAGS) $(Debug_INCS) /c CyUnitInterface1.cpp /FoDebug/CyUnitInterface1.obj

Debug/FAssert.obj: FAssert.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c FAssert.cpp /FoDebug/FAssert.obj

Debug/FDialogTemplate.obj: FDialogTemplate.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c FDialogTemplate.cpp /FoDebug/FDialogTemplate.obj

Debug/_precompile.obj: _precompile.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c _precompile.cpp /FoDebug/_precompile.obj


Final_Release/CvArea.obj: CvArea.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvArea.cpp /FoFinal_Release/CvArea.obj

Final_Release/CvArtFileMgr.obj: CvArtFileMgr.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvArtFileMgr.cpp /FoFinal_Release/CvArtFileMgr.obj

Final_Release/CvCity.obj: CvCity.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvCity.cpp /FoFinal_Release/CvCity.obj

Final_Release/CvCityAI.obj: CvCityAI.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvCityAI.cpp /FoFinal_Release/CvCityAI.obj

Final_Release/CvDLLButtonPopup.obj: CvDLLButtonPopup.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvDLLButtonPopup.cpp /FoFinal_Release/CvDLLButtonPopup.obj

Final_Release/CvDLLEntity.obj: CvDLLEntity.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvDLLEntity.cpp /FoFinal_Release/CvDLLEntity.obj

Final_Release/CvDLLPython.obj: CvDLLPython.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvDLLPython.cpp /FoFinal_Release/CvDLLPython.obj

Final_Release/CvDLLWidgetData.obj: CvDLLWidgetData.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvDLLWidgetData.cpp /FoFinal_Release/CvDLLWidgetData.obj

Final_Release/CvDeal.obj: CvDeal.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvDeal.cpp /FoFinal_Release/CvDeal.obj

Final_Release/CvDiploParameters.obj: CvDiploParameters.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvDiploParameters.cpp /FoFinal_Release/CvDiploParameters.obj

Final_Release/CvFractal.obj: CvFractal.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvFractal.cpp /FoFinal_Release/CvFractal.obj

Final_Release/CvGame.obj: CvGame.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvGame.cpp /FoFinal_Release/CvGame.obj

Final_Release/CvGameAI.obj: CvGameAI.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvGameAI.cpp /FoFinal_Release/CvGameAI.obj

Final_Release/CvGameCoreDLL.obj: CvGameCoreDLL.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvGameCoreDLL.cpp /FoFinal_Release/CvGameCoreDLL.obj

Final_Release/CvGameCoreUtils.obj: CvGameCoreUtils.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvGameCoreUtils.cpp /FoFinal_Release/CvGameCoreUtils.obj

Final_Release/CvGameInterface.obj: CvGameInterface.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvGameInterface.cpp /FoFinal_Release/CvGameInterface.obj

Final_Release/CvGameTextMgr.obj: CvGameTextMgr.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvGameTextMgr.cpp /FoFinal_Release/CvGameTextMgr.obj

Final_Release/CvGlobals.obj: CvGlobals.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvGlobals.cpp /FoFinal_Release/CvGlobals.obj

Final_Release/CvHallOfFameInfo.obj: CvHallOfFameInfo.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvHallOfFameInfo.cpp /FoFinal_Release/CvHallOfFameInfo.obj

Final_Release/CvInfoWater.obj: CvInfoWater.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvInfoWater.cpp /FoFinal_Release/CvInfoWater.obj

Final_Release/CvInfos.obj: CvInfos.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvInfos.cpp /FoFinal_Release/CvInfos.obj

Final_Release/CvInitCore.obj: CvInitCore.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvInitCore.cpp /FoFinal_Release/CvInitCore.obj

Final_Release/CvMap.obj: CvMap.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvMap.cpp /FoFinal_Release/CvMap.obj

Final_Release/CvMapGenerator.obj: CvMapGenerator.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvMapGenerator.cpp /FoFinal_Release/CvMapGenerator.obj

Final_Release/CvPlayer.obj: CvPlayer.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvPlayer.cpp /FoFinal_Release/CvPlayer.obj

Final_Release/CvPlayerAI.obj: CvPlayerAI.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvPlayerAI.cpp /FoFinal_Release/CvPlayerAI.obj

Final_Release/CvPlot.obj: CvPlot.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvPlot.cpp /FoFinal_Release/CvPlot.obj

Final_Release/CvPlotGroup.obj: CvPlotGroup.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvPlotGroup.cpp /FoFinal_Release/CvPlotGroup.obj

Final_Release/CvPopupInfo.obj: CvPopupInfo.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvPopupInfo.cpp /FoFinal_Release/CvPopupInfo.obj

Final_Release/CvPopupReturn.obj: CvPopupReturn.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvPopupReturn.cpp /FoFinal_Release/CvPopupReturn.obj

Final_Release/CvRandom.obj: CvRandom.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvRandom.cpp /FoFinal_Release/CvRandom.obj

Final_Release/CvReplayInfo.obj: CvReplayInfo.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvReplayInfo.cpp /FoFinal_Release/CvReplayInfo.obj

Final_Release/CvReplayMessage.obj: CvReplayMessage.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvReplayMessage.cpp /FoFinal_Release/CvReplayMessage.obj

Final_Release/CvSelectionGroup.obj: CvSelectionGroup.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvSelectionGroup.cpp /FoFinal_Release/CvSelectionGroup.obj

Final_Release/CvSelectionGroupAI.obj: CvSelectionGroupAI.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvSelectionGroupAI.cpp /FoFinal_Release/CvSelectionGroupAI.obj

Final_Release/CvStructs.obj: CvStructs.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvStructs.cpp /FoFinal_Release/CvStructs.obj

Final_Release/CvTalkingHeadMessage.obj: CvTalkingHeadMessage.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvTalkingHeadMessage.cpp /FoFinal_Release/CvTalkingHeadMessage.obj

Final_Release/CvTeam.obj: CvTeam.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvTeam.cpp /FoFinal_Release/CvTeam.obj

Final_Release/CvTeamAI.obj: CvTeamAI.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvTeamAI.cpp /FoFinal_Release/CvTeamAI.obj

Final_Release/CvUnit.obj: CvUnit.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvUnit.cpp /FoFinal_Release/CvUnit.obj

Final_Release/CvUnitAI.obj: CvUnitAI.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvUnitAI.cpp /FoFinal_Release/CvUnitAI.obj

Final_Release/CvXMLLoadUtility.obj: CvXMLLoadUtility.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvXMLLoadUtility.cpp /FoFinal_Release/CvXMLLoadUtility.obj

Final_Release/CvXMLLoadUtilityGet.obj: CvXMLLoadUtilityGet.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvXMLLoadUtilityGet.cpp /FoFinal_Release/CvXMLLoadUtilityGet.obj

Final_Release/CvXMLLoadUtilityInit.obj: CvXMLLoadUtilityInit.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvXMLLoadUtilityInit.cpp /FoFinal_Release/CvXMLLoadUtilityInit.obj

Final_Release/CvXMLLoadUtilitySet.obj: CvXMLLoadUtilitySet.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CvXMLLoadUtilitySet.cpp /FoFinal_Release/CvXMLLoadUtilitySet.obj

Final_Release/CyArea.obj: CyArea.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyArea.cpp /FoFinal_Release/CyArea.obj

Final_Release/CyAreaInterface.obj: CyAreaInterface.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyAreaInterface.cpp /FoFinal_Release/CyAreaInterface.obj

Final_Release/CyArgsList.obj: CyArgsList.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyArgsList.cpp /FoFinal_Release/CyArgsList.obj

Final_Release/CyArtFileMgr.obj: CyArtFileMgr.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyArtFileMgr.cpp /FoFinal_Release/CyArtFileMgr.obj

Final_Release/CyArtFileMgrInterface.obj: CyArtFileMgrInterface.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyArtFileMgrInterface.cpp /FoFinal_Release/CyArtFileMgrInterface.obj

Final_Release/CyCity.obj: CyCity.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyCity.cpp /FoFinal_Release/CyCity.obj

Final_Release/CyCityInterface1.obj: CyCityInterface1.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyCityInterface1.cpp /FoFinal_Release/CyCityInterface1.obj

Final_Release/CyDeal.obj: CyDeal.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyDeal.cpp /FoFinal_Release/CyDeal.obj

Final_Release/CyEnumsInterface.obj: CyEnumsInterface.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyEnumsInterface.cpp /FoFinal_Release/CyEnumsInterface.obj

Final_Release/CyGame.obj: CyGame.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyGame.cpp /FoFinal_Release/CyGame.obj

Final_Release/CyGameCoreUtils.obj: CyGameCoreUtils.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyGameCoreUtils.cpp /FoFinal_Release/CyGameCoreUtils.obj

Final_Release/CyGameCoreUtilsInterface.obj: CyGameCoreUtilsInterface.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyGameCoreUtilsInterface.cpp /FoFinal_Release/CyGameCoreUtilsInterface.obj

Final_Release/CyGameInterface.obj: CyGameInterface.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyGameInterface.cpp /FoFinal_Release/CyGameInterface.obj

Final_Release/CyGameTextMgr.obj: CyGameTextMgr.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyGameTextMgr.cpp /FoFinal_Release/CyGameTextMgr.obj

Final_Release/CyGameTextMgrInterface.obj: CyGameTextMgrInterface.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyGameTextMgrInterface.cpp /FoFinal_Release/CyGameTextMgrInterface.obj

Final_Release/CyGlobalContext.obj: CyGlobalContext.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyGlobalContext.cpp /FoFinal_Release/CyGlobalContext.obj

Final_Release/CyGlobalContextInterface1.obj: CyGlobalContextInterface1.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyGlobalContextInterface1.cpp /FoFinal_Release/CyGlobalContextInterface1.obj

Final_Release/CyGlobalContextInterface2.obj: CyGlobalContextInterface2.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyGlobalContextInterface2.cpp /FoFinal_Release/CyGlobalContextInterface2.obj

Final_Release/CyGlobalContextInterface3.obj: CyGlobalContextInterface3.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyGlobalContextInterface3.cpp /FoFinal_Release/CyGlobalContextInterface3.obj

Final_Release/CyGlobalContextInterface4.obj: CyGlobalContextInterface4.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyGlobalContextInterface4.cpp /FoFinal_Release/CyGlobalContextInterface4.obj

Final_Release/CyHallOfFameInfo.obj: CyHallOfFameInfo.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyHallOfFameInfo.cpp /FoFinal_Release/CyHallOfFameInfo.obj

Final_Release/CyHallOfFameInterface.obj: CyHallOfFameInterface.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyHallOfFameInterface.cpp /FoFinal_Release/CyHallOfFameInterface.obj

Final_Release/CyInfoInterface1.obj: CyInfoInterface1.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyInfoInterface1.cpp /FoFinal_Release/CyInfoInterface1.obj

Final_Release/CyInfoInterface2.obj: CyInfoInterface2.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyInfoInterface2.cpp /FoFinal_Release/CyInfoInterface2.obj

Final_Release/CyInfoInterface3.obj: CyInfoInterface3.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyInfoInterface3.cpp /FoFinal_Release/CyInfoInterface3.obj

Final_Release/CyMap.obj: CyMap.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyMap.cpp /FoFinal_Release/CyMap.obj

Final_Release/CyMapGenerator.obj: CyMapGenerator.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyMapGenerator.cpp /FoFinal_Release/CyMapGenerator.obj

Final_Release/CyMapGeneratorInterface.obj: CyMapGeneratorInterface.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyMapGeneratorInterface.cpp /FoFinal_Release/CyMapGeneratorInterface.obj

Final_Release/CyMapInterface.obj: CyMapInterface.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyMapInterface.cpp /FoFinal_Release/CyMapInterface.obj

Final_Release/CyPlayer.obj: CyPlayer.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyPlayer.cpp /FoFinal_Release/CyPlayer.obj

Final_Release/CyPlayerInterface1.obj: CyPlayerInterface1.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyPlayerInterface1.cpp /FoFinal_Release/CyPlayerInterface1.obj

Final_Release/CyPlayerInterface2.obj: CyPlayerInterface2.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyPlayerInterface2.cpp /FoFinal_Release/CyPlayerInterface2.obj

Final_Release/CyPlot.obj: CyPlot.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyPlot.cpp /FoFinal_Release/CyPlot.obj

Final_Release/CyPlotInterface1.obj: CyPlotInterface1.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyPlotInterface1.cpp /FoFinal_Release/CyPlotInterface1.obj

Final_Release/CyRandomInterface.obj: CyRandomInterface.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyRandomInterface.cpp /FoFinal_Release/CyRandomInterface.obj

Final_Release/CyReplayInfo.obj: CyReplayInfo.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyReplayInfo.cpp /FoFinal_Release/CyReplayInfo.obj

Final_Release/CySelectionGroup.obj: CySelectionGroup.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CySelectionGroup.cpp /FoFinal_Release/CySelectionGroup.obj

Final_Release/CySelectionGroupInterface.obj: CySelectionGroupInterface.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CySelectionGroupInterface.cpp /FoFinal_Release/CySelectionGroupInterface.obj

Final_Release/CyStructsInterface1.obj: CyStructsInterface1.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyStructsInterface1.cpp /FoFinal_Release/CyStructsInterface1.obj

Final_Release/CyTeam.obj: CyTeam.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyTeam.cpp /FoFinal_Release/CyTeam.obj

Final_Release/CyTeamInterface.obj: CyTeamInterface.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyTeamInterface.cpp /FoFinal_Release/CyTeamInterface.obj

Final_Release/CyUnit.obj: CyUnit.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyUnit.cpp /FoFinal_Release/CyUnit.obj

Final_Release/CyUnitInterface1.obj: CyUnitInterface1.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c CyUnitInterface1.cpp /FoFinal_Release/CyUnitInterface1.obj

Final_Release/FAssert.obj: FAssert.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c FAssert.cpp /FoFinal_Release/FAssert.obj

Final_Release/FDialogTemplate.obj: FDialogTemplate.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c FDialogTemplate.cpp /FoFinal_Release/FDialogTemplate.obj

Final_Release/_precompile.obj: _precompile.cpp
	$(Final_Release_CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /c _precompile.cpp /FoFinal_Release/_precompile.obj



