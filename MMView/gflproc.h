enum {
	PVgflLibraryInit = 0,
	PVgflEnableLZW,
	PVgflLibraryExit,
	PVgflGetVersion,
	PVgflLoadBitmapFromMemory,
	PVgflGetDefaultLoadParams,
	PVgflFreeBitmap,
	PVgflResize,
};
char *GflProcNames[] = {"gflLibraryInit", "gflEnableLZW", "gflLibraryExit", "gflGetVersion",
						"gflLoadBitmapFromMemory", "gflGetDefaultLoadParams", "gflFreeBitmap",
						"gflResize"};

typedef int (WINAPI *gfl)() ;
gfl GflProc[NUM_ITEMS(GflProcNames)];

typedef GFL_ERROR		(WINAPI *PVgflLibraryInitT)();
typedef void 			(WINAPI *PVgflEnableLZWT)(GFL_BOOL);
typedef void 			(WINAPI *PVgflLibraryExitT)(void);
typedef const char *	(WINAPI *PVgflGetVersionT)(void);
typedef GFL_ERROR		(WINAPI *PVgflLoadBitmapFromMemoryT)( const GFL_UINT8* data, GFL_UINT32 data_length, GFL_BITMAP** bitmap, const GFL_LOAD_PARAMS* params, GFL_FILE_INFORMATION* info );
typedef void			(WINAPI *PVgflGetDefaultLoadParamsT)(GFL_LOAD_PARAMS *);
typedef void			(WINAPI *PVgflFreeBitmapT)(GFL_BITMAP *);
typedef GFL_ERROR		(WINAPI *PVgflResizeT)(GFL_BITMAP *, GFL_BITMAP **, GFL_INT32, GFL_INT32, GFL_UINT32, GFL_UINT32);

enum { // Результат попытки загрузить GFL
	PVGflLoadOK = 0,
	PVGflLoadNotLoaded,
	PVGflLoadDamaged,
	PVGflLoadLowVersion,
	PVGflLoadNotInit,
};