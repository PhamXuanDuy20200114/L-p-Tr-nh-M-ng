TARGET = resolver

SOURCES = resolver.c

CC = gcc

CFLAGS = -Wall

LDFLAGS = -lcurl

all: ${TARGET}

${TARGET}: ${SOURCES}
	${CC} ${CFLAGS} -o ${TARGET} ${SOURCES} ${LDFLAGS}

clean:
	rm -f ${TARGET}
