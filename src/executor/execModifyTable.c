/*
 *	toadb table modify executor 
 * Copyright (C) 2023-2023, senllang
*/
#include "execModifyTable.h"
#include "tfile.h"
#include "tables.h"
#include "node.h"
#include "buffer.h"
#include "execNode.h"
#include "queryNode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* 
 * routine of modify table operators 
 */
int ExecModifyTable(PExecState eState, PTableList tblInfo, PTableRowData insertdata, NodeType type)
{
    int ret = 0;
    int tabletype = GET_STORAGE_TYPE_SHIFT(tblInfo->tableInfo->header.pageType);

    switch(tabletype)
    {
        case ST_NSM:
            ret = nsm_ExecModifyTable(tblInfo, insertdata, type);
            break;
        case ST_PAX:
            ret = pax_ExecModifyTable(eState, tblInfo, insertdata, type);
            break;
        default:
            hat_log("unknow table storage type %d. \n", tabletype);
            ret = -1;
            break;
    }

    return ret;
}

/* 
 * routine of modify table operators , under NSM storage 
 */
int nsm_ExecModifyTable(PTableList tblInfo, PTableRowData insertdata, NodeType type)
{
    int ret = 0;

    switch(type)
    {
        case T_InsertStmt:
            ret = nsm_ExecInsert(tblInfo, insertdata);
        break;
        default:
            hat_log("unsuport node type %d. \n", type);
            ret = -1;
        break;
    }

    return ret;
}

int nsm_ExecInsert(PTableList tblInfo, PTableRowData insertdata)
{
    int ret = 0;
    PPageDataHeader pageInsert = NULL;

    /* there we find free space. */
    pageInsert = GetSpacePage(tblInfo, insertdata->size, PAGE_NEW, MAIN_FORK);

    /* row data will be writed through to the table file. */
    ret = WriteRowData(tblInfo, pageInsert, insertdata);
    if(ret != 0)
    {
        hat_log("write row to page failure.[%d]\n", ret);
    }
    return ret;
}


/* 
 * routine of modify table operators , under PAX storage 
 */
int pax_ExecModifyTable(PExecState eState, PTableList tblInfo, PTableRowData rowsdata, NodeType type)
{
    int ret = 0;

    switch(type)
    {
        case T_InsertStmt:
            ret = pax_ExecInsert(eState, tblInfo, rowsdata);
        break;
        case T_UpdateStmt:
            ret = pax_ExecUpdate(eState, tblInfo, rowsdata);
        break;
        default:
            hat_log("unsuport node type %d of PAX storage . \n", type);
            ret = -1;
        break;
    }

    return ret;
}

/* 
 * update command excutor 
 * updateDataPos is find real row with table file position, 
 * and replace attribute value with set value already.
 * 
 * steps:
 * first, move old tuple to undo page, as insert command.
 * second, insert new tuple to group the same as old tuple.
 * NOTE: only support pax storage type.
 */
int pax_ExecUpdate(PExecState eState, PTableList tblInfo, PTableRowData updateDataPos)
{
    PTableRowDataWithPos rowDataPos = (PTableRowDataWithPos)updateDataPos;
    PAttrDataPosition attrDataPos = NULL;
    PPageDataHeader originPage = NULL;
    PPageDataHeader newRowPage = NULL;
    PPageDataHeader undoPage = NULL;
    PRowData oldTuple = NULL;
    PRowData newRowData = {0};
    RowHeaderData rowheader = {0};
    PItemData oldItem = NULL;

    int attrIndex = 0;
    int rowSize = 0;
    int ret = 0;

    if(rowDataPos->num <= 0)
    {
        hat_error("no row to update \n");
        return -1;
    }

    /* 
     * form new row data, with undo tuple version chain.
     * pax storage type attrDataPos->rowData.num  always is 1.
    */
    rowSize = sizeof(RowData) + sizeof(PRowColumnData);
    newRowData = (PRowData)AllocMem(rowSize);

    for(attrIndex = 0; attrIndex < rowDataPos->num; attrIndex++)
    {
        attrDataPos = rowDataPos->attrDataPos[attrIndex];

        /* find old tuple position */
        originPage = GetPageByIndex(tblInfo, attrDataPos->headItem.pageno.pageno, MAIN_FORK);

        oldTuple = DeFormRowDatEx(originPage, attrDataPos->headItem.itemData.offset);
        rowSize = sizeof(ItemData) + sizeof(RowData) + oldTuple->rowsData.size;

        /* find free space in the undo page which is the group. */
        undoPage = GetSpaceSpecifyPage(tblInfo, rowSize, PAGE_NEW, MAIN_FORK, originPage->undoPage.pageno, PAGE_UNDO);
        if(NULL == undoPage)
        {
            hat_error("get undo page error, no free space rest of disk. \n");
            return -1;
        }

        /* first use undo page. */
        if(IsInvalidPageNum(originPage->undoPage.pageno))
        {
            originPage->undoPage.pageno = undoPage->header.pageNum;
        }

        rowSize = sizeof(TableRowData) + attrDataPos->rowData.num * sizeof(PRowColumnData);
        memcpy(&(newRowData->rowsData), &(attrDataPos->rowData), rowSize);

        /* oldtuple is copied to undo page. */
        ret = WriteRowItemDataWithHeader(tblInfo, undoPage, oldTuple, &(newRowData->rowheader));
        if(ret < 0)
        {
            break;
        }
        
        /* insert new tuple to page, inplace or new other place. */
        if(attrDataPos->rowData.size <= oldTuple->rowsData.size)
        {
            /* write only data replace. */
            ret = WriteRowDataOnly(tblInfo, originPage, newRowData, &(attrDataPos->headItem.itemData));
        }
        else
        {
            rowSize = sizeof(RowData) + attrDataPos->rowData.size;
            
            newRowPage = GetSpaceSpecifyPage(tblInfo, rowSize, PAGE_NEW, MAIN_FORK, originPage->header.pageNum, PAGE_DATA);
            if(NULL == newRowPage)
            {
                ret = -1;
                break;
            }

            if(newRowPage->header.pageNum == originPage->header.pageNum)
            {
                /* write only data other place of the same page. */
                oldItem = GET_ITEM(attrDataPos->headItem.itemOffset, newRowPage);
                oldItem->offset = newRowPage->dataEndOffset - rowSize;
                newRowPage->dataEndOffset = oldItem->offset;
                ret = WriteRowDataOnly(tblInfo, newRowPage, newRowData, oldItem);
            }
            else
            {
                /* write new item and data in the extension page. */
                ret = WriteRowItemDataWithHeader(tblInfo, newRowPage, newRowData, &rowheader);

                /* old item chang to redirect flag. */
                oldItem = GET_ITEM(attrDataPos->headItem.itemOffset, originPage);
                oldItem->len |= ITEM_REDIRECT_MASK;
                oldItem->offset = rowheader.rowPos.pageIndex.pageno;
                oldItem->len = SetItemSize(oldItem->len, rowheader.rowPos.itemIndex);
                WritePage(tblInfo, originPage, MAIN_FORK);
            }
        }

        /* every column will be release resource */
        if(NULL != undoPage)
        {
            ReleasePage(undoPage);
            undoPage = NULL;
        }

        if(NULL != originPage)
        {
            ReleasePage(originPage);
            originPage  = NULL;
        }
            
        if(NULL != newRowPage)
        {
            ReleasePage(newRowPage);
            newRowPage = NULL;
        }

        if(NULL != oldTuple)
        {
            FreeMem(oldTuple);
            oldTuple = NULL;
        }

        /* some error ocur, then exited. */
        if(ret < 0)
        {
            break;
        }
    }

    FreeMem(newRowData);
    
    return ret;
}

int pax_ExecInsert(PExecState eState, PTableList tblInfo, PTableRowData insertdata)
{
    int ret = 0;
    PPageDataHeader *pageList = NULL;
    PTableRowData colRows = NULL; 
    PTableRowData currRows = NULL;
    PPageDataHeader page = NULL;
    PModifyTblState planState = (PModifyTblState)eState->subPlanStateNode;
    PScanPageInfo scanPageInfo = NULL;
    int ColNum = 0;
    int itemsize = sizeof(TableRowData) + sizeof(PRowColumnData);

    /* 
     * find free space
     * several pages will insert columns, and these are in one group. 
    */
    scanPageInfo = planState->scanState->scanPostionInfo;
    pageList = scanPageInfo->pageList;

    if(scanPageInfo->pageListNum < insertdata->num)
    {
        hat_error("page list has not enogh space, request %d ,but only %d.\n", insertdata->num, scanPageInfo->pageListNum);
        return -1;
    }

    /* find enough free space group page */
    do
    {
        ret = GetSpaceGroupPage(tblInfo, insertdata, PAGE_NEW, pageList, scanPageInfo);
        if(ret < 0)
        {
            /* current groups has not enough free space, we extension a new group. */
            page = ExtensionTbl(tblInfo, tblInfo->tableDef->colNum, MAIN_FORK);

            /* update group file, insert one group data */
            InsertGroupItem(tblInfo, page, tblInfo->tableDef->colNum);

            ReleasePage(page);
        }
    } while (ret < 0);

    /* Now, to form rows column by column. */
    colRows = FormColRowsData(insertdata);

    /* insert into pages */
    for(ColNum = 0; ColNum < insertdata->num; ColNum++)
    {
        currRows = (PTableRowData)((char *) colRows + ColNum * itemsize);
        ret = WriteRowItemData(tblInfo, pageList[ColNum], currRows);
    }

    /* resource release */
    ReleasePageList(pageList, insertdata->num);
    
    return 1;
}

/*
 * modify top node
 * plan node -> modify node -> project -> select -> scan 
 */
PTableRowData ExecTableModifyTbl(PExecState eState)
{
    PModifyTblState planState = NULL;
    PModifyTbl plan = NULL;
    PTableRowData rowData = NULL;
    PTableRowData newRowData = NULL;
    PTableRowDataWithPos rowDataPos = NULL;
    PTableList tblInfo = NULL;
    PRangTblEntry rte = NULL;
    int opRet = -1, suc_rows = 0;

    /*
     * eState->subPlanNode and  eState->subPlanStateNode 
     * is current plan , planState Node;
     */
    plan = (PModifyTbl)eState->subPlanNode;
    planState = (PModifyTblState)eState->subPlanStateNode;
    rte = (PRangTblEntry)plan->rangTbl;
    tblInfo = rte->tblInfo;

    /* process left and right node */
    if(NULL == plan->leftplan)
    {
        eState->retCode = ExecRetCode_ERR;
        return NULL;
    }
    
    /* 
     * retCode is 0 when success ending, or < 0 is error. 
     * rowData is NULL, when returning is not set.
    */
    for( ;; )
    {        
        eState->subPlanNode = (PNode)plan->leftplan;
        eState->subPlanStateNode = (PNode)planState->left;

        rowData = ExecNodeProc(eState);
        if(eState->retCode < ExecRetCode_SUC)
        {
            /* TODO: rollback transaction */
            break;
        }

        /* if success ending, stop circle. */
        if(NULL == rowData)
            break;

        /* modify table data */
        switch(eState->commandType)
        {
            case CMD_INSERT:
                eState->subPlanStateNode = (PNode)planState;
                eState->subPlanNode = (PNode)plan;
                opRet = ExecModifyTable(eState, tblInfo, rowData, T_InsertStmt);
            break;
            case CMD_UPDATE:
                eState->subPlanNode = (PNode)plan->rightplan;
                eState->subPlanStateNode = (PNode)planState->right; 

                /* parent target rowdata. */
                eState->scanRowDataRight = rowData;

                /* excutor set list express. */
                newRowData = ExecNodeProc(eState);
                if((NULL == newRowData) || (eState->retCode < ExecRetCode_SUC))
                {
                    /* TODO: rollback transaction */
                    break;
                }

                eState->subPlanStateNode = (PNode)planState;
                eState->subPlanNode = (PNode)plan;
                opRet = ExecModifyTable(eState, tblInfo, newRowData, T_UpdateStmt);
            break;
            default:
                return NULL;
            break;
        }

        if(opRet < 0)
        {
            eState->retCode = opRet;
            break;
        }

        suc_rows ++;
    }

    /* resource release */
    if((NULL != planState->scanState) 
        && (NULL != planState->scanState->scanPostionInfo->searchPageList->page))
        ReleasePage(planState->scanState->scanPostionInfo->searchPageList->page);

    if(suc_rows > 0)
        eState->retCode = suc_rows;

    /* nothing will be return. */
    return NULL;
}

