#CS 360 Lab 7 Makefile
CC = gcc 
INCLUDES = -I/home/jplank/cs360/include
CFLAGS = -g -Wall $(INCLUDES)
LIBDIR = /home/jplank/cs360/objs
LIBS = $(LIBDIR)/libfdr.a 
EXECUTABLES = jsh jsh1 jsh2 jsh3

all: $(EXECUTABLES)

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $*.c

jsh: jsh.o
	$(CC) $(CFLAGS) -o jsh jsh.o $(LIBS)

jsh1: jsh1.o
	$(CC) $(CFLAGS) -o jsh1 jsh1.o $(LIBS)

jsh2: jsh2.o
	$(CC) $(CFLAGS) -o jsh2 jsh2.o $(LIBS)

jsh3: jsh3.o
	$(CC) $(CFLAGS) -o jsh3 jsh3.o $(LIBS)

#make clean will rid your directory of the executable,
#object files, and any core dumps you've caused
clean:
	rm core $(EXECUTABLES) *.o
