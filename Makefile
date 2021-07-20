TARGET_EXEC := MCP

CFLAGS := -march=native -O2 -ffast-math -flto=4 -fno-fat-lto-objects -pipe -fuse-linker-plugin
LDFLAGS := -lpthread -Wl,-O1 -Wl,--as-needed -Wl,--hash-style=gnu

#CFLAGS := -march=native -O0 -pipe
#LDFLAGS := -lpthread -Wl,-O1 -Wl,--as-needed

BUILD_DIR := ./build
SRC_DIRS := .

SRCS := $(shell find $(SRC_DIRS) -name "*.c")
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)
INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

./$(TARGET_EXEC): $(OBJS)
	gcc $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)
	strip -s $@

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	gcc $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/*.d 
