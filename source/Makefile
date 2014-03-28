VERSION = 0.1
CC         = /usr/bin/g++
INCLUDE    = -I/usr/local/include -I ../include
CFLAGS     = -Wall $(INCLUDE) -g  -Winline -pipe 
LDFLAGS    = -L../lib -L/usr/local/lib
LDLIBS     = -lwiringPi -lwiringPiDev -lbcm2835 -lpthread -lm -lrt -Wwrite-strings

OBJ = $(SRC:.c=.o)
SRC = senso.c  config.c rfid.c rc522.c lcd_senso.c
SPATH = /home/pi/senso/source
BPATH = /home/pi/senso/bin

senso: $(OBJ)
	@echo [link]
	$(CC) $(CFLAGS) -o $@  $(OBJ) $(LDFLAGS) $(LDLIBS)

install:
	mv $(SPATH)/senso $(BPATH)/senso
	chmod 774 $(BPATH)/senso

