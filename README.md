# TermMine
TermMine is an implementation of the game of Minesweeper that you can play in
your terminal.

## Obtaining
### Binaries
Navigate to the releases page and download the appropriate binary for your
system. There is currently support for Linux (termmine) and
Windows (termmine.exe).

### Building from source
Make sure that you have the following installed:
* CMake
* Ninja
* ncurses (MinGW port on Windows)

Download the source code and navigate to the appropriate directory.
```sh
git clone https://github.com/ericw31415/termmine.git
cd termmine
```

Then, generate the buildsystem and build the executable.

Linux
```sh
cmake -S . -B build -G "Ninja Multi-Config" -DCMAKE_CXX_COMPILER=/usr/bin/g++
cmake --build build --config Release
```

Linux cross-compile for Windows
```sh
cmake -S . -B build -G "Ninja Multi-Config" \
    -DCMAKE_CXX_COMPILER=/usr/bin/x86_64-w64-mingw32-g++ \
    -DCMAKE_SYSROOT=/usr/x86_64-w64-mingw32 -DCMAKE_SYSTEM_NAME=Windows
cmake --build build --config Release
```

Windows
```sh
cmake -S . -B build -G "Ninja Multi-Config" ^
    -DCMAKE_CXX_COMPILER=C:\MinGW\bin\x86_64-w64-mingw32-g++ ^
    -DCMAKE_SYSROOT=C:\MinGW -DCMAKE_SYSTEM_NAME=Windows
cmake --build build --config Release
```

The executable file will be built to the `bin/Release` subdirectory.

## Playing
It is not necessary to run the program from the terminal. Double-clicking the
executable should automatically open it there. Controls are as follows:  
<kbd>↑</kbd><kbd>↓</kbd><kbd>←</kbd><kbd>→</kbd>—Movement keys  
<kbd>⏎ Enter</kbd>—Select menu option  
<kbd>1</kbd>—Flag cell  
<kbd>2</kbd>—Mark cell  
<kbd>Space</kbd>—Open cell/chord  
<kbd>Ctrl</kbd>+<kbd>Q</kbd>—Quit game
