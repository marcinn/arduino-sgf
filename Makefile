CLANG_FORMAT ?= clang-format

SRC_FILES := $(shell find src -type f \( -name '*.c' -o -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \))

.PHONY: format
format:
	$(CLANG_FORMAT) -i $(SRC_FILES)
