dmg-emu is a Game Boy emulator written in C++ using SDL.

In order to run dmg-emu.exe, SDL2.dll must be in the same directory.
Options can be specified in a file called options.txt, which also
must be in the same directory.

Options:
--------
romPath:    the path to the rom file to be loaded.
palette:    the palette of the screen, currently selectable between
            "dmg" and "mgb" presets.
pixelScale: the integer that each pixel is scaled by on the
            screen. Minimum 1.
displayFPS: sets whether the fps is displayed. 0 = off, 1 = on.
debugMode:  sets whether debug mode is active, which displays
            additional graphics and audio data. 0 = off, 1 = on.
            pixelScale is ignored if this is set to 1.
logPath:    the path of the log file that will be written to during
            a log capture. Log capture can be toggled by pressing q
            while in debug mode.

Controls:
---------
dmg     |      kb
-----------------
a       |       z
b       |       x
start   |       a
select  |       s
up      |      up
down    |    down
left    |    left
right   |   right
