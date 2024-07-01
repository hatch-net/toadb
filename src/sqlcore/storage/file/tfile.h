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

#ifndef HAT_TFILE_H_H
#define HAT_TFILE_H_H

#include "public_types.h"
#include "smgr.h"



/* default 1GB (1*1024*1024*1024) 4Byte */
#define FILE_SEGMENT_MAX_SIZE (0x40000000) 



#define FILE_SEGMENT_MAX_PAGE (FILE_SEGMENT_MAX_SIZE/PAGE_MAX_SIZE)




PFileHandle CreateFile(char *filename, int mode);
PFileHandle OpenFile(char *filename, int mode);
int DeleteTableFile(char *filename);

int FileReadPage(PFileHandle fd, char *page, INT64 offset, int size);
int FileWritePage(PFileHandle fd, INT64 offset, int size, char *pageBuf);
int FileSync(PFileHandle fd);
int FileClose(PFileHandle fd);







#endif