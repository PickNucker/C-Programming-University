CC=gcc
CFLAGS=-std=c11 -pedantic -Wall -Werror -D_XOPEN_SOURCE=700

all: clash

clash: clash.o plist.o
	${CC} ${CFLAGS} -o clash clash.o plist.o

clash.o: clash.c plist.h
	${CC} ${CFLAGS} -c clash.c

plist.o: plist.c plist.h
	${CC} ${CFLAGS} -c plist.c

clean:
	rm -f clash *.o