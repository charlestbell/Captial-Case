#!/bin/sh
# Builds CapitalCase.exe with the MinGW-w64 cross compiler (Linux) or MinGW on Windows.
set -e
cd "$(dirname "$0")"
x86_64-w64-mingw32-windres app.rc -O coff -o app.res.o
x86_64-w64-mingw32-gcc -O2 -s -static -municode -mwindows -Wall -o ../CapitalCase.exe main.c app.res.o -luser32 -lgdi32
rm -f app.res.o
