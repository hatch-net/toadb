# Makefile toadb project
# create by senllang 2021/1/1
# mail : study@senllang.onaliyun.com
# Copyright (C) 2023-2023, senllang
#


VERSION_H = 0
VERSION_L = 01

TARGET = toadb-${VERSION_H}-${VERSION_L}

##########################################
# toplevel source code directory
# topdir=$(shell pwd)
topdir = .
SRC_DIR=$(topdir)

# filter-out source code directory 
OUT_SRC_DIR = $(topdir)/tools
OUT_DIR = $(shell find $(OUT_SRC_DIR) -maxdepth 5 -type d)
DIR_DISP = $(foreach dir, $(OUT_DIR), $(wildcard $(dir)/*.c))

# search all directory and subdirectory
DIRS = $(shell find $(SRC_DIR) -maxdepth 5 -type d)

# general source file list , and objects list
SOURCE = $(filter-out $(DIR_DISP), $(foreach dir, $(DIRS), $(wildcard $(dir)/*.c)))
OBJS = ${patsubst %.c, %.o, $(SOURCE)}

# .h files take place all directories.
INC = ${foreach dir, $(DIRS), -I$(dir)}

PARSER_DIR=${topdir}/parser
SCANNER = ${PARSER_DIR}/scanner.c
PARSER = ${PARSER_DIR}/grammar.c
PARSER_H = ${PARSER_DIR}/grammar.h
SCANNER_H = scanner.h

##########################################
# test programme
builddir = $(topdir)/build

##########################################
##########################################
DEFINES += -DSTORAGE_FORMAT_PAX -DMEM_DEBUG
# LDFLAGS += -lpthread
DEBUG_FLAGS = -g
##########################################		
		
CC = gcc
#CFLAGS += -O2

CFLAGS += ${INC}
CFLAGS += ${DEFINES}
CFLAGS += ${LDFLAGS}

CFLAGS += ${DEBUG_FLAGS}

# compile start
.PHONY: all clean parser
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
	@echo -e "DIRS "$(DIRS)
	@echo -e "SOURCE "$(SOURCE)
	@echo -e "OBJS "$(OBJS)
	@echo -e "INC "$(INC)
	@echo -e "DIR_DISP "$(DIR_DISP)

parser: $(PARSER) $(SCANNER)
$(PARSER): ${PARSER_DIR}/grammar.y
	cd ${PARSER_DIR}/
	bison --defines=${PARSER_H} -o $@ $<
	cd -

$(SCANNER): ${PARSER_DIR}/scanner.l
	cd ${PARSER_DIR}/
	flex -o $@ $<
	cd -
	
clean:
	rm -rf ${TARGET} ${OBJS} ${builddir} $(topdir)/*.o ${SCANNER} ${PARSER} ${PARSER_H} ${SCANNER_H}
	
