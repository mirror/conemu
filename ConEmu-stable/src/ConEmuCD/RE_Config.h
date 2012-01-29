
#pragma once

class RegConfig;

class RegConfig {
public:
	RegConfig();
	~RegConfig();

	UINT TitleAddLen() { return 0; };

	TCHAR sRegTitlePrefix[20];
	BOOL is64bitOs;
	UINT nAnsiCodePage;
};

extern RegConfig *cfg;
