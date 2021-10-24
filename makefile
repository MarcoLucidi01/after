.POSIX:
.PHONY: all clean install uninstall

CC     = cc
CFLAGS = -std=c99 -pedantic -Wall -Wextra -Werror -D_DEFAULT_SOURCE
PREFIX = /usr/local

all: after

after: after.c
	$(CC) $(CFLAGS) -o $@ $^

install: after
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	install -m 755 after $(DESTDIR)$(PREFIX)/bin

uninstall:
	rm $(DESTDIR)$(PREFIX)/bin/after

clean:
	rm -f after
