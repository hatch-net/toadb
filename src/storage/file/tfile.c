/*
 *	table files 
 * Copyright (C) 2023-2023, senllang
*/

#include "tfile.h"
#include "buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/types.h>
#include <unistd.h>

/* file operator */
#include <fcntl.h>           /* Definition of AT_* constants */


#define hat_log printf 
#define debug  

/* configure param */
int config_fsync = 0;

extern char *DataDir;

int CreateTableFile(char *filename, int mode)
{
    int fd = -1;
    char filepath[1024] = {0};

    snprintf(filepath, 1024, "%s/%s", DataDir, filename);
    debug("Debug: opentable file path:%s \n", filepath);

    // 检查文件是否存在
    if (access(filepath, F_OK) == 0) 
    {
        hat_log("table file %s already exist. err[%d]\n", filepath, errno); 
        return -1;
    }

    // 以二进制形式打开文件
    fd = open(filepath, O_RDWR | O_CREAT, mode);
    if (fd == -1) 
    {
        hat_log("create file %s error, maybe space not enough.errno[%d]\n", filepath, errno);
        return -2;
    }

    return fd;
}

int OpenTableFile(char *filename, int mode)
{
    int fd;
    char filepath[1024];
    int err = 0;

    snprintf(filepath, 1024, "%s/%s", DataDir, filename);
    debug("Debug: opentable file path:%s \n", filepath);

    // 检查文件是否存在
    if (access(filepath, F_OK) != 0) 
    {
        // hat_log("table file %s is not exist. \n", filepath);
        err = errno;
        return -1 * err;
    }

    // 以二进制形式打开文件
    fd = open(filepath, O_RDWR, mode);
    if (fd == -1) 
    {
        hat_log("open file %s error, errno[%d]\n", filepath, errno);
        return -2;
    }

    return fd;
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

    // 以二进制形式打开文件
    ret = unlink(filepath);
    if (ret != 0) 
    {
        hat_log("unlink file %s ,errno %d \n", filepath, errno);
        return -1;
    }

    return ret;
}

int smgrOpen(char *filename)
{
    int fd = OpenTableFile(filename, 0666);
}

int smgrFlush(int fd, char *buffer, int offnum)
{
    int ret = 0;

    if(fd <= 0)
    {
        hat_log("table file not open\n");
        return -1;
    }

    lseek(fd, (offnum-1)*PAGE_MAX_SIZE, SEEK_SET);
    ret = write(fd, buffer, PAGE_MAX_SIZE);

    if(config_fsync)
        ret |= fsync(fd);

    return ret;
}

int smgrClose(int fd)
{
    int ret = 0;
    if(config_fsync)
        ret |= fsync(fd);
        
    close(fd);

    return ret;
}

int smgrRelease(PsgmrInfo sgmrInfo)
{
    PVFVec head = sgmrInfo->vfhead;
    PVFVec tpos = (PVFVec)head->list.next;

    while(tpos != NULL)
    {
        if(tpos == head)
            break;

        DeleteVF(head, tpos);
        smgrClose(tpos->fd);
        FreeMem(tpos);
        tpos = NULL;
        
        tpos = (PVFVec)head->list.next;
    }

    if(tpos != NULL)
    {
        smgrClose(tpos->fd);
        FreeMem(tpos);
        tpos = NULL;
    }

    FreeMem(sgmrInfo);

    return 0;
}

char* smgrReadPage(int fd, int offset, int size)
{
    char *page = NULL;
    int readSize = 0;
    int filelen = 0;

    filelen = lseek(fd, 0, SEEK_END);
    if(offset >= filelen)
    {
        return NULL;
    }

    page = (char *)AllocMem(size);

    /* read page */
    lseek(fd, offset, SEEK_SET);
    readSize = read(fd, (char *)page, size);

    if(readSize < size)
    {
        FreeMem(page);
        return NULL;
    }

    return page;
}

static int objId = 1;
void SetObjectId(int id)
{
    if(id >= objId)
        objId = id + 1;
    return ;
}

int GetObjectId()
{
    return objId++;
}

PVFVec SearchVF(PVFVec head, ForkType forknum)
{
    PVFVec vpos = NULL;
    PVFVec tpos = head;
    while(tpos != NULL)
    {
        if(tpos->forkNum == forknum)
        {
            vpos = tpos;
            break;
        }

        tpos = (PVFVec)tpos->list.next;
        
        if(tpos == head)
            break;
    }

    return vpos;
}

PVFVec DeleteVF(PVFVec head, PVFVec node)
{
    DelDListNode((PDList*)&(head->list), &(node->list));

    return node;
}

PVFVec smgr_open(PsgmrInfo smgrInfo, char *fileName, ForkType forkNum)
{
    PVFVec vpos = NULL;
    char fname[FILE_PATH_MAX_LEN] = {0};
    int newnode = 0;

    if(NULL == smgrInfo->vfhead)
    {
        smgrInfo->vfhead = (PVFVec)AllocMem(sizeof(VFVec));
        INIT_DLIST_NODE(smgrInfo->vfhead->list);
        smgrInfo->vfhead->forkNum = forkNum;
        smgrInfo->vfhead->fd = 0;

        vpos = smgrInfo->vfhead;
    }
    
    /* read info */
    if(NULL == vpos)
    {
        vpos = SearchVF(smgrInfo->vfhead, forkNum);

        if(NULL == vpos)
        {
            vpos = (PVFVec)AllocMem(sizeof(VFVec));
            INIT_DLIST_NODE(smgrInfo->vfhead->list);
            vpos->forkNum = forkNum;
            vpos->fd = 0;
            newnode = 1;
        }
    }

    if(vpos->fd > 0)
        return vpos;

    switch(forkNum)
    {
        case MAIN_FORK:
            snprintf(fname, FILE_PATH_MAX_LEN, "%s", fileName);
            break;
        case GROUP_FORK:
            snprintf(fname, FILE_PATH_MAX_LEN, "%s%s", GROUP_FILE_PRE, fileName);
            break;
        default:
            return NULL;
        break;
    }

    vpos->fd = OpenTableFile(fname, 0666);
    if(vpos->fd < 0)
    {
        // hat_log("open file %s failure.\n", fname);
        FreeMem(vpos);
        return NULL;
    }

    if(newnode)
        AddDListTail((PDList*)&(smgrInfo->vfhead->list), &(vpos->list));
    return vpos;
}

PPageHeader smgr_read(PVFVec vfInfo, PPageOffset pageOffset)
{
    PPageHeader page = NULL;
    int offset = (pageOffset->pageno - 1) * PAGE_MAX_SIZE;

    page = (PPageHeader)smgrReadPage(vfInfo->fd, offset, PAGE_MAX_SIZE);
    if(NULL == page)
    {
        //hat_log("read page %d-%d failure.\n", offset, PAGE_MAX_SIZE);
        return NULL;
    }

    return page;
}

int smgr_create(char *fileName, ForkType forkNum)
{
    char fname[FILE_PATH_MAX_LEN] = {0};
    int fd = -1;

    switch(forkNum)
    {
        case MAIN_FORK:
            snprintf(fname, FILE_PATH_MAX_LEN, "%s", fileName);
            break;
        case GROUP_FORK:
            snprintf(fname, FILE_PATH_MAX_LEN, "%s%s", GROUP_FILE_PRE, fileName);
            break;
        default:
            return -1;
        break;
    }

    fd = CreateTableFile(fname, 0666);
    if(fd < 0)
    {
        hat_log("create file %s failure.\n", fname);
    }

    return fd;
}

int smgr_write(PVFVec vfInfo, PPageOffset pageOffset, PPageHeader page)
{
    int offset = 0;
    int writeSize = 0;

    offset = PAGE_MAX_SIZE * (pageOffset->pageno - 1);
    lseek(vfInfo->fd, offset, SEEK_SET);

    writeSize = write(vfInfo->fd, (char *)page, PAGE_MAX_SIZE);
    if (writeSize != PAGE_MAX_SIZE)
    {
        hat_log("table is extened failure, maybe is not enough space.\n");
        return -1;
    }

    return 0;
}

