VERSION = 0.2

PREFIX ?= /usr/local

CFLAGS  = -std=c99 -Wall -Wextra -pedantic -pedantic-errors
LDFLAGS = -lpthread

CC = gcc

SOURCES = md5cracker.c md5.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = md5cracker

all: showoptions $(EXECUTABLE)

showoptions:
	@echo "${EXECUTABLE} build options:"
	@echo "CFLAGS = ${CFLAGS}"
	@echo "LDFLAGS = ${LDFLAGS}"
	@echo "CC = ${CC}"
	@echo

$(EXECUTABLE): $(OBJECTS)
	@echo "CC -o $@"
	@$(CC) $(LDFLAGS) $? -o $@

.c.o:
	@echo "CC    $<"
	@$(CC) -c $(CFLAGS) $<

clean:
	@echo "cleaning"
	@$(shell rm *.o $(EXECUTABLE) 2>/dev/null)

dist: clean
	@echo "Creating dist tarball"
	@mkdir -p ${EXECUTABLE}-${VERSION}
	@cp -R Makefile ${SOURCES} md5.h COPYING ${EXECUTABLE}-${VERSION}
	@tar -cf ${EXECUTABLE}-${VERSION}.tar ${EXECUTABLE}-${VERSION}
	@gzip ${EXECUTABLE}-${VERSION}.tar
	@rm -rf ${EXECUTABLE}-${VERSION}

install: all
	@echo "Installing executable to ${DESTDIR}${PREFIX}/bin"
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f ${EXECUTABLE} ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/${EXECUTABLE}

uninstall:
	@echo "Removing executable from ${DESTDIR}${PREFIX}/bin"
	@rm -f ${DESTDIR}${PREFIX}/bin/${EXECUTABLE}

.PHONY: clean all showoptions install uninstall dist
