#
# Makefile for the malloc lab driver
#
CC = gcc
CFLAGS = -O2 -Wall -g
DRIVER = driver
OBJS = mm.o

all: mdriver

mdriver: $(OBJS)
	$(MAKE) -C $(DRIVER)
	$(CC) $(CFLAGS) -o mdriver $(OBJS) $(DRIVER)/*.o -lm

mm.o: mm.c mm.h

clean:
	rm -f *~ *.o driver/*.o mdriver
