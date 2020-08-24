#!/usr/bin/env bash

make clean
make
echo
echo
echo
echo "chip8-emu build finished in the /build folder."
echo
echo "if dbus fails: run with \"DBUS_FATAL_WARNINGS=0 ./chip8-emu <blah>\""
echo
