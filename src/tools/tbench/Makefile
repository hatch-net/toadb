# Makefile toadb project
# create by senllang 2021/1/1
# mail : study@senllang.onaliyun.com
#
# Copyright (c) 2023-2024 senllang
# 
# toadb is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
# http://license.coscl.org.cn/MulanPSL2
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.
#

TARGET = toadbench

##########################################
# toplevel source code directory
# topdir=$(shell pwd)
topdir = .
topsrcdir = $(topdir)/../../
SRC_DIR=$(topdir)

# search all directory and subdirectory
DIRS = $(shell find $(SRC_DIR) -maxdepth 5 -type d)
INC_DIR = $(shell find $(topsrcdir) -maxdepth 5 -type d)

# general source file list , and objects list
SOURCE = $(foreach dir, $(DIRS), $(wildcard $(dir)/*.c))
OBJS = ${patsubst %.c, %.o, $(SOURCE)}

# .h files take place all directories.
INC = ${foreach dir, $(INC_DIR), -I$(dir)}

##########################################
##########################################
DEFINES += -DSTORAGE_FORMAT_PAX -DMEM_DEBUG
DEBUG_FLAGS = -g
##########################################		
		
CC = gcc
#CFLAGS += -O2

CFLAGS += ${INC}
CFLAGS += ${DEFINES}
CFLAGS += ${LDFLAGS}

CFLAGS += ${DEBUG_FLAGS}

# compile start
.PHONY: all clean 
all: $(default) $(OBJS) $(TARGET) 


# object compile to O_PATH
.c.o: $(SOURCE)
	$(CC) $(CFLAGS) -c $<

$(TARGET):$(topdir)/*.o
	$(CC) $(CFLAGS) $^ -o $@

# show various value
default:
	echo -e $(DIRS)
	echo -e $(SOURCE)
	echo -e $(OBJS)
	echo -e $(INC)

clean:
	rm -rf ${TARGET} ${OBJS}  $(topdir)/*.o 
	
