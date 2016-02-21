#!/bin/sh -xe

xrandr -q

xset dpms force on

#tail -f /tmp/fade.log | grep = | sed -e 's/\t/\n/g' -e 's/^.*LDR = /0:/' -e 's/^.*PIR = /1:/' -e 's/^.*X=/2:/' -e 's/Y=/3:/' -e 's/Z=/4:/' | grep :

#microcom -p /dev/serial/by-id/usb-FTDI_FT232R_USB_UART_A9UHTFJF-if00-port0 | \
#picocom -b 115200 /dev/serial/by-id/usb-FTDI_FT232R_USB_UART_A9UHTFJF-if00-port0 | \

socat - /dev/serial/by-id/usb-FTDI_FT232R_USB_UART_A9UHTFJF-if00-port0,nonblock,raw,echo=0 \
| tee /dev/stderr \
| grep --line-buffered = \
| sed -e 's/\t/\n/g' -e 's/^.*LDR = /0:/' -e 's/^.*PIR = /1:/' -e 's/^.*X=/2:/' -e 's/Y=/3:/' -e 's/Z=/4:/' \
| grep --line-buffered : \
| ~/cheali-logview-gnuplot/driveGnuPlots.pl 5 10 10 300 300 300 LDR PIR X Y Z
