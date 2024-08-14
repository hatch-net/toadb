/*
 *	toadb undo page 
 *
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
 * 
*/

#include "undopage.h"
#include "tablecom.h"
#include "snapshot.h"
#include "scan.h"
#include "buffer.h"
#include "bufferPool.h"
#include "memStack.h"

PRowColumnData CurrentReadVersion(PRowColumnData headRowData, PScanState tblScan)
{
    RowPosition olderRowPos = {0};
    PRowColumnData rowData = NULL;
    PPageDataHeader page = NULL;
    TupleVisibility isVisible = TUPLE_UNKNOWN;
    PItemData Item = NULL;

    if(NULL == headRowData)
        return NULL;
    
    olderRowPos = headRowData->headerData.undo;
    FreeMem(headRowData);
    headRowData = NULL;

    /* we will follow the version chain, until visible version or end of table. */
    do
    {
        /* check position. */
        if(IsInvalidPageNum(olderRowPos.pageIndex.pageno))
            break;

        /* read page */
        page = ReadPage(tblScan->tblInfo, olderRowPos.pageIndex.pageno, MAIN_FORK);
        if(NULL == page)
            break;

        /* lock page */
        LockPage(page, BUF_READ);

        Item = (PItemData)GET_ITEM_BY_INDEX(olderRowPos.itemIndex, page);
        if(ITEM_END_CHECK(Item, page) || !GetItemValid(Item->len))
            break;

        /* deform row */
        rowData = DeFormRowDatEx2(page, Item->offset);

        /* unlock page */
        UnLockPage(page, BUF_READ);

        if(NULL == rowData)
            break;

        isVisible = TupleVisibilitySatisfy(&rowData->headerData, tblScan->snapshot);
        if(TUPLE_VISIBLE == isVisible)
        {
            return rowData;
        }       

        /* try next version data */
        olderRowPos = rowData->headerData.undo;

        FreeMem(rowData);
        rowData = NULL;
    } while (TUPLE_VISIBLE != isVisible);
    
    return NULL;
}