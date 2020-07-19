#ARDUINO_DIR=/opt/arduino
ARDUINO_SKETCHBOOK = ../

BOARD_TAG    = pro5v328
include /usr/share/arduino/Arduino.mk

pi=10.60.1.201

flash:
	scp `ls -t build-*/*.hex | head -1` pi@${pi}:/tmp/arduino.hex
	ssh pi@${pi} sudo avrdude -c linuxgpio -p atmega328p -U flash:w:/tmp/arduino.hex

serial:
	ssh pi@${pi} microcom -p /dev/ttyAMA0 -s 9600
