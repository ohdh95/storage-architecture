#
# Lab #5 : ZNS+ Simulator
#  - Storage Architecture, SSE3069 *
#
# TA: Youngjin Kim, Eunji Song
# Prof: Dongkun Shin
# Intelligent Embedded Software Laboratory
# Sungkyunkwan University
# http://nyx.skku.ac.kr
#

STUDENT_ID = 2021310325

CC	= gcc
CFLAGS	= -g -O0 -Wall -std=c99
RM	= rm
TAR	= tar

TARGET	= zns_test
SRCS	= zns_test.c zns.c nand.c
HEADERS	= nand.h zns.h
OBJS	= $(SRCS:.c=.o)

all: $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

submit:
	$(RM) -f $(STUDENT_ID).tar.gz
	$(TAR) cvzf $(STUDENT_ID).tar.gz zns.c
	ls -l $(STUDENT_ID).tar.gz

clean:
	$(RM) -f $(TARGET) $(STUDENT_ID).tar.gz $(OBJS) *.txt
