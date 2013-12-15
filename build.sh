#!/bin/bash
set -e
cd libeo
make -f Makefile.linux ARCH=$1 debug
cd ..
make -f Makefile.linux ARCH=$1 debug
./game_"$1".bin 
