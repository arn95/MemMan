#
# Makefile for the malloc lab driver
#
CC = gcc
CFLAGS = -O2 -Wall -g

OBJS = mdriver.o memlib.o pagemap.o fsecs.o fcyc.o clock.o ftimer.o

 all: $(OBJS)

mdriver.o: mdriver.c fsecs.h fcyc.h clock.h memlib.h config.h
memlib.o: memlib.c memlib.h pagemap.h
pagemap.o: pagemap.c pagemap.h
fsecs.o: fsecs.c fsecs.h config.h
fcyc.o: fcyc.c fcyc.h
ftimer.o: ftimer.c ftimer.h config.h
clock.o: clock.c clock.h

clean:
	rm -f *~ *.o
