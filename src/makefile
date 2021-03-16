BUILD:= ../build
SRC:=.
BOOT:= $(SRC)/boot
KERNEL:= $(SRC)/kernel

BOOT_INCLUDE:=$(SRC)/boot
INCLUDE:=$(SRC)/include
KERNEL_INCLUDE:=$(SRC)/kernel/include

BOCHS=bochs
ASM=nasm
CC=gcc
LD=ld

ENTRYPOINT = 0xC0010000

ASM_FLAGS= -f elf

CFLAGS = -I $(INCLUDE) -c -fno-builtin -m32 -fno-stack-protector \
		-nostdlib -nostdinc -nostartfiles -nodefaultlibs -Wall -Wextra 

LDFLAGS = -m elf_i386 -s -Ttext $(ENTRYPOINT)


.PHONY: clean

$(BUILD)/boot.bin: $(BOOT)/boot.asm
	$(ASM) -I $(BOOT_INCLUDE) $< -o $@

$(BUILD)/loader.bin: $(BOOT)/loader.asm
	$(ASM) -I $(BOOT_INCLUDE) $< -o $@

$(BUILD)/start.o: $(KERNEL)/start.asm
	$(ASM) $(ASM_FLAGS) -I $(BOOT_INCLUDE) $< -o $@

$(BUILD)/system.o: $(KERNEL)/system.asm
	$(ASM) $(ASM_FLAGS) -I $(BOOT_INCLUDE) $< -o $@

$(BUILD)/main.o: $(KERNEL)/main.c
	$(CC) $(CFLAGS) -I $(KERNEL_INCLUDE) $< -o $@

$(BUILD)/printk.o: $(KERNEL)/printk.c
	$(CC) $(CFLAGS) -I $(KERNEL_INCLUDE) $< -o $@

$(BUILD)/kernel.bin: $(BUILD)/start.o \
	$(BUILD)/system.o \
	$(BUILD)/printk.o \
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
	cd $(BUILD) && $(BOCHS) -q  -f bochsrc

clean:
	rm -rf $(BUILD)/boot.bin
	rm -rf $(BUILD)/loader.bin
	rm -rf $(BUILD)/kernel.bin
	rm -rf $(BUILD)/*.o
	rm -rf $(BUILD)/harddisk.img*

