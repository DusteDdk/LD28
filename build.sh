#!/bin/bash
set -e
cd libeo
make -f Makefile.linux ARCH=x86 debug
cd ..
make -f Makefile.linux ARCH=x86 debug
./game_x86.bin 1280 800
