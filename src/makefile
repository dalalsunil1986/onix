BUILD:= ../build
SRC:=.
BOOT:= $(SRC)/boot

BOOT_INCLUDE:=$(SRC)/boot

ASM=nasm

.PHONY: clean

$(BUILD)/boot.bin: $(BOOT)/boot.asm
	$(ASM) -I $(BOOT_INCLUDE) $< -o $@

$(BUILD)/harddisk.img: $(BUILD)/boot.bin

ifeq ("$(wildcard $(BUILD)/harddisk.img)", "")
	bximage -q -hd=16 -mode=create -sectsize=512 -imgmode=flat $@
endif
	dd if=$(BUILD)/boot.bin of=$@ bs=512 count=1 conv=notrunc

bochs: $(BUILD)/harddisk.img
	@rm -rf $(BUILD)/harddisk.img.lock
	cd $(BUILD) && bochs -q  -f bochsrc

clean:
	rm -rf $(BUILD)/boot.bin
	rm -rf $(BUILD)/harddisk.img*

