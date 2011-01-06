
/*
Copyright (c) 2009-2010 Maximus5
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include "pluginA.hpp"

extern PluginStartupInfo psi;
extern FarStandardFunctions fsf;

typedef void (WINAPI* FClosePlugin)(HANDLE hPlugin);
void WINAPI ClosePlugin(HANDLE hPlugin);

typedef int (WINAPI* FConfigure)(int ItemNumber);
int WINAPI Configure(int ItemNumber);

typedef void (WINAPI* FExitFAR)(void);
void WINAPI ExitFAR(void);

typedef int (WINAPI* FGetMinFarVersion)(void);
int WINAPI GetMinFarVersion(void);

typedef void (WINAPI* FGetPluginInfo)(struct PluginInfo *Info);
void WINAPI GetPluginInfo(struct PluginInfo *Info);

typedef HANDLE (WINAPI* FOpenFilePlugin)(char *Name,const unsigned char *Data,int DataSize);
HANDLE WINAPI OpenFilePlugin(char *Name,const unsigned char *Data,int DataSize);

typedef HANDLE (WINAPI* FOpenPlugin)(int OpenFrom,INT_PTR Item);
HANDLE WINAPI OpenPlugin(int OpenFrom,INT_PTR Item);

typedef int (WINAPI* FProcessEditorEvent)(int Event,void *Param);
int WINAPI ProcessEditorEvent(int Event,void *Param);

typedef int (WINAPI* FProcessViewerEvent)(int Event,void *Param);
int WINAPI ProcessViewerEvent(int Event,void *Param);

typedef void (WINAPI* FSetStartupInfo)(const struct PluginStartupInfo *Info);
void WINAPI SetStartupInfo(const struct PluginStartupInfo *Info);

// Wrapper
typedef INT_PTR (WINAPI* FWrapperAdvControl)(int ModuleNumber,int Command,void *Param);
INT_PTR WINAPI WrapperAdvControl(int ModuleNumber,int Command,void *Param);
