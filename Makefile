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

STUDENT_ID = 2021310325

CC	= gcc
CFLAGS	= -g -O0 -Wall -std=c99
RM	= rm
TAR	= tar

TARGET	= ftl_test
SRCS	= ftl_test.c ftl.c nand.c
HEADERS	= nand.h ftl.h
OBJS	= $(SRCS:.c=.o)

# H가 비어있으면 0으로 기본값을 설정합니다.
H ?= 0

# H의 값을 VERSION_V* 형태로 변환하여 CFLAGS에 추가합니다.
CFLAGS += -DVERSION_V$(H)

all: $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

submit:
	$(RM) -f $(STUDENT_ID).tar.gz
	$(TAR) cvzf $(STUDENT_ID).tar.gz ftl.c
	ls -l $(STUDENT_ID).tar.gz

clean:
	$(RM) -f $(TARGET) $(STUDENT_ID).tar.gz $(OBJS) *.txt
