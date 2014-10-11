#!/usr/bin/env bash

cat <<EOF
*****************************

Bootstrapping the VM
for embedded avr development

*****************************
EOF

HOME_FOLDER=/home/vagrant

apt-get update

apt-get install -y \
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


compile_avr_gcc() {
	URL=$1
	TAR_FILE=${URL##*/}
	GCC_FOLDER=${TAR_FILE%%.tar*}

	apt-get remove avr-gcc

	echo "We compile avr-gcc ourselves"
	cd $HOME_FOLDER
	wget $URL
	tar xf $TAR_FILE

	echo "Get GCC prerequisites"

	cd $GCC_FOLDER
	./contrib/download_prerequisites
	cd ..

	mkdir build

	cd build

	../$GCC_FOLDER/configure \
		--enable-lto \
		--enable-languages=c,c++ \
		--target=avr \
		--with-avrlibc \
		--disable-nls \
		--disable-libssp \
		--prefix=/usr/lib \
		# --with-sysroot="/usr/lib/avr" \


	make
	make install

	echo "Cleaning up"
	cd $HOME_FOLDER
	rm -r build $GCC_FOLDER $TAR_FILE*
}

# If we want to compile avr-gcc ourselves uncomment the line below
# compile_avr_gcc ftp://ftp.gwdg.de/pub/misc/gcc/releases/gcc-4.9.1/gcc-4.9.1.tar.bz2

ln -s /vagrant $HOME_FOLDER/g5-nodes

cat <<EOF
*****************************

Bootstrapping Done
EOF


cat >$HOME_FOLDER/README <<EOL
The vm should now be ready to be used for embedded avr development.

Since the vm is created using vagrant the folder /vagrant is syncronized with
the development folder on the host. This means you can do the development on the
host using your favorit editor and compiled in this vm. In the homefolder you
will see a symlink called "g5-nodes" to the /vagrant folder.
EOL

cat $HOME_FOLDER/README

cat << EOF

*****************************
EOF
