CC=gcc
CFLAGS=-Wall `pkg-config --cflags gtk+-2.0`
LIBS=`pkg-config --libs gtk+-2.0`
bindir ?= /usr/bin
mandir ?= /usr/share/man

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $<

g3data: main.o sort.o points.o drawing.o g3data.1.gz
	$(CC) $(CFLAGS) -o g3data main.o sort.o points.o drawing.o $(LIBS)
	strip g3data

main.o: main.c main.h strings.h vardefs.h

sort.o: sort.c main.h

points.o: points.c main.h

drawing.o: drawing.c main.h

g3data.1.gz: g3data.sgml
	rm -f *.1
	onsgmls g3data.sgml | sgmlspl /usr/share/sgml/docbook/utils-0.6.14/helpers/docbook2man-spec.pl
	gzip g3data.1

clean:
	rm -f *.o g3data g3data.1.gz *~ manpage.*

install:
	install g3data $(bindir)
	install g3data.1.gz $(mandir)/man1

uninstall:
	rm $(bindir)/g3data
