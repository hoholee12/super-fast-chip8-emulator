# super-fast-chip8-emulator

## Info

your average emulator using c++ and sdl2.0

tried to make it as simple as possible.

interpreter is done. recompiler is in the works

-auto frameskip, auto reduce cycles(because why not)

-video deflicker! - new method to parse opcode patterns. all games ive tested have *zero* flickering now.

-upto 4mhz(4000000hz) in 60fps for intel 5th gen 3.1ghz cpu(msvc2013-debug)

-upto 250mhz(250000000hz) in 60fps on ubuntu linux

(performance improved alot after implementing time scheduling)

## How to compile

### before compiling:

install/reinstall 32bit libs

```
sudo apt install libsdl2-dev:i386 libsdl2-2.0-0:i386 -y;

sudo apt install libjpeg-dev:i386 libwebp-dev:i386 libtiff5-dev:i386 libsdl2-image-dev:i386 libsdl2-image-2.0-0:i386 -y;

sudo apt install libmikmod-dev:i386 libfishsound1-dev:i386 libsmpeg-dev:i386 liboggz2-dev:i386 libflac-dev:i386 libfluidsynth-dev:i386 libsdl2-mixer-dev:i386 libsdl2-mixer-2.0-0:i386 -y;

sudo apt install libfreetype6-dev:i386 libsdl2-ttf-dev:i386 libsdl2-ttf-2.0-0:i386 -y;

sudo apt install g++-multilib -y;

sudo apt install pkg-config:i386 -y;
```

### to compile:

simply run ./build.sh

and do as it says to run it.

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

## Reference?

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
