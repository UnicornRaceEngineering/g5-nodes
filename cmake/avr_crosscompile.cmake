#########################
# Cross compile for AVR #
#########################

# Set cmake to cross compiling
set(CMAKE_SYSTEM_NAME Generic)

# Set the cross compiler that we want to use.
# We only have to specify the c compiler as cmake will automaticly figure out
# what the cxx compiler and binutils are called.
set(CMAKE_C_COMPILER avr-gcc)

# Setup the programmer that is used to write to the MCU
# run `avrdude -c ?` for a list of valid ID's
# run `avrdude -p ?` for a list of valid MCU's
find_program(PROGRAMMER avrdude)
set(PROGRAMMER_ID avrispmkII)
set(PROGRAMMER_MCU c128)
set(AVRDUDE avrdude)


# Sets up a custom command and target that Generates an intel hex file from a
# compiled binary elf file.
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

# Sets up a target that calls the programmer to flash the MCU with the given
# binary. It automaticly converts the elf_file to the correct hex format
function(avr_make_flashable elf_file)
	avr_create_hex(${elf_file})

	add_custom_target(${elf_file}_writeflash
		COMMAND ${PROGRAMMER}
					-c ${PROGRAMMER_ID}
					-p ${PROGRAMMER_MCU}
					-P usb -B 8
					-U flash:w:${elf_file}.hex
		DEPENDS ${elf_file}_hex
		VERBATIM
		)

endfunction()

set(FUSE_1 lfuse:w:0xff:m)
set(FUSE_2 hfuse:w:0xf9:m)
set(FUSE_3 efuse:w:0xdf:m)

function(avr_write_fuses)
	add_custom_target(Fuses_writeflash
		COMMAND ${AVRDUDE} -c ${PROGRAMMER_ID}
			-p ${PROGRAMMER_MCU} -P usb -e
			-u -U ${FUSE_1}
			-u -U ${FUSE_2}
			-u -U ${FUSE_3}
	)
endfunction()
