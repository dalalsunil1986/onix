BUILD:=../build
SRC:=.

BOCHS=bochs
BOCHSGDB=bochs-gdb
ASM=nasm
LD=ld
CC=g++

BOOT_INCLUDE:=$(SRC)/boot

# 创建相关目录
define make_dirs
	@echo "make dir $1"
	@mkdir -p $1
endef


# for boot.bin / loader.bin
$(BUILD)/%.bin: $(SRC)/boot/%.asm
	$(call make_dirs,$(dir $@))
	$(ASM) -g -I $(BOOT_INCLUDE) $< -o $@

$(BUILD)/master.img: $(BUILD)/boot.bin $(BUILD)/loader.bin
ifeq ("$(wildcard $(BUILD)/master.img)", "")
	bximage -q -hd=16 -mode=create -sectsize=512 -imgmode=flat $@
endif
# 将 boot.bin 写入主引导扇区
	dd if=$(BUILD)/boot.bin of=$@ bs=512 count=1 conv=notrunc
# 将 loader.bin 写入硬盘
	dd if=$(BUILD)/loader.bin of=$@ bs=512 count=1 seek=2 conv=notrunc

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
