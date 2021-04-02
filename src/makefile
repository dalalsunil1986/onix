BUILD:= ../build
SRC_DIR:=.
BOOT_DIR:= $(SRC_DIR)/boot
KERNEL_SRC:= $(SRC_DIR)/kernel
LIB_SRC:= $(SRC_DIR)/lib
FS_SRC:= $(SRC_DIR)/fs
FS_ONIX_SRC:= $(SRC_DIR)/fs/onix
DEBUG_SRC:= $(SRC_DIR)/debug

REAL_DIR:=$(BUILD)/real
DEBUG_DIR:=$(BUILD)/debug

BOOT_INCLUDE:=$(SRC_DIR)/boot
ONIX_INCLUDE:=$(SRC_DIR)/include
KERNEL_INCLUDE:=$(SRC_DIR)/kernel/include
FS_INCLUDE:=$(SRC_DIR)/fs
INCLUDE:= -I $(ONIX_INCLUDE) -I $(KERNEL_INCLUDE) -I $(FS_INCLUDE)

BOCHS=bochs
ASM=nasm
CC=gcc
LD=ld

ENTRYPOINT = 0xC0020000

ASM_FLAGS= -f elf

REAL_CFLAGS = $(INCLUDE) -fno-builtin -m32 -fno-stack-protector \
		-nostdlib -nostdinc -nostartfiles -nodefaultlibs -Wall -Wextra \
		-D ONIX_DEBUG

DEBUG_CFLAGS = $(INCLUDE) -m32 -Wall -D ONIX_KERNEL_DEBUG -D ONIX_DEBUG -g 

LDFLAGS = -m elf_i386 -s -Ttext $(ENTRYPOINT)

.PHONY: clean image unlock win fdisk

all: dirs $(BOCHS_TARGET)

dirs:
	@mkdir -p $(REAL_DIR) $(DEBUG_DIR) 

KERNEL_ASM:= $(KERNEL_SRC)/start.asm \
	$(KERNEL_SRC)/system.asm \
	$(KERNEL_SRC)/handler.asm \
	$(KERNEL_SRC)/schedule.asm

KERNEL_C:= $(KERNEL_SRC)/arena.c \
	$(KERNEL_SRC)/assert.c \
	$(KERNEL_SRC)/bitmap.c \
	$(KERNEL_SRC)/clock.c \
	$(KERNEL_SRC)/debug.c \
	$(KERNEL_SRC)/global.c \
	$(KERNEL_SRC)/harddisk.c \
	$(KERNEL_SRC)/interrupt.c \
	$(KERNEL_SRC)/ioqueue.c \
	$(KERNEL_SRC)/keyboard.c \
	$(KERNEL_SRC)/ksyscall.c \
	$(KERNEL_SRC)/ktask.c \
	$(KERNEL_SRC)/main.c \
	$(KERNEL_SRC)/memory.c \
	$(KERNEL_SRC)/mutex.c \
	$(KERNEL_SRC)/pid.c \
	$(KERNEL_SRC)/printk.c \
	$(KERNEL_SRC)/process.c \
	$(KERNEL_SRC)/queue.c \
	$(KERNEL_SRC)/stdlib.c \
	$(KERNEL_SRC)/string.c \
	$(KERNEL_SRC)/task.c \
	$(KERNEL_SRC)/test.c \

LIB_ASM:=$(LIB_SRC)/__syscall.asm

LIB_C:= $(LIB_SRC)/malloc.c \
	$(LIB_SRC)/stdio.c \
	$(LIB_SRC)/syscall.c \
	$(LIB_SRC)/vsprintf.c

FS_ONIX_C:=$(FS_ONIX_SRC)/inode.c \
		$(FS_ONIX_SRC)/fsbitmap.c \
		$(FS_ONIX_SRC)/fs.c

FS_C:=$(FS_SRC)/file.c


DEBUG_C:= $(DEBUG_SRC)/debug_lib.c \
	$(DEBUG_SRC)/debug_main.c

# for boot.bin / loader.bin
$(BUILD)/%.bin: $(BOOT_DIR)/%.asm
	$(ASM) -I $(BOOT_INCLUDE) $< -o $@

$(REAL_DIR)/%.o: $(KERNEL_SRC)/%.asm
	$(ASM) $(ASM_FLAGS) -I $(BOOT_INCLUDE) $< -o $@

$(REAL_DIR)/%.o: $(LIB_SRC)/%.asm
	$(ASM) $(ASM_FLAGS) -I $(BOOT_INCLUDE) $< -o $@

$(REAL_DIR)/%.o: $(KERNEL_SRC)/%.c
	$(CC) $(REAL_CFLAGS) -c $< -o $@

$(REAL_DIR)/%.o: $(LIB_SRC)/%.c
	$(CC) $(REAL_CFLAGS) -c $< -o $@

$(REAL_DIR)/%.o: $(FS_SRC)/%.c
	$(CC) $(REAL_CFLAGS) -c $< -o $@

$(REAL_DIR)/%.o: $(FS_ONIX_SRC)/%.c
	$(CC) $(REAL_CFLAGS) -c $< -o $@

$(DEBUG_DIR)/%.o: $(KERNEL_SRC)/%.c
	$(CC) $(DEBUG_CFLAGS) -c $< -o $@

$(DEBUG_DIR)/%.o: $(LIB_SRC)/%.c
	$(CC) $(DEBUG_CFLAGS) -c $< -o $@

$(DEBUG_DIR)/%.o: $(FS_SRC)/%.c
	$(CC) $(DEBUG_CFLAGS) -c $< -o $@

$(DEBUG_DIR)/%.o: $(FS_ONIX_SRC)/%.c
	$(CC) $(REAL_CFLAGS) -c $< -o $@

$(DEBUG_DIR)/%.o: $(DEBUG_SRC)/%.c
	$(CC) $(DEBUG_CFLAGS) -c $< -o $@

REAL_TARGET:=$(BUILD)/kernel.bin

REAL_OBJECTS= $(KERNEL_ASM:$(KERNEL_SRC)/%.asm=$(REAL_DIR)/%.o) \
			$(LIB_ASM:$(LIB_SRC)/%.asm=$(REAL_DIR)/%.o) \
			$(KERNEL_C:$(KERNEL_SRC)/%.c=$(REAL_DIR)/%.o) \
			$(LIB_C:$(LIB_SRC)/%.c=$(REAL_DIR)/%.o) \
			$(FS_C:$(FS_SRC)/%.c=$(REAL_DIR)/%.o) \
			$(FS_ONIX_C:$(FS_ONIX_SRC)/%.c=$(REAL_DIR)/%.o)

DEBUG_OBJECTS=$(KERNEL_C:$(KERNEL_SRC)/%.c=$(DEBUG_DIR)/%.o) \
			$(LIB_C:$(LIB_SRC)/%.c=$(DEBUG_DIR)/%.o) \
			$(FS_C:$(FS_SRC)/%.c=$(DEBUG_DIR)/%.o) \
			$(FS_C:$(FS_SRC)/%.c=$(REAL_DIR)/%.o) \
			$(FS_ONIX_C:$(FS_ONIX_SRC)/%.c=$(REAL_DIR)/%.o) \
			$(DEBUG_C:$(DEBUG_SRC)/%.c=$(DEBUG_DIR)/%.o)

DEBUG_TARGET:=$(BUILD)/debug.out

$(REAL_TARGET): $(REAL_OBJECTS)
	$(LD) $(LDFLAGS) $^ -o $@

$(DEBUG_TARGET): $(DEBUG_OBJECTS)
	$(CC) $(DEBUG_CFLAGS) $^ -o $@
	@#echo $(DEBUG_OBJECTS)

$(BUILD)/harddisk.img: $(BUILD)/boot.bin \
	$(BUILD)/loader.bin \
	$(BUILD)/kernel.bin

ifeq ("$(wildcard $(BUILD)/harddisk.img)", "")
	bximage -q -hd=16 -mode=create -sectsize=512 -imgmode=flat $@
endif
	dd if=$(BUILD)/boot.bin of=$@ bs=512 count=1 conv=notrunc
	dd if=$(BUILD)/loader.bin of=$@ bs=512 count=4 seek=2 conv=notrunc
	dd if=$(BUILD)/kernel.bin of=$@ bs=512 seek=100 count=200 conv=notrunc

$(BUILD)/slave.img: $(BUILD)/slave.sfdisk

ifeq ("$(wildcard $(BUILD)/slave.img)", "")
	bximage -q -hd=128 -mode=create -sectsize=512 -imgmode=flat $@
endif
	sfdisk $@ < $<

bochs: $(BUILD)/harddisk.img $(BUILD)/slave.img
	@rm -rf $(BUILD)/harddisk.img.lock
	@rm -rf $(BUILD)/bx_enh_dbg.ini
	cd $(BUILD) && $(BOCHS) -q  -f bochsrc

image: $(BUILD)/harddisk.img
	-

unlock:
	rm -rf $(BUILD)/*.img.lock

win:
	ssh arch "cd ~/source/github/onix/src && make image" && make unlock && \
	cd $(BUILD) && bochs -q -f bochsrc.bxrc 

debug: $(BUILD)/debug.out $(BUILD)/slave.img 
	-

clean:
	rm -rf $(BUILD)/boot.bin
	rm -rf $(BUILD)/loader.bin
	rm -rf $(BUILD)/kernel.bin
	rm -rf $(REAL_DIR)/*.o
	rm -rf $(DEBUG_DIR)/*.o
	rm -rf $(BUILD)/*.img*
	rm -rf $(BUILD)/debug.out
