set(F_CPU "-DF_CPU=11059200")
set(CAN_BAUDRATE "-DCAN_BAUDRATE=250")

# Name of target Micro Controller Unit
# see the available avr-gcc mmcur options for possible values
set(CMCU "-mmcu=at90can128")

set(CDEFS "${F_CPU} ${CAN_BAUDRATE}")

set(CSTANDARD "-std=c99")
set(CDEBUG "-g")
set(CWARN "-Wall -Wstrict-prototypes -Werror")
set(CTUNING "-fpack-struct -fshort-enums -funsigned-bitfields -funsigned-char -flto -mrelax")

set(COPT "-Os")

set(CFLAGS "${CMCU} ${CDEFS} ${CINCS} ${CSTANDARD} ${CDEBUG} ${CWARN} ${CTUNING} ${COPT} ${CEXTRA}")
set(CXXFLAGS "${CMCU} ${CDEFS} ${CINCS} ${COPT}")

set(CMAKE_C_FLAGS ${CFLAGS})
set(CMAKE_CXX_FLAGS ${CXXFLAGS})

set(CORELIBS m printf_flt)
