/*
 *	table files 
 * Copyright (c) 2023-2024 senllang
 * 
 * toadb is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 * http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
*/
#define _LARGEFILE64_SOURCE     /* See feature_test_macros(7) */

#include "tfile.h"
#include "buffer.h"
#include "public.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>


#include <sys/types.h>
#include <unistd.h>

/* file operator */
#include <fcntl.h>           /* Definition of AT_* constants */



extern char *DataDir;


#if defined(WIN32) || defined(_WIN32) || defined(_WIN32_) || defined(WIN64) || defined(_WIN64) || defined(_WIN64_)

static int FileExist(char *filepath) 
{
	FILE *file = NULL;
	errno_t err;

	err = fopen_s(&file, filepath, "r"); // 尝试以只读模式打开文件  
	if (err == 0)
	{
		// 文件存在，且成功打开  
		// 关闭文件  
		fclose(file);
		return 1;
	}
	else 
	{
		// 文件不存在或无法打开  
		return 0;
	}
}

PFileHandle CreateFile(char *filename, int mode)
{
	PFileHandle pfh = NULL;
	FILE *fd = NULL;
	char filepath[FILE_PATH_MAX_LEN] = { 0 };
	errno_t err;

	snprintf(filepath, FILE_PATH_MAX_LEN, "%s/%s", DataDir, filename);
	hat_debug("Debug: opentable file path:%s \n", filepath);

	// 检查文件是否存在
	if (FileExist(filepath) > 0)
	{
		hat_log("table file %s already exist. err[%d]\n", filepath, errno);
		return NULL;
	}

	// 以二进制形式打开文件
	err = fopen_s(&fd, filepath, "wb+");
	if (err != 0)
	{
		hat_log("create file %s error, maybe space not enough.errno[%d]\n", filepath, errno);
		return NULL;
	}

	pfh = (PFileHandle)AllocMem(sizeof(FileHandle));
	pfh->fp = fd;

	return pfh;
}

PFileHandle OpenFile(char *filename, int mode)
{
	PFileHandle pfh = NULL;
	FILE * fd = NULL;
	char filepath[FILE_PATH_MAX_LEN];
	errno_t err;

	snprintf(filepath, FILE_PATH_MAX_LEN, "%s/%s", DataDir, filename);
	hat_debug("Debug: opentable file path:%s \n", filepath);

	// 以二进制形式打开文件
	err = fopen_s(&fd, filepath,"rb+");
	if (err != 0)
	{
		hat_log("open file %s error, errno[%d]\n", filepath, errno);
		return pfh;
	}

	pfh = (PFileHandle)AllocMem(sizeof(FileHandle));
	pfh->fp = fd;

	return pfh;
}

int DeleteTableFile(char *filename)
{
	int ret = 0;
	char filepath[FILE_PATH_MAX_LEN];

	snprintf(filepath, FILE_PATH_MAX_LEN, "%s/%s", DataDir, filename);

	// 检查文件是否存在
	if (FileExist(filepath) == 0)
	{
		hat_log("table file %s is not exist. \n", filepath);
		return -1;
	}

	// 删除文件
	ret = remove(filepath);
	if (ret != 0)
	{
		hat_log("unlink file %s ,errno %d \n", filepath, errno);
		return -1;
	}

	return ret;
}


char* FileReadPage(PFileHandle fd, int offset, int size)
{
	char *page = NULL;
	int readSize = 0;
	int filelen = 0;

	if (fd == NULL || fd->fp != NULL)
		return NULL;

	filelen = fseek(fd->fp, 0, SEEK_END);
	if (offset >= filelen)
	{
		hat_error("read offset %d oversize file len %d\n", offset, filelen);
		return NULL;
	}

	page = (char *)AllocMem(size);

	/* read page */
	fseek(fd->fp, offset, SEEK_SET);
	readSize = fread((char *)page, size, 1, fd->fp);

	if (readSize < size)
	{
		FreeMem(page);
		return NULL;
	}

	return page;
}

int FileWritePage(PFileHandle fd, int offset, int size, char *pageBuf)
{
	int writeSize = 0;
	int filelen = 0;

	if (fd == NULL || fd->fp != NULL)
		return -1;

	filelen = fseek(fd->fp, 0, SEEK_END);
	if (offset >= filelen)
	{
		hat_error("write offset %d oversize file len %d\n", offset, filelen);
		return -1;
	}

	/* read page */
	fseek(fd->fp, offset, SEEK_SET);
	writeSize = fwrite(pageBuf, size, 1, fd->fp);
	if (writeSize != size)
	{
		hat_log("table is extened failure, maybe is not enough space.\n");
		return -1;
	}

	return writeSize;
}

int FileSync(PFileHandle fd)
{
	if (fd != NULL && fd->fp != NULL)
		fflush(fd->fp);
	return 0;
}

int FileClose(PFileHandle fd)
{
	if (fd != NULL && fd->fp != NULL)
	{
		fclose(fd->fp);
		FreeMem(fd);
	}
	return 0;
}

#elif defined(__linux__)

PFileHandle CreateFile(char *filename, int mode)
{
    PFileHandle pfh = NULL;
    int fd = -1;
    char filepath[FILE_PATH_MAX_LEN] = {0};

    snprintf(filepath, FILE_PATH_MAX_LEN, "%s/%s", DataDir, filename);
    hat_debug("Debug: opentable file path:%s \n", filepath);

    // 检查文件是否存在
    if (access(filepath, F_OK) == 0) 
    {
        hat_log("table file %s already exist. err[%d]\n", filepath, errno); 
        return NULL;
    }

    // 以二进制形式打开文件
    fd = open(filepath, O_RDWR | O_CREAT, mode);
    if (fd == -1) 
    {
        hat_log("create file %s error, maybe space not enough.errno[%d]\n", filepath, errno);
        return NULL;
    }

    pfh = (PFileHandle)AllocMem(sizeof(FileHandle));
    pfh->fd = fd;

    return pfh;
}

PFileHandle OpenFile(char *filename, int mode)
{
    PFileHandle pfh = NULL;
    int fd = -1;
    char filepath[FILE_PATH_MAX_LEN];
    int err = 0;

    snprintf(filepath, FILE_PATH_MAX_LEN, "%s/%s", DataDir, filename);
    hat_debug("Debug: opentable file path:%s \n", filepath);

    // 检查文件是否存在
    if (access(filepath, F_OK) != 0) 
    {
        hat_log("table file %s is not exist. \n", filepath);
        err = errno;
        return pfh;
    }

    // 以二进制形式打开文件
    fd = open(filepath, O_RDWR, mode);
    if (fd == -1) 
    {
        hat_log("open file %s error, errno[%d]\n", filepath, errno);
        return pfh;
    }

    pfh = (PFileHandle)AllocMem(sizeof(FileHandle));
    pfh->fd = fd;

    return pfh;
}

int DeleteTableFile(char *filename)
{
    int ret = 0;
    char filepath[FILE_PATH_MAX_LEN];

    snprintf(filepath, FILE_PATH_MAX_LEN, "%s/%s", DataDir, filename);

    // 检查文件是否存在
    if (access(filepath, F_OK) != 0) 
    {
        hat_log("table file %s is not exist. \n", filepath);
        return -1;
    }

    // 删除文件
    ret = unlink(filepath);
    if (ret != 0) 
    {
        hat_log("unlink file %s ,errno %d \n", filepath, errno);
        return -1;
    }

    return ret;
}


int FileReadPage(PFileHandle fd, char *page, INT64 offset, int size)
{
    int readSize = 0;
	INT64 realLen = 0;

    if(fd == NULL || fd->fd < 0)
        return -1;

    /* read page */
    realLen = lseek64(fd->fd, offset, SEEK_SET);
	if(realLen < 0)
	{
		if(errno == ENXIO)
		{
			hat_error("required offset %ld is overfile size.\n", offset);
		}

		return -1;
	}

    readSize = read(fd->fd, (char *)page, size);

    return readSize;
}

int FileWritePage(PFileHandle fd, INT64 offset, int size, char *pageBuf)
{
    int writeSize = 0;
    int filelen = 0;

    if(fd == NULL || fd->fd < 0)
        return -1;

    /* read page */
    lseek64(fd->fd, offset, SEEK_SET);

    writeSize = write(fd->fd, pageBuf, size);
    if (writeSize != size)
    {
        hat_log("table is extened failure, maybe is not enough space.\n");
        return -1;
    }

    return writeSize;
}

int FileSync(PFileHandle fd)
{
    if(fd != NULL && fd->fd > 0)
        fsync(fd->fd);
    return 0;
}

int FileClose(PFileHandle fd)
{
    if(fd != NULL && fd->fd > 0)
    {
        close(fd->fd);
        FreeMem(fd);
    }
    return 0;
}

#endif