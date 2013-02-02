/*
Copyright (C) 2001 DarkOne the Hacker
	http://darkone.yo.lv
	DarkOne@mail.navigators.lv

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#include <windows.h>
#include <stdio.h>
#include "wcxhead.h"
#include "PACK.h"

// ------------------------- * Globals * -------------------------

// ------------------------- * Devider * -------------------------

/*
** Dll Entry Point.
** Called by Windows' kernel when dll is (un)loaded!
*/
BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD reason, LPVOID reserved)
{
	return TRUE;
}

// ------------------------- * !DLL Exports! * -------------------------

/*
** Plugin Interface!
*/

// OpenArchive should perform all necessary operations when an archive is to be opened
HANDLE __stdcall OpenArchive(tOpenArchiveData *ArchiveData)
{
	return PAK_Open(ArchiveData);
}

// WinCmd calls ReadHeader to find out what files are in the archive
int __stdcall ReadHeader(HANDLE hArcData, tHeaderData *HeaderData)
{
	return PAK_NextItem(hArcData, HeaderData);
}

// ProcessFile should unpack the specified file or test the integrity of the archive
int __stdcall ProcessFile(HANDLE hArcData, int Operation, char *DestPath, char *DestName)
{
	return PAK_Process(hArcData, Operation, DestPath, DestName);
}

// CloseArchive should perform all necessary operations when an archive is about to be closed
int __stdcall CloseArchive(HANDLE *hArcData)
{
	return PAK_Close(hArcData);
}

// This function allows you to notify user about changing a volume when packing files
void __stdcall SetChangeVolProc(HANDLE hArcData, tChangeVolProc pChangeVolProc1)
{
	PAK_SetCallBackVol(hArcData, pChangeVolProc1);
}

// This function allows you to notify user about the progress when you un/pack files
void __stdcall SetProcessDataProc(HANDLE hArcData, tProcessDataProc pProcessDataProc)
{
	PAK_SetCallBackProc(hArcData, pProcessDataProc);
}

// ------------------------- * optional * -------------------------

// GetPackerCaps tells WinCmd what features your packer plugin supports
int __stdcall GetPackerCaps(void)
{
	return PK_CAPS_NEW|PK_CAPS_MODIFY|PK_CAPS_MULTIPLE|PK_CAPS_DELETE|PK_CAPS_OPTIONS;
}

// PackFiles specifies what should happen when a user creates, or adds files to the archive
int __stdcall PackFiles(char *PackedFile, char *SubPath, char *SrcPath, char *AddList, int Flags)
{
	return PAK_Pack(PackedFile, SubPath, SrcPath, AddList, Flags);
}

// DeleteFiles should delete the specified files from the archive
int __stdcall DeleteFiles(char *PackedFile, char *DeleteList)
{
	return PAK_Delete(PackedFile, DeleteList);
}

// ConfigurePacker gets called when the user clicks the Configure
// button from within "Pack files..." dialog box in WinCmd
void __stdcall ConfigurePacker(HWND Parent, HINSTANCE DllInstance)
{
	PAK_Configure(Parent, DllInstance);
}
