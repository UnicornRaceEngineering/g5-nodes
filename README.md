g5-nodes
========

Software for nodes on the g5 car

Build
-----
Requires [cmake](http://www.cmake.org/) to build

```
mkdir build
cd build
cmake ..
make
```

to flash the node boards `cd` to the node in the build dir and do `make ${NODE_NAME}_writeflash`. You can see all possible targets with `make help`
