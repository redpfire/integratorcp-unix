
CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy
QEMU = qemu-system-arm
GDB = arm-none-eabi-gdb

all: clean obj link

init:
	mkdir -p build

obj:
	$(CC) -mcpu=arm1176jzf-s -ffreestanding -c boot/*.S
	$(CC) -Ikernel/inc -mcpu=arm1176jzf-s -Wall -O2 -nostdlib -nostartfiles -ffreestanding -std=gnu99 -c kernel/*.c

debug_c:
	$(CC) -g -mcpu=arm1176jzf-s -ffreestanding -c boot/*.S
	$(CC) -Ikernel/inc -g -mcpu=arm1176jzf-s -Wall -O0 -nostdlib -nostartfiles -ffreestanding -std=gnu99 -c kernel/*.c

link:
	$(LD) -T link.ld -o build/arm.elf *.o
	$(OBJCOPY) -O binary build/arm.elf build/arm.bin

debug: clean debug_c link

run:
	$(QEMU) -display none -m 256 -machine integratorcp -kernel build/arm.bin -serial stdio

run-debug:
	$(QEMU) -display none -m 256 -machine integratorcp -kernel build/arm.bin -serial stdio -s

gdb:
	$(GDB) -ex 'file ./build/arm.elf' -ex 'target remote localhost:1234' -ex 'b k_exphandler'

clean:
	rm -f build/*
	rm -f *.o
