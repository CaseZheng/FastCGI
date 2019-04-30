#!/usr/bin/sh

mkdir build -p 
cd build 
#rm ./* -rf
cmake ..
make
cd ..
