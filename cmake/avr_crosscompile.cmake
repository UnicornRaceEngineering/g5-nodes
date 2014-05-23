#########################
# Cross compile for AVR #
#########################

# Set cmake to cross compiling
set(CMAKE_SYSTEM_NAME Generic)

# Set the cross compiler that we want to use
set(CMAKE_C_COMPILER avr-gcc)
#set(CMAKE_CXX_COMPILER avr-g++)

# search for programs in the build host directories
#set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

