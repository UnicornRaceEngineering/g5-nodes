PROJECTNAME=io_read_write
SRC=$(wildcard *.c)

TOP=../..
include $(TOP)/mk/config.mk
include $(TOP)/mk/flags.mk
include $(TOP)/mk/executables.mk

DEFINES += -DNO_USART0_SUPPORT #-DNO_USART1_BUFFERED_INPUT -DNO_USART1_BUFFERED_OUTPUT

include $(TOP)/mk/rules.mk

