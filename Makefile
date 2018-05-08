MAKEFLAGS += --warn-undefined-variables

BUILD_DIR_XCODE?=$(PWD)/BuildXcode
BUILD_DIR_NINJA?=$(PWD)/BuildNinja

MULL_UNIT_TESTS_DIR=$(BUILD_DIR_NINJA)/unittests
MULL_UNIT_TESTS=$(MULL_UNIT_TESTS_DIR)/MullUnitTests

CMAKE_COMMAND_LINE_DEBUG_FLAGS=# --trace # --debug-output # --debug-output --trace --trace-expand # --trace # --debug-output #

LLVM_ROOT_MACOS?=/opt/llvm-3.9

# Self-Documented Makefile
# http://marmelab.com/blog/2016/02/29/auto-documented-makefile.html
.PHONY: help
help: ## Show this help message.
	@grep -E '^[a-zA-Z_\.-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'

generate_fixtures: ## Generates fixtures for tests
	cd lab && make synchronize_fixtures

###
###  Ninja
###

ninja.init.macos: $(BUILD_DIR_NINJA) ## Prepare Ninja project on macOS
	rm -rfv $(BUILD_DIR_NINJA)/CMakeCache.txt
	cd $(BUILD_DIR_NINJA) && cmake -G Ninja \
    -DCMAKE_INSTALL_PREFIX=/usr/local/mull \
    $(CMAKE_COMMAND_LINE_DEBUG_FLAGS) \
    -DLLVM_ROOT=$(LLVM_ROOT_MACOS) \
    -DMULL_SUPPORT_RUST=0 \
    ../

ninja.build.mull-driver.macos: ninja.init.macos ## Build mull-driver on macOS
	cd $(BUILD_DIR_NINJA) && ninja mull-driver
	@echo "Resulting binary:\n"$(BUILD_DIR_NINJA)/tools/driver/mull-driver

ninja.build.unit-tests.macos: ninja.init.macos generate_fixtures ## Build unit-tests on macOS
	cd $(BUILD_DIR_NINJA) && ninja MullUnitTests

ninja.build.example.macos: ninja.build.mull-driver.macos ## Build example on macOS
	cd Examples/HelloWorld && \
    make example \
      MULL=$(BUILD_DIR_NINJA)/tools/driver/mull-driver \
      MULL_CC=$(LLVM_ROOT_MACOS)/bin/clang

ninja.run.unit-tests.macos: ninja.build.unit-tests.macos ## Run unit-tests on macOS
	cd $(MULL_UNIT_TESTS_DIR) && $(MULL_UNIT_TESTS)

ninja.run.example.macos: ninja.build.example.macos ## Run example on macOS
	cd Examples/HelloWorld && \
    make run \
      MULL=$(BUILD_DIR_NINJA)/tools/driver/mull-driver \
      MULL_CC=$(LLVM_ROOT_MACOS)/bin/clang

ninja.clean:
	rm -rfv $(BUILD_DIR_NINJA)

$(BUILD_DIR_NINJA):
	mkdir $(BUILD_DIR_NINJA)

###
### Travis
###

travis.test.macos: ninja.run.unit-tests.macos ninja.run.example.macos

travis.install.macos:
	brew update
	brew upgrade cmake
	brew install zlib
	brew install ncurses
	brew install sqlite
	brew install ninja
	cd /opt && \
    sudo wget http://releases.llvm.org/3.9.0/clang+llvm-3.9.0-x86_64-apple-darwin.tar.xz && \
    sudo tar xvf clang+llvm-3.9.0-x86_64-apple-darwin.tar.xz && \
    sudo mv clang+llvm-3.9.0-x86_64-apple-darwin llvm-3.9

