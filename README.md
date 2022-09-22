# Space Invaders

This is a program written in C intended to emulate the old Space Invaders Arcade Cabinets.

## Compilation

Just running `make` should work. Though I have not tested this on Windows but I am relatively positive it should work fine on there as well.


## Usage

```
./emulator invaders.rom
```

## Disclaimer
This emulator requires a rom file to run, they are typically broken up into `invaders.e`, `invaders.f`, `invaders.g` and `invaders.h`. To create a rom file from this you have to run the command
```
   cat invaders.h > invaders.rom
   cat invaders.g >> invaders.rom
   cat invaders.f >> invaders.rom
   cat invaders.e >> invaders.rom
```
to get the file you need for the emulator.
