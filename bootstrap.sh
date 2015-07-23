#!/usr/bin/env bash

sudo apt-get update

sudo apt-get install -y \
	avarice \
	avr-libc \
	avrdude \
	binutils-avr \
	gdb-avr \
	simulavr \
	avarice \
	make \
	cmake \
	g++ \
	gcc-avr \
	minicom \
	screen \
	cmake \
	build-essential \


ROOT_DIR=$PWD
cd third_party
bash get_fatfs.sh
cd $ROOT_DIR

mkdir -p build
cd build
cmake ..
make

echo "Done! :)"
