# Self-Documented Makefile
# http://marmelab.com/blog/2016/02/29/auto-documented-makefile.html
.PHONY: help
help: ## Show this help message.
	@grep -E '^[a-zA-Z0-9_\.-\%]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'

clang_cxx ?= /opt/llvm-8.0.0/bin/clang++
filecheck=$(shell which filecheck)
LIT=$(shell which lit)
test: ## Run integration tests
	CURRENT_DIR=$(PWD) \
		clang_cxx=$(clang_cxx) \
		filecheck=$(filecheck) \
		PATH=$(PWD)/tools:$$PATH \
		$(LIT) \
		--threads 1 \
		-vv $(PWD)/tests # --show-all

# Cleaning

CLEAN_FIND_CMD=find \
	. \
	-type f \( \
		-name '*.html' -or \
		-name '*.json' -or \
		-name '*.o' -or \
		-name '*.out' -or \
		-name '*.script' -or \
		-name '*.tmp' \
	\) \
	-or -type d \( \
		-name '*.dSYM' -or \
		-name 'Output' \
	\) \
	-not -path "./tools"

CLEAN_FILES=$(shell $(CLEAN_FIND_CMD))
clean: ## Clean all temporary artefacts
	echo $(CLEAN_FILES) | \
		xargs rm -rf
