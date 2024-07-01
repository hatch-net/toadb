/*
 *	toadb pax storage 
 * Copyright (C) 2023-2023, senllang
*/
#ifndef HAT_PAX_H_H
#define HAT_PAX_H_H

#include "tablecom.h"


#pragma pack(push, 1)

#define MAX_ROW_PER_GROUP   ((PAGE_MAX_SIZE/(MIN_ROW_SIZE))&0xFFFFFF00)

typedef struct GroupChain
{
    PageOffset prev;
    PageOffset next;
    PageOffset head;    
}GroupChain;

typedef struct GroupPageInfo
{
    unsigned int group_id;
    int columnNum;                /* It specified column index which data will storaged in this page. */
}GroupPageInfo;

#define FIRST_GROUP_ID      1
#define INVALID_GROUP_ID    0


/* 
 * table file format
 * file manage by pages units. 
 * page maxsize 
 * first page: PageDataHeader --- table info -- columninfo ...
 * seconde page: pageheader --- pagegroup-- item1,item2 ....freespace...columndata2,columndata1
 * undo page: pageheader --- pagegroup-- item1,item2 ....freespace...columndata2,columndata1
 */


/* 
 * group file format 
 * first page: GroupPageHeader(pagedata--groupinfo) --- table info -- columninfo ...
 * second page: pageheader --- groupitem1,groupitem2,... freespace ... memdata2,memdata2
 * 
*/
typedef struct GroupPageHeader
{
    PageDataHeader pageheader;
    GroupPageInfo groupInfo;
}GroupPageHeader, *PGroupPageHeader;
#define GROUP_PAGE_HEADER_SIZE (sizeof(GroupPageHeader))

/*
 * size same as item
 */
typedef struct GroupItem
{
    int offset;            
    int len;              /* high bit |32 valid| */
    long long groupid;    /* todo: */
}GroupItem, *PGroupItem;

typedef struct MemberDataItem
{
    int colIndex;       /* record column index, while alter table command to change. */
    PageOffset pageno;
}MemberDataItem, *PMemberDataItem;

typedef struct MemberData
{
    int colIndex ;

    /* always is 1. in the future, it will change. */
    int memNum;

    PageOffset member[FLEXIBLE_SIZE];       // TODO: MemberDataItem member[FLEXIBLE_SIZE];   
}MemberData, *PMemberData;


#pragma pack(pop)

#endif