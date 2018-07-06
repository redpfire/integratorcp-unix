
CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy
QEMU = qemu-system-arm
GDB = arm-none-eabi-gdb
RADARE2 = r2

OBJECTS = kernel/*.o

all: clean obj link

init:
	mkdir -p build

obj:
	make -C kernel

debug_c:
	make -C kernel debug

link:
	$(CC) -nostartfiles -ffreestanding -Xlinker --script=./link.ld -lgcc -o build/arm.elf $(OBJECTS)
	$(OBJCOPY) -O binary build/arm.elf build/arm.bin

debug: clean debug_c link

run:
	$(QEMU) -display none -m 256 -machine integratorcp -kernel build/arm.bin -serial stdio

run-debug:
	$(QEMU) -display none -m 256 -machine integratorcp -kernel build/arm.bin -serial stdio -s

gdb:
	$(GDB) -ex 'file ./build/arm.elf' -ex 'target remote localhost:1234' -ex 'layout asm'

r2:
	$(RADARE2) -a arm -b 32 -D gdb -d gdb://127.0.0.1:1234

clean:
	rm -f build/*
	rm -f $(OBJECTS)
