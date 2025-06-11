/* myFilesystem.c
 *
 * Copyright (C) 2014-2025 wolfSSL Inc.
 *
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

#include "myFilesystem.h"
#include <wolfssh/ssh.h>
#include <wolfssh/log.h>
#include <stdlib.h>
#include <stdio.h>
#include "system/fs/sys_fs.h"

#ifdef WOLFSSH_USER_FILESYSTEM
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
    if (currentUser && XSTRCMP(currentUser, "admin") == 0) {
        return 1;
    }
    return 0;
}


int wFwrite(void *fs, unsigned char* b, int s, int a, WFILE* f)
{
    if (isUserAllowed(fs)) {
        return SYS_FS_FileWrite(*f, b, s * a);
    }
    else {
        return -1;
    }
}


int wChmod(void* fs, const char* path, int mode)
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


int wPwrite(void* fs, WFD fd, unsigned char* buf, unsigned int sz,
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

int wMkdir(void* fs, unsigned char* path)
{
    if (isUserAllowed(fs)) {
        return SYS_FS_DirectoryMake(path);
    }
    else {
        return -1;
    }
}


int wRmdir(void* fs, unsigned char* dir)
{
    if (isUserAllowed(fs)) {
        return SYS_FS_FileDirectoryRemove(dir);
    }
    else {
        return -1;
    }
}

int wRemove(void* fs, unsigned char* dir)
{
    if (isUserAllowed(fs)) {
        return SYS_FS_FileDirectoryRemove(dir);
    }
    else {
        return -1;
    }
}


int wRename(void* fs, unsigned char* orig, unsigned char* newName)
{
    if (isUserAllowed(fs)) {
        return SYS_FS_FileDirectoryRenameMove(orig, newName);
    }
    else {
        return -1;
    }
}


/*******************************************************************************
 "SAFE" function implementations any user is ok
*******************************************************************************/
int wDirOpen(void* heap, WDIR* dir, const char* path)
{
    *dir = SYS_FS_DirOpen(path);
    if (*dir == SYS_FS_HANDLE_INVALID) {
        return -1;
    }
    return 0;
}

int wStat(const char* path, WSTAT_T* stat)
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

char* wGetCwd(char *r, int rSz)
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


int wPread(WFD fd, unsigned char* buf, unsigned int sz,
        const unsigned int* shortOffset)
{
    int ret;

    ret = (int)WFSEEK(NULL, &fd, shortOffset[0], SYS_FS_SEEK_SET);
    if (ret != -1)
        ret = (int)WFREAD(NULL, buf, 1, sz, &fd);

    return ret;
}

#endif /* WOLFSSH_USER_FILESYSTEM */