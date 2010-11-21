CC=gcc
CFLAGS=-Wall `pkg-config --cflags gtk+-2.0`
LIBS=`pkg-config --libs gtk+-2.0`
bindir ?= /usr/bin
mandir ?= /usr/share/man

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $<

g3data: main.o sort.o points.o drawing.o
	$(CC) $(CFLAGS) -o g3data main.o sort.o points.o drawing.o $(LIBS)
	strip g3data

main.o: main.c main.h strings.h

sort.o: sort.c main.h

points.o: points.c main.h

drawing.o: drawing.c main.h

clean:
	rm *.o g3data

install:
	install g3data $(bindir)
	install g3data.1.gz $(mandir)/man1

uninstall:
	rm $(bindir)/g3data
