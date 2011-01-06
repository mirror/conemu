#pragma once

#include "MMView_Lang.h"

//enum { // .LNG FILE
//	FarPicName = 0,
//	FarPicOK,
//	FarPicCancel,
//
//// Configuration strings
//	FarPicConfNotUnloadGfl,
//	FarPicConfBilinear,
//	FarPicConfNotSetStartVolume,
//	FarPicNoGflWarn,
//	FarPicConfDelay9xText,
//	FarPicConfVidSeekIntervalText,
//	FarPicConfVidKeyframe,
//	FarPicConfVideoExtsText,
//	FarPicConfAudioExtsText,
//	FarPicConfPictureExtsText,
//	FarPicConfVideoFullWind,
//	FarPicConfPlugActive,
//	FarPicConfReset,
//
//// dialogue strings
//	FarPicWarnHotKeyText1,
//	FarPicWarnHotKeyText2,
//	FarPicWarnNoMedia,
//	FarPicWarnNoPicture,
//
//// openplugin strings
//	FarPicOpenFitWindow,
//	FarPicOpenReStart,
//
//	FarPicOpenVidForward,
//	FarPicOpenVidFastForward,
//	FarPicOpenVidBack,
//	FarPicOpenVidFastBack,
//	FarPicOpenVidHome,
//	FarPicOpenVidEnd,
//	FarPicOpenVidPause,
//	FarPicOpenVidStop,
//	FarPicOpenFullScreen,
//
//	FarPicOpenVolumeUp,
//	FarPicOpenVolumeDown,
//	FarPicOpenVidJump,
//
//	FarPicOpenVidJumpTitle,
//	FarPicOpenVidJumpText,
//
//	FarPicNavigate,
//// openplugin strings
//
//	FarPicGflErrorLoad,
//	FarPicGflErrorDamg,
//	FarPicGflErrorVer,
//	FarPicGflErrorInit,
//	FarPicNoViewerWindow,
//	FarPicDetectingMedia,
//	FarPicSeeking,
//};


enum { //Config dialogue
	FPCDTitle = 0,

	FPCDPlugActive,
	FPCDCtrlShiftF3,

	FPCDNotUnloadGfl,
	FPCDBilinear,
	FPCDFullWindow,
	FPCDNoSetStartVolume,
	FPCDNoGflWarning,

	FPCDDelay9xText,
	FPCDDelay9xData,

	FPCDVidKeyframe,
	FPCDVidSeekIntervalText,
	FPCDVidSeekIntervalData,
	FPCDVideoExtsText,
	FPCDVideoExtsData,
	FPCDAudioExtsText,
	FPCDAudioExtsData,
	FPCDPictureExtsText,
	FPCDPictureExtsData,

	FPCDVideoExtsReset,
	FPCDAudioExtsReset,
	FPCDPictureExtsReset,

	FPCDOK,
	FPCDCancel,

	FPCDLast,
};

enum { // OpenPlugin gfl Window
	FPODgflFitWindow = 0,
	FPODgflStop,
	FPODgfl1Sep,
	FPODgflReStart,

	FPODgflLast,
};

enum { // OpenPlugin video Window
	FPODVidFitWindow = 0,
	FPODVidForward,
	FPODVidFastForward,
	FPODVidBack,
	FPODVidFastBack,
	FPODVidHome,
	FPODVidEnd,
	FPODVidPause,
	FPODVidStop,
	FPODVidFullScreen,
	FPODVid1Sep,
	FPODVolumeUp,
	FPODVolumeDown,
	FPODVolumeMute,
	FPODVid2Sep,
	FPODVidJump,
	FPODVidReStart,
	FPODVidLast,
};

// Commands from "callplugin(...)" macro
enum {
	MVC_FitWindowSwitch = 1,
	MVC_Forward = 2,
	MVC_FastForward = 3,
	MVC_Back = 4,
	MVC_FastBack = 5,
	MVC_Home = 6,
	MVC_End = 7,
	MVC_PlayPause = 8,
	MVC_Stop = 9,
	MVC_FullScreenSwitch = 10,
	MVC_VolumeUp = 11,
	MVC_VolumeDown = 12,
	MVC_VolumeMute = 13,
	MVC_JumpToTime = 14,
	MVC_Restart = 15,
};
