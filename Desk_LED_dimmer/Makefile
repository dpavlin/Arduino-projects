# Arduino Make file. Refer to https://github.com/sudar/Arduino-Makefile
# apt-get install arduino-mk

#ARDUINO_DIR=/opt/arduino
ARDUINO_SKETCHBOOK = ../

MONITOR_PORT = /dev/serial/by-id/usb-FTDI_FT232R_USB_UART_A9UHTFJF-if00-port0

BOARD_TAG    = pro5v328

include /usr/share/arduino/Arduino.mk

serial:
	microcom -p ${DEVICE_PATH}
