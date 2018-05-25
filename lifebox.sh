#!/bin/bash

echo "Creating lifebox directory"
mkdir rpi-rgb-led-matrix/lifebox/
echo "Download lifebox.cc"
wget https://github.com/lycros/lifebox/raw/master/lifebox.cc
echo "Download Makefile"
wget https://github.com/lycros/lifebox/raw/master/Makefile
echo "Moving Files"
mv lifebox.cc rpi-rgb-led-matrix/lifebox/
mv Makefile rpi-rgb-led-matrix/lifebox/
make
echo "End Script"
