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

// *** id Software's PAK file format ***
// details on this format:
// http://www.gamers.org/dEngine/quake/spec/quake-spec34/qkspec_3.htm


typedef struct
{
	char magic[4];	// "PACK"
	long diroffset;	// Position of PACK directory from start of file
	long dirsize;		// Number of entries * 0x40 (64 char)
} pakheader_t;

typedef struct
{
	char filename[0x38];// Name of the file, Unix style, with extension, 50 chars.
	long offset;				// Position of the entry in PACK file
	long size;					// Size of the entry in PACK file
} pakentry_t;
// sizeof(pakentry_t)==56+4+4=64

void *PAK_Open(tOpenArchiveData *arch);
int PAK_Close(void *a);
void PAK_SetCallBackVol(void *a, tChangeVolProc pChangeVolProc1);
void PAK_SetCallBackProc(void *a, tProcessDataProc pProcessDataProc);
int PAK_NextItem(void *a, tHeaderData *item);
int PAK_Process(void *a, int Operation, char *DestPath, char *DestName);

int PAK_Pack(char *name, char *spath, char *path, char *AddList, int flags);
int PAK_Add(char *name, char *spath, char *path, char *AddList, int flags);
int PAK_Delete(char *name, char *DelList);

void PAK_Configure(HWND Parent, HINSTANCE DllInstance);