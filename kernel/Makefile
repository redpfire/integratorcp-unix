
CC = arm-none-eabi-gcc
CFLAGS = -I../include -Wall -O2 -nostdlib -nostartfiles -ffreestanding -std=gnu99

all:
	$(CC) -ffreestanding -c *.S
	$(CC) $(CFLAGS) -c *.c

debug:
	$(CC) -g -ffreestanding -c *.S
	$(CC) -g $(CFLAGS) -c *.c
