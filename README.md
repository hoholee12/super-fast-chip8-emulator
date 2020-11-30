# super-fast-chip8-emulator

## Info

your average emulator using c++ and sdl2.0

tried to make it as simple as possible.

-auto frameskip

-video deflicker

-recompiler(slower than interpreter atm)

## How to compile

### what does it use?

```
uses GLAD for OpenGL implementation
uses orconut's Dear Imgui for user interface
uses SDL2 for everything else interface related

all rights go to their respective owners.
```
### before compiling:

install/reinstall SDL2 32bit libs

```
sudo apt install libsdl2-dev:i386 libsdl2-2.0-0:i386 -y;

sudo apt install pkg-config:i386 -y;
```

### to compile:

simply run: ./build.sh

generated executable will be in: /build folder.

if dbus fails, run with: 
```DBUS_FATAL_WARNINGS=0 ./chip8-emu```

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

ive implemented three different interpreter methods (switchcase/partitionizedLUT/bigLUT), and a recompiler(WIP)

theres also an option to change clockspeed and fps.

you can change these options when loading the game.

### Demonstration:

[![](http://img.youtube.com/vi/dMV2lnNoxGg/0.jpg)](http://www.youtube.com/watch?v=dMV2lnNoxGg "https://i9.ytimg.com/vi/dMV2lnNoxGg/mq2.jpg?sqp=CKX-hfUF&rs=AOn4CLByAtLQyftORU5wL503Yl857I1f5A")

#### Have fun!

hoholee12@naver.com
