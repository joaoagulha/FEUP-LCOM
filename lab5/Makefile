# Makefile for lab5

COMPILER_TYPE= gnu

CC= gcc

PROG= lab5
SRCS= lab5.c test5.c timer.c vbe.c keyboard.c video_gr.c video_test.c read_xpm.c  sprite.c

CFLAGS= -Wall

DPADD+= ${LIBDRIVER} ${LIBSYS}
LDADD+= -llm -ldriver -lsys

CPPFLAGS += -D lAB4

LDFLAGS+= -L .

MAN=

.include <bsd.prog.mk>
.include <bsd.gcc.mk>
