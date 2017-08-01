CC      = arm-linux-gcc
LD      = arm-linux-ld
AR      = arm-linux-ar
OBJCOPY = arm-linux-objcopy
OBJDUMP = arm-linux-objdump
objs := start.o init.o boot.o

uboot.bin:$(objs)
	${LD} -Tboot.lds -o boot.elf $^
	${OBJCOPY} -O binary -S uboot.elf $@	
	${OBJDUMP} -D -m arm boot.elf > boot.dis	
%.o:%.c
	${CC} $(CPPFLAGS) $(CFLAGS) -c -o $@ $<
%.o:%.S
	${CC} $(CPPFLAGS) $(CFLAGS) -c -o $@ $<