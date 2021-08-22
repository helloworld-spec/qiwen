#!/bin/bash
./ver_firmware.pl include/ssv_version.h
./ver_firmware.pl ../../../../include/ssv_firmware_version.h
make config TARGET=6006_fpga.cfg
make scratch
make install
echo "Done ko!"
