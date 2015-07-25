g5-nodes
========

Software for nodes on the g5 car

Structure
---------
The important folders in the project are:
- `nodes` Contains source for the hardware nodes
- `libat90` Contains hardware abstraction shared between all nodes
- `drivers` Contains drivers for external hardware components
- `examples` Contains examples

Build
-----
The project requires [cmake](http://www.cmake.org/) and a working avr-gcc compiler  to build

When building for the first time you should first run `bash bootstrap` (should only be run once).
This will install all required tools (if you are on ubuntu-linux), create a build directory and build the project.

The project is now ready to be build from the `build` directory.

```
cd build    # Enter the build directory
cmake ..    # Configure the project using cmake (.. is the root folder for the project)
make help   # Print all possible targets
make ComNode_writeflash # Program the ComNode
```

All targets that end in `_writeflash` is used to program a node
