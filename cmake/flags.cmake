set(F_CPU 11059200)
set(CAN_BAUDRATE 250)

option(WITH_LTO
	"Use Link Time Optimizations" ON
)

if (WITH_LTO)
	set(LTO "-flto")
	message(STATUS "Compiling with Link Time Optimizations - LTO ON")
else()
	message(STATUS "Compiling with out Link Time Optimizations - LTO OFF")
endif (WITH_LTO)

# Name of target Micro Controller Unit
# see the available avr-gcc mmcur options for possible values
set(CMCU "-mmcu=at90can128")

set(CSTANDARD "-std=c99")
set(CDEBUG "-g")
set(CWARN "-Wall -Wstrict-prototypes -Werror") # TODO: add -Wextra and -pedantic
set(CTUNING "-fpack-struct -fshort-enums -funsigned-bitfields -funsigned-char ${LTO} -mrelax")

set(COPT "-Os")

add_definitions(-DF_CPU=${F_CPU} -DCAN_BAUDRATE=${CAN_BAUDRATE})
set(CFLAGS "${CMCU} ${CINCS} ${CSTANDARD} ${CDEBUG} ${CWARN} ${CTUNING} ${COPT} ${CEXTRA}")
set(CXXFLAGS "${CMCU} ${CINCS} ${COPT}")

# Without the "CACHE STRING "" FORCE" the CMAKE_C_FLAGS variable is empty when
# entering subdirectories (where we want to do the build). The only other work
# around was to do `make cache_rebuild` right after running `cmake ..` but we
# don't want to put the work on the user to do this.
set(CMAKE_C_FLAGS ${CFLAGS} CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS ${CXXFLAGS} CACHE STRING "" FORCE)
