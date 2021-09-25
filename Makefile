TARGET_EXEC := MCP
VERSION := 0.0.1

CFLAGS := 	-march=native -mcpu=native -Ofast \
		-flto=4 -fno-fat-lto-objects -fuse-linker-plugin \
		-floop-interchange -ftree-loop-distribution -floop-strip-mine -floop-block \
		-fgraphite-identity -floop-nest-optimize -floop-parallelize-all -ftree-parallelize-loops=4 -ftree-vectorize \
		-fipa-pta -fno-semantic-interposition -fno-common -Wall -pipe \
		-DMG_ENABLE_IPV6=1 -DMG_ENABLE_LOG=0 -DMG_ENABLE_SSI=0 -DMCP_VERSION=\"$(VERSION)\" \
		-std=gnu99
# Not working with 64 bit:
#		-mtls-dialect=gnu2
LDFLAGS := -lpthread -Wl,-O1 -Wl,--as-needed -Wl,--hash-style=gnu

#CFLAGS := -march=native -O0 -pipe
#LDFLAGS := -lpthread -Wl,-O1 -Wl,--as-needed

BUILD_DIR := ./build
SRC_DIRS := ./src
INCLUDE_DIRS := ./include

SRCS := $(shell find $(SRC_DIRS) -name "*.c")
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)
INC_DIRS := $(shell find $(INCLUDE_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

./$(TARGET_EXEC): $(OBJS)
	gcc $(CFLAGS) $(INC_FLAGS) $(OBJS) -o $@ $(LDFLAGS)
	strip -s $@

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	gcc $(CFLAGS) $(INC_FLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf MCP $(BUILD_DIR)
