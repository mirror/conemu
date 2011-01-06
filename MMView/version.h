
#define MVV_1 1
#define MVV_2 0
#define MVV_3 7
#define MVV_4 8

#ifdef _UNICODE
	#ifdef _WIN64
		#define VER_SUFFIX " (Unicode x64)"
	#else
		#define VER_SUFFIX " (Unicode x86)"
	#endif
#else
	#ifdef _WIN64
		#define VER_SUFFIX " (Ansi x64)"
	#else
		#define VER_SUFFIX " (Ansi x86)"
	#endif
#endif

#define STRING2(x) #x
#define STRING(x) STRING2(x)

#define MULTIVIEWVERS STRING(MVV_1) ", " STRING(MVV_2) ", " STRING(MVV_3) ", " STRING(MVV_4) VER_SUFFIX "\0"
#define MULTIVIEWVERN MVV_1,MVV_2,MVV_3,MVV_4
