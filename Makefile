#----------------------------------------------------------
#
# Lab #2 : NAND Simulator
#  - Storage Architecture, SSE3069
#
# TA: Jinwoo Jeong, Hyunbin Kang
# Prof: Dongkun Shin
# Intelligent Embedded Software Laboratory
# Sungkyunkwan University
# http://nyx.skku.ac.kr
#
#----------------------------------------------------------

STUDENT_ID = 2021310325

CC	= gcc
CFLAGS	= -g -O2 -Wall -std=c99
RM	= rm
TAR	= tar

TARGET	= ftl_test
SRCS	= ftl_test.c ftl.c nand.c
HEADERS	= nand.h ftl.h
OBJS	= $(SRCS:.c=.o)

# CUSTOM_HEADER가 비어있으면 0으로 기본값을 설정합니다.
CUSTOM_HEADER ?= 0

# CUSTOM_HEADER의 값을 VERSION_V* 형태로 변환하여 CFLAGS에 추가합니다.
CFLAGS += -DVERSION_V$(CUSTOM_HEADER)

all: $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

submit:
	$(RM) -f $(STUDENT_ID).tar.gz
	$(TAR) cvzf $(STUDENT_ID).tar.gz ftl.c nand.c
	ls -l $(STUDENT_ID).tar.gz

clean:
	$(RM) -f $(TARGET) $(STUDENT_ID).tar.gz $(OBJS)
