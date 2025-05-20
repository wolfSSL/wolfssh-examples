/* myFilesystem.h
 *
 * Copyright (C) 2014-2025 wolfSSL Inc.
 *
 * This file is part of wolfSSH.
 *
 * wolfSSH is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * wolfSSH is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with wolfSSH.  If not, see <http://www.gnu.org/licenses/>.
 */


/*
 * The port module wraps standard C library functions with macros to
 * cover portability issues when building in environments that rename
 * those functions. This module also provides local versions of some
 * standard C library functions that are missing on some platforms.
 */


#ifndef _WOLFSSH_USER_PORT_H_
#define _WOLFSSH_USER_PORT_H_

#include <wolfssh/settings.h>
#include <wolfssh/log.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include "system/fs/sys_fs.h"

/*******************************************************************************
 "SAFE" function implementations any user is ok
*******************************************************************************/
static inline int wDirOpen(void* heap, WDIR* dir, const char* path)
{
    *dir = SYS_FS_DirOpen(path);
    if (*dir == SYS_FS_HANDLE_INVALID) {
        return -1;
    }
    return 0;
}

static inline int wStat(const char* path, WSTAT_T* stat)
{
    int ret;

    WMEMSET(stat, 0, sizeof(WSTAT_T));
    ret = SYS_FS_FileStat(path, stat);

    if (ret != SYS_FS_RES_SUCCESS) {
        WLOG(WS_LOG_SFTP,
            "Return from SYS_FS_fileStat [%s] = %d, expecting %d",
            path, ret, SYS_FS_RES_SUCCESS);
        WLOG(WS_LOG_SFTP, "SYS error reason = %d", SYS_FS_Error());
        return -1;
    }
    else {
        return 0;
    }
    return 0;
}

static inline char *ff_getcwd(char *r, int rSz)
{
    SYS_FS_RESULT ret;
    ret = SYS_FS_CurrentWorkingDirectoryGet(r, rSz);
    if (ret != SYS_FS_RES_SUCCESS) {
        return r;
    }
    return r;
}


int wfopen(WFILE* f, const char* filename, SYS_FS_FILE_OPEN_ATTRIBUTES mode)
{
    if (f != NULL) {
        *f = SYS_FS_FileOpen(filename, mode);
        if (*f == WBADFILE) {
            WLOG(WS_LOG_SFTP, "Failed to open file %s", filename);
            return 1;
        }
        else {
            WLOG(WS_LOG_SFTP, "Opened file %s", filename);
            return 0;
        }
    }
    return 1;
}

static int wPread(WFD fd, unsigned char* buf, unsigned int sz,
        const unsigned int* shortOffset)
{
    int ret;

    ret = (int)WFSEEK(NULL, &fd, shortOffset[0], SYS_FS_SEEK_SET);
    if (ret != -1)
        ret = (int)WFREAD(NULL, buf, 1, sz, &fd);

    return ret;
}


#define WFD SYS_FS_HANDLE
int wPwrite(WFD, unsigned char*, unsigned int, const unsigned int*);
int wPread(WFD, unsigned char*, unsigned int, const unsigned int*);

/*******************************************************************************
 Restricted function implementations
*******************************************************************************/

/* helper function to check if the user is allowed to do an operation */
static int isUserAllowed(void* fs)
{
    char* currentUser;
    WOLFSSH* ssh = (WOLFSSH*)fs;

    if (ssh == NULL) {
        return 0;
    }

    currentUser = wolfSSH_GetUsername(ssh);
    if (currentUser && XSTRCMP(currentUser, "admin")) {
        return 1;
    }
    return 0;
}


static inline wFwrite(void *fs, unsigned char* b, int s, int a, WFILE* f)
{
    if (isUserAllowed(fs)) {
        return SYS_FS_FileWrite(*f, b, s * a);
    }
    else {
        return -1;
    }
}


static int wChmod(void* fs, const char* path, int mode)
{
    SYS_FS_RESULT ret;
    SYS_FS_FILE_DIR_ATTR attr = 0;

    if (isUserAllowed(fs)) {
        /* mode is the octal value i.e 666 is 0x1B6 */
        if ((mode & 0x180) != 0x180) { /* not octal 6XX read only */
            attr |= SYS_FS_ATTR_RDO;
        }

        /* toggle the read only attribute */
        ret = SYS_FS_FileDirectoryModeSet(path, attr, SYS_FS_ATTR_RDO);
        if (ret != SYS_FS_RES_SUCCESS) {
            return -1;
        }
        return 0;
    }
    else {
        return -1;
    }
}


static int wPwrite(void* fs, WFD fd, unsigned char* buf, unsigned int sz,
        const unsigned int* shortOffset)
{
    int ret = -1;

    if (isUserAllowed(fs)) {
        ret = (int)WFSEEK(NULL, &fd, shortOffset[0], SYS_FS_SEEK_SET);
        if (ret != -1) {
            ret = (int)WFWRITE(NULL, buf, 1, sz, &fd);
        }
    }

    return ret;
}

static int wMkdir(void* fs, unsigned char* path)
{
    if (isUserAllowed(fs)) {
        return SYS_FS_DirectoryMake(path);
    }
    else {
        return -1;
    }
}


static int wRmdir(void* fs, unsigned char* dir)
{
    if (isUserAllowed(fs)) {
        return SYS_FS_FileDirectoryRemove(dir);
    }
    else {
        return -1;
    }
}

static int wRemove(void* fs, unsigned char* dir)
{
    if (isUserAllowed(fs)) {
        return SYS_FS_FileDirectoryRemove(dir);
    }
    else {
        return -1;
    }
}


static int wRename(void* fs, unsigned char* orig, unsigned char* newName)
{
    if (isUserAllowed(fs)) {
        return SYS_FS_FileDirectoryRenameMove(orig, newName);
    }
    else {
        return -1;
    }
}


/*******************************************************************************
 mapping of file handles and modes
*******************************************************************************/
#define WDIR              SYS_FS_HANDLE
#define WSTAT_T           SYS_FS_FSTAT
#define WS_DELIM          '/'
#define WFFLUSH(s)        SYS_FS_FileSync((s))
#define WFILE             SYS_FS_HANDLE
#define WSEEK_END         SYS_FS_SEEK_END
#define WBADFILE          SYS_FS_HANDLE_INVALID
#define WOLFSSH_O_RDWR    SYS_FS_FILE_OPEN_READ_PLUS
#define WOLFSSH_O_RDONLY  SYS_FS_FILE_OPEN_READ
#define WOLFSSH_O_WRONLY  SYS_FS_FILE_OPEN_WRITE_PLUS
#define WOLFSSH_O_APPEND  SYS_FS_FILE_OPEN_APPEND
#define WOLFSSH_O_CREAT   SYS_FS_FILE_OPEN_WRITE_PLUS
#define WOLFSSH_O_TRUNC   0
#define WOLFSSH_O_EXCL    0
#define FLUSH_STD(a)


/*******************************************************************************
 mapping "SAFE" operations, any user can do
*******************************************************************************/
#define WFOPEN(fs,f,fn,m)   wfopen(*(f),(fn),(m))
#define WFCLOSE(fs,f)       SYS_FS_FileClose(*(f))
#define WFREAD(fs,b,s,a,f)  SYS_FS_FileRead(*(f),(b),(s)*(a))
#define WFSEEK(fs,s,o,w)    SYS_FS_FileSeek(*(s),(o),(w))
#define WFTELL(fs,s)        SYS_FS_FileTell(*(s))
#define WREWIND(fs,s)       SYS_FS_FileSeek(*(s), 0, SYS_FS_SEEK_SET)
#define WCHDIR(fs,b)        SYS_FS_DirectryChange((b))
#define WOPENDIR(fs,h,c,d)  wDirOpen((h), (c),(d))
#define WCLOSEDIR(fs,d)     SYS_FS_DirClose(*(d))
#define WSTAT(fs,p,b)       wStat((p), (b))
#define WPREAD(fs,fd,b,s,o) wPread((fd),(b),(s),(o))
#define WGETCWD(fs,r,rSz)   ff_getcwd(r,(rSz))

/*******************************************************************************
 mapping of operations that have a user check before ran
*******************************************************************************/
#define WFWRITE(fs,b,s,a,f)  wFwrite((fs),(b),(s),(a),(f))
#define WCHMOD(fs,f,m)       wChmod((fs),(f),(m))
#define WMKDIR(fs,p,m)       wMkdir((fs,(p))
#define WRMDIR(fs,d)         wRmdir((fs),(d))
#define WREMOVE(fs,d)        wRemove((fs),(d))
#define WRENAME(fs,o,n)      wRename((fs),(o),(n))
#define WPWRITE(fs,fd,b,s,o) wPwrite((fd),(b),(s),(o))


/*******************************************************************************
 FPUTS/FGETS only used in SFTP client example
*******************************************************************************/
#undef  WFGETS
#define WFGETS(b,s,f)       SYS_FS_FileStringGet((f), (b), (s))
#undef  WFPUTS
#define WFPUTS(b,f)         SYS_FS_FileStringPut((f), (b))


/*******************************************************************************
 Operations that do not have a port for
*******************************************************************************/
#define WUTIMES(a,b)         (0)
#define WSETTIME(fs,f,a,m)   (0)
#define WFSETTIME(fs,fd,a,m) (0)
#define WFCHMOD(fs,fd,m)     (0)


#ifdef __cplusplus
}
#endif

#endif /* _WOLFSSH_USER_PORT_H_ */

