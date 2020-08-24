#!/usr/bin/env bash

make clean
make
echo "chip8-emu build finished in the /build folder. run with DBUS_FATAL_WARNINGS=0 ./chip8-emu <blah>"
