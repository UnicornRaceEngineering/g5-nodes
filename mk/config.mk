# Name of target controller
# (e.g. 'at90s8515', see the available avr-gcc mmcu
# options for possible values)
MCU=at90can128

# id to use with programmer
# default: PROGRAMMER_MCU=$(MCU)
# In case the programer used, e.g avrdude, doesn't
# accept the same MCU name as avr-gcc (for example
# for ATmega8s, avr-gcc expects 'atmega8' and
# avrdude requires 'm8')
PROGRAMMER_MCU=c128

F_CPU=11059200
CAN_BAUDRATE=250

DEFINES=-DF_CPU=$(F_CPU) -DCAN_BAUDRATE=$(CAN_BAUDRATE)

# Source files
# List C/C++/Assembly source files:
# (list all files to compile, e.g. 'a.c b.cpp as.S'):
# Use .cc, .cpp or .C suffix for C++ files, use .S
# (NOT .s !!!) for assembly source code files.
LIB_AT90_DIR=$(TOP)/libat90
LIB_AT90=$(wildcard $(LIB_AT90_DIR)/*.c)

PRJSRC=$(SRC) $(LIB_AT90)

# additional includes (e.g. -I/path/to/mydir)
INC=-I$(LIB_AT90_DIR)

# libraries to link in (e.g. -lmylib)
#LIBS=

# Optimization level,
# use s (size opt), 1, 2, 3 or 0 (off)
OPTLEVEL=s


# programmer id--check the avrdude for complete list
# of available opts.  These should include stk500,
# avr910, avrisp, bsd, pony and more.  Set this to
# one of the valid "-c PROGRAMMER-ID" values
# described in the avrdude info page.
#
AVRDUDE_PROGRAMMERID=avrispmkii#usbtiny#

# port--serial or parallel port to which your
# hardware programmer is attached
#
#AVRDUDE_PORT=/dev/ttyUSB0

