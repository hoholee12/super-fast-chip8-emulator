#!/usr/bin/env bash

cd src
make clean
make
mv -f chip8-emu ../chip8-emu
cd ..
cp -f testroms/pong.ch8 pong.ch8


./chip8-emu pong.ch8
