BUILD:= ../../build
SRC_DIR:=.

REAL_DIR:=$(BUILD)/real
DEBUG_DIR:=$(BUILD)/debug

ONIX_INCLUDE:=$(SRC_DIR)/../include
INCLUDE:= -I $(ONIX_INCLUDE) 

CC=gcc

REAL_CFLAGS = $(INCLUDE) -fno-builtin -m32 -fno-stack-protector \
		-nostdlib -nostdinc -nostartfiles -nodefaultlibs -w \
		-D ONIX_DEBUG \

DEBUG_CFLAGS = $(INCLUDE) -m32 -w -D ONIX_KERNEL_DEBUG -D ONIX_DEBUG -g 

SRC:=$(SRC_DIR)/osh.c 

$(REAL_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(REAL_CFLAGS) -c $< -o $@

$(DEBUG_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(DEBUG_CFLAGS) -c $< -o $@

REAL_TARGET:=$(REAL_DIR)/osh

DEBUG_TARGET:=$(DEBUG_DIR)/osh

$(REAL_TARGET): $(SRC:$(SRC_DIR)/%.c=$(DEBUG_DIR)/%.o)
	$(CC) $(REAL_CFLAGS) $^ -o $@

$(DEBUG_TARGET): $(SRC:$(SRC_DIR)/%.c=$(DEBUG_DIR)/%.o)
	$(CC) $(DEBUG_CFLAGS) $^ -o $@

debug:$(DEBUG_TARGET)
	-

clean:
	rm -rf $(SRC:$(SRC_DIR)/%.c=$(DEBUG_DIR)/%.o)
	rm -rf $(SRC:$(SRC_DIR)/%.c=$(REAL_DIR)/%.o)
	rm -rf $(REAL_TARGET)
	rm -rf $(DEBUG_TARGET)

.PHONY: clean debug