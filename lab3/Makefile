# Makefile for lab3

COMPILER_TYPE= gnu

CC= gcc

PROG= lab3
SRCS= lab3.c test3.c keyboard.c timer.c handler.S

CFLAGS= -Wall

DPADD+= ${LIBDRIVER} ${LIBSYS}
LDADD+= -ldriver -lsys

CPPFLAGS += -D LAB3

LDFLAGS+= -L .

MAN=

.include <bsd.prog.mk>
.include <bsd.gcc.mk>