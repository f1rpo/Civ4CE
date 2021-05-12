//
// published python interface for CyGlobalContext
// Author - Mustafa Thamer
//

#include "CvGameCoreDLL.h"
#include "CyMap.h"
#include "CyPlayer.h"
#include "CyGame.h"
#include "CyGlobalContext.h"
#include "CvRandom.h"
//#include "CvStructs.h"
#include "CvInfos.h"
#include "CyTeam.h"


void CyGlobalContextPythonInterface2(python::class_<CyGlobalContext>& x)
{
	OutputDebugString("Python Extension Module - CyGlobalContextPythonInterface2\n");

	x
		// global defines.xml
		.def("getDefineINT", &CyGlobalContext::getDefineINT, "int ( string szName )" )
		.def("getDefineFLOAT", &CyGlobalContext::getDefineFLOAT, "float ( string szName )" )
		.def("getDefineSTRING", &CyGlobalContext::getDefineSTRING, "string getDefineSTRING( string szName )" )
		.def("setDefineINT", &CyGlobalContext::setDefineINT, "void ( string szName, int iValue )" )
		.def("setDefineFLOAT", &CyGlobalContext::setDefineFLOAT, "void setDefineFLOAT( string szName, float fValue )" )
		.def("setDefineSTRING", &CyGlobalContext::setDefineSTRING, "void ( string szName, string szValue )" )

		.def("getMOVE_DENOMINATOR", &CyGlobalContext::getMOVE_DENOMINATOR, "int ()")
		.def("getNUM_UNIT_PREREQ_OR_BONUSES", &CyGlobalContext::getNUM_UNIT_PREREQ_OR_BONUSES, "int ()")
		.def("getNUM_BUILDING_PREREQ_OR_BONUSES", &CyGlobalContext::getNUM_BUILDING_PREREQ_OR_BONUSES, "int ()")
		.def("getFOOD_CONSUMPTION_PER_POPULATION", &CyGlobalContext::getFOOD_CONSUMPTION_PER_POPULATION, "int ()")
		.def("getMAX_HIT_POINTS", &CyGlobalContext::getMAX_HIT_POINTS, "int ()")
		.def("getHILLS_EXTRA_DEFENSE", &CyGlobalContext::getHILLS_EXTRA_DEFENSE, "int ()")
		.def("getRIVER_ATTACK_MODIFIER", &CyGlobalContext::getRIVER_ATTACK_MODIFIER, "int ()")
		.def("getAMPHIB_ATTACK_MODIFIER", &CyGlobalContext::getAMPHIB_ATTACK_MODIFIER, "int ()")
		.def("getHILLS_EXTRA_MOVEMENT", &CyGlobalContext::getHILLS_EXTRA_MOVEMENT, "int ()")
		.def("getFORTIFY_MODIFIER_PER_TURN", &CyGlobalContext::getFORTIFY_MODIFIER_PER_TURN, "int ()")
		.def("getMAX_CITY_DEFENSE_DAMAGE", &CyGlobalContext::getMAX_CITY_DEFENSE_DAMAGE, "int ()")

		.def("getMAX_CIV_PLAYERS", &CyGlobalContext::getMAX_CIV_PLAYERS, "int ()")
		.def("getMAX_PLAYERS", &CyGlobalContext::getMAX_PLAYERS, "int ()")
		.def("getMAX_CIV_TEAMS", &CyGlobalContext::getMAX_CIV_TEAMS, "int ()")
		.def("getMAX_TEAMS", &CyGlobalContext::getMAX_TEAMS, "int ()")
		.def("getBARBARIAN_PLAYER", &CyGlobalContext::getBARBARIAN_PLAYER, "int ()")
		.def("getBARBARIAN_TEAM", &CyGlobalContext::getBARBARIAN_TEAM, "int ()")
		.def("getINVALID_PLOT_COORD", &CyGlobalContext::getINVALID_PLOT_COORD, "int ()")
		.def("getNUM_CITY_PLOTS", &CyGlobalContext::getNUM_CITY_PLOTS, "int ()")
		.def("getCITY_HOME_PLOT", &CyGlobalContext::getCITY_HOME_PLOT, "int ()")
		;
}
