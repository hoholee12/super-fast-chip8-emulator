#!/usr/bin/env bash

make clean
make
retCode=$?
echo
echo
echo
if [ $retCode -eq 0 ]; then    
    echo "chip8-emu build finished in the /build folder."
    echo
    echo "if dbus fails: run with \"DBUS_FATAL_WARNINGS=0 ./chip8-emu <blah>\""
    echo
else
    echo "failed!"
    echo
fi
exit $retCode