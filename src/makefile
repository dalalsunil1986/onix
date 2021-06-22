BUILD:=../build
SRC:=.

BOCHS=bochs
BOCHSGDB=bochs-gdb
ASM=nasm
LD=ld
CC=gcc

BOOT_INCLUDE:=$(SRC)/boot
INCLUDE:=$(SRC)/include
# ----------------------------


DEBUG:=
DEBUG:=-g

ENTRYPOINT = 0xC0011000
LDS:= $(BUILD)/kernel.lds
ASM_FLAGS= -f elf $(DEBUG)
LDFLAGS = -m elf_i386 -static -Ttext $(ENTRYPOINT) -T $(LDS)
CFLAGS= -m32 $(DEBUG) -fno-builtin -fno-stack-protector -fno-pie -fno-pic\
			-nostdlib -nostdinc -nostartfiles -nodefaultlibs -w \

# 创建相关目录
define make_dirs
	@echo "make dir $1"
	@mkdir -p $1
endef

KERNEL_ASM:= 	$(SRC)/kernel/start.asm \
				$(SRC)/kernel/handler.asm \

KERNEL_C:= 	$(SRC)/kernel/main.c \
			$(SRC)/kernel/io.c \
			$(SRC)/kernel/console.c \
			$(SRC)/kernel/printk.c \
			$(SRC)/kernel/debug.c \
			$(SRC)/kernel/global.c \
			$(SRC)/kernel/memory.c \
			$(SRC)/kernel/interrupt.c \
			$(SRC)/kernel/clock.c \
			

KERNEL_LIB:= $(SRC)/lib/string.c \
			$(SRC)/lib/assert.c \
			$(SRC)/lib/vsprintf.c \
			$(SRC)/lib/bitmap.c \

KERNEL_OBJECTS= $(KERNEL_ASM:$(SRC)/%.asm=$(BUILD)/%.o) \
				$(KERNEL_C:$(SRC)/%.c=$(BUILD)/%.o) \
				$(KERNEL_LIB:$(SRC)/%.c=$(BUILD)/%.o) \

# for boot.bin / loader.bin
$(BUILD)/%.bin: $(SRC)/boot/%.asm
	$(call make_dirs,$(dir $@))
	$(ASM) -g -I $(BOOT_INCLUDE) $< -o $@

$(BUILD)/%.o: $(SRC)/%.asm
	$(call make_dirs,$(dir $@))
	$(ASM) $(ASM_FLAGS) -I $(BOOT_INCLUDE) $< -o $@

$(BUILD)/%.o: $(SRC)/%.c
	$(call make_dirs,$(dir $@))
	$(CC) $(CFLAGS) -I $(INCLUDE) -c $< -o $@

$(BUILD)/kernel.bin: $(KERNEL_OBJECTS)
	$(call make_dirs,$(dir $@))
	$(LD) $(LDFLAGS) $^ -o $@

$(BUILD)/master.img: $(BUILD)/boot.bin $(BUILD)/loader.bin $(BUILD)/kernel.bin
ifeq ("$(wildcard $(BUILD)/master.img)", "")
	bximage -q -hd=16 -mode=create -sectsize=512 -imgmode=flat $@
endif
# 将 boot.bin 写入主引导扇区
	dd if=$(BUILD)/boot.bin of=$@ bs=512 count=1 conv=notrunc
# 将 loader.bin 写入硬盘
	dd if=$(BUILD)/loader.bin of=$@ bs=512 count=4 seek=2 conv=notrunc
# 将 kernel.bin 写入磁盘
	dd if=$(BUILD)/kernel.bin of=$@ bs=512 seek=100 count=200 conv=notrunc

$(BUILD)/slave.img:
ifeq ("$(wildcard $(BUILD)/slave.img)", "")
	bximage -q -hd=128 -mode=create -sectsize=512 -imgmode=flat $@
endif


$(BUILD)/master.vmdk: $(BUILD)/master.img
	qemu-img convert -pO vmdk $< $@

.PHONY:vmdk
vmdk: $(BUILD)/master.vmdk
	-

.PHONY:image
image: $(BUILD)/master.img $(BUILD)/slave.img
	-

.PHONY:test
test: $(BUILD)/kernel.bin
	strip $<
	readelf -a $<

.PHONY:bochs
bochs: image
	rm -rf $(BUILD)/bx_enh_dbg.ini
	cd $(BUILD) && $(BOCHS) -q -f bochsrc -unlock

.PHONY:bochs
gdb: image
	cd $(BUILD) && $(BOCHSGDB) -q -f bochsrc.gdb -unlock

.PHONY:clean
clean:
	rm -rf $(BUILD)/bx_enh_dbg.ini
	rm -rf $(BUILD)/*.img
	rm -rf $(BUILD)/*.lock
	rm -rf $(BUILD)/*.bin
	rm -rf $(BUILD)/*.vmdk
	rm -rf $(BUILD)/kernel
	rm -rf $(BUILD)/lib
