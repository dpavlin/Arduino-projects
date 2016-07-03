#!/bin/sh -xe

BUILD=`pwd`/build

rm -rv $BUILD/* || mkdir $BUILD
/opt/arduino-1.6.8/arduino-builder -dump-prefs -logger=machine -hardware "/opt/arduino-1.6.8/hardware" -hardware "/home/dpavlin/.arduino15/packages" -tools "/opt/arduino-1.6.8/tools-builder" -tools "/opt/arduino-1.6.8/hardware/tools/avr" -tools "/home/dpavlin/.arduino15/packages" -built-in-libraries "/opt/arduino-1.6.8/libraries" -libraries "/home/dpavlin/Arduino/libraries" -fqbn=FPGArduino:f32c:fpga_tb276:cpu=mips,ramsize=16,speed=81,protocol=hex,pinmap=generic,jtag=alterablaster,bitstream=sram -ide-version=10608 -build-path "$BUILD" -warnings=none -prefs=build.warn_data_percentage=75 -verbose "/home/dpavlin/Arduino/tb276/tb276.ino"
/opt/arduino-1.6.8/arduino-builder -compile -logger=machine -hardware "/opt/arduino-1.6.8/hardware" -hardware "/home/dpavlin/.arduino15/packages" -tools "/opt/arduino-1.6.8/tools-builder" -tools "/opt/arduino-1.6.8/hardware/tools/avr" -tools "/home/dpavlin/.arduino15/packages" -built-in-libraries "/opt/arduino-1.6.8/libraries" -libraries "/home/dpavlin/Arduino/libraries" -fqbn=FPGArduino:f32c:fpga_tb276:cpu=mips,ramsize=16,speed=81,protocol=hex,pinmap=generic,jtag=alterablaster,bitstream=sram -ide-version=10608 -build-path "$BUILD" -warnings=none -prefs=build.warn_data_percentage=75 -verbose "/home/dpavlin/Arduino/tb276/tb276.ino"
