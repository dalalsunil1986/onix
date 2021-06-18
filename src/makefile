BUILD:=../build
SRC:=.

BOCHS=bochs
BOCHSGDB=bochs-gdb
ASM=nasm
LD=ld
CC=g++

BOOT_INCLUDE:=$(SRC)/boot

# ----------------------------

# DEBUG:=-g
DEBUG:=
ENTRYPOINT = 0x21000
ASM_FLAGS= -f elf $(DEBUG)
LDFLAGS = -m elf_i386 -static -Ttext $(ENTRYPOINT) -e __kernel_entry 

# 创建相关目录
define make_dirs
	@mkdir -p $1
endef

KERNEL_ASM:= $(SRC)/kernel/entry.asm


KERNEL_OBJECTS= $(KERNEL_ASM:$(SRC)/%.asm=$(BUILD)/%.o)

# for boot.bin / loader.bin
$(BUILD)/%.bin: $(SRC)/boot/%.asm
	$(call make_dirs,$(dir $@))
	$(ASM) -g -I $(BOOT_INCLUDE) $< -o $@

$(BUILD)/%.o: $(SRC)/%.asm
	$(call make_dirs,$(dir $@))
	$(ASM) $(ASM_FLAGS) -I $(BOOT_INCLUDE) $< -o $@

$(BUILD)/kernel.bin: $(KERNEL_OBJECTS)
	$(call make_dirs,$(dir $@))
	$(LD) $(LDFLAGS) $^ -o $@

# ------------------------------------------------

$(BUILD)/master.img: $(BUILD)/boot.bin $(BUILD)/loader.bin $(BUILD)/kernel.bin
ifeq ("$(wildcard $(BUILD)/master.img)", "")
	bximage -q -hd=16 -mode=create -sectsize=512 -imgmode=flat $@
endif
# 将 boot.bin 写入主引导扇区
	dd if=$(BUILD)/boot.bin of=$@ bs=512 count=1 conv=notrunc
# 将 loader.bin 写入磁盘
	dd if=$(BUILD)/loader.bin of=$@ bs=512 count=4 seek=2 conv=notrunc
# 将 kernel.bin 写入磁盘
	dd if=$(BUILD)/kernel.bin of=$@ bs=512 seek=100 count=200 conv=notrunc

$(BUILD)/slave.img:
ifeq ("$(wildcard $(BUILD)/slave.img)", "")
	bximage -q -hd=128 -mode=create -sectsize=512 -imgmode=flat $@
endif

image: $(BUILD)/master.img $(BUILD)/slave.img
	-

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
	rm -rf $(BUILD)/kernel
