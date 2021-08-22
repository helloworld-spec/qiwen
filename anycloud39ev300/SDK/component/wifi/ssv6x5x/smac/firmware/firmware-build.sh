#!/bin/bash
./ver_firmware.pl include/ssv_version.h
./ver_firmware.pl ../../include/ssv_firmware_version.h
make config TARGET=6051.cfg
make scratch
echo "Done ko!"
