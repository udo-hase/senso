--> für den GPS Empfaenger muss in der ardulib in der initialisierung
	4800 als Speed eingefügt werden!!!!
    --> RX vom GPS Board an RX vom Shield oder Raspberry
    --> TX vom GPS Board an TX vom Shield oder Raspberry
    --> 5V vom ArduShield, Mini Schalter beachten (Powersource Switch)

--> Compile the ardulib
rduPi.cpp -o arduPi.a


--> temperatursensor -- treiber module fuer onewire müssen geladen werden!!!
    sudo modprobe w1-gpio
    sudo modprobe w1-therm
    cd /sys/bus/w1/devices
    ls
    cd 28-xxxx (change this to match what serial number pops up)
    cat w1_slave

    --> Anschluss der Datenleitung der DS18B20 an RASPI GPIO PIN4
    --> schwarz (Ground) rot (5V)
