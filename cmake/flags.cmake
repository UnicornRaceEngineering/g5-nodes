set(F_CPU 11059200)
set(CAN_BAUDRATE 250)

# Name of target Micro Controller Unit
# see the available avr-gcc mmcur options for possible values
set(CMCU "-mmcu=at90can128")

set(CSTANDARD "-std=c99")
set(CDEBUG "-g")
set(CWARN "-Wall -Wstrict-prototypes -Werror") # TODO: add -Wextra and -pedantic
set(CTUNING "-fpack-struct -fshort-enums -funsigned-bitfields -funsigned-char -flto -mrelax")

set(COPT "-Os")

add_definitions(-DF_CPU=${F_CPU} -DCAN_BAUDRATE=${CAN_BAUDRATE})
set(CFLAGS "${CMCU} ${CINCS} ${CSTANDARD} ${CDEBUG} ${CWARN} ${CTUNING} ${COPT} ${CEXTRA}")
set(CXXFLAGS "${CMCU} ${CINCS} ${COPT}")

set(CMAKE_C_FLAGS ${CFLAGS})
set(CMAKE_CXX_FLAGS ${CXXFLAGS})
