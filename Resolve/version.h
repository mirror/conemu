
#define MVV_1 1
#define MVV_2 1
#define MVV_3 FARMANAGERVERSION_BUILD
#define MVV_4 0
#define MVV_4a ""

#ifdef _UNICODE
	#if FAR_UNICODE>=1906
		#include "common/far3/pluginW3.hpp"
	#else
		#include "common/unicode/pluginW.hpp"
	#endif
#else
	#include "common/ascii/pluginA.hpp"
#endif


#define STRING2(x) #x
#define STRING(x) STRING2(x)

//#define MYDLLVERS STRING(MVV_1) "." STRING(MVV_2) "." STRING(MVV_3) "#" STRING(MVV_4) "\0"
#define MYDLLVERS STRING(MVV_1) "." STRING(MVV_2) "." STRING(MVV_3) "\0"
#define MYDLLVERN MVV_1,MVV_2,MVV_3,MVV_4

#ifdef _WIN64
#define MYDLLPLTFRM " (x64)"
#else
#define MYDLLPLTFRM " (x86)"
#endif
