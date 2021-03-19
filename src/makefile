BUILD:= ../build
SRC:=.
BOOT:= $(SRC)/boot
KERNEL:= $(SRC)/kernel
TESTS:= $(SRC)/tests

BOOT_INCLUDE:=$(SRC)/boot
INCLUDE:=$(SRC)/include
KERNEL_INCLUDE:=$(SRC)/kernel/include

BOCHS=bochs
ASM=nasm
CC=gcc
LD=ld

ENTRYPOINT = 0xC0020000

ASM_FLAGS= -f elf -g

DEBUG_FLAGS = -D ONIX_DEBUG

CFLAGS = -I $(INCLUDE) -g  -c -fno-builtin -m32 -fno-stack-protector \
		-nostdlib -nostdinc -nostartfiles -nodefaultlibs -Wall -Wextra \
		$(DEBUG_FLAGS)

TESTS_CFLAG = -I $(INCLUDE) -m32 -Wall $(DEBUG_FLAGS)

LDFLAGS = -m elf_i386 -Ttext $(ENTRYPOINT)

.PHONY: clean

kernel: $(BUILD)/kernel.bin
	# ps -ef | grep bochsrc-gdb | grep -v grep | awk '{print $$2}' | xargs -r kill -9
	-

$(BUILD)/boot.bin: $(BOOT)/boot.asm
	$(ASM) -I $(BOOT_INCLUDE) $< -o $@

$(BUILD)/loader.bin: $(BOOT)/loader.asm
	$(ASM) -I $(BOOT_INCLUDE) $< -o $@

$(BUILD)/start.o: $(KERNEL)/start.asm
	$(ASM) $(ASM_FLAGS) -I $(BOOT_INCLUDE) $< -o $@

$(BUILD)/system.o: $(KERNEL)/system.asm
	$(ASM) $(ASM_FLAGS) -I $(BOOT_INCLUDE) $< -o $@

$(BUILD)/global.o: $(KERNEL)/global.c
	$(CC) $(CFLAGS) -I $(KERNEL_INCLUDE) $< -o $@

$(BUILD)/main.o: $(KERNEL)/main.c
	$(CC) $(CFLAGS) -I $(KERNEL_INCLUDE) $< -o $@

$(BUILD)/printk.o: $(KERNEL)/printk.c
	$(CC) $(CFLAGS) -I $(KERNEL_INCLUDE) $< -o $@

$(BUILD)/string.o: $(KERNEL)/string.c
	$(CC) $(CFLAGS) -I $(KERNEL_INCLUDE) $< -o $@

$(BUILD)/vsprintf.o: $(KERNEL)/vsprintf.c
	$(CC) $(CFLAGS) -I $(KERNEL_INCLUDE) $< -o $@

$(BUILD)/assert.o: $(KERNEL)/assert.c
	$(CC) $(CFLAGS) -I $(KERNEL_INCLUDE) $< -o $@

$(BUILD)/interrupt.o: $(KERNEL)/interrupt.c
	$(CC) $(CFLAGS) -I $(KERNEL_INCLUDE) $< -o $@

$(BUILD)/clock.o: $(KERNEL)/clock.c
	$(CC) $(CFLAGS) -I $(KERNEL_INCLUDE) $< -o $@

$(BUILD)/bitmap.o: $(KERNEL)/bitmap.c
	$(CC) $(CFLAGS) -I $(KERNEL_INCLUDE) $< -o $@

$(BUILD)/memory.o: $(KERNEL)/memory.c
	$(CC) $(CFLAGS) -I $(KERNEL_INCLUDE) $< -o $@

$(BUILD)/kernel.bin: $(BUILD)/start.o \
	$(BUILD)/global.o \
	$(BUILD)/system.o \
	$(BUILD)/string.o \
	$(BUILD)/printk.o \
	$(BUILD)/vsprintf.o \
	$(BUILD)/assert.o \
	$(BUILD)/interrupt.o \
	$(BUILD)/bitmap.o \
	$(BUILD)/memory.o \
	$(BUILD)/clock.o \
	$(BUILD)/main.o
	$(LD) $(LDFLAGS) $^ -o $@

$(BUILD)/harddisk.img: $(BUILD)/boot.bin \
	$(BUILD)/loader.bin \
	$(BUILD)/kernel.bin

ifeq ("$(wildcard $(BUILD)/harddisk.img)", "")
	bximage -q -hd=16 -mode=create -sectsize=512 -imgmode=flat $@
endif
	dd if=$(BUILD)/boot.bin of=$@ bs=512 count=1 conv=notrunc
	dd if=$(BUILD)/loader.bin of=$@ bs=512 count=20 seek=2 conv=notrunc
	dd if=$(BUILD)/kernel.bin of=$@ bs=512 seek=100 count=200 conv=notrunc

bochs: $(BUILD)/harddisk.img
	@rm -rf $(BUILD)/harddisk.img.lock
	@rm -rf $(BUILD)/bx_enh_dbg.ini
	cd $(BUILD) && $(BOCHS) -q  -f bochsrc

image: $(BUILD)/harddisk.img
	-

win:
	cd $(BUILD) && bochs -q -f bochsrc.bxrc

clean:
	rm -rf $(BUILD)/boot.bin
	rm -rf $(BUILD)/loader.bin
	rm -rf $(BUILD)/kernel.bin
	rm -rf $(BUILD)/*.o
	rm -rf $(BUILD)/harddisk.img*
	rm -rf $(BUILD)/test

