SHELL := /bin/bash

BUILD_DIR ?= build
BOARD     ?= thejas32
JOBS      ?= $(shell nproc 2>/dev/null || echo 4)

.PHONY: all firmware thejas32 test msg format tree clean help

all: firmware

help:
	@echo "Freertrtos Autopilot"
	@echo ""
	@echo "  make thejas32   Build firmware for the reference THEJAS32 board"
	@echo "  make test       Build and run host unit tests (no hardware needed)"
	@echo "  make msg        Regenerate C structs from src/msg/*.msg"
	@echo "  make format     Run clang-format over C/C++ sources"
	@echo "  make tree       Regenerate docs/file-tree.md"
	@echo "  make clean      Remove the build directory"

firmware: thejas32

thejas32:
	cmake -S . -B $(BUILD_DIR)/thejas32 \
		-DCMAKE_TOOLCHAIN_FILE=$(CURDIR)/cmake/toolchain-thejas32.cmake \
		-DFREERTRTOS_BOARD=thejas32 \
		-DFREERTRTOS_BUILD_FIRMWARE=ON \
		-DFREERTRTOS_BUILD_TESTS=OFF
	cmake --build $(BUILD_DIR)/thejas32 --parallel $(JOBS)

test:
	cmake -S . -B $(BUILD_DIR)/host \
		-DFREERTRTOS_BUILD_FIRMWARE=OFF \
		-DFREERTRTOS_BUILD_TESTS=ON
	cmake --build $(BUILD_DIR)/host --parallel $(JOBS)
	ctest --test-dir $(BUILD_DIR)/host --output-on-failure

msg:
	python3 tools/msg_gen.py --input src/msg --output src/msg/generated

format:
	@command -v clang-format >/dev/null || { echo "clang-format not found"; exit 1; }
	find src boards os tests -type f \( -name '*.c' -o -name '*.h' \) -print0 \
		| xargs -0 clang-format -i
	@echo "formatted C/C++ sources"

tree:
	python3 tools/gen_tree.py > docs/file-tree.md
	@echo "wrote docs/file-tree.md"

clean:
	rm -rf $(BUILD_DIR)
