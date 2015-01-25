all:
	echo "flash serial"

flash:
	scp /tmp/build*.tmp/*.hex pi@rpi.tv:/tmp/
	ssh pi@rpi.tv /home/pi/avrdude.sh

serial:
	ssh pi@rpi.tv microcom -p /dev/ttyAMA0 -s 9600
