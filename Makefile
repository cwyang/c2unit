#
# Makefile               Chul-Woong Yang (cwyang at gmail.com)
#

CC = gcc

LIBNAME = libc2unit.a
FSTLNK = firstlink.o
TARGET = $(LIBNAME) $(FSTLNK)
LIB_CFILES = c2unit.c firstlink.c

$(TARGET): c2unit.o
	$(CC) $(CCFLAGS) -c firstlink.c -o $(FSTLNK)
	ar q $(LIBNAME) c2unit.o
	ranlib $(LIBNAME)

c2unit.o: c2unit.c c2unit.h
	$(CC) $(CCFLAGS) -c c2unit.c -o $@

test: test.c $(TARGET)
	$(CC) -c test.c -o test.o
	$(CC) $(FSTLNK) test.o $(LIBNAME) -o $@

clean:
	rm -f $(TARGET) *.o test *~
