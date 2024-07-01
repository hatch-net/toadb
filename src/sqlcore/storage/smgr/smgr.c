/*
 *	storage manager 
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

#include "smgr.h"
#include "memStack.h"
#include "tfile.h"
#include "memStack.h"

#include <sys/types.h>
#include <unistd.h>

/* configure param */
int config_fsync = 0;

/* 存储管理上下文 */
PStorageMangeContext smgrContext = NULL;


static PVFVec GetVFec(PsgmrInfo smgrInfo, ForkType forkNum);
static PVFVec DeleteVF(PVFVec head, PVFVec node);
static PVFVec FindVF(PVFVec head, ForkType forknum);

static void LockSmgr(LockState state);
static int AddFileInfoToSmgrContext(PsgmrInfo smgrInfo, PVFVec vpos);
static int AddFileToSmgrContext(PVFVec vf, PFileHandle fileHandle);
static int CloseFileSmgrContext();

/* 
 * 初始化存储管理上下文信息 
 * 内存在数据字典上下文中创建，不需要单独释放
*/
int InitSmgr()
{
    if(NULL != smgrContext)
        return 0;

    smgrContext = (PStorageMangeContext)AllocMem(sizeof(StorageMangeContext));
    smgrContext->fileMaxNum = FILE_MAX_OPENED_PER_PROCESS;
    smgrContext->fileOpenedNum = 0;
    smgrContext->vFileNum = 0;
    smgrContext->vfhead = NULL;
    InitRWLock(&smgrContext->lock);
    
    return 0;
}

/* 
 * 添加文件信息描述符到存储管理上下文中；
 * 当打开第一次新的表文件时
 */
static int AddFileInfoToSmgrContext(PsgmrInfo smgrInfo, PVFVec vpos)
{
    if(NULL == smgrContext)
    {
        return -1;
    }
    
    do 
    {
        if(NULL == smgrContext->vfhead)
        {
            smgrContext->vfhead = smgrInfo->vfhead = vpos;
            break;
        }
        
        if(NULL == smgrInfo->vfhead)
        {
            smgrInfo->vfhead = vpos;
            AddDListTail((PDList*)&(smgrContext->vfhead), (PDList)smgrInfo->vfhead);
        }
        else 
        {
            AddDListNext((PDList*)&(smgrInfo->vfhead), (PDList)(vpos));
        }
    }while(0);
    
    smgrInfo->vfend = vpos;
    smgrContext->vFileNum += 1;

    /* TODO: replace fd */
    if(smgrContext->fileOpenedNum > smgrContext->fileMaxNum-1)
        CloseFileSmgrContext();
 
    return 0;
}

static int AddFileToSmgrContext(PVFVec vf, PFileHandle fileHandle)
{
    LockVF(vf, LOCK_EXCLUSIVE);
    vf->pfh = fileHandle;
    smgrContext->fileOpenedNum += 1;
    LockVF(vf, RELEASE_LOCK);
}

/* 
 * 关闭文件句柄
 * 当打开文件过多时，需要关闭一些文件句柄
 * 来给新的表文件打开留出余地
 */
static int CloseFileSmgrContext()
{
    PVFVec vpos = NULL;
    PVFVec tpos = smgrContext->vfhead;

    while(tpos != NULL)
    {
        if(tpos->pfh != NULL)
        {
            vpos = tpos;
            break;
        }

        tpos = (PVFVec)tpos->list.next;
        
        if(tpos == smgrContext->vfhead)
            break;
    }

    /* 简单处理，关闭链头的文件，下次使用时再打开 */
    if(vpos != NULL)
        smgr_close(vpos);

    return 0;
}

static PVFVec CreateVFVec(ForkType forkNum, PFileHandle fhd)
{
    PVFVec vfInfo = (PVFVec)CreateDictionaryItem(sizeof(VFVec));

    vfInfo->forkNum = forkNum;
    vfInfo->pfh = fhd;
    InitRWLock(&vfInfo->lock);

    return vfInfo;
}

static PVFVec AddVFec(PsgmrInfo smgrInfo, ForkType forkNum)
{
    PVFVec vpos = NULL;

    LockSmgr(LOCK_EXCLUSIVE);
    vpos = FindVF(smgrInfo->vfhead, forkNum);
    if(NULL != vpos)
    {
        LockSmgr(RELEASE_LOCK);
        return vpos;
    }

    vpos = CreateVFVec(forkNum, NULL);
    INIT_DLIST_NODE(vpos->list);

    AddFileInfoToSmgrContext(smgrInfo, vpos);
    LockSmgr(RELEASE_LOCK);
    return vpos;
}

static PVFVec GetVFec(PsgmrInfo smgrInfo, ForkType forkNum)
{
    PVFVec vpos = NULL;
    PVFVec vhead = NULL;
    char fname[FILE_PATH_MAX_LEN] = {0};

    LockSmgr(LOCK_SHARED);
    vhead = smgrInfo->vfhead;
    LockSmgr(RELEASE_LOCK);
    if(NULL == vhead)
    {
        vpos = AddVFec(smgrInfo, forkNum);
    }

    /* read info */
    if(NULL == vpos)
    {
        vpos = FindVF(smgrInfo->vfhead, forkNum);
        if(NULL == vpos)
        {
            vpos = AddVFec(smgrInfo, forkNum);
        }
    }

    return vpos;
}

static PVFVec FindVF(PVFVec head, ForkType forknum)
{
    PVFVec vpos = NULL;
    PVFVec tpos = head;
    int searchCnt = MAX_FORK;

    while((tpos != NULL) && (searchCnt-- > 0))
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

/* 
 * 查找某个表对应的虚拟文件描述符
 */
PVFVec SearchVF(PVFVec head, ForkType forknum)
{
    PVFVec vpos = NULL;

    LockSmgr(LOCK_SHARED);   
    vpos = FindVF(head, forknum);
    LockSmgr(RELEASE_LOCK);

    return vpos;
}

/* 
 * 从存储管理中删除虚拟文件节点
 */
PVFVec DeleteVF(PVFVec head, PVFVec node)
{
    /* 如果不相同时，只在此处删除，否则从整个链中删除。 */
    if(head != node)
    {
        DelDListNode((PDList*)&(head), (PDList)(node));
    }
    else 
    {
        if(head != smgrContext->vfhead)
            DelDListNode((PDList*)&(smgrContext->vfhead), (PDList)(node));
        else 
        {
            smgrContext->vfhead = NULL;
        }
    }
    smgrContext->vFileNum -= 1;
    return node;
}

PVFVec smgr_create(PsgmrInfo smgrInfo, char *fileName, ForkType forkNum)
{
    PVFVec vpos = NULL;
    char fname[FILE_PATH_MAX_LEN] = {0};
    PFileHandle fd = NULL;

    if(NULL == smgrInfo)
        return NULL;

    /* vfvec add to mgr first, which avoid other worker create double. */
    vpos = GetVFec(smgrInfo, forkNum);
    if(vpos->pfh != NULL)
        return NULL;

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

    fd = CreateFile(fname, 0666);
    if(fd < 0)
    {
        hat_log("create file %s failure.", fname);
        return NULL;
    }

    AddFileToSmgrContext(vpos, fd);
    return vpos;
}

PVFVec smgr_open(PsgmrInfo smgrInfo, char *fileName, ForkType forkNum)
{
    PVFVec vpos = NULL;
    char fname[FILE_PATH_MAX_LEN] = {0};
    int newnode = 0;
    PFileHandle fd = NULL;

    if(NULL == smgrInfo)
        return NULL;

    vpos = GetVFec(smgrInfo, forkNum);

    if(vpos->pfh != NULL)
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

    fd = OpenFile(fname, 0666);
    if(fd == NULL)
    {
        // hat_log("open file %s failure.", fname);
        FreeMem(vpos);
        return NULL;
    }
    AddFileToSmgrContext(vpos, fd);
        
    return vpos;
}

int smgr_read(PVFVec vfInfo, PPageOffset pageOffset, char *page)
{
    INT64 offset = ((INT64)pageOffset->pageno - 1) * PAGE_MAX_SIZE;
    int readlen = 0;

    LockVF(vfInfo, LOCK_EXCLUSIVE);

    readlen = FileReadPage(vfInfo->pfh, page, offset, PAGE_MAX_SIZE);
    if(PAGE_MAX_SIZE != readlen)
    {
        LockVF(vfInfo, RELEASE_LOCK);
       // hat_error("read page error, readlen %d pageno:%d ", readlen, pageOffset->pageno);
        return -1;
    }

    LockVF(vfInfo, RELEASE_LOCK);

    return readlen;
}

int smgr_write(PVFVec vfInfo, PPageOffset pageOffset, PPageHeader page)
{
    INT64 ret = 0;
    INT64 offset = ((INT64)pageOffset->pageno - 1) * PAGE_MAX_SIZE;

    LockVF(vfInfo, LOCK_EXCLUSIVE);
    
    ret = FileWritePage(vfInfo->pfh, offset, PAGE_MAX_SIZE, (char *)page);
    if(ret < PAGE_MAX_SIZE)
    {
        LockVF(vfInfo, RELEASE_LOCK);
        return -1;
    }
    
    LockVF(vfInfo, RELEASE_LOCK);

    if(config_fsync)
        ret |= FileSync(vfInfo->pfh);

    return ret;
}


int smgr_close(PVFVec vfInfo)
{
    int ret = 0;
    
    if(vfInfo == NULL)
        return -1;

    if(vfInfo->pfh == NULL)
        return 0;

    if(config_fsync)
        ret |= FileSync(vfInfo->pfh);
    
    LockVF(vfInfo, LOCK_EXCLUSIVE);

    FileClose(vfInfo->pfh);
    vfInfo->pfh = NULL;

    LockVF(vfInfo, RELEASE_LOCK);

    smgrContext->fileOpenedNum -= 1;
    return ret;
}

int smgrRelease(PsgmrInfo sgmrInfo)
{
    PVFVec head = NULL;
    PVFVec tpos = NULL;
    PVFVec tpos_prev = NULL;

    LockSmgr(LOCK_EXCLUSIVE);

    head = sgmrInfo->vfhead;
    tpos = (PVFVec)sgmrInfo->vfend;
    tpos_prev = (PVFVec)sgmrInfo->vfend->list.prev;

    while(tpos != NULL)
    {
        if(tpos == head)
            break;

        DeleteVF(head, tpos);
        smgr_close(tpos);
        FreeMem(tpos);
        tpos = NULL;
        
        tpos = tpos_prev;
        tpos_prev = (PVFVec)tpos_prev->list.prev;
    }

    if(tpos != NULL)
    {
        DeleteVF(head, tpos);
        smgr_close(tpos);
        FreeMem(tpos);
        tpos = NULL;
    }

    LockSmgr(RELEASE_LOCK);

    FreeMem(sgmrInfo);

    return 0;
}

void LockVF(PVFVec vfInfo, LockState state)
{
    switch(state)
    {
        case LOCK_EXCLUSIVE:
            AcquireLock(&vfInfo->lock, RWLock_WRITE);
        break;
        case RELEASE_LOCK:
            ReleaseRWLock(&vfInfo->lock, RWLock_WRITE);
        break;
        default:
            hat_error("lockvf failure %d", state);
        break;
    }
}

static void LockSmgr(LockState state)
{
    switch(state)
    {
        case LOCK_SHARED:
            AcquireLock(&smgrContext->lock, RWLock_READ);
        break;

        case RELEASE_LOCK:
            ReleaseRWLock(&smgrContext->lock, RWLock_WRITE);
        break;

        case LOCK_EXCLUSIVE:
            AcquireLock(&smgrContext->lock, RWLock_WRITE);
        break;

        default:
            hat_error("lockvf failure %d", state);
        break;
    }
}
