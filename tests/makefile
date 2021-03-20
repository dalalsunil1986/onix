BUILD:=./build
TEST:=.
KERNEL:=../src/kernel
KERNEL_INCLUDE:=../src/kernel/include
INCLUDE:=../src/include

CC=gcc
LD=ld

CFLAG = -m32 -Wall $(DEBUG_FLAGS) -I $(INCLUDE) -I $(KERNEL_INCLUDE)  -g 

LDFLAGS = -m elf_i386 -s

$(BUILD)/test.o: $(TEST)/test.c
	$(CC) $(CFLAG) -c $< -o $@

$(BUILD)/vsprintf.o: $(KERNEL)/bitmap.c
	$(CC) $(CFLAG) -c $< -o $@

$(BUILD)/bitmap.o: $(KERNEL)/vsprintf.c
	$(CC) $(CFLAG) -c $< -o $@

$(BUILD)/test: $(BUILD)/test.o \
	$(BUILD)/bitmap.o \
	$(BUILD)/vsprintf.o
	$(CC) $(CFLAG) $^ -o $@

test: $(BUILD)/test
	@-

run: $(BUILD)/test
	$(BUILD)/test

clean:
	rm -rf $(BUILD)/*.o
	rm -rf $(BUILD)/test
