# chip8-emulator

## Info

your average emulator using c++ and sdl2.0

tried to make it as simple as possible.

interpreter is done. recompiler is in the works

-auto frameskip, auto reduce cycles(because why not)

-video deflicker! - new method to parse opcode patterns. all games ive tested have *zero* flickering now.

-upto 4mhz(4000000hz) in 60fps for intel 5th gen 3.1ghz cpu(msvc2013-debug)

-upto 250mhz(250000000hz) in 60fps on ubuntu linux

(performance improved alot after implementing time scheduling)

## How to play

pass your rom location to program parameter.

type 'y' if you wish to custom setup interpreter method, clockspeed, fps, and flicker offset!

keyinput goes like this:

|||||
|---|---|---|---|
|1|2|3|4|
|q|w|e|r|
|a|s|d|f|
|z|x|c|v|

## Build?

-this code requires sdl2 and sdl2_mixer to compile(look into defaults.h if you encounter location problems)

-you also need data/sound/klik.wav in same directory format for sound.

please help me make my project run faster :D

this project follows:
http://devernay.free.fr/hacks/chip8/C8TECH10.HTM

## Options

ive implemented three different interpreter methods (switchcase/partitionizedLUT/bigLUT)

theres also an option to change clockspeed and fps.

you can change these options when loading the game.

### Demonstration:

[![](http://img.youtube.com/vi/dMV2lnNoxGg/0.jpg)](http://www.youtube.com/watch?v=dMV2lnNoxGg "https://i9.ytimg.com/vi/dMV2lnNoxGg/mq2.jpg?sqp=CKX-hfUF&rs=AOn4CLByAtLQyftORU5wL503Yl857I1f5A")

#### Have fun!

hoholee12@naver.com
