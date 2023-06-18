# Makefile toadb project
# create by senllang 2021/1/1
# mail : study@senllang.onaliyun.com
# Copyright (C) 2023-2023, senllang
#


VERSION_H = 0
VERSION_L = 01

TARGET = toadbench

##########################################
# toplevel source code directory
# topdir=$(shell pwd)
topdir = .
SRC_DIR=$(topdir)

# search all directory and subdirectory
DIRS = $(shell find $(SRC_DIR) -maxdepth 5 -type d)


# general source file list , and objects list
SOURCE = $(foreach dir, $(DIRS), $(wildcard $(dir)/*.c))
OBJS = ${patsubst %.c, %.o, $(SOURCE)}

# .h files take place all directories.
INC = ${foreach dir, $(DIRS), -I$(dir)}


##########################################
# test programme
builddir = $(topdir)/build

##########################################
##########################################
DEFINES += -DTEST_PRO
LDFLAGS += -lpthread
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
all: $(default) $(builddir) $(OBJS) $(TARGET) 

#build dir create
$(builddir):
	mkdir -p $(builddir)

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
	rm -rf ${TARGET} ${OBJS} ${builddir} $(topdir)/*.o 
	
