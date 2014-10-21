#!/bin/sh

MK=m8
PRG=avr910
BAUD=57600
PORT=/dev/ttyACM0
FLASHFILE=./Debug/lamp-reminder.hex

sudo avrdude -p $MK -c $PRG -b $BAUD -P $PORT -U flash:w:$FLASHFILE

# Manual run:
# sudo avrdude -p m8 -c avr910 -b 57600 -P /dev/ttyACM0 -U flash:w:"./Debug/mega8_wifibot.hex":i
