#----------------------------------------------------------
#
# Lab #3 : Page Mapping FTL II
#  - Storage Architecture, SSE3069
#
#
# TA: Youngjin Kim, Eunji Song
# Prof: Dongkun Shin
# Intelligent Embedded Software Laboratory
# Sungkyunkwan University
# http://nyx.skku.ac.kr
#
#----------------------------------------------------------

STUDENT_ID = 2021000000

CC	= gcc
CFLAGS	= -g -O2 -Wall -std=c99
RM	= rm
TAR	= tar

TARGET	= ftl_test
SRCS	= ftl_test.c ftl.c nand.c
HEADERS	= nand.h ftl.h
OBJS	= $(SRCS:.c=.o)

all: $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

submit:
	$(RM) -f $(STUDENT_ID).tar.gz
	$(TAR) cvzf $(STUDENT_ID).tar.gz ftl.c
	ls -l $(STUDENT_ID).tar.gz

clean:
	$(RM) -f $(TARGET) $(STUDENT_ID).tar.gz $(OBJS)
