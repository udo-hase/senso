VERSION = 0.1
CC         = /usr/bin/g++
INCLUDE    = -I/usr/local/include -I ../include
CFLAGS     = -Wall $(INCLUDE) -g  -Winline -pipe
LDFLAGS    = -L../lib -L/usr/local/lib
LDLIBS     = -lwiringPi -lwiringPiDev -lpthread -lm -lrt

OBJ = ../lib/arduPi.a
SRC = senso.c menu.c
SPATH = /home/pi/senso/source
BPATH = /home/pi/senso/bin

senso: senso.o
	@echo [link]
	$(CC) $(CFLAGS) -o $@ senso.o $(OBJ) $(LDFLAGS) $(LDLIBS)

menu: menu.o
	@echo [link]
	$(CC) $(CFLAGS) -o $@ menu.o  $(LDFLAGS) $(LDLIBS)

install:
	mv $(SPATH)/senso $(BPATH)/senso
	chmod 774 $(BPATH)/senso

