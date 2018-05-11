BOARD_TAG=pro328
ARDUINO_SKETCHBOOK = ../


include /usr/share/arduino/Arduino.mk


all:
	echo "flash serial"

flash:
	scp build-*/*.hex pi@rpi:/tmp/
	ssh pi@rpi /home/pi/avrdude.sh

serial:
	ssh pi@rpi microcom -p /dev/ttyAMA0 -s 9600
