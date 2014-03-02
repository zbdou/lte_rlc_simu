
include ./config.mk

C_FILES = $(wildcard *.c)
C_OBJS = $(notdir $(C_FILES:.c=.o))
CFLAGS += -I./
C_OBJS_LIB = bitcpy.o fastalloc.o list.o log.o ptimer.o rlc_am.o rlc_common.o rlc_tm.o rlc_um.o
C_OBJS_EXAMPLE = example.o
C_OBJS_SIMU_UM = simu_um_mode.o
C_OBJS_DECODER = rlc_decoder.o

TARGET = librlc.a rlc_example rlc_decoder simu_um

all: $(TARGET)
.PHONY: all

clean :
	$(RM) *.o
	$(RM) *~
	$(RM) *.bak
	$(RM) *.exe
	$(RM) $(TARGET) .depend


dep : .depend

include .depend

.depend: $(C_FILES)
	$(CC) $(CFLAGS) -MM $^ > $@

librlc.a: $(C_OBJS_LIB)
	$(AR) $(AFLAGS) $@ $^

rlc_example: $(C_OBJS_EXAMPLE) librlc.a
	$(CC) -o $@ $< -L$(LIBDIR) -lrlc

simu_um: $(C_OBJS_SIMU_UM) librlc.a
	$(CC) -o $@ $< -L$(LIBDIR) -lrlc


rlc_decoder: $(C_OBJS_DECODER) librlc.a
	$(CC) -o $@ $(C_OBJS_DECODER) -L$(LIBDIR) -lrt -lrlc


