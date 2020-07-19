#!/bin/sh -xe

/opt/vc/bin/tvservice -s
/opt/vc/bin/tvservice -p

pidof X || X -ac -listen tcp :1 && export DISPLAY=:1

xrandr -q

xset dpms force on

socat - /dev/serial/by-id/usb-FTDI_FT232R_USB_UART_A9UHTFJF-if00-port0,nonblock,raw,echo=0 \
| tee /dev/stderr \
| grep --line-buffered = \
| sed -e 's/\t/\n/g' -e 's/MOSFET=//' -e 's/X=/3:/' -e 's/Y=/4:/' -e 's/Z=/5:/' \
| grep --line-buffered : \
| ~/cheali-logview-gnuplot/driveGnuPlots.pl 6 \
300 300 300 \
300 300 300 \
MOSFET_0 MOSFET_1 MOSFET_2, \
X Y Z \

# leave empty line above

