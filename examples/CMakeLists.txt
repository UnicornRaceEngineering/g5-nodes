cmake_minimum_required(VERSION 2.6.0)

include(${PROJECT_SOURCE_DIR}/cmake/avr_crosscompile.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/flags.cmake)

add_subdirectory(buffered_usart_echo)
add_subdirectory(can_simple_rx_tx)
add_subdirectory(io_read_write)
