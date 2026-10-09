ROOT := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/..)
AICI_ROOT ?= $(ROOT)/.aici-host
ICK ?=
.PHONY: verify
verify:
	test -x "$(ICK)"
	mkdir -p "$(ROOT)/build/contract"
	"$(ICK)" -fno-link-libatomic -std=c11 -O2 -Wall -Wextra -Werror "$(AICI_ROOT)/src/aici.c" -o "$(ROOT)/build/contract/aici"
	"$(ROOT)/build/contract/aici" verify "$(AICI_ROOT)/contracts/build-toolchain-v0.contract.tsv" "$(ROOT)"
	cat "$(ROOT)/ci/build-toolchain.tsv"
