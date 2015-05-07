NAME=fatfs
SRC_DIR=$PWD/src/$NAME
INCLUDE_DIR=$PWD/include/$NAME
INTERFACE_DIR=$PWD/interface/$NAME

mkdir -p $SRC_DIR
mkdir -p $INCLUDE_DIR

DL_DIR=dl/$NAME

mkdir -p $DL_DIR

pushd $DL_DIR

wget http://elm-chan.org/fsw/ff/ff11.zip
yes A | unzip ff11.zip
rm ff11.zip

popd

# Move the library source into the src dir
mv $DL_DIR/src/* $SRC_DIR

# we link all headers to the include dir
ln -s $SRC_DIR/*.h $INCLUDE_DIR

# and lastly we link our interface sources to the library sources, overwriting
# existing files
ln -sf $INTERFACE_DIR/*.c $SRC_DIR
ln -sf $INTERFACE_DIR/*.h $SRC_DIR

# Cleanup
rm -r $DL_DIR
rm -r dl
