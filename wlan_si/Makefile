all:
	echo "flash serial"

pi=10.60.1.201

flash:
	scp /tmp/build*.tmp/*.hex pi@${pi}:/tmp/arduino.hex
	ssh pi@${pi} sudo avrdude -c linuxgpio -p atmega328p -U flash:w:/tmp/arduino.hex

serial:
	ssh pi@${pi} microcom -p /dev/ttyAMA0 -s 9600
