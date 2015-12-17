set(F_CPU 11059200)
set(CAN_BAUDRATE 204800)

# In relation to GCC Bugzilla – Bug 59396
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=59396
# Using gcc with link time optimizations when compiling code with ISR functions
# leads to gcc throwing a warning by mistake in some versions.
# This bug should be fixed in gcc version 4.8.3
execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
			OUTPUT_VARIABLE GCC_VERSION)

# Name of target Micro Controller Unit
# see the available avr-gcc mmcu options for possible values
set(CMCU "-mmcu=at90can128")

set(CSTANDARD "-std=c99")
set(CDEBUG "-g")
set(CWARN "-Wall -Wstrict-prototypes -Werror") # TODO: add -Wextra and -pedantic
set(CTUNING "-fpack-struct -fshort-enums -funsigned-bitfields -funsigned-char ${LTO} -mrelax")

set(COPT "-Os")

set(FLOAT_PRINT "-Wl,-u,vfprintf -lprintf_flt") # TODO: This should be an optional flag

add_definitions(-DF_CPU=${F_CPU} -DCAN_BAUDRATE=${CAN_BAUDRATE})
set(CFLAGS "${CMCU} ${CINCS} ${CSTANDARD} ${CDEBUG} ${CWARN} ${CTUNING} ${COPT} ${CEXTRA} ${FLOAT_PRINT}")

# Without the "CACHE STRING "" FORCE" the CMAKE_C_FLAGS variable is empty when
# entering subdirectories (where we want to do the build). The only other work
# around was to do `make cache_rebuild` right after running `cmake ..` but we
# don't want to put the work on the user to do this.
set(CMAKE_C_FLAGS ${CFLAGS} CACHE STRING "" FORCE)
