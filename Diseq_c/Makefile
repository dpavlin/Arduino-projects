# Arduino Make file. Refer to https://github.com/sudar/Arduino-Makefile
# apt-get install arduino-mk

#ARDUINO_DIR=/opt/arduino
ARDUINO_SKETCHBOOK = ../

#MONITOR_PORT = /dev/serial/by-id/usb-1a86_USB2.0-Serial-if00-port0 

BOARD_TAG    = nano328

include /usr/share/arduino/Arduino.mk

serial:
	#microcom -s ${MONITOR_BAUDRATE} -p ${DEVICE_PATH} | uniq
	socat ${DEVICE_PATH},b${MONITOR_BAUDRATE} - | uniq

