#!/bin/sh

MK=m8
BAUD=57600
PORT=/dev/ttyACM0
FLASHFILE=./Debug/lamp-reminder.hex
EEPROMFILE=./Debug/lamp-reminder.eep

# avr910 programmer
PRG=avr910
#sudo avrdude -p $MK -c $PRG -b $BAUD -P $PORT -U flash:w:$FLASHFILE

# usbasp programmer
PRG=usbasp
sudo avrdude -p $MK -c $PRG -U flash:w:$FLASHFILE -U eeprom:w:$EEPROMFILE

# Manual run:
# sudo avrdude -p m8 -c avr910 -b 57600 -P /dev/ttyACM0 -U flash:w:"./Debug/mega8_wifibot.hex":i
