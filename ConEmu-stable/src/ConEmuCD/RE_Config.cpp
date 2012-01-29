
#include "header.h"

RegConfig *cfg;
RegConfig TheCfg;

RegConfig::RegConfig()
{
	cfg = this;
	sRegTitlePrefix[0] = 0;
	is64bitOs = FALSE;
	nAnsiCodePage = CP_ACP;
}

RegConfig::~RegConfig()
{
	cfg = NULL;
}
