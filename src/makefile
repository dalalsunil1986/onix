BUILD:= ../build
SRC:=.
BOOT:= $(SRC)/boot

BOOT_INCLUDE:=$(SRC)/boot

ASM=nasm

.PHONY: clean

$(BUILD)/boot.bin: $(BOOT)/boot.asm
	$(ASM) -I $(BOOT_INCLUDE) $< -o $@

$(BUILD)/loader.bin: $(BOOT)/loader.asm
	$(ASM) -I $(BOOT_INCLUDE) $< -o $@

$(BUILD)/harddisk.img: $(BUILD)/boot.bin \
	$(BUILD)/loader.bin

ifeq ("$(wildcard $(BUILD)/harddisk.img)", "")
	bximage -q -hd=16 -mode=create -sectsize=512 -imgmode=flat $@
endif
	dd if=$(BUILD)/boot.bin of=$@ bs=512 count=1 conv=notrunc
	dd if=$(BUILD)/loader.bin of=$@ bs=512 count=20 seek=2 conv=notrunc

bochs: $(BUILD)/harddisk.img
	@rm -rf $(BUILD)/harddisk.img.lock
	cd $(BUILD) && bochs -q  -f bochsrc

clean:
	rm -rf $(BUILD)/boot.bin
	rm -rf $(BUILD)/loader.bin
	rm -rf $(BUILD)/harddisk.img*

