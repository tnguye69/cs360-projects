# CS360 Class Manager
# August 29, 2005
CC = gcc
CLASSDIR = /home/huangj/cs360
CDIR = /home/cs360
INCLUDES = $(CLASSDIR)/include
CFLAGS = -g -I$(INCLUDES)
LIBDIR = $(CDIR)/pub
LIBS = $(LIBDIR)/libfdr.a
EXECUTABLES = famtree

all: $(EXECUTABLES)

clean:
	rm -f core $(EXECUTABLES) *.o

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $*.c

famtree: famtree.o
	$(CC) -g -o famtree famtree.o $(LIBS)
