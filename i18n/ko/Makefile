BOOK_DIR := $(abspath ../..)
TOOLCHAIN_REF := $(shell sed -n 's/.*"bgbspd_ko"[[:space:]]*:[[:space:]]*"\([0-9a-f]*\)".*/\1/p' toolchain.lock)
TOOLCHAIN_DIR := $(CURDIR)/.toolchain/$(TOOLCHAIN_REF)

.PHONY: all build stage docker docker-build html pdf status clean pristine test toolchain
all build stage docker docker-build html pdf status clean pristine test: toolchain
	$(MAKE) -f "$(TOOLCHAIN_DIR)/book.make" BOOK_DIR="$(BOOK_DIR)" $@
toolchain:
	sh ./fetch-toolchain.sh "$(TOOLCHAIN_REF)" "$(TOOLCHAIN_DIR)"
