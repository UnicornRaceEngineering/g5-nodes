#########################
# Cross compile for AVR #
#########################

# Set cmake to cross compiling
set(CMAKE_SYSTEM_NAME Generic)

# Set the cross compiler that we want to use.
# We only have to specify the c compiler as cmake will automaticly figure out
# what the cxx compiler and binutils are called.
set(CMAKE_C_COMPILER avr-gcc)

set(PROGRAMMER avrispmkII)
set(PROGRAMMER_MCU c128)

# Make sure we have a working copy of the correct objcopy
if(NOT CMAKE_OBJCOPY)
	find_program(AVROBJCOPY_PROGRAM avr-objcopy)
    set(CMAKE_OBJCOPY ${AVROBJCOPY_PROGRAM})
endif(NOT CMAKE_OBJCOPY)

find_program(AVRDUDE avrdude)

# Generates an intel hex file from a compile binary elf file
function(avr_create_hex elf_file)
	add_custom_command(
		OUTPUT ${elf_file}.hex
		COMMAND ${CMAKE_OBJCOPY}
		ARGS -O ihex -R .eeprom ${elf_file} ${elf_file}.hex
		DEPENDS ${elf_file}
		VERBATIM
		)

	add_custom_target(${elf_file}_hex
		ALL
		DEPENDS ${elf_file}.hex
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		)
endfunction()

function(avr_make_flashable elf_file)
	avr_create_hex(${elf_file})

	add_custom_target(${elf_file}_writeflash
		COMMAND ${AVRDUDE} -c ${PROGRAMMER} -p ${PROGRAMMER_MCU} -P usb -B 8 -U flash:w:${elf_file}.hex
		DEPENDS ${elf_file}_hex
		VERBATIM
		)

endfunction()
