/*
 *	toadb tables 
 * Copyright (C) 2023-2023, senllang
*/

#include "tables.h"
#include "buffer.h"
#include "tfile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define log printf

int TableOpen(PTableList tblInfo, ForkType forkNum)
{
    PVFVec vfInfo = NULL;
    PageOffset pageoffset = {1,1};
    PPageHeader page = NULL;
    int metadata_size = 0;

    /* add vfinfo linker */
    if(NULL == tblInfo->sgmr)
    {
        tblInfo->sgmr = (PsgmrInfo)AllocMem(sizeof(sgmrInfo));
        tblInfo->sgmr->version = 0x01;
        tblInfo->sgmr->storageType = tblInfo->tableDef->tableType;
        tblInfo->sgmr->vfhead = NULL;
        tblInfo->sgmr->vpos = NULL;
    }

    vfInfo = smgr_open(tblInfo->sgmr, tblInfo->tableDef->tableName, forkNum);
    if(NULL == vfInfo)
    {
        //log("open table %s-%d failure.\n", tblInfo->tableDef->tableName, forkNum);
        return -1;
    }

    if(forkNum == GROUP_FORK)
    {
        page = smgr_read(vfInfo, &pageoffset);
        if(NULL == page)
        {
            //log("read table %s-%d first page failure.\n", tblInfo->tableDef->tableName, forkNum);
            return -2;
        }

        tblInfo->groupInfo = (PGroupPageHeader)page;
    }
    return 0;
}

int TableCreate(char *fileName, ForkType forkNum)
{
    PageOffset pageoffset = {1,1};
    PGroupPageHeader gpage = NULL;
    PPageDataHeader page = NULL;
    PTableMetaInfo tableDef = NULL;
    VFVec vf;
    int metadata_size = 0;
    int gfd = -1, tfd = -1;

    /* create file on the disk */
    gfd = smgr_create(fileName, forkNum);
    if(gfd < 0)
    {
        log("create table %s-%d failure.\n", fileName, forkNum);
        return -1;
    }

    /* initialize first page, which has the metadata infomation. */
    if(GROUP_FORK == forkNum)
    {
        /* read first page of table file */
        tfd = smgrOpen(fileName);
        if(tfd < 0)
        {
            log("create table %s-%d failure.\n", fileName, forkNum);
            smgrClose(gfd);
            return -1;
        }

        vf.fd = tfd;
        pageoffset.pageno = PAGE_HEAD_PAGE_NUM;
        page = (PPageDataHeader)smgr_read(&vf, &pageoffset);
        if(NULL == page)
        {
            log("read table %s-%d first page failure.\n", fileName, forkNum);
            smgrClose(gfd);
            smgrClose(tfd);
            return -1;
        }

        /* first page content is grouppageheader and table metadata */
        gpage = (PGroupPageHeader)AllocMem(PAGE_MAX_SIZE);

        /* page header */
        memcpy(&(gpage->pageheader), page, PAGE_DATA_HEADER_SIZE);

        /* group info */
        tableDef = (PTableMetaInfo) ((char*)(page) + page->dataOffset);
        gpage->groupInfo.group_id = INVALID_GROUP_ID;
        gpage->groupInfo.columnNum = tableDef->colNum;

        /* table metadata, same as table */
        metadata_size = page->dataEndOffset - page->dataOffset;
        memcpy((char*)gpage + GROUP_PAGE_HEADER_SIZE, 
                    tableDef, 
                    metadata_size);
        
        gpage->pageheader.dataOffset = GROUP_PAGE_HEADER_SIZE;
        gpage->pageheader.dataEndOffset = GROUP_PAGE_HEADER_SIZE + metadata_size;

        /* write to disk file */
        smgrFlush(gfd, (char*)gpage, 1);

        FreeMem(gpage);
        smgrClose(tfd);
    }

    smgrClose(gfd);
    return 0;
}

PPageHeader TableRead(PsgmrInfo smgrInfo, PPageOffset pageoffset, ForkType forkNum)
{
    PPageHeader page = NULL;
    PVFVec vpos = NULL;

    vpos = SearchVF(smgrInfo->vfhead, forkNum);
    if(NULL == vpos)
    {
        log("file not opened %d \n", forkNum);
        return NULL;
    }

    page = smgr_read(vpos, pageoffset);

    return page;
}

int TableWrite(PsgmrInfo smgrInfo, PPageHeader page, ForkType forkNum)
{
    PageOffset pageoffset;
    PVFVec vpos = NULL;
    int ret = 0;

    vpos = SearchVF(smgrInfo->vfhead, forkNum);
    if(NULL == vpos)
    {
        log("file not opened %d \n", forkNum);
        return -1;
    }

    pageoffset.pageno = page->pageNum;
    ret = smgr_write(vpos, &pageoffset, page);

    return ret;
}

int TableDrop(PTableList tblInfo)
{
    int ret = 0;
    char filepath[1024] = {0};

    if(NULL == tblInfo)
    {
        log("exec drop table failure. unknow table name.\n");
        return -1;
    }

    /* delete file main fork */
    ret = DeleteTableFile(tblInfo->tableDef->tableName);
    if(0 != ret)
    {
        log("exec drop %s table failure.\n", tblInfo->tableDef->tableName);
        return -1;
    }

    /* delete file main fork */
    snprintf(filepath, 1024, "%s%s", GROUP_FILE_PRE, tblInfo->tableDef->tableName);
    ret = DeleteTableFile(filepath);
    return ret;
}


