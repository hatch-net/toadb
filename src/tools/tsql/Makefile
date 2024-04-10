# Makefile toadb project
# create by senllang 2021/1/1
# mail : study@senllang.onaliyun.com
# Copyright (C) 2023-2023, senllang
#

TARGET = toadsql

##########################################
# toplevel source code directory
# topdir=$(shell pwd)
topdir = .
topsrcdir = $(topdir)/../..
SRC_DIR=$(topdir)


# filter-out source code directory 
OUT_SRC_DIR = $(topsrcdir)/tools $(topsrcdir)/main
OUT_DIR = $(foreach dir, $(OUT_SRC_DIR), $(shell find $(dir) -maxdepth 5 -type d))
DIR_DISP = $(foreach dir, $(OUT_DIR), $(wildcard $(dir)/*.c))


# general source file list , and objects list
DIR_ALL =  $(shell find $(topsrcdir) -maxdepth 5 -type d)
SOURCE = $(filter-out $(DIR_DISP), $(foreach dir, $(DIR_ALL), $(wildcard $(dir)/*.c)))

# search all directory and subdirectory
INC_DIR = $(DIR_ALL)

# general source file list , and objects list
DIRS = $(shell find $(SRC_DIR) -maxdepth 5 -type d)
# SOURCE += $(foreach dir, $(DIRS), $(wildcard $(dir)/*.c))
OBJS = ${patsubst %.c, %.o, $(SOURCE)}

# .h files take place all directories.
INC = ${foreach dir, $(INC_DIR), -I$(dir)}

##########################################
LDFLAGS += -lpthread
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

$(TARGET): $(topdir)/*.o  
	$(CC) $(CFLAGS) $^ -o $@

# show various value
default:
	@echo -e "DIRS "$(DIRS)
	@echo -e "SOURCE "$(SOURCE)
	@echo -e "OBJS "$(OBJS)
	@echo -e "INC "$(INC)
	@echo -e "DIR_DISP "$(DIR_DISP)

clean:
	rm -rf ${TARGET} ${OBJS}  $(topdir)/*.o 
	
