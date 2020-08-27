#!/usr/bin/env bash

./build.sh

objdump -f -d --disassembler-options=intel --source ./build/chip8-emu > dump.txt

echo "dump.txt generated!"