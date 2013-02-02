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
#include <io.h>
#include <direct.h>
#include "wcxhead.h"
#include "PACK.h"

// ------------------------- * Globals * -------------------------
tChangeVolProc ppVol;
tProcessDataProc ppProc;

typedef struct
{
	char name[MAX_PATH];
	FILE *fp;
	pakheader_t header; // header
	pakentry_t *entry;
	int total, current; // entry
	int in_use;
	int mode;
	int opentime;
	tChangeVolProc pVol;
	tProcessDataProc pProc;
} archive_t;

archive_t arh[2];		// max 2 archives opened same time!
// ------------------------- * Devider * -------------------------

int GetCurTime(void)
{
	SYSTEMTIME time;
	int t;

	GetLocalTime(&time);
	
	t=(time.wYear+89)<<24|time.wMonth<<21|time.wDay<<16|
			time.wHour<<11|time.wMinute<<5|time.wSecond;
	
	return t;
}

void Unix2WinName(char *name)
{
	if(!name) return;
	while(*name)
	{
		if(name[0]=='/') name[0]='\\';
		name++;
	}
}

void Win2UnixName(char *name)
{
	if(!name) return;
	while(*name)
	{
		if(name[0]=='\\') name[0]='/';
		name++;
	}
}

void CreatePath(char *path)
{
	char *ofs;

	if(!path) return;
	for(ofs=path+1; *ofs; ofs++)
	{
		if(*ofs=='\\')
		{// create the directory
			*ofs=0;
			_mkdir(path);
			*ofs='\\';
		}
	}
}

// returns 1 if name has \ on the end
int isDir(char *name)
{
	if(!name || !name[0]) return 0;

	while(name[1]) name++;

	if(name[0]=='\\')
		return 1;
	else
		return 0;
}

// get file name from full name
char *getName(char *name)
{
	char *n=name;

	if(!name) return NULL;
	while(*name)
	{
		if(name[0]=='\\')
			n=name+1;
		name++;
	}
	return n;
}

void PrepareName(char *out, char *path, char *name, int sp)
{
	char n[MAX_PATH], *p;

	if(path)
		sprintf(n, "%s\\%s", path, sp?name:getName(name));
	else
		strcpy(n, sp?name:getName(name));
	Win2UnixName(n);

// remove leading slash
	p=(*n=='/')?n+1:n;

	if(strlen(p)>55) // Quake File name len FIXME: raise an error!
		out[0]='\0';
	else
		strcpy(out, p);
}

// ------------------------- * Devider * -------------------------

// open archive
void *PAK_Open(tOpenArchiveData *arch)
{
	archive_t *a=NULL;
	int n;

// find free arhive:
	for(n=0; n<2; n++)
	{
		if(!arh[n].in_use)
		{
			a=&arh[n];
			break;
		}
	}
	if(n==2)
	{
		arch->OpenResult=E_UNKNOWN_FORMAT;
		return NULL;
	}

// trying to open!
	arch->OpenResult=E_UNKNOWN_FORMAT; // CTAHDAPTHAR OTMA3KA [standart disclamer]!

	memset(a, 0, sizeof(archive_t));
	a->fp=fopen(arch->ArcName, "rb");
	if(!a->fp) return NULL;

	if(!fread(&a->header, sizeof(pakheader_t), 1, a->fp)) goto error;

// Signature check
	if(	a->header.magic[0]!='P' || a->header.magic[1]!='A' ||
			a->header.magic[2]!='C' || a->header.magic[3]!='K')
	{// Not a PAK file!
		goto error;
	}

	a->total=a->header.dirsize/sizeof(pakentry_t);

	a->entry=malloc(a->header.dirsize);
	if(!a->entry) goto error;

	fseek(a->fp, a->header.diroffset, SEEK_SET);
	if(!fread(a->entry, a->header.dirsize, 1, a->fp)) goto error;

// set vars
	strcpy(a->name, arch->ArcName);
	a->current=0;
	a->in_use=1;
	a->opentime=GetCurTime();
	a->mode=arch->OpenMode;
	arch->OpenResult=0;

	return a;
error:
	// free all & return error
	if(a->entry) free(a->entry);
	if(a->fp) fclose(a->fp);
	return NULL;
}

// close archive
int PAK_Close(archive_t *a)
{
	if(a)
	{
		if(a->entry) free(a->entry);
		if(a->fp) fclose(a->fp);
		a->in_use=0;
	}
	return 0; // OK! (even it is not)
}

void PAK_SetCallBackVol(archive_t *a, tChangeVolProc pChangeVolProc1)
{
	if((int)a==-1)
		ppVol=pChangeVolProc1; // for Packing/Deleting
	else
		a->pVol=pChangeVolProc1;
}

void PAK_SetCallBackProc(archive_t *a, tProcessDataProc pProcessDataProc)
{
	if((int)a==-1)
		ppProc=pProcessDataProc; // for Packing/Deleting
	else
		a->pProc=pProcessDataProc;
}

// process header!
int PAK_NextItem(archive_t *a, tHeaderData *item)
{
	if(a->current>=a->total)
		return E_END_ARCHIVE;
	strcpy(item->ArcName, a->name);
	strcpy(item->FileName, a->entry[a->current].filename);	// FileName
	Unix2WinName(item->FileName);
	item->PackSize=item->UnpSize=a->entry[a->current].size;	// PackSize & UnpSize
	item->FileTime=a->opentime;															// FileTime
	item->FileCRC=0;
	item->FileAttr=0x20;
	
	a->current++;
	return 0;
}

#define BUF_SIZE 128*1024 // 128 KB buffer
static char data[BUF_SIZE];

// process every file!
int PAK_Process(archive_t *a, int Operation, char *DestPath, char *DestName)
{
	char n[MAX_PATH], *name;
	int size, c_size;
	pakentry_t *e;
	FILE *fp;

	if(Operation==PK_SKIP || Operation==PK_TEST	|| a->mode==PK_OM_LIST)
		return 0;

	if(a->current>0 && a->current<=a->total)
		e=&a->entry[a->current-1];
	else
		return E_BAD_DATA;

	if(DestPath==NULL)
		name=DestName;
	else
		sprintf(name=n, "%s\\%s", DestPath, DestName);
	CreatePath(name);
	fp=fopen(name, "wb");
	if(!fp) return E_EWRITE;
	fseek(a->fp, e->offset, SEEK_SET);
	size=e->size;
	while(size>0)
	{
		c_size=(size>BUF_SIZE)?BUF_SIZE:size;
		fread	(data, c_size, 1, a->fp);
		fwrite(data, c_size, 1, fp);
		a->pProc(name, c_size);
		size-=c_size;
	}
	fclose(fp);
				
	return 0;
}

// ------------------------- * optional * -------------------------

int cpy(FILE *target, FILE *src, char *name)
{
	int size, s, c_size;

	fseek(src, 0, SEEK_END);
	size=s=ftell(src);
	fseek(src, 0, SEEK_SET);

	while(size>0)
	{
		c_size=(size>BUF_SIZE)?BUF_SIZE:size;
		fread	(data, c_size, 1, src);
		fwrite(data, c_size, 1, target);
		ppProc(name, c_size);
		size-=c_size;
	}
	return s;
}

// returns 1 if file exists, 0 otherwise
int File_Exist(char *name)
{
	FILE *fp;

	fp=fopen(name, "rb");
	if(!fp) return 0;
	fclose(fp);
	return 1;
}

#define MAX_FILES_IN_PACK 2048
#define MAX_DIRS 1024

char *files[MAX_FILES_IN_PACK];
char *dirs[MAX_DIRS];
int totalfiles, totaldirs;

int PAK_Pack(char *name, char *spath, char *path, char *AddList, int flags)
{
	char fname[MAX_PATH];
	pakheader_t header;
	pakentry_t *entry;
	int n, offset;
	FILE *fp, *fp1;

	totalfiles=totaldirs=0;

// create file list & calculate amount of files to be packed
	while(*AddList)
	{
		if(!isDir(AddList))
		{
			files[totalfiles++]=AddList;
			if(totalfiles==MAX_FILES_IN_PACK) return E_TOO_MANY_FILES;
		}
		else
		{
			dirs[totaldirs++]=AddList;
			if(totaldirs==MAX_DIRS) return E_TOO_MANY_FILES;
		}
		while(*AddList++);
	}

	if(!totalfiles)
		return E_NO_FILES;
	
// are we adding to existing archive?
	if(File_Exist(name))
		return PAK_Add(name, spath, path, AddList, flags);

// prepare header
	header.magic[0]='P';
	header.magic[1]='A';
	header.magic[2]='C';
	header.magic[3]='K';
	header.dirsize=sizeof(pakentry_t)*totalfiles;

	fp=fopen(name, "wb");
	if(!fp) return E_ECREATE;
	
	entry=malloc(header.dirsize);
	if(!entry) {fclose(fp); return E_NO_MEMORY;}
	memset(entry, 0, header.dirsize);

	fwrite(&header, sizeof(header), 1, fp);
	offset=sizeof(header);

	for(n=0; n<totalfiles; n++)
	{
		entry[n].offset=offset;
		PrepareName(entry[n].filename, spath, files[n], flags&PK_PACK_SAVE_PATHS);
		strlwr(entry[n].filename);
		if(!entry[n].filename[0]) continue;
		if(path)
			sprintf(fname, "%s%s", path, files[n]);
		else
			strcpy(fname, files[n]);
		fp1=fopen(fname, "rb");
		if(!fp1) continue;//{fclose(fp); free(entry); return E_EOPEN;}
		entry[n].size=cpy(fp, fp1, fname);
		fclose(fp1);
// move to archive
		if(flags&PK_PACK_MOVE_FILES) remove(fname);
		offset+=entry[n].size;
	}
	header.diroffset=offset;
	fwrite(entry, header.dirsize, 1, fp);
	fseek(fp, 0, SEEK_SET);
	fwrite(&header, sizeof(header), 1, fp);
	fclose(fp);
// delete all dirs
	if(flags&PK_PACK_MOVE_FILES)
	{
		for(n=totaldirs-1; n>=0; n--)
			_rmdir(dirs[n]);
	}

	return 0;
}

int cmp(pakentry_t const *e1, pakentry_t const *e2)
{
	return e1->offset-e2->offset;
}

void move(FILE *fp, int pos_write, int pos_read, int size)
{
	int c_size, n=0;

	while(size>0)
	{
		c_size=(size>BUF_SIZE)?BUF_SIZE:size;
		if(c_size>pos_read-pos_write)
			c_size=pos_read-pos_write;

		fseek(fp, pos_read, SEEK_SET);
		fread(data, c_size, 1, fp);
		fseek(fp, pos_write, SEEK_SET);
		fwrite(data, c_size, 1, fp);

		pos_read +=c_size;
		pos_write+=c_size;

		size-=c_size;
	}
}

// ------------------------- * Change PACK * -------------------------
pakheader_t header;
pakentry_t *entry;
int total;

// open pack, read data & sort by offset
int PAK_OpenChange(char *name, int nfiles)
{
	FILE *pak;

	pak=fopen(name, "rb");
	if(!pak) return E_EOPEN;
	if(!fread(&header, sizeof(header), 1, pak))
	{
		fclose(pak);
		return E_EREAD;
	}

// Signature check
	if(	header.magic[0]!='P' || header.magic[1]!='A' ||
			header.magic[2]!='C' || header.magic[3]!='K')
	{// Not a PAK file!
		fclose(pak);
		return E_UNKNOWN_FORMAT;
	}
	total=header.dirsize/sizeof(pakentry_t);

	entry=malloc(header.dirsize+nfiles*sizeof(pakentry_t));
	if(!entry)
	{
		fclose(pak);
		return E_NO_MEMORY;
	}
	fseek(pak, header.diroffset, SEEK_SET);
	if(!fread(entry, header.dirsize, 1, pak))
	{
		fclose(pak);
		free(entry);
		return E_EREAD;
	}
	fclose(pak);

// sort entrys by offset
	qsort(entry, total, sizeof(pakentry_t), cmp);
	
	return 0;
}

int PAK_Add(char *name, char *spath, char *path, char *AddList, int flags)
{
	char fname[MAX_PATH], pname[56], a[256];
	int n, m, offset, retval;
	FILE *pak, *fp;

// file list is allready created by PAK_Pack function

	retval=PAK_OpenChange(name, totalfiles);
	if(retval) return retval;

// check for overwrite, if so, delete overwritten entry
	for(n=0; n<totalfiles; n++)
	{
		PrepareName(pname, spath, files[n], flags&PK_PACK_SAVE_PATHS);
		strlwr(entry[n].filename);
		if(!pname[0]) continue;
		for(m=0; m<total; m++)
			if(!strcmp(entry[m].filename, pname))
		{
			memmove(&entry[m], &entry[m+1], sizeof(pakentry_t)*(total-m-1));
			total--; m--;
		}
	}

// prepare PAK file
	pak=fopen(name, "r+b");
	if(!pak)
	{
		free(entry);
		return E_EOPEN;
	}

	offset=sizeof(header); // statring offset

	for(n=0; n<total; n++)
	{
// if offsets do not match move entry
		if(offset!=entry[n].offset)
		{
			move(pak, offset, entry[n].offset, entry[n].size);
			entry[n].offset=offset;

// notify WinCmd with a progress
			sprintf(a, "seeking [%d%%] %s", 100*n/total, entry[n].filename);
			ppProc(a, 0);
		}
		offset+=entry[n].size;
	}

// pack Additional files [Add]
	for(n=0; n<totalfiles; n++)
	{
		entry[total].offset=offset;
		PrepareName(entry[total].filename, spath, files[n], flags&PK_PACK_SAVE_PATHS);
		strlwr(entry[n].filename);
		if(!entry[total].filename[0]) continue;
		if(path)
			sprintf(fname, "%s%s", path, files[n]);
		else
			strcpy(fname, files[n]);
		fp=fopen(fname, "rb");
		if(!fp) continue;//{fclose(pak); free(entry); return E_EOPEN;}
		fseek(pak, offset, SEEK_SET);
		entry[total].size=cpy(pak, fp, fname);
		fclose(fp);
// move to archive
		if(flags&PK_PACK_MOVE_FILES) remove(fname);
		offset+=entry[total].size;
		total++;
	}

	header.dirsize=total*sizeof(pakentry_t);
	header.diroffset=offset;
	fseek(pak, 0, SEEK_SET);
	fwrite(&header, sizeof(header), 1, pak);
	fseek(pak, offset, SEEK_SET);
	fwrite(entry, header.dirsize, 1, pak);
	offset+=header.dirsize;
	chsize(fileno(pak), offset);

	fclose(pak);
	free(entry);

// delete all dirs
	if(flags&PK_PACK_MOVE_FILES)
	{
		for(n=totaldirs-1; n>=0; n--)
			_rmdir(dirs[n]);
	}

	return 0;
}

// Delete entries
int PAK_Delete(char *name, char *DelList)
{
	char pname[56], a[256];
	FILE *pak;
	int n, m, offset, retval;

	totalfiles=totaldirs=0;

// create file list & calculate amount of files to be packed
	while(*DelList)
	{
		if(!isDir(DelList))
		{
			files[totalfiles++]=DelList;
			if(totalfiles==MAX_FILES_IN_PACK) return E_TOO_MANY_FILES;
		}
		while(*DelList++);
	}

	if(!totalfiles)
		return E_NO_FILES;

	retval=PAK_OpenChange(name, 0);
	if(retval) return retval;

// check for overwrite
	for(n=0; n<totalfiles; n++)
	{
		PrepareName(pname, NULL, files[n], 1);
		if(!pname[0]) continue;
		for(m=0; m<total; m++)
			if(!strcmp(entry[m].filename, pname))
		{
			memmove(&entry[m], &entry[m+1], sizeof(pakentry_t)*(total-m-1));
			total--; m--;
		}
	}

// prepare PAK file
	pak=fopen(name, "r+b");
	if(!pak)
	{
		free(entry);
		return E_EOPEN;
	}
	offset=sizeof(header);
	for(n=0; n<total; n++)
	{
// if offsets do not match move entry
		if(offset!=entry[n].offset)
		{
			move(pak, offset, entry[n].offset, entry[n].size);
			entry[n].offset=offset;

// notify WinCmd with a progress
			sprintf(a, "seeking [%d%%] %s", 100*n/total, entry[n].filename);
			ppProc(a, 0);
		}
		offset+=entry[n].size;
	}

	header.dirsize=total*sizeof(pakentry_t);
	header.diroffset=offset;
	fseek(pak, 0, SEEK_SET);
	fwrite(&header, sizeof(header), 1, pak);
	fseek(pak, offset, SEEK_SET);
	fwrite(entry, header.dirsize, 1, pak);
	offset+=header.dirsize;
	chsize(fileno(pak), offset);

	fclose(pak);
	free(entry);
	return 0;
}
