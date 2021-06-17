BUILD:=../build
SRC:=.

BOCHS=bochs
BOCHSGDB=bochs-gdb

$(BUILD)/master.img:
ifeq ("$(wildcard $(BUILD)/master.img)", "")
	bximage -q -hd=16 -mode=create -sectsize=512 -imgmode=flat $@
endif

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
