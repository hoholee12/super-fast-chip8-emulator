#!/usr/bin/env bash

make clean
make
./chip8-emu pong.ch8
