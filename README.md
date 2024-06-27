## Description
Creates a report of all duplicate files in a given directory and its subdirectories. Compile by running `make`. The executable ouput is called `finddup`. Run the command to see available flags and parameters.

## Required setup
You'll need C++17, OpenSSL and `pkg-config` installed. Older versions of g++ may require the `-lstdc++fs` flag. If so, add it to the definition of `LDFLAGS` in the makefile.

## Compile-time constants
`MAX_WIDTH` (in `src/main.cpp`): Maximum width of the name column, in characters. This column contains the paths of each duplicate file found; it appears when viewing the duplicate file report. Adjust this constant as needed to fit your terminal width.
