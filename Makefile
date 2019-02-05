MAKEFLAGS += --warn-undefined-variables

BUILD_DIR_XCODE?=$(PWD)/BuildXcode
BUILD_DIR_NINJA?=$(PWD)/BuildNinja

MULL_UNIT_TESTS_DIR=$(BUILD_DIR_NINJA)/unittests
MULL_UNIT_TESTS=$(MULL_UNIT_TESTS_DIR)/MullUnitTests

INSTALL_DIR?=/usr/local

CMAKE_COMMAND_LINE_DEBUG_FLAGS=# --debug-output # --debug-output --trace --trace-expand # --trace # --debug-output #

OS?=$(shell uname -s)

# Self-Documented Makefile
# http://marmelab.com/blog/2016/02/29/auto-documented-makefile.html
.PHONY: help
help: ## Show this help message.
	@grep -E '^[a-zA-Z_\.-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-27s\033[0m %s\n", $$1, $$2}'

clean: xcode.clean ninja.clean ## Remove all the build artefacts
	@true

test: ninja.run.unit-tests ## Run unit tests
	@true

install: ninja.install.mull-driver ## Install mull-driver into INSTALL_DIR (default: /usr/local/)
	@true

###
###  Ninja
###

ninja.init: $(BUILD_DIR_NINJA) ## Prepare Ninja project on macOS
	cd $(BUILD_DIR_NINJA) && cmake -G Ninja \
    -DCMAKE_INSTALL_PREFIX=$(INSTALL_DIR) \
    $(CMAKE_COMMAND_LINE_DEBUG_FLAGS) \
    -DPATH_TO_LLVM=$(PATH_TO_LLVM) \
    ../

ninja.build.mull-driver: ## Build mull-driver on macOS
	cd $(BUILD_DIR_NINJA) && ninja mull-driver
	@echo "Resulting binary:\n"$(BUILD_DIR_NINJA)/tools/driver/mull-driver

ninja.build.mull-cxx: ## Build mull-driver on macOS
	cd $(BUILD_DIR_NINJA) && ninja mull-cxx
	@echo "Resulting binary:\n"$(BUILD_DIR_NINJA)/tools/driver/mull-cxx

ninja.build.unit-tests: ## Build unit-tests on macOS
	cd $(BUILD_DIR_NINJA) && ninja MullUnitTests -v

ninja.install.mull: ninja.build.mull-driver ninja.build.mull-cxx ## Install mull drivers
	cd $(BUILD_DIR_NINJA) && ninja install

ninja.run.unit-tests: ninja.build.unit-tests ## Run unit-tests on macOS
	cd $(MULL_UNIT_TESTS_DIR) && $(MULL_UNIT_TESTS)

ninja.run.example: ninja.install.mull ## Run example
	cd Examples/HelloWorld && \
    export PATH=$(INSTALL_DIR)/bin:$(PATH) && \
    make run_driver run_cxx \
      CXX=$(PATH_TO_LLVM)/bin/clang++

ninja.clean:
	rm -rfv $(BUILD_DIR_NINJA)

$(BUILD_DIR_NINJA):
	mkdir $(BUILD_DIR_NINJA)

###
###  Xcode
###

xcode.init: $(BUILD_DIR_XCODE) ## Build Xcode project with CMake.
	cd $(BUILD_DIR_XCODE) && cmake ../ -G Xcode \
    $(CMAKE_COMMAND_LINE_DEBUG_FLAGS) \
    -DPATH_TO_LLVM=$(PATH_TO_LLVM) \

xcode.kill-and-rebuild: xcode.kill-and-reopen ## Build Xcode project with CMake, kill Xcode, reopen the project in Xcode

xcode.open: ## Open Mull.xcodeproj in Xcode
	open $(BUILD_DIR_XCODE)/Mull.xcodeproj

# This reopen task is mostly needed to do a work that involves serious
# modifications of CMake's files: **/CMakeLists.txt and toolchain files.
# Xcode does not pickup all of the changes in CMake without being reopened.
xcode.kill-and-reopen: ## Kill Xcode and open Mull.xcodeproj in Xcode.
	killall Xcode || true
	open $(BUILD_DIR_XCODE)/Mull.xcodeproj

xcode.clean: ## Delete Xcode CMake build cache.
	rm -rfv $(BUILD_DIR_XCODE)

$(BUILD_DIR_XCODE):
	mkdir $(BUILD_DIR_XCODE)

