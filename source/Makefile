VERSION = 0.1
CC = /usr/bin/g++
CFLAGS = -Wall -g -lrt -I ../include -L ../lib
LDFLAGS = -lpthread 

OBJ = ../lib/arduPi.a
SRC = senso.c
SPATH = /home/pi/senso/source
BPATH = /home/pi/senso/bin

senso: 
	$(CC) $(CFLAGS) -o senso $(SRC) $(OBJ) $(LDFLAGS)

install:
	mv $(SPATH)/senso $(BPATH)/senso
	chmod 774 $(BPATH)/senso

