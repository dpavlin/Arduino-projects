#!/bin/sh -e

microcom -p /dev/ttyUSB1 | \
while IFS= read -r line; do printf '[%s] %s\n' "$(date '+%Y-%m-%d %H:%M:%S')" "$line"; done | \
tee -a /tmp/fade.log

#gawk '{ print strftime("[%Y-%m-%d %H:%M:%S]"), $0 }' | \

