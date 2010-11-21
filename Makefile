CC=gcc 
CFLAGS=-Wall `gtk-config --cflags` -I/usr/include/gdk-pixbuf-1.0/
LIBS=`gtk-config --libs` -lgdk_pixbuf

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $<

g3data: g3data.o
	$(CC) $(CFLAGS) -o g3data g3data.o $(LIBS)
	strip g3data

g3data.o: g3data.c tooltips.h

clean:
	rm *.o g3data

install:
	cp g3data /usr/local/bin

uninstall:
	rm /usr/local/bin/g3data
