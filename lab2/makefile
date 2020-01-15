# CS360 Class Manager
# August 29, 2005
CC = gcc
CLASSDIR = /home/huangj/cs360
CDIR = /home/cs360
INCLUDES = $(CLASSDIR)/include
CFLAGS = -g -I$(INCLUDES)
LIBDIR = $(CDIR)/pub
LIBS = $(LIBDIR)/libfdr.a
EXECUTABLES = l2p1 l2p2 l2p3

all: $(EXECUTABLES)

clean:
	rm -f core $(EXECUTABLES) *.o

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $*.c

l2p1: l2p1.o
	$(CC) -g -o l2p1 l2p1.o $(LIBS)

l2p2: l2p2.o
	$(CC) -g -o l2p2 l2p2.o $(LIBS)

l2p3: l2p3.o
	$(CC) -g -o l2p3 l2p3.o $(LIBS)
