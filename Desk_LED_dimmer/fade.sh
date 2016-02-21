#!/bin/sh -e

microcom -p /dev/serial/by-id/usb-FTDI_FT232R_USB_UART_A9UHTFJF-if00-port0 | \
while IFS= read -r line; do printf '%s %s\n' "$(date '+%Y-%m-%d %H:%M:%S')" "$line"; done | \
tee -a /tmp/fade.log

#gawk '{ print strftime("[%Y-%m-%d %H:%M:%S]"), $0 }' | \

