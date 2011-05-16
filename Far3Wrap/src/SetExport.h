
/*
Copyright (c) 2011 Maximus5
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

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
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

struct ExportFunc
{
	char OldName[64];
	char NewName[64];
};

static bool ChangeExports( const ExportFunc* Funcs, PBYTE Module )
{
    if( !Module )
        return false;
    
    DWORD ExportDir = 0;
    IMAGE_DOS_HEADER* dos_header = (IMAGE_DOS_HEADER*)Module;
    IMAGE_NT_HEADERS* nt_header = 0;
    if( dos_header->e_magic == 'ZM' )
    {
        nt_header = (IMAGE_NT_HEADERS*)(Module + dos_header->e_lfanew);
        if( nt_header->Signature != 0x004550 )
            return false;
        else
            ExportDir = (DWORD)(nt_header->OptionalHeader.
                                         DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].
                                         VirtualAddress);
    }
    if (!ExportDir)
        return false;

    IMAGE_SECTION_HEADER* section = (IMAGE_SECTION_HEADER*)IMAGE_FIRST_SECTION (nt_header);

    int s = 0;
    for(s = 0; s < nt_header->FileHeader.NumberOfSections; s++)
    {
        if((section[s].VirtualAddress == ExportDir) ||
           (section[s].VirtualAddress <= ExportDir &&
           (section[s].Misc.VirtualSize + section[s].VirtualAddress > ExportDir)))
        {
            int nDiff = section[s].VirtualAddress - section[s].PointerToRawData; // был 0
            IMAGE_EXPORT_DIRECTORY* Export = (IMAGE_EXPORT_DIRECTORY*)(Module + (ExportDir-nDiff));
            DWORD* Name = (DWORD*)((char*)Module + Export->AddressOfNames-nDiff);

			for (DWORD i = 0; Name && i < Export->NumberOfNames; i++)
            {
                for (int j = 0; *Funcs[j].OldName; j++)
				{
					if (Name[i])
					{
						char* pszExpName = (char*)(Module + Name[i]-nDiff);
						if (lstrcmpA(pszExpName, Funcs[j].OldName) == 0)
						{
							_ASSERTE(lstrlenA(Funcs[j].NewName)==lstrlenA(Funcs[j].OldName));
							lstrcpyA(pszExpName, Funcs[j].NewName);
							break;
						}
                    }
				}
            }
        }
    }
    return true;
}
