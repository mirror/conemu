
#define MVV_1 3
#define MVV_2 0
#define MVV_3 0
#define MVV_4 FARMANAGERVERSION_BUILD
#define MVV_4a ""

#ifdef FAR_UNICODE
	#include "headers/pluginW3.hpp"
#else
	#include "headers/plugin.hpp"
#endif


#define STRING2(x) #x
#define STRING(x) STRING2(x)

#define MYDLLVERS STRING(MVV_1) "." STRING(MVV_2) "." STRING(MVV_3) "#" STRING(MVV_4) "\0"
#define MYDLLVERN MVV_1,MVV_2,MVV_3,MVV_4

#ifdef _WIN64
#define MYDLLPLTFRM " (x64)"
#else
#define MYDLLPLTFRM " (x86)"
#endif
