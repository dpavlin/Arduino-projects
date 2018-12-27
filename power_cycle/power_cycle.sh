#!/bin/sh -xe

dev=/dev/ttyUSB4

trap "exit" INT TERM
trap "kill 0" EXIT

stty -F $dev speed 115200 raw
cat < $dev &
(
	echo
	sleep 3
        echo 0
        sleep 3
        echo 1
        sleep 1
) | cat > $dev

kill $!
